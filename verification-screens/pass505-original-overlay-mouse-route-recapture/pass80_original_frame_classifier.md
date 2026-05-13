# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass505-original-overlay-mouse-route-recapture`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- image0001-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0002-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0003-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0004-raw.png: classified graphics_320x200_unclassified, expected spell_panel
- image0005-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0006-raw.png: classified graphics_320x200_unclassified, expected inventory
- duplicate raw frames detected: 2 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass505-original-overlay-mouse-route-recapture/image0001-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `a17790109e74` |
| 2 | `verification-screens/pass505-original-overlay-mouse-route-recapture/image0002-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `b1cefb2478a8` |
| 3 | `verification-screens/pass505-original-overlay-mouse-route-recapture/image0003-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `a17790109e74` |
| 4 | `verification-screens/pass505-original-overlay-mouse-route-recapture/image0004-raw.png` | `graphics_320x200_unclassified` | `spell_panel` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `b1cefb2478a8` |
| 5 | `verification-screens/pass505-original-overlay-mouse-route-recapture/image0005-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `a17790109e74` |
| 6 | `verification-screens/pass505-original-overlay-mouse-route-recapture/image0006-raw.png` | `graphics_320x200_unclassified` | `inventory` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `b1cefb2478a8` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass505-original-overlay-mouse-route-recapture/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0461 | 0.0000 | 2 | 35.64 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0198 | 0.0000 | 2 | 23.71 |
| inventory_extent | 0.0095 | 0.0000 | 2 | 16.50 |
| title_top | 0.1840 | 0.0000 | 2 | 65.87 |

### `verification-screens/pass505-original-overlay-mouse-route-recapture/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0461 | 0.0000 | 2 | 35.64 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0198 | 0.0000 | 2 | 23.71 |
| inventory_extent | 0.0095 | 0.0000 | 2 | 16.50 |
| title_top | 0.1840 | 0.0000 | 2 | 65.87 |

### `verification-screens/pass505-original-overlay-mouse-route-recapture/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0461 | 0.0000 | 2 | 35.64 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0198 | 0.0000 | 2 | 23.71 |
| inventory_extent | 0.0095 | 0.0000 | 2 | 16.50 |
| title_top | 0.1840 | 0.0000 | 2 | 65.87 |

### `verification-screens/pass505-original-overlay-mouse-route-recapture/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0461 | 0.0000 | 2 | 35.64 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0198 | 0.0000 | 2 | 23.71 |
| inventory_extent | 0.0095 | 0.0000 | 2 | 16.50 |
| title_top | 0.1840 | 0.0000 | 2 | 65.87 |

### `verification-screens/pass505-original-overlay-mouse-route-recapture/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0461 | 0.0000 | 2 | 35.64 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0198 | 0.0000 | 2 | 23.71 |
| inventory_extent | 0.0095 | 0.0000 | 2 | 16.50 |
| title_top | 0.1840 | 0.0000 | 2 | 65.87 |

### `verification-screens/pass505-original-overlay-mouse-route-recapture/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0461 | 0.0000 | 2 | 35.64 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0198 | 0.0000 | 2 | 23.71 |
| inventory_extent | 0.0095 | 0.0000 | 2 | 16.50 |
| title_top | 0.1840 | 0.0000 | 2 | 65.87 |
