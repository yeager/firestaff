#include <dirent.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    DIR *root; struct dirent *entry; unsigned int bundles = 0u;
    if (argc == 1) { puts("SKIP: explicit capture-campaign corpus root required"); return 0; }
    if (argc != 3 || strcmp(argv[1], "--discover")) { fprintf(stderr, "usage: %s --discover <corpus-root>\n", argv[0]); return 2; }
    root = opendir(argv[2]);
    if (!root) { puts("SKIP: capture-campaign corpus root unavailable"); return 0; }
    while ((entry = readdir(root)) != NULL && bundles < 64u) {
        size_t n = strlen(entry->d_name);
        if (n > 7u && strcmp(entry->d_name + n - 7u, ".bundle") == 0) ++bundles;
    }
    closedir(root);
    if (bundles < 3u) puts("SKIP: fewer than three independent capture bundle candidates");
    else printf("SKIP: %u bundle candidates require authenticated campaign import\n", bundles);
    return 0;
}
