# Dungeon Master 1 (DM1) — Reverse Engineering Wiki

This page documents the reverse-engineering knowledge base backing Firestaff's
Dungeon Master 1 (DOS PC 3.4, "I34E") implementation. The canonical reference
source is **ReDMCSB**, a reconstructed C decompilation of the original
Dungeon Master / Chaos Strikes Back engine. Firestaff comments cite ReDMCSB
files and F-numbered functions directly (`ReDMCSB FILENAME.C:LINE`,
`F0NNN_FUNCTION_NAME`), and DM1-specific compatibility modules live under
`src/dm1/` and `include/dm1_v1_*_pc34_compat.h`.

All data below was extracted from the current Firestaff source tree with:

```bash
grep -rohE 'ReDMCSB [A-Z0-9_]+\.[CH]' include/dm1_v1_* src/dm1/ | sort -u
grep -rohE 'F[0-9]{4}_[A-Za-z0-9_]+' include/dm1_v1_* src/dm1/ | sort -u
```

---

## 1. ReDMCSB Source File Map

96 distinct ReDMCSB source/header filenames are referenced across DM1
compatibility headers and sources. They are grouped below by subsystem.
Note that DM1 and CSB (Chaos Strikes Back) share the same PC 3.4 engine
codebase in ReDMCSB, so many files serve both games.

### Core / Main

| File | Purpose |
|---|---|
| `MAINLIB.C` | Core primitives: copy/clear bytes, blit-to-screen, random number generators (F0002-F0030) |
| `GAMELOOP.C` | Main game loop (`F0002_MAIN_GameLoop`), tick dispatch |
| `BASE.C` | Base runtime services, error display |
| `COMPILE.H` | Compile-time configuration switches |
| `DEFS.H` | Global constant/macro definitions (element types, thing types, sensor ranges) |
| `MEMORY.C` | Memory allocator (used-list / defrag model for GRAPHICS.DAT) |
| `DATA.C` | Static game data tables |
| `COORD.C` | Coordinate/direction math |

### Startup / Entrance / Title

| File | Purpose |
|---|---|
| `STARTUP2.C` | Startup sequence, F0800-F0910 range functions |
| `ENTRANCE.C` | Dungeon entrance sequence |
| `TITLE.C` | Title screen |
| `SWSH.C` | FTL logo "swoosh" animation |
| `SWITCH.C` / `SWITCHMM.C` | Startup/menu state machine switches |
| `ENDGAME.C` | Endgame/victory sequence |
| `AMIGINIT.C` | Amiga-specific init (ReDMCSB is multi-platform; PC34 reuses shared init logic) |

### Dungeon (data model)

| File | Purpose |
|---|---|
| `DUNGEON.C` | Core dungeon data access: squares, things, thing lists (F0139-F0174) |
| `DUNVIEW.C` | Dungeon-view-adjacent dungeon helpers |
| `NEWMAP.C` | New-map / map-transition handling |
| `MOVESENS.C` | Movement + sensor interaction |
| `GROUP.C` | Monster group data and AI (F0175-F0212) |

### Viewport / Rendering

| File | Purpose |
|---|---|
| `VIEWPORT.C` | Viewport frame orchestration |
| `DRAWVIEW.C` | Dungeon-view drawing (F0093-F0135: floor/wall/door/creature blit chain) |
| `DRAW.C` | General drawing primitives |
| `BLIT.C` / `BLITMASK.C` | Raw bitmap blit + masked blit routines |
| `IMAGE.C` / `IMAGE2.C` | Bitmap/image management |
| `SCREEN.C` | Screen buffer management |
| `PALETTE.C` | Palette load/animate |
| `DARKCOLR.C` | Darkness/lighting color reduction |
| `PRIM1.C` | Low-level graphics primitives |
| `VIDEODRV.C` | Video driver abstraction |
| `MIRROR.C` | Horizontal-flip bitmap helper (used for D2R/D3R mirrored wall tiles) |
| `SYMBOL.C` | Symbol/glyph bitmap handling |

### Objects / Items

| File | Purpose |
|---|---|
| `OBJECT.C` | Object type/icon resolution (F0031-F0039) |
| `INVENTORY.C` / `INVNTORY.C` | Inventory panel drawing and interaction (F0332-F0349) |
| `CHEST.C` | Chest/container open/close |
| `AMMO.C` | Ammunition compatibility checks |

### Champions

| File | Purpose |
|---|---|
| `CHAMPION.C` / `CHAMPION.H` | Champion stats, slots, damage, skills (F0284-F0331) |
| `CHAMDRAW.C` | Champion portrait/state drawing |
| `CLIKCHAM.C` | Mouse click routing onto champion panel |
| `REVIVE.C` | Resurrection/reincarnation |

### Combat

| File | Purpose |
|---|---|
| `MELEE.C` | Melee combat resolution |
| `PROJECTILE.C` | Projectile creation/flight |
| `PROJEXPL.C` | Projectile explosion resolution |
| `ACTIDRAW.C` | Action icon drawing (attack/spell UI) |
| `SPELDRAW.C` | Spell casting UI drawing |

### Timeline / Events

| File | Purpose |
|---|---|
| `TIMELINE.C` | Scheduled-event queue (F0233-F0240) |

### Sensors / Doors

| File | Purpose |
|---|---|
| `SWITCHMM.C` | Wall/floor switch dispatch (shared with menu switches) |
| (sensor logic is embedded in `DUNGEON.C`/`MOVESENS.C`, wrapped by Firestaff `F07xx_*_Compat`) | Door toggling, sensor effect dispatch |

### Audio

| File | Purpose |
|---|---|
| `SOUND.C` | Sound effect request/playback queue (F0064-F0065) |
| `MUSIC.C` | Music driver: pause, play, track select, update (F0740-F0743) |

### Input / Mouse

| File | Purpose |
|---|---|
| `MOUSE.C` | Mouse pointer state, enable/disable, screen area (F0066-F0079) |
| `MOUSESET.C` | Mouse pointer bitmap sets |
| `CLIKMENU.C` | Menu click routing |
| `CLIKVIEW.C` | Viewport click routing |
| `COMMAND.C` | Command dispatch table |

### Save / Load

