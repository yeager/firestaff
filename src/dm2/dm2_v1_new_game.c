/*
 * dm2_v1_new_game.c — DM2 V1 New Game & Session Management
 *
 * Phase 6: Utility/import flow — DM2-specific load/start flow.
 *
 * Implements source-owned new-game/load boundaries and session save/load.
 *
 * Source locks (ReDMCSB WIP20210206):
 *   CHAMPION.C F0280  — CHAMPION_AddCandidateChampionToParty:
 *     portrait-to-squad-position assignment, portrait blit from
 *     GRAPHICS.DAT, initial attribute loop (lines 63-170).
 *     MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD |
 *     MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX
 *     cleared by M009_CLEAR before champion init.
 *   CHAMPRST.C F0278  — CHAMPION_ResetDataToStartGame:
 *     clears G4055_s_LeaderHandObject, G0415_ui_LeaderEmptyHanded,
 *     clears all champion Attributes masks.
 *   CHAMPION.C G0417_apc_BaseSkillNames[4] — class names.
 *   docs/dm2_party_state.md — 261-byte champion record, SUPPRESS mask,
 *     portrait→class mapping, initial HP/stamina/mana values.
 *   docs/dm2_save_format.md — session serialization, slot header layout.
 *   SKULL.ASM T520  — party_placement: Hall of Champions (mapX=15,mapY=15,N).
 *   SKULL.ASM T560  — DUNGEON_Load completion, game state init.
 *   SKULL.ASM T048  — input dispatch / new game gate.
 */

#include "dm2_v1_new_game.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum {
    DM2_RAW_DUNGEON_HEADER_SIZE = 44,
    DM2_RAW_MAP_DESC_SIZE = 16,
    DM2_RAW_THING_TYPE_COUNT = 16
};

static const int s_dm2_raw_db_record_size[DM2_RAW_THING_TYPE_COUNT] = {
    0x04, 0x06, 0x04, 0x08, 0x10, 0x04, 0x04, 0x04,
    0x04, 0x08, 0x04, 0x00, 0x00, 0x00, 0x08, 0x04
};

/* ════════════════════════════════════════════════════════════════
 * Session validation
 * Source: Phase 6 internal consistency checks
 * ════════════════════════════════════════════════════════════════ */

bool dm2_v1_session_validate(const DM2_V1_SessionState *session)
{
    if (!session) return false;

    /* Champion count must be 0-4 */
    if (session->champion_count > 4) return false;

    /* Leader must be valid index */
    if (session->leader_index >= 4) return false;

    /* Party position must be valid */
    if (session->party_x > 63 || session->party_y > 63) return false;
    if (session->party_dir > 3) return false;

    /* Time of day */
    if (session->time_of_day_minutes >= 1440) return false;

    /* Gold should not be negative */
    if (session->gold > 4000000000u) return false;

    /* Bounded WRITE_MINION_ASSOC startup copy. */
    if (session->original_minions.count > DM2_MAX_MINIONS) return false;

    /* Champion records must have non-zero name */
    for (uint8_t i = 0; i < session->champion_count; i++) {
        const DM2_ChampionRecord *rec =
            (const DM2_ChampionRecord *)session->champion_data[i];
        /* At least one character in the first name */
        if (rec->first_name[0] == '\0') return false;
    }

    return true;
}

/* ════════════════════════════════════════════════════════════════
 * Session serialization
 * Source: docs/dm2_save_format.md — session state serialization
 * ════════════════════════════════════════════════════════════════ */

/*
 * Session format on disk (DM2 slot data section):
 *   Offset  Size  Field
 *   ──────  ────  ─────
 *   0       4     game_tick (LE uint32)
 *   4       4     rng_seed (LE uint32)
 *   8       2     champion_count (LE uint16)
 *   10      1     leader_index (uint8)
 *   11      2     party_x (LE uint16)
 *   13      2     party_y (LE uint16)
 *   15      1     party_dir (uint8)
 *   16      1     party_level (uint8)
 *   17      1     outdoor_mode (uint8)
 *   18      2     time_of_day_minutes (LE uint16)
 *   20      2     gold (LE uint16)
 *   22      2     reputation (LE int16)
 *   24      1     rain_intensity (uint8)
 *   25      1     weather_padding (uint8)
 *   26      1     session_version (uint8, =DM2_SESSION_VERSION)
 *   27      235   reserved
 *   29      261   champion[0] record
 *   290     261   champion[1] record
 *   551     261   champion[2] record
 *   812     261   champion[3] record
 *   1073    8     original_global_flags
 *   1081    64    original_global_bytes
 *   1145    128   original_global_words
 *   1273    6     original_spell_effects
 *   1279    1     original_timer_count
 *   1280    320   original_timers[32]
 *   1600    4     original_leader_hand_object
 *   1604    1     original_minions.count
 *   1605    128   original_minions.entries[16] (object_id + owner_champion)
 *   ──────  ────
 *   Total:  1733 bytes
 *
 * Champion records are stored raw (261 bytes each, not SUPPRESS-encoded
 * at this level — the SUPPRESS encoding is done per-field by the
 * dm2_suppress_encode/decode_champion functions when needed).
 *
 * This matches the in-memory DM2_V1_SessionState layout, so we use
 * a direct memory copy for the serialization (zero-copy).
 */

