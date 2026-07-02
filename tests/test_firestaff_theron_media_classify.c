#include "firestaff_theron_media_classify.h"

#include <stdio.h>

int main(void) {
    if (FirestaffTheronMedia_SelfTest() == 0) {
        printf("test_firestaff_theron_media_classify: PASS\n");
        return 0;
    }
    printf("test_firestaff_theron_media_classify: FAIL\n");
    return 1;
}
