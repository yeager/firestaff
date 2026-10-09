# JP Stage-2 `$45xx` tier-3 source match

The authentic JP Rev. 1 Track 02 image contains the same exact bytes as the
already disassembled US `$45xx` tier-3 windows. The comparison uses the
normalized Stage-2 user-data payload from JP BIN MD5
`b7afb338ad31be1025b53f9aff12d73a` and US BIN MD5
`f23601102138f87c33025877767ebf76`. Their Stage-2 records begin at raw
sectors 1223 (JP) and 1224 (US), respectively; each of the compared windows is
read as user data, not from raw-sector headers.

| Stage-2 user offset | Bytes | FNV-1a 64 (both regions) |
| --- | ---: | --- |
| `0x4215` | `0x36` | `caecadb870bf74fd` |
| `0x4417` | `0x8b` | `235acb6a8b2de1fe` |
| `0x44a2` | `0x77` | `1591f4ba3ca4eae2` |
| `0x42db` | `0xc6` | `e53e3602ffa2397c` |
| `0x4519` | `0x39` | `7303d458fe565496` |

The adjacent Stage-2 user window at `0x4190` is also byte-identical in both
images (18 bytes; FNV-1a 64 `ec63ab69de7642e5`). Its encoded `JSR $4215` is at
logical `$819e`, immediately followed by `RTS` at `$81a1`. This statically
connects the caller bytes to the target operand; it does not prove which MPR
mapping executes either copy. The shared instruction listing is in
[`theron-us-stage2-huc6280.asm`](theron-us-stage2-huc6280.asm), and the focused
real-media check is `theron_v1_stage2_45xx_tier3_regional`.

This adds JP byte provenance only. It does not prove runtime bank selection,
execution, VDC behavior, movement, rendering, or gameplay semantics.
