#include "dm1_v1_original_save_classifier.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *file;
    long length;
    unsigned char *bytes;
    Dm1V1OriginalSavePc34HeaderDecodeReceipt receipt;
    if (argc < 2) { puts("SKIP PC34 header receipt: no save path"); return 0; }
    file = fopen(argv[1], "rb");
    if (!file) { puts("SKIP PC34 header receipt: save unavailable"); return 0; }
    fseek(file, 0, SEEK_END); length = ftell(file); rewind(file);
    if (length < 0) { fclose(file); return 1; }
    bytes = (unsigned char *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) { free(bytes); fclose(file); return 1; }
    fclose(file);
    (void)dm1_v1_original_save_pc34_decode_header_receipt(bytes, (size_t)length, &receipt);
    free(bytes);
    printf("PC34 header size=%d decoded=%d checksum=%d expected=%04x actual=%04x format=%u platform=%u dungeon=%u\n", receipt.header_size_ok, receipt.decoded, receipt.checksum_ok, receipt.expected_checksum, receipt.actual_checksum, receipt.format_id, receipt.platform, receipt.dungeon_id);
    return receipt.decoded ? 0 : 1;
}
