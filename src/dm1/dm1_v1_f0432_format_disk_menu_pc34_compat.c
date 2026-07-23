#include "dm1_v1_f0432_format_disk_menu_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB Toolchains/Common/Source/LOADSAVE.C F0432:278-520; "
    "DIALOG.C F0424/F0427; MEMORY.C F0468/F0469. F0432 plans only the "
    "original format-disk dialog states; it owns no formatter or save path.";

static int source_text_is_valid(const DM1_V1_F0432SourceTextPc34 *text,
                                DM1_V1_F0432SourceTextIdPc34 expected)
{
    const char *terminator;

    if (!text || text->id != expected || !text->text.text ||
        !text->text.sourceTextVerified || text->text.textCapacity == 0u ||
        text->text.textCapacity > 60u) {
        return 0;
    }
    terminator = (const char *)memchr(text->text.text, '\0',
                                      text->text.textCapacity);
    return terminator != NULL && terminator != text->text.text;
}

static int source_material_is_valid(
    const DM1_V1_F0424F0427DialogMaterialPc34 *material)
{
    return material && material->indexedPixels &&
        material->sourceGraphicId == DM1_V1_F0424_F0427_DIALOG_GRAPHIC_PC34 &&
        material->width == DM1_V1_F0424_F0427_DIALOG_WIDTH_PC34 &&
        material->height == DM1_V1_F0424_F0427_DIALOG_HEIGHT_PC34 &&
        material->stride >= DM1_V1_F0424_F0427_DIALOG_WIDTH_PC34 &&
        material->indexedPixelCount >= (size_t)material->stride *
            (size_t)material->height && material->sourceGraphicsDatVerified &&
        material->sourcePaletteVerified && material->sourceFontVerified &&
        (material->fontGraphicId == DM1_V1_F0424_F0427_FONT_M653_PC34 ||
         material->fontGraphicId == DM1_V1_F0424_F0427_FONT_M653_LEGACY_PC34);
}

static int admit_dialog(const DM1_V1_F0432FormatDiskRequestPc34 *request,
                        int choice_count,
                        int message2_index,
                        int choice_offset,
                        DM1_V1_F0432FormatDiskReceiptPc34 *receipt)
{
    DM1_V1_F0424F0427DialogRequestPc34 dialog_request;
    int index;

    memset(&dialog_request, 0, sizeof(dialog_request));
    dialog_request.material = request->material;
    dialog_request.message1 = &request->texts[0]->text;
    if (message2_index >= 0) {
        dialog_request.message2 = &request->texts[message2_index]->text;
    }
    dialog_request.choiceCount = choice_count;
    dialog_request.selectedChoice = request->selectedChoice;
    dialog_request.selectedChoiceFromSourceInput =
        request->selectedChoiceFromSourceInput;
    for (index = 0; index < choice_count; ++index) {
        dialog_request.choices[index] =
            &request->texts[index + choice_offset]->text;
    }
    return dm1_v1_f0424_f0427_dialog_admit_pc34(&dialog_request,
                                                 &receipt->dialogReceipt);
}

static DM1_V1_F0432FormatDiskDispositionPc34 disk_disposition(
    DM1_V1_F0432DiskTypePc34 disk_type)
{
    switch (disk_type) {
        case DM1_V1_F0432_DISK_GAME:
            return DM1_V1_F0432_FORMAT_DISK_SHOW_GAME_DISK_ERROR;
        case DM1_V1_F0432_DISK_SAVE_WRITE_PROTECTED:
            return DM1_V1_F0432_FORMAT_DISK_SHOW_WRITE_PROTECTED_ERROR;
        case DM1_V1_F0432_DISK_NONE:
            return DM1_V1_F0432_FORMAT_DISK_SHOW_NO_DISK_ERROR;
        case DM1_V1_F0432_DISK_SAVE_WRITE_ENABLED:
            return DM1_V1_F0432_FORMAT_DISK_REQUIRE_SAVE_DISK_CONFIRMATION;
        case DM1_V1_F0432_DISK_UNFORMATTED:
            return DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND;
        default:
            return DM1_V1_F0432_FORMAT_DISK_REJECTED;
    }
}

