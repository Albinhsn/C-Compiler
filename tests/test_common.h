#ifndef TEST_COMMON_H
#define TEST_COMMON_H

void print_test_fail(const char* name, const char* expected, const char* got);
void print_test_fail_setup(const char* name, const char * msg);
void print_test_complete(const char *msg);
void print_test_running(const char *msg);

#endif
