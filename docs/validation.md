# Verification

Local checks performed on September 5, 2026, using Apple Clang 15 on an ARM64 Mac:

- Native regression suite: **3,147 assertions** including golden fixture (the executable reports 3,146 before the final golden assertion).
- ARM64 optimized, ARM64 AddressSanitizer + UndefinedBehaviorSanitizer, and x86-64 optimized builds all pass the same golden hash: `b92b46e8c4a2443c` (seed 77, rain, 100 joint scripted decisions).
- Checked every snapshot during multiple seeded full episodes, including expired object slots; tested corrupted and truncated snapshot rejection without mutation.
- Checked deterministic forks, readiness, cast timing, one-hit melee, simultaneous damage and KOs, evasive contact, evasion-piercing zones, rock shielding, energy, cooldown, rain, guidance, private enemy resources, extreme input clamping and timeout semantics.
- Action replay roundtrip includes trainer guidance and praise; tampered expected hashes are rejected.
- Python ctypes smoke tests: scalar/batch parity, independent arenas, snapshot forks, malformed input, terminal schema, idempotent close and use-after-close rejection.
- PyTorch 2.14.0 reference policy: 68,903 parameters; tensor shapes, inference, legal-action masks, float-to-integer action submission, finite log probabilities and backward pass.
- SDL2 2.32.6 viewer compiled and launched; the rendered window was visually inspected at Retina output resolution. The packaged executable also launched successfully. Native accessibility automation could not attach, so full mouse/keyboard end-to-end automation is not claimed.

Measured Python collection path: 158,610 joint decisions/s over 256 arenas and 1,000 decisions, including ctypes calls and scripted action collection (1,153 episodes completed).

Initial benchmark (optimized C++, 256 arenas, serial, including scripted policies): approximately **1.8 million physics ticks/s**, or **600,000 joint decisions/s**. World size 1,932 bytes; portable snapshot 1,948 bytes. This is a local sample, not an SLA. Model inference, learner updates, Python trajectory retention, snapshot hashing and IPC are excluded from this native benchmark. Run `python/rollout.py` for the Python collection path.

```sh
make test
./build/benchmark
python3 python/rollout.py --arenas 256 --decisions 1000
c++ -std=c++17 -O1 -g -fsanitize=address,undefined -Iinclude \
  src/sim.cpp tests/sim_tests.cpp -o build/sanitized
./build/sanitized
```

GitHub CI passed CMake core tests on Linux, macOS and Windows, plus Linux viewer/FFI/sanitizer checks. All core platforms matched the golden replay fixture. The additional local x86-64 comparison uses Rosetta. There is no completed RL training run or measured learning curve in this repository.
