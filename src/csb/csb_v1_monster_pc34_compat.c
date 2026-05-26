/*
 * CSB V1 Monster System — Source-Locked Implementation
 *
 * Source-locked to:
 *   CSBWin CSB.h:2323-2375 (MONSTERDESC struct, 26 bytes)
 *   CSBWin Data.h:1093 (MonsterDescriptor[27])
 *   CSBWin Monster.cpp:111-1200 (CreateMonster, MonsterAttacks, DSA hooks)
 *   CSBWin Attack.cpp:2423 (Grey Lord spawn)
 *   CSBWin Chaos.cpp:791-804 (Grey Lord attack bytes)
 *   ReDMCSB GROUP.C:550-932 (drops F0186, attack F0207, damage F0190)
 *   ReDMCSB DEFS.H:5618-5626 (G0245-G0253 drop tables)
 *   ReDMCSB DEFS.H:1673-1680 (C0-C7_ATTACK_* attack type enum)
 *   ReDMCSB DEFS.H:1339-1366 (C00-C26_CREATURE_* enum)
 *   ReDMCSB DEFS.H:1575-1594 (CREATURE_INFO base struct)
 *   ReDMCSB DEFS.H:63-133 (MEDIA529 I34E sound constants)
 *   ReDMCSB DEFS.H:3528-3531 (G2003 C0/C1_ATTACK/MOVEMENT_SOUND columns)
 *
 * Design: Pure functions, no globals. All randomness via RngState_Compat.
 *
 * CSB vs DM1 Differences (summary):
 *   - Grey Lord (0x1a): new named boss, C5_ATTACK_MAGIC
 *   - DSA Monster Attack Filter: script hook intercepting attack parameters
 *   - DSA Monster Movement Filter: per-level script hook for movement decisions
 *   - Extended projectile types: DispellMissile (0x03), ZoSpell (0x04)
 *   - Vexirk/Lord Chaos: 50/50 Fireball vs {DispellMissile, Lightning, PoisonCloud, ZoSpell}
 *   - Wizard Eye: 7/8 Lightning, 1/8 ZoSpell (DM1 uses 1/8 OpenDoor)
 *   - Zytaz: 50% PoisonCloud, 50% Fireball (DM1: Fireball only)
 *   - Sound table: G2003_aauc_CreatureSounds[18][2] (CSB) vs G0244[11] (DM1)
 *   - Drop tables: same G0245-G0253, same F0186 handling (identical)
 *   - Grey Lord BUG0_13: both CSB and DM1 have uninitialized projectile for Grey Lord
 *     (not reachable in any original dungeon; FIREBALL is safe fallback)
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
 *          DEFS.H:1673-1680 C0-C7_ATTACK_* types
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
 *          ReDMCSB GROUP.C:1685-1755 F0207_GROUP_IsCreatureAttacking
 *
 *  CSB vs DM1 differences:
 *    Vexirk/Lord Chaos: CSB uses {DispellMissile,Lightning,PoisonCloud,ZoSpell}
 *                       DM1 uses {HarmNonMaterial,Lightning,PoisonCloud,OpenDoor}
 *    Wizard Eye:        CSB uses 1/8 ZoSpell; DM1 uses 1/8 OpenDoor
 *    Zytaz:             CSB uses 50% PoisonCloud; DM1 uses Fireball only
 *
 *  Grey Lord / Lord Order BUG0_13: both CSB and DM1 fall through to
 *  uninitialized projectile thing. Not reachable in original dungeons;
 *  FIREBALL is the deterministic fallback.
 * ============================================================ */