int dm2_v1_session_serialize(const DM2_V1_SessionState *session,
                               uint8_t *buf, size_t buf_size)
{
    if (!session || !buf) return -1;
    enum {
        DM2_SESSION_BASE_SERIALIZED_SIZE = 29 + 4 * 261,
        DM2_SESSION_SERIALIZED_SIZE =
            DM2_SESSION_BASE_SERIALIZED_SIZE +
            DM2_GLOBAL_FLAGS_SIZE +
            DM2_GLOBAL_BYTES_SIZE +
        DM2_GLOBAL_WORDS_SIZE * 2 +
        DM2_GLOBAL_SPELL_EFFECTS_SIZE +
        1 +
        DM2_MAX_TIMERS * DM2_TIMER_ENTRY_SIZE +
        4 +
        1 +
        DM2_MAX_MINIONS * 8
    };
    uint8_t *ext;
    if (buf_size < (size_t)DM2_SESSION_SERIALIZED_SIZE) return -1;
    memset(buf, 0, (size_t)DM2_SESSION_SERIALIZED_SIZE);

    /* Write header */
    uint8_t *p = buf;
    p[0] = (uint8_t)(session->game_tick & 0xFF);
    p[1] = (uint8_t)((session->game_tick >> 8) & 0xFF);
    p[2] = (uint8_t)((session->game_tick >> 16) & 0xFF);
    p[3] = (uint8_t)((session->game_tick >> 24) & 0xFF);

    p[4] = (uint8_t)(session->rng_seed & 0xFF);
    p[5] = (uint8_t)((session->rng_seed >> 8) & 0xFF);
    p[6] = (uint8_t)((session->rng_seed >> 16) & 0xFF);
    p[7] = (uint8_t)((session->rng_seed >> 24) & 0xFF);

    p[8]  = (uint8_t)(session->champion_count & 0xFF);
    p[9]  = (uint8_t)((session->champion_count >> 8) & 0xFF);
    p[10] = session->leader_index;
    p[11] = (uint8_t)(session->party_x & 0xFF);
    p[12] = (uint8_t)((session->party_x >> 8) & 0xFF);
    p[13] = (uint8_t)(session->party_y & 0xFF);
    p[14] = (uint8_t)((session->party_y >> 8) & 0xFF);
    p[15] = session->party_dir;
    p[16] = session->party_level;
    p[17] = session->outdoor_mode;
    p[18] = (uint8_t)(session->time_of_day_minutes & 0xFF);
    p[19] = (uint8_t)((session->time_of_day_minutes >> 8) & 0xFF);
    p[20] = (uint8_t)(session->gold & 0xFF);
    p[21] = (uint8_t)((session->gold >> 8) & 0xFF);
    p[22] = (uint8_t)((session->gold >> 16) & 0xFF);
    p[23] = (uint8_t)((session->gold >> 24) & 0xFF);
    p[24] = (uint8_t)(session->reputation & 0xFF);
    p[25] = (uint8_t)((session->reputation >> 8) & 0xFF);
    p[26] = session->rain_intensity;
    p[27] = session->weather_padding;
    p[28] = DM2_SESSION_VERSION;
    /* p[29..261]: reserved (already zero from calloc) */

    /* Copy champion records (4 × 261 bytes) */
    uint8_t *chp = buf + 29;
    for (int i = 0; i < 4; i++) {
        memcpy(chp, session->champion_data[i], 261);
        chp += 261;
    }

    ext = buf + DM2_SESSION_BASE_SERIALIZED_SIZE;
    memcpy(ext, session->original_global_flags, DM2_GLOBAL_FLAGS_SIZE);
    ext += DM2_GLOBAL_FLAGS_SIZE;
    memcpy(ext, session->original_global_bytes, DM2_GLOBAL_BYTES_SIZE);
    ext += DM2_GLOBAL_BYTES_SIZE;
    for (int i = 0; i < DM2_GLOBAL_WORDS_SIZE; i++) {
        ext[i * 2 + 0] =
            (uint8_t)(session->original_global_words[i] & 0xFFu);
        ext[i * 2 + 1] =
            (uint8_t)((session->original_global_words[i] >> 8) & 0xFFu);
    }
    ext += DM2_GLOBAL_WORDS_SIZE * 2;
    memcpy(ext, session->original_spell_effects,
           DM2_GLOBAL_SPELL_EFFECTS_SIZE);
    ext += DM2_GLOBAL_SPELL_EFFECTS_SIZE;
    *ext++ = session->original_timer_count > DM2_MAX_TIMERS
                 ? DM2_MAX_TIMERS
                 : session->original_timer_count;
    for (int i = 0; i < DM2_MAX_TIMERS; i++) {
        const DM2_TimerEntry *timer = &session->original_timers[i];
        ext[0] = (uint8_t)(timer->timer_id & 0xFFu);
        ext[1] = (uint8_t)((timer->timer_id >> 8) & 0xFFu);
        ext[2] = (uint8_t)(timer->current_tick & 0xFFu);
        ext[3] = (uint8_t)((timer->current_tick >> 8) & 0xFFu);
        ext[4] = (uint8_t)(timer->interval_ticks & 0xFFu);
        ext[5] = (uint8_t)((timer->interval_ticks >> 8) & 0xFFu);
        ext[6] = (uint8_t)(timer->flags & 0xFFu);
        ext[7] = (uint8_t)((timer->flags >> 8) & 0xFFu);
        ext[8] = (uint8_t)(timer->user_data & 0xFFu);
        ext[9] = (uint8_t)((timer->user_data >> 8) & 0xFFu);
        ext += DM2_TIMER_ENTRY_SIZE;
    }
    ext[0] = (uint8_t)(session->original_leader_hand_object & 0xFFu);
    ext[1] = (uint8_t)((session->original_leader_hand_object >> 8) & 0xFFu);
    ext[2] = (uint8_t)((session->original_leader_hand_object >> 16) & 0xFFu);
    ext[3] = (uint8_t)((session->original_leader_hand_object >> 24) & 0xFFu);
    ext += 4;
    *ext++ = session->original_minions.count > DM2_MAX_MINIONS
                 ? DM2_MAX_MINIONS
                 : session->original_minions.count;
    for (int i = 0; i < DM2_MAX_MINIONS; i++) {
        const DM2_MinionAssoc *m = &session->original_minions.entries[i];
        ext[0] = (uint8_t)(m->object_id & 0xFFu);
        ext[1] = (uint8_t)((m->object_id >> 8) & 0xFFu);
        ext[2] = (uint8_t)((m->object_id >> 16) & 0xFFu);
        ext[3] = (uint8_t)((m->object_id >> 24) & 0xFFu);
        ext[4] = (uint8_t)(m->owner_champion & 0xFFu);
        ext[5] = (uint8_t)((m->owner_champion >> 8) & 0xFFu);
        ext[6] = (uint8_t)((m->owner_champion >> 16) & 0xFFu);
        ext[7] = (uint8_t)((m->owner_champion >> 24) & 0xFFu);
        ext += 8;
    }

    return DM2_SESSION_SERIALIZED_SIZE;
}

