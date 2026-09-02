/*
 * DM1 V1 ceiling-pit viewport draw helper.
 *
 * Source-locked to ReDMCSB DUNVIEW.C:
 *   F0112_DUNGEONVIEW_DrawCeilingPit lines 4341-4470
 *   D2/D1/D0 ceiling-pit call sites lines 7023-7029, 7215-7221,
 *   7359-7365, 7527-7533, 7695-7701, 7928-7934 and 8003-8292
 *   Graphic558 D2 frames lines 568-570
 *   DEFS.H ceiling-pit graphic ids lines 2220-2222, 2248-2250
 *   DEFS.H ceiling-pit zone ids lines 4187-4189, 4211-4213
 */

#ifndef FIRESTAFF_DM1_V1_CEILING_PIT_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CEILING_PIT_VIEWPORT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CEILING_PIT_TRANSPARENT_PC34 10

#define DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34 63
#define DM1_V1_GRAPHIC_CEILING_PIT_D2C_PC34 64
#define DM1_V1_GRAPHIC_CEILING_PIT_D2L_I34E 64
#define DM1_V1_GRAPHIC_CEILING_PIT_D2C_I34E 65
#define DM1_V1_GRAPHIC_CEILING_PIT_D1L_PC34 65
#define DM1_V1_GRAPHIC_CEILING_PIT_D1C_PC34 66
#define DM1_V1_GRAPHIC_CEILING_PIT_D0L_PC34 67
#define DM1_V1_GRAPHIC_CEILING_PIT_D0C_PC34 68
#define DM1_V1_GRAPHIC_CEILING_PIT_D1L_I34E 66
#define DM1_V1_GRAPHIC_CEILING_PIT_D1C_I34E 67
#define DM1_V1_GRAPHIC_CEILING_PIT_D0L_I34E 68
#define DM1_V1_GRAPHIC_CEILING_PIT_D0C_I34E 69

#define DM1_V1_ZONE_CEILING_PIT_D2L_PC34 862
#define DM1_V1_ZONE_CEILING_PIT_D2C_PC34 863
#define DM1_V1_ZONE_CEILING_PIT_D2R_PC34 864
#define DM1_V1_ZONE_CEILING_PIT_D2L_I34E 864
#define DM1_V1_ZONE_CEILING_PIT_D2C_I34E 865
#define DM1_V1_ZONE_CEILING_PIT_D2R_I34E 866
#define DM1_V1_ZONE_CEILING_PIT_D1L_PC34 865
#define DM1_V1_ZONE_CEILING_PIT_D1C_PC34 866
#define DM1_V1_ZONE_CEILING_PIT_D1R_PC34 867
#define DM1_V1_ZONE_CEILING_PIT_D0L_PC34 868
#define DM1_V1_ZONE_CEILING_PIT_D0C_PC34 869
#define DM1_V1_ZONE_CEILING_PIT_D0R_PC34 870
#define DM1_V1_ZONE_CEILING_PIT_D1L_I34E 867
#define DM1_V1_ZONE_CEILING_PIT_D1C_I34E 868
#define DM1_V1_ZONE_CEILING_PIT_D1R_I34E 869
#define DM1_V1_ZONE_CEILING_PIT_D0L_I34E 870
#define DM1_V1_ZONE_CEILING_PIT_D0C_I34E 871
#define DM1_V1_ZONE_CEILING_PIT_D0R_I34E 872

typedef struct DM1V1CeilingPitViewportRectPc34 {
    int x;
    int y;
    int width;
    int height;
} DM1V1CeilingPitViewportRectPc34;

/* F0112's numeric graphic and zone ids overlap between the MEDIA508 and
 * MEDIA720 families.  Callers that render D1/D0 must identify the decoded
 * media family; inferring it from an overlapping (graphic, zone) pair would
 * select a real but wrong source rectangle. */
enum {
    DM1_V1_CEILING_PIT_PROFILE_PC34 = 0, /* MEDIA508 C063..C068 */
    DM1_V1_CEILING_PIT_PROFILE_I34E = 1  /* MEDIA720 C064..C069 */
};

const DM1V1CeilingPitViewportRectPc34 *
dm1_v1_ceiling_pit_viewport_rect_for_profile_pc34(int media_profile,
                                                   int ceiling_pit_graphic,
                                                   int zone,
                                                   int parity_flag);

const DM1V1CeilingPitViewportRectPc34 *
dm1_v1_ceiling_pit_viewport_rect_pc34(int ceiling_pit_graphic,
                                      int zone,
                                      int parity_flag);

int dm1_v1_ceiling_pit_viewport_draw_pc34(int ceiling_pit_graphic,
                                          int zone,
                                          int map_x,
                                          int map_y,
                                          int parity_flag,
                                          unsigned char *dest,
                                          int dest_width,
                                          int dest_height,
                                          int dest_stride,
                                          const unsigned char *source,
                                          int source_width,
                                          int source_height);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CEILING_PIT_VIEWPORT_PC34_COMPAT_H */
