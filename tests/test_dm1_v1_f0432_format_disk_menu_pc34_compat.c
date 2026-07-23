#include "dm1_v1_f0432_format_disk_menu_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static DM1_V1_F0432SourceTextPc34 text(
    DM1_V1_F0432SourceTextIdPc34 id, const char *value)
{
    DM1_V1_F0432SourceTextPc34 result;
    result.id = id;
    result.text.text = value;
    result.text.textCapacity = strlen(value) + 1u;
    result.text.sourceTextVerified = 1;
    return result;
}

int main(void)
{
    static uint8_t pixels[224 * 136];
    DM1_V1_F0424F0427DialogMaterialPc34 material;
    DM1_V1_F0432SourceTextPc34 put_blank = text(DM1_V1_F0432_TEXT_PUT_BLANK_DISK, "PUT BLANK DISK");
    DM1_V1_F0432SourceTextPc34 format = text(DM1_V1_F0432_TEXT_FORMAT_FLOPPY, "FORMAT FLOPPY");
    DM1_V1_F0432SourceTextPc34 cancel = text(DM1_V1_F0432_TEXT_CANCEL, "CANCEL");
    DM1_V1_F0432SourceTextPc34 save_disk = text(DM1_V1_F0432_TEXT_GAME_SAVE_DISK, "GAME SAVE DISK");
    DM1_V1_F0432SourceTextPc34 anyway = text(DM1_V1_F0432_TEXT_FORMAT_ANYWAY, "FORMAT DISK ANYWAY");
    DM1_V1_F0432SourceTextPc34 ok = text(DM1_V1_F0432_TEXT_OK, "OK");
    DM1_V1_F0432SourceTextPc34 formatting = text(DM1_V1_F0432_TEXT_FORMATTING_DISK, "FORMATTING DISK");
    DM1_V1_F0432SourceTextPc34 unable = text(DM1_V1_F0432_TEXT_UNABLE_TO_FORMAT, "UNABLE TO FORMAT DISK");
    DM1_V1_F0432FormatDiskRequestPc34 request;
    DM1_V1_F0432FormatDiskReceiptPc34 receipt;

    memset(&material, 0, sizeof(material));
    material.sourceGraphicId = 17;
    material.indexedPixels = pixels;
    material.indexedPixelCount = sizeof(pixels);
    material.width = 224;
    material.height = 136;
    material.stride = 224;
    material.sourceGraphicsDatVerified = 1;
    material.sourcePaletteVerified = 1;
    material.fontGraphicId = 695;
    material.sourceFontVerified = 1;
    memset(&request, 0, sizeof(request));
    request.material = &material;
    request.stage = DM1_V1_F0432_FORMAT_DISK_STAGE_INSERT_BLANK;
    request.texts[0] = &put_blank;
    request.texts[1] = &format;
    request.texts[2] = &cancel;
    request.textCount = 3;
    request.selectedChoice = 1;
    request.selectedChoiceFromSourceInput = 1;
    request.diskType = DM1_V1_F0432_DISK_UNFORMATTED;
    request.diskTypeReceiptValid = 1;
    request.availableHeapBytes = 32768u;
    request.sourceMemoryReceiptValid = 1;

    CHECK(strstr(dm1_v1_f0432_format_disk_menu_source_evidence_pc34(), "F0432") != NULL);
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt));
    CHECK(receipt.accepted && receipt.usesOriginalDialogLayout &&
          receipt.choiceCount == 2 &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND &&
          receipt.requiresOriginalMemoryBuffer &&
          receipt.originalMemoryBufferBytes == 32768u &&
          receipt.dialogReceipt.accepted && receipt.suppressSyntheticFallback);

    request.sourceMemoryReceiptValid = 0;
    CHECK(!dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt));
    request.sourceMemoryReceiptValid = 1;
    request.diskType = DM1_V1_F0432_DISK_SAVE_WRITE_ENABLED;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_REQUIRE_SAVE_DISK_CONFIRMATION);
    request.diskType = DM1_V1_F0432_DISK_GAME;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_SHOW_GAME_DISK_ERROR &&
          !receipt.requiresOriginalMemoryBuffer);
    request.diskType = DM1_V1_F0432_DISK_SAVE_WRITE_PROTECTED;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_SHOW_WRITE_PROTECTED_ERROR);

    request.stage = DM1_V1_F0432_FORMAT_DISK_STAGE_CONFIRM_SAVE_DISK;
    request.texts[0] = &save_disk;
    request.texts[1] = &anyway;
    request.texts[2] = &ok;
    request.texts[3] = &cancel;
    request.textCount = 4;
    request.selectedChoice = 1;
    request.diskType = DM1_V1_F0432_DISK_SAVE_WRITE_ENABLED;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND &&
          receipt.requiresOriginalMemoryBuffer);
    request.selectedChoice = 2;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_RETURN_TO_PROMPT);

    request.stage = DM1_V1_F0432_FORMAT_DISK_STAGE_FORMATTING;
    request.texts[0] = &formatting;
    request.textCount = 1;
    request.formatResultReceiptValid = 1;
    request.formatSucceeded = 0;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_SHOW_FORMAT_FAILED);
    request.formatSucceeded = 1;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_COMPLETE);

    request.stage = DM1_V1_F0432_FORMAT_DISK_STAGE_FORMAT_FAILED;
    request.texts[0] = &unable;
    request.texts[1] = &ok;
    request.textCount = 2;
    request.selectedChoice = 1;
    CHECK(dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt) &&
          receipt.choiceCount == 1 &&
          receipt.disposition == DM1_V1_F0432_FORMAT_DISK_RETURN_TO_PROMPT);

    material.fontGraphicId = 1;
    CHECK(!dm1_v1_f0432_format_disk_menu_admit_pc34(&request, &receipt));
    printf("test_dm1_v1_f0432_format_disk_menu_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
