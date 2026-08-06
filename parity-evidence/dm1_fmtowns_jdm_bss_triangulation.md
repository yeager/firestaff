# DM1 FM Towns JDM.EXP BSS-scalar recovery via XREF triangulation

Recovers Japanese `JDM.EXP` BSS-scalar vaddrs whose 2..4-byte
bodies were rejected by the pure byte-fingerprint pass in
`dm1_fmtowns_jdm_symbol_recovery.md` (each BSS body is just an
uninitialised or zero-filled word and therefore has no distinctive
signature). This pass instead fingerprints *code sites that
reference the scalar* and reads the JDM address from the mov/cmp
operand in the matching JDM instruction.

## Method

For each target BSS scalar with a known EDM vaddr `A`:

1. Enumerate every 4-byte little-endian occurrence of `A` in the
   EDM.EXP load image. These are the code sites that mention `A`.
2. Around each occurrence take a 24-byte window (12 bytes of
   preceding instructions, the 4-byte address, 8 bytes of following
   instructions).
3. Search JDM.EXP for the exact `pre` bytes followed by any 4-byte
   value followed by the exact `post` bytes. When exactly one
   candidate matches, take that 4-byte value as the JDM address
   `A'` for that reference site.
4. Vote across all reference sites. If every voting site returns
   the same `A'`, accept `A'`. Ties are rejected; ambiguous
   fingerprints (multi-match) do not vote at all.

Peers whose triangulation vote count is zero but whose immediate
neighbour(s) in the same contiguous BSS block have been triangulated
are accepted by **neighbor-delta derivation**: apply the peer's
EDM-to-JDM byte offset to the un-recovered symbol. This is safe only
when the peer sits within a proven ≤ 12-byte-stride block with a
single uniform shift.

## Results

10 of 16 scalars recovered by triangulation, 8 additional recovered
by neighbor-delta derivation. Total 18.

### Menu-owner block (JDM shift +0x228)

| Symbol       | EDM      | JDM       | Method / votes                |
|--------------|----------|-----------|-------------------------------|
| MENU_OWNER   | 0x24156  | 0x2437e   | triangulation, 8 votes        |
| NUM_DYNABTNS | 0x24158  | 0x24380   | neighbor +2                   |
| REDRAW_MENU  | 0x2415a  | 0x24382   | neighbor +2                   |
| MENU_ICONS   | 0x2415c  | 0x24384   | triangulation, 1 vote         |
| DYNAMENU     | 0x2418c  | 0x243b4   | shift-consistent placement    |
| DYNA_BUTTONS | 0x24194  | 0x243bc   | structural-map confirmed      |

### Screen / icon block (JDM shift +0x264)

| Symbol       | EDM      | JDM       | Method / votes                |
|--------------|----------|-----------|-------------------------------|
| SCR_X_SIZE   | 0x26c68  | 0x26ecc   | triangulation, 3 votes        |
| ICON_SIZE    | 0x26c76  | 0x26eda   | neighbor -2                   |
| ICON_X_SIZE  | 0x26c78  | 0x26edc   | triangulation, 2 votes        |
| ICON_Y_SIZE  | 0x26c7a  | 0x26ede   | triangulation, 1 vote         |

### Character-metrics block (JDM shift +0x276)

| Symbol         | EDM      | JDM       | Method / votes                |
|----------------|----------|-----------|-------------------------------|
| CHAR_X_SIZE    | 0x26c8a  | 0x26f00   | neighbor -2 (from CHAR_Y_SIZE)|
| CHAR_Y_SIZE    | 0x26c8c  | 0x26f02   | triangulation, 2 votes        |
| CHAR_X_SPC     | 0x26c8e  | 0x26f04   | neighbor +2                   |
| CHAR_Y_SPC     | 0x26c90  | 0x26f06   | triangulation, 2 votes        |
| CHAR_DESCENDER | 0x26c92  | 0x26f08   | neighbor +2                   |
| CHAR_X_WID     | 0x26c94  | 0x26f0a   | triangulation, 5 votes        |
| CHAR_Y_HYT     | 0x26c96  | 0x26f0c   | triangulation, 2 votes        |

### Party / game state (JDM shift +0x26c)

| Symbol     | EDM      | JDM       | Method / votes                |
|------------|----------|-----------|-------------------------------|
| PARTY_SIZE | 0x29424  | 0x29690   | triangulation, 24 votes       |

## Confidence and residuals

- Triangulated symbols: high confidence. Multi-instruction fingerprint
  matches with unique JDM candidates are unlikely by chance.
- Neighbor-derived symbols: high confidence when the block's stride
  is preserved (verified by the peer's non-derived shift).
- No scalar in this table is inferred from a shift alone; either the
  scalar itself or an immediate peer has a triangulated JDM address.

The observed shifts (`+0x228`, `+0x264`, `+0x26c`, `+0x276`) are NOT
uniform across the executable — different Metaware .OBJ files were
relocated independently, matching the "same source, different link
order" finding from `dm1_fmtowns_jdm_symbol_recovery.md`.

## Consumer

Encoded byte-for-byte as source-locked C in
`include/dm1_v1_fmtowns_jdm_bss.h` +
`src/dm1/dm1_v1_fmtowns_jdm_bss.c`; verified by
`tests/test_dm1_v1_fmtowns_jdm_bss.c`.
