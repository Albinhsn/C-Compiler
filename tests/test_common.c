#include "test_common.h"
#include "../src/common.h"
#include <stdio.h>
void print_test_fail_setup(const char* name, const char* msg)
{
  printf("[ %sFAILED%s ]: %s %s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET, name, msg);
}

void print_test_fail(const char* name, const char* expected, const char* got)
{

  char msg[256];
  sprintf(msg, "%s: expected: %s, got: %s", name, expected, got);
  printf("[ %sFAILED%s ]: %s                                   \n", ANSI_COLOR_RED, ANSI_COLOR_RESET, msg);
}
void print_test_complete(const char* msg)
{
  printf("[ %sPASSED%s ]: %s                                   \n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, msg);
}
void print_test_running(const char* msg)
{
  printf("[ RUNNING ]: %s                                      \r", msg);
}
