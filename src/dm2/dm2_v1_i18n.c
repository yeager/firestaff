#include "dm2_v1_i18n.h"
#include "dm2_v1_asset_loader.h"
#include <stdlib.h>
#include <string.h>

static const char *locale_codes[DM2_LOCALE_COUNT] = {
    "ja", "en", "de", "fr", "es", "it", "pt", "nl", "sv", "da",
    "no", "fi", "pl", "cs", "hu", "ro", "hr", "sk", "ru", "zh"
};

void dm2_v1_i18n_init(DM2_V1_I18nContext *ctx) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->active_locale = DM2_LOCALE_EN;
}

void dm2_v1_i18n_destroy(DM2_V1_I18nContext *ctx) {
    if (!ctx) return;
    free(ctx->entries);
    free(ctx->text_pool);
    memset(ctx, 0, sizeof(*ctx));
}

void dm2_v1_i18n_set_locale(DM2_V1_I18nContext *ctx, DM2_Locale locale) {
    if (!ctx || locale < 0 || locale >= DM2_LOCALE_COUNT) return;
    ctx->active_locale = locale;
}

static int i18n_ensure_pool(DM2_V1_I18nContext *ctx, uint32_t needed) {
    uint32_t new_cap;
    uint8_t *new_pool;
    if (ctx->text_pool_size + needed <= ctx->text_pool_capacity)
        return 1;
    new_cap = ctx->text_pool_capacity ? ctx->text_pool_capacity * 2 : 8192;
    while (new_cap < ctx->text_pool_size + needed)
        new_cap *= 2;
    new_pool = realloc(ctx->text_pool, new_cap);
    if (!new_pool) return 0;
    ctx->text_pool = new_pool;
    ctx->text_pool_capacity = new_cap;
    return 1;
}

static void decode_text(const uint8_t *src, size_t len, uint8_t *dst) {
    size_t i;
    for (i = 0; i < len; i++)
        dst[i] = (uint8_t)((src[i] ^ 0xFF) - (uint8_t)i);
}

