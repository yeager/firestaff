#ifndef FIRESTAFF_CSB_V1_X68K_SOURCE_MEDIA_H
#define FIRESTAFF_CSB_V1_X68K_SOURCE_MEDIA_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_x68k_dungeon_handoff.h"
#include "csb_v1_x68k_enter_sng.h"
#include "csb_v1_x68k_graphics_handoff.h"

/* One read-only receipt for the three X68000 CSB source resources used at
 * startup: GRAPHICS.DAT, DUNGEON.DAT and ENTER.SNG. It joins their existing
 * bounded readers at the raw Human68k HDM boundary, so the caller cannot
 * mistake the shared DMCSB2 graphics byte layout for an Amiga boot profile.
 *
 * A successful receipt proves that this particular HDM is structurally
 * readable and that its source resources reach their respective Firestaff
 * boundaries. It deliberately makes neither an authenticity claim nor a
 * native-X68000-runtime claim. ReDMCSB MEMORY.C's X68000 route and the
 * X68000 executable/disassembly remain required before live boot is enabled.
 */
typedef struct {
    CSB_V1_X68kHdmReceipt media;
    CSB_V1_X68kGraphicsReceipt graphics;
    CSB_V1_X68kEnterSngReceipt entrance_music;
    char hdm_sha256[65];
    uint16_t dungeon_level_count;
    uint16_t dungeon_square_bytes;
    int initial_party_level;
    int initial_party_x;
    int initial_party_y;
    int initial_party_direction;
    int x68000_identity_bound;
    int shared_graphics_layout_only;
    int authenticity_claimed;
    int native_runtime_launch_permitted;
} CSB_V1_X68kSourceMediaReceipt;

typedef enum {
    CSB_V1_X68K_SOURCE_MEDIA_OK = 0,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_ARGUMENT = -1,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_HDM = -2,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_GRAPHICS = -3,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_DUNGEON = -4,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_ENTRANCE_MUSIC = -5,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_HASH = -6
} CSB_V1_X68kSourceMediaResult;

/* Validate the complete source-media set without retaining original game
 * bytes. The receipt is zeroed before any failure is returned. */
int csb_v1_x68k_hdm_source_media_receipt(
    const uint8_t *hdm, size_t hdm_size,
    CSB_V1_X68kSourceMediaReceipt *out_receipt);

#endif /* FIRESTAFF_CSB_V1_X68K_SOURCE_MEDIA_H */
