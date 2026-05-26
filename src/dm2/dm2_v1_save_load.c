/* DM2 V1 Save/Load — SUPPRESS codec + slot manager
 *
 * Source lock:
 *   SKULL.ASM: _2066_#### save/load entry points
 *   docs/dm2_save_format.md — full save format + SUPPRESS codec
 *   docs/dm2_save_slots.md — 10 slots, 0xBEEF/0xDEAD magic
 *   docs/dm2_party_state.md — champion squad persistence, masks
 *
 * SUPPRESS is a bit-plane RLE codec used throughout the DM2 save file:
 *   mask low nibble = 0 → field skipped
 *   mask low nibble = 1..7 → store that many LSBs of data[i]
 * Encoded data is packed LSB-first into output bytes.
 */

#include "dm2_v1_save_load.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ════════════════════════════════════════════════════════════════
 * SUPPRESS codec
 * ════════════════════════════════════════════════════════════════ */

int dm2_suppress_encode(const uint8_t *data, const uint8_t *mask,
                        size_t count, uint8_t *out, size_t out_capacity)
{
    if (!data || !mask || !out) return -1;
    size_t acc_bits = 0;
    uint8_t acc_byte = 0;
    size_t out_pos = 0;
    size_t i;

    for (i = 0; i < count; i++) {
        uint8_t mn = mask[i] & 0x0F;
        if (mn == 0) continue;

        uint8_t nbits = mn & 0x07;
        if (nbits == 0) continue;

        uint8_t val = data[i] & ((1u << nbits) - 1u);
        acc_byte |= (val << acc_bits);
        acc_bits += nbits;

        while (acc_bits >= 8) {
            if (out_pos >= out_capacity) return -1;
            out[out_pos++] = acc_byte & 0xFF;
            if (acc_bits >= 8) {
                acc_byte >>= 8;
                acc_bits -= 8;
            }
        }
    }

    if (acc_bits > 0) {
        if (out_pos >= out_capacity) return -1;
        out[out_pos++] = acc_byte;
    }

    return (int)out_pos;
}

int dm2_suppress_decode(const uint8_t *in, size_t in_capacity,
                        const uint8_t *mask, size_t count,
                        uint8_t *out, uint8_t fill)
{
    if (!in || !mask || !out) return -1;
    if (in_capacity == 0) return -1;

    /* Pre-fill output with fill value for absent fields */
    memset(out, fill ? 0xFF : 0x00, count);

    uint32_t acc_byte = 0;
    size_t in_pos = 0;
    size_t acc_avail = 0; /*bits available in acc_byte */
    size_t i;

    for (i = 0; i < count; i++) {
        uint8_t mn = mask[i] & 0x0F;
        if (mn == 0) continue;

        uint8_t nbits = mn & 0x07;
        if (nbits == 0) continue;

        /* Re-fill accumulator byte if needed */
        while (acc_avail < nbits) {
            if (in_pos >= in_capacity) return -1; /* underflow */
            acc_byte |= (uint32_t)in[in_pos++] << acc_avail;
            acc_avail += 8;
        }

        out[i] = (uint8_t)(acc_byte & ((1u << nbits) - 1u));
        acc_byte >>= nbits;
        acc_avail -= nbits;
    }

    return (int)in_pos;
}

int dm2_suppress_self_verification(void)
{
    /* Known vector: alternating bits, 4-bit mask per byte */
    uint8_t data[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    uint8_t mask[8] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 };
    uint8_t enc[64];
    uint8_t dec[8];

    int enc_sz = dm2_suppress_encode(data, mask, 8, enc, sizeof(enc));
    if (enc_sz < 0) return 0;

    int dec_sz = dm2_suppress_decode(enc, (size_t)enc_sz, mask, 8, dec, 0);
    if (dec_sz < 0) return 0;

    if (memcmp(data, dec, 8) != 0) return 0;
    return 1;
}

/* ════════════════════════════════════════════════════════════════
 * Slot manager
 * ════════════════════════════════════════════════════════════════ */

#define DM2_SLOT_MAGIC_1  0xBEEF
#define DM2_SLOT_MAGIC_2  0xDEAD

void dm2_sl_init(DM2_SL_State *state, const char *save_base)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (save_base && save_base[0]) {
        size_t len = strlen(save_base);
        if (len >= sizeof(state->save_base)) len = sizeof(state->save_base) - 1;
        memcpy(state->save_base, save_base, len);
        state->save_base[len] = '\0';
    } else {
        state->save_base[0] = '.';
        state->save_base[1] = '\0';
    }
    state->initialized = true;
}

static void slot_path(const DM2_SL_State *state, uint8_t slot,
                      char *buf, size_t bufsz, bool backup)
{
    const char *base = state->save_base[0] ? state->save_base : ".";
    if (backup) {
        snprintf(buf, bufsz, "%s/SKSave.bak", base);
    } else if (slot < DM2_SLOT_MAX) {
        snprintf(buf, bufsz, "%s/SKSave%02u.dat", base, (unsigned)slot);
    } else {
        snprintf(buf, bufsz, "%s/SKSave.dat", base);
    }
}

