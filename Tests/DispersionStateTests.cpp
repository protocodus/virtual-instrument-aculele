// Exercise the actual inline loop recurrence and its live state handover.
// This is intentionally one translation unit, without linking AculeleDSP.
#include "../Source/DSP/AculeleEngine.cpp"

#include <cstdio>
#include <cstring>
#include <memory>

namespace aculele
{
struct AculeleEngineTestAccess
{
    using Loop = AculeleEngine::StringLoop;
    using Voice = AculeleEngine::Voice;
    static bool args(const AculeleEngine& engine, int slot, std::array<double, 13>& arguments)
    {
        return engine.preparedDispersionArguments(slot, arguments);
    }
    static void scale(Voice& voice, float gain)
    {
        AculeleEngine::scaleStoredWaves(voice, gain);
    }
};
}

namespace
{
using Access = aculele::AculeleEngineTestAccess;
using Loop = Access::Loop;
int failures = 0;
double worstHandover = 0.0;

void check(bool ok, const char* what)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s\n", what);
        ++failures;
    }
}

float input(int sample)
{
    return static_cast<float>(0.3 * std::sin(0.071 * sample)
        + 0.2 * std::cos(0.329 * sample) + 0.1 * std::sin(1.419 * sample));
}

std::unique_ptr<Loop> makeLoop(bool second, bool third)
{
    auto loop = std::make_unique<Loop>();
    loop->setCurrentDelay(61.375f);
    loop->targetDelay = loop->currentDelay;
    loop->loopGain = 1.0f;
    loop->broadLossMix = loop->highLossMix = 0.0f;
    loop->dispersionA1 = -0.7f;
    loop->dispersionA2 = 0.25f;
    loop->secondDispersionActive = second;
    loop->secondDispersionA1 = second ? -0.45f : 0.0f;
    loop->secondDispersionA2 = second ? 0.2f : 0.0f;
    loop->thirdDispersionActive = third;
    loop->thirdDispersionA1 = third ? -0.3f : 0.0f;
    loop->thirdDispersionA2 = third ? 0.15f : 0.0f;
    loop->reset();
    return loop;
}

float step(Loop& loop, float drive)
{
    const float output = loop.advance(0.0f, 1.0f);
    loop.write(drive);
    return output;
}

void seed(Loop& loop)
{
    for (int n = 0; n < 400; ++n)
        step(loop, input(n));
}

void compare(Loop& a, Loop& b, int samples, const char* what)
{
    double error = 0.0;
    for (int n = 0; n < samples; ++n)
        error = std::max(error, static_cast<double>(
            std::abs(step(a, input(n + 400)) - step(b, input(n + 400)))));
    worstHandover = std::max(worstHandover, error);
    check(error < 2.0e-6, what);
}

void liveHandover()
{
    // Third insertion must preserve an already sounding, nonzero-coefficient
    // second section. Reinitializing the latter would make this fail.
    auto reference = makeLoop(true, false);
    seed(*reference);
    auto changed = std::make_unique<Loop>(*reference);
    changed->switchThirdDispersion(true);
    changed->targetDelay -= 2.0f;
    compare(*reference, *changed, 300, "third insertion preserves the live cascade");
    changed->switchThirdDispersion(false);
    changed->targetDelay += 2.0f;
    compare(*reference, *changed, 300, "third removal preserves the live cascade");

    reference = makeLoop(false, false);
    seed(*reference);
    changed = std::make_unique<Loop>(*reference);
    changed->switchSecondDispersion(true);
    changed->switchThirdDispersion(true);
    changed->targetDelay -= 4.0f;
    compare(*reference, *changed, 300, "two delay-only insertions preserve output");
    changed->switchSecondDispersion(false);
    check(changed->secondDispersionActive && changed->thirdDispersionActive,
          "downstream removal cannot invalidate upstream handover");
    changed->switchThirdDispersion(false);
    changed->switchSecondDispersion(false);
    changed->targetDelay += 4.0f;
    compare(*reference, *changed, 300, "upstream-first removal preserves output");
}

