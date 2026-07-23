#include "dm1_v1_f0809_f0811_copypro_media_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int open_real_hdm(const char* path)
{
    FILE* file;
    unsigned char header[16];
    size_t count;

    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    count = fread(header, 1, sizeof(header), file);
    fclose(file);
    return count == sizeof(header);
}

int main(void)
{
    DM1_V1_CopyProtectionCapturePc34 capture;
    DM1_V1_CopyProtectionReceiptPc34 receipt;
    const char* hdm = getenv("FIRESTAFF_DM1_PC98_HDM");

    memset(&capture, 0, sizeof(capture));
    if (dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
            809, &capture, &receipt) || !receipt.suppressSyntheticDiskResponse ||
        receipt.valid) return 1;
    if (dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
            810, &capture, &receipt) || !receipt.suppressSyntheticDiskResponse ||
        receipt.valid) return 1;
    if (dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
            811, &capture, &receipt) || !receipt.suppressSyntheticDiskResponse ||
        receipt.valid) return 1;

    if (!hdm || !hdm[0] || !open_real_hdm(hdm)) {
        puts("SKIP: real PC-98 HDM not installed");
        return 0;
    }
    /* A real HDM sector image is not a raw controller/flux diagnostic. Its
     * deliberately absent protection sector must never be converted into a
    * successful F0809/F0810/F0811 response. */
    capture.mediaKind = DM1_V1_COPYPRO_MEDIA_SECTOR_IMAGE_PC34;
    capture.mediaHashVerified = 1;
    if (dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
            809, &capture, &receipt) || receipt.valid ||
        dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
            810, &capture, &receipt) || receipt.valid ||
        dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
            811, &capture, &receipt) || receipt.valid) return 1;
    puts("ok: real PC-98 HDM is fail-closed without raw CPSX capture");
    return 0;
}
