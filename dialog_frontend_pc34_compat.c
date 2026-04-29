#include "dialog_frontend_pc34_compat.h"
#include <string.h>

typedef struct DialogCompatEventSpec {
    DialogCompatSourceEventKind kind;
    unsigned int choiceCount;
    unsigned int zoneIndex;
    unsigned int patchZoneIndex;
    unsigned int negativeGraphicIndex;
    unsigned int delayTicks;
    unsigned int vblankCount;
    const char* evidence;
} DialogCompatEventSpec;

static const DialogCompatEventSpec kDialogEvents[] = {
    { DIALOG_COMPAT_SOURCE_EVENT_EXPAND_DIALOG_GRAPHIC, 0, 0, 0, 0, 0, 0, "DIALOG.C:680-688 expands G0343 dialog graphic into viewport and locks viewport pixel dimensions" },
    { DIALOG_COMPAT_SOURCE_EVENT_PRINT_VERSION, 0, 450, 0, 0, 0, 0, "DIALOG.C:714-736 prints engine version at C450_ZONE_DIALOG_VERSION" },
    { DIALOG_COMPAT_SOURCE_EVENT_COUNT_CHOICES, 0, 0, 0, 0, 0, 0, "DIALOG.C:738-758 derives choice count from non-NULL Choice2/Choice3/Choice4" },
    { DIALOG_COMPAT_SOURCE_EVENT_FADE_OR_CLEAR, 0, 0, 0, 0, 0, 0, "DIALOG.C:873-891 applies curtain/fade and M518_FillScreenBlack on clear" },
    { DIALOG_COMPAT_SOURCE_EVENT_PATCH_CHOICE_GRAPHIC, 1, 469, 451, 621, 0, 0, "DIALOG.C:906-917 one-choice path: zone C469, M621 patch to C451, bottom choice C462" },
    { DIALOG_COMPAT_SOURCE_EVENT_PATCH_CHOICE_GRAPHIC, 2, 471, 452, 622, 0, 0, "DIALOG.C:918-930 two-choice path: zone C471, M622 patch to C452, top C463 and bottom C462" },
    { DIALOG_COMPAT_SOURCE_EVENT_PATCH_CHOICE_GRAPHIC, 3, 471, 0, 0, 0, 0, "DIALOG.C:931-943 three-choice path uses unpatched C471 dialog area and zones C463/C466/C467" },
    { DIALOG_COMPAT_SOURCE_EVENT_PATCH_CHOICE_GRAPHIC, 4, 471, 453, 623, 0, 0, "DIALOG.C:944-959 four-choice path: zone C471, M623 patch to C453, zones C464-C467" },
    { DIALOG_COMPAT_SOURCE_EVENT_LAYOUT_MESSAGES, 0, 469, 0, 0, 0, 0, "DIALOG.C:974-1060 measures message widths, chooses one/two lines, vertically centers in C469/C471" },
    { DIALOG_COMPAT_SOURCE_EVENT_BLIT_OR_DRAW_VIEWPORT, 0, 0, 0, 0, 0, 1, "DIALOG.C:853-864 screen dialogs blit viewport to screen box; non-screen draws viewport then waits one VBlank" },
    { DIALOG_COMPAT_SOURCE_EVENT_FADE_IN, 0, 0, 0, 0, 0, 0, "DIALOG.C:1063-1076 restores curtain/fade after draw" },
    { DIALOG_COMPAT_SOURCE_EVENT_GET_CHOICE_LOOP, 0, 0, 0, 0, 0, 1, "DIALOG.C:331-388 swaps dialog inputs, discards input, show pointer, process queue + one VBlank loop until choice != 99" },
    { DIALOG_COMPAT_SOURCE_EVENT_CHOICE_FEEDBACK, 0, 0, 0, 0, 5, 2, "DIALOG.C:409-526 expands selected zone by 2/3 px, draws pressed/unpressed feedback, two F0600 refreshes, F0022_MAIN_Delay(5)" },
    { DIALOG_COMPAT_SOURCE_EVENT_RESTORE_INPUT, 0, 0, 0, 0, 0, 0, "DIALOG.C:534-542 restores previous input tables, discards input, returns selected choice" }
};

unsigned int DIALOG_Compat_GetSourceEventCount(void) {
    return (unsigned int)(sizeof(kDialogEvents) / sizeof(kDialogEvents[0]));
}

int DIALOG_Compat_GetSourceEvent(unsigned int ordinal, DialogCompatSourceEvent* outEvent) {
    DialogCompatSourceEvent event;
    const DialogCompatEventSpec* spec;
    if (!outEvent || ordinal == 0u || ordinal > DIALOG_Compat_GetSourceEventCount()) return 0;
    spec = &kDialogEvents[ordinal - 1u];
    memset(&event, 0, sizeof(event));
    event.ordinal = ordinal;
    event.kind = spec->kind;
    event.choiceCount = spec->choiceCount;
    event.zoneIndex = spec->zoneIndex;
    event.patchZoneIndex = spec->patchZoneIndex;
    event.negativeGraphicIndex = spec->negativeGraphicIndex;
    event.delayTicks = spec->delayTicks;
    event.vblankCount = spec->vblankCount;
    event.sourceLineEvidence = spec->evidence;
    *outEvent = event;
    return 1;
}

unsigned int DIALOG_Compat_GetChoiceLayoutCount(unsigned int choiceCount) {
    return (choiceCount >= 1u && choiceCount <= 4u) ? choiceCount : 0u;
}