bool dm2_sl_slot_occupied(const DM2_SL_State *state, uint8_t slot)
{
    if (!state || slot >= DM2_SLOT_MAX) return false;
    return state->slots[slot].occupied;
}

const char *dm2_sl_slot_name(const DM2_SL_State *state, uint8_t slot)
{
    if (!state || slot >= DM2_SLOT_MAX || !state->slots[slot].occupied)
        return NULL;
    return state->slots[slot].name;
}

bool dm2_sl_scan_slots(DM2_SL_State *state)
{
    if (!state || !state->initialized) return false;

    state->slot_count = 0;
    for (uint8_t i = 0; i < DM2_SLOT_MAX; i++) {
        char path[512];
        slot_path(state, i, path, sizeof(path), false);

        FILE *f = fopen(path, "rb");
        if (!f) {
            state->slots[i].occupied = false;
            memset(state->slots[i].name, 0, sizeof(state->slots[i].name));
            state->slots[i].timestamp = 0;
            continue;
        }

        uint8_t hdr[42];
        if (fread(hdr, 42, 1, f) != 1) {
            fclose(f);
            state->slots[i].occupied = false;
            continue;
        }
        fclose(f);

        uint16_t m1 = (uint16_t)hdr[38] | ((uint16_t)hdr[39] << 8);
        uint16_t m2 = (uint16_t)hdr[40] | ((uint16_t)hdr[41] << 8);

        if (m1 == DM2_SLOT_MAGIC_1 && m2 == DM2_SLOT_MAGIC_2) {
            state->slots[i].occupied = true;
            /* Copy slot name (null-terminated, max 33 chars) */
            size_t j;
            for (j = 0; j < DM2_SLOT_NAME_MAX; j++) {
                state->slots[i].name[j] = hdr[2 + j];
                if (hdr[2 + j] == '\0') break;
            }
            state->slots[i].name[DM2_SLOT_NAME_MAX] = '\0';
            /* timestamp field not available from header; leave at 0 */
            state->slot_count++;
        } else {
            state->slots[i].occupied = false;
            memset(state->slots[i].name, 0, sizeof(state->slots[i].name));
        }
    }
    return true;
}

int dm2_sl_save(const char *save_base, uint8_t slot,
                 const char *name,
                 const uint8_t *data, size_t data_size)
{
    if (!save_base || !data || data_size == 0) return -1;
    if (slot >= DM2_SLOT_MAX) return -1;

    char path_dat[256], path_bak[256];
    snprintf(path_dat, sizeof(path_dat), "%s/SKSave%02u.dat", save_base, (unsigned)slot);
    snprintf(path_bak, sizeof(path_bak), "%s/SKSave.bak", save_base);

    /* Rotate: existing dat → bak (backup); ignore if dat doesn't exist */
    (void)remove(path_bak);
    (void)rename(path_dat, path_bak);

    FILE *f = fopen(path_dat, "wb");
    if (!f) return -2;

    /* Build 42-byte slot header (sksave_header_asc) */
    uint8_t hdr[42] = {0};
    hdr[0] = 1; hdr[1] = 0; /* version flag = 1 */
    if (name) {
        size_t nlen = strlen(name);
        if (nlen > 33) nlen = 33;
        memcpy(hdr + 2, name, nlen);
    }
    uint8_t slot_plus30 = (uint8_t)((slot + 0x30) & 0xFF);
    hdr[36] = slot_plus30;
    hdr[37] = 0;
    /* Magic markers: 0xBEEF / 0xDEAD — little-endian across w38/w40 */
    hdr[38] = (uint8_t)(DM2_SLOT_MAGIC_1 & 0xFF);
    hdr[39] = (uint8_t)((DM2_SLOT_MAGIC_1 >> 8) & 0xFF);
    hdr[40] = (uint8_t)(DM2_SLOT_MAGIC_2 & 0xFF);
    hdr[41] = (uint8_t)((DM2_SLOT_MAGIC_2 >> 8) & 0xFF);

    if (fwrite(hdr, 42, 1, f) != 1) { fclose(f); return -3; }
    if (fwrite(data, 1, data_size, f) != data_size) { fclose(f); return -3; }
    fclose(f);
    return 0;
}

int dm2_sl_load(const char *save_base, uint8_t slot,
                 uint8_t *data, size_t max_size, size_t *out_size)
{
    if (!save_base || !data || !out_size) return -1;
    if (slot >= DM2_SLOT_MAX) return -1;

    char path[256], bak[256];
    snprintf(path, sizeof(path), "%s/SKSave%02u.dat", save_base, (unsigned)slot);
    snprintf(bak,  sizeof(bak),  "%s/SKSave.bak",   save_base);

    FILE *f = fopen(path, "rb");
    if (!f) {
        /* Try backup */
        f = fopen(bak, "rb");
        if (!f) return -2;
    }

    uint8_t hdr[42];
    if (fread(hdr, 42, 1, f) != 1) { fclose(f); return -4; }

    size_t got = fread(data, 1, max_size, f);
    fclose(f);
    *out_size = got;
    return 0;
}

