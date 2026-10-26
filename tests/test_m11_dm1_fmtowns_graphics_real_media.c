#include "asset_find_by_hash.h"
#include "asset_loader_m11.h"
#include "dm1_v1_champion_top_row_assets_pc34_compat.h"
#include "dm1_v1_champion_panel_material_pc34_compat.h"
#include "dm1_v1_fmtowns_font_asset.h"
#include "dm1_v1_fmtowns_hud_layout.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "font_m11.h"
#include "dm1_v1_fmtowns_pic_library.h"
#include "dm1_v1_fmtowns_wall_ornament.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRE(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); goto cleanup; } \
} while (0)

static int check_top_row_source_surfaces(M11_AssetLoader *loader)
{
    static const unsigned required_indices[] = {
        8u, 17u, 26u, 28u, 32u, 15u, 16u, 33u, 34u, 35u
    };
    static const unsigned required_widths[] = {
        67u, 224u, 256u, 76u, 90u, 43u, 32u, 18u, 18u, 18u
    };
    static const unsigned required_heights[] = {
        29u, 136u, 87u, 14u, 15u, 7u, 29u, 18u, 18u, 18u
    };
    static const unsigned indices[] = {8u, 28u, 33u, 34u, 35u};
    static const unsigned widths[] = {67u, 76u, 18u, 18u, 18u};
    static const unsigned heights[] = {29u, 14u, 18u, 18u, 18u};
    int pc_surface_set_accepted = 1;
    for (size_t i = 0u; i < sizeof(indices) / sizeof(indices[0]); ++i) {
        unsigned short width = 0u, height = 0u;
        if (!M11_AssetLoader_QuerySize(loader, indices[i], &width, &height) ||
            width != widths[i] || height != heights[i]) return 0;
    }
    for (size_t i = 0u;
         i < sizeof(required_indices) / sizeof(required_indices[0]); ++i) {
        unsigned short width = 0u, height = 0u;
        int native_fmtowns_variant = 0;
        int size_available = M11_AssetLoader_QuerySize(loader,
            required_indices[i], &width, &height);
        native_fmtowns_variant = size_available &&
            required_indices[i] == 32u && width == 53u && height == 17u;
        if (!size_available ||
            (!native_fmtowns_variant &&
             (width != required_widths[i] || height != required_heights[i]))) {
            fprintf(stderr, "F20 top-row material %u is %ux%u, expected %ux%u\n",
                    required_indices[i], width, height,
                    required_widths[i], required_heights[i]);
            pc_surface_set_accepted = 0;
        }
    }
    if (!pc_surface_set_accepted) return 0;
    {
        Dm1V1ChampionTopRowAssetsReceiptPc34 receipt;
        M11_FontState sourceFont;
        Dm1V1ChampionPanelMaterialReceiptPc34 panelReceipt;
        static const unsigned char nativePalette[16u * 3u] = {0};
        dm1_v1_fmtowns_pic_library_view_t view;
        unsigned char rawFont[M11_FONT_BITMAP_BYTES];
        size_t rawFontLength = 0u;
        int admitted;
        M11_Font_Init(&sourceFont);
        admitted = loader->dm1FmtownsData && loader->dm1FmtownsDataSize > 0 &&
            dm1_v1_fmtowns_pic_library_open_pc34(loader->dm1FmtownsData,
                (size_t)loader->dm1FmtownsDataSize, &view) ==
                    DM1_V1_FMTOWNS_PIC_LIB_OK &&
            dm1_v1_fmtowns_pic_library_load_raw_asset_pc34(&view,
                dm1_v1_fmtowns_font_pic_library_index_pc34(), rawFont,
                sizeof(rawFont), &rawFontLength) ==
                    DM1_V1_FMTOWNS_PIC_LIB_OK &&
            M11_Font_LoadFromRawBitmap(&sourceFont,
                M11_FONT_GRAPHIC_INDEX_LEGACY, rawFont, rawFontLength) &&
            dm1_v1_champion_panel_material_from_m11_loader_fmtowns(
                loader, &sourceFont, nativePalette, 16, &panelReceipt) &&
            panelReceipt.valid && panelReceipt.suppressSyntheticFallback &&
            panelReceipt.m653GraphicIndex == M11_FONT_GRAPHIC_INDEX_LEGACY &&
            panelReceipt.materialFingerprint != 0u;
        if (!admitted) {
            fprintf(stderr,
                    "F20 panel receipt rejected (font=%d index=%d material=%d)\n",
                    M11_Font_IsLoaded(&sourceFont),
                    M11_Font_ResolvedGraphicIndex(&sourceFont),
                    panelReceipt.valid);
            return 0;
        }
        return dm1_v1_champion_top_row_assets_from_m11_loader_pc34(
                   loader, &receipt) && receipt.valid &&
               receipt.c008Accepted && receipt.c028Accepted &&
               receipt.c033Accepted && receipt.c034Accepted &&
               receipt.c035Accepted;
    }
}

