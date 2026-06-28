# CSB V1 viewport/HUD pixel parity readiness

Status: `PASS_OPEN`
Broad parity: `OPEN_UNPAIRED`

This gate keeps the CSB viewport/HUD fixture shape ready for a later
paired original-vs-Firestaff hash pass. It deliberately does not run a
capture or promote parity while all future pairing fields are empty.

## Fixture Surface

| Fixture | State | Region | Geometry | Surface |
|---|---|---|---|---|
| `csb_v1_prison_entrance_viewport_full` | `csb_prison_entrance` | `viewport_full` | `0,0 320x200` | Viewport |
| `csb_v1_prison_entrance_viewport_center` | `csb_prison_entrance` | `viewport_center` | `48,28 224x136` | Viewport |
| `csb_v1_prison_entrance_status_bar` | `csb_prison_entrance` | `status_bar` | `0,0 320x28` | HUD |
| `csb_v1_prison_entrance_chrome_bottom` | `csb_prison_entrance` | `chrome_bottom` | `0,152 320x48` | HUD |
| `csb_v1_prison_entrance_panel_right` | `csb_prison_entrance` | `panel_right` | `240,28 80x172` | HUD |
| `csb_v1_prison_forward_viewport_full` | `csb_prison_forward` | `viewport_full` | `0,0 320x200` | Viewport |

## Promotion Rule

Promote CSB viewport/HUD pixel parity only after every fixture has paired original and Firestaff artifacts, SHA256 values for both, a diff artifact/hash, and explicit changed_pixels/mae/max_delta metrics.

## Non-claims

- no original CSB frame is captured by this gate
- no Firestaff runtime frame is captured by this gate
- no original-vs-Firestaff pixel parity is promoted
- broader CSB viewport/HUD parity remains OPEN_UNPAIRED until paired evidence lands
- no user-supplied game data is committed
