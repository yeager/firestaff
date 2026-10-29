# Japanese cold-start bounded input trace

Date: 2026-10-29

## Setup

This reference-only capture started the authentic Japanese Rev. 1 CUE from
original media with the canonical System Card 3.0 image, without an autoloaded
state. The System Card MD5 was
`ff1a674273fe3540ccef576376407d1d`; Track 02 MD5 was
`b7afb338ad31be1025b53f9aff12d73a`. The fresh instrumented Mednafen 1.32.1
binary was built from the checked-in patch set on TRV2 with one build job
(SHA-256 `0998cb0932375dfba78fce53d07b8d1d04dd3a107b2728b9e53a06a707af0a3e`).
No emulator output, game media, firmware, or save state was added to Git.

The scripted input trace limit was raised to 8,388,608 controller reads to
cover long cold routes. Twelve Run/Button I events were scheduled from frames
480 through 5,520, with 300-frame holds. The capture produced 3,281,738
controller-read rows and the verifier confirmed all twelve scripted masks
were applied and read at port 0. The verifier's original report counted only
the exact `$1000` register address, so it classified the event-window reads as
System Card polls and missed the game's mirrored-address scan at frame 646.
The separately recovered scan was at index 4 with raw `$0000`; it does not
show the scheduled port-0 masks reaching the game.

## Result and limits

The trace contains 65,536 source-valid CD data-port rows before reaching its
configured row limit, 143 CD RAM-target writes, 58,952 VCE writes, and
1,048,576 VWR commits before reaching the VWR limit. At frame 646, eight
game-owned reads at `$42AB/$42B2` accessed `$138D..$1394` with `index=4`,
`raw=0000` and `value=3F`. These addresses are mirrored PCE input registers,
but `index=4` is the fifth controller slot; the configured player input is
index 0. This is game-owned multitap polling, not evidence that the scheduled
port-0 Run press reached the game's control path.

The independent cold-joined JP RAM trace records the code window
`B1 00 9D E0 47 C8 E8 B1` at `$42AB` and
`B1 00 9D E0 47 C8 E8 84` at `$42B2`. HuC6280 disassembly gives `LDA ($00),Y`,
`STA $47E0,X`, `INY`, `INX`, then repeats the read/store pair. The original
Mednafen 1.32.1 source confirms `$138D..$1394` dispatch through its mirrored
input-register decoder: `src/pce_fast/ioread.inc:1-16,84-93` and
`src/pce_fast/input.cpp:141-198`. `INPUT_Read` selects the device using
`read_index`, so the recorded index-4 reads do not reveal a port-0 action.

The wrapper rejected this capture because no complete bounded HuC6280 `$23AD`
invocation was produced.

This run proves bounded scripted-input delivery to the System Card, observes
the game's poll of controller index 4, and records authentic CD/VCE/VWR
activity. It does not prove a Theron menu action, a game-owned poll of
controller index 0 for the scheduled events, entry into the dungeon, or
causal CD-to-RAM-to-VRAM data flow. The trace limits still prevent a complete
rendering join, and no screenshot was captured to establish the visible scene.

The private transient trace was stored under
`/dev/shm/firestaff-theron-cold-render-build-20261029/` on TRV2. It is not a
public evidence bundle and was not copied into the repository.
