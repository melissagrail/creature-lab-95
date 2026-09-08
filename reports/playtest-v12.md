# v0.12 opening playtest revision

This revision addresses the first in-game feedback pass: open exploration, full-health teaching partners, keeper calls, distinct starters, early garden layouts, edge visibility, walking animation, and a gentler first keeper. Original private notes and combat snapshots are not included here.

## Implemented behavior

- Hearthmere's teachers, habitats, conversations and optional discoveries are open. The journal suggests a route. The keeper and onward road retain their story prerequisites.
- Six completed challenges introduce the starter before wild friendship begins. Wins and completed losses count; retreat grants neither introduction progress nor trust. Previously acquired companions stay acquired.
- Full-health Dewotter and Quillrat teaching partners replace the 45%/60% health starts. Four sets of 4/4/3/3 lessons use cover, angled approaches, a charged burst, muddy flanks, wet corners and the lotus. Teachers yield the lotus.
- The first keeper is one full-health Quillrat with two arts. Later keepers retain three opponents.
- New starter selection: Cinderfox, Brambleback, Galecrest. Existing Rimehare and Dewotter starter saves remain supported.
- F attacks, G falls back, C rests and V returns to autonomy. Calls last 90 ticks, respect committed casts and issue ordinary legal actions. A slow burst demonstrates the value of retreating before impact and returning during recovery.
- Quillrat's Panic Quills is globally tuned to 60 startup / 2 active / 54 recovery ticks, 180 cooldown ticks and 3,200 radius. Damage and cost remain 16 and 220 internal energy.
- Campaign presentation runs at 0.75 speed for every bond: 90 simulation seconds allow about two real minutes, excluding pauses. The core remains fixed at 30 Hz.
- The combat field has an inset boundary and outer visual space. Sprite scale differences are preserved. All 40 species fit at all four edges, including the health bar. Manual mouse aim uses the same transform.
- New authored wayfarer gait: four orientations, alternating foot contacts through a passing pose, registered by the head, advanced by actual distance walked. The fourth source column is retained but excluded because of its torso twist. [Art and provenance](../assets/tinikami/wayfarer-prompt.json), [technical importer](../scripts/compile_wayfarer.py).

## Controller limitation discovered by testing

The released RL weights were trained on full kits. In the early full-health diagnostic build, the unassisted learned controller completed only 43 of 120 sampled starter routes; Brambleback sometimes never attacked. The raw diagnostic is retained in `apprenticeship-v12-unassisted-diagnostic.csv`. Reducing enemy health had concealed this weakness.

Bond 1–2 now use an explicitly labeled beginner motor controller: approach around cover, face, plant and choose an affordable offensive art. It does not automatically escape the charged burst. Early temperaments share this scaffold. At bond 3+ the learned controller returns. Keeper calls can temporarily direct either controller. This is deterministic assistance, **not new RL training or learned command compliance**.

The model tensors are unchanged from v0.11. An explicit content migration updates their compatibility header after move tuning; v0.11 checkpoint and manifest are preserved. Rules 9, observations 8, content `55a82aa5`, model checksum `187a9a2f`, simulation golden `4deb7e7e3423a32b`.

## Reproduction and results

```
make test viewer build/apprenticeship_playthrough build/campaign_playthrough
build/apprenticeship_playthrough models/apprentice.tbrain reports/apprenticeship-v12.csv
build/apprenticeship_playthrough models/apprentice.tbrain reports/apprenticeship-v12-calls.csv calls
build/campaign_playthrough models/apprentice.tbrain reports/campaign-playthrough-v12.csv
python3 tests/journey_smoke.py
python3 tests/viewer_smoke.py
.venv/bin/python tests/learning_tests.py models/apprentice.pt
```

Five starters × 24 seeds, each playing fourteen lessons and the keeper solo, with one legal remedy per round:

| Controller condition | Routes completed | Duel wins | Mean simulation seconds |
| --- | ---: | ---: | ---: |
| Beginner autonomy | 120 / 120 | 1,800 / 1,800 | 17.0076 |
| Same controller plus timed keeper calls | 120 / 120 | 1,800 / 1,800 | 18.6880 |

Across the 960 burst/keeper encounters in each cohort, mean damage taken falls from 18.8 to 7.6 with calls. Calls respond after nine startup ticks and attack during recovery; they are a reproducible test policy, not measured human reaction data. These seeded smoke cases contain deterministic duplicates because the beginner controller has no personality variation or randomness; they are not 120 independent balance observations.

The full-route harness completes eight bells and all forty companions from each starter. It uses preparation, legal remedies, repeat encounters and party changes. Recorded encounters/losses: Cinderfox 97/7, Brambleback 103/14, Galecrest 101/7, Rimehare 96/2, Dewotter 103/9. Optional Lantern Walks were not rerun in this revision. These AI-route times do not establish a twenty-hour human campaign.

Validation includes campaign/save checks, fixed-tick golden, environment checks, native model checks, native/Python parity over 480 decisions and all 40 species (maximum float error 4.8e-6; zero quantized-action delta), legacy-model parity, 22 rendered campaign fixtures, 80 native UI/controller checks, and animation padding/frame checks. The UI tests include paused calls, expiry, notebook return, and four-corner sprite bounds. The campaign tests also pass AddressSanitizer/UndefinedBehaviorSanitizer. A private copy of the existing v0.11 journey save validates and round-trips byte-for-byte; its original file was not modified.

## Remaining design work

Restricted-kit and command-conditioned RL training is still needed before removing beginner assistance or expressing early learned temperaments. Field matchups can still favor particular passives; first-contact recognition after a completed loss offers a recovery route. The current final-region balance, optional expedition tuning, and twenty-hour content target remain wider alpha work.
