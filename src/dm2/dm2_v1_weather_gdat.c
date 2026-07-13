#include "dm2_v1_weather_gdat.h"

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

static int dm2_weather_command_is_source_owned(uint8_t command)
{
    return command >= DM2_V1_WEATHER_CLOUD_LIGHT_CMD &&
           command <= DM2_V1_WEATHER_RAIN_STORM_CMD;
}

static int dm2_weather_text_has_nul(const uint8_t *text, size_t size)
{
    return text && memchr(text, '\0', size) != NULL;
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

static int dm2_weather_decode_material(DM2_V1_WeatherCommandReceipt *out)
{
    int found_cd = 0;
    int found_fw = 0;
    int32_t cd = 0;
    int32_t fw = 0;
    uint32_t hash;

    if (!out || !out->raw_text || out->byte_count == 0u ||
        !dm2_v1_weather_cmdstr_query(out->raw_text, out->byte_count,
                                     "CD", &found_cd, &cd) ||
        !dm2_v1_weather_cmdstr_query(out->raw_text, out->byte_count,
                                     "FW", &found_fw, &fw) ||
        !found_cd || cd <= 0 || cd > UINT16_MAX ||
        (found_fw && (fw < 0 || fw > UINT8_MAX))) {
        return 0;
    }
    out->rect_number = (uint16_t)cd;
    out->flip_mode = found_fw ? (uint8_t)fw : 0u;
    hash = out->raw_hash;
    hash ^= out->rect_number;
    hash *= 16777619u;
    hash ^= out->flip_mode;
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
    index = (unsigned int)(command - DM2_V1_WEATHER_CLOUD_LIGHT_CMD);
    source = &receipt->commands[index];
    /* skproject c_weather.cpp lines 221-266 calls
     * DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW before it advances to the next
     * ten-byte command slot.  Do not turn a bare CMDSTR record into pixels. */
    if (source->command != command || !source->material_valid ||
        source->rect_number == 0u ||
        (receipt->material_mask & DM2_V1_WEATHER_COMMAND_MASK(command)) == 0u) {
        return 0;
    }

    out->commands[out->command_count].command = command;
    out->commands[out->command_count].slot_index = slot_index;
    out->commands[out->command_count].rect_number = source->rect_number;
    out->commands[out->command_count].flip_mode = source->flip_mode;
    out->commands[out->command_count].material_hash = source->material_hash;
    ++out->command_count;
    out->required_mask |= DM2_V1_WEATHER_COMMAND_MASK(command);
    out->material_mask |= DM2_V1_WEATHER_COMMAND_MASK(command);
    *hash = dm2_weather_hash_step(*hash, command);
    *hash = dm2_weather_hash_step(*hash, source->material_hash);
    *hash = dm2_weather_hash_step(*hash, source->rect_number);
    *hash = dm2_weather_hash_step(*hash, source->flip_mode);
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
    (void)dm2_weather_decode_material(out);
    return 1;
}

int dm2_v1_weather_gdat_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t graphicsset,
                                 DM2_V1_WeatherGdatReceipt *out)
{
    static const uint8_t commands[] = {
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
