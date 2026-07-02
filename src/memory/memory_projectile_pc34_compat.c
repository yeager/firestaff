/*
 * Projectile & explosion flight data layer — Phase 17 of M10.
 *
 * Authoritative plan: PHASE17_PLAN.md. Function numbering F0810-F0829.
 *
 * Pure, caller-driven per-tick transforms:
 *   - PROJECTILE_MOVE handler (F0811) — mirror of F0219.
 *   - EXPLOSION_ADVANCE handler (F0822) — mirror of F0220.
 *
 * All randomness flows through Phase 13's RngState_Compat (F0732).
 * No globals, no UI, no IO. Cell-content data is supplied by the
 * caller via CellContentDigest_Compat.
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "memory_projectile_pc34_compat.h"
#include "memory_combat_pc34_compat.h"  /* F0192 resistance adjustment */
#include "memory_creature_ai_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "csb_v1_projectile_speed_pc34_compat.h"  /* CHANGE7_20 gate */

/* ==========================================================
 *  Platform + size asserts (MEDIA016 contract).
 * ========================================================== */

_Static_assert(sizeof(int) == 4, "Phase17 requires sizeof(int) == 4");
_Static_assert(sizeof(uint32_t) == 4, "Phase17 requires sizeof(uint32_t) == 4");

_Static_assert(sizeof(struct ProjectileInstance_Compat)
               == PROJECTILE_INSTANCE_SERIALIZED_SIZE,
               "ProjectileInstance_Compat size must be 96 bytes");
_Static_assert(sizeof(struct ExplosionInstance_Compat)
               == EXPLOSION_INSTANCE_SERIALIZED_SIZE,
               "ExplosionInstance_Compat size must be 64 bytes");
_Static_assert(sizeof(struct CellContentDigest_Compat)
               == CELL_CONTENT_DIGEST_SERIALIZED_SIZE,
               "CellContentDigest_Compat size must be 100 bytes");
_Static_assert(sizeof(struct ProjectileTickResult_Compat)
               == PROJECTILE_TICK_RESULT_SERIALIZED_SIZE,
               "ProjectileTickResult_Compat size must be 232 bytes");
_Static_assert(sizeof(struct ExplosionTickResult_Compat)
               == EXPLOSION_TICK_RESULT_SERIALIZED_SIZE,
               "ExplosionTickResult_Compat size must be 184 bytes");
_Static_assert(PROJECTILE_LIST_SERIALIZED_SIZE == 6008,
               "ProjectileList serialized size must be 6008 bytes (60 * 100 + 8)");
_Static_assert(EXPLOSION_LIST_SERIALIZED_SIZE == 2056,
               "ExplosionList serialized size must be 2056 bytes");


static int projectile_digest_door_is_destroyed(const struct CellContentDigest_Compat* digest)
{
    return digest && digest->destDoorState == PROJECTILE_DOOR_STATE_DESTROYED;
}

static int projectile_is_fireball_black_flame_impact(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest)
{
    return in && digest
        && in->projectileSubtype == PROJECTILE_SUBTYPE_FIREBALL
        && digest->destCreatureType == CREATURE_TYPE_BLACK_FLAME;
}

static int projectile_open_door_spell_impacts_door(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest)
{
    /* ReDMCSB PROJEXPL.C:F0217 door branch lines 471-489 checks
     * C0xFF84_THING_EXPLOSION_OPEN_DOOR before the normal destroyed/open/
     * pass-through door tests.  Therefore Open Door impacts every
     * non-destroyed door state, including open and one-quarter doors. */
    return in && digest
        && in->projectileSubtype == PROJECTILE_SUBTYPE_OPEN_DOOR
        && digest->destSquareType == PROJECTILE_ELEMENT_DOOR
        && digest->destDoorState != PROJECTILE_DOOR_STATE_NONE
        && !projectile_digest_door_is_destroyed(digest);
}

static int projectile_create_removes_potion_on_impact(
    const struct ProjectileCreateInput_Compat* in)
{
    unsigned int thing;
    if (!in) return 0;
    if (in->potionPower != 0) return 1;
    thing = (unsigned int)in->associatedThing;
    if (thing == THING_NONE || thing == THING_ENDOFLIST) return 0;
    if (THING_GET_TYPE(thing) != THING_TYPE_POTION) return 0;
    /* ReDMCSB: PROJEXPL.C F0217 lines 444-455 sets RemovePotion for
     * Ven and Ful Bomb by potion type, then uses potion Power only as
     * the explosion attack. A zero-power potion is still consumed. */
    return in->subtype == PROJECTILE_SUBTYPE_POISON_CLOUD ||
           in->subtype == PROJECTILE_SUBTYPE_FIREBALL;
}

enum {
    PHASE17_SOUND_METALLIC_THUD = 0, /* C00_SOUND_METALLIC_THUD */
    PHASE17_SOUND_WOODEN_THUD = 4 /* C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM */
};

/* ==========================================================
 *  Static tables.
 * ========================================================== */

/*
 * SUBTYPE_CREATES_EXPLOSION[256] — mirror of PROJEXPL.C:459
 * L0505_B_CreateExplosionOnImpact predicate. Slime / open-door are
 * the two explicit exclusions among magical subtypes.
 */
static const int Phase17_SubtypeCreatesExplosion[256] = {
    [PROJECTILE_SUBTYPE_FIREBALL]          = 1,
    [PROJECTILE_SUBTYPE_SLIME]             = 0,
    [PROJECTILE_SUBTYPE_LIGHTNING_BOLT]    = 1,
    [PROJECTILE_SUBTYPE_HARM_NON_MATERIAL] = 1,
    [PROJECTILE_SUBTYPE_OPEN_DOOR]         = 0,
    [PROJECTILE_SUBTYPE_POISON_BOLT]       = 1,
    [PROJECTILE_SUBTYPE_POISON_CLOUD]      = 1,
    /* all others (kinetic): 0 */
};

/*
 * Subtype → explosion type mapping used by F0820's
 * populate_explosion helper and F0822 frame table.
 */
static int Phase17_SubtypeToExplosionType(int subtype) {
    switch (subtype) {
    case PROJECTILE_SUBTYPE_FIREBALL:          return C000_EXPLOSION_FIREBALL;
    case PROJECTILE_SUBTYPE_SLIME:             return C001_EXPLOSION_SLIME;
    case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:    return C002_EXPLOSION_LIGHTNING_BOLT;
    case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL: return C003_EXPLOSION_HARM_NON_MATERIAL;
    case PROJECTILE_SUBTYPE_OPEN_DOOR:         return C004_EXPLOSION_OPEN_DOOR;
    case PROJECTILE_SUBTYPE_POISON_BOLT:       return C007_EXPLOSION_POISON_CLOUD;
    case PROJECTILE_SUBTYPE_POISON_CLOUD:      return C007_EXPLOSION_POISON_CLOUD;
    default:                                    return C000_EXPLOSION_FIREBALL;
    }
}

/*
 * Spell type → projectile subtype mapping. Only the five DM1
 * projectile spells. Entries not listed map to 0 (rejected).
 */
static int Phase17_SpellTypeToSubtype(int spellType) {
    switch (spellType) {
    case 0: return PROJECTILE_SUBTYPE_FIREBALL;        /* C0 FIREBALL */
    case 1: return PROJECTILE_SUBTYPE_LIGHTNING_BOLT;  /* C1 LIGHTNING */
    case 2: return PROJECTILE_SUBTYPE_POISON_BOLT;     /* C2 POISON_BOLT */
    case 3: return PROJECTILE_SUBTYPE_POISON_CLOUD;    /* C3 POISON_CLOUD */
    case C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR_COMPAT:
        return PROJECTILE_SUBTYPE_OPEN_DOOR;
    default: return 0;
    }
}

/*
 * Spell type → attack type code (COMBAT_ATTACK_*).
 */
static int Phase17_SpellTypeToAttackType(int spellType) {
    switch (spellType) {
    case 0: return COMBAT_ATTACK_FIRE;       /* fireball */
    case 1: return COMBAT_ATTACK_LIGHTNING;  /* lightning */
    case 2: return COMBAT_ATTACK_NORMAL;     /* poison bolt */
    case 3: return COMBAT_ATTACK_NORMAL;     /* poison cloud */
    case C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR_COMPAT:
        return COMBAT_ATTACK_MAGIC;          /* open door */
    default: return COMBAT_ATTACK_NORMAL;
    }
}

/*
 * Per-explosion-type default max-frame counts. Indexed 0..127.
 * Source-locked per ReDMCSB PROJEXPL.C:490-500 and CEDT030.C:232
 * (the "7C80" branch) — v1 carries approximate values for the
 * sanity cap only; visual frame selection is out of scope for the
 * data layer (rendering lives in the m11 renderer).  See
 * PROJEXPL.C:490-500 for the per-type animation-frame lookup
 * the original uses during render.
 */
static const int Phase17_ExplosionMaxFrames[128] = {
    [C000_EXPLOSION_FIREBALL]            = 3,
    [C001_EXPLOSION_SLIME]               = 3,
    [C002_EXPLOSION_LIGHTNING_BOLT]      = 2,
    [C003_EXPLOSION_HARM_NON_MATERIAL]   = 2,
    [C004_EXPLOSION_OPEN_DOOR]           = 2,
    [C007_EXPLOSION_POISON_CLOUD]        = 30,
    [C040_EXPLOSION_SMOKE]               = 30,
    [C050_EXPLOSION_FLUXCAGE]            = 1,
    [C100_EXPLOSION_REBIRTH_STEP1]       = 2,
    [C101_EXPLOSION_REBIRTH_STEP2]       = 2,
};

/* ==========================================================
 *  Local serialisation helpers — LSB-first int32.
 *  Phase 13-16 precedent: each .c file keeps its own static pair.
 * ========================================================== */

static void write_le_int32(unsigned char* buf, int32_t value) {
    uint32_t u = (uint32_t)value;
    buf[0] = (unsigned char)(u & 0xFFu);
    buf[1] = (unsigned char)((u >> 8) & 0xFFu);
    buf[2] = (unsigned char)((u >> 16) & 0xFFu);
    buf[3] = (unsigned char)((u >> 24) & 0xFFu);
}

static int32_t read_le_int32(const unsigned char* buf) {
    uint32_t u = (uint32_t)buf[0]
               | ((uint32_t)buf[1] << 8)
               | ((uint32_t)buf[2] << 16)
               | ((uint32_t)buf[3] << 24);
    return (int32_t)u;
}

/* ==========================================================
 *  Small utilities.
 * ========================================================== */

