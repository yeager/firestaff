/* Private real-media CAII prerequisite owner; see its public header. */

#include "dm2_v1_caii_source_owner.h"

#include "dm2_v1_data_tables_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

static uint32_t hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t count)
{
    size_t i;
    if (!bytes) return 0u;
    for (i = 0u; i < count; ++i) hash = hash_step(hash, bytes[i]);
    return hash;
}

static void decode_ai_definition(const int8_t raw[36], DM2_AIDefinition *out)
{
    uint8_t bytes[36];
    size_t i;
    if (!raw || !out) return;
    for (i = 0u; i < sizeof(bytes); ++i) bytes[i] = (uint8_t)raw[i];
    out->w0AIFlags = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    out->ArmorClass = bytes[2]; out->b3 = (int8_t)bytes[3];
    out->BaseHP = (uint16_t)bytes[4] | ((uint16_t)bytes[5] << 8);
    out->AttackStrength = bytes[6]; out->PoisonDamage = bytes[7];
    out->Defense = bytes[8]; out->b9x = bytes[9];
    out->w10 = (uint16_t)bytes[10] | ((uint16_t)bytes[11] << 8);
    out->w12 = (uint16_t)bytes[12] | ((uint16_t)bytes[13] << 8);
    out->AttacksSpells = (uint16_t)bytes[14] | ((uint16_t)bytes[15] << 8);
    out->w16 = (uint16_t)bytes[16] | ((uint16_t)bytes[17] << 8);
    out->w18 = (uint16_t)bytes[18] | ((uint16_t)bytes[19] << 8);
    out->w20 = (uint16_t)bytes[20] | ((uint16_t)bytes[21] << 8);
    out->w22 = (uint16_t)bytes[22] | ((uint16_t)bytes[23] << 8);
    out->w24 = (uint16_t)bytes[24] | ((uint16_t)bytes[25] << 8);
    out->w26 = (uint16_t)bytes[26] | ((uint16_t)bytes[27] << 8);
    out->b28 = bytes[28]; out->Weight = bytes[29];
    out->w30 = (uint16_t)bytes[30] | ((uint16_t)bytes[31] << 8);
    out->w32 = (uint16_t)bytes[32] | ((uint16_t)bytes[33] << 8);
    out->b34 = bytes[34]; out->b35 = bytes[35];
}

static int import_ai_bindings(DM2_V1_CaiiSourceOwner *owner,
                              const DM2_V1_AssetLoader *loader)
{
    int type;
    for (type = 0; type < DM2_V1_SOURCE_AI_TABLE_SIZE; ++type) {
        decode_ai_definition(dm2_v1_table_1d296c[type], &owner->ai_rows[type]);
        owner->ai_row_loaded[type] = 1u;
        ++owner->ai_row_count;
    }
    for (type = 0; type < DM2_CREATURE_TYPE_COUNT; ++type) {
        uint16_t row = 0u;
        uint16_t probe = 0u;
        int field;
        if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_CREATURES,
                                           type, 0x05, &row)) {
            /* skcrture.cpp accepts GDAT's missing value as zero only for
             * actual extended GDAT containers, never a host callback. */
            if (loader->gdat_version != 2u && loader->gdat_version != 4u &&
                loader->gdat_version != 5u) continue;
            row = 0u;
        }
        if (row >= DM2_V1_SOURCE_AI_TABLE_SIZE) continue;
        /* EXTENDED_LOAD_AI_DEFINITION overrides a row only when field 4 is
         * a nonzero source value. Reuse exactly that admission gate. */
        if (dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_CREATURE_AI,
                                         row, 4, &probe) && probe != 0u) {
            uint8_t raw[36];
            memset(raw, 0, sizeof(raw));
            for (field = 0; field < (int)sizeof(raw); ++field) {
                uint16_t value = 0u;
                if (dm2_v1_asset_load_word_value(loader,
                        DM2_GDAT_CATEGORY_CREATURE_AI, row, field, &value)) {
                    raw[field] = (uint8_t)value;
                }
            }
            /* The layout decoder is intentionally local: this owner is not
             * allowed to borrow or update the global test/fixture table. */
            {
                int8_t signed_raw[36];
                for (field = 0; field < (int)sizeof(raw); ++field)
                    signed_raw[field] = (int8_t)raw[field];
                decode_ai_definition(signed_raw, &owner->ai_rows[row]);
            }
        }
        owner->creature_ai_row[type] = (uint8_t)row;
        owner->creature_ai_row_loaded[type] = 1u;
        ++owner->creature_binding_count;
    }
    return owner->creature_binding_count > 0u;
}

