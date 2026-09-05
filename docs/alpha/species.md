# Alpha species field guide

All 40 species and 160 signature moves below are executable content. Shared dodge is slot 5. Times are simulation ticks at 30 Hz; range/radius are game units. Axis scores are design intent, not measured power.

| # | Species | Role | Passive | HP | Speed (units/s) |
|---|---|---|---|---:|---:|
| 1 | [Cinderfox](#cinderfox) | burn detonator | Cinder | 113 | 4.98 |
| 2 | [Brambleback](#brambleback) | stationary bastion | Anchor | 136 | 3.63 |
| 3 | [Glasswing](#glasswing) | minimum-range artillery | Focus | 94 | 4.42 |
| 4 | [Voltjack](#voltjack) | mark-and-discharge diver | Conduit | 106 | 4.95 |
| 5 | [Miretoad](#miretoad) | stacking attrition | Venom | 123 | 3.96 |
| 6 | [Basaltusk](#basaltusk) | wall-slam juggernaut | Bulwark | 141 | 3.66 |
| 7 | [Rimehare](#rimehare) | spacing and chill | Frost | 131 | 5.57 |
| 8 | [Gloamcat](#gloamcat) | positional assassin | Backstab | 100 | 5.27 |
| 9 | [Ironmoth](#ironmoth) | projectile counterfighter | Mirror | 116 | 4.28 |
| 10 | [Tidecoil](#tidecoil) | pull-and-pool control | Rainborn | 149 | 4.51 |
| 11 | [Quillrat](#quillrat) | visible trap planner | Trapper | 124 | 4.89 |
| 12 | [Sunstag](#sunstag) | cleanse-and-tempo skirmisher | Renewal | 117 | 4.78 |
| 13 | [Gravemole](#gravemole) | delayed ambusher | Ambush | 111 | 4.48 |
| 14 | [Prismray](#prismray) | ricochet geometry | Ricochet | 104 | 4.42 |
| 15 | [Thornmantis](#thornmantis) | wound pursuit duelist | Wounder | 113 | 5.07 |
| 16 | [Mosswarden](#mosswarden) | territory sustain | Sanctuary | 118 | 3.72 |
| 17 | [Galecrest](#galecrest) | wind mobility | Tailwind | 115 | 5.30 |
| 18 | [Ashram](#ashram) | health-spending berserker | Berserk | 120 | 4.63 |
| 19 | [Clockfin](#clockfin) | alternating-combo engine | Cadence | 129 | 4.54 |
| 20 | [Nullurchin](#nullurchin) | anti-shield disruptor | Nullify | 132 | 3.81 |
| 21 | [Waxwyrm](#waxwyrm) | destructible turret engineer | Architect | 102 | 4.01 |
| 22 | [Ribbonape](#ribbonape) | return-path fighter | Returner | 124 | 5.04 |
| 23 | [Saltcrab](#saltcrab) | resource fortress | Reservoir | 175 | 3.60 |
| 24 | [Nectarbat](#nectarbat) | access-dependent lifesteal | Leech | 105 | 5.19 |
| 25 | [Bellox](#bellox) | three-hit resonator | Resonance | 125 | 4.16 |
| 26 | [Anvilnewt](#anvilnewt) | shield-to-offense converter | Forge | 104 | 4.07 |
| 27 | [Duneskink](#duneskink) | travel-charged skirmisher | Skirmish | 108 | 5.57 |
| 28 | [Kelpwidow](#kelpwidow) | ranged tether controller | Tether | 145 | 4.19 |
| 29 | [Pyrelisk](#pyrelisk) | heat-risk siege mage | Overheat | 111 | 4.31 |
| 30 | [Mooncalf](#mooncalf) | stored retaliation | Retaliate | 102 | 4.31 |
| 31 | [Coppergecko](#coppergecko) | three-cast resource recycler | Recycle | 125 | 4.78 |
| 32 | [Orchardboar](#orchardboar) | delayed garden harvest | Harvest | 132 | 4.01 |
| 33 | [Inkheron](#inkheron) | projectile cover architect | Cover | 121 | 4.72 |
| 34 | [Hooklynx](#hooklynx) | marked catch specialist | Hunter | 152 | 4.89 |
| 35 | [Slagjaw](#slagjaw) | anti-sustain bruiser | Corrode | 140 | 4.31 |
| 36 | [Dewotter](#dewotter) | reactive evasive cleanser | Purify | 101 | 5.21 |
| 37 | [Echofin](#echofin) | timed cast cadence | Rhythm | 93 | 4.57 |
| 38 | [Latchspider](#latchspider) | web-network mover | Web | 150 | 4.75 |
| 39 | [Flintroc](#flintroc) | charged first-strike artillery | Magazine | 104 | 4.25 |
| 40 | [Oathhound](#oathhound) | guard-and-riposte sentinel | Resolve | 114 | 4.19 |

## Cinderfox

A glassy fox that cashes out burn windows.

**Body:** 113 HP; 4.98 units/s; 0.41 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 360 degrees/s; strafe 82%; reverse 60%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 100%.

**Winning pattern:** Tag with Wickshot, approach under flare, cash out burn with Kindlebite.

**Counterplay:** Dodge the tag and re-engage after the burn expires; the fox has no heal.

**Learning test:** Withhold the finisher until burn is active; stop chasing through enemy terrain.

**Axes (1–5):** reach 3, commitment 4, mobility 4, control 1, persistence 3, sustain 1, defense 1, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Kindlebite | Melee | 19 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | burn=30, move_start=55, move_active=55, move_recovery=60, element=1 |
| 2. Wickshot | Bolt | 14 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, burn=90, move_recovery=30, element=1 |
| 3. Flare Step | Lunge | 14 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, haste=45, move_recovery=25, element=1 |
| 4. Ash Circle | Field | 3 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | burn=45, lifetime=120, move_recovery=30, element=1, pulse every 24t |

## Brambleback

A rooted hedgehog that becomes expensive to evict.

**Body:** 136 HP; 3.63 units/s; 0.62 collider radius; mass 145; stamina regeneration 180/s.

**Locomotion:** turns 120 degrees/s; strafe 48%; reverse 35%; acceleration divisor 4; braking divisor 2; dodge speed 80%; wind affinity 60%.

**Winning pattern:** Set a garden, stop moving to grow a shield, use roots to hold the bloom.

**Counterplay:** Force relocation with remote damage; do not fight inside the garden.

**Learning test:** Choose when immobility is worth its defenses.

**Axes (1–5):** reach 2, commitment 2, mobility 1, control 4, persistence 5, sustain 3, defense 5, execution 2.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Briar Club | Melee | 18 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | slow=30, move_recovery=30 |
| 2. Rootfurrow | Bolt | 11 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=290, root=18, move_recovery=30 |
| 3. Bramble Garden | Field | 4 | 15 / 1 / 14 | 105t / 210 | 1.17 / 2.25 | lifetime=150, heal=2, move_recovery=30, surface=3, surface_radius=2100, surface_life=240, pulse every 24t |
| 4. Barkskin | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | shield=22, guard=36, move_recovery=30 |

## Glasswing

A brittle dragonfly whose cannon rewards stillness.

**Body:** 94 HP; 4.42 units/s; 0.35 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 150 degrees/s; strafe 55%; reverse 40%; acceleration divisor 3; braking divisor 1; dodge speed 85%; wind affinity 160%.

**Winning pattern:** Establish distance, charge focus by standing, fire through narrow lanes.

**Counterplay:** Cross the minimum range during a long windup; force repeated turns and moves.

**Learning test:** Find safe stationary firing windows without abandoning the objective.

**Axes (1–5):** reach 5, commitment 5, mobility 2, control 2, persistence 1, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Needle Ray | Beam | 28 | 24 / 1 / 17 | 66t / 190 | 15.14 / 0.17 | min_range=3500, move_recovery=30 |
| 2. Glass Shards | Bolt | 8 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, move_recovery=30, shots=3 |
| 3. Wing Slip | Blink | 0 | 8 / 1 / 13 | 135t / 230 | 2.73 / 0.00 | move_recovery=25 |
| 4. Repulsion | Nova | 11 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.54 | impulse=750, move_recovery=30 |

## Voltjack

A conductive jackal that turns wet tags into dive windows.

**Body:** 106 HP; 4.95 units/s; 0.43 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 420 degrees/s; strafe 78%; reverse 52%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 100%.

**Winning pattern:** Mark with Spark Pin, cross the gap, discharge marks at close range.

**Counterplay:** Spread the engage and deny the mark; dry conditions reduce its spike.

**Learning test:** Choose between ranged probing and a committed marked-target dive.

**Axes (1–5):** reach 3, commitment 5, mobility 4, control 2, persistence 1, sustain 1, defense 2, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Spark Pin | Bolt | 11 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, mark=120, move_recovery=30, element=3 |
| 2. Arc Fang | Lunge | 18 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, bonus_mark=12, move_recovery=25, element=3 |
| 3. Discharge | Nova | 16 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.15 | bonus_mark=15, move_recovery=30, element=3 |
| 4. Static Screen | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | haste=45, shield=15, guard=24, move_recovery=30, element=3 |

## Miretoad

A squat toad whose poison needs repeated access.

**Body:** 123 HP; 3.96 units/s; 0.57 collider radius; mass 100; stamina regeneration 210/s.

**Locomotion:** turns 210 degrees/s; strafe 66%; reverse 48%; acceleration divisor 3; braking divisor 2; dodge speed 90%; wind affinity 100%.

**Winning pattern:** Apply venom at several ranges, maintain stacks, heal while the poison works.

**Counterplay:** Break contact long enough for stacks to expire, then burst the slow body.

**Learning test:** Preserve stacks without trading all of its health for one extra hit.

**Axes (1–5):** reach 3, commitment 2, mobility 1, control 3, persistence 5, sustain 4, defense 2, execution 3.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Venom Tongue | Melee | 9 | 6 / 3 / 9 | 27t / 90 | 2.44 / 0.83 | poison=150, move_recovery=30 |
| 2. Spitball | Bolt | 9 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=320, poison=150, move_recovery=30 |
| 3. Mire Pool | Field | 2 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=150, slow=30, poison=90, move_recovery=30, surface=7, surface_radius=1900, surface_life=240, pulse every 24t |
| 4. Digest | Ward | 0 | 18 / 1 / 9 | 180t / 160 | 0.00 / 0.00 | heal=13, move_recovery=30 |

## Basaltusk

A heavy boar that wants the opponent between tusks and stone.

**Body:** 141 HP; 3.66 units/s; 0.64 collider radius; mass 170; stamina regeneration 180/s.

**Locomotion:** turns 90 degrees/s; strafe 42%; reverse 28%; acceleration divisor 5; braking divisor 2; dodge speed 80%; wind affinity 60%.

**Winning pattern:** Approach facing threats, pull into range, drive the opponent into terrain.

**Counterplay:** Flank the frontal armor; leave a lateral escape before the ram.

**Learning test:** Line up wall collisions instead of charging directly at every target.

**Axes (1–5):** reach 2, commitment 5, mobility 2, control 4, persistence 1, sustain 1, defense 5, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Stone Tusk | Melee | 18 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | impulse=300, move_recovery=30 |
| 2. Fault Ram | Lunge | 21 | 13 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=440, impulse=1000, move_recovery=25 |
| 3. Quarry Hook | Beam | 10 | 18 / 1 / 17 | 66t / 190 | 6.64 / 0.27 | impulse=-850, move_recovery=30 |
| 4. Bedrock | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | shield=24, guard=36, cleanse=1, move_recovery=30 |

## Rimehare

A fast hare that converts repeated slows into brief roots.

**Body:** 131 HP; 5.57 units/s; 0.34 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 480 degrees/s; strafe 95%; reverse 72%; acceleration divisor 2; braking divisor 1; dodge speed 115%; wind affinity 100%.

**Winning pattern:** Clip approach paths, build chill through repeat tags, escape before commitment.

**Counterplay:** Bait the escape then attack from another angle; its direct burst is low.

**Learning test:** Lead lateral movement and conserve dodge for the engage.

**Axes (1–5):** reach 4, commitment 2, mobility 5, control 4, persistence 3, sustain 1, defense 1, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Icicle | Bolt | 16 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=500, slow=45, move_start=100, move_active=100, move_recovery=60, element=2 |
| 2. Snowshoe | Lunge | 12 | 8 / 5 / 8 | 66t / 190 | 1.37 / 0.63 | speed=430, slow=30, move_recovery=25, element=2 |
| 3. Frost Lace | Field | 2 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=150, slow=45, move_start=65, move_active=65, move_recovery=60, surface=2, surface_radius=1900, surface_life=270, element=2, pulse every 24t |
| 4. Whiteout | Nova | 17 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | impulse=300, slow=60, move_recovery=30, element=2, wind_strength=10, wind_duration=120 |

## Gloamcat

A shadow cat paid for reaching the unguarded side.

**Body:** 100 HP; 5.27 units/s; 0.36 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 540 degrees/s; strafe 88%; reverse 65%; acceleration divisor 2; braking divisor 1; dodge speed 115%; wind affinity 100%.

**Winning pattern:** Blink across a firing angle, mark the prey, punish from behind.

**Counterplay:** Track the blink endpoint and rotate your facing; punish missed lunges.

**Learning test:** Reason about opponent orientation rather than only distance.

**Axes (1–5):** reach 2, commitment 5, mobility 5, control 1, persistence 1, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Backfang | Melee | 20 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | execute=10, move_start=70, move_active=70, move_recovery=60 |
| 2. Dusk Needle | Bolt | 10 | 10 / 1 / 11 | 36t / 130 | 7.42 / 0.20 | speed=430, mark=90, move_recovery=30 |
| 3. Night Pounce | Lunge | 22 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=440, bonus_mark=10, move_recovery=25 |
| 4. Crossfade | Blink | 0 | 8 / 1 / 13 | 135t / 230 | 3.71 / 0.00 | haste=30, move_recovery=25 |

## Ironmoth

A metal moth that reflects a projectile during guard.

**Body:** 116 HP; 4.28 units/s; 0.47 collider radius; mass 120; stamina regeneration 180/s.

**Locomotion:** turns 240 degrees/s; strafe 80%; reverse 60%; acceleration divisor 3; braking divisor 1; dodge speed 95%; wind affinity 160%.

**Winning pattern:** Threaten reflective guard to buy space; punish the opponent for waiting.

**Counterplay:** Use beams, zones or a melee approach, and bait the guard first.

**Learning test:** Time a guard to projectile arrival rather than cast start.

**Axes (1–5):** reach 3, commitment 2, mobility 2, control 2, persistence 3, sustain 1, defense 5, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Steel Dust | Bolt | 9 | 10 / 1 / 11 | 36t / 130 | 6.84 / 0.20 | speed=430, move_recovery=30, shots=3 |
| 2. Mirror Carapace | Ward | 0 | 3 / 1 / 9 | 90t / 160 | 0.00 / 0.00 | shield=10, guard=18, move_recovery=30 |
| 3. Shear Wing | Melee | 19 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | wound=60, move_start=50, move_active=50, move_recovery=60 |
| 4. Magnet Wake | Field | 3 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=150, slow=30, move_recovery=30, pulse every 24t |

## Tidecoil

An eel that is faster and more sustainable on wet ground.

**Body:** 149 HP; 4.51 units/s; 0.42 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 270 degrees/s; strafe 85%; reverse 62%; acceleration divisor 3; braking divisor 2; dodge speed 100%; wind affinity 100%.

**Winning pattern:** Pull enemies into a pool and hold a favorable close-mid range.

**Counterplay:** Dodge the pull, leave the pool, and capitalize on dry-weather weakness.

**Learning test:** Compensate for wind and choose pull angles that do not help the enemy.

**Axes (1–5):** reach 3, commitment 3, mobility 3, control 4, persistence 4, sustain 3, defense 2, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Undertow | Beam | 18 | 18 / 1 / 17 | 66t / 190 | 7.81 / 0.27 | impulse=-700, slow=30, move_start=60, move_active=60, move_recovery=60, element=4 |
| 2. Tidal Bite | Melee | 23 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | move_start=35, move_active=35, move_recovery=60, element=4 |
| 3. Rain Basin | Field | 3 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=150, slow=20, heal=2, move_recovery=30, surface=1, surface_radius=2400, surface_life=300, element=4, surface_flow=24, pulse every 24t |
| 4. Surge | Lunge | 14 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, haste=60, move_recovery=25, element=4 |

## Quillrat

A small rodent paid for convincing enemies to take a route.

**Body:** 124 HP; 4.89 units/s; 0.33 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 360 degrees/s; strafe 85%; reverse 68%; acceleration divisor 2; braking divisor 1; dodge speed 105%; wind affinity 100%.

**Winning pattern:** Place visible traps near likely paths; herd with shots and knockback.

**Counterplay:** Clear or route around trap circles and attack before the field is prepared.

**Learning test:** Predict future positions rather than planting directly under the current enemy.

**Axes (1–5):** reach 4, commitment 2, mobility 3, control 4, persistence 5, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Barb Shot | Bolt | 16 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=390, move_start=60, move_active=60, move_recovery=60 |
| 2. Caltrop | Trap | 27 | 10 / 1 / 10 | 81t / 150 | 4.88 / 1.07 | lifetime=270, root=21, move_recovery=30, pulse every 15t |
| 3. Needle Fence | Field | 4 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.37 | lifetime=150, slow=30, move_recovery=30, pulse every 24t |
| 4. Panic Quills | Nova | 16 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | impulse=550, move_recovery=30 |

## Sunstag

A stag that turns a defensive cleanse into tempo.

**Body:** 117 HP; 4.78 units/s; 0.50 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 240 degrees/s; strafe 70%; reverse 50%; acceleration divisor 3; braking divisor 1; dodge speed 95%; wind affinity 100%.

**Winning pattern:** Absorb a debuff, cleanse it, use the haste window to control center.

**Counterplay:** Delay crowd control until cleanse is gone; contest its long-cooldown recovery.

**Learning test:** Distinguish valuable cleanse timing from using every spell on cooldown.

**Axes (1–5):** reach 3, commitment 3, mobility 4, control 1, persistence 1, sustain 4, defense 3, execution 3.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Sun Antler | Melee | 17 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | mark=75, move_start=35, move_active=35, move_recovery=60 |
| 2. Daybreak | Beam | 18 | 18 / 1 / 17 | 66t / 190 | 8.30 / 0.27 | bonus_mark=9, move_recovery=30 |
| 3. Golden Bound | Lunge | 12 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, haste=45, move_recovery=25 |
| 4. Renew | Ward | 0 | 3 / 1 / 9 | 165t / 160 | 0.00 / 0.00 | shield=10, heal=12, guard=24, cleanse=1, move_recovery=30 |

## Gravemole

A burrowing mole that empowers its next hit after displacement.

**Body:** 111 HP; 4.48 units/s; 0.49 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 180 degrees/s; strafe 58%; reverse 40%; acceleration divisor 4; braking divisor 2; dodge speed 90%; wind affinity 100%.

**Winning pattern:** Show a delayed eruption, blink to a new line, land the empowered attack.

**Counterplay:** Move out of the eruption and hold defense for the post-blink hit.

**Learning test:** Coordinate delayed ground damage with its own arrival.

**Axes (1–5):** reach 2, commitment 5, mobility 4, control 3, persistence 3, sustain 1, defense 2, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Grave Claw | Melee | 18 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | move_start=35, move_active=35, move_recovery=60 |
| 2. Burrow | Blink | 0 | 12 / 1 / 13 | 135t / 230 | 4.10 / 0.00 | move_recovery=25 |
| 3. Upheaval | Trap | 24 | 16 / 1 / 10 | 81t / 150 | 4.88 / 1.51 | lifetime=270, root=12, move_recovery=30, surface=7, surface_radius=1500, surface_life=210, pulse every 15t |
| 4. Tunnel Rush | Lunge | 15 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, impulse=350, move_recovery=25 |

## Prismray

A ray that makes walls into indirect firing lanes.

**Body:** 104 HP; 4.42 units/s; 0.44 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 210 degrees/s; strafe 78%; reverse 50%; acceleration divisor 3; braking divisor 1; dodge speed 95%; wind affinity 160%.

**Winning pattern:** Bank shots around rocks, then exploit forced sidesteps with a narrow beam.

**Counterplay:** Fight in open space or close the gap while the bank shot travels.

**Learning test:** Discover wall angles and distinguish return paths from initial aim.

**Axes (1–5):** reach 5, commitment 4, mobility 2, control 1, persistence 2, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Prism Dart | Bolt | 15 | 10 / 1 / 11 | 36t / 130 | 14.65 / 0.20 | speed=490, bounces=1, move_recovery=30 |
| 2. Split Spectrum | Bolt | 9 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, bounces=1, move_recovery=30, shots=3 |
| 3. Refraction | Beam | 24 | 20 / 1 / 17 | 66t / 190 | 10.74 / 0.18 | move_recovery=30 |
| 4. Prism Fold | Blink | 0 | 8 / 1 / 13 | 150t / 230 | 2.54 / 0.00 | move_recovery=25 |

## Thornmantis

A mantis that invests in a wound before its real trade.

**Body:** 113 HP; 5.07 units/s; 0.38 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 450 degrees/s; strafe 85%; reverse 55%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 100%.

**Winning pattern:** Open a wound, stick briefly, cash out a high-damage cut.

**Counterplay:** Kite the short blades and wait out the wound before using a heal.

**Learning test:** Sequence setup and payoff while preserving an exit.

**Axes (1–5):** reach 1, commitment 4, mobility 4, control 1, persistence 1, sustain 2, defense 2, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Open Seam | Melee | 11 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | wound=120, move_start=60, move_active=60, move_recovery=60 |
| 2. Harvest Cut | Melee | 22 | 10 / 3 / 9 | 48t / 90 | 1.76 / 0.78 | move_start=35, move_active=35, move_recovery=60 |
| 3. Thorn Leap | Lunge | 16 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, wound=60, move_recovery=25 |
| 4. Razor Guard | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | haste=30, shield=17, guard=18, move_recovery=30 |

## Mosswarden

A moss-covered giant that heals only while owning a patch.

**Body:** 118 HP; 3.72 units/s; 0.62 collider radius; mass 145; stamina regeneration 180/s.

**Locomotion:** turns 120 degrees/s; strafe 50%; reverse 35%; acceleration divisor 4; braking divisor 2; dodge speed 80%; wind affinity 60%.

**Winning pattern:** Seed shelter on the objective, defend it, replenish slowly between trades.

**Counterplay:** Displace it out of the shelter or apply wounds before committing damage.

**Learning test:** Weigh staying inside a heal zone against avoiding incoming area attacks.

**Axes (1–5):** reach 2, commitment 2, mobility 1, control 2, persistence 5, sustain 5, defense 4, execution 2.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Moss Fist | Melee | 16 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | slow=24, move_recovery=30 |
| 2. Seed Bomb | Bolt | 12 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=300, move_recovery=30 |
| 3. Shelter Grove | Field | 2 | 15 / 1 / 14 | 105t / 210 | 2.54 / 2.34 | lifetime=150, heal=3, move_recovery=30, surface=3, surface_radius=2300, surface_life=270, pulse every 24t |
| 4. Living Wall | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | shield=24, guard=24, move_recovery=30 |

## Galecrest

A crestbird that turns wind into unusually fast rotations.

**Body:** 115 HP; 5.30 units/s; 0.36 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 450 degrees/s; strafe 95%; reverse 65%; acceleration divisor 2; braking divisor 1; dodge speed 115%; wind affinity 240%.

**Winning pattern:** Choose a wind direction with Windwright, ride a tailwind to shape Feather Fan, and contest the vane before the field expires.

**Counterplay:** Force the bird to travel into its own headwind, counter-cast wind, or punish the planted Windwright startup.

**Learning test:** Choose wind heading and strength to improve projectile contact and relocation; distinguish an advantageous tailwind from merely nonzero wind.

**Axes (1–5):** reach 4, commitment 3, mobility 5, control 3, persistence 1, sustain 1, defense 1, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Feather Fan | Bolt | 9 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=480, move_start=100, move_active=100, move_recovery=60, shots=3 |
| 2. Gust Lance | Beam | 17 | 18 / 1 / 17 | 66t / 190 | 8.79 / 0.27 | impulse=600, move_recovery=30, wind_strength=12, wind_duration=90 |
| 3. Jetstream | Lunge | 15 | 8 / 6 / 14 | 66t / 190 | 1.37 / 0.63 | speed=500, move_recovery=25 |
| 4. Windwright | Nova | 16 | 21 / 2 / 15 | 240t / 320 | 0.00 / 2.73 | impulse=450, haste=45, move_recovery=30, wind_strength=24, wind_duration=180 |

## Ashram

A ram that pays real health to create lethal windows.

**Body:** 120 HP; 4.63 units/s; 0.56 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 150 degrees/s; strafe 50%; reverse 32%; acceleration divisor 4; braking divisor 2; dodge speed 90%; wind affinity 100%.

**Winning pattern:** Spend health to enter, exploit missing-health damage, extract through a small heal.

**Counterplay:** Deny the last hit, disengage through its haste, punish reckless self-costs.

**Learning test:** Estimate whether the all-in wins before sacrificing its safety buffer.

**Axes (1–5):** reach 1, commitment 5, mobility 3, control 1, persistence 1, sustain 2, defense 1, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Blood Horn | Melee | 19 | 6 / 3 / 9 | 27t / 60 | 1.76 / 0.78 | health_cost=4, move_start=35, move_active=35, move_recovery=60, element=1 |
| 2. Cinder Charge | Lunge | 22 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, impulse=400, health_cost=6, move_recovery=25, element=1 |
| 3. Fury | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | haste=75, health_cost=5, move_recovery=30, element=1 |
| 4. Last Ember | Nova | 21 | 18 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | heal=6, execute=14, move_recovery=30, element=1 |

## Clockfin

A clockwork fish paid for changing spell slots.

**Body:** 129 HP; 4.54 units/s; 0.39 collider radius; mass 100; stamina regeneration 120/s.

**Locomotion:** turns 330 degrees/s; strafe 85%; reverse 65%; acceleration divisor 2; braking divisor 1; dodge speed 100%; wind affinity 100%.

**Winning pattern:** Alternate cheap tools with expensive payoffs to recover energy.

**Counterplay:** Force a repeated defensive spell or interrupt the slow payoff.

**Learning test:** Learn a flexible sequence instead of a fixed highest-DPS button.

**Axes (1–5):** reach 3, commitment 3, mobility 3, control 1, persistence 1, sustain 2, defense 2, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Tick | Bolt | 14 | 10 / 1 / 11 | 24t / 100 | 11.23 / 0.20 | speed=430, move_start=65, move_active=65, move_recovery=60 |
| 2. Tock | Beam | 24 | 18 / 1 / 17 | 57t / 200 | 10.74 / 0.27 | move_recovery=30 |
| 3. Second Hand | Lunge | 16 | 8 / 7 / 14 | 51t / 190 | 1.37 / 0.63 | speed=380, move_recovery=25 |
| 4. Windup Key | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | haste=45, shield=18, guard=12, move_recovery=30 |

## Nullurchin

An urchin that punishes protective-resource dependence.

**Body:** 132 HP; 3.81 units/s; 0.54 collider radius; mass 135; stamina regeneration 180/s.

**Locomotion:** turns 180 degrees/s; strafe 72%; reverse 55%; acceleration divisor 3; braking divisor 1; dodge speed 90%; wind affinity 100%.

**Winning pattern:** Strip shields with safe pokes, silence a recovery spell, take ground.

**Counterplay:** Offer raw health instead of shields and pressure its poor mobility.

**Learning test:** Identify when disruption matters more than the largest damage number.

**Axes (1–5):** reach 4, commitment 3, mobility 1, control 4, persistence 3, sustain 1, defense 4, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Null Spine | Bolt | 15 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, drain=60, move_recovery=30 |
| 2. Mute Line | Beam | 17 | 18 / 1 / 17 | 66t / 190 | 8.30 / 0.27 | silence=24, move_recovery=30 |
| 3. Deadwater | Field | 3 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=150, wound=60, move_recovery=30, pulse every 24t |
| 4. Void Shell | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | shield=22, guard=24, move_recovery=30 |

## Waxwyrm

A wax wyrm whose strength has a separate health bar.

**Body:** 102 HP; 4.01 units/s; 0.52 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 150 degrees/s; strafe 55%; reverse 40%; acceleration divisor 3; braking divisor 1; dodge speed 85%; wind affinity 100%.

**Winning pattern:** Build a turret with crossfire, defend it with slows and body positioning.

**Counterplay:** Aim at the turret to destroy it before fighting in its lane.

**Learning test:** Place summons where their line of sight and survival both matter.

**Axes (1–5):** reach 4, commitment 2, mobility 1, control 3, persistence 5, sustain 1, defense 3, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Wax Glob | Bolt | 11 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=330, slow=24, move_recovery=30, surface=8, surface_radius=1400, surface_life=180, element=1 |
| 2. Candle Sentry | Turret | 7 | 18 / 1 / 15 | 180t / 260 | 3.91 / 0.57 | speed=310, lifetime=240, move_recovery=30, element=1, pulse every 27t |
| 3. Hot Seal | Field | 4 | 15 / 1 / 14 | 105t / 210 | 3.71 / 1.76 | lifetime=150, move_recovery=30, surface=4, surface_radius=1700, surface_life=180, element=1, pulse every 24t |
| 4. Wax Jacket | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | shield=22, guard=18, move_recovery=30, element=1 |

## Ribbonape

A ribbon-tailed ape that hits along outbound and return paths.

**Body:** 124 HP; 5.04 units/s; 0.43 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 510 degrees/s; strafe 100%; reverse 80%; acceleration divisor 2; braking divisor 1; dodge speed 115%; wind affinity 160%.

**Winning pattern:** Throw a ribbon, change position, make the returning arc cross the enemy.

**Counterplay:** Step off the return line rather than blindly dodging the first pass.

**Learning test:** Use locomotion to steer an existing projectile.

**Axes (1–5):** reach 3, commitment 3, mobility 4, control 2, persistence 2, sustain 1, defense 2, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Ribbon Throw | Bolt | 13 | 10 / 1 / 11 | 36t / 130 | 6.84 / 0.20 | speed=370, lifetime=48, returning=1, move_start=100, move_active=100, move_recovery=60, pulse every 15t |
| 2. Tail Snap | Melee | 21 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | impulse=-250, move_start=75, move_active=75, move_recovery=60 |
| 3. Acrobat | Lunge | 13 | 8 / 5 / 14 | 66t / 190 | 1.37 / 0.63 | speed=420, haste=30, move_recovery=25, wind_strength=8, wind_duration=75 |
| 4. Silk Whorl | Nova | 18 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | slow=30, move_recovery=30 |

## Saltcrab

A crab that replenishes stamina during guard.

**Body:** 175 HP; 3.60 units/s; 0.63 collider radius; mass 165; stamina regeneration 90/s.

**Locomotion:** turns 120 degrees/s; strafe 90%; reverse 40%; acceleration divisor 4; braking divisor 2; dodge speed 85%; wind affinity 60%.

**Winning pattern:** Block while refilling, then spend a full bar to bully a short-range fight.

**Counterplay:** Wait out the shell and attack while it is spending; displace it off center.

**Learning test:** Use defensive downtime as a resource reset rather than permanent turtling.

**Axes (1–5):** reach 2, commitment 3, mobility 1, control 2, persistence 1, sustain 2, defense 5, execution 3.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Salt Pinch | Melee | 24 | 6 / 3 / 9 | 27t / 170 | 1.76 / 0.78 | move_start=50, move_active=50, move_recovery=60 |
| 2. Brine Cannon | Bolt | 26 | 10 / 1 / 11 | 36t / 230 | 11.23 / 0.20 | speed=350, move_recovery=30 |
| 3. Closed Shell | Ward | 0 | 3 / 1 / 9 | 132t / 160 | 0.00 / 0.00 | shield=26, guard=42, move_recovery=30 |
| 4. Scuttle | Lunge | 15 | 8 / 6 / 14 | 66t / 190 | 1.37 / 0.63 | speed=330, move_recovery=25 |

## Nectarbat

A bat that survives by landing risky direct hits.

**Body:** 105 HP; 5.19 units/s; 0.34 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 480 degrees/s; strafe 100%; reverse 75%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 160%.

**Winning pattern:** Take repeated short trades, heal on contact, exit before crowd control lands.

**Counterplay:** Apply wounds or deny contact; its sustain vanishes while kited.

**Learning test:** Select safe access windows instead of interpreting lifesteal as permission to face-tank.

**Axes (1–5):** reach 2, commitment 4, mobility 4, control 1, persistence 1, sustain 4, defense 1, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Siphon Fang | Melee | 19 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | move_start=100, move_active=100, move_recovery=60 |
| 2. Nectar Needle | Bolt | 12 | 10 / 1 / 11 | 36t / 130 | 7.81 / 0.20 | speed=430, move_recovery=30 |
| 3. Swoop | Lunge | 17 | 8 / 5 / 14 | 66t / 190 | 1.37 / 0.63 | speed=430, move_recovery=25 |
| 4. Night Bloom | Nova | 17 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | wound=45, move_recovery=30 |

## Bellox

A bell-backed ox whose third direct hit rings louder.

**Body:** 125 HP; 4.16 units/s; 0.58 collider radius; mass 130; stamina regeneration 180/s.

**Locomotion:** turns 150 degrees/s; strafe 52%; reverse 35%; acceleration divisor 4; braking divisor 2; dodge speed 85%; wind affinity 100%.

**Winning pattern:** Land two reliable taps, line up a decisive third contact.

**Counterplay:** Break its contact rhythm and force the third hit into a guard.

**Learning test:** Track a persistent hit counter through movement and cooldown choices.

**Axes (1–5):** reach 3, commitment 3, mobility 2, control 2, persistence 1, sustain 1, defense 3, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Clapper | Melee | 13 | 6 / 3 / 9 | 21t / 90 | 1.76 / 0.78 | move_recovery=30 |
| 2. Bell Wave | Beam | 17 | 18 / 1 / 17 | 66t / 190 | 8.30 / 0.41 | move_recovery=30 |
| 3. Peal Rush | Lunge | 15 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, impulse=400, move_recovery=25 |
| 4. Resonant Shell | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | shield=20, guard=24, move_recovery=30 |

## Anvilnewt

A newt that forges stronger blows while shielded.

**Body:** 104 HP; 4.07 units/s; 0.54 collider radius; mass 125; stamina regeneration 180/s.

**Locomotion:** turns 90 degrees/s; strafe 45%; reverse 30%; acceleration divisor 5; braking divisor 2; dodge speed 80%; wind affinity 100%.

**Winning pattern:** Prepare a shield, enter while it lasts, choose whether to risk losing the damage buff.

**Counterplay:** Poke away the shield before the real exchange; use anti-shield tools.

**Learning test:** Treat shield as both survival and an expiring offensive resource.

**Axes (1–5):** reach 2, commitment 4, mobility 1, control 2, persistence 1, sustain 1, defense 4, execution 3.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Hammer Tongue | Melee | 16 | 9 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | move_recovery=30 |
| 2. Forged Bolt | Bolt | 14 | 10 / 1 / 11 | 36t / 130 | 8.30 / 0.20 | speed=430, move_recovery=30 |
| 3. Temper | Ward | 0 | 3 / 1 / 9 | 105t / 160 | 0.00 / 0.00 | shield=24, move_recovery=30 |
| 4. Anvil Fall | Nova | 19 | 18 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | slow=24, move_recovery=30 |

## Duneskink

A skink that stores actual distance traveled for the next strike.

**Body:** 108 HP; 5.57 units/s; 0.33 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 420 degrees/s; strafe 90%; reverse 75%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 100%.

**Winning pattern:** Circle to build momentum, spend it in a short trade, move again.

**Counterplay:** Pin it or force it to fight before it has moved enough.

**Learning test:** Plan paths with tactical purpose rather than running in place for charge.

**Axes (1–5):** reach 3, commitment 3, mobility 5, control 2, persistence 1, sustain 1, defense 1, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Sandlash | Melee | 16 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | move_start=80, move_active=80, move_recovery=60 |
| 2. Dune Dart | Bolt | 12 | 10 / 1 / 11 | 36t / 130 | 8.79 / 0.20 | speed=430, move_recovery=30 |
| 3. Sand Sprint | Lunge | 14 | 8 / 6 / 14 | 66t / 190 | 1.37 / 0.63 | speed=470, move_recovery=25 |
| 4. Dust Kick | Nova | 12 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | impulse=350, slow=40, move_recovery=30 |

## Kelpwidow

A long-limbed widow that keeps prey at a precise intermediate range.

**Body:** 145 HP; 4.19 units/s; 0.46 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 210 degrees/s; strafe 75%; reverse 55%; acceleration divisor 3; braking divisor 2; dodge speed 95%; wind affinity 100%.

**Winning pattern:** Tag from distance, pull toward a prepared patch, punish attempts to flee.

**Counterplay:** Commit fully through its preferred ring; do not oscillate on its strongest range.

**Learning test:** Maintain a distance band under pressure rather than maximize distance.

**Axes (1–5):** reach 4, commitment 3, mobility 2, control 5, persistence 4, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Kelp Harpoon | Bolt | 17 | 10 / 1 / 11 | 36t / 130 | 10.25 / 0.20 | speed=390, slow=30, move_recovery=30 |
| 2. Reel In | Beam | 17 | 18 / 1 / 17 | 66t / 190 | 8.30 / 0.27 | impulse=-600, root=12, move_recovery=30 |
| 3. Tangle Bed | Field | 3 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=150, root=9, move_recovery=30, surface=1, surface_radius=2000, surface_life=240, surface_flow=14, pulse every 30t |
| 4. Leg Sweep | Nova | 17 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | impulse=550, move_recovery=30 |

## Pyrelisk

A lizard that becomes stronger and slower as it casts.

**Body:** 111 HP; 4.31 units/s; 0.51 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 180 degrees/s; strafe 58%; reverse 38%; acceleration divisor 3; braking divisor 1; dodge speed 90%; wind affinity 100%.

**Winning pattern:** Build heat safely, spend its amplified area damage before being surrounded.

**Counterplay:** Force mobility at high heat or wait while the heat drains.

**Learning test:** Decide when another cast is worth losing movement speed.

**Axes (1–5):** reach 5, commitment 5, mobility 1, control 2, persistence 4, sustain 1, defense 1, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Heat Lance | Beam | 21 | 20 / 1 / 17 | 66t / 190 | 10.74 / 0.27 | move_recovery=30, element=1 |
| 2. Coal Fan | Bolt | 8 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, burn=45, move_recovery=30, element=1, shots=3 |
| 3. Furnace | Field | 5 | 15 / 1 / 14 | 105t / 210 | 4.69 / 2.05 | burn=30, lifetime=150, move_recovery=30, surface=4, surface_radius=1900, surface_life=180, element=1, pulse every 24t |
| 4. Vent | Nova | 14 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | impulse=500, cleanse=1, move_recovery=30, element=1 |

## Mooncalf

A calf that stores shield damage for a later direct hit.

**Body:** 102 HP; 4.31 units/s; 0.59 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 210 degrees/s; strafe 70%; reverse 55%; acceleration divisor 3; braking divisor 1; dodge speed 90%; wind affinity 100%.

**Winning pattern:** Offer a shielded trade, preserve the stored force, choose a reliable counterstrike.

**Counterplay:** Stop hitting the shield or force its next attack to miss.

**Learning test:** Remember shield absorption and distinguish damage bait from a losing trade.

**Axes (1–5):** reach 3, commitment 3, mobility 2, control 1, persistence 1, sustain 1, defense 5, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Moon Tap | Melee | 12 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | move_recovery=30 |
| 2. Lunar Shield | Ward | 0 | 3 / 1 / 9 | 111t / 160 | 0.00 / 0.00 | shield=25, guard=18, move_recovery=30 |
| 3. Moonbeam | Beam | 16 | 18 / 1 / 17 | 66t / 190 | 9.28 / 0.27 | move_recovery=30 |
| 4. Orbit | Lunge | 10 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, haste=30, move_recovery=25 |

## Coppergecko

A gecko whose every third cast refunds stamina.

**Body:** 125 HP; 4.78 units/s; 0.35 collider radius; mass 100; stamina regeneration 120/s.

**Locomotion:** turns 390 degrees/s; strafe 95%; reverse 75%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 100%.

**Winning pattern:** Use a cheap tool to approach the refund, then spend on a high-cost spell.

**Counterplay:** Pressure it between refunds and deny low-risk filler casts.

**Learning test:** Plan the cast counter and avoid wasting refunds at full energy.

**Axes (1–5):** reach 4, commitment 3, mobility 3, control 1, persistence 1, sustain 1, defense 2, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Copper Ping | Bolt | 13 | 10 / 1 / 11 | 24t / 70 | 11.23 / 0.20 | speed=430, move_start=80, move_active=80, move_recovery=60, element=3 |
| 2. Coil Cannon | Beam | 29 | 18 / 1 / 17 | 69t / 260 | 10.74 / 0.27 | move_recovery=30, element=3 |
| 3. Springwire | Lunge | 17 | 8 / 7 / 14 | 66t / 210 | 1.37 / 0.63 | speed=380, move_recovery=25, element=3 |
| 4. Capacitor | Ward | 0 | 3 / 1 / 9 | 120t / 130 | 0.00 / 0.00 | shield=20, guard=12, move_recovery=30, element=3 |

## Orchardboar

A boar that receives a heal when its garden expires naturally.

**Body:** 132 HP; 4.01 units/s; 0.60 collider radius; mass 140; stamina regeneration 180/s.

**Locomotion:** turns 150 degrees/s; strafe 52%; reverse 35%; acceleration divisor 4; braking divisor 2; dodge speed 85%; wind affinity 60%.

**Winning pattern:** Plant ahead of the fight, survive until harvest, then contest again.

**Counterplay:** Kill it before the payout; destroying turrets does not count as a harvest.

**Learning test:** Track zone lifetimes and avoid taking a fight too early.

**Axes (1–5):** reach 3, commitment 3, mobility 2, control 2, persistence 4, sustain 4, defense 3, execution 3.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Root Tusk | Melee | 19 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | impulse=300, move_recovery=30 |
| 2. Falling Fruit | Bolt | 15 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=310, move_recovery=30 |
| 3. Orchard | Field | 3 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=120, slow=15, move_recovery=30, surface=3, surface_radius=2100, surface_life=270, pulse every 24t |
| 4. Harvest Run | Lunge | 13 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, shield=12, move_recovery=25 |

## Inkheron

A heron whose own ink zones consume hostile projectiles.

**Body:** 121 HP; 4.72 units/s; 0.40 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 270 degrees/s; strafe 85%; reverse 65%; acceleration divisor 3; braking divisor 1; dodge speed 100%; wind affinity 160%.

**Winning pattern:** Put ink between itself and a shooter, then exploit the safe firing angle.

**Counterplay:** Use beams, melee or walk around the cover; smoke is visible and not stealth.

**Learning test:** Distinguish projectile cover from safety against all attack kinds.

**Axes (1–5):** reach 5, commitment 3, mobility 3, control 1, persistence 4, sustain 1, defense 3, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Ink Needle | Bolt | 17 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, move_start=60, move_active=60, move_recovery=60 |
| 2. Ink Cloud | Field | 2 | 15 / 1 / 14 | 105t / 210 | 3.71 / 2.15 | lifetime=120, move_recovery=30, pulse every 24t |
| 3. Brushstroke | Beam | 23 | 18 / 1 / 17 | 66t / 190 | 8.30 / 0.27 | move_recovery=30 |
| 4. Takeoff | Blink | 0 | 8 / 1 / 13 | 135t / 230 | 2.93 / 0.00 | move_recovery=25, wind_strength=12, wind_duration=120 |

## Hooklynx

A lynx that wants to hook a marked target rather than any target.

**Body:** 152 HP; 4.89 units/s; 0.42 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 450 degrees/s; strafe 85%; reverse 55%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 100%.

**Winning pattern:** Tag with a cheap dart, hook into claw range, cash out the mark.

**Counterplay:** Break line of sight after the tag and punish a missed hook recovery.

**Learning test:** Wait for a high-value hook instead of firing it at maximum range every time.

**Axes (1–5):** reach 4, commitment 5, mobility 3, control 4, persistence 1, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Trail Dart | Bolt | 14 | 10 / 1 / 11 | 36t / 130 | 9.77 / 0.20 | speed=430, mark=150, move_start=65, move_active=65, move_recovery=60 |
| 2. Hookline | Beam | 19 | 20 / 1 / 17 | 66t / 190 | 8.79 / 0.27 | impulse=-1100, move_recovery=30 |
| 3. Claim Claw | Melee | 25 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | bonus_mark=12, move_start=35, move_active=35, move_recovery=60 |
| 4. Hunting Bound | Lunge | 17 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, haste=30, move_recovery=25 |

## Slagjaw

A corrosive jaw that makes all contact compromise healing.

**Body:** 140 HP; 4.31 units/s; 0.61 collider radius; mass 125; stamina regeneration 180/s.

**Locomotion:** turns 120 degrees/s; strafe 45%; reverse 30%; acceleration divisor 5; braking divisor 2; dodge speed 80%; wind affinity 100%.

**Winning pattern:** Keep wounds on the opponent and take extended trades their heals cannot erase.

**Counterplay:** Take short burst trades and deny repeated contact rather than relying on recovery.

**Learning test:** Identify sustain windows and keep access without overchasing.

**Axes (1–5):** reach 3, commitment 4, mobility 2, control 1, persistence 3, sustain 1, defense 3, execution 3.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Corrosive Bite | Melee | 20 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | wound=90, move_recovery=30 |
| 2. Acid Spit | Bolt | 15 | 10 / 1 / 11 | 36t / 130 | 9.28 / 0.20 | speed=430, poison=60, move_recovery=30 |
| 3. Slag Trail | Field | 4 | 15 / 1 / 14 | 105t / 210 | 4.69 / 1.76 | lifetime=150, wound=60, move_recovery=30, surface=8, surface_radius=1900, surface_life=240, pulse every 24t |
| 4. Jawbreaker | Lunge | 20 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, impulse=300, move_recovery=25 |

## Dewotter

An otter whose dodge removes slows and damage-over-time effects.

**Body:** 101 HP; 5.21 units/s; 0.36 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 420 degrees/s; strafe 90%; reverse 75%; acceleration divisor 2; braking divisor 1; dodge speed 110%; wind affinity 100%.

**Winning pattern:** Invite a debuff, cleanse through dodge, return while the enemy setup is down.

**Counterplay:** Bait dodge before applying the important debuff; use silence carefully.

**Learning test:** Value cleanse timing while still respecting dodge stamina.

**Axes (1–5):** reach 3, commitment 2, mobility 4, control 2, persistence 1, sustain 3, defense 3, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Dew Dart | Bolt | 14 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=470, move_start=75, move_active=75, move_recovery=60, element=4 |
| 2. River Cut | Melee | 16 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | slow=20, move_start=35, move_active=35, move_recovery=60, element=4 |
| 3. Waterwheel | Nova | 15 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | impulse=350, move_recovery=30, element=4 |
| 4. Fresh Spring | Ward | 0 | 3 / 1 / 9 | 180t / 160 | 0.00 / 0.00 | shield=10, heal=9, guard=24, cleanse=1, move_recovery=30, surface=1, surface_radius=1800, surface_life=210, element=4, surface_flow=12 |

## Echofin

A sonar fish paid for casting within a particular rhythm.

**Body:** 93 HP; 4.57 units/s; 0.42 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 330 degrees/s; strafe 85%; reverse 65%; acceleration divisor 2; braking divisor 1; dodge speed 105%; wind affinity 100%.

**Winning pattern:** Space casts at roughly one-second intervals, then line up a broad payoff.

**Counterplay:** Interrupt its timing by forcing defense or denying range on the beat.

**Learning test:** Learn temporal rhythm rather than pure cooldown greed.

**Axes (1–5):** reach 4, commitment 4, mobility 3, control 1, persistence 1, sustain 1, defense 2, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Sonar Tap | Beam | 12 | 9 / 1 / 8 | 27t / 190 | 7.81 / 0.27 | move_start=65, move_active=65, move_recovery=60 |
| 2. Echo Fan | Bolt | 6 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, move_recovery=30, shots=3 |
| 3. Bass Ring | Nova | 19 | 14 / 2 / 15 | 75t / 170 | 0.00 / 3.32 | min_range=1200, move_recovery=30 |
| 4. Silent Beat | Ward | 0 | 3 / 1 / 9 | 120t / 160 | 0.00 / 0.00 | haste=30, shield=13, guard=12, move_recovery=30 |

## Latchspider

A spider whose own zones accelerate it.

**Body:** 150 HP; 4.75 units/s; 0.41 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 300 degrees/s; strafe 90%; reverse 65%; acceleration divisor 3; braking divisor 1; dodge speed 105%; wind affinity 100%.

**Winning pattern:** Lay a route of webs, fight while moving along it, pull enemies across it.

**Counterplay:** Force it out of the network or contest before it has placed safe routes.

**Learning test:** Use terrain as a locomotion network rather than a stationary damage patch.

**Axes (1–5):** reach 3, commitment 3, mobility 4, control 4, persistence 5, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Silk Dart | Bolt | 17 | 10 / 1 / 11 | 36t / 130 | 11.23 / 0.20 | speed=430, slow=30, move_start=65, move_active=65, move_recovery=60 |
| 2. Webway | Field | 2 | 15 / 1 / 14 | 90t / 210 | 4.69 / 1.95 | lifetime=180, slow=30, move_recovery=30, pulse every 24t |
| 3. Latch | Beam | 19 | 18 / 1 / 17 | 66t / 190 | 7.32 / 0.27 | impulse=-650, move_recovery=30 |
| 4. Skitter | Lunge | 18 | 8 / 5 / 14 | 66t / 190 | 1.37 / 0.63 | speed=400, move_recovery=25 |

## Flintroc

A roc that charges a powerful shot by not attacking.

**Body:** 104 HP; 4.25 units/s; 0.44 collider radius; mass 100; stamina regeneration 180/s.

**Locomotion:** turns 120 degrees/s; strafe 45%; reverse 30%; acceleration divisor 4; braking divisor 1; dodge speed 80%; wind affinity 100%.

**Winning pattern:** Hold fire to charge, select a good lane, then reposition while reloading.

**Counterplay:** Pressure during its reload and force it to spend charge on a poor shot.

**Learning test:** Trade shot frequency for reliability and avoid dumping charge into a dodge.

**Axes (1–5):** reach 5, commitment 5, mobility 2, control 1, persistence 1, sustain 1, defense 1, execution 5.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Flint Cannon | Bolt | 22 | 18 / 1 / 17 | 66t / 130 | 15.14 / 0.16 | speed=580, min_range=2500, move_recovery=30 |
| 2. Stone Scatter | Bolt | 7 | 10 / 1 / 11 | 36t / 130 | 7.81 / 0.20 | speed=430, move_recovery=30, shots=4 |
| 3. Backdraft | Nova | 12 | 14 / 2 / 15 | 75t / 170 | 0.00 / 2.73 | impulse=650, move_recovery=30 |
| 4. Ridge Hop | Blink | 0 | 8 / 1 / 13 | 165t / 230 | 2.44 / 0.00 | move_recovery=25 |

## Oathhound

A hound that resists interruption during guard and rewards a successful block.

**Body:** 114 HP; 4.19 units/s; 0.57 collider radius; mass 140; stamina regeneration 180/s.

**Locomotion:** turns 240 degrees/s; strafe 70%; reverse 50%; acceleration divisor 3; braking divisor 1; dodge speed 95%; wind affinity 60%.

**Winning pattern:** Present a guarded threat, absorb contact, answer with an empowered strike.

**Counterplay:** Feint the guard and use displacement or long-range pressure instead of feeding it.

**Learning test:** Read commitment and distinguish an incoming hit from a harmless telegraph.

**Axes (1–5):** reach 3, commitment 3, mobility 2, control 2, persistence 1, sustain 1, defense 5, execution 4.

| Slot / move | Form | Damage | Startup / active / recovery | CD / stamina | Range / radius | Effects |
|---|---|---:|---|---|---|---|
| 1. Oath Fang | Melee | 15 | 6 / 3 / 9 | 27t / 90 | 1.76 / 0.78 | move_start=45, move_active=45, move_recovery=60 |
| 2. Sworn Guard | Ward | 0 | 3 / 1 / 9 | 96t / 160 | 0.00 / 0.00 | shield=17, guard=27, move_recovery=30 |
| 3. Intercept | Lunge | 15 | 8 / 7 / 14 | 66t / 190 | 1.37 / 0.63 | speed=380, impulse=250, shield=8, move_recovery=25 |
| 4. Judgment | Beam | 18 | 18 / 1 / 17 | 66t / 190 | 7.32 / 0.27 | slow=24, move_recovery=30 |
