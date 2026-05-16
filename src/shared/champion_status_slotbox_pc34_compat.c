#include "champion_status_slotbox_pc34_compat.h"
#include <string.h>

unsigned int CHAMPION_Compat_GetStatusSlotBoxCount(void) { return 8u; }

int CHAMPION_Compat_GetStatusSlotBox(unsigned int slotBoxIndex, ChampionStatusSlotBoxCompat* outBox) {
    if (!outBox || slotBoxIndex > 7u) return 0;
    memset(outBox, 0, sizeof(*outBox));
    outBox->slotBoxIndex = slotBoxIndex;
    outBox->championIndex = slotBoxIndex >> 1;
    outBox->handSlot = slotBoxIndex & 1u;
    outBox->commandId = 20u + slotBoxIndex;
    outBox->zoneIndex = 211u + slotBoxIndex;
    outBox->left = outBox->championIndex * 69u + (outBox->handSlot ? 24u : 4u);
    outBox->right = outBox->left + 15u;
    outBox->top = 10u;
    outBox->bottom = 25u;
    outBox->evidence = "COMMAND.C:208-215 maps status hand hit boxes to x=4/24 + 69*champion, y=10..25; DATA.C:977-985 maps status slot boxes 0..7 to C211..C218; CHAMPION.C:677-683 routes slotBox<8 to championIndex=slotBox>>1 and hand slot M070";
    return 1;
}

const char* CHAMPION_Compat_GetStatusSlotBoxEvidence(void) {
    return "ReDMCSB source lock: PC/F20+ status slot boxes 0..7 are champion top-row ready/action hands only; COMMAND.C fixes their inclusive 16x16 hit boxes at x=4/24 + 69*champion, y=10..25; slotBox>>1 selects champion 0..3 and slotBox&1 selects ready/action hand.";
}

unsigned int CHAMPION_Compat_GetStatusNameBoxCount(void) { return 4u; }

int CHAMPION_Compat_GetStatusNameBox(unsigned int championIndex, ChampionStatusNameBoxCompat* outBox) {
    const unsigned int stride = 69u;
    if (!outBox || championIndex > 3u) return 0;
    memset(outBox, 0, sizeof(*outBox));
    outBox->championIndex = championIndex;
    outBox->commandId = 16u + championIndex;
    outBox->left = championIndex * stride;
    outBox->right = outBox->left + 42u;
    outBox->top = 0u;
    outBox->bottom = 6u;
    outBox->fillWidth = 43u;
    outBox->fillHeight = 7u;
    outBox->evidence = "COMMAND.C:202-216 fixes C016..C019 leader-name hit boxes at x=0..42,69..111,138..180,207..249 y=0..6; CHAMDRAW.C:750 uses C69 stride; CHAMDRAW.C:879-884 fills non-inventory champion name area left=x right=x+42 top=0 bottom=6 and prints name at x+1,y=5";
    return 1;
}

const char* CHAMPION_Compat_GetStatusNameBoxEvidence(void) {
    return "ReDMCSB source lock: champion top-row name/leader lane uses four 43x7 inclusive boxes, spaced every 69 px, with commands C016..C019.";
}


#define CHAMPION_COMPAT_ATTR_NAME_TITLE  0x0080u
#define CHAMPION_COMPAT_ATTR_STATISTICS  0x0100u
#define CHAMPION_COMPAT_ATTR_LOAD        0x0200u
#define CHAMPION_COMPAT_ATTR_ICON        0x0400u
#define CHAMPION_COMPAT_ATTR_PANEL       0x0800u
#define CHAMPION_COMPAT_ATTR_STATUS_BOX  0x1000u
#define CHAMPION_COMPAT_ATTR_WOUNDS      0x2000u
#define CHAMPION_COMPAT_ATTR_VIEWPORT    0x4000u
#define CHAMPION_COMPAT_ATTR_ACTION_HAND 0x8000u

