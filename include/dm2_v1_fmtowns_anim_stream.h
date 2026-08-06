#ifndef FIRESTAFF_DM2_V1_FMTOWNS_ANIM_STREAM_H
#define FIRESTAFF_DM2_V1_FMTOWNS_ANIM_STREAM_H

#include <stddef.h>
#include <stdint.h>

/*
 * Read-only FM Towns TWANIM stream admission.
 *
 * HME-242's TITLE, SWOOSH and END are standalone big-endian record streams,
 * invoked by AUTOEXEC.BAT through the separate Phar Lap P3 TWANIM.EXP
 * player.  These declarations deliberately accept byte buffers only: the
 * selected CD image remains the sole media owner and nothing is unpacked to
 * the host filesystem.
 *
 * Record framing: DMWeb, "Animations" (http://dmweb.free.fr/community/
 * documentation/file-formats/animations/).  The exact startup inventories
 * below were independently checked against Greatstone's HME-242 extraction.
 */

typedef struct {
    int valid;
    uint32_t byte_count;
    uint32_t chunk_count;
    uint16_t width;
    uint16_t height;
    uint16_t bit_depth;
    uint16_t an_trailer;
    uint32_t an_count;
    uint32_t pl_count;
    uint32_t en_count;
    uint32_t dl_count;
    uint32_t sd_count;
    uint32_t br_count;
    uint32_t so_count;
    uint32_t do_count;
    uint32_t fo_count;
    uint32_t ne_count;
    uint32_t bn_count;
} DM2_V1_FmtownsAnimStreamReceipt;

/* Parses every complete record, rejects unknown tags and rejects trailing or
 * truncated bytes.  It does not render pixels; decoding stays unavailable
 * until a source-owned TWANIM execution handoff consumes this receipt. */
int dm2_v1_fmtowns_anim_stream_parse(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimStreamReceipt *out);

/* Strict retail startup-stream identities.  These validate structure after
 * the caller has already established the media's canonical MD5 identity. */
int dm2_v1_fmtowns_anim_stream_is_hme242_swoosh(
    const DM2_V1_FmtownsAnimStreamReceipt *receipt);
int dm2_v1_fmtowns_anim_stream_is_hme242_title(
    const DM2_V1_FmtownsAnimStreamReceipt *receipt);
int dm2_v1_fmtowns_anim_stream_is_hme242_end(
    const DM2_V1_FmtownsAnimStreamReceipt *receipt);

#endif /* FIRESTAFF_DM2_V1_FMTOWNS_ANIM_STREAM_H */
