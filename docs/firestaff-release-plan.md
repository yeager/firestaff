# Firestaff Release Plan — v2.1 & v2.2

## Version roadmap

| Version | Focus | Status | Dependency |
|---------|-------|--------|----------|
| **v1.x** | DM1 V1 1:1 parity (ReDMCSB) | Active — bug fixes in progress | — |
| **v2.1** | Visual upgrade + foundational QoL | ~35% scaffolded | Stable V1 |
| **v2.2** | Gameplay improvements + modern UX | ~30% scaffolded | v2.1 |
| **v2.3** | Touch/mobile + accessibility | ~15% (touch source-locked) | v2.2 |

---

## v2.1 — Visual upgrade + foundational QoL

**Theme:** Look better, play the same. No gameplay changes — presentation only.

### Milestone 1: Rendering pipeline (prerequisite)
- [ ] `dm1_v2_viewport_renderer_pc34` — V2 viewport rendering (1,067 lines, largest V2 file)
- [ ] `dm1_v2_texture_upscale_pc34` — EPX/nearest/bilinear upscaling (169 lines)
- [ ] `dm1_v2_lighting_dynamic_pc34` — dynamic lighting (190 lines)
- [ ] `dm1_v2_item_render_pc34` — improved object rendering (94 lines)

### Milestone 2: Camera and movement
- [ ] `dm1_v2_camera_controller_pc34` — smooth camera control (130 lines)
- [ ] `dm1_v2_camera_shake_pc34` — shake effect on damage/explosion (97 lines)
- [ ] `dm1_v2_smooth_movement_pc34` — interpolated movement (158 lines)
- [ ] `dm1_v2_movement_engine_pc34` — V2 movement engine (159 lines)
- [ ] `dm1_v2_movement_command_adapter_pc34` — V1→V2 command bridge (97 rader)
- [ ] `dm1_v2_level_transition_pc34` — level-transition effects (174 lines)
- [ ] `dm1_v2_screen_transition_pc34` — fade/wipe transitions (53 lines)

### Milestone 3: Animation and timing
- [ ] `dm1_v2_anim_timing_pc34` — V2_AnimClock with seven easing functions (143 lines)
- [ ] `dm1_v2_creature_animation_pc34` — creature sprite animations (177 lines)
- [ ] `dm1_v2_spell_effect_overlay_pc34` — spell effects over the viewport (142 lines)
- [ ] `dm1_v2_particle_system_pc34` — particle system (80 lines)
- [ ] `dm1_v2_particle_emitter_presets_pc34` — preset effects: torch, magic, combat (98 lines)
- [ ] `dm1_v2_weather_fx_pc34` — weather effects (90 lines)

### Milestone 4: Audio
- [ ] `dm1_v2_audio_mixer_pc34` — channel-based audio mixing (102 lines)
- [ ] `dm1_v2_footstep_audio_pc34` — footsteps based on surface (77 lines)

### Milestone 5: HUD and UI foundation
- [ ] `dm1_v2_hud_overlay_pc34` — HUD-overlay (188 rader)
- [ ] `dm1_v2_damage_numbers_pc34` — floating damage numbers (104 rader)
- [ ] `dm1_v2_tooltip_pc34` — hover-tooltips (110 rader)
- [ ] `dm1_v2_message_log_pc34` — scrolling message log (80 lines)

### Milestone 6: V2 runtime + settings
- [ ] `dm1_v2_runtime_pc34` — V2 runtime shell (215 rader)
- [ ] `dm1_v2_settings_pc34` + `dm1_v2_settings_impl` — settings menu (170+169 lines)
- [ ] `dm1_v2_screenshot_pc34` — screenshot function (114 lines)

**v2.1 release criteria:**
1. V1 parity remains intact — all V1 tests pass
2. V2 can be toggled with `FIRESTAFF_V2=0/1`
3. Rendering pipeline + kamera + ljud + HUD fungerar end-to-end
4. At least one game (DM1) playable through level 1 in V2 mode

---

## v2.2 — Gameplay improvements + modern UX

**Theme:** Play smarter. Improve navigation without changing balance.

### Milestone 7: Map features
- [ ] `dm1_v2_minimap` + `dm1_v2_minimap_pc34` — automap med fog of war (84+90 rader)
- [x] `dm1_v2_pathfinding_pc34` — synthetic A* search blocked; PC34 instead
  selects each creature direction in `GROUP.C` F0202/F0203.

### Milestone 8: Inventory and champions
- [ ] `dm1_v2_inventory_sort_pc34` — sort inventory (93 lines)
- [ ] `dm1_v2_champion_select_pc34` — improved champion selection (70 lines)
- [ ] `dm1_v2_hud_interaction_pc34` — clickable HUD (127 lines)

### Milestone 9: Journal and statistics
- [ ] `dm1_v2_journal` + `dm1_v2_journal_pc34` — game journal/logbook (72+102 lines)
- [ ] `dm1_v2_achievements` + `dm1_v2_achievements_pc34` — achievement system (62+100 lines)
- [ ] `dm1_v2_stat_tracker_pc34` — statistics tracking (63 lines)

### Milestone 10: Input and saving
- [ ] `dm1_v2_input_remap_pc34` — customizable key bindings (91 lines)
- [ ] `dm1_v2_auto_save_pc34` — automatic saving on level changes (93 lines)

**v2.2 release criteria:**
1. Everything from v2.1 is stable
2. Minimap, journal, achievements, inventory sort fungerar in-game
3. Auto-save works during level transitions
4. Input remapping persists across sessions

---

## v2.3 — Touch/mobile + accessibility (future)

### Milestone 11: Touchscreen support
- [ ] Touch input abstraktion — click zones → touch zones
- [ ] Scaled UI for mobile screens
- [ ] Touch source lock (in progress — HEARTBEAT.md lane D)
- [ ] Gesture support: swipe to turn, tap to interact

### Milestone 12: Accessibility
- [ ] Scalable fonts
- [ ] Colour-blind mode
- [ ] Keyboard navigation through every menu

---

## Dependencies and ordering

```
V1 bug fixes (in progress, rounds 1-5 complete)
    └── v2.1 Milestones 1-6 (rendering → runtime)
            └── v2.2 Milestones 7-10 (gameplay QoL)
                    └── v2.3 Milestones 11-12 (touch + a11y)
```

## Working rules

1. **V1 parity first.** No V2 feature may break V1 tests.
2. **Toggle.** All V2 features remain behind the `FIRESTAFF_V2` compile flag.
3. **One milestone at a time.** Complete and verify it before starting the next.
4. **No releases without Daniel's approval.**
5. **ReDMCSB reference.** V2 gameplay features must cite ReDMCSB as their baseline.