int dm2_v1_i18n_load_english_overlay(DM2_V1_I18nContext *ctx,
                                     const uint8_t *pc_gdat_data,
                                     size_t pc_gdat_size) {
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntryIterator iter;
    DM2_V1_GdatEntryQueryReceipt receipt;
    uint16_t flag_word = 0;
    int encrypted = 0;
    uint16_t count = 0;

    if (!ctx || !pc_gdat_data || pc_gdat_size == 0) return 0;

    if (dm2_v1_asset_loader_init(&loader, pc_gdat_data, pc_gdat_size) != 0)
        return 0;

    if (dm2_v1_asset_load_word_value(&loader, 0, 0, 0, &flag_word))
        encrypted = (flag_word & 8u) != 0;

    /* Pass 1: count unique text entries */
    memset(&iter, 0, sizeof(iter));
    iter.category_first = 0;
    iter.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iter.index_filter = -1;
    iter.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
    iter.field_filter = -1;

    while (dm2_v1_query_next_gdat_entry(&loader, &iter, &receipt)) {
        if (receipt.present) count++;
    }

    if (count == 0) return 0;

    /* GDAT v5 may have duplicate entries (difficulty levels); deduplicate
     * by (category, index, field) key — keep only the first occurrence. */
    {
        uint16_t alloc_count = count;
        uint16_t unique = 0;
        uint16_t j;
        DM2_V1_I18nTextEntry *entries;
        uint32_t pool_estimate = 0;

        entries = calloc(alloc_count, sizeof(DM2_V1_I18nTextEntry));
        if (!entries) return 0;

        memset(&iter, 0, sizeof(iter));
        iter.category_first = 0;
        iter.category_last = DM2_GDAT_CATEGORY_LIMIT;
        iter.index_filter = -1;
        iter.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
        iter.field_filter = -1;

        while (dm2_v1_query_next_gdat_entry(&loader, &iter, &receipt)) {
            int dup = 0;
            if (!receipt.present) continue;

            for (j = 0; j < unique; j++) {
                if (entries[j].category == receipt.category &&
                    entries[j].index == receipt.index &&
                    entries[j].field == receipt.field) {
                    dup = 1;
                    break;
                }
            }
            if (dup) continue;
            if (unique >= alloc_count) break;

            entries[unique].category = receipt.category;
            entries[unique].index = receipt.index;
            entries[unique].field = receipt.field;
            entries[unique].locale = (uint8_t)DM2_LOCALE_EN;
            entries[unique].text_length = (uint16_t)receipt.raw_length;
            pool_estimate += receipt.raw_length;
            unique++;
        }

        ctx->entries = entries;
        ctx->entry_count = unique;

        if (!i18n_ensure_pool(ctx, pool_estimate)) {
            free(entries);
            ctx->entries = NULL;
            ctx->entry_count = 0;
            return 0;
        }

        /* Pass 2: extract and decode text data */
        for (j = 0; j < unique; j++) {
            DM2_V1_DirectGdatTextReceipt text_receipt;
            size_t text_size = 0;
            const uint8_t *raw = dm2_v1_direct_query_gdat_text_receipt(
                &loader, entries[j].category, entries[j].index,
                entries[j].field, &text_size, &text_receipt);

            entries[j].text_offset = ctx->text_pool_size;
            entries[j].text_length = (uint16_t)text_size;

            if (raw && text_size > 0) {
                if (!i18n_ensure_pool(ctx, (uint32_t)text_size)) break;
                if (encrypted)
                    decode_text(raw, text_size,
                                ctx->text_pool + ctx->text_pool_size);
                else
                    memcpy(ctx->text_pool + ctx->text_pool_size, raw, text_size);
                ctx->text_pool_size += (uint32_t)text_size;
            }
        }
    }

    ctx->valid = 1;
    ctx->text_encrypted = 0;
    return 1;
}

int dm2_v1_i18n_load_locale_overlay(DM2_V1_I18nContext *ctx,
                                    DM2_Locale locale,
                                    const uint8_t *gdat_data,
                                    size_t gdat_size) {
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntryIterator iter;
    DM2_V1_GdatEntryQueryReceipt receipt;
    uint16_t flag_word = 0;
    int encrypted = 0;
    uint16_t count = 0;
    uint16_t new_unique = 0;
    uint16_t j;
    DM2_V1_I18nTextEntry *new_entries;
    uint16_t old_count;

    if (!ctx || !gdat_data || gdat_size == 0) return 0;
    if (locale < 0 || locale >= DM2_LOCALE_COUNT) return 0;

    if (dm2_v1_asset_loader_init(&loader, gdat_data, gdat_size) != 0)
        return 0;

    if (dm2_v1_asset_load_word_value(&loader, 0, 0, 0, &flag_word))
        encrypted = (flag_word & 8u) != 0;

    memset(&iter, 0, sizeof(iter));
    iter.category_first = 0;
    iter.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iter.index_filter = -1;
    iter.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
    iter.field_filter = -1;

    while (dm2_v1_query_next_gdat_entry(&loader, &iter, &receipt)) {
        if (receipt.present) count++;
    }
    if (count == 0) return 0;

    old_count = ctx->entry_count;
    new_entries = realloc(ctx->entries,
        (size_t)(old_count + count) * sizeof(DM2_V1_I18nTextEntry));
    if (!new_entries) return 0;
    ctx->entries = new_entries;

    memset(&iter, 0, sizeof(iter));
    iter.category_first = 0;
    iter.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iter.index_filter = -1;
    iter.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
    iter.field_filter = -1;

    while (dm2_v1_query_next_gdat_entry(&loader, &iter, &receipt)) {
        int dup = 0;
        if (!receipt.present) continue;

        for (j = old_count; j < old_count + new_unique; j++) {
            if (ctx->entries[j].category == receipt.category &&
                ctx->entries[j].index == receipt.index &&
                ctx->entries[j].field == receipt.field) {
                dup = 1;
                break;
            }
        }
        if (dup) continue;
        if (old_count + new_unique >= old_count + count) break;

        ctx->entries[old_count + new_unique].category = receipt.category;
        ctx->entries[old_count + new_unique].index = receipt.index;
        ctx->entries[old_count + new_unique].field = receipt.field;
        ctx->entries[old_count + new_unique].locale = (uint8_t)locale;
        ctx->entries[old_count + new_unique].text_length = (uint16_t)receipt.raw_length;
        new_unique++;
    }

    /* Extract text data */
    for (j = 0; j < new_unique; j++) {
        DM2_V1_DirectGdatTextReceipt text_receipt;
        size_t text_size = 0;
        uint16_t idx = old_count + j;
        const uint8_t *raw = dm2_v1_direct_query_gdat_text_receipt(
            &loader, ctx->entries[idx].category, ctx->entries[idx].index,
            ctx->entries[idx].field, &text_size, &text_receipt);

        ctx->entries[idx].text_offset = ctx->text_pool_size;
        ctx->entries[idx].text_length = (uint16_t)text_size;

        if (raw && text_size > 0) {
            if (!i18n_ensure_pool(ctx, (uint32_t)text_size)) break;
            if (encrypted)
                decode_text(raw, text_size, ctx->text_pool + ctx->text_pool_size);
            else
                memcpy(ctx->text_pool + ctx->text_pool_size, raw, text_size);
            ctx->text_pool_size += (uint32_t)text_size;
        }
    }

    ctx->entry_count = old_count + new_unique;
    ctx->valid = 1;
    return 1;
}

