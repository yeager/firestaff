# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass112-n2-route-probe`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- image0001-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0002-raw.png: classified title_or_menu, expected dungeon_gameplay
- image0003-raw.png: classified title_or_menu, expected dungeon_gameplay
- image0004-raw.png: classified title_or_menu, expected spell_panel
- image0005-raw.png: classified entrance_menu, expected dungeon_gameplay
- image0006-raw.png: classified entrance_menu, expected inventory
- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass112-n2-route-probe/image0001-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `f7f58cbefcf2` |
| 2 | `verification-screens/pass112-n2-route-probe/image0002-raw.png` | `title_or_menu` | `dungeon_gameplay` | NO | sparse viewport plus colorful/right-column title-menu art | `307323fbc1f7` |
| 3 | `verification-screens/pass112-n2-route-probe/image0003-raw.png` | `title_or_menu` | `dungeon_gameplay` | NO | sparse viewport plus colorful/right-column title-menu art | `307323fbc1f7` |
| 4 | `verification-screens/pass112-n2-route-probe/image0004-raw.png` | `title_or_menu` | `spell_panel` | NO | sparse viewport plus colorful/right-column title-menu art | `307323fbc1f7` |
| 5 | `verification-screens/pass112-n2-route-probe/image0005-raw.png` | `entrance_menu` | `dungeon_gameplay` | NO | dungeon entrance/menu controls still occupy the right column | `074422f14319` |
| 6 | `verification-screens/pass112-n2-route-probe/image0006-raw.png` | `entrance_menu` | `inventory` | NO | dungeon entrance/menu controls still occupy the right column | `880d853d1bbe` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass112-n2-route-probe/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0000 | 0.0000 | 1 | 0.00 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0000 | 0.0000 | 1 | 0.00 |
| inventory_extent | 0.0000 | 0.0000 | 1 | 0.00 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass112-n2-route-probe/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass112-n2-route-probe/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass112-n2-route-probe/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass112-n2-route-probe/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.5249 | 0.0471 | 9 | 72.96 |
| right_action | 0.9045 | 0.8902 | 12 | 53.17 |
| spell_area | 0.9269 | 0.9011 | 11 | 47.98 |
| right_column | 0.9376 | 0.8653 | 16 | 47.14 |
| inventory_extent | 0.4513 | 0.0431 | 8 | 53.24 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass112-n2-route-probe/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9116 | 0.0000 | 6 | 59.70 |
| right_action | 0.9231 | 0.8971 | 13 | 51.28 |
| spell_area | 0.9398 | 0.9140 | 11 | 46.84 |
| right_column | 0.9420 | 0.8691 | 16 | 46.79 |
| inventory_extent | 0.7627 | 0.0000 | 6 | 69.81 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |
