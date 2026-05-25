# DM2 V1 Spell Casting Changes — Source Locked

**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 17521-17900 (CAST_SPELL_PLAYER)
**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 18150-18200 (ADD_RUNE_TO_TAIL)
**Source:** `skproject/SKWIN/SkGlobal.cpp` lines 966-1011

## Mana System

### Rune-Based Mana Cost

Each rune symbol costs mana to add to the spell sequence:

```cpp
// SkWinCore.cpp:18159-18174 (ADD_RUNE_TO_TAIL)
U16 di = RuneManaPower[si * 6 +symbol_0to5];
if (si != 0) {  // if not on POWER rune
    di = (RunePowerMultiplicator[i8(champion->GetRunes()[0]) -RUNE_FIRST] * di) >> 3;
}
if (champion->curMP() >= di) {
    champion->curMP(champion->curMP() -di);
    // rune added to sequence
}
```

Mana is deducted when each rune is added, not at cast time. Power rune (first rune) has no mana cost — subsequent runes multiply the base cost.

### Mana Display

```cpp
// SkWinCore.cpp:31732
sprintf((char*)message, "HP %d / %d | STA %d / %d | MANA %d / %d\n", ...);
```

Shown as current/max in character panel.

### Mana from Items

```cpp
// SkWinCore.cpp:5310 (BOOST_ATTRIBUTE)
iBonusModifier = RETRIEVE_ITEM_BONUS(di, GDAT_ITEM_BONUS_MANA, bp0a, si);
// +MANA modifier applied to champion->manaMax
```

Item bonus type `GDAT_ITEM_BONUS_MANA` increases maximum mana.

## Cast Chance Mechanics

From `SkWinCore.cpp:17535-17555` (CAST_SPELL_PLAYER):

```cpp
U16 bp0e = (ref->w6_a_f() * (power +18)) / 24;  // skill decay amount
U16 bp08 = ref->difficulty + power;               // global difficulty
U16 bp0c = 0
    + ((RAND() & 7) + (bp08 << 4))
    + ((ref->difficulty * (power - 1)) << 3)
    + (bp08 * bp08);

U16 bp06 = QUERY_PLAYER_SKILL_LV(player, ref->requiredSkill, 1);
```

Cast loop: `for (bp0a = bp08 - bp06; (bp0a--) > 0; )`
- Roll against `min(WizardAbility + 15, 115)`
- If fail: skill penalty applied, spell fails (goto _26fe)
- If succeed for all difficulty loops: spell succeeds

**Power rune effect**: The first rune (Power rune) determines spell power. Higher power = easier cast but more skill decay.

### Skill Decay on Failure

```cpp
// SkWinCore.cpp:17544
ADJUST_SKILLS(player, ref->requiredSkill, bp0c << (bp08 - bp06));
```

Failed spells penalize the relevant skill. Higher difficulty spells penalize more.

## Cooldown System

```cpp
// SkWinCore.cpp:17623
if (bp0e != 0) {
    ADJUST_HAND_COOLDOWN(player, bp0e, 2);
}
```

Each successful cast applies a cooldown to the hand used. The amount (`bp0e`) scales with spell difficulty and power.

## Spell Type Routing

```cpp
// SkWinCore.cpp:17563
switch (ref->w6 & 15) {
    case SPELL_TYPE_POTION:    // 1
        // requires empty flask in hand
    case SPELL_TYPE_MISSILE:   // 2
        // shoots projectile
    case SPELL_TYPE_SUMMON:    // 4
        // creates minion
    case SPELL_TYPE_GENERAL:   // 3
        // light, enchantments, auras
}
```

Each spell type has distinct execution path.

## Failure Messages

From `ReDMCSB_WIP20210206/Toolchains/Common/Source/SPELFAIL.C`:

| Failure Type | Message |
|---|---|
| C00_FAILURE_NEEDS_MORE_PRACTICE | "NEEDS MORE PRACTICE WITH THIS [SKILL] SPELL" |
| C01_FAILURE_MEANINGLESS_SPELL | "MUMBLES A MEANINGLESS SPELL" |
| C10_FAILURE_NEEDS_FLASK_IN_HAND | "NEEDS AN EMPTY FLASK IN HAND FOR POTION" |
| C11_FAILURE_NEEDS_MAGIC_MAP_IN_HAND | "NEEDS A MAGIC MAP IN ACTION HAND FOR THIS SPELL" (DM2 extended only) |

## Changes from DM1

1. **Mana per rune**: DM2 uses RuneManaPower table for per-symbol mana cost
2. **Cooldowns**: DM2 introduces hand cooldowns after spell casting (DM1: none)
3. **Skill decay on failure**: DM2 has explicit skill penalty mechanism
4. **Power rune multiplier**: First rune modifies subsequent rune mana costs
5. **Extended spell mode**: DM2 supports loading custom spells from GDAT (255 max)

