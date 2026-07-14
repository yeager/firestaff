#include <stdio.h>
#include <string.h>

#include "theron_v1_dungeon_handoff.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static Theron_V1DungeonHandoffFacts valid_facts(void) {
    static Theron_V1RuntimeAdmissionReceipt admission = {1, 1};
    Theron_V1DungeonHandoffFacts facts = {
        &admission, 1, 1, 1,
        THERON_V1_INITIAL_LEVEL_RECORD,
        THERON_V1_INITIAL_LEVEL_USER_DATA_OFFSET,
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
        THERON_V1_INITIAL_LEVEL_IDENTIFIER
    };
    return facts;
}

int main(void) {
    Theron_V1DungeonHandoffFacts facts = valid_facts();
    Theron_V1RuntimeAdmissionReceipt admission = {1, 1};
    Theron_V1DungeonHandoffReceipt receipt;

    facts.runtime_admission = &admission;

    CHECK(theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    CHECK(receipt.selected && receipt.runtime_route_consumed);
    CHECK(receipt.record == 0x0b52u);
    CHECK(receipt.user_data_offset == 0x114u);
    CHECK(receipt.envelope_bytes == 0x36cu);
    CHECK(receipt.level_identifier == 0x0026u);
    CHECK(strcmp(receipt.route, "initial_level_source_locked") == 0);

    admission.admitted = 0;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    admission.admitted = 1;
    facts.record = 0x04e0u;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    facts.record = THERON_V1_INITIAL_LEVEL_RECORD;
    facts.adjacent_boundary_unparsed = 0;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    facts.adjacent_boundary_unparsed = 1;
    facts.level_identifier = 0x0027u;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));

    return failures != 0;
}
