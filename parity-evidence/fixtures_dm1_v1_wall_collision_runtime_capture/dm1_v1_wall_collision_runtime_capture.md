# DM1 V1 wall/collision runtime capture

Deterministic Firestaff runtime repro for wall/collision reports. Each row records exact map/x/y/direction, movement pipeline state, a full-frame PPM, and a source-geometry viewport crop PPM. This is capture readiness only; it does not claim original DOS pixel parity.

| label | action | result | map | x | y | dir | command | turn | step | blocked | dirty | screenshot | viewport crop |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| 01_start_south_1_3 | start | 1 | 0 | 1 | 3 | 2 | 0 | 0 | 0 | 0 | 0 | `01_start_south_1_3.ppm` | `01_start_south_1_3_viewport_224x136.ppm` |
| 02_turn_right_west_1_3 | turn_right | 1 | 0 | 1 | 3 | 3 | 2 | 1 | 0 | 0 | 1 | `02_turn_right_west_1_3.ppm` | `02_turn_right_west_1_3_viewport_224x136.ppm` |
| 03_blocked_west_wall_1_3 | forward_into_west_wall | 1 | 0 | 1 | 3 | 3 | 3 | 0 | 0 | 1 | 0 | `03_blocked_west_wall_1_3.ppm` | `03_blocked_west_wall_1_3_viewport_224x136.ppm` |
| 04_forward_south_1_4 | turn_left_then_forward_south | 1 | 0 | 1 | 4 | 2 | 3 | 0 | 1 | 0 | 1 | `04_forward_south_1_4.ppm` | `04_forward_south_1_4_viewport_224x136.ppm` |