static int clamp_i(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/*
 * Derive a champion index (0..3) from a cell mask for an impacting
 * projectile. The projectile arrives on a specific cell `cell`; the
 * digest's destChampionCellMask has bit `cell` set iff a champion
 * is there. The champion index is the position of that cell in the
 * party layout — for v1 we use the cell ordinal itself, which
 * matches the party packing convention (cell 0..3 <-> champion 0..3).
 * Source-locked per ReDMCSB CHAMPION.C:1556-1620 (F0205_GROUP_SetDirection)
 * and the per-direction cell layout in DATA.C:225.  v1 omits the
 * per-party-direction rotation because DM1 PC 3.4 packs the party
 * with cell ordinal == champion index (leader at cell 0, etc.).
 * See CHAMPION.C:1556 for the original. */
static int champion_index_from_cell(
    const struct CellContentDigest_Compat* digest,
    int cell)
{
    (void)digest;
    if (cell < 0 || cell > 3) return 0;
    return cell & 3;
}

static int projectile_non_explosion_impact_sound_code(
    const struct ProjectileInstance_Compat* in)
{
    unsigned int associatedThing;
    /* ReDMCSB PROJEXPL.C:F0217 lines 587-600 selects
     * C00_SOUND_METALLIC_THUD only when the projectile associated thing
     * is C05_THING_TYPE_WEAPON; all other non-explosion impacts request
     * C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM. */
    if (in) {
        associatedThing = (unsigned int)in->reserved1;
        if (associatedThing != 0 &&
            associatedThing != THING_NONE &&
            associatedThing != THING_ENDOFLIST &&
            THING_GET_TYPE(associatedThing) == THING_TYPE_WEAPON) {
            return PHASE17_SOUND_METALLIC_THUD;
        }
        if (associatedThing != 0 &&
            associatedThing != THING_NONE &&
            associatedThing != THING_ENDOFLIST) {
            return PHASE17_SOUND_WOODEN_THUD;
        }
    }
    /* Legacy synthetic fixtures created before Projectile.Slot was carried
     * through reserved1 used the kinetic-arrow subtype as the local
     * weapon-backed analogue. Keep that narrow fallback for old save/test
     * blobs that still lack the associated Thing. */
    if (in && in->projectileCategory == PROJECTILE_CATEGORY_KINETIC
        && in->projectileSubtype == PROJECTILE_SUBTYPE_KINETIC_ARROW) {
        return PHASE17_SOUND_METALLIC_THUD;
    }
    return PHASE17_SOUND_WOODEN_THUD;
}

/* ==========================================================
 *  Group A — Projectile lifecycle (F0810 – F0813).
 * ========================================================== */

int F0810_PROJECTILE_Create_Compat(
    const struct ProjectileCreateInput_Compat* in,
    struct ProjectileList_Compat* list,
    int* outSlotIndex,
    struct TimelineEvent_Compat* outFirstMoveEvent)
{
    int slot;
    int i;
    int createsExplosion;

    if (in == NULL || list == NULL || outSlotIndex == NULL
        || outFirstMoveEvent == NULL) {
        return 0;
    }
    if (in->direction < 0 || in->direction > 3) return 0;
    if (in->cell < 0 || in->cell > 3) return 0;

    /* BUG0_16 v1 hard cap (plan §1 scope note).
     *
     * ReDMCSB PROJEXPL.C:F0220_EXPLOSION_ProcessEvents50To51 can
     * overfill the per-dungeon projectile list (676 in DM Atari ST
     * 1.0a, 690 in CSB). Original crashes; Firestaff hard-caps at
     * PROJECTILE_LIST_CAPACITY (PJE-05 audit, v2.7.x).
     *
     * The cap rejection is logged once per process so the divergence
     * is observable in long-running sessions.
     */
    if (list->count >= PROJECTILE_LIST_CAPACITY) {
        static int s_warned = 0;
        if (!s_warned) {
            s_warned = 1;
            fprintf(stderr,
                    "BUG0_16: projectile list full (count=%d, cap=%d). "
                    "Firestaff silently drops the overflow per PJE-05; "
                    "the original ReDMCSB F0220 would overflow its "
                    "per-dungeon list.\n",
                    list->count, PROJECTILE_LIST_CAPACITY);
        }
        return 0;
    }

    /* Empty iff the occupied-sentinel bit in reserved3 is clear. */
    slot = -1;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; i++) {
        if (list->entries[i].reserved3 == 0) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return 0;

    createsExplosion = Phase17_SubtypeCreatesExplosion[in->subtype & 0xFF];

    memset(&list->entries[slot], 0, sizeof(list->entries[slot]));
    list->entries[slot].slotIndex             = slot;
    list->entries[slot].projectileCategory    = in->category;
    list->entries[slot].projectileSubtype     = in->subtype;
    list->entries[slot].ownerKind             = in->ownerKind;
    list->entries[slot].ownerIndex            = in->ownerIndex;
    list->entries[slot].mapIndex              = in->mapIndex;
    list->entries[slot].mapX                  = in->mapX;
    list->entries[slot].mapY                  = in->mapY;
    list->entries[slot].cell                  = in->cell & 3;
    list->entries[slot].direction             = in->direction & 3;
    list->entries[slot].kineticEnergy         = clamp_i(in->kineticEnergy, 0, 255);
    list->entries[slot].attack                = clamp_i(in->attack, 0, 255);
    list->entries[slot].launcherStrength      = clamp_i(in->launcherStrength, 0, 255);
    list->entries[slot].stepEnergy            = clamp_i(in->stepEnergy, 0, 255);
    list->entries[slot].firstMoveGraceFlag    = (in->firstMoveGraceFlag != 0) ? 1 : 0;
    list->entries[slot].launchedAtTick        = in->currentTick;
    list->entries[slot].scheduledAtTick       = in->currentTick + 1;
    list->entries[slot].associatedPotionPower = in->potionPower;
    list->entries[slot].poisonAttack          = in->poisonAttack;
    /* ReDMCSB PROJEXPL.C F0216 lines 283-296 initializes
     * G0367_i_ProjectileAttackType to C3_ATTACK_BLUNT for thrown,
     * shot, and non-explosion object impacts before F0217 calls F0321.
     * Spell/explosion callers pass their explicit fire/magic/lightning
     * type, but bare kinetic callers must not silently become C0_NORMAL. */
    list->entries[slot].attackTypeCode        =
        (in->attackTypeCode == COMBAT_ATTACK_NORMAL &&
         in->category == PROJECTILE_CATEGORY_KINETIC)
            ? COMBAT_ATTACK_BLUNT
            : in->attackTypeCode;
    list->entries[slot].flags =
        (projectile_create_removes_potion_on_impact(in)
             ? PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT : 0) |
        (createsExplosion ? PROJECTILE_FLAG_CREATES_EXPLOSION : 0) |
        PROJECTILE_FLAG_IGNORE_DOOR_PASS_THROUGH;
    list->entries[slot].reserved0 = 0;
    list->entries[slot].reserved1 =
        (in->associatedThing != 0) ? in->associatedThing : (int)THING_NONE;
    list->entries[slot].reserved2 = 0;
    list->entries[slot].reserved3 = 1;   /* occupied sentinel */
    list->count++;
    *outSlotIndex = slot;

    memset(outFirstMoveEvent, 0, sizeof(*outFirstMoveEvent));
    outFirstMoveEvent->kind       = TIMELINE_EVENT_PROJECTILE_MOVE;
    outFirstMoveEvent->fireAtTick = (uint32_t)(in->currentTick + 1);
    outFirstMoveEvent->mapIndex   = in->mapIndex;
    outFirstMoveEvent->mapX       = in->mapX;
    outFirstMoveEvent->mapY       = in->mapY;
    outFirstMoveEvent->cell       = in->cell & 3;
    outFirstMoveEvent->aux0       = slot;
    outFirstMoveEvent->aux1       = in->ownerKind;
    outFirstMoveEvent->aux2       = in->ownerIndex;
    outFirstMoveEvent->aux3       = in->subtype;
    outFirstMoveEvent->aux4       = 0;
    return 1;
}

int F0813_PROJECTILE_Despawn_Compat(
    struct ProjectileList_Compat* list,
    int slotIndex)
{
    if (list == NULL) return 0;
    if (slotIndex < 0 || slotIndex >= PROJECTILE_LIST_CAPACITY) return 0;
    if (list->entries[slotIndex].reserved3 == 0) return 0; /* already empty */
    memset(&list->entries[slotIndex], 0, sizeof(list->entries[slotIndex]));
    list->entries[slotIndex].slotIndex = -1;
    if (list->count > 0) list->count--;
    return 1;
}

/* ==========================================================
 *  Group B — Cell-content inspection (F0814 – F0816).
 * ========================================================== */

int F0814_PROJECTILE_InspectDestination_Compat(
    const struct CellContentDigest_Compat* digest,
    int* outBlocker)
{
    if (digest == NULL || outBlocker == NULL) return 0;

    /* Priority order per plan §4.2. Highest wins. */
    if (digest->destIsMapBoundary) {
        *outBlocker = PROJECTILE_BLOCKER_BOUNDARY;
        return 1;
    }
    /* Wall-equivalent solids first (before creature/champion because
     * creature/champion cannot exist on a wall square). */
    if (digest->destSquareType == PROJECTILE_ELEMENT_WALL) {
        *outBlocker = PROJECTILE_BLOCKER_WALL;
        return 1;
    }
    if (digest->destSquareType == PROJECTILE_ELEMENT_FAKEWALL
        && !digest->destFakeWallIsImaginaryOrOpen) {
        *outBlocker = PROJECTILE_BLOCKER_WALL;
        return 1;
    }
    if (digest->destSquareType == PROJECTILE_ELEMENT_STAIRS
        && digest->sourceSquareType == PROJECTILE_ELEMENT_STAIRS) {
        *outBlocker = PROJECTILE_BLOCKER_STAIRS;
        return 1;
    }

    /* Champion / creature / other projectile on destination cell. */
    if (digest->destHasChampion) {
        *outBlocker = PROJECTILE_BLOCKER_OPEN; /* caller handles champion in F0820 */
        return 1;
    }
    if (digest->destHasCreatureGroup) {
        *outBlocker = PROJECTILE_BLOCKER_OPEN;
        return 1;
    }
    if (digest->destHasOtherProjectile) {
        *outBlocker = PROJECTILE_BLOCKER_OTHER_PROJECTILE;
        return 1;
    }

    /* Fluxcage and closed door. */
    if (digest->destHasFluxcage) {
        *outBlocker = PROJECTILE_BLOCKER_FLUXCAGE;
        return 1;
    }
    if (digest->destSquareType == PROJECTILE_ELEMENT_DOOR
        && digest->destDoorState != PROJECTILE_DOOR_STATE_NONE
        && !projectile_digest_door_is_destroyed(digest)
        && digest->destDoorState >= PROJECTILE_DOOR_STATE_CLOSED_HALF) {
        *outBlocker = PROJECTILE_BLOCKER_CLOSED_DOOR;
        return 1;
    }

    *outBlocker = PROJECTILE_BLOCKER_OPEN;
    return 1;
}

int F0815_PROJECTILE_ComputeImpactAttack_Compat(
    const struct ProjectileInstance_Compat* in,
    int* outAttack)
{
    if (in == NULL || outAttack == NULL) return 0;
    /* Mirror of F0216: attack-value base = in->attack. Magical
     * halving (lightning / poison-bolt) is caller's / F0823's job
     * at damage-roll time. Plan §4.8. */
    *outAttack = in->attack;
    return 1;
}

int F0816_PROJECTILE_DoesPassThroughDoor_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    struct RngState_Compat* rng,
    int* outPasses)
{
    if (in == NULL || digest == NULL || outPasses == NULL) return 0;

    *outPasses = 0;

    /* Destroyed or fully open door — passes trivially. */
    if (projectile_digest_door_is_destroyed(digest)
        || digest->destDoorState == PROJECTILE_DOOR_STATE_OPEN
        || digest->destDoorState == PROJECTILE_DOOR_STATE_CLOSED_ONE_FOURTH) {
        *outPasses = 1;
        return 1;
    }

    /* Magical projectile vs a door flagged
     * MASK0x0002_PROJECTILES_CAN_PASS_THROUGH. Source parity:
     * ReDMCSB PROJEXPL.C:F0217 door branch lines 485-505 handles
     * OPEN_DOOR before the pass-through branch (it schedules a
     * C10_EVENT_DOOR/C02_EFFECT_TOGGLE when DOOR->Button is set).
     * The pass-through branch only lets explosion-backed projectiles
     * with associated thing >= C0xFF83 pass; exclude OPEN_DOOR here
     * so it impacts the door instead of silently flying through a
     * portcullis-type door. Lightning (0x82) still strikes. */
    if (in->projectileCategory == PROJECTILE_CATEGORY_MAGICAL
        && digest->destDoorAllowsProjectilePassThrough
        && in->projectileSubtype >= PROJECTILE_SUBTYPE_HARM_NON_MATERIAL
        && in->projectileSubtype != PROJECTILE_SUBTYPE_OPEN_DOOR) {
        *outPasses = 1;
        return 1;
    }

    /* ReDMCSB PROJEXPL.C:F0217 lines 493-501: a kinetic associated
     * object may pass only through a door whose DoorInfo has
     * MASK0x0002, only when Projectile->Attack beats M003_RANDOM(128),
     * and only when ObjectInfo.AllowedSlots has MASK0x0100. PC 3.4 then
     * adds the key-icon rejection below. reserved2 is a transient
     * ObjectInfo.AllowedSlots payload filled by the M10 orchestrator. */
    if (in->projectileCategory == PROJECTILE_CATEGORY_KINETIC
        && digest->destDoorAllowsProjectilePassThrough
        && (in->reserved2 &
            PROJECTILE_ASSOCIATED_ALLOWED_SLOTS_POUCH_PASS_THROUGH_DOORS) != 0
        && digest->destDoorState != PROJECTILE_DOOR_STATE_DESTROYED) {
        if (in->reserved0 >= PROJECTILE_ASSOCIATED_ICON_IRON_KEY &&
            in->reserved0 <= PROJECTILE_ASSOCIATED_ICON_MASTER_KEY) {
            /* ReDMCSB PROJEXPL.C:F0217 lines 496-501, PC 3.4
             * CHANGE2_04: required keys (C176..C191) cannot pass through a
             * closed door even when the door allows projectile pass-through. */
            *outPasses = 0;
            return 1;
        }
        unsigned int roll = F0732_COMBAT_RngRandom_Compat(rng, 128);
        if ((unsigned int)in->attack > roll) {
            *outPasses = 1;
            return 1;
        }
    }
    *outPasses = 0;
    return 1;
}

