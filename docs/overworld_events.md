# DM1 V1 World Events — Source Locked

## Finding: No Time-Based World Events Outside Dungeon View

DM1 V1 has no separate world layer with time-based events. The game world
exists only as the current dungeon map (or the entrance screen). All events
occur within that context.

## Timeline System (In-Dungeon)

Source: TIMELINE.C — The game uses an event timeline that is processed
each game tick:
- Events are stored with a map index + time
- Events fire when G0313_ul_GameTime reaches the event's scheduled time
- Timeline is only active while G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE

## Game Time Advance

Source: GAMELOOP.C — G0313_ul_GameTime increments by 1 per game tick
(~20ms real-time, variable). It pauses when:
- Game is paused (menu open)
- Game is in the entrance/credits screen
- The system is not in the play state

Source: memory_tick_orchestrator_pc34_compat.h:
  // time only advances in play scope, not during UI/menu/transition
  // partyMapIndex updates drive map transitions, not time itself

## Event Types (C22 Event)

Source: NEWMAP.C:28-36 — Event 22 (C22_EVENT_CPSE) is triggered when the
party enters a new map via F0003_MAIN_ProcessNewPartyMap_CPSE:
  if ((G0313_ul_GameTime - G0418_l_LastEvent22Time_CPSE) > 500)
      G0418_l_LastEvent22Time_CPSE = G0313_ul_GameTime + 1
      L0000_s_Event_CPSE.Type = C22_EVENT_CPSE
      M033_SET_MAP_AND_TIME(L0000_s_Event_CPSE.Map_Time, mapIndex, G0418...)
      F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0000_s_Event_CPSE)

This is the only per-map-entry event in the core game loop.

## Entrance/World Events

The entrance screen has no time-driven world events. It shows static art
(C004_GRAPHIC_ENTRANCE) with animated door and button elements:
- C200_COMMAND_ENTRANCE_ENTER_DUNGEON — enter dungeon
- C201_COMMAND_ENTRANCE_ENTER_BONUS_DUNGEON — bonus dungeon
- M567_COMMAND_ENTRANCE_DRAW_CREDITS — credits

Source: ENTRANCE.C — no timeline integration, no world time advance.

## DM2 v2 World Events (Reference Only)

For comparison, DM2 (v2 engine, different game) implements:
- Weather system (rain, fog, storm)
- Day/night cycle
- World-map travel events

These are absent from DM1 V1.

## Event Scope Summary

| Event Type         | Location         | Time-driven? |
|--------------------|------------------|--------------|
| Dungeon timeline   | Current map      | Yes (GameTime) |
| C22 map-entry event| Current map      | Yes (GameTime) |
| Sound events       | Proximity-based  | No           |
| Sensor triggers    | Square/step      | No           |
| Entrance display   | Map 255          | No (static)  |
| World map events   | None             | N/A          |
