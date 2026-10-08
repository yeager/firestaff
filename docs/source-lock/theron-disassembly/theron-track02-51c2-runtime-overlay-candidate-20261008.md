# Theron's Quest Track 02 `$51C2` Runtime-Overlay Candidate

## Evidence and scope

An instrumented Mednafen PCE Fast capture from an existing authentic JP
Ak-Tu-Ba state recorded execution at logical `$51CE`, physical `$0D11CE`, with
MPR2 `$68`. The mapped 12-byte window begins with `RTS` and matches a sequence
found in both hash-verified raw Track 02 images. It does not match the JP
Stage-2 listing at logical `$51CE`, which begins a different instruction
stream. Keep the MPR mapping with this observation; logical PC alone does not
identify a source window.

| Edition | Raw Track 02 MD5 | Candidate block raw offset | Matching `$51CE` window offset |
|---|---|---:|---:|
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | `0x95F42` | `0x95FCE` |
| US | `f23601102138f87c33025877767ebf76` | `0x96877` | `0x96903` |

The raw-byte matches establish source presence only. Aligning each candidate
block at CPU `$5142` makes the JP/US bytes containing the runtime sequence
disassemble at `$51CE`; this is a candidate relocation, not proof that the
game copied either block to the observed MPR2 bank. No loader receipt currently
joins raw-sector source, destination address, and CPU execution.

## Candidate JP listing

MAME `unidasm -arch h6280` over the authentic JP bytes at raw offset
`0x95F42`, with the explicitly hypothetical `-basepc 0x5142`, yields this
bounded path:

```asm
$5142: jsr  $510C
       tax
       lda  #$0C
       jsr  $44E7
       phx
       lda  $69E3,x
       sta  $16
       stz  $17
       lda  $36
       sta  $14
       lda  $37
       and  #$03
       sta  $15
       jsr  $59B9
       plx
       lda  $69F3,x
       clc
       adc  $1A
       sta  $3A
       lda  $6A03,x
       adc  $1B
       sta  $3B
       lda  $6A13,x
       sta  $3C
       jsr  $44E7
       rts
$51C5: pha
       phx
       jsr  $5142
       plx
       ply
       lda  ($3A),y
       rts
$51CF: lda  #$04
       bsr  $51C5
       and  #$20
       asl  a
       asl  a
       asl  a
       rol  a
       rts
$51DA: lda  #$04
       bsr  $51C5
       and  #$40
       asl  a
       asl  a
       rol  a
       rts
```

The JP and US candidate blocks are not identical: even the `$5142` entry
starts with different helper targets and table offsets. The table contents,
called-helper effects, block loader, and meanings of the returned bits remain
unresolved. The observed `$51CE` execution proves only that the mapped byte at
that instant is `RTS`; it does not prove that `$51CF` or `$51DA` executed.

## Next evidence needed

Trace the authentic CD/RAM transfer or other retail copy path from these raw
source offsets into the MPR2 `$68` destination, then record its bounded bytes
and caller/return context. Until that join exists, do not merge this candidate
with the static JP Stage-2 `$51CE` routine or assign game-level semantics.
