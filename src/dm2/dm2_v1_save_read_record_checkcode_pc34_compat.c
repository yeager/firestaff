/* DM2 READ_RECORD_CHECKCODE — recursive record-chain SUPPRESS reader.
 * Source: sksvgame.cpp:1476-1738.
 *
 * Inverse of WRITE_RECORD_CHECKCODE. Reads SUPPRESS-encoded record
 * data and reconstructs records via callbacks. Variable names match
 * the writer's skproject locals for cross-reference. */

#include "dm2_v1_save_read_record_checkcode_pc34_compat.h"
#include <string.h>

void dm2_v1_read_record_session_init(
    DM2_ReadRecordSession *session,
    const uint8_t *in_buf, size_t in_size)
{
    if (!session) return;
    memset(session, 0, sizeof(*session));
    dm2_suppress_reader_init(&session->reader, in_buf, in_size);
    session->in_buf = in_buf;
    session->in_size = in_size;
}

static int read_bit(DM2_ReadRecordSession *s, int *out)
{
    return dm2_suppress_reader_read_bit(&s->reader, out);
}

static int read_suppress(DM2_ReadRecordSession *s,
    uint8_t *data, const uint8_t *mask, size_t count)
{
    return dm2_suppress_reader_read(&s->reader, mask, count, data, 0x00);
}

/* sksvgame.cpp::DM2_SUPPRESS_READER(..., argflag=false) starts each byte
 * from *xeaxp and replaces only selected mask bits. Record bodies use that
 * form after type-specific fields have been prepared. The general reader's
 * zero-fill mode models argflag=true, so do the preserving variant here
 * rather than losing c_record state such as container b_04 bits 1..2. */
static int read_suppress_preserving(DM2_ReadRecordSession *s,
                                    uint8_t *data,
                                    const uint8_t *mask,
                                    size_t count)
{
    size_t i;

    if (!s || !data || !mask) return -1;
    for (i = 0u; i < count; ++i) {
        int bit;
        for (bit = 7; bit >= 0; --bit) {
            const uint8_t bit_mask = (uint8_t)(1u << bit);
            if ((mask[i] & bit_mask) == 0u) continue;
            if (s->reader.bits_remaining == 0u) {
                if (!s->reader.data || s->reader.position >= s->reader.size) {
                    return -1;
                }
                s->reader.current_byte =
                    s->reader.data[s->reader.position++];
                s->reader.bits_remaining = 8u;
            }
            if ((s->reader.current_byte & 0x80u) != 0u) {
                data[i] |= bit_mask;
            } else {
                data[i] &= (uint8_t)~bit_mask;
            }
            s->reader.current_byte <<= 1;
            --s->reader.bits_remaining;
        }
    }
    return 0;
}

