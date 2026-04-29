# Lane HUD/status follow-up — DM1 V1 champion top row/status panel

Run: `verification-m11/lane2-hud-status-followup-20260428T0710Z` on N2 (`<repo>`).
Timestamp: 2026-04-28T07:10Z–2026-04-28T07:12Z.

## Commands

```sh
./run_firestaff_m11_pass41_status_box_stride_probe.sh "$RUN/pass41-status-box-stride"
./run_firestaff_m11_pass42_chrome_reduction_probe.sh "$RUN/pass42-chrome-reduction"
./run_firestaff_m11_pass43_bar_graph_probe.sh "$RUN/pass43-bar-graphs"
./run_firestaff_m11_pass44_spell_label_probe.sh "$RUN/pass44-spell-labels" "$HOME/.firestaff/data/GRAPHICS.DAT"
./run_firestaff_m10_champion_lifecycle_probe.sh "$HOME/.firestaff/data/DUNGEON.DAT" "$RUN/champion-lifecycle"
FIRESTAFF_DATA="$HOME/.firestaff/data" ./run_firestaff_m11_game_view_probe.sh
FIRESTAFF_DATA="$HOME/.firestaff/data" ./run_firestaff_m11_ingame_capture_smoke.sh
```

## Results

- PASS — pass41 status box stride/origin: 24/24 invariants.
- PASS — pass42 chrome/message reroute and C015 V1 message area: 32/32 invariants.
- PASS — pass43 HP/stamina/mana vertical bars and champion color table: 10/10 invariants.
- PASS — pass44 C013 right-column spell area + C011 selected/available rune cells: 8/8 invariants.
- PASS — champion lifecycle probe completed.
- PASS — M11 game-view probe: 578/578 invariants, including V1 champion icons, action-hand cells, status slot/shield frame graphics, bar colors, condition/damage graphics, spell/action/right-column zone geometry, and message area suppression of telemetry.
- PASS — in-game capture smoke: 6 screenshots.

## Evidence files

- `output.log` — combined command log.
- `pass41-status-box-stride/pass41_status_box_stride_probe.log`
- `pass42-chrome-reduction/pass42_chrome_reduction_probe.log`
- `pass43-bar-graphs/pass43_bar_graph_probe.log`
- `pass43-bar-graphs/pass43_party_bar_graphs.pgm`
- `pass44-spell-labels/pass44_spell_label_probe.log`
- `pass44-spell-labels/pass44_spell_label_cells.pgm`
- `champion-lifecycle/champion_lifecycle_probe.md`
- `champion-lifecycle/champion_lifecycle_invariants.md`

No code changes were needed for this lane.
