# DM1 V1 Champion Record Data — Source Audit

## ReDMCSB Source
- DEFS.H:659–705: CHAMPION_INCLUDING_PORTRAIT and CHAMPION_EXCLUDING_PORTRAIT typedefs
- DEFS.H:707–735: Champion constants (statistics, skills, slots, wounds)
- DEFS.H:837–870: PARTY_INFO struct

## CHAMPION Struct (DEFS.H:659–705, PC 3.4 = EXCLUDING_PORTRAIT)
~324 bytes (without portrait):
- Name[8], Title[20] (packed, no NUL)
- Direction(1), Cell(1), Useless1(1), Useless2(1)
- ActionIndex(1), SymbolStep(1), Symbols[5], aUnreferenced(1)
- DirectionMaximumDamageReceived(1), MaximumDamageReceived(1), PoisonEventCount(1), bUnreferenced(1)
- EnableActionEventIndex(2), HideDamageReceivedEventIndex(2)
- Attributes(2), Wounds(2)
- CurrentHealth(2), MaximumHealth(2), CurrentStamina(2), MaximumStamina(2), CurrentMana(2), MaximumMana(2)
- ActionDefense(2), Food(2), Water(2) [Food/Water: int16_t, range -1024 to +2048]
- Statistics[7][3]: 7 attrs x {MIN=0, CUR=1, MAX=2} = 21 bytes + 1 pad = 22 bytes
- Skills[20]: 20 * 6 bytes (SKILL = { uint16_t TemporaryExperience; long Experience; })
- Slots[30]: 30 THING values (inventory)
- Load(2), ShieldDefense(2), Portrait*(4), [CSB-only fields], cUnreferenced[34]

## Wounds Bitfield
MASK0x0001_WOUND_READY_HAND(0x01), MASK0x0002_WOUND_ACTION_HAND(0x02),
MASK0x0004_WOUND_HEAD(0x04), MASK0x0008_WOUND_TORSO(0x08),
MASK0x0010_WOUND_LEGS(0x10), MASK0x0020_WOUND_FEET(0x20)

## Statistics Array
C0_STATISTIC_LUCK(0) through C6_STATISTIC_ANTIFIRE(6), C0_MAXIMUM(0)/C1_CURRENT(1)/C2_MINIMUM(2)

## Firestaff Implementation
File: include/memory_champion_state_pc34_compat.h
All core fields aligned: name/title/portraitIndex/attributes/wounds/hp/stamina/mana/food/water/
skillLevels/skillExperience/inventory/load/direction/mirror text.

Known design simplifications:
- Statistics[7][3] stored as attributes[6] (6 current values only) — HP/Stamina/Mana max used instead
- Skills[20] stored as skillLevels[4] + skillExperience[4] (4 base class skills only)
- Magic map fields (CSB-only) not stored

STATUS: PARTIALLY ALIGNED — Core data fully aligned. Statistics triple-row and 20-skill array simplified. No corruption risk.
