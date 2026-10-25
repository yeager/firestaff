#include "asset_find_by_hash.h"
#include "asset_loader_m11.h"
#include "dm1_v1_fmtowns_pic_library.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRE(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); goto cleanup; } \
} while (0)

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    uint8_t *cue = NULL, *bin = NULL;
    uint8_t *bytes = NULL;
    size_t cueSize = 0u, binSize = 0u, size = 0u;
    char imageMember[256];
    FmtownsDiscProbeResult disc;
    M11_AssetLoader loader;
    unsigned short width = 0u, height = 0u;
    const M11_AssetSlot *title;
    dm1_v1_fmtowns_pic_library_view_t pictureLibrary;
    const uint8_t *titleSpan = NULL;
    uint16_t titleSpanSize = 0u;
    uint8_t *packedTitle = NULL;
    size_t packedTitleWritten = 0u, packedTitleConsumed = 0u;
    size_t pixels = 0u;
    int result = 1;

    memset(&loader, 0, sizeof(loader));
    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_FMTOWNS_ARCHIVE not set");
        return 77;
    }
    if (firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cueSize) != 0 ||
        !cue || !fmtowns_cue_parse_image_member((const char *)cue, cueSize,
                                                 imageMember, sizeof(imageMember)) ||
        firestaff_zip_extract_by_suffix(archive, imageMember, &bin,
                                        &binSize) != 0 || !bin ||
        fmtowns_disc_probe(bin, binSize, FMTOWNS_SECTOR_2048, &disc) != 0 ||
        !disc.valid) {
        fprintf(stderr, "FAIL: cannot read authenticated FM Towns GRAPHICS.DAT\n");
        goto cleanup;
    }
    {
        const FmtownsIsoEntry *entry = fmtowns_disc_find(&disc,
                                                         "DATA/GRAPHICS.DAT");
        if (!entry || fmtowns_disc_extract_alloc(bin, binSize,
                FMTOWNS_SECTOR_2048, entry, &bytes, &size) != 0 || !bytes) {
            fprintf(stderr, "FAIL: cannot read authenticated FM Towns GRAPHICS.DAT\n");
            goto cleanup;
        }
    }
    REQUIRE(size == 396970u &&
        asset_memory_matches_md5(bytes, size,
            "c10c512f63461ebe79b5ac365115b61b"),
        "fixture is the English FM Towns original GRAPHICS.DAT");
    REQUIRE(M11_AssetLoader_InitDm1FmtownsFromBuffer(&loader, bytes,
            (long)size), "native FM Towns loader admits the picture library");
    REQUIRE(loader.dm1Fmtowns && loader.legacyDm1 &&
            !loader.legacyBigEndian && loader.graphicCount == 575u,
        "FM Towns graphics use its native decoder and classic DM1 semantics");
    REQUIRE(M11_AssetLoader_QuerySize(&loader, 1u, &width, &height) &&
            width == 320u && height == 200u,
        "EDM.EXP title raster preserves source dimensions");
    title = M11_AssetLoader_Load(&loader, 1u);
    REQUIRE(title && title->loaded && title->graphicIndex == 1u &&
            title->width == 320u && title->height == 200u && title->pixels,
        "EDM.EXP compact-row RLE loop loads the 320x200 title raster");
    pixels = (size_t)title->width * title->height;
    for (size_t i = 0u; i < pixels; ++i) {
        REQUIRE(title->pixels[i] < 16u,
            "expanded title uses four-bit source palette indices");
    }
    REQUIRE(dm1_v1_fmtowns_pic_library_open_pc34(
                bytes, size, &pictureLibrary) == DM1_V1_FMTOWNS_PIC_LIB_OK &&
            dm1_v1_fmtowns_pic_library_asset_bytes_pc34(
                &pictureLibrary, 1u, &titleSpan, &titleSpanSize) ==
                DM1_V1_FMTOWNS_PIC_LIB_OK,
        "title source span is addressable in the authenticated library");
    packedTitle = (uint8_t *)malloc(pixels / 2u);
    REQUIRE(packedTitle != NULL &&
            dm1_v1_fmtowns_pic_library_decode_asset_pc34(
                titleSpan, titleSpanSize, packedTitle, pixels / 2u,
                &packedTitleWritten, &packedTitleConsumed, NULL) ==
                DM1_V1_FMTOWNS_PIC_LIB_OK &&
            packedTitleWritten == pixels / 2u &&
            packedTitleConsumed == titleSpanSize,
        "native decoder expands the source title before nibble unpacking");
    for (size_t x = 0u; x < 16u; ++x) {
        const uint8_t packed = packedTitle[x / 2u];
        const uint8_t sourceIndex = (uint8_t)((x & 1u)
            ? (packed & 0x0fu) : (packed >> 4));
        REQUIRE(title->pixels[x] == sourceIndex,
            "loader preserves Towns high-nibble-left pixel order");
    }
    {
        static const unsigned sourceGraphics[] = {
            78u, 79u, 86u, 259u, 260u, 345u, 346u
        };
        for (size_t i = 0u;
             i < sizeof(sourceGraphics) / sizeof(sourceGraphics[0]); ++i) {
            const M11_AssetSlot *sourceGraphic = M11_AssetLoader_Load(
                &loader, sourceGraphics[i]);
            REQUIRE(sourceGraphic && sourceGraphic->loaded &&
                    sourceGraphic->pixels && sourceGraphic->width > 0u &&
                    sourceGraphic->height > 0u,
                "native floor, wall, alcove, and mirror assets load");
            for (size_t p = 0u;
                 p < (size_t)sourceGraphic->width * sourceGraphic->height;
                 ++p) {
                REQUIRE(sourceGraphic->pixels[p] < 16u,
                    "native Towns scenery preserves four-bit palette indices");
            }
        }
    }
    REQUIRE(M11_AssetLoader_Load(&loader, 1u) == title,
        "title raster remains stable in the decoded cache");
    printf("PASS: DM1 FM Towns GRAPHICS.DAT title decoded natively (%ux%u)\n",
           title->width, title->height);
    result = 0;

cleanup:
    M11_AssetLoader_Shutdown(&loader);
    free(packedTitle);
    free(cue);
    free(bin);
    free(bytes);
    return result;
}
