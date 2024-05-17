#include "parser.h"
#include "common.h"
#include "precedence.h"
#include "scanner.h"
#include "token.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define ALLOC(parser, type)    (sta_arena_push_struct(parser->scanner->arena, type))
#define ALLOC_NODE(parser)     (ALLOC(parser, AstNode))
#define CURRENT_TYPE(parser)   parser->current->type
#define CURRENT_MEMORY(parser) (parser->scanner->arena->memory + parser->scanner->arena->ptr)

static void      parse_constant(Parser* parser, bool can_assign);
static void      parse_binary(Parser* parser, bool can_assign);
static void      parse_comparison(Parser* parser, bool can_assign);
static void      parse_unary(Parser* parser, bool can_assign);
static void      parse_postfix(Parser* parser, bool can_assign);
static void      parse_variable(Parser* parser, bool can_assign);
static void      parse_expression(Parser* parser, Precedence precedence);
static void      parse_stmt(Parser* parser);
static void      parse_block(Parser* parser);
static void      parse_declaration(Parser* parser);

static ParseRule rules[] = {
    [TOKEN_CHARACTER_CONSTANT] = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_FLOAT_CONSTANT]     = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_FLOAT_HEX_CONSTANT] = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_BINARY_CONSTANT]    = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_OCTAL_CONSTANT]     = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_INT_CONSTANT]       = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_INT_HEX_CONSTANT]   = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_STRING_CONSTANT]    = {parse_constant,                0, PREC_ASSIGNMENT},
    [TOKEN_AUTO]               = {             0,                0,       PREC_NONE},
    [TOKEN_BREAK]              = {             0,                0,       PREC_NONE},
    [TOKEN_CASE]               = {             0,                0,       PREC_NONE},
    [TOKEN_CHAR]               = {             0,                0,       PREC_NONE},
    [TOKEN_CONST]              = {             0,                0,       PREC_NONE},
    [TOKEN_CONTINUE]           = {             0,                0,       PREC_NONE},
    [TOKEN_DEFAULT]            = {             0,                0,       PREC_NONE},
    [TOKEN_DO]                 = {             0,                0,       PREC_NONE},
    [TOKEN_DOUBLE]             = {             0,                0,       PREC_NONE},
    [TOKEN_ELSE]               = {             0,                0,       PREC_NONE},
    [TOKEN_ENUM]               = {             0,                0,       PREC_NONE},
    [TOKEN_FLOAT]              = {             0,                0,       PREC_NONE},
    [TOKEN_FOR]                = {             0,                0,       PREC_NONE},
    [TOKEN_GOTO]               = {             0,                0,       PREC_NONE},
    [TOKEN_IF]                 = {             0,                0,       PREC_NONE},
    [TOKEN_INLINE]             = {             0,                0,       PREC_NONE},
    [TOKEN_INT]                = {             0,                0,       PREC_NONE},
    [TOKEN_LONG]               = {             0,                0,       PREC_NONE},
    [TOKEN_REGISTER]           = {             0,                0,       PREC_NONE},
    [TOKEN_RESTRICT]           = {             0,                0,       PREC_NONE},
    [TOKEN_RETURN]             = {             0,                0,       PREC_NONE},
    [TOKEN_SHORT]              = {             0,                0,       PREC_NONE},
    [TOKEN_SIGNED]             = {             0,                0,       PREC_NONE},
    [TOKEN_SIZEOF]             = {             0,                0,       PREC_NONE},
    [TOKEN_STATIC]             = {             0,                0,       PREC_NONE},
    [TOKEN_STRUCT]             = {             0,                0,       PREC_NONE},
    [TOKEN_SWITCH]             = {             0,                0,       PREC_NONE},
    [TOKEN_TYPEDEF]            = {             0,                0,       PREC_NONE},
    [TOKEN_UNION]              = {             0,                0,       PREC_NONE},
    [TOKEN_UNSIGNED]           = {             0,                0,       PREC_NONE},
    [TOKEN_VOID]               = {             0,                0,       PREC_NONE},
    [TOKEN_VOLATILE]           = {             0,                0,       PREC_NONE},
    [TOKEN_WHILE]              = {             0,                0,       PREC_NONE},
    [TOKEN_BOOL]               = {             0,                0,       PREC_NONE},
    [TOKEN_COMPLEX]            = {             0,                0,       PREC_NONE},
    [TOKEN_IMAGINARY]          = {             0,                0,       PREC_NONE},
    [TOKEN_LEFT_PAREN]         = {             0,                0,       PREC_NONE},
    [TOKEN_RIGHT_PAREN]        = {             0,                0,       PREC_NONE},
    [TOKEN_LEFT_BRACE]         = {             0,                0,       PREC_NONE},
    [TOKEN_RIGHT_BRACE]        = {             0,                0,       PREC_NONE},
    [TOKEN_LEFT_BRACKET]       = {             0,                0,       PREC_NONE},
    [TOKEN_RIGHT_BRACKET]      = {             0,                0,       PREC_NONE},
    [TOKEN_ELLIPSIS]           = {             0,                0,       PREC_NONE},
    [TOKEN_MINUS]              = {             0,                0,       PREC_NONE},
    [TOKEN_PLUS]               = {             0,     parse_binary,       PREC_TERM},
    [TOKEN_SLASH]              = {             0,     parse_binary,       PREC_TERM},
    [TOKEN_STAR]               = {             0,     parse_binary,     PREC_FACTOR},
    [TOKEN_MOD]                = {             0,     parse_binary,     PREC_FACTOR},
    [TOKEN_SHIFT_RIGHT]        = {             0,                0,       PREC_NONE},
    [TOKEN_SHIFT_LEFT]         = {             0,                0,       PREC_NONE},
    [TOKEN_SEMICOLON]          = {             0,                0,       PREC_NONE},
    [TOKEN_COMMA]              = {             0,                0,       PREC_NONE},
    [TOKEN_DOT]                = {             0,                0,       PREC_NONE},
    [TOKEN_BANG]               = {             0,                0,       PREC_NONE},
    [TOKEN_BANG_EQUAL]         = {             0, parse_comparison, PREC_COMPARISON},
    [TOKEN_EQUAL]              = {             0, parse_comparison, PREC_COMPARISON},
    [TOKEN_EQUAL_EQUAL]        = {             0, parse_comparison, PREC_COMPARISON},
    [TOKEN_GREATER]            = {             0, parse_comparison, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]      = {             0, parse_comparison, PREC_COMPARISON},
    [TOKEN_LESS]               = {             0, parse_comparison, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]         = {             0, parse_comparison, PREC_COMPARISON},
    [TOKEN_INCREMENT]          = {   parse_unary,    parse_postfix,       PREC_TERM},
    [TOKEN_DECREMENT]          = {   parse_unary,    parse_postfix,       PREC_TERM},
    [TOKEN_IDENTIFIER]         = {parse_variable,                0,       PREC_NONE},
    [TOKEN_AND_LOGICAL]        = {             0,                0,       PREC_NONE},
    [TOKEN_OR_LOGICAL]         = {             0,                0,       PREC_NONE},
    [TOKEN_AND_BIT]            = {             0,                0,       PREC_NONE},
    [TOKEN_OR_BIT]             = {             0,                0,       PREC_NONE},
    [TOKEN_XOR]                = {             0,                0,       PREC_NONE},
    [TOKEN_EOF]                = {             0,                0,       PREC_NONE}
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

