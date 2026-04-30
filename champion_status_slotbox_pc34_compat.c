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
    outBox->evidence = "DATA.C:977-985 maps status slot boxes 0..7 to C211..C218; COMMAND.C:489-496 maps C020..C027; CHAMPION.C:677-683 routes slotBox<8 to championIndex=slotBox>>1 and hand slot M070";
    return 1;
}

const char* CHAMPION_Compat_GetStatusSlotBoxEvidence(void) {
    return "ReDMCSB source lock: PC/F20+ status slot boxes 0..7 are champion top-row ready/action hands only; slotBox>>1 selects champion 0..3 and slotBox&1 selects ready/action hand.";
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
