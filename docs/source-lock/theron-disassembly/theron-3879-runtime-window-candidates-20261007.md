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
these exact candidate offsets against the authentic media hashes. A follow-up
three-loop comparison also found that each occurrence has the same 141-byte
context (64 bytes on either side), SHA-256
`1f6df3c02976c33d01f8e15cd088ce186e46a7b7405c3bc6ef865c2cd2a94b10`.
Every context contains the eight bytes `a9 08 20 fb 44 4c 20 b9` at the
runtime-window-relative offset 9. This is consistent with the dynamic sequence
observed at `$48a8`, but still does not identify a transfer, destination, or
which of the six copies participated.

## Bounded HuC6280 decode at the dynamic entry

MAME 0.285 `unidasm -arch h6280 -basepc 0x485f -skip 611631 -count 139`
decoded the instruction-aligned prefix identically in three repeated loops.
The listing SHA-256 is
`526a0e4061e81a18786ac3cccb15879b5dfc25ec0f03e697c266e6e0c980e955`; the
reproducible output is saved in
`theron-jp-3879-runtime-window-candidate-20261007.asm`. The two remaining
context bytes are `$bd $1b`; the following three-byte instruction is left
undecoded because it crosses the authenticated 141-byte context boundary.
A prior unversioned listing digest could not be reproduced with MAME 0.285
and is superseded by this bounded command and recorded output. With the
13-byte candidate window provisionally aligned to `$489f`, the dynamic entry
decodes as:

```text
$48a8  a9 08       lda  #$08
$48aa  20 fb 44    jsr  $44fb
$48ad  4c 20 b9    jmp  $b920
```

The first two instructions and the `$48ad` opcode/operands match the captured
runtime PCs and mapped bytes at MPR2=`$68`. The alignment shown here is only a
candidate-relative overlay: the authentic Track 02 raw offset has not been
bound to that CPU address by a loader receipt. These bytes provide a bounded
decode of the observed entry and its immediate call/jump, not the callees'
semantics or a completed gameplay route.

## Candidate-relative `$44fb` target bytes

The observed entry calls logical `$44fb`. To inspect a candidate-relative
span at that address without assuming raw-file bytes map one-to-one to CPU
addresses, the test converts the `$489f` candidate location to MODE1 user-data
coordinates, subtracts `$3a4` user bytes, and converts back to a 2352-byte
raw-sector offset. This yields offsets `610459`, `911515`, `1212571`,
`1513627`, `1814683`, and `2115739`. All six 33-byte spans are identical in
the authenticated JP image (span SHA-256
`30c4e29752ff4fc5363876072d67f7b1f8e68f96af208064f0ee0e58fc62463d`). Their
MAME 0.285 `unidasm -arch h6280 -basepc 0x44fb -skip 610459 -count 33`
listing is recorded in
`theron-jp-44fb-runtime-target-candidate-20261007.asm` (listing SHA-256
`d05c8c2848469977cbfa23f87de74b916dcda10514d4b9f6d61dae41256eac01`).

This is a static candidate-relative overlay only. No loader receipt binds any
of these bytes to CD RAM, and the listing does not prove that this target was
executed or establish its semantics. The source test checks the listing and
the six authentic spans; it does not promote the candidate to runtime code.

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
