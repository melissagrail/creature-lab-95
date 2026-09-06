> **Historical v2 verification.** These balance and performance figures describe release 0.2, before directional locomotion and planted casts. See [v6 validation](alpha/tinikami.md#validation) for the current build.

# Combat alpha verification

Tested September 5, 2026. Rules/observations v2; content fingerprint `7def5f00`.

## Mechanical evidence

- All **160 signature moves plus shared dodge** pass content/schema validation. Every species/slot is checked for request acceptance, stamina/cooldown accounting, telegraph/release, semantic observation data and snapshot restoration.
- Every damaging signature move connects in at least one of eight controlled range probes. This found and corrected an even-pellet spread bug; fan direction has a regression test.
- All **40 passives** have a controlled-state contract assertion. Additional tests cover minimum range, evasion, directional guard, shield breaking, CC resistance, silence/root readiness, cleanse, destructible turrets/traps, arming, ricochet, return phase, objective contest/capture, time verdict, corrupted snapshots, replay tampering and INT_MIN/INT_MAX actions.
- Every species runs a deterministic scripted episode/fork and a random-action soak. Snapshots are periodically restored and compared. Explicit full-pool allocation verifies overflow reporting.
- The final native suite reports **369,233 assertions**. Most are repeated numerical/state checks, not 369,233 independent gameplay designs.
- Optimized ARM64, AddressSanitizer + UndefinedBehaviorSanitizer, and x86-64/Rosetta pass the golden fixture `eed14e993de79fb7`: seed 77, rain, Gravemole vs Kelpwidow, grove, 100 scripted decisions.
- Python ctypes tests verify shape, registry, batch/scalar parity, independent arenas, forks, validation, close lifecycle and the same golden hash.
- PyTorch 2.14.0: the 79,555-parameter recurrent model passes inference, masks, canonical action submission and gradient checks. Gymnasium's checker plus hybrid-action smoke tests cover all 40 species.

## Balance evidence

The final calibration and fresh-seed holdout each contain **56,160 matches**: all 780 unordered pairs × both seats × 3 weather conditions × 3 arenas × 4 scripted style pairings. That is **112,320 matches on final content**, 5,616 games per species across the two sets.

| Metric | Calibration | Fresh-seed holdout |
|---|---:|---:|
| Species aggregate win-rate range | 40.1%–58.4% | 39.9%–58.6% |
| Side A win rate | 51.09% | 50.83% |
| Mean match duration | 25.11 seconds | 25.14 seconds |
| Pool overflow | 0 | 0 |

Reports contain raw rows, full matrices, source/content metadata and worst pairings. Early diagnostic sweeps and logged numeric interventions are retained. The early sweeps used evolving pilots/geometry, so do not interpret their differences as a controlled estimate of any single tuning change.

These are **scripted baseline measurements**. They do not establish trained-agent balance, human fun, skill ceilings or fairness in each matchup. In particular, Miretoad vs Quillrat was 0–72 in the holdout, despite a roughly balanced roster-wide aggregate. [The counterplay follow-up](alpha/counterplay.md) probes extreme pairings with the full cross-product of four available styles. Remaining extremes are release notes, not hidden by the mean.

## Performance and presentation

Local native benchmark: approximately **1.13 million physics ticks/s**, or **378,000 joint decisions/s**, across 256 worlds on one CPU thread, including scripted policies. It cycles the roster and excludes neural inference, gradient updates and archival snapshot hashing. World: **4,656 bytes**; portable snapshot: **4,676 bytes**. These are local samples, not guarantees for other hardware.

The SDL2 2.32.6 viewer and local macOS bundle build and launch. Retina-resolution arena and roster renders were visually inspected. The viewer has both-side species selection, three layouts, all move forms, public state meters, objective pressure, snapshots and replay. Full native mouse/keyboard automation is not claimed: the desktop accessibility bridge could not attach to the SDL app during the initial prototype task.

CI checks the core/golden fixture on Linux, macOS and Windows, plus Linux viewer, ctypes, content validation, sanitizer and tournament smoke. No completed RL training run is included.

```sh
make all test
make build/tournament
./build/tournament 36 reports/matches.csv 3000
python3 scripts/analyze_balance.py reports/matches.csv reports/balance
c++ -std=c++17 -O1 -g -fsanitize=address,undefined -Iinclude \
  src/sim.cpp src/content.cpp tests/sim_tests.cpp -o build/sanitized
./build/sanitized
```
