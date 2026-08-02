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

    for (int i = count - 1; i >= 0; i--) {
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
