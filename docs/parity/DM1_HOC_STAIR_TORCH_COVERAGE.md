# DM1 HoC stair torch real-media coverage

The critical Hall of Champions torch is stored on map 0 at cell `(3,14)`,
beside the stairs at `(3,15)`. The authentic Thing chain contains wall
ornament sensor `0x4c98` (sensor 152, ordinal 7), storage sensor `0x4c07`
(sensor 7, type 13/data 4, ordinal 8), and TORCH weapon Thing `0x5463`
(icon 4). These identifiers are asserted from the loaded original dungeon
media; the test does not create or patch map data.

`m11_dm1_hoc_stair_torch_atari_st`,
`m11_dm1_hoc_stair_torch_amiga20`, and `m11_dm1_hoc_stair_torch_fmtowns`
start their selected original ZIP through the M12-to-M11 path. They admit an
original C127 champion, pick the torch up using the C080 wall pointer route,
equip it through the C508 action-hand slot, and descend the original stairs.
The final light assertion reads M11's F0337-backed light result. These tests
cover ownership and interaction for Atari ST, Amiga 2.0, and FM Towns only;
they make no pixel or wall-art assertions.

CTest reads the original archives from `FIRESTAFF_DM1_DATA_DIR`, supplied by
`FIRESTAFF_DM1_WORKSPACE_DATA_DIR` at configure time. Missing archives are
reported as skipped tests. Original game data is not copied into the source
tree or added to Git.

The IIGS startup harness also checks the original HoC route for both 2.0 and
2.1. After naturally recruiting ELIJA, it walks the original corridor to the
stair wall, verifies the same sensors and Thing, picks up the torch through a
native viewport click, equips it in an action hand, and descends through the
original stairs command. The next-level F0337 light result must be positive.
The IIGS branch dispatches left clicks inside its live viewport through the
shared C080 handler; its native inventory and champion controls remain owned
by their GS tables. The harness covers original-media interaction and light
only, not IIGS wall pixels.

Reference point: `tests/test_m11_dm1_hoc_wall_material_receipt_pc34.c` covers
the corresponding PC34 wall-art and C080/C508 interaction path using the
original PC34 archive. Keep the PC34 graphics assertion scoped to PC34's
bound `GRAPHICS.DAT` material.
