# Terrain and controllable wind — alpha 0.4

Rules/observations v4; content fingerprint `6ed507c8`. The arena now contains reactive ground alongside rocks and persistent spell fields. Ten signatures create ground, elemental attacks transform it, and five signatures can create a timed uniform wind contribution.

## Surfaces and counterplay

| Ground | Combat effect | How it changes |
|---|---|---|
| Water | Extinguishes existing burn at the start of movement; adds 2 to acceleration/braking divisors. Counts as wet for Tidecoil and Voltjack interactions. | Chill freezes it; heat creates steam; shock electrifies it. |
| Ice | 110% travel speed and +5 acceleration/braking divisor: good for committed motion, poor for rapid correction. | Melts to water after 6 seconds, or immediately when hit by heat. |
| Brush | 65% travel speed. Shapes access and escape routes; it is not hidden vision. | Heat ignites it for 4 seconds, after which that patch is consumed. |
| Fire | 4 HP per second to any creature centered in it, including its creator. | Splash turns it into steam. Created fire expires with its authored lifetime. |
| Steam | Projectile velocity retains 92% each tick while starting inside. Drifts with wind. | Condenses to water after 3 seconds, unless its overall lifetime expires first. |
| Charged water | Wet ground plus 3 HP per second to either creature. | Splash discharges it; chill freezes it; heat makes steam. Otherwise returns to water after 4 seconds. |

Hazards pulse on the shared 30-tick cadence. They bypass dodge, guard and shields, like environmental damage; step out of the patch. Enemy damage is credited to the creator or latest transformer, while self-harm never earns damage-dealt credit. No bonus direct-hit passives fire from ground damage.

Elemental projectiles react on contact with a surface, once per surface slot on each flight/return leg. Melee, lunge, beam and nova footprints can react with ground; fields react on their pulse. Surface-creating casts also apply their element over the placement area, so Fresh Spring can extinguish ground fire immediately. A reaction transforms the whole circular patch that overlaps the attack, not individual pixels. Repeated competing elements resolve in stable actor/projectile/zone order; unlike wind addition, chemical reactions are intentionally order-sensitive.

Ground and spell fields are separate layers. Extinguishing ground fire does **not** cancel an ongoing Furnace spell. Surface modifiers apply once per kind at a body center; different kinds can overlap. Planted casts still cancel voluntary drift, and rocks remain solid and indestructible.

## Moves that create and transform the arena

| Creature / move | New ground or wind role |
|---|---|
| Brambleback / Bramble Garden | Creates slowing brush around its defensive setup. |
| Miretoad / Mire Pool | Creates water under its poison field. |
| Rimehare / Frost Lace | Creates ice; its chill-tagged attacks can freeze other water. Whiteout also creates a modest gust. |
| Tidecoil / Rain Basin | Creates water that enables wet-terrain speed/healing; its splash attacks extinguish ground fire. |
| Mosswarden / Shelter Grove | Creates brush around its healing territory. |
| Waxwyrm / Hot Seal | Creates fire; heat can ignite surrounding brush or boil water. |
| Kelpwidow / Tangle Bed | Creates water beneath its control field. |
| Pyrelisk / Furnace | Creates fire; Heat Lance and the rest of its heat-tagged kit transform ground. |
| Orchardboar / Orchard | Creates brush around its persistent garden. |
| Dewotter / Fresh Spring | Creates water at self and splashes existing hazards. |
| Voltjack and Coppergecko | Shock-tagged moves turn water into a dangerous shared conductor. |
| Cinderfox and Ashram | Heat-tagged attacks ignite brush, melt ice, and boil water. |

Exact radii, lifetimes, elements and cast parameters are generated in [the species guide](species.md). The ability's ordinary damage/status effects still apply; the table above describes its new environmental role.

## Wind as a uniform vector field

At any location, wind is the same two-dimensional vector:

`wind = clamp_length(prevailing + active_A + active_B, 24)`

Each creature has one timed contribution. A new wind cast replaces **its own** previous contribution, even if the new one is weaker; it never silently deletes the opponent's contribution. Opposite contributions cancel for their overlapping duration, after which the surviving contribution remains. Timers expiring restore the prevailing weather naturally. Clear weather starts at zero; windy/rainy weather has seeded prevailing x=5–13, y=-4. Positive x is right; positive y is down on screen.

Wind changes three things:

1. **Projectiles:** wind adds to both velocity components each physics tick. Beams are instant geometry and do not bend. The arrow grid shows the field, not a precise forecast of every ricochet or impact.
2. **Travel:** moving with the flow gains speed and moving against it loses speed. The signed projection onto requested movement is multiplied by species wind affinity, capped at ±50% of the current speed. This does not push stationary creatures or override planted casts, and forced dodge/lunge/blink travel retains its own rule.
3. **Steam:** cloud centers move by twice the wind vector each tick, bounded to the arena. A gust can move projectile-damping cover toward or away from a lane.

