# Pass 119 — champion selection gate probe

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-144011-pass119-champion-selection-gate-probe`
- scope: deterministic post-`dungeon_gameplay` probes for original PC DM1 champion/party-control readiness after pass118 state gate.

## Scenario outcomes

### right_column_reenter_controls

- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- sha12: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### viewport_mirror_left_recruit

- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- sha12: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### viewport_mirror_mid_recruit

- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- sha12: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### viewport_mirror_right_recruit

- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- sha12: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### keyboard_confirm_recruit

- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- sha12: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

## Interpretation

This pass is evidence-only. A successful unblock requires any scenario to leave the repeated blank/no-party dungeon hash and/or make pass113 report party_control_ready=true after F1/F4 probes.

## Pass113 summary

### keyboard_confirm_recruit

- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat

### right_column_reenter_controls

- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat

### viewport_mirror_left_recruit

- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat

### viewport_mirror_mid_recruit

- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat

### viewport_mirror_right_recruit

- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat

