# First trained Tinikami pilot

The same 480 native matches compare a deterministic untrained network, an imitation warm start, the selected PPO checkpoint, and a scripted reference. Each of 240 scenarios is played from both seats. All 40 species, six maps, three weather states and four opponent spacing styles are represented. These are scripted-opponent tests, not human or learned-population evidence.

| Controller | Match score | Wins | Draws | Mean duration | Casts / minute |
|---|---:|---:|---:|---:|---:|
| Untrained network | 0.00% | 0 | 0 | 34.02s | 38.47 |
| Imitation only | 8.02% | 38 | 1 | 47.43s | 20.90 |
| Imitation + PPO | 23.75% | 114 | 0 | 47.70s | 32.04 |
| Scripted style 0 reference | 45.21% | 213 | 8 | 49.93s | 29.65 |

Score is 1 for a win, 0.5 for a draw and 0 for a loss. All four runs had zero pool overflows. Paired scenarios are correlated; these percentages are descriptive, not precise population estimates.

Training collected 65,536 demonstration transitions and then 1,966,080 on-policy decisions. The shipped model was selected at 1,146,880 PPO decisions by its score on the fixed 80-match validation set. That selection set covers all 40 species but only arena IDs 0–3; the separate native test covers all six. The training environment samples all six maps. The current trainer defaults to a larger 240-match validation set.

The chosen validation score was 42.5%; the different held-out native scenario set scores lower. No further tuning or checkpoint selection used the held-out native results. The bundled Python holdout is another diagnostic of this same selected model; do not add its games to the native set as independent evidence.

Some species still score zero in these twelve-game-per-species probes. Aim timing, route choice, range management and special-kit execution remain visibly weak. The roster is mechanically imbalanced before learning; a shared short training run does not remedy that. The network has separate recurrent memory per actor, but no per-individual persistent training, live praise-based updates or self-play league is implemented.

## Reproduce

```sh
make core brain build/brain_eval
./build/brain_eval models/baselines/random.tbrain reports/rl-v8-random.csv 240 1900000000
./build/brain_eval models/baselines/imitation.tbrain reports/rl-v8-imitation.csv 240 1900000000
./build/brain_eval models/apprentice.tbrain reports/rl-v8-ppo.csv 240 1900000000
./build/brain_eval scripted reports/rl-v8-scripted.csv 240 1900000000
python3 scripts/analyze_learning.py
```

Raw CSVs, the selected checkpoint, both baseline checkpoints, training configuration and the complete accepted-run log are included. The JSON report records hashes for the model, native controller, simulation and each input CSV. The native controller is the one used by the SDL game.

Brain format 2; rules/observations 7; content `223a8716`; 86,755 parameters; 347,060 bytes; model checksum `c79f418b`.