void retirementAndReset()
{
    auto retained = makeLoop(true, true);
    seed(*retained);
    auto retired = std::make_unique<Loop>(*retained);
    auto target = retained->intrinsicCoefficientTarget;
    target[5] = target[6] = target[7] = target[8] = 0.0f;
    retained->setIntrinsicCoefficients(target, true, 80);
    retired->setIntrinsicCoefficients(target, true, 80);
    retired->secondDispersionRetiring = retired->thirdDispersionRetiring = true;
    const float originalDelay = retired->currentDelay;
    compare(*retained, *retired, 300, "deferred two-section removal is continuous");
    check(!retired->secondDispersionActive && !retired->thirdDispersionActive
        && !retired->secondDispersionRetiring && !retired->thirdDispersionRetiring
        && retired->currentDelay == originalDelay + 4.0f
        && retired->targetDelay == originalDelay + 4.0f,
        "deferred retirement restores exactly four samples once");

    auto reset = makeLoop(true, true);
    seed(*reset);
    reset->setIntrinsicCoefficients(target, true, 80);
    reset->secondDispersionRetiring = reset->thirdDispersionRetiring = true;
    reset->reset();
    check(!reset->secondDispersionActive && !reset->thirdDispersionActive
        && reset->currentDelay == originalDelay + 4.0f
        && reset->intrinsicCoefficientSamples == 0,
        "reset finishes upstream-first retirement and coefficient transition");
    for (int n = 0; n < 160; ++n)
        check(step(*reset, 0.0f) == 0.0f, "reset clears every section history");
}

// A retained wave takes only the part of the line it can read
// (StringLoop::captureFrom). With the rest of its line poisoned it must
// sound, bit for bit, as a whole copy does in the two cases the reach has a
// term of its own for.
void retainedCapture()
{
    const auto equal = [] (Loop& whole, Loop& captured, bool hand)
    {
        bool same = true;
        for (int n = 0; n < 900; ++n)
        {
            const float a = whole.advance(0.0f, 1.0f);
            const float b = captured.advance(0.0f, 1.0f);
            same = same && std::memcmp(&a, &b, sizeof(float)) == 0;
            whole.write(0.6f * a + input(n + 400));
            captured.write(0.6f * b + input(n + 400));
            if (hand)
            {
                whole.applyGestureContact();
                captured.applyGestureContact();
            }
        }
        return same;
    };
    const int size = static_cast<int>(Loop().delay.size());

    // Two sections still to retire give four samples back to the delay.
    // The wave is taken on the sample before the upstream one hands its two
    // back, so the retained wave's first read is already two samples deeper.
    auto source = makeLoop(true, true);
    seed(*source);
    auto target = source->intrinsicCoefficientTarget;
    target[5] = target[6] = target[7] = target[8] = 0.0f;
    source->setIntrinsicCoefficients(target, true, 80);
    source->secondDispersionRetiring = source->thirdDispersionRetiring = true;
    for (int n = 0; n < 400 && !(source->intrinsicCoefficientSamples == 0
                                 && source->thirdDispersionZeroHistory >= 2); ++n)
        step(*source, input(n));
    check(source->thirdDispersionRetiring && source->thirdDispersionActive
        && source->thirdDispersionZeroHistory >= 2,
          "the wave is retained on the sample before a section retires");
    const float originalDelay = source->currentDelay;
    auto whole = std::make_unique<Loop>(*source);
    auto captured = std::make_unique<Loop>();
    int reach = captured->captureFrom(*source, true);
    check(reach == static_cast<int>(originalDelay) + 5 && reach < size,
          "a retained wave takes its delay and its retiring sections' four samples");
    check(equal(*whole, *captured, false)
        && captured->currentDelay == originalDelay + 4.0f
        && !captured->secondDispersionActive && !captured->thirdDispersionActive,
        "a retained wave retires two sections on the line it took");

    // A hand that landed when the string was longer reaches further back
    // than the delay.
    source = makeLoop(false, false);
    source->setCurrentDelay(240.5f);
    source->targetDelay = source->currentDelay;
    seed(*source);
    source->beginGestureContact(0.37f, 0.2f, 288);
    for (int n = 0; n < 40; ++n)
    {
        step(*source, input(n));
        source->applyGestureContact();
    }
    source->setCurrentDelay(61.375f);
    source->targetDelay = source->currentDelay;
    whole = std::make_unique<Loop>(*source);
    captured = std::make_unique<Loop>();
    reach = captured->captureFrom(*source, true);
    check(source->gestureContact.active
        && reach == source->gestureContact.secondAge + 2
        && reach > static_cast<int>(source->currentDelay) + 5 && reach < size,
        "a retained wave takes the line its hand's contact reaches");
    check(equal(*whole, *captured, true),
          "a retained wave's hand works on the line it took");

    // A delay as long as the line keeps all of it.
    source = makeLoop(false, false);
    source->setCurrentDelay(static_cast<float>(size - 3));
    source->targetDelay = source->currentDelay;
    check(captured->captureFrom(*source, true) == size,
          "a delay as long as the line keeps the whole line");
}