const uint8_t *dm2_v1_i18n_query_text(const DM2_V1_I18nContext *ctx,
                                      int category, int index, int field,
                                      size_t *out_size) {
    uint16_t i;

    if (out_size) *out_size = 0;
    if (!ctx || !ctx->valid || !ctx->entries || !ctx->text_pool) return NULL;
    if (ctx->active_locale == DM2_LOCALE_JA) return NULL;

    for (i = 0; i < ctx->entry_count; i++) {
        if (ctx->entries[i].locale == (uint8_t)ctx->active_locale &&
            ctx->entries[i].category == (uint8_t)category &&
            ctx->entries[i].index == (uint8_t)index &&
            ctx->entries[i].field == (uint8_t)field) {
            if (out_size) *out_size = ctx->entries[i].text_length;
            return ctx->text_pool + ctx->entries[i].text_offset;
        }
    }
    /* Fall back to English if no match for active locale */
    if (ctx->active_locale != DM2_LOCALE_EN) {
        for (i = 0; i < ctx->entry_count; i++) {
            if (ctx->entries[i].locale == (uint8_t)DM2_LOCALE_EN &&
                ctx->entries[i].category == (uint8_t)category &&
                ctx->entries[i].index == (uint8_t)index &&
                ctx->entries[i].field == (uint8_t)field) {
                if (out_size) *out_size = ctx->entries[i].text_length;
                return ctx->text_pool + ctx->entries[i].text_offset;
            }
        }
    }
    return NULL;
}

const char *dm2_v1_i18n_locale_code(DM2_Locale locale) {
    if (locale < 0 || locale >= DM2_LOCALE_COUNT) return "en";
    return locale_codes[locale];
}

DM2_Locale dm2_v1_i18n_parse_locale(const char *code) {
    int i;
    if (!code) return DM2_LOCALE_EN;
    for (i = 0; i < DM2_LOCALE_COUNT; i++) {
        if (strcmp(code, locale_codes[i]) == 0)
            return (DM2_Locale)i;
    }
    return DM2_LOCALE_EN;
}
