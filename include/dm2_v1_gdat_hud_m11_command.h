#ifndef FIRESTAFF_DM2_V1_GDAT_HUD_M11_COMMAND_H
#define FIRESTAFF_DM2_V1_GDAT_HUD_M11_COMMAND_H

/* Fail-closed original GDAT HUD command bridge for the M11 host. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_viewport_renderer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DM2_V1_GDAT_HUD_M11_COMMAND_TOP_BAR = 1,
    DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_STRIP,
    DM2_V1_GDAT_HUD_M11_COMMAND_GOLD_BOX,
    DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
    DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL,
    DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT
};

#define DM2_V1_GDAT_HUD_M11_COMMAND_MAX 13
/* The static source family contains only the four surfaces whose exact
 * address is independently retained here. Hand/action backdrops are dynamic:
 * DRAW_HAND_ACTION_ICONS selects INTERFACE_GENERAL/4 fields 2..5 and
 * RECT_46..RECT_4d from live party placement, so they must never be folded
 * into this static plan. */
#define DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT 4
#define DM2_V1_GDAT_HUD_M11_STATIC_OUTDOOR_COMMAND_COUNT 3

typedef struct DM2_V1_GdatHudM11Command {
    int kind;
    int gdat_category;
    int gdat_index;
    int gdat_field;
    int viewport_gdat_index;
    DM2_V1_ViewportRect destination;
    uint16_t destination_rect_id;
    uint32_t destination_table_hash;
    uint8_t *pixels;
    int width;
    int height;
    DM2_ImageFormat format;
    uint8_t palette16[16];
    uint32_t raw_hash;
    uint32_t decoded_hash;
    uint32_t raw_byte_count;
    uint32_t palette_hash;
    uint16_t material_raw_index;
    const uint8_t *material_source_bytes;
    size_t material_source_byte_count;
    uint32_t material_receipt_hash;
} DM2_V1_GdatHudM11Command;

typedef struct DM2_V1_GdatHudM11CommandPlan {
    int valid;
    int command_count;
    uint32_t command_hash;
    DM2_V1_GdatHudM11Command commands[DM2_V1_GDAT_HUD_M11_COMMAND_MAX];
} DM2_V1_GdatHudM11CommandPlan;

typedef struct {
    int valid;
    int no_draw;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint16_t destination_rect_id;
    uint32_t destination_hash;
    uint32_t identity_hash;
} DM2_V1_GdatHudSummaryM11Receipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t scale_x;
    uint16_t scale_y;
    uint16_t destination_rect_id;
    uint32_t summary_identity_hash;
    uint32_t destination_hash;
    uint32_t identity_hash;
} DM2_V1_GdatHudPicstTransformReceipt;

typedef struct {
    int valid;
    int drawn;
    uint16_t destination_rect_id;
    uint16_t width;
    uint16_t height;
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t summary_identity_hash;
    uint32_t transform_identity_hash;
    uint32_t destination_hash;
    uint32_t identity_hash;
} DM2_V1_GdatHudPicstDrawReceipt;

/* Builds the HUD image family from a verified original GRAPHICS.DAT. Every
 * command carries decoded source pixels, its local palette, exact GDAT address,
 * and M11 destination. A missing, malformed, or non-local-palette material
 * rejects the entire plan. When is_outdoor is non-zero the right-side portrait
 * panel is omitted because DM2 outdoor mode does not draw it. */
int dm2_v1_gdat_hud_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader,
    int is_outdoor,
    DM2_V1_GdatHudM11CommandPlan *out_plan);

/* Extends the same verified HUD family with 0..4 occupied party portraits.
 * A missing CHAMPIONS/HeroType/0 image follows SKProject's exact
 * MISCELLANEOUS/254/IMG/254 fallback; every other absent or malformed
 * material rejects the plan rather than replacing it with a coloured slot. */
int dm2_v1_gdat_hud_m11_command_plan_build_for_party(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HudPartyState *party,
    DM2_V1_GdatHudM11CommandPlan *out_plan);

/* Binds original INTERFACE_GENERAL/0/dt04 portrait rectangles
 * (RECT_173..RECT_176) to occupied portraits in squad order. */
int dm2_v1_gdat_hud_m11_command_plan_bind_portrait_destinations(
    DM2_V1_GdatHudM11CommandPlan *plan,
    const DM2_V1_HudPartyState *party,
    const DM2_V1_ViewportRect portrait_destinations[4],
    uint32_t source_table_hash);
uint32_t dm2_v1_gdat_hud_m11_command_pixel_hash(
    const DM2_V1_GdatHudM11Command *command);
uint32_t dm2_v1_gdat_hud_m11_command_plan_hash(
    const DM2_V1_GdatHudM11CommandPlan *plan);

void dm2_v1_gdat_hud_m11_command_plan_free(
    DM2_V1_GdatHudM11CommandPlan *plan);
int dm2_v1_gdat_hud_summary_m11_receipt(
    const DM2_V1_GdatHudM11CommandPlan *plan, int vb_144, int field,
    DM2_V1_GdatHudSummaryM11Receipt *out_receipt);
int dm2_v1_gdat_hud_picst_transform_receipt(
    const DM2_V1_GdatHudSummaryM11Receipt *summary, int source_value,
    DM2_V1_GdatHudPicstTransformReceipt *out_receipt);

/* Draws only the c_gui_draw.cpp:926-942 SUMMARY_IMAGE branch after its
 * authenticated QUERY_PICST_IT receipt. The caller owns the indexed target;
 * partial/unknown clipping and every other transform branch are rejected. */
int dm2_v1_gdat_hud_picst_draw_indexed(
    const DM2_V1_GdatHudM11CommandPlan *plan,
    const DM2_V1_GdatHudSummaryM11Receipt *summary,
    const DM2_V1_GdatHudPicstTransformReceipt *transform,
    int vb_144,
    int field,
    uint8_t *target_pixels,
    int target_width,
    int target_height,
    uint8_t out_palette16[16],
    DM2_V1_GdatHudPicstDrawReceipt *out_receipt);

#endif
