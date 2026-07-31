/* Hash-gated PC Engine Track 02 IPL loader provenance probe.
 *
 * Covers only the original bootstrap: IPL record 0x3a3, loaded executable
 * span, and its System Card CD_READ local-RAM setup.  It does not decode or
 * render graphics.  Optional real-media checks use
 * FIRESTAFF_THERON_TRACK02_{JP,US}_BIN. */

#include "asset_status_m12.h"
#include "theron_v1_boot.h"
#include "theron_v1_stage2_runtime_handoff.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_DATA_OFFSET 16u

static int g_failures;

static void check(int condition, const char *label) {
    if (!condition) {
        ++g_failures;
        printf("FAIL %s\n", label);
    }
}

static void put_user(uint8_t *data, size_t sector, size_t offset, uint8_t value) {
    data[sector * RAW_SECTOR_BYTES + USER_DATA_OFFSET + offset] = value;
}

static void put_bytes(uint8_t *data, size_t sector, size_t offset,
                      const uint8_t *bytes, size_t byte_count) {
    size_t i;
    for (i = 0u; i < byte_count; ++i) put_user(data, sector, offset + i, bytes[i]);
}

static uint8_t *make_fixture(size_t index01, size_t executable_sectors,
                             size_t *out_size) {
    static const uint8_t signature[] = "PC Engine CD-ROM SYSTEM";
    static const uint8_t read_setup[] = {
        0xa9u, 0x00u, 0x85u, 0xfau, 0xa9u, 0x30u, 0x85u, 0xfbu,
        0xa9u, 0x01u, 0x85u, 0xffu, 0x20u, 0x09u, 0xe0u
    };
    static const uint8_t exec_setup[] = {
        0x82u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfcu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfeu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfdu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xf8u,
        0xa9u, 0x00u, 0x85u, 0xfau, 0xa9u, 0x40u, 0x85u, 0xfbu,
        0xa9u, 0x01u, 0x85u, 0xffu, 0x20u, 0x0fu, 0xe0u
    };
    static const uint8_t exec_record[] = {0x00u, 0xe7u, 0x03u, 0x11u};
    static const uint8_t stage2_read_setup[] = {
        0xa9u, 0x01u, 0x85u, 0xf8u, 0xa9u, 0x01u, 0x85u, 0xffu,
        0xa9u, 0x00u, 0x85u, 0xfau, 0xa9u, 0x38u, 0x85u, 0xfbu,
        0x20u, 0x09u, 0xe0u
    };
    static const uint8_t cd_exec_retry_branch[] = {0x80u, 0xd7u};
    static const uint8_t cd_read_table_load[] = {
        0x82u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xfcu,
        0xe8u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xfeu,
        0xe8u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xfdu,
        0xe8u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xf8u
    };
    static const uint8_t stage2_seed_call[] = {0x20u, 0xaeu, 0x40u};
    static const uint8_t stage2_seed_bsr[] = {0x44u, 0x2eu};
    static const uint8_t stage2_entry_prologue[] = {
        0x78u, 0xa2u, 0xffu, 0x9au, 0xadu, 0xf5u, 0xffu, 0x1au,
        0x53u, 0x08u, 0x1au, 0x1au, 0x1au, 0x53u, 0x10u, 0x48u,
        0x58u, 0x20u, 0x00u, 0x80u, 0x68u, 0x1au, 0x53u, 0x20u,
        0x1au, 0x53u, 0x40u, 0x20u, 0xb7u, 0x40u, 0xeau, 0x20u,
        0x42u, 0xe0u, 0x62u, 0x20u, 0x2du, 0xe0u, 0x20u, 0x18u,
        0xe0u
    };
    static const uint8_t stage2_main_path[] = {
        0x20u, 0x0cu, 0xe0u, 0x78u, 0x64u, 0xf5u, 0xa2u, 0xffu,
        0x9au, 0x58u, 0xa9u, 0x10u, 0x85u, 0xffu, 0x20u, 0xd8u,
        0xe0u, 0xa9u, 0x01u, 0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u,
        0x20u, 0x2du, 0x4bu, 0x20u, 0x73u, 0x4bu, 0xa9u, 0x00u,
        0x20u, 0x6cu, 0xe0u, 0xa9u, 0x00u, 0xa2u, 0x20u, 0xa0u,
        0x1eu, 0x20u, 0x6fu, 0xe0u, 0x20u, 0x78u, 0xe0u, 0x20u,
        0x81u, 0xe0u, 0x20u, 0x99u, 0xe0u, 0xa9u, 0x10u, 0x20u,
        0x9cu, 0xe0u, 0x62u, 0x20u, 0x69u, 0xe0u, 0x9cu, 0x0cu,
        0x22u, 0x73u, 0x0cu, 0x22u, 0x0du, 0x22u, 0x07u, 0x00u,
        0x20u, 0x7bu, 0xe0u, 0x73u, 0x00u, 0x27u, 0x00u, 0x20u,
        0x80u, 0x00u
    };
    static const uint8_t stage2_dispatcher[] = {
        0x64u, 0x02u, 0x20u, 0x14u, 0x48u, 0x9cu, 0xc1u, 0x4eu,
        0xa9u, 0x02u, 0x20u, 0x5eu, 0x4fu, 0xa9u, 0x00u, 0x85u,
        0x1cu, 0xa9u, 0x60u, 0x85u, 0x1du, 0x20u, 0xf7u, 0x4au,
        0x20u, 0x63u, 0xe0u, 0xadu, 0x28u, 0x22u, 0x29u, 0x0cu,
        0xf0u, 0x03u, 0xeau, 0xeau, 0xeau, 0xc2u, 0xb2u, 0x1cu,
        0x0au, 0xaau, 0x7cu, 0x0du, 0x41u, 0x18u, 0x65u, 0x1cu,
        0x85u, 0x1cu, 0x62u, 0x65u, 0x1du, 0x85u, 0x1du, 0x80u,
        0xdcu, 0x60u
    };
    static const uint8_t stage2_delay[] = {
        0x48u, 0xdau, 0xa9u, 0xffu, 0xa2u, 0xffu, 0xcau, 0xd0u,
        0xfdu, 0x3au, 0xd0u, 0xf8u, 0xfau, 0x68u, 0x60u
    };
    static const uint8_t stage2_port_clear[] = {
        0x78u, 0x03u, 0x00u, 0x13u, 0x00u, 0x23u, 0x08u, 0x03u,
        0x02u, 0x82u, 0xa0u, 0x78u, 0x13u, 0x00u, 0x23u, 0x00u,
        0xcau, 0xd0u, 0xf9u, 0x88u, 0xd0u, 0xf6u, 0x03u, 0x05u,
        0xa5u, 0xf3u, 0x29u, 0x3fu, 0x85u, 0xf3u, 0x8du, 0x02u,
        0x00u, 0x58u, 0x60u
    };
    static const uint8_t stage2_pointer_setup[] = {
        0xa9u, 0xd3u, 0x85u, 0x00u, 0xa9u, 0x37u, 0x85u, 0x01u,
        0x18u, 0xa0u, 0x01u, 0xa5u, 0x02u, 0x71u, 0x00u, 0x85u,
        0x24u, 0xc8u, 0x62u, 0x71u, 0x00u, 0x85u, 0x23u, 0x62u,
        0x72u, 0x00u, 0x85u, 0x22u, 0xa9u, 0x00u, 0x85u, 0x20u,
        0xa9u, 0x28u, 0x85u, 0x21u, 0xa9u, 0x01u, 0x85u, 0x1eu,
        0x85u, 0x25u, 0x20u, 0x3eu, 0x38u, 0x60u
    };
    static const uint8_t stage2_seed_tail[] = {0xfcu, 0x60u};
    static const uint8_t stage2_dispatch_stubs[] = {
        0xa9u, 0x01u, 0x80u, 0xefu, 0xa9u, 0x02u, 0x80u, 0xebu,
        0xa9u, 0x03u, 0x80u, 0xe7u, 0xa9u, 0x04u, 0x80u, 0xe3u,
        0xa9u, 0x05u, 0x80u, 0xdfu, 0xa9u, 0x07u, 0x80u, 0xdbu,
        0xa9u, 0x09u, 0x80u, 0xd7u
    };
    static const uint8_t stage2_jump_table[] = {
        0xc5u, 0x41u, 0xcbu, 0x41u, 0xd8u, 0x41u, 0xdeu, 0x41u,
        0xe6u, 0x41u, 0xecu, 0x41u, 0xf0u, 0x41u, 0xf4u, 0x41u,
        0x14u, 0x42u, 0x53u, 0x42u
    };
    static const uint8_t stage2_mpr_page[] = {
        0x18u, 0xadu, 0xf5u, 0xffu, 0x69u, 0x01u, 0x53u, 0x08u,
        0x60u
    };
    static const uint8_t stage2_selector[] = {
        0xa2u, 0xc1u, 0xa0u, 0x4eu, 0x20u, 0x14u, 0x31u, 0x60u
    };
    static const uint8_t stage2_l8000[] = {
        0xc6u, 0x5au, 0x9cu, 0x0cu, 0x22u, 0x9cu, 0x0du, 0x22u,
        0x9cu, 0x10u, 0x22u, 0x9cu, 0x11u, 0x22u, 0x03u, 0x08u,
        0x13u, 0x00u, 0x23u, 0x00u, 0x03u, 0x07u, 0x13u, 0x00u,
        0x23u, 0x00u, 0x64u, 0x5au, 0x20u, 0xa6u, 0x45u, 0xa5u,
        0x00u, 0x85u, 0x4cu, 0xa5u, 0x01u, 0x85u, 0x4du, 0xa9u,
        0x08u, 0x18u, 0x65u, 0x00u, 0x85u, 0x00u, 0x90u, 0x02u,
        0xe6u, 0x01u, 0xa5u, 0x00u, 0x8du, 0xcbu, 0x47u, 0xa5u,
        0x01u, 0x8du, 0xccu, 0x47u, 0xa5u, 0x01u, 0x18u, 0x69u,
        0x10u, 0x85u, 0x01u, 0xa5u, 0x00u, 0x8du, 0xcdu, 0x47u,
        0xa5u, 0x01u, 0x8du, 0xceu, 0x47u, 0xa9u, 0x00u, 0x8du,
        0xc7u, 0x47u, 0xa9u, 0x01u, 0x8du, 0xc8u, 0x47u, 0xa0u,
        0x04u, 0xb1u, 0x4cu, 0x8du, 0xbfu, 0x47u, 0x85u, 0x0eu,
        0x64u, 0x0fu, 0xc8u, 0xb1u, 0x4cu, 0x8du, 0xbeu, 0x47u,
        0x85u, 0x10u, 0x20u, 0x96u, 0x46u, 0xa5u, 0x0eu, 0x8du,
        0xc9u, 0x47u, 0xa5u, 0x0fu, 0x8du, 0xcau, 0x47u, 0xa0u,
        0x02u, 0xb1u, 0x4cu, 0x85u, 0x00u, 0xc8u, 0xb1u, 0x4cu,
        0x85u, 0x01u, 0xa5u, 0x4cu, 0x18u, 0x65u, 0x00u, 0x85u,
        0x00u, 0xa5u, 0x4du, 0x65u, 0x01u, 0x85u, 0x01u, 0xa5u,
        0x00u, 0x8du, 0x6au, 0x3bu, 0xa5u, 0x01u, 0x8du, 0x6bu,
        0x3bu, 0xa5u, 0x00u, 0x8du, 0xd1u, 0x47u, 0xa5u, 0x01u,
        0x8du, 0xd2u, 0x47u, 0x64u, 0x02u, 0x64u, 0x03u, 0xa0u,
        0x06u, 0xb1u, 0x4cu, 0x8du, 0x6fu, 0x3bu, 0x0au, 0x0au,
        0x0au, 0x0au, 0xaau, 0xadu, 0x68u, 0x3bu, 0xd0u, 0x03u,
        0x20u, 0xfcu, 0x48u, 0x60u
    };
    static const uint8_t stage2_l45a6[] = {
        0xb1u, 0x1cu, 0x85u, 0x01u, 0xadu, 0xe7u, 0x44u, 0x85u,
        0x02u, 0xadu, 0xe8u, 0x44u, 0x85u, 0x03u, 0xadu, 0xe9u,
        0x44u, 0x85u, 0x04u, 0xadu, 0xeau, 0x44u, 0x85u, 0x05u,
        0x68u, 0x4au, 0xb0u, 0x05u, 0xa9u, 0x17u, 0x20u, 0xb7u,
        0x3au, 0x4cu, 0x05u, 0x41u
    };
    static const uint8_t stage2_jump_table_handlers[] = {
        0x44u, 0xf2u, 0x62u, 0x4cu, 0xe4u, 0x40u, 0x44u, 0x2bu,
        0xd0u, 0x06u, 0x44u, 0xe8u, 0x62u, 0x4cu, 0xe4u, 0x40u,
        0x4cu, 0x01u, 0x41u, 0x44u, 0x1eu, 0xd0u, 0xf3u, 0x80u,
        0xf7u, 0x44u, 0x18u, 0x90u, 0xf3u, 0xf0u, 0xf1u, 0x80u,
        0xe9u, 0x44u, 0x10u, 0xb0u, 0xebu, 0x80u, 0xe3u, 0x44u,
        0x15u, 0x80u, 0xddu, 0x44u, 0x11u, 0x80u, 0xe6u, 0x44u,
        0x0du, 0x80u, 0xf0u, 0xc8u, 0xb1u, 0x1cu, 0xaau, 0xbdu,
        0x80u, 0x27u, 0xc8u, 0xd1u, 0x1cu, 0x60u, 0xc8u, 0xb1u,
        0x1cu, 0xaau, 0xbdu, 0x80u, 0x27u, 0x48u, 0xc8u, 0xb1u,
        0x1cu, 0xaau, 0x68u, 0xddu, 0x80u, 0x27u, 0x60u, 0xc8u,
        0xb1u, 0x1cu, 0x44u, 0x03u, 0x4cu, 0xf5u, 0x40u, 0x8du,
        0xc1u, 0x4eu, 0x8du, 0x7bu, 0x4du, 0x0au, 0x0au, 0x18u,
        0x6du, 0x08u, 0x30u, 0x8du, 0x09u, 0x30u, 0xa9u, 0x02u,
        0x20u, 0x5eu, 0x4fu, 0xb0u, 0x01u, 0x60u, 0x00u, 0xc6u,
        0x5bu, 0x20u, 0xd6u, 0x43u, 0x44u, 0x05u, 0x64u, 0x5bu,
        0x4cu, 0xf5u, 0x40u, 0x20u, 0xd8u, 0x37u, 0xa9u, 0x00u,
        0x85u, 0x20u, 0xa9u, 0x68u, 0x85u, 0x21u, 0xa9u, 0x03u,
        0x85u, 0x1eu, 0x20u, 0x3eu, 0x38u, 0x60u, 0x60u
    };
    static const uint8_t stage2_l4696[] = {
        0x64u, 0x0fu, 0x64u, 0x11u, 0xa5u, 0x0eu, 0x85u, 0x12u,
        0x64u, 0x0eu, 0xa2u, 0x01u, 0xffu, 0x12u, 0x16u, 0xefu,
        0x12u, 0x14u, 0xdfu, 0x12u, 0x12u, 0xcfu, 0x12u, 0x10u,
        0xbfu, 0x12u, 0x0eu, 0xafu, 0x12u, 0x0cu, 0x9fu, 0x12u,
        0x0au, 0x8fu, 0x12u, 0x08u, 0x60u, 0xe8u, 0xe8u, 0xe8u,
        0xe8u, 0xe8u, 0xe8u, 0xe8u, 0x46u, 0x12u, 0x90u, 0x0du,
        0x18u, 0xa5u, 0x10u, 0x65u, 0x0eu, 0x85u, 0x0eu, 0xa5u,
        0x11u, 0x65u, 0x0fu, 0x85u, 0x0fu, 0x06u, 0x10u, 0x26u,
        0x11u, 0xcau, 0xd0u, 0xe8u, 0x60u
    };
    static const uint8_t stage2_l3114[] = {
        0x48u, 0x44u, 0x5bu, 0xadu, 0xdeu, 0x4fu, 0xd0u, 0x04u,
        0x44u, 0x5fu, 0x80u, 0x08u, 0xa9u, 0x1au, 0x20u, 0x66u,
        0x4fu, 0x3au, 0xd0u, 0xfau, 0x68u, 0x8du, 0x8eu, 0x4fu,
        0x68u, 0x8du, 0x8du, 0x4fu, 0xadu, 0x9du, 0x4fu, 0x85u,
        0x06u, 0xadu, 0x9eu, 0x4fu, 0x85u, 0x07u, 0xadu, 0x93u,
        0x4fu, 0x8du, 0x8bu, 0x4fu, 0xadu, 0x94u, 0x4fu, 0x8du,
        0x8cu, 0x4fu, 0x20u, 0x6du, 0x52u, 0x18u, 0xa5u, 0x06u,
        0x69u, 0x04u, 0x85u, 0x06u, 0x90u, 0x02u, 0xe6u, 0x07u,
        0xaeu, 0x8du, 0x4fu, 0xacu, 0x8eu, 0x4fu, 0xdau, 0xa5u,
        0x0eu, 0x48u, 0xa5u, 0x0fu, 0x48u, 0x20u, 0xe0u, 0x55u,
        0x68u, 0x85u, 0x0fu, 0x68u, 0x85u, 0x0eu, 0x20u, 0x13u,
        0x52u, 0xfau, 0x88u, 0xd0u, 0xe9u, 0x60u
    };
    static const uint8_t stage2_l3172[] = {
        0xa9u, 0x01u, 0x8du, 0x22u, 0x5cu, 0xa9u, 0x01u, 0x8du,
        0x23u, 0x5cu, 0x60u
    };
    static const uint8_t stage2_far117d[] = {
        0x20u, 0xf5u, 0x5bu, 0x20u, 0x8cu, 0x5cu, 0x20u, 0xb0u,
        0x5cu, 0x20u, 0x25u, 0x5cu, 0x60u
    };
    static const uint8_t stage2_l4f66[] = {
        0x48u, 0xdau, 0x5au, 0xa9u, 0x03u, 0x82u, 0xc2u, 0x88u,
        0xd0u, 0xfdu, 0xcau, 0xd0u, 0xf9u, 0x3au, 0xd0u, 0xf5u,
        0x7au, 0xfau, 0x68u, 0x60u
    };
    static const uint8_t stage2_l5213[] = {
        0x18u, 0xa5u, 0x0eu, 0x69u, 0x40u, 0x85u, 0x0eu, 0x90u,
        0x02u, 0xe6u, 0x0fu, 0x60u
    };
    static const uint8_t stage2_l526d[] = {
        0x20u, 0xf9u, 0x51u, 0xa2u, 0x04u, 0x46u, 0x07u, 0x66u,
        0x06u, 0xcau, 0xd0u, 0xf9u, 0xa5u, 0x07u, 0x09u, 0xf0u,
        0x85u, 0x07u, 0x60u
    };
    static const uint8_t stage2_l55e0[] = {
        0x44u, 0x14u, 0x44u, 0x04u, 0xcau, 0xd0u, 0xf9u, 0x60u
    };
    static const uint8_t stage2_l4696_call_site[] = {
        0x20u, 0x96u, 0x46u
    };
    static const uint8_t stage2_l51f9[] = {
        0x64u, 0x0eu, 0xadu, 0x8cu, 0x4fu, 0x4au, 0x66u, 0x0eu,
        0x4au, 0x66u, 0x0eu, 0x85u, 0x0fu, 0x18u, 0xa5u, 0x0eu,
        0x6du, 0x8bu, 0x4fu, 0x85u, 0x0eu, 0x90u, 0x02u, 0xe6u,
        0x0fu, 0x60u
    };
    static const uint8_t stage2_l55e8[] = {
        0xe6u, 0x0eu, 0xd0u, 0x02u, 0xe6u, 0x0fu, 0x60u
    };
    static const uint8_t stage2_l55f6[] = {
        0xc6u, 0x5au, 0x20u, 0xa0u, 0x54u, 0x44u, 0x03u, 0x64u,
        0x5au, 0x60u
    };
    static const uint8_t stage2_l5bf5[] = {
        0xa0u, 0x04u, 0x82u, 0xbdu, 0x20u, 0x5cu, 0x9du, 0x8bu,
        0x4fu, 0xe8u, 0x88u, 0xd0u, 0xf6u, 0x9cu, 0xd1u, 0x4fu,
        0x60u
    };
    static const uint8_t stage2_l5c25[] = {
        0xa9u, 0xf0u, 0x8du, 0x24u, 0x5cu, 0x80u, 0x05u, 0xa9u,
        0xefu, 0x8du, 0x24u, 0x5cu, 0x20u, 0x6eu, 0x53u, 0xadu,
        0xb8u, 0x4fu, 0x3au, 0x0au, 0xaau, 0xbdu, 0xb9u, 0x4fu,
        0x18u, 0x69u, 0x06u, 0x85u, 0x04u, 0xe8u, 0x62u, 0x7du,
        0xb9u, 0x4fu, 0x85u, 0x05u, 0xacu, 0x8eu, 0x4fu, 0xaeu,
        0x8du, 0x4fu, 0x5au, 0xdau, 0x44u, 0x16u, 0xfau, 0x7au,
        0x88u, 0xd0u, 0xf7u, 0xadu, 0xd4u, 0x4fu, 0x48u, 0xa9u,
        0x01u, 0x8du, 0xd4u, 0x4fu, 0x20u, 0x39u, 0x54u, 0x68u,
        0x8du, 0xd4u, 0x4fu, 0x60u
    };
    static const uint8_t stage2_l5c8c[] = {
        0x20u, 0x06u, 0x5cu, 0xf0u, 0x03u, 0x20u, 0x9fu, 0x5cu,
        0x20u, 0x63u, 0xe0u, 0xadu, 0x2du, 0x22u, 0xf0u, 0xf0u,
        0x85u, 0x08u, 0x60u
    };
    static const uint8_t stage2_l5cb0[] = {
        0x48u, 0xdau, 0x5au, 0x20u, 0x63u, 0xe0u, 0xadu, 0x28u,
        0x22u, 0xd0u, 0xf8u, 0x7au, 0xfau, 0x68u, 0x60u
    };
    static const uint8_t stage2_l5c06[] = {
        0xceu, 0xd1u, 0x4fu, 0xf0u, 0x01u, 0x60u, 0xadu, 0xd2u,
        0x4fu, 0x49u, 0x01u, 0x8du, 0xd2u, 0x4fu, 0xf0u, 0x05u,
        0x20u, 0x25u, 0x5cu, 0x62u, 0x60u, 0x20u, 0x2cu, 0x5cu,
        0x62u, 0x60u
    };
    static const uint8_t stage2_l5c20[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u
    };
    static const uint8_t stage2_l5c69[] = {
        0xadu, 0x24u, 0x5cu, 0xc9u, 0xefu, 0xf0u, 0x04u, 0xa9u,
        0x0du, 0x80u, 0x02u, 0xa9u, 0x2du, 0x8du, 0x7eu, 0x5cu,
        0xdau, 0xc2u, 0xc8u, 0xb1u, 0x04u, 0x0du, 0x24u, 0x5cu,
        0x91u, 0x04u, 0xc8u, 0xcau, 0xd0u, 0xf4u, 0x68u, 0x20u,
        0x92u, 0x54u, 0x60u
    };
    static const uint8_t stage2_l5c9f[] = {
        0xadu, 0xd4u, 0x4fu, 0x48u, 0xa9u, 0x07u, 0x8du, 0xd4u,
        0x4fu, 0x20u, 0x7au, 0x4fu, 0x68u, 0x8du, 0xd4u, 0x4fu,
        0x60u
    };
    static const uint8_t stage2_l536e[] = {
        0xadu, 0xb8u, 0x4fu, 0x0au, 0xaau, 0xadu, 0xd5u, 0x4fu,
        0x9du, 0xb9u, 0x4fu, 0xe8u, 0xadu, 0xd6u, 0x4fu, 0x9du,
        0xb9u, 0x4fu, 0xeeu, 0xb8u, 0x4fu, 0xdau, 0x64u, 0x0eu,
        0x64u, 0x0fu, 0xaeu, 0x8eu, 0x4fu, 0x18u, 0xa5u, 0x0eu,
        0x6du, 0x8du, 0x4fu, 0x85u, 0x0eu, 0x90u, 0x02u, 0xe6u,
        0x0fu, 0xcau, 0xd0u, 0xf1u, 0x06u, 0x0eu, 0x26u, 0x0fu,
        0xfau, 0x18u, 0xa5u, 0x0eu, 0x6du, 0xd5u, 0x4fu, 0x85u,
        0x0eu, 0xa5u, 0x0fu, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x0fu,
        0xa5u, 0x0fu, 0xc9u, 0xdfu, 0x90u, 0x06u, 0xd0u, 0x04u,
        0xa5u, 0x0eu, 0xc9u, 0xf0u, 0x90u, 0x08u, 0x9eu, 0xb9u,
        0x4fu, 0xcau, 0x9eu, 0xb9u, 0x4fu, 0x60u
    };
    static const uint8_t stage2_l5439[] = {
        0xceu, 0xb8u, 0x4fu, 0xadu, 0xb8u, 0x4fu, 0x0au, 0xaau,
        0xbdu, 0xb9u, 0x4fu, 0x85u, 0x04u, 0xe8u, 0xbdu, 0xb9u,
        0x4fu, 0x85u, 0x05u, 0xa5u, 0x04u, 0xd0u, 0x05u, 0xa5u,
        0x05u, 0xd0u, 0x01u, 0x60u
    };
    static const uint8_t stage2_l54a0[] = {
        0x03u, 0x00u, 0xa5u, 0x0eu, 0x8du, 0x02u, 0x00u, 0xa5u,
        0x0fu, 0x8du, 0x03u, 0x00u, 0x03u, 0x02u, 0x60u
    };
    static const uint8_t stage2_l5600[] = {
        0xa5u, 0x06u, 0x8du, 0x02u, 0x00u, 0xa5u, 0x07u, 0x8du,
        0x03u, 0x00u, 0x60u
    };
    static const uint8_t stage2_l4f7a[] = {
        0xdau, 0x5au, 0xaeu, 0xd4u, 0x4fu, 0xc2u, 0x88u, 0xd0u,
        0xfdu, 0xcau, 0xd0u, 0xf9u, 0x7au, 0xfau, 0x60u
    };
    static const uint8_t stage2_l535e[] = {
        0xb1u, 0x04u, 0xc8u, 0x8du, 0x02u, 0x00u, 0xb1u, 0x04u,
        0xc8u, 0x8du, 0x03u, 0x00u, 0xcau, 0xd0u, 0xf1u, 0x60u
    };
    static const uint8_t stage2_l53c4[] = {
        0x20u, 0xf9u, 0x51u, 0xadu, 0xd5u, 0x4fu, 0x85u, 0x04u,
        0xadu, 0xd6u, 0x4fu, 0x85u, 0x05u, 0xa2u, 0x04u, 0xc2u,
        0xb9u, 0x8bu, 0x4fu, 0x91u, 0x04u, 0xc8u, 0xcau, 0xd0u,
        0xf7u, 0xa5u, 0x0eu, 0x91u, 0x04u, 0xc8u, 0xa5u, 0x0fu,
        0x91u, 0x04u, 0x44u, 0x45u, 0xacu, 0x8eu, 0x4fu, 0xaeu,
        0x8du, 0x4fu, 0x5au, 0xdau, 0x44u, 0x11u, 0xfau, 0x7au,
        0x88u, 0xd0u, 0xf7u, 0xa5u, 0x04u, 0x8du, 0xd5u, 0x4fu,
        0xa5u, 0x05u, 0x8du, 0xd6u, 0x4fu, 0x18u, 0x60u
    };
    static const uint8_t stage2_l542d[] = {
        0x18u, 0xa5u, 0x04u, 0x69u, 0x06u, 0x85u, 0x04u, 0x90u,
        0x02u, 0xe6u, 0x05u, 0x60u
    };
    static const uint8_t stage2_l5455[] = {
        0xa5u, 0x04u, 0x8du, 0xd5u, 0x4fu, 0xa5u, 0x05u, 0x8du,
        0xd6u, 0x4fu, 0xa0u, 0x04u, 0xb1u, 0x04u, 0x85u, 0x0eu,
        0xc8u, 0xb1u, 0x04u, 0x85u, 0x0fu, 0xa0u, 0x02u, 0xb1u,
        0x04u, 0xaau, 0xc8u, 0xb1u, 0x04u, 0xa8u, 0x44u, 0xb8u,
        0x5au, 0xdau, 0x20u, 0x7au, 0x4fu, 0x44u, 0x06u, 0xfau,
        0x7au, 0x88u, 0xd0u, 0xf4u, 0x60u
    };
    static const uint8_t stage2_l5482[] = {
        0xdau, 0xc2u, 0xc6u, 0x5au, 0x20u, 0xa0u, 0x54u, 0x20u,
        0x5eu, 0x53u, 0x64u, 0x5au, 0x68u, 0x44u, 0x01u, 0x60u
    };
    static const uint8_t stage2_l5492[] = {
        0x0au, 0x18u, 0x65u, 0x04u, 0x85u, 0x04u, 0x90u, 0x02u,
        0xe6u, 0x05u, 0x20u, 0x13u, 0x52u, 0x60u
    };
    static const uint8_t stage2_l54af[] = {
        0xadu, 0x9fu, 0x4fu, 0x0au, 0xaau, 0xadu, 0xd7u, 0x4fu,
        0x9du, 0xa0u, 0x4fu, 0xe8u, 0xadu, 0xd8u, 0x4fu, 0x9du,
        0xa0u, 0x4fu, 0xeeu, 0x9fu, 0x4fu, 0x60u
    };
    static const uint8_t stage2_l560b[] = {
        0x18u, 0xadu, 0xd5u, 0x4fu, 0x69u, 0x11u, 0x85u, 0x02u,
        0x62u, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x03u, 0xa9u, 0x67u,
        0x85u, 0x00u, 0xa9u, 0x56u, 0x85u, 0x01u, 0xa2u, 0x09u,
        0xdau, 0xa5u, 0x00u, 0x48u, 0xa5u, 0x01u, 0x48u, 0x44u,
        0x2bu, 0x20u, 0xa2u, 0x52u, 0x82u, 0x20u, 0xc8u, 0x52u,
        0x18u, 0xadu, 0xd7u, 0x4fu, 0x69u, 0x10u, 0x8du, 0xd7u,
        0x4fu, 0x90u, 0x03u, 0xeeu, 0xd8u, 0x4fu, 0x68u, 0x85u,
        0x01u, 0x68u, 0x85u, 0x00u, 0xfau, 0x18u, 0xa5u, 0x00u,
        0x69u, 0x08u, 0x85u, 0x00u, 0x90u, 0x02u, 0xe6u, 0x01u,
        0xcau, 0xd0u, 0xcdu, 0x60u
    };
    static const uint8_t stage2_45xx_routine[] = {
        0xa5u, 0x12u, 0x48u, 0xa4u, 0x11u, 0xa9u, 0x08u, 0x85u,
        0x0eu, 0x20u, 0x96u, 0x46u, 0xa5u, 0x0eu, 0x85u, 0x54u,
        0xa5u, 0x0fu, 0x85u, 0x55u, 0x84u, 0x10u, 0xa9u, 0x08u,
        0x85u, 0x0eu, 0x20u, 0x96u, 0x46u, 0xa5u, 0x0eu, 0x85u,
        0x52u, 0xa5u, 0x0fu, 0x85u, 0x53u, 0x20u, 0x52u, 0x45u,
        0xa5u, 0x13u, 0x0au, 0x8du, 0xb8u, 0x47u, 0x68u, 0x0au,
        0x8du, 0xb9u, 0x47u, 0xc6u, 0x5au, 0x20u, 0x32u, 0x49u,
        0x64u, 0x5au, 0xa5u, 0x14u, 0x85u, 0x0eu, 0xa5u, 0x15u,
        0x85u, 0x0fu, 0x20u, 0x8eu, 0x45u, 0xa5u, 0x02u, 0x85u,
        0x06u, 0xa5u, 0x03u, 0x85u, 0x07u, 0xa5u, 0x56u, 0x8du,
        0x6au, 0x46u, 0xa5u, 0x57u, 0x85u, 0x16u, 0xa5u, 0x58u,
        0x85u, 0x17u, 0xadu, 0xb9u, 0x47u, 0x4au, 0xaau, 0xdau,
        0xadu, 0x6au, 0x46u, 0x85u, 0x56u, 0xa5u, 0x16u, 0x85u,
        0x57u, 0xa5u, 0x17u, 0x85u, 0x58u, 0x20u, 0x4bu, 0x42u,
        0xa9u, 0xe0u, 0x85u, 0x00u, 0xa9u, 0x47u, 0x85u, 0x01u,
        0xa5u, 0x06u, 0x85u, 0x02u, 0xa5u, 0x07u, 0x85u, 0x03u,
        0xadu, 0xb8u, 0x47u, 0x85u, 0x0eu, 0x64u, 0x0fu, 0x20u,
        0x6bu, 0x46u, 0xa9u, 0x40u, 0x18u, 0x65u, 0x06u, 0x85u,
        0x06u, 0x90u, 0x02u, 0xe6u, 0x07u, 0xa5u, 0x17u, 0x49u,
        0x02u, 0x85u, 0x17u, 0xd0u, 0x17u, 0xe6u, 0x16u, 0xa5u,
        0x16u, 0xc9u, 0x10u, 0xd0u, 0x0fu, 0x64u, 0x16u, 0xadu,
        0xbeu, 0x47u, 0x18u, 0x6du, 0xc4u, 0x47u, 0x8du, 0xc4u,
        0x47u, 0x20u, 0xd6u, 0x43u, 0xfau, 0xcau, 0xd0u, 0xa7u,
        0x60u
    };
    static const uint8_t stage2_l424b[] = {
        0x64u, 0x13u, 0x20u, 0xd6u, 0x43u, 0xadu, 0xb8u, 0x47u,
        0x85u, 0x11u, 0xa9u, 0x02u, 0x85u, 0x10u, 0xc2u, 0xb1u,
        0x4eu, 0x85u, 0x14u, 0xc8u, 0xb1u, 0x4eu, 0x85u, 0x15u,
        0xc8u, 0x20u, 0xa1u, 0x43u, 0xa5u, 0x58u, 0x29u, 0x01u,
        0x49u, 0x03u, 0x3au, 0xaau, 0x64u, 0x12u, 0x44u, 0x33u,
        0xc6u, 0x11u, 0xcau, 0xd0u, 0xf9u, 0x20u, 0xbfu, 0x42u,
        0xa5u, 0x58u, 0x29u, 0x02u, 0x85u, 0x58u, 0xa4u, 0x10u,
        0xe6u, 0x10u, 0xe6u, 0x10u, 0xb1u, 0x4eu, 0x85u, 0x14u,
        0xc8u, 0xb1u, 0x4eu, 0x85u, 0x15u, 0x20u, 0xa1u, 0x43u,
        0xa2u, 0x02u, 0x64u, 0x12u, 0x44u, 0x0du, 0xc6u, 0x11u,
        0xf0u, 0x08u, 0xcau, 0xd0u, 0xf7u, 0x20u, 0xbfu, 0x42u,
        0x80u, 0xdcu, 0x60u, 0xdau, 0xa4u, 0x12u, 0xa6u, 0x13u,
        0xb1u, 0x00u, 0x9du, 0xe0u, 0x47u, 0xc8u, 0xe8u, 0xb1u,
        0x00u, 0x9du, 0xe0u, 0x47u, 0xc8u, 0xe8u, 0x84u, 0x12u,
        0x86u, 0x13u, 0xfau, 0x60u
    };
    static const uint8_t stage2_l43d6[] = {
        0xa5u, 0x57u, 0x85u, 0x4eu, 0x64u, 0x4fu, 0x06u, 0x4eu,
        0x26u, 0x4fu, 0x06u, 0x4eu, 0x26u, 0x4fu, 0x06u, 0x4eu,
        0x26u, 0x4fu, 0x06u, 0x4eu, 0x26u, 0x4fu, 0xa5u, 0x56u,
        0x18u, 0x65u, 0x4eu, 0x85u, 0x4eu, 0x62u, 0x65u, 0x4fu,
        0x85u, 0x4fu, 0x06u, 0x4eu, 0x26u, 0x4fu, 0xadu, 0xc4u,
        0x47u, 0xf0u, 0x06u, 0x0au, 0x18u, 0x65u, 0x4fu, 0x85u,
        0x4fu, 0xadu, 0xcdu, 0x47u, 0x18u, 0x65u, 0x4eu, 0x85u,
        0x4eu, 0xadu, 0xceu, 0x47u, 0x65u, 0x4fu, 0x85u, 0x4fu,
        0x60u
    };
    static const uint8_t stage2_l4552[] = {
        0xa5u, 0x52u, 0x4au, 0x4au, 0x4au, 0x4au, 0x85u, 0x56u,
        0xa5u, 0x54u, 0x4au, 0x4au, 0x4au, 0x4au, 0x85u, 0x57u,
        0x62u, 0xa6u, 0x55u, 0xf0u, 0x0cu, 0xadu, 0xbeu, 0x47u,
        0x85u, 0x0eu, 0x62u, 0x18u, 0x65u, 0x0eu, 0xcau, 0xd0u,
        0xfau, 0x18u, 0x65u, 0x53u, 0x8du, 0xc4u, 0x47u, 0xa5u,
        0x54u, 0x29u, 0x08u, 0x4au, 0x4au, 0x85u, 0x0eu, 0xa5u,
        0x52u, 0x29u, 0x08u, 0x4au, 0x4au, 0x4au, 0x18u, 0x65u,
        0x0eu, 0x85u, 0x58u, 0x60u
    };
    static const uint8_t stage2_l458e[] = {
        0x64u, 0x02u, 0xa5u, 0x0eu, 0x4au, 0x66u, 0x02u, 0x4au,
        0x66u, 0x02u, 0x85u, 0x03u, 0xa5u, 0x0fu, 0x18u, 0x65u,
        0x02u, 0x85u, 0x02u, 0x90u, 0x02u, 0xe6u, 0x03u, 0x60u
    };
    static const uint8_t stage2_l466b[] = {
        0xc6u, 0x5au, 0x03u, 0x00u, 0xa5u, 0x02u, 0x8du, 0x02u,
        0x00u, 0xa5u, 0x03u, 0x8du, 0x03u, 0x00u, 0x03u, 0x02u,
        0xa5u, 0x0eu, 0xf0u, 0x14u, 0x8du, 0x91u, 0x46u, 0xa5u,
        0x00u, 0x8du, 0x8du, 0x46u, 0xa5u, 0x01u, 0x8du, 0x8eu,
        0x46u, 0xe3u, 0x00u, 0x00u, 0x02u, 0x00u, 0x00u, 0x00u,
        0x64u, 0x5au, 0x60u
    };
    static const uint8_t stage2_l4932[] = {
        0x03u, 0x05u, 0xa5u, 0xf3u, 0x8du, 0x02u, 0x00u, 0xa5u,
        0xf4u, 0x29u, 0x07u, 0x85u, 0xf4u, 0x8du, 0x03u, 0x00u,
        0x60u
    };
    static const uint8_t stage2_l5403[] = {
        0xdau, 0xc2u, 0xc6u, 0x5au, 0x44u, 0x15u, 0xadu, 0x02u,
        0x00u, 0x91u, 0x04u, 0xc8u, 0xadu, 0x03u, 0x00u, 0x91u,
        0x04u, 0xc8u, 0xcau, 0xd0u, 0xf1u, 0x64u, 0x5au, 0x68u,
        0x44u, 0x75u, 0x60u
    };
    static const uint8_t stage2_l541e[] = {
        0x03u, 0x01u, 0xa5u, 0x0eu, 0x8du, 0x02u, 0x00u, 0xa5u,
        0x0fu, 0x8du, 0x03u, 0x00u, 0x03u, 0x02u, 0x60u
    };
    static const uint8_t stage2_l52a2[] = {
        0xadu, 0xd7u, 0x4fu, 0x85u, 0x06u, 0xadu, 0xd8u, 0x4fu,
        0x85u, 0x07u, 0x18u, 0xadu, 0xd5u, 0x4fu, 0x69u, 0x20u,
        0x85u, 0x04u, 0x62u, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x05u,
        0xa9u, 0x10u, 0x85u, 0x15u, 0x1au, 0x85u, 0x14u, 0x62u,
        0x85u, 0x17u, 0x1au, 0x85u, 0x16u, 0x60u
    };
    static const uint8_t stage2_l52c8[] = {
        0xb5u, 0x14u, 0x18u, 0x6du, 0xd5u, 0x4fu, 0x85u, 0x00u,
        0x62u, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x01u, 0x20u, 0xfdu,
        0x52u, 0x60u
    };
    static const uint8_t stage2_l5657[] = {
        0x82u, 0xc2u, 0xb1u, 0x00u, 0x02u, 0x91u, 0x02u, 0xc8u,
        0xc8u, 0xe8u, 0x02u, 0xc0u, 0x08u, 0xd0u, 0xf3u, 0x60u
    };
    static const uint8_t stage2_l54c5[] = {
        0xceu, 0x9fu, 0x4fu, 0xadu, 0x9fu, 0x4fu, 0x0au, 0xaau,
        0xbdu, 0xa0u, 0x4fu, 0x8du, 0xd7u, 0x4fu, 0xe8u, 0xbdu,
        0xa0u, 0x4fu, 0x8du, 0xd8u, 0x4fu, 0x60u
    };
    static const uint8_t stage2_l5667_data[] = {
        0xffu, 0xffu, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u,
        0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u,
        0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xffu,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xffu,
        0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0xffu
    };
    static const uint8_t stage2_l43a1[] = {
        0xa5u, 0x14u, 0x85u, 0x0eu, 0xa5u, 0x15u, 0x85u, 0x0fu,
        0x06u, 0x0eu, 0x26u, 0x0fu, 0x06u, 0x0eu, 0x26u, 0x0fu,
        0x06u, 0x0eu, 0x26u, 0x0fu, 0xadu, 0xcbu, 0x47u, 0x18u,
        0x65u, 0x0eu, 0x85u, 0x0eu, 0xadu, 0xccu, 0x47u, 0x65u,
        0x0fu, 0x85u, 0x0fu, 0xa5u, 0x58u, 0x0au, 0x18u, 0x65u,
        0x0eu, 0x85u, 0x0eu, 0x85u, 0x00u, 0x62u, 0x65u, 0x0fu,
        0x85u, 0x0fu, 0x85u, 0x01u, 0x60u
    };
    static const uint8_t stage2_l42bf[] = {
        0xe6u, 0x56u, 0xa6u, 0x56u, 0xe0u, 0x10u, 0xd0u, 0x13u,
        0x64u, 0x56u, 0x64u, 0x10u, 0xadu, 0xc4u, 0x47u, 0x48u,
        0x1au, 0x8du, 0xc4u, 0x47u, 0x20u, 0xd6u, 0x43u, 0x68u,
        0x8du, 0xc4u, 0x47u, 0x60u
    };
    static const uint8_t stage2_gap45a6[] = {
        0x9cu, 0xb8u, 0x47u, 0x73u, 0xb8u, 0x47u, 0xb9u, 0x47u,
        0xa7u, 0x00u, 0x60u
    };
    size_t executable_sector = index01 + THERON_TRACK02_IPL_RECORD;
    size_t stage2_sector = index01 + THERON_TRACK02_IPL_STAGE2_RECORD;
    size_t dynamic_sector = executable_sectors == 3u
                                ? THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP
                                : THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US;
    size_t sector_count = dynamic_sector + 1u;
    uint8_t *data;
    if (sector_count < stage2_sector + THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT) {
        sector_count = stage2_sector + THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT;
    }
    data = (uint8_t *)calloc(sector_count, RAW_SECTOR_BYTES);
    if (!data) return NULL;
    put_user(data, index01 + 1u, 0u, 0x00u);
    put_user(data, index01 + 1u, 1u, 0x03u);
    put_user(data, index01 + 1u, 2u, 0xa3u);
    put_user(data, index01 + 1u, 3u, (uint8_t)executable_sectors);
    put_user(data, index01 + 1u, 4u, 0x00u);
    put_user(data, index01 + 1u, 5u, 0x40u);
    put_user(data, index01 + 1u, 6u, 0x00u);
    put_user(data, index01 + 1u, 7u, 0x40u);
    put_bytes(data, index01 + 1u, 32u, signature, sizeof(signature) - 1u);
    put_bytes(data, executable_sector, 0xc1u, read_setup, sizeof(read_setup));
    put_bytes(data, executable_sector, 0x80u, exec_setup, sizeof(exec_setup));
    put_bytes(data, executable_sector, 0xd5u, exec_record, sizeof(exec_record));
    put_bytes(data, executable_sector, 0xa7u, cd_exec_retry_branch,
              sizeof(cd_exec_retry_branch));
    put_bytes(data, executable_sector, 0xa9u, cd_read_table_load,
              sizeof(cd_read_table_load));
    put_bytes(data, stage2_sector, 0x80u, stage2_read_setup,
              sizeof(stage2_read_setup));
    put_bytes(data, stage2_sector, 0x29u, stage2_seed_call,
              sizeof(stage2_seed_call));
    put_bytes(data, stage2_sector, 0x7eu, stage2_seed_bsr,
              sizeof(stage2_seed_bsr));
    put_bytes(data, stage2_sector, 0x00u, stage2_entry_prologue,
              sizeof(stage2_entry_prologue));
    put_bytes(data, stage2_sector, 0x2cu, stage2_main_path,
              sizeof(stage2_main_path));
    put_bytes(data, stage2_sector, 0xb7u, stage2_dispatcher,
              sizeof(stage2_dispatcher));
    /* The delay, port clear, and pointer setup windows live at stage-two
     * image user offsets 0xb2d/0xb73/0x814, which map into the second
     * raw sector of the image at in-sector offsets 0x32d/0x373/0x14. */
    put_bytes(data, stage2_sector + 1u, 0x32du, stage2_delay,
              sizeof(stage2_delay));
    put_bytes(data, stage2_sector + 1u, 0x373u, stage2_port_clear,
              sizeof(stage2_port_clear));
    put_bytes(data, stage2_sector + 1u, 0x14u, stage2_pointer_setup,
              sizeof(stage2_pointer_setup));
    put_bytes(data, stage2_sector, 0xb5u, stage2_seed_tail,
              sizeof(stage2_seed_tail));
    put_bytes(data, stage2_sector, 0xf1u, stage2_dispatch_stubs,
              sizeof(stage2_dispatch_stubs));
    put_bytes(data, stage2_sector, 0x10du, stage2_jump_table,
              sizeof(stage2_jump_table));
    /* The MPR-page and selector windows live at stage-two image user
     * offsets 0xaf7/0xf5e, which map into the second raw sector of the
     * image at in-sector offsets 0x2f7/0x75e. */
    put_bytes(data, stage2_sector + 1u, 0x2f7u, stage2_mpr_page,
              sizeof(stage2_mpr_page));
    put_bytes(data, stage2_sector + 1u, 0x75eu, stage2_selector,
              sizeof(stage2_selector));
    /* The L8000 window lives at stage-two image user offset 0x4000,
     * which maps to the head of the ninth raw sector of the image;
     * L45A6 stays in the first raw sector at in-sector offset 0x5a6. */
    put_bytes(data, stage2_sector + 8u, 0x00u, stage2_l8000,
              sizeof(stage2_l8000));
    put_bytes(data, stage2_sector, 0x5a6u, stage2_l45a6,
              sizeof(stage2_l45a6));
    /* The ten jump-table handler bodies live at stage-two image user
     * offsets 0x1c5..0x254, inside the first raw sector of the image. */
    put_bytes(data, stage2_sector, 0x1c5u, stage2_jump_table_handlers,
              sizeof(stage2_jump_table_handlers));
    /* The L4696 window lives at stage-two image user offset 0x4696,
     * which maps into the ninth raw sector of the image at in-sector
     * offset 0x696; the L3114 window lives at image user offset 0x1114,
     * which maps into the third raw sector at in-sector offset 0x114. */
    put_bytes(data, stage2_sector + 8u, 0x696u, stage2_l4696,
              sizeof(stage2_l4696));
    put_bytes(data, stage2_sector + 2u, 0x114u, stage2_l3114,
              sizeof(stage2_l3114));
    /* The L3172/$117D/L5213/L526D/L55E0 windows live at stage-two image
     * user offsets 0x1172/0x117d/0x1213/0x126d/0x15e0, which map into
     * the third raw sector of the image at in-sector offsets
     * 0x172/0x17d/0x213/0x26d/0x5e0; the L4F66 window lives at image
     * user offset 0x0f66, which maps into the second raw sector at
     * in-sector offset 0x766; the two $45xx-tier L4696 call sites live
     * at image user offsets 0x45ba/0x45cb, which map into the ninth raw
     * sector at in-sector offsets 0x5ba/0x5cb. */
    put_bytes(data, stage2_sector + 2u, 0x172u, stage2_l3172,
              sizeof(stage2_l3172));
    put_bytes(data, stage2_sector + 2u, 0x17du, stage2_far117d,
              sizeof(stage2_far117d));
    put_bytes(data, stage2_sector + 1u, 0x766u, stage2_l4f66,
              sizeof(stage2_l4f66));
    put_bytes(data, stage2_sector + 2u, 0x213u, stage2_l5213,
              sizeof(stage2_l5213));
    put_bytes(data, stage2_sector + 2u, 0x26du, stage2_l526d,
              sizeof(stage2_l526d));
    put_bytes(data, stage2_sector + 2u, 0x5e0u, stage2_l55e0,
              sizeof(stage2_l55e0));
    put_bytes(data, stage2_sector + 8u, 0x5bau, stage2_l4696_call_site,
              sizeof(stage2_l4696_call_site));
    put_bytes(data, stage2_sector + 8u, 0x5cbu, stage2_l4696_call_site,
              sizeof(stage2_l4696_call_site));
    /* The L51F9/L55E8/L55F6 windows live at stage-two image user
     * offsets 0x11f9/0x15e8/0x15f6, which map into the third raw
     * sector of the image at in-sector offsets 0x1f9/0x5e8/0x5f6; the
     * L5BF5/L5C25/L5C8C/L5CB0 windows live at image user offsets
     * 0x1bf5/0x1c25/0x1c8c/0x1cb0, which map into the fourth raw
     * sector at in-sector offsets 0x3f5/0x425/0x48c/0x4b0. */
    put_bytes(data, stage2_sector + 2u, 0x1f9u, stage2_l51f9,
              sizeof(stage2_l51f9));
    put_bytes(data, stage2_sector + 2u, 0x5e8u, stage2_l55e8,
              sizeof(stage2_l55e8));
    put_bytes(data, stage2_sector + 2u, 0x5f6u, stage2_l55f6,
              sizeof(stage2_l55f6));
    put_bytes(data, stage2_sector + 3u, 0x3f5u, stage2_l5bf5,
              sizeof(stage2_l5bf5));
    put_bytes(data, stage2_sector + 3u, 0x425u, stage2_l5c25,
              sizeof(stage2_l5c25));
    put_bytes(data, stage2_sector + 3u, 0x48cu, stage2_l5c8c,
              sizeof(stage2_l5c8c));
    put_bytes(data, stage2_sector + 3u, 0x4b0u, stage2_l5cb0,
              sizeof(stage2_l5cb0));
    /* The L536E/L5439/L54A0/L5600 windows live at stage-two image user
     * offsets 0x136e/0x1439/0x14a0/0x1600, which map into the third
     * raw sector of the image at in-sector offsets
     * 0x36e/0x439/0x4a0/0x600; the L5C06/L5C20/L5C69/L5C9F windows
     * live at image user offsets 0x1c06/0x1c20/0x1c69/0x1c9f, which
     * map into the fourth raw sector at in-sector offsets
     * 0x406/0x420/0x469/0x49f. */
    put_bytes(data, stage2_sector + 2u, 0x36eu, stage2_l536e,
              sizeof(stage2_l536e));
    put_bytes(data, stage2_sector + 2u, 0x439u, stage2_l5439,
              sizeof(stage2_l5439));
    put_bytes(data, stage2_sector + 2u, 0x4a0u, stage2_l54a0,
              sizeof(stage2_l54a0));
    put_bytes(data, stage2_sector + 2u, 0x600u, stage2_l5600,
              sizeof(stage2_l5600));
    put_bytes(data, stage2_sector + 3u, 0x406u, stage2_l5c06,
              sizeof(stage2_l5c06));
    put_bytes(data, stage2_sector + 3u, 0x420u, stage2_l5c20,
              sizeof(stage2_l5c20));
    put_bytes(data, stage2_sector + 3u, 0x469u, stage2_l5c69,
              sizeof(stage2_l5c69));
    put_bytes(data, stage2_sector + 3u, 0x49fu, stage2_l5c9f,
              sizeof(stage2_l5c9f));
    /* The L4F7A window lives at stage-two image user offset 0x0f7a,
     * which maps into the second raw sector of the image at in-sector
     * offset 0x77a; the L535E/L53C4/L542D/L5455/L5482/L5492/L54AF/
     * L560B windows live at image user offsets
     * 0x135e/0x13c4/0x142d/0x1455/0x1482/0x1492/0x14af/0x160b, which
     * map into the third raw sector at in-sector offsets
     * 0x35e/0x3c4/0x42d/0x455/0x482/0x492/0x4af/0x60b. */
    put_bytes(data, stage2_sector + 1u, 0x77au, stage2_l4f7a,
              sizeof(stage2_l4f7a));
    put_bytes(data, stage2_sector + 2u, 0x35eu, stage2_l535e,
              sizeof(stage2_l535e));
    put_bytes(data, stage2_sector + 2u, 0x3c4u, stage2_l53c4,
              sizeof(stage2_l53c4));
    put_bytes(data, stage2_sector + 2u, 0x42du, stage2_l542d,
              sizeof(stage2_l542d));
    put_bytes(data, stage2_sector + 2u, 0x455u, stage2_l5455,
              sizeof(stage2_l5455));
    put_bytes(data, stage2_sector + 2u, 0x482u, stage2_l5482,
              sizeof(stage2_l5482));
    put_bytes(data, stage2_sector + 2u, 0x492u, stage2_l5492,
              sizeof(stage2_l5492));
    put_bytes(data, stage2_sector + 2u, 0x4afu, stage2_l54af,
              sizeof(stage2_l54af));
    put_bytes(data, stage2_sector + 2u, 0x60bu, stage2_l560b,
              sizeof(stage2_l560b));
    put_bytes(data, stage2_sector + 8u, 0x5b1u, stage2_45xx_routine,
              sizeof(stage2_45xx_routine));
    put_bytes(data, stage2_sector + 8u, 0x24bu, stage2_l424b,
              sizeof(stage2_l424b));
    put_bytes(data, stage2_sector + 8u, 0x3d6u, stage2_l43d6,
              sizeof(stage2_l43d6));
    put_bytes(data, stage2_sector + 8u, 0x552u, stage2_l4552,
              sizeof(stage2_l4552));
    put_bytes(data, stage2_sector + 8u, 0x58eu, stage2_l458e,
              sizeof(stage2_l458e));
    put_bytes(data, stage2_sector + 8u, 0x66bu, stage2_l466b,
              sizeof(stage2_l466b));
    put_bytes(data, stage2_sector + 9u, 0x132u, stage2_l4932,
              sizeof(stage2_l4932));
    put_bytes(data, stage2_sector + 2u, 0x403u, stage2_l5403,
              sizeof(stage2_l5403));
    put_bytes(data, stage2_sector + 2u, 0x41eu, stage2_l541e,
              sizeof(stage2_l541e));
    put_bytes(data, stage2_sector + 2u, 0x2a2u, stage2_l52a2,
              sizeof(stage2_l52a2));
    put_bytes(data, stage2_sector + 2u, 0x2c8u, stage2_l52c8,
              sizeof(stage2_l52c8));
    put_bytes(data, stage2_sector + 2u, 0x657u, stage2_l5657,
              sizeof(stage2_l5657));
    put_bytes(data, stage2_sector + 2u, 0x4c5u, stage2_l54c5,
              sizeof(stage2_l54c5));
    put_bytes(data, stage2_sector + 2u, 0x667u, stage2_l5667_data,
              sizeof(stage2_l5667_data));
    put_bytes(data, stage2_sector + 8u, 0x3a1u, stage2_l43a1,
              sizeof(stage2_l43a1));
    put_bytes(data, stage2_sector + 8u, 0x2bfu, stage2_l42bf,
              sizeof(stage2_l42bf));
    put_bytes(data, stage2_sector + 8u, 0x5a6u, stage2_gap45a6,
              sizeof(stage2_gap45a6));
    put_user(data, dynamic_sector, 0u, 0x00u);
    put_user(data, dynamic_sector, 1u, 0xffu);
    put_user(data, dynamic_sector, 2u, 0x03u);
    put_user(data, dynamic_sector, 3u, 0x08u);
    put_user(data, dynamic_sector, 4u, 0x01u);
    *out_size = sector_count * RAW_SECTOR_BYTES;
    return data;
}

