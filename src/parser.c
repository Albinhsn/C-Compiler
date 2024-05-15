#include "parser.h"
#include "common.h"
#include "precedence.h"
#include "scanner.h"
#include "token.h"
#include <stdlib.h>

#define ALLOC(parser, type)  (sta_arena_push_struct(parser->scanner->arena, type))
#define ALLOC_NODE(parser)   (ALLOC(parser, AstNode))
#define CURRENT_TYPE(parser) parser->current->type

static void      parse_constant(Parser* parser, bool canAssign);
static void      parse_binary(Parser* parser, bool canAssign);
static void      parse_expression(Parser* parser, Precedence precedence);

static ParseRule rules[] = {
    [TOKEN_CHARACTER_CONSTANT] = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_FLOAT_CONSTANT]     = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_FLOAT_HEX_CONSTANT] = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_BINARY_CONSTANT]    = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_OCTAL_CONSTANT]     = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_INT_CONSTANT]       = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_INT_HEX_CONSTANT]   = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_STRING_CONSTANT]    = {parse_constant,            0, PREC_ASSIGNMENT},
    [TOKEN_AUTO]               = {             0,            0,       PREC_NONE},
    [TOKEN_BREAK]              = {             0,            0,       PREC_NONE},
    [TOKEN_CASE]               = {             0,            0,       PREC_NONE},
    [TOKEN_CHAR]               = {             0,            0,       PREC_NONE},
    [TOKEN_CONST]              = {             0,            0,       PREC_NONE},
    [TOKEN_CONTINUE]           = {             0,            0,       PREC_NONE},
    [TOKEN_DEFAULT]            = {             0,            0,       PREC_NONE},
    [TOKEN_DO]                 = {             0,            0,       PREC_NONE},
    [TOKEN_DOUBLE]             = {             0,            0,       PREC_NONE},
    [TOKEN_ELSE]               = {             0,            0,       PREC_NONE},
    [TOKEN_ENUM]               = {             0,            0,       PREC_NONE},
    [TOKEN_FLOAT]              = {             0,            0,       PREC_NONE},
    [TOKEN_FOR]                = {             0,            0,       PREC_NONE},
    [TOKEN_GOTO]               = {             0,            0,       PREC_NONE},
    [TOKEN_IF]                 = {             0,            0,       PREC_NONE},
    [TOKEN_INLINE]             = {             0,            0,       PREC_NONE},
    [TOKEN_INT]                = {             0,            0,       PREC_NONE},
    [TOKEN_LONG]               = {             0,            0,       PREC_NONE},
    [TOKEN_REGISTER]           = {             0,            0,       PREC_NONE},
    [TOKEN_RESTRICT]           = {             0,            0,       PREC_NONE},
    [TOKEN_RETURN]             = {             0,            0,       PREC_NONE},
    [TOKEN_SHORT]              = {             0,            0,       PREC_NONE},
    [TOKEN_SIGNED]             = {             0,            0,       PREC_NONE},
    [TOKEN_SIZEOF]             = {             0,            0,       PREC_NONE},
    [TOKEN_STATIC]             = {             0,            0,       PREC_NONE},
    [TOKEN_STRUCT]             = {             0,            0,       PREC_NONE},
    [TOKEN_SWITCH]             = {             0,            0,       PREC_NONE},
    [TOKEN_TYPEDEF]            = {             0,            0,       PREC_NONE},
    [TOKEN_UNION]              = {             0,            0,       PREC_NONE},
    [TOKEN_UNSIGNED]           = {             0,            0,       PREC_NONE},
    [TOKEN_VOID]               = {             0,            0,       PREC_NONE},
    [TOKEN_VOLATILE]           = {             0,            0,       PREC_NONE},
    [TOKEN_WHILE]              = {             0,            0,       PREC_NONE},
    [TOKEN_BOOL]               = {             0,            0,       PREC_NONE},
    [TOKEN_COMPLEX]            = {             0,            0,       PREC_NONE},
    [TOKEN_IMAGINARY]          = {             0,            0,       PREC_NONE},
    [TOKEN_LEFT_PAREN]         = {             0,            0,       PREC_NONE},
    [TOKEN_RIGHT_PAREN]        = {             0,            0,       PREC_NONE},
    [TOKEN_LEFT_BRACE]         = {             0,            0,       PREC_NONE},
    [TOKEN_RIGHT_BRACE]        = {             0,            0,       PREC_NONE},
    [TOKEN_LEFT_BRACKET]       = {             0,            0,       PREC_NONE},
    [TOKEN_RIGHT_BRACKET]      = {             0,            0,       PREC_NONE},
    [TOKEN_ELLIPSIS]           = {             0,            0,       PREC_NONE},
    [TOKEN_MINUS]              = {             0,            0,       PREC_NONE},
    [TOKEN_PLUS]               = {             0, parse_binary,       PREC_TERM},
    [TOKEN_SLASH]              = {             0, parse_binary,       PREC_TERM},
    [TOKEN_STAR]               = {             0, parse_binary,     PREC_FACTOR},
    [TOKEN_MOD]                = {             0, parse_binary,     PREC_FACTOR},
    [TOKEN_SHIFT_RIGHT]        = {             0,            0,       PREC_NONE},
    [TOKEN_SHIFT_LEFT]         = {             0,            0,       PREC_NONE},
    [TOKEN_SEMICOLON]          = {             0,            0,       PREC_NONE},
    [TOKEN_COMMA]              = {             0,            0,       PREC_NONE},
    [TOKEN_DOT]                = {             0,            0,       PREC_NONE},
    [TOKEN_BANG]               = {             0,            0,       PREC_NONE},
    [TOKEN_BANG_EQUAL]         = {             0,            0,       PREC_NONE},
    [TOKEN_EQUAL]              = {             0,            0,       PREC_NONE},
    [TOKEN_EQUAL_EQUAL]        = {             0,            0,       PREC_NONE},
    [TOKEN_GREATER]            = {             0,            0,       PREC_NONE},
    [TOKEN_GREATER_EQUAL]      = {             0,            0,       PREC_NONE},
    [TOKEN_LESS]               = {             0,            0,       PREC_NONE},
    [TOKEN_LESS_EQUAL]         = {             0,            0,       PREC_NONE},
    [TOKEN_INCREMENT]          = {             0,            0,       PREC_NONE},
    [TOKEN_DECREMENT]          = {             0,            0,       PREC_NONE},
    [TOKEN_IDENTIFIER]         = {             0,            0,       PREC_NONE},
    [TOKEN_AND_LOGICAL]        = {             0,            0,       PREC_NONE},
    [TOKEN_OR_LOGICAL]         = {             0,            0,       PREC_NONE},
    [TOKEN_AND_BIT]            = {             0,            0,       PREC_NONE},
    [TOKEN_OR_BIT]             = {             0,            0,       PREC_NONE},
    [TOKEN_XOR]                = {             0,            0,       PREC_NONE},
    [TOKEN_EOF]                = {             0,            0,       PREC_NONE}
};

