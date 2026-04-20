#ifndef REDMCSB_IMAGE_FRONTEND_PC34_H
#define REDMCSB_IMAGE_FRONTEND_PC34_H

void F0693_WaitVerticalBlank(void);
void F0694_SetMultipleColorsInPalette(int16_t P2368_i_PaletteIndex);
void F1012_PALETTE_SetCurtain(unsigned int16_t P2369_ui_);
void F0695_SetCreatureReplacementColors(int P2370_i_ReplacedColor, int P2371_i_ReplacementColorSetIndex);
void F0697_HatchScreenBox(int16_t* P2372_pi_XYZ, unsigned int16_t P2373_ui_Color);
void F0698_InvertBox(int16_t* P2374_pi_XYZ);
void F0699_InitVideoInterrupt(void);

#endif
