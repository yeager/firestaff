/*
 * dm1_v1_object_interaction_pc34_compat.h — DM1 V1 Object/Item Interaction System
 *
 * Source-locked to ReDMCSB:
 *   OBJECT.C:    F0032_OBJECT_GetType, F0033_OBJECT_GetIconIndex
 *   CHAMPION.C:  F0297_CHAMPION_PutObjectInLeaderHand
 *                F0298_CHAMPION_GetObjectRemovedFromLeaderHand
 *                F0299_CHAMPION_ApplyObjectModifiersToStatistics
 *                F0300_CHAMPION_GetObjectRemovedFromSlot
 *                F0301_CHAMPION_AddObjectInSlot
 *                F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox
 *                F0305_CHAMPION_GetThrowingStaminaCost
 *                F0309_CHAMPION_GetMaximumLoad
 *                F0328_CHAMPION_IsObjectThrown
 *   CHEST.C:     F0333_INVENTORY_OpenAndDrawChest
 *                F0334_INVENTORY_CloseChest
 *   COMMAND.C:   F0378_COMMAND_ProcessType81_ClickInPanel (chest panel)
 *   DEFS.H:      Slot constants, object type constants, icon indices,
 *                weight table, AllowedSlots bitmask
 */
#ifndef DM1_V1_OBJECT_INTERACTION_PC34_COMPAT_H
#define DM1_V1_OBJECT_INTERACTION_PC34_COMPAT_H

#include <stdint.h>

/* ── Thing representation (DEFS.H) ───────────────────────────────── */
#define DM1_OBJ_THING_NONE       0xFFFFu
#define DM1_OBJ_THING_ENDOFLIST  0xFFFEu
#define DM1_OBJ_ICON_NONE        ((int16_t)-1)

/* ── Thing type extraction (DEFS.H M012_TYPE) ────────────────────── */
#define DM1_OBJ_TYPE(thing)  (((thing) & 0x3C00) >> 10)

/* ── Thing types (DEFS.H) ────────────────────────────────────────── */
#define DM1_THING_TYPE_WEAPON    5
#define DM1_THING_TYPE_ARMOUR    6
#define DM1_THING_TYPE_SCROLL    7
#define DM1_THING_TYPE_POTION    8
#define DM1_THING_TYPE_CONTAINER 9
#define DM1_THING_TYPE_JUNK     10

/* ── Slot indices (DEFS.H) ───────────────────────────────────────── */
#define DM1_SLOT_READY_HAND       0
#define DM1_SLOT_ACTION_HAND      1
#define DM1_SLOT_HEAD             2
#define DM1_SLOT_TORSO            3
#define DM1_SLOT_LEGS             4
#define DM1_SLOT_FEET             5
#define DM1_SLOT_NECK            10
#define DM1_SLOT_CHEST_1         30
#define DM1_SLOT_COUNT           30

/* ── Icon indices for special items (DEFS.H) ─────────────────────── */
#define DM1_ICON_CHEST_CLOSED       144
#define DM1_ICON_SCROLL_OPEN         30
#define DM1_ICON_SCROLL_CLOSED       31
#define DM1_ICON_RABBITS_FOOT       137
#define DM1_ICON_TORCH_UNLIT          4
#define DM1_ICON_TORCH_LIT            7
#define DM1_ICON_ILLUMULET_UNEQUIPPED 12
#define DM1_ICON_ILLUMULET_EQUIPPED   13
#define DM1_ICON_SYMAL_UNEQUIPPED     10
#define DM1_ICON_SYMAL_EQUIPPED       11
#define DM1_ICON_MACE_OF_ORDER        45
#define DM1_ICON_POTION_FIRST        148
#define DM1_ICON_WATER_FLASK         163
#define DM1_ICON_EMPTY_FLASK         195

/* ── Statistic indices (DEFS.H) ──────────────────────────────────── */
#define DM1_STAT_LUCK         0
#define DM1_STAT_STRENGTH     1
#define DM1_STAT_DEXTERITY    2
#define DM1_STAT_WISDOM       3
#define DM1_STAT_ANTIMAGIC    5
#define DM1_STAT_ANTIFIRE     6
#define DM1_STAT_MANA         8

/* ── Allowed slot masks (DEFS.H) ─────────────────────────────────── */
#define DM1_ALLOWED_MOUTH      0x0001u
#define DM1_ALLOWED_HANDS      0x0200u
#define DM1_ALLOWED_CONTAINER  0x0400u

/* ── Chest slots ─────────────────────────────────────────────────── */
#define DM1_CHEST_SLOT_COUNT  8

/* ── Use actions ─────────────────────────────────────────────────── */
#define DM1_USE_ACTION_EAT     0
#define DM1_USE_ACTION_DRINK   1
#define DM1_USE_ACTION_READ    2
#define DM1_USE_ACTION_NONE   -1

/* ── Object info (G0237_as_Graphic559_ObjectInfo) ────────────────── */
typedef struct {
    int16_t  type;
    uint16_t allowedSlots;
    int16_t  useAction;
} DM1_ObjectInfo;

/* ── Per-object state ────────────────────────────────────────────── */
typedef struct {
    uint16_t thing;
    int16_t  iconIndex;
    int16_t  weight;
    int16_t  objectInfoIndex;
    uint8_t  cursed;
    uint8_t  poisoned;
    uint8_t  broken;
    uint8_t  lit;
    uint8_t  chargeCount;
    int16_t  useAction;
} DM1_ObjectState;

