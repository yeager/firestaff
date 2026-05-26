/*
 * CSB V1 Monster System — Source-Locked Implementation
 *
 * Source-locked to:
 *   CSBWin CSB.h:2323-2375 (MONSTERDESC struct, 26 bytes)
 *   CSBWin Data.h:1093 (MonsterDescriptor[27])
 *   CSBWin Monster.cpp:111-1200 (CreateMonster, MonsterAttacks, DSA hooks)
 *   CSBWin Attack.cpp:2423 (Grey Lord spawn)
 *   CSBWin Chaos.cpp:791-804 (Grey Lord attack bytes)
 *   ReDMCSB GROUP.C:716-1807 (drops, sounds, attack dispatch)
 *   ReDMCSB DEFS.H:5618-5626, 1673-1680
 *
 * Design: Pure functions, no globals. All randomness via RngState_Compat.
 */

#include <string.h>
#include <stdlib.h>
#include "csb_v1_monster_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"  /* RngState_Compat */

/* ============================================================
 *  MONSTERDESC Parsing (26-byte binary format)
 *
 *  Source: CSB.h:2323-2375 · AsciiDump.cpp:1811-1855
 *  Note: All multi-byte fields are little-endian with byte-swapping
 *  on read (matching ReDMCSB DUNGEON.C LE16_SWAP pattern).
 * ============================================================ */

void csb_v1_monsterdesc_parse(const uint8_t *src,
                               CSB_V1_MonsterDesc *dst,
                               int idx) {
    if (!src || !dst) return;

    /* Validate creature type index */
    if (idx < 0 || idx >= CSB_CREATURE_TYPE_COUNT) {
        memset(dst, 0, sizeof(*dst));
        return;
    }

    dst->uByte0        = src[0];
    dst->attackSound   = src[1];
    /* word2: little-endian, byte-swapped on read (DEFS.H:swap pattern) */
    dst->word2         = (int16_t)((src[3] << 8) | src[2]);
    /* word4: little-endian, byte-swapped on read (MONSTERDESC_WORD4) */
    dst->word4         = (uint16_t)((src[5] << 8) | src[4]);
    dst->movementTicks06 = src[6];
    dst->attackTicks07  = src[7];
    dst->defense08      = src[8];
    dst->baseHealth09   = src[9];
    dst->attack10       = src[10];
    dst->poisonAttack11 = src[11];
    dst->dexterity12    = src[12];
    dst->unused13       = src[13];
    /* word14: little-endian */
    dst->word14         = (int16_t)((src[15] << 8) | src[14]);
    /* word16: little-endian */
    dst->word16         = (int16_t)((src[17] << 8) | src[16]);
    /* word18: little-endian */
    dst->word18         = (int16_t)((src[19] << 8) | src[18]);
    /* word20: little-endian */
    dst->word20         = (int16_t)((src[21] << 8) | src[20]);
    /* uByte22[4]: no swap needed */
    dst->uByte22[0] = src[22];
    dst->uByte22[1] = src[23];
    dst->uByte22[2] = src[24];
    dst->uByte22[3] = src[25];

    /* uByte0 should equal idx for valid entries */
    (void)idx; /* suppress unused warning if validation is removed */
}

int csb_v1_monsterdesc_sight_distance(const CSB_V1_MonsterDesc *m) {
    if (!m) return 0;
    return CSB_MON_DESC_SIGHT(m->word14);
}

int csb_v1_monsterdesc_smell_distance(const CSB_V1_MonsterDesc *m) {
    if (!m) return 0;
    return CSB_MON_DESC_SMELL(m->word14);
}

int csb_v1_monsterdesc_attack_range(const CSB_V1_MonsterDesc *m) {
    if (!m) return 0;
    return CSB_MON_DESC_ATTACK_RANGE(m->word14);
}

int csb_v1_monsterdesc_bravery(const CSB_V1_MonsterDesc *m) {
    if (!m) return 0;
    return CSB_MON_DESC_BRAVERY(m->word16);
}

/* ============================================================
 *  Defense Calculation
 *
 *  Source: CSBWin Attack.cpp defense calculation
 *          ReDMCSB CHAMPION.C F0311-F0321 damage pipeline
 * ============================================================ */

int csb_v1_monster_get_defense(const CSB_V1_MonsterDesc *m, int attack_type) {
    if (!m) return 0;
    /* CSBWin/Attack.cpp: defense = base + (type==4 ? base/2 : 0) */
    return m->defense08 + (attack_type == CSB_ATTACK_MATERIAL_PROJECTILE
                           ? m->defense08 / 2 : 0);
}

int csb_v1_attack_resolve(int damage, int defense) {
    int net = damage - defense;
    return (net < 0) ? 0 : net;
}

/* ============================================================
 *  Attack Parameters Building
 *
 *  Source: Monster.cpp:949-1130 (MonsterAttacks parameter building)
 * ============================================================ */

