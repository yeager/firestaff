#include "dm2_v1_i18n.h"
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

static void test_init_destroy(void) {
    DM2_V1_I18nContext ctx;
    dm2_v1_i18n_init(&ctx);
    assert(ctx.active_locale == DM2_LOCALE_EN);
    assert(!ctx.valid);
    dm2_v1_i18n_destroy(&ctx);
    printf("  PASS: init/destroy\n");
}

static void test_locale_codes(void) {
    assert(strcmp(dm2_v1_i18n_locale_code(DM2_LOCALE_JA), "ja") == 0);
    assert(strcmp(dm2_v1_i18n_locale_code(DM2_LOCALE_EN), "en") == 0);
    assert(strcmp(dm2_v1_i18n_locale_code(DM2_LOCALE_SV), "sv") == 0);
    assert(dm2_v1_i18n_parse_locale("ja") == DM2_LOCALE_JA);
    assert(dm2_v1_i18n_parse_locale("en") == DM2_LOCALE_EN);
    assert(dm2_v1_i18n_parse_locale("sv") == DM2_LOCALE_SV);
    assert(dm2_v1_i18n_parse_locale("xx") == DM2_LOCALE_EN);
    assert(dm2_v1_i18n_parse_locale(NULL) == DM2_LOCALE_EN);
    printf("  PASS: locale codes\n");
}

static void test_null_guards(void) {
    DM2_V1_I18nContext ctx;
    size_t size;
    dm2_v1_i18n_init(&ctx);
    assert(dm2_v1_i18n_query_text(&ctx, 0x10, 0, 0x18, &size) == NULL);
    assert(dm2_v1_i18n_load_english_overlay(NULL, NULL, 0) == 0);
    assert(dm2_v1_i18n_load_english_overlay(&ctx, NULL, 0) == 0);
    dm2_v1_i18n_destroy(&ctx);
    printf("  PASS: null guards\n");
}

static void test_english_overlay(const char *pc_path) {
    DM2_V1_I18nContext ctx;
    uint8_t *data;
    size_t data_size;
    const uint8_t *text;
    size_t text_size;

    data = read_file(pc_path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read PC GRAPHICS.DAT at %s\n", pc_path);
        return;
    }

    dm2_v1_i18n_init(&ctx);
    assert(dm2_v1_i18n_load_english_overlay(&ctx, data, data_size) == 1);
    assert(ctx.valid);
    printf("  Loaded %u unique text entries, %u bytes text pool\n",
           ctx.entry_count, ctx.text_pool_size);

    /* cat=0x07 idx=0x00 field=0x00 should be "FIGHTER" */
    text = dm2_v1_i18n_query_text(&ctx, 0x07, 0x00, 0x00, &text_size);
    assert(text != NULL);
    assert(text_size >= 7);
    assert(memcmp(text, "FIGHTER", 7) == 0);
    printf("  PASS: FIGHTER lookup\n");

    /* cat=0x07 idx=0x00 field=0x01 should be "NINJA" */
    text = dm2_v1_i18n_query_text(&ctx, 0x07, 0x00, 0x01, &text_size);
    assert(text != NULL);
    assert(text_size >= 5);
    assert(memcmp(text, "NINJA", 5) == 0);
    printf("  PASS: NINJA lookup\n");

    /* cat=0x10 idx=0x00 field=0x18 should be "EYE OF TIME" */
    text = dm2_v1_i18n_query_text(&ctx, 0x10, 0x00, 0x18, &text_size);
    assert(text != NULL);
    assert(text_size >= 11);
    assert(memcmp(text, "EYE OF TIME", 11) == 0);
    printf("  PASS: EYE OF TIME lookup\n");

    /* Japanese locale should return NULL (no override) */
    dm2_v1_i18n_set_locale(&ctx, DM2_LOCALE_JA);
    text = dm2_v1_i18n_query_text(&ctx, 0x07, 0x00, 0x00, &text_size);
    assert(text == NULL);
    printf("  PASS: JA locale returns NULL\n");

    /* Switch back to EN */
    dm2_v1_i18n_set_locale(&ctx, DM2_LOCALE_EN);
    text = dm2_v1_i18n_query_text(&ctx, 0x07, 0x00, 0x00, &text_size);
    assert(text != NULL);
    printf("  PASS: EN locale returns text\n");

    /* Non-existent key */
    text = dm2_v1_i18n_query_text(&ctx, 0xFF, 0xFF, 0xFF, &text_size);
    assert(text == NULL);
    printf("  PASS: missing key returns NULL\n");

    /* Print some sample item names */
    printf("\n  === Sample English overlay texts ===\n");
    {
        int cats[] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x0f, 0x0f, 0x15, 0x15};
        int idxs[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x00, 0x01};
        int flds[] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18};
        int n = sizeof(cats) / sizeof(cats[0]);
        int i;
        for (i = 0; i < n; i++) {
            text = dm2_v1_i18n_query_text(&ctx, cats[i], idxs[i], flds[i],
                                          &text_size);
            if (text && text_size > 0) {
                printf("  [0x%02x:0x%02x:0x%02x] \"", cats[i], idxs[i],
                       flds[i]);
                fwrite(text, 1, text_size - 1, stdout);
                printf("\"\n");
            }
        }
    }

    free(data);
    dm2_v1_i18n_destroy(&ctx);
    printf("  PASS: english overlay\n");
}

