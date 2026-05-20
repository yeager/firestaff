/* DM1 V1 Save/Load System — source-locked from ReDMCSB
 * LOADSAVE.C: F0433 save game, F0434 load game
 * DECOMPDU.C G0525/G0526: game_id/dungeon_id validation
 * DECOMPDU.C G0534: additional save header data (134 bytes) */

#include "dm1_v1_save_load_system_pc34_compat.h"
#include <stdio.h>
#include <string.h>

uint8_t m11_sl_source_runtime_slot_count(void) {
    return DM1_SL_SOURCE_RUNTIME_SLOT_COUNT;
}

bool m11_sl_source_runtime_slot_supported(uint8_t slot) {
    return slot == DM1_SL_SOURCE_RUNTIME_SLOT;
}

void m11_sl_init(M11_SL_State* state, const char* save_dir) {
    if (!state) return;
    memset(state, 0, sizeof(M11_SL_State));
    if (save_dir && save_dir[0] != 0) {
        size_t len = strlen(save_dir);
        if (len >= sizeof(state->save_dir)) len = sizeof(state->save_dir) - 1;
        memcpy(state->save_dir, save_dir, len);
        state->save_dir[len] = 0;
    } else {
        state->save_dir[0] = '.';
        state->save_dir[1] = 0;
    }
    state->initialized = true;
}

static void slot_path(const M11_SL_State* state, uint8_t slot, char* buf, size_t bufsz) {
    snprintf(buf, bufsz, "%s/save_%02u.dat", state->save_dir, (unsigned)slot);
}

bool m11_sl_scan_slots(M11_SL_State* state) {
    if (!state || !state->initialized) return false;

    state->slot_count = 0;
    for (uint8_t i = 0; i < DM1_SL_MAX_SLOTS; i++) {
        char path[512];
        slot_path(state, i, path, sizeof(path));

        FILE* f = fopen(path, "rb");
        if (!f) {
            state->slots[i].occupied = false;
            memset(&state->slots[i].header, 0, sizeof(state->slots[i].header));
            state->slots[i].label[0] = 0;
            state->slots[i].timestamp = 0;
            continue;
        }

        M11_SL_SaveHeader hdr;
        if (fread(&hdr, sizeof(hdr), 1, f) == 1 && hdr.magic == DM1_SL_SAVE_MAGIC) {
            state->slots[i].occupied = true;
            state->slots[i].header = hdr;
            snprintf(state->slots[i].label, sizeof(state->slots[i].label),
                     "Level %u (%d,%d)", hdr.current_level, hdr.party_x, hdr.party_y);
            state->slot_count++;
        } else {
            state->slots[i].occupied = false;
            memset(&state->slots[i].header, 0, sizeof(state->slots[i].header));
            state->slots[i].label[0] = 0;
            state->slots[i].timestamp = 0;
        }
        fclose(f);
    }
    return true;
}

bool m11_sl_save(M11_SL_State* state, uint8_t slot,
                  const M11_SL_SaveHeader* header,
                  const uint8_t* data, size_t data_size) {
    if (!state || !state->initialized || !header) return false;
    if (data_size > 0 && !data) return false;
    if (slot >= DM1_SL_MAX_SLOTS) return false;

    char path[512];
    slot_path(state, slot, path, sizeof(path));

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    M11_SL_SaveHeader hdr = *header;
    hdr.magic = DM1_SL_SAVE_MAGIC;
    hdr.data_size = (uint32_t)data_size;

    bool ok = (fwrite(&hdr, sizeof(hdr), 1, f) == 1) &&
              (data_size == 0 || fwrite(data, 1, data_size, f) == data_size);
    fclose(f);

    if (ok) {
        bool was_occupied = state->slots[slot].occupied;
        state->slots[slot].occupied = true;
        state->slots[slot].header = hdr;
        state->slots[slot].timestamp = hdr.game_time;
        snprintf(state->slots[slot].label, sizeof(state->slots[slot].label),
                 "Level %u (%d,%d)", hdr.current_level, hdr.party_x, hdr.party_y);
        if (!was_occupied && state->slot_count < DM1_SL_MAX_SLOTS) {
            state->slot_count++;
        }
    }
    return ok;
}

