# DM1 V1 Mirror Candidate C040 Panel Browse Pickup Rotate Race

This pass768 runtime regression gates one narrow race: a C040 resurrect /
reincarnate mirror candidate is live on the inventory panel, `G0426` still
tracks an open chest, and the input window receives both a scroll-wheel leader
rotation and a click over a chest slot.

The expected result is deliberately small:

- The wheel event is queued/read through the Firestaff wheel lineage and the
  leader rotation completes.
- The click over the chest slot is routed through the live C040 panel content,
  not through the chest-panel C058..C065 path.
- No C30+ pickup occurs, no chest close occurs, and `G0426` plus all visible
  `G0425`/C537..C544 slots remain byte-stable.
- The C040 panel, `G0299` candidate ordinal, selected candidate index, and
  champion candidate chain remain unchanged.

## ReDMCSB Anchors

- `CHEST.C F0333:30-67` opens `G0426` and materializes C537..C544 from
  `G0425`.
- `CHEST.C F0334:113-132` closes `G0426` and relinks non-empty `G0425`
  entries. This race must not reach it.
- `CHAMPION.C F0297/F0298:243-298` own leader-hand lifetime.
- `CHAMPION.C F0300:511-515`, `F0301:606-614`, and `F0302:662-714` own the
  slot/C30+ path. This race asserts that the C040 route prevents F0302 pickup.
- `PANEL.C F0346/F0347:1624-1657` gives `G0299`/C040 priority over normal
  action-hand chest-panel content.
- `PANEL.C F0354:2195-2242` is the status portrait redraw route.
- `REVIVE.C F0280:124-132` publishes the C040 candidate; `F0282:744-806` is
  the only accept/cancel clear path and must not run here.
- `COMMAND.C F0359:1452-1668` queues clicks and `F0378:1956-1993` dispatches
  C081 panel clicks by current panel content.
- `COMMAND.C F0361:1709-1813` writes queued keyboard/wheel-like commands.
- `COMMAND.C F0380:2045-2178` drains queued turn/slot commands.
- `CLIKCHAM.C F0367/F0368:20-73` sets leader identity.
- Existing Firestaff wheel lineage names `MOUSE.C F0077:97-126` and
  `F0078:128-168`; this local ReDMCSB Common/Source checkout exposes
  `F0077/F0078` as screen-update prototypes at `DEFS.H:6886-6895`.
- `DEFS.H:2088` C10, `C016..C065`, `C160..C162`, `G0420`, `G0423`, `G0425`,
  `G0426`, C040, C045, and C030 slot constants.

## Non-Overlap Claim

This is not a click-cancel, deadzone, cancel-reselect, resurrect confirmation,
C159 click/rotation, C040 chrome owner-swap, redraw-after-chest-close,
C040/C045 close, C545 pickup/drop, chest-close, chest-open-during-pending,
inventory-click-during-rotation, keyboard/left-click rotation, save/load,
teleporter, scroll-pickup-then-portrait, scroll-pickup while rotation is in
progress, non-leader scroll pickup, or pending-hand queue test.

The fresh surface is the live C040 panel browse state plus same-window wheel
leader rotation and a chest-slot click whose panel route must remain C040.
