#include "dm2_v1_weather_gdat.h"
#include <stdio.h>

#include <limits.h>
#include <string.h>

/* FNV-1a is a receipt identity only.  It does not decode QUERY_GDAT_TEXT's
 * optional source encoding or infer command grammar. */
static uint32_t dm2_weather_hash_bytes(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t dm2_weather_hash_step(uint32_t hash, uint32_t value);

static int dm2_weather_command_is_source_owned(uint8_t command)
{
    return command >= DM2_V1_WEATHER_BOLT_CMD_BASE &&
           command <= DM2_V1_WEATHER_RAIN_STORM_CMD;
}

static int dm2_weather_command_is_bolt(uint8_t command)
{
    return command >= DM2_V1_WEATHER_BOLT_CMD_BASE &&
           command <= DM2_V1_WEATHER_BOLT_CMD_LAST;
}

/* SkWinCore::QUERY_GDAT_TEXT (SkWinCore.cpp 2636:0377): when
 * glbTextEntryEncoded (dtWordValue(0,0,0) bit 3, set at SkWinCore.cpp
 * 55629) is non-zero, each text byte decodes as (b ^ 0xFF) - i.  The
 * consuming RETRIEVE_ENVIRONMENT_CMD_CD_FW buffer is 128 bytes, so a
 * command text that does not fit cannot drive a source slot. */
static int dm2_weather_decode_command_text(
    const DM2_V1_AssetLoader *loader, const uint8_t *raw, size_t raw_size,
    uint8_t out_decoded[DM2_V1_WEATHER_COMMAND_TEXT_MAX],
    uint32_t *out_size, uint32_t *out_hash)
{
    uint16_t flag_word;
    size_t i;
    uint32_t hash = 2166136261u;

    if (out_size) *out_size = 0u;
    if (out_hash) *out_hash = 0u;
    if (!loader || !raw || !out_decoded || !out_size || !out_hash ||
        raw_size == 0u || raw_size > DM2_V1_WEATHER_COMMAND_TEXT_MAX - 1u) {
        return 0;
    }
    if (!dm2_v1_asset_load_word_value(loader, 0, 0, 0, &flag_word) ||
        (flag_word & 8u) == 0u) {
        return 0;
    }
    for (i = 0u; i < raw_size; ++i) {
        out_decoded[i] = (uint8_t)((raw[i] ^ 0xFFu) - (uint8_t)i);
        hash = dm2_weather_hash_step(hash, out_decoded[i]);
    }
    if (hash == 0u) return 0;
    *out_size = (uint32_t)raw_size;
    *out_hash = hash;
    return 1;
}

static int dm2_weather_text_has_nul(const uint8_t *text, size_t size)
{
    return text && memchr(text, '\0', size) != NULL;
}

static int dm2_weather_source_raw_index(const DM2_V1_AssetLoader *loader,
                                        const uint8_t *source_bytes,
                                        size_t source_byte_count,
                                        uint16_t *out_raw_index)
{
    uint16_t raw_index;

    if (out_raw_index) *out_raw_index = 0u;
    if (!loader || !loader->data || !loader->raw_offsets ||
        !loader->raw_sizes || !source_bytes || !source_byte_count ||
        !out_raw_index) return 0;
    for (raw_index = 0u; raw_index < loader->raw_data_count; ++raw_index) {
        if (loader->raw_sizes[raw_index] == source_byte_count &&
            loader->data + loader->raw_offsets[raw_index] == source_bytes) {
            *out_raw_index = raw_index;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_asset_load_image_metadata(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    DM2_V1_GdatImageMetadata *out_metadata)
{
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint16_t cx;
    uint16_t cy;
    uint16_t bpp;
    uint16_t category_offset = 0u;
    uint16_t image_offset = 0u;
    int offset_y;
    uint32_t hash = 2166136261u;

    if (!out_metadata) return 0;
    memset(out_metadata, 0, sizeof(*out_metadata));
    raw = dm2_v1_asset_load_typed_sized(loader, category, index,
                                         DM2_GDAT_ENTRY_TYPE_IMAGE, field,
                                         &raw_size);
    if (!raw || raw_size < 10u) return 0;
    cx = (uint16_t)raw[0] | ((uint16_t)raw[1] << 8);
    cy = (uint16_t)raw[2] | ((uint16_t)raw[3] << 8);
    bpp = (uint16_t)raw[4] | ((uint16_t)raw[5] << 8);
    offset_y = (int)((int16_t)cy >> 10);
    out_metadata->width = (uint16_t)(cx & 0x03ffu);
    out_metadata->height = (uint16_t)(cy & 0x03ffu);
    if (out_metadata->width == 0u || out_metadata->height == 0u) {
        memset(out_metadata, 0, sizeof(*out_metadata));
        return 0;
    }
    /* skproject/SKULLWIN/c_gfx_pal.cpp / c_gfx_blit.cpp IMG3 header order:
     * cx/cy/bpp_word.  The source determines depth from cy's high bits: -32
     * means raw pixels at offset 10; 31 means 8bpp; everything else is 4bpp
     * (c_gdatfile.cpp:1205-1211).  Some compressed IMG3 records store data in
     * the bytes after cy, so an unrecognised bpp_word is treated as the normal
     * 4bpp fallback rather than rejected.  When bpp_word is explicitly 4/8 it
     * is honoured so synthetic fixtures can probe the depth gate. */
    if (offset_y == 31) {
        out_metadata->bits_per_pixel = 8u;
    } else if (bpp == 4u || bpp == 8u) {
        out_metadata->bits_per_pixel = (uint8_t)bpp;
    } else {
        out_metadata->bits_per_pixel = 4u;
    }
    out_metadata->graphicsset_offset_present =
        dm2_v1_asset_load_image_offset(loader, category, index, 0xfe,
                                       &category_offset);
    out_metadata->image_offset_present =
        dm2_v1_asset_load_image_offset(loader, category, index, field,
                                       &image_offset);
    out_metadata->query_offset_x =
        (int16_t)((int8_t)(category_offset >> 8) +
                  (int8_t)(image_offset >> 8));
    out_metadata->query_offset_y =
        (int16_t)((int8_t)category_offset + (int8_t)image_offset);
    hash = dm2_weather_hash_step(hash, out_metadata->width);
    hash = dm2_weather_hash_step(hash, out_metadata->height);
    hash = dm2_weather_hash_step(hash, out_metadata->bits_per_pixel);
    hash = dm2_weather_hash_step(hash, (uint16_t)category_offset);
    hash = dm2_weather_hash_step(hash, (uint16_t)image_offset);
    hash = dm2_weather_hash_step(hash,
                                 (uint32_t)out_metadata->graphicsset_offset_present);
    hash = dm2_weather_hash_step(hash,
                                 (uint32_t)out_metadata->image_offset_present);
    out_metadata->metadata_hash = hash;
    return hash != 0u;
}

int dm2_v1_asset_load_image_local_palette(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    uint8_t out_palette16[16], uint32_t *out_hash)
{
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint16_t cy;
    int offset_y;
    uint32_t hash = 2166136261u;

    if (out_hash) *out_hash = 0u;
    if (!out_palette16) return 0;
    memset(out_palette16, 0, 16u);
    raw = dm2_v1_asset_load_typed_sized(loader, category, index,
                                         DM2_GDAT_ENTRY_TYPE_IMAGE, field,
                                         &raw_size);
    if (!raw || raw_size < 26u) return 0;
    cy = (uint16_t)raw[2] | ((uint16_t)raw[3] << 8);
    offset_y = (int)((int16_t)cy >> 10);
    if (offset_y == 31) return 0;
    memcpy(out_palette16, raw + raw_size - 16u, 16u);
    for (int i = 0; i < 16; ++i) {
        hash = dm2_weather_hash_step(hash, out_palette16[i]);
    }
    if (hash == 0u) return 0;
    if (out_hash) *out_hash = hash;
    return 1;
}

static int dm2_weather_has_environment_image(const DM2_V1_AssetLoader *loader,
                                             uint8_t graphicsset,
                                             uint8_t command)
{
    size_t i;

    if (!loader || !loader->loaded || !loader->entries) return 0;
    /* skproject/SKWIN/c_bkgrnd.cpp
     * ENVIRONMENT_DRAW_DISTANT_ELEMENT calls QUERY_TEMP_PICST with
     * category 0x17, glbMapGraphicsSet and ref->envImg.  ref->envImg is the
     * original command number chosen by c_weather.cpp, not CD's rect number.
     * Keep the check typed and set-specific so a command text cannot borrow
     * a picture from another map style. */
    for (i = 0; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        if (entry->cls1 == DM2_GDAT_CATEGORY_ENVIRONMENT &&
            entry->cls2 == graphicsset &&
            entry->cls3 == DM2_GDAT_ENTRY_TYPE_IMAGE &&
            entry->cls4 == command) {
            return 1;
        }
    }
    return 0;
}

static const DM2_V1_GdatEntry *dm2_weather_find_environment_entry(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset, uint8_t field,
    int want_image)
{
    size_t i;

    if (!loader || !loader->loaded || !loader->entries) return NULL;
    for (i = 0u; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        if (entry->cls1 == DM2_GDAT_CATEGORY_ENVIRONMENT &&
            entry->cls2 == graphicsset && entry->cls4 == field &&
            ((want_image && entry->cls3 == DM2_GDAT_ENTRY_TYPE_IMAGE) ||
             (!want_image && entry->cls3 == DM2_GDAT_ENTRY_TYPE_TEXT))) {
            return entry;
        }
    }
    return NULL;
}

static int dm2_weather_environment_material_receipt(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset, uint8_t field,
    DM2_V1_EnvironmentWeatherMaterialReceipt *out)
{
    const DM2_V1_GdatEntry *text_entry;
    const DM2_V1_GdatEntry *image_entry;
    const uint8_t *text;
    const uint8_t *image;
    size_t text_size;
    size_t image_size;
    int found_cd;
    int found_fw;
    int32_t cd;
    int32_t fw;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    text_entry = dm2_weather_find_environment_entry(loader, graphicsset,
                                                     field, 0);
    image_entry = dm2_weather_find_environment_entry(loader, graphicsset,
                                                      field, 1);
    if (!text_entry || !image_entry || !loader->data || !loader->raw_offsets ||
        !loader->raw_sizes || text_entry->data_index >= loader->raw_data_count ||
        image_entry->data_index >= loader->raw_data_count) {
        return 0;
    }
    text_size = loader->raw_sizes[text_entry->data_index];
    image_size = loader->raw_sizes[image_entry->data_index];
    text = loader->data + loader->raw_offsets[text_entry->data_index];
    image = loader->data + loader->raw_offsets[image_entry->data_index];
    {
        /* QUERY_GDAT_TEXT decode for encoded GDAT text; fall back to the
         * raw bytes when the GDAT header does not set the encode flag. */
        uint8_t decoded[DM2_V1_WEATHER_COMMAND_TEXT_MAX];
        uint32_t decoded_size = 0u;
        uint32_t decoded_hash = 0u;
        const uint8_t *command_text = text;
        size_t command_text_size = text_size;

        if (dm2_weather_decode_command_text(loader, text, text_size,
                                            decoded, &decoded_size,
                                            &decoded_hash)) {
            command_text = decoded;
            command_text_size = decoded_size;
        }
        if (!text || !image || text_size == 0u || image_size == 0u ||
            loader->raw_offsets[text_entry->data_index] > loader->data_size ||
            text_size > loader->data_size - loader->raw_offsets[text_entry->data_index] ||
            loader->raw_offsets[image_entry->data_index] > loader->data_size ||
            image_size > loader->data_size - loader->raw_offsets[image_entry->data_index] ||
            !dm2_v1_weather_cmdstr_query(command_text, command_text_size,
                                         "cd", &found_cd, &cd) ||
            !dm2_v1_weather_cmdstr_query(command_text, command_text_size,
                                         "fw", &found_fw, &fw) ||
            !found_cd || !found_fw || fw < 0 || fw > UINT8_MAX) {
            return 0;
        }
    }
    out->environment_field = field;
    out->command_cd = cd;
    out->command_fw = (uint8_t)fw;
    out->text_hash = dm2_weather_hash_bytes(text, text_size);
    out->image_hash = dm2_weather_hash_bytes(image, image_size);
    out->image_byte_count = (uint32_t)image_size;
    return out->text_hash != 0u && out->image_hash != 0u;
}

int dm2_v1_weather_gdat_environment_receipt(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    uint32_t map_load_token, int outdoor_scene, int weather_enabled,
    uint8_t cloud_field, uint8_t wet_ground_field, int draw_clouds,
    int draw_wet_ground, DM2_V1_EnvironmentWeatherReceipt *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !loader->loaded || map_load_token == 0u) return 0;
    out->valid = 1;
    out->graphicsset = graphicsset;
    out->map_load_token = map_load_token;
    if (!outdoor_scene || !weather_enabled) {
        out->receipt_hash = hash;
        return 1;
    }
    if (draw_clouds && !dm2_weather_environment_material_receipt(
            loader, graphicsset, cloud_field, &out->materials[out->material_count])) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    if (draw_clouds) ++out->material_count;
    if (draw_wet_ground && !dm2_weather_environment_material_receipt(
            loader, graphicsset, wet_ground_field,
            &out->materials[out->material_count])) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    if (draw_wet_ground) ++out->material_count;
    hash = dm2_weather_hash_step(hash, map_load_token);
    hash = dm2_weather_hash_step(hash, graphicsset);
    hash = dm2_weather_hash_step(hash, out->material_count);
    for (unsigned int i = 0u; i < out->material_count; ++i) {
        hash = dm2_weather_hash_step(hash, out->materials[i].environment_field);
        hash = dm2_weather_hash_step(hash, out->materials[i].text_hash);
        hash = dm2_weather_hash_step(hash, out->materials[i].image_hash);
    }
    out->receipt_hash = hash;
    return 1;
}

int dm2_v1_weather_cmdstr_query(const uint8_t *text, size_t text_size,
                                 const char *name, int *out_found,
                                 int32_t *out_value)
{
    size_t name_size;
    size_t cursor = 0u;
    int found = 0;
    int32_t value = 0;

    if (out_found) *out_found = 0;
    if (out_value) *out_value = 0;
    if (!text || !name || name[0] == '\0' ||
        !dm2_weather_text_has_nul(text, text_size)) {
        return 0;
    }
    name_size = strlen(name);
    while (cursor + name_size <= text_size) {
        size_t i;
        size_t at = text_size;
        int negative = 0;

        for (i = cursor; i + name_size <= text_size; ++i) {
            if (memcmp(text + i, name, name_size) == 0) {
                at = i;
                break;
            }
            if (text[i] == '\0') break;
        }
        if (at == text_size) break;
        found = 1;
        at += name_size;
        if (at < text_size && text[at] == '=') ++at;
        if (at < text_size && text[at] == '-') {
            negative = 1;
            ++at;
        }
        while (at < text_size && text[at] >= '0' && text[at] <= '9') {
            /* skproject c_querydb.cpp DM2_QUERY_CMDSTR_TEXT: result is a
             * signed long.  Reject overflow rather than wrapping into a
             * fabricated GDAT rectangle. */
            if (value > (INT32_MAX - (int32_t)(text[at] - '0')) / 10) {
                return 0;
            }
            value = value * 10 + (int32_t)(text[at] - '0');
            ++at;
        }
        if (negative) value = -value;
        cursor = at > cursor ? at : cursor + 1u;
    }
    if (out_found) *out_found = found;
    if (out_value) *out_value = value;
    return 1;
}

static int dm2_weather_decode_material(const DM2_V1_AssetLoader *loader,
                                       uint8_t graphicsset,
                                       DM2_V1_WeatherCommandReceipt *out)
{
    int found_cd = 0;
    int found_fw = 0;
    int32_t cd = 0;
    int32_t fw = 0;
    uint32_t hash;
    uint8_t *pixels;
    const uint8_t *image_source;
    const uint8_t *command_text;
    size_t command_text_size;
    size_t image_source_size = 0u;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;

    if (!out || !out->raw_text || out->byte_count == 0u) {
        return 0;
    }
    /* QUERY_CMDSTR_TEXT consumes QUERY_GDAT_TEXT's decoded form; the source
     * keys are the lowercase EnvCM_CD/EnvCM_FW strings (SkGlobal.cpp:755). */
    command_text = out->decoded_text_size != 0u ? out->decoded_text
                                                : out->raw_text;
    command_text_size = out->decoded_text_size != 0u
                            ? (size_t)out->decoded_text_size
                            : (size_t)out->byte_count;
    if (!dm2_v1_weather_cmdstr_query(command_text, command_text_size,
                                     "cd", &found_cd, &cd) ||
        !dm2_v1_weather_cmdstr_query(command_text, command_text_size,
                                     "fw", &found_fw, &fw) ||
        !found_cd || cd <= 0 || cd > UINT16_MAX ||
        (found_fw && (fw < 0 || fw > UINT8_MAX))) {
        return 0;
    }
    out->rect_number = (uint16_t)cd;
    out->flip_mode = found_fw ? (uint8_t)fw : 0u;
    out->image_field = out->command;
    out->image_present = dm2_weather_has_environment_image(
        loader, graphicsset, out->image_field);
    if (!out->image_present) return 0;
    image_source = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_ENVIRONMENT, graphicsset,
        DM2_GDAT_ENTRY_TYPE_IMAGE, out->image_field, &image_source_size);
    if (!image_source || image_source_size == 0u ||
        image_source_size > UINT32_MAX ||
        !dm2_weather_source_raw_index(loader, image_source,
                                      image_source_size,
                                      &out->image_raw_index)) {
        return 0;
    }
    out->image_source_bytes = image_source;
    out->image_source_byte_count = (uint32_t)image_source_size;
    out->image_raw_hash = dm2_weather_hash_bytes(image_source,
                                                  image_source_size);
    {
        DM2_V1_GdatGfxRawMaterialReceipt material;
        if (out->image_raw_hash == 0u ||
            !dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
                loader, out->image_raw_index, &material) ||
            material.source_bytes != image_source ||
            material.source_byte_count != image_source_size ||
            !material.receipt_hash) {
            return 0;
        }
        out->image_material_receipt_hash = material.receipt_hash;
    }
    out->query_metadata_valid = dm2_v1_asset_load_image_metadata(
        loader, DM2_GDAT_CATEGORY_ENVIRONMENT, graphicsset,
        out->image_field, &out->query_metadata);
    if (!out->query_metadata_valid) return 0;
    /* c_bkgrnd.cpp passes this exact ENVIRONMENT dtImage into
     * QUERY_TEMP_PICST.  The real DM2 GRAPHICS.DAT carries the nine
     * 0x64..0x6c command images as 8bpp IMG9 (global-palette pictures);
     * the synthetic 4bpp IMG3/U4 local-palette form is also admitted.
     * Keep the bounded decoded-pixel receipt first so the real-data
     * evidence is recorded ahead of the palette-translation binding
     * below. */
    pixels = dm2_v1_asset_load_image_field(
        loader, DM2_GDAT_CATEGORY_ENVIRONMENT, graphicsset, out->image_field,
        &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        width != (int)out->query_metadata.width ||
        height != (int)out->query_metadata.height ||
        !((out->query_metadata.bits_per_pixel == 4u &&
           (format == DM2_IMG_FMT_IMG3 || format == DM2_IMG_FMT_U4)) ||
          (out->query_metadata.bits_per_pixel == 8u &&
           format == DM2_IMG_FMT_IMG9))) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    out->decoded_pixels_hash = dm2_weather_hash_bytes(pixels, pixel_count);
    dm2_v1_asset_free_pixels(pixels);
    if (out->decoded_pixels_hash == 0u) return 0;
    out->decoded_pixels_valid = 1;
    out->decoded_width = (uint16_t)width;
    out->decoded_height = (uint16_t)height;
    out->decoded_format = format;
    out->decoded_pixel_count = (uint32_t)pixel_count;
    /* skproject QUERY_TEMP_PICST realizes the 4bpp ENVIRONMENT IMG3 through
     * QUERY_GDAT_IMAGE_LOCALPAL.  A valid command text and dimensions are
     * not enough to authorize that weather material without this per-image
     * palette receipt.  QUERY_GDAT_IMAGE_LOCALPAL (SkWinCore.cpp 3e74:521A,
     * DM2_EXTENDED_MODE==1) returns NULL whenever the realized image is not
     * 4bpp, so the real 8bpp IMG9 command images carry no 16-color local
     * palette; QUERY_GDAT_SUMMARY_IMAGE (0B36:0520) then installs the
     * 256-entry identity translation (ref->b58[i] = i, ref->w56 = 256) and
     * every decoded pixel byte indexes the global screen palette directly.
     * Bind exactly that source translation for each admitted format. */
    if (out->query_metadata.bits_per_pixel == 4u) {
        out->local_palette_valid = dm2_v1_asset_load_image_local_palette(
            loader, DM2_GDAT_CATEGORY_ENVIRONMENT, graphicsset,
            out->image_field, out->local_palette16, &out->local_palette_hash);
        if (!out->local_palette_valid || out->local_palette_hash == 0u) {
            return 0;
        }
        out->palette_translation_count = 16u;
        out->palette_translation_hash = out->local_palette_hash;
    } else {
        /* 8bpp IMG9: the local-palette query is NULL by source rule, so
         * SUMMARY_IMAGE's identity table is the palette material.  Hash the
         * exact 256-byte identity map; its value fully determines the
         * translation. */
        uint32_t identity_hash =
            dm2_v1_weather_environment_identity_palette_hash();
        if (identity_hash == 0u) return 0;
        out->global_palette_identity_valid = 1;
        out->global_palette_identity_hash = identity_hash;
        out->palette_translation_count = 256u;
        out->palette_translation_hash = identity_hash;
    }
    hash = out->raw_hash;
    hash ^= out->decoded_text_hash;
    hash *= 16777619u;
    hash ^= out->rect_number;
    hash *= 16777619u;
    hash ^= out->flip_mode;
    hash *= 16777619u;
    hash ^= out->query_metadata.metadata_hash;
    hash *= 16777619u;
    hash ^= out->palette_translation_hash;
    hash *= 16777619u;
    hash ^= out->palette_translation_count;
    hash *= 16777619u;
    hash ^= out->decoded_pixels_hash;
    hash *= 16777619u;
    hash ^= out->decoded_pixel_count;
    hash *= 16777619u;
    hash ^= out->image_raw_hash;
    hash *= 16777619u;
    hash ^= out->image_source_byte_count;
    hash *= 16777619u;
    hash ^= out->image_material_receipt_hash;
    hash *= 16777619u;
    out->material_hash = hash;
    out->material_valid = 1;
    return 1;
}

uint8_t dm2_v1_weather_gdat_cloud_command_for_level(uint8_t level)
{
    if (level >= 0x80u) return DM2_V1_WEATHER_CLOUD_STORM_CMD;
    if (level >= 0x40u) return DM2_V1_WEATHER_CLOUD_HEAVY_CMD;
    if (level >= 0x10u) return DM2_V1_WEATHER_CLOUD_LIGHT_CMD;
    return 0u;
}

uint8_t dm2_v1_weather_gdat_rain_command_for_level(uint8_t level)
{
    if (level >= 0xc0u) return DM2_V1_WEATHER_RAIN_STORM_CMD;
    if (level >= 0x80u) return DM2_V1_WEATHER_RAIN_HEAVY_CMD;
    if (level >= 0x40u) return DM2_V1_WEATHER_RAIN_LIGHT_CMD;
    return 0u;
}

static uint32_t dm2_weather_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

uint32_t dm2_v1_weather_environment_identity_palette_hash(void)
{
    uint32_t hash = 2166136261u;
    unsigned int entry;

    for (entry = 0u; entry < 256u; ++entry) {
        hash = dm2_weather_hash_step(hash, entry);
    }
    return hash;
}

int dm2_v1_weather_environment_asset_palette_fetch(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset, uint8_t command,
    uint8_t out_palette16[16], uint32_t *out_hash)
{
    DM2_V1_GdatImageMetadata metadata;

    if (out_palette16) memset(out_palette16, 0, 16u);
    if (out_hash) *out_hash = 0u;
    if (!loader || !out_palette16 || !out_hash ||
        command < DM2_V1_WEATHER_BOLT_CMD_BASE ||
        command > DM2_V1_WEATHER_RAIN_STORM_CMD) {
        return 0;
    }
    if (!dm2_v1_asset_load_image_metadata(
            loader, DM2_GDAT_CATEGORY_ENVIRONMENT, (int)graphicsset,
            (int)command, &metadata)) {
        return 0;
    }
    if (metadata.bits_per_pixel == 4u) {
        return dm2_v1_asset_load_image_local_palette(
            loader, DM2_GDAT_CATEGORY_ENVIRONMENT, (int)graphicsset,
            (int)command, out_palette16, out_hash);
    }
    /* 8bpp IMG9: QUERY_GDAT_IMAGE_LOCALPAL returns NULL and SUMMARY_IMAGE
     * installs the 256-entry identity translation (ref->b58[i] = i).  The
     * renderer recognizes the 16-byte identity prefix and passes pixel bytes
     * through to the global palette. */
    for (int i = 0; i < 16; ++i) {
        out_palette16[i] = (uint8_t)i;
    }
    *out_hash = dm2_v1_weather_environment_identity_palette_hash();
    return 1;
}

static int dm2_weather_overlay_append(
    const DM2_V1_WeatherGdatReceipt *receipt,
    uint8_t command,
    uint8_t slot_index,
    DM2_V1_WeatherOverlayPlan *out,
    uint32_t *hash)
{
    const DM2_V1_WeatherCommandReceipt *source;
    unsigned int index;

    if (command == 0u) return 1;
    if (!receipt || !out || !hash ||
        command < DM2_V1_WEATHER_CLOUD_LIGHT_CMD ||
        command > DM2_V1_WEATHER_RAIN_STORM_CMD ||
        out->command_count >= sizeof(out->commands) / sizeof(out->commands[0])) {
        return 0;
    }
    index = (unsigned int)(command - DM2_V1_WEATHER_BOLT_CMD_BASE);
    source = &receipt->commands[index];
    /* skproject c_weather.cpp lines 221-266 calls
     * DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW before it advances to the next
     * ten-byte command slot.  Do not turn a bare CMDSTR record into pixels. */
    if (source->command != command || !source->material_valid ||
        source->rect_number == 0u ||
        !source->query_metadata_valid ||
        !source->decoded_pixels_valid || source->decoded_pixels_hash == 0u ||
        (receipt->material_mask & DM2_V1_WEATHER_COMMAND_MASK(command)) == 0u) {
        return 0;
    }

    out->commands[out->command_count].command = command;
    out->commands[out->command_count].slot_index = slot_index;
    out->commands[out->command_count].rect_number = source->rect_number;
    out->commands[out->command_count].flip_mode = source->flip_mode;
    /* skproject c_querydb.cpp::DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW:
     * DistantEnvironment.w4/w6 = 0 and b8/b9 = 0x40.  The later
     * ENVIRONMENT_DRAW_DISTANT_ELEMENT call owns any movement interpolation
     * and the QUERY_TEMP_PICST image realization; neither is inferred here. */
    out->commands[out->command_count].source_offset_x = 0;
    out->commands[out->command_count].source_offset_y = 0;
    out->commands[out->command_count].source_scale_x = 0x40u;
    out->commands[out->command_count].source_scale_y = 0x40u;
    out->commands[out->command_count].image_width =
        source->query_metadata.width;
    out->commands[out->command_count].image_height =
        source->query_metadata.height;
    out->commands[out->command_count].query_offset_x =
        source->query_metadata.query_offset_x;
    out->commands[out->command_count].query_offset_y =
        source->query_metadata.query_offset_y;
    out->commands[out->command_count].material_hash = source->material_hash;
    ++out->command_count;
    out->required_mask |= DM2_V1_WEATHER_COMMAND_MASK(command);
    out->material_mask |= DM2_V1_WEATHER_COMMAND_MASK(command);
    *hash = dm2_weather_hash_step(*hash, command);
    *hash = dm2_weather_hash_step(*hash, source->material_hash);
    *hash = dm2_weather_hash_step(*hash, source->rect_number);
    *hash = dm2_weather_hash_step(*hash, source->flip_mode);
    *hash = dm2_weather_hash_step(*hash, source->query_metadata.metadata_hash);
    *hash = dm2_weather_hash_step(*hash, 0x40400000u);
    return 1;
}

int dm2_v1_weather_gdat_overlay_plan(
    const DM2_V1_WeatherGdatReceipt *receipt,
    uint8_t cloud_level,
    uint8_t rain_level,
    DM2_V1_WeatherOverlayPlan *out)
{
    uint8_t cloud_command;
    uint8_t rain_command;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!receipt || !receipt->valid || receipt->receipt_hash == 0u) return 0;

    /* skproject/SKULLWIN/c_weather.cpp DM2_UPDATE_WEATHER lines 221-266:
     * cloud selection is emitted first, followed by rain.  The source's
     * xp_1c += 10 applies only after a successful cloud material lookup. */
    cloud_command = dm2_v1_weather_gdat_cloud_command_for_level(cloud_level);
    rain_command = dm2_v1_weather_gdat_rain_command_for_level(rain_level);
    out->cloud_level = cloud_level;
    out->rain_level = rain_level;
    hash = dm2_weather_hash_step(hash, receipt->receipt_hash);
    hash = dm2_weather_hash_step(hash, cloud_level);
    hash = dm2_weather_hash_step(hash, rain_level);
    if (!dm2_weather_overlay_append(receipt, cloud_command, 0u, out, &hash)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    if (!dm2_weather_overlay_append(receipt, rain_command,
                                    out->command_count ? 1u : 0u,
                                    out, &hash)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->plan_hash = hash;
    out->valid = 1;
    return 1;
}

static int dm2_weather_stretch64(int16_t value, int16_t factor,
                                 int16_t *out_value)
{
    int32_t result;

    if (!out_value) return 0;
    /* skproject CALC_STRETCHED_SIZE (0B36:0036):
     * (value * factor + (factor >> 1)) >> 6.  Source inputs are signed
     * 16-bit values.  Refuse an out-of-range result instead of relying on a
     * host-specific narrowing conversion for a displayed destination. */
    result = ((int32_t)value * factor + (factor >> 1)) >> 6;
    if (result < INT16_MIN || result > INT16_MAX) return 0;
    *out_value = (int16_t)result;
    return 1;
}

static int dm2_weather_add_i16(int16_t left, int16_t right,
                               int16_t *out_value)
{
    int32_t result;

    if (!out_value) return 0;
    result = (int32_t)left + right;
    if (result < INT16_MIN || result > INT16_MAX) return 0;
    *out_value = (int16_t)result;
    return 1;
}

static uint16_t dm2_weather_read_u16(const uint8_t *raw)
{
    return (uint16_t)raw[0] | ((uint16_t)raw[1] << 8);
}

static int dm2_weather_image_bounds(const DM2_V1_GdatImageMetadata *metadata,
                                    int16_t *out_left, int16_t *out_top,
                                    int16_t *out_right, int16_t *out_bottom)
{
    int32_t right;
    int32_t bottom;

    if (!metadata || !out_left || !out_top || !out_right || !out_bottom ||
        metadata->width == 0u || metadata->height == 0u) {
        return 0;
    }
    right = (int32_t)metadata->query_offset_x + metadata->width;
    bottom = (int32_t)metadata->query_offset_y + metadata->height;
    if (right < INT16_MIN || right > INT16_MAX ||
        bottom < INT16_MIN || bottom > INT16_MAX) {
        return 0;
    }
    *out_left = metadata->query_offset_x;
    *out_top = metadata->query_offset_y;
    *out_right = (int16_t)right;
    *out_bottom = (int16_t)bottom;
    return 1;
}

static int dm2_weather_flip_from_position(uint8_t kind,
                                          const DM2_V1_WeatherDrawContext *ctx)
{
    int64_t parity;

    if (!ctx) return 0;
    /* skproject SET_GRAPHICS_FLIP_FROM_POSITION (32CB:59CA). */
    parity = (int64_t)ctx->map_x + ctx->map_y + ctx->direction +
             ctx->map_offset_x + ctx->map_offset_y + ctx->map_level;
    parity &= 1;
    if (kind == 1u) {
        if ((ctx->scene_flags & 8u) != 0u) {
            if ((ctx->scene_flags & 0x10u) != 0u) {
                return (ctx->game_tick & 7u) > 3u;
            }
            return (int)parity;
        }
        if ((ctx->scene_flags & 0x40u) != 0u) {
            return (ctx->player_direction & 1u) != 0u;
        }
        return 0;
    }
    if (kind == 0x20u) {
        if ((ctx->scene_flags & 2u) != 0u) {
            if ((ctx->scene_flags & 4u) != 0u) {
                return (ctx->game_tick & 7u) <= 3u;
            }
            return !parity;
        }
        if ((ctx->scene_flags & 0x20u) != 0u) {
            return (ctx->player_direction & 1u) != 0u;
        }
        return 0;
    }
    return (int)parity;
}

static int dm2_weather_gdat_draw_plan_from_raw(
    const DM2_V1_WeatherCommandReceipt *command,
    const uint8_t raw[DM2_V1_DISTANT_ENVIRONMENT_BYTES],
    const DM2_V1_WeatherDrawContext *context,
    DM2_V1_WeatherDrawPlan *out)
{
    int16_t offset_x;
    int16_t offset_y;
    int16_t scale_x;
    int16_t scale_y;
    uint8_t flip_kind = 0u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!command || !raw || !context || !command->material_valid ||
        !command->image_present || !command->query_metadata_valid ||
        command->rect_number == 0u || raw[0] != command->command ||
        dm2_weather_read_u16(raw + 2u) != command->rect_number ||
        !dm2_weather_command_is_source_owned(command->command)) {
        return 0;
    }
    if (dm2_weather_command_is_bolt(command->command)) {
        /* c_weather.cpp:441-474: after a successful retrieve the source
         * overwrites the bolt slot's cmFW byte with DM2_RANDDIR() (0..3).
         * The GDAT bolt text carries no FW key, so byte 1 is the live
         * RANDDIR value, not the command receipt's flip_mode. */
        if (raw[1] > 3u) return 0;
    } else if (raw[1] != command->flip_mode) {
        return 0;
    }

    /* ENVIRONMENT_DRAW_DISTANT_ELEMENT chooses a mirror request only for
     * these four original FW values.  Other source FW values draw unflipped.
     */
    if (raw[1] == 8u || raw[1] == 0x40u) {
        flip_kind = 1u;
    } else if (raw[1] == 2u || raw[1] == 0x20u) {
        flip_kind = 0x20u;
    }
    offset_x = (int16_t)dm2_weather_read_u16(raw + 4u);
    offset_y = (int16_t)dm2_weather_read_u16(raw + 6u);
    scale_x = raw[8];
    scale_y = raw[9];
    if (scale_x <= 0 || scale_y <= 0 || scale_x > 0x40 || scale_y > 0x40) {
        return 0;
    }
    if (context->player_moving) {
        if (!dm2_weather_stretch64(offset_x, 0x34, &offset_x) ||
            !dm2_weather_stretch64(offset_y, 0x34, &offset_y) ||
            !dm2_weather_stretch64(scale_x, 0x34, &scale_x) ||
            !dm2_weather_stretch64(scale_y, 0x34, &scale_y)) {
            return 0;
        }
        if (raw[8] == 0x40u) {
            /* ENVIRONMENT_DRAW_DISTANT_ELEMENT's full-size branch includes
             * x movement and treats the horizon rect 6001 specially. */
            if (!dm2_weather_add_i16(offset_x, context->movement_offset_x,
                                     &offset_x)) {
                return 0;
            }
            if (command->rect_number == 0x1771u) {
                if (!dm2_weather_add_i16(offset_y,
                                         context->moving_horizon_offset_y,
                                         &offset_y)) {
                    return 0;
                }
            } else if (!dm2_weather_add_i16(offset_y,
                                             context->movement_offset_y,
                                     &offset_y)) {
                return 0;
            }
        } else if (!dm2_weather_add_i16(offset_y,
                                         context->moving_other_offset_y,
                                         &offset_y)) {
            return 0;
        }
    }
    /* skproject c_querydb.cpp::DM2_QUERY_TEMP_PICST first realizes the
     * selected IMG3 through QUERY_GDAT_SUMMARY_IMAGE, which applies the
     * category 0xfe and image-field offsets. Retain that exact renderable
     * source extent; destination clipping stays unproven and is not guessed. */
    if (!dm2_weather_image_bounds(&command->query_metadata,
                                  &out->source_left, &out->source_top,
                                  &out->source_right, &out->source_bottom)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->command = command->command;
    out->rect_number = command->rect_number;
    out->image_field = command->image_field;
    if (dm2_weather_command_is_bolt(command->command)) {
        /* ENVIRONMENT_DRAW_DISTANT_ELEMENT evaluates a mirror only for
         * cmFW == 2 (kind 0x20); every other RANDDIR byte draws unflipped. */
        out->mirror_flip = (raw[1] == 2u)
                               ? (uint8_t)dm2_weather_flip_from_position(
                                     0x20u, context)
                               : 0u;
    } else {
        out->mirror_flip = (uint8_t)dm2_weather_flip_from_position(
            flip_kind, context);
    }
    out->scale_x = (uint8_t)scale_x;
    out->scale_y = (uint8_t)scale_y;
    out->draw_offset_x = offset_x;
    out->draw_offset_y = offset_y;
    out->source_bounds_valid = 1;
    out->decoded_pixels_hash = command->decoded_pixels_hash;
    out->decoded_pixel_count = command->decoded_pixel_count;
    /* For 8bpp IMG9 the source local-palette query is NULL; the authoritative
     * palette material is the SUMMARY_IMAGE identity translation.  Use that
     * hash as the comparison value so the renderer's active-palette check
     * matches the identity table the asset provider returns. */
    out->local_palette_hash = command->local_palette_hash != 0u
        ? command->local_palette_hash
        : command->palette_translation_hash;
    out->palette_translation_count = command->palette_translation_count;
    out->palette_translation_hash = command->palette_translation_hash;
    if (out->decoded_pixels_hash == 0u || out->decoded_pixel_count == 0u ||
        out->palette_translation_hash == 0u) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->material_hash = command->material_hash;
    out->valid = 1;
    return 1;
}

int dm2_v1_weather_gdat_draw_plan(
    const DM2_V1_WeatherCommandReceipt *command,
    const DM2_V1_WeatherDrawContext *context,
    DM2_V1_WeatherDrawPlan *out)
{
    uint8_t raw[DM2_V1_DISTANT_ENVIRONMENT_BYTES] = { 0 };

    if (!command) return 0;
    /* RETRIEVE_ENVIRONMENT_CMD_CD_FW initializes w4/w6 to zero and b8/b9
     * to 0x40. This compatibility entry point models only that initial slot;
     * runtime presentation uses the full source-owned receipt below. */
    raw[0] = command->command;
    raw[1] = command->flip_mode;
    raw[2] = (uint8_t)(command->rect_number & 0xffu);
    raw[3] = (uint8_t)(command->rect_number >> 8);
    raw[8] = 0x40u;
    raw[9] = 0x40u;
    return dm2_weather_gdat_draw_plan_from_raw(command, raw, context, out);
}

int dm2_v1_weather_gdat_draw_plan_from_distant_environment(
    const DM2_V1_WeatherCommandReceipt *command,
    const DM2_V1_DistantEnvironmentReceipt *slot,
    const DM2_V1_WeatherDrawContext *context,
    DM2_V1_WeatherDrawPlan *out)
{
    if (!slot || !slot->valid || slot->raw_hash == 0u || !command ||
        slot->raw_hash != dm2_weather_hash_bytes(
                              slot->raw, DM2_V1_DISTANT_ENVIRONMENT_BYTES) ||
        slot->command != command->command) {
        if (out) memset(out, 0, sizeof(*out));
        return 0;
    }
    return dm2_weather_gdat_draw_plan_from_raw(command, slot->raw, context,
                                                out);
}

static uint16_t dm2_weather_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int dm2_weather_rect_raw(const uint8_t *raw, size_t raw_size,
                                uint16_t rect_id,
                                DM2_V1_WeatherDestinationClip *out)
{
    uint16_t groups;
    size_t pos;
    uint16_t group;

    if (!raw || raw_size < 4u || !out || dm2_weather_le16(raw) != 0xfc0du) {
        return 0;
    }
    groups = dm2_weather_le16(raw + 2u);
    if (groups == 0u || 4u + (size_t)groups * 4u > raw_size) return 0;
    pos = 4u + (size_t)groups * 4u;
    for (group = 0u; group < groups; ++group) {
        uint16_t first = dm2_weather_le16(raw + 4u + (size_t)group * 4u);
        uint16_t last = dm2_weather_le16(raw + 6u + (size_t)group * 4u);
        size_t count = last >= first ? (size_t)(last - first + 1u) : 0u;
        const uint8_t *row;

        if (count == 0u || pos + count * 8u > raw_size) return 0;
        if (rect_id < first || rect_id > last) {
            pos += count * 8u;
            continue;
        }
        row = raw + pos + (size_t)(rect_id - first) * 8u;
        out->x = (int16_t)dm2_weather_le16(row);
        out->y = (int16_t)dm2_weather_le16(row + 2u);
        out->w = (int16_t)dm2_weather_le16(row + 4u);
        out->h = (int16_t)dm2_weather_le16(row + 6u);
        return 1;
    }
    return 0;
}

int dm2_v1_weather_gdat_destination_clip(
    const uint8_t *rect_table,
    size_t rect_table_size,
    const DM2_V1_WeatherCommandReceipt *command,
    DM2_V1_WeatherDestinationClip *out)
{
    DM2_V1_WeatherDestinationClip current;
    DM2_V1_WeatherDestinationClip next;
    uint32_t hash = 2166136261u;
    int anchor;
    int x;
    int y;
    int w;
    int h;
    size_t i;
    int guard;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!rect_table || !command || !command->material_valid ||
        command->rect_number == 0u) {
        return 0;
    }
    if (!dm2_weather_rect_raw(rect_table, rect_table_size,
                              command->rect_number, &current) ||
        current.y == 0) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp QUERY_BLIT_RECT (098D:05C6-098D:0891):
     * weather's QUERY_TEMP_PICST supplies CD as rectno. The chain stores
     * anchor/offset entries (x == 1) followed by a terminator (x == 9) that
     * carries the destination width/height and finalizes the clip. The anchor
     * value is taken from the original CD rect, not from intermediate offset
     * entries, so it is preserved across an arbitrarily long offset chain. */
    anchor = current.x;
    x = current.w;
    y = current.h;
    w = 0;
    h = 0;
    for (guard = 0; current.y != 0 && guard < 16; ++guard) {
        if (!dm2_weather_rect_raw(rect_table, rect_table_size,
                                  (uint16_t)current.y, &next)) {
            return 0;
        }
        if (next.x == 1) {
            x += next.w;
            y += next.h;
        } else if (next.x == 9) {
            int dx;
            int dy;
            switch (anchor) {
            case 1: dx = current.w; dy = current.h; break;
            case 4: dx = current.w; dy = current.h - (next.h - 1); break;
            case 7: dx = current.w - ((next.w + 1) >> 1);
                    dy = current.h - (next.h - 1); break;
            default: return 0;
            }
            x += dx;
            y += dy;
            w = next.w;
            h = next.h;
        } else {
            return 0;
        }
        current = next;
    }
    if (current.y != 0 || anchor < 1 || anchor > 8 || w <= 0 || h <= 0 ||
        x < INT16_MIN || x > INT16_MAX || y < INT16_MIN || y > INT16_MAX ||
        w > INT16_MAX || h > INT16_MAX) {
        return 0;
    }
    out->x = (int16_t)((anchor == 1 || anchor == 4 || anchor == 8) ?
                       x : x - ((w + 1) >> 1));
    out->y = (int16_t)((anchor == 1 || anchor == 2 || anchor == 5) ?
                       y : y - (h - 1));
    out->w = (int16_t)w;
    out->h = (int16_t)h;
    for (i = 0u; i < rect_table_size; ++i) {
        hash = dm2_weather_hash_step(hash, rect_table[i]);
    }
    out->table_hash = hash;
    out->valid = hash != 0u;
    return out->valid;
}

int dm2_v1_weather_gdat_renderer_receipt(
    const DM2_V1_WeatherRestoredStateReceipt *restored_state,
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_DistantEnvironmentReceipt *slots,
    unsigned int slot_count,
    const DM2_V1_WeatherDrawContext *context,
    const uint8_t *rect_table,
    size_t rect_table_size,
    DM2_V1_WeatherRendererReceipt *out)
{
    uint32_t distant_hash = 2166136261u;
    uint32_t renderer_hash = 2166136261u;
    unsigned int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!restored_state || !restored_state->valid ||
        restored_state->state_hash == 0u || !weather || !weather->valid ||
        !context || slot_count > DM2_V1_WEATHER_MAX_SLOTS ||
        (slot_count != 0u && !slots)) {
        return 0;
    }

    out->restored_state = *restored_state;
    renderer_hash = dm2_weather_hash_step(renderer_hash, restored_state->state_hash);
    renderer_hash = dm2_weather_hash_step(renderer_hash, weather->receipt_hash);
    for (i = 0u; i < slot_count; ++i) {
        const DM2_V1_DistantEnvironmentReceipt *slot = &slots[i];
        const DM2_V1_WeatherCommandReceipt *command;
        unsigned int command_index;

        /* c_weather.cpp puts the selected command in byte zero, then consumes
         * cloud before rain in successive ten-byte DistantEnvironment slots.
         * Do not infer either selection from restored generic weather fields. */
        if (!slot->valid || slot->slot_index != i ||
            !dm2_weather_command_is_source_owned(slot->command) ||
            slot->raw[0] != slot->command || slot->raw_hash == 0u) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        command_index = (unsigned int)(slot->command -
                                       DM2_V1_WEATHER_BOLT_CMD_BASE);
        if (command_index >= DM2_V1_WEATHER_COMMAND_COUNT) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        command = &weather->commands[command_index];
        if (command->command != slot->command ||
            !command->material_valid ||
            !dm2_v1_weather_gdat_draw_plan_from_distant_environment(
                command, slot, context, &out->draws[i])) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        if (!dm2_v1_weather_gdat_destination_clip(rect_table, rect_table_size,
                                                   command, &out->clips[i])) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        distant_hash = dm2_weather_hash_step(distant_hash, slot->raw_hash);
        renderer_hash = dm2_weather_hash_step(renderer_hash, slot->command);
        renderer_hash = dm2_weather_hash_step(renderer_hash,
                                              out->draws[i].material_hash);
        renderer_hash = dm2_weather_hash_step(renderer_hash,
                                              out->clips[i].table_hash);
    }
    out->command_count = slot_count;
    out->distant_environment_hash = distant_hash;
    out->renderer_hash = dm2_weather_hash_step(renderer_hash, distant_hash);
    out->valid = out->renderer_hash != 0u;
    return out->valid;
}

