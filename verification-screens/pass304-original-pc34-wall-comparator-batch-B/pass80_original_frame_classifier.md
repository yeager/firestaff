# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass304-original-pc34-wall-comparator-batch-B`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0001-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 2 | `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0002-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `fbeb1b82cd09` |
| 3 | `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0003-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `fbeb1b82cd09` |
| 4 | `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0004-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `fbeb1b82cd09` |
| 5 | `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0005-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `fbeb1b82cd09` |
| 6 | `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0006-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `fbeb1b82cd09` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.0000 | 5 | 27.99 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 1.0000 | 0.0000 | 4 | 27.04 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.0000 | 5 | 27.99 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 1.0000 | 0.0000 | 4 | 27.04 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.0000 | 5 | 27.99 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 1.0000 | 0.0000 | 4 | 27.04 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.0000 | 5 | 27.99 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 1.0000 | 0.0000 | 4 | 27.04 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass304-original-pc34-wall-comparator-batch-B/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 1.0000 | 0.0000 | 5 | 27.99 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 1.0000 | 0.0000 | 4 | 27.04 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
