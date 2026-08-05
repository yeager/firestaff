#include "csb_v1_amiga_titl_dat.h"

#include <string.h>

/* ReDMCSB APPA.C:51-53 invokes SWSH followed by ANIM with FTL_TITL.  The
 * Amiga TITL.DAT item order and VBL durations below are independently
 * catalogued by Greatstone's CSB Amiga 3.1 EN/FR/GE extraction. */

static uint16_t csb_v1_rd16be(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static int csb_v1_amiga_titl_take_record(const uint8_t *data, size_t size,
                                         size_t *offset,
                                         char first_tag, char second_tag,
                                         const uint8_t **payload,
                                         uint16_t *payload_size)
{
    size_t at = *offset;
    uint16_t length;

    if (size - at < 4u || data[at] != (uint8_t)first_tag ||
        data[at + 1u] != (uint8_t)second_tag) {
        return -1;
    }
    length = csb_v1_rd16be(data + at + 2u);
    at += 4u;
    if ((size_t)length > size - at) {
        return -1;
    }
    if (size - at < (size_t)length + 2u) {
        return -1;
    }
    *payload = data + at;
    *payload_size = length;
    /* Each payload is followed by an opaque BE16 trailer.  In particular,
     * it is not the Greatstone display-order number, so retain it as part
     * of the container envelope rather than inventing a meaning for it. */
    *offset = at + length + 2u;
    return 0;
}

int csb_v1_amiga_titl_dat_decode(const uint8_t *data, size_t size,
                                 CSB_V1_AmigaTitlSchedule *out)
{
    const uint8_t *payload;
    uint16_t payload_size;
    size_t offset = 0u;
    uint32_t total;
    uint16_t i;

    if (!data || !out) {
        return -1;
    }
    memset(out, 0, sizeof(*out));

    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'A', 'N', &payload, &payload_size) != 0 ||
        payload_size != 8u ||
        csb_v1_rd16be(payload + 2u) != CSB_V1_AMIGA_TITL_WIDTH ||
        csb_v1_rd16be(payload + 4u) != CSB_V1_AMIGA_TITL_HEIGHT ||
        csb_v1_rd16be(payload + 6u) != CSB_V1_AMIGA_TITL_BIT_DEPTH) {
        return -1;
    }

    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'P', 'L', &payload, &payload_size) != 0 ||
        payload_size != 66u) {
        return -1;
    }
    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'E', 'N', &payload, &payload_size) != 0 ||
        payload_size < 2u) {
        return -1;
    }

    total = csb_v1_rd16be(payload);
    out->initial_duration_vbl = (uint16_t)total;
    for (i = 0u; i < CSB_V1_AMIGA_TITL_DELTA_COUNT; ++i) {
        if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                          'D', 'L',
                                          &payload, &payload_size) != 0 ||
            payload_size < 2u) {
            return -1;
        }
        out->delta_durations_vbl[i] = csb_v1_rd16be(payload);
        total += out->delta_durations_vbl[i];
    }
    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'D', 'O', &payload, &payload_size) != 0 ||
        payload_size != 0u || offset != size) {
        return -1;
    }

    out->width = CSB_V1_AMIGA_TITL_WIDTH;
    out->height = CSB_V1_AMIGA_TITL_HEIGHT;
    out->bit_depth = CSB_V1_AMIGA_TITL_BIT_DEPTH;
    out->delta_count = CSB_V1_AMIGA_TITL_DELTA_COUNT;
    out->total_duration_vbl = total;
    return 0;
}
