# Engine contract, rules v1

## Numeric model

Coordinates use signed 32-bit integers at 1,024 units per game unit. Products, dot products and squared distances use signed 64-bit intermediates. Square roots use an integer algorithm; normalization truncates integer division toward zero. Physics never calls floating point, a system RNG, a clock, or an unordered-container iteration. Observations and rendering may use floats without feeding them back into world state. User/model floats must be quantized at the input boundary; record those integer actions.

The arena is a 24 × 18 rectangle with two circular rocks and circular creatures. Movement is kinematic with velocity relaxing toward the desired velocity. Rain increases the relaxation time after wetness builds. Geometry is deliberately approximate at a documented 1/16-unit maximum substep: movement is projected out of rocks; projectiles test obstacles before the opponent in each substep. These are bounded integer tests, not analytic swept time-of-impact physics. This leaves a small contact-order bias at near-simultaneous wall/body intersections. Extend the geometry before adding much smaller or faster objects.

The authoritative phase at state tick T is the phase that will execute during tick T. A request made at T begins at age 0; startup ticks are 0 through startup-1. The first active tick is age startup. Ability triggers occur only on the first tick of `step`; aim locks when the move starts. Both movement axes clamp to [-1024,1024] and the vector clamps to a unit disk. Aim is normalized, with a zero vector retaining the existing heading. Invalid ability indices become no-op. The API permits at most three ticks per call.

## Tick order

1. Advance cooldowns, resource regeneration, guidance age and transient timers; accept both actors' action requests.
2. Integrate both creatures and terrain collision, then symmetric creature separation.
3. Release move effects and collect active melee contacts.
4. Integrate projectiles (including wind), stopping on a rock/wall or one creature contact.
5. Collect persistent-zone damage on its 15-tick pulse.
6. Capture evasive state and apply collected hits in stable order. Attacks already collected may trade even if the actor is killed or interrupted this tick. Damage is capped by remaining HP; later overkill hits can therefore have zero credited damage.
7. Apply burn ticks (global tick cadence, every 30 ticks), age actions, resolve KOs, update wetness, increment tick. A simultaneous KO is a draw. At 2,700 ticks a non-KO episode truncates; it does not award a winner.

No automatic reset occurs. Stepping an ended world is a no-op and reports zero executed ticks. RL code must retain final observations before explicitly resetting. A decision ending on a KO may execute fewer than three ticks.

## Small move vocabulary

`Moves` in `src/sim.cpp` is an immutable data table. Each row provides kind, timings, resource cost, damage, physical geometry, speed, knockback, burn duration, haste duration, and evasion interaction. New moves within these kinds are ordinary data extensions, but the current five-slot action schema and fixed projectile/zone move kinds require a schema change for a larger move library.

| Move | Behavior |
| --- | --- |
| Quick Claw | Forward-offset circular contact, once per target per action |
| Ember Bolt | One projectile; wind bends velocity; burn refreshes duration |
| Thunder Lunge | Locked-direction movement during active ticks; hit interrupts enemy startup |
| Cinder Patch | Places a fire zone at a clamped forward point; pulses every 15 ticks, friendly fire, bypasses evasion; grants the caster 90 ticks of haste |
| Dodge | Locked-direction movement; ordinary contact misses only during active ticks |

Burn and haste are unique statuses using max-remaining-duration refresh; there is no stack-count or multiplicative stacking. Haste modifies movement speed. Burn ticks for two damage, and rain ages burn and zones twice as fast. The prototype does not have arbitrary status definitions, surface diffusion, elemental matchup tables, channels, cancel windows, animation-controlled effects, or probabilistic procs. These omissions keep the initial behavior legible and the serialization contract small.

There are 16 projectile slots, 8 zone slots and 32 historical events. Allocation uses the first free slot. At capacity a spawn is dropped and Overflow is emitted; cost/cooldown still apply. The ring drops the oldest event. Stable slot order is part of the observation contract. A future model should not treat a slot index as permanent entity identity.

## Snapshots and replays

A world occupies 1,932 bytes on the tested ABI. Its portable snapshot is 1,948 bytes, encoded by an explicit field visitor with `CRLB` magic, rules version, all state including RNG and history, and a 64-bit FNV-1a checksum. Do not serialize `sizeof(World)` bytes. The decoder bounds fields and verifies length/version/checksum before changing a world. The checksum catches corruption; it is not a cryptographic authentication primitive. In-process callers must preserve the public C++ state's invariants; arbitrary direct field mutation is for controlled tests only.

The immutable rule table is identified by `RulesVersion`; any combat-rule or schema change must increment the version and intentionally update the golden fixtures. Snapshots cannot silently migrate across rules. Save the code commit, rules version, observation version, model identifier and action quantizer version in a real creature save manifest.

`.crr` replay files contain the initial portable snapshot plus each joint integer action, queued guidance, trainer feedback and expected hash. The parser limits size/count and verifies every frame before accepting it. The initial snapshot can be a mid-fight fork. Events are bounded recent history, not a complete archival log; the action tape is authoritative. Praise/correction live in the tape, not the world snapshot. At present feedback is associated with a decision, without a learned credit-assignment algorithm.

## Determinism scope

The integer design targets identical states across compiler/architecture boundaries, conditional on the same rules and canonical actions. Golden replay checks, independent-world tests, decoder tests and sanitizers provide evidence; they are not a proof for every compiler. Neural inference and training are not guaranteed bit-identical across devices. Replays therefore store actions, not a request to rerun a policy.
