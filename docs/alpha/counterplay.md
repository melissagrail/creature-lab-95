# Counterplay follow-up and alpha risks

Content `7def5f00`, seed offsets 4000–4001. The ten pairings below were selected because the broad holdout exposed extreme outcomes. This is a diagnostic follow-up, not another unbiased holdout.

Each pairing has **576 matches**: all 16 scripted-style combinations × 3 arenas × 3 weather states × 2 seeds × both seats. Total: **5,760 matches**. Each cell averages the named creature's style against all four enemy styles. Draws count as half a win.

The four styles share the same kit-aware action heuristic. Style 1 prefers 75% of ordinary spacing, style 2 prefers 115%, and style 3 emphasizes the bloom. They do not constitute four independently learned strategies.

| Pairing (rate for first species) | Default | Closer | Farther | Bloom |
|---|---:|---:|---:|---:|
| Miretoad vs Quillrat | 1.4% | 0.0% | 0.7% | 0.7% |
| Miretoad vs Dewotter | 0.3% | 0.0% | 0.0% | 0.7% |
| Miretoad vs Gravemole | 2.8% | 1.4% | 0.7% | 0.7% |
| Brambleback vs Ironmoth | 6.9% | 7.3% | 8.0% | 16.3% |
| Voltjack vs Bellox | 65.3% | 65.3% | 88.9% | 73.6% |
| Bellox vs Flintroc | 90.3% | 86.1% | 87.5% | 89.9% |
| Saltcrab vs Anvilnewt | 81.9% | 69.4% | 93.1% | 97.9% |
| Basaltusk vs Gravemole | 9.7% | 0.0% | 4.2% | 4.2% |
| Ashram vs Anvilnewt | 88.9% | 96.9% | 88.9% | 93.4% |
| Bellox vs Slagjaw | 35.4% | 17.4% | 33.7% | 8.7% |

## What remains unresolved

- **Miretoad access/payoff:** its three worst pairings remain near zero under every available spacing style. Poison application, staying alive until poison pays, and cleanse denial need targeted human and learned-policy scenarios. This result does not identify which mechanic is causal. Do not ship ranked play on the assumption that its aggregate rate is sufficient.
- **Brambleback and Basaltusk access:** bloom pressure improves Brambleback versus Ironmoth but does not restore an even fight; Basaltusk versus Gravemole remains highly skewed. Measure successful engage windows, displacement denied by mass, and damage taken per attempted entry before buffing raw health.
- **Bellox behavior sensitivity:** Bellox versus Slagjaw improves from 8.7% to 35.4% when switching from bloom priority to default spacing. Voltjack's advantage over Bellox is also smaller under several styles than in the original holdout. These are evidence that pilot behavior matters, not evidence the matchups are solved.
- **Setup/attrition denial:** Bellox versus Flintroc, Saltcrab versus Anvilnewt, and Ashram versus Anvilnewt remain strongly favored across these averages. Evaluate setup survival, repeat guard/repair value and punish opportunities with species-specific pilots.

## Next playtest gates

1. Human-controlled matchup reversals: record action replays in these ten pairings, alternate sides and arenas, and identify a repeatable counterplan or mark the interaction for redesign. An aggregate roster buff is the wrong first experiment.
2. Train several seeds of species-specific and shared semantic policies; freeze opponents for evaluation and hold out both policies and scenarios. Track exploitability against best responses, not only self-play win rate.
3. Instrument the field guide's species learning tests: setup conversion, access cost, conditional payoff, resource waste and defensive timing. Confirm that a policy is winning through the intended interaction rather than a universal stall or spam tactic.
4. Before competitive certification, agree on a matchup tolerance and demonstrate meaningful response options in every extreme pair. This alpha intentionally makes no such certification.

Reproduce from the repository root:

```sh
make build/counterplay
./build/counterplay
```

Raw results: [counterplay.csv](../../reports/counterplay.csv). Broad evidence: [fresh-seed holdout](../../reports/holdout-balance.md).
