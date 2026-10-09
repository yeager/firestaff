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

Both captures ended with the raw report still at level 2, bank 1, direction 1,
party position `(2,3)`, `transition=missing`, zero CD IRQs, and zero
authenticated CD-to-RAM receipts. Therefore they prove controller input
delivery and a logical/physical bank alias only. They do not prove gameplay
input handling, movement rejection, or a game transition.

Private captures remain on TRV2:

- `/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-input-sweep-20261009-1250/`
- `/home/trv2/firestaff-theron-evidence/capture/l4c46-jp-userstate-input-sweep-20261009-1238/`

## Next evidence needed

Capture the instruction bytes actually fetched from bank `$68` at logical
`$44D2`, then bind that bank window to authentic Track 02 source sectors using
same-session CD read and target-write receipts. Follow the mapped code's
consumer to an observable party-state or screen change before assigning
movement semantics or treating these controller polls as gameplay actions.
