# Pass508 DM1 V1 viewport/wall runtime-readiness evidence

Status: PASS_PASS508_DM1_V1_VIEWPORT_WALL_RUNTIME_READINESS

## ReDMCSB anchors

- PASS DUNVIEW.C:8466-8542
- PASS DUNVIEW.C:7784-7844
- PASS DUNVIEW.C:7873-7938
- PASS DUNVIEW.C:5915-5933
- PASS DRAWVIEW.C:847-858

## Current Firestaff callback audit

- PASS wall, door, and F0115 families are present in `m11_dm1_f0128_execute_source_step`.
- PASS `m11_draw_viewport` uses per-square wall and foreground dispatch.
- PASS no active once-per-frame D3--D1 explosion replay remains.

## Original PC 3.4 members

- PASS /home/yeager/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/DUNGEON.DAT size=33357 sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 (in-memory/no-extraction)
- PASS /home/yeager/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/GRAPHICS.DAT size=363417 sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e (in-memory/no-extraction)
- PASS /home/yeager/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DM.EXE size=11471 sha256=4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4 (in-memory/no-extraction)

## Non-claims and remaining evidence

- No same-frame original DOS viewport capture is supplied by this readiness gate.
- No Firestaff-versus-original pixel equality or timing parity is claimed.
- MEDIA720 D3L2/D3R2 F0107 coordinate rows remain fail-closed pending authentic item-558 or executable evidence.