int dm2_v1_session_deserialize(DM2_V1_SessionState *session,
                                 const uint8_t *buf, size_t buf_size)
{
    if (!session || !buf) return -1;
    enum {
        DM2_SESSION_BASE_SERIALIZED_SIZE = 29 + 4 * 261,
        DM2_SESSION_EXT_V1_SERIALIZED_SIZE =
            DM2_SESSION_BASE_SERIALIZED_SIZE +
            DM2_GLOBAL_FLAGS_SIZE +
            DM2_GLOBAL_BYTES_SIZE +
            DM2_GLOBAL_WORDS_SIZE * 2 +
            DM2_GLOBAL_SPELL_EFFECTS_SIZE +
            1 +
            DM2_MAX_TIMERS * DM2_TIMER_ENTRY_SIZE,
        DM2_SESSION_EXT_V2_SERIALIZED_SIZE =
            DM2_SESSION_EXT_V1_SERIALIZED_SIZE + 4,
        DM2_SESSION_EXT_V3_SERIALIZED_SIZE =
            DM2_SESSION_EXT_V2_SERIALIZED_SIZE + 1 + DM2_MAX_MINIONS * 8
    };
    const uint8_t *ext;
    if (buf_size < (size_t)DM2_SESSION_BASE_SERIALIZED_SIZE) return -1;
    memset(session, 0, sizeof(*session));

    const uint8_t *p = buf;
    session->game_tick =
        ((uint32_t)p[0]) |
        ((uint32_t)p[1] << 8) |
        ((uint32_t)p[2] << 16) |
        ((uint32_t)p[3] << 24);

    session->rng_seed =
        ((uint32_t)p[4]) |
        ((uint32_t)p[5] << 8) |
        ((uint32_t)p[6] << 16) |
        ((uint32_t)p[7] << 24);

    session->champion_count  = (uint8_t)p[8] | ((uint8_t)p[9] << 8);
    session->leader_index    = p[10];
    session->party_x         = (uint16_t)p[11] | ((uint16_t)p[12] << 8);
    session->party_y         = (uint16_t)p[13] | ((uint16_t)p[14] << 8);
    session->party_dir       = p[15];
    session->party_level     = p[16];
    session->outdoor_mode    = p[17];
    session->time_of_day_minutes = (uint16_t)p[18] | ((uint16_t)p[19] << 8);
    session->gold            = ((uint32_t)p[20]) |
                                 ((uint32_t)p[21] << 8) |
                                 ((uint32_t)p[22] << 16) |
                                 ((uint32_t)p[23] << 24);
    session->reputation      = (int16_t)p[24] | ((int16_t)p[25] << 8);
    session->rain_intensity  = p[26];
    session->weather_padding = p[27];
    /* session_version is an explicit profile marker. Reject stale/mismatched
     * fixtures before accepting loaded state (docs/dm2_save_format.md § session
     * serialization format, byte 28). */
    if (p[28] != DM2_SESSION_VERSION) {
        return -1;
    }

    /* Copy champion records */
    const uint8_t *chp = buf + 29;
    for (int i = 0; i < 4; i++) {
        memcpy(session->champion_data[i], chp, 261);
        chp += 261;
    }

    if (buf_size >= (size_t)DM2_SESSION_EXT_V1_SERIALIZED_SIZE) {
        ext = buf + DM2_SESSION_BASE_SERIALIZED_SIZE;
        memcpy(session->original_global_flags, ext, DM2_GLOBAL_FLAGS_SIZE);
        ext += DM2_GLOBAL_FLAGS_SIZE;
        memcpy(session->original_global_bytes, ext, DM2_GLOBAL_BYTES_SIZE);
        ext += DM2_GLOBAL_BYTES_SIZE;
        for (int i = 0; i < DM2_GLOBAL_WORDS_SIZE; i++) {
            session->original_global_words[i] =
                (uint16_t)ext[i * 2 + 0] |
                ((uint16_t)ext[i * 2 + 1] << 8);
        }
        ext += DM2_GLOBAL_WORDS_SIZE * 2;
        memcpy(session->original_spell_effects, ext,
               DM2_GLOBAL_SPELL_EFFECTS_SIZE);
        ext += DM2_GLOBAL_SPELL_EFFECTS_SIZE;
        session->original_timer_count = *ext++;
        if (session->original_timer_count > DM2_MAX_TIMERS) {
            return -1;
        }
        for (int i = 0; i < DM2_MAX_TIMERS; i++) {
            DM2_TimerEntry *timer = &session->original_timers[i];
            timer->timer_id =
                (uint16_t)ext[0] | ((uint16_t)ext[1] << 8);
            timer->current_tick =
                (uint16_t)ext[2] | ((uint16_t)ext[3] << 8);
            timer->interval_ticks =
                (uint16_t)ext[4] | ((uint16_t)ext[5] << 8);
            timer->flags =
                (uint16_t)ext[6] | ((uint16_t)ext[7] << 8);
            timer->user_data =
                (uint16_t)ext[8] | ((uint16_t)ext[9] << 8);
            ext += DM2_TIMER_ENTRY_SIZE;
        }
        if (buf_size >= (size_t)DM2_SESSION_EXT_V2_SERIALIZED_SIZE) {
            session->original_leader_hand_object =
                ((uint32_t)ext[0]) |
                ((uint32_t)ext[1] << 8) |
                ((uint32_t)ext[2] << 16) |
                ((uint32_t)ext[3] << 24);
            ext += 4;
        }
        if (buf_size >= (size_t)DM2_SESSION_EXT_V3_SERIALIZED_SIZE) {
            session->original_minions.count = *ext++;
            if (session->original_minions.count > DM2_MAX_MINIONS) {
                return -1;
            }
            for (int i = 0; i < DM2_MAX_MINIONS; i++) {
                DM2_MinionAssoc *m = &session->original_minions.entries[i];
                m->object_id =
                    ((uint32_t)ext[0]) |
                    ((uint32_t)ext[1] << 8) |
                    ((uint32_t)ext[2] << 16) |
                    ((uint32_t)ext[3] << 24);
                m->owner_champion =
                    ((uint32_t)ext[4]) |
                    ((uint32_t)ext[5] << 8) |
                    ((uint32_t)ext[6] << 16) |
                    ((uint32_t)ext[7] << 24);
                ext += 8;
            }
        }
    }

    /* Validate deserialized session */
    if (!dm2_v1_session_validate(session)) return -1;

    return 0;
}

static int dm2_v1_read_payload_i32_le(const uint8_t *buf,
                                      size_t size,
                                      size_t *pos,
                                      int *out)
{
    uint32_t value;
    if (!buf || !pos || !out || *pos > size || size - *pos < 4u) {
        return 0;
    }
    value = ((uint32_t)buf[*pos + 0u]) |
            ((uint32_t)buf[*pos + 1u] << 8) |
            ((uint32_t)buf[*pos + 2u] << 16) |
            ((uint32_t)buf[*pos + 3u] << 24);
    *pos += 4u;
    *out = (int)value;
    return 1;
}

static int dm2_v1_read_payload_tagged_blob(const uint8_t *buf,
                                           size_t size,
                                           size_t *pos,
                                           const char tag[4],
                                           const uint8_t **out_data,
                                           size_t *out_size)
{
    int blob_size;

    if (!buf || !pos || !tag || !out_data || !out_size ||
        *pos > size || size - *pos < 8u) {
        return 0;
    }
    if (memcmp(buf + *pos, tag, 4u) != 0) {
        return 0;
    }
    *pos += 4u;
    if (!dm2_v1_read_payload_i32_le(buf, size, pos, &blob_size) ||
        blob_size < 0 ||
        (size_t)blob_size > size - *pos) {
        return -1;
    }
    *out_data = buf + *pos;
    *out_size = (size_t)blob_size;
    *pos += (size_t)blob_size;
    return 1;
}

static uint16_t dm2_v1_read_u16_le_at(const uint8_t *buf, size_t offset)
{
    return (uint16_t)buf[offset] | ((uint16_t)buf[offset + 1u] << 8);
}

static uint32_t dm2_v1_read_u32_le_at(const uint8_t *buf, size_t offset)
{
    return ((uint32_t)buf[offset]) |
           ((uint32_t)buf[offset + 1u] << 8) |
           ((uint32_t)buf[offset + 2u] << 16) |
           ((uint32_t)buf[offset + 3u] << 24);
}

static int dm2_v1_import_original_minion_blob(DM2_V1_SessionState *session,
                                              const uint8_t *blob,
                                              size_t blob_size)
{
    size_t pos = 0u;
    int count;

    if (!session || !blob || blob_size < 4u) return -1;
    if (!dm2_v1_read_payload_i32_le(blob, blob_size, &pos, &count) ||
        count < 0 || count > DM2_MAX_MINIONS) {
        return -1;
    }
    if ((size_t)count * 8u > blob_size - pos) return -1;
    memset(&session->original_minions, 0, sizeof(session->original_minions));
    session->original_minions.count = (uint8_t)count;
    for (int i = 0; i < count; i++) {
        session->original_minions.entries[i].object_id =
            dm2_v1_read_u32_le_at(blob, pos);
        session->original_minions.entries[i].owner_champion =
            dm2_v1_read_u32_le_at(blob, pos + 4u);
        pos += 8u;
    }
    return pos == blob_size ? 0 : -1;
}

