# Scripted balance evidence

**56,160 matches; 2,808 games per species; 72 games per unordered matchup.** Every unordered pairing is run in both seats. The 36-seed protocol covers 3 weather states × 3 arenas × 4 style pairings. The species share a kit-aware heuristic; these are not independently trained or optimized policies.

Aggregate species win rates: **22.4%–76.5%**. Side A wins: **51.88%**. Mean fight: **22.4s**; median **19.8s**. Pool overflows: **0**.

Endings: KO 55,317, control 735, time limit 108.

These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.

| Species | Win rate | Games |
|---|---:|---:|
| Quillrat | 76.5% | 2,808 |
| Thornmantis | 74.0% | 2,808 |
| Coppergecko | 73.2% | 2,808 |
| Hooklynx | 70.9% | 2,808 |
| Saltcrab | 66.2% | 2,808 |
| Inkheron | 66.0% | 2,808 |
| Waxwyrm | 65.3% | 2,808 |
| Tidecoil | 63.6% | 2,808 |
| Gravemole | 60.4% | 2,808 |
| Pyrelisk | 58.5% | 2,808 |
| Gloamcat | 57.5% | 2,808 |
| Brambleback | 54.5% | 2,808 |
| Clockfin | 53.9% | 2,808 |
| Dewotter | 53.6% | 2,808 |
| Sunstag | 53.4% | 2,808 |
| Nullurchin | 53.1% | 2,808 |
| Mooncalf | 52.8% | 2,808 |
| Ribbonape | 52.5% | 2,808 |
| Slagjaw | 51.3% | 2,808 |
| Mosswarden | 50.4% | 2,808 |
| Glasswing | 49.7% | 2,808 |
| Oathhound | 48.7% | 2,808 |
| Latchspider | 48.5% | 2,808 |
| Cinderfox | 48.5% | 2,808 |
| Duneskink | 46.7% | 2,808 |
| Rimehare | 46.4% | 2,808 |
| Voltjack | 46.1% | 2,808 |
| Bellox | 46.0% | 2,808 |
| Nectarbat | 42.8% | 2,808 |
| Kelpwidow | 42.1% | 2,808 |
| Orchardboar | 35.8% | 2,808 |
| Basaltusk | 35.1% | 2,808 |
| Ironmoth | 34.8% | 2,808 |
| Flintroc | 34.6% | 2,808 |
| Ashram | 34.5% | 2,808 |
| Galecrest | 34.2% | 2,808 |
| Echofin | 33.8% | 2,808 |
| Miretoad | 32.0% | 2,808 |
| Prismray | 29.8% | 2,808 |
| Anvilnewt | 22.4% | 2,808 |

## Largest matchup asymmetries

| Species A | Species B | A win rate | Games |
|---|---|---:|---:|
| Mosswarden | Inkheron | 0.0% | 72 |
| Mosswarden | Waxwyrm | 0.0% | 72 |
| Thornmantis | Anvilnewt | 100.0% | 72 |
| Thornmantis | Nullurchin | 100.0% | 72 |
| Prismray | Thornmantis | 0.0% | 72 |
| Gravemole | Anvilnewt | 100.0% | 72 |
| Quillrat | Flintroc | 100.0% | 72 |
| Quillrat | Slagjaw | 100.0% | 72 |
| Quillrat | Orchardboar | 100.0% | 72 |
| Quillrat | Anvilnewt | 100.0% | 72 |
| Ironmoth | Inkheron | 0.0% | 72 |
| Basaltusk | Thornmantis | 0.0% | 72 |
| Basaltusk | Quillrat | 0.0% | 72 |
| Miretoad | Dewotter | 0.0% | 72 |
| Miretoad | Sunstag | 0.0% | 72 |
| Miretoad | Quillrat | 0.0% | 72 |
| Brambleback | Inkheron | 0.0% | 72 |
| Brambleback | Bellox | 100.0% | 72 |
| Brambleback | Waxwyrm | 0.0% | 72 |
| Brambleback | Sunstag | 100.0% | 72 |

The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.

## Reproduce

`./build/tournament 36 reports/terrain-v4.csv 2000`

Content fingerprint: `6ed507c8`. Rules/observations: v4. The JSON report records source and engine hashes.
