#include "dm1_v1_amiga_swsh.h"

#include "asset_status_m12.h"

#include <string.h>

enum {
    AMIGA_HUNK_DATA = 0x000003eau,
    AMIGA_SWSH_MAX_EXECUTABLE_BYTES = 8 * 1024 * 1024,
    AMIGA_DISK_SECTOR_BYTES = 512,
    AMIGA_DISK_SECTOR_COUNT = 11,
    AMIGA_BOOT_SECTOR_BYTES = 1024
};

static const char g_amiga_disk_copyright_short[] =
    "Copyright (C) 1988 Software Heaven, Inc.";
static const char g_amiga_disk_copyright_long[] =
    "Copyright (C) 1988 Software Heaven, Inc.  All Rights Reserved.  NOT FOR DISTRIBUTION.";
static const char g_amiga_disk_boot_copyright[] =
    "Copyright (C) 1988 Software Heaven Inc. / FTL Games";

typedef struct {
    const char* md5;
    size_t hunkHeaderOffset;
    size_t hunkDataBytes;
    size_t logoOffset;
    size_t paletteOffset;
    size_t soundOffset;
} DM1_AmigaSwshProfile;

/* ReDMCSB Reference/Original A20 binaries. Segment layout differs between
 * A20ED and later builds; bind every in-segment offset to the full file hash. */
static const DM1_AmigaSwshProfile g_profiles[] = {
    {"28d406007e99b0ae8da0c6f7fc7f183b", 4640u, 73404u,
     308u, 200u, 32314u}, /* A20ED */
    {"a0ffbcc7ae8cecac03128ddb32887ef4", 5000u, 73456u,
     360u, 200u, 32366u}, /* A20E */
    {"1038138978975415571a878bb08f54be", 5088u, 73456u,
     360u, 200u, 32366u}, /* A20F */
    {"916cb3cd462e7582d2a39c73e40b3339", 5088u, 73456u,
     360u, 200u, 32366u}, /* A20G */
    {"658db79c6bb87f3ab09eb7005cd91313", 5104u, 73456u,
     360u, 200u, 32366u}  /* A21E */
};

