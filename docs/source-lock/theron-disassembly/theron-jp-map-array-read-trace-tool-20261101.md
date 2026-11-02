# JP Akutuba map-array read trace instrumentation (2026-11-01)

## Purpose and source boundary

The authentic JP Akutuba map arrays occupy physical HuC CD-RAM range
`[0x0e8b03, 0x0e8e09)`, a total of 774 bytes. The parser-established logical
array spans are `$6B03` (48 bytes), `$6B33` (247 bytes), `$6C2A` (323 bytes),
and `$6D6D` (156 bytes). In the saved state where MPR3 is `$74`, these physical
bytes map to logical `$6B03..$6E08`. The ranges and lengths describe
authenticated source layout only; they do not assign tile semantics.

## Working-tree instrumentation

The current version of
[`instrument_mednafen_theron_pce_fast_movement_context_trace.py`](../../../scripts/instrument_mednafen_theron_pce_fast_movement_context_trace.py)
adds `map_array_read` rows for that physical half-open range. The rows use the
existing shared `event_sequence` and include the observed logical and physical
address, byte value, guest PC, physical PC, and MPRs. Map-array reads have a
separate counter from the pre-array movement-data reads, so earlier reads do
not consume the map-array quota.

The instrumenter defaults to 128 rows and clamps configured values outside
1–4096 back to 128. The wrapper passes
`THERON_CAPTURE_PCE_FAST_MOVEMENT_CONTEXT_SAMPLE_LIMIT` through, defaulting to
128. Rows beyond the configured sample limit now emit one
`map_array_read_truncated=sample_limit` marker. Three authentic state-only
captures ran with a limit of 4096, recorded 1,677, 1,513, and 1,556 map-array
events, and did not reach the sample cap. Each run's bytes matched the
hash-verified JP Track 02 source. They observed 17 distinct addresses across
Map 0 and Map 3; maps 1 and 2 had no reads in these bounded captures. This
does not establish that unobserved bytes or maps are unused. See the
[capture report](theron-jp-state-only-map-array-read-capture-20261101.md).

## Evidence still required

The next authentic same-session trace must join the reader at `$50D2/$50D6`
with pre-instruction A/X/Y/P and opcode bytes, plus the Page1-backed operand
bytes `$2A..$33` used by the PCE Fast interpreter, the resolved read around
`$4FA7`, dynamic `map_array_read` rows, active-map selection, and the tile at
the captured party pose. Join those events to authenticated Track 02 source
reads and an atomic VDC/VCE/VRAM snapshot. Until that join is captured and
validated, this instrumentation is only a discovery aid: it proves neither
active-map identity, movement, rendered output, nor native parity.

See the [candidate reader disassembly](theron-jp-akutuba-map-index-reader-20261101.md),
[state-only UP/control comparison](theron-jp-state-only-up-control-data-read-20261101.md),
and [JP active-map capture chain](../../../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).
