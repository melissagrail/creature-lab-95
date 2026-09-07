# Tinikami controllers — alpha 0.10

`champion.tbrain` and `apprentice.tbrain` contain the same validation-selected rules-8 controller: **89,959 parameters, 359,876 bytes, checksum `d9ff10f9`**. Each has a matching `.pt` checkpoint with optimizer state and a `.json` manifest. The names remain separate so focused baseline and mixed-temperament training can diverge later. Native play needs neither Python nor Torch.

Three local PPO runs collected **15,728,640 new decisions** under the movement changes. The final run's selected checkpoint scored 72.5% on validation before the fresh native evaluation. On the paired native cohort it scores **73.06%** versus **46.25%** for the old weights explicitly migrated to the same new physics. Thirty-eight species improve and two tie in that small per-species cohort. The five temperament scores range from 63.06% to 74.83%; equal personality power is not established.

[Full results and campaign tests](../reports/rl-v10-summary.md), [selection manifest](../reports/rl-v10-selection.json), and [training logs/configs](../reports/training-v10/) include raw evidence and limitations. Rules are **8**, observations **7** (3,620 floats), content `223a8716`, and model format **3**. The core sim golden is `03b73ddd44073999`.

Retained baselines:

- `baselines/rules7/apprentice-v9.*`: the unmodified previously released controller, rules 7. It is intentionally incompatible with the current engine.
- `baselines/rules8/warmstart-v9.*`: explicit weights-only migration, checksum `4a424577`. Migration alone is not training.
- `baselines/rules8/footwork-v10.*` and `cornering-v10.*`: selected intermediate checkpoints. The first preceded the additional cornering rule; the second used the final movement physics.
- `baselines/legacy-footwork.*`: explicitly migrated format-2 compatibility fixture. Older v8/v9 experiments remain historical.

To repeat the final training stage from its immutable input, install `python/requirements-ml.txt`, build `make core brain`, and run:

```sh
python python/train.py --out runs/journey-repeat \
  --warmstart models/baselines/rules8/cornering-v10.pt \
  --seed 202609073 --threads 1 --envs 64 --horizon 128 \
  --updates 960 --sequence 16 --minibatch 32 --epochs 3 \
  --lr 0.00004 --gamma 0.9995 --entropy 0.003 --target-kl 0.06 \
  --eval-every 160 --eval-games 240 --test-games 240 --personality-rate 0.55 \
  --opponent models/baselines/rules8/warmstart-v9.pt \
  --opponent models/baselines/rules8/cornering-v10.pt --learned-opponent-rate 0.25
```

The exact recorded runtime versions are in `reports/training-v10/runtime.json`. Platform and library changes can affect floating-point training; bit-identical results are not promised across environments. Captured original configs reference release-file names; the selection manifest maps those names to the immutable weights actually loaded at startup.

Campaign companions supply persistent temperament and seeded individual inputs to this shared controller. They do not silently modify its weights during play. All weights were trained locally from this project's engine/controllers and are MIT licensed.