int dm2_v1_weather_gdat_outdoor_m11_receipt(
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_WeatherRendererReceipt *renderer,
    const DM2_V1_SetTimerWeatherReceipt *timer_owner,
    DM2_V1_OutdoorWeatherM11Receipt *out)
{
    uint32_t raw_hash = 2166136261u;
    uint32_t receipt_hash = 2166136261u;
    unsigned int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!weather || !weather->valid || weather->receipt_hash == 0u ||
        !renderer || !renderer->valid || renderer->renderer_hash == 0u ||
        renderer->command_count == 0u ||
        renderer->command_count > DM2_V1_WEATHER_MAX_SLOTS ||
        !timer_owner || !timer_owner->valid || !timer_owner->outdoor ||
        !timer_owner->scheduled || timer_owner->receipt_hash == 0u) {
        return 0;
    }
    for (i = 0u; i < renderer->command_count; ++i) {
        const DM2_V1_WeatherDrawPlan *draw = &renderer->draws[i];
        const DM2_V1_WeatherCommandReceipt *command;
        unsigned int command_index;

        if (!draw->valid || !dm2_weather_command_is_source_owned(
                draw->image_field)) {
            return 0;
        }
        command_index = (unsigned int)(draw->image_field -
            DM2_V1_WEATHER_BOLT_CMD_BASE);
        if (command_index >= DM2_V1_WEATHER_COMMAND_COUNT) {
            return 0;
        }
        command = &weather->commands[command_index];
        {
            uint32_t command_palette_hash = command->local_palette_hash != 0u
                ? command->local_palette_hash
                : command->palette_translation_hash;
            if (command->command != draw->image_field || !command->material_valid ||
                !command->raw_text || command->byte_count == 0u ||
                command->raw_hash != dm2_weather_hash_bytes(command->raw_text,
                                                             command->byte_count) ||
                !command->image_source_bytes ||
                command->image_source_byte_count == 0u ||
                command->image_raw_hash != dm2_weather_hash_bytes(
                    command->image_source_bytes, command->image_source_byte_count) ||
                command->image_material_receipt_hash == 0u ||
                command->decoded_pixels_hash != draw->decoded_pixels_hash ||
                command->decoded_pixel_count != draw->decoded_pixel_count ||
                command_palette_hash != draw->local_palette_hash ||
                command->palette_translation_hash !=
                    draw->palette_translation_hash ||
                command->palette_translation_count !=
                    draw->palette_translation_count ||
                command->material_hash != draw->material_hash) {
                return 0;
            }
        }
        raw_hash = dm2_weather_hash_step(raw_hash, command->raw_hash);
        raw_hash = dm2_weather_hash_step(raw_hash, command->byte_count);
        raw_hash = dm2_weather_hash_step(raw_hash, command->image_raw_hash);
        raw_hash = dm2_weather_hash_step(raw_hash,
                                         command->image_source_byte_count);
        if (UINT32_MAX - out->raw_material_byte_count < command->byte_count ||
            UINT32_MAX - out->raw_material_byte_count - command->byte_count <
                command->image_source_byte_count) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->raw_material_byte_count += command->byte_count +
            command->image_source_byte_count;
        out->command_mask |= DM2_V1_WEATHER_COMMAND_MASK(command->command);
    }
    out->graphicsset = weather->graphicsset;
    out->timer_owner_hash = timer_owner->receipt_hash;
    out->weather_receipt_hash = weather->receipt_hash;
    out->renderer_hash = renderer->renderer_hash;
    out->raw_material_hash = raw_hash;
    out->command_count = renderer->command_count;
    receipt_hash = dm2_weather_hash_step(receipt_hash, out->timer_owner_hash);
    receipt_hash = dm2_weather_hash_step(receipt_hash, out->weather_receipt_hash);
    receipt_hash = dm2_weather_hash_step(receipt_hash, out->renderer_hash);
    receipt_hash = dm2_weather_hash_step(receipt_hash, out->raw_material_hash);
    receipt_hash = dm2_weather_hash_step(receipt_hash, out->raw_material_byte_count);
    receipt_hash = dm2_weather_hash_step(receipt_hash, out->command_mask);
    out->receipt_hash = receipt_hash;
    out->valid = receipt_hash != 0u;
    return out->valid;
}