static void variable_declaration(Parser* parser, DataType type, String* name, char type_qualifier, char storage_specifier)
{
  AstNode* node               = parser->node;
  parser->node->type          = NODE_DECLARATION;
  DeclarationNode* variable   = &parser->node->declaration;
  variable->type_qualifier    = type_qualifier;
  variable->storage_specifier = storage_specifier;
  variable->type              = type;

  node->declaration.variables = ALLOC_NODE(parser);
  AstNode* curr               = node->declaration.variables;
  do
  {

    parser->node = curr;
    curr->type   = NODE_VARIABLE;
    if (name == 0)
    {
      consume(parser, TOKEN_IDENTIFIER, "Expected variable name");
      curr->variable.name = &parser->previous->literal;
    }
    else
    {
      curr->variable.name = name;
      name                = 0;
    }

    if (match(parser, TOKEN_EQUAL))
    {
      curr->variable.value = ALLOC_NODE(parser);
      parser->node         = curr->variable.value;
      parse_expression(parser, PREC_ASSIGNMENT);
    }

    if (match(parser, TOKEN_COMMA))
    {
      curr->next = ALLOC_NODE(parser);
      curr       = curr->next;
    }
    else
    {
      break;
    }

  } while (true);

  consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration");
}

static void parse_variable(Parser* parser, bool can_assign)
{
  AstNode* node        = parser->node;
  node->type           = NODE_CONSTANT;
  node->constant.token = parser->previous;
}

