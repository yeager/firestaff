/* Authentic Macintosh application Resource Manager type census.
 *
 * This is a source census, not a UI fixture.  It records the types present
 * in the retained application resource fork so native controls are only
 * admitted when their original resource/code owner is known.
 */

#include "dm2_v1_mac_media.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    int count, i;
    int saw_code = 0, saw_menu = 0, saw_ditl = 0;

    if (!zip || !zip[0]) {
        puts("SKIP: FIRESTAFF_DM2_MAC_EN_ZIP is not set");
        return 77;
    }
    memset(&media, 0, sizeof(media));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        !media.application_resource) {
        fprintf(stderr, "authentic Mac application resource fork unavailable\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    count = dm2_v1_mac_resource_type_count(media.application_resource,
                                            media.application_resource_size);
    if (count <= 0) {
        fprintf(stderr, "authentic Mac resource type list unavailable\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    for (i = 0; i < count; ++i) {
        DM2_V1_MacResourceTypeReceipt receipt;
        if (dm2_v1_mac_resource_type_at(media.application_resource,
                                        media.application_resource_size, i,
                                        &receipt) != 0) {
            fprintf(stderr, "resource type %d unavailable\n", i);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        if (memcmp(receipt.type, "CODE", 4u) == 0) saw_code = 1;
        if (memcmp(receipt.type, "MENU", 4u) == 0) saw_menu = 1;
        if (memcmp(receipt.type, "DITL", 4u) == 0) saw_ditl = 1;
    }
    {
        const uint8_t *data = NULL;
        size_t size = 0u;
        DM2_V1_MacResourceReceipt receipt;
        if (dm2_v1_mac_resource_find(media.application_resource,
                                      media.application_resource_size,
                                      "CNTL", 130, &data, &size,
                                      &receipt) != 0 || size != 32u ||
            data[22] != 9u || memcmp(data + 23u, "7,6,30,11", 9u) != 0 ||
            dm2_v1_mac_resource_find(media.application_resource,
                                     media.application_resource_size,
                                     "CNTL", 131, &data, &size,
                                     &receipt) != 0 || size != 32u ||
            data[22] != 9u || memcmp(data + 23u, "7,6,30,11", 9u) != 0) {
            fprintf(stderr, "authentic Macintosh CNTL resources changed\n");
            dm2_v1_mac_media_free(&media);
            return 1;
        }
    }
    dm2_v1_mac_media_free(&media);
    if (!saw_code || !saw_menu || !saw_ditl) {
        fprintf(stderr, "required authentic application types are missing\n");
        return 1;
    }
    puts("PASS: authentic Macintosh application resource type list is readable");
    return 0;
}
