
#include "scanner.h"
#include "common.h"
#include "stdbool.h"
#include "token.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(const char* message)
{
  printf("%s\n", message);
  exit(1);
}

void init_scanner(Scanner* scanner, Arena* arena, String* literal, const char* filename)
{
  scanner->filename = (u8*)filename;
  scanner->index    = 0;
  scanner->line     = 1;
  scanner->input    = literal;
  scanner->arena    = arena;
}

static bool match_substring(Scanner* scanner, const char* to_match, int length)
{
  String* literal = scanner->input;
  if (literal->len <= scanner->index + length)
  {
    return false;
  }
  if (strncmp(&literal->buffer[scanner->index], to_match, length) == 0)
  {
    scanner->index += length;
  }
  return false;
}

static bool is_out_of_bounds(Scanner* scanner)
{
  return scanner->input->len <= scanner->index;
}

static u8 current_char(Scanner* scanner)
{
  return scanner->input->buffer[scanner->index];
}

static u8 advance(Scanner* scanner)
{
  char out = current_char(scanner);
  scanner->index++;
  return out;
}

static bool match_next(Scanner* scanner, char toMatch)
{
  if (current_char(scanner) == toMatch)
  {
    scanner->index++;
    return true;
  };
  return false;
}

static void skip_whitespace(Scanner* scanner)
{

  while (true)
  {
    if (is_out_of_bounds(scanner))
    {
      return;
    }
    u8 current = current_char(scanner);
    if (isdigit(current) || isalpha(current))
    {
      return;
    }
    switch (current)
    {
    case '/':
    {
      advance(scanner);
      if (match_next(scanner, '/'))
      {
        while (!is_out_of_bounds(scanner) && current_char(scanner) != '\n')
        {
          advance(scanner);
        }
        scanner->line++;
        break;
      }
      scanner->index--;
      return;
    }

    case '\"':
    case '{':
    case '[':
    case ']':
    case '}':
    case '*':
    case ';':
    case '<':
    case '>':
    case '(':
    case ')':
    case '!':
    case '%':
    case '.':
    case '#':
    case '^':
    case ':':
    case ',':
    case '&':
    case '|':
    case '-':
    case '+':
    case '=':
    case '\'':
    {
      return;
    }
    case '\n':
    {
      scanner->line++;
    }
    default:
    {
      scanner->index++;
    }
    }
  }
}

static Token* parse_string(Scanner* scanner)
{
  i32 index   = scanner->index;
  u8  current = advance(scanner);

  while (!is_out_of_bounds(scanner) && current != '\"')
  {
    current = advance(scanner);
  }
  if (is_out_of_bounds(scanner))
  {
    error("Unterminated string!");
  }
  String literal = {};
  literal.buffer = (char*)&scanner->input->buffer[index];
  literal.len    = scanner->index - index - 1;

  return create_token(scanner->arena, TOKEN_STRING_CONSTANT, literal, scanner->line, scanner->index);
}

static TokenType get_keyword(String literal)
{
  static const char* keywords[] = {"auto",     "break",  "case",     "char",  "const",    "continue", "default",
                                   "do",    "double", "else",   "enum",   "extern", "float",  "for",     "goto",  "if",       "inline", "int",      "long",  "register", "restrict", "return",
                                   "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsinged", "void",   "volatile", "while", "_Bool",    "_Complex", "_Imaginary"};
  static TokenType   tokens[]   = {
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
  };
  u32 number_of_keywords = ArrayCount(keywords);
  for (u32 i = 0; i < number_of_keywords; i++)
  {
    if (literal.len == strlen(keywords[i]) && strncmp(keywords[i], literal.buffer, literal.len) == 0)
    {
      return tokens[i];
    }
  }
  return TOKEN_IDENTIFIER;
}

static Token* parse_keyword(Scanner* scanner)
{
  i32 index   = scanner->index - 1;
  u8  current = advance(scanner);
  while (!is_out_of_bounds(scanner) && (isdigit(current) || isalpha(current) || current == '_'))
  {
    current = advance(scanner);
  }

  String literal = {};
  literal.buffer = (char*)&scanner->input->buffer[index];
  scanner->index--;
  literal.len = scanner->index - index;
  return create_token(scanner->arena, get_keyword(literal), literal, scanner->line, scanner->index);
}

static bool is_valid_hex(char c)
{
  return ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}

