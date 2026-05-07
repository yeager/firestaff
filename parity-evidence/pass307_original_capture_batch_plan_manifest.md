# pass307 original capture batch plan manifest

Metadata-only deterministic batch plan for pass304 PC34 original viewport capture commands.

- status: `passed`
- batches: `3`
- promotion shots: `['blocked_forward_south_wall', 'move_forward_west', 'start_south', 'turn_left_east', 'turn_right_west']`

| batch | promotion shots | command sha256 | guards |
|---|---|---|---|
| `batchA_start_right_forward` | `['start_south', 'turn_right_west', 'move_forward_west']` | `7ba8b10b02d86a5553941a1104d9165d9b8a4c0f22ec14f1da33f46c4b786518` | six-shot=yes, PC34=yes, route-events=yes, dosbox-xvfb=yes |
| `batchB_start_left` | `['start_south', 'turn_left_east']` | `418628ade451411cccaec8e587f3bc5c73532cffad282c5aae0cfbea940eb1df` | six-shot=yes, PC34=yes, route-events=yes, dosbox-xvfb=yes |
| `batchC_start_blocked_forward` | `['start_south', 'blocked_forward_south_wall']` | `1c59966d32cbcec07986bc31788ce2a8447eb5cac051fb5ae6e169d0bb191551` | six-shot=yes, PC34=yes, route-events=yes, dosbox-xvfb=yes |

Remaining blockers:
- execute the three DOSBox/Xvfb capture batches and keep screenshots outside the repo unless explicitly promoted as small metadata/evidence
- verify captured 224x136 viewport crops against pass300/pass306 wall-region metadata
- do not promote padding shots or bitmap parity claims from this metadata-only manifest

Not claimed:
- original screenshot capture succeeded
- pixel parity
- bitmap bytes or screenshots in repository
