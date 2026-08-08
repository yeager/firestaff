#ifndef FIRESTAFF_DM2_V1_CAII_SOURCE_OWNER_H
#define FIRESTAFF_DM2_V1_CAII_SOURCE_OWNER_H

/*
 * dm2_v1_caii_source_owner.h — private, source-derived CAII admission.
 *
 * The c_creature array is runtime state, not an on-disk Dungeon record
 * pool.  In particular DM2_ALLOC_CAII_TO_CREATURE allocates a 0x22-byte
 * entry lazily, after it has an active DB4 record and a c_tim owner.  This
 * object therefore owns only the two real prerequisites which are available
 * at admission time: the executable/GDAT AIDefinition table and the exact
 * DB4 rows in the admitted File_header dungeon.  It deliberately owns no
 * CAII slots, timer queue, movement, combat, or CCM command stream.
 *
 * Source: SKWINSPX/src/v5/dm2data.cpp:1041-1045,
 *         SKWINSPX/src/v4/skcrture.cpp:28-36,
 *         SKULLWIN/c_1c9a.cpp::DM2_ALLOC_CAII_TO_CREATURE (5772-5894).
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_data_tables_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    /* All bytes below are copied from the mounted, hash-admitted profile.
     * No accessor falls back to process-global creature state. */
    DM2_AIDefinition ai_rows[DM2_V1_SOURCE_AI_TABLE_SIZE];
    uint8_t ai_row_loaded[DM2_V1_SOURCE_AI_TABLE_SIZE];
    uint8_t creature_ai_row[DM2_CREATURE_TYPE_COUNT];
    uint8_t creature_ai_row_loaded[DM2_CREATURE_TYPE_COUNT];
    /* DB4 index and byte@4 creature type, in source record-pool order. */
    uint16_t *db4_indices;
    uint8_t *db4_creature_types;
    uint16_t db4_record_count;
    uint16_t creature_binding_count;
    uint16_t ai_row_count;
    uint32_t source_hash;
    /* Always one: CAII capacity depends on the later savegame/timer session
     * owner, so this object must never fabricate an array from DB4 count. */
    int slots_unowned;
} DM2_V1_CaiiSourceOwner;

/* Materializes a private read-only CAII prerequisite owner from one
 * hash-admitted GDAT loader and one fully validated File_header dungeon.
 * Returns 1 only when the complete DB4 pool can be copied. */
int dm2_v1_caii_source_owner_init(DM2_V1_CaiiSourceOwner *owner,
                                  const DM2_V1_AssetLoader *loader,
                                  const DM2_V1_DungeonData *dungeon);
void dm2_v1_caii_source_owner_free(DM2_V1_CaiiSourceOwner *owner);

/* Source equivalent of CREATURES[type].word@5 -> AIDefinition. The row is
 * private to this owner and remains valid until _free(). */
int dm2_v1_caii_source_owner_ai_spec_def(
    const DM2_V1_CaiiSourceOwner *owner, int creature_type,
    const DM2_AIDefinition **out_def);

/* Read the exact DB4 type byte copied from the source record-pool index.
 * This is deliberately not a live creature lookup: map-chain membership and
 * an allocated CAII slot are separate, later ownership requirements. */
int dm2_v1_caii_source_owner_db4_type(const DM2_V1_CaiiSourceOwner *owner,
                                      uint16_t db4_index,
                                      uint8_t *out_creature_type);

#ifdef __cplusplus
}
#endif

#endif
