/*
 * Per-game save-byte export/import manifest for M12.
 *
 * This is intentionally narrower than the launcher quick-resume manifest:
 * it wraps actual runtime save bytes, records a versioned manifest, and
 * verifies the payload before import.
 *
 * DM1 source-lock anchor: Firestaff-native DM1 saves mirror ReDMCSB
 * LOADSAVE.C F0433/F0435 continuity by wrapping the serialized world in a
 * small `FSDM1SV1` header (see include/dm1_v1_save_load.h). This gate checks
 * that header, declared byte count, and body CRC before a DM1 payload can
 * cross an export/import boundary.
 */

#include "save_byte_manifest_m12.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DM1_NATIVE_MAGIC "FSDM1SV1"
#define DM1_NATIVE_HEADER_SIZE 64u
#define DM1_NATIVE_FORMAT_VERSION 1u

typedef struct {
    char gameId[M12_SAVE_BYTE_MANIFEST_ID_MAX];
    char formatId[M12_SAVE_BYTE_MANIFEST_ID_MAX];
    char compatibility[M12_SAVE_BYTE_MANIFEST_ID_MAX * 2];
    uint32_t byteCount;
    uint32_t crc32;
} SavePayloadInfo;

static const char* path_basename_m12(const char* path) {
    const char* slash;
    const char* backslash;
    if (!path) return "";
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (backslash && (!slash || backslash > slash)) slash = backslash;
    return slash ? slash + 1 : path;
}