static void parse_comparison(Parser* parser, bool can_assign)
{
  AstNode* left_node = ALLOC_NODE(parser);
  memcpy(left_node, parser->node, sizeof(AstNode));
  memset(parser->node, 0, sizeof(AstNode));
  parser->node->type             = NODE_COMPARISON;
  parser->node->comparison.left  = left_node;
  parser->node->comparison.op    = parser->previous->type;
  parser->node->comparison.right = ALLOC_NODE(parser);
  parser->node                   = parser->node->comparison.right;

  parse_expression(parser, rules[parser->previous->type].precedence);
}

static void parse_unary(Parser* parser, bool can_assign)
{
  assert(0 && "Not implemented");
}
static void parse_postfix(Parser* parser, bool can_assign)
{
  AstNode* postfix_node = ALLOC_NODE(parser);
  memcpy(postfix_node, parser->node, sizeof(AstNode));
  memset(parser->node, 0, sizeof(AstNode));
  parser->node->type            = NODE_POSTFIX;
  parser->node->postfix.postfix = parser->previous;
  parser->node->postfix.node    = postfix_node;
}

static void parse_binary(Parser* parser, bool can_assign)
{
  AstNode* left_node = ALLOC_NODE(parser);
  memcpy(left_node, parser->node, sizeof(AstNode));
  memset(parser->node, 0, sizeof(AstNode));
  parser->node->type         = NODE_BINARY;
  parser->node->binary.left  = left_node;
  parser->node->binary.op    = parser->previous->type;
  parser->node->binary.right = ALLOC_NODE(parser);
  parser->node               = parser->node->binary.right;

  parse_expression(parser, rules[parser->previous->type].precedence);
}

static void parse_constant(Parser* parser, bool can_assign)
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

  bool can_assign = precedence <= PREC_ASSIGNMENT;

  prefix.prefix(parser, can_assign);
  ParseRule current = rules[parser->current->type];

  while (precedence <= current.precedence)
  {
    advance(parser);
    current.infix(parser, can_assign);
    current = rules[parser->current->type];
  }

  if (can_assign && match(parser, TOKEN_EQUAL))
  {
    error("Can't assign to this!");
  }
}

static void parse_return(Parser* parser)
{
  parser->node->type          = NODE_RETURN;
  parser->node->return_.value = ALLOC_NODE(parser);
  parser->node                = parser->node->return_.value;
  advance(parser);
  parse_expression(parser, PREC_ASSIGNMENT);
  consume(parser, TOKEN_SEMICOLON, "Expected ';' after return expression");
}

static bool is_declaration(Parser* parser)
{
  TokenType current_type = CURRENT_TYPE(parser);

  return is_struct(parser) || current_type == TOKEN_CHAR || current_type == TOKEN_SHORT || current_type == TOKEN_INT || current_type == TOKEN_LONG || current_type == TOKEN_FLOAT ||
         current_type == TOKEN_DOUBLE || current_type == TOKEN_SIGNED || current_type == TOKEN_UNSIGNED || current_type == TOKEN_BOOL || current_type == TOKEN_COMPLEX;
}