int dm2_v1_weather_runtime_admission_receipt(
    const DM2_V1_GraphicsDataOpenReceipt *graphics_open,
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_WeatherRendererReceipt *renderer,
    DM2_V1_WeatherRuntimeAdmissionReceipt *out)
{
    uint32_t text_hash = 2166136261u;
    uint32_t hash = 2166136261u;
    unsigned int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!graphics_open || !graphics_open->valid ||
        graphics_open->admission_hash == 0u ||
        !weather || !weather->valid || weather->receipt_hash == 0u ||
        weather->command_mask == 0u) {
        return 0;
    }

    for (i = 0u; i < sizeof(weather->commands) / sizeof(weather->commands[0]);
         ++i) {
        const DM2_V1_WeatherCommandReceipt *command = &weather->commands[i];

        if (!dm2_weather_command_is_source_owned(command->command) ||
            command->byte_count == 0u || command->raw_hash == 0u ||
            (weather->command_mask &
             DM2_V1_WEATHER_COMMAND_MASK(command->command)) == 0u) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        text_hash = dm2_weather_hash_step(text_hash, command->command);
        text_hash = dm2_weather_hash_step(text_hash, command->raw_hash);
        text_hash = dm2_weather_hash_step(text_hash, command->byte_count);
    }

    out->valid = 1;
    out->graphicsset = weather->graphicsset;
    out->graphics_data_open_hash = graphics_open->admission_hash;
    out->weather_receipt_hash = weather->receipt_hash;
    out->command_mask = weather->command_mask;
    out->material_mask = weather->material_mask;
    out->command_text_hash = text_hash;
    out->source_text_ready = 1;
    out->material_ready = weather->material_mask != 0u;
    out->palette_required = out->material_ready;
    out->no_fallback_blit = 1;
    if (renderer) {
        if (!renderer->valid || renderer->renderer_hash == 0u ||
            renderer->command_count == 0u || !out->material_ready) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->renderer_ready = 1;
        out->renderer_hash = renderer->renderer_hash;
        out->blit_authorized = 1;
    }
    hash = dm2_weather_hash_step(hash, out->graphics_data_open_hash);
    hash = dm2_weather_hash_step(hash, out->weather_receipt_hash);
    hash = dm2_weather_hash_step(hash, out->command_text_hash);
    hash = dm2_weather_hash_step(hash, out->material_mask);
    hash = dm2_weather_hash_step(hash, out->renderer_hash);
    hash = dm2_weather_hash_step(hash, (uint32_t)out->blit_authorized);
    out->admission_hash = hash;
    if (out->admission_hash == 0u) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    return 1;
}

