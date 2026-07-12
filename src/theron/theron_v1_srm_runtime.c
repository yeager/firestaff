#include "theron_v1_srm_runtime.h"

#include "theron_v1_startup_media.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define TQR_SRM_PUBLISH_NO_REPLACE(temp_path, destination_path) \
    (CreateHardLinkA((destination_path), (temp_path), NULL) ? 1 : 0)
#else
#include <unistd.h>
#define TQR_SRM_PUBLISH_NO_REPLACE(temp_path, destination_path) \
    (link((temp_path), (destination_path)) == 0)
#endif

#if FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

#define TQR_SRM_PROGRESS_BYTES 44u
#define TQR_SRM_RECORD_BYTES 40u
#define TQR_SRM_PAYLOAD_BYTES (TQR_SRM_PROGRESS_BYTES + 4u + \
    ((size_t)THERON_MAX_CHAMPIONS * TQR_SRM_RECORD_BYTES))

static void wr16le(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void wr32le(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

static uint8_t clamp_byte(int value) {
    return (uint8_t)(value < 0 ? 0 : (value > 255 ? 255 : value));
}

static void runtime_receipt_init(Theron_V1SrmRuntimeReceipt *receipt) {
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = THERON_V1_SRM_RUNTIME_BAD_INPUT;
    receipt->envelope_kind = THERON_V1_SRM_ENVELOPE_KIND_NONE;
    receipt->import_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
}

static void build_party_payload(const Theron_V1_World *world, uint8_t payload[TQR_SRM_PAYLOAD_BYTES]) {
    int i;
    memset(payload, 0, TQR_SRM_PAYLOAD_BYTES);
    memcpy(payload, "FSTQPTY1", 8u);
    payload[8] = 1u;
    payload[9] = (uint8_t)world->progression.current_dungeon;
    payload[10] = world->progression.quest_items_collected;
    payload[11] = world->progression.current_level;
    wr32le(payload + 12u, world->progression.dungeon_playtime_seconds);
    for (i = 0; i < THERON_DUNGEON_COUNT; ++i) wr32le(payload + 16u + (size_t)i * 4u, world->progression.dungeon_seeds[i]);
    wr32le(payload + TQR_SRM_PROGRESS_BYTES, world->party.gold);
    for (i = 0; i < THERON_MAX_CHAMPIONS; ++i) {
        const Theron_V1_Champion *c = &world->party.champions[i];
        uint8_t *r = payload + TQR_SRM_PROGRESS_BYTES + 4u + (size_t)i * TQR_SRM_RECORD_BYTES;
        memcpy(r, c->name, strlen(c->name) < 16u ? strlen(c->name) : 16u);
        r[16] = (uint8_t)c->primary_class; r[17] = c->alive;
        r[18] = clamp_byte(c->health); r[19] = clamp_byte(c->max_health);
        r[20] = clamp_byte(c->stamina); r[21] = clamp_byte(c->max_stamina);
        r[22] = clamp_byte(c->mana); r[23] = clamp_byte(c->max_mana);
        r[24] = clamp_byte(c->strength); r[25] = clamp_byte(c->dexterity);
        r[26] = clamp_byte(c->wisdom); r[27] = clamp_byte(c->vitality);
        r[28] = clamp_byte(c->anti_magic); r[29] = clamp_byte(c->anti_fire);
        r[30] = clamp_byte(c->fighter_level); r[31] = clamp_byte(c->ninja_level);
        r[32] = clamp_byte(c->priest_level); r[33] = clamp_byte(c->wizard_level);
        r[34] = c->wounds; wr16le(r + 35u, c->attributes);
        r[37] = clamp_byte(c->food); r[38] = clamp_byte(c->water);
    }
}

static void reset_world_runtime(Theron_V1_World *world) {
    world->current_dungeon = (int)world->progression.current_dungeon;
    world->current_level = (int)world->progression.current_level - 1;
    world->object_count = 0; world->timer_count = 0; world->transition_pending = 0;
    world->quest_items_in_dungeon = world->progression.quest_items_in_current_dungeon;
    world->dungeon_complete = 0; memset(world->level_loaded, 0, sizeof(world->level_loaded));
}

static void fill_receipt(Theron_V1SrmRuntimeReceipt *out, Theron_V1SrmRuntimeStatus status,
                         Theron_V1SrmEnvelopeKind kind, Theron_V1SrmProgressImportStatus import_status,
                         const Theron_V1_World *world) {
    if (!out) return;
    out->status = status; out->envelope_kind = kind; out->import_status = import_status;
    if (!world) return;
    out->dungeon = world->progression.current_dungeon; out->level = world->progression.current_level;
    out->quest_mask = world->progression.quest_items_collected;
    out->champion_count = (uint8_t)world->party.champion_count; out->party_gold = world->party.gold;
    out->track02_identity = world->runtime_media.identity;
    out->track02_media_route_mask = world->runtime_media.route_mask;
    out->track02_media_checksum = world->runtime_media.checksum;
    out->track02_level_bank = world->runtime_media.level_bank;
}

static int srm_path_exists(const char *path) {
    FILE *fp;
    if (!path || !path[0]) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

Theron_V1SrmRuntimeStatus theron_v1_srm_runtime_export_path(const Theron_V1_World *world, const char *path, Theron_V1SrmRuntimeReceipt *out) {
    runtime_receipt_init(out);
    if (!world || !path || !path[0]) return THERON_V1_SRM_RUNTIME_BAD_INPUT;
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[TQR_SRM_PAYLOAD_BYTES];
    uint8_t compressed[TQR_SRM_PAYLOAD_BYTES + 96u];
    uint8_t gzip[TQR_SRM_PAYLOAD_BYTES + 114u];
    z_stream zs; FILE *fp; size_t compressed_size; char temp[THERON_V1_SRM_PATH_MAX + 8u];
    build_party_payload(world, payload); memset(&zs, 0, sizeof(zs));
    zs.next_in = payload; zs.avail_in = (uInt)sizeof(payload); zs.next_out = compressed; zs.avail_out = (uInt)sizeof(compressed);
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) return THERON_V1_SRM_RUNTIME_IO_FAILED;
    if (deflate(&zs, Z_FINISH) != Z_STREAM_END) { deflateEnd(&zs); return THERON_V1_SRM_RUNTIME_IO_FAILED; }
    compressed_size = (size_t)zs.total_out; deflateEnd(&zs);
    gzip[0] = THERON_V1_SRM_GZIP_MAGIC_0; gzip[1] = THERON_V1_SRM_GZIP_MAGIC_1;
    gzip[2] = THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE; gzip[3] = 0u;
    memset(gzip + 4u, 0, 4u); gzip[8] = 2u; gzip[9] = 255u;
    memcpy(gzip + 10u, compressed, compressed_size);
    wr32le(gzip + 10u + compressed_size, (uint32_t)crc32(crc32(0L, Z_NULL, 0), payload, (uInt)sizeof(payload)));
    wr32le(gzip + 14u + compressed_size, (uint32_t)sizeof(payload));
    compressed_size += 18u;
    snprintf(temp, sizeof(temp), "%s.tmp", path); fp = fopen(temp, "wb");
    if (!fp) return THERON_V1_SRM_RUNTIME_IO_FAILED;
    if (fwrite(gzip, 1u, compressed_size, fp) != compressed_size) {
        fclose(fp);
        remove(temp);
        return THERON_V1_SRM_RUNTIME_IO_FAILED;
    }
    if (fclose(fp) != 0) {
        remove(temp);
        return THERON_V1_SRM_RUNTIME_IO_FAILED;
    }
    /* `rename` would silently replace a real Save Disk.  Link the fully
     * written private file into place instead: the publication either makes
     * a new destination visible or leaves the existing corpus artifact
     * untouched. */
    if (!TQR_SRM_PUBLISH_NO_REPLACE(temp, path)) {
        int destination_exists = srm_path_exists(path);
        Theron_V1SrmRuntimeStatus publish_status = destination_exists
            ? THERON_V1_SRM_RUNTIME_DESTINATION_EXISTS
            : THERON_V1_SRM_RUNTIME_IO_FAILED;
        remove(temp);
        fill_receipt(out, publish_status, THERON_V1_SRM_ENVELOPE_KIND_NONE,
                     THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT, world);
        return publish_status;
    }
    remove(temp);
    if (out) out->srm_size = compressed_size;
    fill_receipt(out, THERON_V1_SRM_RUNTIME_OK, THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY, THERON_V1_SRM_PROGRESS_IMPORT_OK, world);
    return THERON_V1_SRM_RUNTIME_OK;
#else
    (void)path; return THERON_V1_SRM_RUNTIME_ZLIB_UNAVAILABLE;
#endif
}

Theron_V1SrmRuntimeStatus theron_v1_srm_runtime_continue_path(Theron_V1_World *world, const char *path, const uint8_t *track, size_t track_size, const char *md5, Theron_V1SrmRuntimeReceipt *out) {
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES]; Theron_V1SrmEnvelopeReceipt envelope;
    Theron_DungeonProgression progression; Theron_V1_Party party; Theron_V1SrmPartyImportReceipt party_receipt;
    Theron_StartupMediaStateReceipt media_receipt;
    Theron_V1_World staged_world;
    Theron_V1SrmEnvelopeKind kind;
    runtime_receipt_init(out);
    if (!world || !path || !path[0] || !track || !track_size || !md5 || !md5[0]) return THERON_V1_SRM_RUNTIME_BAD_INPUT;
    memset(scratch, 0, sizeof(scratch)); memset(&envelope, 0, sizeof(envelope));
    kind = theron_v1_srm_decode_path(path, -1, scratch, sizeof(scratch), &envelope);
    if (kind != THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY || !envelope.progression.restored || !envelope.party.restored) { fill_receipt(out, THERON_V1_SRM_RUNTIME_UNSUPPORTED_BODY, kind, envelope.decode_status, NULL); return THERON_V1_SRM_RUNTIME_UNSUPPORTED_BODY; }
    if (theron_v1_srm_decode_progression_party_payload(scratch, envelope.inflate_payload_size, &progression, &party, &party_receipt) != THERON_V1_SRM_PROGRESS_IMPORT_OK) { fill_receipt(out, THERON_V1_SRM_RUNTIME_UNSUPPORTED_BODY, kind, envelope.decode_status, NULL); return THERON_V1_SRM_RUNTIME_UNSUPPORTED_BODY; }
    /* A verified identity alone is not drawable media.  Continue must bind
     * all four original Track 02 startup surfaces before it can commit a
     * restored world; otherwise a direct caller could resume into fallback
     * visuals despite a real Track 02 request. */
    theron_v1_startup_media_capture_track02_state_receipt(
        track, track_size, md5, &media_receipt);
    if (!theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            &media_receipt)) {
        fill_receipt(out, THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED, kind,
                     envelope.decode_status, NULL);
        return THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED;
    }
    staged_world = *world;
    staged_world.progression = progression;
    staged_world.party = party;
    reset_world_runtime(&staged_world);
    if (!theron_v1_startup_media_bind_runtime_receipt(
            &staged_world, &media_receipt) ||
        !theron_v1_world_runtime_media_select_level_bank(
            &staged_world, THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME,
            staged_world.progression.current_dungeon,
            staged_world.current_level)) {
        fill_receipt(out, THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED, kind,
                     envelope.decode_status, NULL);
        return THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED;
    }
    *world = staged_world;
    if (out) out->srm_size = (size_t)envelope.file_size;
    fill_receipt(out, THERON_V1_SRM_RUNTIME_OK, kind, envelope.decode_status, world);
    return THERON_V1_SRM_RUNTIME_OK;
}

const char *theron_v1_srm_runtime_status_name(Theron_V1SrmRuntimeStatus status) {
    switch (status) { case THERON_V1_SRM_RUNTIME_OK: return "OK"; case THERON_V1_SRM_RUNTIME_ZLIB_UNAVAILABLE: return "ZLIB_UNAVAILABLE"; case THERON_V1_SRM_RUNTIME_BAD_INPUT: return "BAD_INPUT"; case THERON_V1_SRM_RUNTIME_IO_FAILED: return "IO_FAILED"; case THERON_V1_SRM_RUNTIME_UNSUPPORTED_BODY: return "UNSUPPORTED_BODY"; case THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED: return "MEDIA_UNVERIFIED"; case THERON_V1_SRM_RUNTIME_DESTINATION_EXISTS: return "DESTINATION_EXISTS"; default: return "UNKNOWN"; }
}
