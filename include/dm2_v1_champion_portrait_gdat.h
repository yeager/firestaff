#ifndef FIRESTAFF_DM2_V1_CHAMPION_PORTRAIT_GDAT_H
#define FIRESTAFF_DM2_V1_CHAMPION_PORTRAIT_GDAT_H

#include "dm2_v1_asset_loader.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SKProject reference: SKWIN/SkWinCore.cpp::DRAW_CHAMPION_PICTURE
 * (0x2E62:061D) calls DRAW_ICON_PICT_ENTRY(CHAMPIONS, HeroType(), 0,
 * ..., player + 173, -1).  The HeroType byte is save-owned; a generated
 * Firestaff party has no source evidence for this lookup.
 */
#define DM2_V1_CHAMPION_PORTRAIT_GDAT_FIELD 0x00u
#define DM2_V1_CHAMPION_PORTRAIT_RECT_BASE 173u
#define DM2_V1_CHAMPION_PORTRAIT_PLAYER_COUNT 4u

typedef struct {
    uint8_t player_index;
    uint8_t hero_type;
    int hero_type_source_bound;
} DM2_V1_ChampionPortraitInput;

typedef struct {
    uint8_t category;
    uint8_t hero_type;
    uint8_t field;
    uint16_t rectno;
} DM2_V1_ChampionPortraitGdatRoute;

typedef struct {
    int valid;
    DM2_V1_ChampionPortraitGdatRoute route;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat decoded_format;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint32_t decoded_pixel_count;
    uint32_t decoded_pixels_hash;
    uint32_t material_hash;
} DM2_V1_ChampionPortraitGdatReceipt;

/* Build only SKProject's direct DRAW_CHAMPION_PICTURE GDAT address. */
int dm2_v1_champion_portrait_gdat_route(
    const DM2_V1_ChampionPortraitInput *input,
    DM2_V1_ChampionPortraitGdatRoute *out_route);

/*
 * Admit the decoded portrait and its IMG3-local palette as one source-owned
 * unit.  Missing, malformed, or non-save-owned material produces no receipt;
 * this routine never chooses another champion image or a synthetic portrait.
 */
int dm2_v1_champion_portrait_gdat_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_ChampionPortraitInput *input,
    DM2_V1_ChampionPortraitGdatReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CHAMPION_PORTRAIT_GDAT_H */
