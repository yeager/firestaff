/* Fixture shop economy must deterministically do nothing. */
#include "dm2_v1_shop.h"

#include <stdio.h>

static int failed;
#define CHECK(c, m) do { \
    if (c) printf("PASS: %s\n", m); \
    else { printf("FAIL: %s\n", m); ++failed; } \
} while (0)

int main(void)
{
    uint32_t gold;
    printf("DM2 V1 shop economy source-ownership probe\n");
    dm2_v1_shop_reset_state();
    dm2_v1_shop_set_party_gold(1000);
    gold = dm2_v1_shop_get_party_gold();
    CHECK(!dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL),
          "fixture general store cannot open");
    CHECK(dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0) !=
              (int)DM2_SHOP_RESULT_OK &&
              dm2_v1_shop_sell(DM2_SHOP_ID_GENERAL, 0) !=
                  (int)DM2_SHOP_RESULT_OK,
          "fixture purchase and sale cannot occur");
    CHECK(dm2_v1_shop_get_party_gold() == gold &&
              dm2_v1_shop_buy_count() == 0 && dm2_v1_shop_sell_count() == 0,
          "fixture economy cannot mutate state");
    return failed ? 1 : 0;
}
