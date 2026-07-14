#ifndef FIRESTAFF_DM1_V1_OBJECT_DRAW_ICON_TO_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_DRAW_ICON_TO_SCREEN_PC34_COMPAT_H

/*
 * ReDMCSB OBJECT.C F0037_OBJECT_DrawIconToScreen, PC 3.4 route.
 *
 * F0037 receives an object-icon index and its screen origin, obtains the
 * icon bitmap through F0036, constructs the icon box, then sends it to the
 * screen blitter. This host boundary retains that order without inventing an
 * icon atlas: the active asset runtime supplies the F0036 lookup result.
 */

typedef const unsigned char *(*DM1_V1_ObjectIconLookupPc34)(
    void *context,
    int icon_index,
    int *out_row_bytes);

typedef struct {
    DM1_V1_ObjectIconLookupPc34 lookup_icon;
    void *lookup_context;
    unsigned char *screen_pixels;
    int screen_width;
    int screen_height;
    int screen_row_bytes;
    int transparent_color;
} DM1_V1_ObjectDrawIconSurfacePc34;

enum {
    DM1_V1_OBJECT_ICON_WIDTH_PC34 = 16,
    DM1_V1_OBJECT_ICON_HEIGHT_PC34 = 16
};

/*
 * Runs F0037 against an already-materialized PC34 icon asset. The full
 * 16x16 destination box must fit the supplied screen; a failed lookup or an
 * invalid destination is rejected before any screen byte changes.
 */
int dm1_v1_object_draw_icon_to_screen_f0037_pc34(
    const DM1_V1_ObjectDrawIconSurfacePc34 *surface,
    int icon_index,
    int x,
    int y);

#endif
