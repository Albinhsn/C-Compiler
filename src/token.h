
#ifndef TOKEN_H
#define TOKEN_H
#include "common.h"

enum TokenType {
  // CONSTANTS
  TOKEN_CHARACTER_CONSTANT,
  TOKEN_FLOAT_CONSTANT,
  TOKEN_FLOAT_HEX_CONSTANT,
  TOKEN_BINARY_CONSTANT,
  TOKEN_OCTAL_CONSTANT,
  TOKEN_INT_CONSTANT,
  TOKEN_INT_HEX_CONSTANT,
  TOKEN_STRING_CONSTANT,

  // KEYWORDS
  TOKEN_AUTO,
  TOKEN_BREAK,
  TOKEN_CASE,
  TOKEN_CHAR,
  TOKEN_CONST,
  TOKEN_CONTINUE,
  TOKEN_DEFAULT,
  TOKEN_DO,
  TOKEN_DOUBLE,
  TOKEN_ELSE,
  TOKEN_ENUM,
  TOKEN_EXTERN,
  TOKEN_FLOAT,
  TOKEN_FOR,
  TOKEN_GOTO,
  TOKEN_IF,
  TOKEN_INLINE,
  TOKEN_INT,
  TOKEN_LONG,
  TOKEN_REGISTER,
  TOKEN_RESTRICT,
  TOKEN_RETURN,
  TOKEN_SHORT,
  TOKEN_SIGNED,
  TOKEN_SIZEOF,
  TOKEN_STATIC,
  TOKEN_STRUCT,
  TOKEN_SWITCH,
  TOKEN_TYPEDEF,
  TOKEN_UNION,
  TOKEN_UNSIGNED,
  TOKEN_VOID,
  TOKEN_VOLATILE,
  TOKEN_WHILE,
  TOKEN_BOOL,
  TOKEN_COMPLEX,
  TOKEN_IMAGINARY,


  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_BRACKET,
  TOKEN_RIGHT_BRACKET,
  TOKEN_ELLIPSIS,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SLASH,
  TOKEN_STAR,
  TOKEN_MOD,
  TOKEN_SHIFT_RIGHT,
  TOKEN_SHIFT_LEFT,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,
  TOKEN_INCREMENT,
  TOKEN_DECREMENT,
  TOKEN_AUGMENTED_MINUS,
  TOKEN_AUGMENTED_PLUS,
  TOKEN_AUGMENTED_SLASH,
  TOKEN_AUGMENTED_STAR,
  TOKEN_AUGMENTED_AND,
  TOKEN_AUGMENTED_OR,
  TOKEN_AUGMENTED_XOR,
  TOKEN_IDENTIFIER,
  TOKEN_AND_LOGICAL,
  TOKEN_OR_LOGICAL,
  TOKEN_AND_BIT,
  TOKEN_OR_BIT,
  TOKEN_XOR,
  TOKEN_EOF

};
typedef enum TokenType TokenType;

struct Token {
  TokenType type;
  String literal;
  i32 index;
  i32 line;
};
typedef struct Token Token;

Token *create_token(Arena *arena, TokenType type, String literal, i32 line,
                    i32 index);
void debug_token(Token *token);
const char *get_token_type_string(TokenType type);

#endif
