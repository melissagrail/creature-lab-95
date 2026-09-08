#include "../client/journey_audio.hpp"
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
int main() {
    journey_audio::Sound soundtrack;
    soundtrack.battle.store(true);
    double energy = 0, peak = 0, jump = 0;
    float previous = 0;
    std::vector<float> buffer(1024);
    // Cover the full 16-bar arrangement twice, transitions, bell and mute.
    for (int block = 0; block < 6000; ++block) {
        if (block == 4800)
            soundtrack.battle.store(false);
        if (block == 5200)
            soundtrack.chime();
        if (block == 5600)
            soundtrack.enabled.store(false);
        journey_audio::Sound::callback(&soundtrack, reinterpret_cast<Uint8 *>(buffer.data()),
                                       int(buffer.size() * sizeof(float)));
        for (float sample : buffer) {
            if (!std::isfinite(sample) || std::abs(sample) >= .5 || (block >= 5600 && sample != 0))
                return 1;
            peak = std::max(peak, double(std::abs(sample)));
            if (block < 5600)
                jump = std::max(jump, double(std::abs(sample - previous)));
            previous = sample;
            energy += sample * sample;
        }
    }
    const double rms = std::sqrt(energy / (6000 * buffer.size()));
    if (rms < .005 || jump > .15)
        return 2;
    std::cout << "Audio: complete arrangement, transitions, chime and mute passed; peak " << peak
              << ", RMS " << rms << ", maximum adjacent change " << jump << '\n';
}
