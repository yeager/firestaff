# Japanese Theron PCE mouse serial-read capture

## Setup and input pair

Two eight-second Mednafen `pce_fast` captures used the authentic Japanese
Rev. 1 disc (`TQJP02.bin`, MD5
`b7afb338ad31be1025b53f9aff12d73a`) and the retained authentic Ak-Tu-Ba
dungeon state (MD5 `14dec90b96ec3e14622ec0fab535a92f`). The state was reloaded
for each capture. Both runs used the instrumented PCE Fast port-1 mouse and the
same timing plan; the motion run supplied a +12 X delta for 12 updates, while
the control supplied zero delta for those updates. Other scripted inputs and
timing were held constant.

The probe records the game-side PCE input reads and selected guest memory
accesses. It also captures RAM, VRAM, SAT, VCE and candidate pointer writes.
The motion run logged `pce_input_read` at logical `$44C1` / physical `$0D04C1`,
`sel=1`, value `$3F` on frames 301–312. The zero-delta control read `$30` on
the same frames. Both captures also read the controller register at `$44D2` / `$0D04D2`
with value `$3F` during the sampled interval.

## Consumer and output comparison

In both captures, the input handler at `$44C8/$44DC` wrote `$00` and then `$F0`
to `$28B8`. The later `$D2F2` store to `$2912` wrote `$00` in both captures.
The instrumented candidate pointer-write stream was byte-identical: it only
recorded `$28FE=$3C` from `$CCDD` and `$2902=$78` from `$CCE2`. It recorded no
write to `$2905`, the candidate command byte. The pointer-write trace SHA-256
was `c7ee8c7ab5a1c5c1fbeee27bead70e75625271e00d3f2ee1a2b9fcfa05a02bd8` in
both runs.

VRAM, SAT and VCE snapshots were identical between motion and control. Their
SHA-256 hashes were, respectively:

| Snapshot | SHA-256 |
|---|---|
| VRAM | `713ba0fba87655cb8f39cc6a86b283e7867894026f8597ed5e993bb6f774cdef` |
| SAT | `c546ac140d8bd70165a4fa6dc45d8fa511b4238911976a7553e9e213fab2439d` |
| VCE | `ba02dbd8f03665cce75dae462f53c0120757adc5864d751693d0744e2d0b2875` |

The raw RAM snapshots were not identical: 17 bytes differed, including one
byte at offset `$08B7`. The captures ended with
`transition=missing`, zero CD IRQs, and zero authenticated CD-to-RAM receipts.
Therefore the state replay does not bind runtime code or data to its disc
source, and the RAM differences are not attributed to mouse movement here.

## Conclusion and limits

This pair verifies that a scripted PCE mouse X delta changes the value read by
the game's controller routine at `$44C1`. It does not show the delta being
converted into a game cursor or world coordinate: the sampled `$28B8` and
`$2912` results, candidate pointer writes and graphics snapshots were the same
in both runs. No command or transition was observed. A source-bound cold
runtime capture and a game-owned cursor/target receipt are still required
before deriving native Firestaff mouse behavior. The finding is consistent
with the earlier disassembly and input-poll trace in
[`theron-jp-runtime-input-poll-bank-alias-20261009.md`](theron-jp-runtime-input-poll-bank-alias-20261009.md),
which also cautions that `$28B8` and `$2912` do not by themselves establish
field semantics.

Private captures and raw media remain on TRV2 under
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-pointer-state-consumer-20261029-r9-motion/`
and `...-r10-control/`; they are not committed.
