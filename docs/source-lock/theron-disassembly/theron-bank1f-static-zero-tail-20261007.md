# Theron's Quest bank-$1f static zero tail

This source check records why the static bank-$1f image cannot provide an
instruction listing for the later `$2600` RAM-consumer window. It makes no
claim that the zero bytes are executed or have `BRK` semantics in a gameplay
session.

## Authenticated inputs and byte range

The address range `$252B..$27FF` is 725 bytes, immediately after the
`RTS` at `$252A` in
[theron-us-bank1f-consumer.asm](theron-us-bank1f-consumer.asm). The range was
read from these authenticated projections:

| Projection | Full-file MD5 | File offset for `$252B` |
| --- | --- | --- |
| US `TQUS19.iso` | `51b40a17b92a30339957ba564aa0015c` | `$1F252B` |
| US `TQUS02.bin` | `f23601102138f87c33025877767ebf76` | `$2BD72B` |
| JP `TQJP02.bin` | `b7afb338ad31be1025b53f9aff12d73a` | `$2BCDFB` |

Each projection contains 725 zero bytes for this range. All three spans have
SHA-256
`26df0a7f3645a1ea2058196ac97b67e582bbd5229da670d1e4817398fc3bb6ff`.
The JP ISO was not installed for this check; no JP ISO claim is made here.

MAME `unidasm -arch h6280 -basepc 0x252b` decodes each extracted span as 725
`BRK` rows. Three repeated passes produced the same assembly SHA-256
`e6fcb86eed06ac0711dc3731a82abea311630ea6b3535131bc1be526c20f4655` for all
three projections. Because the source bytes are all zero and follow the
verified `RTS`, the linear `BRK` output is only a disassembler rendering of
padding; it is not a recovered routine.

## Boundary

The static image therefore contains no instruction bytes in `$252B..$27FF`
from which to recover the later `$2600` consumer. That consumer must be
located in dynamically loaded RAM and joined to its authenticated Track 02
source through runtime evidence. This observation does not identify the
loaded routine, source LBA, level, tile, object, or UI semantics.
