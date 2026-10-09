# Theron JP runtime input-poll bank alias at `$44D2`

## Scope

This note separates an authentic JP runtime controller-poll address from the
static Stage-2 bytes at the same HuC6280 logical address. It records a static
byte-for-byte match between runtime instructions and repeated Track 02 source
sequences, but does not identify which duplicate sector was loaded by the
emulator, the runtime source LBA, or a gameplay effect.

## Static Track 02 bytes

The inputs are the hash-verified original Track 02 BINs:

| Edition | Track 02 MD5 | Stage-2 record raw start | Raw offset for `$44D2` |
| --- | --- | ---: | ---: |
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | 2,895,632 | 2,896,866 |
| US | `f23601102138f87c33025877767ebf76` | 2,897,984 | 2,899,218 |

The Stage-2 record begins at logical `$4000`; the listed `$44D2` offsets add
`$04D2` to each record start. The 64-byte raw windows are identical, with
SHA-256 `82103f22af4e6d1530a6a0498c24ae1b502e47e7e26e6441d9e422f1f22d1037`.
Their first 16 bytes are:

```text
46 a5 57 85 16 a5 58 85 17 ad b9 47 4a aa da ad
```

Three MAME 0.285 listings per edition matched, using
`unidasm TQJP02.bin -arch h6280 -basepc 0x44d2 -skip 2896866 -count 64` and
the corresponding US command with `TQUS02.bin -skip 2899218` (listing SHA-256
`e2182a5e0540b8e7271093ca6b8f5a1fac31728fea4a2a77a4dec18b66febdc1`). The
first raw byte is `$46`, not the direct absolute-load encoding for `$1000`
(`ad 00 10`). This is a raw static-window comparison; it does not establish
that `$44D2` is an instruction boundary in the Stage-2 image.

## JP duplicate-candidate continuation

A source-bound loop searched the authentic JP Rev. 1 Track 02 BIN for the
21-byte runtime-poll sequence observed at `$44D2` and found exactly seven
offsets: `0x95072`, `0xde872`, `0x128072`, `0x171872`, `0x1bb072`,
`0x204872`, and `0x24e075`. It checked the 64-byte window at every offset and
ran MAME `unidasm -arch h6280 -basepc 0x44d2` over each one. All seven windows
have SHA-256
`5eb40a9bf8ec8761aa55177acafbbf375bfc730e9519cd14f181ab28e8e3542b` and the
same decoded continuation through `$4510`.

After the poll's `RTS` at `$44E6`, the common bytes decode as four short
`PHP; PHA; CLC; ADC $00; TAM #mask; PLA; PLP; RTS` sequences, using masks
`$08`, `$10`, `$20`, and `$40`. This static comparison adds no
candidate-specific continuation or regional discriminator. The helper
sequences' caller, runtime source LBA, mapped-bank ownership, and gameplay
meaning remain unbound; identical candidates cannot select which copy was
loaded.

## Runtime capture

Two isolated PCE Fast captures used authentic JP Rev. 1 media and distinct
emulator-created states (`14dec90b96ec3e14622ec0fab535a92f` and
`d5c0daa227c04d55bdf20244803c00bd`). Each replay applied and verified four
180-frame cardinal-direction inputs at the non-System-Card polling boundary.
The instrumented Mednafen binary was MD5
`8a43dfb6ae6155d5562d1aa04a5bd761`. Both input traces contain 1,791 reads at
logical PC `$44D2`, all with MPR-derived physical PC `$0D04D2`; the shared
input-trace SHA-256 is
`e151c48723b0f5b95bb0991d8626189716fdde6c60cf515231607997a099dac5`.
The scripted-input verifier reports four event frames applied, followed by
controller reads exposing each scripted mask, and four non-System-Card poll
witnesses at `$44D2`.

