# DM1 V1 Open Door projectile door-impact source lock

Task: spell-open-door-poison-fizzle-20260520
Slice selected: Open Door projectile full non-destroyed door behavior. Poison cloud DOT and spell-failure feedback were not touched.

## ReDMCSB evidence anchors

- Toolchains/Common/Source/MENU.C:F0412_MENUS_GetChampionSpellCastResult lines 1861-1870: projectile spells route through F0327; Open Door doubles skill before kinetic-energy clamp and creates C0xFF84 via spell type + C0xFF80.
- Toolchains/Common/Source/CHAMPION.C:F0327_CHAMPION_IsProjectileSpellCast lines 2073-2103: projectile spell cast spends mana, derives step energy, and calls F0326_CHAMPION_ShootProjectile.
- Toolchains/Common/Source/PROJEXPL.C:F0217_PROJECTILE_HasImpactOccured door branch lines 471-489: for impact type C04_ELEMENT_DOOR, C0xFF84_THING_EXPLOSION_OPEN_DOOR is handled before normal destroyed/open/pass-through door tests; when DOOR->Button is set it schedules F0268_SENSOR_AddEvent(C10_EVENT_DOOR, x, y, 0, C02_EFFECT_TOGGLE, GameTime+1), then breaks with no door-destruction path.
- Toolchains/Common/Source/PROJEXPL.C:F0217_PROJECTILE_HasImpactOccured lines 491-508: non-Open-Door projectiles pass destroyed/open/one-quarter doors or eligible pass-through doors, otherwise roll door-destruction attack.
- Toolchains/Common/Source/GROUP.C lines 1712-1748: spell-casting creatures can create C0xFF84_THING_EXPLOSION_OPEN_DOOR, so behavior is not champion-only.

## Local implementation notes

- src/memory/memory_projectile_pc34_compat.c now treats PROJECTILE_SUBTYPE_OPEN_DOOR as a hit against every non-destroyed door state, including open and one-quarter doors, before the normal pass-through result can turn it into PROJECTILE_RESULT_FLEW.
- Open Door door hits still never emit a door-destruction event; they emit a delayed C10/C02 toggle only when digest->destDoorHasButton is true and always emit the wooden-thud impact code.
- Destroyed doors remain pass-through for Open Door.