void csb_v1_attack_parameters_build(
    CSB_V1_AttackParameters *params,
    int monsterType,
    int monsterX,
    int monsterY,
    int dirToParty,
    int distToParty,
    int partyPos,
    int monsterIndex,
    int monsterLevel)
{
    if (!params) return;
    memset(params, 0, sizeof(*params));

    params->monsterType       = monsterType;
    params->monsterX          = monsterX;
    params->monsterY          = monsterY;
    params->directionToParty   = dirToParty;
    params->distanceToParty    = distToParty;
    params->monsterPos         = partyPos;
    params->monsterIndex       = monsterIndex;
    params->monsterLevel       = monsterLevel;
    params->monsterShouldLaunchMissile = 0;
    params->monsterShouldSteal  = 0;
    params->missileType        = 0;
    params->missileRange       = 0;
    params->missileDamage      = 0;
    params->missileDecayRate   = 8;  /* Always 8 in CSB: Monster.cpp:1124 */
    params->heroToDamage        = -1;
    params->missileOriginPosition = 0;
    params->attackSoundOrdinal  = 0;
    params->supressPoison       = -1;  /* not suppressed by default */
}

/* ============================================================
 *  Projectile Type Resolution
 *
 *  Source: Monster.cpp:1049-1080 (missileType switch)
 *
 *  CSB adds DispellMissile (RNZoSpell family) and ZoSpell projectiles
 *  that DM1 doesn't have. Vexirk and Lord Chaos also get a wider
 *  projectile random selection in CSB vs DM1.
 * ============================================================ */

int csb_v1_projectile_type_for_creature(int creatureType,
                                         struct RngState_Compat *rng) {
    int r;

    if (!rng) return CSB_PROJECTILE_FIREBALL;

    switch (creatureType) {

    case CSB_CREATURE_TYPE_VEXIRK:
    case CSB_CREATURE_TYPE_LORD_CHAOS:
        /* CSB: 50% Fireball, else random {DispellMissile, Lightning, PoisonCloud, ZoSpell} */
        r = F0732_COMBAT_RngRandom_Compat(rng, 2);
        if (r != 0) {
            /* 50% chance: random from 4 other types */
            r = F0732_COMBAT_RngRandom_Compat(rng, 4);
            switch (r) {
            case 0: return CSB_PROJECTILE_DISPELL_MISSILE;
            case 1: return CSB_PROJECTILE_LIGHTNING;
            case 2: return CSB_PROJECTILE_POISON_CLOUD;
            case 3: return CSB_PROJECTILE_ZO_SPELL;
            }
        }
        return CSB_PROJECTILE_FIREBALL;

    case CSB_CREATURE_TYPE_SWAMP_SLIME:
        /* CSB: Poison (RNPoison) — same as DM1 */
        return CSB_PROJECTILE_POISON;

    case CSB_CREATURE_TYPE_WIZARD_EYE:
        /* CSB: 7/8 Lightning, 1/8 ZoSpell (DM1 has 7/8 Lightning, 1/8 OpenDoor) */
        r = F0732_COMBAT_RngRandom_Compat(rng, 8);
        if (r == 0) return CSB_PROJECTILE_ZO_SPELL;
        return CSB_PROJECTILE_LIGHTNING;

    case CSB_CREATURE_TYPE_ZYTAZ:
        /* CSB: 50% PoisonCloud, 50% Fireball (DM1: Fireball only) */
        r = F0732_COMBAT_RngRandom_Compat(rng, 2);
        return (r == 0) ? CSB_PROJECTILE_POISON_CLOUD : CSB_PROJECTILE_FIREBALL;

    case CSB_CREATURE_TYPE_DEMON:
    case CSB_CREATURE_TYPE_RED_DRAGON:
        /* CSB: Fireball — same as DM1 */
        return CSB_PROJECTILE_FIREBALL;

    default:
        /* Default to Fireball (matches GROUP.C F0207 default fallthrough) */
        return CSB_PROJECTILE_FIREBALL;
    }
}

/* ============================================================
 *  Missile Range Computation
 *
 *  Source: Monster.cpp:1116-1124
 *
 *    baseRange = attack10 / 4 + 1
 *    range += random(range) + random(range)
 * ============================================================ */

int csb_v1_missile_range_compute(int attack_power,
                                  struct RngState_Compat *rng) {
    int range;

    if (!rng) return 0;

    range = attack_power / 4 + 1;
    if (range <= 0) range = 1;

    range += F0732_COMBAT_RngRandom_Compat(rng, range);
    range += F0732_COMBAT_RngRandom_Compat(rng, range);

    /* Clamp to reasonable range */
    if (range < 1) range = 1;
    if (range > 255) range = 255;

    return range;
}

/* ============================================================
 *  DSA Filter Support (skeleton — full DSA engine out of scope)
 *
 *  Source: Monster.cpp:1116-1180 (attack filter)
 *          Monster.cpp:3222-3370 (movement filter)
 *
 *  DSA filter invocation requires the full DSA script engine
 *  (ProcessDSAFilter, LoadLevel context restore, etc.).
 *  These stubs return 0 (no filter found) until DSA engine is available.
 * ============================================================ */

