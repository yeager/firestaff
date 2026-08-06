#include "dm1_v1_fmtowns_startup.h"

#include <string.h>

/* These hashes are from the locally verified HMA-240 BIN/CUE archive. */
static const char k_autoexec_md5[] = "64b9a0a317f387c270b485f242e27159";
static const char k_edm_md5[] = "c27e7b984df9753912c3375dc121919f";
static const char k_jdm_md5[] = "acfbcfa5d65032a4bcabc8d5ea062dcc";
static const char k_tmenu_exp_md5[] = "a0ff723135c1e40f6dd471ce78f28040";
static const char k_tmenu_icn_md5[] = "cd45c65b8ba45b43b81b6d544fe2c792";
static const char k_tmenu_inf_md5[] = "f06d543bb7ded911ab9b1c02c48b7150";

typedef struct {
    uint32_t s[4];
    uint64_t count;
    uint8_t buf[64];
} Md5;

static const uint32_t k_md5_k[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};
static const uint8_t k_md5_r[64] = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
};

static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint32_t rol(uint32_t v, unsigned n) { return (v << n) | (v >> (32U - n)); }
static void md5_transform(Md5 *c, const uint8_t *b) {
    uint32_t a=c->s[0], b0=c->s[1], d=c->s[3], cc=c->s[2], m[16];
    int i;
    for (i=0;i<16;i++) m[i]=rd32(b+(size_t)i*4U);
    for (i=0;i<64;i++) {
        uint32_t f,g,t;
        if (i<16) { f=(b0&cc)|(~b0&d); g=(uint32_t)i; }
        else if (i<32) { f=(d&b0)|(~d&cc); g=(uint32_t)((5*i+1)%16); }
        else if (i<48) { f=b0^cc^d; g=(uint32_t)((3*i+5)%16); }
        else { f=cc^(b0|~d); g=(uint32_t)((7*i)%16); }
        t=d; d=cc; cc=b0;
        b0 += rol(a+f+k_md5_k[i]+m[g], k_md5_r[i]);
        a=t;
    }
    c->s[0]+=a; c->s[1]+=b0; c->s[2]+=cc; c->s[3]+=d;
}
static void md5_init(Md5 *c) {
    c->s[0]=0x67452301; c->s[1]=0xefcdab89; c->s[2]=0x98badcfe; c->s[3]=0x10325476; c->count=0;
}
static void md5_update(Md5 *c, const uint8_t *p, size_t n) {
    size_t off=(size_t)(c->count&63U), i;
    c->count += n;
    for (i=0;i<n;i++) { c->buf[off++]=p[i]; if (off==64U) { md5_transform(c,c->buf); off=0; } }
}
static void md5_final(Md5 *c, char out[33]) {
    static const char hex[]="0123456789abcdef";
    uint8_t raw[16], len[8]; size_t off=(size_t)(c->count&63U); int i;
    uint64_t bits=c->count*8U;
    c->buf[off++]=0x80;
    if (off>56U) { memset(c->buf+off,0,64U-off); md5_transform(c,c->buf); off=0; }
    memset(c->buf+off,0,56U-off);
    for (i=0;i<8;i++) len[i]=(uint8_t)(bits>>(8*i));
    memcpy(c->buf+56,len,8); md5_transform(c,c->buf);
    for (i=0;i<4;i++) { raw[4*i]=(uint8_t)c->s[i]; raw[4*i+1]=(uint8_t)(c->s[i]>>8); raw[4*i+2]=(uint8_t)(c->s[i]>>16); raw[4*i+3]=(uint8_t)(c->s[i]>>24); }
    for (i=0;i<16;i++) { out[2*i]=hex[raw[i]>>4]; out[2*i+1]=hex[raw[i]&15]; }
    out[32]='\0';
}
static void hash_bytes(const uint8_t *p, size_t n, char out[33]) { Md5 c; md5_init(&c); md5_update(&c,p,n); md5_final(&c,out); }

static int contains_ascii(const uint8_t *bytes, size_t size, const char *needle) {
    size_t n;
    size_t i;
    if (!bytes || !needle) return 0;
    n = strlen(needle);
    if (n == 0 || n > size) return 0;
    for (i = 0; i + n <= size; ++i) {
        if (memcmp(bytes + i, needle, n) == 0) return 1;
    }
    return 0;
}

