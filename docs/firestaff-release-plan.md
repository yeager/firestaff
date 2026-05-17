# Firestaff Release Plan — v2.1 & v2.2

## Versionsschema

| Version | Fokus | Status | Beroende |
|---------|-------|--------|----------|
| **v1.x** | DM1 V1 1:1 parity (ReDMCSB) | Aktiv — bugfixar pågår | — |
| **v2.1** | Visuell uppgradering + grundläggande QoL | ~35% scaffolded | V1 stabil |
| **v2.2** | Gameplay-förbättringar + modern UX | ~30% scaffolded | v2.1 |
| **v2.3** | Touch/mobil + tillgänglighet | ~15% (touch source-locked) | v2.2 |

---

## v2.1 — Visuell uppgradering + grundläggande QoL

**Tema:** Se bättre ut, spela likadant. Ingen gameplay-ändring — bara presentation.

### Milestone 1: Rendering pipeline (grundförutsättning)
- [ ] `dm1_v2_viewport_renderer_pc34` — V2 viewport-rendering (1067 rader, största V2-filen)
- [ ] `dm1_v2_texture_upscale_pc34` — EPX/nearest/bilinear uppskalning (169 rader)
- [ ] `dm1_v2_lighting_dynamic_pc34` — dynamisk belysning (190 rader)
- [ ] `dm1_v2_item_render_pc34` — förbättrad objektrendering (94 rader)

### Milestone 2: Kamera och rörelse
- [ ] `dm1_v2_camera_controller_pc34` — smooth kamerakontroll (130 rader)
- [ ] `dm1_v2_camera_shake_pc34` — skakeffekt vid skada/explosion (97 rader)
- [ ] `dm1_v2_smooth_movement_pc34` — interpolerad rörelse (158 rader)
- [ ] `dm1_v2_movement_engine_pc34` — V2 movement engine (159 rader)
- [ ] `dm1_v2_movement_command_adapter_pc34` — V1→V2 command bridge (97 rader)
- [ ] `dm1_v2_level_transition_pc34` — nivåövergångseffekter (174 rader)
- [ ] `dm1_v2_screen_transition_pc34` — fade/wipe-övergångar (53 rader)

### Milestone 3: Animation och timing
- [ ] `dm1_v2_anim_timing_pc34` — V2_AnimClock med 7 easing-funktioner (143 rader)
- [ ] `dm1_v2_creature_animation_pc34` — creature sprite-animationer (177 rader)
- [ ] `dm1_v2_spell_effect_overlay_pc34` — spell-effekter ovanpå viewport (142 rader)
- [ ] `dm1_v2_particle_system_pc34` — partikelsystem (80 rader)
- [ ] `dm1_v2_particle_emitter_presets_pc34` — preset-effekter: fackla, magi, strid (98 rader)
- [ ] `dm1_v2_weather_fx_pc34` — vädereffekter (90 rader)

### Milestone 4: Ljud
- [ ] `dm1_v2_audio_mixer_pc34` — ljudmixning med kanaler (102 rader)
- [ ] `dm1_v2_footstep_audio_pc34` — stegljud beroende på underlag (77 rader)

### Milestone 5: HUD och UI-grund
- [ ] `dm1_v2_hud_overlay_pc34` — HUD-overlay (188 rader)
- [ ] `dm1_v2_damage_numbers_pc34` — floating damage numbers (104 rader)
- [ ] `dm1_v2_tooltip_pc34` — hover-tooltips (110 rader)
- [ ] `dm1_v2_message_log_pc34` — scrollande meddelandelogg (80 rader)

### Milestone 6: V2 runtime + settings
- [ ] `dm1_v2_runtime_pc34` — V2 runtime shell (215 rader)
- [ ] `dm1_v2_settings_pc34` + `dm1_v2_settings_impl` — inställningsmeny (170+169 rader)
- [ ] `dm1_v2_screenshot_pc34` — screenshot-funktion (114 rader)

**v2.1 release-kriterier:**
1. V1-paritet obruten — alla V1-tester passerar
2. V2 kan växlas av/på med `FIRESTAFF_V2=0/1`
3. Rendering pipeline + kamera + ljud + HUD fungerar end-to-end
4. Minst ett spel (DM1) spelbart i V2-läge genom level 1

---

## v2.2 — Gameplay-förbättringar + modern UX

**Tema:** Spela smartare. Allt som gör spelet lättare att navigera utan att ändra balansen.

### Milestone 7: Kartfunktioner
- [ ] `dm1_v2_minimap` + `dm1_v2_minimap_pc34` — automap med fog of war (84+90 rader)
- [ ] `dm1_v2_pathfinding_pc34` — A* pathfinding för creature AI (123 rader)

### Milestone 8: Inventory och champion
- [ ] `dm1_v2_inventory_sort_pc34` — sortera inventariet (93 rader)
- [ ] `dm1_v2_champion_select_pc34` — förbättrat champion-val (70 rader)
- [ ] `dm1_v2_hud_interaction_pc34` — klickbar HUD (127 rader)

### Milestone 9: Journal och statistik
- [ ] `dm1_v2_journal` + `dm1_v2_journal_pc34` — speljournal/loggbok (72+102 rader)
- [ ] `dm1_v2_achievements` + `dm1_v2_achievements_pc34` — achievement-system (62+100 rader)
- [ ] `dm1_v2_stat_tracker_pc34` — statistikspårning (63 rader)

### Milestone 10: Input och sparning
- [ ] `dm1_v2_input_remap_pc34` — anpassningsbara tangentbindningar (91 rader)
- [ ] `dm1_v2_auto_save_pc34` — automatisk sparning vid level-byte (93 rader)

**v2.2 release-kriterier:**
1. Allt från v2.1 stabilt
2. Minimap, journal, achievements, inventory sort fungerar in-game
3. Auto-save fungerar vid levelövergångar
4. Input remap sparas/laddas mellan sessioner

---

## v2.3 — Touch/mobil + tillgänglighet (framtid)

### Milestone 11: Pekskärmsstöd
- [ ] Touch input abstraktion — click zones → touch zones
- [ ] Skalad UI för mobila skärmar
- [ ] Touch source-lock (pågår — HEARTBEAT.md lane D)
- [ ] Gesture-stöd: swipe för turn, tap för interact

### Milestone 12: Tillgänglighet
- [ ] Skalbara fonts
- [ ] Färgblindläge
- [ ] Tangentbordsnavigering genom alla menyer

---

## Beroenden och ordning

```
V1 bugfixar (pågår, round 1-5 klara)
    └── v2.1 Milestone 1-6 (rendering → runtime)
            └── v2.2 Milestone 7-10 (gameplay QoL)
                    └── v2.3 Milestone 11-12 (touch + a11y)
```

## Arbetsregler

1. **V1-paritet först.** Ingen V2-feature får bryta V1-tester.
2. **Toggle.** Alla V2-features bakom `FIRESTAFF_V2` compile flag.
3. **En milestone i taget.** Klart + verifierat innan nästa startar.
4. **Inga releases utan Daniels order.**
5. **ReDMCSB-referens.** V2-features som rör gameplay måste citera ReDMCSB som baseline.
