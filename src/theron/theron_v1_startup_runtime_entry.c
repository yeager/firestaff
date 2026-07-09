#include "theron_v1_startup_runtime_entry.h"

#include "theron_v1_boot.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const uint8_t *hucard_rom;
    size_t hucard_rom_size;
    const char *md5_hex;
} Theron_V1StartupRuntimeLevelLoadContext;

static int theron_v1_startup_runtime_level_load_callback(
    Theron_V1_World *world,
    Theron_DungeonID dungeon_id,
    void *userdata,
    char *receipt,
    size_t receipt_cap) {

    const Theron_V1StartupRuntimeLevelLoadContext *ctx =
        (const Theron_V1StartupRuntimeLevelLoadContext *)userdata;

    if (!ctx) {
        return 0;
    }
    return theron_v1_startup_runtime_load_initial_level(world,
                                                        ctx->hucard_rom,
                                                        ctx->hucard_rom_size,
                                                        ctx->md5_hex,
                                                        dungeon_id,
                                                        receipt,
                                                        receipt_cap);
}

static int theron_v1_startup_runtime_try_track02_initial_level(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    char *receipt,
    size_t receipt_cap) {
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    Theron_Track02UserDataWindowCatalog user_window_catalog;
    Theron_Track02StartupTextMarkerCatalog text_marker_catalog;
    Theron_Track02LevelHandoffStatus last_semantic_status =
        THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    Theron_Track02SemanticBindingStatus last_seed_status =
        THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    int last_user_data_offset_valid = 0;
    size_t last_user_data_offset = 0u;
    size_t user_window_descriptor_count = 0u;
    size_t user_window_span_count = 0u;
    size_t user_window_initial_count = 0u;
    size_t startup_text_us_count = 0u;
    size_t startup_text_jp_count = 0u;
    size_t anchor;
    int tried = 0;

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (!world || !hucard_rom || hucard_rom_size == 0u ||
        !md5_hex || md5_hex[0] == '\0') {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "no raw Track 02 bytes");
        }
        return 0;
    }
    if (dungeon_id != THERON_DUNGEON_1_HALL_OF_RECORDS) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt,
                     receipt_cap,
                     "Track 02 initial candidate pending for stage %d",
                     (int)dungeon_id);
        }
        return 0;
    }

    signal_status = theron_v1_track02_find_bank_signal(hucard_rom,
                                                       hucard_rom_size,
                                                       md5_hex,
                                                       &signal);
    if (signal_status != THERON_TRACK02_SIGNAL_OK) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt,
                     receipt_cap,
                     "Track 02 bank signal %s",
                     theron_v1_track02_signal_status_name(signal_status));
        }
        return 0;
    }

    memset(&user_window_catalog, 0, sizeof(user_window_catalog));
    if (theron_v1_track02_catalog_user_data_windows(hucard_rom,
                                                    hucard_rom_size,
                                                    md5_hex,
                                                    &user_window_catalog) ==
        THERON_TRACK02_SIGNAL_OK) {
        size_t i;
        for (i = 0u; i < user_window_catalog.entry_count; ++i) {
            switch (user_window_catalog.entries[i].role) {
            case THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE:
                ++user_window_descriptor_count;
                break;
            case THERON_TRACK02_USER_DATA_WINDOW_POST_BOUNDARY_SPAN:
                ++user_window_span_count;
                break;
            case THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE:
                ++user_window_initial_count;
                break;
            case THERON_TRACK02_USER_DATA_WINDOW_UNKNOWN:
            default:
                break;
            }
        }
    }

    memset(&text_marker_catalog, 0, sizeof(text_marker_catalog));
    if (theron_v1_track02_catalog_startup_text_markers(
            hucard_rom,
            hucard_rom_size,
            md5_hex,
            &text_marker_catalog) == THERON_TRACK02_SIGNAL_OK) {
        size_t i;
        for (i = 0u; i < text_marker_catalog.marker_count; ++i) {
            switch (text_marker_catalog.markers[i].kind) {
            case THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT:
                ++startup_text_us_count;
                break;
            case THERON_TRACK02_STARTUP_TEXT_JP_CHAMPION_ROSTER_CLUSTER:
                ++startup_text_jp_count;
                break;
            case THERON_TRACK02_STARTUP_TEXT_UNKNOWN:
            default:
                break;
            }
        }
    }

    for (anchor = 0u; anchor < signal.anchor_count; ++anchor) {
        Theron_Track02StartupSemanticHandoff semantic_handoff;
        Theron_Track02LevelHandoff semantic_level_handoff;
        Theron_Track02LevelHandoffStatus semantic_status;
        Theron_V1_Level semantic_level;

        semantic_status = theron_v1_track02_load_startup_semantic_level(
            hucard_rom,
            hucard_rom_size,
            md5_hex,
            signal.descriptor_offsets[anchor],
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            &semantic_level,
            &semantic_handoff,
            &semantic_level_handoff);
        ++tried;
        last_semantic_status = semantic_status;
        last_seed_status = semantic_handoff.seed_table_status;
        last_user_data_offset_valid =
            semantic_handoff.user_data_offset_valid;
        last_user_data_offset = semantic_handoff.user_data_offset;
        if (semantic_status == THERON_TRACK02_LEVEL_HANDOFF_OK) {
            world->current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
            world->current_level = 0;
            world->levels[0][0] = semantic_level;
            world->level_loaded[0][0] = 1;
            theron_v1_party_place(world,
                                  world->levels[0][0].start_x,
                                  world->levels[0][0].start_y,
                                  world->levels[0][0].start_dir);
            if (receipt && receipt_cap > 0u) {
                snprintf(receipt,
                         receipt_cap,
                         "Track 02 semantic initial level anchor=%zu offset=0x%zx user_valid=%d user=0x%zx seed_status=%s seed0=%u seed6=%u startup_seed=0x%08x startup_level=0x%04x seed_in_table=%d user_windows=%zu user_desc=%zu user_span=%zu user_initial=%zu text_markers=%zu text_us=%zu text_jp=%zu header=%ux%u start=(%d,%d,%d)",
                         anchor,
                         semantic_level_handoff.absolute_offset,
                         semantic_handoff.user_data_offset_valid,
                         semantic_handoff.user_data_offset,
                         theron_v1_track02_semantic_binding_status_name(
                             semantic_handoff.seed_table_status),
                         (unsigned)semantic_handoff.seed_table_binding
                             .dungeon_seed_table.seeds[0],
                         (unsigned)semantic_handoff.seed_table_binding
                             .dungeon_seed_table
                             .seeds[THERON_TRACK02_DUNGEON_COUNT - 1u],
                         (unsigned)semantic_handoff.startup_seed,
                         (unsigned)semantic_handoff.startup_level_index,
                         semantic_handoff.startup_seed_in_seed_table,
                         user_window_catalog.entry_count,
                         user_window_descriptor_count,
                         user_window_span_count,
                         user_window_initial_count,
                         text_marker_catalog.marker_count,
                         startup_text_us_count,
                         startup_text_jp_count,
                         (unsigned)semantic_level_handoff.header_width,
                         (unsigned)semantic_level_handoff.header_height,
                         (int)world->levels[0][0].start_x,
                         (int)world->levels[0][0].start_y,
                         (int)world->levels[0][0].start_dir);
            }
            return 1;
        }
    }

    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "Track 02 semantic startup handoff scanned anchors=%zu attempts=%d last_status=%s last_seed_status=%s last_user_valid=%d last_user=0x%zx user_windows=%zu user_desc=%zu user_span=%zu user_initial=%zu text_markers=%zu text_us=%zu text_jp=%zu; no semantic runtime level claim",
                 signal.anchor_count,
                 tried,
                 theron_v1_track02_level_handoff_status_name(
                     last_semantic_status),
                 theron_v1_track02_semantic_binding_status_name(
                     last_seed_status),
                 last_user_data_offset_valid,
                 last_user_data_offset,
                 user_window_catalog.entry_count,
                 user_window_descriptor_count,
                 user_window_span_count,
                 user_window_initial_count,
                 text_marker_catalog.marker_count,
                 startup_text_us_count,
                 startup_text_jp_count);
    }
    return 0;
}

