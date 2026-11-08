// The saddle is a one-way force observer. Its load must follow the same
// unweighted normal ports as the mechanical junction, without redesigning a
// filter or clearing its histories on the audio thread. Check the prepared
// bank against the analog component model and direct designs, then exercise
// the real construction, tuning, bend, tail and held-load control paths.
#include "DSP/AculeleEngine.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <limits>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static auto direct(double rate, double z)
    { return AculeleEngine::designPiezoSaddle(rate, z); }
    static auto filter(const AculeleEngine& e) { return e.piezoSaddle_; }
    static auto bank(const AculeleEngine& e) { return e.piezoSaddleBank_; }
    static void prepareBankAt(AculeleEngine& e, double rate)
    {
        // The bank is independent of delay storage. Exercise the extended
        // Reason 192 kHz * 4 domain even in the default core test build.
        e.sampleRate_ = rate;
        e.preparePiezoSaddleBank();
    }
    static float maximum(const AculeleEngine& e) { return e.piezoSaddleMaximum_; }
    static float anchor(const AculeleEngine& e) { return e.piezoSaddleAnchor_; }
    static float selected(const AculeleEngine& e) { return e.piezoSaddleImpedance_; }
    static float junction(const AculeleEngine& e) { return e.lastImpedanceSum_; }
    static double settledPortTolerance(const AculeleEngine& e)
    {
        // Each of the two existing float followers stalls once its update
        // falls below half an ulp. Bound their combined residue; the pickup
        // must track this actual load, not silently snap it to its target.
        return 2.0 * std::numeric_limits<float>::epsilon() / e.delaySmoothing_;
    }
    static void select(AculeleEngine& e, float z) { e.updatePiezoSaddle(z); }
    static void force(AculeleEngine& e, float f) { e.renderPiezo(f); }
    static auto inputHistory(const AculeleEngine& e) { return e.piezoSaddleInput_; }
    static auto outputHistory(const AculeleEngine& e) { return e.piezoSaddleOutput_; }
    static float ports(const AculeleEngine& e)
    {
        float result = 0.0f;
        for (const auto& v : e.voices_)
            if (v.played || e.sympatheticStringsEnabled_)
                result += v.characteristicImpedance * v.appliedBendImpedanceScale
                    + (v.tailActive ? v.tailCharacteristicImpedance : 0.0f);
        return result;
    }
    static bool hasTail(const AculeleEngine& e)
    {
        for (const auto& v : e.voices_) if (v.tailActive) return true;
        return false;
    }
    static void retirePorts(AculeleEngine& e)
    {
        for (auto& v : e.voices_) { v.played = false; v.tailActive = false; }
        e.sympatheticStringsEnabled_ = false;
    }
    static void silenceSensorBank(AculeleEngine& e)
    {
        for (auto& f : e.piezoSaddleBank_) f.b.fill(0.0f);
        e.piezoSaddleImpedance_ = -1.0f;
    }
    static auto mechanicalState(const AculeleEngine& e)
    {
        const auto& p = e.bridgeLoad_;
        return std::array<float, 6> { p.displacement, p.rotation,
            p.mainIntegratedForce, p.mainIntegratedMoment,
            p.bodyIntegratedForce, p.bodyIntegratedMoment };
    }
};
}