bool m11_sl_load_header(M11_SL_State* state, uint8_t slot,
                         M11_SL_SaveHeader* header) {
    if (!state || !state->initialized || !header) return false;
    if (slot >= DM1_SL_MAX_SLOTS) return false;

    char path[512];
    slot_path(state, slot, path, sizeof(path));

    FILE* f = fopen(path, "rb");
    if (!f) return false;

    bool ok = (fread(header, sizeof(*header), 1, f) == 1) &&
              (header->magic == DM1_SL_SAVE_MAGIC);
    fclose(f);
    return ok;
}

bool m11_sl_load_data(M11_SL_State* state, uint8_t slot,
                       uint8_t* data, size_t max_size, size_t* actual_size) {
    if (!state || !state->initialized || !data) return false;
    if (slot >= DM1_SL_MAX_SLOTS) return false;

    char path[512];
    slot_path(state, slot, path, sizeof(path));

    FILE* f = fopen(path, "rb");
    if (!f) return false;

    M11_SL_SaveHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1 || hdr.magic != DM1_SL_SAVE_MAGIC) {
        fclose(f);
        return false;
    }

    if (hdr.data_size > max_size) {
        if (actual_size) *actual_size = hdr.data_size;
        fclose(f);
        return false;
    }

    size_t to_read = hdr.data_size;
    size_t got = fread(data, 1, to_read, f);
    fclose(f);

    if (actual_size) *actual_size = got;
    return (got == to_read);
}

bool m11_sl_delete(M11_SL_State* state, uint8_t slot) {
    if (!state || !state->initialized) return false;
    if (slot >= DM1_SL_MAX_SLOTS) return false;

    char path[512];
    slot_path(state, slot, path, sizeof(path));

    if (remove(path) == 0) {
        bool was_occupied = state->slots[slot].occupied;
        state->slots[slot].occupied = false;
        memset(&state->slots[slot].header, 0, sizeof(M11_SL_SaveHeader));
        state->slots[slot].label[0] = 0;
        state->slots[slot].timestamp = 0;
        if (was_occupied && state->slot_count > 0) {
            state->slot_count--;
        }
        return true;
    }
    return false;
}

bool m11_sl_slot_occupied(const M11_SL_State* state, uint8_t slot) {
    if (!state || slot >= DM1_SL_MAX_SLOTS) return false;
    return state->slots[slot].occupied;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — LOADSAVE.C remaining function citations
 *
 *   LOADSAVE.C:2094 F0079_CPSC_G
 *   LOADSAVE.C:356 F0414_SAVEUTIL_R
 *   LOADSAVE.C:1932 F0415_SAVEUTIL_I
 *   LOADSAVE.C:1632 F0416_SAVEUTIL_I
 *   LOADSAVE.C:1613 F0418_SAVEUTIL_G
 *   LOADSAVE.C:2721 F0419_SAVEUTIL_I
 *   LOADSAVE.C:1626 F0420_SAVEUTIL_I
 *   LOADSAVE.C:1661 F0422_SAVEUTIL_I
 *   LOADSAVE.C:2177 F0423_SAVEUTIL_F
 *   LOADSAVE.C:278 F0432_STARTEND_F
 *   LOADSAVE.C:308 F0452_FLOPPY_G
 *   LOADSAVE.C:497 F0453_FLOPPY_I
 *   LOADSAVE.C:396 F0454_FLOPPY_I
 *   LOADSAVE.C:1502 F0515_CHAMPION_C
 *   LOADSAVE.C:1568 F0516_CHAMPION_C
 *   LOADSAVE.C:373 F0528_FLOPPY_E
 *   LOADSAVE.C:1483 F0776_FILE_C
 *   LOADSAVE.C:1851 F2235_MUSIC_S
 * ══════════════════════════════════════════════════════════════════════ */

