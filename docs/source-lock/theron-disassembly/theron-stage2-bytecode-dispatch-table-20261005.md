# Theron's Quest Stage-Two Bytecode Dispatch Table

## Evidence and scope

The authenticated raw Track 02 BIN files contain the same 170-byte dispatch
table at HuC6280 address `$410d`:

| Edition | Track 02 MD5 | Stage-two user-data start | Table file offset | Table FNV-1a |
|---|---|---:|---:|---:|
| US | `f23601102138f87c33025877767ebf76` | `0x2bed90` | `0x2bee9d` | `0x7f6a7f04` |
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | `0x2be460` | `0x2be56d` | `0x7f6a7f04` |

The stage-two user-data starts are raw sectors 1224 (US) and 1223 (JP), plus
the 16-byte MODE1/2352 sector header. The loader enters the stage-two payload
at `$4000`; therefore CPU `$410d` is file offset `$10d` from those starts.
These offsets and the 17-sector load/entry are recorded in
`docs/source-lock/tqr_v1_track02_ipl_loader_2026-07-11.md`.

At `$40dc`, the interpreter reads one byte through `($1c)`, doubles it, and
uses it as the X offset of `JMP ($410d,x)`. `$410d..$41b6` is consequently an
85-entry little-endian pointer table, ending immediately before the helper at
`$41b7`. The listing in
`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm` now emits
this range as 85 `.addr` entries, labels each table target, and marks the
dispatch explicitly instead of treating the table as linear instructions.

## Index-to-target map

No gameplay or command names are assigned. Each right-hand value is only the
static HuC6280 jump target reached by the corresponding table index.

```text
00:41c5 01:41cb 02:41d8 03:41de 04:41e6 05:41ec 06:41f0 07:41f4
08:4214 09:4253 0a:4254 0b:4259 0c:4263 0d:4271 0e:4280 0f:4288
10:4291 11:42d3 12:4319 13:45f0 14:45f8 15:45fe 16:4615 17:461d
18:4623 19:4635 1a:4629 1b:462f 1c:4345 1d:4497 1e:4433 1f:445f
20:4647 21:464f 22:4234 23:42fb 24:4334 25:4916 26:4910 27:45ca
28:4375 29:43dd 2a:4409 2b:4653 2c:4674 2d:468f 2e:46ca 2f:4794
30:47a6 31:47c5 32:47d3 33:469d 34:44bd 35:46b8 36:4361 37:480a
38:47f3 39:4842 3a:485f 3b:447f 3c:4862 3d:489f 3e:48ac 3f:4901
40:491b 41:42be 42:4973 43:4995 44:45eb 45:49ab 46:49b4 47:49bb
48:4a5e 49:44eb 4a:4a81 4b:4aca 4c:49d3 4d:49e8 4e:4a3b 4f:4a14
50:4a1b 51:4a42 52:4a50 53:49fb 54:484e
```

## Initial comparison and cursor-transfer handlers

The first eight roots form a common control-flow cluster. Index `$00` at
`$41c5` calls `$41b9`, which loads a 16-bit pointer from the current `$1c`
window at offsets `+1/+2` into `$1c/$1d`, then returns through `$40e4` with
`A=0` to the `$40cc` loop. The dispatcher clears Y before each handler and
`$41b9` increments Y before each pointer-byte read. Indices `$01..$04` call `$41f8`: from dispatcher
entry (`Y=0`), it reads offsets 1 and 2, uses the offset-1 byte as X for the
lookup at `$2780,X`, and compares that lookup result with the raw offset-2
byte. Indices `$05..$07` call `$4203`, which reads offsets 1 and 2 as two X
indices and compares their respective `$2780,X` lookup results. The
`$40f0..$421f` interval containing this code is byte-identical in the US and
JP deinterleaved payloads.

| Index | Static comparison path | Pointer replacement via `$41cf` |
|---:|---|---|
| `$01` | `$41f8`, then branch on Z | when Z is set |
| `$02` | `$41f8`, then branch on Z | when Z is clear |
| `$03` | `$41f8`, branch on C then Z | only when C=1 and Z=0 |
| `$04` | `$41f8`, branch on C | when C is clear |
| `$05` | `$4203`, then branch on Z | when Z is set |
| `$06` | `$4203`, then branch on Z | when Z is clear |
| `$07` | `$4203`, then branch on C | when C is clear |

Those compare flags route each handler either through `$41cf` (the same
`$41b9` pointer replacement) or through `$41d5` to `$4101`. The new code range
in the da65 info file roots `$4101..$410c` separately from the adjacent
dispatch table, so the three shared-step stubs decode correctly: `$4101`
loads 5, `$4105` loads 7, and `$4109` loads 9; each reaches `$40e4` to add
that amount to `$1c/$1d` and resume at `$40cc`. This establishes instruction
and cursor-update paths for this cluster, but does not prove that any of its
indices occur in a valid retail stream or assign names to them.

## ID `$4b`: counted indexed-byte comparison

Dispatch entry `$4b` points to `$4aca` in both authentic editions. The
byte-locked `$4aca..$4af6` body in
`tests/test_theron_v1_stage2_disassembly_chain.c` proves this bounded flow:
the handler reads an 8-bit count from stream offset `+1`, clears zero-page
`$00`, and calls its local pair checker once per count iteration. Each pair
loads an X index from the stream and compares the next byte against
`$2780,X`; operand `$ff` bypasses the comparison, while a non-matching value
decrements `$00`. This describes the visible operations only: the candidate
stream, contents of `$2780`, and purpose of the comparison are not
established.

If `$00` is zero after the loop, control jumps to `$41c5`; `$41b9` loads a
little-endian replacement cursor from byte-sized Y offsets `(2+2*count) mod
256` and `(3+2*count) mod 256`, then `$41c5` installs it and resumes dispatch
without adding to the old cursor. If `$00` is nonzero, the handler adds three
to Y and passes that byte to `$40e4`, advancing the old cursor by
`(4 + 2*count) mod 256` and skipping the two replacement-cursor bytes. The
count is not range-checked; the decrement-and-branch loop treats an encoded
zero as 256 iterations, and the one-byte mismatch accumulator can wrap. Do
not infer an all-match predicate or gameplay/resource-selection name from
this static path.

## ID `$51`: staged helper chain

Dispatch entry `$51` points to `$4a42` in both authentic editions. The
byte-locked handler reads stream offset `+1` into `$4ec2` through `$49e1`,
reads offset `+2` into `$4ec1`, calls `$4d0e`, then reaches `$40f9` for the
shared three-byte cursor step. The bounded `$4d0e..$4d66` helper path first
calls `$4d15`; a carry result branches to `$4d67` (`BRK`), otherwise it calls
`$4d4a`. `$4d15` saves and restores `$4d7b`, copies `$4ec3/$4ec4` to
`$4ec5/$4ec6`, calls `$4f48` and `$4ec9`, then stages `$4ec2` through
`$4d68/$4d69`. `$4d4a` sets A to `$04` for `$4f5e`, branches to the same
`BRK` on carry, stages `$4d68/$4d69` through `$4d7b/$4ec2`, calls `$4ef4`,
and restores `$4d7b` from `$4ec1`. The adjacent `$4d6a..$4d78` variant
reuses `$4d15`, sets bit `$80` in `$4ec1`, then calls `$4d4a`. Dispatch ID
`$52` at `$4a50` reads offset `+1` into `$4ec2` and offset `+2` into
`$4ec1`, calls `$4d6a`, then takes the shared `+3` cursor step at `$40f9`.
These byte-level paths do not assign meanings to the fields, calls, carry
conditions, or a retail stream.