`$0D04D2` is consistent with MPR2 `$68` and the `$04D2` offset within its
8-KiB page. Thus, in these captures, the CPU reports the same logical PC while
fetching from bank `$68`, not from the static Stage-2 BIN window above. The
existing PCE Fast input trace records the instruction PC and physical PC at
each I/O read; it does not record the bytes fetched there. No same-session
source-LBA-to-bank-`$68` receipt was produced.

A capture-only Mednafen patch adds a bounded `theron_runtime_code_window`
record at the prefetch hook for logical PC `$44D2`, including three mapped
bytes, each byte's MPR and physical address. A relinked PCE Fast binary
(`c4082f08f5b306e6004863a020b5e22a`) was run for eight seconds with the
authentic JP state (`d5c0daa227c04d55bdf20244803c00bd`), JP Rev. 1 Track 02,
and the verified System Card. Its 64 code-window rows are identical:

```text
theron_runtime_code_window pc=44d2 physical_pc=0d04d2 byte0=ad byte0_mpr=68 byte0_physical=0d04d2 byte1=00 byte1_mpr=68 byte1_physical=0d04d3 byte2=10 byte2_mpr=68 byte2_physical=0d04d4
```

The matching prefetch trace reports `opcode=ad`, `mapped_opcode=ad`, and
operands `00 10`; the input trace at the same PC reads logical register
`$1000`. This establishes that the instruction fetched at the runtime alias
is `LDA $1000` from MPR bank `$68`; hardware MPR0 is separately `$FF` in the
instruction trace. It does not bind the bank window to a Track 02 source LBA.

The replay also applied `up@60:60`; the verifier saw the event at the
controller-poll boundary, with 120 reads carrying raw mask `0010` and 60 at
`$44D2`. The runner ended `BLOCKED`/exit 124 with zero CD IRQs, zero
authenticated CD-to-RAM receipts, and `transition=missing`. No gameplay
effect is established. Private capture:
`/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-runtime-code-window-20261009-1249/`.
The code-window, input and transition trace SHA-256 values are respectively
`b920cabbaf05b8be65f5d7776341f8630eda26d7113a0ba0aba099c0b8811dfb`,
`c385191cc47aa3e165fa57fdffe2647d512909f9b36f8ab87967a9c2e4899240`, and
`bd923242b8d090c6be50edf298235091921e7316861b58ec7032283c638c4e86`.

## Bounded input-poll control-flow replay

The instrumented trace also records the next 128 prefetches after the first
`$44D2` hit. Two eight-second runs used the same binary, JP state, Track 02,
and System Card; only the scripted UP start frame differed. In the control,
UP began at frame 60, so the first poll ran with `raw=0000`. In the comparison,
`up@1:60` was active from the first frame. The raw controller trace shows the
high-nibble poll at `$44C1` changing from `value=3f` to `value=3e`; the low
nibble at `$44D2` remains `value=3f` in both runs.

The mapped instruction sequence is:

```text
$44D2  LDA $1000
$44D5  AND #$0f
$44D7  ORA $28b8
$44DA  EOR #$ff
$44DC  STA $28b8
$44DF  CMP #$0f
$44E1  BNE +3  ; both runs take this branch to $44E6
$44E6  RTS
```

The prefetch register trace at `$44DA` has `A=ff` in the control and `A=ef`
with UP active. At `$44DC`, the value to be stored through the `$28B8` operand
is respectively `00` and `10`. The mapped consumer trace then shows a
different path at `$D32F`:

```text
$D32F  LDA $28B8
$D332  AND #$f0
$D334  CMP $2912
$D337  BNE +$0d
$D346  STZ $2920
$D349  STA $2912
```

With UP active, `$D334` compares `A=10` against the prior `$2912=00` and takes
the branch to `$D346`; the instruction at `$D349` stores `10` to `$2912`. A
later consumer read sees `$2912=10`. In the control, the comparison is `00`
against `00`, so the branch is not taken and `$2912` remains `00`. This
authentic trace proves the scripted UP bit reaches a game-RAM state update;
it does not identify that byte's full meaning or establish movement or a
visible action. Both runners ended `BLOCKED` with zero CD IRQs and
authenticated CD-to-RAM receipts and `transition=missing`. Their final raw
reports were identical: level 2, direction 1, party position `(2,3)`.

