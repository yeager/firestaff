#ifndef NEXUS_V1_INVENTORY_H
#define NEXUS_V1_INVENTORY_H

#include <stdint.h>

/* Nexus V1 inventory system.
 * 30 slots per champion (vs DM1's 12-slot grid).
 * Items stored as indices into a global item encyclopedia.
 * Supports: pickup from floor, drop to floor, equip/unequip,
 * swap between slots, cursor-held item. */

/* Item category enum — values from ITEM.IBS Byte1 (DMWeb documentation) */
typedef enum {
    NEXUS_ITEM_WEAPON  = 0,
    NEXUS_ITEM_ARMOUR  = 1,
    NEXUS_ITEM_FOOD    = 2,
    NEXUS_ITEM_POTION  = 3,
    NEXUS_ITEM_SCROLL  = 4,
    NEXUS_ITEM_MISC    = 7,
    NEXUS_ITEM_CAT_MAX = 8
} Nexus_ItemCategory;
/* Legacy alias */
#define NEXUS_ITEM_ARMOR NEXUS_ITEM_ARMOUR
#define NEXUS_ITEM_COUNT NEXUS_ITEM_CAT_MAX

/* Item definition — ITEM.IBS 40-byte record (DMWeb documentation).
 * Fields map to byte offsets in the IBS item declaration. */
typedef struct {
    const char      *name;
    Nexus_ItemCategory category;  /* Byte1: 0=Weapon 1=Armour 2=Food 3=Potion 4=Scroll 7=Misc */
    uint8_t          carry_locations; /* Byte2: bitfield (bit0=consumable..bit6=misc) */
    uint8_t          ibs_flags;  /* Byte3: bit7=has floor image, bit1=unknown */
    int              weight;     /* Byte8 */
    int              attack;     /* derived / Byte14-15 area */
    int              defense;    /* derived / Byte11 area */
    int              flags;      /* NEXUS_ITEMF_* */
    uint8_t          action_id[3]; /* Byte16-18: action IDs */
    uint16_t         inv_image;  /* Word20: inventory image index */
    uint16_t         floor_image; /* Word22: floor image index (0xFFFF=use inv) */
    uint16_t         name_string; /* Word24 */
    uint16_t         desc_string; /* Word26 */
    uint16_t         action_string[3]; /* Word28-32 */
    uint16_t         attribute;  /* Word36: distance(weapon)/armor-val/food-val */
} Nexus_ItemDef;

/* Equipment slot encoding */
typedef enum {
    NEXUS_SLOT_NONE      = 0,
    NEXUS_SLOT_HEAD      = 1,
    NEXUS_SLOT_TORSO     = 2,
    NEXUS_SLOT_LEGS      = 3,
    NEXUS_SLOT_FEET      = 4,
    NEXUS_SLOT_HANDS     = 5,
    NEXUS_SLOT_WEAPON    = 6,
    NEXUS_SLOT_SHIELD    = 7,
    NEXUS_SLOT_RING1     = 8,
    NEXUS_SLOT_RING2     = 9,
    NEXUS_SLOT_AMULET    = 10,
    NEXUS_SLOT_INVENTORY = 99  /* marker for inventory grid */
} Nexus_EquipSlot;

#define NEXUS_INVENTORY_SLOTS  30
#define NEXUS_EQUIP_SLOTS      11  /* head..amulet, plus 2 rings */
#define NEXUS_MAX_CARRY_WEIGHT 400 /* max carry weight before speed penalty */

#define NEXUS_ITEMF_CONSUMABLE  0x0001
#define NEXUS_ITEMF_EQUIPPABLE   0x0002
#define NEXUS_ITEMF_STACKABLE    0x0004
#define NEXUS_ITEMF_KEY          0x0008
#define NEXUS_ITEMF_NO_DROP      0x0010  /* quest items */

typedef struct {
    int item_id;          /* index into global item catalog, -1 = empty */
    int quantity;          /* stack count, 1 for non-stackable */
} Nexus_InventorySlot;

/* Floor item — dropped/chest item in the dungeon */
typedef struct {
    int item_id;
    int quantity;
    int x, y;             /* dungeon position */
    int on_ground;        /* 1 = on ground, 0 = in container */
    uint8_t source_attribute1; /* DGN Structure1Fa byte 5 */
    uint8_t source_attribute2; /* DGN Structure1Fa byte 7 (charges) */
    int source_dgn_entry;      /* source entry ordinal, or -1 */
} Nexus_FloorItem;

/* Cursor state — item currently held by player */
typedef struct {
    int item_id;
    int quantity;
    int source_type;      /* 0=none, 1=inventory, 2=floor */
    int source_idx;       /* champion slot index or floor item index */
} Nexus_CursorItem;

