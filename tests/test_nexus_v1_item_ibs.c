#include "nexus_v1_item_ibs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

static uint32_t fnv1a_rgba(const uint32_t *pixels, int count) {
    uint32_t hash = 0x811C9DC5U;
    int i;
    for (i = 0; i < count; ++i) {
        uint32_t value = pixels[i];
        hash = (hash ^ (value & 0xffU)) * 0x01000193U;
        hash = (hash ^ ((value >> 8) & 0xffU)) * 0x01000193U;
        hash = (hash ^ ((value >> 16) & 0xffU)) * 0x01000193U;
        hash = (hash ^ ((value >> 24) & 0xffU)) * 0x01000193U;
    }
    return hash;
}

static int test_synthetic(void) {
    Nexus_V1_ItemIbsHeader hdr;
    uint8_t bad[64];
    memset(bad, 0, sizeof(bad));
    if (nexus_v1_item_ibs_parse_header(bad, 64, &hdr)) return 1;
    if (nexus_v1_item_ibs_parse_header(NULL, 0, &hdr)) return 1;
    printf("  PASS synthetic\n");
    return 0;
}

static int test_real_decode(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_ItemIbsDecodeResult result;
    int i, unique_img, unique_floor;
    int reused_palette_index = -1;
    uint32_t seen[256];
    uint32_t floor_rgba[16384];

    if (data_dir && data_dir[0] != '\0') {
        snprintf(path, sizeof(path), "%s/ITEM.IBS", data_dir);
    } else if (home) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/ITEM.IBS", home);
    } else {
        printf("  SKIP real_decode (no data directory)\n");
        return 0;
    }
    data = load_file(path, &size);
    if (!data) { printf("  SKIP real_decode (no ITEM.IBS)\n"); return 0; }

    if (size != 100352) {
        printf("  FAIL unexpected file size %d (expected 100352)\n", size);
        free(data);
        return 1;
    }

    if (!nexus_v1_item_ibs_decode(data, size, &result)) {
        printf("  FAIL decode failed\n");
        free(data);
        return 1;
    }

    printf("  items=%d images=%d floor=%d\n",
           result.items_decoded, result.images_decoded,
           result.floor_images_decoded);

    if (result.items_decoded != 243) {
        printf("  FAIL expected 243 items, got %d\n", result.items_decoded);
        free(data);
        return 1;
    }
    if (result.images_decoded != 223) {
        printf("  FAIL expected 223 images, got %d\n", result.images_decoded);
        free(data);
        return 1;
    }
    if (result.image_hashes[0] != 0) {
        printf("  FAIL unused FF00 association produced a pixel hash\n");
        free(data);
        return 1;
    }
    if (result.floor_images_decoded != 109) {
        printf("  FAIL expected 109 floor images, got %d\n",
               result.floor_images_decoded);
        free(data);
        return 1;
    }

    for (i = 0; i < result.floor_images_decoded; ++i) {
        if (result.floor_descs[i].palette_offset == 0U) {
            reused_palette_index = i;
            break;
        }
    }
    if (reused_palette_index < 0 ||
        nexus_v1_item_ibs_render_floor_image(
            data, size, reused_palette_index, &result, floor_rgba,
            (int)(sizeof(floor_rgba) / sizeof(floor_rgba[0]))) != 1 ||
        fnv1a_rgba(floor_rgba,
                   result.floor_descs[reused_palette_index].width *
                       result.floor_descs[reused_palette_index].height) !=
            result.floor_image_hashes[reused_palette_index]) {
        fprintf(stderr,
                "  FAIL floor image %d did not resolve DMWeb palette reuse\n",
                reused_palette_index);
        free(data);
        return 1;
    }

    unique_img = 0;
    memset(seen, 0, sizeof(seen));
    for (i = 0; i < result.images_decoded; ++i) {
        int j, dup = 0;
        for (j = 0; j < i; ++j) {
            if (result.image_hashes[i] == result.image_hashes[j]) { dup = 1; break; }
        }
        if (!dup) unique_img++;
    }
    printf("  inventory images: %d unique of %d (hash)\n",
           unique_img, result.images_decoded);

    unique_floor = 0;
    for (i = 0; i < result.floor_images_decoded; ++i) {
        int j, dup = 0;
        if (result.floor_image_hashes[i] == 0) continue;
        for (j = 0; j < i; ++j) {
            if (result.floor_image_hashes[i] == result.floor_image_hashes[j]) {
                dup = 1; break;
            }
        }
        if (!dup) unique_floor++;
    }
    printf("  floor images: %d unique (with hash)\n", unique_floor);

    for (i = 0; i < 5 && i < result.items_decoded; ++i) {
        const Nexus_V1_ItemIbsDecl *d = &result.items[i];
        printf("  item[%d]: cat=%d weight=%d inv_img=%d floor_img=%d name=%d\n",
               i, d->category, d->weight,
               d->inventory_image_index, d->floor_image_index,
               d->name_string);
    }

    printf("  PASS real_decode\n");
    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 ITEM.IBS Decoder ===\n");
    fail += test_synthetic();
    fail += test_real_decode();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