int dm1_v1_f0432_format_disk_menu_admit_pc34(
    const DM1_V1_F0432FormatDiskRequestPc34 *request,
    DM1_V1_F0432FormatDiskReceiptPc34 *outReceipt)
{
    DM1_V1_F0432FormatDiskReceiptPc34 receipt;
    DM1_V1_F0432FormatDiskDispositionPc34 disposition;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!request || !source_material_is_valid(request->material) ||
        request->textCount < 1 || request->textCount > 4) {
        *outReceipt = receipt;
        return 0;
    }
    receipt.stage = request->stage;
    receipt.usesOriginalDialogLayout = 1;
    receipt.suppressSyntheticFallback = 1;

    switch (request->stage) {
        case DM1_V1_F0432_FORMAT_DISK_STAGE_INSERT_BLANK:
            if (request->textCount != 3 ||
                !source_text_is_valid(request->texts[0], DM1_V1_F0432_TEXT_PUT_BLANK_DISK) ||
                !source_text_is_valid(request->texts[1], DM1_V1_F0432_TEXT_FORMAT_FLOPPY) ||
                !source_text_is_valid(request->texts[2], DM1_V1_F0432_TEXT_CANCEL) ||
                !admit_dialog(request, 2, -1, 1, &receipt) ||
                !request->diskTypeReceiptValid) {
                *outReceipt = receipt;
                return 0;
            }
            receipt.choiceCount = 2;
            if (request->selectedChoice == 2) {
                receipt.disposition = DM1_V1_F0432_FORMAT_DISK_CANCELLED;
                break;
            }
            disposition = disk_disposition(request->diskType);
            if (disposition == DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND &&
                (!request->sourceMemoryReceiptValid || request->availableHeapBytes == 0u)) {
                *outReceipt = receipt;
                return 0;
            }
            receipt.disposition = disposition;
            receipt.requiresOriginalMemoryBuffer =
                disposition == DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND;
            receipt.originalMemoryBufferBytes = receipt.requiresOriginalMemoryBuffer
                ? request->availableHeapBytes : 0u;
            break;
        case DM1_V1_F0432_FORMAT_DISK_STAGE_CONFIRM_SAVE_DISK:
            if (request->textCount != 4 ||
                !source_text_is_valid(request->texts[0], DM1_V1_F0432_TEXT_GAME_SAVE_DISK) ||
                !source_text_is_valid(request->texts[1], DM1_V1_F0432_TEXT_FORMAT_ANYWAY) ||
                !source_text_is_valid(request->texts[2], DM1_V1_F0432_TEXT_OK) ||
                !source_text_is_valid(request->texts[3], DM1_V1_F0432_TEXT_CANCEL) ||
                !admit_dialog(request, 2, 1, 2, &receipt) ||
                !request->diskTypeReceiptValid) {
                *outReceipt = receipt;
                return 0;
            }
            receipt.choiceCount = 2;
            if (request->selectedChoice == 2) {
                receipt.disposition = DM1_V1_F0432_FORMAT_DISK_RETURN_TO_PROMPT;
                break;
            }
            disposition = disk_disposition(request->diskType);
            if (disposition == DM1_V1_F0432_FORMAT_DISK_REQUIRE_SAVE_DISK_CONFIRMATION) {
                disposition = DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND;
            }
            if (disposition == DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND &&
                (!request->sourceMemoryReceiptValid || request->availableHeapBytes == 0u)) {
                *outReceipt = receipt;
                return 0;
            }
            receipt.disposition = disposition;
            receipt.requiresOriginalMemoryBuffer =
                disposition == DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND;
            receipt.originalMemoryBufferBytes = receipt.requiresOriginalMemoryBuffer
                ? request->availableHeapBytes : 0u;
            break;
        case DM1_V1_F0432_FORMAT_DISK_STAGE_FORMATTING:
            if (request->textCount != 1 ||
                !source_text_is_valid(request->texts[0], DM1_V1_F0432_TEXT_FORMATTING_DISK) ||
                !request->formatResultReceiptValid) {
                *outReceipt = receipt;
                return 0;
            }
            receipt.disposition = request->formatSucceeded
                ? DM1_V1_F0432_FORMAT_DISK_COMPLETE
                : DM1_V1_F0432_FORMAT_DISK_SHOW_FORMAT_FAILED;
            break;
        case DM1_V1_F0432_FORMAT_DISK_STAGE_FORMAT_FAILED:
            if (request->textCount != 2 ||
                !source_text_is_valid(request->texts[0], DM1_V1_F0432_TEXT_UNABLE_TO_FORMAT) ||
                !source_text_is_valid(request->texts[1], DM1_V1_F0432_TEXT_OK) ||
                !admit_dialog(request, 1, -1, 1, &receipt)) {
                *outReceipt = receipt;
                return 0;
            }
            receipt.choiceCount = 1;
            receipt.disposition = DM1_V1_F0432_FORMAT_DISK_RETURN_TO_PROMPT;
            break;
        default:
            *outReceipt = receipt;
            return 0;
    }
    receipt.accepted = receipt.disposition != DM1_V1_F0432_FORMAT_DISK_REJECTED;
    *outReceipt = receipt;
    return receipt.accepted;
}

const char *dm1_v1_f0432_format_disk_menu_source_evidence_pc34(void)
{
    return s_source_evidence;
}