static void check_receipt(const Theron_Track02IplLoaderReceipt *receipt,
                          Theron_Track02Variant variant, size_t index01,
                          size_t sectors, const char *prefix) {
    check(receipt->valid && receipt->variant == variant, prefix);
    check(receipt->data_track_index01_raw_sector == index01 &&
              receipt->information_raw_sector == index01 + 1u &&
              receipt->executable_raw_sector == index01 + THERON_TRACK02_IPL_RECORD,
          "receipt sector span");
    check(receipt->record == THERON_TRACK02_IPL_RECORD &&
              receipt->executable_sector_count == sectors &&
              receipt->executable_user_data_bytes == sectors * 2048u &&
              receipt->load_address == THERON_TRACK02_IPL_LOAD_ADDRESS &&
              receipt->entry_address == THERON_TRACK02_IPL_LOAD_ADDRESS,
          "receipt IPL record/load/entry");
    check(receipt->cd_read_user_data_offset == 0xc1u &&
              receipt->cd_read_cpu_address == THERON_TRACK02_IPL_CD_READ_CPU_ADDRESS &&
              receipt->cd_read_system_card_address ==
                  THERON_TRACK02_IPL_CD_READ_SYSTEM_CARD_ADDRESS &&
              receipt->cd_read_destination == THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM &&
              receipt->cd_read_local_destination ==
                  THERON_TRACK02_IPL_CD_READ_LOCAL_DESTINATION &&
              receipt->cd_exec_cpu_address == THERON_TRACK02_IPL_CD_EXEC_CPU_ADDRESS &&
              receipt->cd_exec_system_card_address ==
                  THERON_TRACK02_IPL_CD_EXEC_SYSTEM_CARD_ADDRESS &&
              receipt->stage2_record == THERON_TRACK02_IPL_STAGE2_RECORD &&
              receipt->stage2_sector_count == THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT &&
              receipt->stage2_destination == THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM &&
              receipt->stage2_load_address == THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS &&
              receipt->stage2_entry_address == THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS &&
              receipt->stage2_raw_sector == index01 + THERON_TRACK02_IPL_STAGE2_RECORD &&
              receipt->stage2_user_data_bytes ==
                  THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT * 2048u &&
              receipt->stage2_user_data_hash != 0u &&
              receipt->stage2_cd_read_cpu_address ==
                  THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS &&
              receipt->stage2_cd_read_sector_count == 1u &&
              receipt->stage2_cd_read_destination == THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM &&
              receipt->stage2_cd_read_local_destination ==
                  THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION &&
              receipt->stage2_cd_read_record_proven &&
              receipt->stage2_cd_read_record ==
                  (variant == THERON_TRACK02_VARIANT_JP_BIN
                       ? THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP
                       : THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US) &&
              receipt->stage2_cd_read_raw_sector == receipt->stage2_cd_read_record &&
              receipt->stage2_cd_read_dynamic_boundary_valid &&
              receipt->stage2_cd_read_live_record_register_mask ==
                  THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_MASK &&
              !receipt->vram_transfer_proven,
          "receipt binds live stage-two record while staying local and VRAM-unbound");
    check(receipt->cd_exec_retry_branch_proven &&
              receipt->cd_read_table_load_proven &&
              receipt->stage2_seed_call_sites_proven,
          "receipt proves static read-window completeness");
}