void init_parser(Parser* parser, Scanner* scanner)
{
  parser->scanner  = scanner;
  parser->current  = 0;
  parser->previous = 0;
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

  advance(parser);
  while (match(parser, TOKEN_STAR))
  {
    out.depth++;
  }

  return out;
}

static void consume(Parser* parser, TokenType type, const char* msg)
{
  if (!match(parser, type))
  {
    error(msg);
  }
}
static bool is_struct(Parser* parser)
{
  return false;
}

static int get_type_qualifier(TokenType type)
{
  switch (type)
  {
  case TOKEN_CONST:
  {
    return 1;
  }
  case TOKEN_RESTRICT:
  {
    return 2;
  }
  case TOKEN_VOLATILE:
  {
    return 4;
  }
  default:
  {
    error("How?");
    exit(1);
  }
  }
}
static int get_storage_specifier(TokenType type)
{
  switch (type)
  {
  case TOKEN_EXTERN:
  {
    return STORAGE_EXTERN;
  }
  case TOKEN_STATIC:
  {
    return STORAGE_STATIC;
  }
  case TOKEN_AUTO:
  {
    return STORAGE_AUTO;
  }
  case TOKEN_REGISTER:
  {
    return STORAGE_REGISTER;
  }
  default:
  {
    error("How?");
    exit(1);
  }
  }
}

