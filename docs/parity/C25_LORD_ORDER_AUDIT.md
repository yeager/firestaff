# C25 Lord Order — Archenemy Behavior Audit

**Date:** 2026-05-24
**Scope:** DM1 V1 creature AI — C23 Lord Chaos & C25 Lord Order
**Source:** ReDMCSB WIP 20210206 + Firestaff PC34 flattened amalgam

---

## 1. Profile Table Status (memory_creature_ai_pc34_compat.c)

Both C23 and C25 are `CREATURE_IMPL_TIER_STUB` — archenemy behavior is deferred.

| Field | C23 Lord Chaos | C25 Lord Order |
|-------|---------------|---------------|
| Level | 5 | 5 |
| Wariness | 10 | 10 |
| Health | 8 | 8 |
| Attack | 70 | 70 |
| Defense | 60 | 60 |
| Damage | 200 | 200 |
| Dexterity | 60 | 60 |
| Attack Mode | COMBAT_ATTACK_MAGIC | COMBAT_ATTACK_MAGIC |
| Attributes | CREATURE_ATTR_MASK_ARCHENEMY | CREATURE_ATTR_MASK_ARCHENEMY |
| Movement | 80 | 80 |
| Impl Tier | STUB | STUB |

Identical profiles — both marked ARCHENEMY flag, both stubs.

---

## 2. Movement / AI Behavior

### Archenemy-flagged movement logic

`MASK0x2000_ARCHENEMY` check implemented in movement gate at amalgam line 2902:

```c
if (M007_GET(P0400_ps_CreatureInfo->Attributes, MASK0x2000_ARCHENEMY)) {
    // Check for Fluxcage squares — archenemy cannot enter them
}
```

Applies to both C23 and C25 since they share the same attribute bit.

### Lord Chaos-specific teleport behavior

`F0223_GROUP_IsLordChaosAllowed()` (line 1987) checks if target square is corridor, pit, door, or teleporter. Called in three places for Lord Chaos:

1. Line 4585 — Random direction retry after failed teleport (Lord Chaos only)
2. Line 13746 — Fuse action portal placement validation

### No equivalent Lord Order teleport function

The amalgam contains **no** `F0223_GROUP_IsLordOrderAllowed` or similar function. Lord Order has no dedicated teleport/movement logic beyond the generic ARCHENEMY Fluxcage gate check.

ReDMCSB MOVESENS.C confirms: C25 appears only in the sound silence list (line 932, grouped with C23 and C26 for rest-sound return). No C25-specific movement cases exist anywhere in ReDMCSB source.

---

## 3. Attack / Spell Casting — BUG0_13 Analysis

### The bug

From amalgam line 3101 (ReDMCSB GROUP.C):

> Lord Order and Grey Lord creatures can cast spells (attack range > 1) but no projectile type is defined for them in the code. If these creatures are present in a dungeon they will cast projectiles containing undefined things because the variable is not initialized.

### Trigger path

Spell-cast switch at line ~3058:

```c
if ((M056_ATTACK_RANGE(L0441_ps_CreatureInfo->Ranges) > 1) && ...) {
    switch (AL0437_ui_CreatureType) {
        default: AL0437_T_Thing = C0xFF80_THING_EXPLOSION_FIREBALL; break;
        case C14_CREATURE_VEXIRK:
        case C23_CREATURE_LORD_CHAOS:
            // multi-projectile spell logic with randomized types
            break;
        case C01_CREATURE_SWAMP_SLIME: ...
        case C03_CREATURE_WIZARD_EYE: ...
        case C19_CREATURE_MATERIALIZER:
        case C22_CREATURE_DEMON:
        case C24_CREATURE_RED_DRAGON:
            AL0437_T_Thing = C0xFF80_THING_EXPLOSION_FIREBALL;
    } /* BUG0_13 comment */
    F0212_PROJECTILE_Create(AL0437_T_Thing, ...);
}
```

C25 Lord Order falls through to `default: FIREBALL`. This is a defined fallback, not an undefined value. The crash scenario in BUG0_13 describes the variable being uninitialized — but in the amalgam, `AL0437_T_Thing` is always set by one of the switch cases (or default), so it is never truly uninitialized at the `F0212_PROJECTILE_Create` call.

### Verdict on BUG0_13

