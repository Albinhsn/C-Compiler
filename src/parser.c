#include "parser.h"
#include "common.h"
#include "precedence.h"
#include "scanner.h"
#include "token.h"
#include <stdlib.h>

#define ALLOC(parser, type)  (sta_arena_push_struct(parser->scanner->arena, type))
#define ALLOC_NODE(parser)   (ALLOC(parser, AstNode))
#define CURRENT_TYPE(parser) parser->current->type

static ParseRule rules[] = {
  // CONSTANTS
    [TOKEN_CHARACTER_CONSTANT] = {0, 0, PREC_NONE},
    [TOKEN_FLOAT_CONSTANT]     = {0, 0, PREC_NONE},
    [TOKEN_FLOAT_HEX_CONSTANT] = {0, 0, PREC_NONE},
    [TOKEN_BINARY_CONSTANT]    = {0, 0, PREC_NONE},
    [TOKEN_OCTAL_CONSTANT]     = {0, 0, PREC_NONE},
    [TOKEN_INT_CONSTANT]       = {0, 0, PREC_NONE},
    [TOKEN_INT_HEX_CONSTANT]   = {0, 0, PREC_NONE},
    [TOKEN_STRING_CONSTANT]    = {0, 0, PREC_NONE},
    [TOKEN_UNSIGNED_POSTFIX]   = {0, 0, PREC_NONE},
    [TOKEN_LONG_POSTFIX]       = {0, 0, PREC_NONE},
    [TOKEN_LONG_LONG_POSTFIX]  = {0, 0, PREC_NONE},
    [TOKEN_FLOAT_POSTFIX]      = {0, 0, PREC_NONE},

 // KEYWORDS
    [TOKEN_AUTO]          = {0, 0, PREC_NONE},
    [TOKEN_BREAK]         = {0, 0, PREC_NONE},
    [TOKEN_CASE]          = {0, 0, PREC_NONE},
    [TOKEN_CHAR]          = {0, 0, PREC_NONE},
    [TOKEN_CONST]         = {0, 0, PREC_NONE},
    [TOKEN_CONTINUE]      = {0, 0, PREC_NONE},
    [TOKEN_DEFAULT]       = {0, 0, PREC_NONE},
    [TOKEN_DO]            = {0, 0, PREC_NONE},
    [TOKEN_DOUBLE]        = {0, 0, PREC_NONE},
    [TOKEN_ELSE]          = {0, 0, PREC_NONE},
    [TOKEN_ENUM]          = {0, 0, PREC_NONE},
    [TOKEN_FLOAT]         = {0, 0, PREC_NONE},
    [TOKEN_FOR]           = {0, 0, PREC_NONE},
    [TOKEN_GOTO]          = {0, 0, PREC_NONE},
    [TOKEN_IF]            = {0, 0, PREC_NONE},
    [TOKEN_INLINE]        = {0, 0, PREC_NONE},
    [TOKEN_INT]           = {0, 0, PREC_NONE},
    [TOKEN_LONG]          = {0, 0, PREC_NONE},
    [TOKEN_REGISTER]      = {0, 0, PREC_NONE},
    [TOKEN_RESTRICT]      = {0, 0, PREC_NONE},
    [TOKEN_RETURN]        = {0, 0, PREC_NONE},
    [TOKEN_SHORT]         = {0, 0, PREC_NONE},
    [TOKEN_SIGNED]        = {0, 0, PREC_NONE},
    [TOKEN_SIZEOF]        = {0, 0, PREC_NONE},
    [TOKEN_STATIC]        = {0, 0, PREC_NONE},
    [TOKEN_STRUCT]        = {0, 0, PREC_NONE},
    [TOKEN_SWITCH]        = {0, 0, PREC_NONE},
    [TOKEN_TYPEDEF]       = {0, 0, PREC_NONE},
    [TOKEN_UNION]         = {0, 0, PREC_NONE},
    [TOKEN_UNSIGNED]      = {0, 0, PREC_NONE},
    [TOKEN_VOID]          = {0, 0, PREC_NONE},
    [TOKEN_VOLATILE]      = {0, 0, PREC_NONE},
    [TOKEN_WHILE]         = {0, 0, PREC_NONE},
    [TOKEN_BOOL]          = {0, 0, PREC_NONE},
    [TOKEN_COMPLEX]       = {0, 0, PREC_NONE},
    [TOKEN_IMAGINARY]     = {0, 0, PREC_NONE},

    [TOKEN_LEFT_PAREN]    = {0, 0, PREC_NONE},
    [TOKEN_RIGHT_PAREN]   = {0, 0, PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {0, 0, PREC_NONE},
    [TOKEN_RIGHT_BRACE]   = {0, 0, PREC_NONE},
    [TOKEN_LEFT_BRACKET]  = {0, 0, PREC_NONE},
    [TOKEN_RIGHT_BRACKET] = {0, 0, PREC_NONE},
    [TOKEN_ELLIPSIS]      = {0, 0, PREC_NONE},
    [TOKEN_MINUS]         = {0, 0, PREC_NONE},
    [TOKEN_PLUS]          = {0, 0, PREC_NONE},
    [TOKEN_SLASH]         = {0, 0, PREC_NONE},
    [TOKEN_STAR]          = {0, 0, PREC_NONE},
    [TOKEN_MOD]           = {0, 0, PREC_NONE},
    [TOKEN_SHIFT_RIGHT]   = {0, 0, PREC_NONE},
    [TOKEN_SHIFT_LEFT]    = {0, 0, PREC_NONE},
    [TOKEN_SEMICOLON]     = {0, 0, PREC_NONE},
    [TOKEN_COMMA]         = {0, 0, PREC_NONE},
    [TOKEN_DOT]           = {0, 0, PREC_NONE},
    [TOKEN_BANG]          = {0, 0, PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {0, 0, PREC_NONE},
    [TOKEN_EQUAL]         = {0, 0, PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {0, 0, PREC_NONE},
    [TOKEN_GREATER]       = {0, 0, PREC_NONE},
    [TOKEN_GREATER_EQUAL] = {0, 0, PREC_NONE},
    [TOKEN_LESS]          = {0, 0, PREC_NONE},
    [TOKEN_LESS_EQUAL]    = {0, 0, PREC_NONE},
    [TOKEN_INCREMENT]     = {0, 0, PREC_NONE},
    [TOKEN_DECREMENT]     = {0, 0, PREC_NONE},
    [TOKEN_IDENTIFIER]    = {0, 0, PREC_NONE},
    [TOKEN_AND_LOGICAL]   = {0, 0, PREC_NONE},
    [TOKEN_OR_LOGICAL]    = {0, 0, PREC_NONE},
    [TOKEN_AND_BIT]       = {0, 0, PREC_NONE},
    [TOKEN_OR_BIT]        = {0, 0, PREC_NONE},
    [TOKEN_XOR]           = {0, 0, PREC_NONE},
    [TOKEN_EOF]           = {0, 0, PREC_NONE}
};

void init_parser(Parser* parser, Scanner* scanner)
{
  parser->scanner  = scanner;
  parser->current  = 0;
  parser->previous = 0;
  parser->current  = 0;
}
static void advance(Parser* parser)
{
  parser->previous = parser->current;
  parser->current  = parse_token(parser->scanner);
  printf("New current %.*s\n", (i32)parser->current->literal.len, parser->current->literal.buffer);
}

static bool is_at_end(Parser* parser)
{
  return CURRENT_TYPE(parser) == TOKEN_EOF;
}

static bool match(Parser* parser, TokenType type)
{
  if (CURRENT_TYPE(parser) == type)
  {
    advance(parser);
    return true;
  }
  return false;
}

static int get_size_from_type(TokenType type)
{
  switch (type)
  {
  case TOKEN_INT:
  {
    return sizeof(int);
  }
  case TOKEN_FLOAT:
  {
    return sizeof(float);
  }
  case TOKEN_LONG:
  {
    return sizeof(long);
  }
  case TOKEN_SHORT:
  {
    return sizeof(short);
  }
  case TOKEN_CHAR:
  {
    return sizeof(char);
  }
  case TOKEN_DOUBLE:
  {
    return sizeof(double);
  }
  default:
  {
    error("Can't get size from this type");
    exit(1);
  }
  }
}

static void parse_integer_type(Parser* parser, DataType* type, bool signedness)
{
  type->type               = DATA_TYPE_INTEGER;
  type->integer.signedness = signedness;
  type->integer.size       = get_size_from_type(CURRENT_TYPE(parser));
}

static void parse_floating_point_type(Parser* parser, DataType* type)
{
  type->type = DATA_TYPE_FLOATING_POINT;
}

static void parse_struct_type(Parser* parser, DataType* type)
{
  type->type = DATA_TYPE_STRUCT;
}

static DataType parse_data_type(Parser* parser)
{
  // parse signedness
  bool     signedness = match(parser, TOKEN_UNSIGNED) ? 0 : 1;

  DataType out        = {};
  // parse type
  switch (CURRENT_TYPE(parser))
  {
  case TOKEN_LONG:
  case TOKEN_INT:
  case TOKEN_SHORT:
  {
    parse_integer_type(parser, &out, signedness);
    break;
  }
  case TOKEN_FLOAT:
  case TOKEN_DOUBLE:
  {
    parse_floating_point_type(parser, &out);
    break;
  }
  case TOKEN_IDENTIFIER:
  {
    break;
  }
  default:
  {
    error("Expected struct or variable type?");
  }
  }

  while (match(parser, TOKEN_STAR))
  {
    out.depth++;
  }

  // parse X amount of pointers

  return out;
}

static void consume(Parser* parser, TokenType type, const char* msg)
{
  if (!match(parser, type))
  {
    error(msg);
  }
}
static void variable_declaration(Parser* parser)
{
}

static void parse_expression(Parser* parser, Precedence precedence)
{
  advance(parser);
  ParseRule prefix = rules[parser->previous->type];
  if (prefix.prefix == 0)
  {
    error("Expected expression!");
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;

  prefix.prefix(parser, canAssign);
  ParseRule current = rules[parser->current->type];

  while (precedence <= current.precedence)
  {
    advance(parser);
    current.infix(parser, canAssign);
    current = rules[parser->current->type];
  }

  if (canAssign && match(parser, TOKEN_EQUAL))
  {
    error("Can't assign to this!");
  }
}

static void parse_return(Parser* parser)
{
  parser->node->type = NODE_RETURN;
  parser->node->down = ALLOC_NODE(parser);
  advance(parser);
  parse_expression(parser, PREC_ASSIGNMENT);
}

static void parse_stmt(Parser* parser)
{
  switch (CURRENT_TYPE(parser))
  {
  case TOKEN_INLINE:
  {
  }
  case TOKEN_ENUM:
  {
  }
  case TOKEN_STRUCT:
  {
  }
  case TOKEN_TYPEDEF:
  {
  }
  case TOKEN_UNION:
  {
    break;
  }
  case TOKEN_RETURN:
  {
    parse_return(parser);
    return;
  }

  // parse some type
  case TOKEN_CONST:
  case TOKEN_SIGNED:
  case TOKEN_UNSIGNED:
  case TOKEN_STATIC:
  case TOKEN_VOLATILE:
  case TOKEN_DOUBLE:
  case TOKEN_CHAR:
  case TOKEN_FLOAT:
  case TOKEN_INT:
  case TOKEN_LONG:
  case TOKEN_SHORT:
  {
    variable_declaration(parser);
    break;
  }

  case TOKEN_LEFT_PAREN:
  {
    // function pointer?
  }

  case TOKEN_SEMICOLON:
  {
    // just empty?
  }
  case TOKEN_IDENTIFIER:
  {
    // should be type?
  }

  case TOKEN_EOF:
  {
  }
  default:
  {
    error("idk can't parse this");
  }
  }
}

static void parse_block(Parser* parser)
{
  while (!match(parser, TOKEN_RIGHT_BRACE))
  {
    AstNode* last = parser->node;
    parse_stmt(parser);
    last->next = ALLOC_NODE(parser);
    last       = last->next;
  }
}

static void parse_function(Parser* parser)
{
  FunctionNode* node = &parser->node->function;
  parser->node->type = NODE_FUNCTION;

  node->return_type  = parse_data_type(parser);
  consume(parser, TOKEN_IDENTIFIER, "Expected function name?");
  node->name = &parser->previous->literal;
  consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name");
  node->argument_count = 0;
  node->arguments      = 0;
  if (!match(parser, TOKEN_RIGHT_PAREN))
  {
    node->arguments = (Argument*)(parser->scanner->arena->ptr + parser->scanner->arena->memory);
    do
    {
      ALLOC(parser, Argument);

      node->arguments[node->argument_count].type = parse_data_type(parser);
      consume(parser, TOKEN_IDENTIFIER, "Expected argument name?");
      node->arguments[node->argument_count].name = &parser->previous->literal;
      node->argument_count++;

    } while (match(parser, TOKEN_COMMA));
  }
  consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after function params");
  if (match(parser, TOKEN_SEMICOLON))
  {
    // only declared it
  }
  else
  {
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' or ';' after function params");
    parse_block(parser);
  }
}

AstNode* parse(Parser* parser)
{
  AstNode* out  = ALLOC_NODE(parser);
  AstNode* last = out;
  parser->node  = out;
  while (!is_at_end(parser))
  {

    advance(parser);
    switch (CURRENT_TYPE(parser))
    {
    case TOKEN_INLINE:
    {
    }
    case TOKEN_ENUM:
    {
    }
    case TOKEN_STRUCT:
    {
    }
    case TOKEN_TYPEDEF:
    {
    }
    case TOKEN_UNION:
    {
    }

      // parse some type
    case TOKEN_CONST:
    case TOKEN_SIGNED:
    case TOKEN_UNSIGNED:
    case TOKEN_STATIC:
    case TOKEN_VOLATILE:
    case TOKEN_DOUBLE:
    case TOKEN_CHAR:
    case TOKEN_FLOAT:
    case TOKEN_INT:
    case TOKEN_LONG:
    case TOKEN_SHORT:
    case TOKEN_VOID:
    {
      parse_function(parser);
      break;
    }

    case TOKEN_LEFT_PAREN:
    {
      // function pointer?
    }

    case TOKEN_SEMICOLON:
    {
      // just continue
    }
    case TOKEN_IDENTIFIER:
    {
      // should be type?
    }

    case TOKEN_EOF:
    {
    }
    default:
    {
      error("idk can't parse this");
    }
    }

    if (!is_at_end(parser))
    {
      last->next = ALLOC_NODE(parser);
      last       = last->next;
    }
  }
  return out;
}
