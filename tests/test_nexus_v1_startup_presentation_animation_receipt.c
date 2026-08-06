#include "nexus_v1_startup_menu.h"
#include "nexus_v1_title_sequence.h"

#include <stdio.h>
#include <string.h>

static int failures;

void nexus_v1_save_init(Nexus_V1_SaveManager *mgr, const char *save_dir)
{
    (void)mgr;
    (void)save_dir;
}

int nexus_v1_save_scan(Nexus_V1_SaveManager *mgr)
{
    (void)mgr;
    return 0;
}

const Nexus_V1_SaveSlot *nexus_v1_save_get_slot(
    const Nexus_V1_SaveManager *mgr,
    uint8_t slot)
{
    (void)mgr;
    (void)slot;
    return 0;
}

void nexus_v1_save_default_dir(char *buf, size_t bufsz)
{
    if (buf && bufsz > 0U) buf[0] = '\0';
}

int nexus_v1_champion_unrecruit_last(Nexus_V1_ChampionPool *pool)
{
    (void)pool;
    return 0;
}

int nexus_v1_champion_next_selectable(const Nexus_V1_ChampionPool *pool,
                                      int start,
                                      int step)
{
    (void)pool;
    (void)step;
    return start;
}

int nexus_v1_champion_recruit_and_advance(Nexus_V1_ChampionPool *pool,
                                          int champion_index,
                                          int *out_next_cursor)
{
    (void)pool;
    if (out_next_cursor) *out_next_cursor = champion_index;
    return 0;
}

int nexus_v1_startup_save_row_rect(int row, Nexus_V1_StartupRect *out_rect)
{
    (void)row;
    (void)out_rect;
    return 0;
}

int nexus_v1_startup_save_hit(int row_count,
                              int x,
                              int y,
                              Nexus_V1_StartupHit *out_hit)
{
    (void)row_count;
    (void)x;
    (void)y;
    (void)out_hit;
    return 0;
}

int nexus_v1_startup_champion_row_rect(int row,
                                       Nexus_V1_StartupRect *out_rect)
{
    (void)row;
    (void)out_rect;
    return 0;
}

int nexus_v1_startup_champion_footer_rect(Nexus_V1_StartupRect *out_rect)
{
    (void)out_rect;
    return 0;
}

int nexus_v1_startup_champion_visible_first_row(int champion_count,
                                                int cursor,
                                                int max_visible)
{
    (void)champion_count;
    (void)max_visible;
    return cursor;
}

int nexus_v1_startup_champion_hit_at_cursor(int champion_count,
                                            int cursor,
                                            int x,
                                            int y,
                                            Nexus_V1_StartupHit *out_hit)
{
    (void)champion_count;
    (void)cursor;
    (void)x;
    (void)y;
    (void)out_hit;
    return 0;
}

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void check_receipt(int title_active,
                          int save_active,
                          int champion_active,
                          int title_frame,
                          const char *expected_phase,
                          const char *expected_animation,
                          int expected_startup_active,
                          int expected_animation_active,
                          int expected_title_frame,
                          const char *message)
{
    char phase[32];
    char animation[32];
    int startup_active = -1;
    int startup_frame = -1;
    int animation_active = -1;
    int title_out = -2;
    int title_max = -1;
    int title_ready = -1;

    memset(phase, 0, sizeof(phase));
    memset(animation, 0, sizeof(animation));
    expect(nexus_v1_startup_presentation_receipt(
               title_active,
               save_active,
               champion_active,
               title_frame,
               phase,
               (int)sizeof(phase),
               &startup_active,
               &startup_frame,
               animation,
               (int)sizeof(animation),
               &animation_active,
               &title_out,
               &title_max,
               &title_ready) &&
               strcmp(phase, expected_phase) == 0 &&
               strcmp(animation, expected_animation) == 0 &&
               startup_active == expected_startup_active &&
               startup_frame == title_frame &&
               animation_active == expected_animation_active &&
               title_out == expected_title_frame &&
               title_max == nexus_v1_boot_start_ready_frames(),
           message);
}

