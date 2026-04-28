# Pass 113 — original party-state probe

This pass113 gate classifies whether an original DM1 PC 3.4 capture is actually usable for party/HUD, spell, or inventory comparison after direct-start/recruitment attempts.

- attempt dir: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-1344-party-state-followup/probe-kp8-to-elija`
- party control ready: `false`
- direct-start/no-party signature: `true`
- honesty: Party-state semantic probe only. It can block unsafe original captures; it does not claim pixel parity or champion identity.

## Problems

- duplicate raw frames detected: 1 unique sha256 value(s) repeat
- direct-start/no-party signature: dungeon viewport reached, but right-column control areas stay blank and movement/probe frames repeat

## Captures

| # | route label | classification | right-column nonblack | spell-area nonblack | sha256 |
|---|-------------|----------------|-----------------------|---------------------|--------|
| 1 | `start` | `dungeon_gameplay` | 0.0000 | 0.0000 | `1ee706538fb3` |
| 2 | `kp8_1` | `dungeon_gameplay` | 0.0626 | 0.0000 | `48ed3743ab6a` |
| 3 | `kp8_2` | `dungeon_gameplay` | 0.0626 | 0.0000 | `48ed3743ab6a` |
| 4 | `kp8_3` | `dungeon_gameplay` | 0.0626 | 0.0000 | `48ed3743ab6a` |
| 5 | `kp8_4` | `dungeon_gameplay` | 0.0626 | 0.0000 | `48ed3743ab6a` |
| 6 | `kp8_5` | `dungeon_gameplay` | 0.0626 | 0.0000 | `48ed3743ab6a` |