- **In original DM1:** Cannot trigger — no original dungeon contains C25 or C26 groups. They are endgame-only creatures.
- **In Firestaff:** The crash path is structurally present in the original code. The amalgam preserves the same switch logic. Since C25/C26 are not placed in any Firestaff content either, the bug is dormant. If custom dungeons placed C25/C26 groups, the default FIREBALL fallback would apply (defined value), so the crash described in BUG0_13 (undefined projectile) would not occur — the crash described is specifically about uninitialized data, which the default case prevents.

### C25 vs C23 spell behavior deviation

C23 Lord Chaos gets randomized multi-spell (fireball/lightning/poison/harm). C25 Lord Order always gets fireball. This is a behavioral gap if C25 is ever used in custom content.

---

## 4. Archenemy Freeze Life Immunity

Line 3252:

```c
if (G0407_s_Party.FreezeLifeTicks && !L0463_B_Archenemy) {
    // reschedule event later
    // Lord Chaos (archenemy) is immune to Freeze Life
}
```

Both C23 and C25 have `MASK0x2000_ARCHENEMY`, so both are immune to Freeze Life. Correct per ReDMCSB.

---

## 5. Endgame Fuse Sequence (ENDGAME.C)

ReDMCSB ENDGAME.C lines 894-910:

1. Lord Chaos (C23) placed in front of party
2. Fireball explosions cycle
3. C23 → C25 Lord Order (type swap)
4. Harm Non-Material explosions cycle
5. Alternating C23/C25 flash (5 cycles)
6. Final explosions → C26 Grey Lord
7. Game-won state

Firestaff has no equivalent endgame code — expected since DM1 V1 is movement/collision only. The amalgam preserves `F0224_GROUP_FluxCageAction` and `F0225_GROUP_FuseAction` for reference but they are not called in the V1 game loop.

---

## 6. Rendering (dm1_v1_creature_render_pc34_compat.c)

Line 71: `{ 85, 795, 0x14, 0xCB, 0x78AA }` — C25 uses the same graphic slot as C26 (offset 85). Matches ReDMCSB DUNVIEW.C which shows C26 using slot 86, C25 using slot 85.

Lord Order rendering data exists and is fully implemented (not stub).

---

## 7. Grey Lord (C26) — BUG0_13 also applies

C26 Grey Lord also has `COMBAT_ATTACK_MAGIC` and no projectile type defined in the spell switch. Same BUG0_13 applies. C26 appears in the endgame fuse sequence (line 910: type becomes C26 after final explosions). In original dungeons, C26 is never placed as a creature group — only appears in the fuse sequence. Same dormancy as C25.

---

## 8. Summary Table

| Behavior | C23 Lord Chaos | C25 Lord Order | C26 Grey Lord |
|----------|---------------|---------------|---------------|
| Profile | STUB | STUB | STUB |
| Movement | Partial (teleport) | Generic only | Generic only |
| Attack | Multi-spell (randomized) | Default FIREBALL | Default FIREBALL |
| BUG0_13 | Dormant | Dormant | Dormant |
| Freeze Life immunity | YES | YES | No (not ARCHENEMY) |
| Fluxcage blocking | YES | YES | No |
| Endgame fuse | YES (type-switch) | YES (type-switch) | YES (final form) |
| Rendering | Full | Full | Full |

---

## 9. Gaps Found

1. **No Lord Order-specific teleport logic.** Lord Order could theoretically teleport (like Lord Chaos) but no `IsLordOrderAllowed` function exists. May be intentional (static archenemy) or unimplemented gap.

2. **C25 attack falls to default FIREBALL** instead of C23's randomized multi-spell. Behavioral deviation if C25 used in custom content.

3. **C26 Grey Lord also vulnerable to BUG0_13** but equally dormant in original content.

---

## 10. Conclusion

- **C23 Lord Chaos:** Partially implemented (teleport, multi-spell, freeze immunity) — core archenemy mechanics present despite STUB classification.
- **C25 Lord Order:** Minimal implementation — generic ARCHENEMY behavior only, no teleport, defaults to fireball. Behavioral deviation from C23's capabilities.
- **BUG0_13:** Dormant in all original content; structurally present but effectively non-triggering due to default fallback and absence of C25/C26 in any dungeon data.
- **Recommendation:** If C25 Lord Order is used in custom dungeons, the spell-type switch should include C25 with the same randomized multi-spell logic as C23. Lord Order teleport function may also be needed depending on design intent.