static int check_f20_hud_layout_media(M11_AssetLoader *loader,
                                      DM1_V1_FmtownsHudEdition edition)
{
    static const unsigned zones[] = {151u, 152u, 153u, 154u};
    static const int expected_x[] = {0, 69, 138, 207};
    unsigned short width = 0u, height = 0u;
    DM1_V1_FmtownsHudRect rect;
    const M11_AssetSlot *iconStrip;
    int found_transparency = 0;
    if (!M11_AssetLoader_QuerySize(loader, 8u, &width, &height) ||
        width != 67u || height != 29u) return 0;
    for (size_t i = 0u; i < sizeof(zones) / sizeof(zones[0]); ++i) {
        if (!dm1_v1_fmtowns_hud_layout_rect(edition, zones[i],
                (int)width, (int)height, &rect) ||
            rect.x != expected_x[i] || rect.y != 0 ||
            rect.width != 67 || rect.height != 29 ||
            rect.source_x != 0 || rect.source_y != 0) return 0;
    }
    if (!M11_AssetLoader_QuerySize(loader, 15u, &width, &height) ||
        width != 43u || height != 7u ||
        !dm1_v1_fmtowns_hud_layout_rect(edition, 159u,
            (int)width, (int)height, &rect) || rect.x != 0 || rect.y != 0 ||
        rect.width != 43 || rect.height != 7 ||
        !dm1_v1_fmtowns_hud_layout_rect(edition, 113u, 19, 14, &rect) ||
        rect.x != 281 || rect.y != 0 || rect.width != 19 ||
        rect.height != 14 ||
        !dm1_v1_fmtowns_hud_layout_rect(edition, 175u, 32, 29, &rect) ||
        rect.x != 7 || rect.y != 0 || rect.width != 32 ||
        rect.height != 29) return 0;
    iconStrip = M11_AssetLoader_Load(loader, 28u);
    if (!iconStrip || !iconStrip->loaded || !iconStrip->pixels ||
        iconStrip->width != 76u || iconStrip->height != 14u) return 0;
    for (size_t i = 0u; i < (size_t)iconStrip->width * iconStrip->height; ++i) {
        if (iconStrip->pixels[i] == 12u) found_transparency = 1;
        if (iconStrip->pixels[i] >= 16u) return 0;
    }
    if (!found_transparency) return 0;
    {
        static const uint8_t expected_colors[4] = {7u, 11u, 8u, 14u};
        for (int slot = 0; slot < 4; ++slot) {
            uint8_t color = 0u;
            if (!dm1_v1_fmtowns_hud_champion_icon_color(
                    edition, slot, 1, &color) ||
                color != expected_colors[slot]) return 0;
        }
    }
    /* F20E/F20J use the same G3034/G3038 geometry. Only native bitmap
     * extents admitted from this edition's media are supplied to F0635. */
    return 1;
}

