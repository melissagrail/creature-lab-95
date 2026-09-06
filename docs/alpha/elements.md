> Historical v5 terrain release. Terrain interactions remain in v6; [Tinikami](tinikami.md) updates pacing, energy, visuals and validation.

# Elements alpha 0.5

Water, ice, mud and oil create reusable terrain decisions. The C++ engine owns every interaction; the renderer draws public state and the model receives semantic surface tokens. The existing wind field, steam, brush, charged water and wind vane remain active. See the [wind foundation](terrain.md) for its controls.

## Terrain contracts

| Surface | Combat behavior | Reactions |
|---|---|---|
| Water | Extinguishes burn, changes traction, enables wet passives; blue arrows show local current | Chill freezes; heat makes steam; shock charges |
| Ice | Faster travel, low traction; stored current is suspended | Heat, expiry or a heavy contact restores water (or mud if frozen from mud) |
| Mud | 50% voluntary movement, kills voluntary inertia; Miretoad gets 90% | Splash washes into water; chill freezes for 6 seconds; heat dries it away |
| Oil | Low traction without a speed bonus | Heat consumes it into 6 seconds of fire; splash does not remove unlit oil |
| Brush | Slows movement to 65% | Heat or adjacent fire ignites it |
| Fire | 4 HP per second to either creature | Spreads through touching brush/oil once per second, one adjacency generation per pulse |
| Charged water | Wet terrain, current remains active; 3 HP per second to either creature | Existing elemental transformations still apply |
| Steam | Drifts with global wind and slows projectiles | Temporary obscuring-looking ground; does not hide actors from observations |

Heavy ice-breaking contacts must deal damage and either have at least 900 absolute impulse, or be melee/lunge attacks from a creature of mass at least 140. Breaking ice itself adds no burst damage.

Currents sum across overlapping wet patches, capped at length 32, then scale by 100/body mass. They push even planted/rooted creatures, so positioning upstream matters before committing to a cast. Frozen flow is stored and resumes when the patch returns to water. Current arrows are distinct from the arena-wide wind grid. Wind continues to affect travel speed and projectiles, without pushing idle bodies.

## Creator kits

| Creature / move | New terrain role |
|---|---|
| Tidecoil / Rain Basin | Water with current strength 24 |
| Kelpwidow / Tangle Bed | Water with current strength 14 |
| Dewotter / Fresh Spring | Water with current strength 12 |
| Miretoad / Mire Pool | Mud; the caster traverses it more efficiently |
| Gravemole / Upheaval | Creates a mud patch with radius 1500 for 7 seconds |
| Slagjaw / Slag Trail | Oil radius 1900 for 8 seconds |
| Waxwyrm / Wax Glob | Projectile leaves oil radius 1400 for 6 seconds when it hits, expires or is absorbed |

Created currents follow locked cast facing. Aim magnitude still controls ground placement. Wax Glob deposits oil at its actual final projectile position, not on release; its heat contact can ignite existing fuel. Existing chill/heat/shock/splash moves supply interactions across kits. See the [generated roster](species.md) for complete numbers.

Grove adds two mud patches; the open arena adds two oil patches. Existing water patches now have authored currents. Try Tidecoil against Rimehare for freeze/thaw control, or Waxwyrm against Pyrelisk on the open arena for fuel management.

## Bounded area control

Ground creation refreshes nearby same-kind owned patches first. At five live owned patches, a new placement replaces that owner's patch with the shortest remaining lifetime. Conversions can take ownership of additional neutral patches without allocating new slots. Other owners' patches are never evicted by creation. Fire propagation snapshots adjacency before ignition, preventing slot-order chain reactions in a single pulse.

Overlapping fire applies only one 4-HP ground pulse per creature each second; charged water likewise applies one 3-HP pulse. Different hazard kinds can combine, and spell-zone damage remains separate. This preserves zoning without unlimited overlap damage.

## Validation

- 644,566 alpha assertions and 279 environment assertions passed, including current transport during planted casts, frozen flow restoration, heavy ice breaking, mud transformations, delayed oil payloads, one-hop ignition, damage caps and patch replacement.
- AddressSanitizer and UndefinedBehaviorSanitizer passed both native suites.
- Python FFI snapshot/replay, the 82,627-parameter reference policy and Gymnasium checks passed.
- Rules and observation version 5; content fingerprint `92ac508a`; shared native/Python golden replay `78b43d0560ba4a49`.
- Actor dimensions remain 3,620 floats. Surface kind normalization is now /8; surface flow and move flow strength occupy previously reserved columns. Old snapshots/models fail version compatibility checks.
- 56,160 scripted matches: zero allocation overflow, 52.01% seat-A score, 22.40-second mean duration, species aggregate scores 20.9–77.3%. These are baseline diagnostics, not balance certification or trained-agent results. [Full report](../../reports/elements-v5-balance.md).
