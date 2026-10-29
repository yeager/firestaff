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
were applied and read. For all twelve event windows, however, the only
classified poll boundary was the System Card (`$E4B4/$E4C5`); no
non-System-Card controller poll was observed.

## Result and limits

The trace contains 65,536 source-valid CD data-port rows before reaching its
configured row limit, 143 CD RAM-target writes, 58,952 VCE writes, and
1,048,576 VWR commits before reaching the VWR limit. Eight reads at guest PCs
`$42AB/$42B2` accessed `$138D..$1394`; these are not reads from the controller
port and do not establish gameplay input. The wrapper rejected the capture
because no complete bounded HuC6280 `$23AD` invocation was produced.

This run therefore proves bounded scripted-input delivery to the System Card
and records authentic CD/VCE/VWR activity. It does not prove a Theron menu
action, a game-owned input poll for the scheduled events, entry into the
dungeon, or causal CD-to-RAM-to-VRAM data flow. The trace limits still prevent
a complete rendering join, and no screenshot was captured to establish the
visible scene.

The private transient trace was stored under
`/dev/shm/firestaff-theron-cold-render-build-20261029/` on TRV2. It is not a
public evidence bundle and was not copied into the repository.
