#include <stdio.h>
#include <string.h>
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

    if (CHAMPION_Compat_GetStatusSlotBoxCount() != 8u) { ok = 0; printf("FAIL: slotBoxCount=%u want=8\n", CHAMPION_Compat_GetStatusSlotBoxCount()); }
    if (strstr(CHAMPION_Compat_GetStatusSlotBoxEvidence(), "COMMAND.C:489-496") == NULL) { ok = 0; printf("FAIL: evidence missing COMMAND.C:489-496\n"); }
    if (strstr(CHAMPION_Compat_GetStatusSlotBoxEvidence(), "COMMAND.C:1437-1440") == NULL) { ok = 0; printf("FAIL: evidence missing COMMAND.C:1437-1440\n"); }
    for (i = 0; i < 8; i++) {
        if (!CHAMPION_Compat_GetStatusSlotBox(i, &b)) { ok = 0; continue; }
        printf("statusSlotBox[%u]=champion:%u handSlot:%u command:%u zone:%u box:%u..%u,%u..%u evidence:%s\n",
               i, b.championIndex, b.handSlot, b.commandId, b.zoneIndex,
               b.left, b.right, b.top, b.bottom, b.evidence);
        if (b.championIndex != i / 2u || b.handSlot != i % 2u ||
            b.commandId != 20u + i || b.zoneIndex != 211u + i ||
            b.left != (i / 2u) * 69u + ((i % 2u) ? 24u : 4u) ||
            b.right != b.left + 15u || b.top != 10u || b.bottom != 25u) { ok = 0; printf("FAIL: slotBox[%u] champ=%u hand=%u cmd=%u zone=%u l=%u r=%u t=%u b=%u\n", i, b.championIndex, b.handSlot, b.commandId, b.zoneIndex, b.left, b.right, b.top, b.bottom); }
        if (strstr(b.evidence, "COMMAND.C:489-496") == NULL) ok = 0;
        if (strstr(b.evidence, "CLIKCHAM.C:27-32") == NULL) ok = 0;
    }
    if (CHAMPION_Compat_GetStatusSlotBox(8u, &b)) { ok = 0; printf("FAIL: GetStatusSlotBox(8) should return false\n"); }

    {
        ChampionStatusRectCompat rect;
        if (CHAMPION_Compat_StatusBoxZoneId(0) != 151 ||
            CHAMPION_Compat_StatusBoxZoneId(3) != 154 ||
            CHAMPION_Compat_StatusBoxZoneId(4) != 0) { ok = 0; printf("FAIL: StatusBoxZoneId 0=%u 3=%u 4=%u\n", CHAMPION_Compat_StatusBoxZoneId(0), CHAMPION_Compat_StatusBoxZoneId(3), CHAMPION_Compat_StatusBoxZoneId(4)); }
        if (!CHAMPION_Compat_StatusBoxZone(3, &rect) ||
            rect.x != 207 || rect.y != 0 || rect.w != 67 ||
            rect.h != 29) { ok = 0; printf("FAIL: StatusBoxZone(3) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (CHAMPION_Compat_StatusBarGraphZoneId(0) != 187 ||
            CHAMPION_Compat_StatusBarGraphZoneId(3) != 190) { ok = 0; printf("FAIL: StatusBarGraphZoneId 0=%u 3=%u\n", CHAMPION_Compat_StatusBarGraphZoneId(0), CHAMPION_Compat_StatusBarGraphZoneId(3)); }
        if (!CHAMPION_Compat_StatusBarGraphRegionZone(3, &rect) ||
            rect.x != 250 || rect.y != 0 || rect.w != 24 ||
            rect.h != 29) { ok = 0; printf("FAIL: StatusBarGraphRegionZone(3) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (CHAMPION_Compat_StatusBarZoneId(0) != 195 ||
            CHAMPION_Compat_StatusBarZoneId(1) != 199 ||
            CHAMPION_Compat_StatusBarZoneId(2) != 203 ||
            CHAMPION_Compat_StatusBarZoneId(3) != 0) { ok = 0; printf("FAIL: StatusBarZoneId 0=%u 1=%u 2=%u 3=%u\n", CHAMPION_Compat_StatusBarZoneId(0), CHAMPION_Compat_StatusBarZoneId(1), CHAMPION_Compat_StatusBarZoneId(2), CHAMPION_Compat_StatusBarZoneId(3)); }
        if (!CHAMPION_Compat_StatusBarZone(2, 1, &rect) ||
            rect.x != 191 || rect.y != 2 || rect.w != 4 ||
            rect.h != 25) { ok = 0; printf("FAIL: StatusBarZone(2,1) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (CHAMPION_Compat_StatusHandParentZoneId(3) != 210 ||
            CHAMPION_Compat_StatusHandZoneId(3, 1) != 218) { ok = 0; printf("FAIL: StatusHand parent=%u zone=%u\n", CHAMPION_Compat_StatusHandParentZoneId(3), CHAMPION_Compat_StatusHandZoneId(3,1)); }
        if (!CHAMPION_Compat_StatusHandZone(3, 1, &rect) ||
            rect.x != 231 || rect.y != 10 || rect.w != 16 ||
            rect.h != 16) { ok = 0; printf("FAIL: StatusHandZone(3,1) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (!CHAMPION_Compat_StatusHandIconZone(3, 1, &rect) ||
            rect.x != 232 || rect.y != 11 || rect.w != 16 ||
            rect.h != 16) { ok = 0; printf("FAIL: StatusHandIconZone(3,1) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (!CHAMPION_Compat_StatusHandSlotBoxZone(3, 1, &rect) ||
            rect.x != 231 || rect.y != 10 || rect.w != 18 ||
            rect.h != 18) { ok = 0; printf("FAIL: StatusHandSlotBoxZone(3,1) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (CHAMPION_Compat_StatusNameClearZoneId(3) != 162 ||
            CHAMPION_Compat_StatusNameTextZoneId(3) != 166) { ok = 0; printf("FAIL: NameClearZoneId=%u NameTextZoneId=%u\n", CHAMPION_Compat_StatusNameClearZoneId(3), CHAMPION_Compat_StatusNameTextZoneId(3)); }
        if (!CHAMPION_Compat_StatusNameZone(3, &rect) ||
            rect.x != 207 || rect.y != 0 || rect.w != 43 ||
            rect.h != 7) { ok = 0; printf("FAIL: StatusNameZone(3) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (!CHAMPION_Compat_StatusNameTextZone(3, &rect) ||
            rect.x != 208 || rect.y != 0 || rect.w != 42 ||
            rect.h != 7) { ok = 0; printf("FAIL: StatusNameTextZone(3) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (CHAMPION_Compat_DamageIndicatorZoneId(3) != 170 ||
            CHAMPION_Compat_InventoryDamageIndicatorZoneId(3) != 182) { ok = 0; printf("FAIL: DamageIndicatorZoneId(3)=%u InvDamageIndicatorZoneId(3)=%u\n", CHAMPION_Compat_DamageIndicatorZoneId(3), CHAMPION_Compat_InventoryDamageIndicatorZoneId(3)); }
        if (!CHAMPION_Compat_DamageIndicatorZone(3, 45, 7, &rect) ||
            rect.x != 218 || rect.y != 11 || rect.w != 45 ||
            rect.h != 7) { ok = 0; printf("FAIL: DamageIndicatorZone(3) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (!CHAMPION_Compat_InventoryDamageIndicatorZone(3, 32, 29, &rect) ||
            rect.x != 214 || rect.y != 0 || rect.w != 32 ||
            rect.h != 29) { ok = 0; printf("FAIL: InvDamageIndicatorZone(3) x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h); }
        if (!CHAMPION_Compat_DamageNumberOrigin(3, &rect) ||
            rect.x != 236 || rect.y != 11) { ok = 0; printf("FAIL: DamageNumberOrigin(3) x=%d y=%d\n", rect.x, rect.y); }
        if (!CHAMPION_Compat_DamageNumberOriginPc34(3, 7, 0, &rect) ||
            rect.x != 226 || rect.y != 5) { ok = 0; printf("FAIL: DamageNumberOriginPc34(3,7,0) x=%d y=%d\n", rect.x, rect.y); }
        if (!CHAMPION_Compat_DamageNumberOriginPc34(3, 77, 1, &rect) ||
            rect.x != 225 || rect.y != 16) { ok = 0; printf("FAIL: DamageNumberOriginPc34(3,77,1) x=%d y=%d\n", rect.x, rect.y); }
        {
            int gfx[3] = {0, 0, 0};
            if (CHAMPION_Compat_StatusShieldBorderGraphics(0, 0, 0, gfx) != 0 ||
                gfx[0] != 0 || gfx[1] != 0 || gfx[2] != 0) { ok = 0; printf("FAIL: ShieldBorder(0,0,0) ret=%d gfx=%d,%d,%d\n", CHAMPION_Compat_StatusShieldBorderGraphics(0,0,0,gfx), gfx[0], gfx[1], gfx[2]); }
            if (CHAMPION_Compat_StatusShieldBorderGraphics(7, 5, 3, gfx) != 3 ||
                gfx[0] != CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_SHIELD ||
                gfx[1] != CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_SPELLSHIELD ||
                gfx[2] != CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_FIRESHIELD) { ok = 0; printf("FAIL: ShieldBorder(7,5,3) ret=%d gfx=%d,%d,%d\n", CHAMPION_Compat_StatusShieldBorderGraphics(7,5,3,gfx), gfx[0], gfx[1], gfx[2]); }
            if (CHAMPION_Compat_StatusShieldBorderGraphics(0, 5, 0, gfx) != 1 ||
                gfx[0] != CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_SPELLSHIELD) { ok = 0; printf("FAIL: ShieldBorder(0,5,0) ret=%d gfx[0]=%d\n", CHAMPION_Compat_StatusShieldBorderGraphics(0,5,0,gfx), gfx[0]); }
        }
        if (CHAMPION_Compat_StatusNameColor(0, 10, 0) != -1) { ok = 0; printf("FAIL: StatusNameColor(0,10,0)=%d want=-1\n", CHAMPION_Compat_StatusNameColor(0, 10, 0)); }
        if (CHAMPION_Compat_StatusNameColor(1, 0, 0) !=
            CHAMPION_STATUS_COMPAT_COLOR_LIGHTEST_GRAY) { ok = 0; printf("FAIL: StatusNameColor(1,0,0)=%d want=%d\n", CHAMPION_Compat_StatusNameColor(1,0,0), CHAMPION_STATUS_COMPAT_COLOR_LIGHTEST_GRAY); }
        if (CHAMPION_Compat_StatusNameColor(1, 10, 1) !=
            CHAMPION_STATUS_COMPAT_COLOR_YELLOW) { ok = 0; printf("FAIL: StatusNameColor(1,10,1)=%d want=%d\n", CHAMPION_Compat_StatusNameColor(1,10,1), CHAMPION_STATUS_COMPAT_COLOR_YELLOW); }
        if (CHAMPION_Compat_StatusNameColor(1, 10, 0) !=
            CHAMPION_STATUS_COMPAT_COLOR_GOLD) { ok = 0; printf("FAIL: StatusNameColor(1,10,0)=%d want=%d\n", CHAMPION_Compat_StatusNameColor(1,10,0), CHAMPION_STATUS_COMPAT_COLOR_GOLD); }
        if (CHAMPION_Compat_StatusNameClearColor() !=
            CHAMPION_STATUS_COMPAT_COLOR_DARK_GRAY_CLEAR) { ok = 0; printf("FAIL: StatusNameClearColor=%d\n", CHAMPION_Compat_StatusNameClearColor()); }
        if (CHAMPION_Compat_StatusBoxFillColor() !=
            CHAMPION_STATUS_COMPAT_COLOR_DARKEST_GRAY) { ok = 0; printf("FAIL: StatusBoxFillColor=%d\n", CHAMPION_Compat_StatusBoxFillColor()); }
        if (CHAMPION_Compat_StatusBoxGraphicId() !=
            CHAMPION_STATUS_COMPAT_GFX_STATUS_BOX) { ok = 0; printf("FAIL: StatusBoxGraphicId=%d\n", CHAMPION_Compat_StatusBoxGraphicId()); }
        if (CHAMPION_Compat_DeadStatusBoxGraphicId() !=
            CHAMPION_STATUS_COMPAT_GFX_STATUS_BOX_DEAD) { ok = 0; printf("FAIL: DeadStatusBoxGraphicId=%d\n", CHAMPION_Compat_DeadStatusBoxGraphicId()); }
        if (CHAMPION_Compat_StatusBoxBaseGraphic(0, 10) != 0) { ok = 0; printf("FAIL: StatusBoxBaseGraphic(0,10)=%d\n", CHAMPION_Compat_StatusBoxBaseGraphic(0,10)); }
        if (CHAMPION_Compat_StatusBoxBaseGraphic(1, 10) != 0) { ok = 0; printf("FAIL: StatusBoxBaseGraphic(1,10)=%d\n", CHAMPION_Compat_StatusBoxBaseGraphic(1,10)); }
        if (CHAMPION_Compat_StatusBoxBaseGraphic(1, 0) !=
            CHAMPION_STATUS_COMPAT_GFX_STATUS_BOX_DEAD) { ok = 0; printf("FAIL: StatusBoxBaseGraphic(1,0)=%d\n", CHAMPION_Compat_StatusBoxBaseGraphic(1,0)); }
    }

    if (CHAMPION_Compat_GetStatusNameBoxCount() != 4u) { ok = 0; printf("FAIL: nameBoxCount=%u\n", CHAMPION_Compat_GetStatusNameBoxCount()); }
    for (i = 0; i < 4; i++) {
        if (!CHAMPION_Compat_GetStatusNameBox(i, &n)) { ok = 0; continue; }
        printf("statusNameBox[%u]=command:%u box:%u..%u,%u..%u fill:%ux%u evidence:%s\n",
               i, n.commandId, n.left, n.right, n.top, n.bottom,
               n.fillWidth, n.fillHeight, n.evidence);
        if (n.commandId != 16u + i || n.left != 69u * i ||
            n.right != 69u * i + 42u || n.top != 0u || n.bottom != 6u ||
            n.fillWidth != 43u || n.fillHeight != 7u) { ok = 0; printf("FAIL: nameBox[%u] cmd=%u l=%u r=%u t=%u b=%u fw=%u fh=%u\n", i, n.commandId, n.left, n.right, n.top, n.bottom, n.fillWidth, n.fillHeight); }
    }


    if (!CHAMPION_Compat_GetStatusRedrawPlan(0u, ATTR_STATUS_BOX, 0u, 1u, &r)) { ok = 0; printf("FAIL: GetStatusRedrawPlan(0) returned false\n"); }
    printf("statusRedrawPlan[liveNonInventory]=status:%u name:%u stats:%u wounds:%u action:%u viewport:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics,
           r.redrawWounds, r.redrawActionHand, r.redrawViewport,
           r.clearAllDirtyFlagsAtEnd, r.evidence);
    if (!r.statusBoxRedrawn || !r.redrawNameTitle || !r.redrawStatistics ||
        !r.redrawWounds || !r.redrawActionHand || r.redrawViewport ||
        !r.clearAllDirtyFlagsAtEnd) { ok = 0; printf("FAIL: redrawPlan[liveNonInventory] s=%u n=%u st=%u w=%u a=%u v=%u c=%u\n", r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics, r.redrawWounds, r.redrawActionHand, r.redrawViewport, r.clearAllDirtyFlagsAtEnd); }

    if (!CHAMPION_Compat_GetStatusRedrawPlan(1u, ATTR_STATUS_BOX, 1u, 1u, &r)) { ok = 0; printf("FAIL: GetStatusRedrawPlan(1) returned false\n"); }
    printf("statusRedrawPlan[liveInventory]=status:%u name:%u stats:%u wounds:%u action:%u viewport:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics,
           r.redrawWounds, r.redrawActionHand, r.redrawViewport,
           r.clearAllDirtyFlagsAtEnd, r.evidence);
    if (!r.statusBoxRedrawn || r.redrawNameTitle || !r.redrawStatistics ||
        r.redrawWounds || r.redrawActionHand || r.redrawViewport ||
        !r.clearAllDirtyFlagsAtEnd) { ok = 0; printf("FAIL: redrawPlan[liveInventory] s=%u n=%u st=%u w=%u a=%u v=%u c=%u\n", r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics, r.redrawWounds, r.redrawActionHand, r.redrawViewport, r.clearAllDirtyFlagsAtEnd); }

    if (!CHAMPION_Compat_GetStatusRedrawPlan(2u, ATTR_STATUS_BOX | ATTR_NAME_TITLE | ATTR_STATISTICS,
                                            0u, 0u, &r)) { ok = 0; printf("FAIL: GetStatusRedrawPlan(2) returned false\n"); }
    printf("statusRedrawPlan[dead]=status:%u deadBox:%u actionBeforeClear:%u name:%u stats:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.drawDeadStatusBox, r.drawActionIconBeforeClear,
           r.redrawNameTitle, r.redrawStatistics, r.clearAllDirtyFlagsAtEnd,
           r.evidence);
    if (!r.statusBoxRedrawn || !r.drawDeadStatusBox || !r.drawActionIconBeforeClear ||
        r.redrawNameTitle || r.redrawStatistics || !r.clearAllDirtyFlagsAtEnd) { ok = 0; printf("FAIL: redrawPlan[dead] s=%u d=%u a=%u n=%u st=%u c=%u\n", r.statusBoxRedrawn, r.drawDeadStatusBox, r.drawActionIconBeforeClear, r.redrawNameTitle, r.redrawStatistics, r.clearAllDirtyFlagsAtEnd); }

    if (!CHAMPION_Compat_GetStatusRedrawPlan(3u, ATTR_NAME_TITLE | ATTR_STATISTICS, 1u, 1u, &r)) { ok = 0; printf("FAIL: GetStatusRedrawPlan(3) returned false\n"); }
    printf("statusRedrawPlan[liveInventoryNameStatsOnly]=status:%u name:%u stats:%u viewport:%u clear:%u evidence:%s\n",
           r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics,
           r.redrawViewport, r.clearAllDirtyFlagsAtEnd, r.evidence);
    if (r.statusBoxRedrawn || !r.redrawNameTitle || !r.redrawStatistics ||
        !r.redrawViewport || !r.clearAllDirtyFlagsAtEnd) { ok = 0; printf("FAIL: redrawPlan[liveInventoryNameStatsOnly] s=%u n=%u st=%u v=%u c=%u\n", r.statusBoxRedrawn, r.redrawNameTitle, r.redrawStatistics, r.redrawViewport, r.clearAllDirtyFlagsAtEnd); }

    printf("championStatusSlotBoxInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