int csb_v1_projectile_type_for_creature(int creatureType,
                                         struct RngState_Compat *rng) {
    int r;

    if (!rng) return CSB_PROJECTILE_FIREBALL;

    switch (creatureType) {

    case CSB_CREATURE_TYPE_VEXIRK:
    case CSB_CREATURE_TYPE_LORD_CHAOS:
        /*
         * CSB: 50% Fireball, else random from 4 CSB-specific projectiles.
         *   Source: Monster.cpp:1062-1075 · GROUP.C:1689-1705
         *   CSB projectiles: DispellMissile, Lightning, PoisonCloud, ZoSpell
         *   DM1 used: HarmNonMaterial, Lightning, PoisonCloud, OpenDoor
         *   Key difference: CSB adds DispellMissile (0x03) and ZoSpell (0x04);
         *   DM1 used HarmNonMaterial (0xFF83) and OpenDoor (0xFF84).
         */
        r = F0732_COMBAT_RngRandom_Compat(rng, 2);
        if (r != 0) {
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
        /* CSB: Poison (RNPoison 0x01) — same as DM1: GROUP.C:1710 */
        return CSB_PROJECTILE_POISON;

    case CSB_CREATURE_TYPE_WIZARD_EYE:
        /*
         * CSB: 7/8 Lightning, 1/8 ZoSpell
         *   Source: Monster.cpp:1076-1080 · GROUP.C:1712-1717
         *   DM1 difference: 7/8 Lightning, 1/8 OpenDoor (GROUP.C:1714)
         *   CSB replaces OpenDoor with ZoSpell (0x04) for Chaos magic flavor.
         */
        r = F0732_COMBAT_RngRandom_Compat(rng, 8);
        if (r == 0) return CSB_PROJECTILE_ZO_SPELL;
        return CSB_PROJECTILE_LIGHTNING;

    case CSB_CREATURE_TYPE_ZYTAZ:
        /*
         * CSB: 50% PoisonCloud, 50% Fireball
         *   Source: Monster.cpp:1060-1061 · GROUP.C:1707-1709
         *   DM1 difference: Fireball only (GROUP.C:1720 fallthrough)
         */
        r = F0732_COMBAT_RngRandom_Compat(rng, 2);
        return (r == 0) ? CSB_PROJECTILE_POISON_CLOUD : CSB_PROJECTILE_FIREBALL;

    case CSB_CREATURE_TYPE_DEMON:
    case CSB_CREATURE_TYPE_RED_DRAGON:
        /* CSB: Fireball — same as DM1: GROUP.C:1719-1722 */
        return CSB_PROJECTILE_FIREBALL;

    default:
        /*
         * Grey Lord (0x1a) / Lord Order (0x19): BUG0_13 in ReDMCSB GROUP.C.
         * Neither game has explicit projectile handling for these types.
         * The comment at GROUP.C:1723 explicitly states:
         *   "BUG0_13 The game may crash when 'Lord Order' or 'Grey Lord' cast
         *    spells. This cannot happen with the original dungeons as they do
         *    not contain any groups of these types."
         * SAFE_FALLBACK: FIREBALL matches MEDIA529 PC34 default behavior.
         */
        return CSB_PROJECTILE_FIREBALL;
    }
}

/* ============================================================
 *  Missile Range Computation
 *
 *  Source: Monster.cpp:1116-1124
 *          ReDMCSB GROUP.C:1730-1735 F0207 kinetic energy computation
 *
 *    baseRange = attack10 / 4 + 1
 *    range += random(range) + random(range)  [two additive randoms]
 *    result clamped to [1, 255]
 *
 *  This matches GROUP.C:1730-1735:
 *    AL0440_i_KineticEnergy = (L0441_ps_CreatureInfo->Attack >> 2) + 1;
 *    AL0440_i_KineticEnergy += M002_RANDOM(AL0440_i_KineticEnergy);
 *    AL0440_i_KineticEnergy += M002_RANDOM(AL0440_i_KineticEnergy);
 * ============================================================ */

int csb_v1_missile_range_compute(int attack_power,
                                  struct RngState_Compat *rng) {
    int range;

    if (!rng) return 0;

    range = attack_power / 4 + 1;
    if (range <= 0) range = 1;

    range += F0732_COMBAT_RngRandom_Compat(rng, range);
    range += F0732_COMBAT_RngRandom_Compat(rng, range);

    if (range < 1) range = 1;
    if (range > 255) range = 255;

    return range;
}

/* ============================================================
 *  Sound Constants (MEDIA529 I34E PC34 format)
 *
 *  Source: ReDMCSB DEFS.H:63-133 (I34E MEDIA529 format)
 *          ReDMCSB DEFS.H:3528-3531 (C0_ATTACK_SOUND=0, C1_MOVEMENT_SOUND=1)
 *          ReDMCSB GROUP.C:1807 (G2003_aauc_CreatureSounds[18][2] lookup)
 *
 *  CSB PC34 uses G2003_aauc_CreatureSounds[18][2] (18 ordinals × 2 columns).
 *  Column 0 = attack sound index, Column 1 = movement sound index.
 *  Ordinal 0 = no sound (CM1_SOUND_NONE = -1).
 *
 *  G2003 replaces the older G0244_auc_Graphic559_CreatureAttackSounds[11]
 *  that was used in older media versions.
 *
 *  The 18 attack/movement sound pairs (indexed by attackSoundOrdinal 1-18):
 *    [0] {C23_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON, C32_SOUND_MOVE_RED_DRAGON}
 *    [1] {C07_SOUND_ATTACK_MUMMY_GHOST_RIVE,            CM1_SOUND_NONE}
 *    [2] {C14_SOUND_ATTACK_SCREAMER_OITU,               C26_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE...}
 *    [3] {C15_SOUND_ATTACK_GIANT_SCORPION_SCORPION,     C26_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE...}
 *    [4] {C19_SOUND_ATTACK_MAGENTA_WORM_WORM,           C26_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE...}
 *    [5] {C21_SOUND_ATTACK_GIGGLER,                     C24_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM...}
 *    [6] {C29_SOUND_ATTACK_ROCK_ROCKPILE,               C26_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE...}
 *    [7] {C30_SOUND_ATTACK_WATER_ELEMENTAL,             C27_SOUND_MOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL}
 *    [8] {C31_SOUND_ATTACK_COUATL,                      C23_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER}
 *    [9] {C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, C24_SOUND_MOVE_MUMMY_TROLIN...}
 *   [10] {M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, C34_SOUND_MOVE_SKELETON}
 *   [11] {M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, C28_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT}
 *   [12] {C07_SOUND_ATTACK_MUMMY_GHOST_RIVE,            C24_SOUND_MOVE_MUMMY_TROLIN...}
 *   [13] {CM1_SOUND_NONE,                               C27_SOUND_MOVE_SWAMP_SLIME...}
 *   [14] {CM1_SOUND_NONE,                               C23_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER}
 *   [15] {CM1_SOUND_NONE,                               C24_SOUND_MOVE_MUMMY_TROLIN...}
 *   [16] {CM1_SOUND_NONE,                               C26_SOUND_MOVE_SCREAMER_ROCK...}
 *   [17] {C23_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON, C26_SOUND_MOVE_SCREAMER_ROCK...}
 *
 *  Source: DEFS.H:73-109 (sound constants), DEFS.H:3528-3531 (C0/C1 columns),
 *          GROUP.C:1807 G2003 lookup, AsciiDump.cpp:1811 G2003 initialization
 * ============================================================ */

/* Sound trigger for drop events (GROUP.C:645):
 *   Weapon dropped: C00_SOUND_METALLIC_THUD (0)
 *   No weapon: C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM (4)
 */
int csb_v1_drop_sound_for_item(int itemType) {
    /* itemType: DM1_DROP_THING_TYPE_WEAPON=5, ARMOUR=6, JUNK=10
     * Source: GROUP.C:645 F0064_SOUND_RequestPlay_CPSD */
    return (itemType == CSB_DROP_THING_TYPE_WEAPON)
        ? CSB_SOUND_METALLIC_THUD
        : CSB_SOUND_WOODEN_THUD;
}

/* ============================================================
 *  DSA Filter Support (skeleton — full DSA engine out of scope)
 *
 *  Source: Monster.cpp:1116-1180 (attack filter)
 *          Monster.cpp:3079-3176 (movement filter loading per level)
 *          Monster.cpp:3222-3370 (movement filter execution)
 *          BugsAndChanges.htm:CHANGE7_23 DSA filter additions
 *          BugsAndChanges.htm:CHANGE8_06 version 21 CSB 2.1
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
     *   6. Copy modified attackParameters back via memcpy
     * Source: Monster.cpp:1134-1180 · BugsAndChanges.htm:CHANGE7_23
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
     *   1. Look up mmfloc[level].filterObj (Monster.cpp:3100-3176)
     *   2. If not RNeof, call ProcessDSAFilter with 7 parameters:
     *        [level, mapX, mapY, monsterID, partyLevel, partyX, partyY]
     *   3. Copy modified flgs[0]/flgs[1] back
     * Source: Monster.cpp:3222-3370 · BugsAndChanges.htm:CHANGE7_23
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
        /* no-op: flags unchanged until DSA engine available */
    }
    return 0;
}

