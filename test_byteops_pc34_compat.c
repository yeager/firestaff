#include <stdio.h>
#include <string.h>

#define COMPILE_H
#define __TURBOC__ 1
#define X463_I34E_I34M 1
#define X465_I34E_I34M 1
#define X736_I34M 1
#define huge
#define HUGE
#define SEPARATOR ,
#define FINAL_SEPARATOR )
#define NULL 0L

void F0007_MAIN_CopyBytes(char* P0005_pc_Source, char* P0006_pc_Destination, long P0007_l_ByteCount);
void F0008_MAIN_ClearBytes(char* P0008_pc_Buffer, unsigned long P0009_i_ByteCount);

#include "byteops_pc34_compat.c"

static int test_copy_forward(void) {
    char src[6] = {'a','b','c','d','e','\0'};
    char dst[6] = {0};
    F0007_MAIN_CopyBytes(src, dst, 6);
    return memcmp(src, dst, 6) == 0;
}

static int test_copy_overlap_backward_safe(void) {
    char buf[8] = {'a','b','c','d','e','f','g','\0'};
    F0007_MAIN_CopyBytes(buf, buf + 2, 5);
    return memcmp(buf, "ababcde", 7) == 0;
}

static int test_copy_overlap_forward_safe(void) {
    char buf[8] = {'a','b','c','d','e','f','g','\0'};
    F0007_MAIN_CopyBytes(buf + 2, buf, 5);
    return memcmp(buf, "cdefgfg", 7) == 0;
}

static int test_clear_bytes(void) {
    char buf[6] = {'x','x','x','x','x','x'};
    F0008_MAIN_ClearBytes(buf + 1, 4);
    return buf[0] == 'x' && buf[1] == 0 && buf[2] == 0 && buf[3] == 0 && buf[4] == 0 && buf[5] == 'x';
}

int main(void) {
    if (!test_copy_forward()) {
        fprintf(stderr, "test_copy_forward failed\n");
        return 1;
    }
    if (!test_copy_overlap_backward_safe()) {
        fprintf(stderr, "test_copy_overlap_backward_safe failed\n");
        return 1;
    }
    if (!test_copy_overlap_forward_safe()) {
        fprintf(stderr, "test_copy_overlap_forward_safe failed\n");
        return 1;
    }
    if (!test_clear_bytes()) {
        fprintf(stderr, "test_clear_bytes failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
