#include "nexus_v1_raw_bin.h"
#include "nexus_v1_iso_reader.h"
#include "nexus_v1_palette.h"
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

static const char *type_name(int t) {
    switch (t) {
        case 1: return "VDP_DATA";
        case 2: return "SH2_CODE";
        case 3: return "TILEMAP";
        default: return "UNKNOWN";
    }
}

static int test_file(const char *name) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");
    char root[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_RawBinDecodeResult r;

    if (data_dir && data_dir[0]) {
        snprintf(root, sizeof(root), "%s", data_dir);
    } else if (home && home[0]) {
        snprintf(root, sizeof(root), "%s/.firestaff/data/nexus", home);
    } else {
        printf("  SKIP %s (Nexus data root is unset)\n", name);
        return 0;
    }
    data = load_retail_file(root, name, &size);
    if (!data) { printf("  SKIP %s (not found)\n", name); return 0; }

    if (!nexus_v1_raw_bin_decode(data, size, &r)) {
        printf("  FAIL %s decode\n", name);
        free(data);
        return 1;
    }

    if (r.content_type != NEXUS_RAW_TYPE_UNKNOWN) {
        printf("  FAIL %s inferred unproven content type=%s\n",
               name, type_name(r.content_type));
        free(data);
        return 1;
    }
    printf("  PASS %s: type=UNKNOWN (receipt-only) size=%d nz=%d prs3=%d hash=0x%08X\n",
           name, r.file_size,
           r.non_zero_bytes, r.prs3_offset, r.data_hash);
    free(data);
    return 0;
}

static int test_stone_pp(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");
    char root[512];
    uint8_t *data;
    uint8_t packed[NEXUS_STONE_PP_PACKED_BYTES];
    uint16_t palette[NEXUS_STONE_PP_PALETTE_COUNT];
    int size = 0;
    Nexus_StonePpReceipt receipt;
    Nexus_StonePpRecordReceipt record;

    if (data_dir && data_dir[0]) {
        snprintf(root, sizeof(root), "%s", data_dir);
    } else if (home && home[0]) {
        snprintf(root, sizeof(root), "%s/.firestaff/data/nexus", home);
    } else {
        printf("  SKIP STONE pp (Nexus data root is unset)\n");
        return 0;
    }
    data = load_retail_file(root, "STONE.BIN", &size);
    if (!data) { printf("  SKIP STONE pp (not found)\n"); return 0; }
    if (!nexus_palette_stone_pp_receipt(data, size, &receipt) ||
        !receipt.valid || receipt.record_count != 8 || receipt.width != 32 ||
        receipt.height != 32 || receipt.palette_count != 16 ||
        receipt.packed_pixel_bytes != 512 ||
        !nexus_palette_decode_stone_pp_record(data, size, 0, packed,
                                              (int)sizeof(packed), palette,
                                              (int)(sizeof(palette) /
                                                    sizeof(palette[0])),
                                              &record) ||
        !record.valid || record.record_offset != 0 || palette[0] != 0x8000U ||
        packed[0] != 0x11U || record.source_record_fnv1a32 == 0U) {
        printf("  FAIL STONE pp receipt/decode\n");
        free(data);
        return 1;
    }
    printf("  PASS STONE pp: records=%d %dx%d palette=%d packed=%d hash=0x%08X\n",
           receipt.record_count, receipt.width, receipt.height,
           receipt.palette_count, receipt.packed_pixel_bytes,
           receipt.source_bytes_fnv1a32);
    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 Raw Binary Decoder ===\n");

    fail += test_file("DM.BIN");
    fail += test_file("NBG3.BIN");
    fail += test_file("STONE.BIN");
    fail += test_file("DEATH.BIN");
    fail += test_file("SWTCHR.BIN");
    fail += test_file("TM.BIN");
    fail += test_file("SDDRVS.TSK");
    fail += test_stone_pp();

    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
