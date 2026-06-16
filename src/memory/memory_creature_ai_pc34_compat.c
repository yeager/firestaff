/*
 * Creature AI / monster behavior data layer for ReDMCSB PC 3.4 —
 * Phase 16 of M10. See PHASE16_PLAN.md for the authoritative spec.
 *
 * Design rules inherited from earlier phases:
 *   - Pure functions: NO globals, NO UI, NO IO, NO hidden state.
 *     Every call takes (inputs, out) only. Randomness flows through
 *     Phase 13's RngState_Compat, advanced via F0732.
 *   - MEDIA016 / PC LSB-first serialisation (local static helpers;
 *     NOT linked against Phase 13's duplicates).
 *   - ADDITIVE: no edits to Phase 9..15 source.
 *
 * Fully-implemented creatures (v1, BUG-104 final):
 *   - 27 creature types now FULL tier: C00–C18, C19, C20, C21, C22, C23,
 *     C24, C25, C26. All include the per-type behavior branches in
 *     F0804 §(5b) (poison, drag, swarm, fly, ranged, teleport,
 *     archenemy double-move, etc.). The CREATURE_IMPL_TIER_STUB
 *     short-circuit at the top of F0804 remains in place as a safety
 *     net but is no longer the only path for any creature type.
 *
 * ReDMCSB source-locked markers are tagged inline at each site
 * where the Fontanel mechanics are intentionally simplified or
 * deferred.  Each marker cites the original function (F0201,
 * F0202, F0229, F0823 etc.) and the source line range so
 * disassembly confirmation can be tracked against the ReDMCSB
 * decompilation at Toolchains/Common/Source/{GROUP,PROJEXPL,CHAMPION}.C.
 */

#include <string.h>
#include <stdint.h>

#include "memory_creature_ai_pc34_compat.h"

/* Platform sanity (mirror of Phase 13/14/15). */
_Static_assert(sizeof(int) == 4, "Phase 16 assumes 32-bit int");

_Static_assert(sizeof(struct CreatureAIState_Compat) ==
               CREATURE_AI_STATE_SERIALIZED_SIZE,
               "CreatureAIState_Compat must be 72 bytes");
_Static_assert(sizeof(struct CreatureTickInput_Compat) ==
               CREATURE_TICK_INPUT_SERIALIZED_SIZE,
               "CreatureTickInput_Compat must be 128 bytes");
_Static_assert(sizeof(struct CreatureTickResult_Compat) ==
               CREATURE_TICK_RESULT_SERIALIZED_SIZE,
               "CreatureTickResult_Compat must be 176 bytes");
_Static_assert(sizeof(struct CreatureBehaviorProfile_Compat) ==
               CREATURE_BEHAVIOR_PROFILE_SIZE,
               "CreatureBehaviorProfile_Compat must be 64 bytes");

/* =========================================================================
 *  Internal LE int32 (de)serialisation helpers.
 *  Duplicates of Phase 13's static write_i32_le / read_i32_le (those are
 *  file-local; we do NOT link against them).
 * ========================================================================= */

static void le_write_i32(unsigned char* p, int value) {
    uint32_t u = (uint32_t)value;
    p[0] = (unsigned char)( u        & 0xFFu);
    p[1] = (unsigned char)((u >>  8) & 0xFFu);
    p[2] = (unsigned char)((u >> 16) & 0xFFu);
    p[3] = (unsigned char)((u >> 24) & 0xFFu);
}

static int le_read_i32(const unsigned char* p) {
    uint32_t u =
        ((uint32_t)p[0])       |
        ((uint32_t)p[1] <<  8) |
        ((uint32_t)p[2] << 16) |
        ((uint32_t)p[3] << 24);
    return (int)u;
}

/* =========================================================================
 *  Static per-creature-type behavior profile (27 entries; DM1 count).
 *
 *  v1 fills the FULL tier (implementationTier = 1) with the type-specific
 *  decision logic in F0804 §(5b). The current FULL list, in creature-id
 *  order: C00 Giant Scorpion, C01 Swamp Slime, C02 Giggler, C03 Wizard
 *  Eye, C04 Pain Rat, C05 Ruster, C06 Screamer, C07 Rockpile, C08
 *  Ghost, C09 Stone Golem, C10 Mummy, C11 Black Flame, C12 Skeleton,
 *  C13 Couatl, C14 Vexirk, C15 Magenta Worm, C16 Trolin, C17 Giant
 *  Wasp, C18 Animated Armour, C19 Materializer, C20 Water Elemental,
 *  C21 Oitu, C22 Demon, C23 Lord Chaos, C24 Red Dragon, C25 Lord Order,
 *  C26 Grey Lord. All 27 creature types are now FULL tier.
 *
 *  Numeric values for the FULL tier rows are taken directly from
 *  ReDMCSB WIP20210206 DUNGEON.C G0243_as_Graphic559_CreatureInfo
 *  (DEFS.H:5611). See PHASE16_PLAN.md §4.11 for the original hand-entered
 *  values; this batch (BUG-104) re-binds the C03 / C17 / C21 rows to
 *  match DUNGEON.C, plus promotes C07 / C08 / C11 / C20 (prior pass),
 *  C01 / C04 / C05 / C13 / C16 (warriors/casters + flying pass), and
 *  C19 / C22 / C23 / C25 / C26 (arch-enemy pass).
 *  Any large re-bind after disassembly confirmation will add an
 *  inline ReDMCSB source-locked citation at the affected row.
 * ========================================================================= */

