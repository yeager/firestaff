# Authentic US Theron VDC turn pair, 2026-10-25

## Source and capture method

This paired capture replays the authentic US Theron save state
`authentic-post-akutuba-f17f377df210b4a3ae904a13fb85a7f0.mc0` (MD5
`f17f377df210b4a3ae904a13fb85a7f0`) with the original USA CD and System Card
3.0. The CD layout ID is `bee0988239a817f20a64cd38fc8caeac`; Track 02 is
`TQUS02.bin` (MD5 `f23601102138f87c33025877767ebf76`); System Card 3.0 has MD5
`ff1a674273fe3540ccef576376407d1d`. These identities match the existing
[authentic Drator route](theron-authentic-drator-menu-route-2026-08-21.md).

The capture used the instrumented Mednafen 1.32.1 **PCE** module, which loaded
this state successfully. The same state is not compatible with this build's
PCE Fast module (`VDC0` is missing), so PCE Fast results must not be substituted
for this pair. A capture-only frame limit stopped both sessions after 200
emulated frames. No game sectors, level data, objects, or state fields were
modified. Controller input was the only injected game stimulus.

## Paired observation

Both captures begin with the same bound party bytes: direction `$203F = 01`,
position `$2040/$2041 = 02/03`, and cursor `$2E3B/$2E7B = 78/78`.

| Capture | Input | Final party bytes | Final cursor | VRAM MD5 |
| --- | --- | --- | --- | --- |
| `us-control-pce-v2` | none | `01 02 03` | `78/78` | `83d417ea0fa4587ec272b948bd83f8db` |
| `us-turn-pce-v2-traced` | `right@1:140,down@2:33,i@145:5` | `02 02 03` | `30/8f` | `60fb28ccd234855e350a530856d4687c` |

The controller-input trace accepted all three planned events and recorded the
right, down, and Button I masks at the PCE input port. The direction byte
changed while both party coordinates remained fixed. The viewport VRAM also
differs between the two captures; the VCE snapshot is identical in both
(`51cc0d97fea1d97b29519f483de3dc82`), and the VDC geometry remains
`MWR=005a`, `HDR=0327`, `VDR=00c7`.

## Audit of a separately labelled forward capture

The private capture directory also contains `us-forward-pce`, recorded with
the same save state, PCE module, 200-frame boundary and input plan. Its input
trace confirms all three planned events were applied (145 apply records).
However, its final party bytes are still direction/position
`02/02/03` (`$203F..$2041`), its cursor remains `$30/$8f`, and `$2905` is
cleared. Those values exactly match `us-turn-pce-v2-traced`; neither capture
shows a coordinate commit. The complete main-RAM snapshots differ in 49
bytes, so they are not duplicate files, but their position and cursor fields
do not support treating `us-forward-pce` as a successful move. Its label is
not movement evidence.

Two follow-up PCE-module replays against the same authentic autoload state
were also rejected as movement evidence. `up@1:140` and
`right@1:20,up@30:140` each had their scripted controller masks applied and
subsequently observed at a non-System-Card controller poll, while their final
party bytes remained `01 02 03`. Both bounded runs emitted complete VRAM,
VCE, SAT, VDC-state, and main-RAM snapshots, but did not reach the dynamic
receipt required by the capture driver before its timeout. They therefore do
not establish a source-owned movement result or a pose-to-frame mapping.

This audit applies only to the private 2026-10-25 capture set and does not
replace the separately source-locked 2026-08-21 movement record, which names
a Track 02 file/transport hash (`ceb02343868f80cec899e9b239aff2da`, MODE1/2048)
different from this pair's raw MODE1/2352 BIN hash. The hashes are not directly
comparable. Reproduce that source record independently before joining either
capture set to a screen cell.

## Limits and next use

This pair proves that an authentic loaded state can produce distinct native
PCE screen captures after controller input and that the saved party position
did not change in this route. It does **not** prove a forward-movement click,
identify which screen tiles correspond to which Track 02 records, or provide a
complete world renderer. The cursor ending at `$30/$8f` and the cleared
`$2905` command byte do not bind a source click target. Do not publish this
input plan as a mouse hitbox or use its output to invent square-to-tile
semantics.

Private VRAM, VCE, SAT, RAM, and input-trace files remain on TRV2 under
`/home/trv2/firestaff-theron-evidence/capture/theron-native-viewpair-20261025/`;
no game media, BIOS, save state, or capture binary is stored in Git. The
instrumented emulator was used only to obtain reference evidence. Firestaff's
production runtime remains native and must continue to render from its own
source-backed game state.
