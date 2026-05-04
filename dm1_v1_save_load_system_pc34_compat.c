/* DM1 V1 Save/Load System — source-locked from ReDMCSB
 * LOADSAVE.C: F0433 save game, F0434 load game
 * DECOMPDU.C G0525/G0526: game_id/dungeon_id validation
 * DECOMPDU.C G0534: additional save header data (134 bytes) */

#include "dm1_v1_save_load_system_pc34_compat.h"
#include <stdio.h>
#include <string.h>

void m11_sl_init(M11_SL_State* state, const char* save_dir) {
    if (!state) return;
    memset(state, 0, sizeof(M11_SL_State));
    if (save_dir) {
        size_t len = strlen(save_dir);
        if (len >= sizeof(state->save_dir)) len = sizeof(state->save_dir) - 1;
        memcpy(state->save_dir, save_dir, len);
        state->save_dir[len] = '\0';
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
        }
        fclose(f);
    }
    return true;
}

bool m11_sl_save(M11_SL_State* state, uint8_t slot,
                  const M11_SL_SaveHeader* header,
                  const uint8_t* data, size_t data_size) {
    if (!state || !state->initialized || !header || !data) return false;
    if (slot >= DM1_SL_MAX_SLOTS) return false;

    char path[512];
    slot_path(state, slot, path, sizeof(path));

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    M11_SL_SaveHeader hdr = *header;
    hdr.magic = DM1_SL_SAVE_MAGIC;
    hdr.data_size = (uint32_t)data_size;

    bool ok = (fwrite(&hdr, sizeof(hdr), 1, f) == 1) &&
              (fwrite(data, 1, data_size, f) == data_size);
    fclose(f);

    if (ok) {
        state->slots[slot].occupied = true;
        state->slots[slot].header = hdr;
        snprintf(state->slots[slot].label, sizeof(state->slots[slot].label),
                 "Level %u (%d,%d)", hdr.current_level, hdr.party_x, hdr.party_y);
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

    size_t to_read = (hdr.data_size < max_size) ? hdr.data_size : max_size;
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
        state->slots[slot].occupied = false;
        memset(&state->slots[slot].header, 0, sizeof(M11_SL_SaveHeader));
        state->slot_count--;
        return true;
    }
    return false;
}

bool m11_sl_slot_occupied(const M11_SL_State* state, uint8_t slot) {
    if (!state || slot >= DM1_SL_MAX_SLOTS) return false;
    return state->slots[slot].occupied;
}
