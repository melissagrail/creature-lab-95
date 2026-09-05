# Alpha integration contract — rules / observations v5

The core is C++17 with no renderer, model runtime or network dependency. `include/creature/api.h` exports opaque handles, registry queries, match reset, batched stepping, structured actor observations, commands, snapshots and hashes. `cr_reset_match` selects both species, weather and arena. `cr_version`, `cr_observation_version` and `cr_content_hash` identify the contract. V1–V4 saves, replays and policies are deliberately incompatible.

## Canonical action and result

Each joint decision is `[2,5]` signed int32 values: move x/y, aim x/y, ability. Motion/aim components are in [-1024,1024]; movement is clamped to a unit disk. Ability 0 is no-op, 1–4 are the selected species' kit slots, 5 is dodge. Input is held for up to three 30-Hz ticks, with a single request on the first tick. Action masks describe readiness before accepting inputs; invalid requests spend nothing.

For fields/traps/turrets, aim magnitude controls ground-target distance as a fraction of cast range. Aim requests physical facing, approached at the species yaw limit while idle or recovering. Startup/active facing is locked. Attacks lock the current physical facing after that tick's turn; an off-angle request is accepted and can miss. Ground casts use that facing and requested magnitude. Blink moves its full authored distance. Dodge instead locks movement input, falling back to the right-hand perpendicular of facing when movement is zero, and does not turn the body toward its travel direction. For a wind-producing move, aim magnitude additionally sets wind strength: max(1, authored strength × magnitude /1024), projected along locked facing. The supplied wind moves are directional, not ground-targeted, so distance and wind strength do not compete for one input. Record the quantized integer actions, not just model floats. `quantize` rejects nonfinite inputs and uses explicit ties-away-from-zero rounding.

`cr_batch_step` outputs `[N,2,3620]` float observations, `[N,2,10]` integer features, and `[N,4]` status. Features are: HP damage dealt, damage taken, contacts dodged, interrupts, KO, death, HP healed, shield damage absorbed, control points gained, stamina spent. On-hit/status damage and environmental damage have deliberately different attribution; self-paid health is a resource cost, not opponent damage. Reward shaping remains the learner's choice.

Status is native terminated, native clock-expired, winner (-1 draw/none), executed physics ticks. No auto-reset. A KO or capture can end before all three ticks execute. The 90-second scored verdict is **terminal for the finite game** even though the native API preserves its separate `truncated`/clock flag for debugging. The Gymnasium adapter maps either native ending to learner termination; an external rollout cutoff would instead be a truncation requiring appropriate bootstrap. Do not bootstrap through an adjudicated final win/loss as though the match continues.

## Actor tensor v5: 3,620 floats

| Slice | Shape | Meaning |
|---|---|---|
| 0:66 | self[66] | Body, resources, cooldowns, statuses, passive meter/counter, physical attributes, axes |
| 66:3102 | entities[69,44] | Enemy, four rock slots, 32 projectile slots, 16 spell-zone slots, 16 surface slots |
| 3102:3342 | moves[5,48] | Semantic descriptors of the creature's four moves and shared dodge |
| 3342:3390 | announced[48] | Enemy's current move descriptor; zeros while idle |
| 3390:3582 | history[24,8] | Recent public events, newest first, masked padding |
| 3582:3614 | global[32] | Weather, terrain layout, time, objective/control and ending state |
| 3614:3620 | mask[6] | No-op plus five legal-action indicators |

Self columns 0–16: HP fraction, stamina/1000, world velocity x/y /512, facing x/y /1024, global (move+1)/161, phase/3, action age/90, stun/30, burn/150, haste/150, radius/1024, world x/(24×1024), y/(18×1024), guidance/3, guidance age/900. Columns 17–21 are cooldown fractions of each selected move.

Self 22–42: species/39, passive/39, shield/60, guard/60, poison/150, poison stacks/5, slow/90, root/30, silence/30, wound/150, mark/150, control resistance/75, passive meter/1000, counter modulo 3 /2, time since cast/90, stationary time/90, control/600, base speed/210, max HP/180, regeneration/12, mass/200. Columns 43–50 are all eight design axes /5. Columns 51–55: previous slot+1 /5, aim magnitude/1024, shield lifetime/90, current slot+1 /5, reserved passive timer/90.

Self 64–65: wind affinity /300 and highest-priority ground kind at the body center /8 (all overlapping surfaces are separately observable).

Self 56–63: yaw degrees/s /600, strafe /100, reverse /100, acceleration divisor /6, braking divisor /6, dodge speed /150, world locked direction x/y /1024.

Entity columns 0–13: kind/5, relative forward/right position /(24×1024), entity world velocity rotated into observer frame ×30/(24×1024), radius/(24×1024), team relation (+1 self, -1 enemy, 0 neutral), remaining lifetime/300, (move+1)/161, phase/3, phase age/90, HP fraction, forward/right facing relative to observer. Velocity is rotated world velocity, not subtraction of observer velocity.

