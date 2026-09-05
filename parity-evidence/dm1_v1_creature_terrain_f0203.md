# DM1 V1 creature terrain parity: GROUP.C F0203

## Gap

The live M10 creature-tick move path used the generic context passability
helper. That helper treated every pit as passable and admitted only door
states 0, 1, and 5 for every creature. This lost two source-owned inputs:
the real PC 3.4 `CreatureInfo.Attributes` word and the destination door
Thing's `Vertical` bit. The result was gameplay-visible: ground creatures
could walk over open pits, while tall creatures and non-material creatures
could be stopped by door apertures that the original admits.

## Source evidence

ReDMCSB `Toolchains/Common/Source/GROUP.C`, function `F0203`, lines
1491-1551 checks open-pit movement against `MASK0x0020_LEVITATION`, then
checks door state against either 1 for a horizontal door or
`M051_CREATURE_HEIGHT(Attributes)` for a vertical door. Destroyed doors and
creatures with `MASK0x0040_NON_MATERIAL` are admitted. `DEFS.H` line 1609
defines creature height as Attributes bits 7-8.

The Firestaff runtime now joins the moving group's real creature type to its
PC 3.4 G0243 Attributes row and reads the real destination DOOR Thing's
orientation before making the terrain decision. Missing creature metadata
fails closed. No generated creature profile, inferred orientation, or other
runtime fallback is introduced.

## Retail-data provenance

The local authenticated input used for the focused run is
`~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip`, containing the
33,357-byte `DATA/DUNGEON.DAT` and 363,417-byte `DATA/GRAPHICS.DAT`. The
runtime code exercised by this fix receives decoded C04 group and C00 door
records from that DUNGEON.DAT path; the focused pure regression separately
locks the source truth table at the terrain boundary.

## Verification

`tests/test_dm1_v1_creature_terrain_passability_pc34_compat.c` covers ground
and levitating creatures on an open pit, both vertical-door height outcomes,
the horizontal-door threshold, non-material passage, and destroyed doors.