int DIALOG_Compat_GetChoiceLayout(unsigned int choiceCount, unsigned int slotOrdinal, DialogCompatChoiceLayout* outLayout) {
    DialogCompatChoiceLayout layout;
    if (!outLayout || slotOrdinal == 0u || slotOrdinal > DIALOG_Compat_GetChoiceLayoutCount(choiceCount)) return 0;
    memset(&layout, 0, sizeof(layout));
    layout.choiceCount = choiceCount;
    layout.slotOrdinal = slotOrdinal;
    if (choiceCount == 1u) {
        layout.textZoneIndex = 462u; layout.centerX = 112u; layout.centerY = 114u;
        layout.sourceLineEvidence = "DIALOG.C:785 / DIALOG.C:912-915 one choice centered bottom at C462";
    } else if (choiceCount == 2u) {
        if (slotOrdinal == 1u) { layout.textZoneIndex = 463u; layout.centerX = 112u; layout.centerY = 77u; }
        else { layout.textZoneIndex = 462u; layout.centerX = 112u; layout.centerY = 114u; }
        layout.sourceLineEvidence = "DIALOG.C:794-795 / DIALOG.C:923-928 two choices top C463 and bottom C462";
    } else if (choiceCount == 3u) {
        if (slotOrdinal == 1u) { layout.textZoneIndex = 463u; layout.centerX = 112u; layout.centerY = 77u; }
        else if (slotOrdinal == 2u) { layout.textZoneIndex = 466u; layout.centerX = 59u; layout.centerY = 114u; }
        else { layout.textZoneIndex = 467u; layout.centerX = 166u; layout.centerY = 114u; }
        layout.sourceLineEvidence = "DIALOG.C:797-800 / DIALOG.C:931-941 three choices top C463, bottom-left C466, bottom-right C467";
    } else {
        if (slotOrdinal == 1u) { layout.textZoneIndex = 464u; layout.centerX = 59u; layout.centerY = 77u; }
        else if (slotOrdinal == 2u) { layout.textZoneIndex = 465u; layout.centerX = 166u; layout.centerY = 77u; }
        else if (slotOrdinal == 3u) { layout.textZoneIndex = 466u; layout.centerX = 59u; layout.centerY = 114u; }
        else { layout.textZoneIndex = 467u; layout.centerX = 166u; layout.centerY = 114u; }
        layout.sourceLineEvidence = "DIALOG.C:802-812 / DIALOG.C:944-958 four choices top-left C464, top-right C465, bottom-left C466, bottom-right C467";
    }
    *outLayout = layout;
    return 1;
}

int DIALOG_Compat_GetLayoutRecord(unsigned int zoneIndex, DialogCompatLayoutRecord* outRecord) {
    static const DialogCompatLayoutRecord records[] = {
        { 450u, 4u, 4u, 192u, 7u, "COORD.C:740-765 G3031 LayoutData16 record for C450_ZONE_DIALOG_VERSION" },
        { 451u, 1u, 4u, 0u, 51u, "COORD.C:740-765 G3031 LayoutData16 record for C451_ZONE_DIALOG_PATCH_1_CHOICE" },
        { 452u, 1u, 4u, 102u, 89u, "COORD.C:740-765 G3031 LayoutData16 record for C452_ZONE_DIALOG_PATCH_2_CHOICES" },
        { 453u, 1u, 4u, 102u, 62u, "COORD.C:740-765 G3031 LayoutData16 record for C453_ZONE_DIALOG_PATCH_4_CHOICES" },
        { 462u, 10u, 456u, 0u, 0u, "COORD.C:740-765 G3031 LayoutData16 record for C462_ZONE_DIALOG_BOTTOM_CHOICE" },
        { 463u, 10u, 457u, 0u, 0u, "COORD.C:740-765 G3031 LayoutData16 record for C463_ZONE_DIALOG_TOP_CHOICE" },
        { 464u, 10u, 458u, 0u, 0u, "COORD.C:740-765 G3031 LayoutData16 record for C464_ZONE_DIALOG_TOP_LEFT_CHOICE" },
        { 465u, 10u, 459u, 0u, 0u, "COORD.C:740-765 G3031 LayoutData16 record for C465_ZONE_DIALOG_TOP_RIGHT_CHOICE" },
        { 466u, 10u, 460u, 0u, 0u, "COORD.C:740-765 G3031 LayoutData16 record for C466_ZONE_DIALOG_BOTTOM_LEFT_CHOICE" },
        { 467u, 10u, 461u, 0u, 0u, "COORD.C:740-765 G3031 LayoutData16 record for C467_ZONE_DIALOG_BOTTOM_RIGHT_CHOICE" },
        { 469u, 0u, 468u, 112u, 49u, "COORD.C:740-765 G3031 LayoutData16 record for C469_ZONE_DIALOG" },
        { 471u, 0u, 470u, 112u, 32u, "COORD.C:740-765 G3031 LayoutData16 record for C471_ZONE_DIALOG" }
    };
    unsigned int i;
    if (!outRecord) return 0;
    for (i = 0u; i < (unsigned int)(sizeof(records) / sizeof(records[0])); ++i) {
        if (records[i].zoneIndex == zoneIndex) {
            *outRecord = records[i];
            return 1;
        }
    }
    return 0;
}

const char* DIALOG_Compat_GetSourceScheduleEvidence(void) {
    return "ReDMCSB DIALOG.C source schedule: F0427 expands dialog graphic, prints version, counts choices, patches 1/2/4-choice layouts (3-choice unpatched), prints choice zones, centers messages in C469/C471, blits/draws viewport and fades; F0424 swaps input tables, loops processQueue+VBlank until selected choice, draws click feedback with two VBlank-related refreshes and F0022_MAIN_Delay(5), then restores input tables.";
}
