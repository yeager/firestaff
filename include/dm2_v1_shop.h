
#ifndef FIRESTAFF_DM2_V1_SHOP_H
#define FIRESTAFF_DM2_V1_SHOP_H

/*
 * dm2_v1_shop.h - DM2 V1 Shops + NPCs Parity
 *
 * DM2 Phase 4 mechanics parity module: in-dungeon shops + NPCs.
 *
 * DM2 shops are floor sensors of type DM2_FLOOR_SENSOR_SHOP (0x30)
 * that activate a shop panel (DM2_ACTUATOR_TYPE_SHOP_PANEL = 0x3F).
 * When the party steps on a shop square, the shop panel opens and
 * shows the shop's inventory + prices; the player can buy/sell via
 * the standard inventory grid.
 *
 * Source-lock anchors (DM2 is reverse-engineered via skproject):
 *   skproject/SKULLWIN/c_shop.cpp         - shop panel + transaction logic
 *   skproject/SKWIN/DME.h:1505-1560       - shop_descriptor_t
 *   skproject/SKWIN/SkGlobal.cpp:966-1011 - dSpellsTable + shop tables
 *   skproject/SKULLWIN/c_npc.cpp          - NPC dialog + personality table
 *   skproject/SKULLWIN/SKWinGlobal.h:42   - NUM_NPCS=4
 *   ReDMCSB docs/dm2_sensors.md           - SHOP sensor (0x30) details
 *   ReDMCSB docs/dm2_actuators.md         - SHOP_PANEL actuator (0x3F)
 *
 * Skproject notes:
 *   - Each shop is owned by one NPC (one of NUM_NPCS=4 personalities).
 *   - Shop stock: up to DM2_SHOP_MAX_STOCK (default 8) item slots.
 *   - Prices are scaled by NPC personality + dungeon level.
 *   - NPCs greet/taunt the player with personality-flavored dialog.
 *
 * Implementation strategy (no SKULL.ASM in ReDMCSB; DM2 decompilation
 * only available through skproject):
 *   1. Built-in shop catalog (5 well-known DM2 shops with stock tables).
 *   2. Built-in NPC dialog table (4 NPCs x 6 lines = 24 lines).
 *   3. Price formula: base_price * (100 - negotiator_skill) / 100.
 *   4. State: active_shop_id, party_gold, party_inventory hashes.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ───────────────────────────────────────────────────── */
#define DM2_SHOP_MAX_STOCK        8     /* items per shop */
#define DM2_NUM_NPCS              4     /* DM2 has 4 NPC personalities */
#define DM2_NUM_BUILTIN_SHOPS     5     /* built-in shop count */
#define DM2_NPC_DIALOG_LINES      6     /* dialog lines per NPC */

/* Shop IDs (built-in catalog) */
#define DM2_SHOP_ID_NONE          0
#define DM2_SHOP_ID_GENERAL       1     /* Town General Store */
#define DM2_SHOP_ID_WEAPONS       2     /* Weapons Master */
#define DM2_SHOP_ID_MAGIC         3     /* Magic Emporium */
#define DM2_SHOP_ID_TAVERN        4     /* Tavern (food + drink) */
#define DM2_SHOP_ID_BLACKSMITH    5     /* Blacksmith (armour + repair) */

/* NPC personality IDs (also used as shop owners) */
#define DM2_NPC_NONE              0
#define DM2_NPC_MERCHANT_FRIENDLY 1
#define DM2_NPC_MERCHANT_GREEDY   2
#define DM2_NPC_WIZARD            3
#define DM2_NPC_BLACKSMITH        4

/* Shop descriptor: location, owner NPC, stock */
typedef struct {
    int      shop_id;            /* DM2_SHOP_ID_* */
    int      npc_id;             /* DM2_NPC_* owner */
    int      map_x;              /* shop square X */
    int      map_y;              /* shop square Y */
    int      map_level;          /* dungeon level index */
    int      stock_count;        /* number of stocked items */
    struct {
        int item_id;             /* DM2 item ID (tech/magic or generic) */
        int base_price;          /* base gold price */
        int stock_remaining;     /* -1 = unlimited */
    } stock[DM2_SHOP_MAX_STOCK];
} DM2_V1_ShopDescriptor;

