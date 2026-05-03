# Firestaff → ReDMCSB 1:1 Kodningsplan

## Nuläge

| Kategori | Antal filer | Status |
|---|---|---|
| Runtime (faktiskt spelkod) | 6 | m11_game_view, main_loop_m11, render_sdl_m11, audio_sdl_m11, font_m11, asset_loader_m11 |
| Compat-lager (ReDMCSB-wrappers) | 234 | Datastrukturer, parsing, probes — men INTE sammankopplade till runtime |
| Probes (test/verifiering) | 34 | Source-lock gates, men flesta failar |
| Tester | 60 | 31 pass, 29 fail |

**Kärnproblem:** Firestaff har massa compat-lager som parsear/validerar ReDMCSB-data, men runtime-koden (det som faktiskt kör spelet) är bara 6 filer som manuellt implementerar rendering/input. Compat-lagren är inte kopplade till runtime.

## Arkitektur

```
┌─────────────────────────────────────────────┐
│  Opus 4.6 (koordinator)                     │
│  - Planerar moduler                         │
│  - Granskar resultat                        │
│  - Bygger & testar lokalt                   │
│  - Committar & pushar till GitHub main      │
└──────────────┬──────────────────────────────┘
               │ spawnar subagenter
    ┌──────────┼──────────────┐
    │          │              │
    ▼          ▼              ▼
 Qwen3-Coder  DeepSeek-R1   Qwen3.5
 (kodning)    (review)      (analys)
 DANNESBURK   DANNESBURK    DANNESBURK
```

### Modellroller
- **Qwen3-Coder** (30B): All kodning. Får ReDMCSB-source + Firestaff-context, skriver implementation.
- **DeepSeek-R1** (32B): Review. Jämför implementation mot ReDMCSB source, hittar buggar.
- **Qwen3.5** (27B): Snabb analys, prototyping, dokumentation.
- **Opus 4.6**: BARA koordination, verifiering, commit/push. Minimala tokens.

### Arbetsflöde per modul
1. Opus identifierar nästa modul + skriver spec med ReDMCSB-referenser
2. Qwen3-Coder subagent på N2: läser ReDMCSB source, skriver implementation + tester
3. Opus pullar, bygger, kör tester (< 30s per iteration)
4. Om fail: steerar subagent med exakt felmeddelande
5. Om pass: committar + pushar till `main`

## Faser

### Fas 0: Foundation (dag 1)
**Mål:** Koppla ihop compat-lagren med runtime.

| # | Modul | ReDMCSB-filer | Status | Beskrivning |
|---|---|---|---|---|
| 0.1 | Game loop integration | GAMELOOP.C, DM.C, _MAIN.C | ⚠️ Stub | Koppla `memory_tick_orchestrator` till `main_loop_m11` |
| 0.2 | Dungeon data → viewport | DUNGEON.C, DATA.C | ⚠️ Delvis | `memory_dungeon_dat` parsear — men viewport läser inte därifrån |
| 0.3 | GRAPHICS.DAT → renderer | GRF1.C, IMAGE.C | ⚠️ Delvis | `memory_graphics_dat` laddar — men renderer använder egen asset pipeline |
| 0.4 | Input → command pipeline | INPUT.C, COMMAND.C, NEC816.C | ⚠️ Delvis | `dm1_v1_input_command_queue` finns — inte kopplad till game loop |

### Fas 1: Dungeon Core (dag 2-3)
**Mål:** Korrekt dungeon-rendering med alla element.

| # | Modul | ReDMCSB-filer | Prio | Beskrivning |
|---|---|---|---|---|
| 1.1 | Viewport rendering | DUNVIEW.C, VIEWPORT.C, DRAWVIEW.C | 🔴 | Back-to-front wall drawing, door states, ornaments |
| 1.2 | Coordinate system | COORD.C | 🔴 | Relative coordinates, facing direction transforms |
| 1.3 | Wall/floor/ceiling blitting | BLIT.C, BLITMASK.C, FILLBOX.C | 🔴 | Transparency, masking, fill operations |
| 1.4 | Door rendering + animation | DUNVIEW.C (door sections) | 🟡 | Door states, opening/closing animation frames |
| 1.5 | Floor items + creatures | DUNVIEW.C (item/creature sections) | 🟡 | Item graphics, creature positioning |

### Fas 2: Champions & UI (dag 3-5)
**Mål:** HUD, inventory, champion status.

