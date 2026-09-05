# Movement alpha: facing, travel and commitment

Rules/observations **v3**, content fingerprint `56c01336`. This iteration addresses the freely drifting circles: a creature now has a physical orientation, a travel budget relative to that orientation, and a per-move permission to move. It changes gameplay, not just drawing.

## Contracts

- **Facing:** the nose is authoritative. Aim requests a heading. Species turn at 90–540 degrees/s, using deterministic integer rotation. Idle and recovery allow turning; startup and active lock facing. A 180-degree tie rotates in the positive direction. An off-angle cast request still spends its resources and fires along actual facing: aim first, then commit. The baseline now waits for alignment.
- **Travel:** WASD remains world-space. Forward uses full speed; sideways and reverse use authored percentages. Intermediate directions interpolate continuously through the movement/facing dot product. A backwards input does not spin the creature around. Turning toward an escape route is the alternative to slower backpedaling.
- **Acceleration and braking:** each species has distinct velocity-convergence divisors. Lower is more responsive. Releasing movement brakes more firmly than the old universal drift; rain adds two ticks to each divisor. Residual speed below eight integer units snaps to zero when input is released.
- **Casting:** 131/160 signatures have zero voluntary startup movement; 29 retain explicit movement. Each move separately authors startup, active and recovery speed percentages. Planted startup/active cancels existing drift immediately, including in rain. Recovery generally allows 25–30% speed on committed moves and 60% on mobile moves. Knockback and body contact can still displace a planted body.
- **Dodge:** Space uses held movement direction while preserving facing. With no movement input, it steps to the creature's right (the clockwise perpendicular in screen coordinates). The direction locks for the dodge, so changing WASD halfway through does not curve it. Distance varies with species dodge speed (80–115% of the shared move). Shared stamina, cooldown and invulnerability timing remain intact. Root still prevents dodge.
- **Mobility moves:** lunges and blinks must brace during startup, then use authored displacement. They do not inherit backward walking penalties; their resource and recovery commitments are the price. They continue to respect collision.

## Different bodies, different solutions

| Species | Turn / second | Side / reverse | Intended consequence |
|---|---:|---:|---|
| Basaltusk | 90° | 42% / 28% | Predict an entry and commit; circling its flank matters. |
| Anvilnewt | 90° | 45% / 30% | Hold a facing and defend a working position. |
| Glasswing | 150° | 55% / 40% | Establish a firing lane, plant for artillery, spend mobility to relocate. |
| Saltcrab | 120° | 90% / 40% | Scuttle sideways behind its frontal defense. |
| Rimehare | 480° | 95% / 72% | Maintain lateral pressure with a mobile Icicle cast. |
| Ribbonape | 510° | 100% / 80% | Reposition while throwing and shape the return path. |
| Gloamcat | 540° | 88% / 65% | Win access to rear angles instead of relying on symmetric circle speed. |

These rules are independently authored; turn speed is not derived from body size or mass. All 40 profiles and each move's exact `move_start`, `move_active`, and `move_recovery` percentages are in the generated [species guide](species.md).

## Reading the workbench

The white chevron/nose shows physical facing. A guard draws only its frontal arc. Yellow attack corridors show startup, orange shows active contact, and a second bar counts down the current phase. Planted casts have ground braces and a PLANTED label. Green corridors show dodge/blink travel; lateral dodges visibly preserve the nose direction. The inspector lists yaw, strafe and reverse values, and the palette distinguishes PLANT/MOBILE/MOVE DODGE.

Corridors show nominal attack geometry and travel intent, not a prediction of future collisions, ricochets or wind. Terrain still clips movement and resolves contact in the simulation. Shapes are clipped to the arena viewport. HP is the upper green bar; the phase bar is below it. The manual cursor cross is requested aim, not a promise that the body can instantly face there.

Reproduce a paused visual inspection:

```sh
./build/creature_lab --species-a 2 --species-b 21 --arena 2 --preview-ticks 45
```

## Validation

- All 40 profiles have checks for bounded initial turning, reaching the opposite heading, unequal travel speeds, input-release braking, facing independent of travel, every signature's startup mobility, planted casts on wet terrain, directional dodge and the zero-input sidestep.
- Existing move, passive, status, collision, corruption and replay tests pass. The suite reports **461,622 assertions**, including repeated state/finite-value checks.
- Current golden replay: `1247c68eb6a064e7` (seed 77, rain, Gravemole/Kelpwidow, grove, 100 decisions). Native optimized and ASan/UBSan pass; Python matches the same fixture. Cross-platform CI runs this fixture on Windows, macOS and Linux.
- Actor observations expand to **2,638 floats**, exposing both bodies' locomotion and locked directions separately from facing, and all moves' phase mobility. The **80,579-parameter** reference policy passes inference, action submission and gradient smoke checks. Gymnasium checker and all 40 species pass. No trained learner is claimed.
- A fresh **56,160-match** diagnostic on v3 reports **21.1%–78.4%** aggregate scripted species rates, 52.54% seat-A wins, 22.34-second mean fights, and **zero pool overflows**. [Full report](../../reports/movement-v3-balance.md).

## Balance implications and next playtest

The old v2 rates are invalid for this ruleset. Quillrat reaches 78.4% while Anvilnewt falls to 21.1% under these available pilots. This is a material balance regression, not statistical evidence that the new feel is wrong. Movement, cast commitment and the baseline's orientation handling changed together, so the sweep does not isolate their causes. No HP or damage buffs were applied to flatten this result.

First compare a heavy defender against a fast strafe kit under manual control: can the defender establish a useful firing lane, can the attacker bait a commitment, and is the miss punish readable? Then distinguish poor access/turning behavior from inadequate payoff using targeted policies. Preserve meaningful movement tradeoffs while retuning; do not restore universal movement just to recover an aggregate number. This build is a movement playtest, not renewed balance certification.

V1/V2 snapshots, replays and policy input shapes are incompatible and fail version checks. The prior release remains available as a reproducible comparison.