/* Shop interaction result codes */
typedef enum {
    DM2_SHOP_RESULT_OK = 0,
    DM2_SHOP_RESULT_NOT_FOUND,
    DM2_SHOP_RESULT_NO_ACTIVE_SHOP,
    DM2_SHOP_RESULT_ITEM_NOT_IN_STOCK,
    DM2_SHOP_RESULT_INSUFFICIENT_GOLD,
    DM2_SHOP_RESULT_INVENTORY_FULL,
    DM2_SHOP_RESULT_INVALID_ITEM,
} DM2_ShopResult;

/* Active shop state (module-static singleton). */
typedef struct {
    int      active_shop_id;
    int      party_negotiator_skill;  /* 0..100; higher = better prices */
    uint32_t party_gold;
    /* Simple inventory model: 32 slots, each item_id + qty. */
    uint16_t inventory_item[32];
    uint16_t inventory_qty[32];
    int      inventory_count;  /* number of populated slots */
    /* Party state hash for "shop leave preserves party state" test. */
    uint32_t party_state_hash;
    /* Observability. */
    int      buy_count;
    int      sell_count;
    int      enter_count;
    int      leave_count;
} DM2_V1_ShopState;

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_shop_reset_state(void);
void dm2_v1_shop_set_party_gold(uint32_t gold);
uint32_t dm2_v1_shop_get_party_gold(void);
void dm2_v1_shop_set_negotiator(int skill);
void dm2_v1_shop_clear_inventory(void);
int  dm2_v1_shop_add_inventory(int item_id, int qty);  /* 1=ok, 0=full */

/* ── Catalog ────────────────────────────────────────────────────── */
int  dm2_v1_shop_get_builtin_count(void);
const DM2_V1_ShopDescriptor *dm2_v1_shop_get_builtin(int shop_id);
int  dm2_v1_shop_lookup_index(int shop_id);  /* returns index or -1 */

/* ── Interaction API ───────────────────────────────────────────── */
int  dm2_v1_shop_enter(int shop_id);              /* 1 = entered, 0 = bad */
int  dm2_v1_shop_buy(int shop_id, int stock_idx); /* 1 = bought, 0 = fail */
int  dm2_v1_shop_sell(int shop_id, int inv_idx);  /* 1 = sold, 0 = fail */
int  dm2_v1_shop_leave(int shop_id);              /* 1 = left, 0 = bad */
int  dm2_v1_shop_is_active(void);                 /* 1 if inside a shop */
int  dm2_v1_shop_get_active_shop(void);           /* DM2_SHOP_ID_* */

/* Price helpers */
int  dm2_v1_shop_get_base_price(int shop_id, int stock_idx);
int  dm2_v1_shop_get_effective_price(int shop_id, int stock_idx);
int  dm2_v1_shop_get_sell_price(int shop_id, int inv_idx);  /* 50% of base */

/* ── NPC dialog ─────────────────────────────────────────────────── */
const char *dm2_v1_npc_get_name(int npc_id);
const char *dm2_v1_npc_get_dialog(int npc_id, int line_idx);  /* 0..5 */
int  dm2_v1_npc_get_count(void);

/* ── State accessor (read-only) ─────────────────────────────────── */
const DM2_V1_ShopState *dm2_v1_shop_get_state(void);
uint32_t dm2_v1_shop_party_state_hash(void);

/* ── Observability counters ─────────────────────────────────────── */
int dm2_v1_shop_buy_count(void);
int dm2_v1_shop_sell_count(void);
int dm2_v1_shop_enter_count(void);
int dm2_v1_shop_leave_count(void);

const char *dm2_v1_shop_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SHOP_H */