| # | Modul | ReDMCSB-filer | Prio | Beskrivning |
|---|---|---|---|---|
| 2.1 | Champion panel | CHAMPION.C, CHAMDRAW.C, PANEL.C | 🔴 | HP/MP/stamina bars, portrait, name |
| 2.2 | Inventory system | CHEST.C, OBJECT.C, GROUP.C | 🔴 | Drag-drop, equipment slots, containers |
| 2.3 | Status screen | STATS.C, PORTRAIT.C | 🟡 | Full champion stats, skill levels |
| 2.4 | Spell interface | SPELDRAW.C, SPELFAIL.C | 🟡 | Spell symbols, casting UI |
| 2.5 | Menu/dialog | MENU.C, MENUDRAW.C, DIALOG.C | 🟡 | Save/load menu, text dialogs |

### Fas 3: Game Mechanics (dag 5-8)
**Mål:** Spelets kärna — movement, combat, magic.

| # | Modul | ReDMCSB-filer | Prio | Beskrivning |
|---|---|---|---|---|
| 3.1 | Movement | MOVESENS.C, COMMAND.C | 🔴 | Party movement, collision, turning, stepping |
| 3.2 | Combat system | (combat sections of COMMAND.C) | 🔴 | Melee, ranged, damage calculation |
| 3.3 | Magic system | (magic sections) | 🔴 | Spell casting, effects, mana cost |
| 3.4 | Creature AI | (creature sections of MOVESENS.C) | 🟡 | Creature movement, aggro, pathfinding |
| 3.5 | Projectiles | PROJEXPL.C | 🟡 | Projectile movement, collision, effects |
| 3.6 | Sensors/triggers | MOVESENS.C (sensor sections) | 🟡 | Pressure plates, wall sensors, teleporters |
| 3.7 | Timeline/ticks | TIMELINE.C, VBLANK.C | 🔴 | Game timing, event scheduling |

### Fas 4: World Systems (dag 8-10)
**Mål:** Komplett spelupplevelse.

| # | Modul | ReDMCSB-filer | Prio | Beskrivning |
|---|---|---|---|---|
| 4.1 | Save/load | LOADSAVE.C, SAVEHEAD.C | 🔴 | Fullständig save/load av spelstate |
| 4.2 | Sound/music | SOUND.C, MUSIC.C | 🟡 | Ljud-effekter, bakgrundsmusik |
| 4.3 | Light/dark | PALETTE.C, DARKCOLR.C | 🟡 | Fackellju, mörker, light level |
| 4.4 | Text/messages | TEXT.C, TEXT2.C, DRAWMSGA.C, FONT.C | 🟡 | Meddelanden, scroll text |
| 4.5 | Endgame | ENDGAME.C | 🟢 | Slutsekvens när spelet klaras |

### Fas 5: Polish & Edge Cases (dag 10-14)
**Mål:** 1:1 parity med ReDMCSB.

| # | Modul | ReDMCSB-filer | Prio | Beskrivning |
|---|---|---|---|---|
| 5.1 | Resurrect/reincarnate | REVIVE.C, CHAMPRST.C | 🟡 | Champion death/revival mechanics |
| 5.2 | Object interactions | GSTHINGS.C, GROUP.C | 🟡 | Item combine, food/water, torches |
| 5.3 | Map transitions | NEWMAP.C | 🟡 | Level transitions, stairs, pits |
| 5.4 | Memory management | MEMORY.C, MEM1*.C | 🟢 | Cache, allocation (modern malloc OK) |
| 5.5 | Edge case parity | Diverse | 🟢 | Bug-for-bug compatibility med original |

## Prioriteringsordning

**Kritisk path till spelbart DM1:**
```
Fas 0 (foundation) → Fas 1.1-1.3 (viewport) → Fas 3.1 (movement) → Fas 3.7 (timing)
→ Fas 2.1 (champion panel) → Fas 3.2 (combat) → Fas 2.2 (inventory) → Fas 3.3 (magic)
→ Fas 4.1 (save/load) → Fas 1.4-1.5 (doors/items) → Fas 3.4-3.6 (creatures/sensors)
```

## Git-strategi

- All kod på `main` — inga feature branches
- Varje modul = 1-3 commits med tydliga meddelanden
- Bygg + test INNAN push
- Inga nya test failures (baseline: 29 fail / 60 total)

## Token-budget

| Modell | Roll | Estimat per modul |
|---|---|---|
| Qwen3-Coder | Kodning | ~50K tokens (gratis, lokal) |
| DeepSeek-R1 | Review | ~20K tokens (gratis, lokal) |
| Qwen3.5 | Analys | ~10K tokens (gratis, lokal) |
| Opus 4.6 | Koordination | ~2K tokens (betald) |

**Total Opus-kostnad per modul:** ~$0.10-0.20
**Total projekt:** ~30 moduler × $0.15 ≈ $4.50 i Opus-tokens

## Nästa steg

1. ✅ Gap-analys körs (Qwen3-Coder)
2. [ ] Granska GAP_ANALYSIS.md
3. [ ] Starta Fas 0.1: Game loop integration
4. [ ] Iterera genom faserna
