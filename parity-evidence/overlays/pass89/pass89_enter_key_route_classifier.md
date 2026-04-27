# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass89-subagent-enter-key-route`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- image0001-raw.png: classified graphics_320x200_unclassified, expected title_or_menu
- image0002-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0003-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0004-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay
- image0005-raw.png: classified graphics_320x200_unclassified, expected spell_panel
- image0006-raw.png: classified wall_closeup, expected inventory
- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass89-subagent-enter-key-route/image0001-raw.png` | `graphics_320x200_unclassified` | `title_or_menu` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `c79222a6c543` |
| 2 | `verification-screens/pass89-subagent-enter-key-route/image0002-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `ea13da01ddfc` |
| 3 | `verification-screens/pass89-subagent-enter-key-route/image0003-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `5ae7199a3df0` |
| 4 | `verification-screens/pass89-subagent-enter-key-route/image0004-raw.png` | `graphics_320x200_unclassified` | `dungeon_gameplay` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `5ae7199a3df0` |
| 5 | `verification-screens/pass89-subagent-enter-key-route/image0005-raw.png` | `graphics_320x200_unclassified` | `spell_panel` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `5ae7199a3df0` |
| 6 | `verification-screens/pass89-subagent-enter-key-route/image0006-raw.png` | `wall_closeup` | `inventory` | NO | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `dcfdff6845bf` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass89-subagent-enter-key-route/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0000 | 0.0000 | 1 | 0.00 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0000 | 0.0000 | 1 | 0.00 |
| inventory_extent | 0.0000 | 0.0000 | 1 | 0.00 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass89-subagent-enter-key-route/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.6373 | 0.0328 | 8 | 76.19 |
| right_action | 0.9006 | 0.8805 | 12 | 53.79 |
| spell_area | 0.9674 | 0.9343 | 12 | 41.34 |
| right_column | 0.9383 | 0.8685 | 16 | 47.02 |
| inventory_extent | 0.5311 | 0.0279 | 8 | 58.84 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass89-subagent-enter-key-route/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass89-subagent-enter-key-route/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass89-subagent-enter-key-route/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass89-subagent-enter-key-route/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
