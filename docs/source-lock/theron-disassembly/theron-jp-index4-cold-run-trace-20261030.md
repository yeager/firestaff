# Japanese cold RUN input at controller index 4

Date: 2026-10-30

## Capture

A fresh cold PCE Fast capture used the authentic Japanese Rev. 1 CUE and
Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) with System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`). No save state was loaded. The
instrumented Mednafen 1.32.1 build used the checked-in Theron trace patches and
was built on TRV2 with one job.

The replay held RUN (`mask=0008`) for 300 emulated frames from frame 480,
through controller index 4. This index was chosen because an earlier cold JP
trace observed the game-side input scan at `$42AB/$42B2` reading index 4.

## Result

At frame 480, PCE Fast recorded `raw=0008` at BIOS PCs `$E4B4/$E4C5`, register
`$1000`, index 4; the active-low result at `$E4C5` was `$37`. The scripted-input
receipt confirms one applied event and one matching controller read, but its
only witness is the System Card poll at `$E4C5`. It observed 23,320 input reads
through frame 2691, with no game or non-System-Card poll boundary.

The same run recorded zero CD IRQ callbacks, raw-sector spans, CD data-port
reads, authenticated CD-to-RAM receipts, and game-main-RAM `$E009` dispatches.
The transition receipt is `missing`; the bounded capture exited at its timeout.
This proves delivery of the held RUN bit to the System Card's fifth controller
slot only. It does not prove a game-menu action, gameplay entry, active map,
or movement semantics. Selecting index 4 did not reach the observed game-side
poll.

The raw trace remains private at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-index4-run-20261030/`.
The input trace SHA-256 is
`b99226c1514b4479e5be9401f0218ac1d9dcbd7a52b5a27f91bc31e9ae4fe52f`; the
transition receipt SHA-256 is
`4ef8363191f002cf97f9e0906a41fa7cef0447e77c9a3a058345cbdb44219b5c`.
No game media, firmware, save state, or screenshot was added to the repository.

## Next evidence needed

Bind the cold System Card selection/launch sequence to a game-owned input poll
and the first authenticated CD-to-RAM handoff. Until that occurs, do not use
the `$42AB/$42B2` index-4 scan as evidence that the System Card RUN event was
consumed by gameplay.
