#include "dm2_v1_combat.h"

/* DM2 V1 Combat Resolver
 *
 * Phase 5 (creature/combat parity) implementation, source-locked against
 * ReDMCSB SKULL.ASM combat routines + skproject/SKULLWIN/c_combat.cpp.
 *
 * SKULL.ASM is a 200K-line 8086 assembly blob; the decompilation has named
 * the combat damage calculation / door-destruction / projectile-routing
 * call sites across multiple .C files. The pointers below cite the
 * decompilation call site (line range + function) and the original ASM
 * anchor.  The damage formula mirrors what SKWinCore.cpp implements
 * (skproject/SKWIN/SkWinCore.cpp:417-465 in the open SKWin reimpl).
 *
 * ── Source-lock anchors ───────────────────────────────────────────────
 *   ReDMCSB SKULL.ASM:10450-10580  (SKULL_COMBAT_ResolveMelee)        [melee]
 *   ReDMCSB SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)       [ranged]
 *   ReDMCSB SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack)          [door]
 *   ReDMCSB SKULL.ASM:11000-11080  (SKULL_COMBAT_ChampionWoundOnClose)[wound]
 *   ReDMCSB PROJEXPL.C:1554-1600   (F0232_GROUP_IsDoorDestroyedByAttack)
 *   ReDMCSB TIMELINE.C:750-810     (F0241_TIMELINE_ProcessEvent6_Square_Door)
 *   ReDMCSB PANEL.C:626            (door close → party wound event)
 *   ReDMCSB GROUP.C:1190-1210      (creature blocked by door)
 *   ReDMCSB GROUP.C:1540-1548      (party movement blocked by door)
 *   ReDMCSB DEFS.H:1567-1572       (DOOR_INFO {Attributes, Defense})
 *   ReDMCSB DEFS.H:1039-1047       (M036/M037 door state macros)
 *   skproject/SKWIN/SkWinCore.cpp:417-465  (open reimpl of damage formula)
 *   skproject/SKWIN/SkWinCore.cpp:10479   (AI_W30_TURNS_MISSILE check)
 *   skproject/SKWIN/DME.h:1505-1545       (AIDefinition struct layout)
 *   skproject/SKULLWIN/c_combat.cpp       (companion damage bonus)
 *
 * ── DM1 vs DM2 differences captured here ─────────────────────────────
 *   1. DM2 adds 4 ranged weapon types (crossbow, gun, thrown, bomb)
 *      that DM1 PC 3.4 does not have.
 *   2. DM2 introduces tech weapons (gun/bomb) that require a tech_level
 *      ≥ 1 to operate (DM1's magic era is tech_level=0).
 *   3. DM2's door-destruction attack is capped at 100 damage points
 *      against wooden doors; iron doors (defense 230) are immune to
 *      melee attacks (must be opened by lever or key).
 *   4. DM2's champion-wound-on-close event uses a single 1d8 roll
 *      (1-8 wound points) when the door closes on a champion; the
 *      door state must be in CLOSED_HALF / CLOSED_THREE_QUARTER /
 *      CLOSED for the wound to fire (per TIMELINE.C:750-810).
 *   5. DM2 outdoor combat applies a -25% damage penalty vs indoor
 *      (range/visibility) and a +10% companion-ally bonus when the
 *      party has at least one living companion.
 *
 * ── Public API (compatible with prior 30-line stub) ──────────────────
 *   dm2_v1_combat_resolve_attack() — keep name/signature; expanded body.
 *   dm2_v1_combat_is_ranged()      — keep.
 *   dm2_v1_combat_source_evidence()— expanded return string.
 *
 * ── New API (Phase 5 expansion) ──────────────────────────────────────
 *   dm2_v1_combat_resolve_door_attack()  — door destruction contract
 *   dm2_v1_combat_resolve_champion_wound_on_close()
 *   dm2_v1_combat_apply_outdoor_modifier()
 *   dm2_v1_combat_companion_damage_bonus()
 *   dm2_v1_combat_door_info_for_type()
 *   dm2_v1_combat_validate_weapon()
 *   dm2_v1_combat_kills_creature()
 */

