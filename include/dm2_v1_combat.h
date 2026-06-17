#ifndef FIRESTAFF_DM2_V1_COMBAT_H
#define FIRESTAFF_DM2_V1_COMBAT_H
#include <stdint.h>

/* DM2 V1 Combat Resolver — Phase 5 source-lock (creature/combat parity)
 *
 * DM2 extends DM1's melee-only combat with:
 *   - 4 ranged weapon types (crossbow, gun, thrown, bomb)
 *   - Tech weapons (gun/bomb) that require tech_level >= 1
 *   - Door destruction contract (capped at 100 attack points)
 *   - Champion wound on door close (1d8 roll)
 *   - Outdoor combat penalty (-25% standard, -50% bow-class)
 *   - Companion damage bonus (+10% per living companion, max +40%)
 *
 * Source: SKULL.ASM combat routines
 *   ReDMCSB SKULL.ASM:10450-10580  (SKULL_COMBAT_ResolveMelee)
 *   ReDMCSB SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)
 *   ReDMCSB SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack)
 *   ReDMCSB SKULL.ASM:11000-11080  (SKULL_COMBAT_ChampionWoundOnClose)
 *   ReDMCSB PROJEXPL.C:1554-1600   (F0232_GROUP_IsDoorDestroyedByAttack)
 *   ReDMCSB TIMELINE.C:750-810     (F0241_TIMELINE_ProcessEvent6_Square_Door)
 *   ReDMCSB PANEL.C:626            (door close → party wound event)
 *   ReDMCSB DEFS.H:1567-1572       (DOOR_INFO {Attributes, Defense})
 *   skproject/SKULLWIN/c_combat.cpp (open reimpl, mirror of SKULL.ASM)
 */

typedef enum {
    DM2_WEAPON_MELEE = 0,
    DM2_WEAPON_THROWN,
    DM2_WEAPON_CROSSBOW,
    DM2_WEAPON_GUN,       /* tech weapon, requires tech_level >= 1 */
    DM2_WEAPON_BOMB,      /* area-effect, requires tech_level >= 1 */
    DM2_WEAPON_MAGIC,
} DM2_WeaponType;

typedef struct {
    DM2_WeaponType type;
    int base_damage;
    int range;        /* tiles, 1 = melee */
    int ammo_required;
    int tech_level;   /* 0 = magic era, 1+ = tech */
} DM2_V1_WeaponInfo;

/* Original 30-line stub API (signature preserved for source compatibility) */
int dm2_v1_combat_resolve_attack(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance);
int dm2_v1_combat_is_ranged(DM2_WeaponType type);
const char *dm2_v1_combat_source_evidence(void);

/* ── Phase 5 expansion: full attack resolver with outdoor + companion ─ */
int dm2_v1_combat_resolve_attack_full(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance,
    int is_outdoor, int companion_count);

/* ── Phase 5 expansion: weapon validation ─────────────────────────── */
int dm2_v1_combat_validate_weapon(const DM2_V1_WeaponInfo *weapon);

/* ── Phase 5 expansion: door destruction contract ─────────────────── */
/* Returns 1 if attack_damage (capped at 100) is enough to destroy
 * the door at door_state.  DESTROYED doors return 0.  Iron/RA doors
 * are immune to melee (defense 230/255 > cap). */
int dm2_v1_combat_resolve_door_attack(int door_type, int door_state,
    int attack_damage);

/* Returns the door's defense value.  Side-effects: writes
 * out_projectile_pass (1 if projectiles pass through) and
 * out_animated (1 if door is animated/RA).  Returns 0 on invalid
 * door_type. */
int dm2_v1_combat_door_info_for_type(int door_type, int *out_projectile_pass,
    int *out_animated);

/* ── Phase 5 expansion: champion wound on door close ──────────────── */
/* Returns the wound points (1..8) when the door closes on a champion.
 * Returns 0 if the door is OPEN or DESTROYED, or if rng_1d8 is invalid. */
int dm2_v1_combat_resolve_champion_wound_on_close(int door_state, int rng_1d8);

/* ── Phase 5 expansion: outdoor modifier helper ───────────────────── */
/* Applies DM2's outdoor penalty to a pre-computed base damage value.
 * ranged_class: 0=melee/magic (no penalty), 1=ranged (-25%),
 *               2=bow-class (-50%). */
int dm2_v1_combat_apply_outdoor_modifier(int base_damage, int is_outdoor,
    int ranged_class);

/* ── Phase 5 expansion: companion damage bonus helper ────────────── */
/* Returns the bonus percent (0..40) for the given number of living
 * companions.  Caps at 4 companions. */
int dm2_v1_combat_companion_damage_bonus(int companion_count);

/* ── Phase 5 expansion: kill threshold check ─────────────────────── */
/* Returns 1 if `damage` is sufficient to kill a creature with
 * `current_hp`.  Mirrors skproject/SKULLWIN/c_combat.cpp:401-420. */
int dm2_v1_combat_kills_creature(int current_hp, int damage);

#endif /* FIRESTAFF_DM2_V1_COMBAT_H */
