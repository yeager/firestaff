#ifndef M11_LOOSE_SIBLING_H
#define M11_LOOSE_SIBLING_H

#include "fs_portable_compat.h"
#include <SDL3/SDL.h>
#include <string.h>

typedef struct M11_LooseSiblingMatch {
    const char* wanted;
    char path[FSP_PATH_MAX];
    int found;
} M11_LooseSiblingMatch;

static SDL_EnumerationResult SDLCALL m11_match_loose_sibling(
    void* opaque, const char* directory, const char* name) {
    M11_LooseSiblingMatch* match = (M11_LooseSiblingMatch*)opaque;
    char candidate[FSP_PATH_MAX];
    if (SDL_strcasecmp(name, match->wanted) != 0) return SDL_ENUM_CONTINUE;
    if (!FSP_JoinPath(candidate, sizeof(candidate), directory, name))
        return SDL_ENUM_FAILURE;
    if (!FSP_FileExists(candidate)) return SDL_ENUM_CONTINUE;
    /* Exact spelling cannot resolve two source siblings differing by case. */
    if (match->found) {
        match->found = 2;
        return SDL_ENUM_FAILURE;
    }
    strcpy(match->path, candidate);
    match->found = 1;
    return SDL_ENUM_CONTINUE;
}

/* Resolve only this directory. Returns 1 for a unique ordinary file, 0 for
 * missing/insufficient output space, -1 for enumeration failure, or -2 for
 * case ambiguity. Neither the directory nor any sibling is searched recursively. */
static inline int m11_find_unique_loose_sibling(
    const char* parent, const char* wanted, char* out, size_t outBytes) {
    M11_LooseSiblingMatch match = {wanted, {0}, 0};
    int enumerated;
    if (!out || !outBytes) return -1;
    out[0] = '\0';
    if (!parent || !wanted || !wanted[0] || strchr(wanted, '/') ||
        strchr(wanted, '\\') || strchr(wanted, ':')) return -1;
    enumerated = SDL_EnumerateDirectory(parent, m11_match_loose_sibling, &match);
    if (match.found == 2) return -2;
    if (!enumerated) return -1;
    if (!match.found || strlen(match.path) >= outBytes) return 0;
    strcpy(out, match.path);
    return 1;
}

#endif
