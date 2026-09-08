# Playtest follow-up — 0.12.1

## Melee approach

Brambleback's beginner controller repeatedly committed Briar Club around 2,010 units from a circling Cinderfox. The target moved out of the planted hitbox during startup; a captured encounter continued to the time limit without dealing damage.

Campaign attack guidance now reserves a movement margin based on observed target speed and the art's startup, bounded to 350–800 units for melee. It closes into that range before planting. Damage, hitboxes, enemy behavior, simulation identity and learned model weights are unchanged. This applies to beginner assistance and explicit attack calls, not the unrestricted learned policy.

Replaying the reported state produced seven successful swings and a win, with Brambleback retaining 73 HP. A fresh encounter also completed with seven hits. Four fresh-seed regression cases require meaningful hits, limited missed casts and completion. Private playtest notes and captures are not included in this repository.

## Battle music

An original 144 BPM, 16-bar procedural chip arrangement adds a lead hook, response phrases, bass, arpeggios, kick, snare, hats and turnaround fills. A quieter bridge creates contrast. Combat begins the arrangement at its opening and smoothly blends with the existing exploration music. F8 and the existing sound lifecycle remain intact. No downloaded music or runtime audio assets are required.

The offline callback test covers the complete arrangement twice, combat exit, reward chime and mute: peak 0.145, RMS 0.0183, no nonfinite samples or clipping. Sound aesthetics still need human listening feedback.

## Validation

- 67,641 campaign checks, 657,305 simulation checks, 74,642 environment checks and 22,052 brain checks passed; deterministic golden remains `4deb7e7e3423a32b`.
- 120/120 opening routes with autonomous assistance and 120/120 with retreat calls; 3,600/3,600 total duel wins, no locked arts selected. These deterministic fixtures are regression coverage, not independent statistical balance evidence.
- All five tested starter routes reached eight bells and forty companions. Full-route CSV records losses and retries; no fabricated outcomes.
- 22 campaign rendering fixtures, 1,280 sprite cells, 80 native controller checks and workbench viewer smoke checks passed.
- Relocated macOS ZIP passed signing, dependency and native controller checks.

Existing journey saves and note formats are unchanged. Addressed feedback is archived locally with its snapshots, outside the active notebook list.