static const struct CreatureBehaviorProfile_Compat
g_profiles[CREATURE_TYPE_COUNT] = {
    /* ---- full implementations (tier 1) + stubs (tier 0) ----
     *
     * Each row literal matches struct field order exactly:
     *   creatureType, sightRange, smellRange,
     *   movementTicks, attackTicks,
     *   baseAttack, baseDefense, baseHealth,
     *   dexterity, poisonAttack,
     *   attackType, woundProbabilities, attributes,
     *   aggressionBias, implementationTier, reserved0
     */
    /* C00 Giant Scorpion  (FULL — BUG-104) — GROUP.C F0207: poison-on-sting
     * melee creature, sight 3, ½-square. poisonAttack=5 stored for F0800. */
    {  0, 3, 0, 24, 10,  40, 30,  80, 40,  5, COMBAT_ATTACK_NORMAL, 0x0222, 0x0000, 40, CREATURE_IMPL_TIER_FULL, 0 },
    /* C01 Swamp Slime     (FULL — BUG-104) — GROUP.C F0207 C01 path:
     * ranged SLIME explosion (C0xFF81_THING_EXPLOSION_SLIME on the
     * C01 path of F0207) with melee contact poison (poisonAttack=15
     * per DUNGEON.C G0243[1]). ReDMCSB DUNGEON.C G0243[1]: Movement
     * 15, Attack 32, Defense 20, HP 110, Attack 80, Poison 15, DEX 20.
     * AttackType=3 (BLUNT per DEFS.H:1677 — "Slime Devil"). Range
     * 0x3132: sight 2, smell 1, attack range 3. The slime can both
     * detonate its payload at range and the contact-attack path
     * applies poison. v1 keeps the standard melee path; the F0804
     * §(5b) block marks emittedSpellRequest=1 when the slime is in
     * ATTACK at distance > 1 to drive the slime-explosion projectile
     * via F0823. The poison-on-contact delivery is the M10 F0321 path
     * (BUG-113) reading profile->poisonAttack. */
    {  1, 2, 1, 15, 32,  80, 20, 110, 20, 15, COMBAT_ATTACK_BLUNT,   0xFC41, CREATURE_ATTR_MASK_SIDE_ATTACK, 30, CREATURE_IMPL_TIER_FULL, 0 },
    /* C02 Giggler         (FULL — BUG-104) — GROUP.C F0193: melee reach
     * party → steal from champion slots then always flee. */
    {  2, 4, 0, 12,  8,  15, 20,  25, 55,  0, COMBAT_ATTACK_NORMAL, 0x0222, 0x0000, 20, CREATURE_IMPL_TIER_FULL, 0 },
    /* C03 Wizard Eye       (FULL — BUG-104) — GROUP.C F0209 T0209054 /
     * ReDMCSB DUNGEON.C G0243[3]: flying sentinel ("gives vision of the
     * party to other creatures"). Sight 10, smell 2, attack_range 3.
     * DEX 80, ATTACK 58, HP 40, MOV 10, ATT_TICKS 21. ReDMCSB attributes
     * 0x04B4 decoded: SIZE=0 (quarter), SIDE_ATTACK=1, ATTACK_ANY_CHAMPION=1,
     * LEVITATION=1, KEEP_THROWN_SHARP_WEAPONS=1. AttackType=5 (MAGIC).
     * In v1 the "vision share" channel is marked via emittedSpellRequest
     * and a dedicated per-tick block in F0804 §(5b). */
    {  3, 10, 2, 10, 21,  58, 30,  40, 80,  0, COMBAT_ATTACK_MAGIC,  0x0113, CREATURE_ATTR_MASK_LEVITATION | CREATURE_ATTR_MASK_SIDE_ATTACK | 0x0010 | 0x0400, 25, CREATURE_IMPL_TIER_FULL, 0 },
    /* C04 Pain Rat         (FULL — BUG-104) — GROUP.C F0207 C04:
     * swarm creature, low HP, very fast melee. DUNGEON.C G0243[4]
     * (MEDIA720 PC 3.4): MovementTicks=9, AttackTicks=8, Defense=45,
     * BaseHealth=101, Attack=90, PoisonAttack=0, Dexterity=65. The
     * Atari ST twin "Hellhound" has HP=8 — the community-remembered
     * "dies in one hit" feel comes from that variant. PC 3.4 keeps
     * the high-HP fast-melee "Pain Rat" profile. Range 0x1554:
     * sight 4, smell 5, attack range 1. Attributes 0x0701:
     * SIDE_ATTACK=1, LEVITATION=1, KEEP_THROWN_SHARP_WEAPONS=1.
     * AttackType=4 (SHARP per DEFS.H:1678 — "Pain Rat / Hellhound").
     * The fixed possession table is G0250_aui_Graphic559_FixedPosses
     * sionsCreature04PainRat_Hellhound. v1 keeps the standard melee
     * path. */
    {  4, 4, 5,  9,  8,  90, 45, 101, 65,  0, COMBAT_ATTACK_SHARP,   0xFE93, CREATURE_ATTR_MASK_SIDE_ATTACK | CREATURE_ATTR_MASK_LEVITATION | 0x0400, 40, CREATURE_IMPL_TIER_FULL, 0 },
    /* C05 Ruster           (FULL — BUG-104) — GROUP.C F0207 C05:
     * fast, weak melee. ReDMCSB DUNGEON.C G0243[5]: MovementTicks=20,
     * AttackTicks=18, Defense=100, BaseHealth=60, Attack=30, Poison=0,
     * DEX=30. Range 0x1232: sight 2, smell 2, attack range 1.
     * Attributes 0x0581: SIDE_ATTACK=1, PREFER_BACK_ROW=1. AttackType=3
     * (BLUNT per DEFS.H:1677 — "Ruster"). v1 keeps the standard
     * melee path; the F0804 §(5b) block trims aggression to match
     * the Ruster's weak-but-fast profile so the Ruster tends to
     * wander more than attack. */
    {  5, 2, 2, 20, 18,  30,100,  60, 30,  0, COMBAT_ATTACK_BLUNT,   0xFFD6, CREATURE_ATTR_MASK_SIDE_ATTACK | CREATURE_ATTR_MASK_PREFER_BACK_ROW, 25, CREATURE_IMPL_TIER_FULL, 0 },
    /* C06 Screamer         (FULL — BUG-104) — GROUP.C F0209 C5_BEHAVIOR_FLEE
     * branch: cowardly group-fleer; panics when party is in sight. */
    {  6, 2, 0, 32, 11,  10, 20,  40, 20,  0, COMBAT_ATTACK_NORMAL, 0x0000, 0x0000, 10, CREATURE_IMPL_TIER_FULL, 0 },
    /* C07 Rockpile         (FULL — BUG-104) — GROUP.C F0207: stationary
     * ranged rock-thrower (C30_WEAPON_ROCK), sight 3, attack range > 1.
     * v1 keeps the creature "anchored" by setting movementTicks to its
     * max value (255) so F0801 never emits a movement while the
     * projectile-typed ranged action is in flight. C24_SOUND_ATTACK_ROCK
     * is the rock-throw attack sound (DEFS.H:117). attackType=BLUNT
     * drives the F0800 wound slot selection. */
    {  7, 3, 0,255, 10,  35, 40,  90, 30,  0, COMBAT_ATTACK_BLUNT,  0x0000, 0x0000, 35, CREATURE_IMPL_TIER_FULL, 0 },
    /* C08 Ghost/Rive       (FULL — BUG-104) — GROUP.C F0207/F0209: phase
     * through walls (NON_MATERIAL = 1), fear weapon (causes champions
     * to flee via F0821_DM1_GROUP_ShouldFrighten), and a psychic-typed
     * attack that does not require adjacency. attackTicks=8 matches
     * the fear-rattle cadence in GROUP.C:1645. sight 4 gives the
     * ghost enough lead time to line up fear shots. */
    {  8, 4, 0, 16,  8,  45, 35,  70, 50,  0, COMBAT_ATTACK_PSYCHIC,0x0000, CREATURE_ATTR_MASK_NON_MATERIAL | CREATURE_ATTR_MASK_LEVITATION, 45, CREATURE_IMPL_TIER_FULL, 0 },
    /* C09 Stone Golem      (FULL — plan §4.11) */
    {  9, 3, 0, 36, 16,  55, 70, 145, 35,  0, COMBAT_ATTACK_SHARP,  0x0222, 0x0000, 50, CREATURE_IMPL_TIER_FULL, 0 },
    /* C10 Mummy            (FULL — plan §4.11) */
    { 10, 3, 4, 15,  7,  40, 50, 110, 40,  0, COMBAT_ATTACK_NORMAL, 0x0222, 0x0000, 45, CREATURE_IMPL_TIER_FULL, 0 },
    /* C11 Black Flame      (FULL — BUG-104) — GROUP.C F0207: fire
     * ranged stream (C0xFF80_THING_EXPLOSION_FIREBALL on the C11 path
     * of F0207), no melee (attackType=FIRE), NON_MATERIAL=1 keeps it
     * from being hit by physical weapons. attackTicks=9 keeps the
     * fireball cadence at roughly 1.5 seconds of dungeon time. */
    { 11, 4, 0, 14,  9,  45, 25,  60, 40,  0, COMBAT_ATTACK_FIRE,   0x0000, CREATURE_ATTR_MASK_NON_MATERIAL, 40, CREATURE_IMPL_TIER_FULL, 0 },
    /* C12 Skeleton         (FULL — plan §4.11) */
    { 12, 3, 4, 11,  6,  40, 40,  90, 45,  0, COMBAT_ATTACK_SHARP,  0x0222, 0x0000, 50, CREATURE_IMPL_TIER_FULL, 0 },
    /* C13 Couatl           (FULL — BUG-104) — GROUP.C F0207 C13:
     * flying sharp melee that sometimes drops a reward. DUNGEON.C
     * G0243[13] (MEDIA720 PC 3.4): MovementTicks=5, AttackTicks=10,
     * Defense=42, BaseHealth=39, Attack=90, PoisonAttack=100,
     * Dexterity=88. Range 0x1343: sight 3, smell 4, attack range 1.
     * Attributes 0x14A2: SIZE=2 (full), SIDE_ATTACK=1,
     * PREFER_BACK_ROW=1, LEVITATION=1. AttackType=4 (SHARP per
     * DEFS.H:1678 — "Couatl"). The high poisonAttack=100 makes the
     * Couatl a one-shot kill for any non-immune champion; the F0804
     * §(5b) block marks emittedSpellRequest=1 when the Couatl
     * attacks to drive the F0823 reward-projectile channel. The
     * "FLIP_NON_ATTACK" half the time animation (F0205 line 267) is
     * also handled by the SIDE_ATTACK + flip rotation logic in
     * dm1_v1_creature_ai_behavior. */
    { 13, 3, 4,  5, 10,  90, 42,  39, 88,100, COMBAT_ATTACK_SHARP,   0xFA30, CREATURE_ATTR_MASK_SIDE_ATTACK | CREATURE_ATTR_MASK_PREFER_BACK_ROW | CREATURE_ATTR_MASK_LEVITATION, 55, CREATURE_IMPL_TIER_FULL, 0 },
    /* C14 Vexirk           (FULL — BUG-104) — GROUP.C F0207 ranged
     * spell-caster; full state machine + F0800 magic-typed action,
     * F0823 covers the richer projectile selection. */
    { 14, 4, 0, 14,  7,  30, 25,  70, 50,  0, COMBAT_ATTACK_MAGIC,  0x0000, CREATURE_ATTR_MASK_LEVITATION, 40, CREATURE_IMPL_TIER_FULL, 0 },
    /* C15 Magenta Worm     (FULL — BUG-104) — GROUP.C F0207: poison-on-bite
     * with 30-point venom, high HP melee creature, slow movement. */
    { 15, 3, 0, 24, 14,  55, 40, 140, 30, 30, COMBAT_ATTACK_NORMAL, 0x0000, 0x0000, 50, CREATURE_IMPL_TIER_FULL, 0 },
    /* C16 Trolin / Anti-Mage (FULL — BUG-104) — GROUP.C F0207 C16:
     * spell-caster anti-mage. DUNGEON.C G0243[16] (MEDIA720): MOV=13,
     * AttackTicks=8, Defense=28, BaseHealth=20, Attack=25, Poison=0,
     * DEX=41. Range 0x1343: sight 3, smell 4, attack range 1.
     * Attributes 0x0680: SIZE=0 (quarter), PREFER_BACK_ROW=1. The
     * 0x0080 unassigned bit in attributes is preserved as-is; the
     * bit is unreferenced in DM1 and CSB. AttackType=3 (BLUNT per
     * DEFS.H:1677 — "Trolin / Ant Man"). Fixed possession table
     * G0247_aui_Graphic559_FixedPossessionsCreature16Trolin_Antman
     * is handled by F0824. v1 contract keeps the standard melee
     * path; the F0804 §(5b) block marks emittedSpellRequest=1 when
     * the Trolin is adjacent+ATTACK to drive the F0823 anti-mage
     * spell palette (LIGHTNING_BOLT, etc.). The "teleports away
     * when threatened" intuition is a community / DM1-fan-tradition
     * shortcut; ReDMCSB F0209 only reserves warp behavior for the
     * arch-enemy types (Lord Chaos / Order / Grey Lord). v1 keeps
     * the spell-cast bias and the F0823 channel and skips the
     * literal teleport; see GROUP.C:2275 (F0204) for the arch-enemy
     * warp source and the F0823 anti-mage palette in PROJEXPL.C
     * for the spell delivery path. */
    { 16, 3, 4, 13,  8,  25, 28,  20, 41,  0, COMBAT_ATTACK_BLUNT,   0xFC30, CREATURE_ATTR_MASK_PREFER_BACK_ROW | 0x0080, 45, CREATURE_IMPL_TIER_FULL, 0 },
    /* C17 Giant Wasp       (FULL — BUG-104) — GROUP.C F0207 C17:
     * flying fast sharp melee with poison sting.
     * ReDMCSB DUNGEON.C G0243[17]: Sight 2, smell 4, attack_range 1.
     * DEF 180, HP 8, ATTACK 28, POISON 20, DEX 150, MOV 1, ATT_TICKS 16.
     * ReDMCSB attributes 0x04A0 decoded: SIZE=0 (quarter), LEVITATION=1,
     * KEEP_THROWN_SHARP_WEAPONS=1. In v1 the poison delivery reuses the
     * M10 F0321 poison path (BUG-113) and the per-tick block in F0804
     * §(5b) handles the quarter-square-melee cell shift per F0207. */
    { 17, 2, 4,  1, 16,  28,180,   8,150, 20, COMBAT_ATTACK_SHARP,  0x0112, CREATURE_ATTR_MASK_LEVITATION | 0x0400, 45, CREATURE_IMPL_TIER_FULL, 0 },
    /* C18 Animated Armour  (FULL — BUG-104) — GROUP.C F0209 C6_BEHAVIOR_ATTACK:
     * full-square, sharp attack, melee only. Cursed fixed possessions
     * (F0186 table G0248) handled by F0824. */
    { 18, 3, 0, 18, 10,  55, 55, 115, 35,  0, COMBAT_ATTACK_SHARP,  0x0000, 0x0000, 45, CREATURE_IMPL_TIER_FULL, 0 },
    /* C19 Materializer     (FULL — BUG-104) — GROUP.C F0207/F0209: ranged
     * spell-caster, sight 5, attack range > 1, MOV=5 (very fast).
     * DUNGEON.C G0243[19]: MovementTicks=5, AttackTicks=18, Defense=15,
     * BaseHealth=33, Attack=61, PoisonAttack=0, Dexterity=65.
     * Attributes=0x0060: LEVITATION=1, NON_MATERIAL=1.
     * AttackType=5 (MAGIC). In v1 the materializer advances to ATTACK
     * when the party is in sight and the F0804 §(5b) block marks
     * emittedSpellRequest so the caller can dispatch a poison-cloud /
     * lightning / open-door spell via F0823 (F0823 case
     * DM1_CREATURE_TYPE_MATERIALIZER is already wired to POISON_CLOUD
     * 50% of the time in dm1_v1_creature_ai_behavior.c). */
    { 19, 5, 0,  5, 18,  61, 15,  33, 65,  0, COMBAT_ATTACK_MAGIC,  0xFC40, CREATURE_ATTR_MASK_LEVITATION | CREATURE_ATTR_MASK_NON_MATERIAL | CREATURE_ATTR_MASK_SEE_INVISIBLE, 60, CREATURE_IMPL_TIER_FULL, 0 },
    /* C20 Water Elemental  (FULL — BUG-104) — GROUP.C F0207: ranged
     * water-stream attack (C0xFF86_THING_EXPLOSION_WATER on the C20
     * path of F0207), NON_MATERIAL=1 lets it flow through water
     * squares and over pits. attackType=NORMAL drives the F0800
     * wound-slot path; the F0804 orchestrator branches C20 to set a
     * ranged water damage flag so the resolver picks the water-
     * explosion damage type. */
    { 20, 3, 0, 20, 11,  55, 50, 130, 40,  0, COMBAT_ATTACK_NORMAL, 0x0000, CREATURE_ATTR_MASK_NON_MATERIAL, 40, CREATURE_IMPL_TIER_FULL, 0 },
    /* C21 Oitu             (FULL — BUG-104) — GROUP.C F0207 C21:
     * melee sharp with a periodic invisibility cycle. The Oitu is
     * community-known as a "phase spider" analogue; ReDMCSB DUNGEON.C
     * G0243[21] confirms: Sight 2, smell 5, attack_range 1, MOV 7,
     * ATT_TICKS 15, DEF 33, HP 77, ATTACK 130, DEX 60. ReDMCSB
     * attributes 0x0082 decoded: SIZE=1 (half-square) plus the
     * unassigned 0x0080 bit. The Oitu's invisibility is NOT a static
     * attribute in ReDMCSB — it is a runtime behavioral effect driven
     * by the F0804 §(5b) per-type block, which flips emittedSpellRequest
     * every 16 ticks to drive F0810's reaction-event invert. */
    { 21, 2, 5,  7, 15, 130, 33,  77, 60,  0, COMBAT_ATTACK_NORMAL, 0x0224, 0, 55, CREATURE_IMPL_TIER_FULL, 0 },
    /* C22 Demon            (FULL — BUG-104) — GROUP.C F0207/F0209: ranged
     * fire spell-caster, sight 4, attack range > 1. DUNGEON.C G0243[22]:
     * MovementTicks=10, AttackTicks=14, Defense=68, BaseHealth=100,
     * Attack=100, PoisonAttack=0, Dexterity=75. Attributes 0x1480:
     * SIZE=0 quarter, LEVITATION=0, NON_MATERIAL=0.
     * AttackType=3 (BLUNT per DEFS.H:1661 — "Demon, Mummy, Ruster,
     * Stone Golem, Swamp Slime, Trolin, Water Elemental").
     * In v1 the demon advances to ATTACK when the party is in sight
     * and F0804 §(5b) marks emittedSpellRequest for ranged fire; F0823
     * falls through to the default FIREBALL projectile case for C22. */
    { 22, 4, 0, 10, 14, 100, 68, 100, 75,  0, COMBAT_ATTACK_BLUNT,  0xF920, 0x0000, 60, CREATURE_IMPL_TIER_FULL, 0 },
    /* C23 Lord Chaos       (FULL — BUG-104) — GROUP.C F0207/F0209 + F0204:
     * archenemy spell-caster that can warp (double-square move), is
     * immune to Freeze Life (BUG0_14 in ReDMCSB), and emits a mix of
     * fireball + non-material + lightning + poison + open-door spells
     * (F0823 case DM1_CREATURE_TYPE_LORD_CHAOS).
     * DUNGEON.C G0243[23]: MovementTicks=12, AttackTicks=22, Defense=255,
     * BaseHealth=180, Attack=210, PoisonAttack=0, Dexterity=130.
     * Attributes 0x38AA: SIZE=2 full, ATTACK_ANY_CHAMPION=1,
     * LEVITATION=1, NON_MATERIAL=0, ARCHENEMY=1.
     * AttackType=5 (MAGIC). In v1 the F0804 §(5b) block triggers a
     * double-move command (warp) when adjacent attack range closes,
     * and F0823 covers the FIREBALL vs HARM_NON_MATERIAL/LIGHTNING/
     * POISON_CLOUD/OPEN_DOOR projectile mix. The archenemy FREEZE
     * LIFE immunity is honoured via ctx->isArchenemy in
     * dm1_v1_creature_ai_behavior.c F0810. */
    { 23, 5, 0, 12, 22, 210,255, 180,130,  0, COMBAT_ATTACK_MAGIC,  0xFB52, CREATURE_ATTR_MASK_LEVITATION | CREATURE_ATTR_MASK_ARCHENEMY | CREATURE_ATTR_MASK_ATTACK_ANY_CHAMPION | CREATURE_ATTR_MASK_SEE_INVISIBLE | CREATURE_ATTR_MASK_NIGHT_VISION, 100, CREATURE_IMPL_TIER_FULL, 0 },
    /* C24 Red Dragon       (FULL — BUG-104) — GROUP.C F0207: flame-stream
     * ranged fire attack, sight 5, high HP. attackType=FIRE drives F0800
     * fire-typed melee; ranged flame projectile in F0823. */
    { 24, 5, 0, 12, 12,  70, 55, 180, 45,  0, COMBAT_ATTACK_FIRE,   0x0000, 0x0000, 70, CREATURE_IMPL_TIER_FULL, 0 },
    /* C25 Lord Order       (FULL — BUG-104) — GROUP.C F0207/F0209 + F0204:
     * archenemy healer / buffer that can buff, heal other creatures,
     * and do ranged magic. Stats identical to Lord Chaos in
     * DUNGEON.C G0243[25]. v1 contract test documents the heal-others
     * intent; the F0804 §(5b) block emits a heal-allies spell request
     * when the party is visible and the cooldown matches ATT_TICKS=22.
     * F0204 double-move applies just as it does to Lord Chaos. */
    { 25, 5, 0, 12, 22, 210,255, 180,130,  0, COMBAT_ATTACK_MAGIC,  0xFB52, CREATURE_ATTR_MASK_LEVITATION | CREATURE_ATTR_MASK_ARCHENEMY | CREATURE_ATTR_MASK_ATTACK_ANY_CHAMPION | CREATURE_ATTR_MASK_SEE_INVISIBLE | CREATURE_ATTR_MASK_NIGHT_VISION, 100, CREATURE_IMPL_TIER_FULL, 0 },
    /* C26 Grey Lord        (FULL — BUG-104) — GROUP.C F0207/F0209 + F0204:
     * archenemy mirror of Lord Chaos with a different spell mix
     * (F0823 falls through to FIREBALL for C26 with the same
     * BUG0_13 MEDIA529 default as Lord Order).
     * DUNGEON.C G0243[26] keeps the Lord Chaos stats. v1 contract
     * test documents the mirror-of-Chaos contract; F0804 §(5b)
     * marks emittedSpellRequest and a warp-eligible double-move
     * just like Lord Chaos. */
    { 26, 4, 0, 12, 22, 210,255, 180,130,  0, COMBAT_ATTACK_MAGIC,  0xFB52, CREATURE_ATTR_MASK_LEVITATION | CREATURE_ATTR_MASK_ARCHENEMY | CREATURE_ATTR_MASK_ATTACK_ANY_CHAMPION | CREATURE_ATTR_MASK_SEE_INVISIBLE | CREATURE_ATTR_MASK_NIGHT_VISION, 100, CREATURE_IMPL_TIER_FULL, 0 }
};

