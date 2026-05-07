# pass307 original capture batch plan manifest

Metadata-only deterministic batch plan for pass304 PC34 original viewport capture commands.

- status: `passed`
- batches: `3`
- promotion shots: `['blocked_forward_south_wall', 'move_forward_west', 'start_south', 'turn_left_east', 'turn_right_west']`

| batch | promotion shots | command sha256 | guards |
|---|---|---|---|
| `batchA_start_right_forward` | `['start_south', 'turn_right_west', 'move_forward_west']` | `487c335f9eb7a83a6bec258e9d0ada5dd043e88d3c74bef01deb3dcc6f4aeaa3` | six-shot=yes, PC34=yes, route-events=yes, dosbox-xvfb=yes |
| `batchB_start_left` | `['start_south', 'turn_left_east']` | `5663d133d6f3353f1b893047737ec5de0f5ebe19bf80ae88d9b3c2214ab21d85` | six-shot=yes, PC34=yes, route-events=yes, dosbox-xvfb=yes |
| `batchC_start_blocked_forward` | `['start_south', 'blocked_forward_south_wall']` | `ea1128975716a71926226239597be778a1320d35c09791c984276640e195a440` | six-shot=yes, PC34=yes, route-events=yes, dosbox-xvfb=yes |

Remaining blockers:
- execute the three DOSBox/Xvfb capture batches and keep screenshots outside the repo unless explicitly promoted as small metadata/evidence
- verify captured 224x136 viewport crops against pass300/pass306 wall-region metadata
- do not promote padding shots or bitmap parity claims from this metadata-only manifest

Not claimed:
- original screenshot capture succeeded
- pixel parity
- bitmap bytes or screenshots in repository
