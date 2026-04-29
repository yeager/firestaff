# Pass 117 — original keyboard movement calibration

- date: 2026-04-28T13:55:00Z
- host: firestaff-worker / N2
- run base: `<N2_RUNS>/20260428-1550-pass117-keyboard-movement-calibration`
- scope: one focused follow-up to pass116. Pass116 showed the original PC DM1 direct-start route can reach a dungeon frame with visible cyan movement controls, but mouse clicks in the right-control grid did not change frames and no party/HUD state was proven. This pass tests the narrower next blocker: whether serialized keyboard arrows advance the original runtime differently from the pass116 mouse grid.

## Source-backed rationale

ReDMCSB BUG0_73 documents an original-engine input queue issue where a mouse click may be ignored when keyboard/mouse commands collide. The local script also preserves this warning for original-route captures. Pass116 used mouse click grids after a keyboard `enter`; therefore pass117 intentionally used a keyboard-only arrow probe after startup, with 1200 ms waits between commands, to avoid mixed mouse/keyboard loss as the first-order variable.

Relevant source note: `<N2_REDMCSB_SOURCE>/dmweb.free.fr_Stuff_ReDMCSB_Documentation_BugsAndChanges.htm.html#BUG0_73` says: "A mouse click may be ignored" and describes mouse commands being overwritten in the command queue.

## Attempt: keyboard_arrows

- program: `DM -vv -sn -pk`
- stage: `<N2_ORIGINAL_GAMES>/DM/_extracted/dm-pc34/DungeonMasterPC34`
- route: `wait:7000 enter wait:1200 shot:start up wait:1200 shot:key_up right wait:1200 shot:key_right left wait:1200 shot:key_left down wait:1200 shot:key_down up wait:1200 shot:key_up2`
- run log: `<N2_RUNS>/20260428-1550-pass117-keyboard-movement-calibration/keyboard_arrows.run.log`
- classifier: `<N2_RUNS>/20260428-1550-pass117-keyboard-movement-calibration/keyboard_arrows/pass80_original_frame_classifier.{json,md}`
- party-state probe: `<N2_RUNS>/20260428-1550-pass117-keyboard-movement-calibration/keyboard_arrows/pass113.{json,md}`

### Classifier outcome

| # | route label | class | sha256 prefix |
|---|-------------|-------|---------------|
| 1 | `start` | `entrance_menu` | `9f95e1d8fae6` |
| 2 | `key_up` | `entrance_menu` | `17bd7e878157` |
| 3 | `key_right` | `entrance_menu` | `17bd7e878157` |
| 4 | `key_left` | `dungeon_gameplay` | `48ed3743ab6a` |
| 5 | `key_down` | `dungeon_gameplay` | `48ed3743ab6a` |
| 6 | `key_up2` | `dungeon_gameplay` | `48ed3743ab6a` |

Duplicate raw frames remained:

```json
{
  "17bd7e87815750b45e742964ffe93e0312d9bbdc45dd8e7358be0a069a6db1b8": 2,
  "48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397": 3
}
```

Party-state probe result:

```json
{
  "party_control_ready": false,
  "direct_start_no_party_signature": false,
  "classes": ["entrance_menu", "entrance_menu", "entrance_menu", "dungeon_gameplay", "dungeon_gameplay", "dungeon_gameplay"],
  "problems": [
    "image0001-raw.png: classified entrance_menu, expected dungeon_gameplay",
    "image0002-raw.png: classified entrance_menu, expected dungeon_gameplay",
    "image0003-raw.png: classified entrance_menu, expected dungeon_gameplay",
    "duplicate raw frames detected: 2 unique sha256 value(s) repeat",
    "party-control state not proven: capture lacks a clean six-frame spell/inventory/HUD-ready sequence"
  ]
}
```

## Conclusion

Pass117 does **not** solve the original HUD/viewport capture blocker. It adds one useful constraint: keyboard-only arrow input is not a direct replacement for the pass116 mouse grid. The first arrows act while the original entrance menu is still present, and once dungeon gameplay appears the later keyboard frames again repeat the pass116 no-party dungeon hash (`48ed3743ab6a`).

Most likely remaining blocker: the capture route still lacks a validated original PC DM1 startup/entrance selection sequence that reliably transitions from the entrance menu into a party-control-ready state. The next tool/evidence missing is a deterministic original-route driver that can wait on/frame-classify the entrance-menu disappearance (or click the actual ENTER menu item) before sending movement/recruitment commands; fixed sleeps plus raw arrows/clicks are insufficient.
