#ifndef PARSER_H
#define PARSER_H

#include "ast_node.h"
#include "common.h"
#include "scanner.h"
#include "token.h"
#include "precedence.h"


typedef struct
{
  Token*   current;
  Token*   previous;
  AstNode* node;
  Scanner* scanner;
} Parser;


typedef void         (*ParseFn)(Parser* parser, bool canAssign);

typedef struct{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

void init_parser(Parser* parser, Scanner* scanner);
AstNode * parse(Parser* parser);

#endif
