#include "theron_v1_startup_runtime_entry.h"

#include "theron_v1_boot.h"
#include "theron_v1_stage3_irq2_dispatch.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const uint8_t *hucard_rom;
    size_t hucard_rom_size;
    const char *md5_hex;
} Theron_V1StartupRuntimeLevelLoadContext;

static void theron_v1_startup_copy_object_anchor_receipt(
    int out_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint32_t out_hash[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint32_t in_hash[THERON_TRACK02_MAX_BANK_ANCHORS]);

static void theron_v1_startup_copy_level_anchor_receipt_u64(
    int out_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint64_t out_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint64_t out_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    int out_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_width[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_height[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint32_t out_seed[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_level_index[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint64_t in_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint64_t in_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_width[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_height[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint32_t in_seed[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_level_index[THERON_TRACK02_MAX_BANK_ANCHORS]);

static int theron_v1_startup_runtime_publish_track02_route(
    Theron_V1_World *world,
    Theron_DungeonID dungeon_id,
    const Theron_Track02DungeonRoute *route);

/* The only original dynamic stage-two CD_READ proved so far is the JP/US
 * one-sector stage-three load. Direct runtime entry must verify its physical
 * `$3800` BRK $ff bytes too, rather than trusting a previously built startup
 * receipt. No descriptor semantics are admitted at this boundary. */
static int theron_v1_startup_runtime_stage3_loader_ready(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex) {
    Theron_Track02Variant variant;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3Irq2DispatchReceipt dispatch;

    if (!track02_data || track02_size == 0u || !md5_hex || !md5_hex[0]) {
        return 0;
    }
    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant != THERON_TRACK02_VARIANT_JP_BIN &&
        variant != THERON_TRACK02_VARIANT_US_BIN) {
        return 1;
    }
    memset(&payload, 0, sizeof(payload));
    memset(&dispatch, 0, sizeof(dispatch));
    return theron_v1_track02_inspect_stage2_dynamic_payload(
               track02_data, track02_size, md5_hex, &payload) ==
               THERON_TRACK02_SIGNAL_OK &&
           theron_v1_stage3_irq2_dispatch_from_original_media(
               track02_data, track02_size, &payload, &dispatch) &&
           dispatch.valid && dispatch.irq2_dispatch_proven;
}

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
    Theron_Track02StartupBitmapCatalog bitmap_catalog;
    Theron_Track02StartupBitmapAtlas bitmap_atlas;
    int bitmap_atlas_ready = 0;
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
    if (!theron_v1_startup_runtime_stage3_loader_ready(
            hucard_rom, hucard_rom_size, md5_hex)) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap,
                     "Track 02 stage-three loader bytes rejected");
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

    memset(&bitmap_catalog, 0, sizeof(bitmap_catalog));
    memset(&bitmap_atlas, 0, sizeof(bitmap_atlas));
    if (theron_v1_track02_catalog_startup_bitmap_samples(
            hucard_rom, hucard_rom_size, md5_hex, &bitmap_catalog) ==
            THERON_TRACK02_SIGNAL_OK &&
        theron_v1_track02_build_startup_bitmap_atlas_wide(
            &bitmap_catalog, &bitmap_atlas) == THERON_TRACK02_SIGNAL_OK) {
        bitmap_atlas_ready = 1;
    }

    for (anchor = 0u; anchor < signal.anchor_count; ++anchor) {
        Theron_Track02StartupSemanticHandoff semantic_handoff;
        Theron_Track02LevelHandoff semantic_level_handoff;
        Theron_Track02LevelHandoffStatus semantic_status;
        Theron_V1_Level semantic_level;

        if (bitmap_atlas_ready) {
            Theron_Track02DungeonRoute route;
            if (theron_v1_track02_load_verified_dungeon_route(
                    hucard_rom, hucard_rom_size, md5_hex,
                    signal.descriptor_offsets[anchor], dungeon_id,
                    &bitmap_atlas, &route) == THERON_TRACK02_DUNGEON_ROUTE_OK &&
                theron_v1_startup_runtime_publish_track02_route(world,
                                                                 dungeon_id,
                                                                 &route)) {
                if (receipt && receipt_cap > 0u) {
                    snprintf(receipt, receipt_cap,
                             "Track 02 dungeon route stage=%d anchor=%zu level=0x%zx object=0x%zx rows=%zu bitmap=0x%08x",
                             (int)dungeon_id, anchor, route.level_raw_offset,
                             route.object_raw_offset, route.objects.record_count,
                             (unsigned)route.bitmap_atlas.checksum);
                }
                return 1;
            }
        }

        semantic_status = theron_v1_track02_load_startup_semantic_level(
            hucard_rom,
            hucard_rom_size,
            md5_hex,
            signal.descriptor_offsets[anchor],
            dungeon_id,
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
            Theron_RuntimeMediaIdentity identity;

            memset(&identity, 0, sizeof(identity));
            identity.ready = 1;
            identity.track02_variant = (int)signal.variant;
            identity.bank_anchor_index = anchor;
            identity.bank_descriptor_offset = signal.descriptor_offsets[anchor];
            identity.bank_first_value = signal.first_value;
            identity.bank_last_value = signal.last_value;
            identity.bank_stride = signal.stride;
            identity.audio_frame_ready = signal.audio_bank_id_recognized[anchor] ? 1 : 0;
            identity.audio_bank_id = signal.audio_bank_id[anchor];
            identity.audio_bank_id_offset = signal.audio_bank_id_offsets[anchor];
            identity.audio_bank_prefix_offset = signal.audio_bank_prefix_offsets[anchor];
            identity.checksum = identity.audio_bank_id ^
                (uint32_t)identity.bank_descriptor_offset ^
                ((uint32_t)identity.bank_first_value << 16) ^ identity.bank_last_value;
            world->current_dungeon = dungeon_id;
            world->current_level = 0;
            world->levels[(int)dungeon_id - 1][0] = semantic_level;
            world->level_loaded[(int)dungeon_id - 1][0] = 1;
            theron_v1_party_place(world,
                                  world->levels[(int)dungeon_id - 1][0].start_x,
                                  world->levels[(int)dungeon_id - 1][0].start_y,
                                  world->levels[(int)dungeon_id - 1][0].start_dir);
            if (!theron_v1_world_runtime_media_set_identity(world, &identity)) {
                return 0;
            }
            if (receipt && receipt_cap > 0u) {
                snprintf(receipt,
                         receipt_cap,
                         "Track 02 semantic initial level stage=%d anchor=%zu offset=0x%zx user_valid=%d user=0x%zx seed_status=%s startup_seed=0x%08x startup_level=0x%04x header=%ux%u start=(%d,%d,%d)",
                         (int)dungeon_id,
                         anchor,
                         semantic_level_handoff.absolute_offset,
                         semantic_handoff.user_data_offset_valid,
                         semantic_handoff.user_data_offset,
                         theron_v1_track02_semantic_binding_status_name(
                             semantic_handoff.seed_table_status),
                         (unsigned)semantic_handoff.startup_seed,
                         (unsigned)semantic_handoff.startup_level_index,
                         (unsigned)semantic_level_handoff.header_width,
                         (unsigned)semantic_level_handoff.header_height,
                         (int)world->levels[(int)dungeon_id - 1][0].start_x,
                         (int)world->levels[(int)dungeon_id - 1][0].start_y,
                         (int)world->levels[(int)dungeon_id - 1][0].start_dir);
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

static int theron_v1_startup_runtime_publish_track02_route(
    Theron_V1_World *world,
    Theron_DungeonID dungeon_id,
    const Theron_Track02DungeonRoute *route) {
    size_t i;

    if (!world || !route || !route->valid ||
        route->objects.record_count > THERON_MAX_OBJECTS ||
        dungeon_id < THERON_DUNGEON_1_HALL_OF_RECORDS ||
        dungeon_id > THERON_DUNGEON_COUNT) return 0;

    world->current_dungeon = dungeon_id;
    world->current_level = 0;
    world->levels[(int)dungeon_id - 1][0] = route->level;
    world->levels[(int)dungeon_id - 1][0].thing_count = 0;
    world->level_loaded[(int)dungeon_id - 1][0] = 1;
    world->object_count = 0;
    for (i = 0u; i < route->objects.record_count; ++i) {
        const Theron_Track02ObjectTableRecord *record = &route->objects.records[i];
        Theron_V1_Object object;
        if (record->level_index != 0u || record->x >= route->level.width ||
            record->y >= route->level.height || record->kind == 0u ||
            record->kind > THERON_OBJTYPE_QUEST_ITEM) return 0;
        memset(&object, 0, sizeof(object));
        object.type = record->kind;
        object.state = record->flags & 0x03u;
        object.x = record->x;
        object.y = record->y;
        object.level = record->level_index;
        object.dungeon_id = dungeon_id;
        object.quantity = record->argument ? record->argument : 1;
        object.flags = record->flags;
        if (theron_v1_object_place(world, &object) != 0) return 0;
        ++world->levels[(int)dungeon_id - 1][0].thing_count;
    }
    theron_v1_party_place(world, route->level.start_x, route->level.start_y,
                          route->level.start_dir);
    return 1;
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
    int verified_track02_request;
    Theron_V1_World staged_world;

    if (!world) {
        return 0;
    }
    if (dungeon_id < THERON_DUNGEON_1_HALL_OF_RECORDS ||
        dungeon_id > THERON_DUNGEON_COUNT) {
        dungeon_id = THERON_DUNGEON_1_HALL_OF_RECORDS;
    }
    dungeon_index = (int)dungeon_id - 1;
    verified_track02_request =
        theron_v1_startup_runtime_has_verified_track02_request(
            hucard_rom, hucard_rom_size, md5_hex);
    /* A real Track 02 route may validate its level and object records in
     * several steps.  Keep every tentative write in a candidate world so a
     * rejected later object/media check cannot leak a partial dungeon into
     * the live runtime. */
    staged_world = *world;
    if (theron_v1_startup_runtime_try_track02_initial_level(&staged_world,
                                                            hucard_rom,
                                                            hucard_rom_size,
                                                            md5_hex,
                                                            dungeon_id,
                                                            receipt,
                                                            receipt_cap)) {
        *world = staged_world;
        return 1;
    }
    if (verified_track02_request) {
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

static void theron_v1_startup_runtime_entry_apply_receipt_copy_media_spans(
    Theron_V1StartupRuntimeEntryApplyReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *media_receipt) {

    if (!out_receipt || !media_receipt) {
        return;
    }
    out_receipt->track02_media_title_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_title_first_raw_offset;
    out_receipt->track02_media_title_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_title_last_raw_offset;
    out_receipt->track02_media_title_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_title_first_user_data_offset;
    out_receipt->track02_media_title_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_title_last_user_data_offset;
    out_receipt->track02_media_stage_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_first_raw_offset;
    out_receipt->track02_media_stage_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_last_raw_offset;
    out_receipt->track02_media_stage_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_first_user_data_offset;
    out_receipt->track02_media_stage_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_last_user_data_offset;
    out_receipt->track02_media_soul_room_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_first_raw_offset;
    out_receipt->track02_media_soul_room_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_last_raw_offset;
    out_receipt->track02_media_soul_room_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_first_user_data_offset;
    out_receipt->track02_media_soul_room_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_last_user_data_offset;
    out_receipt->track02_media_forcefield_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_first_raw_offset;
    out_receipt->track02_media_forcefield_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_last_raw_offset;
    out_receipt->track02_media_forcefield_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_first_user_data_offset;
    out_receipt->track02_media_forcefield_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_last_user_data_offset;
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
    out_receipt->track02_media_route = apply_receipt->track02_media_route;
    out_receipt->track02_media_route_mask =
        apply_receipt->track02_media_route_mask;
    out_receipt->track02_media_checksum =
        apply_receipt->track02_media_checksum;
    out_receipt->track02_media_title_first_raw_offset =
        apply_receipt->track02_media_title_first_raw_offset;
    out_receipt->track02_media_title_last_raw_offset =
        apply_receipt->track02_media_title_last_raw_offset;
    out_receipt->track02_media_title_first_user_data_offset =
        apply_receipt->track02_media_title_first_user_data_offset;
    out_receipt->track02_media_title_last_user_data_offset =
        apply_receipt->track02_media_title_last_user_data_offset;
    out_receipt->track02_media_stage_first_raw_offset =
        apply_receipt->track02_media_stage_first_raw_offset;
    out_receipt->track02_media_stage_last_raw_offset =
        apply_receipt->track02_media_stage_last_raw_offset;
    out_receipt->track02_media_stage_first_user_data_offset =
        apply_receipt->track02_media_stage_first_user_data_offset;
    out_receipt->track02_media_stage_last_user_data_offset =
        apply_receipt->track02_media_stage_last_user_data_offset;
    out_receipt->track02_media_soul_room_first_raw_offset =
        apply_receipt->track02_media_soul_room_first_raw_offset;
    out_receipt->track02_media_soul_room_last_raw_offset =
        apply_receipt->track02_media_soul_room_last_raw_offset;
    out_receipt->track02_media_soul_room_first_user_data_offset =
        apply_receipt->track02_media_soul_room_first_user_data_offset;
    out_receipt->track02_media_soul_room_last_user_data_offset =
        apply_receipt->track02_media_soul_room_last_user_data_offset;
    out_receipt->track02_media_forcefield_first_raw_offset =
        apply_receipt->track02_media_forcefield_first_raw_offset;
    out_receipt->track02_media_forcefield_last_raw_offset =
        apply_receipt->track02_media_forcefield_last_raw_offset;
    out_receipt->track02_media_forcefield_first_user_data_offset =
        apply_receipt->track02_media_forcefield_first_user_data_offset;
    out_receipt->track02_media_forcefield_last_user_data_offset =
        apply_receipt->track02_media_forcefield_last_user_data_offset;
    out_receipt->object_table_blocked_anchor_mask =
        apply_receipt->object_table_blocked_anchor_mask;
    out_receipt->object_table_blocked_anchor_count =
        apply_receipt->object_table_blocked_anchor_count;
    out_receipt->nonstartup_level_blocked_anchor_mask =
        apply_receipt->nonstartup_level_blocked_anchor_mask;
    out_receipt->nonstartup_level_blocked_anchor_count =
        apply_receipt->nonstartup_level_blocked_anchor_count;
    out_receipt->startup_level_blocked_anchor_mask =
        apply_receipt->startup_level_blocked_anchor_mask;
    out_receipt->startup_level_blocked_anchor_count =
        apply_receipt->startup_level_blocked_anchor_count;
    theron_v1_startup_copy_object_anchor_receipt(
        out_receipt->object_table_anchor_binding_status,
        out_receipt->object_table_anchor_hash,
        apply_receipt->object_table_anchor_binding_status,
        apply_receipt->object_table_anchor_hash);
    theron_v1_startup_copy_level_anchor_receipt_u64(
        out_receipt->startup_level_anchor_status,
        out_receipt->startup_level_anchor_raw_offsets,
        out_receipt->startup_level_anchor_user_data_offsets,
        out_receipt->startup_level_anchor_user_data_valid,
        out_receipt->startup_level_anchor_width,
        out_receipt->startup_level_anchor_height,
        out_receipt->startup_level_anchor_seed,
        out_receipt->startup_level_anchor_level_index,
        apply_receipt->startup_level_anchor_status,
        apply_receipt->startup_level_anchor_raw_offsets,
        apply_receipt->startup_level_anchor_user_data_offsets,
        apply_receipt->startup_level_anchor_user_data_valid,
        apply_receipt->startup_level_anchor_width,
        apply_receipt->startup_level_anchor_height,
        apply_receipt->startup_level_anchor_seed,
        apply_receipt->startup_level_anchor_level_index);
    out_receipt->log_first_line = apply_receipt->log_first_line;
    out_receipt->log_receipt = apply_receipt->log_receipt ? 1 : 0;
    return 1;
}

static void theron_v1_startup_runtime_entry_capture_result(
    Theron_V1_World *world,
    const char *runtime_receipt,
    int verified_track02_request,
    int semantic_handoff,
    const Theron_StartupMediaStateReceipt *media_receipt,
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
    if (verified_track02_request && media_receipt &&
        theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            media_receipt)) {
        out_result->track02_media_route = 1;
        out_result->track02_media = *media_receipt;
        if (world->runtime_media.identity.ready) {
            out_result->track02_media.runtime_media_identity =
                world->runtime_media.identity;
        }
        if (world->runtime_media.restored &&
            theron_v1_world_runtime_media_select_level_bank(
                world,
                world->current_level > 0
                    ? THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL
                    : THERON_RUNTIME_LEVEL_BANK_STARTUP_FORCEFIELD,
                (Theron_DungeonID)world->current_dungeon,
                world->current_level)) {
            out_result->track02_level_bank = world->runtime_media.level_bank;
        }
    }
    if (!semantic_handoff && verified_track02_request && media_receipt &&
        theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            media_receipt)) {
        out_result->fallback_visuals_blocked = 1;
    }
    if (semantic_handoff) {
        out_result->runtime_level_source =
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC;
        out_result->track02_semantic_handoff = 1;
    } else if (verified_track02_request && media_receipt &&
               theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
                   media_receipt)) {
        out_result->runtime_level_source =
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_MEDIA;
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
    const Theron_StartupMediaStateReceipt *media_receipt,
    Theron_V1StartupRuntimeEntryResult *out_result) {

    if (!out_result) {
        return;
    }
    if (verified_track02_request && media_receipt &&
        theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            media_receipt)) {
        out_result->track02_media_route = 1;
        out_result->track02_media = *media_receipt;
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

/* A verified Track 02 may legitimately bind the initial level while later
 * object-table evidence remains unbound.  Preserve that bound level route,
 * but make the receipt fail closed for every synthetic continuation surface.
 * ReDMCSB has no Theron's Quest implementation; this is constrained by the
 * hash/anchor-gated Track 02 receipts in theron_v1_track02.c. */
static int theron_v1_startup_runtime_entry_has_not_bound_object_route(
    const Theron_V1StartupRuntimeEntryResult *result) {

    unsigned int i;

    if (!result || !result->object_table_no_fallback_ready ||
        result->object_table_blocked_anchor_mask == 0u) {
        return 0;
    }
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        if ((result->object_table_blocked_anchor_mask & (1u << i)) != 0u &&
            result->object_table_anchor_binding_status[i] ==
                THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND) {
            return 1;
        }
    }
    return 0;
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
    case THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_MEDIA:
        return "track02-media";
    case THERON_V1_STARTUP_RUNTIME_LEVEL_NONE:
    default:
        return "none";
    }
}

void theron_v1_startup_all_dungeon_route_receipt_init(
    Theron_V1StartupAllDungeonRouteReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

static void theron_v1_startup_copy_object_anchor_receipt(
    int out_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint32_t out_hash[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint32_t in_hash[THERON_TRACK02_MAX_BANK_ANCHORS]) {

    memcpy(out_status,
           in_status,
           sizeof(int) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_hash,
           in_hash,
           sizeof(uint32_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
}

static void theron_v1_startup_copy_level_anchor_receipt(
    int out_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint64_t out_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint64_t out_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    int out_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_width[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_height[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint32_t out_seed[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_level_index[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    const size_t in_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const size_t in_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_width[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_height[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint32_t in_seed[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_level_index[THERON_TRACK02_MAX_BANK_ANCHORS]) {

    size_t i;

    memcpy(out_status,
           in_status,
           sizeof(int) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_user_data_valid,
           in_user_data_valid,
           sizeof(int) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_width,
           in_width,
           sizeof(uint16_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_height,
           in_height,
           sizeof(uint16_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_seed,
           in_seed,
           sizeof(uint32_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_level_index,
           in_level_index,
           sizeof(uint16_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    for (i = 0u; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        out_raw_offsets[i] = (uint64_t)in_raw_offsets[i];
        out_user_data_offsets[i] = (uint64_t)in_user_data_offsets[i];
    }
}

static void theron_v1_startup_copy_level_anchor_receipt_u64(
    int out_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint64_t out_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint64_t out_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    int out_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_width[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_height[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint32_t out_seed[THERON_TRACK02_MAX_BANK_ANCHORS],
    uint16_t out_level_index[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_status[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint64_t in_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint64_t in_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const int in_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_width[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_height[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint32_t in_seed[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint16_t in_level_index[THERON_TRACK02_MAX_BANK_ANCHORS]) {

    memcpy(out_status,
           in_status,
           sizeof(int) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_raw_offsets,
           in_raw_offsets,
           sizeof(uint64_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_user_data_offsets,
           in_user_data_offsets,
           sizeof(uint64_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_user_data_valid,
           in_user_data_valid,
           sizeof(int) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_width,
           in_width,
           sizeof(uint16_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_height,
           in_height,
           sizeof(uint16_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_seed,
           in_seed,
           sizeof(uint32_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
    memcpy(out_level_index,
           in_level_index,
           sizeof(uint16_t) * THERON_TRACK02_MAX_BANK_ANCHORS);
}

static int theron_v1_startup_runtime_level_semantics_exact(
    const Theron_V1_World *world,
    Theron_DungeonID dungeon_id) {

    const Theron_V1_Level *level;
    int dungeon_index;
    uint8_t start_square;

    if (!world || dungeon_id < THERON_DUNGEON_1_HALL_OF_RECORDS ||
        dungeon_id > THERON_DUNGEON_COUNT) {
        return 0;
    }
    dungeon_index = (int)dungeon_id - 1;
    if (world->current_dungeon != (int)dungeon_id ||
        world->current_level != 0 ||
        !world->level_loaded[dungeon_index][0]) {
        return 0;
    }
    level = &world->levels[dungeon_index][0];
    if (level->width <= 0 || level->width > THERON_MAX_MAP_SIZE ||
        level->height <= 0 || level->height > THERON_MAX_MAP_SIZE ||
        level->start_x < 0 || level->start_x >= level->width ||
        level->start_y < 0 || level->start_y >= level->height) {
        return 0;
    }
    start_square = level->squares[level->start_y][level->start_x];
    return THERON_SQUARE_IS_PASSABLE(start_square) &&
           world->party.leader_x == level->start_x &&
           world->party.leader_y == level->start_y &&
           world->party.leader_dir == level->start_dir;
}

static int theron_v1_startup_runtime_object_semantics_exact(
    const Theron_V1_World *world,
    Theron_DungeonID dungeon_id) {

    int i;

    if (!world || dungeon_id < THERON_DUNGEON_1_HALL_OF_RECORDS ||
        dungeon_id > THERON_DUNGEON_COUNT || world->object_count < 0 ||
        world->object_count > THERON_MAX_OBJECTS) {
        return 0;
    }
    for (i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        const Theron_V1_Level *level;
        if (object->id <= 0 ||
            object->dungeon_id != (int)dungeon_id ||
            object->level < 0 ||
            object->level >= THERON_MAX_LEVELS_PER_DUNGEON ||
            !world->level_loaded[(int)dungeon_id - 1][object->level]) {
            return 0;
        }
        level = &world->levels[(int)dungeon_id - 1][object->level];
        if (object->x < 0 || object->x >= level->width ||
            object->y < 0 || object->y >= level->height ||
            object->type == THERON_OBJTYPE_NONE) {
            return 0;
        }
    }
    return world->object_count == world->levels[(int)dungeon_id - 1][0].thing_count;
}

static uint32_t theron_v1_startup_runtime_object_route_hash(
    const Theron_V1_World *world,
    Theron_DungeonID dungeon_id) {

    uint32_t hash = 2166136261u;
    int i;

    if (!world) {
        return 0u;
    }
    hash ^= (uint32_t)dungeon_id;
    hash *= 16777619u;
    hash ^= (uint32_t)world->object_count;
    hash *= 16777619u;
    for (i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        hash ^= (uint32_t)object->id;
        hash *= 16777619u;
        hash ^= (uint32_t)object->type;
        hash *= 16777619u;
        hash ^= (uint32_t)((object->level & 0xff) << 16) ^
                (uint32_t)((object->x & 0xff) << 8) ^
                (uint32_t)(object->y & 0xff);
        hash *= 16777619u;
        hash ^= (uint32_t)object->flags;
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

int theron_v1_startup_runtime_capture_all_dungeon_routes(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    const Theron_StartupMediaStateReceipt *media_receipt,
    Theron_V1StartupAllDungeonRouteReceipt *out_receipt) {

    Theron_DungeonID dungeon_id;
    Theron_Track02ObjectTableRouteReceipt object_table_route;
    Theron_Track02LevelRouteReceipt level_route;
    uint32_t hash = 2166136261u;
    uint32_t object_hash = 2166136261u;

    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_all_dungeon_route_receipt_init(out_receipt);
    if (!theron_v1_startup_runtime_has_verified_track02_request(
            hucard_rom, hucard_rom_size, md5_hex) ||
        !media_receipt ||
        !theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            media_receipt)) {
        return 0;
    }

    if (theron_v1_track02_capture_object_table_route_receipt(
            hucard_rom,
            hucard_rom_size,
            md5_hex,
            &object_table_route) &&
        object_table_route.blocked_for_missing_real_object_evidence &&
        !object_table_route.fallback_visuals_allowed) {
        out_receipt->no_fallback_semantic_role_mask |=
            object_table_route.semantic_role_mask;
        out_receipt->object_table_no_fallback_ready = 1;
        out_receipt->object_table_blocked_anchor_mask =
            object_table_route.object_table_blocked_anchor_mask;
        out_receipt->object_table_blocked_anchor_count =
            (int)object_table_route.object_table_blocked_anchor_count;
        out_receipt->object_table_route_hash =
            object_table_route.route_hash;
        theron_v1_startup_copy_object_anchor_receipt(
            out_receipt->object_table_anchor_binding_status,
            out_receipt->object_table_anchor_hash,
            object_table_route.object_table_anchor_binding_status,
            object_table_route.object_table_anchor_hash);
        hash ^= object_table_route.route_hash;
        hash *= 16777619u;
    }
    if (theron_v1_track02_capture_level_route_receipt(hucard_rom,
                                                       hucard_rom_size,
                                                       md5_hex,
                                                       &level_route) &&
        level_route.blocked_for_missing_nonstartup_level_evidence &&
        !level_route.fallback_visuals_allowed) {
        out_receipt->no_fallback_semantic_role_mask |=
            level_route.semantic_role_mask;
        out_receipt->nonstartup_level_no_fallback_ready = 1;
        out_receipt->nonstartup_level_blocked_anchor_mask =
            level_route.nonstartup_level_blocked_anchor_mask;
        out_receipt->nonstartup_level_blocked_anchor_count =
            (int)level_route.nonstartup_level_blocked_anchor_count;
        out_receipt->startup_level_blocked_anchor_mask =
            level_route.startup_level_blocked_anchor_mask;
        out_receipt->startup_level_blocked_anchor_count =
            (int)level_route.startup_level_blocked_anchor_count;
        out_receipt->level_route_hash = level_route.route_hash;
        theron_v1_startup_copy_level_anchor_receipt(
            out_receipt->startup_level_anchor_status,
            out_receipt->startup_level_anchor_raw_offsets,
            out_receipt->startup_level_anchor_user_data_offsets,
            out_receipt->startup_level_anchor_user_data_valid,
            out_receipt->startup_level_anchor_width,
            out_receipt->startup_level_anchor_height,
            out_receipt->startup_level_anchor_seed,
            out_receipt->startup_level_anchor_level_index,
            level_route.startup_level_anchor_status,
            level_route.startup_level_anchor_raw_offsets,
            level_route.startup_level_anchor_user_data_offsets,
            level_route.startup_level_anchor_user_data_valid,
            level_route.startup_level_anchor_width,
            level_route.startup_level_anchor_height,
            level_route.startup_level_anchor_seed,
            level_route.startup_level_anchor_level_index);
        hash ^= level_route.route_hash;
        hash *= 16777619u;
    }

    for (dungeon_id = THERON_DUNGEON_1_HALL_OF_RECORDS;
         dungeon_id <= THERON_DUNGEON_COUNT;
         dungeon_id = (Theron_DungeonID)((int)dungeon_id + 1)) {
        Theron_V1_World world;
        char receipt[320];
        int level_ok;
        int object_ok;

        theron_v1_world_init(&world);
        receipt[0] = '\0';
        if (!theron_v1_startup_media_bind_runtime_receipt(
                &world, media_receipt) ||
            !theron_v1_startup_runtime_try_track02_initial_level(
                &world,
                hucard_rom,
                hucard_rom_size,
                md5_hex,
                dungeon_id,
                receipt,
                sizeof(receipt)) ||
            !theron_v1_world_runtime_media_select_level_bank(
                &world,
                THERON_RUNTIME_LEVEL_BANK_STARTUP_FORCEFIELD,
                dungeon_id,
                0)) {
            return 0;
        }
        level_ok =
            theron_v1_startup_runtime_level_semantics_exact(&world,
                                                            dungeon_id);
        object_ok =
            theron_v1_startup_runtime_object_semantics_exact(&world,
                                                             dungeon_id);
        if (!level_ok || !object_ok) {
            return 0;
        }
        out_receipt->level_banks[(int)dungeon_id - 1] =
            world.runtime_media.level_bank;
        out_receipt->object_counts[(int)dungeon_id - 1] = world.object_count;
        ++out_receipt->object_capture_count;
        out_receipt->object_capture_mask |= 1u << ((unsigned)dungeon_id - 1u);
        out_receipt->object_count_total += world.object_count;
        out_receipt->dungeon_mask |= 1u << ((unsigned)dungeon_id - 1u);
        ++out_receipt->capture_count;
        ++out_receipt->semantic_level_count;
        hash ^= (uint32_t)dungeon_id;
        hash *= 16777619u;
        hash ^= world.runtime_media.level_bank.surface_checksum;
        hash *= 16777619u;
        hash ^= (uint32_t)world.levels[(int)dungeon_id - 1][0].start_x << 16;
        hash ^= (uint32_t)world.levels[(int)dungeon_id - 1][0].start_y;
        hash *= 16777619u;
        object_hash ^= theron_v1_startup_runtime_object_route_hash(
            &world, dungeon_id);
        object_hash *= 16777619u;
    }

    out_receipt->exact_level_semantics_ready =
        out_receipt->semantic_level_count == THERON_DUNGEON_COUNT;
    out_receipt->exact_object_semantics_ready =
        out_receipt->object_capture_count == THERON_DUNGEON_COUNT &&
        out_receipt->object_capture_mask ==
            ((1u << THERON_DUNGEON_COUNT) - 1u);
    out_receipt->real_data_capture_ready =
        out_receipt->capture_count == THERON_DUNGEON_COUNT &&
        out_receipt->dungeon_mask ==
            ((1u << THERON_DUNGEON_COUNT) - 1u) &&
        out_receipt->object_table_no_fallback_ready &&
        out_receipt->nonstartup_level_no_fallback_ready &&
        out_receipt->exact_level_semantics_ready &&
        out_receipt->exact_object_semantics_ready;
    out_receipt->route_hash = hash;
    out_receipt->object_route_hash = object_hash;
    out_receipt->valid = out_receipt->real_data_capture_ready;
    return out_receipt->valid;
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
    Theron_StartupMediaStateReceipt media_receipt;

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
    theron_v1_startup_media_capture_track02_state_receipt(
        request->hucard_rom, request->hucard_rom_size, request->md5_hex,
        &media_receipt);
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
                                                              &media_receipt,
                                                              out_result);
        return 0;
    }

    theron_v1_startup_runtime_entry_capture_result(world,
                                                   receipt,
                                                   verified_track02_request,
                                                   verified_track02_request,
                                                   &media_receipt,
                                                   out_result);
    if (out_result) {
        Theron_V1StartupAllDungeonRouteReceipt all_routes;
        if (theron_v1_startup_runtime_capture_all_dungeon_routes(
                request->hucard_rom,
                request->hucard_rom_size,
                request->md5_hex,
                &media_receipt,
                &all_routes)) {
            out_result->all_dungeon_real_data_capture_ready =
                all_routes.real_data_capture_ready;
            out_result->all_dungeon_capture_count = all_routes.capture_count;
            out_result->all_dungeon_capture_mask = all_routes.dungeon_mask;
            out_result->exact_level_semantics_ready =
                all_routes.exact_level_semantics_ready;
            out_result->exact_object_semantics_ready =
                all_routes.exact_object_semantics_ready;
            out_result->no_fallback_semantic_role_mask =
                all_routes.no_fallback_semantic_role_mask;
            out_result->object_table_no_fallback_ready =
                all_routes.object_table_no_fallback_ready;
            out_result->object_table_blocked_anchor_mask =
                all_routes.object_table_blocked_anchor_mask;
            out_result->object_table_blocked_anchor_count =
                all_routes.object_table_blocked_anchor_count;
            out_result->nonstartup_level_no_fallback_ready =
                all_routes.nonstartup_level_no_fallback_ready;
            out_result->nonstartup_level_blocked_anchor_mask =
                all_routes.nonstartup_level_blocked_anchor_mask;
            out_result->nonstartup_level_blocked_anchor_count =
                all_routes.nonstartup_level_blocked_anchor_count;
            out_result->startup_level_blocked_anchor_mask =
                all_routes.startup_level_blocked_anchor_mask;
            out_result->startup_level_blocked_anchor_count =
                all_routes.startup_level_blocked_anchor_count;
            out_result->object_table_route_hash =
                all_routes.object_table_route_hash;
            out_result->level_route_hash = all_routes.level_route_hash;
            theron_v1_startup_copy_object_anchor_receipt(
                out_result->object_table_anchor_binding_status,
                out_result->object_table_anchor_hash,
                all_routes.object_table_anchor_binding_status,
                all_routes.object_table_anchor_hash);
            theron_v1_startup_copy_level_anchor_receipt_u64(
                out_result->startup_level_anchor_status,
                out_result->startup_level_anchor_raw_offsets,
                out_result->startup_level_anchor_user_data_offsets,
                out_result->startup_level_anchor_user_data_valid,
                out_result->startup_level_anchor_width,
                out_result->startup_level_anchor_height,
                out_result->startup_level_anchor_seed,
                out_result->startup_level_anchor_level_index,
                all_routes.startup_level_anchor_status,
                all_routes.startup_level_anchor_raw_offsets,
                all_routes.startup_level_anchor_user_data_offsets,
                all_routes.startup_level_anchor_user_data_valid,
                all_routes.startup_level_anchor_width,
                all_routes.startup_level_anchor_height,
                all_routes.startup_level_anchor_seed,
                all_routes.startup_level_anchor_level_index);
            if (theron_v1_startup_runtime_entry_has_not_bound_object_route(
                    out_result)) {
                out_result->fallback_visuals_blocked = 1;
            }
        }
    }
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
    out_receipt->track02_media_route = result->track02_media_route;
    out_receipt->track02_media_route_mask =
        result->track02_media.startup_bitmap_atlas_route_mask;
    out_receipt->track02_media_checksum =
        result->track02_media.startup_bitmap_atlas_checksum;
    theron_v1_startup_runtime_entry_apply_receipt_copy_media_spans(
        out_receipt,
        &result->track02_media);
    out_receipt->fallback_visuals_blocked =
        result->fallback_visuals_blocked;
    out_receipt->structured_runtime_route =
        result->structured_runtime_route;
    out_receipt->runtime_receipt_text_route =
        result->runtime_receipt_text_route;
    out_receipt->track02_level_bank = result->track02_level_bank;
    out_receipt->all_dungeon_real_data_capture_ready =
        result->all_dungeon_real_data_capture_ready;
    out_receipt->all_dungeon_capture_count =
        result->all_dungeon_capture_count;
    out_receipt->all_dungeon_capture_mask =
        result->all_dungeon_capture_mask;
    out_receipt->exact_level_semantics_ready =
        result->exact_level_semantics_ready;
    out_receipt->exact_object_semantics_ready =
        result->exact_object_semantics_ready;
    out_receipt->no_fallback_semantic_role_mask =
        result->no_fallback_semantic_role_mask;
    out_receipt->object_table_no_fallback_ready =
        result->object_table_no_fallback_ready;
    out_receipt->object_table_blocked_anchor_mask =
        result->object_table_blocked_anchor_mask;
    out_receipt->object_table_blocked_anchor_count =
        result->object_table_blocked_anchor_count;
    out_receipt->nonstartup_level_no_fallback_ready =
        result->nonstartup_level_no_fallback_ready;
    out_receipt->nonstartup_level_blocked_anchor_mask =
        result->nonstartup_level_blocked_anchor_mask;
    out_receipt->nonstartup_level_blocked_anchor_count =
        result->nonstartup_level_blocked_anchor_count;
    out_receipt->startup_level_blocked_anchor_mask =
        result->startup_level_blocked_anchor_mask;
    out_receipt->startup_level_blocked_anchor_count =
        result->startup_level_blocked_anchor_count;
    out_receipt->object_table_route_hash = result->object_table_route_hash;
    out_receipt->level_route_hash = result->level_route_hash;
    theron_v1_startup_copy_object_anchor_receipt(
        out_receipt->object_table_anchor_binding_status,
        out_receipt->object_table_anchor_hash,
        result->object_table_anchor_binding_status,
        result->object_table_anchor_hash);
    theron_v1_startup_copy_level_anchor_receipt_u64(
        out_receipt->startup_level_anchor_status,
        out_receipt->startup_level_anchor_raw_offsets,
        out_receipt->startup_level_anchor_user_data_offsets,
        out_receipt->startup_level_anchor_user_data_valid,
        out_receipt->startup_level_anchor_width,
        out_receipt->startup_level_anchor_height,
        out_receipt->startup_level_anchor_seed,
        out_receipt->startup_level_anchor_level_index,
        result->startup_level_anchor_status,
        result->startup_level_anchor_raw_offsets,
        result->startup_level_anchor_user_data_offsets,
        result->startup_level_anchor_user_data_valid,
        result->startup_level_anchor_width,
        result->startup_level_anchor_height,
        result->startup_level_anchor_seed,
        result->startup_level_anchor_level_index);
    if (runtime_receipt && runtime_receipt[0]) {
        snprintf(out_receipt->inspect_detail,
                 sizeof(out_receipt->inspect_detail),
                 "%s route=%s semantic=%d fallback_blocked=%d structured=%d text_route=%d no_fallback_roles=0x%x object_block=0x%x level_block=0x%x",
                 runtime_receipt,
                 theron_v1_startup_runtime_level_source_name(
                     result->runtime_level_source),
                 result->track02_semantic_handoff,
                 result->fallback_visuals_blocked,
                 result->structured_runtime_route,
                 result->runtime_receipt_text_route,
                 result->no_fallback_semantic_role_mask,
                 result->object_table_blocked_anchor_mask,
                 result->nonstartup_level_blocked_anchor_mask);
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
        out_receipt->track02_media_route = result->track02_media_route;
        out_receipt->track02_media_route_mask =
            result->track02_media.startup_bitmap_atlas_route_mask;
        out_receipt->track02_media_checksum =
            result->track02_media.startup_bitmap_atlas_checksum;
        theron_v1_startup_runtime_entry_apply_receipt_copy_media_spans(
            out_receipt,
            &result->track02_media);
        out_receipt->fallback_visuals_blocked =
            result->fallback_visuals_blocked;
        out_receipt->structured_runtime_route =
            result->structured_runtime_route;
        out_receipt->runtime_receipt_text_route =
            result->runtime_receipt_text_route;
        out_receipt->all_dungeon_real_data_capture_ready =
            result->all_dungeon_real_data_capture_ready;
        out_receipt->all_dungeon_capture_count =
            result->all_dungeon_capture_count;
        out_receipt->all_dungeon_capture_mask =
            result->all_dungeon_capture_mask;
        out_receipt->exact_level_semantics_ready =
            result->exact_level_semantics_ready;
        out_receipt->exact_object_semantics_ready =
            result->exact_object_semantics_ready;
        out_receipt->no_fallback_semantic_role_mask =
            result->no_fallback_semantic_role_mask;
        out_receipt->object_table_no_fallback_ready =
            result->object_table_no_fallback_ready;
        out_receipt->object_table_blocked_anchor_mask =
            result->object_table_blocked_anchor_mask;
        out_receipt->object_table_blocked_anchor_count =
            result->object_table_blocked_anchor_count;
        out_receipt->nonstartup_level_no_fallback_ready =
            result->nonstartup_level_no_fallback_ready;
        out_receipt->nonstartup_level_blocked_anchor_mask =
            result->nonstartup_level_blocked_anchor_mask;
        out_receipt->nonstartup_level_blocked_anchor_count =
            result->nonstartup_level_blocked_anchor_count;
        out_receipt->startup_level_blocked_anchor_mask =
            result->startup_level_blocked_anchor_mask;
        out_receipt->startup_level_blocked_anchor_count =
            result->startup_level_blocked_anchor_count;
        out_receipt->object_table_route_hash =
            result->object_table_route_hash;
        out_receipt->level_route_hash = result->level_route_hash;
        theron_v1_startup_copy_object_anchor_receipt(
            out_receipt->object_table_anchor_binding_status,
            out_receipt->object_table_anchor_hash,
            result->object_table_anchor_binding_status,
            result->object_table_anchor_hash);
        theron_v1_startup_copy_level_anchor_receipt_u64(
            out_receipt->startup_level_anchor_status,
            out_receipt->startup_level_anchor_raw_offsets,
            out_receipt->startup_level_anchor_user_data_offsets,
            out_receipt->startup_level_anchor_user_data_valid,
            out_receipt->startup_level_anchor_width,
            out_receipt->startup_level_anchor_height,
            out_receipt->startup_level_anchor_seed,
            out_receipt->startup_level_anchor_level_index,
            result->startup_level_anchor_status,
            result->startup_level_anchor_raw_offsets,
            result->startup_level_anchor_user_data_offsets,
            result->startup_level_anchor_user_data_valid,
            result->startup_level_anchor_width,
            result->startup_level_anchor_height,
            result->startup_level_anchor_seed,
            result->startup_level_anchor_level_index);
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
    out_receipt->runtime_structured_route =
        result->structured_runtime_route;
    out_receipt->runtime_receipt_text_route =
        result->runtime_receipt_text_route;
    out_receipt->runtime_track02_media_route =
        result->track02_media_route;
    out_receipt->runtime_track02_media_route_mask =
        result->track02_media.startup_bitmap_atlas_route_mask;
    out_receipt->runtime_track02_media_checksum =
        result->track02_media.startup_bitmap_atlas_checksum;
    out_receipt->runtime_track02_media_title_first_raw_offset =
        (uint64_t)result->track02_media.startup_bitmap_title_first_raw_offset;
    out_receipt->runtime_track02_media_title_last_raw_offset =
        (uint64_t)result->track02_media.startup_bitmap_title_last_raw_offset;
    out_receipt->runtime_track02_media_title_first_user_data_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_title_first_user_data_offset;
    out_receipt->runtime_track02_media_title_last_user_data_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_title_last_user_data_offset;
    out_receipt->runtime_track02_media_stage_first_raw_offset =
        (uint64_t)result->track02_media.startup_bitmap_stage_first_raw_offset;
    out_receipt->runtime_track02_media_stage_last_raw_offset =
        (uint64_t)result->track02_media.startup_bitmap_stage_last_raw_offset;
    out_receipt->runtime_track02_media_stage_first_user_data_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_stage_first_user_data_offset;
    out_receipt->runtime_track02_media_stage_last_user_data_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_stage_last_user_data_offset;
    out_receipt->runtime_track02_media_soul_room_first_raw_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_soul_room_first_raw_offset;
    out_receipt->runtime_track02_media_soul_room_last_raw_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_soul_room_last_raw_offset;
    out_receipt->runtime_track02_media_soul_room_first_user_data_offset =
        (uint64_t)result->track02_media
            .startup_bitmap_soul_room_first_user_data_offset;
    out_receipt->runtime_track02_media_soul_room_last_user_data_offset =
        (uint64_t)result->track02_media
            .startup_bitmap_soul_room_last_user_data_offset;
    out_receipt->runtime_track02_media_forcefield_first_raw_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_forcefield_first_raw_offset;
    out_receipt->runtime_track02_media_forcefield_last_raw_offset =
        (uint64_t)
            result->track02_media.startup_bitmap_forcefield_last_raw_offset;
    out_receipt->runtime_track02_media_forcefield_first_user_data_offset =
        (uint64_t)result->track02_media
            .startup_bitmap_forcefield_first_user_data_offset;
    out_receipt->runtime_track02_media_forcefield_last_user_data_offset =
        (uint64_t)result->track02_media
            .startup_bitmap_forcefield_last_user_data_offset;
    out_receipt->runtime_object_table_blocked_anchor_mask =
        result->object_table_blocked_anchor_mask;
    out_receipt->runtime_object_table_blocked_anchor_count =
        result->object_table_blocked_anchor_count;
    out_receipt->runtime_nonstartup_level_blocked_anchor_mask =
        result->nonstartup_level_blocked_anchor_mask;
    out_receipt->runtime_nonstartup_level_blocked_anchor_count =
        result->nonstartup_level_blocked_anchor_count;
    out_receipt->runtime_startup_level_blocked_anchor_mask =
        result->startup_level_blocked_anchor_mask;
    out_receipt->runtime_startup_level_blocked_anchor_count =
        result->startup_level_blocked_anchor_count;
    theron_v1_startup_copy_object_anchor_receipt(
        out_receipt->runtime_object_table_anchor_binding_status,
        out_receipt->runtime_object_table_anchor_hash,
        result->object_table_anchor_binding_status,
        result->object_table_anchor_hash);
    theron_v1_startup_copy_level_anchor_receipt_u64(
        out_receipt->runtime_startup_level_anchor_status,
        out_receipt->runtime_startup_level_anchor_raw_offsets,
        out_receipt->runtime_startup_level_anchor_user_data_offsets,
        out_receipt->runtime_startup_level_anchor_user_data_valid,
        out_receipt->runtime_startup_level_anchor_width,
        out_receipt->runtime_startup_level_anchor_height,
        out_receipt->runtime_startup_level_anchor_seed,
        out_receipt->runtime_startup_level_anchor_level_index,
        result->startup_level_anchor_status,
        result->startup_level_anchor_raw_offsets,
        result->startup_level_anchor_user_data_offsets,
        result->startup_level_anchor_user_data_valid,
        result->startup_level_anchor_width,
        result->startup_level_anchor_height,
        result->startup_level_anchor_seed,
        result->startup_level_anchor_level_index);
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
    Theron_StartupMediaStateReceipt media_receipt;

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
    theron_v1_startup_media_capture_track02_state_receipt(
        hucard_rom, hucard_rom_size, md5_hex, &media_receipt);
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
                                                              &media_receipt,
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
                                                   verified_track02_request,
                                                   &media_receipt,
                                                   result);
    if (verified_track02_request) {
        Theron_V1StartupAllDungeonRouteReceipt all_routes;
        if (theron_v1_startup_runtime_capture_all_dungeon_routes(
                hucard_rom,
                hucard_rom_size,
                md5_hex,
                &media_receipt,
                &all_routes)) {
            result->all_dungeon_real_data_capture_ready =
                all_routes.real_data_capture_ready;
            result->all_dungeon_capture_count = all_routes.capture_count;
            result->all_dungeon_capture_mask = all_routes.dungeon_mask;
            result->exact_level_semantics_ready =
                all_routes.exact_level_semantics_ready;
            result->exact_object_semantics_ready =
                all_routes.exact_object_semantics_ready;
            result->no_fallback_semantic_role_mask =
                all_routes.no_fallback_semantic_role_mask;
            result->object_table_no_fallback_ready =
                all_routes.object_table_no_fallback_ready;
            result->object_table_blocked_anchor_mask =
                all_routes.object_table_blocked_anchor_mask;
            result->object_table_blocked_anchor_count =
                all_routes.object_table_blocked_anchor_count;
            result->nonstartup_level_no_fallback_ready =
                all_routes.nonstartup_level_no_fallback_ready;
            result->nonstartup_level_blocked_anchor_mask =
                all_routes.nonstartup_level_blocked_anchor_mask;
            result->nonstartup_level_blocked_anchor_count =
                all_routes.nonstartup_level_blocked_anchor_count;
            result->startup_level_blocked_anchor_mask =
                all_routes.startup_level_blocked_anchor_mask;
            result->startup_level_blocked_anchor_count =
                all_routes.startup_level_blocked_anchor_count;
            result->object_table_route_hash =
                all_routes.object_table_route_hash;
            result->level_route_hash = all_routes.level_route_hash;
            theron_v1_startup_copy_object_anchor_receipt(
                result->object_table_anchor_binding_status,
                result->object_table_anchor_hash,
                all_routes.object_table_anchor_binding_status,
                all_routes.object_table_anchor_hash);
            theron_v1_startup_copy_level_anchor_receipt_u64(
                result->startup_level_anchor_status,
                result->startup_level_anchor_raw_offsets,
                result->startup_level_anchor_user_data_offsets,
                result->startup_level_anchor_user_data_valid,
                result->startup_level_anchor_width,
                result->startup_level_anchor_height,
                result->startup_level_anchor_seed,
                result->startup_level_anchor_level_index,
                all_routes.startup_level_anchor_status,
                all_routes.startup_level_anchor_raw_offsets,
                all_routes.startup_level_anchor_user_data_offsets,
                all_routes.startup_level_anchor_user_data_valid,
                all_routes.startup_level_anchor_width,
                all_routes.startup_level_anchor_height,
                all_routes.startup_level_anchor_seed,
                all_routes.startup_level_anchor_level_index);
            if (theron_v1_startup_runtime_entry_has_not_bound_object_route(
                    result)) {
                result->fallback_visuals_blocked = 1;
            }
        }
    }
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