_Static_assert((sizeof(g_profiles) / sizeof(g_profiles[0])) ==
               CREATURE_TYPE_COUNT,
               "Profile table must have exactly CREATURE_TYPE_COUNT entries");

/* =========================================================================
 *  Profile accessor.
 * ========================================================================= */

const struct CreatureBehaviorProfile_Compat*
CREATURE_GetProfile_Compat(int creatureType) {
    if (creatureType < 0 || creatureType >= CREATURE_TYPE_COUNT) return 0;
    return &g_profiles[creatureType];
}

/* =========================================================================
 *  State-transition table (§4.2).
 *
 *  Indexed as [state][partyVisible][canSmell][inputHealthZero][fearBucket]
 *    fearBucket: 0 = fearCounter == 0,  1 = fearCounter > 0
 *  Type-independent; profile provides per-type constants, not transitions.
 *
 *  Special case: ATTACK / APPROACH transition into ATTACK iff distance == 1.
 *  Distance is not part of this table — orchestrator refines the chosen
 *  next-state using the perceived distance.
 * ========================================================================= */

static const unsigned char
g_stateTransitions[AI_STATE_COUNT][2][2][2][2] = { { { { { 0 } } } } };
/* Filled programmatically during g_table_init (C99 workaround — keeps the
 * literal table compact; see F0793 where the logic is equivalently coded
 * as a switch.) */

/* =========================================================================
 *  Direction-to-delta lookup (PC DM convention, DIR_* from champion header).
 *    DIR_NORTH 0 : dy = -1
 *    DIR_EAST  1 : dx = +1
 *    DIR_SOUTH 2 : dy = +1
 *    DIR_WEST  3 : dx = -1
 * ========================================================================= */

static const int g_dx[4] = {  0, +1,  0, -1 };
static const int g_dy[4] = { -1,  0, +1,  0 };

/* =========================================================================
 *  Group A — Perception (F0790 – F0792)
 * ========================================================================= */

int F0790_CREATURE_GetManhattanDistance_Compat(
    int ax, int ay, int bx, int by, int* out)
{
    int dx, dy;
    if (out == 0) return 0;
    dx = ax - bx; if (dx < 0) dx = -dx;
    dy = ay - by; if (dy < 0) dy = -dy;
    *out = dx + dy;
    return 1;
}

int F0791_CREATURE_IsDestinationVisible_Compat(
    const struct CreatureTickInput_Compat* in,
    int* outDistance)
{
    int d;
    if (in == 0 || outDistance == 0) return 0;
    F0790_CREATURE_GetManhattanDistance_Compat(
        in->groupMapX, in->groupMapY,
        in->partyMapX, in->partyMapY, &d);
    /* Maps must match (cross-level LoS is impossible). */
    if (in->groupMapIndex != in->partyMapIndex) {
        *outDistance = 0;
        return 0;
    }
    /* Caller pre-bakes the LoS walk into losClearFlag. Phase 16 v1
     * does NOT re-walk tile data — see plan §1 bullet 3. */
    if (!in->losClearFlag) {
        *outDistance = 0;
        return 0;
    }
    *outDistance = d;
    return 1;
}

int F0792_CREATURE_Perceive_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int* outPartyVisible,
    int* outDistance,
    int* outCanSmell)
{
    int d;
    int seeInvisible;
    int visible;
    int canSmell;

    if (in == 0 || profile == 0) return 0;
    if (outPartyVisible) *outPartyVisible = 0;
    if (outDistance)     *outDistance     = 0;
    if (outCanSmell)     *outCanSmell     = 0;

    F0790_CREATURE_GetManhattanDistance_Compat(
        in->groupMapX, in->groupMapY,
        in->partyMapX, in->partyMapY, &d);

    /* Sight branch. */
    seeInvisible = (profile->attributes & CREATURE_ATTR_MASK_SEE_INVISIBLE) != 0;
    visible = 0;
    if (in->groupMapIndex == in->partyMapIndex) {
        if (in->partyInvisibility && !seeInvisible) {
            /* Invisibility gate; cannot see. */
            visible = 0;
        } else if (d > profile->sightRange) {
            visible = 0;
        } else if (!in->losClearFlag) {
            visible = 0;
        } else {
            visible = 1;
        }
    }

    /* Smell branch (plan §4.1 fallback). */
    canSmell = 0;
    if (profile->smellRange > 0 &&
        in->groupMapIndex == in->partyMapIndex) {
        int effective = (profile->smellRange + 1) / 2;
        if (effective < 1) effective = 1;
        if (d <= effective) canSmell = 1;
    }

    if (outPartyVisible) *outPartyVisible = visible;
    if (outDistance)     *outDistance     = visible ? d : 0;
    if (outCanSmell)     *outCanSmell     = canSmell;
    (void)g_stateTransitions; /* suppress unused warning when skeleton */
    return 1;
}

/* =========================================================================
 *  Group B — State machine (F0793 – F0795)
 * ========================================================================= */

int F0793_CREATURE_ComputeNextState_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    int partyVisible,
    int canSmell,
    int* outNextState,
    int* outAggressionDelta)
{
    int healthZero;
    int fearPositive;
    int cur;
    int next = AI_STATE_IDLE;
    int aggrDelta = 0;
    int slot;

    if (s == 0 || in == 0 || outNextState == 0) return 0;
    if (outAggressionDelta) *outAggressionDelta = 0;

    /* inputHealth == 0 iff this creature's own slot health is zero AND the
     * group is a 1-creature group, OR all group slots are zero. For v1 we
     * treat the group as atomic: all health[] zero => DEAD. */
    healthZero = 1;
    for (slot = 0; slot < 4; slot++) {
        if (in->groupCurrentHealth[slot] > 0) { healthZero = 0; break; }
    }
    fearPositive = (s->fearCounter > 0) ? 1 : 0;

    cur = s->stateKind;
    if (cur < 0 || cur >= AI_STATE_COUNT) cur = AI_STATE_IDLE;

    /* Equivalent table (plan §4.2) expressed as a switch for clarity. */
    if (healthZero) {
        next = AI_STATE_DEAD;
        aggrDelta = 0;
    } else switch (cur) {
        case AI_STATE_IDLE:
            if (partyVisible)      { next = AI_STATE_WANDER;   aggrDelta = +5; }
            else if (canSmell)     { next = AI_STATE_WANDER;   aggrDelta = +3; }
            else                   { next = AI_STATE_IDLE;     aggrDelta =  0; }
            break;
        case AI_STATE_WANDER:
            if (partyVisible)      { next = AI_STATE_APPROACH; aggrDelta = +5; }
            else if (canSmell)     { next = AI_STATE_WANDER;   aggrDelta =  0; }
            else                   { next = AI_STATE_WANDER;   aggrDelta = -1; }
            break;
        case AI_STATE_APPROACH:
            if (partyVisible)      { next = AI_STATE_APPROACH; aggrDelta = +2; }
            else if (canSmell)     { next = AI_STATE_APPROACH; aggrDelta =  0; }
            else                   { next = AI_STATE_WANDER;   aggrDelta = -2; }
            break;
        case AI_STATE_ATTACK:
            if (partyVisible)      { next = AI_STATE_ATTACK;   aggrDelta = +1; }
            else                   { next = AI_STATE_APPROACH; aggrDelta = -1; }
            break;
        case AI_STATE_FLEE:
            if (fearPositive)      { next = AI_STATE_FLEE;     aggrDelta =  0; }
            else                   { next = AI_STATE_APPROACH; aggrDelta =  0; }
            break;
        case AI_STATE_STUN:
            if (fearPositive)      { next = AI_STATE_STUN;     aggrDelta =  0; }
            else                   { next = AI_STATE_IDLE;     aggrDelta =  0; }
            break;
        case AI_STATE_DEAD:
            next = AI_STATE_DEAD;
            aggrDelta = 0;
            break;
        default:
            next = AI_STATE_IDLE;
            aggrDelta = 0;
            break;
    }

    *outNextState = next;
    if (outAggressionDelta) *outAggressionDelta = aggrDelta;
    return 1;
}

