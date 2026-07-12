#ifndef THERON_V1_STAGE2_RUNTIME_HANDOFF_H
#define THERON_V1_STAGE2_RUNTIME_HANDOFF_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Bounded semantic handoff for the first live stage-two CD_READ.
 *
 * Original evidence: the authenticated HuC6280 stage-two loader executes
 * CD_READ at $4090 into local RAM $3800, then transfers control with
 * `jmp L3800` after the completed read.  See
 * docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:163-181.
 *
 * This proves the record is a stage-three executable handoff.  It does not
 * classify the 218 manifest entries inside that executable as graphics,
 * palettes, objects, levels, or any other game data.
 */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    size_t raw_sector;
    size_t user_data_offset;
    size_t user_data_bytes;
    uint16_t load_address;
    uint16_t entry_address;
    int execute_after_load;
    uint16_t cleared_work_ram_start;
    size_t cleared_work_ram_bytes;
    uint16_t cleared_work_ram_end;
    int work_ram_cleared_before_entry;
    uint16_t header_word0;
    uint16_t header_word1;
    size_t manifest_bytes;
    size_t manifest_entry_count;
    uint32_t user_data_hash;
    int manifest_entries_semantically_unbound;
    int physical_stage3_entry_verified;
    uint8_t stage3_entry_opcode;
    uint8_t stage3_irq2_selector;
    uint16_t stage3_continuation_address;
} Theron_V1Stage2RuntimeHandoff;

/* Converts an already hash-gated, structurally validated dynamic-payload
 * receipt into the executable handoff above.  Invalid or non-raw variants
 * reject without producing a partial handoff. */
int theron_v1_stage2_runtime_handoff_from_dynamic_payload(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage2RuntimeHandoff *out_handoff);

/* Full original-media form consumed by CUE boot, startup receipt, and direct
 * runtime entry. It binds IPL CD_EXEC, the traced stage-two CD_READ, cleared
 * work RAM, and the physical stage-three BRK $ff bytes. It intentionally does
 * not classify the loaded manifest. */
int theron_v1_stage2_runtime_handoff_from_original_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_V1Stage2RuntimeHandoff *out_handoff);

#endif /* THERON_V1_STAGE2_RUNTIME_HANDOFF_H */