Enemy columns 14–30: shield/60, guard/60, burn/150, poison stacks/5, slow/90, mark/150, species/39, passive meter/1000, resistance/75, counter modulo 3 /2, root/30, silence/30, wound/150, haste/150, poison duration/150, stun/30, locked aim magnitude/1024. Enemy columns 31–36: yaw degrees/s /600, strafe /100, reverse /100, acceleration divisor /6, braking divisor /6, dodge speed /150. Columns 37–38 are locked direction forward/right in the observer frame; columns 12–13 remain physical facing even during a lateral dodge. Enemy column 39 is wind affinity /300; column 40 is local ground kind /8; 41–42 are reserved. Column 43 is presence for every entity. Projectile columns 14/15 carry return phase and remaining bounces/3. Zone column 14 carries move kind/10; HP is /45. Zero rows are padding. Surface rows 53–68 use columns 14–16 for current kind /8, fallback kind /8 and transformation timer /300; team relation identifies the latest creator/transformer. Surface type IDs: bare 0, water 1, ice 2, brush 3, fire 4, steam 5, charged water 6, mud 7, oil 8. Surface columns 17–18 are world-space current x/y divided by 32. Type IDs: enemy 1, rock 2, projectile 3, spell zone 4, surface 5. Slot positions are stable for an object's lifetime but are reusable, not permanent identities.

Move columns 0–35: kind/10; startup/30; active/30; recovery/30; cooldown/180; stamina/1000; per-contact damage/60; range/(24×1024); radius/(4×1024); speed/800; impulse/1600; burn/150; haste/150; authored evasive-hit flag; minimum range/(24×1024); lifetime/300; interval/90; shots/4; spread/400; slow/90; root/30; silence/30; poison/150; wound/150; mark/150; shield/60; heal/30; guard/60; cleanse flag; stamina drain/300; marked payoff/30; execute bonus/30; health cost/20; bounces/3; returning flag; piercing flag. Column 36 is present=1 and 37–39 are voluntary movement multipliers in startup/active/recovery, each /100. Move 40–45: created surface kind /8, created radius /(4×1024), surface lifetime /600, element /4, wind strength /24, wind duration /600. Element IDs: neutral 0, heat 1, chill 2, shock 3, splash 4. Column 46 is created current strength /32; column 47 is reserved. Fields/traps hit evasion by shared kind rule even if the authored flag is zero.

History: age/150, kind/17, actor relation, target relation, (move+1)/161, amount/180, visible=1, present=1. Guidance is not inserted into public event history; it lives in private self state and replay input records. Event IDs 16 and 17 are ground changed and wind changed. Ground hazard damage uses move=-1, credits enemy damage to the last transformer/creator, and never credits self-harm as damage dealt. Opponent stamina and cooldowns, guidance, RNG and latent model memory are not exposed. Spatial state and combat telegraphs are public; rocks are collision occluders, not vision occluders.

Global 0–12: time/2700, rain/1000, resultant wind x/24, wetness/1000, native terminated, native clock-expired, arena/2, objective enabled, own/enemy control /600, own/enemy capture preparation /30, end reason/3. Global 13–15: resultant wind y/24, longest remaining gust /600, vane cooldown /240. Global 16–21: own then enemy contribution x/24, y/24, life/600. Global 22–28: prevailing x/y /24, vane world x/(24×1024), y/(18×1024), own/enemy capture progress /45, vane enabled. 29–31 reserved. Time-like normalization factors are scales, not clipping guarantees; some features exceed 1.

## Python and model lifecycle

`Batch` is a dependency-free ctypes bridge. It owns reusable output buffers; copy them before the next step when storing experience. NumPy can view these via `np.ctypeslib.as_array`. Separate worlds/batches may run on different workers; do not concurrently mutate one handle. The native batch loop is serial and allocation-free per tick. Snapshots/hashes allocate temporary buffers and are not necessary on every training tick.

`policy.py` demonstrates a 82,627-parameter recurrent network. It pools masked entities/history, encodes the actual move and announced enemy move descriptors, uses a 96-wide GRU, and scores each slot by combining its token with the recurrent state. This avoids pooling away which descriptor belongs to which action. We test inference, masks, quantization, native submission and gradients. Weights are random. No learner, trained checkpoint, or claim of convergence is included.

`gym_env.py` adds a checked Gymnasium interface for a custom hybrid-action learner versus a scripted opponent. Action space is a Dict with continuous `motion[4]` and Discrete `ability`; observation space is the structured Dict above. Not every off-the-shelf PPO implementation supports hybrid Dict actions. Use the reference heads in a suitable learner, or adapt intentionally; do not silently discretize aim/motion and assume equivalent gameplay. [Gymnasium's environment API](https://gymnasium.farama.org/api/env/) defines the reset/step and termination contracts used here.

Keep recurrent state per creature/arena. Reset memory between fights. A simulation snapshot alone is insufficient to fork an RNN policy; retain the corresponding memory or reconstruct it through observation replay. Historical trajectories are not automatically valid on-policy PPO data. Store behavior log probabilities, canonical actions, model/version metadata and the final observation before reset.

## Renderer / Unity boundary

The SDL workbench reads native C++ state directly. A future Unity client should use C ABI render-state/event buffers added as a versioned presentation adapter, not decode C++ object layout or drive damage from Unity physics/animation. The core, content, actions and snapshots already remain independent of the renderer. Production Unity bindings, model export/runtime packaging and networking are not implemented.

Inference should stay local. Training can later run in a local Python sidecar or authenticated remote worker. Hold weights fixed during an episode; evaluate candidates on historical opponents and species-specific scenarios before atomic between-fight promotion. Per-creature adapters, signed registry entries, replay queues and rollback are still future services.

Water and charged-water currents are external forces: overlapping vectors sum with length capped at 32, then scale by 100/body mass. They can carry planted or rooted bodies. Created currents use locked cast facing; freezing suspends the stored flow until thaw or break. No new action channel is needed. Rules/observation v4 snapshots and policies are incompatible despite unchanged tensor dimensions.
