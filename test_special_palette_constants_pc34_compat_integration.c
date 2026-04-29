#include <stdio.h>
#include "special_palette_constants_pc34_compat.h"
int main(void) { unsigned int ok = special_palette_constants_GetInvariant(); printf("probe=firestaff_special_palette_constants\n"); printf("sourceEvidence=%s\n", special_palette_constants_GetEvidence()); printf("specialPaletteConstantsInvariantOk=%u\n", ok); return ok ? 0 : 1; }
