# Passive contracts

All passive state is in the deterministic world and snapshots. Public meters/counters are exposed to the actor. A direct hit means a melee, lunge, beam, nova, or projectile contact; field/trap pulses and damage-over-time ticks do not trigger direct-hit bonuses. Turret projectiles are direct contacts. Unless stated otherwise, bonuses add before defense and shields.

| Species | Executable contract | Denial / decision |
|---|---|---|
| Cinderfox | +6 direct damage against a burning target | Deny or wait out burn before the real exchange |
| Brambleback | After 30 stationary ticks, each global 60-tick cadence refreshes shield to at least 18 for 90 ticks | Make the anchor relocate between refreshes |
| Glasswing | +1 damage per 6 stationary ticks, capped at +12 | Interrupt safe firing residency |
| Voltjack | +15% direct damage above 300 wetness | Respect wet burst; exploit dry baseline |
| Miretoad | Its poison stacks cap at 5 instead of 3 | Break contact before the stack count matters |
| Basaltusk | 20% reduction to frontal contact damage | Rear/flank access defeats the reduction |
| Rimehare | Every third slow-applying contact roots for 15 ticks if resistance permits | Track chill rhythm and CC resistance |
| Gloamcat | +9 direct damage when positioned behind the target's facing | Turn to face it, don't just run away |
| Ironmoth | Guard reflects frontal enemy projectiles, changing owner and reversing velocity | Feint, use non-projectile attacks, or change angle |
| Tidecoil | Above 300 wetness: +15% speed and 2 HP each second | Avoid paying for a long wet trade |
| Quillrat | +8 trap trigger damage | Avoid, clear or deny placement |
| Sunstag | Its cleansing action grants 60 ticks of haste | Force an early cleanse, then reapply pressure |
| Gravemole | Blink/lunge release arms +9 on the next direct contact | Protect the arrival payoff rather than the movement itself |
| Prismray | +2 on bolt contacts; its authored bolts also have one ricochet | Bank geometry is the main identity, the passive is a modest reliability budget |
| Thornmantis | +6 direct damage against wounded targets | Wound setup precedes the dangerous trade |
| Mosswarden | Heal 3 each second while within an owned field | Evict it; wounds halve the healing |
| Galecrest | +2 speed units per tick for each absolute wind unit | Wind rewards movement, but does not remove collision constraints |
| Ashram | Up to +12 direct damage proportional to missing HP | Deny the final exchange; self-cost remains real |
| Clockfin | Casting a different slot from the previous one refunds 70 stamina | Interrupt the sequence or force repeated defense |
| Nullurchin | Direct contacts remove twice as much shield per blocked damage | HP is not doubled damage; only shields suffer extra depletion |
| Waxwyrm | Turrets have 45 HP instead of 32 | Explicitly aim at the setup |
| Ribbonape | +2 on its returning-projectile contacts; return phase resets hit eligibility | Reposition to steer the return and avoid the second pass |
| Saltcrab | +7 stamina regeneration per tick during guard | Its defensive downtime funds the next offense |
| Nectarbat | Heal 20% of actual direct HP damage, minimum 1 per damaging contact | Shields, misses and wounds deny sustain |
| Bellox | Every third direct contact adds 12 damage | Track contacts, not casts; a miss doesn't advance it |
| Anvilnewt | +7 direct damage while shielded | Strip the shield before committing |
| Duneskink | Store traveled distance /16, up to 1,000; next direct hit adds meter/100 and consumes it | Root and confinement deny preparation; moving in place does not charge |
| Kelpwidow | Direct contacts beyond 3.42 units apply a 24-tick slow | Cross through the favored distance band decisively |
| Pyrelisk | Casts add 20 heat, max 100; direct hits add heat/10; speed falls by heat/3 percent; loses 1 heat per 6 ticks | Heat is strength coupled to exposure. Vent clears it |
| Mooncalf | Shield absorption stores up to 20 retaliation; next direct hit adds up to 15 and consumes it | Stop feeding the shield or bait the payoff |
| Coppergecko | Every third cast refunds 180 stamina | A cast-counter economy, distinct from Clockfin's alternation economy |
| Orchardboar | Natural expiry of an owned field heals 9 | The payout is delayed; KO or wounds can deny its value |
| Inkheron | Owned ordinary fields consume hostile projectiles inside their footprint | Cover is not stealth and does not stop beams or melee |
| Hooklynx | +5 direct damage against a marked target | The explicit claw payoff also consumes its owner's mark |
| Slagjaw | Every contact applies at least 90 ticks of wound | Break repeated contact before healing |
| Dewotter | Accepting dodge cleanses burn, poison/stacks, slow, root, silence and wound | Root prevents accepting dodge; do not mistake cleanse-on-dodge for root immunity |
| Echofin | Cast after 21–42 ticks since the last cast to gain 20 rhythm, capped 60; next direct contact adds meter/6 and consumes it | Timing is based on casts, including defense, not only hits |
| Latchspider | +35% speed inside its own fields | Its authored webs also slow the opponent; force it off the network |
| Flintroc | Gains 3 charge per idle tick, max 100; direct contact adds up to 16 (meter/6) and consumes it | Cast timing and shot reliability matter; a missed projectile does not spend charge |
| Oathhound | A frontal guarded contact arms +8 on the next direct hit; guard prevents lunge interruption | The defense is directional and temporary |

Knockback, healing, shield and counter formulas use integer rounding. Multiple contacts in a single tick still have a defined stable order: the first eligible contact can consume a stored payoff. A shotgun does not receive a stored one-hit bonus on every pellet. Damage-over-time is excluded from lifesteal and these offensive passive bonuses.
