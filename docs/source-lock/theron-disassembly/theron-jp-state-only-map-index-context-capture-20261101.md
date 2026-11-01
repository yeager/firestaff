# JP state-only map-reader context capture (2026-11-01)

## Capture identity and acceptance

A new isolated Mednafen 1.32.1 PCE Fast build enabled the v4 movement-context
hook and used standalone SDL 2.32.10. Its binary SHA-256 is
`498d8576acf9afee9541e03ac2ca7ac743641d6a98827885d8a0a5e9fbf48245`.
The wrapper loaded the authentic emulator-created JP state
(MD5 `61a1155e9bbbb0628bda2bda21aee85d`) and accepted two bounded 20-second
state-only captures: one `UP` replay (`up@61:120`) and one no-input control.
The capture IDs were `9e4b482f027a424d9b989a6555efd729` and
`077f3325dd424314a79697292539d7d4` respectively. Both ended by the requested
timeout, which is expected for these bounded captures.

This is not a cold boot or CD handoff. Each receipt reports zero CD IRQs,
zero CDDA commands, and `transition=missing`. The replay trace verifies the
scripted UP at guest input poll `$44C1` with raw controller bits `$0010` on
frames 61–180 (240 polls). No `movement_command_write` row was observed. This
proves guest input consumption only; it does not prove movement, position
change, or a rendered response. The captures used the same authentic JP Track
02 identity (MD5 `b7afb338ad31be1025b53f9aff12d73a`). Raw traces and emulator
states remain on TRV2 and are not committed.

## Dynamic map-array reads

The UP replay's v4 movement-context sidecar contains 946 `map_array_read`
rows, 946 `map_array_instruction_context` rows, and 1,908
`map_index_reader_instruction_context` rows (636 each at `$50D2`, `$50D6`, and
`$4FA7`). All map-array reads report MPR3 `$74`; their shared event sequences
increase, and the sidecar has no map-array or context truncation marker. The
four observed reader PCs are `$50FE`, `$4FFC`, `$4FD4`, and `$AE66`. The trace
observes 24 distinct physical array addresses: 22 within the Map 0 span and
two within Map 3. It does not observe Map 1 or Map 2 reads in this capture.

Comparing physical addresses against the authenticated raw BIN with the
established source projection
`raw_offset = 0xB10D3 + physical_address - 0x0E8B03` gives exact byte matches
for 23 of the 24 observed addresses. The remaining address,
`$0E8B14` (raw offset `$B10E4`), is read as `$BC` 50 times; the authenticated
BIN contains `$B4` there. The same address/value discrepancy is present in the
no-input control. The isolated target-write trace recorded no write to this
address during the replay window. Because the state was autoloaded, the capture
does not show how this byte acquired its saved value or establish loader
provenance. Keep the byte and its meaning unresolved.

The first 946 normalized map-array address/value/PC/MPR tuples in the UP
replay match the no-input control exactly. Their captured frame ranges differ
(1–262 for UP; 1–305 for control), so this prefix comparison is not evidence
that UP has no effect. The matched observations do not bind the reads to a
selected map, party pose, tile consumer, or video update. The raw movement-
context trace SHA-256 for the UP replay is
`fbc2050b80b2fafbe709ca6b7d146befb690cf54fa040851e7a8d81765464cae`.

## Next source-bound step

Trace writers to the `$0E8B14` storage across a cold, authenticated JP loader
route, then join the saved and live byte to the active-map selector and the
pose-bound tile consumer. Preserve opcode/register/Page1/MPR context at the
three `$50D2`/`$50D6`/`$4FA7` sites and the four map-array readers, followed by
atomic VDC/VCE/VRAM state. Do not infer movement, collision, map selection, or
tile semantics from the saved-state read alone. The complete acceptance chain
remains in the [Theron TODO](../../../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).