/* ============================================================
 *  Fixed Possessions Drop
 *
 *  Source: GROUP.C:550-648 F0186_GROUP_DropCreatureFixedPossessions
 *          DEFS.H:5618-5626 G0245-G0253 drop table declarations
 *          GROUP.C:644-648 weapon/armour/junk type resolution
 *
 *  CSB drop tables are identical to DM1 (same G0245-G0253 globals,
 *  same F0186 implementation). Creature types that drop possessions:
 *    Skeleton (C12):      G0245[3] — weapon or armour
 *    Stone Golem (C09):   G0246[2] — weapon or armour
 *    Trolin/AntMan (C16): G0247[2] — weapon or armour
 *    Animated Armour (C18): G0248[7] — full loadout (cursed)
 *    Rock/RockPile (C07): G0249[5] — junk
 *    Pain Rat (C04):      G0250[3] — junk
 *    Screamer (C06):      G0251[3] — junk
 *    Magenta Worm (C15):  G0252[4] — junk
 *    Red Dragon (C24):    G0253[11] — full loadout
 *
 *  Drop logic (GROUP.C:618-643):
 *    - Random drop flag (0x8000): 50% chance to skip entry
 *    - Object info index >= 127: type=junk, index -= 127
 *    - Object info index >= 69:  type=armour, index -= 69
 *    - Otherwise: type=weapon, index -= 23, weaponDropped=true
 *    - Cell: single-centered (0xFF) or random or P0358_ui_Cell
 *    - Animated Armour drops are CURSED (L0361_B_Cursed = true)
 * ============================================================ */

