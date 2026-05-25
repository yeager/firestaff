# GAP C25/C26: Lord Order and Grey Lord Creature Handler Gap

## Status
**GAP — Latent handler gap, safe for normal play, modders need explicit handler**

## Affected Creatures
- `DM1_CREATURE_TYPE_LORD_CHAOS` (C23, index 23) — **has explicit handler**
- `DM1_CREATURE_TYPE_LORD_ORDER` (C25, index 25) — **NO explicit handler**
- `DM1_CREATURE_TYPE_GREY_LORD`  (C26, index 26) — **NO explicit handler**

## Source Location
`src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c`, function `F0823_DM1_GROUP_ResolveProjectileAttack_Compat` (line 228).

## Gap Description

F0823 dispatches creature projectile attacks via a `switch(ctx->creatureType)`.

**C01–C24 explicit cases (partial list):**
- C01 Vexirk → FIREBALL (50%) or other (50%)
- C05 Swamp Slime → SLIME
- C09 Wizard Eye → LIGHTNING_BOLT or OPEN_DOOR
- C23 Lord Chaos → same 50/50 as Vexirk

**C25 Lord Order and C26 Grey Lord are absent from the switch.**

They fall through to `default:`:
```c
default:
    out->projectileThing = DM1_PROJECTILE_THING_FIREBALL;
    break;
```

Both creatures use the same graphic base as Lord Chaos (`{ 85|86, *, 0x14, 0xCB, 0x78AA }`), suggesting boss-tier classification. The identical graphic base and Lord-prefixed naming strongly implies similar attack behavior to Lord Chaos — but this is not confirmed.

## F0823 Default Fallback
- `out->useSpellSoundFallback = 1` — spell-cast sound plays (not creature sound)
- `out->shouldLaunch = 1`
- `out->kineticEnergy` computed from creature attack stat
- `out->attack = ctx->creatureInfo.dexterity`

## BUG0_13
BUG0_13 documents the latent ambiguity: two named creatures silently receiving identical FIREBALL-default behavior without explicit handler confirmation. If Lord Order or Grey Lord has a different attack type in the original DOS binary, this gap causes behavior divergence from the reference.

## DOS Runtime Capture Blocked
pass449 (Lord Order combat capture) and pass450 (Grey Lord combat capture) are blocked on missing DOS runtime environment. Without these captures the correct projectile type per creature is unconfirmed.

## Safe for Normal Play
For standard campaign dungeons where Lord Order and Grey Lord do not appear, this gap has zero effect. Even if they appear and default to FIREBALL, the creatures are fully hostile and dangerous.

## Modding Implication
Modders placing Lord Order or Grey Lord in custom dungeons via RE-DUNGEON.DAT will get FIREBALL as the attack. If the intended behavior differs (e.g., LIGHTNING_BOLT, POISON_CLOUD, unique projectile), F0823 must be patched.

## Required Fix (Non-Blocking)
1. Add `DM1_CREATURE_TYPE_LORD_ORDER` and `DM1_CREATURE_TYPE_GREY_LORD` to the enum (currently only LORD_CHAOS is defined for the C23+ tier)
2. Add explicit `case` entries to F0823 once DOS runtime confirms intended projectile type
3. Until then, FIREBALL default is the documented fallback behavior
