#include "firestaff_amiga_adf.h"
#include "firestaff_amiga_mfm.h"

#include <stdlib.h>
#include <string.h>

enum { ADF_BLOCK = 512, ADF_WORDS = 128, ADF_FILE_TYPE = 2, ADF_DATA_TYPE = 8,
       ADF_LIST_TYPE = 16, ADF_DATA_BYTES = 488, ADF_POINTERS = 72,
       ADF_TRACKS = 160, ADF_SECTORS_PER_TRACK = 11,
       ADF_UAE_HEADER_BYTES = 12, ADF_UAE_TRACK_BYTES = 12,
       ADF_UAE_MAX_TRACKS = 168, ADF_UAE_MAX_RAW_TRACK_BYTES = 16384 };

/* 0xfffffffd is an AmigaDOS on-disk tag, not an ISO C enum value.  Keeping it
 * typed as uint32_t avoids -Wpedantic failures on Clang, where enumerators
 * are required to fit in int. */
static const uint32_t ADF_FILE_SUBTYPE = UINT32_C(0xfffffffd);

static uint32_t adf_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static const uint8_t *adf_block(const uint8_t *image, const uint8_t *present,
                                size_t blocks, uint32_t number) {
    if (!image || number == 0U || (size_t)number >= blocks) return NULL;
    if (present && !present[number]) return NULL;
    return image + (size_t)number * ADF_BLOCK;
}

static int adf_block_is_missing(const uint8_t *present, size_t blocks,
                                uint32_t number) {
    return present && number != 0U && (size_t)number < blocks &&
           !present[number];
}

typedef struct AdfMfmTrack {
    uint8_t *image;
    uint8_t *present;
    unsigned track;
    int invalid;
} AdfMfmTrack;

static int adf_mfm_sector(const FirestaffAmigaMfmSector *sector, void *opaque) {
    AdfMfmTrack *track = (AdfMfmTrack *)opaque;
    size_t block;
    if (!sector || !track || sector->track != track->track ||
        sector->sector >= ADF_SECTORS_PER_TRACK) {
        if (track) track->invalid = 1;
        return 1;
    }
    block = (size_t)track->track * ADF_SECTORS_PER_TRACK + sector->sector;
    if (track->present[block]) {
        track->invalid = 1;
        return 1;
    }
    memcpy(track->image + block * ADF_BLOCK, sector->payload, ADF_BLOCK);
    track->present[block] = 1U;
    return 0;
}

/* Decode only the bounded UAE-1ADF sector and raw-MFM track forms needed by
 * OFS startup media. This parser is clean-room; absent MFM sectors remain
 * absent in the presence map and are never synthesized as zero sectors. */
