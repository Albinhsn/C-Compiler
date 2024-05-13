#include "../src/files.h"
#include "../src/scanner.h"
#include "test_common.h"
#include <stdbool.h>
#include <stdlib.h>

static void test_constants()
{
  const char* name = "test_constants";
  print_test_running(name);

  Scanner scanner = {};
  Arena   arena   = {};
  sta_arena_init_heap(&arena, 4096);
  String file = {};

  if (!sta_read_file(&arena, &file, "./tests/test_constants.jc"))
  {
    print_test_fail_setup(name, "Failed to read file \"test_constants.jc\"");
    return;
  }

  init_scanner(&scanner, &arena, &file, "scanner_test.jc");

  TokenType tokens[] = {
      TOKEN_INT_CONSTANT, TOKEN_INT_HEX_CONSTANT, TOKEN_OCTAL_CONSTANT, TOKEN_CHARACTER_CONSTANT, TOKEN_FLOAT_CONSTANT,

  };
  bool      result = true;
  TokenType expected;
  TokenType got;
  for (int i = 0; i < ArrayCount(tokens); i++)
  {
    Token* out = parse_token(&scanner);
    if (tokens[i] != out->type)
    {
      result   = false;
      expected = tokens[i];
      got      = out->type;
      break;
    }
  }
  Token* out = parse_token(&scanner);
  if (out->type != TOKEN_EOF)
  {
    print_test_fail(name, get_token_type_string(TOKEN_EOF), get_token_type_string(out->type));
  }

  if (result)
  {
    print_test_complete(name);
  }
  else
  {
    print_test_fail(name, get_token_type_string(expected), get_token_type_string(got));
  }

  free((void*)arena.memory);
}

void run_scanner_tests()
{
  test_constants();
}