int dm2_v1_weather_distant_environment_receipt(
    const DM2_V1_WeatherGdatReceipt *weather,
    uint8_t command,
    uint8_t slot_index,
    const uint8_t raw[DM2_V1_DISTANT_ENVIRONMENT_BYTES],
    DM2_V1_DistantEnvironmentReceipt *out)
{
    unsigned int index;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!weather || !weather->valid || !raw ||
        slot_index >= DM2_V1_WEATHER_MAX_SLOTS ||
        !dm2_weather_command_is_source_owned(command)) return 0;
    index = (unsigned int)(command - DM2_V1_WEATHER_BOLT_CMD_BASE);
    if (index >= DM2_V1_WEATHER_COMMAND_COUNT ||
        !weather->commands[index].material_valid ||
        weather->commands[index].command != command) return 0;
    /* skproject c_weather.cpp DM2_UPDATE_WEATHER fills cloud then rain in
     * DistantEnvironment ten-byte slots before c_bkgrnd DRAW_TEMP_PICST. */
    out->command = command;
    out->slot_index = slot_index;
    memcpy(out->raw, raw, DM2_V1_DISTANT_ENVIRONMENT_BYTES);
    out->raw_hash = dm2_weather_hash_bytes(out->raw,
                                            DM2_V1_DISTANT_ENVIRONMENT_BYTES);
    if (out->raw_hash == 0u) return 0;
    out->valid = 1;
    return 1;
}

