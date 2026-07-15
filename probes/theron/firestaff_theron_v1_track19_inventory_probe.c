#include "theron_v1_track19_inventory.h"

int main(void) {
    Theron_V1Track19InventoryReceipt receipt;

    return theron_v1_track19_inventory(
            "51b40a17b92a30339957ba564aa0015c",
            5983488u,
            &receipt) &&
        receipt.container_format_unproven &&
        !receipt.startup_usable &&
        !receipt.level_usable &&
        !receipt.bitmap_usable &&
        !theron_v1_track19_inventory("bad", 5983488u, &receipt) ? 0 : 1;
}
