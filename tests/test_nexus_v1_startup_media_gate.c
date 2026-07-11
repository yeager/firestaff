/* Nexus startup media must be decoded before it is presented by M11. */
#include "nexus_v1_engine.h"
#include "nexus_v1_ui_surfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

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
    char path[1024];
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
    unsigned char *local_title;
    size_t local_warning_size = 0U;
    Nexus_UI_Dgt2PpView warning_view;

    memset(&ui, 0, sizeof(ui));
    memset(&engine, 0, sizeof(engine));
    memset(opaque_title, 0x5a, sizeof(opaque_title));
    memset(opaque_warning, 0x52, sizeof(opaque_warning));
    memcpy(opaque_warning, "RES*", 4);

    nexus_ui_manager_init(&ui);
    expect_true(nexus_ui_load_title(&ui, opaque_title,
                                    (int)sizeof(opaque_title), NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_TITLE].data == NULL,
                "opaque TITLE.CG bytes cannot become a startup raster");
    expect_true(nexus_ui_load_warning(&ui, opaque_warning,
                                      (int)sizeof(opaque_warning), NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_WARNING].data == NULL,
                "RES* WARNING.BIN bytes cannot become a startup raster");
    nexus_ui_manager_free(&ui);

    local_warning = read_local_nexus_file("WARNING.BIN", &local_warning_size);
    if (!local_warning) {
        puts("SKIP: local Nexus WARNING.BIN not present");
    } else {
        nexus_ui_manager_init(&ui);
        expect_true(nexus_ui_res_dgt2_pp_view(local_warning, local_warning_size,
                                               0U, &warning_view) == 0 &&
                        warning_view.width == 240 && warning_view.height == 96 &&
                        warning_view.pixel_bytes == 240U * 96U &&
                        warning_view.clut_bgr555_be == local_warning + 0x56 &&
                        warning_view.pixels == local_warning + 0x256,
                    "local WARNING.BIN resource zero is documented DGT2 PP art");
        expect_true(nexus_ui_load_warning(&ui, local_warning,
                                          (int)local_warning_size, NULL) > 0 &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].w == 240 &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].h == 96 &&
                        ui.surfaces[NEXUS_SURFACE_WARNING].data[0] ==
                            local_warning[0x256],
                    "WARNING.BIN loads its decoded Saturn DGT2 pixel plane");
        nexus_ui_manager_free(&ui);
        free(local_warning);
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
                        ui.surfaces[NEXUS_SURFACE_TITLE].data[1] == 7,
                    "local TITLE.CG expands its verified 328x1024 4bpp atlas");
        nexus_ui_manager_free(&ui);
        free(local_title);
    }

    engine.ui_faces_expected = 24;
    engine.ui_faces_loaded = 0;
    engine.ui_faces_fallback = 24;
    expect_true(!nexus_v1_startup_faces_ready(&engine),
                "startup face placeholders cannot satisfy the route");
    engine.ui_faces_loaded = 24;
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