static Token* parse_hex(Scanner* scanner, i32 index)
{
  char      current = advance(scanner);
  TokenType type    = TOKEN_INT_HEX_CONSTANT;
  while (!is_out_of_bounds(scanner) && (isdigit(current) || is_valid_hex(current)))
  {
    current = advance(scanner);
  }

  if (current == '.')
  {
    type    = TOKEN_FLOAT_HEX_CONSTANT;
    current = advance(scanner);
    while (!is_out_of_bounds(scanner) && (isdigit(current) || is_valid_hex(current)))
    {
      current = advance(scanner);
    }

    // binary exponent part
    if (current != 'p')
    {
      error("Expected p after float hex");
    }
    // sign
    match_next(scanner, '-');
    match_next(scanner, '+');
    // digit_sequence
    current = current_char(scanner);
    while (!is_out_of_bounds(scanner) && isdigit(current))
    {
      current = advance(scanner);
    }
  }

  scanner->index--;
  String literal = {};
  literal.buffer = (char*)&scanner->input->buffer[index];
  literal.len    = scanner->index - index;

  return create_token(scanner->arena, type, literal, scanner->line, scanner->index);
}

static Token* parse_binary(Scanner* scanner, i32 index)
{
  char current = advance(scanner);
  while (!is_out_of_bounds(scanner) && (current == '0' || current == '1'))
  {
    current = advance(scanner);
  }
  scanner->index--;
  String literal = {};
  literal.buffer = (char*)&scanner->input->buffer[index];
  literal.len    = scanner->index - index;
  return create_token(scanner->arena, TOKEN_INT_CONSTANT, literal, scanner->line, scanner->index);
}

static Token* parse_pound_define(Scanner* scanner)
{
  char current = current_char(scanner);
  while (!is_out_of_bounds(scanner) && !(isalpha(current) || isdigit(current)))
  {
  }
  return 0;
}

static inline bool is_valid_octal(char current)
{
  return '0' <= current && current <= '7';
}

static Token* parse_octal(Scanner* scanner, int index)
{
  char current = advance(scanner);
  while (!is_out_of_bounds(scanner) && is_valid_octal(current))
  {
    current = advance(scanner);
  }

  scanner->index--;
  String literal = {};
  literal.buffer = (char*)&scanner->input->buffer[index];
  literal.len    = scanner->index - index;
  return create_token(scanner->arena, TOKEN_OCTAL_CONSTANT, literal, scanner->line, scanner->index);
}

static Token* parse_number(Scanner* scanner)
{
  // integer-constant
  //  decimal integer-suffix (opt)
  //  octal-constant integer-suffix (opt)
  //  hexadecimal-constant integer-suffix (opt)

  // floating-constant
  //  decimal-floating-constant
  //  hexadecimal-floating-constant
  i32       index   = scanner->index - 1;
  TokenType type    = TOKEN_INT_CONSTANT;

  char      current = advance(scanner);
  if (scanner->input->buffer[index] == '0')
  {
    if (current == 'x' || current == 'X')
    {
      return parse_hex(scanner, index);
    }
    else if (current == 'b')
    {
      return parse_binary(scanner, index);
    }
    else if (isdigit(current))
    {
      return parse_octal(scanner, index);
    }
  }

  while (!is_out_of_bounds(scanner) && isdigit(current))
  {
    current = advance(scanner);
  }

  if (current == '.')
  {
    current = current_char(scanner);
    type    = TOKEN_FLOAT_CONSTANT;

    while (!is_out_of_bounds(scanner) && isdigit(current))
    {
      current = advance(scanner);
    }
  }
  scanner->index--;
  String literal = {};
  literal.buffer = (char*)&scanner->input->buffer[index];
  literal.len    = scanner->index - index;
  return create_token(scanner->arena, type, literal, scanner->line, scanner->index);
}

Token* parse_character(Scanner* scanner)
{
  String literal = {};
  literal.buffer = (char*)&scanner->input->buffer[scanner->index];
  literal.len    = 1;
  advance(scanner);
  if (!match_next(scanner, '\''))
  {
    error("Expected ' after character?");
  }
  return create_token(scanner->arena, TOKEN_CHARACTER_CONSTANT, literal, scanner->line, scanner->index);
}

