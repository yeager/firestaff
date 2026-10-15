#ifndef DM1_V1_FMTOWNS_LAYOUT_CROP_H
#define DM1_V1_FMTOWNS_LAYOUT_CROP_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH = 224,
       DM1_V1_FMTOWNS_LAYOUT_VIEW_HEIGHT = 136,
       DM1_V1_FMTOWNS_LAYOUT_TRANSPARENT_INDEX = 10 };

typedef struct DM1_V1_FmtownsLayoutCrop {
    int source_x;
    int source_y;
    int destination_x;
    int destination_y;
    int width;
    int height;
} DM1_V1_FmtownsLayoutCrop;

typedef struct DM1_V1_FmtownsWallPanelRoute {
    unsigned int source_graphic_base;
    unsigned int layout_zone;
    unsigned int pc34_zone;
    int source_x;
    int logical_width;
    int flip_horizontally;
} DM1_V1_FmtownsWallPanelRoute;

/* Translate the shared PC34 wall semantic into ReDMCSB's F20 source graphic
 * and layout-696 zone. `flip_walls` is F0128's map/direction parity state. */
int dm1_v1_fmtowns_wall_panel_route(unsigned int pc34_graphic_index,
                                   int flip_walls,
                                   DM1_V1_FmtownsWallPanelRoute *out_route);

/* F0630 resolves a negative-bitmap alias before F0635 places its logical
 * dimensions. Returned source offsets address the full native bitmap. */
int dm1_v1_fmtowns_wall_resolve_crop(
    const DM1_V1_FmtownsWallPanelRoute *route,
    int bitmap_width, int bitmap_height, DM1_V1_FmtownsLayoutCrop *out_crop);

int dm1_v1_fmtowns_wall_blit(
    uint8_t *destination, size_t destination_size, int destination_stride,
    const uint8_t *source, size_t source_size, int source_stride,
    int bitmap_width, int bitmap_height,
    const DM1_V1_FmtownsWallPanelRoute *route, int transparent_index);

/* Resolve the F20E/F20J static G2174 zone records from ReDMCSB COORD.C
 * G3019 (700..717) against a native Towns bitmap's dimensions. */
int dm1_v1_fmtowns_layout_resolve_crop(unsigned int zone_index,
                                      int bitmap_width, int bitmap_height,
                                      DM1_V1_FmtownsLayoutCrop *out_crop);

/* Source-faithful F0132 copy of the resolved crop. All extents and strides
 * are validated before the first destination write. */
int dm1_v1_fmtowns_layout_blit(uint8_t *destination,
                              size_t destination_size,
                              int destination_stride,
                              const uint8_t *source,
                              size_t source_size, int source_stride,
                              int bitmap_width, int bitmap_height,
                              unsigned int zone_index,
                              int transparent_index);

#ifdef __cplusplus
}
#endif

#endif
