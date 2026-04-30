#include "touch_click_zone_matrix_pc34_compat.h"
#include <string.h>

/* Source-locked starter matrix for the touchscreen/click abstraction.
 * Coordinates are the PC34-compatible 320x200 screen boxes already used by
 * the V1 path.  Inventory slot boxes retain the original CM2 viewport-relative
 * command mode, matching COMMAND.C before any caller maps them into a screen. */
static const TouchClickZonePc34Compat kTouchClickZones[] = {
    { 80u,   7u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,      0,  31, 224, 136, "viewport",  "COMMAND.C:403 maps C080 to C007_ZONE_VIEWPORT; COORD.C:1667-1683 anchors viewport at x=0 y=31 size 224x136" },
    {  1u,  68u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,    234, 125,  28,  21, "movement",  "COMMAND.C:396-405 maps movement commands to C068..C073; COORD.C:237-263 defines movement-arrow child zones" },
    {  3u,  70u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,    263, 125,  27,  21, "movement",  "COMMAND.C:397-398 maps turn-left and move-forward; CLIKMENU.C:155-166 and 255-263 highlight those same zones" },
    {111u,  11u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,    233,  77,  87,  45, "action",    "COMMAND.C:392-393 maps C111 to C011_ZONE_ACTION_AREA; DATA.C:119-121 gives action area box 224..319,77..121 for pre-layout builds; COORD.C:270-327 defines C075..C098 action subzones" },
    {112u,  98u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,    285,  77,  35,   7, "action",    "COMMAND.C:461-466 maps C112 pass; CLIKMENU.C:530-538 highlights C098_ZONE_ACTION_AREA_PASS and gives legacy box 285..319,77..83" },
    {113u,  82u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,    234,  86,  85,  11, "action",    "COMMAND.C:461-466 maps C113 action 0; CLIKMENU.C:541-548 highlights C082_ZONE_ACTION_AREA_ACTION_0 and gives legacy box 234..318,86..96" },
    {100u,  13u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,    233,  42,  87,  33, "spell",     "COMMAND.C:392 and 473-483 map spell clicks; COORD.C:333-381 defines C221..C264 spell-area subzones; m11 keeps 33px click height for C013" },
    {101u, 245u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,    235,  51,  13,  11, "spell",     "COMMAND.C:475 maps C101 to C245_ZONE_SPELL_AREA_SYMBOL_1; COORD.C:361-372 defines C245..C250 from C244 + 2,9 increments" },
    {  7u, 151u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,      0,   0,  67,  29, "champion",  "COMMAND.C:375-395 maps champion HUD interactions to C151..C154 and C187..C190; COORD.C:386-459 defines champion status boxes" },
    { 28u, 507u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,    6,  53,  16,  16, "inventory", "COMMAND.C:419 maps C028 to C507_ZONE_SLOT_BOX_08_INVENTORY_READY_HAND; DATA.C:927-975 and COORD.C:493-522 give inventory slot coordinates" }
};

unsigned int TOUCHCLICK_Compat_GetZoneCount(void) {
    return (unsigned int)(sizeof(kTouchClickZones) / sizeof(kTouchClickZones[0]));
}

int TOUCHCLICK_Compat_GetZone(unsigned int ordinal, TouchClickZonePc34Compat* outZone) {
    if (!outZone || ordinal >= TOUCHCLICK_Compat_GetZoneCount()) return 0;
    *outZone = kTouchClickZones[ordinal];
    return 1;
}

int TOUCHCLICK_Compat_HitTest(int screenX, int screenY, TouchClickZonePc34Compat* outZone) {
    unsigned int i;
    const TouchClickZonePc34Compat* best = 0;
    int bestArea = 0x7fffffff;
    for (i = 0; i < TOUCHCLICK_Compat_GetZoneCount(); ++i) {
        const TouchClickZonePc34Compat* z = &kTouchClickZones[i];
        int area;
        if (z->coordMode != TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT) continue;
        if (screenX < z->x || screenY < z->y ||
            screenX >= z->x + z->w || screenY >= z->y + z->h) continue;
        area = z->w * z->h;
        if (area < bestArea) {
            best = z;
            bestArea = area;
        }
    }
    if (best) {
        if (outZone) *outZone = *best;
        return 1;
    }
    if (outZone) memset(outZone, 0, sizeof(*outZone));
    return 0;
}

const char* TOUCHCLICK_Compat_GetSourceEvidence(void) {
    return "COMMAND.C:375-483 is the command-to-zone click table; CLIKMENU.C:72 and 155-263/386-575 prove zone-based highlight/dispatch; COORD.C:7-24 and 2036-2238 define and resolve layout records; DATA.C:927-1023 binds inventory slot boxes to C507..C536.";
}