static int theron_v1_startup_runtime_has_verified_track02_request(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex) {

    if (!hucard_rom || hucard_rom_size == 0u ||
        !md5_hex || md5_hex[0] == '\0') {
        return 0;
    }
    return theron_v1_track02_variant_for_md5(md5_hex) !=
        THERON_TRACK02_VARIANT_UNKNOWN;
}

int theron_v1_startup_runtime_load_initial_level(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    char *receipt,
    size_t receipt_cap) {
    uint8_t level_data[12 + 10 * 10];
    const Theron_DungeonMeta *meta;
    Theron_V1_Level preview;
    size_t level_size;
    int dungeon_index;
    uint32_t seed;
    Theron_MapLoadResult r;

    if (!world) {
        return 0;
    }
    if (dungeon_id < THERON_DUNGEON_1_HALL_OF_RECORDS ||
        dungeon_id > THERON_DUNGEON_COUNT) {
        dungeon_id = THERON_DUNGEON_1_HALL_OF_RECORDS;
    }
    dungeon_index = (int)dungeon_id - 1;
    if (theron_v1_startup_runtime_try_track02_initial_level(world,
                                                            hucard_rom,
                                                            hucard_rom_size,
                                                            md5_hex,
                                                            dungeon_id,
                                                            receipt,
                                                            receipt_cap)) {
        return 1;
    }
    if (theron_v1_startup_runtime_has_verified_track02_request(
            hucard_rom,
            hucard_rom_size,
            md5_hex)) {
        if (receipt && receipt_cap > 0u) {
            char semantic_detail[256];
            semantic_detail[0] = '\0';
            if (receipt[0] != '\0') {
                snprintf(semantic_detail,
                         sizeof(semantic_detail),
                         "%s",
                         receipt);
            }
            if (semantic_detail[0] != '\0') {
                snprintf(receipt,
                         receipt_cap,
                         "Track 02 verified profile present but no semantic runtime handoff; fallback visuals blocked; %s",
                         semantic_detail);
            } else {
                snprintf(receipt,
                         receipt_cap,
                         "Track 02 verified profile present but no semantic runtime handoff; fallback visuals blocked");
            }
        }
        return 0;
    }
    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }

    /* THQUEST.ASM T400/T520/T560: runtime entry owns dungeon bank load,
     * party start position, and initial map handoff. Until every Track 02
     * bank offset is decoded, keep the bounded deterministic startup room
     * on the real theron_v1_level_load() path. */
    meta = theron_v1_dungeon_meta(dungeon_id);
    seed = meta ? meta->dungeon_seed : 313u;
    memset(&preview, 0, sizeof(preview));
    level_size = theron_v1_startup_fallback_room_synthesize(
        level_data,
        sizeof(level_data),
        dungeon_id,
        &preview);
    if (level_size == 0u) {
        return 0;
    }

    world->current_dungeon = dungeon_id;
    world->current_level = 0;
    r = theron_v1_level_load(&world->levels[dungeon_index][0],
                             level_data,
                             (int)level_size,
                             (int)world->current_dungeon,
                             world->current_level);
    if (r != THERON_MAP_OK) {
        return 0;
    }
    world->levels[dungeon_index][0].start_x = preview.start_x;
    world->levels[dungeon_index][0].start_y = preview.start_y;
    world->levels[dungeon_index][0].start_dir = preview.start_dir;
    world->level_loaded[dungeon_index][0] = 1;
    theron_v1_party_place(world,
                          world->levels[dungeon_index][0].start_x,
                          world->levels[dungeon_index][0].start_y,
                          world->levels[dungeon_index][0].start_dir);
    if (receipt && receipt_cap > 0u && receipt[0] == '\0') {
        snprintf(receipt,
                 receipt_cap,
                 "Track 02 descriptor scan unavailable; fallback room stage=%d size=%dx%d seed=%u start=(%d,%d,%d)",
                 (int)dungeon_id,
                 preview.width,
                 preview.height,
                 (unsigned)seed,
                 (int)preview.start_x,
                 (int)preview.start_y,
                 (int)preview.start_dir);
    }
    return 1;
}