static int check_f20_hand_slot_pixels(M11_AssetLoader *loader,
                                      DM1_V1_FmtownsHudEdition edition,
                                      const char *edition_name)
{
    static const struct {
        unsigned int zone;
        unsigned int frame_graphic;
        int icon_index;
    } cases[] = {
        {211u, 33u, 212}, /* ready, empty */
        {211u, 34u, 213}, /* ready, wounded, empty */
        {212u, 35u, 214}, /* action, acting, empty */
        {212u, 33u, 168}  /* action, occupied by the Apple */
    };
    static const int expected_x[8] = {4, 24, 73, 93, 142, 162, 211, 231};
    uint8_t framebuffer[320u * 200u];
    for (size_t zone_ordinal = 0u; zone_ordinal < 8u; ++zone_ordinal) {
        DM1_V1_FmtownsHudRect rect;
        const unsigned int zone = 211u + (unsigned int)zone_ordinal;
        if (!dm1_v1_fmtowns_hud_layout_rect(edition, zone, 16, 16, &rect) ||
            rect.x != expected_x[zone_ordinal] || rect.y != 10 ||
            rect.width != 16 || rect.height != 16) {
            fprintf(stderr, "%s hand zone %u resolved to (%d,%d %dx%d)\n",
                    edition_name, zone, rect.x, rect.y,
                    rect.width, rect.height);
            return 0;
        }
    }
    for (size_t case_index = 0u;
         case_index < sizeof(cases) / sizeof(cases[0]); ++case_index) {
        const M11_AssetSlot *frame = M11_AssetLoader_Load(
            loader, cases[case_index].frame_graphic);
        DM1_V1_ObjectIconSourceZonePc34 icon_zone;
        const M11_AssetSlot *atlas;
        DM1_V1_FmtownsHudRect rect;
        if (!frame || !frame->loaded || !frame->pixels ||
            frame->width != 18u || frame->height != 18u ||
            !dm1_v1_fmtowns_hud_layout_rect(edition, cases[case_index].zone,
                                            16, 16, &rect) ||
            !dm1_v1_object_icon_source_zone_pc34(
                cases[case_index].icon_index, &icon_zone) ||
            icon_zone.w != 16 || icon_zone.h != 16) {
            fprintf(stderr, "%s hand material admission failed (zone=%u, frame=%u, icon=%d)\n",
                    edition_name, cases[case_index].zone,
                    cases[case_index].frame_graphic,
                    cases[case_index].icon_index);
            return 0;
        }
        atlas = M11_AssetLoader_Load(loader,
                                    (unsigned int)icon_zone.graphic_index);
        if (!atlas || !atlas->loaded || !atlas->pixels ||
            icon_zone.x < 0 || icon_zone.y < 0 ||
            icon_zone.x + icon_zone.w > (int)atlas->width ||
            icon_zone.y + icon_zone.h > (int)atlas->height) {
            fprintf(stderr, "%s hand icon %d is outside original atlas %d (%ux%u)\n",
                    edition_name, cases[case_index].icon_index,
                    icon_zone.graphic_index,
                    atlas ? atlas->width : 0u, atlas ? atlas->height : 0u);
            return 0;
        }
        /* C12 is the F20 hand-zone clear color as well as the frame's
         * transparent index; transparent border pixels must preserve it. */
        memset(framebuffer, 12, sizeof(framebuffer));
        M11_AssetLoader_BlitRegion(frame, 0, 0, 18, 18,
            framebuffer, 320, 200, rect.x - 1, rect.y - 1, 12);
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                framebuffer[(rect.y + y) * 320 + rect.x + x] =
                    atlas->pixels[(icon_zone.y + y) * (int)atlas->width +
                                  icon_zone.x + x];
            }
        }
        for (int y = 0; y < 18; ++y) {
            for (int x = 0; x < 18; ++x) {
                const int px = rect.x - 1 + x;
                const int py = rect.y - 1 + y;
                uint8_t expected = 12u;
                if (x >= 1 && x <= 16 && y >= 1 && y <= 16) {
                    expected = atlas->pixels[(icon_zone.y + y - 1) *
                                 (int)atlas->width + icon_zone.x + x - 1];
                } else if (frame->pixels[y * (int)frame->width + x] != 12u) {
                    expected = frame->pixels[y * (int)frame->width + x];
                }
                if (framebuffer[py * 320 + px] != expected) {
                    fprintf(stderr,
                        "%s hand pixel mismatch case=%zu at (%d,%d): got=%u expected=%u\n",
                        edition_name, case_index, x, y,
                        framebuffer[py * 320 + px], expected);
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int check_f20_inscription_font_media(M11_AssetLoader *loader,
                                             const char *edition)
{
    unsigned short nativeWidth = 0u, nativeHeight = 0u;
    unsigned short pc34Width = 0u, pc34Height = 0u;
    const M11_AssetSlot *font;
    if (!M11_AssetLoader_QuerySize(loader, 120u,
                                   &nativeWidth, &nativeHeight) ||
        nativeWidth != 288u || nativeHeight != 8u ||
        !M11_AssetLoader_QuerySize(loader, 258u,
                                   &pc34Width, &pc34Height) ||
        pc34Width != 62u || pc34Height != 21u) {
        fprintf(stderr,
            "%s M648 index audit: native 120=%ux%u, PC34 258=%ux%u\n",
            edition, nativeWidth, nativeHeight, pc34Width, pc34Height);
        return 0;
    }
    font = M11_AssetLoader_Load(loader, 120u);
    return font && font->loaded && font->pixels &&
           font->graphicIndex == 120u && font->width == 288u &&
           font->height == 8u;
}

static int audit_f20_mirror_geometry_media(M11_AssetLoader *loader,
                                           DM1_V1_FmtownsHudEdition edition,
                                           const char *edition_name)
{
    unsigned short portraitWidth = 0u, portraitHeight = 0u;
    unsigned short backingWidth = 0u, backingHeight = 0u;
    DM1_V1_FmtownsHudRect portraitRect, backingRect;
    int overlaps;
    if (!loader ||
        !M11_AssetLoader_QuerySize(loader, 26u,
                                   &portraitWidth, &portraitHeight) ||
        !M11_AssetLoader_QuerySize(loader, 208u,
                                   &backingWidth, &backingHeight) ||
        portraitWidth < 256u || portraitHeight < 87u ||
        !dm1_v1_fmtowns_hud_layout_rect(edition, 733u, 32, 29,
                                        &portraitRect) ||
        !dm1_v1_fmtowns_hud_layout_rect(edition, 1081u,
                (int)backingWidth, (int)backingHeight, &backingRect)) {
        return 0;
    }
    overlaps = portraitRect.x < backingRect.x + backingRect.width &&
        portraitRect.x + portraitRect.width > backingRect.x &&
        portraitRect.y < backingRect.y + backingRect.height &&
        portraitRect.y + portraitRect.height > backingRect.y;
    fprintf(stderr,
        "F20 mirror audit %s: source C208 %ux%u -> F0635 zone1081 (%d,%d %dx%d); "
        "source C026 %ux%u -> F0635/M635 C733 (%d,%d %dx%d); "
        "rectangles %s; Firestaff PC34 destination=(96,35)\n",
        edition_name, backingWidth, backingHeight,
        backingRect.x, backingRect.y, backingRect.width, backingRect.height,
        portraitWidth, portraitHeight,
        portraitRect.x, portraitRect.y,
        portraitRect.width, portraitRect.height,
        overlaps ? "overlap" : "do not overlap");
    return portraitRect.x == 0 && portraitRect.y == 19 &&
           portraitRect.width == 32 && portraitRect.height == 29;
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    uint8_t *cue = NULL, *bin = NULL;
    uint8_t *bytes = NULL, *japaneseBytes = NULL;
    size_t cueSize = 0u, binSize = 0u, size = 0u, japaneseSize = 0u;
    char imageMember[256];
    FmtownsDiscProbeResult disc;
    M11_AssetLoader loader, japaneseLoader;
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
    memset(&japaneseLoader, 0, sizeof(japaneseLoader));
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
    REQUIRE(check_top_row_source_surfaces(&loader),
        "original F20E status box, champion strip, and slot states pass the shared source-surface receipt");
    REQUIRE(check_f20_hud_layout_media(&loader,
                DM1_V1_FMTOWNS_HUD_EDITION_F20E),
        "original F20E bitmap dimensions resolve through native COORD.C HUD zones");
    REQUIRE(check_f20_hand_slot_pixels(&loader,
                DM1_V1_FMTOWNS_HUD_EDITION_F20E, "F20E"),
        "original F20E C211-C218 ready, wounded, acting, and occupied hands match F0291/F0038 pixels");
    REQUIRE(check_f20_inscription_font_media(&loader, "F20E"),
        "ReDMCSB M648 ordinal 120 resolves the authentic F20E inscription font, unlike PC34 ordinal 258");
    REQUIRE(audit_f20_mirror_geometry_media(&loader,
                DM1_V1_FMTOWNS_HUD_EDITION_F20E, "F20E"),
        "original F20E C208/C026 media resolves through the F0635 mirror zones");
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
    for (int global = 0; global < 60; ++global) {
        for (int view = 0; view < 13; ++view) {
            DM1_V1_FmtownsWallOrnament ornament;
            unsigned short ornamentWidth = 0u, ornamentHeight = 0u;
            REQUIRE(dm1_v1_fmtowns_wall_ornament_resolve(
                        global, view, 0, &ornament),
                "F20E resolves each original wall ornament and view");
            REQUIRE(ornament.coordinate_set >= 0 &&
                    ornament.coordinate_set <= 7 &&
                    ornament.zone_index ==
                        1004 + ornament.coordinate_set * 13 + view &&
                    ornament.graphic_index ==
                        121 + global * 2 +
                            ((view == 2 || view == 3 || view == 4 ||
                              view == 7 || view == 8 || view == 9 ||
                              view == 11 || view == 12) ? 1 : 0) &&
                    ornament.layout[1] == 4,
                "F20E uses its 13-zone layout, paired native bitmaps, and anchors");
            REQUIRE(M11_AssetLoader_QuerySize(&loader,
                        (unsigned int)ornament.graphic_index,
                        &ornamentWidth, &ornamentHeight) &&
                    ornamentWidth > 0u && ornamentHeight > 0u,
                "selected F20E ornament bitmap exists in original media");
        }
    }
    {
        const FmtownsIsoEntry *entry = fmtowns_disc_find(
            &disc, "JDATA/GRAPHICS.DAT");
        REQUIRE(entry && fmtowns_disc_extract_alloc(bin, binSize,
                    FMTOWNS_SECTOR_2048, entry, &japaneseBytes,
                    &japaneseSize) == 0 && japaneseBytes &&
                M11_AssetLoader_InitDm1FmtownsFromBuffer(&japaneseLoader,
                    japaneseBytes, (long)japaneseSize),
            "original Japanese F20 graphics initialize their native decoder");
    }
    REQUIRE(check_top_row_source_surfaces(&japaneseLoader),
        "original F20J status box, champion strip, and slot states pass the shared source-surface receipt");
    REQUIRE(check_f20_hud_layout_media(&japaneseLoader,
                DM1_V1_FMTOWNS_HUD_EDITION_F20J),
        "original F20J bitmap dimensions resolve through native COORD.C HUD zones");
    REQUIRE(check_f20_hand_slot_pixels(&japaneseLoader,
                DM1_V1_FMTOWNS_HUD_EDITION_F20J, "F20J"),
        "original F20J C211-C218 ready, wounded, acting, and occupied hands match F0291/F0038 pixels");
    REQUIRE(check_f20_inscription_font_media(&japaneseLoader, "F20J"),
        "ReDMCSB M648 ordinal 120 resolves the authentic F20J inscription font, unlike PC34 ordinal 258");
    REQUIRE(audit_f20_mirror_geometry_media(&japaneseLoader,
                DM1_V1_FMTOWNS_HUD_EDITION_F20J, "F20J"),
        "original F20J C208/C026 media resolves through the F0635 mirror zones");
    for (int global = 0; global < 60; ++global) {
        for (int view = 0; view < 13; ++view) {
            DM1_V1_FmtownsWallOrnament ornament;
            unsigned short ornamentWidth = 0u, ornamentHeight = 0u;
            REQUIRE(dm1_v1_fmtowns_wall_ornament_resolve(
                        global, view, 1, &ornament),
                "F20J resolves each original wall ornament and view");
            REQUIRE(M11_AssetLoader_QuerySize(&japaneseLoader,
                        (unsigned int)ornament.graphic_index,
                        &ornamentWidth, &ornamentHeight) &&
                    ornamentWidth > 0u && ornamentHeight > 0u,
                "selected F20J ornament bitmap exists in original media");
        }
    }
    {
        DM1_V1_FmtownsWallOrnament english, japanese;
        REQUIRE(dm1_v1_fmtowns_wall_ornament_resolve(0, 0, 0, &english) &&
                dm1_v1_fmtowns_wall_ornament_resolve(0, 0, 1, &japanese) &&
                english.zone_index == 1095 && japanese.zone_index == 1095 &&
                english.layout[3] == 41 && japanese.layout[3] == 38,
            "F20E and F20J preserve their distinct unreadable-inscription anchors");
    }
    REQUIRE(M11_AssetLoader_Load(&loader, 1u) == title,
        "title raster remains stable in the decoded cache");
    printf("PASS: DM1 FM Towns GRAPHICS.DAT title decoded natively (%ux%u)\n",
           title->width, title->height);
    result = 0;

cleanup:
    M11_AssetLoader_Shutdown(&japaneseLoader);
    M11_AssetLoader_Shutdown(&loader);
    free(packedTitle);
    free(cue);
    free(bin);
    free(bytes);
    free(japaneseBytes);
    return result;
}
