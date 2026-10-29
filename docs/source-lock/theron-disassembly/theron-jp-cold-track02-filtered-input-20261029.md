# Japanese cold Track 02 filtered input trace

Date: 2026-10-29

## Setup

This reference-only PCE Fast run started the authentic Japanese Rev. 1 CUE
from original media with no loaded state. The full System Card 3.0 image has
MD5 `ff1a674273fe3540ccef576376407d1d`; Track 02 has MD5
`b7afb338ad31be1025b53f9aff12d73a`. The instrumented Mednafen 1.32.1 binary
was built on TRV2 with one job and has SHA-256
`157f4ea67e0b678b9f571fbcc6768145a7e7b0e80a5243fde8fe5793c1d499a1`.

The replay began with a four-second RUN hold at frame 9,600, four four-second
Button I holds from frames 10,080 through 11,520, then six quarter-second
Button I presses from frames 12,000 through 13,275. The CD data-port trace was
filtered to source-valid reads inside Track 02 with a 1,048,576-row cap. The
input trace cap was 8,388,608 rows. All raw traces stayed under a task-specific
TRV2 `/dev/shm` directory and were not added to Git.

## Results

The scripted-input verifier accepted all 11 events: each had an apply receipt
and a matching active-low controller read at input index 0. All 11 matching
reads were at System Card PC `$E4C5`; no scheduled mask reached a
non-System-Card read during its hold. The trace later contains idle reads at
`$44D2`, after the scripted events. This proves delivery to the firmware poll,
not Theron gameplay input or a game action.

The filtered CD trace contains 483,328 source-valid reads, all within Track 02,
from LBA 3,590 through 5,259. It includes a read at Track 02 raw file offset
`643,413` (trace sequence 251,157, LBA 3,639, System Card PC `$EA99`). That
offset matches the static `$D515` source candidate described in the
[`$D515/$D588` regional disassembly](theron-jp-runtime-input-poll-bank-alias-20261009.md).
This is a BIOS read of candidate media bytes; it does not prove that the bytes
were copied into the game bank or executed.

The capture recorded 173 CD-RAM target writes in the `$100000..$100010`
physical window. It produced no complete bounded HuC6280 `$23AD` invocation,
so there is no authenticated CD-to-game-RAM receipt. The VRAM-commit trace
reached its 1,048,576-row limit; the VCE trace contains 74,571 writes. The
bounded graphics chronology is therefore incomplete, and this run did not
produce a source-bound gameplay screenshot or pointer/action receipt.

Mednafen reported that the Japanese SBI sidecar was missing. Its absence is
recorded, but this run does not establish it as the cause of the missing game
handoff. Keep native world clicks and state-driven dungeon projection closed.

## Private trace hashes

| Trace | SHA-256 |
| --- | --- |
| Input | `b0f769ce8e99bc2f3596a4c1e6052f1250a52f37ff05ca760f131ccb10a8225b` |
| Track 02 CD data-port reads | `741f9b2c4db342b698bc7cd82eedfbfc18dcdd26dee5ca0afc72d39ee39a7ad1` |
| CD-RAM target writes | `5c90d6fa20987a9393cab286fcf9221667b43102e19109f7f511ffef611885da` |

The source media and firmware remain on TRV2 and were not copied into the
repository. The task-owned `/dev/shm` build and raw capture artifacts were
removed after analysis; only this summary and the trace hashes remain.