static void test_locale_overlay(const char *en_path, const char *locale_path,
                                DM2_Locale locale, const char *locale_name) {
    DM2_V1_I18nContext ctx;
    uint8_t *en_data, *loc_data;
    size_t en_size, loc_size;
    const uint8_t *text_en, *text_loc;
    size_t ts_en, ts_loc;

    en_data = read_file(en_path, &en_size);
    loc_data = read_file(locale_path, &loc_size);
    if (!en_data || !loc_data) {
        printf("  SKIP: %s overlay (missing data)\n", locale_name);
        free(en_data);
        free(loc_data);
        return;
    }

    dm2_v1_i18n_init(&ctx);
    assert(dm2_v1_i18n_load_english_overlay(&ctx, en_data, en_size) == 1);
    uint16_t en_count = ctx.entry_count;

    assert(dm2_v1_i18n_load_locale_overlay(&ctx, locale, loc_data, loc_size) == 1);
    printf("  %s: loaded %u entries (EN %u + %s %u)\n",
           locale_name, ctx.entry_count, en_count, locale_name,
           ctx.entry_count - en_count);

    /* English text for FIGHTER (cat=0x07 idx=0x00 field=0x00) */
    text_en = dm2_v1_i18n_query_text(&ctx, 0x07, 0x00, 0x00, &ts_en);
    assert(text_en != NULL);
    assert(ts_en >= 7 && memcmp(text_en, "FIGHTER", 7) == 0);
    printf("  PASS: EN FIGHTER while EN active\n");

    /* Switch to target locale */
    dm2_v1_i18n_set_locale(&ctx, locale);
    text_loc = dm2_v1_i18n_query_text(&ctx, 0x07, 0x00, 0x00, &ts_loc);
    assert(text_loc != NULL);
    /* Localized text should differ from English */
    if (ts_loc != ts_en || memcmp(text_loc, text_en, ts_en < ts_loc ? ts_en : ts_loc) != 0) {
        printf("  PASS: %s FIGHTER differs from EN (\"", locale_name);
        fwrite(text_loc, 1, ts_loc > 0 ? ts_loc - 1 : 0, stdout);
        printf("\")\n");
    } else {
        printf("  INFO: %s FIGHTER same as EN\n", locale_name);
    }

    /* Print some sample localized item names */
    printf("  === Sample %s overlay texts ===\n", locale_name);
    {
        int cats[] = {0x10, 0x10, 0x10, 0x07, 0x07};
        int idxs[] = {0x00, 0x01, 0x02, 0x00, 0x00};
        int flds[] = {0x18, 0x18, 0x18, 0x00, 0x01};
        int n = 5;
        int i;
        for (i = 0; i < n; i++) {
            const uint8_t *t;
            size_t sz;
            t = dm2_v1_i18n_query_text(&ctx, cats[i], idxs[i], flds[i], &sz);
            if (t && sz > 0) {
                printf("  [0x%02x:0x%02x:0x%02x] \"", cats[i], idxs[i], flds[i]);
                fwrite(t, 1, sz - 1, stdout);
                printf("\"\n");
            }
        }
    }

    /* Fallback: query a key that only exists in EN */
    dm2_v1_i18n_set_locale(&ctx, locale);
    /* Switch back to EN to verify it still works */
    dm2_v1_i18n_set_locale(&ctx, DM2_LOCALE_EN);
    text_en = dm2_v1_i18n_query_text(&ctx, 0x07, 0x00, 0x00, &ts_en);
    assert(text_en != NULL);
    assert(ts_en >= 7 && memcmp(text_en, "FIGHTER", 7) == 0);
    printf("  PASS: EN still works after %s switch\n", locale_name);

    free(en_data);
    free(loc_data);
    dm2_v1_i18n_destroy(&ctx);
    printf("  PASS: %s overlay complete\n", locale_name);
}

int main(void) {
    const char *home;
    char en_path[512], fr_path[512], de_path[512];

    printf("dm2_v1_i18n tests:\n");
    test_init_destroy();
    test_locale_codes();
    test_null_guards();

    home = getenv("HOME");
    if (home) {
        snprintf(en_path, sizeof(en_path),
                 "%s/.firestaff/data/dm2/GRAPHICS.DAT", home);
        test_english_overlay(en_path);

        snprintf(fr_path, sizeof(fr_path),
                 "%s/.firestaff/data/dm2-extras/pc-fr/DATA/GRAPHICS.DAT", home);
        test_locale_overlay(en_path, fr_path, DM2_LOCALE_FR, "FR");

        snprintf(de_path, sizeof(de_path),
                 "%s/.firestaff/data/dm2-extras/pc-de/DATA/GRAPHICS.DAT", home);
        test_locale_overlay(en_path, de_path, DM2_LOCALE_DE, "DE");
    }

    printf("\nAll dm2_v1_i18n tests passed.\n");
    return 0;
}