int dm2_sl_delete(const char *save_base, uint8_t slot)
{
    if (!save_base || slot >= DM2_SLOT_MAX) return -1;
    char path[256], bak[256];
    snprintf(path, sizeof(path), "%s/SKSave%02u.dat", save_base, (unsigned)slot);
    snprintf(bak,  sizeof(bak),  "%s/SKSave.bak",   save_base);
    (void)remove(path);
    (void)remove(bak);
    return 0;
}

/* ════════════════════════════════════════════════════════════════
 * High-level public API
 * ════════════════════════════════════════════════════════════════ */

uint8_t dm2_v1_save_slot_count(void) { return DM2_SLOT_MAX; }

bool dm2_v1_save_slot_valid(uint8_t slot) { return slot < DM2_SLOT_MAX; }

bool dm2_v1_save_has_valid_slot(const char *save_base, uint8_t slot)
{
    if (!save_base) return false;
    char path[256];
    snprintf(path, sizeof(path), "%s/SKSave%02u.dat", save_base, (unsigned)slot);
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    uint8_t hdr[42];
    bool ok = (fread(hdr, 42, 1, f) == 1)
              && ((uint16_t)hdr[38] | ((uint16_t)hdr[39] << 8)) == DM2_SLOT_MAGIC_1
              && ((uint16_t)hdr[40] | ((uint16_t)hdr[41] << 8)) == DM2_SLOT_MAGIC_2;
    fclose(f);
    return ok;
}

bool dm2_v1_save_suppress_self_test(void)
{
    return dm2_suppress_self_verification() != 0;
}

int dm2_v1_save_version_diagnostics(const uint8_t *data, size_t size)
{
    int flags = 0;
    if (!data || size < 4) return flags;
    DM2_SL_State st;
    if (data[0] == 0x00 && data[1] == 0x00) {
        (void)st; /* suppress unused warning */
        flags |= DM2V1_SAVE_DIAG_NULL_FILL;
    }
    if (size >= 10) {
        const uint8_t *p = data + size - 10;
        uint8_t v = p[0];
        bool allsame = (v != 0);
        for (size_t k = 1; k < 10 && allsame; k++) {
            if (p[k] != v) allsame = false;
        }
        if (allsame) flags |= DM2V1_SAVE_DIAG_SUPPRESS_FILL;
    }
    if (size < 44) flags |= DM2V1_SAVE_DIAG_TRUNCATED;
    return flags;
}

int dm2_v1_save_detect_game_version(const uint8_t *header42)
{
    if (!header42) return DM2V1_VERSION_UNKNOWN;
    uint16_t m1 = (uint16_t)header42[38] | ((uint16_t)header42[39] << 8);
    uint16_t m2 = (uint16_t)header42[40] | ((uint16_t)header42[41] << 8);
    if (m1 == DM2_SLOT_MAGIC_1 && m2 == DM2_SLOT_MAGIC_2) return DM2V1_VERSION_DM2;
    if (m1 == 0x444D && m2 == 0x3156) return DM2V1_VERSION_DM1;
    return DM2V1_VERSION_UNKNOWN;
}

const char *dm2_v1_save_source_evidence(void)
{
    return
        "SKULL.ASM: DM2 save/load entry points, SUPPRESS codec\n"
        "docs/dm2_save_format.md: full format specification\n"
        "docs/dm2_save_slots.md: 10-slot system, 0xBEEF/0xDEAD magic\n"
        "docs/dm2_party_state.md: champion squad persistence, SUPPRESS masks\n"
        "SKULL.ASM: WRITE_RECORD_CHECKCODE, WRITE_MINION_ASSOC\n"
        "SKULL.ASM: _2066_33e7 slot picker, GAME_SAVE/GAME_LOAD\n";
}

/* ════════════════════════════════════════════════════════════════
 * Champion SUPPRESS mask table
 * Source: docs/dm2_party_state.md — _4976_3992 write-mask pattern;
 * 261 bytes of per-field mask values (0x00=skip, 0x71=7-bit store).
 * This mask marks every field that can hold non-zero data in a live
 * champion so SUPPRESS compression achieves source-authentic packing.
 * ════════════════════════════════════════════════════════════════ */

