# DM1 V1 Hall walkaround runtime probe

Source lock: LOADSAVE.C:1940-1944 initial party position; COMMAND.C:2150-2156 turn/step dispatch; CLIKMENU.C:142-173 turn path; CLIKMENU.C:264-328 movement blockers; GAMELOOP.C:80-90 viewport redraw; DUNGEON.C:2608-2612 and MOVESENS.C:1501-1503 isolate champion portrait/candidate routes; REVIVE.C:272-276 appends candidate, 744-785 cancels/removes, 785-835 confirms/disables/reincarnates.

| step | result | tick | pos | dir | champions | candidate ord/index/panel | front map | front element | front thing | mirror | action | outcome |
| --- | ---: | ---: | --- | --- | ---: | --- | --- | --- | ---: | ---: | --- | --- |
| start_hall_initial_south | 1 | 0 | 0,1,3 | 2/SOUTH | 0 | -1/-1/0 | 1,4 | Corridor | 0x0C10 | -1 | BOOT | GAME DATA LOADED |
| turn_left_east_view_changes | 1 | 1 | 0,1,3 | 1/EAST | 0 | -1/-1/0 | 2,3 | Wall | 0xCC17 | -1 | TURN LEFT | FACING UPDATED |
| step_east_blocked_by_hall_side | 1 | 2 | 0,1,3 | 1/EAST | 0 | -1/-1/0 | 2,3 | Wall | 0xCC17 | -1 | FORWARD | MOVEMENT BLOCKED |
| turn_left_north_front_mirror | 1 | 3 | 0,1,3 | 0/NORTH | 0 | -1/-1/0 | 1,2 | Door | 0x0801 | 1 | TURN LEFT | FACING UPDATED |
| step_north_mirror_wall_blocked | 1 | 4 | 0,1,3 | 0/NORTH | 0 | -1/-1/0 | 1,2 | Door | 0x0801 | 1 | FORWARD | MOVEMENT BLOCKED |
| turn_right_south_corridor | 1 | 6 | 0,1,3 | 2/SOUTH | 0 | -1/-1/0 | 1,4 | Corridor | 0x0C10 | -1 | TURN RIGHT | FACING UPDATED |
| step_south_advances_corridor | 1 | 7 | 0,1,4 | 2/SOUTH | 0 | -1/-1/0 | 1,5 | Corridor | 0x0803 | 3 | FORWARD | PARTY MOVED |
| turn_around_face_north_from_corridor | 1 | 9 | 0,1,4 | 0/NORTH | 0 | -1/-1/0 | 1,3 | Teleporter | 0x0802 | 2 | TURN RIGHT | FACING UPDATED |
| corridor_front_second_mirror | 1 | 9 | 0,1,4 | 0/NORTH | 0 | -1/-1/0 | 1,3 | Teleporter | 0x0802 | 2 | TURN RIGHT | FACING UPDATED |
| select_second_mirror_candidate_appends_party | 1 | 9 | 0,1,4 | 0/NORTH | 1 | 2/0/1 | 1,3 | Teleporter | 0x0802 | 2 | MIRROR | RESURRECT OR REINCARNATE |
| cancel_candidate_removes_appended_party_member | 1 | 9 | 0,1,4 | 0/NORTH | 0 | -1/-1/0 | 1,3 | Teleporter | 0x0802 | 2 | MIRROR | CANCELLED |
| select_again_for_resurrect | 1 | 9 | 0,1,4 | 0/NORTH | 1 | 2/0/1 | 1,3 | Teleporter | 0x0802 | 2 | MIRROR | RESURRECT OR REINCARNATE |
| resurrect_keeps_candidate_and_disables_reselect | 1 | 9 | 0,1,4 | 0/NORTH | 1 | -1/-1/0 | 1,3 | Teleporter | 0xFFFE | -1 | MIRROR | RESURRECTED |
| resurrected_mirror_reselect_blocked | 0 | 9 | 0,1,4 | 0/NORTH | 1 | -1/-1/0 | 1,3 | Teleporter | 0xFFFE | -1 | MIRROR | RESURRECTED |
| reincarnate_candidate_halves_vitals | 1 | 7 | 0,1,4 | 0/NORTH | 1 | -1/-1/0 | 1,3 | Teleporter | 0xFFFE | -1 | MIRROR | REINCARNATED |
