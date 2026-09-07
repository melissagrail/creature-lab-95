# Rules-8 pilots and campaign validation — alpha 0.10

The release controller (`d9ff10f9`) scores **73.06%** on 1,440 fresh native matches, compared with **46.25%** for the explicitly migrated rules-7 weights under the same final rules-8 physics. This is a **26.81 percentage-point** improvement on this cohort. It is not a comparison across different combat rules.

## Training and selection

Three local recurrent PPO runs collected **15,728,640 new decisions**. The selected checkpoint inherits 15,073,280 of those decisions through the selected intermediate checkpoints. The first run introduced the footwork cost; the two later runs included the final cornering constraint. All retained weights have 89,959 parameters, a 96-value recurrent state, species embeddings, conditional ability controls and temperament inputs.

The final run trained 7,864,320 decisions, starting from the retained `cornering-v10` checkpoint. It sampled all forty species, mixed temperaments on 55% of episodes, and used frozen learned opponents on 25% of episodes alongside scripted styles. The 960-update checkpoint won the fixed steady validation set with a 72.5% score. Selection happened before the fresh native test. The smaller Python held-out set also scored 72.5% (240 matches); it is reported separately and was not used to reselect.

[Selection and immutable checkpoint mapping](rl-v10-selection.json) records the exact run seeds, selected steps and hashes. [Training logs, configs and runtime](training-v10/) retain all three runs. The original rules-7 model, the weights-only migrated baseline, and both intermediate selected models remain in `models/baselines`. The migration itself is not counted as training.

## Fresh native evaluation

Seed base `2160000000`, 720 crossed scenarios per temperament, both seats: every species appears on all six arenas in all three weather states. Four scripted opponent styles and changing opponent species are included. Each preset has 1,440 matches; the full selected-model test has 7,200. Score awards one for a win, one-half for a draw and zero for a loss. All matches completed without a capacity overflow.

| Controller / preset | Games | Score | Wins | Draws | Mean seconds | Mean energy | Mean separation | Stationary |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Migrated old weights / steady | 1,440 | 46.25% | 665 | 2 | 37.6 | 22.5 / 100 | 3.62 | 22.2% |
| Steady | 1,440 | 73.06% | 1051 | 2 | 34.1 | 61.4 / 100 | 2.48 | 12.2% |
| Aggressive | 1,440 | 74.83% | 1076 | 3 | 33.4 | 66.0 / 100 | 2.36 | 11.6% |
| Skittish | 1,440 | 69.44% | 1000 | 0 | 36.8 | 60.0 / 100 | 2.57 | 12.8% |
| Patient | 1,440 | 63.06% | 907 | 2 | 37.8 | 75.5 / 100 | 2.29 | 10.8% |
| Territorial | 1,440 | 74.79% | 1076 | 2 | 34.0 | 65.6 / 100 | 2.35 | 11.9% |

