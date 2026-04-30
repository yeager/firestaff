#include "inventory_panel_slot_routes_pc34_compat.h"
#include <string.h>

#define DM1_THING_TYPE_SCROLL_PC34_COMPAT 7u
#define DM1_THING_TYPE_CONTAINER_PC34_COMPAT 9u

static const InventorySlotRoutePc34Compat kInventorySlotRoutes[] = {
    {  0u,  8u, 28u, 507u, "ready hand",       "COMMAND.C:419 maps C028 to C507; DATA.C:986 binds slotBox 8 to C507" },
    {  1u,  9u, 29u, 508u, "action hand",      "COMMAND.C:420 maps C029 to C508; DATA.C:987 binds slotBox 9 to C508" },
    {  2u, 10u, 30u, 509u, "head",             "COMMAND.C:421 maps C030 to C509; DATA.C:988 binds slotBox 10 to C509" },
    {  3u, 11u, 31u, 510u, "torso",            "COMMAND.C:422 maps C031 to C510; DATA.C:989 binds slotBox 11 to C510" },
    {  4u, 12u, 32u, 511u, "legs",             "COMMAND.C:423 maps C032 to C511; DATA.C:990 binds slotBox 12 to C511" },
    {  5u, 13u, 33u, 512u, "feet",             "COMMAND.C:424 maps C033 to C512; DATA.C:991 binds slotBox 13 to C512" },
    {  6u, 14u, 34u, 513u, "pouch 2",          "COMMAND.C:425 maps C034 to C513; DATA.C:992 binds slotBox 14 to C513" },
    {  7u, 15u, 35u, 514u, "quiver line2 1",   "COMMAND.C:428 maps C035 to C514; DATA.C:993 binds slotBox 15 to C514" },
    {  8u, 16u, 36u, 515u, "quiver line1 2",   "COMMAND.C:429 maps C036 to C515; DATA.C:994 binds slotBox 16 to C515" },
    {  9u, 17u, 37u, 516u, "quiver line2 2",   "COMMAND.C:430 maps C037 to C516; DATA.C:995 binds slotBox 17 to C516" },
    { 10u, 18u, 38u, 517u, "neck",             "COMMAND.C:431 maps C038 to C517; DATA.C:996 binds slotBox 18 to C517" },
    { 11u, 19u, 39u, 518u, "pouch 1",          "COMMAND.C:432 maps C039 to C518; DATA.C:997 binds slotBox 19 to C518" },
    { 12u, 20u, 40u, 519u, "quiver line1 1",   "COMMAND.C:433 maps C040 to C519; DATA.C:998 binds slotBox 20 to C519" },
    { 13u, 21u, 41u, 520u, "backpack line1 1", "COMMAND.C:434 maps C041 to C520; DATA.C:999 binds slotBox 21 to C520" },
    { 14u, 22u, 42u, 521u, "backpack line2 2", "COMMAND.C:435 maps C042 to C521; DATA.C:1000 binds slotBox 22 to C521" },
    { 15u, 23u, 43u, 522u, "backpack line2 3", "COMMAND.C:436 maps C043 to C522; DATA.C:1001 binds slotBox 23 to C522" },
    { 16u, 24u, 44u, 523u, "backpack line2 4", "COMMAND.C:437 maps C044 to C523; DATA.C:1002 binds slotBox 24 to C523" },
    { 17u, 25u, 45u, 524u, "backpack line2 5", "COMMAND.C:438 maps C045 to C524; DATA.C:1003 binds slotBox 25 to C524" },
    { 18u, 26u, 46u, 525u, "backpack line2 6", "COMMAND.C:439 maps C046 to C525; DATA.C:1004 binds slotBox 26 to C525" },
    { 19u, 27u, 47u, 526u, "backpack line2 7", "COMMAND.C:440 maps C047 to C526; DATA.C:1005 binds slotBox 27 to C526" },
    { 20u, 28u, 48u, 527u, "backpack line2 8", "COMMAND.C:441 maps C048 to C527; DATA.C:1006 binds slotBox 28 to C527" },
    { 21u, 29u, 49u, 528u, "backpack line2 9", "COMMAND.C:442 maps C049 to C528; DATA.C:1007 binds slotBox 29 to C528" },
    { 22u, 30u, 50u, 529u, "backpack line1 2", "COMMAND.C:443 maps C050 to C529; DATA.C:1008 binds slotBox 30 to C529" },
    { 23u, 31u, 51u, 530u, "backpack line1 3", "COMMAND.C:444 maps C051 to C530; DATA.C:1009 binds slotBox 31 to C530" },
    { 24u, 32u, 52u, 531u, "backpack line1 4", "COMMAND.C:445 maps C052 to C531; DATA.C:1010 binds slotBox 32 to C531" },
    { 25u, 33u, 53u, 532u, "backpack line1 5", "COMMAND.C:446 maps C053 to C532; DATA.C:1011 binds slotBox 33 to C532" },
    { 26u, 34u, 54u, 533u, "backpack line1 6", "COMMAND.C:447 maps C054 to C533; DATA.C:1012 binds slotBox 34 to C533" },
    { 27u, 35u, 55u, 534u, "backpack line1 7", "COMMAND.C:448 maps C055 to C534; DATA.C:1013 binds slotBox 35 to C534" },
    { 28u, 36u, 56u, 535u, "backpack line1 8", "COMMAND.C:449 maps C056 to C535; DATA.C:1014 binds slotBox 36 to C535" },
    { 29u, 37u, 57u, 536u, "backpack line1 9", "COMMAND.C:450 maps C057 to C536; DATA.C:1015 binds slotBox 37 to C536" }
};