static int dm2_v1_import_original_optional_sections(
    DM2_V1_SessionState *session,
    const uint8_t *buf,
    size_t size,
    size_t *pos)
{
    const uint8_t *blob = NULL;
    size_t blob_size = 0u;
    int r;

    if (!session || !buf || !pos) return -1;

    while (*pos < size) {
        if (size - *pos < 8u) return -1;

        r = dm2_v1_read_payload_tagged_blob(buf, size, pos, "GFLG",
                                            &blob, &blob_size);
        if (r < 0) return -1;
        if (r > 0) {
            if (dm2_suppress_decode_global_flags(
                    blob, blob_size, session->original_global_flags, 0) !=
                (int)blob_size) {
                return -1;
            }
            continue;
        }

        r = dm2_v1_read_payload_tagged_blob(buf, size, pos, "GBYT",
                                            &blob, &blob_size);
        if (r < 0) return -1;
        if (r > 0) {
            if (dm2_suppress_decode_global_bytes(
                    blob, blob_size, session->original_global_bytes, 0) !=
                (int)blob_size) {
                return -1;
            }
            continue;
        }

        r = dm2_v1_read_payload_tagged_blob(buf, size, pos, "GWRD",
                                            &blob, &blob_size);
        if (r < 0) return -1;
        if (r > 0) {
            if (dm2_suppress_decode_global_words(
                    blob, blob_size, session->original_global_words, 0) !=
                (int)blob_size) {
                return -1;
            }
            continue;
        }

        r = dm2_v1_read_payload_tagged_blob(buf, size, pos, "SPFX",
                                            &blob, &blob_size);
        if (r < 0) return -1;
        if (r > 0) {
            if (dm2_suppress_decode_spell_effects(
                    blob, blob_size, session->original_spell_effects, 0) !=
                (int)blob_size) {
                return -1;
            }
            continue;
        }

        r = dm2_v1_read_payload_tagged_blob(buf, size, pos, "TMR0",
                                            &blob, &blob_size);
        if (r < 0) return -1;
        if (r > 0) {
            size_t timer_pos = 0u;
            int timer_count;

            if (!dm2_v1_read_payload_i32_le(blob, blob_size,
                                            &timer_pos, &timer_count) ||
                timer_count < 0 ||
                timer_count > DM2_MAX_TIMERS) {
                return -1;
            }
            session->original_timer_count = (uint8_t)timer_count;
            for (int i = 0; i < timer_count; i++) {
                int timer_size;
                if (!dm2_v1_read_payload_i32_le(blob, blob_size,
                                                &timer_pos, &timer_size) ||
                    timer_size <= 0 ||
                    (size_t)timer_size > blob_size - timer_pos ||
                    dm2_suppress_decode_timer(
                        blob + timer_pos,
                        (size_t)timer_size,
                        &session->original_timers[i],
                        0) != timer_size) {
                    return -1;
                }
                timer_pos += (size_t)timer_size;
            }
            if (timer_pos != blob_size) return -1;
            continue;
        }

        r = dm2_v1_read_payload_tagged_blob(buf, size, pos, "MIN0",
                                            &blob, &blob_size);
        if (r < 0) return -1;
        if (r > 0) {
            if (dm2_v1_import_original_minion_blob(session, blob,
                                                  blob_size) != 0) {
                return -1;
            }
            continue;
        }

        return -1;
    }
    return 0;
}

static void dm2_v1_apply_original_gamestate(DM2_V1_SessionState *session,
                                            const DM2_GameStateBlock *gs)
{
    if (!session || !gs) return;

    /* GAME_LOAD/SKLOAD restores the saved game-state and its following
     * SUPPRESS champion records.  Do not seed that import with
     * The test-only session fixture's fixed party, gold and entrance pose are
     * not values owned by this original payload.
     * A later malformed champion section must therefore leave no invented
     * party behind. Source: SKWINSPX/src/v4/skcore.cpp GAME_LOAD/SKLOAD
     * reads the game-state block and saved records as one transaction. */
    memset(session, 0, sizeof(*session));
    session->game_tick = gs->dwGameTick;
    session->rng_seed = gs->dwRandomSeed;
    session->party_x = gs->wPlayerPosX;
    session->party_y = gs->wPlayerPosY;
    session->party_dir = (uint8_t)(gs->wPlayerDir & 3u);
    session->party_level = (uint8_t)(gs->wPlayerMap & 0xFFu);
    session->leader_index = (uint8_t)(gs->wChampionLeader & 3u);
    session->rain_intensity = gs->rain_state[0];
}

static int dm2_v1_decode_original_champions(DM2_V1_SessionState *session,
                                            const uint8_t *buf,
                                            size_t buf_size,
                                            size_t *pos,
                                            int length_prefixed,
                                            int requested_champions)
{
    uint8_t champ_mask[261];
    int decoded_champions = 0;

    if (!session || !buf || !pos || *pos > buf_size) return -1;
    if (requested_champions < 0) requested_champions = 0;
    if (requested_champions > 4) requested_champions = 4;

    dm2_suppress_champion_mask(champ_mask);
    while (*pos < buf_size && decoded_champions < requested_champions) {
        int champ_size;
        DM2_ChampionRecord *champ =
            (DM2_ChampionRecord *)session->champion_data[decoded_champions];

        if (length_prefixed) {
            if (!dm2_v1_read_payload_i32_le(buf, buf_size, pos,
                                            &champ_size) ||
                champ_size <= 0 ||
                (size_t)champ_size > buf_size - *pos) {
                return -1;
            }
        } else {
            champ_size = (int)(buf_size - *pos);
        }

        memset(champ, 0, sizeof(*champ));
        champ_size = dm2_suppress_decode_champion(buf + *pos,
                                                  (size_t)champ_size,
                                                  champ_mask,
                                                  champ,
                                                  0);
        if (champ_size <= 0) return -1;
        *pos += (size_t)champ_size;
        decoded_champions++;
    }

    if (decoded_champions <= 0) return -1;
    session->champion_count = (uint8_t)decoded_champions;
    if (session->leader_index >= session->champion_count) {
        session->leader_index = 0;
    }
    return 0;
}

