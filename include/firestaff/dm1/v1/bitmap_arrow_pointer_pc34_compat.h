#ifndef FIRESTAFF_DM1_V1_BITMAPARROWPOINTER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BITMAPARROWPOINTER_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0042_auc_Graphic562_Bitmap_ArrowPointer[2][66].
 *
 * G0042 is the arrow-pointer mouse-cursor 4bpp-packed bitmap
 * (2 planes × 66 bytes per plane, 11 rows × 6 bytes/row). The
 * first plane is the colored arrow shape (palette-mapped), the
 * second plane is the outline mask. PC 3.4 init block:
 * DATA.C:362-376. Read sites: IO.C cursor blit path (the G0042
 * bitmap + G0044 PaletteChanges_MousePointerIcon + G0046
 * ChampionColor form the arrow-pointer triplet).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-852.
 */

#define DM1_V1_BITMAP_ARROW_POINTER_PC34_COMPAT_PLANES 2
#define DM1_V1_BITMAP_ARROW_POINTER_PC34_COMPAT_PLANE_BYTES 66

typedef struct DM1_V1_BitmapArrowPointerResultPc34 {
    int accepted;
    int assertionCount;
    int planeSizeBytes;
    int tableMatchesDeclaration;
    int plane0FirstByte0xFF;
    int plane0LastByte0x00;
    int plane1FirstByte0xFF;
    int plane1LastByte0x00;
    int planesByteIdentical;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_BitmapArrowPointerResultPc34;

const unsigned char *
dm1_v1_bitmap_arrow_pointer_plane_pc34(int plane_index);

int
dm1_v1_bitmap_arrow_pointer_plane_bytes_pc34(void);

int
dm1_v1_bitmap_arrow_pointer_get_pc34(int plane_index, int byte_index);

int
dm1_v1_bitmap_arrow_pointer_run_pc34(
    DM1_V1_BitmapArrowPointerResultPc34 *out);

#endif
