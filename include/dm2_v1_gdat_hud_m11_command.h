#ifndef FIRESTAFF_DM2_V1_GDAT_HUD_M11_COMMAND_H
#define FIRESTAFF_DM2_V1_GDAT_HUD_M11_COMMAND_H

/* Fail-closed original GDAT HUD command bridge for the M11 host. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_viewport_renderer.h"

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

typedef struct DM2_V1_GdatHudM11Command {
    int kind;
    int gdat_category;
    int gdat_index;
    int gdat_field;
    int viewport_gdat_index;
    DM2_V1_ViewportRect destination;
    uint8_t *pixels;
    int width;
    int height;
    DM2_ImageFormat format;
    uint8_t palette16[16];
    uint32_t raw_hash;
    uint32_t raw_byte_count;
    uint32_t palette_hash;
} DM2_V1_GdatHudM11Command;

typedef struct DM2_V1_GdatHudM11CommandPlan {
    int valid;
    int command_count;
    uint32_t command_hash;
    DM2_V1_GdatHudM11Command commands[DM2_V1_GDAT_HUD_M11_COMMAND_MAX];
} DM2_V1_GdatHudM11CommandPlan;

/* Builds the complete indoor HUD image family from a verified original
 * GRAPHICS.DAT. Every command carries decoded source pixels, its local
 * palette, exact GDAT address, and M11 destination. A missing, malformed, or
 * non-local-palette material rejects the entire plan. */
int dm2_v1_gdat_hud_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatHudM11CommandPlan *out_plan);

/* Extends the same verified HUD family with the occupied party portraits.
 * A portrait without its exact CHAMPIONS/index/0 material rejects all 13
 * commands rather than replacing it with a coloured slot. */
int dm2_v1_gdat_hud_m11_command_plan_build_for_party(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HudPartyState *party,
    DM2_V1_GdatHudM11CommandPlan *out_plan);

void dm2_v1_gdat_hud_m11_command_plan_free(
    DM2_V1_GdatHudM11CommandPlan *plan);

#endif
