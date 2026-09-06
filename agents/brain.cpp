#include "creature/brain.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
namespace creature {
namespace {
uint32_t word(const unsigned char *p) {
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}
uint32_t fingerprint(const unsigned char *p, size_t size) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < size; ++i)
        h = (h ^ p[i]) * 16777619u;
    return h;
}
struct Linear {
    const float *weight, *bias;
    int in, out;
    void run(const float *x, float *y, bool activate = false) const {
        for (int i = 0; i < out; ++i) {
            float sum = bias[i];
            for (int j = 0; j < in; ++j)
                sum += weight[i * in + j] * x[j];
            y[i] = activate ? std::tanh(sum) : sum;
        }
    }
};
float sigmoid(float x) {
    return 1.f / (1.f + std::exp(-std::clamp(x, -60.f, 60.f)));
}
bool finite(const float *p, size_t n, float bound = 1000) {
    for (size_t i = 0; i < n; ++i)
        if (!std::isfinite(p[i]) || std::abs(p[i]) > bound)
            return false;
    return true;
}
} // namespace
bool Brain::load(const std::string &path, std::string &error) {
    constexpr size_t size = 36 + BrainParameters * 4 + 4;
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f || f.tellg() != std::streampos(size)) {
        error = "Missing brain or incorrect file length";
        return false;
    }
    std::vector<unsigned char> bytes(size);
    f.seekg(0);
    f.read(reinterpret_cast<char *>(bytes.data()), size);
    const auto *b = bytes.data();
    if (!f || std::memcmp(b, "TINIBRN1", 8) || word(b + 8) != BrainFormat ||
        word(b + 12) != RulesVersion || word(b + 16) != ObservationVersion ||
        word(b + 20) != ContentHash || word(b + 24) != ObservationSize ||
        word(b + 28) != BrainHidden || word(b + 32) != BrainParameters) {
        error = "Incompatible brain format, rules, observations or content";
        return false;
    }
    auto checksum = fingerprint(b, size - 4);
    if (checksum != word(b + size - 4)) {
        error = "Brain checksum mismatch";
        return false;
    }
    std::vector<float> candidate(BrainParameters);
    static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
                  "Brain requires IEEE float32");
    for (int i = 0; i < BrainParameters; ++i) {
        uint32_t bits = word(b + 36 + i * 4);
        std::memcpy(&candidate[i], &bits, 4);
    }
    if (!finite(candidate.data(), candidate.size())) {
        error = "Invalid or excessive brain weights";
        return false;
    }
    weights = std::move(candidate);
    checksum_ = checksum;
    error.clear();
    return true;
}
bool Brain::forward(const Observation &o, const BrainMemory &memory, BrainOutput &out) const {
    if (!ready() || !finite(memory.data(), memory.size(), 1.01f) ||
        !finite(o.self.data(), o.self.size()) || !finite(o.entities.data(), o.entities.size()) ||
        !finite(o.moves.data(), o.moves.size()) ||
        !finite(o.announced.data(), o.announced.size()) ||
        !finite(o.history.data(), o.history.size()) || !finite(o.global.data(), o.global.size()) ||
        !finite(o.mask.data(), o.mask.size()) || o.mask[0] < .5f)
        return false;
    const float *p = weights.data() + 4; // log_std is first in the PyTorch parameter order;
                                         // deterministic runtime does not sample.
    auto layer = [&](int in, int output) {
        const float *w = p;
        p += in * output;
        const float *b = p;
        p += output;
        return Linear{w, b, in, output};
    };
    auto entity = layer(43, 32), move = layer(48, 24), event = layer(7, 24),
         encoder = layer(245, 96);
    const float *wi = p;
    p += 288 * 96;
    const float *wh = p;
    p += 288 * 96;
    const float *bi = p;
    p += 288;
    const float *bh = p;
    p += 288;
    auto motion = layer(96, 4), noop = layer(96, 1), score1 = layer(120, 32), score2 = layer(32, 1),
         value = layer(96, 1);
    if (p != weights.data() + BrainParameters)
        return false;
    std::array<float, 245> input{};
    int cursor = 0;
    auto append = [&](const float *a, int n) {
        std::copy(a, a + n, input.begin() + cursor);
        cursor += n;
    };
    append(o.self.data(), 66);
    append(o.entities.data(), 43);
    float pooled[32]{}, tmp[32]{}, count = 0;
    for (int i = 0; i < EntityCount; ++i) {
        const auto *row = o.entities.data() + i * EntitySize;
        float mask = row[43];
        if (mask == 0)
            continue;
        entity.run(row, tmp, true);
        for (int j = 0; j < 32; ++j)
            pooled[j] += tmp[j] * mask;
        count += mask;
    }
    for (auto &v : pooled)
        v /= std::max(1.f, count);
    append(pooled, 32);
    float moves[5][24]{}, mean_move[24]{};
    for (int i = 0; i < 5; ++i) {
        move.run(o.moves.data() + i * 48, moves[i], true);
        for (int j = 0; j < 24; ++j)
            mean_move[j] += moves[i][j] / 5.f;
    }
    append(mean_move, 24);
    float history[24]{};
    count = 0;
    for (int i = 0; i < EventCount; ++i) {
        const auto *row = o.history.data() + i * EventSize;
        float mask = row[7];
        if (mask == 0)
            continue;
        event.run(row, tmp, true);
        for (int j = 0; j < 24; ++j)
            history[j] += tmp[j] * mask;
        count += mask;
    }
    for (auto &v : history)
        v /= std::max(1.f, count);
    append(history, 24);
    append(o.global.data(), 32);
    float announced[24];
    move.run(o.announced.data(), announced, true);
    append(announced, 24);
    float encoded[96], gi[288], gh[288];
    encoder.run(input.data(), encoded, true);
    Linear{wi, bi, 96, 288}.run(encoded, gi);
    Linear{wh, bh, 96, 288}.run(memory.data(), gh);
    BrainOutput result;
    for (int i = 0; i < 96; ++i) {
        float r = sigmoid(gi[i] + gh[i]), z = sigmoid(gi[96 + i] + gh[96 + i]);
        float n = std::tanh(gi[192 + i] + r * gh[192 + i]);
        result.memory[i] = (1 - z) * n + z * memory[i];
    }
    motion.run(result.memory.data(), result.mean.data());
    noop.run(result.memory.data(), result.logits.data());
    float slot[120];
    std::copy(result.memory.begin(), result.memory.end(), slot);
    for (int i = 0; i < 5; ++i) {
        std::copy(moves[i], moves[i] + 24, slot + 96);
        score1.run(slot, tmp, true);
        score2.run(tmp, &result.logits[i + 1]);
    }
    for (int i = 0; i < 6; ++i)
        if (o.mask[i] < .5f)
            result.logits[i] = -1e9f;
    value.run(result.memory.data(), &result.value);
    out = result;
    return true;
}
Action Brain::decode(const Observation &o, const BrainOutput &result) {
    int32_t q[4];
    float fx = o.self[4], fy = o.self[5];
    float x = o.entities[1] * fx - o.entities[2] * fy, y = o.entities[1] * fy + o.entities[2] * fx;
    float length = std::sqrt(x * x + y * y);
    if (length > 1e-8f) {
        fx = x / length;
        fy = y / length;
    }
    for (int i = 0; i < 2; ++i) {
        float forward = std::tanh(result.mean[i * 2]), right = std::tanh(result.mean[i * 2 + 1]);
        float v[2] = {forward * fx - right * fy, forward * fy + right * fx};
        for (int j = 0; j < 2; ++j) {
            float s = std::clamp(v[j], -1.f, 1.f) * 1024.f;
            q[i * 2 + j] = int32_t(std::copysign(std::floor(std::abs(s) + .5f), s));
        }
    }
    int ability =
        int(std::max_element(result.logits.begin(), result.logits.end()) - result.logits.begin());
    return {q[0], q[1], q[2], q[3], ability};
}
Action Brain::action(const Observation &o, BrainMemory &memory) const {
    BrainOutput result;
    if (!forward(o, memory, result))
        return {};
    memory = result.memory;
    return decode(o, result);
}
} // namespace creature
namespace {
creature::Observation unpack(const float *p) {
    creature::Observation o;
    auto take = [&](auto &a) {
        std::copy(p, p + a.size(), a.begin());
        p += a.size();
    };
    take(o.self);
    take(o.entities);
    take(o.moves);
    take(o.announced);
    take(o.history);
    take(o.global);
    take(o.mask);
    return o;
}
} // namespace
extern "C" {
void *tb_create(const char *path) {
    if (!path)
        return nullptr;
    try {
        auto b = std::make_unique<creature::Brain>();
        std::string error;
        if (b->load(path, error))
            return b.release();
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}
void tb_destroy(void *brain) {
    delete static_cast<creature::Brain *>(brain);
}
int32_t tb_forward(void *brain, const float *observation, const float *memory, float *output) {
    if (!brain || !observation || !memory || !output)
        return -1;
    creature::BrainMemory h;
    std::copy(memory, memory + h.size(), h.begin());
    creature::BrainOutput result;
    if (!static_cast<creature::Brain *>(brain)->forward(unpack(observation), h, result))
        return -1;
    std::copy(result.mean.begin(), result.mean.end(), output);
    std::copy(result.logits.begin(), result.logits.end(), output + 4);
    output[10] = result.value;
    std::copy(result.memory.begin(), result.memory.end(), output + 11);
    return 0;
}
int32_t tb_action(void *brain, const float *observation, float *memory, int32_t *action) {
    if (!brain || !observation || !memory || !action)
        return -1;
    auto o = unpack(observation);
    creature::BrainMemory h;
    std::copy(memory, memory + h.size(), h.begin());
    creature::BrainOutput result;
    if (!static_cast<creature::Brain *>(brain)->forward(o, h, result))
        return -1;
    auto a = creature::Brain::decode(o, result);
    int32_t values[] = {a.mx, a.my, a.ax, a.ay, a.ability};
    std::copy(values, values + 5, action);
    std::copy(result.memory.begin(), result.memory.end(), memory);
    return 0;
}
}
