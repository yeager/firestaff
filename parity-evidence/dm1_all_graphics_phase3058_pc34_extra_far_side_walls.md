# DM1 V1 PC34 extra far side wall planes

Source lock: ReDMCSB WIP20210206 `Toolchains/Common/Source`.

- `DEFS.H:2595-2611` adds PC34/I34E view squares D2L2, D2R2, D3L2, D3R2 outside the 12-square core.
- `DEFS.H:2695-2710` assigns PC34 view-wall indices with D3L2/D3R2 before the normal D3/D2/D1 wall planes.
- `DUNVIEW.C:6226-6331` draws D3L2/D3R2 wall branches only when `F0172_DUNGEON_SetSquareAspect` classifies the square as wall.
- `DUNVIEW.C:6837-6893` draws D2L2/D2R2 wall branches under the same wall classification.
- `DUNVIEW.C:8479-8508` inserts those supplemental planes around the normal D3/D2 row draw order.

Gate: `firestaff_dm1_v1_walls_occlusion_blockers_probe` now verifies the D3L2/D3R2/D2L2/D2R2 coordinates and wall-only visibility mask without changing the legacy core wall mask contract.
