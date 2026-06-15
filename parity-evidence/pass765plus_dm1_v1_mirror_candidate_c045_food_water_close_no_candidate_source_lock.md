# pass765plus DM1 V1 C045 Food/Water Close No Candidate Source Lock

## Source Anchors

- `CHEST.C F0333:30-67` materializes the open chest into `G0425` and `G0426`.
- `CHEST.C F0334:113-132` clears `G0426` and relinks only non-empty visible `G0425` slots.
- `CHAMPION.C F0297:243-298`, `F0298:270-298`, `F0300:511-515`, `F0301:606-614`, and `F0302:662-714` own leader-hand and C30/G0425 slot mutation.
- `PANEL.C F0344:1493-1561` and `F0345:1563-1616` draw and read the food/water panel state.
- `PANEL.C F0354:2299-2352` is the inventory/panel close territory used by the C503/C018 close path.
- `REVIVE.C F0280:124-132` and `F0282:744-806` are the C040 candidate path; this C045 close gate proves that path is not entered.
- `COMMAND.C F0359:1985-1990` dispatches M568/C040 panel clicks only for the resurrect/reincarnate candidate panel.
- `DEFS.H:2200`, `2205`, `3005`, `3008`, and `5876-5881` bind C040, C045, M565, M568, and G0423/G0425/G0426.

## Disjointness

This contract is asset-free and intentionally disjoint from the existing `mirror_candidate_c040_*`, `mirror_candidate_resurrect_*`, `mirror_candidate_c545_*`, `mirror_candidate_chest_close_*`, `mirror_candidate_full_chain_*`, `mirror_candidate_no_pending_resurrect_*`, `mirror_candidate_open_then_reselect_*`, and `scroll_pickup_*` gates. It covers only the C144 eye route from a chest-bound food C30/G0425 slot into the C045 food/water panel, followed by the C503/C018 close path, proving that the pending C040 candidate state is not displayed or consumed and that the transient C30 slot returns to the source chest chain before later food consumption reads the preserved panel data.
