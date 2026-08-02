#ifndef DM2_V1_I18N_H
#define DM2_V1_I18N_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 i18n/l10n: text overlay for FM Towns multi-language support.
 *
 * The FM Towns GDAT v4 contains Japanese text (Shift-JIS).  The PC GDAT v5
 * contains English text.  This module provides a text overlay that maps
 * GDAT text keys (category, index, field) to localized strings, allowing
 * the FM Towns runtime to display text in English or other languages.
 *
 * Architecture:
 *   - Base layer: FM Towns GDAT v4 (Japanese, native platform assets)
 *   - Overlay layer: PC GDAT v5 text entries (English, extracted at boot)
 *   - Translation tables: per-locale overrides (future, for 18 additional languages)
 *
 * The overlay intercepts text queries by (category, index, field) key.
 * When a match exists, the overlay text is returned instead of the base.
 * When no match exists, the base GDAT text is used (Japanese fallback).
 *
 * Source: skproject c_gfx_str.cpp DM2_QUERY_GDAT_TEXT (lines 566-588).
 * Text cipher: ~byte - counter (when GDAT flag word bit 3 is set). */

typedef enum {
    DM2_LOCALE_JA = 0,
    DM2_LOCALE_EN,
    DM2_LOCALE_DE,
    DM2_LOCALE_FR,
    DM2_LOCALE_ES,
    DM2_LOCALE_IT,
    DM2_LOCALE_PT,
    DM2_LOCALE_NL,
    DM2_LOCALE_SV,
    DM2_LOCALE_DA,
    DM2_LOCALE_NO,
    DM2_LOCALE_FI,
    DM2_LOCALE_PL,
    DM2_LOCALE_CS,
    DM2_LOCALE_HU,
    DM2_LOCALE_RO,
    DM2_LOCALE_HR,
    DM2_LOCALE_SK,
    DM2_LOCALE_RU,
    DM2_LOCALE_ZH,
    DM2_LOCALE_COUNT
} DM2_Locale;

typedef struct {
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t locale;
    uint16_t text_length;
    uint32_t text_offset;
} DM2_V1_I18nTextEntry;

typedef struct {
    int valid;
    DM2_Locale active_locale;
    DM2_V1_I18nTextEntry *entries;
    uint16_t entry_count;
    uint8_t *text_pool;
    uint32_t text_pool_size;
    uint32_t text_pool_capacity;
    int text_encrypted;
} DM2_V1_I18nContext;

/* Initialize i18n context.  Does not load any overlay text. */
void dm2_v1_i18n_init(DM2_V1_I18nContext *ctx);

/* Free all memory owned by the i18n context. */
void dm2_v1_i18n_destroy(DM2_V1_I18nContext *ctx);

/* Set the active locale. */
void dm2_v1_i18n_set_locale(DM2_V1_I18nContext *ctx, DM2_Locale locale);

/* Load English text overlay from a PC GDAT v5 GRAPHICS.DAT file.
 * Extracts all text entries and stores decoded (plaintext) copies.
 * The data pointer must remain valid only for the duration of this call.
 * Returns 1 on success, 0 on failure. */
int dm2_v1_i18n_load_english_overlay(DM2_V1_I18nContext *ctx,
                                     const uint8_t *pc_gdat_data,
                                     size_t pc_gdat_size);

/* Load a locale text overlay from a GDAT v5 GRAPHICS.DAT file.
 * Multiple locales can be loaded; entries are appended and tagged
 * with their locale.  Query uses the active locale to pick the right entry.
 * Returns 1 on success, 0 on failure. */
int dm2_v1_i18n_load_locale_overlay(DM2_V1_I18nContext *ctx,
                                    DM2_Locale locale,
                                    const uint8_t *gdat_data,
                                    size_t gdat_size);

/* Query text override for the given GDAT text key.
 * If an override exists for the active locale, returns a pointer to the
 * decoded (plaintext) text and sets *out_size.  The returned pointer is
 * owned by the i18n context and valid until destroy.
 * Returns NULL if no override exists (caller should fall back to base GDAT). */
const uint8_t *dm2_v1_i18n_query_text(const DM2_V1_I18nContext *ctx,
                                      int category, int index, int field,
                                      size_t *out_size);

/* Get the ISO 639-1 code for a locale. */
const char *dm2_v1_i18n_locale_code(DM2_Locale locale);

/* Parse an ISO 639-1 code to a locale enum.  Returns DM2_LOCALE_EN on
 * unrecognized input. */
DM2_Locale dm2_v1_i18n_parse_locale(const char *code);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_I18N_H */
