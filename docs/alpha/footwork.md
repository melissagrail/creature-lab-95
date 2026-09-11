# Current movement constraints — rules 10

Ability energy is now independent of ordinary movement. Walking, strafing and reversing neither spend it nor suppress its regeneration. The universal dodge costs zero energy, uses its own 48-tick cooldown, and does not introduce an energy recovery delay. An existing delay from casting an art still expires normally during movement/dodge. Authored movement arts such as lunges retain their art costs.

Forward speed, species-specific strafe/reverse ratios, turning, braking and finite lateral acceleration remain. Lateral and reverse drive receive a 70% multiplier, smoothly returning to full drive when facing forward. Empty and full ability-energy reserves produce identical travel. This is a geometric movement constraint, not another hidden resource bar. `footwork_load` is diagnostic only.

Rules 10 / observations 8, content `8a2d462d`, golden `998dc4f56283125f`. Model weights are explicitly warm-started, not retrained, via `scripts/migrate-movement-energy.py`. Energy no longer appears as a movement charge in `Features.spent`.

The following is retained historical evidence for the superseded shared-energy experiment; it does not describe the current game.

---

# Footwork and animated spirits — rules 8

The new constraint is on sustained combat movement. It keeps repositioning useful while attaching an opportunity cost to endlessly circling with your face aimed at an opponent.

## Movement contract

`footwork_load(body)` derives effort from actual velocity relative to facing and the species' authored lateral/reverse speed. The first 25% of lateral-speed effort is free. Higher effort progressively suppresses base regeneration, reaching complete suppression at 90% effort. Reverse effort is weighted at 85% before applying that ramp.

An idle, unrooted, unstunned body at load 70 or above spends one internal energy unit per tick: **three displayed energy per second**. Active cast commitments already have their own resource and regeneration rules. Below 25 displayed energy, lateral drive scales from 55% to 100% with reserve. Forward travel retains its normal speed and recovery. Paid dodges and lunges retain their authored bursts.

Travel also has a finite lateral acceleration budget at speed. Above one-third of base speed, sideways acceleration relative to current velocity is capped at `max(2, speed * (5 + turn_degrees) / 240)` internal units per tick. Braking and paid bursts remain available. Heavy, slow-turning spirits make wider curves; quick-turning spirits can bend that constraint further.

These rules use existing public velocity, facing, species and energy state. There is no hidden orbit counter and no new tensor shape. Rules increment to 8; observation schema remains 7. Models and replays from the prior rules are rejected unless a model is deliberately migrated for a warm start.

The renderer marks sustained effort with **STRAIN** and reports limited energy recovery. The campaign also exposes both control bars, so retreating around the boundary has a visible positional cost.

## Mechanical comparison

`tests/footwork_benchmark.cpp` compares the released rules-7 engine at commit `0276e418768f5dc5aaa00731d0b08ed07835c578` with rules 8. Forty species start at 60 energy in a twenty-second open-ground scenario. An orbiting probe continuously faces the opponent, circles and does not cast; a separate probe faces its travel direction.

| Probe | Previous mean end energy | Rules 8 mean end energy |
| --- | ---: | ---: |
| Face opponent and strafe | 100.00 | 21.16 |
| Face travel direction | 100.00 | 100.00 |
| Stand still | 99.86 | 99.86 |

This demonstrates a resource constraint, not universal tactical dominance. The noncasting orbit probe is intentionally narrow. Some matches end early, the opposing scripted controller is also affected by the new physics, and raw damage comparisons cannot isolate the movement change. Forward-facing travel remains viable; turning to attack then exposes the usual facing, commitment and lateral-resource tradeoffs.

Raw rows: [rules 7](../../reports/footwork-rules7.csv), [rules 8](../../reports/footwork-v10.csv). Tests cover free forward movement, limited lateral recovery, low-reserve drive, finite cornering, paid dodge exceptions, observation values and snapshot parity across all forty species.

## Animation assets

Each spirit has a retained source PNG and an imported 8×4 atlas in `assets/tinikami/animations`. Rows are south, east, north and west. Columns are idle, walk A, walk B, windup, attack, hit, fall and fainted. The forty sheets contain **1,280 authored frames**. Three source sheets were corrected after the audit found a missing windup column.

`scripts/compile_animations.py` imports whole alpha components rather than cutting arbitrary nominal grid lines through tails or feet. It removes source mattes, preserves detached effects, normalizes one scale per sheet, and uses a common foot pivot. It does not synthesize a directional pose by rotating the old single sprite. The raw runtime atlas has 96-pixel cells and a `(48,88)` pivot. Source artwork, original generation prompts and correction prompts are retained.

The world sprite-size mapping expands the old narrow visual range to 44–116 pixels across the current roster, driven by physical species radius. Animation follows engine phase and hit/knockout events; a separate presentation clock completes the fall/faint transition after the deterministic world stops. Neither art nor presentation clocks enter physics, snapshots, RNG or observations.

Environment additions include an illustrated world atlas, thirty-two general props and character frames, thirty-two regional landmark/vegetation props, a sanctuary interior and an application icon. Runtime assets require no PNG decoder or image-generation service.

## Learned controller

The rules-7 checkpoint was preserved before explicit migration. Continued recurrent PPO then trained against the updated movement rules, scripted styles and frozen learned opponents, including mixed temperaments. `reports/rl-v10-summary.md` records final selection, checksums, native evaluation and limitations. Warm starts are identified as warm starts; merely changing the compatibility manifest is not counted as training.

The canonical replay golden is `03b73ddd44073999`. Content remains `223a8716`, and observations remain 3,620 floats. `Features.spent` now includes footwork expenditure. Evaluation counts accepted `Started` events for casts, rather than mistaking every energy-spending movement decision for an ability.
