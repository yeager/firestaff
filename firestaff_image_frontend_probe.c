#define EXEID 84
#define __TURBOC__ 1
#define X736_I34M 1
#include "COMPILE.H"
#ifdef G0490_ac_Graphic560_ActionNames
#undef G0490_ac_Graphic560_ActionNames
#endif

/* BEGIN IMAGE.C (frontend only) */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
unsigned char* G0348_Bitmap_Screen = NULL;
char* G2059_pc_Unused = NULL;
char* G2060_pc_Unused = NULL;
int16_t G2123_ = -1;
int16_t G2061_DungeonViewPaletteIndices[6] = { 0, 1, 2, 3, 4, 5 };

VIDEO_DRIVER* G2156_VideoDriver;
unsigned int16_t G2157_;
char G2208_Unreferenced[4];
unsigned char G2158_auc_Bitmap_PixelLine[160];
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;


/* stripped for frontend probe: #include "IMAGE3.C" */
/* stripped for frontend probe: #include "FILLBOX.C" */
/* stripped for frontend probe: #include "BLIT.C" */
/* stripped for frontend probe: #include "EXPAND.C" */


void F0693_WaitVerticalBlank(
)
{
        (*(G2156_VideoDriver->VIDRV_07_WaitVerticalBlank))();
}


void F0694_SetMultipleColorsInPalette(int16_t P2368_i_PaletteIndex)
{
        (*(G2156_VideoDriver->VIDRV_08_SetMultipleColorsInPalette))(P2368_i_PaletteIndex);
}

void F1012_PALETTE_SetCurtain(unsigned int16_t P2369_ui_)
{
        (*(G2156_VideoDriver->VIDRV_11_PALETTE_SetCurtain))((unsigned char)P2369_ui_);
}

void F0695_SetCreatureReplacementColors(int P2370_i_ReplacedColor, int P2371_i_ReplacementColorSetIndex)
{
        (*(G2156_VideoDriver->VIDRV_12_SetCreatureReplacementColors))(P2370_i_ReplacedColor, P2371_i_ReplacementColorSetIndex);
        G2123_ = -1;
}

/* stripped for frontend probe: #include "DRAWVIEW.C" */
/* stripped for frontend probe: #include "DRAWMSGA.C" */

void F0697_HatchScreenBox(int16_t* P2372_pi_XYZ, unsigned int16_t P2373_ui_Color)
{
        int16_t L2780_i_X1;
        int16_t L2781_i_X2;
        int16_t L2782_i_Y1;
        int16_t L2783_i_Y2;


        L2780_i_X1 = M704_ZONE_LEFT(P2372_pi_XYZ);
        L2781_i_X2 = M705_ZONE_RIGHT(P2372_pi_XYZ);
        L2782_i_Y1 = M706_ZONE_TOP(P2372_pi_XYZ);
        L2783_i_Y2 = M707_ZONE_BOTTOM(P2372_pi_XYZ);
        (*(G2156_VideoDriver->VIDRV_06_HatchScreenBox))(L2780_i_X1, L2781_i_X2, L2782_i_Y1, L2783_i_Y2);
}

void F0698_InvertBox(int16_t* P2374_pi_XYZ)
{
        int16_t L2785_i_X1;
        int16_t L2786_i_X2;
        int16_t L2787_i_Y1;
        int16_t L2788_i_Y2;


        L2785_i_X1 = M704_ZONE_LEFT(P2374_pi_XYZ);
        L2786_i_X2 = M705_ZONE_RIGHT(P2374_pi_XYZ);
        L2787_i_Y1 = M706_ZONE_TOP(P2374_pi_XYZ);
        L2788_i_Y2 = M707_ZONE_BOTTOM(P2374_pi_XYZ);
        (*(G2156_VideoDriver->VIDRV_05_InvertBox))(L2785_i_X1, L2786_i_X2, L2787_i_Y1, L2788_i_Y2);
}

void F0699_InitVideoInterrupt(
)
{
        G2156_VideoDriver = (VIDEO_DRIVER*)getvect(C255_DM_VIDEO_INTERRUPT);
        (*(G2156_VideoDriver->VIDRV_13_InitializeUnusedGlobalVariables))(&G2059_pc_Unused, &G2060_pc_Unused);
}
/* END IMAGE.C */
