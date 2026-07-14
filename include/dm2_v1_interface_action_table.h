#ifndef FIRESTAFF_DM2_V1_INTERFACE_ACTION_TABLE_H
#define FIRESTAFF_DM2_V1_INTERFACE_ACTION_TABLE_H

#include "dm2_v1_asset_loader.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * skproject/SKWIN/SkWinCore.cpp::LOAD_GDAT_INTERFACE_00_02 reads
 * INTERFACE_GENERAL/0/dtRaw7/2 as:
 *
 *   group_count, group_length[group_count], block_a, block_b, command_tail
 *
 * Both variable blocks contain one group of group_length bytes in the same
 * order. This is a borrowed view of the original GDAT bytes; the loader and
 * its source data must outlive the table.
 */
#define DM2_V1_INTERFACE_ACTION_GROUP_LIMIT 255u

typedef struct {
    const uint8_t *raw;
    size_t raw_size;
    uint8_t group_count;
    size_t group_bytes;
    const uint8_t *group_a[DM2_V1_INTERFACE_ACTION_GROUP_LIMIT];
    const uint8_t *group_b[DM2_V1_INTERFACE_ACTION_GROUP_LIMIT];
    uint8_t group_size[DM2_V1_INTERFACE_ACTION_GROUP_LIMIT];
    const uint8_t *command_tail;
    size_t command_tail_size;
} DM2_V1_InterfaceActionTable;

/* Decode a raw dtRaw7/2 payload exactly as LOAD_GDAT_INTERFACE_00_02.
 * Returns 1 on success, or 0 on malformed/truncated input. */
int dm2_v1_interface_action_table_parse(const uint8_t *raw,
                                        size_t raw_size,
                                        DM2_V1_InterfaceActionTable *out);

/* Resolve and decode INTERFACE_GENERAL/0/dtRaw7/2 from a real GDAT loader. */
int dm2_v1_interface_action_table_load(const DM2_V1_AssetLoader *loader,
                                       DM2_V1_InterfaceActionTable *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_INTERFACE_ACTION_TABLE_H */
