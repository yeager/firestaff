# Theron Track 02 `$287f-$2896` buffer setup candidate

## Authentic media and exact matches

This is a static disassembly result from the authenticated raw Track 02 BINs;
it is not a runtime source-LBA receipt. The JP Rev. 1 BIN has MD5
`b7afb338ad31be1025b53f9aff12d73a` and SHA-256
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`. The US
BIN has MD5 `f23601102138f87c33025877767ebf76` and SHA-256
`f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565`.

The exact 26-byte fragment

```text
a9 97 8d 1a 27 62 8d 80 28 8d 81 28 8d 82 28 3a
8d 7f 28 73 7f 28 83 28 14 00
```

has SHA-256 `f8ebdfb2908aa067d0d7f09d57281533616f74c1fcba4b522e92d78030c76981`.
A raw-byte sweep found exactly seven matches in each edition:

| Edition | Matching Track 02 raw offsets |
| --- | --- |
| JP Rev. 1 | `0x0a7344`, `0x0f0b44`, `0x13a344`, `0x183b44`, `0x1cd344`, `0x216b44`, `0x260344` |
| US | `0x0a7c74`, `0x0f1474`, `0x13ac74`, `0x184474`, `0x1cdc74`, `0x217474`, `0x260c74` |

The JP first match was disassembled with MAME `unidasm -arch h6280` from
logical `$a704` (raw offset `0x0a7344`):

```text
$a704  LDA #$97
$a706  STA $271A
$a709  CLA
$a70A  STA $2880
$a70D  STA $2881
$a710  STA $2882
$a713  DEA
$a714  STA $287F
$a717  TII $287F,$2883,$0014
```

The sequence sets `$287f` to `$ff`, clears `$2880-$2882`, and executes a
20-byte forward HuC6280 `TII` from `$287f` to `$2883-$2896`. Because the source
and destination overlap by 16 bytes, the initial four-byte pattern propagates
through the copied range. This is consistent with the repeated
`ff 00 00 00` pattern seen in a separate restored-state transfer trace.

## Limits and next evidence

These are seven repeated static candidates per edition. They do not prove
which copy executes, its runtime bank mapping or source LBA, or whether this
buffer is the six-rectangle classifier's live data. The following `$43ab` JP
and `$43ae` US calls also differ by edition; no cross-region logical address
or runtime caller binding is claimed here. The fragment initializes only the
observed prefix and copies that prefix through the buffer; it does not reveal
the descriptor table, rectangle meanings, dispatch result, or a gameplay
effect.

`tests/test_theron_v1_huc6280_disassembly.c` verifies the exact fragment occurs
at exactly these seven offsets in each authenticated raw BIN. Keep direct
world clicks disabled until one same-session capture binds a specific
candidate to PC/MPR, authenticated CD source bytes, the rectangle contents,
the selected command, and a resulting state transition.
