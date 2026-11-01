# Theron JP long-hold title action and cold Akutuba capture (2026-10-31)

## Authentic inputs and isolation

The cold reference run used the authentic Japanese Rev. 1 Track 02
(`TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`) in Mednafen 1.32.1, PCE Fast. The
matching CUE and regional audio came from the user's original Theron archive.
Mednafen save, backup, snapshot and movie directories pointed to a separate
task-scoped folder on the external disk. No source media, firmware or
emulator-produced artifact was copied into the repository.

## Observed cold route

1. A four-second Return/RUN hold at the System Card prompt reached Theron's
   title menu.
2. With NEW GAME selected, holding Button I (Z) for four seconds opened the
   file-selection screen.
3. With FILE 1 selected, a second four-second Button I hold entered the
   Akutuba opening/story sequence.
4. Twelve Button I presses, each held for about one second with a one-second
   release interval, advanced the story screens to the first-person Akutuba
   dungeon.
5. A Mednafen state-save command wrote a 235,138-byte `.mc0` state from that
   loaded dungeon screen. Its SHA-256 is
   `b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068`.

The private screenshot after FILE 1/story entry has SHA-256
`d6d10b394e3c3c3329bee2d8b333c02e7a64754eedf2f8c7d64b86353b49cd43`; the
first-dungeon screenshot has SHA-256
`36da65085fd2438184d87a2af1b5ddddf2a402a2b0ef20dc3e2aa2d2a60327f0`.
These images and the `.mc0` state remain outside Git. The save-state is an
emulator CPU/device snapshot, not an in-game Backup RAM save.

## Evidence boundary

This visibly confirms that, on the Japanese Rev. 1 route, a four-second
Button I hold on the selected NEW GAME and FILE 1 actions advances into the
real game. It also provides an authentic, emulator-created dungeon state for
future diagnostic replay. It does not establish a same-session guest input
read, authenticated CD-to-RAM handoff receipt, a reachable movement result,
positive Backup RAM progress, or native Firestaff parity. A later exploratory
mouse input ended in Game Over; no movement or click semantics are inferred
from that attempt. The retail manual documents Button I selection, but not a
multi-second timing requirement.
