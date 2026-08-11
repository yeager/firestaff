#ifndef FIRESTAFF_DM2_V1_DUNGEON_INPUT_OWNER_H
#define FIRESTAFF_DM2_V1_DUNGEON_INPUT_OWNER_H

/*
 * Source-owned PC DM2 dungeon pointer route.
 *
 * The route table is the relocated MOUSE_INPUT inventory from SKWIN's
 * skval1.h, decoded against the PC-English GRAPHICS.DAT rectangle pool.
 * The caller must supply the scanner's immutable GRAPHICS.DAT MD5; the
 * route is deliberately unavailable for a different edition instead of
 * borrowing a geometry table from DM1, FM Towns, or Amiga.
 *
 * Source: SKWIN skval1.h:92/109/110/113/114; SkWinCore.cpp:12131-12242,
 * 15239-15303, 55037-55120 (DM2_1031_07d6 / _030a / _0a88).
 */

#include <stdint.h>

#include "dm2_touch_click_zone_matrix_pc34_compat.h"
#include "dm2_v1_boot.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_DUNGEON_INPUT_PC_EN_GRAPHICS_MD5 \
    "25247ede4dabb6a71e5dabdfbcd5907d"

typedef struct {
    int active;
    int fmtowns;
    char graphics_md5[33];
    uint32_t source_table_hash;
    const DM2_V1_BootProfile *boot_profile;
} DM2_V1_DungeonInputOwner;

typedef struct {
    int accepted;
    int blocked_unverified_graphics;
    int blocked_no_source_zone;
    uint16_t event_index;
    uint16_t source_zone_index;
    int16_t source_x;
    int16_t source_y;
    int16_t source_w;
    int16_t source_h;
    uint32_t source_table_hash;
} DM2_V1_DungeonInputReceipt;

/* One raw record from the authenticated FM Towns SKULL.EXP MOUSE_INPUT
 * span.  event_index removes the source high-bit flag from the event word;
 * event_flags preserves it so callers cannot silently lose source state.
 * This is a candidate inventory, not a context-free hit-test: the same
 * rectangle can occur in more than one source UI branch. */
typedef struct {
    uint16_t source_record_index;
    uint16_t event_index;
    uint16_t event_flags;
    uint16_t rect_id;
    uint16_t button_mask;
} DM2_V1_FmtownsMouseInputCandidate;

/* A native MOUSE_INPUT record is shared by several source UI branches.
 * Callers must name the branch whose live owner is active; a raw rectangle
 * is never promoted to a global hitbox. */
typedef enum {
    DM2_V1_FMTOWNS_UI_DUNGEON = 1,
    DM2_V1_FMTOWNS_UI_INVENTORY,
    DM2_V1_FMTOWNS_UI_STATUS,
    DM2_V1_FMTOWNS_UI_DIALOGUE
} DM2_V1_FmtownsUiContext;

typedef struct {
    int accepted;
    DM2_V1_FmtownsUiContext context;
    DM2_V1_FmtownsMouseInputCandidate candidate;
    Dm2TouchClickZonePc34Compat source_context;
    DM2_V1_BootExpandedRectReceipt native_rect;
    uint32_t source_table_hash;
} DM2_V1_FmtownsUiRouteReceipt;

unsigned int dm2_v1_dungeon_input_owner_fmtowns_candidate_count(
    const DM2_V1_DungeonInputOwner *owner);

int dm2_v1_dungeon_input_owner_fmtowns_candidate(
    const DM2_V1_DungeonInputOwner *owner,
    unsigned int ordinal,
    DM2_V1_FmtownsMouseInputCandidate *out_candidate);

/* Bind a raw Towns record to the source route's UI context.  The returned
 * geometry is PC reference metadata only; FM Towns callers must resolve the
 * candidate's rect_id through the authenticated native RAW4 table. */
int dm2_v1_dungeon_input_owner_fmtowns_candidate_context(
    const DM2_V1_DungeonInputOwner *owner,
    unsigned int ordinal,
    Dm2TouchClickZonePc34Compat *out_context);
unsigned int dm2_v1_dungeon_input_owner_fmtowns_candidate_context_count(
    const DM2_V1_DungeonInputOwner *owner, unsigned int ordinal);
int dm2_v1_dungeon_input_owner_fmtowns_candidate_context_at(
    const DM2_V1_DungeonInputOwner *owner, unsigned int ordinal,
    unsigned int context_ordinal, Dm2TouchClickZonePc34Compat *out_context);

/* Resolve a candidate's rectangle from the authenticated FM Towns RAW4
 * table. No PC coordinates are returned or used. */
int dm2_v1_dungeon_input_owner_fmtowns_candidate_native_rect(
    const DM2_V1_DungeonInputOwner *owner,
    unsigned int ordinal,
    DM2_V1_BootExpandedRectReceipt *out_rect);

/* Resolve one authenticated Towns pointer through an explicitly selected
 * source UI branch.  The returned source_context is semantic provenance
 * only; native_rect is the sole geometry used for the Towns hit-test. */
int dm2_v1_dungeon_input_owner_fmtowns_route_context(
    const DM2_V1_DungeonInputOwner *owner,
    DM2_V1_FmtownsUiContext context,
    int16_t screen_x, int16_t screen_y, unsigned int button_mask,
    DM2_V1_FmtownsUiRouteReceipt *out_receipt);

typedef void (*DM2_V1_DungeonInputEventSink)(void *ctx,
                                              int16_t event_index,
                                              int16_t x,
                                              int16_t y);

/* Activates the PC-English table only after asset discovery has supplied
 * the exact GRAPHICS.DAT MD5.  It does not read, extract, or manufacture
 * game data. */
int dm2_v1_dungeon_input_owner_init(DM2_V1_DungeonInputOwner *owner,
                                    const char *graphics_md5);

/* Activates only the source event/rect subset recovered for the authentic
 * FM-Towns GRAPHICS.DAT: movement, action-panel command slots and hand
 * selection. Rectangles are queried from the boot profile's authenticated
 * INTERFACE_GENERAL RAW4 table; no PC geometry is reused. */
int dm2_v1_dungeon_input_owner_init_fmtowns(
    DM2_V1_DungeonInputOwner *owner,
    const DM2_V1_BootProfile *profile);

/* Routes one physical mouse/touch button through the source dungeon view.
 * On a hit, exactly the source event number and source-space pointer
 * coordinates are passed to c_input's eventual event sink. */
int dm2_v1_dungeon_input_owner_route(
    const DM2_V1_DungeonInputOwner *owner,
    int16_t screen_x,
    int16_t screen_y,
    unsigned int button_mask,
    DM2_V1_DungeonInputEventSink sink,
    void *sink_ctx,
    DM2_V1_DungeonInputReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
