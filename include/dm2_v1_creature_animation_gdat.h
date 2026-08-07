#ifndef FIRESTAFF_DM2_V1_CREATURE_ANIMATION_GDAT_H
#define FIRESTAFF_DM2_V1_CREATURE_ANIMATION_GDAT_H

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"

#include <stddef.h>
#include <stdint.h>

/* SKProject's V5 GET_CREATURE_ANIMATION_FRAME path selects a command row
 * from FB, steps mutable frame state through FC, then reads a directional
 * image id from FD. This decoder does not create or advance that state. */
typedef struct {
    int valid;
    int dynamic;
    uint8_t creature_type;
    uint16_t command;
    uint16_t sequence_offset;
    uint16_t previous_frame;
    uint16_t selected_frame;
    uint8_t direction;
    uint8_t image_id;
    uint32_t table_hash;
} DM2_V1_CreatureAnimationGdatReceipt;

/* Source-owned DM2_1c9a_0958 GDAT fetch.  The caller supplies the exact
 * `sk1c9a02c3::w0/w2` pair returned by DM2_query_1c9a_02c3; this helper does
 * not derive a record, CAII slot, or animation cursor.  It performs the
 * source's DM2_query_4E26(w2) lookup and reads the bounded 0xfc row. */
typedef struct {
    int valid;
    int blocked_missing_gdat;
    int blocked_out_of_range;
    uint8_t creature_type;
    uint16_t animation_base;
    uint16_t timer_word_before;
    uint16_t timer_word_after;
    uint16_t query_index;
    uint32_t blended_value;
    uint8_t frame_bit14;
    int cursor_owner_bound;
    int cursor_static_owner;
    int blocked_record_owner;
    int blocked_caii_owner;
    uint8_t caii_slot;
    uint16_t cursor_w0;
    uint16_t cursor_w2;
} DM2_V1_CreatureAnimation0958Receipt;

int dm2_v1_creature_animation_gdat_query_0958(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    uint16_t animation_base,
    uint16_t *io_timer_word,
    uint32_t game_tick,
    DM2_V1_CreatureAnimation0958Receipt *out_receipt);

/* Source-owned DB4/CAII bridge for DM2_1c9a_0958.  This performs the
 * SKProject `DM2_query_1c9a_02c3(record, aidef)` owner selection from
 * c_querydb.cpp:2978-2990 before entering the bounded 0xfc reader above:
 * static AIDefinition rows use DB4 +8, while live creatures use the
 * authenticated CAII slot selected by DB4 byte@5, then +8.  Missing owner
 * state is rejected rather than replaced with caller-filled cursor words. */
int dm2_v1_creature_animation_gdat_query_0958_record(
    const DM2_V1_AssetLoader *loader,
    uint8_t *creature_record,
    size_t creature_record_size,
    const DM2_AIDefinition *ai_spec,
    uint8_t *caii_slots,
    size_t caii_capacity,
    uint32_t game_tick,
    DM2_V1_CreatureAnimation0958Receipt *out_receipt);

/* Returns zero unless the complete V5 table triad is present and the caller
 * supplies mutable state for a non-static creature. `previous_frame` is the
 * source-owned iAnimInfo value; 0xffff starts the sequence. */
int dm2_v1_creature_animation_gdat_select_dynamic_v5(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    uint16_t command,
    uint16_t previous_frame,
    uint16_t ai_flags,
    int direction,
    DM2_V1_CreatureAnimationGdatReceipt *out_receipt);

#endif /* FIRESTAFF_DM2_V1_CREATURE_ANIMATION_GDAT_H */
