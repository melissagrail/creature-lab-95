# Scripted balance evidence

**56,160 matches; 2,808 games per species; 72 games per unordered matchup.** Every unordered pairing is run in both seats. The 36-seed protocol covers 3 weather states × 3 arenas × 4 style pairings. The species share a kit-aware heuristic; these are not independently trained or optimized policies.

Aggregate species win rates: **21.1%–78.4%**. Side A wins: **52.54%**. Mean fight: **22.3s**; median **19.6s**. Pool overflows: **0**.

Endings: KO 55,324, control 719, time limit 117.

These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.

| Species | Win rate | Games |
|---|---:|---:|
| Quillrat | 78.4% | 2,808 |
| Thornmantis | 72.2% | 2,808 |
| Coppergecko | 70.4% | 2,808 |
| Hooklynx | 69.9% | 2,808 |
| Saltcrab | 69.1% | 2,808 |
| Waxwyrm | 67.4% | 2,808 |
| Gravemole | 63.2% | 2,808 |
| Inkheron | 61.0% | 2,808 |
| Tidecoil | 59.1% | 2,808 |
| Gloamcat | 57.8% | 2,808 |
| Dewotter | 57.4% | 2,808 |
| Clockfin | 56.4% | 2,808 |
| Cinderfox | 56.0% | 2,808 |
| Brambleback | 53.8% | 2,808 |
| Sunstag | 53.5% | 2,808 |
| Pyrelisk | 52.5% | 2,808 |
| Glasswing | 52.0% | 2,808 |
| Mosswarden | 51.2% | 2,808 |
| Latchspider | 51.0% | 2,808 |
| Mooncalf | 50.8% | 2,808 |
| Nullurchin | 50.7% | 2,808 |
| Slagjaw | 49.7% | 2,808 |
| Voltjack | 49.5% | 2,808 |
| Ribbonape | 48.9% | 2,808 |
| Rimehare | 48.1% | 2,808 |
| Oathhound | 46.7% | 2,808 |
| Bellox | 43.1% | 2,808 |
| Kelpwidow | 42.6% | 2,808 |
| Ironmoth | 42.2% | 2,808 |
| Duneskink | 41.6% | 2,808 |
| Nectarbat | 38.9% | 2,808 |
| Orchardboar | 38.4% | 2,808 |
| Echofin | 37.4% | 2,808 |
| Ashram | 36.4% | 2,808 |
| Miretoad | 34.0% | 2,808 |
| Basaltusk | 34.0% | 2,808 |
| Flintroc | 33.6% | 2,808 |
| Prismray | 32.4% | 2,808 |
| Galecrest | 27.4% | 2,808 |
| Anvilnewt | 21.1% | 2,808 |

## Largest matchup asymmetries

| Species A | Species B | A win rate | Games |
|---|---|---:|---:|
| Saltcrab | Anvilnewt | 100.0% | 72 |
| Thornmantis | Anvilnewt | 100.0% | 72 |
| Gravemole | Anvilnewt | 100.0% | 72 |
| Sunstag | Anvilnewt | 100.0% | 72 |
| Sunstag | Galecrest | 100.0% | 72 |
| Quillrat | Flintroc | 100.0% | 72 |
| Quillrat | Orchardboar | 100.0% | 72 |
| Quillrat | Anvilnewt | 100.0% | 72 |
| Ironmoth | Quillrat | 0.0% | 72 |
| Gloamcat | Anvilnewt | 100.0% | 72 |
| Basaltusk | Waxwyrm | 0.0% | 72 |
| Basaltusk | Thornmantis | 0.0% | 72 |
| Basaltusk | Quillrat | 0.0% | 72 |
| Miretoad | Sunstag | 0.0% | 72 |
| Miretoad | Quillrat | 0.0% | 72 |
| Voltjack | Anvilnewt | 100.0% | 72 |
| Brambleback | Inkheron | 0.0% | 72 |
| Brambleback | Bellox | 100.0% | 72 |
| Brambleback | Quillrat | 0.0% | 72 |
| Brambleback | Basaltusk | 100.0% | 72 |

The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.

## Reproduce

`./build/tournament 36 reports/movement-v3.csv 2000`

Content fingerprint: `56c01336`. Rules/observations: v3. The JSON report records source and engine hashes.
