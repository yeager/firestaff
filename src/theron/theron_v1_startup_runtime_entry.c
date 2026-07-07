#include "theron_v1_startup_runtime_entry.h"

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
    size_t user_window_descriptor_count = 0u;
    size_t user_window_span_count = 0u;
    size_t user_window_initial_count = 0u;
    size_t startup_text_us_count = 0u;
    size_t startup_text_jp_count = 0u;
    size_t anchor;
    size_t entry;
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
        Theron_Track02LevelHandoff initial_handoff;
        Theron_Track02LevelHandoffStatus initial_status;
        Theron_V1_Level initial_candidate;

        initial_status = theron_v1_track02_load_initial_level_candidate(
            hucard_rom,
            hucard_rom_size,
            md5_hex,
            signal.descriptor_offsets[anchor],
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            &initial_candidate,
            &initial_handoff);
        ++tried;
        if (initial_status == THERON_TRACK02_LEVEL_HANDOFF_OK) {
            world->current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
            world->current_level = 0;
            world->levels[0][0] = initial_candidate;
            world->level_loaded[0][0] = 1;
            theron_v1_party_place(world,
                                  world->levels[0][0].start_x,
                                  world->levels[0][0].start_y,
                                  world->levels[0][0].start_dir);
            if (receipt && receipt_cap > 0u) {
                snprintf(receipt,
                         receipt_cap,
                         "Track 02 initial level bind=%s anchor=%zu offset=0x%zx user_valid=%d user=0x%zx user_windows=%zu user_desc=%zu user_span=%zu user_initial=%zu text_markers=%zu text_us=%zu text_jp=%zu expected=0x%zx delta=0x%zx candidates=%zu match=%d header=%ux%u seed=0x%08x start=(%d,%d,%d)",
                         theron_v1_track02_level_handoff_status_name(
                             (Theron_Track02LevelHandoffStatus)
                                 initial_handoff.binding_status),
                         anchor,
                         initial_handoff.absolute_offset,
                         initial_handoff.user_data_offset_valid,
                         initial_handoff.user_data_offset,
                         user_window_catalog.entry_count,
                         user_window_descriptor_count,
                         user_window_span_count,
                         user_window_initial_count,
                         text_marker_catalog.marker_count,
                         startup_text_us_count,
                         startup_text_jp_count,
                         initial_handoff.expected_offset,
                         initial_handoff.descriptor_delta,
                         initial_handoff.candidate_count,
                         initial_handoff.matches_initial_anchor,
                         (unsigned)initial_handoff.header_width,
                         (unsigned)initial_handoff.header_height,
                         (unsigned)initial_handoff.header_seed,
                         (int)world->levels[0][0].start_x,
                         (int)world->levels[0][0].start_y,
                         (int)world->levels[0][0].start_dir);
            }
            return 1;
        }

        for (entry = 0u; entry < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++entry) {
            Theron_Track02LevelHandoff handoff;
            Theron_Track02LevelHandoffStatus level_status;
            Theron_V1_Level candidate;

            level_status = theron_v1_track02_load_descriptor_window_level(
                hucard_rom,
                hucard_rom_size,
                signal.descriptor_offsets[anchor],
                entry,
                THERON_DUNGEON_1_HALL_OF_RECORDS,
                (int)entry,
                &candidate,
                &handoff);
            ++tried;
            if (level_status == THERON_TRACK02_LEVEL_HANDOFF_OK) {
                world->current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
                world->current_level = 0;
                world->levels[0][0] = candidate;
                world->level_loaded[0][0] = 1;
                theron_v1_party_place(world,
                                      world->levels[0][0].start_x,
                                      world->levels[0][0].start_y,
                                      world->levels[0][0].start_dir);
                if (receipt && receipt_cap > 0u) {
                    snprintf(receipt,
                             receipt_cap,
                             "Track 02 level anchor=%zu entry=%zu offset=0x%zx size=%zu",
                             anchor,
                             entry,
                             handoff.absolute_offset,
                             handoff.byte_count);
                }
                return 1;
            }
        }
    }

    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "Track 02 descriptors scanned anchors=%zu windows=%d; no level claim yet",
                 signal.anchor_count,
                 tried);
    }
    return 0;
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
}

void theron_v1_startup_runtime_entry_apply_receipt_init(
    Theron_V1StartupRuntimeEntryApplyReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
}

static void theron_v1_startup_runtime_entry_capture_result(
    const Theron_V1_World *world,
    Theron_V1StartupRuntimeEntryResult *out_result) {

    if (!world || !out_result) {
        return;
    }
    theron_v1_startup_runtime_entry_result_init(out_result);
    out_result->result = THERON_STARTUP_OK;
    out_result->level_loaded = 1;
    out_result->party_x = world->party.leader_x;
    out_result->party_y = world->party.leader_y;
    out_result->party_dir = world->party.leader_dir;
    out_result->tick_count = (int)world->world_tick;
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
        return 0;
    }

    theron_v1_startup_runtime_entry_capture_result(world, out_result);
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
    if (runtime_receipt && runtime_receipt[0]) {
        snprintf(out_receipt->inspect_detail,
                 sizeof(out_receipt->inspect_detail),
                 "%s",
                 runtime_receipt);
        out_receipt->log_receipt = 1;
    }
    out_receipt->log_first_line = "T0: THERON LOADED";
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
