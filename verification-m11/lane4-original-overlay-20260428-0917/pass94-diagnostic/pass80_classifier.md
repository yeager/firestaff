# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- image0001-raw.png: classified graphics_320x200_unclassified, expected title_or_menu
- image0003-raw.png: classified entrance_menu, expected dungeon_gameplay
- image0004-raw.png: classified entrance_menu, expected dungeon_gameplay
- image0005-raw.png: classified wall_closeup, expected dungeon_gameplay
- image0006-raw.png: classified wall_closeup, expected dungeon_gameplay
- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0001-raw.png` | `graphics_320x200_unclassified` | `title_or_menu` | NO | 320x200 graphics frame, but layout heuristics did not match a known class | `71607bffd8c1` |
| 2 | `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0002-raw.png` | `entrance_menu` | `entrance_menu` | yes | dungeon entrance/menu controls still occupy the right column | `9f95e1d8fae6` |
| 3 | `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0003-raw.png` | `entrance_menu` | `dungeon_gameplay` | NO | dungeon entrance/menu controls still occupy the right column | `17bd7e878157` |
| 4 | `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0004-raw.png` | `entrance_menu` | `dungeon_gameplay` | NO | dungeon entrance/menu controls still occupy the right column | `17bd7e878157` |
| 5 | `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0005-raw.png` | `wall_closeup` | `dungeon_gameplay` | NO | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `355a191cd07b` |
| 6 | `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0006-raw.png` | `wall_closeup` | `dungeon_gameplay` | NO | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `48ed3743ab6a` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0000 | 0.0000 | 1 | 0.00 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0000 | 0.0000 | 1 | 0.00 |
| inventory_extent | 0.0000 | 0.0000 | 1 | 0.00 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.5769 | 0.0404 | 9 | 74.68 |
| right_action | 0.9001 | 0.8817 | 12 | 53.84 |
| spell_area | 0.9361 | 0.9172 | 11 | 46.39 |
| right_column | 0.9365 | 0.8670 | 16 | 47.27 |
| inventory_extent | 0.4917 | 0.0384 | 8 | 54.53 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
