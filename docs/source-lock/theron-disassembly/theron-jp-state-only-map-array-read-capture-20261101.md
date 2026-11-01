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
bytes or maps are unused. The events record the read address, byte, guest PC, physical PC, MPRs, frame,
and shared event sequence. They do not record contemporaneous A/X/Y or the
Page1-backed operand bytes used by the PCE Fast indirect-indexed instructions.

The sparse address/value set is uniquely located at the expected Akutuba
source offsets in the authenticated raw BIN. The five-byte anchor
`10 30 00 B4 00` at raw offset `$B10E1` occurs seven times in the BIN; only
that occurrence matches all 17 dynamic observations. The other six anchor
occurrences each disagree with two observations. This authenticates the
observed bytes' source match, not the meaning of the reader PCs or map bytes.

## Static source-matched reader opcodes

The four recorded reader PCs were checked against the authentic JP BIN's
MODE1/2048 user-data projection and the same state's `HuC.ROMSpace` mapping
(banks `$68` and `$70`). A unique surrounding code window contains each
state-resident PC. The shorter windows below are evidence for the static decode,
not a trace-time opcode-byte capture; execution may modify or relocate code.

| Trace PC | Bank | State bytes at PC | Static instruction | Unique user-data window (start PC / projection offset; SHA-256) | Raw BIN instruction offset |
|---|---:|---|---|---|---:|
| `$50FE` | `$68` | `B1 14` | `LDA ($14),Y` | 256 bytes, `$507E` / `0x8287e`, SHA-256 `0727d1be2a1996e3a7ff99cde69f0bb702631d12b058e6d51edb1800578e48f1` | `$95EFE` |
| `$4FFC` | `$68` | `B1 14` | `LDA ($14),Y` | 256 bytes, `$4F7C` / `0x8277c`, SHA-256 `ad909004f49881dc4d30c3578a5e36342521d88f68c1b894f93095c5c43aa58a` | `$95CCC` |
| `$4FD4` | `$68` | `B1 14` | `LDA ($14),Y` | 256 bytes, `$4F54` / `0x82754`, SHA-256 `b564ba2754884f38d30c33002908c94016ac20c4c2e74d17bd1087c06a208bf3` | `$95CA4` |
| `$AE66` | `$70` | `7D F6 6D` | `ADC $6DF6,X` | 128 bytes, `$AE26` / `0x92626`, SHA-256 `2e39186314b9cd90625bccb827d92c699461551e9ced421b3e2e649b8e04e247` | `$A8136` |

`$B1 14` is indirect-indexed LDA. In the isolated Mednafen 1.32.1 PCE Fast
implementation, `GetIY` reads its 16-bit base from `HU_Page1[0x14..0x15]`, adds
the live Y register, then calls `RdMem`. `HU_Page1` is backed by the MPR[1]
page pointer in this core; the reference state's saved MPR[1] is `$F8`. The
next trace should therefore record these page-1 operand bytes and their MPR
mapping, rather than label them as values read through `MPR[0]`. The `$AE66`
indexed read addresses `$6DFD` and `$6DFF` imply X values 7 and 9, respectively,
if the state-resident opcode is unchanged at execution. The capture did not
record these registers, prove the fetched bytes matched the state, or bind the
reads to a map-selection routine. The PCE Fast
interpreter keeps X, Y and P in locals during execution, so a future hook must
read `X_local`, `Y_local` and `P_local` at opcode fetch; `HuCPU.X/Y/P` can be
stale there. With lazy flags enabled, record `HuCPU.ZNFlags` as well if the
architectural N/Z state is needed. These instructions describe access forms
only; they do not identify cell meaning,
active map, party pose, collision or rendering. The opcode forms were checked
against the HuC6280 implementation in the isolated Mednafen 1.32.1 source
(`src/pce_fast/huc6280_ops.inc`, cases `$B1` and `$7D`;
`src/pce_fast/huc6280.cpp`, `GetIY` and `LD_ABX`).

The `$4FFC` and `$4FD4` context windows cross MODE1 sector payload boundaries,
so their projection offsets are not contiguous raw-BIN ranges. The listed raw
offsets identify the instruction bytes after applying the sector projection.

## Limits and next evidence

This state-only replay does not establish which map is active, what the
candidate `$50D2` list selects, how the observed reads relate to party pose,
collision or object logic, or whether any byte reaches the viewport. It
produced VDC/VCE/VRAM snapshots, but those have not been joined to a
pose-bound map tile. Continue with a same-session capture of instruction
contexts at the observed reader PCs, the `$50D2/$50D6` inputs, resolved `$4FA7`
access, active-map selector, pose-bound tile consumer and atomic video state. Keep unknown list and tile semantics opaque.

The raw trace files and emulator state remain outside Git. The first
movement-context trace SHA-256 is
`ee843e48d29ac86fbbe986d161d1445b9d3d307400ed4bd09d4305ecc81878ab`; the
second and third are `793f4822f9264964d10905afaea59c244fa49f2df9e7e78db073f5cbc856b15a`
and `25949871b41b14ac16b7eaa100f7226c13827c68a239af5e46d38f8911c996d7`.
See the [reader disassembly](theron-jp-akutuba-map-index-reader-20261101.md),
[instrumentation note](theron-jp-map-array-read-trace-tool-20261101.md), and
[active-map capture chain](../../../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).
