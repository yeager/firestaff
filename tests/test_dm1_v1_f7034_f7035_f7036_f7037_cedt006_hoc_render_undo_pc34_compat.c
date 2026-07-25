#include "dm1_v1_cedt006_champion_editor_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static DM1_V1_CEDT006_BoxPc34 box(int x, int y, int width, int height)
{
    DM1_V1_CEDT006_BoxPc34 out;

    out.x = x;
    out.y = y;
    out.width = width;
    out.height = height;
    out.sourceBoxProven = 1;
    return out;
}

static void test_button_receipt_uses_proven_box_and_text(void)
{
    DM1_V1_CEDT006_BoxPc34 buttonBox = box(12, 34, 48, 11);
    DM1_V1_CEDT006_ButtonReceiptPc34 receipt;
    (void)receipt;

    assert(F7034_DrawButton(&buttonBox, "UNDO", 3, &receipt) == 1);
    assert(receipt.valid == 1);
    assert(receipt.box.x == 12);
    assert(receipt.box.y == 34);
    assert(receipt.box.width == 48);
    assert(receipt.box.height == 11);
    assert(strcmp(receipt.text, "UNDO") == 0);
    assert(receipt.fillColor == 3);
    assert(receipt.drawBoxRequested == 1);
    assert(receipt.drawTextRequested == 1);
    assert(receipt.sourceLineStart == 398);

    buttonBox.sourceBoxProven = 0;
    assert(F7034_DrawButton(&buttonBox, "UNDO", 3, &receipt) == 0);
    buttonBox.sourceBoxProven = 1;
    assert(F7034_DrawButton(&buttonBox, 0, 3, &receipt) == 0);
    assert(F7034_DrawButton(&buttonBox, "UNDO", 99, &receipt) == 0);
}

static void test_color_selection_receipts_are_source_bounded(void)
{
    DM1_V1_CEDT006_BoxPc34 previousBox = box(4, 5, 6, 7);
    (void)previousBox;
    DM1_V1_CEDT006_BoxPc34 selectedBox = box(14, 15, 6, 7);
    DM1_V1_CEDT006_SelectedColorBoxReceiptPc34 boxReceipt;
    (void)boxReceipt;
    DM1_V1_CEDT006_SelectedColorIndexReceiptPc34 indexReceipt;
    (void)indexReceipt;

    assert(F7035_SetSelectedColorBox(2, &selectedBox, &boxReceipt) == 1);
    assert(boxReceipt.valid == 1);
    assert(boxReceipt.colorIndex == 2);
    assert(boxReceipt.colorBox.x == 14);
    assert(boxReceipt.drawSelectionBoxRequested == 1);
    assert(boxReceipt.sourceLineStart == 423);

    assert(F7036_SetSelectedColorIndex(2, 5, &previousBox, &selectedBox,
                                       &indexReceipt) == 1);
    assert(indexReceipt.valid == 1);
    assert(indexReceipt.previousColorIndex == 2);
    assert(indexReceipt.selectedColorIndex == 5);
    assert(indexReceipt.clearPreviousRequested == 1);
    assert(indexReceipt.drawSelectedRequested == 1);
    assert(indexReceipt.sourceLineStart == 442);

    assert(F7036_SetSelectedColorIndex(5, 5, &selectedBox, &selectedBox,
                                       &indexReceipt) == 1);
    assert(indexReceipt.clearPreviousRequested == 0);

    assert(F7035_SetSelectedColorBox(-1, &selectedBox, &boxReceipt) == 0);
    selectedBox.width = 0;
    assert(F7036_SetSelectedColorIndex(1, 2, &previousBox, &selectedBox,
                                       &indexReceipt) == 0);
}

static void test_undo_bitmap_copies_only_proven_caller_bytes(void)
{
    const uint8_t portraitBytes[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    (void)portraitBytes;
    uint8_t undoBytes[8];
    DM1_V1_CEDT006_UndoBitmapReceiptPc34 receipt;
    (void)receipt;

    memset(undoBytes, 0xcc, sizeof(undoBytes));
    assert(F7037_UpdateUndoBitmap(portraitBytes, sizeof(portraitBytes), 1,
                                  undoBytes, sizeof(undoBytes),
                                  &receipt) == 1);
    assert(receipt.valid == 1);
    assert(receipt.byteCount == sizeof(portraitBytes));
    assert(receipt.sourceBytesProven == 1);
    assert(receipt.copiedByteCount == 8);
    assert(receipt.sourceLineStart == 460);
    assert(memcmp(undoBytes, portraitBytes, sizeof(portraitBytes)) == 0);

    memset(undoBytes, 0xcc, sizeof(undoBytes));
    assert(F7037_UpdateUndoBitmap(portraitBytes, sizeof(portraitBytes), 0,
                                  undoBytes, sizeof(undoBytes),
                                  &receipt) == 0);
    assert(undoBytes[0] == 0xcc);
    assert(F7037_UpdateUndoBitmap(portraitBytes, sizeof(portraitBytes), 1,
                                  undoBytes, 4, &receipt) == 0);
    assert(F7037_UpdateUndoBitmap(0, sizeof(portraitBytes), 1,
                                  undoBytes, sizeof(undoBytes),
                                  &receipt) == 0);
}

static void test_source_evidence_names_no_synthetic_boundaries(void)
{
    const char *evidence = F7034_F7035_F7036_F7037_CEDT006_SourceEvidencePc34();
    (void)evidence;

    assert(strstr(evidence, "CEDT006.C:398") != 0);
    assert(strstr(evidence, "CEDT006.C:423") != 0);
    assert(strstr(evidence, "CEDT006.C:442") != 0);
    assert(strstr(evidence, "CEDT006.C:460") != 0);
    assert(strstr(evidence, "caller-owned boxes") != 0);
    assert(strstr(evidence, "does not synthesize") != 0);
    assert(strstr(evidence, "screen pixels") != 0);
    assert(strstr(evidence, "portrait bytes") != 0);
}

int main(void)
{
    test_button_receipt_uses_proven_box_and_text();
    test_color_selection_receipts_are_source_bounded();
    test_undo_bitmap_copies_only_proven_caller_bytes();
    test_source_evidence_names_no_synthetic_boundaries();
    return 0;
}
