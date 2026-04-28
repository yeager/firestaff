# Lane 2 HUD/status panel follow-up
2026-04-28T07:01:46Z; pass44/right-column rerun 2026-04-28T07:05Z on N2

## Commands
./run_firestaff_m11_pass41_status_box_stride_probe.sh "$RUN/pass41-status-box-stride"
./run_firestaff_m11_pass42_chrome_reduction_probe.sh "$RUN/pass42-chrome-reduction"
./run_firestaff_m11_pass43_bar_graph_probe.sh "$RUN/pass43-bar-graphs"
./run_firestaff_m11_pass44_spell_label_probe.sh "$RUN/pass44-spell-labels" "$HOME/.firestaff/data/GRAPHICS.DAT"
./run_firestaff_m10_champion_lifecycle_probe.sh "$HOME/.firestaff/data/DUNGEON.DAT" "$RUN/champion-lifecycle"
./run_firestaff_memory_graphics_dat_original_main_loop_command_probe.sh "$HOME/.firestaff/data/GRAPHICS.DAT"
./run_firestaff_memory_graphics_dat_original_main_loop_command_queue_probe.sh "$HOME/.firestaff/data/GRAPHICS.DAT"
./run_firestaff_memory_graphics_dat_original_main_loop_command_loop_probe.sh "$HOME/.firestaff/data/GRAPHICS.DAT" 128 3
./run_firestaff_memory_graphics_dat_original_input_command_queue_probe.sh "$HOME/.firestaff/data/GRAPHICS.DAT"
FIRESTAFF_DATA="$HOME/.firestaff/data" ./run_firestaff_m11_game_view_probe.sh
FIRESTAFF_DATA="$HOME/.firestaff/data" ./run_firestaff_m11_ingame_capture_smoke.sh

## Results
# summary: 24/24 invariants passed
# summary: 32/32 invariants passed
# summary: 10/10 invariants passed
# summary: 8/8 invariants passed
M11 game-view probe: # summary: 578/578 invariants passed
In-game capture smoke PASS: 6 screenshots
Command routing probes: ok (`commandHandled=1`, `handledCommandCount=3`, `handledTickCommandCount=2`)
Champion lifecycle probe complete

## HUD/status surfaces covered
- champion status box stride/origin: pass41 24/24
- chrome/message reroute and C015 V1 message area: pass42 32/32
- HP/stamina/mana vertical bars and champion color table: pass43 10/10
- C013 right-column spell area + C011 selected/available rune cells: pass44 8/8
- command/input routing through original main-loop/typed queues: ok
- full M11 game-view gate, including action/spell/status zone invariants: 578/578
- pass43 screenshot: pass43-bar-graphs/pass43_party_bar_graphs.pgm
- pass44 screenshot: pass44-spell-labels/pass44_spell_label_cells.pgm