Energy and separation are per-match means; stationary means low translational speed, not necessarily a planted cast. Personality names describe training intent. Patient holds more energy but remains weaker than the other presets. Skittish keeps slightly more separation than Aggressive. Territorial does not hold closer to the objective in this cohort (5.61 versus Steady's 5.26 mean distance), so that aspect still needs work. These outcomes do not establish equal power or faithful personality expression in every species.

[Selected-model raw matches](brain-v10-heldout.csv), [migrated-baseline raw matches](brain-v10-warmstart.csv). Earlier development-cohort tests are retained as `brain-v10-cornering.csv` and `brain-v10-warmstart-cornering-cohort.csv`; they were not used to select the final journey-run checkpoint.

## Species coverage

38 of forty species improve on the paired steady cohort. Each row contains only 36 matches per controller, so individual rows are diagnostic rather than balance certification. A strong shared controller is not evidence that the underlying forty kits are equally powerful.

| Species | Old weights | Selected | Change (points) |
| --- | ---: | ---: | ---: |
| Cinderfox | 38.9% | 91.7% | +52.8 |
| Brambleback | 38.9% | 80.6% | +41.7 |
| Glasswing | 29.2% | 61.1% | +31.9 |
| Voltjack | 5.6% | 55.6% | +50.0 |
| Miretoad | 22.2% | 55.6% | +33.3 |
| Basaltusk | 25.0% | 72.2% | +47.2 |
| Rimehare | 69.4% | 75.0% | +5.6 |
| Gloamcat | 63.9% | 91.7% | +27.8 |
| Ironmoth | 41.7% | 83.3% | +41.7 |
| Tidecoil | 63.9% | 97.2% | +33.3 |
| Quillrat | 55.6% | 61.1% | +5.6 |
| Sunstag | 72.2% | 97.2% | +25.0 |
| Gravemole | 25.0% | 58.3% | +33.3 |
| Prismray | 30.6% | 76.4% | +45.8 |
| Thornmantis | 77.8% | 94.4% | +16.7 |
| Mosswarden | 19.4% | 41.7% | +22.2 |
| Galecrest | 86.1% | 100.0% | +13.9 |
| Ashram | 55.6% | 94.4% | +38.9 |
| Clockfin | 33.3% | 69.4% | +36.1 |
| Nullurchin | 25.0% | 27.8% | +2.8 |
| Waxwyrm | 8.3% | 16.7% | +8.3 |
| Ribbonape | 80.6% | 91.7% | +11.1 |
| Saltcrab | 50.0% | 86.1% | +36.1 |
| Nectarbat | 80.6% | 100.0% | +19.4 |
| Bellox | 36.1% | 55.6% | +19.4 |
| Anvilnewt | 11.1% | 33.3% | +22.2 |
| Duneskink | 69.4% | 97.2% | +27.8 |
| Kelpwidow | 44.4% | 44.4% | +0.0 |
| Pyrelisk | 58.3% | 58.3% | +0.0 |
| Mooncalf | 19.4% | 55.6% | +36.1 |
| Coppergecko | 63.9% | 88.9% | +25.0 |
| Orchardboar | 33.3% | 50.0% | +16.7 |
| Inkheron | 52.8% | 77.8% | +25.0 |
| Hooklynx | 77.8% | 100.0% | +22.2 |
| Slagjaw | 51.4% | 86.1% | +34.7 |
| Dewotter | 52.8% | 86.1% | +33.3 |
| Echofin | 25.0% | 61.1% | +36.1 |
| Latchspider | 61.1% | 91.7% | +30.6 |
| Flintroc | 47.2% | 73.6% | +26.4 |
| Oathhound | 47.2% | 83.3% | +36.1 |

## Campaign playthroughs

The native playthrough driver uses actual engine outcomes, ordinary thread offerings, free sanctuary rests and legal remedies. It changes owned party members after losses; after repeated losses it also selects steady/aggressive temperaments and an unlocked Stone charm. This models available preparation choices; it does not grant wins, extra health, currency or unearned charms. It does not simulate human navigation, reading or puzzle solving.

| Starter | Bells | Friends | Encounters | Losses | Simulated combat minutes |
| --- | ---: | ---: | ---: | ---: | ---: |
| Cinderfox | 8 | 40 | 83 | 6 | 109.2 |
| Rimehare | 8 | 40 | 83 | 5 | 103.3 |
| Dewotter | 8 | 40 | 81 | 2 | 106.3 |

All eight quiet Lantern Walks also completed with the Cinderfox-route collection, taking one to three attempts each and 46.9 combined simulated combat minutes. All eight restless routes also completed in one to four attempts each, totaling 145.3 simulated combat minutes. That tester invited healthy reserves between rooms using the normal spirit-book controls, in addition to preparation and legal remedies. See [restless results](campaign-restless-v10.csv.walks.csv). A fixed-three-party version cleared only three of eight within six attempts; [that report](campaign-restless-v10-fixed-party.csv.walks.csv) is retained. The difference demonstrates the value of a broad collection, and the UI now makes bench vitality and reserve invitations explicit.

The initial fixed-temperament preparation harness stalled on the Dewotter route and the Hearthmere expedition. Those failures remain in [the initial route report](campaign-playthrough-v10-initial.csv) and [initial expedition report](campaign-playthrough-v10-initial.csv.walks.csv). Adding legal temperament and charm choices to the tester resolved those routes without changing the shipped controller or combat stats. This is also a usability finding: the game now points players toward those preparation choices after a loss.

[Final main-route results](campaign-playthrough-v10.csv), [quiet expedition results](campaign-playthrough-v10.csv.walks.csv). Both endings and serialized state are independently validated. **These times do not establish a twenty-hour human playthrough.** The current authored campaign is smaller than that target.

## Validation and limits

The deterministic simulation, terrain, model and campaign checks pass, including the rules-8 golden `03b73ddd44073999`, save corruption/backup handling, all-site reachability, acquisition, charms and expedition transitions. Python/native inference agrees over 480 decisions across all forty species (maximum float difference 4.3e-6; identical quantized actions in this test). The engine and campaign also pass AddressSanitizer/UndefinedBehaviorSanitizer checks.

SDL fixtures cover all nineteen campaign screens/states, including expedition results, and verify all 1,280 animation cells. The native controller harness exercises a real learned-pilot encounter and campaign menu/save flow. The relocated Mac app includes SDL2 and both models and needs neither Python nor a local checkout. Native desktop UI automation was unavailable; a human usability pass is still required.

No claim is made of expert play, universal counterplay, balanced personality power, full self-play robustness or automatic individual learning during the campaign. Neural inference is floating point; the integer simulation and recorded actions provide deterministic replay. Campaign remedies are explicit scenario interventions, and campaign combat is not silently exported as ordinary replay or PPO training data.
