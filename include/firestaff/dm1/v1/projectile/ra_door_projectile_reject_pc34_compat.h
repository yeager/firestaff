#ifndef FIRESTAFF_DM1_V1_PROJECTILE_RA_DOOR_PROJECTILE_REJECT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_RA_DOOR_PROJECTILE_REJECT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_PORTCULLIS_PC34 = 0,
    DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_WOODEN_PC34 = 1,
    DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_IRON_PC34 = 2,
    DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_RA_PC34 = 3,
    DM1_V1_RA_DOOR_PROJECTILE_MAX_NON_MAGIC_ATTACK_PC34 = 100,
    DM1_V1_RA_DOOR_PROJECTILE_RA_DEFENSE_PC34 = 255,
    DM1_V1_RA_DOOR_PROJECTILE_WOODEN_DEFENSE_PC34 = 42
};

typedef struct {
    int door_type;
    int defense;
    int attributes;
    int projectiles_can_pass_through;
    int creatures_can_see_through;
    int animated;
    const char *name;
    const char *source_anchor;
} DM1_V1_RaDoorProjectileDoorInfoPc34;

typedef struct {
    int contract_only;
    int no_real_assets;
    int f0217_calls_f0232_for_non_open_door_projectiles;
    int f0217_open_door_spell_skips_f0232;
    int f0217_passes_magic_attack_false;
    int f0232_requires_attack_at_least_defense;
    int f0232_requires_closed_door_state;
    int ra_door_type;
    int ra_defense;
    int melee_cap;
    int ra_rejected_at_melee_cap;
    int wooden_destroyed_at_melee_cap;
    int door_info_count;
    const char *dungeon_anchor;
    const char *projexpl_f0217_anchor;
    const char *projexpl_f0232_anchor;
    const char *non_overlap;
} DM1_V1_RaDoorProjectileRejectContractPc34;

typedef struct {
    int door_type;
    int door_defense;
    int input_attack;
    int bounded_attack;
    int door_state_closed;
    int is_open_door_spell;
    int magic_attack_flag;
    int f0217_reaches_f0232;
    int f0232_destroyed;
    int projectile_consumed;
    int door_state_after_destroyed;
    int source_locked_reject;
    const char *source_anchor;
} DM1_V1_RaDoorProjectileRejectResultPc34;

const DM1_V1_RaDoorProjectileRejectContractPc34 *
dm1_v1_ra_door_projectile_reject_contract_pc34(void);

const DM1_V1_RaDoorProjectileDoorInfoPc34 *
dm1_v1_ra_door_projectile_reject_door_info_pc34(size_t index);

size_t dm1_v1_ra_door_projectile_reject_door_info_count_pc34(void);

int dm1_v1_ra_door_projectile_reject_simulate_pc34(
    int door_type,
    int projectile_attack,
    int is_open_door_spell,
    int door_state_closed,
    DM1_V1_RaDoorProjectileRejectResultPc34 *out_result);

const char *
dm1_v1_ra_door_projectile_reject_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
