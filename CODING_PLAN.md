# Firestaff → ReDMCSB 1:1 Kodningsplan (rev 2)

## Nulägesanalys (korrigerad)

Koden är MYCKET mer komplett än initialt uppskattat. Tick orchestrator (`F0884`) ÄR kopplad till runtime via `m11_apply_tick()`. Stora delar av rendering, movement, combat, inventory, creatures, spells, save/load finns redan.

### Redan implementerat
- ✅ Tick orchestrator (F0884) → runtime integration
- ✅ Movement (N/S/E/W, turn L/R) med collision
- ✅ Combat (CMD_ATTACK, damage emissions)
- ✅ Spell casting (CMD_CAST_SPELL)
- ✅ Creature AI (341 refs, tick-driven)
- ✅ Inventory system (96 refs, panel toggle)
- ✅ Door rendering + animation (212 refs)
- ✅ Pit/stair/teleport (91 refs)
- ✅ Torch/light system (106 refs)
- ✅ QuickSave/QuickLoad (binary serialization)
- ✅ Entrance sequence + palette crossfade
- ✅ Title animation
- ✅ Viewport: ceiling, floor, side walls, front walls
- ✅ Viewport: ornaments, items, creatures
- ✅ HUD: champion portraits, bars, status

### Saknade commands (vs orchestrator)
- ❌ CMD_DRINK
- ❌ CMD_EAT
- ❌ CMD_REST_TOGGLE
- ❌ CMD_THROW_ITEM
- ❌ CMD_USE_ITEM

### Saknade/ofullständiga features
- ⚠️ Wall rendering bugg (flip/draw order)
- ⚠️ Viewport frame debugging → alla visuella artefakter
- ❌ Item use/eat/drink/throw UI routing
- ❌ Spell symbol UI (draw spell runes, select symbols)
- ❌ Full save/load (ReDMCSB format, not quicksave)
- ❌ Sound effects (audio_sdl_m11 har stubs)
- ❌ Music (song.dat loader exists but not wired)
- ❌ Endgame sequence (compat exists, not wired to runtime)
- ❌ Resurrect/reincarnate UI flow
- ❌ Map overlay / automap
- ❌ Text/message scroll display
- ❌ FTL swoosh (real graphics, not placeholder)

## Ny arkitektur

```
┌─────────────────────────────────────────────┐
│  Opus 4.6 (koordinator)                     │
│  - Analyserar gaps                          │
│  - Skriver specs + context snippets         │
│  - Granskar resultat                        │
│  - Bygger & testar lokalt                   │
│  - Committar & pushar till GitHub main      │
└──────────────┬──────────────────────────────┘
               │ spawnar subagenter med EXAKT
               │ kontext (< 2000 rader per jobb)
    ┌──────────┼──────────────┐
    │          │              │
    ▼          ▼              ▼
 Qwen3-Coder  DeepSeek-R1   Qwen3.5
 (kodning)    (review)      (analys)
 DANNESBURK   DANNESBURK    DANNESBURK
```

### Modellroller (reviderade)
- **Qwen3-Coder**: Små, avgränsade uppgifter: "skriv denna funktion", "implementera denna command handler"
- **DeepSeek-R1**: Review — jämför implementation mot ReDMCSB source snippet
- **Qwen3.5**: Snabb analys av enstaka filer
- **Opus 4.6**: Analyserar kodbasen, planerar, granskar, commit/push

### Lärdomar från rev 1
- Qwen3-Coder 30B (3B aktiv MoE) klarar INTE att läsa stora filer + analysera + producera kod i en session
- Maximal kontext per subagent-jobb: ~2000 rader, 1 specifik funktion/feature
- Opus måste förbereda exakt kontext (relevanta code snippets) istället för "läs hela filen"

## Reviderade faser

### Fas 1: Visuell paritet (viewport fix) — PÅGÅENDE
| # | Uppgift | Status |
|---|---------|--------|
| 1.1 | Wall flip bit korrekt | ⚠️ Implementerad men ej verifierad |
| 1.2 | Wall draw order back-to-front | ⚠️ Behöver verifiering |
| 1.3 | Side wall occlusion | ⚠️ Behöver verifiering |
| 1.4 | Door state rendering | ⚠️ Behöver verifiering |
| 1.5 | Asset trace (stderr debug) | ❌ Producerar ingen output |

### Fas 2: Saknade commands (5 st)
| # | Command | ReDMCSB | Beskrivning |
|---|---------|---------|-------------|
| 2.1 | CMD_EAT | COMMAND.C | Äta mat-objekt, recover food counter |
| 2.2 | CMD_DRINK | COMMAND.C | Dricka vatten/potion |
| 2.3 | CMD_THROW_ITEM | COMMAND.C | Kasta objekt (skapar projectile) |
| 2.4 | CMD_USE_ITEM | COMMAND.C | Använd objekt (nycklar, scrolls, etc) |
| 2.5 | CMD_REST_TOGGLE | COMMAND.C | Vila-läge toggle |

### Fas 3: UI completion
| # | Uppgift | ReDMCSB | Beskrivning |
|---|---------|---------|-------------|
| 3.1 | Spell symbol UI | SPELDRAW.C | Rita/välj trollformelruner |
| 3.2 | Resurrect/reincarnate flow | REVIVE.C | Champion death UI |
| 3.3 | Text/message display | TEXT.C, DRAWMSGA.C | Meddelanden i viewport |
| 3.4 | Map overlay | — | Automap (om original har det) |

### Fas 4: Audio
| # | Uppgift | ReDMCSB | Beskrivning |
|---|---------|---------|-------------|
| 4.1 | Sound effects | SOUND.C | Ljud via emissions (EMIT_SOUND_REQUEST) |
| 4.2 | Music playback | MUSIC.C | Bakgrundsmusik från SONG.DAT |

### Fas 5: Save/Load + Endgame
| # | Uppgift | ReDMCSB | Beskrivning |
|---|---------|---------|-------------|
| 5.1 | ReDMCSB save format | LOADSAVE.C | Kompatibel save/load |
| 5.2 | Endgame sequence | ENDGAME.C | Slutsekvens |

## Git-strategi
- All kod på `main`
- Varje modul = 1-3 commits
- Bygg + test INNAN push
- Baseline: 28 failures / 60 tests (0 nya tillåtna)

## Prioritering
```
Viewport fix → Saknade commands → Spell UI → Audio → Save/Load → Endgame
```