int dm2_v1_weather_timer_transaction_receipt(
    const DM2_V1_WeatherGdatReceipt *weather,
    const uint8_t *timer_bytes, size_t timer_size,
    const uint8_t distant_environment[DM2_V1_DISTANT_ENVIRONMENT_BYTES],
    DM2_V1_WeatherTimerTransactionReceipt *out)
{
    uint8_t command;
    unsigned int index;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!weather || !weather->valid || !timer_bytes || timer_size == 0u ||
        !distant_environment) return 0;
    command = distant_environment[0];
    if (!dm2_weather_command_is_source_owned(command)) return 0;
    index = (unsigned int)(command - DM2_V1_WEATHER_BOLT_CMD_BASE);
    if (index >= DM2_V1_WEATHER_COMMAND_COUNT ||
        !weather->commands[index].material_valid) return 0;
    out->timer_hash = dm2_weather_hash_bytes(timer_bytes, timer_size);
    out->distant_environment_hash = dm2_weather_hash_bytes(
        distant_environment, DM2_V1_DISTANT_ENVIRONMENT_BYTES);
    out->transaction_hash = dm2_weather_hash_step(out->timer_hash,
                                                    out->distant_environment_hash);
    out->proven = out->timer_hash != 0u && out->distant_environment_hash != 0u;
    return out->proven;
}