static uint32_t dm2_v1_raw_sksave_hash(const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!data || size == 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int dm2_v1_parse_raw_sksave_dungeon_prefix(
    const uint8_t *buf,
    size_t buf_size,
    DM2_V1_OriginalRawDungeonReceipt *out_receipt)
{
    int map_count;
    size_t column_index_base;
    size_t total_columns = 0u;
    size_t minimum_map_data_bytes = 0u;
    size_t map_data_bytes;
    size_t sft_count;
    size_t text_words;
    size_t cursor;
    DM2_V1_OriginalRawDungeonReceipt candidate;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!buf || !out_receipt ||
        buf_size < (size_t)(DM2_RAW_DUNGEON_HEADER_SIZE +
                            DM2_RAW_MAP_DESC_SIZE)) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));

    /* docs/dm2_save_format.md: original SKSave bodies start with a 44-byte
     * dungeon header and 16-byte map descriptors. This bounded locator mirrors
     * dm2_v1_dungeon_loader.c's skproject-layout math, then hands off at the
     * documented game-state SUPPRESS block. */
    map_count = (int)buf[4];
    if (map_count < 1 || map_count > 64) return 0;
    column_index_base = (size_t)DM2_RAW_DUNGEON_HEADER_SIZE +
                        (size_t)map_count * DM2_RAW_MAP_DESC_SIZE;
    if (column_index_base > buf_size) return 0;

    for (int i = 0; i < map_count; i++) {
        const size_t desc_off = (size_t)DM2_RAW_DUNGEON_HEADER_SIZE +
                                (size_t)i * DM2_RAW_MAP_DESC_SIZE;
        const uint16_t w8 = dm2_v1_read_u16_le_at(buf, desc_off + 8u);
        const size_t rel = dm2_v1_read_u16_le_at(buf, desc_off + 0u);
        const size_t width = (size_t)(((w8 >> 6) & 0x1Fu) + 1u);
        const size_t height = (size_t)(((w8 >> 11) & 0x1Fu) + 1u);
        size_t end;

        if (width == 0u || width > 64u || height == 0u || height > 64u) {
            return 0;
        }
        end = rel + width * height;
        if (end < rel) return 0;
        if (end > minimum_map_data_bytes) minimum_map_data_bytes = end;
        total_columns += width;
        if (total_columns > 4096u) return 0;
    }
    if (minimum_map_data_bytes == 0u) return 0;

    sft_count = dm2_v1_read_u16_le_at(buf, 10u);
    text_words = dm2_v1_read_u16_le_at(buf, 6u);
    map_data_bytes = dm2_v1_read_u16_le_at(buf, 2u);
    /* c_savegame.cpp::DM2_READ_DUNGEON_STRUCTURE reads warr_00[1] bytes
     * into v1e03e0. Descriptor geometry proves only a lower bound; it must
     * never replace this source-owned saved length. */
    if (map_data_bytes < minimum_map_data_bytes) return 0;
    cursor = column_index_base + total_columns * 2u +
             sft_count * 2u + text_words * 2u;
    if (cursor < column_index_base || cursor > buf_size) return 0;

    candidate.map_count = (uint8_t)map_count;
    candidate.map_data_byte_count = (uint16_t)map_data_bytes;
    candidate.column_index_count = (uint16_t)total_columns;
    candidate.ground_stack_count = (uint16_t)sft_count;
    candidate.text_word_count = (uint16_t)text_words;
    candidate.descriptor_hash = dm2_v1_raw_sksave_hash(
        buf + DM2_RAW_DUNGEON_HEADER_SIZE,
        (size_t)map_count * DM2_RAW_MAP_DESC_SIZE);
    candidate.column_index_hash = dm2_v1_raw_sksave_hash(
        buf + column_index_base, total_columns * 2u);
    candidate.ground_stack_hash = dm2_v1_raw_sksave_hash(
        buf + column_index_base + total_columns * 2u, sft_count * 2u);
    candidate.text_hash = dm2_v1_raw_sksave_hash(
        buf + column_index_base + total_columns * 2u + sft_count * 2u,
        text_words * 2u);

    for (int i = 0; i < DM2_RAW_THING_TYPE_COUNT; i++) {
        const size_t count = dm2_v1_read_u16_le_at(buf, 12u + (size_t)i * 2u);
        const size_t bytes = (size_t)s_dm2_raw_db_record_size[i];
        const size_t pool_bytes = count * bytes;
        if (bytes != 0u && pool_bytes / bytes != count) return 0;
        if (pool_bytes > buf_size - cursor) return 0;
        candidate.db_record_counts[i] = (uint16_t)count;
        candidate.db_pool_offsets[i] = cursor;
        candidate.db_pool_hashes[i] = dm2_v1_raw_sksave_hash(
            buf + cursor, pool_bytes);
        cursor += pool_bytes;
    }
    if (map_data_bytes > buf_size - cursor) return 0;
    candidate.map_data_offset = cursor;
    candidate.map_data_hash = dm2_v1_raw_sksave_hash(buf + cursor,
                                                     map_data_bytes);
    cursor += map_data_bytes;
    /* A receipt consumer may deliberately retain only the exact dungeon
     * prefix.  Full raw-SKSave ingestion still requires the following
     * SUPPRESS stream in dm2_v1_session_import_raw_sksave_payload(). */
    if (cursor > buf_size) return 0;

    candidate.prefix_hash = dm2_v1_raw_sksave_hash(buf, cursor);
    candidate.suppress_state_offset = cursor;
    candidate.valid = candidate.descriptor_hash != 0u &&
                      candidate.column_index_hash != 0u &&
                      candidate.map_data_hash != 0u;
    if (!candidate.valid) return 0;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_dungeon_receipt(
    const uint8_t *buf,
    size_t buf_size,
    DM2_V1_OriginalRawDungeonReceipt *out_receipt)
{
    return dm2_v1_parse_raw_sksave_dungeon_prefix(buf, buf_size,
                                                   out_receipt);
}

