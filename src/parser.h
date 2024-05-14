#ifndef PARSER_H
#define PARSER_H

#include "ast_node.h"
#include "common.h"
#include "scanner.h"
#include "token.h"

typedef struct
{
  Token*   current;
  Token*   previous;
  AstNode* head;
  Scanner* scanner;
  Arena*   arena;
} Parser;

void init_parser(Parser* parser, Arena* arena, Scanner* scanner);
void parse(Parser* parser);

#endif
