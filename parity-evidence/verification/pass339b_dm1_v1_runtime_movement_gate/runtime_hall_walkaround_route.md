# DM1 V1 Hall walkaround runtime probe

Source lock: LOADSAVE.C:1940-1944 initial party position; COMMAND.C:2150-2156 turn/step dispatch; CLIKMENU.C:142-173 turn path; CLIKMENU.C:264-328 movement blockers; GAMELOOP.C:80-90 viewport redraw; DUNGEON.C:2608-2612 and MOVESENS.C:1501-1503 isolate champion portrait/candidate routes.

| step | result | tick | pos | dir | champions | candidate | front map | front element | front thing | mirror | action | outcome |
| --- | ---: | ---: | --- | --- | ---: | --- | --- | --- | ---: | ---: | --- | --- |
| start_hall_initial_south | 1 | 0 | 0,1,3 | 2/SOUTH | 0 | -1/0 | 1,4 | Wall | 0x0C10 | -1 | BOOT | GAME DATA LOADED |
| turn_left_east_view_changes | 1 | 1 | 0,1,3 | 1/EAST | 0 | -1/0 | 2,3 | Wall | 0xCC17 | -1 | TURN LEFT | FACING UPDATED |
| step_east_blocked_by_hall_side | 1 | 2 | 0,1,3 | 1/EAST | 0 | -1/0 | 2,3 | Wall | 0xCC17 | -1 | FORWARD | MOVEMENT BLOCKED |
| turn_left_north_front_mirror | 1 | 3 | 0,1,3 | 0/NORTH | 0 | -1/0 | 1,2 | Wall | 0x0801 | 1 | TURN LEFT | FACING UPDATED |
| step_north_mirror_wall_blocked | 1 | 4 | 0,1,3 | 0/NORTH | 0 | -1/0 | 1,2 | Wall | 0x0801 | 1 | FORWARD | MOVEMENT BLOCKED |
| turn_left_west_open_lane | 1 | 5 | 0,1,3 | 3/WEST | 0 | -1/0 | 0,3 | FakeWall | 0x0441 | -1 | TURN LEFT | FACING UPDATED |
| step_west_moves_in_hall | 1 | 6 | 0,0,3 | 3/WEST | 0 | -1/0 | -1,3 | OUT_OF_BOUNDS | 0xFFFE | -1 | FORWARD | PARTY MOVED |
| turn_right_north_from_west_cell | 1 | 7 | 0,0,3 | 0/NORTH | 0 | -1/0 | 0,2 | Wall | 0x006B | -1 | TURN RIGHT | FACING UPDATED |
| turn_right_east_front_second_mirror | 1 | 8 | 0,0,3 | 1/EAST | 0 | -1/0 | 1,3 | Stairs | 0x0802 | 2 | TURN RIGHT | FACING UPDATED |
