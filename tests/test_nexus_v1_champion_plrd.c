#include "nexus_v1_champions.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_rlowfix_text.h"
#include "nexus_v1_startup_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    char item_path[1024];
    FILE *f;
    long size;
    uint8_t *bytes;
    Nexus_V1_ChampionPool pool;
    Nexus_V1_ItemIbsBank bank;
    Nexus_V1_RlowfixText text;
    Nexus_V1_RlowfixText menu_text;
    Nexus_V1_RlowfixTabl tabl;
    if (!root || !root[0]) root = ".firestaff/data/nexus";
    if (snprintf(path, sizeof(path), "%s/RLOWFIX.BIN", root) >=
            (int)sizeof(path) ||
        snprintf(item_path, sizeof(item_path), "%s/ITEM.IBS", root) >=
            (int)sizeof(item_path)) {
        return 77;
    }
    f = fopen(path, "rb");
    if (!f) { puts("SKIP: local Nexus RLOWFIX.BIN not present"); return 0; }
    if (fseek(f, 0, SEEK_END) != 0) return 1;
    size = ftell(f);
    if (size <= 0 || fseek(f, 0, SEEK_SET) != 0) return 1;
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, f) != (size_t)size) return 1;
    fclose(f);
    if (!nexus_v1_champions_init_from_rlowfix(&pool, bytes, (size_t)size)) return 1;
    if (!nexus_v1_rlowfix_text_parse(bytes, (size_t)size, 0xa374, &text) ||
        text.resource_index != 0 || text.string_count != 450) return 1;
    /* European RLOWFIX.BIN: TEXT resource 4 at 0xF270. */
    if (!nexus_v1_rlowfix_text_parse(bytes, (size_t)size, 0xf270,
                                     &menu_text) ||
        menu_text.resource_index != 4 || menu_text.string_count != 15) return 1;
    {
        uint16_t menu_index;
        for (menu_index = 0; menu_index < menu_text.string_count;
             ++menu_index) {
            const uint8_t *menu_bytes;
            size_t menu_size;
            if (!nexus_v1_rlowfix_text_span(bytes, (size_t)size,
                                            &menu_text, menu_index,
                                            &menu_bytes, &menu_size) ||
                !menu_bytes || menu_size == 0) return 1;
        }
    }
    /* European RLOWFIX.BIN: TABL directory record at 0x1232C. */
    if (!nexus_v1_rlowfix_tabl_parse(bytes, (size_t)size, 0x1232c, &tabl) ||
        tabl.entry_count != 216 || tabl.code[0] != 0x05 ||
        tabl.code[1] != 0xe16e) return 1;
    {
        const uint8_t *span;
        size_t span_size;
    if (!nexus_v1_rlowfix_text_span(bytes, (size_t)size, &text, 0,
                                        &span, &span_size) || !span ||
            span_size == 0) { fprintf(stderr,"span\\n"); return 1; }
    }
    {
        uint8_t *tampered = (uint8_t *)malloc((size_t)size);
        if (!tampered) return 1;
        memcpy(tampered, bytes, (size_t)size);
        /* TEXT#0 string 1 offset is relative to the eight-byte header. */
        tampered[0xa374U + 10U + 2U] = 0U;
        tampered[0xa374U + 10U + 3U] = 0U;
        if (nexus_v1_rlowfix_text_parse(tampered, (size_t)size,
                                        0xa374U, &text)) {
            free(tampered);
            return 1;
        }
        free(tampered);
    }
    if (pool.champion_count != NEXUS_NEXUS_PLRD_CHAMPION_COUNT) return 1;
    if (pool.champions[0].name_jp[0] != '\0' ||
        pool.champions[0].name_ascii[0] != '\0' ||
        pool.champions[0].health != 50 || pool.champions[0].stamina != 57 ||
        pool.champions[0].mana != 13 || pool.champions[19].health != 125 ||
        pool.champions[19].wizard_level != 2 ||
        pool.champions[0].food != 0 || pool.champions[0].water != 0 ||
        pool.champions[0].name_tabl_index[0] != 0x21 ||
        pool.champions[0].name_tabl_code[0] != 0x00c1) return 1;
    {
        Nexus_V1_StartupChampionRenderRow row;
        Nexus_V1_StartupChampionFooterRender footer;
        memset(&row, 0, sizeof(row));
        memset(&footer, 0, sizeof(footer));
        if (nexus_v1_startup_menu_build_champion_render_rows(
                &pool, 0, &row, 1, &footer) != 1 ||
            !row.source_name_glyphs_valid ||
            row.source_name_glyph_count != 4 ||
            row.source_name_glyphs[0] != 0x00c1U ||
            row.source_name_glyphs[3] != 0x00d8U ||
            row.label[0] != '\0' ||
            footer.label[0] != '\0') return 1;
        /* A stale host name must not reopen the ASCII compatibility lane for
         * an authenticated PLRD row that already owns TABL glyph codes. */
        snprintf(pool.champions[0].name_ascii,
                 sizeof(pool.champions[0].name_ascii), "STALE-HOST-NAME");
        memset(&row, 0, sizeof(row));
        memset(&footer, 0, sizeof(footer));
        if (nexus_v1_startup_menu_build_champion_render_rows(
                &pool, 0, &row, 1, &footer) != 1 ||
            row.label[0] != '\0' || footer.label[0] != '\0') return 1;
    }
    {
        FILE *item_file = fopen(item_path, "rb");
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
        nexus_itemdef_clear_ibs_declarations();
        if (nexus_itemdef_count() != 0 || nexus_itemdef_get(0) != NULL)
            return 1;
        free(item_bytes);
    }
    free(bytes);
    puts("test_nexus_v1_champion_plrd: PASS");
    return 0;
}