| File | Purpose |
|---|---|
| `LOADSAVE.C` | Save/load orchestration (F0417-F0435 checksum/obfuscation, dungeon tail) |
| `SAVEHEAD.C` | Save header checksum/obfuscation (F0429/F0430) |
| `SAVEPATH.C` | Save file path resolution |
| `SAVEUTIL.C` | `F0417_SAVEUTIL_GetChecksumAndObfuscate` and related utilities |
| `READWRIT.C` | Low-level read/write + obfuscation helpers |
| `FIO1.C` / `FIO1MAIN.C` | File I/O primitives |
| `FLOPPYAM.C` / `FLOPPYST.C` | Amiga/Atari ST floppy I/O (multi-platform ReDMCSB) |

### Graphics data (GRAPHICS.DAT)

| File | Purpose |
|---|---|
| `GRF1.C` | GRAPHICS.DAT access layer |
| `LZW.C` | LZW decompression of IMG3 graphic entries |
| `COPYPROE.C` | Copy-protection related graphics/data checks |

### UI / Text / Menu

| File | Purpose |
|---|---|
| `TEXT.C` | Message area, text printing (F0040-F0054) |
| `MENU.C` / `MENUDRAW.C` | Menu system and drawing |
| `DIALOG.C` | Dialog box rendering |
| `PANEL.C` | Side panel drawing |

### Hint system (Chaos Strikes Back editor / hint tooling)

| File | Purpose |
|---|---|
| `HINT001.C`, `HINT004.C`, `HINTCASE.C`, `HINTGRAP.C`, `HINTHINT.C`, `HINTIORQ.C` | CSB hint-oracle subsystem (shared engine, CSB-specific content) |
| `CEDT004.C`, `CEDTINCD.C`, `CEDTINCI.C` | Character/portrait editor tooling |

### Platform / Localization

| File | Purpose |
|---|---|
| `ATARIST.H` | Atari ST platform header (multi-platform ReDMCSB build) |
| `NEC816.C` / `NECIO.C` | NEC/PC-98 platform I/O (Japanese release support) |
| `JAPANESE.C` | Japanese text/localization handling |
| `USIO1.C` | US/international I/O variant |
| `UTAMSCR.C`, `UTSTGRAP.C`, `UTSTVDI3.C` | Atari ST utility/graphics/VDI helpers |
| `INT1.C` | Interrupt handler shim |
| `APPA.C` | Appendix/miscellaneous support routines |

---

## 2. F-Number Function Registry

912 distinct F-numbered identifiers are referenced in DM1 code (functions,
constants, and struct/field markers combined). The tables below cover the
~300 function-shaped F-numbers in the ranges most central to DM1 parity work,
grouped by subsystem. Names are the canonical Firestaff spelling found in
source comments; the originating ReDMCSB file is noted per group above.

### Core utilities (F0002-F0030) — `MAINLIB.C`

| F# | Function |
|---|---|
| F0002 | `MAIN_GameLoop` |
| F0007 | `MAIN_CopyBytes` |
| F0008 | `MAIN_ClearBytes` |
| F0010 | `MAIN_WriteSpacedWords` |
| F0020 | `MAIN_BlitToViewport` |
| F0021 | `MAIN_BlitToScreen` |
| F0022 | `MAIN_Delay` |
| F0024 | `MAIN_GetMinimumValue` |
| F0025 | `MAIN_GetMaximumValue` |
| F0026 | `MAIN_GetBoundedValue` |
| F0027 | `MAIN_Get16bitRandomNumber` |
| F0028 | `MAIN_Get1BitRandomNumber` |
| F0030 | `MAIN_GetScaledProduct` |

### Object/icon handling (F0031-F0039) — `OBJECT.C`

| F# | Function |
|---|---|
| F0031 | `OBJECT_LoadNames` |
| F0032 | `OBJECT_GetType` |
| F0033 | `OBJECT_GetIconIndex` |
| F0034 | `OBJECT_DrawLeaderHandObjectName` |
| F0035 | `OBJECT_ClearLeaderHandObjectName` |
| F0036 | `OBJECT_ExtractIconFromBitmap` |
| F0037 | `OBJECT_DrawIconToScreen` |
| F0038 | `OBJECT_DrawIconInSlotBox` |
| F0039 | `OBJECT_GetIconIndexInSlotBox` |

### Text/message area (F0040-F0054) — `TEXT.C`

| F# | Function |
|---|---|
| F0040 | `TEXT_Print` |
| F0042 | `TEXT_MESSAGEAREA_MoveCursor` |
| F0043 | `TEXT_MESSAGEAREA_ClearAllRows` |
| F0044 | `TEXT_MESSAGEAREA_ClearExpiredRows` |
| F0045 | `TEXT_MESSAGEAREA_CreateNewRow` |
| F0046 | `TEXT_MESSAGEAREA_PrintString` |
| F0047 | `TEXT_MESSAGEAREA_PrintMessage` |
| F0048 | `TEXT_MESSAGEAREA_PrintCharacter` |
| F0051 | `TEXT_MESSAGEAREA_PrintLineFeed` |
| F0052 | `TEXT_PrintToViewport` |
| F0053 | `TEXT_PrintToLogicalScreen` |
| F0054 | `TEXT_Initialize` |

### Sound (F0060-F0065) — `SOUND.C`

| F# | Function |
|---|---|
| F0064 | `SOUND_RequestPlay` |
| F0065 | `SOUND_PlayPendingSound` |

### Mouse/pointer (F0066-F0079) — `MOUSE.C`

| F# | Function |
|---|---|
| F0067 | `MOUSE_SetPointerToNormal` |
| F0068 | `MOUSE_SetPointerToObject` |
| F0073 | `MOUSE_BuildPointerScreenArea` |
| F0077 | `MOUSE_EnableScreen` |
| F0078 | `MOUSE_Disable` |

### Dungeon viewport (F0093-F0135) — `DRAWVIEW.C`, `VIDEODRV.C`