## ID `$4d`: two-operand helper handoff

Dispatch entry `$4d` points to `$49e8` in both authentic editions. The
byte-locked `$49e8..$49fa` body in
`tests/test_theron_v1_stage2_disassembly_chain.c` reads stream offsets `+2`
and `+3` into `$4ec5` and `$4ec6`, respectively, then calls `$4c30`. On
return, the inline `$49f9` branch reaches `$49db`, which jumps to `$40fd` for
the shared four-byte cursor advance. The preceding `$49e1` helper reads
offset `+1` into `$4ec2`. This records the visible handoff and cursor path
only; `$4c30` effects, operand meanings, and a valid retail stream using this
dispatch index remain unproven.

## Indexed-byte handlers and nested cursor path

The next table roots expose another byte-level group. Index `$0b` at `$4259`
uses `$41f8` to place the offset-1 byte in X, then stores the offset-2 byte
into `$2780,X`; index `$0c` at `$4263` adds that offset-2 byte to `$2780,X`
with carry clear, and index `$0d` at `$4271` subtracts it with carry set.
These three paths reach `$40f9`, the shared `+3` cursor step. Index `$0e` at
`$4280` increments `$2780,X` and index `$0f` at `$4288` decrements it; both
reach `$40f5` (`+2`). Their call to `$41f8` also reads the byte at offset 2,
but the shared cursor step leaves that byte at the next dispatch position.
These are exact memory operations only; `$2780` entry meanings are unknown.
The `stage2_id0b_0f_indexed_mutation` assertion byte-locks the shared `$41f8`
operand reader, all five roots, and their `$40f9`/`$40f5` tails independently
against authentic US and JP Track 02.

Index `$10` at `$4291` reads the offset-1 byte, uses it as an index into a
little-endian pointer table based at `$6800`, saves the original `$1c/$1d`
cursor, and calls `$40cc` using the selected pointer. On return it restores
the original cursor and takes the `$40f5` (`+2`) path. Index `$09` at `$4253`
is a one-instruction `RTS`, compatible with ending such a nested call, but no
authentic stream binds the selector, table entry, and return instruction as a
pair. The nested execution path is therefore a static call-graph observation,
not proof of a valid retail stream structure. The
`stage2_id09_id0a_id10_nested_cursor` source-lock assertion checks the `$09`
RTS byte at `$4253`, the `$0a` root at `$4254`, the `$10` cursor-save/restore
root, their dispatch words, and the `$6800` pointer-reader helper against both
authentic Track 02 editions.

The same authentic 17-sector payload contains the bytes addressed as `$6800`
when loaded at `$4000` (payload offset `$2800`). Its first 13 little-endian
words are the following same-region pointers in both US and JP; the next word
is `$0000`:

| Selector | Pointer | First byte at target | Dispatch entry | Statically visible step |
|---:|---:|---:|---:|---|
| `$00` | `$6e98` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$01` | `$6c4e` | `$21` | `$464f` | Fixed call argument `$03`; cursor `+1` |
| `$02` | `$6c92` | `$12` | `$4319` | Loads inner cursor from offsets `+1/+2`, then restores saved cursor; `+3` after indirect call |
| `$03` | `$6cfa` | `$12` | `$4319` | Loads inner cursor from offsets `+1/+2`, then restores saved cursor; `+3` after indirect call |
| `$04` | `$694d` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$05` | `$69a1` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$06` | `$6a16` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$07` | `$6a7c` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$08` | `$6ae2` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$09` | `$6b48` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$0a` | `$6bbd` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$0b` | `$6c13` | `$08` | `$4214` | Reads offset 1; cursor `+2` |
| `$0c` | `$681c` | `$08` | `$4214` | Reads offset 1; cursor `+2` |

Every target begins with an ID in the recovered 85-entry dispatch map. This
byte-backed pointer prefix connects the `$4291` lookup code to retail
payload addresses at the static-source level. The zero word after these 13
entries is only observed data: the code shown above performs no selector
bounds check, so it is not established as a runtime limit or sentinel. No
capture yet proves which selector is actually read or executed.

