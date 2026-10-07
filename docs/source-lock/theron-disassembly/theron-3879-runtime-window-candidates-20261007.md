# Theron `$4f06 → $48a8` runtime-window candidates

This evidence note narrows the source of the runtime bytes without claiming
that a Track 02 candidate was loaded into CD RAM or that the routine has
gameplay semantics.

## Authenticated runtime observation

An isolated PCE Fast capture loaded the emulator-created Japanese Rev. 1
Akutuba state (MD5
`14dec90b96ec3e14622ec0fab535a92f`) with the authentic JP Rev. 1 CUE,
System Card 3.0, and Track 02 (MD5
`b7afb338ad31be1025b53f9aff12d73a`). The instrumented Mednafen binaries
were identified by MD5 `b7fb204439657af0e15e6515806b0194` and
`727dd43ffe04bbb827a02ddf32afe1d1`.

At both logical `$4f06` and `$48a8`, the capture observed MPR2=`$68`, physical
PCs `$0d0f06` and `$0d08a8`, and the mapped 13-byte `$489f..$48ab` window:

```text
16 00 04 1c 02 80 65 00 00 a9 08 20 fb
```

Three captures reproduced both runtime rows, including two runs from the
second instrumented build. Thus the runtime opcode at `$48a8` is `$a9`, unlike
the static US and JP Track 02 window, which has `$80` there. These are
emulator observations, not Firestaff parity evidence.

## Authentic media candidate scan

Three bounded, SHA-256-verified scans of authentic `TQUS02.bin` and
`TQJP02.bin` searched for the full 13-byte runtime window. The US image
(8,104,992 bytes; SHA-256
`f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565`)
contains no exact match. The JP image (8,102,640 bytes; SHA-256
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`)
contains six:

| Raw file offset | Raw sector | In-sector offset |
| ---: | ---: | ---: |
| 611,695 | 260 | 175 |
| 912,751 | 388 | 175 |
| 1,213,807 | 516 | 175 |
| 1,514,863 | 644 | 175 |
| 1,815,919 | 772 | 175 |
| 2,116,975 | 900 | 175 |

The repeated copies are candidates only. The scan does not establish which,
if any, supplied the runtime bytes. The CTest
`theron_3879_runtime_window_candidates` repeats the bounded scans and locks
these exact candidate offsets against the authentic media hashes.

## CD-RAM write probe and remaining gap

Mednafen PCE Fast source maps CD banks `$68..$87` to `ROMSpace` and
`HuCRAMWrite` (`src/pce_fast/huc.cpp`, `HuCRead`/`HuCRAMWrite`, lines 73–80;
CD mapping lines 315–322). The instrumented cold boot therefore traced CPU
writes to physical `$0d089f..$0d08ab` while using the authentic JP CUE and
System Card, without loading a save state. The only 13 writes in that window
were zero-initialization by System Card PC `$ea9c` (physical `$000a9c`) with
MPR2=`$80`; there were no CD read commands, no non-System-Card polls, and no
transition. The separate save-state capture had no writes in the window
because RAM contents were restored before tracing began.

The source route that places the nonzero runtime bytes in CD RAM remains
unknown. The next positive capture must show a non-System-Card loader or
writer, bind its bytes to one of the six authentic JP offsets, and then
reproduce the `$4f06 → $48a8` execution before any alternate-route semantics
are assigned. Do not infer self-modifying code, a direct CD transfer, or
gameplay behavior from the current evidence.