| F# | Function |
|---|---|
| F0093 | `DUNGEONVIEW_DrawDungeon` |
| F0094 | `DUNGEONVIEW_LoadFloorSet` |
| F0095 | `DUNGEONVIEW_LoadDoorSet` |
| F0096 | `DUNGEONVIEW_LoadCurrentMapGraphics` |
| F0097 | `DUNGEONVIEW_DrawViewport` |
| F0098 | `DUNGEONVIEW_DrawFloorAnd` (ceiling) |
| F0099 | `DUNGEONVIEW_CopyBitmapAndFlipHorizontal` |
| F0100 | `DrawWallSetBitmap` |
| F0101 | `DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency` |
| F0102 | `DUNGEONVIEW_DrawDoorBitmap` |
| F0103 | `DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally` |
| F0104 | `DrawFloorPitOrStairsBitmap` |
| F0105 | `DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlipped` |
| F0106 | `DUNGEONVIEW_TestResetToStep1` |
| F0107 | `IsDrawnWallOrnamentAnAlcove` |
| F0108 | `DrawFloorOrnament` |
| F0109 | `DUNGEONVIEW_DrawDoorOrnament` |
| F0110 | `DUNGEONVIEW_DrawDoorButton` |
| F0111 | `DrawD2C` |
| F0112 | `DUNGEONVIEW_DrawCeilingPit` |
| F0113 | `DUNGEONVIEW_DrawField` |
| F0114 | `GetExplosionBitmap` |
| F0115 | `DUNGEONVIEW_DrawObjectsCreatures` |
| F0116-F0121 | `DrawD3L/D3R/D3C/D2L/D2R/D2C` (distance-3 and distance-2 wall column draws) |
| F0122-F0127 | `DrawSquareD1L/D1R/D1C/D0L/D0R/D0C` (distance-1 and distance-0/foreground column draws) |
| F0128 | `G0076GetPc34Compat` |
| F0129 | `VIDEO_BlitShrinkWithPaletteChanges` |
| F0130 | `VIDEO_FlipHorizontal` |
| F0132 | `VIDEO_Blit` |
| F0133 | `VIDEO_BlitBoxFilledWithMaskedBitmap` |
| F0134 | `VIDEO_FillBitmap` |
| F0135 | `VIDEO_FillBox` |

This F0093-F0135 range is the **material matrix**: the D0/D1/D2/D3 naming
encodes viewport column distance (D0 = nearest row, D3 = farthest) and
L/R/C encode left/right/center screen position — this is the routing table
behind `m11_draw_viewport` and the CSB viewport callback path described in
CLAUDE.md.

### Dungeon data (F0139-F0174) — `DUNGEON.C`

| F# | Function |
|---|---|
| F0139 | `MakeThingPc34` (creature-allowed-on-map gate) |
| F0140, F0141 | `ObjectWorldPc34` |
| F0142 | `DUNGEON_GetProjectileAspect` |
| F0143 | `ArmourInfoPc34` / `DUNGEON_GetArmourDefense` |
| F0144 | `MakeThingPc34` (creature attributes) |
| F0145 | `DUNGEON_GetGroupCells` |
| F0146 | `DUNGEON_SetGroupCells` |
| F0147 | `DUNGEON_GetGroupDirections` |
| F0148 | `DUNGEON_SetGroupDirections` |
| F0149 | `DUNGEON_IsWallOrnamentAnAlcove` |
| F0150 | `DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` |
| F0151 | `DUNGEON_GetSquare` |
| F0152 | `DUNGEON_GetRelativeSquare` |
| F0153 | `DUNGEON_GetRelativeSquareType` |
| F0154 | `DUNGEON_GetLocationAfterLevelChange` |
| F0155 | `DUNGEON_GetStairsExitDirection` |
| F0156 | `DUNGEON_GetThingData` |
| F0157 | `DUNGEON_GetSquareFirstThingData` |
| F0158 | `DUNGEON_GetWeaponInfo` |
| F0159 | `DUNGEON_GetNextThing` |
| F0160 | `DUNGEON_GetSquareFirstThingIndex` |
| F0161 | `DUNGEON_GetSquareFirstThing` |
| F0162 | `DUNGEON_GetSquareFirstObject` |
| F0163 | `DUNGEON_LinkThingToList` |
| F0164 | `DUNGEON_UnlinkThingFromList` |
| F0165 | `DungeonOps` (discard-thing route) |
| F0166 | `DUNGEON_GetUnusedThing` |
| F0167 | `DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator` |
| F0169 | `DUNGEON_GetRandomOrnamentIndex` |
| F0170 | `DUNGEON_GetRandomOrnamentOrdinal` |
| F0171 | `DUNGEON_SetSquareAspectRandomWallOrnament` |
| F0172 | `SetSquareAspect` |
| F0173 | `DUNGEON_SetCurrentMap` |
| F0174 | `DUNGEON_SetCurrentMapAndPartyMap` |

### Groups/creatures (F0175-F0212) — `GROUP.C`

| F# | Function |
|---|---|
| F0175 | `GROUP_GetThing` |
| F0176 | `GROUP_GetCreatureOrdinalInCell` |
| F0177 | `GROUP_GetMeleeTargetCreatureOrdinal` |
| F0178 | `GROUP_GetGroupValueUpdatedWithCreatureValue` |
| F0179 | `CreatureAspectUpdateReceipt` |
| F0180 | `GROUP_StartWandering` |
| F0181 | `GROUP_DeleteEvents` |
| F0182 | `GROUP_StopAttacking` |
| F0183 | `GROUP_AddActiveGroup` |
| F0184 | `GROUP_RemoveActiveGroup` |
| F0185 | `GROUP_GetGenerated` |
| F0186 | `GROUP_DropCreatureFixedPossessions` |
| F0187 | `GROUP_DropMovingCreatureFixedPossessions` |
| F0188 | `GROUP_DropGroupPossessions` |
| F0189 | `GROUP_Delete` |
| F0190 | `GROUP_GetDamageCreatureOutcome` |
| F0191 | `GROUP_GetDamageAllCreaturesOutcome` |
| F0192 | `GROUP_GetResistanceAdjustedPoisonAttack` |
| F0193 | `GROUP_StealFromChampion` |
| F0194 | `GROUP_RemoveAllActiveGroups` |
| F0195 | `GroupEventPc34Compat` |
| F0196 | `ActiveGroupSlotPc34Compat` |
| F0197 | `GROUP_IsViewPartyBlocked` |
| F0198 | `GROUP_IsSmellPartyBlocked` |
| F0199 | `GROUP_GetDistanceBetweenUnblockedSquares` |
| F0200 | `GROUP_GetDistanceToVisibleParty` |
| F0201 | `GROUP_GetSmelledPartyPrimaryDirectionOrdinal` |
| F0202 | `GROUP_IsMovementPossible` |
| F0203 | `GROUP_GetFirstPossibleMovementDirectionOrdinal` |
| F0205 | `GROUP_SetDirection` |
| F0206 | `GROUP_SetDirectionGroup` |
| F0207 | `GROUP_IsCreatureAttacking` |
| F0208 | `GROUP_AddEvent` |
| F0209 | `GROUP_ProcessEvents29to41` |
| F0212 | `PROJECTILE_Create` |