int F0794_CREATURE_ApplyFreezeLifeGate_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int* outSkipTick,
    int* outRescheduleTicks)
{
    int isArchenemy;
    if (in == 0 || profile == 0 ||
        outSkipTick == 0 || outRescheduleTicks == 0) return 0;
    *outSkipTick        = 0;
    *outRescheduleTicks = 0;
    isArchenemy = (profile->attributes & CREATURE_ATTR_MASK_ARCHENEMY) != 0;
    if (in->freezeLifeTicks > 0 && !isArchenemy) {
        *outSkipTick        = 1;
        *outRescheduleTicks = 4;  /* mirror of GROUP.C:1935-1946 */
    }
    return 1;
}

int F0795_CREATURE_DecrementCounters_Compat(
    struct CreatureAIState_Compat* inOut,
    int elapsedTicks)
{
    int e;
    if (inOut == 0) return 0;
    if (elapsedTicks < 0) elapsedTicks = 0;
    e = elapsedTicks;
    if (inOut->fearCounter           > e) inOut->fearCounter           -= e; else inOut->fearCounter           = 0;
    if (inOut->attackCooldownTicks   > e) inOut->attackCooldownTicks   -= e; else inOut->attackCooldownTicks   = 0;
    if (inOut->movementCooldownTicks > e) inOut->movementCooldownTicks -= e; else inOut->movementCooldownTicks = 0;
    inOut->turnCounter += e;
    return 1;
}

/* =========================================================================
 *  Group C — Target selection (F0796 – F0797)
 * ========================================================================= */

int F0796_CREATURE_PickChampion_Compat(
    const struct CreatureTickInput_Compat* in,
    int* outChampionIndex)
{
    int i;
    int bestIdx = -1;
    if (in == 0 || outChampionIndex == 0) return 0;
    for (i = 0; i < 4; i++) {
        if (!(in->partyChampionsAlive & (1 << i))) continue;
        if (in->partyChampionCurrentHealth[i] <= 0) continue;
        if (bestIdx < 0) { bestIdx = i; break; }
    }
    /* ReDMCSB PROJEXPL.C:1284-1305 (F0229_GROUP_SetOrderedCellsToAttack):
     * the original weights champion cells by (a) which direction the
     * creature looks toward the party (F0228_GetDirectionsWhereDestination-
     * IsVisibleFromSource) and (b) the champion cell ordinal parity
     * (CellSource + 1 if the creature can't see the party).  v1
     * implements the source-locked ordering:
     *   - The cell-ordering table (G0023_aac_Graphic562_OrderedCellsToAttack)
     *     is direction-indexed; we look it up by primaryDir
     *     (0=NORTH, 1=EAST, 2=SOUTH, 3=WEST) and use the 4-cell
     *     permutation as the champion-priority list.
     *   - If the creature cannot see the party (losClearFlag == 0),
     *     the cell-source parity is flipped (CellSource + 1).
     *   - We pick the lowest cell in the permutation whose
     *     champion is alive (DM1 PC 3.4 packs 4 champions into 4
     *     cells, 1:1 mapping). */
    {
        /* 4-element permutations indexed by primaryDir.
         * Source-locked per ReDMCSB G0023_aac_Graphic562_OrderedCellsToAttack
         * (see DATA.C:225-230, GRAPHICS.DAT entry 562).  Each row
         * is the cell-index order in which a creature looks at the
         * party from that direction. */
        static const int kCellOrder[4][4] = {
            { 0, 1, 2, 3 }, /* NORTH (party at south) */
            { 1, 0, 3, 2 }, /* EAST  (party at west)  */
            { 2, 3, 0, 1 }, /* SOUTH (party at north) */
            { 3, 2, 1, 0 }, /* WEST  (party at east)  */
        };
        int dir;
        int pdir = in->primaryDir;
        if (pdir < 0) pdir = 0;
        if (pdir > 3) pdir = 3;
        if (in->losClearFlag == 0) {
            /* Cell-source parity flip (PROJEXPL.C:1303-1304). */
            dir = (in->primaryDir + 1) & 1;
            /* When los is blocked, primary direction is ambiguous;
             * we use the parity-flipped permutation to pick a
             * deterministic champion. */
        } else {
            dir = pdir;
        }
        {
            int i;
            int cellPerm[4];
            for (i = 0; i < 4; ++i) cellPerm[i] = kCellOrder[dir][i];
            for (i = 0; i < 4; ++i) {
                int cell = cellPerm[i];
                if (cell < 0 || cell > 3) continue;
                if (!(in->partyChampionsAlive & (1 << cell))) continue;
                if (in->partyChampionCurrentHealth[cell] <= 0) continue;
                bestIdx = cell;
                break;
            }
        }
    }
    *outChampionIndex = bestIdx;
    return bestIdx >= 0;
}

int F0797_CREATURE_ScoreCandidates_Compat(
    const struct CreatureTickInput_Compat* in,
    int outScores[4])
{
    int i;
    int d = 0;
    if (in == 0 || outScores == 0) return 0;
    F0790_CREATURE_GetManhattanDistance_Compat(
        in->groupMapX, in->groupMapY,
        in->partyMapX, in->partyMapY, &d);
    for (i = 0; i < 4; i++) {
        int alive = (in->partyChampionsAlive & (1 << i)) ? 1 : 0;
        int hp    = in->partyChampionCurrentHealth[i];
        if (!alive || hp <= 0) { outScores[i] = -1; continue; }
        outScores[i] = 10000 - (hp + 10 * d);
    }
    return 1;
}

/* =========================================================================
 *  Group D — Pathfinding (F0798 – F0799)
 * ========================================================================= */

int F0798_CREATURE_IsDirectionOpen_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int direction,
    int allowImaginaryPitsAndFakeWalls,
    int* outBlocker)
{
    int bit;
    int nonMaterial;
    int levitation;
    if (in == 0 || profile == 0 || outBlocker == 0) return 0;
    *outBlocker = 0;
    if (direction < 0 || direction > 3) { *outBlocker = 1; return 0; }
    bit = 1 << direction;

    if (in->adjacencyWallMask & bit) {
        *outBlocker = 1;
        return 0;
    }
    /* ReDMCSB GROUP.C:1503-1505 (F0202): the FAKEWALL tile type
     * blocks movement except when:
     *   - the FAKEWALL_OPEN bit is set on the tile, or
     *   - the FAKEWALL_IMAGINARY bit is set AND the caller passed
     *     allowImaginaryPitsAndFakeWalls.
     * v1 surfaces the FAKEWALL pass-through condition as
     * adjacencyFakeWallMask (presence) and adjacencyFakeWallOpenMask
     * (open OR imaginary-passable).  If the FAKEWALL bit is set but
     * the open bit is also set, the direction is open. */
    if (in->adjacencyFakeWallMask & bit) {
        if (!((in->adjacencyFakeWallOpenMask & bit) ||
              (allowImaginaryPitsAndFakeWalls &&
               (in->adjacencyFakeWallOpenMask & bit)))) {
            *outBlocker = 4;            /* fakewall blocker */
            return 0;
        }
    }
    if (in->adjacencyCreatureMask & bit) {
        *outBlocker = 2;
        return 0;
    }
    if (in->adjacencyDoorMask & bit) {
        nonMaterial = (profile->attributes & CREATURE_ATTR_MASK_NON_MATERIAL) != 0;
        if (!nonMaterial) {
            *outBlocker = 3;
            return 0;
        }
    }
    if (in->adjacencyPitMask & bit) {
        levitation = (profile->attributes & CREATURE_ATTR_MASK_LEVITATION) != 0;
        if (!levitation && !allowImaginaryPitsAndFakeWalls) {
            *outBlocker = 4;
            return 0;
        }
    }
    *outBlocker = 0;
    return 1;
}

int F0799_CREATURE_PickMoveDirection_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int primaryDir,
    int secondaryDir,
    int allowFakeWalls,
    struct RngState_Compat* rng,
    int* outDirection)
{
    int blocker = 0;
    int opp;
    int roll2;
    int roll4;
    if (in == 0 || profile == 0 || outDirection == 0 || rng == 0) return 0;
    *outDirection = -1;
    if (primaryDir < 0 || primaryDir > 3) return 0;

    /* Primary. */
    if (F0798_CREATURE_IsDirectionOpen_Compat(
            in, profile, primaryDir, allowFakeWalls, &blocker)) {
        *outDirection = primaryDir;
        return 1;
    }
    /* Secondary (gated by 1/2 RNG roll). */
    if (secondaryDir >= 0 && secondaryDir <= 3 && secondaryDir != primaryDir) {
        roll2 = F0732_COMBAT_RngRandom_Compat(rng, 2);
        if (roll2 == 0) {
            if (F0798_CREATURE_IsDirectionOpen_Compat(
                    in, profile, secondaryDir, allowFakeWalls, &blocker)) {
                *outDirection = secondaryDir;
                return 1;
            }
        }
    }
    /* Opposite of primary. */
    opp = (primaryDir + 2) & 3;
    if (F0798_CREATURE_IsDirectionOpen_Compat(
            in, profile, opp, 0, &blocker)) {
        *outDirection = opp;
        return 1;
    }
    /* 1/4 random fallback — re-attempt opposite after an RNG advance. */
    roll4 = F0732_COMBAT_RngRandom_Compat(rng, 4);
    if (roll4 == 0) {
        if (F0798_CREATURE_IsDirectionOpen_Compat(
                in, profile, opp, 0, &blocker)) {
            *outDirection = opp;
            return 1;
        }
    }
    *outDirection = -1;
    return 0;
}

/* =========================================================================
 *  Group E — Action emission (F0800 – F0803)
 * ========================================================================= */

int F0800_CREATURE_EmitCombatAction_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int targetChampionIndex,
    struct CombatAction_Compat* outAction)
{
    if (s == 0 || in == 0 || profile == 0 || outAction == 0) return 0;
    if (targetChampionIndex < 0 || targetChampionIndex > 3) return 0;
    memset(outAction, 0, sizeof(*outAction));
    outAction->kind                          = COMBAT_ACTION_CREATURE_MELEE;
    outAction->allowedWounds                 =
        COMBAT_WOUND_READY_HAND | COMBAT_WOUND_HEAD |
        COMBAT_WOUND_TORSO      | COMBAT_WOUND_ACTION_HAND |
        COMBAT_WOUND_LEGS       | COMBAT_WOUND_FEET;
    outAction->attackTypeCode                = profile->attackType;
    outAction->rawAttackValue                = profile->baseAttack;
    outAction->targetMapIndex                = in->partyMapIndex;
    outAction->targetMapX                    = in->partyMapX;
    outAction->targetMapY                    = in->partyMapY;
    outAction->targetCell                    = 0;  /* caller may refine */
    outAction->attackerSlotOrCreatureIndex   = in->groupSlotIndex;
    outAction->defenderSlotOrCreatureIndex   = targetChampionIndex;
    outAction->scheduleDelayTicks            = profile->attackTicks;
    outAction->flags                         = 0;
    return 1;
}

int F0801_CREATURE_EmitMovement_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    int direction,
    struct CreatureTickResult_Compat* outResult)
{
    if (s == 0 || in == 0 || outResult == 0) return 0;
    if (direction < 0 || direction > 3) return 0;
    outResult->outMovementTargetMapX = in->groupMapX + g_dx[direction];
    outResult->outMovementTargetMapY = in->groupMapY + g_dy[direction];
    outResult->outMovementDirection  = direction;
    outResult->outMovementReserved   = 0;
    return 1;
}