#define CHAMPION_COMPAT_ATTR_ALL_DIRTY \
    (CHAMPION_COMPAT_ATTR_NAME_TITLE | CHAMPION_COMPAT_ATTR_STATISTICS | \
     CHAMPION_COMPAT_ATTR_LOAD | CHAMPION_COMPAT_ATTR_ICON | \
     CHAMPION_COMPAT_ATTR_PANEL | CHAMPION_COMPAT_ATTR_STATUS_BOX | \
     CHAMPION_COMPAT_ATTR_WOUNDS | CHAMPION_COMPAT_ATTR_VIEWPORT | \
     CHAMPION_COMPAT_ATTR_ACTION_HAND)

int CHAMPION_Compat_GetStatusRedrawPlan(unsigned int championIndex,
                                        unsigned int attributes,
                                        unsigned int isInventoryChampion,
                                        unsigned int currentHealth,
                                        ChampionStatusRedrawPlanCompat* outPlan) {
    if (!outPlan || championIndex > 3u) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = championIndex;
    outPlan->sourceAttributes = attributes;
    outPlan->isInventoryChampion = isInventoryChampion ? 1u : 0u;
    outPlan->currentHealth = currentHealth ? 1u : 0u;

    if (!(attributes & CHAMPION_COMPAT_ATTR_ALL_DIRTY)) {
        outPlan->evidence = "CHAMDRAW.C:755-758 returns immediately when no dirty champion attributes are set";
        return 1;
    }

    if ((attributes & CHAMPION_COMPAT_ATTR_STATUS_BOX) != 0u) {
        outPlan->statusBoxRedrawn = 1u;
        if (currentHealth) {
            outPlan->redrawStatistics = 1u;
            if (isInventoryChampion) {
                outPlan->evidence = "CHAMDRAW.C:771-815 redraws a live status box; inventory champion sets MASK0x0100_STATISTICS only after portrait redraw";
            } else {
                outPlan->redrawNameTitle = 1u;
                outPlan->redrawWounds = 1u;
                outPlan->redrawActionHand = 1u;
                outPlan->evidence = "CHAMDRAW.C:771-815 redraws a live status box; non-inventory champions set NAME_TITLE|STATISTICS|WOUNDS|ACTION_HAND";
            }
        } else {
            outPlan->drawDeadStatusBox = 1u;
            outPlan->drawActionIconBeforeClear = 1u;
            outPlan->clearAllDirtyFlagsAtEnd = 1u;
            outPlan->evidence = "CHAMDRAW.C:816-842 draws the dead status box, draws the action icon, then jumps to final dirty-flag clear";
            return 1;
        }
    }

    if (!currentHealth) {
        outPlan->clearAllDirtyFlagsAtEnd = 1u;
        outPlan->evidence = "CHAMDRAW.C:841-842 skips the remaining live-only redraw lanes for dead champions before final dirty-flag clear";
        return 1;
    }

    if ((attributes & CHAMPION_COMPAT_ATTR_NAME_TITLE) != 0u) {
        outPlan->redrawNameTitle = 1u;
        if (isInventoryChampion) outPlan->redrawViewport = 1u;
    }
    if ((attributes & CHAMPION_COMPAT_ATTR_STATISTICS) != 0u || outPlan->redrawStatistics) {
        outPlan->redrawStatistics = 1u;
    }
    outPlan->clearAllDirtyFlagsAtEnd = 1u;
    if (!outPlan->evidence) {
        outPlan->evidence = "CHAMDRAW.C:843-907 processes live NAME_TITLE before STATISTICS/bar-graphs; CHAMDRAW.C:1110 clears all dirty flags at function end";
    }
    return 1;
}

const char* CHAMPION_Compat_GetStatusRedrawPlanEvidence(void) {
    return "ReDMCSB source lock: F0292_CHAMPION_DrawState expands STATUS_BOX into dependent live redraw lanes before the STATISTICS/bar-graph pass, while dead champions draw dead box/action icon then clear dirty flags.";
}