static void parse_do(Parser* parser)
{
  AstNode* do_node = parser->node;
  do_node->type    = NODE_DO;

  advance(parser);
  do_node->do_.body = ALLOC_NODE(parser);
  parser->node      = do_node->do_.body;
  parse_stmt(parser);

  consume(parser, TOKEN_WHILE, "Expected while after do");
  consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after while in do while");
  do_node->do_.condition = ALLOC_NODE(parser);
  parser->node           = do_node->do_.condition;

  parse_expression(parser, PREC_ASSIGNMENT);
  consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after while condition");
  consume(parser, TOKEN_SEMICOLON, "Expected ';' after do while");
}
static void parse_while(Parser* parser)
{

  AstNode* while_node = parser->node;
  while_node->type    = NODE_WHILE;
  advance(parser);
  consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after for");

  while_node->while_.condition = ALLOC_NODE(parser);
  parser->node                 = while_node->while_.condition;
  parse_expression(parser, PREC_ASSIGNMENT);
  consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after while condition");

  while_node->while_.body = ALLOC_NODE(parser);
  parser->node            = while_node->while_.body;
  parse_stmt(parser);
}
static void parse_for(Parser* parser)
{
  AstNode* for_node = parser->node;
  for_node->type    = NODE_FOR;

  advance(parser);
  consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after for");

  for_node->for_.init = 0;
  if (is_declaration(parser))
  {
    for_node->for_.init = ALLOC_NODE(parser);
    parser->node        = for_node->for_.init;
    parse_declaration(parser);
  }
  else
  {
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after first clause");
  }

  for_node->for_.condition = 0;
  if (!match(parser, TOKEN_SEMICOLON))
  {
    for_node->for_.condition = ALLOC_NODE(parser);
    parser->node             = for_node->for_.condition;
    parse_expression(parser, PREC_ASSIGNMENT);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after second expression");
  }

  for_node->for_.update = 0;
  if (!match(parser, TOKEN_RIGHT_PAREN))
  {
    for_node->for_.update = ALLOC_NODE(parser);
    parser->node          = for_node->for_.update;
    parse_expression(parser, PREC_ASSIGNMENT);
    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after third expression");
  }

  for_node->for_.body = ALLOC_NODE(parser);
  parser->node        = for_node->for_.body;
  parse_stmt(parser);
}
static void parse_typedef(Parser* parser)
{
  // struct union enum
}

static bool is_int_constant(Parser* parser)
{
  TokenType current_type = CURRENT_TYPE(parser);
  return (current_type == TOKEN_INT_CONSTANT || current_type == TOKEN_INT_HEX_CONSTANT || current_type == TOKEN_OCTAL_CONSTANT);
}

static void parse_enum(Parser* parser)
{
  AstNode* node       = parser->node;
  node->type          = NODE_ENUM;
  EnumNode* enum_node = &node->enum_;
  advance(parser);

  consume(parser, TOKEN_IDENTIFIER, "Expected enum name");
  enum_node->name = &parser->previous->literal;

  consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after enum");
  if (match(parser, TOKEN_RIGHT_BRACE))
  {
    error("No empty enum");
  }

  enum_node->values = ALLOC(parser, EnumValue);
  EnumValue* value  = enum_node->values;
  while (true)
  {
    consume(parser, TOKEN_IDENTIFIER, "Expected identifier for enum");
    value->name     = &parser->previous->literal;
    value->constant = 0;
    if (match(parser, TOKEN_EQUAL))
    {
      if (!(is_int_constant(parser) || CURRENT_TYPE(parser) == TOKEN_IDENTIFIER))
      {
        error("Enum value needs to be int constant or enum value");
      }
      advance(parser);
      value->constant = parser->previous;
    }
    if (match(parser, TOKEN_COMMA))
    {
      value->next = ALLOC(parser, EnumValue);
      value       = value->next;
    }
    else
    {
      break;
    }
  }

  consume(parser, TOKEN_RIGHT_BRACE, "Need to close enum?");
  consume(parser, TOKEN_SEMICOLON, "Need to close enum?");
}

static void parse_fields(Parser* parser, StructField** field_ptr)
{
  StructField* field = 0;
  if (!match(parser, TOKEN_RIGHT_BRACE))
  {

    do
    {
      if (field == 0)
      {
        *field_ptr = ALLOC(parser, StructField);
        field      = *field_ptr;
      }
      else
      {
        field->next = ALLOC(parser, StructField);
        field       = field->next;
      }

      if (!(is_declaration(parser)))
      {
        error("Not struct or type?");
      }
      field->type = parse_data_type(parser);
      consume(parser, TOKEN_IDENTIFIER, "Expected field name");
      field->name = &parser->previous->literal;
      consume(parser, TOKEN_SEMICOLON, "Expected ';' after field");

    } while (!match(parser, TOKEN_RIGHT_BRACE));
  }
}

