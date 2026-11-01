# JP cold host-key route: System Card branch still unjoined

**Capture date:** 2026-11-01

**Result:** bounded reference capture; no verified BIOS-to-game handoff

**Scope:** compare a physical Mednafen host-key route with the scripted cold
route using authentic Japanese Rev. 1 media and System Card 3.0.

## Setup and input

The isolated PCE Fast run used the authentic JP CUE and System Card 3.0, no
save-state, and a separate Xvfb display and Mednafen profile. The instrumented
Mednafen binary was built from the local trace patch set and had SHA-256
`688f12959c49aa51d36ced85f4bd8223e87dfce070e1a20bf36f23c822b95c67`.
Host-key events were RUN at 8 seconds, Button I at 20 and 32 seconds, then
Button I at 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, and 64 seconds. RUN and
the first two Button I holds lasted four seconds; the remaining holds lasted
one second.

The input trace contains one non-repeat RUN key-down and key-up and 14
non-repeat Button I key-down and key-up pairs. System Card controller reads at
`$E4C5` returned active-low RUN (`raw=$0008`, `value=$37`) during frames
155–192. Thus the host-key route reached a real BIOS controller poll; event
records alone are not being used as guest-consumption evidence.

## BIOS and loader observations

The bounded bootstrap observer recorded 8,192 rows and reached its configured
cap. It observed `$E11B`, `$E14C`, and `$E176`, but recorded no instruction
start at `$E11D`, `$E12C`, `$E130`, `$E174`, `$E209`, or `$E20A`. The decision
trace independently passed its structural verifier and recorded 163 rows over
six configured PCs. Its only `$E176` decision row does not establish the
clear-carry launch route.

The observer also recorded 128 instructions in its `$4000` target window, at
its configured cap. That window is not joined to the required `$E11D` carry
branch, so it does not by itself prove the BIOS launch handoff or game-owned
execution. The separate `$6148` code-follow observer reported
`target_not_reached` with zero rows. The exact-LBA 3674 CD data-port trace has
zero reads. No authenticated CD source read, CD-to-RAM receipt, game-owned
input poll, or gameplay transition was established.

The status trace hit its 65,536-row cap. Structural and source checks passed
for the available bounded traces:

| Observer | Verifier result |
|---|---|
| System Card status | 65,536 rows; cap reached |
| Callback state | 587 rows passed |
| System Card I/O/RAM | 22 rows passed; two of 30 configured addresses observed |
| System Card decisions | 163 rows passed |
| Bootstrap/target windows | 8,192 / 128 rows; branch-to-target launch check failed because `$E11D` was absent |
| `$6148` code-follow | `target_not_reached`, zero rows |
| Exact-LBA 3674 CD data port | zero reads |

The capture wrapper did not produce a final accepted receipt: its post-capture
step could not open `verify_theron_jp_code_follow_trace.py`, which had not been
copied into the isolated remote script directory. The trace files were then
checked directly with the current repository validators. These independent
checks validate the available bounded records; they do not convert this run
into a wrapper-accepted handoff.

## Trace identities

Raw traces and original media remain on TRV2 at
`/home/trv2/.cache/codex-theron-cdda-route-20261101/hostkey-handoff-ab-20261101/`;
they are not committed.

| Trace | SHA-256 |
|---|---|
| Input | `1ae03cd1de7067478197f1ae130168303ca80ce555b213d81a9e4770e9ac8844` |
| Bootstrap/target follow | `68748afe3650709d7ec3d8a1d6664a7b0bdc7f96eb7d2d461c91696a35d35738` |
| Callback state | `e90bb90c4808296c2596a707f9ffc83c9ea88e8a75801e1e129a8fbde8518d2c` |
| Decision | `37c37884f3759a00f99012ca4b9903b7dfc0fabc1966c80ef1df024118165349` |
| I/O/RAM | `aeea2707694ad1d3c087ce5198ffcbfa6e67cb296229c7d82e06a1689fea72b5` |
| Status | `04fb69e52da1f53b2762b6558590bdd3c72f1580a600a728d42c483fda0ec4d3` |
| Dispatch | `c73cd91e642ede02058eb8d70adbd243200547795e02d16fa436fd368f4d1255` |

## Next step

Keep the manual host-key route as the input reference, but capture the missing
System Card carry decision and a continuous, source-authenticated transfer to
game-owned code. Preserve the independent `$4000` window and source-scope the
BIOS/controller observers so they do not cap before that branch. Then require
a wrapper-accepted cold handoff before resuming the LBA 3674 loader-to-map-byte
join. This reference run provides no new evidence for native gameplay parity.

See the [JP System Card RUN analysis](theron-jp-system-card-run-path-20261030.md),
[cold map-byte writer capture](theron-jp-cold-map-byte-writer-capture-20261101.md),
[capture readiness](../../theron/THERON_CAPTURE_READINESS.md), and
[Theron TODO](../../../TODO-theron.md).
