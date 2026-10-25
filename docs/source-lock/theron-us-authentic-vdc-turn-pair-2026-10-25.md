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
