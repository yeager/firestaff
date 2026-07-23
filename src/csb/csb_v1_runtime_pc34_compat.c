/*
 * csb_v1_runtime_pc34_compat.c — CSB V1 Runtime Profile Implementation
 *
 * Source-lock anchors:
 *   ENTRANCE.C: F0806_F0806_ENTRANCE_int       (game boot sequence)
 *   ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)
 *   ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics init)
 *   SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful   (header verify)
 *   SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful (header write)
 *   LOADSAVE.C: F0435_STARTEND_LoadGame              (save load)
 *   LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame (save)
 *   DUNGEON.C:  F0237_DUNGEON_DungeonLoad            (hash-verified load)
 *   CASTER.C:   F0211_CASTER_ClearSpellEffects       (spell grid reset at boot)
 *   BugsAndChanges.htm: CHANGE7_29   (new header format: CSBGAME.DAT)
 *   MEDIA529_F20E_F20J: F20E/F21E  (ST save path)
 *   MEDIA332_F20E_F21E_A31E_F31E: CSB C29 key index
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_f0247_launcher_materialization_pc34_compat.h"
#include "csb_v1_f0247_launcher_rng_pc34_compat.h"
#include "csb_v1_f0248_endgame_runtime_pc34_compat.h"
#include "csb_v1_f0248_local_effect_runtime_pc34_compat.h"
#include "csb_v1_f0217_group_lookup_pc34_compat.h"
#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_dungeon_world_pc34_compat.h"
#include "csb_v1_f0243_timeline_door_destruction_pc34_compat.h"
#include "csb_v1_movement_command_step_runtime_pc34_compat.h"
#include "csb_v1_teleporter_rotation_runtime_pc34_compat.h"
#include "asset_find_by_hash.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "csb_v1_utility_flow_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "dm1_v1_creature_render_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0493_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_runtime_dynamics_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <sys/stat.h>

/* ── Known CSB hashes ──────────────────────────────────────────────────── */

/*
 * PC 3.4 English Atari ST + Amiga: same dungeon hash.
 * All CSB platforms share the same dungeon.dat content — only
 * graphics/assets vary by platform.
 */
static const char *const g_csb_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",
    NULL
};

static int csb_v1_runtime_locate_appended_expool_record_internal(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size);
static int csb_v1_runtime_replace_appended_expool_record_internal(
    CSB_V1_RuntimeProfile *candidate,
    uint32_t record_id,
    const uint8_t *payload,
    size_t payload_size);
static uint8_t *csb_v1_runtime_mutable_thing_record(
    CSB_V1_DungeonData *dungeon, uint16_t thing, int *out_type,
    int *out_size);
static int csb_v1_runtime_stage_csbwin_dsa_tracing(
    CSB_V1_RuntimeProfile *candidate);
static int csb_v1_runtime_stage_csbwin_global_variables(
    CSB_V1_RuntimeProfile *candidate);
static int csb_v1_runtime_stage_csbwin_overlay_palette(
    CSB_V1_RuntimeProfile *candidate);
static int csb_v1_runtime_write_csbwin_global_variables(
    CSB_V1_RuntimeProfile *candidate);
static int csb_v1_runtime_stage_csbwin_save_policy(
    CSB_V1_RuntimeProfile *candidate);
static uint32_t csb_v1_runtime_fnv1a32(const uint8_t *bytes, size_t size);
static uint32_t csb_v1_runtime_read_le32(const uint8_t *bytes);
static int csb_v1_runtime_dsa_get_wing_talents(void *user,
                                               uint16_t fingerprint,
                                               uint32_t *out_talents);
static int csb_v1_runtime_dsa_has_wing_character(void *user,
                                                  uint16_t fingerprint);
static int csb_v1_runtime_dsa_set_wing_talents(void *user,
                                               uint16_t fingerprint,
                                               uint32_t talents);
static void csb_v1_runtime_add_party_steal_skill_experience(
    CSB_V1_RuntimeProfile *profile,
    int amount);
static void csb_v1_runtime_trigger_remote_sensor_event_after(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell,
    int delay);
static int csb_v1_runtime_append_unmerged_map_timer(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_Event_V1 *event);
static int csb_v1_runtime_square_contains_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int level,
    int map_x,
    int map_y);
static int csb_v1_runtime_dsa_get_skin(void *user,
                                       uint32_t location,
                                       uint8_t *out_skin);
static int csb_v1_runtime_dsa_set_skin(void *user,
                                       uint32_t location,
                                       uint8_t skin);
static int csb_v1_runtime_dsa_get_info(void *user, uint16_t thing,
                                        int *out_selector, int *out_state,
                                        int *out_parameter_a,
                                        int *out_parameter_b);
static int csb_v1_runtime_dsa_get_excell_flags(void *user,
                                                uint32_t location,
                                                uint32_t out_words[8]);
static int csb_v1_runtime_dsa_set_excell_flags(void *user,
                                                uint32_t location,
                                                uint32_t flags);
static int csb_v1_runtime_dsa_get_generator_delay(void *user,
                                                   uint32_t location,
                                                   int *out_delay);
static int csb_v1_runtime_dsa_set_generator_delay(void *user,
                                                   uint32_t location,
                                                   int delay);
static int csb_v1_runtime_dsa_get_monster_info(void *user,
                                                uint16_t thing,
                                                uint32_t out_values[8]);
static int csb_v1_runtime_dsa_set_monster_info(void *user,
                                                uint16_t thing,
                                                const uint32_t values[8],
                                                uint8_t write_mask);
static int csb_v1_runtime_dsa_get_champion_possession(
    void *user, int champion_index, uint32_t slot_index, int32_t *out_thing);
static int csb_v1_runtime_dsa_get_monster_possession(
    void *user, uint16_t monster_thing, uint32_t possession_index,
    int32_t *out_thing);
static int csb_v1_runtime_dsa_inspect_cells(
    void *user, uint32_t location, uint32_t criteria_mask,
    uint32_t first_cell, uint32_t last_cell, uint32_t *out_result);
static int csb_v1_runtime_dsa_get_thing_type(
    void *user, int32_t thing_index, int32_t *out_type);
static int csb_v1_runtime_dsa_is_carried(
    void *user, int32_t character_selector, int32_t object_selector,
    int32_t *out_result);
static int csb_v1_runtime_dsa_get_level_multiplier(
    void *user, int32_t level, int32_t *out_multiplier);
static int csb_v1_runtime_dsa_get_missile_info(
    void *user, uint16_t thing, uint32_t out_values[4]);
static int csb_v1_runtime_dsa_set_missile_info(
    void *user, uint16_t thing, const uint32_t values[4]);
static int csb_v1_runtime_dsa_get_cell_info(void *user,
                                             uint32_t location,
                                             uint32_t out_values[5]);
static int csb_v1_runtime_dsa_resolve_cell_store(void *user,
                                                  uint32_t location,
                                                  uint32_t expected_room_type);
static int csb_v1_runtime_dsa_set_cell_info(void *user,
                                             uint32_t location,
                                             const uint32_t values[5],
                                             uint8_t write_mask);
static int csb_v1_runtime_dsa_copy_teleporter(
    void *user, uint32_t source_location, uint32_t destination_location);
static int csb_v1_runtime_dsa_queue_switch_action(
    void *user, uint32_t delay, uint32_t action, uint32_t target_location,
    int message_route, uint8_t *out_event_type);
static int csb_v1_runtime_dsa_get_object_property(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t *out_value);
static int csb_v1_runtime_dsa_set_object_property(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t value);
static int csb_v1_runtime_dsa_normalize_object_property(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t input_value, uint32_t *out_value);
static int csb_v1_runtime_dsa_prepare_experience_plus(
    void *user, int32_t character_selector, int32_t skill_number,
    int32_t experience);
static int csb_v1_runtime_dsa_add_experience_plus(
    void *user, int32_t character_selector, int32_t skill_number,
    int32_t experience);
static int csb_v1_runtime_dsa_prepare_cause_poison(
    void *user, int32_t character_selector, int32_t poison_value);
static int csb_v1_runtime_dsa_commit_cause_poison(
    void *user, int32_t character_selector, int32_t poison_value);
static int csb_v1_runtime_dsa_get_mastery(
    void *user, uint32_t champion_index, uint32_t skill_index,
    uint32_t flags, uint32_t *out_mastery);
static int csb_v1_runtime_dsa_get_party_info(
    void *user, uint32_t out_values[12]);
static int csb_v1_runtime_csbwin_chest_weight_from_expool(
    const CSB_V1_RuntimeProfile *profile,
    int *out_weight);
static int csb_v1_runtime_reheapify_live_csbwin_timer_queue(
    CSB_V1_RuntimeProfile *profile);
static int csb_v1_runtime_replace_dispatched_csbwin_timer(
    CSB_V1_RuntimeProfile *profile,
    uint16_t consumed_queue_slot,
    uint16_t consumed_timer_index,
    const CSB_V1_CSBWin512TimerSummary *successor,
    int successor_event_index);
static void csb_v1_runtime_projectile_step(int direction, int *out_dx, int *out_dy);
static int csb_v1_runtime_square_type_from_raw(
    const CSB_V1_DungeonData *dungeon,
    int raw_square);
static uint8_t *csb_v1_runtime_square_byte_ptr(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y,
    int *out_square_type);
static void csb_v1_runtime_schedule_projectile_move_event(
    CSB_V1_RuntimeProfile *profile,
    const struct TimelineEvent_Compat *event);
static void csb_v1_runtime_schedule_explosion_advance_event(
    CSB_V1_RuntimeProfile *profile,
    const struct TimelineEvent_Compat *event);
static int csb_v1_runtime_stat_or_default(
    const CSB_V1_Champion *champion,
    int stat_index,
    int stat_kind);
static uint32_t csb_v1_runtime_creature_attack_seed(
    const CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int creature_type,
    int creature_index,
    int champion_index);
static int csb_v1_runtime_fill_defender_combat_snapshot(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    struct CombatantChampionSnapshot_Compat *out);
static void csb_v1_runtime_mark_champion_dead(
    CSB_V1_RuntimeProfile *profile,
    int champion_index);
static int csb_v1_runtime_location_after_level_change(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int level_delta,
    int *inout_map_x,
    int *inout_map_y,
    int *out_map_index);
static int csb_v1_runtime_stairs_exit_direction(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y);
static int csb_v1_runtime_object_type_from_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing);
static void csb_v1_runtime_trigger_remote_sensor_event(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell);
static void csb_v1_runtime_process_object_floor_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t placed_thing,
    int level,
    int map_x,
    int map_y,
    int add_thing);
static void csb_v1_runtime_process_object_wall_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t placed_thing,
    int level,
    int map_x,
    int map_y,
    int add_thing);
static int csb_v1_runtime_rotate_wall_cell_sensors(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    int cell);
static uint16_t csb_v1_runtime_csbwin_item16_group_thing(
    uint16_t monster_index);

static unsigned short csb_v1_runtime_clamp_u16(int value)
{
    if (value <= 0) return 0u;
    if (value > 65535) return 65535u;
    return (unsigned short)value;
}

static void csb_v1_runtime_pack_printable(unsigned char *dst,
                                          int dst_len,
                                          const char *src,
                                          int src_len)
{
    int i;
    if (!dst || dst_len <= 0) {
        return;
    }
    for (i = 0; i < dst_len; ++i) {
        dst[i] = ' ';
    }
    if (!src || src_len <= 0) {
        return;
    }
    for (i = 0; i < dst_len && i < src_len && src[i] != '\0'; ++i) {
        unsigned char ch = (unsigned char)src[i];
        dst[i] = (ch >= 0x20u && ch <= 0x7eu) ? ch : ' ';
    }
}

static void csb_v1_runtime_copy_stat(struct ChampionStat_Compat *dst,
                                     int current,
                                     int maximum)
{
    unsigned short max_value;
    unsigned int shifted;
    if (!dst) return;
    max_value = csb_v1_runtime_clamp_u16(maximum);
    if (max_value == 0u) {
        max_value = csb_v1_runtime_clamp_u16(current);
    }
    dst->current = csb_v1_runtime_clamp_u16(current);
    dst->maximum = max_value;
    shifted = (unsigned int)max_value << 1;
    dst->shifted = (unsigned short)(shifted > 65535u ? 65535u : shifted);
}

static int csb_v1_runtime_m11_inventory_slot_for_csb_slot(int csb_slot)
{
    if (csb_slot == CSB_V1_SLOT_READY_HAND) return CHAMPION_SLOT_HAND_LEFT;
    if (csb_slot == CSB_V1_SLOT_ACTION_HAND) return CHAMPION_SLOT_ACTION_HAND;
    if (csb_slot >= CSB_V1_SLOT_BELT_1 && csb_slot <= CSB_V1_SLOT_BELT_4) {
        return CHAMPION_SLOT_POUCH_1 + (csb_slot - CSB_V1_SLOT_BELT_1);
    }
    if (csb_slot >= CSB_V1_SLOT_PACK_1 && csb_slot <= CSB_V1_SLOT_PACK_8) {
        return CHAMPION_SLOT_BACKPACK_1 + (csb_slot - CSB_V1_SLOT_PACK_1);
    }
    if (csb_slot >= CSB_V1_SLOT_PACK_9 && csb_slot <= CSB_V1_SLOT_PACK_12) {
        return CHAMPION_SLOT_BACKPACK_9 + (csb_slot - CSB_V1_SLOT_PACK_9);
    }
    return -1;
}

static int csb_v1_runtime_copy_portrait_compat(
    struct ChampionState_Compat *dst,
    const CSB_V1_Champion *src)
{
    int i;
    int compatible_nonzero = 0;
    int wide_nonzero = 0;
    if (!dst || !src) return 0;
    for (i = 0; i < CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT; ++i) {
        if (src->Portrait[i] != 0u) {
            compatible_nonzero = 1;
            break;
        }
    }
    if (compatible_nonzero) {
        /* ReDMCSB: PANEL.C F0354 draws status-box portraits from
         * M516_CHAMPIONS[i].Portrait. Utility Disk CMP import and DM1 save
         * import keep the DM1-compatible 32x29x4bpp packed portrait in the
         * first 464 bytes, so preserve that byte-exact fast path. */
        memcpy(dst->portraitBitmap,
               src->Portrait,
               CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
        dst->portraitBitmapValid = 1;
        return 1;
    }
    for (i = CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
         i < CSB_V1_PORTRAIT_BYTE_COUNT;
         ++i) {
        if (src->Portrait[i] != 0u) {
            wide_nonzero = 1;
            break;
        }
    }
    if (!wide_nonzero) {
        dst->portraitBitmapValid = 0;
        memset(dst->portraitBitmap, 0, sizeof(dst->portraitBitmap));
        return 0;
    }

    /* ReDMCSB: CEDT006.C F7048 copies loaded portraits into the 464-byte
     * on-screen portrait bitmap, and PORTRAIT.C F7251/F7252 convert between
     * Atari ST planar storage and that packed 4bpp HUD bitmap. */
    for (i = 0; i < CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT; ++i) {
        const int base = i * 8;
        unsigned int left = 0u;
        unsigned int right = 0u;
        if (base + 7 < CSB_V1_PORTRAIT_BYTE_COUNT) {
            left = (unsigned int)(src->Portrait[base] |
                                  src->Portrait[base + 1] |
                                  src->Portrait[base + 2] |
                                  src->Portrait[base + 3]);
            right = (unsigned int)(src->Portrait[base + 4] |
                                   src->Portrait[base + 5] |
                                   src->Portrait[base + 6] |
                                   src->Portrait[base + 7]);
        }
        dst->portraitBitmap[i] =
            (unsigned char)(((left & 0x0fu) << 4) | (right & 0x0fu));
    }
    dst->portraitBitmapValid = 1;
    return 1;
}

static int csb_v1_runtime_dsa_get_actuator_payload(
    void *user, uint16_t thing, uint8_t out_payload[6])
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const uint8_t *record;
    int type;
    int size;

    if (!profile || !profile->dungeon_handle || !out_payload) return -1;
    record = csb_v1_dungeon_get_thing_record(profile->dungeon_handle, thing,
                                              &type, NULL, &size);
    if (!record) return -1;
    if (type != CSB_V1_THING_TYPE_ACTUATOR) return 0;
    if (size < 8) return -1;
    memcpy(out_payload, record + 2, 6u);
    return 1;
}

static int csb_v1_runtime_dsa_set_actuator_payload(
    void *user, uint16_t thing, const uint8_t payload[6])
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    uint8_t *record;
    int type;
    int size;

    if (!profile || !profile->dungeon_handle || !payload) return -1;
    record = csb_v1_runtime_mutable_thing_record(profile->dungeon_handle,
                                                  thing, &type, &size);
    if (!record) return -1;
    if (type != CSB_V1_THING_TYPE_ACTUATOR) return 0;
    if (size < 8) return -1;
    memcpy(record + 2, payload, 6u);
    return 1;
}

/* STKOP_Copy is a DB3-to-DB3 transaction.  Re-resolve both Things in the
 * candidate dungeon and reject if the source image drifted after staging. */
static int csb_v1_runtime_dsa_copy_actuator_payload(
    void *user, uint16_t source_thing, uint16_t destination_thing,
    const uint8_t source_payload[6])
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    const uint8_t *source;
    uint8_t *destination;
    int source_type;
    int source_size;
    int destination_type;
    int destination_size;

    if (!profile || !profile->dungeon_handle || !source_payload) return -1;
    source = csb_v1_dungeon_get_thing_record(profile->dungeon_handle,
                                              source_thing, &source_type,
                                              NULL, &source_size);
    destination = csb_v1_runtime_mutable_thing_record(
        profile->dungeon_handle, destination_thing, &destination_type,
        &destination_size);
    if (!source || !destination) return -1;
    if (source_type != CSB_V1_THING_TYPE_ACTUATOR ||
        destination_type != CSB_V1_THING_TYPE_ACTUATOR) return 0;
    if (source_size < 8 || destination_size < 8 ||
        memcmp(source + 2, source_payload, 6u) != 0) return 0;
    memcpy(destination + 2, source_payload, 6u);
    return 1;
}

typedef struct {
    const CSB_V1_RuntimeProfile *profile;
    const CSB_V1_DungeonData *dungeon;
} CSB_V1_RuntimeSkinCacheLookupCtx;

static int csb_v1_runtime_skin_cache_record_lookup(
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size,
    void *user)
{
    const CSB_V1_RuntimeSkinCacheLookupCtx *ctx =
        (const CSB_V1_RuntimeSkinCacheLookupCtx *)user;

    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!ctx) return 0;

    /* CSBWin DSA.cpp SETSKIN writes through Expool. A resumed CSBWin save
     * can therefore carry skin records that supersede the static dungeon
     * DB11 records; use the runtime save tail first, then fall back to the
     * loaded dungeon's original Expool. */
    if (csb_v1_runtime_locate_appended_expool_record_internal(
            ctx->profile, record_id, out_bytes, out_size)) {
        return 1;
    }
    return csb_v1_dungeon_skin_cache_record_lookup(
        record_id,
        out_bytes,
        out_size,
        (void *)ctx->dungeon);
}

/* GRAPHICS.DAT (or CSB.DAT / CSBGRAPH.DAT) MD5 hashes for all known
 * CSB variants — mirrors g_csb_boot_graphics_hashes in csb_v1_boot.c
 * so the runtime path-finder matches the same set of files that the
 * scanner accepts. 2026-06-20: extended the runtime search to be
 * hash-based so files in arbitrary subdirs (e.g. Meynaf FR hard-disk
 * layouts, CSB expansion sets) are found. */
static const char *const g_csb_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611", /* CSB PC 3.4 English GRAPHICS.DAT */
    "ebf6a57af3f27782e358c0490bfd2f2e", /* CSB Atari ST 2.0/2.1 English */
    "e0ce7ac5160ca5540e90cf09ab9fad49", /* CSB Atari ST 2.x hard-disk */
    "291e1bc6803e3dc4b974c60117ca5d68", /* CSB Amiga 3.5 English */
    "cefaddfdf5651df2c91f61b5611a8362", /* CSB Amiga 3.5 Multilanguage */
    NULL
};

/* ── Variant info table ─────────────────────────────────────────────── */

/*
 * CSB variant info.  Platform-specific; game logic is identical.
 * md5_gfx  = GRAPHICS.DAT hash for this variant
 * md5_graf = CSBGRAPH.DAT / CSB.DAT hash (same hash as GRAPHICS.DAT for
 *            the "graphics-only" variants — they lack the overlay archive)
 * md5_dungeon = DUNGEON.DAT hash (shared by all CSB platforms)
 *
 * ReDMCSB COMPILE.H MEDIA tags + CSBWin AssetCache variant mapping.
 */
static const CSB_V1_VariantInfo g_csb_variants[CSB_V1_VARIANT_COUNT] = {
    [CSB_V1_VARIANT_UNKNOWN] = {
        CSB_V1_VARIANT_UNKNOWN,
        "Unknown",
        "",
        "",
        "",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_PC34_EN] = {
        CSB_V1_VARIANT_PC34_EN,
        "PC DOS 3.4 English",
        "MEDIA278:P20JA_P20JB",
        "61fbfd56887c94adc26888a9491c6611",
        "61fbfd56887c94adc26888a9491c6611",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_PC34_MULTI] = {
        CSB_V1_VARIANT_PC34_MULTI,
        "PC DOS 3.4 Multilanguage",
        "MEDIA278:I34E_I34M",
        "cefaddfdf5651df2c91f61b5611a8362",
        "cefaddfdf5651df2c91f61b5611a8362",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST20_EN] = {
        CSB_V1_VARIANT_ST20_EN,
        "Atari ST 2.0 English",
        "MEDIA332:S20E_S21E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST21_EN] = {
        CSB_V1_VARIANT_ST21_EN,
        "Atari ST 2.1 English",
        "MEDIA332:S20E_S21E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_AMIGA35_EN] = {
        CSB_V1_VARIANT_AMIGA35_EN,
        "Amiga 3.5 English",
        "MEDIA529:A35E",
        "291e1bc6803e3dc4b974c60117ca5d68",
        "291e1bc6803e3dc4b974c60117ca5d68",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_AMIGA35_MULTI] = {
        CSB_V1_VARIANT_AMIGA35_MULTI,
        "Amiga 3.5 Multilanguage",
        "MEDIA529:A35M",
        "cefaddfdf5651df2c91f61b5611a8362",
        "cefaddfdf5651df2c91f61b5611a8362",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST_F20J] = {
        CSB_V1_VARIANT_ST_F20J,
        "Atari ST TT (F20J)",
        "MEDIA529:F20J",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST_F20E] = {
        CSB_V1_VARIANT_ST_F20E,
        "Atari ST (F20E)",
        "MEDIA529:F20E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    }
};

_Static_assert(CSB_V1_VARIANT_ST_F20E == CSB_V1_VARIANT_COUNT - 1,
               "CSB_V1_VARIANT_COUNT must match last enum value");

/* ── Platform-specific save dir ────────────────────────────────────── */

#if defined(_WIN32)
#define CSB_PATH_SEP '\\'
#else
#define CSB_PATH_SEP '/'
#endif

static char g_csb_save_dir_buf[512];
static char g_csb_save_path_buf[512];
static int  g_save_dir_init = 0;

static int csb_v1_runtime_first_living_champion(
    const CSB_V1_PartyState *party);

#define CSB_V1_RUNTIME_SAVE_MAGIC   0x46534352u /* FSCR */
#define CSB_V1_RUNTIME_SAVE_VERSION 11u

typedef struct {
    int valid;
    uint16_t group_thing;
    int map_index;
    int map_x;
    int map_y;
    uint8_t delay_fleeing_from_target;
} CSB_V1_RuntimeActiveGroupStateV7;

typedef struct {
    int valid;
    uint16_t group_thing;
    int map_index;
    int map_x;
    int map_y;
    uint8_t cells;
    uint16_t directions;
    int prior_map_x;
    int prior_map_y;
    int home_map_x;
    int home_map_y;
    uint32_t last_move_time;
    uint8_t delay_fleeing_from_target;
} CSB_V1_RuntimeActiveGroupStateV8;

typedef struct {
    int valid;
    uint16_t group_thing;
    int map_index;
    int map_x;
    int map_y;
    uint8_t cells;
    uint16_t directions;
    int prior_map_x;
    int prior_map_y;
    int home_map_x;
    int home_map_y;
    uint32_t last_move_time;
    uint8_t aspect[4];
    uint8_t delay_fleeing_from_target;
} CSB_V1_RuntimeActiveGroupStateV9;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t byte_size;
    int32_t variant_id;
    int32_t difficulty;
    uint32_t dungeon_seed;
    uint16_t dungeon_game_id;
    uint16_t reserved0;
    int32_t current_level;
    int32_t current_world;
    int32_t level_count;
    int32_t world_count;
    int32_t party_x;
    int32_t party_y;
    int32_t party_z;
    int32_t party_dir;
    int32_t champion_count;
    int32_t leader_index;
    int32_t magic_caster_index;
    int32_t party_state_valid;
    int32_t state;
    int32_t paused;
    int32_t victory;
    int32_t game_over;
    uint32_t entrance_map_index;
    uint32_t start_map_index;
    uint64_t game_ticks;
    uint32_t game_time;
    uint64_t total_play_ms;
    uint32_t tick_count;
    struct DM1_EventQueue_V1 timeline_queue;
    struct DM1_TickDispatchResult_V1 last_timeline_dispatch;
    uint32_t timeline_dispatch_count;
    struct Dm1V1InputCommandQueuePc34Compat input_command_queue;
    struct Dm1V1InputQueueProcessResultPc34Compat last_input_dispatch;
    uint32_t input_dispatch_count;
    CSB_V1_ChaosAmbientState chaos_magic;
    CSB_V1_PartyState party_state;
    struct ProjectileList_Compat projectiles;
    struct ExplosionList_Compat explosions;
    int32_t csbwin_header_tail_valid;
    uint8_t csbwin_header_byte22808[132];
    int32_t csbwin_appended_tail_valid;
    uint32_t csbwin_appended_tail_size;
    uint32_t csbwin_appended_tail_preserved_size;
    uint32_t csbwin_appended_tail_fnv1a;
    int32_t csbwin_appended_tail_truncated;
    uint8_t csbwin_appended_tail[CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES];
    uint16_t active_group_state_count;
    uint16_t active_group_state_reserved0;
    CSB_V1_RuntimeActiveGroupState
        active_group_state[CSB_V1_RUNTIME_ACTIVE_GROUP_CAP];
    CsbV1AudioSaveSnapshot audio_snapshot;
} CSB_V1_RuntimeSaveImageV1;

#define CSB_V1_RUNTIME_SAVE_V1_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, projectiles))
#define CSB_V1_RUNTIME_SAVE_V4_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_header_tail_valid))
#define CSB_V1_RUNTIME_SAVE_V5_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_appended_tail_valid))
#define CSB_V1_RUNTIME_SAVE_V6_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, active_group_state_count))
/* Version 7 carried the first active-group side-state table before Cells,
 * Directions, Prior/Home, and LastMoveTime were added.  Keep it loadable as
 * an older image; version 8 preserves the wider table, version 9 adds
 * Aspect[4], and version 10 adds ReDMCSB ActiveGroup.TargetMapX/Y. */
#define CSB_V1_RUNTIME_SAVE_V7_SIZE \
    (CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u + \
     (CSB_V1_RUNTIME_ACTIVE_GROUP_CAP * \
      (uint32_t)sizeof(CSB_V1_RuntimeActiveGroupStateV7)))
#define CSB_V1_RUNTIME_SAVE_V8_SIZE \
    (CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u + \
     (CSB_V1_RUNTIME_ACTIVE_GROUP_CAP * \
      (uint32_t)sizeof(CSB_V1_RuntimeActiveGroupStateV8)))
#define CSB_V1_RUNTIME_SAVE_V9_SIZE \
    (CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u + \
     (CSB_V1_RUNTIME_ACTIVE_GROUP_CAP * \
      (uint32_t)sizeof(CSB_V1_RuntimeActiveGroupStateV9)))
#define CSB_V1_RUNTIME_SAVE_V10_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, audio_snapshot))

_Static_assert(sizeof(CSB_V1_RuntimeActiveGroupStateV7) == 24u,
               "CSB native save v7 active-group entry size drifted");
_Static_assert(sizeof(CSB_V1_RuntimeActiveGroupStateV8) == 48u,
               "CSB native save v8 active-group entry size drifted");
_Static_assert(sizeof(CSB_V1_RuntimeActiveGroupStateV9) == 52u,
               "CSB native save v9 active-group entry size drifted");

static int csb_v1_runtime_first_living_champion(
    const CSB_V1_PartyState *party);
static uint16_t csb_v1_runtime_normalize_leader_hand_thing(uint16_t thing);
static uint16_t csb_v1_runtime_export_leader_hand_thing(
    const CSB_V1_RuntimeProfile *profile);
static int csb_v1_runtime_target_champion_for_adjacent_attack(
    const CSB_V1_RuntimeProfile *profile,
    int attacker_x,
    int attacker_y,
    int creature_cell);
static CSB_V1_RuntimeActiveGroupState *
csb_v1_runtime_active_group_state_for_thing(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing);
static void csb_v1_runtime_set_active_group_target(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int target_x,
    int target_y);

static void csb_v1_init_save_dir(void)
{
    if (g_save_dir_init) return;
    g_save_dir_init = 1;

    if (0) {}
#if defined(_WIN32)
    {
        const char *appdata = getenv("APPDATA");
        const char *base = appdata ? appdata : "C:\\";
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s\\Firestaff\\csb\\saves\\", base);
    }
#elif defined(__APPLE__)
    {
        const char *home = getenv("HOME");
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s/Library/Application Support/Firestaff/csb/saves/",
                home ? home : "");
    }
#else
    {
        const char *home = getenv("HOME");
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s/.local/share/firestaff/csb/saves/",
                home ? home : "");
    }
#endif
}

/* ── Internal MD5 helper ─────────────────────────────────────────────── */

/*
 * Compute MD5 hex of a file.  Returns 0 on success, -1 on error.
 * outHex must be at least 33 bytes.
 * Uses the same MD5 backend as asset_find_by_hash.c.
 */
static int __attribute__((unused)) csb_v1_file_md5_hex (const char *path, char *outHex, size_t hexSize)
{
    /* Use asset_find_by_md5_list internally for file existence + MD5.
     * We only expose the hash computation through asset_status_m12.
     * For runtime use, the hash comes from M12_AssetStatus scan results.
     * This function stubs to the simplest cross-platform approach. */
    (void)path;
    (void)outHex;
    (void)hexSize;
    if (outHex && hexSize >= 33) {
        outHex[0] = '\0';
    }
    return -1;
}

/* ── Difficulty helpers ─────────────────────────────────────────────── */

int csb_v1_runtime_calc_difficulty(int champion_count)
{
    int x100;
    if (champion_count < 1) champion_count = 1;
    if (champion_count > 4) champion_count = 4;
    x100 = CSB_V1_DIFFICULTY_BASE + (champion_count - 1) * CSB_V1_DIFFICULTY_PER_CHAMP;
    return x100;
}

const char *csb_v1_runtime_difficulty_str(int difficulty_x100)
{
    switch (difficulty_x100) {
        case 100: return "Easy (1 champion)";
        case 125: return "Normal (2 champions)";
        case 150: return "Hard (3 champions)";
        case 200: return "Extreme (4 champions)";
        default:  return "Unknown";
    }
}

/* ── Variant diagnostics ────────────────────────────────────────────── */

const char *csb_v1_runtime_variant_name(CSB_V1_VariantId id)
{
    if (id >= 0 && id < CSB_V1_VARIANT_COUNT) {
        return g_csb_variants[id].name;
    }
    return "Unknown";
}

const CSB_V1_VariantInfo *csb_v1_runtime_get_variant_info(CSB_V1_VariantId id)
{
    if (id >= 0 && id < CSB_V1_VARIANT_COUNT) {
        return &g_csb_variants[id];
    }
    return &g_csb_variants[CSB_V1_VARIANT_UNKNOWN];
}

/*
 * Detect variant by matching gfx + dungeon MD5 hashes to known variants.
 * Falls back to UNKNOWN if no hash matches (assets not yet verified).
 * The dungeon hash is the primary filter (all CSB shares same dungeon hash).
 */
int csb_v1_runtime_detect_variant(const char *gfx_path,
                                    const char *dungeon_path,
                                    const char *md5_gfx,
                                    const char *md5_dungeon)
{
    int i;
    (void)gfx_path;  /* gfx_path used only for diagnostics, md5_gfx is the key */
    (void)dungeon_path;  /* same — md5_dungeon is the key */

    if (!md5_dungeon) return CSB_V1_VARIANT_UNKNOWN;
    if (strcmp(md5_dungeon, "6695d2acebce49f95db1d8f3a5c733de") != 0) {
        return CSB_V1_VARIANT_UNKNOWN;
    }

    if (md5_gfx) {
        if (strcmp(md5_gfx, "e0ce7ac5160ca5540e90cf09ab9fad49") == 0) {
            return CSB_V1_VARIANT_ST21_EN;
        }
        for (i = 1; i < CSB_V1_VARIANT_COUNT; i++) {
            if (g_csb_variants[i].md5_gfx[0] != '\0' &&
                strcmp(md5_gfx, g_csb_variants[i].md5_gfx) == 0) {
                return g_csb_variants[i].id;
            }
        }
    }

    return CSB_V1_VARIANT_UNKNOWN;
}

/* ── Asset discovery ────────────────────────────────────────────────── */

/*
 * Search for CSB DUNGEON.DAT by hash.
 * ReDMCSB: DUNGEON.C F0237_DUNGEON_DungeonLoad (hash-gated open).
 */
const char *csb_v1_runtime_find_dungeon(const char *data_dir,
                                         CSB_V1_AssetResult *out_result)
{
    static char found_path[ASSET_PATH_MAX];

    if (!data_dir || !out_result) return NULL;
    memset(out_result, 0, sizeof(*out_result));

    if (!asset_find_by_md5_list(data_dir, g_csb_dungeon_hashes,
                                 found_path, sizeof(found_path), NULL, 8)) {
        return NULL;
    }

    out_result->path = found_path;
    out_result->kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    return found_path;
}

/*
 * Search for CSB graphics archive.
 *
 * ReDMCSB asset search (DISK.C + CSBWin AssetCache):
 *   Floppy platforms: CSB.DAT replaces GRAPHICS.DAT entirely
 *   Data/CD platforms: CSBGRAPH.DAT overlays GRAPHICS.DAT
 *   Hybrid platforms:  CSBGRAPH.DAT takes precedence over GRAPHICS.DAT
 *
 * File search order; we try all known archive names and trust the
 * caller (asset_status_m12) to verify the hash.
 */
static const char *const g_csb_gfx_search[] = {
    "csb.dat",
    "CSB.DAT",
    "csbgraph.dat",
    "CSBGRAPH.DAT",
    "graphics.dat",
    "GRAPHICS.DAT",
    NULL
};

const char *csb_v1_runtime_find_graphics(const char *data_dir,
                                             const char *version_hint,
                                             CSB_V1_AssetResult *out_result)
{
    static char found_path[ASSET_PATH_MAX];
    const char *const *names;
    (void)version_hint; /* TODO: narrow search by version hint */

    if (!data_dir || !out_result) return NULL;
    memset(out_result, 0, sizeof(*out_result));

    /* Prefer MD5-hash search so files in arbitrary subdirs and renamed
     * user layouts are discovered before any legacy filename fallback. */
    int matchIndex = -1;
    if (asset_find_by_md5_list(data_dir, g_csb_graphics_hashes,
                                 found_path, sizeof(found_path),
                                 &matchIndex, 8)) {
        /* Determine archive kind from the matched hash + extension */
        CSB_V1_AssetGfxArchiveType kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
        const char *base = strrchr(found_path, '/');
        base = base ? base + 1 : found_path;
        if (strcasecmp(base, "CSB.DAT") == 0 ||
            strcasecmp(base, "csb.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSB;
        } else if (strcasecmp(base, "CSBGRAPH.DAT") == 0 ||
                   strcasecmp(base, "csbgraph.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
        }
        out_result->path = found_path;
        out_result->kind = kind;
        return found_path;
    }

    for (names = g_csb_gfx_search; *names != NULL; names++) {
        char tmp[ASSET_PATH_MAX];
        struct stat st;
        CSB_V1_AssetGfxArchiveType kind;

        snprintf(tmp, sizeof(tmp), "%s/%s", data_dir, *names);
        if (stat(tmp, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        strncpy(found_path, tmp, sizeof(found_path) - 1);
        found_path[sizeof(found_path) - 1] = '\0';
        out_result->path = found_path;

        if (strcasecmp(*names, "CSB.DAT") == 0 ||
            strcasecmp(*names, "csb.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSB;
        } else if (strcasecmp(*names, "CSBGRAPH.DAT") == 0 ||
                   strcasecmp(*names, "csbgraph.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
        } else {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
        }
        out_result->kind = kind;
        return found_path;
    }
    return NULL;
}

/* ── Save namespace paths ───────────────────────────────────────────── */

const char *csb_v1_runtime_save_dir(void)
{
    csb_v1_init_save_dir();
    return g_csb_save_dir_buf;
}

const char *csb_v1_runtime_save_path(int slot)
{
    csb_v1_init_save_dir();
    if (slot < 0 || slot > 9) slot = 0;
    snprintf(g_csb_save_path_buf, sizeof(g_csb_save_path_buf),
             "%s%ccsb_save_%d.fsav",
             g_csb_save_dir_buf, CSB_PATH_SEP, slot);
    return g_csb_save_path_buf;
}

static uint16_t csb_v1_runtime_effective_game_id(
    const CSB_V1_RuntimeProfile *profile)
{
    if (profile && profile->dungeon_game_id) {
        return profile->dungeon_game_id;
    }
    return 0x1234u;
}

static void csb_v1_runtime_capture_save_image(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimeSaveImageV1 *image)
{
    memset(image, 0, sizeof(*image));
    image->magic = CSB_V1_RUNTIME_SAVE_MAGIC;
    image->version = CSB_V1_RUNTIME_SAVE_VERSION;
    image->byte_size = (uint32_t)sizeof(*image);
    image->variant_id = (int32_t)profile->variant_id;
    image->difficulty = (int32_t)profile->difficulty;
    image->dungeon_seed = profile->dungeon_seed;
    image->dungeon_game_id = csb_v1_runtime_effective_game_id(profile);
    image->current_level = profile->current_level;
    image->current_world = profile->current_world;
    image->level_count = profile->level_count;
    image->world_count = profile->world_count;
    image->party_x = profile->party_x;
    image->party_y = profile->party_y;
    image->party_z = profile->party_z;
    image->party_dir = profile->party_dir;
    image->champion_count = profile->champion_count;
    image->leader_index = profile->leader_index;
    image->magic_caster_index = profile->magic_caster_index;
    image->party_state_valid = profile->party_state_valid;
    image->state = profile->state;
    image->paused = profile->paused;
    image->victory = profile->victory;
    image->game_over = profile->game_over;
    image->entrance_map_index = profile->entrance_map_index;
    image->start_map_index = profile->start_map_index;
    image->game_ticks = profile->game_ticks;
    image->game_time = profile->game_time;
    image->total_play_ms = profile->total_play_ms;
    image->tick_count = profile->tick_count;
    image->timeline_queue = profile->timeline_queue;
    image->last_timeline_dispatch = profile->last_timeline_dispatch;
    image->timeline_dispatch_count = profile->timeline_dispatch_count;
    image->input_command_queue = profile->input_command_queue;
    image->last_input_dispatch = profile->last_input_dispatch;
    image->input_dispatch_count = profile->input_dispatch_count;
    image->chaos_magic = profile->chaos_magic;
    image->party_state = profile->party_state;
    image->projectiles = profile->projectiles;
    image->explosions = profile->explosions;
    image->csbwin_header_tail_valid =
        profile->csbwin_header_tail_valid ? 1 : 0;
    memcpy(image->csbwin_header_byte22808,
           profile->csbwin_header_byte22808,
           sizeof(image->csbwin_header_byte22808));
    image->csbwin_appended_tail_valid =
        profile->csbwin_appended_tail_valid ? 1 : 0;
    image->csbwin_appended_tail_size =
        (uint32_t)profile->csbwin_appended_tail_size;
    image->csbwin_appended_tail_preserved_size =
        (uint32_t)profile->csbwin_appended_tail_preserved_size;
    image->csbwin_appended_tail_fnv1a =
        profile->csbwin_appended_tail_fnv1a;
    image->csbwin_appended_tail_truncated =
        profile->csbwin_appended_tail_truncated ? 1 : 0;
    memcpy(image->csbwin_appended_tail,
           profile->csbwin_appended_tail,
           sizeof(image->csbwin_appended_tail));
    image->active_group_state_count = profile->active_group_state_count;
    memcpy(image->active_group_state,
           profile->active_group_state,
           sizeof(image->active_group_state));
    csb_v1_audio_runtime_save_snapshot(&profile->audio_runtime,
                                       &image->audio_snapshot);
}

static int csb_v1_runtime_validate_projectile_list(
    const struct ProjectileList_Compat *list)
{
    int i;
    int active_count = 0;

    if (!list) return 0;
    if (list->count < 0 || list->count > PROJECTILE_LIST_CAPACITY) return 0;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat *projectile =
            &list->entries[i];
        if (projectile->reserved3 == 0) {
            continue;
        }
        if (projectile->slotIndex != i ||
            projectile->mapIndex < 0 ||
            projectile->cell < 0 ||
            projectile->cell > 3 ||
            projectile->direction < 0 ||
            projectile->direction > 3) {
            return 0;
        }
        active_count++;
    }
    return active_count == list->count;
}

static int csb_v1_runtime_validate_explosion_list(
    const struct ExplosionList_Compat *list)
{
    int i;
    int active_count = 0;

    if (!list) return 0;
    if (list->count < 0 || list->count > EXPLOSION_LIST_CAPACITY) return 0;
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
        const struct ExplosionInstance_Compat *explosion =
            &list->entries[i];
        if (explosion->reserved0 == 0) {
            continue;
        }
        if (explosion->slotIndex != i ||
            explosion->mapIndex < 0 ||
            explosion->cell < 0 ||
            explosion->cell > EXPLOSION_CELL_CENTERED) {
            return 0;
        }
        active_count++;
    }
    return active_count == list->count;
}

static int csb_v1_runtime_validate_active_group_state(
    const CSB_V1_RuntimeSaveImageV1 *image)
{
    uint16_t i;
    uint16_t active = 0u;

    if (!image) return 0;
    if (image->active_group_state_count >
        CSB_V1_RUNTIME_ACTIVE_GROUP_CAP) {
        return 0;
    }
    for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
        const CSB_V1_RuntimeActiveGroupState *state =
            &image->active_group_state[i];
        if (!state->valid) continue;
        if (state->map_index < 0 ||
            state->map_x < 0 ||
            state->map_y < 0 ||
            state->target_map_x < 0 ||
            state->target_map_y < 0 ||
            ((state->group_thing >> 10) & 0x0Fu) != 4u) {
            return 0;
        }
        ++active;
    }
    return active == image->active_group_state_count;
}

static int csb_v1_runtime_active_group_state_entry_valid(
    const CSB_V1_RuntimeActiveGroupState *state)
{
    if (!state) return 0;
    if (!state->valid) return 1;
    if (state->map_index < 0 ||
        state->map_x < 0 ||
        state->map_y < 0 ||
        state->target_map_x < 0 ||
        state->target_map_y < 0 ||
        ((state->group_thing >> 10) & 0x0Fu) != 4u) {
        return 0;
    }
    return 1;
}

static int csb_v1_runtime_apply_active_group_state_from_save_image(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeSaveImageV1 *image)
{
    uint16_t i;
    uint16_t active = 0u;
    const uint8_t *base;

    if (!profile || !image) return -1;
    profile->active_group_state_count = 0u;
    memset(profile->active_group_state, 0,
           sizeof(profile->active_group_state));

    if (image->byte_size < CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u) {
        return 0;
    }
    if (image->active_group_state_count >
        CSB_V1_RUNTIME_ACTIVE_GROUP_CAP) {
        return -1;
    }

    base = ((const uint8_t *)image) +
           offsetof(CSB_V1_RuntimeSaveImageV1, active_group_state);
    if (image->version == 7u &&
        image->byte_size == CSB_V1_RUNTIME_SAVE_V7_SIZE) {
        const CSB_V1_RuntimeActiveGroupStateV7 *legacy =
            (const CSB_V1_RuntimeActiveGroupStateV7 *)base;
        for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
            CSB_V1_RuntimeActiveGroupState *state =
                &profile->active_group_state[i];
            if (!legacy[i].valid) continue;
            state->valid = legacy[i].valid;
            state->group_thing = legacy[i].group_thing;
            state->map_index = legacy[i].map_index;
            state->map_x = legacy[i].map_x;
            state->map_y = legacy[i].map_y;
            state->prior_map_x = legacy[i].map_x;
            state->prior_map_y = legacy[i].map_y;
            state->home_map_x = legacy[i].map_x;
            state->home_map_y = legacy[i].map_y;
            state->target_map_x = legacy[i].map_x;
            state->target_map_y = legacy[i].map_y;
            state->delay_fleeing_from_target =
                legacy[i].delay_fleeing_from_target;
            if (!csb_v1_runtime_active_group_state_entry_valid(state)) {
                return -1;
            }
            ++active;
        }
    } else if (image->version == 8u &&
               image->byte_size == CSB_V1_RUNTIME_SAVE_V8_SIZE) {
        const CSB_V1_RuntimeActiveGroupStateV8 *legacy =
            (const CSB_V1_RuntimeActiveGroupStateV8 *)base;
        for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
            CSB_V1_RuntimeActiveGroupState *state =
                &profile->active_group_state[i];
            if (!legacy[i].valid) continue;
            state->valid = legacy[i].valid;
            state->group_thing = legacy[i].group_thing;
            state->map_index = legacy[i].map_index;
            state->map_x = legacy[i].map_x;
            state->map_y = legacy[i].map_y;
            state->cells = legacy[i].cells;
            state->directions = legacy[i].directions;
            state->prior_map_x = legacy[i].prior_map_x;
            state->prior_map_y = legacy[i].prior_map_y;
            state->home_map_x = legacy[i].home_map_x;
            state->home_map_y = legacy[i].home_map_y;
            state->last_move_time = legacy[i].last_move_time;
            state->target_map_x = legacy[i].map_x;
            state->target_map_y = legacy[i].map_y;
            state->delay_fleeing_from_target =
                legacy[i].delay_fleeing_from_target;
            if (!csb_v1_runtime_active_group_state_entry_valid(state)) {
                return -1;
            }
            ++active;
        }
    } else if (image->version == 9u &&
               image->byte_size == CSB_V1_RUNTIME_SAVE_V9_SIZE) {
        const CSB_V1_RuntimeActiveGroupStateV9 *legacy =
            (const CSB_V1_RuntimeActiveGroupStateV9 *)base;
        for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
            CSB_V1_RuntimeActiveGroupState *state =
                &profile->active_group_state[i];
            if (!legacy[i].valid) continue;
            state->valid = legacy[i].valid;
            state->group_thing = legacy[i].group_thing;
            state->map_index = legacy[i].map_index;
            state->map_x = legacy[i].map_x;
            state->map_y = legacy[i].map_y;
            state->cells = legacy[i].cells;
            state->directions = legacy[i].directions;
            state->prior_map_x = legacy[i].prior_map_x;
            state->prior_map_y = legacy[i].prior_map_y;
            state->home_map_x = legacy[i].home_map_x;
            state->home_map_y = legacy[i].home_map_y;
            state->last_move_time = legacy[i].last_move_time;
            state->target_map_x = legacy[i].map_x;
            state->target_map_y = legacy[i].map_y;
            memcpy(state->aspect, legacy[i].aspect, sizeof(state->aspect));
            state->delay_fleeing_from_target =
                legacy[i].delay_fleeing_from_target;
            if (!csb_v1_runtime_active_group_state_entry_valid(state)) {
                return -1;
            }
            ++active;
        }
    } else if (image->byte_size >=
               offsetof(CSB_V1_RuntimeSaveImageV1, active_group_state) +
                   sizeof(image->active_group_state)) {
        if (!csb_v1_runtime_validate_active_group_state(image)) {
            return -1;
        }
        profile->active_group_state_count =
            image->active_group_state_count;
        memcpy(profile->active_group_state,
               image->active_group_state,
               sizeof(profile->active_group_state));
        return 0;
    }

    if (active != image->active_group_state_count) {
        memset(profile->active_group_state, 0,
               sizeof(profile->active_group_state));
        return -1;
    }
    profile->active_group_state_count = active;
    return 0;
}

static int csb_v1_runtime_apply_save_image(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeSaveImageV1 *image,
    const CSB_V1_SaveHeader *header)
{
    int leader;
    if (!profile || !image || !header) return -1;
    if (image->magic != CSB_V1_RUNTIME_SAVE_MAGIC) {
        return -1;
    }
    if (!((image->version == 1u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V1_SIZE) ||
          (image->version == 4u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V4_SIZE) ||
          (image->version == 5u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V5_SIZE) ||
          (image->version == 6u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V6_SIZE) ||
          (image->version == 7u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V7_SIZE) ||
          (image->version == 8u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V8_SIZE) ||
          (image->version == 10u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V10_SIZE) ||
          (image->version == CSB_V1_RUNTIME_SAVE_VERSION &&
           image->byte_size == sizeof(*image)))) {
        return -1;
    }
    if (header->Magic != CSB_V1_SAVE_MAGIC_CSB ||
        header->GameID != image->dungeon_game_id) {
        return -1;
    }
    if (image->champion_count < 0 ||
        image->champion_count > CSB_V1_MAX_CHAMPIONS ||
        image->party_state.ChampionCount < 0 ||
        image->party_state.ChampionCount > CSB_V1_MAX_CHAMPIONS ||
        image->party_dir < 0 || image->party_dir > 3) {
        return -1;
    }

    /* ReDMCSB LOADSAVE.C F0435 lines 2721-2800 restores GLOBAL_DATA,
     * PARTY, EVENTS, and TIMELINE into live globals before play resumes.
     * Firestaff's CSB profile currently owns those boundaries directly in
     * this POD runtime image; asset paths and the loaded dungeon handle stay
     * with the caller's already booted profile. */
    profile->variant_id = (CSB_V1_VariantId)image->variant_id;
    profile->difficulty = (CSB_V1_Difficulty)image->difficulty;
    profile->dungeon_seed = image->dungeon_seed;
    profile->dungeon_game_id = image->dungeon_game_id;
    profile->current_level = image->current_level;
    csb_v1_dungeon_set_current_level(profile->current_level);
    profile->current_world = image->current_world;
    profile->level_count = image->level_count;
    profile->world_count = image->world_count;
    profile->party_x = image->party_x;
    profile->party_y = image->party_y;
    profile->party_z = image->party_z;
    profile->party_dir = image->party_dir & 3;
    profile->champion_count = image->champion_count;
    profile->leader_index = image->leader_index;
    profile->magic_caster_index = image->magic_caster_index;
    profile->party_state_valid = image->party_state_valid ? 1 : 0;
    profile->state = image->state;
    profile->paused = image->paused;
    profile->victory = image->victory;
    profile->game_over = image->game_over;
    profile->entrance_map_index = image->entrance_map_index;
    profile->start_map_index = image->start_map_index;
    profile->game_ticks = image->game_ticks;
    profile->game_time = image->game_time;
    profile->total_play_ms = image->total_play_ms;
    profile->tick_count = image->tick_count;
    profile->timeline_queue = image->timeline_queue;
    profile->last_timeline_dispatch = image->last_timeline_dispatch;
    profile->timeline_dispatch_count = image->timeline_dispatch_count;
    if (image->byte_size >=
        offsetof(CSB_V1_RuntimeSaveImageV1, explosions) +
            sizeof(image->explosions)) {
        if (!csb_v1_runtime_validate_projectile_list(&image->projectiles) ||
            !csb_v1_runtime_validate_explosion_list(&image->explosions)) {
            return -1;
        }
        profile->projectiles = image->projectiles;
        profile->explosions = image->explosions;
    } else {
        memset(&profile->projectiles, 0, sizeof(profile->projectiles));
        memset(&profile->explosions, 0, sizeof(profile->explosions));
    }
    profile->input_command_queue = image->input_command_queue;
    profile->last_input_dispatch = image->last_input_dispatch;
    profile->input_dispatch_count = image->input_dispatch_count;
    profile->chaos_magic = image->chaos_magic;
    profile->party_state = image->party_state;
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(
            profile->party_state.LeaderHandThing);
    /* The boot-owned CSBWin GAMEBLOCK2 summary is not serialized in the
     * native image, but its live hand mirror must not diverge from the
     * restored PARTY state after LOADSAVE.C F0435 ownership resumes. */
    if (profile->csbwin_gameblock2_summary_valid &&
        profile->party_state_valid) {
        profile->csbwin_object_in_hand =
            profile->party_state.LeaderHandThing;
    }
    if (image->byte_size >=
        offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_header_byte22808) +
            sizeof(image->csbwin_header_byte22808)) {
        profile->csbwin_header_tail_valid =
            image->csbwin_header_tail_valid ? 1 : 0;
        memcpy(profile->csbwin_header_byte22808,
               image->csbwin_header_byte22808,
               sizeof(profile->csbwin_header_byte22808));
    } else {
        profile->csbwin_header_tail_valid = 0;
        memset(profile->csbwin_header_byte22808, 0,
               sizeof(profile->csbwin_header_byte22808));
    }
    if (image->byte_size >=
        offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_appended_tail) +
            sizeof(image->csbwin_appended_tail)) {
        if (image->csbwin_appended_tail_preserved_size >
                CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
            image->csbwin_appended_tail_size <
                image->csbwin_appended_tail_preserved_size) {
            return -1;
        }
        profile->csbwin_appended_tail_valid =
            image->csbwin_appended_tail_valid ? 1 : 0;
        profile->csbwin_appended_tail_size =
            image->csbwin_appended_tail_size;
        profile->csbwin_appended_tail_preserved_size =
            image->csbwin_appended_tail_preserved_size;
        profile->csbwin_appended_tail_fnv1a =
            image->csbwin_appended_tail_fnv1a;
        profile->csbwin_appended_tail_truncated =
            image->csbwin_appended_tail_truncated ? 1 : 0;
        memcpy(profile->csbwin_appended_tail,
               image->csbwin_appended_tail,
               sizeof(profile->csbwin_appended_tail));
    } else {
        profile->csbwin_appended_tail_valid = 0;
        profile->csbwin_appended_tail_size = 0u;
        profile->csbwin_appended_tail_preserved_size = 0u;
        profile->csbwin_appended_tail_fnv1a = 0u;
        profile->csbwin_appended_tail_truncated = 0;
        memset(profile->csbwin_appended_tail, 0,
               sizeof(profile->csbwin_appended_tail));
    }
    if (csb_v1_runtime_apply_active_group_state_from_save_image(
            profile,
            image) != 0) {
        return -1;
    }
    if (image->byte_size >=
        offsetof(CSB_V1_RuntimeSaveImageV1, audio_snapshot) +
            sizeof(image->audio_snapshot)) {
        csb_v1_audio_runtime_load_snapshot(&profile->audio_runtime,
                                           &image->audio_snapshot);
    } else {
        csb_v1_audio_runtime_init(&profile->audio_runtime);
    }

    profile->party_state.PartyMapX = profile->party_x;
    profile->party_state.PartyMapY = profile->party_y;
    profile->party_state.PartyDirection = (uint8_t)(profile->party_dir & 3);
    profile->party_state.MagicCasterIndex = profile->magic_caster_index;
    leader = profile->leader_index;
    if (leader < -1 || leader >= profile->party_state.ChampionCount) {
        leader = csb_v1_runtime_first_living_champion(&profile->party_state);
    }
    profile->leader_index = leader;
    profile->party_state.LeaderIndex = leader;
    profile->timeline_queue.gameTick = profile->game_time;
    return 0;
}

int csb_v1_runtime_save_game_to_path(const CSB_V1_RuntimeProfile *profile,
                                     const char *path)
{
    CSB_V1_RuntimeSaveImageV1 image;
    CSB_V1_SaveHeader header;
    uint16_t game_id;

    if (!profile || !path) return -1;
    game_id = csb_v1_runtime_effective_game_id(profile);
    csb_v1_runtime_capture_save_image(profile, &image);
    memset(&header, 0, sizeof(header));
    if (csb_v1_save_header_build(&header,
                                  CSB_V1_SAVE_MAGIC_CSB,
                                  game_id,
                                  profile->dungeon_seed,
                                  profile->party_x,
                                  profile->party_y,
                                  profile->current_level,
                                  profile->party_dir,
                                  profile->champion_count,
                                  profile->game_time,
                                  (uint32_t)profile->total_play_ms) != 0) {
        return -1;
    }
    return csb_v1_save_game(path, &image, (int)sizeof(image), &header);
}

int csb_v1_runtime_import_csbgame_roster_from_path(
    CSB_V1_RuntimeProfile *profile,
    const char *path)
{
    CSB_V1_PartyState party;
    int imported;
    int pose_x;
    int pose_y;
    int pose_z;
    int pose_dir;
    int pose_level;

    if (!profile || !path) return CSB_SAVE_IMPORT_ERR_NULL;

    pose_x = profile->party_x;
    pose_y = profile->party_y;
    pose_z = profile->party_z;
    pose_dir = profile->party_dir & 3;
    pose_level = profile->current_level;

    memset(&party, 0, sizeof(party));
    imported = csb_v1_import_csb_save_file(&party, path);
    if (imported <= 0) {
        return imported;
    }

    /* ReDMCSB LOADSAVE.C F0435 restores a full running game, while
     * CHARACTER.C ReadingChampion()/CEDTINC8.C import only the roster
     * payload from CSBGAME.DAT.  Until the CSBGAME dungeon/global-data
     * body is source-locked, keep the already booted runtime pose and
     * promote only the champion roster into live CSB state. */
    party.PartyMapX = pose_x;
    party.PartyMapY = pose_y;
    party.PartyDirection = (uint8_t)pose_dir;
    party.MagicCasterIndex = party.LeaderIndex;

    if (csb_v1_runtime_set_party_state(profile, &party) != 0) {
        return -1;
    }

    profile->party_x = pose_x;
    profile->party_y = pose_y;
    profile->party_z = pose_z;
    profile->party_dir = pose_dir;
    profile->current_level = pose_level;
    csb_v1_dungeon_set_current_level(profile->current_level);
    profile->difficulty =
        (CSB_V1_Difficulty)csb_v1_runtime_calc_difficulty(imported);
    profile->party_state.PartyMapX = pose_x;
    profile->party_state.PartyMapY = pose_y;
    profile->party_state.PartyDirection = (uint8_t)pose_dir;
    profile->party_state.MagicCasterIndex = profile->magic_caster_index;
    profile->timeline_queue.gameTick = profile->game_time;
    return CSB_V1_LOAD_OK;
}

int csb_v1_runtime_load_game_from_path(CSB_V1_RuntimeProfile *profile,
                                       const char *path)
{
    CSB_V1_RuntimeSaveImageV1 image;
    CSB_V1_SaveHeader header;
    int result;

    if (!profile || !path) return -1;
    memset(&image, 0, sizeof(image));
    memset(&header, 0, sizeof(header));

    /* CSBWin GAMEBLOCK1 is also 512 bytes, so the older Firestaff-native
     * header probe can produce a misleading positive before the CSBWin body
     * sections are considered. Try the authenticated CSBWin resume body
     * first; malformed/foreign fixtures still fail its checksum and section
     * verification before falling through to the native and roster paths. */
    if (csb_v1_runtime_apply_csbwin_resume_file(profile, path, 0u) == 0) {
        return CSB_V1_LOAD_OK;
    }

    result = csb_v1_load_game(path, &image, (int)sizeof(image), &header);
    if (result != CSB_V1_LOAD_OK) {
        int import_result;

        /* CSBWin SaveGame.cpp/Chaos.cpp first validates GAMEBLOCK1, then
         * reads GAMEBLOCK2, ITEM16, CHARDESC, TIMER, and timer queue. Keep
         * this runtime entry point aligned with M11 startup/F9 so callers do
         * not have to special-case verified CSBWin saves outside runtime. */
        import_result =
            csb_v1_runtime_import_csbgame_roster_from_path(profile, path);
        return (import_result == CSB_V1_LOAD_OK) ? CSB_V1_LOAD_OK : result;
    }
    return csb_v1_runtime_apply_save_image(profile, &image, &header);
}

int csb_v1_runtime_can_load_resume_path(const char *path)
{
    enum { MAX_CSBWIN_RESUME_BYTES = 4 * 1024 * 1024 };
    CSB_V1_SaveHeader header;
    CSB_V1_CSBWin512BodyReport report;
    CSB_V1_PartyState party;
    FILE *fp;
    long file_size_long;
    size_t file_size;
    uint8_t *bytes;
    size_t got;
    int rc;

    if (!path || path[0] == '\0') {
        return 0;
    }
    memset(&header, 0, sizeof(header));
    if (csb_v1_load_game(path, NULL, 0, &header) == CSB_V1_LOAD_OK &&
        header.Magic == CSB_V1_SAVE_MAGIC_CSB) {
        return 1;
    }

    fp = fopen(path, "rb");
    if (fp) {
        if (fseek(fp, 0L, SEEK_END) == 0 &&
            (file_size_long = ftell(fp)) >= 0 &&
            (file_size = (size_t)file_size_long) <=
                (size_t)MAX_CSBWIN_RESUME_BYTES &&
            fseek(fp, 0L, SEEK_SET) == 0) {
            bytes = (uint8_t *)malloc(file_size > 0u ? file_size : 1u);
            if (bytes) {
                got = fread(bytes, 1u, file_size, fp);
                if (got == file_size) {
                    memset(&report, 0, sizeof(report));
                    rc = csb_v1_csbwin_512_verify_save_body(
                        bytes,
                        file_size,
                        0u,
                        &report);
                    if (rc == CSB_V1_CSBWIN_512_OK &&
                        report.header.verdict ==
                            CSB_V1_CSBWIN_512_VERDICT_CSB) {
                        free(bytes);
                        fclose(fp);
                        return 1;
                    }
                }
                free(bytes);
            }
        }
        fclose(fp);
    }

    memset(&party, 0, sizeof(party));
    return csb_v1_import_csb_save_file(&party, path) > 0;
}

int csb_v1_runtime_import_dm1_party_path(CSB_V1_RuntimeProfile *profile,
                                         const char *path,
                                         int *out_count,
                                         int *out_utility_state,
                                         char *out_utility_prompt,
                                         size_t out_utility_prompt_size)
{
    CSB_V1_UtilFlowContext flow;
    CSB_V1_PartyState party;
    int count;

    if (out_count) {
        *out_count = 0;
    }
    if (out_utility_state) {
        *out_utility_state = (int)CSB_V1_UTIL_FLOW_INIT;
    }
    if (out_utility_prompt && out_utility_prompt_size > 0u) {
        out_utility_prompt[0] = '\0';
    }
    if (!profile || !path || path[0] == '\0') {
        return 0;
    }

    /* ReDMCSB/CSBWin utility startup imports a DM1 party before the
     * CSB dungeon starts.  Keep this as a runtime-party handoff rather
     * than a Resume save: the entrance still owns the final Enter click.
     * Drive the CSB utility setup state machine so the runtime handoff
     * follows the same IMPORT -> CONFIRM_IMPORT -> NEW_GAME surface used
     * by the launcher utility path. */
    csb_v1_util_flow_init(&flow);
    csb_v1_util_flow_set_dm1_path(&flow, path);
    csb_v1_util_flow_mark_utility_disk_verified(&flow, 1);
    if (csb_v1_util_flow_step(&flow) != 0 ||
        flow.state != CSB_V1_UTIL_FLOW_INSERT_DISK) {
        return 0;
    }
    if (csb_v1_util_flow_step(&flow) != 0 ||
        flow.state != CSB_V1_UTIL_FLOW_VERIFY_DISK) {
        return 0;
    }
    if (csb_v1_util_flow_step(&flow) != 0 ||
        flow.state != CSB_V1_UTIL_FLOW_DISK_OK) {
        return 0;
    }
    if (csb_v1_util_flow_step(&flow) != 0 ||
        flow.state != CSB_V1_UTIL_FLOW_SELECT_ACTION) {
        return 0;
    }
    if (!csb_v1_util_flow_accept_import_action(&flow)) {
        return 0;
    }
    if (csb_v1_util_flow_step(&flow) != 0 ||
        flow.state != CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS) {
        return 0;
    }
    if (csb_v1_util_flow_step(&flow) != 0 ||
        flow.state != CSB_V1_UTIL_FLOW_CONFIRM_IMPORT) {
        return 0;
    }
    csb_v1_util_flow_confirm_import(&flow, 1);
    if (csb_v1_util_flow_step(&flow) != 0 ||
        flow.state != CSB_V1_UTIL_FLOW_NEW_GAME) {
        return 0;
    }
    if (csb_v1_util_flow_step(&flow) != 1 ||
        flow.state != CSB_V1_UTIL_FLOW_DONE) {
        return 0;
    }
    if (out_utility_state) {
        *out_utility_state = (int)flow.state;
    }
    if (out_utility_prompt && out_utility_prompt_size > 0u) {
        snprintf(out_utility_prompt,
                 out_utility_prompt_size,
                 "%s",
                 csb_v1_util_flow_prompt(&flow));
    }
    memset(&party, 0, sizeof(party));
    count = csb_v1_util_flow_get_party(&flow, &party);
    if (count <= 0 || !party.ImportedFromDM1) {
        return 0;
    }
    if (csb_v1_runtime_set_party_state(profile, &party) != 0) {
        return 0;
    }
    if (out_count) {
        *out_count = count;
    }
    return 1;
}

void csb_v1_runtime_startup_handoff_receipt_init_pc34(
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->kind = CSB_V1_RUNTIME_STARTUP_HANDOFF_NONE_PC34;
    receipt->import_utility_state = (int)CSB_V1_UTIL_FLOW_INIT;
}

int csb_v1_runtime_apply_startup_handoff_pc34(
    CSB_V1_RuntimeProfile *profile,
    const char *save_path,
    const char *import_dm1_save_path,
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_startup_handoff_receipt_init_pc34(out_receipt);
    if (!profile) {
        out_receipt->status_scope = "BOOT";
        out_receipt->status = "CSB BOOT FAILED";
        return 0;
    }
    if (save_path && save_path[0] != '\0') {
        out_receipt->kind = CSB_V1_RUNTIME_STARTUP_HANDOFF_RESUME_PC34;
        if (csb_v1_runtime_load_game_from_path(profile, save_path) !=
            CSB_V1_LOAD_OK) {
            out_receipt->status_scope = "BOOT";
            out_receipt->status = "CSB RESUME FAILED";
            return 0;
        }
        out_receipt->direct_resume_loaded = 1;
        out_receipt->status_scope = "BOOT";
        out_receipt->status = "CSB RESUMED";
        return 1;
    }
    if (import_dm1_save_path && import_dm1_save_path[0] != '\0') {
        out_receipt->kind =
            CSB_V1_RUNTIME_STARTUP_HANDOFF_IMPORT_DM1_PC34;
        out_receipt->import_attempted = 1;
        if (!csb_v1_runtime_import_dm1_party_path(
                profile,
                import_dm1_save_path,
                &out_receipt->import_champion_count,
                &out_receipt->import_utility_state,
                out_receipt->import_utility_prompt,
                sizeof(out_receipt->import_utility_prompt))) {
            out_receipt->status_scope = "BOOT";
            out_receipt->status = "CSB IMPORT FAILED";
            return 0;
        }
        out_receipt->import_succeeded = 1;
        out_receipt->status_scope = "BOOT";
        out_receipt->status = "CSB IMPORT READY";
        return 1;
    }
    out_receipt->status_scope = "BOOT";
    out_receipt->status = "CSB READY";
    return 1;
}

static int csb_v1_runtime_copy_startup_string_pc34(char *dst,
                                                   size_t dst_size,
                                                   const char *src)
{
    int rc;
    if (!dst || dst_size == 0u) {
        return 0;
    }
    dst[0] = '\0';
    if (!src || src[0] == '\0') {
        return 0;
    }
    rc = snprintf(dst, dst_size, "%s", src);
    if (rc <= 0 || (size_t)rc >= dst_size) {
        dst[0] = '\0';
        return 0;
    }
    return 1;
}

int csb_v1_runtime_build_startup_session_options_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeStartupHandoffReceipt_PC34 *handoff,
    const char *import_dm1_save_path,
    const char *entrance_resume_save_path,
    CSB_V1_StartupSessionOptions_PC34 *out_options)
{
    if (!out_options) {
        return 0;
    }
    memset(out_options, 0, sizeof(*out_options));
    out_options->import_utility_state = (int)CSB_V1_UTIL_FLOW_INIT;
    if (handoff) {
        out_options->import_utility_state = handoff->import_utility_state;
        if (handoff->direct_resume_loaded) {
            return 1;
        }
        if (handoff->import_succeeded && profile) {
            CSB_V1_PartyState imported_party;
            memset(&imported_party, 0, sizeof(imported_party));
            if (csb_v1_runtime_get_party_state(profile, &imported_party) >= 0 &&
                imported_party.ImportedFromDM1 &&
                imported_party.ChampionCount > 0 &&
                csb_v1_runtime_copy_startup_string_pc34(
                    out_options->import_dm1_save_path,
                    sizeof(out_options->import_dm1_save_path),
                    import_dm1_save_path)) {
                out_options->import_available = 1;
                out_options->import_champion_count =
                    handoff->import_champion_count > 0
                        ? handoff->import_champion_count
                        : imported_party.ChampionCount;
                out_options->import_selected_action_index = 0;
                out_options->import_preview_active = 0;
                (void)csb_v1_runtime_copy_startup_string_pc34(
                    out_options->import_utility_prompt,
                    sizeof(out_options->import_utility_prompt),
                    handoff->import_utility_prompt);
            }
        }
    }
    if (entrance_resume_save_path && entrance_resume_save_path[0] != '\0' &&
        csb_v1_runtime_can_load_resume_path(entrance_resume_save_path) &&
        csb_v1_runtime_copy_startup_string_pc34(
            out_options->entrance_resume_path,
            sizeof(out_options->entrance_resume_path),
            entrance_resume_save_path)) {
        out_options->entrance_resume_available = 1;
    }
    return 1;
}

void csb_v1_runtime_startup_session_state_receipt_init_pc34(
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->import_utility_state = (int)CSB_V1_UTIL_FLOW_INIT;
}

int csb_v1_runtime_startup_session_state_receipt_from_options_pc34(
    const CSB_V1_StartupSessionOptions_PC34 *options,
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_startup_session_state_receipt_init_pc34(out_receipt);
    if (!options) {
        return 0;
    }
    out_receipt->entrance_resume_available =
        options->entrance_resume_available ? 1 : 0;
    (void)csb_v1_runtime_copy_startup_string_pc34(
        out_receipt->entrance_resume_path,
        sizeof(out_receipt->entrance_resume_path),
        options->entrance_resume_path);
    out_receipt->import_available = options->import_available ? 1 : 0;
    out_receipt->import_champion_count = options->import_champion_count;
    out_receipt->import_selected_action_index =
        options->import_selected_action_index;
    out_receipt->import_preview_active =
        options->import_preview_active ? 1 : 0;
    out_receipt->import_utility_state = options->import_utility_state;
    (void)csb_v1_runtime_copy_startup_string_pc34(
        out_receipt->import_dm1_save_path,
        sizeof(out_receipt->import_dm1_save_path),
        options->import_dm1_save_path);
    (void)csb_v1_runtime_copy_startup_string_pc34(
        out_receipt->import_utility_prompt,
        sizeof(out_receipt->import_utility_prompt),
        options->import_utility_prompt);
    return 1;
}

int csb_v1_runtime_build_startup_session_state_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeStartupHandoffReceipt_PC34 *handoff,
    const char *import_dm1_save_path,
    const char *entrance_resume_save_path,
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionOptions_PC34 options;
    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_startup_session_state_receipt_init_pc34(out_receipt);
    if (!csb_v1_runtime_build_startup_session_options_pc34(
            profile,
            handoff,
            import_dm1_save_path,
            entrance_resume_save_path,
            &options)) {
        return 0;
    }
    return csb_v1_runtime_startup_session_state_receipt_from_options_pc34(
        &options,
        out_receipt);
}

void csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int csb_v1_runtime_apply_startup_runtime_plan_pc34(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeStartupRuntimePlan_PC34 *runtime_plan,
    const char *resume_path,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(out_receipt);
    if (!profile || !runtime_plan ||
        runtime_plan->kind == CSB_V1_RUNTIME_STARTUP_PLAN_NONE_PC34) {
        return 0;
    }

    if (runtime_plan->set_bonus_dungeon) {
        (void)csb_v1_runtime_set_load_bonus_dungeon(
            profile,
            runtime_plan->bonus_dungeon ? 1 : 0);
        out_receipt->bonus_requested_changed = 1;
        out_receipt->bonus_requested =
            runtime_plan->bonus_dungeon ? 1 : 0;
        if (runtime_plan->bonus_dungeon) {
            out_receipt->bonus_dungeon_loaded =
                csb_v1_runtime_try_load_bonus_dungeon(profile) ? 1 : 0;
            if (out_receipt->bonus_dungeon_loaded) {
                out_receipt->sync_profile_state = 1;
            }
        }
    }

    if (runtime_plan->requires_resume_load) {
        out_receipt->resume_available =
            (resume_path && resume_path[0] != '\0') ? 1 : 0;
        if (out_receipt->resume_available &&
            csb_v1_runtime_load_game_from_path(profile, resume_path) ==
                CSB_V1_LOAD_OK) {
            out_receipt->resume_loaded = 1;
            out_receipt->sync_profile_state = 1;
            out_receipt->sync_leader_hand = 1;
        }
    }
    return 1;
}

int csb_v1_runtime_apply_startup_sequence_plan_pc34(
    CSB_V1_RuntimeProfile *profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeStartupRuntimePlan_PC34 runtime_plan;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(out_receipt);
    if (!startup_plan ||
        startup_plan->kind == CSB_V1_STARTUP_RUNTIME_PLAN_NONE_PC34) {
        return 0;
    }

    memset(&runtime_plan, 0, sizeof(runtime_plan));
    runtime_plan.set_bonus_dungeon = startup_plan->set_bonus_dungeon;
    runtime_plan.bonus_dungeon = startup_plan->bonus_dungeon;
    runtime_plan.requires_resume_load = startup_plan->requires_resume_load;

    switch (startup_plan->kind) {
        case CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_DUNGEON_PC34:
            runtime_plan.kind =
                CSB_V1_RUNTIME_STARTUP_PLAN_ENTER_DUNGEON_PC34;
            break;
        case CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_BONUS_DUNGEON_PC34:
            runtime_plan.kind =
                CSB_V1_RUNTIME_STARTUP_PLAN_ENTER_BONUS_DUNGEON_PC34;
            break;
        case CSB_V1_STARTUP_RUNTIME_PLAN_RESUME_PC34:
            runtime_plan.kind =
                CSB_V1_RUNTIME_STARTUP_PLAN_RESUME_PC34;
            break;
        case CSB_V1_STARTUP_RUNTIME_PLAN_NONE_PC34:
        default:
            return 0;
    }

    return csb_v1_runtime_apply_startup_runtime_plan_pc34(profile,
                                                          &runtime_plan,
                                                          resume_path,
                                                          out_receipt);
}

void csb_v1_runtime_view_state_receipt_init_pc34(
    CSB_V1_RuntimeViewStateReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int csb_v1_runtime_view_state_receipt_from_profile_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimeViewStateReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_view_state_receipt_init_pc34(out_receipt);
    if (!profile) {
        return 0;
    }

    /* ReDMCSB: ENTRANCE.C F0806 hands selected dungeon map and party
     * pose to the runtime before the view mirrors the active state. */
    out_receipt->level_loaded = profile->dungeon_handle ? 1 : 0;
    out_receipt->current_level = profile->current_level;
    out_receipt->party_x = profile->party_x;
    out_receipt->party_y = profile->party_y;
    out_receipt->party_dir = profile->party_dir;
    out_receipt->tick_count = (int)profile->tick_count;
    return 1;
}

void csb_v1_runtime_party_mirror_receipt_init_pc34(
    CSB_V1_RuntimePartyMirrorReceipt_PC34 *receipt)
{
    int i;
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&receipt->party.champions[i]);
    }
    receipt->party.activeChampionIndex = -1;
}

int csb_v1_runtime_party_mirror_receipt_from_profile_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimePartyMirrorReceipt_PC34 *out_receipt)
{
    const CSB_V1_PartyState *src_party;
    int count;
    int leader;
    int i;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_party_mirror_receipt_init_pc34(out_receipt);
    if (!profile) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->party.championCount = 0;
    out_receipt->party.mapIndex = profile->current_level;
    out_receipt->party.mapX = profile->party_x;
    out_receipt->party.mapY = profile->party_y;
    out_receipt->party.direction = profile->party_dir & 3;
    out_receipt->party.activeChampionIndex = -1;

    if (!profile->party_state_valid) {
        return 1;
    }

    src_party = &profile->party_state;
    count = src_party->ChampionCount;
    if (count < 0) count = 0;
    if (count > CHAMPION_MAX_PARTY) count = CHAMPION_MAX_PARTY;
    out_receipt->party.championCount = count;

    leader = profile->leader_index;
    if (leader < 0 || leader >= count) {
        leader = src_party->LeaderIndex;
    }
    if (leader < 0 || leader >= count) {
        leader = (count > 0) ? 0 : -1;
    }
    out_receipt->party.activeChampionIndex = leader;

    for (i = 0; i < count; ++i) {
        const CSB_V1_Champion *src = &src_party->Champions[i];
        struct ChampionState_Compat *dst = &out_receipt->party.champions[i];
        int attr;
        int skill;
        int csb_slot;

        F0600_CHAMPION_InitEmpty_Compat(dst);
        dst->present = 1;
        dst->portraitIndex = i;
        csb_v1_runtime_pack_printable(dst->name,
                                      CHAMPION_NAME_LENGTH,
                                      src->Name,
                                      CSB_V1_MAX_NAME_LEN);
        csb_v1_runtime_pack_printable(dst->title,
                                      CHAMPION_TITLE_LENGTH,
                                      src->Title,
                                      CSB_V1_MAX_TITLE_LEN);
        csb_v1_runtime_copy_stat(&dst->hp,
                                 src->CurrentHealth,
                                 src->MaximumHealth);
        csb_v1_runtime_copy_stat(&dst->stamina,
                                 src->CurrentStamina,
                                 src->MaximumStamina);
        csb_v1_runtime_copy_stat(&dst->mana,
                                 src->CurrentMana,
                                 src->MaximumMana);
        for (attr = 0; attr < CHAMPION_ATTR_COUNT &&
                       attr < CSB_V1_STAT_COUNT; ++attr) {
            dst->attributes[attr] =
                csb_v1_runtime_clamp_u16((int)src->Statistics[attr][CSB_V1_STAT_CUR]);
            dst->attributeMaximums[attr] =
                csb_v1_runtime_clamp_u16((int)src->Statistics[attr][CSB_V1_STAT_MAX]);
        }
        for (skill = 0; skill < CHAMPION_SKILL_COUNT &&
                        skill < CSB_V1_SKILL_COUNT; ++skill) {
            /* CSBWin Code17818.cpp::DetermineMastery is the live CHARDESC
             * owner when a loaded CSBWin save supplies the complete 20-row
             * SKILL table.  Do not mirror the lossy cached 16-byte row after
             * a DSA Magic.cpp::AddToSkill mutation. */
            int level = csb_v1_runtime_get_champion_skill_level(
                profile, i, skill);
            dst->skillLevels[skill] = (uint8_t)((level > 0) ? level : 1);
        }
        /* ReDMCSB: DEFS.H C00_SLOT_READY_HAND/C01_SLOT_ACTION_HAND and
         * PANEL.C F0354 status boxes read champion slots by semantic slot.
         * Only storage slots with a clear shared M11 equivalent are mirrored;
         * CSB chest slots remain runtime-owned. */
        for (csb_slot = 0; csb_slot < CSB_V1_SLOT_COUNT; ++csb_slot) {
            int dst_slot = csb_v1_runtime_m11_inventory_slot_for_csb_slot(csb_slot);
            if (dst_slot >= 0 && dst_slot < CHAMPION_SLOT_COUNT) {
                dst->inventory[dst_slot] = src->Slots[csb_slot];
            }
        }
        dst->load = src->Load;
        dst->maxLoad = 0u;
        dst->cell = (unsigned char)(src->Cell & 3u);
        dst->direction = (unsigned char)(src->Direction & 3u);
        dst->wounds = src->Wounds;
        dst->poisonDose = src->PoisonDose;
        dst->food = src->Food;
        dst->water = src->Water;
        dst->actionDefense = 0;
        dst->actionIndex = src->ActionIndex;
        (void)csb_v1_runtime_copy_portrait_compat(dst, src);
    }
    return 1;
}

void csb_v1_runtime_m11_mirror_receipt_init_pc34(
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_runtime_view_state_receipt_init_pc34(&receipt->view);
    csb_v1_runtime_party_mirror_receipt_init_pc34(&receipt->party);
}

int csb_v1_runtime_m11_mirror_receipt_from_profile_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *out_receipt)
{
    uint16_t leader_hand;
    if (!out_receipt) {
        return 0;
    }
    csb_v1_runtime_m11_mirror_receipt_init_pc34(out_receipt);
    if (!profile) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C F0806 and CSBWin runtime handoff both publish
     * one live party pose/profile boundary before the host mirrors UI state. */
    if (!csb_v1_runtime_view_state_receipt_from_profile_pc34(
            profile,
            &out_receipt->view)) {
        return 0;
    }
    if (!csb_v1_runtime_party_mirror_receipt_from_profile_pc34(
            profile,
            &out_receipt->party)) {
        return 0;
    }
    leader_hand = csb_v1_runtime_export_leader_hand_thing(profile);
    if (leader_hand == THING_NONE || leader_hand == THING_ENDOFLIST) {
        out_receipt->leader_hand_present = 0;
        out_receipt->leader_hand_thing = THING_NONE;
        out_receipt->leader_hand_icon_index = -1;
        out_receipt->leader_hand_object_name[0] = '\0';
    } else {
        out_receipt->leader_hand_present = 1;
        out_receipt->leader_hand_thing = leader_hand;
        out_receipt->leader_hand_icon_index =
            csb_v1_runtime_object_icon_index(profile, leader_hand);
        (void)csb_v1_runtime_object_name(
            profile,
            leader_hand,
            out_receipt->leader_hand_object_name,
            sizeof(out_receipt->leader_hand_object_name));
    }
    out_receipt->valid = out_receipt->party.valid ? 1 : 0;
    return 1;
}

static void csb_v1_runtime_apply_timeline_dispatch_side_effects(
    CSB_V1_RuntimeProfile *profile,
    const uint16_t *event_indices,
    int event_count);
static int csb_v1_runtime_pre_dispatch_saved_csbwin_generator_timer(
    CSB_V1_RuntimeProfile *profile,
    uint16_t event_index,
    uint16_t queue_slot);
static int csb_v1_runtime_dispatch_saved_csbwin_timer_dsa(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    uint16_t queue_slot);
static int csb_v1_runtime_dispatch_saved_csbwin_falsewall_clear(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    CSB_V1_CSBWin512TimerSummary *timer,
    uint16_t timer_index,
    uint16_t queue_slot);

/* ── Internal tick helper ─────────────────────────────────────────────── */

static void csb_v1_fire_tick(CSB_V1_RuntimeProfile *profile)
{
    struct DM1_EventQueue_V1 queue_snapshot;
    uint16_t source_queue_slots[DM1_DISPATCH_MAX_PER_TICK];
    uint16_t source_event_indices[DM1_DISPATCH_MAX_PER_TICK];
    uint8_t source_generator_consumed[DM1_DISPATCH_MAX_PER_TICK] = { 0 };
    int source_count = 0;
    int i;
    int dispatched;

    /* Source: ReDMCSB GAMELOOP.C F0002 lines 69-124 calls
     * F0065_SOUND_ProcessPendingSound before F0261_TIMELINE_Process_CPSEF()
     * and then increments G0313_ul_GameTime.  TIMELINE.C F0240 lines
     * 702-708 expires the first heap event when event_time <= G0313_ul_GameTime. */
    (void)csb_v1_audio_runtime_flush_pending(&profile->audio_runtime);
    profile->timeline_queue.gameTick = profile->game_time;
    queue_snapshot = profile->timeline_queue;
    while (source_count < DM1_DISPATCH_MAX_PER_TICK &&
           dm1v1_event_is_first_expired(&queue_snapshot)) {
        uint16_t event_index = queue_snapshot.timeline[0];
        struct DM1_Event_V1 ignored;

        source_event_indices[source_count] = event_index;
        source_queue_slots[source_count++] = event_index < DM1_EVENT_MAX_COUNT
            ? profile->csbwin_timeline_event_queue_slot[event_index]
            : CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
        if (!dm1v1_event_extract_first(&queue_snapshot, &ignored)) break;
    }
    for (i = 0; i < source_count; ++i) {
        if (csb_v1_runtime_pre_dispatch_saved_csbwin_generator_timer(
                profile, source_event_indices[i], source_queue_slots[i])) {
            /* CSBWin owns TT_60/TT_61 before the shared M10 C60/C61 group
             * dispatch can reinterpret timerObj8 as generic event payload.
             * Do not alter the queued EVENT before F0239 extracts it: the
             * queue still has to retire the exact saved TIMER slot.  Mark its
             * dispatch receipt instead, below, before any M10 side effects. */
            source_generator_consumed[i] = 1u;
        }
    }
    memset(&profile->last_timeline_dispatch, 0,
           sizeof(profile->last_timeline_dispatch));
    dispatched = dm1v1_event_process_tick(&profile->timeline_queue,
                                          &profile->last_timeline_dispatch);
    if (dispatched > 0) {
        profile->timeline_dispatch_count += (uint32_t)dispatched;
        for (i = 0; i < dispatched && i < source_count; ++i) {
            if (source_event_indices[i] < DM1_EVENT_MAX_COUNT) {
                profile->csbwin_timeline_event_queue_slot[source_event_indices[i]] =
                    CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
            }
            if (source_generator_consumed[i]) {
                /* The source handler either requeued the authenticated
                 * ProcessTimer60and61 branch or consumed its unsupported
                 * receipt.  In both cases shared C60/C61 must see nothing. */
                profile->last_timeline_dispatch.records[i].eventType =
                    DM1_EVENT_NONE;
                profile->last_timeline_dispatch.records[i].dispatchKind =
                    DM1_DISPATCH_UNSUPPORTED;
                continue;
            }
            if (csb_v1_runtime_dispatch_saved_csbwin_timer_dsa(
                    profile, &profile->last_timeline_dispatch.records[i],
                    source_queue_slots[i])) {
                /* Source-owned saved CSBWin timer functions can numerically
                 * alias shared DM1 events. Their mutations belong exclusively
                 * to the source-identity bridge below. */
                profile->last_timeline_dispatch.records[i].eventType =
                    DM1_EVENT_NONE;
            }
        }
        csb_v1_runtime_apply_timeline_dispatch_side_effects(
            profile, source_event_indices, dispatched);

        /* A supported CSBWin timer successor retains its original TIMER
         * record but must pass through Timer.cpp's heap adjustment before a
         * core save can be emitted. This is intentionally a no-op when a
         * timer was consumed or any live event lost its source receipt. */
        (void)csb_v1_runtime_reheapify_live_csbwin_timer_queue(profile);
    }

    profile->game_time++;
    profile->tick_count++;
    profile->game_ticks += CSB_V1_TICK_MS_NOMINAL;

    /* Chaos Magic spell grid is versioned on each tick.
     * F0211_CASTER_ClearSpellEffects increments spell_grid_version at world load.
     * We advance chaos_level here for ambient oscillation.
     * Source: CSBWin Magic.cpp ambient loop (no direct ReDMCSB ref). */
    if (profile->chaos_magic.magic_initialized) {
        uint32_t beat = profile->tick_count % 20U;
        profile->chaos_magic.chaos_level = (uint8_t)((beat < 10U) ? 0U : 1U);
        profile->chaos_magic.spell_grid_version++;
    }
}

static int csb_v1_runtime_default_wall_probe(
    const CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    void *context)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;
    int door_state;

    (void)context;
    dungeon = (profile && profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) {
        /* A CSB party step has no source-authoritative destination without
         * the hash-verified dungeon loaded by the runtime boot boundary. */
        return 1;
    }
    level = csb_v1_dungeon_get_current_level();
    if (level < 0 || level >= dungeon->level_count) {
        level = 0;
    }
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0) return 1;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    if (dungeon->square_bytes == 1) {
        if (square_type == 0) return 1;
        if (square_type == 4) {
            /* ReDMCSB: CLIKMENU.C F0366 lines 282-286 blocks doors
             * unless M036_DOOR_STATE is open, one-fourth closed, or
             * destroyed.  DEFS.H lines 1039-1046 define states 0,1,5. */
            door_state = raw_square & 0x07;
            return door_state != 0 && door_state != 1 && door_state != 5;
        }
        if (square_type == 6) {
            /* ReDMCSB: CLIKMENU.C F0366 lines 287-290 blocks fake walls
             * unless MASK0x0004_FAKEWALL_OPEN or
             * MASK0x0001_FAKEWALL_IMAGINARY is set. */
            return !(raw_square & 0x04) && !(raw_square & 0x01);
        }
        return 0;
    }
    if (square_type == 4) {
        door_state = raw_square & 0x07;
        return door_state != 0 && door_state != 1 && door_state != 5;
    }
    if (square_type == 6) {
        return !(raw_square & 0x04) && !(raw_square & 0x01);
    }
    return square_type == 1;
}

static int csb_v1_runtime_square_has_group(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y);

static int csb_v1_runtime_party_destination_is_blocked(
    const CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    void *context)
{
    const CSB_V1_DungeonData *dungeon;
    int level;

    (void)context;
    if (csb_v1_runtime_default_wall_probe(profile, map_x, map_y, NULL)) {
        return 1;
    }
    dungeon = (profile && profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!profile || !dungeon || !dungeon->raw_data) return 1;
    level = profile->current_level;
    if (level < 0 || level >= dungeon->level_count) {
        level = csb_v1_dungeon_get_current_level();
    }
    if (level < 0 || level >= dungeon->level_count) return 1;

    /* ReDMCSB: CLIKMENU.C F0366 lines 305-310 rejects a destination
     * occupied by F0175_GROUP_GetThing before F0267 can commit the party. */
    return csb_v1_runtime_square_has_group(dungeon, level, map_x, map_y);
}

static void csb_v1_runtime_schedule_party_bump_group_reaction(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y,
    CSB_V1_InputCommandRuntimeResult *result)
{
    struct DM1_Event_V1 event;

    if (!profile || !result || level < 0 || level > 255 ||
        map_x < 0 || map_x > 255 || map_y < 0 || map_y > 255) {
        return;
    }
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(level, profile->game_time + 1u);
    event.type = DM1_EVENT_GROUP_REACTION_PARTY_IS_ADJACENT;
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    if (dm1v1_event_add(&profile->timeline_queue, &event) >= 0) {
        result->movement_group_reaction_scheduled = 1;
    }
    /* ReDMCSB: CLIKMENU.C F0366 lines 305-310 calls GROUP.C F0209 with
     * CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT. GROUP.C
     * F0209 lines 1971-1979 materializes C31 with its fixed one-tick delay. */
}

static int csb_v1_runtime_sample_destination_square(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;

    if (!profile || !result || !result->movement_step_attempted) return -1;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return -1;

    level = profile->current_level;
    if (level < 0 || level >= dungeon->level_count) {
        level = csb_v1_dungeon_get_current_level();
    }
    if (level < 0 || level >= dungeon->level_count) {
        level = 0;
    }

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        result->movement_destination_x,
        result->movement_destination_y);
    if (raw_square < 0) return -1;

    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    result->movement_destination_raw_square = raw_square;
    result->movement_destination_square_type = square_type;
    if (square_type == 4) {
        result->movement_destination_door_state = raw_square & 0x07;
    }
    return level;
}

static int csb_v1_runtime_current_square_is_stairs(
    CSB_V1_RuntimeProfile *profile,
    int *out_raw_square,
    int *out_level)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;

    if (out_raw_square) *out_raw_square = -1;
    if (out_level) *out_level = -1;
    if (!profile) return 0;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return 0;
    level = profile->current_level;
    if (level < 0 || level >= dungeon->level_count) {
        level = csb_v1_dungeon_get_current_level();
    }
    if (level < 0 || level >= dungeon->level_count) return 0;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        profile->party_x,
        profile->party_y);
    if (raw_square < 0) return 0;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    if (square_type != 3) return 0;
    if (out_raw_square) *out_raw_square = raw_square;
    if (out_level) *out_level = level;
    return 1;
}

static int csb_v1_runtime_decode_destination_teleporter(
    const CSB_V1_DungeonData *dungeon,
    int level,
    const CSB_V1_InputCommandRuntimeResult *result,
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *out_teleporter,
    int *out_scope)
{
    int raw_square;
    int first_thing;
    int thing_type;
    int thing_size;
    uint16_t word;
    uint16_t target_word;
    const uint8_t *record;

    if (out_scope) *out_scope = 0;
    if (!dungeon || !result || !out_teleporter) return -1;
    if (result->movement_destination_square_type != 5) return -1;
    if (!(result->movement_destination_raw_square & 0x08)) return 0;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        result->movement_destination_x,
        result->movement_destination_y);
    if (first_thing < 0) return 0;
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        (uint16_t)first_thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_type != 1 || thing_size < 6) return 0;

    /* ReDMCSB: DEFS.H TELEPORTER for PC/I34E stores Next, then a packed
     * word with TargetMapX/Y, Rotation, AbsoluteRotation, Scope, Audible,
     * followed by Unreferenced and TargetMapIndex bytes. */
    word = (uint16_t)record[2] | (uint16_t)((uint16_t)record[3] << 8);
    target_word = (uint16_t)record[4] | (uint16_t)((uint16_t)record[5] << 8);
    memset(out_teleporter, 0, sizeof(*out_teleporter));
    out_teleporter->target_map_x = (int)(word & 0x1Fu);
    out_teleporter->target_map_y = (int)((word >> 5) & 0x1Fu);
    out_teleporter->rotation = (int)((word >> 10) & 0x03u);
    out_teleporter->absolute_rotation = (word & 0x1000u) ? 1 : 0;
    if (out_scope) *out_scope = (int)((word >> 13) & 0x03u);
    out_teleporter->audible = (word & 0x8000u) ? 1 : 0;
    out_teleporter->target_map_index = (int)((target_word >> 8) & 0xFFu);
    return 1;
}

static int csb_v1_runtime_decode_teleporter_at_square(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    int raw_square,
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *out_teleporter,
    int *out_scope)
{
    int first_thing;
    int thing_type;
    int thing_size;
    uint16_t word;
    uint16_t target_word;
    const uint8_t *record;

    if (out_scope) *out_scope = 0;
    if (!dungeon || !out_teleporter) return -1;
    if (((raw_square >> 5) & 0x07) != 5) return -1;
    if (!(raw_square & 0x08)) return 0;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (first_thing < 0) return 0;
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        (uint16_t)first_thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_type != 1 || thing_size < 6) return 0;

    word = (uint16_t)record[2] | (uint16_t)((uint16_t)record[3] << 8);
    target_word = (uint16_t)record[4] | (uint16_t)((uint16_t)record[5] << 8);
    memset(out_teleporter, 0, sizeof(*out_teleporter));
    out_teleporter->target_map_x = (int)(word & 0x1Fu);
    out_teleporter->target_map_y = (int)((word >> 5) & 0x1Fu);
    out_teleporter->rotation = (int)((word >> 10) & 0x03u);
    out_teleporter->absolute_rotation = (word & 0x1000u) ? 1 : 0;
    if (out_scope) *out_scope = (int)((word >> 13) & 0x03u);
    out_teleporter->audible = (word & 0x8000u) ? 1 : 0;
    out_teleporter->target_map_index = (int)((target_word >> 8) & 0xFFu);
    return 1;
}

static void csb_v1_runtime_apply_destination_teleporter(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 teleporter_result;
    int level;
    int scope = 0;
    int decoded;

    if (!profile || !result || !result->movement_step_applied) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;

    level = csb_v1_runtime_sample_destination_square(profile, result);
    if (level < 0) return;
    if (result->movement_destination_square_type != 5) return;
    result->teleporter_open =
        (result->movement_destination_raw_square & 0x08) ? 1 : 0;
    if (!result->teleporter_open) return;

    decoded = csb_v1_runtime_decode_destination_teleporter(
        dungeon,
        level,
        result,
        &teleporter,
        &scope);
    if (decoded <= 0) return;
    result->teleporter_scope = scope;
    result->teleporter_rotation = teleporter.rotation;
    result->teleporter_absolute_rotation = teleporter.absolute_rotation;
    result->teleporter_audible = teleporter.audible;
    result->teleporter_target_x = teleporter.target_map_x;
    result->teleporter_target_y = teleporter.target_map_y;
    result->teleporter_target_level = teleporter.target_map_index;
    if (!(scope & 0x02)) return;
    if (teleporter.target_map_index < 0 ||
        teleporter.target_map_index >= dungeon->level_count) {
        return;
    }

    /* ReDMCSB: MOVESENS.C F0267 lines 475-518 enters an open
     * C05_ELEMENT_TELEPORTER, requires object/party scope for the party,
     * moves to TargetMapX/Y/TargetMapIndex, and applies teleporter rotation
     * through CHAMPION.C F0284.  This bounded runtime handoff applies one
     * party teleporter only; chained teleporters, sounds, redraw timing, and
     * object/group/projectile teleportation remain separate work. */
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (csb_v1_teleporter_rotation_apply_party_pc34_compat(
            profile,
            &teleporter,
            &teleporter_result) != 0) {
        return;
    }
    csb_v1_dungeon_set_current_level(profile->current_level);
    result->new_party_level = profile->current_level;
    result->teleporter_transition_applied = 1;
}

static void csb_v1_runtime_apply_destination_stairs(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;
    int stair_up;
    int target_level;
    int target_x;
    int target_y;
    int exit_direction;

    if (!profile || !result || !result->movement_step_applied ||
        result->teleporter_transition_applied ||
        result->pit_fall_applied ||
        result->stair_transition_applied) {
        return;
    }
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;

    level = csb_v1_runtime_sample_destination_square(profile, result);
    if (level < 0) return;
    raw_square = result->movement_destination_raw_square;
    square_type = result->movement_destination_square_type;
    if (square_type != 3) return;

    /* ReDMCSB: CLIKMENU.C F0366 lines 264-276 reaches C03_ELEMENT_STAIRS
     * and calls F0364_COMMAND_TakeStairs.  F0364 lines 136-138 resolves
     * the destination through DUNGEON.C F0154, then applies F0155 exit
     * direction through CHAMPION.C F0284. */
    stair_up = (raw_square & 0x04) ? 1 : 0;
    target_x = result->movement_destination_x;
    target_y = result->movement_destination_y;
    result->stair_up = stair_up;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (!csb_v1_runtime_location_after_level_change(
            dungeon,
            level,
            stair_up ? -1 : 1,
            &target_x,
            &target_y,
            &target_level)) {
        return;
    }

    exit_direction = csb_v1_runtime_stairs_exit_direction(
        dungeon, target_level, target_x, target_y);
    if (exit_direction < 0) return;
    profile->current_level = target_level;
    profile->party_x = target_x;
    profile->party_y = target_y;
    profile->party_state.PartyMapX = target_x;
    profile->party_state.PartyMapY = target_y;
    csb_v1_dungeon_set_current_level(target_level);
    (void)csb_v1_runtime_rotate_party(profile, exit_direction);
    result->new_party_level = target_level;
    result->stair_transition_applied = 1;
}

static void csb_v1_runtime_take_current_stairs(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int stair_up;
    int target_level;
    int target_x;
    int target_y;
    int exit_direction;

    if (!profile || !result) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;
    if (!csb_v1_runtime_current_square_is_stairs(
            profile,
            &raw_square,
            &level)) {
        return;
    }

    /* ReDMCSB: CLIKMENU.C F0365 lines 164-168 and F0366 lines 264-266
     * route turning on stairs and moving backward on stairs to
     * F0364_COMMAND_TakeStairs instead of applying the requested turn or
     * one-square backward step.  F0364 lines 135-138 processes party removal
     * from the stairs square, resolves DUNGEON.C F0154, then applies
     * F0155/F0284 exit direction. */
    stair_up = (raw_square & 0x04) ? 1 : 0;
    target_x = profile->party_x;
    target_y = profile->party_y;
    result->movement_destination_x = profile->party_x;
    result->movement_destination_y = profile->party_y;
    result->movement_destination_raw_square = raw_square;
    result->movement_destination_square_type = 3;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    result->stair_up = stair_up;
    if (!csb_v1_runtime_location_after_level_change(
            dungeon,
            level,
            stair_up ? -1 : 1,
            &target_x,
            &target_y,
            &target_level)) {
        return;
    }

    exit_direction = csb_v1_runtime_stairs_exit_direction(
        dungeon, target_level, target_x, target_y);
    if (exit_direction < 0) return;
    profile->current_level = target_level;
    profile->party_x = target_x;
    profile->party_y = target_y;
    profile->party_state.PartyMapX = target_x;
    profile->party_state.PartyMapY = target_y;
    csb_v1_dungeon_set_current_level(target_level);
    (void)csb_v1_runtime_rotate_party(profile, exit_direction);
    result->new_party_level = target_level;
    result->stair_transition_applied = 1;
}

static int csb_v1_runtime_apply_party_fall_damage(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result,
    int map_index,
    int map_x,
    int map_y)
{
    struct RngState_Compat rng;
    int random_window;
    int base_attack;
    int damaged_count = 0;
    int total_damage = 0;
    int wound_mask = 0;
    int i;

    if (!profile || !profile->party_state_valid ||
        profile->party_state.ChampionCount <= 0) {
        return 0;
    }

    random_window = (20 >> 3) + 1;
    base_attack = 20 - random_window;
    random_window <<= 1;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)(map_index & 0xff) << 4) ^
            ((uint32_t)(map_x & 0xff) << 12) ^
            ((uint32_t)(map_y & 0xff) << 20) ^
            0xF0324u);

    /* ReDMCSB MOVESENS.C F0267 lines 590-603 applies
     * CHAMPION.C F0324 after a party pit fall with attack 20,
     * MASK0x0010_WOUND_LEGS | MASK0x0020_WOUND_FEET, and C2_ATTACK_SELF.
     * CHAMPION.C F0324 lines 1991-2022 randomizes attack by +/- 1/8 for
     * each champion before F0321 scaling/wound selection.  This bounded
     * CSB bridge uses deterministic local RNG until the runtime owns the
     * original global RNG stream. */
    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; ++i) {
        CSB_V1_Champion *champion = &profile->party_state.Champions[i];
        struct CombatantChampionSnapshot_Compat defender;
        int randomized_attack;
        int scaled_attack = 0;
        int selected_wounds = 0;

        if (champion->CurrentHealth <= 0 ||
            (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
            continue;
        }

        randomized_attack = base_attack +
            F0732_COMBAT_RngRandom_Compat(&rng, random_window);
        if (randomized_attack < 1) randomized_attack = 1;

        if (!csb_v1_runtime_fill_defender_combat_snapshot(
                profile,
                i,
                &defender) ||
            !F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
                COMBAT_ATTACK_SELF,
                randomized_attack,
                COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET,
                &defender,
                &rng,
                &scaled_attack,
                NULL) ||
            scaled_attack <= 0) {
            continue;
        }

        if (!F0739c_COMBAT_SelectChampionWoundsF0321Rng_Compat(
                scaled_attack,
                COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET,
                &defender,
                &rng,
                &selected_wounds,
                NULL)) {
            selected_wounds = 0;
        }

        champion->Wounds = (uint16_t)(champion->Wounds |
                                      (uint16_t)selected_wounds);
        wound_mask |= selected_wounds;
        if (scaled_attack >= champion->CurrentHealth) {
            total_damage += champion->CurrentHealth;
            champion->CurrentHealth = 0;
            csb_v1_runtime_mark_champion_dead(profile, i);
        } else {
            champion->CurrentHealth =
                (int16_t)(champion->CurrentHealth - scaled_attack);
            total_damage += scaled_attack;
        }
        damaged_count++;
    }

    if (result) {
        result->pit_fall_damaged_champion_count += damaged_count;
        result->pit_fall_total_damage += total_damage;
        result->pit_fall_wound_mask |= wound_mask;
    }
    return damaged_count;
}

static void csb_v1_runtime_apply_destination_pit(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int target_level;
    int target_x;
    int target_y;

    if (!profile || !result || !result->movement_step_applied ||
        result->teleporter_transition_applied ||
        result->stair_transition_applied) {
        return;
    }
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;

    level = csb_v1_runtime_sample_destination_square(profile, result);
    if (level < 0) return;
    raw_square = result->movement_destination_raw_square;
    if (result->movement_destination_square_type != 2) return;
    result->pit_open = (raw_square & 0x08) ? 1 : 0;
    if (!result->pit_open || (raw_square & 0x01)) return;

    /* ReDMCSB: MOVESENS.C F0267 lines 538-603 handles an open,
     * non-imaginary C02_ELEMENT_PIT by calling DUNGEON.C F0154 for a
     * downward map transition, then applies F0324 party fall damage.
     * Rope, sounds, and view redraw timing stay separate. */
    target_x = result->movement_destination_x;
    target_y = result->movement_destination_y;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (!csb_v1_runtime_location_after_level_change(
            dungeon,
            level,
            1,
            &target_x,
            &target_y,
            &target_level)) {
        return;
    }

    profile->current_level = target_level;
    profile->party_x = target_x;
    profile->party_y = target_y;
    profile->party_state.PartyMapX = target_x;
    profile->party_state.PartyMapY = target_y;
    csb_v1_dungeon_set_current_level(target_level);
    result->new_party_level = target_level;
    result->pit_fall_applied = 1;
    (void)csb_v1_runtime_apply_party_fall_damage(
        profile,
        result,
        target_level,
        target_x,
        target_y);
}

static void csb_v1_runtime_copy_first_teleporter_result(
    CSB_V1_InputCommandRuntimeResult *result,
    const CSB_V1_InputCommandRuntimeResult *step)
{
    if (!result || !step) return;
    if (!result->teleporter_transition_applied) {
        result->teleporter_open = step->teleporter_open;
        result->teleporter_scope = step->teleporter_scope;
        result->teleporter_absolute_rotation = step->teleporter_absolute_rotation;
        result->teleporter_rotation = step->teleporter_rotation;
        result->teleporter_audible = step->teleporter_audible;
        result->teleporter_target_x = step->teleporter_target_x;
        result->teleporter_target_y = step->teleporter_target_y;
        result->teleporter_target_level = step->teleporter_target_level;
    }
    result->teleporter_transition_applied = 1;
    result->teleporter_chain_count++;
}

static void csb_v1_runtime_copy_first_pit_result(
    CSB_V1_InputCommandRuntimeResult *result,
    const CSB_V1_InputCommandRuntimeResult *step)
{
    if (!result || !step) return;
    if (!result->pit_fall_applied) {
        result->pit_open = step->pit_open;
    }
    result->pit_fall_applied = 1;
    result->pit_chain_count++;
    result->pit_fall_damaged_champion_count +=
        step->pit_fall_damaged_champion_count;
    result->pit_fall_total_damage += step->pit_fall_total_damage;
    result->pit_fall_wound_mask |= step->pit_fall_wound_mask;
}

static void csb_v1_runtime_apply_destination_chain(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    int i;

    if (!profile || !result || !result->movement_step_applied) return;

    /* ReDMCSB: MOVESENS.C F0267 lines 468-574 repeats teleporter/pit
     * consequences in one move result.  PC34/I34E MEDIA529 caps chained
     * moves at 100, covering teleporter-to-teleporter and pit-series
     * routes while avoiding infinite self-feeding dungeon setups.  This
     * party-only runtime bridge keeps object/group/projectile movement,
     * audio, fall damage, rope, and redraw timing as separate work. */
    for (i = 0; i < 100; ++i) {
        CSB_V1_InputCommandRuntimeResult step;
        int before_x;
        int before_y;
        int before_level;
        int self_target_teleporter;

        memset(&step, 0, sizeof(step));
        step.movement_step_attempted = 1;
        step.movement_step_applied = 1;
        step.movement_destination_x = profile->party_x;
        step.movement_destination_y = profile->party_y;
        step.old_party_level = profile->current_level;
        step.new_party_level = profile->current_level;
        if (csb_v1_runtime_sample_destination_square(profile, &step) < 0) {
            break;
        }

        before_x = profile->party_x;
        before_y = profile->party_y;
        before_level = profile->current_level;
        if (step.movement_destination_square_type == 5) {
            csb_v1_runtime_apply_destination_teleporter(profile, &step);
            if (!step.teleporter_transition_applied) {
                if (!result->teleporter_open) {
                    result->teleporter_open = step.teleporter_open;
                }
                break;
            }
            self_target_teleporter =
                step.teleporter_target_x == before_x &&
                step.teleporter_target_y == before_y &&
                step.teleporter_target_level == before_level;
            csb_v1_runtime_copy_first_teleporter_result(result, &step);
            result->chained_move_count++;
            result->new_party_level = profile->current_level;
            if (self_target_teleporter) {
                break;
            }
            continue;
        }

        if (step.movement_destination_square_type == 2) {
            csb_v1_runtime_apply_destination_pit(profile, &step);
            if (!step.pit_fall_applied) {
                if (!result->pit_open) {
                    result->pit_open = step.pit_open;
                }
                break;
            }
            csb_v1_runtime_copy_first_pit_result(result, &step);
            result->chained_move_count++;
            result->new_party_level = profile->current_level;
            continue;
        }

        break;
    }
    if (i >= 100) {
        result->chained_move_limit_hit = 1;
    }
}

static int csb_v1_runtime_square_event_type_for_sensor_target(int square_type)
{
    static const int event_type_by_square_type[7] = {
        DM1_EVENT_WALL,
        DM1_EVENT_CORRIDOR,
        DM1_EVENT_PIT,
        DM1_EVENT_NONE,
        DM1_EVENT_DOOR,
        DM1_EVENT_TELEPORTER,
        DM1_EVENT_FAKEWALL
    };
    if (square_type < 0 || square_type >= 7) return DM1_EVENT_NONE;
    return event_type_by_square_type[square_type];
}

static int csb_v1_runtime_sensor_next_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing)
{
    const uint8_t *record;
    int thing_type;
    int thing_size;

    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_size < 2) return 0xFFFE;
    return (int)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
}

static uint16_t csb_v1_runtime_read_u16(const uint8_t *p)
{
    if (!p) return 0;
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static int csb_v1_runtime_stage_openroom_text_message(
    CSB_V1_RuntimeProfile *profile, uint16_t thing, uint16_t text_word,
    CSB_V1_RuntimeTextMessageReceipt *out_receipt)
{
    CSB_V1_DungeonData *dungeon;
    struct DungeonTextString_Compat text_string;
    struct DungeonThings_Compat things;
    int text_offset;
    int decoded;

    if (!profile || !out_receipt || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    text_offset = (int)((text_word >> 3) & 0x1fffu);
    if (!(text_word & 0x0001u) || dungeon->text_data_base < 0 ||
        dungeon->text_word_count <= 0 || !dungeon->raw_data ||
        text_offset < 0 || text_offset >= dungeon->text_word_count ||
        (long)dungeon->text_data_base +
            (long)dungeon->text_word_count * 2L > dungeon->raw_size) {
        return 0;
    }

    memset(&text_string, 0, sizeof(text_string));
    memset(&things, 0, sizeof(things));
    memset(out_receipt, 0, sizeof(*out_receipt));
    text_string.visible = 1;
    text_string.textDataWordOffset = (unsigned short)text_offset;
    things.textData = (unsigned short *)(void *)(dungeon->raw_data +
                                                 dungeon->text_data_base);
    things.textDataWordCount = dungeon->text_word_count;
    things.textStrings = &text_string;
    things.textStringCount = 1;
    decoded = F0508_DUNGEON_DecodeTextStringThing_Compat(
        &things, 0, DUNGEON_TEXT_TYPE_MESSAGE, out_receipt->text,
        (int)sizeof(out_receipt->text));
    /* F0168 prepends the source message separator.  C015 owns a single
     * QuePrintLines row here, so reject multi-line/empty output rather than
     * approximating the unavailable queue and wrapping behavior. */
    if (decoded <= 1 || out_receipt->text[0] != '\n' ||
        strchr(out_receipt->text + 1, '\n') != NULL) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    memmove(out_receipt->text, out_receipt->text + 1,
            strlen(out_receipt->text));
    out_receipt->valid = 1;
    out_receipt->text_thing = thing;
    out_receipt->source_game_time = profile->game_time;
    return 1;
}

static int csb_v1_runtime_dsa_discard_text(void *user)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;

    if (!profile) return 0;
    /* CSBWin DSA.cpp STKOP_DiscardText delegates to the current text owner.
     * Firestaff has exactly one such owner: the authenticated TT_OPENROOM
     * DB2/F0168 receipt.  Clearing an absent source receipt is the original
     * no-op; it must never manufacture a generic host message surface. */
    memset(&profile->csbwin_text_message_receipt, 0,
           sizeof(profile->csbwin_text_message_receipt));
    return 1;
}

static void csb_v1_runtime_write_u16(uint8_t *p, uint16_t value)
{
    if (!p) return;
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static void csb_v1_runtime_decode_sensor_words(
    uint16_t next_word,
    uint16_t type_data,
    uint16_t flags_word,
    uint16_t target_word,
    struct DungeonSensor_Compat *out_sensor)
{
    if (!out_sensor) return;
    memset(out_sensor, 0, sizeof(*out_sensor));
    out_sensor->next = next_word;
    out_sensor->sensorType = (unsigned char)(type_data & 0x007Fu);
    out_sensor->sensorData = (unsigned short)(type_data >> 7);
    out_sensor->onceOnly = (unsigned char)((flags_word >> 2) & 0x01u);
    out_sensor->effect = (unsigned char)((flags_word >> 3) & 0x03u);
    out_sensor->revertEffect = (unsigned char)((flags_word >> 5) & 0x01u);
    out_sensor->audible = (unsigned char)((flags_word >> 6) & 0x01u);
    out_sensor->value = (unsigned char)((flags_word >> 7) & 0x0Fu);
    out_sensor->localEffect = (unsigned char)((flags_word >> 11) & 0x01u);
    out_sensor->ornamentOrdinal = (unsigned char)((flags_word >> 12) & 0x0Fu);
    out_sensor->targetCell = (unsigned char)((target_word >> 4) & 0x03u);
    out_sensor->targetMapX = (unsigned char)((target_word >> 6) & 0x1Fu);
    out_sensor->targetMapY = (unsigned char)((target_word >> 11) & 0x1Fu);
    out_sensor->localMultiple = (unsigned short)(target_word & 0x0FFFu);
}

static int csb_v1_runtime_projectile_subtype_from_explosion_thing(
    uint16_t associated_thing)
{
    unsigned int explosion_type;
    if (associated_thing < DM1_THING_FIRST_EXPLOSION) {
        return PROJECTILE_SUBTYPE_FIREBALL;
    }
    explosion_type = (unsigned int)(associated_thing - DM1_THING_FIRST_EXPLOSION);
    switch (explosion_type) {
    case C000_EXPLOSION_FIREBALL:
        return PROJECTILE_SUBTYPE_FIREBALL;
    case C001_EXPLOSION_SLIME:
        return PROJECTILE_SUBTYPE_SLIME;
    case C002_EXPLOSION_LIGHTNING_BOLT:
        return PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    case C003_EXPLOSION_HARM_NON_MATERIAL:
        return PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    case C004_EXPLOSION_OPEN_DOOR:
        return PROJECTILE_SUBTYPE_OPEN_DOOR;
    case C007_EXPLOSION_POISON_CLOUD:
        return PROJECTILE_SUBTYPE_POISON_CLOUD;
    default:
        return PROJECTILE_SUBTYPE_FIREBALL;
    }
}

static int csb_v1_runtime_projectile_attack_type_from_subtype(int subtype)
{
    switch (subtype) {
    case PROJECTILE_SUBTYPE_FIREBALL:
        return COMBAT_ATTACK_FIRE;
    case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:
        return COMBAT_ATTACK_LIGHTNING;
    case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
    case PROJECTILE_SUBTYPE_OPEN_DOOR:
        return COMBAT_ATTACK_MAGIC;
    default:
        return COMBAT_ATTACK_NORMAL;
    }
}

static int csb_v1_runtime_sensor_type_is_explosion_launcher(int sensor_type)
{
    return sensor_type == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION ||
           sensor_type == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION;
}

static int csb_v1_runtime_sensor_type_is_square_object_launcher(int sensor_type)
{
    return sensor_type == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ ||
           sensor_type == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ;
}

static int csb_v1_runtime_sensor_type_is_new_object_launcher(int sensor_type)
{
    return sensor_type == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ ||
           sensor_type == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ;
}

static uint8_t *csb_v1_runtime_mutable_thing_record(
    CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int *out_type,
    int *out_size)
{
    const uint8_t *record;

    if (out_type) *out_type = -1;
    if (out_size) *out_size = 0;
    if (!dungeon || !dungeon->raw_data) return NULL;
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        out_type,
        NULL,
        out_size);
    if (!record) return NULL;
    return dungeon->raw_data + (record - (const uint8_t *)dungeon->raw_data);
}

static uint8_t *csb_v1_runtime_square_first_thing_ptr(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int i;
    int column_index = 0;
    int column_counts_base;
    int thing_index;
    int thing_offset;
    int square_offset;

    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1) return NULL;
    if (level < 0 || level >= dungeon->level_count) return NULL;
    if (map_x < 0 || map_x >= dungeon->level_widths[level] ||
        map_y < 0 || map_y >= dungeon->level_heights[level]) {
        return NULL;
    }
    square_offset = dungeon->level_offsets[level] +
                    map_x * dungeon->level_heights[level] +
                    map_y;
    if (square_offset < 0 || square_offset >= dungeon->raw_size) return NULL;
    if ((dungeon->raw_data[square_offset] & 0x10u) == 0u) return NULL;

    column_counts_base = 44 + dungeon->level_count * 16;
    for (i = 0; i < level; ++i) {
        column_index += dungeon->level_widths[i];
    }
    column_counts_base += (column_index + map_x) * 2;
    if (column_counts_base + 2 > dungeon->raw_size) return NULL;
    thing_index = (int)csb_v1_runtime_read_u16(
        dungeon->raw_data + column_counts_base);
    for (i = 0; i < map_y; ++i) {
        if (dungeon->raw_data[dungeon->level_offsets[level] +
                              map_x * dungeon->level_heights[level] + i] &
            0x10u) {
            thing_index++;
        }
    }
    if (thing_index < 0 || thing_index >= dungeon->square_first_thing_count) {
        return NULL;
    }
    thing_offset = dungeon->square_first_thing_base + thing_index * 2;
    if (thing_offset + 2 > dungeon->raw_size) return NULL;
    return dungeon->raw_data + thing_offset;
}

static uint8_t *csb_v1_runtime_create_square_first_thing_ptr(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    uint16_t first_thing)
{
    int i;
    int global_column_index = 0;
    int total_columns = 0;
    int column_counts_base;
    int insertion_index;
    int square_offset;
    int insert_offset;
    int last_offset;
    int move_bytes;

    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1) return NULL;
    if (level < 0 || level >= dungeon->level_count) return NULL;
    if (map_x < 0 || map_x >= dungeon->level_widths[level] ||
        map_y < 0 || map_y >= dungeon->level_heights[level]) {
        return NULL;
    }
    if (dungeon->square_first_thing_count <= 0) return NULL;
    if (dungeon->square_first_thing_base < 0 ||
        dungeon->square_first_thing_base +
            dungeon->square_first_thing_count * 2 > dungeon->raw_size) {
        return NULL;
    }

    square_offset = dungeon->level_offsets[level] +
                    map_x * dungeon->level_heights[level] +
                    map_y;
    if (square_offset < 0 || square_offset >= dungeon->raw_size) return NULL;
    if ((dungeon->raw_data[square_offset] & 0x10u) != 0u) {
        return csb_v1_runtime_square_first_thing_ptr(
            dungeon,
            level,
            map_x,
            map_y);
    }

    last_offset = dungeon->square_first_thing_base +
                  (dungeon->square_first_thing_count - 1) * 2;
    if (csb_v1_runtime_read_u16(dungeon->raw_data + last_offset) != 0xFFFFu) {
        return NULL;
    }

    for (i = 0; i < dungeon->level_count; ++i) {
        if (i < level) global_column_index += dungeon->level_widths[i];
        total_columns += dungeon->level_widths[i];
    }
    global_column_index += map_x;
    column_counts_base = 44 + dungeon->level_count * 16;
    if (column_counts_base + total_columns * 2 > dungeon->raw_size) {
        return NULL;
    }

    insertion_index = (int)csb_v1_runtime_read_u16(
        dungeon->raw_data + column_counts_base + global_column_index * 2);
    for (i = 0; i < map_y; ++i) {
        if (dungeon->raw_data[dungeon->level_offsets[level] +
                              map_x * dungeon->level_heights[level] + i] &
            0x10u) {
            insertion_index++;
        }
    }
    if (insertion_index < 0 ||
        insertion_index >= dungeon->square_first_thing_count) {
        return NULL;
    }

    insert_offset = dungeon->square_first_thing_base + insertion_index * 2;
    move_bytes = (dungeon->square_first_thing_count -
                  insertion_index - 1) * 2;
    if (move_bytes > 0) {
        memmove(dungeon->raw_data + insert_offset + 2,
                dungeon->raw_data + insert_offset,
                (size_t)move_bytes);
    }
    csb_v1_runtime_write_u16(dungeon->raw_data + insert_offset, first_thing);
    dungeon->raw_data[square_offset] |= 0x10u;
    for (i = global_column_index + 1; i < total_columns; ++i) {
        uint8_t *count_ptr = dungeon->raw_data + column_counts_base + i * 2;
        uint16_t count = csb_v1_runtime_read_u16(count_ptr);
        csb_v1_runtime_write_u16(count_ptr, (uint16_t)(count + 1u));
    }
    /* ReDMCSB DUNGEON.C F0163 lines 1790-1829 creates a new square
     * first-thing slot by setting MASK0x0010_THING_LIST_PRESENT, inserting
     * into G0283_pT_SquareFirstThings, and incrementing later cumulative
     * column counters.  The original relies on preallocated free slots and
     * BUG0_08 if none remain; this bridge refuses insertion unless the last
     * slot is THING_NONE. */
    return dungeon->raw_data + insert_offset;
}

static int csb_v1_runtime_find_unused_group_record(
    CSB_V1_DungeonData *dungeon,
    uint8_t **out_record,
    int *out_index)
{
    int i;

    if (out_record) *out_record = NULL;
    if (out_index) *out_index = -1;
    if (!dungeon || !dungeon->raw_data) return 0;
    for (i = 0; i < dungeon->thing_type_counts[4]; ++i) {
        int offset = dungeon->thing_data_bases[4] + i * 16;
        if (offset < 0 || offset + 16 > dungeon->raw_size) return 0;
        if (csb_v1_runtime_read_u16(dungeon->raw_data + offset) == 0xFFFFu) {
            if (out_record) *out_record = dungeon->raw_data + offset;
            if (out_index) *out_index = i;
            return 1;
        }
    }
    return 0;
}

static int csb_v1_runtime_creature_movement_ticks(int creature_type)
{
    static const unsigned char movement_ticks[27] = {
        8, 15, 3, 10, 9, 20, 120, 185, 11,
        21, 17, 255, 7, 5, 10, 18, 13, 1,
        6, 10, 255, 17, 15, 10, 60, 10, 10
    };
    if (creature_type < 0 || creature_type >= 27) return 255;
    return (int)movement_ticks[creature_type];
}

static int csb_v1_runtime_creature_attack_ticks(int creature_type)
{
    static const unsigned char attack_ticks[27] = {
        20, 32, 5, 21, 8, 18, 10, 15, 16,
        14, 12, 8, 7, 10, 20, 19, 8, 16,
        6, 18, 25, 15, 14, 22, 28, 22, 22
    };
    if (creature_type < 0 || creature_type >= 27) return 1;
    return (int)attack_ticks[creature_type];
}

static int csb_v1_runtime_creature_attack_sound_index(int creature_type)
{
    static const signed char attack_sound_ordinal[27] = {
        4, 0, 6, 0, 1, 0, 3, 7, 2,
        10, 2, 0, 11, 9, 0, 5, 10, 0,
        11, 0, 8, 3, 0, 0, 1, 0, 0
    };
    static const signed char creature_sounds_attack[18] = {
        23, 25, 19, 20, 21, 22, 24, 26, 27,
        CSB_V1_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM,
        CSB_V1_SOUND_COMBAT, CSB_V1_SOUND_COMBAT, 25, -1, -1, -1, -1, 23
    };
    int ordinal;

    if (creature_type < 0 || creature_type >= 27) return CSB_V1_SOUND_NONE;
    ordinal = (int)attack_sound_ordinal[creature_type];
    if (ordinal <= 0 || ordinal > 18) return CSB_V1_SOUND_NONE;
    return (int)creature_sounds_attack[ordinal - 1];
}

static int csb_v1_runtime_creature_movement_sound_index(int creature_type)
{
    static const signed char attack_sound_ordinal[27] = {
        4, 0, 6, 0, 1, 0, 3, 7, 2,
        10, 2, 0, 11, 9, 0, 5, 10, 0,
        11, 0, 8, 3, 0, 0, 1, 0, 0
    };
    static const signed char creature_sounds_movement[18] = {
        CSB_V1_SOUND_MOVE_RED_DRAGON, -1,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SWAMP_SLIME_WATER_ELEMENTAL,
        CSB_V1_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SKELETON,
        CSB_V1_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SWAMP_SLIME_WATER_ELEMENTAL,
        CSB_V1_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU
    };
    int ordinal;

    if (creature_type < 0 || creature_type >= 27) return CSB_V1_SOUND_NONE;
    ordinal = (int)attack_sound_ordinal[creature_type];
    if (ordinal <= 0 || ordinal > 18) return CSB_V1_SOUND_NONE;
    return (int)creature_sounds_movement[ordinal - 1];
}

static void csb_v1_runtime_request_creature_movement_sound(
    CSB_V1_RuntimeProfile *profile,
    int creature_type,
    int map_x,
    int map_y)
{
    CsbV1AudioRequest request;
    int sound_index;

    if (!profile) return;
    sound_index = csb_v1_runtime_creature_movement_sound_index(creature_type);
    if (sound_index == CSB_V1_SOUND_NONE) return;

    memset(&request, 0, sizeof(request));
    request.soundIndex = (int16_t)sound_index;
    request.mapX = (int16_t)map_x;
    request.mapY = (int16_t)map_y;
    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
    request.volume = 64;
    request.priority = 4u;
    /* ReDMCSB MOVESENS.C F0267 lines 847-853 calls F0514, which maps
     * CreatureInfo.AttackSoundOrdinal through DUNGEON.C
     * G2003_aauc_CreatureSounds[][C1_MOVEMENT_SOUND] and requests
     * SOUND.C F0064 with C01_MODE_PLAY_IF_PRIORITIZED after a group move. */
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
}

static void csb_v1_runtime_request_creature_attack_sound(
    CSB_V1_RuntimeProfile *profile,
    int creature_type,
    int map_x,
    int map_y)
{
    CsbV1AudioRequest request;
    int sound_index;

    if (!profile) return;
    sound_index = csb_v1_runtime_creature_attack_sound_index(creature_type);
    if (sound_index == CSB_V1_SOUND_NONE) return;

    memset(&request, 0, sizeof(request));
    request.soundIndex = (int16_t)sound_index;
    request.mapX = (int16_t)map_x;
    request.mapY = (int16_t)map_y;
    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
    request.volume = 64;
    request.priority = 6u;
    /* ReDMCSB GROUP.C F0207 lines 1807-1808 maps
     * CreatureInfo.AttackSoundOrdinal through DUNGEON.C
     * G2003_aauc_CreatureSounds[][C0_ATTACK_SOUND], then requests
     * SOUND.C F0064 with C01_MODE_PLAY_IF_PRIORITIZED. */
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
}

static void csb_v1_runtime_request_buzz_sound(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y)
{
    CsbV1AudioRequest request;

    if (!profile) return;
    memset(&request, 0, sizeof(request));
    request.soundIndex = CSB_V1_SOUND_BUZZ;
    request.mapX = (int16_t)map_x;
    request.mapY = (int16_t)map_y;
    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
    request.volume = 64;
    request.priority = 4u;
    /* ReDMCSB GROUP.C F0209 line 2283 requests M560_SOUND_BUZZ at the
     * archenemy double-move destination with C01_MODE_PLAY_IF_PRIORITIZED. */
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
}

static int csb_v1_runtime_direction_from_source_to_destination(
    int source_x,
    int source_y,
    int dest_x,
    int dest_y);
static void csb_v1_runtime_set_active_group_aspect_attacking(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    int attacking);
static void csb_v1_runtime_compact_active_group_state_after_kill(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_index,
    int creature_count);
static void csb_v1_runtime_set_active_group_direction_all(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction);
static void csb_v1_runtime_set_active_group_direction_creature(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_index,
    int creature_count,
    int two_half_square_creatures);
static void csb_v1_runtime_set_active_group_direction_group(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size);
static void csb_v1_runtime_turn_active_group_toward_attack(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size);

static void csb_v1_runtime_schedule_c37_group_event(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    uint32_t delay_ticks)
{
    struct DM1_Event_V1 event;
    int movement_ticks;

    if (!profile || delay_ticks == 0u) return;
    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        map_index,
        profile->game_time + delay_ticks);
    event.type = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    event.priority = (uint8_t)(255 - movement_ticks);
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_schedule_c38_attack_event(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    uint32_t delay_ticks)
{
    struct DM1_Event_V1 event;
    int movement_ticks;

    if (!profile || delay_ticks == 0u || creature_index < 0 ||
        creature_index > 3) {
        return;
    }
    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        map_index,
        profile->game_time + delay_ticks);
    event.type = (uint8_t)(DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 +
                           creature_index);
    event.priority = (uint8_t)(255 - movement_ticks);
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_schedule_c38_followup_event(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    uint32_t attack_delay_ticks)
{
    struct DM1_Event_V1 event;
    uint32_t attack_time;
    uint32_t aspect_time;
    int movement_ticks;

    if (!profile || creature_index < 0 || creature_index > 3 ||
        attack_delay_ticks == 0u) {
        return;
    }
    attack_time = profile->game_time + attack_delay_ticks;
    aspect_time = profile->game_time + 1u;
    if (aspect_time >= attack_time) {
        csb_v1_runtime_schedule_c38_attack_event(
            profile,
            map_index,
            map_x,
            map_y,
            creature_type,
            creature_index,
            attack_delay_ticks);
        return;
    }

    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(map_index, aspect_time);
    event.type = (uint8_t)(DM1_EVENT_UPDATE_ASPECT_CREATURE_0 +
                           creature_index);
    event.priority = (uint8_t)(255 - movement_ticks);
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    event.c_effect = (uint8_t)((attack_time - aspect_time) & 0xffu);
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_apply_creature_aspect_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int creature_index;
    int remaining_ticks;
    int thing;
    int guard;

    if (!profile || !record || !profile->dungeon_handle) return;
    creature_index = record->eventType - DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    if (creature_index < 0 || creature_index > 3) return;
    remaining_ticks = record->effect & 0xff;
    if (remaining_ticks <= 0) remaining_ticks = 1;

    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *thing_record;
        uint16_t flags;
        int thing_type;
        int thing_size;

        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 4 && thing_size >= 16) {
            flags = csb_v1_runtime_read_u16(thing_record + 14);
            if ((flags & 0x000Fu) != 6u) return;
            if (creature_index > (int)((flags >> 5) & 0x03u)) return;

            /* ReDMCSB GROUP.C F0209 lines 2075-2148 handles C33..C36
             * aspect events by preparing the matching C38..C41 behavior
             * event.  F0208 lines 1820-1834 stores the remaining behavior
             * delay in C.Ticks; Firestaff's V1 queue carries it in
             * c_effect/record->effect.  Mirror the native ActiveGroup
             * attack-bit transition here; broader flip/offset RNG remains a
             * later sprite-exact slice. */
            csb_v1_runtime_set_active_group_aspect_attacking(
                profile,
                (uint16_t)thing,
                record->mapIndex,
                record->mapX,
                record->mapY,
                (int)thing_record[4],
                creature_index,
                0);
            csb_v1_runtime_schedule_c38_attack_event(
                profile,
                record->mapIndex,
                record->mapX,
                record->mapY,
                (int)thing_record[4],
                creature_index,
                (uint32_t)remaining_ticks);
            return;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_schedule_c38_attack_events(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    uint16_t group_flags)
{
    int count_index;
    int movement_ticks;

    if (!profile) return;
    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    count_index = (int)((group_flags >> 5) & 0x03u);
    for (; count_index >= 0; --count_index) {
        struct DM1_Event_V1 event;

        /* ReDMCSB GROUP.C F0209 lines 2108-2127 switches to C6 attack
         * and queues C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + creature index
         * for each creature in the group, reusing the group event priority
         * initialized as 255 - MovementTicks.  This bounded CSB bridge only
         * schedules those per-creature attack events; the C38 damage/evasion
         * body remains a later runtime slice. */
        memset(&event, 0, sizeof(event));
        event.map_time = DM1_MAP_TIME_MAKE(map_index, profile->game_time + 1u);
        event.type = (uint8_t)(DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 +
                               count_index);
        event.priority = (uint8_t)(255 - movement_ticks);
        event.b_mapX = (uint8_t)map_x;
        event.b_mapY = (uint8_t)map_y;
        (void)dm1v1_event_add(&profile->timeline_queue, &event);
    }
}

static int csb_v1_runtime_group_destination_is_blocked(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int raw_square;
    int square_type;
    int door_state;

    if (!dungeon || !dungeon->raw_data || level < 0 ||
        level >= dungeon->level_count) {
        return 1;
    }
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0) return 1;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    if (square_type == 0) return 1;
    if (square_type == 4) {
        door_state = raw_square & 0x07;
        return door_state != 0 && door_state != 1 && door_state != 5;
    }
    if (square_type == 6) {
        return !(raw_square & 0x04) && !(raw_square & 0x01);
    }
    return 0;
}

static int csb_v1_runtime_find_group_thing_location(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int *out_level,
    int *out_x,
    int *out_y)
{
    int level;
    int x;
    int y;

    if (out_level) *out_level = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (!dungeon || !dungeon->raw_data) return 0;
    for (level = 0; level < dungeon->level_count; ++level) {
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int thing = csb_v1_dungeon_get_first_thing(
                    dungeon,
                    level,
                    x,
                    y);
                int guard;

                for (guard = 0;
                     guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
                     ++guard) {
                    const uint8_t *record;
                    int thing_type = -1;
                    int thing_size = 0;

                    if ((uint16_t)thing == group_thing) {
                        if (out_level) *out_level = level;
                        if (out_x) *out_x = x;
                        if (out_y) *out_y = y;
                        return 1;
                    }
                    record = csb_v1_dungeon_get_thing_record(
                        dungeon,
                        (uint16_t)thing,
                        &thing_type,
                        NULL,
                        &thing_size);
                    if (!record || thing_size < 2) break;
                    thing = (int)csb_v1_runtime_read_u16(record);
                }
            }
        }
    }
    return 0;
}

static int csb_v1_runtime_square_has_group(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int thing;
    int guard;

    if (!dungeon || !dungeon->raw_data) return 0;
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        int thing_type = -1;
        int thing_size = 0;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 4) return 1;
        thing = (int)csb_v1_runtime_read_u16(record);
    }
    return 0;
}

static int csb_v1_runtime_group_destination_has_party_or_group(
    const CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y)
{
    if (!profile || !profile->dungeon_handle) return 1;
    if (profile->current_level == level &&
        profile->party_x == map_x &&
        profile->party_y == map_y &&
        profile->champion_count > 0) {
        return 1;
    }
    return csb_v1_runtime_square_has_group(
        profile->dungeon_handle,
        level,
        map_x,
        map_y);
}

static void csb_v1_runtime_schedule_move_group_event(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int target_level,
    int target_x,
    int target_y,
    int audible)
{
    struct DM1_Event_V1 event;

    if (!profile) return;
    if (target_level < 0 || target_level > 255 ||
        target_x < 0 || target_x > 255 ||
        target_y < 0 || target_y > 255) {
        return;
    }
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        target_level,
        profile->game_time + 5u);
    event.type = (uint8_t)(audible
        ? DM1_EVENT_MOVE_GROUP_AUDIBLE
        : DM1_EVENT_MOVE_GROUP_SILENT);
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)(group_thing & 0xFFu);
    event.c_effect = (uint8_t)((group_thing >> 8) & 0xFFu);
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static int csb_v1_runtime_move_group_thing_to_square(
    CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int old_level,
    int old_x,
    int old_y,
    int new_level,
    int new_x,
    int new_y)
{
    uint8_t *source_first_ptr;
    uint8_t *dest_first_ptr;
    uint8_t *group_record;
    uint8_t *previous_record;
    uint16_t thing;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon ||
        (old_level == new_level && old_x == new_x && old_y == new_y)) {
        return 0;
    }

    /* ReDMCSB MOVESENS.C F0267 reaches DUNGEON.C F0164/F0163 for the
     * ordinary same-map group relocation.  Keep that mutation in M10's live
     * Thing-chain primitive so the compressed square-first-Thing table and
     * Generic.Next links have one owner.  Cross-map pit/stairs moves still
     * need F0267's map/active-group consequences below. */
    if (csb_dungeon_move_thing_between_levels_default(
            group_thing, old_level, old_x, old_y,
            new_level, new_x, new_y) == 0) {
        return 1;
    }
    /* Cross-map F0267 callers may use legacy decoded save layouts whose
     * square records are not the original byte-map representation.  Those
     * remain below until their loader boundary is retired. */
    source_first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        old_level,
        old_x,
        old_y);
    if (!source_first_ptr) return 0;

    previous_record = NULL;
    thing = csb_v1_runtime_read_u16(source_first_ptr);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(group_record);
        if (thing == group_thing && thing_type == 4 && thing_size >= 16) {
            if (previous_record) {
                csb_v1_runtime_write_u16(previous_record, next_thing);
            } else {
                csb_v1_runtime_write_u16(source_first_ptr, next_thing);
            }
            dest_first_ptr = csb_v1_runtime_square_first_thing_ptr(
                dungeon,
                new_level,
                new_x,
                new_y);
            if (dest_first_ptr) {
                csb_v1_runtime_write_u16(
                    group_record,
                    csb_v1_runtime_read_u16(dest_first_ptr));
                csb_v1_runtime_write_u16(dest_first_ptr, group_thing);
                return 1;
            }
            dest_first_ptr = csb_v1_runtime_create_square_first_thing_ptr(
                dungeon,
                new_level,
                new_x,
                new_y,
                group_thing);
            if (!dest_first_ptr) {
                if (previous_record) {
                    csb_v1_runtime_write_u16(previous_record, group_thing);
                } else {
                    csb_v1_runtime_write_u16(source_first_ptr, group_thing);
                }
                csb_v1_runtime_write_u16(group_record, next_thing);
                return 0;
            }
            /* ReDMCSB: MOVESENS.C F0267 lines 858-867 moves C04 groups by
             * relinking with DUNGEON.C F0163. F0163 lines 1804-1829 creates
             * a square-first entry when the destination square has no thing
             * list. Firestaff keeps the original preallocated-slot contract
             * bounded by refusing insertion when no trailing THING_NONE slot
             * exists. */
            csb_v1_runtime_write_u16(group_record, 0xFFFEu);
            return 1;
        }
        previous_record = group_record;
        thing = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_apply_group_fall_damage(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y);
static void csb_v1_runtime_sync_active_group_state_from_record(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    const uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int preserve_home,
    int moved);

static uint16_t csb_v1_runtime_repeated_group_direction_pack(int direction)
{
    int d = direction & 3;
    return (uint16_t)(d | (d << 2) | (d << 4) | (d << 6));
}

static int csb_v1_runtime_decode_group_teleporter_at_square(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    int raw_square,
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *out_teleporter,
    int *out_scope)
{
    int thing;
    int guard;

    if (out_scope) *out_scope = 0;
    if (!dungeon || !out_teleporter) return -1;
    if (((raw_square >> 5) & 0x07) != 5) return -1;
    if ((raw_square & 0x08) == 0) return 0;

    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    if (thing < 0) return 0;

    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        const uint8_t *record;
        int thing_type = -1;
        int thing_size = 0;
        uint16_t word;
        uint16_t target_word;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 1 && thing_size >= 6) {
            word = csb_v1_runtime_read_u16(record + 2);
            target_word = csb_v1_runtime_read_u16(record + 4);
            memset(out_teleporter, 0, sizeof(*out_teleporter));
            out_teleporter->target_map_x = (int)(word & 0x1Fu);
            out_teleporter->target_map_y = (int)((word >> 5) & 0x1Fu);
            out_teleporter->rotation = (int)((word >> 10) & 0x03u);
            out_teleporter->absolute_rotation = (word & 0x1000u) ? 1 : 0;
            if (out_scope) *out_scope = (int)((word >> 13) & 0x03u);
            out_teleporter->audible = (word & 0x8000u) ? 1 : 0;
            out_teleporter->target_map_index =
                (int)((target_word >> 8) & 0xFFu);
            return 1;
        }
        thing = (int)csb_v1_runtime_read_u16(record);
    }
    return 0;
}

static int csb_v1_runtime_apply_group_consequences_at_square(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int *inout_map_index,
    int *inout_map_x,
    int *inout_map_y,
    int *out_group_alive)
{
    CSB_V1_DungeonData *dungeon;
    int moved_count = 0;
    int chain_guard;

    if (!profile || !profile->dungeon_handle || !inout_map_index ||
        !inout_map_x || !inout_map_y) {
        return 0;
    }
    if (out_group_alive) *out_group_alive = 1;
    dungeon = profile->dungeon_handle;
    for (chain_guard = 0; chain_guard < 100; ++chain_guard) {
        CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
        CSB_V1_TeleporterRotationRuntimeGroupPc34 group;
        CSB_V1_TeleporterRotationRuntimeGroupResultPc34 result;
        uint8_t *group_record;
        const struct CreatureBehaviorProfile_Compat *creature_profile;
        int raw_square;
        int square_type;
        int scope = 0;
        int thing_type = -1;
        int thing_size = 0;
        int creature_type;
        int current_map_index;
        uint16_t flags;
        int direction;

        current_map_index = *inout_map_index;
        if (current_map_index < 0 ||
            current_map_index >= dungeon->level_count) {
            break;
        }
        raw_square = csb_v1_dungeon_get_raw_square(
            dungeon,
            current_map_index,
            *inout_map_x,
            *inout_map_y);
        if (raw_square < 0) {
            break;
        }
        square_type = (raw_square >> 5) & 0x07;
        if (square_type == 2) {
            int target_map_index;
            int target_x = *inout_map_x;
            int target_y = *inout_map_y;

            if ((raw_square & 0x08) == 0 ||
                (raw_square & 0x01) != 0 ||
                !csb_v1_runtime_location_after_level_change(
                    dungeon,
                    current_map_index,
                    1,
                    &target_x,
                    &target_y,
                    &target_map_index)) {
                break;
            }
            if (csb_v1_runtime_group_destination_has_party_or_group(
                    profile,
                    target_map_index,
                    target_x,
                    target_y)) {
                csb_v1_runtime_schedule_move_group_event(
                    profile,
                    group_thing,
                    target_map_index,
                    target_x,
                    target_y,
                    0);
                break;
            }
            if (!csb_v1_runtime_move_group_thing_to_square(
                    dungeon,
                    group_thing,
                    current_map_index,
                    *inout_map_x,
                    *inout_map_y,
                    target_map_index,
                    target_x,
                    target_y)) {
                break;
            }
            *inout_map_index = target_map_index;
            *inout_map_x = target_x;
            *inout_map_y = target_y;
            moved_count++;
            if (csb_v1_runtime_apply_group_fall_damage(
                    profile,
                    group_thing,
                    target_map_index,
                    target_x,
                    target_y) == 2) {
                if (out_group_alive) *out_group_alive = 0;
                break;
            }
            continue;
        }
        if (square_type != 5 || (raw_square & 0x08) == 0) break;
        if (csb_v1_runtime_decode_group_teleporter_at_square(
                dungeon,
                current_map_index,
                *inout_map_x,
                *inout_map_y,
                raw_square,
                &teleporter,
                &scope) <= 0 ||
            (scope & 0x01) == 0) {
            break;
        }
        if (teleporter.target_map_index == current_map_index &&
            teleporter.target_map_x == *inout_map_x &&
            teleporter.target_map_y == *inout_map_y) {
            break;
        }
        if (teleporter.target_map_index < 0 ||
            teleporter.target_map_index >= dungeon->level_count) {
            break;
        }
        if (csb_v1_runtime_group_destination_is_blocked(
                dungeon,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y)) {
            break;
        }
        if (csb_v1_runtime_group_destination_has_party_or_group(
                profile,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y)) {
            csb_v1_runtime_schedule_move_group_event(
                profile,
                group_thing,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y,
                teleporter.audible);
            break;
        }
        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_type != 4 || thing_size < 16) break;

        creature_type = (int)group_record[4];
        creature_profile = CREATURE_GetProfile_Compat(creature_type);
        flags = csb_v1_runtime_read_u16(group_record + 14);
        direction = (int)((flags >> 8) & 0x03u);

        memset(&group, 0, sizeof(group));
        group.count = (int)((flags >> 5) & 0x03u);
        group.creature_size = creature_profile
            ? (int)(creature_profile->attributes & 0x0003)
            : 0;
        group.directions_packed =
            csb_v1_runtime_repeated_group_direction_pack(direction);
        group.cells_packed = (uint16_t)group_record[5];
        group.behavior = (int)(flags & 0x000Fu);
        group.active_group_index = 0;
        group.source_map_index = current_map_index;
        group.party_map_index = profile->current_level;
        if (csb_v1_teleporter_rotation_apply_group_pc34_compat(
                &teleporter,
                &group,
                &result) != 0) {
            break;
        }
        if (!csb_v1_runtime_move_group_thing_to_square(
                dungeon,
                group_thing,
                current_map_index,
                *inout_map_x,
                *inout_map_y,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y)) {
            break;
        }

        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_type != 4 || thing_size < 16) break;
        group_record[5] = (uint8_t)(result.cells_packed & 0xFFu);
        flags = csb_v1_runtime_read_u16(group_record + 14);
        flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 8)) |
                           (uint16_t)((result.directions_packed & 0x03u) << 8));
        csb_v1_runtime_write_u16(group_record + 14, flags);
        *inout_map_index = teleporter.target_map_index;
        *inout_map_x = teleporter.target_map_x;
        *inout_map_y = teleporter.target_map_y;
        moved_count++;
    }
    if (moved_count > 0 && (!out_group_alive || *out_group_alive)) {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_sync_active_group_state_from_record(
                profile,
                group_thing,
                group_record,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y,
                1,
                moved_count > 0);
        }
    }
    /* ReDMCSB MOVESENS.C F0267 lines 493-617 moves C04 groups through
     * creature-scope teleporters and open pits in the same PC34 100-step
     * chain. Teleporters call F0262 for direction/cell rotation; pits call
     * DUNGEON.C F0154 to resolve the lower target map/coordinate. This
     * bounded CSB runtime bridge handles generated groups with raw C04
     * records and mirrors the surviving group's native active-group side
     * state; buzz audio and full F0191 fall-damage aftermath remain separate
     * work. */
    return moved_count;
}

static void csb_v1_runtime_apply_group_behavior_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int thing;
    int guard;
    int distance_x;
    int distance_y;

    if (!profile || !record || !profile->dungeon_handle) return;
    if (profile->champion_count <= 0) return;
    if (record->mapIndex != profile->current_level) return;
    distance_x = abs(profile->party_x - record->mapX);
    distance_y = abs(profile->party_y - record->mapY);

    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB GROUP.C F0209 lines 2098-2139 processes C37.  A wandering
     * group that sees the party switches to C6 attack when in same-row/column
     * attack range, otherwise to C7 approach and queues the next C37.  This
     * CSB bridge applies the real-format group behavior byte mutation first;
     * follow-up movement/attack event expansion remains in the source-locked
     * creature-AI layer. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *thing_record;
        uint16_t flags;
        int thing_type;
        int thing_size;
        int behavior;
        int next_behavior;
        int movement_ticks;
        int creature_count;
        int creature_size;
        uint16_t group_thing;
        const struct CreatureBehaviorProfile_Compat *creature_profile;

        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 4 && thing_size >= 16) {
            flags = csb_v1_runtime_read_u16(thing_record + 14);
            behavior = (int)(flags & 0x000Fu);
            group_thing = (uint16_t)thing;
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            creature_profile = CREATURE_GetProfile_Compat((int)thing_record[4]);
            creature_size = creature_profile
                ? (int)(creature_profile->attributes & 0x0003u)
                : 0;
            if ((behavior == 0 || behavior == 2 || behavior == 3) &&
                (distance_x == 0 || distance_y == 0)) {
                next_behavior = (distance_x + distance_y <= 1) ? 6 : 7;
                flags = (uint16_t)((flags & 0xFFF0u) |
                                   (uint16_t)(next_behavior & 0x0F));
                csb_v1_runtime_write_u16(thing_record + 14, flags);
                csb_v1_runtime_set_active_group_target(
                    profile,
                    group_thing,
                    record->mapIndex,
                    record->mapX,
                    record->mapY,
                    profile->party_x,
                    profile->party_y);
                if (next_behavior == 6) {
                    csb_v1_runtime_turn_active_group_toward_attack(
                        profile,
                        group_thing,
                        thing_record,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        csb_v1_runtime_direction_from_source_to_destination(
                            record->mapX,
                            record->mapY,
                            profile->party_x,
                            profile->party_y),
                        creature_count,
                        creature_size);
                    csb_v1_runtime_schedule_c38_attack_events(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        flags);
                }
                if (next_behavior == 7) {
                    csb_v1_runtime_set_active_group_direction_all(
                        profile,
                        group_thing,
                        thing_record,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        csb_v1_runtime_direction_from_source_to_destination(
                            record->mapX,
                            record->mapY,
                            profile->party_x,
                            profile->party_y));
                    /* ReDMCSB GROUP.C F0209 lines 2135-2140 sets behavior
                     * C7 approach and increments the next C37 time by one
                     * tick; lines 2450-2463 then add C37 through F0208 with
                     * the existing movement-tick priority. */
                    csb_v1_runtime_schedule_c37_group_event(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        1u);
                }
                return;
            }
            if (behavior == 7) {
                CSB_V1_RuntimeActiveGroupState *active_state;
                int party_visible = (distance_x == 0 || distance_y == 0);
                int target_map_index = record->mapIndex;
                int target_x = record->mapX;
                int target_y = record->mapY;
                int approach_target_x = profile->party_x;
                int approach_target_y = profile->party_y;
                int moved = 0;
                int deferred = 0;
                int initial_move_direction = 0;
                int is_archenemy = creature_profile &&
                    ((creature_profile->attributes &
                      CREATURE_ATTR_MASK_ARCHENEMY) != 0);

                movement_ticks = csb_v1_runtime_creature_movement_ticks(
                    (int)thing_record[4]);
                if (party_visible && distance_x + distance_y <= 1) {
                    flags = (uint16_t)((flags & 0xFFF0u) | 6u);
                    csb_v1_runtime_write_u16(thing_record + 14, flags);
                    csb_v1_runtime_set_active_group_target(
                        profile,
                        group_thing,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        profile->party_x,
                        profile->party_y);
                    csb_v1_runtime_turn_active_group_toward_attack(
                        profile,
                        group_thing,
                        thing_record,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        csb_v1_runtime_direction_from_source_to_destination(
                            record->mapX,
                            record->mapY,
                            profile->party_x,
                            profile->party_y),
                        creature_count,
                        creature_size);
                    csb_v1_runtime_schedule_c38_attack_events(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        flags);
                    return;
                }
                if (party_visible) {
                    csb_v1_runtime_set_active_group_target(
                        profile,
                        group_thing,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        profile->party_x,
                        profile->party_y);
                } else {
                    active_state = csb_v1_runtime_active_group_state_for_thing(
                        profile,
                        group_thing);
                    if (!active_state) return;
                    approach_target_x = active_state->target_map_x;
                    approach_target_y = active_state->target_map_y;
                    if (approach_target_x == record->mapX &&
                        approach_target_y == record->mapY) {
                        flags = (uint16_t)(flags & 0xFFF0u);
                        csb_v1_runtime_write_u16(thing_record + 14, flags);
                        csb_v1_runtime_set_active_group_direction_group(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            (int)((flags >> 8) & 0x03u),
                            creature_count,
                            creature_size);
                        return;
                    }
                }
                if (approach_target_y != record->mapY) {
                    target_y += (approach_target_y > record->mapY) ? 1 : -1;
                } else if (approach_target_x != record->mapX) {
                    target_x += (approach_target_x > record->mapX) ? 1 : -1;
                } else {
                    return;
                }
                initial_move_direction =
                    csb_v1_runtime_direction_from_source_to_destination(
                        record->mapX,
                        record->mapY,
                        target_x,
                        target_y);
                if (!csb_v1_runtime_group_destination_is_blocked(
                        dungeon,
                        record->mapIndex,
                        target_x,
                        target_y)) {
                    if (csb_v1_runtime_group_destination_has_party_or_group(
                            profile,
                            record->mapIndex,
                            target_x,
                            target_y)) {
                        csb_v1_runtime_schedule_move_group_event(
                            profile,
                            group_thing,
                            record->mapIndex,
                            target_x,
                            target_y,
                            0);
                        deferred = 1;
                    } else {
                        csb_v1_runtime_sync_active_group_state_from_record(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            0,
                            0);
                        moved = csb_v1_runtime_move_group_thing_to_square(
                            dungeon,
                            group_thing,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            record->mapIndex,
                            target_x,
                            target_y);
                    }
                    if (moved) {
                        int group_alive = 1;
                        int consequence_moves;
                        csb_v1_runtime_request_creature_movement_sound(
                            profile,
                            (int)thing_record[4],
                            target_x,
                            target_y);
                        consequence_moves =
                            csb_v1_runtime_apply_group_consequences_at_square(
                            profile,
                            group_thing,
                            &target_map_index,
                            &target_x,
                            &target_y,
                            &group_alive);
                        if (!group_alive) {
                            return;
                        }
                        thing_record = csb_v1_runtime_mutable_thing_record(
                            dungeon,
                            group_thing,
                            &thing_type,
                            &thing_size);
                        if (thing_record && thing_type == 4 &&
                            thing_size >= 16) {
                            if (consequence_moves == 0) {
                                csb_v1_runtime_set_active_group_direction_group(
                                    profile,
                                    group_thing,
                                    thing_record,
                                    target_map_index,
                                    target_x,
                                    target_y,
                                    initial_move_direction,
                                    creature_count,
                                    creature_size);
                            }
                            csb_v1_runtime_sync_active_group_state_from_record(
                                profile,
                                group_thing,
                                thing_record,
                                target_map_index,
                                target_x,
                                target_y,
                                1,
                                1);
                        }
                    }
                } else {
                    int double_target_x = record->mapX;
                    int double_target_y = record->mapY;
                    if (initial_move_direction == 0) {
                        double_target_y -= 2;
                    } else if (initial_move_direction == 1) {
                        double_target_x += 2;
                    } else if (initial_move_direction == 2) {
                        double_target_y += 2;
                    } else {
                        double_target_x -= 2;
                    }
                    if (is_archenemy &&
                        !csb_v1_runtime_group_destination_is_blocked(
                            dungeon,
                            record->mapIndex,
                            double_target_x,
                            double_target_y) &&
                        !csb_v1_runtime_group_destination_has_party_or_group(
                            profile,
                            record->mapIndex,
                            double_target_x,
                            double_target_y)) {
                        target_x = double_target_x;
                        target_y = double_target_y;
                        csb_v1_runtime_sync_active_group_state_from_record(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            0,
                            0);
                        moved = csb_v1_runtime_move_group_thing_to_square(
                            dungeon,
                            group_thing,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            record->mapIndex,
                            target_x,
                            target_y);
                    }
                    if (moved) {
                        int group_alive = 1;
                        int consequence_moves;
                        csb_v1_runtime_request_buzz_sound(
                            profile,
                            target_x,
                            target_y);
                        csb_v1_runtime_request_creature_movement_sound(
                            profile,
                            (int)thing_record[4],
                            target_x,
                            target_y);
                        consequence_moves =
                            csb_v1_runtime_apply_group_consequences_at_square(
                            profile,
                            group_thing,
                            &target_map_index,
                            &target_x,
                            &target_y,
                            &group_alive);
                        if (!group_alive) {
                            return;
                        }
                        thing_record = csb_v1_runtime_mutable_thing_record(
                            dungeon,
                            group_thing,
                            &thing_type,
                            &thing_size);
                        if (thing_record && thing_type == 4 &&
                            thing_size >= 16) {
                            if (consequence_moves == 0) {
                                csb_v1_runtime_set_active_group_direction_group(
                                    profile,
                                    group_thing,
                                    thing_record,
                                    target_map_index,
                                    target_x,
                                    target_y,
                                    initial_move_direction,
                                    creature_count,
                                    creature_size);
                            }
                            csb_v1_runtime_sync_active_group_state_from_record(
                                profile,
                                group_thing,
                                thing_record,
                                target_map_index,
                                target_x,
                                target_y,
                                1,
                                1);
                        }
                    } else {
                        csb_v1_runtime_sync_active_group_state_from_record(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            0,
                            0);
                        csb_v1_runtime_set_active_group_direction_group(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            initial_move_direction,
                            creature_count,
                            creature_size);
                    }
                }
                /* ReDMCSB GROUP.C F0209 lines 2228-2272 walks C7 approach
                 * toward the target using F0202 movement checks, then lines
                 * 2273-2284 allow an archenemy double-square move that ignores
                 * the first square and requests M560_SOUND_BUZZ at the second
                 * square before moving the group. Lines
                 * 2450-2463 schedule the next C37.  This bounded bridge
                 * relinks a real-format C04 group, creates destination
                 * square-first slots through the bounded F0163 bridge when
                 * needed, schedules C60/C61-style retry when blocked by
                 * party/group occupancy, then applies bounded creature-scope
                 * teleporter/pit chains. Full F0202 occupancy breadth and
                 * attack expansion remain separate work. */
                if (!deferred) {
                    csb_v1_runtime_schedule_c37_group_event(
                        profile,
                        moved ? target_map_index : record->mapIndex,
                        moved ? target_x : record->mapX,
                        moved ? target_y : record->mapY,
                        (int)thing_record[4],
                        (uint32_t)((movement_ticks > 1) ? movement_ticks : 1));
                }
            }
            return;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_move_group_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    uint16_t group_thing;
    int source_level;
    int source_x;
    int source_y;
    int target_level;
    int target_x;
    int target_y;
    int group_alive = 1;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    group_thing = (uint16_t)(((uint16_t)(record->effect & 0xFF) << 8) |
                             (uint16_t)(record->cell & 0xFF));
    target_level = record->mapIndex;
    target_x = record->mapX;
    target_y = record->mapY;
    if (group_thing == 0xFFFEu || group_thing == 0xFFFFu) return;
    if (!csb_v1_runtime_find_group_thing_location(
            dungeon,
            group_thing,
            &source_level,
            &source_x,
            &source_y)) {
        return;
    }
    if (csb_v1_runtime_group_destination_is_blocked(
            dungeon,
            target_level,
            target_x,
            target_y)) {
        return;
    }
    if (csb_v1_runtime_group_destination_has_party_or_group(
            profile,
            target_level,
            target_x,
            target_y)) {
        csb_v1_runtime_schedule_move_group_event(
            profile,
            group_thing,
            target_level,
            target_x,
            target_y,
            record->eventType == DM1_EVENT_MOVE_GROUP_AUDIBLE);
        return;
    }
    {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_sync_active_group_state_from_record(
                profile,
                group_thing,
                group_record,
                source_level,
                source_x,
                source_y,
                0,
                0);
        }
    }
    if (!csb_v1_runtime_move_group_thing_to_square(
            dungeon,
            group_thing,
            source_level,
            source_x,
            source_y,
            target_level,
            target_x,
            target_y)) {
        return;
    }
    {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_request_creature_movement_sound(
                profile,
                (int)group_record[4],
                target_x,
                target_y);
        }
    }
    /* ReDMCSB: MOVESENS.C F0265 lines 169-189 schedules C60/C61 with
     * destination map/x/y and group thing in C.Slot after a blocked group
     * move. TIMELINE.C later retries F0267 on that group. This bounded CSB
     * bridge carries C.Slot in c_cell/c_effect and retries movement when
     * the party/group obstruction is gone. */
    (void)csb_v1_runtime_apply_group_consequences_at_square(
        profile,
        group_thing,
        &target_level,
        &target_x,
        &target_y,
        &group_alive);
    if (group_alive) {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_sync_active_group_state_from_record(
                profile,
                group_thing,
                group_record,
                target_level,
                target_x,
                target_y,
                1,
                1);
        }
    }
}

static int csb_v1_runtime_stat_or_default(
    const CSB_V1_Champion *champion,
    int stat_index,
    int stat_kind)
{
    int value;

    if (!champion ||
        stat_index < 0 ||
        stat_index >= CSB_V1_STAT_COUNT ||
        stat_kind < 0 ||
        stat_kind > CSB_V1_STAT_MAX) {
        return 30;
    }
    value = (int)champion->Statistics[stat_index][stat_kind];
    return (value > 0) ? value : 30;
}

static int csb_v1_runtime_imported_skill_level(
    const CSB_V1_Champion *champion,
    int skill_index)
{
    int level;

    if (!champion ||
        skill_index < 0 ||
        skill_index >= CSB_V1_FULL_SKILL_COUNT) {
        return 1;
    }

    if (champion->SkillExperienceValid) {
        int64_t experience;

        /* CSBWin SaveGame.cpp:1838 swaps four CHARDESC records whose SKILL
         * rows are CSB.h CHARDESC::skill[20] tempAdjust/experience pairs.
         * ReDMCSB CHAMPION.C F0303 lines 752-768 adds temporary experience,
         * averages hidden skills with their base skill, then halves from the
         * 500 XP threshold to derive the live skill level. */
        experience = (int64_t)champion->SkillExperience[skill_index] +
                     (int64_t)champion->SkillTemporaryExperience[skill_index];
        if (skill_index > 3) {
            const int base_skill = (skill_index - 4) >> 2;
            experience +=
                (int64_t)champion->SkillExperience[base_skill] +
                (int64_t)champion->SkillTemporaryExperience[base_skill];
            experience >>= 1;
        }
        if (experience < 0) experience = 0;
        level = 1;
        while (experience >= 500 && level < 16) {
            experience >>= 1;
            level++;
        }
        return level;
    }

    if (skill_index >= CSB_V1_SKILL_COUNT) {
        return 1;
    }
    level = (int)champion->Skills[skill_index];
    if (level < 1) level = 1;
    if (level > 16) level = 16;
    return level;
}

/* CSBWin Code17818.cpp::DetermineMastery called by Magic.cpp::AddToSkill
 * supplies 0xc000, so possession and temporary-adjustment state are both
 * excluded.  This narrow helper deliberately retains only that source path. */
static int csb_v1_runtime_csbwin_unadjusted_mastery(uint32_t experience)
{
    int mastery = 1;

    while (experience >= 500u) {
        experience >>= 1;
        ++mastery;
    }
    return mastery;
}

/* Magic.cpp::AddToSkill updates the selected SKILL, then its basic skill for
 * subskills, with a UI16 increment and LimitSkillExperience cap.  LevelUp
 * has random/stat/UI effects outside this accepted runtime transaction, so
 * callers must reject a mastery transition rather than publishing half of it. */
static int csb_v1_runtime_csbwin_prepare_add_to_skill(
    const CSB_V1_RuntimeProfile *profile, int32_t character_selector,
    int32_t skill_number, int32_t experience, uint32_t *out_skill_experience,
    uint32_t *out_basic_experience, int *out_basic_skill)
{
    const CSB_V1_Champion *champion;
    uint32_t increment;
    uint32_t skill_experience;
    uint32_t basic_experience;
    uint64_t accumulated;
    int basic_skill;

    if (!profile || !out_skill_experience || !out_basic_experience ||
        !out_basic_skill || !profile->party_state_valid ||
        character_selector < 0 ||
        character_selector >= profile->party_state.ChampionCount ||
        character_selector >= CSB_V1_MAX_CHAMPIONS ||
        skill_number < 0 || skill_number >= CSB_V1_FULL_SKILL_COUNT ||
        experience <= 0) {
        return -1;
    }
    champion = &profile->party_state.Champions[character_selector];
    if (champion->CurrentHealth <= 0) return 0;
    if (!champion->SkillExperienceValid) return -1;

    basic_skill = skill_number < 4 ? skill_number : (skill_number - 4) / 4;
    increment = (uint16_t)experience;
    accumulated = (uint64_t)champion->SkillExperience[skill_number] + increment;
    skill_experience = accumulated > 0x10000000u ? 0x10000000u :
        (uint32_t)accumulated;
    if (skill_number == basic_skill) {
        basic_experience = skill_experience;
    } else {
        accumulated = (uint64_t)champion->SkillExperience[basic_skill] +
            increment;
        basic_experience = accumulated > 0x10000000u ? 0x10000000u :
            (uint32_t)accumulated;
    }

    *out_skill_experience = skill_experience;
    *out_basic_experience = basic_experience;
    *out_basic_skill = basic_skill;
    if (csb_v1_runtime_csbwin_unadjusted_mastery(
            champion->SkillExperience[basic_skill]) !=
        csb_v1_runtime_csbwin_unadjusted_mastery(basic_experience)) {
        return -2;
    }
    return 1;
}

int csb_v1_runtime_csbwin_dsa_levelup_prerequisite_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int32_t character_selector,
    int32_t skill_number, int32_t experience,
    CSB_V1_CSBWinDSALevelUpPrerequisiteReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinDSALevelUpPrerequisiteReceipt_PC34 receipt;
    uint32_t selected_after = 0u;
    uint32_t basic_after = 0u;
    int basic_skill = -1;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out_receipt = receipt;
    if (!profile || !profile->party_state_valid || character_selector < 0 ||
        character_selector >= profile->party_state.ChampionCount ||
        character_selector >= CSB_V1_MAX_CHAMPIONS || skill_number < 0 ||
        skill_number >= CSB_V1_FULL_SKILL_COUNT ||
        csb_v1_runtime_csbwin_prepare_add_to_skill(
            profile, character_selector, skill_number, experience,
            &selected_after, &basic_after, &basic_skill) != -2) {
        return 0;
    }
    receipt.valid = 1;
    receipt.levelup_required = 1;
    receipt.character_selector = character_selector;
    receipt.selected_skill_number = skill_number;
    receipt.basic_skill_number = basic_skill;
    receipt.increment_ui16 = (uint16_t)experience;
    receipt.selected_before = profile->party_state.Champions[character_selector]
        .SkillExperience[skill_number];
    receipt.selected_after = selected_after;
    receipt.basic_before = profile->party_state.Champions[character_selector]
        .SkillExperience[basic_skill];
    receipt.basic_after = basic_after;
    receipt.basic_mastery_before = csb_v1_runtime_csbwin_unadjusted_mastery(
        receipt.basic_before);
    receipt.basic_mastery_after = csb_v1_runtime_csbwin_unadjusted_mastery(
        receipt.basic_after);
    *out_receipt = receipt;
    return 1;
}

int csb_v1_runtime_csbwin_dsa_levelup_prerequisite_current_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWinDSALevelUpPrerequisiteReceipt_PC34 *receipt)
{
    CSB_V1_CSBWinDSALevelUpPrerequisiteReceipt_PC34 current;

    if (!profile || !receipt || !receipt->valid || !receipt->levelup_required ||
        !csb_v1_runtime_csbwin_dsa_levelup_prerequisite_receipt_pc34(
            profile, receipt->character_selector,
            receipt->selected_skill_number, receipt->increment_ui16,
            &current)) {
        return 0;
    }
    return current.basic_skill_number == receipt->basic_skill_number &&
        current.selected_before == receipt->selected_before &&
        current.selected_after == receipt->selected_after &&
        current.basic_before == receipt->basic_before &&
        current.basic_after == receipt->basic_after &&
        current.basic_mastery_before == receipt->basic_mastery_before &&
        current.basic_mastery_after == receipt->basic_mastery_after;
}

static int csb_v1_runtime_dsa_prepare_experience_plus(
    void *user, int32_t character_selector, int32_t skill_number,
    int32_t experience)
{
    uint32_t skill_experience;
    uint32_t basic_experience;
    int basic_skill;

    return csb_v1_runtime_csbwin_prepare_add_to_skill(
        (const CSB_V1_RuntimeProfile *)user, character_selector, skill_number,
        experience, &skill_experience, &basic_experience, &basic_skill);
}

static int csb_v1_runtime_dsa_add_experience_plus(
    void *user, int32_t character_selector, int32_t skill_number,
    int32_t experience)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    uint32_t skill_experience;
    uint32_t basic_experience;
    int basic_skill;
    int prepared;

    prepared = csb_v1_runtime_csbwin_prepare_add_to_skill(
        profile, character_selector, skill_number, experience,
        &skill_experience, &basic_experience, &basic_skill);
    if (prepared <= 0) return prepared;
    profile->party_state.Champions[character_selector]
        .SkillExperience[skill_number] = skill_experience;
    profile->party_state.Champions[character_selector]
        .SkillExperience[basic_skill] = basic_experience;
    return 1;
}

/* CSBWin DSA.cpp:3389-3409 passes the original flags through to
 * Code17818.cpp::DetermineMastery.  The loaded CHARDESC rows own experience
 * and temporary adjustments.  The runtime has no verified CSBWin name-index
 * owner for Firestaff/Pendant/Gem possession bonuses, therefore only the
 * source caller flag which suppresses possessions is accepted here. */
static int csb_v1_runtime_dsa_get_mastery(
    void *user, uint32_t champion_index, uint32_t skill_index,
    uint32_t flags, uint32_t *out_mastery)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_Champion *champion;
    int64_t experience;
    int leader;
    int mastery;

    if (!profile || !out_mastery || !profile->party_state_valid) return -1;
    if (champion_index == 4u) {
        leader = profile->leader_index;
        if (leader < 0 || leader >= profile->party_state.ChampionCount) {
            leader = profile->party_state.LeaderIndex;
        }
        if (leader < 0 || leader >= profile->party_state.ChampionCount) {
            return 0;
        }
        champion_index = (uint32_t)leader;
    }
    if (champion_index >= (uint32_t)profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        skill_index >= CSB_V1_FULL_SKILL_COUNT) {
        return 0;
    }
    if ((flags & 1u) == 0u) return -1;
    if (profile->csbwin_party_sleeping) {
        *out_mastery = 1u;
        return 1;
    }

    champion = &profile->party_state.Champions[champion_index];
    if (!champion->SkillExperienceValid) return -1;
    experience = (int64_t)champion->SkillExperience[skill_index];
    if ((flags & 2u) == 0u) {
        experience += champion->SkillTemporaryExperience[skill_index];
    }
    if (skill_index > 3u) {
        const uint32_t basic_skill = (skill_index - 4u) >> 2;

        experience += champion->SkillExperience[basic_skill];
        if ((flags & 2u) == 0u) {
            experience += champion->SkillTemporaryExperience[basic_skill];
        }
        experience >>= 1;
    }
    if (experience < 0) experience = 0;
    mastery = 1;
    while (experience >= 500) {
        experience >>= 1;
        ++mastery;
    }
    *out_mastery = (uint32_t)mastery;
    return 1;
}

/* DSA.cpp:4127-4165 copies this exact GAMEBLOCK2/character-tail image into
 * DSAVARS.  GAMEBLOCK2 owns pose, count, and HandChar; the verified body
 * owns spell-effect tail values.  Do not assemble a partial party image. */
static int csb_v1_runtime_dsa_get_party_info(
    void *user, uint32_t out_values[12])
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    int champion_count;
    int hand_character;

    if (!profile || !out_values || !profile->party_state_valid ||
        !profile->csbwin_gameblock2_summary_valid ||
        !profile->csbwin_body_runtime_summary_valid) {
        return -1;
    }
    champion_count = profile->party_state.ChampionCount;
    if (champion_count < 0 || champion_count > CSB_V1_MAX_CHAMPIONS ||
        profile->champion_count != champion_count) {
        return -1;
    }
    hand_character = profile->leader_index;
    if (hand_character < 0 || hand_character >= champion_count) {
        hand_character = profile->party_state.LeaderIndex;
    }
    if (champion_count != 0 &&
        (hand_character < 0 || hand_character >= champion_count)) {
        return -1;
    }

    out_values[0] = (uint32_t)champion_count;
    out_values[1] = (uint32_t)profile->current_level;
    out_values[2] = (uint32_t)profile->party_x;
    out_values[3] = (uint32_t)profile->party_y;
    out_values[4] = (uint32_t)(profile->party_dir & 3);
    out_values[5] = profile->csbwin_party_sleeping ? 1u : 0u;
    out_values[6] = profile->csbwin_character_tail_see_thru_walls;
    out_values[7] = profile->csbwin_character_tail_magic_footprints_active;
    out_values[8] = champion_count == 0 ? 0u : (uint32_t)hand_character;
    out_values[9] = profile->csbwin_character_tail_invisible;
    out_values[10] = (uint32_t)(int32_t)
        profile->csbwin_character_tail_fire_shield;
    out_values[11] = (uint32_t)(int32_t)
        profile->csbwin_character_tail_spell_shield;
    return 1;
}

int csb_v1_runtime_get_champion_skill_level(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int skill_index)
{
    if (!profile ||
        !profile->party_state_valid ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    return csb_v1_runtime_imported_skill_level(
        &profile->party_state.Champions[champion_index],
        skill_index);
}

static int csb_v1_runtime_fill_creature_combat_snapshot(
    int creature_type,
    int creature_index,
    struct CombatantCreatureSnapshot_Compat *out)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    if (!creature_profile) return 0;

    /* ReDMCSB DUNGEON.C G0243 supplies immutable creature attack fields;
     * live C04 real-format group records supply the per-creature slot. */
    out->creatureType = creature_type;
    out->attack = creature_profile->baseAttack;
    out->defense = creature_profile->baseDefense;
    out->dexterity = creature_profile->dexterity;
    out->baseHealth = creature_profile->baseHealth;
    out->poisonAttack = creature_profile->poisonAttack;
    out->attackType = creature_profile->attackType;
    out->attributes = creature_profile->attributes;
    out->woundProbabilities = creature_profile->woundProbabilities;
    out->properties = creature_profile->properties;
    out->doubledMapDifficulty = 0;
    out->creatureIndex = creature_index;
    out->healthBefore = 0;
    return 1;
}

static int csb_v1_runtime_fill_defender_combat_snapshot(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    struct CombatantChampionSnapshot_Compat *out)
{
    const CSB_V1_Champion *champion;

    if (!profile || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (champion->CurrentHealth <= 0 ||
        (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0321 consumes a snapshot of current champion
     * statistics, wounds, defenses, and party shields.  CSB V1's imported
     * champion block currently carries the source statistics and compact
     * skill row but not yet the full DM1 armor/wound/rest/shield side state,
     * so those fields stay at bounded zero until the shared inventory/
     * lifecycle bridge is attached. */
    out->championIndex = champion_index;
    out->currentHealth = champion->CurrentHealth;
    out->dexterity = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_DEX,
        CSB_V1_STAT_CUR);
    out->skillLevelParry = csb_v1_runtime_imported_skill_level(champion, 7);
    out->statisticVitality = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_VIT,
        CSB_V1_STAT_CUR);
    out->statisticAntifire = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_ANTIFIRE,
        CSB_V1_STAT_CUR);
    out->statisticAntimagic = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_ANTIMAGIC,
        CSB_V1_STAT_CUR);
    out->statisticWisdom = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_WIS,
        CSB_V1_STAT_CUR);
    out->statisticLuck = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_LUCK,
        CSB_V1_STAT_CUR);
    out->statisticLuckMax = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_LUCK,
        CSB_V1_STAT_MAX);
    out->statisticLuckMin = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_LUCK,
        CSB_V1_STAT_MIN);
    out->wounds = (int)champion->Wounds;
    return 1;
}

static uint32_t csb_v1_runtime_creature_attack_seed(
    const CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int creature_type,
    int creature_index,
    int champion_index)
{
    uint32_t seed;

    seed = 0xC5B1C038u ^
           ((uint32_t)profile->game_time * 1103515245u) ^
           ((uint32_t)(record->mapX & 0xFF) << 24) ^
           ((uint32_t)(record->mapY & 0xFF) << 16) ^
           ((uint32_t)(creature_type & 0xFF) << 8) ^
           (uint32_t)((creature_index & 0x03) |
                      ((champion_index & 0x03) << 2));
    return (seed != 0u) ? seed : 1u;
}

static void csb_v1_runtime_mark_champion_dead(
    CSB_V1_RuntimeProfile *profile,
    int champion_index)
{
    int next_leader;

    if (!profile ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return;
    }
    /* CSBWin Character.cpp::KillCharacter:2532-2585 invokes the optional
     * CharDeathFilter before it changes the hero.  Only the existing
     * checksum/FNV-authenticated pure-stack DSA runner is admitted here. */
    (void)csb_v1_runtime_execute_csbwin_character_death_filter(
        profile, champion_index);
    (void)csb_v1_champion_kill(
        &profile->party_state.Champions[champion_index]);
    if (profile->party_state.LeaderIndex == champion_index ||
        profile->leader_index == champion_index) {
        next_leader = csb_v1_runtime_first_living_champion(
            &profile->party_state);
        profile->party_state.LeaderIndex = next_leader;
        profile->leader_index = next_leader;
        if (next_leader < 0) {
            /* ReDMCSB CHAMPION.C F0319 lines 1662-1668 sets
             * G0303_B_PartyDead when no champion still has CurrentHealth
             * after damage application. */
            profile->game_over = 1;
        }
    }
}

static void csb_v1_runtime_schedule_poison_champion_event(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int poison_attack)
{
    struct DM1_Event_V1 event;
    int event_index;

    if (!profile || champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        poison_attack <= 0) {
        return;
    }
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        profile->current_level,
        profile->game_time + 36u);
    event.type = DM1_EVENT_POISON_CHAMPION;
    event.priority = (uint8_t)champion_index;
    event.b_mapX = (uint8_t)profile->party_x;
    event.b_mapY = (uint8_t)profile->party_y;
    event.c_effect = (uint8_t)(poison_attack & 0xff);
    event_index = csb_v1_runtime_add_timeline_event(profile, &event);
    if (event_index >= 0 && event_index < DM1_EVENT_MAX_COUNT) {
        profile->csbwin_poison_event_attack[event_index] =
            (uint16_t)poison_attack;
        profile->csbwin_poison_event_attack_valid[event_index] = 1u;
    }
    if (event_index >= 0 &&
        profile->party_state.Champions[champion_index].PoisonEventCount <
            255u) {
        profile->party_state.Champions[champion_index].PoisonEventCount++;
    }
}

static void csb_v1_runtime_apply_poison_attack_to_champion(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int poison_attack)
{
    CSB_V1_Champion *champion;
    int poison_damage;
    int next_attack;
    unsigned int dose;

    if (!profile ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        poison_attack <= 0) {
        return;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (champion->CurrentHealth <= 0 ||
        (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
        return;
    }

    /* ReDMCSB CHAMPION.C F0322 lines 1949-1960 applies immediate
     * max(1, Attack >> 6) damage, increments PoisonEventCount when
     * rescheduling C75, and stores Attack-1 in EVENT.B.Attack. The shared
     * event retains the low byte for ordinary V1 dispatch while the runtime
     * slot receipt preserves the full CSBWin timerWord6 continuation. */
    poison_damage = poison_attack >> 6;
    if (poison_damage < 1) poison_damage = 1;
    if (poison_damage >= champion->CurrentHealth) {
        champion->CurrentHealth = 0;
        csb_v1_runtime_mark_champion_dead(profile, champion_index);
        return;
    }
    champion->CurrentHealth = (int16_t)(champion->CurrentHealth -
                                        poison_damage);

    dose = (unsigned int)champion->PoisonDose +
           (unsigned int)poison_attack;
    if (dose > 0xffffu) dose = 0xffffu;
    champion->PoisonDose = (uint16_t)dose;

    next_attack = poison_attack - 1;
    if (next_attack > 0) {
        csb_v1_runtime_schedule_poison_champion_event(
            profile,
            champion_index,
            next_attack);
    }
}

/* CSBWin DSA.cpp STKOP_CausePoison delegates its two source words to
 * CSBCode.cpp::PoisonCharacter.  This callback admits only the loaded party
 * owner; the caller executes it against a profile candidate and publishes the
 * party and C75 queue together after the complete DSA action succeeds. */
static int csb_v1_runtime_dsa_prepare_cause_poison(
    void *user, int32_t character_selector, int32_t poison_value)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_Champion *champion;

    if (!profile || !profile->party_state_valid ||
        profile->party_state.ChampionCount < 0 ||
        profile->party_state.ChampionCount > CSB_V1_MAX_CHAMPIONS ||
        poison_value <= 0 || poison_value > 0xffff) {
        return -1;
    }
    if (character_selector < 0 ||
        character_selector >= profile->party_state.ChampionCount) {
        /* CSBWin's selected-character route has no target in this party. */
        return 0;
    }
    champion = &profile->party_state.Champions[character_selector];
    if (champion->CurrentHealth <= 0 ||
        (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
        return 0;
    }
    if (poison_value > 1 && champion->PoisonEventCount == 255u) {
        return -1;
    }
    return 1;
}

static int csb_v1_runtime_dsa_commit_cause_poison(
    void *user, int32_t character_selector, int32_t poison_value)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_Champion *champion;
    uint8_t event_count_before;

    if (csb_v1_runtime_dsa_prepare_cause_poison(
            user, character_selector, poison_value) != 1) {
        return 0;
    }
    champion = &profile->party_state.Champions[character_selector];
    event_count_before = champion->PoisonEventCount;
    csb_v1_runtime_apply_poison_attack_to_champion(
        profile, character_selector, poison_value);

    /* A live survivor with a positive continuation must own one concrete
     * source C75.  A full queue cannot silently turn this into a partial
     * damage-only publish because the enclosing profile candidate is dropped. */
    if (champion->CurrentHealth > 0 && poison_value > 1 &&
        champion->PoisonEventCount != (uint8_t)(event_count_before + 1u)) {
        return 0;
    }
    return 1;
}

static int csb_v1_runtime_find_cause_poison_event(
    const CSB_V1_RuntimeProfile *profile, int champion_index,
    uint16_t continuation_attack, uint16_t *out_event_index,
    uint32_t *out_event_time)
{
    int ordinal;

    if (!profile || !out_event_index || !out_event_time ||
        champion_index < 0 || champion_index >= CSB_V1_MAX_CHAMPIONS ||
        continuation_attack == 0u ||
        profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount > DM1_EVENT_MAX_COUNT) {
        return 0;
    }
    for (ordinal = 0; ordinal < profile->timeline_queue.eventCount; ++ordinal) {
        int event_index = profile->timeline_queue.timeline[ordinal];
        const struct DM1_Event_V1 *event;

        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) return 0;
        event = &profile->timeline_queue.events[event_index];
        if (event->type == DM1_EVENT_POISON_CHAMPION &&
            event->priority == (uint8_t)champion_index &&
            profile->csbwin_poison_event_attack_valid[event_index] &&
            profile->csbwin_poison_event_attack[event_index] ==
                continuation_attack) {
            *out_event_index = (uint16_t)event_index;
            *out_event_time = event->map_time;
            return 1;
        }
    }
    return 0;
}

static void csb_v1_runtime_apply_poison_event_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    uint16_t event_index)
{
    int champion_index;
    int poison_attack;

    if (!profile || !record) return;
    champion_index = record->aux0;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return;
    }
    /* ReDMCSB TIMELINE.C C75 lines 1991-1993 decrements the current
     * poison event count before F0322 reschedules Attack-1. */
    if (profile->party_state.Champions[champion_index].PoisonEventCount >
        0u) {
        profile->party_state.Champions[champion_index].PoisonEventCount--;
    }
    poison_attack = record->effect;
    if (event_index < DM1_EVENT_MAX_COUNT &&
        profile->csbwin_poison_event_attack_valid[event_index]) {
        poison_attack = profile->csbwin_poison_event_attack[event_index];
        profile->csbwin_poison_event_attack_valid[event_index] = 0u;
        profile->csbwin_poison_event_attack[event_index] = 0u;
    }
    csb_v1_runtime_apply_poison_attack_to_champion(
        profile, champion_index, poison_attack);
}

static int csb_v1_runtime_apply_explosion_party_action(
    CSB_V1_RuntimeProfile *profile,
    const struct CombatAction_Compat *action,
    struct RngState_Compat *rng)
{
    int applied;
    int random_window;
    int base_attack;
    int i;

    if (!profile || !action || !rng ||
        !profile->party_state_valid ||
        action->kind != COMBAT_ACTION_APPLY_DAMAGE_CHAMPION ||
        action->rawAttackValue <= 0) {
        return 0;
    }

    applied = 0;
    random_window = (action->rawAttackValue >> 3) + 1;
    base_attack = action->rawAttackValue - random_window;
    random_window <<= 1;

    /* ReDMCSB PROJEXPL.C F0213 lines 169-174 and F0220 lines
     * 858-862 dispatch party-square fireball/lightning/poison-cloud
     * explosions through CHAMPION.C F0324.  F0324 fans out to every
     * living champion, randomizes attack by +/- 1/8, and then calls F0321
     * for shield/defense/wound scaling before F0319 death handling. */
    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; ++i) {
        CSB_V1_Champion *champion = &profile->party_state.Champions[i];
        struct CombatantChampionSnapshot_Compat defender;
        int randomized_attack;
        int scaled_attack = 0;
        int selected_wounds = 0;

        if (champion->CurrentHealth <= 0 ||
            (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
            continue;
        }
        randomized_attack = base_attack +
            F0732_COMBAT_RngRandom_Compat(rng, random_window);
        if (randomized_attack < 1) randomized_attack = 1;
        if (!csb_v1_runtime_fill_defender_combat_snapshot(
                profile,
                i,
                &defender) ||
            !F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
                action->attackTypeCode,
                randomized_attack,
                action->allowedWounds,
                &defender,
                rng,
                &scaled_attack,
                NULL) ||
            scaled_attack <= 0) {
            continue;
        }
        if (!F0739c_COMBAT_SelectChampionWoundsF0321Rng_Compat(
                scaled_attack,
                action->allowedWounds,
                &defender,
                rng,
                &selected_wounds,
                NULL)) {
            selected_wounds = 0;
        }
        champion->Wounds = (uint16_t)(champion->Wounds |
                                      (uint16_t)selected_wounds);
        if (scaled_attack >= champion->CurrentHealth) {
            champion->CurrentHealth = 0;
            csb_v1_runtime_mark_champion_dead(profile, i);
        } else {
            champion->CurrentHealth =
                (int16_t)(champion->CurrentHealth - scaled_attack);
        }
        applied++;
    }
    return applied;
}

static int csb_v1_runtime_group_cell_value(int cells, int index)
{
    if (cells == 0xFF) return 0xFF;
    if (index < 0 || index > 3) return 0;
    return (cells >> (index * 2)) & 0x03;
}

static int csb_v1_runtime_group_cells_set_value(
    int cells,
    int index,
    int value)
{
    int shift;

    if (cells == 0xFF) return cells;
    if (index < 0 || index > 3) return cells;
    shift = index * 2;
    cells &= ~(0x03 << shift);
    cells |= (value & 0x03) << shift;
    return cells & 0xFF;
}

static int csb_v1_runtime_group_direction_value(uint16_t directions, int index)
{
    if (index < 0 || index > 3) return 0;
    return (int)((directions >> (index * 2)) & 0x03u);
}

static uint16_t csb_v1_runtime_group_directions_set_value(
    uint16_t directions,
    int index,
    int value)
{
    int shift;

    if (index < 0 || index > 3) return directions;
    shift = index * 2;
    directions = (uint16_t)(directions & ~(uint16_t)(0x03u << shift));
    directions =
        (uint16_t)(directions | (uint16_t)((value & 0x03) << shift));
    return directions;
}

static void csb_v1_runtime_unlink_group_thing_from_square(
    CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y)
{
    uint8_t *first_ptr;
    uint8_t *record;
    uint8_t *previous_record;
    uint16_t thing;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon) return;
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);
    if (!first_ptr) return;

    previous_record = NULL;
    thing = csb_v1_runtime_read_u16(first_ptr);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return;
        next_thing = csb_v1_runtime_read_u16(record);
        if (thing == group_thing && thing_type == 4) {
            if (previous_record) {
                csb_v1_runtime_write_u16(previous_record, next_thing);
            } else {
                csb_v1_runtime_write_u16(first_ptr, next_thing);
            }
            return;
        }
        previous_record = record;
        thing = next_thing;
    }
}

static int csb_v1_runtime_append_thing_to_square_tail(
    CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int level,
    int map_x,
    int map_y)
{
    uint8_t *first_ptr;
    uint8_t *record;
    uint16_t current;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon || thing == 0xFFFEu || thing == 0xFFFFu) return 0;
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);

    record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        thing,
        &thing_type,
        &thing_size);
    if (!record || thing_size < 2) return 0;
    csb_v1_runtime_write_u16(record, 0xFFFEu);
    if (!first_ptr) {
        first_ptr = csb_v1_runtime_create_square_first_thing_ptr(
            dungeon,
            level,
            map_x,
            map_y,
            thing);
        return first_ptr ? 1 : 0;
    }

    current = csb_v1_runtime_read_u16(first_ptr);
    if (current == 0xFFFEu || current == 0xFFFFu) {
        csb_v1_runtime_write_u16(first_ptr, thing);
        return 1;
    }
    for (guard = 0; guard < 128; ++guard) {
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            current,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(record);
        if (next_thing == 0xFFFEu || next_thing == 0xFFFFu) {
            csb_v1_runtime_write_u16(record, thing);
            return 1;
        }
        current = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_unlink_thing_from_square(
    CSB_V1_DungeonData *dungeon,
    uint16_t target_thing,
    int level,
    int map_x,
    int map_y)
{
    uint8_t *first_ptr;
    uint8_t *record;
    uint8_t *previous_record;
    uint16_t thing;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon || target_thing == 0xFFFEu || target_thing == 0xFFFFu) {
        return 0;
    }
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);
    if (!first_ptr) return 0;

    previous_record = NULL;
    thing = csb_v1_runtime_read_u16(first_ptr);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(record);
        if ((thing & 0x3FFFu) == (target_thing & 0x3FFFu)) {
            if (previous_record) {
                csb_v1_runtime_write_u16(previous_record, next_thing);
            } else {
                csb_v1_runtime_write_u16(first_ptr, next_thing);
            }
            csb_v1_runtime_write_u16(record, 0xFFFEu);
            return 1;
        }
        previous_record = record;
        thing = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_projectile_result_places_associated_object(
    int result_kind)
{
    switch (result_kind) {
    case PROJECTILE_RESULT_DESPAWN_ENERGY:
    case PROJECTILE_RESULT_HIT_CHAMPION:
    case PROJECTILE_RESULT_HIT_CREATURE:
    case PROJECTILE_RESULT_HIT_DOOR:
    case PROJECTILE_RESULT_HIT_WALL:
    case PROJECTILE_RESULT_HIT_OTHER_PROJECTILE:
        return 1;
    default:
        return 0;
    }
}

static int csb_v1_runtime_stairs_exit_direction(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int direction = csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
        dungeon, level, map_x, map_y);
    return direction;
}

static int csb_v1_runtime_location_after_level_change(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int level_delta,
    int *inout_map_x,
    int *inout_map_y,
    int *out_map_index)
{
    int target_map;

    if (out_map_index) *out_map_index = -1;
    if (!out_map_index) return 0;
    target_map = csb_v1_dungeon_f0154_get_location_after_level_change_pc34(
        dungeon, map_index, level_delta, inout_map_x, inout_map_y);
    if (target_map < 0) return 0;
    *out_map_index = target_map;
    return 1;
}

static int csb_v1_runtime_apply_object_consequences_at_square(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t *inout_thing,
    int source_map_x,
    int *inout_map_index,
    int *inout_map_x,
    int *inout_map_y)
{
    int moved_count = 0;
    int chain_guard;

    if (!dungeon || !inout_thing || !inout_map_index ||
        !inout_map_x || !inout_map_y) {
        return 0;
    }
    for (chain_guard = 0; chain_guard < 100; ++chain_guard) {
        CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
        CSB_V1_TeleporterRotationRuntimeObjectResultPc34 result;
        int raw_square;
        int square_type;
        int scope = 0;
        int self_target;

        if (*inout_map_index < 0 ||
            *inout_map_index >= dungeon->level_count) {
            break;
        }
        raw_square = csb_v1_dungeon_get_raw_square(
            dungeon,
            *inout_map_index,
            *inout_map_x,
            *inout_map_y);
        if (raw_square < 0) {
            break;
        }
        square_type = (dungeon->square_bytes == 1)
            ? ((raw_square >> 5) & 0x07)
            : (raw_square & 0x1F);
        if (square_type != PROJECTILE_ELEMENT_TELEPORTER) {
            int target_level;
            int old_level;
            int old_x;
            int old_y;
            uint16_t old_thing;

            old_level = *inout_map_index;
            old_x = *inout_map_x;
            old_y = *inout_map_y;
            old_thing = *inout_thing;
            if (square_type == 2) {
                int target_x = *inout_map_x;
                int target_y = *inout_map_y;
                if ((raw_square & 0x08) == 0 ||
                    (raw_square & 0x01) != 0) {
                    break;
                }
                if (!csb_v1_runtime_location_after_level_change(
                        dungeon,
                        *inout_map_index,
                        1,
                        &target_x,
                        &target_y,
                        &target_level)) {
                    break;
                }
                csb_v1_runtime_process_object_floor_sensors_at(
                    profile, dungeon, *inout_thing, old_level, old_x, old_y,
                    0);
                if (!csb_v1_runtime_unlink_thing_from_square(
                        dungeon,
                        *inout_thing,
                        old_level,
                        old_x,
                        old_y)) {
                    break;
                }
                *inout_map_index = target_level;
                *inout_map_x = target_x;
                *inout_map_y = target_y;
            } else if (square_type == 3) {
                static const int step_east[4] = { 0, 1, 0, -1 };
                static const int step_north[4] = { -1, 0, 1, 0 };
                uint16_t moved_thing;
                int direction;
                int cell;

                if ((raw_square & 0x04) == 0) {
                    int target_x = *inout_map_x;
                    int target_y = *inout_map_y;
                    if (!csb_v1_runtime_location_after_level_change(
                            dungeon,
                            *inout_map_index,
                            1,
                            &target_x,
                            &target_y,
                            &target_level)) {
                        break;
                    }
                    *inout_map_index = target_level;
                    *inout_map_x = target_x;
                    *inout_map_y = target_y;
                }
                direction = csb_v1_runtime_stairs_exit_direction(
                    dungeon,
                    *inout_map_index,
                    *inout_map_x,
                    *inout_map_y);
                if (direction < 0) {
                    *inout_map_index = old_level;
                    *inout_map_x = old_x;
                    *inout_map_y = old_y;
                    break;
                }
                cell = (*inout_thing >> 14) & 0x03;
                cell = (((cell - direction + 1) & 0x02) >> 1) + direction;
                cell &= 0x03;
                moved_thing = (uint16_t)((*inout_thing & 0x3FFFu) |
                                         (uint16_t)(cell << 14));
                *inout_map_x += step_east[direction];
                *inout_map_y += step_north[direction];
                csb_v1_runtime_process_object_floor_sensors_at(
                    profile, dungeon, *inout_thing, old_level, old_x, old_y,
                    0);
                if (!csb_v1_runtime_unlink_thing_from_square(
                        dungeon,
                        *inout_thing,
                        old_level,
                        old_x,
                        old_y)) {
                    *inout_map_index = old_level;
                    *inout_map_x = old_x;
                    *inout_map_y = old_y;
                    break;
                }
                *inout_thing = moved_thing;
            } else {
                break;
            }
            if (!csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    *inout_thing,
                    *inout_map_index,
                    *inout_map_x,
                    *inout_map_y)) {
                *inout_map_index = old_level;
                *inout_map_x = old_x;
                *inout_map_y = old_y;
                *inout_thing = old_thing;
                (void)csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    old_thing,
                    old_level,
                    old_x,
                    old_y);
                break;
            }
            moved_count++;
            csb_v1_runtime_process_object_floor_sensors_at(
                profile,
                dungeon,
                *inout_thing,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y,
                1);
            continue;
        }
        if ((raw_square & 0x08) == 0) break;
        if (csb_v1_runtime_decode_group_teleporter_at_square(
                dungeon,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y,
                raw_square,
                &teleporter,
                &scope) <= 0 ||
            scope == 0x01) {
            break;
        }
        if (csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
                &teleporter,
                *inout_thing,
                source_map_x,
                &result) != 0) {
            break;
        }
        self_target = teleporter.target_map_index == *inout_map_index &&
                      teleporter.target_map_x == *inout_map_x &&
                      teleporter.target_map_y == *inout_map_y;
        csb_v1_runtime_process_object_floor_sensors_at(
            profile, dungeon, *inout_thing, *inout_map_index, *inout_map_x,
            *inout_map_y, 0);
        if (!csb_v1_runtime_unlink_thing_from_square(
                dungeon,
                *inout_thing,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y)) {
            break;
        }
        *inout_thing = result.thing;
        *inout_map_index = teleporter.target_map_index;
        *inout_map_x = teleporter.target_map_x;
        *inout_map_y = teleporter.target_map_y;
        if (!csb_v1_runtime_append_thing_to_square_tail(
                dungeon,
                *inout_thing,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y)) {
            break;
        }
        moved_count++;
        csb_v1_runtime_process_object_floor_sensors_at(
            profile,
            dungeon,
            *inout_thing,
            *inout_map_index,
            *inout_map_x,
            *inout_map_y,
            1);
        if (self_target) break;
    }
    /* ReDMCSB MOVESENS.C F0267 lines 450-530 lets non-party, non-group
     * objects use object/party-capable teleporters, rejects creature-only
     * teleporters, rotates object cells only for relative teleporters unless
     * the object came from the CM2 projectile-associated-object path, and
     * continues into open non-imaginary pits and non-projectile stairs in the
     * same PC34 100-step chain. This CSB bridge also dispatches bounded C004
     * object floor sensors after successful materialization/movement; buzz
     * audio and broader floor sensor types remain separate runtime work. */
    return moved_count;
}

int csb_v1_runtime_f0267_move_original_object(
    CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    int source_level,
    int source_map_x,
    int source_map_y,
    int destination_level,
    int destination_map_x,
    int destination_map_y)
{
    CSB_V1_DungeonData *dungeon;
    uint16_t moved_thing;
    const uint8_t *record;
    int thing_type;
    int thing_size;

    if (!profile || !(dungeon = profile->dungeon_handle) ||
        !dungeon->raw_data || dungeon->square_bytes != 1 ||
        source_level < 0 || source_level >= dungeon->level_count ||
        destination_level < 0 || destination_level >= dungeon->level_count ||
        csb_v1_dungeon_get_raw_square(
            dungeon, source_level, source_map_x, source_map_y) < 0 ||
        csb_v1_dungeon_get_raw_square(
            dungeon, destination_level, destination_map_x, destination_map_y) < 0 ||
        !csb_v1_runtime_square_first_thing_ptr(
            dungeon, destination_level, destination_map_x, destination_map_y)) {
        return 0;
    }

    record = csb_v1_dungeon_get_thing_record(
        dungeon, thing, &thing_type, NULL, &thing_size);
    if (!record || thing_type <= 4 || thing_type >= 14 || thing_size < 4 ||
        !csb_v1_runtime_square_contains_thing(
            dungeon, thing, source_level, source_map_x, source_map_y)) {
        return 0;
    }

    /* ReDMCSB MOVESENS.C F0267 calls F0276 around the actual original
     * Generic.Next mutation.  This admission deliberately has no decoded
     * map or detached-object fallback: the source chain and destination
     * first-thing slot must both belong to loaded PC34 DUNGEON.DAT bytes. */
    csb_v1_runtime_process_object_floor_sensors_at(
        profile, dungeon, thing, source_level, source_map_x, source_map_y, 0);
    if (!csb_v1_runtime_unlink_thing_from_square(
            dungeon, thing, source_level, source_map_x, source_map_y)) {
        return 0;
    }
    if (!csb_v1_runtime_append_thing_to_square_tail(
            dungeon, thing, destination_level, destination_map_x, destination_map_y)) {
        (void)csb_v1_runtime_append_thing_to_square_tail(
            dungeon, thing, source_level, source_map_x, source_map_y);
        return 0;
    }

    moved_thing = thing;
    csb_v1_runtime_process_object_floor_sensors_at(
        profile, dungeon, moved_thing, destination_level,
        destination_map_x, destination_map_y, 1);
    (void)csb_v1_runtime_apply_object_consequences_at_square(
        profile, dungeon, &moved_thing, source_map_x, &destination_level,
        &destination_map_x, &destination_map_y);
    return 1;
}

static int csb_v1_runtime_materialize_projectile_associated_object(
    CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    const struct ProjectileTickResult_Compat *tick_result)
{
    CSB_V1_DungeonData *dungeon;
    uint16_t associated_thing;
    uint16_t placed_thing;
    int thing_type;
    int map_index;
    int map_x;
    int map_y;
    int cell;

    if (!profile || !projectile || !tick_result) return 0;
    if (!tick_result->despawn ||
        !csb_v1_runtime_projectile_result_places_associated_object(
            tick_result->resultKind)) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    if (!dungeon) return 0;

    associated_thing = (uint16_t)projectile->reserved1;
    if (associated_thing == 0u ||
        associated_thing == 0xFFFEu ||
        associated_thing == 0xFFFFu) {
        return 0;
    }
    thing_type = (associated_thing >> 10) & 0x0F;
    if (thing_type == 14 || associated_thing >= DM1_THING_FIRST_EXPLOSION) {
        return 0;
    }

    map_index = tick_result->newMapIndex;
    map_x = tick_result->newMapX;
    map_y = tick_result->newMapY;
    cell = tick_result->newCell & 3;
    if (map_index < 0 || map_x < 0 || map_y < 0) return 0;

    placed_thing = (uint16_t)((associated_thing & 0x3FFFu) |
                              (uint16_t)(cell << 14));
    /* ReDMCSB PROJEXPL.C F0215 lines 239-259: deleting a projectile whose
     * Projectile.Slot is not an explosion moves that associated object to
     * the projectile square via F0267_MOVE_GetMoveResult_CPSCE. This CSB
     * bridge performs the real-format thing-list writeback for squares whose
     * first-thing slot already exists; first-thing table expansion remains a
     * later full F0267 integration slice. */
    if (!csb_v1_runtime_append_thing_to_square_tail(
            dungeon,
            placed_thing,
            map_index,
            map_x,
            map_y)) {
        return 0;
    }
    csb_v1_runtime_process_object_floor_sensors_at(
        profile,
        dungeon,
        placed_thing,
        map_index,
        map_x,
        map_y,
        1);
    return csb_v1_runtime_apply_object_consequences_at_square(
               profile,
               dungeon,
               &placed_thing,
               CSB_V1_TELEPORTER_ROTATION_SOURCE_PROJECTILE_ASSOCIATED_OBJECT_PC34,
               &map_index,
               &map_x,
               &map_y) >= 0;
}

static int csb_v1_runtime_materialize_launcher_create_failure(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    int map_index,
    const struct ProjectileLauncherLaunch_Compat *launch)
{
    CSB_V1_F0247LauncherMaterializationReceipt_PC34 receipt;
    uint16_t placed_thing;
    int map_x;
    int map_y;

    if (!profile || !dungeon || !launch ||
        !csb_v1_f0247_launcher_create_failure_materialization_pc34_compat(
            launch->associatedThing, launch->mapX, launch->mapY,
            launch->cell, &receipt)) {
        return 0;
    }
    map_x = receipt.map_x;
    map_y = receipt.map_y;
    placed_thing = receipt.thing;
    if (!csb_v1_runtime_append_thing_to_square_tail(
            dungeon, placed_thing, map_index, map_x, map_y)) {
        return 0;
    }
    csb_v1_runtime_process_object_floor_sensors_at(
        profile, dungeon, placed_thing, map_index, map_x, map_y, 1);
    return csb_v1_runtime_apply_object_consequences_at_square(
               profile, dungeon, &placed_thing,
               CSB_V1_TELEPORTER_ROTATION_SOURCE_PROJECTILE_ASSOCIATED_OBJECT_PC34,
               &map_index, &map_x, &map_y) >= 0;
}

static int csb_v1_runtime_collect_square_launcher_things(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    struct ProjectileLauncherSquareThing_Compat *out,
    int out_capacity)
{
    int thing;
    int count = 0;
    int guard;

    if (!dungeon || !out || out_capacity <= 0) return 0;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (thing < 0) return 0;

    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        /* F0247 consumes the live C03 square chain.  A truncated or cyclic
         * chain is not an object source: do not retain a partial candidate
         * set and turn it into a launcher projectile. */
        if (!record || thing_size < 2) return 0;
        if (count < out_capacity) {
            out[count].thing = (unsigned short)thing;
            out[count].cell =
                csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                    (uint16_t)thing);
            out[count].thingType = thing_type;
            ++count;
        }
        thing = (int)csb_v1_runtime_read_u16(record);
    }
    if (guard >= 128) return 0;
    return count;
}

static int csb_v1_runtime_launcher_result_has_loaded_destinations(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    const struct ProjectileLauncherResult_Compat *result)
{
    int index;

    if (!profile || !result || result->launchCount <= 0) return 0;
    for (index = 0; index < result->launchCount; ++index) {
        const struct ProjectileLauncherLaunch_Compat *launch =
            &result->launches[index];

        if (!launch->valid ||
            !csb_v1_runtime_square_byte_ptr(
                profile, map_index, launch->mapX, launch->mapY,
                NULL)) {
            return 0;
        }
    }
    return 1;
}

static uint16_t csb_v1_runtime_thing_with_cell(
    int thing_type,
    int thing_index,
    int cell)
{
    return (uint16_t)(((cell & 0x03) << 14) |
                      ((thing_type & 0x0F) << 10) |
                      (thing_index & 0x03FF));
}

static int csb_v1_runtime_find_unused_object_record(
    CSB_V1_DungeonData *dungeon,
    int thing_type,
    uint8_t **out_record,
    int *out_index)
{
    uint16_t thing;
    int thing_size;
    int thing_index;
    const uint8_t *record;

    if (out_record) *out_record = NULL;
    if (out_index) *out_index = -1;
    if (!dungeon || !dungeon->raw_data ||
        (thing_type != DM1_DROP_THING_TYPE_WEAPON &&
         thing_type != DM1_DROP_THING_TYPE_ARMOUR &&
         thing_type != DM1_DROP_THING_TYPE_JUNK)) {
        return 0;
    }

    thing = csb_v1_dungeon_f0166_get_unused_thing_pc34(
        dungeon, (uint16_t)thing_type, NULL, NULL);
    if (thing == 0xffffu) return 0;
    record = csb_v1_dungeon_get_thing_record(
        dungeon, thing, NULL, &thing_index, &thing_size);
    if (!record || thing_size < 4 || thing_index < 0) return 0;
    if (out_record) *out_record = (uint8_t *)record;
    if (out_index) *out_index = thing_index;
    return 1;
}

static uint16_t csb_v1_runtime_allocate_fixed_possession_thing(
    CSB_V1_DungeonData *dungeon,
    const struct DM1FixedPossessionDrop_Compat *drop)
{
    uint8_t *record;
    uint16_t item_bits;
    int index;

    if (!dungeon || !drop) return 0xFFFFu;
    if (!csb_v1_runtime_find_unused_object_record(
            dungeon,
            drop->thingType,
            &record,
            &index)) {
        return 0xFFFFu;
    }

    item_bits = (uint16_t)(drop->itemType & 0x7F);
    if (drop->cursed) item_bits |= 0x0100u;
    csb_v1_runtime_write_u16(record + 0, 0xFFFEu);
    csb_v1_runtime_write_u16(record + 2, item_bits);
    return csb_v1_runtime_thing_with_cell(
        drop->thingType,
        index,
        drop->cell);
}

static int csb_v1_runtime_new_object_launcher_icon_to_object(
    int icon_index,
    int *out_thing_type,
    int *out_item_type)
{
    int thing_type = DM1_DROP_THING_TYPE_WEAPON;
    int item_type;

    /* ReDMCSB DUNGEON.C F0167 lines 2140-2200 maps object-generator and
     * projectile-launcher icon indices to object thing types.  This bounded
     * CSB runtime bridge covers the launcher objects used by original-style
     * traps and leaves the full object-info table for the broader item DB. */
    if (icon_index >= 4 && icon_index <= 7) icon_index = 4;
    switch (icon_index) {
    case 4:   item_type = 2; break;   /* C004 torch -> C02 weapon torch */
    case 32:  item_type = 8; break;   /* C032 dagger */
    case 51:  item_type = 27; break;  /* C051 arrow */
    case 52:  item_type = 28; break;  /* C052 slayer */
    case 54:  item_type = 30; break;  /* C054 rock */
    case 55:  item_type = 31; break;  /* C055 poison dart */
    case 56:  item_type = 32; break;  /* C056 throwing star */
    case 128:
        item_type = 25;               /* C128 boulder */
        thing_type = DM1_DROP_THING_TYPE_JUNK;
        break;
    default:
        return 0;
    }
    if (out_thing_type) *out_thing_type = thing_type;
    if (out_item_type) *out_item_type = item_type;
    return 1;
}

static uint16_t csb_v1_runtime_allocate_new_object_launcher_thing(
    CSB_V1_DungeonData *dungeon,
    int icon_index)
{
    uint8_t *record;
    int thing_type;
    int item_type;
    int index;

    if (!dungeon ||
        !csb_v1_runtime_new_object_launcher_icon_to_object(
            icon_index,
            &thing_type,
            &item_type)) {
        return 0xFFFFu;
    }
    if (!csb_v1_runtime_find_unused_object_record(
            dungeon,
            thing_type,
            &record,
            &index)) {
        return 0xFFFFu;
    }
    csb_v1_runtime_write_u16(record + 0, 0xFFFEu);
    csb_v1_runtime_write_u16(record + 2, (uint16_t)(item_type & 0x7Fu));
    return csb_v1_runtime_thing_with_cell(thing_type, index, 0);
}

static void csb_v1_runtime_drop_creature_fixed_possessions(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    int creature_type,
    int source_cell,
    int level,
    int map_x,
    int map_y)
{
    struct DM1FixedPossessionDrop_Compat drops[DM1_MAX_FIXED_POSSESSION_DROPS];
    struct RngState_Compat rng;
    int drop_count = 0;
    int weapon_dropped = 0;
    int i;

    if (!profile || !dungeon) return;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)map_x << 8) ^
            ((uint32_t)map_y << 16) ^
            ((uint32_t)creature_type << 24) ^
            0xF0186u);
    if (!F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
            creature_type,
            source_cell,
            &rng,
            drops,
            DM1_MAX_FIXED_POSSESSION_DROPS,
            &drop_count,
            &weapon_dropped)) {
        return;
    }
    /* ReDMCSB GROUP.C F0186 lines 580-645 resolves fixed creature
     * possessions, allocates unused C05/C06/C10 records, stores item type
     * plus cursed bit, assigns a floor cell, moves the thing to the source
     * square through F0267, then requests C00 when any resolved possession
     * is a weapon and C04 otherwise. */
    for (i = 0; i < drop_count; ++i) {
        uint16_t thing = csb_v1_runtime_allocate_fixed_possession_thing(
            dungeon,
            &drops[i]);
        if (thing == 0xFFFFu) continue;
        if (!csb_v1_runtime_append_thing_to_square_tail(
                dungeon,
                thing,
                level,
                map_x,
                map_y)) {
            uint8_t *record;
            int thing_type;
            int thing_size;
            record = csb_v1_runtime_mutable_thing_record(
                dungeon,
                thing,
                &thing_type,
                &thing_size);
            if (record && thing_size >= 2) {
                csb_v1_runtime_write_u16(record, 0xFFFFu);
            }
        } else {
            int drop_level = level;
            int drop_x = map_x;
            int drop_y = map_y;
            csb_v1_runtime_process_object_floor_sensors_at(
                profile,
                dungeon,
                thing,
                drop_level,
                drop_x,
                drop_y,
                1);
            (void)csb_v1_runtime_apply_object_consequences_at_square(
                profile,
                dungeon,
                &thing,
                -1,
                &drop_level,
                &drop_x,
                &drop_y);
        }
    }

    {
        CsbV1AudioRequest request;
        memset(&request, 0, sizeof(request));
        request.soundIndex = weapon_dropped
            ? CSB_V1_SOUND_METALLIC_THUD
            : CSB_V1_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM;
        request.mapX = (int16_t)map_x;
        request.mapY = (int16_t)map_y;
        request.mode = CSB_V1_MODE_PLAY_IMMEDIATELY;
        request.volume = 64;
        request.priority = 4u;
        /* ReDMCSB GROUP.C F0186 line 645 calls F0064 even when every
         * allocation was exhausted; L0362 is based on the resolved table,
         * not on successful F0166 allocations. */
        (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
    }
}

static void csb_v1_runtime_drop_group_slot_possessions(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y)
{
    struct RngState_Compat rng;
    uint16_t thing;
    int guard;
    int weapon_dropped = 0;
    int saw_possession = 0;

    if (!profile || !dungeon || !group_record) return;
    thing = csb_v1_runtime_read_u16(group_record + 2);
    if (thing == 0xFFFEu || thing == 0xFFFFu) return;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)map_x << 8) ^
            ((uint32_t)map_y << 16) ^
            0xF0188u);

    /* ReDMCSB GROUP.C F0188 lines 724-731 walks GROUP.Slot, rewrites each
     * carried thing with a random floor cell, and moves it onto the group
     * square before F0189 deletes the group. */
    for (guard = 0; guard < 64 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        uint8_t *record;
        uint16_t next_thing;
        uint16_t dropped_thing;
        int thing_type;
        int thing_size;

        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) break;
        next_thing = csb_v1_runtime_read_u16(record);
        saw_possession = 1;
        if (thing_type == DM1_DROP_THING_TYPE_WEAPON) {
            weapon_dropped = 1;
        }
        dropped_thing = (uint16_t)((thing & 0x3FFFu) |
                                   (uint16_t)(F0732_COMBAT_RngRandom_Compat(
                                                  &rng,
                                                  4) << 14));
        if (!csb_v1_runtime_append_thing_to_square_tail(
                dungeon,
                dropped_thing,
                level,
                map_x,
                map_y)) {
            break;
        }
        {
            int drop_level = level;
            int drop_x = map_x;
            int drop_y = map_y;
            csb_v1_runtime_process_object_floor_sensors_at(
                profile,
                dungeon,
                dropped_thing,
                drop_level,
                drop_x,
                drop_y,
                1);
            (void)csb_v1_runtime_apply_object_consequences_at_square(
                profile,
                dungeon,
                &dropped_thing,
                -1,
                &drop_level,
                &drop_x,
                &drop_y);
        }
        thing = next_thing;
    }
    csb_v1_runtime_write_u16(group_record + 2, 0xFFFEu);
    if (saw_possession) {
        CsbV1AudioRequest request;
        memset(&request, 0, sizeof(request));
        request.soundIndex = weapon_dropped
            ? CSB_V1_SOUND_METALLIC_THUD
            : CSB_V1_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM;
        request.mapX = (int16_t)map_x;
        request.mapY = (int16_t)map_y;
        request.mode = CSB_V1_MODE_PLAY_IMMEDIATELY;
        request.volume = 64;
        request.priority = 4u;
        /* ReDMCSB GROUP.C F0188 lines 724-734 requests one thud after the
         * complete Slot chain is moved, choosing C00 if any item was C05. */
        (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
    }
}

static uint32_t csb_v1_runtime_champion_occupied_slot_mask(
    const CSB_V1_Champion *champion)
{
    uint32_t mask = 0u;
    int i;

    if (!champion) return 0u;
    for (i = 0; i < CSB_V1_SLOT_COUNT && i < 32; ++i) {
        uint16_t thing = champion->Slots[i];
        if (thing != 0xFFFFu && thing != 0xFFFEu) {
            mask |= (uint32_t)(1u << i);
        }
    }
    return mask;
}

static int csb_v1_runtime_link_stolen_thing_to_group_slot(
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    uint16_t stolen_thing)
{
    uint16_t group_slot;

    if (!dungeon || !group_record ||
        stolen_thing == 0xFFFFu ||
        stolen_thing == 0xFFFEu) {
        return 0;
    }

    group_slot = csb_v1_runtime_read_u16(group_record + 2);
    if (group_slot != 0xFFFFu && group_slot != 0xFFFEu) {
        uint8_t *stolen_record;
        int thing_type;
        int thing_size;

        stolen_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            stolen_thing,
            &thing_type,
            &thing_size);
        if (!stolen_record || thing_size < 2) return 0;
        csb_v1_runtime_write_u16(stolen_record, group_slot);
    }
    /* ReDMCSB GROUP.C F0193 lines 1041-1054 links a stolen object into
     * GROUP.Slot.  It intentionally preserves BUG0_12 for an empty Giggler:
     * the first stolen object's Next word is not cleared before becoming the
     * group slot head. */
    csb_v1_runtime_write_u16(group_record + 2, stolen_thing);
    return 1;
}

static int csb_v1_runtime_apply_giggler_steal_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    const struct DM1_DispatchRecord_V1 *record,
    int creature_index,
    int champion_index)
{
    CSB_V1_Champion *champion;
    struct DM1GigglerStealResult_Compat steal;
    struct RngState_Compat rng;
    uint32_t remaining_mask;
    uint16_t flags;
    int dexterity;
    int attempt;

    if (!profile || !dungeon || !group_record || !record ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount) {
        return 0;
    }

    champion = &profile->party_state.Champions[champion_index];
    dexterity = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_DEX,
        CSB_V1_STAT_CUR);
    F0730_COMBAT_RngInit_Compat(
        &rng,
        csb_v1_runtime_creature_attack_seed(
            profile,
            record,
            DM1_CREATURE_TYPE_GIGGLER,
            creature_index,
            champion_index));
    if (!F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
            dexterity,
            csb_v1_runtime_champion_occupied_slot_mask(champion),
            0,
            &rng,
            &steal)) {
        return 0;
    }

    /* ReDMCSB GROUP.C F0193 lines 1032-1075 walks the source G0025 steal
     * table (DATA.C:244-251) with the shared resolver's counter and
     * backpack RANDOM(17) expansion; the returned stolenSlotMask already
     * carries the exact source slot indices, so the runtime consumes the
     * mask directly instead of re-walking a local slot table. */
    remaining_mask = steal.stolenSlotMask;
    for (attempt = 0; attempt < CSB_V1_SLOT_COUNT && attempt < 32 &&
                    remaining_mask != 0u; ++attempt) {
        int slot = attempt;
        uint32_t slot_mask = (uint32_t)(1u << slot);
        uint16_t stolen_thing;

        if ((remaining_mask & slot_mask) == 0u) continue;
        remaining_mask &= ~slot_mask;
        stolen_thing = champion->Slots[slot];
        if (csb_v1_runtime_link_stolen_thing_to_group_slot(
                dungeon,
                group_record,
                stolen_thing)) {
            champion->Slots[slot] = 0xFFFFu;
        }
    }

    flags = csb_v1_runtime_read_u16(group_record + 14);
    if (steal.shouldFlee) {
        flags = (uint16_t)((flags & 0xFFF0u) |
                           (uint16_t)(steal.newBehavior & 0x0F));
        csb_v1_runtime_write_u16(group_record + 14, flags);
        csb_v1_runtime_schedule_c37_group_event(
            profile,
            record->mapIndex,
            record->mapX,
            record->mapY,
            DM1_CREATURE_TYPE_GIGGLER,
            (uint32_t)((steal.fleeDelayTicks > 0) ?
                           steal.fleeDelayTicks :
                           1));
    } else if (!profile->game_over) {
        csb_v1_runtime_schedule_c38_followup_event(
            profile,
            record->mapIndex,
            record->mapX,
            record->mapY,
            DM1_CREATURE_TYPE_GIGGLER,
            creature_index,
            (uint32_t)csb_v1_runtime_creature_attack_ticks(
                DM1_CREATURE_TYPE_GIGGLER));
    }
    return 1;
}

static void csb_v1_runtime_rewrite_group_events_after_creature_death(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_index)
{
    struct DM1_EventQueue_V1 *queue;
    uint16_t active_indices[DM1_EVENT_MAX_COUNT];
    int active_count;
    int i;

    if (!profile || creature_index < 0 || creature_index > 3) return;
    queue = &profile->timeline_queue;
    active_count = queue->eventCount;
    if (active_count < 0) active_count = 0;
    if (active_count > DM1_EVENT_MAX_COUNT) active_count = DM1_EVENT_MAX_COUNT;
    for (i = 0; i < active_count; ++i) {
        active_indices[i] = queue->timeline[i];
    }

    for (i = 0; i < active_count; ++i) {
        int event_index = active_indices[i];
        struct DM1_Event_V1 *event;
        int event_type;
        int event_creature_index = -1;

        if (event_index < 0 || event_index >= queue->maxEvents) continue;
        event = &queue->events[event_index];
        event_type = event->type;
        if (event_type == DM1_EVENT_NONE ||
            DM1_MAP_TIME_MAP(event->map_time) != (uint32_t)map_index ||
            event->b_mapX != (uint8_t)map_x ||
            event->b_mapY != (uint8_t)map_y) {
            continue;
        }
        if (event_type >= DM1_EVENT_UPDATE_ASPECT_CREATURE_0 &&
            event_type <= DM1_EVENT_UPDATE_ASPECT_CREATURE_3) {
            event_creature_index =
                event_type - DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
        } else if (event_type >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
                   event_type <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            event_creature_index =
                event_type - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
        } else {
            continue;
        }

        /* ReDMCSB GROUP.C F0190 lines 852-875 deletes queued aspect/attack
         * events for the killed creature and decrements higher creature-index
         * event types before fixing timeline heap placement. */
        if (event_creature_index == creature_index) {
            (void)dm1v1_event_delete(queue, event_index);
        } else if (event_creature_index > creature_index) {
            event->type--;
            (void)dm1v1_event_fix_existing_placement(queue, event_index);
        }
    }
}

static void csb_v1_runtime_delete_group_events_at_square(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y)
{
    struct DM1_EventQueue_V1 *queue;
    uint16_t active_indices[DM1_EVENT_MAX_COUNT];
    int active_count;
    int i;

    if (!profile) return;
    queue = &profile->timeline_queue;
    active_count = queue->eventCount;
    if (active_count < 0) active_count = 0;
    if (active_count > DM1_EVENT_MAX_COUNT) active_count = DM1_EVENT_MAX_COUNT;
    for (i = 0; i < active_count; ++i) {
        active_indices[i] = queue->timeline[i];
    }

    for (i = 0; i < active_count; ++i) {
        int event_index = active_indices[i];
        struct DM1_Event_V1 *event;
        int event_type;

        if (event_index < 0 || event_index >= queue->maxEvents) continue;
        event = &queue->events[event_index];
        event_type = event->type;
        /* ReDMCSB GROUP.C F0181 lines 340-366 deletes C29..C41 group
         * reaction/aspect/behavior events for a square after F0189 removes
         * the final creature group. */
        if (event_type >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
            event_type <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 &&
            DM1_MAP_TIME_MAP(event->map_time) == (uint32_t)map_index &&
            event->b_mapX == (uint8_t)map_x &&
            event->b_mapY == (uint8_t)map_y) {
            (void)dm1v1_event_delete(queue, event_index);
        }
    }
}

static int csb_v1_runtime_f0190_smoke_attack_for_creature(int creature_type)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;
    int size;

    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    size = creature_profile ? (creature_profile->attributes & 0x0003) : 0;
    if (size == 0) return 110;
    if (size == 1) return 190;
    return 255;
}

static CSB_V1_RuntimeActiveGroupState *
csb_v1_runtime_active_group_state_for(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int create)
{
    uint16_t i;
    int first_empty = -1;

    if (!profile || ((group_thing >> 10) & 0x0Fu) != 4u) {
        return NULL;
    }
    for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
        CSB_V1_RuntimeActiveGroupState *state =
            &profile->active_group_state[i];
        if (!state->valid) {
            if (first_empty < 0) first_empty = (int)i;
            continue;
        }
        if (state->group_thing == group_thing &&
            state->map_index == level &&
            state->map_x == map_x &&
            state->map_y == map_y) {
            return state;
        }
    }
    if (!create || first_empty < 0 ||
        profile->active_group_state_count >=
            CSB_V1_RUNTIME_ACTIVE_GROUP_CAP) {
        return NULL;
    }
    {
        CSB_V1_RuntimeActiveGroupState *state =
            &profile->active_group_state[first_empty];
        memset(state, 0, sizeof(*state));
        state->valid = 1;
        state->group_thing = group_thing;
        state->map_index = level;
        state->map_x = map_x;
        state->map_y = map_y;
        state->target_map_x = map_x;
        state->target_map_y = map_y;
        ++profile->active_group_state_count;
        return state;
    }
}

static void csb_v1_runtime_clear_active_group_state(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y)
{
    CSB_V1_RuntimeActiveGroupState *state =
        csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            0);
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (profile->active_group_state_count > 0u) {
        --profile->active_group_state_count;
    }
    if (profile->half_square_direction_debounce_valid &&
        profile->half_square_direction_debounce_group == group_thing) {
        profile->half_square_direction_debounce_valid = 0;
    }
}

static CSB_V1_RuntimeActiveGroupState *
csb_v1_runtime_active_group_state_for_thing(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing)
{
    uint16_t i;

    if (!profile || ((group_thing >> 10) & 0x0Fu) != 4u) {
        return NULL;
    }
    for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
        CSB_V1_RuntimeActiveGroupState *state =
            &profile->active_group_state[i];
        if (state->valid && state->group_thing == group_thing) {
            return state;
        }
    }
    return NULL;
}

static void csb_v1_runtime_sync_active_group_state_from_record(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    const uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int preserve_home,
    int moved)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint16_t flags;
    uint16_t directions;
    int direction;
    uint8_t cells;
    int existed;

    /* ReDMCSB GROUP.C F0183/F0184/F0200 keeps active-group Cells,
     * Directions, Aspect[4], GroupThingIndex, Prior/Home map coordinates,
     * and LastMoveTime beside the raw C04 group record. */
    if (!profile || !group_record) return;
    flags = csb_v1_runtime_read_u16(group_record + 14);
    direction = (int)((flags >> 8) & 0x03u);
    cells = group_record[5];
    directions = csb_v1_runtime_repeated_group_direction_pack(direction);

    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    existed = state ? 1 : 0;
    if (existed) {
        directions = state->directions;
    }
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    if (moved) {
        state->prior_map_x = state->map_x;
        state->prior_map_y = state->map_y;
    } else {
        state->prior_map_x = map_x;
        state->prior_map_y = map_y;
    }
    state->map_index = level;
    state->map_x = map_x;
    state->map_y = map_y;
    if (!existed) {
        state->target_map_x = map_x;
        state->target_map_y = map_y;
    }
    if (!preserve_home || !existed) {
        state->home_map_x = map_x;
        state->home_map_y = map_y;
    }
    state->cells = cells;
    state->directions = directions;
    state->last_move_time = moved
        ? profile->game_time
        : (profile->game_time >= 127u ? profile->game_time - 127u : 0u);
}

int csb_v1_runtime_f0195_group_add_all_active_groups(
    CSB_V1_RuntimeProfile *profile)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int map_x;
    int map_y;
    int added = 0;

    if (!profile || !profile->dungeon_handle) return -1;
    dungeon = profile->dungeon_handle;
    level = profile->current_level;
    if (!dungeon->raw_data || level < 0 || level >= dungeon->level_count ||
        dungeon->level_widths[level] <= 0 ||
        dungeon->level_heights[level] <= 0) {
        return -1;
    }

    /* ReDMCSB GROUP.C F0195 walks every square on the party map and adds
     * each C04 group to ActiveGroups. The dungeon accessors preserve the
     * native square-first-thing table and compact Next links. */
    for (map_x = 0; map_x < dungeon->level_widths[level]; ++map_x) {
        for (map_y = 0; map_y < dungeon->level_heights[level]; ++map_y) {
            int thing = csb_v1_dungeon_get_first_thing(
                dungeon, level, map_x, map_y);
            int guard;

            if (thing < 0 || thing == THING_NONE ||
                thing == THING_ENDOFLIST) {
                continue;
            }
            for (guard = 0;
                 guard < 128 && thing != THING_NONE &&
                     thing != THING_ENDOFLIST;
                 ++guard) {
                const uint8_t *record;
                int thing_type = -1;
                int record_size = 0;
                int was_active;

                record = csb_v1_dungeon_get_thing_record(
                    dungeon,
                    (uint16_t)thing,
                    &thing_type,
                    NULL,
                    &record_size);
                if (!record || record_size < 2) return -1;
                if (thing_type == CSB_V1_THING_TYPE_GROUP) {
                    if (record_size < 16) return -1;
                    was_active = csb_v1_runtime_active_group_state_for_thing(
                        profile, (uint16_t)thing) != NULL;
                    csb_v1_runtime_sync_active_group_state_from_record(
                        profile,
                        (uint16_t)thing,
                        record,
                        level,
                        map_x,
                        map_y,
                        0,
                        0);
                    if (!was_active &&
                        csb_v1_runtime_active_group_state_for_thing(
                            profile, (uint16_t)thing)) {
                        ++added;
                    }
                }
                thing = (int)csb_v1_runtime_read_u16(record);
            }
            if (guard == 128 && thing != THING_NONE &&
                thing != THING_ENDOFLIST) {
                return -1;
            }
        }
    }
    return added;
}

static void csb_v1_runtime_set_active_group_target(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int target_x,
    int target_y)
{
    CSB_V1_RuntimeActiveGroupState *state;

    if (!profile || target_x < 0 || target_y < 0) return;
    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    /* ReDMCSB GROUP.C F0209 lines 2111-2112, 2137-2138, and
     * 2247-2252 keep ActiveGroup.TargetMapX/Y as the last visible party
     * square so C7 approach can continue walking after the party is no
     * longer visible. */
    state->target_map_x = target_x;
    state->target_map_y = target_y;
}

static void csb_v1_runtime_set_active_group_aspect_attacking(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    int attacking)
{
    CSB_V1_RuntimeActiveGroupState *state;
    struct RngState_Compat rng;
    uint32_t seed;
    int random_value;

    if (!profile || creature_index < 0 || creature_index > 3) return;
    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;

    if (creature_type < 0 || creature_type >= DM1_CREATURE_TYPE_COUNT) {
        if (attacking) {
            state->aspect[creature_index] =
                (uint8_t)(state->aspect[creature_index] | 0x80u);
        } else {
            state->aspect[creature_index] =
                (uint8_t)(state->aspect[creature_index] & ~0x80u);
        }
        return;
    }

    /* ReDMCSB GROUP.C F0179 lines 222-305 rewrites ActiveGroup.Aspect
     * with horizontal/vertical offset bits plus attack/non-attack flip
     * state, preserving only IS_ATTACKING and FLIP_BITMAP from the previous
     * value before the update. Firestaff reuses the shared DM1/CSB creature
     * graphic-info helper for that bit layout and keeps the RNG local to
     * this bounded runtime slice. */
    seed = profile->dungeon_seed ^
           (uint32_t)(profile->game_time * 2654435761u) ^
           ((uint32_t)group_thing << 10) ^
           ((uint32_t)(creature_index & 3) << 4) ^
           (attacking ? 0xA17Au : 0x51C3u);
    F0730_COMBAT_RngInit_Compat(&rng, seed);
    random_value = F0732_COMBAT_RngRandom_Compat(&rng, 65536);
    state->aspect[creature_index] = dm1_creature_cycle_aspect_frame(
        creature_type,
        state->aspect[creature_index],
        attacking,
        random_value);
}

static void csb_v1_runtime_set_active_group_direction_all(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint16_t flags;

    if (!profile || !group_record) return;
    direction &= 3;
    flags = csb_v1_runtime_read_u16(group_record + 14);
    flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 8)) |
                       (uint16_t)(direction << 8));
    csb_v1_runtime_write_u16(group_record + 14, flags);

    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    /* ReDMCSB GROUP.C F0205/F0206 mutate ActiveGroup.Directions as a
     * 2-bit per-creature field, and F0184 normalizes it back into the GROUP
     * record when the active group is removed.  This bounded CSB bridge
     * writes the shared group facing into all four slots while Firestaff's
     * raw C04 record can still store only the normalized direction. */
    state->directions = csb_v1_runtime_repeated_group_direction_pack(direction);
}

static int csb_v1_runtime_direction_delta(int from_direction, int to_direction)
{
    return (from_direction - to_direction) & 3;
}

static void csb_v1_runtime_set_active_group_direction_creature(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_index,
    int creature_count,
    int two_half_square_creatures)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint16_t flags;
    int final_direction;

    if (!profile || !group_record || creature_index < 0 ||
        creature_index > 3) {
        return;
    }
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    if (creature_index >= creature_count) return;
    direction &= 3;

    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    /* ReDMCSB GROUP.C F0205 lines 1598-1607 debounces two half-square
     * creatures by active-group pointer and game time before mutating
     * directions.  Firestaff's runtime profile owns the equivalent marker so
     * multiple CSB profiles/tests in one process do not share process-global
     * debounce state. */
    if (two_half_square_creatures &&
        profile->half_square_direction_debounce_valid &&
        profile->half_square_direction_debounce_time == profile->game_time &&
        profile->half_square_direction_debounce_group == group_thing) {
        return;
    }

    flags = csb_v1_runtime_read_u16(group_record + 14);
    flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 8)) |
                       (uint16_t)(direction << 8));
    csb_v1_runtime_write_u16(group_record + 14, flags);

    final_direction = direction;
    /* ReDMCSB GROUP.C F0205 lines 1607-1621 changes opposite turns one step
     * at a time.  This bounded bridge uses the deterministic CSB runtime
     * stream seed instead of the process-global source RNG. */
    if (csb_v1_runtime_direction_delta(
            csb_v1_runtime_group_direction_value(
                state->directions,
                creature_index),
            direction) == 2) {
        struct RngState_Compat rng;
        F0730_COMBAT_RngInit_Compat(
            &rng,
            profile->dungeon_seed ^ profile->game_time ^
                ((uint32_t)group_thing << 7) ^
                ((uint32_t)creature_index << 17) ^
                0xF0205u);
        final_direction =
            (direction + 1 +
             (F0732_COMBAT_RngRandom_Compat(&rng, 65536) & 0x0002)) & 3;
    }

    state->directions = csb_v1_runtime_group_directions_set_value(
        state->directions,
        creature_index,
        final_direction);
    if (two_half_square_creatures) {
        /* ReDMCSB F0205 mirrors the direction to the paired half-square
         * creature when F0206 selects the second creature of a two-half group. */
        state->directions = csb_v1_runtime_group_directions_set_value(
            state->directions,
            creature_index ^ 1,
            final_direction);
        profile->half_square_direction_debounce_valid = 1;
        profile->half_square_direction_debounce_time = profile->game_time;
        profile->half_square_direction_debounce_group = group_thing;
    }
}

static void csb_v1_runtime_set_active_group_direction_group(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size)
{
    struct RngState_Compat rng;
    int creature_index;
    int two_half_square_creatures;

    if (!profile || !group_record) return;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    creature_index = creature_count - 1;
    two_half_square_creatures =
        (creature_index != 0 && creature_size == 1) ? 1 : 0;
    if (two_half_square_creatures) {
        creature_index--;
    }
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)group_thing << 5) ^
            ((uint32_t)direction << 19) ^
            0xF0206u);
    do {
        /* ReDMCSB GROUP.C F0206 lines 1632-1645 always lets creature 0 turn
         * and randomly includes higher slots.  This bounded runtime bridge
         * keeps the same shape with Firestaff's deterministic local stream. */
        if (creature_index == 0 ||
            F0732_COMBAT_RngRandom_Compat(&rng, 2) != 0) {
            csb_v1_runtime_set_active_group_direction_creature(
                profile,
                group_thing,
                group_record,
                level,
                map_x,
                map_y,
                direction,
                creature_index,
                creature_count,
                two_half_square_creatures);
        }
    } while (creature_index-- > 0);
}

static void csb_v1_runtime_turn_active_group_toward_attack(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size)
{
    CSB_V1_RuntimeActiveGroupState *state;
    struct RngState_Compat rng;
    int i;
    int two_half_square_creatures;

    if (!profile || !group_record) return;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    direction &= 3;

    csb_v1_runtime_sync_active_group_state_from_record(
        profile,
        group_thing,
        group_record,
        level,
        map_x,
        map_y,
        0,
        0);
    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) return;

    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)group_thing << 3) ^
            ((uint32_t)direction << 21) ^
            0xF0209u);

    /* ReDMCSB GROUP.C F0209 lines 2114-2128 turns attacking groups toward
     * G0382_i_CurrentGroupPrimaryDirectionToParty per creature, only always
     * selecting creature 0; F0205 lines 1609-1621 then performs the one-step
     * opposite-turn clamp and mirrors two half-square pairs. */
    for (i = creature_count - 1; i >= 0; --i) {
        if (csb_v1_runtime_group_direction_value(state->directions, i) ==
            direction) {
            continue;
        }
        if (i != 0 && F0732_COMBAT_RngRandom_Compat(&rng, 2) != 0) {
            continue;
        }
        two_half_square_creatures =
            (i != 0 && creature_size == 1) ? 1 : 0;
        csb_v1_runtime_set_active_group_direction_creature(
            profile,
            group_thing,
            group_record,
            level,
            map_x,
            map_y,
            direction,
            i,
            creature_count,
            two_half_square_creatures);
    }
}

static void csb_v1_runtime_compact_active_group_state_after_kill(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_index,
    int creature_count)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint8_t old_cells;
    uint16_t old_directions;
    int i;

    if (!profile || creature_index < 0 || creature_index >= creature_count) {
        return;
    }
    state = csb_v1_runtime_active_group_state_for(
        profile,
        group_thing,
        level,
        map_x,
        map_y,
        0);
    if (!state) return;

    old_cells = state->cells;
    old_directions = state->directions;
    /* ReDMCSB GROUP.C F0190 lines 892-905 compacts cells/directions for
     * surviving creatures after a partial kill.  F0205/F0206 keep directions
     * in the ACTIVE_GROUP side table, so Firestaff must pack this native state
     * along with the raw GROUP record. */
    for (i = creature_index; i < creature_count - 1 && i < 3; ++i) {
        if (old_cells != 0xFFu) {
            state->cells = (uint8_t)csb_v1_runtime_group_cells_set_value(
                state->cells,
                i,
                csb_v1_runtime_group_cell_value(old_cells, i + 1));
        }
        state->directions = csb_v1_runtime_group_directions_set_value(
            state->directions,
            i,
            csb_v1_runtime_group_direction_value(old_directions, i + 1));
        state->aspect[i] = state->aspect[i + 1];
    }
    if (creature_count > 0 && creature_count <= 4) {
        if (state->cells != 0xFFu) {
            state->cells = (uint8_t)(state->cells &
                (uint8_t)((1u << ((creature_count - 1) * 2)) - 1u));
        }
        state->directions = (uint16_t)(state->directions &
            (uint16_t)((1u << ((creature_count - 1) * 2)) - 1u));
        state->aspect[creature_count - 1] = 0u;
    }
}

static void csb_v1_runtime_write_f0190_flee_delay_to_active_group(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int flee_delay)
{
    CSB_V1_RuntimeActiveGroupState *state;

    if (!profile || flee_delay <= 0) return;
    state = csb_v1_runtime_active_group_state_for(
        profile,
        group_thing,
        level,
        map_x,
        map_y,
        1);
    if (!state) return;
    state->delay_fleeing_from_target =
        (uint8_t)(flee_delay > 255 ? 255 : flee_delay);
}

static void csb_v1_runtime_write_f0190_flee_delay_to_item16(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int flee_delay)
{
    uint16_t i;
    uint8_t delay;

    if (!profile || flee_delay <= 0) return;
    delay = (uint8_t)(flee_delay > 255 ? 255 : flee_delay);

    for (i = 0u; i < profile->csbwin_runtime_item16_count; ++i) {
        CSB_V1_CSBWinRuntimeItem16 *item =
            &profile->csbwin_runtime_item16[i];
        if (!item->valid ||
            !item->live_ai_owned ||
            item->live_ai_group_thing != group_thing ||
            item->live_ai_map_index != level ||
            item->live_ai_map_x != map_x ||
            item->live_ai_map_y != map_y) {
            continue;
        }
        item->delay_or_flee_timer = delay;
    }

    if (!profile->csbwin_body_runtime_summary_valid) return;
    for (i = 0u; i < profile->csbwin_item16_summary_count; ++i) {
        CSB_V1_CSBWin512Item16Summary *item =
            &profile->csbwin_item16[i];
        if (!item->valid ||
            csb_v1_runtime_csbwin_item16_group_thing(item->monster_index) !=
                group_thing ||
            item->current_x != (uint8_t)map_x ||
            item->current_y != (uint8_t)map_y) {
            continue;
        }
        item->ubyte5 = delay;
    }
}

static void csb_v1_runtime_apply_f0190_fear_after_partial_kill(
    CSB_V1_RuntimeProfile *profile,
    uint8_t *group_record,
    uint16_t group_thing,
    uint16_t *inout_flags,
    int creature_type,
    int creature_count,
    int level,
    int map_x,
    int map_y,
    struct RngState_Compat *rng)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;
    struct DM1GroupBehaviorContext_Compat ctx;
    int should_flee = 0;
    int flee_delay = 0;

    if (!profile || !group_record || !inout_flags || !rng ||
        level != profile->current_level ||
        ((*inout_flags) & 0x000Fu) != 6u) {
        return;
    }
    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    if (!creature_profile) return;
    memset(&ctx, 0, sizeof(ctx));
    ctx.currentGroupMapX = map_x;
    ctx.currentGroupMapY = map_y;
    ctx.creatureType = creature_type;
    ctx.creatureInfo.properties = creature_profile->properties;

    /* ReDMCSB GROUP.C F0190 lines 887-890 tests fear only for attacking
     * groups on the party map, stores DelayFleeingFromTarget on ACTIVE_GROUP,
     * and switches GROUP.Behavior to C5.  Firestaff's CSB native active-group
     * array is still bounded, but a CSBWin-imported ITEM16 record that has
     * claimed this live C04 group is the matching active-monster side state. */
    if (F0821_DM1_GROUP_ShouldFrighten_Compat(
            &ctx,
            creature_count,
            rng,
            &should_flee,
            &flee_delay) &&
        should_flee) {
        *inout_flags = (uint16_t)((*inout_flags & ~(uint16_t)0x000Fu) | 5u);
        csb_v1_runtime_write_u16(group_record + 14, *inout_flags);
        csb_v1_runtime_write_f0190_flee_delay_to_active_group(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            flee_delay);
        csb_v1_runtime_write_f0190_flee_delay_to_item16(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            flee_delay);
    }
}

static void csb_v1_runtime_spawn_f0190_death_smoke(
    CSB_V1_RuntimeProfile *profile,
    int creature_type,
    int killed_cell,
    int map_index,
    int map_x,
    int map_y)
{
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_advance;
    int slot = -1;

    if (!profile) return;
    memset(&input, 0, sizeof(input));
    /* ReDMCSB GROUP.C F0190 lines 907-917 creates C040 smoke at the
     * killed creature cell, with attack 110/190/255 by creature size. */
    input.explosionType = C040_EXPLOSION_SMOKE;
    input.attack = csb_v1_runtime_f0190_smoke_attack_for_creature(creature_type);
    input.mapIndex = map_index;
    input.mapX = map_x;
    input.mapY = map_y;
    input.cell = (killed_cell == EXPLOSION_CELL_CENTERED)
        ? EXPLOSION_CELL_CENTERED : (killed_cell & 3);
    input.centered = (input.cell == EXPLOSION_CELL_CENTERED) ? 1 : 0;
    input.currentTick = (int)profile->game_time;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    if (F0821_EXPLOSION_Create_Compat(
            &input,
            &profile->explosions,
            &slot,
            &first_advance)) {
        csb_v1_runtime_schedule_explosion_advance_event(profile, &first_advance);
    }
}

static void csb_v1_runtime_pack_dead_group_creature(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_index,
    struct RngState_Compat *rng)
{
    uint16_t flags;
    int raw_count;
    int creature_count;
    int cells;
    int killed_cell;
    int creature_type;
    int i;

    if (!dungeon || !group_record ||
        creature_index < 0 || creature_index > 3) {
        return;
    }
    flags = csb_v1_runtime_read_u16(group_record + 14);
    raw_count = (int)((flags >> 5) & 0x03u);
    creature_count = raw_count + 1;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    if (creature_index >= creature_count) return;
    cells = group_record[5];
    killed_cell = (cells == 0xFF)
        ? EXPLOSION_CELL_CENTERED
        : csb_v1_runtime_group_cell_value(cells, creature_index);
    creature_type = group_record[4];
    csb_v1_runtime_spawn_f0190_death_smoke(
        profile,
        creature_type,
        killed_cell,
        level,
        map_x,
        map_y);
    csb_v1_runtime_drop_creature_fixed_possessions(
        profile,
        dungeon,
        creature_type,
        killed_cell,
        level,
        map_x,
        map_y);

    if (creature_count <= 1) {
        /* ReDMCSB GROUP.C F0190 lines 831-840 calls F0189_GROUP_Delete
         * when the last creature dies.  This bounded CSB bridge removes the
         * C04 thing from the square chain, drops the carried Slot chain, and
         * marks the real-format record unused; fixed possessions, sounds, and
         * active-group side state are later slices. */
        csb_v1_runtime_drop_group_slot_possessions(
            profile,
            dungeon,
            group_record,
            level,
            map_x,
            map_y);
        csb_v1_runtime_delete_group_events_at_square(
            profile,
            level,
            map_x,
            map_y);
        csb_v1_runtime_clear_active_group_state(
            profile,
            group_thing,
            level,
            map_x,
            map_y);
        csb_v1_runtime_unlink_group_thing_from_square(
            dungeon,
            group_thing,
            level,
            map_x,
            map_y);
        memset(group_record, 0, 16);
        csb_v1_runtime_write_u16(group_record + 0, 0xFFFFu);
        csb_v1_runtime_write_u16(group_record + 2, 0xFFFEu);
        return;
    }

    if ((flags & 0x000fu) == 6u) {
        csb_v1_runtime_rewrite_group_events_after_creature_death(
            profile,
            level,
            map_x,
            map_y,
            creature_index);
        csb_v1_runtime_apply_f0190_fear_after_partial_kill(
            profile,
            group_record,
            group_thing,
            &flags,
            creature_type,
            creature_count,
            level,
            map_x,
            map_y,
            rng);
    }

    /* ReDMCSB GROUP.C F0190 lines 892-905 compacts Health, directions,
     * cells, active aspect, then decrements GROUP.Count.  CSB's bounded
     * real-format bridge owns Health/Cells and mirrors the native
     * active-group Cells/Directions/Aspect side-table compaction here. */
    csb_v1_runtime_compact_active_group_state_after_kill(
        profile,
        group_thing,
        level,
        map_x,
        map_y,
        creature_index,
        creature_count);
    for (i = creature_index; i < creature_count - 1; ++i) {
        uint16_t next_hp =
            csb_v1_runtime_read_u16(group_record + 6 + (i + 1) * 2);
        int next_cell = csb_v1_runtime_group_cell_value(cells, i + 1);
        csb_v1_runtime_write_u16(group_record + 6 + i * 2, next_hp);
        cells = csb_v1_runtime_group_cells_set_value(cells, i, next_cell);
    }
    csb_v1_runtime_write_u16(group_record + 6 + (creature_count - 1) * 2, 0);
    if (cells != 0xFF) {
        cells &= (1 << ((creature_count - 1) * 2)) - 1;
    }
    group_record[5] = (uint8_t)(cells & 0xFF);
    flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 5)) |
                       (uint16_t)(((raw_count - 1) & 0x03) << 5));
    csb_v1_runtime_write_u16(group_record + 14, flags);
}

static int csb_v1_runtime_apply_group_fall_damage(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *group_record;
    struct RngState_Compat rng;
    uint16_t flags;
    int thing_type = -1;
    int thing_size = 0;
    int creature_count;
    int random_window;
    int base_attack;
    int killed_some = 0;
    int i;

    if (!profile || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    group_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        group_thing,
        &thing_type,
        &thing_size);
    if (!group_record || thing_type != 4 || thing_size < 16) return 0;

    flags = csb_v1_runtime_read_u16(group_record + 14);
    creature_count = (int)((flags >> 5) & 0x03u) + 1;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    random_window = (20 >> 3) + 1;
    base_attack = 20 - random_window;
    random_window <<= 1;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)map_index << 4) ^
            ((uint32_t)map_x << 8) ^
            ((uint32_t)map_y << 16) ^
            0xF0191u);

    /* ReDMCSB MOVESENS.C F0267 lines 596-617 applies GROUP.C F0191 with
     * attack 20 after a moving C04 group falls through a pit.  GROUP.C F0191
     * lines 952-973 fans out attack +/- 1/8 from Count down to creature 0.
     * This bounded bridge reuses the existing real-format F0190 pack/drop/
     * smoke helper; ActiveGroup side state and audio remain separate work. */
    for (i = creature_count - 1; i >= 0; --i) {
        uint16_t hp;
        int damage;

        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_type != 4 || thing_size < 16) {
            return killed_some ? 2 : 0;
        }
        hp = csb_v1_runtime_read_u16(group_record + 6 + i * 2);
        if (hp == 0) continue;
        damage = base_attack + F0732_COMBAT_RngRandom_Compat(
            &rng,
            random_window);
        if (damage < 1) damage = 1;
        if (damage >= (int)hp) {
            killed_some = 1;
            csb_v1_runtime_pack_dead_group_creature(
                profile,
                dungeon,
                group_record,
                group_thing,
                map_index,
                map_x,
                map_y,
                i,
                &rng);
            if (creature_count <= 1) {
                return 2;
            }
        } else {
            csb_v1_runtime_write_u16(
                group_record + 6 + i * 2,
                (uint16_t)((int)hp - damage));
        }
    }
    group_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        group_thing,
        &thing_type,
        &thing_size);
    if (!group_record || thing_type != 4 || thing_size < 16) {
        return killed_some ? 2 : 0;
    }
    if (csb_v1_runtime_read_u16(group_record + 0) == 0xFFFFu) {
        return 2;
    }
    return killed_some ? 1 : 0;
}

static int csb_v1_runtime_apply_explosion_group_action(
    CSB_V1_RuntimeProfile *profile,
    const struct CombatAction_Compat *action,
    struct RngState_Compat *rng)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *thing_record;
    int first_thing;
    int thing;
    int guard;
    int thing_type;
    int thing_size;
    int applied = 0;

    if (!profile || !action || !rng ||
        action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP ||
        action->rawAttackValue <= 0 ||
        !profile->dungeon_handle) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    {
        uint16_t group_thing;
        if (!csb_v1_f0217_find_group_thing_pc34_compat(
                dungeon, action->targetMapIndex, action->targetMapX,
                action->targetMapY, &group_thing)) {
            return 0;
        }
        first_thing = (int)group_thing;
    }

    for (guard = 0, thing = first_thing; guard == 0; ++guard) {
        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record || thing_size < 16) return applied;
        if (thing_type == 4) {
            uint16_t flags;
            int creature_count;
            int random_window;
            int base_attack;
            int i;

            flags = csb_v1_runtime_read_u16(thing_record + 14);
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            if (creature_count < 1) creature_count = 1;
            if (creature_count > 4) creature_count = 4;
            random_window = (action->rawAttackValue >> 3) + 1;
            base_attack = action->rawAttackValue - random_window;
            random_window <<= 1;

            /* ReDMCSB GROUP.C F0191 lines 952-973 applies the same
             * +/- 1/8 all-creature attack fanout used by explosion group
             * impacts in PROJEXPL.C F0213/F0220.  This real-format bridge
             * mutates GROUP.Health[4]; active-group aspect cleanup, drops,
             * and fixed possessions remain later CSB runtime slices. */
            for (i = 0; i < creature_count; ) {
                uint8_t *hp_ptr = thing_record + 6 + i * 2;
                uint16_t hp = csb_v1_runtime_read_u16(hp_ptr);
                int damage;
                if (hp == 0) {
                    i++;
                    continue;
                }
                damage = base_attack +
                    F0732_COMBAT_RngRandom_Compat(rng, random_window);
                if (damage < 1) damage = 1;
                if (damage >= (int)hp) {
                    csb_v1_runtime_pack_dead_group_creature(
                        profile,
                        dungeon,
                        thing_record,
                        (uint16_t)thing,
                        action->targetMapIndex,
                        action->targetMapX,
                        action->targetMapY,
                        i,
                        rng);
                    creature_count--;
                    if (creature_count <= 0) {
                        applied++;
                        return applied;
                    }
                } else {
                    csb_v1_runtime_write_u16(
                        hp_ptr,
                        (uint16_t)((int)hp - damage));
                    i++;
                }
                applied++;
            }
            return applied;
        }
        thing = csb_v1_runtime_read_u16(thing_record + 0);
    }
    return applied;
}

static int csb_v1_runtime_projectile_weapon_type_is_kept_sharp(int weapon_type)
{
    return weapon_type == 8  ||  /* C08_WEAPON_DAGGER */
           weapon_type == 27 ||  /* C27_WEAPON_ARROW */
           weapon_type == 28 ||  /* C28_WEAPON_SLAYER */
           weapon_type == 31 ||  /* C31_WEAPON_POISON_DART */
           weapon_type == 32;    /* C32_WEAPON_THROWING_STAR */
}

static int csb_v1_runtime_link_projectile_thing_to_group_slot_tail(
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    uint16_t associated_thing)
{
    uint16_t group_slot;
    uint16_t tail;
    int guard;

    if (!dungeon || !group_record ||
        associated_thing == 0xFFFEu ||
        associated_thing == 0xFFFFu) {
        return 0;
    }
    group_slot = csb_v1_runtime_read_u16(group_record + 2);
    if (group_slot == 0xFFFEu || group_slot == 0xFFFFu) {
        uint8_t *record;
        int thing_type;
        int thing_size;
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            associated_thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        csb_v1_runtime_write_u16(record, 0xFFFEu);
        csb_v1_runtime_write_u16(group_record + 2, associated_thing);
        return 1;
    }

    tail = group_slot;
    for (guard = 0; guard < 64 && tail != 0xFFFEu && tail != 0xFFFFu;
         ++guard) {
        uint8_t *record;
        uint16_t next_thing;
        int thing_type;
        int thing_size;
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            tail,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(record);
        if (next_thing == 0xFFFEu || next_thing == 0xFFFFu) {
            uint8_t *associated_record =
                csb_v1_runtime_mutable_thing_record(
                    dungeon,
                    associated_thing,
                    &thing_type,
                    &thing_size);
            if (!associated_record || thing_size < 2) return 0;
            csb_v1_runtime_write_u16(associated_record, 0xFFFEu);
            csb_v1_runtime_write_u16(record, associated_thing);
            return 1;
        }
        tail = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_maybe_attach_projectile_weapon_to_group_slot(
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    const struct ProjectileInstance_Compat *projectile)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;
    uint8_t *weapon_record;
    uint16_t associated_thing;
    int thing_type;
    int thing_size;
    int weapon_type;
    int creature_type;

    if (!dungeon || !group_record || !projectile) return 0;
    associated_thing = (uint16_t)projectile->reserved1;
    if (associated_thing == 0u ||
        associated_thing == 0xFFFEu ||
        associated_thing == 0xFFFFu ||
        ((associated_thing >> 10) & 0x0Fu) != 5u ||
        (projectile->flags & PROJECTILE_FLAG_CREATES_EXPLOSION) != 0) {
        return 0;
    }
    creature_type = group_record[4];
    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    if (!creature_profile ||
        ((creature_profile->attributes &
          CREATURE_ATTR_MASK_KEEP_THROWN_SHARP_WEAPONS) == 0)) {
        return 0;
    }
    weapon_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        associated_thing,
        &thing_type,
        &thing_size);
    if (!weapon_record || thing_type != 5 || thing_size < 4) return 0;
    weapon_type = (int)(csb_v1_runtime_read_u16(weapon_record + 2) & 0x7Fu);
    if (!csb_v1_runtime_projectile_weapon_type_is_kept_sharp(weapon_type)) {
        return 0;
    }

    /* ReDMCSB PROJEXPL.C F0217 lines 540-553 selects GROUP.Slot for
     * surviving dagger/arrow/slayer/poison-dart/throwing-star impacts
     * against creatures with KEEP_THROWN_SHARP_WEAPONS; F0215 lines
     * 239-256 then appends the projectile associated thing to that slot
     * list instead of moving it to the floor square. */
    return csb_v1_runtime_link_projectile_thing_to_group_slot_tail(
        dungeon,
        group_record,
        associated_thing);
}

static int csb_v1_runtime_apply_projectile_group_action(
    CSB_V1_RuntimeProfile *profile,
    const struct CombatAction_Compat *action,
    struct ProjectileInstance_Compat *projectile)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *thing_record;
    int first_thing;
    int thing;
    int guard;
    int thing_type;
    int thing_size;
    struct RngState_Compat rng;

    if (!profile || !action || !projectile ||
        action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP ||
        action->rawAttackValue <= 0 ||
        !profile->dungeon_handle) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        action->targetMapIndex,
        action->targetMapX,
        action->targetMapY);
    if (first_thing < 0) return 0;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)(projectile->slotIndex & 0xFF) << 4) ^
            ((uint32_t)(projectile->mapX & 0xFF) << 12) ^
            ((uint32_t)(projectile->mapY & 0xFF) << 20) ^
            0xF0190u);

    for (guard = 0, thing = first_thing;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record || thing_type != 4 || thing_size < 16) return 0;
        if (thing_type == 4) {
            uint16_t flags;
            uint16_t hp;
            uint8_t *hp_ptr;
            int creature_count;
            int creature_index = -1;
            int cells;
            int i;

            flags = csb_v1_runtime_read_u16(thing_record + 14);
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            if (creature_count < 1) creature_count = 1;
            if (creature_count > 4) creature_count = 4;
            cells = thing_record[5];
            if (cells == 0xFF) {
                creature_index = 0;
            } else {
                for (i = 0; i < creature_count; ++i) {
                    if (csb_v1_runtime_group_cell_value(cells, i) ==
                        (action->targetCell & 3)) {
                        creature_index = i;
                        break;
                    }
                }
            }
            if (creature_index < 0 || creature_index >= creature_count) {
                return 0;
            }
            hp_ptr = thing_record + 6 + creature_index * 2;
            hp = csb_v1_runtime_read_u16(hp_ptr);
            if (hp == 0) return 0;
            if (action->rawAttackValue >= (int)hp) {
                csb_v1_runtime_pack_dead_group_creature(
                    profile,
                    dungeon,
                    thing_record,
                    (uint16_t)thing,
                    action->targetMapIndex,
                    action->targetMapX,
                    action->targetMapY,
                    creature_index,
                    &rng);
                return 1;
            }

            csb_v1_runtime_write_u16(
                hp_ptr,
                (uint16_t)((int)hp - action->rawAttackValue));
            if (csb_v1_runtime_maybe_attach_projectile_weapon_to_group_slot(
                    dungeon,
                    thing_record,
                    projectile)) {
                projectile->reserved1 = 0xFFFF;
            }
            return 1;
        }
    }
    return 0;
}

static void csb_v1_runtime_apply_creature_attack_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *thing_record;
    uint16_t flags;
    int thing;
    int guard;
    int creature_index;
    int creature_count;
    int thing_type;
    int thing_size;
    int champion_index;
    int creature_cell;
    int damage;
    struct CombatantCreatureSnapshot_Compat attacker;
    struct CombatantChampionSnapshot_Compat defender;
    struct CombatResult_Compat combat;
    struct RngState_Compat rng;

    if (!profile || !record || !profile->dungeon_handle ||
        !profile->party_state_valid) {
        return;
    }
    creature_index = record->eventType - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    if (creature_index < 0 || creature_index > 3) return;

    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB GROUP.C F0209 lines 1443-1515 processes C38-C41 as
     * per-creature attack decisions after C6 attack entry and calls the
     * common creature melee damage path (PROJEXPL.C F0230, then
     * CHAMPION.C F0321).  CSB keeps the real-format group lookup and target
     * selection here, then delegates the bounded damage roll to the shared
     * M10 combat resolver used by DM1.  Full CSB runtime RNG state, poison,
     * armor inventory, rest wake, ranged attacks, and broader aspect timing
     * remain later slices. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 4 && thing_size >= 16) {
            flags = csb_v1_runtime_read_u16(thing_record + 14);
            if ((flags & 0x000Fu) != 6u) return;
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            if (creature_index >= creature_count) return;
            creature_cell = (thing_record[3] == 0xFFu)
                ? 0
                : ((int)thing_record[3] >> (creature_index * 2)) & 0x03;
            champion_index = csb_v1_runtime_target_champion_for_adjacent_attack(
                profile,
                record->mapX,
                record->mapY,
                creature_cell);
            if (champion_index < 0) {
                champion_index =
                    csb_v1_runtime_first_living_champion(&profile->party_state);
            }
            if (champion_index < 0) return;
            csb_v1_runtime_set_active_group_direction_creature(
                profile,
                (uint16_t)thing,
                thing_record,
                record->mapIndex,
                record->mapX,
                record->mapY,
                csb_v1_runtime_direction_from_source_to_destination(
                    record->mapX,
                    record->mapY,
                    profile->party_x,
                    profile->party_y),
                creature_index,
                creature_count,
                0);
            csb_v1_runtime_set_active_group_aspect_attacking(
                profile,
                (uint16_t)thing,
                record->mapIndex,
                record->mapX,
                record->mapY,
                (int)thing_record[4],
                creature_index,
                1);
            csb_v1_runtime_request_creature_attack_sound(
                profile,
                (int)thing_record[4],
                record->mapX,
                record->mapY);
            if ((int)thing_record[4] == DM1_CREATURE_TYPE_GIGGLER) {
                (void)csb_v1_runtime_apply_giggler_steal_timeline_record(
                    profile,
                    dungeon,
                    thing_record,
                    record,
                    creature_index,
                    champion_index);
                return;
            }
            memset(&combat, 0, sizeof(combat));
            if (!csb_v1_runtime_fill_creature_combat_snapshot(
                    (int)thing_record[4],
                    creature_index,
                    &attacker)) {
                return;
            }
            if (!csb_v1_runtime_fill_defender_combat_snapshot(
                    profile,
                    champion_index,
                    &defender)) {
                return;
            }
            (void)F0730_COMBAT_RngInit_Compat(
                &rng,
                csb_v1_runtime_creature_attack_seed(
                    profile,
                    record,
                    attacker.creatureType,
                    creature_index,
                    champion_index));
            if (!F0736_COMBAT_ResolveCreatureMelee_Compat(
                    &attacker,
                    &defender,
                    &rng,
                    &combat)) {
                return;
            }
            damage = (combat.outcome == COMBAT_OUTCOME_HIT_DAMAGE &&
                      combat.damageApplied > 0)
                ? combat.damageApplied
                : 0;
            if (damage > 0) {
                int filtered_damage;

                /* CSBWin Monster.cpp:4541-4545 passes its final C38 damage,
                 * selected wound mask, and descriptor attack type through
                 * Character.cpp::DamageCharacter before mutating the hero. */
                if (csb_v1_runtime_execute_csbwin_damage_character_filter(
                        profile, champion_index, damage,
                        (uint16_t)combat.woundMaskAdded,
                        (uint16_t)attacker.attackType, &filtered_damage)) {
                    damage = filtered_damage;
                }
            }
            if (damage <= 0) {
                if (!profile->game_over) {
                    csb_v1_runtime_schedule_c38_followup_event(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        creature_index,
                        (uint32_t)csb_v1_runtime_creature_attack_ticks(
                            (int)thing_record[4]));
                }
                return;
            }
            if (profile->party_state.Champions[champion_index].CurrentHealth <=
                damage) {
                profile->party_state.Champions[champion_index].Wounds =
                    (uint16_t)(profile->party_state
                                   .Champions[champion_index]
                                   .Wounds |
                               (uint16_t)combat.woundMaskAdded);
                csb_v1_runtime_mark_champion_dead(profile, champion_index);
            } else {
                profile->party_state.Champions[champion_index].Wounds =
                    (uint16_t)(profile->party_state
                                   .Champions[champion_index]
                                   .Wounds |
                               (uint16_t)combat.woundMaskAdded);
                profile->party_state.Champions[champion_index].CurrentHealth =
                    (int16_t)(profile->party_state
                                  .Champions[champion_index]
                                  .CurrentHealth -
                              damage);
                if (combat.poisonAttackPending > 0) {
                    csb_v1_runtime_apply_poison_attack_to_champion(
                        profile,
                        champion_index,
                        combat.poisonAttackPending);
                }
            }
            if (!profile->game_over) {
                /* ReDMCSB GROUP.C F0209 lines 2343-2422 computes the next
                 * C38 attack time and F0208 lines 1820-1834 may convert the
                 * paired earlier aspect update into C33..C36 with C.Ticks
                 * carrying the remaining attack delay; when C33 dispatches
                 * it prepares the matching C38.  This bounded CSB bridge
                 * keeps the source AttackTicks base and explicit C33->C38
                 * handoff; RNG jitter and live ActiveGroup aspect sprites
                 * remain later slices. */
                uint32_t attack_delay =
                    (uint32_t)csb_v1_runtime_creature_attack_ticks(
                        (int)thing_record[4]);
                csb_v1_runtime_schedule_c38_followup_event(
                    profile,
                    record->mapIndex,
                    record->mapX,
                    record->mapY,
                    (int)thing_record[4],
                    creature_index,
                    attack_delay);
            }
            return;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_trigger_floor_sensor_event(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell,
    int delay,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    struct DM1_Event_V1 event;
    int raw_square;
    int square_type;
    int event_type;
    uint32_t old_time;

    if (!profile || !result) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data) return;
    if (level < 0 || level >= dungeon->level_count) return;

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        target_x,
        target_y);
    if (raw_square < 0) return;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    event_type = csb_v1_runtime_square_event_type_for_sensor_target(square_type);
    if (event_type == DM1_EVENT_NONE) return;
    /* ReDMCSB MOVESENS.C F0272 passes Remote.TargetCell to F0268 only for
     * wall targets. Every other target square is addressed at northwest. */
    if (square_type != DM1_SQUARE_WALL) target_cell = 0;

    old_time = profile->game_time;
    if (delay > 0) profile->game_time += (uint32_t)delay;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        level,
        profile->game_time);
    event.type = (uint8_t)event_type;
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)target_cell;
    event.c_effect = (uint8_t)sensor_effect;
    if (dm1v1_event_add(&profile->timeline_queue, &event) >= 0) {
        result->sensor_event_count++;
        result->sensor_last_event_type = event_type;
    }
    profile->game_time = old_time;
}

static int csb_v1_runtime_queue_remote_square_event(
    CSB_V1_RuntimeProfile *profile,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell)
{
    const CSB_V1_DungeonData *dungeon;
    struct DM1_Event_V1 event;
    int raw_square;
    int square_type;
    int event_type;

    if (!profile) return 0;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data) return 0;

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        profile->current_level,
        target_x,
        target_y);
    if (raw_square < 0) return 0;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    event_type = csb_v1_runtime_square_event_type_for_sensor_target(square_type);
    if (event_type == DM1_EVENT_NONE) return 0;
    /* ReDMCSB MOVESENS.C F0272:1201-1207 preserves Remote.TargetCell only
     * for a wall target. F0268 receives CELL_NORTHWEST for every other
     * square type, including fakewalls, teleporters, pits, and doors. */
    if (square_type != DM1_SQUARE_WALL) target_cell = 0;

    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        profile->current_level,
        profile->game_time);
    event.type = (uint8_t)event_type;
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)target_cell;
    event.c_effect = (uint8_t)sensor_effect;
    return dm1v1_event_add(&profile->timeline_queue, &event) >= 0 ? 1 : 0;
}

static int csb_v1_runtime_object_type_from_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing)
{
    const uint8_t *record;
    int thing_type;
    int thing_size;

    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_type <= 4 || thing_type >= 14 || thing_size < 4) {
        return -1;
    }
    return (int)(csb_v1_runtime_read_u16(record + 2) & 0x007Fu);
}

static void csb_v1_runtime_process_object_floor_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t placed_thing,
    int level,
    int map_x,
    int map_y,
    int add_thing)
{
    int raw_square;
    int first_thing;
    int thing;
    int object_type;
    int placed_thing_type;
    int guard;
    int pending_local_effect = DM1_EFFECT_NONE;

    if (!profile || !dungeon || !dungeon->raw_data ||
        dungeon->square_bytes != 1) {
        return;
    }
    if (profile->dungeon_handle != dungeon) return;
    if (level < 0 || level >= dungeon->level_count) return;
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0) return;
    if (((raw_square >> 5) & 0x07) == DM1_SQUARE_WALL) {
        csb_v1_runtime_process_object_wall_sensors_at(
            profile, dungeon, placed_thing, level, map_x, map_y, add_thing);
        return;
    }
    if (!csb_v1_dungeon_get_thing_record(
            dungeon,
            placed_thing,
            &placed_thing_type,
            NULL,
            NULL)) {
        return;
    }
    object_type = csb_v1_runtime_object_type_from_thing(dungeon, placed_thing);
    if (object_type < 0) return;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (first_thing < 0 || first_thing == 0xFFFE || first_thing == 0xFFFF) {
        return;
    }

    /* ReDMCSB MOVESENS.C F0276 lines 1608-1655 classifies the moving
     * THING, scans the square for matching object types before add, then
     * lines 1691-1694 trigger C004 only when the sensor data matches
     * F0032_OBJECT_GetType(P0590_T_Thing) and no same-type object is already
     * present. Lines 1666-1669 trigger C001 for the added/removed object
     * unless the square already holds another object or a group. Firestaff
     * calls this after link (add) or before unlink (removal), so the scan
     * ignores the just-placed THING and reproduces the source pre-link /
     * post-unlink occupancy observation for both guards. */
    thing = first_thing;
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int same_type_present = 0;
        int object_present = 0;
        int group_present = 0;
        int scan;
        int scan_guard;
        int sensor_effect;
        int trigger;
        int target_cell;
        int target_x;
        int target_y;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record) break;
        if (thing_type >= 4) break;
        if (thing_type != 3 || thing_size < 8) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        type_data = csb_v1_runtime_read_u16(record + 2);
        flags_word = csb_v1_runtime_read_u16(record + 4);
        target_word = csb_v1_runtime_read_u16(record + 6);
        sensor_type = (int)(type_data & 0x007Fu);
        sensor_data = (int)(type_data >> 7);
        if (sensor_type != DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT &&
            sensor_type != DM1_SENSOR_FLOOR_OBJECT) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        scan = first_thing;
        for (scan_guard = 0;
             scan_guard < 128 && scan != 0xFFFE && scan != 0xFFFF;
             ++scan_guard) {
            int scan_type;
            const uint8_t *scan_record = csb_v1_dungeon_get_thing_record(
                dungeon,
                (uint16_t)scan,
                &scan_type,
                NULL,
                NULL);
            if (!scan_record) break;
            if (scan_type == 4) {           /* C04_THING_TYPE_GROUP */
                group_present = 1;
            } else if ((uint16_t)scan != placed_thing &&
                       scan_type > 4 && scan_type < 14) {
                object_present = 1;
                if (csb_v1_runtime_object_type_from_thing(
                        dungeon,
                        (uint16_t)scan) == object_type) {
                    same_type_present = 1;
                }
            }
            scan = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)scan);
        }
        if (sensor_type == DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT) {
            /* F0276:1666-1669 — C001 skips when another object or a group
             * occupies the square (PartySquare is a party-path input and
             * does not apply to this object bridge). */
            if (object_present || group_present) {
                thing = csb_v1_runtime_sensor_next_thing(
                    dungeon, (uint16_t)thing);
                continue;
            }
        } else if (sensor_data != object_type || same_type_present) {
            /* F0276:1691-1694 — C004 requires the sensor data to match the
             * moved object type and no same-type object already present. */
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        /* MOVESENS.C F0276 evaluates the same C004 record on both sides of
         * F0267's link mutation. Revert flips that add/remove observation;
         * only HOLD has a result on either side. */
        trigger = add_thing ? 1 : 0;
        sensor_effect = (int)((flags_word >> 3) & 0x03u);
        if ((flags_word >> 5) & 0x01u) {
            trigger ^= 1;
        }
        if (sensor_effect == DM1_EFFECT_HOLD) {
            sensor_effect = trigger ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
        } else if (!trigger) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        target_cell = (int)((target_word >> 4) & 0x03u);
        target_x = (int)((target_word >> 6) & 0x1Fu);
        target_y = (int)((target_word >> 11) & 0x1Fu);
        if ((flags_word >> 6) & 0x01u) {
            CsbV1AudioRequest request;

            memset(&request, 0, sizeof(request));
            request.soundIndex = CSB_V1_SOUND_SWITCH;
            request.mapX = (int16_t)map_x;
            request.mapY = (int16_t)map_y;
            request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
            request.volume = 64;
            request.priority = 4u;
            (void)csb_v1_audio_runtime_request(&profile->audio_runtime,
                                                &request);
        }
        if ((flags_word >> 2) & 0x01u) {
            uint8_t *mutable_sensor = csb_v1_runtime_mutable_thing_record(
                dungeon, (uint16_t)thing, NULL, NULL);
            if (mutable_sensor) {
                csb_v1_runtime_write_u16(
                    mutable_sensor + 2, (uint16_t)(type_data & 0xFF80u));
            }
        }
        if ((flags_word >> 11) & 0x01u) {
            int local_effect = (int)(target_word & 0x0FFFu);

            if (local_effect == 10) {
                csb_v1_runtime_add_party_steal_skill_experience(profile, 0);
            } else {
                pending_local_effect = local_effect;
            }
        } else {
            /* MOVESENS.C F0276 keeps C004 timing in the sensor record: the
             * packed delay schedules F0268 rather than applying a generic
             * immediate remote effect. */
            csb_v1_runtime_trigger_remote_sensor_event_after(
                profile,
                level,
                sensor_effect,
                target_x,
                target_y,
                target_cell,
                (int)((flags_word >> 7) & 0x0Fu));
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    if (pending_local_effect == DM1_EFFECT_CLEAR ||
        pending_local_effect == DM1_EFFECT_TOGGLE) {
        (void)csb_v1_runtime_rotate_wall_cell_sensors(
            dungeon,
            level,
            map_x,
            map_y,
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(placed_thing));
    }
}

static void csb_v1_runtime_process_object_wall_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t placed_thing,
    int level,
    int map_x,
    int map_y,
    int add_thing)
{
    int raw_square;
    int first_thing;
    int placed_thing_type;
    int object_type;
    int cell;
    int thing;
    int guard;
    int pending_local_effect = DM1_EFFECT_NONE;

    if (!profile || !dungeon || !dungeon->raw_data ||
        dungeon->square_bytes != 1 || profile->dungeon_handle != dungeon ||
        level < 0 || level >= dungeon->level_count) {
        return;
    }
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0 || ((raw_square >> 5) & 0x07) != DM1_SQUARE_WALL) {
        return;
    }
    if (!csb_v1_dungeon_get_thing_record(
            dungeon, placed_thing, &placed_thing_type, NULL, NULL) ||
        placed_thing_type <= 4 || placed_thing_type >= 14) {
        return;
    }
    object_type = csb_v1_runtime_object_type_from_thing(dungeon, placed_thing);
    if (object_type < 0) return;
    cell = csb_v1_teleporter_rotation_thing_cell_pc34_compat(placed_thing);
    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon, level, map_x, map_y);
    if (first_thing < 0 || first_thing == 0xFFFE || first_thing == 0xFFFF) {
        return;
    }

    /* ReDMCSB MOVESENS.C F0276 lines 1733-1793: an object moved through
     * F0267 triggers only C001..C003 wall sensors in its own packed cell.
     * The caller has already unlinked removals and linked additions; exclude
     * the moved Thing to retain F0276's pre-link occupancy observation. */
    thing = first_thing;
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int same_type_present = 0;
        int different_type_present = 0;
        int scan;
        int scan_guard;
        int trigger;
        int sensor_effect;

        record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, &thing_type, NULL, &thing_size);
        if (!record) break;
        if (thing_type >= 4) break;
        if (thing_type != 3 || thing_size < 8 ||
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                (uint16_t)thing) != cell) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        type_data = csb_v1_runtime_read_u16(record + 2);
        flags_word = csb_v1_runtime_read_u16(record + 4);
        target_word = csb_v1_runtime_read_u16(record + 6);
        sensor_type = (int)(type_data & 0x007Fu);
        if (sensor_type < DM1_SENSOR_WALL_ORNAMENT_CLICK || sensor_type > 3) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        sensor_data = (int)(type_data >> 7);
        scan = first_thing;
        for (scan_guard = 0;
             scan_guard < 128 && scan != 0xFFFE && scan != 0xFFFF;
             ++scan_guard) {
            int scan_type;

            if ((uint16_t)scan != placed_thing &&
                csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                    (uint16_t)scan) == cell &&
                csb_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)scan, &scan_type, NULL, NULL) &&
                scan_type > 4 && scan_type < 14) {
                int scan_object_type = csb_v1_runtime_object_type_from_thing(
                    dungeon, (uint16_t)scan);
                if (scan_object_type == object_type) {
                    same_type_present = 1;
                } else {
                    different_type_present = 1;
                }
            }
            scan = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)scan);
        }
        if (sensor_type == DM1_SENSOR_WALL_ORNAMENT_CLICK) {
            if (same_type_present || different_type_present) {
                thing = csb_v1_runtime_sensor_next_thing(
                    dungeon, (uint16_t)thing);
                continue;
            }
        } else if (sensor_type ==
                   DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT) {
            if (same_type_present || sensor_data != object_type) {
                thing = csb_v1_runtime_sensor_next_thing(
                    dungeon, (uint16_t)thing);
                continue;
            }
        } else if (different_type_present || sensor_data == object_type) {
            thing = csb_v1_runtime_sensor_next_thing(
                dungeon, (uint16_t)thing);
            continue;
        }
        trigger = add_thing ? 1 : 0;
        if ((flags_word >> 5) & 0x01u) trigger ^= 1;
        sensor_effect = (int)((flags_word >> 3) & 0x03u);
        if (sensor_effect == DM1_EFFECT_HOLD) {
            sensor_effect = trigger ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
        } else if (!trigger) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        if ((flags_word >> 6) & 0x01u) {
            CsbV1AudioRequest request;

            memset(&request, 0, sizeof(request));
            request.soundIndex = CSB_V1_SOUND_SWITCH;
            request.mapX = (int16_t)map_x;
            request.mapY = (int16_t)map_y;
            request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
            request.volume = 64;
            request.priority = 4u;
            (void)csb_v1_audio_runtime_request(&profile->audio_runtime,
                                                &request);
        }
        if ((flags_word >> 2) & 0x01u) {
            uint8_t *mutable_sensor = csb_v1_runtime_mutable_thing_record(
                dungeon, (uint16_t)thing, NULL, NULL);
            if (mutable_sensor) {
                csb_v1_runtime_write_u16(
                    mutable_sensor + 2, (uint16_t)(type_data & 0xFF80u));
            }
        }
        if ((flags_word >> 11) & 0x01u) {
            int local_effect = (int)(target_word & 0x0FFFu);
            if (local_effect == 10) {
                csb_v1_runtime_add_party_steal_skill_experience(profile, 0);
            } else {
                pending_local_effect = local_effect;
            }
        } else {
            csb_v1_runtime_trigger_remote_sensor_event_after(
                profile,
                level,
                sensor_effect,
                (int)((target_word >> 6) & 0x1Fu),
                (int)((target_word >> 11) & 0x1Fu),
                (int)((target_word >> 4) & 0x03u),
                (int)((flags_word >> 7) & 0x0Fu));
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    if (pending_local_effect == DM1_EFFECT_CLEAR ||
        pending_local_effect == DM1_EFFECT_TOGGLE) {
        (void)csb_v1_runtime_rotate_wall_cell_sensors(
            dungeon, level, map_x, map_y, cell);
    }
}

static int csb_v1_runtime_find_wall_cell_object_of_type(
    const CSB_V1_DungeonData *dungeon,
    int first_thing,
    int cell,
    int object_type,
    uint16_t *out_thing)
{
    int thing = first_thing;
    int guard;

    if (out_thing) *out_thing = 0xFFFFu;
    if (!dungeon || object_type < 0) return 0;
    for (guard = 0;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        int thing_type;
        const uint8_t *record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            NULL);
        if (!record) break;
        if ((thing & 3) == (cell & 3) && thing_type > 4 && thing_type < 14 &&
            csb_v1_runtime_object_type_from_thing(
                dungeon,
                (uint16_t)thing) == object_type) {
            if (out_thing) *out_thing = (uint16_t)thing;
            return 1;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    return 0;
}

static int csb_v1_runtime_find_first_square_object(
    const CSB_V1_DungeonData *dungeon,
    int first_thing,
    uint16_t *out_thing)
{
    int thing = first_thing;
    int guard;

    if (out_thing) *out_thing = 0xFFFFu;
    if (!dungeon) return 0;
    for (guard = 0;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        int thing_type;
        const uint8_t *record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            NULL);
        if (!record) break;
        if (thing_type > 4 && thing_type < 14) {
            if (out_thing) *out_thing = (uint16_t)thing;
            return 1;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    return 0;
}

static int csb_v1_runtime_has_later_wall_cell_sensor(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int cell)
{
    int guard;

    if (!dungeon || thing == 0xFFFEu || thing == 0xFFFFu) return 0;
    thing = csb_v1_runtime_sensor_next_thing(dungeon, thing);
    for (guard = 0;
         guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        const uint8_t *record = csb_v1_dungeon_get_thing_record(
            dungeon,
            thing,
            &thing_type,
            NULL,
            NULL);
        if (!record || thing_type >= 4) break;
        if (thing_type == 3 && ((thing & 3) == (uint16_t)(cell & 3))) {
            return 1;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, thing);
    }
    return 0;
}

static int csb_v1_runtime_remove_wall_sensor_after_previous(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    uint16_t previous_thing,
    uint16_t sensor_thing)
{
    uint8_t *previous_record;
    uint8_t *sensor_record;
    uint16_t next_thing;
    int previous_size;
    int sensor_type;
    int sensor_size;

    if (!dungeon || previous_thing == sensor_thing ||
        sensor_thing == 0xFFFEu || sensor_thing == 0xFFFFu) {
        return 0;
    }
    previous_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        previous_thing,
        NULL,
        &previous_size);
    sensor_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        sensor_thing,
        &sensor_type,
        &sensor_size);
    if (!previous_record || previous_size < 2 ||
        !sensor_record || sensor_size < 2 ||
        sensor_type != 3) {
        return 0;
    }
    (void)level;
    (void)map_x;
    (void)map_y;
    next_thing = csb_v1_runtime_read_u16(sensor_record);
    csb_v1_runtime_write_u16(previous_record, next_thing);
    csb_v1_runtime_write_u16(sensor_record, 0xFFFFu);
    return 1;
}

static int csb_v1_runtime_rotate_wall_cell_sensors(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    int cell)
{
    uint8_t *first_ptr;
    uint8_t *prev_first_link = NULL;
    uint8_t *first_sensor_record = NULL;
    uint8_t *last_sensor_record = NULL;
    uint16_t thing;
    uint16_t first_sensor_thing = 0xFFFFu;
    uint16_t after_first;
    uint16_t last_next;
    int guard;

    if (!dungeon || !dungeon->raw_data) return 0;
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);
    if (!first_ptr) return 0;

    thing = csb_v1_runtime_read_u16(first_ptr);
    prev_first_link = first_ptr;
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        int thing_size;
        uint8_t *record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 3 &&
            (cell < 0 || csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                             thing) == (cell & 3))) {
            first_sensor_thing = thing;
            first_sensor_record = record;
            break;
        }
        prev_first_link = record;
        thing = csb_v1_runtime_read_u16(record);
    }
    if (!first_sensor_record) return 0;

    thing = csb_v1_runtime_read_u16(first_sensor_record);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        int thing_size;
        uint8_t *record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 3 &&
            (cell < 0 || csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                             thing) == (cell & 3))) {
            last_sensor_record = record;
            break;
        }
        thing = csb_v1_runtime_read_u16(record);
    }
    if (!last_sensor_record) return 0;

    thing = csb_v1_runtime_read_u16(last_sensor_record);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        int thing_size;
        uint8_t *record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2 || thing_type != 3) break;
        if (cell < 0 || csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                            thing) == (cell & 3)) {
            last_sensor_record = record;
        }
        thing = csb_v1_runtime_read_u16(record);
    }

    /* ReDMCSB MOVESENS.C F0271 lines 1113-1138: after a local
     * C02 rotation effect, unlink the first matching sensor and append
     * it after the last matching sensor in that square/cell's sensor run. */
    after_first = csb_v1_runtime_read_u16(first_sensor_record);
    csb_v1_runtime_write_u16(prev_first_link, after_first);
    last_next = csb_v1_runtime_read_u16(last_sensor_record);
    csb_v1_runtime_write_u16(first_sensor_record, last_next);
    csb_v1_runtime_write_u16(last_sensor_record, first_sensor_thing);
    return 1;
}

static int csb_v1_runtime_trigger_wall_ornament_click_core(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    int object_type_override,
    uint16_t *leader_hand_thing)
{
    CSB_V1_DungeonData *dungeon;
    int raw_square;
    int first_thing;
    int thing;
    int previous_thing;
    int guard;
    int queued = 0;
    uint16_t hand_thing = 0xFFFFu;
    int object_type = object_type_override;

    if (!profile) return 0;
    if (!profile->dungeon_handle) return 0;
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1) return 0;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        profile->current_level,
        map_x,
        map_y);
    /* MOVESENS.C F0276:1737-1760 selects this cell-scoped C001..C003
     * path only after F0267 has supplied an actual wall square.  A loaded
     * sensor list on a corridor/floor is not interchangeable wall data. */
    if (raw_square < 0 || ((raw_square >> 5) & 0x07) != DM1_SQUARE_WALL) {
        return 0;
    }

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        profile->current_level,
        map_x,
        map_y);
    if (first_thing < 0 || first_thing == 0xFFFE || first_thing == 0xFFFF) {
        return 0;
    }
    if (leader_hand_thing) {
        hand_thing = *leader_hand_thing;
        if (hand_thing != 0xFFFEu && hand_thing != 0xFFFFu) {
            object_type = csb_v1_runtime_object_type_from_thing(
                dungeon,
                hand_thing);
        }
    }

    /* ReDMCSB: MOVESENS.C F0275/F0276 handles wall ornament clicks by
     * matching the clicked cell, resolving Revert/HOLD, applying local
     * storage/rotation side effects, then calling F0272_SENSOR_TriggerEffect. */
    previous_thing = first_thing;
    thing = first_thing;
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        uint8_t *record;
        int thing_type;
        int thing_size;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int sensor_cell;
        int square_has_object = 0;
        int same_type_present = 0;
        int different_type_present = 0;
        int trigger = 1;
        int sensor_effect;
        int target_cell;
        int target_x;
        int target_y;
        int scan;
        int scan_guard;
        uint16_t storage_thing = 0xFFFFu;
        uint16_t square_thing = 0xFFFFu;
        uint16_t generated_thing = 0xFFFFu;
        int storage_action = 0;
        int do_not_trigger = 0;
        int rotate_after = 0;
        int remove_current_sensor = 0;

        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!record) break;
        if (thing_type >= 4) break;
        if (thing_type != 3 || thing_size < 8) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        sensor_cell = thing & 3;
        if (sensor_cell != (cell & 3)) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        scan = first_thing;
        for (scan_guard = 0;
             scan_guard < 128 && scan != 0xFFFE && scan != 0xFFFF;
             ++scan_guard) {
            int scan_type;
            int scan_object_type;
            const uint8_t *scan_record = csb_v1_dungeon_get_thing_record(
                dungeon,
                (uint16_t)scan,
                &scan_type,
                NULL,
                NULL);
            (void)scan_record;
            if (!scan_record) break;
            if ((scan & 3) == (cell & 3) && scan_type > 4 && scan_type < 14) {
                square_has_object = 1;
                scan_object_type = csb_v1_runtime_object_type_from_thing(
                    dungeon,
                    (uint16_t)scan);
                if (scan_object_type == object_type) {
                    same_type_present = 1;
                } else {
                    different_type_present = 1;
                }
            }
            scan = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)scan);
        }

        type_data = csb_v1_runtime_read_u16(record + 2);
        flags_word = csb_v1_runtime_read_u16(record + 4);
        target_word = csb_v1_runtime_read_u16(record + 6);
        sensor_type = (int)(type_data & 0x007Fu);
        sensor_data = (int)(type_data >> 7);
        switch (sensor_type) {
        case 1: /* C001_SENSOR_WALL_ORNAMENT_CLICK */
            trigger = (object_type < 0 && !square_has_object) ? 1 : 0;
            break;
        case 2: /* C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT */
            trigger = (object_type >= 0 &&
                       !same_type_present &&
                       sensor_data == object_type) ? 1 : 0;
            break;
        case 3: /* C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT */
            trigger = (object_type >= 0 &&
                       !different_type_present &&
                       sensor_data != object_type) ? 1 : 0;
            break;
        case DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE:
        case DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR:
            if (!leader_hand_thing ||
                csb_v1_runtime_has_later_wall_cell_sensor(
                    dungeon,
                    (uint16_t)thing,
                    cell)) {
                trigger = 0;
                break;
            }
            trigger = (object_type >= 0 && sensor_data == object_type) ? 1 : 0;
            if (sensor_type == DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE) {
                rotate_after = 1;
            } else if (sensor_type ==
                       DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR) {
                remove_current_sensor = 1;
            }
            storage_action = 3;
            break;
        case DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED:
            /* MOVESENS.C F0275 C004 consumes a matching hand object.
             * C011/C017 alone require the final same-cell sensor. */
            if (!leader_hand_thing) {
                trigger = 0;
                break;
            }
            trigger = (object_type >= 0 && sensor_data == object_type) ? 1 : 0;
            storage_action = 3;
            break;
        case DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE:
            if (!leader_hand_thing ||
                csb_v1_runtime_has_later_wall_cell_sensor(
                    dungeon,
                    (uint16_t)thing,
                    cell)) {
                trigger = 0;
                break;
            }
            trigger = (object_type < 0) ? 1 : 0;
            if (trigger) rotate_after = 1;
            storage_action = 4;
            break;
        case DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE:
            if (!leader_hand_thing) {
                trigger = 0;
                break;
            }
            if (object_type < 0) {
                if (!csb_v1_runtime_find_wall_cell_object_of_type(
                        dungeon,
                        first_thing,
                        cell,
                        sensor_data,
                        &storage_thing)) {
                    trigger = 0;
                    break;
                }
                storage_action = 1;
            } else {
                if (object_type != sensor_data ||
                    csb_v1_runtime_find_wall_cell_object_of_type(
                        dungeon,
                        first_thing,
                        cell,
                        sensor_data,
                        NULL)) {
                    trigger = 0;
                    break;
                }
                storage_action = 2;
                do_not_trigger = (int)(((flags_word >> 3) & 0x03u) ==
                                       DM1_EFFECT_HOLD);
            }
            trigger = 1;
            break;
        case DM1_SENSOR_WALL_OBJECT_EXCHANGER:
            if (!leader_hand_thing ||
                csb_v1_runtime_has_later_wall_cell_sensor(
                    dungeon,
                    (uint16_t)thing,
                    cell)) {
                trigger = 0;
                break;
            }
            if (object_type != sensor_data ||
                !csb_v1_runtime_find_first_square_object(
                    dungeon,
                    first_thing,
                    &square_thing)) {
                trigger = 0;
                break;
            }
            storage_action = 5;
            trigger = 1;
            break;
        default:
            trigger = 0;
            break;
        }

        if ((flags_word >> 5) & 0x01u) {
            trigger ^= 1;
        }
        sensor_effect = (int)((flags_word >> 3) & 0x03u);
        if (sensor_effect == DM1_EFFECT_HOLD) {
            sensor_effect = do_not_trigger ? DM1_EFFECT_CLEAR : DM1_EFFECT_SET;
        } else if (!trigger || do_not_trigger) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        if (!trigger) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        if (storage_action == 1) {
            if (!csb_v1_runtime_unlink_thing_from_square(
                    dungeon,
                    storage_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            *leader_hand_thing = storage_thing;
            object_type = sensor_data;
        } else if (storage_action == 2) {
            uint16_t stored_thing = csb_v1_runtime_thing_with_cell(
                (hand_thing >> 10) & 0x0F,
                hand_thing & 0x03FFu,
                cell);
            if (!csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    stored_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            *leader_hand_thing = 0xFFFFu;
            object_type = -1;
        } else if (storage_action == 3) {
            uint8_t *hand_record = csb_v1_runtime_mutable_thing_record(
                dungeon,
                hand_thing,
                NULL,
                NULL);
            if (!hand_record) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            csb_v1_runtime_write_u16(hand_record, 0xFFFFu);
            *leader_hand_thing = 0xFFFFu;
            object_type = -1;
        } else if (storage_action == 4) {
            generated_thing = csb_v1_runtime_allocate_new_object_launcher_thing(
                dungeon,
                sensor_data);
            /* ReDMCSB DUNGEON.C F0167 is the materialization boundary for
             * C012.  No object means no completed sensor action: do not
             * rotate the source cell or publish F0272/F0268 for a launcher
             * that could not allocate a real dungeon record. */
            if (generated_thing == 0xFFFFu || generated_thing == 0xFFFEu) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(
                    dungeon, (uint16_t)thing);
                continue;
            }
            *leader_hand_thing = generated_thing;
            object_type = csb_v1_runtime_object_type_from_thing(
                dungeon,
                generated_thing);
        } else if (storage_action == 5) {
            uint16_t stored_thing;
            if (!csb_v1_runtime_unlink_thing_from_square(
                    dungeon,
                    square_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            stored_thing = csb_v1_runtime_thing_with_cell(
                (hand_thing >> 10) & 0x0F,
                hand_thing & 0x03FFu,
                cell);
            if (!csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    stored_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                (void)csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    square_thing,
                    profile->current_level,
                    map_x,
                    map_y);
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            *leader_hand_thing = square_thing;
            object_type = csb_v1_runtime_object_type_from_thing(
                dungeon,
                square_thing);
        }
        if (storage_action == 1 || storage_action == 2 || rotate_after) {
            (void)csb_v1_runtime_rotate_wall_cell_sensors(
                dungeon,
                profile->current_level,
                map_x,
                map_y,
                cell);
        }
        if (remove_current_sensor) {
            (void)csb_v1_runtime_remove_wall_sensor_after_previous(
                dungeon,
                profile->current_level,
                map_x,
                map_y,
                (uint16_t)previous_thing,
                (uint16_t)thing);
        }

        target_cell = (int)((target_word >> 4) & 0x03u);
        target_x = (int)((target_word >> 6) & 0x1Fu);
        target_y = (int)((target_word >> 11) & 0x1Fu);
        queued += csb_v1_runtime_queue_remote_square_event(
            profile,
            sensor_effect,
            target_x,
            target_y,
            target_cell);
        break;
    }

    return queued;
}

int csb_v1_runtime_trigger_wall_ornament_click(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    int object_type)
{
    return csb_v1_runtime_trigger_wall_ornament_click_core(
        profile,
        map_x,
        map_y,
        cell,
        object_type,
        NULL);
}

int csb_v1_runtime_trigger_wall_ornament_click_ex(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    uint16_t *leader_hand_thing)
{
    return csb_v1_runtime_trigger_wall_ornament_click_core(
        profile,
        map_x,
        map_y,
        cell,
        -1,
        leader_hand_thing);
}

int csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell)
{
    uint16_t leader_hand;
    int queued;

    if (!profile || !profile->party_state_valid) return 0;
    leader_hand = csb_v1_runtime_normalize_leader_hand_thing(
        profile->party_state.LeaderHandThing);
    queued = csb_v1_runtime_trigger_wall_ornament_click_core(
        profile,
        map_x,
        map_y,
        cell,
        -1,
        &leader_hand);
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(leader_hand);
    if (profile->csbwin_gameblock2_summary_valid) {
        profile->csbwin_object_in_hand = profile->party_state.LeaderHandThing;
    }
    return queued;
}

static int csb_v1_runtime_scan_thing_chain_for_object_type(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int object_type)
{
    int guard;

    if (!dungeon || object_type < 0) return 0;
    for (guard = 0;
         guard < DM1_SENSOR_POSSESSION_MAX_SCAN_STEPS &&
             thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type > 4 && thing_type < 14 &&
            csb_v1_runtime_object_type_from_thing(dungeon, thing) ==
                object_type) {
            return 1;
        }
        thing = csb_v1_runtime_read_u16(record);
    }
    return 0;
}

static int csb_v1_runtime_object_info_index_from_record(
    int thing_type,
    const uint8_t *record,
    int record_size)
{
    uint16_t word;
    int subtype;

    if (!record || record_size < 4) return -1;
    word = csb_v1_runtime_read_u16(record + 2);
    switch (thing_type) {
    case THING_TYPE_SCROLL:
        return 0;
    case THING_TYPE_CONTAINER:
        if (record_size < 8) return -1;
        /* ReDMCSB DEFS.H CONTAINER stores Next at +0, Slot at +2, and Type
         * at +4.  Do not derive the chest object-info subtype from Slot; a
         * non-empty chest would then change icon/action metadata based on
         * its first contained thing. */
        word = csb_v1_runtime_read_u16(record + 4);
        subtype = (int)((word >> 1) & 0x03u);
        if (subtype > 0) subtype = 0;
        return 1 + subtype;
    case THING_TYPE_POTION:
        subtype = (int)((word >> 8) & 0x7Fu);
        if (subtype > 20) return -1;
        return 2 + subtype;
    case THING_TYPE_WEAPON:
        subtype = (int)(word & 0x7Fu);
        if (subtype > 45) return -1;
        return 23 + subtype;
    case THING_TYPE_ARMOUR:
        subtype = (int)(word & 0x7Fu);
        if (subtype > 57) return -1;
        return 69 + subtype;
    case THING_TYPE_JUNK:
        subtype = (int)(word & 0x7Fu);
        if (subtype > 52) return -1;
        return 127 + subtype;
    default:
        return -1;
    }
}

int csb_v1_runtime_read_container_slots(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t container_thing,
    uint16_t out_slots[8])
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t thing;
    int thing_type;
    int record_size;
    int count = 0;
    int guard = 0;
    int i;

    if (!out_slots) return -1;
    for (i = 0; i < 8; ++i) out_slots[i] = THING_NONE;
    if (!profile ||
        container_thing == THING_NONE ||
        container_thing == THING_ENDOFLIST ||
        THING_GET_TYPE(container_thing) != THING_TYPE_CONTAINER) {
        return -1;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        container_thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record || thing_type != THING_TYPE_CONTAINER || record_size < 8) {
        return -1;
    }

    /* ReDMCSB CHEST.C F0333 lines 58-75 reads CONTAINER.Slot, follows
     * F0159_DUNGEON_GetNextThing, and materializes at most C537..C544. */
    thing = csb_v1_runtime_read_u16(record + 2);
    while (thing != THING_NONE &&
           thing != THING_ENDOFLIST &&
           count < 8 &&
           guard++ < DM1_SENSOR_POSSESSION_MAX_SCAN_STEPS) {
        const uint8_t *thing_record = csb_v1_dungeon_get_thing_record(
            dungeon,
            thing,
            NULL,
            NULL,
            &record_size);
        if (!thing_record || record_size < 2) break;
        out_slots[count++] = thing;
        thing = csb_v1_runtime_read_u16(thing_record);
    }
    return count;
}

int csb_v1_runtime_set_thing_next(
    CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    uint16_t next_thing)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *record;
    int record_size;

    if (!profile ||
        thing == THING_NONE ||
        thing == THING_ENDOFLIST) {
        return 0;
    }
    dungeon = (CSB_V1_DungeonData *)(profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current());
    record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        thing,
        NULL,
        &record_size);
    if (!record || record_size < 2) return 0;
    csb_v1_runtime_write_u16(record, next_thing);
    return 1;
}

uint16_t csb_v1_runtime_next_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing)
{
    const uint8_t *record;
    int thing_type = -1;
    int record_size = 0;

    if (!dungeon || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record || record_size < 2 || thing_type < 0) {
        return THING_ENDOFLIST;
    }
    /* ReDMCSB DUNGEON.C F0159/F0160 use the first word of every thing
     * record as the compact Next link. */
    return csb_v1_runtime_read_u16(record);
}

int csb_v1_runtime_thing_type_is_floor_object(int thing_type)
{
    return thing_type == THING_TYPE_WEAPON ||
           thing_type == THING_TYPE_ARMOUR ||
           thing_type == THING_TYPE_SCROLL ||
           thing_type == THING_TYPE_POTION ||
           thing_type == THING_TYPE_CONTAINER ||
           thing_type == THING_TYPE_JUNK;
}

int csb_v1_runtime_object_overlay_info(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t object_thing,
    CSB_V1_RuntimeObjectOverlayInfo *out_info)
{
    int thing_type;

    if (!profile || !out_info ||
        object_thing == THING_NONE ||
        object_thing == THING_ENDOFLIST) {
        return 0;
    }
    thing_type = (int)THING_GET_TYPE(object_thing);
    if (!csb_v1_runtime_thing_type_is_floor_object(thing_type)) {
        return 0;
    }

    memset(out_info, 0, sizeof(*out_info));
    /* ReDMCSB DEFS.H THING packs Cell/Type/Index in the thing word, while
     * WEAPON..JUNK records carry the object subtype data used by F0115 item
     * rendering.  Keep that compact routing in CSB runtime instead of M11. */
    out_info->thing_type = thing_type;
    out_info->relative_cell =
        ((int)THING_GET_CELL(object_thing) - profile->party_dir) & 3;
    out_info->icon_index =
        csb_v1_runtime_object_icon_index(profile, object_thing);
    out_info->subtype_index =
        csb_v1_runtime_object_subtype_index(profile, object_thing);
    return 1;
}

int csb_v1_runtime_group_record_creature_type(
    const uint8_t *record,
    int size)
{
    if (!record || size < 16) {
        return -1;
    }
    /* ReDMCSB DEFS.H GROUP stores the creature type byte before Cells.
     * Runtime overlay drawing uses it only after the C04 record has been
     * validated by the CSB dungeon accessor. */
    return (int)record[4];
}

int csb_v1_runtime_group_record_direction(
    const uint8_t *record,
    int size)
{
    uint16_t flags;

    if (!record || size < 16) {
        return 0;
    }
    /* ReDMCSB GROUP.C stores the shared primary group direction in
     * GROUP.Dir bits 8..9 for non-active C04 records. */
    flags = csb_v1_runtime_read_u16(record + 14);
    return (int)((flags >> 8) & 0x03u);
}

int csb_v1_runtime_group_record_visible_count(
    const uint8_t *record,
    int size)
{
    uint16_t flags;
    int count;

    if (!record || size < 16) {
        return 1;
    }
    if (record[5] == 0xFFu) {
        return 1;
    }
    /* ReDMCSB DEFS.H GROUP.Count stores count - 1 in bits 5..6 of the
     * PC34/I34E GROUP flag word. */
    flags = csb_v1_runtime_read_u16(record + 14);
    count = (int)((flags >> 5) & 0x03u) + 1;
    if (count < 1) count = 1;
    if (count > 4) count = 4;
    return count;
}

int csb_v1_runtime_group_record_creature_cell(
    const uint8_t *record,
    int size,
    int creature_index)
{
    if (!record || size < 16 || creature_index < 0) {
        return 0;
    }
    if (record[5] == 0xFFu) {
        return 0;
    }
    if (creature_index > 3) {
        creature_index = 3;
    }
    /* ReDMCSB DEFS.H M050_CREATURE_VALUE reads packed GROUP.Cells:
     * two bits per creature.  F0115 uses this as the C3200 view cell. */
    return ((int)record[5] >> (creature_index << 1)) & 0x03;
}

int csb_v1_runtime_group_overlay_info(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    CSB_V1_RuntimeGroupOverlayInfo *out_info)
{
    const uint8_t *record;
    int thing_type = -1;
    int record_size = 0;
    int i;

    if (!dungeon || !out_info ||
        group_thing == THING_NONE ||
        group_thing == THING_ENDOFLIST ||
        THING_GET_TYPE(group_thing) != THING_TYPE_GROUP) {
        return 0;
    }
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        group_thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record || record_size < 16 || thing_type != THING_TYPE_GROUP) {
        return 0;
    }

    memset(out_info, 0, sizeof(*out_info));
    /* ReDMCSB DEFS.H GROUP: C04 records carry Type, Cells, Count, and
     * Direction in the compact record consumed by DUNVIEW.C F0115 when the
     * thing chain reaches a creature group. */
    out_info->creature_type =
        csb_v1_runtime_group_record_creature_type(record, record_size);
    out_info->direction =
        csb_v1_runtime_group_record_direction(record, record_size);
    out_info->visible_count =
        csb_v1_runtime_group_record_visible_count(record, record_size);
    if (out_info->visible_count < 1) out_info->visible_count = 1;
    if (out_info->visible_count > 4) out_info->visible_count = 4;
    for (i = 0; i < out_info->visible_count; ++i) {
        out_info->cells[i] =
            csb_v1_runtime_group_record_creature_cell(record,
                                                      record_size,
                                                      i);
    }
    return 1;
}

int csb_v1_runtime_write_container_slots(
    CSB_V1_RuntimeProfile *profile,
    uint16_t container_thing,
    const uint16_t slots[8])
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *container_record;
    uint8_t *records[8];
    uint16_t things[8];
    int thing_type;
    int record_size;
    int count = 0;
    int i;

    if (!profile ||
        !slots ||
        container_thing == THING_NONE ||
        container_thing == THING_ENDOFLIST ||
        THING_GET_TYPE(container_thing) != THING_TYPE_CONTAINER) {
        return 0;
    }
    dungeon = (CSB_V1_DungeonData *)(profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current());
    container_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        container_thing,
        &thing_type,
        &record_size);
    if (!container_record ||
        thing_type != THING_TYPE_CONTAINER ||
        record_size < 8) {
        return 0;
    }

    for (i = 0; i < 8; ++i) {
        uint16_t thing = slots[i];
        uint8_t *record;
        if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            NULL,
            &record_size);
        if (!record || record_size < 2) return 0;
        things[count] = thing;
        records[count] = record;
        ++count;
    }

    /* ReDMCSB CHEST.C F0334 lines 112-133 rewrites CONTAINER.Slot from
     * G0425_aT_ChestSlots only and compacts visible non-empty slots into a
     * fresh Next chain, truncating any hidden tail beyond slot 8. */
    csb_v1_runtime_write_u16(container_record + 2, THING_ENDOFLIST);
    for (i = 0; i < count; ++i) {
        uint16_t next = (i + 1 < count) ? things[i + 1] : THING_ENDOFLIST;
        csb_v1_runtime_write_u16(records[i], next);
        if (i == 0) {
            csb_v1_runtime_write_u16(container_record + 2, things[i]);
        }
    }
    return 1;
}

typedef struct {
    unsigned char weight;
    unsigned char weapon_class;
    unsigned char strength;
    unsigned char kinetic_energy;
    unsigned char shoot_attack;
} CSB_V1_RuntimeWeaponInfoPc34;

/* ReDMCSB DUNGEON.C G0238_as_Graphic559_WeaponInfo.  CSB runtime uses the
 * source fields needed by CHAMPION.C F0312/F0328 and MENU.C F0407. */
static const CSB_V1_RuntimeWeaponInfoPc34
    g_csb_v1_weapon_info_pc34[46] = {
    {  1, 130,   2,   0,   0}, {  1, 131,   2,   0,   0},
    { 11,   0,   8,   2,   0}, { 12, 112,  10,  80,  40},
    {  9, 129,  16,   7,   0}, { 30, 113,  49, 110,  66},
    { 47,   0,  55,  20,   0}, { 24, 255,  25,  10, 255},
    {  5,   2,  10,  19,   0}, { 33,   0,  30,   8,   0},
    { 32,   0,  34,  10,   0}, { 26,   0,  38,  10,   0},
    { 35,   0,  42,  11,   0}, { 36,   0,  46,  12,   0},
    { 33,   0,  50,  14,   0}, { 37,   0,  62,  14,   0},
    { 30,   0,  48,  13,   0}, { 39,   0,  58,  15,   0},
    { 43,   2,  49,  33,   0}, { 65,   2,  70,  44,   0},
    { 31,   0,  32,  10,   0}, { 41,   0,  42,  13,   0},
    { 50,   0,  60,  15,   0}, { 36,   0,  19,  10,   0},
    {110,   0,  44,  22,   0}, { 10,  20,   1,  50,  50},
    { 28,  30,   1, 180, 120}, {  2,  10,   2,  10,   0},
    {  2,  10,   2,  28,   0}, { 19,  39,   5,  20,  50},
    { 10,  11,   6,  18,   0}, {  3,  12,   7,  23,   0},
    {  1,   1,   3,  19,   0}, {  8,   0,   4,   4,   0},
    { 26, 129,  12,   4,   0}, {  1, 130,   0,   0,   0},
    {  2, 140,   1,  20,   0}, { 35, 128,  18,   6,   0},
    { 29, 159,   0,   4,   0}, { 21, 131,   0,   3,   0},
    { 33, 136,   0,   7,   0}, {  8, 132,   3,   1,   0},
    { 18, 131,   9,   4,   0}, {  8, 192,   1,   1,   0},
    { 30,  26,   1, 220, 125}, { 36, 255, 100,  50, 255}
};

static int csb_v1_runtime_weapon_info_for_thing(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    CSB_V1_RuntimeWeaponInfoPc34 *out)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word;
    int thing_type;
    int record_size;
    int weapon_type;

    if (out) memset(out, 0, sizeof(*out));
    if (!profile || !out ||
        thing == THING_NONE ||
        thing == THING_ENDOFLIST) {
        return 0;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record ||
        record_size < 4 ||
        thing_type != THING_TYPE_WEAPON) {
        return 0;
    }
    word = csb_v1_runtime_read_u16(record + 2);
    weapon_type = (int)(word & 0x7Fu);
    if (weapon_type < 0 ||
        weapon_type >= (int)(sizeof(g_csb_v1_weapon_info_pc34) /
                             sizeof(g_csb_v1_weapon_info_pc34[0]))) {
        return 0;
    }
    *out = g_csb_v1_weapon_info_pc34[weapon_type];
    return 1;
}

static int csb_v1_runtime_shoot_step_energy(
    int action_class,
    int *out_step_energy)
{
    if (!out_step_energy) return 0;
    if (action_class >= 16 && action_class <= 31) {
        *out_step_energy = action_class - 16;
        return 1;
    }
    if (action_class >= 32 && action_class <= 47) {
        *out_step_energy = action_class - 32;
        return 1;
    }
    return 0;
}

static int csb_v1_runtime_shoot_ammunition_matches(
    const CSB_V1_RuntimeWeaponInfoPc34 *action_info,
    const CSB_V1_RuntimeWeaponInfoPc34 *ready_info)
{
    int action_class;
    int ready_class;

    if (!action_info || !ready_info) return 0;
    action_class = (int)action_info->weapon_class;
    ready_class = (int)ready_info->weapon_class;
    if (action_class >= 16 && action_class <= 31) {
        return ready_class == 10;
    }
    if (action_class >= 32 && action_class <= 47) {
        return ready_class == 11;
    }
    return 0;
}

static int csb_v1_runtime_stamina_adjusted_value(
    const CSB_V1_Champion *champion,
    int value)
{
    int half_max;
    int half_value;

    if (!champion) return 0;
    half_max = (int)champion->MaximumStamina >> 1;
    if (half_max > 0 && (int)champion->CurrentStamina < half_max) {
        half_value = value >> 1;
        return half_value +
               (int)(((long)half_value * (long)champion->CurrentStamina) /
                     (long)half_max);
    }
    return value;
}

static int csb_v1_runtime_get_object_weight_internal_pc34_compat(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    int depth)
{
    static const unsigned char kArmourWeights[58] = {
          3,   4,   3,   6,  16,   4,   4,   3,   3,   4,
          2,   4,   5,   3,   3,   4,   6,   8,  14,   6,
          5,   5,   5,   4,   6,  11,  14,  15,  11,  10,
         14,  21,  65,  53,  52,  41,  16,  16,  19, 120,
         80,  28,  34,  17, 108,  72,  24,  30,  35, 141,
         90,  31,  40,  14,  57,  81,   3,   2
    };
    static const unsigned char kJunkWeights[53] = {
          1,   3,   2,   2,   4,  15,   1,   1,   1,   2,
          1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
          1,   1,   1,   1,   1,  81,   2,   3,   2,   4,
          4,   3,   8,   5,  11,   4,   6,   2,   3,   2,
          2,   2,   6,   9,   3,  10,   1,   0,   1,   1,
          2,   0,   8
    };
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word;
    int thing_type;
    int record_size;
    int subtype;

    if (!profile ||
        thing == THING_NONE ||
        thing == THING_ENDOFLIST ||
        depth > DM1_SENSOR_POSSESSION_MAX_SCAN_STEPS) {
        return 0;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record || record_size < 4) return 0;
    word = csb_v1_runtime_read_u16(record + 2);

    switch (thing_type) {
    case THING_TYPE_SCROLL:
        return 1;
    case THING_TYPE_POTION:
        subtype = (int)((word >> 8) & 0x7Fu);
        return (subtype == 20) ? 1 : 3;
    case THING_TYPE_WEAPON: {
        CSB_V1_RuntimeWeaponInfoPc34 info;
        return csb_v1_runtime_weapon_info_for_thing(profile, thing, &info)
            ? (int)info.weight
            : 0;
    }
    case THING_TYPE_ARMOUR:
        subtype = (int)(word & 0x7Fu);
        return (subtype >= 0 &&
                subtype < (int)(sizeof(kArmourWeights) /
                                sizeof(kArmourWeights[0])))
            ? (int)kArmourWeights[subtype]
            : 0;
    case THING_TYPE_JUNK:
        subtype = (int)(word & 0x7Fu);
        if (subtype < 0 ||
            subtype >= (int)(sizeof(kJunkWeights) / sizeof(kJunkWeights[0]))) {
            return 0;
        }
        if (subtype == 1) {
            return (int)kJunkWeights[subtype] +
                   (((int)((word >> 14) & 0x03u)) << 1);
        }
        return (int)kJunkWeights[subtype];
    case THING_TYPE_CONTAINER: {
        uint16_t child;
        int total;
        int guard = 0;

        /* CSBWin Mouse.cpp::GetObjectWeight obtains the chest base from the
         * save-owned EDBT_ObjectWeights record before adding live contents.
         * A malformed saved EXPOOL receipt must not silently fall back to 50. */
        if (csb_v1_runtime_csbwin_chest_weight_from_expool(
                profile, &total) != 0) {
            return 0;
        }
        if (record_size < 8) return total;
        /* ReDMCSB DEFS.H CONTAINER stores contained-object head in Slot at
         * +2; +4 is the container Type word.  F0328/F0312 object-weight
         * arithmetic must include live chest contents, not reinterpret Type
         * bits as a child THING. */
        child = csb_v1_runtime_read_u16(record + 2);
        while (child != THING_NONE &&
               child != THING_ENDOFLIST &&
               guard++ < DM1_SENSOR_POSSESSION_MAX_SCAN_STEPS) {
            const uint8_t *child_record;
            total += csb_v1_runtime_get_object_weight_internal_pc34_compat(
                profile, child, depth + 1);
            child_record = csb_v1_dungeon_get_thing_record(
                dungeon,
                child,
                NULL,
                NULL,
                &record_size);
            if (!child_record || record_size < 2) break;
            child = csb_v1_runtime_read_u16(child_record);
        }
        return total;
    }
    default:
        return 0;
    }
}

/* CSBWin Mouse.cpp:52-75 obtains the chest base through
 * EXPOOL::Locate((EDT_Database << 24) | (EDBT_ObjectWeights << 16)).
 * The record is optional (default 50) but, once a CSBWin tail is present,
 * it is usable only while its preserved bytes still match the FNV receipt. */
static int csb_v1_runtime_csbwin_chest_weight_from_expool(
    const CSB_V1_RuntimeProfile *profile,
    int *out_weight)
{
    const uint32_t object_weights_record = 5u << 24;
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;
    uint32_t weight;

    if (!profile || !out_weight) return -1;
    *out_weight = 50;

    if (!profile->csbwin_appended_tail_valid ||
        profile->csbwin_appended_tail_size == 0u) {
        return 0;
    }
    if (profile->csbwin_appended_tail_truncated ||
        profile->csbwin_appended_tail_size !=
            profile->csbwin_appended_tail_preserved_size ||
        profile->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        profile->csbwin_appended_tail_fnv1a != csb_v1_runtime_fnv1a32(
            profile->csbwin_appended_tail,
            profile->csbwin_appended_tail_preserved_size)) {
        return -1;
    }
    if (!csb_v1_runtime_locate_appended_expool_record_internal(
            profile, object_weights_record, &payload, &payload_size)) {
        return 0;
    }
    if (!payload || payload_size < sizeof(uint32_t)) return -1;
    weight = csb_v1_runtime_read_le32(payload);
    if (weight > 0x7fffffffu) return -1;
    *out_weight = (int)weight;
    return 0;
}

int csb_v1_runtime_get_object_weight_pc34_compat(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    return csb_v1_runtime_get_object_weight_internal_pc34_compat(
        profile, thing, 0);
}

int csb_v1_runtime_recompute_champion_load_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    int champion_index)
{
    CSB_V1_Champion *champion;
    int slot;
    int total = 0;

    if (!profile || !profile->dungeon_handle ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount) {
        return -1;
    }

    champion = &profile->party_state.Champions[champion_index];
    /* ReDMCSB CHAMPION.C maintains Load incrementally as F0297/F0298 and
     * F0300/F0301 alter slots.  This load/import boundary rebuilds the same
     * cached field by summing each C00..C29 Thing through DUNGEON.C F0140. */
    for (slot = 0; slot < CSB_V1_SLOT_COUNT; ++slot) {
        total += csb_v1_runtime_get_object_weight_pc34_compat(
            profile, champion->Slots[slot]);
    }
    champion->Load = csb_v1_runtime_clamp_u16(total);
    return total;
}

int csb_v1_runtime_recompute_party_loads_pc34_compat(
    CSB_V1_RuntimeProfile *profile)
{
    int champion_index;
    int recomputed = 0;

    if (!profile || !profile->dungeon_handle) return -1;
    for (champion_index = 0;
         champion_index < profile->party_state.ChampionCount;
         ++champion_index) {
        if (csb_v1_runtime_recompute_champion_load_pc34_compat(
                profile, champion_index) >= 0) {
            ++recomputed;
        }
    }
    return recomputed;
}

static int csb_v1_runtime_f0312_action_hand_strength(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    const CSB_V1_Champion *champion,
    uint16_t thing,
    const CSB_V1_RuntimeWeaponInfoPc34 *weapon_info,
    struct RngState_Compat *rng)
{
    int strength;
    int object_weight;
    int max_load;
    int one_sixteenth_maximum_load;
    int load_threshold;
    int weapon_class;
    int skill_bonus = 0;

    if (!profile || !champion || !rng) return 0;
    strength = F0732_COMBAT_RngRandom_Compat(rng, 16) +
               (int)champion->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR];
    object_weight = csb_v1_runtime_get_object_weight_pc34_compat(profile, thing);
    max_load = (int)csb_v1_champion_get_maximum_load(champion);
    if (max_load <= 0) {
        max_load =
            ((int)champion->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] << 3) +
            100;
    }
    one_sixteenth_maximum_load = max_load >> 4;
    if (object_weight <= one_sixteenth_maximum_load) {
        strength += object_weight - 12;
    } else {
        load_threshold =
            one_sixteenth_maximum_load +
            ((one_sixteenth_maximum_load - 12) >> 1);
        if (object_weight <= load_threshold) {
            strength += (object_weight - one_sixteenth_maximum_load) >> 1;
        } else {
            strength -= (object_weight - load_threshold) << 1;
        }
    }

    if (weapon_info && THING_GET_TYPE(thing) == THING_TYPE_WEAPON) {
        strength += (int)weapon_info->strength;
        weapon_class = (int)weapon_info->weapon_class;
        if (weapon_class == 0 || weapon_class == 2) {
            skill_bonus += csb_v1_runtime_get_champion_skill_level(
                profile, champion_index, 4);
        }
        if (weapon_class != 0 && weapon_class < 16) {
            skill_bonus += csb_v1_runtime_get_champion_skill_level(
                profile, champion_index, 10);
        }
        if (weapon_class >= 16 && weapon_class < 112) {
            skill_bonus += csb_v1_runtime_get_champion_skill_level(
                profile, champion_index, 11);
        }
        if (skill_bonus < 0) skill_bonus = 0;
        strength += skill_bonus << 1;
    }

    strength = csb_v1_runtime_stamina_adjusted_value(champion, strength);
    if ((champion->Wounds & COMBAT_WOUND_ACTION_HAND) != 0) {
        strength >>= 1;
    }
    strength >>= 1;
    if (strength < 0) return 0;
    if (strength > 100) return 100;
    return strength;
}

static int csb_v1_runtime_potion_projectile_subtype(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    int *out_potion_power)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word;
    int thing_type;
    int record_size;
    int potion_type;

    if (out_potion_power) *out_potion_power = 0;
    if (!profile || THING_GET_TYPE(thing) != THING_TYPE_POTION) return -1;
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record || record_size < 4 || thing_type != THING_TYPE_POTION) {
        return -1;
    }
    word = csb_v1_runtime_read_u16(record + 2);
    potion_type = (int)((word >> 8) & 0x7Fu);
    if (out_potion_power) *out_potion_power = (int)(word & 0xFFu);
    if (potion_type == 3) return PROJECTILE_SUBTYPE_POISON_CLOUD;
    if (potion_type == 19) return PROJECTILE_SUBTYPE_FIREBALL;
    return -1;
}

static int csb_v1_runtime_object_icon_from_object_info(
    int object_info_index)
{
    static const unsigned char kObjectInfoIcon[180] = {
         30,144,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,
        166,167,195, 16, 18,  4, 14, 20, 23, 25, 27, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
         61, 62, 63, 64, 65, 66,135,143, 28, 80, 81, 82,112,114, 67, 83, 68, 84, 69, 70,
         85, 86, 71, 87,119, 72, 88,113, 89, 73, 74, 90,103,104, 96, 97, 98,105,106,108,
        107, 75, 91, 76, 92, 99,115,100, 77, 93,116,109,101, 78, 94,117,110,102, 79, 95,
        118,111,140,141,142,194,196,  0,  8, 10, 12,146,147,125,126,127,176,177,178,179,
        180,181,182,183,184,185,186,187,188,189,190,191,128,129,130,131,168,169,170,171,
        172,173,174,175,120,121,122,123,124,132,133,134,136,137,138,139,192,193,197,198
    };
    if (object_info_index < 0 || object_info_index >= 180) return -1;
    return (int)kObjectInfoIcon[object_info_index];
}

static int csb_v1_runtime_object_action_set_from_object_info(
    int object_info_index)
{
    static const unsigned char kPotionActionSet[20] = {
         0, 0, 0, 42, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 42
    };
    static const unsigned char kWeaponActionSet[46] = {
        43,  7,  5,  6,  8,  9, 10, 11, 12, 13,
        13, 14, 15, 15, 16, 17, 18, 19, 20, 21,
        22, 22, 23, 24, 24, 27, 27, 26, 26, 27,
        42, 40, 42,  5,  5, 28, 29, 30, 31, 32,
        33,  5, 35, 36, 27,  1
    };
    static const unsigned char kArmourActionSet[58] = {
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0, 41, 41,
        41, 41,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 41,  0,  0,  0,  0, 41,  0,  0,
         0,  0, 41,  0,  0,  0,  0,  0
    };
    static const unsigned char kJunkActionSet[53] = {
         0,  0,  0,  0,  0,  0, 37, 37, 37,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 38, 38,  0, 39,  0,  0,  0,  0,
         0,  0,  0
    };

    /* ReDMCSB: DUNGLOB.C G0237 supplies ObjectInfo.ActionSetIndex;
     * MENU.C F0386/F0389 consumes it for action icons and action menus. */
    if (object_info_index >= 2 && object_info_index < 22) {
        return (int)kPotionActionSet[object_info_index - 2];
    }
    if (object_info_index >= 23 && object_info_index < 69) {
        return (int)kWeaponActionSet[object_info_index - 23];
    }
    if (object_info_index >= 69 && object_info_index < 127) {
        return (int)kArmourActionSet[object_info_index - 69];
    }
    if (object_info_index >= 127 && object_info_index < 180) {
        return (int)kJunkActionSet[object_info_index - 127];
    }
    return 0;
}

static uint16_t csb_v1_runtime_object_allowed_slots_from_object_info(
    int object_info_index)
{
    static const uint16_t kObjectInfoAllowedSlots[180] = {
        0x0500, 0x0200, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0501, 0x0501, 0x0501, 0x0501,
        0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
        0x0500, 0x0400, 0x0400, 0x0040, 0x0040, 0x0040, 0x0040, 0x05C0, 0x0040, 0x0040, 0x0040, 0x0040,
        0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0440, 0x0040, 0x0040,
        0x0040, 0x0040, 0x05C0, 0x05C0, 0x0440, 0x05C0, 0x05C0, 0x05C0, 0x0040, 0x0040, 0x0540, 0x0540,
        0x0040, 0x0040, 0x0040, 0x0040, 0x0440, 0x0040, 0x0440, 0x0040, 0x0040, 0x040C, 0x040C, 0x0410,
        0x0420, 0x0420, 0x0408, 0x0410, 0x0408, 0x0410, 0x0408, 0x0408, 0x0410, 0x0410, 0x0408, 0x0410,
        0x0420, 0x0408, 0x0410, 0x0420, 0x0410, 0x0408, 0x0408, 0x0410, 0x0402, 0x0402, 0x0402, 0x0402,
        0x0402, 0x0400, 0x0200, 0x0200, 0x0200, 0x0408, 0x0410, 0x0408, 0x0410, 0x0402, 0x0420, 0x0402,
        0x0008, 0x0010, 0x0420, 0x0200, 0x0402, 0x0008, 0x0010, 0x0420, 0x0200, 0x0402, 0x0008, 0x0010,
        0x0420, 0x0200, 0x0402, 0x0408, 0x0010, 0x0420, 0x0408, 0x0500, 0x0501, 0x0504, 0x0504, 0x0500,
        0x0400, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
        0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0200, 0x0500, 0x0500, 0x0500,
        0x0501, 0x0501, 0x0501, 0x0501, 0x0401, 0x0401, 0x0501, 0x0501, 0x0504, 0x0504, 0x0504, 0x0504,
        0x0504, 0x0500, 0x0500, 0x0500, 0x0400, 0x0500, 0x0500, 0x0504, 0x0500, 0x0500, 0x0000, 0x0400
    };
    if (object_info_index < 0 || object_info_index >= 180) return 0;
    return kObjectInfoAllowedSlots[object_info_index];
}

static const char *csb_v1_runtime_object_name_from_record(
    int thing_type,
    const uint8_t *record,
    int record_size)
{
    static const char *const kWeaponTypeNames[] = {
        "EYE OF TIME", "STORMRING", "TORCH", "FLAMITT",
        "STAFF OF CLAWS", "BOLT BLADE", "FURY", "THE FIRESTAFF",
        "DAGGER", "FALCHION", "SWORD", "RAPIER",
        "SABRE", "SAMURAI SWORD", "DELTA", "DIAMOND EDGE",
        "VORPAL BLADE", "THE INQUISITOR", "AXE", "HARDCLEAVE",
        "MACE", "MACE OF ORDER", "MORNING STAR", "CLUB",
        "STONE CLUB", "BOW", "CROSSBOW", "ARROW",
        "SLAYER", "SLING", "ROCK", "POISON DART",
        "THROWING STAR", "STICK", "STAFF", "WAND",
        "TEOWAND", "YEW STAFF", "STAFF OF MANAR", "SNAKE STAFF",
        "THE CONDUIT", "DRAGON SPIT", "SCEPTRE OF LYF", "HORN OF FEAR",
        "SPEED BOW", "THE FIRESTAFF"
    };
    static const char *const kPotionTypeNames[] = {
        "MON POTION", "UM POTION", "DES POTION", "VEN POTION",
        "SAR POTION", "ZO POTION", "ROS POTION", "KU POTION",
        "DANE POTION", "NETA POTION", "BRO POTION", "MA POTION",
        "YA POTION", "EE POTION", "VI POTION", "WATER FLASK",
        "EMPTY FLASK"
    };
    static const char *const kArmourTypeNames[] = {
        "CAPE", "CLOAK OF NIGHT", "BARBARIAN HIDE", "SANDALS",
        "LEATHER BOOTS", "ELVEN BOOTS", "LEATHER JERKIN", "LEATHER PANTS",
        "SUEDE BOOTS", "BLUE PANTS", "GHI", "GHI TROUSERS",
        "CALISTA", "CROWN OF NERRA", "BEZERKER HELM", "HELMET",
        "BASINET", "NETA SHIRT", "CHAINMAIL", "PLATE MAIL",
        "MITHRAL MAIL", "MITHRAL HOSEN", "LEG MAIL", "FOOT PLATE",
        "SMALL SHIELD", "WOODEN SHIELD", "LARGE SHIELD", "SHIELD OF LYTE",
        "SHIELD OF DARC", "DEXHELM"
    };
    static const char *const kJunkTypeNames[] = {
        "COMPASS", "TORCH", "WATERSKIN", "JEWEL SYMAL",
        "ILLUMULET", "ASHES", "BONES", "SAR COIN",
        "GOLD COIN", "IRON KEY", "KEY OF B", "SOLID KEY",
        "SQUARE KEY", "TOURQUOISE KEY", "CROSS KEY", "ONYX KEY",
        "SKELETON KEY", "GOLD KEY", "WINGED KEY", "TOPAZ KEY",
        "SAPPHIRE KEY", "EMERALD KEY", "RUBY KEY", "RA KEY",
        "MASTER KEY", "BOULDER", "BLUE GEM", "ORANGE GEM",
        "GREEN GEM", "APPLE", "CORN", "BREAD",
        "CHEESE", "SCREAMER SLICE", "WORM ROUND", "DRUMSTICK",
        "DRAGON STEAK", "GEM OF AGES", "EKKHARD CROSS", "MOONSTONE",
        "THE HELLION", "PENDANT FERAL", "MAGICAL BOX", "MIRROR OF DAWN",
        "ROPE", "RABBIT FOOT", "CORBAMITE", "CHOKER",
        "LOCK PICKS", "MAGNIFIER", "ZOKATHRA SPELL", "EMPTY FLASK"
    };
    uint16_t word;
    int subtype;

    if (!record || record_size < 4) return NULL;
    word = csb_v1_runtime_read_u16(record + 2);
    switch (thing_type) {
    case THING_TYPE_SCROLL:
        return "SCROLL";
    case THING_TYPE_CONTAINER:
        return "CHEST";
    case THING_TYPE_POTION:
        subtype = (int)((word >> 8) & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kPotionTypeNames) / sizeof(kPotionTypeNames[0]))) {
            return kPotionTypeNames[subtype];
        }
        return "POTION";
    case THING_TYPE_WEAPON:
        subtype = (int)(word & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kWeaponTypeNames) / sizeof(kWeaponTypeNames[0]))) {
            return kWeaponTypeNames[subtype];
        }
        return "WEAPON";
    case THING_TYPE_ARMOUR:
        subtype = (int)(word & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kArmourTypeNames) / sizeof(kArmourTypeNames[0]))) {
            return kArmourTypeNames[subtype];
        }
        return "ARMOUR";
    case THING_TYPE_JUNK:
        subtype = (int)(word & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kJunkTypeNames) / sizeof(kJunkTypeNames[0]))) {
            return kJunkTypeNames[subtype];
        }
        return "JUNK";
    default:
        return NULL;
    }
}

int csb_v1_runtime_object_icon_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word;
    int thing_type;
    int record_size;
    int object_info_index;
    int icon_index;

    if (!profile || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return -1;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record) return -1;

    object_info_index = csb_v1_runtime_object_info_index_from_record(
        thing_type,
        record,
        record_size);
    icon_index = csb_v1_runtime_object_icon_from_object_info(
        object_info_index);
    if (icon_index < 0) return -1;

    /* ReDMCSB DUNGEON.C F0141 maps thing type/subtype to G0237 object info;
     * OBJECT.C F0033 then applies per-object dynamic icon adjustments for
     * compass direction, lit torches, closed scrolls, charge-bearing junk,
     * and charge-bearing magical weapons. */
    word = csb_v1_runtime_read_u16(record + 2);
    if (thing_type == THING_TYPE_WEAPON) {
        int charge = (int)((word >> 10) & 0x0Fu);
        int lit = (word & 0x8000u) ? 1 : 0;
        if (icon_index == 4 && lit) {
            static const unsigned char kChargeCountToTorchIconOffset[16] = {
                0,1,1,1,2,2,2,2,3,3,3,3,3,3,3,3
            };
            icon_index += (int)kChargeCountToTorchIconOffset[charge & 0x0F];
        } else if (charge &&
                   (icon_index == 14 || icon_index == 16 ||
                    icon_index == 18 || icon_index == 20 ||
                    icon_index == 23 || icon_index == 25)) {
            icon_index += 1;
        }
    } else if (thing_type == THING_TYPE_SCROLL) {
        int closed = (int)((word >> 10) & 0x3Fu);
        if (icon_index == 30 && closed) icon_index += 1;
    } else if (thing_type == THING_TYPE_JUNK) {
        int charge = (int)((word >> 14) & 0x03u);
        if (icon_index == 0) {
            icon_index += profile->party_dir & 0x03;
        } else if (charge &&
                   (icon_index == 8 || icon_index == 10 ||
                    icon_index == 12)) {
            icon_index += 1;
        }
    }
    return icon_index;
}

int csb_v1_runtime_object_subtype_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word;
    int thing_type;
    int record_size;
    int subtype;

    if (!profile || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return -1;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record || record_size < 4) {
        return -1;
    }
    if (thing_type == THING_TYPE_SCROLL) {
        return 0;
    }
    if (thing_type == THING_TYPE_CONTAINER) {
        if (record_size < 8) return -1;
        /* ReDMCSB DEFS.H CONTAINER: Next +0, Slot +2, Type +4.
         * Keep render subtype tied to Type, not first contained thing. */
        word = csb_v1_runtime_read_u16(record + 4);
        subtype = (int)((word >> 1) & 0x03u);
        return subtype > 0 ? 0 : subtype;
    }
    word = csb_v1_runtime_read_u16(record + 2);
    if (thing_type == THING_TYPE_POTION) {
        subtype = (int)((word >> 8) & 0x7Fu);
        return subtype <= 20 ? subtype : -1;
    }
    subtype = (int)(word & 0x7Fu);
    if (thing_type == THING_TYPE_WEAPON) {
        return subtype <= 45 ? subtype : -1;
    }
    if (thing_type == THING_TYPE_ARMOUR) {
        return subtype <= 57 ? subtype : -1;
    }
    if (thing_type == THING_TYPE_JUNK) {
        return subtype <= 52 ? subtype : -1;
    }
    return -1;
}

int csb_v1_runtime_object_action_set_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    int thing_type;
    int record_size;
    int object_info_index;

    if (!profile) return 0;
    dungeon = (const CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon ||
        thing == THING_NONE ||
        thing == THING_ENDOFLIST) {
        return 0;
    }
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record) return 0;
    object_info_index = csb_v1_runtime_object_info_index_from_record(
        thing_type,
        record,
        record_size);
    return csb_v1_runtime_object_action_set_from_object_info(
        object_info_index);
}

uint16_t csb_v1_runtime_object_allowed_slots(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    int thing_type;
    int record_size;
    int object_info_index;

    if (!profile) return 0;
    dungeon = (const CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon ||
        thing == THING_NONE ||
        thing == THING_ENDOFLIST) {
        return 0;
    }
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record) return 0;
    object_info_index = csb_v1_runtime_object_info_index_from_record(
        thing_type,
        record,
        record_size);
    return csb_v1_runtime_object_allowed_slots_from_object_info(
        object_info_index);
}

int csb_v1_runtime_throw_action_hand(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int *out_projectile_slot)
{
    CSB_V1_Champion *champion;
    CSB_V1_RuntimeWeaponInfoPc34 weapon_info;
    CSB_V1_RuntimeWeaponInfoPc34 *weapon_info_ptr = NULL;
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat first_move;
    struct RngState_Compat rng;
    uint16_t thrown_thing;
    int slot = -1;
    int party_dir;
    int throw_side;
    int throw_skill_level;
    int weapon_kinetic_energy = 1;
    int kinetic_energy;
    int attack;
    int step_energy;
    int projectile_subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    int potion_power = 0;
    int potion_subtype;

    if (out_projectile_slot) *out_projectile_slot = -1;
    if (!profile || !profile->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }

    champion = &profile->party_state.Champions[champion_index];
    if (csb_v1_champion_is_dead(champion) ||
        champion->CurrentHealth <= 0) {
        return 0;
    }

    thrown_thing = champion->Slots[CSB_V1_SLOT_ACTION_HAND];
    if (thrown_thing == THING_NONE || thrown_thing == THING_ENDOFLIST) {
        return 0;
    }
    if (csb_v1_runtime_weapon_info_for_thing(
            profile, thrown_thing, &weapon_info)) {
        weapon_info_ptr = &weapon_info;
        if ((int)weapon_info.weapon_class <= 12) {
            weapon_kinetic_energy = (int)weapon_info.kinetic_energy;
        }
    }
    potion_subtype = csb_v1_runtime_potion_projectile_subtype(
        profile,
        thrown_thing,
        &potion_power);
    if (potion_subtype >= 0) {
        projectile_subtype = potion_subtype;
    }

    party_dir = profile->party_dir & 3;
    throw_side = (((int)champion->Cell == ((party_dir + 1) & 3)) ||
                  ((int)champion->Cell == ((party_dir + 2) & 3))) ? 1 : 0;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^
            ((uint32_t)profile->game_time * 1103515245u) ^
            ((uint32_t)(champion_index & 0x03) << 24) ^
            ((uint32_t)(THING_GET_TYPE(thrown_thing) & 0x0F) << 16) ^
            ((uint32_t)(THING_GET_INDEX(thrown_thing) & 0x03FF) << 4) ^
            0xF0328u);
    kinetic_energy = csb_v1_runtime_f0312_action_hand_strength(
        profile,
        champion_index,
        champion,
        thrown_thing,
        weapon_info_ptr,
        &rng);
    throw_skill_level = csb_v1_runtime_get_champion_skill_level(
        profile,
        champion_index,
        10);
    if (throw_skill_level < 0) throw_skill_level = 0;
    kinetic_energy += weapon_kinetic_energy;
    kinetic_energy += F0732_COMBAT_RngRandom_Compat(&rng, 16) +
                      (kinetic_energy >> 1) +
                      throw_skill_level;
    attack = (throw_skill_level << 3) +
             F0732_COMBAT_RngRandom_Compat(&rng, 32);
    if (attack < 40) attack = 40;
    if (attack > 200) attack = 200;
    step_energy = 11 - throw_skill_level;
    if (step_energy < 5) step_energy = 5;

    memset(&input, 0, sizeof(input));
    memset(&first_move, 0, sizeof(first_move));
    input.category = PROJECTILE_CATEGORY_KINETIC;
    input.subtype = projectile_subtype;
    input.ownerKind = PROJECTILE_OWNER_CHAMPION;
    input.ownerIndex = champion_index;
    input.mapIndex = profile->current_level;
    input.mapX = profile->party_x;
    input.mapY = profile->party_y;
    input.cell = (party_dir + throw_side) & 3;
    input.direction = party_dir;
    /* ReDMCSB CHAMPION.C F0328 lines 2158-2189:
     * F0312 strength + throwable weapon kinetic energy + THROW skill/RNG,
     * bounded attack 40..200, and step energy max(5, 11 - THROW skill)
     * are passed to PROJEXPL.C F0212. */
    input.kineticEnergy = kinetic_energy;
    input.attack = attack;
    input.launcherStrength = input.attack;
    input.stepEnergy = step_energy;
    input.currentTick = (int)profile->game_time;
    input.attackTypeCode = COMBAT_ATTACK_NORMAL;
    input.associatedThing = (int)thrown_thing;
    input.potionPower = potion_power;
    input.firstMoveGraceFlag = 1;

    if (!F0810_PROJECTILE_Create_Compat(
            &input,
            &profile->projectiles,
            &slot,
            &first_move)) {
        return 0;
    }

    csb_v1_runtime_schedule_projectile_move_event(profile, &first_move);
    champion->Slots[CSB_V1_SLOT_ACTION_HAND] = THING_NONE;
    if (out_projectile_slot) *out_projectile_slot = slot;
    return 1;
}

int csb_v1_runtime_shoot_ready_hand(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int *out_projectile_slot)
{
    CSB_V1_Champion *champion;
    CSB_V1_RuntimeWeaponInfoPc34 action_info;
    CSB_V1_RuntimeWeaponInfoPc34 ready_info;
    uint16_t action_thing;
    uint16_t ready_thing;
    int action_class;
    int step_energy;
    int skill_level;
    int attack;
    int kinetic_energy;
    int projectile_slot = -1;

    if (out_projectile_slot) *out_projectile_slot = -1;
    if (!profile || !profile->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (csb_v1_champion_is_dead(champion) ||
        champion->CurrentHealth <= 0) {
        return 0;
    }

    action_thing = champion->Slots[CSB_V1_SLOT_ACTION_HAND];
    ready_thing = champion->Slots[CSB_V1_SLOT_READY_HAND];
    if (!csb_v1_runtime_weapon_info_for_thing(
            profile, action_thing, &action_info) ||
        !csb_v1_runtime_weapon_info_for_thing(
            profile, ready_thing, &ready_info)) {
        return 0;
    }
    action_class = (int)action_info.weapon_class;
    if (!csb_v1_runtime_shoot_ammunition_matches(&action_info, &ready_info) ||
        !csb_v1_runtime_shoot_step_energy(action_class, &step_energy)) {
        return 0;
    }

    skill_level = csb_v1_runtime_get_champion_skill_level(
        profile,
        champion_index,
        11);
    if (skill_level < 0) skill_level = 0;
    attack = ((int)action_info.shoot_attack + skill_level) << 1;
    if (attack > 255) attack = 255;
    kinetic_energy = (int)action_info.kinetic_energy +
                     (int)ready_info.kinetic_energy;

    /* ReDMCSB MENU.C F0407 lines 1363-1396 removes C00 ready hand and
     * calls CHAMPION.C F0326; F0326 feeds PROJEXPL.C F0212/F0810. */
    if (!csb_v1_runtime_spawn_champion_projectile(
            profile,
            champion_index,
            32,
            PROJECTILE_SUBTYPE_KINETIC_ARROW,
            PROJECTILE_CATEGORY_KINETIC,
            kinetic_energy,
            attack,
            COMBAT_ATTACK_NORMAL,
            step_energy,
            ready_thing,
            0,
            0,
            &projectile_slot)) {
        return 0;
    }
    champion->Slots[CSB_V1_SLOT_READY_HAND] = THING_NONE;
    if (out_projectile_slot) *out_projectile_slot = projectile_slot;
    return 1;
}

int csb_v1_runtime_refill_ready_hand_after_shoot(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int *out_source_slot,
    uint16_t *out_thing)
{
    static const int kSourceQuiverOrder[] = {
        12, /* ReDMCSB C12_SLOT_QUIVER_LINE1_1 */
        7,  /* ReDMCSB C07_SLOT_QUIVER_LINE2_1 */
        8,  /* ReDMCSB C08_SLOT_QUIVER_LINE1_2 */
        9   /* ReDMCSB C09_SLOT_QUIVER_LINE2_2 */
    };
    CSB_V1_Champion *champion;
    CSB_V1_RuntimeWeaponInfoPc34 action_info;
    int i;

    if (out_source_slot) *out_source_slot = -1;
    if (out_thing) *out_thing = THING_NONE;
    if (!profile || !profile->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (csb_v1_champion_is_dead(champion) ||
        champion->CurrentHealth <= 0 ||
        champion->Slots[CSB_V1_SLOT_READY_HAND] != THING_NONE) {
        return 0;
    }
    if (!csb_v1_runtime_weapon_info_for_thing(
            profile,
            champion->Slots[CSB_V1_SLOT_ACTION_HAND],
            &action_info)) {
        return 0;
    }

    for (i = 0;
         i < (int)(sizeof(kSourceQuiverOrder) / sizeof(kSourceQuiverOrder[0]));
         ++i) {
        int slot = kSourceQuiverOrder[i];
        uint16_t ammo_thing;
        CSB_V1_RuntimeWeaponInfoPc34 ammo_info;

        if (slot < 0 || slot >= CSB_V1_SLOT_COUNT) continue;
        ammo_thing = champion->Slots[slot];
        if (ammo_thing == THING_NONE || ammo_thing == THING_ENDOFLIST) {
            continue;
        }
        if (!csb_v1_runtime_weapon_info_for_thing(
                profile,
                ammo_thing,
                &ammo_info) ||
            !csb_v1_runtime_shoot_ammunition_matches(
                &action_info,
                &ammo_info)) {
            continue;
        }

        /* ReDMCSB TIMELINE.C F0253 lines ~1597-1607 moves the first
         * compatible quiver object into C00_READY_HAND when the action
         * enable event closes after SHOOT. */
        champion->Slots[CSB_V1_SLOT_READY_HAND] = ammo_thing;
        champion->Slots[slot] = THING_NONE;
        if (out_source_slot) *out_source_slot = slot;
        if (out_thing) *out_thing = ammo_thing;
        return 1;
    }
    return 0;
}

int csb_v1_runtime_spawn_champion_projectile(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int action_index,
    int projectile_subtype,
    int projectile_category,
    int kinetic_energy,
    int attack,
    int attack_type_code,
    int step_energy,
    uint16_t associated_thing,
    int poison_attack,
    int potion_power,
    int *out_projectile_slot)
{
    CSB_V1_Champion *champion;
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat first_move;
    int slot = -1;
    int party_dir;
    int launch_cell;

    if (out_projectile_slot) *out_projectile_slot = -1;
    if (!profile || !profile->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        action_index < 0 ||
        action_index > 255) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (csb_v1_champion_is_dead(champion) ||
        champion->CurrentHealth <= 0) {
        return 0;
    }

    party_dir = profile->party_dir & 3;
    launch_cell = ((((((int)champion->Cell & 3) - party_dir + 1) & 2) >> 1) +
                   party_dir) & 3;

    memset(&input, 0, sizeof(input));
    memset(&first_move, 0, sizeof(first_move));
    input.category = projectile_category;
    input.subtype = projectile_subtype;
    input.ownerKind = PROJECTILE_OWNER_CHAMPION;
    input.ownerIndex = champion_index;
    input.mapIndex = profile->current_level;
    input.mapX = profile->party_x;
    input.mapY = profile->party_y;
    input.cell = launch_cell;
    input.direction = party_dir;
    input.kineticEnergy = kinetic_energy;
    input.attack = attack;
    input.launcherStrength = attack;
    input.stepEnergy = step_energy > 0 ? step_energy : 1;
    input.currentTick = (int)profile->game_time;
    input.poisonAttack = poison_attack;
    input.attackTypeCode = attack_type_code;
    input.potionPower = potion_power;
    input.associatedThing =
        (associated_thing != THING_NONE && associated_thing != THING_ENDOFLIST)
            ? (int)associated_thing
            : (int)THING_NONE;
    input.firstMoveGraceFlag = 1;

    if (!F0810_PROJECTILE_Create_Compat(
            &input,
            &profile->projectiles,
            &slot,
            &first_move)) {
        return 0;
    }

    csb_v1_runtime_schedule_projectile_move_event(profile, &first_move);
    champion->ActionIndex = (uint8_t)action_index;
    if (out_projectile_slot) *out_projectile_slot = slot;
    return 1;
}

int csb_v1_runtime_record_champion_action(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int action_index)
{
    CSB_V1_Champion *champion;

    if (!profile || !profile->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        action_index < 0 ||
        action_index > 255) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (csb_v1_champion_is_dead(champion) ||
        champion->CurrentHealth <= 0) {
        return 0;
    }

    /* ReDMCSB MENU.C F0407 records the selected action in the champion action
     * state before the common action tail.  M11 owns the visible action-disable
     * countdown, but CSB runtime saves/resume need the selected source action
     * to stay on the CSB party snapshot instead of being lost in DM1 state. */
    champion->ActionIndex = (uint8_t)action_index;
    return 1;
}

static int csb_v1_runtime_action_is_melee_contact(int action_index)
{
    switch (action_index) {
    case DM1_ACTION_PUNCH:
    case DM1_ACTION_STAB_NINJA:
    case DM1_ACTION_STAB_FIGHTER:
    case DM1_ACTION_THRUST:
    case DM1_ACTION_JAB:
    case DM1_ACTION_PARRY:
    case DM1_ACTION_DISRUPT:
    case DM1_ACTION_MELEE:
    case DM1_ACTION_SLASH:
    case DM1_ACTION_CLEAVE:
    case DM1_ACTION_STUN:
        return 1;
    default:
        return 0;
    }
}

static int csb_v1_runtime_action_hits_closed_door(int action_index)
{
    switch (action_index) {
    case DM1_ACTION_CHOP:
    case DM1_ACTION_KICK:
    case DM1_ACTION_SWING:
    case DM1_ACTION_HACK:
    case DM1_ACTION_BERZERK:
    case DM1_ACTION_BASH:
        return 1;
    default:
        return 0;
    }
}

static int csb_v1_runtime_door_defense_pc34(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int door_type)
{
    static const unsigned char s_i34_door_defense[4] = {
        110, 42, 230, 255
    };
    int door_set;

    if (!dungeon || level < 0 || level >= dungeon->level_count) return 255;
    door_set = door_type ? dungeon->map_door_set1[level]
                         : dungeon->map_door_set0[level];
    return (int)s_i34_door_defense[door_set & 3];
}

static void csb_v1_runtime_request_closed_door_melee_sounds(
    CSB_V1_RuntimeProfile *profile)
{
    CsbV1AudioRequest request;

    if (!profile) return;
    memset(&request, 0, sizeof(request));
    request.soundIndex = CSB_V1_SOUND_COMBAT;
    request.mapX = (int16_t)profile->party_x;
    request.mapY = (int16_t)profile->party_y;
    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
    request.volume = 64;
    request.priority = 6u;
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);

    memset(&request, 0, sizeof(request));
    request.soundIndex = CSB_V1_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM;
    request.mapX = (int16_t)profile->party_x;
    request.mapY = (int16_t)profile->party_y;
    request.mode = CSB_V1_MODE_PLAY_ONE_TICK_LATER;
    request.volume = 64;
    request.priority = 4u;
    /* ReDMCSB MENU.C F0407 lines 1308-1324 requests M563 immediately and
     * C04 one tick later for the closed-door melee branch. The bounded CSB
     * audio runtime records pending sound arbitration; actual mixing remains
     * outside this game-logic bridge. */
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
}

static int csb_v1_runtime_queue_door_destruction_event(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y)
{
    struct DM1_Event_V1 event;

    if (!profile) return 0;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(level, profile->game_time + 2u);
    event.type = DM1_EVENT_DOOR_DESTRUCTION;
    event.priority = 0;
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    return dm1v1_event_add(&profile->timeline_queue, &event) >= 0;
}

static int csb_v1_runtime_try_closed_door_melee(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    CSB_V1_Champion *champion,
    int champion_index,
    int action_index,
    int target_x,
    int target_y,
    int raw_square,
    uint16_t action_thing,
    const CSB_V1_RuntimeWeaponInfoPc34 *runtime_weapon,
    struct RngState_Compat *rng,
    CSB_V1_RuntimeMeleeActionResult *out_result)
{
    int first_thing;
    int thing;
    int guard;

    if (!profile || !dungeon || !champion || !rng) return 0;
    if (!csb_v1_runtime_action_hits_closed_door(action_index)) return 0;
    if (csb_v1_runtime_square_type_from_raw(dungeon, raw_square) != 4 ||
        (raw_square & 0x07) != 4) {
        return 0;
    }

    if (out_result) out_result->hit_door = 1;
    csb_v1_runtime_request_closed_door_melee_sounds(profile);

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        profile->current_level,
        target_x,
        target_y);
    if (first_thing < 0) return 1;

    for (guard = 0, thing = first_thing;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        uint8_t *record;
        uint16_t door_word;
        int thing_type;
        int thing_size;
        int door_type;
        int attack;

        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 4) return 1;
        if (thing_type != THING_TYPE_DOOR) {
            thing = csb_v1_runtime_read_u16(record + 0);
            continue;
        }

        door_word = csb_v1_runtime_read_u16(record + 2);
        if ((door_word & 0x0100u) == 0u) {
            return 1;
        }
        door_type = (int)(door_word & 0x0001u);
        attack = csb_v1_runtime_f0312_action_hand_strength(
            profile,
            champion_index,
            champion,
            action_thing,
            runtime_weapon,
            rng);
        /* ReDMCSB: MENU.C F0407 lines 1308-1324 routes only the door-hit
         * actions here; PROJEXPL.C F0232 lines 1575-1594 checks the door
         * record's MeleeDestructible bit, compares attack with the active
         * map door-set defense, and schedules C02 at GameTime+2 while the
         * closed square remains unchanged in the action tick. */
        if (attack >= csb_v1_runtime_door_defense_pc34(
                dungeon,
                profile->current_level,
                door_type) &&
            csb_v1_runtime_queue_door_destruction_event(
                profile,
                profile->current_level,
                target_x,
                target_y)) {
            if (out_result) {
                out_result->door_destroyed = 1;
                out_result->door_event_scheduled = 1;
            }
        }
        return 1;
    }

    return 1;
}

static int csb_v1_runtime_target_group_creature_index(
    int champion_cell,
    int cells,
    int creature_count)
{
    int i;
    if (creature_count < 1) return -1;
    if (cells == 0xFF) return 0;
    for (i = 0; i < creature_count; ++i) {
        if (csb_v1_runtime_group_cell_value(cells, i) ==
            (champion_cell & 3)) {
            return i;
        }
    }
    return 0;
}

static void csb_v1_runtime_build_melee_weapon_profile(
    int action_index,
    uint16_t action_thing,
    const CSB_V1_RuntimeWeaponInfoPc34 *runtime_info,
    struct WeaponProfile_Compat *out)
{
    int hit_probability;
    int damage_factor;

    memset(out, 0, sizeof(*out));
    hit_probability =
        dm1_v1_graphic560_action_hit_probability_get_pc34(action_index);
    damage_factor =
        dm1_v1_graphic560_action_damage_factor_get_pc34(action_index);
    if (hit_probability < 0) hit_probability = 0;
    if (damage_factor < 0) damage_factor = 0;

    out->weaponType = (int)(action_thing & 0x03FFu);
    out->weaponClass = runtime_info ? (int)runtime_info->weapon_class : 255;
    out->weaponStrength = runtime_info ? (int)runtime_info->strength : 0;
    out->kineticEnergy = runtime_info ? (int)runtime_info->kinetic_energy : 0;
    out->hitProbability = hit_probability;
    if (action_index == DM1_ACTION_DISRUPT) {
        out->hitProbability |= 0x8000;
    }
    out->damageFactor = damage_factor;
}

int csb_v1_runtime_perform_melee_action(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int action_index,
    CSB_V1_RuntimeMeleeActionResult *out_result)
{
    CSB_V1_DungeonData *dungeon;
    CSB_V1_Champion *champion;
    CSB_V1_RuntimeWeaponInfoPc34 runtime_weapon;
    CSB_V1_RuntimeWeaponInfoPc34 *runtime_weapon_ptr = NULL;
    struct CombatantChampionSnapshot_Compat attacker;
    struct CombatantCreatureSnapshot_Compat defender;
    struct WeaponProfile_Compat weapon;
    struct CombatResult_Compat combat;
    struct RngState_Compat rng;
    uint16_t action_thing;
    int dx = 0;
    int dy = 0;
    int target_x;
    int target_y;
    int square_type;
    int raw_square;
    int first_thing;
    int thing;
    int guard;

    if (out_result) memset(out_result, 0, sizeof(*out_result));
    if (!profile || !profile->party_state_valid || !profile->dungeon_handle) {
        return 0;
    }
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        action_index < 0 ||
        action_index > 255) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (csb_v1_champion_is_dead(champion) ||
        champion->CurrentHealth <= 0) {
        return 0;
    }
    if (!csb_v1_runtime_record_champion_action(
            profile,
            champion_index,
            action_index)) {
        return 0;
    }

    dungeon = profile->dungeon_handle;
    csb_v1_runtime_projectile_step(profile->party_dir, &dx, &dy);
    target_x = profile->party_x + dx;
    target_y = profile->party_y + dy;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        profile->current_level,
        target_x,
        target_y);
    square_type = csb_v1_dungeon_get_square_type(
        dungeon,
        profile->current_level,
        target_x,
        target_y);
    if (out_result) {
        out_result->action_index = action_index;
        out_result->performed = 1;
        out_result->target_map_index = profile->current_level;
        out_result->target_map_x = target_x;
        out_result->target_map_y = target_y;
        out_result->target_square_type = square_type;
    }
    action_thing = champion->Slots[CSB_V1_SLOT_ACTION_HAND];
    if (csb_v1_runtime_weapon_info_for_thing(
            profile,
            action_thing,
            &runtime_weapon)) {
        runtime_weapon_ptr = &runtime_weapon;
    }

    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^
            (uint32_t)(profile->game_time * 1103515245u) ^
            ((uint32_t)(champion_index & 0x03) << 4) ^
            ((uint32_t)(action_index & 0xFF) << 8) ^
            ((uint32_t)(target_x & 0xFF) << 16) ^
            ((uint32_t)(target_y & 0xFF) << 24) ^
            0xF0231u);

    if (csb_v1_runtime_try_closed_door_melee(
            profile,
            dungeon,
            champion,
            champion_index,
            action_index,
            target_x,
            target_y,
            raw_square,
            action_thing,
            runtime_weapon_ptr,
            &rng,
            out_result)) {
        return 1;
    }
    if (!csb_v1_runtime_action_is_melee_contact(action_index)) return 1;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        profile->current_level,
        target_x,
        target_y);
    if (first_thing < 0) return 1;

    /* ReDMCSB: MENU.C F0402 lines ~1021-1057 resolves a concrete melee
     * target creature before calling PROJEXPL.C F0231. This CSB bridge walks
     * the real-format square thing list and mutates C04 GROUP.Health[4]
     * directly; active-group drops/aspect/smoke side effects remain separate
     * CSB runtime slices. */
    for (guard = 0, thing = first_thing;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        uint8_t *record;
        uint16_t flags;
        uint16_t hp;
        uint8_t *hp_ptr;
        int thing_type;
        int thing_size;
        int creature_count;
        int creature_type;
        int creature_index;
        int cells;

        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 16) return 1;
        if (thing_type != THING_TYPE_GROUP) {
            thing = csb_v1_runtime_read_u16(record + 0);
            continue;
        }

        flags = csb_v1_runtime_read_u16(record + 14);
        creature_count = (int)((flags >> 5) & 0x03u) + 1;
        if (creature_count < 1) creature_count = 1;
        if (creature_count > 4) creature_count = 4;
        cells = record[5];
        creature_type = (int)record[4];
        creature_index = csb_v1_runtime_target_group_creature_index(
            champion->Cell,
            cells,
            creature_count);
        if (creature_index < 0 || creature_index >= creature_count) return 1;

        if (!csb_v1_runtime_fill_creature_combat_snapshot(
                creature_type,
                creature_index,
                &defender)) {
            return 1;
        }
        hp_ptr = record + 6 + creature_index * 2;
        hp = csb_v1_runtime_read_u16(hp_ptr);
        if (hp == 0) return 1;
        defender.healthBefore = (int)hp;

        memset(&attacker, 0, sizeof(attacker));
        attacker.championIndex = champion_index;
        attacker.currentHealth = champion->CurrentHealth;
        attacker.dexterity = csb_v1_runtime_stat_or_default(
            champion,
            CSB_V1_STAT_DEX,
            CSB_V1_STAT_CUR);
        attacker.strengthActionHand =
            csb_v1_runtime_f0312_action_hand_strength(
                profile,
                champion_index,
                champion,
                action_thing,
                runtime_weapon_ptr,
                &rng);
        attacker.skillLevelParry =
            csb_v1_runtime_imported_skill_level(champion, 7);
        attacker.skillLevelAction =
            csb_v1_runtime_imported_skill_level(champion, 4);
        attacker.statisticLuck = csb_v1_runtime_stat_or_default(
            champion,
            CSB_V1_STAT_LUCK,
            CSB_V1_STAT_CUR);
        attacker.statisticLuckMax = csb_v1_runtime_stat_or_default(
            champion,
            CSB_V1_STAT_LUCK,
            CSB_V1_STAT_MAX);
        attacker.statisticLuckMin = csb_v1_runtime_stat_or_default(
            champion,
            CSB_V1_STAT_LUCK,
            CSB_V1_STAT_MIN);
        attacker.wounds = champion->Wounds;
        csb_v1_runtime_build_melee_weapon_profile(
            action_index,
            action_thing,
            runtime_weapon_ptr,
            &weapon);

        if (!F0735_COMBAT_ResolveChampionMelee_Compat(
                &attacker,
                &weapon,
                &defender,
                &rng,
                &combat)) {
            return 1;
        }
        if (combat.damageApplied > 0) {
            if (combat.damageApplied >= (int)hp) {
                csb_v1_runtime_pack_dead_group_creature(
                    profile,
                    dungeon,
                    record,
                    (uint16_t)thing,
                    profile->current_level,
                    target_x,
                    target_y,
                    creature_index,
                    &rng);
                if (out_result) {
                    out_result->killed_group =
                        (csb_v1_runtime_read_u16(record + 0) == 0xFFFFu);
                }
            } else {
                csb_v1_runtime_write_u16(
                    hp_ptr,
                    (uint16_t)((int)hp - combat.damageApplied));
            }
        }
        if (out_result) {
            out_result->hit_group = 1;
            out_result->creature_index = creature_index;
            out_result->damage = combat.damageApplied;
        }
        return 1;
    }

    return 1;
}

int csb_v1_runtime_load_object_names_m564(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t *bytes,
    size_t byte_count)
{
    size_t offset = 0u;
    int name_index;

    if (!profile || !bytes || byte_count == 0u) return 0;
    memset(profile->object_names, 0, sizeof(profile->object_names));
    profile->object_name_table_valid = 0;

    /* ReDMCSB OBJECT.C F0031 lines ~58-109 loads
     * M564_GRAPHIC_OBJECT_NAMES for PC media as C199 icon-indexed strings.
     * Each string ends when the source byte has bit 7 set; the stored
     * character is byte & 0x7f, followed by a C null terminator. */
    for (name_index = 0;
         name_index < CSB_V1_OBJECT_NAME_COUNT;
         ++name_index) {
        size_t written = 0u;
        int terminated = 0;

        while (offset < byte_count) {
            unsigned char c = bytes[offset++];
            if (written < (size_t)CSB_V1_OBJECT_NAME_MAX_CHARS) {
                profile->object_names[name_index][written++] =
                    (char)(c & 0x7fu);
            }
            if ((c & 0x80u) != 0u) {
                terminated = 1;
                break;
            }
        }
        profile->object_names[name_index][written] = '\0';
        if (!terminated) {
            memset(profile->object_names, 0, sizeof(profile->object_names));
            return 0;
        }
    }

    profile->object_name_table_valid = 1;
    return 1;
}

int csb_v1_runtime_object_name(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    char *out,
    size_t out_size)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    const char *name;
    int thing_type;
    int record_size;
    int icon_index;

    if (!out || out_size == 0U) return 0;
    out[0] = '\0';
    if (!profile || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return 0;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record) return 0;

    /* ReDMCSB OBJECT.C F0031 loads C199 icon-indexed names and F0034 draws
     * the leader-hand object name after F0033 icon resolution.  Prefer the
     * CSB-owned decoded M564 table; the subtype fallback is retained only for
     * startup/probe paths before CSBGRAPH has been bound. */
    icon_index = csb_v1_runtime_object_icon_index(profile, thing);
    if (profile->object_name_table_valid &&
        icon_index >= 0 &&
        icon_index < CSB_V1_OBJECT_NAME_COUNT &&
        profile->object_names[icon_index][0] != '\0') {
        snprintf(out, out_size, "%s", profile->object_names[icon_index]);
        return out[0] != '\0';
    }

    name = csb_v1_runtime_object_name_from_record(
        thing_type,
        record,
        record_size);
    if (!name || name[0] == '\0') return 0;
    snprintf(out, out_size, "%s", name);
    return out[0] != '\0';
}

static int csb_v1_runtime_party_has_possession_object_type(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    int object_type)
{
    int champion_count;
    int champion_index;

    if (!profile || !dungeon || object_type < 0 ||
        !profile->party_state_valid) {
        return 0;
    }
    champion_count = profile->party_state.ChampionCount;
    if (champion_count < 0) champion_count = 0;
    if (champion_count > CSB_V1_MAX_CHAMPIONS) {
        champion_count = CSB_V1_MAX_CHAMPIONS;
    }
    for (champion_index = 0;
         champion_index < champion_count;
         ++champion_index) {
        const CSB_V1_Champion *champion =
            &profile->party_state.Champions[champion_index];
        int slot_index;

        if (champion->CurrentHealth <= 0) continue;
        for (slot_index = 0;
             slot_index < CSB_V1_SLOT_COUNT &&
                 slot_index < DM1_SENSOR_POSSESSION_SLOT_LAST;
             ++slot_index) {
            uint16_t slot_thing = champion->Slots[slot_index];
            if (slot_thing == 0xFFFEu || slot_thing == 0xFFFFu) {
                continue;
            }
            if (csb_v1_runtime_scan_thing_chain_for_object_type(
                    dungeon,
                    slot_thing,
                    object_type)) {
                return 1;
            }
        }
    }
    /* ReDMCSB MOVESENS.C F0274 lines 1271-1306 checks G4055's leader-hand
     * object once after the loaded CHARDESC slots.  The runtime owns that
     * GAMEBLOCK2/party mirror as LeaderHandThing; a held C144 follows only
     * its source CONTAINER.Slot chain, never an M11 inventory projection. */
    {
        uint16_t leader_hand = csb_v1_runtime_export_leader_hand_thing(profile);

        if (leader_hand != THING_NONE && leader_hand != THING_ENDOFLIST) {
            if (csb_v1_runtime_scan_thing_chain_for_object_type(
                    dungeon, leader_hand, object_type)) {
                return 1;
            }
            if (THING_GET_TYPE(leader_hand) == THING_TYPE_CONTAINER) {
                uint16_t slots[8];
                int count = csb_v1_runtime_read_container_slots(
                    profile, leader_hand, slots);
                int slot_index;

                for (slot_index = 0; slot_index < count; ++slot_index) {
                    if (csb_v1_runtime_scan_thing_chain_for_object_type(
                            dungeon, slots[slot_index], object_type)) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static int csb_v1_runtime_f0276_pc34_version_checker_passes(
    int sensor_data)
{
    /* ReDMCSB MOVESENS.C F0276 C009 performs this PC34-only comparison
     * directly against its compiled engine value.  It is not a caller- or
     * save-provided version field: retaining it here prevents a restored
     * profile from selecting a substitute engine mode. */
    enum { CSB_V1_F0276_PC34_VERSION_CHECKER_MAX = 34 };

    return sensor_data >= 0 &&
        sensor_data <= CSB_V1_F0276_PC34_VERSION_CHECKER_MAX;
}

static void csb_v1_runtime_process_party_floor_sensors_at_level(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y,
    int add_party,
    int party_square,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int first_thing;
    int thing;
    int guard;
    int square_contains_object = 0;
    int square_contains_group = 0;
    int pending_local_effect = DM1_EFFECT_NONE;

    if (!profile || !result) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data) return;
    if (level < 0 || level >= dungeon->level_count) return;
    if (map_x < 0 || map_y < 0) return;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (first_thing < 0 || first_thing == 0xFFFE) return;

    /* ReDMCSB: MOVESENS.C F0267 lines 800-822 calls
     * F0276_SENSOR_ProcessThingAdditionOrRemoval when the party leaves and
     * enters a square.  F0276 lines 1587-1620 scans the square before the
     * sensor walk to classify occupancy (group present / object present),
     * then lines 1658-1785 walks C03 sensor things until the first
     * non-sensor, checks floor sensor types C001/C002/C003/C005/C008/C009
     * for the party, resolves HOLD into SET/CLEAR, then calls F0272/F0268 to
     * enqueue the square-effect event.  This CSB runtime slice covers party
     * floor sensors, including C008 party-possession checks over imported
     * champion slots; object/group movement sensors remain separate runtime
     * work. */
    {
        int scan = first_thing;
        int scan_guard;

        for (scan_guard = 0;
             scan_guard < 128 && scan != 0xFFFE && scan != 0xFFFF;
             ++scan_guard) {
            int scan_type;
            if (!csb_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)scan, &scan_type, NULL, NULL)) {
                break;
            }
            if (scan_type == 4) {           /* C04_THING_TYPE_GROUP */
                square_contains_group = 1;
            } else if (scan_type > 4 && scan_type < 14) {
                /* > C04_THING_TYPE_GROUP && < C14_THING_TYPE_PROJECTILE */
                square_contains_object = 1;
            }
            scan = csb_v1_runtime_sensor_next_thing(
                dungeon, (uint16_t)scan);
        }
    }
    thing = first_thing;
    for (guard = 0; guard < 128 && thing != 0xFFFE; ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int sensor_effect;
        int trigger;
        int target_x;
        int target_y;
        int target_cell;
        int delay;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record) break;
        if (thing_type >= 4) break;
        if (thing_type == 2) {
            /* ReDMCSB MOVESENS.C F0276:1630-1633 decodes a TextString when
             * the party enters its square.  Firestaff owns only one C015
             * receipt, so admit one visible original C02 record and reject a
             * multi-text list rather than inventing QuePrintLines behavior. */
            if (add_party && !party_square && dungeon->square_bytes == 1 &&
                thing_size >= 4) {
                int scan = first_thing;
                int scan_guard;
                int text_count = 0;
                CSB_V1_RuntimeTextMessageReceipt message_receipt;

                for (scan_guard = 0;
                     scan_guard < 128 && scan != 0xFFFE && scan != 0xFFFF;
                     ++scan_guard) {
                    int scan_type;
                    if (!csb_v1_dungeon_get_thing_record(
                            dungeon, (uint16_t)scan, &scan_type, NULL, NULL)) {
                        break;
                    }
                    if (scan_type == 2) ++text_count;
                    scan = csb_v1_runtime_sensor_next_thing(
                        dungeon, (uint16_t)scan);
                }
                if (text_count == 1) {
                    memset(&message_receipt, 0, sizeof(message_receipt));
                    if (csb_v1_runtime_stage_openroom_text_message(
                            profile, (uint16_t)thing,
                            csb_v1_runtime_read_u16(record + 2),
                            &message_receipt)) {
                        profile->csbwin_text_message_receipt = message_receipt;
                    }
                }
            }
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        if (thing_type != 3 || thing_size < 8) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        type_data = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
        flags_word = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
        target_word = (uint16_t)record[6] | ((uint16_t)record[7] << 8);
        sensor_type = (int)(type_data & 0x007Fu);
        sensor_data = (int)(type_data >> 7);
        if (sensor_type == 0) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        trigger = add_party ? 1 : 0;
        switch (sensor_type) {
        case 1: /* C001_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT */
            /* ReDMCSB MOVESENS.C F0276:1666-1669: suppressed when the party
             * was already on the square (PartySquare) or when the square
             * holds an object or a group (BUG0_30 levitation note kept in
             * the source; this slice has no levitation state). */
            if (party_square || square_contains_object ||
                square_contains_group) {
                trigger = 0;
            }
            break;
        case 2: /* C002_SENSOR_FLOOR_THERON_PARTY_CREATURE */
            /* ReDMCSB MOVESENS.C F0276:1670-1673: suppressed for objects
             * (implicit here: this path only runs party additions), when the
             * party was already on the square, or when a group is present. */
            if (party_square || square_contains_group) {
                trigger = 0;
            }
            break;
        case 3: /* C003_SENSOR_FLOOR_PARTY */
            if (profile->champion_count <= 0) {
                trigger = 0;
            } else if (sensor_data != 0) {
                trigger = add_party &&
                    (sensor_data == ((profile->party_dir & 3) + 1));
            } else if (party_square) {
                /* ReDMCSB MOVESENS.C F0276:1677-1680: a non-directional C003
                 * does not re-trigger while the party stays on the square. */
                trigger = 0;
            }
            break;
        case 5: /* C005_SENSOR_FLOOR_PARTY_ON_STAIRS */
            {
                int raw_square = csb_v1_dungeon_get_raw_square(
                    dungeon,
                    level,
                    map_x,
                    map_y);
                int square_type = (raw_square < 0) ? -1 :
                    ((dungeon->square_bytes == 1)
                        ? ((raw_square >> 5) & 0x07)
                        : (raw_square & 0x1F));
                trigger = (square_type == 3) ? trigger : 0;
            }
            break;
        case 8: /* C008_SENSOR_FLOOR_PARTY_POSSESSION */
            trigger = add_party &&
                csb_v1_runtime_party_has_possession_object_type(
                    profile,
                    dungeon,
                    sensor_data);
            break;
        case 9: /* C009_SENSOR_FLOOR_VERSION_CHECKER */
            /* ReDMCSB MOVESENS.C F0276:1716-1720: only a party addition that
             * newly enters the square can trigger the version checker. */
            trigger = add_party && !party_square &&
                csb_v1_runtime_f0276_pc34_version_checker_passes(sensor_data);
            break;
        default:
            trigger = 0;
            break;
        }

        sensor_effect = (int)((flags_word >> 3) & 0x03u);
        if ((flags_word >> 5) & 0x01u) {
            trigger ^= 1;
        }
        if (sensor_effect == DM1_EFFECT_HOLD) {
            sensor_effect = trigger ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
        } else if (!trigger) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        target_cell = (int)((target_word >> 4) & 0x03u);
        target_x = (int)((target_word >> 6) & 0x1Fu);
        target_y = (int)((target_word >> 11) & 0x1Fu);
        delay = (int)((flags_word >> 7) & 0x0Fu);
        if ((flags_word >> 6) & 0x01u) {
            CsbV1AudioRequest request;

            memset(&request, 0, sizeof(request));
            request.soundIndex = CSB_V1_SOUND_SWITCH;
            request.mapX = (int16_t)map_x;
            request.mapY = (int16_t)map_y;
            request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
            request.volume = 64;
            request.priority = 4u;
            (void)csb_v1_audio_runtime_request(&profile->audio_runtime,
                                                &request);
            result->sensor_audible_count++;
        }
        if ((flags_word >> 2) & 0x01u) {
            uint8_t *mutable_sensor = csb_v1_runtime_mutable_thing_record(
                (CSB_V1_DungeonData *)dungeon, (uint16_t)thing, NULL, NULL);
            if (mutable_sensor) {
                csb_v1_runtime_write_u16(
                    mutable_sensor + 2, (uint16_t)(type_data & 0xFF80u));
            }
        }
        result->sensor_trigger_count++;
        result->sensor_last_type = sensor_type;
        result->sensor_last_data = sensor_data;
        result->sensor_last_effect = sensor_effect;
        result->sensor_last_target_x = target_x;
        result->sensor_last_target_y = target_y;
        result->sensor_last_target_cell = target_cell;
        if ((flags_word >> 11) & 0x01u) {
            int local_effect = (int)(target_word & 0x0FFFu);

            if (local_effect == 10) {
                csb_v1_runtime_add_party_steal_skill_experience(profile, 0);
            } else {
                pending_local_effect = local_effect;
            }
        } else {
            csb_v1_runtime_trigger_floor_sensor_event(
                profile,
                level,
                sensor_effect,
                target_x,
                target_y,
                target_cell,
                delay,
                result);
        }

        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    if (pending_local_effect == DM1_EFFECT_CLEAR ||
        pending_local_effect == DM1_EFFECT_TOGGLE) {
        (void)csb_v1_runtime_rotate_wall_cell_sensors(
            (CSB_V1_DungeonData *)dungeon, level, map_x, map_y, -1);
    }
}

static void csb_v1_runtime_process_party_floor_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int add_party,
    int party_square,
    CSB_V1_InputCommandRuntimeResult *result)
{
    if (!profile) return;
    csb_v1_runtime_process_party_floor_sensors_at_level(
        profile,
        profile->current_level,
        map_x,
        map_y,
        add_party,
        party_square,
        result);
}

static void csb_v1_runtime_apply_party_floor_sensor_consequences(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    if (!profile || !result) return;
    if (!result->movement_step_applied && !result->stair_transition_applied) {
        return;
    }

    result->sensor_source_remove_checked = 1;
    /* ReDMCSB MOVESENS.C F0267:801-806: the removal pass observes the square
     * the party is leaving, i.e. L0725_B_PartySquare is true there. */
    csb_v1_runtime_process_party_floor_sensors_at_level(
        profile,
        result->old_party_level,
        result->old_party_x,
        result->old_party_y,
        0,
        1,
        result);
    if (result->stair_transition_applied && result->movement_step_applied) {
        csb_v1_runtime_process_party_floor_sensors_at_level(
            profile,
            result->old_party_level,
            result->movement_destination_x,
            result->movement_destination_y,
            0,
            0,
            result);
    }
    if (result->new_party_level == result->old_party_level) {
        result->sensor_destination_add_checked = 1;
        csb_v1_runtime_process_party_floor_sensors_at(
            profile,
            profile->party_x,
            profile->party_y,
            1,
            0,
            result);
    }
}

static void csb_v1_runtime_apply_party_turn_floor_sensor_add_consequences(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    int level;
    int map_x;
    int map_y;

    if (!profile || !result) return;
    level = profile->current_level;
    map_x = profile->party_x;
    map_y = profile->party_y;
    if (level < 0) return;

    /* ReDMCSB: CLIKMENU.C F0365 lines 169-172 processes party floor
     * sensors on the current square before and after F0284 rotates the
     * party.  Directional C003 floor sensors therefore see the old facing
     * on removal and the new facing on addition.  The caller runs the
     * pre-rotation removal pass; this helper runs the post-rotation add. */
    result->sensor_destination_add_checked = 1;
    csb_v1_runtime_process_party_floor_sensors_at_level(
        profile,
        level,
        map_x,
        map_y,
        1,
        1,
        result);
}

static void csb_v1_runtime_mark_deferred_new_party_map_index(
    CSB_V1_InputCommandRuntimeResult *result)
{
    if (!result ||
        (!result->movement_step_applied &&
         !result->stair_transition_applied)) {
        return;
    }
    if (result->new_party_level == result->old_party_level) return;

    /* ReDMCSB MOVESENS.C F0267 lines 830-842: when party movement ends on
     * another map, the source engine does not run destination party sensors
     * immediately; it publishes G0327_i_NewPartyMapIndex for the outer game
     * loop/map handoff. Firestaff's bounded CSB runtime already updates the
     * active map directly, but it also exposes the source handoff signal so
     * M11/startup callers can observe the same boundary explicitly. */
    result->deferred_new_party_map_index_valid = 1;
    result->deferred_new_party_map_index = result->new_party_level;
}

static void csb_v1_runtime_trigger_remote_sensor_event(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell)
{
    CSB_V1_DungeonData *dungeon;
    struct DM1_Event_V1 event;
    int raw_square;
    int square_type;
    int event_type;

    if (!profile || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data) return;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        target_x,
        target_y);
    if (raw_square < 0) return;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    event_type = csb_v1_runtime_square_event_type_for_sensor_target(square_type);
    if (event_type == DM1_EVENT_NONE) return;
    /* F0272 preserves Remote.TargetCell only for wall targets before F0268.
     * Fakewalls, doors, pits, and teleporters are always north-west. */
    if (square_type != DM1_SQUARE_WALL) target_cell = 0;

    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(level, profile->game_time);
    event.type = (uint8_t)event_type;
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)target_cell;
    event.c_effect = (uint8_t)sensor_effect;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_trigger_remote_sensor_event_after(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell,
    int delay)
{
    uint32_t old_time;
    if (!profile) return;
    old_time = profile->game_time;
    if (delay > 0) profile->game_time += (uint32_t)delay;
    csb_v1_runtime_trigger_remote_sensor_event(profile, level, sensor_effect,
                                               target_x, target_y, target_cell);
    profile->game_time = old_time;
}

static void csb_v1_runtime_add_party_steal_skill_experience(
    CSB_V1_RuntimeProfile *profile,
    int leader_only)
{
    /* ReDMCSB: MOVESENS.C F0270_SENSOR_TriggerLocalEffect C10
     * (C10_EFFECT_ADD_300XP_STEAL_SKILL) calls F0269_SENSOR_AddSkillExperience
     * (C08_SKILL_STEAL, 300, cell != CM1_CELL_ANY).  F0269 lines 1058-1075
     * divides the 300 by G0305_ui_PartyChampionCount once and credits every
     * living champion (or the full 300 to G0411_i_LeaderIndex when
     * leader-only).  CHAMPION.C F0304 lines 865-893 then credits the hidden
     * C08 skill, grows its TemporaryExperience by F0026-bounded share/8
     * (cap 32000), and credits the same share to base skill
     * C01 = (C08-C04)>>2 because Steal is a Ninja hidden skill.  This
     * runtime slice owns no creature-attack clock (G0361/G0313) or map
     * difficulty, so the quiescent-timeline F0304 path (no >>1/<<1 combat
     * scaling, difficulty factor 0) is the faithful model here. */
    enum {
        CSB_V1_SKILL_STEAL_C08 = 8,
        CSB_V1_STEAL_XP_TOTAL_C10 = 300,
        CSB_V1_TEMPORARY_EXPERIENCE_CAP = 32000
    };
    int champion_index;
    int champion_count;
    int first_index = 0;
    int last_index;
    uint32_t share;

    if (!profile || !profile->party_state_valid) return;
    champion_count = profile->party_state.ChampionCount;
    if (champion_count > CSB_V1_MAX_CHAMPIONS) {
        champion_count = CSB_V1_MAX_CHAMPIONS;
    }
    if (champion_count <= 0) return;
    last_index = champion_count;
    share = (uint32_t)CSB_V1_STEAL_XP_TOTAL_C10 / (uint32_t)champion_count;
    if (leader_only) {
        /* F0269:1060-1063 — full 300 to the leader, no division, no
         * CurrentHealth guard. */
        if (profile->leader_index < 0 ||
            profile->leader_index >= champion_count) {
            return;
        }
        first_index = profile->leader_index;
        last_index = first_index + 1;
        share = (uint32_t)CSB_V1_STEAL_XP_TOTAL_C10;
    }
    if (share == 0) return;
    for (champion_index = first_index;
         champion_index < last_index;
         ++champion_index) {
        CSB_V1_Champion *champion =
            &profile->party_state.Champions[champion_index];
        unsigned int base_skill;
        uint32_t temp_gain;

        if (!leader_only && champion->CurrentHealth == 0) {
            continue; /* F0269:1071-1073 CurrentHealth guard */
        }
        champion->SkillExperience[CSB_V1_SKILL_STEAL_C08] += share;
        if (champion->SkillTemporaryExperience[CSB_V1_SKILL_STEAL_C08] <
            CSB_V1_TEMPORARY_EXPERIENCE_CAP) {
            /* F0304:888-890 F0026 bounded (1, share>>3, 100) */
            temp_gain = share >> 3;
            if (temp_gain < 1u) temp_gain = 1u;
            if (temp_gain > 100u) temp_gain = 100u;
            champion->SkillTemporaryExperience[CSB_V1_SKILL_STEAL_C08] +=
                (int16_t)temp_gain;
        }
        /* F0304:875-877,891-893 — hidden skill C08 credits base skill
         * (C08-C04)>>2 = C01 Ninja with the same share. */
        base_skill = (unsigned int)(CSB_V1_SKILL_STEAL_C08 - 4) >> 2;
        champion->SkillExperience[base_skill] += share;
    }
}

/* Restored 2026-07-18 after the worktree merge drift (df88dbda4/
 * a192cb2b0) clobbered the CSBWin Timer.cpp SetTimer
 * deleteDuplicateTimers==0 preservation chain: the shared M10
 * dm1v1_event_add merge policy must never collapse distinct saved
 * map TIMER slots.  Source-lock: CSBWin Timer.cpp:728-772 heap
 * ordering + SetTimer:967-1007 duplicate policy. */
static int csb_v1_runtime_event_is_before(
    const struct DM1_Event_V1 *a, int index_a,
    const struct DM1_Event_V1 *b, int index_b)
{
    uint32_t time_a = DM1_MAP_TIME_TIME(a->map_time);
    uint32_t time_b = DM1_MAP_TIME_TIME(b->map_time);
    uint16_t type_priority_a;
    uint16_t type_priority_b;

    if (time_a != time_b) return time_a < time_b;
    type_priority_a = (uint16_t)((uint16_t)a->type << 8) | a->priority;
    type_priority_b = (uint16_t)((uint16_t)b->type << 8) | b->priority;
    if (type_priority_a != type_priority_b) {
        return type_priority_a > type_priority_b;
    }
    return index_a <= index_b;
}

static void csb_v1_runtime_fix_unmerged_timer_placement(
    struct DM1_EventQueue_V1 *queue, int timeline_index)
{
    int event_index;
    int parent_index;
    int child_index;
    int half_index;
    int moved_up = 0;

    if (!queue || queue->eventCount <= 1) return;
    event_index = queue->timeline[timeline_index];
    while (timeline_index > 0) {
        parent_index = (timeline_index - 1) >> 1;
        if (!csb_v1_runtime_event_is_before(
                &queue->events[event_index], event_index,
                &queue->events[queue->timeline[parent_index]],
                queue->timeline[parent_index])) {
            break;
        }
        queue->timeline[timeline_index] = queue->timeline[parent_index];
        timeline_index = parent_index;
        moved_up = 1;
    }
    if (moved_up) {
        queue->timeline[timeline_index] = (uint16_t)event_index;
        return;
    }
    half_index = (queue->eventCount - 2) >> 1;
    while (timeline_index <= half_index) {
        child_index = (timeline_index << 1) + 1;
        if (child_index + 1 < queue->eventCount &&
            csb_v1_runtime_event_is_before(
                &queue->events[queue->timeline[child_index + 1]],
                queue->timeline[child_index + 1],
                &queue->events[queue->timeline[child_index]],
                queue->timeline[child_index])) {
            ++child_index;
        }
        if (!csb_v1_runtime_event_is_before(
                &queue->events[queue->timeline[child_index]],
                queue->timeline[child_index], &queue->events[event_index],
                event_index)) {
            break;
        }
        queue->timeline[timeline_index] = queue->timeline[child_index];
        timeline_index = child_index;
    }
    queue->timeline[timeline_index] = (uint16_t)event_index;
}

static int csb_v1_runtime_append_unmerged_map_timer_to_queue(
    struct DM1_EventQueue_V1 *queue,
    const struct DM1_Event_V1 *event)
{
    int index;
    int position;

    if (!queue || !event) return -1;
    if (queue->eventCount >= queue->maxEvents) return -1;
    index = queue->firstUnusedIndex;
    if (index < 0 || index >= queue->maxEvents) return -1;

    /* CSBWin Timer.cpp SetTimer's `deleteDuplicateTimers == 0` branch owns
     * a distinct map TIMER even when the shared DM1 F0238 helper would merge
     * it. Preserve that original slot and only reuse the common heap ordering
     * primitive, not the common merge policy. */
    queue->events[index] = *event;
    do {
        ++queue->firstUnusedIndex;
    } while (queue->firstUnusedIndex < queue->maxEvents &&
             queue->events[queue->firstUnusedIndex].type != DM1_EVENT_NONE);
    position = queue->eventCount;
    queue->timeline[position] = (uint16_t)index;
    ++queue->eventCount;
    csb_v1_runtime_fix_unmerged_timer_placement(queue, position);
    return index;
}

static int csb_v1_runtime_append_unmerged_map_timer(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_Event_V1 *event)
{
    if (!profile) return -1;
    return csb_v1_runtime_append_unmerged_map_timer_to_queue(
        &profile->timeline_queue, event);
}

static int csb_v1_runtime_square_contains_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int level,
    int map_x,
    int map_y)
{
    int current;
    int guard;
    if (!dungeon || level < 0 || level >= dungeon->level_count) return 0;
    current = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    for (guard = 0;
         guard < 128 && current >= 0 && current != 0xFFFE && current != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        if ((uint16_t)current == thing) return 1;
        record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)current, NULL, NULL, NULL);
        if (!record) break;
        current = (int)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
    }
    return 0;
}

static uint8_t *csb_v1_runtime_square_byte_ptr(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y,
    int *out_square_type)
{
    CSB_V1_DungeonData *dungeon;
    int offset;

    if (out_square_type) *out_square_type = -1;
    if (!profile || !profile->dungeon_handle) return NULL;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1) return NULL;
    if (level < 0 || level >= dungeon->level_count) return NULL;
    if (map_x < 0 || map_x >= dungeon->level_widths[level] ||
        map_y < 0 || map_y >= dungeon->level_heights[level]) {
        return NULL;
    }
    offset = dungeon->level_offsets[level] +
             map_x * dungeon->level_heights[level] +
             map_y;
    if (offset < 0 || offset >= dungeon->raw_size) return NULL;
    if (out_square_type) *out_square_type = (dungeon->raw_data[offset] >> 5) & 0x07;
    return &dungeon->raw_data[offset];
}

static void csb_v1_runtime_projectile_step(int direction, int *out_dx, int *out_dy)
{
    int dx = 0;
    int dy = 0;
    switch (direction & 3) {
    case 0: dy = -1; break;
    case 1: dx = 1; break;
    case 2: dy = 1; break;
    case 3: dx = -1; break;
    default: break;
    }
    if (out_dx) *out_dx = dx;
    if (out_dy) *out_dy = dy;
}

static int csb_v1_runtime_projectile_teleporter_scope_allows(int scope)
{
    /* ReDMCSB MOVESENS.C F0267 lines 450-482 gives non-party,
     * non-group things a combined creatures|objects-or-party requirement,
     * then rejects only creature-only teleporters for non-groups.  C14
     * projectiles therefore use object/party-capable teleporters, not the
     * party-only scope test used by normal C003..C006 movement. */
    return scope != 0x01;
}

static void csb_v1_runtime_schedule_projectile_move_event(
    CSB_V1_RuntimeProfile *profile,
    const struct TimelineEvent_Compat *event)
{
    struct DM1_Event_V1 dm1_event;

    if (!profile || !event) return;
    if (event->kind != TIMELINE_EVENT_PROJECTILE_MOVE) return;
    if (event->aux0 < 0 || event->aux0 > 255) return;

    memset(&dm1_event, 0, sizeof(dm1_event));
    dm1_event.map_time = DM1_MAP_TIME_MAKE(
        event->mapIndex,
        event->fireAtTick);
    dm1_event.type = DM1_EVENT_MOVE_PROJECTILE;
    dm1_event.priority = (uint8_t)event->aux0;
    dm1_event.b_mapX = (uint8_t)event->mapX;
    dm1_event.b_mapY = (uint8_t)event->mapY;
    dm1_event.c_cell = (uint8_t)(event->cell & 3);
    dm1_event.c_effect = (uint8_t)(event->aux3 & 0xFF);
    (void)dm1v1_event_add(&profile->timeline_queue, &dm1_event);
}

static void csb_v1_runtime_schedule_explosion_advance_event(
    CSB_V1_RuntimeProfile *profile,
    const struct TimelineEvent_Compat *event)
{
    struct DM1_Event_V1 dm1_event;

    if (!profile || !event) return;
    if (event->kind != TIMELINE_EVENT_EXPLOSION_ADVANCE) return;
    if (event->aux0 < 0 || event->aux0 > 255) return;

    memset(&dm1_event, 0, sizeof(dm1_event));
    dm1_event.map_time = DM1_MAP_TIME_MAKE(
        event->mapIndex,
        event->fireAtTick);
    dm1_event.type = DM1_EVENT_EXPLOSION;
    dm1_event.priority = (uint8_t)event->aux0;
    dm1_event.b_mapX = (uint8_t)event->mapX;
    dm1_event.b_mapY = (uint8_t)event->mapY;
    dm1_event.c_cell = (uint8_t)(event->cell & 0xFF);
    dm1_event.c_effect = (uint8_t)(event->aux1 & 0xFF);
    (void)dm1v1_event_add(&profile->timeline_queue, &dm1_event);
}

static int csb_v1_runtime_projectile_instance_active(
    const struct ProjectileInstance_Compat *projectile)
{
    return projectile &&
           projectile->slotIndex >= 0 &&
           projectile->reserved3 != 0;
}

static int csb_v1_runtime_explosion_instance_active(
    const struct ExplosionInstance_Compat *explosion)
{
    return explosion &&
           explosion->slotIndex >= 0 &&
           explosion->reserved0 != 0;
}

static int csb_v1_runtime_square_type_from_raw(
    const CSB_V1_DungeonData *dungeon,
    int raw_square)
{
    if (!dungeon || raw_square < 0) return PROJECTILE_ELEMENT_WALL;
    return (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
}

static int csb_v1_runtime_projectile_resolve_teleporter_chain(
    const CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    int start_map_index,
    int start_map_x,
    int start_map_y,
    int base_cell,
    int *out_map_index,
    int *out_map_x,
    int *out_map_y,
    int *out_direction,
    int *out_cell)
{
    const CSB_V1_DungeonData *dungeon;
    int map_index;
    int map_x;
    int map_y;
    int direction;
    int cell;
    int i;
    int applied = 0;

    if (out_map_index) *out_map_index = start_map_index;
    if (out_map_x) *out_map_x = start_map_x;
    if (out_map_y) *out_map_y = start_map_y;
    if (out_direction) {
        *out_direction = projectile ? (projectile->direction & 3) : -1;
    }
    if (out_cell) *out_cell = base_cell;
    if (!profile || !projectile || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->level_count <= 0) return 0;

    map_index = start_map_index;
    map_x = start_map_x;
    map_y = start_map_y;
    direction = projectile->direction & 3;
    cell = (base_cell >= 0) ? (base_cell & 3) : 0;

    /* ReDMCSB MOVESENS.C F0267 lines 466-530 chains teleporters up to the
     * PC34/I34E MEDIA529 100-step cap.  F0264 lines 148-170 marks C14
     * projectiles as levitating, so pits are intentionally not part of this
     * projectile chain helper. */
    for (i = 0; i < 100; ++i) {
        CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
        CSB_V1_TeleporterRotationRuntimeProjectileResultPc34 teleporter_result;
        uint16_t projectile_thing;
        int raw_square;
        int scope = 0;
        int self_target;

        if (map_index < 0 || map_index >= dungeon->level_count) break;
        raw_square = csb_v1_dungeon_get_raw_square(
            dungeon,
            map_index,
            map_x,
            map_y);
        if (raw_square < 0 ||
            csb_v1_runtime_square_type_from_raw(dungeon, raw_square) !=
                PROJECTILE_ELEMENT_TELEPORTER ||
            (raw_square & 0x08) == 0) {
            break;
        }
        if (csb_v1_runtime_decode_teleporter_at_square(
                dungeon,
                map_index,
                map_x,
                map_y,
                raw_square,
                &teleporter,
                &scope) <= 0) {
            break;
        }
        if (!csb_v1_runtime_projectile_teleporter_scope_allows(scope) ||
            teleporter.target_map_index < 0 ||
            teleporter.target_map_index >= dungeon->level_count) {
            break;
        }

        projectile_thing = (uint16_t)(((cell & 3) << 14) |
                                      (14u << 10) |
                                      (uint16_t)(projectile->slotIndex &
                                                 0x03FF));
        if (csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
                &teleporter,
                projectile_thing,
                direction,
                &teleporter_result) != 0) {
            break;
        }

        self_target = teleporter.target_map_index == map_index &&
                      teleporter.target_map_x == map_x &&
                      teleporter.target_map_y == map_y;
        map_index = teleporter.target_map_index;
        map_x = teleporter.target_map_x;
        map_y = teleporter.target_map_y;
        direction = teleporter_result.direction & 3;
        cell = csb_v1_teleporter_rotation_thing_cell_pc34_compat(
            teleporter_result.thing) & 3;
        applied++;
        if (self_target) break;
    }

    if (!applied) return 0;
    if (out_map_index) *out_map_index = map_index;
    if (out_map_x) *out_map_x = map_x;
    if (out_map_y) *out_map_y = map_y;
    if (out_direction) *out_direction = direction;
    if (out_cell) *out_cell = (base_cell >= 0) ? cell : base_cell;
    return applied;
}

static int csb_v1_runtime_party_champion_cell_mask(
    const CSB_V1_RuntimeProfile *profile)
{
    int i;
    int mask = 0;

    if (!profile || !profile->party_state_valid) return 0;

    /* ReDMCSB: MOVESENS.C F0266 lines 241-247 fills the projectile
     * impact cell table from F0285_CHAMPION_GetIndexInCell(), so empty
     * party cells must not be exposed as champion impact targets. */
    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; ++i) {
        const CSB_V1_Champion *champion = &profile->party_state.Champions[i];
        if (csb_v1_champion_is_dead(champion) ||
            champion->CurrentHealth <= 0) {
            continue;
        }
        mask |= 1 << ((int)champion->Cell & 3);
    }
    return mask;
}

static int csb_v1_runtime_build_projectile_digest(
    const CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    int projectile_index,
    struct CellContentDigest_Compat *out)
{
    const CSB_V1_DungeonData *dungeon;
    int raw_square;
    int dest_raw_square;
    int dx;
    int dy;
    int dest_x;
    int dest_y;
    int i;

    if (!profile || !projectile || !out || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data ||
        projectile->mapIndex < 0 ||
        projectile->mapIndex >= dungeon->level_count) {
        return 0;
    }

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        projectile->mapIndex,
        projectile->mapX,
        projectile->mapY);
    if (raw_square < 0) return 0;

    csb_v1_runtime_projectile_step(projectile->direction, &dx, &dy);
    dest_x = projectile->mapX + dx;
    dest_y = projectile->mapY + dy;

    memset(out, 0, sizeof(*out));
    out->sourceMapIndex = projectile->mapIndex;
    out->sourceMapX = projectile->mapX;
    out->sourceMapY = projectile->mapY;
    out->sourceSquareType =
        csb_v1_runtime_square_type_from_raw(dungeon, raw_square);
    out->destTeleporterNewDirection = -1;
    out->destDoorState = PROJECTILE_DOOR_STATE_NONE;

    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat *other =
            &profile->projectiles.entries[i];
        if (i == projectile_index ||
            !csb_v1_runtime_projectile_instance_active(other)) {
            continue;
        }
        if (other->mapIndex == projectile->mapIndex &&
            other->mapX == projectile->mapX &&
            other->mapY == projectile->mapY &&
            other->cell == projectile->cell) {
            out->sourceHasOtherProjectile = 1;
            break;
        }
    }

    out->destMapIndex = projectile->mapIndex;
    out->destMapX = dest_x;
    out->destMapY = dest_y;
    dest_raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        projectile->mapIndex,
        dest_x,
        dest_y);
    if (dest_raw_square < 0) {
        out->destIsMapBoundary = 1;
        out->destSquareType = PROJECTILE_ELEMENT_WALL;
        return 1;
    }

    out->destSquareType =
        csb_v1_runtime_square_type_from_raw(dungeon, dest_raw_square);
    if (out->destSquareType == PROJECTILE_ELEMENT_TELEPORTER &&
        (dest_raw_square & 0x08) != 0) {
        int chain_map_index = projectile->mapIndex;
        int chain_x = dest_x;
        int chain_y = dest_y;
        int chain_direction = projectile->direction;
        if (csb_v1_runtime_projectile_resolve_teleporter_chain(
                profile,
                projectile,
                projectile->mapIndex,
                dest_x,
                dest_y,
                -1,
                &chain_map_index,
                &chain_x,
                &chain_y,
                &chain_direction,
                NULL) > 0) {
            out->destTeleporterNewDirection = chain_direction;
            out->destMapIndex = chain_map_index;
            out->destMapX = chain_x;
            out->destMapY = chain_y;
            dest_x = out->destMapX;
            dest_y = out->destMapY;
            dest_raw_square = csb_v1_dungeon_get_raw_square(
                dungeon,
                out->destMapIndex,
                dest_x,
                dest_y);
            if (dest_raw_square >= 0) {
                out->destSquareType =
                    csb_v1_runtime_square_type_from_raw(
                        dungeon,
                        dest_raw_square);
            }
        }
    }
    if (out->destSquareType == PROJECTILE_ELEMENT_FAKEWALL) {
        out->destFakeWallIsImaginaryOrOpen =
            (dest_raw_square & 0x05) ? 1 : 0;
    }
    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int door_state = dest_raw_square & 0x07;
        if (door_state == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (door_state <= 4) {
            out->destDoorState = door_state;
        } else if (door_state == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
        }
        out->destDoorAllowsProjectilePassThrough = 0;
    }
    /* F0219 commits the C14 to F0267's resolved destination before checking
     * champions, groups, or another C14.  A C05 chain may therefore change
     * map as well as coordinates; never inspect the launcher/source map
     * after `out->destMapIndex` has been resolved. */
    if (profile->current_level == out->destMapIndex &&
        profile->party_x == dest_x &&
        profile->party_y == dest_y) {
        out->destChampionCellMask =
            csb_v1_runtime_party_champion_cell_mask(profile);
        out->destHasChampion = out->destChampionCellMask ? 1 : 0;
        out->destPartyDirection = profile->party_dir & 3;
    }
    {
        uint16_t group_thing;
        int thing_type = -1;
        int thing_size = 0;
        const uint8_t *group;

        /* ReDMCSB GROUP1.C F0175 walks the complete source Thing chain.
         * A C01 teleporter/actuator may precede C04 on a real target square. */
        if (csb_v1_f0217_find_group_thing_pc34_compat(
                dungeon, out->destMapIndex, dest_x, dest_y, &group_thing)) {
            group = csb_v1_dungeon_get_thing_record(
                dungeon, group_thing, &thing_type, NULL, &thing_size);
            out->destHasCreatureGroup = 1;
            out->destCreatureType =
                (group && thing_type == 4 && thing_size > 4) ? group[4] : 0;
            out->destCreatureCellMask = 0x0F;
            {
                const struct CreatureBehaviorProfile_Compat *creature_profile =
                    CREATURE_GetProfile_Compat(out->destCreatureType);
                out->destCreatureIsNonMaterial =
                    creature_profile &&
                    ((creature_profile->attributes &
                      CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            }
        }
    }
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat *other =
            &profile->projectiles.entries[i];
        int new_cell;
        if (i == projectile_index ||
            !csb_v1_runtime_projectile_instance_active(other)) {
            continue;
        }
        if (other->mapIndex != out->destMapIndex ||
            other->mapX != dest_x ||
            other->mapY != dest_y) {
            continue;
        }
        if ((projectile->direction & 1) == (projectile->cell & 1)) {
            new_cell = (projectile->cell - 1) & 3;
        } else {
            new_cell = (projectile->cell + 1) & 3;
        }
        if (other->cell == new_cell) {
            out->destHasOtherProjectile = 1;
            break;
        }
    }
    return 1;
}

static int csb_v1_runtime_projectile_teleporter_rotated_cell(
    const CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    int base_cell,
    int *out_direction,
    int *out_cell)
{
    int dx;
    int dy;
    int dest_x;
    int dest_y;

    if (out_direction) *out_direction = -1;
    if (out_cell) *out_cell = -1;
    if (!profile || !projectile || !out_cell) {
        return 0;
    }
    csb_v1_runtime_projectile_step(projectile->direction, &dx, &dy);
    dest_x = projectile->mapX + dx;
    dest_y = projectile->mapY + dy;
    return csb_v1_runtime_projectile_resolve_teleporter_chain(
        profile,
        projectile,
        projectile->mapIndex,
        dest_x,
        dest_y,
        base_cell,
        NULL,
        NULL,
        NULL,
        out_direction,
        out_cell) > 0;
}

static int csb_v1_runtime_build_explosion_digest(
    const CSB_V1_RuntimeProfile *profile,
    const struct ExplosionInstance_Compat *explosion,
    struct CellContentDigest_Compat *out)
{
    const CSB_V1_DungeonData *dungeon;
    int raw_square;

    if (!profile || !explosion || !out || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data ||
        explosion->mapIndex < 0 ||
        explosion->mapIndex >= dungeon->level_count) {
        return 0;
    }

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        explosion->mapIndex,
        explosion->mapX,
        explosion->mapY);
    if (raw_square < 0) return 0;

    memset(out, 0, sizeof(*out));
    out->sourceMapIndex = explosion->mapIndex;
    out->sourceMapX = explosion->mapX;
    out->sourceMapY = explosion->mapY;
    out->sourceSquareType =
        csb_v1_runtime_square_type_from_raw(dungeon, raw_square);
    out->destMapIndex = explosion->mapIndex;
    out->destMapX = explosion->mapX;
    out->destMapY = explosion->mapY;
    out->destSquareType = out->sourceSquareType;
    out->destTeleporterNewDirection = -1;
    out->destDoorState = PROJECTILE_DOOR_STATE_NONE;

    if (out->destSquareType == PROJECTILE_ELEMENT_FAKEWALL) {
        out->destFakeWallIsImaginaryOrOpen =
            (raw_square & 0x05) ? 1 : 0;
    }
    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int door_state = raw_square & 0x07;
        if (door_state == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (door_state <= 4) {
            out->destDoorState = door_state;
        } else if (door_state == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
        }
        out->destDoorAllowsProjectilePassThrough = 0;
    }
    if (profile->current_level == explosion->mapIndex &&
        profile->party_x == explosion->mapX &&
        profile->party_y == explosion->mapY) {
        out->destChampionCellMask =
            csb_v1_runtime_party_champion_cell_mask(profile);
        out->destHasChampion = out->destChampionCellMask ? 1 : 0;
        out->destPartyDirection = profile->party_dir & 3;
    }
    {
        int first_thing = csb_v1_dungeon_get_first_thing(
            dungeon,
            explosion->mapIndex,
            explosion->mapX,
            explosion->mapY);
        if (first_thing >= 0 &&
            ((first_thing >> 10) & 0x0F) == 4) {
            int thing_type = -1;
            int thing_index = -1;
            int thing_size = 0;
            const uint8_t *group = csb_v1_dungeon_get_thing_record(
                dungeon,
                first_thing,
                &thing_type,
                &thing_index,
                &thing_size);
            out->destHasCreatureGroup = 1;
            out->destCreatureType =
                (group && thing_type == 4 && thing_size > 4) ? group[4] : 0;
            out->destCreatureCellMask = 0x0F;
            {
                const struct CreatureBehaviorProfile_Compat *creature_profile =
                    CREATURE_GetProfile_Compat(out->destCreatureType);
                out->destCreatureIsNonMaterial =
                    creature_profile &&
                    ((creature_profile->attributes &
                      CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            }
            (void)thing_index;
        }
    }
    return 1;
}

static void csb_v1_runtime_apply_projectile_move_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    struct ProjectileInstance_Compat *projectile;
    struct ProjectileInstance_Compat new_state;
    struct ProjectileTickResult_Compat tick_result;
    struct CellContentDigest_Compat digest;
    struct RngState_Compat rng;
    int slot;

    if (!profile || !record) return;
    slot = record->aux0;
    if (slot < 0 || slot >= PROJECTILE_LIST_CAPACITY) return;
    projectile = &profile->projectiles.entries[slot];
    if (!csb_v1_runtime_projectile_instance_active(projectile)) return;
    if (!csb_v1_runtime_build_projectile_digest(
            profile,
            projectile,
            slot,
            &digest)) {
        (void)F0813_PROJECTILE_Despawn_Compat(&profile->projectiles, slot);
        return;
    }

    /* ReDMCSB PROJEXPL.C F0219 is dispatched from TIMELINE.C F0261 C48/C49.
     * CSB's bounded runtime delegates the source projectile motion math to
     * M10 F0811, then owns only CSB real-format byte-map digesting and event
     * requeueing. */
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)projectile->mapX << 8) ^
            ((uint32_t)projectile->mapY << 16) ^
            (uint32_t)(slot << 24));
    if (!F0811_PROJECTILE_Advance_Compat(
            projectile,
            &digest,
            profile->game_time,
            &rng,
            &new_state,
            &tick_result)) {
        (void)F0813_PROJECTILE_Despawn_Compat(&profile->projectiles, slot);
        return;
    }
    if (tick_result.emittedExplosion) {
        struct ExplosionCreateInput_Compat explosion_input;
        struct TimelineEvent_Compat first_advance;
        int explosion_slot = -1;
        memset(&explosion_input, 0, sizeof(explosion_input));
        memset(&first_advance, 0, sizeof(first_advance));
        explosion_input.explosionType = tick_result.outExplosion.explosionType;
        explosion_input.attack = tick_result.outExplosion.attack;
        explosion_input.mapIndex = tick_result.outExplosion.mapIndex;
        explosion_input.mapX = tick_result.outExplosion.mapX;
        explosion_input.mapY = tick_result.outExplosion.mapY;
        explosion_input.cell = tick_result.outExplosion.cell;
        explosion_input.centered = tick_result.outExplosion.centered;
        explosion_input.poisonAttack = tick_result.outExplosion.poisonAttack;
        explosion_input.currentTick = (int)profile->game_time;
        explosion_input.ownerKind = tick_result.outExplosion.ownerKind;
        explosion_input.ownerIndex = tick_result.outExplosion.ownerIndex;
        explosion_input.creatorProjectileSlot =
            tick_result.outExplosion.creatorProjectileSlot;
        if (F0821_EXPLOSION_Create_Compat(
                &explosion_input,
                &profile->explosions,
                &explosion_slot,
                &first_advance)) {
            csb_v1_runtime_schedule_explosion_advance_event(
                profile,
                &first_advance);
        }
    }
    if (tick_result.emittedCombatAction &&
        tick_result.outAction.kind == COMBAT_ACTION_APPLY_DAMAGE_GROUP) {
        (void)csb_v1_runtime_apply_projectile_group_action(
            profile,
            &tick_result.outAction,
            projectile);
    }
    if (!tick_result.despawn &&
        tick_result.resultKind == PROJECTILE_RESULT_FLEW &&
        digest.destTeleporterNewDirection >= 0) {
        int rotated_direction = -1;
        int rotated_cell = -1;
        if (csb_v1_runtime_projectile_teleporter_rotated_cell(
                profile,
                projectile,
                new_state.cell,
                &rotated_direction,
                &rotated_cell)) {
            if (rotated_direction >= 0) {
                new_state.direction = rotated_direction & 3;
                tick_result.newDirection = new_state.direction;
            }
            new_state.cell = rotated_cell & 3;
            tick_result.newCell = new_state.cell;
            tick_result.outNextTick.cell = new_state.cell;
        }
    }
    if (tick_result.despawn) {
        (void)csb_v1_runtime_materialize_projectile_associated_object(
            profile,
            projectile,
            &tick_result);
        (void)F0813_PROJECTILE_Despawn_Compat(&profile->projectiles, slot);
        return;
    }
    *projectile = new_state;
    if (tick_result.resultKind == PROJECTILE_RESULT_FLEW &&
        digest.destTeleporterNewDirection >= 0) {
        size_t receipt_index = profile->post_teleport_projectile_count;
        CSB_V1_RuntimePostTeleportProjectileReceiptPc34 *receipt;

        /* A C05 chain reached this exact resolved C14 state.  Keep only the
         * runtime identity; F0128 later validates the original raw C14 list
         * link before admitting its F0115 material. */
        if (receipt_index >= CSB_V1_RUNTIME_POST_TELEPORT_PROJECTILE_MAX_PC34) {
            receipt_index = CSB_V1_RUNTIME_POST_TELEPORT_PROJECTILE_MAX_PC34 - 1u;
        } else {
            ++profile->post_teleport_projectile_count;
        }
        receipt = &profile->post_teleport_projectiles[receipt_index];
        memset(receipt, 0, sizeof(*receipt));
        receipt->valid = 1;
        receipt->projectile_slot = slot;
        receipt->map_index = new_state.mapIndex;
        receipt->map_x = new_state.mapX;
        receipt->map_y = new_state.mapY;
        receipt->cell = new_state.cell & 3;
        receipt->game_time = profile->game_time;
    }
    if (tick_result.resultKind == PROJECTILE_RESULT_FLEW) {
        csb_v1_runtime_schedule_projectile_move_event(
            profile,
            &tick_result.outNextTick);
    }
    if (tick_result.emittedDoorDestructionEvent ||
        tick_result.emittedDoorToggleEvent) {
        struct DM1_Event_V1 door_event;
        memset(&door_event, 0, sizeof(door_event));
        door_event.map_time = DM1_MAP_TIME_MAKE(
            tick_result.outNextTick.mapIndex,
            tick_result.outNextTick.fireAtTick);
        door_event.type = tick_result.emittedDoorDestructionEvent
            ? DM1_EVENT_DOOR_DESTRUCTION
            : DM1_EVENT_DOOR;
        door_event.b_mapX = (uint8_t)tick_result.outNextTick.mapX;
        door_event.b_mapY = (uint8_t)tick_result.outNextTick.mapY;
        door_event.c_cell = (uint8_t)(tick_result.outNextTick.cell & 3);
        door_event.c_effect = (uint8_t)tick_result.outNextTick.aux0;
        (void)dm1v1_event_add(&profile->timeline_queue, &door_event);
    }
}

static void csb_v1_runtime_apply_explosion_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    struct ExplosionInstance_Compat *explosion;
    struct ExplosionInstance_Compat new_state;
    struct ExplosionTickResult_Compat tick_result;
    struct CellContentDigest_Compat digest;
    struct RngState_Compat rng;
    int slot;

    if (!profile || !record) return;
    slot = record->aux0;
    if (slot < 0 || slot >= EXPLOSION_LIST_CAPACITY) return;
    explosion = &profile->explosions.entries[slot];
    if (!csb_v1_runtime_explosion_instance_active(explosion)) return;
    /* F0213 creates C25 from the exact C15 explosion Thing, location and
     * scheduled tick.  The compact runtime queue keeps that identity as the
     * slot plus C25 location/type fields; reject stale or aliased C25 records
     * before touching an active recycled explosion slot. */
    if (record->eventType != DM1_EVENT_EXPLOSION ||
        record->mapIndex != explosion->mapIndex ||
        record->mapX != explosion->mapX ||
        record->mapY != explosion->mapY ||
        record->cell != (explosion->cell & 0xFF) ||
        record->effect != explosion->explosionType ||
        (int)profile->game_time != explosion->scheduledAtTick ||
        !csb_v1_runtime_square_byte_ptr(
            profile, explosion->mapIndex, explosion->mapX, explosion->mapY,
            NULL)) {
        return;
    }
    if (!csb_v1_runtime_build_explosion_digest(
            profile,
            explosion,
            &digest)) {
        (void)F0824_EXPLOSION_Despawn_Compat(&profile->explosions, slot);
        return;
    }

    /* ReDMCSB PROJEXPL.C F0220 is dispatched from TIMELINE.C F0261 C25.
     * CSB keeps real-format byte-map lookup here and delegates explosion
     * frame/attack/lifecycle parity to the shared M10 F0822 mirror. */
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)explosion->mapX << 8) ^
            ((uint32_t)explosion->mapY << 16) ^
            (uint32_t)(slot << 24));
    if (!F0822_EXPLOSION_Advance_Compat(
            explosion,
            &digest,
            profile->game_time,
            &rng,
            &new_state,
            &tick_result)) {
        (void)F0824_EXPLOSION_Despawn_Compat(&profile->explosions, slot);
        return;
    }

    if (tick_result.emittedCombatActionPartyCount > 0) {
        (void)csb_v1_runtime_apply_explosion_party_action(
            profile,
            &tick_result.outActionParty,
            &rng);
    }
    if (tick_result.emittedCombatActionGroupCount > 0) {
        (void)csb_v1_runtime_apply_explosion_group_action(
            profile,
            &tick_result.outActionGroup,
            &rng);
    }

    if (tick_result.emittedDoorDestructionEvent) {
        struct DM1_Event_V1 door_event;
        memset(&door_event, 0, sizeof(door_event));
        door_event.map_time = DM1_MAP_TIME_MAKE(
            tick_result.outNextTick.mapIndex,
            tick_result.outNextTick.fireAtTick);
        door_event.type = DM1_EVENT_DOOR_DESTRUCTION;
        door_event.priority = 0;
        door_event.b_mapX = (uint8_t)tick_result.outNextTick.mapX;
        door_event.b_mapY = (uint8_t)tick_result.outNextTick.mapY;
        door_event.c_cell = (uint8_t)(tick_result.outNextTick.cell & 0xFF);
        door_event.c_effect = 0;
        (void)dm1v1_event_add(&profile->timeline_queue, &door_event);
    }
    if (tick_result.despawn) {
        (void)F0824_EXPLOSION_Despawn_Compat(&profile->explosions, slot);
        return;
    }

    *explosion = new_state;
    if (tick_result.outNextTick.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE) {
        explosion->scheduledAtTick = (int)tick_result.outNextTick.fireAtTick;
        csb_v1_runtime_schedule_explosion_advance_event(
            profile,
            &tick_result.outNextTick);
    }
}

static void csb_v1_runtime_schedule_door_animation_followup(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int effect)
{
    struct DM1_Event_V1 event;

    if (!profile || !record) return;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        record->mapIndex,
        profile->game_time + 1u);
    event.type = DM1_EVENT_DOOR_ANIMATION;
    event.b_mapX = (uint8_t)record->mapX;
    event.b_mapY = (uint8_t)record->mapY;
    event.c_cell = (uint8_t)record->cell;
    event.c_effect = (uint8_t)effect;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_apply_door_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    uint8_t *square;
    int square_type;
    int door_state;
    int effect;
    int next_state;

    if (!profile || !record) return;
    square = csb_v1_runtime_square_byte_ptr(
        profile,
        record->mapIndex,
        record->mapX,
        record->mapY,
        &square_type);
    if (!square || square_type != 4) return;

    door_state = (int)(*square & 0x07u);
    if (door_state == 5) return;
    effect = record->effect;
    if (effect == DM1_EFFECT_TOGGLE) {
        effect = (door_state == 0) ? DM1_EFFECT_CLEAR : DM1_EFFECT_SET;
    }
    if (effect != DM1_EFFECT_SET && effect != DM1_EFFECT_CLEAR) return;
    if ((effect == DM1_EFFECT_SET && door_state == 0) ||
        (effect == DM1_EFFECT_CLEAR && door_state == 4)) {
        return;
    }

    next_state = door_state + ((effect == DM1_EFFECT_SET) ? -1 : 1);
    if (next_state < 0) next_state = 0;
    if (next_state > 4) next_state = 4;
    *square = (uint8_t)((*square & (uint8_t)~0x07u) | (uint8_t)next_state);
    if ((effect == DM1_EFFECT_SET && next_state != 0) ||
        (effect == DM1_EFFECT_CLEAR && next_state != 4)) {
        csb_v1_runtime_schedule_door_animation_followup(
            profile,
            record,
            effect);
    }
}

static int csb_v1_runtime_apply_saved_csbwin_door_animation_timer(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    CSB_V1_CSBWin512TimerSummary *timer,
    uint16_t timer_index,
    uint16_t queue_slot)
{
    struct DM1_Event_V1 next;
    uint8_t *square;
    int square_type;
    int door_state;
    int effect;
    int next_state;
    uint8_t staged_square;
    int event_index;
    CSB_V1_CSBWin512TimerSummary successor;

    /* CSBWin Timer.cpp ProcessTT_DOOR changes the saved TIMER to TT_1, then
     * TIMELINE.C F0241 owns every subsequent door-animation step. Retain the
     * same source TIMER and queue slot for those successors: a plain M10 C01
     * follow-up has no CSBWin restart receipt. */
    if (!profile || !record || !timer || !profile->dungeon_handle ||
        timer->function != 1u || !timer->valid || timer->truncated ||
        timer->source_index != timer_index ||
        record->eventType != timer->function ||
        record->mapIndex != timer->level || record->mapX != timer->ubyte6 ||
        record->mapY != timer->ubyte7 || record->cell != timer->ubyte8 ||
        record->effect != timer->ubyte9 || record->aux0 != timer->ubyte5 ||
        timer->ubyte9 > 1u || profile->game_time >= 0x00ffffffu) {
        return 0;
    }
    square = csb_v1_runtime_square_byte_ptr(
        profile, timer->level, timer->ubyte6, timer->ubyte7, &square_type);
    if (!square || square_type != 4) return 0;

    door_state = *square & 0x07u;
    effect = timer->ubyte9;
    if (door_state == 5 ||
        (effect == DM1_EFFECT_SET && door_state == 0) ||
        (effect == DM1_EFFECT_CLEAR && door_state == 4)) {
        return 1;
    }

    if (effect == DM1_EFFECT_CLEAR && timer->level == profile->current_level &&
        timer->ubyte6 == profile->party_x && timer->ubyte7 == profile->party_y &&
        profile->party_state_valid && profile->party_state.ChampionCount > 0) {
        /* The source damage + sound transaction cannot yet roll back with
         * the TIMER pool. Do not publish a partial door/timer successor. */
        return 0;
    } else {
        next_state = door_state + (effect == DM1_EFFECT_SET ? -1 : 1);
        staged_square = (uint8_t)((*square & (uint8_t)~0x07u) |
                                  (uint8_t)next_state);
    }

    if ((effect == DM1_EFFECT_SET && (staged_square & 0x07u) == 0u) ||
        (effect == DM1_EFFECT_CLEAR && (staged_square & 0x07u) == 4u)) {
        *square = staged_square;
        return 1;
    }
    memset(&next, 0, sizeof(next));
    next.map_time = DM1_MAP_TIME_MAKE(timer->level, profile->game_time + 1u);
    next.type = 1u;
    next.priority = timer->ubyte5;
    next.b_mapX = timer->ubyte6;
    next.b_mapY = timer->ubyte7;
    next.c_cell = timer->ubyte8;
    next.c_effect = timer->ubyte9;
    event_index = csb_v1_runtime_add_timeline_event(profile, &next);
    if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) return 0;
    successor = *timer;
    successor.time = profile->game_time + 1u;
    if (!csb_v1_runtime_replace_dispatched_csbwin_timer(
            profile, queue_slot, timer_index, &successor, event_index)) {
        (void)dm1v1_event_delete(&profile->timeline_queue, event_index);
        return 0;
    }
    *square = staged_square;
    return 1;
}

static int csb_v1_runtime_square_has_material_group(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int thing;
    int guard;

    if (!dungeon) return -1;
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    /* A real square with no MASK0x0010 list has no C04 to block F0242. */
    if (thing < 0) return 0;
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        const uint8_t *thing_record;
        const struct CreatureBehaviorProfile_Compat *creature;
        int thing_type;
        int thing_size;

        thing_record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, &thing_type, NULL, &thing_size);
        if (!thing_record || thing_size < 2) return -1;
        if (thing_type == 4) {
            if (thing_size < 5) return -1;
            creature = CREATURE_GetProfile_Compat(thing_record[4]);
            if (!creature ||
                (creature->attributes & CREATURE_ATTR_MASK_NON_MATERIAL) == 0) {
                return 1;
            }
        }
        thing = (int)csb_v1_runtime_read_u16(thing_record);
    }
    return guard >= 128 ? -1 : 0;
}

static void csb_v1_runtime_requeue_square_state_event(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    struct DM1_Event_V1 event;

    if (!profile || !record || profile->game_time >= 0x00ffffffu) return;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(record->mapIndex,
                                       profile->game_time + 1u);
    event.type = (uint8_t)record->eventType;
    event.priority = (uint8_t)record->aux0;
    event.b_mapX = (uint8_t)record->mapX;
    event.b_mapY = (uint8_t)record->mapY;
    event.c_cell = (uint8_t)record->cell;
    event.c_effect = (uint8_t)record->effect;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_apply_open_square_party_consequences(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_InputCommandRuntimeResult result;

    if (!profile || !record || !profile->party_state_valid ||
        profile->party_state.ChampionCount <= 0 ||
        profile->current_level != record->mapIndex ||
        profile->party_x != record->mapX || profile->party_y != record->mapY) {
        return;
    }
    memset(&result, 0, sizeof(result));
    result.movement_step_attempted = 1;
    result.movement_step_applied = 1;
    result.movement_destination_x = record->mapX;
    result.movement_destination_y = record->mapY;
    result.old_party_level = record->mapIndex;
    result.new_party_level = record->mapIndex;
    /* F0250/F0251 write OPEN before F0249 moves the party and resident
     * Things. The existing F0267 party chain consumes only this real square;
     * unsupported resident Thing categories remain untouched. */
    csb_v1_runtime_apply_destination_chain(profile, &result);
}

static void csb_v1_runtime_apply_square_state_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int expected_square_type,
    uint8_t open_mask)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *square;
    int square_type;
    int effect;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    square = csb_v1_runtime_square_byte_ptr(
        profile, record->mapIndex, record->mapX, record->mapY, &square_type);
    if (!square || square_type != expected_square_type) return;

    effect = record->effect;
    if (effect == DM1_EFFECT_TOGGLE) {
        effect = (*square & open_mask) ? DM1_EFFECT_CLEAR : DM1_EFFECT_SET;
    }
    if (effect != DM1_EFFECT_SET && effect != DM1_EFFECT_CLEAR) return;

    if (record->eventType == DM1_EVENT_FAKEWALL &&
        effect == DM1_EFFECT_CLEAR) {
        int group_state;
        int party_on_square = profile->party_state_valid &&
            profile->party_state.ChampionCount > 0 &&
            profile->current_level == record->mapIndex &&
            profile->party_x == record->mapX && profile->party_y == record->mapY;

        group_state = csb_v1_runtime_square_has_material_group(
            dungeon, record->mapIndex, record->mapX, record->mapY);
        if (party_on_square || group_state > 0) {
            /* F0242 defers a closing C07 by exactly one game tick while a
             * party or material C04 occupies the source square. */
            csb_v1_runtime_requeue_square_state_event(profile, record);
            return;
        }
        if (group_state < 0) return;
    }

    if (effect == DM1_EFFECT_SET) {
        *square = (uint8_t)(*square | open_mask);
        if (record->eventType == DM1_EVENT_TELEPORTER ||
            record->eventType == DM1_EVENT_PIT) {
            csb_v1_runtime_apply_open_square_party_consequences(profile, record);
        }
    } else {
        *square = (uint8_t)(*square & (uint8_t)~open_mask);
    }
}

static void csb_v1_runtime_schedule_enable_group_generator_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    uint32_t ticks)
{
    struct DM1_Event_V1 event;

    if (!profile || !record || ticks == 0u) return;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        record->mapIndex,
        profile->game_time + ticks);
    event.type = DM1_EVENT_ENABLE_GROUP_GENERATOR;
    event.b_mapX = (uint8_t)record->mapX;
    event.b_mapY = (uint8_t)record->mapY;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_materialize_corridor_generator_group(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int sensor_data,
    uint16_t flags_word,
    uint16_t local_word)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *group_record;
    uint8_t *first_thing_ptr;
    struct GeneratorContext_Compat ctx;
    struct GeneratorResult_Compat result;
    struct RngState_Compat rng;
    uint16_t previous_first;
    uint16_t group_thing;
    uint16_t group_flags;
    int group_index;
    int i;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    if (!csb_v1_runtime_find_unused_group_record(
            dungeon,
            &group_record,
            &group_index)) {
        return;
    }
    first_thing_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (!first_thing_ptr) return;

    memset(&ctx, 0, sizeof(ctx));
    memset(&result, 0, sizeof(result));
    ctx.mapIndex = record->mapIndex;
    ctx.mapX = record->mapX;
    ctx.mapY = record->mapY;
    ctx.creatureType = sensor_data;
    ctx.creatureCountRaw = (int)((flags_word >> 7) & 0x0Fu);
    ctx.randomizeCount = (ctx.creatureCountRaw & 0x08) ? 1 : 0;
    ctx.healthMultiplier = (int)(local_word & 0x000Fu);
    ctx.ticksRaw = (int)(local_word >> 4);
    ctx.onceOnly = (int)((flags_word >> 2) & 0x01u);
    ctx.audible = (int)((flags_word >> 6) & 0x01u);
    ctx.mapDifficulty = 1;
    ctx.isOnPartyMap = (record->mapIndex == profile->current_level) ? 1 : 0;
    ctx.currentActiveGroupCount = 0;
    ctx.maxActiveGroupCount = 60;

    /* ReDMCSB TIMELINE.C F0245 lines 970-978 calls GROUP.C F0185 with
     * sensor data, health multiplier, count, random direction, and square
     * coordinates.  Firestaff's M10 F0860 owns the source-locked
     * count/random/health/cell calculation; this bridge binds its result to
     * CSB real-format C04 group slots and square-first-thing linkage. */
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)record->mapX << 8) ^ ((uint32_t)record->mapY << 16));
    if (!F0860_RUNTIME_HandleGroupGenerator_Compat(
            &ctx,
            &rng,
            profile->game_time,
            &result) ||
        !result.spawned) {
        return;
    }

    previous_first = csb_v1_runtime_read_u16(first_thing_ptr);
    group_thing = (uint16_t)((4u << 10) | (uint16_t)(group_index & 0x03FF));
    csb_v1_runtime_write_u16(group_record + 0, previous_first);
    csb_v1_runtime_write_u16(group_record + 2, 0xFFFEu);
    group_record[4] = (uint8_t)(result.spawnedCreatureType & 0xFF);
    group_record[5] = (uint8_t)(result.spawnedGroupCells & 0xFF);
    for (i = 0; i < 4; ++i) {
        int hp = result.spawnedGroupHealth[i];
        if (hp < 0) hp = 0;
        if (hp > 0xFFFF) hp = 0xFFFF;
        csb_v1_runtime_write_u16(
            group_record + 6 + i * 2,
            (uint16_t)hp);
    }
    group_flags = (uint16_t)(((result.spawnedCreatureCount & 0x03) << 5) |
                             ((result.spawnedDirection & 0x03) << 8));
    csb_v1_runtime_write_u16(group_record + 14, group_flags);
    csb_v1_runtime_write_u16(first_thing_ptr, group_thing);
    csb_v1_runtime_sync_active_group_state_from_record(
        profile,
        group_thing,
        group_record,
        record->mapIndex,
        record->mapX,
        record->mapY,
        0,
        0);
    /* ReDMCSB GROUP.C F0180 lines 311-340 starts wandering by scheduling
     * C37 at game_time + 1 and prioritizes faster creatures as
     * 255 - MovementTicks. */
    csb_v1_runtime_schedule_c37_group_event(
        profile,
        record->mapIndex,
        record->mapX,
        record->mapY,
        result.spawnedCreatureType,
        1u);
}

static void csb_v1_runtime_apply_corridor_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int raw_square;
    int thing;
    int guard;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon, record->mapIndex, record->mapX, record->mapY);
    /* F0245 is reached only from a real C05 corridor event.  A queued event
     * must not reinterpret wall/floor bytes as a generator list. */
    if (raw_square < 0 || dungeon->square_bytes != 1 ||
        ((raw_square >> 5) & 0x07) != DM1_SQUARE_CORRIDOR) {
        return;
    }
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB TIMELINE.C F0245 lines 944-1001 walks C05 corridor square
     * things.  C02 textstrings toggle/set/clear Visible, while C006 floor
     * group generators disable once-only sensors or disable-and-schedule C65
     * after M046_TICKS.  Group materialization via GROUP.C F0185 remains a
     * separate CSB runtime binding because it needs live group-slot state. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *thing_record;
        int thing_type;
        int thing_size;

        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 2 && thing_size >= 4) {
            uint16_t text_word = csb_v1_runtime_read_u16(thing_record + 2);
            if (record->effect == DM1_EFFECT_TOGGLE) {
                text_word ^= 0x0001u;
            } else if (record->effect == DM1_EFFECT_SET) {
                text_word |= 0x0001u;
            } else {
                text_word &= (uint16_t)~0x0001u;
            }
            csb_v1_runtime_write_u16(thing_record + 2, text_word);
        } else if (thing_type == 3 && thing_size >= 8) {
            uint16_t type_data = csb_v1_runtime_read_u16(thing_record + 2);
            uint16_t flags_word = csb_v1_runtime_read_u16(thing_record + 4);
            uint16_t local_word = csb_v1_runtime_read_u16(thing_record + 6);
            int sensor_type = (int)(type_data & 0x007Fu);
            int sensor_data = (int)(type_data >> 7);
            int once_only = (int)((flags_word >> 2) & 0x01u);
            uint32_t ticks = (uint32_t)(local_word >> 4);

            if (sensor_type == 6) {
                csb_v1_runtime_materialize_corridor_generator_group(
                    profile,
                    record,
                    sensor_data,
                    flags_word,
                    local_word);
                /* ReDMCSB TIMELINE.C F0245 calls F0185 first, requests the
                 * source C17 buzz when Audible is set, then mutates the
                 * generator type and schedules C65. */
                if ((flags_word & (1u << 6)) != 0u) {
                    CsbV1AudioRequest request;

                    memset(&request, 0, sizeof(request));
                    request.soundIndex = CSB_V1_SOUND_BUZZ;
                    request.mapX = (int16_t)record->mapX;
                    request.mapY = (int16_t)record->mapY;
                    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
                    request.volume = 64;
                    request.priority = 4u;
                    (void)csb_v1_audio_runtime_request(
                        &profile->audio_runtime, &request);
                }
                if (once_only) {
                    type_data &= 0xFF80u;
                    csb_v1_runtime_write_u16(thing_record + 2, type_data);
                } else if (ticks != 0u) {
                    if (ticks > 127u) {
                        ticks = (ticks - 126u) << 6;
                    }
                    type_data &= 0xFF80u;
                    csb_v1_runtime_write_u16(thing_record + 2, type_data);
                    csb_v1_runtime_schedule_enable_group_generator_record(
                        profile,
                        record,
                        ticks);
                }
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_enable_group_generator_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int thing;
    int guard;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB TIMELINE.C F0246 lines 1009-1027 walks square things and
     * changes the first disabled C03 sensor type back to C006 group
     * generator. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *sensor;
        int thing_type;
        int thing_size;
        uint16_t type_data;

        sensor = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!sensor) break;
        if (thing_type == 3 && thing_size >= 8) {
            type_data = csb_v1_runtime_read_u16(sensor + 2);
            if ((type_data & 0x007Fu) == 0u) {
                type_data = (uint16_t)((type_data & 0xFF80u) | 6u);
                csb_v1_runtime_write_u16(sensor + 2, type_data);
                return;
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_wall_sensor_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int raw_square;
    int thing;
    int guard;
    CSB_V1_F0248LocalEffectReceipt_PC34 pending_local_receipt;
    int has_pending_local_rotation = 0;

    if (!profile || !record || !profile->dungeon_handle) return;
    memset(&pending_local_receipt, 0, sizeof(pending_local_receipt));
    dungeon = profile->dungeon_handle;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon, record->mapIndex, record->mapX, record->mapY);
    /* F0248 belongs exclusively to a C06 wall event.  Missing or mismatched
     * raw dungeon state is rejected before any sensor-data mutation. */
    if (raw_square < 0 || dungeon->square_bytes != 1 ||
        ((raw_square >> 5) & 0x07) != DM1_SQUARE_WALL) {
        return;
    }
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB TIMELINE.C F0248 lines 1175-1195 toggles same-cell wall
     * TextString visibility, then lines 1198-1308 handles wall C006
     * countdown and C005 AND/OR gate sensors by mutating M040_DATA and
     * feeding matching remote effects back through F0272_SENSOR_TriggerEffect.
     * Lines 1317-1339 handle C018 endgame sensors.  F0272 lines 1191-1197
     * also disables once-only triggered sensors and routes LocalEffect
     * sensors through F0270/F0271 instead of queuing a remote square event. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *sensor;
        int thing_type;
        int thing_size;
        uint16_t next_word;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int target_x;
        int target_y;
        int target_cell;
        int local_effect;
        int local_multiple;
        int once_only;
        int trigger = 0;
        int trigger_effect = DM1_EFFECT_SET;
        int trigger_delay = 0;
        int trigger_audible = 0;

        sensor = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!sensor) break;
        if (thing_type == 2 && thing_size >= 4 &&
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                (uint16_t)thing) == (record->cell & 3)) {
            uint16_t text_word = csb_v1_runtime_read_u16(sensor + 2);
            if (record->effect == DM1_EFFECT_TOGGLE) {
                text_word ^= 0x0001u;
            } else if (record->effect == DM1_EFFECT_SET) {
                text_word |= 0x0001u;
            } else {
                text_word &= (uint16_t)~0x0001u;
            }
            csb_v1_runtime_write_u16(sensor + 2, text_word);
        } else if (thing_type == 3 && thing_size >= 8) {
            next_word = csb_v1_runtime_read_u16(sensor + 0);
            type_data = csb_v1_runtime_read_u16(sensor + 2);
            flags_word = csb_v1_runtime_read_u16(sensor + 4);
            target_word = csb_v1_runtime_read_u16(sensor + 6);
            sensor_type = (int)(type_data & 0x007Fu);
            sensor_data = (int)(type_data >> 7);
            once_only = (int)((flags_word >> 2) & 0x01u);
            local_effect = (int)((flags_word >> 11) & 0x01u);
            local_multiple = (int)(target_word & 0x0FFFu);
            target_cell = (int)((target_word >> 4) & 0x03u);
            target_x = (int)((target_word >> 6) & 0x1Fu);
            target_y = (int)((target_word >> 11) & 0x1Fu);

            if (sensor_type == DM1_SENSOR_WALL_COUNTDOWN) {
                struct DungeonSensor_Compat decoded_sensor;
                struct SensorTriggerResult_Compat countdown_result;

                csb_v1_runtime_decode_sensor_words(
                    next_word, type_data, flags_word, target_word,
                    &decoded_sensor);
                memset(&countdown_result, 0, sizeof(countdown_result));
                if (F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
                        &decoded_sensor, record->effect, record->mapX,
                        record->mapY, record->cell, &countdown_result) &&
                    countdown_result.sensorDataChanged) {
                    type_data = (uint16_t)(
                        (type_data & 0x007Fu) |
                        ((uint16_t)countdown_result.sensorDataAfter << 7));
                    csb_v1_runtime_write_u16(sensor + 2, type_data);
                }
                if (countdown_result.triggered) {
                    trigger = 1;
                    trigger_effect = countdown_result.resolvedEffect;
                    trigger_delay = countdown_result.delayTicks;
                    trigger_audible = countdown_result.audible;
                    once_only = countdown_result.sensorDisabled;
                    local_effect = countdown_result.isLocal;
                    local_multiple = countdown_result.localEffectValue;
                    target_x = countdown_result.targetMapX;
                    target_y = countdown_result.targetMapY;
                    target_cell = countdown_result.targetCell;
                }
            } else if (sensor_type == DM1_SENSOR_WALL_AND_OR_GATE) {
                struct DungeonSensor_Compat decoded_sensor;
                struct SensorTriggerResult_Compat gate_result;
                int target_square_type = -1;

                /* F0248 reads the destination square before it delegates
                 * the C005 bit-mask mutation and F0272 dispatch to F0730.
                 * Do not manufacture a target event when live DUNGEON.DAT
                 * cannot supply that square type. */
                if (!csb_v1_runtime_square_byte_ptr(
                        profile, record->mapIndex, target_x, target_y,
                        &target_square_type)) {
                    thing = csb_v1_runtime_sensor_next_thing(
                        dungeon, (uint16_t)thing);
                    continue;
                }
                csb_v1_runtime_decode_sensor_words(
                    next_word, type_data, flags_word, target_word,
                    &decoded_sensor);
                memset(&gate_result, 0, sizeof(gate_result));
                if (F0730_SENSOR_EvaluateWallAndOrGateEvent_Compat(
                        &decoded_sensor, record->cell, record->effect,
                        target_square_type, record->mapX, record->mapY,
                        &gate_result)) {
                    if (gate_result.sensorDataChanged) {
                        type_data = (uint16_t)(
                            (type_data & 0x007Fu) |
                            ((uint16_t)gate_result.sensorDataAfter << 7));
                        csb_v1_runtime_write_u16(sensor + 2, type_data);
                    }
                    if (gate_result.triggered) {
                        trigger = 1;
                        trigger_effect = gate_result.resolvedEffect;
                        trigger_delay = gate_result.delayTicks;
                        trigger_audible = decoded_sensor.audible;
                        once_only = gate_result.sensorDisabled;
                        local_effect = gate_result.isLocal;
                        local_multiple = gate_result.localEffectValue;
                        target_x = gate_result.targetMapX;
                        target_y = gate_result.targetMapY;
                        target_cell = gate_result.targetCell;
                    }
                }
            } else if (sensor_type == DM1_SENSOR_WALL_END_GAME) {
                struct DungeonSensor_Compat decoded_sensor;
                struct SensorTriggerResult_Compat endgame_result;
                CSB_V1_F0248EndgameRuntimeReceipt_PC34 endgame_receipt;
                csb_v1_runtime_decode_sensor_words(
                    next_word,
                    type_data,
                    flags_word,
                    target_word,
                    &decoded_sensor);
                memset(&endgame_result, 0, sizeof(endgame_result));
                if (F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
                        &decoded_sensor,
                        csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                            (uint16_t)thing),
                        record->effect,
                        record->cell,
                        &endgame_result) &&
                    csb_v1_f0248_endgame_consume_pc34_compat(
                        &endgame_result, profile->victory,
                        &endgame_receipt)) {
                    profile->victory = endgame_receipt.game_won;
                    profile->state = CSB_STATE_VICTORY;
                }
            } else if (csb_v1_runtime_sensor_type_is_explosion_launcher(
                           sensor_type) ||
                       csb_v1_runtime_sensor_type_is_new_object_launcher(
                           sensor_type) ||
                       csb_v1_runtime_sensor_type_is_square_object_launcher(
                           sensor_type)) {
                struct DungeonSensor_Compat decoded_sensor;
                struct ProjectileLauncherContext_Compat launcher_ctx;
                struct ProjectileLauncherResult_Compat launcher_result;
                struct ProjectileLauncherSquareThing_Compat square_things[64];
                int square_thing_count = 0;
                int launch_index;
                int is_square_object_launcher =
                    csb_v1_runtime_sensor_type_is_square_object_launcher(
                        sensor_type);
                int is_new_object_launcher =
                    csb_v1_runtime_sensor_type_is_new_object_launcher(
                        sensor_type);

                csb_v1_runtime_decode_sensor_words(
                    next_word,
                    type_data,
                    flags_word,
                    target_word,
                    &decoded_sensor);
                memset(&launcher_ctx, 0, sizeof(launcher_ctx));
                /* F0247 reaches M005_RANDOM(2) only after a single
                 * projectile has been selected.  Start with zero so the
                 * first evaluation can establish whether a projectile exists
                 * without advancing the persisted ReDMCSB G349 stream. */
                launcher_ctx.randomBit = 0;
                launcher_ctx.newObjectThings[0] = 0xFFFFu;
                launcher_ctx.newObjectThings[1] = 0xFFFFu;
                if (is_new_object_launcher) {
                    launcher_ctx.newObjectThings[0] =
                        csb_v1_runtime_allocate_new_object_launcher_thing(
                            dungeon,
                            sensor_data);
                    if (sensor_type ==
                        DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ) {
                        launcher_ctx.newObjectThings[1] =
                            csb_v1_runtime_allocate_new_object_launcher_thing(
                                dungeon,
                                sensor_data);
                    }
                }
                if (is_square_object_launcher) {
                    square_thing_count =
                        csb_v1_runtime_collect_square_launcher_things(
                            dungeon,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            square_things,
                            (int)(sizeof(square_things) /
                                  sizeof(square_things[0])));
                    launcher_ctx.squareThings = square_things;
                    launcher_ctx.squareThingCount = square_thing_count;
                }
                memset(&launcher_result, 0, sizeof(launcher_result));
                if (F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
                        &decoded_sensor,
                        csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                            (uint16_t)thing),
                        record->mapX,
                        record->mapY,
                        record->cell,
                        &launcher_ctx,
                        &launcher_result) &&
                    launcher_result.triggered) {
                    /* F0247 removes C014/C015 source objects before F0212.
                     * Firestaff only transfers that ownership when every
                     * resulting launch square is loaded from real
                     * DUNGEON.DAT; an unavailable destination is no-draw and
                     * leaves the source sensor/list untouched. */
                    if (is_square_object_launcher &&
                        !csb_v1_runtime_launcher_result_has_loaded_destinations(
                            profile, record->mapIndex, &launcher_result)) {
                        launcher_result.triggered = 0;
                    }
                    if (launcher_result.launchSingleProjectile &&
                        launcher_result.launchCount > 0) {
                        int random_bit;

                        if (!csb_v1_f0247_launcher_next_random_bit_pc34_compat(
                                &profile->csbwin_random_seed,
                                &random_bit)) {
                            continue;
                        }
                        launcher_ctx.randomBit = random_bit;
                        memset(&launcher_result, 0, sizeof(launcher_result));
                        if (!F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
                                &decoded_sensor,
                                csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                                    (uint16_t)thing),
                                record->mapX,
                                record->mapY,
                                record->cell,
                                &launcher_ctx,
                                &launcher_result) ||
                            !launcher_result.triggered) {
                            continue;
                        }
                    }
                    if (launcher_result.sensorDisabled) {
                        type_data = (uint16_t)(type_data & 0xFF80u);
                        csb_v1_runtime_write_u16(sensor + 2, type_data);
                    }
                    if (is_square_object_launcher) {
                        int unlink_index;
                        /* ReDMCSB TIMELINE.C F0247 lines 1079-1100 finds
                         * same/next-cell square objects, then unlinks them
                         * through F0164 before F0212 creates launcher
                         * projectiles.  Sensor effects are intentionally not
                         * triggered by this ownership transfer. */
                        for (unlink_index = 0;
                             unlink_index < launcher_result.unlinkCount;
                             ++unlink_index) {
                            (void)csb_v1_runtime_unlink_thing_from_square(
                                dungeon,
                                launcher_result.unlinkThings[unlink_index],
                                record->mapIndex,
                                record->mapX,
                                record->mapY);
                        }
                    }
                    for (launch_index = 0;
                         launch_index < launcher_result.launchCount;
                         ++launch_index) {
                        const struct ProjectileLauncherLaunch_Compat *launch =
                            &launcher_result.launches[launch_index];
                        struct ProjectileCreateInput_Compat input;
                        struct TimelineEvent_Compat first_move;
                        int slot = -1;
                        int subtype;

                        if (!launch->valid) continue;
                        subtype =
                            csb_v1_runtime_projectile_subtype_from_explosion_thing(
                                launch->associatedThing);
                        memset(&input, 0, sizeof(input));
                        memset(&first_move, 0, sizeof(first_move));
                        input.category = (is_square_object_launcher ||
                                          is_new_object_launcher)
                            ? PROJECTILE_CATEGORY_KINETIC
                            : PROJECTILE_CATEGORY_MAGICAL;
                        input.subtype = (is_square_object_launcher ||
                                         is_new_object_launcher)
                            ? PROJECTILE_SUBTYPE_KINETIC_ARROW
                            : subtype;
                        input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
                        input.ownerIndex = -1;
                        input.mapIndex = record->mapIndex;
                        input.mapX = launch->mapX;
                        input.mapY = launch->mapY;
                        input.cell = launch->cell;
                        input.direction = launch->direction;
                        input.kineticEnergy = launch->kineticEnergy;
                        input.attack = launch->attack;
                        input.launcherStrength = launch->attack;
                        input.stepEnergy = launch->stepEnergy;
                        input.currentTick = (int)profile->game_time;
                        input.poisonAttack = (!is_square_object_launcher &&
                                              !is_new_object_launcher &&
                                              subtype ==
                                                  PROJECTILE_SUBTYPE_POISON_CLOUD)
                            ? launch->attack
                            : 0;
                        input.attackTypeCode = (is_square_object_launcher ||
                                                is_new_object_launcher)
                            ? COMBAT_ATTACK_BLUNT
                            : csb_v1_runtime_projectile_attack_type_from_subtype(
                                  subtype);
                        input.associatedThing = (int)launch->associatedThing;
                        input.firstMoveGraceFlag = 0;
                        if (F0810_PROJECTILE_Create_Compat(
                                &input,
                                &profile->projectiles,
                                &slot,
                                &first_move)) {
                            csb_v1_runtime_schedule_projectile_move_event(
                                profile,
                                &first_move);
                        } else {
                            /* CSB21 PROJEXPL.C F0212 CHANGE8_00_FIX keeps
                             * C007/C009/C014/C015 associated objects when
                             * no C14 projectile record can be allocated.
                             * Explosion launcher pseudo-things are rejected
                             * by the receipt and therefore remain no-draw. */
                            (void)csb_v1_runtime_materialize_launcher_create_failure(
                                profile, dungeon, record->mapIndex, launch);
                        }
                    }
                }
            }
            if (trigger) {
                if (trigger_audible) {
                    CsbV1AudioRequest request;

                    memset(&request, 0, sizeof(request));
                    request.soundIndex = CSB_V1_SOUND_SWITCH;
                    request.mapX = (int16_t)record->mapX;
                    request.mapY = (int16_t)record->mapY;
                    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
                    request.volume = 64;
                    request.priority = 4u;
                    (void)csb_v1_audio_runtime_request(
                        &profile->audio_runtime, &request);
                }
                if (once_only) {
                    type_data = (uint16_t)(type_data & 0xFF80u);
                    csb_v1_runtime_write_u16(sensor + 2, type_data);
                }
                if (local_effect) {
                    struct SensorTriggerResult_Compat local_result;
                    CSB_V1_F0248LocalEffectReceipt_PC34 local_receipt;

                    memset(&local_result, 0, sizeof(local_result));
                    local_result.triggered = 1;
                    local_result.isLocal = 1;
                    local_result.localEffectValue = local_multiple;
                    if (csb_v1_f0248_local_effect_consume_pc34_compat(
                            &local_result, record->mapX, record->mapY,
                            record->cell, &local_receipt)) {
                        if (local_receipt.award_steal_experience) {
                            csb_v1_runtime_add_party_steal_skill_experience(
                                profile, local_receipt.leader_only);
                        } else {
                            /* SENSOR.C F0270 retains only the last non-XP
                             * local effect; F0271 consumes it after the
                             * entire C06 list has been processed. */
                            pending_local_receipt = local_receipt;
                            has_pending_local_rotation = 1;
                        }
                    }
                } else {
                    csb_v1_runtime_trigger_remote_sensor_event_after(
                        profile,
                        record->mapIndex,
                        trigger_effect,
                        target_x,
                        target_y,
                        target_cell,
                        trigger_delay);
                }
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    if (has_pending_local_rotation &&
        (pending_local_receipt.rotation_effect == DM1_EFFECT_CLEAR ||
         pending_local_receipt.rotation_effect == DM1_EFFECT_TOGGLE)) {
        (void)csb_v1_runtime_rotate_wall_cell_sensors(
            dungeon, record->mapIndex, pending_local_receipt.map_x,
            pending_local_receipt.map_y, pending_local_receipt.cell);
    }
}

static void csb_v1_runtime_apply_timeline_dispatch_side_effects(
    CSB_V1_RuntimeProfile *profile,
    const uint16_t *event_indices,
    int event_count)
{
    int i;

    if (!profile || !event_indices || event_count < 0) return;
    /* ReDMCSB: TIMELINE.C F0261 lines 1875-1901 dispatches C05/C06/C07/C08/C09/C10
     * to F0242/F0250/F0251/F0244; F0244 immediately routes doors through
     * C01 door-animation, and F0241 lines 754-809 steps the door state one
     * value per event.  This runtime bridge mutates real-format CSB byte-map
     * square flags and bounded wall/generator sensor state for the startup
     * playability path; projectile launchers, group movement, damage, sounds,
     * and DSA effects remain separate work. */
    for (i = 0; i < profile->last_timeline_dispatch.count && i < event_count;
         ++i) {
        const struct DM1_DispatchRecord_V1 *record =
            &profile->last_timeline_dispatch.records[i];
        switch (record->eventType) {
        case DM1_EVENT_CORRIDOR:
            csb_v1_runtime_apply_corridor_timeline_record(profile, record);
            break;
        case DM1_EVENT_WALL:
            csb_v1_runtime_apply_wall_sensor_timeline_record(profile, record);
            break;
        case DM1_EVENT_MOVE_PROJECTILE:
        case DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS:
            csb_v1_runtime_apply_projectile_move_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_EXPLOSION:
            csb_v1_runtime_apply_explosion_timeline_record(profile, record);
            break;
        case DM1_EVENT_DOOR:
        case DM1_EVENT_DOOR_ANIMATION:
            csb_v1_runtime_apply_door_timeline_record(profile, record);
            break;
        case DM1_EVENT_DOOR_DESTRUCTION:
            {
                uint8_t *square;
                int square_type;
                square = csb_v1_runtime_square_byte_ptr(
                    profile,
                    record->mapIndex,
                    record->mapX,
                    record->mapY,
                    &square_type);
                if (square && square_type == 4) {
                    F0243_TIMELINE_ProcessEvent2_DoorDestruction(square);
                }
            }
            break;
        case DM1_EVENT_FAKEWALL:
            csb_v1_runtime_apply_square_state_timeline_record(
                profile,
                record,
                6,
                0x04u);
            break;
        case DM1_EVENT_TELEPORTER:
            csb_v1_runtime_apply_square_state_timeline_record(
                profile,
                record,
                5,
                0x08u);
            break;
        case DM1_EVENT_PIT:
            csb_v1_runtime_apply_square_state_timeline_record(
                profile,
                record,
                2,
                0x08u);
            break;
        case DM1_EVENT_ENABLE_GROUP_GENERATOR:
            csb_v1_runtime_apply_enable_group_generator_record(
                profile,
                record);
            break;
        case DM1_EVENT_UPDATE_BEHAVIOR_GROUP:
            csb_v1_runtime_apply_group_behavior_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_MOVE_GROUP_SILENT:
        case DM1_EVENT_MOVE_GROUP_AUDIBLE:
            csb_v1_runtime_apply_move_group_timeline_record(profile, record);
            break;
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_0:
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_1:
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_2:
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_3:
            csb_v1_runtime_apply_creature_aspect_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0:
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1:
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_2:
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3:
            csb_v1_runtime_apply_creature_attack_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_POISON_CHAMPION:
            csb_v1_runtime_apply_poison_event_record(
                profile, record, event_indices[i]);
            break;
        default:
            break;
        }
    }
}

/* ── Runtime profile API ────────────────────────────────────────────── */

void csb_v1_runtime_init(CSB_V1_RuntimeProfile *profile, const char *data_dir)
{
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));

    profile->variant_id     = CSB_V1_VARIANT_UNKNOWN;
    profile->difficulty    = CSB_V1_DIFFICULTY_HARD; /* default: 3 champions */
    profile->current_level = 0;
    profile->current_world = 0;
    profile->level_count   = 1;
    profile->world_count   = 1;
    profile->champion_count = 3;
    profile->leader_index = -1;
    profile->magic_caster_index = -1;
    profile->party_state_valid = 0;
    csb_v1_character_init_default(&profile->party_state);

    profile->party_x = CSB_V1_START_PARTY_X;
    profile->party_y = CSB_V1_START_PARTY_Y;
    profile->party_z = CSB_V1_START_PARTY_Z;
    profile->party_dir = CSB_V1_START_PARTY_DIR;

    profile->state     = CSB_STATE_TITLE;
    profile->load_bonus_dungeon = 0;
    profile->paused    = 0;
    profile->victory   = 0;
    profile->game_over = 0;

    profile->game_ticks    = 0;
    profile->game_time     = 0;
    profile->total_play_ms = 0;
    profile->tick_count    = 0;
    profile->csbwin_party_sleeping = 0;
    dm1v1_event_queue_init(&profile->timeline_queue, profile->game_time);
    csb_v1_audio_runtime_init(&profile->audio_runtime);
    memset(&profile->last_timeline_dispatch, 0,
           sizeof(profile->last_timeline_dispatch));
    profile->timeline_dispatch_count = 0;
    csb_v1_skin_cache_init(&profile->skin_cache);
    csb_v1_chaos_init(&profile->csbwin_extended_dsa_state);
    memset(profile->csbwin_extended_level_dsa_index, 0xff,
           sizeof(profile->csbwin_extended_level_dsa_index));
    DM1_V1_InputCommandQueue_InitPc34Compat(&profile->input_command_queue);
    memset(&profile->last_input_dispatch, 0,
           sizeof(profile->last_input_dispatch));
    profile->input_dispatch_count = 0;

    profile->data_dir = data_dir;
    profile->save_dir = csb_v1_runtime_save_dir();
}

int csb_v1_runtime_custom_background_skin_grid(
    CSB_V1_RuntimeProfile *profile,
    uint8_t *out_cell_skins,
    int out_cell_skin_capacity,
    int *out_width,
    int *out_height,
    int *out_loaded_level,
    int *out_default_skin)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int width;
    int height;
    int x;
    int y;
    int has_skin = 0;
    int tail_receipt_valid;
    uint8_t default_skin;
    uint32_t tail_fnv1a = 0u;
    CSB_V1_RuntimeSkinCacheLookupCtx lookup_ctx;

    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_loaded_level) *out_loaded_level = -1;
    if (out_default_skin) *out_default_skin = 0;
    if (!profile || !out_cell_skins || out_cell_skin_capacity <= 0 ||
        !profile->dungeon_handle) {
        return 0;
    }

    dungeon = profile->dungeon_handle;
    level = profile->current_level;
    if (level < 0 || level >= dungeon->level_count) {
        return 0;
    }
    width = dungeon->level_widths[level];
    height = dungeon->level_heights[level];
    if (width <= 0 || height <= 0 ||
        width * height > out_cell_skin_capacity) {
        return 0;
    }

    /* A saved EDT_Skins column owns the cached bytes. Reuse it only while
     * the exact admitted tail still matches; a missing, truncated, or
     * altered tail must not leave a prior resume's custom background live. */
    tail_receipt_valid = profile->csbwin_appended_tail_valid &&
        !profile->csbwin_appended_tail_truncated &&
        profile->csbwin_appended_tail_preserved_size > 0u &&
        profile->csbwin_appended_tail_preserved_size <=
            sizeof(profile->csbwin_appended_tail);
    if (tail_receipt_valid) {
        tail_fnv1a = csb_v1_runtime_fnv1a32(
            profile->csbwin_appended_tail,
            profile->csbwin_appended_tail_preserved_size);
        tail_receipt_valid = tail_fnv1a == profile->csbwin_appended_tail_fnv1a;
    }
    if (!profile->csbwin_skin_cache_tail_receipt_valid ||
        profile->csbwin_skin_cache_tail_valid != tail_receipt_valid ||
        profile->csbwin_skin_cache_tail_size !=
            (tail_receipt_valid ? profile->csbwin_appended_tail_preserved_size : 0u) ||
        profile->csbwin_skin_cache_tail_fnv1a !=
            (tail_receipt_valid ? tail_fnv1a : 0u)) {
        csb_v1_skin_cache_init(&profile->skin_cache);
        profile->csbwin_skin_cache_tail_receipt_valid = 1;
        profile->csbwin_skin_cache_tail_valid = tail_receipt_valid;
        profile->csbwin_skin_cache_tail_size =
            tail_receipt_valid ? profile->csbwin_appended_tail_preserved_size : 0u;
        profile->csbwin_skin_cache_tail_fnv1a =
            tail_receipt_valid ? tail_fnv1a : 0u;
    }

    memset(out_cell_skins, 0, (size_t)width * (size_t)height);
    /* CSBWin data.cpp SKIN_CACHE::GetSkin/GetDefaultSkin reads Expool
     * EDT_Skins records through Locate(); Firestaff resolves the runtime
     * CSBWin save tail first so saved SETSKIN state can override the loaded
     * dungeon DB11 defaults during startup/resume rendering. */
    lookup_ctx.profile = profile;
    lookup_ctx.dungeon = dungeon;
    default_skin = csb_v1_skin_cache_get_default_skin(
        &profile->skin_cache,
        csb_v1_runtime_skin_cache_record_lookup,
        &lookup_ctx,
        level);
    if (default_skin != 0u) {
        has_skin = 1;
    }
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint8_t skin = csb_v1_skin_cache_get_skin(
                &profile->skin_cache,
                csb_v1_runtime_skin_cache_record_lookup,
                &lookup_ctx,
                level,
                width,
                height,
                x,
                y);
            out_cell_skins[(size_t)y * (size_t)width + (size_t)x] = skin;
            if (skin != 0u) {
                has_skin = 1;
            }
        }
    }

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    if (out_loaded_level) *out_loaded_level = level;
    if (out_default_skin) *out_default_skin = (int)default_skin;
    return has_skin;
}

int csb_v1_runtime_set_load_bonus_dungeon(CSB_V1_RuntimeProfile *profile,
                                          int enabled)
{
    if (!profile) return 0;
    /* ReDMCSB COMMAND.C lines 2438-2445 sets G1147_B_LoadBonusDungeon when
     * C201_COMMAND_ENTRANCE_ENTER_BONUS_DUNGEON is selected.  LOADSAVE.C
     * lines 2316-2334 later consumes that flag while trying the platform's
     * bonus dungeon filename before falling back to the normal dungeon path. */
    profile->load_bonus_dungeon = enabled ? 1 : 0;
    return 1;
}

int csb_v1_runtime_get_load_bonus_dungeon(
    const CSB_V1_RuntimeProfile *profile)
{
    return (profile && profile->load_bonus_dungeon) ? 1 : 0;
}

static int csb_v1_runtime_file_exists(const char *path)
{
    struct stat st;
    return path && path[0] != '\0' && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int csb_v1_runtime_dirname(const char *path, char *out, size_t out_size)
{
    const char *slash;
    const char *backslash;
    const char *last;
    size_t len;
    if (!path || !out || out_size == 0u) return 0;
    out[0] = '\0';
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    last = slash;
    if (backslash && (!last || backslash > last)) {
        last = backslash;
    }
    if (!last || last == path) return 0;
    len = (size_t)(last - path);
    if (len + 1u > out_size) return 0;
    memcpy(out, path, len);
    out[len] = '\0';
    return 1;
}

static int csb_v1_runtime_join_path(char *out,
                                    size_t out_size,
                                    const char *dir,
                                    const char *name)
{
    int rc;
    const char *sep = "/";
    size_t dir_len;
    if (!out || out_size == 0u || !dir || !dir[0] || !name || !name[0]) {
        return 0;
    }
    dir_len = strlen(dir);
    if (dir[dir_len - 1u] == '/' || dir[dir_len - 1u] == '\\') {
        sep = "";
    }
    rc = snprintf(out, out_size, "%s%s%s", dir, sep, name);
    return rc > 0 && rc < (int)out_size;
}

static int csb_v1_runtime_replace_dungeon_handle(CSB_V1_RuntimeProfile *profile,
                                                 const char *path)
{
    CSB_V1_DungeonData *dungeon;
    if (!profile || !path || path[0] == '\0') return 0;
    dungeon = (CSB_V1_DungeonData *)calloc(1, sizeof(*dungeon));
    if (!dungeon) return 0;
    if (csb_v1_dungeon_load_from_file(dungeon, path) != 0) {
        free(dungeon);
        return 0;
    }
    csb_v1_dungeon_unload();
    if (profile->dungeon_handle) {
        free(profile->dungeon_handle);
    }
    profile->dungeon_handle = dungeon;
    profile->dungeon_path = path;
    csb_v1_dungeon_set_current(dungeon);
    csb_v1_dungeon_set_current_level(0);
    return 1;
}

static int csb_v1_runtime_try_bonus_candidate(CSB_V1_RuntimeProfile *profile,
                                              const char *dir,
                                              const char *name)
{
    char candidate[ASSET_PATH_MAX];
    if (!csb_v1_runtime_join_path(candidate, sizeof(candidate), dir, name)) {
        return 0;
    }
    if (!csb_v1_runtime_file_exists(candidate)) {
        return 0;
    }
    if (!csb_v1_runtime_replace_dungeon_handle(profile, candidate)) {
        return 0;
    }
    snprintf(profile->bonus_dungeon_path,
             sizeof(profile->bonus_dungeon_path),
             "%s",
             candidate);
    profile->dungeon_path = profile->bonus_dungeon_path;
    return 1;
}

int csb_v1_runtime_try_load_bonus_dungeon(CSB_V1_RuntimeProfile *profile)
{
    static const char *const kBonusNames[] = {
        "DUNGEONB.DAT",
        "DungeonB.dat",
        "dungeonb.dat",
        "DUNGEON.BONUS",
        NULL
    };
    char normal_dir[ASSET_PATH_MAX];
    char csb_dir[ASSET_PATH_MAX];
    int i;

    if (!profile || !profile->load_bonus_dungeon) return 0;
    profile->bonus_dungeon_path[0] = '\0';

    /* ReDMCSB LOADSAVE.C lines 2316-2334 tries the platform bonus dungeon
     * filename when G1147_B_LoadBonusDungeon is true, then falls back to the
     * ordinary dungeon load path. Search the directory that supplied the
     * verified normal dungeon first, then the runtime data root and its csb/
     * child. Do not destroy the existing dungeon handle unless a candidate
     * file is actually loadable. */
    if (csb_v1_runtime_dirname(profile->dungeon_path,
                               normal_dir,
                               sizeof(normal_dir))) {
        for (i = 0; kBonusNames[i]; ++i) {
            if (csb_v1_runtime_try_bonus_candidate(profile,
                                                   normal_dir,
                                                   kBonusNames[i])) {
                return 1;
            }
        }
    }
    if (profile->data_dir && profile->data_dir[0] != '\0') {
        for (i = 0; kBonusNames[i]; ++i) {
            if (csb_v1_runtime_try_bonus_candidate(profile,
                                                   profile->data_dir,
                                                   kBonusNames[i])) {
                return 1;
            }
        }
        if (csb_v1_runtime_join_path(csb_dir,
                                     sizeof(csb_dir),
                                     profile->data_dir,
                                     "csb")) {
            for (i = 0; kBonusNames[i]; ++i) {
                if (csb_v1_runtime_try_bonus_candidate(profile,
                                                       csb_dir,
                                                       kBonusNames[i])) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

const char *csb_v1_runtime_get_bonus_dungeon_path(
    const CSB_V1_RuntimeProfile *profile)
{
    return (profile && profile->bonus_dungeon_path[0] != '\0')
        ? profile->bonus_dungeon_path
        : NULL;
}

int csb_v1_runtime_add_timeline_event(CSB_V1_RuntimeProfile *profile,
                                      const struct DM1_Event_V1 *event)
{
    int i;
    if (!profile || !event) return -1;
    profile->timeline_queue.gameTick = profile->game_time;

    /* CSBWin Timer.cpp GameTimers::SetTimer:967-1007 checks the restored
     * EDBT_DeleteDuplicateTimers policy only for map timers C05..C10. A
     * matching source timer retains its queue slot and receives the new
     * action byte; TT_STONEROOM additionally requires the same position.
     * Keep the comparison inside the live CSB timer owner, so a save policy
     * never reaches a generic wrapper or a caller-built replacement queue. */
    if (profile->csbwin_delete_duplicate_timers != 0u &&
        event->type >= DM1_EVENT_CORRIDOR && event->type <= DM1_EVENT_DOOR) {
        for (i = 0; i < profile->timeline_queue.eventCount; ++i) {
            int event_index = profile->timeline_queue.timeline[i];
            struct DM1_Event_V1 *existing;

            if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
                continue;
            }
            existing = &profile->timeline_queue.events[event_index];
            if (existing->type < DM1_EVENT_CORRIDOR ||
                existing->type > DM1_EVENT_DOOR ||
                existing->type != event->type ||
                existing->map_time != event->map_time ||
                existing->b_mapX != event->b_mapX ||
                existing->b_mapY != event->b_mapY) {
                continue;
            }
            if (existing->type == DM1_EVENT_WALL &&
                existing->c_cell != event->c_cell) {
                continue;
            }
            existing->c_effect = event->c_effect;
            return event_index;
        }
        return csb_v1_runtime_append_unmerged_map_timer(profile, event);
    }
    if (event->type >= DM1_EVENT_CORRIDOR &&
        event->type <= DM1_EVENT_DOOR &&
        profile->csbwin_delete_duplicate_timers == 0u) {
        return csb_v1_runtime_append_unmerged_map_timer(profile, event);
    }
    return dm1v1_event_add(&profile->timeline_queue, event);
}

int csb_v1_runtime_get_last_timeline_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct DM1_TickDispatchResult_V1 *out_result)
{
    if (!profile || !out_result) return -1;
    *out_result = profile->last_timeline_dispatch;
    return out_result->count;
}

int csb_v1_runtime_enqueue_input_command(CSB_V1_RuntimeProfile *profile,
                                         int command,
                                         int x,
                                         int y)
{
    if (!profile) return 0;
    return DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
        &profile->input_command_queue, command, x, y);
}

int csb_v1_runtime_process_one_input_command(
    CSB_V1_RuntimeProfile *profile,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction)
{
    CSB_V1_InputCommandRuntimeResult result;

    if (!profile) return -1;
    return csb_v1_runtime_process_input_queue(
        profile,
        &profile->input_command_queue,
        disabled_movement_ticks,
        projectile_disabled_movement_ticks,
        last_projectile_disabled_movement_direction,
        &result);
}

int csb_v1_runtime_get_last_input_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputQueueProcessResultPc34Compat *out_result)
{
    if (!profile || !out_result) return -1;
    *out_result = profile->last_input_dispatch;
    return out_result->dequeued;
}

static int csb_v1_runtime_first_living_champion(const CSB_V1_PartyState *party)
{
    int i;
    if (!party) return -1;
    for (i = 0; i < party->ChampionCount && i < CSB_V1_MAX_CHAMPIONS; i++) {
        if (!csb_v1_champion_is_dead(&party->Champions[i]) &&
            party->Champions[i].CurrentHealth > 0) {
            return i;
        }
    }
    return -1;
}

static uint16_t csb_v1_runtime_normalize_leader_hand_thing(uint16_t thing)
{
    return thing == 0u ? 0xffffu : thing;
}

static uint16_t csb_v1_runtime_export_leader_hand_thing(
    const CSB_V1_RuntimeProfile *profile)
{
    if (!profile) return 0xffffu;
    if (profile->party_state_valid) {
        return csb_v1_runtime_normalize_leader_hand_thing(
            profile->party_state.LeaderHandThing);
    }
    return profile->csbwin_gameblock2_summary_valid
        ? csb_v1_runtime_normalize_leader_hand_thing(
              profile->csbwin_object_in_hand)
        : 0xffffu;
}

static int csb_v1_runtime_direction_from_source_to_destination(
    int source_x,
    int source_y,
    int dest_x,
    int dest_y)
{
    if (source_x == dest_x) return (source_y > dest_y) ? 0 : 2;
    if (source_y == dest_y) return (source_x > dest_x) ? 3 : 1;
    return 0;
}

static int csb_v1_runtime_champion_index_in_cell(
    const CSB_V1_PartyState *party,
    int cell)
{
    int i;
    if (!party) return -1;
    for (i = 0; i < party->ChampionCount && i < CSB_V1_MAX_CHAMPIONS; i++) {
        const CSB_V1_Champion *champion = &party->Champions[i];
        if (((int)champion->Cell & 3) == (cell & 3) &&
            !csb_v1_champion_is_dead(champion) &&
            champion->CurrentHealth > 0) {
            return i;
        }
    }
    return -1;
}

static int csb_v1_runtime_target_champion_for_adjacent_attack(
    const CSB_V1_RuntimeProfile *profile,
    int attacker_x,
    int attacker_y,
    int creature_cell)
{
    static const unsigned char ordered_cells[8][4] = {
        { 0, 1, 3, 2 },
        { 1, 0, 2, 3 },
        { 1, 2, 0, 3 },
        { 2, 1, 3, 0 },
        { 3, 2, 0, 1 },
        { 2, 3, 1, 0 },
        { 0, 3, 1, 2 },
        { 3, 0, 2, 1 }
    };
    int distance_x;
    int distance_y;
    int direction;
    int table_index;
    int i;

    if (!profile || !profile->party_state_valid) return -1;
    distance_x = abs(profile->party_x - attacker_x);
    distance_y = abs(profile->party_y - attacker_y);
    if (distance_x > 1 || distance_y > 1) return -1;

    /* ReDMCSB CHAMPION.C F0286 calls PROJEXPL.C F0229, which derives an
     * ordered four-cell attack list from attacker/party coordinates and the
     * source creature cell, then returns the first living champion in those
     * cells. */
    direction = csb_v1_runtime_direction_from_source_to_destination(
        profile->party_x,
        profile->party_y,
        attacker_x,
        attacker_y);
    table_index = direction << 1;
    if ((table_index & 0x0002) == 0) {
        creature_cell++;
    }
    table_index += (creature_cell >> 1) & 0x0001;
    table_index &= 7;
    for (i = 0; i < 4; i++) {
        int champion_index = csb_v1_runtime_champion_index_in_cell(
            &profile->party_state,
            ordered_cells[table_index][i]);
        if (champion_index >= 0) return champion_index;
    }
    return -1;
}

int csb_v1_runtime_set_party_state(CSB_V1_RuntimeProfile *profile,
                                   const CSB_V1_PartyState *party)
{
    int leader;
    if (!profile || !party) return -1;
    if (party->ChampionCount < 0 ||
        party->ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    profile->party_state = *party;
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(
            profile->party_state.LeaderHandThing);
    profile->party_state_valid = 1;
    profile->champion_count = party->ChampionCount;
    profile->party_dir = party->PartyDirection & 3;
    profile->magic_caster_index = party->MagicCasterIndex;

    leader = party->LeaderIndex;
    if (leader < 0 || leader >= party->ChampionCount ||
        csb_v1_champion_is_dead(&party->Champions[leader]) ||
        party->Champions[leader].CurrentHealth <= 0) {
        leader = csb_v1_runtime_first_living_champion(party);
    }
    profile->leader_index = leader;
    profile->party_state.LeaderIndex = leader;
    return 0;
}

int csb_v1_runtime_get_party_state(const CSB_V1_RuntimeProfile *profile,
                                   CSB_V1_PartyState *out_party)
{
    if (!profile || !out_party || !profile->party_state_valid) return -1;
    *out_party = profile->party_state;
    return out_party->ChampionCount;
}

int csb_v1_runtime_apply_csbwin_gameblock2_summary(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    if (!profile || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT) {
        return -1;
    }
    if (summary->num_character > CSB_V1_MAX_CHAMPIONS ||
        summary->party_x > CSB_V1_MAX_PARTY_X ||
        summary->party_y > CSB_V1_MAX_PARTY_Y ||
        summary->party_facing > 3u ||
        summary->appended_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        summary->appended_size < summary->appended_preserved_size) {
        return -1;
    }

    /* CSBWin SaveGame.cpp lines 1775-1811 applies GAMEBLOCK2 after
     * `swapBlock2()`: time/RNG, party pose, hand/caster indexes, timer
     * metadata, cursor object, and ITEM16 capacity. Firestaff stores this
     * as a bounded startup/resume handoff until the decoded champion,
     * item, and timer bodies are imported into the live runtime. */
    profile->game_time = summary->game_time;
    profile->timeline_queue.gameTick = profile->game_time;
    profile->party_x = (int)summary->party_x;
    profile->party_y = (int)summary->party_y;
    profile->party_z = (int)summary->party_level;
    profile->current_level = (int)summary->party_level;
    csb_v1_dungeon_set_current_level(profile->current_level);
    profile->party_dir = (int)(summary->party_facing & 3u);
    profile->champion_count = (int)summary->num_character;
    profile->leader_index = (summary->hand_char < summary->num_character)
        ? (int)summary->hand_char
        : -1;
    profile->magic_caster_index = (summary->magic_caster < summary->num_character)
        ? (int)summary->magic_caster
        : -1;
    if (profile->party_state_valid) {
        profile->party_state.ChampionCount = profile->champion_count;
        profile->party_state.PartyDirection = (uint8_t)(profile->party_dir & 3);
        profile->party_state.LeaderIndex = profile->leader_index;
        profile->party_state.MagicCasterIndex = profile->magic_caster_index;
    }

    profile->csbwin_gameblock2_summary_valid = 1;
    profile->csbwin_random_seed = summary->random_seed;
    profile->csbwin_object_in_hand = summary->object_in_hand;
    if (profile->party_state_valid) {
        profile->party_state.LeaderHandThing =
            csb_v1_runtime_normalize_leader_hand_thing(
                summary->object_in_hand);
    }
    profile->csbwin_num_timer = summary->num_timer;
    profile->csbwin_first_avail_timer = summary->first_avail_timer;
    profile->csbwin_max_timers = summary->max_timers;
    profile->csbwin_item16_queue_len = summary->item16_queue_len;
    profile->csbwin_max_item16 = summary->max_item16;
    profile->csbwin_timer_sequence = summary->timer_sequence;
    profile->csbwin_last_monster_attack_time =
        summary->last_monster_attack_time;
    profile->csbwin_last_party_move_time = summary->last_party_move_time;
    profile->csbwin_party_move_disable_timer =
        summary->party_move_disable_timer;
    profile->csbwin_word11712 = summary->word11712;
    profile->csbwin_word11714 = summary->word11714;
    profile->csbwin_header_tail_valid = 1;
    memcpy(profile->csbwin_header_byte22808,
           summary->header.public_fields.csbwin_byte22808,
           sizeof(profile->csbwin_header_byte22808));
    profile->csbwin_appended_tail_valid =
        summary->appended_size != 0u ? 1 : 0;
    profile->csbwin_appended_tail_size = summary->appended_size;
    profile->csbwin_appended_tail_preserved_size =
        summary->appended_preserved_size;
    profile->csbwin_appended_tail_fnv1a = summary->appended_fnv1a;
    profile->csbwin_appended_tail_truncated = summary->appended_truncated;
    memset(profile->csbwin_appended_tail, 0,
           sizeof(profile->csbwin_appended_tail));
    if (summary->appended_preserved_size != 0u) {
        memcpy(profile->csbwin_appended_tail,
               summary->appended_preserved,
               summary->appended_preserved_size);
    }
    return 0;
}

static void csb_v1_runtime_copy_csbwin_champion_text(char *dst,
                                                     size_t dst_size,
                                                     const char *src)
{
    if (!dst || dst_size == 0u) return;
    memset(dst, 0, dst_size);
    if (!src) return;
    strncpy(dst, src, dst_size - 1u);
}

static uint16_t csb_v1_runtime_csbwin_attr_to_firestaff_stat(
    const CSB_V1_CSBWin512ChampionSummary *src,
    int csbwin_attr,
    int firestaff_component)
{
    static const int component_map[3] = {
        2, /* Firestaff minimum <- CSBWin ATTRIBUTE.ubMinimum */
        1, /* Firestaff current <- CSBWin ATTRIBUTE.ubCurrent */
        0  /* Firestaff maximum <- CSBWin ATTRIBUTE.ubMaximum */
    };
    if (!src || csbwin_attr < 0 || csbwin_attr >= 7 ||
        firestaff_component < 0 || firestaff_component >= 3) {
        return 0u;
    }
    return (uint16_t)src->attributes[csbwin_attr]
                                      [component_map[firestaff_component]];
}

int csb_v1_runtime_apply_csbwin_champion_summaries(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    static const int attr_to_stat[CSB_V1_STAT_COUNT] = {
        1, /* STR <- CSBWin attribute[1] Strength */
        2, /* DEX <- CSBWin attribute[2] Dexterity */
        3, /* WIS <- CSBWin attribute[3] Wisdom */
        4, /* VIT <- CSBWin attribute[4] Vitality */
        5, /* AntiMagic <- CSBWin attribute[5] */
        6, /* AntiFire <- CSBWin attribute[6] */
        0  /* Luck <- CSBWin attribute[0] */
    };
    int champion_count;
    int champion_index;

    if (!profile || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT ||
        summary->num_character > CSB_V1_MAX_CHAMPIONS ||
        summary->party_facing > 3u) {
        return -1;
    }

    champion_count = (int)summary->num_character;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = champion_count;
    profile->party_state.PartyDirection = (int)(summary->party_facing & 3u);
    profile->party_state.PartyMapX = (int)summary->party_x;
    profile->party_state.PartyMapY = (int)summary->party_y;
    profile->party_state.LeaderIndex =
        (summary->hand_char < summary->num_character)
            ? (int)summary->hand_char
            : -1;
    profile->party_state.MagicCasterIndex =
        (summary->magic_caster < summary->num_character)
            ? (int)summary->magic_caster
            : -1;
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(
            summary->object_in_hand);

    for (champion_index = 0; champion_index < CSB_V1_MAX_CHAMPIONS;
         ++champion_index) {
        CSB_V1_Champion *dst =
            &profile->party_state.Champions[champion_index];
        const CSB_V1_CSBWin512ChampionSummary *src =
            &summary->champions[champion_index];
        int stat_index;
        int skill_index;
        int slot_index;

        csb_v1_champion_init(dst);
        if (champion_index >= champion_count || !src->valid) {
            continue;
        }

        /* CSBWin SaveGame.cpp:1838 swapCharacterData() consumes four
         * CHARDESC records. CSBWin/CSB.h:2486-2597 gives fixed offsets for
         * identity, vitals, attributes, possessions, timers, load, and
         * portrait bytes at offset 336. */
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->Name, sizeof(dst->Name), src->name);
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->Title, sizeof(dst->Title), src->title);
        memcpy(dst->Portrait, src->portrait, sizeof(src->portrait));
        dst->CsbWinWord24 = src->word24;
        dst->CurrentHealth = src->hp;
        dst->MaximumHealth = src->max_hp;
        dst->CurrentStamina = src->stamina;
        dst->MaximumStamina = src->max_stamina;
        dst->CurrentMana = src->mana;
        dst->MaximumMana = src->max_mana;
        dst->CsbWinWord64 = src->word64;
        for (stat_index = 0; stat_index < CSB_V1_STAT_COUNT; ++stat_index) {
            const int csbwin_attr = attr_to_stat[stat_index];
            dst->Statistics[stat_index][CSB_V1_STAT_MIN] =
                csb_v1_runtime_csbwin_attr_to_firestaff_stat(
                    src, csbwin_attr, CSB_V1_STAT_MIN);
            dst->Statistics[stat_index][CSB_V1_STAT_CUR] =
                csb_v1_runtime_csbwin_attr_to_firestaff_stat(
                    src, csbwin_attr, CSB_V1_STAT_CUR);
            dst->Statistics[stat_index][CSB_V1_STAT_MAX] =
                csb_v1_runtime_csbwin_attr_to_firestaff_stat(
                    src, csbwin_attr, CSB_V1_STAT_MAX);
        }
        dst->SkillExperienceValid = 1u;
        for (skill_index = 0;
             skill_index < CSB_V1_FULL_SKILL_COUNT;
             ++skill_index) {
            dst->SkillExperience[skill_index] =
                src->skill_experience[skill_index];
            dst->SkillTemporaryExperience[skill_index] =
                src->skill_temp_adjust[skill_index];
        }
        for (skill_index = 0;
             skill_index < CSB_V1_SKILL_COUNT;
             ++skill_index) {
            dst->Skills[skill_index] =
                (uint8_t)csb_v1_runtime_imported_skill_level(
                    dst,
                    skill_index);
        }
        for (slot_index = 0; slot_index < CSB_V1_SLOT_COUNT; ++slot_index) {
            dst->Slots[slot_index] = src->possessions[slot_index];
        }
        dst->Cell = (uint8_t)(src->char_position & 3u);
        dst->Direction = (uint8_t)(src->facing & 3u);
        dst->DirectionMaximumDamageReceived = src->max_recent_damage;
        dst->CsbWinByte30 = src->byte30;
        dst->CsbWinByte31 = src->byte31;
        memcpy(dst->Incantation, src->incantation, sizeof(dst->Incantation));
        dst->CsbWinByte33 = src->byte33;
        dst->CsbWinFacing3 = src->facing3;
        dst->CsbWinUByte43 = src->ubyte43;
        dst->ActionIndex = (src->attack_type < 0)
            ? CSB_V1_ACTION_NONE
            : (uint8_t)src->attack_type;
        dst->EnableActionEventIndex = src->busy_timer;
        dst->HideDamageReceivedEventIndex = src->timer_index;
        dst->Attributes = (uint16_t)src->char_flags;
        dst->Wounds = (uint16_t)src->wounds;
        dst->PoisonEventCount = src->poison_count;
        dst->Food = src->food;
        dst->Water = src->water;
        dst->Load = src->load;
        dst->ShieldStrength = src->shield_strength;
        dst->Talents = src->talents;
        dst->Fingerprint = src->fingerprint;
        dst->CauseOfDamage = src->cause_of_damage;
        dst->MonsterCausingDamage = src->monster_causing_damage;
        dst->EventIndex = src->timer_index;
    }

    profile->champion_count = champion_count;
    profile->leader_index = profile->party_state.LeaderIndex;
    profile->magic_caster_index = profile->party_state.MagicCasterIndex;
    return 0;
}

int csb_v1_runtime_export_csbwin_champion_summaries(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWin512BodyReport *out_summary)
{
    static const int attr_to_stat[CSB_V1_STAT_COUNT] = {
        1, /* STR -> CSBWin attribute[1] Strength */
        2, /* DEX -> CSBWin attribute[2] Dexterity */
        3, /* WIS -> CSBWin attribute[3] Wisdom */
        4, /* VIT -> CSBWin attribute[4] Vitality */
        5, /* AntiMagic -> CSBWin attribute[5] */
        6, /* AntiFire -> CSBWin attribute[6] */
        0  /* Luck -> CSBWin attribute[0] */
    };
    int champion_count;
    int champion_index;

    if (!profile || !out_summary || !profile->party_state_valid) {
        return -1;
    }

    memset(out_summary, 0, sizeof(*out_summary));
    champion_count = profile->party_state.ChampionCount;
    if (champion_count < 0) champion_count = 0;
    if (champion_count > CSB_V1_MAX_CHAMPIONS) {
        champion_count = CSB_V1_MAX_CHAMPIONS;
    }

    /* CSBWin SaveGame.cpp:1838 writes four CHARDESC records after
     * swapCharacterData(); CSBWin/CSB.h:2486-2597 defines the fixed field
     * order. This export is a bounded runtime summary, not the encrypted
     * 512-byte CSBWin file writer. */
    out_summary->header_valid = 1;
    out_summary->sections_verified = CSB_V1_CSBWIN_512_SECTION_COUNT;
    out_summary->num_character = (uint16_t)champion_count;
    out_summary->party_x = (uint16_t)(profile->party_state.PartyMapX & 0xffff);
    out_summary->party_y = (uint16_t)(profile->party_state.PartyMapY & 0xffff);
    out_summary->party_level = (uint16_t)(profile->current_level & 0xffff);
    out_summary->party_facing =
        (uint16_t)(profile->party_state.PartyDirection & 3);
    out_summary->hand_char =
        (profile->party_state.LeaderIndex >= 0 &&
         profile->party_state.LeaderIndex < champion_count)
            ? (uint16_t)profile->party_state.LeaderIndex
            : 0xffffu;
    out_summary->magic_caster =
        (profile->party_state.MagicCasterIndex >= 0 &&
         profile->party_state.MagicCasterIndex < champion_count)
            ? (uint16_t)profile->party_state.MagicCasterIndex
            : 0xffffu;
    out_summary->object_in_hand = csb_v1_runtime_export_leader_hand_thing(
        profile);

    for (champion_index = 0; champion_index < CSB_V1_MAX_CHAMPIONS;
         ++champion_index) {
        const CSB_V1_Champion *src =
            &profile->party_state.Champions[champion_index];
        CSB_V1_CSBWin512ChampionSummary *dst =
            &out_summary->champions[champion_index];
        int stat_index;
        int skill_index;
        int slot_index;

        if (champion_index >= champion_count) {
            continue;
        }

        dst->valid = 1;
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->name, sizeof(dst->name), src->Name);
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->title, sizeof(dst->title), src->Title);
        dst->word24 = src->CsbWinWord24;
        dst->facing = (uint8_t)(src->Direction & 3u);
        dst->char_position = (uint8_t)(src->Cell & 3u);
        dst->byte30 = src->CsbWinByte30;
        dst->byte31 = src->CsbWinByte31;
        dst->attack_type = (src->ActionIndex == CSB_V1_ACTION_NONE)
            ? -1
            : (int8_t)src->ActionIndex;
        dst->byte33 = src->CsbWinByte33;
        memcpy(dst->incantation, src->Incantation, sizeof(dst->incantation));
        dst->facing3 = src->CsbWinFacing3;
        dst->max_recent_damage =
            (uint8_t)(src->DirectionMaximumDamageReceived & 0xffu);
        dst->poison_count = src->PoisonEventCount;
        dst->ubyte43 = src->CsbWinUByte43;
        dst->busy_timer = src->EnableActionEventIndex;
        dst->timer_index = src->HideDamageReceivedEventIndex;
        dst->char_flags = (int16_t)src->Attributes;
        dst->wounds = (int16_t)src->Wounds;
        dst->hp = src->CurrentHealth;
        dst->max_hp = src->MaximumHealth;
        dst->stamina = src->CurrentStamina;
        dst->max_stamina = src->MaximumStamina;
        dst->mana = src->CurrentMana;
        dst->max_mana = src->MaximumMana;
        dst->word64 = src->CsbWinWord64;
        dst->food = src->Food;
        dst->water = src->Water;
        for (stat_index = 0; stat_index < CSB_V1_STAT_COUNT; ++stat_index) {
            const int csbwin_attr = attr_to_stat[stat_index];
            dst->attributes[csbwin_attr][0] =
                (uint8_t)(src->Statistics[stat_index][CSB_V1_STAT_MAX] &
                          0xffu);
            dst->attributes[csbwin_attr][1] =
                (uint8_t)(src->Statistics[stat_index][CSB_V1_STAT_CUR] &
                          0xffu);
            dst->attributes[csbwin_attr][2] =
                (uint8_t)(src->Statistics[stat_index][CSB_V1_STAT_MIN] &
                          0xffu);
        }
        if (src->SkillExperienceValid) {
            for (skill_index = 0;
                 skill_index < CSB_V1_FULL_SKILL_COUNT;
                 ++skill_index) {
                dst->skill_experience[skill_index] =
                    src->SkillExperience[skill_index];
                dst->skill_temp_adjust[skill_index] =
                    src->SkillTemporaryExperience[skill_index];
            }
        }
        for (slot_index = 0; slot_index < CSB_V1_SLOT_COUNT; ++slot_index) {
            dst->possessions[slot_index] = src->Slots[slot_index];
        }
        dst->load = src->Load;
        dst->shield_strength = src->ShieldStrength;
        dst->talents = src->Talents;
        dst->fingerprint = src->Fingerprint;
        dst->cause_of_damage = src->CauseOfDamage;
        dst->monster_causing_damage = src->MonsterCausingDamage;
        memcpy(dst->portrait, src->Portrait, sizeof(dst->portrait));
    }

    return champion_count;
}

int csb_v1_runtime_apply_csbwin_body_runtime_summaries(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    if (!profile || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT ||
        summary->item16_summary_count >
            CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES ||
        summary->timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        summary->timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES ||
        summary->num_timer > summary->timer_queue_summary_count ||
        summary->num_timer > summary->max_timers ||
        summary->max_timers != summary->timer_summary_count) {
        return -1;
    }

    /* CSBWin SaveGame.cpp:535-543 swapCharacterData() and
     * SaveGame.cpp:1822-1855 body load restore character-tail spell state,
     * ITEM16 active-monster records, timers, and timer queue after the
     * GAMEBLOCK2 handoff. This runtime step is intentionally still a
     * bounded summary copy: it preserves verified decoded state for startup
     * resume while the full event/item materialization remains separate. */
    profile->csbwin_body_runtime_summary_valid = 1;
    profile->csbwin_character_tail_brightness =
        summary->character_tail_brightness;
    profile->csbwin_character_tail_see_thru_walls =
        summary->character_tail_see_thru_walls;
    profile->csbwin_character_tail_magic_footprints_active =
        summary->character_tail_magic_footprints_active;
    profile->csbwin_character_tail_party_shield =
        summary->character_tail_party_shield;
    profile->csbwin_character_tail_fire_shield =
        summary->character_tail_fire_shield;
    profile->csbwin_character_tail_spell_shield =
        summary->character_tail_spell_shield;
    profile->csbwin_character_tail_num_footprint_entries =
        summary->character_tail_num_footprint_entries;
    profile->csbwin_character_tail_freeze_life_timer =
        summary->character_tail_freeze_life_timer;
    profile->csbwin_character_tail_first_magic_footprint =
        summary->character_tail_first_magic_footprint;
    profile->csbwin_character_tail_last_magic_footprint =
        summary->character_tail_last_magic_footprint;
    memcpy(profile->csbwin_character_tail_party_footprints,
           summary->character_tail_party_footprints,
           sizeof(profile->csbwin_character_tail_party_footprints));
    memcpy(profile->csbwin_character_tail_byte13220,
           summary->character_tail_byte13220,
           sizeof(profile->csbwin_character_tail_byte13220));
    profile->csbwin_character_tail_invisible =
        summary->character_tail_invisible;

    profile->csbwin_item16_summary_count = summary->item16_summary_count;
    profile->csbwin_item16_summary_total = summary->item16_summary_total;
    memcpy(profile->csbwin_item16,
           summary->item16,
           sizeof(profile->csbwin_item16));
    profile->csbwin_timer_summary_count = summary->timer_summary_count;
    profile->csbwin_timer_summary_total = summary->timer_summary_total;
    memcpy(profile->csbwin_timers,
           summary->timers,
           sizeof(profile->csbwin_timers));
    profile->csbwin_timer_queue_summary_count =
        summary->timer_queue_summary_count;
    profile->csbwin_timer_queue_summary_total =
        summary->timer_queue_summary_total;
    memcpy(profile->csbwin_timer_queue,
           summary->timer_queue,
           sizeof(profile->csbwin_timer_queue));
    return 0;
}

int csb_v1_runtime_materialize_csbwin_item16_summaries(
    CSB_V1_RuntimeProfile *profile)
{
    uint16_t item_index;
    uint16_t staged_count = 0u;
    CSB_V1_CSBWinRuntimeItem16 staged_items[
        CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES];
    int imported = 0;

    if (!profile || !profile->csbwin_body_runtime_summary_valid) {
        return -1;
    }
    if (profile->csbwin_item16_summary_count >
            CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES ||
        (profile->csbwin_item16_summary_total != 0u &&
         profile->csbwin_item16_summary_total !=
             profile->csbwin_item16_summary_count)) {
        return -1;
    }

    memset(staged_items, 0, sizeof(staged_items));

    /* CSBWin CSB.h:2257-2280 defines ITEM16 as active-monster state:
     * word0 DB4 monster index, packed facings/positions, d.Time low byte,
     * target/previous/current coordinates, and four SINGLE_MONSTER_STATUS
     * bytes. SaveGame.cpp:491-499 swaps only word0 after loading. A negative
     * word0 marks an unused slot, so Firestaff skips 0xffff here while
     * preserving the bounded active records for later AI/runtime ownership. */
    for (item_index = 0u;
         item_index < profile->csbwin_item16_summary_count;
         ++item_index) {
        const CSB_V1_CSBWin512Item16Summary *src =
            &profile->csbwin_item16[item_index];
        CSB_V1_CSBWinRuntimeItem16 *dst;

        /* SaveGame.cpp reads the complete MaxITEM16 stream before runtime
         * ownership. An absent decoded record is corruption, while 0xffff is
         * the source's explicit unused ITEM16 marker. */
        if (!src->valid) {
            return -1;
        }
        if (src->monster_index == 0xffffu) {
            continue;
        }
        if (staged_count >=
            CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES) {
            return -1;
        }

        dst = &staged_items[staged_count];
        memset(dst, 0, sizeof(*dst));
        dst->valid = 1;
        dst->monster_index = src->monster_index;
        dst->live_ai_group_thing = 0xFFFFu;
        dst->live_ai_map_index = -1;
        dst->live_ai_map_x = -1;
        dst->live_ai_map_y = -1;
        dst->facings = src->facings;
        dst->positions = src->positions;
        dst->last_move_time_lsb = src->ubyte4;
        dst->delay_or_flee_timer = src->ubyte5;
        dst->target_x = src->target_x;
        dst->target_y = src->target_y;
        dst->previous_x = src->previous_x;
        dst->previous_y = src->previous_y;
        dst->current_x = src->current_x;
        dst->current_y = src->current_y;
        memcpy(dst->single_monster_status,
               src->single_monster_status,
               sizeof(dst->single_monster_status));
        ++staged_count;
        ++imported;
    }

    /* Do not publish a shortened CSBWin active-monster table. The source
     * body is already checksum-authenticated; this second boundary keeps the
     * decoded summary and live ownership transactionally aligned. */
    memcpy(profile->csbwin_runtime_item16, staged_items,
           sizeof(staged_items));
    profile->csbwin_runtime_item16_count = staged_count;
    profile->csbwin_runtime_item16_total =
        profile->csbwin_item16_summary_total;
    return imported;
}

static uint16_t csb_v1_runtime_csbwin_item16_group_thing(uint16_t monster_index)
{
    if (((monster_index >> 10) & 0x0Fu) == 4u) {
        return monster_index;
    }
    return (uint16_t)((4u << 10) | (monster_index & 0x03FFu));
}

static int csb_v1_runtime_has_c37_for_square(
    const CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y)
{
    int i;

    if (!profile || map_index < 0 || map_x < 0 || map_y < 0) return 0;
    for (i = 0; i < profile->timeline_queue.eventCount; ++i) {
        int event_index = profile->timeline_queue.timeline[i];
        const struct DM1_Event_V1 *event =
            NULL;
        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
            continue;
        }
        event = &profile->timeline_queue.events[event_index];
        if (event->type == DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
            DM1_MAP_TIME_MAP(event->map_time) == (uint8_t)map_index &&
            event->b_mapX == (uint8_t)map_x &&
            event->b_mapY == (uint8_t)map_y) {
            return 1;
        }
    }
    return 0;
}

int csb_v1_runtime_claim_csbwin_item16_ai_ownership(
    CSB_V1_RuntimeProfile *profile)
{
    uint16_t item_index;
    int claimed = 0;

    if (!profile || !profile->dungeon_handle) return 0;

    for (item_index = 0u;
         item_index < profile->csbwin_runtime_item16_count;
         ++item_index) {
        CSB_V1_CSBWinRuntimeItem16 *item =
            &profile->csbwin_runtime_item16[item_index];
        uint16_t group_thing;
        int map_index = -1;
        int map_x = -1;
        int map_y = -1;
        uint8_t *group_record;
        int thing_type;
        int thing_size;
        int creature_type = 0;

        if (!item->valid || item->live_ai_owned) {
            continue;
        }

        group_thing =
            csb_v1_runtime_csbwin_item16_group_thing(item->monster_index);
        if (!csb_v1_runtime_find_group_thing_location(
                profile->dungeon_handle,
                group_thing,
                &map_index,
                &map_x,
                &map_y)) {
            continue;
        }

        item->live_ai_owned = 1;
        item->live_ai_group_thing = group_thing;
        item->live_ai_map_index = map_index;
        item->live_ai_map_x = map_x;
        item->live_ai_map_y = map_y;
        group_record = csb_v1_runtime_mutable_thing_record(
            profile->dungeon_handle,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size > 4) {
            creature_type = (int)group_record[4];
        }
        if (!csb_v1_runtime_has_c37_for_square(
                profile,
                map_index,
                map_x,
                map_y)) {
            csb_v1_runtime_schedule_c37_group_event(
                profile,
                map_index,
                map_x,
                map_y,
                creature_type,
                1u);
            item->live_ai_c37_queued = 1;
        }
        ++claimed;
    }

    /* CSBWin CSB.h ITEM16 stores active-monster records keyed by the DB4
     * monster/group index, while ReDMCSB GROUP.C F0209 resumes live monster
     * behavior through C37 square events.  This bridge claims each decoded
     * ITEM16 whose C04 group thing is still present in the loaded dungeon and
     * ensures there is a C37 owner tick unless the imported timer queue
     * already supplied one for that square. */
    return claimed;
}

static int csb_v1_runtime_csbwin_timer_is_before(
    const CSB_V1_CSBWin512TimerSummary *a, uint16_t index_a,
    const CSB_V1_CSBWin512TimerSummary *b, uint16_t index_b)
{
    const int a_is_parameter_message = a->function == 101u;
    const int b_is_parameter_message = b->function == 101u;

    if (a->time != b->time) return a->time < b->time;
    /* CSBWin Timer.cpp TIMER::operator<:733-770 places parameter messages
     * before every other same-time timer, then uses their byte-5 sequence. */
    if (a_is_parameter_message != b_is_parameter_message) {
        return a_is_parameter_message;
    }
    if (a->function != b->function) {
        return a_is_parameter_message
            ? a->function == 101u
            : a->function > b->function;
    }
    if (a->ubyte5 != b->ubyte5) {
        return a_is_parameter_message
            ? a->ubyte5 < b->ubyte5
            : a->ubyte5 > b->ubyte5;
    }
    if (a->sequence != b->sequence) return a->sequence < b->sequence;
    return index_a <= index_b;
}

/* CSBWin SaveGame.cpp restores a fixed TIMER slot pool (`MaxTimers`) and a
 * separate active TimerQueue (`NumTimer`).  A free slot is not an active
 * timer and must never be projected into M10 merely because it has a saved
 * array index. */
static int csb_v1_runtime_csbwin_timer_pool_counts_valid(
    const CSB_V1_RuntimeProfile *profile)
{
    if (!profile || !profile->csbwin_body_runtime_summary_valid ||
        profile->csbwin_timer_summary_total !=
            profile->csbwin_timer_summary_count ||
        profile->csbwin_timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        profile->csbwin_timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES ||
        profile->csbwin_max_timers != profile->csbwin_timer_summary_count ||
        profile->csbwin_timer_queue_summary_total <
            profile->csbwin_timer_queue_summary_count ||
        profile->csbwin_timer_queue_summary_count < profile->csbwin_num_timer ||
        profile->csbwin_num_timer > profile->csbwin_max_timers ||
        profile->csbwin_first_avail_timer > profile->csbwin_max_timers) {
        return 0;
    }
    return 1;
}

static int csb_v1_runtime_validate_csbwin_timer_heap(
    const CSB_V1_RuntimeProfile *profile)
{
    uint16_t queue_index;

    if (!profile) return 0;
    for (queue_index = 0u;
         queue_index < profile->csbwin_num_timer;
         ++queue_index) {
        const uint16_t timer_index = profile->csbwin_timer_queue[queue_index];
        const CSB_V1_CSBWin512TimerSummary *timer;
        uint16_t child;

        if (timer_index >= profile->csbwin_timer_summary_count) return 0;
        timer = &profile->csbwin_timers[timer_index];
        if (!timer->valid || timer->function == DM1_EVENT_NONE) return 0;
        for (child = (uint16_t)(queue_index * 2u + 1u);
             child < profile->csbwin_num_timer &&
             child <= (uint16_t)(queue_index * 2u + 2u);
             ++child) {
            const uint16_t child_timer_index = profile->csbwin_timer_queue[child];
            const CSB_V1_CSBWin512TimerSummary *child_timer;

            if (child_timer_index >= profile->csbwin_timer_summary_count) {
                return 0;
            }
            child_timer = &profile->csbwin_timers[child_timer_index];
            if (!child_timer->valid ||
                child_timer->function == DM1_EVENT_NONE ||
                csb_v1_runtime_csbwin_timer_is_before(
                    child_timer, child_timer_index, timer, timer_index)) {
                return 0;
            }
        }
    }
    return 1;
}

/* CSBWin Timer.cpp DeleteTimer:912-941 followed by SetTimer:944-1172.
 * A dispatched timer is removed from the serialized heap before its source
 * successor obtains the source allocator's first available TIMER slot. Keep
 * that whole mutation in
 * local arrays: a failed successor must leave both the restored slot pool and
 * Firestaff's event-to-slot receipts unchanged. */
static int csb_v1_runtime_replace_dispatched_csbwin_timer(
    CSB_V1_RuntimeProfile *profile,
    uint16_t consumed_queue_slot,
    uint16_t consumed_timer_index,
    const CSB_V1_CSBWin512TimerSummary *successor,
    int successor_event_index)
{
    CSB_V1_CSBWin512TimerSummary
        staged_timers[CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES];
    uint16_t staged_queue[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES];
    uint16_t staged_slots[DM1_EVENT_MAX_COUNT];
    uint16_t old_count;
    uint16_t last_queue_slot;
    uint16_t next_free;
    uint16_t staged_first_avail;
    uint16_t staged_sequence;
    uint16_t i;

    if (!profile || !successor || successor_event_index < 0 ||
        successor_event_index >= DM1_EVENT_MAX_COUNT ||
        !csb_v1_runtime_csbwin_timer_pool_counts_valid(profile) ||
        consumed_queue_slot >= profile->csbwin_timer_queue_summary_count ||
        consumed_timer_index >= profile->csbwin_timer_summary_count ||
        profile->csbwin_timer_queue[consumed_queue_slot] !=
            consumed_timer_index ||
        !profile->csbwin_timers[consumed_timer_index].valid ||
        profile->csbwin_timers[consumed_timer_index].truncated ||
        profile->csbwin_timers[consumed_timer_index].function ==
            DM1_EVENT_NONE ||
        profile->csbwin_timers[consumed_timer_index].source_index !=
            consumed_timer_index ||
        !successor->valid || successor->truncated ||
        successor->function == DM1_EVENT_NONE ||
        profile->csbwin_timeline_event_queue_slot[successor_event_index] !=
            CSB_V1_CSBWIN_TIMER_QUEUE_NONE) {
        return 0;
    }

    old_count = profile->csbwin_timer_queue_summary_count;
    if (old_count == 0u || old_count > CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES ||
        profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount > DM1_EVENT_MAX_COUNT) {
        return 0;
    }
    memcpy(staged_timers, profile->csbwin_timers, sizeof(staged_timers));
    memcpy(staged_queue, profile->csbwin_timer_queue,
           (size_t)old_count * sizeof(staged_queue[0]));
    memcpy(staged_slots, profile->csbwin_timeline_event_queue_slot,
           sizeof(staged_slots));

    /* DeleteTimer clears only Function, moves the final heap handle into the
     * vacated queue position, and makes the deleted handle eligible for the
     * next SetTimer allocation. */
    staged_timers[consumed_timer_index].function = DM1_EVENT_NONE;
    last_queue_slot = (uint16_t)(old_count - 1u);
    if (consumed_queue_slot != last_queue_slot) {
        staged_queue[consumed_queue_slot] = staged_queue[last_queue_slot];
    }

    for (i = 0u; i < DM1_EVENT_MAX_COUNT; ++i) {
        const struct DM1_Event_V1 *mapped_event;

        if (i == (uint16_t)successor_event_index) continue;
        if (staged_slots[i] == consumed_queue_slot) {
            /* Pre-dispatch TT_60/61 still has its due EVENT in the live
             * timeline. Retire only that exact source receipt as DeleteTimer
             * consumes its handle; any unrelated alias remains invalid. */
            mapped_event = &profile->timeline_queue.events[i];
            if (mapped_event->map_time !=
                    profile->csbwin_timers[consumed_timer_index].time ||
                mapped_event->type !=
                    profile->csbwin_timers[consumed_timer_index].function ||
                mapped_event->priority !=
                    profile->csbwin_timers[consumed_timer_index].ubyte5 ||
                mapped_event->b_mapX !=
                    profile->csbwin_timers[consumed_timer_index].ubyte6 ||
                mapped_event->b_mapY !=
                    profile->csbwin_timers[consumed_timer_index].ubyte7 ||
                mapped_event->c_cell !=
                    profile->csbwin_timers[consumed_timer_index].ubyte8 ||
                mapped_event->c_effect !=
                    profile->csbwin_timers[consumed_timer_index].ubyte9) {
                return 0;
            }
            staged_slots[i] = CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
        }
        if (consumed_queue_slot != last_queue_slot &&
            staged_slots[i] == last_queue_slot) {
            staged_slots[i] = consumed_queue_slot;
        }
    }

    /* DeleteTimer changes firstAvail only when the released handle precedes
     * it. SetTimer consumes that exact cursor; it does not rescan from slot
     * zero and thereby alter the saved allocator's deterministic ownership. */
    staged_first_avail = profile->csbwin_first_avail_timer;
    if (consumed_timer_index < staged_first_avail) {
        staged_first_avail = consumed_timer_index;
    }
    next_free = staged_first_avail;
    if (next_free >= profile->csbwin_max_timers ||
        staged_timers[next_free].function != DM1_EVENT_NONE) {
        return 0;
    }

    staged_timers[next_free] = *successor;
    staged_timers[next_free].valid = 1;
    staged_timers[next_free].truncated = 0;
    staged_timers[next_free].source_index = next_free;
    staged_sequence = profile->csbwin_timer_sequence;
    staged_timers[next_free].sequence = staged_sequence;
    staged_queue[last_queue_slot] = next_free;
    staged_slots[successor_event_index] = last_queue_slot;

    staged_first_avail = profile->csbwin_max_timers;
    for (i = (uint16_t)(next_free + 1u); i < profile->csbwin_max_timers; ++i) {
        if (staged_timers[i].function == DM1_EVENT_NONE) {
            staged_first_avail = i;
            break;
        }
    }
    memcpy(profile->csbwin_timers, staged_timers, sizeof(staged_timers));
    memcpy(profile->csbwin_timer_queue, staged_queue,
           (size_t)old_count * sizeof(staged_queue[0]));
    memcpy(profile->csbwin_timeline_event_queue_slot, staged_slots,
           sizeof(staged_slots));
    profile->csbwin_timer_sequence = (uint16_t)(staged_sequence + 1u);
    profile->csbwin_first_avail_timer = staged_first_avail;
    return 1;
}

/* Timer.cpp DeleteTimer removes the dispatched handle and SetTimer assigns a
 * fresh first-available handle before AdjustTimerQueue restores heap order.
 * Once a source transaction has staged that exact ownership change, rebuild
 * only the serialized heap topology and its event-to-slot receipts. */
static int csb_v1_runtime_reheapify_live_csbwin_timer_queue(
    CSB_V1_RuntimeProfile *profile)
{
    uint16_t staged_queue[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES];
    uint16_t staged_slots[DM1_EVENT_MAX_COUNT];
    uint8_t seen_timers[CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES] = { 0 };
    int event_ordinal;
    uint16_t i;

    if (!csb_v1_runtime_csbwin_timer_pool_counts_valid(profile) ||
        profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount !=
            (int)profile->csbwin_timer_queue_summary_count) {
        return 0;
    }

    memcpy(staged_queue, profile->csbwin_timer_queue,
           (size_t)profile->csbwin_timer_queue_summary_count *
               sizeof(staged_queue[0]));
    for (i = 0u; i < DM1_EVENT_MAX_COUNT; ++i) {
        staged_slots[i] = CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    }

    if (profile->csbwin_timer_queue_summary_count != 0u) {
        const uint16_t timer_index = staged_queue[0];
        if (timer_index >= profile->csbwin_timer_summary_count ||
            !profile->csbwin_timers[timer_index].valid ||
            profile->csbwin_timers[timer_index].truncated ||
            profile->csbwin_timers[timer_index].function == DM1_EVENT_NONE ||
            profile->csbwin_timers[timer_index].source_index != timer_index) {
            return 0;
        }
        seen_timers[timer_index] = 1u;
    }

    /* The restored queue contains each active timer exactly once. Rebuild it
     * from the retained entries using CSBWin TIMER::operator< ordering. */
    for (i = 1u; i < profile->csbwin_timer_queue_summary_count; ++i) {
        const uint16_t timer_index = staged_queue[i];
        uint16_t position = i;

        if (timer_index >= profile->csbwin_timer_summary_count ||
            !profile->csbwin_timers[timer_index].valid ||
            profile->csbwin_timers[timer_index].truncated ||
            profile->csbwin_timers[timer_index].function == DM1_EVENT_NONE ||
            profile->csbwin_timers[timer_index].source_index != timer_index ||
            seen_timers[timer_index]) {
            return 0;
        }
        seen_timers[timer_index] = 1u;
        while (position > 0u) {
            const uint16_t parent = (uint16_t)((position - 1u) / 2u);
            const uint16_t parent_index = staged_queue[parent];
            if (parent_index >= profile->csbwin_timer_summary_count ||
                !csb_v1_runtime_csbwin_timer_is_before(
                    &profile->csbwin_timers[timer_index], timer_index,
                    &profile->csbwin_timers[parent_index], parent_index)) {
                break;
            }
            staged_queue[position] = parent_index;
            position = parent;
        }
        staged_queue[position] = timer_index;
    }
    for (event_ordinal = 0;
         event_ordinal < profile->timeline_queue.eventCount;
         ++event_ordinal) {
        const int event_index = profile->timeline_queue.timeline[event_ordinal];
        const struct DM1_Event_V1 *event;
        uint16_t previous_slot;
        uint16_t timer_index;
        uint16_t queue_slot;

        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) return 0;
        previous_slot = profile->csbwin_timeline_event_queue_slot[event_index];
        if (previous_slot >= profile->csbwin_timer_queue_summary_count) return 0;
        timer_index = profile->csbwin_timer_queue[previous_slot];
        if (timer_index >= profile->csbwin_timer_summary_count) return 0;
        for (queue_slot = 0u;
             queue_slot < profile->csbwin_timer_queue_summary_count;
             ++queue_slot) {
            if (staged_queue[queue_slot] == timer_index) break;
        }
        if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
            staged_slots[event_index] != CSB_V1_CSBWIN_TIMER_QUEUE_NONE) {
            return 0;
        }
        event = &profile->timeline_queue.events[event_index];
        if (event->map_time != profile->csbwin_timers[timer_index].time ||
            event->type != profile->csbwin_timers[timer_index].function ||
            event->priority != profile->csbwin_timers[timer_index].ubyte5 ||
            event->b_mapX != profile->csbwin_timers[timer_index].ubyte6 ||
            event->b_mapY != profile->csbwin_timers[timer_index].ubyte7 ||
            event->c_cell != profile->csbwin_timers[timer_index].ubyte8 ||
            event->c_effect != profile->csbwin_timers[timer_index].ubyte9) {
            return 0;
        }
        staged_slots[event_index] = queue_slot;
    }

    memcpy(profile->csbwin_timer_queue, staged_queue,
           (size_t)profile->csbwin_timer_queue_summary_count *
               sizeof(staged_queue[0]));
    memcpy(profile->csbwin_timeline_event_queue_slot, staged_slots,
           sizeof(staged_slots));
    return 1;
}

int csb_v1_runtime_materialize_csbwin_timer_queue(
    CSB_V1_RuntimeProfile *profile)
{
    uint16_t queue_index;
    struct DM1_EventQueue_V1 staged_queue;
    uint16_t staged_slots[DM1_EVENT_MAX_COUNT];
    int imported = 0;

    if (!profile || !profile->csbwin_body_runtime_summary_valid) {
        return -1;
    }
    if (profile->csbwin_timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES ||
        profile->csbwin_timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        (profile->csbwin_timer_summary_total != 0u &&
         profile->csbwin_timer_summary_total !=
             profile->csbwin_timer_summary_count) ||
        (profile->csbwin_timer_queue_summary_total != 0u &&
         profile->csbwin_timer_queue_summary_total !=
             profile->csbwin_timer_queue_summary_count)) {
        return -1;
    }
    /* CSBWin Timer.cpp CheckTimers:884-906 rejects a saved queue if any
     * child precedes its parent. Validate the complete source heap before
     * staging so a malformed or reordered original-save queue cannot publish
     * a partially rebuilt live timeline. */
    if (!csb_v1_runtime_validate_csbwin_timer_heap(profile)) return -1;

    /* CSBWin Timer.cpp:728-772 orders timers by full m_time, then
     * timerFunction, then m_timerUByte5, then m_timerSequence when enabled.
     * The decoded CSBWin timer queue already captures the source order; this
     * handoff rebuilds Firestaff's timeline heap from that queue, preserving
     * every serialized timer slot. Do not run a restored queue through
     * GameTimers::SetTimer policy: CSBWin has already accepted these entries
     * before SaveGame.cpp writes them. Unsupported side effects remain harmless
     * dispatch records until their runtime handlers are implemented. */
    dm1v1_event_queue_init(&staged_queue, profile->game_time);
    for (queue_index = 0u; queue_index < DM1_EVENT_MAX_COUNT; ++queue_index) {
        staged_slots[queue_index] =
            CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    }
    /* CSBWin Timer.cpp DeleteTimer:912-941 keeps the active heap in the
     * TimerQueue prefix [0..NumTimer): slots beyond NumTimer are free
     * handles, not live timers. SaveGame.cpp:1867/1887/1906 likewise walks
     * only m_numTimer entries after load. Materialize the source-owned
     * active prefix, never the full MaxTimer pool storage. */
    if (profile->csbwin_num_timer >
        profile->csbwin_timer_queue_summary_count) {
        return -1;
    }
    for (queue_index = 0u;
         queue_index < profile->csbwin_num_timer;
         ++queue_index) {
        uint16_t timer_index = profile->csbwin_timer_queue[queue_index];
        const CSB_V1_CSBWin512TimerSummary *timer;
        struct DM1_Event_V1 event;

        if (timer_index >= profile->csbwin_timer_summary_count) {
            return -1;
        }
        timer = &profile->csbwin_timers[timer_index];
        if (!timer->valid || timer->function == DM1_EVENT_NONE) {
            return -1;
        }

        memset(&event, 0, sizeof(event));
        /* CSBWin's serialized TIMER time word already carries the source
         * level/time representation consumed by ProcessTimers. The decoded
         * level field is a receipt for LoadLevel, not a replacement high byte
         * for the original timer word. */
        event.map_time = timer->time;
        event.type = timer->function;
        event.priority = timer->ubyte5;
        event.b_mapX = timer->ubyte6;
        event.b_mapY = timer->ubyte7;
        event.c_cell = timer->ubyte8;
        event.c_effect = timer->ubyte9;
        {
            int event_index = csb_v1_runtime_append_unmerged_map_timer_to_queue(
                &staged_queue, &event);
            if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
                return -1;
            }
            staged_slots[event_index] = queue_index;
            ++imported;
        }
    }
    profile->timeline_queue = staged_queue;
    memcpy(profile->csbwin_timeline_event_queue_slot, staged_slots,
           sizeof(staged_slots));
    return imported;
}

static void csb_v1_runtime_reset_csbwin_extended_metadata(
    CSB_V1_RuntimeProfile *profile)
{
    if (!profile) return;
    profile->csbwin_extended_game_info = NULL;
    profile->csbwin_extended_game_info_size = 0u;
    profile->csbwin_extended_game_info_fnv1a = 0u;
    profile->csbwin_extended_features_valid = 0;
    memset(&profile->csbwin_last_dsa_execution_receipt, 0,
           sizeof(profile->csbwin_last_dsa_execution_receipt));
    profile->csbwin_last_saved_timer_dsa_valid = 0;
    profile->csbwin_last_saved_timer_dsa_queue_slot =
        CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    profile->csbwin_last_saved_timer_dsa_timer_index =
        CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    profile->csbwin_last_saved_timer_dsa_id = 0xffu;
    profile->csbwin_last_saved_timer_dsa_state_index = 0u;
    profile->csbwin_last_saved_timer_dsa_column = 0u;
    profile->csbwin_last_saved_timer_dsa_action_ordinal = -1;
    profile->csbwin_extended_features_version = 0u;
    profile->csbwin_extended_features_flags = 0u;
    profile->csbwin_extended_features_flags32 = 0u;
    profile->csbwin_extended_editing_options = 0u;
    profile->csbwin_extended_cell_flag_array_size = 0u;
    profile->csbwin_extended_graphics_signature1 = 0u;
    profile->csbwin_extended_graphics_signature2 = 0u;
    profile->csbwin_extended_spell_filter_location = 0u;
    profile->csbwin_extended_overlay_ordinal = 0;
    profile->csbwin_extended_overlay_p1 = 0;
    profile->csbwin_extended_overlay_p2 = 0;
    profile->csbwin_extended_overlay_p3 = 0;
    profile->csbwin_extended_overlay_p4 = 0;
    profile->csbwin_extended_csbgraphics_signature1 = 0u;
    profile->csbwin_extended_csbgraphics_signature2 = 0u;
    memset(profile->csbwin_extended_hint_key, 0,
           sizeof(profile->csbwin_extended_hint_key));
    profile->csbwin_extended_level_index_present = 0;
    memset(profile->csbwin_extended_level_dsa_index, 0xff,
           sizeof(profile->csbwin_extended_level_dsa_index));
}

static void csb_v1_runtime_cleanup_csbwin_extended_state(
    CSB_V1_RuntimeProfile *profile)
{
    if (!profile) return;
    csb_v1_chaos_cleanup(&profile->csbwin_extended_dsa_state);
    free(profile->csbwin_extended_game_info);
    csb_v1_runtime_reset_csbwin_extended_metadata(profile);
}

static int csb_v1_runtime_csbwin_inventory_thing_is_null(uint16_t thing)
{
    /* RNnul is 0xffff in CSBWin. Older Firestaff summaries also used zero
     * for an empty slot, so accept both representations at this boundary. */
    return thing == 0u || thing == THING_NONE;
}

static int csb_v1_runtime_validate_csbwin_inventory_ownership(
    const CSB_V1_CSBWin512BodyReport *summary)
{
    uint16_t champion_index;
    uint16_t slot_index;

    if (!summary || summary->num_character > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    /* CSBWin SaveGame.cpp:1023-1032 removes the cursor RN's weight from
     * d.CH16482[d.HandChar] before serializing GAMEBLOCK2. On restore,
     * SaveGame.cpp:1802-1808 restores that same RN and CSBCode.cpp:6830-6865
     * puts it back on the cursor owner. A non-null cursor RN without a
     * declared saved champion owner is therefore not a usable save atom. */
    if (!csb_v1_runtime_csbwin_inventory_thing_is_null(
            summary->object_in_hand)) {
        if (summary->hand_char >= summary->num_character ||
            !summary->champions[summary->hand_char].valid ||
            summary->object_in_hand == THING_ENDOFLIST) {
            return -1;
        }
    }

    /* CHARDESC::possessions is the source-owned C00..C29 inventory. Keep
     * each raw RN intact here; its dungeon-record identity is resolved only
     * after the matching original dungeon is live. This gate must not invent
     * a separate object database from a save summary, but an end-of-list
     * sentinel is never a serializable inventory object. */
    for (champion_index = 0u;
         champion_index < summary->num_character;
         ++champion_index) {
        const CSB_V1_CSBWin512ChampionSummary *champion =
            &summary->champions[champion_index];

        if (!champion->valid) return -1;
        for (slot_index = 0u;
             slot_index < CSB_V1_SLOT_COUNT;
             ++slot_index) {
            uint16_t thing = champion->possessions[slot_index];

            if (csb_v1_runtime_csbwin_inventory_thing_is_null(thing)) {
                continue;
            }
            if (thing == THING_ENDOFLIST) return -1;
        }
    }
    return 0;
}

static int csb_v1_runtime_stage_csbwin_resume_report(
    CSB_V1_RuntimeProfile *candidate,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    uint16_t champion_index;
    uint16_t queue_index;

    if (!candidate || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT ||
        summary->num_character > CSB_V1_MAX_CHAMPIONS ||
        summary->party_x > CSB_V1_MAX_PARTY_X ||
        summary->party_y > CSB_V1_MAX_PARTY_Y ||
        summary->party_facing > 3u ||
        summary->item16_summary_count >
            CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES ||
        summary->timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        summary->timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES) {
        return -1;
    }

    /* CSBWin SaveGame.cpp:1838-1867 reads every declared CHARDESC and
     * TimerQueue entry after their checked stream sections.  Do not quietly
     * turn a malformed decoded reference into a shorter live queue. */
    for (champion_index = 0u;
         champion_index < summary->num_character;
         ++champion_index) {
        if (!summary->champions[champion_index].valid) {
            return -1;
        }
    }
    for (queue_index = 0u;
         queue_index < summary->num_timer;
         ++queue_index) {
        if (summary->timer_queue[queue_index] >=
            summary->timer_summary_count) {
            return -1;
        }
    }
    if (csb_v1_runtime_validate_csbwin_inventory_ownership(summary) != 0) {
        return -1;
    }

    /* CSBWin SaveGame.cpp:1768-1855 loads GAMEBLOCK2, ITEM16,
     * character data, timers, then the timer queue. Firestaff keeps the
     * lower-level handoff helpers testable, but startup/resume callers stage
     * the complete ordered handoff before publishing it. GAMEBLOCK2 also
     * updates the shared current-dungeon level, so restore that singleton if
     * a later candidate step fails. */
    if (csb_v1_runtime_apply_csbwin_gameblock2_summary(
            candidate, summary) != 0) {
        return -1;
    }
    if (csb_v1_runtime_apply_csbwin_champion_summaries(
            candidate, summary) != 0) {
        return -1;
    }
    if (csb_v1_runtime_apply_csbwin_body_runtime_summaries(
            candidate, summary) != 0) {
        return -1;
    }
    if (csb_v1_runtime_materialize_csbwin_item16_summaries(candidate) < 0) {
        return -1;
    }
    if (csb_v1_runtime_materialize_csbwin_timer_queue(candidate) < 0) {
        return -1;
    }
    return 0;
}

static int csb_v1_runtime_stage_csbwin_extended_state(
    CSB_V1_RuntimeProfile *candidate,
    const uint8_t *bytes,
    size_t size,
    const CSB_V1_CSBWinExtendedFeaturesReport *features,
    const CSB_V1_CSBWinExtendedTailReport *tail)
{
    char *game_info;

    if (!candidate || !bytes || !features || !tail || !features->valid ||
        !tail->valid ||
        tail->game_info_offset > size ||
        tail->game_info_size > size - tail->game_info_offset) {
        return -1;
    }
    game_info = (char *)malloc((size_t)tail->game_info_size + 1u);
    if (!game_info) return -1;
    memcpy(game_info, bytes + tail->game_info_offset, tail->game_info_size);
    game_info[tail->game_info_size] = '\0';

    /* CSBWin SaveGame.cpp:211-260 authenticates DSA records before it
     * publishes gameInfo. DSA.cpp:5637-5798 defines those records. Keep the
     * imported action words opaque until a source-faithful interpreter exists. */
    csb_v1_chaos_init(&candidate->csbwin_extended_dsa_state);
    if (csb_v1_chaos_import_extended_save_dsas(
            &candidate->csbwin_extended_dsa_state, bytes, (int)size) < 0) {
        free(game_info);
        return -1;
    }
    candidate->csbwin_extended_features_valid = 1;
    candidate->csbwin_extended_features_version = features->version;
    candidate->csbwin_extended_features_flags = features->flags;
    candidate->csbwin_extended_features_flags32 = features->extended_flags;
    candidate->csbwin_extended_editing_options = features->editing_options;
    candidate->csbwin_extended_cell_flag_array_size =
        features->cell_flag_array_size;
    candidate->csbwin_extended_graphics_signature1 =
        features->graphics_signature1;
    candidate->csbwin_extended_graphics_signature2 =
        features->graphics_signature2;
    candidate->csbwin_extended_spell_filter_location =
        features->spell_filter_location;
    candidate->csbwin_extended_overlay_ordinal = features->overlay_ordinal;
    candidate->csbwin_extended_overlay_p1 = features->overlay_p1;
    candidate->csbwin_extended_overlay_p2 = features->overlay_p2;
    candidate->csbwin_extended_overlay_p3 = features->overlay_p3;
    candidate->csbwin_extended_overlay_p4 = features->overlay_p4;
    candidate->csbwin_extended_csbgraphics_signature1 =
        features->csbgraphics_signature1;
    candidate->csbwin_extended_csbgraphics_signature2 =
        features->csbgraphics_signature2;
    memcpy(candidate->csbwin_extended_hint_key, features->hint_key,
           sizeof(candidate->csbwin_extended_hint_key));
    candidate->csbwin_extended_game_info = game_info;
    candidate->csbwin_extended_game_info_size = tail->game_info_size;
    candidate->csbwin_extended_game_info_fnv1a = tail->game_info_fnv1a;
    candidate->csbwin_extended_level_index_present =
        tail->level_index_present;
    memcpy(candidate->csbwin_extended_level_dsa_index,
           tail->level_dsa_index,
           sizeof(candidate->csbwin_extended_level_dsa_index));
    return 0;
}

int csb_v1_runtime_apply_csbwin_resume_report(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    CSB_V1_RuntimeProfile candidate;
    int previous_dungeon_level;

    if (!profile || !summary) return -1;
    candidate = *profile;
    previous_dungeon_level = csb_v1_dungeon_get_current_level();
    if (csb_v1_runtime_stage_csbwin_resume_report(&candidate, summary) != 0) {
        csb_v1_dungeon_set_current_level(previous_dungeon_level);
        return -1;
    }
    /* A core-only report has no Extended Features preamble. CSBWin clears its
     * DSA/game-info/index owners before loading the save body. */
    csb_v1_chaos_init(&candidate.csbwin_extended_dsa_state);
    csb_v1_runtime_reset_csbwin_extended_metadata(&candidate);
    csb_v1_runtime_cleanup_csbwin_extended_state(profile);
    *profile = candidate;
    (void)csb_v1_runtime_claim_csbwin_item16_ai_ownership(profile);
    return 0;
}

int csb_v1_runtime_apply_csbwin_resume_file(
    CSB_V1_RuntimeProfile *profile,
    const char *path,
    size_t max_size)
{
    enum { DEFAULT_MAX_BYTES = 4 * 1024 * 1024 };
    FILE *fp;
    long file_size_long;
    size_t file_size;
    uint8_t *bytes;
    size_t got;
    int rc;
    CSB_V1_CSBWin512BodyReport report;
    CSB_V1_CSBWinExtendedFeaturesReport features;
    CSB_V1_CSBWinExtendedDSAReport dsa;
    CSB_V1_CSBWinExtendedTailReport tail;
    CSB_V1_RuntimeProfile candidate;
    size_t core_offset = 0u;
    int previous_dungeon_level;

    if (!profile || !path || path[0] == '\0') {
        return -1;
    }
    if (max_size == 0u) {
        max_size = (size_t)DEFAULT_MAX_BYTES;
    }

    fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    file_size_long = ftell(fp);
    if (file_size_long < 0) {
        fclose(fp);
        return -1;
    }
    file_size = (size_t)file_size_long;
    if (file_size > max_size) {
        fclose(fp);
        return -1;
    }
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    bytes = (uint8_t *)malloc(file_size > 0u ? file_size : 1u);
    if (!bytes) {
        fclose(fp);
        return -1;
    }
    got = fread(bytes, 1u, file_size, fp);
    fclose(fp);
    if (got != file_size) {
        free(bytes);
        return -1;
    }

    memset(&features, 0, sizeof(features));
    memset(&dsa, 0, sizeof(dsa));
    memset(&tail, 0, sizeof(tail));
    rc = csb_v1_csbwin_512_inspect_extended_tail(
        bytes, file_size, &tail, &dsa, &features);
    if (rc == CSB_V1_CSBWIN_EXTENDED_ABSENT) {
        core_offset = 0u;
    } else if (rc == CSB_V1_CSBWIN_EXTENDED_OK && tail.valid &&
               tail.next_payload_offset <= file_size) {
        core_offset = tail.next_payload_offset;
    } else {
        free(bytes);
        return -1;
    }
    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbwin_512_verify_save_body(bytes + core_offset,
                                             file_size - core_offset,
                                             0u, &report);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        free(bytes);
        return -1;
    }
    candidate = *profile;
    previous_dungeon_level = csb_v1_dungeon_get_current_level();
    if (csb_v1_runtime_stage_csbwin_resume_report(&candidate, &report) != 0) {
        csb_v1_dungeon_set_current_level(previous_dungeon_level);
        free(bytes);
        return -1;
    }
    if (core_offset != 0u) {
        if (csb_v1_runtime_stage_csbwin_extended_state(
                &candidate, bytes, file_size, &features, &tail) != 0) {
            csb_v1_runtime_cleanup_csbwin_extended_state(&candidate);
            csb_v1_dungeon_set_current_level(previous_dungeon_level);
            free(bytes);
            return -1;
        }
    } else {
        csb_v1_chaos_init(&candidate.csbwin_extended_dsa_state);
        csb_v1_runtime_reset_csbwin_extended_metadata(&candidate);
    }
    free(bytes);
    csb_v1_runtime_cleanup_csbwin_extended_state(profile);
    *profile = candidate;
    (void)csb_v1_runtime_claim_csbwin_item16_ai_ownership(profile);
    return 0;
}

static int csb_v1_runtime_build_csbwin_core_summary(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWin512BodyReport *summary)
{
    int rc;
    uint16_t i;
    uint16_t timer_count;
    uint16_t item_count;

    if (!profile || !summary || !profile->party_state_valid) {
        return -1;
    }

    rc = csb_v1_runtime_export_csbwin_champion_summaries(profile, summary);
    if (rc < 0) {
        return -1;
    }

    summary->header_valid = 1;
    summary->header.verdict = CSB_V1_CSBWIN_512_VERDICT_CSB;
    summary->header.key_index = CSB_V1_CSBWIN_512_KEY_CSB;
    summary->timer_record_size = 16u;
    summary->header.public_fields.csbwin_random_game_id =
        (uint32_t)csb_v1_runtime_effective_game_id(profile);
    summary->header.public_fields.csbwin_total_move_count =
        profile->tick_count;
    if (profile->csbwin_header_tail_valid) {
        memcpy(summary->header.public_fields.csbwin_byte22808,
               profile->csbwin_header_byte22808,
               sizeof(summary->header.public_fields.csbwin_byte22808));
    }
    if (profile->csbwin_appended_tail_valid) {
        if (profile->csbwin_appended_tail_preserved_size >
                CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
            profile->csbwin_appended_tail_size !=
                profile->csbwin_appended_tail_preserved_size) {
            return -1;
        }
        summary->appended_offset = 0u;
        summary->appended_size = profile->csbwin_appended_tail_size;
        summary->appended_preserved_size =
            profile->csbwin_appended_tail_preserved_size;
        summary->appended_fnv1a = profile->csbwin_appended_tail_fnv1a;
        summary->appended_truncated =
            profile->csbwin_appended_tail_truncated;
        memcpy(summary->appended_preserved,
               profile->csbwin_appended_tail,
               summary->appended_preserved_size);
    }
    summary->game_time = profile->game_time;
    summary->random_seed = profile->csbwin_gameblock2_summary_valid
        ? profile->csbwin_random_seed
        : profile->dungeon_seed;
    summary->object_in_hand = csb_v1_runtime_export_leader_hand_thing(profile);
    summary->last_monster_attack_time =
        profile->csbwin_last_monster_attack_time;
    summary->last_party_move_time = profile->csbwin_last_party_move_time;
    summary->party_move_disable_timer =
        profile->csbwin_party_move_disable_timer;
    summary->word11712 = profile->csbwin_word11712;
    summary->word11714 = profile->csbwin_word11714;

    if (profile->csbwin_body_runtime_summary_valid) {
        summary->character_tail_brightness =
            profile->csbwin_character_tail_brightness;
        summary->character_tail_see_thru_walls =
            profile->csbwin_character_tail_see_thru_walls;
        summary->character_tail_magic_footprints_active =
            profile->csbwin_character_tail_magic_footprints_active;
        summary->character_tail_party_shield =
            profile->csbwin_character_tail_party_shield;
        summary->character_tail_fire_shield =
            profile->csbwin_character_tail_fire_shield;
        summary->character_tail_spell_shield =
            profile->csbwin_character_tail_spell_shield;
        summary->character_tail_num_footprint_entries =
            profile->csbwin_character_tail_num_footprint_entries;
        summary->character_tail_freeze_life_timer =
            profile->csbwin_character_tail_freeze_life_timer;
        summary->character_tail_first_magic_footprint =
            profile->csbwin_character_tail_first_magic_footprint;
        summary->character_tail_last_magic_footprint =
            profile->csbwin_character_tail_last_magic_footprint;
        memcpy(summary->character_tail_party_footprints,
               profile->csbwin_character_tail_party_footprints,
               sizeof(summary->character_tail_party_footprints));
        memcpy(summary->character_tail_byte13220,
               profile->csbwin_character_tail_byte13220,
               sizeof(summary->character_tail_byte13220));
        summary->character_tail_invisible =
            profile->csbwin_character_tail_invisible;
    }

    item_count = profile->csbwin_body_runtime_summary_valid
        ? profile->csbwin_item16_summary_total
        : profile->csbwin_runtime_item16_total;
    if (item_count > CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES) {
        return -1;
    }
    summary->max_item16 = item_count;
    summary->item16_queue_len = item_count;
    summary->item16_summary_total = item_count;
    summary->item16_summary_count = item_count;
    if (profile->csbwin_body_runtime_summary_valid) {
        memcpy(summary->item16,
               profile->csbwin_item16,
               (size_t)item_count * sizeof(summary->item16[0]));
    } else {
        for (i = 0u; i < item_count; ++i) {
            const CSB_V1_CSBWinRuntimeItem16 *src =
                &profile->csbwin_runtime_item16[i];
            CSB_V1_CSBWin512Item16Summary *dst = &summary->item16[i];
            if (!src->valid) {
                continue;
            }
            dst->valid = 1;
            dst->monster_index = src->monster_index;
            dst->facings = src->facings;
            dst->positions = src->positions;
            dst->ubyte4 = src->last_move_time_lsb;
            dst->ubyte5 = src->delay_or_flee_timer;
            dst->target_x = src->target_x;
            dst->target_y = src->target_y;
            dst->previous_x = src->previous_x;
            dst->previous_y = src->previous_y;
            dst->current_x = src->current_x;
            dst->current_y = src->current_y;
            memcpy(dst->single_monster_status,
                   src->single_monster_status,
                   sizeof(dst->single_monster_status));
        }
    }

    if (profile->csbwin_body_runtime_summary_valid) {
        uint8_t seen[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES] = { 0 };

        /* A resumed CSBWin save owns both TIMER and TimerQueue. Preserve
         * those bytes only while every live timeline event still has its
         * exact original queue-slot receipt. Once a timer has fired or a new
         * event has been introduced, this runtime cannot reconstruct the
         * source heap's free-list/requeue state, so do not emit a plausible
         * but invented replacement save. */
        if (!csb_v1_runtime_csbwin_timer_pool_counts_valid(profile) ||
            profile->timeline_queue.eventCount < 0 ||
            profile->timeline_queue.eventCount > DM1_EVENT_MAX_COUNT ||
            profile->timeline_queue.eventCount !=
                (int)profile->csbwin_timer_queue_summary_count) {
            return -1;
        }
        /* CSBWin SaveGame.cpp writes the TIMER array together with its
         * TimerQueue heap. A live event can still match its source slot
         * after a timer mutation, while the retained serialized heap no
         * longer orders that timer correctly. Do not emit that plausible
         * but invalid resume artifact. */
        if (!csb_v1_runtime_validate_csbwin_timer_heap(profile)) {
            return -1;
        }
        for (i = 0u; i < (uint16_t)profile->timeline_queue.eventCount; ++i) {
            const int event_index = profile->timeline_queue.timeline[i];
            const struct DM1_Event_V1 *event;
            uint16_t queue_slot;
            uint16_t timer_index;
            const CSB_V1_CSBWin512TimerSummary *timer;

            if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
                return -1;
            }
            queue_slot = profile->csbwin_timeline_event_queue_slot[event_index];
            if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
                seen[queue_slot]) {
                return -1;
            }
            timer_index = profile->csbwin_timer_queue[queue_slot];
            if (timer_index >= profile->csbwin_timer_summary_count) {
                return -1;
            }
            event = &profile->timeline_queue.events[event_index];
            timer = &profile->csbwin_timers[timer_index];
            if (!timer->valid || timer->truncated ||
                timer->source_index != timer_index ||
                event->map_time != timer->time ||
                event->type != timer->function ||
                event->priority != timer->ubyte5 ||
                event->b_mapX != timer->ubyte6 ||
                event->b_mapY != timer->ubyte7 ||
                event->c_cell != timer->ubyte8 ||
                event->c_effect != timer->ubyte9) {
                return -1;
            }
            seen[queue_slot] = 1u;
        }
        for (i = 0u; i < profile->csbwin_timer_queue_summary_count; ++i) {
            if (!seen[i]) return -1;
        }

        summary->max_timers = profile->csbwin_max_timers;
        summary->num_timer = profile->csbwin_num_timer;
        summary->first_avail_timer = profile->csbwin_first_avail_timer;
        summary->timer_sequence = profile->csbwin_timer_sequence;
        summary->timer_summary_total = profile->csbwin_timer_summary_total;
        summary->timer_summary_count = profile->csbwin_timer_summary_count;
        summary->timer_queue_summary_total =
            profile->csbwin_timer_queue_summary_total;
        summary->timer_queue_summary_count =
            profile->csbwin_timer_queue_summary_count;
        memcpy(summary->timers, profile->csbwin_timers,
               sizeof(summary->timers));
        memcpy(summary->timer_queue, profile->csbwin_timer_queue,
               sizeof(summary->timer_queue));
        summary->sections_verified = CSB_V1_CSBWIN_512_SECTION_COUNT;
        return 0;
    }

    if (profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount >
            (int)CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        profile->timeline_queue.eventCount >
            (int)CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES) {
        return -1;
    }
    timer_count = (uint16_t)profile->timeline_queue.eventCount;
    summary->max_timers = timer_count;
    summary->num_timer = timer_count;
    summary->first_avail_timer = timer_count;
    summary->timer_sequence = profile->csbwin_timer_sequence;
    summary->timer_summary_total = timer_count;
    summary->timer_summary_count = timer_count;
    summary->timer_queue_summary_total = timer_count;
    summary->timer_queue_summary_count = timer_count;

    /* CSBWin SaveGame.cpp writes the TIMER array and then the timer queue
     * after GAMEBLOCK2/ITEM16/CHARDESC. For Firestaff's bounded core export,
     * write the current runtime timeline heap order as the CSBWin queue so a
     * re-import reaches the same event boundary without relying on stale
     * imported timer bytes. */
    for (i = 0u; i < timer_count; ++i) {
        int event_index = profile->timeline_queue.timeline[i];
        const struct DM1_Event_V1 *event;
        CSB_V1_CSBWin512TimerSummary *timer = &summary->timers[i];
        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
            return -1;
        }
        event = &profile->timeline_queue.events[event_index];
        timer->valid = 1;
        timer->time = event->map_time;
        timer->function = (uint8_t)event->type;
        timer->ubyte5 = event->priority;
        timer->ubyte6 = event->b_mapX;
        timer->ubyte7 = event->b_mapY;
        timer->ubyte8 = event->c_cell;
        timer->ubyte9 = event->c_effect;
        timer->sequence =
            (uint16_t)(profile->csbwin_timer_sequence + i);
        timer->level = (uint8_t)DM1_MAP_TIME_MAP(event->map_time);
        summary->timer_queue[i] = i;
    }

    summary->sections_verified = CSB_V1_CSBWIN_512_SECTION_COUNT;
    return 0;
}

static int csb_v1_runtime_locate_appended_expool_record_internal(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size)
{
    CSB_V1_CSBWin512BodyReport report;
    const uint8_t *report_bytes = NULL;
    size_t report_size = 0u;
    size_t offset;

    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!profile || !profile->csbwin_appended_tail_valid ||
        profile->csbwin_appended_tail_truncated ||
        profile->csbwin_appended_tail_size == 0u ||
        profile->csbwin_appended_tail_size !=
            profile->csbwin_appended_tail_preserved_size ||
        profile->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        /* b35d17974 receipt contract (restored after the a192cb2b0
         * worktree-merge clobber): a stale saved EXPOOL receipt must block
         * the runtime record lookup before any consumption. */
        profile->csbwin_appended_tail_fnv1a !=
            csb_v1_runtime_fnv1a32(
                profile->csbwin_appended_tail,
                profile->csbwin_appended_tail_preserved_size)) {
        return 0;
    }

    memset(&report, 0, sizeof(report));
    report.appended_size = profile->csbwin_appended_tail_size;
    report.appended_preserved_size =
        profile->csbwin_appended_tail_preserved_size;
    report.appended_fnv1a = profile->csbwin_appended_tail_fnv1a;
    report.appended_truncated = profile->csbwin_appended_tail_truncated;
    if ((report.appended_size % CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES) == 0u) {
        report.appended_expool_candidate = 1;
        report.appended_expool_block_count =
            (uint16_t)(report.appended_size /
                       CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES);
    }
    memcpy(report.appended_preserved,
           profile->csbwin_appended_tail,
           report.appended_preserved_size);

    /* CSBWin data.cpp EXPOOL::Locate returns a pointer into the DB11 block.
     * The lower-level helper works on a verifier report, so translate the
     * found offset back into the runtime profile's preserved tail storage. */
    if (!csb_v1_csbwin_512_appended_expool_locate_record(
            &report, record_id, &report_bytes, &report_size) ||
        !report_bytes ||
        report_bytes < report.appended_preserved) {
        return 0;
    }
    offset = (size_t)(report_bytes - report.appended_preserved);
    if (offset > profile->csbwin_appended_tail_preserved_size ||
        report_size >
            profile->csbwin_appended_tail_preserved_size - offset) {
        return 0;
    }
    if (out_bytes) *out_bytes = profile->csbwin_appended_tail + offset;
    if (out_size) *out_size = report_size;
    return 1;
}

static uint32_t csb_v1_runtime_read_le32(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

static void csb_v1_runtime_write_le32(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static uint32_t csb_v1_runtime_fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

/* CSBWin SaveGame.cpp publishes the level index and DSA stream together from
 * one Extended Features tail. Do not let a decoded catalog outlive the bytes
 * that authenticated it. */
static int csb_v1_runtime_has_verified_csbwin_extended_dsa_tail(
    const CSB_V1_RuntimeProfile *profile)
{
    return profile && profile->csbwin_extended_features_valid &&
           profile->csbwin_appended_tail_valid &&
           !profile->csbwin_appended_tail_truncated &&
           profile->csbwin_appended_tail_size != 0u &&
           profile->csbwin_appended_tail_size ==
               profile->csbwin_appended_tail_preserved_size &&
           profile->csbwin_appended_tail_preserved_size <=
               CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES &&
           profile->csbwin_appended_tail_fnv1a == csb_v1_runtime_fnv1a32(
               profile->csbwin_appended_tail,
               profile->csbwin_appended_tail_preserved_size);
}

/* CSBWin DSA.cpp PutState:562-571 writes LocalState=2 through DB3::ParameterB.
 * Firestaff's loaded dungeon owns only the original eight-byte DB3 record, so
 * accept the source compact form only: MakeBig's unretained word8 extension
 * must be zero and no widened 18-bit value may be invented. */
static int csb_v1_runtime_persist_csbwin_localstate2_dsa(
    CSB_V1_RuntimeProfile *candidate,
    const CSB_V1_CSBWinDSAFilterStackRunnerContext *before,
    const CSB_V1_CSBWinDSAFilterStackRunnerContext *after)
{
    CSB_V1_CSBWinDSAImportedHeader *header;
    uint8_t *record;
    uint16_t word2;
    uint16_t word4;
    uint16_t word6;
    uint32_t final_state;
    int64_t relative_state;
    int type;
    int size;

    if (!candidate || !before || !after || !candidate->dungeon_handle ||
        !before->dsa_slave_thing_valid ||
        before->dsa_id < 0 || before->dsa_id >= CSB_V1_MAX_DSA_SCRIPTS ||
        before->action_ordinal != after->action_ordinal) {
        return 0;
    }
    header = &candidate->csbwin_extended_dsa_state.imported_headers[
        before->dsa_id];
    if (!header->valid || header->local_state != 2u ||
        header->state_slot_count == 0u) {
        return 0;
    }
    if (after->transfer_execution_count != before->transfer_execution_count) {
        if (after->transfer_execution_count !=
                before->transfer_execution_count + 1 ||
            after->last_transfer.final_state < 0) {
            return 0;
        }
        final_state = (uint32_t)after->last_transfer.final_state;
    } else {
        if (after->execution_count != before->execution_count + 1) return 0;
        if (after->last_execution.forced_state >= 0) {
            final_state = (uint32_t)after->last_execution.forced_state;
        } else {
            relative_state = (int64_t)before->state_index +
                (int64_t)after->last_execution.next_state;
            if (relative_state < 0 || relative_state > UINT32_MAX) return 0;
            final_state = (uint32_t)relative_state;
        }
    }
    if (final_state >= header->state_slot_count || final_state > 0x3fffu) {
        return 0;
    }
    record = csb_v1_runtime_mutable_thing_record(candidate->dungeon_handle,
        before->dsa_slave_thing, &type, &size);
    if (!record || type != CSB_V1_THING_TYPE_ACTUATOR || size < 8) return 0;
    word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    word4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
    word6 = (uint16_t)record[6] | ((uint16_t)record[7] << 8);
    if ((word2 & 0x007fu) != CSB_V1_DSA_FILTER_ACTUATOR_TYPE ||
        (word4 & 0xc000u) != 0u || (word6 & 0xc000u) != 0u ||
        (word6 & 0x3fffu) != before->state_index) {
        return 0;
    }
    record[6] = (uint8_t)final_state;
    record[7] = (uint8_t)(final_state >> 8);
    return 1;
}

/* CSBWin DSA.cpp GetState/PutState:548-572 stores LocalState=0 in the
 * type-47 DB3 actuator's DSAstate nibble (word2 bits 12..15).  The level
 * selector and DSA stream have already been admitted by ProcessDSATimer6;
 * re-check the complete RCS/FNV tail before touching the live dungeon byte. */
static int csb_v1_runtime_persist_csbwin_localstate0_dsa(
    CSB_V1_RuntimeProfile *candidate,
    const CSB_V1_CSBWinDSAFilterStackRunnerContext *before,
    const CSB_V1_CSBWinDSAFilterStackRunnerContext *after)
{
    CSB_V1_CSBWinDSAImportedHeader *header;
    uint8_t *record;
    uint16_t word2;
    uint32_t final_state;
    int64_t relative_state;
    int type;
    int size;

    if (!candidate || !before || !after || !candidate->dungeon_handle ||
        !before->dsa_slave_thing_valid ||
        !csb_v1_runtime_has_verified_csbwin_extended_dsa_tail(candidate) ||
        before->dsa_id < 0 || before->dsa_id >= CSB_V1_MAX_DSA_SCRIPTS ||
        before->action_ordinal != after->action_ordinal) {
        return 0;
    }
    header = &candidate->csbwin_extended_dsa_state.imported_headers[
        before->dsa_id];
    if (!header->valid || header->local_state != 0u ||
        header->state_slot_count == 0u) {
        return 0;
    }
    if (after->transfer_execution_count != before->transfer_execution_count) {
        if (after->transfer_execution_count !=
                before->transfer_execution_count + 1 ||
            after->last_transfer.final_state < 0) {
            return 0;
        }
        final_state = (uint32_t)after->last_transfer.final_state;
    } else {
        if (after->execution_count != before->execution_count + 1) return 0;
        if (after->last_execution.forced_state >= 0) {
            final_state = (uint32_t)after->last_execution.forced_state;
        } else {
            relative_state = (int64_t)before->state_index +
                (int64_t)after->last_execution.next_state;
            if (relative_state < 0 || relative_state > UINT32_MAX) return 0;
            final_state = (uint32_t)relative_state;
        }
    }
    if (final_state >= header->state_slot_count || final_state > 0x0fu) {
        return 0;
    }
    record = csb_v1_runtime_mutable_thing_record(candidate->dungeon_handle,
        before->dsa_slave_thing, &type, &size);
    if (!record || type != CSB_V1_THING_TYPE_ACTUATOR || size < 4) return 0;
    word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    if ((word2 & 0x007fu) != CSB_V1_DSA_FILTER_ACTUATOR_TYPE ||
        ((word2 >> 7) & 0x1fu) >= 32u ||
        ((word2 >> 12) & 0x0fu) != before->state_index) {
        return 0;
    }
    word2 = (uint16_t)((word2 & 0x0fffu) | (final_state << 12));
    record[2] = (uint8_t)word2;
    record[3] = (uint8_t)(word2 >> 8);
    return 1;
}

/* Persist the one CSBWin DSA state store that has a complete source-owned
 * representation here: DSA::m_state (LocalState 1).  CSBWin DSA.cpp
 * ProcessDSATimer6 (lines 5315-5465) obtains a state through GetState(),
 * executes the exact selected action, then commits the final/forced state
 * through PutState().  SaveGame.cpp ReadDSAs/WriteDSAs (211-241, 775-790)
 * wraps that serialized DSA stream in data.cpp's RCS checksum.  Re-check the
 * whole stream before changing its two-byte m_state field so a stale, partial,
 * or caller-built receipt cannot become a saved runtime transition. */
static int csb_v1_runtime_persist_csbwin_localstate1_dsa(
    CSB_V1_RuntimeProfile *candidate,
    const CSB_V1_CSBWinDSAFilterStackRunnerContext *before,
    const CSB_V1_CSBWinDSAFilterStackRunnerContext *after)
{
    CSB_V1_CSBWinExtendedDSAReport report;
    CSB_V1_CSBWinExtendedFeaturesReport features;
    CSB_V1_CSBWinDSAImportedHeader *header;
    size_t offset;
    uint32_t final_state;
    uint32_t dsa_ordinal;
    int64_t relative_state;

    if (!candidate || !before || !after ||
        !candidate->csbwin_appended_tail_valid ||
        candidate->csbwin_appended_tail_truncated ||
        candidate->csbwin_appended_tail_size == 0u ||
        candidate->csbwin_appended_tail_size !=
            candidate->csbwin_appended_tail_preserved_size ||
        candidate->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        candidate->csbwin_appended_tail_fnv1a != csb_v1_runtime_fnv1a32(
            candidate->csbwin_appended_tail,
            candidate->csbwin_appended_tail_preserved_size) ||
        before->dsa_id < 0 || before->dsa_id >= CSB_V1_MAX_DSA_SCRIPTS ||
        before->action_ordinal != after->action_ordinal) {
        return 0;
    }
    header = &candidate->csbwin_extended_dsa_state.imported_headers[
        before->dsa_id];
    if (!header->valid || header->local_state != 1u ||
        header->persistent_state != before->state_index ||
        header->state_slot_count == 0u) {
        return 0;
    }

    if (after->transfer_execution_count != before->transfer_execution_count) {
        if (after->transfer_execution_count !=
                before->transfer_execution_count + 1 ||
            after->last_transfer.final_state < 0) {
            return 0;
        }
        final_state = (uint32_t)after->last_transfer.final_state;
    } else {
        if (after->execution_count != before->execution_count + 1) return 0;
        if (after->last_execution.forced_state >= 0) {
            final_state = (uint32_t)after->last_execution.forced_state;
        } else {
            relative_state = (int64_t)before->state_index +
                (int64_t)after->last_execution.next_state;
            if (relative_state < 0 || relative_state > UINT32_MAX) return 0;
            final_state = (uint32_t)relative_state;
        }
    }
    if (final_state >= header->state_slot_count || final_state > 0xffffu) {
        return 0;
    }

    memset(&report, 0, sizeof(report));
    memset(&features, 0, sizeof(features));
    if (csb_v1_csbwin_512_inspect_extended_dsa_section(
            candidate->csbwin_appended_tail,
            candidate->csbwin_appended_tail_preserved_size, &report,
            &features) != CSB_V1_CSBWIN_EXTENDED_OK || !report.valid ||
        report.dsa_count == 0u ||
        report.next_payload_offset >
            candidate->csbwin_appended_tail_preserved_size) {
        return 0;
    }

    offset = features.extension_payload_offset;
    for (dsa_ordinal = 0u; dsa_ordinal < features.dsa_count; ++dsa_ordinal) {
        uint32_t dsa_id;
        uint32_t state_slots;
        uint32_t non_empty_states;
        uint32_t state_ordinal;

        if (offset > report.dsa_payload_offset + report.dsa_payload_size ||
            report.dsa_payload_offset + report.dsa_payload_size - offset <
                100u) {
            return 0;
        }
        dsa_id = csb_v1_runtime_read_le32(
            candidate->csbwin_appended_tail + offset);
        state_slots = csb_v1_runtime_read_le32(
            candidate->csbwin_appended_tail + offset + 96u);
        non_empty_states = csb_v1_runtime_read_le32(
            candidate->csbwin_appended_tail + offset + 104u);
        if (dsa_id == (uint32_t)before->dsa_id) {
            if (csb_v1_runtime_read_le32(
                    candidate->csbwin_appended_tail + offset + 84u) !=
                    header->persistent_state ||
                csb_v1_runtime_read_le32(
                    candidate->csbwin_appended_tail + offset + 88u) != 1u ||
                csb_v1_runtime_read_le32(
                    candidate->csbwin_appended_tail + offset + 92u) !=
                    header->group_id ||
                state_slots != header->state_slot_count) {
                return 0;
            }
            csb_v1_runtime_write_le32(
                candidate->csbwin_appended_tail + offset + 84u,
                final_state);
            /* CSBWin data.cpp RCS(ui8 *, i32):1818-1827. */
            {
                uint32_t checksum = 0xffffu;
                size_t i;
                for (i = 0u; i < report.dsa_payload_size; ++i) {
                    checksum = checksum * 0xbb40e62du + 11u +
                        candidate->csbwin_appended_tail[
                            report.dsa_payload_offset + i];
                }
                csb_v1_runtime_write_le32(
                    candidate->csbwin_appended_tail +
                        report.dsa_payload_offset + report.dsa_payload_size,
                    checksum);
            }
            candidate->csbwin_appended_tail_fnv1a = csb_v1_runtime_fnv1a32(
                candidate->csbwin_appended_tail,
                candidate->csbwin_appended_tail_preserved_size);
            header->persistent_state = final_state;
            return 1;
        }

        offset += 108u;
        for (state_ordinal = 0u; state_ordinal < non_empty_states;
             ++state_ordinal) {
            uint32_t action_count;
            uint32_t action_ordinal;

            if (offset > report.dsa_payload_offset + report.dsa_payload_size ||
                report.dsa_payload_offset + report.dsa_payload_size - offset <
                    8u) {
                return 0;
            }
            action_count = csb_v1_runtime_read_le32(
                candidate->csbwin_appended_tail + offset + 4u);
            offset += 8u;
            for (action_ordinal = 0u; action_ordinal < action_count;
                 ++action_ordinal) {
                uint32_t words;
                size_t byte_count;

                if (offset > report.dsa_payload_offset + report.dsa_payload_size ||
                    report.dsa_payload_offset + report.dsa_payload_size -
                        offset < 8u) {
                    return 0;
                }
                words = csb_v1_runtime_read_le32(
                    candidate->csbwin_appended_tail + offset + 4u);
                if ((size_t)words > (SIZE_MAX - 8u) / 2u) return 0;
                byte_count = (size_t)words * 2u;
                if (byte_count > report.dsa_payload_offset +
                        report.dsa_payload_size - offset - 8u) {
                    return 0;
                }
                offset += 8u + byte_count;
            }
        }
    }
    return 0;
}

/* CSBWin data.cpp EXPOOL::enlarge() lays a DB11 block out as equal-sized
 * nodes beginning at word 1.  The source assumes a trusted save buffer; the
 * runtime must prove that a saved free-list pointer still denotes one of
 * those nodes before using it.  In particular, a pointer into the DB11
 * header must never become an EXPOOL record or free-list link. */
static int csb_v1_runtime_expool_node_is_valid(const uint8_t *bytes,
                                               uint32_t total_words,
                                               uint32_t node,
                                               uint32_t size_words)
{
    uint32_t block_base;
    uint32_t node_offset;
    uint32_t stored_size;

    if (!bytes || size_words < 2u || size_words > 31u ||
        node >= total_words) {
        return 0;
    }
    block_base = node & 0xffffffc0u;
    node_offset = node & 0x3fu;
    if (block_base >= total_words || node_offset == 0u ||
        node_offset + size_words > 64u ||
        node + size_words > total_words) {
        return 0;
    }
    stored_size = (uint32_t)bytes[(size_t)block_base * 4u + 2u] |
        ((uint32_t)bytes[(size_t)block_base * 4u + 3u] << 8);
    if (stored_size != size_words) {
        return 0;
    }
    return ((node_offset - 1u) % size_words) == 0u;
}

/* CSBWin data.cpp EXPOOL::Read/Write, limited to an already preserved DB11
 * tail. This is intentionally not an allocator: EXPOOL::enlarge would create
 * a new save block and has no authenticated source-tail receipt here. The
 * source Read first unlinks the old node into its exact-size free list; Write
 * then consumes an exact free node for the replacement, or rejects. */
static int csb_v1_runtime_replace_appended_expool_record_internal(
    CSB_V1_RuntimeProfile *candidate,
    uint32_t record_id,
    const uint8_t *payload,
    size_t payload_size)
{
    uint8_t *bytes;
    uint32_t total_words;
    uint32_t hash;
    uint32_t hashi;
    uint32_t bucket;
    uint32_t prior = 0u;
    uint32_t old_node = 0u;
    uint32_t old_size_words = 0u;
    uint32_t write_words;
    int guard;

    if (!candidate || !candidate->csbwin_appended_tail_valid ||
        candidate->csbwin_appended_tail_truncated ||
        candidate->csbwin_appended_tail_size == 0u ||
        candidate->csbwin_appended_tail_size !=
            candidate->csbwin_appended_tail_preserved_size ||
        (candidate->csbwin_appended_tail_preserved_size & 3u) != 0u ||
        candidate->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        candidate->csbwin_appended_tail_fnv1a != csb_v1_runtime_fnv1a32(
            candidate->csbwin_appended_tail,
            candidate->csbwin_appended_tail_preserved_size) ||
        (payload_size != 0u && (!payload || (payload_size & 3u) != 0u))) {
        return 0;
    }

    bytes = candidate->csbwin_appended_tail;
    total_words = (uint32_t)(candidate->csbwin_appended_tail_preserved_size / 4u);
    if (total_words < 64u) return 0;
    hash = record_id * 0xbb40e62du;
    hashi = 32u + (hash >> 27);
    if (hashi >= total_words) return 0;
    bucket = csb_v1_runtime_read_le32(bytes + (size_t)hashi * 4u);
    if ((bucket & 0x80000000u) != 0u) {
        hashi = (bucket & 0x7fffffffu) + ((hash >> 21) & 0x3fu);
        if (hashi >= total_words) return 0;
        bucket = csb_v1_runtime_read_le32(bytes + (size_t)hashi * 4u);
    }

    for (guard = 0; guard < (int)total_words && bucket != 0u; ++guard) {
        uint32_t block_base;

        if (bucket + 2u > total_words) return 0;
        block_base = bucket & 0xffffffc0u;
        if (block_base >= total_words) return 0;
        old_size_words = (uint32_t)bytes[(size_t)block_base * 4u + 2u] |
            ((uint32_t)bytes[(size_t)block_base * 4u + 3u] << 8);
        if (!csb_v1_runtime_expool_node_is_valid(
                bytes, total_words, bucket, old_size_words)) {
            return 0;
        }
        if (csb_v1_runtime_read_le32(bytes + (size_t)(bucket + 1u) * 4u) ==
            record_id) {
            old_node = bucket;
            break;
        }
        prior = bucket;
        bucket = csb_v1_runtime_read_le32(bytes + (size_t)bucket * 4u);
    }
    if (guard == (int)total_words) return 0;

    /* EXPOOL::Read: unlink the old record and return its exact DB11 node to
     * the size-specific free list. A missing record is valid for Write. */
    if (old_node != 0u) {
        uint32_t next = csb_v1_runtime_read_le32(
            bytes + (size_t)old_node * 4u);
        if (old_size_words + 2u >= total_words) return 0;
        if (prior == 0u) {
            csb_v1_runtime_write_le32(bytes + (size_t)hashi * 4u, next);
        } else {
            csb_v1_runtime_write_le32(bytes + (size_t)prior * 4u, next);
        }
        csb_v1_runtime_write_le32(bytes + (size_t)old_node * 4u,
            csb_v1_runtime_read_le32(bytes + (size_t)old_size_words * 4u));
        csb_v1_runtime_write_le32(bytes + (size_t)old_size_words * 4u,
            old_node);
    }

    /* SetSkin returns after Read when the trimmed column is empty. */
    if (payload_size == 0u) return old_node != 0u;
    write_words = (uint32_t)(payload_size / 4u) + 2u;
    if (write_words < 2u || write_words > 31u ||
        write_words + 2u >= total_words) return 0;
    bucket = csb_v1_runtime_read_le32(bytes + (size_t)write_words * 4u);
    if (bucket == 0u || !csb_v1_runtime_expool_node_is_valid(
            bytes, total_words, bucket, write_words)) {
        return 0;
    }
    csb_v1_runtime_write_le32(bytes + (size_t)write_words * 4u,
        csb_v1_runtime_read_le32(bytes + (size_t)bucket * 4u));
    csb_v1_runtime_write_le32(bytes + (size_t)bucket * 4u,
        csb_v1_runtime_read_le32(bytes + (size_t)hashi * 4u));
    csb_v1_runtime_write_le32(bytes + (size_t)hashi * 4u, bucket);
    csb_v1_runtime_write_le32(bytes + (size_t)(bucket + 1u) * 4u, record_id);
    memcpy(bytes + (size_t)(bucket + 2u) * 4u, payload, payload_size);
    return 1;
}

static int csb_v1_runtime_stage_csbwin_global_variables(
    CSB_V1_RuntimeProfile *candidate)
{
    const uint32_t record_base = (5u << 24) | (4u << 16);
    const uint32_t words_per_record = 16u;
    const uint32_t record_count =
        CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY / words_per_record;
    uint32_t staged[CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY] = { 0u };
    uint32_t count = 0u;
    uint32_t record_index;

    if (!candidate) return -1;

    /* CSBWin SaveGame.cpp ReadSavegame() reads record i as
     * (EDT_Database << 24) | (EDBT_GlobalVariables << 16) | i, appending
     * exactly sixteen ui32 values until EXPOOL::Locate first fails, then
     * invokes DSAINDEX::ReadTracing. Keep the bounded Firestaff DSA bank
     * source-sized and stage every word before publishing it. */
    if (candidate->csbwin_appended_tail_valid &&
        candidate->csbwin_appended_tail_size != 0u &&
        (candidate->csbwin_appended_tail_truncated ||
         candidate->csbwin_appended_tail_size !=
             candidate->csbwin_appended_tail_preserved_size ||
         candidate->csbwin_appended_tail_preserved_size >
             CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
         candidate->csbwin_appended_tail_fnv1a !=
             csb_v1_runtime_fnv1a32(
                 candidate->csbwin_appended_tail,
                 candidate->csbwin_appended_tail_preserved_size))) {
        return -1;
    }

    for (record_index = 0u; record_index < record_count; ++record_index) {
        const uint8_t *payload = NULL;
        size_t payload_size = 0u;
        uint32_t word;

        if (!csb_v1_runtime_locate_appended_expool_record_internal(
                candidate, record_base | record_index,
                &payload, &payload_size)) {
            break;
        }
        if (!payload || payload_size < words_per_record * sizeof(uint32_t)) {
            return -1;
        }
        for (word = 0u; word < words_per_record; ++word) {
            staged[count + word] = csb_v1_runtime_read_le32(
                payload + word * sizeof(uint32_t));
        }
        count += words_per_record;
    }
    /* The runner's source-sized bounded bank can hold six complete records
     * (96 values). Never silently drop a seventh source record merely because
     * the 100-cell stack bridge has no partial-record representation. */
    if (record_index == record_count &&
        csb_v1_runtime_locate_appended_expool_record_internal(
            candidate, record_base | record_index, NULL, NULL)) {
        return -1;
    }

    memset(candidate->csbwin_global_variables, 0,
           sizeof(candidate->csbwin_global_variables));
    memcpy(candidate->csbwin_global_variables, staged,
           count * sizeof(staged[0]));
    candidate->csbwin_global_variable_count = (uint16_t)count;
    candidate->csbwin_global_variables_valid = 1;
    return 0;
}

static int csb_v1_runtime_stage_csbwin_overlay_palette(
    CSB_V1_RuntimeProfile *candidate)
{
    enum {
        CSBWIN_EDT_PALETTE = 7u,
        CSBWIN_PALETTE_RECORD_COUNT = 24u,
        CSBWIN_PALETTE_RECORD_BYTES = 64u
    };
    uint8_t staged[CSB_V1_CSBWIN_OVERLAY_PALETTE_BYTES];
    uint32_t record_index;

    if (!candidate) return -1;

    /* CSBWin SaveGame.cpp:1948-1970 calls EXPOOL::Locate for every record
     * before publishing overlayPaletteRed/Green/Blue. Firestaff is stricter
     * at the host boundary: an incomplete or stale save-tail receipt has no
     * palette surface, rather than reusing pixels from a prior save. */
    if (!candidate->csbwin_appended_tail_valid ||
        candidate->csbwin_appended_tail_size == 0u) {
        candidate->csbwin_overlay_palette_valid = 0;
        candidate->csbwin_overlay_palette_tail_fnv1a = 0u;
        memset(candidate->csbwin_overlay_palette, 0,
               sizeof(candidate->csbwin_overlay_palette));
        return 0;
    }
    if (candidate->csbwin_appended_tail_truncated ||
        candidate->csbwin_appended_tail_size !=
            candidate->csbwin_appended_tail_preserved_size ||
        candidate->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        candidate->csbwin_appended_tail_fnv1a !=
            csb_v1_runtime_fnv1a32(
                candidate->csbwin_appended_tail,
                candidate->csbwin_appended_tail_preserved_size)) {
        return -1;
    }

    for (record_index = 0u;
         record_index < CSBWIN_PALETTE_RECORD_COUNT;
         ++record_index) {
        const uint8_t *payload = NULL;
        size_t payload_size = 0u;

        if (!csb_v1_runtime_locate_appended_expool_record_internal(
                candidate, (CSBWIN_EDT_PALETTE << 24) | record_index,
                &payload, &payload_size) ||
            !payload || payload_size < CSBWIN_PALETTE_RECORD_BYTES) {
            /* SaveGame.cpp simply retains its current overlay when a save
             * does not carry the entire 24-record bundle. Firestaff has no
             * hidden fallback surface here: invalidate this profile's receipt
             * and let the caller continue restoring other authentic records. */
            candidate->csbwin_overlay_palette_valid = 0;
            candidate->csbwin_overlay_palette_tail_fnv1a = 0u;
            memset(candidate->csbwin_overlay_palette, 0,
                   sizeof(candidate->csbwin_overlay_palette));
            return 0;
        }
        memcpy(staged + (size_t)record_index * CSBWIN_PALETTE_RECORD_BYTES,
               payload, CSBWIN_PALETTE_RECORD_BYTES);
    }

    memcpy(candidate->csbwin_overlay_palette, staged, sizeof(staged));
    candidate->csbwin_overlay_palette_valid = 1;
    candidate->csbwin_overlay_palette_tail_fnv1a =
        candidate->csbwin_appended_tail_fnv1a;
    return 0;
}

static int csb_v1_runtime_write_csbwin_global_variables(
    CSB_V1_RuntimeProfile *candidate)
{
    const uint32_t record_base = (5u << 24) | (4u << 16);
    const uint32_t words_per_record = 16u;
    uint32_t record_count;
    uint32_t record_index;

    if (!candidate || !candidate->csbwin_global_variables_valid) return -1;
    if (candidate->csbwin_global_variable_count == 0u) return 0;
    if ((candidate->csbwin_global_variable_count % words_per_record) != 0u ||
        candidate->csbwin_global_variable_count >
            CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY ||
        !candidate->csbwin_appended_tail_valid ||
        candidate->csbwin_appended_tail_truncated ||
        candidate->csbwin_appended_tail_size == 0u ||
        candidate->csbwin_appended_tail_size !=
            candidate->csbwin_appended_tail_preserved_size ||
        candidate->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES) {
        return -1;
    }

    record_count = candidate->csbwin_global_variable_count / words_per_record;
    for (record_index = 0u; record_index < record_count; ++record_index) {
        const uint8_t *payload = NULL;
        size_t payload_size = 0u;
        size_t payload_offset;
        uint32_t word;

        if (!csb_v1_runtime_locate_appended_expool_record_internal(
                candidate, record_base | record_index,
                &payload, &payload_size) ||
            !payload || payload_size < words_per_record * sizeof(uint32_t) ||
            payload < candidate->csbwin_appended_tail) {
            return -1;
        }
        payload_offset = (size_t)(payload - candidate->csbwin_appended_tail);
        if (payload_offset > candidate->csbwin_appended_tail_preserved_size ||
            payload_size > candidate->csbwin_appended_tail_preserved_size -
                payload_offset) {
            return -1;
        }
        for (word = 0u; word < words_per_record; ++word) {
            csb_v1_runtime_write_le32(
                candidate->csbwin_appended_tail + payload_offset +
                    word * sizeof(uint32_t),
                candidate->csbwin_global_variables[
                    record_index * words_per_record + word]);
        }
    }
    candidate->csbwin_appended_tail_fnv1a = csb_v1_runtime_fnv1a32(
        candidate->csbwin_appended_tail,
        candidate->csbwin_appended_tail_preserved_size);
    return 0;
}

static int csb_v1_runtime_write_csbwin_overlay_palette(
    CSB_V1_RuntimeProfile *candidate)
{
    enum {
        CSBWIN_EDT_PALETTE = 7u,
        CSBWIN_PALETTE_RECORD_COUNT = 24u,
        CSBWIN_PALETTE_RECORD_BYTES = 64u
    };
    size_t payload_offsets[CSBWIN_PALETTE_RECORD_COUNT];
    uint32_t record_index;

    if (!candidate || !candidate->csbwin_overlay_palette_valid ||
        !candidate->csbwin_appended_tail_valid ||
        candidate->csbwin_appended_tail_truncated ||
        candidate->csbwin_appended_tail_size == 0u ||
        candidate->csbwin_appended_tail_size !=
            candidate->csbwin_appended_tail_preserved_size ||
        candidate->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        candidate->csbwin_appended_tail_fnv1a !=
            csb_v1_runtime_fnv1a32(candidate->csbwin_appended_tail,
                                    candidate->csbwin_appended_tail_preserved_size)) {
        return -1;
    }

    /* CSBWin SaveGame.cpp:1213-1224 discards each old EDT_Palette record and
     * writes exactly sixteen words. Do not emulate EXPOOL::Write expansion:
     * every target must already be a complete source-owned record. */
    for (record_index = 0u;
         record_index < CSBWIN_PALETTE_RECORD_COUNT;
         ++record_index) {
        const uint8_t *payload = NULL;
        size_t payload_size = 0u;
        size_t payload_offset;

        if (!csb_v1_runtime_locate_appended_expool_record_internal(
                candidate, (CSBWIN_EDT_PALETTE << 24) | record_index,
                &payload, &payload_size) ||
            !payload || payload_size < CSBWIN_PALETTE_RECORD_BYTES ||
            payload < candidate->csbwin_appended_tail) {
            return -1;
        }
        payload_offset = (size_t)(payload - candidate->csbwin_appended_tail);
        if (payload_offset > candidate->csbwin_appended_tail_preserved_size ||
            CSBWIN_PALETTE_RECORD_BYTES >
                candidate->csbwin_appended_tail_preserved_size - payload_offset) {
            return -1;
        }
        payload_offsets[record_index] = payload_offset;
    }
    /* Every lookup verifies the original tail receipt. Validate every target
     * before changing the first byte, then commit the fixed source bundle. */
    for (record_index = 0u;
         record_index < CSBWIN_PALETTE_RECORD_COUNT;
         ++record_index) {
        memcpy(candidate->csbwin_appended_tail + payload_offsets[record_index],
               candidate->csbwin_overlay_palette +
                   (size_t)record_index * CSBWIN_PALETTE_RECORD_BYTES,
               CSBWIN_PALETTE_RECORD_BYTES);
    }
    candidate->csbwin_appended_tail_fnv1a = csb_v1_runtime_fnv1a32(
        candidate->csbwin_appended_tail,
        candidate->csbwin_appended_tail_preserved_size);
    candidate->csbwin_overlay_palette_tail_fnv1a =
        candidate->csbwin_appended_tail_fnv1a;
    return 0;
}

static int csb_v1_runtime_stage_csbwin_save_policy(
    CSB_V1_RuntimeProfile *candidate)
{
    const uint32_t database_base = 5u << 24;
    const uint32_t disable_saves_record = database_base | (5u << 16);
    const uint32_t delete_duplicate_timers_record = database_base | (1u << 16);
    const uint32_t runtime_signatures_record = database_base | (2u << 16);
    const uint32_t debugging_record = database_base | (3u << 16);
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;

    if (!candidate) return -1;
    candidate->csbwin_saves_disabled = 0;
    candidate->csbwin_delete_duplicate_timers = 1u;
    candidate->csbwin_debugging_data = 0u;
    candidate->csbwin_csbgraphics_signature_data = 0u;
    candidate->csbwin_graphics_signature_data = 0u;
    candidate->csbwin_version_data = 0u;

    /* CSBWin SaveGame.cpp:1972-2034 restores DisableSaves,
     * DeleteDuplicateTimers, Debuging, and RuntimeFileSignatures after its
     * palette records. These are exact DB11 records, not Firestaff settings;
     * an absent record retains the source default while a short present record
     * rejects the whole candidate before runtime state is published. */
    if (!candidate->csbwin_appended_tail_valid ||
        candidate->csbwin_appended_tail_size == 0u) {
        return 0;
    }
    if (candidate->csbwin_appended_tail_truncated ||
        candidate->csbwin_appended_tail_size !=
            candidate->csbwin_appended_tail_preserved_size ||
        candidate->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        candidate->csbwin_appended_tail_fnv1a !=
            csb_v1_runtime_fnv1a32(
                candidate->csbwin_appended_tail,
                candidate->csbwin_appended_tail_preserved_size)) {
        return -1;
    }
    if (csb_v1_runtime_locate_appended_expool_record_internal(
            candidate, disable_saves_record, &payload, &payload_size)) {
        if (!payload || payload_size == 0u) return -1;
        candidate->csbwin_saves_disabled = 1;
    }
    if (csb_v1_runtime_locate_appended_expool_record_internal(
            candidate, delete_duplicate_timers_record, &payload,
            &payload_size)) {
        if (!payload || payload_size < sizeof(uint32_t)) return -1;
        candidate->csbwin_delete_duplicate_timers =
            csb_v1_runtime_read_le32(payload);
    }
    if (csb_v1_runtime_locate_appended_expool_record_internal(
            candidate, debugging_record, &payload, &payload_size)) {
        if (!payload || payload_size < sizeof(uint32_t)) return -1;
        candidate->csbwin_debugging_data = csb_v1_runtime_read_le32(payload);
    }
    if (candidate->csbwin_debugging_data == 0u) {
        if (csb_v1_runtime_locate_appended_expool_record_internal(
                candidate, runtime_signatures_record, &payload,
                &payload_size)) {
            if (!payload || payload_size < sizeof(uint32_t)) return -1;
            candidate->csbwin_csbgraphics_signature_data =
                csb_v1_runtime_read_le32(payload);
        }
        if (csb_v1_runtime_locate_appended_expool_record_internal(
                candidate, runtime_signatures_record | 1u, &payload,
                &payload_size)) {
            if (!payload || payload_size < sizeof(uint32_t)) return -1;
            candidate->csbwin_graphics_signature_data =
                csb_v1_runtime_read_le32(payload);
        }
        if (csb_v1_runtime_locate_appended_expool_record_internal(
                candidate, runtime_signatures_record | 2u, &payload,
                &payload_size)) {
            if (!payload || payload_size < sizeof(uint32_t)) return -1;
            candidate->csbwin_version_data = csb_v1_runtime_read_le32(payload);
        }
    }
    return 0;
}

static int csb_v1_runtime_stage_csbwin_dsa_tracing(
    CSB_V1_RuntimeProfile *candidate)
{
    CSB_V1_CSBWin512BodyReport report;
    int rc;

    if (!candidate) return -1;
    memset(&candidate->csbwin_dsa_tracing, 0,
           sizeof(candidate->csbwin_dsa_tracing));
    candidate->csbwin_dsa_tracing.valid = 1;
    candidate->csbwin_dsa_tracing.record_id =
        CSB_V1_CSBWIN_DSA_TRACING_RECORD_ID;

    /* CSBWin DSA.cpp DSAINDEX::ReadTracing lines 5553-5583 asks EXPOOL for
     * this optional record only after its save body is available. A save
     * without an appended database therefore has a valid absent bitmap. */
    if (!candidate->csbwin_appended_tail_valid ||
        candidate->csbwin_appended_tail_size == 0u) {
        return 0;
    }
    if (candidate->csbwin_appended_tail_truncated ||
        candidate->csbwin_appended_tail_size !=
            candidate->csbwin_appended_tail_preserved_size ||
        candidate->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES) {
        return -1;
    }

    memset(&report, 0, sizeof(report));
    report.appended_size = candidate->csbwin_appended_tail_size;
    report.appended_preserved_size =
        candidate->csbwin_appended_tail_preserved_size;
    report.appended_fnv1a = candidate->csbwin_appended_tail_fnv1a;
    report.appended_truncated = candidate->csbwin_appended_tail_truncated;
    if ((report.appended_size % CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES) == 0u) {
        report.appended_expool_candidate = 1;
        report.appended_expool_block_count = (uint16_t)(
            report.appended_size / CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES);
    }
    memcpy(report.appended_preserved, candidate->csbwin_appended_tail,
           report.appended_preserved_size);
    rc = csb_v1_csbwin_512_inspect_appended_dsa_tracing(
        &report, &candidate->csbwin_dsa_tracing);
    return rc == CSB_V1_CSBWIN_512_OK ? 0 : -1;
}

int csb_v1_runtime_locate_csbwin_appended_expool_record(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size)
{
    return csb_v1_runtime_locate_appended_expool_record_internal(
        profile,
        record_id,
        out_bytes,
        out_size);
}

int csb_v1_runtime_read_csbwin_wing_talents(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint,
    uint32_t *out_talents)
{
    enum {
        csbwin_edt_character = 8,
        csbwin_character_record_count = 8,
        csbwin_character_record_bytes = 100,
        csbwin_character_talents_offset = 276,
        csbwin_character_fingerprint_offset = 280
    };
    uint8_t character[csbwin_character_record_count *
                      csbwin_character_record_bytes];
    int record_index;

    if (out_talents) *out_talents = 0u;
    if (!profile || !out_talents || !profile->csbwin_appended_tail_valid ||
        profile->csbwin_appended_tail_truncated ||
        profile->csbwin_appended_tail_size == 0u ||
        profile->csbwin_appended_tail_size !=
            profile->csbwin_appended_tail_preserved_size ||
        profile->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        profile->csbwin_appended_tail_fnv1a != csb_v1_runtime_fnv1a32(
            profile->csbwin_appended_tail,
            profile->csbwin_appended_tail_preserved_size)) {
        return -1;
    }
    for (record_index = 0; record_index < csbwin_character_record_count;
         ++record_index) {
        const uint8_t *payload = NULL;
        size_t payload_size = 0u;
        uint32_t record_id = ((uint32_t)csbwin_edt_character << 24) |
            ((uint32_t)record_index << 16) | fingerprint;

        if (!csb_v1_runtime_locate_appended_expool_record_internal(
                profile, record_id, &payload, &payload_size)) {
            return record_index == 0 ? 0 : -1;
        }
        if (!payload || payload_size != csbwin_character_record_bytes) {
            return -1;
        }
        memcpy(character + (size_t)record_index *
                   csbwin_character_record_bytes,
               payload, csbwin_character_record_bytes);
    }
    if (((uint16_t)character[csbwin_character_fingerprint_offset] |
         ((uint16_t)character[csbwin_character_fingerprint_offset + 1u] << 8)) !=
        fingerprint) {
        return -1;
    }
    *out_talents = csb_v1_runtime_read_le32(
        character + csbwin_character_talents_offset);
    return 1;
}

int csb_v1_runtime_has_csbwin_wing_character(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint)
{
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;
    const uint32_t record_id = (8u << 24) | fingerprint;

    if (!profile || !profile->csbwin_appended_tail_valid ||
        profile->csbwin_appended_tail_truncated ||
        profile->csbwin_appended_tail_size == 0u ||
        profile->csbwin_appended_tail_size !=
            profile->csbwin_appended_tail_preserved_size ||
        profile->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        profile->csbwin_appended_tail_fnv1a != csb_v1_runtime_fnv1a32(
            profile->csbwin_appended_tail,
            profile->csbwin_appended_tail_preserved_size)) {
        return -1;
    }
    if (!csb_v1_runtime_locate_appended_expool_record_internal(
            profile, record_id, &payload, &payload_size)) {
        return 0;
    }
    return payload && payload_size == 25u * sizeof(uint32_t) ? 1 : -1;
}

int csb_v1_runtime_set_csbwin_wing_talents(
    CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint,
    uint32_t talents)
{
    enum { character_records = 8, character_record_bytes = 100 };
    CSB_V1_RuntimeProfile candidate;
    uint32_t current_talents = 0u;
    int record_index;

    if (!profile || csb_v1_runtime_read_csbwin_wing_talents(
                        profile, fingerprint, &current_talents) != 1) {
        return -1;
    }
    if (current_talents == talents) return 0;
    candidate = *profile;
    for (record_index = 0; record_index < character_records; ++record_index) {
        const uint8_t *source = NULL;
        size_t source_size = 0u;
        uint8_t payload[character_record_bytes];
        uint32_t record_id = (8u << 24) |
            ((uint32_t)record_index << 16) | fingerprint;

        if (!csb_v1_runtime_locate_appended_expool_record_internal(
                &candidate, record_id, &source, &source_size) ||
            !source || source_size != sizeof(payload)) {
            return -1;
        }
        memcpy(payload, source, sizeof(payload));
        if (record_index == 2) {
            csb_v1_runtime_write_le32(payload + 76u, talents);
        }
        if (!csb_v1_runtime_replace_appended_expool_record_internal(
                &candidate, record_id, payload, sizeof(payload))) {
            return -1;
        }
        candidate.csbwin_appended_tail_fnv1a = csb_v1_runtime_fnv1a32(
            candidate.csbwin_appended_tail,
            candidate.csbwin_appended_tail_preserved_size);
    }
    *profile = candidate;
    return 1;
}

static int csb_v1_runtime_dsa_get_wing_talents(void *user,
                                               uint16_t fingerprint,
                                               uint32_t *out_talents)
{
    return csb_v1_runtime_read_csbwin_wing_talents(
        (const CSB_V1_RuntimeProfile *)user, fingerprint, out_talents);
}

static int csb_v1_runtime_dsa_has_wing_character(void *user,
                                                  uint16_t fingerprint)
{
    return csb_v1_runtime_has_csbwin_wing_character(
        (const CSB_V1_RuntimeProfile *)user, fingerprint);
}

static int csb_v1_runtime_dsa_set_wing_talents(void *user,
                                               uint16_t fingerprint,
                                               uint32_t talents)
{
    return csb_v1_runtime_set_csbwin_wing_talents(
        (CSB_V1_RuntimeProfile *)user, fingerprint, talents);
}

static int csb_v1_runtime_dsa_get_info(void *user, uint16_t thing,
                                        int *out_selector, int *out_state,
                                        int *out_parameter_a,
                                        int *out_parameter_b)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const uint8_t *record;
    uint16_t word2;
    int type;
    int index;
    int size;

    if (!out_selector || !out_state || !out_parameter_a || !out_parameter_b ||
        !profile || !profile->dungeon_handle) return 0;
    record = csb_v1_dungeon_get_thing_record(profile->dungeon_handle, thing,
                                              &type, &index, &size);
    (void)index;
    if (!record || type != CSB_V1_THING_TYPE_ACTUATOR || size < 8) return 0;
    word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    if ((word2 & 0x007fu) != CSB_V1_DSA_FILTER_ACTUATOR_TYPE) return 0;
    *out_selector = (int)((word2 >> 7) & 0x1fu);
    *out_state = (int)((word2 >> 12) & 0x0fu);
    *out_parameter_a = (int)((uint16_t)record[4] |
                             ((uint16_t)record[5] << 8));
    *out_parameter_b = (int)((uint16_t)record[6] |
                             ((uint16_t)record[7] << 8));
    return 1;
}

static int csb_v1_runtime_dsa_get_skin(void *user,
                                       uint32_t location,
                                       uint8_t *out_skin)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_RuntimeSkinCacheLookupCtx lookup_ctx;
    int level = (int)((location >> 10) & 0x3fu);
    int x = (int)((location >> 5) & 0x1fu);
    int y = (int)(location & 0x1fu);
    if (!profile || !out_skin || !profile->dungeon_handle ||
        level < 0 || level >= profile->dungeon_handle->level_count) {
        return 0;
    }
    memset(&lookup_ctx, 0, sizeof(lookup_ctx));
    lookup_ctx.profile = profile;
    lookup_ctx.dungeon = profile->dungeon_handle;
    *out_skin = csb_v1_skin_cache_get_skin(
        &profile->skin_cache,
        csb_v1_runtime_skin_cache_record_lookup,
        &lookup_ctx,
        level,
        profile->dungeon_handle->level_widths[level],
        profile->dungeon_handle->level_heights[level],
        x,
        y);
    return 1;
}

int csb_v1_runtime_set_csbwin_saved_skin(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int x,
    int y,
    uint8_t skin_num)
{
    CSB_V1_RuntimeProfile candidate;
    uint8_t column[CSB_V1_SKIN_CACHE_COLUMN_BYTES];
    uint32_t record_id;
    int index;
    int last_nonzero;
    size_t source_write_size;

    /* CSBWin DSA.cpp:3122-3135 decodes the five-bit x/y and six-bit level
     * from SETSKIN's location word. data.cpp:2130-2167 then reads exactly
     * one EDT_Skins DB11 record, changes one byte, trims zero suffixes, and
     * writes it back through EXPOOL. Use that actual Read/Write contract:
     * the replacement must be satisfied by the original save tail's exact
     * DB11 free list. We deliberately do not call EXPOOL::enlarge or invent
     * a new block when the source tail has no suitable free node. */
    if (!profile || level < 0 || level >= CSB_V1_SKIN_CACHE_MAX_LEVELS ||
        x < 0 || x >= 32 || y < 0 || y >= 32) {
        return 0;
    }
    record_id = csb_v1_skin_cache_column_record_id(level, x);
    index = 2 * y + (x & 1);
    if (index < 0 || index >= CSB_V1_SKIN_CACHE_COLUMN_BYTES) return 0;

    candidate = *profile;
    memset(column, 0, sizeof(column));
    {
        const uint8_t *existing = NULL;
        size_t existing_size = 0u;

        if (csb_v1_runtime_locate_appended_expool_record_internal(
                &candidate, record_id, &existing, &existing_size)) {
            if (!existing || existing_size > sizeof(column)) return 0;
            memcpy(column, existing, existing_size);
            if ((size_t)index < existing_size && column[index] == skin_num) {
                return 1;
            }
        } else if (skin_num == 0u) {
            return 1;
        }
    }
    column[index] = skin_num;
    last_nonzero = (int)sizeof(column) - 1;
    while (last_nonzero >= 0 && column[last_nonzero] == 0u) {
        --last_nonzero;
    }
    if (last_nonzero < 0) {
        if (!csb_v1_runtime_replace_appended_expool_record_internal(
                &candidate, record_id, NULL, 0u)) {
            return 0;
        }
    } else {
        source_write_size = (size_t)((last_nonzero + 4) / 4) * 4u;
        if (!csb_v1_runtime_replace_appended_expool_record_internal(
                &candidate, record_id, column, source_write_size)) {
            return 0;
        }
    }
    candidate.csbwin_appended_tail_fnv1a = csb_v1_runtime_fnv1a32(
        candidate.csbwin_appended_tail,
        candidate.csbwin_appended_tail_preserved_size);
    /* Force the next HUD read through the newly authenticated source tail. */
    csb_v1_skin_cache_init(&candidate.skin_cache);
    candidate.csbwin_skin_cache_tail_receipt_valid = 0;
    *profile = candidate;
    return 1;
}
static int csb_v1_runtime_dsa_set_skin(void *user,
                                       uint32_t location,
                                       uint8_t skin)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    int level = (int)((location >> 10) & 0x3fu);
    int x = (int)((location >> 5) & 0x1fu);
    int y = (int)(location & 0x1fu);
    if (!profile || !profile->dungeon_handle ||
        level < 0 || level >= profile->dungeon_handle->level_count) {
        return 0;
    }
    return csb_v1_skin_cache_set_skin(
        &profile->skin_cache,
        level,
        profile->dungeon_handle->level_widths[level],
        profile->dungeon_handle->level_heights[level],
        x,
        y,
        skin);
}

int csb_v1_runtime_read_csbwin_extended_cell_flags(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t location,
    uint32_t out_words[8])
{
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;
    uint32_t record_id;
    unsigned int i;

    if (!out_words || !profile || !profile->csbwin_appended_tail_valid ||
        profile->csbwin_appended_tail_truncated ||
        profile->csbwin_appended_tail_fnv1a != csb_v1_runtime_fnv1a32(
            profile->csbwin_appended_tail,
            profile->csbwin_appended_tail_preserved_size)) {
        return -1;
    }
    memset(out_words, 0, 8u * sizeof(out_words[0]));
    record_id = (2u << 24) | (((location >> 10) & 63u) << 5) |
        ((location >> 5) & 31u);
    if (!csb_v1_runtime_locate_appended_expool_record_internal(
            profile, record_id, &payload, &payload_size) ||
        !payload || payload_size < 8u * sizeof(uint32_t)) {
        return 1;
    }
    for (i = 0u; i < 8u; ++i) {
        out_words[i] = csb_v1_runtime_read_le32(
            payload + i * sizeof(uint32_t));
    }
    return 1;
}

int csb_v1_runtime_set_csbwin_extended_cell_flags(
    CSB_V1_RuntimeProfile *profile,
    uint32_t location,
    uint32_t flags)
{
    CSB_V1_RuntimeProfile candidate;
    uint32_t words[8];
    uint8_t payload[8u * sizeof(uint32_t)];
    uint32_t record_id;
    uint32_t mask;
    unsigned int i;

    if (!profile || csb_v1_runtime_read_csbwin_extended_cell_flags(
            profile, location, words) != 1) {
        return -1;
    }
    candidate = *profile;
    mask = 1u << (location & 31u);
    for (i = 0u; i < 8u; ++i) {
        words[i] &= ~mask;
        if ((flags & 1u) != 0u) words[i] |= mask;
        flags >>= 1;
        csb_v1_runtime_write_le32(payload + i * sizeof(uint32_t), words[i]);
    }
    record_id = (2u << 24) | (((location >> 10) & 63u) << 5) |
        ((location >> 5) & 31u);
    if (!csb_v1_runtime_replace_appended_expool_record_internal(
            &candidate, record_id, payload, sizeof(payload))) {
        return -1;
    }
    candidate.csbwin_appended_tail_fnv1a = csb_v1_runtime_fnv1a32(
        candidate.csbwin_appended_tail,
        candidate.csbwin_appended_tail_preserved_size);
    *profile = candidate;
    return 1;
}

static int csb_v1_runtime_dsa_get_excell_flags(void *user,
                                                uint32_t location,
                                                uint32_t out_words[8])
{
    return csb_v1_runtime_read_csbwin_extended_cell_flags(
        (const CSB_V1_RuntimeProfile *)user, location, out_words);
}

static int csb_v1_runtime_dsa_set_excell_flags(void *user,
                                                uint32_t location,
                                                uint32_t flags)
{
    return csb_v1_runtime_set_csbwin_extended_cell_flags(
        (CSB_V1_RuntimeProfile *)user, location, flags) == 1;
}

static int csb_v1_runtime_dsa_get_generator_delay(void *user,
                                                   uint32_t location,
                                                   int *out_delay)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_DungeonData *dungeon;
    int level = (int)((location >> 10) & 63u);
    int map_x = (int)((location >> 5) & 31u);
    int map_y = (int)(location & 31u);
    int raw_square;
    int thing;
    int guard;

    if (!out_delay || !profile || !profile->dungeon_handle) return 0;
    *out_delay = -1;
    dungeon = (const CSB_V1_DungeonData *)profile->dungeon_handle;
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    /* Code11f52.cpp FindFirstMonsterGenerator rejects roomSTONE before its
     * object-chain scan.  Original compact map room type occupies bits 5..7. */
    if (raw_square < 0 || ((raw_square >> 5) & 7) == 0) return 1;
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    for (guard = 0; thing >= 0 && thing != 0xfffe && thing != 0xffff &&
         guard < 128; ++guard) {
        const uint8_t *record;
        int type;
        int size;

        record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                  &type, NULL, &size);
        if (!record || size < 2) return 0;
        if (type == CSB_V1_THING_TYPE_ACTUATOR && size >= 8 &&
            (((uint16_t)record[2] | ((uint16_t)record[3] << 8)) &
             0x007fu) == 6u) {
            *out_delay = record[7];
            return 1;
        }
        thing = (int)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
    }
    return guard < 128 ? 1 : 0;
}

static int csb_v1_runtime_dsa_set_generator_delay(void *user,
                                                   uint32_t location,
                                                   int delay)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    int level = (int)((location >> 10) & 63u);
    int map_x = (int)((location >> 5) & 31u);
    int map_y = (int)(location & 31u);
    int raw_square;
    int thing;
    uint8_t *disabled = NULL;
    int guard;

    if (!profile || !profile->dungeon_handle) return 0;
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0 || ((raw_square >> 5) & 7) == 0) return 1;
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    for (guard = 0; thing >= 0 && thing != 0xfffe && thing != 0xffff &&
         guard < 128; ++guard) {
        uint8_t *record;
        int type;
        int size;
        uint16_t type_data;

        record = csb_v1_runtime_mutable_thing_record(dungeon, (uint16_t)thing,
                                                      &type, &size);
        if (!record || size < 2) return 0;
        if (type == CSB_V1_THING_TYPE_ACTUATOR && size >= 8) {
            type_data = csb_v1_runtime_read_u16(record + 2);
            if ((type_data & 0x007fu) == 6u) {
                record[7] = (uint8_t)delay;
                return 1;
            }
            if ((type_data & 0x007fu) == 0u && !disabled) disabled = record;
        }
        thing = (int)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
    }
    if (guard >= 128) return 0;
    if (disabled) disabled[7] = (uint8_t)delay;
    return 1;
}

static int csb_v1_runtime_dsa_commit_generator_delay(
    void *user, uint32_t location, int expected_delay, int delay)
{
    int current_delay = -1;

    if (!csb_v1_runtime_dsa_get_generator_delay(user, location,
                                                 &current_delay) ||
        current_delay != expected_delay) return 0;
    return csb_v1_runtime_dsa_set_generator_delay(user, location, delay);
}

static int csb_v1_runtime_dsa_get_monster_info(void *user,
                                                uint16_t thing,
                                                uint32_t out_values[8])
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t flags;
    int type;
    int index;
    int size;

    if (!profile || !profile->dungeon_handle || !out_values) return 0;
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    memset(out_values, 0, 8u * sizeof(out_values[0]));
    out_values[1] = UINT32_MAX;
    record = csb_v1_dungeon_get_thing_record(dungeon, thing,
                                              &type, &index, &size);
    (void)index;
    if (!record || type != CSB_V1_THING_TYPE_GROUP || size < 16) return 1;

    flags = csb_v1_runtime_read_u16(record + 14);
    out_values[0] = ((uint32_t)(flags >> 5) & 0x03u) + 1u;
    out_values[1] = (uint32_t)(record[4] & 0x1fu);
    out_values[2] = csb_v1_runtime_read_u16(record + 6);
    out_values[3] = csb_v1_runtime_read_u16(record + 8);
    out_values[4] = csb_v1_runtime_read_u16(record + 10);
    out_values[5] = csb_v1_runtime_read_u16(record + 12);
    if ((profile->csbwin_extended_features_flags32 & 0x00000002u) != 0u &&
        (flags & 0x0800u) != 0u) {
        out_values[6] |= 1u;
    }
    if ((profile->csbwin_extended_features_flags32 & 0x00000004u) != 0u) {
        if ((flags & 0x1000u) != 0u) out_values[6] |= 2u;
        if ((flags & 0x2000u) != 0u) out_values[6] |= 4u;
        if ((flags & 0x4000u) != 0u) out_values[6] |= 8u;
    }
    out_values[7] = (uint32_t)(record[4] >> 5);
    return 1;
}

static int csb_v1_runtime_dsa_set_monster_info(void *user,
                                                uint16_t thing,
                                                const uint32_t values[8],
                                                uint8_t write_mask)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    uint8_t *record;
    uint16_t flags;
    int type;
    int size;
    int i;

    if (!profile || !profile->dungeon_handle || !values || write_mask == 0u) {
        return 0;
    }
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    record = csb_v1_runtime_mutable_thing_record(dungeon, thing,
                                                  &type, &size);
    if (!record || type != CSB_V1_THING_TYPE_GROUP || size < 16) return 0;
    for (i = 0; i < 4; ++i) {
        if ((write_mask & (uint8_t)(1u << (2 + i))) != 0u) {
            csb_v1_runtime_write_u16(record + 6 + i * 2,
                                     (uint16_t)values[2 + i]);
        }
    }
    if ((write_mask & (1u << 6)) != 0u) {
        flags = csb_v1_runtime_read_u16(record + 14);
        flags = (uint16_t)((flags & ~(uint16_t)0x7800u) |
                           (uint16_t)((values[6] & 0x0fu) << 11));
        csb_v1_runtime_write_u16(record + 14, flags);
    }
    if ((write_mask & (1u << 7)) != 0u) {
        record[4] = (uint8_t)((record[4] & 0x1fu) |
                              ((values[7] & 0x07u) << 5));
    }
    return 1;
}

/* CSBWin DSA.cpp STKOP_ChPoss:3330-3356.  The four normal champion
 * selectors address the persisted CHARDESC slot array; selector four is
 * normalized by the executor to GAMEBLOCK2.handChar and is therefore still
 * a real loaded party member. */
static int csb_v1_runtime_dsa_get_champion_possession(
    void *user, int champion_index, uint32_t slot_index, int32_t *out_thing)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    uint16_t thing;

    if (!out_thing || !profile || !profile->party_state_valid) return -1;
    *out_thing = -1;
    if (slot_index >= CSB_V1_SLOT_COUNT) return 1;
    if (champion_index < 0) {
        thing = csb_v1_runtime_export_leader_hand_thing(profile);
    } else {
        if (champion_index >= profile->party_state.ChampionCount ||
            champion_index >= CSB_V1_MAX_CHAMPIONS) {
            return 1;
        }
        thing = profile->party_state.Champions[champion_index].Slots[slot_index];
    }
    if (thing != THING_NONE && thing != THING_ENDOFLIST) {
        *out_thing = (int32_t)thing;
    }
    return 1;
}

/* CSBWin DSA.cpp STKOP_MonPoss:3358-3386 starts at DB4.possession2 (+2)
 * and follows DBCOMMON.next (+0).  Never derive a possession list from
 * renderer/runtime metadata: the raw loaded record chain is authoritative. */
static int csb_v1_runtime_dsa_get_monster_possession(
    void *user, uint16_t monster_thing, uint32_t possession_index,
    int32_t *out_thing)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t thing;
    int type;
    int size;
    int guard;

    if (!out_thing || !profile || !profile->dungeon_handle) return -1;
    *out_thing = -1;
    dungeon = (const CSB_V1_DungeonData *)profile->dungeon_handle;
    record = csb_v1_dungeon_get_thing_record(dungeon, monster_thing,
                                              &type, NULL, &size);
    if (!record || type != CSB_V1_THING_TYPE_GROUP || size < 16) return 1;

    thing = csb_v1_runtime_read_u16(record + 2);
    for (guard = 0;
         possession_index > 0u && thing != THING_NONE &&
             thing != THING_ENDOFLIST && guard < 128;
         ++guard, --possession_index) {
        record = csb_v1_dungeon_get_thing_record(dungeon, thing,
                                                  NULL, NULL, &size);
        if (!record || size < 2) return -1;
        thing = csb_v1_runtime_read_u16(record);
    }
    if (possession_index > 0u && thing != THING_NONE &&
        thing != THING_ENDOFLIST) {
        return -1;
    }
    if (thing != THING_NONE && thing != THING_ENDOFLIST) {
        *out_thing = (int32_t)thing;
    }
    return 1;
}

/* CSBWin DSA.cpp ExamineCell:2210-2309.  This owns no synthesized room or
 * Thing chain: every bit is classified from the loaded CELLFLAG byte and
 * the existing DB1/DB4 records in the same original dungeon. */
static int csb_v1_runtime_dsa_inspect_cells(
    void *user, uint32_t location, uint32_t criteria_mask,
    uint32_t first_cell, uint32_t last_cell, uint32_t *out_result)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_DungeonData *dungeon;
    uint32_t result = 0u;
    uint32_t result_bit = 1u;
    uint32_t i;
    int level;
    int base_x;
    int base_y;

    if (!out_result || !profile || !profile->dungeon_handle ||
        first_cell > last_cell || last_cell > 4u) {
        return -1;
    }
    dungeon = (const CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1) return -1;
    level = (int)((location >> 10) & 0x3fu);
    base_x = (int)((location >> 5) & 0x1fu);
    base_y = (int)(location & 0x1fu);

    for (i = first_cell; i <= last_cell; ++i, result_bit <<= 1) {
        int map_x = base_x;
        int map_y = base_y;
        int raw_square;
        int room_type;
        uint32_t cell_bits = 0u;

        if (i == 0u) --map_y;
        else if (i == 1u) ++map_x;
        else if (i == 2u) ++map_y;
        else if (i == 3u) --map_x;

        raw_square = csb_v1_dungeon_get_raw_square(dungeon, level,
                                                    map_x, map_y);
        if (raw_square < 0) {
            cell_bits = (1u << 0) | (1u << 31);
        } else {
            int thing;
            int guard;
            room_type = (raw_square >> 5) & 0x07;
            switch (room_type) {
            case 0: /* roomSTONE */
                cell_bits = (1u << 1) | (1u << 31);
                break;
            case 1: /* roomOPEN */
            case 2: /* roomPIT */
            case 3: /* roomSTAIRS */
            case 4: /* roomDOOR */
            case 5: /* roomTELEPORTER */
            case 6: /* roomFALSEWALL */
                break;
            default:
                return -1;
            }
            if (room_type == 1) {
                cell_bits |= 1u << 19;
            } else if (room_type == 2) {
                if ((raw_square & 0x01) != 0) cell_bits |= 1u << 4;
                else if ((raw_square & 0x08) != 0) cell_bits |= 1u << 2;
                else cell_bits |= 1u << 3;
            } else if (room_type == 3) {
                cell_bits |= (raw_square & 0x04) != 0 ? 1u << 23 : 1u << 24;
            } else if (room_type == 4) {
                cell_bits |= 1u << (((raw_square & 0x07) > 5) ? 9 :
                                    ((raw_square & 0x07) + 5));
            } else if (room_type == 5) {
                int found_teleporter = 0;
                thing = csb_v1_dungeon_get_first_thing(dungeon, level,
                                                        map_x, map_y);
                for (guard = 0;
                     thing >= 0 && thing != THING_NONE &&
                         thing != THING_ENDOFLIST && guard < 128;
                     ++guard) {
                    const uint8_t *record;
                    int type;
                    int size;

                    record = csb_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)thing, &type, NULL, &size);
                    if (!record || size < 2) return -1;
                    if (type == THING_TYPE_TELEPORTER && size >= 4) {
                        uint16_t word2 = csb_v1_runtime_read_u16(record + 2);
                        int bit = 11 + (int)((word2 >> 10) & 0x03u);
                        if ((raw_square & 0x08) == 0) ++bit;
                        cell_bits |= 1u << bit;
                        found_teleporter = 1;
                        break;
                    }
                    thing = (int)csb_v1_runtime_read_u16(record);
                }
                if (guard >= 128 && !found_teleporter) return -1;
            } else if (room_type == 6) {
                if ((raw_square & 0x04) != 0) cell_bits |= 1u << 22;
                else if ((raw_square & 0x01) != 0) cell_bits |= 1u << 20;
                else cell_bits |= 1u << 21;
            }

            if (profile->party_state_valid && profile->current_level == level &&
                profile->party_x == map_x && profile->party_y == map_y) {
                cell_bits |= 1u << 29;
            } else {
                int found_group = 0;
                thing = csb_v1_dungeon_get_first_thing(dungeon, level,
                                                        map_x, map_y);
                for (guard = 0;
                     thing >= 0 && thing != THING_NONE &&
                         thing != THING_ENDOFLIST && guard < 128;
                     ++guard) {
                    const uint8_t *record;
                    int type;
                    int size;

                    record = csb_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)thing, &type, NULL, &size);
                    if (!record || size < 2) return -1;
                    if (type == CSB_V1_THING_TYPE_GROUP) {
                        found_group = 1;
                        break;
                    }
                    thing = (int)csb_v1_runtime_read_u16(record);
                }
                if (guard >= 128 && !found_group) return -1;
                cell_bits |= found_group ? 1u << 30 : 1u << 31;
            }
        }
        if ((criteria_mask & cell_bits & 0x01ffffffu) != 0u &&
            (criteria_mask & cell_bits & 0xe0000000u) != 0u) {
            result |= result_bit;
        }
    }
    *out_result = result;
    return 1;
}

/* CSBWin DSA.cpp EX_TYPE:1388-1511.  This is deliberately a raw DB decoder:
 * Firestaff object names, icons, and renderer metadata are not inputs. */
static int csb_v1_runtime_dsa_get_thing_type(
    void *user, int32_t thing_index, int32_t *out_type)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word2;
    int type;
    int size;
    int id = 0;

    if (!out_type || !profile || !profile->dungeon_handle) return -1;
    *out_type = -1;
    if (thing_index < 0 || thing_index > UINT16_MAX) return 1;
    dungeon = (const CSB_V1_DungeonData *)profile->dungeon_handle;
    record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing_index,
                                              &type, NULL, &size);
    if (!record || size < 2) return 1;
    if (type >= 0 && type <= 10 && size < 4) return -1;
    word2 = size >= 4 ? csb_v1_runtime_read_u16(record + 2) : 0u;
    switch (type) {
    case 0: /* DB0 */
        id = (word2 >> 6) & 1;
        if ((word2 & 0x0100u) != 0) id |= 2;
        if ((word2 & 0x0080u) != 0) id |= 4;
        id |= ((word2 >> 5) & 1) << 3;
        id |= ((word2 >> 1) & 15) << 4;
        id |= (word2 & 1) << 8;
        break;
    case 1: /* DB1 */
        id = (word2 >> 10) & 3;
        id |= ((word2 >> 12) & 1) << 2;
        id |= ((word2 >> 13) & 3) << 3;
        id |= ((word2 >> 15) & 1) << 5;
        break;
    case 2: /* DB2 */
        id = word2 & 1;
        break;
    case 3: /* DB3 */
        if (size < 6) return -1;
        id = (csb_v1_runtime_read_u16(record + 4) >> 6) & 1;
        id |= ((csb_v1_runtime_read_u16(record + 4) >> 2) & 1) << 1;
        id |= (word2 & 0x007fu) << 2;
        break;
    case 4: /* DB4 */
        if (size < 5) return -1;
        id = record[4] & 0x1f;
        break;
    case 5: /* DB5 */
    case 6: /* DB6 */
    case 10: /* DB10 */
        id = word2 & 0x007f;
        break;
    case 7: /* DB7 */
        id = 0;
        break;
    case 8: /* DB8 */
        id = (word2 & 0x00ffu) | (((word2 >> 8) & 0x007fu) << 8);
        break;
    case 9: /* DB9 */
        {
            uint16_t child = word2;
            int guard;

            if (size < 8) return -1;
            for (guard = 0;
                 child != THING_NONE && child != THING_ENDOFLIST &&
                     guard < 128;
                 ++guard) {
                const uint8_t *child_record;
                int child_size;

                child_record = csb_v1_dungeon_get_thing_record(
                    dungeon, child, NULL, NULL, &child_size);
                if (!child_record || child_size < 2) return -1;
                ++id;
                child = csb_v1_runtime_read_u16(child_record);
            }
            if (guard >= 128 && child != THING_NONE &&
                child != THING_ENDOFLIST) return -1;
        }
        break;
    case 11: /* DB11 / EXPOOL */
    case 12: /* source unused DB12 */
    case 13: /* source unused DB13 */
    case 14: /* DB14 missile */
        id = 0;
        break;
    default:
        /* Firestaff's type 15 currently denotes its DSA record surface,
         * while CSBWin's DB15 is a cloud.  Do not label it as either until
         * the original raw layout is proven. */
        return -1;
    }
    *out_type = type * 10000 + id;
    return 1;
}

static int csb_v1_runtime_dsa_carried_chain(
    const CSB_V1_DungeonData *dungeon, uint16_t thing, int32_t object_selector,
    int *count, int *found, int depth)
{
    int guard = 0;

    if (!dungeon || !count || !found || depth > 30) return -1;
    while (thing != THING_NONE && thing != THING_ENDOFLIST && guard++ < 128) {
        const uint8_t *record;
        int type;
        int size;

        record = csb_v1_dungeon_get_thing_record(dungeon, thing, &type,
                                                  NULL, &size);
        if (!record || size < 2) return -1;
        if (object_selector >= 0) {
            if ((int32_t)thing == object_selector) {
                *found = 1;
                return 1;
            }
        } else {
            int wanted = -object_selector;
            if (wanted == 1) wanted = 0;
            if (type == wanted) ++*count;
        }
        if (type == 9) {
            if (size < 8 || csb_v1_runtime_dsa_carried_chain(
                    dungeon, csb_v1_runtime_read_u16(record + 2),
                    object_selector, count, found, depth + 1) < 0) return -1;
            if (*found) return 1;
        }
        thing = csb_v1_runtime_read_u16(record);
    }
    return guard >= 128 ? -1 : 1;
}

static int csb_v1_runtime_dsa_is_carried(
    void *user, int32_t character_selector, int32_t object_selector,
    int32_t *out_result)
{
    const CSB_V1_RuntimeProfile *profile = user;
    const CSB_V1_DungeonData *dungeon;
    int champion_index;
    int count = 0;

    if (!out_result || !profile || !profile->party_state_valid ||
        !profile->dungeon_handle) return -1;
    dungeon = profile->dungeon_handle;
    for (champion_index = 0; champion_index < profile->party_state.ChampionCount &&
         champion_index < CSB_V1_MAX_CHAMPIONS; ++champion_index) {
        const CSB_V1_Champion *champion;
        int slot;
        int selected = character_selector == champion_index ||
            (character_selector == 4 && champion_index == profile->leader_index) ||
            character_selector == 5;
        if (!selected) continue;
        champion = &profile->party_state.Champions[champion_index];
        for (slot = 0; slot < CSB_V1_SLOT_COUNT; ++slot) {
            int found = 0;
            if (csb_v1_runtime_dsa_carried_chain(dungeon, champion->Slots[slot],
                                                 object_selector, &count, &found, 0) < 0) return -1;
            if (found) { *out_result = slot + 256 * champion_index; return 1; }
        }
        if (champion_index == profile->leader_index) {
            int found = 0;
            if (csb_v1_runtime_dsa_carried_chain(
                    dungeon, csb_v1_runtime_export_leader_hand_thing(profile),
                    object_selector, &count, &found, 0) < 0) return -1;
            if (found) { *out_result = 255 + 256 * champion_index; return 1; }
        }
    }
    *out_result = object_selector >= 0 ? -1 : count;
    return 1;
}

static int csb_v1_runtime_dsa_get_level_multiplier(
    void *user, int32_t level, int32_t *out_multiplier)
{
    const CSB_V1_RuntimeProfile *profile = user;
    const CSB_V1_DungeonData *dungeon;
    if (!out_multiplier || !profile || !profile->dungeon_handle) return -1;
    dungeon = profile->dungeon_handle;
    /* LEVELDESC.word12 exists only in the original 16-byte descriptor path. */
    if (!dungeon->raw_data || dungeon->square_bytes != 1) return -1;
    *out_multiplier = (level >= 0 && level < dungeon->level_count)
        ? dungeon->map_experience_multiplier[level] : 1;
    return 1;
}

/* CSBWin DSA.cpp:2795-2822 reads DB14's exact four mutable fields and the
 * owning TIMER word8 direction. Both owners must come from one complete
 * loaded CSBWin save/runtime profile; DB14 alone cannot supply direction. */
static int csb_v1_runtime_dsa_get_missile_info(
    void *user, uint16_t thing, uint32_t out_values[4])
{
    const CSB_V1_RuntimeProfile *profile = user;
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    const CSB_V1_CSBWin512TimerSummary *timer;
    uint16_t timer_index;
    int type;
    int size;

    if (!out_values || !profile || !profile->dungeon_handle) return -1;
    out_values[0] = UINT32_MAX;
    out_values[1] = UINT32_MAX;
    out_values[2] = UINT32_MAX;
    out_values[3] = UINT32_MAX;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1 ||
        !profile->csbwin_body_runtime_summary_valid ||
        profile->csbwin_timer_summary_total !=
            profile->csbwin_timer_summary_count ||
        profile->csbwin_timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES) {
        return -1;
    }
    record = csb_v1_dungeon_get_thing_record(dungeon, thing, &type, NULL,
                                              &size);
    if (!record || type != 14) return 0;
    if (size < 8) return -1;
    timer_index = csb_v1_runtime_read_u16(record + 6);
    if (timer_index >= profile->csbwin_timer_summary_count) return -1;
    timer = &profile->csbwin_timers[timer_index];
    if (!timer->valid || timer->truncated ||
        timer->source_index != timer_index) return -1;
    out_values[0] = csb_v1_runtime_read_u16(record + 2);
    out_values[1] = record[4];
    out_values[2] = record[5];
    out_values[3] = (timer->ubyte8 >> 2) & 0x03u;
    return 1;
}

/* Commit the two source owners together: DB14 range/damage and the saved
 * missile timer's word8 direction. The mapped live timer event receives the
 * same byte, preserving the existing original-save queue contract. */
static int csb_v1_runtime_dsa_set_missile_info(
    void *user, uint16_t thing, const uint32_t values[4])
{
    CSB_V1_RuntimeProfile *profile = user;
    CSB_V1_DungeonData *dungeon;
    uint8_t *record;
    CSB_V1_CSBWin512TimerSummary *timer;
    struct DM1_Event_V1 *event = NULL;
    uint16_t timer_index;
    uint16_t event_index;
    int type;
    int size;
    int matches = 0;

    if (!profile || !profile->dungeon_handle || !values) return 0;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1 ||
        !profile->csbwin_body_runtime_summary_valid ||
        profile->csbwin_timer_summary_total !=
            profile->csbwin_timer_summary_count ||
        profile->csbwin_timer_queue_summary_total !=
            profile->csbwin_timer_queue_summary_count ||
        !csb_v1_runtime_validate_csbwin_timer_heap(profile)) {
        return 0;
    }
    record = csb_v1_runtime_mutable_thing_record(dungeon, thing, &type, &size);
    if (!record || type != 14 || size < 8) return 0;
    timer_index = csb_v1_runtime_read_u16(record + 6);
    if (timer_index >= profile->csbwin_timer_summary_count) return 0;
    timer = &profile->csbwin_timers[timer_index];
    if (!timer->valid || timer->truncated ||
        timer->source_index != timer_index) return 0;
    for (event_index = 0u; event_index < DM1_EVENT_MAX_COUNT; ++event_index) {
        const uint16_t queue_slot =
            profile->csbwin_timeline_event_queue_slot[event_index];
        if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
            profile->csbwin_timer_queue[queue_slot] != timer_index) {
            continue;
        }
        if (++matches != 1 || event_index >= DM1_EVENT_MAX_COUNT) return 0;
        event = &profile->timeline_queue.events[event_index];
    }
    if (matches != 1 || !event) return 0;
    record[4] = (uint8_t)values[1];
    record[5] = (uint8_t)values[2];
    timer->ubyte8 = (uint8_t)((timer->ubyte8 & ~0x0cu) |
                              ((values[3] & 3u) << 2));
    event->c_cell = timer->ubyte8;
    return 1;
}

static int csb_v1_runtime_dsa_commit_missile_info(
    void *user, uint16_t thing, const uint32_t expected_values[4],
    const uint32_t values[4])
{
    uint32_t current_values[4];

    if (!expected_values || !values ||
        csb_v1_runtime_dsa_get_missile_info(user, thing, current_values) != 1 ||
        memcmp(current_values, expected_values, sizeof(current_values)) != 0) {
        return 0;
    }
    return csb_v1_runtime_dsa_set_missile_info(user, thing, values);
}

/* CSBWin DSA.cpp:3411-3675.  These opcodes operate on the raw DB5/DB6/DB8/
 * DB10 word2 field, never on Firestaff object metadata.  Return zero for the
 * original silent wrong-type/invalid-Thing result and -1 only when no loaded
 * original dungeon can own the request. */
static int csb_v1_runtime_dsa_get_object_property(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t *out_value)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word;
    int type;
    int size;

    if (!out_value || !profile || !profile->dungeon_handle) return -1;
    *out_value = 0u;
    dungeon = profile->dungeon_handle;
    record = csb_v1_dungeon_get_thing_record(dungeon, thing,
                                              &type, NULL, &size);
    if (!record || size < 4) return 0;
    word = csb_v1_runtime_read_u16(record + 2);
    switch (property) {
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE:
        if (type != THING_TYPE_WEAPON && type != THING_TYPE_ARMOUR &&
            type != THING_TYPE_JUNK) return 0;
        *out_value = (word >> 8) & 1u;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN:
        if (type == THING_TYPE_WEAPON) *out_value = (word >> 14) & 1u;
        else if (type == THING_TYPE_ARMOUR) *out_value = (word >> 13) & 1u;
        else return 0;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED:
        if (type != THING_TYPE_WEAPON) return 0;
        *out_value = (word >> 9) & 1u;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES:
        if (type == THING_TYPE_WEAPON) *out_value = (word >> 10) & 0x0fu;
        else if (type == THING_TYPE_ARMOUR) *out_value = (word >> 9) & 0x0fu;
        else if (type == THING_TYPE_POTION) *out_value = word & 0x00ffu;
        else if (type == THING_TYPE_JUNK) *out_value = (word >> 14) & 0x03u;
        else return 0;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE:
        if (type != THING_TYPE_JUNK) return 0;
        *out_value = (word >> 9) & 0x1fu;
        return 1;
    default:
        return -1;
    }
}

static int csb_v1_runtime_dsa_set_object_property(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t value)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    uint8_t *record;
    uint16_t word;
    int type;
    int size;

    if (!profile || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    record = csb_v1_runtime_mutable_thing_record(dungeon, thing, &type, &size);
    if (!record || size < 4) return 1;
    word = csb_v1_runtime_read_u16(record + 2);
    switch (property) {
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE:
        if (type != THING_TYPE_WEAPON && type != THING_TYPE_ARMOUR &&
            type != THING_TYPE_JUNK) return 1;
        word = (uint16_t)((word & ~(uint16_t)0x0100u) |
                          (value != 0u ? 0x0100u : 0u));
        break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN:
        if (type == THING_TYPE_WEAPON) {
            word = (uint16_t)((word & ~(uint16_t)0x4000u) |
                              (value != 0u ? 0x4000u : 0u));
        } else if (type == THING_TYPE_ARMOUR) {
            word = (uint16_t)((word & ~(uint16_t)0x2000u) |
                              (value != 0u ? 0x2000u : 0u));
        } else return 1;
        break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED:
        if (type != THING_TYPE_WEAPON) return 1;
        word = (uint16_t)((word & ~(uint16_t)0x0200u) |
                          (value != 0u ? 0x0200u : 0u));
        break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES:
        if (type == THING_TYPE_WEAPON) {
            word = (uint16_t)((word & ~(uint16_t)0x3c00u) |
                              ((value & 0x0fu) << 10));
        } else if (type == THING_TYPE_ARMOUR) {
            word = (uint16_t)((word & ~(uint16_t)0x1e00u) |
                              ((value & 0x0fu) << 9));
        } else if (type == THING_TYPE_POTION) {
            word = (uint16_t)((word & ~(uint16_t)0x00ffu) |
                              (value & 0xffu));
        } else if (type == THING_TYPE_JUNK) {
            word = (uint16_t)((word & ~(uint16_t)0xc000u) |
                              ((value & 0x03u) << 14));
        } else return 1;
        break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE:
        if (type != THING_TYPE_JUNK) return 1;
        word = (uint16_t)((word & ~(uint16_t)0x3e00u) |
                          ((value & 0x1fu) << 9));
        break;
    default:
        return 0;
    }
    csb_v1_runtime_write_u16(record + 2, word);
    return 1;
}

static int csb_v1_runtime_dsa_normalize_object_property(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t input_value, uint32_t *out_value)
{
    const CSB_V1_RuntimeProfile *profile =
        (const CSB_V1_RuntimeProfile *)user;
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    int type;
    int size;

    if (!out_value || !profile || !profile->dungeon_handle) return -1;
    *out_value = 0u;
    dungeon = profile->dungeon_handle;
    record = csb_v1_dungeon_get_thing_record(dungeon, thing,
                                              &type, NULL, &size);
    if (!record || size < 4) return 0;
    switch (property) {
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE:
        if (type != THING_TYPE_WEAPON && type != THING_TYPE_ARMOUR &&
            type != THING_TYPE_JUNK) return 0;
        *out_value = input_value != 0u;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN:
        if (type != THING_TYPE_WEAPON && type != THING_TYPE_ARMOUR) return 0;
        *out_value = input_value != 0u;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED:
        if (type != THING_TYPE_WEAPON) return 0;
        *out_value = input_value != 0u;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES:
        if (type == THING_TYPE_WEAPON || type == THING_TYPE_ARMOUR) {
            *out_value = input_value & 0x0fu;
        } else if (type == THING_TYPE_POTION) {
            *out_value = input_value & 0xffu;
        } else if (type == THING_TYPE_JUNK) {
            *out_value = input_value & 0x03u;
        } else return 0;
        return 1;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE:
        if (type != THING_TYPE_JUNK) return 0;
        *out_value = input_value & 0x1fu;
        return 1;
    default:
        return -1;
    }
}

static int csb_v1_runtime_dsa_get_cell_info(void *user,
                                             uint32_t location,
                                             uint32_t out_values[5])
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    int level = (int)((location >> 10) & 0x3fu);
    int map_x = (int)((location >> 5) & 0x1fu);
    int map_y = (int)(location & 0x1fu);
    int raw_square;
    int room_type;
    int thing_type;
    int thing_index;
    int thing_size;
    int thing;
    uint16_t word2;
    uint16_t word4;

    if (!profile || !profile->dungeon_handle || !out_values) return 0;
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1) return 0;
    memset(out_values, 0, 5u * sizeof(out_values[0]));
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0) return 1;
    room_type = (raw_square >> 5) & 0x07;
    out_values[0] = (uint32_t)room_type;

    switch (room_type) {
    case 0: /* roomSTONE */
        if ((raw_square & 0x08) != 0) out_values[1] |= 0x01u;
        if ((raw_square & 0x04) != 0) out_values[1] |= 0x02u;
        if ((raw_square & 0x02) != 0) out_values[1] |= 0x04u;
        if ((raw_square & 0x01) != 0) out_values[1] |= 0x08u;
        break;
    case 1: /* roomOPEN */
        if ((raw_square & 0x08) != 0) out_values[1] |= 0x01u;
        break;
    case 2: /* roomSTAIRS */
        if ((raw_square & 0x04) != 0) out_values[1] |= 0x04u;
        if ((raw_square & 0x08) != 0) out_values[1] |= 0x08u;
        break;
    case 3: /* roomPIT */
        if ((raw_square & 0x01) != 0) out_values[1] |= 0x01u;
        if ((raw_square & 0x04) != 0) out_values[1] |= 0x04u;
        if ((raw_square & 0x08) != 0) out_values[1] |= 0x08u;
        break;
    case 6: /* roomFALSEWALL */
        if ((raw_square & 0x01) != 0) out_values[1] |= 0x01u;
        if ((raw_square & 0x04) != 0) out_values[1] |= 0x04u;
        break;
    case 5: /* roomTELEPORTER */
        if ((raw_square & 0x08) != 0) out_values[1] |= 0x08u;
        if ((raw_square & 0x04) != 0) out_values[1] |= 0x04u;
        thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
        if (thing < 0) break;
        record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                  &thing_type, &thing_index,
                                                  &thing_size);
        (void)thing_index;
        if (!record || thing_type != 1 || thing_size < 6) break;
        word2 = csb_v1_runtime_read_u16(record + 2);
        word4 = csb_v1_runtime_read_u16(record + 4);
        out_values[2] = ((uint32_t)(word2 >> 10) & 0x03u) |
                        (((uint32_t)(word2 >> 12) & 0x01u) << 2);
        out_values[3] = ((uint32_t)word2 >> 13) & 0x03u;
        out_values[4] = (((uint32_t)word4 >> 8) << 10) |
                        (((uint32_t)word2 & 0x001fu) << 5) |
                        (((uint32_t)word2 >> 5) & 0x001fu);
        break;
    case 4: /* roomDOOR */
        thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
        if (thing < 0) break;
        record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                  &thing_type, &thing_index,
                                                  &thing_size);
        (void)thing_index;
        if (!record || thing_type != 0 || thing_size < 4) break;
        word2 = csb_v1_runtime_read_u16(record + 2);
        if ((raw_square & 0x04) != 0) out_values[1] |= 0x01u;
        if ((word2 & 0x0020u) != 0) out_values[1] |= 0x02u;
        if ((word2 & 0x0040u) != 0) out_values[1] |= 0x04u;
        if ((word2 & 0x0080u) != 0) out_values[1] |= 0x08u;
        if ((word2 & 0x0100u) != 0) out_values[1] |= 0x10u;
        out_values[2] = (uint32_t)(raw_square & 0x07);
        out_values[3] = (uint32_t)(word2 & 0x01u);
        out_values[4] = ((uint32_t)word2 >> 1) & 0x0fu;
        break;
    default:
        break;
    }
    return 1;
}

static int csb_v1_runtime_dsa_queue_switch_action(
    void *user, uint32_t delay, uint32_t action, uint32_t target_location,
    int message_route, uint8_t *out_event_type)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    struct DM1_Event_V1 event;
    int level;
    int map_x;
    int map_y;
    int target_pos;
    int event_type;

    if (out_event_type) *out_event_type = 0u;
    if (!profile || action > 2u) return -1;
    level = (int)((target_location >> 10) & 0x3fu);
    map_x = (int)((target_location >> 5) & 0x1fu);
    map_y = (int)(target_location & 0x1fu);
    target_pos = (int)((target_location >> 16) & 0x03u);

    if (message_route == 'D') {
        event_type = 102; /* CSBWin TT_DESSAGE */
    } else if (message_route == 'M') {
        int raw_square;
        int room_type;

        if (!profile->dungeon_handle) return -1;
        dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
        if (!dungeon->raw_data || dungeon->square_bytes != 1) return -1;
        raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
        if (raw_square < 0) return -1;
        room_type = (raw_square >> 5) & 0x07;
        switch (room_type) {
        case 0: /* roomSTONE */
            event_type = DM1_EVENT_WALL;
            break;
        case 1: /* roomOPEN */
            target_pos = 0;
            event_type = DM1_EVENT_CORRIDOR;
            break;
        case 2: /* roomPIT */
            target_pos = 0;
            event_type = DM1_EVENT_PIT;
            break;
        case 4: /* roomDOOR */
            target_pos = 0;
            event_type = DM1_EVENT_DOOR;
            break;
        case 5: /* roomTELEPORTER */
            target_pos = 0;
            event_type = DM1_EVENT_TELEPORTER;
            break;
        case 6: /* roomFALSEWALL */
            event_type = DM1_EVENT_FAKEWALL;
            break;
        default:
            return 0;
        }
    } else {
        return -1;
    }

    memset(&event, 0, sizeof(event));
    event.map_time =
        DM1_MAP_TIME_MAKE((uint32_t)level, profile->game_time + delay);
    event.type = (uint8_t)event_type;
    event.priority = 0u;
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    event.c_cell = (uint8_t)target_pos;
    event.c_effect = (uint8_t)action;
    if (csb_v1_runtime_add_timeline_event(profile, &event) < 0) {
        return -1;
    }
    if (out_event_type) *out_event_type = (uint8_t)event_type;
    return 1;
}

static int csb_v1_runtime_dsa_resolve_cell_store(void *user,
                                                  uint32_t location,
                                                  uint32_t expected_room_type)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    int level = (int)((location >> 10) & 0x3fu);
    int map_x = (int)((location >> 5) & 0x1fu);
    int map_y = (int)(location & 0x1fu);
    int raw_square;
    int thing;
    int type;
    int index;
    int size;

    if (!profile || !profile->dungeon_handle) return -1;
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1) return -1;
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0 || ((uint32_t)(raw_square >> 5) & 0x07u) !=
                              expected_room_type) {
        return 0;
    }
    if (expected_room_type != 4u && expected_room_type != 5u) return 1;
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    if (thing < 0) return 0;
    record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                              &type, &index, &size);
    (void)index;
    if (!record || size < (expected_room_type == 4u ? 4 : 6) ||
        type != (expected_room_type == 4u ? 0 : 1)) {
        return 0;
    }
    return 1;
}

static int csb_v1_runtime_dsa_set_cell_info(void *user,
                                             uint32_t location,
                                             const uint32_t values[5],
                                             uint8_t write_mask)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    uint8_t *cell;
    uint8_t *record = NULL;
    int level = (int)((location >> 10) & 0x3fu);
    int map_x = (int)((location >> 5) & 0x1fu);
    int map_y = (int)(location & 0x1fu);
    int raw_square;
    int room_type;
    int thing;
    int type;
    int size;
    uint16_t word2;

    if (!profile || !profile->dungeon_handle || !values || write_mask == 0u) {
        return 0;
    }
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1 ||
        level < 0 || level >= dungeon->level_count || map_x < 0 ||
        map_x >= dungeon->level_widths[level] || map_y < 0 ||
        map_y >= dungeon->level_heights[level]) return 0;
    cell = dungeon->raw_data + dungeon->level_offsets[level] +
        map_x * dungeon->level_heights[level] + map_y;
    raw_square = *cell;
    room_type = (raw_square >> 5) & 0x07;
    if ((uint32_t)room_type != values[0]) return 0;
    if (room_type == 4 || room_type == 5) {
        thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
        if (thing < 0) return 0;
        record = csb_v1_runtime_mutable_thing_record(dungeon, (uint16_t)thing,
                                                      &type, &size);
        if (!record || type != (room_type == 4 ? 0 : 1) ||
            size < (room_type == 4 ? 4 : 6)) return 0;
    }
    switch (room_type) {
    case 0:
        if ((write_mask & (1u << 1)) != 0u) {
            raw_square &= 0xe0;
            if ((values[1] & 0x01u) != 0u) raw_square |= 0x08;
            if ((values[1] & 0x02u) != 0u) raw_square |= 0x04;
            if ((values[1] & 0x04u) != 0u) raw_square |= 0x02;
            if ((values[1] & 0x08u) != 0u) raw_square |= 0x01;
        }
        break;
    case 1:
        if ((write_mask & (1u << 1)) != 0u) {
            raw_square = (raw_square & ~0x08) |
                ((values[1] & 0x01u) != 0u ? 0x08 : 0);
        }
        break;
    case 3:
        if ((write_mask & (1u << 1)) != 0u) {
            raw_square &= 0xf2;
            if ((values[1] & 0x01u) != 0u) raw_square |= 0x01;
            if ((values[1] & 0x04u) != 0u) raw_square |= 0x04;
            if ((values[1] & 0x08u) != 0u) raw_square |= 0x08;
        }
        break;
    case 6:
        if ((write_mask & (1u << 1)) != 0u) {
            raw_square &= 0xfa;
            if ((values[1] & 0x01u) != 0u) raw_square |= 0x01;
            if ((values[1] & 0x04u) != 0u) raw_square |= 0x04;
        }
        break;
    case 5:
        if ((write_mask & (1u << 1)) != 0u) {
            raw_square = (raw_square & ~0x0c) | (int)(values[1] & 0x0cu);
        }
        word2 = csb_v1_runtime_read_u16(record + 2);
        if ((write_mask & (1u << 2)) != 0u) {
            word2 = (uint16_t)((word2 & ~(uint16_t)0x1c00u) |
                (uint16_t)((values[2] & 0x03u) << 10) |
                (uint16_t)((values[2] & 0x04u) << 10));
        }
        if ((write_mask & (1u << 3)) != 0u) {
            word2 = (uint16_t)((word2 & ~(uint16_t)0x6000u) |
                               (uint16_t)((values[3] & 0x03u) << 13));
        }
        csb_v1_runtime_write_u16(record + 2, word2);
        break;
    case 4:
        word2 = csb_v1_runtime_read_u16(record + 2);
        if ((write_mask & (1u << 1)) != 0u) {
            word2 = (uint16_t)((word2 & ~(uint16_t)0x01e0u) |
                               (uint16_t)((values[1] & 0x1eu) << 4));
        }
        if ((write_mask & (1u << 3)) != 0u) {
            word2 = (uint16_t)((word2 & ~(uint16_t)0x0001u) |
                               (uint16_t)(values[3] & 0x01u));
        }
        if ((write_mask & (1u << 4)) != 0u) {
            word2 = (uint16_t)((word2 & ~(uint16_t)0x001eu) |
                               (uint16_t)((values[4] & 0x0fu) << 1));
        }
        if ((write_mask & (1u << 2)) != 0u) {
            raw_square = (raw_square & ~0x07) | (int)(values[2] & 0x07u);
        }
        csb_v1_runtime_write_u16(record + 2, word2);
        break;
    default:
        break;
    }
    *cell = (uint8_t)raw_square;
    return 1;
}

static uint8_t *csb_v1_runtime_dsa_find_teleporter_record_at(
    CSB_V1_DungeonData *dungeon, int level, int map_x, int map_y,
    uint8_t **out_cell)
{
    int thing;
    int guard = 0;

    if (out_cell) *out_cell = NULL;
    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1 ||
        level < 0 || level >= dungeon->level_count ||
        map_x < 0 || map_x >= dungeon->level_widths[level] ||
        map_y < 0 || map_y >= dungeon->level_heights[level]) {
        return NULL;
    }
    if (out_cell) {
        *out_cell = dungeon->raw_data + dungeon->level_offsets[level] +
            map_x * dungeon->level_heights[level] + map_y;
    }
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && guard++ < 128) {
        uint8_t *record;
        int type;
        int size;

        record = csb_v1_runtime_mutable_thing_record(
            dungeon, (uint16_t)thing, &type, &size);
        if (!record) return NULL;
        if (type == 1 && size >= 6) return record;
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    return NULL;
}

static int csb_v1_runtime_dsa_copy_teleporter(
    void *user, uint32_t source_location, uint32_t destination_location)
{
    CSB_V1_RuntimeProfile *profile = (CSB_V1_RuntimeProfile *)user;
    CSB_V1_DungeonData *dungeon;
    uint8_t *source_cell = NULL;
    uint8_t *destination_cell = NULL;
    uint8_t *source_record;
    uint8_t *destination_record;
    int source_level = (int)((source_location >> 10) & 0x3fu);
    int source_x = (int)((source_location >> 5) & 0x1fu);
    int source_y = (int)(source_location & 0x1fu);
    int destination_level = (int)((destination_location >> 10) & 0x3fu);
    int destination_x = (int)((destination_location >> 5) & 0x1fu);
    int destination_y = (int)(destination_location & 0x1fu);

    if (!profile || !profile->dungeon_handle) return -1;
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    source_record = csb_v1_runtime_dsa_find_teleporter_record_at(
        dungeon, source_level, source_x, source_y, &source_cell);
    destination_record = csb_v1_runtime_dsa_find_teleporter_record_at(
        dungeon, destination_level, destination_x, destination_y,
        &destination_cell);
    if (!source_record || !destination_record) return 0;
    if (!source_cell || !destination_cell) return -1;

    /* DB1::copyTeleporter copies teleporter fields, not the linked-list
     * `Next` word. The CELLFLAG byte is copied separately by EX_COPYTELEPORTER. */
    memcpy(destination_record + 2, source_record + 2, 4u);
    *destination_cell = *source_cell;
    return 1;
}

int csb_v1_runtime_get_csbwin_dsa_tracing(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSATracingReport *out_report)
{
    if (!profile || !out_report || !profile->csbwin_dsa_tracing.valid) {
        return -1;
    }
    *out_report = profile->csbwin_dsa_tracing;
    return 0;
}

int csb_v1_runtime_restore_csbwin_expool_global_variables(
    CSB_V1_RuntimeProfile *profile)
{
    CSB_V1_RuntimeProfile candidate;

    if (!profile) return -1;
    candidate = *profile;
    if (csb_v1_runtime_stage_csbwin_global_variables(&candidate) != 0) {
        return -1;
    }
    profile->csbwin_global_variables_valid =
        candidate.csbwin_global_variables_valid;
    profile->csbwin_global_variable_count =
        candidate.csbwin_global_variable_count;
    memcpy(profile->csbwin_global_variables,
           candidate.csbwin_global_variables,
           sizeof(profile->csbwin_global_variables));
    return 0;
}

int csb_v1_runtime_restore_csbwin_expool_overlay_palette(
    CSB_V1_RuntimeProfile *profile)
{
    CSB_V1_RuntimeProfile candidate;

    if (!profile) return -1;
    candidate = *profile;
    if (csb_v1_runtime_stage_csbwin_overlay_palette(&candidate) != 0) {
        return -1;
    }
    profile->csbwin_overlay_palette_valid =
        candidate.csbwin_overlay_palette_valid;
    profile->csbwin_overlay_palette_tail_fnv1a =
        candidate.csbwin_overlay_palette_tail_fnv1a;
    memcpy(profile->csbwin_overlay_palette,
           candidate.csbwin_overlay_palette,
           sizeof(profile->csbwin_overlay_palette));
    return 0;
}

int csb_v1_runtime_get_csbwin_expool_overlay_palette(
    const CSB_V1_RuntimeProfile *profile,
    const uint8_t **out_palette,
    size_t *out_size)
{
    if (out_palette) *out_palette = NULL;
    if (out_size) *out_size = 0u;
    if (!profile || !out_palette || !out_size ||
        !profile->csbwin_overlay_palette_valid ||
        !profile->csbwin_appended_tail_valid ||
        profile->csbwin_appended_tail_truncated ||
        profile->csbwin_overlay_palette_tail_fnv1a !=
            profile->csbwin_appended_tail_fnv1a ||
        profile->csbwin_appended_tail_size !=
            profile->csbwin_appended_tail_preserved_size ||
        profile->csbwin_appended_tail_fnv1a !=
            csb_v1_runtime_fnv1a32(
                profile->csbwin_appended_tail,
                profile->csbwin_appended_tail_preserved_size)) {
        return 0;
    }
    *out_palette = profile->csbwin_overlay_palette;
    *out_size = sizeof(profile->csbwin_overlay_palette);
    return 1;
}

int csb_v1_runtime_set_csbwin_expool_overlay_palette(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t *palette,
    size_t palette_size)
{
    CSB_V1_RuntimeProfile candidate;

    if (!profile || !palette ||
        palette_size != CSB_V1_CSBWIN_OVERLAY_PALETTE_BYTES) {
        return -1;
    }
    candidate = *profile;
    memcpy(candidate.csbwin_overlay_palette, palette,
           sizeof(candidate.csbwin_overlay_palette));
    candidate.csbwin_overlay_palette_valid = 1;
    if (csb_v1_runtime_write_csbwin_overlay_palette(&candidate) != 0) {
        return -1;
    }
    *profile = candidate;
    return 0;
}

int csb_v1_runtime_csbwin_saves_disabled(
    const CSB_V1_RuntimeProfile *profile)
{
    return profile && profile->csbwin_saves_disabled ? 1 : 0;
}

int csb_v1_runtime_restore_csbwin_save_policy(
    CSB_V1_RuntimeProfile *profile)
{
    CSB_V1_RuntimeProfile candidate;

    if (!profile) return -1;
    candidate = *profile;
    if (csb_v1_runtime_stage_csbwin_save_policy(&candidate) != 0) {
        return -1;
    }
    profile->csbwin_saves_disabled = candidate.csbwin_saves_disabled;
    profile->csbwin_delete_duplicate_timers =
        candidate.csbwin_delete_duplicate_timers;
    profile->csbwin_debugging_data = candidate.csbwin_debugging_data;
    profile->csbwin_csbgraphics_signature_data =
        candidate.csbwin_csbgraphics_signature_data;
    profile->csbwin_graphics_signature_data =
        candidate.csbwin_graphics_signature_data;
    profile->csbwin_version_data = candidate.csbwin_version_data;
    return 0;
}

int csb_v1_runtime_get_csbwin_save_policy(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t *out_delete_duplicate_timers,
    uint32_t *out_debugging_data,
    uint32_t *out_csbgraphics_signature,
    uint32_t *out_graphics_signature,
    uint32_t *out_version)
{
    if (!profile || !out_delete_duplicate_timers || !out_debugging_data ||
        !out_csbgraphics_signature || !out_graphics_signature ||
        !out_version) {
        return -1;
    }
    *out_delete_duplicate_timers = profile->csbwin_delete_duplicate_timers;
    *out_debugging_data = profile->csbwin_debugging_data;
    *out_csbgraphics_signature = profile->csbwin_csbgraphics_signature_data;
    *out_graphics_signature = profile->csbwin_graphics_signature_data;
    *out_version = profile->csbwin_version_data;
    return 0;
}

int csb_v1_runtime_admit_csbwin_csbgraphics_plan(
    const CSB_V1_RuntimeProfile *profile,
    const struct CSB_V1_CSBGraphicsRuntimePlan *plan,
    CSB_V1_CSBWinGraphicsSignatureReceipt *out_receipt)
{
    if (!profile || !plan || !profile->csbwin_extended_features_valid ||
        !plan->ready || !plan->cache_loaded || plan->source_md5[0] == '\0') {
        return -1;
    }
    return csb_v1_csbwin_graphics_signature_gate_validate_md5(
        plan->source_md5, CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_CUSTOM,
        profile->csbwin_extended_csbgraphics_signature1,
        profile->csbwin_extended_csbgraphics_signature2,
        profile->csbwin_csbgraphics_signature_data,
        profile->csbwin_debugging_data, out_receipt) ==
        CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK ? 0 : -1;
}

int csb_v1_runtime_resolve_csbwin_dsa_filter_binding(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *location,
    CSB_V1_RuntimeDSAFilterBinding *out_binding)
{
    const uint8_t *record;
    CSB_V1_RuntimeDSAFilterBinding candidate;
    uint16_t word2;
    uint16_t mapped_dsa;
    int type;
    int index;
    int size;
    int i;

    if (!profile || !dungeon || !location || !out_binding ||
        !csb_v1_runtime_has_verified_csbwin_extended_dsa_tail(profile) ||
        !profile->csbwin_extended_level_index_present ||
        location->level < 0 || location->level >= 64 ||
        location->actuator_thing == 0xffffu) {
        return 0;
    }

    record = csb_v1_dungeon_get_thing_record(dungeon,
        location->actuator_thing, &type, &index, &size);
    (void)index;
    if (!record || type != CSB_V1_THING_TYPE_ACTUATOR || size < 4) {
        return 0;
    }
    word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    if ((word2 & 0x007fu) != CSB_V1_DSA_FILTER_ACTUATOR_TYPE) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.location = *location;
    candidate.dsa_selector = (uint8_t)((word2 >> 7) & 0x1fu);
    mapped_dsa = profile->csbwin_extended_level_dsa_index[
        location->level][candidate.dsa_selector];
    if (mapped_dsa == 0xffffu || mapped_dsa >= CSB_V1_MAX_DSA_SCRIPTS) {
        return 0;
    }
    candidate.dsa_id = (uint8_t)mapped_dsa;

    /* ProcessDSATimer6 rejects an undefined DSA after the selector lookup.
     * A real runtime binding is useful only when the staged authenticated
     * extension actually owns at least one action for that absolute DSA. */
    if (!profile->csbwin_extended_dsa_state.imported_headers[
            candidate.dsa_id].valid) {
        return 0;
    }
    for (i = 0; i < profile->csbwin_extended_dsa_state.imported_action_count;
         ++i) {
        if (profile->csbwin_extended_dsa_state.imported_actions[i].dsa_id ==
            candidate.dsa_id) {
            candidate.actuator_identity_valid = 1;
            *out_binding = candidate;
            return 1;
        }
    }
    return 0;
}

int csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    int timer_function,
    int timer_position,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    CSB_V1_RuntimeCSBWinDSATimer6Resolution candidate;
    const CSB_V1_CSBWinDSAImportedHeader *header;
    const CSB_V1_DSAImportedAction *action;
    const uint8_t *record;
    uint16_t word2;
    int type;
    int index;
    int size;
    int ordinal = 0;
    int i;

    if (!profile || !dungeon || !slave_location || !out_resolution ||
        timer_function < 0 || timer_function > 2 ||
        timer_position < 0 || timer_position > 3 ||
        !profile->csbwin_extended_features_valid) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_resolve_csbwin_dsa_filter_binding(
            profile, dungeon, slave_location, &candidate.slave)) {
        return 0;
    }
    header = &profile->csbwin_extended_dsa_state.imported_headers[
        candidate.slave.dsa_id];
    if (!header->valid || header->state_slot_count == 0u) return 0;

    /* CSBWin DSA.cpp FindMaster (534-547) supports only IsMaster(), which
     * is exactly LocalState != 3. Its slave branch calls "not implemented"
     * and returns the input object, so accepting it would invent ownership.
     * The supported branch has the slave itself as the authenticated master. */
    if (header->local_state == 3u) return 0;
    candidate.master = candidate.slave;
    candidate.master_location =
        ((uint32_t)(slave_location->position & 3) << 16) |
        ((uint32_t)(slave_location->level & 0x3f) << 10) |
        ((uint32_t)(slave_location->x & 0x1f) << 5) |
        (uint32_t)(slave_location->y & 0x1f);
    candidate.input_column = (uint32_t)(3 * timer_position + timer_function);

    record = csb_v1_dungeon_get_thing_record(dungeon,
        slave_location->actuator_thing, &type, &index, &size);
    (void)index;
    if (!record || type != CSB_V1_THING_TYPE_ACTUATOR || size < 4) return 0;
    word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    if ((word2 & 0x007fu) != CSB_V1_DSA_FILTER_ACTUATOR_TYPE) return 0;

    /* DSA.cpp GetState: LocalState 0 is DB3::DSAstate, while LocalState 1
     * is serialized DSA::m_state. data.cpp DB3::MakeBig moves raw word6 bits
     * 14..15 to expanded word8 bits 6..7, then masks word6 to fourteen bits.
     * ParameterB reads word8 bits 2..3 instead, so compact saved state is
     * exactly word6 & 0x3fff. Firestaff does not admit a writable expanded
     * word8 state record. */
    if (header->local_state == 0u) {
        candidate.state_index = (uint32_t)((word2 >> 12) & 0x0fu);
    } else if (header->local_state == 1u) {
        candidate.state_index = header->persistent_state;
    } else if (header->local_state == 2u) {
        uint16_t compact_word6;

        if (size < 8) return 0;
        compact_word6 = (uint16_t)record[6] | ((uint16_t)record[7] << 8);
        candidate.state_index = (uint32_t)(compact_word6 & 0x3fffu);
    } else {
        return 0;
    }
    if (candidate.state_index >= header->state_slot_count) return 0;

    action = csb_v1_chaos_find_imported_action_column(
        &profile->csbwin_extended_dsa_state, candidate.master.dsa_id,
        candidate.state_index, candidate.input_column);
    if (!action) return 0;
    for (i = 0; i < profile->csbwin_extended_dsa_state.imported_action_count;
         ++i) {
        const CSB_V1_DSAImportedAction *item =
            &profile->csbwin_extended_dsa_state.imported_actions[i];
        if (item->dsa_id == candidate.master.dsa_id &&
            item->state_index == candidate.state_index) {
            if (item == action) {
                candidate.action_ordinal = ordinal;
                *out_resolution = candidate;
                return 1;
            }
            ++ordinal;
        }
    }
    return 0;
}

int csb_v1_runtime_resolve_csbwin_stoneroom_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    /* CSBWin Timer.cpp ProcessTT_STONEROOM (lines 2180-2260) visits every
     * type-47 actuator on the timer target square, then hands its raw action
     * and position to DSA.cpp ProcessDSATimer6.  A restored summary is safe
     * to use only when it is the original function-6 form and its target
     * still names this concrete loaded-square actuator. */
    if (!profile || !dungeon || !slave_location || !timer || !out_resolution ||
        !timer->valid || timer->truncated || timer->function != 6u ||
        timer->ubyte9 > 2u || timer->ubyte8 > 3u ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y) {
        return 0;
    }
    return csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
        profile, dungeon, slave_location, (int)timer->ubyte9,
        (int)timer->ubyte8, out_resolution);
}

int csb_v1_runtime_resolve_csbwin_falsewall_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    /* CSBWin: Timer.cpp ProcessTT_FALSEWALL -> DSA.cpp ProcessDSATimer7. */
    if (!profile || !dungeon || !slave_location || !timer || !out_resolution ||
        !timer->valid || timer->truncated || timer->function != 7u ||
        timer->ubyte9 > 2u || timer->ubyte8 > 3u ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y) {
        return 0;
    }
    return csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
        profile, dungeon, slave_location, (int)timer->ubyte9,
        (int)timer->ubyte8, out_resolution);
}

int csb_v1_runtime_prepare_csbwin_falsewall_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;

    /* CSBWin Timer.cpp::ProcessTT_FALSEWALL passes function-7 records to
     * DSA.cpp::ProcessDSATimer7, which selects the same authenticated
     * ProcessDSATimer6 action receipt.  Retain that exact selection rather
     * than allowing a caller to substitute equivalent-looking DSA words. */
    if (!profile || !dungeon || !slave_location || !timer || !out_runner ||
        !out_action) {
        return 0;
    }
    memset(&resolution, 0, sizeof(resolution));
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_resolve_csbwin_falsewall_dsa_timer_action(
            profile, dungeon, slave_location, timer, &resolution)) {
        return 0;
    }
    action = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, resolution.master.dsa_id,
        resolution.state_index, resolution.action_ordinal);
    if (!action || action->column != resolution.input_column ||
        !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, &resolution.master, resolution.state_index,
            resolution.action_ordinal, resolution.master_location,
            &candidate)) {
        return 0;
    }
    *out_runner = candidate;
    *out_action = action;
    return 1;
}

int csb_v1_runtime_prepare_csbwin_stoneroom_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;

    if (!profile || !dungeon || !slave_location || !timer || !out_runner ||
        !out_action) {
        return 0;
    }
    memset(&resolution, 0, sizeof(resolution));
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_resolve_csbwin_stoneroom_dsa_timer_action(
            profile, dungeon, slave_location, timer, &resolution)) {
        return 0;
    }
    action = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, resolution.master.dsa_id,
        resolution.state_index, resolution.action_ordinal);
    if (!action || action->column != resolution.input_column ||
        !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, &resolution.master, resolution.state_index,
            resolution.action_ordinal, resolution.master_location,
            &candidate)) {
        return 0;
    }
    *out_runner = candidate;
    *out_action = action;
    return 1;
}

int csb_v1_runtime_resolve_csbwin_openroom_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    /* CSBWin Timer.cpp ProcessTT_OPENROOM (1641-1711) visits target-square
     * type-47 actuators and passes each unchanged normal timer to
     * ProcessDSATimer5. DSA.cpp ProcessDSATimer5 is only the source wrapper
     * around ProcessDSATimer6, therefore the same bounded resolver is the
     * end-to-end owner of this saved route. */
    if (!profile || !dungeon || !slave_location || !timer || !out_resolution ||
        !timer->valid || timer->truncated || timer->function != 5u ||
        timer->ubyte9 > 2u || timer->ubyte8 > 3u ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y) {
        return 0;
    }
    return csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
        profile, dungeon, slave_location, (int)timer->ubyte9,
        (int)timer->ubyte8, out_resolution);
}

int csb_v1_runtime_prepare_csbwin_openroom_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;

    if (!profile || !dungeon || !slave_location || !timer || !out_runner ||
        !out_action) {
        return 0;
    }
    memset(&resolution, 0, sizeof(resolution));
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_resolve_csbwin_openroom_dsa_timer_action(
            profile, dungeon, slave_location, timer, &resolution)) {
        return 0;
    }
    action = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, resolution.master.dsa_id,
        resolution.state_index, resolution.action_ordinal);
    if (!action || action->column != resolution.input_column ||
        !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, &resolution.master, resolution.state_index,
            resolution.action_ordinal, resolution.master_location,
            &candidate)) {
        return 0;
    }
    *out_runner = candidate;
    *out_action = action;
    return 1;
}

int csb_v1_runtime_resolve_csbwin_dessage_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    /* CSBWin CSBCode.cpp:6435 routes TT_DESSAGE (102) to
     * Timer.cpp::ProcessTT_OPENROOM. Its type-47 arm calls
     * ProcessDSATimer5 unchanged, while the source skips non-DSA objects for
     * DSA messages. The saved action/position/target fields therefore remain
     * the sole admitted input to the existing ProcessDSATimer6 receipt. */
    if (!profile || !dungeon || !slave_location || !timer || !out_resolution ||
        !timer->valid || timer->truncated || timer->function != 102u ||
        timer->ubyte9 > 2u || timer->ubyte8 > 3u ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y) {
        return 0;
    }
    return csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
        profile, dungeon, slave_location, (int)timer->ubyte9,
        (int)timer->ubyte8, out_resolution);
}

int csb_v1_runtime_prepare_csbwin_dessage_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;

    if (!profile || !dungeon || !slave_location || !timer || !out_runner ||
        !out_action) {
        return 0;
    }
    memset(&resolution, 0, sizeof(resolution));
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_resolve_csbwin_dessage_dsa_timer_action(
            profile, dungeon, slave_location, timer, &resolution)) {
        return 0;
    }
    action = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, resolution.master.dsa_id,
        resolution.state_index, resolution.action_ordinal);
    if (!action || action->column != resolution.input_column ||
        !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, &resolution.master, resolution.state_index,
            resolution.action_ordinal, resolution.master_location,
            &candidate)) {
        return 0;
    }
    *out_runner = candidate;
    *out_action = action;
    return 1;
}

int csb_v1_runtime_resolve_csbwin_door_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    /* CSBWin Timer.cpp ProcessTT_DOOR (1509-1541) rejects the source's
     * disabled action, then calls ActivateDSA. ActivateDSA (1453-1490)
     * constructs the same action/position/level/target timer consumed by
     * ProcessDSATimer5. The normal run initializes the 0/1/2 modifier map in
     * CSBCode.cpp:6403-6405, so a restored raw 0/1/2 action is the only
     * source-proven saved input accepted here. */
    if (!profile || !dungeon || !slave_location || !timer || !out_resolution ||
        !timer->valid || timer->truncated || timer->function != 10u ||
        timer->ubyte9 > 2u || timer->ubyte8 > 3u ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y) {
        return 0;
    }
    return csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
        profile, dungeon, slave_location, (int)timer->ubyte9,
        (int)timer->ubyte8, out_resolution);
}

int csb_v1_runtime_prepare_csbwin_door_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;

    if (!profile || !dungeon || !slave_location || !timer || !out_runner ||
        !out_action) {
        return 0;
    }
    memset(&resolution, 0, sizeof(resolution));
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_resolve_csbwin_door_dsa_timer_action(
            profile, dungeon, slave_location, timer, &resolution)) {
        return 0;
    }
    action = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, resolution.master.dsa_id,
        resolution.state_index, resolution.action_ordinal);
    if (!action || action->column != resolution.input_column ||
        !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, &resolution.master, resolution.state_index,
            resolution.action_ordinal, resolution.master_location,
            &candidate)) {
        return 0;
    }
    *out_runner = candidate;
    *out_action = action;
    return 1;
}

static int csb_v1_runtime_resolve_csbwin_activate_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    uint8_t source_function,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    if (!profile || !dungeon || !slave_location || !timer || !out_resolution ||
        !timer->valid || timer->truncated || timer->function != source_function ||
        timer->ubyte9 > 2u || timer->ubyte8 > 3u ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y) {
        return 0;
    }
    return csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
        profile, dungeon, slave_location, (int)timer->ubyte9,
        (int)timer->ubyte8, out_resolution);
}

static int csb_v1_runtime_prepare_csbwin_activate_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    uint8_t source_function,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;

    if (!out_runner || !out_action) return 0;
    memset(&resolution, 0, sizeof(resolution));
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_resolve_csbwin_activate_dsa_timer_action(
            profile, dungeon, slave_location, timer, source_function,
            &resolution)) {
        return 0;
    }
    action = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, resolution.master.dsa_id,
        resolution.state_index, resolution.action_ordinal);
    if (!action || action->column != resolution.input_column ||
        !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, &resolution.master, resolution.state_index,
            resolution.action_ordinal, resolution.master_location,
            &candidate)) {
        return 0;
    }
    *out_runner = candidate;
    *out_action = action;
    return 1;
}

int csb_v1_runtime_resolve_csbwin_teleporter_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    /* CSBWin Timer.cpp::ProcessTT_TELEPORTER lines 2343-2367 invokes
     * ActivateDSA first. Its exact source input is function 8 and the normal
     * per-tick timerTypeModifier 0/1/2 mapping. */
    return csb_v1_runtime_resolve_csbwin_activate_dsa_timer_action(
        profile, dungeon, slave_location, timer, 8u, out_resolution);
}

int csb_v1_runtime_prepare_csbwin_teleporter_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    return csb_v1_runtime_prepare_csbwin_activate_dsa_timer_stack_runner(
        profile, dungeon, slave_location, timer, 8u, out_runner, out_action);
}

int csb_v1_runtime_resolve_csbwin_pitroom_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution)
{
    /* CSBWin Timer.cpp::ProcessTT_PITROOM lines 2473-2508 has the identical
     * pre-mutation ActivateDSA handoff, with saved function 9. */
    return csb_v1_runtime_resolve_csbwin_activate_dsa_timer_action(
        profile, dungeon, slave_location, timer, 9u, out_resolution);
}

int csb_v1_runtime_prepare_csbwin_pitroom_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action)
{
    return csb_v1_runtime_prepare_csbwin_activate_dsa_timer_stack_runner(
        profile, dungeon, slave_location, timer, 9u, out_runner, out_action);
}

static int csb_v1_runtime_csbwin_timer_matches_saved_slot(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512TimerSummary *timer);

static int csb_v1_runtime_find_saved_timer_queue_slot(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512TimerSummary *timer,
    uint16_t *out_queue_slot);

static void csb_v1_runtime_record_saved_timer_dsa_execution(
    CSB_V1_RuntimeProfile *profile, uint16_t queue_slot,
    uint16_t timer_index, const CSB_V1_DSAImportedAction *action);

static void csb_v1_runtime_invalidate_saved_timer_dsa_execution(
    CSB_V1_RuntimeProfile *profile)
{
    if (!profile) return;
    memset(&profile->csbwin_last_dsa_execution_receipt, 0,
           sizeof(profile->csbwin_last_dsa_execution_receipt));
    profile->csbwin_last_saved_timer_dsa_valid = 0;
    profile->csbwin_last_saved_timer_dsa_queue_slot =
        CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    profile->csbwin_last_saved_timer_dsa_timer_index =
        CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    profile->csbwin_last_saved_timer_dsa_id = 0xffu;
    profile->csbwin_last_saved_timer_dsa_state_index = 0u;
    profile->csbwin_last_saved_timer_dsa_column = 0u;
    profile->csbwin_last_saved_timer_dsa_action_ordinal = -1;
}

static int csb_v1_runtime_dsa_execution_receipt_current(
    const CSB_V1_RuntimeProfile *profile)
{
    const CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 *receipt;
    uint32_t expected_tail_fnv1a;

    if (!profile) return 0;
    receipt = &profile->csbwin_last_dsa_execution_receipt;
    if (!receipt->valid) return 0;
    if (receipt->party_talents_changed) {
        int i;
        if (!profile->party_state_valid ||
            receipt->party_talents_champion_count < 0 ||
            receipt->party_talents_champion_count > profile->party_state.ChampionCount ||
            receipt->party_talents_champion_count > CSB_V1_MAX_CHAMPIONS) {
            return 0;
        }
        for (i = 0; i < receipt->party_talents_champion_count; ++i) {
            if (profile->party_state.Champions[i].Fingerprint !=
                    receipt->party_talents_fingerprints[i] ||
                profile->party_state.Champions[i].Talents !=
                    receipt->party_talents_after[i]) return 0;
        }
    }
    if (receipt->cause_poison_count != 0u) {
        const CSB_V1_Champion *champion;
        uint16_t event_index = receipt->last_cause_poison_timer_event_index;

        if (receipt->cause_poison_count != 1u ||
            !profile->party_state_valid ||
            receipt->last_cause_poison_character_selector < 0 ||
            receipt->last_cause_poison_character_selector >=
                profile->party_state.ChampionCount ||
            receipt->last_cause_poison_character_selector >=
                CSB_V1_MAX_CHAMPIONS ||
            receipt->last_cause_poison_attack <= 0 ||
            receipt->last_cause_poison_attack > 0xffff) {
            return 0;
        }
        champion = &profile->party_state.Champions[
            receipt->last_cause_poison_character_selector];
        if (champion->CurrentHealth != receipt->last_cause_poison_health_after ||
            champion->PoisonDose != receipt->last_cause_poison_dose_after ||
            champion->PoisonEventCount !=
                receipt->last_cause_poison_event_count_after) {
            return 0;
        }
        if (event_index == DM1_EVENT_MAX_COUNT) {
            if (receipt->last_cause_poison_timer_attack != 0u ||
                receipt->last_cause_poison_timer_time != 0u) return 0;
        } else {
            const struct DM1_Event_V1 *event;

            if (event_index >= DM1_EVENT_MAX_COUNT ||
                !profile->csbwin_poison_event_attack_valid[event_index] ||
                profile->csbwin_poison_event_attack[event_index] !=
                    receipt->last_cause_poison_timer_attack) {
                return 0;
            }
            event = &profile->timeline_queue.events[event_index];
            if (event->type != DM1_EVENT_POISON_CHAMPION ||
                event->priority != (uint8_t)
                    receipt->last_cause_poison_character_selector ||
                event->map_time != receipt->last_cause_poison_timer_time) {
                return 0;
            }
        }
    }
    if (receipt->text_message_changed &&
        memcmp(&profile->csbwin_text_message_receipt,
               &receipt->text_message_after,
               sizeof(receipt->text_message_after)) != 0) {
        return 0;
    }
    if (receipt->saves_disabled_changed &&
        (profile->csbwin_saves_disabled ? 1 : 0) !=
            receipt->saves_disabled_after) {
        return 0;
    }
    if (receipt->random_state_changed &&
        (!profile->csbwin_gameblock2_summary_valid ||
         profile->csbwin_random_seed != receipt->random_state_after)) {
        return 0;
    }
    if (receipt->object_property_store_count != 0u) {
        uint32_t value = 0u;

        if (receipt->last_object_property_kind >
                CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE ||
            csb_v1_runtime_dsa_get_object_property(
                (void *)profile, receipt->last_object_property_thing,
                (CSB_V1_CSBWinDSAObjectProperty)
                    receipt->last_object_property_kind,
                &value) != 1 || value != receipt->last_object_property_after) {
            return 0;
        }
    }
    if (receipt->teleporter_copy_count != 0u) {
        uint32_t source_values[5];
        uint32_t destination_values[5];

        if (!csb_v1_runtime_dsa_get_cell_info(
                (void *)profile,
                receipt->last_teleporter_copy_source_location,
                source_values) ||
            !csb_v1_runtime_dsa_get_cell_info(
                (void *)profile,
                receipt->last_teleporter_copy_destination_location,
                destination_values) ||
            memcmp(source_values, receipt->last_teleporter_copy_source_before,
                   sizeof(source_values)) != 0 ||
            memcmp(destination_values,
                   receipt->last_teleporter_copy_destination_after,
                   sizeof(destination_values)) != 0) {
            return 0;
        }
    }
    if (receipt->cell_store_count != 0u) {
        uint32_t cell_values[5];

        if (receipt->last_cell_store_write_mask == 0u ||
            !csb_v1_runtime_dsa_get_cell_info(
                (void *)profile, receipt->last_cell_store_location,
                cell_values) ||
            memcmp(cell_values, receipt->last_cell_store_after,
                   sizeof(cell_values)) != 0) {
            return 0;
        }
    }
    if (receipt->false_pit_count != 0u) {
        if (receipt->false_pit_count != 1u || receipt->cell_store_count != 1u ||
            receipt->last_false_pit_location !=
                receipt->last_cell_store_location ||
            receipt->last_false_pit_before[0] != 3u ||
            receipt->last_false_pit_after[0] != 3u ||
            receipt->last_cell_store_write_mask != (1u << 1) ||
            memcmp(receipt->last_false_pit_before,
                   receipt->last_cell_store_before,
                   sizeof(receipt->last_false_pit_before)) != 0 ||
            memcmp(receipt->last_false_pit_after,
                   receipt->last_cell_store_after,
                   sizeof(receipt->last_false_pit_after)) != 0 ||
            ((receipt->last_false_pit_before[1] & ~1u) !=
                 (receipt->last_false_pit_after[1] & ~1u)) ||
            (receipt->last_false_pit_after[1] & 1u) > 1u ||
            memcmp(receipt->last_false_pit_before + 2u,
                   receipt->last_false_pit_after + 2u,
                   3u * sizeof(uint32_t)) != 0) {
            return 0;
        }
    }
    if (receipt->monster_store_count != 0u) {
        uint32_t monster_values[8];

        if (receipt->last_monster_store_write_mask == 0u ||
            !csb_v1_runtime_dsa_get_monster_info(
                (void *)profile, receipt->last_monster_store_thing,
                monster_values) ||
            memcmp(monster_values, receipt->last_monster_store_after,
                   sizeof(monster_values)) != 0) {
            return 0;
        }
    }
    if (receipt->experience_plus_count != 0u) {
        int selector = receipt->last_experience_character_selector;
        int skill = receipt->last_experience_skill_number;
        int basic_skill = receipt->last_experience_basic_skill_number;

        if (!profile->party_state_valid || selector < 0 ||
            selector >= profile->party_state.ChampionCount ||
            selector >= CSB_V1_MAX_CHAMPIONS || skill < 0 ||
            skill >= CSB_V1_FULL_SKILL_COUNT || basic_skill < 0 ||
            basic_skill >= CSB_V1_FULL_SKILL_COUNT ||
            profile->party_state.Champions[selector].SkillExperience[skill] !=
                receipt->last_experience_selected_after ||
            profile->party_state.Champions[selector]
                .SkillExperience[basic_skill] !=
                receipt->last_experience_basic_after) {
            return 0;
        }
    }

    /* StoreExCellFlg writes the serialized EXPOOL tail.  Unlike the pure
     * stack actions, its receipt remains source-owned only while the exact
     * post-write tail is still present and matches its declared FNV. */
    if (receipt->excell_store_count == 0u &&
        receipt->wing_talents_store_count == 0u) {
        return 1;
    }
    expected_tail_fnv1a = receipt->excell_store_count != 0u ?
        receipt->excell_tail_fnv1a_after :
        receipt->wing_talents_tail_fnv1a_after;
    if (receipt->excell_store_count != 0u &&
        receipt->wing_talents_store_count != 0u &&
        receipt->excell_tail_fnv1a_after !=
            receipt->wing_talents_tail_fnv1a_after) {
        return 0;
    }
    return profile->csbwin_appended_tail_valid &&
        !profile->csbwin_appended_tail_truncated &&
        profile->csbwin_appended_tail_preserved_size > 0u &&
        profile->csbwin_appended_tail_preserved_size <=
            sizeof(profile->csbwin_appended_tail) &&
        profile->csbwin_appended_tail_fnv1a ==
            expected_tail_fnv1a &&
        profile->csbwin_appended_tail_fnv1a == csb_v1_runtime_fnv1a32(
            profile->csbwin_appended_tail,
            profile->csbwin_appended_tail_preserved_size);
}

int csb_v1_runtime_execute_csbwin_saved_timer_dsa_stack_action(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer)
{
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    const CSB_V1_DSAImportedAction *action = NULL;
    uint16_t queue_slot;
    int prepared = 0;

    if (!profile) return 0;
    csb_v1_runtime_invalidate_saved_timer_dsa_execution(profile);
    if (!dungeon || !slave_location || !timer ||
        !csb_v1_runtime_find_saved_timer_queue_slot(
            profile, timer, &queue_slot)) {
        return 0;
    }
    memset(&runner, 0, sizeof(runner));

    /* CSBWin Timer.cpp:1453-1491 constructs a new TIMER and a
     * NEWDSAPARAMETERS object before ProcessDSATimer5.  The latter sets the
     * source parameter count to zero.  These four saved timer kinds are the
     * only routes here whose source parameter surface is fully proven.  The
     * existing runner admits only authenticated pure-stack actions, so this
     * does not commit a DSA master state, mutate a cell, or run an unbounded
     * ProcessDSATimer6 program. */
    switch (timer->function) {
    case 102u: /* TT_DESSAGE -> ProcessTT_OPENROOM -> ProcessDSATimer5. */
        prepared = csb_v1_runtime_prepare_csbwin_dessage_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 10u: /* TT_DOOR -> ActivateDSA -> ProcessDSATimer5. */
        prepared = csb_v1_runtime_prepare_csbwin_door_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 8u: /* TT_TELEPORTER -> ActivateDSA -> ProcessDSATimer5. */
        prepared = csb_v1_runtime_prepare_csbwin_teleporter_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 9u: /* TT_PITROOM -> ActivateDSA -> ProcessDSATimer5. */
        prepared = csb_v1_runtime_prepare_csbwin_pitroom_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    default:
        return 0;
    }
    if (!prepared || !action) {
        return 0;
    }
    /* ProcessTimers owns this transient SET/CLEAR/TOGGLE map.  It starts
     * with the source defaults for each restored dispatch; AMPERSAND2 may
     * replace it only while this exact runner is active. */
    runner.timer_type_modifiers_valid = 1;
    runner.timer_type_modifiers[0] = 0u;
    runner.timer_type_modifiers[1] = 1u;
    runner.timer_type_modifiers[2] = 2u;
    if (!csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
            profile, &runner, action, NULL, 0, NULL)) {
        return 0;
    }
    csb_v1_runtime_record_saved_timer_dsa_execution(
        profile, queue_slot, timer->source_index, action);
    return 1;
}

static int csb_v1_runtime_csbwin_timer_matches_saved_slot(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512TimerSummary *timer)
{
    const CSB_V1_CSBWin512TimerSummary *saved;

    if (!profile || !timer || timer->source_index >=
            profile->csbwin_timer_summary_count ||
        timer->source_index >= CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES) {
        return 0;
    }
    saved = &profile->csbwin_timers[timer->source_index];
    return saved->valid && !saved->truncated &&
        saved->source_index == timer->source_index &&
        saved->time == timer->time && saved->function == timer->function &&
        saved->ubyte5 == timer->ubyte5 && saved->ubyte6 == timer->ubyte6 &&
        saved->ubyte7 == timer->ubyte7 && saved->ubyte8 == timer->ubyte8 &&
        saved->ubyte9 == timer->ubyte9 && saved->sequence == timer->sequence &&
        saved->level == timer->level;
}

static int csb_v1_runtime_find_saved_timer_queue_slot(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512TimerSummary *timer,
    uint16_t *out_queue_slot)
{
    uint16_t found_queue_slot = CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    size_t i;

    /* CSBWin SaveGame.cpp restores TIMER and m_timerQueue as one state.
     * A value-shaped TIMER from a caller is not enough to enter
     * ProcessDSATimer5: it must retain one unique serialized queue owner. */
    if (!profile || !timer || !out_queue_slot ||
        !csb_v1_runtime_csbwin_timer_pool_counts_valid(profile) ||
        !csb_v1_runtime_csbwin_timer_matches_saved_slot(profile, timer)) {
        return 0;
    }
    for (i = 0u; i < profile->csbwin_timer_queue_summary_count; ++i) {
        if (profile->csbwin_timer_queue[i] == timer->source_index) {
            if (found_queue_slot != CSB_V1_CSBWIN_TIMER_QUEUE_NONE) {
                return 0;
            }
            found_queue_slot = (uint16_t)i;
        }
    }
    if (found_queue_slot == CSB_V1_CSBWIN_TIMER_QUEUE_NONE) return 0;
    *out_queue_slot = found_queue_slot;
    return 1;
}

static void csb_v1_runtime_record_saved_timer_dsa_execution(
    CSB_V1_RuntimeProfile *profile, uint16_t queue_slot,
    uint16_t timer_index, const CSB_V1_DSAImportedAction *action)
{
    int action_ordinal;

    if (!profile || !action) return;
    if (!csb_v1_runtime_dsa_execution_receipt_current(profile)) {
        csb_v1_runtime_invalidate_saved_timer_dsa_execution(profile);
        return;
    }
    for (action_ordinal = 0;
         action_ordinal < profile->csbwin_extended_dsa_state.imported_action_count;
         ++action_ordinal) {
        if (&profile->csbwin_extended_dsa_state.imported_actions[action_ordinal] ==
            action) {
            break;
        }
    }
    if (action_ordinal ==
        profile->csbwin_extended_dsa_state.imported_action_count) {
        return;
    }
    profile->csbwin_last_saved_timer_dsa_valid = 1;
    profile->csbwin_last_saved_timer_dsa_queue_slot = queue_slot;
    profile->csbwin_last_saved_timer_dsa_timer_index = timer_index;
    profile->csbwin_last_saved_timer_dsa_id = action->dsa_id;
    profile->csbwin_last_saved_timer_dsa_state_index = action->state_index;
    profile->csbwin_last_saved_timer_dsa_column = action->column;
    profile->csbwin_last_saved_timer_dsa_action_ordinal = action_ordinal;

    /* ModifyMessage is scoped to this ProcessTimers invocation.  Its map is
     * useful only when the exact saved TIMER/queue/action triple still owns
     * the just-completed execution receipt. */
    if (profile->csbwin_last_dsa_execution_receipt.valid &&
        profile->csbwin_last_dsa_execution_receipt.timer_type_modifiers_valid &&
        queue_slot < profile->csbwin_timer_queue_summary_count &&
        timer_index < profile->csbwin_timer_summary_count &&
        profile->csbwin_timer_queue[queue_slot] == timer_index &&
        profile->csbwin_timers[timer_index].valid &&
        !profile->csbwin_timers[timer_index].truncated &&
        profile->csbwin_timers[timer_index].source_index == timer_index &&
        profile->csbwin_last_dsa_execution_receipt.dsa_id == action->dsa_id &&
        profile->csbwin_last_dsa_execution_receipt.state_index ==
            action->state_index &&
        profile->csbwin_last_dsa_execution_receipt.column == action->column &&
        profile->csbwin_last_dsa_execution_receipt.action_ordinal ==
            action_ordinal) {
        const CSB_V1_CSBWin512TimerSummary *timer =
            &profile->csbwin_timers[timer_index];

        profile->csbwin_last_dsa_execution_receipt.saved_timer_scope_valid = 1;
        profile->csbwin_last_dsa_execution_receipt.saved_timer_queue_slot =
            queue_slot;
        profile->csbwin_last_dsa_execution_receipt.saved_timer_index =
            timer_index;
        profile->csbwin_last_dsa_execution_receipt.saved_timer_function =
            timer->function;
        profile->csbwin_last_dsa_execution_receipt.saved_timer_action =
            timer->ubyte9;
        profile->csbwin_last_dsa_execution_receipt.saved_timer_position =
            timer->ubyte8;
        profile->csbwin_last_dsa_execution_receipt.saved_timer_time =
            timer->time;
    }
}

int csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer)
{
    CSB_V1_CSBWin512TimerSummary dispatched;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    const CSB_V1_DSAImportedAction *action = NULL;
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;
    uint32_t record_id;
    int parameters[26];
    size_t parameter_count;
    size_t i;
    int square_type;
    int prepared;

    /* CSBWin CSBCode.cpp ProcessTimers:6436-6454 gives a parameter message
     * its allocated timer index, selects STONEROOM only for roomSTONE, and
     * otherwise invokes OPENROOM. Timer.cpp's two handlers read the exact
     * EDT_MessageParameters record before entering ProcessDSATimer[56]. */
    if (!profile) return 0;
    csb_v1_runtime_invalidate_saved_timer_dsa_execution(profile);
    if (!dungeon || !slave_location || !timer ||
        !timer->valid || timer->truncated || timer->function != 101u ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y ||
        !csb_v1_runtime_csbwin_timer_matches_saved_slot(profile, timer)) {
        return 0;
    }
    record_id = (1u << 24) | timer->source_index;
    if (!csb_v1_runtime_locate_appended_expool_record_internal(
            profile, record_id, &payload, &payload_size) || !payload ||
        (payload_size & 3u) != 0u || payload_size / 4u > 26u) {
        return 0;
    }
    parameter_count = payload_size / 4u;
    for (i = 0u; i < parameter_count; ++i) {
        parameters[i] = (int)csb_v1_runtime_read_le32(payload + i * 4u);
    }

    square_type = csb_v1_dungeon_get_square_type(
        dungeon, slave_location->level, slave_location->x,
        slave_location->y);
    if (square_type < 0) return 0;
    dispatched = *timer;
    /* Timer.cpp ProcessTT_STONEROOM changes the timer function after Read;
     * ProcessTT_OPENROOM keeps 101. Both ProcessDSATimer paths consume the
     * action/position bytes, so normalize only for Firestaff's exact receipt
     * resolvers instead of admitting a caller-selected timer family. */
    dispatched.function = square_type == 0 ? 6u : 5u;
    memset(&runner, 0, sizeof(runner));
    if (square_type == 0) {
        prepared = csb_v1_runtime_prepare_csbwin_stoneroom_dsa_timer_stack_runner(
            profile, dungeon, slave_location, &dispatched, &runner, &action);
    } else {
        prepared = csb_v1_runtime_prepare_csbwin_openroom_dsa_timer_stack_runner(
            profile, dungeon, slave_location, &dispatched, &runner, &action);
    }
    if (!prepared || !action) {
        return 0;
    }
    runner.timer_type_modifiers_valid = 1;
    runner.timer_type_modifiers[0] = 0u;
    runner.timer_type_modifiers[1] = 1u;
    runner.timer_type_modifiers[2] = 2u;
    if (!csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
            profile, &runner, action, parameters, (int)parameter_count, NULL)) {
        return 0;
    }
    for (i = 0u; i < profile->csbwin_timer_queue_summary_count; ++i) {
        if (profile->csbwin_timer_queue[i] == timer->source_index) {
            csb_v1_runtime_record_saved_timer_dsa_execution(
                profile, (uint16_t)i, timer->source_index, action);
            break;
        }
    }
    return 1;
}

int csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    uint16_t queue_index)
{
    const CSB_V1_CSBWin512TimerSummary *timer;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    const CSB_V1_DSAImportedAction *action = NULL;
    uint16_t timer_index;
    int prepared;

    /* SaveGame.cpp restores both serialized arrays before ProcessTimers
     * consumes m_timerQueue. Do not allow a caller-built TIMER shape to
     * stand in for either authenticated saved record. */
    if (!profile) return 0;
    csb_v1_runtime_invalidate_saved_timer_dsa_execution(profile);
    if (!dungeon || !slave_location ||
        !csb_v1_runtime_csbwin_timer_pool_counts_valid(profile) ||
        queue_index >= profile->csbwin_timer_queue_summary_count) {
        return 0;
    }
    timer_index = profile->csbwin_timer_queue[queue_index];
    if (timer_index >= profile->csbwin_timer_summary_count) return 0;
    timer = &profile->csbwin_timers[timer_index];
    if (!timer->valid || timer->truncated ||
        timer->source_index != timer_index ||
        timer->level != (uint8_t)slave_location->level ||
        timer->ubyte6 != (uint8_t)slave_location->x ||
        timer->ubyte7 != (uint8_t)slave_location->y) {
        return 0;
    }
    memset(&runner, 0, sizeof(runner));
    switch (timer->function) {
    case 5u:
        prepared = csb_v1_runtime_prepare_csbwin_openroom_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 6u:
        prepared = csb_v1_runtime_prepare_csbwin_stoneroom_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 7u:
        prepared = csb_v1_runtime_prepare_csbwin_falsewall_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 8u:
        /* CSBWin Timer.cpp ProcessTT_TELEPORTER enters ActivateDSA before
         * changing the teleporter cell flag or wiggling occupants. */
        prepared = csb_v1_runtime_prepare_csbwin_teleporter_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 9u:
        /* CSBWin Timer.cpp ProcessTT_PITROOM enters ActivateDSA before
         * mutating the pit cell flag or wiggling occupants. */
        prepared = csb_v1_runtime_prepare_csbwin_pitroom_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 10u:
        /* CSBWin Timer.cpp ProcessTT_DOOR enters ActivateDSA before it
         * changes function to TT_1 and requeues a door timer. */
        prepared = csb_v1_runtime_prepare_csbwin_door_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    case 102u:
        /* CSBWin ProcessTimers routes TT_DESSAGE to ProcessTT_OPENROOM,
         * which gives ProcessDSATimer5 a zero-parameter DSA message. */
        prepared = csb_v1_runtime_prepare_csbwin_dessage_dsa_timer_stack_runner(
            profile, dungeon, slave_location, timer, &runner, &action);
        break;
    default:
        return 0;
    }
    if (!prepared || !action) {
        return 0;
    }
    runner.timer_type_modifiers_valid = 1;
    runner.timer_type_modifiers[0] = 0u;
    runner.timer_type_modifiers[1] = 1u;
    runner.timer_type_modifiers[2] = 2u;
    if (!csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
            profile, &runner, action, NULL, 0, NULL)) {
        return 0;
    }
    csb_v1_runtime_record_saved_timer_dsa_execution(
        profile, queue_index, timer_index, action);
    return 1;
}

int csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 receipt;
    const CSB_V1_ChaosMagicState *state;
    uint8_t seen_slots[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES] = { 0 };
    int mapped_index_entry = 0;
    int mapped_runtime_action = 0;
    int action_index;
    int level;
    int selector;
    int event_ordinal;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.last_dsa_id = 0xffu;
    receipt.last_action_ordinal = -1;
    receipt.last_queue_slot = CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    receipt.last_timer_index = CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    *out_receipt = receipt;

    if (!profile || !profile->csbwin_body_runtime_summary_valid ||
        !profile->csbwin_extended_features_valid ||
        !profile->csbwin_extended_level_index_present ||
        !csb_v1_runtime_csbwin_timer_pool_counts_valid(profile)) {
        return 0;
    }
    if (profile->csbwin_last_saved_timer_dsa_valid &&
        !csb_v1_runtime_dsa_execution_receipt_current(profile)) {
        return 0;
    }
    state = &profile->csbwin_extended_dsa_state;
    if (!state->imported_actions || state->imported_action_count <= 0) {
        return 0;
    }
    receipt.imported_action_count = state->imported_action_count;
    for (action_index = 0; action_index < state->imported_action_count;
         ++action_index) {
        const CSB_V1_DSAImportedAction *action =
            &state->imported_actions[action_index];

        if (!state->imported_headers[action->dsa_id].valid ||
            (action->program_word_count > 0 && !action->program_words)) {
            return 0;
        }
    }
    receipt.dsa_catalog_valid = 1;

    for (level = 0; level < 64; ++level) {
        for (selector = 0; selector < 32; ++selector) {
            uint16_t dsa_id =
                profile->csbwin_extended_level_dsa_index[level][selector];

            if (dsa_id == 0xffffu) continue;
            ++mapped_index_entry;
            if (dsa_id < CSB_V1_MAX_DSA_SCRIPTS &&
                state->imported_headers[dsa_id].valid) {
                for (action_index = 0;
                     action_index < state->imported_action_count;
                     ++action_index) {
                    if (state->imported_actions[action_index].dsa_id ==
                        (uint8_t)dsa_id) {
                        ++mapped_runtime_action;
                        break;
                    }
                }
            }
        }
    }
    if (mapped_index_entry == 0 || mapped_runtime_action == 0) {
        return 0;
    }
    receipt.level_index_valid = 1;

    if (profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount > DM1_EVENT_MAX_COUNT) {
        return 0;
    }
    for (event_ordinal = 0;
         event_ordinal < profile->timeline_queue.eventCount;
         ++event_ordinal) {
        int event_index = profile->timeline_queue.timeline[event_ordinal];
        uint16_t queue_slot;
        uint16_t timer_index;
        const CSB_V1_CSBWin512TimerSummary *timer;
        const struct DM1_Event_V1 *event;

        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) return 0;
        queue_slot = profile->csbwin_timeline_event_queue_slot[event_index];
        if (queue_slot == CSB_V1_CSBWIN_TIMER_QUEUE_NONE) {
            continue;
        }
        if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
            seen_slots[queue_slot]) {
            return 0;
        }
        timer_index = profile->csbwin_timer_queue[queue_slot];
        if (timer_index >= profile->csbwin_timer_summary_count) return 0;
        timer = &profile->csbwin_timers[timer_index];
        event = &profile->timeline_queue.events[event_index];
        if (!timer->valid || timer->truncated ||
            timer->source_index != timer_index ||
            event->map_time != timer->time ||
            event->type != timer->function ||
            event->priority != timer->ubyte5 ||
            event->b_mapX != timer->ubyte6 ||
            event->b_mapY != timer->ubyte7 ||
            event->c_cell != timer->ubyte8 ||
            event->c_effect != timer->ubyte9) {
            return 0;
        }
        seen_slots[queue_slot] = 1u;
    }
    receipt.timer_queue_event_chain_valid = 1;
    receipt.live_timer_event_count =
        (uint16_t)profile->timeline_queue.eventCount;

    if (profile->csbwin_last_saved_timer_dsa_valid) {
        const CSB_V1_DSAImportedAction *action;
        uint16_t queue_slot = profile->csbwin_last_saved_timer_dsa_queue_slot;
        uint16_t timer_index = profile->csbwin_last_saved_timer_dsa_timer_index;
        int ordinal = profile->csbwin_last_saved_timer_dsa_action_ordinal;

        if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
            timer_index >= profile->csbwin_timer_summary_count ||
            profile->csbwin_timer_queue[queue_slot] != timer_index ||
            ordinal < 0 || ordinal >= state->imported_action_count) {
            return 0;
        }
        action = &state->imported_actions[ordinal];
        if (!state->imported_headers[action->dsa_id].valid ||
            action->dsa_id != profile->csbwin_last_saved_timer_dsa_id ||
            action->state_index !=
                profile->csbwin_last_saved_timer_dsa_state_index ||
            action->column != profile->csbwin_last_saved_timer_dsa_column) {
            return 0;
        }
        receipt.saved_timer_dsa_execution_valid = 1;
        receipt.last_dsa_id = action->dsa_id;
        receipt.last_state_index = action->state_index;
        receipt.last_input_column = action->column;
        receipt.last_action_ordinal = ordinal;
        receipt.last_queue_slot = queue_slot;
        receipt.last_timer_index = timer_index;
    }

    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 *out_receipt)
{
    if (!profile || !out_receipt ||
        !csb_v1_runtime_dsa_execution_receipt_current(profile)) {
        return 0;
    }
    *out_receipt = profile->csbwin_last_dsa_execution_receipt;
    return 1;
}

static int csb_v1_runtime_dispatch_saved_csbwin_timer_dsa(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    uint16_t queue_slot)
{
    const CSB_V1_DungeonData *dungeon;
    CSB_V1_CSBWin512TimerSummary *timer;
    uint16_t timer_index;
    int thing;
    int guard = 0;
    int dsa_count = 0;
    int dsa_success_count = 0;

    /* CSBWin CSBCode.cpp ProcessTimers gets a TIMER from m_timerQueue, then
     * Timer.cpp walks that exact square's objects. Keep the Firestaff bridge
     * tied to the materialized queue slot and the dispatched event fields;
     * no public API can provide a substitute TIMER or actuator location. */
    if (!profile || !record || queue_slot == CSB_V1_CSBWIN_TIMER_QUEUE_NONE ||
        queue_slot >= profile->csbwin_timer_queue_summary_count ||
        !profile->csbwin_body_runtime_summary_valid) {
        return 0;
    }
    timer_index = profile->csbwin_timer_queue[queue_slot];
    /* The event arrived through a materialized CSBWin TimerQueue slot.  If
     * its live summary receipt has since become invalid, it is still not an
     * M10 event: consuming it here prevents a numeric timer-function alias
     * from reaching a generic mutation path with incomplete CSBWin state. */
    if (timer_index >= profile->csbwin_timer_summary_count) return 1;
    timer = &profile->csbwin_timers[timer_index];
    if (timer->function == DM1_EVENT_WATCHDOG) {
        struct DM1_Event_V1 next;
        CSB_V1_CSBWin512TimerSummary successor;
        uint32_t next_time;
        int event_index;

        /* CSBWin CSBCode.cpp:6504 delegates TT_53 to Timer.cpp:2770-2782.
         * SetWatchdogTimer creates a zero-payload level-zero TT_53 exactly
         * 300 game ticks after the current d.Time. Timer.cpp DeleteTimer
         * removes the consumed handle, then SetTimer assigns the successor
         * to the first real TT_EMPTY slot. A separately invented M10 C53
         * event would lose original-save ownership and cannot round-trip. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 &&
            profile->game_time <= 0x00fffed3u) {
            next_time = profile->game_time + 300u;
            memset(&next, 0, sizeof(next));
            next.map_time = DM1_MAP_TIME_MAKE(0u, next_time);
            next.type = DM1_EVENT_WATCHDOG;
            event_index = csb_v1_runtime_add_timeline_event(profile, &next);
            if (event_index >= 0 && event_index < DM1_EVENT_MAX_COUNT) {
                successor = *timer;
                successor.time = next_time;
                successor.level = 0u;
                successor.ubyte5 = 0u;
                successor.ubyte6 = 0u;
                successor.ubyte7 = 0u;
                successor.ubyte8 = 0u;
                successor.ubyte9 = 0u;
                if (!csb_v1_runtime_replace_dispatched_csbwin_timer(
                        profile, queue_slot, timer_index, &successor,
                        event_index)) {
                    (void)dm1v1_event_delete(&profile->timeline_queue,
                                              event_index);
                }
            }
        }
        /* TT_53 aliases the M10 watchdog classification. A malformed saved
         * timer cannot be allowed to enter a generic watchdog path. */
        return 1;
    }
    if (timer->function == 13u) {
        CSB_V1_Champion *champion;
        int champion_index;
        int maximum_health;

        /* CSBWin CSBCode.cpp:6469 dispatches TT_ViAltar to Timer.cpp:
         * 2663-2763. Retain only ProcessTT_ViAltar's final packedState()==0
         * step, which calls Character.cpp:804-825 BringCharacterToLife.
         * State 2 needs CreateCloud. State 1 has both the CSBWin
         * EDT_ChampionBones fingerprint path and the old-save DB10 fallback.
         * The source stores packed position in timerUByte5 bits 2..3. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 &&
            (timer->ubyte5 & 3u) == 1u &&
            profile->csbwin_appended_tail_valid && profile->dungeon_handle &&
            timer->time < 0x00ffffffu) {
            const uint16_t bones_thing = (uint16_t)timer->ubyte8 |
                ((uint16_t)timer->ubyte9 << 8);
            const uint32_t record_id = (9u << 24) | (uint32_t)bones_thing;
            const uint8_t *payload = NULL;
            size_t payload_size = 0u;
            uint16_t fingerprint;
            uint8_t *bones_record;
            struct DM1_Event_V1 next;
            int bones_type;
            int bones_size;
            int event_index;

            /* CSBWin Timer.cpp:2692-2741 uses EXPOOL::GetChampionBonesRecord
             * for current saves. It consumes only a one-word record, matches
             * its low-16 fingerprint to a dead party champion, then removes
             * the exact DB10 bones object and queues the state-zero successor.
             * data.cpp EXPOOL::Read removes that record, so use the existing
             * authenticated Read-shaped delete rather than retaining stale
             * save-tail ownership after the bones leave the dungeon. */
            bones_record = csb_v1_runtime_mutable_thing_record(
                profile->dungeon_handle, bones_thing, &bones_type, &bones_size);
            if (bones_record && bones_type == 10 && bones_size >= 4 &&
                ((bones_thing >> 14) & 3u) == ((timer->ubyte5 >> 2) & 3u) &&
                csb_v1_runtime_square_contains_thing(profile->dungeon_handle,
                                                       bones_thing,
                                                       timer->level,
                                                       timer->ubyte6,
                                                       timer->ubyte7) &&
                csb_v1_runtime_locate_appended_expool_record_internal(
                    profile, record_id, &payload, &payload_size) &&
                payload && payload_size == sizeof(uint32_t)) {
                fingerprint = (uint16_t)csb_v1_runtime_read_le32(payload);
                for (champion_index = 0;
                     champion_index < profile->party_state.ChampionCount;
                     ++champion_index) {
                    if (profile->party_state.Champions[champion_index].Fingerprint ==
                        fingerprint) {
                        break;
                    }
                }
                if (champion_index < profile->party_state.ChampionCount &&
                    profile->party_state.Champions[champion_index].CurrentHealth ==
                        0) {
                    memset(&next, 0, sizeof(next));
                    next.map_time = DM1_MAP_TIME_MAKE(timer->level,
                                                       timer->time + 1u);
                    next.type = timer->function;
                    next.priority = (uint8_t)(champion_index << 2);
                    next.b_mapX = timer->ubyte6;
                    next.b_mapY = timer->ubyte7;
                    next.c_cell = timer->ubyte8;
                    next.c_effect = timer->ubyte9;
                    event_index = csb_v1_runtime_add_timeline_event(profile,
                                                                      &next);
                    if (event_index >= 0 && event_index < DM1_EVENT_MAX_COUNT &&
                        csb_v1_runtime_replace_appended_expool_record_internal(
                            profile, record_id, NULL, 0u) &&
                        csb_v1_runtime_unlink_thing_from_square(
                            profile->dungeon_handle, bones_thing, timer->level,
                            timer->ubyte6, timer->ubyte7)) {
                        csb_v1_runtime_write_u16(bones_record, 0xfffeu);
                        csb_v1_runtime_write_u16(bones_record + 2, 0u);
                        profile->csbwin_appended_tail_fnv1a =
                            csb_v1_runtime_fnv1a32(
                                profile->csbwin_appended_tail,
                                profile->csbwin_appended_tail_preserved_size);
                        timer->time += 1u;
                        timer->ubyte5 = (uint8_t)(champion_index << 2);
                        profile->csbwin_timeline_event_queue_slot[event_index] =
                            queue_slot;
                    }
                }
            }
            return 1;
        }
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 &&
            (timer->ubyte5 & 3u) == 1u && !profile->csbwin_appended_tail_valid &&
            profile->dungeon_handle && timer->time < 0x00ffffffu) {
            uint16_t bones_thing = (uint16_t)timer->ubyte8 |
                ((uint16_t)timer->ubyte9 << 8);
            uint16_t word2;
            uint8_t *bones_record;
            struct DM1_Event_V1 next;
            int bones_type;
            int bones_size;
            int event_index;

            /* CSBWin Timer.cpp:2692-2751 old-save arm uses DB10::value as
             * its champion ordinal only after it finds this exact bones RN
             * on the saved square and at the packed source position. */
            bones_record = csb_v1_runtime_mutable_thing_record(
                profile->dungeon_handle, bones_thing, &bones_type, &bones_size);
            if (bones_record && bones_type == 10 && bones_size >= 4 &&
                ((bones_thing >> 14) & 3u) == ((timer->ubyte5 >> 2) & 3u) &&
                csb_v1_runtime_square_contains_thing(profile->dungeon_handle,
                                                       bones_thing,
                                                       timer->level,
                                                       timer->ubyte6,
                                                       timer->ubyte7)) {
                word2 = csb_v1_runtime_read_u16(bones_record + 2);
                champion_index = (int)((word2 >> 14) & 3u);
                if ((word2 & 0x007fu) == 5u && profile->party_state_valid &&
                    profile->champion_count > 0 &&
                    profile->champion_count <= CSB_V1_MAX_CHAMPIONS &&
                    profile->party_state.ChampionCount == profile->champion_count &&
                    champion_index < profile->party_state.ChampionCount &&
                    profile->party_state.Champions[champion_index].CurrentHealth ==
                        0) {
                    memset(&next, 0, sizeof(next));
                    next.map_time = DM1_MAP_TIME_MAKE(timer->level,
                                                       timer->time + 1u);
                    next.type = timer->function;
                    next.priority = (uint8_t)(champion_index << 2);
                    next.b_mapX = timer->ubyte6;
                    next.b_mapY = timer->ubyte7;
                    next.c_cell = timer->ubyte8;
                    next.c_effect = timer->ubyte9;
                    event_index = csb_v1_runtime_add_timeline_event(profile,
                                                                      &next);
                    if (event_index >= 0 && event_index < DM1_EVENT_MAX_COUNT &&
                        csb_v1_runtime_unlink_thing_from_square(
                            profile->dungeon_handle, bones_thing, timer->level,
                            timer->ubyte6, timer->ubyte7)) {
                        /* DB10::Clear sets both link=RNeof and word2=0. */
                        csb_v1_runtime_write_u16(bones_record, 0xfffeu);
                        csb_v1_runtime_write_u16(bones_record + 2, 0u);
                        timer->time += 1u;
                        timer->ubyte5 = (uint8_t)(champion_index << 2);
                        profile->csbwin_timeline_event_queue_slot[event_index] =
                            queue_slot;
                    }
                }
            }
            return 1;
        }
        champion_index = (int)((timer->ubyte5 >> 2) & 3u);
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 &&
            (timer->ubyte5 & 3u) == 0u && profile->party_state_valid &&
            profile->champion_count > 0 &&
            profile->champion_count <= CSB_V1_MAX_CHAMPIONS &&
            profile->party_state.ChampionCount == profile->champion_count &&
            champion_index < profile->party_state.ChampionCount) {
            champion = &profile->party_state.Champions[champion_index];
            maximum_health = champion->MaximumHealth;
            if (champion->CurrentHealth == 0 && maximum_health > 0) {
                maximum_health -= maximum_health / 64 + 1;
                if (maximum_health < 25) maximum_health = 25;
                champion->MaximumHealth = (int16_t)maximum_health;
                champion->CurrentHealth = (int16_t)(maximum_health / 2);
                champion->Direction = (uint8_t)(profile->party_dir & 3);
                champion->Attributes &=
                    ~(uint16_t)CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
            }
        }
        /* TT_13 overlaps the generic M10 Vi Altar event. Even a malformed
         * restored receipt remains source-owned and cannot enter that route. */
        return 1;
    }
    if (timer->function == 65u) {
        CSB_V1_DungeonData *saved_dungeon;
        uint16_t generator_thing;
        uint16_t first_disabled = 0xffffu;
        int thing;
        int guard = 0;

        /* CSBWin CSBCode.cpp:6473-6475 dispatches TT_ReactivateGenerator
         * to Timer.cpp:1788-1836. The original tries timerObj8 first, then
         * keeps an explicit old-save fallback to the first type-zero DB3
         * actuator on the saved square. ubyte8/ubyte9 are that preserved
         * little-endian timerObj8 word, not generic cell/effect input. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && profile->dungeon_handle) {
            saved_dungeon = profile->dungeon_handle;
            generator_thing = (uint16_t)timer->ubyte8 |
                ((uint16_t)timer->ubyte9 << 8);
            thing = csb_v1_dungeon_get_first_thing(
                saved_dungeon, record->mapIndex, record->mapX, record->mapY);
            while (thing >= 0 && thing != 0xfffe && thing != 0xffff &&
                   guard++ < 128) {
                uint8_t *actuator;
                int type;
                int size;
                uint16_t type_data;

                actuator = csb_v1_runtime_mutable_thing_record(
                    saved_dungeon, (uint16_t)thing, &type, &size);
                if (!actuator) break;
                if (type == CSB_V1_THING_TYPE_ACTUATOR && size >= 4) {
                    type_data = csb_v1_runtime_read_u16(actuator + 2);
                    if ((type_data & 0x007fu) == 0u) {
                        if (first_disabled == 0xffffu) {
                            first_disabled = (uint16_t)thing;
                        }
                        if ((uint16_t)thing == generator_thing) {
                            first_disabled = (uint16_t)thing;
                            break;
                        }
                    }
                }
                thing = csb_v1_runtime_sensor_next_thing(
                    saved_dungeon, (uint16_t)thing);
            }
            if (first_disabled != 0xffffu) {
                uint8_t *actuator;
                int type;
                int size;
                uint16_t type_data;

                actuator = csb_v1_runtime_mutable_thing_record(
                    saved_dungeon, first_disabled, &type, &size);
                if (actuator && type == CSB_V1_THING_TYPE_ACTUATOR &&
                    size >= 4) {
                    type_data = csb_v1_runtime_read_u16(actuator + 2);
                    if ((type_data & 0x007fu) == 0u) {
                        /* Timer.cpp's actuatorTypeOR(6) preserves every
                         * source bit except the disabled type-zero state. */
                        csb_v1_runtime_write_u16(
                            actuator + 2, (uint16_t)(type_data | 6u));
                    }
                }
            }
        }
        /* C65 aliases generic M10 generator handling. Always consume an
         * imported TT_65 here so malformed saved data cannot reach that
         * broader first-disabled-sensor route without CSBWin identity proof. */
        return 1;
    }
    if (timer->function == 79u) {
        /* CSBWin CSBCode.cpp:6563 and Magic.cpp:1329-1339 expire TT_79 by
         * removing exactly one active Magic Footprints effect. The complete
         * source TIMER identity is still required even though its payload is
         * unused: byte fields must not become a synthetic footprint route.
         * The footprint-record cleanup/draw route has no restored owner, so
         * this bridge owns only the saved counter decrement. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 &&
            profile->csbwin_character_tail_magic_footprints_active > 0u) {
            --profile->csbwin_character_tail_magic_footprints_active;
        }
        return 1;
    }
    if (timer->function == 78u) {
        int16_t shield_delta;

        /* CSBWin CSBCode.cpp:6560-6562 and Attack.cpp:1208-1227 expire
         * TT_78 by subtracting its signed timerWord6 from FireShield. Keep
         * the original little-endian word and full materialized queue/event
         * identity together; malformed, negative, or underflowing records
         * cannot create a replacement shield state. MarkAllPortraitsChanged
         * remains unavailable without the source HUD owner. */
        shield_delta = (int16_t)((uint16_t)timer->ubyte6 |
            ((uint16_t)timer->ubyte7 << 8));
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && shield_delta > 0 &&
            profile->csbwin_character_tail_fire_shield >= shield_delta) {
            profile->csbwin_character_tail_fire_shield = (int16_t)(
                profile->csbwin_character_tail_fire_shield - shield_delta);
        }
        return 1;
    }
    if (timer->function == 75u) {
        uint16_t poison_attack;
        int champion_index;

        /* CSBWin CSBCode.cpp:6545-6555 dispatches TT_75 through
         * PoisonCharacter; ReDMCSB TIMELINE.C F0261 lines 1991-1993 then
         * decrements PoisonEventCount before CHAMPION.C F0322 applies the
         * Attack damage and source +36 requeue. The restored TIMER owns its
         * champion priority and little-endian Attack word. The runtime C75
         * continuation currently retains only an 8-bit attack, so reject a
         * wider saved word rather than truncate it into a synthetic chain. */
        poison_attack = (uint16_t)timer->ubyte6 |
            ((uint16_t)timer->ubyte7 << 8);
        champion_index = (int)timer->ubyte5;
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && profile->party_state_valid &&
            profile->champion_count > 0 &&
            profile->champion_count <= CSB_V1_MAX_CHAMPIONS &&
            profile->party_state.ChampionCount == profile->champion_count &&
            champion_index < profile->party_state.ChampionCount &&
            poison_attack > 0u &&
            profile->party_state.Champions[champion_index].PoisonEventCount >
                0u) {
            --profile->party_state.Champions[champion_index].PoisonEventCount;
            csb_v1_runtime_apply_poison_attack_to_champion(
                profile, champion_index, (int)poison_attack);
        }
        /* C75 remains source-owned even when malformed, preventing a future
         * generic event path from treating timerUByte9 as its Attack word. */
        return 1;
    }
    if (timer->function == 77u) {
        int16_t shield_delta;

        /* CSBWin CSBCode.cpp:6556-6558 and ReDMCSB TIMELINE.C F0261 lines
         * 1985-1989 expire C77 by subtracting B.Defense from SpellShield.
         * The restored TIMER owns that little-endian signed word and the
         * restored character tail owns the signed defense total. Require a
         * complete queue/timer/event identity, a positive source defense, and
         * a non-underflowing tail value; no source HUD redraw is inferred. */
        shield_delta = (int16_t)((uint16_t)timer->ubyte6 |
            ((uint16_t)timer->ubyte7 << 8));
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && shield_delta > 0 &&
            profile->csbwin_character_tail_spell_shield >= shield_delta) {
            profile->csbwin_character_tail_spell_shield = (int16_t)(
                profile->csbwin_character_tail_spell_shield - shield_delta);
        }
        /* C77 remains source-owned even when malformed, preventing a future
         * generic event path from bypassing saved CSBWin identity checks. */
        return 1;
    }
    if (timer->function == 74u) {
        int16_t shield_delta;

        /* CSBWin CSBCode.cpp:6541-6543 and ReDMCSB TIMELINE.C F0261 lines
         * 1975-1984 expire C74 by subtracting B.Defense from PartyShield.
         * The restored TIMER owns that little-endian signed word and the
         * restored character tail owns the signed defense total. Require a
         * complete queue/timer/event identity, a positive source defense, and
         * a non-underflowing tail value; no source HUD redraw is inferred. */
        shield_delta = (int16_t)((uint16_t)timer->ubyte6 |
            ((uint16_t)timer->ubyte7 << 8));
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && shield_delta > 0 &&
            profile->csbwin_character_tail_party_shield >= shield_delta) {
            profile->csbwin_character_tail_party_shield = (int16_t)(
                profile->csbwin_character_tail_party_shield - shield_delta);
        }
        /* C74 remains source-owned even when malformed, preventing a future
         * generic event path from bypassing saved CSBWin identity checks. */
        return 1;
    }
    if (timer->function == 73u) {
        /* CSBWin CSBCode.cpp:6540 and ReDMCSB TIMELINE.C F0261 lines
         * 1972-1974 expire C73 by decrementing the party's Thieves' Eye
         * count. The restored character tail owns that count. Require the
         * complete queue/timer/event identity and a positive count so a stale
         * or malformed saved timer cannot underflow runtime visibility state. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 &&
            profile->csbwin_character_tail_see_thru_walls > 0u) {
            --profile->csbwin_character_tail_see_thru_walls;
        }
        /* C73 remains source-owned even when malformed, preventing a future
         * generic event path from bypassing saved CSBWin identity checks. */
        return 1;
    }
    if (timer->function == 72u) {
        CSB_V1_Champion *champion;
        uint16_t shield_delta;

        /* CSBWin CSBCode.cpp:6511-6538 and ReDMCSB TIMELINE.C F0261 lines
         * 1966-1971 expire C72 from its champion priority and B.Defense
         * word. The restored TIMER keeps that union as ubyte5 and little-
         * endian ubyte6..7, while CHARDESC owns ShieldStrength. Do not admit
         * a stale record, an absent champion, or an underflowing decrement;
         * the latter cannot be proven as a live saved shield contribution.
         * Source status-panel redraw remains blocked without its M11 owner. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && profile->party_state_valid &&
            profile->champion_count > 0 &&
            profile->champion_count <= CSB_V1_MAX_CHAMPIONS &&
            profile->party_state.ChampionCount == profile->champion_count &&
            timer->ubyte5 < (uint8_t)profile->party_state.ChampionCount &&
            timer->ubyte5 < CSB_V1_MAX_CHAMPIONS) {
            champion = &profile->party_state.Champions[timer->ubyte5];
            shield_delta = (uint16_t)timer->ubyte6 |
                ((uint16_t)timer->ubyte7 << 8);
            if (shield_delta <= champion->ShieldStrength) {
                champion->ShieldStrength =
                    (uint16_t)(champion->ShieldStrength - shield_delta);
            }
        }
        /* C72 is source-owned even if malformed, so generic timeline code
         * cannot gain a future champion-shield mutation without this receipt. */
        return 1;
    }
    if (timer->function == 71u) {
        /* CSBWin CSBCode.cpp:6510 and ReDMCSB TIMELINE.C F0261 lines
         * 1953-1965 expire C71 by decrementing the party invisibility count.
         * This saved body already owns that exact count. Require a complete
         * queue/timer/event identity and a positive count rather than allowing
         * an untrusted or stale record to underflow it. The source redraw
         * branch depends on an inventory-champion UI owner not present in this
         * restored runtime profile, so no HUD work is inferred here. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 &&
            profile->csbwin_character_tail_invisible > 0u) {
            --profile->csbwin_character_tail_invisible;
        }
        /* Keep every restored function-71 receipt source-owned so it cannot
         * acquire a future generic event path without saved identity checks. */
        return 1;
    }
    if (timer->function == 24u) {
        CSB_V1_DungeonData *saved_dungeon;
        uint16_t timer_object;
        uint8_t *object_record;
        int object_size;

        /* CSBWin CSBCode.cpp:6490-6502 removes timerObj8 from its saved
         * square, then writes RNfree to its common record. Preserve that
         * exact two-part ownership only when the complete timer receipt and
         * original Thing chain both validate. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && profile->dungeon_handle) {
            timer_object = (uint16_t)timer->ubyte8 |
                ((uint16_t)timer->ubyte9 << 8);
            if (timer_object == 0xfffeu || timer_object == 0xffffu) return 1;
            saved_dungeon = profile->dungeon_handle;
            object_record = csb_v1_runtime_mutable_thing_record(
                saved_dungeon, timer_object, NULL, &object_size);
            if (!object_record || object_size < 2) return 1;
            if (csb_v1_runtime_unlink_thing_from_square(
                    saved_dungeon, timer_object, record->mapIndex,
                    record->mapX, record->mapY)) {
                csb_v1_runtime_write_u16(object_record, 0xffffu);
            }
        }
        /* TT_24 has no generic timeline equivalent with this object lifetime.
         * Invalid receipts remain source-owned and cannot fall through. */
        return 1;
    }
    if (timer->function == DM1_EVENT_CPSE) {
        /* CSBWin CSBCode.cpp:6564-6569 documents TT_22 as a timer restored
         * with a saved game whose original handler is intentionally a no-op.
         * Consume it only when its imported TIMER and materialized event are
         * still the same queue-owned record; C22 must not become a synthetic
         * generic runtime action. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5) {
            return 1;
        }
        return 0;
    }
    if (timer->function == 1u) {
        uint8_t *square;
        int square_type;
        int door_state;
        int next_door_state;
        int thing;
        int guard = 0;
        int material_group_occupies = 0;
        struct DM1_Event_V1 next;
        CSB_V1_CSBWin512TimerSummary successor;
        int event_index;

        /* CSBWin CSBCode.cpp:6429 dispatches TT_1 to Timer.cpp:1224-1341.
         * Retain ProcessTT_1's collision-free state step and its exact
         * +1-tick successor through Timer.cpp's DeleteTimer/SetTimer pool
         * transaction. Party
         * damage, material-group damage, and QueueSound need source state
         * not present in this profile, so those shapes remain fail-closed. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && timer->ubyte9 <= 1u) {
            square = csb_v1_runtime_square_byte_ptr(
                profile, record->mapIndex, record->mapX, record->mapY,
                &square_type);
            if (!square || square_type != 4) return 1;
            door_state = *square & 0x07u;
            if (door_state == 5) return 1;
            if ((timer->ubyte9 == 0u && door_state == 0) ||
                (timer->ubyte9 == 1u && door_state == 4)) {
                return 1;
            }

            /* Timer.cpp probes the first group after its party route. A
             * material group takes the damage/reaction path, which must not
             * be replaced by a synthetic door animation. */
            if (!profile->dungeon_handle) return 1;
            if (timer->ubyte9 == 1u && door_state != 0 &&
                profile->current_level == record->mapIndex &&
                profile->party_x == record->mapX &&
                profile->party_y == record->mapY) {
                return 1;
            }
            thing = csb_v1_dungeon_get_first_thing(
                profile->dungeon_handle, record->mapIndex, record->mapX,
                record->mapY);
            while (thing >= 0 && thing != 0xfffe && thing != 0xffff &&
                   guard++ < 128) {
                const uint8_t *thing_record;
                int type;
                int size;

                thing_record = csb_v1_dungeon_get_thing_record(
                    profile->dungeon_handle, (uint16_t)thing, &type, NULL,
                    &size);
                if (!thing_record || size < 2) return 1;
                if (type == 4) {
                    const struct CreatureBehaviorProfile_Compat *creature;

                    if (size <= 4) return 1;
                    creature = CREATURE_GetProfile_Compat((int)thing_record[4]);
                    if (!creature) return 1;
                    material_group_occupies =
                        (creature->attributes & CREATURE_ATTR_MASK_NON_MATERIAL)
                            == 0;
                    break;
                }
                thing = csb_v1_runtime_sensor_next_thing(
                    profile->dungeon_handle, (uint16_t)thing);
            }
            if (guard >= 128 || material_group_occupies) return 1;

            next_door_state =
                door_state + (timer->ubyte9 == 0u ? -1 : 1);
            if ((timer->ubyte9 == 0u && next_door_state == 0) ||
                (timer->ubyte9 == 1u && next_door_state == 4)) {
                *square = (uint8_t)((*square & (uint8_t)~0x07u) |
                                    (uint8_t)next_door_state);
                return 1;
            }
            /* The materialized M10 dispatch runs after its source timer's
             * saved tick. Preserve the existing runtime bridge's live clock
             * boundary while still making SetTimer own the replacement slot. */
            if (profile->game_time >= 0x00ffffffu) return 1;
            memset(&next, 0, sizeof(next));
            next.map_time = DM1_MAP_TIME_MAKE(
                timer->level, profile->game_time + 1u);
            next.type = timer->function;
            next.priority = timer->ubyte5;
            next.b_mapX = timer->ubyte6;
            next.b_mapY = timer->ubyte7;
            next.c_cell = timer->ubyte8;
            next.c_effect = timer->ubyte9;
            event_index = csb_v1_runtime_add_timeline_event(profile, &next);
            if (event_index >= 0 && event_index < DM1_EVENT_MAX_COUNT) {
                successor = *timer;
                successor.time = profile->game_time + 1u;
                if (csb_v1_runtime_replace_dispatched_csbwin_timer(
                        profile, queue_slot, timer_index, &successor,
                        event_index)) {
                    *square = (uint8_t)((*square & (uint8_t)~0x07u) |
                                        (uint8_t)next_door_state);
                } else {
                    (void)dm1v1_event_delete(&profile->timeline_queue,
                                              event_index);
                }
            }
        }
        /* TT_1 aliases shared door animation. Invalid and collision-owned
         * saved receipts must never reach that generic mutation path. */
        return 1;
    }
    if (timer->function == 5u) {
        uint8_t *text_record;
        int thing;
        int thing_type;
        int thing_size;
        uint16_t text_word;
        uint16_t next_text_word;
        int target_is_party_square;
        CSB_V1_RuntimeTextMessageReceipt message_receipt;

        /* CSBWin Timer.cpp::ProcessTT_OPENROOM:1641-1711 changes DB2::show
         * with timerTypeModifier[0..2]. If a newly visible text is under the
         * party, it enters QuePrintLines.  Admit that narrow source path only
         * for a sole authenticated DB2 record and retain its F0168 bytes as
         * a runtime receipt; DSA and mixed lists remain outside this owner. */
        if (!timer->valid || timer->truncated ||
            timer->source_index != timer_index ||
            record->eventType != timer->function ||
            record->mapIndex != timer->level ||
            record->mapX != timer->ubyte6 || record->mapY != timer->ubyte7 ||
            record->cell != timer->ubyte8 || record->effect != timer->ubyte9 ||
            record->aux0 != timer->ubyte5 || timer->ubyte9 > 2u ||
            !profile->dungeon_handle) {
            return 1;
        }
        thing = csb_v1_dungeon_get_first_thing(
            profile->dungeon_handle, timer->level, timer->ubyte6,
            timer->ubyte7);
        if (thing < 0 || thing == 0xfffe || thing == 0xffff) return 1;
        text_record = csb_v1_runtime_mutable_thing_record(
            profile->dungeon_handle, (uint16_t)thing, &thing_type, &thing_size);
        if (!text_record) return 1;
        if (thing_type == CSB_THING_TYPE_TEXTSTRING) {
            if (thing_size < 4 ||
                csb_v1_runtime_read_u16(text_record) != 0xfffeu) {
                return 1;
            }
            text_word = csb_v1_runtime_read_u16(text_record + 2);
            next_text_word = text_word;
            if (timer->ubyte9 == 0u) next_text_word |= 0x0001u;
            else if (timer->ubyte9 == 1u) {
                next_text_word &= (uint16_t)~0x0001u;
            } else next_text_word ^= 0x0001u;
            target_is_party_square = timer->level == profile->current_level &&
                profile->party_x == timer->ubyte6 &&
                profile->party_y == timer->ubyte7;
            memset(&message_receipt, 0, sizeof(message_receipt));
            if (target_is_party_square && !(text_word & 0x0001u) &&
                (next_text_word & 0x0001u)) {
                (void)csb_v1_runtime_stage_openroom_text_message(
                    profile, (uint16_t)thing, next_text_word,
                    &message_receipt);
            }
            csb_v1_runtime_write_u16(text_record + 2, next_text_word);
            if (message_receipt.valid) {
                profile->csbwin_text_message_receipt = message_receipt;
            } else if (target_is_party_square &&
                       profile->csbwin_text_message_receipt.valid &&
                       profile->csbwin_text_message_receipt.text_thing ==
                           (uint16_t)thing && !(next_text_word & 0x0001u)) {
                memset(&profile->csbwin_text_message_receipt, 0,
                       sizeof(profile->csbwin_text_message_receipt));
            }
            return 1;
        }
    }
    if (timer->function == 6u) {
        uint8_t *text_record;
        int thing;
        int thing_type;
        int thing_size;
        uint16_t text_word;

        /* CSBWin Timer.cpp::ProcessTT_STONEROOM:2118-2175 updates only the
         * matching-position DB2::show bit before its actuator/endgame arms.
         * Retain a sole DB2 target with the original position encoded in its
         * Thing handle. Any DSA/mixed list needs separate ownership and must
         * not acquire generic timeline behavior as a substitute. */
        if (!timer->valid || timer->truncated ||
            timer->source_index != timer_index ||
            record->eventType != timer->function ||
            record->mapIndex != timer->level ||
            record->mapX != timer->ubyte6 || record->mapY != timer->ubyte7 ||
            record->cell != timer->ubyte8 || record->effect != timer->ubyte9 ||
            record->aux0 != timer->ubyte5 || timer->ubyte9 > 2u ||
            !profile->dungeon_handle) {
            return 1;
        }
        thing = csb_v1_dungeon_get_first_thing(
            profile->dungeon_handle, timer->level, timer->ubyte6,
            timer->ubyte7);
        if (thing < 0 || thing == 0xfffe || thing == 0xffff) return 1;
        text_record = csb_v1_runtime_mutable_thing_record(
            profile->dungeon_handle, (uint16_t)thing, &thing_type, &thing_size);
        if (!text_record) return 1;
        if (thing_type == CSB_THING_TYPE_TEXTSTRING) {
            if (thing_size < 4 ||
                ((uint16_t)thing >> 14) != timer->ubyte8 ||
                csb_v1_runtime_read_u16(text_record) != 0xfffeu) {
                return 1;
            }
            text_word = csb_v1_runtime_read_u16(text_record + 2);
            if (timer->ubyte9 == 0u) text_word |= 0x0001u;
            else if (timer->ubyte9 == 1u) text_word &= (uint16_t)~0x0001u;
            else text_word ^= 0x0001u;
            csb_v1_runtime_write_u16(text_record + 2, text_word);
            return 1;
        }
    }
    if (timer->function == 8u || timer->function == 9u) {
        uint8_t *square;
        int square_type;
        int expected_square_type;
        int thing;
        int action;

        /* CSBWin Timer.cpp::ProcessTT_TELEPORTER:2343-2367 and
         * ProcessTT_PITROOM:2473-2505 run ActivateDSA, then update bit 3 and
         * call WiggleEverything when a closed target opens. An empty Thing
         * chain has neither a type-47 owner nor a party/monster/drawable Thing
         * for WiggleEverything to move. Retain only that complete no-op-wiggle
         * shape; a listed Thing falls through to the DSA receipt and remains
         * mutation-blocked below. */
        if (!timer->valid || timer->truncated ||
            timer->source_index != timer_index ||
            record->eventType != timer->function ||
            record->mapIndex != timer->level ||
            record->mapX != timer->ubyte6 || record->mapY != timer->ubyte7 ||
            record->cell != timer->ubyte8 || record->effect != timer->ubyte9 ||
            record->aux0 != timer->ubyte5 || timer->ubyte9 > 2u ||
            !profile->dungeon_handle) {
            return 1;
        }
        expected_square_type = timer->function == 8u ? 5 : 2;
        square = csb_v1_runtime_square_byte_ptr(
            profile, timer->level, timer->ubyte6, timer->ubyte7, &square_type);
        if (!square || square_type != expected_square_type) return 1;
        thing = csb_v1_dungeon_get_first_thing(
            profile->dungeon_handle, timer->level, timer->ubyte6,
            timer->ubyte7);
        if (thing < 0 || thing == 0xfffe || thing == 0xffff) {
            /* The saved runtime profile has no independent party-level
             * field. Block the ambiguous same-square shape rather than infer
             * that CSBWin's party wiggle was a no-op. */
            if (profile->party_x == timer->ubyte6 &&
                profile->party_y == timer->ubyte7) {
                return 1;
            }
            action = (int)timer->ubyte9;
            if (action == 2) action = (*square & 0x08u) ? 1 : 0;
            if (action == 0) *square |= 0x08u;
            else *square &= (uint8_t)~0x08u;
            return 1;
        }
    }
    if (timer->function == 12u) {
        /* ReDMCSB TIMELINE.C F0254 lines 1614-1637 and CSBWin
         * CSBCode.cpp:6468/Timer.cpp:2644-2664 consume TT_12 through its
         * champion priority, first clearing HideDamageReceivedEventIndex.
         * The source's two redraw branches depend on the live inventory
         * champion ordinal, which this restored CSBWin profile does not own;
         * do not infer it or fabricate a HUD redraw. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && profile->party_state_valid &&
            profile->champion_count > 0 &&
            profile->champion_count <= CSB_V1_MAX_CHAMPIONS &&
            profile->party_state.ChampionCount == profile->champion_count &&
            timer->ubyte5 < (uint8_t)profile->party_state.ChampionCount &&
            timer->ubyte5 < CSB_V1_MAX_CHAMPIONS) {
            profile->party_state.Champions[timer->ubyte5]
                .HideDamageReceivedEventIndex = -1;
        }
        /* TT_12 aliases the shared hide-damage event. Keep every restored
         * function-12 receipt out of any generic path, including malformed
         * identities, until an exact inventory redraw surface is available. */
        return 1;
    }
    if (timer->function == 11u) {
        CSB_V1_Champion *champion;

        /* ReDMCSB TIMELINE.C F0253 lines 1574-1612 and CSBWin
         * CSBCode.cpp:6457-6466/Timer.cpp:2591-2642 own TT_11. Retain only
         * the saved no-rearm, non-SHOOT receipt: F0253 clears the action
         * lock, clears the stored action defense, and resets ActionIndex.
         * Ammunition selection and CSBWin's TAG0115ee branch have no complete
         * save-owned inventory handoff here, so they remain fail-closed. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && timer->ubyte6 == 0u &&
            profile->party_state_valid &&
            profile->champion_count > 0 &&
            profile->champion_count <= CSB_V1_MAX_CHAMPIONS &&
            profile->party_state.ChampionCount == profile->champion_count &&
            timer->ubyte5 < (uint8_t)profile->party_state.ChampionCount &&
            timer->ubyte5 < CSB_V1_MAX_CHAMPIONS) {
            champion = &profile->party_state.Champions[timer->ubyte5];
            if (champion->ActionIndex != 32u) { /* CSBWin atk_SHOOT. */
                champion->EnableActionEventIndex = -1;
                champion->Attributes &= (uint16_t)~0x0008u;
                champion->CsbWinWord64 = 0;
                champion->ActionIndex = CSB_V1_ACTION_NONE;
            }
        }
        /* Function 11 aliases the shared action-enable event. A malformed
         * restored TT_11 must not reach that generic path on a later pass. */
        return 1;
    }
    if (timer->function == 2u) {
        uint8_t *square;
        int square_type;

        /* CSBWin CSBCode.cpp:6431 dispatches TT_BASH_DOOR directly to
         * Timer.cpp:1445-1451. Its function value aliases Firestaff's shared
         * door-destruction timeline event, so consume every saved function-2
         * receipt here even when it is malformed; otherwise the generic
         * timeline path could mutate a door without saved CSBWin identity. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5) {
            square = csb_v1_runtime_square_byte_ptr(
                profile, record->mapIndex, record->mapX, record->mapY,
                &square_type);
            /* ProcessTT_BASH_DOOR sets the source door's low three cell bits
             * to five. A restored receipt may alter only a loaded byte-map
             * door square; all other target shapes fail closed. */
            if (square && square_type == 4) {
                *square = (uint8_t)((*square & (uint8_t)~0x07u) | 5u);
            }
        }
        return 1;
    }
    if (timer->function == 1u) {
        /* TT_1 is the source-owned continuation installed by ProcessTT_DOOR.
         * It must never use the shared C01 scheduler, because every later
         * animation step has to retain this TIMER's original queue receipt. */
        (void)csb_v1_runtime_apply_saved_csbwin_door_animation_timer(
            profile, record, timer, timer_index, queue_slot);
        return 1;
    }
    if (timer->function == 7u) {
        uint8_t *square;
        int square_type;

        /* CSBWin Timer.cpp:1343-1442 runs the saved DSA receipt before
         * applying timerTypeModifier[0]'s unconditional falsewall SET.
         * Keep only that complete bit update here; CLEAR/TOGGLE need the
         * source party/nonmaterial-group deferral and successor ownership. */
        if (timer->valid && !timer->truncated &&
            timer->source_index == timer_index &&
            record->eventType == timer->function &&
            record->mapIndex == timer->level &&
            record->mapX == timer->ubyte6 && record->mapY == timer->ubyte7 &&
            record->cell == timer->ubyte8 && record->effect == timer->ubyte9 &&
            record->aux0 == timer->ubyte5 && timer->ubyte9 == 0u &&
            profile->dungeon_handle) {
            square = csb_v1_runtime_square_byte_ptr(
                profile, record->mapIndex, record->mapX, record->mapY,
                &square_type);
            if (!square || square_type < 0) return 0;
            *square |= 0x04u;
        }
    }
    if (!profile->dungeon_handle) return 1;
    if (!timer->valid || timer->truncated || timer->source_index != timer_index ||
        ((timer->function < 5u || timer->function > 10u) &&
         timer->function != 101u && timer->function != 102u) ||
        record->eventType != timer->function ||
        record->mapIndex != timer->level || record->mapX != timer->ubyte6 ||
        record->mapY != timer->ubyte7 || record->cell != timer->ubyte8 ||
        record->effect != timer->ubyte9 || record->aux0 != timer->ubyte5) {
        return 1;
    }
    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon, record->mapIndex, record->mapX, record->mapY);
    while (thing >= 0 && thing != 0xfffe && thing != 0xffff && guard++ < 1024) {
        const uint8_t *thing_record;
        int type;
        int size;
        CSB_V1_DSAFilterLocation location;

        thing_record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, &type, NULL, &size);
        if (!thing_record || size < 2) {
            return (timer->function == 5u || timer->function == 6u ||
                    timer->function == 8u || timer->function == 9u) ? 1 : 0;
        }
        if (type == CSB_V1_THING_TYPE_ACTUATOR && size >= 4 &&
            (((uint16_t)thing_record[2] | ((uint16_t)thing_record[3] << 8)) &
             0x007fu) == CSB_V1_DSA_FILTER_ACTUATOR_TYPE) {
            memset(&location, 0, sizeof(location));
            location.level = record->mapIndex;
            location.x = record->mapX;
            location.y = record->mapY;
            location.position = (thing >> 14) & 3;
            location.actuator_thing = (uint16_t)thing;
            ++dsa_count;
            if (timer->function == 101u) {
                if (csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
                        profile, dungeon, &location, timer)) {
                    ++dsa_success_count;
                }
            } else {
                if (csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
                        profile, dungeon, &location, queue_slot)) {
                    ++dsa_success_count;
                }
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    if (timer->function == 10u) {
        uint8_t *square;
        int square_type;
        int door_state;
        int door_action;
        struct DM1_Event_V1 next;
        CSB_V1_CSBWin512TimerSummary successor;
        int event_index;

        /* CSBWin Timer.cpp::ProcessTT_DOOR:1509-1540 invokes every DB3
         * type-47 through ActivateDSA before it examines the door and turns
         * the same TIMER into TT_1. A single successful authenticated
         * pure-stack action has no world mutation surface in Firestaff, so
         * it may retain that exact handoff. Multiple or failed DSA actions
         * remain blocked rather than guessing their combined world effects. */
        if (dsa_count != 0 && (dsa_count != 1 || dsa_success_count != 1)) {
            return 1;
        }
        square = csb_v1_runtime_square_byte_ptr(
            profile, timer->level, timer->ubyte6, timer->ubyte7, &square_type);
        if (!square || square_type != 4) return 1;
        door_state = *square & 0x07u;
        if (door_state == 5) return 1;
        door_action = (int)timer->ubyte9;
        if (door_action == 2) door_action = door_state == 0 ? 1 : 0;
        if ((door_action == 0 && door_state == 0) ||
            (door_action == 1 && door_state == 4)) {
            return 1;
        }
        memset(&next, 0, sizeof(next));
        next.map_time = DM1_MAP_TIME_MAKE(timer->level, timer->time);
        next.type = 1u;
        next.priority = timer->ubyte5;
        next.b_mapX = timer->ubyte6;
        next.b_mapY = timer->ubyte7;
        next.c_cell = timer->ubyte8;
        next.c_effect = (uint8_t)door_action;
        event_index = csb_v1_runtime_add_timeline_event(profile, &next);
        if (event_index >= 0 && event_index < DM1_EVENT_MAX_COUNT) {
            successor = *timer;
            successor.function = 1u;
            successor.ubyte9 = (uint8_t)door_action;
            if (!csb_v1_runtime_replace_dispatched_csbwin_timer(
                    profile, queue_slot, timer_index, &successor,
                    event_index)) {
                (void)dm1v1_event_delete(&profile->timeline_queue,
                                          event_index);
            }
        }
        return 1;
    }
    if (timer->function == 7u &&
        (timer->ubyte9 == 1u || timer->ubyte9 == 2u)) {
        (void)csb_v1_runtime_dispatch_saved_csbwin_falsewall_clear(
            profile, record, timer, timer_index, queue_slot);
        return 1;
    }
    if (timer->function == 5u || timer->function == 6u ||
        timer->function == 8u || timer->function == 9u) {
        /* A listed target can have DSA, HUD, or WiggleEverything ownership
         * absent from this restored profile. Any pure-stack DSA receipt above
         * may run, but it cannot fall through to M10's generic mutation. */
        return 1;
    }
    /* A source-owned queue receipt must be consumed even when its exact
     * CSBWin handler has no proven mutation path. It cannot become a generic
     * M10 timer just because validation failed after resume. */
    return 1;
}

static int csb_v1_runtime_dispatch_saved_csbwin_falsewall_clear(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    CSB_V1_CSBWin512TimerSummary *timer,
    uint16_t timer_index,
    uint16_t queue_slot)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *square;
    int square_type;
    int thing;
    int guard;
    int defer = 0;
    struct DM1_Event_V1 next;
    CSB_V1_CSBWin512TimerSummary successor;
    int event_index;

    /* CSBWin Timer.cpp ProcessTT_FALSEWALL:1343-1442 clears bit 0x04 only
     * after DSA/portrait processing. With neither owner on the saved square,
     * timerTypeModifier[1/2] remains the canonical CLEAR/TOGGLE mapping
     * established by CSBCode.cpp ProcessTimers:6403-6405. */
    if (!profile || !record || !timer || !profile->dungeon_handle ||
        timer->function != 7u ||
        (timer->ubyte9 != 1u && timer->ubyte9 != 2u) ||
        !timer->valid || timer->truncated ||
        timer->source_index != timer_index ||
        record->eventType != timer->function ||
        record->mapIndex != timer->level || record->mapX != timer->ubyte6 ||
        record->mapY != timer->ubyte7 || record->cell != timer->ubyte8 ||
        record->effect != timer->ubyte9 || record->aux0 != timer->ubyte5) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    square = csb_v1_runtime_square_byte_ptr(
        profile, record->mapIndex, record->mapX, record->mapY, &square_type);
    if (!square || square_type < 0) return 0;

    /* The source visits DSA type-47 and matching portrait type-127 objects
     * before reading timerTypeModifier. Their effects are not represented by
     * this cell-only bridge, so an exact clear is unavailable in that case. */
    thing = csb_v1_dungeon_get_first_thing(
        dungeon, record->mapIndex, record->mapX, record->mapY);
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        const uint8_t *thing_record;
        int type;
        int size;

        thing_record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, &type, NULL, &size);
        if (!thing_record || size < 2) return 0;
        if (type == CSB_V1_THING_TYPE_ACTUATOR && size >= 4) {
            uint16_t actuator_data = (uint16_t)thing_record[2] |
                ((uint16_t)thing_record[3] << 8);
            uint16_t actuator_type = actuator_data & 0x007fu;

            if (actuator_type == CSB_V1_DSA_FILTER_ACTUATOR_TYPE ||
                (actuator_type == 127u &&
                 ((uint16_t)thing >> 14) == timer->ubyte8)) {
                return 0;
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    if (guard >= 128) return 0;

    /* ProcessTT_FALSEWALL resolves TOGGLE from the current bit before the
     * CLEAR deferral branch. A closed falsewall therefore takes the exact
     * unconditional SET arm; an open one shares the original CLEAR path. */
    if (timer->ubyte9 == 2u && (*square & 0x04u) == 0u) {
        *square |= 0x04u;
        return 1;
    }

    if (profile->current_level == timer->level &&
        profile->party_x == timer->ubyte6 && profile->party_y == timer->ubyte7) {
        defer = 1;
    } else {
        thing = csb_v1_dungeon_get_first_thing(
            dungeon, record->mapIndex, record->mapX, record->mapY);
        for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
             ++guard) {
            const uint8_t *thing_record;
            int type;
            int size;

            thing_record = csb_v1_dungeon_get_thing_record(
                dungeon, (uint16_t)thing, &type, NULL, &size);
            if (!thing_record || size < 2) return 0;
            if (type == CSB_V1_THING_TYPE_GROUP) {
                const struct CreatureBehaviorProfile_Compat *creature;

                if (size < 16) return 0;
                creature = CREATURE_GetProfile_Compat((int)thing_record[4]);
                if (!creature) return 0;
                defer = (creature->attributes &
                         CREATURE_ATTR_MASK_NON_MATERIAL) != 0;
                break;
            }
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
        }
        if (guard >= 128) return 0;
    }
    if (!defer) {
        *square &= (uint8_t)~0x04u;
        return 1;
    }
    if (timer->time >= 0x00ffffffu) return 0;
    memset(&next, 0, sizeof(next));
    next.map_time = DM1_MAP_TIME_MAKE(timer->level, timer->time + 1u);
    next.type = timer->function;
    next.priority = timer->ubyte5;
    next.b_mapX = timer->ubyte6;
    next.b_mapY = timer->ubyte7;
    next.c_cell = timer->ubyte8;
    next.c_effect = timer->ubyte9;
    event_index = csb_v1_runtime_add_timeline_event(profile, &next);
    if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) return 0;
    successor = *timer;
    successor.time = timer->time + 1u;
    if (!csb_v1_runtime_replace_dispatched_csbwin_timer(
            profile, queue_slot, timer_index, &successor, event_index)) {
        (void)dm1v1_event_delete(&profile->timeline_queue, event_index);
        return 0;
    }
    return 1;
}

static int csb_v1_runtime_pre_dispatch_saved_csbwin_generator_timer(
    CSB_V1_RuntimeProfile *profile,
    uint16_t event_index,
    uint16_t queue_slot)
{
    const CSB_V1_DungeonData *dungeon;
    const struct DM1_Event_V1 *event;
    CSB_V1_CSBWin512TimerSummary *timer;
    const uint8_t *group_record;
    uint16_t timer_index;
    uint16_t group_thing;
    int thing_type;
    int thing_size;
    struct DM1_Event_V1 next;
    CSB_V1_CSBWin512TimerSummary successor;
    int successor_index;

    /* CSBWin CSBCode.cpp:6471-6472 dispatches the TIMER directly to
     * Timer.cpp ProcessTimer60and61:2519-2584. This must run before M10
     * classifies C60/C61 as a generic group move, because timerObj8 is a
     * CSBWin Thing handle, not M10's cell/effect payload. */
    if (!profile || event_index >= DM1_EVENT_MAX_COUNT ||
        queue_slot == CSB_V1_CSBWIN_TIMER_QUEUE_NONE ||
        queue_slot >= profile->csbwin_timer_queue_summary_count ||
        !profile->csbwin_body_runtime_summary_valid || !profile->dungeon_handle) {
        return 0;
    }
    timer_index = profile->csbwin_timer_queue[queue_slot];
    if (timer_index >= profile->csbwin_timer_summary_count) return 0;
    timer = &profile->csbwin_timers[timer_index];
    event = &profile->timeline_queue.events[event_index];
    if (timer->function != 60u && timer->function != 61u) {
        return 0;
    }

    /* timerObj8 is a CSBWin Thing handle, whereas the shared M10 C60/C61
     * handler treats the same bytes as generic event payload. Once a live
     * queue slot names a saved TT_60/TT_61, every unsupported source shape
     * must be consumed here. Only the narrow, fully authenticated branch
     * below may requeue it; no rejected receipt may fall through and acquire
     * a different M10 group mutation. */
    if (!timer->valid || timer->truncated ||
        timer->source_index != timer_index ||
        event->type != timer->function ||
        DM1_MAP_TIME_MAP(event->map_time) != timer->level ||
        DM1_MAP_TIME_TIME(event->map_time) != timer->time ||
        event->priority != timer->ubyte5 || event->b_mapX != timer->ubyte6 ||
        event->b_mapY != timer->ubyte7 || event->c_cell != timer->ubyte8 ||
        event->c_effect != timer->ubyte9) {
        return 1;
    }

    /* ProcessTimer60and61 only reaches this deterministic requeue when the
     * target is the party square and the exact DB4 object is not Lord Chaos.
     * Moving the object, the sound branch, occupied-square detection, and
     * Lord Chaos random detour require source state not retained here. */
    if (profile->current_level != timer->level ||
        profile->party_x != timer->ubyte6 || profile->party_y != timer->ubyte7) {
        return 1;
    }
    dungeon = profile->dungeon_handle;
    group_thing = (uint16_t)timer->ubyte8 | ((uint16_t)timer->ubyte9 << 8);
    group_record = csb_v1_dungeon_get_thing_record(
        dungeon, group_thing, &thing_type, NULL, &thing_size);
    if (!group_record || thing_type != CSB_V1_THING_TYPE_GROUP ||
        thing_size < 16 || group_record[4] == 0x17u ||
        timer->time > 0x00fffffau) {
        return 1;
    }

    memset(&next, 0, sizeof(next));
    next.map_time = DM1_MAP_TIME_MAKE(timer->level, timer->time + 5u);
    next.type = timer->function;
    next.priority = timer->ubyte5;
    next.b_mapX = timer->ubyte6;
    next.b_mapY = timer->ubyte7;
    next.c_cell = timer->ubyte8;
    next.c_effect = timer->ubyte9;
    successor_index = csb_v1_runtime_add_timeline_event(profile, &next);
    if (successor_index < 0 || successor_index >= DM1_EVENT_MAX_COUNT) return 1;

    successor = *timer;
    successor.time = timer->time + 5u;
    if (!csb_v1_runtime_replace_dispatched_csbwin_timer(
            profile, queue_slot, timer_index, &successor, successor_index)) {
        (void)dm1v1_event_delete(&profile->timeline_queue, successor_index);
    }
    return 1;
}

int csb_v1_runtime_resolve_csbwin_attack_filter_stack_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    CSB_V1_RuntimeDSAFilterBinding *out_binding,
    uint32_t *out_state_index,
    int *out_action_ordinal,
    uint32_t *out_master_location)
{
    CSB_V1_DSAFilterLocation location;
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;

    if (!profile || !dungeon || !out_binding || !out_state_index ||
        !out_action_ordinal || !out_master_location ||
        !profile->csbwin_extended_features_valid) return 0;
    memset(&location, 0, sizeof(location));
    memset(&resolution, 0, sizeof(resolution));
    if (!csb_v1_dungeon_resolve_dsa_filter_location(
            dungeon, 0, 0, &location)) return 0;
    /* Monster.cpp:1154-1166 builds timer function/position 0/0.  DSA.cpp
     * ProcessDSATimer6 derives inputMsgType = 3 * position + function. */
    if (!csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
            profile, dungeon, &location, 0, 0, &resolution)) return 0;
    *out_binding = resolution.master;
    *out_state_index = resolution.state_index;
    *out_action_ordinal = resolution.action_ordinal;
    *out_master_location = resolution.master_location;
    return 1;
}

static int csb_v1_runtime_execute_csbwin_special_filter_action(
    CSB_V1_RuntimeProfile *profile, uint8_t special_location,
    int timer_function, int *parameters, int parameter_count)
{
    CSB_V1_DSAFilterLocation location;
    CSB_V1_RuntimeCSBWinDSATimer6Resolution resolution;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    const CSB_V1_DSAImportedAction *action;
    int flags[2] = { 0, 0 };

    if (!profile || !profile->dungeon_handle || !parameters ||
        parameter_count < 1 || parameter_count > 26 ||
        timer_function < 0 || timer_function > 2 ||
        !profile->csbwin_extended_features_valid) {
        return 0;
    }
    memset(&location, 0, sizeof(location));
    memset(&resolution, 0, sizeof(resolution));
    memset(&runner, 0, sizeof(runner));
    if (!csb_v1_dungeon_resolve_dsa_special_location(
            profile->dungeon_handle, special_location,
            &location) ||
        !csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
            profile, profile->dungeon_handle, &location, timer_function, 0,
            &resolution) ||
        !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, &resolution.master, resolution.state_index,
            resolution.action_ordinal, resolution.master_location, &runner)) {
        return 0;
    }
    action = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, resolution.master.dsa_id,
        resolution.state_index, resolution.action_ordinal);
    if (!action) return 0;
    return csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
        profile, &runner, action, parameters, parameter_count, flags);
}

int csb_v1_runtime_execute_csbwin_character_death_filter(
    CSB_V1_RuntimeProfile *profile, int champion_index)
{
    int parameters[2];

    if (!profile || champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    parameters[0] = 1;
    parameters[1] = champion_index;
    return csb_v1_runtime_execute_csbwin_special_filter_action(
        profile, CSB_V1_EXPOOL_ESL_CHAR_DEATH_FILTER, 0, parameters, 2);
}

int csb_v1_runtime_execute_csbwin_equip_filter(
    CSB_V1_RuntimeProfile *profile, int champion_index, int slot_index,
    uint16_t old_thing, uint16_t new_thing)
{
    int parameters[5];

    if (!profile || champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS || slot_index < 0 ||
        slot_index >= CSB_V1_SLOT_COUNT) {
        return 0;
    }
    parameters[0] = 4;
    parameters[1] = champion_index;
    parameters[2] = slot_index;
    parameters[4] = 0;
    /* CSBWin Character.cpp::SetPossession uses RNnul (0xffff) as the
     * absence sentinel and performs the old-object callback first. */
    if (old_thing != 0xffffu) {
        parameters[3] = (int)old_thing;
        if (!csb_v1_runtime_execute_csbwin_special_filter_action(
                profile, CSB_V1_EXPOOL_ESL_EQUIP_FILTER, 1, parameters, 5)) {
            return 0;
        }
    }
    if (new_thing != 0xffffu) {
        parameters[3] = (int)new_thing;
        if (!csb_v1_runtime_execute_csbwin_special_filter_action(
                profile, CSB_V1_EXPOOL_ESL_EQUIP_FILTER, 0, parameters, 5)) {
            return 0;
        }
    }
    return 1;
}

int csb_v1_runtime_execute_csbwin_damage_character_filter(
    CSB_V1_RuntimeProfile *profile, int champion_index, int requested_damage,
    uint16_t wound_mask, uint16_t attack_type, int *out_final_damage)
{
    int parameters[7];
    int final_damage;

    if (!profile || !out_final_damage || champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS || requested_damage < 0 ||
        requested_damage > 32767) {
        return 0;
    }
    /* Character.cpp:2611-2800 assigns the seven A..G words after defense,
     * then uses only DSA parameter E as its pending-damage increment. */
    parameters[0] = champion_index;
    parameters[1] = profile->party_state.Champions[champion_index].Fingerprint;
    parameters[2] = requested_damage;
    parameters[3] = requested_damage;
    parameters[4] = (int)wound_mask;
    parameters[5] = (int)attack_type;
    parameters[6] = 0;
    if (!csb_v1_runtime_execute_csbwin_special_filter_action(
            profile, CSB_V1_EXPOOL_ESL_DAMAGE_CHAR_FILTER, 0,
            parameters, 7)) {
        return 0;
    }
    final_damage = (int)(int16_t)parameters[3];
    if (final_damage < 0) return 0;
    *out_final_damage = final_damage;
    return 1;
}

int csb_v1_runtime_execute_csbwin_cursor_read_game_filter(
    CSB_V1_RuntimeProfile *profile, uint16_t object_thing)
{
    int parameters[6];

    if (!profile) return 0;
    /* CSBWin SaveGame.cpp:1754-1760 restores GAMEBLOCK2.objectInHand, then
     * sends CURSORFILTER_ReadGame through MoveObject.cpp::CursorFilter.
     * Unlike cancelable cursor operations, that call ignores returned packet
     * words, so never let this bounded notification replace the save-owned
     * cursor object. */
    parameters[0] = (int)object_thing;
    parameters[1] = 1; /* CURSORFILTER_ReadGame */
    parameters[2] = 0;
    parameters[3] = 0;
    parameters[4] = 0;
    parameters[5] = 0;
    return csb_v1_runtime_execute_csbwin_special_filter_action(
        profile, CSB_V1_EXPOOL_ESL_CURSOR_FILTER, 0, parameters, 6);
}

int csb_v1_runtime_execute_csbwin_cursor_resume_saved_game_filter(
    CSB_V1_RuntimeProfile *profile, uint16_t object_thing)
{
    int parameters[6];

    if (!profile || object_thing == 0xffffu) return 0;
    /* CSBWin CSBCode.cpp::TAG0138ec sends this packet after SaveGame.cpp's
     * ReadGame notification and before ObjectToCursor. TAG0138ec does not
     * inspect the returned packet, so this callback cannot cancel or replace
     * the source-owned restored hand. */
    parameters[0] = (int)object_thing;
    parameters[1] = 9; /* CURSORFILTER_ResumeSavedGame */
    parameters[2] = 0;
    parameters[3] = 0;
    parameters[4] = 0;
    parameters[5] = 0;
    return csb_v1_runtime_execute_csbwin_special_filter_action(
        profile, CSB_V1_EXPOOL_ESL_CURSOR_FILTER, 0, parameters, 6);
}

int csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner)
{
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;

    if (!profile || !binding || !out_runner || action_ordinal < 0 ||
        !profile->csbwin_extended_features_valid ||
        !csb_v1_chaos_find_imported_action(
            &profile->csbwin_extended_dsa_state, binding->dsa_id,
            state_index, action_ordinal)) {
        return 0;
    }

    /* CSBWin DSA.cpp:5366-5407 maps the slave selector through the loaded
     * level index before Execute. The explicit action lookup above keeps this
     * receipt tied to that authenticated owner rather than a caller bytecode
     * buffer. */
    memset(&candidate, 0, sizeof(candidate));
    candidate.programs = &profile->csbwin_extended_dsa_state;
    candidate.dsa_id = binding->dsa_id;
    candidate.state_index = state_index;
    candidate.action_ordinal = action_ordinal;
    candidate.master_location = master_location;
    candidate.party_location_valid = profile->party_state_valid ? 1 : 0;
    candidate.party_level = profile->current_level;
    candidate.party_x = profile->party_x;
    candidate.party_y = profile->party_y;
    candidate.party_direction = profile->party_dir & 3;
    candidate.game_time_valid = 1;
    candidate.game_time = profile->game_time;
    candidate.random_state_valid = profile->csbwin_gameblock2_summary_valid ? 1 : 0;
    candidate.random_state = profile->csbwin_random_seed;
    candidate.dsa_slave_thing_valid = binding->actuator_identity_valid;
    candidate.dsa_slave_thing = binding->location.actuator_thing;
    /* Monster.cpp clears these four directions immediately before its
     * movement filter. The prepared runner owns only this transient filter
     * state; a later movement caller must consume it explicitly. */
    candidate.monster_move_inhibit_valid = 1;
    memset(candidate.monster_move_inhibit, 0,
           sizeof(candidate.monster_move_inhibit));
    candidate.saves_disabled_valid = 1;
    candidate.saves_disabled = profile->csbwin_saves_disabled ? 1 : 0;
    candidate.party_leader_index = -1;
    if (profile->party_state_valid &&
        profile->party_state.ChampionCount >= 0 &&
        profile->party_state.ChampionCount <= 4) {
        int champion_index;

        candidate.party_champions_valid = 1;
        candidate.party_champion_count = profile->party_state.ChampionCount;
        candidate.party_leader_index = profile->leader_index;
        if (candidate.party_leader_index < 0 ||
            candidate.party_leader_index >= candidate.party_champion_count) {
            candidate.party_leader_index = profile->party_state.LeaderIndex;
        }
        if (candidate.party_leader_index < 0 ||
            candidate.party_leader_index >= candidate.party_champion_count) {
            candidate.party_leader_index = -1;
        }
        for (champion_index = 0;
             champion_index < candidate.party_champion_count;
             ++champion_index) {
            const CSB_V1_Champion *champion =
                &profile->party_state.Champions[champion_index];

            candidate.party_champion_talents[champion_index] =
                champion->Talents;
            candidate.party_champion_fingerprints[champion_index] =
                champion->Fingerprint;
            candidate.party_champion_wounds[champion_index] =
                champion->Wounds;
            candidate.party_champion_health[champion_index] =
                champion->CurrentHealth;
        }
    }
    candidate.get_wing_talents = csb_v1_runtime_dsa_get_wing_talents;
    candidate.has_wing_character = csb_v1_runtime_dsa_has_wing_character;
    candidate.get_excell_flags = csb_v1_runtime_dsa_get_excell_flags;
    candidate.set_excell_flags = csb_v1_runtime_dsa_set_excell_flags;
    candidate.excell_user = (void *)profile;
    candidate.get_generator_delay = csb_v1_runtime_dsa_get_generator_delay;
    candidate.set_generator_delay = csb_v1_runtime_dsa_set_generator_delay;
    candidate.commit_generator_delay = csb_v1_runtime_dsa_commit_generator_delay;
    candidate.get_monster_info = csb_v1_runtime_dsa_get_monster_info;
    candidate.set_monster_info = csb_v1_runtime_dsa_set_monster_info;
    candidate.get_champion_possession =
        csb_v1_runtime_dsa_get_champion_possession;
    candidate.get_monster_possession = csb_v1_runtime_dsa_get_monster_possession;
    candidate.inspect_cells = csb_v1_runtime_dsa_inspect_cells;
    candidate.get_thing_type = csb_v1_runtime_dsa_get_thing_type;
    candidate.is_carried = csb_v1_runtime_dsa_is_carried;
    candidate.get_level_multiplier = csb_v1_runtime_dsa_get_level_multiplier;
    candidate.get_missile_info = csb_v1_runtime_dsa_get_missile_info;
    candidate.set_missile_info = csb_v1_runtime_dsa_set_missile_info;
    candidate.commit_missile_info = csb_v1_runtime_dsa_commit_missile_info;
    candidate.monster_invisible_enabled =
        (profile->csbwin_extended_features_flags32 & 0x00000002u) != 0u;
    candidate.monster_size4_enabled =
        (profile->csbwin_extended_features_flags32 & 0x00000004u) != 0u;
    candidate.get_cell_info = csb_v1_runtime_dsa_get_cell_info;
    candidate.resolve_cell_store = csb_v1_runtime_dsa_resolve_cell_store;
    candidate.set_cell_info = csb_v1_runtime_dsa_set_cell_info;
    candidate.copy_teleporter = csb_v1_runtime_dsa_copy_teleporter;
    candidate.get_object_property = csb_v1_runtime_dsa_get_object_property;
    candidate.set_object_property = csb_v1_runtime_dsa_set_object_property;
    candidate.normalize_object_property =
        csb_v1_runtime_dsa_normalize_object_property;
    candidate.get_actuator_payload = csb_v1_runtime_dsa_get_actuator_payload;
    candidate.set_actuator_payload = csb_v1_runtime_dsa_set_actuator_payload;
    candidate.copy_actuator_payload =
        csb_v1_runtime_dsa_copy_actuator_payload;
    candidate.prepare_experience_plus =
        csb_v1_runtime_dsa_prepare_experience_plus;
    candidate.add_experience_plus = csb_v1_runtime_dsa_add_experience_plus;
    candidate.prepare_cause_poison = csb_v1_runtime_dsa_prepare_cause_poison;
    candidate.commit_cause_poison = csb_v1_runtime_dsa_commit_cause_poison;
    candidate.get_mastery = csb_v1_runtime_dsa_get_mastery;
    candidate.get_party_info = csb_v1_runtime_dsa_get_party_info;
    candidate.queue_switch_action = csb_v1_runtime_dsa_queue_switch_action;
    candidate.dungeon_user = (void *)profile;
    candidate.wing_user = (void *)profile;
    if (profile->csbwin_global_variables_valid) {
        candidate.global_variable_count =
            profile->csbwin_global_variable_count;
        memcpy(candidate.global_variables, profile->csbwin_global_variables,
               (size_t)candidate.global_variable_count *
                   sizeof(candidate.global_variables[0]));
    }
    *out_runner = candidate;
    return 1;
}

int csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *runner,
    const CSB_V1_DSAImportedAction *action,
    int *parameters,
    int parameter_count,
    int flgs_inout[2])
{
    CSB_V1_CSBWinDSAFilterStackRunnerContext candidate;
    CSB_V1_RuntimeProfile profile_candidate;
    CSB_V1_DungeonData dungeon_candidate;
    CSB_V1_CSBWinDSACoreProgramReceipt core_receipt;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 execution_receipt;
    uint8_t *dungeon_raw_candidate = NULL;
    const CSB_V1_DSAImportedAction *expected;
    int staged_parameters[26];
    int global_count;
    int globals_changed;
    int saves_disabled_changed;
    int saves_disabled_before;
    int saves_disabled_after;
    int expool_changed;
    int dsa_state_changed;
    int party_talents_changed;
    uint32_t party_talents_before[4] = { 0u, 0u, 0u, 0u };
    uint32_t party_talents_after[4] = { 0u, 0u, 0u, 0u };
    int party_skill_experience_changed;
    int random_state_changed;
    uint32_t random_state_before;
    uint32_t random_state_after;
    int text_message_changed;
    CSB_V1_RuntimeTextMessageReceipt text_message_before;
    CSB_V1_RuntimeTextMessageReceipt text_message_after;
    int dungeon_changed = 0;
    int saved_dsa_state_transition_valid = 0;
    uint8_t saved_dsa_state_storage_kind = 0u;
    uint32_t saved_dsa_state_before = 0u;
    uint32_t saved_dsa_state_after = 0u;
    uint32_t saved_dsa_state_tail_fnv1a = 0u;
    uint8_t *saved_dsa_state_record;
    int saved_dsa_state_record_type;
    int saved_dsa_state_record_size;
    int missile_info_timer_owner_valid = 0;
    uint16_t missile_info_timer_index = CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    uint16_t missile_info_timer_queue_slot = CSB_V1_CSBWIN_TIMER_QUEUE_NONE;
    uint8_t missile_info_timer_function = 0u;
    uint8_t missile_info_timer_position_before = 0u;
    uint8_t missile_info_timer_position_after = 0u;
    uint32_t missile_info_timer_time = 0u;
    uint32_t excell_tail_fnv1a_before = 0u;
    uint32_t wing_talents_tail_fnv1a_before = 0u;
    int experience_receipt_valid = 0;
    int experience_selector = -1;
    int experience_skill = -1;
    int experience_basic_skill = -1;
    uint32_t experience_selected_before = 0u;
    uint32_t experience_selected_after = 0u;
    uint32_t experience_basic_before = 0u;
    uint32_t experience_basic_after = 0u;
    int monster_store_receipt_valid = 0;
    uint32_t monster_store_before[8];
    uint32_t monster_store_after[8];
    int cell_store_receipt_valid = 0;
    int false_pit_receipt_valid = 0;
    int cause_poison_receipt_valid = 0;
    int cause_poison_selector = -1;
    int cause_poison_attack = 0;
    int16_t cause_poison_health_before = 0;
    int16_t cause_poison_health_after = 0;
    uint16_t cause_poison_dose_before = 0u;
    uint16_t cause_poison_dose_after = 0u;
    uint8_t cause_poison_event_count_before = 0u;
    uint8_t cause_poison_event_count_after = 0u;
    uint16_t cause_poison_timer_event_index = DM1_EVENT_MAX_COUNT;
    uint16_t cause_poison_timer_attack = 0u;
    uint32_t cause_poison_timer_time = 0u;
    uint32_t cell_store_before[5];
    uint32_t cell_store_after[5];
    int teleporter_copy_receipt_valid = 0;
    uint32_t teleporter_copy_source_before[5];
    uint32_t teleporter_copy_destination_before[5];
    uint32_t teleporter_copy_destination_after[5];
    int i;

    if (!profile || !runner || !action ||
        (parameter_count > 0 && !parameters) || parameter_count < 0 ||
        parameter_count > 26 ||
        runner->programs != &profile->csbwin_extended_dsa_state ||
        !profile->csbwin_extended_features_valid) {
        return 0;
    }
    expected = csb_v1_chaos_find_imported_action(
        &profile->csbwin_extended_dsa_state, runner->dsa_id,
        runner->state_index, runner->action_ordinal);
    if (expected != action) return 0;
    excell_tail_fnv1a_before = profile->csbwin_appended_tail_fnv1a;
    wing_talents_tail_fnv1a_before = profile->csbwin_appended_tail_fnv1a;
    random_state_before = profile->csbwin_random_seed;
    saves_disabled_before = profile->csbwin_saves_disabled ? 1 : 0;
    text_message_before = profile->csbwin_text_message_receipt;
    memset(&profile->csbwin_last_dsa_execution_receipt, 0,
           sizeof(profile->csbwin_last_dsa_execution_receipt));
    if (csb_v1_csbwin_dsa_verify_authenticated_core_program(
            &profile->csbwin_extended_dsa_state, runner->dsa_id,
            runner->state_index, runner->action_ordinal, &core_receipt) !=
            CSB_V1_CSBWIN_DSA_CORE_OK ||
        !core_receipt.valid) {
        return 0;
    }

    global_count = profile->csbwin_global_variables_valid ?
        (int)profile->csbwin_global_variable_count : 0;
    if (global_count < 0 ||
        global_count > CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY) {
        return 0;
    }

    /* CSBWin DSA.cpp GLOBALFETCH/GLOBALSTORE address SaveGame.cpp's global
     * bank. Rehydrate immediately before Execute so caller-provided runner
     * bytes cannot become runtime state. */
    candidate = *runner;
    profile_candidate = *profile;
    /* DSA stores address the loaded DB records directly.  The profile copy is
     * otherwise shallow, so give the candidate its own raw dungeon bytes and
     * publish them only after every save-side effect has been persisted. */
    if (profile->dungeon_handle && profile->dungeon_handle->raw_data &&
        profile->dungeon_handle->raw_size > 0) {
        dungeon_candidate = *profile->dungeon_handle;
        dungeon_raw_candidate = (uint8_t *)malloc(dungeon_candidate.raw_size);
        if (!dungeon_raw_candidate) return 0;
        memcpy(dungeon_raw_candidate, dungeon_candidate.raw_data,
               dungeon_candidate.raw_size);
        dungeon_candidate.raw_data = dungeon_raw_candidate;
        profile_candidate.dungeon_handle = &dungeon_candidate;
    }
    memset(candidate.global_variables, 0, sizeof(candidate.global_variables));
    candidate.global_variable_count = global_count;
    memcpy(candidate.global_variables, profile->csbwin_global_variables,
           (size_t)global_count * sizeof(candidate.global_variables[0]));
    candidate.saves_disabled_valid = 1;
    candidate.saves_disabled = profile->csbwin_saves_disabled ? 1 : 0;
    candidate.random_state_valid = profile->csbwin_gameblock2_summary_valid ? 1 : 0;
    candidate.random_state = profile->csbwin_random_seed;
    if (profile->party_state_valid &&
        profile->party_state.ChampionCount >= 0 &&
        profile->party_state.ChampionCount <= 4) {
        candidate.party_champions_valid = 1;
        candidate.party_champion_count = profile->party_state.ChampionCount;
        candidate.party_leader_index = profile->leader_index;
        if (candidate.party_leader_index < 0 ||
            candidate.party_leader_index >= candidate.party_champion_count) {
            candidate.party_leader_index = profile->party_state.LeaderIndex;
        }
        for (i = 0; i < candidate.party_champion_count; ++i) {
            candidate.party_champion_talents[i] =
                profile->party_state.Champions[i].Talents;
            candidate.party_champion_fingerprints[i] =
                profile->party_state.Champions[i].Fingerprint;
        }
    } else {
        candidate.party_champions_valid = 0;
        candidate.party_champion_count = 0;
        candidate.party_leader_index = -1;
        memset(candidate.party_champion_talents, 0,
               sizeof(candidate.party_champion_talents));
        memset(candidate.party_champion_fingerprints, 0,
               sizeof(candidate.party_champion_fingerprints));
    }
    /* CSBWin DSA.cpp:3107-3135 reaches the loaded SKIN_CACHE through the
     * DSA stack.  Bind it to a profile candidate so an unsupported record,
     * bad location, or later bytecode failure cannot publish an EXPOOL edit. */
    candidate.get_skin = csb_v1_runtime_dsa_get_skin;
    candidate.set_skin = csb_v1_runtime_dsa_set_skin;
    candidate.skin_user = &profile_candidate;
    candidate.get_wing_talents = csb_v1_runtime_dsa_get_wing_talents;
    candidate.has_wing_character = csb_v1_runtime_dsa_has_wing_character;
    candidate.set_wing_talents = csb_v1_runtime_dsa_set_wing_talents;
    candidate.get_dsa_info = csb_v1_runtime_dsa_get_info;
    candidate.wing_user = &profile_candidate;
    candidate.get_excell_flags = csb_v1_runtime_dsa_get_excell_flags;
    candidate.set_excell_flags = csb_v1_runtime_dsa_set_excell_flags;
    candidate.excell_user = &profile_candidate;
    candidate.get_generator_delay = csb_v1_runtime_dsa_get_generator_delay;
    candidate.set_generator_delay = csb_v1_runtime_dsa_set_generator_delay;
    candidate.commit_generator_delay = csb_v1_runtime_dsa_commit_generator_delay;
    candidate.get_monster_info = csb_v1_runtime_dsa_get_monster_info;
    candidate.set_monster_info = csb_v1_runtime_dsa_set_monster_info;
    candidate.get_champion_possession =
        csb_v1_runtime_dsa_get_champion_possession;
    candidate.get_monster_possession = csb_v1_runtime_dsa_get_monster_possession;
    candidate.inspect_cells = csb_v1_runtime_dsa_inspect_cells;
    candidate.get_thing_type = csb_v1_runtime_dsa_get_thing_type;
    candidate.is_carried = csb_v1_runtime_dsa_is_carried;
    candidate.get_level_multiplier = csb_v1_runtime_dsa_get_level_multiplier;
    candidate.get_missile_info = csb_v1_runtime_dsa_get_missile_info;
    candidate.set_missile_info = csb_v1_runtime_dsa_set_missile_info;
    candidate.commit_missile_info = csb_v1_runtime_dsa_commit_missile_info;
    candidate.monster_invisible_enabled =
        (profile_candidate.csbwin_extended_features_flags32 & 0x00000002u) != 0u;
    candidate.monster_size4_enabled =
        (profile_candidate.csbwin_extended_features_flags32 & 0x00000004u) != 0u;
    candidate.get_cell_info = csb_v1_runtime_dsa_get_cell_info;
    candidate.resolve_cell_store = csb_v1_runtime_dsa_resolve_cell_store;
    candidate.set_cell_info = csb_v1_runtime_dsa_set_cell_info;
    candidate.copy_teleporter = csb_v1_runtime_dsa_copy_teleporter;
    candidate.get_object_property = csb_v1_runtime_dsa_get_object_property;
    candidate.set_object_property = csb_v1_runtime_dsa_set_object_property;
    candidate.normalize_object_property =
        csb_v1_runtime_dsa_normalize_object_property;
    candidate.get_actuator_payload = csb_v1_runtime_dsa_get_actuator_payload;
    candidate.set_actuator_payload = csb_v1_runtime_dsa_set_actuator_payload;
    candidate.copy_actuator_payload =
        csb_v1_runtime_dsa_copy_actuator_payload;
    candidate.prepare_experience_plus =
        csb_v1_runtime_dsa_prepare_experience_plus;
    candidate.add_experience_plus = csb_v1_runtime_dsa_add_experience_plus;
    candidate.prepare_cause_poison = csb_v1_runtime_dsa_prepare_cause_poison;
    candidate.commit_cause_poison = csb_v1_runtime_dsa_commit_cause_poison;
    candidate.get_mastery = csb_v1_runtime_dsa_get_mastery;
    candidate.get_party_info = csb_v1_runtime_dsa_get_party_info;
    candidate.discard_text = csb_v1_runtime_dsa_discard_text;
    candidate.queue_switch_action = csb_v1_runtime_dsa_queue_switch_action;
    candidate.dungeon_user = &profile_candidate;
    for (i = 0; i < parameter_count; ++i) {
        staged_parameters[i] = parameters[i];
    }
    if (!csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
            action, staged_parameters, parameter_count, flgs_inout,
            &candidate)) {
        free(dungeon_raw_candidate);
        return 0;
    }

    if (candidate.last_execution.cause_poison_count != 0u) {
        const CSB_V1_Champion *before;
        const CSB_V1_Champion *after;

        /* The public receipt is deliberately one exact PoisonCharacter
         * transaction.  A multi-target action needs an ordered source receipt
         * for every affected CHARDESC/TIMER pair and remains closed here. */
        if (candidate.last_execution.cause_poison_count != 1u) {
            free(dungeon_raw_candidate);
            return 0;
        }
        cause_poison_selector =
            candidate.last_execution.last_cause_poison_character_selector;
        cause_poison_attack = candidate.last_execution.last_cause_poison_attack;
        if (cause_poison_selector < 0 ||
            cause_poison_selector >= profile->party_state.ChampionCount ||
            cause_poison_selector >= CSB_V1_MAX_CHAMPIONS ||
            cause_poison_attack <= 0 || cause_poison_attack > 0xffff) {
            free(dungeon_raw_candidate);
            return 0;
        }
        before = &profile->party_state.Champions[cause_poison_selector];
        after = &profile_candidate.party_state.Champions[cause_poison_selector];
        if (before->CurrentHealth <= 0 ||
            (before->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0 ||
            after->CurrentHealth >= before->CurrentHealth) {
            free(dungeon_raw_candidate);
            return 0;
        }
        cause_poison_health_before = before->CurrentHealth;
        cause_poison_health_after = after->CurrentHealth;
        cause_poison_dose_before = before->PoisonDose;
        cause_poison_dose_after = after->PoisonDose;
        cause_poison_event_count_before = before->PoisonEventCount;
        cause_poison_event_count_after = after->PoisonEventCount;
        if (after->CurrentHealth > 0 && cause_poison_attack > 1) {
            cause_poison_timer_attack = (uint16_t)(cause_poison_attack - 1);
            if (after->PoisonEventCount !=
                    (uint8_t)(before->PoisonEventCount + 1u) ||
                !csb_v1_runtime_find_cause_poison_event(
                    &profile_candidate, cause_poison_selector,
                    cause_poison_timer_attack, &cause_poison_timer_event_index,
                    &cause_poison_timer_time)) {
                free(dungeon_raw_candidate);
                return 0;
            }
        } else if (after->PoisonEventCount != before->PoisonEventCount) {
            free(dungeon_raw_candidate);
            return 0;
        }
        cause_poison_receipt_valid = 1;
    }

    /* CSBWin SaveGame.cpp writes changed GLOBALSTORE state through EXPOOL.
     * A source-pure DSA action such as AMPERSAND2 NUMPARAM must not rewrite a
     * save tail merely because it crossed the runtime boundary. */
    globals_changed = memcmp(candidate.global_variables,
                             profile->csbwin_global_variables,
                             (size_t)global_count *
                                 sizeof(candidate.global_variables[0])) != 0;
    if (globals_changed) {
        memcpy(profile_candidate.csbwin_global_variables,
               candidate.global_variables,
               (size_t)global_count * sizeof(candidate.global_variables[0]));
        if (csb_v1_runtime_write_csbwin_global_variables(&profile_candidate) != 0) {
            free(dungeon_raw_candidate);
            return 0;
        }
    }
    saves_disabled_changed = candidate.saves_disabled !=
        (profile->csbwin_saves_disabled ? 1 : 0);
    saves_disabled_after = candidate.saves_disabled ? 1 : 0;
    if (saves_disabled_changed) {
        profile_candidate.csbwin_saves_disabled = candidate.saves_disabled;
    }
    random_state_changed = candidate.random_state_valid &&
        candidate.random_state != profile->csbwin_random_seed;
    random_state_after = candidate.random_state;
    if (random_state_changed) {
        profile_candidate.csbwin_random_seed = candidate.random_state;
    }
    text_message_changed = memcmp(&profile_candidate.csbwin_text_message_receipt,
                                  &profile->csbwin_text_message_receipt,
                                  sizeof(profile->csbwin_text_message_receipt)) != 0;
    text_message_after = profile_candidate.csbwin_text_message_receipt;
    party_talents_changed = 0;
    for (i = 0; i < candidate.party_champion_count; ++i) {
        party_talents_before[i] = profile->party_state.Champions[i].Talents;
        party_talents_after[i] = candidate.party_champion_talents[i];
        if (candidate.party_champion_talents[i] !=
            profile->party_state.Champions[i].Talents) {
            profile_candidate.party_state.Champions[i].Talents =
                candidate.party_champion_talents[i];
            party_talents_changed = 1;
        }
    }
    party_skill_experience_changed = 0;
    for (i = 0; i < candidate.party_champion_count; ++i) {
        if (memcmp(profile_candidate.party_state.Champions[i].SkillExperience,
                   profile->party_state.Champions[i].SkillExperience,
                   sizeof(profile_candidate.party_state.Champions[i]
                              .SkillExperience)) != 0) {
            party_skill_experience_changed = 1;
            break;
        }
    }
    dsa_state_changed = 0;
    if (profile_candidate.csbwin_extended_dsa_state.imported_headers[
            candidate.dsa_id].valid &&
        profile_candidate.csbwin_extended_dsa_state.imported_headers[
            candidate.dsa_id].local_state == 0u) {
        saved_dsa_state_before = runner->state_index;
        if (!csb_v1_runtime_persist_csbwin_localstate0_dsa(
                &profile_candidate, runner, &candidate)) {
            free(dungeon_raw_candidate);
            return 0;
        }
        saved_dsa_state_record = csb_v1_runtime_mutable_thing_record(
            profile_candidate.dungeon_handle, runner->dsa_slave_thing,
            &saved_dsa_state_record_type, &saved_dsa_state_record_size);
        if (!saved_dsa_state_record ||
            saved_dsa_state_record_type != CSB_V1_THING_TYPE_ACTUATOR ||
            saved_dsa_state_record_size < 4) {
            free(dungeon_raw_candidate);
            return 0;
        }
        dsa_state_changed = 1;
        saved_dsa_state_after = (uint32_t)(
            ((uint16_t)saved_dsa_state_record[2] |
             ((uint16_t)saved_dsa_state_record[3] << 8)) >> 12) & 0x0fu;
        saved_dsa_state_tail_fnv1a =
            profile_candidate.csbwin_appended_tail_fnv1a;
        saved_dsa_state_storage_kind =
            CSB_V1_CSBWIN_DSA_STATE_STORAGE_DB3_DSASTATE;
        saved_dsa_state_transition_valid = 1;
    } else if (profile_candidate.csbwin_appended_tail_valid &&
        profile_candidate.csbwin_appended_tail_preserved_size >=
            sizeof(" Extended Features ") &&
        memcmp(profile_candidate.csbwin_appended_tail,
               " Extended Features ", sizeof(" Extended Features ")) == 0 &&
        profile_candidate.csbwin_extended_dsa_state.imported_headers[
            candidate.dsa_id].valid &&
        profile_candidate.csbwin_extended_dsa_state.imported_headers[
            candidate.dsa_id].local_state == 1u) {
        saved_dsa_state_before = runner->state_index;
        if (!csb_v1_runtime_persist_csbwin_localstate1_dsa(
                &profile_candidate, runner, &candidate)) {
            free(dungeon_raw_candidate);
            return 0;
        }
        dsa_state_changed = 1;
        saved_dsa_state_after = profile_candidate.csbwin_extended_dsa_state
            .imported_headers[candidate.dsa_id].persistent_state;
        saved_dsa_state_tail_fnv1a =
            profile_candidate.csbwin_appended_tail_fnv1a;
        saved_dsa_state_storage_kind =
            CSB_V1_CSBWIN_DSA_STATE_STORAGE_SAVED_M_STATE;
        saved_dsa_state_transition_valid = 1;
    } else if (profile_candidate.csbwin_extended_dsa_state.imported_headers[
                   candidate.dsa_id].valid &&
               profile_candidate.csbwin_extended_dsa_state.imported_headers[
                   candidate.dsa_id].local_state == 2u) {
        /* LocalState 2 reads compact DB3 ParameterB.  A source action that
         * leaves state unchanged does not need a save-tail/DB3 writeback
         * proof; only transfer/SetNewState/relative-state changes attempt
         * to persist through the authenticated source owner. */
        if (!(candidate.transfer_execution_count ==
                  runner->transfer_execution_count &&
              candidate.execution_count == runner->execution_count + 1 &&
              candidate.last_execution.forced_state < 0 &&
              candidate.last_execution.next_state == 0)) {
            if (!csb_v1_runtime_has_verified_csbwin_extended_dsa_tail(
                    &profile_candidate) ||
                !csb_v1_runtime_persist_csbwin_localstate2_dsa(
                    &profile_candidate, runner, &candidate)) {
                free(dungeon_raw_candidate);
                return 0;
            }
            saved_dsa_state_record = csb_v1_runtime_mutable_thing_record(
                profile_candidate.dungeon_handle, runner->dsa_slave_thing,
                &saved_dsa_state_record_type, &saved_dsa_state_record_size);
            if (!saved_dsa_state_record ||
                saved_dsa_state_record_type != CSB_V1_THING_TYPE_ACTUATOR ||
                saved_dsa_state_record_size < 8) {
                free(dungeon_raw_candidate);
                return 0;
            }
            saved_dsa_state_before = runner->state_index;
            saved_dsa_state_after = (uint32_t)(
                ((uint16_t)saved_dsa_state_record[6] |
                 ((uint16_t)saved_dsa_state_record[7] << 8)) & 0x3fffu);
            saved_dsa_state_tail_fnv1a =
                profile_candidate.csbwin_appended_tail_fnv1a;
            saved_dsa_state_storage_kind =
                CSB_V1_CSBWIN_DSA_STATE_STORAGE_DB3_PARAMETER_B;
            saved_dsa_state_transition_valid = 1;
        }
    }
    expool_changed = profile_candidate.csbwin_appended_tail_fnv1a !=
                         profile->csbwin_appended_tail_fnv1a ||
        memcmp(profile_candidate.csbwin_appended_tail,
               profile->csbwin_appended_tail,
               sizeof(profile->csbwin_appended_tail)) != 0;
    if (candidate.last_execution.wing_talents_store_count != 0u) {
        uint32_t before_talents = 0u;
        uint32_t after_talents = 0u;

        /* CHARDESC::SaveToWings rewrites the complete eight-record wing
         * bundle. Prove the selected fingerprint and talent word on both
         * sides before the candidate tail becomes live. */
        if (!expool_changed ||
            csb_v1_runtime_read_csbwin_wing_talents(
                profile,
                candidate.last_execution.last_wing_talents_fingerprint,
                &before_talents) != 1 ||
            before_talents != candidate.last_execution.last_wing_talents_before ||
            csb_v1_runtime_read_csbwin_wing_talents(
                &profile_candidate,
                candidate.last_execution.last_wing_talents_fingerprint,
                &after_talents) != 1 ||
            after_talents != candidate.last_execution.last_wing_talents_after) {
            free(dungeon_raw_candidate);
            return 0;
        }
    }
    if (candidate.last_execution.experience_plus_count != 0u) {
        int selector = candidate.last_execution.last_experience_character_selector;
        int skill = candidate.last_execution.last_experience_skill_number;
        int basic_skill;
        uint32_t selected_after;
        uint32_t basic_after;

        /* Magic.cpp::AddToSkill writes one selected SKILL and, for a
         * subskill, its source basic SKILL. Re-derive that exact candidate
         * pair from the live CHARDESC image before publication. */
        if (csb_v1_runtime_csbwin_prepare_add_to_skill(
                profile, selector, skill,
                candidate.last_execution.last_experience_amount,
                &selected_after, &basic_after, &basic_skill) != 1 ||
            selector < 0 || selector >= profile->party_state.ChampionCount ||
            skill < 0 || skill >= CSB_V1_FULL_SKILL_COUNT || basic_skill < 0 ||
            basic_skill >= CSB_V1_FULL_SKILL_COUNT ||
            profile_candidate.party_state.Champions[selector]
                .SkillExperience[skill] != selected_after ||
            profile_candidate.party_state.Champions[selector]
                .SkillExperience[basic_skill] != basic_after) {
            free(dungeon_raw_candidate);
            return 0;
        }
        experience_receipt_valid = 1;
        experience_selector = selector;
        experience_skill = skill;
        experience_basic_skill = basic_skill;
        experience_selected_before =
            profile->party_state.Champions[selector].SkillExperience[skill];
        experience_selected_after = selected_after;
        experience_basic_before = profile->party_state.Champions[selector]
            .SkillExperience[basic_skill];
        experience_basic_after = basic_after;
    }
    if (candidate.last_execution.monster_store_count != 0u) {
        uint32_t before_values[8];
        uint32_t after_values[8];

        /* DSA.cpp MonsterStore has one DB4 owner.  Compare the complete
         * source image on both sides, not merely the selected write bits,
         * before the cloned dungeon can become live. */
        if (candidate.last_execution.last_monster_store_write_mask == 0u ||
            !csb_v1_runtime_dsa_get_monster_info(
                profile, candidate.last_execution.last_monster_store_thing,
                before_values) ||
            !csb_v1_runtime_dsa_get_monster_info(
                &profile_candidate,
                candidate.last_execution.last_monster_store_thing,
                after_values) ||
            memcmp(before_values,
                   candidate.last_execution.last_monster_store_before,
                   sizeof(before_values)) != 0 ||
            memcmp(after_values,
                   candidate.last_execution.last_monster_store_after,
                   sizeof(after_values)) != 0) {
            free(dungeon_raw_candidate);
            return 0;
        }
        memcpy(monster_store_before, before_values,
               sizeof(monster_store_before));
        memcpy(monster_store_after, after_values,
               sizeof(monster_store_after));
        monster_store_receipt_valid = 1;
    }
    if (candidate.last_execution.cell_store_count != 0u) {
        uint32_t before_values[5];
        uint32_t after_values[5];

        /* CellStore owns only one existing CELLFLAG plus the first source
         * DB0/DB1 record. Re-read that full Cell@ image across the clone
         * boundary before any raw Dungeon bytes are made live. */
        if (candidate.last_execution.last_cell_store_write_mask == 0u ||
            !csb_v1_runtime_dsa_get_cell_info(
                profile, candidate.last_execution.last_cell_store_location,
                before_values) ||
            !csb_v1_runtime_dsa_get_cell_info(
                &profile_candidate,
                candidate.last_execution.last_cell_store_location,
                after_values) ||
            memcmp(before_values,
                   candidate.last_execution.last_cell_store_before,
                   sizeof(before_values)) != 0 ||
            memcmp(after_values,
                   candidate.last_execution.last_cell_store_after,
                   sizeof(after_values)) != 0) {
            free(dungeon_raw_candidate);
            return 0;
        }
        memcpy(cell_store_before, before_values, sizeof(cell_store_before));
        memcpy(cell_store_after, after_values, sizeof(cell_store_after));
        cell_store_receipt_valid = 1;
    }
    if (candidate.last_execution.false_pit_count != 0u) {
        if (!cell_store_receipt_valid ||
            candidate.last_execution.false_pit_count != 1u ||
            candidate.last_execution.cell_store_count != 1u ||
            candidate.last_execution.last_false_pit_location !=
                candidate.last_execution.last_cell_store_location ||
            candidate.last_execution.last_false_pit_before[0] != 3u ||
            candidate.last_execution.last_false_pit_after[0] != 3u ||
            candidate.last_execution.last_cell_store_write_mask != (1u << 1) ||
            memcmp(candidate.last_execution.last_false_pit_before,
                   cell_store_before,
                   sizeof(cell_store_before)) != 0 ||
            memcmp(candidate.last_execution.last_false_pit_after,
                   cell_store_after,
                   sizeof(cell_store_after)) != 0 ||
            ((cell_store_before[1] & ~1u) !=
                 (cell_store_after[1] & ~1u)) ||
            memcmp(cell_store_before + 2u, cell_store_after + 2u,
                   3u * sizeof(uint32_t)) != 0) {
            free(dungeon_raw_candidate);
            return 0;
        }
        false_pit_receipt_valid = 1;
    }
    if (candidate.last_execution.teleporter_copy_count != 0u) {
        uint32_t source_before[5];
        uint32_t destination_before[5];
        uint32_t destination_after[5];

        if (!csb_v1_runtime_dsa_get_cell_info(
                profile,
                candidate.last_execution.last_teleporter_copy_source_location,
                source_before) ||
            !csb_v1_runtime_dsa_get_cell_info(
                profile,
                candidate.last_execution.last_teleporter_copy_destination_location,
                destination_before) ||
            !csb_v1_runtime_dsa_get_cell_info(
                &profile_candidate,
                candidate.last_execution.last_teleporter_copy_destination_location,
                destination_after) ||
            memcmp(source_before,
                   candidate.last_execution.last_teleporter_copy_source_before,
                   sizeof(source_before)) != 0 ||
            memcmp(destination_before,
                   candidate.last_execution.last_teleporter_copy_destination_before,
                   sizeof(destination_before)) != 0 ||
            memcmp(destination_after,
                   candidate.last_execution.last_teleporter_copy_destination_after,
                   sizeof(destination_after)) != 0) {
            free(dungeon_raw_candidate);
            return 0;
        }
        memcpy(teleporter_copy_source_before, source_before,
               sizeof(teleporter_copy_source_before));
        memcpy(teleporter_copy_destination_before, destination_before,
               sizeof(teleporter_copy_destination_before));
        memcpy(teleporter_copy_destination_after, destination_after,
               sizeof(teleporter_copy_destination_after));
        teleporter_copy_receipt_valid = 1;
    }
    if (candidate.last_execution.missile_info_store_count != 0u) {
        const uint8_t *before_missile;
        const uint8_t *after_missile;
        const CSB_V1_CSBWin512TimerSummary *before_timer;
        const CSB_V1_CSBWin512TimerSummary *after_timer;
        uint16_t candidate_missile_queue_slot;
        int before_type;
        int after_type;
        int before_size;
        int after_size;

        before_missile = csb_v1_dungeon_get_thing_record(
            profile->dungeon_handle,
            candidate.last_execution.last_missile_info_thing,
            &before_type, NULL, &before_size);
        after_missile = csb_v1_dungeon_get_thing_record(
            profile_candidate.dungeon_handle,
            candidate.last_execution.last_missile_info_thing,
            &after_type, NULL, &after_size);
        if (!before_missile || !after_missile || before_type != 14 ||
            after_type != 14 || before_size < 8 || after_size < 8 ||
            csb_v1_runtime_read_u16(before_missile + 6) !=
                csb_v1_runtime_read_u16(after_missile + 6)) {
            free(dungeon_raw_candidate);
            return 0;
        }
        missile_info_timer_index = csb_v1_runtime_read_u16(before_missile + 6);
        if (missile_info_timer_index >= profile->csbwin_timer_summary_count ||
            missile_info_timer_index >=
                profile_candidate.csbwin_timer_summary_count ||
            !csb_v1_runtime_find_saved_timer_queue_slot(
                profile, &profile->csbwin_timers[missile_info_timer_index],
                &missile_info_timer_queue_slot) ||
            !csb_v1_runtime_find_saved_timer_queue_slot(
                &profile_candidate,
                &profile_candidate.csbwin_timers[missile_info_timer_index],
                &candidate_missile_queue_slot) ||
            candidate_missile_queue_slot != missile_info_timer_queue_slot) {
            free(dungeon_raw_candidate);
            return 0;
        }
        before_timer = &profile->csbwin_timers[missile_info_timer_index];
        after_timer = &profile_candidate.csbwin_timers[missile_info_timer_index];
        if (!before_timer->valid || !after_timer->valid ||
            before_timer->truncated || after_timer->truncated ||
            before_timer->source_index != missile_info_timer_index ||
            after_timer->source_index != missile_info_timer_index ||
            before_timer->function != after_timer->function ||
            before_timer->time != after_timer->time ||
            ((before_timer->ubyte8 >> 2) & 3u) !=
                candidate.last_execution.last_missile_info_before[3] ||
            ((after_timer->ubyte8 >> 2) & 3u) !=
                candidate.last_execution.last_missile_info_after[3]) {
            free(dungeon_raw_candidate);
            return 0;
        }
        missile_info_timer_owner_valid = 1;
        missile_info_timer_function = before_timer->function;
        missile_info_timer_position_before = before_timer->ubyte8;
        missile_info_timer_position_after = after_timer->ubyte8;
        missile_info_timer_time = before_timer->time;
    }
    if (globals_changed) {
        memcpy(profile->csbwin_global_variables,
               profile_candidate.csbwin_global_variables,
               sizeof(profile->csbwin_global_variables));
    }
    if (saves_disabled_changed) {
        profile->csbwin_saves_disabled =
            profile_candidate.csbwin_saves_disabled;
    }
    if (random_state_changed) {
        profile->csbwin_random_seed = profile_candidate.csbwin_random_seed;
    }
    if (text_message_changed) {
        profile->csbwin_text_message_receipt =
            profile_candidate.csbwin_text_message_receipt;
    }
    if (party_talents_changed) {
        for (i = 0; i < candidate.party_champion_count; ++i) {
            profile->party_state.Champions[i].Talents =
                profile_candidate.party_state.Champions[i].Talents;
        }
    }
    if (party_skill_experience_changed) {
        for (i = 0; i < candidate.party_champion_count; ++i) {
            memcpy(profile->party_state.Champions[i].SkillExperience,
                   profile_candidate.party_state.Champions[i].SkillExperience,
                   sizeof(profile->party_state.Champions[i].SkillExperience));
        }
    }
    if (cause_poison_receipt_valid) {
        profile->party_state = profile_candidate.party_state;
        profile->leader_index = profile_candidate.leader_index;
        profile->game_over = profile_candidate.game_over;
        profile->timeline_queue = profile_candidate.timeline_queue;
        memcpy(profile->csbwin_poison_event_attack,
               profile_candidate.csbwin_poison_event_attack,
               sizeof(profile->csbwin_poison_event_attack));
        memcpy(profile->csbwin_poison_event_attack_valid,
               profile_candidate.csbwin_poison_event_attack_valid,
               sizeof(profile->csbwin_poison_event_attack_valid));
    }
    if (expool_changed) {
        memcpy(profile->csbwin_appended_tail,
               profile_candidate.csbwin_appended_tail,
               sizeof(profile->csbwin_appended_tail));
        profile->csbwin_appended_tail_fnv1a =
            profile_candidate.csbwin_appended_tail_fnv1a;
        profile->csbwin_skin_cache_tail_receipt_valid =
            profile_candidate.csbwin_skin_cache_tail_receipt_valid;
        profile->csbwin_skin_cache_tail_valid =
            profile_candidate.csbwin_skin_cache_tail_valid;
        profile->csbwin_skin_cache_tail_size =
            profile_candidate.csbwin_skin_cache_tail_size;
        profile->csbwin_skin_cache_tail_fnv1a =
            profile_candidate.csbwin_skin_cache_tail_fnv1a;
        profile->skin_cache = profile_candidate.skin_cache;
    }
    if (dsa_state_changed) {
        profile->csbwin_extended_dsa_state.imported_headers[candidate.dsa_id]
            .persistent_state =
            profile_candidate.csbwin_extended_dsa_state.imported_headers[
                candidate.dsa_id].persistent_state;
    }
    if (dungeon_raw_candidate) {
        dungeon_changed = memcmp(dungeon_raw_candidate,
                                 profile->dungeon_handle->raw_data,
                                 profile->dungeon_handle->raw_size) != 0;
        if (dungeon_changed) {
            memcpy(profile->dungeon_handle->raw_data, dungeon_raw_candidate,
                   profile->dungeon_handle->raw_size);
        }
        free(dungeon_raw_candidate);
    }
    for (i = 0; i < parameter_count; ++i) {
        parameters[i] = staged_parameters[i];
    }
    memset(&execution_receipt, 0, sizeof(execution_receipt));
    execution_receipt.valid = 1;
    execution_receipt.transfer_only = core_receipt.transfer_only ? 1 : 0;
    execution_receipt.stack_core = core_receipt.stack_core ? 1 : 0;
    execution_receipt.requires_runtime_owner =
        core_receipt.requires_runtime_owner ? 1 : 0;
    execution_receipt.conditional_core =
        core_receipt.conditional_core ? 1 : 0;
    execution_receipt.arithmetic_core =
        core_receipt.arithmetic_core ? 1 : 0;
    execution_receipt.variable_core = core_receipt.variable_core ? 1 : 0;
    execution_receipt.timer_core = core_receipt.timer_core ? 1 : 0;
    execution_receipt.message_core = core_receipt.message_core ? 1 : 0;
    execution_receipt.dungeon_mutation_core =
        core_receipt.dungeon_mutation_core ? 1 : 0;
    execution_receipt.timer_type_modifiers_valid =
        candidate.timer_type_modifiers_valid ? 1 : 0;
    if (execution_receipt.timer_type_modifiers_valid) {
        memcpy(execution_receipt.timer_type_modifiers,
               candidate.timer_type_modifiers,
               sizeof(execution_receipt.timer_type_modifiers));
    }
    execution_receipt.saved_dsa_state_transition_valid =
        saved_dsa_state_transition_valid;
    if (execution_receipt.saved_dsa_state_transition_valid) {
        execution_receipt.saved_dsa_state_storage_kind =
            saved_dsa_state_storage_kind;
        execution_receipt.saved_dsa_state_before = saved_dsa_state_before;
        execution_receipt.saved_dsa_state_after = saved_dsa_state_after;
        execution_receipt.saved_dsa_state_tail_fnv1a =
            saved_dsa_state_tail_fnv1a;
    }
    execution_receipt.rollback_guarded = 1;
    execution_receipt.parameter_count = parameter_count;
    execution_receipt.command_count = core_receipt.command_count;
    execution_receipt.words_consumed = core_receipt.words_consumed;
    execution_receipt.stack_depth = candidate.last_execution.stack_depth;
    execution_receipt.timer_scheduled_count =
        candidate.last_execution.timer_scheduled_count;
    execution_receipt.last_scheduled_event_type =
        candidate.last_execution.last_scheduled_event_type;
    execution_receipt.last_scheduled_target_location =
        candidate.last_execution.last_scheduled_target_location;
    execution_receipt.teleporter_copy_count =
        candidate.last_execution.teleporter_copy_count;
    execution_receipt.last_teleporter_copy_source_location =
        candidate.last_execution.last_teleporter_copy_source_location;
    execution_receipt.last_teleporter_copy_destination_location =
        candidate.last_execution.last_teleporter_copy_destination_location;
    if (execution_receipt.teleporter_copy_count != 0u) {
        if (!teleporter_copy_receipt_valid) return 0;
        memcpy(execution_receipt.last_teleporter_copy_source_before,
               teleporter_copy_source_before,
               sizeof(execution_receipt.last_teleporter_copy_source_before));
        memcpy(execution_receipt.last_teleporter_copy_destination_before,
               teleporter_copy_destination_before,
               sizeof(execution_receipt.last_teleporter_copy_destination_before));
        memcpy(execution_receipt.last_teleporter_copy_destination_after,
               teleporter_copy_destination_after,
               sizeof(execution_receipt.last_teleporter_copy_destination_after));
    }
    execution_receipt.actuator_copy_count =
        candidate.last_execution.actuator_copy_count;
    execution_receipt.last_actuator_copy_source_thing =
        candidate.last_execution.last_actuator_copy_source_thing;
    execution_receipt.last_actuator_copy_destination_thing =
        candidate.last_execution.last_actuator_copy_destination_thing;
    execution_receipt.skin_store_count =
        candidate.last_execution.skin_store_count;
    if (execution_receipt.skin_store_count != 0u) {
        execution_receipt.last_skin_store_location =
            candidate.last_execution.last_skin_store_location;
        execution_receipt.last_skin_store_before =
            candidate.last_execution.last_skin_store_before;
        execution_receipt.last_skin_store_after =
            candidate.last_execution.last_skin_store_after;
    }
    execution_receipt.wing_talents_store_count =
        candidate.last_execution.wing_talents_store_count;
    if (execution_receipt.wing_talents_store_count != 0u) {
        execution_receipt.last_wing_talents_fingerprint =
            candidate.last_execution.last_wing_talents_fingerprint;
        execution_receipt.last_wing_talents_before =
            candidate.last_execution.last_wing_talents_before;
        execution_receipt.last_wing_talents_after =
            candidate.last_execution.last_wing_talents_after;
        execution_receipt.wing_talents_tail_fnv1a_before =
            wing_talents_tail_fnv1a_before;
        execution_receipt.wing_talents_tail_fnv1a_after =
            profile_candidate.csbwin_appended_tail_fnv1a;
    }
    execution_receipt.experience_plus_count =
        candidate.last_execution.experience_plus_count;
    if (execution_receipt.experience_plus_count != 0u) {
        if (!experience_receipt_valid) return 0;
        execution_receipt.last_experience_character_selector =
            experience_selector;
        execution_receipt.last_experience_skill_number = experience_skill;
        execution_receipt.last_experience_basic_skill_number =
            experience_basic_skill;
        execution_receipt.last_experience_amount =
            candidate.last_execution.last_experience_amount;
        execution_receipt.last_experience_selected_before =
            experience_selected_before;
        execution_receipt.last_experience_selected_after =
            experience_selected_after;
        execution_receipt.last_experience_basic_before = experience_basic_before;
        execution_receipt.last_experience_basic_after = experience_basic_after;
    }
    execution_receipt.monster_store_count =
        candidate.last_execution.monster_store_count;
    if (execution_receipt.monster_store_count != 0u) {
        if (!monster_store_receipt_valid) return 0;
        execution_receipt.last_monster_store_thing =
            candidate.last_execution.last_monster_store_thing;
        execution_receipt.last_monster_store_write_mask =
            candidate.last_execution.last_monster_store_write_mask;
        memcpy(execution_receipt.last_monster_store_before,
               monster_store_before,
               sizeof(execution_receipt.last_monster_store_before));
        memcpy(execution_receipt.last_monster_store_after,
               monster_store_after,
               sizeof(execution_receipt.last_monster_store_after));
    }
    execution_receipt.cell_store_count =
        candidate.last_execution.cell_store_count;
    if (execution_receipt.cell_store_count != 0u) {
        if (!cell_store_receipt_valid) return 0;
        execution_receipt.last_cell_store_location =
            candidate.last_execution.last_cell_store_location;
        execution_receipt.last_cell_store_write_mask =
            candidate.last_execution.last_cell_store_write_mask;
        memcpy(execution_receipt.last_cell_store_before, cell_store_before,
               sizeof(execution_receipt.last_cell_store_before));
        memcpy(execution_receipt.last_cell_store_after, cell_store_after,
               sizeof(execution_receipt.last_cell_store_after));
    }
    execution_receipt.false_pit_count = candidate.last_execution.false_pit_count;
    if (execution_receipt.false_pit_count != 0u) {
        if (!false_pit_receipt_valid) return 0;
        execution_receipt.last_false_pit_location =
            candidate.last_execution.last_false_pit_location;
        memcpy(execution_receipt.last_false_pit_before,
               candidate.last_execution.last_false_pit_before,
               sizeof(execution_receipt.last_false_pit_before));
        memcpy(execution_receipt.last_false_pit_after,
               candidate.last_execution.last_false_pit_after,
               sizeof(execution_receipt.last_false_pit_after));
    }
    execution_receipt.object_property_store_count =
        candidate.last_execution.object_property_store_count;
    if (execution_receipt.object_property_store_count != 0u) {
        execution_receipt.last_object_property_thing =
            candidate.last_execution.last_object_property_thing;
        execution_receipt.last_object_property_kind =
            candidate.last_execution.last_object_property_kind;
        execution_receipt.last_object_property_before =
            candidate.last_execution.last_object_property_before;
        execution_receipt.last_object_property_after =
            candidate.last_execution.last_object_property_after;
    }
    execution_receipt.missile_info_store_count =
        candidate.last_execution.missile_info_store_count;
    execution_receipt.last_missile_info_thing =
        candidate.last_execution.last_missile_info_thing;
    if (execution_receipt.missile_info_store_count != 0u) {
        memcpy(execution_receipt.last_missile_info_before,
               candidate.last_execution.last_missile_info_before,
               sizeof(execution_receipt.last_missile_info_before));
        memcpy(execution_receipt.last_missile_info_after,
               candidate.last_execution.last_missile_info_after,
               sizeof(execution_receipt.last_missile_info_after));
        execution_receipt.missile_info_timer_owner_valid =
            missile_info_timer_owner_valid;
        execution_receipt.missile_info_timer_index = missile_info_timer_index;
        execution_receipt.missile_info_timer_queue_slot =
            missile_info_timer_queue_slot;
        execution_receipt.missile_info_timer_function =
            missile_info_timer_function;
        execution_receipt.missile_info_timer_position_before =
            missile_info_timer_position_before;
        execution_receipt.missile_info_timer_position_after =
            missile_info_timer_position_after;
        execution_receipt.missile_info_timer_time = missile_info_timer_time;
    }
    execution_receipt.excell_store_count =
        candidate.last_execution.excell_store_count;
    if (execution_receipt.excell_store_count != 0u) {
        execution_receipt.last_excell_store_location =
            candidate.last_execution.last_excell_store_location;
        memcpy(execution_receipt.last_excell_store_before,
               candidate.last_execution.last_excell_store_before,
               sizeof(execution_receipt.last_excell_store_before));
        memcpy(execution_receipt.last_excell_store_after,
               candidate.last_execution.last_excell_store_after,
               sizeof(execution_receipt.last_excell_store_after));
        execution_receipt.excell_tail_fnv1a_before =
            excell_tail_fnv1a_before;
        execution_receipt.excell_tail_fnv1a_after =
            profile_candidate.csbwin_appended_tail_fnv1a;
    }
    execution_receipt.generator_delay_store_count =
        candidate.last_execution.generator_delay_store_count;
    if (execution_receipt.generator_delay_store_count != 0u) {
        execution_receipt.last_generator_delay_location =
            candidate.last_execution.last_generator_delay_location;
        execution_receipt.last_generator_delay_before =
            candidate.last_execution.last_generator_delay_before;
        execution_receipt.last_generator_delay_after =
            candidate.last_execution.last_generator_delay_after;
        execution_receipt.generator_delay_has_generator =
            candidate.last_execution.last_generator_delay_has_generator;
    }
    execution_receipt.cause_poison_count =
        candidate.last_execution.cause_poison_count;
    if (execution_receipt.cause_poison_count != 0u) {
        if (!cause_poison_receipt_valid) return 0;
        execution_receipt.last_cause_poison_character_selector =
            cause_poison_selector;
        execution_receipt.last_cause_poison_attack = cause_poison_attack;
        execution_receipt.last_cause_poison_health_before =
            cause_poison_health_before;
        execution_receipt.last_cause_poison_health_after =
            cause_poison_health_after;
        execution_receipt.last_cause_poison_dose_before =
            cause_poison_dose_before;
        execution_receipt.last_cause_poison_dose_after =
            cause_poison_dose_after;
        execution_receipt.last_cause_poison_event_count_before =
            cause_poison_event_count_before;
        execution_receipt.last_cause_poison_event_count_after =
            cause_poison_event_count_after;
        execution_receipt.last_cause_poison_timer_event_index =
            cause_poison_timer_event_index;
        execution_receipt.last_cause_poison_timer_attack =
            cause_poison_timer_attack;
        execution_receipt.last_cause_poison_timer_time =
            cause_poison_timer_time;
    }
    if (candidate.transfer_execution_count != runner->transfer_execution_count) {
        execution_receipt.transfer_count = candidate.last_transfer.transfer_count;
        execution_receipt.transfer_return_count =
            candidate.last_transfer.return_count;
        execution_receipt.transfer_frame_push_count =
            candidate.last_transfer.frame_push_count;
        execution_receipt.transfer_frame_pop_count =
            candidate.last_transfer.frame_pop_count;
        execution_receipt.maximum_subroutine_depth =
            candidate.last_transfer.maximum_subroutine_depth;
        execution_receipt.transfer_returned_by_missing_program =
            candidate.last_transfer.returned_by_missing_program;
    }
    execution_receipt.next_state = candidate.last_execution.next_state;
    execution_receipt.forced_state = candidate.last_execution.forced_state;
    execution_receipt.transfer_final_state =
        candidate.transfer_execution_count != runner->transfer_execution_count ?
            candidate.last_transfer.final_state : -1;
    execution_receipt.dsa_id = (uint8_t)runner->dsa_id;
    execution_receipt.state_index = runner->state_index;
    execution_receipt.column = action->column;
    execution_receipt.action_ordinal = runner->action_ordinal;
    execution_receipt.globals_changed = globals_changed ? 1 : 0;
    execution_receipt.saves_disabled_changed = saves_disabled_changed ? 1 : 0;
    if (execution_receipt.saves_disabled_changed) {
        execution_receipt.saves_disabled_before = saves_disabled_before;
        execution_receipt.saves_disabled_after = saves_disabled_after;
    }
    execution_receipt.random_state_changed = random_state_changed ? 1 : 0;
    if (execution_receipt.random_state_changed) {
        execution_receipt.random_state_before = random_state_before;
        execution_receipt.random_state_after = random_state_after;
    }
    execution_receipt.text_message_changed = text_message_changed ? 1 : 0;
    if (execution_receipt.text_message_changed) {
        execution_receipt.text_message_before = text_message_before;
        execution_receipt.text_message_after = text_message_after;
    }
    execution_receipt.party_talents_changed = party_talents_changed ? 1 : 0;
    if (execution_receipt.party_talents_changed) {
        execution_receipt.party_talents_champion_count =
            candidate.party_champion_count;
        memcpy(execution_receipt.party_talents_fingerprints,
               candidate.party_champion_fingerprints,
               sizeof(execution_receipt.party_talents_fingerprints));
        memcpy(execution_receipt.party_talents_before, party_talents_before,
               sizeof(execution_receipt.party_talents_before));
        memcpy(execution_receipt.party_talents_after, party_talents_after,
               sizeof(execution_receipt.party_talents_after));
    }
    execution_receipt.party_skill_experience_changed =
        party_skill_experience_changed ? 1 : 0;
    execution_receipt.expool_changed = expool_changed ? 1 : 0;
    execution_receipt.dsa_state_changed = dsa_state_changed ? 1 : 0;
    execution_receipt.dungeon_changed = dungeon_changed ? 1 : 0;
    profile->csbwin_last_dsa_execution_receipt = execution_receipt;
    *runner = candidate;
    return 1;
}

int csb_v1_runtime_prepare_csbwin_dsa_filter_stack_adapter(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    CSB_V1_RuntimeDSAFilterStackAdapter *out_adapter)
{
    CSB_V1_RuntimeDSAFilterStackAdapter candidate;

    if (!profile || !binding || !out_adapter) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    if (!csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
            profile, binding, state_index, action_ordinal, master_location,
            &candidate.runner)) {
        return 0;
    }
    /* CSBWin Monster.cpp:1134-1180 and 3222-3370 invoke ProcessDSAFilter
     * through a function-shaped boundary. Keep its user data tied to the
     * save/profile that authenticated the selected DSAAction. */
    candidate.profile = profile;
    *out_adapter = candidate;
    return 1;
}

int csb_v1_runtime_csbwin_dsa_filter_stack_runner_callback(
    const CSB_V1_DSAImportedAction *action,
    int *parameters,
    int parameter_count,
    int flgs_inout[2],
    void *user)
{
    CSB_V1_RuntimeDSAFilterStackAdapter *adapter =
        (CSB_V1_RuntimeDSAFilterStackAdapter *)user;

    if (!adapter || !adapter->profile) {
        return 0;
    }
    return csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
        adapter->profile, &adapter->runner, action, parameters,
        parameter_count, flgs_inout);
}

static void csb_v1_runtime_init_csbwin_filter_candidate(
    CSB_V1_DSAFilterRuntime *filter)
{
    int level;

    memset(filter, 0, sizeof(*filter));
    filter->attack_filter_dsa_id = -1;
    filter->attack_filter_action = -1;
    for (level = 0; level < (int)(sizeof(filter->movement_filter_dsa_id) /
                                  sizeof(filter->movement_filter_dsa_id[0]));
         ++level) {
        filter->movement_filter_dsa_id[level] = -1;
        filter->movement_filter_action[level] = -1;
    }
}

int csb_v1_runtime_bind_csbwin_attack_filter_stack_runtime(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    int loaded_level,
    CSB_V1_DSAFilterRuntime *out_filter,
    CSB_V1_RuntimeDSAFilterStackAdapter *out_adapter)
{
    CSB_V1_DSAFilterRuntime filter_candidate;
    CSB_V1_RuntimeDSAFilterStackAdapter adapter_candidate;

    if (!profile || !binding || !out_filter || !out_adapter ||
        loaded_level < 0) {
        return 0;
    }

    /* CSBWin Monster.cpp:1134-1180 resolves the selected type-47 filter,
     * packs ATTACK_PARAMETERES, then calls ProcessDSAFilter. Install exactly
     * that callback shape only after the save-owned action was authenticated.
     * Staging both objects keeps a rejected binding from exposing a partial
     * live filter. */
    memset(&adapter_candidate, 0, sizeof(adapter_candidate));
    if (!csb_v1_runtime_prepare_csbwin_dsa_filter_stack_adapter(
            profile, binding, state_index, action_ordinal, master_location,
            &adapter_candidate)) {
        return 0;
    }
    csb_v1_runtime_init_csbwin_filter_candidate(&filter_candidate);
    filter_candidate.programs = &profile->csbwin_extended_dsa_state;
    filter_candidate.runner =
        csb_v1_runtime_csbwin_dsa_filter_stack_runner_callback;
    filter_candidate.runner_user = &adapter_candidate;
    filter_candidate.loaded_level = loaded_level;
    filter_candidate.attack_filter_dsa_id = binding->dsa_id;
    filter_candidate.attack_filter_state = state_index;
    filter_candidate.attack_filter_action = action_ordinal;

    *out_adapter = adapter_candidate;
    /* runner_user must refer to the published adapter, never its staged copy. */
    filter_candidate.runner_user = out_adapter;
    *out_filter = filter_candidate;
    return 1;
}

int csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    int loaded_level,
    CSB_V1_DSAFilterRuntime *out_filter,
    CSB_V1_RuntimeDSAFilterStackAdapter *out_adapter)
{
    CSB_V1_DSAFilterRuntime filter_candidate;
    CSB_V1_RuntimeDSAFilterStackAdapter adapter_candidate;
    int filter_level;

    if (!profile || !binding || !out_filter || !out_adapter ||
        loaded_level < 0) {
        return 0;
    }
    filter_level = binding->location.level;
    if (filter_level < 0 || filter_level >=
        (int)(sizeof(filter_candidate.movement_filter_dsa_id) /
              sizeof(filter_candidate.movement_filter_dsa_id[0]))) {
        return 0;
    }

    /* CSBWin Monster.cpp:3079-3176 resolves one type-47 movement filter for
     * a loaded level, then 3222-3370 enters ProcessDSAFilter with its seven
     * movement parameters. As with attacks, install nothing until the exact
     * save-owned action has passed the existing authentication boundary. */
    memset(&adapter_candidate, 0, sizeof(adapter_candidate));
    if (!csb_v1_runtime_prepare_csbwin_dsa_filter_stack_adapter(
            profile, binding, state_index, action_ordinal, master_location,
            &adapter_candidate)) {
        return 0;
    }
    csb_v1_runtime_init_csbwin_filter_candidate(&filter_candidate);
    filter_candidate.programs = &profile->csbwin_extended_dsa_state;
    filter_candidate.runner =
        csb_v1_runtime_csbwin_dsa_filter_stack_runner_callback;
    filter_candidate.runner_user = &adapter_candidate;
    filter_candidate.loaded_level = loaded_level;
    filter_candidate.movement_filter_dsa_id[filter_level] = binding->dsa_id;
    filter_candidate.movement_filter_state[filter_level] = state_index;
    filter_candidate.movement_filter_action[filter_level] = action_ordinal;

    *out_adapter = adapter_candidate;
    filter_candidate.runner_user = out_adapter;
    *out_filter = filter_candidate;
    return 1;
}

int csb_v1_runtime_csbwin_movement_filter_stack_runner_callback(
    const CSB_V1_DSAImportedAction *action,
    int *parameters,
    int parameter_count,
    int flgs_inout[2],
    void *user)
{
    CSB_V1_RuntimeDSAMovementFilterStackAdapter *adapter =
        (CSB_V1_RuntimeDSAMovementFilterStackAdapter *)user;
    int i;

    if (!adapter || !adapter->profile || !action ||
        adapter->runner_count < 1 || adapter->runner_count >
            CSB_V1_RUNTIME_CSBWIN_MOVEMENT_FILTER_CAP) {
        return 0;
    }
    for (i = 0; i < adapter->runner_count; ++i) {
        CSB_V1_CSBWinDSAFilterStackRunnerContext *runner =
            &adapter->runners[i];
        const CSB_V1_DSAImportedAction *expected =
            csb_v1_chaos_find_imported_action(
                &adapter->profile->csbwin_extended_dsa_state,
                runner->dsa_id, runner->state_index, runner->action_ordinal);

        if (expected == action) {
            return csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                adapter->profile, runner, action, parameters,
                parameter_count, flgs_inout);
        }
    }
    return 0;
}

int csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime_multi(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAMovementFilterRequest *requests,
    size_t request_count,
    int loaded_level,
    CSB_V1_DSAFilterRuntime *out_filter,
    CSB_V1_RuntimeDSAMovementFilterStackAdapter *out_adapter)
{
    CSB_V1_DSAFilterRuntime filter_candidate;
    CSB_V1_RuntimeDSAMovementFilterStackAdapter adapter_candidate;
    const CSB_V1_DSAImportedAction *selected[
        CSB_V1_RUNTIME_CSBWIN_MOVEMENT_FILTER_CAP];
    size_t i;

    if (!profile || !requests || request_count == 0u ||
        request_count > CSB_V1_RUNTIME_CSBWIN_MOVEMENT_FILTER_CAP ||
        loaded_level < 0 || !out_filter || !out_adapter) {
        return 0;
    }
    csb_v1_runtime_init_csbwin_filter_candidate(&filter_candidate);
    memset(&adapter_candidate, 0, sizeof(adapter_candidate));
    memset(selected, 0, sizeof(selected));
    adapter_candidate.profile = profile;

    /* CSBWin Monster.cpp:3079-3176 caches a separate source filter for each
     * level. Stage every request before publishing so duplicate level slots,
     * unauthenticated actions, or ambiguous action ownership cannot expose a
     * partial callback surface. */
    for (i = 0u; i < request_count; ++i) {
        const CSB_V1_RuntimeDSAMovementFilterRequest *request = &requests[i];
        CSB_V1_CSBWinDSAFilterStackRunnerContext *runner =
            &adapter_candidate.runners[i];
        const CSB_V1_DSAImportedAction *action;
        int level = request->binding.location.level;
        size_t prior;

        if (level < 0 || level >=
            CSB_V1_RUNTIME_CSBWIN_MOVEMENT_FILTER_CAP ||
            filter_candidate.movement_filter_dsa_id[level] != -1 ||
            !csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
                profile, &request->binding, request->state_index,
                request->action_ordinal, request->master_location, runner)) {
            return 0;
        }
        action = csb_v1_chaos_find_imported_action(
            &profile->csbwin_extended_dsa_state, runner->dsa_id,
            runner->state_index, runner->action_ordinal);
        if (!action) return 0;
        for (prior = 0u; prior < i; ++prior) {
            if (selected[prior] == action) return 0;
        }
        selected[i] = action;
        filter_candidate.movement_filter_dsa_id[level] = request->binding.dsa_id;
        filter_candidate.movement_filter_state[level] = request->state_index;
        filter_candidate.movement_filter_action[level] = request->action_ordinal;
    }

    adapter_candidate.runner_count = (int)request_count;
    filter_candidate.programs = &profile->csbwin_extended_dsa_state;
    filter_candidate.runner =
        csb_v1_runtime_csbwin_movement_filter_stack_runner_callback;
    filter_candidate.runner_user = &adapter_candidate;
    filter_candidate.loaded_level = loaded_level;
    *out_adapter = adapter_candidate;
    filter_candidate.runner_user = out_adapter;
    *out_filter = filter_candidate;
    return 1;
}

int csb_v1_runtime_export_csbwin_core_save_to_memory(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_size)
{
    CSB_V1_CSBWin512BodyReport summary;

    if (!profile || !out || !out_size) {
        return -1;
    }
    *out_size = 0u;
    memset(&summary, 0, sizeof(summary));
    if (csb_v1_runtime_build_csbwin_core_summary(profile, &summary) != 0) {
        return -1;
    }
    return csb_v1_csbwin_512_build_writable_core_save(
        &summary,
        out,
        out_capacity,
        out_size) == CSB_V1_CSBWIN_512_OK ? 0 : -1;
}

int csb_v1_runtime_export_csbwin_core_save_to_path(
    const CSB_V1_RuntimeProfile *profile,
    const char *path)
{
    uint8_t bytes[CSB_V1_CSBWIN_BLOCK1_BYTES + 128u +
                  CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES * 16u +
                  3328u +
                  CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES * 16u +
                  CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES * 2u +
                  CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES];
    size_t size = 0u;
    FILE *fp;
    size_t wrote;

    if (!profile || !path || path[0] == '\0') {
        return -1;
    }
    if (csb_v1_runtime_export_csbwin_core_save_to_memory(
            profile, bytes, sizeof(bytes), &size) != 0) {
        return -1;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return -1;
    }
    wrote = fwrite(bytes, 1u, size, fp);
    if (fclose(fp) != 0) {
        return -1;
    }
    return wrote == size ? 0 : -1;
}

int csb_v1_runtime_set_leader(CSB_V1_RuntimeProfile *profile,
                              int champion_index)
{
    CSB_V1_Champion *champion;
    if (!profile || !profile->party_state_valid) return -1;
    if (champion_index == profile->leader_index) return 0;
    if (champion_index < 0) {
        profile->leader_index = -1;
        profile->party_state.LeaderIndex = -1;
        return 0;
    }
    if (champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    champion = &profile->party_state.Champions[champion_index];
    /* Mirrors the source-locked F0368 guard and selection side effect:
     * ignore dead/empty champions, then write G0411_i_LeaderIndex and align
     * the selected champion direction to G0308_i_PartyDirection.
     * Source: ReDMCSB CLIKCHAM.C F0368_COMMAND_SetLeader lines 51-68. */
    if (csb_v1_champion_is_dead(champion) || champion->CurrentHealth <= 0) {
        return -1;
    }
    profile->leader_index = champion_index;
    profile->party_state.LeaderIndex = champion_index;
    champion->Direction = (uint8_t)(profile->party_dir & 3);
    return 0;
}

int csb_v1_runtime_select_champion_portrait_render_source(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    CSB_V1_ChampionPortraitRenderSource *out_source)
{
    const CSB_V1_Champion *champion;
    if (out_source) {
        memset(out_source, 0, sizeof(*out_source));
        out_source->champion_index = -1;
    }
    if (!profile || !out_source || !profile->party_state_valid) return -1;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    champion = &profile->party_state.Champions[champion_index];
    /* ReDMCSB: PANEL.C F0354 lines 2195-2239 draws the status-box portrait
     * directly from M516_CHAMPIONS[ChampionIndex].Portrait.  Firestaff keeps
     * the same ownership boundary here: Utility Disk/CMP import may populate
     * CSB_V1_Champion.Portrait, and the renderer receives only a source view
     * into the runtime-owned champion snapshot. */
    out_source->portrait = champion->Portrait;
    out_source->portrait_byte_count = CSB_V1_PORTRAIT_BYTE_COUNT;
    out_source->portrait_width = CSB_V1_PORTRAIT_WIDTH;
    out_source->portrait_height = CSB_V1_PORTRAIT_HEIGHT;
    out_source->portrait_byte_width = CSB_V1_PORTRAIT_BYTE_WIDTH;
    out_source->champion_index = champion_index;
    out_source->is_leader = (champion_index == profile->leader_index);
    out_source->name = champion->Name;
    out_source->title = champion->Title;
    return 0;
}

int csb_v1_runtime_rotate_party(CSB_V1_RuntimeProfile *profile,
                                 int target_dir)
{
    /* Source: ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection lines
     * 117-130.  The PC 3.4 C version (MEDIA182) computes
     *   delta = (P0600_i_Direction - G0308_i_PartyDirection); if delta<0
     *   then delta += 4;
     * then loops over G0305_ui_PartyChampionCount champions, applying
     *   Champion.Cell      = (Champion.Cell + delta) & 3;
     *   Champion.Direction = (Champion.Direction + delta) & 3;
     * and finally writes G0308_i_PartyDirection = P0600_i_Direction.
     * The M021_NORMALIZE() macro is just (x & 3). */
    int delta;
    int current_dir;
    int i;

    /* ReDMCSB CHAMPION.C F0284 lines 117-130 touches only champion
     * Cell/Direction and G0308_i_PartyDirection; it never consults the
     * dungeon.  The c354907a5 startup-capture pass added a dungeon_handle
     * guard here as a drive-by, which broke the documented F0284 runtime
     * boundary (turn commands and teleporter rotation must succeed on the
     * bounded synthetic profile before any hash-verified dungeon exists).
     * Reverted 2026-07-18; keep the boundary source-faithful. */
    if (!profile) return -1;
    /* ReDMCSB CHAMPION.C F0284 always writes G0308_i_PartyDirection after
     * its bounded champion loop.  A new PC34 game reaches its first input
     * before a party has been imported, so ChampionCount may be zero here;
     * that must still rotate the dungeon-facing party direction rather than
     * rejecting the command behind Firestaff's party_state_valid marker. */
    if (target_dir < 0 || target_dir > 3) return -1;

    current_dir = profile->party_dir & 3;
    if (target_dir == current_dir) {
        /* Source-locked F0284 early return.  No champion state is
         * touched and the caller still gets 0. */
        return 0;
    }

    delta = target_dir - current_dir;
    if (delta < 0) delta += 4;

    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; i++) {
        uint8_t *cell = &profile->party_state.Champions[i].Cell;
        uint8_t *dir = &profile->party_state.Champions[i].Direction;
        *cell = (uint8_t)(((int)*cell + delta) & 3);
        *dir  = (uint8_t)(((int)*dir  + delta) & 3);
    }

    profile->party_dir = (uint8_t)target_dir;
    profile->party_state.PartyDirection = (uint8_t)target_dir;
    return 0;
}

int csb_v1_runtime_process_input_queue(
    CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction,
    CSB_V1_InputCommandRuntimeResult *out_result)
{
    CSB_V1_InputCommandRuntimeResult local_result;
    int target_dir;

    if (!profile || !queue) return -1;
    memset(&local_result, 0, sizeof(local_result));

    local_result.old_party_x = profile->party_x;
    local_result.old_party_y = profile->party_y;
    local_result.old_party_dir = profile->party_dir & 3;
    local_result.new_party_x = profile->party_x;
    local_result.new_party_y = profile->party_y;
    local_result.new_party_dir = profile->party_dir & 3;
    local_result.old_party_level = profile->current_level;
    local_result.new_party_level = profile->current_level;

    /* Source: ReDMCSB COMMAND.C F0380 lines 2045-2156 owns the command
     * queue dequeue/gate/dispatch boundary.  The shared V1 queue helper
     * preserves that C001/C002 vs C003-C006 split before this CSB runtime
     * adapter applies only the state transition that has a CSB profile
     * boundary today. */
    local_result.queue_result =
        DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
            queue,
            profile->party_dir,
            disabled_movement_ticks,
            projectile_disabled_movement_ticks,
            last_projectile_disabled_movement_direction);

    /* Publish every dequeue outcome, including the empty-queue C000 result,
     * so csb_v1_runtime_get_last_input_dispatch never reports a stale
     * dequeued command after the queue has drained (the pass680 queue
     * overflow contract).  The dispatch counter still advances only for
     * accepted commands. */
    profile->last_input_dispatch = local_result.queue_result;

    if (!local_result.queue_result.dequeued) {
        if (out_result) *out_result = local_result;
        return 0;
    }

    profile->input_dispatch_count++;

    switch (local_result.queue_result.command) {
    case DM1_V1_COMMAND_TURN_LEFT:
        if (csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL)) {
            csb_v1_runtime_take_current_stairs(profile, &local_result);
            csb_v1_runtime_mark_deferred_new_party_map_index(&local_result);
            csb_v1_runtime_apply_party_floor_sensor_consequences(
                profile,
                &local_result);
            break;
        }
        target_dir = (profile->party_dir + 3) & 3;
        local_result.sensor_source_remove_checked = 1;
        csb_v1_runtime_process_party_floor_sensors_at_level(
            profile,
            local_result.old_party_level,
            local_result.old_party_x,
            local_result.old_party_y,
            0,
            1,
            &local_result);
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        csb_v1_runtime_apply_party_turn_floor_sensor_add_consequences(
            profile,
            &local_result);
        break;
    case DM1_V1_COMMAND_TURN_RIGHT:
        if (csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL)) {
            csb_v1_runtime_take_current_stairs(profile, &local_result);
            csb_v1_runtime_mark_deferred_new_party_map_index(&local_result);
            csb_v1_runtime_apply_party_floor_sensor_consequences(
                profile,
                &local_result);
            break;
        }
        target_dir = (profile->party_dir + 1) & 3;
        local_result.sensor_source_remove_checked = 1;
        csb_v1_runtime_process_party_floor_sensors_at_level(
            profile,
            local_result.old_party_level,
            local_result.old_party_x,
            local_result.old_party_y,
            0,
            1,
            &local_result);
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        csb_v1_runtime_apply_party_turn_floor_sensor_add_consequences(
            profile,
            &local_result);
        break;
    case DM1_V1_COMMAND_MOVE_FORWARD:
    case DM1_V1_COMMAND_MOVE_RIGHT:
    case DM1_V1_COMMAND_MOVE_BACKWARD:
    case DM1_V1_COMMAND_MOVE_LEFT:
        {
            CSB_V1_MovementCommandStepRuntimeResultPc34Compat step_result;
            int step_status =
                (local_result.queue_result.command ==
                     DM1_V1_COMMAND_MOVE_BACKWARD &&
                 csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL))
                    ? 0
                    :
                csb_v1_movement_command_step_runtime_apply_pc34_compat(
                    profile,
                    local_result.queue_result.command,
                    csb_v1_runtime_party_destination_is_blocked,
                    NULL,
                    &step_result);
            if (step_status < 0) {
                return -1;
            }
            if (local_result.queue_result.command ==
                    DM1_V1_COMMAND_MOVE_BACKWARD &&
                csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL)) {
                local_result.movement_command_handled = 1;
                local_result.movement_step_attempted = 1;
                local_result.disabled_movement_ticks_after = 1;
                csb_v1_runtime_take_current_stairs(profile, &local_result);
            } else {
                local_result.unsupported_runtime_command =
                    step_result.unsupported_runtime_command;
                local_result.movement_command_handled =
                    step_result.command_handled;
                local_result.movement_step_attempted =
                    step_result.step_attempted;
                local_result.movement_step_applied = step_result.step_applied;
                local_result.movement_blocked_by_wall =
                    step_result.blocked_by_wall;
                local_result.movement_destination_x =
                    step_result.destination_x;
                local_result.movement_destination_y =
                    step_result.destination_y;
                local_result.disabled_movement_ticks_after =
                    step_result.disabled_movement_ticks_after;
                csb_v1_runtime_sample_destination_square(profile, &local_result);
                if (local_result.movement_blocked_by_wall) {
                    if (local_result.movement_destination_square_type == 4) {
                        local_result.movement_blocked_by_door = 1;
                    } else if (local_result.movement_destination_square_type == 6) {
                        local_result.movement_blocked_by_fakewall = 1;
                    } else if (profile->dungeon_handle &&
                               local_result.old_party_level >= 0 &&
                               csb_v1_runtime_square_has_group(
                                   (const CSB_V1_DungeonData *)profile->dungeon_handle,
                                   local_result.old_party_level,
                                   local_result.movement_destination_x,
                                   local_result.movement_destination_y)) {
                        local_result.movement_blocked_by_group = 1;
                        csb_v1_runtime_schedule_party_bump_group_reaction(
                            profile,
                            local_result.old_party_level,
                            local_result.movement_destination_x,
                            local_result.movement_destination_y,
                            &local_result);
                    }
                }
                csb_v1_runtime_apply_destination_chain(profile, &local_result);
                if (local_result.movement_step_applied &&
                    local_result.movement_destination_square_type ==
                        DM1_SQUARE_STAIRS) {
                    /* MOVESENS.C F0267 reaches F0276 for the entered C03
                     * square before CLIKMENU.C F0366 hands that party to
                     * F0364.  C005 must therefore see the source stair
                     * square, not the post-transition level. */
                    local_result.sensor_destination_add_checked = 1;
                    csb_v1_runtime_process_party_floor_sensors_at_level(
                        profile,
                        local_result.old_party_level,
                        local_result.movement_destination_x,
                        local_result.movement_destination_y,
                        1,
                        0,
                        &local_result);
                }
                csb_v1_runtime_apply_destination_stairs(profile, &local_result);
            }
            csb_v1_runtime_mark_deferred_new_party_map_index(&local_result);
            csb_v1_runtime_apply_party_floor_sensor_consequences(
                profile,
                &local_result);
        }
        break;
    default:
        local_result.unsupported_runtime_command = 1;
        break;
    }

    local_result.new_party_x = profile->party_x;
    local_result.new_party_y = profile->party_y;
    local_result.new_party_dir = profile->party_dir & 3;
    local_result.runtime_state_changed =
        (local_result.old_party_x != local_result.new_party_x) ||
        (local_result.old_party_y != local_result.new_party_y) ||
        (local_result.old_party_dir != local_result.new_party_dir) ||
        (local_result.teleporter_transition_applied != 0) ||
        (local_result.stair_transition_applied != 0) ||
        (local_result.pit_fall_applied != 0) ||
        (local_result.sensor_event_count != 0);

    if (out_result) *out_result = local_result;
    return 1;
}

void csb_v1_runtime_cleanup(CSB_V1_RuntimeProfile *profile) {
    if (!profile) return;
    /*
     * Unload the dungeon loaded by csb_v1_runtime_boot().
     * csb_v1_dungeon_unload() frees the dungeon data (raw_data,
     * dsa_offsets) via csb_v1_dungeon_free() and clears s_current_dungeon,
     * resetting dungeon-layer accessors to ENDOF.
     *
     * FIX (pass608): dungeon is now heap-allocated in csb_v1_runtime_boot()
     * and profile->dungeon_handle owns the allocation.  After calling
     * csb_v1_dungeon_unload() we also free(profile->dungeon_handle) to
     * release the heap struct and NULL the pointer.
     *
     * csb_v1_dungeon_free() does NOT free the struct itself (only inner
     * pointers), so free(dungeon_handle) is safe after csb_v1_dungeon_unload().
     */
    csb_v1_dungeon_unload();
    if (profile->dungeon_handle) {
        free(profile->dungeon_handle);
        profile->dungeon_handle = NULL;
    }
    csb_v1_skin_cache_cleanup(&profile->skin_cache);
    csb_v1_runtime_cleanup_csbwin_extended_state(profile);
}


int csb_v1_runtime_boot(CSB_V1_RuntimeProfile *profile,
                          const char *data_dir,
                          const char *version_hint)
{
    CSB_V1_AssetResult dun_result;
    CSB_V1_AssetResult gfx_result;
    const char *dun_path;
    const char *gfx_path;
    const char *search_dir;

    if (!profile) return -1;

    search_dir = data_dir ? data_dir : ".",

    /* Step 1: Find dungeon by CSB hash (ReDMCSB DUNGEON.C F0237) */
    dun_path = csb_v1_runtime_find_dungeon(search_dir, &dun_result);
    if (!dun_path) return -1;
    profile->dungeon_path = dun_path;
    profile->dungeon_asset = dun_result;

    /* Step 1b: Load the dungeon data (CSB V1 Phase 2 — real asset ingestion)
     * Uses csb_v1_dungeon_load_from_file() to read the actual DUNGEON.DAT
     * into the current dungeon context.  Dungeon-layer accessors in
     * csb_v1_dungeon_world_pc34_compat.h become live after this.
     *
     * FIX (pass608): dungeon MUST be heap-allocated.  The previous
     * implementation created a stack-local CSB_V1_DungeonData and passed
     * &dungeon to csb_v1_dungeon_set_current().  After boot() returns,
     * the stack variable goes out of scope and s_current_dungeon becomes
     * a dangling pointer.  This is a critical memory safety bug.
     *
     * The dungeon_handle field in CSB_V1_RuntimeProfile owns the heap
     * allocation.  csb_v1_dungeon_set_current() transfers inner-data
     * ownership (raw_data/dsa_offsets) but the struct itself is freed
     * by the profile in csb_v1_runtime_cleanup().
     *
     * Source: CSBWin/CSBCode.cpp LoadDungeon lines 6800-6950 */
    {
        /* Heap-allocate to avoid dangling pointer in s_current_dungeon */
        CSB_V1_DungeonData *dungeon = calloc(1, sizeof(CSB_V1_DungeonData));
        if (!dungeon) {
            /* Fall through — dungeon-layer accessors return ENDOF */
        } else if (csb_v1_dungeon_load_from_file(dungeon, dun_path) == 0) {
            profile->dungeon_handle = dungeon;
            csb_v1_dungeon_set_current(dungeon); /* singleton now points to heap */
            csb_v1_dungeon_set_current_level(0);   /* start at level 0 */
            if (profile->party_state_valid) {
                (void)csb_v1_runtime_recompute_party_loads_pc34_compat(profile);
            }
        } else {
            free(dungeon);
            profile->dungeon_handle = NULL;
        }
        /* If load fails (corrupt/missing file), boot continues without dungeon.
         * Dungeon-layer accessors will return ENDOF until a dungeon is loaded. */
    }

    /* Step 2: Find graphics (ReDMCSB DISK.C / CSBWin AssetCache) */
    gfx_path = csb_v1_runtime_find_graphics(search_dir, version_hint, &gfx_result);
    profile->graphics_path = gfx_path ? gfx_path : "";
    profile->graphics_asset = gfx_result;

    /* Step 3: Detect variant from asset hashes */
    profile->variant_id = csb_v1_runtime_detect_variant(
        gfx_path, dun_path, NULL, NULL);

    /* Step 4: Initialize Chaos Magic spell grid (ReDMCSB CASTER.C F0211) */
    profile->chaos_magic.magic_initialized = 1;
    profile->chaos_magic.spell_grid_version = 0;
    profile->chaos_magic.chaos_level = 0;

    /* Step 5: Set initial state to TITLE (ReDMCSB ENTRANCE.C G0298) */
    profile->state = CSB_STATE_TITLE;

    return 0;
}

void csb_v1_runtime_tick(CSB_V1_RuntimeProfile *profile, uint32_t dt_ms)
{
    if (!profile || profile->paused) return;
    if (profile->game_over || profile->victory) return;

    profile->total_play_ms += dt_ms;

    /* The original runtime gates event expiry against G0313_ul_GameTime,
     * not against a single frame delta.  Accumulate wall time first, then
     * fire every due 55ms quantum so common frame slices such as 16+16+23ms
     * still produce one V1 tick.
     * Source: ReDMCSB TIMELINE.C F0235 lines 702-708
     * Source: ReDMCSB COMMAND.C F0380 lines 2383-2429
     * (C147/C148 toggle G0301_B_GameTimeTicking). */
    while (csb_v1_runtime_tick_due(profile, 0U)) {
        csb_v1_fire_tick(profile);
    }
}

int csb_v1_runtime_tick_v1(CSB_V1_RuntimeProfile *profile)
{
    if (!profile || profile->paused) return 0;
    if (profile->game_over || profile->victory) return 0;

    profile->total_play_ms += CSB_V1_TICK_MS_NOMINAL;
    csb_v1_fire_tick(profile);
    return 1;
}

int csb_v1_runtime_f0240_is_first_event_expired(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_F0240_FirstEventExpiredReceipt *out_receipt)
{
    const struct DM1_EventQueue_V1 *queue;
    int event_index;
    const struct DM1_Event_V1 *event;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile) {
        out_receipt->status = "missing-profile";
        return 0;
    }

    queue = &profile->timeline_queue;
    out_receipt->game_time = profile->game_time & 0x00ffffffu;
    out_receipt->event_count = queue->eventCount;
    if (queue->eventCount <= 0) {
        out_receipt->valid = 1;
        out_receipt->first_event_index = -1;
        out_receipt->status = "empty-timeline";
        return 1;
    }
    if (queue->eventCount > DM1_EVENT_MAX_COUNT ||
        queue->maxEvents <= 0 || queue->maxEvents > DM1_EVENT_MAX_COUNT) {
        out_receipt->status = "malformed-timeline-count";
        return 0;
    }
    event_index = queue->timeline[0];
    out_receipt->first_event_index = event_index;
    if (event_index < 0 || event_index >= queue->maxEvents ||
        event_index >= DM1_EVENT_MAX_COUNT) {
        out_receipt->status = "malformed-first-event-index";
        return 0;
    }
    event = &queue->events[event_index];
    if (event->type == DM1_EVENT_NONE) {
        out_receipt->status = "malformed-empty-first-event";
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->first_event_type = event->type;
    out_receipt->first_event_time = DM1_MAP_TIME_TIME(event->map_time);
    out_receipt->expired =
        out_receipt->first_event_time <= out_receipt->game_time;
    out_receipt->status = out_receipt->expired
        ? "expired-first-event"
        : "waiting-first-event";
    return 1;
}

int csb_v1_runtime_f0261_process_tick(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_F0261_ProcessTickReceipt *out_receipt)
{
    const struct DM1_EventQueue_V1 *queue;
    int event_index = -1;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->first_event_index = -1;
    if (!profile) {
        out_receipt->status = "missing-profile";
        return 0;
    }
    if (profile->paused || profile->game_over || profile->victory) {
        out_receipt->status = "tick-blocked";
        return 0;
    }

    queue = &profile->timeline_queue;
    out_receipt->pre_event_count = queue->eventCount;
    out_receipt->game_time_before = profile->game_time & 0x00ffffffu;
    out_receipt->timeline_dispatch_count_before =
        profile->timeline_dispatch_count;
    if (queue->eventCount < 0 ||
        queue->eventCount > DM1_EVENT_MAX_COUNT ||
        queue->maxEvents <= 0 ||
        queue->maxEvents > DM1_EVENT_MAX_COUNT) {
        out_receipt->status = "malformed-timeline-count";
        return 0;
    }
    if (queue->eventCount > 0) {
        event_index = queue->timeline[0];
        out_receipt->first_event_index = event_index;
        if (event_index < 0 || event_index >= queue->maxEvents ||
            event_index >= DM1_EVENT_MAX_COUNT) {
            out_receipt->status = "malformed-first-event-index";
            return 0;
        }
        out_receipt->first_event_type = queue->events[event_index].type;
        out_receipt->first_event_time =
            DM1_MAP_TIME_TIME(queue->events[event_index].map_time);
        if (out_receipt->first_event_type == DM1_EVENT_NONE) {
            out_receipt->status = "malformed-empty-first-event";
            return 0;
        }
    }

    out_receipt->tick_fired = csb_v1_runtime_tick_v1(profile);
    out_receipt->game_time_after = profile->game_time & 0x00ffffffu;
    out_receipt->post_event_count = profile->timeline_queue.eventCount;
    out_receipt->timeline_dispatch_count_after =
        profile->timeline_dispatch_count;
    out_receipt->dispatched_count =
        (int)(out_receipt->timeline_dispatch_count_after -
              out_receipt->timeline_dispatch_count_before);
    out_receipt->valid = out_receipt->tick_fired == 1;
    if (!out_receipt->valid) {
        out_receipt->status = "tick-not-fired";
        return 0;
    }
    out_receipt->status = out_receipt->dispatched_count > 0
        ? "processed-expired-events"
        : "processed-no-expired-events";
    return 1;
}

int csb_v1_runtime_tick_due(const CSB_V1_RuntimeProfile *profile, uint32_t now_ms)
{
    uint64_t wall_ms;
    uint64_t game_ticks_now;
    if (!profile) return 0;

    wall_ms = (now_ms != 0U) ? (uint64_t)now_ms : profile->total_play_ms;
    game_ticks_now = wall_ms / CSB_V1_TICK_MS_NOMINAL;
    return (profile->tick_count < game_ticks_now) ? 1 : 0;
}

/* ── Source evidence ────────────────────────────────────────────────── */

const char *csb_v1_runtime_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C: F0806_F0806_ENTRANCE_int (game boot)\n"
        "ReDMCSB ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)\n"
        "ReDMCSB ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics)\n"
        "ReDMCSB ENTRANCE.C: F0580_ENTRANCE_DrawDoorAnimationStep\n"
        "ReDMCSB ENTRANCE.C: F0581_ENTRANCE_BlitDoors\n"
        "ReDMCSB ENTRANCE.C: C28_ENTRANCE_CSB palette index\n"
        "ReDMCSB ENTRANCE.C: G0298_B_NewGame state machine control\n"
        "ReDMCSB ENTRANCE.C: G0309_i_PartyMapIndex init (party start)\n"
        "ReDMCSB ENTRANCE.C: MEDIA529_F20E_F20J save path decision\n"
        "ReDMCSB ENTRANCE.C: M567_COMMAND_ENTRANCE_DRAW_CREDITS\n"
        "ReDMCSB SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful\n"
        "ReDMCSB SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful\n"
        "ReDMCSB LOADSAVE.C: F0435_STARTEND_LoadGame\n"
        "ReDMCSB LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame\n"
        "ReDMCSB DUNGEON.C: F0237_DUNGEON_DungeonLoad (hash-gated load)\n"
        "ReDMCSB CASTER.C: F0211_CASTER_ClearSpellEffects (spell grid boot)\n"
        "ReDMCSB PROJEXPL.C: F0213_EXPLOSION_Create per-square invocation slots\n"
        "ReDMCSB PROJEXPL.C: F0220_EXPLOSION_ProcessEvent25_Explosion C25 advance\n"
        "ReDMCSB BugsAndChanges.htm: CHANGE7_29 (new CSBGAME.DAT header)\n"
        "ReDMCSB CEDTINC7.C: G3764_THAT_S_THE_CSB_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTIL_DISK\n"
        "ReDMCSB CEDTINC8.C: G3921/G3755/G3764 utility disk strings\n"
        "ReDMCSB F0417: F0417_SAVEUTIL_GetChecksumAndObfuscate\n"
        "ReDMCSB COMPILE.H MEDIA332 (S20E/S21E Atari ST 2.0/2.1)\n"
        "ReDMCSB COMPILE.H MEDIA529 (A35E/A35M Amiga 3.5)\n"
        "ReDMCSB COMPILE.H MEDIA278 (P20JA/P20JB PC DOS 3.4)\n"
        "ReDMCSB COMPILE.H MEDIA278_I34E_I34M (PC DOS multilanguage)\n"
        "CSBWin SaveGame.cpp: LoadGame() / SaveGame() (2953 lines)\n"
        "CSBWin Character.cpp: Character::import_dm1_save()\n"
        "CSBWin Magic.cpp: ChaosMagic namespace\n"
        "CSBWin AssetCache: variant_id mapping for all platforms\n"
        "asset_status_m12.c: g_csbVersions[] MD5 table (all 4 variants)\n"
        "asset_find_by_hash.c: hash-based asset discovery API\n"
        "\n"
        "CSB vs DM1 runtime differences:\n"
        "  - Dungeon hash: 6695d2acebce49f95db1d8f3a5c733de (CSB)\n"
        "  - Save namespace: csb_save_N.fsav (CSB) vs save_NN.dat (DM1)\n"
        "  - Save header: CSB_MAGIC 0x43534201 (CSB) vs DM_MAGIC 0x444D0001\n"
        "  - Save key index: C29 (CSB) vs C10 (DM1) per MEDIA187/MEDIA332\n"
        "  - Difficulty scale: +25% per champion (CSB) vs DM1 flat\n"
        "  - Chaos Magic: present at CSB boot (F0211)\n"
        "  - Entry: same ENTRANCE, C28_ENTRANCE_CSB palette\n"
        "  - Champion import: F0153 from DM1 save supported at CSB boot\n";
}
