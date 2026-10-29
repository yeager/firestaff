# Japanese cold-start dungeon and gated VWR captures

Date: 2026-10-29

## Setup

These reference-only PCE Fast runs used the authentic Japanese Rev. 1 CUE and
Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) with the authentic headered
System Card image (MD5 `ff1a674273fe3540ccef576376407d1d`; runtime body MD5
`38179df8f4ac870017db21ebcbf53114`). No save state was loaded. The
instrumented Mednafen 1.32.1 binary was built on TRV2; the transition receipt
records binary MD5 `00b2aa9c00cc158bb2dc87380b7f7c69`. Both runs replayed the
same eleven-event cold route: a four-second RUN hold, four four-second Button
I holds, then six short Button I presses.

The raw media, BIOS, screenshots and traces remain private on TRV2 under
`/dev/shm/firestaff-theron-vwr-cd-write-20261029-r3/captures/`.

## R3: initial RUN-only attempt

The initial 230-second cold run scheduled only one four-second RUN hold at
frame 9,600. Its input receipt placed the read at System Card PC `$E4C5`; no
game-owned poll or transition was observed. Track 02-filtered CD reads covered
LBA 3,590–5,214 with 63,488 source-valid reads and zero unbound reads. There
were 64 CD-RAM target writes, no authenticated CD-to-RAM receipt, no CD IRQ,
and `transition=missing`. The 1,048,576-row VWR trace hit its ceiling; the
11,592-write VCE trace merged into 1,123,720 temporal events marked
`possible_truncation_at_event_limit`. No screenshot was captured.

| R3 artifact | SHA-256 |
| --- | --- |
| Scripted input | `7ae3e5b5d1cc4acb84d7e3948f2ec0a29f13251fd09f499276a5fee8b15c34ef` |
| Input-consumption receipt | `b904ca495f2cbc2b0a3691921412d63c49f3ce333a5ec3d2e8fde7cbb5d7bba1` |
| Track 02 CD data-port reads | `94d80ed89fdfbbdfc3d3b9792834b5180340174f5d623a11f9ea81c39063ea64` |
| CD-RAM target writes | `ec6ba6f9873ff7686a3bca40b54f7c15075b67499fad019df08848a2ef40afcc` |
| PCE Fast VWR commits | `31ac52e9b3576903dd6c0d87ae7cfbf903ba231a258539ddbdca20b15e7ca1da` |
| PCE Fast VCE writes | `49aca345aede488f84d0e2a0a99049e630dc75d4f95a04f79d61aec311d13466` |
| Merged source-graphics chronology | `ef11a2788c1e8a505e345ef748632a21b2386d727619cf28ea44af50fee62e50` |

## R4: dungeon image, incomplete input and graphics traces

A real runtime screenshot captured during the route shows the first-person
Theron dungeon view. Its SHA-256 is
`171d6c45a24e05f148498854f746be3b1554d8c1d2cfb14a643778459fbc9877`. This is
Mednafen reference evidence; it does not establish native renderer parity.

The input-read trace stopped at its 1,048,576-row ceiling, ending at replay
frame 12,791. The first nine scheduled events have port-0 CPU-read witnesses
at `$E4C5`; the final two do not because their read windows fell beyond the
trace ceiling. RUN is recorded as raw `0008` / returned `37`, and Button I as
raw `0001` / returned `3e`.

The capture recorded 483,328 Track 02 source-valid CD data-port reads and 172
CD-RAM target writes, but no authenticated CD-to-RAM receipts, CD IRQ
callbacks, non-System-Card CD reads, or game `$E009` dispatch. Its transition
receipt says `transition=missing`.

The source-valid Track 02 gate opened the VWR trace, which reached the new
4,194,304-row limit. The VCE trace has 74,568 writes. The merger produced
4,752,372 events and reported `merged_limit_reached`; its order is temporal,
not causal, and the VWR hook excludes DMA writes. The wrapper rejected this
run because the input trace was truncated and required dynamic receipts were
absent.

## R5: all scripted masks witnessed, handoff still unproven

R5 repeated the same authentic cold route with a 4,194,304-read input ceiling.
Its scripted-input receipt SHA-256 is
`19831d067cfbf8890aec094b4b7eb3f9348a8bbf62fbc1e965daf6ba4e43a7aa`; its
transition receipt SHA-256 is
`6d880407294ea79abf4010fa5fa030b54b261e0265975016189b18d788395b6f`.

All 11 events have apply receipts and port-0 reads exposing the requested
masks. The verifier's event-window witnesses are all System Card polls at
`$E4C5` (`system_card_poll_reads=11`,
`non_system_card_poll_reads=0`). The complete input trace later includes game
code reads at `$44C1/$44D2` and `$42AB/$42B2`, after the scheduled event
windows. Existing mouse-consumer evidence identifies `$44C1/$44D2` as the
game's mouse serial-read routine; these later reads do not prove that the
scheduled cold-start masks reached a game-owned poll.

The transition receipt records 1,125,186 input reads, 483,328 source-valid
Track 02 reads, 172 CD-RAM target writes, zero authenticated CD-to-RAM
receipts, zero CD IRQ callbacks, zero non-System-Card CD reads, zero `$E009`
dispatches, and `transition=missing`. The VWR trace again reached its
4,194,304-row limit; the 4,752,372-event source-graphics merge reports
`merged_limit_reached`. The wrapper ended at its 300-second timeout with
`BLOCKED: dynamic receipts absent`.

## Limits and next evidence

The screenshot proves that the reference emulator displayed a first-person
dungeon scene during R4. The trace receipts do not independently prove the
cold menu-to-game handoff, source-bound transfer into game-owned RAM, causal
CD-to-VRAM flow, DMA graphics writes, or native Firestaff parity. The next
capture needs an authenticated game-owned handoff trigger and a bounded VWR
window that does not begin at the first Track 02 read; do not infer a code or
rendering map from the current time-ordered trace.

## TRV2 trace deduplication

After R5, the R4 and R5 artifact pairs were compared byte-for-byte on TRV2.
The identical R4 copies of the CD data-port, CD-RAM target, main-RAM consumer,
VCE, VWR, SAT/VDC state, pointer/movement, mouse, and SRAM traces were removed
from the tmpfs capture directory; their R5 counterparts remain there. The
R4-specific screenshot, input trace, and `3879-indirect-target` trace remain,
as do all R5 artifacts. This removes duplicate storage without discarding
unique observations.

## Save-state input-slot probe

On 2026-10-29, a separately built Mednafen 1.32.1 PCE Fast reference binary
replayed Button I at frame 120 for 30 frames against the Japanese Rev. 1
original CUE and an emulator-created Akutuba dungeon state. The slot-0 and
slot-4 input traces contain 2,390 and 2,392 `pce_input_read` rows respectively;
both report index 0 for every row, and neither records an input read at index 4.
Both transition receipts report `transition=missing`. Slot 4 therefore did
not produce a guest-poll witness; slot 0 also did not establish a game handoff.
This save-state probe does not establish cold-start provenance or native
Firestaff input parity, and no product input mapping was inferred from it.

The input traces were kept only in a task-specific `/dev/shm` build directory
on TRV2 and removed after recording the result. Their SHA-256 values were
`04a9ec93f39422d36cffb0b62045cb7e037643cf8cebc8b5e228541bd679495c` (slot 0)
and `081982846c23a513bb2cc91f06fe398a584fc5305e006898334fc0d94e4be573`
(slot 4).
