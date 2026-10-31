# Theron cold-boot stage-2 `$4002` static source candidate

An authentic JP Rev. 1 cold-start capture sampled instruction-start memory
snapshots at logical PCs `$4002` (physical `$100002`) and `$4009` (physical
`$100009`). The v16 source-correlated trace indicates that the first 11 bytes at physical
`$100002-$10000c` were loaded from the contiguous Track 02 span described
below; four-byte snapshots at both PCs match the latest source-joined writes
recorded before the samples. This note extends the static media comparison through the
`RTS` at `$40d4` (211 bytes) and includes a MAME HuC6280 linear listing. Only
the first 11 bytes have this dynamic source-bound receipt. The evidence does
not identify a routine or establish loader/gameplay semantics.

The first-stage and v16 runtime evidence are documented in
[`theron-jp-cold-boot-cd-data-port-20261009.md`](theron-jp-cold-boot-cd-data-port-20261009.md)
and [`theron-jp-system-card-run-path-20261030.md`](theron-jp-system-card-run-path-20261030.md).
The v16 analysis associates the initial 11-byte buffer span with authentic
reads and latest recorded writes. Its CD sidecar headers do not identify the
shared event-sequence source, so the old cross-file join is insufficient to
prove common counter provenance. Its hook samples mapped bytes before the CPU
opcode read; it does not report actual CPU fetches. It does not extend this
bounded source correlation to the remaining 200 bytes of the static candidate.

## Authentic media spans

| Edition | Track 02 | Full-image MD5 | Full-image SHA-256 | Raw file offset | Span length | Span SHA-256 |
| --- | --- | --- | --- | ---: | ---: | --- |
| JP Rev. 1 | `TQJP02.bin` | `b7afb338ad31be1025b53f9aff12d73a` | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | `0x2973a2` | 211 | `3d16abea96bf6ce9610c7b3c9bc827f73e67f922dcda63e1c8681c7e3cd71a96` |
| US Rev. 1 | `TQUS02.bin` | `f23601102138f87c33025877767ebf76` | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | `0x297cd2` | 211 | `3d16abea96bf6ce9610c7b3c9bc827f73e67f922dcda63e1c8681c7e3cd71a96` |

The 211-byte span is unique in each full Track 02 image and is byte-identical
between the editions. The first 8-byte window and the overlapping continuation
bytes reproduce the two sampled runtime windows; this correlation does not
prove a dynamic source transfer. For JP, the candidate begins at LBA 4521,
raw-sector offset 18, user-data offset 2.

The listing in
[`theron-jp-us-cold-boot-stage2-4002-candidate-20261010.asm`](theron-jp-us-cold-boot-stage2-4002-candidate-20261010.asm)
was generated from JP media with MAME 0.285:

```sh
unidasm TQJP02.bin -arch h6280 -basepc 0x4002 -skip 2716578 -count 211
```

It is byte-checked against both authentic spans by
`tests/test_theron_v1_jp_us_stage2_4002_static_source_lock.py`. The registered
test also checks the complete-media hashes, exact offsets, candidate
uniqueness, span hashes, listing address continuity, and exact decoded bytes.

## Evidence boundary

The 211-byte listing remains static disassembly of a unique source candidate;
only its initial 11 bytes have a bounded runtime source correlation. The v16
capture associates each source read at `$EA99` with a value-equal `$EA9C`
store using event IDs, and the recorded instruction-start snapshots at
`$4002/$4009` match the latest writes in that capture. Its missing shared
sequence header markers limit independent cross-sidecar provenance. It does
not explain why the second-stage code ran, identify the remainder of the
loaded image, or show a game-owned transition. The candidate has no
established function name here; instruction mnemonics and targets are
presented as MAME's linear decode. See
[`theron-jp-system-card-run-path-20261030.md`](theron-jp-system-card-run-path-20261030.md)
for capture hashes, event IDs, and remaining limits.
