#include <stdio.h>
#include "champion_status_slotbox_pc34_compat.h"

int main(void) {
    unsigned int i;
    int ok = 1;
    ChampionStatusSlotBoxCompat b;
    ChampionStatusNameBoxCompat n;
    ChampionStatusRedrawPlanCompat r;
    enum {
        ATTR_NAME_TITLE = 0x0080u,
        ATTR_STATISTICS = 0x0100u,
        ATTR_STATUS_BOX = 0x1000u,
        ATTR_WOUNDS = 0x2000u,
        ATTR_VIEWPORT = 0x4000u,
        ATTR_ACTION_HAND = 0x8000u
    };

    printf("probe=firestaff_champion_status_slotbox_source\n");
    printf("statusSlotBoxEvidence=%s\n", CHAMPION_Compat_GetStatusSlotBoxEvidence());
    printf("statusNameBoxEvidence=%s\n", CHAMPION_Compat_GetStatusNameBoxEvidence());
    printf("statusRedrawPlanEvidence=%s\n", CHAMPION_Compat_GetStatusRedrawPlanEvidence());

    if (CHAMPION_Compat_GetStatusSlotBoxCount() != 8u) ok = 0;
    for (i = 0; i < 8; i++) {
        if (!CHAMPION_Compat_GetStatusSlotBox(i, &b)) { ok = 0; continue; }
        printf("statusSlotBox[%u]=champion:%u handSlot:%u command:%u zone:%u box:%u..%u,%u..%u evidence:%s\n",
               i, b.championIndex, b.handSlot, b.commandId, b.zoneIndex,
               b.left, b.right, b.top, b.bottom, b.evidence);
        if (b.championIndex != i / 2u || b.handSlot != i % 2u ||
            b.commandId != 20u + i || b.zoneIndex != 211u + i ||
            b.left != (i / 2u) * 69u + ((i % 2u) ? 24u : 4u) ||
            b.right != b.left + 15u || b.top != 10u || b.bottom != 25u) ok = 0;
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


    if (!CHAMPION_Compat_GetStatusRedrawPlan(0u, ATTR_STATUS_BOX, 0u, 1u, &r)) ok = 0;
    printf("statusRedrawPlan[liveNonInventory]=status:%u name:%u stats:%u wounds:%u action:%u viewport:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics,
           r.redrawWounds, r.redrawActionHand, r.redrawViewport,
           r.clearAllDirtyFlagsAtEnd, r.evidence);
    if (!r.statusBoxRedrawn || !r.redrawNameTitle || !r.redrawStatistics ||
        !r.redrawWounds || !r.redrawActionHand || r.redrawViewport ||
        !r.clearAllDirtyFlagsAtEnd) ok = 0;

    if (!CHAMPION_Compat_GetStatusRedrawPlan(1u, ATTR_STATUS_BOX, 1u, 1u, &r)) ok = 0;
    printf("statusRedrawPlan[liveInventory]=status:%u name:%u stats:%u wounds:%u action:%u viewport:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics,
           r.redrawWounds, r.redrawActionHand, r.redrawViewport,
           r.clearAllDirtyFlagsAtEnd, r.evidence);
    if (!r.statusBoxRedrawn || r.redrawNameTitle || !r.redrawStatistics ||
        r.redrawWounds || r.redrawActionHand || r.redrawViewport ||
        !r.clearAllDirtyFlagsAtEnd) ok = 0;

    if (!CHAMPION_Compat_GetStatusRedrawPlan(2u, ATTR_STATUS_BOX | ATTR_NAME_TITLE | ATTR_STATISTICS,
                                            0u, 0u, &r)) ok = 0;
    printf("statusRedrawPlan[dead]=status:%u deadBox:%u actionBeforeClear:%u name:%u stats:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.drawDeadStatusBox, r.drawActionIconBeforeClear,
           r.redrawNameTitle, r.redrawStatistics, r.clearAllDirtyFlagsAtEnd,
           r.evidence);
    if (!r.statusBoxRedrawn || !r.drawDeadStatusBox || !r.drawActionIconBeforeClear ||
        r.redrawNameTitle || r.redrawStatistics || !r.clearAllDirtyFlagsAtEnd) ok = 0;

    if (!CHAMPION_Compat_GetStatusRedrawPlan(3u, ATTR_NAME_TITLE | ATTR_STATISTICS, 1u, 1u, &r)) ok = 0;
    printf("statusRedrawPlan[liveInventoryNameStatsOnly]=status:%u name:%u stats:%u viewport:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics,
           r.redrawViewport, r.clearAllDirtyFlagsAtEnd, r.evidence);
    if (r.statusBoxRedrawn || !r.redrawNameTitle || !r.redrawStatistics ||
        !r.redrawViewport || !r.clearAllDirtyFlagsAtEnd) ok = 0;

    printf("championStatusSlotBoxInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
