# Scripted balance evidence

**56,160 matches; 2,808 games per species; 72 games per unordered matchup.** Every unordered pairing is run in both seats. The 36-seed protocol covers 3 weather states × 3 arenas × 4 style pairings. The species share a kit-aware heuristic; these are not independently trained or optimized policies.

Aggregate species win rates: **39.9%–58.6%**. Side A wins: **50.83%**. Mean fight: **25.1s**; median **22.5s**. Pool overflows: **0**.

Endings: KO 55,979, control 136, time limit 45.

These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.

| Species | Win rate | Games |
|---|---:|---:|
| Dewotter | 58.6% | 2,808 |
| Rimehare | 58.6% | 2,808 |
| Nectarbat | 57.4% | 2,808 |
| Voltjack | 56.9% | 2,808 |
| Coppergecko | 56.7% | 2,808 |
| Quillrat | 56.4% | 2,808 |
| Gloamcat | 56.2% | 2,808 |
| Echofin | 55.9% | 2,808 |
| Ironmoth | 55.5% | 2,808 |
| Glasswing | 55.0% | 2,808 |
| Duneskink | 54.4% | 2,808 |
| Pyrelisk | 53.7% | 2,808 |
| Thornmantis | 52.7% | 2,808 |
| Hooklynx | 52.1% | 2,808 |
| Waxwyrm | 52.0% | 2,808 |
| Saltcrab | 51.7% | 2,808 |
| Tidecoil | 50.3% | 2,808 |
| Ashram | 50.3% | 2,808 |
| Mooncalf | 50.3% | 2,808 |
| Cinderfox | 49.9% | 2,808 |
| Ribbonape | 49.8% | 2,808 |
| Clockfin | 49.3% | 2,808 |
| Inkheron | 49.2% | 2,808 |
| Slagjaw | 49.1% | 2,808 |
| Orchardboar | 48.0% | 2,808 |
| Gravemole | 47.2% | 2,808 |
| Prismray | 47.1% | 2,808 |
| Latchspider | 46.8% | 2,808 |
| Flintroc | 46.4% | 2,808 |
| Brambleback | 46.2% | 2,808 |
| Sunstag | 45.6% | 2,808 |
| Bellox | 45.5% | 2,808 |
| Mosswarden | 45.3% | 2,808 |
| Galecrest | 44.6% | 2,808 |
| Basaltusk | 44.5% | 2,808 |
| Kelpwidow | 44.3% | 2,808 |
| Oathhound | 42.8% | 2,808 |
| Anvilnewt | 42.7% | 2,808 |
| Nullurchin | 41.0% | 2,808 |
| Miretoad | 39.9% | 2,808 |

## Largest matchup asymmetries

| Species A | Species B | A win rate | Games |
|---|---|---:|---:|
| Miretoad | Quillrat | 0.0% | 72 |
| Miretoad | Dewotter | 1.4% | 72 |
| Miretoad | Gravemole | 2.8% | 72 |
| Brambleback | Ironmoth | 3.5% | 72 |
| Voltjack | Bellox | 95.8% | 72 |
| Bellox | Flintroc | 94.4% | 72 |
| Saltcrab | Anvilnewt | 94.4% | 72 |
| Basaltusk | Gravemole | 5.6% | 72 |
| Ashram | Anvilnewt | 92.4% | 72 |
| Bellox | Slagjaw | 8.3% | 72 |
| Basaltusk | Gloamcat | 8.3% | 72 |
| Brambleback | Quillrat | 8.3% | 72 |
| Flintroc | Oathhound | 9.7% | 72 |
| Sunstag | Nectarbat | 9.7% | 72 |
| Basaltusk | Thornmantis | 11.1% | 72 |
| Saltcrab | Bellox | 88.9% | 72 |
| Thornmantis | Bellox | 88.9% | 72 |
| Rimehare | Gravemole | 88.9% | 72 |
| Ashram | Oathhound | 87.5% | 72 |
| Miretoad | Flintroc | 12.5% | 72 |

The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.

## Reproduce

`./build/tournament 36 reports/holdout.csv 2000`

Content fingerprint: `7def5f00`. Rules/observations: v2. The JSON report records source and engine hashes.
