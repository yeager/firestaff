/* Nexus startup media must be decoded before it is presented by M11. */
#include "nexus_v1_engine.h"
#include "nexus_v1_rasterizer.h"
#include "nexus_v1_title.h"
#include "nexus_v1_ui_surfaces.h"
#include "nexus_v1_face_bin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static unsigned char expand_5bit(unsigned value)
{
    return (unsigned char)((value << 3) | (value >> 2));
}

static unsigned dgt2_rgba(const unsigned char *clut, int index)
{
    unsigned value = ((unsigned)clut[index * 2] << 8) |
                     (unsigned)clut[index * 2 + 1];
    return 0xff000000U |
           ((unsigned)expand_5bit((value >> 10) & 0x1fU) << 16) |
           ((unsigned)expand_5bit((value >> 5) & 0x1fU) << 8) |
           (unsigned)expand_5bit(value & 0x1fU);
}

static void expect_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static unsigned char *read_local_nexus_file(const char *name, size_t *out_size)
{
    const char *home = getenv("HOME");
    char path[2048];
    FILE *file;
    long size;
    unsigned char *data;

    if (!home || !name || !out_size ||
        snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/%s", home, name) >=
            (int)sizeof(path)) {
        return NULL;
    }
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data || fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return data;
}

int main(void)
{
    Nexus_UI_Manager ui;
    Nexus_V1_Engine engine;
    unsigned char opaque_title[64001];
    unsigned char opaque_warning[64001];
    unsigned char *local_warning;
    unsigned char *local_gameover;
    unsigned char *local_title;
    size_t local_warning_size = 0U;
    Nexus_UI_Dgt2PpView warning_view;
    unsigned char no_draw_fb[16];
    int no_draw_index;

    memset(&ui, 0, sizeof(ui));
    memset(&engine, 0, sizeof(engine));
    memset(opaque_title, 0x5a, sizeof(opaque_title));
    memset(opaque_warning, 0x52, sizeof(opaque_warning));
    memcpy(opaque_warning, "RES*", 4);

    nexus_ui_manager_init(&ui);
    memset(no_draw_fb, 0xa5, sizeof(no_draw_fb));
    nexus_ui_render_title(&ui, no_draw_fb, 4, 4);
    nexus_ui_render_warning(&ui, no_draw_fb, 4, 4);
    nexus_ui_render_gameover(&ui, no_draw_fb, 4, 4);
    nexus_ui_render_stabg(&ui, no_draw_fb, 4, 4, 0, 0);
    nexus_ui_render_portrait(&ui, 0, no_draw_fb, 4, 4, 0, 0, 0);
    for (no_draw_index = 0; no_draw_index < (int)sizeof(no_draw_fb);
         ++no_draw_index) {
        expect_true(no_draw_fb[no_draw_index] == 0xa5,
                    "startup UI convenience wrappers remain Saturn-capture no-draw");
    }
    expect_true(nexus_ui_load_title(&ui, NULL, 0, NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_TITLE].data == NULL,
                "missing TITLE.CG cannot become a generated title surface");
    expect_true(nexus_ui_load_stabg(&ui, opaque_title,
                                    (int)sizeof(opaque_title), NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_STABG].data == NULL,
                "unframed STABG.BIN bytes cannot become a guessed UI surface");
    {
        unsigned char face_header[48 * 48] = { 'F', 'A', 'C', 'E' };
        expect_true(nexus_ui_load_faces(&ui, face_header, 0,
                                        (int)sizeof(face_header), 0, 48, 48,
                                        NULL) < 0 &&
                        ui.surfaces[NEXUS_SURFACE_FACE0].data == NULL,
                    "FACE.BIN container bytes cannot enter the raw portrait path");
    }
    expect_true(nexus_ui_load_title(&ui, opaque_title,
                                    (int)sizeof(opaque_title), NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_TITLE].data == NULL,
                "opaque TITLE.CG bytes cannot become a startup raster");
    expect_true(nexus_ui_surface_load(&ui, NEXUS_SURFACE_TITLE,
                                      opaque_title, (int)sizeof(opaque_title),
                                      65535, 65535, 0, 0,
                                      "overflow-fixture") < 0 &&
                    ui.surfaces[NEXUS_SURFACE_TITLE].data == NULL,
                "surface dimensions cannot overflow the allocation size");
    {
        unsigned char malformed_face[347];
        Nexus_UI_FaceLayout face_layout;
        memset(malformed_face, 0, sizeof(malformed_face));
        memcpy(malformed_face, "FACE", 4);
        malformed_face[4] = 0;
        malformed_face[5] = 0;
        malformed_face[6] = 1;
        malformed_face[7] = 0x5b;
        malformed_face[8] = 0;
        malformed_face[9] = 20;
        memcpy(malformed_face + 184, "PRS3", 4);
        malformed_face[191] = 1;
        malformed_face[194] = 0x0c;
        malformed_face[195] = 0x40;
        malformed_face[199] = 1;
        expect_true(nexus_ui_face_layout_detect(malformed_face,
                                                (int)sizeof(malformed_face),
                                                &face_layout) == 0 &&
                        face_layout.valid == 0 &&
                        face_layout.portrait_w == 0 &&
                        face_layout.portrait_h == 0,
                    "rejected FACE bytes cannot retain fallback portrait geometry");
    }
    expect_true(nexus_ui_load_warning(&ui, opaque_warning,
                                      (int)sizeof(opaque_warning), NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_WARNING].data == NULL,
                "RES* WARNING.BIN bytes cannot become a startup raster");
    expect_true(nexus_ui_load_gameover(&ui, opaque_warning,
                                       (int)sizeof(opaque_warning), NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_GAMEOVER].data == NULL,
                "malformed GAMEOVER.BIN cannot become a guessed raster");
    nexus_ui_manager_free(&ui);

    {
        unsigned char opaque_stabg[53744];
        Nexus_UI_StabgStmpFraming stabg_framing;
        unsigned char *local_stabg;
        memset(opaque_stabg, 0x53, sizeof(opaque_stabg));
        expect_true(nexus_ui_stabg_stmp_framing_receipt(
                        opaque_stabg, (int)sizeof(opaque_stabg),
                        &stabg_framing) < 0 &&
                    stabg_framing.valid == 0,
                "opaque STABG.BIN-sized bytes fail the STMP framing receipt");
        local_stabg = read_local_nexus_file("STABG.BIN", &local_warning_size);
        if (!local_stabg) {
            puts("SKIP: local Nexus STABG.BIN not present");
        } else {
            expect_true(nexus_ui_stabg_stmp_framing_receipt(
                            local_stabg, (int)local_warning_size,
                            &stabg_framing) == 0 &&
                        stabg_framing.valid == 1 &&
                        stabg_framing.declared_size == local_warning_size &&
                        stabg_framing.directory_offset == 0x20U &&
                        stabg_framing.palette_size == 512U &&
                        stabg_framing.pixels_offset +
                            stabg_framing.pixels_size ==
                            local_warning_size &&
                        stabg_framing.map_count == 11 &&
                        stabg_framing.background_cell_w == 40 &&
                        stabg_framing.background_cell_h == 21 &&
                        stabg_framing.cell_offsets_bounded == 1,
                    "local STABG.BIN proves its STMP container framing");
            {
                unsigned char *unterminated = (unsigned char *)malloc(
                    local_warning_size);
                if (unterminated) {
                    Nexus_UI_StabgDmwebReceipt dmweb_receipt;
                    memcpy(unterminated, local_stabg, local_warning_size);
                    /* The retail table has 11 offsets followed by zero at
                     * part-1 offset 0x4c. Replace that terminator with a
                     * nonzero offset; the parser must not accept 64 entries
                     * merely because its bounded array is full. */
                    unterminated[0x4c] = 0x00;
                    unterminated[0x4d] = 0x00;
                    unterminated[0x4e] = 0x00;
                    unterminated[0x4f] = 0x30;
                    expect_true(nexus_ui_stabg_dmweb_decode_receipt(
                                    unterminated, (int)local_warning_size,
                                    &dmweb_receipt) < 0 &&
                                    dmweb_receipt.valid == 0,
                                "STABG DMWeb offset table requires a terminator");
                    free(unterminated);
                } else {
                    expect_true(0, "STABG terminator regression allocation");
                }
            }
            expect_true(nexus_ui_load_stabg(&ui, local_stabg,
                                            (int)local_warning_size,
                                            NULL) > 0 &&
                        ui.surfaces[NEXUS_SURFACE_STABG].data != NULL &&
                        ui.surfaces[NEXUS_SURFACE_STABG].w == 320 &&
                        ui.surfaces[NEXUS_SURFACE_STABG].h == 168 &&
                        ui.surfaces[NEXUS_SURFACE_STABG].source_palette_loaded &&
                        ui.surfaces[NEXUS_SURFACE_STABG].source_bytes_fnv1a64 != 0U &&
                        ui.surfaces[NEXUS_SURFACE_STABG].source_bytes_size ==
                            local_warning_size,
                    "framed STABG.BIN loads its verified first map and palette");
            free(local_stabg);
        }
    }

    local_warning = read_local_nexus_file("WARNING.BIN", &local_warning_size);
    if (!local_warning) {
        puts("SKIP: local Nexus WARNING.BIN not present");
    } else {
        Nexus_TitleScreen title;
        Nexus_Framebuffer framebuffer;
        nexus_ui_manager_init(&ui);
        expect_true(nexus_ui_res_dgt2_pp_view(local_warning, local_warning_size,
                                               0U, &warning_view) == 0 &&
                        warning_view.width == 240 && warning_view.height == 96 &&
                        warning_view.pixel_bytes == 240U * 96U &&
                        warning_view.clut_bgr555_be == local_warning + 0x56 &&
                        warning_view.pixels == local_warning + 0x256,
                    "local WARNING.BIN resource zero is documented DGT2 PP art");
        {
            unsigned char *malformed = (unsigned char *)malloc(local_warning_size);
            if (malformed) {
                memcpy(malformed, local_warning, local_warning_size);
                /* Entry 1 is not selected below. Move it into entry 0's
                 * payload; a selected-entry-only scan would accept resource
                 * zero unchanged. */
                malformed[32] = 0x00;
                malformed[33] = 0x00;
                malformed[34] = 0x00;
                malformed[35] = 0x59;
                expect_true(nexus_ui_res_dgt2_pp_view(
                                malformed, local_warning_size, 0U,
                                &warning_view) < 0,
                            "RES* lookup validates unselected DGT2 records");
                free(malformed);
            } else {
                expect_true(0, "RES* directory regression allocation");
            }
        }
        expect_true(nexus_ui_load_warning(&ui, local_warning,
                                          (int)local_warning_size, NULL) > 0 &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].w == 240 &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].h == 96 &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].data[0] ==
                            local_warning[0x256] &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].dgt2_palette_loaded &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].dgt2_palette_rgba[0] ==
                            dgt2_rgba(warning_view.clut_bgr555_be, 0) &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].dgt2_palette_rgba[255] ==
                            dgt2_rgba(warning_view.clut_bgr555_be, 255) &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].source_bytes_fnv1a64 != 0U &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].source_bytes_size ==
                            local_warning_size,
                    "WARNING.BIN binds its documented DGT2 pixel plane and BGR555 CLUT");
        memset(&title, 0, sizeof(title));
        title.warning_pixels = ui.surfaces[NEXUS_SURFACE_WARNING].data;
        title.warning_width = ui.surfaces[NEXUS_SURFACE_WARNING].w;
        title.warning_height = ui.surfaces[NEXUS_SURFACE_WARNING].h;
        title.warning_loaded = 1;
        memcpy(title.warning_palette_rgba,
               ui.surfaces[NEXUS_SURFACE_WARNING].dgt2_palette_rgba,
               sizeof(title.warning_palette_rgba));
        title.warning_palette_loaded = 1;
        nexus_fb_init(&framebuffer);
        nexus_render_title(&title, &framebuffer, 0);
        {
            size_t pixel_index;
            int nonzero_pixels = 0;
            for (pixel_index = 0; pixel_index <
                              (size_t)NEXUS_FB_W * (size_t)NEXUS_FB_H;
                 ++pixel_index) {
                if (framebuffer.color_buffer[pixel_index] != 0U) {
                    ++nonzero_pixels;
                }
            }
            expect_true(nonzero_pixels == 0,
                        "Nexus warning title host renderer stays no-draw without Saturn capture");
        }
        nexus_ui_manager_free(&ui);
        free(local_warning);
    }

    local_gameover = read_local_nexus_file("GAMEOVER.BIN", &local_warning_size);
    if (!local_gameover) {
        puts("SKIP: local Nexus GAMEOVER.BIN not present");
    } else {
        nexus_ui_manager_init(&ui);
        expect_true(nexus_ui_res_dgt2_pp_view(local_gameover, local_warning_size,
                                               0U, &warning_view) == 0 &&
                        nexus_ui_load_gameover(&ui, local_gameover,
                                               (int)local_warning_size, NULL) > 0 &&
                        ui.surfaces[NEXUS_SURFACE_GAMEOVER].w == warning_view.width &&
                        ui.surfaces[NEXUS_SURFACE_GAMEOVER].h == warning_view.height &&
                        ui.surfaces[NEXUS_SURFACE_GAMEOVER].data[0] == warning_view.pixels[0] &&
                        ui.surfaces[NEXUS_SURFACE_GAMEOVER].dgt2_palette_loaded &&
                        ui.surfaces[NEXUS_SURFACE_GAMEOVER].dgt2_palette_rgba[0] ==
                            dgt2_rgba(warning_view.clut_bgr555_be, 0) &&
                        ui.surfaces[NEXUS_SURFACE_GAMEOVER].source_bytes_fnv1a64 != 0U &&
                        ui.surfaces[NEXUS_SURFACE_GAMEOVER].source_bytes_size ==
                            local_warning_size,
                    "GAMEOVER.BIN binds its verified DGT2 PP pixels and CLUT");
        nexus_ui_manager_free(&ui);
        free(local_gameover);
    }

    local_title = read_local_nexus_file("TITLE.CG", &local_warning_size);
    if (!local_title) {
        puts("SKIP: local Nexus TITLE.CG not present");
    } else {
        nexus_ui_manager_init(&ui);
        expect_true(nexus_ui_load_title(&ui, local_title,
                                        (int)local_warning_size, NULL) > 0 &&
                        ui.surfaces[NEXUS_SURFACE_TITLE].w ==
                            NEXUS_UI_TITLE_CG_WIDTH &&
                        ui.surfaces[NEXUS_SURFACE_TITLE].h ==
                            NEXUS_UI_TITLE_CG_HEIGHT &&
                        ui.surfaces[NEXUS_SURFACE_TITLE].data[0] == 7 &&
                        ui.surfaces[NEXUS_SURFACE_TITLE].data[1] == 7 &&
                        ui.surfaces[NEXUS_SURFACE_TITLE].source_bytes_fnv1a64 != 0U &&
                        ui.surfaces[NEXUS_SURFACE_TITLE].source_bytes_size ==
                            local_warning_size,
                    "local TITLE.CG expands its verified 328x1024 4bpp atlas");
        nexus_ui_manager_free(&ui);
        free(local_title);
    }

    engine.ui_faces_expected = NEXUS_FACE_BIN_PORTRAIT_COUNT;
    engine.ui_faces_loaded = 0;
    engine.ui_faces_fallback = NEXUS_FACE_BIN_PORTRAIT_COUNT;
    expect_true(!nexus_v1_startup_faces_ready(&engine),
                "startup face placeholders cannot satisfy the route");
    engine.ui_faces_loaded = NEXUS_FACE_BIN_PORTRAIT_COUNT;
    engine.ui_faces_fallback = 0;
    expect_true(nexus_v1_startup_faces_ready(&engine),
                "only complete decoded face records satisfy the route");

    engine.ui_startup_surfaces_expected = 4;
    engine.ui_startup_surfaces_loaded = 3;
    engine.ui_startup_surfaces_fallback = 1;
    expect_true(!nexus_v1_startup_surfaces_ready(&engine),
                "startup surface fallback cannot satisfy the route");
    engine.ui_startup_surfaces_loaded = 4;
    engine.ui_startup_surfaces_fallback = 0;
    expect_true(nexus_v1_startup_surfaces_ready(&engine),
                "only complete decoded startup surfaces satisfy the route");

    if (failures != 0) {
        fprintf(stderr, "Nexus startup media gate failed: %d\n", failures);
        return 1;
    }
    printf("Nexus startup media gate: PASS\n");
    return 0;
}
