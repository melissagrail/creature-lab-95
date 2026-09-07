# The Unwritten Road — campaign alpha 0.10

The combat laboratory now has a complete campaign route around it: an opening, eight regions, forty obtainable companions, persistent progression, a final decision and postgame exploration. This is a playable campaign alpha. **Twenty hours remains the intended release-length experience; it is not a measured duration of the current content.**

## The road

The sanctuary bells once carried names between villages. Keeper Nara tried to repair their failures by binding each bell to another. Their promises now converge at Hush Crown, where a spirit has become the keeper of everyone else's certainty. You travel with a field book, meet the spirits whose lives were changed, and decide how the road should work after the old arrangement ends.

| Region | Geography and character | Combat emphasis |
| --- | --- | --- |
| Hearthmere | Mossy village, river and shrine paths | Breath, planted casts, brush and fire |
| Reedglass Fen | Wetland islands and wooden crossings | Currents, cleansing and persistent terrain |
| Copperwind Rise | Terraces and a winding cliff road | Wind direction, approach angles and mobility |
| Orchard of Names | Blossom groves, hedges and a looping road | Prepared space, pressure and control |
| Emberkiln | A route around a basalt crater | Oil, ignition and committed close exchanges |
| Winter Archive | Frozen pools and an archival town | Inertia, braking and patient timing |
| Inkshore | Diagonal coastline, piers and islands | Concealment, returns and delayed effects |
| Hush Crown | Broken islands joined by narrow paths | Mixed terrain, wind and party adaptation |

Each region contains twenty authored sites: a sanctuary, five species habitats, four two-spirit road trials, three main conversations, a village request, a memory, a cache, a bell riddle, a three-spirit keeper, an onward gate and an optional Lantern Walk. Roads and sites have different arrangements in every region. All 160 locations are checked for reachability. The world has visible encounters; walking through grass never launches a random battle.

The two final choices lead to different epilogues: shared keepership through an open circle, or an unbound road without a permanent keeper. Both leave the world available for collection and optional play.

## Companions and preparation

Choose Cinderfox, Rimehare or Dewotter. Every species is available later regardless of the starter. A new journey creates a stored seed for individual temperaments; `--seed N` reproduces one deliberately.

A completed first friendly encounter earns one trust mark even in defeat. Retreating does not. Further victories earn additional marks, or three memory threads buy one mark after recognition. Species need one, two or three marks according to their habitat tier. There are no capture probabilities. The spirit book holds every acquired species; three travel in the active party.

Bond experience is shared by the party after encounters. It records familiarity without increasing base HP or damage: the forty combat kits keep their common power budget. Bond rank 2 opens starting charms. Village requests add three further options. Temperaments can be changed in the spirit book, and each companion retains a small reproducible individual variation. This changes controller input; it does not retrain or edit the species' physics.

| Charm | Starting tradeoff | Unlock |
| --- | --- | --- |
| Open Hand | Full 100 energy | Always |
| Stone | 12 shield for 15 seconds; 85 energy | Bond 2 |
| Wind | Three seconds of haste; 85 energy | Bond 2 |
| Reed | 30% recovery between victorious relay rounds instead of 15%; 85 energy | Fen request + bond 2 |
| Bell | Five seconds of control resistance; 80 energy | Archive request + bond 2 |
| Lantern | 24 shield for 15 seconds; 65 energy | Hearthmere request + bond 2 |

A sanctuary fully recovers every companion and ensures at least three remedies. Two threads craft an extra remedy. Each remedy heals one-third of maximum vitality and can be used once per duel round. They are manual decisions even while a spirit pilots itself.

Relays retain party condition. A defeated companion yields to the next available party member; the enemy keeps its remaining vitality. A victorious companion recovers 15% before the next opponent, or 30% with Reed. Energy and cooldowns reset between rounds. A total defeat returns the party to the sanctuary and recovers it freely. No companion, thread or equipment is permanently lost.

## Lantern Walks

A restored keeper opens an optional eight-room expedition. At each room choose a quiet one-spirit encounter or a restless two-spirit relay with a different garden and potentially different weather. Later keepers (from the Orchard onward) and restless Lantern rooms use learned opposing pilots with regional temperaments. Earlier road opponents use the four scripted styles. Restless paths award three threads per opposing spirit instead of two; the garden and weather are shown before choosing. Both final paths lead to a three-spirit relay. Two campfire opportunities trade two threads for 35% party recovery, one remedy and passage through that room.

