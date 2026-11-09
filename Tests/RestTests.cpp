// A sounding instrument comes to exact rest, held strings included
// (AculeleEngine::processIdleFlush), and computes on no subnormal number
// once it is there. Before 0.4.60 a chord kept under its keys or the pedal
// never reached the idle flush, which waited for every string to be handed
// back: its waves and their filters' memories ran down into subnormal
// numbers and stayed there, and where the host has not set flush-to-zero a
// block then took 5-7 times as long for as long as the chord was held. An
// idle instrument's level observers stopped at the smallest subnormal the
// same way (Docs/validation/consolidation-audit-2026-11-09.md).
//
// For every factory preset, through the Performer at 48 kHz in 256-sample
// blocks, as a host that sets neither flush-to-zero nor denormals-are-zero:
//
// - four notes are struck; one is let go and damped, two are kept by the
//   pedal and one by its key. Within 60 s the instrument is flushed with
//   three strings still played: every string's line, loop filter memories
//   and level are exactly zero, the output is exact silence, and no block
//   raises the denormal-operand or underflow flag;
// - the body changes size under the resting chord: the fingers' shift along
//   the new scale is all that sounds (under -60 dBFS), the designs are made
//   again within 4 s, and at rest again no block raises either flag;
// - a note the pedal holds, struck again, sounds;
// - everything let go, the instrument is flushed again within 60 s with no
//   string played, silent and raising neither flag, though the engine is
//   awake (nothing here suspends it).

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>

#if defined(__SSE2__) || defined(_M_X64)
#include <xmmintrin.h>
#define ACULELE_TEST_HAS_MXCSR 1
#endif

namespace aculele
{
struct AculeleEngineTestAccess
{
    static bool flushed(const AculeleEngine& engine) { return engine.idleFlushed_; }
    // Every string's line, each loop filter's memory and its level are zero.
    static bool stringsAtRest(const AculeleEngine& engine)
    {
        const auto atRest = [] (const AculeleEngine::SecondOrderAllpass& section)
        {
            return section.x1 == 0.0f && section.x2 == 0.0f
                && section.y1 == 0.0f && section.y2 == 0.0f;
        };
        for (const auto& voice : engine.voices_)
        {
            if (voice.level != 0.0f)
                return false;
            for (const auto& loop : voice.loops)
            {
                for (std::size_t index = 0; index < loop.delay.size(); ++index)
                    if (loop.delay[index] != 0.0f)
                        return false;
                if (loop.allpassY1 != 0.0f || loop.allpassY2 != 0.0f
                    || loop.lossFilter.state != 0.0f || loop.broadLossFilter.state != 0.0f
                    || loop.bendingLossY1 != 0.0f || loop.bendingLossY2 != 0.0f
                    || loop.constantLossY1 != 0.0f || loop.constantLossY2 != 0.0f
                    || !atRest(loop.dispersion) || !atRest(loop.secondDispersion)
                    || !atRest(loop.thirdDispersion)
                    || loop.gestureContact.memory != 0.0f)
                    return false;
            }
        }
        return true;
    }
};
} // namespace aculele

namespace
{
using Access = aculele::AculeleEngineTestAccess;
constexpr double rate = 48000.0;
constexpr int block = 256;
int failures = 0;

void check(bool ok, const char* what, const char* preset)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s: %s\n", preset, what);
        ++failures;
    }
}

struct Player
{
    std::unique_ptr<aculele::Performer> performer = std::make_unique<aculele::Performer>();
    std::array<float, block> left {}, right {};
    std::array<std::array<std::uint8_t, 3>, 8> pending {};
    int pendingCount = 0;

    void message(std::uint8_t status, std::uint8_t data1, std::uint8_t data2)
    {
        pending[static_cast<std::size_t>(pendingCount++)] = { status, data1, data2 };
    }
    // One block, the pending messages on its first sample; its peak.
    float render()
    {
        performer->beginBlock(left.data(), right.data(), block);
        for (int index = 0; index < pendingCount; ++index)
            performer->handleMidi(0, pending[static_cast<std::size_t>(index)].data(), 3);
        pendingCount = 0;
        performer->endBlock();
        float peak = 0.0f;
        for (int index = 0; index < block; ++index)
            peak = std::max({ peak, std::abs(left[static_cast<std::size_t>(index)]),
                              std::abs(right[static_cast<std::size_t>(index)]) });
        return peak;
    }
    float render(double seconds)
    {
        float peak = 0.0f;
        for (int count = static_cast<int>(seconds * rate) / block; count > 0; --count)
            peak = std::max(peak, render());
        return peak;
    }
    // Blocks until the instrument is flushed, at most 60 s; the seconds.
    double renderUntilFlushed()
    {
        int blocks = 0;
        const int limit = static_cast<int>(60.0 * rate) / block;
        while (blocks < limit && !Access::flushed(performer->engine()))
        {
            render();
            ++blocks;
        }
        return blocks * block / rate;
    }
    // The peak of `blocks` more, and whether any raised the
    // denormal-operand or underflow flag.
    struct Rest { float peak; bool subnormal; };
    Rest renderAtRest(int blocks)
    {
        Rest rest { 0.0f, false };
#if defined(ACULELE_TEST_HAS_MXCSR)
        _mm_setcsr(_mm_getcsr() & ~0x003Fu);
#endif
        for (int index = 0; index < blocks; ++index)
            rest.peak = std::max(rest.peak, render());
#if defined(ACULELE_TEST_HAS_MXCSR)
        constexpr unsigned denormalOperand = 0x0002u, underflow = 0x0010u;
        rest.subnormal = (_mm_getcsr() & (denormalOperand | underflow)) != 0u;
#endif
        return rest;
    }
};
} // namespace

