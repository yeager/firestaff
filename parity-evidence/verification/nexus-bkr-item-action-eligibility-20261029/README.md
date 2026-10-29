# Nexus ITEM.IBS action eligibility and BKR field evidence

Read-only pcrecomp/Ghidra analysis of the authentic Japanese retail
`DM.BIN`, SHA-256
`3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`, imported
at `0x06010000` as big-endian SH-2. The preserved decompilation logs are
Ghidra output copies with trailing whitespace trimmed; no other log content
was changed. `SHA256SUMS` covers these copies and this report.

## Supported relationships

- `FUN_0603a472` checks a four-index selector against records addressed from
  the ITEM.IBS metadata base at `0x06064610`, with 40-byte record stride. In
  the hash-pinned Japanese `DM.BIN`, the pointer literal at file offset
  `0x2a660` is `06 06 46 10`, which resolves big-endian to `0x06064610` when
  the image is loaded at `0x06010000`. The preserved
  `pointer-resolution.txt` records this direct retail-byte check. The helper
  returns an eligibility result; the table's domain names are not recovered.
- `FUN_0603a55c` and `FUN_0603a602` are direct callers that read and update a
  separate selector state at champion-row offsets `+0x35`, `+0x36`, and
  `+0x38`. Additional computed call references are retained in
  `menu-callers.log`.
- `FUN_06039f76`, called by the selector helper on some paths, reads runtime
  champion-row `+0x18` and contributes to its threshold result. The domain
  meaning of the saved field remains unknown.
- The authentic Magic composer `FUN_0602a234` instead writes its entries at
  `+0x3e + cursor` and tracks cursor at `+0x3d`. No call from that composer to
  `FUN_0603a472`, or from the helper to a proven Magic cast path, is shown by
  these references. Authentic A and Down+A captures also leave `+0x35..+0x38`
  unchanged.

## Interpretation boundary

The four helper indices are not evidence of the composer's four rune entries.
The ITEM.IBS path's broader gameplay role and any relationship to the visible
Magic window remain unproven. Do not assign names to the selector categories,
the `+0x18` field, or any effect from this analysis. Continue by resolving the
computed callers and correlating an authentic runtime context with the
selector state and the separate Magic composer.

## Preserved logs

- `pointer-resolution.txt`: hash-pinned retail image identity, load address,
  pointer-literal offset and bytes, and the resolved big-endian address.
- `input-dispatch-decomp.log`: retail event dispatcher, composer, and
  `FUN_0603a472` decompilation.
- `decomp-targets.log`: composer helpers and `+0x35..+0x38` selector callers.
- `field-reader-followup.log`: `FUN_06039f76` and BKR-related field readers.
- `menu-callers.log`: recorded direct and computed references to the selector.
