# DM2 V1 Companion AI — Party Companions & Minion Behavior

## Source Evidence
- `skproject/SKULLWIN/c_ai.cpp` — `DM2_THINK_CREATURE` (NPC/companion planning)
- `skproject/SKULLWIN/c_creature.cpp` — merchant/shop creature types (0x17-0x2B)
- `skproject/SKULLWIN/c_creature.h` — creature struct fields
- `src/dm2/dm2_v1_companion.c` — Firestaff companion state struct
- `src/dm1/` — DM1 has NO companion system (no party members)

## No DM1 Equivalent
DM1 **has no companion/minion system**. The party is always 4 human-controlled champions. No NPC followers, no loyalty, no companion inventories.

DM2 introduces:
- **NPC party companions** — recruited creatures with own health, loyalty, inventory
- **Trading** — companions can trade items with champions
- **Loyalty tracking** — `loyalty` value (0-100) affects whether companion stays or leaves
- **Companion portrait/UI** — V2 UI shows portrait, health bar, loyalty bar

## Companion Struct
```c
// From dm2_v1_companion.c — Firestaff implementation
typedef struct {
    char name[16];
    int health, max_health;
    int attack, defense;
    int loyalty;        // 0-100, loyalty meter
    int ai_behavior;    // behavior flags (follow / fight / idle)
    int alive;          // 0 = dead / dismissed
} DM2_V1_Companion;

// SKULL.ASM evidence: NPC companion AI, loyalty, trading
// SKULL.ASM evidence: DM2 feature: party companions with own inventory
```

## Companion AI Behavior
Firestaff companion tick is a stub:
```c
void dm2_v1_companion_tick(DM2_V1_CompanionState *state) {
    /* AI tick: companions follow party or fight nearby enemies */
    (void)state;
}
```

Two intended behavioral modes:
1. **Follow party** — companion moves to stay near party leader
2. **Fight nearby enemies** — companion engages threats within range

## Loyalty System
Loyalty (0-100) in DM2:
- Decreases when a champion party member dies nearby
- Decreases if companion is not fed/restored
- Affects whether companion abandons the party
- No equivalent in DM1

## Minions / Summoned Creatures
DM2 has `DM2_CREATURE_EXPLODE_OR_SUMMON` (action 0x26-0x28):
- Creature self-destructs and spawns a minion at its position
- Or summons a creature from the dungeon record
- No DM1 equivalent — DM1 had no summoning

## Group Behavior for Companions
Unlike DM1 group coordination (shared movement cells, leader-follower), DM2 companions are treated as individual party members. Group behavior is handled at the **party level** (follow the party leader), not the creature-group level.

## Open Implementation Gaps
- Companion tick is a stub — no actual pathfinding or follow logic implemented
- Companion loyalty decay is not implemented
- Companion trading is not implemented in the V1 codebase
- SKULL.ASM companion AI not yet reverse-engineered into state machine code
