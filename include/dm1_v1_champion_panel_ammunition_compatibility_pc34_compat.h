#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_AMMUNITION_COMPATIBILITY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_AMMUNITION_COMPATIBILITY_PC34_COMPAT_H

/*
 * DM1 V1 Champion Panel ammunition compatibility contract.
 *
 * Source-lock anchors:
 * - ReDMCSB AMMO.C F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon:1-81
 *   rejects a non-weapon shooter slot, derives a required ammunition class
 *   from bow/sling weapon class ranges, then accepts only weapon ammunition
 *   whose class matches that derived class.
 * - ReDMCSB DEFS.H:1723-1729 defines the bow/sling ammunition classes and
 *   bow/sling shooter class ranges.
 * - ReDMCSB DEFS.H:7908-7914 declares F0294.
 * - ReDMCSB TIMELINE.C:1598 and 1603 are the two PC34 callers.
 *
 * Companion ammo_type gate (G0303 ammo type comparison; ammo_type,
 * ammo_class, ammo_compatibility fields) and contract-only synthetic
 * weapon class constants.
 *
 * Contract only: this slice uses synthetic thing type and WEAPON_INFO Class
 * values. It does not load game data, render graphics, or model real
 * champions, inventories, savegames, or F0293/F0297/F0298 behavior.
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_NONE_PC34 -1
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_MISC_PC34 0
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34 5

#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_SWING_WEAPON_PC34 0
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34 10
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_SLING_AMMUNITION_PC34 11
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_POISON_DART_PC34 12
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_BOW_PC34 16
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_BOW_PC34 31
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_SLING_PC34 32
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_SLING_PC34 47
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_MAGIC_WEAPON_PC34 112
#define DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_NONE_PC34 -1

typedef int dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t;
typedef int dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_ammunition_class_t;
typedef bool dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_bool_t;

typedef struct {
    int thing_type;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t weapon_class;
} dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_slot_t;

typedef struct {
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_slot_t weapon_slot;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_slot_t ammunition_slot;
} dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t;

typedef struct {
    const char *function_name;
    const char *function_anchor;
    const char *class_anchor;
    const char *prototype_anchor;
    const char *dispatch_context_anchor;
    const char *leader_hand_context_anchor;
    const char *timeline_caller_anchor;
    const char *non_overlap_anchor;
    const char *contract_scope;
    const char *no_real_asset_claim;
} dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor_t;

typedef struct {
    bool contract_only;
    bool loads_graphics_dat;
    bool loads_dungeon_dat;
    bool models_real_champion;
    bool covers_f0294_only;
    bool weapon_slot_must_be_weapon;
    bool bow_range_requires_bow_ammunition;
    bool sling_range_requires_sling_ammunition;
    bool non_shooting_weapon_rejected;
    bool ammunition_slot_must_be_weapon;
    bool ammunition_class_must_match_derived_class;
    int weapon_thing_type_constant;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_ammunition_class_t bow_ammunition_class;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_ammunition_class_t sling_ammunition_class;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t first_bow_class;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t last_bow_class;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t first_sling_class;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t last_sling_class;
} dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_invariant_t;

typedef struct {
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_invariant_t invariant;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor_t anchor;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_bool_t compatible;
    bool weapon_slot_is_weapon;
    bool ammunition_slot_is_weapon;
    bool weapon_info_queried;
    bool ammunition_info_queried;
    bool weapon_is_bow_range;
    bool weapon_is_sling_range;
    bool weapon_is_supported_shooter;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t weapon_class_seen;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_ammunition_class_t required_ammunition_class;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_ammunition_class_t ammunition_class_seen;
    bool null_input_defaults_used;
} dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t;

dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t
dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(
    const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t *input);

const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor_t *
dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor(void);

const char *
dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_AMMUNITION_COMPATIBILITY_PC34_COMPAT_H */