/* ── Champion inventory (for object interaction) ─────────────────── */
typedef struct {
    uint16_t slots[DM1_SLOT_COUNT];
    uint16_t load;
    int16_t  currentHealth;
    int16_t  maxStamina;
    int16_t  currentStamina;
    uint8_t  statistics[7][3];  /* [stat][max=0/current=1/min=2] */
    int16_t  maxMana;
    uint8_t  wounds;
    uint16_t attributes;
} DM1_ChampionInv;

/* ── Leader hand (G4055_s_LeaderHandObject) ──────────────────────── */
typedef struct {
    uint16_t thing;
    int16_t  iconIndex;
    int16_t  weight;
} DM1_LeaderHand;

/* ── Chest state (G0425/G0426) ───────────────────────────────────── */
typedef struct {
    uint16_t openChestThing;
    uint16_t chestSlots[DM1_CHEST_SLOT_COUNT];
} DM1_ChestState;

/* ── Object interaction context ──────────────────────────────────── */
typedef struct {
    DM1_LeaderHand leaderHand;
    DM1_ChestState chest;
    int            leaderIndex;
    int            leaderEmptyHanded;
    /* Slot bitmask table: which slot accepts which object types
     * G0038_ai_Graphic562_SlotMasks[slotIndex] */
    uint16_t       slotMasks[DM1_SLOT_COUNT + DM1_CHEST_SLOT_COUNT];
} DM1_ObjCtx;

/* ── Throw result ────────────────────────────────────────────────── */
typedef struct {
    int      success;
    int16_t  kineticEnergy;
    int16_t  attack;
    int16_t  stepEnergy;
    int16_t  staminaCost;
} DM1_ThrowResult;

/* ── Use result ──────────────────────────────────────────────────── */
typedef struct {
    int      consumed;
    int16_t  foodGain;
    int16_t  waterGain;
    int16_t  healthGain;
    int16_t  staminaGain;
    int16_t  manaGain;
    int      poisoned;
    int      scrollRead;
} DM1_UseResult;

/* ── API ─────────────────────────────────────────────────────────── */
void dm1_obj_seed_rng(uint32_t seed);
int  dm1_obj_random(int modulus);

void dm1_obj_ctx_init(DM1_ObjCtx* ctx);
void dm1_obj_champion_inv_init(DM1_ChampionInv* ch);

/* F0140_DUNGEON_GetObjectWeight */
int16_t dm1_obj_get_weight(const DM1_ObjectState* obj);

/* F0309_CHAMPION_GetMaximumLoad */
uint16_t dm1_obj_get_max_load(const DM1_ChampionInv* ch);

/* F0297: put object in leader hand */
void dm1_obj_put_in_leader_hand(DM1_ObjCtx* ctx, DM1_ChampionInv champions[],
                                 uint16_t thing, int16_t iconIndex, int16_t weight);

/* F0298: remove object from leader hand */
uint16_t dm1_obj_remove_from_leader_hand(DM1_ObjCtx* ctx, DM1_ChampionInv champions[]);

/* F0300: remove object from champion slot */
uint16_t dm1_obj_remove_from_slot(DM1_ObjCtx* ctx, DM1_ChampionInv* ch,
                                   int slotIndex, int16_t weight);

/* F0301: add object to champion slot */
int dm1_obj_add_to_slot(DM1_ObjCtx* ctx, DM1_ChampionInv* ch,
                         uint16_t thing, int slotIndex,
                         int16_t iconIndex, int16_t weight);

/* F0302: click on slot box (swap leader hand / slot) */
int dm1_obj_click_on_slot(DM1_ObjCtx* ctx, DM1_ChampionInv champions[],
                           int champIndex, int slotIndex,
                           uint16_t allowedSlotsMask);

/* F0299: apply/remove stat modifiers */
void dm1_obj_apply_stat_modifiers(DM1_ChampionInv* ch, int slotIndex,
                                   int16_t iconIndex, int modifierFactor);

/* F0333: open chest */
void dm1_obj_open_chest(DM1_ObjCtx* ctx, uint16_t chestThing,
                         const uint16_t contents[], int contentCount);

/* F0334: close chest */
void dm1_obj_close_chest(DM1_ObjCtx* ctx);

/* Use item (eat/drink/read) */
DM1_UseResult dm1_obj_use_item(const DM1_ObjectState* obj);

/* F0305: throwing stamina cost */
int16_t dm1_obj_get_throwing_stamina_cost(int16_t weight);

/* F0328: throw object */
DM1_ThrowResult dm1_obj_throw(DM1_ObjCtx* ctx, DM1_ChampionInv* ch,
                               int slotIndex, int16_t weight,
                               int strength, int throwSkillLevel);

/* Pick up from floor → leader hand */
int dm1_obj_pick_up_from_floor(DM1_ObjCtx* ctx, DM1_ChampionInv champions[],
                                uint16_t thing, int16_t iconIndex, int16_t weight);

/* Drop from leader hand → floor */
int dm1_obj_drop_to_floor(DM1_ObjCtx* ctx, DM1_ChampionInv champions[]);

#endif /* DM1_V1_OBJECT_INTERACTION_PC34_COMPAT_H */
