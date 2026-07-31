/* Test-only CSBWin save corpus. Production code only accepts supplied bytes.
 * ReDMCSB SAVEHEAD.C F0429/F0430 and CSBWin SaveGame.cpp:927/1711/2111. */
#include "csb_v1_csbwin_save_loader_boundary_fixture.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include <string.h>

static size_t build_game(uint8_t *b, size_t cap, unsigned v, unsigned n, int cdsa)
{
    size_t i, j, total = CSB_SAVE_HEADER_SIZE + (size_t)n * CSB_SAVE_CHAMP_SIZE;
    if (!b || n > CSB_V1_MAX_CHAMPIONS || total > cap) return 0u;
    memset(b, 0, total); memcpy(b, "CSBGAME\0", CSB_SAVE_MAGIC_LEN);
    b[CSB_SAVE_HDR_OFF_VERSION] = (uint8_t)v;
    b[CSB_SAVE_HDR_OFF_VERSION + 1u] = (uint8_t)(v >> 8);
    b[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = (uint8_t)n;
    for (i = 0u; i < n; ++i) {
        uint8_t *r = b + CSB_SAVE_HEADER_SIZE + i * CSB_SAVE_CHAMP_SIZE;
        memcpy(r + CSB_SAVE_CH_OFF_NAME, "CHAMP_", 6u);
        r[CSB_SAVE_CH_OFF_NAME + 6u] = (uint8_t)('0' + i);
        r[CSB_SAVE_CH_OFF_CUR_HP] = r[CSB_SAVE_CH_OFF_MAX_HP] = 100u;
        for (j = 0u; j < CSB_V1_STAT_COUNT; ++j) {
            r[CSB_SAVE_CH_OFF_STAT_CUR + 2u * j] = 60u;
            r[CSB_SAVE_CH_OFF_STAT_MAX + 2u * j] = 60u;
        }
    }
    if (cdsa) memcpy(b + CSB_SAVE_HDR_OFF_CHAMP_COUNT, "CDSA", 4u);
    return total;
}

static size_t build_header(uint8_t *b, size_t cap, unsigned count, unsigned version)
{
    if (!b || cap < CSB_SAVE_HEADER_SIZE) return 0u;
    memset(b, 0, CSB_SAVE_HEADER_SIZE); memcpy(b, "CSBGAME\0", CSB_SAVE_MAGIC_LEN);
    b[CSB_SAVE_HDR_OFF_VERSION] = (uint8_t)version;
    b[CSB_SAVE_HDR_OFF_VERSION + 1u] = (uint8_t)(version >> 8);
    b[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = (uint8_t)count;
    return CSB_SAVE_HEADER_SIZE;
}

size_t csb_v1_csbwin_save_loader_boundary_build_fixture(
    CSB_V1_CSBWinSaveShape shape, uint8_t *b, size_t cap)
{
    size_t n;
    switch (shape) {
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20: return build_game(b, cap, CSB_SAVE_VERSION_V20, 1u, 0);
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V21: return build_game(b, cap, CSB_SAVE_VERSION_V21, 1u, 0);
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA: return build_game(b, cap, CSB_SAVE_VERSION_V20, 1u, 1);
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0: return build_game(b, cap, CSB_SAVE_VERSION_V20, 0u, 0);
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD: return build_game(b, cap, CSB_SAVE_VERSION_V20, 1u, 0);
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_5: return build_header(b, cap, 5u, CSB_SAVE_VERSION_V20);
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS: return build_header(b, cap, 4u, CSB_SAVE_VERSION_V20);
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_BAD_VERSION: return build_header(b, cap, 1u, 0x55u);
    case CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15:
        if (!b || cap < CSB_SAVE_HEADER_SIZE) return 0u;
        memset(b, 0, CSB_SAVE_HEADER_SIZE); memcpy(b, "RDMCSB15", 8u); return CSB_SAVE_HEADER_SIZE;
    case CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8: n = 4u; break;
    case CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS: n = CSB_SAVE_HEADER_SIZE; break;
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1:
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01:
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT: n = 512u; break;
    default: return 0u;
    }
    if (!b || cap < n) return 0u;
    memset(b, shape == CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS ? 0x7eu : 0, n);
    if (shape == CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8) memset(b, 0xa5, n);
    if (shape == CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1) memcpy(b, "CSB\1", 4u);
    if (shape == CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01) { b[0] = 'D'; b[1] = 'M'; b[3] = 1u; }
    if (shape == CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT) memcpy(b, "CEDT", 4u);
    return n;
}

int csb_v1_csbwin_save_loader_boundary_check_shape(
    CSB_V1_CSBWinSaveShape shape, CSB_V1_CSBWinLoaderBoundaryResult *out)
{
    uint8_t scratch[CSB_SAVE_HEADER_SIZE + 4u * CSB_SAVE_CHAMP_SIZE + 16u];
    size_t size;
    if (!out) return CSB_SAVE_IMPORT_ERR_NULL;
    size = csb_v1_csbwin_save_loader_boundary_build_fixture(shape, scratch, sizeof(scratch));
    if (size == 0u) {
        memset(out, 0, sizeof(*out));
        out->shape = shape;
        out->shape_label = csb_v1_csbwin_save_loader_boundary_shape_name(shape);
        out->loader_code = CSB_SAVE_IMPORT_ERR_NULL;
        out->expected_code = CSB_SAVE_IMPORT_ERR_NULL;
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    return csb_v1_csbwin_save_loader_boundary_check(scratch, size, shape, out);
}
