/* Hash-gated real-media receipt for the System Card 3.0 sequence after $e8ec.
 *
 * Opt in with explicit paths only:
 *   FIRESTAFF_THERON_SYSCARD3_PCE=/absolute/path/syscard3.pce
 *   FIRESTAFF_THERON_TRACK02_US_BIN=/absolute/path/TQUS02.bin
 *   FIRESTAFF_THERON_19_TRACK_CUE=/absolute/path/TQUS.cue
 *
 * This proves bytes and branch addresses only. It assigns neither command nor
 * CD-controller semantics to the original port values.
 */

#include "asset_status_m12.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYSCARD3_MD5 "ff1a674273fe3540ccef576376407d1d"
#define SYSCARD3_BYTES 0x40200u
#define SYSCARD3_HEADER_BYTES 0x0200u
#define SYSCARD3_E000_WINDOW_OFFSET 0x0900u
#define SYSCARD3_E944_WINDOW_OFFSET 0x0944u
#define SYSCARD3_E95A_WINDOW_OFFSET 0x095au
#define SYSCARD3_E97A_WINDOW_OFFSET 0x097au
#define SYSCARD3_EA27_WINDOW_OFFSET 0x0a27u
#define SYSCARD3_EA35_WINDOW_OFFSET 0x0a35u

static const unsigned char g_post_delay_execution_bytes[] = {
    0xadu, 0xa4u, 0x22u, 0xd0u, 0xfbu, /* LDA $22a4 / BNE $e900 */
    0x9cu, 0x9bu, 0x22u,                /* STZ $229b */
    0xa9u, 0x81u, 0x8du, 0x01u, 0x18u,  /* LDA #$81 / STA $1801 */
    0x93u, 0x80u, 0x00u, 0x18u,         /* TST #$80,$1800 */
    0xf0u, 0x31u                        /* BEQ $e944 */
};

static const unsigned char g_e944_branch_target_bytes[] = {
    0x8du, 0x00u, 0x18u, /* STA $1800 */
    0x82u,               /* CLX */
    0xadu, 0x00u, 0x18u, /* LDA $1800 */
    0x29u, 0x40u,        /* AND #$40 */
    0xd0u, 0x0bu         /* BNE $e95a */
};

static const unsigned char g_e95a_set_branch_bytes[] = {
    0x9cu, 0x7au, 0x22u,             /* STZ $227a */
    0x82u, 0xadu, 0x00u, 0x18u,      /* CLX / LDA $1800 */
    0x29u, 0xf8u, 0x8du, 0x7au, 0x22u, /* AND #$f8 / STA $227a */
    0xc9u, 0xd0u, 0xf0u, 0x10u,      /* CMP #$d0 / BEQ $e97a */
    0x29u, 0xb8u, 0xc9u, 0x98u, 0xf0u, 0x1au,
    0xc9u, 0x88u, 0xf0u, 0x16u,
    0xc9u, 0x80u, 0xf0u, 0x12u,      /* raw comparisons/branches */
    0x80u, 0xe4u                       /* BRA $e95a */
};

static const unsigned char g_e97a_d0_branch_bytes[] = {
    0xbdu, 0x4cu, 0x22u, 0xe8u,      /* LDA $224c,X / INX */
    0x8du, 0x01u, 0x18u,             /* STA $1801 */
    0xeau, 0xeau, 0xeau, 0xeau,      /* NOP x4 */
    0x20u, 0x27u, 0xeau,             /* JSR $ea27 */
    0x80u, 0xd4u                      /* BRA $e95e */
};

static const unsigned char g_ea27_wait_bytes[] = {
    0x44u, 0x0cu,                     /* BSR $ea35 */
    0x93u, 0x40u, 0x00u, 0x18u,       /* TST #$40,$1800 */
    0xd0u, 0xfau,                     /* BNE $ea29 */
    0xa9u, 0x80u, 0x1cu, 0x02u, 0x18u, /* LDA #$80 / TRB $1802 */
    0x60u                              /* RTS */
};

static const unsigned char g_ea35_bsr_target_bytes[] = {
    0xa9u, 0x80u, 0x0cu, 0x02u, 0x18u, 0x60u /* LDA/TSB $1802/RTS */
};

static int g_failures;

static void check(int condition, const char *label) {
    if (!condition) {
        ++g_failures;
        printf("FAIL %s\n", label);
    } else {
        printf("PASS %s\n", label);
    }
}