Token* parse_token(Scanner* scanner)
{
  skip_whitespace(scanner);
  if (is_out_of_bounds(scanner))
  {
    String literal = {};
    literal.buffer = "EOF";
    literal.len    = 3;
    return create_token(scanner->arena, TOKEN_EOF, literal, scanner->line, scanner->index);
  }

  char current = advance(scanner);

  if (isalpha(current))
  {
    return parse_keyword(scanner);
  }

  if (isdigit(current))
  {
    return parse_number(scanner);
  }

  switch (current)
  {
  case '{':
  {
    String literal = {};
    literal.buffer = "{";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_LEFT_BRACE, literal, scanner->line, scanner->index);
  }
  case '}':
  {
    String literal = {};
    literal.buffer = "}";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_RIGHT_BRACE, literal, scanner->line, scanner->index);
  }
  case '[':
  {
    String literal = {};
    literal.buffer = "[";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_LEFT_BRACKET, literal, scanner->line, scanner->index);
  }
  case '\'':
  {
    return parse_character(scanner);
  }
  case ']':
  {
    String literal = {};
    literal.buffer = "]";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_RIGHT_BRACKET, literal, scanner->line, scanner->index);
  }
  case ';':
  {
    String literal = {};
    literal.buffer = ";";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_SEMICOLON, literal, scanner->line, scanner->index);
  }
  case '%':
  {
    String literal = {};
    literal.buffer = "%";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_MOD, literal, scanner->line, scanner->index);
  }
  case ',':
  {
    String literal = {};
    literal.buffer = ",";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_COMMA, literal, scanner->line, scanner->index);
  }
  case '(':
  {
    String literal = {};
    literal.buffer = "(";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_LEFT_PAREN, literal, scanner->line, scanner->index);
  }
  case ')':
  {
    String literal = {};
    literal.buffer = ")";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_RIGHT_PAREN, literal, scanner->line, scanner->index);
  }
  case '/':
  {
    String literal = {};
    literal.buffer = "/";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_SLASH, literal, scanner->line, scanner->index);
  }
  case '*':
  {
    String literal = {};
    literal.buffer = "*";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_STAR, literal, scanner->line, scanner->index);
  }
  case '&':
  {
    String literal = {};
    if (match_next(scanner, '&'))
    {
      literal.buffer = "&&";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_AND_LOGICAL, literal, scanner->line, scanner->index);
    }

    literal.buffer = "&";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_AND_BIT, literal, scanner->line, scanner->index);
  }
  case '|':
  {
    String literal = {};
    if (match_next(scanner, '|'))
    {
      literal.buffer = "||";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_OR_LOGICAL, literal, scanner->line, scanner->index);
    }

    literal.buffer = "|";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_OR_BIT, literal, scanner->line, scanner->index);
  }
  case '!':
  {
    String literal = {};
    if (match_next(scanner, '='))
    {
      literal.buffer = "!=";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_BANG_EQUAL, literal, scanner->line, scanner->index);
    }
    literal.buffer = "!";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_BANG, literal, scanner->line, scanner->index);
  }
  case '>':
  {
    String literal = {};
    if (match_next(scanner, '='))
    {
      literal.buffer = ">=";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_GREATER_EQUAL, literal, scanner->line, scanner->index);
    }
    else if (match_next(scanner, '>'))
    {
      literal.buffer = ">>";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_SHIFT_RIGHT, literal, scanner->line, scanner->index);
    }
    literal.buffer = ">";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_GREATER, literal, scanner->line, scanner->index);
  }
  case '-':
  {
    String literal = {};
    if (match_next(scanner, '-'))
    {
      literal.buffer = "--";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_DECREMENT, literal, scanner->line, scanner->index);
    }
    literal.buffer = "-";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_MINUS, literal, scanner->line, scanner->index);
  }
  case '+':
  {
    String literal = {};
    if (match_next(scanner, '+'))
    {
      literal.buffer = "++";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_INCREMENT, literal, scanner->line, scanner->index);
    }
    literal.buffer = "+";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_PLUS, literal, scanner->line, scanner->index);
  }
  case '=':
  {
    String literal = {};
    if (match_next(scanner, '='))
    {
      literal.buffer = "==";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_EQUAL_EQUAL, literal, scanner->line, scanner->index);
    }
    literal.buffer = "=";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_EQUAL, literal, scanner->line, scanner->index);
  }
  case '<':
  {
    String literal = {};
    if (match_next(scanner, '='))
    {
      literal.buffer = "<=";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_LESS_EQUAL, literal, scanner->line, scanner->index);
    }
    else if (match_next(scanner, '<'))
    {
      literal.buffer = "<<";
      literal.len    = 2;
      return create_token(scanner->arena, TOKEN_SHIFT_LEFT, literal, scanner->line, scanner->index);
    }
    literal.buffer = "<";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_LESS, literal, scanner->line, scanner->index);
  }
  case '^':
  {
    String literal = {};
    literal.buffer = "^";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_XOR, literal, scanner->line, scanner->index);
  }
  case '.':
  {
    String literal      = {};
    int    match_length = 2;
    if (isdigit(current_char(scanner)))
    {
      // .5;
    }
    else if (match_substring(scanner, "..", match_length))
    {
      literal.buffer = "...";
      literal.len    = 3;
      return create_token(scanner->arena, TOKEN_ELLIPSIS, literal, scanner->line, scanner->index);
    }
    literal.buffer = ".";
    literal.len    = 1;
    return create_token(scanner->arena, TOKEN_DOT, literal, scanner->line, scanner->index);
  }
  case '\"':
  {
    return parse_string(scanner);
  }
  case '#':
  {
    return parse_pound_define(scanner);
  }
  default:
  {
    printf("%c %d\n", scanner->input->buffer[scanner->index], scanner->input->buffer[scanner->index]);
    error("Illegal character!");
    exit(1);
  }
  }
}
