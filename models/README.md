# Tinikami controllers — alpha 0.12

`apprentice.tbrain` and `champion.tbrain` contain the same explicitly expanded v0.10 learned weights: **90,727 parameters, 362,948 bytes, checksum `187a9a2f`**. Rules **9**, observations **8** (3,628 floats), content `55a82aa5`, model format **3**, recurrent memory **96 floats** per actor. The new simulation golden is `4deb7e7e3423a32b`.

v0.12 changes only Quillrat's charged burst tuning. The v0.11 weights are retained exactly, with an explicit content compatibility migration in `scripts/migrate-quill-tuning.py`; the original v0.11 checkpoint and manifest remain in `baselines/rules9/release-v11/`. This is not training. Beginner assistance pilots bond 1–2 in the campaign, because the full-kit learned model performs poorly on several restricted opening matchups. Keeper calls are a deterministic controller layer, not learned compliance.

Eight new encoder inputs expose developmental limits. Their weights initialize to zero; every previous weight is retained. This is a schema migration, not new training. The `.pt` files are weights-only warm starts; optimizer moments from the old-shaped encoder are not silently reused. The original full checkpoint and manifest are retained in `baselines/rules8/release-v10/`. The historical `scripts/migrate-development.py SOURCE.pt TARGET` reproduces that expansion with the v0.11 checkout. In the current checkout, migrate the retained v0.11 checkpoint with `scripts/migrate-quill-tuning.py SOURCE.pt TARGET`. Older models remain deliberately incompatible.

The native and Python models pass inference parity across all forty species. Campaign profiles change available actions, physical speed, energy capacity and regeneration, and always apply the engine's legal-action mask. `python/train.py --development-rate .35` opts into a mixture of developmental scenarios for future PPO training. The launcher training scripts enable this mixture. The shipped weights have not undergone that curriculum training.

`baselines/legacy-development.*` is the similarly expanded format-2 compatibility fixture (87,523 parameters). Historical rules-7/8 checkpoints and their evidence remain intact. To reproduce the historical training command below, check out release `v0.10.0-alpha.1`; to train on the current engine use a migrated checkpoint and current opponents.

Campaign companions supply persistent temperament and seeded variation to a shared controller. They do not change its weights during play. Native play requires neither Python nor Torch. All project-trained weights are MIT licensed.

## Historical v0.10 training and evaluation


`champion.tbrain` and `apprentice.tbrain` contain the same validation-selected rules-8 controller: **89,959 parameters, 359,876 bytes, checksum `d9ff10f9`**. Each has a matching `.pt` checkpoint with optimizer state and a `.json` manifest. The names remain separate so focused baseline and mixed-temperament training can diverge later. Native play needs neither Python nor Torch.

Three local PPO runs collected **15,728,640 new decisions** under the movement changes. The final run's selected checkpoint scored 72.5% on validation before the fresh native evaluation. On the paired native cohort it scores **73.06%** versus **46.25%** for the old weights explicitly migrated to the same new physics. Thirty-eight species improve and two tie in that small per-species cohort. The five temperament scores range from 63.06% to 74.83%; equal personality power is not established.

[Full results and campaign tests](../reports/rl-v10-summary.md), [selection manifest](../reports/rl-v10-selection.json), and [training logs/configs](../reports/training-v10/) include raw evidence and limitations. Rules are **8**, observations **7** (3,620 floats), content `223a8716`, and model format **3**. The historical core sim golden is `03b73ddd44073999`.

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
