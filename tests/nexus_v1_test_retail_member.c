#include "nexus_v1_test_retail_member.h"

#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_iso_reader.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

uint8_t *nexus_v1_test_read_retail_member(const char *locator,
                                          size_t *out_size,
                                          char out_sha256[65]) {
    const char *separator;
    char *cue_path;
    Nexus_ISOReader reader;
    const Nexus_ISOFile *member;
    uint8_t *bytes = NULL;
    size_t cue_length;

    if (out_size) *out_size = 0U;
    if (out_sha256) out_sha256[0] = '\0';
    if (!locator || !out_size || !out_sha256 ||
        !(separator = strstr(locator, "::")) || !separator[2]) return NULL;
    cue_length = (size_t)(separator - locator);
    if (!cue_length) return NULL;
    cue_path = (char *)malloc(cue_length + 1U);
    if (!cue_path) return NULL;
    memcpy(cue_path, locator, cue_length);
    cue_path[cue_length] = '\0';
    memset(&reader, 0, sizeof(reader));
    if (nexus_iso_open_cue(&reader, cue_path) <= 0 ||
        !nexus_iso_is_nexus(&reader) ||
        !(member = nexus_iso_find(&reader, separator + 2)) ||
        member->size == 0U || member->size > (uint32_t)INT_MAX) goto done;
    bytes = (uint8_t *)malloc((size_t)member->size);
    if (!bytes || nexus_iso_read_file(&reader, member, bytes,
                                      (int)member->size) != (int)member->size ||
        firestaff_x68k_media_receipt_sha256_hex(
            bytes, (size_t)member->size, out_sha256, 65U) != 0) {
        free(bytes);
        bytes = NULL;
        goto done;
    }
    if ((strcmp(separator + 2, "TITLE.BIN") == 0 &&
         strcmp(out_sha256, "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3") != 0 &&
         strcmp(out_sha256, "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44") != 0) ||
        (strcmp(separator + 2, "WARNING.BIN") == 0 &&
         strcmp(out_sha256, "8783fa9defda0a358d0474da56480d476b5511c8ca6d3eb61fe097c5697d44ab") != 0) ||
        (strcmp(separator + 2, "FONT256.S2D") == 0 &&
         strcmp(out_sha256, "b820d606b4de4fbaa21d4e32f1df56b4cce6898939fb04f73cb6f55f4ebd13af") != 0 &&
         strcmp(out_sha256, "764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb") != 0) ||
        (strcmp(separator + 2, "STABG.BIN") == 0 &&
         strcmp(out_sha256, "7b8e44ffd1249175da1c407993b983a26bc180204e63f9b69274014b336c6913") != 0)) {
        free(bytes);
        bytes = NULL;
        out_sha256[0] = '\0';
        goto done;
    }
    *out_size = (size_t)member->size;
done:
    nexus_iso_close(&reader);
    free(cue_path);
    return bytes;
}