static int menu_info_selects_program(const uint8_t *menu_info, size_t size,
                                     int english) {
    static const char k_japanese_entry[] = "JDM     .EXP";
    static const char k_english_entry[] = "EDM     .EXP";
    const char *entry = english ? k_english_entry : k_japanese_entry;
    size_t offset = english ? 0x80u : 0u;
    size_t entry_size = 0x80u;
    if (!menu_info || offset >= size) return 0;
    if (entry_size > size - offset) entry_size = size - offset;
    return contains_ascii(menu_info + offset, entry_size, entry);
}

static uint16_t read_le16(const uint8_t *p) {
    return (uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8);
}

static uint32_t read_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int validate_p3_header(const uint8_t *program, size_t size,
                              uint32_t *header_size,
                              uint32_t *load_image_offset,
                              uint32_t *load_image_size,
                              uint32_t *symbol_table_offset,
                              uint32_t *symbol_table_size,
                              uint32_t *initial_eip) {
    uint32_t file_size;
    uint32_t runtime_offset;
    uint32_t runtime_size;
    uint32_t relocation_offset;
    uint32_t relocation_size;
    uint32_t memory_requirements;
    uint16_t level;
    if (!program || size < 0x80u ||
        program[0] != 'P' || program[1] != '3') return 0;
    level = read_le16(program + 2);
    if (level != 1u) return 0;
    *header_size = read_le16(program + 4);
    file_size = read_le32(program + 6);
    runtime_offset = read_le32(program + 0x0cu);
    runtime_size = read_le32(program + 0x10u);
    relocation_offset = read_le32(program + 0x14u);
    relocation_size = read_le32(program + 0x18u);
    *load_image_offset = read_le32(program + 0x26u);
    *load_image_size = read_le32(program + 0x2au);
    *symbol_table_offset = read_le32(program + 0x2eu);
    *symbol_table_size = read_le32(program + 0x32u);
    *initial_eip = read_le32(program + 0x68u);
    memory_requirements = read_le32(program + 0x74u);
    if (*header_size < 0x80u || *header_size > size || file_size == 0u ||
        file_size > size || runtime_offset < *header_size ||
        runtime_offset > size || runtime_size > size - runtime_offset ||
        *load_image_offset < *header_size ||
        *load_image_offset > size ||
        *load_image_size > size - *load_image_offset ||
        *load_image_offset + *load_image_size > file_size ||
        relocation_offset > size || relocation_size > size - relocation_offset ||
        (relocation_size != 0u && relocation_offset < *header_size) ||
        (*symbol_table_offset != 0u &&
         (*symbol_table_offset < *header_size ||
          *symbol_table_offset > size ||
          *symbol_table_size > size - *symbol_table_offset)) ||
        memory_requirements < *load_image_size ||
        *initial_eip >= memory_requirements) return 0;
    return 1;
}

