# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass112-n2-stable-hud-route`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- image0003-raw.png: classified wall_closeup, expected dungeon_gameplay
- image0004-raw.png: classified wall_closeup, expected spell_panel
- image0005-raw.png: classified wall_closeup, expected dungeon_gameplay
- image0006-raw.png: classified wall_closeup, expected inventory
- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass112-n2-stable-hud-route/image0001-raw.png` | `dungeon_gameplay` | `dungeon_gameplay` | yes | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 2 | `verification-screens/pass112-n2-stable-hud-route/image0002-raw.png` | `dungeon_gameplay` | `dungeon_gameplay` | yes | viewport content with mostly dark in-game right column | `47d61e2ae941` |
| 3 | `verification-screens/pass112-n2-stable-hud-route/image0003-raw.png` | `wall_closeup` | `dungeon_gameplay` | NO | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `ee7741746ea9` |
| 4 | `verification-screens/pass112-n2-stable-hud-route/image0004-raw.png` | `wall_closeup` | `spell_panel` | NO | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `ee7741746ea9` |
| 5 | `verification-screens/pass112-n2-stable-hud-route/image0005-raw.png` | `wall_closeup` | `dungeon_gameplay` | NO | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `ee7741746ea9` |
| 6 | `verification-screens/pass112-n2-stable-hud-route/image0006-raw.png` | `wall_closeup` | `inventory` | NO | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `ee7741746ea9` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass112-n2-stable-hud-route/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass112-n2-stable-hud-route/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9169 | 0.0000 | 6 | 58.13 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7673 | 0.0000 | 6 | 68.02 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass112-n2-stable-hud-route/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9969 | 0.0000 | 6 | 42.00 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.9937 | 0.0000 | 6 | 41.58 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass112-n2-stable-hud-route/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9969 | 0.0000 | 6 | 42.00 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.9937 | 0.0000 | 6 | 41.58 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass112-n2-stable-hud-route/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9969 | 0.0000 | 6 | 42.00 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.9937 | 0.0000 | 6 | 41.58 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass112-n2-stable-hud-route/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9969 | 0.0000 | 6 | 42.00 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.9937 | 0.0000 | 6 | 41.58 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