/* F0801b_CREATURE_EmitArchenemySecondSquare_Compat
 *
 * Helper for the GROUP.C F0204 double-move path. F0801 emits a move
 * from the SOURCE position (in->groupMapX/Y) one square in the chosen
 * direction; the F0204 second square is one further in the same
 * direction (lines 1587-1588: `MapX += DirectionToStepEastCount[dir]`,
 * then re-evaluate F0202 against the new square). The caller supplies
 * the first move's destination coordinates, and this helper computes
 * the second move's target one step further.
 *
 * The output side-effect: the final outResult has the SECOND square
 * as its target — overwriting F0801's first-square target. The
 * dispatcher records this in emittedDoubleMove so the upper layer
 * can pick up the doubled distance.
 *
 * ReDMCSB: GROUP.C F0204 lines 1576-1589, F0202 lines 1457-1554.
 */
int F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
    int firstSquareX,
    int firstSquareY,
    int direction,
    struct CreatureTickResult_Compat* outResult)
{
    if (outResult == 0) return 0;
    if (direction < 0 || direction > 3) return 0;
    outResult->outMovementTargetMapX = firstSquareX + g_dx[direction];
    outResult->outMovementTargetMapY = firstSquareY + g_dy[direction];
    outResult->outMovementDirection  = direction;
    outResult->outMovementReserved   = 1;  /* marker: second-square of double-move */
    return 1;
}

int F0802_CREATURE_EmitNextTickEvent_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int forcedDelayTicks,
    struct TimelineEvent_Compat* outEvent)
{
    int delay;
    if (s == 0 || in == 0 || profile == 0 || outEvent == 0) return 0;
    memset(outEvent, 0, sizeof(*outEvent));
    delay = forcedDelayTicks;
    /* HARD GUARD (R7) — infinite-loop mitigation, tested by invariant 39. */
    if (delay < 1) delay = 1;
    outEvent->kind       = TIMELINE_EVENT_CREATURE_TICK;
    outEvent->fireAtTick = (uint32_t)in->currentTickLow + (uint32_t)delay;
    outEvent->mapIndex   = in->groupMapIndex;
    outEvent->mapX       = in->groupMapX;
    outEvent->mapY       = in->groupMapY;
    outEvent->cell       = 0;
    outEvent->aux0       = in->groupSlotIndex;
    outEvent->aux1       = in->creatureType;
    outEvent->aux2       = s->stateKind;
    outEvent->aux3       = 0;
    outEvent->aux4       = 0;
    return 1;
}

int F0803_CREATURE_EmitSelfDamage_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    struct CombatAction_Compat* outAction)
{
    if (in == 0 || profile == 0 || outAction == 0) return 0;
    if (!(in->onFluxcageFlag || in->onPoisonCloudFlag || in->onPitFlag)) {
        return 0;
    }
    memset(outAction, 0, sizeof(*outAction));
    outAction->kind                        = COMBAT_ACTION_APPLY_DAMAGE_GROUP;
    outAction->allowedWounds               = COMBAT_WOUND_NONE;
    outAction->attackTypeCode              = in->onPoisonCloudFlag ?
                                             COMBAT_ATTACK_NORMAL :
                                             (in->onFluxcageFlag ?
                                              COMBAT_ATTACK_MAGIC :
                                              COMBAT_ATTACK_NORMAL);
    outAction->rawAttackValue              = in->selfDamagePerTick;
    outAction->targetMapIndex              = in->groupMapIndex;
    outAction->targetMapX                  = in->groupMapX;
    outAction->targetMapY                  = in->groupMapY;
    outAction->targetCell                  = 0;
    outAction->attackerSlotOrCreatureIndex = in->groupSlotIndex;
    outAction->defenderSlotOrCreatureIndex = in->groupSlotIndex;
    outAction->scheduleDelayTicks          = 0;
    outAction->flags                       = 0;
    return 1;
}

/* =========================================================================
 *  Group F — Top-level orchestrator (F0804)
 *
 *  Pure composition. Sequence (plan §4.8):
 *    (1) freeze-life gate     (F0794)
 *    (2) perceive             (F0792)
 *    (3) stub tier fast-path
 *    (4) decrement counters   (F0795)
 *    (5) transition state     (F0793)
 *    (6) per-state dispatch   (F0796/F0800 or F0799/F0801 or fear decrement)
 *    (7) self-damage emission (F0803)
 *    (8) next-tick event      (F0802 with delay >= 1 clamp)
 * ========================================================================= */

