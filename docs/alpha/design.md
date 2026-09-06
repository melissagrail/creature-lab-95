# Combat alpha design: a roster of decisions

This alpha is a competitive **one-creature duel**, with MOBA-style commitment, zoning, resource exchanges and objective pressure. It is not a five-player lane map. Forty executable kits are here to test whether learned control can produce different forms of mastery. A species' numbers remain fixed between fights; personal progression should later come from decisions, not multiplying stats.

## The central promise

A creature should feel different because it is trying to make a different situation happen. Cinderfox wants a burning target. Mooncalf wants a shielded trade that stores retaliation. Glasswing wants a long, motionless firing window. Ribbonape wants its return path to cross the enemy after it has moved. Latchspider wants a web route. The opponent must have an intelligible way to deny that situation.

The winning pattern, losing pattern and learning test for every species are in [the species guide](species.md). Move numbers are generated directly from the same JSON the engine compiles; the guide is not a separate, drifting design spreadsheet.

## Eight axes and their extremes

| Axis | Low end | High end | What pays for the high end |
|---|---|---|---|
| Reach | Contact blades | Long, narrow, minimum-range artillery | Fragility, startup, bad close defense, lost center access |
| Commitment | Quick poke then move | Locked lunge / long beam windup | A conspicuous punish window on a miss |
| Mobility | Slow body, defensive residency | Fast strafe, blink, long dash | Low health, energy competition, limited ranged pressure |
| Control | Damage must persuade movement | Pull, push, root, silence, wall slam | Setup reliability, modest direct damage, lockout against chains |
| Persistence | All value comes from the current cast | Gardens, traps, turrets, poison | Preparation, locality, destructible assets or expiry |
| Sustain | One finite health bar | Healing contingent on contact, territory or delayed harvest | Wounds, access denial, burst before the payout |
| Defense | Avoid damage spatially | Guard, shield, reflect, heavy mass | Facing, cooldown gaps, anti-shield tools, relocation cost |
| Execution | Broad, reliable pattern | Return geometry, banks, rear angles, timed combos | Lower reliability must buy an attainable payoff, not a theoretical one |

These are qualitative design axes, not additive power scores. A value of five does not mean a species is five times stronger. We intentionally avoid giving a kit strong reach, access, safety, sustain and control at the same time. Each kit has four signature actions plus a shared dodge, so offensive identity cannot remove the baseline ability to escape an ordinary telegraph.

## Match structure: the bloom

The central 2.44-unit-radius bloom makes holding space valuable. After one uncontested second inside it, a creature gains one control point each tick. Six hundred total points wins the fight; leaving or contesting resets the one-second capture preparation, not banked points. There is no automatic HP or damage buff for controlling it. A KO wins immediately and takes priority over capture on the same tick.

At 60 seconds, a visible circular boundary begins contracting from 12 units to 5 by the 90-second time limit. Every half-second outside it deals three direct environmental damage. At the time limit, control score decides first, then remaining health fraction, then draw. The API still marks the time limit as truncation and reports the adjudicated winner separately.

Why this structure: a protection kit needs a way to cash out denied damage; a displacement kit needs somewhere that matters; an artillery kit needs a reason to find productive pressure rather than retreat forever. The bloom is not intended to erase range. It is a major balance variable: we will measure whether strong trained artillery can deny capture without being forced into melee. Pure damage-only tests can disable the objective in a controlled C++ scenario.

Three layouts stress different choices: two central pillars, a four-rock grove, and an open arena. Weather is clear, wind, or rain plus wind. The balance sweep crosses these independently rather than conflating a map with a weather condition.

## Shared rules that preserve counterplay