static uint32_t read_be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t read_be16(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static int profile_payload_valid(const uint8_t* data,
                                 const DM1_AmigaSwshProfile* profile) {
    unsigned int i;
    unsigned int colorEvents = 0u;
    unsigned int waitEvents = 0u;
    const uint8_t* palette;
    const uint64_t requiredEnd = (uint64_t)profile->soundOffset +
                                 DM1_V1_AMIGA_SWSH_SOUND_BYTES;
    if (requiredEnd > profile->hunkDataBytes ||
        (uint64_t)profile->logoOffset + DM1_V1_AMIGA_SWSH_PLANAR_BYTES >
            profile->hunkDataBytes ||
        (uint64_t)profile->paletteOffset + DM1_V1_AMIGA_SWSH_PALETTE_BYTES >
            profile->hunkDataBytes) {
        return 0;
    }
    palette = data + profile->paletteOffset;
    for (i = 0u; i < DM1_V1_AMIGA_SWSH_PALETTE_PAIRS; ++i) {
        int paletteIndex = (int)(int16_t)read_be16(palette + i * 4u);
        unsigned int value = read_be16(palette + i * 4u + 2u);
        if (paletteIndex == -1) {
            if (value > 60u) return 0;
            ++waitEvents;
        } else if (paletteIndex >= 0 && paletteIndex < 16 && value <= 0x0fffu) {
            ++colorEvents;
        } else {
            return 0;
        }
    }
    return colorEvents > 0u && waitEvents > 0u;
}

int dm1_v1_amiga_original_disk_check(const uint8_t* adf_bytes,
                                     size_t adf_byte_count,
                                     int allow_boot_sector_fallback) {
    const size_t second_track_offset =
        (size_t)AMIGA_DISK_SECTOR_BYTES * AMIGA_DISK_SECTOR_COUNT;
    const size_t copyright_offset = 52u;
    size_t cursor;
    size_t copyright_index = 0u;
    if (!adf_bytes || adf_byte_count < second_track_offset +
            AMIGA_DISK_SECTOR_BYTES) return 0;
    if (memcmp(adf_bytes + second_track_offset,
               g_amiga_disk_copyright_short,
               sizeof(g_amiga_disk_copyright_short) - 1u) == 0 ||
        memcmp(adf_bytes + second_track_offset,
               g_amiga_disk_copyright_long,
               sizeof(g_amiga_disk_copyright_long) - 1u) == 0) {
        return 1;
    }
    /* MEDIA441 A20E/A20F/A21E falls back to F0907 after a failed sector-11
     * read. F0907 checks the Amiga boot checksum word then cycles the exact
     * ReDMCSB copyright text from byte 52 through the 1 KiB read buffer. */
    if (!allow_boot_sector_fallback ||
        adf_byte_count < AMIGA_BOOT_SECTOR_BYTES ||
        read_be32(adf_bytes + 4u) != UINT32_C(0xe3a6ec5b)) return 0;
    for (cursor = copyright_offset; cursor < AMIGA_BOOT_SECTOR_BYTES;
         ++cursor) {
        if (adf_bytes[cursor] !=
            (uint8_t)g_amiga_disk_boot_copyright[copyright_index]) return 0;
        ++copyright_index;
        if (g_amiga_disk_boot_copyright[copyright_index] == '\0')
            copyright_index = 0u;
    }
    return 1;
}

int dm1_v1_amiga_swsh_parse(const uint8_t* bytes,
                            size_t byteCount,
                            DM1_V1_AmigaSwshAssets* outAssets) {
    char md5[33];
    size_t i;
    const DM1_AmigaSwshProfile* profile = NULL;
    const uint8_t* hunkData;
    if (!outAssets) return 0;
    memset(outAssets, 0, sizeof(*outAssets));
    if (!bytes || byteCount == 0u ||
        byteCount > (size_t)AMIGA_SWSH_MAX_EXECUTABLE_BYTES ||
        !m12_bytes_md5_hex(bytes, byteCount, md5)) {
        return 0;
    }
    for (i = 0u; i < sizeof(g_profiles) / sizeof(g_profiles[0]); ++i) {
        if (strcmp(md5, g_profiles[i].md5) == 0) {
            profile = &g_profiles[i];
            break;
        }
    }
    if (!profile ||
        (uint64_t)profile->hunkHeaderOffset + 8u + profile->hunkDataBytes >
            byteCount ||
        read_be32(bytes + profile->hunkHeaderOffset) != AMIGA_HUNK_DATA ||
        read_be32(bytes + profile->hunkHeaderOffset + 4u) !=
            (uint32_t)(profile->hunkDataBytes / 4u)) {
        return 0;
    }
    hunkData = bytes + profile->hunkHeaderOffset + 8u;
    if (!profile_payload_valid(hunkData, profile)) return 0;
    outAssets->planarLogo = hunkData + profile->logoOffset;
    outAssets->palettePairs = hunkData + profile->paletteOffset;
    outAssets->sound = hunkData + profile->soundOffset;
    outAssets->soundBytes = DM1_V1_AMIGA_SWSH_SOUND_BYTES;
    outAssets->executableMd5 = profile->md5;
    return 1;
}

int dm1_v1_amiga_swsh_decode_logo(const DM1_V1_AmigaSwshAssets* assets,
                                  uint8_t* outIndexed,
                                  size_t outCapacity) {
    unsigned int y;
    unsigned int x;
    if (!assets || !assets->planarLogo || !outIndexed ||
        outCapacity < (size_t)DM1_V1_AMIGA_SWSH_WIDTH *
                      DM1_V1_AMIGA_SWSH_HEIGHT) {
        return 0;
    }
    for (y = 0u; y < DM1_V1_AMIGA_SWSH_HEIGHT; ++y) {
        for (x = 0u; x < DM1_V1_AMIGA_SWSH_WIDTH; ++x) {
            const size_t byteOffset = (size_t)y * 40u + x / 8u;
            const unsigned int bit = 7u - (x & 7u);
            unsigned int color = 0u;
            unsigned int plane;
            for (plane = 0u; plane < 4u; ++plane) {
                const uint8_t byte = assets->planarLogo[
                    (size_t)plane * 8000u + byteOffset];
                color |= (unsigned int)((byte >> bit) & 1u) << plane;
            }
            outIndexed[(size_t)y * DM1_V1_AMIGA_SWSH_WIDTH + x] =
                (uint8_t)color;
        }
    }
    return 1;
}

int dm1_v1_amiga_swsh_palette_event(const DM1_V1_AmigaSwshAssets* assets,
                                     unsigned int eventIndex,
                                     int* outPaletteIndex,
                                     unsigned int* outValue) {
    const uint8_t* row;
    int paletteIndex;
    unsigned int value;
    if (!assets || !assets->palettePairs || !outPaletteIndex || !outValue ||
        eventIndex >= DM1_V1_AMIGA_SWSH_PALETTE_PAIRS) {
        return 0;
    }
    row = assets->palettePairs + eventIndex * 4u;
    paletteIndex = (int)(int16_t)read_be16(row);
    value = read_be16(row + 2u);
    if (paletteIndex != -1 && (paletteIndex < 0 || paletteIndex >= 16)) return 0;
    *outPaletteIndex = paletteIndex;
    *outValue = value;
    return 1;
}
