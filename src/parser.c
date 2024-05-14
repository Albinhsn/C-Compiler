#include "parser.h"

void init_parser(Parser* parser, Arena* arena, Scanner* scanner)
{
  parser->arena    = arena;
  parser->scanner  = scanner;
  parser->current  = 0;
  parser->previous = 0;
  parser->head     = 0;
}
static void advance(Parser* parser)
{
  parser->previous = parser->current;
  parser->current  = parse_token(parser->scanner);
  printf("New current %.*s\n", (i32)parser->current->literal.len, parser->current->literal.buffer);
}

bool is_at_end(Parser* parser)
{
  return parser->current->type == TOKEN_EOF;
}

void parse(Parser* parser)
{
  do
  {
    advance(parser);
    switch (parser->current->type)
    {
    case TOKEN_CHARACTER_CONSTANT:
    {
    }
    case TOKEN_FLOAT_CONSTANT:
    {
    }
    case TOKEN_FLOAT_HEX_CONSTANT:
    {
    }
    case TOKEN_BINARY_CONSTANT:
    {
    }
    case TOKEN_OCTAL_CONSTANT:
    {
    }
    case TOKEN_INT_CONSTANT:
    {
    }
    case TOKEN_INT_HEX_CONSTANT:
    {
    }
    case TOKEN_STRING_CONSTANT:
    {
    }
    case TOKEN_UNSIGNED_POSTFIX:
    {
    }
    case TOKEN_LONG_POSTFIX:
    {
    }
    case TOKEN_LONG_LONG_POSTFIX:
    {
    }
    case TOKEN_FLOAT_POSTFIX:
    {
    }
    case TOKEN_AUTO:
    {
    }
    case TOKEN_BREAK:
    {
    }
    case TOKEN_CASE:
    {
    }
    case TOKEN_CHAR:
    {
    }
    case TOKEN_CONST:
    {
    }
    case TOKEN_CONTINUE:
    {
    }
    case TOKEN_DEFAULT:
    {
    }
    case TOKEN_DO:
    {
    }
    case TOKEN_DOUBLE:
    {
    }
    case TOKEN_ELSE:
    {
    }
    case TOKEN_ENUM:
    {
    }
    case TOKEN_EXTERN:
    {
    }
    case TOKEN_FLOAT:
    {
    }
    case TOKEN_FOR:
    {
    }
    case TOKEN_GOTO:
    {
    }
    case TOKEN_IF:
    {
    }
    case TOKEN_INLINE:
    {
    }
    case TOKEN_INT:
    {
    }
    case TOKEN_LONG:
    {
    }
    case TOKEN_REGISTER:
    {
    }
    case TOKEN_RESTRICT:
    {
    }
    case TOKEN_RETURN:
    {
    }
    case TOKEN_SHORT:
    {
    }
    case TOKEN_SIGNED:
    {
    }
    case TOKEN_SIZEOF:
    {
    }
    case TOKEN_STATIC:
    {
    }
    case TOKEN_STRUCT:
    {
    }
    case TOKEN_SWITCH:
    {
    }
    case TOKEN_TYPEDEF:
    {
    }
    case TOKEN_UNION:
    {
    }
    case TOKEN_UNSIGNED:
    {
    }
    case TOKEN_VOID:
    {
    }
    case TOKEN_VOLATILE:
    {
    }
    case TOKEN_WHILE:
    {
    }
    case TOKEN_BOOL:
    {
    }
    case TOKEN_COMPLEX:
    {
    }
    case TOKEN_IMAGINARY:
    {
    }

    case TOKEN_LEFT_PAREN:
    {
    }
    case TOKEN_RIGHT_PAREN:
    {
    }
    case TOKEN_LEFT_BRACE:
    {
    }
    case TOKEN_RIGHT_BRACE:
    {
    }
    case TOKEN_LEFT_BRACKET:
    {
    }
    case TOKEN_RIGHT_BRACKET:
    {
    }
    case TOKEN_ELLIPSIS:
    {
    }
    case TOKEN_MINUS:
    {
    }
    case TOKEN_PLUS:
    {
    }
    case TOKEN_SLASH:
    {
    }
    case TOKEN_STAR:
    {
    }
    case TOKEN_MOD:
    {
    }
    case TOKEN_SHIFT_RIGHT:
    {
    }
    case TOKEN_SHIFT_LEFT:
    {
    }
    case TOKEN_SEMICOLON:
    {
    }
    case TOKEN_COMMA:
    {
    }
    case TOKEN_DOT:
    {
    }
    case TOKEN_BANG:
    {
    }
    case TOKEN_BANG_EQUAL:
    {
    }
    case TOKEN_EQUAL:
    {
    }
    case TOKEN_EQUAL_EQUAL:
    {
    }
    case TOKEN_GREATER:
    {
    }
    case TOKEN_GREATER_EQUAL:
    {
    }
    case TOKEN_LESS:
    {
    }
    case TOKEN_LESS_EQUAL:
    {
    }
    case TOKEN_INCREMENT:
    {
    }
    case TOKEN_DECREMENT:
    {
    }
    case TOKEN_AUGMENTED_MINUS:
    {
    }
    case TOKEN_AUGMENTED_PLUS:
    {
    }
    case TOKEN_AUGMENTED_SLASH:
    {
    }
    case TOKEN_AUGMENTED_STAR:
    {
    }
    case TOKEN_AUGMENTED_AND:
    {
    }
    case TOKEN_AUGMENTED_OR:
    {
    }
    case TOKEN_AUGMENTED_XOR:
    {
    }
    case TOKEN_IDENTIFIER:
    {
    }
    case TOKEN_AND_LOGICAL:
    {
    }
    case TOKEN_OR_LOGICAL:
    {
    }
    case TOKEN_AND_BIT:
    {
    }
    case TOKEN_OR_BIT:
    {
    }
    case TOKEN_XOR:
    {
    }
    case TOKEN_EOF:
    {
    }
    }

  } while (!is_at_end(parser));
}
