#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argv;
    if (argc == 1) { puts("SKIP: explicit authenticated plan, CUE, Mednafen trace, MD5s, and three output paths required"); return 0; }
    fputs("REJECTED: this operator probe requires an in-process authenticated three-route plan; it cannot construct capture rows\n", stderr);
    return 1;
}