Private control capture:
`/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-runtime-code-follow-20261009-1252/`.
Private UP-at-frame-1 capture:
`/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-runtime-code-follow-input-20261009-1255/`.
The respective code-trace SHA-256 values are
`fba73593e2d714aefacd1da0868b78f1409d7adbbb77e943b9c6c7618583179f` and
`6bd8222e1b1f378397772d71a5490c0e92893bad43ace7e285bf7f765d5f88ad`;
the input traces are
`c385191cc47aa3e165fa57fdffe2647d512909f9b36f8ab87967a9c2e4899240` and
`759e32789568ce128bb980aafbdeeddf49ae74a6d5c4c74babe7916a7cd400a1`;
the main-RAM consumer read traces have SHA-256
`39c7c46b91d7aabfde2f755d250b8deb034fed327a201250a2af0377b9fdea65` and
`f8b4ad931f6be2ca4e28da3dceb6c2489ab22ee742e99b33ef8cd93dc2349dd2`.

The paired code paths and later reads establish that UP changes the observed
execution path and `$2912` value. They do not establish the byte's gameplay
meaning, movement, a visible action, or a game transition. Both captures ended
with the raw report still at level 2, bank 1, direction 1, party position
`(2,3)`, `transition=missing`, zero CD IRQs, and zero authenticated CD-to-RAM
receipts.

## Bounded caller continuation from the UP replay

The UP-at-frame-1 trace contains 128 consecutive instruction-prefetch records
after its first `$44D2` hit. This extends the dynamic disassembly through the
poll caller and into the state consumer, using the same authentic JP Rev. 1
media and operator-created dungeon state as the preceding section. The trace
SHA-256 is
`6bd8222e1b1f378397772d71a5490c0e92893bad43ace7e285bf7f765d5f88ad`.

The observed control-flow spine is:

```text
$44E6 RTS
$4349 INC $28b7
$434C JSR $4701
$4701 LDA $2922
$4704 BEQ +4
$470A LDA $290d
$470D CMP #$00
$470F BNE +7
$4718 LDA #$03
$471A JSR $4505
$471D JSR $D26B
$D26B LDA #$00
$D26D STA $2911
$D270 LDA #$03
$D272 JSR $44E7
$D275 LDX $290d
$D278 CPX #$00
$D27A BEQ +4
$D27C BSR $D2F9
...
$D312 LDA $2e02
$D315 ORA $2e04
$D318 ORA $2925
$D31B BNE -$0c             ; not taken in this replay
$D31D LDA $2dfa
$D320 BEQ +3               ; taken in this replay
$D325 JSR $D4EC
$D4EC LDA $290d
$D4EF BEQ +$0d              ; not taken
$D4F1 CMP #$0d
$D4F3 BCC +$20              ; taken to $D515 with A=$0c
$D515 ASL A
$D516 TAX
$D517 LDA $77ce,X
$D51A STA $c5
$D51C LDA $77cf,X
$D51F STA $c6
...
$D580 RTS
$D328 LDA $28b8
$D32B AND #$0f
$D32D BNE +$14              ; not taken
$D32F LDA $28b8
$D332 AND #$f0
$D334 CMP $2912
$D337 BNE +$0d              ; taken to $D346 with A=$10
$D346 STZ $2920
$D349 STA $2912
```

Ellipses mark instructions in the captured path that are not needed to show
these branch and call boundaries; the trace ends at step 127 (`$D35E`), so it
does not represent a complete caller or frame routine. The `$D4EC` path reads
an indexed pointer from `$77CE/$77CF`, follows it through zero-page `$C5/$C6`,
and returns before the `$28B8` comparison. Those bytes and pointer contents
are runtime observations only: this capture provides neither their original
Track 02 source receipt nor enough evidence to name the pointed-to record.

