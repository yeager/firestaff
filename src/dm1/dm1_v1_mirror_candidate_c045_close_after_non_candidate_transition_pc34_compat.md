# DM1 V1 mirror candidate C045 close after non-candidate transition

Contract-only runtime evidence for the DM1 V1 food/water C045 close path when it fires after a non-candidate panel transition while the leader hand holds a non-empty C30 thing.

Source anchors:

- `CHEST.C F0333:30-67`: chest open materializes the visible G0425 chain.
- `CHEST.C F0334:113-132`: chest close clears G0426 and relinks non-empty G0425 slots.
- `CHAMPION.C F0297:243-298` / `F0298:270-298`: leader-hand put/remove mutation.
- `CHAMPION.C F0300:511-515` / `F0301:606-614`: C30 and chest-slot mutation.
- `CHAMPION.C F0302:662-714`: C537..C544 slot-box dispatch.
- `REVIVE.C F0280:124-132` / `F0282:744-806`: C040 candidate gate and resurrect panel path.
- `COMMAND.C F0359:1985-1990`: M568/C040 dispatch requires the resurrect panel and an empty leader hand.
- `PANEL.C F0344:1493-1561`, `F0345:1563-1616`, `F0354:2299-2352`: food/water draw/read and panel close chrome.
- `DEFS.H`: C030, C040, C045, C537..C544, G0425, and G0426.

Pinned behavior:

- A C045 close after a non-candidate transition preserves the leader-hand C30 thing.
- The source chain, visible G0425 slots, G0426 open-chest state, and C540 route stay unchanged.
- The close path does not enter `F0280`, `F0282`, `F0333`, or `F0334`.
- The close surface hash is identical with and without the immediately preceding non-candidate transition.

This is intentionally disjoint from pass674, pass686, pass710, pass711, pass736, pass745, and pass765plus. It does not claim original DOS pixel parity; it only records deterministic synthetic-framebuffer contract behavior.