int dm2_v1_read_record_checkcode(
    DM2_ReadRecordSession *session,
    const DM2_ReadRecordCallbacks *cb,
    uint16_t *owner_link,
    int map_x, int map_y,
    int read_sub_chain_info,
    int follow_chain)
{
    if (!session || !cb || !cb->alloc_record || !cb->set_data ||
        !cb->append_record)
        return -1;

    for (;;) {
        int bit = 0;
        int record_type;
        const uint8_t *mask;
        uint8_t rec_data[16];
        uint16_t record_link;
        uint16_t sub_chain_bits = 0u;
        int is_map_or_nested = 0;

        /* Read continuation bit. 0 = end of chain. */
        if (read_bit(session, &bit)) return 1;
        if (bit == 0) return 0;

        /* Read 4-bit record type.
         * Source: sksvgame.cpp:1489-1500. */
        {
            uint8_t type_byte = 0;
            uint8_t type_mask = 0x0f;
            if (read_suppress(session, &type_byte, &type_mask, 1)) return 1;
            record_type = type_byte & 0x0f;
        }

        /* Source: sksvgame.cpp:839-851 */
        if (read_sub_chain_info == 0) {
            /* no sub_chain_info field */
        } else if (record_type == 4) {
            /* creature: skip sub_chain_info */
        } else {
            uint8_t sub_byte = 0;
            uint8_t sub_mask = 3;
            if (read_suppress(session, &sub_byte, &sub_mask, 1)) return 1;
            /* sksvgame.cpp:857/864: this two-bit placement is carried in
             * the returned record link, not discarded as reader metadata. */
            sub_chain_bits = (uint16_t)((sub_byte & 3u) << 14);
        }

        /* Type 0xF with nested_type_0e: read 7-bit link and return.
         * Source: sksvgame.cpp:852-861. */
        if (record_type == 0xf && session->nested_type_0e) {
            uint8_t link_bytes[2] = {0, 0};
            uint8_t link_mask[2] = {0x7f, 0x00};
            if (read_suppress(session, link_bytes, link_mask, 2)) return 1;
            if (owner_link) {
                *owner_link = (uint16_t)(link_bytes[0] | 0xff80u);
            }
            return 0;
        }

        /* Get mask for this record type. */
        mask = dm2_v1_save_record_mask_for_type(record_type);
        if (mask == NULL) {
            if (!follow_chain) break;
            continue;
        }

        /* Allocate the record. */
        record_link = cb->alloc_record(cb->ctx, record_type);
        if (record_link == 0xFFFE) {
            session->error = 1;
            return -1;
        }
        record_link = (uint16_t)((record_link & 0x3fffu) | sub_chain_bits);

        /* ReDMCSB/SKProject: sksvgame.cpp:864-866 invokes
         * DM2_APPEND_RECORD_TO immediately after allocation.  The callback
         * owns the record's end marker and parent/tile-chain insertion; do
         * not replace either with a Firestaff-private link scheme. */
        if (cb->append_record(cb->ctx, record_link, owner_link,
                              map_x, map_y) != 0) {
            session->error = 1;
            return -1;
        }

        memset(rec_data, 0, sizeof(rec_data));

        /* Type-specific pre-read processing.
         * Source: sksvgame.cpp:1508-1560. */
        if (record_type < 4) {
            if (record_type == 3) {
                /* Weapon: check for extended sub-type via initial mask read. */
            }
        } else if (record_type == 4) {
            /* Creature: read b_04 with 7-bit mask. */
            uint8_t b04 = 0;
            uint16_t ai_flags = 0u;
            uint8_t b04_mask = 0x7f;
            if (read_suppress(session, &b04, &b04_mask, 1)) return 1;
            rec_data[4] = b04;
            /* SKProject SKULLWIN/c_savegame.cpp::DM2_READ_RECORD_CHECKCODE
             * lines 876-881 obtains this from
             * QUERY_CREATURE_AI_SPEC_FLAGS(vl_1c).  Do not silently select
             * the default v1d647f mask when the live CREATURES →
             * CREATURE_AI GDAT lookup is unavailable: every subsequent
             * record in this one SUPPRESS stream would then be misread. */
            if (!cb->query_creature_ai_flags ||
                cb->query_creature_ai_flags(cb->ctx, record_link, b04,
                                            &ai_flags) != 0) {
                session->error = 1;
                return -1;
            }
            if ((ai_flags & 0x0001u) != 0u) {
                mask = dm2_v1_save_record_mask_creature_ai_spec();
            }
            session->creatures_read++;
        } else if (record_type == 9) {
            /* sksvgame.cpp::DM2_IS_CONTAINER_MAP tests c_record b_04
             * directly: ((word@4 & 6) == 2). Preserve the two source bits
             * before selecting the body mask. */
            uint8_t cont_byte = 0;
            uint8_t cont_mask = 3;
            if (read_suppress(session, &cont_byte, &cont_mask, 1)) return 1;
            rec_data[4] = (uint8_t)((cont_byte << 1) & 0x03u);
            if ((rec_data[4] & 0x06u) == 0x02u) {
                mask = dm2_v1_save_record_mask_container_map();
                is_map_or_nested = 1;
                session->map_containers_read++;
            }
            session->containers_read++;
        } else if (record_type == 0xe) {
            if (session->nested_creature) {
                mask = dm2_v1_save_record_mask_type_0e_nested();
                is_map_or_nested = 1;
            }
        }

        /* Read the record body through SUPPRESS. */
        {
            const uint8_t *sizes = dm2_v1_save_record_sizes();
            size_t rec_size = sizes[record_type];
            if (rec_size > 0 && rec_size <= sizeof(rec_data)) {
                if (read_suppress_preserving(session, rec_data, mask,
                                             rec_size)) return 1;
            }
        }

        /* Store the decoded record. */
        {
            const uint8_t *sizes = dm2_v1_save_record_sizes();
            if (cb->set_data(cb->ctx, record_link,
                             rec_data, sizes[record_type]) != 0) {
                session->error = 1;
                return -1;
            }
        }

        session->records_read++;

        /* Type-specific post-read processing (recursive).
         * Source: sksvgame.cpp:1600-1720. */
        if (record_type == 4) {
            /* Creature: recurse into possession chain. */
            int saved = session->nested_creature;
            uint16_t *child_owner = NULL;
            if (!cb->child_owner ||
                cb->child_owner(cb->ctx, record_link, &child_owner) != 0 ||
                !child_owner) return -1;
            *child_owner = 0xfffeu;
            session->nested_creature = 1;
            int rc = dm2_v1_read_record_checkcode(session, cb, child_owner,
                                                   -1, 0, 0, 1);
            session->nested_creature = saved;
            if (rc) return rc;
        } else if (record_type == 9) {
            if (!is_map_or_nested) {
                /* Normal container: recurse into contents. */
                uint16_t *child_owner = NULL;
                if (!cb->child_owner ||
                    cb->child_owner(cb->ctx, record_link, &child_owner) != 0 ||
                    !child_owner) return -1;
                *child_owner = 0xfffeu;
                int rc = dm2_v1_read_record_checkcode(session, cb, child_owner,
                                                       -1, 0, 0, 1);
                if (rc) return rc;
            } else {
                /* Map container: read 1-bit has-contents. */
                int has = 0;
                if (read_bit(session, &has)) return 1;
                if (has) {
                    if (cb->add_possession_index) {
                        cb->add_possession_index(cb->ctx, record_link);
                    }
                    session->possessions_read++;
                }
            }
        } else if (record_type == 0xe) {
            if (is_map_or_nested) {
                if (cb->add_possession_index) {
                    cb->add_possession_index(cb->ctx, record_link);
                }
                session->possessions_read++;
            } else {
                int saved = session->nested_type_0e;
                uint16_t *child_owner = NULL;
                if (!cb->child_owner ||
                    cb->child_owner(cb->ctx, record_link, &child_owner) != 0 ||
                    !child_owner) return -1;
                *child_owner = 0xfffeu;
                session->nested_type_0e = 1;
                int rc = dm2_v1_read_record_checkcode(session, cb, child_owner,
                                                       -1, 0, 0, 0);
                session->nested_type_0e = saved;
                if (rc) return rc;
            }
        } else if (record_type == 0xf) {
            /* Type 0xF: read timer match bit. */
            int found = 0;
            if (read_bit(session, &found)) return 1;
            if (found) {
                uint8_t idx_bytes[2] = {0, 0};
                uint8_t idx_mask[2] = {0xff, 0x03};
                if (read_suppress(session, idx_bytes, idx_mask, 2)) return 1;
            }
        }

        if (!follow_chain) break;
    }

    return 0;
}
