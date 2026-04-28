# Lane 2 HUD/status panel follow-up
2026-04-28T07:01:46Z

## Commands
./run_firestaff_m11_pass41_status_box_stride_probe.sh "$RUN/pass41-status-box-stride"
./run_firestaff_m11_pass42_chrome_reduction_probe.sh "$RUN/pass42-chrome-reduction"
./run_firestaff_m11_pass43_bar_graph_probe.sh "$RUN/pass43-bar-graphs"

## Results
# summary: 24/24 invariants passed
# summary: 32/32 invariants passed
# summary: 10/10 invariants passed

## HUD/status surfaces covered
- champion status box stride/origin: pass41 24/24
- chrome/message reroute and C015 V1 message area: pass42 32/32
- HP/stamina/mana vertical bars and champion color table: pass43 10/10
- pass43 screenshot: pass43-bar-graphs/pass43_party_bar_graphs.pgm