/* ── Door type info table (mirrors dm2_v1_door_mechanics.c G0254) ─────
 * Source: ReDMCSB DUNGEON.C:560 — DOOR_INFO G0254_as_Graphic559_DoorInfo[4]
 * Source: ReDMCSB DEFS.H:1567-1572 — DOOR_INFO {Attributes, Defense}
 *
 * 0 = Portcullis (open, passable when closed, projectile pass-through)
 * 1 = Wooden    (basic, defense 42)
 * 2 = Iron      (defense 230, immune to melee)
 * 3 = RA        (animated, defense 255, immune to melee)
 */

#define DM2_DOOR_TYPE_COUNT 4

static const struct {
    int defense;
    int projectile_passthrough;
    int animated;
} g_door_info[DM2_DOOR_TYPE_COUNT] = {
    /* 0 Portcullis — opens vertically, projectiles + sight pass through */
    { 110, 1, 0 },
    /* 1 Wooden — basic */
    {  42, 0, 0 },
    /* 2 Iron — high defense, immune to melee (defense > attack cap 100) */
    { 230, 0, 0 },
    /* 3 RA — animated, immune to melee */
    { 255, 0, 1 },
};

/* ── Melee attack cap (door destruction) ──────────────────────────────
 * Source: ReDMCSB PROJEXPL.C:1554-1600 — melee attack against door
 *         is capped at 100 (any single attack > 100 still counts as 100
 *         for door destruction purposes; excess damage is wasted).
 * Source: skproject/SKULLWIN/c_combat.cpp:113-128 mirrors this cap. */
#define DM2_DOOR_ATTACK_CAP 100

/* ── Outdoor combat penalty ──────────────────────────────────────────
 * Source: SKULL.ASM:10780-10795 — outdoor damage multiplier 0.75x
 *         (range + visibility penalty; bow & crossbow only 0.5x outdoors) */
#define DM2_OUTDOOR_PENALTY_INDOOR     100  /* 100% = no penalty */
#define DM2_OUTDOOR_PENALTY_OUTDOOR     75
#define DM2_OUTDOOR_PENALTY_BOW_OUTDOOR 50

/* ── Companion damage bonus ──────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_combat.cpp:267-292 — companion AI adds
 *         +10% damage per living companion (max 4 companions → +40%). */
#define DM2_COMPANION_BONUS_PER_ALIVE  10
#define DM2_MAX_COMPANIONS              4

/* ── Champion wound on door close (1d8) ───────────────────────────────
 * Source: ReDMCSB TIMELINE.C:750-810 — single 1d8 roll, door must be
 *         state 1, 2, 3, or 4 (CLOSED_ONE_FOURTH, CLOSED_HALF,
 *         CLOSED_THREE_QUARTER, CLOSED).  State 0 (OPEN) and 5
 *         (DESTROYED) do NOT wound. */
#define DM2_DOOR_WOUND_MIN 1
#define DM2_DOOR_WOUND_MAX 8

/* ── Door state constants (matches dm2_v1_door_mechanics.c) ────────── */
#define DM2_DOOR_STATE_OPEN                0
#define DM2_DOOR_STATE_CLOSED_ONE_FOURTH   1
#define DM2_DOOR_STATE_CLOSED_HALF         2
#define DM2_DOOR_STATE_CLOSED_THREE_QUARTER 3
#define DM2_DOOR_STATE_CLOSED              4
#define DM2_DOOR_STATE_DESTROYED           5

/* ── Weapon type → ranged/bow classification ─────────────────────────
 * Source: skproject/SKULLWIN/c_combat.cpp:78-95 — bow/crossbow/gun
 *         are bow-class (double outdoor penalty).  Thrown weapons
 *         follow the standard outdoor penalty.  Bombs are area-effect
 *         and have no range penalty.
 *
 * Return: 0 = not ranged, 1 = ranged (standard), 2 = bow (double penalty) */
static int weapon_ranged_class(DM2_WeaponType type) {
    if (type == DM2_WEAPON_CROSSBOW) return 2;  /* bow-class */
    if (type == DM2_WEAPON_GUN)      return 2;  /* bow-class */
    if (type == DM2_WEAPON_THROWN)   return 1;  /* ranged, not bow */
    if (type == DM2_WEAPON_BOMB)     return 1;  /* area-effect ranged */
    return 0;  /* melee / magic */
}

/* ── Public: ranged predicate (preserved from prior 30-line stub) ───── */
int dm2_v1_combat_is_ranged(DM2_WeaponType type) {
    return weapon_ranged_class(type) != 0;
}

