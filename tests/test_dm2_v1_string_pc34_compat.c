#include "dm2_v1_string_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_scriptchr_uppercase(void)
{
    assert(dm2_v1_skchr_to_scriptchr('A') == 0);
    assert(dm2_v1_skchr_to_scriptchr('Z') == 25);
    assert(dm2_v1_skchr_to_scriptchr('M') == 12);
    printf("test_scriptchr_uppercase OK\n");
}

static void test_scriptchr_dot(void)
{
    assert(dm2_v1_skchr_to_scriptchr('.') == 0x1b);
    printf("test_scriptchr_dot OK\n");
}

static void test_scriptchr_other(void)
{
    assert(dm2_v1_skchr_to_scriptchr('a') == 0x1a);
    assert(dm2_v1_skchr_to_scriptchr('5') == 0x1a);
    printf("test_scriptchr_other OK\n");
}

static void test_ltoa10_positive(void)
{
    char buf[32];
    dm2_v1_ltoa10(12345, buf);
    assert(strcmp(buf, "12345") == 0);
    printf("test_ltoa10_positive OK\n");
}

static void test_ltoa10_zero(void)
{
    char buf[32];
    dm2_v1_ltoa10(0, buf);
    assert(strcmp(buf, "0") == 0);
    printf("test_ltoa10_zero OK\n");
}

static void test_ltoa10_negative(void)
{
    char buf[32];
    dm2_v1_ltoa10(-42, buf);
    assert(strcmp(buf, "-42") == 0);
    printf("test_ltoa10_negative OK\n");
}

static void test_fmt_num_unpadded(void)
{
    char *r = dm2_v1_fmt_num(123, 0, 0);
    assert(strcmp(r, "123") == 0);
    printf("test_fmt_num_unpadded OK\n");
}

static void test_fmt_num_zero(void)
{
    char *r = dm2_v1_fmt_num(0, 0, 0);
    assert(strcmp(r, "0") == 0);
    printf("test_fmt_num_zero OK\n");
}

static void test_fmt_num_padded(void)
{
    char *r = dm2_v1_fmt_num(7, 1, 4);
    assert(strlen(r) == 4);
    assert(r[3] == '7');
    printf("test_fmt_num_padded OK\n");
}

static void test_fill_str(void)
{
    char buf[8] = {0};
    dm2_v1_fill_str(buf, 'X', 1, 4);
    assert(buf[0] == 'X');
    assert(buf[3] == 'X');
    assert(buf[4] == 0);
    printf("test_fill_str OK\n");
}

static void test_fill_str_step2(void)
{
    char buf[8] = {0};
    dm2_v1_fill_str(buf, '#', 2, 3);
    assert(buf[0] == '#');
    assert(buf[1] == 0);
    assert(buf[2] == '#');
    assert(buf[4] == '#');
    printf("test_fill_str_step2 OK\n");
}

int main(void)
{
    test_scriptchr_uppercase();
    test_scriptchr_dot();
    test_scriptchr_other();
    test_ltoa10_positive();
    test_ltoa10_zero();
    test_ltoa10_negative();
    test_fmt_num_unpadded();
    test_fmt_num_zero();
    test_fmt_num_padded();
    test_fill_str();
    test_fill_str_step2();
    printf("All dm2_v1_string tests passed.\n");
    return 0;
}
