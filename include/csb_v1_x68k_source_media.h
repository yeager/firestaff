#ifndef FIRESTAFF_CSB_V1_X68K_SOURCE_MEDIA_H
#define FIRESTAFF_CSB_V1_X68K_SOURCE_MEDIA_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_x68k_dungeon_handoff.h"
#include "csb_v1_x68k_enter_sng.h"
#include "csb_v1_x68k_graphics_handoff.h"
#include "csb_v1_x68k_autoexec.h"
#include "csb_v1_x68k_program.h"

/* One read-only receipt for the X68000 CSB startup media: GRAPHICS.DAT,
 * DUNGEON.DAT, ENTER.SNG, AUTOEXEC.BAT and CHAOS_ST.X. It joins their existing
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
    CSB_V1_X68kAutoexecReceipt autoexec;
    CSB_V1_X68kProgramReceipt program;
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
    CSB_V1_X68K_SOURCE_MEDIA_ERR_HASH = -6,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_AUTOEXEC = -7,
    CSB_V1_X68K_SOURCE_MEDIA_ERR_PROGRAM = -8
} CSB_V1_X68kSourceMediaResult;

/* Validate startup command order and program layout as well as the complete
 * source-media set, without retaining original game bytes. */
int csb_v1_x68k_hdm_source_media_receipt(
    const uint8_t *hdm, size_t hdm_size,
    CSB_V1_X68kSourceMediaReceipt *out_receipt);

#endif /* FIRESTAFF_CSB_V1_X68K_SOURCE_MEDIA_H */
