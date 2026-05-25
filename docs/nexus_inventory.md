# Nexus V1 — Inventory System

**Audit date:** 2026-05-25
**Sources:** `include/nexus_v1_champions.h`, `include/firestaff_inventory_ui.h`, `src/ui/firestaff_inventory_ui.c` (if exists), `src/nexus/nexus_v1_champions.c`, `docs/nexus_champions.md`, `docs/dm2_inventory.md`

---

## 1. Champion Inventory Structure

The Nexus_V1_Champion struct stores inventory as a flat array of item indices:

```c
uint8_t inventory[30];    /* item indices — 30 slots */
```

**Key observation:** 30 inventory slots, compared to DM1s 12-slot grid.
This is a Firestaff extension. The 30 slots likely cover the main bag plus
hand/ring/amulet slots encoded in a flat array, or represent a different
internal encoding than DM1.

Source: `include/nexus_v1_champions.h`

---

## 2. Inventory UI Structure

The Firestaff UI layer defines its own inventory model:

```c
#define INV_SLOTS 30
#define INV_GRID_COLS 5
#define INV_GRID_ROWS 6
#define INV_CELL_SIZE 10

typedef struct {
    int item_id;
    int count;
    char name[32];
    int weight;
    int equipped;  /* slot: 0=none, 1=head, 2=torso, etc */
} FS_InventoryItem;

typedef struct {
    char name[32];
    int portrait_index;
    int health, max_health;
    int stamina, max_stamina;
    int mana, max_mana;
    int food, water;
    int alive;
    FS_InventoryItem items[INV_SLOTS];
    int item_count;
} FS_ChampionUI;
```

The UI layer uses a 5-column x 6-row grid (30 slots), matching the champion struct.

Source: `include/firestaff_inventory_ui.h`

---

## 3. Equipment Slots (UI Layer)

The UI header describes equipment slots:
```
Equipment slots: head, torso, legs, feet, hands, weapon, shield
```

FS_InventoryItem.equipped field encoding:
- 0 = not equipped (in inventory)
- 1 = head, 2 = torso, 3 = legs, 4 = feet, 5 = hands, 6 = weapon, 7 = shield

This is 7 equipment slots + 30 inventory slots = 37 total per champion.
This exceeds DM1s 12 bag + 2 hands + 2 rings + 2 amulets = 18 slots.
The 30-slot + 7-slot model needs parity verification against DM1 save data.

Source: `include/firestaff_inventory_ui.h`

---

## 4. Panel Layout

Original 320x200 screen layout:

| Region | Coordinates | Dimensions | Contents |
|---|---|---|---|
| Viewport | 33-256, 0-135 | 224x136 | 3D first-person view |
| Right panel | 257-319, 0-135 | 63x136 | Champion portraits |
| Bottom panel | 0-319, 136-199 | 320x64 | Inventory / status |

The inventory panel renders at the bottom of the screen, below the viewport.
Champion portraits are in the right panel.

Source: `include/firestaff_inventory_ui.h`

---

## 5. Inventory Management Functions

```c
void fs_inv_init(FS_InventoryUI *inv);
void fs_inv_render(const FS_InventoryUI *inv, uint8_t *framebuffer, const uint32_t *palette);
void fs_inv_select_champion(FS_InventoryUI *inv, int index);
void fs_inv_move_cursor(FS_InventoryUI *inv, int dx, int dy);
int fs_inv_pick_up(FS_InventoryUI *inv);
int fs_inv_put_down(FS_InventoryUI *inv);
```

Core UI operations:
- init — initialize inventory UI state
- render — draw inventory panel to framebuffer
- select_champion — switch which champions inventory is shown
- move_cursor — navigate the inventory grid
- pick_up — pick up an item (attach to cursor)
- put_down — place held item in a slot

Source: `include/firestaff_inventory_ui.h`

---

## 6. Champion Pool

```c
#define NEXUS_MAX_CHAMPIONS 24
#define NEXUS_MAX_PARTY 4

typedef struct {
    Nexus_V1_Champion champions[NEXUS_MAX_CHAMPIONS];
    int champion_count;
    int party[NEXUS_MAX_PARTY];
    int party_count;
    int leader_index;
} Nexus_V1_ChampionPool;
```

- Hall of Champions: 24 champions (same as DM1)
- Active party: 4 champions maximum (same as DM1)
- leader_index — which champion is currently leading

Source: `include/nexus_v1_champions.h`

---

## 7. Item Weight and Encumbrance

Items have a weight field (in the encyclopedia). Total carried weight affects
champion movement speed. Over a threshold (~400 lbs in DM1), movement speed
is halved. The encumbrance check is likely in the movement system and was
not found in the Nexus V1 source audit.

Source: `src/ui/firestaff_item_encyclopedia.c`, `docs/dm2_inventory.md`

---

## 8. Food and Water (Separate from Inventory)

Champions have food and water stats stored directly in the struct:
```c
int food, water;  /* resource tracking, not in inventory slots */
```

Depleted over time. Restored by:
- Food: eating food items (Corn, weight=3)
- Water: drinking Water Flask items or consuming potions

Food and water are intrinsic champion stats, not items in a slot.

Source: `include/nexus_v1_champions.h`, `include/firestaff_inventory_ui.h`

---

## 9. Item Interaction: Held Item Tracking

The UI layer supports cursor-held items:
```c
int held_item;  /* -1 = nothing, else item index */
```

This enables swap operations between inventory slots. Picking up from the floor
and placing in inventory slots are handled by fs_inv_pick_up / fs_inv_put_down.

Source: `include/firestaff_inventory_ui.h`

---

## 10. Nexus V1 vs DM1 vs DM2

| Aspect | DM1 | Nexus V1 | DM2 |
|---|---|---|---|
| Bag slots per champion | 12 | 30 (flat array) | 12 |
| Hand slots | 2 | 2 | 2 |
| Ring slots | 2 | (in 30?) | 2 |
| Amulet slots | 2 | (in 30?) | 2 |
| Grid layout | 4x3 | 5x6 | 4x3 |
| Equipment slots | Separate | 7 named slots | Separate |
| Max party size | 4 | 4 | 4 |
| Hall of Champions | 24 | 24 | 24 |
| Food/water tracking | Yes | Yes | Yes |
| Container items | Yes | Yes | Yes |
| Item weight/encumbrance | Yes | Yes | Yes |
| Item encyclopedia UI | No | Yes | Yes |

**Major difference:** The 30-slot `inventory[30]` vs DM1s 12-slot grid.
This is the most significant structural difference and may be a Firestaff
extension, or may represent a different internal item encoding.

Source: cross-reference of all structs and enum definitions

---

## 11. Status: PARTIALLY SOURCE-LOCKED

- Champion inventory array (30 slots): **source-locked** — `nexus_v1_champions.h`
- Inventory UI grid layout: **source-locked** — `firestaff_inventory_ui.h`
- Equipment slot model (7 slots): **source-locked** — `firestaff_inventory_ui.h`
- Champion pool (24 hall, 4 party): **source-locked** — `nexus_v1_champions.h`
- Food/water: **source-locked** — `nexus_v1_champions.h`
- Item weight: **source-locked** — `firestaff_item_encyclopedia.c`
- Encumbrance effect on movement: **NOT FOUND** in Nexus V1 source
- UI item rendering: **source-locked** — `firestaff_inventory_ui.c`
- **NOTE:** 30-slot vs 12-slot discrepancy is a significant structural difference
  from DM1 that needs parity verification against DM1 save data
