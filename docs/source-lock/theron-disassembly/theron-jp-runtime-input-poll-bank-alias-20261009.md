# Theron JP runtime input-poll bank alias at `$44D2`

## Scope

This note separates an authentic JP runtime controller-poll address from the
static Stage-2 bytes at the same HuC6280 logical address. It does not identify
the runtime instruction bytes' source, input consumer, or gameplay effect.

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
`759e32789568ce128bb980aafbdeeddf49ae74a6d5c4c74babe7916a7cd400a1`.

Both captures ended with the raw report still at level 2, bank 1, direction 1,
party position `(2,3)`, `transition=missing`, zero CD IRQs, and zero
authenticated CD-to-RAM receipts. Therefore they prove controller input
delivery and a logical/physical bank alias only. They do not prove gameplay
input handling, movement rejection, or a game transition.

Private captures remain on TRV2:

- `/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-input-sweep-20261009-1250/`
- `/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-userstate-input-sweep-20261009-1238/`

## Next evidence needed

Trace the `$28B8` writer's consumer and bind the runtime bank-`$68` window at
`$44D2` to authentic Track 02 source sectors using same-session CD read and
target-write receipts. Follow the poll's caller to an observable
party-position or screen change before assigning movement semantics or
treating these controller polls as completed gameplay actions.
