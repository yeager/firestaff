# Pass 74 — original full-frame panel overlay measurements

Date: 2026-04-26

## Goal

Extend the pass-70 viewport-only comparison into full 320×200 frame measurements for the current action/spell/inventory evidence frames.

This is measurement-only. It does **not** claim semantic route parity or pixel parity.

## Tool

- `tools/pass74_fullscreen_panel_pair_compare.py`
- Stats: `parity-evidence/overlays/pass74/pass74_fullscreen_panel_compare_stats.json`
- Masks: `parity-evidence/overlays/pass74/*_full_frame_mask.png`

The tool pairs the six Firestaff full-frame captures with the six original raw DOSBox screenshots from pass 70 and measures these source-relevant regions:

| region | xywh |
| --- | --- |
| full_frame | `0,0,320,200` |
| viewport | `0,33,224,136` |
| action_area_C011 | `224,45,87,45` |
| spell_area_C013 | `224,90,87,25` |
| right_column_action_spell | `224,45,87,70` |
| message_area | `0,169,224,31` |
| inventory_panel_C101_extent | `80,53,144,73` |

## Gate command

```sh
python3 tools/pass74_fullscreen_panel_pair_compare.py
```

Result:

```text
pairs=6, problems=[]
```

## First measurements

| scene | full | viewport | action C011 | spell C013 | inventory panel extent |
| --- | ---: | ---: | ---: | ---: | ---: |
| 01 ingame_start | 82.8250% | 81.0826% | 89.8340% | 94.3908% | 62.5951% |
| 02 ingame_turn_right | 82.9281% | 81.2992% | 89.8340% | 94.3908% | 62.5951% |
| 03 ingame_move_forward | 76.7172% | 68.2511% | 89.8340% | 94.3908% | 46.5563% |
| 04 ingame_spell_panel | 64.4172% | 41.9249% | 92.5415% | 94.4368% | 35.5023% |
| 05 ingame_after_cast | 84.7844% | 78.1250% | 90.2427% | 98.1149% | 70.4148% |
| 06 ingame_inventory_panel | 94.9594% | 92.8407% | 98.1865% | 97.5172% | 87.0053% |

## Interpretation

The comparison path is now full-frame and region-aware, but the high deltas confirm the existing blocker: the original route and Firestaff deterministic fixture are not yet a semantically identical state. These numbers are useful as regression/evidence inputs, not as parity success criteria.