### Explosions/projectiles (F0213-F0218) — `PROJECTILE.C`, `PROJEXPL.C`

| F# | Function |
|---|---|
| F0213 | `EXPLOSION_Create` |
| F0216 | `PROJECTILE_GetImpactAttack` |
| F0218 | `PROJECTILE_GetImpactCount` |

### Timeline (F0233-F0240) — `TIMELINE.C`

| F# | Function |
|---|---|
| F0233 | `TIMELINE_Initialize` |
| F0234 | `TIMELINE_IsEventABeforeEventB` |
| F0235 | `TIMELINE_GetIndex` |
| F0236 | `TIMELINE_FixPlacement` |
| F0237 | `TIMELINE_DeleteEvent` |
| F0238 | `TIMELINE_AddEvent` |
| F0239 | `TIMELINE_ExtractFirstEvent` |
| F0240 | `TIMELINE_IsFirstEventExpired` |

### Sensors (F0267-F0276) — `MOVESENS.C`

| F# | Function |
|---|---|
| F0267 | `MOVE_GetMoveResult` |
| F0268 | `SENSOR_AddEvent` |
| F0269 | `SENSOR_AddSkillExperience` |
| F0270 | `SENSOR_TriggerLocalEffect` |
| F0271 | `SENSOR_ProcessRotationEffect` |
| F0272 | `SENSOR_TriggerEffect` |
| F0273 | `SENSOR_GetObjectOfTypeInCell` |
| F0274 | `SENSOR_IsObjectInPartyPossession` |
| F0275 | `SENSOR_IsTriggeredByClickOnWall` |
| F0276 | `SENSOR_ProcessThingAdditionOrRemoval` |

### Champions (F0284-F0349) — `CHAMPION.C`, `INVENTORY.C`

| F# | Function |
|---|---|
| F0284 | `CHAMPION_SetPartyDirection` |
| F0285 | `CHAMPION_GetIndexInCell` |
| F0286 | `CHAMPION_GetTargetChampionIndex` |
| F0287 | `CHAMPION_DrawBarGraphs` |
| F0289 | `CHAMPION_DrawHealthOrStaminaOrManaValue` |
| F0290 | `CHAMPION_DrawHealthStaminaManaValues` |
| F0291 | `CHAMPION_DrawSlot` |
| F0292 | `CHAMPION_DrawState` |
| F0293 | `CHAMPION_DrawAllChampionStates` |
| F0294 | `CHAMPION_IsAmmunitionCompatibleWithWeapon` |
| F0295 | `CHAMPION_HasObjectIconInSlotBoxChanged` |
| F0296 | `CHAMPION_DrawChangedObjectIcons` |
| F0297 | `CHAMPION_PutObjectInLeaderHand` |
| F0298 | `CHAMPION_GetObjectRemovedFromLeaderHand` |
| F0299 | `ApplyObjectModifiersToStatistics` |
| F0300 | `CHAMPION_GetObjectRemovedFromSlot` |
| F0301 | `CHAMPION_AddObjectInSlot` |
| F0302 | `INVENTORY_ProcessCommands28To65_ClickOnSlotBox` |
| F0303 | `CHAMPION_GetSkillLevel` |
| F0304 | `CHAMPION_AddSkillExperience` |
| F0305 | `CHAMPION_GetThrowingStaminaCost` |
| F0306 | `CHAMPION_GetStaminaAdjustedValue` |
| F0307 | `CHAMPION_GetStatisticAdjustedAttack` |
| F0308 | `CHAMPION_IsLucky` |
| F0309 | `CHAMPION_GetMaximumLoad` |
| F0310 | `CHAMPION_GetMovementTicks` |
| F0311 | `CHAMPION_GetDexterity` |
| F0312 | `CHAMPION_GetStrength` |
| F0313 | `CHAMPION_GetWoundDefense` |
| F0314 | `CHAMPION_WakeUp` |
| F0318 | `CHAMPION_DropAllObjects` |
| F0319 | `CHAMPION_Kill` |
| F0320 | `CHAMPION_ApplyAndDrawPendingDamageAndWounds` |
| F0321 | `CHAMPION_AddPendingDamageAndWounds` |
| F0322 | `CHAMPION_Poison` |
| F0323 | `CHAMPION_Unpoison` |
| F0324 | `CHAMPION_DamageAll_GetDamagedChampionCount` |
| F0325 | `CHAMPION_DecrementStamina` |
| F0326 | `CHAMPION_ShootProjectile` |
| F0327 | `CHAMPION_IsProjectileSpellCast` |
| F0328 | `CHAMPION_IsObjectThrown` |
| F0329 | `CHAMPION_AddObjectInLeaderHand` |
| F0330 | `CHAMPION_DisableAction` |
| F0331 | `CHAMPION_InitializeParty` / `CHAMPION_ApplyTimeEffects` |
| F0332 | `INVENTORY_DrawIconToViewport` |
| F0333 | `INVENTORY_OpenAndDrawChest` |
| F0334 | `INVENTORY_CloseChest` |
| F0335 | `INVENTORY_DestroyChest` |
| F0336 | `INVENTORY_DrawPanel_BuildObjectAttributesString` |
| F0337 | `INVENTORY_SetDungeonViewPalette` |
| F0338 | `INVENTORY_DecreaseTorchesLightPower` |
| F0339 | `INVENTORY_DrawPanel_ArrowOrEye` |
| F0340 | `INVENTORY_DrawPanel_ScrollTextLine` |
| F0341 | `INVENTORY_DrawPanel_Scroll` |
| F0344 | `INVENTORY_DrawPanel_FoodOrWaterBar` |
| F0345 | `INVENTORY_DrawPanel_FoodWaterPoisoned` |
| F0346 | `INVENTORY_DrawPanel_Resurrect` |
| F0347 | `INVENTORY_DrawPanel` |
| F0349 | `INVENTORY_ProcessCommand70_ClickOnMouth` |

### Doors / movement / sensor dispatch (F0700-F0730) — Firestaff `_Compat` layer

