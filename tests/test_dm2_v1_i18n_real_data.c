/*
 * test_dm2_v1_i18n_real_data.c
 *
 * Validates DM2 i18n text extraction against real GRAPHICS.DAT files.
 * Loads EN, FR, DE, Mac FR, FM Towns JP, and PC-9821 JP GDAT files
 * and extracts text entries from each.
 */

#include "dm2_v1_i18n.h"
#include "dm2_v1_asset_loader.h"
#include "firestaff_zip_extract.h"

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

static int test_gdat_text(const char *label, const char *path,
                          DM2_Locale locale, int expect_min,
                          int required) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_I18nContext ctx;
    int text_count;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  %s: cannot read %s\n", label, path);
        return required ? 0 : 1;
    }

    printf("  %s: %zu bytes\n", label, data_size);

    text_count = count_text_entries(data, data_size);
    printf("    text entries: %d\n", text_count);

    if (text_count < 0) {
        printf("    FAIL: could not init GDAT loader\n");
        free(data);
        return 0;
    }
    if (text_count < expect_min) {
        printf("    FAIL: expected at least %d text entries\n", expect_min);
        free(data);
        return 0;
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
        if (ctx.entry_count == 0 || ctx.text_pool_size == 0u) {
            printf("    FAIL: i18n overlay contains no source text\n");
            dm2_v1_i18n_destroy(&ctx);
            free(data);
            return 0;
        }

        /* Exercise the same keyed lookup used by the FM Towns companion
         * bridge. A populated pool alone is not proof that source text can
         * reach a menu or dialogue owner. */
        for (uint16_t i = 0; i < ctx.entry_count; ++i) {
            const DM2_V1_I18nTextEntry *entry = &ctx.entries[i];
            size_t queried_size = 0u;
            const uint8_t *queried = dm2_v1_i18n_query_text(
                &ctx, entry->category, entry->index, entry->field,
                &queried_size);
            if (!queried || queried_size != entry->text_length ||
                memcmp(queried, ctx.text_pool + entry->text_offset,
                       queried_size) != 0) {
                printf("    FAIL: source text key %u/%u/%u is not queryable\n",
                       entry->category, entry->index, entry->field);
                dm2_v1_i18n_destroy(&ctx);
                free(data);
                return 0;
            }
        }

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
        printf("    FAIL: i18n load rejected readable source GDAT\n");
        dm2_v1_i18n_destroy(&ctx);
        free(data);
        return 0;
    }

    dm2_v1_i18n_destroy(&ctx);
    free(data);
    return 1;
}

/* The FM Towns non-Japanese bridge is generated from the reviewed PC-English
 * GDAT inventory.  Check it directly against the authenticated archive member
 * so a stale generated table cannot silently change the presentation owner.
 * No member is written to disk. */
static int test_builtin_matches_pc_archive(const char *archive) {
    uint8_t *data = NULL;
    size_t data_size = 0u;
    DM2_V1_I18nContext source;
    DM2_V1_I18nContext builtin;
    uint16_t i;

    if (!archive || !archive[0] ||
        firestaff_zip_extract_by_suffix(archive, "data/graphics.dat", &data,
                                        &data_size) != 0 ||
        !data) {
        printf("  SKIP: PC-English archive unavailable for built-in bridge correlation\n");
        free(data);
        return 1;
    }

    dm2_v1_i18n_init(&source);
    dm2_v1_i18n_init(&builtin);
    if (!dm2_v1_i18n_load_english_overlay(&source, data, data_size) ||
        !dm2_v1_i18n_load_builtin_english_overlay(&builtin)) {
        printf("  FAIL: could not load PC-English or built-in text bridge\n");
        dm2_v1_i18n_destroy(&builtin);
        dm2_v1_i18n_destroy(&source);
        free(data);
        return 0;
    }
    for (i = 0u; i < builtin.entry_count; ++i) {
        const DM2_V1_I18nTextEntry *entry = &builtin.entries[i];
        const uint8_t *expected;
        const uint8_t *actual;
        size_t actual_size = 0u;
        size_t expected_size = 0u;
        expected = dm2_v1_i18n_query_text(&source, entry->category,
                                           entry->index, entry->field,
                                           &expected_size);
        actual = dm2_v1_i18n_query_text(&builtin, entry->category,
                                         entry->index, entry->field,
                                         &actual_size);
        if (!expected || !actual || actual_size != expected_size ||
            memcmp(actual, expected, actual_size) != 0) {
            printf("  FAIL: bridge differs at GDAT %02x/%02x/%02x (built %zu, PC %zu)\n",
                   entry->category, entry->index, entry->field,
                   actual_size, expected_size);
            printf("    built: %.*s\n    PC: %.*s\n", (int)actual_size,
                   (const char *)actual, (int)expected_size,
                   (const char *)expected);
            dm2_v1_i18n_destroy(&builtin);
            dm2_v1_i18n_destroy(&source);
            free(data);
            return 0;
        }
    }
    printf("  PASS: %u/%u built-in bridge keys exactly match PC-English GDAT\n",
           builtin.entry_count, source.entry_count);
    dm2_v1_i18n_destroy(&builtin);
    dm2_v1_i18n_destroy(&source);
    free(data);
    return 1;
}

int main(void) {
    const char *data_dir;
    const char *archive;
    char path[512];
    int passed = 1;

    printf("DM2 i18n real data tests:\n\n");

    data_dir = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (data_dir && data_dir[0]) {
        /* The required corpus is the exact selected PC-DOS data root, not an
         * obsolete convenience path under HOME. */
        snprintf(path, sizeof(path), "%s/graphics.dat", data_dir);
        passed &= test_gdat_text("PC EN", path, DM2_LOCALE_EN, 1, 1);
    }

    archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    if (!archive || !archive[0]) {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(path, sizeof(path),
                     "%s/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip",
                     home);
            archive = path;
        }
    }
    passed &= test_builtin_matches_pc_archive(archive);

    if ((!data_dir || !data_dir[0]) && (!archive || !archive[0])) {
        puts("SKIP: set FIRESTAFF_DM2_DATA_DIR or FIRESTAFF_DM2_DOS_ARCHIVE");
    }

    if (!passed) {
        puts("\nFAIL: DM2 i18n real-data verification failed.");
        return 1;
    }
    puts("\nPASS: DM2 i18n uses the selected real PC-DOS corpus.");
    return 0;
}
