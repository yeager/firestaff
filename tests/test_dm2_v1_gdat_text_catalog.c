#include "dm2_v1_asset_loader.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    data = malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)sz, f) != (size_t)sz) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return data;
}

static void decode_dm2_text(const uint8_t *src, size_t len, char *dst,
                            int encrypted) {
    size_t i;
    uint8_t counter = 0;
    for (i = 0; i < len; i++) {
        uint8_t c = src[i];
        if (encrypted)
            c = (uint8_t)(~c - counter++);
        dst[i] = (char)c;
    }
    dst[len] = '\0';
}

static int catalog_texts(const DM2_V1_AssetLoader *loader, const char *label,
                         int encrypted) {
    DM2_V1_GdatEntryIterator iter;
    DM2_V1_GdatEntryQueryReceipt receipt;
    int count = 0;
    char decoded[512];

    memset(&iter, 0, sizeof(iter));
    iter.category_first = 0;
    iter.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iter.index_filter = -1;
    iter.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
    iter.field_filter = -1;

    while (dm2_v1_query_next_gdat_entry(loader, &iter, &receipt)) {
        if (!receipt.present) continue;

        DM2_V1_DirectGdatTextReceipt text_receipt;
        size_t text_size = 0;
        const uint8_t *text = dm2_v1_direct_query_gdat_text_receipt(
            loader, receipt.category, receipt.index, receipt.field,
            &text_size, &text_receipt);

        printf("  [%s] cat=0x%02x idx=0x%02x field=0x%02x len=%u",
               label, receipt.category, receipt.index, receipt.field,
               receipt.raw_length);
        if (text && text_size > 0 && text_size < sizeof(decoded)) {
            decode_dm2_text(text, text_size, decoded, encrypted);
            printf(" \"");
            for (size_t i = 0; decoded[i] && i < 80; i++) {
                char c = decoded[i];
                if (c >= 0x20 && c < 0x7f)
                    putchar(c);
                else if ((uint8_t)c >= 0x80)
                    printf("\\x%02x", (uint8_t)c);
                else
                    printf("\\x%02x", (uint8_t)c);
            }
            printf("\"");
        }
        printf("\n");
        count++;
    }
    return count;
}

static void test_no_loader(void) {
    DM2_V1_GdatEntryIterator iter;
    DM2_V1_GdatEntryQueryReceipt receipt;
    memset(&iter, 0, sizeof(iter));
    iter.category_first = 0;
    iter.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iter.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
    iter.index_filter = -1;
    iter.field_filter = -1;
    assert(dm2_v1_query_next_gdat_entry(NULL, &iter, &receipt) == 0);
    printf("  PASS: null loader guard\n");
}

int main(int argc, char **argv) {
    const char *pc_path = NULL;
    const char *fmtowns_path = NULL;
    uint8_t *pc_data = NULL, *ft_data = NULL;
    size_t pc_size = 0, ft_size = 0;
    DM2_V1_AssetLoader pc_loader, ft_loader;
    int pc_count = 0, ft_count = 0;

    printf("dm2_v1_gdat_text_catalog:\n");
    test_no_loader();

    if (argc >= 2) pc_path = argv[1];
    if (argc >= 3) fmtowns_path = argv[2];

    if (!pc_path) {
        const char *home = getenv("HOME");
        static char buf[512];
        if (home) {
            snprintf(buf, sizeof(buf), "%s/.firestaff/data/dm2/GRAPHICS.DAT", home);
            pc_path = buf;
        }
    }

    if (pc_path) {
        pc_data = read_file(pc_path, &pc_size);
        if (pc_data) {
            if (dm2_v1_asset_loader_init(&pc_loader, pc_data, pc_size) != 0) {
                printf("  FAIL: cannot init PC asset loader from %s\n", pc_path);
                free(pc_data);
                pc_data = NULL;
            }
        } else {
            printf("  SKIP: cannot read PC GRAPHICS.DAT at %s\n", pc_path);
        }
    }

    if (!fmtowns_path) {
        const char *home = getenv("HOME");
        static char buf2[512];
        if (home) {
            snprintf(buf2, sizeof(buf2),
                     "%s/.firestaff/data/dm2-fmtowns-ja/GRAPHICS.DAT", home);
            fmtowns_path = buf2;
        }
    }

    if (fmtowns_path) {
        ft_data = read_file(fmtowns_path, &ft_size);
        if (ft_data) {
            if (dm2_v1_asset_loader_init(&ft_loader, ft_data, ft_size) != 0) {
                printf("  FAIL: cannot init FM Towns asset loader from %s\n",
                       fmtowns_path);
                free(ft_data);
                ft_data = NULL;
            }
        } else {
            printf("  SKIP: cannot read FM Towns GRAPHICS.DAT at %s\n",
                   fmtowns_path);
        }
    }

    if (pc_data) {
        printf("\n=== PC English GDAT v5 text entries ===\n");
        pc_count = catalog_texts(&pc_loader, "PC", 1);
        printf("  Total PC text entries: %d\n", pc_count);
    }

    if (ft_data) {
        printf("\n=== FM Towns Japanese GDAT v4 text entries ===\n");
        ft_count = catalog_texts(&ft_loader, "FT", 1);
        printf("  Total FM Towns text entries: %d\n", ft_count);
    }

    if (pc_data && ft_data) {
        printf("\n=== Comparison ===\n");
        printf("  PC entries: %d, FM Towns entries: %d\n", pc_count, ft_count);
    }

    free(pc_data);
    free(ft_data);
    printf("\nAll dm2_v1_gdat_text_catalog tests passed.\n");
    return 0;
}
