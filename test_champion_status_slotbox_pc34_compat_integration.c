#include <stdio.h>
#include "champion_status_slotbox_pc34_compat.h"

int main(void) {
    unsigned int i;
    int ok = 1;
    ChampionStatusSlotBoxCompat b;
    ChampionStatusNameBoxCompat n;

    printf("probe=firestaff_champion_status_slotbox_source\n");
    printf("statusSlotBoxEvidence=%s\n", CHAMPION_Compat_GetStatusSlotBoxEvidence());
    printf("statusNameBoxEvidence=%s\n", CHAMPION_Compat_GetStatusNameBoxEvidence());

    if (CHAMPION_Compat_GetStatusSlotBoxCount() != 8u) ok = 0;
    for (i = 0; i < 8; i++) {
        if (!CHAMPION_Compat_GetStatusSlotBox(i, &b)) { ok = 0; continue; }
        printf("statusSlotBox[%u]=champion:%u handSlot:%u command:%u zone:%u evidence:%s\n",
               i, b.championIndex, b.handSlot, b.commandId, b.zoneIndex, b.evidence);
        if (b.championIndex != i / 2u || b.handSlot != i % 2u ||
            b.commandId != 20u + i || b.zoneIndex != 211u + i) ok = 0;
    }

    if (CHAMPION_Compat_GetStatusNameBoxCount() != 4u) ok = 0;
    for (i = 0; i < 4; i++) {
        if (!CHAMPION_Compat_GetStatusNameBox(i, &n)) { ok = 0; continue; }
        printf("statusNameBox[%u]=command:%u box:%u..%u,%u..%u fill:%ux%u evidence:%s\n",
               i, n.commandId, n.left, n.right, n.top, n.bottom,
               n.fillWidth, n.fillHeight, n.evidence);
        if (n.commandId != 16u + i || n.left != 69u * i ||
            n.right != 69u * i + 42u || n.top != 0u || n.bottom != 6u ||
            n.fillWidth != 43u || n.fillHeight != 7u) ok = 0;
    }

    printf("championStatusSlotBoxInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