- Movement, aim and action selection are independent inputs. Physical facing turns at the authored yaw rate and locks when a cast begins. Forward, sideways and reverse movement have separate budgets; most casts hard-stop voluntary movement through startup/active. Selected mobile attacks retain an explicit fraction. Active lunges/dodges provide their own motion. [Movement v3](movement.md) defines the exact contracts. Blink follows collision-constrained travel and cannot teleport through rocks.
- Each action has startup, active and recovery. There is no animation authority, random crit, chance-to-dodge statistic, or hidden hit roll. Narrow geometry and timing create misses.
- Ground-targeted fields/traps/turrets use aim-vector magnitude for distance, bounded by cast range. A zero-length aim places at self while retaining facing. Other directional moves use normalized aim. Blinks currently travel their defined full distance.
- Guard reduces frontal contact damage by 40%; it does not protect from the rear. Shields absorb post-guard damage and refresh to the larger remaining amount, not additive stacks. They expire. Barrier numbers are an intentionally scarce budget.
- A root prevents movement and new mobility actions. Silence prevents slots 1–4 but leaves dodge available. Slow reduces movement by 35%. Haste increases it by 25%. Slow/root/silence are not the same mechanic.
- A successful root, silence or interrupt establishes a shared 75-tick resistance window. Further such applications fail during that window. This deliberately prevents permanent control chains. Slows can refresh; pulls/pushes remain displacement and do not inherit the lockout.
- Dodge ignores ordinary direct contact only in its active phase. Persistent fields and armed traps hit evasive targets: leaving their footprint is the counterplay. Resources and recovery still make a failed dodge costly.
- Burn refreshes and deals two damage per second. Poison stacks to three, or five for Miretoad, and deals stack count per second. DoT ticks use a global tick cadence. Wounds halve healing. Marks have an owner and are consumed only by their owner's explicit payoff.
- Damage can be traded. Contacts are collected before resolution; a hit already collected is not erased by a later KO/interrupt that tick. On-hit statuses apply through shields, but not through a successful dodge.
- Push/pull respects mass and terrain. An obstructed positive knockback adds six wall-slam damage. It is damage, not another stun. A pull does not add wall damage.
- Traps are visible, have 12 HP, arm after 15 ticks, and trigger once. Turrets have 32 HP (45 for Waxwyrm), fire only with line of sight within 7.32 units, and expire. All direct attack shapes can damage enemy targetable setup. Persistent ordinary fields cannot be destroyed.
- Terrain blocks movement, projectiles, beams and direct melee/area line of sight. Fields are placed at a valid position but can be cast across rocks. Spell fields damage enemies and heal their owner. Ground fire and charged-water hazards affect both creatures, including their creator. Ground and spell fields are separate layers: extinguishing ground does not cancel a spell.
- Rain builds arena-wide wetness, changing traction; it ages burn and burning fields twice as fast. Wind is now a uniform two-dimensional vector field, composed from prevailing weather and two timed player contributions. It accelerates projectiles, changes travel speed relative to the flow, and advects steam. [Terrain/wind foundation](terrain.md) and [elements v5](elements.md) define the contracts. There is no hidden random weather change during a fight.

## Resource economy and tempo

Health, energy, cooldowns and position are separate budgets. Energy regenerates at a species-specific rate only after the 24-tick spending lock expires, while idle or recovering. Every move and dodge competes for the same 1,000 internal units (100 displayed energy); movement is free. [V6 pacing and exceptions](tinikami.md) describes the economy. Cooldowns begin on acceptance, and a rejected input spends nothing. Health-cost moves cannot self-KO. Utility casts do not generate invulnerability merely because they are being cast.

A nominal damage-per-second calculation is only a first filter. We also inspect damage available without access, damage that survives a miss, setup denial cost, safe damage per energy, shield/heal throughput, and how much of a kit remains while retreating. In particular, fan projectiles have per-pellet damage: close-range multi-contact is a real burst spike, not an accidental assumption that one cast always equals one hit.

The bounded numeric tuning script is an aid for initial outliers. It adjusts health/direct damage and reduces repeat sustain for strong outliers, preserving reach, timing and special interactions. Its outputs are logged, never silently applied in a build. It is not a substitute for authored judgment, and a weak scripted pilot is not proof a species needs a buff.

## Training and player guidance

Attack/retreat/conserve are observations, not physics overrides. Praise/correction are experience annotations. The baseline responds to guidance by simple heuristics; learned obedience is future work. Models should infer locally and remain fixed during a fight. Individual adaptation belongs between episodes, with evaluation and rollback before promotion.

We expose semantic move descriptors, announced enemy move data, body attributes, public passive meters, status state, terrain entities, summon health, and objective pressure. Models should not need to rediscover a species by pixels or memorize that action 2 always means a particular spell. The reference recurrent policy now scores each actual move token, preserving the connection between descriptor and slot.

Species-specific learning tests should be judged on held-out scenarios, not just wins: rear-hit rate without excess deaths for Gloamcat; shield arrival timing for Ironmoth; conditional payoff damage for Voltjack; return-path hits for Ribbonape; turret uptime with useful lines for Waxwyrm; reduced poisoned time for Dewotter. These tests are a curriculum specification, not claims that any model has learned them yet.

## Alpha boundaries

This is a playable engine/content alpha, not a completed game or a competitively certified roster. The two skins share the same simulation. There is no campaign, collection economy, matchmaking, network authority, trained checkpoint, species progression service or per-creature persistent learner. There are no stealth/vision rules, teams, jump height, arbitrary scripted callbacks or rock destruction. Brush can burn away and water can change state. The central mechanics, all 40 kits, model interface, test harness and design contracts are implemented; the next decisive evidence is trained-policy and human playtesting.