static int file_exists_m12(const char* path) {
    struct stat st;
    return path && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int safe_basename_m12(const char* name) {
    if (!name || !*name) return 0;
    if (strstr(name, "..") != NULL) return 0;
    if (strchr(name, '/') || strchr(name, '\\')) return 0;
    return 1;
}

static uint32_t read_u32_le_m12(const unsigned char* p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint32_t crc32_update_m12(uint32_t crc, const unsigned char* data,
                                 size_t len) {
    size_t i;
    int bit;
    crc ^= 0xFFFFFFFFu;
    for (i = 0; i < len; ++i) {
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit) {
            crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320u) : (crc >> 1);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

static int copy_file_m12(const char* srcPath, const char* dstPath) {
    FILE* src;
    FILE* dst;
    unsigned char buf[8192];
    size_t n;
    int ok = 0;

    if (!srcPath || !dstPath || strcmp(srcPath, dstPath) == 0) return -1;
    src = fopen(srcPath, "rb");
    if (!src) return -1;
    dst = fopen(dstPath, "wb");
    if (!dst) {
        fclose(src);
        return -1;
    }
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            ok = -1;
            break;
        }
    }
    if (ferror(src)) ok = -1;
    if (fclose(dst) != 0) ok = -1;
    fclose(src);
    return ok;
}

static int read_file_bytes_m12(const char* path,
                               unsigned char** outBytes,
                               size_t* outSize) {
    FILE* fp;
    long size;
    unsigned char* bytes;

    if (!path || !outBytes || !outSize) return 0;
    *outBytes = NULL;
    *outSize = 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    bytes = (unsigned char*)malloc((size_t)size ? (size_t)size : 1u);
    if (!bytes) {
        fclose(fp);
        return 0;
    }
    if ((size_t)size > 0 && fread(bytes, 1, (size_t)size, fp) != (size_t)size) {
        free(bytes);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    *outBytes = bytes;
    *outSize = (size_t)size;
    return 1;
}

static int classify_dm1_save_m12(const unsigned char* bytes, size_t size,
                                 SavePayloadInfo* out) {
    uint32_t version;
    uint32_t totalSize;
    uint32_t headerCrc;
    uint32_t bodyCrc;

    if (!bytes || !out || size < DM1_NATIVE_HEADER_SIZE) return 0;
    if (memcmp(bytes, DM1_NATIVE_MAGIC, 8) != 0) return 0;
    version = read_u32_le_m12(bytes + 8);
    totalSize = read_u32_le_m12(bytes + 12);
    headerCrc = read_u32_le_m12(bytes + 16);
    if (version != DM1_NATIVE_FORMAT_VERSION) return 0;
    if (totalSize != (uint32_t)size) return 0;
    if (bytes[39] != 1u) return 0;
    bodyCrc = crc32_update_m12(0, bytes + DM1_NATIVE_HEADER_SIZE,
                               size - DM1_NATIVE_HEADER_SIZE);
    if (bodyCrc != headerCrc) return 0;

    memset(out, 0, sizeof(*out));
    snprintf(out->gameId, sizeof(out->gameId), "dm1");
    snprintf(out->formatId, sizeof(out->formatId), "%s", DM1_NATIVE_MAGIC);
    snprintf(out->compatibility, sizeof(out->compatibility),
             "firestaff-dm1-v1-native");
    out->byteCount = (uint32_t)size;
    out->crc32 = crc32_update_m12(0, bytes, size);
    return 1;
}

static int classify_save_payload_m12(const char* path,
                                     SavePayloadInfo* out) {
    unsigned char* bytes = NULL;
    size_t size = 0;
    int ok = 0;

    if (!read_file_bytes_m12(path, &bytes, &size)) return 0;
    if (classify_dm1_save_m12(bytes, size, out)) ok = 1;
    free(bytes);
    return ok;
}

static int join_path_m12(char* out, int outSize,
                         const char* dir, const char* base) {
    int n;
    if (!out || outSize <= 0 || !dir || !*dir || !base || !*base) return 0;
    n = snprintf(out, (size_t)outSize, "%s/%s", dir, base);
    return n > 0 && n < outSize;
}

static int manifest_dir_m12(const char* manifestPath, char* out, int outSize) {
    const char* slash;
    const char* backslash;
    size_t len;
    if (!manifestPath || !out || outSize <= 0) return 0;
    slash = strrchr(manifestPath, '/');
    backslash = strrchr(manifestPath, '\\');
    if (backslash && (!slash || backslash > slash)) slash = backslash;
    if (!slash) {
        snprintf(out, (size_t)outSize, ".");
        return 1;
    }
    len = (size_t)(slash - manifestPath);
    if (len == 0 || len >= (size_t)outSize) return 0;
    memcpy(out, manifestPath, len);
    out[len] = '\0';
    return 1;
}

static int write_manifest_m12(const char* path,
                              const M12_SaveByteManifest* manifest) {
    FILE* fp;
    if (!path || !manifest) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp, "{\n");
    fprintf(fp, "  \"type\": \"%s\",\n", M12_SAVE_BYTE_MANIFEST_TYPE);
    fprintf(fp, "  \"manifest_version\": %d,\n", manifest->manifestVersion);
    fprintf(fp, "  \"runtime_save_bytes_included\": %d,\n",
            manifest->runtimeSaveBytesIncluded);
    fprintf(fp, "  \"game_id\": \"%s\",\n", manifest->gameId);
    fprintf(fp, "  \"format_id\": \"%s\",\n", manifest->formatId);
    fprintf(fp, "  \"compatibility\": \"%s\",\n", manifest->compatibility);
    fprintf(fp, "  \"source_filename\": \"%s\",\n",
            manifest->sourceFilename);
    fprintf(fp, "  \"byte_count\": %u,\n", manifest->byteCount);
    fprintf(fp, "  \"crc32\": \"%08X\"\n", manifest->crc32);
    fprintf(fp, "}\n");
    return fclose(fp) == 0;
}

static int read_text_file_m12(const char* path, char* out, size_t outSize) {
    FILE* fp;
    size_t n;
    if (!path || !out || outSize == 0) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    n = fread(out, 1, outSize - 1u, fp);
    out[n] = '\0';
    fclose(fp);
    return n > 0;
}

static const char* find_json_value_m12(const char* json, const char* key) {
    char needle[96];
    const char* p;
    if (!json || !key) return NULL;
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    p = strstr(json, needle);
    if (!p) return NULL;
    p = strchr(p + strlen(needle), ':');
    if (!p) return NULL;
    ++p;
    while (*p && isspace((unsigned char)*p)) ++p;
    return p;
}

static int json_read_string_m12(const char* json, const char* key,
                                char* out, size_t outSize) {
    const char* p;
    const char* end;
    size_t len;
    if (!out || outSize == 0) return 0;
    out[0] = '\0';
    p = find_json_value_m12(json, key);
    if (!p || *p != '"') return 0;
    ++p;
    end = strchr(p, '"');
    if (!end) return 0;
    len = (size_t)(end - p);
    if (len >= outSize) return 0;
    memcpy(out, p, len);
    out[len] = '\0';
    return 1;
}

static int json_read_int_m12(const char* json, const char* key, int* out) {
    const char* p;
    if (!out) return 0;
    p = find_json_value_m12(json, key);
    if (!p) return 0;
    *out = (int)strtol(p, NULL, 10);
    return 1;
}

static int json_read_u32_m12(const char* json, const char* key,
                             uint32_t* out) {
    const char* p;
    unsigned long v;
    if (!out) return 0;
    p = find_json_value_m12(json, key);
    if (!p) return 0;
    if (*p == '"') {
        char tmp[32];
        const char* end = strchr(p + 1, '"');
        size_t len;
        if (!end) return 0;
        len = (size_t)(end - (p + 1));
        if (len == 0 || len >= sizeof(tmp)) return 0;
        memcpy(tmp, p + 1, len);
        tmp[len] = '\0';
        v = strtoul(tmp, NULL, 16);
    } else {
        v = strtoul(p, NULL, 10);
    }
    *out = (uint32_t)v;
    return 1;
}

int M12_SaveByteManifest_Read(const char* manifestPath,
                              M12_SaveByteManifest* outManifest) {
    char json[4096];
    char type[96];
    M12_SaveByteManifest m;

    if (!manifestPath || !outManifest) return -1;
    if (!read_text_file_m12(manifestPath, json, sizeof(json))) return -1;
    memset(&m, 0, sizeof(m));
    if (!json_read_string_m12(json, "type", type, sizeof(type)) ||
        strcmp(type, M12_SAVE_BYTE_MANIFEST_TYPE) != 0) {
        return -1;
    }
    if (!json_read_int_m12(json, "manifest_version", &m.manifestVersion) ||
        !json_read_int_m12(json, "runtime_save_bytes_included",
                           &m.runtimeSaveBytesIncluded) ||
        !json_read_string_m12(json, "game_id", m.gameId, sizeof(m.gameId)) ||
        !json_read_string_m12(json, "format_id", m.formatId,
                              sizeof(m.formatId)) ||
        !json_read_string_m12(json, "compatibility", m.compatibility,
                              sizeof(m.compatibility)) ||
        !json_read_string_m12(json, "source_filename", m.sourceFilename,
                              sizeof(m.sourceFilename)) ||
        !json_read_u32_m12(json, "byte_count", &m.byteCount) ||
        !json_read_u32_m12(json, "crc32", &m.crc32)) {
        return -1;
    }
    if (m.manifestVersion != M12_SAVE_BYTE_MANIFEST_VERSION ||
        m.runtimeSaveBytesIncluded != 1 ||
        !safe_basename_m12(m.sourceFilename)) {
        return -1;
    }
    *outManifest = m;
    return 0;
}

int M12_SaveByteManifest_VerifyPayload(const char* manifestPath,
                                       const M12_SaveByteManifest* manifest) {
    char dir[M12_SAVE_BYTE_MANIFEST_PATH_MAX];
    char payload[M12_SAVE_BYTE_MANIFEST_PATH_MAX];
    SavePayloadInfo info;

    if (!manifestPath || !manifest) return -1;
    if (!manifest_dir_m12(manifestPath, dir, (int)sizeof(dir))) return -1;
    if (!join_path_m12(payload, (int)sizeof(payload), dir,
                       manifest->sourceFilename)) {
        return -1;
    }
    if (!classify_save_payload_m12(payload, &info)) return -1;
    if (strcmp(info.gameId, manifest->gameId) != 0 ||
        strcmp(info.formatId, manifest->formatId) != 0 ||
        info.byteCount != manifest->byteCount ||
        info.crc32 != manifest->crc32) {
        return -1;
    }
    return 0;
}

int M12_SaveByteManifest_ExportGameSave(const char* gameId,
                                        const char* savePath,
                                        const char* exportDir,
                                        char* outManifestPath,
                                        int outManifestPathSize,
                                        char* outPayloadPath,
                                        int outPayloadPathSize) {
    const char* base;
    SavePayloadInfo info;
    M12_SaveByteManifest manifest;
    char payloadPath[M12_SAVE_BYTE_MANIFEST_PATH_MAX];
    char manifestPath[M12_SAVE_BYTE_MANIFEST_PATH_MAX];

    if (outManifestPath && outManifestPathSize > 0) outManifestPath[0] = '\0';
    if (outPayloadPath && outPayloadPathSize > 0) outPayloadPath[0] = '\0';
    if (!gameId || !savePath || !exportDir || !*exportDir) return -1;
    if (strcmp(gameId, "dm1") != 0) return -1;
    if (!classify_save_payload_m12(savePath, &info)) return -1;
    if (strcmp(info.gameId, gameId) != 0) return -1;

    base = path_basename_m12(savePath);
    if (!safe_basename_m12(base)) return -1;
    if (!join_path_m12(payloadPath, (int)sizeof(payloadPath), exportDir, base)) {
        return -1;
    }
    if (snprintf(manifestPath, sizeof(manifestPath), "%s/%s.manifest.json",
                 exportDir, base) >= (int)sizeof(manifestPath)) {
        return -1;
    }
    if (copy_file_m12(savePath, payloadPath) != 0) return -1;

    memset(&manifest, 0, sizeof(manifest));
    manifest.manifestVersion = M12_SAVE_BYTE_MANIFEST_VERSION;
    manifest.runtimeSaveBytesIncluded = 1;
    snprintf(manifest.gameId, sizeof(manifest.gameId), "%s", info.gameId);
    snprintf(manifest.formatId, sizeof(manifest.formatId), "%s", info.formatId);
    snprintf(manifest.compatibility, sizeof(manifest.compatibility), "%s",
             info.compatibility);
    snprintf(manifest.sourceFilename, sizeof(manifest.sourceFilename), "%s",
             base);
    manifest.byteCount = info.byteCount;
    manifest.crc32 = info.crc32;
    if (!write_manifest_m12(manifestPath, &manifest)) return -1;

    if (outManifestPath && outManifestPathSize > 0) {
        snprintf(outManifestPath, (size_t)outManifestPathSize, "%s",
                 manifestPath);
    }
    if (outPayloadPath && outPayloadPathSize > 0) {
        snprintf(outPayloadPath, (size_t)outPayloadPathSize, "%s",
                 payloadPath);
    }
    return 0;
}

int M12_SaveByteManifest_ImportGameSave(const char* dataDir,
                                        const char* manifestPath,
                                        char* outImportedPath,
                                        int outImportedPathSize) {
    M12_SaveByteManifest manifest;
    char manifestDir[M12_SAVE_BYTE_MANIFEST_PATH_MAX];
    char payloadPath[M12_SAVE_BYTE_MANIFEST_PATH_MAX];
    char dstPath[M12_SAVE_BYTE_MANIFEST_PATH_MAX];

    if (outImportedPath && outImportedPathSize > 0) outImportedPath[0] = '\0';
    if (!dataDir || !*dataDir || !manifestPath) return -1;
    if (M12_SaveByteManifest_Read(manifestPath, &manifest) != 0) return -1;
    if (M12_SaveByteManifest_VerifyPayload(manifestPath, &manifest) != 0) {
        return -1;
    }
    if (!manifest_dir_m12(manifestPath, manifestDir, (int)sizeof(manifestDir))) {
        return -1;
    }
    if (!join_path_m12(payloadPath, (int)sizeof(payloadPath), manifestDir,
                       manifest.sourceFilename)) {
        return -1;
    }
    if (!join_path_m12(dstPath, (int)sizeof(dstPath), dataDir,
                       manifest.sourceFilename)) {
        return -1;
    }
    if (file_exists_m12(dstPath)) return -1;
    if (copy_file_m12(payloadPath, dstPath) != 0) return -1;
    if (outImportedPath && outImportedPathSize > 0) {
        snprintf(outImportedPath, (size_t)outImportedPathSize, "%s", dstPath);
    }
    return 0;
}