These are Firestaff-authored bridge functions (not raw ReDMCSB F-numbers)
that wrap `DUNGEON.C`/`MOVESENS.C` logic into a callback-oriented,
testable API. They still carry F-numbers to preserve traceability to the
functional area they compat-wrap.

| F# | Function |
|---|---|
| F0700 | `TriggerImmediateMouseEvent` |
| F0701 | `MOVEMENT_GetStepDelta_Compat` |
| F0702 | `MOVEMENT_TryMove_Compat` |
| F0703 | `ReleaseChampionIcon` |
| F0704 | `MOVEMENT_ResolvePostMoveEnvironment_Compat` |
| F0707 | `SoundDescriptorPc34` |
| F0708 | `MOVEMENT_IsPartyStepBlockedByGroup_Compat` |
| F0711 | `ConvertScanCodeToASCII` |
| F0712 | `AnyKeyboardOrMouseInput` |
| F0715 | `DOOR_ResolveToggleAction_Compat` |
| F0716 | `DOOR_RouteFrontCellClick_Compat` |
| F0718 | `SENSOR_ProcessPartyEnterLeave_Compat` |
| F0720 | `TIMELINE_Init_Compat` |
| F0721 | `TIMELINE_Schedule_Compat` |
| F0722 | `SENSOR_EvaluateFloor_Compat` |
| F0723 | `SENSOR_EvaluateWall_Compat` |
| F0724 | `SENSOR_ResolveEffectDispatch_Compat` |
| F0725 | `SENSOR_ProcessFloorSquare_Compat` |
| F0726 | `SENSOR_ProcessWallClick_Compat` |
| F0727 | `SENSOR_SquareTypeToEventType_Compat` |
| F0728 | `SENSOR_ResolveHoldEffect_Compat` |
| F0729 | `SENSOR_EvaluateWallCountdownEvent_Compat` |
| F0730 | `NextCell_Compat` |

### Music (F0740-F0743) — `MUSIC.C`

| F# | Function |
|---|---|
| F0740 | `MUSIC_Pause` |
| F0741 | `MUSIC_PlayGameMusic` |
| F0742 | `MUSIC_SetTrack` (bound on stairs/teleporter transition) |
| F0743 | `MUSIC_Update` (per-tick driver update) |

Wired into `M11_GameViewState` via `dm1MusicSource` / `dm1MusicState` /
`dm1MusicDriver` fields (see CLAUDE.md). Only the game-won track (C2) is
currently proven playable end-to-end; other tracks remain fail-closed.

### Startup (F0800-F0910) — `STARTUP2.C`, `GROUP.C` (DM1-specific AI mirror), `REVIVE.C`

| F# | Function |
|---|---|
| F0802 | `IsMagicMap` |
| F0803 | `DrawMagicMapIcon` |
| F0804 | `DrawMagicMap` |
| F0805 | `CreatureNameScroll` |
| F0810 | `DM1_GROUP_DispatchBehavior_Compat` |
| F0811 | `DM1_GROUP_IsMovementPossible_Compat` |
| F0812 | `DM1_GROUP_GetFirstPossibleMovementDir_Compat` |
| F0813 | `DM1_GROUP_PickSingleSquareMove_Compat` |
| F0814 | `DM1_GROUP_ShouldAttack_Compat` |
| F0815 | `DM1_GROUP_IsMeleeRange_Compat` |
| F0816 | `DM1_GROUP_ShouldUseProjectile_Compat` |
| F0817 | `DM1_GROUP_SetGroupDirection_Compat` |
| F0818 | `DM1_GROUP_GetDistanceToVisibleParty_Compat` |
| F0819 | `DM1_GROUP_GetSmelledPartyDirOrdinal_Compat` |
| F0820 | `DM1_GROUP_GetFleeDirection_Compat` |
| F0821 | `DM1_GROUP_ShouldFrighten_Compat` |
| F0822 | `DM1_GIGGLER_ResolveStealAttempt_Compat` |
| F0823 | `DM1_GROUP_ResolveProjectileAttack_Compat` |
| F0824 | `DM1_GROUP_ResolveFixedPossessionDrops_Compat` |
| F0833 | `HungerThirstInput_Compat` |
| F0834 | `LIFECYCLE_ClampFoodWater_Compat` |
| F0841 | `LIFECYCLE_ComputeMoveTicks_Compat` |
| F0859 | `LIFECYCLE_Init_Compat` |
| F0860 | `RESURRECTION_ComputeBonesCreation_Compat` |
| F0861 | `RESURRECTION_ShouldTriggerViAltarRebirth_Compat` |
| F0862 | `RESURRECTION_GetChampionIndexFromBones_Compat` |
| F0863 | `RESURRECTION_ComputeRebirthHealth_Compat` |
| F0864 | `RESURRECTION_ComputeReincarnation_Compat` |
| F0865 | `RESURRECTION_IsCommandValid_Compat` |
| F0866 | `RESURRECTION_RouteChampionPortraitClick_Compat` |
| F0867 | `RESURRECTION_ProcessCandidatePanelCommand_Compat` |
| F0868 | `RESURRECTION_RunViAltarFullCycle_Compat` |
| F0869 | `RESURRECTION_DecodeChampionValue_Compat` |
| F0870 | `RESURRECTION_ResetDataToStartGamePlan_Compat` |
| F0871-F0873 | `RESURRECTION_BuildHocMirrorCandidate{Selection,Finalize,Runtime}Receipt_Compat` |
| F0881 | `WORLD_InitDefault_Compat` |
| F0883 | `WORLD_Free_Compat` |
| F0891 | `ORCH_WorldHash_Compat` |
| F0897-F0899 | `WORLD_Serialize_Compat` / `WORLD_Deserialize_Compat` / `WORLD_SerializedSize_Compat` |
| F0902 | `DrawFTLLogo` |
| F0903 | `DrawErrorMessage` |
| F0904 | `PaletteAnimation` |

---

## 3. DUNGEON.DAT File Format

Reference: `include/memory_dungeon_dat_pc34_compat.h`, ReDMCSB `DUNGEON.C`.

The on-disk layout (little-endian, Watcom/Borland LSB-first bitfields) is:

```
Offset 0:   DUNGEON_HEADER            (44 bytes)
Offset 44:  MAP[header.mapCount]      (16 bytes each)
Then:       raw map (square) data, thing arrays, square-first-thing table, text data
```

