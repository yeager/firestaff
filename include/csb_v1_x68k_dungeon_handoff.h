#ifndef FIRESTAFF_CSB_V1_X68K_DUNGEON_HANDOFF_H
#define FIRESTAFF_CSB_V1_X68K_DUNGEON_HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_x68k_hdm.h"

/* Read the X68000 HDM's DUNGEON.DAT through the Human68k FAT12 boundary and
 * hand the exact bytes to the existing CSB source-dungeon loader. X68000
 * DUNGEON.DAT is the big-endian 0x8104 compressed form already handled by
 * that loader; this adapter does not reinterpret maps or synthesize a world. */

typedef enum {
    CSB_V1_X68K_DUNGEON_HANDOFF_OK = 0,
    CSB_V1_X68K_DUNGEON_HANDOFF_ERR_ARGUMENT = -1,
    CSB_V1_X68K_DUNGEON_HANDOFF_ERR_MEDIA = -2,
    CSB_V1_X68K_DUNGEON_HANDOFF_ERR_LOAD = -3
} CSB_V1_X68kDungeonHandoffResult;

/* On success, out_dungeon owns its decoded source bytes and must be released
 * with csb_v1_dungeon_free(). No raw HDM or compressed dungeon buffer remains
 * allocated by this adapter after it returns. */
int csb_v1_x68k_hdm_load_dungeon(CSB_V1_DungeonData *out_dungeon,
                                 const uint8_t *hdm, size_t hdm_size,
                                 CSB_V1_X68kHdmReceipt *out_receipt);

#endif /* FIRESTAFF_CSB_V1_X68K_DUNGEON_HANDOFF_H */
