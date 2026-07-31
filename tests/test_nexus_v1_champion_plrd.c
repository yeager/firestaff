#include "nexus_v1_champions.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_rlowfix_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *path = "/Users/bosse/.firestaff/data/nexus/RLOWFIX.BIN";
    FILE *f = fopen(path, "rb");
    long size;
    uint8_t *bytes;
    Nexus_V1_ChampionPool pool;
    Nexus_V1_ItemIbsBank bank;
    Nexus_V1_RlowfixText text;
    Nexus_V1_RlowfixTabl tabl;
    if (!f) { puts("SKIP: local Nexus RLOWFIX.BIN not present"); return 0; }
    if (fseek(f, 0, SEEK_END) != 0) return 1;
    size = ftell(f);
    if (size <= 0 || fseek(f, 0, SEEK_SET) != 0) return 1;
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, f) != (size_t)size) return 1;
    fclose(f);
    if (!nexus_v1_champions_init_from_rlowfix(&pool, bytes, (size_t)size)) return 1;
    if (!nexus_v1_rlowfix_text_parse(bytes, (size_t)size, 0xa374, &text) ||
        text.resource_index != 0 || text.string_count != 449) return 1;
    if (!nexus_v1_rlowfix_tabl_parse(bytes, (size_t)size, 0x118d4, &tabl) ||
        tabl.entry_count != 216 || tabl.code[0] != 0x05 ||
        tabl.code[1] != 0xe16e) return 1;
    {
        const uint8_t *span;
        size_t span_size;
        if (!nexus_v1_rlowfix_text_span(bytes, (size_t)size, &text, 0,
                                        &span, &span_size) || !span ||
            span_size == 0) return 1;
    }
    if (pool.champion_count != NEXUS_NEXUS_PLRD_CHAMPION_COUNT) return 1;
    if (strcmp(pool.champions[0].name_jp, "アレックス") != 0 ||
        pool.champions[0].health != 50 || pool.champions[0].stamina != 57 ||
        pool.champions[0].mana != 13 || pool.champions[19].health != 125 ||
        pool.champions[19].wizard_level != 2 ||
        pool.champions[0].name_tabl_index[0] != 0x91 ||
        pool.champions[0].name_tabl_code[0] != 0x0064) return 1;
    {
        FILE *item_file = fopen("/Users/bosse/.firestaff/data/nexus/ITEM.IBS", "rb");
        long item_size;
        uint8_t *item_bytes;
        if (!item_file || fseek(item_file, 0, SEEK_END) != 0) return 1;
        item_size = ftell(item_file);
        if (item_size <= 0 || fseek(item_file, 0, SEEK_SET) != 0) return 1;
        item_bytes = (uint8_t *)malloc((size_t)item_size);
        if (!item_bytes || fread(item_bytes, 1, (size_t)item_size, item_file) !=
                (size_t)item_size) return 1;
        fclose(item_file);
        if (nexus_v1_item_ibs_parse_verified(item_bytes, (int)item_size, 1,
                                             &bank) != 0) return 1;
        nexus_itemdef_bind_ibs_declarations(bank.item_category,
                                             bank.item_weight,
                                             bank.item_name_string,
                                             bank.item_desc_string,
                                             bank.item_action1_string,
                                             bank.item_action2_string,
                                             bank.item_action3_string,
                                             NEXUS_V1_ITEM_IBS_DECLARATION_COUNT);
        if (nexus_itemdef_count() != NEXUS_V1_ITEM_IBS_DECLARATION_COUNT ||
            !nexus_itemdef_get(0) ||
            nexus_itemdef_get(0)->weight != bank.item_weight[0] ||
            nexus_itemdef_get(0)->name != NULL) return 1;
        free(item_bytes);
    }
    free(bytes);
    puts("test_nexus_v1_champion_plrd: PASS");
    return 0;
}