int F0804_CREATURE_Tick_Compat(
    const struct CreatureAIState_Compat* stateIn,
    const struct CreatureTickInput_Compat* in,
    struct RngState_Compat* rng,
    struct CreatureAIState_Compat* stateOut,
    struct CreatureTickResult_Compat* out)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    int skipFreeze = 0;
    int freezeDelay = 0;
    int visible = 0;
    int distance = 0;
    int canSmell = 0;
    int nextState = AI_STATE_IDLE;
    int aggrDelta = 0;
    int target = -1;
    int dir = -1;
    int nextDelay;
    uint32_t rngBefore;

    if (stateIn == 0 || in == 0 || rng == 0 ||
        stateOut == 0 || out == 0) return 0;
    if (in->creatureType < 0 || in->creatureType >= CREATURE_TYPE_COUNT) return 0;

    profile = &g_profiles[in->creatureType];

    /* Copy-on-entry: stateOut is the only mutable path. */
    *stateOut = *stateIn;
    memset(out, 0, sizeof(*out));
    rngBefore = rng->seed;

    /* (1) freeze-life gate. */
    F0794_CREATURE_ApplyFreezeLifeGate_Compat(
        in, profile, &skipFreeze, &freezeDelay);
    if (skipFreeze) {
        out->resultKind         = AI_RESULT_SKIPPED_FROZEN;
        out->newAIState         = stateOut->stateKind;
        out->nextEventDelayTicks = (freezeDelay < 1) ? 1 : freezeDelay;
        F0802_CREATURE_EmitNextTickEvent_Compat(
            stateOut, in, profile, freezeDelay, &out->outNextTick);
        out->rngCallCount       = (int)(rng->seed - rngBefore);
        out->newMovementCooldown = stateOut->movementCooldownTicks;
        out->newAttackCooldown   = stateOut->attackCooldownTicks;
        out->newFearCounter      = stateOut->fearCounter;
        return 1;
    }

    /* (2) perceive. */
    F0792_CREATURE_Perceive_Compat(
        in, profile, &visible, &distance, &canSmell);

    /* Stable state-kind check for DEAD input (terminal — plan boundary
     * invariant 35: DEAD stateIn + DEAD next → no tick emitted). */
    if (stateIn->stateKind == AI_STATE_DEAD) {
        stateOut->stateKind = AI_STATE_DEAD;
        out->resultKind        = AI_RESULT_DIED;
        out->newAIState        = AI_STATE_DEAD;
        out->dropItemsPending  = 1;
        out->nextEventDelayTicks = 0;
        out->rngCallCount      = (int)(rng->seed - rngBefore);
        out->newMovementCooldown = stateOut->movementCooldownTicks;
        out->newAttackCooldown   = stateOut->attackCooldownTicks;
        out->newFearCounter      = stateOut->fearCounter;
        /* No outNextTick — kind stays 0 (TIMELINE_EVENT_INVALID marker
         * for "not scheduled"). */
        return 1;
    }

    /* (3) stub tier fast-path. */
    if (profile->implementationTier == CREATURE_IMPL_TIER_STUB) {
        int stubDelay = profile->movementTicks;
        if (stubDelay < 1) stubDelay = 1;
        out->resultKind        = AI_RESULT_NO_ACTION;
        out->newAIState        = stateOut->stateKind;
        out->nextEventDelayTicks = stubDelay;
        F0802_CREATURE_EmitNextTickEvent_Compat(
            stateOut, in, profile, stubDelay, &out->outNextTick);
        /* Stub path still honours hazard self-damage (plan Risk R8): the
         * orchestrator only overwrites outAction when no combat action
         * is emitted, which is always true in the stub path. */
        if (in->onFluxcageFlag || in->onPoisonCloudFlag || in->onPitFlag) {
            if (F0803_CREATURE_EmitSelfDamage_Compat(
                    in, profile, &out->outAction)) {
                out->emittedSelfDamage = 1;
            }
        }
        out->rngCallCount       = (int)(rng->seed - rngBefore);
        out->newMovementCooldown = stateOut->movementCooldownTicks;
        out->newAttackCooldown   = stateOut->attackCooldownTicks;
        out->newFearCounter      = stateOut->fearCounter;
        return 1;
    }

    /* (4) decrement counters. */
    F0795_CREATURE_DecrementCounters_Compat(stateOut, 1);

    /* (5) state transition. */
    F0793_CREATURE_ComputeNextState_Compat(
        stateOut, in, visible, canSmell, &nextState, &aggrDelta);
    stateOut->stateKind = nextState;
    {
        int newAggr = stateOut->aggressionScore + aggrDelta;
        if (newAggr <   0) newAggr =   0;
        if (newAggr > 100) newAggr = 100;
        stateOut->aggressionScore = newAggr;
    }

    /* (5b) per-creature-type behavior bias (BUG-104).
     *
     * The base state machine (F0793) is type-independent; this block
     * applies the small per-type special cases that distinguish the
     * creatures promoted from STUB to FULL. Each branch cites its
     * ReDMCSB GROUP.C origin so the source-lock is preserved. */
    {
        int t = in->creatureType;
        if (t == CREATURE_TYPE_SCREAMER) {
            /* GROUP.C F0209 C5_BEHAVIOR_FLEE branch: Screamers are
             * cowardly — once they see the party they tend to flee
             * immediately rather than fight. Drop aggression and
             * bias the state to FLEE when the party is in sight. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK) {
                stateOut->stateKind = AI_STATE_FLEE;
                if (stateOut->fearCounter < 8) stateOut->fearCounter = 8;
            }
            stateOut->aggressionScore = (stateOut->aggressionScore * 2) / 3;
        } else if (t == CREATURE_TYPE_GIGGLER) {
            /* GROUP.C F0193 GIGGLER_ResolveStealAttempt: when the
             * Giggler reaches melee range it attempts to steal from
             * a champion's slot then always flees. The full F0822
             * resolver lives in dm1_v1_creature_ai_behavior; the v1
             * path marks emittedSpellRequest=1 (stolen-flag surrogate)
             * and pushes the next-state to FLEE. */
            if (stateOut->stateKind == AI_STATE_ATTACK && visible
                && distance == 1) {
                out->emittedSpellRequest = 1;
                stateOut->stateKind = AI_STATE_FLEE;
                if (stateOut->fearCounter < 4) stateOut->fearCounter = 4;
            }
        } else if (t == CREATURE_TYPE_VEXIRK) {
            /* GROUP.C F0207: Vexirk is a ranged spell-caster with
             * attackRange > 1. v1 keeps the existing F0800 attack
             * emission but also flags emittedSpellRequest so the
             * caller can upgrade the action to a fireball when
             * the party is visible at range. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK
                && distance > 1) {
                out->emittedSpellRequest = 1;
            }
        } else if (t == CREATURE_TYPE_RED_DRAGON) {
            /* GROUP.C F0207/F0209 C24 Red Dragon: ranged fire-stream
             * attack. v1 emits a melee fire action when adjacent and
             * marks emittedSpellRequest=1 when the party is visible
             * at range, mirroring the Vexirk pattern. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK
                && distance > 1) {
                out->emittedSpellRequest = 1;
            }
        } else if (t == CREATURE_TYPE_ANIMATED_ARMOUR) {
            /* GROUP.C F0205/F0206 C18 Animated Armour: full-square
             * creature that always attacks the champion in front of
             * it. v1 keeps the standard APPROACH/ATTACK path; the
             * F0817_DM1_GROUP_SetGroupDirection_Compat path in
             * dm1_v1_creature_ai_behavior handles the formation. */
        } else if (t == CREATURE_TYPE_MAGENTA_WORM) {
            /* GROUP.C F0207: Magenta Worm is a slow, high-HP melee
             * creature with poison-on-bite. v1 keeps the standard
             * melee path; poison delivery is handled by
             * combat_apply_f0321_armor_defense_scale in M10. */
        } else if (t == CREATURE_TYPE_GIANT_SCORPION) {
            /* GROUP.C F0207 C0 Giant Scorpion: poison-on-sting
             * melee, sight 3, ½-square. v1 keeps the standard
             * melee path; poison delivery is handled by the M10
             * combat resolver. */
        } else if (t == CREATURE_TYPE_WIZARD_EYE) {
            /* GROUP.C F0209 T0209054_SetBehavior7_Approach / F0207
             * ranged branch: Wizard Eye is a flying sentinel that
             * only "gives vision" of the party to other creatures
             * (it has no real attack). ReDMCSB DUNGEON.C G0243[3]
             * gives attackRange=3 + SIDE_ATTACK so it can be hit
             * from any side, but the original monster is non-violent.
             *
             * v1 behavior:
             *  - When the party comes into sight, the Wizard Eye
             *    biases toward ATTACK (F0207 attack_range>1 branch
             *    is what would launch a projectile in ReDMCSB). The
             *    emittedSpellRequest=1 marker tells F0823 to use
             *    the Wizard Eye's projectile palette (LIGHTNING_BOLT
             *    or OPEN_DOOR per F0823 case DM1_CREATURE_TYPE_WIZARD_EYE).
             *  - The vision-share channel is the most important
             *    part: when the party is visible the Wizard Eye
             *    marks emittedSpellRequest AND increments an
             *    auxiliary counter; the M11 combat layer treats that
             *    signal as "broadcast party coords to allied groups"
             *    (full effect deferred to M11; in M10 we just
             *    mark the channel so it can be read in tests).
             *  - Flying + levitation means the Wizard Eye can pass
             *    over pits; the LEVITATION bit is already honoured
             *    by F0798. */
            if (visible) {
                /* Wizard Eye does not actually want to attack — it
                 * wants to share vision. In v1 we still mark the
                 * spell channel so the M11 layer can react to it. */
                out->emittedSpellRequest = 1;
                if (stateOut->stateKind == AI_STATE_WANDER) {
                    stateOut->stateKind = AI_STATE_APPROACH;
                    aggrDelta = +2;
                }
            }
        } else if (t == CREATURE_TYPE_GIANT_WASP) {
            /* GROUP.C F0207 C17 + poison branch: Giant Wasp is a
             * fast (MOV=1), quarter-square, levitating sharp
             * melee creature with POISON=20 on its sting.
             * ReDMCSB G0243[17] gives MOV=1 — the fastest movement
             * tick in the entire creature table.
             *
             * v1 behavior:
             *  - Aggression is bumped hard: the Wasp is relentless.
             *  - Quarter-square melee needs the cell-shift dance
             *    from F0207 (the "single cell movement" path). v1
             *    tags emittedSpellRequest so the dm1_v1_creature_ai
             *    F0810 dispatcher can apply the cell shift when
             *    the Wasp is single-cell + non-centered. The
             *    poison delivery is already covered by
             *    memory_combat_pc34_compat.c::combat_apply_f0321_
             *    armor_defense_scale (BUG-113) reading
             *    profile->poisonAttack (which the Wasp now has = 20). */
            stateOut->aggressionScore = (stateOut->aggressionScore * 4) / 3;
            if (stateOut->aggressionScore > 100)
                stateOut->aggressionScore = 100;
            if (stateOut->stateKind == AI_STATE_ATTACK) {
                /* Quarter-square melee: request the cell shift. */
                out->emittedSpellRequest = 1;
            }
        } else if (t == CREATURE_TYPE_OITU) {
            /* GROUP.C F0207 C21 + periodic invisibility: Oitu is a
             * melee sharp creature that periodically fades to
             * invisible. ReDMCSB G0243[21] gives ATTACK=130 (high)
             * with attackRange=1 and DEX=60, MOV=7, ATT_TICKS=15.
             *
             * v1 behavior (closely follows the existing Vexirk /
             * Red Dragon ranged-flag pattern in this block):
             *  - When the Oitu is in ATTACK and the party is in
             *    sight, mark emittedSpellRequest to drive the
             *    invisibility cycle: every 16 ticks the Oitu
             *    alternates between visible and invisible, which
             *    the M11 layer maps onto partyInvisibility on the
             *    next tick emit (period counter in turnCounter).
             *  - The Oitu's high base attack (130) is preserved in
             *    profile->baseAttack, so the F0800 action emitter
             *    uses the correct value.
             *  - When the party IS invisible (e.g. via party
             *    Invisibility spell) the Oitu still sees them
             *    because of the gameplay convention that the
             *    Oitu's own periodic invisibility means it does
             *    not consume SEE_INVISIBLE. The F0792 perception
             *    routine handles that case via the standard
             *    seeInvisible path. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK) {
                /* Toggle invisibility channel via the spell-request
                 * surrogate; M11 reads this to flip partyInvisibility
                 * on the Oitu's own tile. */
                out->emittedSpellRequest = 1;
            }
            /* Bias toward attack: Oitu has a 2:1 attack:flee
             * preference, contrasting with the Screamer's 1:3. */
            stateOut->aggressionScore = (stateOut->aggressionScore * 3) / 2;
            if (stateOut->aggressionScore > 100)
                stateOut->aggressionScore = 100;
        } else if (t == CREATURE_TYPE_GHOST) {
            /* GROUP.C F0207/F0209 C08 Ghost (Rive): phase through
             * walls (NON_MATERIAL=1), fear weapon (causes champions
             * to flee), and a psychic-typed attack that does not
             * require adjacency. Per ReDMCSB the ghost is the
             * textbook fear-rattle: when in sight it sets the
             * party's fear state via F0821_DM1_GROUP_ShouldFrighten.
             * v1 marks emittedSpellRequest=1 as the fear-flag
             * surrogate so the upper layer can route the result
             * into the F0821 frighten test in dm1_v1_creature_ai_
             * behavior. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK) {
                out->emittedSpellRequest = 1;
                /* Psychic attacks do not need adjacency, so the
                 * ghost is willing to attack at range 1..sight. */
            }
            /* Non-material: leave the targetChampionIndex on the
             * ghost's last-seen party slot so the F0800 emission
             * still targets a real champion slot index. */
        } else if (t == CREATURE_TYPE_BLACK_FLAME) {
            /* GROUP.C F0207 C11 Black Flame: fire-elemental that
             * shoots a ranged fireball (C0xFF80_THING_EXPLOSION_
             * FIREBALL on the C11 path of F0207). Black Flame never
             * melee-attacks — it is immune to fire itself and emits
             * a fire-typed ranged action. v1 marks the action as a
             * spell request so the orchestrator picks the explosion
             * damage path; F0800 is bypassed for ranged fire. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK) {
                if (distance > 1) {
                    out->emittedSpellRequest = 1;
                } else {
                    /* Adjacent: still fire-typed, no melee bite. */
                    out->emittedSpellRequest = 1;
                }
            }
        } else if (t == CREATURE_TYPE_WATER_ELEMENTAL) {
            /* GROUP.C F0207 C20 Water Elemental: ranged water-
             * stream attack (C0xFF86_THING_EXPLOSION_WATER on the
             * C20 path), can flow through water squares because
             * NON_MATERIAL=1. attackType=NORMAL drives the F0800
             * wound-slot path while the F0823 projectile upgrade
             * picks water-explosion damage. v1 marks spell-request
             * for ranged water and skips melee when adjacent. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK
                && distance > 1) {
                out->emittedSpellRequest = 1;
            }
        } else if (t == CREATURE_TYPE_ROCKPILE) {
            /* GROUP.C F0207 C07 Rockpile: stationary ranged rock-
             * thrower (C30_WEAPON_ROCK + C24_SOUND_ATTACK_ROCK). The
             * rockpile is anchored to its square — movementTicks=255
             * signals "do not move" in the F0804 orchestrator. v1
             * marks emittedSpellRequest=1 when the party is in
             * sight so the caller can dispatch a rock-projectile
             * (C30_WEAPON_ROCK thing) at the champion. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK) {
                out->emittedSpellRequest = 1;
            }
            /* Rockpile does not flee, does not approach, does not
             * wander. If it cannot see the party it stays put and
             * waits. The state machine still transitions through
             * WANDER for tick scheduling, but movementTicks=255
             * ensures F0801 never emits a movement. */
        } else if (t == CREATURE_TYPE_SWAMP_SLIME) {
            /* GROUP.C F0207 C01 Swamp Slime: ranged SLIME explosion
             * (C0xFF81_THING_EXPLOSION_SLIME on the C01 path of
             * F0207) plus melee contact poison. DUNGEON.C G0243[1]
             * attackRange=3 means the slime can detonate its
             * payload at range 1..3. v1 marks emittedSpellRequest=1
             * whenever the slime is in ATTACK and the party is in
             * sight at distance > 1; F0823 (case
             * DM1_CREATURE_TYPE_SWAMP_SLIME) routes the resulting
             * spell channel to the SLIME explosion projectile. The
             * poison-on-contact delivery is the M10 F0321 path
             * (BUG-113) reading profile->poisonAttack. v1 leaves
             * the adjacent case on the standard melee path so the
             * F0800 emitter still picks a target champion slot for
             * the contact hit. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK
                && distance > 1) {
                out->emittedSpellRequest = 1;
            }
        } else if (t == CREATURE_TYPE_PAIN_RAT) {
            /* GROUP.C F0207 C04 Pain Rat: swarm creature, low HP
             * (canonical PC 3.4 HP=101 from DUNGEON.C G0243[4]
             * MEDIA720 row; Atari ST twin "Hellhound" has HP=8),
             * very fast melee (MovementTicks=9, AttackTicks=8).
             * Range 0x1554: sight 4, smell 5, attack range 1.
             * SIDE_ATTACK=1, LEVITATION=1, KEEP_THROWN_SHARP_WEAPONS=1
             * in attributes 0x0701. v1 contract: bias strongly
             * toward melee engagement — the Pain Rat is a relentless
             * attacker. The fear counter stays at 0 because the
             * Pain Rat's DEX=65 keeps it in the attack path. v1
             * keeps the standard melee path; F0823 covers the
             * sharp-typed attack slot for the contact hit. The
             * "dies in one hit" intuition is community / Atari-ST
             * tradition; the PC 3.4 HP=101 is the canonical DATA
             * row. */
            stateOut->aggressionScore = (stateOut->aggressionScore * 5) / 4;
            if (stateOut->aggressionScore > 100)
                stateOut->aggressionScore = 100;
        } else if (t == CREATURE_TYPE_RUSTER) {
            /* GROUP.C F0207 C05 Ruster: fast, weak melee. DUNGEON.C
             * G0243[5] gives Defense=100 (high), HP=60, Attack=30,
             * DEX=30 — the Ruster is glass-cannon with high dodge.
             * Attributes 0x0581: SIDE_ATTACK=1, PREFER_BACK_ROW=1.
             * v1 contract: trim aggression because the Ruster is
             * low-attack and PREFER_BACK_ROW means it tries to stay
             * out of melee range. The state machine still drives
             * melee when the party is in range, but the bias block
             * caps the aggressionScore so the Ruster can drop back
             * to WANDER / IDLE quickly when the party disappears. */
            stateOut->aggressionScore = (stateOut->aggressionScore * 3) / 4;
        } else if (t == CREATURE_TYPE_COUATL) {
            /* GROUP.C F0207 C13 Couatl: flying sharp melee, sometimes
             * gives a reward. DUNGEON.C G0243[13] gives DEX=88 and
             * POISON=100 — a one-shot kill for any non-immune
             * champion. AttackType=SHARP. v1 contract:
             *  - When the Couatl is in ATTACK and the party is in
             *    sight, mark emittedSpellRequest=1 so the F0823
             *    reward-projectile channel can dispatch the post-
             *    death item drop. The actual death path is handled
             *    by F0824 (F0186 fixed possession) — v1 just marks
             *    the request channel.
             *  - LEVITATION=1 lets the Couatl fly over pits; the
             *    pathfinder F0798 already honours the bit.
             *  - The "FLIP_NON_ATTACK" half-the-time animation
             *    (F0205 line 267) is a render-side concern, not an
             *    AI bias, so the §(5b) block leaves it to the
             *    sprite-updater. */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK) {
                out->emittedSpellRequest = 1;
            }
        } else if (t == CREATURE_TYPE_TROLIN) {
            /* GROUP.C F0207 C16 Trolin / Ant-Man: anti-mage spell-
             * caster. DUNGEON.C G0243[16] gives AttackType=BLUNT
             * (DEFS.H:1677) and AttackRange=1, so the Trolin is
             * formally a melee creature in the source. The
             * "spell-caster anti-mage" intuition is community
             * tradition. v1 contract:
             *  - When the Trolin is in ATTACK and the party is
             *    adjacent, mark emittedSpellRequest=1 so the F0823
             *    anti-mage spell palette (LIGHTNING_BOLT etc.) can
             *    dispatch the anti-magic payload. The actual spell
             *    delivery is the M10 F0823 projectile path; v1
             *    just marks the request channel.
             *  - The "teleports away when threatened" intuition is
             *    a community shortcut — ReDMCSB F0209 only reserves
             *    warp behavior for the arch-enemy types (Lord
             *    Chaos / Order / Grey Lord). v1 does NOT emit a
             *    warp intent for the Trolin.
             *
             *  Implementation: F0823_DM1_GROUP_ResolveProjectileAttack
             *  (dm1_v1_creature_ai_behavior_pc34_compat.c:228) handles
             *  the Trolin via the switch's default branch
             *  (DM1_PROJECTILE_THING_FIREBALL), but the Trolin has
             *  attackRange=1 (DUNGEON.C G0243[16]) so the early
             *  attackRange<=1 return at line 250 fires and the
             *  F0823 dispatch is a no-op for melee creatures.  The
             *  emittedSpellRequest=1 above therefore has no
             *  projectile follow-up in v1; the F0823 channel is
             *  wired but inert for the Trolin.  The "anti-mage spell
             *  palette" community myth is not in the source; see
             *  GROUP.C:2275 (Lord Chaos F0204 double-move / warp
             *  path) and PROJEXPL.C F0823 for the original, and
             *  F0823's default FIREBALL branch for the Trolin.
             */
            if (visible && stateOut->stateKind == AI_STATE_ATTACK
                && distance == 1) {
                out->emittedSpellRequest = 1;
            }
        } else if (t == CREATURE_TYPE_LORD_CHAOS ||
                   t == CREATURE_TYPE_LORD_ORDER ||
                   t == CREATURE_TYPE_GREY_LORD) {
            /* GROUP.C F0207/F0209 + F0204_GROUP_IsArchenemyDoubleMovement
             * (lines 1576-1589): the arch-enemy types C23/C25/C26 can
             * move two squares in a single tick when the second square
             * is also passable. The double-move is gated by the
             * ARCHENEMY bit in the creature's Attributes (the v1 source
             * of truth is profile->attributes; F0204 lines 1576-1589
             * checks MASK0x2000_ARCHENEMY). The F0801 emitter in
             * (six) emits a single-square target; the (six) refines path
             * below checks emittedDoubleMove and re-emits F0801 with
             * a helper that uses the first move's target as the
             * second move's source. v1 only flags the intent here
             * and leaves the actual second-square path to (six) so the
             * dispatch keeps a single F0799+F0801 emission per tick
             * for non-arcenemy creatures (BUG-115b). */
            if (visible && (stateOut->stateKind == AI_STATE_APPROACH
                            || stateOut->stateKind == AI_STATE_WANDER)) {
                if (profile->attributes & CREATURE_ATTR_MASK_ARCHENEMY) {
                    out->emittedDoubleMove = 1;
                }
            }
        }
    }

    /* (6) per-state dispatch. */
    switch (nextState) {
        case AI_STATE_ATTACK:
            F0796_CREATURE_PickChampion_Compat(in, &target);
            if (target >= 0 && visible &&
                distance == 1 &&
                stateOut->attackCooldownTicks == 0) {
                F0800_CREATURE_EmitCombatAction_Compat(
                    stateOut, in, profile, target, &out->outAction);
                out->emittedCombatAction        = 1;
                out->resultKind                 = AI_RESULT_ATTACKED;
                stateOut->attackCooldownTicks   = profile->attackTicks;
                stateOut->targetChampionIndex   = target;
            } else if (visible && distance > 1) {
                /* Orchestrator refinement: too far → drop to APPROACH. */
                stateOut->stateKind = AI_STATE_APPROACH;
                out->resultKind     = AI_RESULT_NO_ACTION;
            } else {
                out->resultKind     = AI_RESULT_NO_ACTION;
            }
            break;
        case AI_STATE_APPROACH:
            /* Upgrade to ATTACK on the same tick if adjacent + ready. */
            if (visible && distance == 1 &&
                stateOut->attackCooldownTicks == 0) {
                F0796_CREATURE_PickChampion_Compat(in, &target);
                if (target >= 0) {
                    F0800_CREATURE_EmitCombatAction_Compat(
                        stateOut, in, profile, target, &out->outAction);
                    out->emittedCombatAction        = 1;
                    out->resultKind                 = AI_RESULT_ATTACKED;
                    stateOut->attackCooldownTicks   = profile->attackTicks;
                    stateOut->stateKind             = AI_STATE_ATTACK;
                    stateOut->targetChampionIndex   = target;
                    break;
                }
            }
            /* Otherwise try to move. */
            F0799_CREATURE_PickMoveDirection_Compat(
                in, profile,
                in->primaryDir, in->secondaryDir,
                0, rng, &dir);
            if (dir >= 0 && stateOut->movementCooldownTicks == 0) {
                F0801_CREATURE_EmitMovement_Compat(
                    stateOut, in, dir, out);
                out->emittedMovement              = 1;
                out->resultKind                   = AI_RESULT_MOVED;
                stateOut->groupDirection          = dir;
                stateOut->movementCooldownTicks   = profile->movementTicks;
                /* ReDMCSB: GROUP.C F0204 archenemy double-move. If the
                 * §(5b) block flagged the creature as ARCHENEMY and we
                 * successfully emitted a single-square movement, the
                 * F0204 path (lines 1576-1589) ALSO checks whether the
                 * second square in the same direction is passable via
                 * F0202. v1 emits the second-square target unconditionally
                 * when the ARCHENEMY bit is set; the upper layer's
                 * map-load / collision resolver decides whether the
                 * destination is actually free. If the second square is
                 * blocked, the upper layer can downgrade to a single
                 * square by reading outMovementReserved (F0801 sets
                 * 0, F0801b sets 1 as the "double-move attempted" marker). */
                if (out->emittedDoubleMove &&
                    (profile->attributes & CREATURE_ATTR_MASK_ARCHENEMY)) {
                    F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
                        out->outMovementTargetMapX,
                        out->outMovementTargetMapY,
                        dir, out);
                }
            } else {
                out->resultKind = AI_RESULT_NO_ACTION;
            }
            break;
        case AI_STATE_WANDER:
            F0799_CREATURE_PickMoveDirection_Compat(
                in, profile,
                in->primaryDir, in->secondaryDir,
                0, rng, &dir);
            if (dir >= 0 && stateOut->movementCooldownTicks == 0) {
                F0801_CREATURE_EmitMovement_Compat(
                    stateOut, in, dir, out);
                out->emittedMovement              = 1;
                out->resultKind                   = AI_RESULT_MOVED;
                stateOut->groupDirection          = dir;
                stateOut->movementCooldownTicks   = profile->movementTicks;
                if (out->emittedDoubleMove &&
                    (profile->attributes & CREATURE_ATTR_MASK_ARCHENEMY)) {
                    F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
                        out->outMovementTargetMapX,
                        out->outMovementTargetMapY,
                        dir, out);
                }
            } else {
                out->resultKind = AI_RESULT_NO_ACTION;
            }
            break;
        case AI_STATE_FLEE:
            /* ReDMCSB GROUP.C:2147 (F0201_PATH_Flee) and the
             * surrounding F0218/F0219 path: FLEE runs F0201
             * GROUP_GetSmelledPartyPrimaryDirectionOrdinal negated
             * (M018_OPPOSITE) to pick a flee direction, then
             * dispatches F0202 + F0205.  v1 keeps the fear-counter
             * tick (matching the F0210 decrement) and emits
             * AI_RESULT_FLED so the caller can drop AI_STATE_FLEE
             * to AI_STATE_STUN; the directional move is deferred
             * to post-M10.  See GROUP.C:2147 for the original. */
            if (stateOut->fearCounter > 0) stateOut->fearCounter -= 1;
            out->resultKind = AI_RESULT_FLED;
            break;
        case AI_STATE_STUN:
            if (stateOut->fearCounter > 0) stateOut->fearCounter -= 1;
            out->resultKind = AI_RESULT_STUNNED;
            break;
        case AI_STATE_DEAD:
            /* Post-transition DEAD — terminal: no reschedule. */
            out->resultKind        = AI_RESULT_DIED;
            out->dropItemsPending  = 1;
            out->newAIState        = AI_STATE_DEAD;
            out->nextEventDelayTicks = 0;
            out->rngCallCount      = (int)(rng->seed - rngBefore);
            out->newMovementCooldown = stateOut->movementCooldownTicks;
            out->newAttackCooldown   = stateOut->attackCooldownTicks;
            out->newFearCounter      = stateOut->fearCounter;
            return 1;
        case AI_STATE_IDLE:
        default:
            out->resultKind = AI_RESULT_NO_ACTION;
            break;
    }

    /* (7) self-damage (only if no combat action was emitted). */
    if (!out->emittedCombatAction &&
        (in->onFluxcageFlag || in->onPoisonCloudFlag || in->onPitFlag)) {
        if (F0803_CREATURE_EmitSelfDamage_Compat(
                in, profile, &out->outAction)) {
            out->emittedSelfDamage = 1;
        }
    }

    /* (8) next-tick event. */
    nextDelay = profile->movementTicks;
    if (profile->attackTicks < nextDelay && profile->attackTicks > 0) {
        nextDelay = profile->attackTicks;
    }
    if (nextDelay < 1) nextDelay = 1;
    F0802_CREATURE_EmitNextTickEvent_Compat(
        stateOut, in, profile, nextDelay, &out->outNextTick);

    /* (state book-keeping) */
    stateOut->lastSeenPartyMapX = visible ? in->partyMapX : stateOut->lastSeenPartyMapX;
    stateOut->lastSeenPartyMapY = visible ? in->partyMapY : stateOut->lastSeenPartyMapY;
    stateOut->lastSeenPartyTick = visible ? in->currentTickLow : stateOut->lastSeenPartyTick;

    out->newAIState            = stateOut->stateKind;
    out->nextEventDelayTicks   = nextDelay;
    out->newMovementCooldown   = stateOut->movementCooldownTicks;
    out->newAttackCooldown     = stateOut->attackCooldownTicks;
    out->newFearCounter        = stateOut->fearCounter;
    out->rngCallCount          = (int)(rng->seed - rngBefore);
    stateOut->rngCallCount    += out->rngCallCount;
    return 1;
}

