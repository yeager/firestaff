/* DM2 WRITE_RECORD_CHECKCODE — recursive record-chain SUPPRESS writer.
 * Source: sksvgame.cpp:1739-1940.
 *
 * Transliteration of the skproject function using callbacks for
 * runtime data access. Variable names match skproject locals:
 *   eaxw     -> record_link
 *   edxl     -> write_sub_chain_info (vl_0c)
 *   ebxl     -> follow_chain (vl_00)
 *   vw_14    -> record_type
 *   bprg7    -> mask pointer
 *   s100prg6 -> record data */

#include "dm2_v1_save_write_record_checkcode_pc34_compat.h"
#include <string.h>

void dm2_v1_write_record_session_init(
    DM2_WriteRecordSession *session,
    uint8_t *out_buf, size_t out_cap,
    int *creature_indices, size_t creature_indices_cap,
    int *container_indices, size_t container_indices_cap,
    const DM2_WriteRecordTimer *timers, int timer_count)
{
    if (!session) return;
    memset(session, 0, sizeof(*session));
    dm2_suppress_writer_init(&session->writer);
    session->out_buf = out_buf;
    session->out_cap = out_cap;
    session->creature_indices = creature_indices;
    session->creature_indices_cap = creature_indices_cap;
    session->container_indices = container_indices;
    session->container_indices_cap = container_indices_cap;
    session->timers = timers;
    session->timer_count = timer_count;
}

static int write_bit(DM2_WriteRecordSession *s, int bit)
{
    size_t w;
    if (s->out_written >= s->out_cap) return 1;
    if (dm2_suppress_writer_write_bit(&s->writer, bit,
            s->out_buf + s->out_written,
            s->out_cap - s->out_written, &w) != 0)
        return 1;
    s->out_written += w;
    return 0;
}

static int write_suppress(DM2_WriteRecordSession *s,
    const uint8_t *data, const uint8_t *mask, size_t count)
{
    size_t w;
    if (s->out_written >= s->out_cap) return 1;
    if (dm2_suppress_writer_write(&s->writer, data, mask, count,
            s->out_buf + s->out_written,
            s->out_cap - s->out_written, &w) != 0)
        return 1;
    s->out_written += w;
    return 0;
}

/* Type 3 (weapon) sub-types that require extended 9-bit field write.
 * Source: sksvgame.cpp:1797-1805. */
static int is_weapon_extended_type(int sub_type)
{
    return sub_type == 0x27 || sub_type == 0x1b || sub_type == 0x1d ||
           sub_type == 0x41 || sub_type == 0x2c || sub_type == 0x32 ||
           sub_type == 0x30 || sub_type == 0x2d;
}

