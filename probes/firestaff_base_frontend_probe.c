#define EXEID 84
#define __TURBOC__ 1
#define X463_I34E_I34M 1
#define X465_I34E_I34M 1
#define X736_I34M 1
#include "COMPILE.H"
#ifdef G0490_ac_Graphic560_ActionNames
#undef G0490_ac_Graphic560_ActionNames
#endif
void F0007_MAIN_CopyBytes(char* a, char* b, long c) {}
void F0008_MAIN_ClearBytes(char* a, unsigned long b) {}

/* BEGIN BASE.C frontend-only */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
unsigned char* G0296_puc_Bitmap_Viewport;
BOOLEAN G0297_B_DrawFloorAndCeilingRequested = C1_TRUE;
unsigned char G2040_auc_MemoryFlags[6]; /* Flags drawn when "mem" typed in game. First flag is G0661_B_LargeHeapMemory in Atari ST version */
BOOLEAN G0298_B_NewGame = C001_MODE_LOAD_DUNGEON;
unsigned int16_t G0299_ui_CandidateChampionOrdinal;
BOOLEAN G0300_B_PartyIsResting;
BOOLEAN G0301_B_GameTimeTicking;
BOOLEAN G0302_B_GameWon;
BOOLEAN G0303_B_PartyDead;
int16_t G0304_i_DungeonViewPaletteIndex;
unsigned int16_t G0305_ui_PartyChampionCount;
int16_t G0306_i_PartyMapX;
int16_t G0307_i_PartyMapY;
int16_t G0308_i_PartyDirection;
int16_t G0309_i_PartyMapIndex;
int16_t G0310_i_DisabledMovementTicks;
int16_t G0311_i_ProjectileDisabledMovementTicks;
unsigned int16_t G0312_i_LastProjectileDisabledMovementDirection;
unsigned long G0313_ul_GameTime;
int16_t G0317_i_WaitForInputVerticalBlankCount;
int16_t G0318_i_WaitForInputMaximumVerticalBlankCount;
unsigned long G0319_ul_LoadGameTime;
BOOLEAN G2024_B_PendingMusicOn = C0_FALSE;
int16_t G2106_B_ShowMusicState = C0_FALSE;
BOOLEAN G0321_B_StopWaitingForPlayerInput = C1_TRUE;
BOOLEAN G0325_B_SetMousePointerToObjectInMainLoop;
BOOLEAN G0326_B_RefreshMousePointerInMainLoop;
int16_t G0327_i_NewPartyMapIndex = CM1_MAP_INDEX_NONE;
BOOLEAN G0331_B_PressingEye;
BOOLEAN G0333_B_PressingMouth;
int16_t G2152_B_PressingClosedImaginaryFakeWall;
int16_t G2012_HighlightedZone[4];
BOOLEAN G2150_B_;
BOOLEAN G0341_B_HighlightBoxEnabled;
BOOLEAN G0342_B_RefreshDungeonViewPaletteRequested;
unsigned char* G0343_puc_Graphic_DialogBox;
unsigned long G0349_ul_LastRandomNumber = 0;
int16_t G2199_Unreferenced;
char G2200_Unreferenced[64];
int16_t G2201_Unreferenced;
char G2202_Unreferenced[64];
int16_t G2203_Unreferenced;
BOOLEAN G2151_ExitGameImmediately;






/* stripped for base frontend probe: #include "COPYBYTE.C" */
/* stripped for base frontend probe: #include "CLEARBYT.C" */

