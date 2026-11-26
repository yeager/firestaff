#ifndef CSB_V1_ATARI_F0115_ALCOVE_SHIFT_H
#define CSB_V1_ATARI_F0115_ALCOVE_SHIFT_H

/* ReDMCSB DUNVIEW.C F0115 advances G0217 only after the G0218 alcove
 * coordinate has a nonzero Y value; sprite visibility does not gate it. */
unsigned int csb_v1_atari_f0115_alcove_shift_advance(
    unsigned int shift_index, int alcove_y);

#endif
