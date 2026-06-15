#include "dm1_v1_champion_panel_ammunition_compatibility_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_str_eq(const char *id, const char *got, const char *want,
                          const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               id, got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s == \"%s\" (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t
make_input(int weapon_type, int weapon_class, int ammo_type, int ammo_class)
{
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t input;

    input.weapon_slot.thing_type = weapon_type;
    input.weapon_slot.weapon_class = weapon_class;
    input.ammunition_slot.thing_type = ammo_type;
    input.ammunition_slot.weapon_class = ammo_class;
    return input;
}

static void test_evidence_and_invariants(void)
{
    const dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor_t *anchor =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_anchor();
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(NULL);
    const char *source =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_source_evidence();

    expect_bool("invariant.contract_only", result.invariant.contract_only, true,
                "AMMO.C F0294:1-81 contract-only gate");
    expect_bool("invariant.no_graphics_dat", result.invariant.loads_graphics_dat, false,
                "AMMO.C F0294:1-81 no GRAPHICS.DAT load");
    expect_bool("invariant.no_dungeon_dat", result.invariant.loads_dungeon_dat, false,
                "AMMO.C F0294:1-81 no DUNGEON.DAT load");
    expect_bool("invariant.no_real_champion", result.invariant.models_real_champion, false,
                "AMMO.C F0294:34 reads M516 only in source, synthetic here");
    expect_bool("invariant.covers_f0294_only", result.invariant.covers_f0294_only, true,
                "AMMO.C F0294:1-81 bounded scope");
    expect_bool("invariant.weapon_type_gate",
                result.invariant.weapon_slot_must_be_weapon, true,
                "AMMO.C F0294:43-47 weapon slot type gate");
    expect_bool("invariant.bow_range",
                result.invariant.bow_range_requires_bow_ammunition, true,
                "AMMO.C F0294:54-55 bow class range");
    expect_bool("invariant.sling_range",
                result.invariant.sling_range_requires_sling_ammunition, true,
                "AMMO.C F0294:57-58 sling class range");
    expect_bool("invariant.non_shooting_rejected",
                result.invariant.non_shooting_weapon_rejected, true,
                "AMMO.C F0294:59-60 unsupported weapon classes");
    expect_bool("invariant.ammo_type_gate",
                result.invariant.ammunition_slot_must_be_weapon, true,
                "AMMO.C F0294:78-79 ammunition type gate");
    expect_bool("invariant.ammo_class_match",
                result.invariant.ammunition_class_must_match_derived_class, true,
                "AMMO.C F0294:78-79 class equality gate");
    expect_int("invariant.weapon_type_constant",
               result.invariant.weapon_thing_type_constant, 5,
               "AMMO.C F0294:43-47 C05_THING_TYPE_WEAPON");
    expect_int("invariant.bow_ammo_class",
               result.invariant.bow_ammunition_class, 10,
               "DEFS.H:1723 C010_CLASS_BOW_AMMUNITION");
    expect_int("invariant.sling_ammo_class",
               result.invariant.sling_ammunition_class, 11,
               "DEFS.H:1724 C011_CLASS_SLING_AMMUNITION");
    expect_int("invariant.first_bow", result.invariant.first_bow_class, 16,
               "DEFS.H:1726 C016_CLASS_FIRST_BOW");
    expect_int("invariant.last_bow", result.invariant.last_bow_class, 31,
               "DEFS.H:1727 C031_CLASS_LAST_BOW");
    expect_int("invariant.first_sling", result.invariant.first_sling_class, 32,
               "DEFS.H:1728 C032_CLASS_FIRST_SLING");
    expect_int("invariant.last_sling", result.invariant.last_sling_class, 47,
               "DEFS.H:1729 C047_CLASS_LAST_SLING");

    expect_str_eq("anchor.function_name", anchor->function_name,
                  "F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon",
                  "DEFS.H:7908-7914 F0294 prototype");
    expect_str_eq("anchor.function_anchor", anchor->function_anchor,
                  "AMMO.C F0294:1-81",
                  "AMMO.C F0294:1-81");
    expect_str_eq("anchor.class_anchor", anchor->class_anchor,
                  "DEFS.H:1723-1729",
                  "DEFS.H:1723-1729 class constants");
    expect_str_eq("anchor.timeline_callers", anchor->timeline_caller_anchor,
                  "TIMELINE.C:1598,1603",
                  "TIMELINE.C:1598 and 1603 real callers");
    expect_contains("anchor.non_overlap", anchor->non_overlap_anchor,
                    "does not cover F0293 dispatch",
                    "CHAMDRAW.C F0293:1117-1143 non-overlap");
    expect_contains("source.pc34_get_weapon_info", source,
                    "F0158_DUNGEON_GetWeaponInfo",
                    "AMMO.C F0294:51-58 and 72-79 lookup anchor");
    expect_contains("source.timeline_callers", source,
                    "TIMELINE.C:1598",
                    "TIMELINE.C F0253 caller one");
    expect_bool("null_input.defaults", result.null_input_defaults_used, true,
                "AMMO.C F0294:43-47 synthetic empty slot probe");
    expect_bool("null_input.compatible", result.compatible, false,
                "AMMO.C F0294:43-47 empty weapon slot rejects");
}

