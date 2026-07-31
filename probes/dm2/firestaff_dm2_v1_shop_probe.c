/* Verifies that DM2 shop fixture data cannot become runtime game data. */
#include "dm2_v1_shop.h"

#include <stdio.h>
#include <string.h>

static int errors;
#define CHECK(c, m) do { \
    if (c) printf("PASS: %s\n", m); \
    else { printf("FAIL: %s\n", m); ++errors; } \
} while (0)

int main(void)
{
    const char *e;
    printf("DM2 V1 shop source-ownership probe\n");
    dm2_v1_shop_reset_state();
    dm2_v1_shop_set_party_gold(500);
    CHECK(dm2_v1_shop_get_builtin_count() == 0,
          "no fixture shop catalog is admitted");
    CHECK(dm2_v1_shop_get_builtin(DM2_SHOP_ID_WEAPONS) == NULL &&
              dm2_v1_shop_lookup_index(DM2_SHOP_ID_WEAPONS) == -1,
          "fixture shop position and stock are unavailable");
    CHECK(dm2_v1_npc_get_count() == 0 &&
              dm2_v1_npc_get_name(DM2_NPC_MERCHANT_GREEDY) == NULL,
          "fixture NPC text is unavailable");
    CHECK(!dm2_v1_shop_enter(DM2_SHOP_ID_WEAPONS) &&
              dm2_v1_shop_buy(DM2_SHOP_ID_WEAPONS, 0) !=
                  (int)DM2_SHOP_RESULT_OK &&
              dm2_v1_shop_get_party_gold() == 500,
          "fixture shop cannot mutate party gold or inventory");
    e = dm2_v1_shop_source_evidence();
    CHECK(e && strstr(e, "actuator") && strstr(e, "legacy fixture shops"),
          "source evidence requires original actuator ownership");
    return errors ? 1 : 0;
}