static void variable_declaration(Parser* parser)
{
  AstNode* node               = parser->node;
  parser->node->type          = NODE_DECLARATION;
  DeclarationNode* variable   = &parser->node->declaration;
  variable->type_qualifier    = 0;
  variable->storage_specifier = 0;
  bool got_type               = false;
  // storage specifier
  // type specifier
  bool got_identifier = false;
  while (!got_identifier)
  {
    switch (CURRENT_TYPE(parser))
    {
    case TOKEN_EXTERN:
    case TOKEN_STATIC:
    case TOKEN_AUTO:
    case TOKEN_REGISTER:
    {
      if (variable->storage_specifier != 0)
      {
        error("Already declared this storage qualifier");
      }
      variable->storage_specifier |= get_storage_specifier(CURRENT_TYPE(parser));
      advance(parser);
      break;
    }
      // const restrict volatile, type-qualifier
    case TOKEN_CONST:
    case TOKEN_RESTRICT:
    case TOKEN_VOLATILE:
    {
      if ((variable->type_qualifier & get_type_qualifier(CURRENT_TYPE(parser))) != 0)
      {
        error("Already declared this type qualifier");
      }
      variable->type_qualifier |= get_type_qualifier(CURRENT_TYPE(parser));
      advance(parser);
      break;
    }

    case TOKEN_IDENTIFIER:
    {
      if (is_struct(parser))
      {
        if (got_type)
        {
          error("Already got type");
        }
        got_type       = true;
        variable->type = parse_data_type(parser);
      }
      else
      {
        got_identifier = true;
      }
      break;
    }

    case TOKEN_CHAR:
    case TOKEN_SHORT:
    case TOKEN_INT:
    case TOKEN_LONG:
    case TOKEN_FLOAT:
    case TOKEN_DOUBLE:
    case TOKEN_SIGNED:
    case TOKEN_UNSIGNED:
    case TOKEN_BOOL:
    case TOKEN_COMPLEX:
    {
      if (got_type)
      {
        error("Already got type");
      }
      got_type       = true;
      variable->type = parse_data_type(parser);
      break;
    }
    default:
    {
      error("Huh?");
    }
    }
  }

  do
  {

    if (node->down == 0)
    {
      node->down     = ALLOC_NODE(parser);
      node->down->up = node;
      parser->node   = node->down;
    }
    else
    {
      AstNode* new_variable = ALLOC_NODE(parser);
      node->down->prev      = new_variable;
      new_variable->next    = node->down;

      new_variable->up      = node;
      node->down->up        = 0;

      node->down            = new_variable;
    }

    consume(parser, TOKEN_IDENTIFIER, "Expected variable name");
    AstNode* variable       = node->down;
    variable->type          = NODE_VARIABLE;
    variable->variable.name = parser->previous;
    parser->node            = variable;

    if (match(parser, TOKEN_EQUAL))
    {
      parser->node->down     = ALLOC_NODE(parser);
      parser->node->down->up = parser->node;
      parser->node           = parser->node->down;
      parse_expression(parser, PREC_ASSIGNMENT);
    }

  } while (match(parser, TOKEN_COMMA));

  consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration");
}

static void parse_binary(Parser* parser, bool canAssign)
{
  AstNode* binary_node = ALLOC_NODE(parser);
  if (parser->node->up != 0)
  {
    parser->node->up->down = binary_node;
    binary_node->up        = parser->node->up;
  }
  else
  {
    parser->node->prev->next = binary_node;
    binary_node->prev        = parser->node;
  }

  binary_node->type             = NODE_BINARY;
  binary_node->binary.op        = parser->previous->type;
  binary_node->down             = parser->node;
  binary_node->down->next       = ALLOC_NODE(parser);
  binary_node->down->next->prev = binary_node->down;
  parser->node                  = binary_node->down->next;

  parse_expression(parser, rules[parser->previous->type].precedence);
}

static void parse_constant(Parser* parser, bool canAssign)
{
  parser->node->type           = NODE_CONSTANT;
  parser->node->constant.token = parser->previous;
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
  parser->node->type     = NODE_RETURN;
  parser->node->down     = ALLOC_NODE(parser);
  parser->node->down->up = parser->node;
  parser->node           = parser->node->down;
  advance(parser);
  parse_expression(parser, PREC_ASSIGNMENT);
  consume(parser, TOKEN_SEMICOLON, "Expected ';' after return expression");
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
  case TOKEN_REGISTER:
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
  AstNode* curr      = ALLOC_NODE(parser);
  parser->node->down = curr;
  curr->up           = parser->node;
  parser->node       = curr;
  while (true)
  {
    parse_stmt(parser);
    if (match(parser, TOKEN_RIGHT_BRACE))
    {
      break;
    }
    curr->next = ALLOC_NODE(parser);
    curr       = curr->next;
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
      (void)ALLOC(parser, Argument);

      node->arguments[node->argument_count].type = parse_data_type(parser);
      consume(parser, TOKEN_IDENTIFIER, "Expected argument name?");
      node->arguments[node->argument_count].name = &parser->previous->literal;
      node->argument_count++;

    } while (match(parser, TOKEN_COMMA));
    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after function params");
  }
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
  advance(parser);
  while (!is_at_end(parser))
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
