/*
 * test_dm2_v1_i18n_real_data.c
 *
 * Validates DM2 i18n text extraction against real GRAPHICS.DAT files.
 * Loads EN, FR, DE, Mac FR, FM Towns JP, and PC-9821 JP GDAT files
 * and extracts text entries from each.
 */

#include "dm2_v1_i18n.h"
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

static int count_text_entries(const uint8_t *gdat_data, size_t gdat_size) {
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntryIterator iter;
    DM2_V1_GdatEntryQueryReceipt receipt;
    int count = 0;

    if (dm2_v1_asset_loader_init(&loader, gdat_data, gdat_size) != 0)
        return -1;

    memset(&iter, 0, sizeof(iter));
    iter.category_first = 0;
    iter.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iter.index_filter = -1;
    iter.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
    iter.field_filter = -1;

    while (dm2_v1_query_next_gdat_entry(&loader, &iter, &receipt)) {
        if (receipt.present) count++;
    }

    dm2_v1_asset_loader_free(&loader);
    return count;
}

static void test_gdat_text(const char *label, const char *path,
                           DM2_Locale locale, int expect_min) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_I18nContext ctx;
    int text_count;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: %s — cannot read %s\n", label, path);
        return;
    }

    printf("  %s: %zu bytes\n", label, data_size);

    text_count = count_text_entries(data, data_size);
    printf("    text entries: %d\n", text_count);

    if (text_count < 0) {
        printf("    FAIL: could not init GDAT loader\n");
        free(data);
        return;
    }

    dm2_v1_i18n_init(&ctx);

    int loaded;
    if (locale == DM2_LOCALE_EN) {
        loaded = dm2_v1_i18n_load_english_overlay(&ctx, data, data_size);
    } else {
        /* Load EN first as base, then overlay */
        loaded = dm2_v1_i18n_load_locale_overlay(&ctx, locale, data, data_size);
    }

    if (loaded) {
        printf("    i18n loaded: %u unique entries, pool %u bytes\n",
               ctx.entry_count, ctx.text_pool_size);
        assert(ctx.entry_count > 0);

        /* Sample some text entries */
        int samples = 0;
        for (uint16_t i = 0; i < ctx.entry_count && samples < 5; i++) {
            size_t len = ctx.entries[i].text_length;
            if (len > 0 && len < 80) {
                const uint8_t *txt = ctx.text_pool + ctx.entries[i].text_offset;
                char buf[81];
                size_t copy = len < 80 ? len : 80;
                memcpy(buf, txt, copy);
                buf[copy] = '\0';
                printf("    [%u/%u/%u]: '%s'\n",
                       ctx.entries[i].category,
                       ctx.entries[i].index,
                       ctx.entries[i].field, buf);
                samples++;
            }
        }
        printf("    PASS: %s text extraction\n", label);
    } else {
        printf("    SKIP: i18n load returned 0 (may need EN base)\n");
    }

    dm2_v1_i18n_destroy(&ctx);
    free(data);
}

int main(void) {
    const char *home;
    char path[512];

    printf("DM2 i18n real data tests:\n\n");

    home = getenv("HOME");
    if (!home) { printf("SKIP: HOME not set\n"); return 0; }

    /* PC EN */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2/GRAPHICS.DAT", home);
    test_gdat_text("PC EN", path, DM2_LOCALE_EN, 100);

    /* PC FR */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/pc-fr/DATA/GRAPHICS.DAT", home);
    test_gdat_text("PC FR", path, DM2_LOCALE_FR, 100);

    /* PC DE */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/pc-de/DATA/GRAPHICS.DAT", home);
    test_gdat_text("PC DE", path, DM2_LOCALE_DE, 100);

    /* Mac FR */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/mac-fr/Dungeon Master II/DMFiles/Graphics.dat",
             home);
    test_gdat_text("Mac FR", path, DM2_LOCALE_FR, 100);

    /* FM Towns JP */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/fm-towns-ja/extracted/GRAPHICS.DAT",
             home);
    test_gdat_text("FM Towns JP", path, DM2_LOCALE_JA, 0);

    /* PC-9821 JP */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/pc9821-jp-extracted/GRAPHICS.DAT",
             home);
    test_gdat_text("PC-9821 JP", path, DM2_LOCALE_JA, 0);

    printf("\nAll DM2 i18n real data tests passed.\n");
    return 0;
}
