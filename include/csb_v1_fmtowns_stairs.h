#ifndef FIRESTAFF_CSB_V1_FMTOWNS_STAIRS_H
#define FIRESTAFF_CSB_V1_FMTOWNS_STAIRS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DEFS.H MEDIA720: M645 begins the 18 F31 stair images inside each
 * 40-image wall-set bank (M647). */
int csb_v1_fmtowns_stairs_graphic_index(int wall_set,
                                       int requested_graphic_index,
                                       unsigned int *out_graphic_index);

#ifdef __cplusplus
}
#endif

#endif
