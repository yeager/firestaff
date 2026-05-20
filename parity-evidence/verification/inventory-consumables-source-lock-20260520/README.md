# DM1 V1 Inventory Consumables Source Lock

Task: `inventory-consumables-source-lock-20260520`.

## ReDMCSB Evidence Anchors

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `PANEL.C:1743-1785` — `F0349_INVENTORY_ProcessCommand70_ClickOnMouth` local state for mouth consumption.
- `PANEL.C:1824-1844` — mouth-allowed object gate, water/waterskin charge gate, and leader-hand removal choice.
- `PANEL.C:1850-1917` — potion effect switch: ROS/KU/DANE/NETA/ANTIVENIN/MON/YA/EE/VI/water flask, then `Type = C20_POTION_EMPTY_FLASK` without clearing `Power`.
- `PANEL.C:1918-1919` — food icons `C168..C175` add `G0242_ai_Graphic559_FoodAmounts[iconIndex - C168]`, capped at 2048.
- `PANEL.C:1922-1945` — health/stamina clamps and `C08_SOUND_SWALLOW` after successful consumption.
- `DUNGEON.C:428-436` — `G0242_ai_Graphic559_FoodAmounts`: apple 500, corn 600, bread 650, cheese 820, screamer slice 550, worm round 350, drumstick/shank 990, dragon steak 1400.
- `DUNGEON.C:1108-1127` — junk weight uses waterskin charge count, and empty flask weighs 1 while non-empty potion weighs 3.
- `DEFS.H:1468-1481` — potion type constants, including `C15_POTION_WATER_FLASK` and `C20_POTION_EMPTY_FLASK`.
- `DEFS.H:1517-1524` — junk type constants for waterskin and food drops.
- `DEFS.H:1891-1947` — water/waterskin and food icon constants.

## Implemented Scope

- Added a pure DM1 V1 PC34 consumables helper for potion drinking, water/waterskin drinking, food eating, source food amounts, water/food cap 2048, waterskin charge decrement, and potion conversion to empty flask type 20 while preserving potion power.
- Wired the M11 inventory mouth-click path through that helper for runtime click-to-eat/drink behavior.
- Left fuller UI animation, swallow audio routing, and exact VI wound RNG sourcing as pending runtime polish; the helper accepts deterministic wound masks and the click path currently uses no RNG masks.

## Verification Outputs

- `probe-output.txt` — deterministic helper probe.
- `ctest-output.txt` — targeted CTest run for consumables and existing mouth/eye route gate.
