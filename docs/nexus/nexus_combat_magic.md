# Nexus magic in combat: evidence and implementation status

**Updated:** 2026-10-29

This page replaces the earlier DM1-derived spell formulas, potion list, and
combat-effect claims. Those values came from diagnostic or shared Firestaff
code and are not verified retail Nexus behavior.

The current evidence establishes a byte match for a 32-halfword table in the
Japanese retail `DM.BIN`, six numeric composer choices with their observed
mana costs, and the Y input that opens the Magic window. It does not establish
rune meaning, complete-spell acceptance, combat targeting, damage,
healing, buffs, creature magic, or any post-cast state mutation. Details and
limits are in [`nexus_spells.md`](nexus_spells.md).

The production target links fail-closed spell operations. Do not enable
`nexus_v1_magic.c` or infer combat effects from DM1/DM2. To promote a combat
spell, capture its authentic input, retail consumer, state writes, target,
random inputs, and visible result from a reproducible pre-action save.
