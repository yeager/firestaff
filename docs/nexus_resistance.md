# Nexus V1 — Magic Resistance

**Audit date:** 2026-08-13
**Sources:** `src/nexus/nexus_v1_champions.c`, `include/nexus_v1_champions.h`,
`src/nexus/nexus_v1_magic.c`, `src/nexus/nexus_v1_spell_effects.c`,
`src/nexus/nexus_v1_combat.c`, and the retail Nexus `DM.BIN` disassembly.

## Current binding

Nexus champions have `anti_magic` and `anti_fire` fields. They are loaded from
the authenticated 64-byte `PLRD` records at offsets 17 and 18. The loader does
not invent replacement values when an authenticated PLRD source is absent.
The fields are serialized by the save code and the party shield/fire-shield
paths increase the corresponding field.

The fields are not yet consumed by a complete hostile-spell or fire-damage
route. This is deliberately different from claiming that the resistance
system is complete.

## What the retail evidence shows

The disassembly identifies a status-effect hit test around `DM.BIN` address
`0x0204E2`, including caster power, target defense, a random component, and
anti-magic effectiveness. The current `nexus_v1_spell_effect_debuff()` API
only receives an already-selected target status array. It has no caster stat,
target defense, or authenticated retail RNG/target-routing context, so it only
applies the requested status and does not run that resistance check.

No authenticated Nexus creature-resistance consumer is currently bound in
`nexus_v1_creatures.c`, and no source-bound `anti_fire` reduction path exists
in combat. DM1 resistance documentation is a reference only; its values and
formulas must not be copied into Nexus.

## Status

- Champion resistance fields: **implemented from PLRD and serialized**.
- Shield stat changes: **implemented**.
- Hostile status resistance: **not admitted; source inputs are missing**.
- Creature resistance: **not admitted; retail consumer is missing**.
- Fire resistance during damage: **not admitted**.

## Remaining work

- Bind the `DM.BIN` status-test inputs to an authenticated Saturn combat route.
- Bind creature resistance values to their retail consumer before adding them
  to the creature model.
- Bind fire-damage reduction to the retail damage route.
- Capture an authentic startup/combat/save path proving how PLRD resistance
  values are consumed.