static int adf_decode_uae1adf(const uint8_t *container, size_t container_size,
                              uint8_t **out_image, uint8_t **out_present) {
    const size_t image_size = (size_t)ADF_TRACKS * ADF_SECTORS_PER_TRACK * ADF_BLOCK;
    uint8_t *image = NULL, *present = NULL;
    unsigned track_count, track;
    size_t table_end, data_pos;
    int ok = 0;

    if (out_image) *out_image = NULL;
    if (out_present) *out_present = NULL;
    if (!container || !out_image || !out_present || container_size < ADF_UAE_HEADER_BYTES ||
        memcmp(container, "UAE-1ADF", 8U) != 0 ||
        container[8] != 0U || container[9] != 0U)
        return 0;
    track_count = ((unsigned)container[10] << 8) | container[11];
    if (track_count < ADF_TRACKS || track_count > ADF_UAE_MAX_TRACKS)
        return 0;
    table_end = ADF_UAE_HEADER_BYTES + (size_t)track_count * ADF_UAE_TRACK_BYTES;
    if (table_end > container_size) return 0;

    image = (uint8_t *)calloc(1U, image_size);
    present = (uint8_t *)calloc(image_size / ADF_BLOCK, 1U);
    if (!image || !present) goto done;
    data_pos = table_end;

    for (track = 0U; track < track_count; ++track) {
        const uint8_t *entry = container + ADF_UAE_HEADER_BYTES +
                               (size_t)track * ADF_UAE_TRACK_BYTES;
        unsigned flags = ((unsigned)entry[0] << 8) | entry[1];
        unsigned type = ((unsigned)entry[2] << 8) | entry[3];
        uint32_t byte_length = adf_be32(entry + 4U);
        uint32_t bit_count = adf_be32(entry + 8U);
        size_t sectors = (size_t)track * ADF_SECTORS_PER_TRACK;

        if (flags != 0U || byte_length > FIRESTAFF_AMIGA_MFM_MAX_TRACK_BYTES ||
            byte_length > container_size - data_pos ||
            bit_count > (uint64_t)byte_length * 8U)
            goto done;
        if (track >= ADF_TRACKS) {
            if (type != 1U || byte_length != 0U || bit_count != 0U) goto done;
            continue;
        }
        if (type == 0U) {
            if (byte_length != ADF_SECTORS_PER_TRACK * ADF_BLOCK ||
                bit_count != byte_length * 8U)
                goto done;
            memcpy(image + sectors * ADF_BLOCK, container + data_pos, byte_length);
            memset(present + sectors, 1, ADF_SECTORS_PER_TRACK);
        } else if (type == 1U) {
            AdfMfmTrack decoded;
            int found;
            decoded.image = image;
            decoded.present = present;
            decoded.track = track;
            decoded.invalid = 0;
            if (byte_length == 0U && bit_count == 0U) {
                data_pos += byte_length;
                continue;
            }
            if (byte_length == 0U ||
                byte_length > ADF_UAE_MAX_RAW_TRACK_BYTES || bit_count < 64U)
                goto done;
            found = firestaff_amiga_mfm_scan_track(
                container + data_pos, byte_length, bit_count, track,
                ADF_SECTORS_PER_TRACK, adf_mfm_sector, &decoded);
            if (found < 0 || decoded.invalid) goto done;
        } else {
            goto done;
        }
        data_pos += byte_length;
    }
    if (data_pos != container_size || memcmp(image, "DOS\0", 4U) != 0) goto done;
    *out_image = image;
    *out_present = present;
    image = NULL;
    present = NULL;
    ok = 1;
done:
    free(image);
    free(present);
    return ok;
}

static int adf_append_pointers(const uint8_t *image, const uint8_t *present,
                               size_t blocks,
                               const uint8_t *header,
                               uint32_t **out, size_t *count, size_t *capacity) {
    const uint8_t *node = header;
    uint32_t extension = adf_be32(header + 126U * 4U);
    unsigned int guard = 0U;
    for (;;) {
        uint32_t block_count = adf_be32(node + 2U * 4U);
        size_t i;
        if (block_count > ADF_POINTERS || *count + block_count > 65536U) return 0;
        if (*count + block_count > *capacity) {
            size_t next = *capacity ? *capacity * 2U : 128U;
            uint32_t *grown;
            while (next < *count + block_count) next *= 2U;
            grown = (uint32_t *)realloc(*out, next * sizeof(**out));
            if (!grown) return 0;
            *out = grown; *capacity = next;
        }
        for (i = 0U; i < block_count; ++i) {
            uint32_t ptr = adf_be32(node + (ADF_WORDS - 51U - i) * 4U);
            if (!adf_block(image, present, blocks, ptr))
                return adf_block_is_missing(present, blocks, ptr) ? -1 : 0;
            (*out)[(*count)++] = ptr;
        }
        if (extension == 0U) return 1;
        if (++guard > 1024U) return 0;
        node = adf_block(image, present, blocks, extension);
        if (!node)
            return adf_block_is_missing(present, blocks, extension) ? -1 : 0;
        if (adf_be32(node) != ADF_LIST_TYPE ||
            adf_be32(node + 127U * 4U) != ADF_FILE_SUBTYPE ||
            adf_be32(node + 4U) != extension) return 0;
        extension = adf_be32(node + 126U * 4U);
    }
}

