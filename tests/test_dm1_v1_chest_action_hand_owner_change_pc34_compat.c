#include "firestaff/dm1/v1/chest/dm1_v1_chest_action_hand_owner_change_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int assertions;
    int failures;
} Counters;

static void check_true(Counters* c, int condition, const char* label)
{
    ++c->assertions;
    if (!condition) {
        ++c->failures;
        printf("FAIL %s\n", label);
    }
}

static void check_int(Counters* c, int actual, int expected, const char* label)
{
    ++c->assertions;
    if (actual != expected) {
        ++c->failures;
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
    }
}

static void check_contains(Counters* c,
                           const char* text,
                           const char* needle,
                           const char* label)
{
    ++c->assertions;
    if (!text || !needle || strstr(text, needle) == 0) {
        ++c->failures;
        printf("FAIL %s missing=%s\n", label, needle ? needle : "(null)");
    }
}

static void check_source(Counters* c)
{
    const char* evidence =
        dm1_v1_chest_action_hand_owner_change_source_evidence_pc34();
    const DM1_V1_ChestActionHandOwnerChangeSpecPc34* spec =
        dm1_v1_chest_action_hand_owner_change_spec_pc34();

    check_contains(c, evidence, "CHEST.C F0333:30-67", "F0333 evidence");
    check_contains(c, evidence, "CHEST.C F0334:113-132", "F0334 evidence");
    check_contains(c, evidence, "CHAMPION.C F0297:243-268", "F0297 evidence");
    check_contains(c, evidence, "CHAMPION.C F0298:270-298", "F0298 evidence");
    check_contains(c, evidence, "CHAMPION.C F0300:511-515", "F0300 evidence");
    check_contains(c, evidence, "CHAMPION.C F0301:606-614", "F0301 evidence");
    check_contains(c, evidence, "CHAMPION.C F0302:662-714", "F0302 evidence");
    check_contains(c, evidence, "PANEL.C F0347:1639-1691", "F0347 evidence");
    check_contains(c, evidence, "M569_PANEL_CHEST", "M569 evidence");
    check_contains(c, evidence, "M565_PANEL_FOOD_WATER_POISONED",
                   "M565 evidence");
    check_contains(c, evidence, "C09_THING_TYPE_CONTAINER", "thing type 9");
    check_contains(c, evidence, "C05_THING_TYPE_WEAPON", "thing type 5");
    check_contains(c, evidence, "C07_THING_TYPE_SCROLL", "thing type 7");
    check_contains(c, evidence, "MASK0x0800_PANEL", "MASK 0x0800 evidence");
    check_contains(c, evidence, "CHANGE7_27_FIX", "CHANGE7_27_FIX");
    check_contains(c, evidence, "CHANGE8_09_FIX", "CHANGE8_09_FIX");
    check_contains(c, evidence, "MEDIA278", "MEDIA278");
    check_contains(c, evidence, "MEDIA348", "MEDIA348");

    check_contains(c, spec->chestOpenAnchor, "F0333:30-67", "spec F0333");
    check_contains(c, spec->chestCloseAnchor, "F0334:113-132", "spec F0334");
    check_contains(c, spec->championHandPutAnchor, "F0297:243-268",
                   "spec F0297");
    check_contains(c, spec->championHandRemoveAnchor, "F0298:270-298",
                   "spec F0298");
    check_contains(c, spec->championSlotRemoveAnchor, "F0300:511-515",
                   "spec F0300");
    check_contains(c, spec->championSlotAddAnchor, "F0301:606-614",
                   "spec F0301");
    check_contains(c, spec->championSlotBoxClickAnchor, "F0302:662-714",
                   "spec F0302");
    check_contains(c, spec->panelRedrawAnchor, "F0347:1639-1691",
                   "spec F0347");
    check_contains(c, spec->defsAnchor, "M569_PANEL_CHEST", "spec defs chest");
    check_contains(c, spec->defsAnchor, "M565_PANEL_FOOD_WATER_POISONED",
                   "spec defs food");
    check_contains(c, spec->defsAnchor, "G0426", "spec G0426");
    check_contains(c, spec->defsAnchor, "G0425", "spec G0425");
    check_contains(c, spec->defsAnchor, "G0424", "spec G0424");
    check_contains(c, spec->defsAnchor, "G0423", "spec G0423");
    check_contains(c, spec->defsAnchor, "C537..C544", "spec C537..C544");
    check_contains(c, spec->thingTypeAnchor, "C00_THING_TYPE_DOOR",
                   "spec thing type door");
    check_contains(c, spec->thingTypeAnchor, "C05_THING_TYPE_WEAPON",
                   "spec thing type weapon");
    check_contains(c, spec->thingTypeAnchor, "C09_THING_TYPE_CONTAINER",
                   "spec thing type container");
    check_contains(c, spec->thingTypeAnchor, "C07_THING_TYPE_SCROLL",
                   "spec thing type scroll");
    check_int(c, spec->sourceLockedContractOnly, 1, "spec source locked");
    check_int(c, spec->assetFree, 1, "spec asset free");
    check_contains(c, spec->disjointnessNote, "pass780", "disjoint pass780");
    check_contains(c, spec->disjointnessNote, "close stack-merge",
                   "disjoint close stack-merge");
    check_contains(c, spec->disjointnessNote, "reopen-after-leader-rotation",
                   "disjoint reopen-after-leader-rotation");
    check_contains(c, spec->disjointnessNote, "scroll-wheel",
                   "disjoint scroll-wheel");
    check_contains(c, spec->disjointnessNote, "empty-slot",
                   "disjoint empty-slot");
    check_contains(c, spec->disjointnessNote, "occupied-slot",
                   "disjoint occupied-slot");
    check_contains(c, spec->disjointnessNote, "non-leader hand",
                   "disjoint non-leader hand");
    check_contains(c, spec->disjointnessNote, "teleporter",
                   "disjoint teleporter");
    check_contains(c, spec->disjointnessNote, "save/load",
                   "disjoint save/load");
    check_contains(c, spec->disjointnessNote, "resurrect-pending",
                   "disjoint resurrect-pending");
    check_contains(c, spec->disjointnessNote, "F0347:1639-1691",
                   "disjoint F0347 close-chest-first");
}