static Nexus_V1_StartupPresentationAnimationPackageGateInput package_gate_input(
    const char *animation,
    int animation_active,
    int title_active,
    int save_active,
    int champion_active)
{
    Nexus_V1_StartupPresentationAnimationPackageGateInput input;

    memset(&input, 0, sizeof(input));
    input.animation = animation;
    input.animation_active = animation_active;
    input.title_active = title_active;
    input.save_select_active = save_active;
    input.champion_select_active = champion_active;
    input.full_start_package_receipt_ready = 1;
    input.host_display_caller_expected = 1;
    input.real_package_assets_bound = 1;
    input.saturn_timing_exact = 1;
    input.saturn_capture_frames_exact = 1;
    input.saturn_presentation_capture_bound = 1;
    return input;
}

static void check_package_gate(
    Nexus_V1_StartupPresentationAnimationPackageGateInput input,
    int expected_result,
    int expected_draw,
    int expected_title,
    int expected_champion,
    int expected_inactive,
    const char *message)
{
    Nexus_V1_StartupPresentationAnimationPackageGateReceipt receipt;
    int result;

    memset(&receipt, 0xff, sizeof(receipt));
    result = nexus_v1_startup_presentation_animation_package_gate(
        &input,
        &receipt);
    expect(result == expected_result &&
               receipt.animation_draw_permitted == expected_draw &&
               receipt.package_animation_bound == expected_draw &&
               receipt.title_animation_requested == expected_title &&
               receipt.champion_animation_requested == expected_champion &&
               receipt.inactive_runtime_or_save == expected_inactive &&
               receipt.synthetic_visuals_permitted == 0 &&
               receipt.guessed_saturn_decoder_permitted == 0 &&
               receipt.fallback_visuals_blocked == 1 &&
               receipt.blocked_no_draw == (expected_draw ? 0 : 1),
           message);
}

static Nexus_V1_StartupMenuAnimationHandoffGateInput handoff_gate_input(
    const Nexus_V1_StartupPresentationAnimationPackageGateReceipt *package)
{
    Nexus_V1_StartupMenuAnimationHandoffGateInput input;

    memset(&input, 0, sizeof(input));
    input.package_gate = package;
    input.menu_bpk_handoff_receipt_valid = 1;
    input.menu_bpk_source_hash_verified = 1;
    input.menu_bpk_can_render_stored_surfaces = 1;
    return input;
}

static void check_handoff_gate(
    Nexus_V1_StartupMenuAnimationHandoffGateInput input,
    int expected_result,
    int expected_draw,
    int expected_handoff_bound,
    int expected_prs3_required,
    const char *message)
{
    Nexus_V1_StartupMenuAnimationHandoffGateReceipt receipt;
    int result;

    memset(&receipt, 0xff, sizeof(receipt));
    result = nexus_v1_startup_menu_animation_handoff_gate(&input, &receipt);
    expect(result == expected_result &&
               receipt.animation_draw_permitted == expected_draw &&
               receipt.boot_menu_real_data_route_bound == expected_draw &&
               receipt.menu_bpk_handoff_bound == expected_handoff_bound &&
               receipt.prs3_trace_required == expected_prs3_required &&
               receipt.synthetic_visuals_permitted == 0 &&
               receipt.guessed_saturn_decoder_permitted == 0 &&
               receipt.fallback_visuals_blocked == 1 &&
               receipt.blocked_no_draw == (expected_draw ? 0 : 1),
           message);
}

