# Authentic Drator Menu Route, 2026-08-21

This note records the first cold start that proceeds from the original disc
and an authentic Akutuba completion to Drator's actual menu branch. No sector
bytes, level records or game objects were synthesized. The only injected
values are research inputs at exactly signature-verified original routines.

## Source

- USA-CD: layout-id `bee0988239a817f20a64cd38fc8caeac`.
- System Card 3.0: the local authentic firmware file.
- BRAM before startup: MD5 `ffabc8d19b0915d4d9632a7ae2e90a97`.
- Record `DMS-SG.001` has campaign byte `01` at file offset `$20`.

The capture must bind `filesys.path_sav` to the private capture home’s `sav`
directory. A copied `mednafen.cfg` may contain an older absolute path; without
the explicit binding, earlier attempts read an empty BRAM even though the
correct file was present in the capture home.

## Original Menu Sequence

The running 64 KiB logical snapshot shows:

- title/intermediate selection wait loop at `L6E3E`;
- generic dialog confirmation at `L5C8C`;
- scenario loop at `L6DBA`;
- availability table `$6D8F..$6D95 = 01 00 00 00 00 00 00`.

Mask `01` therefore means that the first scenario entry is the only one
unlocked. Drator is selected by accepting the default entry directly. Moving
down one step selects a locked entry and leaves the original in the same menu.

The reproducible research route is:

1. Actual RUN signal at emulated frame 9600 for the System Card screen;
2. `UP`, then `I`, in the signature-verified two-choice routine `L6E3E`;
3. two `I` confirmations in the signature-verified dialog routine `L5C8C`;
4. `I` directly in the signature-verified scenario loop `L6DBA`.

## Actual CD Branch

After scenario selection, the following original commands are added beyond
the shared startup chain:

| generation | LBA | sectors |
| ---: | ---: | ---: |
| 63 | 4886 | 8 |
| 64 | 4896 | 4 |
| 65 | 4901 | 1 |
| 66 | 4902 | 1 |
| 67 | 4903 | 1 |
| 68 | 4269 | 2 |

This proves an authentic Drator-specific menu/introduction branch.

## Dungeon Entry in the Same Session

A continued run along exactly the same route confirmed original routine
`L7552` and three additional `L5C8C` dialogs. The original then returned to
Track 02 and performed:

- LBA 3236, one sector, twice;
- LBA 3237 followed by contiguous 16-sector blocks;
- block commands through LBA 3381 in the captured load sequence.

Frame 160000 shows the actual 320×200 dungeon view with Theron's HUD and a
rendered stone corridor. VDC state is `MWR=005a`, `HDR=0327`, `VDR=00c7`, and
the processor runs the original dungeon program with MPR
`ff,f8,68,74,79,70,6d,00`.

Authentic snapshot hashes:

- VRAM: MD5 `0f7ed47b5f1f94d8f0151bf145c52a94`;
- VCE: MD5 `334dec8878e177123882beec2c3d3f83`;
- logical 64 KiB memory: MD5 `a826ce5386da13b08759a639a9cbbdab`;
- BaseRAM: MD5 `0462d75c9b934c2128fab28c1bd5a139`;
- CD trace: MD5 `20b09312c17ee0af11b056090f71393d`, 839767 bytes.

The original BaseRAM state in the same frame is:

- `$2031 = 02` (current level 2; the field remains unchanged while the
  coordinates in the authentic movement captures change);
- `$2038 = 02` in the final frame, but dynamic monitoring shows that the
  address is a working byte that previously cycled through values including
  `00→03→23→63`; it is explicitly not a verified level number;
- `$203F = 01` (eastward direction according to the already verified T520
  binding);
- `$2040/$2041 = 02/03` (party position `(2,3)`);
- `$20DA/$20DB = 01/00` (raw bank provenance, not a dungeon identity by itself).

The combination of the authentic campaign file, the original's only unlocked
scenario choice, the Drator-specific LBA chain, the subsequent Track 02 load
and the rendered dungeon view now proves Drator's actual dungeon entry on
level 2, at position `(2,3)`, facing direction 1/east. `$2038` is retained as
a raw working byte and must not be used as a level number.

## Cold-Start Retry with the Raw US Disc, 2026-09-25

