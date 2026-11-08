// Independent round-trip frequency roots on the stiffest low-G notes.
// Evaluate transfer functions from float coefficients and bisect the actual
// resonances; the optimizer's weighted phase proxy is not the assertion.
// 17 cents halves the pre-change 33.45-cent worst error across these rates.
namespace
{
void dispersionAccuracy()
{
    using namespace aculele;
    using Complex = std::complex<double>;
    double worstPartial = 0.0, worstFundamental = 0.0, magnitudeError = 0.0;
    int checked = 0;
    const auto allpass = [](Complex z, double a1, double a2)
    {
        return (a2 + a1 * z + z * z) / (1.0 + a1 * z + a2 * z * z);
    };
    for (double rate : { 44100.0, 48000.0, 96000.0 })
    for (int shape = 0; shape < 3; ++shape)
    {
        auto engine = std::make_unique<AculeleEngine>();
        EngineParameters parameters;
        parameters.shape = static_cast<BodyShape>(shape);
        parameters.tuning = Tuning::LowG;
        engine->setParameters(parameters);
        engine->prepare(rate, 64);
        for (int fret = 13; fret <= 18; ++fret)
        {
            std::array<double, 13> a {};
            check(Access::args(*engine, 2 * fret, a), "physical stiff-note design request exists");
            auto design = calibrateDispersion(a[0], a[1], a[2], a[3], a[4],
                a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12]);
            const double delay = static_cast<float>(design.delay);
            const int anchor = static_cast<int>(std::floor(delay - 1.1));
            const double fraction = delay - anchor;
            const double t1 = static_cast<float>(-2.0 * (fraction - 2.0) / (fraction + 1.0));
            const double t2 = static_cast<float>((fraction - 2.0) * (fraction - 1.0)
                / ((fraction + 1.0) * (fraction + 2.0)));
            for (std::size_t section = 0; section < design.a1.size(); ++section)
            {
                const float a1 = static_cast<float>(design.a1[section]);
                const float a2 = static_cast<float>(design.a2[section]);
                check(std::abs(a2) < 1.0f && std::abs(a1) < 1.0f + a2,
                      "every quantized dispersion pole remains strictly stable");
            }
            const auto response = [&](double frequency)
            {
                const double omega = 2.0 * piDouble * frequency / rate;
                const Complex z = std::polar(1.0, -omega);
                Complex h = std::polar(1.0, -anchor * omega) * allpass(z, t1, t2);
                for (std::size_t section = 0; section < design.a1.size(); ++section)
                {
                    if (section > 0 && !(design.decayRatio[section] > 0.0))
                        continue;
                    const Complex sectionH = allpass(z,
                        static_cast<float>(design.a1[section]),
                        static_cast<float>(design.a2[section]));
                    magnitudeError = std::max(magnitudeError, std::abs(std::abs(sectionH) - 1.0));
                    h *= sectionH;
                }
                const double lossOmega = 2.0 * std::atan(rate / 48000.0 * std::tan(omega * 0.5));
                const Complex lossZ = std::polar(1.0, -lossOmega);
                for (int index : { 3, 5 })
                    h *= 1.0 - a[index + 1] + a[index + 1]
                        * (1.0 - a[index]) / (1.0 - a[index] * lossZ);
                h /= 1.0 + a[7] * z + a[8] * z * z;
                const float gain = static_cast<float>(1.0 + a[9] + a[10] - a[11] - a[12]);
                const double g0 = 1.0f - gain;
                const double g1 = static_cast<float>(a[12]) - static_cast<float>(a[10]);
                h *= 1.0 - (1.0 - z) * (g0 + g1 * z)
                    / (1.0 + a[9] * z + a[10] * z * z);
                return h;
            };
            for (int n = 1; n <= 12; ++n)
            {
                const double target = n * a[1] * std::sqrt((1.0 + a[0] * n * n) / (1.0 + a[0]));
                if (target >= 0.42 * rate)
                    break;
                double low = target * std::exp2(-40.0 / 1200.0);
                double high = target * std::exp2(40.0 / 1200.0);
                check(std::arg(response(low)) > 0.0 && std::arg(response(high)) < 0.0,
                      "physical resonance is bracketed independently");
                for (int iteration = 0; iteration < 48; ++iteration)
                {
                    const double middle = 0.5 * (low + high);
                    if (std::arg(response(middle)) > 0.0) low = middle;
                    else high = middle;
                }
                const double cents = std::abs(1200.0 * std::log2(0.5 * (low + high) / target));
                if (n == 1) worstFundamental = std::max(worstFundamental, cents);
                else worstPartial = std::max(worstPartial, cents);
            }
            ++checked;
        }
    }
    check(worstPartial < 17.0, "stiff-note resonances halve the prior worst frequency error");
    check(worstFundamental < 0.01, "fundamental remains tuned after all sections");
    check(magnitudeError < 1e-10, "dispersion remains lossless in its stationary transfer");
    std::printf("dispersion roots: %d notes, partial %.6f cents, fundamental %.7f cents, magnitude %.3g\n",
                checked, worstPartial, worstFundamental, magnitudeError);
}
}