/* ==========================================================
 *  Group C — Collision resolution (F0817 – F0820).
 * ========================================================== */

static int projectile_build_hit_creature_action_with_rng(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactAttack,
    struct RngState_Compat* rng,
    struct CombatAction_Compat* outAction)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    int defense;
    int scaledAttack;
    int poisonAttack;
    if (in == NULL || digest == NULL || outAction == NULL) return 0;
    memset(outAction, 0, sizeof(*outAction));
    profile = CREATURE_GetProfile_Compat(digest->destCreatureType);
    defense = profile ? profile->baseDefense : 0;
    if (defense <= 0) defense = 1;
    if (projectile_is_fireball_black_flame_impact(in, digest)) {
        scaledAttack = impactAttack;
    } else {
        scaledAttack = (int)(((long)impactAttack << 6) / defense);
    }
    if (scaledAttack < 0) scaledAttack = 0;
    poisonAttack = 0;
    if (scaledAttack > 0) {
        (void)F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
            digest->destCreatureType, in->poisonAttack, rng, &poisonAttack);
    }
    outAction->kind                          = COMBAT_ACTION_APPLY_DAMAGE_GROUP;
    outAction->allowedWounds                 = 0;  /* group damage not wound-slotted */
    outAction->attackTypeCode                = in->attackTypeCode;
    /* ReDMCSB PROJEXPL.C F0217 lines 535-537 scales projectile
     * impact by CreatureInfo.Defense, then adds F0192's
     * resistance-adjusted projectile poison attack.  The public F0817
     * wrapper has no RNG parameter and uses F0192's documented
     * no-RNG lower-bound bump of zero; F0820 runtime calls this helper
     * with the live RNG. */
    outAction->rawAttackValue                = scaledAttack + poisonAttack;
    outAction->targetMapIndex                = digest->destMapIndex;
    outAction->targetMapX                    = digest->destMapX;
    outAction->targetMapY                    = digest->destMapY;
    outAction->targetCell                    = in->cell & 3;
    outAction->attackerSlotOrCreatureIndex   = in->ownerIndex;
    outAction->defenderSlotOrCreatureIndex   = digest->destCreatureType;
    outAction->scheduleDelayTicks            = 0;
    outAction->flags                         = 0;
    return 1;
}

int F0817_PROJECTILE_BuildHitCreatureAction_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactAttack,
    struct CombatAction_Compat* outAction)
{
    return projectile_build_hit_creature_action_with_rng(
        in, digest, impactAttack, NULL, outAction);
}

int F0818_PROJECTILE_BuildHitChampionAction_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactAttack,
    int championIndex,
    struct CombatAction_Compat* outAction)
{
    if (in == NULL || digest == NULL || outAction == NULL) return 0;
    if (championIndex < 0 || championIndex > 3) return 0;

    memset(outAction, 0, sizeof(*outAction));
    outAction->kind                          = COMBAT_ACTION_APPLY_DAMAGE_CHAMPION;
    /* PROJEXPL.C:550 — projectile hits only HEAD/TORSO slots.
     * Phase 13's mask values are HEAD=0x0002, TORSO=0x0004. */
    outAction->allowedWounds                 = COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO;
    outAction->attackTypeCode                = in->attackTypeCode;
    outAction->rawAttackValue                = impactAttack;
    outAction->targetMapIndex                = digest->destMapIndex;
    outAction->targetMapX                    = digest->destMapX;
    outAction->targetMapY                    = digest->destMapY;
    outAction->targetCell                    = championIndex & 3;
    outAction->attackerSlotOrCreatureIndex   = in->ownerIndex;
    outAction->defenderSlotOrCreatureIndex   = championIndex;
    outAction->scheduleDelayTicks            = 0;
    outAction->flags                         = 0;
    return 1;
}

int F0819_PROJECTILE_BuildDoorDestructionEvent_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactAttack,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct TimelineEvent_Compat* outEvent)
{
    int roll = 0;
    if (in == NULL || digest == NULL || outEvent == NULL) return 0;

    if (rng != NULL && impactAttack > 0) {
        roll = F0732_COMBAT_RngRandom_Compat(rng, impactAttack);
    }

    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind       = TIMELINE_EVENT_DOOR_DESTRUCTION;
    outEvent->fireAtTick = currentTick + 1u;
    outEvent->mapIndex   = digest->destMapIndex;
    outEvent->mapX       = digest->destMapX;
    outEvent->mapY       = digest->destMapY;
    outEvent->cell       = in->cell & 3;
    outEvent->aux0       = impactAttack + roll;
    outEvent->aux1       = in->projectileSubtype;
    outEvent->aux2       = in->ownerIndex;
    outEvent->aux3       = in->ownerKind;
    outEvent->aux4       = 0;
    return 1;
}

/*
 * Private helper: populate an ExplosionInstance_Compat in-place as
 * side output of a magical-projectile hit. Slot-management is not
 * Phase 17's concern here — caller decides whether to actually push
 * it into an ExplosionList_Compat via F0821.
 */
static int projectile_impact_explosion_attack(
    const struct ProjectileInstance_Compat* in)
{
    int attack;
    if (!in) return 0;
    attack = in->kineticEnergy;
    if (in->projectileSubtype == PROJECTILE_SUBTYPE_POISON_BOLT) {
        attack >>= 2;
    } else if (in->projectileSubtype == PROJECTILE_SUBTYPE_LIGHTNING_BOLT) {
        attack >>= 1;
    }
    return attack;
}