static int adf_visit_file(const uint8_t *image, const uint8_t *present,
                          size_t blocks, uint32_t number,
                          FirestaffAmigaAdfFileVisitor visitor, void *user) {
    const uint8_t *header = adf_block(image, present, blocks, number);
    uint32_t bytes; uint32_t *pointers = NULL; size_t count = 0U, capacity = 0U;
    uint8_t *contents; size_t used = 0U, i; char name[32]; unsigned int name_len;
    int rc, pointer_result;
    if (!header || adf_be32(header) != ADF_FILE_TYPE ||
        adf_be32(header + 127U * 4U) != ADF_FILE_SUBTYPE ||
        adf_be32(header + 4U) != number) return 0;
    name_len = header[432U];
    if (name_len == 0U || name_len > 30U) return 0;
    memcpy(name, header + 433U, name_len); name[name_len] = '\0';
    bytes = adf_be32(header + 81U * 4U);
    if (bytes == 0U || bytes > 32U * 1024U * 1024U) {
        free(pointers); return 0;
    }
    pointer_result = adf_append_pointers(image, present, blocks, header,
                                         &pointers, &count, &capacity);
    if (pointer_result <= 0) {
        free(pointers);
        return pointer_result < 0 ? -1 : 0;
    }
    contents = (uint8_t *)malloc(bytes);
    if (!contents) { free(pointers); return 0; }
    for (i = 0U; i < count && used < bytes; ++i) {
        const uint8_t *data = adf_block(image, present, blocks, pointers[i]);
        uint32_t part;
        if (!data) {
            int missing = adf_block_is_missing(present, blocks, pointers[i]);
            free(contents); free(pointers);
            return missing ? -1 : 0;
        }
        if (adf_be32(data) != ADF_DATA_TYPE || adf_be32(data + 4U) != number ||
            adf_be32(data + 8U) != i + 1U || (part = adf_be32(data + 12U)) > ADF_DATA_BYTES ||
            part > bytes - used) { free(contents); free(pointers); return 0; }
        memcpy(contents + used, data + 24U, part); used += part;
    }
    free(pointers);
    if (used != bytes) { free(contents); return 0; }
    rc = visitor ? visitor(name, contents, bytes, user) : 0;
    free(contents);
    return rc < 0 ? -1 : 1;
}

FIRESTAFF_AMIGA_ADF_API int firestaff_amiga_adf_visit_ofs_files(
    const uint8_t *image, size_t image_size,
    FirestaffAmigaAdfFileVisitor visitor, void *user_data) {
    size_t blocks, i; int found = 0;
    uint8_t *decoded_image = NULL, *present = NULL;
    const uint8_t *source = image;
    if (!image) return -1;
    if (image_size >= 8U && memcmp(image, "UAE-1ADF", 8U) == 0) {
        if (!adf_decode_uae1adf(image, image_size, &decoded_image, &present)) return -1;
        source = decoded_image;
        image_size = (size_t)ADF_TRACKS * ADF_SECTORS_PER_TRACK * ADF_BLOCK;
    }
    if (image_size < ADF_BLOCK * 4U || image_size % ADF_BLOCK != 0U ||
        memcmp(source, "DOS\0", 4U) != 0) {
        free(decoded_image);
        free(present);
        return -1;
    }
    blocks = image_size / ADF_BLOCK;
    for (i = 2U; i < blocks; ++i) {
        const uint8_t *block = adf_block(source, present, blocks, (uint32_t)i);
        int result;
        if (!block) continue;
        if (adf_be32(block) != ADF_FILE_TYPE ||
            adf_be32(block + 127U * 4U) != ADF_FILE_SUBTYPE) continue;
        result = adf_visit_file(source, present, blocks, (uint32_t)i, visitor, user_data);
        if (result < 0) {
            free(decoded_image);
            free(present);
            return -1;
        }
        if (result > 0) ++found;
    }
    free(decoded_image);
    free(present);
    return found;
}
