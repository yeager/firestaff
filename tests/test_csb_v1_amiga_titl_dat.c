/* Real-media verification for the Amiga TITL.DAT title timeline.
 * Set FIRESTAFF_CSB_AMIGA_TITL to an extracted TITL.DAT. */

#include "csb_v1_amiga_titl_dat.h"

#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *data;

    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return -1;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return -1;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)length;
    return 0;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_AMIGA_TITL");
    CSB_V1_AmigaTitlPalette palette;
    CSB_V1_AmigaTitlDeltaReceipt delta;
    CSB_V1_AmigaTitlFrameReceipt frame;
    CSB_V1_AmigaTitlSchedule schedule;
    uint8_t *data;
    uint8_t *pixels;
    size_t size;
    uint16_t delta_index;
    uint64_t frame_hash = UINT64_C(1469598103934665603);
    size_t pixel_index;

    if (!path || !*path) {
        puts("skip: FIRESTAFF_CSB_AMIGA_TITL is not set");
        return 77;
    }
    if (read_file(path, &data, &size) != 0 ||
        csb_v1_amiga_titl_dat_decode(data, size, &schedule) != 0 ||
        csb_v1_amiga_titl_dat_decode_palette(data, size, &palette) != 0) {
        free(data);
        fputs("FAIL: cannot decode real Amiga TITL.DAT\n", stderr);
        return 1;
    }
    if (schedule.width != 320u || schedule.height != 200u ||
        schedule.bit_depth != 4u || schedule.delta_count != 31u ||
        schedule.initial_duration_vbl != 126u ||
        schedule.delta_durations_vbl[9] != 18u ||
        schedule.delta_durations_vbl[25] != 12u ||
        schedule.delta_durations_vbl[30] != 282u ||
        schedule.total_duration_vbl != 606u) {
        fputs("FAIL: unexpected Amiga TITL.DAT schedule\n", stderr);
        return 1;
    }
    if (palette.color_count != 16u || palette.rgb4[0][0] != 0u ||
        palette.rgb4[0][1] != 0u || palette.rgb4[0][2] != 0u ||
        palette.rgb4[1][0] != 15u || palette.rgb4[1][1] != 0u ||
        palette.rgb4[1][2] != 0u) {
        fputs("FAIL: unexpected Amiga TITL.DAT palette\n", stderr);
        return 1;
    }
    pixels = (uint8_t *)malloc(320u * 200u);
    if (!pixels || !csb_v1_amiga_titl_dat_decode_initial_frame(
            data, size, pixels, 320u * 200u, &frame) ||
        frame.width != 320u || frame.height != 200u ||
        frame.decoded_pixel_count != 320u * 200u ||
        frame.source_bytes_consumed != 254u ||
        pixels[89u * 320u + 186u] != 7u) {
        free(pixels);
        free(data);
        fputs("FAIL: cannot decode real Amiga TITL.DAT EN frame\n", stderr);
        return 1;
    }
    /* The first thirty DL records have complete source-backed command streams.
     * The final record's source read continues beyond the on-disk FTL item and
     * deliberately remains fail-closed until that allocation boundary is
     * independently proven. ReDMCSB ANIM.C F1205 / EXPAND.C F0466. */
    for (delta_index = 0u; delta_index < 30u; ++delta_index) {
        if (!csb_v1_amiga_titl_dat_apply_delta(
                data, size, delta_index, pixels, 320u * 200u, &delta) ||
            delta.width != 320u || delta.height != 200u ||
            delta.delta_index != delta_index || delta.duration_vbl == 0u ||
            delta.decoded_pixel_count != 320u * 200u) {
            free(pixels);
            free(data);
            fputs("FAIL: cannot apply real Amiga TITL.DAT DL frame\n", stderr);
            return 1;
        }
    }
    for (pixel_index = 0u; pixel_index < 320u * 200u; ++pixel_index) {
        frame_hash ^= pixels[pixel_index];
        frame_hash *= UINT64_C(1099511628211);
    }
    if (pixels[0] != 12u || pixels[28666u] != 11u ||
        pixels[100u * 320u + 160u] != 1u ||
        frame_hash != UINT64_C(0xeed602e590f79a0e)) {
        free(pixels);
        free(data);
        fputs("FAIL: unexpected real Amiga TITL.DAT DL pixels\n", stderr);
        return 1;
    }
    free(pixels);
    free(data);
    puts("ok: real Amiga TITL.DAT has 32 title frames over 606 VBL");
    return 0;
}