The step notes follow the rooted handler bytes, not a claim that any pointer
was selected. `$4214` reads the following stream byte into A before calling
`$421c`, then advances through `$40f5` (`+2`). `$464f` supplies constant
`$03` to `$3ab7` and advances through `$40f1` (`+1`). `$4319` replaces the
current cursor from its embedded `+1/+2` pointer, performs an indirect call
through `$201c`, restores the saved cursor on return, and advances through
`$40f9` (`+3`). The call target and the behavior of `$3ab7` are outside this
step summary. The `stage2_id12_indirect_call` assertion byte-locks the `$4131`
dispatch word and `$4319` handler against authentic US and JP Track 02; it
does not identify the selected target or bind a retail stream to this path.
The `stage2_id08_local_helper` assertion byte-locks the `$411d` dispatch word,
the `$4214` root, and the `$421c` helper against both authentic editions; the
conditional branch after `$4f5e` remains only statically observed.
The `stage2_id13_id16_fixed_arguments` assertion binds table entries `$13` and
`$16`, their fixed-argument roots `$45f0/$4615`, and the shared `$40f1` `+1`
step. The `$3ab7` callee effects and retail stream execution remain unknown.
The `stage2_id17_id1b_fixed_argument_roots` assertion locks IDs `$17..$1b`,
their converging `$463b` operand reader, and `$4641` continuation in both
authentic editions. It does not assign meanings to the immediate values or
callee effects.
The `stage2_id20_id21_fixed_arguments` assertion also locks their `$4647/$464f`
roots and convergence path against authentic US and JP bytes; the `$3ab7`
effects remain unassigned.
The `stage2_id1e_id1f_bounded_handlers` assertion locks the complete `$1e`
root/helper pair (`$4433..$445e`) and adjacent `$1f` root (`$445f..$447e`),
including both dispatch words, in both authentic editions. The bytes establish
bounded instruction paths only, not operand meanings or runtime selection.
The `stage2_id1c_id1d_cursor_roots` assertion locks the complete roots at
`$4345` and `$4497`, their dispatch words, and the adjacent `$4101` `+5` and
`$4105` `+7` cursor-step stubs in both authentic editions. The roots read
bytes via `$1c` and call `$3ab7`; those bytes establish fixed read/cursor
patterns only. Stream structure, called-helper effects, and runtime selection
remain unknown.
The `stage2_id24_bounded_wait_root` assertion locks dispatch index `$29`
(`$24`), its complete `$43dd..$4402` path, and the `$4403` polling helper in
both authentic editions. The handler calls `$4403` before and conditionally
after its `$e03c` call, then takes the `$40f5` (`+2`) cursor path. This is
bounded byte/control-flow evidence only: neither helper meaning nor stream
selection is established.
The `stage2_id23_regional_handoff` assertion locks the `$4153` table word and
complete `$42fb..$4318` root in both authentic editions, including the
regional call operand at `$4314/$4315` (`$56af` US, `$5729` JP). The selected
helper body is byte-locked separately by `stage2_id2b_regional_handoff`; this
does not establish selector execution, helper effects, or gameplay meaning.
The `stage2_id27_bounded_pointer_setup` assertion locks the `$415b` dispatch
word and complete `$45ca..$45ea` handler path in both editions. It copies the
bytes at `$442f/$4430` to zero page, derives `$300a` from `$4d7b` and `$3008`,
calls `$3ab7` with immediate `$05`, then takes `$40f5` (`+2`). These are
static byte/control-flow observations; the pointer's role, callee effects,
and runtime selection remain unknown.
The `stage2_id2a_entry_helper` assertion locks the `$4161` dispatch word,
`$4409..$4414` entry, and `$4415..$442e` local helper in both editions. The
entry calls expansion-ROM routines before and after a local helper call; the
local code reads through `$1c`, indexes `$4b3c`, and writes fixed scratch
bytes. External effects, table meaning, and runtime selection remain unknown.
The `stage2_id2d_overlapping_poll_root` assertion locks index `$2d` at
`$468f` through its `$40f5` cursor tail in both editions. Starting at that
root, the bytes read the next `$1c` byte, clear and compare `$3b33`, loop on
the carry branch, then advance by two. Because `$468f` overlaps a different
linear decode, this is root-specific byte/control-flow evidence only; the
counter's role and stream execution remain unknown.
The `stage2_id2e_bounded_windows` assertion locks the `$4169` dispatch word
and selected authentic windows: the `$46ca` branch/MPR prefix, alternate
`$474a..$4769` BIOS-call path, `$476a..$4788` local pair loop, and
`$4789..$4793` operand reader. It does not lock the entire `$46ca` branch or
its long BIOS setup path. BIOS/callee effects, bank mapping, `$0060` table
contents, and runtime selection remain unresolved.
The `stage2_id2f_parameter_handoff` assertion locks index `$2f` at `$4794`
through `$40f5` (`+2`) in both editions. It loads the following stream byte
into `$f8`, sets `$ff` to `$0b`, then calls `$e0d8` and `$4b2d`. This is only
bounded byte/control-flow evidence; both callees' effects and retail stream
selection remain unknown.
The `stage2_id30_overlapping_branch_root` assertion locks dispatch index
`$30` at `$47a6` and the complete `$47a6..$47c4` byte window in both editions.
The conditional target `$47b9` enters the middle of a linear decode; from the
root, the zero branch calls `$e0d8` with fixed bytes and the nonzero branch
uses the stream byte at `$f8` before another `$e0d8` call. Both visible paths
jump to `$40f5`. This is root-relative byte/control-flow evidence only; BIOS
effects and runtime stream selection remain unknown.
The `stage2_id31_fixed_argument_handoff` assertion locks index `$31` at
`$416f`, its exact `$47c5..$47d2` 14-byte root, and the `$40f1` (`+1`) tail
in both editions. It loads fixed values into `$f8/$ff`, calls `$e0d8`, and
jumps to the shared cursor helper. BIOS effects and retail stream selection
remain unknown.
The `stage2_id32_conditional_handoff` assertion locks the `$4171` dispatch
word and complete `$47d3..$47f2` path in both editions. Its two branches load
different fixed `$f8/$ff` values, call `$e0d8`, and converge on `$40f5` (`+2`).
The branch condition's meaning, BIOS effects, and retail stream selection
remain unknown.
The `stage2_id33_four_byte_handoff` assertion locks index `$33` at `$4173`
and the complete `$469d..$46b7` path in both editions. It reads four bytes
from the current `$1c` cursor, stores them at `$0402..$0405`, and jumps to
`$4101` (`+5`). These static stores do not establish the hardware/data roles
or prove retail stream execution.
The `stage2_id34_fixed_argument_select` assertion locks the `$4175` dispatch
word and complete `$44bd..$44e6` branch tree in both editions. It reads one
cursor byte, selects one of the fixed `$3ab7` arguments `$10,$11,$12,$16`,
and converges on `$40f5` (`+2`). The selector and argument meanings, callee
effects, and runtime stream selection remain unknown.

The 17 authenticated stage-two user-data sectors contain no direct absolute
`STA`, `STX`, `STY`, or `STZ` encoding to `$201c` in either US or JP. This
does not rule out indexed/indirect writes, DMA copies, or initialization
outside this payload, so the indirect-call vector remains unresolved.

### Selectors `$04..$0c`: common static prefix

The nine pointers for selectors `$04..$0c` each begin with the same
eight-byte candidate prefix in both editions: `$08,$00,$25,$26,$16,$21,$45`
followed by one byte. Applying the rooted cursor steps maps those bytes as
`$08` (`+2`), `$25` (`+1`), `$26` (`+1`), `$16` (`+1`), `$21` (`+1`), then
`$45` (reads its offset-1 byte and advances `+2`). The final byte of each
prefix is therefore read by `$45` at offset 1; it is `$b2` for selectors
`$04..$0a`, `$cf` for `$0b`, and `$d3` for `$0c`.

| Selector | Target | Eight-byte prefix |
|---:|---:|---|
| `$04` | `$694d` | `$08,$00,$25,$26,$16,$21,$45,$b2` |
| `$05` | `$69a1` | `$08,$00,$25,$26,$16,$21,$45,$b2` |
| `$06` | `$6a16` | `$08,$00,$25,$26,$16,$21,$45,$b2` |
| `$07` | `$6a7c` | `$08,$00,$25,$26,$16,$21,$45,$b2` |
| `$08` | `$6ae2` | `$08,$00,$25,$26,$16,$21,$45,$b2` |
| `$09` | `$6b48` | `$08,$00,$25,$26,$16,$21,$45,$b2` |
| `$0a` | `$6bbd` | `$08,$00,$25,$26,$16,$21,$45,$b2` |
| `$0b` | `$6c13` | `$08,$00,$25,$26,$16,$21,$45,$cf` |
| `$0c` | `$681c` | `$08,$00,$25,$26,$16,$21,$45,$d3` |

The source-lock listing continues each rooted prefix below. These target
streams overlap linear instruction decoding, so the byte-table range includes
overlap context; the actual selector roots remain the addresses in the table.
Selector-to-pointer runtime use is still unobserved.

### Selectors `$04..$0c`: conditional continuations

The common prefix reaches `$12` at target `+$37` for selectors `$04..$0a`
and `$0c`; selector `$0b` reaches `$12` at `+$30`. Every suffix below is
conditional on `$4319`'s indirect call through `$201c` returning. Cursor
offsets are relative to that selector's table target.

| Selector | Rooted suffix after `$12` returns | Conditional boundary |
|---:|---|---|
| `$04` | `+$40: $41->$73b2/$73b4`; `+$43: $41->$7470/$7472`; `+$46: $11`; then `+$50: $13`, `+$51: $2d` | `$11`'s regional callee and `$3b33` poll must return |
| `$05` | `+$40: $36,$00,$02`; `+$43: $41->$7464/$7466`; `+$46: $11`; then `+$50: $13`, `+$51: $2d` | `$11`'s regional callee and `$3b33` poll must return |
| `$06` | `+$40: $36,$01,$00`; `+$43: $41->$7470/$7472`; `+$46: $11`; then `+$50: $13`, `+$51: $2d` | `$11`'s regional callee and `$3b33` poll must return |
| `$07` | `+$40: $36,$00,$00`; `+$43: $41->$7470/$7472`; `+$46: $11`; then `+$50: $13`, `+$51: $2d` | `$11`'s regional callee and `$3b33` poll must return |
| `$08` | `+$40: $36,$00,$00`; `+$43: $41->$7464/$7466`; `+$46: $11`; then `+$50: $13`, `+$51: $2d` | `$11`'s regional callee and `$3b33` poll must return |
| `$09` | `+$40: $36,$00,$00`; `+$43: ($14,$00,$00),($15,$00,$00),($14,$02,$02),($15,$02,$02)`; `+$55: $11`; then `+$59: $13`, `+$60: $2d` | `$11`'s regional callee and `$3b33` poll must return |
| `$0a` | `+$40: $41->$73b2/$73b4`; `+$43: $41->$7470/$7472`; `+$46: $11`; then `+$50: $13`, `+$51: $2d` | `$11`'s regional callee and `$3b33` poll must return |
| `$0b` | `+$33: $41->$73b2/$73b4`; `+$36: $41->$7470/$7472`; `+$39: $13`; its `+1` step reaches `$2d` | `$3b33` poll must exit |
| `$0c` | `+$40: $41->$73b2/$73b4`; `+$43: $41->$7446/$7448`; `+$46: $01` | `$01` compares mutable `$2781`; details below |

