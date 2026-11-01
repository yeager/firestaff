# US Drator row reader source candidates

## Scope and input identity

This note maps the two reader-PC windows retained by the 2026-10-31
authenticated US Drator trace to static byte candidates in the authentic US
Track 02 BIN. Input identity was checked before searching:

| Image | Size | MD5 | SHA-256 |
|---|---:|---|---|
| `TQUS02.bin` | 8,104,992 bytes | `f23601102138f87c33025877767ebf76` | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` |

The source files remain in the operator's game-data directory. Only short
instruction windows and candidate offsets are recorded here.

## `$CBC9` indexed-load loop

The captured consumer rows name logical reader PC `$CBC9` and physical reader
PC `$0DABC9`. The trace hook samples `GetRegister(GSREG_PC)` during the memory
read. A static Track 02 candidate explains this PC value as the address after
the three-byte `LDA absolute,Y` instruction that starts at `$CBC6`:

```asm
        .org $cbc6
LCBC6:  lda     $2935,y
        sta     ($bc),y
        dey
        bpl     LCBC6
        rts
```

When `Y=6`, the load reads `$293B`; with MPR1=`$F8`, that logical address maps
to physical `$1F093B`, matching the captured row-byte address. The following
store writes the loaded byte through the pointer in `$BC/$BD`. The static
sequence therefore fits the observed read and copy-loop pattern, but the
captured filtered rows do not retain the instruction opcode or an authenticated
CD-source receipt for the code bytes.

The exact eight-byte sequence `b9 35 29 91 bc 88 10 f8` occurs 14 times in
the authenticated US BIN, at raw offsets:

```text
0x0a085a  0x0a1989  0x0ea05a  0x0eb189  0x13385a  0x134989  0x17d05a
0x17e189  0x1c685a  0x1c7989  0x21005a  0x211189  0x25985a  0x25a989
```

There are five distinct surrounding 80-byte contexts. The hit count and
context variation prevent assigning the live bank to one unique raw occurrence
from this search alone.

## `$45C3` ten-byte block copy

The captured trace also names reader PC `$45C3` / physical `$0D05C3` while the
same session reads `$293B-$293D`. The exact seven-byte US Track 02 sequence at
that logical address is:

```asm
        .org $45c3
L45C3:  tii     $2935,$2400,$000a
```

The HuC6280 `TII` copies ten bytes from `$2935-$293E` to `$2400-$2409`, so the
observed source addresses `$293B-$293D` are transfer offsets 6–8. This
establishes a static instruction candidate whose operands cover the captured
bytes. It does not independently prove that this exact source copy was mapped
in the live session or identify the meaning of any record field.

The exact bytes `73 35 29 00 24 0a 00` occur seven times in the authenticated
US BIN, at raw offsets `0x095a93`, `0x0df293`, `0x128a93`, `0x172293`,
`0x1bba93`, `0x205293`, and `0x24ea93`. Their surrounding 80-byte windows are
identical (SHA-256
`d894062eb10eb85a6f3e6d24bb8ecb560f84b2130809198a9902ac35b2191b92`).

## Retail-window verifier follow-up (2026-11-01)

The strict fetched-code verifier was first exercised with complete 512-byte
windows read directly from the hash-verified retail BIN at candidate raw
offsets. It accepted the `$CBC6` window at offsets `0x0A085A`, `0x13385A`,
`0x21005A`, and `0x25985A`, and the `$45C3` window at `0x095A93`,
`0x205293`, and `0x24EA93`. Those constructed rows validate the verifier but
do not identify a live occurrence.

A later authentic cold US capture did emit live code-fetch windows. Its
`$CBC6` windows match the retail image at multiple candidate offsets, while
the `$45C3` windows do not match the retail Track 02 BIN at any exact
512-byte offset. The strict wrapper therefore rejected the source report; the
live `$45C3` code remains source-unbound. The capture also reached the bounded
consumer and System Card trace caps. See the
[ordered `$C852` call-window report](theron-us-c852-runtime-call-capture-20261101.md)
for the separate complete 129-dispatch sidecar and its rejection boundary.

## Evidence boundary

Together, the row-byte trace and static source candidates remain consistent
with the proposed read/copy loop, but the live `$45C3` bytes contradict the
candidate for this execution. The `$CBC6` live source occurrence also remains
ambiguous across duplicate offsets. Keep candidate occurrences distinct and
row bytes semantically opaque. Do not use these matches to infer creature
identity, position, stats, RNG behavior, AI, combat, or permission to create
native creatures.

See the [cold Drator capture boundary](../theron-us-drator-c852-cold-capture-20261031.md),
the [generator/RNG state report](../theron-generator-rng-state-2026-08-21.md),
and [current capture readiness](../../theron/THERON_CAPTURE_READINESS.md).
