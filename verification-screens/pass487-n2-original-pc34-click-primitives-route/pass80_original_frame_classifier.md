# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass487-n2-original-pc34-click-primitives-route`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0001-raw.png` | `entrance_menu` | `` |  | dungeon entrance/menu controls still occupy the right column | `17bd7e878157` |
| 2 | `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0002-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 3 | `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0003-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 4 | `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0004-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 5 | `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0005-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 6 | `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0006-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `48ed3743ab6a` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass487-n2-original-pc34-click-primitives-route/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
