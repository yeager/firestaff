# Theron cold-boot stage-2 `$4002` static source candidate

An authentic JP Rev. 1 cold-start capture sampled execution at logical PC
`$4002` (physical PC `$100002`) and `$4009`. The sampled instruction bytes
match a contiguous 15-byte candidate in JP Track 02. This note extends the
static media comparison to 127 bytes and includes a MAME HuC6280 linear
listing. It does not establish that Track 02 supplied the bytes executed at
`$4002`, identify a routine, or infer loader/gameplay semantics.

The runtime evidence and its first-stage boundary are documented in
[`theron-jp-cold-boot-cd-data-port-20261009.md`](theron-jp-cold-boot-cd-data-port-20261009.md).
That capture binds first-stage CD reads to authentic JP media and mapped RAM,
then samples the second-stage instruction windows. Its source-to-RAM receipt
does not extend to these `$4002` bytes.

## Authentic media spans

| Edition | Track 02 | Full-image MD5 | Full-image SHA-256 | Raw file offset | Span length | Span SHA-256 |
| --- | --- | --- | --- | ---: | ---: | --- |
| JP Rev. 1 | `TQJP02.bin` | `b7afb338ad31be1025b53f9aff12d73a` | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | `0x2973a2` | 127 | `9e0c7e6926c8ad3c02284e4027f37bcbd6eaa6cb4507f321cce814e4f2eb2aec` |
| US Rev. 1 | `TQUS02.bin` | `f23601102138f87c33025877767ebf76` | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | `0x297cd2` | 127 | `9e0c7e6926c8ad3c02284e4027f37bcbd6eaa6cb4507f321cce814e4f2eb2aec` |

The 127-byte span is unique in each full Track 02 image and is byte-identical
between the editions. The first 8-byte window and the overlapping continuation
bytes reproduce the two sampled runtime windows; this correlation does not
prove a dynamic source transfer. For JP, the candidate begins at LBA 4521,
raw-sector offset 18, user-data offset 2.

The listing in
[`theron-jp-us-cold-boot-stage2-4002-candidate-20261010.asm`](theron-jp-us-cold-boot-stage2-4002-candidate-20261010.asm)
was generated from JP media with MAME 0.285:

```sh
unidasm TQJP02.bin -arch h6280 -basepc 0x4002 -skip 2716578 -count 127
```

It is byte-checked against both authentic spans by
`tests/test_theron_v1_jp_us_stage2_4002_static_source_lock.py`. The registered
test also checks the complete-media hashes, exact offsets, candidate
uniqueness, span hashes, listing address continuity, and exact decoded bytes.

## Evidence boundary

This is static disassembly of a unique source candidate, not runtime
source-binding evidence. It does not prove that the candidate was loaded into
the `$4002` execution mapping, explain why the second-stage code ran, or show a
game-owned transition. The candidate has no established function name here;
instruction mnemonics and targets are presented as MAME's linear decode only.