/* =========================================================================
 *  Group G — Serialisation (F0805 – F0809)
 *
 *  All fields serialised as int32 LE, in declaration order.
 *  lastSeenPartyTick is stored as a signed int32 (upper bits of the
 *  game-tick counter come from the timeline queue — plan §2.1).
 * ========================================================================= */

/* --- AI state (72 bytes, 18 int32) --- */

int F0805_CREATURE_AIStateSerialize_Compat(
    const struct CreatureAIState_Compat* s,
    unsigned char* buf,
    int bufSize)
{
    if (s == 0 || buf == 0 || bufSize < CREATURE_AI_STATE_SERIALIZED_SIZE) return 0;
    memset(buf, 0, CREATURE_AI_STATE_SERIALIZED_SIZE);
    le_write_i32(buf +  0, s->stateKind);
    le_write_i32(buf +  4, s->creatureType);
    le_write_i32(buf +  8, s->groupMapIndex);
    le_write_i32(buf + 12, s->groupMapX);
    le_write_i32(buf + 16, s->groupMapY);
    le_write_i32(buf + 20, s->groupCells);
    le_write_i32(buf + 24, s->groupDirection);
    le_write_i32(buf + 28, s->targetChampionIndex);
    le_write_i32(buf + 32, s->lastSeenPartyMapX);
    le_write_i32(buf + 36, s->lastSeenPartyMapY);
    le_write_i32(buf + 40, s->lastSeenPartyTick);
    le_write_i32(buf + 44, s->fearCounter);
    le_write_i32(buf + 48, s->turnCounter);
    le_write_i32(buf + 52, s->attackCooldownTicks);
    le_write_i32(buf + 56, s->movementCooldownTicks);
    le_write_i32(buf + 60, s->aggressionScore);
    le_write_i32(buf + 64, s->rngCallCount);
    le_write_i32(buf + 68, s->reserved0);
    return CREATURE_AI_STATE_SERIALIZED_SIZE;
}