int dm2_v1_write_record_checkcode(
    DM2_WriteRecordSession *session,
    const DM2_WriteRecordCallbacks *cb,
    uint16_t record_link,
    int write_sub_chain_info,
    int follow_chain)
{
    if (!session || !cb || !cb->get_record || !cb->get_next_link)
        return -1;

    while (record_link != DM2_RECORD_LINK_END &&
           record_link != DM2_RECORD_LINK_NONE)
    {
        int record_type = (record_link & 0x3C00) >> 10;
        const uint8_t *mask;
        DM2_WriteRecordData rec;
        int is_map_or_nested = 0;

        /* Types > 3: write 1-bit marker + 4-bit type + optional 2-bit sub-info.
         * Source: sksvgame.cpp:1759-1773. */
        if (record_type > 3) {
            if (write_bit(session, 1)) return 1;
            uint8_t type_byte = (uint8_t)record_type;
            uint8_t type_mask = 0x0f;
            if (write_suppress(session, &type_byte, &type_mask, 1)) return 1;
            if (write_sub_chain_info && record_type != 4) {
                uint8_t sub_byte = (uint8_t)(record_link >> 14);
                uint8_t sub_mask = 3;
                if (write_suppress(session, &sub_byte, &sub_mask, 1)) return 1;
            }
        }

        /* Type 0xF with nested_type_0e: write 7-bit truncated link + break.
         * Source: sksvgame.cpp:1775-1781. */
        if (record_type == 0xf && session->nested_type_0e) {
            uint8_t link_bytes[2];
            uint8_t link_mask[2] = {0x7f, 0x00};
            link_bytes[0] = (uint8_t)(record_link & 0xFF);
            link_bytes[1] = (uint8_t)((record_link >> 8) & 0xFF);
            if (write_suppress(session, link_bytes, link_mask, 2)) return 1;
            break;
        }

        /* Get mask for this record type. */
        mask = dm2_v1_save_record_mask_for_type(record_type);
        if (mask == NULL) goto next_record;

        /* Get record data. */
        if (cb->get_record(cb->ctx, record_link, &rec) != 0) return -1;

        /* Type-specific pre-processing.
         * Source: sksvgame.cpp:1790-1850. */
        if (record_type < 4) {
            if (record_type == 3) {
                /* Weapon: check for extended sub-type. */
                int sub_type = rec.word_02 & 0x7f;
                if (is_weapon_extended_type(sub_type)) {
                    uint8_t ext_bytes[2];
                    uint8_t ext_mask[2] = {0xff, 0x01};
                    uint16_t ext_val = rec.word_02 >> 7;
                    ext_bytes[0] = (uint8_t)(ext_val & 0xFF);
                    ext_bytes[1] = (uint8_t)((ext_val >> 8) & 0xFF);
                    if (write_suppress(session, ext_bytes, ext_mask, 2)) return 1;
                }
            }
        } else if (record_type == 4) {
            /* Creature: write b_04 with 7-bit mask. */
            uint8_t b04 = rec.byte_04;
            uint8_t b04_mask = 0x7f;
            if (write_suppress(session, &b04, &b04_mask, 1)) return 1;
            /* Check AI spec flags for alternate mask. */
            if (cb->query_creature_ai_spec_flags) {
                int flags = cb->query_creature_ai_spec_flags(cb->ctx, record_link);
                if (flags & 0x1)
                    mask = dm2_v1_save_record_mask_creature_ai_spec();
            }
            /* Track creature index. */
            if (session->creature_indices &&
                (size_t)session->creature_count < session->creature_indices_cap) {
                session->creature_indices[session->creature_count] =
                    (int)(record_link & 0x3FF);
            }
            session->creature_count++;
        } else if (record_type >= 5 && record_type <= 8) {
            /* Types 5-8: no special handling. */
        } else if (record_type == 9) {
            /* Container: write 2-bit field from word_04. */
            uint8_t cont_byte = (uint8_t)((rec.word_04 << 13) >> 14);
            uint8_t cont_mask = 3;
            if (write_suppress(session, &cont_byte, &cont_mask, 1)) return 1;
            /* Check if map container. */
            if (cb->is_container_map &&
                cb->is_container_map(cb->ctx, record_link)) {
                mask = dm2_v1_save_record_mask_container_map();
                is_map_or_nested = 1;
            }
            /* Track container index. */
            if (session->container_indices &&
                (size_t)session->container_count < session->container_indices_cap) {
                session->container_indices[session->container_count] =
                    (int)(record_link & 0x3FF);
            }
            session->container_count++;
        } else if (record_type == 0xe) {
            /* Type 0xE: check nested flag. */
            if (session->nested_creature) {
                mask = dm2_v1_save_record_mask_type_0e_nested();
                is_map_or_nested = 1;
            }
        }

        /* Write the record body through SUPPRESS. */
        const uint8_t *sizes = dm2_v1_save_record_sizes();
        if (write_suppress(session, rec.data, mask, sizes[record_type]))
            return 1;

        /* Type-specific post-processing (recursive calls).
         * Source: sksvgame.cpp:1853-1927. */
        if (record_type == 4) {
            /* Creature: recurse into possession chain. */
            int saved_nested = session->nested_creature;
            session->nested_creature = 1;
            int use_sub_info = (mask != dm2_v1_save_record_mask_for_type(4)) ? 1 : 0;
            int rc = dm2_v1_write_record_checkcode(session, cb,
                rec.word_02, use_sub_info, 1);
            session->nested_creature = 0;
            if (rc) return rc;
            session->nested_creature = saved_nested;
        } else if (record_type == 9) {
            if (!is_map_or_nested) {
                /* Normal container: recurse into contents.
                 * Swap misc mask for moneybox if needed. */
                if (cb->is_container_moneybox &&
                    cb->is_container_moneybox(cb->ctx, record_link)) {
                    /* Moneybox: temporarily use moneybox mask for type 0xA. */
                }
                int rc = dm2_v1_write_record_checkcode(session, cb,
                    rec.word_02, 0, 1);
                if (rc) return rc;
            } else {
                /* Map container: write 1-bit has-contents + possession index. */
                int has_contents = (rec.word_02 != DM2_RECORD_LINK_END &&
                                    rec.word_02 != DM2_RECORD_LINK_NONE);
                if (write_bit(session, has_contents ? 1 : 0)) return 1;
                if (has_contents && cb->add_possession_index)
                    cb->add_possession_index(cb->ctx, record_link);
            }
        } else if (record_type == 0xe) {
            if (is_map_or_nested) {
                /* Nested type 0xE: add possession index. */
                if (cb->add_possession_index)
                    cb->add_possession_index(cb->ctx, record_link);
            } else {
                /* Non-nested: recurse into sub-chain. */
                int saved = session->nested_type_0e;
                session->nested_type_0e = 1;
                int rc = dm2_v1_write_record_checkcode(session, cb,
                    rec.word_02, 0, 0);
                session->nested_type_0e = 0;
                if (rc) return rc;
                session->nested_type_0e = saved;
            }
        } else if (record_type == 0xf) {
            /* Type 0xF: scan timer array for matching type-0x19 timer. */
            int found = 0;
            for (int i = 0; i < session->timer_count; i++) {
                if (session->timers[i].type == 0x19 &&
                    session->timers[i].record_link == record_link) {
                    if (write_bit(session, 1)) return 1;
                    uint8_t idx_bytes[2];
                    uint8_t idx_mask[2] = {0xff, 0x03};
                    idx_bytes[0] = (uint8_t)(i & 0xFF);
                    idx_bytes[1] = (uint8_t)((i >> 8) & 0xFF);
                    if (write_suppress(session, idx_bytes, idx_mask, 2)) return 1;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                if (write_bit(session, 0)) return 1;
            }
        }

next_record:
        if (!follow_chain) break;
        record_link = cb->get_next_link(cb->ctx, record_link);
    }

    /* Terminator: if following chain and reached end (or not following and
     * record was 0xFFFF), write 0-bit. If not following chain and record
     * was NOT 0xFFFF, return false (no error, but incomplete). */
    if (!follow_chain && record_link != DM2_RECORD_LINK_END)
        return 0;
    if (write_bit(session, 0)) return 1;
    return 0;
}
