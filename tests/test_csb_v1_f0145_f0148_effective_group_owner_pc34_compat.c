#include "csb_v1_f0145_f0148_effective_group_owner_pc34_compat.h"

#include <assert.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

static CsbV1F0145F0148EffectiveGroupOwnerPc34Compat make_owner(
    uint8_t record[16],
    uint16_t map_index,
    uint16_t party_map_index,
    CSB_V1_RuntimeActiveGroupState *active_groups,
    size_t active_group_count)
{
    CsbV1F0145F0148EffectiveGroupOwnerPc34Compat owner;
    memset(&owner, 0, sizeof(owner));
    owner.group_record = record;
    owner.record_size = 16u;
    owner.group_thing = (uint16_t)((4u << 10) | 7u);
    owner.map_index = map_index;
    owner.party_map_index = party_map_index;
    owner.active_groups = active_groups;
    owner.active_group_count = active_group_count;
    return owner;
}

int main(void)
{
    uint8_t raw[16] = { 0 };
    uint8_t party_record[16] = { 0 };
    CSB_V1_RuntimeActiveGroupState active[3];
    CsbV1F0145F0148EffectiveGroupOwnerPc34Compat owner;
    CsbV1F0145F0148EffectiveGroupValuesPc34Compat values;
    CsbV1F0145F0148EffectiveGroupMutationPc34Compat mutation;

    memset(active, 0, sizeof(active));
    raw[5] = 0x6cu;
    raw[15] = 0xfdu;
    owner = make_owner(raw, 1u, 2u, NULL, 0u);
    CHECK(csb_v1_f0145_f0148_effective_group_read_pc34_compat(&owner, &values));
    CHECK(values.cells == 0x6cu);
    CHECK(values.directions == 0x55u);

    mutation.write_cells = 1;
    mutation.cells = 0xa5u;
    mutation.write_directions = 1;
    mutation.directions = 2u;
    CHECK(csb_v1_f0145_f0148_effective_group_write_pc34_compat(&owner, &mutation));
    CHECK(raw[5] == 0xa5u);
    CHECK(raw[15] == 0xfeu);
    CHECK(csb_v1_f0145_f0148_effective_group_read_pc34_compat(&owner, &values));
    CHECK(values.directions == 0xaau);

    party_record[5] = 1u;
    party_record[15] = 3u;
    active[1].valid = 1;
    active[1].cells = 0x33u;
    active[1].directions = 0x5a5au;
    owner = make_owner(party_record, 2u, 2u, active, 3u);
    CHECK(csb_v1_f0145_f0148_effective_group_read_pc34_compat(&owner, &values));
    CHECK(values.cells == 0x33u);
    CHECK(values.directions == 0x5a5au);

    mutation.cells = 0xceu;
    mutation.directions = 0x1234u;
    CHECK(csb_v1_f0145_f0148_effective_group_write_pc34_compat(&owner, &mutation));
    CHECK(party_record[5] == 1u);
    CHECK(party_record[15] == 3u);
    CHECK(active[1].cells == 0xceu);
    CHECK(active[1].directions == 0x1234u);

    party_record[5] = 2u;
    CHECK(!csb_v1_f0145_f0148_effective_group_read_pc34_compat(&owner, &values));
    CHECK(!csb_v1_f0145_f0148_effective_group_write_pc34_compat(&owner, &mutation));
    CHECK(active[1].cells == 0xceu);
    CHECK(active[1].directions == 0x1234u);

    owner.group_thing = 7u;
    CHECK(!csb_v1_f0145_f0148_effective_group_read_pc34_compat(&owner, &values));
    return 0;
}