static void parse_struct(Parser* parser)
{
  AstNode* node = parser->node;
  node->type    = NODE_STRUCT;
  advance(parser);
  // ToDo should be if
  consume(parser, TOKEN_IDENTIFIER, "Expected struct name");
  node->struct_.name = &parser->previous->literal;
  consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after struct name");

  parse_fields(parser, &node->struct_.fields);
  consume(parser, TOKEN_SEMICOLON, "Expected '}' after struct declaration");
}
static void parse_union(Parser* parser)
{
  AstNode* node = parser->node;
  node->type    = NODE_UNION;
  advance(parser);
  // ToDo should be if
  consume(parser, TOKEN_IDENTIFIER, "Expected struct name");
  node->union_.name = &parser->previous->literal;
  consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after struct name");
  parse_fields(parser, &node->union_.fields);
  consume(parser, TOKEN_SEMICOLON, "Expected '}' after struct declaration");
}

static void parse_if_block(Parser* parser, IfBlock* block)
{
  consume(parser, TOKEN_LEFT_PAREN, "Expected '(' for if condition");
  block->condition = ALLOC_NODE(parser);
  parser->node     = block->condition;

  parse_expression(parser, PREC_ASSIGNMENT);
  consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after if condition");
  block->body  = ALLOC_NODE(parser);
  parser->node = block->body;
  parse_stmt(parser);
}

static void parse_if(Parser* parser)
{

  AstNode* node    = parser->node;
  node->type       = NODE_IF;
  node->if_.blocks = ALLOC(parser, IfBlock);
  IfBlock* blocks  = node->if_.blocks;
  advance(parser);
  parse_if_block(parser, blocks);

  while (match(parser, TOKEN_ELSE))
  {
    if (match(parser, TOKEN_IF))
    {
      blocks->next = ALLOC(parser, IfBlock);
      blocks       = blocks->next;
      parse_if_block(parser, blocks);
    }
    else
    {
      node->if_.else_ = ALLOC_NODE(parser);
      parser->node    = node->if_.else_;
      parse_stmt(parser);
      break;
    }
  }
}

static void parse_switch(Parser* parser)
{
  advance(parser);
  consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after switch");
  AstNode* node           = parser->node;
  node->type              = NODE_SWITCH;
  node->switch_.condition = ALLOC_NODE(parser);

  parser->node            = node->switch_.condition;
  parse_expression(parser, PREC_ASSIGNMENT);
  consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after switch condition");
  node->switch_.block = ALLOC_NODE(parser);
  parser->node        = node->switch_.block;
  parse_stmt(parser);
}

static void parse_case(Parser* parser)
{
  advance(parser);
}

static void parse_default(Parser* parser)
{
  advance(parser);
}

static void parse_stmt(Parser* parser)
{
  switch (CURRENT_TYPE(parser))
  {
  case TOKEN_IF:
  {
    parse_if(parser);
    break;
  }
  case TOKEN_CASE:
  {
    parse_case(parser);
    break;
  }
  case TOKEN_DEFAULT:
  {
    parse_default(parser);
    break;
  }
  case TOKEN_SWITCH:
  {
    parse_switch(parser);
    break;
  }
  case TOKEN_ENUM:
  {
    parse_enum(parser);
    break;
  }
  case TOKEN_STRUCT:
  {
    parse_struct(parser);
    break;
  }
  case TOKEN_TYPEDEF:
  {
    parse_typedef(parser);
  }
  case TOKEN_UNION:
  {
    parse_union(parser);
    break;
  }
  case TOKEN_LEFT_BRACE:
  {
    advance(parser);
    parse_block(parser);
    return;
  }
  case TOKEN_WHILE:
  {
    parse_while(parser);
    return;
  }
  case TOKEN_BREAK:
  {
    assert(0 && "Not implemented");
  }
  case TOKEN_CONTINUE:
  {
    assert(0 && "Not implemented");
  }
  case TOKEN_DO:
  {
    parse_do(parser);
    return;
  }
  case TOKEN_FOR:
  {
    parse_for(parser);
    return;
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
    parse_declaration(parser);
    break;
  }

  case TOKEN_SEMICOLON:
  {
    parser->node->type = NODE_EMPTY;
    break;
    // just empty?
  }
  // ToDo fix function pointer?
  case TOKEN_LEFT_PAREN:
  case TOKEN_IDENTIFIER:
  {
    if (is_struct(parser))
    {
      parse_declaration(parser);
    }
    else
    {
      // evaluate the expression
      // while right side assign
      AstNode* prev = parser->node;
      parse_expression(parser, PREC_ASSIGNMENT + 1);
      while (match(parser, TOKEN_EQUAL))
      {
        AstNode* node = ALLOC_NODE(parser);
        AstNode  tmp  = *prev;
        memset(prev, 0, sizeof(AstNode));
        parser->node         = node;
        prev->type           = NODE_ASSIGN;

        prev->assign.target  = ALLOC_NODE(parser);
        *prev->assign.target = tmp;
        prev->assign.value   = node;
        parse_expression(parser, PREC_ASSIGNMENT + 1);
        prev = node;
      }

      consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression stmt?");
    }
    break;
  }

  case TOKEN_EOF:
  default:
  {
    error("idk can't parse this");
  }
  }
}

