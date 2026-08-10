#ifndef FIRESTAFF_DM1_PROBE_PORTRAIT_SEED_H
#define FIRESTAFF_DM1_PROBE_PORTRAIT_SEED_H

/*
 * Shared "seed real GRAPHICS.DAT champion portraits into the seeded party"
 * helper for DM1 M11 runtime probes.
 *
 * Firestaff's DM1 V1 top-row composition pipeline
 * (m11_draw_dm1_v1_top_row_receipt in src/engine/m11_game_view.c) is source-
 * locked: the champion-status boxes are drawn only when
 * dm1_v1_champion_top_row_atomic_frame_pc34 sees "originalMaterialsPublished"
 * for every non-skipped lane. That in turn requires
 * dm1_v1_champion_portrait_status_redraw_policy_pc34 to assign the OWNED
 * policy to each lane, which requires live_material_is_original to be true,
 * which requires the champion's portraitBitmapValid flag AND a non-null
 * portraitBitmap[] whose bytes came from the DM1_GFX_CHAMPION_PORTRAITS
 * atlas. When any of those are missing the pipeline correctly declines to
 * publish original material and m11_clear_dm1_v1_top_row_receipt_zones
 * BLACK-fills every champion status zone -- and the probe sees an empty
 * frame with 0/N C033 perimeter pixels.
 *
 * A probe that seeds a fake party without loading real portrait bytes is
 * therefore testing the receipt-pipeline's clear path, not the compose
 * path. Call firestaff_dm1_probe_seed_original_portraits(game, count) after
 * seeding your party to make the compose path publish real material.
 *
 * The helper only reads GRAPHICS.DAT via the already-open M11 asset loader
 * (loading the atlas that was verified by the containing test data manifest)
 * and packs the four 32x29 lanes into the fake champions' portraitBitmap[]
 * buffers as pairs of 4-bit indices. It never invents pixels.
 *
 * Returns 1 on success, 0 if the atlas could not be loaded or is smaller
 * than expected -- caller should print a SKIP diagnostic and exit 0 in that
 * case, just as its data-dir resolution does.
 */

#include <stdio.h>

#include "asset_loader_m11.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"

static int firestaff_dm1_probe_seed_original_portraits(
    M11_GameViewState *game, int championCount)
{
    const M11_AssetSlot *portraits;
    int championIndex;

    if (!game || championCount <= 0) return 0;
    portraits = M11_AssetLoader_Load(
        &game->assetLoader,
        (unsigned int)dm1_v1_graphic_champion_portraits_pc34());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < CHAMPION_PORTRAIT_BITMAP_WIDTH * 8 ||
        portraits->height < CHAMPION_PORTRAIT_BITMAP_HEIGHT) {
        return 0;
    }
    for (championIndex = 0; championIndex < championCount &&
                             championIndex < CHAMPION_MAX_PARTY;
         ++championIndex) {
        struct ChampionState_Compat *champion =
            &game->world.party.champions[championIndex];
        const int sourceX = championIndex * CHAMPION_PORTRAIT_BITMAP_WIDTH;
        int y;
        for (y = 0; y < CHAMPION_PORTRAIT_BITMAP_HEIGHT; ++y) {
            const unsigned char *source = portraits->pixels +
                y * (int)portraits->width + sourceX;
            unsigned char *destination = champion->portraitBitmap +
                y * (CHAMPION_PORTRAIT_BITMAP_WIDTH / 2);
            int x;
            for (x = 0; x < CHAMPION_PORTRAIT_BITMAP_WIDTH; x += 2) {
                destination[x / 2] = (unsigned char)(
                    ((source[x] & 0x0fu) << 4) | (source[x + 1] & 0x0fu));
            }
        }
        champion->portraitBitmapValid = 1;
    }
    return 1;
}

#endif /* FIRESTAFF_DM1_PROBE_PORTRAIT_SEED_H */