namespace
{
using Engine = aculele::AculeleEngine;
using Access = aculele::AculeleEngineTestAccess;
constexpr double pi = 3.14159265358979323846;
int failures = 0;
void expect(bool pass, const char* message)
{
    if (!pass) { ++failures; std::fprintf(stderr, "FAIL %s\n", message); }
}

template <typename Filter>
std::complex<double> response(const Filter& f, double hz, double rate)
{
    const auto z = std::polar(1.0, -2.0 * pi * hz / rate);
    std::complex<double> b {}, power { 1.0, 0.0 };
    for (float tap : f.b) { b += double(tap) * power; power *= z; }
    return b / (1.0 + double(f.a1) * z + double(f.a2) * z * z);
}

std::complex<double> analog(double hz, double load)
{
    // Independent component relation; no fitted coefficient is used here.
    const std::complex<double> s { 0.0, 2.0 * pi * hz };
    const double omega = 2.0 * pi * 6000.0;
    const double k = 0.0038 * omega * omega;
    const auto seat = k / s + k / (18.0 * omega);
    const auto mobility = 1.0 / (0.0038 * s + load) + 0.009;
    return seat * mobility / (1.0 + seat * mobility);
}

void bankAccuracy()
{
    auto engine = std::make_unique<Engine>();
    for (int rate : { 8000, 16000, 44100, 48000, 96000, 192000, 384000, 768000 })
    {
        Access::prepareBankAt(*engine, rate);
        const double maximum = Access::maximum(*engine);
        expect(maximum >= 2.35514 && maximum <= 2.356001,
               "prepared load bound disagrees with supported string domain");
        const double anchor = Access::anchor(*engine);
        expect(std::abs(anchor - 0.55709017) < 1e-6,
               "default Concert/Nylon/Standard load anchor changed");
        Access::select(*engine, float(anchor));
        const auto anchorFilter = Access::filter(*engine);
        const auto directAnchor = Access::direct(rate, anchor);
        expect(anchorFilter.b == directAnchor.b && anchorFilter.a1 == directAnchor.a1
                   && anchorFilter.a2 == directAnchor.a2,
               "default anchor is not an exact directly designed filter");
        double worstDb = 0.0, worstPhase = 0.0, worstTop = 0.0;
        double interpolationDb = 0.0, interpolationPhase = 0.0, worstDc = 0.0;
        for (int point = 0; point <= 256; ++point)
        {
            // Include every grid point and quarter interval in both spans.
            const double load = point <= 128 ? anchor * point / 128.0
                : anchor + (maximum - anchor) * (point - 128) / 128.0;
            Access::select(*engine, float(load));
            const auto f = Access::filter(*engine);
            expect(1.0 + f.a1 + f.a2 > 0.0 && 1.0 - f.a1 + f.a2 > 0.0
                       && 1.0 - f.a2 > 0.0, "interpolated saddle is not Schur stable");
            double dc = 0.0, absolute = std::abs(f.a1) + std::abs(f.a2);
            for (float b : f.b) { dc += b; absolute += std::abs(b); }
            const double denominator = 1.0 + f.a1 + f.a2;
            const double dcError = std::abs(dc / denominator - 1.0);
            worstDc = std::max(worstDc, dcError);
            // At 384/768 kHz the denominator's DC cancellation amplifies
            // float coefficient roundoff, in the original direct fit too.
            // Two epsilons times the coefficient norm covers float storage
            // and interpolation before that amplification. Ordinary rates
            // retain the existing 1e-5 relative DC bound as well.
            expect(std::abs(dc - denominator) <= 2.0
                       * std::numeric_limits<float>::epsilon() * absolute,
                   "interpolated saddle loses DC beyond float rounding");
            if (rate <= 192000)
                expect(dcError < 1e-5, "ordinary-rate saddle loses unit DC transmission");
            // Direct fits at every eighth interval midpoint isolate the
            // bank interpolation error from the existing analog fit error.
            const bool compareDirect = point % 32 == 2;
            const auto direct = compareDirect ? Access::direct(rate, float(load)) : f;
            const double top = std::min(20000.0, 0.45 * rate);
            for (int bin = 0; bin <= 256; ++bin)
            {
                const double hz = 20.0 * std::pow(top / 20.0, bin / 256.0);
                const auto digital = response(f, hz, rate);
                const auto ratio = digital * std::polar(1.0, 2.0*pi*hz*7.0/rate)
                    / analog(hz, float(load));
                const double db = std::abs(20.0 * std::log10(std::abs(ratio)));
                if (hz <= 12000.0 || rate >= 88200) worstDb = std::max(worstDb, db);
                else worstTop = std::max(worstTop, db);
                worstPhase = std::max(worstPhase, std::abs(std::arg(ratio)) * 180.0/pi);
                if (compareDirect)
                {
                    const auto error = digital / response(direct, hz, rate);
                    interpolationDb = std::max(interpolationDb,
                        std::abs(20.0 * std::log10(std::abs(error))));
                    interpolationPhase = std::max(interpolationPhase,
                        std::abs(std::arg(error)) * 180.0/pi);
                }
            }
        }
        expect(interpolationDb < 0.002 && interpolationPhase < 0.01,
               "load-bank interpolation exceeds its numerical error budget");
        // The supported 8 kHz FIR is intentionally only a bounded low-rate
        // approximation; retain the direct design's established behavior.
        if (rate >= 16000)
            expect(worstDb <= 0.35 && worstTop <= 0.6 && worstPhase <= 2.0,
                   "live load bank exceeds the independent analog response bound");
        std::printf("load bank %d: analog %.6g dB / %.6g deg, top %.6g dB; "
                    "interpolation %.6g dB / %.6g deg; DC %.6g\n", rate,
                    worstDb, worstPhase, worstTop, interpolationDb, interpolationPhase, worstDc);
    }
}

void tick(Engine& e, int samples)
{
    for (int sample = 0; sample < samples; ++sample)
    {
        float l {}, r {};
        e.process(&l, &r, 1);
        expect(std::isfinite(l) && std::isfinite(r), "nonfinite live load transition");
        expect(Access::selected(e) == Access::junction(e),
               "sensor load does not equal actual held normal junction load");
        expect(Access::junction(e) <= Access::maximum(e),
               "physical junction exceeded the derived load-bank domain");
    }
}

void liveControls()
{
    auto live = std::make_unique<Engine>();
    auto fresh = std::make_unique<Engine>();
    aculele::PerformanceRealism realism;
    realism.frettingPitch = false; // compare the same tension, no contact draw
    live->setPerformanceRealism(realism);
    fresh->setPerformanceRealism(realism);
    live->prepare(48000, 1);
    fresh->prepare(48000, 1);
    aculele::EngineParameters p;
    live->setStringPerChannelMode(true);
    live->noteOn(67, 0.7f, 1);
    tick(*live, 4096);
    for (int shape = 0; shape < 3; ++shape)
        for (int material = 0; material < 2; ++material)
            for (int tuning = 0; tuning < 3; ++tuning)
            {
                p.shape = static_cast<aculele::BodyShape>(shape);
                p.strings = static_cast<aculele::StringMaterial>(material);
                p.tuning = static_cast<aculele::Tuning>(tuning);
                live->setParameters(p);
                tick(*live, 7200);
                fresh->setParameters(p);
                fresh->reset();
                tick(*fresh, 1);
                const float z = Access::selected(*live), cold = Access::selected(*fresh);
                std::printf("construction %d/%d/%d load live %.9g cold %.9g delta %.9g\n",
                    shape, material, tuning, z, cold, z-cold);
                expect(std::abs(z - cold) < Access::settledPortTolerance(*live) * cold,
                       "settled live construction load differs from fresh instrument");
                const auto a = Access::filter(*live), b = Access::filter(*fresh);
                expect(std::abs(response(a, 6000, 48000) / response(b, 6000, 48000) - 1.0) < 1e-5,
                       "settled live sensor transfer differs from fresh construction");
            }
    const float beforeTune = Access::selected(*live);
    live->setMasterTuneCents(100.0f);
    tick(*live, 1);
    expect(std::abs(Access::selected(*live) - beforeTune) < 1e-5,
           "master retune bypasses the mechanical port's existing follower");
    tick(*live, 7200);
    fresh->setMasterTuneCents(100.0f);
    fresh->reset();
    tick(*fresh, 1);
    expect(std::abs(Access::selected(*live) - Access::selected(*fresh))
               < Access::settledPortTolerance(*live) * Access::selected(*fresh),
           "settled master tuning does not match a fresh sensor load");
    std::printf("master load live %.9g cold %.9g\n", Access::selected(*live), Access::selected(*fresh));

    // Different-pitch refret on the same tab string creates a retained tail.
    live->setParameters({}); live->setMasterTuneCents(0); live->reset();
    live->noteOn(67, 0.7f, 1); tick(*live, 2048);
    live->noteOn(69, 0.7f, 1);
    bool sawTail = false, sawRetirement = false;
    for (int i = 0; i < 16000; ++i)
    {
        tick(*live, 1);
        sawTail = sawTail || Access::hasTail(*live);
        sawRetirement = sawRetirement || (sawTail && !Access::hasTail(*live));
    }
    expect(sawTail && sawRetirement, "tail loading and retirement were not exercised");
    expect(std::abs(Access::ports(*live) - Access::selected(*live)) < 1e-6,
           "sensor load does not match unweighted branch inventory");
    const float held = Access::selected(*live);
    Access::retirePorts(*live);
    tick(*live, 1);
    expect(Access::selected(*live) == held, "empty inventory loses held junction load");

    live->setSympatheticStringsEnabled(true); live->reset();
    live->setStringPerChannelMode(false); live->setLowerZoneMemberCount(4);
    live->noteOn(67, 0.7f, 2); tick(*live, 4096);
    const float unbent = Access::selected(*live);
    live->setPitchBend(2.0f, 1); tick(*live, 7200);
    expect(std::abs(Access::selected(*live) - unbent) < 1e-6,
           "manager slide incorrectly changes saddle mechanical load");
    live->setPitchBend(2.0f, 2); tick(*live, 7200);
    expect(Access::selected(*live) > unbent, "member tension bend did not load the sensor");
    live->setPitchBend(6.0f, 2); tick(*live, 7200);
    const float saturated = Access::selected(*live);
    live->setPitchBend(48.0f, 2); tick(*live, 7200);
    std::printf("bend saturation six %.9g forty-eight %.9g\n", saturated, Access::selected(*live));
    expect(std::abs(Access::selected(*live) - saturated) < 2e-5,
           "member slide beyond tension limit changes sensor load");
    std::puts("live construction/master/member/manager/tail/held-load paths passed");
}

void historiesAndIsolation()
{
    auto a = std::make_unique<Engine>();
    auto b = std::make_unique<Engine>();
    a->prepare(48000, 1); b->prepare(48000, 1);
    for (int i = 0; i < 64; ++i) Access::force(*a, float(std::sin(0.1*i)*0.01));
    const auto input = Access::inputHistory(*a);
    const auto output = Access::outputHistory(*a);
    const auto bank = Access::bank(*a);
    const auto unchanged = Access::filter(*a);
    for (float invalid : { std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN() })
        Access::select(*a, invalid);
    expect(Access::filter(*a).b == unchanged.b
               && Access::filter(*a).a1 == unchanged.a1
               && Access::filter(*a).a2 == unchanged.a2,
           "invalid observer load changes coefficients");
    Access::select(*a, Access::maximum(*a));
    expect(input == Access::inputHistory(*a) && output == Access::outputHistory(*a),
           "load update clears or rewrites sensor force histories");
    a->reset();
    const auto after = Access::bank(*a);
    bool same = true;
    for (std::size_t i=0; i<bank.size(); ++i)
        same = same && bank[i].b == after[i].b
            && bank[i].a1 == after[i].a1 && bank[i].a2 == after[i].a2;
    expect(same, "reset changes the prepared filter bank");
    Access::silenceSensorBank(*b);
    a->noteOn(67, 0.7f); b->noteOn(67, 0.7f);
    bool equal = true;
    for (int i = 0; i < 12000; ++i)
    {
        if (i == 4096) { a->setMasterTuneCents(100); b->setMasterTuneCents(100); }
        float al {}, ar {}, bl {}, br {};
        a->process(&al, &ar, 1); b->process(&bl, &br, 1);
        equal = equal && Access::mechanicalState(*a) == Access::mechanicalState(*b)
            && al == bl && ar == br;
    }
    expect(equal, "sensor load adds a second mechanical load or changes microphones");
}
}

int main()
{
    bankAccuracy(); liveControls(); historiesAndIsolation();
    if (failures == 0) std::puts("All Aculele live piezo load tests passed");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
