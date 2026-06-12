# DM1 V1 Champion Panel Portrait-Box Redraw States

This is a contract-only source-lock slice. It does not sample original DOS
pixels, load GRAPHICS.DAT or DUNGEON.DAT, or claim real-asset parity. The model
pins which redraw events reach the 67x29 C151..C154 status box, which events
continue to the inventory portrait-box blit, and which events stop at the
name/action-hand/status-chrome lanes.

## Source Anchors

- `CHAMDRAW.C F0291:498-677`: maps inventory/status slot boxes, selects
  `C033`, `C034`, or `C035` hand-slot chrome, then draws the object icon.
- `CHAMDRAW.C F0292:757-815`: gates the nine dirty bits, resolves the 67x29
  `C151..C154` status-box zone, fills live boxes with `C12`, calls `F0354` only
  for the inventory champion, or arms the non-inventory continuation
  `NAME_TITLE|STATISTICS|WOUNDS|ACTION_HAND`.
- `CHAMDRAW.C F0292:843-895`: applies the PC34 leader-name cascade, `C11` for
  the leader and `C09` for nonleaders.
- `CHAMDRAW.C F0292:898-935`: recomputes statistics and redraws `C033/C034`
  mouth/eye chrome.
- `CHAMDRAW.C F0292:1080-1110`: redraws the action hand through `F0291`, draws
  the action icon, optionally requests viewport redraw, and clears all nine
  dirty bits.
- `CHAMDRAW.C F0293:1117-1143`: applies a mask to active champions and calls
  `F0292` in champion-index order.
- `CHAMDRAW.C F0295/F0296:1153-1260`: scans changed leader/status/inventory/
  chest icons, suppresses the candidate-without-inventory case, and hands
  visible owner chrome changes to `F0292`.
- `CHAMPION.C F0302:662-714`: resolves the hand-slot pointer before the final
  `F0292` redraw of the affected champion.
- `DEFS.H C113..C116`, `C033/C034/C035`, and `C151..C154`: 16x14 champion icon
  zones, hand-slot box graphics, and the 67x29 status-box stride.

## Matrix

| Event | Status fill | F0354 portrait blit | Notes |
| --- | --- | --- | --- |
| Party leader rotation | No | No | Name-color cascade only; this slice records the C11/C09 result without duplicating pass683. |
| Hand-slot swap | No | No | `F0302` resolves the pointer, then `F0291`/`F0292` redraw action-hand chrome. |
| Status-hand rotation | No | No | Same order evidence as hand-slot swap; pass765 owns the M516/M070 route. |
| Mirror-candidate open/close | Yes | No | Owner/chrome transition reaches non-inventory status fill and continuation lanes. |
| Chest open/close | No | No | `F0296` scans chest icons and may request owner viewport redraw, but does not enter status fill. |
| Resurrect pending | No | No | `F0296` returns early when a candidate exists and no inventory owner is open. |
| Candidate pick | Yes | No | New candidate status fill takes the non-inventory continuation path. |

The only row in this matrix that would call `F0354` is an inventory champion
with `MASK0x1000_STATUS_BOX`; that narrow predicate is already owned by the
portrait-box blit dispatch gate. This pass instead locks the redraw-state
matrix around that predicate so future changes do not accidentally route
leader rotation, hand-slot swaps, status-hand rotation, chest refreshes, or
resurrect pending states into a portrait-box blit.
