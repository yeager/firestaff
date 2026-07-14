#ifndef FIRESTAFF_DM2_V1_CREATURE_ANIMATION_GDAT_H
#define FIRESTAFF_DM2_V1_CREATURE_ANIMATION_GDAT_H

#include "dm2_v1_asset_loader.h"

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
