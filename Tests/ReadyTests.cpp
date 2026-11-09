// The instrument stays silent until it is ready (owner, 2026-11-09): until
// every string's dispersion design is made, a note would run its own search
// in its note-on block, so the Performer ignores note-ons
// (AculeleEngine::ready). For every factory preset at 48 kHz:
//
// - a freshly prepared Performer is ready, and a note on its first sample
//   sounds;
// - after a Shape change it is not ready, and a note-on then changes no
//   sample against a twin that was sent none;
// - it is ready again within 10 s, and a note then sounds;
// - a Wood change, which leaves every design as it was, is ready again
//   within eleven steps of the designs (2,816 samples): ten steps of
//   sixteen checks cover the 152 slots, and the first may come up to a
//   step late.
//
// 64-sample blocks, both main channels compared.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 64;
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
    // Messages for the next block's first sample, which the Performer takes
    // between beginBlock and endBlock.
    std::array<std::array<std::uint8_t, 3>, 4> pending {};
    int pendingCount = 0;

    void message(std::uint8_t status, std::uint8_t data1, std::uint8_t data2)
    {
        pending[static_cast<std::size_t>(pendingCount++)] = { status, data1, data2 };
    }
    void render()
    {
        performer->beginBlock(left.data(), right.data(), block);
        for (int index = 0; index < pendingCount; ++index)
            performer->handleMidi(0, pending[static_cast<std::size_t>(index)].data(), 3);
        pendingCount = 0;
        performer->endBlock();
    }
    // Blocks until the engine is ready, at most `limit`; the samples taken.
    int renderUntilReady(int limit)
    {
        int samples = 0;
        while (!performer->engine().ready() && samples < limit)
        {
            render();
            samples += block;
        }
        return samples;
    }
};

double blockEnergy(const Player& player)
{
    double sum = 0.0;
    for (int i = 0; i < block; ++i)
        sum += static_cast<double>(player.left[static_cast<std::size_t>(i)])
                * player.left[static_cast<std::size_t>(i)]
            + static_cast<double>(player.right[static_cast<std::size_t>(i)])
                * player.right[static_cast<std::size_t>(i)];
    return sum;
}

bool sameBlock(const Player& a, const Player& b)
{
    return a.left == b.left && a.right == b.right;
}
} // namespace

int main()
{
    for (const auto& preset : aculele::factoryPresets)
    {
        const auto parameters = aculele::engineParameters(preset);
        Player played;
        played.performer->setParameters(parameters);
        played.performer->prepare(rate, block);
        played.performer->setParameters(parameters);
        check(played.performer->engine().ready(), "ready once prepared", preset.name);

        // A note on the first sample sounds.
        played.message(0x90, 64, 100);
        double energy = 0.0;
        for (int index = 0; index < 20; ++index)
        {
            played.render();
            energy += blockEnergy(played);
        }
        check(energy > 0.0, "a note on the first sample sounds", preset.name);

        Player a, b;
        for (auto* player : { &a, &b })
        {
            player->performer->setParameters(parameters);
            player->performer->prepare(rate, block);
            player->performer->setParameters(parameters);
        }

        // A Shape change: not ready, and a note-on changes nothing.
        auto reshaped = parameters;
        reshaped.shape = parameters.shape == aculele::BodyShape::Tenor
            ? aculele::BodyShape::Soprano : aculele::BodyShape::Tenor;
        a.performer->setParameters(reshaped);
        b.performer->setParameters(reshaped);
        check(!a.performer->engine().ready(), "unready after a Shape change", preset.name);
        a.message(0x90, 64, 100);
        bool silent = true;
        for (int index = 0; index < 8 && !a.performer->engine().ready(); ++index)
        {
            a.render();
            b.render();
            silent = silent && sameBlock(a, b);
        }
        check(silent, "a note-on while unready changes no sample", preset.name);
        a.message(0x80, 64, 0);

        const int waited = a.renderUntilReady(static_cast<int>(10.0 * rate));
        check(a.performer->engine().ready(), "ready within 10 s of a Shape change", preset.name);
        b.renderUntilReady(static_cast<int>(10.0 * rate));
        a.message(0x90, 64, 100);
        energy = 0.0;
        bool differs = false;
        for (int index = 0; index < 20; ++index)
        {
            a.render();
            b.render();
            energy += blockEnergy(a);
            differs = differs || !sameBlock(a, b);
        }
        check(differs && energy > 0.0, "a note sounds once ready again", preset.name);
        a.message(0x80, 64, 0);

        // A Wood change leaves every design as it was.
        auto rewooded = reshaped;
        rewooded.bodyMaterial = reshaped.bodyMaterial == aculele::BodyMaterial::Spruce
            ? aculele::BodyMaterial::Mahogany : aculele::BodyMaterial::Spruce;
        a.performer->setParameters(rewooded);
        const int rewoodWait = a.renderUntilReady(static_cast<int>(10.0 * rate));
        check(a.performer->engine().ready() && rewoodWait <= 2816,
              "ready within 2,816 samples of a Wood change", preset.name);
        std::printf("%s: ready %.2f s after a Shape change, %d samples after a Wood change\n",
                    preset.name, waited / rate, rewoodWait);
    }
    if (failures == 0)
        std::printf("PASS ready\n");
    return failures == 0 ? 0 : 1;
}
