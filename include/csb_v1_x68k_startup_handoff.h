#ifndef FIRESTAFF_CSB_V1_X68K_STARTUP_HANDOFF_H
#define FIRESTAFF_CSB_V1_X68K_STARTUP_HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#include "asset_loader_m11.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_x68k_source_media.h"

/* Native-source admission for the X68000 HDM.  It joins the read-only media
 * receipt, the X68000-labelled M11 graphics cache and the source dungeon in
 * one owned object.  This is deliberately not a Human68k program loader or
 * an authenticity classification for a cracked image. */
typedef struct {
    int admitted;
    int x68000_identity_bound;
    int host_program_execution_permitted;
    CSB_V1_X68kSourceMediaReceipt media;
    M11_AssetLoader graphics;
    CSB_V1_DungeonData dungeon;
    int initial_level;
    int initial_x;
    int initial_y;
    int initial_direction;
} CSB_V1_X68kStartupHandoff;

typedef enum {
    CSB_V1_X68K_STARTUP_HANDOFF_OK = 0,
    CSB_V1_X68K_STARTUP_HANDOFF_ERR_ARGUMENT = -1,
    CSB_V1_X68K_STARTUP_HANDOFF_ERR_MEDIA = -2,
    CSB_V1_X68K_STARTUP_HANDOFF_ERR_GRAPHICS = -3,
    CSB_V1_X68K_STARTUP_HANDOFF_ERR_DUNGEON = -4,
    CSB_V1_X68K_STARTUP_HANDOFF_ERR_POSE = -5
} CSB_V1_X68kStartupHandoffResult;

/* The caller retains the HDM bytes. On success out_handoff owns only decoded
 * graphics and dungeon state and must be released with cleanup(). */
int csb_v1_x68k_startup_handoff_admit(
    CSB_V1_X68kStartupHandoff *out_handoff,
    const uint8_t *hdm, size_t hdm_size);

void csb_v1_x68k_startup_handoff_cleanup(
    CSB_V1_X68kStartupHandoff *handoff);

#endif /* FIRESTAFF_CSB_V1_X68K_STARTUP_HANDOFF_H */