A new local run used the complete raw US CD and BRAM file above, not the
previously normalized ISO layout. The CUE hash was
`63dbd2fab613b2e8030ff4e44b978a39`, Track 02 had MD5
`f23601102138f87c33025877767ebf76`, the CD layout ID was
`bee0988239a817f20a64cd38fc8caeac`, and System Card 3.0 had MD5
`ff1a674273fe3540ccef576376407d1d`. The BRAM hash was
`ffabc8d19b0915d4d9632a7ae2e90a97` both before and after the run.

Mednafen applied the RUN mask `$0008` at input frame 9600, but the CPU trace
contains no `$1000` read with `raw=0008` in that run. The authentic CD
performed four SCSI reads and delivered 25 raw sectors, but the run did not
reach the signature-verified Drator menu code: no Drator routine steps or
title-wait injection were logged, no CD-to-RAM destination could be bound,
and `$20DB` remained `00`. A separate run with the same media and
`THERON_CAPTURE_TITLE_WAIT_INPUT=run` also failed to reach the
signature-verified title-wait routine.

The local traces are in ignored `.codex-scratch`. They confirm the media and
input identities, but do not replace the earlier positive Drator capture or
provide new support for level, object or generator semantics. The reason this
cold-start replay does not reach the menu branch remains unresolved.

### Longer Replay with Full Controller Trace, 2026-09-25

The same raw US CUE, Track 02, System Card 3.0 and unchanged BRAM were replayed
with RUN at frame 9600. All four source hashes matched the values above. The
configurable input limit was raised to 262144 reads and 262144 writes; the
transition receipt reports 524288 total PCE input transactions. The CPU trace
shows zero `$1000` results with `raw=0008`; the RUN mask was therefore applied
by the input producer but did not reach the observed CPU read before exit.

The capture still reached only 25 raw sectors in four SCSI reads. The observed
game-owned `$E009` dispatch returned without a data read: zero game-owned
CD-to-RAM receipts, zero authenticated CD-to-RAM destinations and `$20DB=00`.
BRAM before and after had MD5 `ffabc8d19b0915d4d9632a7ae2e90a97`. The larger
trace limit therefore resolved log truncation around RUN but did not change
the menu outcome. The result remains negative and admits no Drator, level,
object or generator semantics.

A second raw-CUE run used replay plan
`run@1:1,run@480:30,i@900:30`. CPU reads at `$E4B7/$E4C8` observed RUN
(`raw=0008`) and I (`raw=0001`), but the CD receipt showed the same four
SCSI reads, 25 raw sectors, zero authenticated CD-to-RAM destinations and
`$20DB=00`. The BRAM hash was unchanged. See the ignored trace under
`.codex-scratch/theron-raw-cue-known-input-20260925/`.

The run was then repeated for 230 seconds with the same plan and a maximum
trace cap of 1048576 per read/write source. All three events, including I's
release at frame 930, appear in the trace; the longer time window added no
further CD reads. The receipt reached exactly 2097152 total PCE input
transactions. The CPU read RUN 24963 times and I 25029 times; all three events,
including release, thus reached BIOS reads. Even so, only the four commands
above and 25 sectors were recorded, with zero CD-to-RAM receipts and zero
`$E009` data reads. BRAM was unchanged. See
`.codex-scratch/theron-raw-cue-known-input-long-20260925/`.

### Control with the MODE1/2048 Projection, 2026-09-25

The same emulator, BRAM and input plan ran for 90 seconds with the authenticated
US Track 02 ISO (`ceb02343868f80cec899e9b239aff2da`) instead of raw
MODE1/2352 (`f23601102138f87c33025877767ebf76`). The CPU read RUN 24988 times
and I 25023 times, but the receipt still showed four SCSI commands, 25 raw
sectors, zero CD-to-RAM receipts and zero `$E009` data reads. BRAM remained
unchanged. In this current headless/scripted-input configuration, the sector
format therefore did not change the negative outcome; historical positive
captures use other input/run environments and cannot be attributed to the
format alone. Trace:
`.codex-scratch/theron-normalized-cue-ab-20260925/`.

As a control, a separate raw-CUE run attempted to use PID-bound Cocoa/Quartz
input (`run@8,run@20,i@30`). Mednafen could not start: SDL reported
`The video driver did not add any displays`, and the capture script exited
before sending any host key. This is not a negative game result and provides
no input receipt. The next reproducible step requires an actual SDL display/GUI
session; then the BIOS result should be followed through the CD command path
and compared with the earlier positive Cocoa capture.
