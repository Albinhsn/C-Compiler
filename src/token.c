#include "token.h"
#include "common.h"
#include <stdio.h>

Token* create_token(Arena* arena, TokenType type, String literal, int line, int index)
{
  Token* token   = sta_arena_push_struct(arena, Token);
  token->type    = type;
  token->literal = literal;
  token->line    = line;
  token->index   = index;
  return token;
}

const char* get_token_type_string(TokenType type)
{

  static const char* token_names[] = {"TOKEN_CHARACTER_CONSTANT", "TOKEN_FLOAT_CONSTANT", "TOKEN_FLOAT_HEX_CONSTANT", "TOKEN_BINARY_CONSTANT",
                                      "TOKEN_OCTAL_CONSTANT",     "TOKEN_INT_CONSTANT",   "TOKEN_INT_HEX_CONSTANT",   "TOKEN_STRING_CONSTANT"};

  if (type >= ArrayCount(token_names))
  {
    return "UNKNOWN";
  }

  return token_names[type];
}

void debug_token(Token* token)
{

  printf("%.*s", (i32)token->literal.len, token->literal.buffer);
  // printf("%s %.*s @%d\n", token_names[token->type], (int)token->literal.len,
  // token->literal.buffer, token->line);
}