int dm2_v1_weather_query_rainfall_param_receipt(
    uint8_t rain_intensity,
    uint16_t weather_turn,
    uint16_t party_turn,
    DM2_V1_RainfallParamReceipt *out)
{
    uint16_t delta;
    uint8_t base;
    uint8_t turn = 0u;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    delta = (uint16_t)((weather_turn - party_turn) & 0x3u);
    base = (delta != 0u && delta != 2u) ? 0x6du : 0x71u;
    if (rain_intensity < 0x10u) {
        turn = 1u;
    } else if (rain_intensity < 0x40u) {
        turn = 0u;
    } else if (rain_intensity < 0x80u) {
        turn = 2u;
    } else {
        turn = 3u;
    }
    out->valid = 1;
    out->rain_intensity = rain_intensity;
    out->weather_turn = weather_turn;
    out->party_turn = party_turn;
    out->turn_delta = delta;
    out->image_field = (uint8_t)(base + turn);
    out->mirror_phase = (uint8_t)(delta == 1u || delta == 3u);
    hash = dm2_weather_hash_step(hash, rain_intensity);
    hash = dm2_weather_hash_step(hash, weather_turn);
    hash = dm2_weather_hash_step(hash, party_turn);
    hash = dm2_weather_hash_step(hash, out->image_field);
    hash = dm2_weather_hash_step(hash, out->mirror_phase);
    out->receipt_hash = hash;
    return out->receipt_hash != 0u;
}

