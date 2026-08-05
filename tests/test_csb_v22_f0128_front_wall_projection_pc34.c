#include "csb_v1_boot.h"
#include "csb_v22_inplace_route_pc34.h"
#include "csb_v22_shapes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int depth;
    int graphic_index;
    int width;
    int height;
    int clip_x;
    int clip_y;
    int draw_order;
    uint32_t decoded_fnv1a;
    const char* asset_id;
    const char* record_sha256;
} FrontWallExpected;

static const FrontWallExpected kExpected[] = {
    { 0, 97, 160, 111, 32, 9, 0x0124, 0xc299474au,
      "wall_dungeon_d0_01",
      "33e729bf8c6aebf2d1c9f41bd9df5476c1f72a27be2bb3b6b87b88a151246303" },
    { 1, 102, 106, 74, 59, 19, 0x0121, 0x3f9521ceu,
      "wall_dungeon_d1_01",
      "f0f8a977b6b011b3ca59a14ef113e94c0a48b971bb157e9d500be8f863a1a59c" },
    { 2, 107, 70, 49, 77, 25, 0x0118, 0x3b83fab3u,
      "wall_dungeon_d2_01",
      "faae0db907fbd5ad9ab6a67700fc2f6a533d394331990e028dffc5f7404fd3a6" }
};

static int failures;
static int checks;

#define CHECK(expr) do { \
    ++checks; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static uint32_t fnv1a32(const unsigned char* bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t index;

    for (index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static CSB_V22_RouteProvenancePc34 make_provenance(
    const FrontWallExpected* expected)
{
    CSB_V22_RouteProvenancePc34 provenance;

    memset(&provenance, 0, sizeof(provenance));
    provenance.valid = 1;
    snprintf(provenance.id, sizeof(provenance.id), "%s", expected->asset_id);
    snprintf(provenance.category, sizeof(provenance.category), "wall_shapes");
    provenance.source_graphic_index = expected->graphic_index;
    provenance.source_width = expected->width;
    provenance.source_height = expected->height;
    snprintf(provenance.source_record_sha256,
             sizeof(provenance.source_record_sha256), "%s",
             expected->record_sha256);
    provenance.output_width = expected->width;
    provenance.output_height = expected->height;
    return provenance;
}

static void check_projection_contract(void)
{
    size_t index;

    for (index = 0; index < sizeof(kExpected) / sizeof(kExpected[0]); ++index) {
        const FrontWallExpected* expected = &kExpected[index];
        CSB_V22_RouteProvenancePc34 provenance =
            make_provenance(expected);
        CSB_V22_F0128ProjectionCommandPc34 projection;

        memset(&projection, 0, sizeof(projection));
        CHECK(csb_v22_admit_f0128_front_wall_projection_pc34(
            expected->depth, 0, 749u, expected->record_sha256,
            &provenance, &projection));
        CHECK(projection.valid);
        CHECK(strcmp(projection.category, "wall_shapes") == 0);
        CHECK(strcmp(projection.asset_id, expected->asset_id) == 0);
        CHECK(projection.source_graphic_index == expected->graphic_index);
        CHECK(projection.source_width == expected->width);
        CHECK(projection.source_height == expected->height);
        CHECK(projection.transparent_index == -1);
        CHECK(projection.clip_x == expected->clip_x);
        CHECK(projection.clip_y == expected->clip_y);
        CHECK(projection.clip_w == expected->width);
        CHECK(projection.clip_h == expected->height);
        CHECK(projection.draw_order == expected->draw_order);
    }
}

static void check_fail_closed_contract(void)
{
    CSB_V22_RouteProvenancePc34 provenance = make_provenance(&kExpected[0]);
    CSB_V22_F0128ProjectionCommandPc34 projection;

    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        -1, 0, 749u, kExpected[0].record_sha256, &provenance, &projection));
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        3, 0, 749u, kExpected[0].record_sha256, &provenance, &projection));
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        0, 1, 749u, kExpected[0].record_sha256, &provenance, &projection));
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        0, 0, 97u, kExpected[0].record_sha256, &provenance, &projection));

    provenance.source_width = 159;
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        0, 0, 749u, kExpected[0].record_sha256, &provenance, &projection));
    provenance = make_provenance(&kExpected[0]);
    provenance.output_width = 96;
    provenance.output_height = 96;
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        0, 0, 749u, kExpected[0].record_sha256, &provenance, &projection));
    provenance = make_provenance(&kExpected[0]);
    snprintf(provenance.category, sizeof(provenance.category), "door_shapes");
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        0, 0, 749u, kExpected[0].record_sha256, &provenance, &projection));
    provenance = make_provenance(&kExpected[0]);
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        0, 0, 749u,
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        &provenance, &projection));
    CHECK(!csb_v22_admit_f0128_front_wall_projection_pc34(
        0, 0, 749u, "not-a-sha256", &provenance, &projection));
    CHECK(!projection.valid);
}

