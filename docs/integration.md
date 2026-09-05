# RL and rendering integration

## Entry points

C++: `reset`, `step`, `observe`, `action_mask`, `snapshot`, `restore`, `hash`, `command`. `World fork = world` creates an in-memory simulation branch. All state is per-world. Independent worlds can be assigned to separate workers; simultaneous calls on one world are unsupported.

C: `include/creature/api.h` exposes opaque handles and caller-owned numeric buffers. No STL types or C++ objects cross this ABI. `cr_batch_step` performs N arenas in one native call; it is currently serial within the call. Parallelize independent batches at the worker level. The native loop has no per-tick heap allocations. Snapshot/hash allocate temporary buffers and should not be used on every high-throughput training tick unless needed.

Python: `python/creature.py` uses only the standard library. `Batch` is a context manager with explicit reset and close. Output buffers are reused; copy observations/features into trajectory storage before the next call. NumPy can view them without copies using `np.ctypeslib.as_array`. A `[N,2,678]` tensor contains both players after each joint decision. The reset-observation method is separate, so final observations are never implicitly replaced.

Actions have shape `[N,2,5]`: movement x/y, aim x/y in world coordinates and ability (0 no-op, 1–4 move slots, 5 dodge). Four vector components are Q integers in [-1024,1024]. The optional model returns tanh-bounded floats; `quantize` implements explicit ties-away-from-zero rounding. Keep this quantizer versioned. If an actor outputs egocentric actions, rotate them back using its facing before quantization. The reference policy currently outputs world-space actions and receives absolute self position/facing for that purpose.

Features have shape `[N,2,6]`: actual damage dealt, damage taken, contacts dodged, interrupts, KO, death. They aggregate across all executed ticks. Friendly fire can appear in both dealt and taken for the same player. Status is `[N,4]`: terminated, truncated, winner (-1 none/draw), executed ticks. This is not a reward API. A sparse example is KO minus death; scale damage shaping separately and anneal it if it changes the desired tactics. Treat truncation bootstrap consistently in the learner.

## Actor schema v1: 678 floats

| Slice | Shape | Contents |
| --- | --- | --- |
| 0:24 | self[24] | HP/100, energy/1000, velocity x/y divided by 512, facing x/y divided by Q, (move+1)/5, phase/3, age/90, stun/30, burn/90, haste/90, radius/Q, x/(24Q), y/(18Q), guidance/3, guidance_age/900, five cooldown fractions, two reserved zeros |
| 24:456 | entities[27,16] | Opponent, two rocks, 16 projectile slots, eight zone slots |
| 456:536 | moves[5,16] | Kind/4, startup/30, active/30, recovery/30, cooldown/90, cost/1000, damage/100, range/(24Q), radius/(24Q), speed/512, impulse/512, burn/90, haste/90, hits_evasive, present=1, reserved=0 |
| 536:664 | history[16,8] | Newest visible events first; age/90, kind/9, actor relation, target relation, (move+1)/5, amount/100, visible=1, present=1 |
| 664:672 | global[8] | tick/2700, rain/1000, wind_x/16, wetness/1000, terminal, truncated, two reserved zeros |
| 672:678 | mask[6] | Readiness for no-op and five moves; no-op always legal |

Entity columns: kind/4, relative forward position/(24Q), relative right position/(24Q), relative-frame velocity forward and right ×30/(24Q), radius/(24Q), team relation (self +1, opponent -1, neutral 0), remaining lifetime/180, (move+1)/5, phase/3, phase age/90, HP/100, burn/90, facing forward, facing right, presence mask. Velocities use entity world velocity rotated into the actor frame, not velocity relative to the actor. Kind IDs: opponent 1, obstacle 2, projectile 3, zone 4. Zero rows are padding. Some normalized time features may exceed 1; these are scale factors, not universal clipping limits.

Phase: idle 0, startup 1, active 2, recovery 3. Guidance: free 0, attack 1, retreat 2, conserve 3; it expires at 900 ticks. Event kinds are documented by the enum in `sim.hpp`. Relations are ±1 from the observer's perspective. Enemy guidance events are filtered. The public history ring is bounded, so private commands may evict older records when saturated; strict information-theoretic privacy would require per-observer rings before adversarial competitive use.

The world is fully visible spatially: obstacles block contact but do not hide creatures. Actors do not receive enemy stamina, cooldowns, stun duration, guidance or RNG state. The renderer can inspect complete state. Enemy facing / locked telegraph aim is exposed in the actor frame; projectiles expose their travel direction. Add semantic announced-move descriptors before testing randomized opponent move libraries. A critic can receive the full World through a separately implemented training adapter; there is no privileged tensor in this version.

## Model and training boundary

`policy.py` demonstrates a 68,903-parameter encoder/GRU with continuous movement/aim, a masked categorical move head, and a value head. Masked pooling keeps padding from changing the latent state. It is a gradient/inference integration test with random weights, not a learning result. Start with PPO against scripted and historical opponents, a short curriculum, several random seeds, and held-out evaluation. Do not estimate actual convergence from raw simulator throughput.

Keep recurrent hidden state separate per creature per arena; clear it on reset and mask it across terminal transitions. For counterfactual branching, either retain the hidden state at the snapshot or reconstruct it by replaying observations. Copying the world alone does not copy the creature's memory. Store actual sampled integer actions and behavior-policy log probabilities in PPO rollout buffers. A fresh on-policy rollout from a historical world must use correctly initialized recurrent state; old replay trajectories are not automatically valid PPO data.

For lifelong updates, begin with episodic training in a local Python process. Hold the deployed weights fixed for a fight. Train a candidate, evaluate against a diverse fixed opponent suite and the previous brain, then promote it atomically between episodes. Record base/adapter/observation/rules versions. A frozen encoder plus trainable GRU/heads is a possible later parameter split, not a proven optimum. Feedback remains a learning signal and never alters physics. Authentication, durable queues, signed manifests, cloud sync and rollback remain future service work.

## Unity client plan

Use `DllImport` with C calling convention to wrap the existing C ABI. Drive policy decisions at 10 Hz; call `cr_step` once per joint action and interpolate presentation independently. Do not let Unity colliders or animation events apply combat damage. The initial ABI exposes observations and snapshot bytes; add a versioned render-state/event buffer for production Unity integration rather than decoding private C++ object layout. The SDL client currently links directly to the C++ state for inspection. This is a clean authority boundary, not a complete Unity package.

Keep inference local. Torch, ONNX Runtime or Unity's inference runtime can implement the policy adapter without being linked into the engine. Verify the exported model's input/output schema and quantization through replay fixtures before swapping runtimes. GPU model execution is outside the deterministic-simulation guarantee.