static int populate_explosion_on_impact(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    struct ExplosionInstance_Compat* out)
{
    int explType = Phase17_SubtypeToExplosionType(in->projectileSubtype);
    int removePotion = (in->flags & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT) != 0;
    int explosionAttack = removePotion
                          ? in->associatedPotionPower
                          : projectile_impact_explosion_attack(in);
    if (!removePotion && explosionAttack <= 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->slotIndex             = -1;        /* caller assigns */
    out->explosionType         = explType;
    out->mapIndex              = digest->destMapIndex;
    out->mapX                  = digest->destMapX;
    out->mapY                  = digest->destMapY;
    out->cell                  = (explType == C007_EXPLOSION_POISON_CLOUD)
                                  ? EXPLOSION_CELL_CENTERED : (in->cell & 3);
    out->centered              = (out->cell == EXPLOSION_CELL_CENTERED) ? 1 : 0;
    /* ReDMCSB PROJEXPL.C F0217 lines 444-455 and 565-584 creates
     * thrown Ven/Ful potion explosions from potion Power. Other impact
     * explosions use projectile KineticEnergy, with Lightning / 2 and
     * Poison Bolt / 4; if that adjusted value is zero the original
     * deletes the projectile without creating the explosion. */
    out->attack                = explosionAttack;
    out->currentFrame          = 0;
    out->maxFrames             = (explType >= 0 && explType < 128)
                                  ? Phase17_ExplosionMaxFrames[explType] : 1;
    out->poisonAttack          = in->poisonAttack;
    out->scheduledAtTick       = 0;         /* caller sets */
    out->ownerKind             = in->ownerKind;
    out->ownerIndex            = in->ownerIndex;
    out->creatorProjectileSlot = in->slotIndex;
    out->reserved0             = 0;
    return 1;
}

int F0820_PROJECTILE_ResolveCollision_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactTarget,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct ProjectileTickResult_Compat* outResult)
{
    int impactAttack = 0;
    int createsExplosion;

    if (in == NULL || digest == NULL || outResult == NULL) return 0;

    memset(&outResult->outAction,    0, sizeof(outResult->outAction));
    memset(&outResult->outExplosion, 0, sizeof(outResult->outExplosion));
    memset(&outResult->outNextTick,  0, sizeof(outResult->outNextTick));
    outResult->emittedCombatAction         = 0;
    outResult->emittedExplosion            = 0;
    outResult->emittedDoorDestructionEvent = 0;
    outResult->emittedDoorToggleEvent       = 0;
    outResult->emittedSoundRequest          = 0;
    outResult->emittedSoundCode            = 0;

    F0815_PROJECTILE_ComputeImpactAttack_Compat(in, &impactAttack);

    createsExplosion = ((in->projectileCategory == PROJECTILE_CATEGORY_MAGICAL)
                        && Phase17_SubtypeCreatesExplosion[in->projectileSubtype & 0xFF])
                       || ((in->flags & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT) != 0);

    switch (impactTarget) {
    case PROJECTILE_RESULT_HIT_CHAMPION: {
        int championIndex = champion_index_from_cell(digest, in->cell);
        outResult->resultKind = PROJECTILE_RESULT_HIT_CHAMPION;
        F0818_PROJECTILE_BuildHitChampionAction_Compat(
            in, digest, impactAttack, championIndex, &outResult->outAction);
        outResult->emittedCombatAction = 1;
        if (createsExplosion) {
            outResult->emittedExplosion = populate_explosion_on_impact(
                in, digest, &outResult->outExplosion);
        }
        outResult->despawn = 1;
        return 1;
    }
    case PROJECTILE_RESULT_HIT_CREATURE:
        if (!projectile_is_fireball_black_flame_impact(in, digest)
            && digest->destCreatureIsNonMaterial
            && in->projectileSubtype != PROJECTILE_SUBTYPE_HARM_NON_MATERIAL) {
            /* Defensive parity fallback for direct F0820 callers. F0217
             * returns FALSE for this case (PROJEXPL.C:532-533), so the
             * F0219 caller keeps the projectile alive rather than applying
             * creature damage or despawning it. F0811 handles the full
             * movement/reschedule path before dispatch. */
            outResult->resultKind = PROJECTILE_RESULT_FLEW;
            outResult->despawn    = 0;
            return 1;
        }
        outResult->resultKind = PROJECTILE_RESULT_HIT_CREATURE;
        projectile_build_hit_creature_action_with_rng(
            in, digest, impactAttack, rng, &outResult->outAction);
        outResult->emittedCombatAction = 1;
        if (createsExplosion
            && !projectile_is_fireball_black_flame_impact(in, digest)) {
            /* ReDMCSB PROJEXPL.C:F0217 lines 527-531 heals Black Flame
             * on Fireball impact and jumps to T0217044, skipping the
             * normal explosion/sound side effects before projectile delete. */
            outResult->emittedExplosion = populate_explosion_on_impact(
                in, digest, &outResult->outExplosion);
        }
        outResult->despawn = 1;
        return 1;
    case PROJECTILE_RESULT_HIT_DOOR: {
        if (in->projectileSubtype != PROJECTILE_SUBTYPE_OPEN_DOOR) {
            int passes = 0;
            F0816_PROJECTILE_DoesPassThroughDoor_Compat(in, digest, rng, &passes);
            if (passes) {
                outResult->resultKind = PROJECTILE_RESULT_FLEW;
                outResult->despawn    = 0;
                /* Caller re-enqueues next PROJECTILE_MOVE normally. */
                return 1;
            }
        }
        outResult->resultKind = PROJECTILE_RESULT_HIT_DOOR;
        if (in->projectileSubtype == PROJECTILE_SUBTYPE_OPEN_DOOR) {
            outResult->emittedSoundRequest = 1;
            outResult->emittedSoundCode = PHASE17_SOUND_WOODEN_THUD;
            /* ReDMCSB PROJEXPL.C:F0217 lines 485-489: an OPEN_DOOR
             * projectile does not use the door-destruction attack path;
             * if DOOR->Button is set it enqueues
             * F0268_SENSOR_AddEvent(C10_EVENT_DOOR, x, y, 0,
             * C02_EFFECT_TOGGLE, GameTime+1). Without the button bit
             * ReDMCSB just breaks from F0217: no destruction event, no
             * sensor event, projectile consumed. F0217 then falls through
             * to the non-explosion impact sound path at PROJEXPL.C:587-600,
             * which requests C04_SOUND_WOODEN_THUD for this non-weapon
             * associated thing. */
            if (!digest->destDoorHasButton) {
                outResult->despawn = 1;
                return 1;
            }
            memset(&outResult->outNextTick, 0, sizeof(outResult->outNextTick));
            outResult->outNextTick.kind       = TIMELINE_EVENT_SENSOR_DELAYED;
            outResult->outNextTick.fireAtTick = currentTick + 1u;
            outResult->outNextTick.mapIndex   = digest->destMapIndex;
            outResult->outNextTick.mapX       = digest->destMapX;
            outResult->outNextTick.mapY       = digest->destMapY;
            outResult->outNextTick.cell       = 0;
            outResult->outNextTick.aux0       = 10; /* C10_EVENT_DOOR */
            outResult->outNextTick.aux1       = 2;  /* C02_EFFECT_TOGGLE */
            outResult->outNextTick.aux2       = in->projectileSubtype;
            outResult->outNextTick.aux3       = in->ownerIndex;
            outResult->outNextTick.aux4       = in->ownerKind;
            outResult->emittedDoorToggleEvent = 1;
        } else {
            /* ReDMCSB PROJEXPL.C:F0217 line 506 calls
             * F0232_GROUP_IsDoorDestroyedByAttack with
             * F0216_PROJECTILE_GetImpactAttack(...) + 1. */
            F0819_PROJECTILE_BuildDoorDestructionEvent_Compat(
                in, digest, impactAttack + 1, currentTick, rng,
                &outResult->outNextTick);
            outResult->emittedDoorDestructionEvent = 1;
            if (createsExplosion) {
                outResult->emittedExplosion = populate_explosion_on_impact(
                    in, digest, &outResult->outExplosion);
            } else {
                outResult->emittedSoundRequest = 1;
                outResult->emittedSoundCode =
                    projectile_non_explosion_impact_sound_code(in);
            }
        }
        outResult->despawn = 1;
        return 1;
    }
    case PROJECTILE_RESULT_HIT_WALL:
        outResult->resultKind = PROJECTILE_RESULT_HIT_WALL;
        if (createsExplosion) {
            outResult->emittedExplosion = populate_explosion_on_impact(
                in, digest, &outResult->outExplosion);
        } else {
            outResult->emittedSoundRequest = 1;
            outResult->emittedSoundCode =
                projectile_non_explosion_impact_sound_code(in);
        }
        outResult->despawn = 1;
        return 1;
    case PROJECTILE_RESULT_HIT_FLUXCAGE:
        outResult->resultKind = PROJECTILE_RESULT_HIT_FLUXCAGE;
        outResult->despawn    = 1;
        return 1;
    case PROJECTILE_RESULT_HIT_OTHER_PROJECTILE:
        outResult->resultKind = PROJECTILE_RESULT_HIT_OTHER_PROJECTILE;
        /* ReDMCSB PROJEXPL.C:F0218 lines 621-638 calls F0217 for each
         * same-cell projectile impact. F0217 then reaches the common
         * impact tail at lines 560-600, so projectile-vs-projectile impacts
         * still create spell explosions or request mundane thud sounds before
         * deleting the projectile. */
        if (createsExplosion) {
            outResult->emittedExplosion = populate_explosion_on_impact(
                in, digest, &outResult->outExplosion);
        } else {
            outResult->emittedSoundRequest = 1;
            outResult->emittedSoundCode =
                projectile_non_explosion_impact_sound_code(in);
        }
        outResult->despawn    = 1;
        return 1;
    default:
        outResult->resultKind = PROJECTILE_RESULT_INVALID;
        outResult->despawn    = 1;
        return 0;
    }
}

/* ==========================================================
 *  Group E (scheduling) — F0825 / F0826.
 *  Declared out of order so F0811 can call F0825.
 * ========================================================== */

int F0825_PROJECTILE_ScheduleNextMove_Compat(
    const struct ProjectileInstance_Compat* in,
    int onPartyMap,
    uint32_t currentTick,
    struct TimelineEvent_Compat* outEvent)
{
    int delay;
    if (in == NULL || outEvent == NULL) return 0;
    /* PROJEXPL.C CHANGE7_20_IMPROVEMENT branch: +1 on party map, +3
     * elsewhere. CSB V1 closes this gap (full speed on ALL maps).
     * The CSB gate is opt-in via the projectile-speed-normalization
     * flag (csb_v1_projectile_speed_normalization_get).  When the
     * flag is on, delay is 1 regardless of onPartyMap.  Hard
     * clamp to >=1 (loop-guard) in both branches. */
    if (csb_v1_projectile_speed_normalization_get()) {
        delay = 1;
    } else {
        delay = onPartyMap ? 1 : 3;
    }
    if (delay < 1) delay = 1;

    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind       = TIMELINE_EVENT_PROJECTILE_MOVE;
    outEvent->fireAtTick = currentTick + (uint32_t)delay;
    outEvent->mapIndex   = in->mapIndex;
    outEvent->mapX       = in->mapX;
    outEvent->mapY       = in->mapY;
    outEvent->cell       = in->cell & 3;
    outEvent->aux0       = in->slotIndex;
    outEvent->aux1       = in->ownerKind;
    outEvent->aux2       = in->ownerIndex;
    outEvent->aux3       = in->projectileSubtype;
    outEvent->aux4       = 0;
    return 1;
}

int F0826_EXPLOSION_ScheduleNextAdvance_Compat(
    const struct ExplosionInstance_Compat* in,
    uint32_t currentTick,
    int forcedDelay,
    struct TimelineEvent_Compat* outEvent)
{
    int delay;
    if (in == NULL || outEvent == NULL) return 0;
    delay = (forcedDelay < 1) ? 1 : forcedDelay;

    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind       = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    outEvent->fireAtTick = currentTick + (uint32_t)delay;
    outEvent->mapIndex   = in->mapIndex;
    outEvent->mapX       = in->mapX;
    outEvent->mapY       = in->mapY;
    outEvent->cell       = (in->cell & 0xFF);
    outEvent->aux0       = in->slotIndex;
    outEvent->aux1       = in->explosionType;
    outEvent->aux2       = in->attack;
    outEvent->aux3       = in->ownerKind;
    outEvent->aux4       = in->ownerIndex;
    return 1;
}