static void test_compatible_bow_and_sling_cases(void)
{
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t bow =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_BOW_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t sling =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_SLING_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_SLING_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t bow_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&bow);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t sling_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&sling);

    expect_bool("bow.compatible", bow_result.compatible, true,
                "AMMO.C F0294:54-55 and 78-79 bow ammo accepted");
    expect_bool("bow.weapon_info_queried", bow_result.weapon_info_queried, true,
                "AMMO.C F0294:51-52 shooter F0158 lookup");
    expect_bool("bow.ammo_info_queried", bow_result.ammunition_info_queried, true,
                "AMMO.C F0294:72-73 ammunition F0158 lookup");
    expect_bool("bow.is_bow_range", bow_result.weapon_is_bow_range, true,
                "AMMO.C F0294:54 C016 first bow boundary");
    expect_int("bow.required_class", bow_result.required_ammunition_class, 10,
               "AMMO.C F0294:55 derives C010 bow ammunition");
    expect_int("bow.ammo_class_seen", bow_result.ammunition_class_seen, 10,
               "AMMO.C F0294:78-79 class equality");

    expect_bool("sling.compatible", sling_result.compatible, true,
                "AMMO.C F0294:57-58 and 78-79 sling ammo accepted");
    expect_bool("sling.is_sling_range", sling_result.weapon_is_sling_range, true,
                "AMMO.C F0294:57 C047 last sling boundary");
    expect_int("sling.required_class", sling_result.required_ammunition_class, 11,
               "AMMO.C F0294:58 derives C011 sling ammunition");
    expect_int("sling.weapon_class_seen", sling_result.weapon_class_seen, 47,
               "DEFS.H:1729 C047_CLASS_LAST_SLING");
    expect_bool("sling.supported_shooter", sling_result.weapon_is_supported_shooter, true,
                "AMMO.C F0294:57-58 supported shooter");
}

static void test_mismatched_ammunition_cases(void)
{
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t bow_with_sling =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_LAST_BOW_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_SLING_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t sling_with_bow =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_SLING_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t bow_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&bow_with_sling);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t sling_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&sling_with_bow);

    expect_bool("bow_with_sling.compatible", bow_result.compatible, false,
                "AMMO.C F0294:78-79 bow requires C010 ammo class");
    expect_bool("bow_with_sling.boundary", bow_result.weapon_is_bow_range, true,
                "AMMO.C F0294:54 C031 last bow boundary");
    expect_int("bow_with_sling.required", bow_result.required_ammunition_class, 10,
               "AMMO.C F0294:55 derives bow ammo");
    expect_int("bow_with_sling.seen", bow_result.ammunition_class_seen, 11,
               "DEFS.H:1724 sling ammunition mismatch");

    expect_bool("sling_with_bow.compatible", sling_result.compatible, false,
                "AMMO.C F0294:78-79 sling requires C011 ammo class");
    expect_bool("sling_with_bow.boundary", sling_result.weapon_is_sling_range, true,
                "AMMO.C F0294:57 C032 first sling boundary");
    expect_int("sling_with_bow.required", sling_result.required_ammunition_class, 11,
               "AMMO.C F0294:58 derives sling ammo");
    expect_int("sling_with_bow.seen", sling_result.ammunition_class_seen, 10,
               "DEFS.H:1723 bow ammunition mismatch");
}

