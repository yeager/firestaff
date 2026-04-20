#include "card_art_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    const char* gameId;
    const char* slotLabel;
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
    {"dm1", "ORIGINAL DM1 CARD", g_dm1CardCandidates},
    {"csb", "ORIGINAL CSB CARD", g_csbCardCandidates},
    {"dm2", "ORIGINAL DM2 CARD", g_dm2CardCandidates}
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

static void m12_fill_rect(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char color) {
    int row;
    int col;
    if (!framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0 ||
        w <= 0 || h <= 0) {
        return;
    }
    for (row = 0; row < h; ++row) {
        int py = y + row;
        if (py < 0 || py >= framebufferHeight) {
            continue;
        }
        for (col = 0; col < w; ++col) {
            int px = x + col;
            if (px < 0 || px >= framebufferWidth) {
                continue;
            }
            framebuffer[(size_t)py * (size_t)framebufferWidth + (size_t)px] = color;
        }
    }
}

static void m12_draw_frame(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x,
                           int y,
                           int w,
                           int h,
                           unsigned char color) {
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, 1, color);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y + h - 1, w, 1, color);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, 1, h, color);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + w - 1, y, 1, h, color);
}

static void m12_draw_vertical_bars(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   int x,
                                   int y,
                                   int w,
                                   int h,
                                   unsigned char colorA,
                                   unsigned char colorB) {
    int col;
    for (col = 0; col < w; ++col) {
        unsigned char color = (col / 4) % 2 == 0 ? colorA : colorB;
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + col, y, 1, h, color);
    }
}

static void m12_draw_arch(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char outer,
                          unsigned char inner) {
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y + 6, w, h - 6, outer);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 5, y + 10, w - 10, h - 10, inner);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 2, y + 3, w - 4, 4, outer);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 6, y + 1, w - 12, 3, outer);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 10, y, w - 20, 2, outer);
}

static void m12_draw_eye(unsigned char* framebuffer,
                         int framebufferWidth,
                         int framebufferHeight,
                         int x,
                         int y,
                         int w,
                         int h,
                         unsigned char outer,
                         unsigned char iris,
                         unsigned char pupil) {
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 6, w - 8, h - 12, outer);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 8, y + 10, w - 16, h - 20, iris);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + (w / 2) - 4, y + (h / 2) - 4, 8, 8, pupil);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 2, y + 8, 4, h - 16, outer);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + w - 6, y + 8, 4, h - 16, outer);
}

static void m12_draw_keep(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char wall,
                          unsigned char gate,
                          unsigned char sky) {
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h / 3, sky);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + h / 3, w - 8, h - (h / 3), wall);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 8, y + h / 3 - 6, 10, 10, wall);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + w - 18, y + h / 3 - 6, 10, 10, wall);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + (w / 2) - 6, y + h - 16, 12, 16, gate);
}

static void m12_draw_dm1_preview(const M12_GameCardArt* art,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int w,
                                 int h,
                                 unsigned char paper,
                                 unsigned char ink,
                                 unsigned char accent,
                                 unsigned char glow) {
    (void)art;
    m12_draw_vertical_bars(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, paper, ink);
    m12_draw_arch(framebuffer, framebufferWidth, framebufferHeight, x + 8, y + 6, w - 16, h - 10, accent, ink);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + (w / 2) - 2, y + 8, 4, 18, glow);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + (w / 2) - 6, y + 22, 12, 4, glow);
}

static void m12_draw_csb_preview(const M12_GameCardArt* art,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int w,
                                 int h,
                                 unsigned char paper,
                                 unsigned char ink,
                                 unsigned char accent,
                                 unsigned char glow) {
    (void)art;
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, ink);
    m12_draw_frame(framebuffer, framebufferWidth, framebufferHeight, x + 2, y + 2, w - 4, h - 4, accent);
    m12_draw_eye(framebuffer, framebufferWidth, framebufferHeight, x + 8, y + 8, w - 16, h - 16, paper, glow, accent);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 10, y + h - 9, w - 20, 3, paper);
}

static void m12_draw_dm2_preview(const M12_GameCardArt* art,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int w,
                                 int h,
                                 unsigned char paper,
                                 unsigned char ink,
                                 unsigned char accent,
                                 unsigned char glow) {
    (void)art;
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, paper);
    m12_draw_keep(framebuffer, framebufferWidth, framebufferHeight, x + 6, y + 5, w - 12, h - 8, accent, ink, glow);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + w - 14, y + 4, 8, 8, glow);
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
    if (!spec) {
        return;
    }

    art->hasImageFile = 1;
    m12_copy_text(art->slotLabel, sizeof(art->slotLabel), spec->slotLabel);
    m12_copy_text(art->fileName, sizeof(art->fileName), "internal-original");

    if (!dataDir || dataDir[0] == '\0') {
        return;
    }
    for (i = 0U; spec->candidates[i] != NULL; ++i) {
        if (!m12_join_path(art->resolvedPath,
                           sizeof(art->resolvedPath),
                           dataDir,
                           spec->candidates[i])) {
            continue;
        }
        if (m12_file_exists(art->resolvedPath)) {
            art->hasExternalFile = 1;
            m12_copy_text(art->fileName,
                          sizeof(art->fileName),
                          m12_basename(spec->candidates[i]));
            return;
        }
    }
    art->resolvedPath[0] = '\0';
}

int M12_CardArt_HasImage(const M12_GameCardArt* art) {
    return art ? art->hasImageFile : 0;
}

int M12_CardArt_HasExternalFile(const M12_GameCardArt* art) {
    return art ? art->hasExternalFile : 0;
}

const char* M12_CardArt_GetResolvedPath(const M12_GameCardArt* art) {
    return art && art->resolvedPath[0] != '\0' ? art->resolvedPath : "";
}

const char* M12_CardArt_GetFileName(const M12_GameCardArt* art) {
    return art && art->fileName[0] != '\0' ? art->fileName : "";
}

const char* M12_CardArt_GetSlotLabel(const M12_GameCardArt* art) {
    return art && art->slotLabel[0] != '\0' ? art->slotLabel : "";
}

void M12_CardArt_DrawPreview(const M12_GameCardArt* art,
                             unsigned char* framebuffer,
                             int framebufferWidth,
                             int framebufferHeight,
                             int x,
                             int y,
                             int w,
                             int h,
                             unsigned char paper,
                             unsigned char ink,
                             unsigned char accent,
                             unsigned char glow) {
    if (!art || !framebuffer || w <= 0 || h <= 0) {
        return;
    }
    if (strcmp(art->gameId, "dm1") == 0) {
        m12_draw_dm1_preview(art,
                             framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             x,
                             y,
                             w,
                             h,
                             paper,
                             ink,
                             accent,
                             glow);
        return;
    }
    if (strcmp(art->gameId, "csb") == 0) {
        m12_draw_csb_preview(art,
                             framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             x,
                             y,
                             w,
                             h,
                             paper,
                             ink,
                             accent,
                             glow);
        return;
    }
    if (strcmp(art->gameId, "dm2") == 0) {
        m12_draw_dm2_preview(art,
                             framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             x,
                             y,
                             w,
                             h,
                             paper,
                             ink,
                             accent,
                             glow);
        return;
    }
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, paper);
    m12_draw_frame(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, accent);
}
