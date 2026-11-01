# JP Akutuba map-array read trace instrumentation (2026-11-01)

## Purpose and source boundary

The authentic JP Akutuba map arrays occupy physical HuC CD-RAM range
`[0x0e8b03, 0x0e8e09)`, a total of 774 bytes. The parser-established logical
array spans are `$6B03` (48 bytes), `$6B33` (247 bytes), `$6C2A` (323 bytes),
and `$6D6D` (156 bytes). In the saved state where MPR3 is `$74`, these physical
bytes map to logical `$6B03..$6E08`. The ranges and lengths describe
authenticated source layout only; they do not assign tile semantics.

## Working-tree instrumentation

The current working-tree version of
[`instrument_mednafen_theron_pce_fast_movement_context_trace.py`](../../../scripts/instrument_mednafen_theron_pce_fast_movement_context_trace.py)
adds `map_array_read` rows for that physical half-open range. The rows use the
existing shared `event_sequence` and include the observed logical and physical
address, byte value, guest PC, physical PC, and MPRs. Map-array reads have a
separate counter from the pre-array movement-data reads, so earlier reads do
not consume the map-array quota.

This change has not yet been built or exercised in an authentic capture. The
instrumenter defaults to 128 rows and clamps configured values outside 1–256
back to 128. The wrapper passes
`THERON_CAPTURE_PCE_FAST_MOVEMENT_CONTEXT_SAMPLE_LIMIT` through, defaulting to
128. Therefore a complete individual-array sweep of the 323-byte Map 2 needs
both an instrumenter ceiling of at least 323 and a wrapper setting of at least
323 (preferably 512). To retain 774 read events from all four arrays, use at
least 774; this covers every array byte only if each is read once. Repeated
reads consume rows too. A capped or partial trace cannot establish that unread
arrays were unused.

## Evidence still required

The next authentic same-session trace must join the reader at `$50D2/$50D6`
with contemporaneous A/X/Y and zero-page `$2A..$33`, the resolved read around
`$4FA7`, dynamic `map_array_read` rows, active-map selection, and the tile at
the captured party pose. Join those events to authenticated Track 02 source
reads and an atomic VDC/VCE/VRAM snapshot. Until that join is captured and
validated, this instrumentation is only a discovery aid: it proves neither
active-map identity, movement, rendered output, nor native parity.

See the [candidate reader disassembly](theron-jp-akutuba-map-index-reader-20261101.md),
[state-only UP/control comparison](theron-jp-state-only-up-control-data-read-20261101.md),
and [JP active-map capture chain](../../../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).