The byte pairs above follow rooted handlers: `$36` reads two bytes and steps
`+3`; `$11` reaches `$40fd` (`+4`) after its regional call; `$14/$15` each
read one operand and step `+3`. The `$41` targets `$73b2/$73b4` and
`$7470/$7472` have the static return paths described in selector `$00`.
Targets `$7464` US / `$7466` JP are suffixes of a longer matching byte stream:
each reads three `$14/$15` pairs with operands `$02,$01,$00` and reaches
`$09`/`RTS`. The containing roots `$7446` US / `$7448` JP begin the same
eight-pair sequence with operands descending `$07..$00`, then `$09`/`RTS`.
`theron-stage2-da65.info` marks `$7445..$7478` as bytes to preserve this
overlap; `$7445` is context before the US root, and `$7445..$7447` is
context before the JP root. These are static return paths only if the nested
interpreter is entered and returns.

For selector `$0c`, after the `$7446/$7448` nested call returns, `$01` at
target `+$46` reads index `$01`, compares `$2781` with the byte at `+$48`,
and either advances by five when unequal or loads the little-endian pointer
at `+$49/+50` when equal. The seven consecutive comparison entries test
values `$00..$06` and point to `$686d,$688d,$68ad,$68cd,$68ed,$690d,$692d`;
each target begins `$1a,<matching value>,$13,$2d`. A mismatch after the
seventh row reaches `$686d` as fall-through. Each 32-byte target has the same
remaining cursor pattern through a final `$09`/`RTS`, with only the compared
value and regional `$12` pointers changing. This is conditional on the
`$3b33` polls, the `$11/$2b` calls, and the `$201c` indirect targets
returning. The stream does not establish the runtime value of `$2781` or
prove this selector executes.

Each of these seven blocks has `$2b,$02,<row>` at offsets `+$10..+$12`,
where `<row>` is `$00..$06`. The rooted `$4653` handler for ID `$2b` consumes
those two operands, preserves the first across two local calls, invokes the
regional `$56af/$5729` helper, and reaches the shared `+3` cursor step. Its
next dispatch ID is therefore the `$1a` at `+$13` in this static walk. The
regional helper's behavior and actual execution remain unresolved.

The rooted IDs in each block occur at target-relative cursors
`+$00,$02,$03,$05,$06,$0a,$0c,$10,$13,$15,$18,$1a,$1c,$1f`:
`$1a,$13,$2d,$20,$3e,$2d,$11,$2b,$1a,$12,$17,$2d,$12,$09`. Their fixed
steps account for all 32 bytes when each conditional handler path returns.
The first two `$2d` operands are `$02`; the third is `$03`. The `$12` words
are `$7552/$7554` and `$74f2/$74f4` for US/JP respectively; both remain
conditional on the runtime `$201c` vector selecting returning callees.

### Dispatch ID `$11`: overlapping code root

The `$11` dispatch path calls `$5e27` in US and `$5e57` in JP. Rooting the
US entry exposes `STZ $4f9c`; the following bytes overlap the linear `$5e2b`
decode and are the seven-byte HuC6280 `TII` descriptor `$4f9c -> $4f9d`,
length `$0037`. The two relative calls then target `$5e4d` and `$5e40` (JP:
`$5e7d` and `$5e70`). The continuation loads `$0c/$0d`, stores them at
`$4fd9/$4fda`, and returns. The first callee sets VDC registers `$02/$03`,
transfers 64 bytes from `$5e5f` to `$0404`, then returns; the second copies
`$4fdb/$4fdc` to `$4fd5/$4fd6` and returns. The TII opcode and its operand
bytes are marked as a rooted descriptor in the info file so the overlapping
linear decode is not mistaken for the ID `$11` path.

The bounded caller/callee bytes contain no direct `$3b33` access. This does
not rule out effects through other code or runtime state and does not establish
that a candidate stream reaches ID `$11`.

### Rooted `$2b` handler: regional helper handoff

Dispatch table entry `$2b` targets `$4653` in both editions. The rooted
continuation reads stream offsets `+1/+2`, pushes the first value, places the
second in X, and calls `$4b00` then `$4f48`. It copies `$4d79/$4d7a` to
`$4fdb/$4fdc`, restores the pushed value to A, calls `$56af` (US) or `$5729`
(JP), and jumps to `$40f9` for the shared three-byte cursor advance. The
regional call operand at `$466f` is now asserted against both raw editions.
For selector `$0c`'s seven candidate blocks, the handler receives A=`$02` and
uses it as an index multiplied by two into the pointer table reached through
`$4fd9/$4fda`; the selected word is added to that base and written to
`$0c/$0d`. Its `$56af/$5729` prefix saves X at `$4f91`, computes that pointer,
calls `$5e40/$5e70`, then tests whether the selected pointer is zero. The
`$572c/$57a6` helper swaps the `$0a/$0b` and `$0c/$0d` pointer pairs. The
`$573f/$57b9` check ORs the two bytes at `($0c)` and calls `$5761/$57db` when
nonzero. That helper adds the word at `($0c)` to `$4fd9/$4fda`, stores the
result in `$0a/$0b`, and advances `$0c/$0d` by two. The candidate row supplies
X=`<row>` before two local calls, but their effects on X are not established.
Later helper effects and runtime execution remain unproven.

### Rooted `$2c` handler: external helper handoff

Dispatch table entry `$2c` targets `$4674` in both editions. Its `$4483`
helper increments Y, stores stream offset `+1` at `$4ec2`, calls `$4ec9`,
copies `$4ec3/$4ec4` to zero-page `$00/$01`, and returns. The caller then sets
Y to `$02`, reads stream offsets `+2/+3/+4` into `$02/$03/$0e`, loads A with
`$0f`, calls `$3ab7`, and jumps to `$4101`, the shared five-byte cursor
advance. `$4ec9`'s work and the `$3ab7` implementation are outside this
bounded decompilation; `$3ab7` is below the stage-two `$4000..$7fff` image.
This establishes static setup and cursor movement only, not runtime effects or
a gameplay meaning.

The rooted `$4ec9` entry decrements `$5b`, copies `$4ec2` to `$37cc`, then
calls `$4f31` and `$3a2e`. On the no-carry path it copies `$37ce/$37cf` to
`$4ec3/$4ec4` and `$37d0/$37d1` to `$4ec7/$4ec8`; both visible outcomes clear
`$5b` and return. `$4f31` forms a pointer at `$02/$03 = $4d7c`, doubles the
byte at `$4d7b` into Y, loads the indexed word through `($02),Y` into
`$00/$01`, and returns. These bytes are locked in both editions by the raw
Track 02 test. `$3a2e` is below the stage-two image, so its implementation and
the carry/result contract at this call remain unresolved; no gameplay meaning
is assigned to the copied fields.

