#include "nexus_v1_title_cg.h"
#include "nexus_v1_res.h"
#include "nexus_v1_font012.h"
#include "nexus_v1_iso_reader.h"
#include <limits.h>
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

static uint8_t *load_retail_file(const char *root, const char *name,
                                 int *out_size) {
    static const char *const cue_names[] = {
        "Dungeon Master Nexus (Japan).cue",
        "Dungeon Master Nexus (English).cue",
        NULL
    };
    Nexus_ISOReader iso;
    const Nexus_ISOFile *member;
    char path[1024];
    int index;
    uint8_t *data;

    if (!root || !name || !out_size) return NULL;
    snprintf(path, sizeof(path), "%s/%s", root, name);
    data = load_file(path, out_size);
    if (data) return data;
    memset(&iso, 0, sizeof(iso));
    if (strlen(root) >= 4U && strcmp(root + strlen(root) - 4U, ".cue") == 0) {
        if (nexus_iso_open_cue(&iso, root) <= 0) return NULL;
    } else {
        for (index = 0; cue_names[index]; ++index) {
            snprintf(path, sizeof(path), "%s/%s", root, cue_names[index]);
            if (nexus_iso_open_cue(&iso, path) > 0) break;
        }
        if (!iso.valid) return NULL;
    }
    member = nexus_iso_find(&iso, name);
    if (!member || member->size == 0U || member->size > (uint32_t)INT_MAX) {
        nexus_iso_close(&iso);
        return NULL;
    }
    data = (uint8_t *)malloc(member->size);
    if (!data || nexus_iso_read_file(&iso, member, data, (int)member->size) !=
                     (int)member->size) {
        free(data);
        data = NULL;
    } else {
        *out_size = (int)member->size;
    }
    nexus_iso_close(&iso);
    return data;
}

static const char *retail_root(char *out, size_t out_size) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");

    if (data_dir && data_dir[0]) {
        snprintf(out, out_size, "%s", data_dir);
        return out;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/nexus", home);
        return out;
    }
    return NULL;
}

static int test_title_cg(void) {
    char root[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_TitleCgDecodeResult r;

    if (!retail_root(root, sizeof(root))) {
        printf("  SKIP title_cg (Nexus data root is unset)\n");
        return 0;
    }
    data = load_retail_file(root, "TITLE.CG", &size);
    if (!data) { printf("  SKIP title_cg (no file)\n"); return 0; }

    if (!nexus_v1_title_cg_decode(data, size, &r)) {
        printf("  FAIL title_cg decode\n");
        free(data);
        return 1;
    }

    if (r.tile_count != NEXUS_TITLE_CG_TILE_COUNT) {
        printf("  FAIL tile_count=%d expected=%d\n",
               r.tile_count, NEXUS_TITLE_CG_TILE_COUNT);
        free(data);
        return 1;
    }

    printf("  PASS title_cg: tiles=%d hash=0x%08X\n",
           r.tile_count, r.tile_hash);
    free(data);
    return 0;
}

static int test_res_file(const char *name) {
    char root[512];
    uint8_t *data;
    int size = 0, i;
    Nexus_V1_ResDecodeResult r;

    if (!retail_root(root, sizeof(root))) {
        return 0;
    }
    data = load_retail_file(root, name, &size);
    if (!data) { printf("  SKIP %s (not found)\n", name); return 0; }

    if (!nexus_v1_res_decode(data, size, &r)) {
        printf("  FAIL %s: RES* decode failed\n", name);
        free(data);
        return 1;
    }

    printf("  PASS %-14s entries=%d size=%u\n", name, r.entry_count, r.file_size);
    for (i = 0; i < r.entry_count && i < 8; ++i) {
        printf("    [%d] %s#%u off=0x%X size=%u\n",
               i, r.entries[i].tag, r.entries[i].index,
               r.entries[i].offset, r.entries[i].size);
    }
    if (r.entry_count > 8) printf("    ... +%d more\n", r.entry_count - 8);

    {
        uint8_t *tampered = (uint8_t *)malloc((size_t)size);
        if (!tampered) {
            free(data);
            return 1;
        }
        memcpy(tampered, data, (size_t)size);
        tampered[7] ^= 1U; /* RES* declared size must equal the source. */
        if (nexus_v1_res_decode(tampered, size, &r)) {
            free(tampered);
            free(data);
            return 1;
        }
        free(tampered);
    }

    free(data);
    return 0;
}

static int test_font012_headers(void) {
    char root[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_ResDecodeResult res;
    Nexus_V1_Font012Receipt receipt;
    uint8_t glyph[12 * 12];
    const uint32_t indices[] = {0U, 1U, 2U};
    const uint32_t counts[] = {291U, 250U, 710U};
    const uint32_t widths[] = {6U, 12U, 12U};
    const uint32_t offsets[] = {0xC0U, 0x1C2CU, 0x3F78U};
    int i;

    if (!retail_root(root, sizeof(root))) {
        return 0;
    }
    data = load_retail_file(root, "RLOWFIX.BIN", &size);
    if (!data) { printf("  SKIP FONT012 (no file)\n"); return 0; }
    if (!nexus_v1_res_decode(data, size, &res)) { free(data); return 1; }
    for (i = 0; i < 3; ++i) {
        const Nexus_V1_ResEntry *entry =
            nexus_v1_res_find(&res, "FONT", (int)indices[i]);
        if (!entry || entry->offset != offsets[i] ||
            !nexus_v1_font012_parse(data + entry->offset, entry->size,
                                     indices[i], &receipt) || !receipt.valid ||
            receipt.character_count != counts[i] ||
            receipt.character_width != widths[i] ||
            receipt.character_height != 12U) {
            free(data);
            return 1;
        }
        if (!nexus_v1_font012_decode_glyph(data + entry->offset, entry->size,
                                           indices[i], 0U, glyph,
                                           sizeof(glyph))) {
            free(data);
            return 1;
        }
    }
    free(data);
    puts("  PASS FONT012 headers: FONT#0/#1/#2 retail geometry admitted");
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 TITLE.CG & RES* Decoder ===\n");
    fail += test_title_cg();
    fail += test_res_file("TITLE.BIN");
    fail += test_res_file("RLOWFIX.BIN");
    fail += test_res_file("RHIFIX.BIN");
    fail += test_res_file("POTEFT.BIN");
    fail += test_font012_headers();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
