# Firestaff 1.9.3

## Bug Fixes

### Champion mirrors now visible in Hall of Champions
Implemented the ReDMCSB random wall ornament system (F0169-F0172). Previously,
wall ornaments were only loaded from sensor things, completely missing the
deterministic pseudo-random ornament placement that makes champion mirrors,
fountains, and other decorative wall features appear throughout the dungeon.

The fix reads per-face ornament permission bits from each wall square byte and
computes the ornament index using the original PRNG algorithm:
`((value1 * 31417 >> 1) + (value2 * 11) + ornamentRandomSeed) >> 2) % 30`

### Source lock
- DUNGEON.C F0169 (line 2371): `F0169_DUNGEON_GetRandomOrnamentIndex`
- DUNGEON.C F0170 (line 2382): `F0170_DUNGEON_GetRandomOrnamentOrdinal`
- DUNGEON.C F0171 (line 2407): `F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals`
- DUNGEON.C F0172 (line 2466): `F0172_DUNGEON_SetSquareAspect` WALL case
- DEFS.H: `MASK0x0001-0x0008` wall ornament flags

## Known Issues
- Some side walls at certain positions may still be missing (occlusion filter investigation ongoing)
- Swoosh animation not yet connected to boot sequence
- Entrance screen accepts keyboard Return (original PC34 behavior; click-only mode planned)
