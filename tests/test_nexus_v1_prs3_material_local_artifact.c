#include "nexus_v1_launcher.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void be32(uint8_t *p, uint32_t v) { p[0]=(uint8_t)(v>>24); p[1]=(uint8_t)(v>>16); p[2]=(uint8_t)(v>>8); p[3]=(uint8_t)v; }
static void be64(uint8_t *p, uint64_t v) { int i; for (i=7;i>=0;--i) { p[i]=(uint8_t)v; v>>=8; } }
static uint64_t fnv(const uint8_t *p, size_t n) { uint64_t h=UINT64_C(1469598103934665603); while(n--) { h^=*p++; h*=UINT64_C(1099511628211); } return h; }

int main(void)
{
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt route;
    Nexus_V1_LauncherM11Prs3MaterialLocalArtifactReceipt receipt;
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt capture_route, resume;
    uint8_t capture[NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES + 8U];
    static const uint8_t payload[8] = { 1, 3, 3, 7, 9, 2, 4, 8 };

    memset(&route, 0, sizeof(route));
    route.valid = route.opaque_saturn_card_only = route.no_draw_only = route.draw_disabled = 1;
    route.route_epoch = 7U; route.package_fnv1a64 = UINT64_C(0x1020304050607080);
    route.card_fnv1a64 = UINT64_C(0x8877665544332211);
    route.card.valid = route.card.opaque_only = 1;
    route.card.card_fnv1a64 = route.card_fnv1a64;
    route.card.package_fnv1a64 = route.package_fnv1a64;
    route.card.route_epoch = route.route_epoch;
    route.m11.valid = route.m11.no_draw_only = route.m11.draw_disabled = 1;
    route.m11.route_epoch = route.route_epoch; route.m11.package_fnv1a64 = route.package_fnv1a64;
    route.m11.presentation.valid = route.m11.presentation.no_draw_only = 1;
    route.m11.presentation.entry_index = 4U;
    route.m11.presentation.compression.valid = 1;
    route.m11.presentation.compression.entry_index = 4U;
    route.m11.presentation.compression.compressed_offset = 0x120U;
    route.m11.presentation.compression.compressed_length = 0x34U;
    route.m11.presentation.compression.compressed_fnv1a64 = UINT64_C(0x1122334455667788);
    route.m11.presentation.compression.declared_output_bytes = 0x80U;
    memset(capture, 0, sizeof(capture));
    memcpy(capture, NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_MAGIC, 8U);
    be32(capture + 8U, NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_VERSION);
    be32(capture + 12U, NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES);
    be64(capture + 16U, route.route_epoch); be64(capture + 24U, route.package_fnv1a64);
    be64(capture + 32U, route.card_fnv1a64); be32(capture + 40U, 4U);
    be32(capture + 44U, 0x120U); be32(capture + 48U, 0x34U); be32(capture + 52U, 0x80U);
    be64(capture + 56U, route.m11.presentation.compression.compressed_fnv1a64);
    be32(capture + 64U, NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES); be32(capture + 68U, sizeof(payload));
    memcpy(capture + NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES, payload, sizeof(payload));
    be64(capture + 72U, fnv(payload, sizeof(payload))); be64(capture + 80U, UINT64_C(0x55)); be64(capture + 88U, 12U);
    if (!nexus_v1_launcher_verify_m11_prs3_material_local_artifact(&route, capture, sizeof(capture), &receipt) ||
        !receipt.valid || !receipt.route_bound || !receipt.entry_bound || !receipt.body_bound ||
        !receipt.declared_output_bound || !receipt.payload_bounds_bound || !receipt.payload_hash_bound ||
        !receipt.payload_opaque || !receipt.no_draw_only || receipt.decoder_permitted || receipt.fallback_visuals_permitted) return 1;
    if (!nexus_v1_launcher_admit_m12_m11_prs3_material_capture_required(&route,
        "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", &capture_route) ||
        !capture_route.capture_required || !capture_route.operator_only || !capture_route.no_draw_only) return 1;
    if (!nexus_v1_launcher_resume_m12_m11_prs3_material_capture(&capture_route,
        "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
        capture, sizeof(capture), &resume) || !resume.resume_ready || !resume.capture_imported ||
        resume.capture_required || !resume.no_draw_only || resume.decoder_permitted) return 1;
    if (nexus_v1_launcher_resume_m12_m11_prs3_material_capture(&capture_route,
        "06e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
        capture, sizeof(capture), &resume) || resume.valid || !resume.capture_required) return 1;
    capture[40U] ^= 1U;
    if (nexus_v1_launcher_verify_m11_prs3_material_local_artifact(&route, capture, sizeof(capture), &receipt) || receipt.valid) return 1;
    capture[40U] ^= 1U; capture[56U] ^= 1U;
    if (nexus_v1_launcher_verify_m11_prs3_material_local_artifact(&route, capture, sizeof(capture), &receipt) || receipt.valid) return 1;
    capture[56U] ^= 1U; route.card_fnv1a64 ^= 1U;
    if (nexus_v1_launcher_verify_m11_prs3_material_local_artifact(&route, capture, sizeof(capture), &receipt) || receipt.valid || !receipt.no_draw_only) return 1;
    return 0;
}