/* ==========================================================
 *  F0811 — primary per-tick projectile advance.
 *  Mirror of F0219_PROJECTILE_ProcessEvents48To49 (PROJEXPL.C:644).
 * ========================================================== */

int F0811_PROJECTILE_Advance_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct ProjectileInstance_Compat* outNewState,
    struct ProjectileTickResult_Compat* outResult)
{
    int crossesCell = 0;
    int newCell;
    int blocker = PROJECTILE_BLOCKER_OPEN;
    int dispatch = -1;
    int decrementedBeforeImpact = 0;

    if (in == NULL || digest == NULL || outNewState == NULL || outResult == NULL) {
        return 0;
    }
    if (in->direction < 0 || in->direction > 3
        || in->cell < 0 || in->cell > 3) {
        memset(outResult, 0, sizeof(*outResult));
        outResult->resultKind = PROJECTILE_RESULT_INVALID;
        outResult->despawn    = 1;
        return 0;
    }

    memset(outResult, 0, sizeof(*outResult));
    *outNewState = *in;

    /* Populate trivial "new*" fields up-front so non-move paths
     * still report a self-consistent state. */
    outResult->newCell               = in->cell;
    outResult->newMapIndex           = in->mapIndex;
    outResult->newMapX               = in->mapX;
    outResult->newMapY               = in->mapY;
    outResult->newDirection          = in->direction;
    outResult->newKineticEnergy      = in->kineticEnergy;
    outResult->newAttack             = in->attack;
    outResult->newFirstMoveGraceFlag = in->firstMoveGraceFlag;

    /* (1) First-tick grace flag — skip impact checks on current cell. */
    if (in->firstMoveGraceFlag) {
        outNewState->firstMoveGraceFlag = 0;
        outResult->newFirstMoveGraceFlag = 0;
        goto MOTION_STEP;
    }

    /* (2) Impact on *current* (source) cell before motion.
     *     Champion -> creature -> other-projectile priority.
     *     We treat the source-square champion/creature presence as
     *     equal to the destination digest when source==dest; callers
     *     using the one-square digest pack that via destHas* on the
     *     same cell. */
    if (digest->destHasChampion
        && (digest->destChampionCellMask & (1 << in->cell))
        && digest->sourceMapIndex == digest->destMapIndex
        && digest->sourceMapX == digest->destMapX
        && digest->sourceMapY == digest->destMapY) {
        dispatch = PROJECTILE_RESULT_HIT_CHAMPION;
        goto RESOLVE;
    }
    if (digest->destHasCreatureGroup
        && (digest->destCreatureCellMask & (1 << in->cell))
        && digest->sourceMapIndex == digest->destMapIndex
        && digest->sourceMapX == digest->destMapX
        && digest->sourceMapY == digest->destMapY) {
        /* ReDMCSB PROJEXPL.C:F0217 lines 532-533 returns FALSE for
         * non-material creatures unless the projectile is
         * HARM_NON_MATERIAL. In F0219 lines 693-697, FALSE means
         * "no impact" and the projectile keeps moving/reschedules; it
         * is not consumed. */
        if (projectile_is_fireball_black_flame_impact(in, digest)
            || !(digest->destCreatureIsNonMaterial
                 && in->projectileSubtype != PROJECTILE_SUBTYPE_HARM_NON_MATERIAL)) {
            dispatch = PROJECTILE_RESULT_HIT_CREATURE;
            goto RESOLVE;
        }
    }
    if (digest->sourceHasOtherProjectile) {
        dispatch = PROJECTILE_RESULT_HIT_OTHER_PROJECTILE;
        goto RESOLVE;
    }

    /* (3) Lifespan gate. Mirror of PROJEXPL.C:697-702. */
    if (in->kineticEnergy <= in->stepEnergy) {
        outResult->despawn    = 1;
        outResult->resultKind = PROJECTILE_RESULT_DESPAWN_ENERGY;
        return 1;
    }

    /* (4) Decrement energies. Mirror of PROJEXPL.C:706-712. */
    outNewState->kineticEnergy -= in->stepEnergy;
    {
        int attackDec = (in->attack < in->stepEnergy) ? in->attack : in->stepEnergy;
        outNewState->attack -= attackDec;
    }
    decrementedBeforeImpact = 1;

MOTION_STEP:
    /* (5) Cross-cell vs intra-square flip.
     *     Fontanel parity: projectile crosses iff the cell matches
     *     the direction or (direction+1)&3. Mirror of PROJEXPL.C:714-719. */
    crossesCell = ((in->direction == in->cell)
                   || (((in->direction + 1) & 3) == in->cell));
    /* ReDMCSB PROJEXPL.C:F0219 lines 717-725 can resolve a wall impact
     * immediately after this cross-square gate, before the destination
     * square is committed. Publish the gate result even for those early
     * impact returns so probes can distinguish side-wall travel from an
     * intra-square cell flip. */
    outResult->crossedCell = crossesCell ? 1 : 0;

    /* (6) New cell via parity rule. PROJEXPL.C:721-725. */
    if ((in->direction & 1) == (in->cell & 1)) {
        newCell = (in->cell - 1) & 3;
    } else {
        newCell = (in->cell + 1) & 3;
    }

    if (crossesCell) {
        /* (7) Inspect destination cell. */
        F0814_PROJECTILE_InspectDestination_Compat(digest, &blocker);

        if (projectile_open_door_spell_impacts_door(in, digest)) {
            dispatch = PROJECTILE_RESULT_HIT_DOOR;
            goto RESOLVE;
        }

        switch (blocker) {
        case PROJECTILE_BLOCKER_WALL:
        case PROJECTILE_BLOCKER_STAIRS:
        case PROJECTILE_BLOCKER_BOUNDARY:
            dispatch = PROJECTILE_RESULT_HIT_WALL;
            goto RESOLVE;
        case PROJECTILE_BLOCKER_FLUXCAGE:
            dispatch = PROJECTILE_RESULT_HIT_FLUXCAGE;
            goto RESOLVE;
        case PROJECTILE_BLOCKER_CLOSED_DOOR: {
            int passesDoor = 0;
            F0816_PROJECTILE_DoesPassThroughDoor_Compat(
                outNewState, digest, rng, &passesDoor);
            if (!passesDoor) {
                dispatch = PROJECTILE_RESULT_HIT_DOOR;
                goto RESOLVE;
            }
            /* ReDMCSB/CSBWin parity: F0217/ProcessMissileEncounter
             * returns false for pass-through doors, so F0219/ProcessTT_25
             * continues the same motion step and later reschedules the
             * projectile instead of treating it as a resolved hit. */
            break;
        }
        case PROJECTILE_BLOCKER_OTHER_PROJECTILE:
            dispatch = PROJECTILE_RESULT_HIT_OTHER_PROJECTILE;
            goto RESOLVE;
        case PROJECTILE_BLOCKER_OPEN:
        default:
            break;
        }

        /* Destination champion / creature (cross-cell). */
        if (digest->destHasChampion
            && (digest->destChampionCellMask & (1 << newCell))) {
            outNewState->cell = newCell;
            outNewState->mapIndex = digest->destMapIndex;
            outNewState->mapX     = digest->destMapX;
            outNewState->mapY     = digest->destMapY;
            dispatch = PROJECTILE_RESULT_HIT_CHAMPION;
            goto RESOLVE;
        }
        if (digest->destHasCreatureGroup
            && (digest->destCreatureCellMask & (1 << newCell))) {
            outNewState->cell = newCell;
            outNewState->mapIndex = digest->destMapIndex;
            outNewState->mapX     = digest->destMapX;
            outNewState->mapY     = digest->destMapY;
            if (projectile_is_fireball_black_flame_impact(in, digest)
                || !(digest->destCreatureIsNonMaterial
                     && in->projectileSubtype != PROJECTILE_SUBTYPE_HARM_NON_MATERIAL)) {
                dispatch = PROJECTILE_RESULT_HIT_CREATURE;
                goto RESOLVE;
            }
        }

        /* Commit the cross-cell step. Teleporter rotation: v1 does
         * NOT rotate; caller pre-rotates via
         * destTeleporterNewDirection when != -1. */
        outNewState->cell = newCell;
        outNewState->mapIndex = digest->destMapIndex;
        outNewState->mapX     = digest->destMapX;
        outNewState->mapY     = digest->destMapY;
        if (digest->destTeleporterNewDirection >= 0
            && digest->destTeleporterNewDirection <= 3) {
            outNewState->direction = digest->destTeleporterNewDirection;
        }
    } else {
        /* Intra-cell flip, did we land on a door inside the square? */
        if (projectile_open_door_spell_impacts_door(in, digest)) {
            dispatch = PROJECTILE_RESULT_HIT_DOOR;
            goto RESOLVE;
        }
        if (digest->destSquareType == PROJECTILE_ELEMENT_DOOR
            && digest->destDoorState != PROJECTILE_DOOR_STATE_NONE
            && !projectile_digest_door_is_destroyed(digest)
            && digest->destDoorState >= PROJECTILE_DOOR_STATE_CLOSED_HALF) {
            int passesDoor = 0;
            F0816_PROJECTILE_DoesPassThroughDoor_Compat(
                outNewState, digest, rng, &passesDoor);
            if (!passesDoor) {
                dispatch = PROJECTILE_RESULT_HIT_DOOR;
                goto RESOLVE;
            }
            /* Door pass-through mirrors PROJEXPL.C:F0217 returning
             * C0_FALSE; the caller keeps flying and reschedules. */
        }
        outNewState->cell = newCell;
    }

    /* (8) Reschedule next PROJECTILE_MOVE event. */
    outResult->resultKind = PROJECTILE_RESULT_FLEW;
    F0825_PROJECTILE_ScheduleNextMove_Compat(
        outNewState,
        (digest->destMapIndex == digest->sourceMapIndex) ? 1 : 0,
        currentTick,
        &outResult->outNextTick);

