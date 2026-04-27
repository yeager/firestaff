# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass87-subagent-click-route`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- image0001-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0002-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0003-raw.png: classified title_or_menu, expected dungeon_gameplay
- image0004-raw.png: classified title_or_menu, expected spell_panel
- image0005-raw.png: classified title_or_menu, expected dungeon_gameplay
- image0006-raw.png: classified title_or_menu, expected inventory
- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass87-subagent-click-route/image0001-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `4454c84bcc58` |
| 2 | `verification-screens/pass87-subagent-click-route/image0002-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `34e78ea8facf` |
| 3 | `verification-screens/pass87-subagent-click-route/image0003-raw.png` | `title_or_menu` | `dungeon_gameplay` | NO | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 4 | `verification-screens/pass87-subagent-click-route/image0004-raw.png` | `title_or_menu` | `spell_panel` | NO | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 5 | `verification-screens/pass87-subagent-click-route/image0005-raw.png` | `title_or_menu` | `dungeon_gameplay` | NO | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 6 | `verification-screens/pass87-subagent-click-route/image0006-raw.png` | `title_or_menu` | `inventory` | NO | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass87-subagent-click-route/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.4136 | 15 | 77.47 |
| right_action | 1.0000 | 0.6825 | 12 | 82.56 |
| spell_area | 1.0000 | 0.2455 | 2 | 31.27 |
| right_column | 1.0000 | 0.1915 | 14 | 56.79 |
| inventory_extent | 1.0000 | 0.6422 | 15 | 72.71 |
| title_top | 1.0000 | 0.0418 | 12 | 37.98 |

### `verification-screens/pass87-subagent-click-route/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.1208 | 15 | 48.98 |
| right_action | 1.0000 | 0.1903 | 12 | 61.93 |
| spell_area | 1.0000 | 0.0708 | 2 | 18.64 |
| right_column | 1.0000 | 0.0532 | 14 | 32.01 |
| inventory_extent | 1.0000 | 0.1884 | 15 | 52.39 |
| title_top | 1.0000 | 0.0138 | 10 | 21.52 |

### `verification-screens/pass87-subagent-click-route/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass87-subagent-click-route/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass87-subagent-click-route/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass87-subagent-click-route/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |
