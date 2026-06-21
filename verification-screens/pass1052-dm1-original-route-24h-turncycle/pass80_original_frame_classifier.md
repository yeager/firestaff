# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass1052-dm1-original-route-24h-turncycle`
- capture count: 4
- pass: `True`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0001-raw.png` | `dungeon_gameplay` | `dungeon_gameplay` | yes | viewport content with mostly dark in-game right column | `40c678403d8f` |
| 2 | `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0002-raw.png` | `wall_closeup` | `wall_closeup` | yes | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `932d9d84e55f` |
| 3 | `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0003-raw.png` | `dungeon_gameplay` | `dungeon_gameplay` | yes | viewport content with mostly dark in-game right column | `966190b6ed4d` |
| 4 | `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0004-raw.png` | `wall_closeup` | `wall_closeup` | yes | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `278ba175878d` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9169 | 0.0000 | 6 | 58.13 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7673 | 0.0000 | 6 | 68.02 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.0000 | 5 | 27.99 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 1.0000 | 0.0000 | 5 | 27.17 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.8907 | 0.0000 | 6 | 62.41 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.6851 | 0.0000 | 6 | 76.84 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9998 | 0.0000 | 6 | 30.72 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 1.0000 | 0.0000 | 5 | 27.17 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
