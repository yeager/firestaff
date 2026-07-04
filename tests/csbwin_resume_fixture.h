#ifndef FIRESTAFF_TESTS_CSBWIN_RESUME_FIXTURE_H
#define FIRESTAFF_TESTS_CSBWIN_RESUME_FIXTURE_H

#include <stddef.h>
#include <stdint.h>

size_t firestaff_test_build_csbwin_resume_fixture(uint8_t *buf,
                                                  size_t capacity,
                                                  int corrupt_timer_queue);
int firestaff_test_write_csbwin_resume_fixture(const char *path,
                                               int corrupt_timer_queue);

#endif