### Header (44 bytes) — `DungeonHeader_Compat`

| Field | Type | Notes |
|---|---|---|
| `ornamentRandomSeed` | uint16 | Seed for random wall/floor ornament placement (F0169-F0171) |
| `rawMapDataByteCount` | uint16 | Byte size of the packed square-data blob |
| `mapCount` | uint8 | Number of maps (levels) |
| `unreferenced` | uint8 | Padding, unused |
| `textDataWordCount` | uint16 | Word count of the text data section |
| `initialPartyLocation` | uint16 | Bitfield: direction (bits 11-10), y (bits 9-5), x (bits 4-0) — decoded by F0501 |
| `squareFirstThingCount` | uint16 | Number of compact `SquareFirstThings` entries |
| `thingCounts[16]` | uint16[16] | Per-type thing counts (indexed by `THING_TYPE_*`) |

Known dungeon IDs (from `DEFS.H`): `DUNGEON_ID_DM=10`,
`DUNGEON_ID_CSB_PRISON=12`, `DUNGEON_ID_CSB_GAME=13`, `DUNGEON_ID_KID=50`.
A compressed-dungeon (save-game) signature of `0x8104` is explicitly
rejected by the DUNGEON.DAT-only loader.

### Map descriptors (16 bytes each) — `DungeonMapDesc_Compat`

| Field | Notes |
|---|---|
| `rawMapDataByteOffset` | Offset into the raw square-data blob |
| `aUnreferenced`, `bUnreferenced` | Padding |
| `offsetMapX`, `offsetMapY` | Map placement offsets |
| `level`, `width`, `height` | Decoded from bitfield A; width/height are stored value + 1 |
| `rawBitfieldB/C/D` | Raw 16-bit bitfields, kept alongside decoded fields |
| `wallOrnamentCount`, `randomWallOrnamentCount`, `floorOrnamentCount`, `randomFloorOrnamentCount` | Decoded from bitfield B |
| `doorOrnamentCount`, `creatureTypeCount`, `difficulty`, `allowedCreatureTypes[16]` | Decoded from bitfield C |
| `floorSet`, `wallSet`, `doorSet0`, `doorSet1` | Decoded from bitfield D — selects which GRAPHICS.DAT graphic set to bind |

### Square (tile) encoding

`M034_SQUARE_TYPE(square) = square >> 5`. Each square byte packs:

| Mask | Meaning |
|---|---|
| `0xE0` (`DUNGEON_SQUARE_MASK_TYPE`) | 3-bit element type |
| `0x1F` (`DUNGEON_SQUARE_MASK_ATTRIBS`) | 5-bit element-specific attributes |
| `0x10` (`DUNGEON_SQUARE_MASK_THING_LIST`) | Set if the square has an entry in `SquareFirstThings` |

### Element types (0-6)

| Value | Name |
|---|---|
| 0 | Wall |
| 1 | Corridor |
| 2 | Pit |
| 3 | Stairs |
| 4 | Door |
| 5 | Teleporter |
| 6 | Fake wall |

### Thing types (0-15, 16 slots total)

| Value | Type | Bytes on disk |
|---|---|---|
| 0 | Door | 4 |
| 1 | Teleporter | 6 |
| 2 | TextString | 4 |
| 3 | Sensor | 8 |
| 4 | Group | 16 |
| 5 | Weapon | 4 |
| 6 | Armour | 4 |
| 7 | Scroll | 4 |
| 8 | Potion | 4 |
| 9 | Container | 8 |
| 10 | Junk | 4 |
| 11-13 | (unused) | 0 |
| 14 | Projectile | 8 |
| 15 | Explosion | 4 |

Per-type byte counts mirror ReDMCSB's `G0235_auc_Graphic559_ThingDataByteCount`.

### THING encoding (uint16 handle)

```
bits 15:14  direction   (used at runtime only, not in on-disk thing arrays)
bits 13:10  thing type  (0-15, see table above)
bits  9:0   index       into ThingData[type] array
```

Special sentinel values: `THING_NONE = 0xFFFF` (unused slot),
`THING_ENDOFLIST = 0xFFFE` (end of a linked list).

### Thing linked lists

Each square flagged with `DUNGEON_SQUARE_MASK_THING_LIST` has a compact
entry in the `SquareFirstThings` array (one entry per flagged square, not
one per tile — looked up via F0160/F0161). Each decoded thing record
carries a `next` field (a THING handle) forming a singly linked list per
square; walking stops at `THING_ENDOFLIST`. F0163/F0164
(`DUNGEON_LinkThingToList` / `DUNGEON_UnlinkThingFromList`) mutate these
lists; both fail closed if the loaded tables cannot admit the exact
requested mutation, including reproducing the original engine's
`BUG0_08` full-SFT-overflow corruption case for diagnostic purposes
(`F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat`).

### Per-type thing record layouts (decoded structs)

- **Door** (4B): `next`, `type` (1b: set0/set1), `ornamentOrdinal` (4b),
  `vertical` (1b), `button` (1b), `magicDestructible` (1b),
  `meleeDestructible` (1b)
- **TextString** (4B): `next`, `visible` (1b), `textDataWordOffset` (13b)
- **Teleporter** (6B): `next`, `targetMapX/Y` (5b each), `rotation` (2b),
  `absoluteRotation` (1b), `scope` (2b), `audible` (1b), `targetMapIndex` (8b)
- **Sensor** (8B): `next`, `sensorType` (7b, `M039_TYPE`), `sensorData`
  (9b, `M040_DATA`), `onceOnly` (1b), `effect` (2b), `revertEffect` (1b),
  `audible` (1b), `value` (4b), `localEffect` (1b), `ornamentOrdinal` (4b),
  plus a **Remote/Local union** over the last 2 bytes: Remote =
  `targetCell` (2b) + `targetMapX/Y` (5b each); Local = `localMultiple`
  (12b, health multiplier / tick count / kinetic energy depending on
  sensor type)
- **Group** (16B): `next`, `slot` (possession chain), `creatureType` (8b,
  0-26 for DM), `cells` (8b, packs up to 4 creature cell positions),
  `health[4]`, `behavior` (4b), `count` (2b, actual count = value + 1),
  `direction` (2b), `doNotDiscard` (1b)
