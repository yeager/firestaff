# Pass512 - DM1 V1 movement cross-reference audit

Status: BLOCKED_PASS512_DM1_V1_MOVEMENT_RUNTIME_CAPTURE_MISSING

Scope: DM1 V1 movement only. This is evidence, not a runtime behavior change.

## Repository ReDMCSB locks

- PASS reference/redmcsb-20210206/Toolchains/Common/Source/GAMELOOP.C:164-219 - keyboard input is drained through F0361 before F0380 processes the queue
- PASS reference/redmcsb-20210206/Toolchains/Common/Source/COMMAND.C:396-405,636-685,2045-2156 - movement tables map C001..C006 and F0380 dispatches after cooldown filtering
- PASS reference/redmcsb-20210206/Toolchains/Common/Source/CLIKMENU.C:142-347 - F0365/F0366 own turning, relative stepping, collision, sensors and cooldown
- PASS reference/redmcsb-20210206/Toolchains/Common/Source/MOVESENS.C:438-497,760-818,1553-1794 - F0267 commits party coordinates and source-before-destination sensors
- PASS reference/redmcsb-20210206/Toolchains/Common/Source/DUNVIEW.C:8318-8338,8468-8542 - F0128 consumes direction/mapX/mapY and derives visible squares with F0150

## Authentic PC 3.4 data anchors

- PASS ~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- PASS ~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS ~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::TITLE sha256 adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745

## Current blocker

Authentic original-runtime evidence is still required: a non-static PC/I34E keyboard-buffer-to-F0380 route transcript and representative movement/HUD/viewport captures tied to before/after party tuples.

The old Greatstone, CSBWin, CSB clone, extracted-media, and prior-generated-manifest dependencies are not movement truth owners and are no longer prerequisites for this gate.

Not claimed: pixel parity, viewport rendering changes, a binary-level F0380 body breakpoint, or route promotion from source/media identity alone.