/* ═══════════════════════════════════════════════════════════════════
 * Live Nexus lookup is source-gated: no item definition exists until a
 * verified Saturn ITEM.IBS bank has been bound.
 * ═══════════════════════════════════════════════════════════════════ */
int nexus_itemdef_count(void);
const Nexus_ItemDef *nexus_itemdef_get(int id);
/* Bind raw 40-byte ITEM.IBS records (DMWeb format).
 * data points to count * 40 bytes of item declarations. */
void nexus_itemdef_bind_ibs_raw(const uint8_t *data, int count);
/* Legacy field-split binding (kept for existing tests). */
void nexus_itemdef_bind_ibs_declarations(const uint8_t *category,
                                         const uint8_t *weight,
                                         const uint16_t *name_string,
                                         const uint16_t *desc_string,
                                         const uint16_t *action1_string,
                                         const uint16_t *action2_string,
                                         const uint16_t *action3_string,
                                         int count);
void nexus_itemdef_clear_ibs_declarations(void);
/* Forward-declared; defined in nexus_v1_dungeon.h */
#ifndef NEXUS_V1_DUNGEON_H
typedef struct { int valid; } Nexus_V1_ItemIbsBank_Fwd;
#endif
void nexus_itemdef_bind_ibs_bank(const void *bank, int count);
const char *nexus_itemdef_category_name(Nexus_ItemCategory cat);

/* ═══════════════════════════════════════════════════════════════════
 * Inventory management
 * ═══════════════════════════════════════════════════════════════════ */

/* Init an inventory to empty */
void nexus_inventory_init(Nexus_InventorySlot *inv, int count);

/* Get item in slot (returns item def or NULL) */
const Nexus_ItemDef *nexus_inventory_get(const Nexus_InventorySlot *inv, int slot);

/* Add item to first available slot. Returns slot index or -1. */
int nexus_inventory_add(Nexus_InventorySlot *inv, int count, int item_id, int qty);

/* Remove item from slot */
void nexus_inventory_remove(Nexus_InventorySlot *inv, int slot);

/* Move item within inventory */
int nexus_inventory_move(Nexus_InventorySlot *inv, int from, int to);

/* Get total weight of inventory */
int nexus_inventory_weight(const Nexus_InventorySlot *inv, int count);

/* Count items of a given category in inventory */
int nexus_inventory_count_category(const Nexus_InventorySlot *inv, int count, Nexus_ItemCategory cat);

/* Find first slot containing item_id (or -1) */
int nexus_inventory_find(const Nexus_InventorySlot *inv, int count, int item_id);

/* Equip item from inventory slot. Returns slot that was cleared or -1.
 * Armor-slot mutation remains unavailable until the Saturn action/slot
 * dispatcher is source-bound; no inherited item-ID mapping is permitted. */
int nexus_inventory_equip(Nexus_InventorySlot *inv, int slot,
                           int weapon_slot, int shield_slot,
                           int ring1_slot, int ring2_slot,
                           int head_slot, int torso_slot,
                           int legs_slot, int feet_slot, int hands_slot,
                           int amulet_slot);

/* Unequip and return to inventory. Returns slot index or -1. */
int nexus_inventory_unequip(Nexus_InventorySlot *inv, int count,
                             int equip_slot,
                             int weapon_slot, int shield_slot,
                             int ring1_slot, int ring2_slot,
                             int head_slot, int torso_slot,
                             int legs_slot, int feet_slot, int hands_slot,
                             int amulet_slot);

/* Cursor item helpers */
void nexus_cursor_clear(Nexus_CursorItem *cursor);
int nexus_cursor_can_pickup(const Nexus_CursorItem *cursor);
int nexus_cursor_pickup(Nexus_CursorItem *cursor, int item_id, int qty,
                         int source_type, int source_idx);
int nexus_cursor_place(Nexus_CursorItem *cursor);

/* Floor item management */
void nexus_floor_init(void);
/* Caller-supplied loot/drop mutation is not a retail Nexus route. */
int nexus_floor_drop(int x, int y, int item_id, int qty);
/* Source-preserving floor admission.  The two attributes are retained as
 * raw DGN bytes; they are not interpreted as gameplay until the Saturn
 * action/slot consumer is source-bound. */
int nexus_floor_drop_source(int x, int y, int item_id, int qty,
                            uint8_t attribute1, uint8_t attribute2,
                            int source_dgn_entry);
int nexus_floor_pickup(int floor_idx, int *out_item_id, int *out_qty);
int nexus_floor_count_at(int x, int y);
int nexus_floor_get_at(int x, int y, int idx, int *out_item_id, int *out_qty);
int nexus_floor_get_at_source(int x, int y, int idx, int *out_item_id,
                              int *out_qty, uint8_t *out_attribute1,
                              uint8_t *out_attribute2, int *out_dgn_entry);

#endif /* NEXUS_V1_INVENTORY_H */
