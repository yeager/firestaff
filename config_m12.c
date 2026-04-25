#include "config_m12.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static void m12_copy_string(char* out, size_t outSize, const char* value) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!value) {
        value = "";
    }
    snprintf(out, outSize, "%s", value);
}

static int m12_string_equals(const char* a, const char* b) {
    return a && b && strcmp(a, b) == 0;
}

static void m12_trim(char* text) {
    char* start;
    char* end;
    size_t len;
    if (!text) {
        return;
    }
    start = text;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
        ++start;
    }
    if (start != text) {
        memmove(text, start, strlen(start) + 1U);
    }
    len = strlen(text);
    while (len > 0U) {
        end = &text[len - 1U];
        if (*end != ' ' && *end != '\t' && *end != '\r' && *end != '\n') {
            break;
        }
        *end = '\0';
        --len;
    }
}

static int m12_parse_int(const char* value, int fallback) {
    char* end = NULL;
    long parsed;
    if (!value || value[0] == '\0') {
        return fallback;
    }
    parsed = strtol(value, &end, 10);
    if (!end || *end != '\0') {
        return fallback;
    }
    return (int)parsed;
}

static int m12_starts_with_lang(const char* value, const char* langCode) {
    size_t i;
    if (!value || !langCode) {
        return 0;
    }
    for (i = 0U; langCode[i] != '\0'; ++i) {
        if (value[i] == '\0') {
            return 0;
        }
        if (tolower((unsigned char)value[i]) != tolower((unsigned char)langCode[i])) {
            return 0;
        }
    }
    return value[i] == '\0' || value[i] == '_' || value[i] == '-' || value[i] == '.' || value[i] == '@';
}

int M12_Config_GetAutoLanguageIndex(void) {
    const char* candidates[] = {
        getenv("LC_ALL"),
        getenv("LC_MESSAGES"),
        getenv("LANG")
    };
    size_t i;
    for (i = 0U; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        const char* value = candidates[i];
        if (!value || value[0] == '\0') {
            continue;
        }
        if (m12_starts_with_lang(value, "sv")) {
            return 1;
        }
        if (m12_starts_with_lang(value, "fr")) {
            return 2;
        }
    }
    return 0;
}

/* m12_build_parent_dir, m12_ensure_directory, m12_default_data_dir,
 * m12_default_config_path — replaced by fs_portable_compat. */

static void m12_default_config_path(char* out, size_t outSize) {
    char configDir[FSP_PATH_MAX];
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        if (FSP_JoinPath(out, outSize, configDir, "startup-menu.toml")) {
            return;
        }
    }
    m12_copy_string(out, outSize, "startup-menu.toml");
}

static int m12_read_quoted_value(char* out, size_t outSize, const char* raw) {
    size_t src = 0U;
    size_t dst = 0U;
    if (!out || outSize == 0U || !raw) {
        return 0;
    }
    if (raw[0] != '"') {
        return 0;
    }
    ++src;
    while (raw[src] != '\0') {
        if (raw[src] == '"') {
            out[dst] = '\0';
            return 1;
        }
        if (raw[src] == '\\' && raw[src + 1U] != '\0') {
            ++src;
        }
        if (dst + 1U >= outSize) {
            return 0;
        }
        out[dst++] = raw[src++];
    }
    return 0;
}

static void m12_escape_and_write(FILE* fp, const char* value) {
    const char* p;
    fputc('"', fp);
    for (p = value ? value : ""; *p != '\0'; ++p) {
        if (*p == '\\' || *p == '"') {
            fputc('\\', fp);
        }
        fputc(*p, fp);
    }
    fputc('"', fp);
}

void M12_Config_SetDefaults(M12_Config* config) {
    if (!config) {
        return;
    }
    memset(config, 0, sizeof(*config));
    config->languageIndex = M12_Config_GetAutoLanguageIndex();
    config->languageExplicit = 0;
    config->graphicsIndex = 0;
    config->rendererBackendIndex = 0;
    config->windowModeIndex = 0;
    config->scaleModeIndex = 4;
    config->integerScaling = 1;
    config->scalingFilterIndex = 0;
    config->vsyncIndex = 1;
    FSP_GetDefaultOriginalsDir(config->dataDir, sizeof(config->dataDir));
    m12_default_config_path(config->path, sizeof(config->path));
}

