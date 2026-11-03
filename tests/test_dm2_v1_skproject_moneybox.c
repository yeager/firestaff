#include "dm2_v1_skproject_core.h"
#include "dm2_v1_hero_ops_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    uint16_t head;
    uint16_t next[8];
    uint16_t db_type[8];
    uint16_t denomination[8];
    uint8_t charge[8];
    unsigned deallocations;
} MoneyboxFixture;

static int is_moneybox(void *ctx, uint16_t container)
{
    (void)ctx;
    return container == 1u;
}

static int is_currency(void *ctx, uint16_t item)
{
    MoneyboxFixture *f = (MoneyboxFixture *)ctx;
    return f->db_type[item] == DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY;
}

static uint16_t get_db_type(void *ctx, uint16_t item)
{
    return ((MoneyboxFixture *)ctx)->db_type[item];
}

static uint16_t get_denomination(void *ctx, uint16_t item)
{
    return ((MoneyboxFixture *)ctx)->denomination[item];
}

static uint16_t get_head(void *ctx, uint16_t container)
{
    (void)container;
    return ((MoneyboxFixture *)ctx)->head;
}

static uint16_t get_next(void *ctx, uint16_t item)
{
    return ((MoneyboxFixture *)ctx)->next[item];
}

static uint8_t get_charge(void *ctx, uint16_t item)
{
    return ((MoneyboxFixture *)ctx)->charge[item];
}

static void set_charge(void *ctx, uint16_t item, uint8_t charge)
{
    ((MoneyboxFixture *)ctx)->charge[item] = charge;
}

static void append_item(void *ctx, uint16_t item, uint16_t container)
{
    MoneyboxFixture *f = (MoneyboxFixture *)ctx;
    uint16_t tail;
    (void)container;
    if (f->head == DM2_V1_OBJECT_END) {
        f->head = item;
        f->next[item] = DM2_V1_OBJECT_END;
        return;
    }
    tail = f->head;
    while (f->next[tail] != DM2_V1_OBJECT_END)
        tail = f->next[tail];
    f->next[tail] = item;
    f->next[item] = DM2_V1_OBJECT_END;
}

static void cut_item(void *ctx, uint16_t item, uint16_t container)
{
    MoneyboxFixture *f = (MoneyboxFixture *)ctx;
    uint16_t current;
    (void)container;
    if (f->head == item) {
        f->head = f->next[item];
        return;
    }
    current = f->head;
    while (current != DM2_V1_OBJECT_END && f->next[current] != item)
        current = f->next[current];
    if (current != DM2_V1_OBJECT_END)
        f->next[current] = f->next[item];
}

static void dealloc_record(void *ctx, uint16_t item)
{
    (void)item;
    ((MoneyboxFixture *)ctx)->deallocations++;
}

static uint16_t fail_allocation(void *ctx, uint16_t denomination)
{
    (void)ctx;
    (void)denomination;
    return DM2_V1_OBJECT_NULL;
}

static int32_t next_record(uint16_t item, void *ctx)
{
    (void)item;
    (void)ctx;
    return DM2_V1_OBJECT_END;
}

static int32_t creature_at(int16_t x, int16_t y, void *ctx)
{
    (void)x;
    (void)y;
    (void)ctx;
    return 0x1200;
}

static int32_t can_handle_item(uint16_t type, uint16_t possession,
                               uint16_t slot, void *ctx)
{
    (void)type;
    (void)possession;
    (void)slot;
    (void)ctx;
    return -1;
}

static void cmd2165(uint16_t mode, int16_t x, int16_t y,
                    int16_t target_x, int16_t target_y,
                    int16_t arg5, int16_t arg6, void *ctx)
{
    (void)mode;
    (void)x;
    (void)y;
    (void)target_x;
    (void)target_y;
    (void)arg5;
    (void)arg6;
    (void)ctx;
}

static int32_t append_possession(uint16_t coin, void *ctx)
{
    (void)coin;
    (void)ctx;
    return 1;
}

int main(void)
{
    static const uint16_t coin_types[] = {10u, 20u};
    static const uint16_t coin_values[] = {1u, 2u};
    MoneyboxFixture fixture;
    DM2_V1_WalletCallbacks callbacks;
    DM2_V1_SkprojectExchangeMoneyboxRequest request;
    DM2_V1_SkprojectExchangeMoneyboxReceipt receipt;

    memset(&fixture, 0, sizeof(fixture));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&request, 0, sizeof(request));
    fixture.head = 2u;
    fixture.next[2] = DM2_V1_OBJECT_END;
    fixture.db_type[2] = DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY;
    fixture.denomination[2] = coin_types[0];
    fixture.charge[2] = 1u;

    callbacks.is_container_moneybox = is_moneybox;
    callbacks.is_item_currency = is_currency;
    callbacks.get_item_db_type = get_db_type;
    callbacks.get_distinctive_item_type = get_denomination;
    callbacks.get_container_head = get_head;
    callbacks.get_next_item_in_list = get_next;
    callbacks.get_item_charge = get_charge;
    callbacks.set_item_charge = set_charge;
    callbacks.append_item_to_container = append_item;
    callbacks.cut_item_from_container = cut_item;
    callbacks.dealloc_record = dealloc_record;
    callbacks.alloc_new_dbitem = fail_allocation;
    callbacks.coin_type_table = coin_types;
    callbacks.coin_type_table_size = 2;

    request.coin_values = coin_values;
    request.coin_type_count = 2u;
    request.wallet_callbacks = &callbacks;
    request.wallet_ctx = &fixture;
    request.wallet_handle = 1u;

    if (dm2_v1_skproject_exchange_moneybox(&request, &receipt) != 0 ||
        !receipt.blocked_missing_context || receipt.mutated ||
        fixture.head != 2u || fixture.charge[2] != 1u ||
        fixture.deallocations != 0u) {
        fprintf(stderr, "moneybox allocation failure changed the live wallet\n");
        return 1;
    }

    puts("PASS: SKProject moneybox allocation failure preserves existing coins");

    {
        DM2_V1_SkprojectXactContext xact;
        DM2_V1_SkprojectXact82Receipt xact_receipt;
        memset(&xact, 0, sizeof(xact));
        xact.creature_word_e = 0u;
        xact.possession = DM2_V1_OBJECT_END;
        xact.v1e0572 = 3;
        xact.creature_w0c = 2;
        xact.coin_values = coin_values;
        xact.coin_type_count = 2u;
        xact.wallet_callbacks = &callbacks;
        xact.wallet_ctx = &fixture;
        xact.next_fn = next_record;
        xact.creature_at_fn = creature_at;
        xact.can_handle_item_in_fn = can_handle_item;
        xact.cmd2165_fn = cmd2165;
        xact.append_coin_to_possession_fn = append_possession;
        xact.xact82_variant = DM2_V1_XACT82_SKULLWIN_SPX_V5;
        if (dm2_v1_skproject_proceed_xact_82(&xact, &xact_receipt) != 0 ||
            !xact_receipt.payout_incomplete || !xact_receipt.blocked_missing_context ||
            xact_receipt.price_consumed || xact.creature_w0c != 2 ||
            xact_receipt.generated_coin_count != 0u) {
            fprintf(stderr, "XACT 82 allocation failure consumed sale price\n");
            return 1;
        }
    }
    puts("PASS: SKProject XACT 82 allocation failure preserves creature price");
    return 0;
}
