/* Nexus startup media must be decoded before it is presented by M11. */
#include "nexus_v1_engine.h"
#include "nexus_v1_ui_surfaces.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    Nexus_UI_Manager ui;
    Nexus_V1_Engine engine;
    unsigned char opaque_title[64001];
    unsigned char opaque_warning[64001];
    unsigned char decoded_plane[320 * 200];

    memset(&ui, 0, sizeof(ui));
    memset(&engine, 0, sizeof(engine));
    memset(opaque_title, 0x5a, sizeof(opaque_title));
    memset(opaque_warning, 0x52, sizeof(opaque_warning));
    memset(decoded_plane, 0x11, sizeof(decoded_plane));
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
    expect_true(nexus_ui_load_title(&ui, decoded_plane,
                                    (int)sizeof(decoded_plane), NULL) > 0 &&
                    ui.surfaces[NEXUS_SURFACE_TITLE].data != NULL,
                "the generic title helper still accepts an exact decoded plane");
    nexus_ui_manager_free(&ui);

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
