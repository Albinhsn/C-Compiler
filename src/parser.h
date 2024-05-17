#ifndef PARSER_H
#define PARSER_H

#include "ast_node.h"
#include "common.h"
#include "precedence.h"
#include "scanner.h"
#include "token.h"

typedef enum
{
  STORAGE_EXTERN   = 1,
  STORAGE_STATIC   = 2,
  STORAGE_AUTO     = 4,
  STORAGE_REGISTER = 8
} StorageSpecifier;

typedef enum
{
  TYPE_QUAL_CONST    = 1,
  TYPE_QUAL_RESTRICT = 2,
  TYPE_QUAL_VOLATILE = 4,
} TypeQualifier;

typedef struct
{
  Token*   current;
  Token*   previous;
  AstNode* node;
  Scanner* scanner;
} Parser;

typedef void (*ParseFn)(Parser* parser, bool canAssign);

typedef struct
{
  ParseFn    prefix;
  ParseFn    infix;
  Precedence precedence;
} ParseRule;

void     init_parser(Parser* parser, Scanner* scanner);
AstNode* parse(Parser* parser);

#endif
