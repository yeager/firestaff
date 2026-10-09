# Theron JP runtime RAM-writer code and Track 02 candidates

## Capture identity and boundary

The emulator-created JP Ak-Tu-Ba savestate (`14dec90b96ec3e14622ec0fab535a92f`)
was replayed twice with authentic JP Rev. 1 Track 02 (`TQJP02.bin`, MD5
`b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`). The instrumented PCE Fast Mednafen
binary MD5 is `2984bfe4b13979c697921a50b07c7a7a`. The private captures are
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-up-hold300-opcodes-20261010/`
(`up@1:300`) and
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-no-input-control-opcodes-20261010/`
(no replay input).

The held-input trace records 600 active-low UP reads (`raw=0010`); the control
records only neutral reads. Both transition summaries end at level `02`,
direction `01`, coordinates `(02,03)`, and `transition=missing`, with zero CD
IRQs and zero authenticated CD-to-RAM receipts. The party-RAM write traces are
byte-identical (SHA-256
`e24f76f0a3d137355b3ae399b6a2d5797ef42895febf342c4c8c7c8518bb8ed2`), each
containing 2,880 rows. Thus the repeated `$31`/`$3F` writes in this replay are
not attributable to the held UP input. They do not prove a movement, map
transition, or field semantics.

## Runtime instruction windows

The instrumentation logs the instruction bytes at each write PC. MPR2 `$68`
maps these logical PCs to physical `$0Dxxxx` addresses. Repeated rows at each
PC have the same 12-byte window in both captures:

| Logical PC | Physical PC | Runtime bytes | Selected decode |
| --- | --- | --- | --- |
| `$506A` | `$0D106A` | `85 31 86 32 84 33 A5 2F 44 18 A5 31` | `STA $31; STX $32; STY $33; LDA $2F; BSR +$18; LDA $31` |
| `$5079` | `$0D1079` | `85 31 A5 30 44 0D A6 32 A4 33 60 00` | `STA $31; LDA $30; BSR +$0D; LDX $32; LDY $33; RTS` |
| `$57D7` | `$0D17D7` | `E6 3F A5 B8 F0 06 FA DA 44 22 85 4E` | `INC $3F; LDA $B8; BEQ +6; PLX; PHX; BSR +$22; STA $4E` |
| `$57E3` | `$0D17E3` | `E6 3F A5 B7 F0 06 FA DA 44 16 85 4F` | `INC $3F; LDA $B7; BEQ +6; PLX; PHX; BSR +$16; STA $4F` |
| `$57EF` | `$0D17EF` | `E6 3F A5 B6 F0 06 FA DA 44 0A 85 50` | `INC $3F; LDA $B6; BEQ +6; PLX; PHX; BSR +$0A; STA $50` |
| `$5800` | `$0D1800` | `85 3F 60 A5 3F 29 03 1A 20 0D 5B 86` | `STA $3F; RTS; LDA $3F; AND #$03; INC A; JSR $5B0D` |

The three relative BSRs at `$57DF`, `$57EB`, and `$57F7` target `$5803`.
The trace name `party_ram_write` describes the instrumentation filter, not the
meaning or ownership of these RAM fields. In particular, do not treat writes
to `$2031` as level changes or writes to `$203F` as player-facing direction
changes without a consumer-level proof.

## Static authentic-media matches

Each runtime code window at `$506A`, `$5079`, `$57D7`, `$57E3`, and `$57EF`
occurs seven times in each hash-verified raw Track 02 BIN. A three-loop scan
reproduced these candidate clusters:

| Edition | `$506A` offsets (zero-based raw Track 02 bytes) | Candidate LBAs |
| --- | --- | --- |
| JP Rev. 1 | `0x95e6a`, `0xdf66a`, `0x128e6a`, `0x17266a`, `0x1bbe6a`, `0x20566a`, `0x24ee6d` | 3627, 3755, 3883, 4011, 4139, 4267, 4395 |
| US | `0x9679f`, `0xdff9f`, `0x12979f`, `0x172f9f`, `0x1bc79f`, `0x205f9f`, `0x24f79f` | 3271, 3399, 3527, 3655, 3783, 3911, 4039 |

The earlier LBA labels omitted each raw member's pre-INDEX 01 sectors and are
superseded by the corrected values above. The JP Rev. 1 CUE places Track 02
INDEX 01 at byte offset 526,848 (224 sectors); the authentic source trace binds
that offset to LBA 3590. The US CloneCD descriptor's Track 02 data entry is
PLBA 3234. Byte-exact alignment of all seven candidates between authenticated
`TQUS02.bin` (MD5 `f23601102138f87c33025877767ebf76`) and its original IMG
member derives a raw-member INDEX 01 offset of 225 sectors. The corrected
values use these edition-specific anchors.

The other four windows in each edition occur at their corresponding logical-PC
displacements in those same seven candidate clusters. The `$5800` window has
six JP occurrences at `0x96730`, `0xdff30`, `0x129730`, `0x172f30`,
`0x1bc730`, and `0x205f30` (LBAs 3628, 3756, 3884, 4012, 4140, and 4268),
and no exact US occurrence. This is static source matching only. The save-state
replays read no non-System-Card CD sectors, so none of these candidate copies
can be identified as the bytes loaded into the runtime bank.

## Next proof

Use a cold-start source-reading session to bind the dynamic `$506A..$5803`
windows to CD sectors and RAM destinations in one receipt. Then follow their
callers and consumers with input/control replays before assigning gameplay
semantics or changing production movement logic.