void theron_v1_startup_runtime_entry_request_init(
    Theron_V1StartupRuntimeEntryRequest *request) {

    if (!request) {
        return;
    }
    memset(request, 0, sizeof(*request));
}

void theron_v1_startup_runtime_entry_result_init(
    Theron_V1StartupRuntimeEntryResult *result) {

    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->result = THERON_STARTUP_OK;
    result->runtime_level_source = THERON_V1_STARTUP_RUNTIME_LEVEL_NONE;
}

void theron_v1_startup_runtime_entry_apply_receipt_init(
    Theron_V1StartupRuntimeEntryApplyReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
}

int theron_v1_startup_host_receipt_from_runtime_entry_apply(
    const Theron_V1StartupRuntimeEntryApplyReceipt *apply_receipt,
    Theron_StartupHostReceipt *out_receipt) {

    if (!apply_receipt || !out_receipt) {
        return 0;
    }
    theron_v1_startup_host_receipt_init(out_receipt);
    out_receipt->input_result = apply_receipt->input_result;
    out_receipt->status_scope = apply_receipt->status_scope;
    out_receipt->status = apply_receipt->status;
    out_receipt->inspect_scope = apply_receipt->inspect_scope;
    snprintf(out_receipt->inspect_detail,
             sizeof(out_receipt->inspect_detail),
             "%s",
             apply_receipt->inspect_detail);
    out_receipt->log_first_line = apply_receipt->log_first_line;
    out_receipt->log_receipt = apply_receipt->log_receipt ? 1 : 0;
    return 1;
}