static void test_edge_rejections(void)
{
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t empty_weapon =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_NONE_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_BOW_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t non_weapon =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_MISC_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_BOW_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t swing =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_SWING_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t empty_ammo =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_BOW_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_NONE_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_input_t magic =
        make_input(DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_FIRST_MAGIC_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_THING_TYPE_WEAPON_PC34,
                   DM1_V1_CHAMPION_PANEL_AMMO_COMPAT_CLASS_BOW_AMMUNITION_PC34);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t empty_weapon_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&empty_weapon);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t non_weapon_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&non_weapon);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t swing_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&swing);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t empty_ammo_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&empty_ammo);
    dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe_result_t magic_result =
        dm1_v1_champion_panel_ammunition_compatibility_pc34_compat_probe(&magic);

    expect_bool("empty_weapon.compatible", empty_weapon_result.compatible, false,
                "AMMO.C F0294:43-47 empty weapon slot rejects");
    expect_bool("empty_weapon.weapon_info_not_queried",
                empty_weapon_result.weapon_info_queried, false,
                "AMMO.C F0294:43-47 return before F0158");
    expect_bool("non_weapon.compatible", non_weapon_result.compatible, false,
                "AMMO.C F0294:43-47 non-weapon slot rejects");
    expect_bool("non_weapon.weapon_slot_is_weapon",
                non_weapon_result.weapon_slot_is_weapon, false,
                "AMMO.C F0294:43-47 C05 type required");

    expect_bool("swing.compatible", swing_result.compatible, false,
                "AMMO.C F0294:59-60 swing weapon is not bow/sling");
    expect_bool("swing.weapon_info_queried", swing_result.weapon_info_queried, true,
                "AMMO.C F0294:51-52 lookup before class rejection");
    expect_bool("swing.ammo_info_not_queried", swing_result.ammunition_info_queried, false,
                "AMMO.C F0294:59-60 return before ammunition lookup");
    expect_int("swing.required_none", swing_result.required_ammunition_class, -1,
               "AMMO.C F0294:59-60 no derived ammunition class");

    expect_bool("empty_ammo.compatible", empty_ammo_result.compatible, false,
                "AMMO.C F0294:78-79 ammunition slot must be weapon");
    expect_bool("empty_ammo.ammo_info_queried",
                empty_ammo_result.ammunition_info_queried, true,
                "AMMO.C F0294:72-73 PC34 fetches ammunition info before return");
    expect_bool("empty_ammo.ammo_slot_is_weapon",
                empty_ammo_result.ammunition_slot_is_weapon, false,
                "AMMO.C F0294:78-79 C05 type required for ammo");

    expect_bool("magic.compatible", magic_result.compatible, false,
                "AMMO.C F0294:59-60 magic weapon class rejected");
    expect_bool("magic.supported_shooter", magic_result.weapon_is_supported_shooter, false,
                "DEFS.H:1730 magic/special class outside bow/sling ranges");
    expect_int("magic.weapon_class_seen", magic_result.weapon_class_seen, 112,
               "DEFS.H:1730 C112_CLASS_FIRST_MAGIC_WEAPON");
}

int main(void)
{
    test_evidence_and_invariants();
    test_compatible_bow_and_sling_cases();
    test_mismatched_ammunition_cases();
    test_edge_rejections();

    if (g_failures) {
        printf("FAIL dm1_v1_champion_panel_ammunition_compatibility_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_champion_panel_ammunition_compatibility_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