int main()
{
#if defined(ACULELE_TEST_HAS_MXCSR)
    // Neither flush-to-zero nor denormals-are-zero, whatever the test's
    // own host set.
    const unsigned hostState = _mm_getcsr();
    _mm_setcsr(hostState & ~0x8040u);
#endif
    for (const auto& preset : aculele::factoryPresets)
    {
        Player player;
        auto parameters = aculele::engineParameters(preset);
        player.performer->setParameters(parameters);
        player.performer->prepare(rate, block);
        player.performer->setParameters(parameters);
        const auto& engine = player.performer->engine();

        // In every tuning's range, so that no note is moved an octave or
        // left out (Performer::foldChord).
        const std::array<std::uint8_t, 4> chord { 67, 72, 64, 69 };
        for (const auto note : chord)
            player.message(0x90, note, 110);
        player.render(0.1);
        player.message(0x80, chord[3], 0);
        player.render(0.1);
        player.message(0xB0, 64, 127);
        player.render(0.1);
        player.message(0x80, chord[1], 0);
        player.message(0x80, chord[2], 0);
        player.render(0.7);
        check(engine.getActiveVoiceCount() == 3, "three strings are held", preset.name);

        const double heldRest = player.renderUntilFlushed();
        check(Access::flushed(engine), "a held chord comes to rest within 60 s", preset.name);
        if (!Access::flushed(engine))
            continue;
        check(engine.getActiveVoiceCount() == 3, "the flush keeps held strings held", preset.name);
        check(Access::stringsAtRest(engine),
              "a string at rest keeps no wave, filter memory or level", preset.name);
        auto rest = player.renderAtRest(12);
        check(rest.peak == 0.0f, "a held chord at rest is exact silence", preset.name);
        check(!rest.subnormal, "a held chord at rest computes on no subnormal number",
              preset.name);

        // The designs are made again within 2.5 s of a Shape change.
        parameters.shape = parameters.shape == aculele::BodyShape::Tenor
            ? aculele::BodyShape::Soprano : aculele::BodyShape::Tenor;
        player.performer->setParameters(parameters);
        const float changePeak = player.render(4.0);
        check(engine.ready(), "ready within 4 s of a Shape change", preset.name);
        // The fingers' shift along the new scale is heard, faintly.
        check(changePeak < 1.0e-3f, "a body change under a resting chord is near silent",
              preset.name);
        player.renderUntilFlushed();
        check(Access::flushed(engine) && engine.getActiveVoiceCount() == 3,
              "a held chord comes to rest again after a body change", preset.name);
        rest = player.renderAtRest(12);
        check(rest.peak == 0.0f && !rest.subnormal,
              "a resting chord after a body change computes on no subnormal number",
              preset.name);

        // A string the pedal holds, struck again.
        player.message(0x90, chord[1], 110);
        check(player.render(0.2) > 0.01f, "a held string struck after its rest sounds",
              preset.name);

        player.message(0x80, chord[0], 0);
        player.message(0x80, chord[1], 0);
        player.message(0xB0, 64, 0);
        player.render(0.5);
        const double idleRest = player.renderUntilFlushed();
        check(Access::flushed(engine) && engine.getActiveVoiceCount() == 0,
              "let go, the instrument is flushed with no string played", preset.name);
        check(Access::stringsAtRest(engine),
              "an idle string keeps no wave, filter memory or level", preset.name);
        rest = player.renderAtRest(12);
        check(rest.peak == 0.0f, "an idle instrument is exact silence", preset.name);
        check(!rest.subnormal, "an idle instrument computes on no subnormal number",
              preset.name);
        std::printf("%s: at rest %.1f s after the held chord, %.1f s after it was let go\n",
                    preset.name, 1.0 + heldRest, 0.5 + idleRest);
    }
#if defined(ACULELE_TEST_HAS_MXCSR)
    _mm_setcsr(hostState);
#endif
    if (failures == 0)
        std::printf("PASS rest\n");
    return failures == 0 ? 0 : 1;
}
