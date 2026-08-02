/* DM2 SKSAVE record suppress mask tables.
 * Source: skproject dm2data.cpp vsgame[120], table1d64db[16],
 *         skrecord.cpp table_recordsizes[16]. */
#include "dm2_v1_save_record_masks_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const uint8_t *sizes = dm2_v1_save_record_sizes();
    const uint8_t *mask;
    size_t vsgame_size;

    /* table_recordsizes[16] */
    assert(sizes[0] == 4);
    assert(sizes[1] == 6);
    assert(sizes[2] == 4);
    assert(sizes[3] == 8);
    assert(sizes[4] == 16);
    assert(sizes[5] == 4);
    assert(sizes[6] == 4);
    assert(sizes[7] == 4);
    assert(sizes[8] == 4);
    assert(sizes[9] == 8);
    assert(sizes[10] == 4);
    assert(sizes[11] == 0);
    assert(sizes[12] == 0);
    assert(sizes[13] == 0);
    assert(sizes[14] == 8);
    assert(sizes[15] == 4);

    /* Types with NULL masks */
    assert(dm2_v1_save_record_mask_for_type(1) == NULL);
    assert(dm2_v1_save_record_mask_for_type(11) == NULL);
    assert(dm2_v1_save_record_mask_for_type(12) == NULL);
    assert(dm2_v1_save_record_mask_for_type(13) == NULL);
    assert(dm2_v1_save_record_mask_for_type(-1) == NULL);
    assert(dm2_v1_save_record_mask_for_type(16) == NULL);

    /* Type 0 mask: vsgame+0x0c = {0x00, 0x00, 0x00, 0x3e} (4 bytes) */
    mask = dm2_v1_save_record_mask_for_type(0);
    assert(mask != NULL);
    assert(mask[0] == 0x00);
    assert(mask[3] == 0x3e);

    /* Type 4 mask (creature): vsgame+0x1c, 16 bytes */
    mask = dm2_v1_save_record_mask_for_type(4);
    assert(mask != NULL);
    assert(mask[0] == 0x00);

    /* Creature AI spec alternate mask: vsgame+0x2c */
    mask = dm2_v1_save_record_mask_creature_ai_spec();
    assert(mask != NULL);
    assert(mask[0] == 0x00);
    assert(mask[6] == 0xff);
    assert(mask[7] == 0x3f);

    /* Container map alternate: vsgame+0x54 */
    mask = dm2_v1_save_record_mask_container_map();
    assert(mask != NULL);

    /* Misc default vs moneybox: vsgame+0x5c vs 0x60 */
    assert(dm2_v1_save_record_mask_misc_default() != NULL);
    assert(dm2_v1_save_record_mask_misc_moneybox() != NULL);
    assert(dm2_v1_save_record_mask_misc_default() !=
           dm2_v1_save_record_mask_misc_moneybox());

    /* Type 0xe nested: vsgame+0x70 */
    mask = dm2_v1_save_record_mask_type_0e_nested();
    assert(mask != NULL);

    /* vsgame raw */
    mask = dm2_v1_save_vsgame_raw(&vsgame_size);
    assert(mask != NULL);
    assert(vsgame_size == 120);
    assert(mask[0] == 0xff);
    assert(mask[119] == 0x00);

    printf("PASS: dm2_v1_save_record_masks\n");
    return 0;
}
