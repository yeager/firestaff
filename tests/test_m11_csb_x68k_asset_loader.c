#include "asset_loader_m11.h"
#include "csb_v1_x68k_hdm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size) {
    FILE *file = NULL;
    long length;
    uint8_t *bytes = NULL;
    if (!path || !out || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0 || !(bytes = malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        if (file) fclose(file);
        free(bytes);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)length;
    return 1;
}

static int synthetic_contract(void) {
    enum { COUNT = 700, HEADER = 4 + COUNT * 8, SIZE = 300000 };
    uint8_t *data = calloc(SIZE, 1u);
    M11_AssetLoader loader;
    const M11_AssetSlot *slot;
    uint16_t width = 0u, height = 0u;
    int ok;

    if (!data) return 0;
    memset(&loader, 0, sizeof(loader));
    data[0] = 0x80u; data[1] = 0x01u;
    data[2] = (uint8_t)(COUNT >> 8u); data[3] = (uint8_t)COUNT;
    data[4u + 13u * 2u] = 0u; data[5u + 13u * 2u] = 5u;
    data[4u + (size_t)COUNT * 2u + 13u * 2u] = 0u;
    data[5u + (size_t)COUNT * 2u + 13u * 2u] = 5u;
    data[HEADER] = 0u; data[HEADER + 1u] = 1u;
    data[HEADER + 2u] = 0u; data[HEADER + 3u] = 1u;
    data[HEADER + 4u] = 0x0bu;
    ok = M11_AssetLoader_InitCsbX68kFromBuffer(&loader, data, SIZE) &&
        loader.csbX68k && !loader.csbAmiga && loader.graphicCount == COUNT &&
        M11_AssetLoader_QuerySize(&loader, 13u, &width, &height) &&
        width == 1u && height == 1u &&
        (slot = M11_AssetLoader_Load(&loader, 13u)) != NULL &&
        slot->width == 1u && slot->height == 1u && slot->pixels[0] == 11u &&
        M11_AssetLoader_Load(&loader, COUNT) == NULL;
    if (loader.initialized) M11_AssetLoader_Shutdown(&loader);
    free(data);
    return ok;
}

int main(void) {
    const char *path = getenv("FIRESTAFF_CSB_X68K_HDM");
    if (!synthetic_contract()) {
        puts("test_m11_csb_x68k_asset_loader: synthetic contract failed");
        return 1;
    }
    if (!path || !path[0]) {
        puts("test_m11_csb_x68k_asset_loader: SKIP FIRESTAFF_CSB_X68K_HDM unset");
        return 0;
    }
    {
        uint8_t *hdm = NULL;
        uint8_t *graphics = NULL;
        size_t hdm_size = 0u, graphics_size = 0u;
        M11_AssetLoader loader;
        const M11_AssetSlot *slot;
        int initialized = 0;
        int ok;
        memset(&loader, 0, sizeof(loader));
        ok = read_file(path, &hdm, &hdm_size) &&
            csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "GRAPHICS.DAT",
                                               NULL, 0u, &graphics_size, NULL) &&
            (graphics = malloc(graphics_size)) != NULL &&
            csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "GRAPHICS.DAT",
                                               graphics, graphics_size,
                                               &graphics_size, NULL) &&
            (initialized = M11_AssetLoader_InitCsbX68kFromBuffer(
                &loader, graphics, (long)graphics_size)) &&
            loader.csbX68k && !loader.csbAmiga && loader.graphicCount == 732u &&
            (slot = M11_AssetLoader_Load(&loader, 13u)) != NULL &&
            slot->width == 96u && slot->height == 41u;
        if (initialized) M11_AssetLoader_Shutdown(&loader);
        free(graphics);
        free(hdm);
        if (!ok) {
            puts("test_m11_csb_x68k_asset_loader: original media mismatch");
            return 1;
        }
    }
    puts("test_m11_csb_x68k_asset_loader: PASS");
    return 0;
}