Individual condition persists, and preparation is available between rooms. The field book travels with you: you may invite fresh owned spirits at a fork, so a broad collection has practical value. Switching out an injured spirit does not heal it; the book shows every companion's remaining vitality. Finishing awards a regional seal and twelve extra threads. Leaving returns home with earned rewards. These are repeatable encounter routes; they are not eight additional authored story campaigns. The current quiet/restless reward tradeoffs and long-run difficulty need more human testing.

## Controls and inspection

Walk with WASD/arrows or click a destination. Clicking the local map can plan a longer walk; clicking a nearby marked location walks there and interacts. Keyboard movement cancels the planned route. Shift hurries. E or Enter interacts with the nearest site.

B opens companions, Tab the travel atlas, J the journal, Escape the field guide. The companion book exposes exact move costs, timings, effects and physical characteristics under **Arts and Field Notes**. The title screen's **Sprite Studio** displays all thirty-two frames of any species and compares its world scale.

In combat, M toggles learned pilot/manual control; WASD moves, mouse aims, 1–4 request arts, Space dodges. P pauses, R uses a remedy, H shows collision geometry, Z/X change wind strength, Escape retreats. The central garden's control bars provide an alternative win route. F8 toggles the original procedural soundtrack. The music accelerates for battle and varies its root by region.

`Combat Lab.command` or `--workbench` opens the original debugging/research environment, including both skins, arbitrary matchups, snapshots and hash-checked replays. Campaign remedies and progression are external scenario interventions, so campaign play is not silently presented as an ordinary replay or PPO dataset.

## Saves

Campaign saves use the independent `TINISAV2` envelope, fixed little-endian fields, length validation and an FNV checksum. They are separate from combat snapshots and model files. Load validates into a candidate before replacing live state. Save writes a temporary file, preserves a validated `.bak`, then replaces the current file. A corrupt current file cannot overwrite a valid backup.

On macOS the default location is:

```text
~/Library/Application Support/Tinikami/The Unwritten Road/journey.tini
```

Linux and Windows use SDL's corresponding per-user application-data directory. `--save-path PATH` selects a deliberate alternate slot. Starting a new journey archives any existing primary file, including an unreadable one, before writing. Save failures are visible. F5 saves outside combat; the world also autosaves and progression writes immediately.

Battles resume from the saved pre-encounter checkpoint. Lantern Walks resume at their current room choice. The game does not serialize a half-finished duel or recurrent controller memory into the campaign save. Unfocused time and paused battle time do not advance the playtime counter.

## Architecture and validation

`include/creature/campaign.hpp` and `src/campaign.cpp` contain progression, acquisition, party composition, preparation, scenario setup, geography access and save validation. `content/campaign.json` is the authored story/site source; `scripts/compile_campaign.py` and `scripts/campaign_geography.py` compile immutable C++ tables. The runtime does not need a JSON parser.

`client/journey.hpp` supplies SDL presentation and input. Duel actions still pass through `creature::step`; effects, collision and results are never driven by sprite timing. Campaign scenario setup uses existing observable surface/wind/status fields. The core C ABI and native brain runtime remain independent of campaign art and UI.

Validation includes connected-map checks, all-species acquisition, both endings, request/charms, expedition/save transitions, corruption and backup tests, a native campaign controller harness with a real learned-pilot encounter, all rendered screen fixtures, and full combat playthroughs from all three starters. The playthrough driver uses actual results and legal remedies; after losses it changes party composition, and repeated losses prompt legal temperament and charm choices. It does not grant victory or extra battle stats.

All three prepared starter routes restored all eight bells and befriended all forty species, with 103–109 minutes of simulated combat each. All eight quiet Lantern Walks also completed, as did all eight restless routes when the tester used fresh reserves between rooms. The [validation report](../../reports/rl-v10-summary.md) retains both successful runs and an earlier fixed-temperament harness that stalled.

The automated runs measure combat simulation time, not an average human playthrough. Native desktop UI automation was unavailable during this pass; input/controller code and headless SDL renders were exercised, but a human usability pass is still needed.

## The twenty-hour target

The release-length structure is eight chapters at roughly two hours each, with four hours for personal collection choices, optional requests and an expedition or two. That is a **content budget**, not a reason to slow walking, require repeated losses or inflate enemy health.

Before calling this a twenty-hour game, the road needs more authored local journeys and interactions between its existing landmarks, deeper village request chains, more varied keeper encounters, additional exploration puzzles, and playtests that measure navigation, reading, party decisions and retries separately. The current 160-site route establishes the structure, economy and story arc. It is smaller than that intended release-length experience.

Other alpha limits: one companion of each species, no nickname editor, four directional rows rather than eight, one shared attack pose per direction across a species' four arts, manual remedy use, no online services, and no automatic in-game weight updates. The macOS release is ad hoc signed rather than notarized.