The continued trace shows the input latch reaching a branch and writes to
`$2920/$2912`. It still does not identify the semantics of those fields, show
a party-coordinate write, or establish visible movement. The transition
summary remains `transition=missing`, with zero CD data-port reads and zero
CD-RAM target writes. The source file contains seven identical JP candidates
for the initial poll routine, so the disassembly cannot choose which one
populated the running bank.

## Static authentic-source match for the runtime poll routine

The authenticated JP Rev. 1 Track 02 BIN (`TQJP02.bin`, MD5
`b7afb338ad31be1025b53f9aff12d73a`, 8,102,640 bytes) contains the exact
21-byte sequence beginning at the runtime poll PC `$44D2`:

```text
ad 00 10 29 0f 0d b8 28 49 ff 8d b8 28 c9 0f d0 03 4c 00 e0 60
```

The sequence occurs seven times in the Track 02 raw BIN, at track-relative
raw offsets `0x95072`, `0xde872`, `0x128072`, `0x171872`, `0x1bb072`,
`0x204872`, and `0x24e075` (zero-based raw sector indices 259, 387, 515,
643, 771, 899, and 1027; raw-sector offsets `0x4e2` for the first six and
`0x4e5` for the last). In the authentic UP-at-frame-1 runtime trace, the
prefetch rows from `$44D5` through the returned `$44E6` expose matching opcode
and operand bytes for `AND #$0f`, `ORA $28b8`, `EOR #$ff`, `STA $28b8`,
`CMP #$0f`, `BNE +3`, and `RTS`. Each row matches the corresponding bytes at
the first listed Track 02 candidate when mapped by the PC delta from `$44D2`.
The initial `$44D2` bytes are independently logged as `ad 00 10` in the
runtime code-window record. The code trace SHA-256 is
`6bd8222e1b1f378397772d71a5490c0e92893bad43ace7e285bf7f765d5f88ad`; the
capture is
`/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-runtime-code-follow-input-20261009-1255/`.

The retained source-binding capture `theron-cd-ram-source-bind-20261007`,
run 13, records Track 02 start LBA 3590. Under that media identity, the seven
static candidates correspond to LBAs 3849, 3977, 4105, 4233, 4361, 4489, and
4617. However, run 13's 4,096 CD data-port reads cover only LBAs 3590 and
3591; its 65 target writes are at physical `$0D089F..$0D08AB`. It does not
read any candidate LBA, and its trace does not contain the `$44D2` runtime
code-window receipt. This older provenance session therefore cannot be joined
to the later input replay to identify which candidate populated the poll
routine.

The same UP-at-frame-1 runtime trace also source-matches the observed
`$28B8` consumer path. Its six prefetch rows at `$D32F`, `$D332`, `$D334`,
`$D337`, `$D346`, and `$D349` match the corresponding bytes at seven JP
Track 02 raw offsets: `0x9cf6f`, `0xe676f`, `0x12ff6f`, `0x17976f`,
`0x1c2f6f`, `0x20c76f`, and `0x255f6f`. The candidates are anchored by
`ad b8 28 29 f0 cd 12 29 d0 0d`; at offset `+0x17`, each has
`9c 20 29 8d 12 29`, matching the observed branch target stores. Their
track-relative sector indices are 273, 401, 529, 657, 785, 913, and 1041,
which map using the same LBA origin to candidate LBAs 3863, 3991, 4119, 4247,
4375, 4503, and 4631. These seven byte-identical copies remain ambiguous.
The UP replay itself had zero CD reads and writes; the prior source-binding
session read only LBAs 3590-3591. This extends the static/runtime code match
through the observed RAM consumer, but still does not show that any candidate
sector was loaded in either runtime session.

### Static disassembly of the poll-consumer continuation