Galecrest has **240% wind affinity**; most ordinary bodies use 100%, heavy bodies 60%, and selected airborne/acrobatic bodies 160%. Affinity amplifies both benefit and penalty, so creating wind in the wrong direction is a real mistake.

| Wind-producing move | Maximum strength | Duration | Commitment |
|---|---:|---:|---|
| Galecrest / **Windwright** (slot 4, replaces Updraft) | 24 | 6 s | 0.7 s planted startup; 320 stamina; 8 s cooldown |
| Galecrest / Gust Lance | 12 | 3 s | A directional beam also reshapes the field. |
| Rimehare / Whiteout | 10 | 4 s | A local nova with a directional environmental consequence. |
| Ribbonape / Acrobat | 8 | 2.5 s | A small gust accompanies a committed lunge. |
| Inkheron / Takeoff | 12 | 4 s | Blink direction determines the gust it leaves. |

For manual play, **Z/X selects 25%, 50%, 75% or 100% cast strength**, and mouse aim requests the direction. Physical facing locks at acceptance; the field activates only on release. Interrupting startup prevents it. Wind magnitude uses the action aim-vector length (native minimum scalar strength is 1; integer vector projection can round very weak diagonal components to zero). The HUD shows prevailing wind, both contributions, remaining times and the resultant flow arrows. A wind-producing cast announces its direction and strength with its telegraph.

The core sums both released contributions before updating projectiles. Body movement in that tick uses the pre-release field. There is no last-player-wins race for simultaneous opposing wind casts.

## Neutral wind vane and layouts

The wind vane at (12, 2) gives every creature a route to wind control. Hold within 950 integer units of its center **uncontested for 45 ticks / 1.5 seconds**. It creates a strength-16, five-second contribution in the holder's current facing and then recharges for eight seconds. Leaving or contesting resets capture preparation. Taking the vane replaces that creature's current wind contribution. The bloom remains the win objective: travelling north and waiting at the vane sacrifices time controlling the center.

- **Pillars:** paired water patches and central ice that melts after six seconds. Rocks define firing lanes; the ground adds wet, frozen and electrical possibilities.
- **Grove:** paired brush and central water. Burning the brush can open a slow route, while freezing or electrifying the pool changes center access.
- **Open:** offset corner water and a northern ice patch, with no rocks. Useful for isolating projectile curvature and movement effects.

Map surfaces last for the episode unless transformed/consumed; created surfaces have authored finite lifetimes. There are 16 surface slots. A nearby same-kind patch belonging to the same owner is refreshed; otherwise a free slot is allocated. Overflow emits an explicit event/counter after the cast pays its normal cost.

## Validation

The final native suite passes **644,789 assertions** plus **254 environment assertions**. Python and native share golden replay `5e4d1c87cb0a9ade` (seed 77, rain, Gravemole/Kelpwidow, grove, 100 decisions). Most native assertions are repeated numerical/state checks.

The final **56,160-match** scripted diagnostic spans **22.4%–76.5%** aggregate species win rates, with **51.88%** seat-A wins, **22.38-second** mean fights and **zero pool overflows**. These are current-content measurements, not competitive certification or an independent trained-policy holdout. See the generated [current sweep](../../reports/terrain-v4-balance.md). All 40 species and their ordinary move/passive contracts remain tested, plus a dedicated environment suite covering:

- Every surface-creating and wind-producing signature, cast strength/duration, elemental projectile/beam/field reactions, expiry, steam drift/drag, water extinguishing and ice momentum.
- Two-axis uniform projectile acceleration, headwind/tailwind movement, opposing wind cancellation, independent expiry and simultaneous opposing casts.
- Vane capture, contest, leaving reset and cooldown; friendly ground hazard damage and attribution.
- Snapshot/fork parity with active wind and transformed ground, bounded-pool overflow, and atomic rejection of malformed new fields.

Rules v4 expands observations to **3,620 floats**: 69 entity rows include all surface slots; move tokens expose terrain/element/wind parameters; globals expose both contributions and vane progress. The updated **82,627-parameter** recurrent policy and Gymnasium wrapper pass integration checks. [Exact schema](../integration.md).

The scripted pilot avoids hazardous ground, gives Galecrest a vane-seeking condition, and estimates projectile wind curvature from public vectors and expiry. It does not deliberately optimize every elemental combo or learn wind strategy. Auxiliary wind on a dash/blink is a bonus to an otherwise viable mobility use, not a reason to spend it blindly. No trained policy, skill ceiling or competitive balance is claimed.
