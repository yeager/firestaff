#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "image_backend_pc34_compat.h"

/*
 Early compatibility layer for tiny named parts of the legacy IMAGE backend.
 Focus: nibble-level pixel writes used by AtariST/PC bitmap expansion paths.
*/

STATICFUNCTION unsigned char F0789_IMG_GetNibble(
unsigned short P2341_ui_ FINAL_SEPARATOR
{
        unsigned char L0001_uc_Value;


        L0001_uc_Value = G2160_puc_Bitmap_Destination[P2341_ui_ >> 1];
        if (P2341_ui_ & 1) {
                return L0001_uc_Value & 0x0F;
        }
        return L0001_uc_Value >> 4;
}

STATICFUNCTION void F0790_(
unsigned short P2342_ui_ SEPARATOR
char           P2343_c_ FINAL_SEPARATOR
{
        unsigned short L0001_ui_ByteIndex;
        unsigned char  L0002_uc_Value;
        unsigned char  L0003_uc_Color;


        L0001_ui_ByteIndex = P2342_ui_ >> 1;
        L0003_uc_Color = (unsigned char)P2343_c_ & 0x0F;
        L0002_uc_Value = G2160_puc_Bitmap_Destination[L0001_ui_ByteIndex];
        if (P2342_ui_ & 1) {
                G2160_puc_Bitmap_Destination[L0001_ui_ByteIndex] = (L0002_uc_Value & 0xF0) | L0003_uc_Color;
        } else {
                G2160_puc_Bitmap_Destination[L0001_ui_ByteIndex] = (L0002_uc_Value & 0x0F) | (unsigned char)(L0003_uc_Color << 4);
        }
}

unsigned char F0687_IMG3_GetNibble(
void
)
{
        if ((G2157_ & 1) != 0) {
                return G2159_puc_Bitmap_Source[G2157_++ >> 1] & 0x0F;
        }
        return G2159_puc_Bitmap_Source[G2157_++ >> 1] >> 4;
}

short F0688_IMG3_GetPixelCount(
void
)
{
        short L2714_i_;


        if ((L2714_i_ = F0687_IMG3_GetNibble()) == 15) {
                if ((L2714_i_ = (F0687_IMG3_GetNibble() << 4) | F0687_IMG3_GetNibble()) == 255) {
                        L2714_i_ = (F0687_IMG3_GetNibble() << 12) | (F0687_IMG3_GetNibble() << 8) | (F0687_IMG3_GetNibble() << 4) | F0687_IMG3_GetNibble();
                } else {
                        L2714_i_ += 17;
                }
        } else {
                L2714_i_ += 2;
        }
        return L2714_i_;
}

void F0685_IMG3_LineColorFilling(
unsigned short P2344_ui_      SEPARATOR
char           P2345_c_Color  SEPARATOR
int            P2346_i_       FINAL_SEPARATOR
{
        while (P2346_i_-- > 0) {
                F0790_(P2344_ui_++, P2345_c_Color);
        }
}

void F0686_IMG_CopyFromPreviousLine(
unsigned short P2350_ui_ SEPARATOR
unsigned short P2351_ui_ SEPARATOR
int            P2352_i_  FINAL_SEPARATOR
{
        while (P2352_i_-- > 0) {
                F0790_(P2350_ui_++, (char)F0789_IMG_GetNibble(P2351_ui_++));
        }
}

STATICFUNCTION void IMG3_Compat_WriteRunPadded(
unsigned char  P3000_uc_Kind             SEPARATOR
char           P3001_c_Color             SEPARATOR
unsigned short P3002_ui_Count            SEPARATOR
unsigned short P3003_ui_VisibleWidth     SEPARATOR
unsigned short P3004_ui_LineStride       SEPARATOR
unsigned short* P3005_pui_DestinationOffset SEPARATOR
unsigned short* P3006_pui_RemainingInLine FINAL_SEPARATOR
{
        unsigned short L3001_ui_Count;
        unsigned short L3002_ui_DestinationOffset;
        unsigned short L3003_ui_RemainingInLine;
        unsigned short L3004_ui_Chunk;


        L3001_ui_Count = P3002_ui_Count;
        L3002_ui_DestinationOffset = *P3005_pui_DestinationOffset;
        L3003_ui_RemainingInLine = *P3006_pui_RemainingInLine;
        while (L3001_ui_Count > 0) {
                L3004_ui_Chunk = (L3001_ui_Count >= L3003_ui_RemainingInLine) ? L3003_ui_RemainingInLine : L3001_ui_Count;
                if (P3000_uc_Kind == 6) {
                        F0686_IMG_CopyFromPreviousLine(L3002_ui_DestinationOffset, L3002_ui_DestinationOffset - P3004_ui_LineStride, L3004_ui_Chunk);
                } else {
                        F0685_IMG3_LineColorFilling(L3002_ui_DestinationOffset, P3001_c_Color, L3004_ui_Chunk);
                }
                L3002_ui_DestinationOffset += L3004_ui_Chunk;
                L3001_ui_Count -= L3004_ui_Chunk;
                L3003_ui_RemainingInLine -= L3004_ui_Chunk;
                if (L3003_ui_RemainingInLine == 0) {
                        L3002_ui_DestinationOffset += P3004_ui_LineStride - P3003_ui_VisibleWidth;
                        L3003_ui_RemainingInLine = P3003_ui_VisibleWidth;
                }
        }
        *P3005_pui_DestinationOffset = L3002_ui_DestinationOffset;
        *P3006_pui_RemainingInLine = L3003_ui_RemainingInLine;
}

void IMG3_Compat_ExpandOneCommandSameStride(
const unsigned char* P3000_puc_LocalPalette SEPARATOR
unsigned short       P3001_ui_LineStride     SEPARATOR
unsigned short*      P3002_pui_DestinationOffset FINAL_SEPARATOR
{
        unsigned char  L3001_uc_Command;
        unsigned char  L3002_uc_Kind;
        unsigned short L3003_ui_Count;
        char           L3004_c_Color;
        unsigned short L3005_ui_DestinationOffset;


        L3001_uc_Command = F0687_IMG3_GetNibble();
        L3002_uc_Kind = L3001_uc_Command & 0x07;
        L3005_ui_DestinationOffset = *P3002_pui_DestinationOffset;
        if (L3002_uc_Kind == 6) {
                if (L3001_uc_Command & 0x08) {
                        L3003_ui_Count = (unsigned short)F0688_IMG3_GetPixelCount();
                } else {
                        L3003_ui_Count = 1;
                }
                F0686_IMG_CopyFromPreviousLine(L3005_ui_DestinationOffset, L3005_ui_DestinationOffset - P3001_ui_LineStride, L3003_ui_Count);
        } else {
                if (L3002_uc_Kind < 6) {
                        L3004_c_Color = (char)P3000_puc_LocalPalette[L3002_uc_Kind];
                } else {
                        L3004_c_Color = (char)F0687_IMG3_GetNibble();
                }
                if (L3001_uc_Command & 0x08) {
                        L3003_ui_Count = (unsigned short)F0688_IMG3_GetPixelCount();
                } else {
                        L3003_ui_Count = 1;
                }
                F0685_IMG3_LineColorFilling(L3005_ui_DestinationOffset, L3004_c_Color, L3003_ui_Count);
        }
        *P3002_pui_DestinationOffset = L3005_ui_DestinationOffset + L3003_ui_Count;
}

unsigned short IMG3_Compat_ExpandOneCommandPadded(
const unsigned char* P3000_puc_LocalPalette SEPARATOR
unsigned short       P3001_ui_VisibleWidth   SEPARATOR
unsigned short       P3002_ui_LineStride     SEPARATOR
unsigned short*      P3003_pui_DestinationOffset SEPARATOR
unsigned short*      P3004_pui_RemainingInLine FINAL_SEPARATOR
{
        unsigned char  L3001_uc_Command;
        unsigned char  L3002_uc_Kind;
        unsigned short L3003_ui_Count;
        char           L3004_c_Color;


        L3001_uc_Command = F0687_IMG3_GetNibble();
        L3002_uc_Kind = L3001_uc_Command & 0x07;
        if (L3002_uc_Kind < 6) {
                L3004_c_Color = (char)P3000_puc_LocalPalette[L3002_uc_Kind];
        } else if (L3002_uc_Kind == 6) {
                L3004_c_Color = 0;
        } else {
                L3004_c_Color = (char)F0687_IMG3_GetNibble();
        }
        if (L3001_uc_Command & 0x08) {
                L3003_ui_Count = (unsigned short)F0688_IMG3_GetPixelCount();
        } else {
                L3003_ui_Count = 1;
        }
        IMG3_Compat_WriteRunPadded(L3002_uc_Kind, L3004_c_Color, L3003_ui_Count, P3001_ui_VisibleWidth, P3002_ui_LineStride, P3003_pui_DestinationOffset, P3004_pui_RemainingInLine);
        return L3003_ui_Count;
}

void IMG3_Compat_ExpandCommandsSameStride(
const unsigned char* P3000_puc_LocalPalette SEPARATOR
unsigned short       P3001_ui_LineStride     SEPARATOR
unsigned short       P3002_ui_TotalPixelCount SEPARATOR
unsigned short*      P3003_pui_DestinationOffset FINAL_SEPARATOR
{
        unsigned short L3001_ui_StartOffset;


        L3001_ui_StartOffset = *P3003_pui_DestinationOffset;
        while ((unsigned short)(*P3003_pui_DestinationOffset - L3001_ui_StartOffset) < P3002_ui_TotalPixelCount) {
                IMG3_Compat_ExpandOneCommandSameStride(P3000_puc_LocalPalette, P3001_ui_LineStride, P3003_pui_DestinationOffset);
        }
}

void IMG3_Compat_ExpandCommandsPadded(
const unsigned char* P3000_puc_LocalPalette SEPARATOR
unsigned short       P3001_ui_VisibleWidth   SEPARATOR
unsigned short       P3002_ui_LineStride     SEPARATOR
unsigned short       P3003_ui_TotalPixelCount SEPARATOR
unsigned short*      P3004_pui_DestinationOffset SEPARATOR
unsigned short*      P3005_pui_RemainingInLine FINAL_SEPARATOR
{
        unsigned short L3001_ui_TotalDone;
        unsigned short L3002_ui_Consumed;


        L3001_ui_TotalDone = 0;
        while (L3001_ui_TotalDone < P3003_ui_TotalPixelCount) {
                L3002_ui_Consumed = IMG3_Compat_ExpandOneCommandPadded(P3000_puc_LocalPalette, P3001_ui_VisibleWidth, P3002_ui_LineStride, P3004_pui_DestinationOffset, P3005_pui_RemainingInLine);
                L3001_ui_TotalDone += L3002_ui_Consumed;
        }
}

void IMG3_Compat_ExpandFromSource(
const unsigned char* P3000_puc_BitmapSource SEPARATOR
unsigned char*       P3001_puc_BitmapDestination FINAL_SEPARATOR
{
        unsigned short L3001_ui_VisibleWidth;
        unsigned short L3002_ui_Height;
        unsigned short L3003_ui_LineStride;
        unsigned short L3004_ui_TotalVisiblePixels;
        unsigned short L3005_ui_DestinationOffset;
        unsigned short L3006_ui_RemainingInLine;
        unsigned char  L3007_auc_LocalPalette[6];
        int            L3008_i_Index;


        G2159_puc_Bitmap_Source = (unsigned char*)P3000_puc_BitmapSource;
        G2160_puc_Bitmap_Destination = P3001_puc_BitmapDestination;
        L3001_ui_VisibleWidth = (unsigned short)(P3000_puc_BitmapSource[0] | (P3000_puc_BitmapSource[1] << 8));
        L3002_ui_Height = (unsigned short)(P3000_puc_BitmapSource[2] | (P3000_puc_BitmapSource[3] << 8));
        L3003_ui_LineStride = (unsigned short)((L3001_ui_VisibleWidth + 1) & ~1);
        L3004_ui_TotalVisiblePixels = (unsigned short)(L3001_ui_VisibleWidth * L3002_ui_Height);
        G2157_ = 8;
        for (L3008_i_Index = 0; L3008_i_Index < 6; L3008_i_Index++) {
                L3007_auc_LocalPalette[L3008_i_Index] = F0687_IMG3_GetNibble();
        }
        L3005_ui_DestinationOffset = 0;
        if (L3001_ui_VisibleWidth == L3003_ui_LineStride) {
                IMG3_Compat_ExpandCommandsSameStride(L3007_auc_LocalPalette, L3003_ui_LineStride, L3004_ui_TotalVisiblePixels, &L3005_ui_DestinationOffset);
        } else {
                L3006_ui_RemainingInLine = L3001_ui_VisibleWidth;
                IMG3_Compat_ExpandCommandsPadded(L3007_auc_LocalPalette, L3001_ui_VisibleWidth, L3003_ui_LineStride, L3004_ui_TotalVisiblePixels, &L3005_ui_DestinationOffset, &L3006_ui_RemainingInLine);
        }
}
