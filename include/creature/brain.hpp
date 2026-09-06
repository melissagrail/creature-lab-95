#pragma once
#include "brain_api.h"
#include "sim.hpp"
#include <array>
#include <string>
#include <vector>
namespace creature {
constexpr int BrainFormat = 2, BrainHidden = 96, BrainParameters = 86755;
using BrainMemory = std::array<float, BrainHidden>;
struct BrainOutput {
    std::array<float, 4> mean{};
    std::array<float, 6> logits{};
    float value = 0;
    BrainMemory memory{};
};
// Optional floating-point controller. It never owns or mutates a World.
// Keep one memory per actor, and persist it separately when forking the controller.
class Brain {
    std::vector<float> weights;
    uint32_t checksum_ = 0;

  public:
    bool load(const std::string &path, std::string &error);
    bool ready() const {
        return weights.size() == BrainParameters;
    }
    uint32_t checksum() const {
        return checksum_;
    }
    bool forward(const Observation &, const BrainMemory &, BrainOutput &) const;
    Action action(const Observation &, BrainMemory &) const;
    static Action decode(const Observation &, const BrainOutput &);
};
} // namespace creature
