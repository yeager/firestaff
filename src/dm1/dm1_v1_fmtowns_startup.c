#include "dm1_v1_fmtowns_startup.h"
#include "fs_portable_compat.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

int dm1_v1_fmtowns_menu_receipt(const uint8_t *menu_info, size_t menu_info_size,
                                DM1_V1_FmtownsMenuReceipt *out)
{
    static const char k_programs[DM1_FMTOWNS_TMENU_ENTRY_COUNT][13] = {
        "JDM     .EXP", "EDM     .EXP"
    };
    static const char k_paths[DM1_FMTOWNS_TMENU_ENTRY_COUNT][9] = {
        "\\JDM.EXP", "\\EDM.EXP"
    };
    static const DM1_V1_FmtownsLang k_languages[
        DM1_FMTOWNS_TMENU_ENTRY_COUNT] = {
        DM1_FMTOWNS_LANG_JP, DM1_FMTOWNS_LANG_EN
    };
    size_t entry_index;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!menu_info || menu_info_size != 0x100u) return 0;

    for (entry_index = 0u;
         entry_index < DM1_FMTOWNS_TMENU_ENTRY_COUNT;
         ++entry_index) {
        const size_t base = entry_index * 0x80u;
        DM1_V1_FmtownsMenuEntry *entry = &out->entries[entry_index];
        /* HMA-240 TMENU.INF: an 8.3 program name begins each record and
         * the absolute DOS path begins at byte 0x40.  Do not infer an
         * arbitrary executable from printable bytes elsewhere in the file. */
        if (memcmp(menu_info + base, k_programs[entry_index], 12u) != 0 ||
            memcmp(menu_info + base + 0x40u, k_paths[entry_index], 8u) != 0) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        entry->valid = 1;
        entry->language = k_languages[entry_index];
        memcpy(entry->program_name, k_programs[entry_index], 12u);
        entry->program_name[12] = '\0';
        memcpy(entry->program_path, k_paths[entry_index], 8u);
        entry->program_path[8] = '\0';
        memcpy(entry->title_bytes, menu_info + base + 0x37u,
               sizeof(entry->title_bytes));
    }
    out->valid = 1;
    return 1;
}

