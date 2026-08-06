#include "dm1_v1_fmtowns_dyna_buttons_ja.h"
#include <string.h>

/* Source-locked Japanese DYNA_BUTTONS-equivalent label pool. Every
 * byte is a byte-verified extraction of the JDM.EXP pool at vaddr
 * 0x243bc. Strings are Shift-JIS byte sequences preserved verbatim;
 * ASCII-C-string literals cannot express half-width kana or
 * multi-byte kanji, so raw \xNN escapes are used throughout to
 * lock the exact source bytes. Do not "clean up" these strings by
 * converting to UTF-8; the runtime consumer requires the exact
 * Shift-JIS bytes JDM.EXP holds. */

static const char *const k_dm1_fmtowns_dyna_button_labels_ja_pc34[
        DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT_JA] = {
    /*  0 */ "N",                                     /* placeholder */
    /*  1 */ "\x82\xb3\x82\xa6\x82\xac\x82\xe9",     /* さえぎる (BLOCK)     */
    /*  2 */ "\x92\x40\x82\xab\x90\xd8\x82\xe9",     /* 叩き切る (CHOP)      */
    /*  3 */ "X",                                     /* placeholder */
    /*  4 */ "\x8a\x70\x93\x4a\x82\xf0\x90\x81\x82\xad",       /* 角笛を吹く (BLOW HORN) */
    /*  5 */ "\xba\xb2\xdd\xc4\xbd",                 /* ｺｲﾝﾄｽ (FLIP - half-width kana) */
    /*  6 */ "\x89\xa3\x82\xe9",                     /* 殴る (PUNCH)         */
    /*  7 */ "\x8f\x52\x82\xe9",                     /* 蹴る (KICK)          */
    /*  8 */ "\x82\xc6\x82\xab\x82\xcc\x90\xba",     /* ときの声 (WAR CRY)   */
    /*  9 */ "\x8e\x68\x82\xb7",                     /* 刺す (STAB)          */
    /* 10 */ "\x8d\x7e\x82\xe8\x82\xe9",             /* 降りる (CLIMB DOWN)  */
    /* 11 */ "\x8e\x9e\x8a\xd4\x93\x80\x8c\x8b",     /* 時間凍結 (FREEZE LIFE) */
    /* 12 */ "\x91\xc5\x82\xc2",                     /* 打つ (HIT)           */
    /* 13 */ "\x90\x55\x82\xe8\x89\xf1\x82\xb7",     /* 振り回す (SWING)     */
    /* 14 */ "\x8e\x68\x82\xb7",                     /* 刺す (STAB, dup)     */
    /* 15 */ "\x93\xcb\x82\xab\x8e\x68\x82\xb7",     /* 突き刺す (THRUST)    */
    /* 16 */ "\x93\xcb\x82\xad",                     /* 突く (JAB)           */
    /* 17 */ "\x82\xa9\x82\xed\x82\xb7",             /* かわす (PARRY)       */
    /* 18 */ "\x92\x40\x82\xab\x90\xd8\x82\xe9",     /* 叩き切る (HACK)      */
    /* 19 */ "\x96\x5c\x82\xea\x89\xf1\x82\xe9",     /* 暴れ回る (BERZERK)   */
    /* 20 */ "\x89\xce\x89\x8a\x92\x65",             /* 火炎弾 (FIREBALL)    */
    /* 21 */ "\x91\xce\x97\xec\x8e\xf4\x95\xb6",     /* 対霊呪文 (DISPELL)   */
    /* 22 */ "\x8d\xc3\x96\xb0\x8f\x70",             /* 催眠術 (CONFUSE)     */
    /* 23 */ "\x88\xee\x8d\xc8\x82\xcc\x8f\x70",     /* 稲妻の術 (LIGHTNING) */
    /* 24 */ "\x91\xce\x97\xec\x95\x90\x8a\xed",     /* 対霊武器 (DISRUPT)   */
    /* 25 */ "\x8e\x61\x82\xe8\x95\xa5\x82\xa4",     /* 斬り払う (MELEE)     */
    /* 26 */ "X",                                     /* placeholder */
    /* 27 */ "\x94\x4f\x82\xb6\x82\xe9",             /* 念じる (INVOKE)      */
    /* 28 */ "\x8e\x61\x82\xe8\x89\xba\x82\xeb\x82\xb7", /* 斬り下ろす (SLASH) */
    /* 29 */ "\x8e\x61\x82\xe8\x97\xf4\x82\xad",     /* 斬り裂く (CLEAVE)    */
    /* 30 */ "\x91\xc5\x82\xbf\x8a\x84\x82\xe9",     /* 打ち割る (BASH)      */
    /* 31 */ "\x8b\x43\x90\xe2\x82\xb3\x82\xb9\x82\xe9", /* 気絶させる (STUN) */
    /* 32 */ "\x8e\xcb\x82\xe9",                     /* 射る (SHOOT)         */
    /* 33 */ "\x8e\xf4\x95\xb6\x96\x68\x8c\xe4",     /* 呪文防御 (SPELLSHIELD) */
    /* 34 */ "\x89\xce\x89\x8a\x96\x68\x8c\xe4",     /* 火炎防御 (FIRESHIELD) */
    /* 35 */ "\x8e\xf4\x94\x9b\x82\xb7\x82\xe9",     /* 呪縛する (FLUXCAGE)  */
    /* 36 */ "\x8e\xa1\x97\xc3\x82\xb7\x82\xe9",     /* 治療する (HEAL)      */
    /* 37 */ "\x8e\xe8\x82\xc8\x82\xb8\x82\xaf\x82\xe9", /* 手なずける (CALM) */
    /* 38 */ "\x96\x82\x8f\x70\x93\x94\x89\xce",     /* 魔術灯火 (LIGHT)     */
    /* 39 */ "\x93\xa7\x8e\x8b",                     /* 透視 (WINDOW)        */
    /* 40 */ "\x89\xce\x89\x8a\x8d\x55\x8c\x82",     /* 火炎攻撃 (SPIT)      */
    /* 41 */ "\x92\xc7\x82\xa2\x95\xa5\x82\xa4",     /* 追い払う (BRANDISH)  */
    /* 42 */ "\x93\x8a\x82\xb0\x82\xe9",             /* 投げる (THROW)       */
    /* 43 */ "\x97\x5a\x8d\x87\x82\xb7\x82\xe9",     /* 融合する (FUSE)      */
};

const char *dm1_v1_fmtowns_dyna_button_label_ja_pc34(unsigned int index) {
    if (index == DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL_JA) {
        return "";
    }
    if (index >= DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT_JA) {
        return NULL;
    }
    return k_dm1_fmtowns_dyna_button_labels_ja_pc34[index];
}

size_t dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(unsigned int index) {
    const char *s;
    if (index == DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL_JA) return 0;
    if (index >= DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT_JA) return 0;
    s = k_dm1_fmtowns_dyna_button_labels_ja_pc34[index];
    return strlen(s);
}

size_t dm1_v1_fmtowns_dyna_buttons_source_span_bytes_ja_pc34(void) {
    size_t total = 0;
    unsigned int i;
    for (i = 0; i < DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT_JA; ++i) {
        total += strlen(k_dm1_fmtowns_dyna_button_labels_ja_pc34[i]) + 1;
    }
    return total;
}
