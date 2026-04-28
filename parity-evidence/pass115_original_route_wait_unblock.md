# Pass 115 — original route wait/movement unblock

- date: 2026-04-28T13:11:15+00:00
- host: firestaff-worker
- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-130807-original-route-wait-unblock`
- scope: follow-up to pass114. Adds the observed post-ENTER/menu transition wait, then tests movement/control routes. Route-unblock evidence only; not pixel parity.

## Attempts
### elija_click_probe

- route:
  wait:7000 enter wait:1200 click:276,140 wait:4200 click:276,140 wait:900 click:276,140 wait:900 click:276,140 wait:900 click:276,140 wait:900 click:276,140 wait:1200 shot:at_wall click:112,84 wait:1200 shot:wall_click click:160,84 wait:1200 shot:center_click click:276,126 wait:900 shot:panel_click click:276,150 wait:900 shot:panel_down click:276,140 wait:900 shot:panel_up
- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat
- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- shas: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### stable_forward_y126

- route:
  wait:7000 enter wait:1200 click:276,140 wait:4200 shot:start click:276,126 wait:900 shot:fwd1 click:276,126 wait:900 shot:fwd2 click:276,126 wait:900 shot:fwd3 click:276,126 wait:900 shot:fwd4 click:276,126 wait:900 shot:fwd5
- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat
- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- shas: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### stable_forward_y140

- route:
  wait:7000 enter wait:1200 click:276,140 wait:4200 shot:start click:276,140 wait:900 shot:fwd1 click:276,140 wait:900 shot:fwd2 click:276,140 wait:900 shot:fwd3 click:276,140 wait:900 shot:fwd4 click:276,140 wait:900 shot:fwd5
- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat
- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- shas: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### stable_kp8

- route:
  wait:7000 enter wait:1200 click:276,140 wait:4200 shot:start kp8 wait:900 shot:kp8_1 kp8 wait:900 shot:kp8_2 kp8 wait:900 shot:kp8_3 kp8 wait:900 shot:kp8_4 kp8 wait:900 shot:kp8_5
- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat
- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- shas: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### stable_up

- route:
  wait:7000 enter wait:1200 click:276,140 wait:4200 shot:start up wait:900 shot:up1 up wait:900 shot:up2 up wait:900 shot:up3 up wait:900 shot:up4 up wait:900 shot:up5
- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat
- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- shas: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