static void theron_v1_startup_runtime_entry_capture_result(
    const Theron_V1_World *world,
    const char *runtime_receipt,
    int verified_track02_request,
    Theron_V1StartupRuntimeEntryResult *out_result) {

    (void)runtime_receipt;
    if (!world || !out_result) {
        return;
    }
    theron_v1_startup_runtime_entry_result_init(out_result);
    out_result->result = THERON_STARTUP_OK;
    out_result->level_loaded = 1;
    out_result->structured_runtime_route = 1;
    out_result->runtime_receipt_text_route = 0;
    if (verified_track02_request) {
        out_result->runtime_level_source =
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC;
        out_result->track02_semantic_handoff = 1;
    } else {
        out_result->runtime_level_source =
            THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM;
    }
    out_result->party_x = world->party.leader_x;
    out_result->party_y = world->party.leader_y;
    out_result->party_dir = world->party.leader_dir;
    out_result->tick_count = (int)world->world_tick;
}

static void theron_v1_startup_runtime_entry_capture_failure_route(
    const char *runtime_receipt,
    int verified_track02_request,
    Theron_V1StartupRuntimeEntryResult *out_result) {

    if (!out_result) {
        return;
    }
    if (verified_track02_request) {
        out_result->runtime_level_source =
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED;
        out_result->fallback_visuals_blocked = 1;
        out_result->structured_runtime_route = 1;
        out_result->runtime_receipt_text_route = 0;
        return;
    }
    if (runtime_receipt && strstr(runtime_receipt, "fallback visuals blocked")) {
        out_result->runtime_level_source =
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED;
        out_result->fallback_visuals_blocked = 1;
        out_result->structured_runtime_route = 0;
        out_result->runtime_receipt_text_route = 1;
    }
}

static const char *theron_v1_startup_runtime_level_source_name(
    int runtime_level_source) {

    switch (runtime_level_source) {
    case THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM:
        return "fallback-room";
    case THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC:
        return "track02-semantic";
    case THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED:
        return "track02-blocked";
    case THERON_V1_STARTUP_RUNTIME_LEVEL_NONE:
    default:
        return "none";
    }
}

