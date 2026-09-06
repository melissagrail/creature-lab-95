# Scripted balance evidence

**56,160 matches; 2,808 games per species; 72 games per unordered matchup.** Every unordered pairing is run in both seats. The 36-seed protocol covers 3 weather states × 3 arenas × 4 style pairings. The species share a kit-aware heuristic; these are not independently trained or optimized policies.

Aggregate species win rates: **13.3%–87.0%**. Side A wins: **52.19%**. Mean fight: **44.7s**; median **38.6s**. Pool overflows: **0**.

Endings: KO 48,789, control 928, time limit 6,443.

These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.

| Species | Win rate | Games |
|---|---:|---:|
| Quillrat | 87.0% | 2,808 |
| Flintroc | 78.3% | 2,808 |
| Coppergecko | 76.2% | 2,808 |
| Gravemole | 75.2% | 2,808 |
| Hooklynx | 75.1% | 2,808 |
| Pyrelisk | 72.0% | 2,808 |
| Ribbonape | 71.4% | 2,808 |
| Ashram | 67.2% | 2,808 |
| Glasswing | 67.1% | 2,808 |
| Latchspider | 64.8% | 2,808 |
| Rimehare | 63.4% | 2,808 |
| Inkheron | 63.0% | 2,808 |
| Slagjaw | 60.5% | 2,808 |
| Kelpwidow | 60.0% | 2,808 |
| Clockfin | 55.4% | 2,808 |
| Brambleback | 54.1% | 2,808 |
| Tidecoil | 54.1% | 2,808 |
| Cinderfox | 53.5% | 2,808 |
| Galecrest | 51.0% | 2,808 |
| Miretoad | 50.7% | 2,808 |
| Ironmoth | 50.2% | 2,808 |
| Duneskink | 46.5% | 2,808 |
| Gloamcat | 45.7% | 2,808 |
| Nullurchin | 44.7% | 2,808 |
| Echofin | 44.5% | 2,808 |
| Thornmantis | 43.6% | 2,808 |
| Sunstag | 42.1% | 2,808 |
| Orchardboar | 40.5% | 2,808 |
| Nectarbat | 39.7% | 2,808 |
| Dewotter | 37.7% | 2,808 |
| Prismray | 37.1% | 2,808 |
| Basaltusk | 35.8% | 2,808 |
| Anvilnewt | 35.2% | 2,808 |
| Waxwyrm | 33.7% | 2,808 |
| Voltjack | 26.5% | 2,808 |
| Bellox | 22.4% | 2,808 |
| Mooncalf | 21.7% | 2,808 |
| Saltcrab | 20.3% | 2,808 |
| Oathhound | 19.1% | 2,808 |
| Mosswarden | 13.3% | 2,808 |

## Largest matchup asymmetries

| Species A | Species B | A win rate | Games |
|---|---|---:|---:|
| Dewotter | Flintroc | 0.0% | 72 |
| Dewotter | Latchspider | 0.0% | 72 |
| Slagjaw | Oathhound | 100.0% | 72 |
| Coppergecko | Oathhound | 100.0% | 72 |
| Bellox | Orchardboar | 0.0% | 72 |
| Saltcrab | Coppergecko | 0.0% | 72 |
| Ribbonape | Oathhound | 100.0% | 72 |
| Mosswarden | Slagjaw | 0.0% | 72 |
| Mosswarden | Hooklynx | 0.0% | 72 |
| Mosswarden | Coppergecko | 0.0% | 72 |
| Mosswarden | Ashram | 0.0% | 72 |
| Mosswarden | Galecrest | 0.0% | 72 |
| Gravemole | Mosswarden | 100.0% | 72 |
| Quillrat | Saltcrab | 100.0% | 72 |
| Quillrat | Waxwyrm | 100.0% | 72 |
| Quillrat | Mosswarden | 100.0% | 72 |
| Quillrat | Thornmantis | 100.0% | 72 |
| Rimehare | Mosswarden | 100.0% | 72 |
| Basaltusk | Hooklynx | 0.0% | 72 |
| Basaltusk | Quillrat | 0.0% | 72 |

The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.

## Reproduce

`./build/tournament 36 reports/energy-v6.csv 2000`

Content fingerprint: `223a8716`. Rules/observations: v5. The JSON report records source and engine hashes.