    outResult->newCell               = outNewState->cell;
    outResult->newMapIndex           = outNewState->mapIndex;
    outResult->newMapX               = outNewState->mapX;
    outResult->newMapY               = outNewState->mapY;
    outResult->newDirection          = outNewState->direction;
    outResult->newKineticEnergy      = outNewState->kineticEnergy;
    outResult->newAttack             = outNewState->attack;
    outResult->newFirstMoveGraceFlag = outNewState->firstMoveGraceFlag;
    return 1;

RESOLVE:
    /* Any hit (except door pass-through inside F0820) consumes the
     * projectile. F0820 handles DOOR_HIT + passes=1 by setting
     * despawn=0 + resultKind=FLEW. */
    {
        struct ProjectileInstance_Compat resolveState;
        const struct ProjectileInstance_Compat* resolveIn = in;
        struct CellContentDigest_Compat resolveDigest;
        const struct CellContentDigest_Compat* resolveDigestPtr = digest;

        /* ReDMCSB PROJEXPL.C:F0219 lines 729-740 applies the cell parity
         * step before F0267 commits an unblocked cross-square projectile
         * move. F0217 lines 509-558 then receives the impact cell for
         * champion damage; preserve wall/door blocker behavior by only
         * forwarding the parity-adjusted cell for champion/creature hits. */
        if ((dispatch == PROJECTILE_RESULT_HIT_DOOR && decrementedBeforeImpact) ||
            ((dispatch == PROJECTILE_RESULT_HIT_CHAMPION
              || dispatch == PROJECTILE_RESULT_HIT_CREATURE)
             && (outNewState->cell != in->cell
                 || outNewState->mapIndex != in->mapIndex
                 || outNewState->mapX != in->mapX
                 || outNewState->mapY != in->mapY))) {
            resolveState = (dispatch == PROJECTILE_RESULT_HIT_DOOR)
                           ? *outNewState : *in;
            if (dispatch == PROJECTILE_RESULT_HIT_CHAMPION
                || dispatch == PROJECTILE_RESULT_HIT_CREATURE) {
                resolveState.cell     = outNewState->cell;
                resolveState.mapIndex = outNewState->mapIndex;
                resolveState.mapX     = outNewState->mapX;
                resolveState.mapY     = outNewState->mapY;
            }
            resolveIn = &resolveState;
        }
        if (dispatch == PROJECTILE_RESULT_HIT_OTHER_PROJECTILE &&
            digest->sourceHasOtherProjectile) {
            /* ReDMCSB PROJEXPL.C:F0219 lines 692-697 checks impacts on the
             * current projectile cell before the energy decrement/move. When
             * F0218/F0217 resolves a same-cell projectile hit at that stage,
             * the common explosion/sound tail uses the projectile's current
             * square, not the lookahead destination square. */
            resolveDigest = *digest;
            resolveDigest.destMapIndex = digest->sourceMapIndex;
            resolveDigest.destMapX = digest->sourceMapX;
            resolveDigest.destMapY = digest->sourceMapY;
            resolveDigest.destSquareType = digest->sourceSquareType;
            resolveDigestPtr = &resolveDigest;
        }

        F0820_PROJECTILE_ResolveCollision_Compat(
            resolveIn, resolveDigestPtr, dispatch, currentTick, rng, outResult);
    }
    /* ReDMCSB PROJEXPL.C:F0219 lines 717-755 resolves wall/door
     * impacts before the destination-square move is committed. Keep
     * the public tick result in sync with outNewState so regression
     * probes can assert the impacted thrown item stayed on the source
     * square instead of being materialized in the blocked side cell. */
    outResult->newCell               = outNewState->cell;
    outResult->newMapIndex           = outNewState->mapIndex;
    outResult->newMapX               = outNewState->mapX;
    outResult->newMapY               = outNewState->mapY;
    outResult->newDirection          = outNewState->direction;
    outResult->newKineticEnergy      = outNewState->kineticEnergy;
    outResult->newAttack             = outNewState->attack;
    outResult->newFirstMoveGraceFlag = outNewState->firstMoveGraceFlag;
    return 1;
}

/* ==========================================================
 *  F0812 — spell effect → projectile bridge.
 * ========================================================== */

int F0812_PROJECTILE_CreateFromSpellEffect_Compat(
    const struct SpellEffect_Compat* effect,
    int casterChampionIndex,
    int partyMapIndex,
    int partyMapX,
    int partyMapY,
    int partyDirection,
    uint32_t currentTick,
    struct ProjectileList_Compat* list,
    int* outSlotIndex,
    struct TimelineEvent_Compat* outFirstMoveEvent)
{
    struct ProjectileCreateInput_Compat input;
    int subtype;
    int isPoison;

    if (effect == NULL || list == NULL || outSlotIndex == NULL
        || outFirstMoveEvent == NULL) {
        return 0;
    }
    if (effect->spellKind != C2_SPELL_KIND_PROJECTILE_COMPAT) return 0;
    if (casterChampionIndex < 0 || casterChampionIndex > 3) return 0;
    if (partyDirection < 0 || partyDirection > 3) return 0;

    subtype = Phase17_SpellTypeToSubtype(effect->spellType);
    if (subtype == 0) return 0;

    isPoison = (subtype == PROJECTILE_SUBTYPE_POISON_BOLT
                || subtype == PROJECTILE_SUBTYPE_POISON_CLOUD);

    memset(&input, 0, sizeof(input));
    input.category           = PROJECTILE_CATEGORY_MAGICAL;
    input.subtype            = subtype;
    input.ownerKind          = PROJECTILE_OWNER_CHAMPION;
    input.ownerIndex         = casterChampionIndex;
    input.mapIndex           = partyMapIndex;
    input.mapX               = partyMapX;
    input.mapY               = partyMapY;
    input.cell               = casterChampionIndex & 3;
    input.direction          = partyDirection;
    input.kineticEnergy      = effect->kineticEnergy;
    input.attack             = effect->impactAttack;
    input.launcherStrength   = effect->impactAttack;
    input.stepEnergy         = 1;
    input.currentTick        = (int)currentTick;
    input.poisonAttack       = isPoison ? effect->impactAttack : 0;
    input.attackTypeCode     = Phase17_SpellTypeToAttackType(effect->spellType);
    input.potionPower        = 0;
    input.firstMoveGraceFlag = 1;

    return F0810_PROJECTILE_Create_Compat(
        &input, list, outSlotIndex, outFirstMoveEvent);
}

/* ==========================================================
 *  Group D — Explosion lifecycle (F0821 – F0824).
 * ========================================================== */

int F0821_EXPLOSION_Create_Compat(
    const struct ExplosionCreateInput_Compat* in,
    struct ExplosionList_Compat* list,
    int* outSlotIndex,
    struct TimelineEvent_Compat* outFirstAdvanceEvent)
{
    int slot;
    int i;
    int delay;

    if (in == NULL || list == NULL || outSlotIndex == NULL
        || outFirstAdvanceEvent == NULL) {
        return 0;
    }
    if (list->count >= EXPLOSION_LIST_CAPACITY) return 0;

    slot = -1;
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; i++) {
        if (list->entries[i].reserved0 == 0) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return 0;

    memset(&list->entries[slot], 0, sizeof(list->entries[slot]));
    list->entries[slot].slotIndex             = slot;
    list->entries[slot].explosionType         = in->explosionType;
    list->entries[slot].mapIndex              = in->mapIndex;
    list->entries[slot].mapX                  = in->mapX;
    list->entries[slot].mapY                  = in->mapY;
    list->entries[slot].cell                  = in->cell;
    list->entries[slot].centered              = (in->centered != 0) ? 1 : 0;
    list->entries[slot].attack                = clamp_i(in->attack, 0, 255);
    list->entries[slot].currentFrame          = 0;
    list->entries[slot].maxFrames             = (in->explosionType >= 0
                                                 && in->explosionType < 128)
        ? Phase17_ExplosionMaxFrames[in->explosionType] : 1;
    list->entries[slot].poisonAttack          = in->poisonAttack;
    list->entries[slot].ownerKind             = in->ownerKind;
    list->entries[slot].ownerIndex            = in->ownerIndex;
    list->entries[slot].creatorProjectileSlot = in->creatorProjectileSlot;
    list->entries[slot].reserved0             = 1; /* occupied */

    /* Rebirth step 1: 5-tick reschedule; everything else: +1. */
    delay = (in->explosionType == C100_EXPLOSION_REBIRTH_STEP1) ? 5 : 1;
    list->entries[slot].scheduledAtTick = in->currentTick + delay;
    list->count++;
    *outSlotIndex = slot;

    memset(outFirstAdvanceEvent, 0, sizeof(*outFirstAdvanceEvent));
    outFirstAdvanceEvent->kind       = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    outFirstAdvanceEvent->fireAtTick = (uint32_t)(in->currentTick + delay);
    outFirstAdvanceEvent->mapIndex   = in->mapIndex;
    outFirstAdvanceEvent->mapX       = in->mapX;
    outFirstAdvanceEvent->mapY       = in->mapY;
    outFirstAdvanceEvent->cell       = in->cell;
    outFirstAdvanceEvent->aux0       = slot;
    outFirstAdvanceEvent->aux1       = in->explosionType;
    outFirstAdvanceEvent->aux2       = in->attack;
    outFirstAdvanceEvent->aux3       = in->ownerKind;
    outFirstAdvanceEvent->aux4       = in->ownerIndex;
    return 1;
}

int F0823_EXPLOSION_ComputeAoE_Compat(
    const struct ExplosionInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    struct RngState_Compat* rng,
    int* outAttackApplied)
{
    int raw;
    (void)digest;
    if (in == NULL || outAttackApplied == NULL) return 0;

    switch (in->explosionType) {
    case C007_EXPLOSION_POISON_CLOUD: {
        int base = (in->attack >> 5);
        if (base > 4) base = 4;
        if (rng != NULL) base += F0732_COMBAT_RngRandom_Compat(rng, 2);
        if (base < 1) base = 1;
        *outAttackApplied = base;
        break;
    }
    case C002_EXPLOSION_LIGHTNING_BOLT:
        raw = (in->attack >> 1) + 1;
        if (rng != NULL) raw += F0732_COMBAT_RngRandom_Compat(rng, raw) + 1;
        else             raw += 1;
        *outAttackApplied = raw >> 1;  /* lightning halved again */
        break;
    case C000_EXPLOSION_FIREBALL:
    default:
        raw = (in->attack >> 1) + 1;
        if (rng != NULL) raw += F0732_COMBAT_RngRandom_Compat(rng, raw) + 1;
        else             raw += 1;
        *outAttackApplied = raw;
        break;
    }
    return 1;
}

int F0830_EXPLOSION_HarmNonMaterialCreatureAttack_Compat(
    int baseAttack,
    int creatureType,
    int explosionOnPartyMap,
    int activeGroupAspect,
    struct RngState_Compat* rng,
    int* outAttackApplied)
{
    int attack;
    int additionalAttack;

    if (outAttackApplied == NULL) return 0;
    *outAttackApplied = 0;
    attack = baseAttack;
    if (attack < 0) attack = 0;

    /* ReDMCSB PROJEXPL.C:F0220 lines 832-846: Harm Non Material
     * damages every non-material creature normally, except party-map
     * Materializer/Zytaz (C19), where each slot is damaged only while
     * that slot's active-group Aspect has MASK0x0080_IS_ATTACKING set. */
    if (creatureType == 19 && explosionOnPartyMap) {
        if ((activeGroupAspect & 0x80) == 0) {
            return 1;
        }
        additionalAttack = attack >> 3;
        attack -= additionalAttack;
        additionalAttack = (additionalAttack << 1) + 1;
        attack += F0732_COMBAT_RngRandom_Compat(rng, additionalAttack);
        attack += F0732_COMBAT_RngRandom_Compat(rng, 4);
    }

    *outAttackApplied = attack;
    return 1;
}

