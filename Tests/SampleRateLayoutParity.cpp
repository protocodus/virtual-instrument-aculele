// Built twice: on the library every other test links, and on the Rack's
// layout (ACULELE_EXTENDED_SAMPLE_RATES, AculeleDSPCxx17), whose delay lines
// are four times as long and wrap differently. Aculele.SampleRateLayoutParity
// compares the two programs' float bytes: the extra capacity must not alter
// a note at any rate a Rack host runs.
#include "DSP/AculelePerformer.h"

#include <array>
#include <fstream>
#include <memory>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;
    std::ofstream output(argv[1], std::ios::binary);
    for (const double rate : { 44100.0, 48000.0, 96000.0, 192000.0 })
    {
        auto performer = std::make_unique<aculele::Performer>();
        performer->prepare(rate, 64);
        aculele::EngineParameters parameters;
        parameters.room = 0.12f;
        parameters.piezoMix = 0.25f;
        performer->setParameters(parameters);
        performer->setTempoBpm(97.0);
        std::array<float, 64> left {}, right {};
        for (int block = 0; block < 1200; ++block)
        {
            performer->beginBlock(left.data(), right.data(), 64);
            if (block == 0)
            {
                performer->noteOn(3, 1, 60, 92);
                performer->noteOn(3, 1, 67, 105);
                performer->noteOn(3, 1, 76, 90);
            }
            if (block == 50)
            {
                performer->noteOff(10, 1, 76);
                performer->noteOn(12, 1, 76, 82);
            }
            if (block == 70) performer->controlChange(17, 1, 1, 30);
            if (block == 120) performer->pitchWheel(29, 1, 0.25f);
            if (block == 160) performer->noteOn(11, 1, 81, 96);
            if (block == 200) performer->controlChange(13, 1, 123, 0);
            // Repeated four-string strokes over ringing strings: retained
            // waves, their copies, and the shorter ring after several wraps.
            if (block >= 400 && block < 1000 && block % 100 == 0)
                for (const int note : { 67, 60, 64, 69 })
                {
                    performer->noteOff(2, 1, note);
                    performer->noteOn(3, 1, note, 110);
                }
            performer->endBlock();
            if (block == 1050) performer->reset();
            for (const auto* bus : { &left, &right })
                output.write(reinterpret_cast<const char*>(bus->data()),
                             static_cast<std::streamsize>(sizeof(float) * bus->size()));
        }
    }
    output.close();
    return output ? 0 : 1;
}