int dm2_v1_caii_source_owner_init(DM2_V1_CaiiSourceOwner *owner,
                                  const DM2_V1_AssetLoader *loader,
                                  const DM2_V1_DungeonData *dungeon)
{
    DM2_V1_RecordPoolSet pools;
    DM2_V1_CaiiSourceOwner candidate;
    const DM2_V1_RecordPool *db4;
    uint32_t hash = 2166136261u;
    int index;

    if (!owner) return 0;
    memset(owner, 0, sizeof(*owner));
    memset(&candidate, 0, sizeof(candidate));
    memset(&pools, 0, sizeof(pools));
    if (!loader || !loader->loaded || !dungeon || !dungeon->raw_data ||
        dungeon->raw_size <= 0 || !dungeon->record_graph_complete ||
        !dm2_v1_record_pool_set_init_from_dungeon(&pools, dungeon)) goto fail;
    db4 = &pools.pools[4];
    if (!pools.valid || !pools.record_graph_complete || db4->record_size < 5 ||
        db4->record_count <= 0 || db4->record_count > UINT16_MAX || !db4->bytes)
        goto fail;
    candidate.db4_indices = (uint16_t *)calloc((size_t)db4->record_count,
                                                sizeof(*candidate.db4_indices));
    candidate.db4_creature_types = (uint8_t *)calloc((size_t)db4->record_count,
                                                      sizeof(*candidate.db4_creature_types));
    if (!candidate.db4_indices || !candidate.db4_creature_types ||
        !import_ai_bindings(&candidate, loader)) goto fail;
    candidate.db4_record_count = (uint16_t)db4->record_count;
    for (index = 0; index < db4->record_count; ++index) {
        const uint8_t *record = db4->bytes + (size_t)index * (size_t)db4->record_size;
        candidate.db4_indices[index] = (uint16_t)index;
        candidate.db4_creature_types[index] = record[4];
        hash = hash_step(hash, (uint32_t)index);
        hash = hash_step(hash, record[4]);
    }
    hash = hash_bytes(hash, (const uint8_t *)candidate.ai_rows,
                      sizeof(candidate.ai_rows));
    hash = hash_bytes(hash, candidate.creature_ai_row,
                      sizeof(candidate.creature_ai_row));
    hash = hash_bytes(hash, candidate.creature_ai_row_loaded,
                      sizeof(candidate.creature_ai_row_loaded));
    if (hash == 0u) goto fail;
    candidate.source_hash = hash;
    candidate.slots_unowned = 1;
    candidate.valid = 1;
    dm2_v1_record_pool_set_free(&pools);
    *owner = candidate;
    return 1;
fail:
    dm2_v1_record_pool_set_free(&pools);
    dm2_v1_caii_source_owner_free(&candidate);
    return 0;
}

int dm2_v1_caii_source_owner_clone(DM2_V1_CaiiSourceOwner *out,
                                   const DM2_V1_CaiiSourceOwner *source)
{
    DM2_V1_CaiiSourceOwner candidate;
    size_t count;

    if (!out || !source || !source->valid || source->db4_record_count == 0u ||
        !source->db4_indices || !source->db4_creature_types ||
        source->source_hash == 0u) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&candidate, 0, sizeof(candidate));
    candidate = *source;
    candidate.db4_indices = NULL;
    candidate.db4_creature_types = NULL;
    count = (size_t)source->db4_record_count;
    candidate.db4_indices = malloc(count * sizeof(*candidate.db4_indices));
    candidate.db4_creature_types = malloc(
        count * sizeof(*candidate.db4_creature_types));
    if (!candidate.db4_indices || !candidate.db4_creature_types) {
        dm2_v1_caii_source_owner_free(&candidate);
        return 0;
    }
    memcpy(candidate.db4_indices, source->db4_indices,
           count * sizeof(*candidate.db4_indices));
    memcpy(candidate.db4_creature_types, source->db4_creature_types,
           count * sizeof(*candidate.db4_creature_types));
    *out = candidate;
    return 1;
}

void dm2_v1_caii_source_owner_free(DM2_V1_CaiiSourceOwner *owner)
{
    if (!owner) return;
    free(owner->db4_indices);
    free(owner->db4_creature_types);
    memset(owner, 0, sizeof(*owner));
}

int dm2_v1_caii_source_owner_ai_spec_def(const DM2_V1_CaiiSourceOwner *owner,
                                         int creature_type,
                                         const DM2_AIDefinition **out_def)
{
    uint8_t row;
    if (!out_def) return 0;
    *out_def = NULL;
    if (!owner || !owner->valid || creature_type < 0 ||
        creature_type >= DM2_CREATURE_TYPE_COUNT ||
        !owner->creature_ai_row_loaded[creature_type]) return 0;
    row = owner->creature_ai_row[creature_type];
    if (row >= DM2_V1_SOURCE_AI_TABLE_SIZE || !owner->ai_row_loaded[row]) return 0;
    *out_def = &owner->ai_rows[row];
    return 1;
}

int dm2_v1_caii_source_owner_db4_type(const DM2_V1_CaiiSourceOwner *owner,
                                      uint16_t db4_index,
                                      uint8_t *out_creature_type)
{
    if (!out_creature_type) return 0;
    *out_creature_type = 0u;
    if (!owner || !owner->valid || !owner->db4_indices ||
        !owner->db4_creature_types || db4_index >= owner->db4_record_count ||
        owner->db4_indices[db4_index] != db4_index) return 0;
    *out_creature_type = owner->db4_creature_types[db4_index];
    return 1;
}
