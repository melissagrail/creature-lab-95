# Scripted balance evidence

**112,320 matches; 5,616 games per species; 144 games per unordered matchup.** Every unordered pairing is run in both seats. This run uses 72 seeds across 6 arenas and 3 weather states; a complete four-style pass requires 72 seeds. The species share a kit-aware heuristic; these are not independently trained or optimized policies.

Aggregate species win rates: **14.4%–91.0%**. Side A wins: **51.14%**. Mean fight: **46.7s**; median **42.1s**. Pool overflows: **0**.

Endings: KO 90,945, control 10,814, time limit 10,561.

These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.

| Species | Win rate | Games |
|---|---:|---:|
| Quillrat | 91.0% | 5,616 |
| Hooklynx | 78.1% | 5,616 |
| Gravemole | 74.8% | 5,616 |
| Pyrelisk | 74.1% | 5,616 |
| Ribbonape | 73.0% | 5,616 |
| Coppergecko | 72.6% | 5,616 |
| Flintroc | 70.0% | 5,616 |
| Glasswing | 66.0% | 5,616 |
| Rimehare | 62.7% | 5,616 |
| Latchspider | 62.2% | 5,616 |
| Inkheron | 61.0% | 5,616 |
| Kelpwidow | 59.9% | 5,616 |
| Ashram | 58.0% | 5,616 |
| Miretoad | 57.4% | 5,616 |
| Tidecoil | 57.1% | 5,616 |
| Cinderfox | 56.1% | 5,616 |
| Slagjaw | 55.9% | 5,616 |
| Clockfin | 54.6% | 5,616 |
| Duneskink | 50.9% | 5,616 |
| Ironmoth | 49.4% | 5,616 |
| Galecrest | 48.9% | 5,616 |
| Brambleback | 47.7% | 5,616 |
| Gloamcat | 47.2% | 5,616 |
| Nectarbat | 46.5% | 5,616 |
| Thornmantis | 45.5% | 5,616 |
| Orchardboar | 44.3% | 5,616 |
| Sunstag | 43.9% | 5,616 |
| Dewotter | 41.4% | 5,616 |
| Prismray | 41.0% | 5,616 |
| Nullurchin | 40.8% | 5,616 |
| Echofin | 39.0% | 5,616 |
| Basaltusk | 35.1% | 5,616 |
| Waxwyrm | 33.5% | 5,616 |
| Anvilnewt | 30.3% | 5,616 |
| Voltjack | 28.9% | 5,616 |
| Mooncalf | 23.5% | 5,616 |
| Bellox | 23.3% | 5,616 |
| Saltcrab | 21.1% | 5,616 |
| Oathhound | 18.8% | 5,616 |
| Mosswarden | 14.4% | 5,616 |

## Largest matchup asymmetries

| Species A | Species B | A win rate | Games |
|---|---|---:|---:|
| Ribbonape | Bellox | 100.0% | 144 |
| Mosswarden | Hooklynx | 0.0% | 144 |
| Quillrat | Oathhound | 100.0% | 144 |
| Quillrat | Saltcrab | 100.0% | 144 |
| Voltjack | Quillrat | 0.0% | 144 |
| Saltcrab | Coppergecko | 0.7% | 144 |
| Quillrat | Waxwyrm | 99.3% | 144 |
| Basaltusk | Hooklynx | 0.7% | 144 |
| Miretoad | Mooncalf | 99.3% | 144 |
| Ribbonape | Oathhound | 98.6% | 144 |
| Ribbonape | Mooncalf | 98.6% | 144 |
| Quillrat | Mooncalf | 98.6% | 144 |
| Quillrat | Mosswarden | 98.6% | 144 |
| Basaltusk | Quillrat | 1.4% | 144 |
| Mosswarden | Ribbonape | 2.1% | 144 |
| Mosswarden | Galecrest | 2.1% | 144 |
| Quillrat | Bellox | 97.9% | 144 |
| Gloamcat | Mosswarden | 97.9% | 144 |
| Miretoad | Oathhound | 97.9% | 144 |
| Gravemole | Mosswarden | 97.6% | 144 |

The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.

## Reproduce

`./build/tournament 72 reports/gardens-v7-matches.csv 6000`

Content fingerprint: `223a8716`. Rules/observations: v7. The JSON report records source and engine hashes.
