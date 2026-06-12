# pass765plus DM1 V1 chest pickup during resurrect-pending non-leader source lock

## Source anchors

- ReDMCSB `CHEST.C` `F0333:30-67`: open chest G0426 materializes first visible C537..C544 entries through G0425.
- ReDMCSB `CHEST.C` `F0334:113-132`: close clears G0426 and relinks only non-empty G0425 slots.
- ReDMCSB `CHAMPION.C` `F0297:243-298`, `F0298:270-298`, `F0300:511-515`, `F0301:606-614`, `F0302:662-714`, and `F0284:93-131`: leader hand, C30+ chest slot clear/write, slot-box dispatch, and party leader context.
- ReDMCSB `REVIVE.C` `F0280:124-132` and `F0282:744-806`: C040 candidate pending and resurrect/reincarnate commit clear.
- ReDMCSB `PANEL.C` `F0344:113-145`, `F0345:155-200`, and `F0352:2111-2160`: panel redraw/eye boundaries while C040 chrome remains live.
- ReDMCSB `COMMAND.C` `F0359:1985-1990` and `F0378:1973-1983`: C040 and chest-panel pointer dispatch.
- ReDMCSB `DEFS.H:2088` plus C30/G0425/G0426/G0423/G0305/M070/M516/C040 and `C537..C544` (`3906-3913`).

## Disjointness

This gate is intentionally separate from the C545 non-leader hand mid-cast path, scroll-wheel resurrect confirmation path, mirror-candidate resurrect family, mirror-candidate chest-open-during-pending path, and chest-close-while-party-rotate-pickup-pending path. It uses no scroll wheel, no C545 mouth route, no party rotation, and no chest-open mutation while pending. The covered race is: non-leader G0426 chest open, C040 resurrect pending, C537 click reserved, C503/C045-style close underneath the C040 panel, C040 chrome and candidate slot preserved, then queued C537 resolved into the post-resurrect leader hand with the non-leader close chain compacted.
