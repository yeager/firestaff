#include "m11_game_view.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verify_pc34_d2_pixels(const unsigned char *framebuffer,
                                 const M11_AssetSlot *source,
                                 const M11_Dm1FloorItemHostPresentationReceipt *r)
{
    int expectedW;
    int expectedH;
    int x;
    int y;
    if (!framebuffer || !source || !r || r->sourceScaleIndex != 1 ||
        r->assetWidth != (int)source->width ||
        r->assetHeight != (int)source->height) return 0;
    expectedW = (int)source->width * 20 / 32;
    if ((source->width & 1u) == 0u) {
        expectedW = ((((int)(source->width / 2u) * 20 + 10) >> 5) << 1);
    }
    expectedH = ((int)source->height * 20 + 10) >> 5;
    if (expectedH > 20) {
        expectedH = 20;
        expectedW = (expectedH * (int)source->width) / (int)source->height;
    }
    if (expectedW > 32) {
        expectedW = 32;
        expectedH = (expectedW * (int)source->height) / (int)source->width;
    }
    if (r->destinationW != expectedW || r->destinationH != expectedH) return 0;
    for (y = 0; y < r->destinationH; ++y) {
        uint64_t stepY = ((uint64_t)source->height << 16) /
                         (unsigned)r->destinationH;
        unsigned sy = (unsigned)((stepY / 2u + 32767u +
                                  (uint64_t)y * stepY) >> 16);
        for (x = 0; x < r->destinationW; ++x) {
            uint64_t stepX = ((uint64_t)source->width << 16) /
                             (unsigned)r->destinationW;
            int sampleX = r->sourceMirrored ? r->destinationW - 1 - x : x;
            unsigned sx = (unsigned)((stepX / 2u + 32767u +
                                      (uint64_t)sampleX * stepX) >> 16);
            unsigned char pixel = source->pixels[sy * source->width + sx];
            unsigned char expected = DM1_FloorOrnPalette_D2[pixel & 0x0fu];
            unsigned char actual = framebuffer[(r->destinationY + y) * 320 +
                                                r->destinationX + x];
            if (expected == r->transparentColor) expected = 0xfeu;
            if (actual != expected) {
                fprintf(stderr, "pixel mismatch %d,%d src=%u expected=%u actual=%u plan=%d,%d %dx%d mirror=%d\n",
                        x, y, pixel, expected, actual, r->destinationX,
                        r->destinationY, r->destinationW, r->destinationH,
                        r->sourceMirrored);
                return 0;
            }
        }
    }
    return 1;
}

static int find_real_pc34_item(M11_GameViewState *state,
                               int *out_type,
                               int *out_subtype)
{
    static const struct { int type; int lastSubtype; } itemKinds[] = {
        {THING_TYPE_WEAPON, 45}, {THING_TYPE_ARMOUR, 57},
        {THING_TYPE_SCROLL, 0}, {THING_TYPE_POTION, 20},
        {THING_TYPE_CONTAINER, 0}, {THING_TYPE_JUNK, 52}
    };
    size_t kind;
    for (kind = 0u; kind < sizeof(itemKinds) / sizeof(itemKinds[0]); ++kind) {
        int subtype;
        for (subtype = 0; subtype <= itemKinds[kind].lastSubtype; ++subtype) {
            unsigned int graphic = dm1_item_sprite_index(itemKinds[kind].type,
                                                          subtype);
            const M11_AssetSlot *slot;
            DM1_ItemSpriteBlitPlan plan;
            size_t pixel;
            int visible = 0;
            if (graphic == 0u || graphic >= 584u) continue;
            slot = M11_AssetLoader_Load(&state->assetLoader, graphic);
            if (!slot || !slot->pixels || slot->width == 0u ||
                slot->height == 0u ||
                !dm1_item_sprite_blit_plan(&plan, itemKinds[kind].type,
                    subtype, 0, 0, 1, 0, 0, 33,
                    32, 65, 32, 20,
                    (int)slot->width, (int)slot->height)) continue;
            for (pixel = 0u; pixel < (size_t)slot->width * slot->height;
                 ++pixel) {
                if (DM1_FloorOrnPalette_D2[slot->pixels[pixel] & 0x0fu] != 10u) {
                    visible = 1;
                    break;
                }
            }
            if (visible) {
                *out_type = itemKinds[kind].type;
                *out_subtype = subtype;
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    M11_GameViewState state;
    M11_Dm1FloorItemHostPresentationReceipt receipt;
    M11_Dm1FloorItemHostPresentationReceipt alcoveReceipt;
    unsigned char framebuffer[320 * 200];
    const M11_AssetSlot *source;
    const char *data_dir = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    int thingType = -1;
    int subtype = -1;

    if (!data_dir || !data_dir[0]) {
        puts("skip: FIRESTAFF_DM1_PC34_ARCHIVE is not set");
        return 77;
    }
    M11_GameView_Init(&state);
    memset(framebuffer, 0, sizeof(framebuffer));
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        fprintf(stderr, "could not start DM1 from PC34 archive: %s\n", data_dir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (receipt.valid || receipt.graphicsId != 0 || receipt.destinationW != 0 ||
        receipt.destinationH != 0 || receipt.assetWidth != 0 ||
        receipt.assetHeight != 0) {
        return 1;
    }

    /* Draw the production F0115 route from an authenticated PC34 archive,
     * then inspect the same decoded GRAPHICS.DAT bitmap selected by receipt. */
    if (!find_real_pc34_item(&state, &thingType, &subtype)) {
        fprintf(stderr, "PC34 source archive has no drawable D2 floor-item sprite\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0xfe, sizeof(framebuffer));
    if (!M11_GameView_ProbeDrawDm1FloorItemHostReceipt(
            &state, framebuffer, 320, 200, thingType, subtype)) {
        fprintf(stderr, "real PC34 floor-item probe could not draw type=%d subtype=%d\n",
                thingType, subtype);
        return 1;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    source = M11_AssetLoader_Load(&state.assetLoader,
                                  (unsigned int)receipt.graphicsId);
    if (!receipt.valid || !receipt.floorItemLane ||
        !receipt.usesF0791Blit || receipt.transparentColor != 10 ||
        !M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1) ||
        !verify_pc34_d2_pixels(framebuffer, source, &receipt)) {
        fprintf(stderr, "receipt scale=%d size=%dx%d asset=%dx%d\n",
                receipt.sourceScaleIndex, receipt.destinationW,
                receipt.destinationH, receipt.assetWidth, receipt.assetHeight);
        fprintf(stderr, "PC34 D2 material output differs from F0129 sampling/palette\n");
        return 1;
    }

    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (!M11_GameView_ProbeDrawDm1AlcoveItemForFloorItemReceipt(
            &state, framebuffer, 320, 200)) {
        return 1;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (receipt.valid || receipt.floorItemLane ||
        M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1)) {
        return 1;
    }
    M11_GameView_GetDm1AlcoveItemHostPresentationReceipt(&alcoveReceipt);
    if (!alcoveReceipt.valid || alcoveReceipt.floorItemLane ||
        !alcoveReceipt.usesF0791Blit || alcoveReceipt.transparentColor != 10 ||
        alcoveReceipt.sourceZone < 2548 || alcoveReceipt.destinationW <= 0 ||
        alcoveReceipt.destinationH <= 0) {
        return 1;
    }

    M11_GameView_Shutdown(&state);
    puts("ok: M11 keeps F0115 floor and alcove host receipts separate");
    return 0;
}
