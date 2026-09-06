# Tinikami controllers — alpha 0.9

`champion.tbrain` is the focused default baseline; `apprentice.tbrain` is the temperament-conditioned spirit controller. Both currently contain the same selected format-3 weights: **89,959 parameters, 359,876 bytes, checksum `4a424577`**. They are separate files so focused and mixed-personality training can continue independently. Each has a matching `.pt` checkpoint with optimizer state and `.json` manifest.

The temperament trial began from v8, added 131,072 demonstration/DAgger-style transitions and 5,242,880 PPO decisions, and selected the checkpoint at 4,259,840 new PPO decisions. The winning-only trial collected 3,276,800 PPO decisions. Neutral validation selected the temperament model (40.42% versus 38.54%) before the larger test. The [selection manifest](../reports/rl-v9-selection.json) records the model hashes.

Fresh native scores across 1,440 paired games each: v8 22.85%, selected controller 37.60%, winning-only comparison 40.52%, scripted reference 49.76%. The default was not reselected using test scores. Several species remain weak and some temperament differences are subtle; see the [full report](../reports/rl-v9-summary.md).

`baselines/apprentice-v8.*` preserves the previous released model. `baselines/winning-only-v9.*` preserves the comparison run. Older random/imitation baselines remain available. All weights were trained locally from this project's simulation and pilots and are MIT licensed.

Rules/observations remain v7, content `223a8716`. Format 2 still plays with neutral preferences. Format 3 adds species embeddings, persistent temperament inputs and ability-conditioned controls. [Play, train and integrate](../docs/alpha/temperament.md).
