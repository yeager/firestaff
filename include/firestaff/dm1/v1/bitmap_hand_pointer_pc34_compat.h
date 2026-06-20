#ifndef FIRESTAFF_DM1_V1_BITMAPHANDPOINTER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BITMAPHANDPOINTER_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0043_auc_Graphic562_Bitmap_HandPointer[2][128].
 *
 * G0043 is the hand-pointer mouse-cursor 4bpp-packed bitmap
 * (2 planes × 128 bytes per plane, 16 rows × 8 bytes/row). The
 * first plane is the colored hand shape (palette-mapped), the
 * second plane is the outline mask. PC 3.4 init block:
 * DATA.C:385-410. Read sites: IO.C cursor blit path (the G0043
 * bitmap + G0045 PaletteChanges_MousePointerIconShadow form the
 * hand-pointer pair).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-852.
 */

#define DM1_V1_BITMAP_HAND_POINTER_PC34_COMPAT_PLANES 2
#define DM1_V1_BITMAP_HAND_POINTER_PC34_COMPAT_PLANE_BYTES 128

typedef struct DM1_V1_BitmapHandPointerResultPc34 {
    int accepted;
    int assertionCount;
    int planeSizeBytes;
    int tableMatchesDeclaration;
    int plane0FirstByte0xBB;
    int plane0LastByte0xBB;
    int plane1FirstByte0xFF;
    int plane1LastByte0xFF;
    int planesByteIdentical;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_BitmapHandPointerResultPc34;

const unsigned char *
dm1_v1_bitmap_hand_pointer_plane_pc34(int plane_index);

int
dm1_v1_bitmap_hand_pointer_plane_bytes_pc34(void);

int
dm1_v1_bitmap_hand_pointer_get_pc34(int plane_index, int byte_index);

int
dm1_v1_bitmap_hand_pointer_run_pc34(
    DM1_V1_BitmapHandPointerResultPc34 *out);

#endif