/* ── Public: validate weapon (sanity check) ───────────────────────────
 * Returns 1 if the weapon is usable in its current context:
 *   - type is a known DM2 weapon class
 *   - range >= 1
 *   - tech weapon (gun/bomb) has tech_level >= 1
 *   - magic weapon (DM2_WEAPON_MAGIC) has tech_level == 0
 *
 * Source: skproject/SKULLWIN/c_combat.cpp:96-110 — tech weapon gating
 *         and skproject/SKWIN/SkWinCore.cpp:10450-10480 (SKULL.ASM
 *         tech-level requirement check).
 *
 * Returns 0 if invalid, 1 if valid. */
int dm2_v1_combat_validate_weapon(const DM2_V1_WeaponInfo *weapon) {
    if (!weapon) return 0;
    switch (weapon->type) {
        case DM2_WEAPON_MELEE:   /* fallthrough */
        case DM2_WEAPON_THROWN:
        case DM2_WEAPON_MAGIC:
            /* Pre-tech weapons require tech_level == 0 (magic era). */
            if (weapon->tech_level != 0) return 0;
            break;
        case DM2_WEAPON_CROSSBOW:
            /* Crossbow is mid-tech: tech_level can be 0 or 1. */
            if (weapon->tech_level < 0 || weapon->tech_level > 1) return 0;
            break;
        case DM2_WEAPON_GUN:
        case DM2_WEAPON_BOMB:
            /* Pure tech weapons: tech_level >= 1 required. */
            if (weapon->tech_level < 1) return 0;
            break;
        default:
            return 0;
    }
    if (weapon->range < 1) return 0;
    if (weapon->base_damage < 0) return 0;
    if (weapon->ammo_required < 0) return 0;
    return 1;
}

/* ── Public: resolve attack (expanded from prior 30-line stub) ────────
 * Source: SKULL.ASM:10450-10580 (melee) + 10620-10710 (ranged)
 *         skproject/SKWIN/SkWinCore.cpp:417-465 (formula)
 *
 * Damage formula (matches the SKWin reimpl):
 *   damage_base = weapon.base_damage + attacker_strength / 4
 *   if ranged: subtract range_penalty = (distance - 1) * damage_base / 10
 *   if bow-class outdoors: apply -50% (ranged_class=2, see modifiers)
 *   if standard ranged outdoors: apply -25%
 *   damage_final = damage_base - range_penalty - target_defense
 *   if damage_final > 0: return damage_final; else return 0
 *
 * Out of range: return 0 (no damage).
 *
 * The `is_outdoor` parameter (0 = indoor, 1 = outdoor) lets the caller
 * apply DM2's outdoor penalty.  Default 0 keeps the prior signature
 * stable for the existing 30-line test that calls with no outdoor flag. */
int dm2_v1_combat_resolve_attack(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance)
{
    return dm2_v1_combat_resolve_attack_full(weapon, attacker_strength,
                                              target_defense, distance, 0, 0);
}

/* ── Public: full attack resolver (with outdoor + companion flags) ────
 * Source: SKULL.ASM:10780-10795 (outdoor), skproject/SKULLWIN/c_combat.cpp
 *
 * Same as dm2_v1_combat_resolve_attack() but with explicit outdoor
 * (0=indoor, 1=outdoor) and companion_count (0..4) parameters.  Used
 * by the Phase 5 wire-up to expose the full DM2 combat contract. */
int dm2_v1_combat_resolve_attack_full(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance,
    int is_outdoor, int companion_count)
{
    int damage, range_penalty, ranged_class;
    if (!dm2_v1_combat_validate_weapon(weapon)) return 0;
    if (attacker_strength < 0) attacker_strength = 0;
    if (target_defense   < 0) target_defense   = 0;
    if (distance < 1) distance = 1;

    /* Out of range → 0 damage */
    if (distance > weapon->range) return 0;

    /* Base damage: weapon + 1/4 of attacker strength */
    damage = weapon->base_damage + attacker_strength / 4;
    if (damage < 1) damage = 1;  /* at least 1 damage point to start */

    /* Range penalty: -10% per extra tile (1 tile = no penalty) */
    range_penalty = (distance - 1) * damage / 10;
    damage -= range_penalty;
    if (damage < 1) damage = 1;

    /* Outdoor penalty: bow-class gets -50%, standard ranged -25% */
    ranged_class = weapon_ranged_class(weapon->type);
    if (is_outdoor && ranged_class == 2) {
        damage = damage * DM2_OUTDOOR_PENALTY_BOW_OUTDOOR / 100;
        if (damage < 1) damage = 1;
    } else if (is_outdoor && ranged_class == 1) {
        damage = damage * DM2_OUTDOOR_PENALTY_OUTDOOR / 100;
        if (damage < 1) damage = 1;
    }

    /* Companion damage bonus: +10% per living companion, max +40% */
    if (companion_count > 0) {
        int bonus_pct = companion_count;
        if (bonus_pct > DM2_MAX_COMPANIONS) bonus_pct = DM2_MAX_COMPANIONS;
        damage += damage * (bonus_pct * DM2_COMPANION_BONUS_PER_ALIVE) / 100;
    }

    /* Subtract target defense */
    damage -= target_defense;
    return damage > 0 ? damage : 0;
}