int main(void)
{
    Nexus_V1_StartupPresentationAnimationPackageGateReceipt title_package;
    Nexus_V1_StartupPresentationAnimationPackageGateInput title_package_input;

    check_receipt(1,
                  0,
                  0,
                  7,
                  "nexus-title",
                  "nexus-title",
                  1,
                  1,
                  7,
                  "title presentation exposes active Saturn title animation");
    check_receipt(0,
                  1,
                  0,
                  11,
                  "nexus-save-select",
                  "nexus-runtime",
                  1,
                  0,
                  -1,
                  "save menu uses runtime animation without synthetic activity");
    check_receipt(0,
                  0,
                  1,
                  13,
                  "nexus-champion-select",
                  "nexus-champion-select",
                  1,
                  1,
                  -1,
                  "champion menu marks its named presentation animation active");
    check_receipt(0,
                  0,
                  0,
                  17,
                  "nexus-runtime",
                  "nexus-runtime",
                  0,
                  0,
                  -1,
                  "runtime state does not claim startup animation activity");

    check_package_gate(package_gate_input("nexus-title", 1, 1, 0, 0),
                       1,
                       1,
                       1,
                       0,
                       0,
                       "title animation gate requires proven package data");
    check_package_gate(
        package_gate_input("nexus-champion-select", 1, 0, 0, 1),
        1,
        1,
        0,
        1,
        0,
        "champion animation gate accepts proven package data");

    {
        Nexus_V1_StartupPresentationAnimationPackageGateInput blocked =
            package_gate_input("nexus-champion-select", 1, 0, 0, 1);
        blocked.menu_bpk_prs3_blocked = 1;
        check_package_gate(blocked,
                           0,
                           0,
                           0,
                           1,
                           0,
                           "PRS3-blocked champion gate remains no-draw");
    }
    {
        Nexus_V1_StartupPresentationAnimationPackageGateInput blocked =
            package_gate_input("nexus-champion-select", 1, 0, 0, 1);
        blocked.fallback_visuals_permitted = 1;
        check_package_gate(blocked,
                           0,
                           0,
                           0,
                           1,
                           0,
                           "fallback visuals never unlock menu animation");
    }
    {
        Nexus_V1_StartupPresentationAnimationPackageGateInput blocked =
            package_gate_input("nexus-champion-select", 1, 0, 0, 1);
        blocked.saturn_timing_exact = 0;
        check_package_gate(blocked,
                           0,
                           0,
                           0,
                           1,
                           0,
                           "inexact Saturn timing keeps animation no-draw");
    }
    {
        Nexus_V1_StartupPresentationAnimationPackageGateInput blocked =
            package_gate_input("nexus-title", 1, 1, 0, 0);
        blocked.saturn_presentation_capture_bound = 0;
        check_package_gate(blocked,
                           0,
                           0,
                           1,
                           0,
                           0,
                           "source assets and timing without Saturn presentation capture remain no-draw");
    }
    check_package_gate(package_gate_input("nexus-runtime", 0, 0, 1, 0),
                       1,
                       0,
                       0,
                       0,
                       1,
                       "save/runtime presentation remains accepted no-draw");

    title_package_input = package_gate_input("nexus-title", 1, 1, 0, 0);
    expect(nexus_v1_startup_presentation_animation_package_gate(
               &title_package_input,
               &title_package) == 1 &&
               title_package.animation_draw_permitted == 1,
           "title package gate prepares handoff input");
    check_handoff_gate(handoff_gate_input(&title_package),
                       1,
                       1,
                       1,
                       0,
                       "ready stored MENU.BPK handoff keeps route real-data");
    {
        Nexus_V1_StartupMenuAnimationHandoffGateInput blocked =
            handoff_gate_input(&title_package);
        blocked.menu_bpk_prs3_trace_required = 1;
        blocked.menu_bpk_blocks_real_menu_surface_render = 1;
        blocked.menu_bpk_can_render_stored_surfaces = 0;
        check_handoff_gate(blocked,
                           0,
                           0,
                           0,
                           1,
                           "PRS3 trace requirement blocks menu animation");
    }
    {
        Nexus_V1_StartupMenuAnimationHandoffGateInput blocked =
            handoff_gate_input(&title_package);
        blocked.menu_bpk_source_hash_verified = 0;
        check_handoff_gate(blocked,
                           0,
                           0,
                           0,
                           0,
                           "unverified MENU.BPK source blocks menu animation");
    }
    {
        Nexus_V1_StartupMenuAnimationHandoffGateInput blocked =
            handoff_gate_input(&title_package);
        blocked.fallback_visuals_permitted = 1;
        check_handoff_gate(blocked,
                           0,
                           0,
                           0,
                           0,
                           "fallback handoff never unlocks menu animation");
    }

    if (failures) return 1;
    puts("Nexus startup presentation animation receipt passed");
    return 0;
}