int dm1_v1_fmtowns_startup_receipt(const uint8_t *autoexec, size_t autoexec_size,
    const uint8_t *game_program, size_t game_program_size,
    const uint8_t *menu_program, size_t menu_program_size,
    const uint8_t *menu_icon, size_t menu_icon_size,
    const uint8_t *menu_info, size_t menu_info_size,
    DM1_V1_FmtownsStartupReceipt *out) {
    char autoHash[33], gameHash[33], menuHash[33], iconHash[33], infoHash[33];
    int english, japanese;
    int menuSymbols;
    int gameSymbols;
    uint32_t gameHeaderSize = 0U;
    uint32_t gameLoadOffset = 0U;
    uint32_t gameLoadSize = 0U;
    uint32_t gameSymbolOffset = 0U;
    uint32_t gameSymbolSize = 0U;
    uint32_t gameInitialEip = 0U;
    uint32_t menuHeaderSize = 0U;
    uint32_t menuLoadOffset = 0U;
    uint32_t menuLoadSize = 0U;
    uint32_t menuSymbolOffset = 0U;
    uint32_t menuSymbolSize = 0U;
    uint32_t menuInitialEip = 0U;
    if (!out) return 0;
    memset(out,0,sizeof(*out));
    if (!autoexec || !game_program || !menu_program || !menu_icon || !menu_info ||
        autoexec_size == 0 || game_program_size < 4 || menu_program_size < 4) return 0;
    hash_bytes(autoexec,autoexec_size,autoHash); hash_bytes(game_program,game_program_size,gameHash);
    hash_bytes(menu_program,menu_program_size,menuHash); hash_bytes(menu_icon,menu_icon_size,iconHash); hash_bytes(menu_info,menu_info_size,infoHash);
    english=!strcmp(gameHash,k_edm_md5); japanese=!strcmp(gameHash,k_jdm_md5);
    /* TMENU.EXP is the TownsOS file browser, not the game executable.  Its
     * stable ownership evidence is the real menu assets and Towns mouse/OS
     * bindings; the game-owned title symbols live in EDM/JDM below. */
    menuSymbols = contains_ascii(menu_program, menu_program_size, "\\TMENU.INF") &&
                  contains_ascii(menu_program, menu_program_size, "\\TMENU.ICN") &&
                  contains_ascii(menu_program, menu_program_size, "TownsOS") &&
                  contains_ascii(menu_program, menu_program_size, "TOWNS MOUSE LIBRARY");
    gameSymbols = contains_ascii(game_program, game_program_size, "DO_TITLE_ANIMATION") &&
                  contains_ascii(game_program, game_program_size, "TITLE_MASTER") &&
                  contains_ascii(game_program, game_program_size, "TITLE_PRESENTS") &&
                  contains_ascii(game_program, game_program_size, "SHOW_DUNGEON") &&
                  contains_ascii(game_program, game_program_size, "LOAD_3D_GRAPHICS") &&
                  contains_ascii(game_program, game_program_size, "CD_LEVEL_SONG");
    if (!validate_p3_header(menu_program, menu_program_size,
                            &menuHeaderSize, &menuLoadOffset, &menuLoadSize,
                            &menuSymbolOffset, &menuSymbolSize,
                            &menuInitialEip) ||
        !validate_p3_header(game_program, game_program_size,
                            &gameHeaderSize, &gameLoadOffset, &gameLoadSize,
                            &gameSymbolOffset, &gameSymbolSize,
                            &gameInitialEip)) return 0;
    if (strcmp(autoHash,k_autoexec_md5) || strcmp(menuHash,k_tmenu_exp_md5) ||
        strcmp(iconHash,k_tmenu_icn_md5) || strcmp(infoHash,k_tmenu_inf_md5) || (!english && !japanese) ||
        game_program[0] != 'P' || game_program[1] != '3' || game_program[2] != 1 || game_program[3] != 0 ||
        menu_program[0] != 'P' || menu_program[1] != '3' || menu_program[2] != 1 || menu_program[3] != 0 ||
        !menuSymbols || !gameSymbols) return 0;
    out->valid=1; out->language=english?DM1_FMTOWNS_LANG_EN:DM1_FMTOWNS_LANG_JP;
    out->autoexec_size=(uint32_t)autoexec_size; out->game_program_size=(uint32_t)game_program_size; out->menu_program_size=(uint32_t)menu_program_size; out->menu_icon_size=(uint32_t)menu_icon_size; out->menu_info_size=(uint32_t)menu_info_size;
    memcpy(out->game_program_name,english?"EDM.EXP":"JDM.EXP",7); out->game_program_name[7]='\0';
    strcpy(out->autoexec_md5,autoHash); strcpy(out->game_program_md5,gameHash); strcpy(out->menu_program_md5,menuHash); strcpy(out->menu_icon_md5,iconHash); strcpy(out->menu_info_md5,infoHash);
    out->menu_info_selects_game = menu_info_selects_program(menu_info, menu_info_size, english);
    out->menu_program_symbols_verified = menuSymbols;
    out->game_program_symbols_verified = gameSymbols;
    out->menu_p3_header_verified = 1;
    out->game_p3_header_verified = 1;
    out->game_p3_header_size = gameHeaderSize;
    out->game_p3_load_image_offset = gameLoadOffset;
    out->game_p3_load_image_size = gameLoadSize;
    out->game_p3_symbol_table_offset = gameSymbolOffset;
    out->game_p3_symbol_table_size = gameSymbolSize;
    out->game_p3_initial_eip = gameInitialEip;
    if (!out->menu_info_selects_game) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->title_track=2; out->hall_track=3; out->entrance_track=5;
    return 1;
}

int dm1_v1_fmtowns_startup_receipt_is_native(const DM1_V1_FmtownsStartupReceipt *receipt) {
    return receipt && receipt->valid && (receipt->language == DM1_FMTOWNS_LANG_EN || receipt->language == DM1_FMTOWNS_LANG_JP);
}

int dm1_v1_fmtowns_startup_receipt_has_native_owners(
    const DM1_V1_FmtownsStartupReceipt *receipt) {
    return dm1_v1_fmtowns_startup_receipt_is_native(receipt) &&
           receipt->menu_info_selects_game &&
           receipt->menu_program_symbols_verified &&
           receipt->game_program_symbols_verified &&
           receipt->menu_p3_header_verified &&
           receipt->game_p3_header_verified;
}
