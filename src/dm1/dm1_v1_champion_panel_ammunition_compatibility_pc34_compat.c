#include "dm1_v1_champion_panel_ammunition_compatibility_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-locked contract gate only.
 *
 * AMMO.C F0294:43-79 is the PC34-compatible ammunition gate. This module
 * models only the slot-type and WEAPON_INFO Class contract used by the two
 * TIMELINE.C callers; it does not touch real champion slots or game data.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. AMMO.C F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon:"
    "43-47 rejects weapon slots whose M012_TYPE is not C05_THING_TYPE_WEAPON. "
    "AMMO.C:51-58 uses F0158_DUNGEON_GetWeaponInfo and maps C016..C031 bow "
    "classes to C010_CLASS_BOW_AMMUNITION, and C032..C047 sling classes to "
    "C011_CLASS_SLING_AMMUNITION. AMMO.C:59-60 rejects other weapon classes. "
    "AMMO.C:72-79 fetches ammunition WEAPON_INFO on PC34 and returns true only "
    "when the ammunition slot type is C05_THING_TYPE_WEAPON and its Class "
    "equals the derived ammunition class. DEFS.H:1723-1729 anchors classes. "
    "DEFS.H:7908-7914 declares F0294. CHAMDRAW.C F0293:1117-1143 is only "
    "dispatch context and is not covered here. CHAMPION.C:243-292 covers "
    "leader hand put/get context and is not covered here. TIMELINE.C:1598 "
    "and 1603 are the two PC34 callers.";

static const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor_t s_anchor = {
    "F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon",
    "AMMO.C F0294:1-81",
    "DEFS.H:1723-1729",
    "DEFS.H:7908-7914",
    "CHAMDRAW.C F0293:1117-1143",
    "CHAMPION.C F0297/F0298:243-292",
    "TIMELINE.C:1598,1603",
    "contract-only F0294; does not cover F0293 dispatch, F0297/F0298 leader hand, graphics, savegame, or real champion state",
    "slot type plus WEAPON_INFO Class compatibility gate",
    "no real-asset bitmap parity; no GRAPHICS.DAT/DUNGEON.DAT load"
};

static const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_invariant_t s_invariant = {
    true,
    false,
    false,
    false,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
    DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34,
    DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_SLING_AMMUNITION_PC34,
    DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_BOW_PC34,
    DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_BOW_PC34,
    DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_SLING_PC34,
    DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_SLING_PC34
};

static bool is_weapon_type(
    const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_slot_t *slot)
{
    return slot->thing_type ==
           DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34;
}

static bool is_bow_class(
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t weapon_class)
{
    return weapon_class >=
               DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_BOW_PC34 &&
           weapon_class <=
               DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_BOW_PC34;
}

static bool is_sling_class(
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_weapon_class_t weapon_class)
{
    return weapon_class >=
               DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_SLING_PC34 &&
           weapon_class <=
               DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_SLING_PC34;
}

dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t
dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(
    const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t *input)
{
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t result;
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t local_input;

    memset(&result, 0, sizeof(result));
    result.invariant = s_invariant;
    result.anchor = s_anchor;
    result.required_ammunition_class =
        DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_NONE_PC34;
    result.ammunition_class_seen =
        DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_NONE_PC34;
    result.weapon_class_seen =
        DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_NONE_PC34;

    if (!input) {
        memset(&local_input, 0, sizeof(local_input));
        local_input.weapon_slot.thing_type =
            DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_NONE_PC34;
        local_input.ammunition_slot.thing_type =
            DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_NONE_PC34;
        input = &local_input;
        result.null_input_defaults_used = true;
    }

    result.weapon_slot_is_weapon = is_weapon_type(&input->weapon_slot);
    if (!result.weapon_slot_is_weapon) {
        return result;
    }

    /*
     * AMMO.C F0294:51-58: PC34 calls F0158_DUNGEON_GetWeaponInfo for the
     * shooter slot and derives the only accepted ammunition class.
     */
    result.weapon_info_queried = true;
    result.weapon_class_seen = input->weapon_slot.weapon_class;
    result.weapon_is_bow_range = is_bow_class(result.weapon_class_seen);
    result.weapon_is_sling_range = is_sling_class(result.weapon_class_seen);

    if (result.weapon_is_bow_range) {
        result.required_ammunition_class =
            DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34;
    } else if (result.weapon_is_sling_range) {
        result.required_ammunition_class =
            DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_SLING_AMMUNITION_PC34;
    } else {
        return result;
    }

    result.weapon_is_supported_shooter = true;
    result.ammunition_slot_is_weapon = is_weapon_type(&input->ammunition_slot);

    /*
     * AMMO.C F0294:72-79: on PC34 the ammunition WEAPON_INFO lookup precedes
     * the final type-and-class return expression.
     */
    result.ammunition_info_queried = true;
    result.ammunition_class_seen = input->ammunition_slot.weapon_class;
    result.compatible =
        result.ammunition_slot_is_weapon &&
        result.ammunition_class_seen == result.required_ammunition_class;

    return result;
}

const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor_t *
dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor(void)
{
    return &s_anchor;
}

const char *
dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_source_evidence(void)
{
    return s_source_evidence;
}
