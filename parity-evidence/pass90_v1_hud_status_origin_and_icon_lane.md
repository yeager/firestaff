# Pass 90 — V1 HUD status-box origin and champion icon lane

Scope: DM1 PC 3.4 V1 original-faithful HUD/champion top row only.

## Change

The V1 champion status-box x origin now uses the recovered layout-696 source origin `0` instead of Firestaff's legacy 12 px party-panel inset.

Source anchors from `zones_h_reconstruction.json`:

- `C150` template: 67×29
- `C151..C154`: child offsets `0, 69, 138, 207`
- `C113..C116` champion icon zones: top-right cluster at x `281/301`

This yields V1 status boxes:

- slot 0: `0..66`
- slot 1: `69..135`
- slot 2: `138..204`
- slot 3: `207..273`

The C113/C116 champion icon lane begins at x=281, so the fourth status box no longer shares pixels with the icon lane.  V2 vertical-slice HUD assets keep the old x=12 origin.

## Files

- `m11_game_view.c`: adds `M11_V1_PARTY_PANEL_X = 0` and `m11_party_panel_x()`; V1 status/name/bar/hand zone helpers route through the source x origin while V2 keeps the legacy inset.
- `probes/m11/firestaff_m11_game_view_probe.c`: expected V1 status/name/bar/hand geometry updated to layout-696 x=0.
- `firestaff_m11_pass41_status_box_stride_probe.c`: stride probe now locks the source x origin/right edge.
- `firestaff_m11_pass43_bar_graph_probe.c`: bar graph probe follows the shifted C187/C195-derived positions.

## Gates

Run:

```sh
./run_firestaff_m11_pass41_status_box_stride_probe.sh
./run_firestaff_m11_pass43_bar_graph_probe.sh
./run_firestaff_m11_game_view_probe.sh
./run_firestaff_m11_phase_a_probe.sh
```
