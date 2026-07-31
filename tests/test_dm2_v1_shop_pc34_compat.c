/* DM2 shop fixtures must remain unavailable until source ownership exists. */
#include "dm2_v1_shop.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(c, m) do { \
    if (c) { ++passed; printf("  PASS: %s\n", m); } \
    else { ++failed; printf("  FAIL: %s\n", m); } \
} while (0)

int main(void)
{
    const char *e;

    printf("DM2 V1 shop source-ownership gate\n\n");
    dm2_v1_shop_reset_state();
    dm2_v1_shop_set_party_gold(1000);
    CHECK(dm2_v1_shop_get_builtin_count() == 0,
          "fixture shop catalog is not admitted");
    CHECK(dm2_v1_shop_get_builtin(DM2_SHOP_ID_GENERAL) == NULL &&
              dm2_v1_shop_lookup_index(DM2_SHOP_ID_GENERAL) == -1,
          "fixture shop identity is unavailable");
    CHECK(dm2_v1_npc_get_count() == 0 &&
              dm2_v1_npc_get_name(DM2_NPC_MERCHANT_FRIENDLY) == NULL &&
              dm2_v1_npc_get_dialog(DM2_NPC_MERCHANT_FRIENDLY, 0) == NULL,
          "fixture NPC names and dialog are unavailable");
    CHECK(!dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL) &&
              !dm2_v1_shop_is_active() &&
              dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0) !=
                  (int)DM2_SHOP_RESULT_OK,
          "fixture catalog cannot create a transaction");
    CHECK(dm2_v1_shop_get_party_gold() == 1000 &&
              dm2_v1_shop_buy_count() == 0 && dm2_v1_shop_sell_count() == 0,
          "rejected transaction preserves party state");
    e = dm2_v1_shop_source_evidence();
    CHECK(e && strstr(e, "legacy fixture shops") != NULL,
          "evidence declares the fixture catalog unavailable");
    printf("\n%d/%d checks passed\n", passed, passed + failed);
    return failed ? 1 : 0;
}
