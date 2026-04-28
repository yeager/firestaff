# Pass 116 — original input/program matrix

- date: 2026-04-28T13:33:07+00:00
- host: firestaff-worker
- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-133035-original-input-program-matrix`
- scope: diagnose pass115 no-movement/no-party-control blocker by varying original program invocation and input family. Evidence only; no pixel parity claim.

## Attempts
### dm_flags_grid

- program=DM -vv -sn -pk skip=1
- route:
  wait:7000 enter wait:1200 click:276,140 wait:4500 shot:start click:247,135 wait:1100 shot:left_top click:276,126 wait:1100 shot:mid_top click:305,135 wait:1100 shot:right_top click:247,157 wait:1100 shot:left_bottom click:276,157 wait:1100 shot:mid_bottom
- party control ready: `false`
- direct-start/no-party signature: `true`
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat
- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- shas: `48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### dm_selector_grid

- program=DM skip=0
- route:
  wait:7000 enter wait:1200 click:276,140 wait:4500 shot:start click:247,135 wait:1100 shot:left_top click:276,126 wait:1100 shot:mid_top click:305,135 wait:1100 shot:right_top click:247,157 wait:1100 shot:left_bottom click:276,157 wait:1100 shot:mid_bottom
- party control ready: `false`
- direct-start/no-party signature: `false`
- classes: `entrance_menu, entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 4}`
- shas: `1339aaf0473c, 17bd7e878157, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a, 48ed3743ab6a`

### dm_vga_fkeys

- program=DM VGA skip=1
- route:
  wait:7000 enter wait:1200 click:276,140 wait:4500 shot:start f1 wait:1100 shot:f1 f2 wait:1100 shot:f2 f3 wait:1100 shot:f3 f4 wait:1100 shot:f4 space wait:1100 shot:space
- party control ready: `false`
- direct-start/no-party signature: `false`
- classes: `non_graphics_blocker, non_graphics_blocker, non_graphics_blocker, non_graphics_blocker, non_graphics_blocker, non_graphics_blocker`
- duplicate hashes: `{'e8f15fd9977a65523ee17a04cabbefa0064c9c690f317339f1003fd4e0a3e502': 3, 'b39958fc85bc31f7f0adec7b9434a346e0fdf6cef8330c2298274dc1c91c2f67': 2}`
- shas: `e5c3caeb8406, e8f15fd9977a, b39958fc85bc, e8f15fd9977a, e8f15fd9977a, b39958fc85bc`

### dm_vga_grid

- program=DM VGA skip=1
- route:
  wait:7000 enter wait:1200 click:276,140 wait:4500 shot:start click:247,135 wait:1100 shot:left_top click:276,126 wait:1100 shot:mid_top click:305,135 wait:1100 shot:right_top click:247,157 wait:1100 shot:left_bottom click:276,157 wait:1100 shot:mid_bottom
- party control ready: `false`
- direct-start/no-party signature: `false`
- classes: `non_graphics_blocker, non_graphics_blocker, non_graphics_blocker, non_graphics_blocker, non_graphics_blocker, non_graphics_blocker`
- duplicate hashes: `{'e5c3caeb8406c54a69b245ed0f84dcdb130e2cda68cc8daae98b6ac249533b32': 3, '82d0a42d7a05ed137d919c07deb65931bb0a08c0b49234f51b173f81774f347d': 3}`
- shas: `e5c3caeb8406, 82d0a42d7a05, 82d0a42d7a05, e5c3caeb8406, 82d0a42d7a05, e5c3caeb8406`

