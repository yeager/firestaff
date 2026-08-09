/* DM2 WRITE_POSSESSION_INDICES — deferred possession link writer.
 * Source: sksvgame.cpp:1684-1713. */

#include "dm2_v1_save_write_possession_indices_pc34_compat.h"

int dm2_v1_write_possession_indices(
    DM2_WriteRecordSession *session,
    const DM2_WritePossessionCallbacks *cb,
    const uint16_t *possession_links,
    int count)
{
    if (!session || !cb || !cb->resolve_possession_index) return -1;
    if (count > 0 && !possession_links) return -1;

    /* DM2_WRITE_POSSESSION_INDICES (sksvgame.cpp:1690-1695) walks the link
     * array forward (`*wptrrg5++`), and so does its reader DM2_2066_062b
     * (:1013-1019) and dm2_v1_read_record_checkcode. Emitting them in
     * reverse bound every 10-bit continuation value to the wrong record on
     * round-trip -- fully reversed when every entry is type 9 or 0xE. */
    for (int i = 0; i < count; i++) {
        uint16_t link = possession_links[i];
        int record_type = (link & 0x3C00) >> 10;

        if (record_type != 9 && record_type != 0xe)
            continue;

        int resolved = cb->resolve_possession_index(cb->ctx, link);
        if (resolved < 0)
            continue;

        uint8_t idx_bytes[2];
        uint8_t idx_mask[2] = {0xff, 0x03};
        idx_bytes[0] = (uint8_t)(resolved & 0xFF);
        idx_bytes[1] = (uint8_t)((resolved >> 8) & 0xFF);

        size_t w;
        if (session->out_written >= session->out_cap) return 1;
        if (dm2_suppress_writer_write(&session->writer,
                idx_bytes, idx_mask, 2,
                session->out_buf + session->out_written,
                session->out_cap - session->out_written, &w) != 0)
            return 1;
        session->out_written += w;
    }

    return 0;
}