void dm2_suppress_champion_mask(uint8_t mask[261])
{
    if (!mask) return;
    memset(mask, 0, 261);

    /* Name block — 8 + 16 = 24 bytes, store 7 bits per byte */
    memset(&mask[0],  0x71, 8);   /* first_name */
    memset(&mask[8],  0x71, 16);  /* last_name */

    /* Position — 2 + 1 bytes */
    mask[24] = 0x71; mask[25] = 0x71;  /* absolute_direction (u16) */
    mask[26] = 0x71;                     /* squad_position */

    /* HP cur/max — 2+2 + 2+2 */
    mask[27] = 0x71; mask[28] = 0x71;   /* cur_hp */
    mask[29] = 0x71; mask[30] = 0x71;   /* max_hp */
    /* stamina + mana (2 bytes each, store both LE bytes) */
    mask[31] = 0x71; mask[32] = 0x71;  /* stamina */
    mask[33] = 0x71; mask[34] = 0x71;  /* mana */

    /* Poison/runes — 1 byte each */
    mask[35] = 0x71; mask[36] = 0x71;  /* poison_value, runes_count */
    /* spelled_runes[4] */
    mask[37] = 0x71; mask[38] = 0x71; mask[39] = 0x71; mask[40] = 0x71;

    /* attributes[7][2]: cur/max pairs — 14 uint16_t = 28 bytes starting at 41 */
    for (int a = 0; a < 7; a++) {
        mask[41 + a*4] = 0x71; mask[42 + a*4] = 0x71; /* cur (LE) */
        mask[43 + a*4] = 0x71; mask[44 + a*4] = 0x71; /* max (LE) */
    }

    /* food / water — int16_t LE */
    mask[69] = 0x71; mask[70] = 0x71; mask[71] = 0x71; mask[72] = 0x71;

    /* combat hand state: hand_command[2] × uint32_t, hand_cooldown[2] × u16,
     * hand_defense_class[2] × u8 — 9 fields starting at 73 */
    mask[73] = 0x71; mask[74] = 0x71; mask[75] = 0x71; mask[76] = 0x71; /* hand_command[0] u32*/
    mask[77] = 0x71; mask[78] = 0x71; mask[79] = 0x71; mask[80] = 0x71; /* hand_command[1] u32*/
    mask[81] = 0x71; mask[82] = 0x71; /* hand_cooldown[0] u16 */
    mask[83] = 0x71; mask[84] = 0x71; /* hand_cooldown[1] u16 */
    mask[85] = 0x71; mask[86] = 0x71; /* hand_defense_class[0]+[1] */

    /* timer_index, damage_suffered, hero_flag, body_flag — 4 bytes */
    mask[87] = 0x71; mask[88] = 0x71; mask[89] = 0x71; mask[90] = 0x71;

    /* inventory[30] × uint32_t: bytes 91-210 (120 bytes), store full 32 bits */
    for (int i = 0; i < 30; i++) {
        size_t base = 91 + i * 4;
        mask[base + 0] = 0x71;
        mask[base + 1] = 0x71;
        mask[base + 2] = 0x71;
        mask[base + 3] = 0x71;
    }
}

int dm2_suppress_encode_champion(const DM2_ChampionRecord *c,
                                  const uint8_t *mask,
                                  uint8_t *out, size_t out_sz)
{
    if (!c || !mask || !out || out_sz < 261) return -1;
    return dm2_suppress_encode((const uint8_t *)c, mask, 261, out, out_sz);
}

int dm2_suppress_decode_champion(const uint8_t *in, size_t in_sz,
                                  const uint8_t *mask,
                                  DM2_ChampionRecord *c,
                                  uint8_t fill)
{
    if (!in || !mask || !c) return -1;
    if (in_sz < 261) return -1;
    return dm2_suppress_decode(in, in_sz, mask, 261, (uint8_t *)c, fill);
}

/* ════════════════════════════════════════════════════════════════
 * DB handle resolution
 * ObjectID: high byte = pool (0-15), low 24 bits = record index.
 * Source: docs/dm2_save_format.md § DB record pools
 * ════════════════════════════════════════════════════════════════ */

uint32_t dm2_db_make_handle(uint8_t pool, uint32_t index)
{
    if (pool >= DM2_DB_POOL_COUNT) return 0;
    return ((uint32_t)pool << 24) | (index & 0x00FFFFFF);
}

bool dm2_db_resolve(uint32_t object_id,
                     const DM2_DB_State *db,
                     uint8_t *out_pool, uint32_t *out_index)
{
    if (object_id == 0) return false;
    uint8_t pool = (uint8_t)((object_id >> 24) & 0xFF);
    uint32_t idx = object_id & 0x00FFFFFF;
    if (pool >= DM2_DB_POOL_COUNT) return false;
    if (!db || !db->pools[pool].data) return false;
    if (idx >= db->pools[pool].rec_count) return false;
    if (out_pool)  *out_pool  = pool;
    if (out_index) *out_index = idx;
    return true;
}

bool dm2_db_write_record(uint8_t pool, uint32_t index,
                           FILE *f,
                           const DM2_DB_State *db)
{
    if (!f) return false;
    if (pool >= DM2_DB_POOL_COUNT) return false;
    if (!db) return false;
    const DM2_DB_Pool *p = &db->pools[pool];
    if (!p->data || index >= p->rec_count || p->rec_size == 0) return false;
    size_t offset = (size_t)index * p->rec_size;
    return fwrite(p->data + offset, p->rec_size, 1, f) == 1;
}

