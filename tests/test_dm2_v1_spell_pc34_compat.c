/* Source-ownership gate for the former DM2 tech/magic item fixture table. */
#include "dm2_v1_spell.h"
#include "dm2_v1_tech_magic.h"

#include <stdio.h>
#include <string.h>

static int failed;
#define CHECK(c, m) do { \
    if (c) printf("  PASS: %s\n", m); \
    else { printf("  FAIL: %s\n", m); ++failed; } \
} while (0)

int main(void)
{
    DM2_V1_TechMagicItem item;
    static const DM2_V1_SpellRecord expected[DM2_MAX_SPELL_ORIGINAL] = {
        {0x00686f76u,4,0x11,0x2c03},{0x006a6f77u,1,0x0f,0x1813},{0x00666f74u,4,0x0f,0x3823},{0x00686d77u,3,0x11,0x5833},
        {0x00666f00u,2,0x0f,0x3c43},{0x00690000u,1,0x10,0x1c53},{0x00686d74u,2,2,0x3863},{0x00686d73u,2,2,0x3873},
        {0x00697075u,4,0x0f,0x3883},{0x00686d75u,2,2,0x3893},{0x00686d72u,2,2,0x38a3},{0x00686f73u,4,3,0x78b3},
        {0x006b7073u,3,2,0x78e3},{0x00666d00u,0,3,0x04f3},{0x00686c00u,3,0x13,0x4072},{0x00686e76u,4,0x11,0x3c22},
        {0x00696f00u,3,0x10,0x5402},{0x00697072u,4,0x0d,0x1c71},{0x006a6d00u,1,0x12,0x2832},{0x006a6c00u,1,0x13,0x2062},
        {0x006b0000u,1,0x11,0x1c42},{0x00667000u,2,0x0f,0x30c1},{0x00660000u,2,0x0d,0x1cb1},{0x00667074u,4,0x0d,0x1c81},
        {0x00667075u,4,0x0d,0x1c91},{0x00670000u,1,0x0d,0x40e1},{0x00677000u,1,0x0d,0x34a1},{0x00687073u,4,0x0d,0x1c61},
        {0x006b7076u,3,2,0x80d1},{0x006b6d72u,6,3,0x7b14},{0x006b6d75u,4,0x0f,0x3f44},{0x006b6d73u,5,2,0x3354},
        {0x00686e72u,2,3,0x5892},{0x00686e73u,2,3,0x58a2},
    };
    int i;
    const char *e;

    printf("DM2 V1 spell and item source-ownership gates\n\n");
    CHECK(dm2_v1_spell_count() == DM2_MAX_SPELL_ORIGINAL &&
              dm2_v1_spell_get(5) != NULL,
          "source-locked spell table remains available");
    for (i = 0; i < DM2_MAX_SPELL_ORIGINAL; ++i) {
        const DM2_V1_SpellRecord *record = dm2_v1_spell_source_record(i);
        CHECK(record && memcmp(record, &expected[i], sizeof(*record)) == 0,
              "fixed spell record matches SKProject dSpellsTable");
    }
    CHECK(dm2_v1_tech_magic_lookup(DM2_ITEM_CROSSBOW, &item) == 0 &&
              dm2_v1_tech_magic_item_name(DM2_ITEM_HEAL_POTION) == NULL,
          "fixture item IDs and names are unavailable");
    CHECK(item.item_id == 0 && item.name == NULL &&
              dm2_v1_tech_magic_lookup(DM2_ITEM_PISTOL, NULL) == 0,
          "rejected lookup clears the result and handles null output");
    e = dm2_v1_tech_magic_source_evidence();
    CHECK(e && strstr(e, "legacy fixture item IDs") != NULL,
          "evidence declares fixture item data unavailable");
    printf("\n%d failure(s)\n", failed);
    return failed ? 1 : 0;
}
