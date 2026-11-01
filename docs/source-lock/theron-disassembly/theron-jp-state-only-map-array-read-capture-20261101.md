# JP state-only map-array read capture (2026-11-01)

## Capture identity

Three isolated Mednafen 1.32.1 PCE Fast runs autoloaded the same authentic JP
save state (MD5 `8bf0caf3a98b5748d2a78ba8016d2f01`) with no scripted input. The
trace-only receipt reports `autoload_state_load_status=verified` and
`transition=missing` for each run. Each bounded run ended at its requested
wall-clock timeout; this is not a cold boot or CD handoff capture. The
reference binary was built against standalone SDL 2.32.10 with the
map-array-read hook and a per-kind sample limit of 4096.

The authenticated JP Track 02 file under `.firestaff/data/theron` has MD5
`b7afb338ad31be1025b53f9aff12d73a` and SHA-256
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`. Its
parser-established array bytes begin at raw BIN offset `$B10D3`; the observed
physical HuC CD-RAM range is `[0x0e8b03, 0x0e8e09)`.

## Observations

The first run recorded 1,677 `map_array_read` rows and 17 distinct physical
bytes. Two additional sequential runs recorded 1,513 and 1,556 rows. All three
reported MPR3 `$74`, emitted no `map_array_read_truncated` marker, and had zero
byte mismatches against the authenticated BIN at
`raw_offset = $B10D3 + physical_address - $0E8B03`. The normalized ordered
address/value/PC/MPR tuples in each shorter run matched the corresponding
prefix of the first run; capture lengths differed because the runs ended on a
wall-clock timeout.

| Parser array | Read rows, run 1 | Distinct bytes read | Observed reader PCs (run 1) |
|---|---:|---:|---|
| Map 0, `[$6B03,$6B33)` | 1,599 | 15 of 48 | `$50FE` (663), `$4FFC` (585), `$4FD4` (351) |
| Map 1, `[$6B33,$6C2A)` | 0 | 0 of 247 | — |
| Map 2, `[$6C2A,$6D6D)` | 0 | 0 of 323 | — |
| Map 3, `[$6D6D,$6E09)` | 78 | 2 of 156 | `$AE66` (78) |

The three runs observed the same 17 distinct addresses and values. These are
dynamic reads within the parser-established arrays, not evidence that other
bytes or maps are unused. The events record the read address, byte, guest PC,
physical PC, MPRs, frame, and shared event sequence. They do not record
contemporaneous A/X/Y or the reader's zero-page inputs.

The sparse address/value set is uniquely located at the expected Akutuba
source offsets in the authenticated raw BIN. The five-byte anchor
`10 30 00 B4 00` at raw offset `$B10E1` occurs seven times in the BIN; only
that occurrence matches all 17 dynamic observations. The other six anchor
occurrences each disagree with two observations. This authenticates the
observed bytes' source match, not the meaning of the reader PCs or map bytes.

## Limits and next evidence

This state-only replay does not establish which map is active, what the
candidate `$50D2` list selects, how the observed reads relate to party pose,
collision or object logic, or whether any byte reaches the viewport. It
produced VDC/VCE/VRAM snapshots, but those have not been joined to a
pose-bound map tile. Continue with a source-locked disassembly of the observed
reader PCs and a same-session capture of the `$50D2/$50D6` inputs, resolved
`$4FA7` access, active-map selector, pose-bound tile consumer and atomic video
state. Keep unknown list and tile semantics opaque.

The raw trace files and emulator state remain outside Git. The first
movement-context trace SHA-256 is
`ee843e48d29ac86fbbe986d161d1445b9d3d307400ed4bd09d4305ecc81878ab`; the
second and third are `793f4822f9264964d10905afaea59c244fa49f2df9e7e78db073f5cbc856b15a`
and `25949871b41b14ac16b7eaa100f7226c13827c68a239af5e46d38f8911c996d7`.
See the [reader disassembly](theron-jp-akutuba-map-index-reader-20261101.md),
[instrumentation note](theron-jp-map-array-read-trace-tool-20261101.md), and
[active-map capture chain](../../../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).