The logical target `$3a2e` lies in the `$2000..$3fff` MPR1 window. The direct
stage-two entry instructions at `$4000` write MPR3..MPR6, then call `$8000`
before continuing; the MPR1 state across `$8000` and its helpers is not
established by the entry-byte check. The separate authenticated `MPR1=$f8`
receipt from the `$de21` backup-RAM writer cannot be transferred to this call.
A valid disassembly source must join the `$3a2e` execution with MPR1 and its
physical PC/bank, or prove the loader span that sets that mapping.
The original `pce` trace build has a bounded probe at `$4ec9` and `$3a2e`, but
its `$4ec9` row reads candidate bytes rather than proving that `$3a2e` executes.
An equivalent `pce_fast` instruction-loop probe now emits MPR1, executing
physical PC, and 64 mapped bytes only when the actual instruction PC is
`$3a2e`. The isolated trv2 build compiled; a cold authentic-media run and a
run loading the authentic JP gameplay state did not execute that target before
their strict capture gates stopped the runs.

The JP F5 state (SHA-256
`2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee`) provides
a separate, later gameplay snapshot from the authenticated JP Rev. 1 CUE
(MD5 `85706e7c2f658bc2792511d618dfc7a5`), raw Track 02
(MD5 `b7afb338ad31be1025b53f9aff12d73a`), and System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`). Its `CPU` section records PC `$c692` and
MPRs `$ff,$f8,$68,$78,$79,$72,$69,$00`, so that saved instruction maps to
physical `$0d2692`. The `HuC` section's `ROMSpace + $68 * 8192` bank image
provides the code window below; da65 V2.18 decodes the captured bytes, but no
function/gameplay semantics are assigned. At that saved frame MPR1 is `$f8`,
and logical `$3a2e` addresses BaseRAM offset `$1a2e`; the corresponding saved
BaseRAM byte is zero. This later-state observation cannot be transferred to
the earlier `$4ec9` call and does not establish the `$3a2e` mapping or bytes
when the helper is invoked.

```asm
; Authentic JP gameplay-state byte window, logical $c662..$c6e1
; Captured PC: $c692, MPR6=$69, physical PC=$0d2692
LC679:  bbr1    $9d,LC68c
        jsr     LC5e1
        stx     LC686
        sta     LC687
        .byte   $73
LC68c:  bbr2    $9d,LC69f
        jsr     LC5e1
        stx     LC699             ; captured PC $c692
        sta     LC69a
        .byte   $73
LC69f:  bbr3    $9d,LC6b2
        jsr     LC5e1
        stx     LC6ac
        sta     LC6ad
        .byte   $73
LC6b2:  pla
        beq     LC6c1
        lda     $9f
        jsr     L44e7
        pla
        sta     $a1
        pla
        sta     $a0
        ply
LC6c1:  sty     $20
        lda     #$01
        bbr0    $27,LC6ca
        lda     #$09
LC6ca:  jsr     L44fb
        ldx     $24
        beq     LC6ec
```

The listing is a byte decode rooted at the saved PC, not a complete routine
boundary or a substitute for the pending same-execution `$3a2e` receipt. Keep
the helper locked until an authentic run captures its actual MPR1/physical PC
and mapped bytes.

## Bounded `$8000` entry-callee dataflow

The authenticated US Rev. 1 bytes bind `$8000` to the first `$4000` entry
call. Its body clears VDC registers, calls `$45a6`, saves the returned
zero-page pair `$00/$01` into `$4c/$4d`, and derives the `$47cb..$47ce` and
`$47d1/$47d2` pointer fields from that pair and bytes read through `($4c),Y`.
The bytes at `($4c),Y + 4/+5` are staged in `$47bf/$47be` and `$0e/$10` before
the call to `$4696`; `$0e/$0f` are then published at `$47c9/$47ca`. The byte at
`($4c),Y + 6` is copied to `$3b6f`, shifted left four times, and used as the
X value after clearing `$02/$03`. When `$3b68` is zero, control calls `$48fc`
before returning.

The called body at CPU `$8696` (loaded-image offset `$4696`; see
`theron-us-stage2-huc6280.asm:10049-10085`) is independently byte-bound by
`theron_v1_track02_verify_stage2_l4696_l3114()` for authentic US Rev. 1 media.
The corresponding regression is `test_stage2_l4696_l3114()` in
`tests/test_theron_v1_stage2_disassembly_chain.c`. It computes an unsigned
8-by-8 product: `$0e` is the multiplier,
`$10` is the multiplicand, and the routine clears `$0f` and `$11` before
starting, so `$11:$10` is initially the zero-extended multiplicand. It moves
the multiplier into scratch `$12`, clears `$0e`, selects the number of
shift-add iterations from the multiplier's highest set bit, then shifts `$12`
right one bit per iteration. A shifted-out set bit adds `$11:$10` into the
`$0f:$0e` accumulator; the multiplicand shifts left between iterations. The
returned 16-bit product is therefore in `$0f:$0e`, matching `$8000`'s stores
to `$47ca:$47c9`. A zero multiplier returns zero. This establishes arithmetic
behavior only; no meaning is assigned to the two input bytes or result field.

The `$48fc` entry is the second half of a loop rooted at `$48ec`. Each
iteration clears `$0404/$0405`, checks `$00`, decrements `$01` when `$00` is
zero, decrements `$00`, then returns when the OR of the two bytes is zero;
otherwise it branches back to `$48ec`. Thus the bytes implement a 16-bit
decrement-to-zero loop, including low-byte borrow. A new raw-sector regression
binds the bytes from `$48ec` through the RTS at `$4900` independently in
authentic US and JP Track 02. This proves only the byte-level loop behavior;
the caller-derived value in `$00/$01` and its purpose remain unnamed.

The paired US raw-media regression binds `$8000` and `$45a6`, including the
call sites at `$45a6`, `$4696`, and `$48fc`. It also records several da65
decode-artifact overlaps; the raw media bytes, not the rendered labels, are
authoritative for those spans. The pointed-to structure's field meanings
remain unresolved. Both `$8000` and `$4696` receipts are US-only here; this
source-lock does not prove that these spans are identical in JP. In
particular, neither receipt establishes the MPR1 mapping needed to identify
the below-window `$3a2e` call.

The adjacent `$4ef4` helper uses the same `$4ec2 -> $37cc` handoff, additionally
copies `$4ec7/$4ec8` to `$37d0/$37d1`, calls `$4f31` and `$3879`, then clears
`$5b` and returns. `$4be2` calls it directly; `$4c17` reaches it only when the
preceding `$4f5e` call clears carry. `$4f11` is a separate table-entry writer:
after `$4f31` selects a pointer, it writes `$00,$00,$60` through `($00)`.
The caller roots and helpers are now byte-locked against both editions. The
`$3879` routine remains below the loaded stage-two window, so its effects and
the runtime conditions for either caller remain unknown.

### Rooted `$2d` handler: counter poll

Although the linear listing at `$4691` decodes a different overlapping
instruction, dispatch index `$2d` enters at `$468f`. From that root, the bytes
are `INY; LDA ($1c),Y; STZ $3b33; CMP $3b33; BCS $4695; JMP $40f5`. The
handler loads its offset-1 operand into A, clears `$3b33`, and loops at
`$4695` while A is greater than or equal to the counter. It reaches `$40f5`
(`$1c += 2`) only when `$3b33` becomes greater than the operand. The listing
contains an increment at `$89e7`, reached from `$89e2` only when the stacked
value has bit `$20` set (`PLA; AND #$20; BEQ $89ed; INC $3b33; INC $2249`).
The preceding `$8975` gate independently tests `#$20` and jumps to `$49e2`
when clear; static adjacency does not prove that the gate and popped byte
share a producer. The visible setup at `$895d` pushes a byte loaded from
`$0000`, not processor status, before this epilogue. `$88a6`'s clear/nonzero
wait helper is called directly by BSRs at `$8862,$8877,$88b1,$88d9` in both
editions.

