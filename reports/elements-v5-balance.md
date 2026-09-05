# Scripted balance evidence

**56,160 matches; 2,808 games per species; 72 games per unordered matchup.** Every unordered pairing is run in both seats. The 36-seed protocol covers 3 weather states × 3 arenas × 4 style pairings. The species share a kit-aware heuristic; these are not independently trained or optimized policies.

Aggregate species win rates: **20.9%–77.3%**. Side A wins: **52.01%**. Mean fight: **22.4s**; median **19.8s**. Pool overflows: **0**.

Endings: KO 55,257, control 809, time limit 94.

These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.

| Species | Win rate | Games |
|---|---:|---:|
| Quillrat | 77.3% | 2,808 |
| Thornmantis | 72.8% | 2,808 |
| Coppergecko | 71.7% | 2,808 |
| Hooklynx | 67.9% | 2,808 |
| Waxwyrm | 67.8% | 2,808 |
| Saltcrab | 66.1% | 2,808 |
| Inkheron | 65.2% | 2,808 |
| Tidecoil | 61.5% | 2,808 |
| Gravemole | 61.1% | 2,808 |
| Pyrelisk | 59.5% | 2,808 |
| Gloamcat | 58.3% | 2,808 |
| Mooncalf | 57.1% | 2,808 |
| Clockfin | 55.4% | 2,808 |
| Brambleback | 55.2% | 2,808 |
| Dewotter | 54.3% | 2,808 |
| Sunstag | 53.7% | 2,808 |
| Ribbonape | 52.1% | 2,808 |
| Glasswing | 50.2% | 2,808 |
| Nullurchin | 50.1% | 2,808 |
| Mosswarden | 49.8% | 2,808 |
| Voltjack | 49.4% | 2,808 |
| Slagjaw | 48.9% | 2,808 |
| Oathhound | 48.3% | 2,808 |
| Latchspider | 47.8% | 2,808 |
| Duneskink | 47.6% | 2,808 |
| Rimehare | 47.5% | 2,808 |
| Bellox | 46.5% | 2,808 |
| Cinderfox | 45.4% | 2,808 |
| Nectarbat | 43.1% | 2,808 |
| Kelpwidow | 40.2% | 2,808 |
| Orchardboar | 37.1% | 2,808 |
| Ironmoth | 35.7% | 2,808 |
| Basaltusk | 34.6% | 2,808 |
| Echofin | 34.4% | 2,808 |
| Flintroc | 34.3% | 2,808 |
| Ashram | 34.2% | 2,808 |
| Miretoad | 34.2% | 2,808 |
| Galecrest | 34.0% | 2,808 |
| Prismray | 28.7% | 2,808 |
| Anvilnewt | 20.9% | 2,808 |

## Largest matchup asymmetries

| Species A | Species B | A win rate | Games |
|---|---|---:|---:|
| Saltcrab | Anvilnewt | 100.0% | 72 |
| Waxwyrm | Anvilnewt | 100.0% | 72 |
| Galecrest | Oathhound | 0.0% | 72 |
| Mosswarden | Waxwyrm | 0.0% | 72 |
| Prismray | Mooncalf | 0.0% | 72 |
| Prismray | Thornmantis | 0.0% | 72 |
| Gravemole | Anvilnewt | 100.0% | 72 |
| Sunstag | Galecrest | 100.0% | 72 |
| Quillrat | Anvilnewt | 100.0% | 72 |
| Quillrat | Mosswarden | 100.0% | 72 |
| Gloamcat | Anvilnewt | 100.0% | 72 |
| Basaltusk | Thornmantis | 0.0% | 72 |
| Miretoad | Pyrelisk | 0.0% | 72 |
| Voltjack | Quillrat | 0.0% | 72 |
| Brambleback | Mooncalf | 100.0% | 72 |
| Brambleback | Waxwyrm | 0.0% | 72 |
| Brambleback | Quillrat | 0.0% | 72 |
| Brambleback | Basaltusk | 100.0% | 72 |
| Mosswarden | Bellox | 98.6% | 72 |
| Thornmantis | Anvilnewt | 98.6% | 72 |

The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.

## Reproduce

`./build/tournament 36 reports/elements-v5.csv 2000`

Content fingerprint: `92ac508a`. Rules/observations: v5. The JSON report records source and engine hashes.