static void check_real_media(const char *path, const char *md5,
                             Theron_Track02Variant variant) {
    FILE *file;
    long length;
    uint8_t *data;
    char actual_md5[33];
    Theron_Track02IplLoaderReceipt receipt;
    Theron_Track02Stage2DynamicPayloadReceipt dynamic_payload;
    Theron_V1Stage2RuntimeHandoff runtime_handoff;

    if (!path) return;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        printf("SKIP real IPL media unreadable: %s\n", path);
        return;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        ++g_failures;
        printf("FAIL real IPL media read\n");
        return;
    }
    fclose(file);
    check(m12_file_md5_hex(path, actual_md5) && strcmp(actual_md5, md5) == 0,
          "real IPL media MD5");
    check(theron_v1_track02_find_ipl_loader(data, (size_t)length, md5, &receipt) ==
              THERON_TRACK02_SIGNAL_OK,
          "real IPL loader receipt");
    check(receipt.variant == variant && receipt.executable_user_data_hash != 0u,
          "real IPL loader identity/hash");
    check(theron_v1_track02_inspect_stage2_dynamic_payload(
              data, (size_t)length, md5, &dynamic_payload) == THERON_TRACK02_SIGNAL_OK &&
              dynamic_payload.valid && dynamic_payload.variant == variant &&
              dynamic_payload.track02_record == receipt.stage2_cd_read_record &&
              dynamic_payload.raw_sector == receipt.stage2_cd_read_raw_sector &&
              dynamic_payload.user_data_bytes ==
                  THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES &&
              dynamic_payload.header_word0 == 0x00ffu &&
              dynamic_payload.header_word1 == 0x0308u &&
              dynamic_payload.manifest_bytes ==
                  THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES &&
              dynamic_payload.manifest_entry_count ==
                  THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT &&
              dynamic_payload.nonzero_byte_count > 0u && dynamic_payload.user_data_hash != 0u,
          "real Track 02 dynamic payload manifest receipt");
    check(theron_v1_stage2_runtime_handoff_from_dynamic_payload(
              &dynamic_payload, &runtime_handoff) && runtime_handoff.valid &&
              runtime_handoff.variant == variant &&
              runtime_handoff.track02_record == dynamic_payload.track02_record &&
              runtime_handoff.load_address == 0x3800u &&
              runtime_handoff.entry_address == 0x3800u &&
              runtime_handoff.execute_after_load &&
              runtime_handoff.manifest_entries_semantically_unbound &&
              runtime_handoff.user_data_hash == dynamic_payload.user_data_hash,
          "real Track 02 dynamic record binds only a stage-three executable handoff");
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        Theron_Track02Stage2EntryPathReceipt entry_path;
        Theron_Track02Stage2CallGraphReceipt call_graph;
        Theron_Track02Stage2DispatchMachineReceipt dispatch_machine;
        Theron_Track02Stage2L8000PairReceipt l8000_pair;

        check(theron_v1_track02_verify_stage2_entry_path(
                  data, (size_t)length, md5, &entry_path) ==
                  THERON_TRACK02_SIGNAL_OK &&
                  entry_path.valid &&
                  entry_path.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  entry_path.stage2_raw_sector == receipt.stage2_raw_sector &&
                  entry_path.entry_path_prologue_bytes == 0x29u &&
                  entry_path.entry_path_main_path_bytes == 0x52u &&
                  entry_path.entry_path_bound_bytes == 0xb5u &&
                  entry_path.entry_prologue_proven &&
                  entry_path.main_path_proven &&
                  entry_path.entry_path_contiguous_proven,
              "real US stage-two entry path is contiguously byte-bound");
        check(theron_v1_track02_verify_stage2_call_graph(
                  data, (size_t)length, md5, &call_graph) ==
                  THERON_TRACK02_SIGNAL_OK &&
                  call_graph.valid &&
                  call_graph.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  call_graph.stage2_raw_sector == receipt.stage2_raw_sector &&
                  call_graph.dispatcher_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES &&
                  call_graph.delay_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DELAY_BYTES &&
                  call_graph.port_clear_bytes ==
                      THERON_TRACK02_IPL_STAGE2_PORT_CLEAR_BYTES &&
                  call_graph.pointer_setup_bytes ==
                      THERON_TRACK02_IPL_STAGE2_POINTER_SETUP_BYTES &&
                  call_graph.call_graph_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES &&
                  call_graph.dispatcher_proven &&
                  call_graph.delay_proven &&
                  call_graph.port_clear_proven &&
                  call_graph.pointer_setup_proven,
              "real US stage-two call-graph continuations are byte-bound");
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, (size_t)length, md5, &dispatch_machine) ==
                  THERON_TRACK02_SIGNAL_OK &&
                  dispatch_machine.valid &&
                  dispatch_machine.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  dispatch_machine.stage2_raw_sector ==
                      receipt.stage2_raw_sector &&
                  dispatch_machine.seed_tail_bytes ==
                      THERON_TRACK02_IPL_STAGE2_SEED_TAIL_BYTES &&
                  dispatch_machine.dispatch_stubs_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_BYTES &&
                  dispatch_machine.jump_table_bytes ==
                      THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_BYTES &&
                  dispatch_machine.jump_table_entries ==
                      THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES &&
                  dispatch_machine.mpr_page_bytes ==
                      THERON_TRACK02_IPL_STAGE2_MPR_PAGE_BYTES &&
                  dispatch_machine.selector_bytes ==
                      THERON_TRACK02_IPL_STAGE2_SELECTOR_BYTES &&
                  dispatch_machine.loop_closure_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_LOOP_CLOSURE_BOUND_BYTES &&
                  dispatch_machine.dispatch_machine_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DISPATCH_MACHINE_BOUND_BYTES &&
                  dispatch_machine.seed_tail_proven &&
                  dispatch_machine.dispatch_stubs_proven &&
                  dispatch_machine.jump_table_proven &&
                  dispatch_machine.mpr_page_proven &&
                  dispatch_machine.selector_proven &&
                  dispatch_machine.dispatch_machine_contiguous_proven,
              "real US stage-two dispatch machine is contiguously byte-bound");
        check(theron_v1_track02_verify_stage2_l8000_pair(
                  data, (size_t)length, md5, &l8000_pair) ==
                  THERON_TRACK02_SIGNAL_OK &&
                  l8000_pair.valid &&
                  l8000_pair.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  l8000_pair.stage2_raw_sector ==
                      receipt.stage2_raw_sector &&
                  l8000_pair.l8000_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L8000_BYTES &&
                  l8000_pair.l45a6_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L45A6_BYTES &&
                  l8000_pair.pair_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES &&
                  l8000_pair.l8000_proven &&
                  l8000_pair.l45a6_proven &&
                  l8000_pair.l8000_call_site_proven &&
                  l8000_pair.l45a6_single_caller_proven,
              "real US stage-two L8000/L45A6 callee pair is byte-bound");
        {
            Theron_Track02Stage2JumpTableHandlersReceipt handlers;
            check(theron_v1_track02_verify_stage2_jump_table_handlers(
                      data, (size_t)length, md5, &handlers) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      handlers.valid &&
                      handlers.variant == THERON_TRACK02_VARIANT_US_BIN &&
                      handlers.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      handlers.handlers_bytes ==
                          THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES &&
                      handlers.handler_count ==
                          THERON_TRACK02_IPL_STAGE2_HANDLER_COUNT &&
                      handlers.first_handler_cpu_address ==
                          THERON_TRACK02_IPL_STAGE2_HANDLERS_FIRST_CPU_ADDRESS &&
                      handlers.last_handler_cpu_address ==
                          THERON_TRACK02_IPL_STAGE2_HANDLERS_LAST_CPU_ADDRESS &&
                      handlers.handlers_proven &&
                      handlers.handler_entry_chain_proven &&
                      handlers.handlers_contiguous_proven,
                  "real US stage-two jump-table handler bodies are byte-bound");
        }
        {
            Theron_Track02Stage2L4696L3114Receipt far_callees;
            check(theron_v1_track02_verify_stage2_l4696_l3114(
                      data, (size_t)length, md5, &far_callees) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      far_callees.valid &&
                      far_callees.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      far_callees.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      far_callees.l4696_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L4696_BYTES &&
                      far_callees.l3114_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L3114_BYTES &&
                      far_callees.l4696_l3114_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES &&
                      far_callees.l4696_cpu_address ==
                          THERON_TRACK02_IPL_STAGE2_L4696_CPU_ADDRESS &&
                      far_callees.l3114_cpu_address ==
                          THERON_TRACK02_IPL_STAGE2_L3114_CPU_ADDRESS &&
                      far_callees.l4696_proven &&
                      far_callees.l3114_proven &&
                      far_callees.l4696_call_site_proven &&
                      far_callees.l3114_call_site_proven,
                  "real US stage-two L4696/L3114 far-callee bodies are byte-bound");
        }
        {
            Theron_Track02Stage2L3114CalleesReceipt l3114_callees;
            check(theron_v1_track02_verify_stage2_l3114_callees(
                      data, (size_t)length, md5, &l3114_callees) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      l3114_callees.valid &&
                      l3114_callees.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      l3114_callees.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      l3114_callees.l3114_callees_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES &&
                      l3114_callees.l3172_proven &&
                      l3114_callees.far117d_proven &&
                      l3114_callees.l4f66_proven &&
                      l3114_callees.l5213_proven &&
                      l3114_callees.l526d_proven &&
                      l3114_callees.l55e0_proven &&
                      l3114_callees.l3114_call_sites_proven &&
                      l3114_callees.l4696_call_sites_45xx_proven,
                  "real US stage-two L3114 callee bodies are byte-bound");
        }
        {
            Theron_Track02Stage2L3114Tier2CalleesReceipt tier2_callees;
            check(theron_v1_track02_verify_stage2_l3114_tier2_callees(
                      data, (size_t)length, md5, &tier2_callees) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      tier2_callees.valid &&
                      tier2_callees.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      tier2_callees.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      tier2_callees.tier2_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES &&
                      tier2_callees.l51f9_proven &&
                      tier2_callees.l55e8_proven &&
                      tier2_callees.l55f6_proven &&
                      tier2_callees.l5bf5_proven &&
                      tier2_callees.l5c25_proven &&
                      tier2_callees.l5c8c_proven &&
                      tier2_callees.l5cb0_proven &&
                      tier2_callees.far117d_call_sites_proven &&
                      tier2_callees.l526d_call_site_proven &&
                      tier2_callees.l55e0_call_sites_proven,
                  "real US stage-two L3114 tier-2 callee bodies are byte-bound");
        }
        {
            Theron_Track02Stage2L3114Tier3CalleesReceipt tier3_callees;
            check(theron_v1_track02_verify_stage2_l3114_tier3_callees(
                      data, (size_t)length, md5, &tier3_callees) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      tier3_callees.valid &&
                      tier3_callees.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      tier3_callees.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      tier3_callees.tier3_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES &&
                      tier3_callees.l5c06_proven &&
                      tier3_callees.l5c20_proven &&
                      tier3_callees.l5c69_proven &&
                      tier3_callees.l5c9f_proven &&
                      tier3_callees.l536e_proven &&
                      tier3_callees.l5439_proven &&
                      tier3_callees.l54a0_proven &&
                      tier3_callees.l5600_proven &&
                      tier3_callees.l5c2c_entry_proven &&
                      tier3_callees.l5c8c_call_sites_proven &&
                      tier3_callees.l5c25_call_sites_proven &&
                      tier3_callees.l55f6_call_sites_proven &&
                      tier3_callees.l5bf5_data_site_proven,
                  "real US stage-two L3114 tier-3 callee bodies are byte-bound");
        }
        {
            Theron_Track02Stage2L3114Tier4CalleesReceipt tier4_callees;
            check(theron_v1_track02_verify_stage2_l3114_tier4_callees(
                      data, (size_t)length, md5, &tier4_callees) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      tier4_callees.valid &&
                      tier4_callees.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      tier4_callees.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      tier4_callees.tier4_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES &&
                      tier4_callees.l4f7a_proven &&
                      tier4_callees.l535e_proven &&
                      tier4_callees.l53c4_proven &&
                      tier4_callees.l542d_proven &&
                      tier4_callees.l5455_proven &&
                      tier4_callees.l5482_proven &&
                      tier4_callees.l5492_proven &&
                      tier4_callees.l54af_proven &&
                      tier4_callees.l560b_proven &&
                      tier4_callees.l5c9f_call_site_proven &&
                      tier4_callees.l5c69_call_site_proven &&
                      tier4_callees.tier4_chain_contiguous_proven,
                  "real US stage-two L3114 tier-4 callee bodies are byte-bound");
        }
        {
            Theron_Track02Stage2Enclosing45xxReceipt enclosing_45xx;
            check(theron_v1_track02_verify_stage2_enclosing_45xx(
                      data, (size_t)length, md5, &enclosing_45xx) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      enclosing_45xx.valid &&
                      enclosing_45xx.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      enclosing_45xx.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      enclosing_45xx.routine_bytes ==
                          THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES &&
                      enclosing_45xx.routine_proven &&
                      enclosing_45xx.l4696_call_sites_within_proven,
                  "real US stage-two enclosing $45xx routine is byte-bound");
        }
        {
            Theron_Track02Stage2Enclosing45xxCalleesReceipt callees_45xx;
            check(theron_v1_track02_verify_stage2_enclosing_45xx_callees(
                      data, (size_t)length, md5, &callees_45xx) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      callees_45xx.valid &&
                      callees_45xx.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      callees_45xx.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      callees_45xx.callees_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_45XX_CALLEES_BOUND_BYTES &&
                      callees_45xx.l424b_proven &&
                      callees_45xx.l43d6_proven &&
                      callees_45xx.l4552_proven &&
                      callees_45xx.l458e_proven &&
                      callees_45xx.l466b_proven &&
                      callees_45xx.l4932_proven &&
                      callees_45xx.l424b_call_site_proven &&
                      callees_45xx.adjacency_proven,
                  "real US stage-two $45xx callee bodies are byte-bound");
        }
        {
            Theron_Track02Stage2L3114Tier5CalleesReceipt tier5_callees;
            check(theron_v1_track02_verify_stage2_l3114_tier5_callees(
                      data, (size_t)length, md5, &tier5_callees) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      tier5_callees.valid &&
                      tier5_callees.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      tier5_callees.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      tier5_callees.tier5_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES &&
                      tier5_callees.l5403_proven &&
                      tier5_callees.l541e_proven &&
                      tier5_callees.l52a2_proven &&
                      tier5_callees.l52c8_proven &&
                      tier5_callees.l5657_proven &&
                      tier5_callees.l54c5_proven &&
                      tier5_callees.l5667_proven &&
                      tier5_callees.l53c4_call_site_proven &&
                      tier5_callees.l560b_call_sites_proven &&
                      tier5_callees.tier5_chain_contiguous_proven,
                  "real US stage-two L3114 tier-5 callee bodies are byte-bound");
        }
        {
            Theron_Track02Stage245xxTier2CalleesReceipt tier2_45xx;
            check(theron_v1_track02_verify_stage2_45xx_tier2_callees(
                      data, (size_t)length, md5, &tier2_45xx) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      tier2_45xx.valid &&
                      tier2_45xx.variant ==
                          THERON_TRACK02_VARIANT_US_BIN &&
                      tier2_45xx.stage2_raw_sector ==
                          receipt.stage2_raw_sector &&
                      tier2_45xx.tier2_bound_bytes ==
                          THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES &&
                      tier2_45xx.l43a1_proven &&
                      tier2_45xx.l42bf_proven &&
                      tier2_45xx.gap45a6_proven &&
                      tier2_45xx.l424b_call_sites_proven &&
                      tier2_45xx.adjacency_proven,
                  "real US stage-two $45xx tier-2 windows are byte-bound");
        }
    }
    free(data);
}

