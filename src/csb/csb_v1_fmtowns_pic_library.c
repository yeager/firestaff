#include "csb_v1_fmtowns_pic_library.h"
#include <string.h>

int csb_v1_fmtowns_pic_library_open_ext_v1_pc34(
        const uint8_t *data, size_t data_size,
        dm1_v1_fmtowns_pic_library_view_t *out_view) {
    if (!data || !out_view) return 0;
    if (data_size < 4) return 0;
    /* Verify sig 0x8001. */
    uint16_t sig = (uint16_t)(data[0] | (data[1] << 8));
    if (sig != 0x8001u) return 0;
    /* Skip 2-byte sig prefix; hand the rest to DM1 pic_library
     * parser. DM1's parser expects word0 = count. */
    dm1_v1_fmtowns_pic_library_status_t st =
        dm1_v1_fmtowns_pic_library_open_pc34(
            data + 2, data_size - 2, out_view);
    return (st == DM1_V1_FMTOWNS_PIC_LIB_OK) ? 1 : 0;
}
