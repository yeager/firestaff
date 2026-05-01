# Pass 127 turn viewport orientation probe

Source lock: ReDMCSB COMMAND.C:2150-2156 dispatches turn/step commands; CLIKMENU.C:156-173/237-347 sets StopWaitingForPlayerInput after successful turn/step; GAMELOOP.C:90 and DUNVIEW.C:8318-8616 redraw/present from current party direction/map coordinates.

| snapshot | tick | pos | dir | lane | map | square | element | firstThing | door | pit | teleporter |
| --- | ---: | --- | --- | ---: | --- | --- | --- | --- | ---: | ---: | ---: |
| start_south | 0 | 0,1,3 | 2/SOUTH | -1 | 2,4 | 0x68 | Stairs | 0x080B | 0 | 0 | 0 |
| start_south | 0 | 0,1,3 | 2/SOUTH | +0 | 1,4 | 0x00 | Wall | 0x0C10 | 0 | 0 | 0 |
| start_south | 0 | 0,1,3 | 2/SOUTH | +1 | 0,4 | 0x37 | Corridor | 0x0C40 | 0 | 0 | 0 |
| turn_right_west | 1 | 0,1,3 | 3/WEST | -1 | 0,4 | 0x37 | Corridor | 0x0C40 | 0 | 0 | 0 |
| turn_right_west | 1 | 0,1,3 | 3/WEST | +0 | 0,3 | 0xC6 | FakeWall | 0x0441 | 0 | 0 | 1 |
| turn_right_west | 1 | 0,1,3 | 3/WEST | +1 | 0,2 | 0x0A | Wall | 0x006B | 1 | 0 | 0 |
| move_forward_west | 2 | 0,0,3 | 3/WEST | -1 | -1,4 | 0x00 | OUT_OF_BOUNDS | 0xFFFE | 0 | 0 | 0 |
| move_forward_west | 2 | 0,0,3 | 3/WEST | +0 | -1,3 | 0x00 | OUT_OF_BOUNDS | 0xFFFE | 0 | 0 | 0 |
| move_forward_west | 2 | 0,0,3 | 3/WEST | +1 | -1,2 | 0x00 | OUT_OF_BOUNDS | 0xFFFE | 0 | 0 | 0 |
| turn_left_east | 1 | 0,1,3 | 1/EAST | -1 | 2,2 | 0x65 | Stairs | 0x0C16 | 0 | 0 | 0 |
| turn_left_east | 1 | 0,1,3 | 1/EAST | +0 | 2,3 | 0x00 | Wall | 0xCC17 | 0 | 0 | 0 |
| turn_left_east | 1 | 0,1,3 | 1/EAST | +1 | 2,4 | 0x68 | Stairs | 0x080B | 0 | 0 | 0 |
| blocked_forward_south_wall | 1 | 0,1,3 | 2/SOUTH | -1 | 2,4 | 0x68 | Stairs | 0x080B | 0 | 0 | 0 |
| blocked_forward_south_wall | 1 | 0,1,3 | 2/SOUTH | +0 | 1,4 | 0x00 | Wall | 0x0C10 | 0 | 0 | 0 |
| blocked_forward_south_wall | 1 | 0,1,3 | 2/SOUTH | +1 | 0,4 | 0x37 | Corridor | 0x0C40 | 0 | 0 | 0 |
