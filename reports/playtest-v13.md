# Playtest follow-up — 0.13.0

Three reported issues were addressed: young Galecrest appearing motionless and ignoring temperament changes, generic companion terminology, and movement sharing ability energy.

## Changes

- Ordinary travel no longer spends ability energy, suppresses its recovery, or changes speed based on remaining energy. Forward/strafe/reverse ratios, finite turning and cornering, braking, terrain and cast commitments constrain movement. Lateral/reverse drive is now 70% of each species' authored ratio, smoothly returning to full forward drive.
- Universal dodge costs no ability energy and adds no regeneration lock; its independent 48-tick cooldown remains. Existing art-cast recovery delays continue normally. Authored movement arts keep their individual costs.
- The beginner helper now receives the party's selected temperament. Aggressive ranged kami press closer between casts, skittish retreats when crowded, patient keeps a longer firing lane, and territorial returns toward the lotus while its target remains reachable. Explicit keeper calls override these habits. Planted casts still require standing still; aimless constant motion is not the objective.
- The party page distinguishes guided early habits from the learned pilot used at bond 3+. “COMMITTED” was a casting-state label, not a nature; it now reads “CAST IN PROGRESS.” Player-facing companion terminology is replaced by Tinikami/kami while serialized/internal field names remain compatible.

## Evidence and limits

- 661,144 simulation checks; golden `998dc4f56283125f`. Tests cover all 40 species moving at empty/full energy, equal velocity, free regeneration, independent dodge availability/payment/cooldown, and finite cornering.
- 67,645 campaign checks and 73,192 environment checks passed.
- 22,106 brain checks passed. Native/Python inference parity: 480 decisions, maximum float difference 4.8e-6, zero quantized action differences.
- 120/120 opening routes passed autonomously and with keeper calls, 3,600/3,600 duel wins. The harness now passes the actual saved temperament into the beginner helper.
- All five full-route starters reached eight bells and forty kami, with retries. Losses increased relative to v12.1: Cinderfox 19, Brambleback 21, Galecrest 29, Rimehare 30, Dewotter 22. This is a real balance/controller regression under the corrected resource model, not evidence of improved learned play. These are fixed scripted playthroughs, not human difficulty estimates.
- 22 rendered campaign fixtures, all 1,280 sprite cells, native controller flow and workbench smoke checks passed. Relocated macOS bundle passed 80 controller checks and strict ad hoc signature verification.

Rules increment to 10, observations remain 8 (3,628 floats), content `8a2d462d`, native model checksum `795e5e98`. The existing learned tensors are unchanged and explicitly warm-started; they have not been retrained for the new movement/dodge rules. Previous release models remain in `models/baselines/rules9/release-v12.1/`. Campaign saves remain compatible; historical combat snapshots require their matching engine.

## Overworld decision

[Framework assessment and integration contract](../docs/alpha/overworld-direction.md) recommends evaluating an RPG Maker MZ battle-scene/Wasm integration for its ready-made RPG tooling, with Godot as the stronger native C++ alternative. No framework migration or purchase has been performed. The acceptance target is one polished village-route-battle-save loop before expanding the campaign further.
