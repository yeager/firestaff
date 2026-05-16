#ifndef REDMCSB_IMAGE_BACKEND_PC34_COMPAT_H
#define REDMCSB_IMAGE_BACKEND_PC34_COMPAT_H

extern unsigned short G2157_;
extern unsigned char* G2159_puc_Bitmap_Source;
extern unsigned char* G2160_puc_Bitmap_Destination;

unsigned char F0687_IMG3_GetNibble(void);
short F0688_IMG3_GetPixelCount(void);
void F0685_IMG3_LineColorFilling(unsigned short P2344_ui_, char P2345_c_Color, int P2346_i_);
void F0686_IMG_CopyFromPreviousLine(unsigned short P2350_ui_, unsigned short P2351_ui_, int P2352_i_);
void IMG3_Compat_ExpandOneCommandSameStride(const unsigned char* P3000_puc_LocalPalette, unsigned short P3001_ui_LineStride, unsigned short* P3002_pui_DestinationOffset);
unsigned short IMG3_Compat_ExpandOneCommandPadded(const unsigned char* P3000_puc_LocalPalette, unsigned short P3001_ui_VisibleWidth, unsigned short P3002_ui_LineStride, unsigned short* P3003_pui_DestinationOffset, unsigned short* P3004_pui_RemainingInLine);
void IMG3_Compat_ExpandCommandsSameStride(const unsigned char* P3000_puc_LocalPalette, unsigned short P3001_ui_LineStride, unsigned short P3002_ui_TotalPixelCount, unsigned short* P3003_pui_DestinationOffset);
void IMG3_Compat_ExpandCommandsPadded(const unsigned char* P3000_puc_LocalPalette, unsigned short P3001_ui_VisibleWidth, unsigned short P3002_ui_LineStride, unsigned short P3003_ui_TotalPixelCount, unsigned short* P3004_pui_DestinationOffset, unsigned short* P3005_pui_RemainingInLine);
void IMG3_Compat_ExpandFromSource(const unsigned char* P3000_puc_BitmapSource, unsigned char* P3001_puc_BitmapDestination);

#endif