int dm2_v1_original_raw_sksave_db_record_receipt(
    const uint8_t *buf, size_t buf_size, int db_pool, int record_index,
    DM2_V1_OriginalRawDbRecordReceipt *out_receipt)
{
    DM2_V1_OriginalRawDungeonReceipt dungeon;
    DM2_V1_OriginalRawDbRecordReceipt candidate;
    size_t record_size;
    size_t offset;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!buf || db_pool < 0 || db_pool >= DM2_RAW_SKSAVE_DB_POOL_COUNT ||
        record_index < 0 ||
        !dm2_v1_parse_raw_sksave_dungeon_prefix(buf, buf_size, &dungeon) ||
        !dungeon.valid || record_index >= dungeon.db_record_counts[db_pool]) {
        return 0;
    }
    record_size = (size_t)s_dm2_raw_db_record_size[db_pool];
    if (record_size == 0u || (size_t)record_index > SIZE_MAX / record_size) {
        return 0;
    }
    offset = dungeon.db_pool_offsets[db_pool] + (size_t)record_index * record_size;
    if (offset < dungeon.db_pool_offsets[db_pool] ||
        record_size > dungeon.suppress_state_offset - offset ||
        offset + record_size > buf_size) return 0;
    memset(&candidate, 0, sizeof(candidate));
    candidate.db_pool = (uint8_t)db_pool;
    candidate.record_index = (uint16_t)record_index;
    candidate.record_size = (uint16_t)record_size;
    candidate.record_offset = offset;
    candidate.pool_hash = dungeon.db_pool_hashes[db_pool];
    candidate.record_hash = dm2_v1_raw_sksave_hash(buf + offset, record_size);
    candidate.valid = candidate.pool_hash != 0u && candidate.record_hash != 0u;
    if (!candidate.valid) return 0;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_door_receipt(
    const uint8_t *buf, size_t buf_size, int record_index,
    DM2_V1_OriginalRawDoorReceipt *out_receipt)
{
    DM2_V1_OriginalRawDoorReceipt candidate;
    uint16_t attributes;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, 0, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 4u ||
        candidate.record.record_offset > buf_size - 4u) return 0;
    /* DME.h Door: w0 remains opaque; w2 owns these exact fields. */
    attributes = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 2u);
    candidate.attributes = attributes;
    candidate.button = (uint8_t)((attributes >> 6) & 1u);
    candidate.door_type = (uint8_t)(attributes & 1u);
    candidate.button_state = (uint8_t)((attributes >> 11) & 1u);
    candidate.opening_dir = (uint8_t)((attributes >> 5) & 1u);
    candidate.ornate_index = (uint8_t)((attributes >> 1) & 0x0fu);
    candidate.destroyable_by_fireball = (uint8_t)((attributes >> 7) & 1u);
    candidate.bashable_by_chopping = (uint8_t)((attributes >> 8) & 1u);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_actuator_receipt(
    const uint8_t *buf, size_t buf_size, int record_index,
    DM2_V1_OriginalRawActuatorReceipt *out_receipt)
{
    DM2_V1_OriginalRawActuatorReceipt candidate;
    uint16_t w2, w4, w6;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, 3, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 8u ||
        candidate.record.record_offset > buf_size - 8u) return 0;
    w2 = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 2u);
    w4 = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 4u);
    w6 = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 6u);
    candidate.actuator_type = (uint8_t)(w2 & 0x007fu);
    candidate.actuator_data = (uint16_t)((w2 >> 7) & 0x01ffu);
    candidate.graphic_number = (uint8_t)((w4 >> 12) & 0x000fu);
    candidate.disabled = (uint8_t)((w4 >> 11) & 1u);
    candidate.delay = (uint8_t)((w4 >> 7) & 0x000fu);
    candidate.sound_effect = (uint8_t)((w4 >> 6) & 1u);
    candidate.revert_effect = (uint8_t)((w4 >> 5) & 1u);
    candidate.action_type = (uint8_t)((w4 >> 3) & 3u);
    candidate.once_only = (uint8_t)((w4 >> 2) & 1u);
    candidate.active_status = (uint8_t)(w4 & 1u);
    candidate.target_direction = (uint8_t)((w6 >> 4) & 3u);
    candidate.target_x = (uint8_t)((w6 >> 6) & 0x001fu);
    candidate.target_y = (uint8_t)((w6 >> 11) & 0x001fu);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_creature_receipt(
    const uint8_t *buf, size_t buf_size, int record_index,
    DM2_V1_OriginalRawCreatureReceipt *out_receipt)
{
    DM2_V1_OriginalRawCreatureReceipt candidate;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, 4, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 16u ||
        candidate.record.record_offset > buf_size - 16u) return 0;
    /* SKWIN/DME.h Creature: possession w2 stays opaque. */
    candidate.creature_type = buf[candidate.record.record_offset + 4u];
    candidate.hp1 = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 6u);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_text_receipt(
    const uint8_t *buf, size_t buf_size, int record_index,
    DM2_V1_OriginalRawTextReceipt *out_receipt)
{
    DM2_V1_OriginalRawTextReceipt candidate;
    uint16_t attributes;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, 2, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 4u ||
        candidate.record.record_offset > buf_size - 4u) {
        return 0;
    }
    attributes = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 2u);
    candidate.visible = (uint8_t)(attributes & 1u);
    candidate.mode = (uint8_t)((attributes >> 1) & 3u);
    candidate.text_index = (uint16_t)((attributes >> 3) & 0x1fffu);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_teleporter_receipt(
    const uint8_t *buf, size_t buf_size, int record_index,
    DM2_V1_OriginalRawTeleporterReceipt *out_receipt)
{
    DM2_V1_OriginalRawTeleporterReceipt candidate;
    uint16_t w2;
    uint16_t w4;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, 1, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 6u ||
        candidate.record.record_offset > buf_size - 6u) {
        return 0;
    }
    /* DME.h::Teleporter keeps the next ObjectID in w0. Read only the direct
     * destination and transform fields used by the source map route. */
    w2 = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 2u);
    w4 = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 4u);
    candidate.destination_x = (uint8_t)(w2 & 0x001fu);
    candidate.destination_y = (uint8_t)((w2 >> 5) & 0x001fu);
    candidate.destination_map = (uint8_t)(w4 >> 8);
    candidate.scope = (uint8_t)((w2 >> 13) & 3u);
    candidate.sound = (uint8_t)((w2 >> 15) & 1u);
    candidate.rotation = (uint8_t)((w2 >> 10) & 3u);
    candidate.rotation_type = (uint8_t)((w2 >> 12) & 1u);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_container_receipt(
    const uint8_t *buf, size_t buf_size, int record_index,
    DM2_V1_OriginalRawContainerReceipt *out_receipt)
{
    DM2_V1_OriginalRawContainerReceipt candidate;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, 9, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 8u ||
        candidate.record.record_offset > buf_size - 8u) {
        return 0;
    }
    candidate.opened = (uint8_t)(buf[candidate.record.record_offset + 4u] & 1u);
    candidate.container_type = (uint8_t)(
        (buf[candidate.record.record_offset + 4u] >> 1) & 3u);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_weapon_receipt(
    const uint8_t *buf, size_t buf_size, int record_index,
    DM2_V1_OriginalRawWeaponReceipt *out_receipt)
{
    DM2_V1_OriginalRawWeaponReceipt candidate;
    uint16_t w2;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, 5, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 4u ||
        candidate.record.record_offset > buf_size - 4u) return 0;
    w2 = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 2u);
    candidate.item_type = (uint8_t)(w2 & 0x007fu);
    candidate.important = (uint8_t)((w2 >> 7) & 1u);
    candidate.charges = (uint8_t)((w2 >> 10) & 0x0fu);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_original_raw_sksave_item_receipt(
    const uint8_t *buf, size_t buf_size, int db_pool, int record_index,
    DM2_V1_OriginalRawItemReceipt *out_receipt)
{
    DM2_V1_OriginalRawItemReceipt candidate;
    uint16_t attributes;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if ((db_pool != 5 && db_pool != 6 && db_pool != 7 && db_pool != 10) ||
        !dm2_v1_original_raw_sksave_db_record_receipt(
            buf, buf_size, db_pool, record_index, &candidate.record) ||
        !candidate.record.valid || candidate.record.record_size != 4u ||
        candidate.record.record_offset > buf_size - 4u) {
        return 0;
    }
    /* DME.h Weapon::ItemType, Cloth::ItemType, Scroll::ItemType, and
     * Miscellaneous_item::ItemType all read w2 bits 0..6. */
    attributes = dm2_v1_read_u16_le_at(buf, candidate.record.record_offset + 2u);
    candidate.item_type = (uint8_t)(attributes & 0x007fu);
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_session_import_raw_sksave_payload(DM2_V1_SessionState *session,
                                             const uint8_t *buf,
                                             size_t buf_size)
{
    DM2_V1_SessionState candidate;
    DM2_GameStateBlock gs;
    DM2_V1_OriginalRawDungeonReceipt dungeon_receipt;
    size_t pos = 0u;
    int decoded;

    if (!session || !buf) return -1;
    if (!dm2_v1_parse_raw_sksave_dungeon_prefix(buf, buf_size,
                                                &dungeon_receipt)) {
        return -1;
    }
    pos = dungeon_receipt.suppress_state_offset;

    memset(&gs, 0, sizeof(gs));
    decoded = dm2_suppress_decode_gamestate(buf + pos, buf_size - pos,
                                            &gs, 0);
    if (decoded <= 0) return -1;
    pos += (size_t)decoded;

    /* SKProject GAME_LOAD restores into a rebuilt game state only after its
     * save sections have been accepted. Keep the Firestaff raw importer
     * transactional too: a truncated later SUPPRESS section must not leave
     * an existing live session partly overwritten. */
    memset(&candidate, 0, sizeof(candidate));
    dm2_v1_apply_original_gamestate(&candidate, &gs);

    decoded = dm2_suppress_decode_global_flags(buf + pos, buf_size - pos,
                                               candidate.original_global_flags,
                                               0);
    if (decoded <= 0) return -1;
    pos += (size_t)decoded;

    decoded = dm2_suppress_decode_global_bytes(buf + pos, buf_size - pos,
                                               candidate.original_global_bytes,
                                               0);
    if (decoded <= 0) return -1;
    pos += (size_t)decoded;

    decoded = dm2_suppress_decode_global_words(buf + pos, buf_size - pos,
                                               candidate.original_global_words,
                                               0);
    if (decoded <= 0) return -1;
    pos += (size_t)decoded;

    if (dm2_v1_decode_original_champions(&candidate, buf, buf_size, &pos, 0,
                                         (int)gs.wChampionsCount) != 0) {
        return -1;
    }

    decoded = dm2_suppress_decode_spell_effects(buf + pos, buf_size - pos,
                                                candidate.original_spell_effects,
                                                0);
    if (decoded <= 0) return -1;
    pos += (size_t)decoded;

    candidate.original_timer_count = 0;
    if (gs.wTimersCount > 0u) {
        int timer_count = (int)gs.wTimersCount;
        if (timer_count > DM2_MAX_TIMERS) timer_count = DM2_MAX_TIMERS;
        for (int i = 0; i < timer_count; i++) {
            decoded = dm2_suppress_decode_timer(buf + pos, buf_size - pos,
                                                &candidate.original_timers[i],
                                                0);
            if (decoded <= 0) return -1;
            pos += (size_t)decoded;
            candidate.original_timer_count++;
        }
    }

    if (candidate.champion_count > 0u) {
        const size_t inv_bytes =
            (size_t)candidate.champion_count *
            (size_t)DM2_CHAMPION_INVENTORY_SLOTS * 4u;
        if (inv_bytes > buf_size - pos) return -1;
        for (int c = 0; c < (int)candidate.champion_count; c++) {
            DM2_ChampionRecord *champ =
                (DM2_ChampionRecord *)candidate.champion_data[c];
            for (int slot = 0; slot < DM2_CHAMPION_INVENTORY_SLOTS;
                 slot++) {
                champ->inventory[slot] =
                    dm2_v1_read_u32_le_at(buf, pos);
                pos += 4u;
            }
        }
    }
    if (buf_size - pos < 4u) return -1;
    candidate.original_leader_hand_object = dm2_v1_read_u32_le_at(buf, pos);
    pos += 4u;
    if (pos < buf_size) {
        const uint8_t count = buf[pos++];
        if (count > DM2_MAX_MINIONS ||
            (size_t)count * 8u > buf_size - pos) {
            return -1;
        }
        memset(&candidate.original_minions, 0,
               sizeof(candidate.original_minions));
        candidate.original_minions.count = count;
        for (int i = 0; i < (int)count; i++) {
            candidate.original_minions.entries[i].object_id =
                dm2_v1_read_u32_le_at(buf, pos);
            candidate.original_minions.entries[i].owner_champion =
                dm2_v1_read_u32_le_at(buf, pos + 4u);
            pos += 8u;
        }
    }

    if (pos != buf_size || !dm2_v1_session_validate(&candidate)) return -1;
    *session = candidate;
    return 0;
}

int dm2_v1_session_parse_save_candidate(DM2_V1_SaveCandidate *out_candidate,
                                         const uint8_t *buf,
                                         size_t buf_size)
{
    DM2_V1_SaveCandidate candidate;

    if (!out_candidate || !buf || buf_size == 0u) return -1;
    memset(&candidate, 0, sizeof(candidate));

    if (dm2_v1_session_deserialize(&candidate.session, buf, buf_size) == 0) {
        candidate.kind = DM2_V1_SAVE_CANDIDATE_FIRESTAFF_SESSION;
    } else if (dm2_v1_session_import_original_payload(&candidate.session,
                                                       buf, buf_size) == 0) {
        candidate.kind = DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE;
    } else {
        /* skproject/SKULLWIN/skgame.cpp's load path restores the dungeon DB
         * before applying global/party state. The raw SKSave prefix is kept
         * as caller-owned bytes here so runtime can enforce that same order
         * without accepting a partial or differently-sized dungeon. */
        DM2_V1_OriginalRawDungeonReceipt dungeon_receipt;
        if (!dm2_v1_parse_raw_sksave_dungeon_prefix(buf, buf_size,
                                                    &dungeon_receipt) ||
            dungeon_receipt.suppress_state_offset == 0u ||
            dm2_v1_session_import_raw_sksave_payload(&candidate.session,
                                                      buf, buf_size) != 0) {
            return -1;
        }
        candidate.kind = DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW;
        candidate.dungeon_bytes = buf;
        candidate.dungeon_size = dungeon_receipt.suppress_state_offset;
        candidate.dungeon_receipt = dungeon_receipt;
    }

    if (!dm2_v1_session_validate(&candidate.session)) return -1;
    *out_candidate = candidate;
    return 0;
}

int dm2_v1_session_import_original_payload(DM2_V1_SessionState *session,
                                           const uint8_t *buf,
                                           size_t buf_size)
{
    DM2_V1_SessionState candidate;
    DM2_GameStateBlock gs;
    size_t pos = 0u;
    int gs_size;
    int requested_champions;

    if (!session || !buf || buf_size < 12u) {
        return -1;
    }
    if (memcmp(buf, "D2RS", 4) != 0) {
        return -1;
    }
    pos = 4u;
    if (!dm2_v1_read_payload_i32_le(buf, buf_size, &pos, &gs_size) ||
        gs_size <= 0 ||
        (size_t)gs_size > buf_size - pos) {
        return -1;
    }
    memset(&gs, 0, sizeof(gs));
    if (dm2_suppress_decode_gamestate(buf + pos,
                                      (size_t)gs_size,
                                      &gs,
                                      0) != gs_size) {
        return -1;
    }
    pos += (size_t)gs_size;

    /* SKProject SKWINSPX/src/v4/skcore.cpp::GAME_LOAD (lines 9823-9970)
     * reads the state, global rows, champions, effects and timers as one
     * ordered save sequence. Keep the Firestaff envelope atomic so an
     * optional malformed SUPPRESS section cannot replace only part of a
     * live party. */
    memset(&candidate, 0, sizeof(candidate));
    dm2_v1_apply_original_gamestate(&candidate, &gs);
    requested_champions = (int)gs.wChampionsCount;
    if (dm2_v1_decode_original_champions(&candidate, buf, buf_size, &pos, 1,
                                         requested_champions) != 0) {
        return -1;
    }
    if (pos < buf_size &&
        dm2_v1_import_original_optional_sections(&candidate, buf,
                                                 buf_size, &pos) != 0) {
        return -1;
    }
    if (pos != buf_size) {
        return -1;
    }
    if (!dm2_v1_session_validate(&candidate)) {
        return -1;
    }
    *session = candidate;
    return 0;
}

/* ════════════════════════════════════════════════════════════════
 * Session → slot manager integration
 * Source: dm2_v1_save_load.h dm2_sl_save/dm2_sl_load
 *         docs/dm2_save_slots.md — 10-slot SKSave%02u.dat layout
 * ════════════════════════════════════════════════════════════════ */

/*
 * Slot data format:
 *   [42-byte slot header from dm2_sl_save]
 *   [session data — 1306 bytes]
 *
 * Total slot data size = 42 + 1306 = 1348 bytes
 */
enum { DM2_SLOT_SESSION_DATA_SIZE = 26 + 4 * 261 };

int dm2_v1_session_save_slot(const char *save_base, uint8_t slot,
                               const char *name,
                               const DM2_V1_SessionState *session)
{
    (void)save_base;
    (void)slot;
    (void)name;
    (void)session;
    /* SKProject: SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_SAVE emits the
     * complete globals/hero/timer/dungeon graph.  The old compact D2RS
     * session serializer is import support only and cannot be presented as
     * an original SKSaveNN.dat. */
    return DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED;
}

int dm2_v1_session_save_last_session(const char *save_base,
                                      const char *name,
                                      const DM2_V1_SessionState *session)
{
    (void)save_base;
    (void)name;
    (void)session;
    return DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED;
}

int dm2_v1_session_load_slot(const char *save_base, uint8_t slot,
                               DM2_V1_SessionState *session)
{
    if (!save_base || !session) return -1;

    /* Load raw data from slot */
    uint8_t buf[DM2_SESSION_MAX_SIZE];
    size_t out_size = 0;
    int r = dm2_sl_load(save_base, slot, buf, sizeof(buf), &out_size);
    if (r != 0) return r;

    {
        DM2_V1_SaveCandidate candidate;
        if (dm2_v1_session_parse_save_candidate(&candidate, buf, out_size) != 0)
            return -1;
        *session = candidate.session;
    }

    return 0;
}

int dm2_v1_session_load_last_session(const char *save_base,
                                      DM2_V1_SessionState *session)
{
    if (!save_base || !session) return -1;

    uint8_t buf[DM2_SESSION_MAX_SIZE];
    size_t out_size = 0;
    int r = dm2_sl_load_last_session(save_base, buf, sizeof(buf), &out_size);
    if (r != 0) return r;

    {
        DM2_V1_SaveCandidate candidate;
        if (dm2_v1_session_parse_save_candidate(&candidate, buf, out_size) != 0)
            return -1;
        *session = candidate.session;
    }

    return 0;
}

int dm2_v1_session_delete_slot(const char *save_base, uint8_t slot)
{
    if (!save_base) return -1;
    return dm2_sl_delete(save_base, slot);
}

/* ════════════════════════════════════════════════════════════════
 * New game flow
 * Source: SKULL.ASM T520 — party_placement (Hall of Champions, N)
 *         SKULL.ASM T560 — DUNGEON_Load completion
 *         CHAMPION.C F0280 — starter party generation
 *         CHAMPRST.C F0278 — CHAMPION_ResetDataToStartGame
 *         REQDISK.C F0428 — disk gate (N/A for modern file loading)
 * ════════════════════════════════════════════════════════════════ */

DM2_FlowResult dm2_v1_new_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot)
{
    if (!session) return DM2_FLOW_BAD_SESSION;
    if (!boot)    return DM2_FLOW_NO_ASSETS;

    /* Verify assets are available */
    if (!boot->assets_verified) {
        /* Try scanning assets if not done */
        DM2_V1_BootProfile tmp;
        dm2_v1_boot_profile_init(&tmp);
        if (dm2_v1_boot_scan_assets(&tmp,
                                    boot->asset_root[0]
                                        ? boot->asset_root
                                        : ".") != 0) {
            return DM2_FLOW_NO_ASSETS;
        }
    }

    /* SKWINSPX SkWinCore.cpp::SHOW_MENU_SCREEN returns to INIT, then
     * GAME_LOAD()/LOAD_NEW_DUNGEON owns the original records, party choice,
     * random seed and entrance pose. Test fixtures must never substitute for
     * that transaction. */
    return DM2_FLOW_GAME_LOAD_REQUIRED;
}