void csb_v1_drop_fixed_possessions(int creatureType,
                                   int mapX,
                                   int mapY,
                                   int cell,
                                   int mode)
{
    /* Stub: full drop implementation requires dungeon object placement.
     * GROUP.C F0186 handles:
     *   - Creature type switch → pointer to appropriate G0245-G0253 table
     *   - Animated Armour: cursed flag set
     *   - Each entry: random drop check, type resolution, thing allocation,
     *     cell assignment, F0267_MOVE_GetMoveResult_CPSCE placement
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
        "CSBWin/Monster.cpp:1049-1080 projectile type switch\n"
        "CSBWin/Monster.cpp:1116-1124 missile range computation\n"
        "CSBWin/Monster.cpp:3079-3176 DSA movement filter loading per level\n"
        "CSBWin/Monster.cpp:3222-3370 DSA movement filter execution\n"
        "CSBWin/Monster.cpp:1134-1180 DSA attack filter invocation\n"
        "CSBWin/Attack.cpp:2423 Grey Lord monsterType(mon_GreyLord) spawn\n"
        "CSBWin/Chaos.cpp:791-804 Grey Lord attack byte sequences\n"
        "ReDMCSB/DEFS.H:1339-1366 C00-C26_CREATURE_* enum, C027_CREATURE_TYPE_COUNT=27\n"
        "ReDMCSB/DEFS.H:1575-1594 CREATURE_INFO base struct\n"
        "ReDMCSB/DEFS.H:5618-5626 G0245-G0253 fixed possessions drop tables\n"
        "ReDMCSB/DEFS.H:1673-1680 C0-C7_ATTACK_* attack type enum (Grey Lord = C5)\n"
        "ReDMCSB/DEFS.H:63-133 MEDIA529 I34E PC34 sound constants\n"
        "ReDMCSB/DEFS.H:3528-3531 C0_ATTACK_SOUND=0, C1_MOVEMENT_SOUND=1\n"
        "ReDMCSB/DEFS.H:6510 extern char G2003_aauc_CreatureSounds[18][2]\n"
        "ReDMCSB/GROUP.C:550 F0186_GROUP_DropCreatureFixedPossessions\n"
        "ReDMCSB/GROUP.C:648 F0187_GROUP_DropMovingCreatureFixedPossessions\n"
        "ReDMCSB/GROUP.C:676 F0188_GROUP_DropGroupPossessions\n"
        "ReDMCSB/GROUP.C:769 F0190_GROUP_GetDamageCreatureOutcome\n"
        "ReDMCSB/GROUP.C:1013 F0193_GROUP_StealFromChampion (Giggler)\n"
        "ReDMCSB/GROUP.C:1645 F0207_GROUP_IsCreatureAttacking\n"
        "ReDMCSB/GROUP.C:1685-1723 projectile type switch + BUG0_13 Grey Lord note\n"
        "ReDMCSB/GROUP.C:1730-1735 kinetic energy = attack>>2 + 1 + rand + rand\n"
        "ReDMCSB/GROUP.C:1803-1807 G0244/G2003 creature attack/movement sounds\n"
        "ReDMCSB/CHAMPION.C:F0311-F0321 champion damage pipeline\n"
        "AsciiDump.cpp:1811-1855 DumpMonster full field listing\n"
        "AsciiDump.cpp:306-344 MonsterName() full 0x00-0x1a name table\n"
        "BugsAndChanges.htm:CHANGE7_23 DSA filter additions\n"
        "BugsAndChanges.htm:CHANGE8_06 version 21 CSB 2.1\n"
    ;
}