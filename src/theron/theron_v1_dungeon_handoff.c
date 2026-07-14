#include "theron_v1_dungeon_handoff.h"

#include <string.h>

int theron_v1_dungeon_handoff_select_initial_level(
    const Theron_V1DungeonHandoffFacts *facts,
    Theron_V1DungeonHandoffReceipt *out_receipt) {
    Theron_V1DungeonHandoffReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!facts || !out_receipt || !facts->runtime_admission ||
        !facts->runtime_admission->attached ||
        !facts->runtime_admission->admitted || !facts->track02_hash_verified ||
        !facts->initial_level_envelope_verified ||
        !facts->adjacent_boundary_unparsed ||
        facts->record != THERON_V1_INITIAL_LEVEL_RECORD ||
        facts->user_data_offset != THERON_V1_INITIAL_LEVEL_USER_DATA_OFFSET ||
        facts->envelope_bytes != THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        facts->level_identifier != THERON_V1_INITIAL_LEVEL_IDENTIFIER) {
        return 0;
    }

    receipt.selected = 1;
    receipt.runtime_route_consumed = 1;
    receipt.record = facts->record;
    receipt.user_data_offset = facts->user_data_offset;
    receipt.envelope_bytes = facts->envelope_bytes;
    receipt.level_identifier = facts->level_identifier;
    receipt.route = "initial_level_source_locked";
    *out_receipt = receipt;
    return 1;
}
