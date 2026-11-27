/* Authenticated Mac pointer-owner source gate.
 *
 * This is deliberately a resource-fork gate, not a pointer-coordinate
 * fixture. CODE(3)/CODE(11) build dynamic controls at runtime; Firestaff
 * must not turn their disassembly into guessed static rectangles.
 */

#include "dm2_v1_mac_media.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int anchor(const uint8_t *bytes, size_t size, size_t offset,
                  uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return bytes && offset + 4u <= size && bytes[offset] == a &&
           bytes[offset + 1u] == b && bytes[offset + 2u] == c &&
           bytes[offset + 3u] == d;
}

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    const uint8_t *code3 = NULL;
    const uint8_t *code8 = NULL;
    const uint8_t *code11 = NULL;
    const uint8_t *data0 = NULL;
    size_t code3_size = 0u;
    size_t code8_size = 0u;
    size_t code11_size = 0u;
    size_t data0_size = 0u;
    DM2_V1_MacResourceReceipt receipt;

    if (!zip || !zip[0]) {
        puts("SKIP: FIRESTAFF_DM2_MAC_EN_ZIP is not set");
        return 77;
    }
    memset(&media, 0, sizeof(media));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        !media.application_resource ||
        dm2_v1_mac_resource_find(media.application_resource,
                                  media.application_resource_size,
                                  "CODE", 3, &code3, &code3_size,
                                  &receipt) != 0 ||
        dm2_v1_mac_resource_find(media.application_resource,
                                  media.application_resource_size,
                                  "CODE", 8, &code8, &code8_size,
                                  &receipt) != 0 ||
        dm2_v1_mac_resource_find(media.application_resource,
                                 media.application_resource_size,
                                 "CODE", 11, &code11, &code11_size,
                                 &receipt) != 0 ||
        dm2_v1_mac_resource_find(media.application_resource,
                                 media.application_resource_size,
                                 "DATA", 0, &data0, &data0_size,
                                 &receipt) != 0) {
        fprintf(stderr, "authentic Mac CODE resources unavailable\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    if (code3_size != 29456u || code11_size != 23908u ||
        code8_size != 14578u || data0_size != 19300u ||
        !anchor(code3, code3_size, 0x00c2u, 0x4e, 0x56, 0xff, 0xf8) ||
        !anchor(code3, code3_size, 0x0170u, 0x4e, 0x56, 0xff, 0xfa) ||
        !anchor(code3, code3_size, 0x0358u, 0x4e, 0x56, 0xff, 0xdc) ||
        !anchor(code11, code11_size, 0x0214u, 0x4e, 0x56, 0xff, 0xfc) ||
        !anchor(code11, code11_size, 0x0308u, 0x4e, 0x56, 0xff, 0xf6) ||
        memcmp(code8 + 0x1b46u,
               "\x0c\x47\x00\xe4\x65\x18\x0c\x47\x00\xe9\x62\x12"
               "\x30\x07\x06\x40\xff\x1c\x3f\x00\x4e\xad\x0e\x82"
               "\x54\x8f", 26u) != 0 ||
        memcmp(data0 + 0x3c42u,
               "\x00\xe4\x00\xdd\x00\x02\x00\xe5\x00\xde\x00\x02"
               "\x00\xe6\x00\xdf\x00\x02\x00\xe7\x00\xe0\x00\x02"
               "\x00\xe8\x00\xe1\x00\x02\x00\xe9\x00\xe2\x00\x02",
               36u) != 0) {
        fprintf(stderr, "Mac pointer-owner CODE anchors changed\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    dm2_v1_mac_media_free(&media);
    puts("PASS: authentic Mac CHARSHEET and moneybox pointer owners are locked");
    return 0;
}