void F0009_MAIN_WriteSpacedBytes(
REGISTER char*            P0010_pc_Buffer    SEPARATOR
REGISTER unsigned int16_t P0011_ui_ByteCount SEPARATOR
REGISTER char             P0012_c_ByteValue  SEPARATOR
REGISTER int16_t          P0013_i_Spacing    FINAL_SEPARATOR
{
        int16_t L2159_i_Counter;
        int16_t L2160_i_ByteIndex;


        for (L2160_i_ByteIndex = 0, L2159_i_Counter = 0; L2159_i_Counter < P0011_ui_ByteCount; L2160_i_ByteIndex += P0013_i_Spacing, L2159_i_Counter++) {
                P0010_pc_Buffer[L2160_i_ByteIndex] = P0012_c_ByteValue;
        }
}

void F0010_MAIN_WriteSpacedWords(
REGISTER int16_t*          P0014_pi_Buffer    SEPARATOR
REGISTER unsigned int16_t  P0015_ui_WordCount SEPARATOR
REGISTER int16_t           P0016_i_WordValue  SEPARATOR
REGISTER int16_t           P0017_i_Spacing    FINAL_SEPARATOR
{
        int16_t L2161_i_Counter;
        int16_t L2162_i_WordIndex;


        for (L2162_i_WordIndex = 0, P0017_i_Spacing >>= 1, L2161_i_Counter = 0; L2161_i_Counter < P0015_ui_WordCount; L2162_i_WordIndex += P0017_i_Spacing, L2161_i_Counter++) {
                P0014_pi_Buffer[L2162_i_WordIndex] = P0016_i_WordValue;
        }
}






void F0019_MAIN_DisplayErrorAndStop(
int16_t P0018_i_ErrorNumber FINAL_SEPARATOR
{
        F0047_TEXT_MESSAGEAREA_PrintMessage(C08_COLOR_RED, "\nSYSTEM ERROR ");
        F0048_TEXT_MESSAGEAREA_PrintCharacter(C08_COLOR_RED, (char)((P0018_i_ErrorNumber / 10) + '0'));
        F0048_TEXT_MESSAGEAREA_PrintCharacter(C08_COLOR_RED, (char)((P0018_i_ErrorNumber % 10) + '0'));
        F0540_INPUT_Crawcin();
        F0666_endgame();
}

unsigned int16_t F0653_GetBitmapByteCount(
REGISTER unsigned char* P2231_puc_Bitmap FINAL_SEPARATOR
{
        return M103_BITMAP_BYTE_COUNT(M100_PIXEL_WIDTH(P2231_puc_Bitmap), M101_PIXEL_HEIGHT(P2231_puc_Bitmap));
}

void F0654_Call_F0132_VIDEO_Blit(
REGISTER char* P2232_puc_Bitmap_Source      SEPARATOR
REGISTER char* P2233_puc_Bitmap_Destination SEPARATOR
int16_t*       P2234_pi_XYZ                 SEPARATOR
int16_t        P2235_i_X                    SEPARATOR
int16_t        P2236_i_Y                    SEPARATOR
int16_t        P2237_i_TransparentColor     FINAL_SEPARATOR
{
        F0132_VIDEO_Blit(P2232_puc_Bitmap_Source, P2233_puc_Bitmap_Destination, P2234_pi_XYZ, P2235_i_X, P2236_i_Y, M100_PIXEL_WIDTH(P2232_puc_Bitmap_Source), M100_PIXEL_WIDTH(P2233_puc_Bitmap_Destination), P2237_i_TransparentColor, MASK0x0000_NO_FLIP);
}


void F0655_CopyBitmapAndFlip(
unsigned char* P2245_puc_Bitmap_Source      SEPARATOR
unsigned char* P2246_puc_Bitmap_Destination SEPARATOR
int16_t        P2247_i_Flip                 FINAL_SEPARATOR
{
        int16_t L2163_i_Width;
        int16_t L2164_i_Height;
        L2163_i_Width = M100_PIXEL_WIDTH(P2245_puc_Bitmap_Source);
        L2164_i_Height = M101_PIXEL_HEIGHT(P2245_puc_Bitmap_Source);
        F0615_CopyBitmapDimensions(P2245_puc_Bitmap_Source, P2246_puc_Bitmap_Destination);
        F0132_VIDEO_Blit(P2245_puc_Bitmap_Source, P2246_puc_Bitmap_Destination, F0627_GetTemporaryZoneInitializedFromDimensions(L2163_i_Width, L2164_i_Height), 0, 0, L2163_i_Width, L2163_i_Width, CM1_COLOR_NO_TRANSPARENCY, P2247_i_Flip);
}

void F0020_MAIN_BlitToViewport(
REGISTER unsigned char* P0019_puc_Bitmap         SEPARATOR
int16_t*                P2248_pi_XYZ             SEPARATOR
int16_t                 P0022_i_TransparentColor FINAL_SEPARATOR
{
        F0132_VIDEO_Blit(P0019_puc_Bitmap, G0296_puc_Bitmap_Viewport, P2248_pi_XYZ, 0, 0, M100_PIXEL_WIDTH(P0019_puc_Bitmap), G2073_C224_ViewportPixelWidth, P0022_i_TransparentColor, MASK0x0000_NO_FLIP);
}

void F0656_BlitBitmapToViewportZoneIndexWithTransparency(
unsigned char* P2249_puc_Bitmap         SEPARATOR
int16_t        P2250_i_ZoneIndex        SEPARATOR
int16_t        P2251_i_TransparentColor FINAL_SEPARATOR
{
        int16_t L2166_i_X;
        int16_t L2167_i_Y;
        int16_t L2168_ai_XYZ[4];


        L2166_i_X = 0;
        L2167_i_Y = 0;
        if (F0635_(P2249_puc_Bitmap, L2168_ai_XYZ, P2250_i_ZoneIndex, &L2166_i_X, &L2167_i_Y)) {
                F0132_VIDEO_Blit(P2249_puc_Bitmap, G0296_puc_Bitmap_Viewport, L2168_ai_XYZ, L2166_i_X, L2167_i_Y, M100_PIXEL_WIDTH(P2249_puc_Bitmap), G2073_C224_ViewportPixelWidth, P2251_i_TransparentColor, MASK0x0000_NO_FLIP);
        }
}

void F0657_BlitBitmapIndexToViewportZoneWithTransparency(
int16_t  P2252_i_BitmapIndex      SEPARATOR
int16_t* P2253_pi_XYZ             SEPARATOR
int16_t  P2254_i_TransparentColor FINAL_SEPARATOR
{
        REGISTER unsigned char* L2169_puc_Bitmap;
        STRUCT2 L2170_s_Struct2;


        L2169_puc_Bitmap = F0630_InitBitmapStruct2(P2252_i_BitmapIndex, &L2170_s_Struct2);
        F0132_VIDEO_Blit(L2169_puc_Bitmap, G0296_puc_Bitmap_Viewport, P2253_pi_XYZ, L2170_s_Struct2.X, L2170_s_Struct2.Y, M100_PIXEL_WIDTH(L2169_puc_Bitmap), G2073_C224_ViewportPixelWidth, P2254_i_TransparentColor, MASK0x0000_NO_FLIP);
}

void F0658_BlitBitmapIndexToZoneIndexWithTransparency(
int16_t P2255_i_BitmapIndex      SEPARATOR
int16_t P2256_i_ZoneIndex        SEPARATOR
int16_t P2257_i_TransparentColor FINAL_SEPARATOR
{
        REGISTER unsigned char* L2171_puc_Bitmap;
        int16_t L2172_i_Width;
        int16_t L2173_i_Height;
        int16_t L2174_ai_XYZ[4];
        STRUCT2 L2175_s_Struct2;


        L2171_puc_Bitmap = F0630_InitBitmapStruct2(P2255_i_BitmapIndex, &L2175_s_Struct2);
        if (P2255_i_BitmapIndex < 0) {
                L2172_i_Width = L2175_s_Struct2.Width;
                L2173_i_Height = L2175_s_Struct2.Height;
        } else {
                L2172_i_Width = 0;
                L2173_i_Height = 0;
        }
        if (F0635_(L2171_puc_Bitmap, L2174_ai_XYZ, P2256_i_ZoneIndex, &L2172_i_Width, &L2173_i_Height)) {
                F0132_VIDEO_Blit(L2171_puc_Bitmap, G0296_puc_Bitmap_Viewport, L2174_ai_XYZ, L2175_s_Struct2.X + L2172_i_Width, L2175_s_Struct2.Y + L2173_i_Height, M100_PIXEL_WIDTH(L2171_puc_Bitmap), G2073_C224_ViewportPixelWidth, P2257_i_TransparentColor, MASK0x0000_NO_FLIP);
        }
}

void F0766_BlitToScreen(
unsigned char* P2258_puc_Bitmap         SEPARATOR
int16_t*       P2259_pi_XYZ             SEPARATOR
int16_t        P2260_i_TransparentColor FINAL_SEPARATOR
{
        F0132_VIDEO_Blit(P2258_puc_Bitmap, G0348_Bitmap_Screen, P2259_pi_XYZ, 0, 0, M100_PIXEL_WIDTH(P2258_puc_Bitmap), G2071_C320_ScreenPixelWidth, P2260_i_TransparentColor, MASK0x0000_NO_FLIP);
}

void F0021_MAIN_BlitToScreen(
unsigned char* P0023_puc_Bitmap         SEPARATOR
int16_t        P2261_i_ZoneIndex        SEPARATOR
int16_t        P0026_i_TransparentColor FINAL_SEPARATOR
{
        int16_t L2176_i_Width;
        int16_t L2177_i_Height;
        int16_t L2178_ai_XYZ[4];


        L2176_i_Width = 0;
        L2177_i_Height = 0;
        if (F0635_(P0023_puc_Bitmap, L2178_ai_XYZ, P2261_i_ZoneIndex, &L2176_i_Width, &L2177_i_Height)) {
                F0132_VIDEO_Blit(P0023_puc_Bitmap, G0348_Bitmap_Screen, L2178_ai_XYZ, L2176_i_Width, L2177_i_Height, M100_PIXEL_WIDTH(P0023_puc_Bitmap), G2071_C320_ScreenPixelWidth, P0026_i_TransparentColor, MASK0x0000_NO_FLIP);
        }
}

void F0659_(
int16_t  P2262_i_P1    SEPARATOR
int16_t* P2263_pi_XYZ  SEPARATOR
int16_t  P2264_i_Color FINAL_SEPARATOR
{
        REGISTER unsigned char* L2179_puc_Bitmap;
        STRUCT2 L2180_s_Struct2;


        L2179_puc_Bitmap = F0630_InitBitmapStruct2(P2262_i_P1, &L2180_s_Struct2);
        F0132_VIDEO_Blit(L2179_puc_Bitmap, G0348_Bitmap_Screen, P2263_pi_XYZ, L2180_s_Struct2.X, L2180_s_Struct2.Y, M100_PIXEL_WIDTH(L2179_puc_Bitmap), G2071_C320_ScreenPixelWidth, P2264_i_Color, MASK0x0000_NO_FLIP);
}

void F0660_(
int16_t P2265_i_BitmapIndex      SEPARATOR
int16_t P2266_i_ZoneIndex        SEPARATOR
int16_t P2267_i_TransparentColor FINAL_SEPARATOR
{
        REGISTER unsigned char* L2181_puc_Bitmap;
        int16_t L2182_i_Width;
        int16_t L2183_i_Height;
        int16_t L2184_ai_XYZ[4];
        STRUCT2 L2185_s_Struct2;


        L2181_puc_Bitmap = F0630_InitBitmapStruct2(P2265_i_BitmapIndex, &L2185_s_Struct2);
        if (P2265_i_BitmapIndex < 0) {
                L2182_i_Width = L2185_s_Struct2.Width;
                L2183_i_Height = L2185_s_Struct2.Height;
        } else {
                L2182_i_Width = 0;
                L2183_i_Height = 0;
        }
        if (F0635_(L2181_puc_Bitmap, L2184_ai_XYZ, P2266_i_ZoneIndex, &L2182_i_Width, &L2183_i_Height)) {
                F0132_VIDEO_Blit(L2181_puc_Bitmap, G0348_Bitmap_Screen, L2184_ai_XYZ, L2185_s_Struct2.X + L2182_i_Width, L2185_s_Struct2.Y + L2183_i_Height, M100_PIXEL_WIDTH(L2181_puc_Bitmap), G2071_C320_ScreenPixelWidth, P2267_i_TransparentColor, MASK0x0000_NO_FLIP);
        }
}

unsigned char* F0767_(
int16_t          P2268_i_NativeBitmapIndex  SEPARATOR
REGISTER int16_t P2269_i_DerivedBitmapIndex SEPARATOR
int16_t          P2270_i_Width              SEPARATOR
int16_t          P2271_i_Height             SEPARATOR
unsigned char*   P2272_puc_PaletteChanges   FINAL_SEPARATOR
{
        REGISTER unsigned char* L2186_puc_DerivedBitmap;
        REGISTER unsigned char* L2187_puc_NativeBitmap;


        L2187_puc_NativeBitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(P2268_i_NativeBitmapIndex);
        L2186_puc_DerivedBitmap = F0492_CACHE_GetDerivedBitmap(P2269_i_DerivedBitmapIndex);
        F0129_VIDEO_BlitShrinkWithPaletteChanges(L2187_puc_NativeBitmap, L2186_puc_DerivedBitmap, M100_PIXEL_WIDTH(L2187_puc_NativeBitmap), M101_PIXEL_HEIGHT(L2187_puc_NativeBitmap), M100_PIXEL_WIDTH(L2186_puc_DerivedBitmap) = P2270_i_Width, M101_PIXEL_HEIGHT(L2186_puc_DerivedBitmap) = P2271_i_Height, P2272_puc_PaletteChanges);
        return L2186_puc_DerivedBitmap;
}

unsigned char* F0661_GetShrinkedBitmap(
int16_t          P2273_i_NativeBitmapIndex  SEPARATOR
REGISTER int16_t P2274_i_DerivedBitmapIndex SEPARATOR
int16_t          P2275_i_Width              SEPARATOR
int16_t          P2276_i_Height             SEPARATOR
unsigned char*   P2277_puc_PaletteChanges   FINAL_SEPARATOR
{
        REGISTER unsigned char* L2188_puc_DerivedBitmap;
        REGISTER unsigned char* L2189_puc_NativeBitmap;


        if (F0491_CACHE_IsDerivedBitmapInCache(P2274_i_DerivedBitmapIndex)) {
                L2188_puc_DerivedBitmap = F0492_CACHE_GetDerivedBitmap(P2274_i_DerivedBitmapIndex);
        } else {
                L2189_puc_NativeBitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(P2273_i_NativeBitmapIndex);
                L2188_puc_DerivedBitmap = F0492_CACHE_GetDerivedBitmap(P2274_i_DerivedBitmapIndex);
                F0129_VIDEO_BlitShrinkWithPaletteChanges(L2189_puc_NativeBitmap, L2188_puc_DerivedBitmap, M100_PIXEL_WIDTH(L2189_puc_NativeBitmap), M101_PIXEL_HEIGHT(L2189_puc_NativeBitmap), M100_PIXEL_WIDTH(L2188_puc_DerivedBitmap) = P2275_i_Width, M101_PIXEL_HEIGHT(L2188_puc_DerivedBitmap) = P2276_i_Height, P2277_puc_PaletteChanges);
                F0493_CACHE_AddDerivedBitmap(P2274_i_DerivedBitmapIndex);
        }
        return L2188_puc_DerivedBitmap;
}

void F0662_ApplyPaletteChanges(
REGISTER char* P2278_pc_ChampionIconBitmap SEPARATOR
unsigned char* P2279_puc_PaletteChanges    FINAL_SEPARATOR
{
        F0129_VIDEO_BlitShrinkWithPaletteChanges(M774_CAST_PUC(P2278_pc_ChampionIconBitmap), M774_CAST_PUC(P2278_pc_ChampionIconBitmap), M100_PIXEL_WIDTH(P2278_pc_ChampionIconBitmap), M101_PIXEL_HEIGHT(P2278_pc_ChampionIconBitmap), M100_PIXEL_WIDTH(P2278_pc_ChampionIconBitmap), M101_PIXEL_HEIGHT(P2278_pc_ChampionIconBitmap), P2279_puc_PaletteChanges);
}

void F0663_CopyBitmapWithPaletteChanges(
REGISTER unsigned char* P2280_puc_Bitmap         SEPARATOR
REGISTER unsigned char* P2281_puc_Bitmap2        SEPARATOR
unsigned char*          P2282_puc_PaletteChanges FINAL_SEPARATOR
{
        REGISTER int16_t L2190_i_Width;
        REGISTER int16_t L2191_i_Height;


        L2190_i_Width = M100_PIXEL_WIDTH(P2280_puc_Bitmap);
        L2191_i_Height = M101_PIXEL_HEIGHT(P2280_puc_Bitmap);
        F0129_VIDEO_BlitShrinkWithPaletteChanges(P2280_puc_Bitmap, P2281_puc_Bitmap2, L2190_i_Width, L2191_i_Height, M100_PIXEL_WIDTH(P2281_puc_Bitmap2) = L2190_i_Width, M101_PIXEL_HEIGHT(P2281_puc_Bitmap2) = L2191_i_Height, P2282_puc_PaletteChanges);
}


void F0022_MAIN_Delay(
unsigned int16_t P0027_ui_VerticalBlankCount FINAL_SEPARATOR
{
        while (P0027_ui_VerticalBlankCount--) {
                M526_WaitVerticalBlank();
        }
}

int16_t F0023_MAIN_GetAbsoluteValue(
REGISTER int16_t P0028_i_Value FINAL_SEPARATOR
{
        return (P0028_i_Value < 0) ? -P0028_i_Value : P0028_i_Value;
}

int16_t F0024_MAIN_GetMinimumValue(
int16_t P0029_i_Value1 SEPARATOR
int16_t P0030_i_Value2 FINAL_SEPARATOR
{
        return (P0029_i_Value1 < P0030_i_Value2) ? P0029_i_Value1 : P0030_i_Value2;
}

int16_t F0025_MAIN_GetMaximumValue(
int16_t P0031_i_Value1 SEPARATOR
int16_t P0032_i_Value2 FINAL_SEPARATOR
{
        return (P0031_i_Value1 > P0032_i_Value2) ? P0031_i_Value1 : P0032_i_Value2;
}

int16_t F0026_MAIN_GetBoundedValue(
int16_t P0033_i_MinimumAllowedValue SEPARATOR
int16_t P0034_i_Value               SEPARATOR
int16_t P0035_i_MaximumAllowedValue FINAL_SEPARATOR
{
        return (P0034_i_Value < P0033_i_MinimumAllowedValue) ? P0033_i_MinimumAllowedValue : (P0034_i_Value > P0035_i_MaximumAllowedValue) ? P0035_i_MaximumAllowedValue : P0034_i_Value;
}

unsigned int16_t F0027_MAIN_Get16bitRandomNumber(
void
)
{
        return (unsigned int16_t)((G0349_ul_LastRandomNumber = G0349_ul_LastRandomNumber * 0xBB40E62D + 11) >> 8); /* Pseudorandom number generator. 0xBB40E62D = 3141592621 */
}

unsigned int16_t F0028_MAIN_Get1BitRandomNumber(
void
)
{
        return (unsigned int16_t)((G0349_ul_LastRandomNumber = G0349_ul_LastRandomNumber * 0xBB40E62D + 11) >> 8) & 0x0001; /* Pseudorandom number generator. 0xBB40E62D = 3141592621 */
}

unsigned int16_t F0029_MAIN_Get2BitRandomNumber(
void
)
{
        return (unsigned int16_t)((G0349_ul_LastRandomNumber = G0349_ul_LastRandomNumber * 0xBB40E62D + 11) >> 8) & 0x0003; /* Pseudorandom number generator. 0xBB40E62D = 3141592621 */
}

unsigned int16_t F0030_MAIN_GetScaledProduct(
unsigned int16_t P0036_ui_Value1 SEPARATOR
unsigned int16_t P0037_ui_Scale  SEPARATOR
unsigned int16_t P0038_ui_Value2 FINAL_SEPARATOR
{
        return ((long)P0036_ui_Value1 * P0038_ui_Value2) >> P0037_ui_Scale;
}
/* END BASE.C frontend-only */
