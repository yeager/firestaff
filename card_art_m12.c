#include "card_art_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    const char* gameId;
    const char* const* candidates;
} M12_CardArtSpec;

static const char* const g_dm1CardCandidates[] = {
    "cards/dm1.png",
    "cards/dm1.jpg",
    "cards/dm1.jpeg",
    "cards/dm1.webp",
    "cards/dungeon-master.png",
    NULL
};

static const char* const g_csbCardCandidates[] = {
    "cards/csb.png",
    "cards/csb.jpg",
    "cards/csb.jpeg",
    "cards/csb.webp",
    "cards/chaos-strikes-back.png",
    NULL
};

static const char* const g_dm2CardCandidates[] = {
    "cards/dm2.png",
    "cards/dm2.jpg",
    "cards/dm2.jpeg",
    "cards/dm2.webp",
    "cards/skullkeep.png",
    NULL
};

static const M12_CardArtSpec g_cardSpecs[] = {
    {"dm1", g_dm1CardCandidates},
    {"csb", g_csbCardCandidates},
    {"dm2", g_dm2CardCandidates}
};

static void m12_copy_text(char* out, size_t outSize, const char* value) {
    if (!out || outSize == 0U) {
        return;
    }
    snprintf(out, outSize, "%s", value ? value : "");
}

static int m12_join_path(char* out,
                         size_t outSize,
                         const char* dir,
                         const char* leaf) {
    int rc;
    size_t dirLen;
    if (!out || outSize == 0U || !dir || !leaf) {
        return 0;
    }
    dirLen = strlen(dir);
    rc = snprintf(out,
                  outSize,
                  "%s%s%s",
                  dir,
                  (dirLen > 0U && dir[dirLen - 1U] == '/') ? "" : "/",
                  leaf);
    return rc > 0 && (size_t)rc < outSize;
}

static int m12_file_exists(const char* path) {
    struct stat st;
    return path && path[0] != '\0' && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static const M12_CardArtSpec* m12_find_card_spec(const char* gameId) {
    size_t i;
    if (!gameId) {
        return NULL;
    }
    for (i = 0U; i < sizeof(g_cardSpecs) / sizeof(g_cardSpecs[0]); ++i) {
        if (strcmp(g_cardSpecs[i].gameId, gameId) == 0) {
            return &g_cardSpecs[i];
        }
    }
    return NULL;
}

static const char* m12_basename(const char* path) {
    const char* slash;
    if (!path) {
        return "";
    }
    slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

void M12_CardArt_Resolve(M12_GameCardArt* art,
                         const char* gameId,
                         const char* dataDir) {
    const M12_CardArtSpec* spec;
    size_t i;
    if (!art) {
        return;
    }
    memset(art, 0, sizeof(*art));
    m12_copy_text(art->gameId, sizeof(art->gameId), gameId);
    spec = m12_find_card_spec(gameId);
    if (!spec || !dataDir || dataDir[0] == '\0') {
        return;
    }
    for (i = 0U; spec->candidates[i] != NULL; ++i) {
        if (!m12_join_path(art->resolvedPath,
                           sizeof(art->resolvedPath),
                           dataDir,
                           spec->candidates[i])) {
            continue;
        }
        m12_copy_text(art->fileName,
                      sizeof(art->fileName),
                      m12_basename(spec->candidates[i]));
        if (m12_file_exists(art->resolvedPath)) {
            art->hasImageFile = 1;
            return;
        }
    }
}

int M12_CardArt_HasImage(const M12_GameCardArt* art) {
    return art ? art->hasImageFile : 0;
}

const char* M12_CardArt_GetResolvedPath(const M12_GameCardArt* art) {
    return art && art->resolvedPath[0] != '\0' ? art->resolvedPath : "";
}

const char* M12_CardArt_GetFileName(const M12_GameCardArt* art) {
    return art && art->fileName[0] != '\0' ? art->fileName : "";
}