int theron_v1_startup_runtime_enter_from_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const Theron_V1StartupRuntimeEntryRequest *request,
    Theron_V1StartupRuntimeEntryResult *out_result,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupRuntimeLevelLoadContext level_load_context;
    Theron_StartupResult result;
    int verified_track02_request = 0;

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (out_result) {
        theron_v1_startup_runtime_entry_result_init(out_result);
    }
    if (!flow || !world || !request) {
        if (out_result) {
            out_result->result = THERON_STARTUP_ERR_NULL;
        }
        return 0;
    }

    result = theron_v1_startup_enter_forcefield_with_roster(
        flow,
        &world->party,
        request->roster_names,
        request->roster_name_count);
    if (result != THERON_STARTUP_OK) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt,
                     receipt_cap,
                     "startup-flow forcefield failed: %s",
                     theron_v1_startup_result_name(result));
        }
        if (out_result) {
            out_result->result = result;
        }
        return 0;
    }

    level_load_context.hucard_rom = request->hucard_rom;
    level_load_context.hucard_rom_size = request->hucard_rom_size;
    level_load_context.md5_hex = request->md5_hex;
    verified_track02_request =
        theron_v1_startup_runtime_has_verified_track02_request(
            request->hucard_rom,
            request->hucard_rom_size,
            request->md5_hex);
    result = theron_v1_startup_enter_runtime_from_forcefield(
        flow,
        world,
        theron_v1_startup_runtime_level_load_callback,
        &level_load_context,
        receipt,
        receipt_cap);
    if (out_result) {
        out_result->result = result;
    }
    if (result != THERON_STARTUP_OK) {
        if (receipt && receipt_cap > 0u && receipt[0] == '\0') {
            snprintf(receipt,
                     receipt_cap,
                     "startup-flow runtime entry failed: %s",
                     theron_v1_startup_result_name(result));
        }
        theron_v1_startup_runtime_entry_capture_failure_route(receipt,
                                                              verified_track02_request,
                                                              out_result);
        return 0;
    }

    theron_v1_startup_runtime_entry_capture_result(world,
                                                   receipt,
                                                   verified_track02_request,
                                                   out_result);
    return 1;
}

int theron_v1_startup_runtime_entry_apply_receipt(
    const Theron_StartupActionPlan *plan,
    const Theron_V1StartupRuntimeEntryResult *result,
    const char *runtime_receipt,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_receipt) {

    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_runtime_entry_apply_receipt_init(out_receipt);
    if (!plan || !result || result->result != THERON_STARTUP_OK ||
        !result->level_loaded) {
        return 0;
    }

    out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
    out_receipt->status_scope = plan->status_scope
        ? plan->status_scope
        : "BOOT";
    out_receipt->status = plan->status ? plan->status : "THERON READY";
    out_receipt->inspect_scope = "READY";
    out_receipt->runtime_level_source = result->runtime_level_source;
    out_receipt->track02_semantic_handoff =
        result->track02_semantic_handoff;
    out_receipt->fallback_visuals_blocked =
        result->fallback_visuals_blocked;
    out_receipt->structured_runtime_route =
        result->structured_runtime_route;
    out_receipt->runtime_receipt_text_route =
        result->runtime_receipt_text_route;
    if (runtime_receipt && runtime_receipt[0]) {
        snprintf(out_receipt->inspect_detail,
                 sizeof(out_receipt->inspect_detail),
                 "%s route=%s semantic=%d fallback_blocked=%d structured=%d text_route=%d",
                 runtime_receipt,
                 theron_v1_startup_runtime_level_source_name(
                     result->runtime_level_source),
                 result->track02_semantic_handoff,
                 result->fallback_visuals_blocked,
                 result->structured_runtime_route,
                 result->runtime_receipt_text_route);
        out_receipt->log_receipt = 1;
    }
    out_receipt->log_first_line = "T0: THERON LOADED";
    return 1;
}

static int theron_v1_startup_runtime_entry_failure_apply_receipt(
    const Theron_StartupActionPlan *plan,
    const Theron_V1StartupRuntimeEntryResult *result,
    const char *runtime_receipt,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_receipt) {

    const char *status;

    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_runtime_entry_apply_receipt_init(out_receipt);
    status = (runtime_receipt && runtime_receipt[0])
        ? runtime_receipt
        : "THERON RUNTIME ENTRY FAILED";
    out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
    out_receipt->status_scope = (plan && plan->status_scope)
        ? plan->status_scope
        : "READY";
    out_receipt->status = status;
    out_receipt->inspect_scope = "READY";
    if (result) {
        out_receipt->runtime_level_source = result->runtime_level_source;
        out_receipt->track02_semantic_handoff =
            result->track02_semantic_handoff;
        out_receipt->fallback_visuals_blocked =
            result->fallback_visuals_blocked;
        out_receipt->structured_runtime_route =
            result->structured_runtime_route;
        out_receipt->runtime_receipt_text_route =
            result->runtime_receipt_text_route;
    }
    snprintf(out_receipt->inspect_detail,
             sizeof(out_receipt->inspect_detail),
             "%s%s%s route=%s semantic=%d fallback_blocked=%d structured=%d text_route=%d",
             status,
             result ? " result=" : "",
             result ? theron_v1_startup_result_name(result->result) : "",
             result
                 ? theron_v1_startup_runtime_level_source_name(
                       result->runtime_level_source)
                 : "none",
             result ? result->track02_semantic_handoff : 0,
             result ? result->fallback_visuals_blocked : 0,
             result ? result->structured_runtime_route : 0,
             result ? result->runtime_receipt_text_route : 0);
    out_receipt->log_first_line = "T0: THERON BLOCKED";
    out_receipt->log_receipt = 1;
    return 1;
}

