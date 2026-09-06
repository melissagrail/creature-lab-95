# Tinikami apprentice

`apprentice.tbrain` is the selected trained controller used by the native game. It has 86,755 parameters, 347,060 bytes, brain format 2 and checksum `c79f418b`. It requires rules/observation v7 and content fingerprint `223a8716`.

`apprentice.pt` contains the same PyTorch weights plus optimizer state for further training. `apprentice.json` records identity, tensor order and training configuration. `baselines/` preserves the untrained and imitation-only checkpoints/exports used in the matched evaluation. All these weights were produced locally from this project's simulator and scripted pilots; they are included under the repository MIT license.

The accepted run had 65,536 demonstration transitions, 24 imitation epochs and 1,966,080 PPO decisions. Selection occurred at 1,146,880 PPO decisions on a separate 80-match validation set. Native held-out score: 23.75% across 480 paired games, versus 8.02% for imitation alone. The controller remains weak on several species; it is not a competitive trained population.

[Play, train and integration](../docs/alpha/learning.md) · [Full evaluation](../reports/rl-v8-summary.md) · [Training configuration](../reports/rl-v8-config.json) · [Training trace](../reports/rl-v8-learning.jsonl)