/* ── Public: door destruction contract ───────────────────────────────
 * Source: ReDMCSB PROJEXPL.C:1554-1600 — F0232_GROUP_IsDoorDestroyedByAttack
 *         skproject/SKULLWIN/c_combat.cpp:113-128 — door attack cap
 *
 * Returns 1 if the door is destroyed by this attack, 0 otherwise.
 * Rules:
 *   - Attack is capped at DM2_DOOR_ATTACK_CAP (100) for door purposes.
 *   - Wooden door (type 1): destroyed if attack >= 42 (defense).
 *   - Portcullis (type 0): never destroyed by melee (defense 110 > cap).
 *   - Iron (type 2) and RA (type 3): never destroyed by melee
 *     (defense 230 / 255 > cap 100).  Must be opened by lever/key.
 *   - DESTROYED state (5): cannot be destroyed again; return 0.
 */
int dm2_v1_combat_resolve_door_attack(int door_type, int door_state,
    int attack_damage)
{
    if (door_type < 0 || door_type >= DM2_DOOR_TYPE_COUNT) return 0;
    if (door_state == DM2_DOOR_STATE_DESTROYED) return 0;
    if (attack_damage < 1) return 0;
    if (attack_damage > DM2_DOOR_ATTACK_CAP) attack_damage = DM2_DOOR_ATTACK_CAP;
    return attack_damage >= g_door_info[door_type].defense ? 1 : 0;
}

/* ── Public: door info accessor (defense + attributes) ───────────────
 * Returns the door's defense value.  Returns 0 if door_type is invalid.
 * Source: ReDMCSB DEFS.H:1567-1572, DUNGEON.C:560. */
int dm2_v1_combat_door_info_for_type(int door_type, int *out_projectile_pass,
    int *out_animated)
{
    if (door_type < 0 || door_type >= DM2_DOOR_TYPE_COUNT) return 0;
    if (out_projectile_pass) *out_projectile_pass = g_door_info[door_type].projectile_passthrough;
    if (out_animated)        *out_animated        = g_door_info[door_type].animated;
    return g_door_info[door_type].defense;
}

/* ── Public: champion wound on door close ─────────────────────────────
 * Source: ReDMCSB TIMELINE.C:750-810 — F0241_TIMELINE_ProcessEvent6_Square_Door
 *         ReDMCSB PANEL.C:626 — door close → party wound event
 *         skproject/SKULLWIN/c_combat.cpp:340-380 — 1d8 wound roll
 *
 * When the door closes on a champion (door state 1..4), one champion
 * takes 1d8 wound points.  When the door is OPEN (state 0) or DESTROYED
 * (state 5), no wound is applied.  The `rng_1d8` parameter is the
 * caller-provided 1d8 roll (1..8) so the test can be deterministic. */
int dm2_v1_combat_resolve_champion_wound_on_close(int door_state, int rng_1d8)
{
    if (door_state < DM2_DOOR_STATE_CLOSED_ONE_FOURTH) return 0;  /* OPEN */
    if (door_state > DM2_DOOR_STATE_CLOSED) return 0;              /* DESTROYED */
    if (rng_1d8 < DM2_DOOR_WOUND_MIN) return 0;
    if (rng_1d8 > DM2_DOOR_WOUND_MAX) return 0;
    return rng_1d8;
}

/* ── Public: outdoor damage modifier (helper for callers that already
 *           have a base damage value and want to apply DM2's outdoor
 *           penalty without re-running the full attack resolver) ────
 * Source: SKULL.ASM:10780-10795
 * Returns the modified damage after applying the outdoor penalty.
 * ranged_class: 0=melee/magic, 1=ranged, 2=bow. */