- **Weapon** (4B): `next`, `type` (7b), `doNotDiscard` (1b), `cursed` (1b),
  `poisoned` (1b), `chargeCount` (4b), `broken` (1b), `lit` (1b, torches)
- **Armour** (4B): `next`, `type` (7b), `doNotDiscard` (1b), `cursed` (1b),
  `chargeCount` (4b), `broken` (1b)
- **Scroll** (4B): `next`, `textStringThingIndex` (10b), `closed` (6b)
- **Potion** (4B): `next`, `power` (8b), `type` (7b), `doNotDiscard` (1b)
- **Container** (8B): `next`, `slot` (contents chain), `type` (2b)
- **Junk** (4B): `next`, `type` (7b), `doNotDiscard` (1b), `cursed` (1b),
  `chargeCount` (2b)
- **Projectile** (8B): `next`, `slot`, `kineticEnergy` (8b), `attack`
  (8b), `eventIndex` (16b)
- **Explosion** (4B): `next`, `type` (7b), `centered` (1b), `attack` (8b)

### Text data encoding

Each 16-bit word packs three 5-bit codes (`code[0]` = bits 14-10,
`code[1]` = bits 9-5, `code[2]` = bits 4-0; bit 15 unused):

| Code | Meaning |
|---|---|
| 0-25 | `'A'`-`'Z'` |
| 26 | Space |
| 27 | Period |
| 28 | Separator (newline / 0x80 depending on context) |
| 29 | Escape — next code indexes the symbol table |
| 30 | Escape — next code indexes the word table (`"THE "`, `"YOU "`, ...) |
| 31 | End of text |

Strings are exposed by index (not content) since the table is
language-dependent. Text types: inscription (0), message (1), scroll (2),
with an optional `0x8000` mask to force decode even if the underlying
TextString thing is marked invisible.

---

## 4. GRAPHICS.DAT Format

DM1 PC 3.4 and CSB PC 3.4 both use GRAPHICS.DAT, but the on-disk
compression differs by platform port in the ReDMCSB source tree:

- **PC IMG3** — the PC 3.4 (DOS) graphic entry format. Entries are
  **LZW-compressed** (ReDMCSB `LZW.C`), decoded through the
  `F0687`-`F0691` chain implemented in Firestaff as
  `redmcsb_f0687_f0688_img3_pc34_compat`, `redmcsb_f0689_img3_expand_pc34_compat`,
  `redmcsb_f0685_img3_line_fill_pc34_compat`, and
  `redmcsb_f0691_draw_compressed_img3_pc34_compat`. Firestaff's DM1 path
  reads GRAPHICS.DAT headers via `memory_graphics_dat_header_pc34_compat.h`,
  which stores parallel `compressedByteCounts` / `decompressedByteCounts`
  arrays per entry — confirming each entry can be independently
  compressed/uncompressed.
- **Amiga IMG1** — CSB's Amiga GRAPHICS.DAT uses a **nibble RLE** scheme,
  distinct from PC IMG3 (see `csb_v1_amiga_graphics_dat.h`,
  `dm1_v1_amiga_graphics_dat.h`, `csb_v1_csbgraphics_dat_lzw_boundary.h`,
  `csb_v1_csbgraphics_dat_classify.h`). This is *not* interchangeable with
  the PC LZW format even though both are called "GRAPHICS.DAT".
- **FM Towns** variants exist for DM2 (`dm2_v1_fmtowns_graphics_dat.h`,
  `csb_v1_fmtowns_graphics_dat.h`). The DM1 FM Towns port ships on a
  BIN/CUE disc image and drives all in-game presentation through the
  TownsOS EGB graphics library and its own Phar Lap P3 executable
  (`EDM.EXP` / `JDM.EXP`); see the dedicated
  [DM1 FM Towns guide](DM1-FMTowns-Guide.md) and the disassembly
  evidence at
  [`parity-evidence/dm1_fmtowns_menu_p3_disassembly.md`](../../parity-evidence/dm1_fmtowns_menu_p3_disassembly.md).

Firestaff's memory-allocator model for GRAPHICS.DAT (`memory_graphics_dat_*`
modules — allocator entry/boundary/orchestrator, defrag loop/entry, used
list, slots, bitmap reuse) mirrors ReDMCSB `MEMORY.C`'s used-list +
defragmentation allocator so that graphic bitmap lifetime and memory
pressure match the original engine's caching behavior (entries can be
evicted and re-decoded from the compressed source, matching the original's
memory-constrained DOS environment).

---

## 5. Save File Format

Reference: ReDMCSB `LOADSAVE.C`, `SAVEHEAD.C`, `SAVEUTIL.C`, `READWRIT.C`;
Firestaff headers `include/memory_savegame_pc34_compat.h`,
`include/redmcsb_f0435_save_tail_pc34_compat.h`,
`include/dm1_v1_original_save_pc34_handoff.h`.

### Obfuscation (F0417/F0418/F0419/F0420)

`F0417_SAVEUTIL_GetChecksumAndObfuscate` (`SAVEUTIL.C`) is the shared
XOR-obfuscation-plus-checksum primitive used across the save format. The
save header uses a minimal subset: **10 uint16 "Noise" entries** are fed
through the F0417 XOR+checksum loop (`SAVEHEAD.C:44,97,104`). The dungeon
tail and other sections reuse the same primitive via `READWRIT.C`
F0417/F0419/F0420.

### Load order (F0429-F0435)

1. `SAVEHEAD.C` **F0429/F0430** — read and deobfuscate/checksum the save
   header.
2. `READWRIT.C` **F0433** — dungeon-tail checksum/obfuscation pass.
3. `LOADSAVE.C` **F0435** — continuation after the header's first three
   parts: reads **EVENTS** (with per-part `Keys[C3]`/`Checksums[C3]`),
   then the uint16 **TIMELINE**, then the dungeon tail proper.
   Firestaff models this as an indexed part sequence:
   `REDMCSB_F0435_PC34_EVENTS_PART_INDEX = 3`,
   `REDMCSB_F0435_PC34_TIMELINE_PART_INDEX = 4`, with distinct failure
   codes (`EVENTS_FAILED = -1`, `TIMELINE_FAILED = -2`,
   `DUNGEON_TAIL_FAILED = -3`) so a byte-identical original save can be
   diagnosed at the exact failing part.

### Save tail layout (matches F0433/F0435)

