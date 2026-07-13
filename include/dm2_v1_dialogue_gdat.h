#ifndef DM2_V1_DIALOGUE_GDAT_H
#define DM2_V1_DIALOGUE_GDAT_H

#include <stdint.h>

#include "dm2_v1_asset_loader.h"

/* skproject/SKULLWIN/c_gui_vp.cpp dialogue branch uses GRAPHICSSET images
 * -4/-3/-2 for the dialogue shell and field 3 for its glyph source.  This
 * receipt binds only those original IMG3 records and their local palettes;
 * it deliberately does not infer dialogue layout, text, or a draw call. */
#define DM2_V1_DIALOGUE_SHELL_FIELD_MIN 0xfcu
#define DM2_V1_DIALOGUE_SHELL_FIELD_MAX 0xfeu
#define DM2_V1_DIALOGUE_GLYPH_FIELD     0x03u

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint8_t shell_field;
    DM2_V1_GdatImageMetadata shell_metadata;
    DM2_V1_GdatImageMetadata glyph_metadata;
    uint8_t shell_palette[16];
    uint8_t glyph_palette[16];
    uint32_t shell_palette_hash;
    uint32_t glyph_palette_hash;
    uint32_t receipt_hash;
} DM2_V1_DialogueGdatReceipt;

/* Returns an exact material receipt only when both source images are IMG3
 * 4bpp images with their own QUERY_GDAT_IMAGE_LOCALPAL tail. */
int dm2_v1_dialogue_gdat_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t graphicsset,
                                 uint8_t shell_field,
                                 DM2_V1_DialogueGdatReceipt *out);

#endif