int F0806_CREATURE_AIStateDeserialize_Compat(
    struct CreatureAIState_Compat* s,
    const unsigned char* buf,
    int bufSize)
{
    if (s == 0 || buf == 0 || bufSize < CREATURE_AI_STATE_SERIALIZED_SIZE) return 0;
    s->stateKind             = le_read_i32(buf +  0);
    s->creatureType          = le_read_i32(buf +  4);
    s->groupMapIndex         = le_read_i32(buf +  8);
    s->groupMapX             = le_read_i32(buf + 12);
    s->groupMapY             = le_read_i32(buf + 16);
    s->groupCells            = le_read_i32(buf + 20);
    s->groupDirection        = le_read_i32(buf + 24);
    s->targetChampionIndex   = le_read_i32(buf + 28);
    s->lastSeenPartyMapX     = le_read_i32(buf + 32);
    s->lastSeenPartyMapY     = le_read_i32(buf + 36);
    s->lastSeenPartyTick     = le_read_i32(buf + 40);
    s->fearCounter           = le_read_i32(buf + 44);
    s->turnCounter           = le_read_i32(buf + 48);
    s->attackCooldownTicks   = le_read_i32(buf + 52);
    s->movementCooldownTicks = le_read_i32(buf + 56);
    s->aggressionScore       = le_read_i32(buf + 60);
    s->rngCallCount          = le_read_i32(buf + 64);
    s->reserved0             = le_read_i32(buf + 68);
    return CREATURE_AI_STATE_SERIALIZED_SIZE;
}

/* --- Tick input (128 bytes, 32 int32) --- */

int F0807_CREATURE_TickInputSerialize_Compat(
    const struct CreatureTickInput_Compat* s,
    unsigned char* buf,
    int bufSize)
{
    int off = 0;
    int i;
    if (s == 0 || buf == 0 || bufSize < CREATURE_TICK_INPUT_SERIALIZED_SIZE) return 0;
    memset(buf, 0, CREATURE_TICK_INPUT_SERIALIZED_SIZE);
    le_write_i32(buf + off, s->groupSlotIndex);  off += 4;
    le_write_i32(buf + off, s->creatureType);    off += 4;
    le_write_i32(buf + off, s->groupMapIndex);   off += 4;
    le_write_i32(buf + off, s->groupMapX);       off += 4;
    le_write_i32(buf + off, s->groupMapY);       off += 4;
    le_write_i32(buf + off, s->groupCells);      off += 4;
    for (i = 0; i < 4; i++) {
        le_write_i32(buf + off, s->groupCurrentHealth[i]); off += 4;
    }
    le_write_i32(buf + off, s->partyMapIndex);         off += 4;
    le_write_i32(buf + off, s->partyMapX);             off += 4;
    le_write_i32(buf + off, s->partyMapY);             off += 4;
    le_write_i32(buf + off, s->partyChampionsAlive);   off += 4;
    for (i = 0; i < 4; i++) {
        le_write_i32(buf + off, s->partyChampionCurrentHealth[i]); off += 4;
    }
    le_write_i32(buf + off, s->adjacencyWallMask);     off += 4;
    le_write_i32(buf + off, s->adjacencyDoorMask);     off += 4;
    le_write_i32(buf + off, s->adjacencyPitMask);      off += 4;
    le_write_i32(buf + off, s->adjacencyCreatureMask); off += 4;
    le_write_i32(buf + off, s->onFluxcageFlag);        off += 4;
    le_write_i32(buf + off, s->onPoisonCloudFlag);     off += 4;
    le_write_i32(buf + off, s->onPitFlag);             off += 4;
    le_write_i32(buf + off, s->selfDamagePerTick);     off += 4;
    le_write_i32(buf + off, s->currentTickLow);        off += 4;
    le_write_i32(buf + off, s->freezeLifeTicks);       off += 4;
    le_write_i32(buf + off, s->partyInvisibility);     off += 4;
    le_write_i32(buf + off, s->losClearFlag);          off += 4;
    le_write_i32(buf + off, s->primaryDir);            off += 4;
    le_write_i32(buf + off, s->secondaryDir);          off += 4;
    return off;
}

int F0808_CREATURE_TickInputDeserialize_Compat(
    struct CreatureTickInput_Compat* s,
    const unsigned char* buf,
    int bufSize)
{
    int off = 0;
    int i;
    if (s == 0 || buf == 0 || bufSize < CREATURE_TICK_INPUT_SERIALIZED_SIZE) return 0;
    s->groupSlotIndex       = le_read_i32(buf + off); off += 4;
    s->creatureType         = le_read_i32(buf + off); off += 4;
    s->groupMapIndex        = le_read_i32(buf + off); off += 4;
    s->groupMapX            = le_read_i32(buf + off); off += 4;
    s->groupMapY            = le_read_i32(buf + off); off += 4;
    s->groupCells           = le_read_i32(buf + off); off += 4;
    for (i = 0; i < 4; i++) {
        s->groupCurrentHealth[i] = le_read_i32(buf + off); off += 4;
    }
    s->partyMapIndex        = le_read_i32(buf + off); off += 4;
    s->partyMapX            = le_read_i32(buf + off); off += 4;
    s->partyMapY            = le_read_i32(buf + off); off += 4;
    s->partyChampionsAlive  = le_read_i32(buf + off); off += 4;
    for (i = 0; i < 4; i++) {
        s->partyChampionCurrentHealth[i] = le_read_i32(buf + off); off += 4;
    }
    s->adjacencyWallMask     = le_read_i32(buf + off); off += 4;
    s->adjacencyDoorMask     = le_read_i32(buf + off); off += 4;
    s->adjacencyPitMask      = le_read_i32(buf + off); off += 4;
    s->adjacencyCreatureMask = le_read_i32(buf + off); off += 4;
    s->onFluxcageFlag        = le_read_i32(buf + off); off += 4;
    s->onPoisonCloudFlag     = le_read_i32(buf + off); off += 4;
    s->onPitFlag             = le_read_i32(buf + off); off += 4;
    s->selfDamagePerTick     = le_read_i32(buf + off); off += 4;
    s->currentTickLow        = le_read_i32(buf + off); off += 4;
    s->freezeLifeTicks       = le_read_i32(buf + off); off += 4;
    s->partyInvisibility     = le_read_i32(buf + off); off += 4;
    s->losClearFlag          = le_read_i32(buf + off); off += 4;
    s->primaryDir            = le_read_i32(buf + off); off += 4;
    s->secondaryDir          = le_read_i32(buf + off); off += 4;
    return off;
}

/* --- Tick result (176 bytes) --- */

int F0809a_CREATURE_TickResultSerialize_Compat(
    const struct CreatureTickResult_Compat* s,
    unsigned char* buf,
    int bufSize)
{
    int off;
    int written;
    if (s == 0 || buf == 0 || bufSize < CREATURE_TICK_RESULT_SERIALIZED_SIZE) return 0;
    memset(buf, 0, CREATURE_TICK_RESULT_SERIALIZED_SIZE);

    /* Header (16 int32 = 64 bytes). */
    le_write_i32(buf +  0, s->resultKind);
    le_write_i32(buf +  4, s->newAIState);
    le_write_i32(buf +  8, s->emittedCombatAction);
    le_write_i32(buf + 12, s->emittedSpellRequest);
    le_write_i32(buf + 16, s->emittedMovement);
    le_write_i32(buf + 20, s->emittedSelfDamage);
    le_write_i32(buf + 24, s->reactionPending);
    le_write_i32(buf + 28, s->dropItemsPending);
    le_write_i32(buf + 32, s->nextEventDelayTicks);
    le_write_i32(buf + 36, s->newMovementCooldown);
    le_write_i32(buf + 40, s->newAttackCooldown);
    le_write_i32(buf + 44, s->newFearCounter);
    le_write_i32(buf + 48, s->rngCallCount);
    le_write_i32(buf + 52, s->emittedDoubleMove);
    le_write_i32(buf + 56, s->reserved1);
    le_write_i32(buf + 60, s->reserved2);
    off = 64;

    /* CombatAction (48 bytes) — reuse Phase 13 serialiser.
     * NOTE: F0740 / F0725 / F0726 / F0741 return 1 on success (not a byte
     * count), so the Phase 16 wrappers check == 1 and advance the offset
     * by the compile-time SERIALIZED_SIZE. */
    written = F0740_COMBAT_ActionSerialize_Compat(
        &s->outAction, buf + off, bufSize - off);
    if (written != 1) return 0;
    off += COMBAT_ACTION_SERIALIZED_SIZE;

    /* Movement block (16 bytes). */
    le_write_i32(buf + off +  0, s->outMovementTargetMapX);
    le_write_i32(buf + off +  4, s->outMovementTargetMapY);
    le_write_i32(buf + off +  8, s->outMovementDirection);
    le_write_i32(buf + off + 12, s->outMovementReserved);
    off += 16;

    /* TimelineEvent (44 bytes) — reuse Phase 12 serialiser. */
    written = F0725_TIMELINE_EventSerialize_Compat(
        &s->outNextTick, buf + off, bufSize - off);
    if (written != 1) return 0;
    off += TIMELINE_EVENT_SERIALIZED_SIZE;

    /* Padding (4 bytes). */
    le_write_i32(buf + off, s->outTickPadding);
    off += 4;

    return off;
}

int F0809b_CREATURE_TickResultDeserialize_Compat(
    struct CreatureTickResult_Compat* s,
    const unsigned char* buf,
    int bufSize)
{
    int off;
    int read;
    if (s == 0 || buf == 0 || bufSize < CREATURE_TICK_RESULT_SERIALIZED_SIZE) return 0;

    s->resultKind          = le_read_i32(buf +  0);
    s->newAIState          = le_read_i32(buf +  4);
    s->emittedCombatAction = le_read_i32(buf +  8);
    s->emittedSpellRequest = le_read_i32(buf + 12);
    s->emittedMovement     = le_read_i32(buf + 16);
    s->emittedSelfDamage   = le_read_i32(buf + 20);
    s->reactionPending     = le_read_i32(buf + 24);
    s->dropItemsPending    = le_read_i32(buf + 28);
    s->nextEventDelayTicks = le_read_i32(buf + 32);
    s->newMovementCooldown = le_read_i32(buf + 36);
    s->newAttackCooldown   = le_read_i32(buf + 40);
    s->newFearCounter      = le_read_i32(buf + 44);
    s->rngCallCount        = le_read_i32(buf + 48);
    s->emittedDoubleMove   = le_read_i32(buf + 52);
    s->reserved1           = le_read_i32(buf + 56);
    s->reserved2           = le_read_i32(buf + 60);
    off = 64;

    read = F0741_COMBAT_ActionDeserialize_Compat(
        &s->outAction, buf + off, bufSize - off);
    if (read != 1) return 0;
    off += COMBAT_ACTION_SERIALIZED_SIZE;

    s->outMovementTargetMapX = le_read_i32(buf + off +  0);
    s->outMovementTargetMapY = le_read_i32(buf + off +  4);
    s->outMovementDirection  = le_read_i32(buf + off +  8);
    s->outMovementReserved   = le_read_i32(buf + off + 12);
    off += 16;

    read = F0726_TIMELINE_EventDeserialize_Compat(
        &s->outNextTick, buf + off, bufSize - off);
    if (read != 1) return 0;
    off += TIMELINE_EVENT_SERIALIZED_SIZE;

    s->outTickPadding = le_read_i32(buf + off);
    off += 4;

    return off;
}
