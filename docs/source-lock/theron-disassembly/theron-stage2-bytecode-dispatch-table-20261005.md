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

Index `$10` at `$4291` reads the offset-1 byte, uses it as an index into a
little-endian pointer table based at `$6800`, saves the original `$1c/$1d`
cursor, and calls `$40cc` using the selected pointer. On return it restores
the original cursor and takes the `$40f5` (`+2`) path. Index `$09` at `$4253`
is a one-instruction `RTS`, compatible with ending such a nested call, but no
authentic stream binds the selector, table entry, and return instruction as a
pair. The nested execution path is therefore a static call-graph observation,
not proof of a valid retail stream structure.

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
step summary.

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
The callees' runtime effects and the candidate stream's execution remain
unproven.

### Rooted `$2c` handler: external helper handoff

Dispatch table entry `$2c` targets `$4674` in both editions. This root calls
`$4483`, sets Y to `$02`, reads stream offsets `+2`, `+3`, and `+4` into zero
page `$02`, `$03`, and `$0e`, loads A with `$0f`, calls `$3ab7`, then jumps to
`$4101`, the shared five-byte cursor advance. The earlier `$4483` call and the
`$3ab7` implementation are not decoded by this bounded root; `$3ab7` is below
the stage-two `$4000..$7fff` image. The handoff establishes the static input
registers and cursor step only, not the callee's effect or a gameplay meaning.

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
the actual roots are `$73b2` US and `$73b4` JP.

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
the `$7470/$7472` roots begin two bytes apart. If the nested invocation
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
restores the pair, and calls `$3848`. The immediate at `$43bd` differs (`#$9f`
US / `#$cf` JP); each resulting pointer is one byte past that edition's
64-byte TIA source span above. This alignment is established statically; the
role of the byte loaded to `$37cc` and the pointer passed to `$37d8` remain
unknown. These snippets establish instruction flow and regional operands, not
what any dispatch index means.

```text
US $46c4: JSR $5e4d   JP $46c4: JSR $5e7d
```

The surrounding control flow is byte-identical: `INY; LDA ($1c),Y; BNE $46c4;
LDA #$13; JSR $3ab7; BRA $46c7; JSR [regional target]; JMP $40f5`. The two
regional callees have the same instruction sequence, shifted by `$30`: set
`$0402` to `$e0`, set `$0403` to zero, transfer 64 bytes with `TIA` to `$0404`,
then return. Their source spans (`$5e5f..$5e9e` US and `$5e8f..$5ece` JP) are
byte-identical with FNV-1a `591d332b`. These observations establish static
control flow and data identity only; they do not assign a gameplay command
meaning or prove the index occurs in a valid stream.

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
