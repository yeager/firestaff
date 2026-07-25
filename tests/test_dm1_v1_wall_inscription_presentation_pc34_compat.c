#include "dm1_v1_wall_inscription_presentation_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_projection_enum(void)
{
    int clear = DM1_V1_INSCRIPTION_PROJECTION_CLEAR_ONLY_PC34;
    int front = DM1_V1_INSCRIPTION_PROJECTION_D1C_FRONT_PC34;
    int side = DM1_V1_INSCRIPTION_PROJECTION_SIDE_OR_DEPTH_PC34;
    (void)clear;
    (void)front;
    (void)side;
    assert(clear == 0);
    assert(front == 1);
    assert(side == 2);
}

static void test_receipt_struct_layout(void)
{
    DM1_V1_WallInscriptionPresentationReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    assert(receipt.valid == 0);
    assert(receipt.lineCount == 0);
}

static void test_viewport_receipt_struct_layout(void)
{
    DM1_V1_ViewportInscriptionReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    assert(receipt.valid == 0);
    assert(receipt.clearPreviousMaterial == 0);
    assert(receipt.drawFrontMaterial == 0);
}

int main(void)
{
    test_projection_enum();
    test_receipt_struct_layout();
    test_viewport_receipt_struct_layout();
    puts("ok: DM1 wall inscription presentation (Q-DM1-03) 3 tests passed");
    return 0;
}