int dm2_v1_scene_weather_light_runtime_receipt(
    const DM2_V1_GdatSceneLightM11Receipt *scene_light_receipt,
    const DM2_V1_CLightM11Receipt *c_light_receipt,
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_WeatherRendererReceipt *renderer,
    const DM2_V1_WeatherRuntimeAdmissionReceipt *admission,
    const DM2_V1_EnvironmentWeatherReceipt *environment,
    const DM2_V1_RainfallParamReceipt *rainfall,
    DM2_V1_SceneWeatherLightRuntimeReceipt *out)
{
    uint32_t hash = 2166136261u;
    uint32_t source_hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!scene_light_receipt || !scene_light_receipt->valid ||
        scene_light_receipt->receipt_hash == 0u ||
        scene_light_receipt->scene_control_hash == 0u ||
        !c_light_receipt || !c_light_receipt->valid ||
        c_light_receipt->receipt_hash == 0u ||
        c_light_receipt->graphicsset != scene_light_receipt->graphicsset ||
        c_light_receipt->scene_control_hash !=
            scene_light_receipt->scene_control_hash ||
        !weather || !weather->valid || weather->receipt_hash == 0u ||
        weather->graphicsset != scene_light_receipt->graphicsset ||
        !admission || !admission->valid ||
        admission->admission_hash == 0u ||
        admission->weather_receipt_hash != weather->receipt_hash ||
        admission->graphicsset != weather->graphicsset ||
        !admission->source_text_ready || !admission->no_fallback_blit) {
        return 0;
    }
    out->graphicsset = weather->graphicsset;
    out->scene_light_hash = scene_light_receipt->receipt_hash;
    out->c_light_hash = c_light_receipt->receipt_hash;
    out->weather_receipt_hash = weather->receipt_hash;
    out->weather_admission_hash = admission->admission_hash;
    out->command_mask = weather->command_mask;
    out->material_mask = weather->material_mask;
    out->no_synthetic_weather_fallback = 1;
    if (renderer) {
        if (!renderer->valid || renderer->renderer_hash == 0u ||
            renderer->restored_state.state_hash == 0u ||
            renderer->command_count == 0u ||
            !admission->renderer_ready ||
            admission->renderer_hash != renderer->renderer_hash ||
            !admission->blit_authorized) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->weather_renderer_hash = renderer->renderer_hash;
        out->renderer_command_count = renderer->command_count;
        out->distant_environment_display_bound = 1;
    }
    if (environment) {
        if (!environment->valid || environment->receipt_hash == 0u ||
            environment->graphicsset != weather->graphicsset) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->map_load_token = environment->map_load_token;
        out->environment_receipt_hash = environment->receipt_hash;
        out->environment_material_count = environment->material_count;
    }
    if (rainfall) {
        unsigned int index;
        if (!rainfall->valid || rainfall->receipt_hash == 0u ||
            rainfall->image_field < DM2_V1_WEATHER_RAIN_LIGHT_CMD ||
            rainfall->image_field > 0x74u) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        index = (unsigned int)(rainfall->image_field -
                               DM2_V1_WEATHER_RAIN_LIGHT_CMD);
        if (index < 3u) {
            const DM2_V1_WeatherCommandReceipt *command =
                &weather->commands[6u + index];
            if (command->command != rainfall->image_field ||
                !command->material_valid) {
                memset(out, 0, sizeof(*out));
                return 0;
            }
        }
        out->rainfall_receipt_hash = rainfall->receipt_hash;
        out->draw_rain_bound = 1;
    }
    source_hash = dm2_weather_hash_step(source_hash, 0x44524e45u);
    source_hash = dm2_weather_hash_step(source_hash, 0x44534554u);
    source_hash = dm2_weather_hash_step(source_hash, 0x44524149u);
    source_hash = dm2_weather_hash_step(source_hash, 0x51524149u);
    source_hash = dm2_weather_hash_step(source_hash, 0x41424c54u);
    source_hash = dm2_weather_hash_step(source_hash, 0x434b524cu);
    out->source_symbol_hash = source_hash;
    hash = dm2_weather_hash_step(hash, out->scene_light_hash);
    hash = dm2_weather_hash_step(hash, out->c_light_hash);
    hash = dm2_weather_hash_step(hash, out->weather_receipt_hash);
    hash = dm2_weather_hash_step(hash, out->weather_renderer_hash);
    hash = dm2_weather_hash_step(hash, out->weather_admission_hash);
    hash = dm2_weather_hash_step(hash, out->environment_receipt_hash);
    hash = dm2_weather_hash_step(hash, out->rainfall_receipt_hash);
    hash = dm2_weather_hash_step(hash, out->command_mask);
    hash = dm2_weather_hash_step(hash, out->material_mask);
    hash = dm2_weather_hash_step(hash, out->source_symbol_hash);
    out->receipt_hash = hash;
    out->valid = out->receipt_hash != 0u;
    return out->valid;
}

