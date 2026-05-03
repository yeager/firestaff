# Fas 0.2: Dungeon Data → Viewport Integration

## Sektion 1: Compat-lagrets API

### Funktioner i `memory_dungeon_dat_pc34_compat.h`:

1. `F0882_WORLD_InitFromDungeonDat_Compat` - Initierar världen från DUNGEON.DAT
2. `F0883_WORLD_Free_Compat` - Frigör världens minne
3. `F0884_ORCH_AdvanceOneTick_Compat` - Framtvingar en tick
4. `F0885_ORCH_AdjustPartyPosition_Compat` - Justerar partyposition
5. `F0886_ORCH_SetPartyPosition_Compat` - Sätter partyposition
6. `F0887_ORCH_CalculatePartyMovement_Compat` - Beräknar partymovment
7. `F0888_ORCH_CalculatePartyTurn_Compat` - Beräknar partytur
8. `F0889_ORCH_SolveCreatureAction_Compat` - Löser kreaturshandling
9. `F0890_ORCH_CalculatePartyDirection_Compat` - Beräknar partyriktning
10. `F0891_ORCH_WorldHash_Compat` - Genererar världshash
11. `F0892_ORCH_GetPartyCurrentSquare_Compat` - Hämtar nuvarande kvadrat
12. `F0893_ORCH_GetPartySquareType_Compat` - Hämtar kvadrattyp

## Sektion 2: Viewport-kodens datakällor

Viewport-koden i `m11_game_view.c` har en egen världsmodell i `M11_GameViewState` struct.

### Struct-fält som används:

1. `state->world.dungeon` - DUNGEON.DAT struktur
2. `state->world.party` - Partystatus (champion data)
3. `state->world.gameTick` - Spelets tick
4. `state->world.things` - Saker och ting på kartan
5. `state->exploredBits` - Utforskat bitar
6. `state->messageLog` - Meddelandelogg
7. `state->lastTickResult` - Resultat från senaste tick

### Funktioner som läser världsdata:

1. `m11_sample_viewport_cell` - Hämtar information om en cell
2. `M11_GameView_Draw` - Renderar viewport
3. `m11_get_first_square_thing` - Hämtar första saken på en ruta
4. `m11_find_group_on_square` - Hittar grupp på en ruta
5. `m11_square_walkable_for_creature` - Kontrollerar om kvadrat är gåbar
6. `m11_apply_tick` - Använder världens data för att avancera tick
7. `m11_process_creature_ticks` - Hanterar kreaturs-tick
8. `m11_get_active_champion` - Hämtar aktiv champion
9. `m11_mark_explored` - Markerar ruta som utforskad
10. `m11_find_group_position` - Hittar gruppens position

## Sektion 3: Kopplingspunkter

### Funktioner som bör ersättas/ändras:

1. `m11_sample_viewport_cell` - Denna är huvudfunktion som används för viewport rendering och borde läsa från compat-lagret istället för egen data
2. `m11_get_first_square_thing` - Används för att hämta saker på kartan
3. `m11_find_group_on_square` - Används för att hitta grupper
4. `m11_square_walkable_for_creature` - Kontrollerar gåbarhet
5. `m11_get_active_champion` - Hämtar aktiv champion
6. `m11_find_group_position` - Hittar gruppers position
7. `m11_apply_tick` - Använder världens värden

## Sektion 4: Implementationsplan

1. **Identifiera världens datastrukturer** - Leta efter hur `state->world` använder `m11_game_view.c` data och hur den kopplas till `memory_dungeon_dat_pc34_compat.c`
2. **Skapa kompatibilitetsfunktioner** - Skapa wrapper-funktioner i `m11_game_view.c` som kallar compat-funktioner istället för egen logik
3. **Anpassa `m11_sample_viewport_cell`** - Ändra den så den läser direkt från compat-lagret istället av egen data
4. **Uppdatera källreferenser** - Anpassa `m11_game_view.c` att använda `M11_GameView_GetSquare` eller liknande kompat-funktioner istället för eget `state->world.dungeon->maps[...].` läsning
5. **Anpassa datatyper** - Se till att `M11_GameViewState` och `GameWorld_Compat` är korrekt kopplade
6. **Kontrollera utdata-format** - Se till att viewport-renderingen får korrekt dataformat från compat-lagret
7. **Testa interaktioner** - Kontrollera att kreaturshandlingar, partymovment, och andra interaktioner fungerar som förväntat
8. **Uppdatera dokumentation** - Reflektera ändringarna i kodens kommentarer och dokumentation
9. **Verifiera rendering** - Se till att viewporten visar korrekt data från det parsade formatet
10. **Fullständig integrationstest** - Testa hela systemet för att säkerställa att det fungerar korrekt