An authentic-media disassembly of the 256-byte windows beginning at `$D32F`
extends the candidate check beyond the six prefetched rows. All seven JP
copies are byte-identical, as are all seven US copies. The regional windows
have SHA-256 `2d187b0eb974b353c2c384f858d337e04068b952c188d85ba326f98cafa89353`
(JP) and `8baba7511f4bdb3a12073c925777d84030b82dd65ffbc12df855941079229091`
(US). The first 107 bytes are also separately matched as exact byte strings
in the source regression; the 256-byte digest covers the following
helper-selection code.

Selected disassembly from the authentic JP and US images:

```text
$D32F LDA $28B8         $D337 BNE $D346
$D339 INC $2920         $D343 JMP $D3BC (JP) / $D3CA (US)
$D346 STZ $2920         $D34C TII $290D,$290F,$0002
$D36F JMP $D41A (JP) / $D428 (US)
$D394 JSR $D4EC (JP) / $D4FA (US)
$D3CF LDA $28B8         LSR x4 ; TAX
$D3D7 LDY $D616,X (JP) / $D624,X (US)
$D3DC LDX $D471,Y (JP) / $D47F,Y (US)
$D3DF BSR $D3F9         $D3E1 BNE $D419
$D3EC JSR $D4EC (JP) / $D4FA (US)
$D3F9 LDY $D46D,X (JP) / $D47B,X (US)
$D3FC LDA $20C7,Y      $D3FF CMP #$FF
$D408 STA $290D         $D40B JSR $D479 (JP) / $D487 (US)
$D414 LDA #$01 / RTS; $D417 LDA #$00 / RTS
```

The static decode shows masks and indexed lookups driven by `$28B8`, reads from
tables at region-specific addresses, tests `$20C7,Y`, and returns zero or one
through the `$D3F9` helper path. The code also copies bytes with `TII` and
touches `$290D/$290E`. These are instruction-level observations, not verified
field names, accepted-command semantics, or proof of a party-coordinate
change. The authentic UP replay follows only through `$D35E`; it does not
reach this later code. The seven source copies per edition remain ambiguous,
and no runtime source-LBA receipt is added. The authentic-media regression
locks the full 256-byte window for both regions and independently mutates
each candidate to verify rejection.

The 77-byte helper slice at `$D3D7` is also locked as an exact signature. It
occurs seven times in JP at raw offsets `0x9d017`, `0xe6817`, `0x130017`,
`0x179817`, `0x1c3017`, `0x20c817`, and `0x256017`; the US copies are at
`0x9d955`, `0xe7155`, `0x130955`, `0x17a155`, `0x1c3955`, `0x20d155`, and
`0x256955`. The corresponding SHA-256 values are
`bcb0572f9c87ff15ddf0367863505de51fb2a7f6d463ac43afaf593a81e6436a` (JP) and
`a589cfe87ebcd89c0dded16d6dba2ad4e1095c83e66f48e18a1d45352aae3be9` (US).
This confirms the regional indexed-load/call targets in the media bytes, not
the runtime index or the meaning of any selected table entry. The
authentic-media decode at `$D3E1` is `BNE $D419`; no CLC/BCC path is claimed.
The helper signature regression independently mutates each of its seven
copies per region.

### Static source candidates for the caller's indexed-table path

The same 128-step UP trace exposes `$D4EC` through `$D4F3` and the indexed
reader at `$D515`. The runtime bytes at `$D4EC` are
`ad 0d 29 f0 0d c9 0d 90 20`; the taken `$D4F3` branch enters `$D515` with
`A=$0c`. The JP bytes at `$D515` begin
`0a aa bd ce 77 85 c5 bd cf 77 85 c6 a0 01 b1 c5 85 c7 c8 b1 c5 c9 fe d0 06`,
including indexed reads based on `$77CE/$77CF` and pointer setup in `$C5/$C6`.

