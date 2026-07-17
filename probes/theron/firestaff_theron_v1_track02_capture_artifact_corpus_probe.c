#include <dirent.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    DIR *root; struct dirent *entry; unsigned int bundles = 0u;
    if (argc == 1) { puts("SKIP: explicit external capture-artifact corpus root required"); return 0; }
    if (argc != 3 || strcmp(argv[1], "--discover")) { fprintf(stderr, "usage: %s --discover <corpus-root>\n", argv[0]); return 2; }
    root = opendir(argv[2]);
    if (!root) { puts("SKIP: capture-artifact corpus root unavailable"); return 0; }
    while ((entry = readdir(root)) != NULL && bundles < 64u) {
        size_t bytes = strlen(entry->d_name);
        if (bytes > 7u && strcmp(entry->d_name + bytes - 7u, ".bundle") == 0) ++bundles;
    }
    closedir(root);
    if (!bundles) puts("SKIP: no external capture-artifact bundles found");
    else printf("SKIP: %u bundle candidate(s) require an authenticated three-route plan and Mednafen trace\n", bundles);
    return 0;
}
