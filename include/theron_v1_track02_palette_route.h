#ifndef THERON_V1_TRACK02_PALETTE_ROUTE_H
#define THERON_V1_TRACK02_PALETTE_ROUTE_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_track02.h"

/* This is deliberately separate from loader intake. It is the narrow
 * original-CD boundary for a future authenticated VCE write trace. */

#define THERON_V1_TRACK02_PALETTE_ROUTE_STORE_COUNT 3u

typedef enum {
    THERON_V1_TRACK02_PALETTE_STORE_INDEX = 0,
    THERON_V1_TRACK02_PALETTE_STORE_LOW,
    THERON_V1_TRACK02_PALETTE_STORE_HIGH
} Theron_V1Track02PaletteStoreKind;

typedef struct {
    int observed_direct_sta;
    uint16_t program_counter;
    uint16_t address;
    uint8_t accumulator;
    Theron_V1Track02PaletteStoreKind kind;
} Theron_V1Track02PaletteStoreReceipt;

/* Owned by the trace parser. This route treats it as an opaque completed
 * provenance gate and never substitutes a synthetic trace. */
typedef struct {
    int trace_authenticated;
    int dynamic_cd_read_authenticated;
    int controller_transition_authenticated;
} Theron_V1Track02PaletteTraceReceipt;

typedef struct {
    const uint8_t *raw_track02;
    size_t raw_track02_bytes;
    const char *raw_track02_md5;
    const Theron_V1Track02PaletteTraceReceipt *trace_receipt;
    const Theron_V1Track02PaletteStoreReceipt *stores;
    size_t store_count;
} Theron_V1Track02PaletteRouteFacts;

typedef struct {
    int accepted;
    int real_cd_verified;
    int render_allowed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint16_t vce_index_address;
    uint16_t vce_low_address;
    uint16_t vce_high_address;
    uint8_t vce_index;
    uint8_t vce_low;
    uint8_t vce_high;
    const char *status;
} Theron_V1Track02PaletteRouteReceipt;

/* Accepts only the exact direct STA index/low/high sequence observed after a
 * completed authenticated Track 02 transaction and an MD5-verified original
 * raw BIN. It does not associate those writes with bytes, a palette table,
 * an indexed bitmap route, or RGB output. render_allowed is always zero. */
int theron_v1_track02_palette_route_validate(
    const Theron_V1Track02PaletteRouteFacts *facts,
    Theron_V1Track02PaletteRouteReceipt *out_receipt);

#endif