int dm2_v1_weather_gdat_command_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t graphicsset,
    uint8_t command,
    DM2_V1_WeatherCommandReceipt *out)
{
    const uint8_t *raw;
    size_t size = 0u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!dm2_weather_command_is_source_owned(command)) return 0;

    /* skproject c_weather.cpp: DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW calls
     * QUERY_GDAT_TEXT(0x17, ddat.v1d6c02, command). */
    raw = dm2_v1_asset_load_text_sized(loader,
                                        DM2_GDAT_CATEGORY_ENVIRONMENT,
                                        graphicsset,
                                        command,
                                        &size);
    if (!raw || size == 0u || size > UINT32_MAX) return 0;

    out->command = command;
    out->raw_text = raw;
    out->byte_count = (uint32_t)size;
    out->raw_hash = dm2_weather_hash_bytes(raw, size);
    if (!out->raw_hash || !dm2_weather_source_raw_index(loader, raw, size,
                                                         &out->text_raw_index)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    /* QUERY_GDAT_TEXT decode (2636:0377) for encoded GDAT text; the raw
     * receipt above keeps its identity over the undecoded bytes. */
    (void)dm2_weather_decode_command_text(loader, raw, size,
                                          out->decoded_text,
                                          &out->decoded_text_size,
                                          &out->decoded_text_hash);
    (void)dm2_weather_decode_material(loader, graphicsset, out);
    return 1;
}

int dm2_v1_weather_gdat_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t graphicsset,
                                 DM2_V1_WeatherGdatReceipt *out)
{
    static const uint8_t commands[] = {
        DM2_V1_WEATHER_BOLT_CMD_BASE,
        DM2_V1_WEATHER_BOLT_CMD_BASE + 1u,
        DM2_V1_WEATHER_BOLT_CMD_LAST,
        DM2_V1_WEATHER_CLOUD_LIGHT_CMD,
        DM2_V1_WEATHER_CLOUD_HEAVY_CMD,
        DM2_V1_WEATHER_CLOUD_STORM_CMD,
        DM2_V1_WEATHER_RAIN_LIGHT_CMD,
        DM2_V1_WEATHER_RAIN_HEAVY_CMD,
        DM2_V1_WEATHER_RAIN_STORM_CMD
    };
    uint16_t misty_map;
    uint32_t hash = 2166136261u;
    unsigned int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !dm2_v1_asset_load_word_value(
                       loader,
                       DM2_GDAT_CATEGORY_GRAPHICSSET,
                       graphicsset,
                       DM2_GDAT_GFXSET_MISTY_MAP,
                       &misty_map)) {
        return 0;
    }

    for (i = 0; i < sizeof(commands); ++i) {
        DM2_V1_WeatherCommandReceipt *command = &out->commands[i];
        if (!dm2_v1_weather_gdat_command_receipt(loader,
                                                  graphicsset,
                                                  commands[i],
                                                  command)) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->command_mask |= DM2_V1_WEATHER_COMMAND_MASK(commands[i]);
        if (command->material_valid) {
            out->material_mask |= DM2_V1_WEATHER_COMMAND_MASK(commands[i]);
        }
        hash ^= command->command;
        hash *= 16777619u;
        hash ^= command->raw_hash;
        hash *= 16777619u;
        hash ^= command->byte_count;
        hash *= 16777619u;
        hash ^= command->material_hash;
        hash *= 16777619u;
    }

    out->valid = 1;
    out->graphicsset = graphicsset;
    out->misty_map = misty_map;
    out->receipt_hash = hash;
    return 1;
}