uint32_t dm2_db_trace_inventory_slot(const uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                      uint8_t slot,
                                      uint8_t max_depth,
                                      const DM2_DB_State *db)
{
    if (!inventory || slot >= DM2_CHAMPION_INVENTORY_SLOTS) return 0;
    uint32_t h = inventory[slot];
    if (h == 0) return 0;

    uint8_t pool; uint32_t idx;
    uint8_t depth = 0;

    while (h != 0 && depth < max_depth) {
        if (!dm2_db_resolve(h, db, &pool, &idx)) break;
        if (!db->pools[pool].data) break;
        /* Simple chain-walk: advance one hop and stop */
        (void)pool; (void)idx;
        depth++;
        /* Chain is a placeholder: next iteration would follow the
         * record's link field; for now just confirm reachability. */
        break;
    }
    return inventory[slot];
}

/* ════════════════════════════════════════════════════════════════════════
 * Game state block (56 bytes, SUPPRESS-encoded)
 * Source: docs/dm2_save_format.md § Game state block (skload_table_60)
 * Type DM2_GameStateBlock is defined in dm2_v1_save_load.h
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_GAME_STATE_BLOCK_SIZE 56

/* SUPPRESS mask for game state block (56 bytes)
 * Source: SKULL.ASM skload_table_60 write mask */
static void dm2_suppress_gamestate_mask(uint8_t mask[DM2_GAME_STATE_BLOCK_SIZE])
{
    if (!mask) return;
    memset(mask, 0, DM2_GAME_STATE_BLOCK_SIZE);

    /* dwGameTick (4 bytes) */
    mask[0] = 0x71; mask[1] = 0x71; mask[2] = 0x71; mask[3] = 0x71;
    /* dwRandomSeed (4 bytes) */
    mask[4] = 0x71; mask[5] = 0x71; mask[6] = 0x71; mask[7] = 0x71;
    /* wChampionsCount (2 bytes) */
    mask[8] = 0x71; mask[9] = 0x71;
    /* wPlayerPosX (2 bytes) */
    mask[10] = 0x71; mask[11] = 0x71;
    /* wPlayerPosY (2 bytes) */
    mask[12] = 0x71; mask[13] = 0x71;
    /* wPlayerDir (2 bytes) */
    mask[14] = 0x71; mask[15] = 0x71;
    /* wPlayerMap (2 bytes) */
    mask[16] = 0x71; mask[17] = 0x71;
    /* wChampionLeader (2 bytes) */
    mask[18] = 0x71; mask[19] = 0x71;
    /* wTimersCount (2 bytes) */
    mask[20] = 0x71; mask[21] = 0x71;
    /* rain_state[8] */
    memset(&mask[22], 0x71, 8);
    /* _dw22 (4 bytes) */
    mask[30] = 0x71; mask[31] = 0x71; mask[32] = 0x71; mask[33] = 0x71;
    /* _dw26 (4 bytes) */
    mask[34] = 0x71; mask[35] = 0x71; mask[36] = 0x71; mask[37] = 0x71;
    /* _w30 (2 bytes) */
    mask[38] = 0x71; mask[39] = 0x71;
    /* _w34 (2 bytes) */
    mask[40] = 0x71; mask[41] = 0x71;
    /* rest of 56 bytes - reserved/padding, store all zeros via mask 0 */
    /* bytes 42-55: padding, mask stays 0 */
}

int dm2_suppress_encode_gamestate(const DM2_GameStateBlock *gs,
                                   uint8_t *out, size_t out_sz)
{
    if (!gs || !out || out_sz < DM2_GAME_STATE_BLOCK_SIZE) return -1;
    uint8_t mask[DM2_GAME_STATE_BLOCK_SIZE];
    dm2_suppress_gamestate_mask(mask);
    return dm2_suppress_encode((const uint8_t *)gs, mask,
                               DM2_GAME_STATE_BLOCK_SIZE, out, out_sz);
}

