#include "theron_v1_startup_flow.h"
#include "theron_v1_boot.h"
#include "theron_v1_startup_media.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v1_startup_save_resume.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

static void check_int(const char *label, int got, int expected) {
    if (got == expected) {
        printf("PASS %s=%d\n", label, got);
        ++g_pass;
    } else {
        printf("FAIL %s got=%d expected=%d\n", label, got, expected);
        ++g_fail;
    }
}

static void check_contains(const char *label, const char *haystack, const char *needle) {
    if (haystack && needle && strstr(haystack, needle)) {
        printf("PASS %s contains %s\n", label, needle);
        ++g_pass;
    } else {
        printf("FAIL %s missing %s in %s\n",
               label,
               needle ? needle : "(null)",
               haystack ? haystack : "(null)");
        ++g_fail;
    }
}

static void check_str(const char *label, const char *got, const char *expected) {
    if (got && expected && strcmp(got, expected) == 0) {
        printf("PASS %s=%s\n", label, got);
        ++g_pass;
    } else {
        printf("FAIL %s got=%s expected=%s\n",
               label,
               got ? got : "(null)",
               expected ? expected : "(null)");
        ++g_fail;
    }
}

static int plan_has_graphic_kind(
    const Theron_StartupRenderPlan *plan,
    Theron_StartupRenderGraphicKind kind) {
    int i;
    if (!plan) {
        return 0;
    }
    for (i = 0; i < plan->graphic_count; ++i) {
        if (plan->graphics[i].kind == kind) {
            return 1;
        }
    }
    return 0;
}

static void check_render_plan_graphic(
    const char *label,
    const Theron_StartupRenderPlan *plan,
    Theron_StartupRenderGraphicKind kind) {
    check_int(label, plan_has_graphic_kind(plan, kind), 1);
}

typedef struct GraphicProbe {
    int fill_count;
    int rect_count;
    int pixel_count;
} GraphicProbe;

static void graphic_probe_fill(
    void *userdata, int x, int y, int w, int h, int color) {
    GraphicProbe *probe = (GraphicProbe*)userdata;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)color;
    if (probe) {
        ++probe->fill_count;
    }
}

static void graphic_probe_rect(
    void *userdata, int x, int y, int w, int h, int color) {
    GraphicProbe *probe = (GraphicProbe*)userdata;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)color;
    if (probe) {
        ++probe->rect_count;
    }
}

static void graphic_probe_pixel(void *userdata, int x, int y, int color) {
    GraphicProbe *probe = (GraphicProbe*)userdata;
    (void)x;
    (void)y;
    (void)color;
    if (probe) {
        ++probe->pixel_count;
    }
}

static void check_render_plan_executor(
    const char *label,
    const Theron_StartupRenderPlan *plan,
    int expect_pixels) {
    GraphicProbe probe;
    Theron_StartupGraphicExecutor executor;
    char scoped[128];

    memset(&probe, 0, sizeof(probe));
    executor.userdata = &probe;
    executor.fill_rect = graphic_probe_fill;
    executor.draw_rect = graphic_probe_rect;
    executor.plot_pixel = graphic_probe_pixel;
    snprintf(scoped, sizeof(scoped), "%s executor rc", label);
    check_int(scoped, theron_v1_startup_execute_graphics_plan(plan, &executor), 1);
    snprintf(scoped, sizeof(scoped), "%s executor fill", label);
    check_int(scoped, probe.fill_count > 0, 1);
    snprintf(scoped, sizeof(scoped), "%s executor rect", label);
    check_int(scoped, probe.rect_count > 0, 1);
    if (expect_pixels) {
        snprintf(scoped, sizeof(scoped), "%s executor pixels", label);
        check_int(scoped, probe.pixel_count > 0, 1);
    }
}

static void raw_sector_put_text(unsigned char *sector,
                                size_t sector_size,
                                size_t user_offset,
                                const char *text) {
    size_t text_len = text ? strlen(text) : 0u;
    size_t raw_offset = THERON_TRACK02_RAW_USER_DATA_OFFSET + user_offset;
    if (!sector || !text || raw_offset >= sector_size ||
        text_len > sector_size - raw_offset) {
        return;
    }
    memcpy(sector + raw_offset, text, text_len);
}

static int fake_runtime_level_load(Theron_V1_World *world,
                                   Theron_DungeonID dungeon_id,
                                   void *userdata,
                                   char *receipt,
                                   size_t receipt_cap) {
    int *called = (int*)userdata;

    if (called) {
        ++(*called);
    }
    if (!world || dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT) {
        return 0;
    }
    world->current_dungeon = dungeon_id;
    world->current_level = 0;
    world->level_loaded[dungeon_id - 1][0] = 1;
    world->party.leader_x = 5;
    world->party.leader_y = 6;
    world->party.leader_dir = 1;
    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "fake level stage=%d start=(5,6,1)",
                 (int)dungeon_id);
    }
    return 1;
}

static int fake_runtime_level_load_fail(Theron_V1_World *world,
                                        Theron_DungeonID dungeon_id,
                                        void *userdata,
                                        char *receipt,
                                        size_t receipt_cap) {
    int *called = (int*)userdata;

    (void)world;
    (void)dungeon_id;
    if (called) {
        ++(*called);
    }
    if (receipt && receipt_cap > 0u) {
        snprintf(receipt, receipt_cap, "forced load failure");
    }
    return 0;
}