static int menu_info_selects_program(const uint8_t *menu_info, size_t size,
                                     int english) {
    DM1_V1_FmtownsMenuReceipt receipt;
    const unsigned int expected_index = english ? 1u : 0u;
    return dm1_v1_fmtowns_menu_receipt(menu_info, size, &receipt) &&
           receipt.entries[expected_index].valid &&
           receipt.entries[expected_index].language ==
               (english ? DM1_FMTOWNS_LANG_EN : DM1_FMTOWNS_LANG_JP);
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

typedef struct {
    int found;
    uint32_t value;
} SymbolMatch;

static int bytes_equal_at(const uint8_t *bytes, size_t size, size_t offset,
                          const uint8_t *expected, size_t expected_size) {
    return bytes && expected && offset <= size && expected_size <= size - offset &&
           memcmp(bytes + offset, expected, expected_size) == 0;
}

static int parse_title_animation_plan(const uint8_t *program, size_t size,
                                      uint32_t load_offset,
                                      uint32_t load_size,
                                      DM1_V1_FmtownsStartupReceipt *out) {
    static const uint8_t k_get_title_graphic[] = {
        0x6a, 0x00, 0x6a, 0x00, 0x50, 0x6a, 0x01
    };
    static const uint8_t k_presents_source[] = {
        0x68, 0x40, 0x01, 0x00, 0x00, 0x68, 0x89, 0x00, 0x00, 0x00
    };
    static const uint8_t k_master_source[] = {
        0x6a, 0x50, 0x6a, 0x00
    };
    static const uint8_t k_zoom_count[] = {0x66, 0x83, 0xff, 0x12};
    static const uint8_t k_zoom_width_step[] = {0x66, 0x83, 0xeb, 0x04};
    static const uint8_t k_zoom_height_step[] = {0x66, 0x83, 0xee, 0x10};
    static const uint16_t k_swoosh_rect[4] = {0u, 319u, 0u, 56u};
    static const uint16_t k_presents_rect[4] = {0u, 319u, 90u, 105u};
    static const uint16_t k_master_rect[4] = {0u, 319u, 118u, 174u};
    size_t base;
    size_t i;
    if (!program || !out || load_offset > size || load_size > size - load_offset ||
        load_size < 0xc600u) return 0;
    base = (size_t)load_offset;
    if (!bytes_equal_at(program, size, base + 0xc3d1u,
                        k_get_title_graphic, sizeof(k_get_title_graphic)) ||
        !bytes_equal_at(program, size, base + 0xc3f9u,
                        k_presents_source, sizeof(k_presents_source)) ||
        !bytes_equal_at(program, size, base + 0xc440u,
                        k_master_source, sizeof(k_master_source)) ||
        !bytes_equal_at(program, size, base + 0xc45fu,
                        k_zoom_count, sizeof(k_zoom_count)) ||
        !bytes_equal_at(program, size, base + 0xc522u,
                        k_zoom_width_step, sizeof(k_zoom_width_step)) ||
        !bytes_equal_at(program, size, base + 0xc526u,
                        k_zoom_height_step, sizeof(k_zoom_height_step))) return 0;
    for (i = 0; i < 4u; ++i) {
        size_t rect_base = base + 0x28f50u + i * 2u;
        if (rect_base > size || sizeof(uint16_t) > size - rect_base) return 0;
        /* The verified FM Towns P3 image is little-endian. */
        if (read_le16(program + rect_base) != k_master_rect[i]) return 0;
        out->game_title_master_rect[i] = k_master_rect[i];
    }
    for (i = 0; i < 4u; ++i) {
        size_t swoosh_base = base + 0x28f58u + i * 2u;
        size_t presents_base = base + 0x28f60u + i * 2u;
        if (swoosh_base > size || presents_base > size ||
            sizeof(uint16_t) > size - swoosh_base ||
            sizeof(uint16_t) > size - presents_base ||
            read_le16(program + swoosh_base) != k_swoosh_rect[i] ||
            read_le16(program + presents_base) != k_presents_rect[i]) return 0;
        out->game_title_swoosh_rect[i] = k_swoosh_rect[i];
        out->game_title_presents_rect[i] = k_presents_rect[i];
    }
    out->game_title_animation_plan_verified = 1;
    out->game_title_graphic_index = 1u;
    out->game_title_presents_source_y = 137u;
    out->game_title_master_source_y = 80u;
    out->game_title_zoom_source_width = 320u;
    out->game_title_zoom_source_height = 80u;
    out->game_title_zoom_start_width = 320u;
    out->game_title_zoom_start_height = 80u;
    out->game_title_zoom_width_step = 16u;
    out->game_title_zoom_height_step = 4u;
    out->game_title_zoom_step_count = 18u;
    return 1;
}

/* Phar Lap's real SYM1 table is a compact sequence of
 * [name-length][name][DWORD value][WORD type] records.  The first PROGRAM
 * record occupies the format's fixed preamble; the regular sequence starts
 * at byte 0x22.  Keep this parser deliberately bounded and source-neutral:
 * it reports only names actually present in the executable and never invents
 * a call target from a nearby string. */
static int parse_sym1_table(const uint8_t *table, size_t size,
                            uint32_t *entry_count,
                            SymbolMatch *title_animation,
                            SymbolMatch *title_presents,
                            SymbolMatch *title_dungeon,
                            SymbolMatch *draw_dmenu,
                            SymbolMatch *dynamenu,
                            SymbolMatch *menu_icons,
                            SymbolMatch *cd_level_song) {
    size_t cursor = 0x22u;
    uint32_t count = 0u;
    SymbolMatch *matches[] = {
        title_animation, title_presents, title_dungeon, draw_dmenu,
        dynamenu, menu_icons, cd_level_song
    };
    static const char *names[] = {
        "DO_TITLE_ANIMATION", "TITLE_PRESENTS", "TITLE_DUNGEON",
        "DRAW_DMENU", "DYNAMENU", "MENU_ICONS", "CD_LEVEL_SONG"
    };
    size_t i;
    if (!table || size < cursor || memcmp(table, "SYM1", 4) != 0 ||
        !entry_count || !title_animation || !title_presents ||
        !title_dungeon || !draw_dmenu || !dynamenu || !menu_icons ||
        !cd_level_song) return 0;
    for (i = 0; i < 7u; ++i) {
        matches[i]->found = 0;
        matches[i]->value = 0u;
    }
    while (cursor < size) {
        size_t name_size;
        uint32_t value;
        if (size - cursor < 1u) return 0;
        name_size = table[cursor++];
        if (name_size == 0u || name_size > 127u ||
            name_size > size - cursor || size - cursor - name_size < 6u) return 0;
        value = (uint32_t)table[cursor + name_size] |
                ((uint32_t)table[cursor + name_size + 1u] << 8) |
                ((uint32_t)table[cursor + name_size + 2u] << 16) |
                ((uint32_t)table[cursor + name_size + 3u] << 24);
        for (i = 0; i < 7u; ++i) {
            size_t expected_size = strlen(names[i]);
            if (name_size == expected_size &&
                memcmp(table + cursor, names[i], expected_size) == 0) {
                matches[i]->found = 1;
                matches[i]->value = value;
            }
        }
        cursor += name_size + 6u;
        ++count;
    }
    *entry_count = count;
    return count > 0u;
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
    uint32_t gameSymbolCount = 0U;
    SymbolMatch titleAnimation = {0, 0U};
    SymbolMatch titlePresents = {0, 0U};
    SymbolMatch titleDungeon = {0, 0U};
    SymbolMatch drawDmenu = {0, 0U};
    SymbolMatch dynamenu = {0, 0U};
    SymbolMatch menuIcons = {0, 0U};
    SymbolMatch cdLevelSong = {0, 0U};
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
    if (gameSymbolOffset != 0u && gameSymbolSize != 0u &&
        gameSymbolOffset <= game_program_size &&
        gameSymbolSize <= game_program_size - gameSymbolOffset) {
        (void)parse_sym1_table(game_program + gameSymbolOffset,
                               gameSymbolSize, &gameSymbolCount,
                               &titleAnimation, &titlePresents, &titleDungeon,
                               &drawDmenu, &dynamenu, &menuIcons,
                               &cdLevelSong);
    }
    if (strcmp(autoHash,k_autoexec_md5) || strcmp(menuHash,k_tmenu_exp_md5) ||
        strcmp(iconHash,k_tmenu_icn_md5) || strcmp(infoHash,k_tmenu_inf_md5) || (!english && !japanese) ||
        game_program[0] != 'P' || game_program[1] != '3' || game_program[2] != 1 || game_program[3] != 0 ||
        menu_program[0] != 'P' || menu_program[1] != '3' || menu_program[2] != 1 || menu_program[3] != 0 ||
        !menuSymbols || !gameSymbols ||
        (english && (!titleAnimation.found || !titlePresents.found ||
                     !titleDungeon.found || !drawDmenu.found ||
                     !dynamenu.found || !menuIcons.found ||
                     !cdLevelSong.found))) return 0;
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
    out->game_symbol_table_verified = gameSymbolCount > 0u;
    out->game_symbol_table_entry_count = gameSymbolCount;
    out->game_do_title_animation_entry = titleAnimation.value;
    out->game_title_presents_entry = titlePresents.value;
    out->game_title_dungeon_entry = titleDungeon.value;
    out->game_draw_dmenu_entry = drawDmenu.value;
    out->game_dynamenu_entry = dynamenu.value;
    out->game_menu_icons_entry = menuIcons.value;
    out->game_cd_level_song_entry = cdLevelSong.value;
    if (english && !parse_title_animation_plan(
            game_program, game_program_size, gameLoadOffset, gameLoadSize, out)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
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

static int read_startup_file(const char *root, const char *name,
                             uint8_t **out_data, size_t *out_size) {
    char path[FSP_PATH_MAX];
    FILE *file;
    long size;
    uint8_t *data;
    if (!root || !name || !out_data || !out_size ||
        !FSP_JoinPath(path, sizeof(path), root, name) ||
        !(file = fopen(path, "rb"))) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0) { fclose(file); return 0; }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1u, (size_t)size, file) != (size_t)size) {
        free(data); fclose(file); return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

int dm1_v1_fmtowns_startup_receipt_from_directory(
    const char *root, int japanese, DM1_V1_FmtownsStartupReceipt *out) {
    uint8_t *autoexec = NULL, *game = NULL, *menu = NULL;
    uint8_t *icons = NULL, *info = NULL;
    size_t autoexecSize = 0, gameSize = 0, menuSize = 0;
    size_t iconSize = 0, infoSize = 0;
    int ok = 0;
    const char *gameName = japanese ? "JDM.EXP" : "EDM.EXP";
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!root ||
        !read_startup_file(root, "AUTOEXEC.BAT", &autoexec, &autoexecSize) ||
        !read_startup_file(root, gameName, &game, &gameSize) ||
        !read_startup_file(root, "TMENU.EXP", &menu, &menuSize) ||
        !read_startup_file(root, "TMENU.ICN", &icons, &iconSize) ||
        !read_startup_file(root, "TMENU.INF", &info, &infoSize)) goto done;
    ok = dm1_v1_fmtowns_startup_receipt(
        autoexec, autoexecSize, game, gameSize, menu, menuSize,
        icons, iconSize, info, infoSize, out);
done:
    free(autoexec); free(game); free(menu); free(icons); free(info);
    return ok;
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