/* ════════════════════════════════════════════════════════════════
 * Load game flow
 * Source: dm2_v1_save_load.h dm2_sl_load
 *         docs/dm2_save_format.md — save slot layout
 * ════════════════════════════════════════════════════════════════ */

DM2_FlowResult dm2_v1_load_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot,
                                      uint8_t slot)
{
    if (!session) return DM2_FLOW_BAD_SESSION;
    if (!dm2_v1_save_slot_valid(slot)) return DM2_FLOW_SLOT_ERROR;

    /* Load session from slot */
    int r = dm2_v1_session_load_slot(boot->save_root, slot, session);
    if (r != 0) return DM2_FLOW_SLOT_ERROR;

    if (!dm2_v1_session_validate(session)) {
        return DM2_FLOW_BAD_SESSION;
    }

    return DM2_FLOW_OK;
}

/* ════════════════════════════════════════════════════════════════
 * Source evidence
 * ════════════════════════════════════════════════════════════════ */

const char *dm2_v1_new_game_source_evidence(void)
{
    return
        "DM2 V1 New Game & Session Management — Phase 6 implementation\n"
        "CHAMPION.C F0280 lines 63-170: CHAMPION_AddCandidateChampionToParty\n"
        "  — portrait-to-squad-position assignment, attribute loop\n"
        "CHAMPRST.C F0278: CHAMPION_ResetDataToStartGame — clears party state\n"
        "SKULL.ASM T520: party_placement — Hall of Champions (15,15,N)\n"
        "SKULL.ASM T560: DUNGEON_Load completion\n"
        "docs/dm2_party_state.md: 261-byte champion record, SUPPRESS mask\n"
        "docs/dm2_save_format.md: session serialization (1306 bytes)\n"
        "docs/dm2_save_slots.md: 10-slot SKSave%02u.dat layout (42+1306 bytes)\n";
}
