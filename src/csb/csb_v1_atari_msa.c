#include "csb_v1_atari_msa.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

enum { MSA_HEADER_BYTES = 10, MSA_RLE_MARKER = 0xe5, FAT12_EOC = 0xff8 };

static uint16_t be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static uint16_t le16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint16_t fat16(const uint8_t *p, int little) {
    return little ? le16(p) : be16(p);
}

static uint32_t fat32(const uint8_t *p, int little) {
    return little ? le32(p) : be32(p);
}

static int msa_decode(const uint8_t *msa, size_t msa_size, uint8_t **out_disk,
                      size_t *out_size, CSB_V1_AtariMsaReceipt *receipt) {
    uint16_t sectors, sides, first, last;
    size_t track_bytes, disk_bytes, cursor = MSA_HEADER_BYTES, track;
    uint8_t *disk;

    if (!msa || !out_disk || !out_size || msa_size < MSA_HEADER_BYTES ||
        be16(msa) != 0x0e0fu) return 0;
    sectors = be16(msa + 2); sides = (uint16_t)(be16(msa + 4) + 1u);
    first = be16(msa + 6); last = be16(msa + 8);
    if (!sectors || sectors > 64u || !sides || sides > 2u || last < first ||
        (size_t)sectors > SIZE_MAX / 512u) return 0;
    track_bytes = (size_t)sectors * 512u;
    if ((size_t)(last - first + 1u) > SIZE_MAX / sides ||
        (size_t)(last - first + 1u) * sides > SIZE_MAX / track_bytes) return 0;
    disk_bytes = (size_t)(last - first + 1u) * sides * track_bytes;
    disk = (uint8_t *)malloc(disk_bytes);
    if (!disk) return 0;
    for (track = 0; track < (size_t)(last - first + 1u) * sides; ++track) {
        const size_t end = (track + 1u) * track_bytes;
        size_t written = track * track_bytes;
        uint16_t packed;
        if (cursor + 2u > msa_size || !(packed = be16(msa + cursor)) ||
            cursor + 2u + packed > msa_size || packed > track_bytes) goto fail;
        cursor += 2u;
        if (packed == track_bytes) {
            memcpy(disk + written, msa + cursor, track_bytes);
            cursor += track_bytes;
            continue;
        }
        while (written < end && cursor < msa_size) {
            uint8_t byte = msa[cursor++];
            if (byte != MSA_RLE_MARKER) disk[written++] = byte;
            else {
                uint16_t count;
                uint8_t value;
                if (cursor + 3u > msa_size) goto fail;
                value = msa[cursor++]; count = be16(msa + cursor); cursor += 2u;
                if (!count || count > end - written) goto fail;
                memset(disk + written, value, count); written += count;
            }
        }
        if (written != end) goto fail;
    }
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->sectors_per_track = sectors; receipt->side_count = sides;
        receipt->first_track = first; receipt->last_track = last;
        receipt->decoded_disk_bytes = (uint32_t)disk_bytes;
    }
    *out_disk = disk; *out_size = disk_bytes; return 1;
fail:
    free(disk); return 0;
}

static int msa_name83(const char *name, uint8_t out[11]) {
    size_t i = 0u, base = 0u, ext = 0u;
    const char *dot;
    if (!name || !name[0]) return 0;
    memset(out, ' ', 11u); dot = strrchr(name, '.');
    if (dot) {
        if (dot == name || dot[1] == '\0') return 0;
        for (i = 0u; name + i < dot; ++i) {
            if (i >= 8u || !isalnum((unsigned char)name[i])) return 0;
            out[base++] = (uint8_t)toupper((unsigned char)name[i]);
        }
        for (i = 0u; dot[1u + i]; ++i) {
            if (i >= 3u || !isalnum((unsigned char)dot[1u + i])) return 0;
            out[8u + ext++] = (uint8_t)toupper((unsigned char)dot[1u + i]);
        }
    } else {
        for (i = 0u; name[i]; ++i) {
            if (i >= 8u || !isalnum((unsigned char)name[i])) return 0;
            out[base++] = (uint8_t)toupper((unsigned char)name[i]);
        }
    }
    return base && (!dot || ext);
}