Conservative decompilation of the `$89e2` epilogue only:

```text
stack_byte = pop()
if (stack_byte & 0x20) {
    ++memory[$3b33]
    ++memory[$2249]
}
restore_mpr(0x40, pop())
restore_mpr(0x20, pop())
restore_mpr(0x10, pop())
restore_mpr(0x08, pop())
return
```

The earlier `$8975` gate and this popped-byte condition are separate static
tests. Naming either bit as an interrupt flag or as the producer of `$3b33`
would go beyond these instruction roots.
Other paths in the US listing also clear or poll `$3b33`: `$503d/$5048` clear
and wait for nonzero before calling `$51ae`; `$7539/$753c` clear and wait for
a value of at least three; `$7549/$754c` clear and wait for nonzero; and
`$7733/$7736` clear after `$4f7a` returns and wait for nonzero. `$88a6` is
another clear/nonzero wait helper. The JP payload's `$753x`/`$773x` sites
have corresponding roots two bytes later; at `$5044` its `$3b33` clear is
followed by `JSR $e063`, and its later `$50xx` control flow differs from the
US `$5048` wait.
These distinct uses do not identify which path, if any, advances the counter
during a candidate `$2d` poll. The US listing and regional raw bytes do not
prove the `$89e7` path runs during a candidate interpreter call or that any
polling condition is satisfied.

### `$88a6` counter-wait callers

The clear/nonzero wait at `$88a6` has four direct relative `BSR` callers in
both editions. Their rooted post-wait instructions differ:

| BSR site | Static continuation after `$88a6` returns |
|---:|---|
| `$8862` | Selects VDC register `$05`, masks/updates `$f3`, writes it to `$0002`, clears `$5a`, returns |
| `$8877` | Selects VDC register `$05`, masks `$f3`, writes `$0002`, then initializes `$3b78` and `$3b70..$3b77` |
| `$88b1` | Clears VDC registers through `$00/$01/$02`, then loops over `$01/$02` writes |
| `$88d9` | Loads `$27da/$27db` into `$0002/$0003`, then loops over `$01/$02` writes |

These byte-rooted continuations establish that the helper gates separate
register-write paths, but do not assign names to those operations or show
that any caller advances the `$468f` poll. The pointer scan also found no
absolute-word root for `$8975` or `$89e2`; indirect/runtime entry remains
possible and unresolved.

### Selector `$00` target: conditional static cursor walk

At `$6e98`, the candidate begins with `$08`. Applying the rooted handler
steps yields `$25,$26,$21,$1f,$3e,$1e,$41` at the cursors below. The `$1f`
handler reads bytes at offsets 1–3; `$3e` reads offsets 1–3 and both rooted
branches advance by four; `$1e` reads offset 1. This is a static walk only,
not evidence that selector `$00` executes or that the entire candidate is a
valid retail stream.

| Cursor | ID | Root | Static read/step | Next cursor |
|---:|---:|---:|---|---:|
| `$6e98` | `$08` | `$4214` | Reads `$00` at offset 1; `+2` | `$6e9a` |
| `$6e9a` | `$25` | `$4916` | No stream read; `+1` | `$6e9b` |
| `$6e9b` | `$26` | `$4910` | No stream read; `+1` | `$6e9c` |
| `$6e9c` | `$21` | `$464f` | Fixed argument `$03`; `+1` | `$6e9d` |
| `$6e9d` | `$1f` | `$445f` | Reads `$d8/$05,$00,$10` at offsets 1–3; `+4` | `$6ea1` |
| `$6ea1` | `$3e` | `$48ac` | Reads offsets 1–3; both branches `+4` | `$6ea5` |
| `$6ea5` | `$1e` | `$4433` | Reads `$d9/$06` at offset 1; `+2` | `$6ea7` |
| `$6ea7` | `$41` | `$42be` | Loads embedded pointer at offsets `+1/+2`, invokes `$40cc` recursively, then `+3` on return | `$6eaa` |

The embedded pointer bytes at `$6ea8/$6ea9` are `$b2,$73` in US and
`$b4,$73` in JP, forming static targets `$73b2` and `$73b4` respectively.
Both lie inside the loaded stage-two payload. At either target, authentic
bytes are `$1d,$00,$00,$20,$20,$00,$00,$09`. Root `$4497` reads six operand
bytes and advances by seven through `$4105`; the next `$09` maps to `$4253`,
whose handler is `RTS`. Thus a recursive `$42be` invocation has a statically
visible return path if it enters this nested stream. Selector execution is
still unobserved. The source listing marks the overlap window `$73b1..$73b9`
as bytes because these candidate stream roots overlap linear code decoding;
the actual roots are `$73b2` US and `$73b4` JP. The
`stage2_selector_00_03_pointer_roots` assertion currently checks the embedded
pointer words and recursive stream bytes, while
`stage2_id09_id0a_id10_nested_cursor` checks `$4253` and `$4497`; together
these checks preserve the byte-level return-path evidence for both regions.

If that nested `$09` returns, `$42be` resumes the outer stream at `$6eaa`.
The rooted cursor steps continue through `$20,$3e,$08,$25,$26,$08,$25`, then
bounded `$4c`/operand pairs and `$45`/operand pairs. The final known sequence
is `$34,$34,$16,$21,$1f,$1e,$27,$41` at cursors `$6f15..$6f23`; this `$41`
embeds `$73b2` US / `$73b4` JP. If it returns, the outer cursor reaches a
third `$41` at `$6f26`, embedding `$7470` in US and `$7472` in JP.

Both target-relative byte sequences are `$14,$00,$00,$15,$00,$00,$09`.
Root `$45f8` reads one zero byte and advances by three to `$15`; `$45fe` does
the same and reaches `$09`, whose `$4253` handler is `RTS`. This gives both
regional recursive paths a statically visible return when entered. The
source listing marks the containing overlap window `$7445..$7478` as bytes;
the `$7470/$7472` roots begin two bytes apart. The
`stage2_id14_id15_operand_reader` assertion separately byte-locks the
dispatch words `$4135/$4137`, the `$45f8/$45fe` roots, shared `$4604` reader,
and `$460f` cursor tail in authentic US and JP. If the nested invocation
returns, the outer cursor reaches `$6f29`, whose `$2a` dispatch target is
`$4409`. That handler clears A, calls `$e02d`, then its local `$4415` routine
reads the byte at `$1c+1` (`$00` in both regions), indexes `$4b3c`, and loads
its first byte `$03` into `$f8`. BCD increment then stores `$04` in `$fc`; the
helper also sets `$fb=$80` and `$ff=$83` before calling `$e012`. It finally
jumps to `$40f5`, whose code adds two to `$1c`. Since the external calls'
effects on `$1c` are not yet known, this does not establish the eventual
outer-stream continuation. The handler's static operations are bounded, but
external routine effects and command meaning remain unknown. The listing
marks `$6e98..$6f28` as candidate bytes; the following `$2a` has a rooted
static handler trace, not a gameplay or execution proof.

### Selector `$01` target: conditional static cursor walk

