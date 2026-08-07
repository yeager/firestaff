#include "dm1v2/dm1_v2_filters.h"
#include "vga_palette_pc34_compat.h"

#include <string.h>

int dm1_v2_filter_palette_build_lut(int gamma100,
                                    int brightness,
                                    int contrast,
                                    unsigned char out_lut[DM1_V2_PALETTE_LEVELS][16][3]) {
    (void)gamma100;
    (void)brightness;
    (void)contrast;
    if (!out_lut) return -1;
    /* Preserve the exact authenticated VGA table. Gamma, brightness and
     * contrast compensation are host presentation inventions, not PC34 data. */
    memcpy(out_lut, G9010_auc_VgaPaletteAll_Compat,
           sizeof(G9010_auc_VgaPaletteAll_Compat));
    return 0;
}
