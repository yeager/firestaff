#ifndef FIRESTAFF_REDMCSB_F0692_FILLBOX_F0135_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0692_FILLBOX_F0135_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB FILLBOX.C F0692_FillBox is the source-named F0135 admission
 * route for a caller-owned planar bitmap. The IMAGE3 F0692 variants use a
 * different packed-raster body and deliberately remain separate.
 */
int redmcsb_f0692_fillbox_f0135_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    size_t row_bytes,
    size_t pixel_height,
    const int16_t box[4],
    uint16_t color);

const char *redmcsb_f0692_fillbox_f0135_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0692_FILLBOX_F0135_PC34_COMPAT_H */