At `$6c4e`, the candidate stream begins with `$21`. If selector `$01` is
selected, each listed handler's rooted code advances the cursor to the next
row below. Handler `$3e` has internal branches, but both paths reach the same
fixed `+4` cursor step. The `$12` row reaches its next cursor only if the
indirect callee returns; the `$41` row does so only if the nested interpreter
returns. This is a static walk over authenticated bytes, not a capture that
selector `$01` was chosen or that the whole stream is valid/executed.

| Cursor | ID | Root | Static read/step | Next cursor |
|---:|---:|---:|---|---:|
| `$6c4e` | `$21` | `$464f` | Fixed argument `$03`; `+1` | `$6c4f` |
| `$6c4f` | `$16` | `$4615` | Fixed argument `$07`; `+1` | `$6c50` |
| `$6c50` | `$08` | `$4214` | Reads `$00` at offset 1; `+2` | `$6c52` |
| `$6c52` | `$25` | `$4916` | No stream read; `+1` | `$6c53` |
| `$6c53` | `$26` | `$4910` | No stream read; `+1` | `$6c54` |
| `$6c54` | `$4c` | `$49d3` | Reads `$00` at offset 1; `+2` | `$6c56` |
| `$6c56` | `$1f` | `$445f` | Reads `$aa,$00,$10` at offsets 1–3; `+4` | `$6c5a` |
| `$6c5a` | `$3e` | `$48ac` | Reads offsets 1–3; both branches `+4` | `$6c5e` |
| `$6c5e` | `$4c` | `$49d3` | Reads `$ad` at offset 1; `+2` | `$6c60` |
| `$6c60` | `$1e` | `$4433` | Reads `$ab` at offset 1; `+2` | `$6c62` |
| `$6c62` | `$27` | `$45ca` | Reads `$ac` at offset 1; `+2` | `$6c64` |
| `$6c64` | `$3e` | `$48ac` | Reads offsets 1–3; both branches `+4` | `$6c68` |
| `$6c68` | `$0b` | `$4259` | Reads `$00,$ad` at offsets 1–2; `+3` | `$6c6b` |
| `$6c6b` | `$12` | `$4319` | Loads inner cursor from offsets `+1/+2`, then restores saved cursor; `+3` on return | `$6c6e` |
| `$6c6e` | `$41` | `$42be` | Loads inner cursor from offsets `+1/+2`, calls `$40cc` recursively, then `+3` on nested return | `$6c71` |

The source-lock listing marks `$6c4e..$6c6d` and `$6c6e..$6c70` as bytes for
these bounded candidate cursor spans. The ranges record IDs and span bytes;
they do not assign operand boundaries to the trailing bytes of `$12` or
`$41`, nor claim a decoded stream past `$6c70`. In the first span the US and
JP bytes match except at `$6c6c` (`$ea` US, `$ec` JP); `$4319` reads this as
the low pointer byte at `$6c6b`, yielding `$78ea` US and `$78ec` JP. In the
second span they match except at `$6c6f` (`$b2` US, `$b4` JP); `$42be` reads
this as the low pointer byte at `$6c6e`, yielding `$73b2` US and `$73b4` JP.
These are static address derivations; the targets' runtime contents and
execution remain unverified.

If the `$4319` indirect callee returns, the cursor next reaches `$6c6e`,
whose authentic byte is `$41` in both regions. This maps to `$42be`, which
saves the outer cursor, loads another pointer through `$41b9`, invokes the
`$40cc` interpreter recursively, restores the outer cursor, and advances by
three after the nested interpreter returns. At `$6c6e`, `$41b9` reads the
embedded bytes at `$6c6f/$6c70`, forming static target `$73b2` in US and
`$73b4` in JP. Both targets are inside the loaded payload and contain the
same bounded `$1d`-plus-six-operands-then-`$09` stream described above, so the
recursive handler path has a static return through `$4253` if entered. At the
earlier `$12` cursor `$6c6b`, `$4319` reads `$ea,$78` in US or `$ec,$78` in JP
at `$6c6c/$6c6d`, forming `$78ea` or `$78ec` respectively. Those addresses
point at byte-identical 22-byte HuC6280 routine bodies in the respective
regions (`LDA $2780; STA $4ec2; JSR $4ec9; ...; RTS`), but `$4319` jumps
through RAM vector `$201c`; this static listing does not establish that the
vector selects or executes those bytes. The listing marks the bounded
three-byte `$41` span `$6c6e..$6c70`.

Selectors `$02` and `$03` each begin with `$12` at `$6c92` and `$6cfa`.
Their embedded pointer bytes are `$f2,$74` in US and `$f4,$74` in JP at
`$6c93/$6c94` and `$6cfb/$6cfc`, forming `$74f2` in US and `$74f4` in JP.
Both regions contain the same 21-byte HuC6280 routine at their respective
targets (`STZ $27d9; TMA #$04; PHA; CLC; ...; RTS`). As above, the dynamic
value of `$201c` is not established, so pointer consumption does not prove
that this routine is selected or executed.

The regenerated listing marks the pointer words and only the identified root
bytes (plus the `$08` handler's one-byte operand where its two-byte step is
static) as data. The bounded `$6c4e..$6c6d`, `$6c6e..$6c70`,
`$6e98..$6f28`, `$73b1..$73b9`, and `$7445..$7478` candidate spans are also
emitted as bytes, without assigning operand names. They do not mark
the remainder of these streams as decoded bytecode: their extents and operand
boundaries have not been established.

## Target-rooted disassembly and regional comparison

`theron-stage2-da65.info` marks `$410d..$41b6` as an address table and gives
each of its 85 target addresses a one-byte `CODE` root. This makes da65
decode from every indirect-jump destination, even where a linear sweep had
rendered the bytes as data. Two corrected examples are index `$02` at `$41d8`
(`BSR $41f8`) and index `$41` at `$42be` (`LDA $1c; PHA; LDA $1d; PHA`).
All 85 roots now have a decoded HuC6280 instruction at their entry under da65
V2.18; this confirms opcode decoding only, not that every index occurs in a
valid bytecode stream. Subsequent bytes on some paths can still be ambiguous
or remain `.byte` data because the listing is a static linear output; the root
markers do not prove complete handler control flow.
The info file also marks the 64-byte source of the US `TIA` at `$5e57` as data
(`$5e5f..$5e9e`). The resulting US source-lock listing is
`theron-us-stage2-huc6280.asm`.

To reproduce the payload, deinterleave the 17 raw MODE1/2352 sectors: keep
bytes 16 through 2063 from each sector, for US raw sectors 1224–1240 or JP
raw sectors 1223–1239, then concatenate those 2048-byte user-data portions.
A contiguous 34,816-byte copy from the first user-data offset is wrong because
it includes the next sector's 16-byte header at each boundary. The `$410d`
dispatch table is at payload offset `$010d` in the first user-data portion;
the nested pointer prefix at `$6800` is at payload offset `$2800` in the sixth
user-data portion.

The 170 table bytes are identical in both editions. In the deinterleaved
payload interval corresponding to `$4000..$4acf`, the editions differ at only
seven bytes, in five spans: `$42f6`, `$4314..$4315`, `$43be`, `$466f..$4670`,
and `$46c5`. All 85 target first bytes match. Their first 16 bytes also match
at 84 targets; the sole differing prefix is index `$35` at `$46b8`:

