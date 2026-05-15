# Pass565 DM1 V1 far-side floor/field order source lock

Status: passed

Closed gap: DM1_ViewportFloorFieldOrderSpec now includes mirrored D3R2 and the PC34/I34E-only D2L2/D2R2 far-side helpers. The D2 far-side helpers are intentionally marked as no floor ornament/F0115 thing pass because ReDMCSB only handles wall return or direct teleporter field there.

## Primary ReDMCSB evidence
- DUNVIEW.C:6304-6356 F0677_DrawD3R2: stairs/pit/floor ornament, F0115, teleporter field; wall branch returns.
- DUNVIEW.C:6846-6865 F0678_DrawD2L2: wall branch returns; teleporter draws field directly in C707_ZONE_WALL_D2L2; no F0115 thing pass.
- DUNVIEW.C:6877-6896 F0679_DrawD2R2: mirrored wall branch returns; teleporter draws field directly in C708_ZONE_WALL_D2R2; no F0115 thing pass.
- DUNVIEW.C:8478-8508 F0128_DUNGEONVIEW_Draw_CPSF: far-side replay order D3L2, D3R2, then D2L2, D2R2 before nearer lanes.

## Firestaff evidence
- dm1_v1_viewport_3d_pc34_compat.c: added the three missing floor/field order specs and source evidence strings.
- test_dm1_v1_viewport_3d_pc34_compat.c: source-lock test count now covers 13 specs and asserts D2L2 has no thing pass.

## Local references
- dm1_graphics /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT exists=True sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- dm1_dungeon /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT exists=True sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- greatstone_summary /home/trv2/.openclaw/data/firestaff-greatstone-atlas/index/SUMMARY.md exists=True sha256=b8ee685a2b60a49f305d0f1423e329d5e1019382b53598510833a46840bc3e2d
- csbwin_viewport /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp exists=True sha256=eb3b407d34b48f98113275e967dd10bc52114fad9957b46895fc22bdf64824b9
- csb_viewport /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Viewport.cpp exists=True sha256=2acaeec457166e4af8e18009de3b9081f773761fa5d8913cce0ccacb8c8ea12c

## Verification
- cmake --build build --target test_dm1_v1_viewport_3d_pc34_compat -j2 passed.
- ./build/test_dm1_v1_viewport_3d_pc34_compat passed.

## Non-claims
- No original-vs-Firestaff pixel parity is claimed.
- No DOSBox runtime capture was produced.
- Secondary CSB/CSBWin/Greatstone references are local lineage/context only; ReDMCSB remains primary.
