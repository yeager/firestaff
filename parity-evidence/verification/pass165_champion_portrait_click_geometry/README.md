# Pass 165 — champion portrait click geometry

Source-first answer for the next runtime route attempt.

## Recommended click

- Viewport-relative portrait box: x=96..127, y=35..63
- Safe center viewport click: x=111, y=49
- PC screen click after viewport origin y=33: x=111, y=82

Use this as the first action while facing a champion mirror/front wall. Only after candidate state is visible should C160/C161 be clicked.

## Checks

- SRC_GEOM_001 PASS — `DUNVIEW.C` lines 525, 2466, 3916: Legacy/byte-box portrait-on-wall area is viewport-relative x=96..127 y=35..63 (32x29).
- SRC_GEOM_002 PASS — `COORD.C` lines 1693, 1698, 1748, 1749: PC viewport origin is screen x=0 y=33; champion portrait dimensions are 32x29.
- SRC_GEOM_003 PASS — `DUNVIEW.C` lines 3609, 3924, 3928, 2105: For the front wall, the source copies the drawn wall/portrait zone into C05 clickable storage before drawing the portrait.
- SRC_GEOM_004 PASS — `CLIKVIEW.C` lines 348, 349, 415, 417, 431: Runtime clicks are converted to viewport-relative coordinates, then tested against C05; C05 triggers F0372.