static void build_explosion_champion_action(
    const struct ExplosionInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int attackApplied,
    int attackTypeCode,
    struct CombatAction_Compat* out)
{
    int cellForChampion = (in->cell == EXPLOSION_CELL_CENTERED)
                          ? 0 : (in->cell & 3);
    memset(out, 0, sizeof(*out));
    out->kind                        = COMBAT_ACTION_APPLY_DAMAGE_CHAMPION;
    /* ReDMCSB PROJEXPL.C:F0213 line 173 routes fireball/lightning
     * party-square explosions through F0324 with every champion wound
     * slot allowed.  Poison clouds override this to WOUND_NONE in the
     * caller after this generic party action is built. */
    out->allowedWounds               = COMBAT_WOUND_READY_HAND |
                                       COMBAT_WOUND_ACTION_HAND |
                                       COMBAT_WOUND_HEAD |
                                       COMBAT_WOUND_TORSO |
                                       COMBAT_WOUND_LEGS |
                                       COMBAT_WOUND_FEET;
    out->attackTypeCode              = attackTypeCode;
    out->rawAttackValue              = attackApplied;
    out->targetMapIndex              = digest->destMapIndex;
    out->targetMapX                  = digest->destMapX;
    out->targetMapY                  = digest->destMapY;
    out->targetCell                  = cellForChampion;
    out->attackerSlotOrCreatureIndex = in->ownerIndex;
    out->defenderSlotOrCreatureIndex = cellForChampion;
    out->scheduleDelayTicks          = 0;
    out->flags                       = 0;
}

static void build_explosion_group_action(
    const struct ExplosionInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int attackApplied,
    int attackTypeCode,
    struct CombatAction_Compat* out)
{
    memset(out, 0, sizeof(*out));
    out->kind                        = COMBAT_ACTION_APPLY_DAMAGE_GROUP;
    out->allowedWounds               = 0;
    out->attackTypeCode              = attackTypeCode;
    out->rawAttackValue              = attackApplied;
    out->targetMapIndex              = digest->destMapIndex;
    out->targetMapX                  = digest->destMapX;
    out->targetMapY                  = digest->destMapY;
    out->targetCell                  = (in->cell == EXPLOSION_CELL_CENTERED)
                                        ? 0 : (in->cell & 3);
    out->attackerSlotOrCreatureIndex = in->ownerIndex;
    out->defenderSlotOrCreatureIndex = digest->destCreatureType;
    out->scheduleDelayTicks          = 0;
    out->flags                       = 0;
}

int F0822_EXPLOSION_Advance_Compat(
    const struct ExplosionInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct ExplosionInstance_Compat* outNewState,
    struct ExplosionTickResult_Compat* outResult)
{
    int attackApplied = 0;
    int attackTypeCode;

    if (in == NULL || digest == NULL || outNewState == NULL
        || outResult == NULL) {
        return 0;
    }

    memset(outResult, 0, sizeof(*outResult));
    *outNewState = *in;

    F0823_EXPLOSION_ComputeAoE_Compat(in, digest, rng, &attackApplied);

    switch (in->explosionType) {
    case C000_EXPLOSION_FIREBALL:
    case C002_EXPLOSION_LIGHTNING_BOLT: {
        attackTypeCode = (in->explosionType == C002_EXPLOSION_LIGHTNING_BOLT)
                         ? COMBAT_ATTACK_LIGHTNING : COMBAT_ATTACK_FIRE;
        if (digest->destHasChampion) {
            /* ReDMCSB PROJEXPL.C:F0213 lines 129-146 checks the
             * party square first; creature damage is in the else branch. */
            build_explosion_champion_action(in, digest, attackApplied,
                                            attackTypeCode,
                                            &outResult->outActionParty);
            outResult->emittedCombatActionPartyCount = 1;
        } else if (digest->destHasCreatureGroup) {
            int groupAttackApplied = attackApplied;
            /* ReDMCSB PROJEXPL.C:F0213 lines 137-142 quarters
             * fireball/lightning damage against non-material creatures
             * before resistance adjustment. The digest has the
             * non-material bit, so preserve that high-impact parity even
             * though fire-resistance remains caller-side Phase 13 data. */
            if (digest->destCreatureIsNonMaterial) {
                groupAttackApplied >>= 2;
            }
            if (groupAttackApplied > 0) {
                build_explosion_group_action(in, digest, groupAttackApplied,
                                             attackTypeCode,
                                             &outResult->outActionGroup);
                outResult->emittedCombatActionGroupCount = 1;
            }
        }
        if (digest->destSquareType == PROJECTILE_ELEMENT_DOOR
            && digest->destDoorState != PROJECTILE_DOOR_STATE_NONE
            && !projectile_digest_door_is_destroyed(digest)) {
            memset(&outResult->outNextTick, 0, sizeof(outResult->outNextTick));
            outResult->outNextTick.kind       = TIMELINE_EVENT_DOOR_DESTRUCTION;
            outResult->outNextTick.fireAtTick = currentTick + 1u;
            outResult->outNextTick.mapIndex   = digest->destMapIndex;
            outResult->outNextTick.mapX       = digest->destMapX;
            outResult->outNextTick.mapY       = digest->destMapY;
            outResult->outNextTick.cell       = (in->cell == EXPLOSION_CELL_CENTERED)
                                                ? 0 : (in->cell & 3);
            outResult->outNextTick.aux0       = attackApplied;
            outResult->outNextTick.aux1       = in->explosionType;
            outResult->outNextTick.aux2       = in->ownerIndex;
            outResult->emittedDoorDestructionEvent = 1;
        }
        outResult->despawn    = 1;
        outResult->resultKind = EXPLOSION_RESULT_ONE_SHOT;
        break;
    }
    case C003_EXPLOSION_HARM_NON_MATERIAL:
        if (digest->destHasCreatureGroup && digest->destCreatureIsNonMaterial) {
            build_explosion_group_action(in, digest, attackApplied,
                                         COMBAT_ATTACK_MAGIC,
                                         &outResult->outActionGroup);
            outResult->emittedCombatActionGroupCount = 1;
        }
        outResult->despawn    = 1;
        outResult->resultKind = EXPLOSION_RESULT_ONE_SHOT;
        break;
    case C100_EXPLOSION_REBIRTH_STEP1:
        outNewState->explosionType = C101_EXPLOSION_REBIRTH_STEP2;
        F0826_EXPLOSION_ScheduleNextAdvance_Compat(
            outNewState, currentTick, 5, &outResult->outNextTick);
        outResult->despawn    = 0;
        outResult->resultKind = EXPLOSION_RESULT_ADVANCED_FRAME;
        break;
    case C040_EXPLOSION_SMOKE:
        if (in->attack > 55) {
            outNewState->attack = in->attack - 40;
            F0826_EXPLOSION_ScheduleNextAdvance_Compat(
                outNewState, currentTick, 1, &outResult->outNextTick);
            outResult->newAttack  = outNewState->attack;
            outResult->despawn    = 0;
            outResult->resultKind = EXPLOSION_RESULT_ADVANCED_FRAME;
        } else {
            outResult->despawn    = 1;
            outResult->resultKind = EXPLOSION_RESULT_ONE_SHOT;
        }
        break;
    case C007_EXPLOSION_POISON_CLOUD:
        if (digest->destHasChampion) {
            build_explosion_champion_action(in, digest, attackApplied,
                                            COMBAT_ATTACK_NORMAL,
                                            &outResult->outActionParty);
            outResult->outActionParty.allowedWounds = 0;
            outResult->emittedCombatActionPartyCount = 1;
        } else if (digest->destHasCreatureGroup) {
            /* ReDMCSB PROJEXPL.C:F0220 line 863: when a poison cloud
             * lands on a creature group, the original reassigns
             * L0530_i_Attack = F0192_GROUP_GetResistanceAdjustedPoison-
             * Attack(creatureType, L0530_i_Attack) and then calls
             * F0191_GROUP_GetDamageAllCreaturesOutcome with the
             * F0192-adjusted value.  F0191 does the per-creature
             * damage application (GROUP.C:F0190 lines 932-1010) and
             * does NOT re-apply resistance.  Pass the F0192-adjusted
             * attack to build_explosion_group_action so rawAttackValue
             * is the resistance-adjusted value (e.g. attack=96 with
             * Mummy resistance=5 gives rawAttackValue=4, not 3).
             * f60e82f11 incorrectly removed this F0192 pre-scale; the
             * ReDMCSB source line clearly reassigns L0530_i_Attack
             * from the F0192 result.  Restored for v2.7.22 and
             * locked in source comments + test_dm1_v1_monster_poison_
             * cloud_overlap_tick_pc34_compat.  The pre-F0192-removal
             * test_dm1_v1_projectile_explosion_render_pc34_compat
             * expectations are stale and are updated to match. */
            int resistanceAdjusted = 0;
            F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
                digest->destCreatureType, attackApplied, rng,
                &resistanceAdjusted);
            if (resistanceAdjusted > 0) {
                build_explosion_group_action(in, digest, resistanceAdjusted,
                                             COMBAT_ATTACK_NORMAL,
                                             &outResult->outActionGroup);
                outResult->emittedCombatActionGroupCount = 1;
            }
        }
        if (in->attack >= 6) {
            outNewState->attack = in->attack - 3;
            F0826_EXPLOSION_ScheduleNextAdvance_Compat(
                outNewState, currentTick, 1, &outResult->outNextTick);
            outResult->newAttack  = outNewState->attack;
            outResult->despawn    = 0;
            outResult->resultKind = EXPLOSION_RESULT_ADVANCED_FRAME;
        } else {
            outResult->despawn    = 1;
            outResult->resultKind = EXPLOSION_RESULT_ONE_SHOT;
        }
        break;
    case C050_EXPLOSION_FLUXCAGE:
    case C101_EXPLOSION_REBIRTH_STEP2:
        /* Persistent; REMOVE_FLUXCAGE event is Phase 12's slot —
         * Phase 17 does not emit it in v1 (plan §1 #8). */
        outResult->despawn    = 0;
        outResult->resultKind = EXPLOSION_RESULT_PERSISTENT;
        break;
    default:
        outResult->despawn    = 1;
        outResult->resultKind = EXPLOSION_RESULT_ONE_SHOT;
        break;
    }

    outNewState->currentFrame = in->currentFrame + 1;
    outResult->newCurrentFrame = outNewState->currentFrame;
    if (outResult->despawn) outResult->newAttack = 0;
    return 1;
}