void warmAndAdd()
{
    auto first = makeLoop(true, true);
    auto second = makeLoop(true, true);
    auto combined = makeLoop(true, true);
    std::array<float, 200> a {}, b {}, sum {};
    for (std::size_t n = 0; n < a.size(); ++n)
    {
        a[n] = input(static_cast<int>(n));
        b[n] = input(static_cast<int>(n) + 73);
        sum[n] = a[n] + 0.375f * b[n];
    }
    first->warmSections(a.data(), static_cast<int>(a.size()));
    second->warmSections(b.data(), static_cast<int>(b.size()));
    combined->warmSections(sum.data(), static_cast<int>(sum.size()));
    first->addSectionStates(*second, 0.375f);
    compare(*first, *combined, 300,
        "warming and retained-tail addition obey linear superposition");

    // A separate direct-form cascade establishes that warmSections includes
    // the third at the correct position, rather than merely agreeing with
    // another call to warmSections that omits the same state.
    auto manual = makeLoop(true, true);
    for (float x : sum)
    {
        x = manual->thirdDispersion.process(x, manual->thirdDispersionA1,
                                              manual->thirdDispersionA2);
        x = manual->secondDispersion.process(x, manual->secondDispersionA1,
                                               manual->secondDispersionA2);
        manual->dispersion.process(x, manual->dispersionA1, manual->dispersionA2);
    }
    auto warmed = makeLoop(true, true);
    warmed->warmSections(sum.data(), static_cast<int>(sum.size()));
    check(warmed->thirdDispersion.y1 == manual->thirdDispersion.y1
        && warmed->secondDispersion.y1 == manual->secondDispersion.y1
        && warmed->dispersion.y1 == manual->dispersion.y1,
        "warm histories equal an independent three-section cascade");

    auto voice = std::make_unique<Access::Voice>();
    voice->loops[0] = *warmed;
    Access::scale(*voice, 0.25f);
    const auto& after = voice->loops[0].thirdDispersion;
    const auto& before = warmed->thirdDispersion;
    check(after.x1 == 0.25f * before.x1 && after.x2 == 0.25f * before.x2
        && after.y1 == 0.25f * before.y1 && after.y2 == 0.25f * before.y2,
        "stored-wave scaling includes all third-section histories");
}

void coefficientDeadline()
{
    auto loop = makeLoop(true, true);
    auto target = loop->intrinsicCoefficientTarget;
    target[7] = -0.6f;
    target[8] = 0.3f;
    loop->setIntrinsicCoefficients(target, true, 100);
    for (int n = 0; n < 17; ++n)
        step(*loop, input(n));
    const auto before = loop->intrinsicCoefficientStep;
    loop->setIntrinsicCoefficients(target, true, 100);
    check(loop->intrinsicCoefficientSamples == 83
        && before == loop->intrinsicCoefficientStep,
        "identical third-section targets retain the transition deadline");
    for (int n = 0; n < 83; ++n)
        step(*loop, input(n));
    check(loop->thirdDispersionA1 == target[7] && loop->thirdDispersionA2 == target[8]
        && loop->intrinsicCoefficientSamples == 0,
        "third-section coefficients reach their exact common deadline");
    loop->reset();
    auto fresh = makeLoop(true, true);
    fresh->setIntrinsicCoefficients(target, false);
    fresh->reset();
    compare(*loop, *fresh, 300, "reset and fresh configuration have identical trajectories");
}
}

#include "DispersionAccuracyChecks.h"

int main()
{
    liveHandover();
    retirementAndReset();
    retainedCapture();
    warmAndAdd();
    coefficientDeadline();
    dispersionAccuracy();
    std::printf("dispersion state: failures=%d, largest output difference=%.9g\n",
                failures, worstHandover);
    return failures == 0 ? 0 : 1;
}
