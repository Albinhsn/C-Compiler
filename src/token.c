#include "token.h"
#include "common.h"

Token *create_token(Arena *arena, TokenType type, String literal, int line,
                    int index) {
  Token *token = sta_arena_push_struct(arena, Token);
  token->type = type;
  token->literal = literal;
  token->line = line;
  token->index = index;
  return token;
}

const char *get_token_type_string(TokenType type) {

  static const char *token_names[] = {"TOKEN_VOID",
                                      "TOKEN_STRING",
                                      "TOKEN_STRING_LITERAL",
                                      "TOKEN_BYTE",
                                      "TOKEN_INT",
                                      "TOKEN_FLOAT",
                                      "TOKEN_STRUCT",
                                      "TOKEN_RETURN",
                                      "TOKEN_DOUBLE",
                                      "TOKEN_LONG",
                                      "TOKEN_SHORT",
                                      "TOKEN_LEFT_PAREN",
                                      "TOKEN_RIGHT_PAREN",
                                      "TOKEN_LEFT_BRACE",
                                      "TOKEN_RIGHT_BRACE",
                                      "TOKEN_LEFT_BRACKET",
                                      "TOKEN_RIGHT_BRACKET",
                                      "TOKEN_INCLUDE",
                                      "TOKEN_EXTERN",
                                      "TOKEN_DEFINE",
                                      "TOKEN_ELLIPSIS",
                                      "TOKEN_MINUS",
                                      "TOKEN_PLUS",
                                      "TOKEN_SLASH",
                                      "TOKEN_STAR",
                                      "TOKEN_MOD",
                                      "TOKEN_SHIFT_RIGHT",
                                      "TOKEN_SHIFT_LEFT",
                                      "TOKEN_SEMICOLON",
                                      "TOKEN_COMMA",
                                      "TOKEN_DOT",
                                      "TOKEN_BANG",
                                      "TOKEN_BANG_EQUAL",
                                      "TOKEN_EQUAL",
                                      "TOKEN_EQUAL_EQUAL",
                                      "TOKEN_GREATER",
                                      "TOKEN_GREATER_EQUAL",
                                      "TOKEN_LESS",
                                      "TOKEN_LESS_EQUAL",
                                      "TOKEN_INCREMENT",
                                      "TOKEN_DECREMENT",
                                      "TOKEN_IF",
                                      "TOKEN_ELSE",
                                      "TOKEN_FOR",
                                      "TOKEN_WHILE",
                                      "TOKEN_AUGMENTED_MINUS",
                                      "TOKEN_AUGMENTED_PLUS",
                                      "TOKEN_AUGMENTED_SLASH",
                                      "TOKEN_AUGMENTED_STAR",
                                      "TOKEN_AUGMENTED_AND",
                                      "TOKEN_AUGMENTED_OR",
                                      "TOKEN_AUGMENTED_XOR",
                                      "TOKEN_IDENTIFIER",
                                      "TOKEN_AND_LOGICAL",
                                      "TOKEN_OR_LOGICAL",
                                      "TOKEN_AND_BIT",
                                      "TOKEN_OR_BIT",
                                      "TOKEN_XOR",
                                      "TOKEN_INT_LITERAL",
                                      "TOKEN_FLOAT_LITERAL",
                                      "TOKEN_EOF"

  };

  return token_names[type];
}

void debug_token(Token *token) {

  printf("%.*s", (i32)token->literal.len, token->literal.buffer);
  // printf("%s %.*s @%d\n", token_names[token->type], (int)token->literal.len,
  // token->literal.buffer, token->line);
}