static void m12_parse_line(M12_Config* config, char* line) {
    char* equals;
    char* key;
    char* value;
    char quoted[M12_CONFIG_DATA_DIR_CAPACITY];
    if (!config || !line) {
        return;
    }
    m12_trim(line);
    if (line[0] == '\0' || line[0] == '#') {
        return;
    }
    equals = strchr(line, '=');
    if (!equals) {
        return;
    }
    *equals = '\0';
    key = line;
    value = equals + 1;
    m12_trim(key);
    m12_trim(value);

    if (m12_string_equals(key, "language_index")) {
        config->languageIndex = m12_parse_int(value, config->languageIndex);
        return;
    }
    if (m12_string_equals(key, "language_explicit")) {
        config->languageExplicit = m12_parse_int(value, config->languageExplicit) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "graphics_index") ||
        m12_string_equals(key, "presentation_mode_index")) {
        config->graphicsIndex = m12_parse_int(value, config->graphicsIndex);
        return;
    }
    if (m12_string_equals(key, "renderer_backend_index")) {
        config->rendererBackendIndex = m12_parse_int(value, config->rendererBackendIndex);
        return;
    }
    if (m12_string_equals(key, "window_mode_index")) {
        config->windowModeIndex = m12_parse_int(value, config->windowModeIndex);
        return;
    }
    if (m12_string_equals(key, "scale_mode_index")) {
        config->scaleModeIndex = m12_parse_int(value, config->scaleModeIndex);
        return;
    }
    if (m12_string_equals(key, "integer_scaling")) {
        config->integerScaling = m12_parse_int(value, config->integerScaling) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "scaling_filter_index")) {
        config->scalingFilterIndex = m12_parse_int(value, config->scalingFilterIndex);
        return;
    }
    if (m12_string_equals(key, "vsync_index")) {
        config->vsyncIndex = m12_parse_int(value, config->vsyncIndex);
        return;
    }
    if (strncmp(key, "game_", 5) == 0) {
        int gameIndex = -1;
        char field[64];
        if (sscanf(key, "game_%d_%63s", &gameIndex, field) == 2 &&
            gameIndex >= 0 && gameIndex < M12_CONFIG_GAME_COUNT) {
            if (m12_string_equals(field, "use_patch")) {
                config->gameUsePatch[gameIndex] = m12_parse_int(value, config->gameUsePatch[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "version_index")) {
                config->gameVersionIndex[gameIndex] = m12_parse_int(value, config->gameVersionIndex[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "language_index")) {
                config->gameLanguageIndex[gameIndex] = m12_parse_int(value, config->gameLanguageIndex[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "cheats_enabled")) {
                config->gameCheatsEnabled[gameIndex] = m12_parse_int(value, config->gameCheatsEnabled[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "speed")) {
                config->gameSpeed[gameIndex] = m12_parse_int(value, config->gameSpeed[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "aspect_ratio")) {
                config->gameAspectRatio[gameIndex] = m12_parse_int(value, config->gameAspectRatio[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "resolution")) {
                config->gameResolution[gameIndex] = m12_parse_int(value, config->gameResolution[gameIndex]);
                return;
            }
        }
    }
    if (m12_string_equals(key, "data_dir") &&
        m12_read_quoted_value(quoted, sizeof(quoted), value)) {
        m12_copy_string(config->dataDir, sizeof(config->dataDir), quoted);
    }
}

int M12_Config_Save(const M12_Config* config) {
    char parentDir[M12_CONFIG_PATH_CAPACITY];
    char tmpPath[M12_CONFIG_PATH_CAPACITY + 16];
    FILE* fp;
    if (!config || config->path[0] == '\0') {
        return 0;
    }
    if (FSP_ParentDir(parentDir, sizeof(parentDir), config->path)) {
        if (!FSP_CreateDirectoryRecursive(parentDir)) {
            return 0;
        }
    }
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", config->path);
    fp = fopen(tmpPath, "wb");
    if (!fp) {
        return 0;
    }
    fprintf(fp, "# Firestaff startup menu config\n");
    fprintf(fp, "language_index = %d\n", config->languageIndex);
    fprintf(fp, "language_explicit = %d\n", config->languageExplicit ? 1 : 0);
    fprintf(fp, "presentation_mode_index = %d\n", config->graphicsIndex);
    fprintf(fp, "graphics_index = %d\n", config->graphicsIndex);
    fprintf(fp, "renderer_backend_index = %d\n", config->rendererBackendIndex);
    fprintf(fp, "window_mode_index = %d\n", config->windowModeIndex);
    fprintf(fp, "scale_mode_index = %d\n", config->scaleModeIndex);
    fprintf(fp, "integer_scaling = %d\n", config->integerScaling ? 1 : 0);
    fprintf(fp, "scaling_filter_index = %d\n", config->scalingFilterIndex);
    fprintf(fp, "vsync_index = %d\n", config->vsyncIndex);
    {
        int gi;
        for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
            fprintf(fp, "game_%d_use_patch = %d\n", gi, config->gameUsePatch[gi]);
            fprintf(fp, "game_%d_version_index = %d\n", gi, config->gameVersionIndex[gi]);
            fprintf(fp, "game_%d_language_index = %d\n", gi, config->gameLanguageIndex[gi]);
            fprintf(fp, "game_%d_cheats_enabled = %d\n", gi, config->gameCheatsEnabled[gi]);
            fprintf(fp, "game_%d_speed = %d\n", gi, config->gameSpeed[gi]);
            fprintf(fp, "game_%d_aspect_ratio = %d\n", gi, config->gameAspectRatio[gi]);
            fprintf(fp, "game_%d_resolution = %d\n", gi, config->gameResolution[gi]);
        }
    }
    fputs("data_dir = ", fp);
    m12_escape_and_write(fp, config->dataDir);
    fputc('\n', fp);
    if (fclose(fp) != 0) {
        remove(tmpPath);
        return 0;
    }
    if (rename(tmpPath, config->path) != 0) {
        remove(tmpPath);
        return 0;
    }
    return 1;
}

int M12_Config_Load(M12_Config* config, const char* dataDirOverride) {
    FILE* fp;
    char line[1024];
    int hadExistingFile;
    int shouldSave = 0;
    if (!config) {
        return 0;
    }
    M12_Config_SetDefaults(config);
    if (dataDirOverride && dataDirOverride[0] != '\0') {
        m12_copy_string(config->dataDir, sizeof(config->dataDir), dataDirOverride);
    }
    fp = fopen(config->path, "rb");
    if (!fp) {
        M12_Config_Save(config);
        return 0;
    }
    hadExistingFile = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        m12_parse_line(config, line);
    }
    fclose(fp);
    if (!config->languageExplicit) {
        int autoLanguage = M12_Config_GetAutoLanguageIndex();
        if (config->languageIndex != autoLanguage) {
            config->languageIndex = autoLanguage;
            shouldSave = 1;
        }
    }
    if (dataDirOverride && dataDirOverride[0] != '\0') {
        m12_copy_string(config->dataDir, sizeof(config->dataDir), dataDirOverride);
        shouldSave = 1;
    }
    if (config->dataDir[0] == '\0') {
        FSP_ResolveDataDir(config->dataDir, sizeof(config->dataDir), NULL);
        shouldSave = 1;
    }
    if (shouldSave) {
        M12_Config_Save(config);
    }
    return hadExistingFile;
}

const char* M12_Config_GetPath(const M12_Config* config) {
    if (!config || config->path[0] == '\0') {
        return "startup-menu.toml";
    }
    return config->path;
}
