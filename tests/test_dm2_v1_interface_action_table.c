#include "dm2_v1_interface_action_table.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *message)
{
    if (condition) {
        printf("PASS: %s\n", message);
        return 0;
    }
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    /* Two groups: sizes 2 and 3, followed by the original command tail. */
    const uint8_t raw[] = {
        2u, 2u, 3u,
        0x10u, 0x11u, 0x12u, 0x13u, 0x14u,
        0x20u, 0x21u, 0x22u, 0x23u, 0x24u,
        0xa0u, 0xa1u
    };
    DM2_V1_InterfaceActionTable table;
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entry;
    uint32_t raw_offset = 0u;
    uint32_t raw_size = (uint32_t)sizeof(raw);
    int failed = 0;

    failed |= check(dm2_v1_interface_action_table_parse(
                        raw, sizeof(raw), &table) == 1,
                    "skproject dtRaw7 action table decodes");
    failed |= check(table.group_count == 2u && table.group_bytes == 5u &&
                        table.group_size[0] == 2u && table.group_size[1] == 3u,
                    "group count and source lengths are retained");
    failed |= check(table.group_a[0][0] == 0x10u && table.group_a[1][2] == 0x14u &&
                        table.group_b[0][0] == 0x20u && table.group_b[1][2] == 0x24u,
                    "both source variable-length blocks retain group boundaries");
    failed |= check(table.command_tail_size == 2u &&
                        table.command_tail[0] == 0xa0u && table.command_tail[1] == 0xa1u,
                    "command tail starts after both source blocks");
    failed |= check(dm2_v1_interface_action_table_parse(raw, 12u, &table) == 0 &&
                        table.raw == NULL,
                    "truncated second source block fails closed");
    failed |= check(dm2_v1_interface_action_table_parse(raw, 2u, &table) == 0 &&
                        table.raw == NULL,
                    "truncated group-length list fails closed");
    failed |= check(dm2_v1_interface_action_table_parse(NULL, 0u, &table) == 0 &&
                        table.raw == NULL,
                    "absent source payload fails closed");

    memset(&loader, 0, sizeof(loader));
    memset(&entry, 0, sizeof(entry));
    loader.data = raw;
    loader.data_size = sizeof(raw);
    loader.loaded = 1;
    loader.raw_data_count = 1u;
    loader.raw_offsets = &raw_offset;
    loader.raw_sizes = &raw_size;
    loader.entries = &entry;
    loader.entry_count = 1u;
    entry.cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entry.cls3 = DM2_GDAT_ENTRY_TYPE_RAW7;
    entry.cls4 = DM2_GDAT_INTERFACE_RAW_ACTION_TABLE;
    failed |= check(dm2_v1_interface_action_table_load(&loader, &table) == 1 &&
                        table.command_tail_size == 2u &&
                        table.group_b[1][1] == 0x23u,
                    "typed GDAT lookup binds the source action-table address");

    return failed ? 1 : 0;
}