static void check_real_cue_boot_handoff(const char *cue_path, const char *md5,
                                        size_t index01_raw_sector) {
    char payload[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    FILE *file;
    long length;
    size_t required;
    uint8_t *data;
    Theron_Track02StartupLoaderReceipt receipt;

    if (!cue_path || !cue_path[0]) {
        printf("SKIP real CUE boot handoff: no CUE staged\n");
        return;
    }
    if (theron_v1_track02_resolve_media_path(cue_path, payload) !=
        THERON_TRACK02_SIGNAL_OK) {
        ++g_failures;
        printf("FAIL real CUE payload resolve\n");
        return;
    }
    required = (index01_raw_sector +
                (size_t)THERON_TRACK02_IPL_STAGE2_RECORD +
                (size_t)THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT) *
        RAW_SECTOR_BYTES;
    file = fopen(payload, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || (size_t)length < required ||
        fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        ++g_failures;
        printf("FAIL real CUE payload read\n");
        return;
    }
    data = (uint8_t *)malloc(required);
    if (!data || fread(data, 1u, required, file) != required) {
        free(data);
        fclose(file);
        ++g_failures;
        printf("FAIL real CUE payload allocation/read\n");
        return;
    }
    fclose(file);
    memset(&receipt, 0, sizeof(receipt));
    check(theron_v1_track02_find_ipl_loader(data, required, md5,
                                             &receipt.ipl_loader) ==
              THERON_TRACK02_SIGNAL_OK,
          "real CUE scanner IPL receipt");
    receipt.valid = 1;
    receipt.cue_backed = 1;
    receipt.track02_md5_verified = 1;
    receipt.mode1_2352 = 1;
    receipt.no_synthetic_cache = 1;
    snprintf(receipt.cue_path, sizeof(receipt.cue_path), "%s", cue_path);
    snprintf(receipt.track02_path, sizeof(receipt.track02_path), "%s", payload);
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", md5);
    check(theron_v1_boot_validate_track02_loader_receipt(&receipt, md5),
          "real CUE M11 handoff revalidates the traced dynamic record payload");
    free(data);
}

int main(void) {
    uint8_t *data;
    size_t data_size;
    Theron_Track02IplLoaderReceipt receipt;
    Theron_Track02Stage2DynamicPayloadReceipt dynamic_payload;
    Theron_Track02Stage2EntryPathReceipt entry_path;
    Theron_Track02Stage2CallGraphReceipt call_graph;
    Theron_Track02Stage2DispatchMachineReceipt dispatch_machine;
    Theron_Track02Stage2L8000PairReceipt l8000_pair;
    Theron_Track02Stage2JumpTableHandlersReceipt jump_table_handlers;
    Theron_Track02Stage2L4696L3114Receipt l4696_l3114;
    Theron_Track02Stage2L3114CalleesReceipt l3114_callees;
    Theron_Track02Stage2L3114Tier2CalleesReceipt tier2_callees;
    Theron_Track02Stage2L3114Tier3CalleesReceipt tier3_callees;
    Theron_Track02Stage2L3114Tier4CalleesReceipt tier4_callees;
    Theron_Track02Stage2Enclosing45xxReceipt enclosing_45xx;
    Theron_Track02Stage2Enclosing45xxCalleesReceipt callees_45xx;
    Theron_Track02Stage2L3114Tier5CalleesReceipt tier5_callees;
    Theron_Track02Stage245xxTier2CalleesReceipt tier2_45xx;

    data = make_fixture(225u, 4u, &data_size);
    check(data != NULL, "US IPL fixture allocation");
    if (data) {
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_OK,
              "US IPL fixture accepted");
        check_receipt(&receipt, THERON_TRACK02_VARIANT_US_BIN, 225u, 4u,
                      "US IPL fixture identity");
        check(theron_v1_track02_inspect_stage2_dynamic_payload(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dynamic_payload) == THERON_TRACK02_SIGNAL_OK &&
                  dynamic_payload.track02_record ==
                      THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US &&
                  dynamic_payload.manifest_entry_count ==
                      THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT,
              "US dynamic payload fixture accepted");
        put_user(data, THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US,
                 THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES, 0x01u);
        check(theron_v1_track02_inspect_stage2_dynamic_payload(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dynamic_payload) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "dynamic payload tail rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_RECORD, 0xa7u, 0x00u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing CD_EXEC retry branch rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_RECORD, 0xa7u, 0x80u);
        put_user(data, 225u + THERON_TRACK02_IPL_RECORD, 0xa9u, 0x00u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing CD_READ table load rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_RECORD, 0xa9u, 0x82u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x29u, 0x00u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing stage-two seed JSR rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x29u, 0x20u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x7eu, 0x00u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing stage-two seed BSR rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x7eu, 0x44u);
        check(theron_v1_track02_verify_stage2_entry_path(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &entry_path) == THERON_TRACK02_SIGNAL_OK &&
                  entry_path.valid &&
                  entry_path.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  entry_path.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  entry_path.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  entry_path.entry_path_prologue_bytes == 0x29u &&
                  entry_path.entry_path_main_path_bytes == 0x52u &&
                  entry_path.entry_path_bound_bytes == 0xb5u &&
                  entry_path.entry_prologue_proven &&
                  entry_path.main_path_proven &&
                  entry_path.entry_path_contiguous_proven,
              "US stage-two entry path proves [0x00..0xb5) contiguity");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x00u, 0x00u);
        check(theron_v1_track02_verify_stage2_entry_path(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &entry_path) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed entry prologue byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x00u, 0x78u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x2cu, 0x00u);
        check(theron_v1_track02_verify_stage2_entry_path(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &entry_path) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed main path byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x2cu, 0x20u);
        check(theron_v1_track02_verify_stage2_call_graph(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &call_graph) == THERON_TRACK02_SIGNAL_OK &&
                  call_graph.valid &&
                  call_graph.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  call_graph.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  call_graph.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  call_graph.dispatcher_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES &&
                  call_graph.delay_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DELAY_BYTES &&
                  call_graph.port_clear_bytes ==
                      THERON_TRACK02_IPL_STAGE2_PORT_CLEAR_BYTES &&
                  call_graph.pointer_setup_bytes ==
                      THERON_TRACK02_IPL_STAGE2_POINTER_SETUP_BYTES &&
                  call_graph.call_graph_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES &&
                  call_graph.dispatcher_proven &&
                  call_graph.delay_proven &&
                  call_graph.port_clear_proven &&
                  call_graph.pointer_setup_proven,
              "US stage-two call-graph continuations prove four callee bodies");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0xb7u, 0x00u);
        check(theron_v1_track02_verify_stage2_call_graph(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &call_graph) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed dispatcher byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0xb7u, 0x64u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x32du,
                 0x00u);
        check(theron_v1_track02_verify_stage2_call_graph(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &call_graph) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed delay byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x32du,
                 0x48u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x373u,
                 0x00u);
        check(theron_v1_track02_verify_stage2_call_graph(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &call_graph) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed port clear byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x373u,
                 0x78u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x14u,
                 0x00u);
        check(theron_v1_track02_verify_stage2_call_graph(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &call_graph) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed pointer setup byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x14u,
                 0xa9u);
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dispatch_machine) == THERON_TRACK02_SIGNAL_OK &&
                  dispatch_machine.valid &&
                  dispatch_machine.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  dispatch_machine.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  dispatch_machine.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  dispatch_machine.seed_tail_bytes ==
                      THERON_TRACK02_IPL_STAGE2_SEED_TAIL_BYTES &&
                  dispatch_machine.dispatch_stubs_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_BYTES &&
                  dispatch_machine.jump_table_bytes ==
                      THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_BYTES &&
                  dispatch_machine.jump_table_entries ==
                      THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES &&
                  dispatch_machine.mpr_page_bytes ==
                      THERON_TRACK02_IPL_STAGE2_MPR_PAGE_BYTES &&
                  dispatch_machine.selector_bytes ==
                      THERON_TRACK02_IPL_STAGE2_SELECTOR_BYTES &&
                  dispatch_machine.loop_closure_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_LOOP_CLOSURE_BOUND_BYTES &&
                  dispatch_machine.dispatch_machine_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_DISPATCH_MACHINE_BOUND_BYTES &&
                  dispatch_machine.seed_tail_proven &&
                  dispatch_machine.dispatch_stubs_proven &&
                  dispatch_machine.jump_table_proven &&
                  dispatch_machine.mpr_page_proven &&
                  dispatch_machine.selector_proven &&
                  dispatch_machine.dispatch_machine_contiguous_proven,
              "US stage-two dispatch machine proves [0x00..0x121) contiguity");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0xb5u, 0x00u);
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dispatch_machine) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed seed tail byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0xb5u, 0xfcu);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0xf1u, 0x00u);
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dispatch_machine) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed dispatch stub byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0xf1u, 0xa9u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x10du, 0x00u);
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dispatch_machine) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed jump table byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x10du, 0xc5u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x2f7u,
                 0x00u);
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dispatch_machine) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed MPR page byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x2f7u,
                 0x18u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x75eu,
                 0x00u);
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &dispatch_machine) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed selector byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u, 0x75eu,
                 0xa2u);
        check(theron_v1_track02_verify_stage2_l8000_pair(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l8000_pair) == THERON_TRACK02_SIGNAL_OK &&
                  l8000_pair.valid &&
                  l8000_pair.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  l8000_pair.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  l8000_pair.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  l8000_pair.l8000_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L8000_BYTES &&
                  l8000_pair.l45a6_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L45A6_BYTES &&
                  l8000_pair.pair_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES &&
                  l8000_pair.l8000_proven &&
                  l8000_pair.l45a6_proven &&
                  l8000_pair.l8000_call_site_proven &&
                  l8000_pair.l45a6_single_caller_proven,
              "US stage-two L8000/L45A6 callee pair is byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u, 0x00u,
                 0x00u);
        check(theron_v1_track02_verify_stage2_l8000_pair(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l8000_pair) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L8000 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u, 0x00u,
                 0xc6u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u, 0x0bu,
                 0x00u);
        check(theron_v1_track02_verify_stage2_l8000_pair(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l8000_pair) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L8000 decode-artifact byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u, 0x0bu,
                 0x9cu);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x5a6u,
                 0x00u);
        check(theron_v1_track02_verify_stage2_l8000_pair(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l8000_pair) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L45A6 byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x5a6u,
                 0xb1u);
        check(theron_v1_track02_verify_stage2_jump_table_handlers(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &jump_table_handlers) == THERON_TRACK02_SIGNAL_OK &&
                  jump_table_handlers.valid &&
                  jump_table_handlers.variant ==
                      THERON_TRACK02_VARIANT_US_BIN &&
                  jump_table_handlers.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  jump_table_handlers.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  jump_table_handlers.handlers_bytes ==
                      THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES &&
                  jump_table_handlers.handler_count ==
                      THERON_TRACK02_IPL_STAGE2_HANDLER_COUNT &&
                  jump_table_handlers.first_handler_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_HANDLERS_FIRST_CPU_ADDRESS &&
                  jump_table_handlers.last_handler_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_HANDLERS_LAST_CPU_ADDRESS &&
                  jump_table_handlers.handlers_proven &&
                  jump_table_handlers.handler_entry_chain_proven &&
                  jump_table_handlers.handlers_contiguous_proven,
              "US stage-two jump-table handler bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x1c5u,
                 0x00u);
        check(theron_v1_track02_verify_stage2_jump_table_handlers(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &jump_table_handlers) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed first handler head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x1c5u,
                 0x44u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x1fcu,
                 0x00u);
        check(theron_v1_track02_verify_stage2_jump_table_handlers(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &jump_table_handlers) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed handler decode-artifact byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x1fcu,
                 0xbdu);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x253u,
                 0x00u);
        check(theron_v1_track02_verify_stage2_jump_table_handlers(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &jump_table_handlers) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed last handler byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD, 0x253u,
                 0x60u);
        check(theron_v1_track02_verify_stage2_l4696_l3114(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l4696_l3114) == THERON_TRACK02_SIGNAL_OK &&
                  l4696_l3114.valid &&
                  l4696_l3114.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  l4696_l3114.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  l4696_l3114.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  l4696_l3114.l4696_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L4696_BYTES &&
                  l4696_l3114.l3114_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_BYTES &&
                  l4696_l3114.l4696_l3114_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES &&
                  l4696_l3114.l4696_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L4696_CPU_ADDRESS &&
                  l4696_l3114.l3114_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CPU_ADDRESS &&
                  l4696_l3114.l4696_proven &&
                  l4696_l3114.l3114_proven &&
                  l4696_l3114.l4696_call_site_proven &&
                  l4696_l3114.l3114_call_site_proven,
              "US stage-two L4696/L3114 far-callee bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x696u, 0x00u);
        check(theron_v1_track02_verify_stage2_l4696_l3114(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l4696_l3114) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L4696 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x696u, 0x64u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x698u, 0x00u);
        check(theron_v1_track02_verify_stage2_l4696_l3114(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l4696_l3114) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L4696 zero-page-artifact byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x698u, 0x64u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x114u, 0x00u);
        check(theron_v1_track02_verify_stage2_l4696_l3114(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l4696_l3114) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L3114 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x114u, 0x48u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x171u, 0x00u);
        check(theron_v1_track02_verify_stage2_l4696_l3114(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l4696_l3114) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L3114 tail byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x171u, 0x60u);
        check(theron_v1_track02_verify_stage2_l3114_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l3114_callees) == THERON_TRACK02_SIGNAL_OK &&
                  l3114_callees.valid &&
                  l3114_callees.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  l3114_callees.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  l3114_callees.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  l3114_callees.l3172_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_BYTES &&
                  l3114_callees.far117d_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES &&
                  l3114_callees.l4f66_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_BYTES &&
                  l3114_callees.l5213_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_BYTES &&
                  l3114_callees.l526d_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_BYTES &&
                  l3114_callees.l55e0_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES &&
                  l3114_callees.l3114_callees_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES &&
                  l3114_callees.l4f66_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_CPU_ADDRESS &&
                  l3114_callees.l5213_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_CPU_ADDRESS &&
                  l3114_callees.l526d_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_CPU_ADDRESS &&
                  l3114_callees.l55e0_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_CPU_ADDRESS &&
                  l3114_callees.l3172_proven &&
                  l3114_callees.far117d_proven &&
                  l3114_callees.l4f66_proven &&
                  l3114_callees.l5213_proven &&
                  l3114_callees.l526d_proven &&
                  l3114_callees.l55e0_proven &&
                  l3114_callees.l3114_call_sites_proven &&
                  l3114_callees.l4696_call_sites_45xx_proven,
              "US stage-two L3114 callee bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x172u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l3114_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L3172 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x172u, 0xa9u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u,
                 0x766u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l3114_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L4F66 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u,
                 0x766u, 0x48u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x5e0u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l3114_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L55E0 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x5e0u, 0x44u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x5bau, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &l3114_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed $45xx-tier L4696 call-site byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x5bau, 0x20u);
        check(theron_v1_track02_verify_stage2_l3114_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_callees) == THERON_TRACK02_SIGNAL_OK &&
                  tier2_callees.valid &&
                  tier2_callees.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  tier2_callees.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier2_callees.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier2_callees.l51f9_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_BYTES &&
                  tier2_callees.l55e8_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_BYTES &&
                  tier2_callees.l55f6_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_BYTES &&
                  tier2_callees.l5bf5_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_BYTES &&
                  tier2_callees.l5c25_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES &&
                  tier2_callees.l5c8c_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_BYTES &&
                  tier2_callees.l5cb0_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_BYTES &&
                  tier2_callees.tier2_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES &&
                  tier2_callees.l51f9_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_CPU_ADDRESS &&
                  tier2_callees.l55e8_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_CPU_ADDRESS &&
                  tier2_callees.l55f6_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_CPU_ADDRESS &&
                  tier2_callees.l5bf5_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_CPU_ADDRESS &&
                  tier2_callees.l5c25_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_CPU_ADDRESS &&
                  tier2_callees.l5c8c_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_CPU_ADDRESS &&
                  tier2_callees.l5cb0_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_CPU_ADDRESS &&
                  tier2_callees.l51f9_proven &&
                  tier2_callees.l55e8_proven &&
                  tier2_callees.l55f6_proven &&
                  tier2_callees.l5bf5_proven &&
                  tier2_callees.l5c25_proven &&
                  tier2_callees.l5c8c_proven &&
                  tier2_callees.l5cb0_proven &&
                  tier2_callees.far117d_call_sites_proven &&
                  tier2_callees.l526d_call_site_proven &&
                  tier2_callees.l55e0_call_sites_proven,
              "US stage-two L3114 tier-2 callee bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x1f9u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L51F9 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x1f9u, 0x64u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x5f6u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L55F6 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x5f6u, 0xc6u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x425u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L5C25 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x425u, 0xa9u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x48cu, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L5C8C head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x48cu, 0x20u);
        check(theron_v1_track02_verify_stage2_l3114_tier3_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier3_callees) == THERON_TRACK02_SIGNAL_OK &&
                  tier3_callees.valid &&
                  tier3_callees.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  tier3_callees.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier3_callees.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier3_callees.l5c06_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_BYTES &&
                  tier3_callees.l5c20_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_BYTES &&
                  tier3_callees.l5c69_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_BYTES &&
                  tier3_callees.l5c9f_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES &&
                  tier3_callees.l536e_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_BYTES &&
                  tier3_callees.l5439_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_BYTES &&
                  tier3_callees.l54a0_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_BYTES &&
                  tier3_callees.l5600_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_BYTES &&
                  tier3_callees.tier3_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES &&
                  tier3_callees.l5c06_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_CPU_ADDRESS &&
                  tier3_callees.l5c20_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_CPU_ADDRESS &&
                  tier3_callees.l5c69_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_CPU_ADDRESS &&
                  tier3_callees.l5c9f_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_CPU_ADDRESS &&
                  tier3_callees.l536e_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_CPU_ADDRESS &&
                  tier3_callees.l5439_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_CPU_ADDRESS &&
                  tier3_callees.l54a0_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_CPU_ADDRESS &&
                  tier3_callees.l5600_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_CPU_ADDRESS &&
                  tier3_callees.l5c06_proven &&
                  tier3_callees.l5c20_proven &&
                  tier3_callees.l5c69_proven &&
                  tier3_callees.l5c9f_proven &&
                  tier3_callees.l536e_proven &&
                  tier3_callees.l5439_proven &&
                  tier3_callees.l54a0_proven &&
                  tier3_callees.l5600_proven &&
                  tier3_callees.l5c2c_entry_proven &&
                  tier3_callees.l5c8c_call_sites_proven &&
                  tier3_callees.l5c25_call_sites_proven &&
                  tier3_callees.l55f6_call_sites_proven &&
                  tier3_callees.l5bf5_data_site_proven,
              "US stage-two L3114 tier-3 callee bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x36eu, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier3_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier3_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L536E head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x36eu, 0xadu);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x406u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier3_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier3_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L5C06 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x406u, 0xceu);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x4a0u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier3_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier3_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L54A0 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x4a0u, 0x03u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x420u, 0xffu);
        check(theron_v1_track02_verify_stage2_l3114_tier3_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier3_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L5C20 table byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 3u,
                 0x420u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier4_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier4_callees) == THERON_TRACK02_SIGNAL_OK &&
                  tier4_callees.valid &&
                  tier4_callees.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  tier4_callees.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier4_callees.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier4_callees.l4f7a_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_BYTES &&
                  tier4_callees.l535e_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_BYTES &&
                  tier4_callees.l53c4_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_BYTES &&
                  tier4_callees.l542d_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_BYTES &&
                  tier4_callees.l5455_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_BYTES &&
                  tier4_callees.l5482_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_BYTES &&
                  tier4_callees.l5492_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_BYTES &&
                  tier4_callees.l54af_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_BYTES &&
                  tier4_callees.l560b_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES &&
                  tier4_callees.tier4_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES &&
                  tier4_callees.l4f7a_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_CPU_ADDRESS &&
                  tier4_callees.l535e_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_CPU_ADDRESS &&
                  tier4_callees.l53c4_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_CPU_ADDRESS &&
                  tier4_callees.l542d_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_CPU_ADDRESS &&
                  tier4_callees.l5455_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_CPU_ADDRESS &&
                  tier4_callees.l5482_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_CPU_ADDRESS &&
                  tier4_callees.l5492_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_CPU_ADDRESS &&
                  tier4_callees.l54af_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_CPU_ADDRESS &&
                  tier4_callees.l560b_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_CPU_ADDRESS &&
                  tier4_callees.l4f7a_proven &&
                  tier4_callees.l535e_proven &&
                  tier4_callees.l53c4_proven &&
                  tier4_callees.l542d_proven &&
                  tier4_callees.l5455_proven &&
                  tier4_callees.l5482_proven &&
                  tier4_callees.l5492_proven &&
                  tier4_callees.l54af_proven &&
                  tier4_callees.l560b_proven &&
                  tier4_callees.l5c9f_call_site_proven &&
                  tier4_callees.l5c69_call_site_proven &&
                  tier4_callees.tier4_chain_contiguous_proven,
              "US stage-two L3114 tier-4 callee bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u,
                 0x77au, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier4_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier4_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L4F7A head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 1u,
                 0x77au, 0xdau);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x3c4u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier4_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier4_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L53C4 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x3c4u, 0x20u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x455u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier4_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier4_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L5455 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x455u, 0xa5u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x60bu, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier4_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier4_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L560B head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x60bu, 0x18u);
        check(theron_v1_track02_verify_stage2_enclosing_45xx(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &enclosing_45xx) == THERON_TRACK02_SIGNAL_OK &&
                  enclosing_45xx.valid &&
                  enclosing_45xx.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  enclosing_45xx.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  enclosing_45xx.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  enclosing_45xx.routine_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES &&
                  enclosing_45xx.routine_cpu_address == 0u &&
                  enclosing_45xx.routine_proven &&
                  enclosing_45xx.l4696_call_sites_within_proven,
              "US stage-two enclosing $45xx routine is byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x5b1u, 0x00u);
        check(theron_v1_track02_verify_stage2_enclosing_45xx(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &enclosing_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed $45xx routine head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x5b1u, 0xa5u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x667u, 0x00u);
        check(theron_v1_track02_verify_stage2_enclosing_45xx(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &enclosing_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed $45xx routine loop-back branch rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x667u, 0xd0u);
        check(theron_v1_track02_verify_stage2_enclosing_45xx_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &callees_45xx) == THERON_TRACK02_SIGNAL_OK &&
                  callees_45xx.valid &&
                  callees_45xx.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  callees_45xx.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  callees_45xx.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  callees_45xx.l424b_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES &&
                  callees_45xx.l43d6_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_BYTES &&
                  callees_45xx.l4552_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_BYTES &&
                  callees_45xx.l458e_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_BYTES &&
                  callees_45xx.l466b_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_BYTES &&
                  callees_45xx.l4932_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_BYTES &&
                  callees_45xx.callees_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEES_BOUND_BYTES &&
                  callees_45xx.l424b_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_CPU_ADDRESS &&
                  callees_45xx.l43d6_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_CPU_ADDRESS &&
                  callees_45xx.l4552_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_CPU_ADDRESS &&
                  callees_45xx.l458e_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_CPU_ADDRESS &&
                  callees_45xx.l466b_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_CPU_ADDRESS &&
                  callees_45xx.l4932_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_CPU_ADDRESS &&
                  callees_45xx.l424b_proven &&
                  callees_45xx.l43d6_proven &&
                  callees_45xx.l4552_proven &&
                  callees_45xx.l458e_proven &&
                  callees_45xx.l466b_proven &&
                  callees_45xx.l4932_proven &&
                  callees_45xx.l424b_call_site_proven &&
                  callees_45xx.adjacency_proven,
              "US stage-two $45xx callee bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x24bu, 0x00u);
        check(theron_v1_track02_verify_stage2_enclosing_45xx_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &callees_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L424B head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x24bu, 0x64u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x552u, 0x00u);
        check(theron_v1_track02_verify_stage2_enclosing_45xx_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &callees_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L4552 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x552u, 0xa5u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 9u,
                 0x132u, 0x00u);
        check(theron_v1_track02_verify_stage2_enclosing_45xx_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &callees_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L4932 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 9u,
                 0x132u, 0x03u);
        check(theron_v1_track02_verify_stage2_l3114_tier5_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier5_callees) == THERON_TRACK02_SIGNAL_OK &&
                  tier5_callees.valid &&
                  tier5_callees.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  tier5_callees.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier5_callees.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier5_callees.l5403_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_BYTES &&
                  tier5_callees.l541e_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_BYTES &&
                  tier5_callees.l52a2_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_BYTES &&
                  tier5_callees.l52c8_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_BYTES &&
                  tier5_callees.l5657_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_BYTES &&
                  tier5_callees.l54c5_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_BYTES &&
                  tier5_callees.l5667_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_BYTES &&
                  tier5_callees.tier5_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES &&
                  tier5_callees.l5403_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_CPU_ADDRESS &&
                  tier5_callees.l541e_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_CPU_ADDRESS &&
                  tier5_callees.l52a2_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_CPU_ADDRESS &&
                  tier5_callees.l52c8_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_CPU_ADDRESS &&
                  tier5_callees.l5657_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_CPU_ADDRESS &&
                  tier5_callees.l54c5_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_CPU_ADDRESS &&
                  tier5_callees.l5667_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_CPU_ADDRESS &&
                  tier5_callees.l5403_proven &&
                  tier5_callees.l541e_proven &&
                  tier5_callees.l52a2_proven &&
                  tier5_callees.l52c8_proven &&
                  tier5_callees.l5657_proven &&
                  tier5_callees.l54c5_proven &&
                  tier5_callees.l5667_proven &&
                  tier5_callees.l53c4_call_site_proven &&
                  tier5_callees.l560b_call_sites_proven &&
                  tier5_callees.tier5_chain_contiguous_proven,
              "US stage-two L3114 tier-5 callee bodies are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x403u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier5_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier5_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L5403 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x403u, 0xdau);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x667u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier5_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier5_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L5667 table byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x667u, 0xffu);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x4c5u, 0x00u);
        check(theron_v1_track02_verify_stage2_l3114_tier5_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier5_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L54C5 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 2u,
                 0x4c5u, 0xceu);
        check(theron_v1_track02_verify_stage2_45xx_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_45xx) == THERON_TRACK02_SIGNAL_OK &&
                  tier2_45xx.valid &&
                  tier2_45xx.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  tier2_45xx.stage2_record ==
                      THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier2_45xx.stage2_raw_sector ==
                      225u + THERON_TRACK02_IPL_STAGE2_RECORD &&
                  tier2_45xx.l43a1_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_BYTES &&
                  tier2_45xx.l42bf_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_BYTES &&
                  tier2_45xx.gap45a6_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_BYTES &&
                  tier2_45xx.tier2_bound_bytes ==
                      THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES &&
                  tier2_45xx.l43a1_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_CPU_ADDRESS &&
                  tier2_45xx.l42bf_cpu_address ==
                      THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_CPU_ADDRESS &&
                  tier2_45xx.gap45a6_cpu_address == 0u &&
                  tier2_45xx.l43a1_proven &&
                  tier2_45xx.l42bf_proven &&
                  tier2_45xx.gap45a6_proven &&
                  tier2_45xx.l424b_call_sites_proven &&
                  tier2_45xx.adjacency_proven,
              "US stage-two $45xx tier-2 windows are byte-bound");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x3a1u, 0x00u);
        check(theron_v1_track02_verify_stage2_45xx_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L43A1 head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x3a1u, 0xa5u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x2bfu, 0x00u);
        check(theron_v1_track02_verify_stage2_45xx_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed L42BF head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x2bfu, 0xe6u);
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x5a6u, 0x00u);
        check(theron_v1_track02_verify_stage2_45xx_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_US_BIN,
                  &tier2_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "changed $45A6 gap head byte rejects");
        put_user(data, 225u + THERON_TRACK02_IPL_STAGE2_RECORD + 8u,
                 0x5a6u, 0x9cu);
        put_user(data, 226u, 3u, 3u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "wrong IPL sector count rejects");
        free(data);
    }
    data = make_fixture(224u, 3u, &data_size);
    check(data != NULL, "JP IPL fixture allocation");
    if (data) {
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_OK,
              "JP IPL fixture accepted");
        check_receipt(&receipt, THERON_TRACK02_VARIANT_JP_BIN, 224u, 3u,
                      "JP IPL fixture identity");
        check(theron_v1_track02_verify_stage2_entry_path(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &entry_path) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !entry_path.valid,
              "JP entry path stays out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_call_graph(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &call_graph) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !call_graph.valid,
              "JP call graph stays out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_dispatch_machine(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &dispatch_machine) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !dispatch_machine.valid,
              "JP dispatch machine stays out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_l8000_pair(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &l8000_pair) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !l8000_pair.valid,
              "JP L8000/L45A6 pair stays out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_jump_table_handlers(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &jump_table_handlers) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !jump_table_handlers.valid,
              "JP jump-table handlers stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_l4696_l3114(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &l4696_l3114) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !l4696_l3114.valid,
              "JP L4696/L3114 far callees stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_l3114_callees(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &l3114_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !l3114_callees.valid,
              "JP L3114 callees stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_l3114_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &tier2_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !tier2_callees.valid,
              "JP L3114 tier-2 callees stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_l3114_tier3_callees(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &tier3_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !tier3_callees.valid,
              "JP L3114 tier-3 callees stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_l3114_tier4_callees(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &tier4_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !tier4_callees.valid,
              "JP L3114 tier-4 callees stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_enclosing_45xx(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &enclosing_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !enclosing_45xx.valid,
              "JP enclosing $45xx routine stays out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_enclosing_45xx_callees(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &callees_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !callees_45xx.valid,
              "JP $45xx callees stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_l3114_tier5_callees(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &tier5_callees) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !tier5_callees.valid,
              "JP L3114 tier-5 callees stay out of the US-proven scope");
        check(theron_v1_track02_verify_stage2_45xx_tier2_callees(
                  data, data_size, THERON_TRACK02_MD5_JP_BIN,
                  &tier2_45xx) == THERON_TRACK02_SIGNAL_NOT_FOUND &&
                  !tier2_45xx.valid,
              "JP $45xx tier-2 windows stay out of the US-proven scope");
        put_user(data, 1155u, 0xcdu, 0x00u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing System Card CD_READ rejects");
        put_user(data, 1155u, 0xcdu, 0x20u);
        data[(1223u * RAW_SECTOR_BYTES) + USER_DATA_OFFSET + 0x80u] = 0u;
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing second-stage local CD_READ rejects");
        put_user(data, 1223u, 0x80u, 0xa9u);
        put_user(data, 1155u, 0xd5u, 0x01u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "wrong CD_EXEC record table rejects");
        free(data);
    }
    check(theron_v1_track02_find_ipl_loader(NULL, 0u, THERON_TRACK02_MD5_US_BIN,
                                             &receipt) == THERON_TRACK02_SIGNAL_BAD_INPUT,
          "bad IPL input rejects");
    check(theron_v1_track02_find_ipl_loader((const uint8_t *)"x", 1u,
                                             THERON_TRACK02_MD5_US_ISO,
                                             &receipt) == THERON_TRACK02_SIGNAL_BAD_INPUT,
          "non-sector input rejects before variant");

    check_real_media(getenv("FIRESTAFF_THERON_TRACK02_JP_BIN"),
                     THERON_TRACK02_MD5_JP_BIN, THERON_TRACK02_VARIANT_JP_BIN);
    check_real_media(getenv("FIRESTAFF_THERON_TRACK02_US_BIN"),
                     THERON_TRACK02_MD5_US_BIN, THERON_TRACK02_VARIANT_US_BIN);
    check_real_cue_boot_handoff(getenv("FIRESTAFF_THERON_TRACK02_JP_CUE"),
                                THERON_TRACK02_MD5_JP_BIN,
                                THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR);
    check_real_cue_boot_handoff(getenv("FIRESTAFF_THERON_TRACK02_US_CUE"),
                                THERON_TRACK02_MD5_US_BIN,
                                THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR);
    printf("summary: fail=%d\n", g_failures);
    return g_failures ? 1 : 0;
}
