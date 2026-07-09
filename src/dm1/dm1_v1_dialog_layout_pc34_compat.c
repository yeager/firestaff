#include "dm1_v1_dialog_layout_pc34_compat.h"

enum {
    kDialogViewportX = 0,
    kDialogViewportY = 33,
    kSourceTextHeight = 6,
    kSourceTextPad = 1,
    kSourceTextBaseline = 6
};

static int set_rect(DM1_V1_DialogRectPc34* out,
                    int x,
                    int y,
                    int w,
                    int h) {
    if (!out) return 0;
    out->x = x;
    out->y = y;
    out->w = w;
    out->h = h;
    return 1;
}

static int message_text_y_for_rect(int lineCount, int zoneY, int zoneH) {
    int count = lineCount < 1 ? 1 : lineCount;
    int block = count * (kSourceTextHeight + (kSourceTextPad * 2) - 1 + 1);
    int relativeY = zoneY + ((zoneH - (block - (kSourceTextPad * 2))) / 2) +
                    kSourceTextBaseline - 1;
    return kDialogViewportY + relativeY;
}

const char* dm1_v1_dialog_layout_source_evidence_pc34(void) {
    return "ReDMCSB DIALOG.C F0427/F0424; COORD.C C450..C471; "
           "DEFS.H C456..C467; G2067/G2068 viewport origin";
}

int dm1_v1_dialog_version_text_origin_pc34(int* outX, int* outY) {
    if (outX) *outX = kDialogViewportX + 192;
    if (outY) *outY = kDialogViewportY + 7;
    return 1;
}

int dm1_v1_dialog_choice_patch_pc34(
    int choiceCount,
    DM1_V1_DialogPatchPc34* out) {
    if (!out || choiceCount == 3) return 0;
    if (choiceCount <= 1) {
        out->srcX = 0;
        out->srcY = 14;
        out->w = 224;
        out->h = 75;
        out->dstX = 0;
        out->dstY = 51;
    } else if (choiceCount == 2) {
        out->srcX = 102;
        out->srcY = 52;
        out->w = 21;
        out->h = 37;
        out->dstX = 102;
        out->dstY = 89;
    } else {
        out->srcX = 102;
        out->srcY = 99;
        out->w = 21;
        out->h = 36;
        out->dstX = 102;
        out->dstY = 62;
    }
    return 1;
}

int dm1_v1_dialog_single_choice_message_text_y_pc34(int lineCount) {
    return message_text_y_for_rect(lineCount, 49, 25);
}

int dm1_v1_dialog_multi_choice_message_text_y_pc34(int lineCount) {
    return message_text_y_for_rect(lineCount, 32, 5);
}

int dm1_v1_dialog_message_rect_pc34(
    int choiceCount,
    DM1_V1_DialogRectPc34* out) {
    if (choiceCount > 1) {
        return set_rect(out, 112, 32, 77, 5);
    }
    return set_rect(out, 112, 49, 77, 25);
}

int dm1_v1_dialog_message_width_pc34(int choiceCount) {
    DM1_V1_DialogRectPc34 r;
    if (!dm1_v1_dialog_message_rect_pc34(choiceCount, &r)) return 0;
    return r.w;
}

int dm1_v1_dialog_choice_text_zone_id_pc34(
    int choiceCount,
    int choiceIndex) {
    if (choiceIndex < 0) return 0;
    switch (choiceCount) {
        case 1:
            return choiceIndex == 0 ? 462 : 0;
        case 2:
            if (choiceIndex == 0) return 463;
            if (choiceIndex == 1) return 462;
            return 0;
        case 3:
            if (choiceIndex == 0) return 463;
            if (choiceIndex == 1) return 466;
            if (choiceIndex == 2) return 467;
            return 0;
        default:
            if (choiceCount < 4) return 0;
            if (choiceIndex == 0) return 464;
            if (choiceIndex == 1) return 465;
            if (choiceIndex == 2) return 466;
            if (choiceIndex == 3) return 467;
            return 0;
    }
}

int dm1_v1_dialog_choice_text_rect_pc34(
    int choiceCount,
    int choiceIndex,
    DM1_V1_DialogRectPc34* out) {
    int zoneId = dm1_v1_dialog_choice_text_zone_id_pc34(choiceCount,
                                                        choiceIndex);
    switch (zoneId) {
        case 462: return set_rect(out, 16, 110, 192, 7);
        case 463: return set_rect(out, 16, 73, 192, 7);
        case 464: return set_rect(out, 16, 73, 86, 7);
        case 465: return set_rect(out, 123, 73, 86, 7);
        case 466: return set_rect(out, 16, 110, 86, 7);
        case 467: return set_rect(out, 123, 110, 86, 7);
        default: return 0;
    }
}

int dm1_v1_dialog_choice_button_zone_id_pc34(
    int choiceCount,
    int choiceIndex) {
    if (choiceIndex < 0) return 0;
    switch (choiceCount) {
        case 1:
            return choiceIndex == 0 ? 456 : 0;
        case 2:
            if (choiceIndex == 0) return 457;
            if (choiceIndex == 1) return 456;
            return 0;
        case 3:
            if (choiceIndex == 0) return 457;
            if (choiceIndex == 1) return 460;
            if (choiceIndex == 2) return 461;
            return 0;
        default:
            if (choiceCount < 4) return 0;
            if (choiceIndex == 0) return 458;
            if (choiceIndex == 1) return 459;
            if (choiceIndex == 2) return 460;
            if (choiceIndex == 3) return 461;
            return 0;
    }
}

int dm1_v1_dialog_choice_hit_rect_pc34(
    int choiceCount,
    int choiceIndex,
    DM1_V1_DialogRectPc34* out) {
    int zoneId = dm1_v1_dialog_choice_button_zone_id_pc34(choiceCount,
                                                          choiceIndex);
    switch (zoneId) {
        case 456: return set_rect(out, 16, 104, 192, 17);
        case 457: return set_rect(out, 16, 67, 192, 17);
        case 458: return set_rect(out, 16, 67, 86, 17);
        case 459: return set_rect(out, 123, 67, 86, 17);
        case 460: return set_rect(out, 16, 104, 86, 17);
        case 461: return set_rect(out, 123, 104, 86, 17);
        default: return 0;
    }
}
