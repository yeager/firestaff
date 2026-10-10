# Theron stage-2 indexed parameter blocks at `$40d5` and `$40dc`

This note extends the authentic JP/US `$4002` candidate and `$40e3` alternate
entry by source-locking the 14 bytes immediately between them. It records the
register values loaded by the two indexed blocks, but does not identify the
System Card calls' API or claim that the static candidate supplied runtime
code.

## Authentic source bytes

Both Track 02 images pass their full-image identities in
[`theron-jp-us-cold-boot-stage2-4002-candidate-20261010.md`](theron-disassembly/theron-jp-us-cold-boot-stage2-4002-candidate-20261010.md).
The candidate listing ends at `$40d5`; the alternate entry starts at `$40e3`.
The intervening bytes are:

| Logical address | JP Rev. 1 | US Rev. 1 | Static use established here |
| --- | --- | --- | --- |
| `$40d5-$40d8` | `00 e7 03 11` | `00 e7 03 11` | Read in order by `$4081-$4096` with X initialized to 0 |
| `$40d9-$40db` | `00 26 85` | `00 fc 83` | Edition-specific bytes; no use established |
| `$40dc-$40df` | `00 e3 03 02` | `00 e3 03 02` | Read in order by `$40aa-$40bf` with X initialized to 0 |
| `$40e0-$40e2` | `00 2b 0d` | `00 2b 0d` | No use established by the locked listing |

The table spans are at raw offsets `0x297475` (JP) and `0x297DA5` (US),
immediately after the 211-byte `$4002` candidate. Their SHA-256 values are
`1c2e057f0ab72fd57419891c821f32c41c47f835156daa177dd461c57e122e41` (JP)
and `b74b4fc9fdd5e8cf0d51167ad910c06740721e9ad1de661c273a13ba8e7f21ca`
(US). The real-media regression
`tests/test_theron_v1_jp_us_stage2_4002_static_source_lock.py` checks the
complete image hashes, exact offsets, all 14 bytes, each hash and both
four-byte tuples.

## Register-flow facts from the locked candidate

The listing at `$4080` clears X, then loads four consecutive bytes beginning
at `$40d5` into `$FC`, `$FE`, `$FD` and `$F8`, respectively. It sets `$FA=0`,
`$FB=$40` and `$FF=1` before calling `$E00F` at `$40a4`. Thus the indexed
tuple contributes `$FC=00`, `$FE=e7`, `$FD=03` and `$F8=11` in both editions.
Neither block assigns `$F9`; its value is therefore inherited from preceding
execution at these call sites. The code then branches unconditionally back to
`$4080` after the call.

At `$40a9`, X is cleared again and four bytes beginning at `$40dc` are loaded
into the same registers. The routine sets `$FA=0`, `$FB=$30` and `$FF=1`,
calls `$E009` at `$40cd`, compares A with zero and repeats while nonzero;
zero returns through `$40d4`. This tuple contributes `$FC=00`, `$FE=e3`,
`$FD=03` and `$F8=02` in both editions. These are literal register-flow
facts, not parameter meanings. In particular, the current evidence does not
identify the purpose of `$E00F`, `$E009`, or the values in the parameter block.

The `$4002` candidate also begins with `TII $2000,$2001,$000f`, followed by
`TII $2000,$2700,$0080`. Mednafen 1.32.1's PCE Fast HuC6280 implementation
(`src/pce_fast/huc6280.cpp`, `BMT_TII`, line 464) reads then writes one byte
per iteration and increments both logical addresses. That source confirms
forward-copy order. If the mapped source and destination are writable
aliases, the first overlapping transfer would repeatedly observe the
preceding write; this conditional consequence does not establish actual
runtime MPR mappings or contents. The second transfer is a 128-byte copy
between non-overlapping logical ranges.

## Boundary

The 14 bytes are now locked against authentic JP and US Track 02, including
the regional difference at `$40da-$40db`. Static reads show that the two
indexed four-byte blocks match across editions. No story selector, dungeon
ordinal, BIOS-call semantics, runtime transfer, or gameplay behavior follows
from this table. The source-to-runtime handoff at `$4002` remains unproven;
do not use these blocks as production data or infer the purpose of neighboring
regional bytes.