static unsigned char *read_file(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long length;
    unsigned char *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (unsigned char *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

static int inspect_19_track_cue(const char *path) {
    FILE *file;
    char line[1024];
    unsigned int track;
    unsigned int expected_track = 1u;
    unsigned int track_count = 0u;
    int track02_mode1_2352 = 0;

    if (!path || !(file = fopen(path, "rb"))) return 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        char mode[32];
        if (sscanf(line, " TRACK %u %31s", &track, mode) == 2) {
            if (track != expected_track) {
                fclose(file);
                return 0;
            }
            ++expected_track;
            ++track_count;
            if (track == 2u && strcmp(mode, "MODE1/2352") == 0) {
                track02_mode1_2352 = 1;
            }
        }
    }
    fclose(file);
    return track_count == 19u && track02_mode1_2352;
}

int main(void) {
    const char *system_card_path = getenv("FIRESTAFF_THERON_SYSCARD3_PCE");
    const char *track02_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *cue_path = getenv("FIRESTAFF_THERON_19_TRACK_CUE");
    unsigned char *system_card;
    size_t system_card_size;
    char system_card_md5[33];
    char track02_md5[33];
    const size_t execution_offset = SYSCARD3_HEADER_BYTES +
        SYSCARD3_E000_WINDOW_OFFSET;
    const size_t e944_offset = SYSCARD3_HEADER_BYTES +
        SYSCARD3_E944_WINDOW_OFFSET;
    const size_t e95a_offset = SYSCARD3_HEADER_BYTES +
        SYSCARD3_E95A_WINDOW_OFFSET;
    const size_t e97a_offset = SYSCARD3_HEADER_BYTES +
        SYSCARD3_E97A_WINDOW_OFFSET;
    const size_t ea27_offset = SYSCARD3_HEADER_BYTES +
        SYSCARD3_EA27_WINDOW_OFFSET;
    const size_t ea35_offset = SYSCARD3_HEADER_BYTES +
        SYSCARD3_EA35_WINDOW_OFFSET;

    if (!system_card_path || !track02_path || !cue_path) {
        printf("SKIP set System Card, US raw Track02, and 19-track CUE paths\n");
        return 0;
    }
    system_card = read_file(system_card_path, &system_card_size);
    check(system_card && system_card_size == SYSCARD3_BYTES &&
              m12_file_md5_hex(system_card_path, system_card_md5) &&
              strcmp(system_card_md5, SYSCARD3_MD5) == 0,
          "authenticated System Card 3.0 container");
    check(m12_file_md5_hex(track02_path, track02_md5) &&
              strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) == 0,
          "authenticated US raw Track02 identity");
    check(inspect_19_track_cue(cue_path),
          "explicit 19-track CUE metadata remains structurally bounded");
    check(system_card && execution_offset + sizeof(g_post_delay_execution_bytes) <=
              system_card_size &&
              memcmp(system_card + execution_offset, g_post_delay_execution_bytes,
                     sizeof(g_post_delay_execution_bytes)) == 0,
          "$e900 waits on $22a4 then writes $81 to $1801 and tests $1800 bit 7");
    check(system_card && e944_offset + sizeof(g_e944_branch_target_bytes) <=
              system_card_size &&
              memcmp(system_card + e944_offset, g_e944_branch_target_bytes,
                     sizeof(g_e944_branch_target_bytes)) == 0,
          "$e944 writes $1800 then tests bit 6 with branch target $e95a");
    check(system_card && e95a_offset + sizeof(g_e95a_set_branch_bytes) <=
              system_card_size &&
              memcmp(system_card + e95a_offset, g_e95a_set_branch_bytes,
                     sizeof(g_e95a_set_branch_bytes)) == 0,
          "$e95a masks $1800 and branches on raw values to $e97a/$e98a/$e988/$e986");
    check(system_card && e97a_offset + sizeof(g_e97a_d0_branch_bytes) <=
              system_card_size &&
              memcmp(system_card + e97a_offset, g_e97a_d0_branch_bytes,
                     sizeof(g_e97a_d0_branch_bytes)) == 0,
          "$e97a loads $224c,X, increments X, writes $1801, calls $ea27, then returns to $e95e");
    check(system_card && ea27_offset + sizeof(g_ea27_wait_bytes) <=
              system_card_size &&
              memcmp(system_card + ea27_offset, g_ea27_wait_bytes,
                     sizeof(g_ea27_wait_bytes)) == 0,
          "$ea27 branches to $ea35 then polls $1800 bit 6 before TRB $1802");
    check(system_card && ea35_offset + sizeof(g_ea35_bsr_target_bytes) <=
              system_card_size &&
              memcmp(system_card + ea35_offset, g_ea35_bsr_target_bytes,
                     sizeof(g_ea35_bsr_target_bytes)) == 0,
          "$ea35 target writes raw $80 through TSB $1802 and returns");
    printf("receipt: e900_wait_address=22a4 first_port_write=1801 "
           "port_value=81 tst_address=1800 tst_mask=80 zero_branch=e944 "
           "e944_write_address=1800 e944_tst_mask=40 e944_set_branch=e95a "
           "e95a_mask_address=1800 e95a_mask=f8 e95a_store=227a "
           "e95a_raw_branches=d0:e97a,98:e98a,88:e988,80:e986 "
           "e97a_load_address=224c e97a_index_increment=1 "
           "e97a_write_address=1801 e97a_call=ea27 e97a_return=e95e "
           "ea27_bsr_target=ea35 ea27_tst_address=1800 ea27_tst_mask=40 "
           "ea27_set_branch=ea29 ea27_trb_address=1802 ea35_tsb_address=1802 "
           "controller_semantics_unproven=1 cue_track02_binding_unproven=1\n");
    free(system_card);
    return g_failures ? 1 : 0;
}