static void check_legacy_route_api_is_product_no_draw(void)
{
    CSB_V22_AssetRouteDecision decision;
    char asset_id[CSB_V22_ASSET_ID_MAX];
    char category[CSB_V22_CATEGORY_MAX];
    char reason[CSB_V22_REASON_MAX];
    const char* evidence;

    memset(&decision, 0xA5, sizeof(decision));
    csb_v22_inplace_route_cell(0, 0, 0x00, 1, &decision);
    CHECK(!decision.use_v22);
    CHECK(decision.asset_id[0] == '\0' && decision.category[0] == '\0');
    CHECK(decision.shape_type == -1);
    CHECK(strcmp(decision.fallback_reason,
                 "v1_original_material_unbound_raw_cell") == 0);

    csb_v22_inplace_route_square_element_pc34(0, 0, 0, 1, &decision);
    CHECK(!decision.use_v22);
    CHECK(strcmp(decision.fallback_reason,
                 "v1_original_material_unbound_square_element") == 0);

    memset(asset_id, 0xA5, sizeof(asset_id));
    memset(category, 0xA5, sizeof(category));
    memset(reason, 0xA5, sizeof(reason));
    CHECK(!csb_v22_inplace_route_for_shape(
        CSB_V22_SHAPE_CEILING_PLAIN, 1, asset_id, sizeof(asset_id),
        category, sizeof(category), reason, sizeof(reason)));
    CHECK(asset_id[0] == '\0' && category[0] == '\0');
    CHECK(strcmp(reason, "v1_original_material_unbound_ceiling") == 0);
    CHECK(csb_v22_inplace_route_pair_count() == 0);
    CHECK(!csb_v22_inplace_route_pair_recognized(
        "wall_shapes", "wall_dungeon_d0_01"));
    evidence = csb_v22_inplace_route_source_evidence();
    CHECK(evidence && strstr(evidence, "test-only"));
}

static const char* real_graphics_path(void)
{
    static char path[1024];
    const char* configured = getenv("FIRESTAFF_CSB_PC34_GRAPHICS_DAT");
    const char* home;
    FILE* file;

    if (configured && configured[0]) return configured;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/csb/GRAPHICS.DAT", home);
    file = fopen(path, "rb");
    if (!file) return NULL;
    fclose(file);
    return path;
}

static void check_real_pc34_records(void)
{
    const char* path = real_graphics_path();
    size_t index;

    if (!path) {
        puts("SKIP: no local CSB PC3.4 GRAPHICS.DAT");
        return;
    }

    for (index = 0; index < sizeof(kExpected) / sizeof(kExpected[0]); ++index) {
        const FrontWallExpected* expected = &kExpected[index];
        CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;
        CSB_V22_RouteProvenancePc34 provenance =
            make_provenance(expected);
        CSB_V22_F0128ProjectionCommandPc34 projection;
        unsigned char* pixels = NULL;
        int width = 0;
        int height = 0;

        memset(&receipt, 0, sizeof(receipt));
        CHECK(csb_v1_boot_decode_graphics_dat_asset_pc34(
            path, (unsigned int)expected->graphic_index, &pixels,
            &width, &height, &receipt));
        CHECK(receipt.valid && pixels);
        CHECK(width == expected->width);
        CHECK(height == expected->height);
        CHECK(fnv1a32(pixels, (size_t)width * (size_t)height) ==
              expected->decoded_fnv1a);
        CHECK(receipt.indexed_pixel_fnv1a == expected->decoded_fnv1a);
        /* The live F0128 gate must bind this exact compressed F0490 record,
         * not merely an equivalent decoded raster. */
        CHECK(strcmp(receipt.compressed_record_sha256,
                     expected->record_sha256) == 0);
        CHECK(csb_v22_admit_f0128_front_wall_projection_pc34(
            expected->depth, 0, 749u, expected->record_sha256,
            &provenance, &projection));
        CHECK(projection.clip_w == width && projection.clip_h == height);
        free(pixels);
    }
}

int main(void)
{
    const char* evidence =
        csb_v22_f0128_front_wall_projection_source_evidence_pc34();

    check_projection_contract();
    check_fail_closed_contract();
    check_legacy_route_api_is_product_no_draw();
    CHECK(evidence && strstr(evidence, "F0095_LoadWallSet"));
    CHECK(strstr(evidence, "C712_ZONE_WALL_D1C"));
    CHECK(strstr(evidence, "C709_ZONE_WALL_D2C"));
    CHECK(strstr(evidence, "C704_ZONE_WALL_D3C"));
    CHECK(strstr(evidence, "NO_TRANSPARENCY"));
    CHECK(strstr(evidence, "no side-wall"));
    check_real_pc34_records();

    printf("csb_v22_f0128_front_wall_projection_pc34: checks=%d failures=%d\n",
           checks, failures);
    return failures ? 1 : 0;
}