int theron_v1_startup_runtime_entry_state_receipt_from_result(
    const Theron_StartupFlow *flow,
    const Theron_V1StartupRuntimeEntryResult *result,
    Theron_StartupStateReceipt *out_receipt) {

    if (!flow || !result || !out_receipt ||
        result->result != THERON_STARTUP_OK) {
        return 0;
    }
    if (!theron_v1_startup_state_receipt_from_flow(flow, out_receipt)) {
        return 0;
    }
    out_receipt->set_level_loaded = 1;
    out_receipt->level_loaded = result->level_loaded;
    out_receipt->set_party_pose = 1;
    out_receipt->party_x = result->party_x;
    out_receipt->party_y = result->party_y;
    out_receipt->party_dir = result->party_dir;
    out_receipt->set_tick_count = 1;
    out_receipt->tick_count = result->tick_count;
    out_receipt->set_runtime_level_route = 1;
    out_receipt->runtime_level_source = result->runtime_level_source;
    out_receipt->runtime_track02_semantic_handoff =
        result->track02_semantic_handoff;
    out_receipt->runtime_fallback_visuals_blocked =
        result->fallback_visuals_blocked;
    return 1;
}

int theron_v1_startup_runtime_load_initial_level_with_receipts(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupRuntimeEntryResult local_result;
    Theron_V1StartupRuntimeEntryResult *result =
        out_result ? out_result : &local_result;
    Theron_V1StartupRuntimeEntryApplyReceipt apply_receipt;
    Theron_StartupFlow flow;
    int verified_track02_request;

    theron_v1_startup_runtime_entry_result_init(result);
    theron_v1_startup_runtime_entry_apply_receipt_init(&apply_receipt);
    if (out_apply_receipt) {
        theron_v1_startup_runtime_entry_apply_receipt_init(
            out_apply_receipt);
    }
    if (out_state_receipt) {
        theron_v1_startup_state_receipt_init(out_state_receipt);
    }

    verified_track02_request =
        theron_v1_startup_runtime_has_verified_track02_request(
            hucard_rom,
            hucard_rom_size,
            md5_hex);
    if (!theron_v1_startup_runtime_load_initial_level(world,
                                                      hucard_rom,
                                                      hucard_rom_size,
                                                      md5_hex,
                                                      dungeon_id,
                                                      receipt,
                                                      receipt_cap)) {
        result->result = THERON_STARTUP_ERR_LEVEL_LOAD;
        theron_v1_startup_runtime_entry_capture_failure_route(receipt,
                                                              verified_track02_request,
                                                              result);
        theron_v1_startup_runtime_entry_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_apply_receipt) {
            *out_apply_receipt = apply_receipt;
        }
        return 0;
    }

    theron_v1_startup_runtime_entry_capture_result(world,
                                                   receipt,
                                                   verified_track02_request,
                                                   result);
    theron_v1_startup_flow_init(&flow);
    flow.phase = THERON_STARTUP_PHASE_IN_DUNGEON;
    flow.selected_dungeon = dungeon_id;
    flow.forcefield_entered = 1;
    if (out_state_receipt &&
        !theron_v1_startup_runtime_entry_state_receipt_from_result(
            &flow,
            result,
            out_state_receipt)) {
        return 0;
    }
    if (!theron_v1_startup_runtime_entry_apply_receipt(plan,
                                                       result,
                                                       receipt,
                                                       &apply_receipt)) {
        return 0;
    }
    if (out_apply_receipt) {
        *out_apply_receipt = apply_receipt;
    }
    return 1;
}

