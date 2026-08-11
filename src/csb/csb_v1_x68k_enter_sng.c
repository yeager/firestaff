#include "csb_v1_x68k_enter_sng.h"

#include "csb_v1_x68k_hdm.h"

#include <stdlib.h>
#include <string.h>

static uint16_t be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static int read_vlq(const uint8_t *bytes, size_t bytes_left, size_t *used,
                    uint32_t *out_value) {
    size_t i;
    uint32_t value = 0u;
    if (!bytes || !used || !out_value) return 0;
    for (i = 0u; i < 4u; ++i) {
        uint8_t byte;
        if (i >= bytes_left) return 0;
        byte = bytes[i];
        value = (value << 7) | (uint32_t)(byte & 0x7fu);
        if (!(byte & 0x80u)) {
            *used = i + 1u; *out_value = value; return 1;
        }
    }
    return 0;
}

static int consume_data(const uint8_t *bytes, size_t bytes_left,
                        size_t count) {
    size_t i;
    if (count > bytes_left) return 0;
    for (i = 0u; i < count; ++i)
        if (bytes[i] & 0x80u) return 0;
    return 1;
}

static int parse_track(const uint8_t *bytes, size_t byte_count,
                       CSB_V1_X68kEnterSngReceipt *receipt) {
    size_t at = 0u;
    uint8_t running_status = 0u;
    uint64_t ticks = 0u;
    int ended = 0;

    while (at < byte_count) {
        size_t used;
        uint32_t delta;
        uint8_t status;
        if (ended || !read_vlq(bytes + at, byte_count - at, &used, &delta) ||
            used > byte_count - at || ticks > UINT64_MAX - delta) return 0;
        at += used; ticks += delta;
        if (at >= byte_count) return 0;
        status = bytes[at];
        if (!(status & 0x80u)) {
            if (running_status < 0x80u || running_status >= 0xf0u) return 0;
            status = running_status;
        } else {
            ++at;
            if (status < 0xf0u) running_status = status;
            else running_status = 0u;
        }
        if (receipt->event_count == UINT32_MAX) return 0;
        ++receipt->event_count;
        if (status == 0xffu) {
            uint8_t type;
            uint32_t length;
            if (at >= byte_count) return 0;
            type = bytes[at++];
            if (!read_vlq(bytes + at, byte_count - at, &used, &length) ||
                used > byte_count - at) return 0;
            at += used;
            if ((uint64_t)length > byte_count - at ||
                receipt->meta_event_count == UINT32_MAX) return 0;
            ++receipt->meta_event_count;
            if (type == 0x51u) {
                if (length != 3u || receipt->tempo_event_count == UINT32_MAX) return 0;
                ++receipt->tempo_event_count;
            }
            if (type == 0x2fu) {
                if (length != 0u || receipt->end_of_track_count == UINT32_MAX) return 0;
                ++receipt->end_of_track_count; ended = 1;
            }
            at += length;
        } else if (status == 0xf0u || status == 0xf7u) {
            uint32_t length;
            if (!read_vlq(bytes + at, byte_count - at, &used, &length) ||
                used > byte_count - at) return 0;
            at += used;
            if ((uint64_t)length > byte_count - at ||
                receipt->sysex_event_count == UINT32_MAX) return 0;
            ++receipt->sysex_event_count; at += length;
        } else if (status < 0xf0u) {
            uint8_t kind = (uint8_t)(status >> 4);
            size_t data_count = (kind == 0xcu || kind == 0xdu) ? 1u : 2u;
            if (!consume_data(bytes + at, byte_count - at, data_count) ||
                receipt->channel_event_count == UINT32_MAX) return 0;
            if (kind == 0x9u && bytes[at + 1u] != 0u) {
                if (receipt->note_on_count == UINT32_MAX) return 0;
                ++receipt->note_on_count;
            }
            ++receipt->channel_event_count; at += data_count;
        } else {
            size_t data_count;
            switch (status) {
                case 0xf1u: case 0xf3u: data_count = 1u; break;
                case 0xf2u: data_count = 2u; break;
                case 0xf6u: case 0xf8u: case 0xf9u: case 0xfau:
                case 0xfbu: case 0xfcu: case 0xfdu: case 0xfeu:
                    data_count = 0u; break;
                default: return 0;
            }
            if (!consume_data(bytes + at, byte_count - at, data_count)) return 0;
            at += data_count;
        }
    }
    if (!ended) return 0;
    if (ticks > receipt->longest_track_ticks)
        receipt->longest_track_ticks = ticks;
    return 1;
}

int csb_v1_x68k_enter_sng_probe(const uint8_t *bytes, size_t byte_count,
                                CSB_V1_X68kEnterSngReceipt *out_receipt) {
    CSB_V1_X68kEnterSngReceipt receipt;
    uint16_t track_count, i;
    size_t at = 14u;

    if (!bytes || byte_count < 14u || memcmp(bytes, "MThd", 4u) != 0 ||
        be32(bytes + 4u) != 6u || be16(bytes + 8u) != 1u ||
        !(track_count = be16(bytes + 10u)) || (bytes[12] & 0x80u) ||
        !be16(bytes + 12u)) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.format = 1u; receipt.track_count = track_count;
    receipt.ticks_per_quarter_note = be16(bytes + 12u);
    for (i = 0u; i < track_count; ++i) {
        uint32_t track_bytes;
        if (at > byte_count || byte_count - at < 8u ||
            memcmp(bytes + at, "MTrk", 4u) != 0) return 0;
        track_bytes = be32(bytes + at + 4u); at += 8u;
        if ((uint64_t)track_bytes > byte_count - at ||
            !parse_track(bytes + at, track_bytes, &receipt)) return 0;
        at += track_bytes;
    }
    if (at != byte_count || receipt.end_of_track_count != track_count) return 0;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int csb_v1_x68k_enter_sng_probe_hdm(const uint8_t *hdm, size_t hdm_size,
                                    CSB_V1_X68kEnterSngReceipt *out_receipt) {
    uint8_t *bytes;
    size_t byte_count = 0u;
    int result;
    if (!csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "ENTER.SNG", NULL,
                                            0u, &byte_count, NULL) ||
        !(bytes = (uint8_t *)malloc(byte_count))) return 0;
    result = csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "ENTER.SNG", bytes,
                                                byte_count, &byte_count, NULL) &&
        csb_v1_x68k_enter_sng_probe(bytes, byte_count, out_receipt);
    free(bytes);
    return result;
}
