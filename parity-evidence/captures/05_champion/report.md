# DM1 V1 Original Capture Pair Report

Pair index: **05_champion**
Pair kind: **champion**
Firestaff paired capture: (none)
pass80 classifier verdict: ****

## Captures

### 05_champion_hud

- Path: `/tmp/dm1_original_capture/05_champion/05_champion_hud.png`
- SHA256: `9d314f20ebfc222d26d40fe8e4090d8b826ab6d109dcaa71b0f98a0668ff8b49`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame showing 4-champion party HUD at top of viewport

### 05_champion_hud_after

- Path: `/tmp/dm1_original_capture/05_champion/05_champion_hud_after.png`
- SHA256: `9d314f20ebfc222d26d40fe8e4090d8b826ab6d109dcaa71b0f98a0668ff8b49`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Second HUD capture to verify distinct frame

## Notes

- 05_champion_hud: 4-champion HUD with portraits + status boxes + bar graphs (HP/stamina/mana). ReDMCSB champion_panel_hud_pc34_compat.c pins the geometry (slot stride 69 px, bar 4x25). The full 320x200 capture includes the champion panel at y=0..64.

## Pass/Fail Verdict

**GAP_CLOSED**
- 1 duplicate SHA(s) detected (expected for collision/creature/wall-blocked pairs)

### SHA distribution

- `9d314f20ebfc`: 2 capture(s)
