#include "redmcsb_f0798_command_is_point_in_zone_pc34_compat.h"

#include <string.h>

static int expect(int condition, int failure_code)
{
    return condition ? 0 : failure_code;
}

int main(void)
{
    const int16_t zone[4] = {10, 20, 3, 4};
    const int16_t point_zone[4] = {-4, 7, 1, 1};
    const int16_t empty_width[4] = {10, 20, 0, 4};
    const int16_t empty_height[4] = {10, 20, 3, 0};
    int failure;

    failure = expect(
        redmcsb_f0798_command_is_point_in_zone_pc34_compat(zone, 11, 22), 1);
    if (failure) {
        return failure;
    }
    failure = expect(
        redmcsb_f0798_command_is_point_in_zone_pc34_compat(zone, 10, 20), 2);
    if (failure) {
        return failure;
    }
    failure = expect(
        redmcsb_f0798_command_is_point_in_zone_pc34_compat(zone, 12, 23), 3);
    if (failure) {
        return failure;
    }
    failure = expect(
        !redmcsb_f0798_command_is_point_in_zone_pc34_compat(zone, 9, 22), 4);
    if (failure) {
        return failure;
    }
    failure = expect(
        !redmcsb_f0798_command_is_point_in_zone_pc34_compat(zone, 13, 22), 5);
    if (failure) {
        return failure;
    }
    failure = expect(
        !redmcsb_f0798_command_is_point_in_zone_pc34_compat(zone, 11, 19), 6);
    if (failure) {
        return failure;
    }
    failure = expect(
        !redmcsb_f0798_command_is_point_in_zone_pc34_compat(zone, 11, 24), 7);
    if (failure) {
        return failure;
    }
    failure = expect(
        redmcsb_f0798_command_is_point_in_zone_pc34_compat(point_zone, -4, 7), 8);
    if (failure) {
        return failure;
    }
    failure = expect(
        !redmcsb_f0798_command_is_point_in_zone_pc34_compat(empty_width, 10, 20), 9);
    if (failure) {
        return failure;
    }
    failure = expect(
        !redmcsb_f0798_command_is_point_in_zone_pc34_compat(empty_height, 10, 20), 10);
    if (failure) {
        return failure;
    }
    return expect(
        strstr(redmcsb_f0798_command_is_point_in_zone_source_evidence_pc34(),
               "COORD.C:1915-1920") != 0,
        11);
}
