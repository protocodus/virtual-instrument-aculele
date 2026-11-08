#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace aculele
{
enum class HandGesture { StringBrush, FretShift, BodyTap, MuteStrings };

// Short contacts with the instrument. The force and moment drive its
// existing modal body and pickup observers. Friction also has a quiet
// direct path to the microphones, as it radiates at the contact itself.
class HandGestureSource
{
public:
    struct Drive
    {
        float force {}, moment {}, pickup {}, air {}, side {};
    };

    void clear() noexcept { contacts_ = {}; active_ = 0; }
    void restart() noexcept { seed_ = initialSeed; }
    [[nodiscard]] bool active() const noexcept { return active_ > 0; }

    void stopChannels(int first, int last) noexcept
    {
        for (auto& contact : contacts_)
            if (contact.remaining() > 0 && contact.channel >= first && contact.channel <= last)
            {
                contact = {};
                --active_;
            }
    }

    bool trigger(HandGesture gesture, float velocity, int channel, double rate) noexcept
    {
        const auto index = static_cast<std::size_t>(gesture);
        if (index >= profiles.size() || !std::isfinite(velocity) || velocity <= 0.0f)
            return false;

        const auto& profile = profiles[index];
        auto& contact = *std::min_element(contacts_.begin(), contacts_.end(),
            [](const Contact& a, const Contact& b) { return a.remaining() < b.remaining(); });
        if (contact.remaining() == 0)
            ++active_;
        contact = {};
        // Each event gets its own texture stream. Changing an earlier
        // contact's duration or block partition cannot change the next hit.
        seed_ += 0x9e3779b9u;
        auto event = seed_;
        event = (event ^ (event >> 16)) * 0x21f0aaadu;
        event = (event ^ (event >> 15)) * 0x735a2d97u;
        event ^= event >> 15;
        contact.random = event != 0 ? event : initialSeed;
        contact.channel = channel;
        const float strength = std::min(velocity, 1.0f);
        const float level = strength * (0.35f + 0.65f * strength)
            * (1.0f + 0.15f * draw(contact.random));
        const float rateHz = static_cast<float>(rate);
        contact.span = std::max(3, static_cast<int>(profile.seconds * rateHz
            * (1.15f - 0.25f * strength + 0.12f * draw(contact.random))));
        contact.spacing = static_cast<int>(profile.spacing * rateHz
            * (1.25f - 0.42f * strength + 0.12f * draw(contact.random)));
        contact.count = profile.count;
        contact.end = contact.span + (contact.count - 1) * contact.spacing;
        contact.force = level * profile.force;
        contact.pickup = level * profile.pickup;
        contact.air = level * profile.air;
        contact.arm = profile.arm + 0.014f * draw(contact.random);
        contact.impact = profile.impact;
        const float frequency = std::min(0.17f * rateHz, profile.colourHz
            * (0.65f + 0.50f * strength + 0.12f * draw(contact.random)));
        contact.lowPole = pole(0.48f * frequency, rate);
        contact.highPole = pole(2.0f * frequency, rate);
        // Normalize the impulse energy of the two-pole band so the contact's
        // noise RMS is independent of sample rate.
        const double a = contact.lowPole, b = contact.highPole;
        const double energy = a / (2.0 - a) + b / (2.0 - b)
            - 2.0 * a * b / (a + b - a * b);
        contact.noiseGain = static_cast<float>(std::sqrt(3.0 / energy));
        return true;
    }

    [[nodiscard]] Drive render() noexcept
    {
        Drive drive {};
        for (auto& contact : contacts_)
        {
            if (contact.remaining() == 0)
                continue;
            const int age = contact.age++;
            float envelope = 0.0f;
            for (int i = 0; i < contact.count; ++i)
                envelope += pulse(age - i * contact.spacing, contact.span);
            envelope /= static_cast<float>(contact.count);
            const float white = draw(contact.random);
            contact.low += contact.lowPole * (white - contact.low);
            contact.high += contact.highPole * (white - contact.high);
            const float friction = envelope * contact.noiseGain * (contact.high - contact.low);
            const float force = contact.impact * envelope + (1.0f - 0.8f * contact.impact) * friction;
            const float heave = contact.force * force;
            drive.force += heave;
            drive.moment += heave * contact.arm;
            drive.pickup += contact.pickup * force;
            drive.air += contact.air * friction;
            drive.side += contact.air * (4.0f * contact.arm) * friction;
            if (contact.remaining() == 0)
                --active_;
        }
        return drive;
    }

private:
    struct Profile
    {
        float seconds, spacing, colourHz, force, pickup, air, arm, impact;
        int count;
    };
    // Authored envelopes and effective forces. The brush crosses four
    // strings, the plain-string shift uses a soft ~1 kHz friction band,
    // and the heel landing on four strings has softer contacts than a tap
    // to the top. Their levels are set below a Concert Strum at the same
    // velocity. None of these gestures pluck a pitched string.
    static constexpr std::array<Profile, 4> profiles {{
        { 0.009f, 0.004f, 1850.0f, 0.00140f, 0.00080f, 0.00095f,  0.031f, 0.12f, 4 },
        { 0.075f, 0.000f, 1100.0f, 0.00016f, 0.00013f, 0.00028f, -0.046f, 0.00f, 1 },
        { 0.005f, 0.000f, 1900.0f, 0.00200f, 0.00064f, 0.00029f,  0.024f, 0.82f, 1 },
        { 0.011f, 0.002f,  720.0f, 0.00070f, 0.00050f, 0.00022f,  0.029f, 0.58f, 4 }
    }};
    struct Contact
    {
        std::uint32_t random {};
        int channel {}, age {}, end {}, span {}, spacing {}, count {};
        float force {}, pickup {}, air {}, arm {}, impact {};
        float lowPole {}, highPole {}, low {}, high {}, noiseGain {};
        [[nodiscard]] int remaining() const noexcept { return end - age; }
    };

    static float pole(float hz, double rate) noexcept
    {
        return static_cast<float>(1.0 - std::exp(-6.283185307179586 * hz / rate));
    }

    static float pulse(int age, int samples) noexcept
    {
        if (age < 0 || age >= samples)
            return 0.0f;
        const float position = static_cast<float>(age) / static_cast<float>(samples - 1);
        const float shape = 4.0f * position * (1.0f - position);
        return shape * shape;
    }

    static float draw(std::uint32_t& state) noexcept
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return static_cast<float>(state >> 8) / 8388608.0f - 1.0f;
    }

    static constexpr std::uint32_t initialSeed = 0xf36bca27u;
    std::uint32_t seed_ { initialSeed };
    std::array<Contact, 8> contacts_ {};
    int active_ {};
};
} // namespace aculele
