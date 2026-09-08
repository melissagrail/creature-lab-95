# Development and notebook verification — v0.11

Engine rules 9, observation schema 8, content `223a8716`, model `a6a76794`. This is an explicit eight-input expansion of the v0.10 weights, not curriculum-trained replacement weights. Full-development physics and the previous weight tensors are retained; the snapshot envelope includes the new fields and has golden `8f436a06e647a94c`.

## Introductory encounters

`apprenticeship-v11.csv` contains 72 starter/personality-seed runs: three starters × 24 individual seeds, the full fourteen lessons, no party swaps or temperament changes. The harness uses the actual learned pilot, the actual lesson opponent controller, legal one-per-round remedies and the same campaign scenario/resolution functions as the client.

**All 72 finish all fourteen lessons: 1,008 wins in 1,008 duels; no locked ability requests and no blocked runs.** Mean simulation duration is 11.20 seconds; the early 80% presentation clock makes that roughly 14 wall-clock combat seconds, before reading, travel and result screens. This is intentionally an easy introduction, not a strong-opponent win-rate benchmark.

| Teacher | Duels | Wins | Mean simulation seconds | Mean accepted casts |
| --- | ---: | ---: | ---: | ---: |
| First Breathing Space | 288 | 288 | 10.25 | 4.82 |
| A Place to Stand | 288 | 288 | 12.55 | 5.54 |
| Between Thorn and Flame | 216 | 216 | 12.17 | 6.43 |
| A Promise Kept | 216 | 216 | 9.68 | 5.72 |

The first eight victories unlock the first new habitat. Unit tests separately verify that locked new species cannot be recruited earlier, lesson progress persists, retreat grants no XP, rank thresholds change the actual masks/caps, and later recruits arrive with regional experience.

A diagnostic variant that introduced Cinderfox's burn combo as the final lesson is retained in `apprenticeship-v11-diagnostic.csv`. It exposed poor restricted-kit responses, especially from Rimehare. The shipped teaching sequence uses Dewotter/Rimehare and reserves Cinderfox opponents for ordinary encounters. The shipped model still needs development-curriculum training; these lesson results do not establish balanced power across every species at every bond.

## Complete campaign routes

`campaign-playthrough-v11.csv` records real engine outcomes through all eight regions, using legal rests, remedies, party/temperament adjustments and thread offerings. Every starter restored eight bells and collected forty species.

| Starter | Encounters | Losses | Simulated combat minutes |
| --- | ---: | ---: | ---: |
| Cinderfox | 89 | 1 | 33.91 |
| Rimehare | 102 | 13 | 128.93 |
| Dewotter | 99 | 11 | 101.47 |

These different routes and controller results are not human playtime measurements or equal-power certification. Story travel, reading and optional exploration are outside those combat totals. The 20-hour human campaign remains an unverified design target.

## Contracts and UI

- 42,368 campaign assertions include all 160 sites, all species, both endings, progression limits, snapshot rejection, charm energy ratios, migration, corruption and backups.
- 657,296 simulation checks and 74,642 environment checks pass; C++ and Python agree on the golden and batch/scalar stepping.
- 22,052 native brain checks cover 3,591 decisions across all forty species.
- Native/Python parity covers 480 decisions across forty species, including restricted own/opponent profiles: maximum observed float difference 4.8e-6, quantized action difference zero. Legacy format-2 parity also passes.
- A one-update PPO smoke run with `--development-rate 1` exercises the new curriculum and a migrated learned opponent. Its tiny scores are not used to select or advertise a controller.
- 20 native SDL scene fixtures and 36 controller checks cover notes, editing, browsing, retained drafts after failed writes, combat snapshots, battle return state, saves, progression and campaign navigation. The 1,280 animation-cell and existing workbench checks still pass.
- AddressSanitizer/UndefinedBehaviorSanitizer campaign tests pass. Native scene fixtures are inspected; this is not a substitute for human input/UX playtesting.

Reproduce the lesson run:

```sh
make build/apprenticeship_playthrough
./build/apprenticeship_playthrough models/apprentice.tbrain reports/local-apprenticeship.csv
```