static void check_probe(Counters* c,
                        DM1_V1_ChestActionHandOwnerChangeProbePc34* p)
{
    int ok;

    memset(p, 0, sizeof(*p));
    ok = dm1_v1_chest_action_hand_owner_change_run_pc34(p);

    check_int(c, ok, 1, "run ok");
    check_int(c, p->contractOnlyFailures, 0, "contract failures");
    check_true(c, p->contractOnlyAssertions >= 50, "model assertion floor");
    check_int(c, p->sourceLockedContractOnly, 1, "contract only");
    check_int(c, p->assetFree, 1, "asset free");

    check_int(c, p->leader, DM1_PC34_CAOC_LEADER, "leader");
    check_int(c, p->inventoryOwner, DM1_PC34_CAOC_INVENTORY_OWNER,
              "inventory owner");
    check_int(c, p->otherChampion, DM1_PC34_CAOC_OTHER_CHAMPION,
              "other champion");
    check_int(c, p->deadChampion, DM1_PC34_CAOC_DEAD_CHAMPION,
              "dead champion");
    check_int(c, p->partyChampionCount, DM1_PC34_CAOC_CHAMPION_COUNT,
              "party count");
    check_int(c, p->openChestThing, DM1_PC34_CAOC_OPEN_CHEST_THING,
              "open chest thing");

    /* Initial action hand. */
    check_int(c, p->initialActionHandItem, DM1_PC34_CAOC_CONTAINER_ITEM,
              "initial action hand item");
    check_int(c, p->initialActionHandAllowedSlots, DM1_PC34_ALLOWED_HANDS,
              "initial action hand mask");
    check_int(c, p->initialActionHandThingType,
              DM1_PC34_CAOC_THING_TYPE_CONTAINER,
              "initial action hand thing type");

    /* Open. */
    check_int(c, p->openResult, 1, "open result");
    check_int(c, p->openPanelContentBefore, DM1_PC34_CAOC_PANEL_INVENTORY,
              "panel content before open");
    check_int(c, p->openPanelContentAfter, DM1_PC34_CAOC_PANEL_CHEST,
              "panel content after open");
    check_int(c, p->openChampionAfter, DM1_PC34_CAOC_INVENTORY_OWNER,
              "open champion after");
    check_int(c, p->initialPanelContent, DM1_PC34_CAOC_PANEL_CHEST,
              "initial panel content");

    /* Visible G0425. */
    check_int(c, p->visibleCountBefore, DM1_PC34_CAOC_SLOT_COUNT,
              "visible count before");
    check_int(c, p->visibleBeforeSlot0Type,
              DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 0, "visible slot0 type");
    check_int(c, p->visibleBeforeSlot0Weight,
              DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 0, "visible slot0 weight");
    check_int(c, p->visibleBeforeSlot0Charges,
              DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 0, "visible slot0 charges");
    check_int(c, p->visibleBeforeSlot3Type,
              DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 3, "visible slot3 type");
    check_int(c, p->visibleBeforeSlot3Weight,
              DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 3, "visible slot3 weight");
    check_int(c, p->visibleBeforeSlot3Charges,
              DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 3, "visible slot3 charges");
    check_int(c, p->visibleBeforeSlot7Type,
              DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 7, "visible slot7 type");
    check_int(c, p->visibleBeforeSlot7Weight,
              DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 7, "visible slot7 weight");
    check_int(c, p->visibleBeforeSlot7Charges,
              DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 7, "visible slot7 charges");

    /* New action hand. */
    check_int(c, p->newActionHandItem, DM1_PC34_CAOC_NON_CONTAINER_ITEM,
              "new action hand item");
    check_int(c, p->newActionHandAllowedSlots, DM1_PC34_ALLOWED_HANDS,
              "new action hand mask");
    check_int(c, p->newActionHandThingType, DM1_PC34_CAOC_THING_TYPE_WEAPON,
              "new action hand thing type");
    check_true(c, p->newActionHandThingType !=
                      p->initialActionHandThingType,
               "thing type changed");

    /* Counters. */
    check_int(c, p->f0302SlotBoxClickCount, 1, "F0302 click count");
    check_int(c, p->f0300RemoveC030Count, 1, "F0300 remove count");
    check_int(c, p->f0301AddC030Count, 1, "F0301 add count");
    check_int(c, p->f0299ObjectModifierApplyCount, 2,
              "F0299 modifier apply count");
    check_int(c, p->f0292DrawStateCount, 1, "F0292 draw state count");
    check_int(c, p->f0297PutLeaderHandCount, 0, "F0297 quiet");
    check_int(c, p->f0298RemoveLeaderHandCount, 0, "F0298 quiet");
    check_int(c, p->f0347PanelRedrawCount, 1, "F0347 redraw count");
    check_int(c, p->f0334CloseCount, 1, "F0334 close count");
    check_int(c, p->f0333OpenCount, 1, "F0333 open count");

    /* Close path result. */
    check_int(c, p->openChestThingAfterClose, 0, "open chest thing after");
    check_int(c, p->closedChainMatchesVisible, 1, "closed chain matches");
    check_int(c, p->closedVisibleItemCount, DM1_PC34_CAOC_SLOT_COUNT,
              "closed visible count");
    check_int(c, p->closedSlot0Type, DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 0,
              "closed slot0 type");
    check_int(c, p->closedSlot0Weight,
              DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 0, "closed slot0 weight");
    check_int(c, p->closedSlot0Charges,
              DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 0,
              "closed slot0 charges");
    check_int(c, p->closedSlot3Type, DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 3,
              "closed slot3 type");
    check_int(c, p->closedSlot3Weight,
              DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 3, "closed slot3 weight");
    check_int(c, p->closedSlot3Charges,
              DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 3,
              "closed slot3 charges");
    check_int(c, p->closedSlot7Type, DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 7,
              "closed slot7 type");
    check_int(c, p->closedSlot7Weight,
              DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 7, "closed slot7 weight");
    check_int(c, p->closedSlot7Charges,
              DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 7,
              "closed slot7 charges");

    /* Panel reroute. */
    check_int(c, p->panelContentAfterClose,
              DM1_PC34_CAOC_PANEL_FOOD_WATER_POISONED,
              "panel content after close");
    check_int(c, p->panelContentReRoutedToFood, 1, "panel rerouted to food");
    check_int(c, p->panelContentDidNotStayAtChest, 1,
              "panel did not stay at chest");
    check_int(c, p->panelContentDidNotStayAtScroll, 1,
              "panel did not stay at scroll");

    /* Other champions untouched. */
    check_int(c, p->otherChampionActionHandBefore,
              p->otherChampionActionHandAfter, "other champion stable");
    check_int(c, p->deadChampionActionHandBefore,
              p->deadChampionActionHandAfter, "dead champion stable");
    check_int(c, p->leaderHandBefore, p->leaderHandAfter, "leader hand stable");

    /* Action hand post-state. */
    check_int(c, p->actionHandTypeAfter, DM1_PC34_CAOC_NON_CONTAINER_ITEM,
              "action hand type after");
    check_int(c, p->actionHandAllowedSlotsAfter, DM1_PC34_ALLOWED_HANDS,
              "action hand mask after");
    check_int(c, p->actionHandWeightAfter,
              DM1_PC34_CAOC_NON_CONTAINER_WEIGHT, "action hand weight after");
    check_int(c, p->actionHandThingTypeAfter, DM1_PC34_CAOC_THING_TYPE_WEAPON,
              "action hand thing type after");

    check_true(c, p->deterministicHash != 0, "deterministic hash set");
}

int main(void)
{
    Counters c;
    DM1_V1_ChestActionHandOwnerChangeProbePc34 p;

    memset(&c, 0, sizeof(c));
    check_source(&c);
    check_probe(&c, &p);

    printf("dm1_v1_chest_action_hand_owner_change_pc34_compat: ");
    if (c.failures != 0) {
        printf("FAILED %d/%d\n", c.failures, c.assertions);
        return 1;
    }
    printf("ok %d/%d hash=0x%08X\n",
           c.assertions,
           c.assertions,
           (unsigned int)p.deterministicHash);
    return 0;
}