int csb_v1_atari_msa_probe(const uint8_t *msa, size_t msa_size,
                           CSB_V1_AtariMsaReceipt *out_receipt) {
    uint8_t *disk = NULL;
    size_t disk_size = 0u;
    CSB_V1_AtariMsaReceipt receipt;
    int result = msa_decode(msa, msa_size, &disk, &disk_size, &receipt);
    free(disk);
    if (!result) return 0;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int csb_v1_atari_msa_extract_root_file(const uint8_t *msa, size_t msa_size,
                                       const char *name, uint8_t *out_bytes,
                                       size_t out_capacity, size_t *out_size,
                                       CSB_V1_AtariMsaReceipt *out_receipt) {
    uint8_t wanted[11], *disk = NULL;
    size_t disk_size = 0u, root_offset, root_bytes, data_offset, cluster_bytes;
    uint16_t bps, reserved, fats, root_entries, sectors_per_fat, cluster, next;
    uint8_t sectors_per_cluster, *fat, *entry = NULL;
    uint32_t file_size;
    size_t i, copied = 0u;
    CSB_V1_AtariMsaReceipt receipt;
    int little;

    if (out_size) *out_size = 0u;
    if (!out_size || !msa_name83(name, wanted) ||
        !msa_decode(msa, msa_size, &disk, &disk_size, &receipt) || disk_size < 512u)
        return 0;
    little = be16(disk + 11) != 512u && le16(disk + 11) == 512u;
    bps = fat16(disk + 11, little); sectors_per_cluster = disk[13];
    reserved = fat16(disk + 14, little); fats = disk[16];
    root_entries = fat16(disk + 17, little); sectors_per_fat = fat16(disk + 22, little);
    if (bps != 512u || !sectors_per_cluster || !fats || !root_entries ||
        !sectors_per_fat) goto fail;
    root_offset = ((size_t)reserved + (size_t)fats * sectors_per_fat) * bps;
    root_bytes = (size_t)root_entries * 32u;
    data_offset = root_offset + ((root_bytes + bps - 1u) / bps) * bps;
    cluster_bytes = (size_t)sectors_per_cluster * bps;
    if (root_offset > disk_size || root_bytes > disk_size - root_offset ||
        data_offset > disk_size || cluster_bytes == 0u) goto fail;
    fat = disk + (size_t)reserved * bps;
    for (i = 0u; i < root_entries; ++i) {
        uint8_t *candidate = disk + root_offset + i * 32u;
        if (candidate[0] == 0x00u) break;
        if (candidate[0] != 0xe5u && (candidate[11] & 0x18u) == 0u) receipt.root_file_count++;
        if (candidate[0] != 0xe5u && (candidate[11] & 0x18u) == 0u &&
            memcmp(candidate, wanted, 11u) == 0) entry = candidate;
    }
    if (!entry) goto fail;
    cluster = fat16(entry + 26, little); file_size = fat32(entry + 28, little);
    if (!file_size || cluster < 2u || file_size > disk_size ||
        (out_bytes && out_capacity < file_size)) goto fail;
    while (copied < file_size) {
        size_t offset, take = file_size - copied;
        if (cluster < 2u || cluster >= FAT12_EOC || (size_t)cluster > SIZE_MAX / cluster_bytes)
            goto fail;
        offset = data_offset + (size_t)(cluster - 2u) * cluster_bytes;
        if (offset > disk_size || cluster_bytes > disk_size - offset) goto fail;
        if (take > cluster_bytes) take = cluster_bytes;
        if (out_bytes) memcpy(out_bytes + copied, disk + offset, take);
        copied += take;
        if (copied == file_size) break;
        if ((size_t)cluster + (cluster >> 1) + 1u >= (size_t)sectors_per_fat * bps) goto fail;
        next = (uint16_t)(fat[cluster + (cluster >> 1)] |
            ((uint16_t)fat[cluster + (cluster >> 1) + 1u] << 8));
        next = (cluster & 1u) ? (uint16_t)(next >> 4) : (uint16_t)(next & 0x0fffu);
        if (next == cluster) goto fail;
        cluster = next;
    }
    *out_size = file_size; if (out_receipt) *out_receipt = receipt; free(disk); return 1;
fail:
    free(disk); return 0;
}
