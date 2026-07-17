#ifndef THERON_V1_PALETTE_RUNTIME_ADMISSION_H
#define THERON_V1_PALETTE_RUNTIME_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_track02_palette_route.h"
#include "theron_v1_world.h"

/* Runtime-owned provenance receipt for an observed VCE store route. The
 * receipt admits the store sequence beside one source-owned indexed surface;
 * it deliberately does not infer that the stores color any bitmap pixels. */
typedef struct {
    int valid;
    int authenticated_palette_route_consumed;
    int runtime_surface_consumed;
    int runtime_palette_admission_allowed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    Theron_RuntimeMediaSurfaceKind surface_kind;
    unsigned int bitmap_route_bit;
    uint16_t bitmap_width;
    uint16_t bitmap_height;
    size_t bitmap_first_raw_offset;
    size_t bitmap_first_user_data_offset;
    uint32_t bitmap_checksum;
    uint16_t vce_index_address;
    uint16_t vce_low_address;
    uint16_t vce_high_address;
    uint8_t vce_index;
    uint8_t vce_low;
    uint8_t vce_high;
    int bitmap_palette_relation_verified;
    int render_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1PaletteRuntimeAdmissionReceipt;

/* Consumes only a completed raw-CD palette-route receipt and a matching
 * raw-source-verified runtime surface. Palette-to-bitmap semantics and all
 * visual output stay blocked. */
int theron_v1_palette_runtime_admit_track02_surface(
    const Theron_V1Track02PaletteRouteReceipt *palette_route,
    const Theron_V1_World *world,
    Theron_RuntimeMediaSurfaceKind surface_kind,
    Theron_V1PaletteRuntimeAdmissionReceipt *out);

#endif
