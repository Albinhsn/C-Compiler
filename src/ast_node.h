#ifndef AST_NODE_H
#define AST_NODE_H

typedef enum
{
  NODE_FUNCTION,
  NODE_ARRAY,
  NODE_ASSIGN,
  NODE_EXPRESSION,
  NODE_FOR,
  NODE_WHILE,
  NODE_IF,
  NODE_RETURN,
  NODE_VARIABLE,
  NODE_STRUCT,
  NODE_DEFINE,
  NODE_INCLUDE,
  NODE_BINARY,
  NODE_CALL,
  NODE_CAST,
  NODE_COMPARISON,
  NODE_DOT,
  NODE_GROUPED,
  NODE_INDEX,
  NODE_LITERAL,
  NODE_LOGICAL,
  NODE_POSTFIX,
  NODE_UNARY,
  NODE_IDENTIFIER
} AstNodeType;

typedef struct AstNode AstNode;

struct AstNode
{
  AstNode*    next;
  AstNode*    first;
  AstNode*    last;
  AstNodeType type;
};

#endif