int main(void) {
    Theron_DungeonProgression progression;
    Theron_StartupFlow flow;
    Theron_V1_Party party;
    Theron_V1_World world;
    Theron_StartupResult result;
    char phase_label[64];
    int startup_active = -1;

    printf("=== Theron V1 startup flow probe ===\n");
    printf("source: %s\n", theron_v1_startup_flow_source_evidence());

    theron_v1_dungeon_progression_init(&progression);
    theron_v1_startup_flow_init(&flow);
    check_int("init phase", flow.phase, THERON_STARTUP_PHASE_TITLE);
    check_int("init selected dungeon", flow.selected_dungeon, THERON_DUNGEON_INVALID);
    check_int("init companion count", flow.companion_count, 0);
    {
        int order[THERON_STARTUP_MAX_COMPANIONS] = { 2, 4, 1 };
        Theron_StartupFlowSnapshotRequest request;
        memset(&request, 0, sizeof(request));
        request.phase = THERON_STARTUP_PHASE_SOUL_ROOM;
        request.selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
        request.selected_mirrors_mask = (1 << 2) | (1 << 4) | (1 << 1);
        request.companion_count = 99;
        request.selected_mirror_order = order;
        request.selected_mirror_order_count = THERON_STARTUP_MAX_COMPANIONS;
        result = theron_v1_startup_flow_rebuild_from_request(&request,
                                                             &progression,
                                                             &flow);
        check_int("rebuild from request result", result, THERON_STARTUP_OK);
        check_int("rebuild from request phase",
                  flow.phase,
                  THERON_STARTUP_PHASE_READY);
        check_int("rebuild from request selected dungeon",
                  flow.selected_dungeon,
                  THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("rebuild from request companion clamp",
                  flow.companion_count,
                  THERON_STARTUP_MAX_COMPANIONS);
        theron_v1_startup_flow_init(&flow);
    }
    {
        unsigned char us_sector[THERON_TRACK02_RAW_SECTOR_BYTES];
        Theron_StartupMedia media;
        memset(us_sector, 0, sizeof(us_sector));
        raw_sector_put_text(us_sector,
                            sizeof(us_sector),
                            32u,
                            "GO AWAY AND RESURRECT THERON");
        theron_v1_startup_media_capture_track02(us_sector,
                                                sizeof(us_sector),
                                                THERON_TRACK02_MD5_US_BIN,
                                                &media);
        check_int("startup media US text status",
                  media.startup_text_prompt_status,
                  THERON_TRACK02_SIGNAL_OK);
        check_int("startup media US text count",
                  media.startup_text_prompt_count,
                  1);
        check_str("startup media US text",
                  media.startup_text_prompt,
                  "GO AWAY AND RESURRECT THERON");
        check_int("startup media US roster unsupported",
                  media.startup_roster_name_status,
                  THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
    }
    {
        unsigned char jp_sector[THERON_TRACK02_RAW_SECTOR_BYTES];
        Theron_StartupMedia media;
        memset(jp_sector, 0, sizeof(jp_sector));
        raw_sector_put_text(jp_sector,
                            sizeof(jp_sector),
                            48u,
                            "THERON MARA GUARDIAN OF WISDO LINOS THE RESOLUTE "
                            "HEXA LORD OF FEALTY HAKAR THE BRAVE TIRAN "
                            "KNIGHT OF STRENGT DOTAN MASTER OF THE WIN "
                            "PENTAI THE SURVIVOR");
        theron_v1_startup_media_capture_track02(jp_sector,
                                                sizeof(jp_sector),
                                                THERON_TRACK02_MD5_JP_BIN,
                                                &media);
        check_int("startup media JP roster status",
                  media.startup_roster_name_status,
                  THERON_TRACK02_SIGNAL_OK);
        check_int("startup media JP roster count",
                  media.startup_roster_name_count,
                  8);
        check_str("startup media JP mirror 0 roster",
                  media.startup_roster_names[4],
                  "HAKAR");
        check_str("startup media JP mirror 6 title",
                  media.startup_roster_titles[7],
                  "THE SURVIVOR");
        check_int("startup media JP text status",
                  media.startup_text_prompt_status,
                  THERON_TRACK02_SIGNAL_OK);
        check_int("startup media JP US prompt absent",
                  media.startup_text_prompt_count,
                  0);
    }
    check_int("Firestaff none input maps to Theron idle",
              theron_v1_startup_input_from_firestaff_menu_code(0),
              THERON_STARTUP_INPUT_NONE);
    check_int("Firestaff up input maps to Theron up",
              theron_v1_startup_input_from_firestaff_menu_code(1),
              THERON_STARTUP_INPUT_UP);
    check_int("Firestaff down input maps to Theron down",
              theron_v1_startup_input_from_firestaff_menu_code(2),
              THERON_STARTUP_INPUT_DOWN);
    check_int("Firestaff left input maps to Theron left",
              theron_v1_startup_input_from_firestaff_menu_code(3),
              THERON_STARTUP_INPUT_LEFT);
    check_int("Firestaff right input maps to Theron right",
              theron_v1_startup_input_from_firestaff_menu_code(4),
              THERON_STARTUP_INPUT_RIGHT);
    check_int("Firestaff turn-left input maps to Theron left",
              theron_v1_startup_input_from_firestaff_menu_code(7),
              THERON_STARTUP_INPUT_LEFT);
    check_int("Firestaff turn-right input maps to Theron right",
              theron_v1_startup_input_from_firestaff_menu_code(8),
              THERON_STARTUP_INPUT_RIGHT);
    check_int("Firestaff accept input maps to Theron accept",
              theron_v1_startup_input_from_firestaff_menu_code(9),
              THERON_STARTUP_INPUT_ACCEPT);
    check_int("Firestaff back input maps to Theron back",
              theron_v1_startup_input_from_firestaff_menu_code(10),
              THERON_STARTUP_INPUT_BACK);
    check_int("Firestaff action input maps to Theron action",
              theron_v1_startup_input_from_firestaff_menu_code(11),
              THERON_STARTUP_INPUT_ACTION);
    check_int("unknown Firestaff input maps to Theron idle",
              theron_v1_startup_input_from_firestaff_menu_code(999),
              THERON_STARTUP_INPUT_NONE);
    check_int("startup availability stage 1",
              theron_v1_startup_stage_available(
                  &progression,
                  THERON_DUNGEON_1_HALL_OF_RECORDS),
              1);
    check_int("startup availability locked stage 2",
              theron_v1_startup_stage_available(
                  &progression,
                  THERON_DUNGEON_2_CRYPT_OF_SHADOWS),
              0);
    check_int("startup availability invalid stage",
              theron_v1_startup_stage_available(
                  &progression,
                  THERON_DUNGEON_INVALID),
              0);
    {
        Theron_StartupLayoutState layout_state;
        Theron_StartupLayoutElement elements[16];
        Theron_StartupRenderPlan render_plan;
        int element_count;

        theron_v1_startup_layout_state_init(&layout_state);
        layout_state.phase = THERON_STARTUP_PHASE_TITLE;
        layout_state.selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
        snprintf(layout_state.chapter_label,
                 sizeof(layout_state.chapter_label),
                 "Chapter 1: Hall of Records");
        element_count = theron_v1_startup_layout_build(
            &layout_state, elements, 16);
        check_int("startup title render plan builds",
                  theron_v1_startup_render_plan_build(
                      &layout_state, elements, element_count, &render_plan),
                  1);
        check_render_plan_graphic(
            "startup title owns title mark graphic",
            &render_plan,
            THERON_STARTUP_RENDER_GRAPHIC_TITLE_MARK);
        check_render_plan_executor(
            "startup title",
            &render_plan,
            1);

        {
            Theron_StartupLayoutStateRequest request;
            Theron_StartupChapterInspectRequest inspect_request;
            Theron_StartupChapterInspectReceipt inspect_receipt;
            Theron_V1_BootProfile profile;
            Theron_V1_World world;
            char fact_names[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY + 1]
                           [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
            char fact_titles[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY + 1]
                            [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];
            const char *names[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY + 1];
            const char *titles[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY + 1];
            int order[THERON_STARTUP_MAX_COMPANIONS + 1] = { 6, 2, 1, 0 };

            memset(&request, 0, sizeof(request));
            memset(&profile, 0, sizeof(profile));
            memset(fact_names, 0, sizeof(fact_names));
            memset(fact_titles, 0, sizeof(fact_titles));
            memset(names, 0, sizeof(names));
            memset(titles, 0, sizeof(titles));
            theron_v1_world_init(&world);
            world.progression.current_dungeon =
                THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
            names[0] = "THERON";
            titles[0] = "APPRENTICE";
            names[1] = "HAKAR";
            titles[1] = "FIGHTER";
            request.phase = THERON_STARTUP_PHASE_READY;
            request.selected_dungeon = THERON_DUNGEON_COUNT + 99;
            request.boot_profile = &profile;
            request.world = &world;
            request.soul_cursor = 7;
            request.continue_focus = 1;
            request.has_tqsv_continue = 1;
            request.tqsv_slot = 3;
            request.startup_text_prompt = "READY";
            request.startup_roster_names = names;
            request.startup_roster_titles = titles;
            request.startup_roster_name_count =
                THERON_STARTUP_LAYOUT_ROSTER_CAPACITY + 1;
            request.selected_mirrors_mask = 0x45;
            request.selected_mirror_order = order;
            request.selected_mirror_order_count =
                THERON_STARTUP_MAX_COMPANIONS + 1;

            check_int("layout state request builds",
                      theron_v1_startup_layout_state_from_request(
                          &request, &layout_state),
                      1);
            check_int("layout state request clamps dungeon",
                      layout_state.selected_dungeon,
                      THERON_DUNGEON_1_HALL_OF_RECORDS);
            check_contains("layout state request chapter",
                           layout_state.chapter_label,
                           "Chapter 2");
            check_int("layout state request roster cap",
                      layout_state.startup_roster_name_count,
                      THERON_STARTUP_LAYOUT_ROSTER_CAPACITY);
            check_int("layout state request order cap",
                      layout_state.selected_mirror_order_count,
                      THERON_STARTUP_MAX_COMPANIONS);
            check_str("layout state request roster name",
                      layout_state.startup_roster_names[1],
                      "HAKAR");
            check_str("layout state request prompt",
                      layout_state.startup_text_prompt,
                      "READY");
            snprintf(fact_names[1], sizeof(fact_names[1]), "%s", "HAKAR");
            snprintf(fact_titles[1], sizeof(fact_titles[1]), "%s", "FIGHTER");
            check_int("layout state facts helper builds",
                      theron_v1_startup_layout_state_from_facts(
                          THERON_STARTUP_PHASE_READY,
                          THERON_DUNGEON_COUNT + 99,
                          &profile,
                          &world,
                          7,
                          1,
                          1,
                          3,
                          0,
                          -1,
                          "READY",
                          fact_names,
                          fact_titles,
                          THERON_STARTUP_LAYOUT_ROSTER_CAPACITY + 1,
                          0x45,
                          order,
                          THERON_STARTUP_MAX_COMPANIONS + 1,
                          &layout_state),
                      1);
            check_int("layout state facts helper roster cap",
                      layout_state.startup_roster_name_count,
                      THERON_STARTUP_LAYOUT_ROSTER_CAPACITY);
            check_str("layout state facts helper roster name",
                      layout_state.startup_roster_names[1],
                      "HAKAR");
            check_int("layout state facts helper order cap",
                      layout_state.selected_mirror_order_count,
                      THERON_STARTUP_MAX_COMPANIONS);

            memset(&inspect_request, 0, sizeof(inspect_request));
            inspect_request.boot_profile = &profile;
            inspect_request.world = &world;
            inspect_request.prefix = "continued slot=3";
            check_int("chapter inspect receipt builds",
                      theron_v1_startup_chapter_inspect_receipt_from_request(
                          &inspect_request,
                          &inspect_receipt),
                      1);
            check_str("chapter inspect receipt scope",
                      inspect_receipt.inspect_scope,
                      "STARTUP");
            check_contains("chapter inspect receipt marker",
                           inspect_receipt.marker_line,
                           "Chapter 2");
            check_contains("chapter inspect receipt detail prefix",
                           inspect_receipt.inspect_detail,
                           "continued slot=3");
            check_contains("chapter inspect receipt detail marker",
                           inspect_receipt.inspect_detail,
                           "Chapter 2");
            check_int("chapter inspect facts receipt builds",
                      theron_v1_startup_chapter_inspect_receipt_from_facts(
                          &profile,
                          &world,
                          "forcefield",
                          &inspect_receipt),
                      1);
            check_contains("chapter inspect facts marker",
                           inspect_receipt.marker_line,
                           "Chapter 2");
            check_contains("chapter inspect facts detail prefix",
                           inspect_receipt.inspect_detail,
                           "forcefield");
        }

        theron_v1_startup_layout_state_init(&layout_state);
        layout_state.selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
        snprintf(layout_state.chapter_label,
                 sizeof(layout_state.chapter_label),
                 "Chapter 1: Hall of Records");
        layout_state.phase = THERON_STARTUP_PHASE_STAGE_SELECT;
        layout_state.progression = &progression;
        element_count = theron_v1_startup_layout_build(
            &layout_state, elements, 16);
        check_int("startup stage render plan builds",
                  theron_v1_startup_render_plan_build(
                      &layout_state, elements, element_count, &render_plan),
                  1);
        check_render_plan_graphic(
            "startup stage owns stage-panel graphics",
            &render_plan,
            THERON_STARTUP_RENDER_GRAPHIC_STAGE_PANEL);
        check_render_plan_executor(
            "startup stage",
            &render_plan,
            0);
        {
            Theron_StartupAction pointer_action;
            Theron_StartupResult pointer_result = THERON_STARTUP_ERR_NULL;
            int handled;

            handled = theron_v1_startup_handle_pointer_from_layout_state(
                &layout_state,
                0,
                0,
                0,
                44,
                66,
                &pointer_result,
                &pointer_action);
            check_int("startup stage pointer handled", handled, 1);
            check_int("startup stage pointer result",
                      pointer_result,
                      THERON_STARTUP_OK);
            check_int("startup stage pointer action",
                      pointer_action.kind,
                      THERON_STARTUP_ACTION_CHOOSE_STAGE);
            check_int("startup stage pointer selected dungeon",
                      pointer_action.selected_dungeon,
                      THERON_DUNGEON_1_HALL_OF_RECORDS);
            handled = theron_v1_startup_handle_pointer_from_layout_state(
                &layout_state,
                0,
                0,
                0,
                1,
                1,
                &pointer_result,
                &pointer_action);
            check_int("startup stage pointer miss ignored", handled, 0);
            check_int("startup stage pointer miss result",
                      pointer_result,
                      THERON_STARTUP_OK);
            check_int("startup stage pointer miss action",
                      pointer_action.kind,
                      THERON_STARTUP_ACTION_NONE);
            layout_state.has_tqsv_continue = 1;
            layout_state.tqsv_slot = 2;
            layout_state.continue_focus = 0;
            handled = theron_v1_startup_handle_input_from_layout_state(
                &layout_state,
                THERON_STARTUP_INPUT_UP,
                &pointer_action);
            check_int("startup layout input result",
                      handled,
                      THERON_STARTUP_OK);
            check_int("startup layout input action",
                      pointer_action.kind,
                      THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR);
            check_int("startup layout input focuses continue",
                      pointer_action.continue_focus,
                      1);
        }

        layout_state.phase = THERON_STARTUP_PHASE_SOUL_ROOM;
        layout_state.soul_cursor = 0;
        element_count = theron_v1_startup_layout_build(
            &layout_state, elements, 16);
        check_int("startup soul-room render plan builds",
                  theron_v1_startup_render_plan_build(
                      &layout_state, elements, element_count, &render_plan),
                  1);
        check_render_plan_graphic(
            "startup soul-room owns mirror graphics",
            &render_plan,
            THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME);
        check_render_plan_graphic(
            "startup soul-room owns forcefield graphic",
            &render_plan,
            THERON_STARTUP_RENDER_GRAPHIC_FORCEFIELD);
        check_render_plan_executor(
            "startup soul-room",
            &render_plan,
            0);
        {
            Theron_StartupAction pointer_action;
            Theron_StartupResult pointer_result = THERON_STARTUP_ERR_NULL;
            int handled = theron_v1_startup_handle_pointer_from_layout_state(
                &layout_state,
                0,
                0,
                0,
                50,
                78,
                &pointer_result,
                &pointer_action);
            check_int("startup soul pointer handled", handled, 1);
            check_int("startup soul pointer result",
                      pointer_result,
                      THERON_STARTUP_OK);
            check_int("startup soul pointer action",
                      pointer_action.kind,
                      THERON_STARTUP_ACTION_TOGGLE_MIRROR);
            check_int("startup soul pointer mirror",
                      pointer_action.mirror_index,
                      0);
        }
    }

    {
        int selected = -1;
        result = theron_v1_startup_show_stage_select(
            &flow,
            THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
        check_int("show stage-select rc", result, THERON_STARTUP_OK);
        check_int("show stage-select phase",
                  flow.phase,
                  THERON_STARTUP_PHASE_STAGE_SELECT);
        check_int("show stage-select selected dungeon",
                  flow.selected_dungeon,
                  THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
        result = theron_v1_startup_toggle_mirror(&flow, 0, &selected);
        check_int("toggle mirror before choose rejected",
                  result,
                  THERON_STARTUP_ERR_NO_STAGE);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("toggle fixture choose rc", result, THERON_STARTUP_OK);
        result = theron_v1_startup_toggle_mirror(&flow, 2, &selected);
        check_int("toggle mirror selects rc", result, THERON_STARTUP_OK);
        check_int("toggle mirror selected flag", selected, 1);
        check_int("toggle mirror selected mask",
                  flow.selected_mirrors_mask,
                  1 << 2);
        result = theron_v1_startup_toggle_mirror(&flow, 2, &selected);
        check_int("toggle mirror deselects rc", result, THERON_STARTUP_OK);
        check_int("toggle mirror deselected flag", selected, 0);
        check_int("toggle mirror deselected mask",
                  flow.selected_mirrors_mask,
                  0);
    }

    theron_v1_startup_flow_init(&flow);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror before stage rejected", result, THERON_STARTUP_ERR_NO_STAGE);

    {
        Theron_StartupFlow rebuilt;
        Theron_StartupFlowSnapshotRequest snapshot_request;
        Theron_StartupFlowSnapshot snapshot;
        int request_order[THERON_STARTUP_MAX_COMPANIONS + 1] = {6, 2, 0, 4};

        memset(&snapshot_request, 0, sizeof(snapshot_request));
        snapshot_request.phase = THERON_STARTUP_PHASE_READY;
        snapshot_request.selected_dungeon = THERON_DUNGEON_COUNT + 4;
        snapshot_request.selected_mirrors_mask = (1 << 6) | (1 << 2) | 1;
        snapshot_request.companion_count = THERON_STARTUP_MAX_COMPANIONS + 2;
        snapshot_request.selected_mirror_order = request_order;
        snapshot_request.selected_mirror_order_count =
            THERON_STARTUP_MAX_COMPANIONS + 1;
        check_int("snapshot request builds",
                  theron_v1_startup_flow_snapshot_from_request(
                      &snapshot_request, &snapshot),
                  1);
        check_int("snapshot request phase",
                  snapshot.phase,
                  THERON_STARTUP_PHASE_READY);
        check_int("snapshot request companion cap",
                  snapshot.companion_count,
                  THERON_STARTUP_MAX_COMPANIONS);
        check_int("snapshot request order cap value",
                  snapshot.selected_mirror_order[2],
                  0);

        snapshot.phase = THERON_STARTUP_PHASE_STAGE_SELECT;
        snapshot.selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
        snapshot.selected_mirrors_mask = 0;
        snapshot.companion_count = 0;
        result = theron_v1_startup_flow_rebuild_from_snapshot(
            &snapshot,
            &progression,
            &rebuilt);
        check_int("snapshot rebuild stage-select rc",
                  result,
                  THERON_STARTUP_OK);
        check_int("snapshot rebuild stage-select phase",
                  rebuilt.phase,
                  THERON_STARTUP_PHASE_STAGE_SELECT);
        check_int("snapshot rebuild stage-select companion count",
                  rebuilt.companion_count,
                  0);

        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("snapshot stage choose rc", result, THERON_STARTUP_OK);
        check_int("snapshot select mirror 6 rc",
                  theron_v1_startup_select_mirror(&flow, 6),
                  THERON_STARTUP_OK);
        check_int("snapshot select mirror 2 rc",
                  theron_v1_startup_select_mirror(&flow, 2),
                  THERON_STARTUP_OK);
        check_int("snapshot select mirror 0 rc",
                  theron_v1_startup_select_mirror(&flow, 0),
                  THERON_STARTUP_OK);
        theron_v1_startup_flow_capture_snapshot(&flow, &snapshot);
        check_int("snapshot phase", snapshot.phase, THERON_STARTUP_PHASE_READY);
        check_int("snapshot companion count", snapshot.companion_count, 3);
        check_int("snapshot order 0", snapshot.selected_mirror_order[0], 6);
        check_int("snapshot order 1", snapshot.selected_mirror_order[1], 2);
        check_int("snapshot order 2", snapshot.selected_mirror_order[2], 0);
        snapshot.selected_mirror_order[1] = 6;
        result = theron_v1_startup_flow_rebuild_from_snapshot(
            &snapshot,
            &progression,
            &rebuilt);
        check_int("snapshot rebuild rc", result, THERON_STARTUP_OK);
        check_int("snapshot rebuild phase", rebuilt.phase, THERON_STARTUP_PHASE_READY);
        check_int("snapshot rebuild companion count", rebuilt.companion_count, 3);
        check_int("snapshot rebuild order 0", rebuilt.selected_mirror_order[0], 6);
        check_int("snapshot rebuild order 1 skips duplicate",
                  rebuilt.selected_mirror_order[1],
                  0);
        check_int("snapshot rebuild order 2 fills masked mirror",
                  rebuilt.selected_mirror_order[2],
                  2);
        result = theron_v1_startup_flow_rebuild_from_facts(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_COUNT + 4,
            (1 << 6) | (1 << 2) | 1,
            THERON_STARTUP_MAX_COMPANIONS + 2,
            request_order,
            THERON_STARTUP_MAX_COMPANIONS + 1,
            &progression,
            &rebuilt);
        check_int("facts rebuild rc", result, THERON_STARTUP_OK);
        check_int("facts rebuild clamps dungeon",
                  rebuilt.selected_dungeon,
                  THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("facts rebuild companion count",
                  rebuilt.companion_count,
                  3);
        check_int("facts rebuild order 0",
                  rebuilt.selected_mirror_order[0],
                  6);
        check_int("facts rebuild order 1",
                  rebuilt.selected_mirror_order[1],
                  2);
        check_int("facts rebuild order 2",
                  rebuilt.selected_mirror_order[2],
                  0);
    }

    {
        Theron_StartupAction action;
        Theron_StartupHit hit;
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_TITLE,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("title accept action rc", result, THERON_STARTUP_OK);
        check_int("title accept shows stage select",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            1,
            THERON_STARTUP_INPUT_UP,
            &action);
        check_int("stage up wraps to continue rc", result, THERON_STARTUP_OK);
        check_int("stage up action moves cursor",
                  action.kind,
                  THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR);
        check_int("stage up focuses continue", action.continue_focus, 1);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_3_ABYSS_OF_FLAMES,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_DOWN,
            &action);
        check_int("stage down rc", result, THERON_STARTUP_OK);
        check_int("stage down selected dungeon",
                  action.selected_dungeon,
                  THERON_DUNGEON_4_TOMB_OF_WOE);
        result = theron_v1_startup_handle_input_with_progression(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            &progression,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_DOWN,
            &action);
        check_int("initial locked stages skipped rc", result, THERON_STARTUP_OK);
        check_int("initial locked stages keep stage 1",
                  action.selected_dungeon,
                  THERON_DUNGEON_1_HALL_OF_RECORDS);
        {
            Theron_DungeonProgression progressed;
            theron_v1_dungeon_progression_init(&progressed);
            (void)theron_v1_dungeon_advance(&progressed);
            result = theron_v1_startup_handle_input_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_1_HALL_OF_RECORDS,
                &progressed,
                0,
                0,
                0,
                THERON_STARTUP_INPUT_DOWN,
                &action);
            check_int("post-first stage down rc", result, THERON_STARTUP_OK);
            check_int("post-first stage down skips complete stage",
                      action.selected_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
            result = theron_v1_startup_handle_input_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_6_CASTLE_OF_FATE,
                &progressed,
                0,
                0,
                0,
                THERON_STARTUP_INPUT_DOWN,
                &action);
            check_int("locked final stage skipped rc", result, THERON_STARTUP_OK);
            check_int("locked final stage skipped",
                      action.selected_dungeon,
                      THERON_DUNGEON_6_CASTLE_OF_FATE);
            result = theron_v1_startup_handle_input_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                &progressed,
                0,
                0,
                1,
                THERON_STARTUP_INPUT_UP,
                &action);
            check_int("post-first stage up rc", result, THERON_STARTUP_OK);
            check_int("post-first stage up focuses continue",
                      action.continue_focus,
                      1);
            check_int("post-first stage up keeps first available",
                      action.selected_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
        }
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_4_TOMB_OF_WOE,
            0,
            1,
            1,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("stage continue accept rc", result, THERON_STARTUP_OK);
        check_int("stage continue accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_CONTINUE_SAVE);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_4_TOMB_OF_WOE,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_ACTION,
            &action);
        check_int("stage choose action rc", result, THERON_STARTUP_OK);
        check_int("stage choose action kind",
                  action.kind,
                  THERON_STARTUP_ACTION_CHOOSE_STAGE);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_SOUL_ROOM,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_LEFT,
            &action);
        check_int("soul left rc", result, THERON_STARTUP_OK);
        check_int("soul left wraps to forcefield",
                  action.cursor,
                  THERON_STARTUP_HERO_MIRROR_COUNT);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            THERON_STARTUP_HERO_MIRROR_COUNT,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("ready forcefield accept rc", result, THERON_STARTUP_OK);
        check_int("ready forcefield accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_ENTER_FORCEFIELD);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            3,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("ready mirror accept rc", result, THERON_STARTUP_OK);
        check_int("ready mirror accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_TOGGLE_MIRROR);
        check_int("ready mirror accept index", action.mirror_index, 3);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            3,
            0,
            0,
            THERON_STARTUP_INPUT_BACK,
            &action);
        check_int("ready back rc", result, THERON_STARTUP_OK);
        check_int("ready back action",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);
        check_contains("action label",
                       theron_v1_startup_action_name(
                           THERON_STARTUP_ACTION_ENTER_FORCEFIELD),
                       "forcefield");
        {
            Theron_StartupActionPlan plan;
            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER;
            result = theron_v1_startup_plan_for_action(&action, &plan)
                         ? THERON_STARTUP_OK
                         : THERON_STARTUP_ERR_BAD_STAGE;
            check_int("plan return rc", result, THERON_STARTUP_OK);
            check_int("plan return kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_RETURN_TO_LAUNCHER);
            check_str("plan return scope", plan.status_scope, "RETURN");
            check_str("plan return status", plan.status, "BACK TO LAUNCHER");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
            action.selected_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
            action.cursor = 1;
            action.continue_focus = 0;
            check_int("plan stage-select rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan stage-select kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_SHOW_STAGE_SELECT);
            check_int("plan stage-select dungeon",
                      plan.selected_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
            check_str("plan stage-select status", plan.status, "STAGE SELECT");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR;
            action.selected_dungeon = THERON_DUNGEON_4_TOMB_OF_WOE;
            action.continue_focus = 1;
            check_int("plan stage-cursor rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan stage-cursor kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_MOVE_STAGE_CURSOR);
            check_int("plan stage-cursor continue",
                      plan.continue_focus,
                      1);
            check_str("plan stage-cursor status", plan.status, "STAGE CURSOR");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_CONTINUE_SAVE;
            check_int("plan continue rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan continue kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_CONTINUE_SAVE);
            check_str("plan continue status", plan.status, "CONTINUE LOADED");
            check_str("plan continue failure",
                      plan.failure_status,
                      "CONTINUE FAILED");
            {
                Theron_V1StartupContinueResult continue_result;
                Theron_V1StartupContinueApplyReceipt continue_receipt;
                Theron_V1StartupContinueAvailability continue_availability;
                Theron_StartupStateReceipt state_receipt;

                theron_v1_startup_continue_result_init(&continue_result);
                continue_result.source =
                    THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV;
                continue_result.selected_dungeon =
                    THERON_DUNGEON_4_TOMB_OF_WOE;
                continue_result.party_x = 9;
                continue_result.party_y = 10;
                continue_result.party_dir = 2;
                continue_result.tick_count = 77;
                check_int("continue receipt rc",
                          theron_v1_startup_continue_apply_receipt(
                              &plan,
                              &continue_result,
                              "continued slot=2 dungeon=1 label=TQSV",
                              "chapter=1 level=0",
                              &continue_receipt),
                          1);
                check_int("continue receipt redraw",
                          continue_receipt.input_result,
                          THERON_STARTUP_INPUT_RESULT_REDRAW);
                check_str("continue receipt status",
                          continue_receipt.status,
                          "CONTINUE LOADED");
                check_str("continue receipt inspect",
                          continue_receipt.inspect_scope,
                          "STARTUP");
                check_contains("continue receipt detail",
                               continue_receipt.inspect_detail,
                               "continued slot=2");
                check_contains("continue receipt marker",
                               continue_receipt.inspect_detail,
                               "chapter=1");
                check_int("continue availability tqsv rc",
                          theron_v1_startup_continue_availability_from_state(
                              THERON_V1_STARTUP_RESUME_TQSV,
                              2,
                              -1,
                              THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT,
                              &continue_availability),
                          1);
                check_int("continue availability tqsv",
                          continue_availability.has_tqsv_continue,
                          1);
                check_int("continue availability any",
                          continue_availability.has_any_continue,
                          1);
                check_int("continue availability srm off",
                          continue_availability.has_srm_continue,
                          0);
                check_int("continue availability srm rc",
                          theron_v1_startup_continue_availability_from_state(
                              THERON_V1_STARTUP_RESUME_NONE,
                              -1,
                              3,
                              THERON_V1_SRM_PROGRESS_IMPORT_OK,
                              &continue_availability),
                          1);
                check_int("continue availability srm",
                          continue_availability.has_srm_continue,
                          1);
                check_int("continue availability srm slot",
                          continue_availability.srm_slot,
                          3);
                check_int("continue availability null",
                          theron_v1_startup_continue_availability_from_state(
                              THERON_V1_STARTUP_RESUME_DUAL,
                              1,
                              2,
                              THERON_V1_SRM_PROGRESS_IMPORT_OK,
                              NULL),
                          0);
                check_int("continue state receipt rc",
                          theron_v1_startup_continue_state_receipt_from_result(
                              &continue_result,
                              &state_receipt),
                          1);
                check_int("continue state receipt flow",
                          state_receipt.flow_changed,
                          1);
                check_int("continue state receipt phase",
                          state_receipt.flow.phase,
                          THERON_STARTUP_PHASE_STAGE_SELECT);
                check_int("continue state receipt dungeon",
                          state_receipt.flow.selected_dungeon,
                          THERON_DUNGEON_4_TOMB_OF_WOE);
                check_int("continue state receipt pose",
                          state_receipt.party_x,
                          9);
                check_int("continue state receipt tick",
                          state_receipt.tick_count,
                          77);
                {
                    Theron_V1StartupContinueRequest continue_request;
                    char continue_receipt_text[96];
                    theron_v1_startup_continue_request_init(
                        &continue_request);
                    continue_receipt_text[0] = '\0';
                    check_int("continue combined no-source rc",
                              theron_v1_startup_continue_apply_request_with_receipts(
                                  &world,
                                  &continue_request,
                                  &plan,
                                  "chapter=1 level=0",
                                  &continue_result,
                                  &continue_receipt,
                                  &state_receipt,
                                  continue_receipt_text,
                                  sizeof(continue_receipt_text)),
                              0);
                    check_int("continue combined no-source result",
                              continue_result.source,
                              THERON_V1_STARTUP_CONTINUE_SOURCE_NONE);
                    check_int("continue combined no-source receipt",
                              continue_receipt.input_result,
                              THERON_STARTUP_INPUT_RESULT_IGNORED);
                    check_contains("continue combined no-source text",
                                   continue_receipt_text,
                                   "Continue requires");
                    check_int("continue facts no-source rc",
                              theron_v1_startup_continue_apply_facts_with_inspect_receipts(
                                  &world,
                                  THERON_V1_STARTUP_RESUME_NONE,
                                  -1,
                                  NULL,
                                  -1,
                                  THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT,
                                  NULL,
                                  &plan,
                                  NULL,
                                  &continue_result,
                                  &continue_receipt,
                                  &state_receipt,
                                  continue_receipt_text,
                                  sizeof(continue_receipt_text)),
                              0);
                    check_int("continue facts no-source result",
                              continue_result.source,
                              THERON_V1_STARTUP_CONTINUE_SOURCE_NONE);
                    check_int("continue facts no-source receipt",
                              continue_receipt.input_result,
                              THERON_STARTUP_INPUT_RESULT_IGNORED);
                }
            }

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_CHOOSE_STAGE;
            action.selected_dungeon = THERON_DUNGEON_5_VAULT_OF_SECRETS;
            check_int("plan choose-stage rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan choose-stage kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_CHOOSE_STAGE);
            check_str("plan choose-stage status", plan.status, "SOUL ROOM");
            check_str("plan choose-stage failure",
                      plan.failure_status,
                      "STAGE LOCKED");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR;
            action.cursor = 4;
            check_int("plan soul-cursor rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan soul-cursor kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_MOVE_SOUL_CURSOR);
            check_int("plan soul-cursor cursor", plan.cursor, 4);
            check_str("plan soul-cursor status",
                      plan.status,
                      "SOUL ROOM CURSOR");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_TOGGLE_MIRROR;
            action.mirror_index = 6;
            check_int("plan mirror-toggle rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan mirror-toggle kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_TOGGLE_MIRROR);
            check_int("plan mirror-toggle index", plan.mirror_index, 6);
            check_str("plan mirror-toggle status",
                      plan.status,
                      "HERO RESURRECTED");
            check_str("plan mirror-toggle alternate",
                      plan.alternate_status,
                      "HERO RELEASED");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
            check_int("plan forcefield rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan forcefield kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_ENTER_FORCEFIELD);
            check_str("plan forcefield scope", plan.status_scope, "BOOT");
            check_str("plan forcefield status", plan.status, "THERON READY");
            {
                Theron_V1StartupRuntimeEntryResult runtime_result;
                Theron_V1StartupRuntimeEntryApplyReceipt runtime_receipt;
                Theron_StartupStateReceipt state_receipt;
                Theron_StartupFlow runtime_flow;

                theron_v1_startup_runtime_entry_result_init(&runtime_result);
                theron_v1_startup_flow_init(&runtime_flow);
                runtime_flow.phase = THERON_STARTUP_PHASE_IN_DUNGEON;
                runtime_flow.selected_dungeon =
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
                runtime_result.level_loaded = 1;
                runtime_result.party_x = 3;
                runtime_result.party_y = 4;
                runtime_result.party_dir = 1;
                runtime_result.tick_count = 99;
                check_int("runtime receipt rc",
                          theron_v1_startup_runtime_entry_apply_receipt(
                              &plan,
                              &runtime_result,
                              "Track 02 initial level bind=OK",
                              &runtime_receipt),
                          1);
                check_int("runtime receipt redraw",
                          runtime_receipt.input_result,
                          THERON_STARTUP_INPUT_RESULT_REDRAW);
                check_str("runtime receipt status",
                          runtime_receipt.status,
                          "THERON READY");
                check_str("runtime receipt inspect",
                          runtime_receipt.inspect_scope,
                          "READY");
                check_str("runtime receipt log",
                          runtime_receipt.log_first_line,
                          "T0: THERON LOADED");
                check_int("runtime receipt logs receipt",
                          runtime_receipt.log_receipt,
                          1);
                check_int("runtime state receipt rc",
                          theron_v1_startup_runtime_entry_state_receipt_from_result(
                              &runtime_flow,
                              &runtime_result,
                              &state_receipt),
                          1);
                check_int("runtime state receipt flow",
                          state_receipt.flow_changed,
                          1);
                check_int("runtime state receipt phase",
                          state_receipt.flow.phase,
                          THERON_STARTUP_PHASE_IN_DUNGEON);
                check_int("runtime state receipt level",
                          state_receipt.level_loaded,
                          1);
                check_int("runtime state receipt tick",
                          state_receipt.tick_count,
                          99);
            }

            theron_v1_startup_action_init(&action);
            check_int("plan ignore rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan ignore kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_IGNORE);

            {
                Theron_StartupExecution execution;
                Theron_StartupApplyReceipt receipt;
                Theron_StartupStateReceipt state_receipt;
                Theron_StartupFlow exec_flow;

                theron_v1_startup_action_init(&action);
                action.kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
                action.selected_dungeon =
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
                action.cursor = 1;
                check_int("exec stage-select plan rc",
                          theron_v1_startup_plan_for_action(&action, &plan),
                          1);
                theron_v1_startup_flow_init(&exec_flow);
                check_int("exec stage-select rc",
                          theron_v1_startup_execute_flow_plan(
                              &plan,
                              &progression,
                              &exec_flow,
                              &execution),
                          1);
                check_int("exec stage-select result",
                          execution.result,
                          THERON_STARTUP_OK);
                check_int("exec stage-select phase",
                          exec_flow.phase,
                          THERON_STARTUP_PHASE_STAGE_SELECT);
                check_int("exec stage-select dungeon",
                          exec_flow.selected_dungeon,
                          THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
                check_int("exec stage-select cursor",
                          execution.cursor,
                          1);
                check_int("exec stage-select receipt rc",
                          theron_v1_startup_apply_receipt_from_flow_execution(
                              &plan,
                              &execution,
                              &receipt),
                          1);
                check_int("exec stage-select receipt redraw",
                          receipt.input_result,
                          THERON_STARTUP_INPUT_RESULT_REDRAW);
                check_int("exec stage-select receipt flow",
                          receipt.flow_changed,
                          1);
                check_int("exec stage-select state receipt rc",
                          theron_v1_startup_state_receipt_from_flow_apply(
                              &exec_flow,
                              &receipt,
                              &state_receipt),
                          1);
                check_int("exec stage-select state receipt flow",
                          state_receipt.flow_changed,
                          1);
                check_int("exec stage-select state receipt phase",
                          state_receipt.flow.phase,
                          THERON_STARTUP_PHASE_STAGE_SELECT);
                check_int("exec stage-select state receipt cursor set",
                          state_receipt.set_startup_cursor,
                          1);
                check_int("exec stage-select state receipt cursor",
                          state_receipt.startup_cursor,
                          1);
                check_str("exec stage-select receipt status",
                          receipt.status,
                          "STAGE SELECT");
                theron_v1_startup_flow_init(&exec_flow);
                check_int("exec stage-select combined rc",
                          theron_v1_startup_execute_flow_plan_with_receipts(
                              &plan,
                              &progression,
                              &exec_flow,
                              &execution,
                              &receipt,
                              &state_receipt),
                          1);
                check_int("exec stage-select combined result",
                          execution.result,
                          THERON_STARTUP_OK);
                check_int("exec stage-select combined redraw",
                          receipt.input_result,
                          THERON_STARTUP_INPUT_RESULT_REDRAW);
                check_int("exec stage-select combined state flow",
                          state_receipt.flow_changed,
                          1);
                check_int("exec stage-select combined state phase",
                          state_receipt.flow.phase,
                          THERON_STARTUP_PHASE_STAGE_SELECT);
                check_int("exec stage-select combined cursor",
                          state_receipt.startup_cursor,
                          1);

                theron_v1_startup_action_init(&action);
                action.kind = THERON_STARTUP_ACTION_CHOOSE_STAGE;
                action.selected_dungeon =
                    THERON_DUNGEON_1_HALL_OF_RECORDS;
                check_int("exec choose-stage plan rc",
                          theron_v1_startup_plan_for_action(&action, &plan),
                          1);
                check_int("exec choose-stage rc",
                          theron_v1_startup_execute_flow_plan(
                              &plan,
                              &progression,
                              &exec_flow,
                              &execution),
                          1);
                check_int("exec choose-stage result",
                          execution.result,
                          THERON_STARTUP_OK);
                check_int("exec choose-stage phase",
                          exec_flow.phase,
                          THERON_STARTUP_PHASE_SOUL_ROOM);
                check_int("exec choose-stage cursor reset",
                          execution.cursor,
                          0);

                theron_v1_startup_action_init(&action);
                action.kind = THERON_STARTUP_ACTION_TOGGLE_MIRROR;
                action.mirror_index = 2;
                check_int("exec mirror plan rc",
                          theron_v1_startup_plan_for_action(&action, &plan),
                          1);
                check_int("exec mirror select rc",
                          theron_v1_startup_execute_flow_plan(
                              &plan,
                              &progression,
                              &exec_flow,
                              &execution),
                          1);
                check_int("exec mirror select result",
                          execution.result,
                          THERON_STARTUP_OK);
                check_int("exec mirror selected flag",
                          execution.mirror_selected,
                          1);
                check_int("exec mirror receipt rc",
                          theron_v1_startup_apply_receipt_from_flow_execution(
                              &plan,
                              &execution,
                              &receipt),
                          1);
                check_str("exec mirror receipt selected",
                          receipt.status,
                          "HERO RESURRECTED");
                check_int("exec mirror count",
                          exec_flow.companion_count,
                          1);
                check_int("exec mirror deselect rc",
                          theron_v1_startup_execute_flow_plan(
                              &plan,
                              &progression,
                              &exec_flow,
                              &execution),
                          1);
                check_int("exec mirror deselect result",
                          execution.result,
                          THERON_STARTUP_OK);
                check_int("exec mirror deselected flag",
                          execution.mirror_selected,
                          0);
                check_int("exec mirror deselect receipt rc",
                          theron_v1_startup_apply_receipt_from_flow_execution(
                              &plan,
                              &execution,
                              &receipt),
                          1);
                check_str("exec mirror receipt deselected",
                          receipt.status,
                          "HERO RELEASED");
                check_int("exec mirror count after deselect",
                          exec_flow.companion_count,
                          0);
            }
            {
                Theron_V1StartupContinueRequest continue_request;
                Theron_V1StartupContinueResult continue_result;
                char continue_receipt[128];

                theron_v1_startup_continue_request_init(&continue_request);
                theron_v1_startup_continue_result_init(&continue_result);
                check_int("continue request init claim",
                          continue_request.resume_claim,
                          THERON_V1_STARTUP_RESUME_NONE);
                check_int("continue request init tqsv slot",
                          continue_request.tqsv_slot_index,
                          -1);
                check_int("continue result init source",
                          continue_result.source,
                          THERON_V1_STARTUP_CONTINUE_SOURCE_NONE);

                continue_receipt[0] = '\0';
                theron_v1_world_init(&world);
                check_int("continue apply no source rc",
                          theron_v1_startup_continue_apply_request(
                              &world,
                              &continue_request,
                              &continue_result,
                              continue_receipt,
                              sizeof(continue_receipt)),
                          0);
                check_contains("continue apply no source receipt",
                               continue_receipt,
                               "Continue requires");
                check_int("continue apply no source result",
                          continue_result.source,
                          THERON_V1_STARTUP_CONTINUE_SOURCE_NONE);

                continue_request.resume_claim = THERON_V1_STARTUP_RESUME_TQSV;
                continue_request.tqsv_slot_index = THERON_SAVE_SLOT_COUNT;
                continue_receipt[0] = '\0';
                check_int("continue apply bad tqsv slot rc",
                          theron_v1_startup_continue_apply_request(
                              &world,
                              &continue_request,
                              &continue_result,
                              continue_receipt,
                              sizeof(continue_receipt)),
                          0);
                check_contains("continue apply bad tqsv slot receipt",
                               continue_receipt,
                               "Continue requires");
            }
        }

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_TITLE;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_TITLE,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            &hit,
            &action);
        check_int("title hit rc", result, THERON_STARTUP_OK);
        check_int("title hit shows stage select",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_PANEL;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            1,
            &hit,
            &action);
        check_int("panel hit rc", result, THERON_STARTUP_OK);
        check_int("panel hit consumes without action",
                  action.kind,
                  THERON_STARTUP_ACTION_NONE);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_CONTINUE;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            1,
            &hit,
            &action);
        check_int("continue hit rc", result, THERON_STARTUP_OK);
        check_int("continue hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_CONTINUE_SAVE);
        check_int("continue hit focus", action.continue_focus, 1);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_STAGE;
        hit.selected_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            1,
            1,
            &hit,
            &action);
        check_int("stage hit rc", result, THERON_STARTUP_OK);
        check_int("stage hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_CHOOSE_STAGE);
        check_int("stage hit selected dungeon",
                  action.selected_dungeon,
                  THERON_DUNGEON_3_ABYSS_OF_FLAMES);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_STAGE;
        hit.selected_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
        result = theron_v1_startup_handle_hit_with_progression(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            &progression,
            0,
            0,
            0,
            &hit,
            &action);
        check_int("locked stage hit rc", result, THERON_STARTUP_OK);
        check_int("locked stage hit consumed",
                  action.kind,
                  THERON_STARTUP_ACTION_NONE);
        check_int("locked stage hit keeps target for status",
                  action.selected_dungeon,
                  THERON_DUNGEON_3_ABYSS_OF_FLAMES);
        {
            Theron_DungeonProgression progressed_hit;
            theron_v1_dungeon_progression_init(&progressed_hit);
            (void)theron_v1_dungeon_advance(&progressed_hit);
            result = theron_v1_startup_handle_hit_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_1_HALL_OF_RECORDS,
                &progressed_hit,
                0,
                0,
                0,
                &hit,
                &action);
            check_int("unlocked stage hit rc", result, THERON_STARTUP_OK);
            check_int("unlocked stage hit chooses",
                      action.kind,
                      THERON_STARTUP_ACTION_CHOOSE_STAGE);
            check_int("unlocked stage hit selected dungeon",
                      action.selected_dungeon,
                      THERON_DUNGEON_3_ABYSS_OF_FLAMES);
        }

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_MIRROR;
        hit.mirror_index = 4;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_SOUL_ROOM,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            &hit,
            &action);
        check_int("mirror hit rc", result, THERON_STARTUP_OK);
        check_int("mirror hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_TOGGLE_MIRROR);
        check_int("mirror hit index", action.mirror_index, 4);
        check_int("mirror hit cursor", action.cursor, 4);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_FORCEFIELD;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            2,
            0,
            0,
            &hit,
            &action);
        check_int("forcefield hit rc", result, THERON_STARTUP_OK);
        check_int("forcefield hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_ENTER_FORCEFIELD);
        check_int("forcefield hit cursor",
                  action.cursor,
                  THERON_STARTUP_HERO_MIRROR_COUNT);

        {
            Theron_StartupLayoutState layout_state;
            Theron_StartupLayoutElement elements[16];
            Theron_StartupRenderPlan render_plan;
            char rows[16][THERON_STARTUP_RENDER_ROW_CAPACITY];
            int order[THERON_STARTUP_MAX_COMPANIONS] = {6, 2, -1};
            int layout_count;
            int row_count;

            theron_v1_startup_layout_state_init(&layout_state);
            layout_state.phase = THERON_STARTUP_PHASE_STAGE_SELECT;
            layout_state.selected_dungeon =
                THERON_DUNGEON_1_HALL_OF_RECORDS;
            layout_state.progression = &progression;
            layout_state.has_tqsv_continue = 1;
            layout_state.tqsv_slot = 2;
            snprintf(layout_state.chapter_label,
                     sizeof(layout_state.chapter_label),
                     "Chapter 1: Hall of Records");
            layout_count = theron_v1_startup_layout_build(
                &layout_state,
                elements,
                (int)(sizeof(elements) / sizeof(elements[0])));
            check_int("layout stage count", layout_count, 10);
            check_int("layout title kind",
                      elements[0].kind,
                      THERON_STARTUP_LAYOUT_ELEMENT_TITLE);
            check_int("layout continue kind",
                      elements[2].kind,
                      THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE);
            check_int("layout continue slot", elements[2].save_slot, 2);
            check_int("layout stage1 kind",
                      elements[3].kind,
                      THERON_STARTUP_LAYOUT_ELEMENT_STAGE);
            check_int("layout stage1 enabled", elements[3].enabled, 1);
            check_int("layout stage2 locked", elements[4].enabled, 0);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                elements,
                layout_count,
                42,
                66,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout continue hit rc", result, THERON_STARTUP_OK);
            check_int("layout continue hit kind",
                      hit.kind,
                      THERON_STARTUP_HIT_CONTINUE);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                elements,
                layout_count,
                42,
                91,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout locked stage hit rc", result, THERON_STARTUP_OK);
            check_int("layout locked stage hit panel",
                      hit.kind,
                      THERON_STARTUP_HIT_PANEL);
            row_count = theron_v1_startup_render_rows_build(
                &layout_state,
                elements,
                layout_count,
                rows,
                (int)(sizeof(rows) / sizeof(rows[0])));
            check_int("layout stage rows count", row_count, 12);
            check_contains("layout stage rows chapter",
                           rows[1],
                           "Chapter 1: Hall of Records");
            check_contains("layout stage rows continue",
                           rows[3],
                           "CONTINUE  TQSV SLOT 2");
            check_contains("layout stage rows cursor",
                           rows[4],
                           "> 1  Hall of Records");
            check_int("layout stage render plan rc",
                      theron_v1_startup_render_plan_build(
                          &layout_state,
                          elements,
                          layout_count,
                          &render_plan),
                      1);
            check_int("layout stage render plan phase",
                      render_plan.phase,
                      THERON_STARTUP_PHASE_STAGE_SELECT);
            check_int("layout stage render plan border x",
                      render_plan.border_x,
                      12);
            check_int("layout stage render plan border color",
                      render_plan.border_color,
                      11);
            check_int("layout stage render plan text count",
                      render_plan.text_count,
                      12);
            check_str("layout stage render plan title",
                      render_plan.text[0].text,
                      "THERON'S QUEST");
            check_int("layout stage render plan title style",
                      render_plan.text[0].style,
                      THERON_STARTUP_RENDER_TEXT_TITLE);
            check_str("layout stage render plan prompt",
                      render_plan.text[2].text,
                      "CHOOSE A STAGE");
            check_int("layout stage render plan continue y",
                      render_plan.text[3].y,
                      66);
            check_contains("layout stage render plan continue",
                           render_plan.text[3].text,
                           "CONTINUE  TQSV SLOT 2");
            check_int("layout stage render plan selected style",
                      render_plan.text[4].style,
                      THERON_STARTUP_RENDER_TEXT_ACTIVE);
            check_int("layout stage render plan locked style",
                      render_plan.text[5].style,
                      THERON_STARTUP_RENDER_TEXT_LOCKED);
            check_contains("layout stage render plan footer",
                           render_plan.text[11].text,
                           "ENTER CONTINUE/STAGE");

            theron_v1_startup_layout_state_init(&layout_state);
            layout_state.phase = THERON_STARTUP_PHASE_READY;
            layout_state.selected_dungeon =
                THERON_DUNGEON_1_HALL_OF_RECORDS;
            layout_state.soul_cursor = THERON_STARTUP_HERO_MIRROR_COUNT;
            snprintf(layout_state.chapter_label,
                     sizeof(layout_state.chapter_label),
                     "Chapter 1: Hall of Records");
            snprintf(layout_state.startup_text_prompt,
                     sizeof(layout_state.startup_text_prompt),
                     "GO AWAY AND RESURRECT THERON");
            layout_state.startup_roster_name_count = 8;
            layout_state.startup_roster_names[4] = "HAKAR";
            layout_state.startup_roster_names[5] = "TIRAN";
            layout_state.startup_roster_names[7] = "PENTAI";
            layout_state.startup_roster_titles[4] = "THE BRAVE";
            layout_state.startup_roster_titles[7] = "THE SURVIVOR";
            layout_state.selected_mirrors_mask = (1 << 6) | (1 << 2);
            layout_state.selected_mirror_order = order;
            layout_state.selected_mirror_order_count =
                THERON_STARTUP_MAX_COMPANIONS;
            layout_count = theron_v1_startup_layout_build(
                &layout_state,
                elements,
                (int)(sizeof(elements) / sizeof(elements[0])));
            check_int("layout ready count", layout_count, 10);
            check_int("layout mirror6 order",
                      elements[8].selected_order,
                      1);
            check_int("layout forcefield enabled",
                      elements[9].enabled,
                      1);
            check_int("layout forcefield cursor",
                      elements[9].cursor,
                      1);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_READY,
                elements,
                layout_count,
                48,
                144,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout mirror hit rc", result, THERON_STARTUP_OK);
            check_int("layout mirror hit kind",
                      hit.kind,
                      THERON_STARTUP_HIT_MIRROR);
            check_int("layout mirror hit index", hit.mirror_index, 6);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_READY,
                elements,
                layout_count,
                48,
                162,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout forcefield hit rc", result, THERON_STARTUP_OK);
            check_int("layout forcefield hit kind",
                      hit.kind,
                      THERON_STARTUP_HIT_FORCEFIELD);
            row_count = theron_v1_startup_render_rows_build(
                &layout_state,
                elements,
                layout_count,
                rows,
                (int)(sizeof(rows) / sizeof(rows[0])));
            check_int("layout ready rows count", row_count, 13);
            check_contains("layout ready rows prompt",
                           rows[3],
                           "GO AWAY AND RESURRECT THERON");
            check_contains("layout ready rows decoded mirror",
                           rows[4],
                           "HAKAR");
            check_contains("layout ready rows selected order",
                           rows[10],
                           "RESURRECTED #1");
            check_int("layout ready render plan rc",
                      theron_v1_startup_render_plan_build(
                          &layout_state,
                          elements,
                          layout_count,
                          &render_plan),
                      1);
            check_int("layout ready render plan phase",
                      render_plan.phase,
                      THERON_STARTUP_PHASE_READY);
            check_int("layout ready render plan text count",
                      render_plan.text_count,
                      13);
            check_str("layout ready render plan title",
                      render_plan.text[0].text,
                      "THERON'S QUEST");
            check_str("layout ready render plan room",
                      render_plan.text[2].text,
                      "SOUL ROOM");
            check_contains("layout ready render plan prompt",
                           render_plan.text[3].text,
                           "GO AWAY AND RESURRECT THERON");
            check_contains("layout ready render plan decoded mirror",
                           render_plan.text[4].text,
                           "HAKAR");
            check_int("layout ready render plan selected style",
                      render_plan.text[10].style,
                      THERON_STARTUP_RENDER_TEXT_PICKED);
            check_int("layout ready render plan forcefield style",
                      render_plan.text[11].style,
                      THERON_STARTUP_RENDER_TEXT_ACTIVE);
            check_contains("layout ready render plan footer",
                           render_plan.text[12].text,
                           "ACTION ENTERS");
        }
    }

    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
    check_int("locked stage rejected", result, THERON_STARTUP_ERR_STAGE_LOCKED);

    theron_v1_dungeon_advance(&progression);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_5_VAULT_OF_SECRETS);
    check_int("middle stage accepted after first completion", result, THERON_STARTUP_OK);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_7_TOWER_OF_EPILOGUE);
    check_int("final stage still locked after first completion",
              result,
              THERON_STARTUP_ERR_STAGE_LOCKED);

    theron_v1_dungeon_progression_init(&progression);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage 1 accepted", result, THERON_STARTUP_OK);
    check_int("stage phase soul room", flow.phase, THERON_STARTUP_PHASE_SOUL_ROOM);
    check_int("stage theron present", flow.theron_present, 1);

    result = theron_v1_startup_select_mirror(&flow, 6);
    check_int("mirror 6 accepted first", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 6);
    check_int("duplicate mirror rejected", result, THERON_STARTUP_ERR_DUPLICATE_MIRROR);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror 0 accepted second", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 2);
    check_int("mirror 2 accepted third", result, THERON_STARTUP_OK);
    result = theron_v1_startup_deselect_mirror(&flow, 0);
    check_int("mirror 0 deselected", result, THERON_STARTUP_OK);
    check_int("companion count after middle deselect", flow.companion_count, 2);
    check_int("first order after middle deselect", flow.selected_mirror_order[0], 6);
    check_int("second order after middle deselect", flow.selected_mirror_order[1], 2);
    check_int("third order cleared after middle deselect",
              flow.selected_mirror_order[2],
              0xff);
    result = theron_v1_startup_deselect_mirror(&flow, 0);
    check_int("unselected mirror deselect rejected",
              result,
              THERON_STARTUP_ERR_MIRROR_NOT_SELECTED);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror 0 reaccepted third", result, THERON_STARTUP_OK);
    check_int("three companions selected", flow.companion_count, 3);
    check_int("first selected mirror order", flow.selected_mirror_order[0], 6);
    check_int("second selected mirror order", flow.selected_mirror_order[1], 2);
    check_int("third selected mirror order", flow.selected_mirror_order[2], 0);
    check_int("ready phase", flow.phase, THERON_STARTUP_PHASE_READY);

    result = theron_v1_startup_select_mirror(&flow, 4);
    check_int("fourth companion rejected", result, THERON_STARTUP_ERR_PARTY_FULL);

    memset(&party, 0, sizeof(party));
    result = theron_v1_startup_enter_forcefield(&flow, &party);
    check_int("forcefield accepted", result, THERON_STARTUP_OK);
    check_int("forcefield flag", flow.forcefield_entered, 1);
    check_int("in dungeon phase", flow.phase, THERON_STARTUP_PHASE_IN_DUNGEON);
    check_int("party count theron plus three", party.champion_count, 4);
    check_int("leader slot is Theron", party.active_slot, THERON_CHAMPION_SLOT_THERON);
    check_contains("slot 0 name", party.champions[0].name, "Theron");
    check_contains("slot 1 mirror name", party.champions[1].name, "Pental");
    check_contains("slot 2 mirror name", party.champions[2].name, "Tiran");
    check_contains("slot 3 mirror name", party.champions[3].name, "Hakar");
    check_int("slot 1 portrait", party.champions[1].portrait_index, 7);
    check_int("slot 2 portrait", party.champions[2].portrait_index, 3);
    check_int("slot 3 portrait", party.champions[3].portrait_index, 1);
    check_int("slot 1 class", party.champions[1].primary_class, THERON_CLASS_FIGHTER);
    check_int("slot 2 class", party.champions[2].primary_class, THERON_CLASS_FIGHTER);
    check_int("slot 3 class", party.champions[3].primary_class, THERON_CLASS_FIGHTER);

    {
        const char *roster_names[8] = {
            "THERON-JP",
            "MARA-JP",
            "LINOS-JP",
            "HEXA-JP",
            "HAKAR-JP",
            "TIRAN-JP",
            "DOTAN-JP",
            "PENTAI-JP"
        };
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("roster forcefield choose rc", result, THERON_STARTUP_OK);
        check_int("roster forcefield mirror 6 rc",
                  theron_v1_startup_select_mirror(&flow, 6),
                  THERON_STARTUP_OK);
        check_int("roster forcefield mirror 2 rc",
                  theron_v1_startup_select_mirror(&flow, 2),
                  THERON_STARTUP_OK);
        memset(&party, 0, sizeof(party));
        snprintf(party.champions[THERON_CHAMPION_SLOT_THERON].name,
                 sizeof(party.champions[THERON_CHAMPION_SLOT_THERON].name),
                 "THERON SAVED");
        result = theron_v1_startup_enter_forcefield_with_roster(
            &flow,
            &party,
            roster_names,
            8);
        check_int("roster forcefield rc", result, THERON_STARTUP_OK);
        check_contains("roster preserves Theron",
                       party.champions[0].name,
                       "THERON SAVED");
        check_contains("roster applies mirror 6 raw name",
                       party.champions[1].name,
                       "PENTAI-JP");
        check_contains("roster applies mirror 2 raw name",
                       party.champions[2].name,
                       "TIRAN-JP");
        theron_v1_world_init(&world);
        world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0] = 1;
        world.object_count = 7;
        world.timer_count = 3;
        result = theron_v1_startup_enter_world_from_forcefield(
            &flow,
            &world);
        check_int("world entry after forcefield rc",
                  result,
                  THERON_STARTUP_OK);
        check_int("world entry current dungeon",
                  world.current_dungeon,
                  THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("world entry current level", world.current_level, 0);
        check_int("world entry progression level",
                  world.progression.current_level,
                  1);
        check_int("world entry clears level-loaded row",
                  world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
                  0);
        check_int("world entry clears objects", world.object_count, 0);
        check_int("world entry clears timers", world.timer_count, 0);
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_enter_world_from_forcefield(
            &flow,
            &world);
        check_int("world entry before forcefield rejected",
                  result,
                  THERON_STARTUP_ERR_NOT_READY);
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("runtime entry choose rc", result, THERON_STARTUP_OK);
        memset(&party, 0, sizeof(party));
        result = theron_v1_startup_enter_forcefield(&flow, &party);
        check_int("runtime entry forcefield rc", result, THERON_STARTUP_OK);
        theron_v1_world_init(&world);
        world.party = party;
        {
            int called = 0;
            char runtime_receipt[192];
            runtime_receipt[0] = '\0';
            result = theron_v1_startup_enter_runtime_from_forcefield(
                &flow,
                &world,
                fake_runtime_level_load,
                &called,
                runtime_receipt,
                sizeof(runtime_receipt));
            check_int("runtime entry rc", result, THERON_STARTUP_OK);
            check_int("runtime entry callback called", called, 1);
            check_int("runtime entry level loaded",
                      world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
                      1);
            check_int("runtime entry party x", world.party.leader_x, 5);
            check_contains("runtime entry receipt",
                           runtime_receipt,
                           "fake level stage=1");
        }
        {
            char load_receipt[192];
            load_receipt[0] = '\0';
            theron_v1_world_init(&world);
            result = theron_v1_startup_runtime_load_initial_level(
                &world,
                NULL,
                0u,
                NULL,
                THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                load_receipt,
                sizeof(load_receipt));
            check_int("runtime loader fallback rc", result, 1);
            check_int("runtime loader fallback dungeon",
                      world.current_dungeon,
                      THERON_DUNGEON_3_ABYSS_OF_FLAMES);
            check_int("runtime loader fallback level", world.current_level, 0);
            check_int("runtime loader fallback loaded",
                      world.level_loaded[THERON_DUNGEON_3_ABYSS_OF_FLAMES - 1][0],
                      1);
            check_int("runtime loader fallback party x",
                      world.party.leader_x,
                      world.levels[THERON_DUNGEON_3_ABYSS_OF_FLAMES - 1][0].start_x);
            check_contains("runtime loader fallback receipt",
                           load_receipt,
                           "fallback room stage=3");
        }
        {
            Theron_V1StartupRuntimeEntryRequest runtime_request;
            Theron_V1StartupRuntimeEntryResult runtime_result;
            const char *runtime_roster[8] = {
                "THERON",
                "MARA-JP",
                "LINOS-JP",
                "HEXA-JP",
                "HAKAR-JP",
                "TIRAN-JP",
                "DOTAN-JP",
                "PENTAI-JP"
            };
            char runtime_receipt[256];

            theron_v1_startup_runtime_entry_request_init(&runtime_request);
            theron_v1_startup_runtime_entry_result_init(&runtime_result);
            check_int("runtime wrapper init result",
                      runtime_result.result,
                      THERON_STARTUP_OK);

            theron_v1_startup_flow_init(&flow);
            result = theron_v1_startup_runtime_enter_from_forcefield(
                &flow,
                &world,
                &runtime_request,
                &runtime_result,
                runtime_receipt,
                sizeof(runtime_receipt));
            check_int("runtime wrapper not-ready rc", result, 0);
            check_int("runtime wrapper not-ready result",
                      runtime_result.result,
                      THERON_STARTUP_ERR_NO_STAGE);

            theron_v1_startup_flow_init(&flow);
            theron_v1_world_init(&world);
            result = theron_v1_startup_choose_stage(
                &flow,
                &progression,
                THERON_DUNGEON_1_HALL_OF_RECORDS);
            check_int("runtime wrapper choose rc",
                      result,
                      THERON_STARTUP_OK);
            result = theron_v1_startup_select_mirror(&flow, 0);
            check_int("runtime wrapper select mirror rc",
                      result,
                      THERON_STARTUP_OK);
            runtime_request.roster_names = runtime_roster;
            runtime_request.roster_name_count = 8;
            runtime_receipt[0] = '\0';
            check_int("runtime wrapper enter rc",
                      theron_v1_startup_runtime_enter_from_forcefield(
                          &flow,
                          &world,
                          &runtime_request,
                          &runtime_result,
                          runtime_receipt,
                          sizeof(runtime_receipt)),
                      1);
            check_int("runtime wrapper result",
                      runtime_result.result,
                      THERON_STARTUP_OK);
            check_int("runtime wrapper flow phase",
                      flow.phase,
                      THERON_STARTUP_PHASE_IN_DUNGEON);
            check_int("runtime wrapper level loaded",
                      runtime_result.level_loaded,
                      1);
            check_int("runtime wrapper world loaded",
                      world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
                      1);
            check_str("runtime wrapper roster name",
                      world.party.champions[1].name,
                      "HAKAR-JP");
            check_contains("runtime wrapper receipt",
                           runtime_receipt,
                           "fallback room stage=1");
            {
                Theron_StartupAction forcefield_action;
                Theron_StartupActionPlan forcefield_plan;
                Theron_V1StartupRuntimeEntryApplyReceipt runtime_apply_receipt;
                Theron_StartupStateReceipt state_receipt;
                char fixed_roster[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                                 [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY] = {
                    "THERON",
                    "MARA-FX",
                    "LINOS-FX",
                    "HEXA-FX",
                    "HAKAR-FX",
                    "TIRAN-FX",
                    "DOTAN-FX",
                    "PENTAI-FX"
                };
                check_int("runtime wrapper state receipt rc",
                          theron_v1_startup_runtime_entry_state_receipt_from_result(
                              &flow,
                              &runtime_result,
                              &state_receipt),
                          1);
                check_int("runtime wrapper state receipt phase",
                          state_receipt.flow.phase,
                          THERON_STARTUP_PHASE_IN_DUNGEON);
                check_int("runtime wrapper state receipt dungeon",
                          state_receipt.flow.selected_dungeon,
                          THERON_DUNGEON_1_HALL_OF_RECORDS);
                check_int("runtime wrapper state receipt party x",
                          state_receipt.party_x,
                          runtime_result.party_x);
                check_int("runtime wrapper state receipt level loaded",
                          state_receipt.level_loaded,
                          1);

                theron_v1_startup_action_init(&forcefield_action);
                forcefield_action.kind =
                    THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
                check_int("runtime combined plan rc",
                          theron_v1_startup_plan_for_action(
                              &forcefield_action,
                              &forcefield_plan),
                          1);
                theron_v1_startup_flow_init(&flow);
                theron_v1_world_init(&world);
                runtime_receipt[0] = '\0';
                check_int("runtime combined choose rc",
                          theron_v1_startup_choose_stage(
                              &flow,
                              &progression,
                              THERON_DUNGEON_1_HALL_OF_RECORDS),
                          THERON_STARTUP_OK);
                check_int("runtime combined mirror rc",
                          theron_v1_startup_select_mirror(&flow, 0),
                          THERON_STARTUP_OK);
                check_int("runtime combined enter rc",
                          theron_v1_startup_runtime_enter_from_forcefield_with_receipts(
                              &flow,
                              &world,
                              &runtime_request,
                              &forcefield_plan,
                              &runtime_result,
                              &runtime_apply_receipt,
                              &state_receipt,
                              runtime_receipt,
                              sizeof(runtime_receipt)),
                          1);
                check_int("runtime combined apply redraw",
                          runtime_apply_receipt.input_result,
                          THERON_STARTUP_INPUT_RESULT_REDRAW);
                check_int("runtime combined state phase",
                          state_receipt.flow.phase,
                          THERON_STARTUP_PHASE_IN_DUNGEON);
                check_int("runtime combined state loaded",
                          state_receipt.level_loaded,
                          1);

                theron_v1_startup_flow_init(&flow);
                theron_v1_world_init(&world);
                runtime_receipt[0] = '\0';
                check_int("runtime facts choose rc",
                          theron_v1_startup_choose_stage(
                              &flow,
                              &progression,
                              THERON_DUNGEON_1_HALL_OF_RECORDS),
                          THERON_STARTUP_OK);
                check_int("runtime facts mirror rc",
                          theron_v1_startup_select_mirror(&flow, 0),
                          THERON_STARTUP_OK);
                check_int("runtime facts enter rc",
                          theron_v1_startup_runtime_enter_from_forcefield_facts_with_receipts(
                              &flow,
                              &world,
                              NULL,
                              0u,
                              NULL,
                              fixed_roster,
                              (int)THERON_STARTUP_MEDIA_ROSTER_CAPACITY,
                              &forcefield_plan,
                              &runtime_result,
                              &runtime_apply_receipt,
                              &state_receipt,
                              runtime_receipt,
                              sizeof(runtime_receipt)),
                          1);
                check_str("runtime facts roster name",
                          world.party.champions[1].name,
                          "HAKAR-FX");
                check_int("runtime facts state phase",
                          state_receipt.flow.phase,
                          THERON_STARTUP_PHASE_IN_DUNGEON);
                check_int("runtime facts apply redraw",
                          runtime_apply_receipt.input_result,
                          THERON_STARTUP_INPUT_RESULT_REDRAW);
            }
        }
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("runtime fail choose rc", result, THERON_STARTUP_OK);
        memset(&party, 0, sizeof(party));
        result = theron_v1_startup_enter_forcefield(&flow, &party);
        check_int("runtime fail forcefield rc", result, THERON_STARTUP_OK);
        theron_v1_world_init(&world);
        world.party = party;
        {
            int called = 0;
            char runtime_receipt[192];
            runtime_receipt[0] = '\0';
            result = theron_v1_startup_enter_runtime_from_forcefield(
                &flow,
                &world,
                fake_runtime_level_load_fail,
                &called,
                runtime_receipt,
                sizeof(runtime_receipt));
            check_int("runtime entry load failure rc",
                      result,
                      THERON_STARTUP_ERR_LEVEL_LOAD);
            check_int("runtime entry failing callback called", called, 1);
            check_contains("runtime entry load failure receipt",
                           runtime_receipt,
                           "level load failed");
        }
        {
            char exit_receipt[128];
            theron_v1_world_init(&world);
            world.progression.dungeon_states[THERON_DUNGEON_1_HALL_OF_RECORDS - 1] =
                THERON_DUNGEON_STATE_COMPLETE;
            world.party.champion_count = 4;
            world.party.active_slot = 2;
            world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0] = 1;
            world.dungeon_complete = 1;
            world.quest_items_in_dungeon = 1;
            theron_v1_startup_flow_init(&flow);
            exit_receipt[0] = '\0';
            result = theron_v1_startup_return_to_stage_select_after_exit(
                &world,
                &flow,
                exit_receipt,
                sizeof(exit_receipt));
            check_int("exit return rc", result, THERON_STARTUP_OK);
            check_int("exit return current dungeon",
                      world.progression.current_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
            check_int("exit return phase",
                      flow.phase,
                      THERON_STARTUP_PHASE_STAGE_SELECT);
            check_int("exit return selected dungeon",
                      flow.selected_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
            check_int("exit return party count", world.party.champion_count, 1);
            check_int("exit return active slot",
                      world.party.active_slot,
                      THERON_CHAMPION_SLOT_THERON);
            check_int("exit return clears level",
                      world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
                      0);
            check_int("exit return clears dungeon flag",
                      world.dungeon_complete,
                      0);
            check_contains("exit return receipt",
                           exit_receipt,
                           "next stage=2");
        }
        {
            char exit_receipt[128];
            theron_v1_world_init(&world);
            theron_v1_startup_flow_init(&flow);
            exit_receipt[0] = '\0';
            result = theron_v1_startup_return_to_stage_select_after_exit(
                &world,
                &flow,
                exit_receipt,
                sizeof(exit_receipt));
            check_int("exit return rejects incomplete rc",
                      result,
                      THERON_STARTUP_ERR_DUNGEON_ENTRY);
            check_contains("exit return rejects incomplete receipt",
                           exit_receipt,
                           "dungeon exit rejected");
        }
    }

    check_contains("mirror 0 meta", theron_v1_startup_mirror_meta(0)->name, "Hakar");
    check_contains("mirror 1 meta", theron_v1_startup_mirror_meta(1)->name, "Mara");
    check_contains("mirror 3 meta", theron_v1_startup_mirror_meta(3)->name, "Linos");
    check_contains("mirror 6 meta", theron_v1_startup_mirror_meta(6)->name, "Pental");
    check_int("mirror 0 roster index", theron_v1_startup_roster_index_for_mirror(0), 4);
    check_int("mirror 1 roster index", theron_v1_startup_roster_index_for_mirror(1), 1);
    check_int("mirror 2 roster index", theron_v1_startup_roster_index_for_mirror(2), 5);
    check_int("mirror 3 roster index", theron_v1_startup_roster_index_for_mirror(3), 2);
    check_int("mirror 4 roster index", theron_v1_startup_roster_index_for_mirror(4), 6);
    check_int("mirror 5 roster index", theron_v1_startup_roster_index_for_mirror(5), 3);
    check_int("mirror 6 roster index", theron_v1_startup_roster_index_for_mirror(6), 7);
    check_int("bad mirror roster index", theron_v1_startup_roster_index_for_mirror(99), -1);
    check_int("roster index 4 mirror", theron_v1_startup_mirror_index_for_roster(4), 0);
    check_int("roster index 1 mirror", theron_v1_startup_mirror_index_for_roster(1), 1);
    check_int("roster index 5 mirror", theron_v1_startup_mirror_index_for_roster(5), 2);
    check_int("roster index 2 mirror", theron_v1_startup_mirror_index_for_roster(2), 3);
    check_int("roster index 6 mirror", theron_v1_startup_mirror_index_for_roster(6), 4);
    check_int("roster index 3 mirror", theron_v1_startup_mirror_index_for_roster(3), 5);
    check_int("roster index 7 mirror", theron_v1_startup_mirror_index_for_roster(7), 6);
    check_int("Theron roster index has no mirror", theron_v1_startup_mirror_index_for_roster(0), -1);
    check_contains("class label", theron_v1_startup_class_name(THERON_CLASS_PRIEST), "PRIEST");

    theron_v1_startup_flow_init(&flow);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage accepted for Theron-only run", result, THERON_STARTUP_OK);
    memset(&party, 0, sizeof(party));
    result = theron_v1_startup_enter_forcefield(&flow, &party);
    check_int("Theron-only forcefield accepted", result, THERON_STARTUP_OK);
    check_int("Theron-only party count", party.champion_count, 1);

    check_contains("title phase name",
                   theron_v1_startup_phase_name(THERON_STARTUP_PHASE_TITLE),
                   "title");
    check_contains("phase name",
                   theron_v1_startup_phase_name(THERON_STARTUP_PHASE_SOUL_ROOM),
                   "soul");
    check_contains("result name", theron_v1_startup_result_name(THERON_STARTUP_ERR_PARTY_FULL), "party");
    check_contains("deselect result name",
                   theron_v1_startup_result_name(THERON_STARTUP_ERR_MIRROR_NOT_SELECTED),
                   "mirror");
    check_contains("source evidence", theron_v1_startup_flow_source_evidence(), "dmweb");
    check_int("receipt title rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_TITLE,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt title phase", phase_label, "theron-startup-0");
    check_int("receipt title active", startup_active, 1);
    check_int("receipt stage rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_STAGE_SELECT,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt stage phase", phase_label, "theron-startup-1");
    check_int("receipt stage active", startup_active, 1);
    check_int("receipt soul rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_SOUL_ROOM,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt soul phase", phase_label, "theron-startup-2");
    check_int("receipt soul active", startup_active, 1);
    check_int("receipt ready rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_READY,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt ready phase", phase_label, "theron-startup-3");
    check_int("receipt ready active", startup_active, 1);
    check_int("receipt runtime rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_IN_DUNGEON,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt runtime phase", phase_label, "theron-runtime");
    check_int("receipt runtime active", startup_active, 0);

    printf("# total=%d passed=%d failed=%d\n", g_pass + g_fail, g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