unsigned int INVENTORY_Compat_GetInventorySlotRouteCount(void) {
    return (unsigned int)(sizeof(kInventorySlotRoutes) / sizeof(kInventorySlotRoutes[0]));
}

int INVENTORY_Compat_GetInventorySlotRoute(unsigned int inventorySlotIndex,
                                           InventorySlotRoutePc34Compat* outRoute) {
    if (!outRoute || inventorySlotIndex >= INVENTORY_Compat_GetInventorySlotRouteCount()) return 0;
    *outRoute = kInventorySlotRoutes[inventorySlotIndex];
    return 1;
}

InventoryPanelRoutePc34Compat INVENTORY_Compat_EvaluatePanelRoute(InventoryPanelInputPc34Compat input) {
    if (input.pressingMouth) return INVENTORY_PANEL_ROUTE_FOOD_WATER_POISONED_PC34_COMPAT;
    if (input.pressingEye) {
        return input.leaderEmptyHanded ? INVENTORY_PANEL_ROUTE_SKILLS_STATISTICS_PC34_COMPAT
                                       : INVENTORY_PANEL_ROUTE_LEADER_HAND_OBJECT_PC34_COMPAT;
    }
    if (input.candidateChampionOpen) return INVENTORY_PANEL_ROUTE_RESURRECT_REINCARNATE_PC34_COMPAT;
    if (input.actionHandThingType == DM1_THING_TYPE_CONTAINER_PC34_COMPAT) return INVENTORY_PANEL_ROUTE_CHEST_OBJECT_PC34_COMPAT;
    if (input.actionHandThingType == DM1_THING_TYPE_SCROLL_PC34_COMPAT) return INVENTORY_PANEL_ROUTE_SCROLL_OBJECT_PC34_COMPAT;
    return INVENTORY_PANEL_ROUTE_FOOD_WATER_POISONED_PC34_COMPAT;
}

const char* INVENTORY_Compat_GetPanelSlotRouteEvidence(void) {
    return "COMMAND.C:419-451 maps inventory slots, mouth/eye, and panel to viewport-relative zones; DATA.C:986-1015 binds inventory slot boxes 8..37 to C507..C536; CHAMDRAW.C:1060-1078 prioritizes mouth, eye, then F0347 panel refresh; PANEL.C:1651-1691 routes normal panel content from candidate/action-hand thing type; PANEL.C:1788-1817 and 2111-2158 define mouth/eye press panel behavior.";
}