```
DUNGEON_HEADER
MAP[]
column-cumulative SquareFirstThings counts
SquareFirstThings
text data
thing data for all 16 types
raw map (square) data
checksum
```

Firestaff retains the original tail bytes verbatim
(`originalSaveTailBytes`/`originalSaveTailByteCount`/
`originalSaveTailPristine` in `DungeonDatState_Compat`) until a live world
serializer explicitly replaces them, so `F0433` can round-trip an original
save without lossy decode/re-encode of reserved dungeon fields — this is
the mechanism behind the project's byte-identical DOS savegame
compatibility requirement.

---

## 6. Key Data Structures

### `DungeonThings_Compat` (`include/memory_dungeon_dat_pc34_compat.h`)

Aggregates all decoded thing data for a loaded dungeon:

- `squareFirstThings[]` + `squareFirstThingCount`
- Per-type decoded arrays + counts: `doors`, `textStrings`, `teleporters`,
  `sensors`, `groups`, `weapons`, `armours`, `scrolls`, `potions`,
  `containers`, `junks`, `projectiles`, `explosions`
- `rawThingData[16]` + `thingCounts[16]` — raw byte blobs for all 16
  thing-type slots (kept even for types not yet fully decoded)
- `textData[]` + `textDataWordCount`
- `loaded` flag

### `DungeonMapDesc_Compat`

16-byte on-disk map descriptor, decoded into level/width/height, ornament
counts, difficulty, allowed creature types, and floor/wall/door graphic
set selectors (see §3).

### `DungeonDatState_Compat`

Top-level loader state: `header`, `maps[]`, per-column cumulative
square-first-thing counts (mirrors ReDMCSB `G0280`), `tiles[]` (loaded
lazily by F0502, separate from header/map load by F0500), file size,
loaded/tilesLoaded flags, and the pristine save-tail byte cache described
above.

### Thing type size/count limits

| Constant | Value |
|---|---|
| `DUNGEON_THING_TYPE_COUNT` | 16 |
| `DUNGEON_CREATURE_TYPE_MAX` | 26 (27 types including 0) |
| `DUNGEON_WEAPON_TYPE_MAX` | 45 |
| `DUNGEON_ARMOUR_TYPE_MAX` | 57 |
| `DUNGEON_JUNK_TYPE_MAX` | 52 |
| `DUNGEON_POTION_TYPE_MAX` | 20 |
| `DUNGEON_CONTAINER_TYPE_MAX` | 3 |
| `DUNGEON_MAX_MAPS` | 32 (sanity cap) |

---

## 7. Creature Type Table

DM1 defines `DUNGEON_CREATURE_TYPE_MAX = 26`, meaning creature type indices
run **0-26 inclusive (27 distinct types)**. Per-type attributes (used for
combat resolution, AI behavior gating, and sound selection) are held in
`DM1_V1_F0144_CreatureInfoPc34` records (`include/dm1_v1_creature_attributes_f0144_pc34_compat.h`,
sourced from ReDMCSB `DUNGEON.C` F0144), keyed by `creatureType` (uint8,
0-26). Related lookup tables:

- **Map eligibility**: `dm1_v1_creature_allowed_on_map_f0139_pc34_compat.h`
  (F0139) — cross-references `DungeonMapDesc_Compat.allowedCreatureTypes[16]`
  and `creatureTypeCount` from the map descriptor.
- **AI behavior**: `dm1_v1_creature_ai_behavior_pc34_compat.h`,
  `dm1_v1_creature_behavior_bootstrap_pc34_compat.h` — behavior dispatch
  mirrored in the F0810-F0824 range (movement, melee/projectile attack
  selection, flee/frighten logic, giggler steal-attempt resolution).
- **Rendering**: `dm1_v1_creature_render_pc34_compat.h`,
  `dm1_v1_creature_viewport_pc34_compat.h`,
  `dm1_v1_f0115_f0219_creature_item_material_pc34_compat.h` — viewport
  creature draw path feeding into F0115 (`DUNGEONVIEW_DrawObjectsCreatures`).
- **Sound**: `dm1_v1_creature_sound_pc34_compat.h`.
- **Palette**: `redmcsb_f8160_creature_palette_c25_pc34_compat.h` (F8160,
  creature replacement-color palette, C25) and
  `redmcsb_f0695_set_creature_replacement_colors_pc34_compat.h` (F0695).
- **Name scroll UI**: `redmcsb_f0805_creature_name_scroll_pc34_compat.h`
  (F0805, used by the magic-map / bestiary display).

Groups reference `creatureType` (8-bit field, 0-26) directly in the
on-disk `DungeonGroup_Compat` record (§3), with up to 4 creatures per
group packed via the `cells` and `health[4]` fields.

Note: Firestaff exposes these tables by **index**, not by hardcoded
English name — matching the project's index-first design used for text
strings (§3) so that future localization work is not blocked by
name-string assumptions baked into the reverse-engineered data layer.

---

## 8. Parity Evidence

The `parity-evidence/` directory contains the cross-game pass corpus. The
exact current count is repository-derived rather than maintained as a
narrative number; the DM1-specific subset is identified by
`pass{NNN}_dm1_v1_*` filenames,
following the naming convention `pass{NNN}_{description}.md` described in
CLAUDE.md.

Each pass document is a source-lock evidence record: it ties a specific
Firestaff implementation change to the exact ReDMCSB source lines it
reproduces, and typically records the `pc34_compat` module(s) touched.
Examples visible in the current tree span the full breadth of subsystems
covered above — inventory slot placement and drop order (`pass1091`,
`pass863`), viewport occlusion gating (`pass404`), initial-hall capture
paths (`pass466`), door/entrance graphics (`pass840`), palette and cursor
bitmap handling (`pass850`-`pass858`, `pass870`-`pass871`), mandatory
graphic indices (`pass852`), wound defense factor (`pass853`), champion
rename/reincarnate character-string handling (`pass854`-`pass856`), square
type → event type mapping (`pass859`, matching `F0727` above), champion
icon and bar-graph byte offsets (`pass860`-`pass862`), and box
screen-edge layout constants (`pass864`-`pass866`).

This evidence corpus is the practical cross-reference for any future
reverse-engineering work: before re-deriving behavior from ReDMCSB source,
check whether a `parity-evidence/pass*_dm1_v1_*` document already records
the exact lines and the Firestaff module that encodes them.