int F0824_EXPLOSION_Despawn_Compat(
    struct ExplosionList_Compat* list,
    int slotIndex)
{
    if (list == NULL) return 0;
    if (slotIndex < 0 || slotIndex >= EXPLOSION_LIST_CAPACITY) return 0;
    if (list->entries[slotIndex].reserved0 == 0) return 0;
    memset(&list->entries[slotIndex], 0, sizeof(list->entries[slotIndex]));
    list->entries[slotIndex].slotIndex = -1;
    if (list->count > 0) list->count--;
    return 1;
}

/* ==========================================================
 *  Group F — Serialisation (F0827 – F0829).
 * ========================================================== */

int F0827_PROJECTILE_InstanceSerialize_Compat(
    const struct ProjectileInstance_Compat* in,
    unsigned char* outBuf,
    int outBufSize)
{
    int o = 0;
    if (in == NULL || outBuf == NULL) return 0;
    if (outBufSize < PROJECTILE_INSTANCE_SERIALIZED_SIZE) return 0;

    write_le_int32(outBuf + o, in->slotIndex);              o += 4;
    write_le_int32(outBuf + o, in->projectileCategory);     o += 4;
    write_le_int32(outBuf + o, in->projectileSubtype);      o += 4;
    write_le_int32(outBuf + o, in->ownerKind);              o += 4;
    write_le_int32(outBuf + o, in->ownerIndex);             o += 4;
    write_le_int32(outBuf + o, in->mapIndex);               o += 4;
    write_le_int32(outBuf + o, in->mapX);                   o += 4;
    write_le_int32(outBuf + o, in->mapY);                   o += 4;
    write_le_int32(outBuf + o, in->cell);                   o += 4;
    write_le_int32(outBuf + o, in->direction);              o += 4;
    write_le_int32(outBuf + o, in->kineticEnergy);          o += 4;
    write_le_int32(outBuf + o, in->attack);                 o += 4;
    write_le_int32(outBuf + o, in->stepEnergy);             o += 4;
    write_le_int32(outBuf + o, in->firstMoveGraceFlag);     o += 4;
    write_le_int32(outBuf + o, in->launchedAtTick);         o += 4;
    write_le_int32(outBuf + o, in->scheduledAtTick);        o += 4;
    write_le_int32(outBuf + o, in->associatedPotionPower);  o += 4;
    write_le_int32(outBuf + o, in->poisonAttack);           o += 4;
    write_le_int32(outBuf + o, in->attackTypeCode);         o += 4;
    write_le_int32(outBuf + o, in->flags);                  o += 4;
    write_le_int32(outBuf + o, in->launcherStrength);       o += 4;
    write_le_int32(outBuf + o, in->reserved0);              o += 4;
    write_le_int32(outBuf + o, in->reserved1);              o += 4;
    write_le_int32(outBuf + o, in->reserved2);              o += 4;
    write_le_int32(outBuf + o, in->reserved3);              o += 4;
    return o;
}

int F0827_PROJECTILE_InstanceDeserialize_Compat(
    struct ProjectileInstance_Compat* out,
    const unsigned char* buf,
    int bufSize)
{
    int o = 0;
    if (out == NULL || buf == NULL) return 0;
    if (bufSize < PROJECTILE_INSTANCE_SERIALIZED_SIZE) return 0;

    out->slotIndex             = read_le_int32(buf + o); o += 4;
    out->projectileCategory    = read_le_int32(buf + o); o += 4;
    out->projectileSubtype     = read_le_int32(buf + o); o += 4;
    out->ownerKind             = read_le_int32(buf + o); o += 4;
    out->ownerIndex            = read_le_int32(buf + o); o += 4;
    out->mapIndex              = read_le_int32(buf + o); o += 4;
    out->mapX                  = read_le_int32(buf + o); o += 4;
    out->mapY                  = read_le_int32(buf + o); o += 4;
    out->cell                  = read_le_int32(buf + o); o += 4;
    out->direction             = read_le_int32(buf + o); o += 4;
    out->kineticEnergy         = read_le_int32(buf + o); o += 4;
    out->attack                = read_le_int32(buf + o); o += 4;
    out->stepEnergy            = read_le_int32(buf + o); o += 4;
    out->firstMoveGraceFlag    = read_le_int32(buf + o); o += 4;
    out->launchedAtTick        = read_le_int32(buf + o); o += 4;
    out->scheduledAtTick       = read_le_int32(buf + o); o += 4;
    out->associatedPotionPower = read_le_int32(buf + o); o += 4;
    out->poisonAttack          = read_le_int32(buf + o); o += 4;
    out->attackTypeCode        = read_le_int32(buf + o); o += 4;
    out->flags                 = read_le_int32(buf + o); o += 4;
    out->launcherStrength      = read_le_int32(buf + o); o += 4;
    out->reserved0             = read_le_int32(buf + o); o += 4;
    out->reserved1             = read_le_int32(buf + o); o += 4;
    out->reserved2             = read_le_int32(buf + o); o += 4;
    out->reserved3             = read_le_int32(buf + o); o += 4;
    return o;
}

int F0828_EXPLOSION_InstanceSerialize_Compat(
    const struct ExplosionInstance_Compat* in,
    unsigned char* outBuf,
    int outBufSize)
{
    int o = 0;
    if (in == NULL || outBuf == NULL) return 0;
    if (outBufSize < EXPLOSION_INSTANCE_SERIALIZED_SIZE) return 0;

    write_le_int32(outBuf + o, in->slotIndex);             o += 4;
    write_le_int32(outBuf + o, in->explosionType);         o += 4;
    write_le_int32(outBuf + o, in->mapIndex);              o += 4;
    write_le_int32(outBuf + o, in->mapX);                  o += 4;
    write_le_int32(outBuf + o, in->mapY);                  o += 4;
    write_le_int32(outBuf + o, in->cell);                  o += 4;
    write_le_int32(outBuf + o, in->centered);              o += 4;
    write_le_int32(outBuf + o, in->attack);                o += 4;
    write_le_int32(outBuf + o, in->currentFrame);          o += 4;
    write_le_int32(outBuf + o, in->maxFrames);             o += 4;
    write_le_int32(outBuf + o, in->poisonAttack);          o += 4;
    write_le_int32(outBuf + o, in->scheduledAtTick);       o += 4;
    write_le_int32(outBuf + o, in->ownerKind);             o += 4;
    write_le_int32(outBuf + o, in->ownerIndex);            o += 4;
    write_le_int32(outBuf + o, in->creatorProjectileSlot); o += 4;
    write_le_int32(outBuf + o, in->reserved0);             o += 4;
    return o;
}

int F0828_EXPLOSION_InstanceDeserialize_Compat(
    struct ExplosionInstance_Compat* out,
    const unsigned char* buf,
    int bufSize)
{
    int o = 0;
    if (out == NULL || buf == NULL) return 0;
    if (bufSize < EXPLOSION_INSTANCE_SERIALIZED_SIZE) return 0;

    out->slotIndex             = read_le_int32(buf + o); o += 4;
    out->explosionType         = read_le_int32(buf + o); o += 4;
    out->mapIndex              = read_le_int32(buf + o); o += 4;
    out->mapX                  = read_le_int32(buf + o); o += 4;
    out->mapY                  = read_le_int32(buf + o); o += 4;
    out->cell                  = read_le_int32(buf + o); o += 4;
    out->centered              = read_le_int32(buf + o); o += 4;
    out->attack                = read_le_int32(buf + o); o += 4;
    out->currentFrame          = read_le_int32(buf + o); o += 4;
    out->maxFrames             = read_le_int32(buf + o); o += 4;
    out->poisonAttack          = read_le_int32(buf + o); o += 4;
    out->scheduledAtTick       = read_le_int32(buf + o); o += 4;
    out->ownerKind             = read_le_int32(buf + o); o += 4;
    out->ownerIndex            = read_le_int32(buf + o); o += 4;
    out->creatorProjectileSlot = read_le_int32(buf + o); o += 4;
    out->reserved0             = read_le_int32(buf + o); o += 4;
    return o;
}

int F0829_PROJECTILE_ListSerialize_Compat(
    const struct ProjectileList_Compat* in,
    unsigned char* outBuf,
    int outBufSize)
{
    int o;
    int i;
    if (in == NULL || outBuf == NULL) return 0;
    if (outBufSize < PROJECTILE_LIST_SERIALIZED_SIZE) return 0;
    write_le_int32(outBuf + 0, in->count);
    write_le_int32(outBuf + 4, in->reserved);
    o = 8;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; i++) {
        int n = F0827_PROJECTILE_InstanceSerialize_Compat(
            &in->entries[i], outBuf + o,
            outBufSize - o);
        if (n != PROJECTILE_INSTANCE_SERIALIZED_SIZE) return 0;
        o += n;
    }
    return o;
}

int F0829_PROJECTILE_ListDeserialize_Compat(
    struct ProjectileList_Compat* out,
    const unsigned char* buf,
    int bufSize)
{
    int o;
    int i;
    if (out == NULL || buf == NULL) return 0;
    if (bufSize < PROJECTILE_LIST_SERIALIZED_SIZE) return 0;
    out->count    = read_le_int32(buf + 0);
    out->reserved = read_le_int32(buf + 4);
    o = 8;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; i++) {
        int n = F0827_PROJECTILE_InstanceDeserialize_Compat(
            &out->entries[i], buf + o, bufSize - o);
        if (n != PROJECTILE_INSTANCE_SERIALIZED_SIZE) return 0;
        o += n;
    }
    return o;
}

int F0829_EXPLOSION_ListSerialize_Compat(
    const struct ExplosionList_Compat* in,
    unsigned char* outBuf,
    int outBufSize)
{
    int o;
    int i;
    if (in == NULL || outBuf == NULL) return 0;
    if (outBufSize < EXPLOSION_LIST_SERIALIZED_SIZE) return 0;
    write_le_int32(outBuf + 0, in->count);
    write_le_int32(outBuf + 4, in->reserved);
    o = 8;
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; i++) {
        int n = F0828_EXPLOSION_InstanceSerialize_Compat(
            &in->entries[i], outBuf + o, outBufSize - o);
        if (n != EXPLOSION_INSTANCE_SERIALIZED_SIZE) return 0;
        o += n;
    }
    return o;
}

int F0829_EXPLOSION_ListDeserialize_Compat(
    struct ExplosionList_Compat* out,
    const unsigned char* buf,
    int bufSize)
{
    int o;
    int i;
    if (out == NULL || buf == NULL) return 0;
    if (bufSize < EXPLOSION_LIST_SERIALIZED_SIZE) return 0;
    out->count    = read_le_int32(buf + 0);
    out->reserved = read_le_int32(buf + 4);
    o = 8;
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; i++) {
        int n = F0828_EXPLOSION_InstanceDeserialize_Compat(
            &out->entries[i], buf + o, bufSize - o);
        if (n != EXPLOSION_INSTANCE_SERIALIZED_SIZE) return 0;
        o += n;
    }
    return o;
}
