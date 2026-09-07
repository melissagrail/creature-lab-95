#pragma once
#include <atomic>
namespace journey_audio {
struct Sound {
    SDL_AudioDeviceID device = 0;
    std::atomic<int> region{0}, pulse{0};
    std::atomic<bool> enabled{true}, battle{false};
    uint64_t sample = 0;
    double bell_age = 100;
    static void callback(void *userdata, Uint8 *stream, int bytes) {
        auto &s = *static_cast<Sound *>(userdata);
        auto *out = reinterpret_cast<float *>(stream);
        if (s.pulse.exchange(0))
            s.bell_age = 0;
        const bool audible = s.enabled.load();
        const int area = s.region.load();
        const bool combat = s.battle.load();
        static constexpr int notes[16] = {0, 7, 12, 4, 9, 7, 4, 2, 0, 4, 7, 14, 12, 9, 7, 2};
        for (int i = 0; i < bytes / int(sizeof(float)); i++) {
            double t = double(s.sample++) / 44100.;
            double beat = combat ? .42 : .76;
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
            out[i] = audible ? float(melody + chord + bell) : 0.f;
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
