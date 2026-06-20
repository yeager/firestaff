# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass1055-dm1-original-closed-door-collision`
- capture count: 4
- pass: `True`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Warnings

- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass1055-dm1-original-closed-door-collision/image0001-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `40c678403d8f` |
| 2 | `verification-screens/pass1055-dm1-original-closed-door-collision/image0002-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `a0d3a9cdbddc` |
| 3 | `verification-screens/pass1055-dm1-original-closed-door-collision/image0003-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `a0d3a9cdbddc` |
| 4 | `verification-screens/pass1055-dm1-original-closed-door-collision/image0004-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `a0d3a9cdbddc` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass1055-dm1-original-closed-door-collision/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9169 | 0.0000 | 6 | 58.13 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7673 | 0.0000 | 6 | 68.02 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass1055-dm1-original-closed-door-collision/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9908 | 0.0078 | 8 | 43.51 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.9856 | 0.0174 | 8 | 44.80 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass1055-dm1-original-closed-door-collision/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9908 | 0.0078 | 8 | 43.51 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.9856 | 0.0174 | 8 | 44.80 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass1055-dm1-original-closed-door-collision/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9908 | 0.0078 | 8 | 43.51 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.9856 | 0.0174 | 8 | 44.80 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
