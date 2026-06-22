#include "firestaff_ftl_container.h"

#include <stdio.h>

int main(void) {
    if (FirestaffFtlContainer_SelfTest() == 0) {
        printf("test_firestaff_ftl_container: PASS\n");
        return 0;
    }
    printf("test_firestaff_ftl_container: FAIL\n");
    return 1;
}
