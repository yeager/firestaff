#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_palette_runtime_admission.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

int main(void) {
    Theron_V1_World world;
    Theron_V1Track02PaletteRouteReceipt palette = {0};
    Theron_V1PaletteRuntimeAdmissionReceipt admission;
    uint8_t pixels[64u * 8u] = {0};

    theron_v1_world_init(&world);
    theron_v1_world_runtime_media_clear(&world);
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_TITLE,
        THERON_TRACK02_MD5_US_BIN, THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
        64u, 8u, 0x00200010u, 0x0020002cu, 0x001ff020u, 8u, 512u,
        0x11112222u, pixels, sizeof(pixels)));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE,
        THERON_TRACK02_MD5_US_BIN, THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
        64u, 8u, 0x00280010u, 0x0028002cu, 0x0027f020u, 8u, 512u,
        0x33334444u, pixels, sizeof(pixels)));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM,
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 64u, 8u,
        0x00300010u, 0x0030002cu, 0x002ff020u, 8u, 512u,
        0x44445555u, pixels, sizeof(pixels)));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD,
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 64u, 8u,
        0x00380010u, 0x0038002cu, 0x0037f020u, 8u, 512u,
        0x55556666u, pixels, sizeof(pixels)));
    world.runtime_media.identity.ready = 1;
    world.runtime_media.identity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;

    palette.accepted = 1;
    palette.real_cd_verified = 1;
    palette.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(palette.track02_md5, sizeof(palette.track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    palette.vce_index_address = 0x0402u;
    palette.vce_low_address = 0x0403u;
    palette.vce_high_address = 0x0404u;
    palette.vce_index = 0x06u;
    palette.vce_low = 0x2au;
    palette.vce_high = 0x01u;

    CHECK(theron_v1_palette_runtime_admit_track02_surface(
        &palette, &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE, &admission));
    CHECK(admission.valid);
    CHECK(admission.authenticated_palette_route_consumed);
    CHECK(admission.runtime_surface_consumed);
    CHECK(admission.runtime_palette_admission_allowed);
    CHECK(admission.bitmap_route_bit == THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);
    CHECK(admission.bitmap_palette_relation_verified == 0);
    CHECK(!admission.render_allowed);
    CHECK(!admission.dungeon_draw_allowed);
    CHECK(!admission.fallback_visuals_allowed);

    palette.track02_variant = THERON_TRACK02_VARIANT_JP_BIN;
    CHECK(!theron_v1_palette_runtime_admit_track02_surface(
        &palette, &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE, &admission));
    palette.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(world.runtime_media.stage.track02_md5, THERON_TRACK02_MD5_JP_BIN);
    CHECK(!theron_v1_palette_runtime_admit_track02_surface(
        &palette, &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE, &admission));
    strcpy(world.runtime_media.stage.track02_md5, THERON_TRACK02_MD5_US_BIN);
    palette.render_allowed = 1;
    CHECK(!theron_v1_palette_runtime_admit_track02_surface(
        &palette, &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE, &admission));

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