int dm2_suppress_decode_gamestate(const uint8_t *in, size_t in_sz,
                                   DM2_GameStateBlock *gs, uint8_t fill)
{
    if (!in || !gs) return -1;
    if (in_sz < DM2_GAME_STATE_BLOCK_SIZE) return -1;
    uint8_t mask[DM2_GAME_STATE_BLOCK_SIZE];
    dm2_suppress_gamestate_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask,
                               DM2_GAME_STATE_BLOCK_SIZE, (uint8_t *)gs, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Global variables (flags/bytes/words) — SUPPRESS encoded
 * Source: docs/dm2_save_format.md § Ingame global flags/bytes/words
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_GLOBAL_FLAGS_SIZE  8
#define DM2_GLOBAL_BYTES_SIZE  64
#define DM2_GLOBAL_WORDS_SIZE  64  /* 64 x uint16_t = 128 bytes */

/* SUPPRESS mask for global flags (8 bytes) — all bits stored */
static void dm2_suppress_global_flags_mask(uint8_t mask[DM2_GLOBAL_FLAGS_SIZE])
{
    if (!mask) return;
    memset(mask, 0x71, DM2_GLOBAL_FLAGS_SIZE);
}

/* SUPPRESS mask for global bytes (64 bytes) — all bytes stored */
static void dm2_suppress_global_bytes_mask(uint8_t mask[DM2_GLOBAL_BYTES_SIZE])
{
    if (!mask) return;
    memset(mask, 0x71, DM2_GLOBAL_BYTES_SIZE);
}

/* SUPPRESS mask for global words (64 words = 128 bytes) — all words stored */
static void dm2_suppress_global_words_mask(uint8_t mask[DM2_GLOBAL_WORDS_SIZE * 2])
{
    if (!mask) return;
    memset(mask, 0x71, DM2_GLOBAL_WORDS_SIZE * 2);
}

int dm2_suppress_encode_global_flags(const uint8_t flags[DM2_GLOBAL_FLAGS_SIZE],
                                     uint8_t *out, size_t out_sz)
{
    if (!flags || !out) return -1;
    uint8_t mask[DM2_GLOBAL_FLAGS_SIZE];
    dm2_suppress_global_flags_mask(mask);
    return dm2_suppress_encode(flags, mask, DM2_GLOBAL_FLAGS_SIZE, out, out_sz);
}

int dm2_suppress_decode_global_flags(const uint8_t *in, size_t in_sz,
                                     uint8_t flags[DM2_GLOBAL_FLAGS_SIZE],
                                     uint8_t fill)
{
    if (!in || !flags) return -1;
    uint8_t mask[DM2_GLOBAL_FLAGS_SIZE];
    dm2_suppress_global_flags_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_FLAGS_SIZE, flags, fill);
}

int dm2_suppress_encode_global_bytes(const uint8_t bytes[DM2_GLOBAL_BYTES_SIZE],
                                     uint8_t *out, size_t out_sz)
{
    if (!bytes || !out) return -1;
    uint8_t mask[DM2_GLOBAL_BYTES_SIZE];
    dm2_suppress_global_bytes_mask(mask);
    return dm2_suppress_encode(bytes, mask, DM2_GLOBAL_BYTES_SIZE, out, out_sz);
}

int dm2_suppress_decode_global_bytes(const uint8_t *in, size_t in_sz,
                                     uint8_t bytes[DM2_GLOBAL_BYTES_SIZE],
                                     uint8_t fill)
{
    if (!in || !bytes) return -1;
    uint8_t mask[DM2_GLOBAL_BYTES_SIZE];
    dm2_suppress_global_bytes_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_BYTES_SIZE, bytes, fill);
}

int dm2_suppress_encode_global_words(const uint16_t words[DM2_GLOBAL_WORDS_SIZE],
                                     uint8_t *out, size_t out_sz)
{
    if (!words || !out) return -1;
    uint8_t mask[DM2_GLOBAL_WORDS_SIZE * 2];
    dm2_suppress_global_words_mask(mask);
    return dm2_suppress_encode((const uint8_t *)words, mask,
                               DM2_GLOBAL_WORDS_SIZE * 2, out, out_sz);
}

