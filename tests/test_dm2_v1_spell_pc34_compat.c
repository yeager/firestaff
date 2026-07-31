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
    const char *e;

    printf("DM2 V1 spell and item source-ownership gates\n\n");
    CHECK(dm2_v1_spell_count() == DM2_MAX_SPELL_ORIGINAL &&
              dm2_v1_spell_get(5) != NULL,
          "source-locked spell table remains available");
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
