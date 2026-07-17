#include "dm2_v1_dungeon_loader.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_G1DirectWeaponRoot weapon = { 11, 12, 0x1401u, 1u, 2u, 0x22u, 0u, 0u };
    DM2_V1_G1DirectContainerRoot closed = { 13, 14, 0xe401u, 1u, 3u, 0u, 1u };
    DM2_V1_G1DirectContainerRoot open = closed;
    DM2_V1_G1StaticObjectMaterialSelector selector;
    int pass = 1;
    open.opened = 1u;
    pass &= dm2_v1_g1_static_object_material_selector(&weapon, 0xfe02u, &selector) &&
        selector.category == 0x10u && selector.image_field == 0u &&
        selector.image_offset == 0xfe02u && selector.identity_hash != 0u;
    pass &= dm2_v1_g1_static_container_material_selector(&closed, 0x0102u, &selector) &&
        selector.category == 0x14u && selector.image_field == 0u && !selector.container_open;
    pass &= dm2_v1_g1_static_container_material_selector(&open, 0x0102u, &selector) &&
        selector.image_field == 4u && selector.container_open;
    if (!pass) fputs("DM2 static object selector failed\n", stderr);
    return pass ? 0 : 1;
}