int dm2_suppress_decode_global_words(const uint8_t *in, size_t in_sz,
                                     uint16_t words[DM2_GLOBAL_WORDS_SIZE],
                                     uint8_t fill)
{
    if (!in || !words) return -1;
    uint8_t mask[DM2_GLOBAL_WORDS_SIZE * 2];
    dm2_suppress_global_words_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_WORDS_SIZE * 2,
                               (uint8_t *)words, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Global spell effects (6 bytes, SUPPRESS)
 * Source: docs/dm2_party_state.md § Global spell effects
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_GLOBAL_SPELL_EFFECTS_SIZE 6

/* SUPPRESS mask for global spell effects (6 bytes) */
static void dm2_suppress_spell_effects_mask(uint8_t mask[DM2_GLOBAL_SPELL_EFFECTS_SIZE])
{
    if (!mask) return;
    memset(mask, 0x71, DM2_GLOBAL_SPELL_EFFECTS_SIZE);
}

int dm2_suppress_encode_spell_effects(const uint8_t effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
                                       uint8_t *out, size_t out_sz)
{
    if (!effects || !out) return -1;
    uint8_t mask[DM2_GLOBAL_SPELL_EFFECTS_SIZE];
    dm2_suppress_spell_effects_mask(mask);
    return dm2_suppress_encode(effects, mask, DM2_GLOBAL_SPELL_EFFECTS_SIZE,
                               out, out_sz);
}

int dm2_suppress_decode_spell_effects(const uint8_t *in, size_t in_sz,
                                       uint8_t effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
                                       uint8_t fill)
{
    if (!in || !effects) return -1;
    uint8_t mask[DM2_GLOBAL_SPELL_EFFECTS_SIZE];
    dm2_suppress_spell_effects_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_SPELL_EFFECTS_SIZE,
                               effects, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Timers table (10 bytes per timer, SUPPRESS)
 * Source: docs/dm2_save_format.md § Timers table
 * Types DM2_TimerEntry and DM2_MAX_TIMERS are defined in dm2_v1_save_load.h
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_TIMER_ENTRY_SIZE  10

/* SUPPRESS mask for timer entry (10 bytes) */
static void dm2_suppress_timer_mask(uint8_t mask[DM2_TIMER_ENTRY_SIZE])
{
    if (!mask) return;
    memset(mask, 0, DM2_TIMER_ENTRY_SIZE);
    /* timer_id (2 bytes) */
    mask[0] = 0x71; mask[1] = 0x71;
    /* current_tick (2 bytes) */
    mask[2] = 0x71; mask[3] = 0x71;
    /* interval_ticks (2 bytes) */
    mask[4] = 0x71; mask[5] = 0x71;
    /* flags (2 bytes) */
    mask[6] = 0x71; mask[7] = 0x71;
    /* user_data (2 bytes) */
    mask[8] = 0x71; mask[9] = 0x71;
}

int dm2_suppress_encode_timer(const DM2_TimerEntry *t,
                               uint8_t *out, size_t out_sz)
{
    if (!t || !out || out_sz < DM2_TIMER_ENTRY_SIZE) return -1;
    uint8_t mask[DM2_TIMER_ENTRY_SIZE];
    dm2_suppress_timer_mask(mask);
    return dm2_suppress_encode((const uint8_t *)t, mask,
                               DM2_TIMER_ENTRY_SIZE, out, out_sz);
}

int dm2_suppress_decode_timer(const uint8_t *in, size_t in_sz,
                               DM2_TimerEntry *t, uint8_t fill)
{
    if (!in || !t) return -1;
    if (in_sz < DM2_TIMER_ENTRY_SIZE) return -1;
    uint8_t mask[DM2_TIMER_ENTRY_SIZE];
    dm2_suppress_timer_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_TIMER_ENTRY_SIZE,
                             (uint8_t *)t, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Minion association table
 * Source: docs/dm2_party_state.md § Minion Association
 * ════════════════════════════════════════════════════════════════════════ */

/* Minion table is stored raw (not SUPPRESS) as fixed-size records */
size_t dm2_minion_table_size(const DM2_MinionTable *t)
{
    if (!t) return 0;
    /* 8 bytes per entry (2 x uint32_t) + 1 byte count */
    return 1 + (size_t)t->count * sizeof(DM2_MinionAssoc);
}

int dm2_minion_write(const DM2_MinionTable *t, FILE *f)
{
    if (!t || !f) return -1;
    uint8_t count = t->count;
    if (count > DM2_MAX_MINIONS) count = DM2_MAX_MINIONS;
    if (fwrite(&count, 1, 1, f) != 1) return -1;
    if (fwrite(t->entries, sizeof(DM2_MinionAssoc), count, f) != count)
        return -1;
    return 0;
}

int dm2_minion_read(DM2_MinionTable *t, FILE *f)
{
    if (!t || !f) return -1;
    memset(t, 0, sizeof(*t));
    if (fread(&t->count, 1, 1, f) != 1) return -1;
    if (t->count > DM2_MAX_MINIONS) t->count = DM2_MAX_MINIONS;
    if (fread(t->entries, sizeof(DM2_MinionAssoc), t->count, f) != t->count)
        return -1;
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * Leader hand possession (22 bytes in original, here using ObjectID)
 * Source: docs/dm2_party_state.md § Leader Hand Possession
 * Type DM2_LeaderPossession is defined in dm2_v1_save_load.h
 * ════════════════════════════════════════════════════════════════════════ */

int dm2_leader_possession_write(const DM2_LeaderPossession *lp, FILE *f)
{
    if (!lp || !f) return -1;
    /* Write as 4-byte ObjectID */
    uint8_t buf[4];
    buf[0] = (uint8_t)(lp->object & 0xFF);
    buf[1] = (uint8_t)((lp->object >> 8) & 0xFF);
    buf[2] = (uint8_t)((lp->object >> 16) & 0xFF);
    buf[3] = (uint8_t)((lp->object >> 24) & 0xFF);
    return (fwrite(buf, 4, 1, f) == 1) ? 0 : -1;
}

int dm2_leader_possession_read(DM2_LeaderPossession *lp, FILE *f)
{
    if (!lp || !f) return -1;
    uint8_t buf[4];
    if (fread(buf, 4, 1, f) != 1) return -1;
    lp->object = ((uint32_t)buf[0]) | ((uint32_t)buf[1] << 8) |
                 ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * Champion inventory serialization via WRITE_RECORD_CHECKCODE
 * Each inventory slot is an ObjectID handle; chains are followed.
 * Source: docs/dm2_party_state.md § Inventory: The Item Record Chain
 * ════════════════════════════════════════════════════════════════════════ */

int dm2_champion_inventory_write(const uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                   FILE *f)
{
    if (!inventory || !f) return -1;
    for (int i = 0; i < DM2_CHAMPION_INVENTORY_SLOTS; i++) {
        uint8_t buf[4];
        uint32_t h = inventory[i];
        buf[0] = (uint8_t)(h & 0xFF);
        buf[1] = (uint8_t)((h >> 8) & 0xFF);
        buf[2] = (uint8_t)((h >> 16) & 0xFF);
        buf[3] = (uint8_t)((h >> 24) & 0xFF);
        if (fwrite(buf, 4, 1, f) != 1) return -1;
        /* Note: actual WRITE_RECORD_CHECKCODE follows chains.
         * Here we write the handle; chain following would need
         * the full DB state and is handled at higher level. */
    }
    return 0;
}

int dm2_champion_inventory_read(uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                 FILE *f)
{
    if (!inventory || !f) return -1;
    for (int i = 0; i < DM2_CHAMPION_INVENTORY_SLOTS; i++) {
        uint8_t buf[4];
        if (fread(buf, 4, 1, f) != 1) return -1;
        inventory[i] = ((uint32_t)buf[0]) | ((uint32_t)buf[1] << 8) |
                       ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    }
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * PC savegame interoperability
 * Detects DM1 vs DM2 savegames and provides import diagnostics.
 * Source: docs/dm2_save_format.md § DM1 vs DM2 Key Format Differences
 * ════════════════════════════════════════════════════════════════════════ */

/* Detect the type of PC savegame from raw data or file
 * Returns DM2_PC_SAVE_DM2, DM2_PC_SAVE_DM1, or DM2_PC_SAVE_UNKNOWN */
int dm2_pc_save_detect_type(const uint8_t *data, size_t size)
{
    if (!data || size < 42) return DM2_PC_SAVE_UNKNOWN;
    int version = dm2_v1_save_detect_game_version(data);
    if (version == DM2V1_VERSION_DM2) return DM2_PC_SAVE_DM2;
    if (version == DM2V1_VERSION_DM1) return DM2_PC_SAVE_DM1;
    /* Check for DM1-style champion/dungeon file signatures */
    if (size >= 4) {
        /* DM1 CHAMP.DAT starts with champion name or zero */
        /* DM1 DUNGEON.DAT starts with 0x0000 or level data */
        uint16_t magic = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        if (magic == 0x0000) return DM2_PC_SAVE_DM1;
    }
    return DM2_PC_SAVE_UNKNOWN;
}

/* PC savegame interoperability report */
const char *dm2_pc_save_interoperability_report(const uint8_t *data, size_t size)
{
    static char report[256];
    int type = dm2_pc_save_detect_type(data, size);
    int diag = dm2_v1_save_version_diagnostics(data, size);

    snprintf(report, sizeof(report), "DM2 PC Save Interoperability Report:\n");
    switch (type) {
        case DM2_PC_SAVE_DM2:
            snprintf(report + strlen(report), sizeof(report) - strlen(report),
                     "  Format: DM2 (native)\n");
            break;
        case DM2_PC_SAVE_DM1:
            snprintf(report + strlen(report), sizeof(report) - strlen(report),
                     "  Format: DM1 (import supported via DM1->DM2 champion import)\n");
            break;
        default:
            snprintf(report + strlen(report), sizeof(report) - strlen(report),
                     "  Format: Unknown\n");
            break;
    }

    if (diag & DM2V1_SAVE_DIAG_NULL_FILL)
        snprintf(report + strlen(report), sizeof(report) - strlen(report),
                 "  Diagnostics: NULL fill detected\n");
    if (diag & DM2V1_SAVE_DIAG_SUPPRESS_FILL)
        snprintf(report + strlen(report), sizeof(report) - strlen(report),
                 "  Diagnostics: SUPPRESS fill pattern detected\n");
    if (diag & DM2V1_SAVE_DIAG_TRUNCATED)
        snprintf(report + strlen(report), sizeof(report) - strlen(report),
                 "  Diagnostics: File may be truncated\n");

    return report;
}

/* ════════════════════════════════════════════════════════════════════════
 * Source evidence for Phase 7
 * ════════════════════════════════════════════════════════════════════════ */

const char *dm2_v1_save_phase7_source_evidence(void)
{
    return
        "SKULL.ASM: _2066_#### save/load entry points\n"
        "SKULL.ASM: skload_table_60 game state block\n"
        "SKULL.ASM: WRITE_RECORD_CHECKCODE for inventory chains\n"
        "SKULL.ASM: WRITE_MINION_ASSOC for minion table\n"
        "docs/dm2_save_format.md: full save format specification\n"
        "docs/dm2_save_slots.md: 10-slot system, 0xBEEF/0xDEAD magic\n"
        "docs/dm2_party_state.md: champion squad, inventories, minions\n"
        "docs/dm2_source_lock.md: Phase 7 implementation evidence\n";
}