int theron_v1_startup_runtime_load_initial_level_with_host_receipts(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupRuntimeEntryApplyReceipt apply_receipt;

    theron_v1_startup_runtime_entry_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (!theron_v1_startup_runtime_load_initial_level_with_receipts(
            world,
            hucard_rom,
            hucard_rom_size,
            md5_hex,
            dungeon_id,
            plan,
            out_result,
            &apply_receipt,
            out_state_receipt,
            receipt,
            receipt_cap)) {
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_runtime_entry_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_runtime_entry_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_runtime_enter_from_forcefield_with_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const Theron_V1StartupRuntimeEntryRequest *request,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupRuntimeEntryResult local_result;
    Theron_V1StartupRuntimeEntryResult *result =
        out_result ? out_result : &local_result;

    theron_v1_startup_runtime_entry_result_init(result);
    if (out_apply_receipt) {
        theron_v1_startup_runtime_entry_apply_receipt_init(
            out_apply_receipt);
    }
    if (out_state_receipt) {
        theron_v1_startup_state_receipt_init(out_state_receipt);
    }
    if (!theron_v1_startup_runtime_enter_from_forcefield(flow,
                                                         world,
                                                         request,
                                                         result,
                                                         receipt,
                                                         receipt_cap)) {
        if (out_apply_receipt) {
            theron_v1_startup_runtime_entry_failure_apply_receipt(
                plan,
                result,
                receipt,
                out_apply_receipt);
        }
        return 0;
    }
    if (out_state_receipt &&
        !theron_v1_startup_runtime_entry_state_receipt_from_result(
            flow,
            result,
            out_state_receipt)) {
        return 0;
    }
    if (out_apply_receipt &&
        !theron_v1_startup_runtime_entry_apply_receipt(plan,
                                                       result,
                                                       receipt,
                                                       out_apply_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_runtime_enter_from_forcefield_facts_with_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    int startup_roster_name_count,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupRuntimeEntryRequest request;
    const char *roster_names[THERON_STARTUP_MEDIA_ROSTER_CAPACITY];
    int roster_count;
    int i;

    memset(roster_names, 0, sizeof(roster_names));
    roster_count = startup_roster_name_count;
    if (roster_count < 0) {
        roster_count = 0;
    }
    if (roster_count > (int)THERON_STARTUP_MEDIA_ROSTER_CAPACITY) {
        roster_count = (int)THERON_STARTUP_MEDIA_ROSTER_CAPACITY;
    }
    for (i = 0; i < roster_count; ++i) {
        roster_names[i] = startup_roster_names ? startup_roster_names[i] : NULL;
    }

    theron_v1_startup_runtime_entry_request_init(&request);
    request.hucard_rom = hucard_rom;
    request.hucard_rom_size = hucard_rom_size;
    request.md5_hex = md5_hex;
    request.roster_names = roster_names;
    request.roster_name_count = roster_count;
    return theron_v1_startup_runtime_enter_from_forcefield_with_receipts(
        flow,
        world,
        &request,
        plan,
        out_result,
        out_apply_receipt,
        out_state_receipt,
        receipt,
        receipt_cap);
}

int theron_v1_startup_runtime_enter_from_forcefield_facts_with_host_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    int startup_roster_name_count,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupRuntimeEntryApplyReceipt apply_receipt;

    theron_v1_startup_runtime_entry_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (!theron_v1_startup_runtime_enter_from_forcefield_facts_with_receipts(
            flow,
            world,
            hucard_rom,
            hucard_rom_size,
            md5_hex,
            startup_roster_names,
            startup_roster_name_count,
            plan,
            out_result,
            &apply_receipt,
            out_state_receipt,
            receipt,
            receipt_cap)) {
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_runtime_entry_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_runtime_entry_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_runtime_enter_from_forcefield_boot_profile_with_host_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const void *boot_profile,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    int startup_roster_name_count,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    const Theron_V1_BootProfile *profile =
        (const Theron_V1_BootProfile *)boot_profile;
    const char *md5_hex =
        (profile && profile->graphics_md5[0]) ? profile->graphics_md5 : NULL;

    return theron_v1_startup_runtime_enter_from_forcefield_facts_with_host_receipts(
        flow,
        world,
        hucard_rom,
        hucard_rom_size,
        md5_hex,
        startup_roster_names,
        startup_roster_name_count,
        plan,
        out_result,
        out_host_receipt,
        out_state_receipt,
        receipt,
        receipt_cap);
}