The other regional deltas also fall on three additional call sites and one
immediate operand in separately rooted listings. Index `$11` at `$42d3` calls
`$5e27` (US) or `$5e57` (JP) at `$42f5`; only the low operand byte `$42f6`
differs. Index `$23` at `$42fb` and index `$2b` at `$4653` call `$56af` (US) or
`$5729` (JP) at `$4313` and `$466e`, respectively (changed operands
`$4314..$4315` and `$466f..$4670`). Index `$28` at `$4375` first reads `($1c),Y` with `Y=1`
and branches on that byte. Both branches then call `$43d6`, which increments Y
and loads the byte at `($1c),Y` into `$37cc`; along this path that is the byte
at offset 2. Both branches also call `$43b5`, which saves the pointer pair
`$37d6/$37d7`, loads `$5e9f` (US) or `$5ecf` (JP) into it, calls `$37d8`,
restores the pair, and calls `$3848`. The `$43bd` byte is the `LDA #` opcode;
its immediate at `$43be` differs (`#$9f` US / `#$cf` JP). Each resulting
pointer is one byte past that edition's 64-byte TIA source span above. This
alignment is established statically; the role of the byte loaded to `$37cc`
and the pointer passed to `$37d8` remain unknown. The
`stage2_id28_conditional_handoff` source-lock assertion binds the `$415d`
dispatch word, `$4375` handler, both helpers, and the `$43be` regional
operand against authentic US and JP Track 02. These snippets establish
instruction flow and regional operands, not what any dispatch index means.

```text
US $46c4: JSR $5e4d   JP $46c4: JSR $5e7d
```

The ID `$35` source-lock test binds the table entry at `$4177` to `$46b8` and
the 18-byte handler window `$46b8..$46c9` in both authentic editions, including
the regional call operand at `$46c4`.

The surrounding control flow is byte-identical: `INY; LDA ($1c),Y; BNE $46c4;
LDA #$13; JSR $3ab7; BRA $46c7; JSR [regional target]; JMP $40f5`. The two
regional callees have the same instruction sequence, shifted by `$30`: set
`$0402` to `$e0`, set `$0403` to zero, transfer 64 bytes with `TIA` to `$0404`,
then return. Their source spans (`$5e5f..$5e9e` US and `$5e8f..$5ece` JP) are
byte-identical with FNV-1a `591d332b`. These observations establish static
control flow and data identity only; they do not assign a gameplay command
meaning or prove the index occurs in a valid stream.

ID `$36` points to `$4361` in both editions. Its 18-byte handler window
`$4361..$4372` is `INY; LDA ($1c),Y; STA $15; INY; LDA ($1c),Y; STA $14;
LDA #$14; JSR $3ab7; JMP $40f9`. The source-lock test binds that table entry
and byte window against authentic US and JP data. This records static stream
reads and control flow only; the `$3ab7` effects and runtime selection remain
unproven.

ID `$37` points to `$480a` in both editions. Its root `$480a..$4813` reads one
stream byte to `$02`, calls the local helper at `$4814`, and jumps to `$40f5`.
The helper window `$4814..$4841` is byte-identical in the authentic US and JP
images; it constructs operands from the byte in `$02`, calls `$383e`, and
returns. The source-lock test binds the table pointer and both byte windows.
No semantic meaning is assigned to the written zero-page values or to `$383e`,
whose effects and relation to a valid retail stream remain unproven. At `$4820`
the listing's alternate overlapping decode lands inside the preceding `LDA`
operand, so the bounded raw bytes, not a linear disassembly interpretation,
are the asserted evidence.

ID `$38` points to `$47f3` in both editions. Its 23-byte root
`$47f3..$4809` reads three successive stream bytes into `$0e`, `$10`, and
`$12`, loads `#$15`, calls `$3ab7`, and jumps to `$40fd` for the static `+4`
cursor advance. The source-lock test binds the table word and entire byte
window against authentic US and JP data. The values' meanings, `$3ab7`'s
effects, and runtime stream selection remain unresolved.

ID `$39` points to `$4842` in both editions. Its root `$4842..$484d` reads
one stream byte into `$4ec2`, calls the local helper `$4be7`, and jumps to
`$40f5`. The 25-byte helper `$4be7..$4bff` calls `$4f31`, copies the current
`$00/$01` pair into `$02/$03`, copies `$4ec2` into `$37cc`, calls `$37a0`,
clears `$5b`, and returns. The ID-specific root and helper windows are
byte-locked against authentic US and JP media. This establishes bounded static
data flow only; the roles of these fields, called-helper effects, and actual
retail stream selection remain unproven.

ID `$3a` points to `$485f` in both editions. Its complete three-byte root is
`JMP $40f1`; `$40f1..$40f4` loads `#$01` and branches into the shared `$40e4`
cursor-update path. The source-lock test binds the table word and stub only,
without extending into the neighboring ID `$3c` root. This statically proves
the `+1` dispatch adjustment, not what item that adjustment consumes or
whether the ID occurs in an authentic executed stream.

ID `$3b` points to `$447f` in both editions. Its four-byte root is `BSR $4483;
BRA $443f`: it calls the shared `$4483` helper already locked by the ID `$2c`
test, then enters `$443f`. A separate assertion binds this table word and root
against authentic US and JP data. This records static branch structure only;
the `$443f` continuation and runtime selection are not implied by the byte
lock.

ID `$3c` points to `$4862` in both editions. The 51-byte window
`$4862..$4894` contains a conditional BIOS branch, a poll of `$4895`, a
20-iteration call loop at `$4885`, and a shared exit through `$40f1` to the
`+1` cursor path. The source-lock test binds the dispatch word and complete
window against authentic US and JP data. `$4895..$489e` is adjacent data read
by the handler, not included in the code window; ID `$3d` begins at `$489f`.
The meaning of `$2228`/`$4895`, external BIOS call effects, branch execution,
and retail stream selection are not established by this static lock.

ID `$3d` points to `$489f`; its 13-byte source window `$489f..$48ab` is
`JSR $e063; LDA $2228; BEQ $4892; STA $2780; BRA $4892`. The test binds this
window up to, but not including, the adjacent ID `$3e` root at `$48ac`. Its
branch returns into `$4892`, within ID `$3c`'s bounded window, so this is an
overlapping control-flow slice rather than an independent complete handler.
No BIOS behavior or runtime execution is inferred.

ID `$3e` points to `$48ac` in both editions. The complete `$48ac..$4900`
window (85 bytes) contains the bounded handler, its `$48de..$4900` polling
helper, and an exit through `$40fd` to the fixed `+4` cursor path. It is
byte-identical in authentic US and JP Track 02. The next dispatch root, ID
`$3f`, begins at `$4901`. This establishes only static branches, stores, and
loop structure; it does not prove condition outcomes, field meanings, runtime
selection, or gameplay behavior.

ID `$3f` points to `$4901` in both editions. Its 15-byte window
`$4901..$490f` clears X, reads eight successive stream bytes into `$3b70,X`,
then jumps to `$4109`. The source-lock assertion stops at `$490f`; the distinct
ID `$26` table target begins at `$4910`. This records static indexed transfer
and control flow only, not the stored bytes' meaning or runtime selection.

## Verification boundary

`theron_v1_huc6280_disassembly_read_file()` now verifies the complete table
against both raw BIN identities, checks all 85 decoded little-endian targets
against the listing, and confirms each target lies in the `$4000..$7fff`
stage-two address span. It does not independently establish a runtime memory
mapping for that span. The receipt is static-source evidence only. The table
alone does not prove which indices occur in valid stream data, the meaning of
any handler, operand/stream advancement for every entry, or which real
level/resource uses the interpreter. It does not authorize a host bytecode
interpreter or gameplay semantics.
