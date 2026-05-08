# DM1 V1 pass422 fireball/lightning explosion creature-source audit

Primary source: ReDMCSB `PROJEXPL.C` (`/home/trv2/.openclaw/data/redmcsb-n2-build-probe/ibm-pc-i34e-fires/HARDDISK/SOURCE/PROJEXPL.C`).

## Anchors

- `F0213_EXPLOSION_Create`, lines 123-148: fireball/lightning creation switches to target coordinates, computes the attack roll, then checks party-square damage first; creature-square damage is in the `else` branch.
- `F0213_EXPLOSION_Create`, lines 137-142: when the target group is non-material, fireball/lightning attack is quartered before fire-resistance subtraction and `F0191_GROUP_GetDamageAllCreaturesOutcome`.
- `F0217_PROJECTILE_HasImpactOccured`, lines 413-417: non-material creatures ignore non-`HARM_NON_MATERIAL` projectile impacts before normal group damage is computed.
- `F0219_PROJECTILE_ProcessEvents48To49`, lines 693-697 in the same file (audited in combined lineage too): a false impact result keeps the projectile moving/rescheduled rather than consuming it.
- `F0220_EXPLOSION_ProcessEvent25_Explosion`, lines 665-673: delayed fireball/lightning explosion event handles door destruction; creature/party damage for the immediate fireball/lightning blast is anchored in `F0213`.

Secondary lineage cross-check: flattened Firestaff/CSBWin-derived probes contain the same `F0213`/`F0220` blocks around `firestaff_combined_frontends_probe.c` lines 13087-13123 and 13542-13558, but ReDMCSB remains authoritative.

## Implemented in pass422

- `memory_projectile_pc34_compat.c:F0822_EXPLOSION_Advance_Compat` now preserves the `F0213` party-first/else-group split for fireball/lightning explosion damage.
- Group fireball/lightning damage is quartered for `destCreatureIsNonMaterial` before emitting the Phase 13 `CombatAction_Compat`.
- Fire-resistance random subtraction remains caller/Phase-13-side because `CellContentDigest_Compat` currently carries only type/non-material data, not resistance nibbles.