int csb_v1_dsa_filter_attack_preprocess(
    CSB_V1_AttackParameters *params,
    const struct CSB_V1_DungeonData_Compat *dungeon)
{
    /* Stub: DSA engine not yet implemented.
     * Full implementation requires:
     *   1. Locate DSA object at (EDT_SpecialLocations<<24)|ESL_MONSTERATTACKFILTER
     *   2. Verify it is an actuator of type 47
     *   3. Save current level, LoadLevel(filter level)
     *   4. Call ProcessDSAFilter(obj, &timer, locr, NULL, &dsaVars)
     *   5. Restore current level
     *   6. Copy modified attackParameters back
     * Currently returns 0 (no filter intercept).
     */
    (void)params;
    (void)dungeon;
    return 0;
}

int csb_v1_dsa_filter_movement_preprocess(
    int level,
    int mapX,
    int mapY,
    int32_t monster,
    int partyLevel,
    int partyX,
    int partyY,
    int flgs_inout[2],
    const struct CSB_V1_DungeonData_Compat *dungeon)
{
    /* Stub: DSA engine not yet implemented.
     * Full implementation requires:
     *   1. Look up mmfloc[level].filterObj
     *   2. If not RNeof, call ProcessDSAFilter with 7 parameters:
     *        [level, mapX, mapY, monsterID, partyLevel, partyX, partyY]
     *   3. Copy modified flgs[0]/flgs[1] back
     * Currently returns 0 (no filter intercept).
     */
    (void)level;
    (void)mapX;
    (void)mapY;
    (void)monster;
    (void)partyLevel;
    (void)partyX;
    (void)partyY;
    (void)dungeon;
    if (flgs_inout) {
        /* no-op: flags unchanged */
    }
    return 0;
}

/* ============================================================
 *  Fixed Possessions Drop
 *
 *  Source: GROUP.C:716-750 · DEFS.H:5618-5626
 *
 *  Uses creature type index to look up the appropriate drop table
 *  (G0245-G0253). Each table entry is an object index + cell + flags.
 *  Drop is placed at (mapX, mapY, cell) in the dungeon.
 * ============================================================ */

void csb_v1_drop_fixed_possessions(int creatureType,
                                   int mapX,
                                   int mapY,
                                   int cell,
                                   int mode)
{
    /* Stub: full drop implementation requires dungeon object placement.
     * GROUP.C F0186 handles:
     *   - MASK0x0200_DROP_FIXED_POSSESSIONS check on creature Attributes
     *   - Lookup of G0245-G0253 table for creature type
     *   - Object creation at cell position with appropriate thing type
     *   - Sound trigger (C00_SOUND_METALLIC_THUD or C04_SOUND_WOODEN_THUD)
     * Currently returns without action until dungeon object system is wired.
     */
    (void)creatureType;
    (void)mapX;
    (void)mapY;
    (void)cell;
    (void)mode;
}

/* ============================================================
 *  Source Evidence
 * ============================================================ */

const char *csb_v1_monster_source_evidence(void) {
    return
        "CSBWin/CSB.h:2323-2375 MONSTERDESC struct (26 bytes)\n"
        "CSBWin/Data.h:1093 MonsterDescriptor[27] array\n"
        "CSBWin/Monster.cpp:111 CreateMonster ASSERT(monsterType<27)\n"
        "CSBWin/Monster.cpp:923 ATTACK_PARAMETERS struct\n"
        "CSBWin/Monster.cpp:949 MonsterAttacks() attack parameter building\n"
        "CSBWin/Monster.cpp:1049-1080 projectile type switch (Vexirk/LordChaos/ZoSpell)\n"
        "CSBWin/Monster.cpp:3079-3176 DSA movement filter loading per level\n"
        "CSBWin/Monster.cpp:3222-3370 DSA movement filter execution\n"
        "CSBWin/Attack.cpp:2423 Grey Lord monsterType(mon_GreyLord) spawn\n"
        "CSBWin/Chaos.cpp:791-804 Grey Lord attack byte sequences\n"
        "ReDMCSB/DEFS.H:1339-1366 C00-C26_CREATURE_* enum, C027_CREATURE_TYPE_COUNT=27\n"
        "ReDMCSB/DEFS.H:1575-1594 CREATURE_INFO base struct\n"
        "ReDMCSB/DEFS.H:5618-5626 G0245-G0253 fixed possessions drop tables\n"
        "ReDMCSB/DEFS.H:1673-1680 C0-C5_ATTACK_* attack type enum\n"
        "ReDMCSB/GROUP.C:716 F0186_GROUP_DropCreatureFixedPossessions\n"
        "ReDMCSB/GROUP.C:1803-1807 G0244/G2003 creature attack/movement sounds\n"
        "ReDMCSB/GROUP.C:1645-1770 F0207_GROUP_IsCreatureAttacking projectile launch\n"
        "ReDMCSB/GROUP.C:769-932 F0190_GROUP_GetDamageCreatureOutcome\n"
        "ReDMCSB/CHAMPION.C:F0311-F0321 champion damage pipeline\n"
        "AsciiDump.cpp:1811-1855 DumpMonster full field listing\n"
        "BugsAndChanges.htm:CHANGE7_23 DSA filter additions\n"
        "BugsAndChanges.htm:CHANGE8_06 version 21 CSB 2.1\n"
    ;
}