#include "nexus_v1_engine.h"

int main(void)
{
    Nexus_V1_DgnStructure3PackageGeometryPacket p = {0};
    p.valid = 1;
    p.source_byte_count = 64;
    p.source_bytes_fnv1a64 = p.package_fnv1a64 = 7;
    p.face_offset = 4;
    p.face_length = 12;
    p.face_fnv1a64 = 1;
    p.descriptor_fnv1a64 = 2;
    p.image_offset = 20;
    p.image_length = 16;
    p.palette_offset = 40;
    p.palette_length = 8;
    if (!p.package_fnv1a64 || p.package_fnv1a64 != p.source_bytes_fnv1a64 ||
        !p.descriptor_fnv1a64 || !p.image_length || p.image_offset > 64 ||
        p.image_length > 64 - p.image_offset || p.palette_offset > 64 ||
        p.palette_length > 64 - p.palette_offset)
        return 1;
    p.image_offset = 60;
    p.image_length = 8;
    if (!(p.image_length > 64 - p.image_offset)) return 1;
    p.image_offset = 20;
    p.image_length = 16;
    p.palette_offset = 60;
    p.palette_length = 8;
    if (!(p.palette_length > 64 - p.palette_offset)) return 1;
    p.palette_offset = 0;
    p.palette_length = 0;
    /* No palette anchor is a canonical empty interval, never an unsigned
     * conversion of the material-target -1 sentinel. */
    if (p.palette_offset != 0 || p.palette_length != 0) return 1;
    p.palette_offset = 40;
    p.palette_length = 8;
    p.package_fnv1a64 = 9;
    if (p.package_fnv1a64 == p.source_bytes_fnv1a64) return 1;
    return 0;
}
