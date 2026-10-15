#include "asset_status_m12.h"
#include "firestaff_amiga_adf.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define CTRAW_LIMIT (16u * 1024u * 1024u)

typedef struct {
    const char *names[5];
    const char *hashes[5];
    unsigned int verified;
    unsigned int visited;
} OriginalFiles;

static int name_equal(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a++) != tolower((unsigned char)*b++))
            return 0;
    }
    return *a == *b;
}

static int verify_original(const char *name, const uint8_t *bytes,
                            size_t size, void *user)
{
    OriginalFiles *files = (OriginalFiles *)user;
    unsigned int i;
    if (!name || !bytes || !size || !files) return -1;
    ++files->visited;
    for (i = 0u; i < 5u; ++i) {
        char actual[33];
        if (!name_equal(name, files->names[i])) continue;
        if ((files->verified & (1u << i)) ||
            !m12_bytes_md5_hex(bytes, size, actual) ||
            strcmp(actual, files->hashes[i]) != 0) {
            fprintf(stderr, "FAIL: original CTRaw file identity rejected: %s\n", name);
            return -1;
        }
        files->verified |= 1u << i;
        printf("verified file=%s bytes=%zu md5=%s\n", name, size, actual);
        break;
    }
    return 0;
}

int main(int argc, char **argv)
{
    /* Original A35 startup identities are independently recorded in
     * test_csb_archive_required_split.c. ReDMCSB COMPILE.H:274-298 separates
     * the English direct APPB game from the multilingual KAOS/ANIM layout. */
    OriginalFiles english = {
        {"Graphics.DAT", "APPB.FTL", "BJELoad_R", "ANIM.FTL", "VDEO.FTL"},
        {"291e1bc6803e3dc4b974c60117ca5d68", "11d8d059cd8f241d6f68ec09c5c8b66d",
         "6aec320606f014a149548e664719cf5b", "60ffbbe31830f2fe262cb8dee862b7fc",
         "d86f045085481448f6ae3db8d3d640d2"}, 0u, 0u
    };
    OriginalFiles multi = {
        {"Graphics.DAT", "KAOS.FTL", "ANIM.FTL", "APPB.FTL", "TITL.DAT"},
        {"cefaddfdf5651df2c91f61b5611a8362", "229d3253968d6e616f8b5171efdf04a5",
         "0685af41f4ec2d941eb6450d0f331b2c", "1533410beaeea4fa614ae0f0142e0861",
         "5b590ea3a6f5eed513b5678b01468ee4"}, 0u, 0u
    };
    OriginalFiles *files;
    uint8_t *image;
    size_t size;
    size_t expected;
    int result;
    if (argc != 2 || (strcmp(argv[1], "en") && strcmp(argv[1], "multi"))) {
        fputs("usage: test_firestaff_amiga_ctraw_real_media en|multi < original.raw\n", stderr);
        return 1;
    }
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1) return 1;
#endif
    files = strcmp(argv[1], "en") == 0 ? &english : &multi;
    expected = files == &english ? 3158314u : 3331587u;
    image = (uint8_t *)malloc(CTRAW_LIMIT);
    if (!image) return 1;
    size = fread(image, 1u, CTRAW_LIMIT, stdin);
    if (ferror(stdin) || size != expected || fgetc(stdin) != EOF ||
        size < 4u || memcmp(image, "CAPS", 4u) != 0) {
        fprintf(stderr, "FAIL: unexpected original CTRaw input size/signature: %zu\n", size);
        free(image);
        return 1;
    }
    result = firestaff_amiga_adf_visit_ofs_files(image, size, verify_original, files);
    free(image);
    if (result < 0 || files->verified != 0x1fu ||
        (unsigned int)result != files->visited) {
        fprintf(stderr, "FAIL: original CTRaw OFS edition=%s files=%d verified=0x%x\n",
                argv[1], result, files->verified);
        return 1;
    }
    printf("PASS: original CSB Amiga 3.5 CTRaw edition=%s files=%u all-five-identities=verified\n",
           argv[1], files->visited);
    return 0;
}
