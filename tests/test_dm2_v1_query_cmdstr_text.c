#include "dm2_v1_weather_gdat.h"

#include <assert.h>
#include <stdint.h>

static void test_query_cmdstr_text_cd_fw(void)
{
    static const uint8_t text[] = "AA=1 CD=42 FW=3";
    int found = 0;
    int32_t value = 0;

    assert(dm2_v1_weather_cmdstr_query(
               text, sizeof(text), "CD", &found, &value) == 1);
    assert(found == 1);
    assert(value == 42);

    assert(dm2_v1_weather_cmdstr_query(
               text, sizeof(text), "FW", &found, &value) == 1);
    assert(found == 1);
    assert(value == 3);
}

static void test_query_cmdstr_text_missing_and_signed(void)
{
    static const uint8_t text[] = "CD=-7";
    int found = 0;
    int32_t value = 0;

    assert(dm2_v1_weather_cmdstr_query(
               text, sizeof(text), "FW", &found, &value) == 1);
    assert(found == 0);
    assert(value == 0);

    assert(dm2_v1_weather_cmdstr_query(
               text, sizeof(text), "CD", &found, &value) == 1);
    assert(found == 1);
    assert(value == -7);
}

static void test_query_cmdstr_text_fail_closed(void)
{
    static const uint8_t no_nul[] = { 'C', 'D', '=', '1' };
    static const uint8_t overflow[] = "CD=999999999999999999999";
    int found = 0;
    int32_t value = 0;

    assert(dm2_v1_weather_cmdstr_query(
               no_nul, sizeof(no_nul), "CD", &found, &value) == 0);
    assert(dm2_v1_weather_cmdstr_query(
               overflow, sizeof(overflow), "CD", &found, &value) == 0);
}

int main(void)
{
    test_query_cmdstr_text_cd_fw();
    test_query_cmdstr_text_missing_and_signed();
    test_query_cmdstr_text_fail_closed();
    return 0;
}
