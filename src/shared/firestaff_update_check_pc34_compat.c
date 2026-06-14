/*
 * firestaff_update_check_pc34_compat.c
 *
 * Self-update-check helpers.  No network dependency.  The
 * M12 launcher fetches the latest release JSON out-of-band
 * and calls firestaff_update_check_evaluate().  This file
 * owns:
 *  - Semver compare (drops leading "v" and pre-release
 *    suffix).
 *  - Minimal JSON tag_name extractor (handles simple
 *    backslash un-escapes; rejects malformed bodies).
 *  - Build-time escape hatch (FIRESTAFF_NO_UPDATE_CHECK).
 */
#include "firestaff_update_check_pc34_compat.h"
#include "changelog_m12.h"

#include <SDL3/SDL.h>
#include <string.h>
#include <ctype.h>

/* ---- semver compare ---- */
static int semver_part(const char* s, int* idx) {
    int n = 0;
    while (s[*idx] >= '0' && s[*idx] <= '9') {
        n = n * 10 + (s[*idx] - '0');
        (*idx)++;
    }
    return n;
}

int firestaff_update_check_compare_semver(const char* a, const char* b) {
    int ia = 0, ib = 0;
    int aMaj, aMin, aPat, bMaj, bMin, bPat;
    if (!a || !b) return 0;
    if (a[0] == 'v' || a[0] == 'V') ia++;
    if (b[0] == 'v' || b[0] == 'V') ib++;
    aMaj = semver_part(a, &ia);
    bMaj = semver_part(b, &ib);
    if (a[ia] == '.') ia++;
    if (b[ib] == '.') ib++;
    aMin = semver_part(a, &ia);
    bMin = semver_part(b, &ib);
    if (a[ia] == '.') ia++;
    if (b[ib] == '.') ib++;
    aPat = semver_part(a, &ia);
    bPat = semver_part(b, &ib);
    if (aMaj != bMaj) return aMaj < bMaj ? -1 : +1;
    if (aMin != bMin) return aMin < bMin ? -1 : +1;
    if (aPat != bPat) return aPat < bPat ? -1 : +1;
    return 0;
}

/* ---- JSON field extractor ---- */
int firestaff_update_check_extract_tag(const char* body,
                                       int bodyLen,
                                       char* outBuf, int outCap) {
    static const char kKey[] = "tag_name";
    int kLen = (int)sizeof(kKey) - 1;
    int i = 0;
    if (!body || bodyLen <= 0 || !outBuf || outCap <= 0) return 0;
    while (i + kLen + 2 < bodyLen) {
        if (body[i] == '"' &&
            memcmp(body + i + 1, kKey, kLen) == 0 &&
            body[i + 1 + kLen] == '"' &&
            body[i + 2 + kLen] == ':') {
            int j = i + 3 + kLen;
            while (j < bodyLen && (body[j] == ' ' || body[j] == '\t')) j++;
            if (j >= bodyLen || body[j] != '"') return 0;
            j++;
            int k = 0;
            while (j < bodyLen && body[j] != '"' && k < outCap - 1) {
                if (body[j] == '\\' && j + 1 < bodyLen) {
                    char esc = body[j + 1];
                    if      (esc == 'n') outBuf[k++] = '\n';
                    else if (esc == 't') outBuf[k++] = '\t';
                    else if (esc == 'r') outBuf[k++] = '\r';
                    else outBuf[k++] = esc;
                    j += 2;
                } else {
                    outBuf[k++] = body[j++];
                }
            }
            outBuf[k] = '\0';
            return 1;
        }
        i++;
    }
    return 0;
}

/* ---- evaluate body ---- */
FirestaffUpdateResult firestaff_update_check_evaluate(
    const char* body, int bodyLen) {
    char tag[64];
    if (firestaff_update_check_disabled()) {
        return FIRESTAFF_UPDATE_DISABLED;
    }
    if (!body || bodyLen <= 0) return FIRESTAFF_UPDATE_PARSE_ERROR;
    if (!firestaff_update_check_extract_tag(body, bodyLen, tag, (int)sizeof(tag))) {
        return FIRESTAFF_UPDATE_PARSE_ERROR;
    }
    const char* cur = M12_Changelog_VersionString();
    int cmp = firestaff_update_check_compare_semver(tag, cur);
    return cmp > 0 ? FIRESTAFF_UPDATE_NEW_AVAILABLE
                   : FIRESTAFF_UPDATE_UP_TO_DATE;
}

int firestaff_update_check_disabled(void) {
    const char* no = SDL_getenv("FIRESTAFF_NO_UPDATE_CHECK");
    if (!no) return 0;
    if (no[0] == '\0' || no[0] == '0') return 0;
    return 1;
}
