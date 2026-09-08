#pragma once
#include <SDL.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
namespace journey_audio {
struct Sound {
    SDL_AudioDeviceID device = 0;
    std::atomic<int> region{0}, pulse{0};
    std::atomic<bool> enabled{true}, battle{false};
    uint64_t sample = 0;
    double bell_age = 100;
    double combat_mix = 0;
    uint64_t battle_sample = 0;
    uint32_t noise = 0x74696e69;
    bool was_combat = false;
    static double tone(double time, double midi) {
        const double phase = time * 440. * std::pow(2., (midi - 69.) / 12.) * 6.283185307;
        return std::sin(phase) + .25 * std::sin(3 * phase) + .10 * std::sin(5 * phase);
    }
    // Original 16-bar chip arrangement: four-bar call/response, rising second
    // phrase, half-time bridge, and a drum fill returning to the opening hook.
    double battle_music() {
        constexpr double eighth = 60. / 144. / 2.;
        const double t = double(battle_sample++) / 44100.;
        const int step = int(t / eighth), bar = (step / 8) % 16, slot = step % 8;
        const double age = std::fmod(t, eighth);
        static constexpr int roots[16] = {45, 45, 41, 43, 45, 48, 41, 43,
                                          38, 41, 45, 43, 41, 43, 44, 45};
        static constexpr int hook[4][8] = {{0, 7, 12, -99, 10, 7, 3, 7},
                                           {12, 10, 7, 3, 5, 7, 10, -99},
                                           {0, 3, 7, 12, 15, 12, 7, 3},
                                           {10, 7, 5, 3, 2, 5, 7, 11}};
        const int root = roots[bar], note = hook[bar % 4][slot];
        const double gate = std::min(1., age / .006) * std::min(1., (eighth - age) / .022);
        const double lead = note == -99 ? 0
                                        : tone(t, root + 24 + note) * gate * std::exp(-age * 5) *
                                              (bar >= 8 && bar < 12 ? .017 : .027);
        const double bass =
            tone(t, root + (slot % 4 == 3 ? 12 : 0)) * gate * std::exp(-age * 9) * .038;
        const double arp = tone(t, root + 12 +
                                       (slot % 3 == 0   ? 0
                                        : slot % 3 == 1 ? 7
                                                        : 12)) *
                           gate * std::exp(-age * 16) * .011;
        noise ^= noise << 13;
        noise ^= noise >> 17;
        noise ^= noise << 5;
        const double hiss = double(noise & 65535) / 32767.5 - 1.;
        const bool kick = slot == 0 || slot == 4 || (bar % 4 == 3 && slot == 7);
        const bool snare = slot == 2 || slot == 6 || (bar % 4 == 3 && slot >= 5);
        const double drum =
            (kick ? std::sin(6.283185307 * (48 * age + 7 * (1 - std::exp(-age * 25)))) *
                        std::exp(-age * 24) * .065
                  : 0) +
            (snare ? (hiss * .75 + std::sin(age * 1130) * .25) * std::exp(-age * 32) * .043 : 0) +
            hiss * std::exp(-age * (slot % 2 ? 85 : 160)) * .009;
        return lead + bass + arp + drum;
    }
    static void callback(void *userdata, Uint8 *stream, int bytes) {
        auto &s = *static_cast<Sound *>(userdata);
        auto *out = reinterpret_cast<float *>(stream);
        if (s.pulse.exchange(0))
            s.bell_age = 0;
        const bool audible = s.enabled.load();
        const int area = s.region.load();
        const bool combat = s.battle.load();
        if (combat && !s.was_combat)
            s.battle_sample = 0;
        s.was_combat = combat;
        static constexpr int notes[16] = {0, 7, 12, 4, 9, 7, 4, 2, 0, 4, 7, 14, 12, 9, 7, 2};
        for (int i = 0; i < bytes / int(sizeof(float)); i++) {
            double t = double(s.sample++) / 44100.;
            double beat = .76;
            int n = int(t / beat);
            double age = std::fmod(t, beat),
                   root = 130.8128 * std::pow(2., double((area * 2) % 7) / 12.);
            double f = root * std::pow(2., notes[n % 16] / 12.);
            double env = std::min(1., age / .012) * std::exp(-age * 5.5);
            double melody =
                (std::sin(t * f * 6.2831853) + .2 * std::sin(t * f * 12.56637)) * env * .028;
            double chord =
                (std::sin(t * root * .5 * 6.2831853) + .35 * std::sin(t * root * .75 * 6.2831853)) *
                .012;
            double bell =
                std::sin(s.bell_age * 1046.5 * 6.2831853) * std::exp(-s.bell_age * 7) * .055;
            s.bell_age += 1. / 44100.;
            // Smooth transitions and mute without stopping the audio clock.
            s.combat_mix += ((combat ? 1. : 0.) - s.combat_mix) / 6000.;
            const double combat_track = s.battle_music();
            out[i] = audible ? float((melody + chord) * (1 - s.combat_mix) +
                                     combat_track * s.combat_mix + bell)
                             : 0.f;
        }
    }
    void start() {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
            return;
        SDL_AudioSpec want{};
        want.freq = 44100;
        want.format = AUDIO_F32SYS;
        want.channels = 1;
        want.samples = 1024;
        want.callback = callback;
        want.userdata = this;
        device = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
        if (device)
            SDL_PauseAudioDevice(device, 0);
    }
    void chime() {
        pulse.store(1);
    }
    ~Sound() {
        if (device)
            SDL_CloseAudioDevice(device);
    }
};
} // namespace journey_audio
