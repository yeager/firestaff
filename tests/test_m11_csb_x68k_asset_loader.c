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

static void put_le16(uint8_t *p, unsigned int value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8u);
}

static void put_le32(uint8_t *p, size_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8u);
    p[2] = (uint8_t)(value >> 16u);
    p[3] = (uint8_t)(value >> 24u);
}

static void put_fat12(uint8_t *fat, unsigned int cluster, unsigned int value) {
    size_t at = (size_t)cluster + ((size_t)cluster >> 1u);
    if (cluster & 1u) {
        fat[at] = (uint8_t)((fat[at] & 0x0fu) | (value << 4u));
        fat[at + 1u] = (uint8_t)(value >> 4u);
    } else {
        fat[at] = (uint8_t)value;
        fat[at + 1u] = (uint8_t)((fat[at + 1u] & 0xf0u) | (value >> 8u));
    }
}

static uint8_t *make_hdm_with_graphics(const uint8_t *graphics,
                                       size_t graphics_size) {
    enum { ROOT = 5 * 1024, DATA = 11 * 1024, SECTOR = 1024 };
    uint8_t *hdm;
    size_t offset, copied;
    unsigned int cluster, clusters;

    if (!graphics || !graphics_size) return NULL;
    clusters = (unsigned int)((graphics_size + SECTOR - 1u) / SECTOR);
    if (clusters > 1000u) return NULL;
    hdm = calloc(CSB_V1_X68K_HDM_BYTES_PER_DISK, 1u);
    if (!hdm) return NULL;
    hdm[0] = 0x60u; hdm[1] = 0x1cu;
    memcpy(hdm + 2u, "Hudson soft 2.00", 16u);
    hdm[1024] = 0xfeu; hdm[1025] = 0xffu; hdm[1026] = 0xffu;
    memcpy(hdm + ROOT, "GRAPHICSDAT", 11u);
    put_le16(hdm + ROOT + 26u, 2u);
    put_le32(hdm + ROOT + 28u, graphics_size);
    for (cluster = 2u; cluster < 2u + clusters; ++cluster)
        put_fat12(hdm + 1024u, cluster,
                  cluster + 1u == 2u + clusters ? 0xfffu : cluster + 1u);
    copied = 0u;
    for (cluster = 2u; copied < graphics_size; ++cluster) {
        size_t take = graphics_size - copied;
        if (take > SECTOR) take = SECTOR;
        offset = DATA + (size_t)(cluster - 2u) * SECTOR;
        memcpy(hdm + offset, graphics + copied, take);
        copied += take;
    }
    return hdm;
}

static int synthetic_contract(void) {
    enum { COUNT = 700, HEADER = 4 + COUNT * 8, SIZE = 300000 };
    uint8_t *data = calloc(SIZE, 1u);
    M11_AssetLoader loader;
    const M11_AssetSlot *slot;
    uint16_t width = 0u, height = 0u;
    uint8_t *hdm;
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
    hdm = make_hdm_with_graphics(data, SIZE);
    ok = hdm && M11_AssetLoader_InitCsbX68kFromHdm(&loader, hdm,
        CSB_V1_X68K_HDM_BYTES_PER_DISK) &&
        loader.csbX68k && !loader.csbAmiga && loader.graphicCount == COUNT &&
        M11_AssetLoader_QuerySize(&loader, 13u, &width, &height) &&
        width == 1u && height == 1u &&
        (slot = M11_AssetLoader_Load(&loader, 13u)) != NULL &&
        slot->width == 1u && slot->height == 1u && slot->pixels[0] == 11u &&
        M11_AssetLoader_Load(&loader, COUNT) == NULL;
    if (loader.initialized) M11_AssetLoader_Shutdown(&loader);
    free(hdm);
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
        size_t hdm_size = 0u, graphics_size = 0u;
        M11_AssetLoader loader;
        const M11_AssetSlot *slot;
        int initialized = 0;
        int ok;
        memset(&loader, 0, sizeof(loader));
        ok = read_file(path, &hdm, &hdm_size) &&
            csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "GRAPHICS.DAT",
                                               NULL, 0u, &graphics_size, NULL) &&
            graphics_size == 373583u &&
            (initialized = M11_AssetLoader_InitCsbX68kFromHdm(
                &loader, hdm, hdm_size)) &&
            loader.csbX68k && !loader.csbAmiga && loader.graphicCount == 732u &&
            (slot = M11_AssetLoader_Load(&loader, 13u)) != NULL &&
            slot->width == 96u && slot->height == 41u;
        if (initialized) M11_AssetLoader_Shutdown(&loader);
        free(hdm);
        if (!ok) {
            puts("test_m11_csb_x68k_asset_loader: original media mismatch");
            return 1;
        }
    }
    puts("test_m11_csb_x68k_asset_loader: PASS");
    return 0;
}
