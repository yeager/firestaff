/*
 * test_dm2_v1_weather_runtime_admission.c
 *
 * Binds GRAPHICS_DATA_OPEN to DM2 weather runtime admission without inventing
 * weather blits when real GDAT only proves command text.
 */

#include "dm2_v1_graphics_data_open.h"
#include "dm2_v1_weather_gdat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_graphics(uint8_t **out_data, size_t *out_size,
                         char *path, size_t path_size)
{
    const char *dm2_data = getenv("FIRESTAFF_DM2_DATA_DIR");
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    size_t i;

    /* FIRESTAFF_DM2_DATA_DIR names the mounted game directory itself;
     * use its original file directly before checking parent-root layouts. */
    if (dm2_data && dm2_data[0]) {
        static const char *names[] = {
            "GRAPHICS.DAT", "graphics.dat", "DM2GRAPHICS.DAT", "DM2GRA.DAT"
        };
        for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i) {
            snprintf(path, path_size, "%s/%s", dm2_data, names[i]);
            if (read_file(path, out_data, out_size)) return 1;
        }
    }

    for (i = 0u; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_GraphicsDataOpenReceipt open_receipt;
    DM2_V1_AssetLoader loader;
    DM2_V1_WeatherGdatReceipt weather;
    DM2_V1_WeatherRuntimeAdmissionReceipt admission;
    int weather_found = 0;

    printf("=== DM2 V1 Weather Runtime Admission Test ===\n");
    memset(&loader, 0, sizeof(loader));
    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return 0;
    }

    CHECK(dm2_v1_GRAPHICS_DATA_OPEN_receipt(
              graphics, graphics_size, &open_receipt) == 1 &&
              open_receipt.valid,
          "real GRAPHICS_DATA_OPEN receipt admits source GDAT");
    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real GRAPHICS.DAT initializes weather GDAT loader");

    for (int graphicsset = 1; graphicsset <= 5 && !weather_found;
         ++graphicsset) {
        if (dm2_v1_weather_gdat_receipt(&loader, (uint8_t)graphicsset,
                                        &weather)) {
            weather_found = 1;
        }
    }
    CHECK(weather_found && weather.valid && weather.command_mask != 0u,
          "real weather GDAT exposes a source command-text receipt");
    if (weather_found) {
        CHECK(dm2_v1_weather_runtime_admission_receipt(
                  &open_receipt, &weather, NULL, &admission) == 1 &&
                  admission.valid && admission.source_text_ready &&
                  admission.graphics_data_open_hash ==
                      open_receipt.admission_hash &&
                  admission.weather_receipt_hash == weather.receipt_hash,
              "weather runtime admission consumes GRAPHICS_DATA_OPEN and text");
        CHECK(admission.no_fallback_blit && !admission.blit_authorized &&
                  !admission.renderer_ready &&
                  admission.material_mask == weather.material_mask &&
                  admission.material_ready == (weather.material_mask != 0u),
              "real weather remains no-draw until its material renderer exists");

        memset(&open_receipt, 0, sizeof(open_receipt));
        CHECK(dm2_v1_weather_runtime_admission_receipt(
                  &open_receipt, &weather, NULL, &admission) == 0 &&
                  !admission.valid,
              "missing GRAPHICS_DATA_OPEN blocks weather runtime admission");
    }

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
