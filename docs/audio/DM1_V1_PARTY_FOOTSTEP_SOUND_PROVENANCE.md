# DM1 V1 party footstep sound provenance

Status: source-locked absence

## Conclusion

ReDMCSB has no source-backed DM1 V1 party footstep sound trigger. The official source-backed movement-adjacent sounds are blocked-party damage, audible teleporters, pit-fall screams, and creature movement sounds. Footprints are graphics/event state driven by scent and the C79 footprint event, not an audio path.

Treat party footsteps as a non-V1/procedural audio decision unless a stronger original-source anchor is found. Do not invent V1 playback from memory, emulator feel, or unrelated creature movement sounds.

## Audited source

Primary source tree:
`/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

Required files inspected: `COMMAND.C`, `MOVESENS.C`, `GROUP.C`, `SOUND.C`, `TIMELINE.C`, `DUNGEON.C`, `DATA.C`, `DEFS.H`. `CLIKMENU.C` is included because `COMMAND.C` dispatches party movement to `F0366_COMMAND_ProcessTypes3To6_MoveParty` there.

## Source evidence

### Party movement path

- `COMMAND.C:2150-2156` dispatches turn commands to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and movement commands `C003_COMMAND_MOVE_FORWARD..C006_COMMAND_MOVE_LEFT` to `F0366_COMMAND_ProcessTypes3To6_MoveParty`; this dispatch contains no sound request.
- `CLIKMENU.C:256-269` converts the move command into a relative movement vector with `G0465_ai_Graphic561_MovementArrowToStepForwardCount`, `G0466_ai_Graphic561_MovementArrowToStepRightCount`, and `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`.
- `CLIKMENU.C:271-329` handles stairs, wall/door/fake-wall/group blocking, then commits legal party movement through `F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, ...)`.
- `CLIKMENU.C:330-347` only computes movement cooldown ticks (`F0310_CHAMPION_GetMovementTicks`, `G0310_i_DisabledMovementTicks`, `G0311_i_ProjectileDisabledMovementTicks`) after a legal move; there is no `F0064_SOUND_RequestPlay_CPSD`, `F0060_SOUND_Play_CPSX`, or `F0709_StartSound` on the successful party-step path.
- `CLIKMENU.C:291-309` is the movement-specific party sound exception: blocked wall/door/fake-wall impact can request `M562_SOUND_PARTY_DAMAGED`. That is collision damage, not a footstep.
- `MOVESENS.C:441-451` updates `G0306_i_PartyMapX`, `G0307_i_PartyMapY`, and party direction context for party movement inside `F0267_MOVE_GetMoveResult_CPSCE`; no generic movement sound is requested there.
- `MOVESENS.C:493-498` requests `M560_SOUND_BUZZ` only for an audible teleporter during party movement.
- `MOVESENS.C:575-603` requests `M561_SOUND_SCREAM` only when pit falling damages the party.
- `MOVESENS.C:811-819` processes party arrival sensors; no party footstep sound is requested.

### Footprint event/graphics path

- `DEFS.H:855-865` stores footprint state in `G0407_s_Party.Event79Count_Footprints`, scent counters, and scent arrays.
- `MOVESENS.C:764-783` updates party scents on movement and uses `Event79Count_Footprints` to advance `LastScentIndex`; this block has no sound request.
- `DUNGEON.C:2631`, `DUNGEON.C:2648`, `DUNGEON.C:2664-2668`, and `DUNGEON.C:2696-2718` decide whether footprints are allowed for square rendering.
- `DUNGEON.C:2716-2719` turns footprint visibility into graphics by setting `MASK0x8000_FOOTPRINTS` on `M558_FLOOR_ORNAMENT_ORDINAL` when the current square scent falls inside the active footprint range.
- `TIMELINE.C:1998-1999` expires `C79_EVENT_FOOTPRINTS` by decrementing `G0407_s_Party.Event79Count_Footprints`; it does not request sound.

### Creature movement sound path

- `MOVESENS.C:847-849` requests a movement sound only in the group/creature branch, using `F0514_MOVE_GetSound(...)` and `C01_MODE_PLAY_IF_PRIORITIZED`.
- `GROUP.C:267-280` has a separate creature-animation movement sound path for Couatl movement/bitmap flipping, also routed through `F0514_MOVE_GetSound`/`F0064_SOUND_RequestPlay_CPSD` for I34E-family builds.
- `MOVESENS.C:984-995` shows the I34E-era `F0514_MOVE_GetSound` path: it returns a creature movement sound from `G2003_aauc_CreatureSounds[...][C1_MOVEMENT_SOUND]` based on creature type, or `-1` if resting/no movement sound applies. This function is creature-oriented and is not called by the successful party-step path.

### Sound request/flush path

- `DEFS.H:6834-6844` declares `F0064_SOUND_RequestPlay_CPSD(soundIndex, mapX, mapY, mode)`.
- `SOUND.C:1476-1544` implements request scheduling and delayed `C20_EVENT_PLAY_SOUND` creation.
- `SOUND.C:1608-1642` plays immediate sounds or records pending/prioritized requests depending on mode/platform.
- `SOUND.C:1756-1850` flushes pending sounds through `F0065_SOUND_PlayPendingSound_CPSD` and `F0709_StartSound`/`F0060_SOUND_Play_CPSX`.
- `TIMELINE.C:1903-1905` processes delayed `C20_EVENT_PLAY_SOUND` by calling `F0064_SOUND_RequestPlay_CPSD`.

### I34E sound table/constants

- `DEFS.H:58-59` sets `M513_SOUND_COUNT` to 35 for the `MEDIA485_P20JB_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J` family.
- `DEFS.H:100-128` lists I34E game constants. They include base sounds, attack sounds, and creature movement sounds (`C28_SOUND_MOVE_*` through `C34_SOUND_MOVE_SKELETON`), but no party footstep/walk/step constant.
- `DATA.C:1264-1310` materializes the I34E-family `G0060_as_Graphic562_Sounds` table. Lines `1304-1310` are explicitly the seven creature movement sounds; there is no party footstep entry.

## Search result

The provenance probe searches the audited files for foot/footprint/step/walk/move/sound request patterns, verifies the exact source anchors above, verifies that the successful party-step path has no sound request/play call, and verifies that the only movement sound table entries are creature movement entries.

Verification command:

```sh
python3 tools/verify_dm1_v1_party_footstep_sound_provenance.py
```