Both JP sequences occur at seven identical copies. The `$D4EC` signature is
at raw offsets `0x9d12c`, `0xe692c`, `0x13012c`, `0x17992c`, `0x1c312c`,
`0x20c92c`, and `0x25612c` (sector indices 273, 401, 529, 657, 785, 913,
and 1041; within-sector offset `0x4fc`). The longer `$D515` prefix occurs
29 bytes later in each copy, at `0x9d155`, `0xe6955`, `0x130155`,
`0x179955`, `0x1c3155`, `0x20c955`, and `0x256155` (within-sector offset
`0x525`). These are the same seven raw sectors that contain the `$D32F`
consumer candidates listed above.

The US `$D4EC` branch signature also occurs seven times, at
`0x9da6a`, `0xe726a`, `0x130a6a`, `0x17a26a`, `0x1c3a6a`, `0x20d26a`, and
`0x256a6a` (sector indices 274, 402, 530, 658, 786, 914, and 1042; within-
sector offset `0x50a`). The longer 25-byte `$D515` JP prefix has no exact US
match because the US code uses table-base operands `$77DC/$77DD` instead of
JP `$77CE/$77CF`. The corresponding US 25-byte prefix is
`0a aa bd dc 77 85 c5 bd dd 77 85 c6 a0 01 b1 c5 85 c7 c8 b1 c5 c9 fe d0 06`.
It occurs at `0x9da93`, `0xe7293`, `0x130a93`, `0x17a293`, `0x1c3a93`,
`0x20d293`, and `0x256a93` (the same sector indices, within-sector offset
`0x533`). Across this 25-byte prefix, the editions differ only in the two
low-byte operands: JP `CE/CF`, US `DC/DD`. This establishes a static regional
address difference, not the reason for it or a runtime US path.

`tests/test_theron_v1_runtime_input_poll_media_candidates.py` checks both
regional hashes, exact occurrence counts/offsets for the poll, consumer,
caller-branch, and regional indexed-table signatures, and rejects each
candidate under an independent in-memory byte mutation. The test passed against the authentic
`TQJP02.bin` and `TQUS02.bin` in `/home/trv2/.firestaff/data/theron/`. Those
mutated copies exist only inside the negative test; they are not substitute
game data. Static duplicates remain unresolved, and no same-session CD-load
receipt is added.

## US Track 02 static regional candidates

The same poll and consumer signatures were also searched in the authentic US
Track 02 BIN (`TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`,
8,104,992 bytes). Each signature has seven occurrences. The poll routine
offsets are `0x959a8`, `0xdf1a8`, `0x1289a8`, `0x1721a8`, `0x1bb9a8`,
`0x2051a8`, and `0x24e9a8` (raw sector indices 260, 388, 516, 644, 772,
900, and 1028; within-sector offset `0x4e8`). The `$28B8` consumer anchors
are at `0x9d8ad`, `0xe70ad`, `0x1308ad`, `0x17a0ad`, `0x1c38ad`,
`0x20d0ad`, and `0x2568ad` (raw sector indices 274, 402, 530, 658, 786,
914, and 1042; within-sector offset `0x34d`). These are static source
candidates only; no US runtime capture has been matched to these bytes or
sectors.

This is a strong static/runtime instruction-byte identity, not a same-session
load receipt. That capture's transition summary reports zero CD data-port
reads, zero source-bound CD reads, and zero CD-to-RAM target writes. The seven
identical candidates therefore remain indistinguishable as the specific
runtime source. Do not claim a unique source LBA, bank-population event, or
that the save-state replay loaded any of these sectors during the capture.

Private captures remain on TRV2:

- `/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-input-sweep-20261009-1250/`
- `/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-userstate-input-sweep-20261009-1238/`

## Next evidence needed

Use a cold-start or otherwise source-reading authentic JP session to join the
runtime bank-`$68` window at `$44D2` to a same-session CD read and target-write
receipt. The static byte match narrows the candidate set to seven identical
Track 02 sequences but does not identify which was loaded. Then follow the
poll's caller to an observable party-position or screen change before
assigning movement semantics or treating these controller polls as completed
gameplay actions.