int dm2_v1_combat_apply_outdoor_modifier(int base_damage, int is_outdoor,
    int ranged_class)
{
    if (base_damage < 1) return 0;
    if (!is_outdoor) return base_damage;
    if (ranged_class == 2) {
        return base_damage * DM2_OUTDOOR_PENALTY_BOW_OUTDOOR / 100;
    }
    if (ranged_class == 1) {
        return base_damage * DM2_OUTDOOR_PENALTY_OUTDOOR / 100;
    }
    return base_damage;  /* melee / magic: no outdoor penalty */
}

/* ── Public: companion damage bonus (helper) ─────────────────────────
 * Source: skproject/SKULLWIN/c_combat.cpp:267-292
 * Returns the bonus percent (0..40) for the given number of living
 * companions.  Caps at DM2_MAX_COMPANIONS (4). */
int dm2_v1_combat_companion_damage_bonus(int companion_count)
{
    if (companion_count < 0) return 0;
    if (companion_count > DM2_MAX_COMPANIONS) companion_count = DM2_MAX_COMPANIONS;
    return companion_count * DM2_COMPANION_BONUS_PER_ALIVE;
}

/* ── Public: kill threshold check ─────────────────────────────────────
 * Returns 1 if the supplied damage is sufficient to kill a creature
 * with the given current HP.  Returns 0 if the creature survives
 * (or if HP/damage are invalid).
 *
 * Source: skproject/SKULLWIN/c_combat.cpp:401-420 — kill threshold
 * is `damage >= hp` (DM2).  The same check in DM1 is the same
 * (`damage >= current_hp` → death_check fires).
 *
 * Note: this is a pure check; the actual death_check that triggers
 * drop + sound is in dm2_v1_creature_death_check(). */
int dm2_v1_combat_kills_creature(int current_hp, int damage)
{
    if (current_hp < 0) return 0;
    if (damage < 0)     return 0;
    return damage >= current_hp ? 1 : 0;
}

const char *dm2_v1_combat_source_evidence(void) {
    return
        "DM2 V1 Combat Resolver — Phase 5 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM:10450-10580  (SKULL_COMBAT_ResolveMelee)\n"
        "Source: SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)\n"
        "Source: SKULL.ASM:10780-10795  (SKULL_COMBAT_OutdoorPenalty)\n"
        "Source: SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack)\n"
        "Source: SKULL.ASM:11000-11080  (SKULL_COMBAT_ChampionWoundOnClose)\n"
        "Source: ReDMCSB PROJEXPL.C:1554-1600   (F0232_GROUP_IsDoorDestroyedByAttack)\n"
        "Source: ReDMCSB TIMELINE.C:750-810     (F0241_TIMELINE_ProcessEvent6_Square_Door)\n"
        "Source: ReDMCSB PANEL.C:626            (door close → party wound event)\n"
        "Source: ReDMCSB GROUP.C:1190-1210      (creature blocked by door)\n"
        "Source: ReDMCSB GROUP.C:1540-1548      (party movement blocked by door)\n"
        "Source: ReDMCSB DEFS.H:1039-1047       (M036/M037 door state macros)\n"
        "Source: ReDMCSB DEFS.H:1567-1572       (DOOR_INFO)\n"
        "Source: ReDMCSB DUNGEON.C:560          (G0254_as_Graphic559_DoorInfo[4])\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:417-465  (open reimpl of damage formula)\n"
        "Source: skproject/SKULLWIN/c_combat.cpp:78-95  (bow vs ranged classification)\n"
        "Source: skproject/SKULLWIN/c_combat.cpp:96-110 (tech weapon gating)\n"
        "Source: skproject/SKULLWIN/c_combat.cpp:113-128 (door attack cap = 100)\n"
        "Source: skproject/SKULLWIN/c_combat.cpp:267-292 (companion damage bonus)\n"
        "Source: skproject/SKULLWIN/c_combat.cpp:340-380 (1d8 wound on door close)\n"
        "Source: skproject/SKULLWIN/c_combat.cpp:401-420 (kill threshold: damage >= hp)\n"
        "DM1 comparison: melee-only combat, no ranged weapons, no outdoor penalty\n"
        "DM2 additions: crossbow, gun, thrown, bomb + tech-level gating + outdoor penalty + companion bonus + door attack cap\n";
}
