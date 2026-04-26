# Pass 83 — champion HUD source-zone overlay

This pass adds visual evidence for the top-row V1 champion HUD. It overlays and crops the DM1 PC 3.4 layout-696 champion status zones already asserted by the M11 game-view probe.

Honesty: these files are evidence aids only. They do not claim pixel-perfect parity with original runtime screenshots.

- stats: `parity-evidence/overlays/pass83/pass83_champion_hud_zone_overlay_stats.json`
- scenes: 3
- pass: `True`

## Generated overlays

- `party_hud_with_champions`: `parity-evidence/overlays/pass83/party_hud_with_champions_champion_hud_zones_overlay.png` from `verification-screens/07_party_hud_with_champions.png`
- `spell_panel_with_champions`: `parity-evidence/overlays/pass83/spell_panel_with_champions_champion_hud_zones_overlay.png` from `verification-screens/08_spell_panel_with_champions.png`
- `four_champion_priority`: `parity-evidence/overlays/pass83/four_champion_priority_champion_hud_zones_overlay.png` from `verification-screens/pass81-champion-hud-priority/party_hud_four_champions_vga.png`

## Source zones

| zone | xywh | source reference |
|------|------|------------------|
| `C151_status_box_slot0` | `[12, 0, 67, 29]` | layout-696 C151..C154: 67x29 champion status boxes; DEFS.H C69 spacing |
| `C159_name_clear_slot0` | `[12, 0, 43, 7]` | layout-696 C159..C162: F0292 clears champion-name field before centered text |
| `C163_name_text_slot0` | `[13, 0, 42, 7]` | layout-696 C163..C166: clipped centered champion-name text zone |
| `C211_ready_hand_slot0` | `[16, 10, 16, 16]` | layout-696 C211/C213/C215/C217 ready-hand icon parent |
| `C212_action_hand_slot0` | `[36, 10, 16, 16]` | layout-696 C212/C214/C216/C218 action-hand icon parent |
| `C195_hp_bar_slot0` | `[58, 4, 4, 25]` | layout-696 C195..C198: F0287 HP vertical bar container |
| `C199_stamina_bar_slot0` | `[65, 4, 4, 25]` | layout-696 C199..C202: F0287 stamina vertical bar container |
| `C203_mana_bar_slot0` | `[72, 4, 4, 25]` | layout-696 C203..C206: F0287 mana vertical bar container |
| `C152_status_box_slot1` | `[81, 0, 67, 29]` | layout-696 C151..C154: 67x29 champion status boxes; DEFS.H C69 spacing |
| `C160_name_clear_slot1` | `[81, 0, 43, 7]` | layout-696 C159..C162: F0292 clears champion-name field before centered text |
| `C164_name_text_slot1` | `[82, 0, 42, 7]` | layout-696 C163..C166: clipped centered champion-name text zone |
| `C213_ready_hand_slot1` | `[85, 10, 16, 16]` | layout-696 C211/C213/C215/C217 ready-hand icon parent |
| `C214_action_hand_slot1` | `[105, 10, 16, 16]` | layout-696 C212/C214/C216/C218 action-hand icon parent |
| `C196_hp_bar_slot1` | `[127, 4, 4, 25]` | layout-696 C195..C198: F0287 HP vertical bar container |
| `C200_stamina_bar_slot1` | `[134, 4, 4, 25]` | layout-696 C199..C202: F0287 stamina vertical bar container |
| `C204_mana_bar_slot1` | `[141, 4, 4, 25]` | layout-696 C203..C206: F0287 mana vertical bar container |
| `C153_status_box_slot2` | `[150, 0, 67, 29]` | layout-696 C151..C154: 67x29 champion status boxes; DEFS.H C69 spacing |
| `C161_name_clear_slot2` | `[150, 0, 43, 7]` | layout-696 C159..C162: F0292 clears champion-name field before centered text |
| `C165_name_text_slot2` | `[151, 0, 42, 7]` | layout-696 C163..C166: clipped centered champion-name text zone |
| `C215_ready_hand_slot2` | `[154, 10, 16, 16]` | layout-696 C211/C213/C215/C217 ready-hand icon parent |
| `C216_action_hand_slot2` | `[174, 10, 16, 16]` | layout-696 C212/C214/C216/C218 action-hand icon parent |
| `C197_hp_bar_slot2` | `[196, 4, 4, 25]` | layout-696 C195..C198: F0287 HP vertical bar container |
| `C201_stamina_bar_slot2` | `[203, 4, 4, 25]` | layout-696 C199..C202: F0287 stamina vertical bar container |
| `C205_mana_bar_slot2` | `[210, 4, 4, 25]` | layout-696 C203..C206: F0287 mana vertical bar container |
| `C154_status_box_slot3` | `[219, 0, 67, 29]` | layout-696 C151..C154: 67x29 champion status boxes; DEFS.H C69 spacing |
| `C162_name_clear_slot3` | `[219, 0, 43, 7]` | layout-696 C159..C162: F0292 clears champion-name field before centered text |
| `C166_name_text_slot3` | `[220, 0, 42, 7]` | layout-696 C163..C166: clipped centered champion-name text zone |
| `C217_ready_hand_slot3` | `[223, 10, 16, 16]` | layout-696 C211/C213/C215/C217 ready-hand icon parent |
| `C218_action_hand_slot3` | `[243, 10, 16, 16]` | layout-696 C212/C214/C216/C218 action-hand icon parent |
| `C198_hp_bar_slot3` | `[265, 4, 4, 25]` | layout-696 C195..C198: F0287 HP vertical bar container |
| `C202_stamina_bar_slot3` | `[272, 4, 4, 25]` | layout-696 C199..C202: F0287 stamina vertical bar container |
| `C206_mana_bar_slot3` | `[279, 4, 4, 25]` | layout-696 C203..C206: F0287 mana vertical bar container |
