# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass93-subagent-key-explore`
- capture count: 6
- pass: `True`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Warnings

- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass93-subagent-key-explore/image0001-raw.png` | `graphics_320x200_unclassified` | `` |  | 320x200 graphics frame, but layout heuristics did not match a known class | `c79222a6c543` |
| 2 | `verification-screens/pass93-subagent-key-explore/image0002-raw.png` | `graphics_320x200_unclassified` | `` |  | 320x200 graphics frame, but layout heuristics did not match a known class | `b6398313372c` |
| 3 | `verification-screens/pass93-subagent-key-explore/image0003-raw.png` | `graphics_320x200_unclassified` | `` |  | 320x200 graphics frame, but layout heuristics did not match a known class | `12d500d7b8d6` |
| 4 | `verification-screens/pass93-subagent-key-explore/image0004-raw.png` | `graphics_320x200_unclassified` | `` |  | 320x200 graphics frame, but layout heuristics did not match a known class | `5ae7199a3df0` |
| 5 | `verification-screens/pass93-subagent-key-explore/image0005-raw.png` | `graphics_320x200_unclassified` | `` |  | 320x200 graphics frame, but layout heuristics did not match a known class | `5ae7199a3df0` |
| 6 | `verification-screens/pass93-subagent-key-explore/image0006-raw.png` | `wall_closeup` | `` |  | flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence | `dcfdff6845bf` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass93-subagent-key-explore/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.0000 | 0.0000 | 1 | 0.00 |
| right_action | 0.0000 | 0.0000 | 1 | 0.00 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0000 | 0.0000 | 1 | 0.00 |
| inventory_extent | 0.0000 | 0.0000 | 1 | 0.00 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass93-subagent-key-explore/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.4458 | 0.0549 | 9 | 68.43 |
| right_action | 0.9132 | 0.8920 | 13 | 52.15 |
| spell_area | 0.9205 | 0.8920 | 11 | 49.02 |
| right_column | 0.9299 | 0.8635 | 16 | 48.21 |
| inventory_extent | 0.3908 | 0.0479 | 8 | 50.38 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass93-subagent-key-explore/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.8871 | 0.0022 | 8 | 63.59 |
| right_action | 0.9126 | 0.8922 | 13 | 52.40 |
| spell_area | 0.9297 | 0.9025 | 11 | 48.00 |
| right_column | 0.9372 | 0.8692 | 16 | 47.33 |
| inventory_extent | 0.7270 | 0.0059 | 8 | 70.16 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass93-subagent-key-explore/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass93-subagent-key-explore/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass93-subagent-key-explore/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
