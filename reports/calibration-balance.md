# Scripted balance evidence

**56,160 matches; 2,808 games per species; 72 games per unordered matchup.** Every unordered pairing is run in both seats. The 36-seed protocol covers 3 weather states × 3 arenas × 4 style pairings. The species share a kit-aware heuristic; these are not independently trained or optimized policies.

Aggregate species win rates: **40.1%–58.4%**. Side A wins: **51.09%**. Mean fight: **25.1s**; median **22.5s**. Pool overflows: **0**.

Endings: KO 55,980, control 130, time limit 50.

These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.

| Species | Win rate | Games |
|---|---:|---:|
| Coppergecko | 58.4% | 2,808 |
| Quillrat | 57.5% | 2,808 |
| Dewotter | 57.4% | 2,808 |
| Voltjack | 57.4% | 2,808 |
| Echofin | 56.4% | 2,808 |
| Rimehare | 55.9% | 2,808 |
| Thornmantis | 55.3% | 2,808 |
| Pyrelisk | 55.2% | 2,808 |
| Gloamcat | 54.9% | 2,808 |
| Duneskink | 54.8% | 2,808 |
| Ironmoth | 54.8% | 2,808 |
| Nectarbat | 54.8% | 2,808 |
| Glasswing | 54.7% | 2,808 |
| Saltcrab | 53.3% | 2,808 |
| Mooncalf | 50.5% | 2,808 |
| Hooklynx | 50.3% | 2,808 |
| Clockfin | 50.1% | 2,808 |
| Ribbonape | 50.0% | 2,808 |
| Waxwyrm | 49.7% | 2,808 |
| Orchardboar | 49.6% | 2,808 |
| Cinderfox | 49.5% | 2,808 |
| Tidecoil | 49.4% | 2,808 |
| Inkheron | 48.9% | 2,808 |
| Gravemole | 48.4% | 2,808 |
| Slagjaw | 48.3% | 2,808 |
| Flintroc | 48.2% | 2,808 |
| Ashram | 48.1% | 2,808 |
| Bellox | 47.7% | 2,808 |
| Latchspider | 47.6% | 2,808 |
| Prismray | 46.7% | 2,808 |
| Sunstag | 46.4% | 2,808 |
| Basaltusk | 44.7% | 2,808 |
| Brambleback | 44.1% | 2,808 |
| Oathhound | 44.1% | 2,808 |
| Mosswarden | 43.7% | 2,808 |
| Miretoad | 43.5% | 2,808 |
| Galecrest | 43.4% | 2,808 |
| Kelpwidow | 43.3% | 2,808 |
| Anvilnewt | 43.1% | 2,808 |
| Nullurchin | 40.1% | 2,808 |

## Largest matchup asymmetries

| Species A | Species B | A win rate | Games |
|---|---|---:|---:|
| Basaltusk | Thornmantis | 0.0% | 72 |
| Miretoad | Dewotter | 0.0% | 72 |
| Miretoad | Gravemole | 1.4% | 72 |
| Saltcrab | Anvilnewt | 97.2% | 72 |
| Basaltusk | Gravemole | 2.8% | 72 |
| Miretoad | Quillrat | 2.8% | 72 |
| Thornmantis | Nullurchin | 95.8% | 72 |
| Miretoad | Sunstag | 4.2% | 72 |
| Miretoad | Mooncalf | 94.4% | 72 |
| Mosswarden | Pyrelisk | 6.9% | 72 |
| Brambleback | Quillrat | 6.9% | 72 |
| Brambleback | Ironmoth | 7.6% | 72 |
| Basaltusk | Gloamcat | 8.3% | 72 |
| Bellox | Flintroc | 91.7% | 72 |
| Saltcrab | Bellox | 91.7% | 72 |
| Anvilnewt | Mooncalf | 9.7% | 72 |
| Gloamcat | Prismray | 90.3% | 72 |
| Miretoad | Flintroc | 9.7% | 72 |
| Voltjack | Oathhound | 90.3% | 72 |
| Voltjack | Bellox | 90.3% | 72 |

The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.

## Reproduce

`./build/tournament 36 reports/calibration.csv 1000`

Content fingerprint: `7def5f00`. Rules/observations: v2. The JSON report records source and engine hashes.
