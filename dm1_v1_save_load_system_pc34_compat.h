/* DM1 V1 Save/Load System — source-locked from ReDMCSB
 * LOADSAVE.C: save game file format, dungeon state serialization
 * DECOMPDU.C G0534_ac_SaveHeaderAdditionalData[134]: save header extra data
 * DECOMPDU.C G0525_l_GameID: game identification for save validation */
#ifndef FIRESTAFF_DM1_V1_SAVE_LOAD_SYSTEM_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SAVE_LOAD_SYSTEM_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_SL_SAVE_MAGIC        0x444D3156  /* "DM1V" */
#define DM1_SL_HEADER_SIZE       256
#define DM1_SL_ADDITIONAL_DATA   134  /* G0534 size */
#define DM1_SL_MAX_SLOTS         20

typedef struct {
    uint32_t magic;               /* DM1_SL_SAVE_MAGIC */
    uint32_t game_id;             /* G0525 mirror */
    uint16_t dungeon_id;          /* G0526 mirror */
    int16_t  platform;            /* G0527 mirror */
    int16_t  format;              /* G0528 mirror */
    uint16_t current_level;
    int16_t  party_x, party_y;
    uint8_t  party_facing;
    uint32_t game_time;           /* Ticks since game start */
    uint32_t data_size;           /* Size of save data following header */
    char     additional[DM1_SL_ADDITIONAL_DATA]; /* G0534 pattern */
} M11_SL_SaveHeader;

typedef struct {
    bool     occupied;
    char     label[32];           /* Display name for slot */
    uint32_t timestamp;           /* Real-time save timestamp */
    M11_SL_SaveHeader header;
} M11_SL_SlotInfo;

typedef struct {
    M11_SL_SlotInfo slots[DM1_SL_MAX_SLOTS];
    uint8_t slot_count;
    char    save_dir[256];        /* Directory for save files */
    bool    initialized;
} M11_SL_State;

void m11_sl_init(M11_SL_State* state, const char* save_dir);
bool m11_sl_scan_slots(M11_SL_State* state);
bool m11_sl_save(M11_SL_State* state, uint8_t slot,
                  const M11_SL_SaveHeader* header,
                  const uint8_t* data, size_t data_size);
bool m11_sl_load_header(M11_SL_State* state, uint8_t slot,
                         M11_SL_SaveHeader* header);
bool m11_sl_load_data(M11_SL_State* state, uint8_t slot,
                       uint8_t* data, size_t max_size, size_t* actual_size);
bool m11_sl_delete(M11_SL_State* state, uint8_t slot);
bool m11_sl_slot_occupied(const M11_SL_State* state, uint8_t slot);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SAVE_LOAD_SYSTEM_PC34_COMPAT_H */