static void parse_block(Parser* parser)
{
  parser->node->type = NODE_BLOCK;
  AstNode* curr      = 0;
  while (!match(parser, TOKEN_RIGHT_BRACE))
  {
    if (curr == 0)
    {
      parser->node->block.nodes = ALLOC_NODE(parser);
      curr                      = parser->node->block.nodes;
      curr->next                = 0;
    }
    else
    {
      curr->next = ALLOC_NODE(parser);
      curr       = curr->next;
    }
    parser->node = curr;
    parse_stmt(parser);
  }
}

static void parse_function(Parser* parser, DataType type, String* literal)
{
  FunctionNode* node   = &parser->node->function;
  parser->node->type   = NODE_FUNCTION;

  node->return_type    = type;
  node->name           = literal;
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
    node->block  = ALLOC_NODE(parser);
    parser->node = node->block;
    parse_block(parser);
  }
}

static void parse_declaration(Parser* parser)
{

  DataType type;
  String*  name              = 0;
  char     storage_specifier = 0;
  char     type_qualifier    = 0;
  bool     got_type          = false;
  bool     got_identifier    = false;
  while (!got_identifier)
  {
    switch (CURRENT_TYPE(parser))
    {
    case TOKEN_EXTERN:
    case TOKEN_STATIC:
    case TOKEN_AUTO:
    case TOKEN_REGISTER:
    {
      if (storage_specifier != 0)
      {
        error("Already declared this storage qualifier");
      }
      storage_specifier |= get_storage_specifier(CURRENT_TYPE(parser));
      advance(parser);
      break;
    }
      // const restrict volatile, type-qualifier
    case TOKEN_CONST:
    case TOKEN_RESTRICT:
    case TOKEN_VOLATILE:
    {
      if ((type_qualifier & get_type_qualifier(CURRENT_TYPE(parser))) != 0)
      {
        error("Already declared this type qualifier");
      }
      type_qualifier |= get_type_qualifier(CURRENT_TYPE(parser));
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
        got_type = true;
        type     = parse_data_type(parser);
      }
      else
      {
        got_identifier = true;
        name           = &parser->current->literal;
        advance(parser);
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
      got_type = true;
      type     = parse_data_type(parser);
      break;
    }
    default:
    {
      error("Huh?");
    }
    }
  }

  if (match(parser, TOKEN_LEFT_PAREN))
  {
    if (type_qualifier != 0 || storage_specifier != 0)
    {
      error("Can't declare a function with type/storage specifiers");
    }
    parse_function(parser, type, name);
  }
  else
  {
    variable_declaration(parser, type, name, type_qualifier, storage_specifier);
  }
}

AstNode* parse(Parser* parser)
{
  AstNode* out  = ALLOC_NODE(parser);
  AstNode* last = out;
  advance(parser);
  while (!is_at_end(parser))
  {
    parser->node = last;

    switch (CURRENT_TYPE(parser))
    {
    case TOKEN_INLINE:
    {
    }
    case TOKEN_ENUM:
    {
      parse_enum(parser);
      break;
    }
    case TOKEN_STRUCT:
    {
      parse_struct(parser);
      break;
    }
    case TOKEN_TYPEDEF:
    {
    }
    case TOKEN_UNION:
    {
      parse_union(parser);
      break;
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
      parse_declaration(parser);
      break;
    }

    case TOKEN_LEFT_PAREN:
    {
      // function pointer?
    }

    case TOKEN_SEMICOLON:
    {
      parser->node->type = NODE_EMPTY;
      break;
    }
    case TOKEN_IDENTIFIER:
    {
      if (is_struct(parser))
      {
        parse_declaration(parser);
        break;
      }
      else
      {
        error("Expected type or smth?");
      }
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
