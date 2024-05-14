#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"

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
  NODE_CONSTANT,
  NODE_LOGICAL,
  NODE_POSTFIX,
  NODE_UNARY,
  NODE_BREAK,
  NODE_CONTINUE,
  NODE_IDENTIFIER,
  NODE_GOTO

} AstNodeType;

typedef struct
{
  Token* token;
} ConstantNode;

typedef enum
{
  DATA_TYPE_INTEGER,
  DATA_TYPE_STRUCT,
  DATA_TYPE_FLOATING_POINT,
  DATA_TYPE_FUNCTION,
} DataTypeType;

typedef struct
{
  Token*        token; // literal
  unsigned char size; // bitfield
  unsigned char type;       // hex, octal, binary etc
  bool          signedness; // signed/unsigned
} DataTypeInteger;

typedef struct
{
  Token*        token; // literal
  unsigned char size;
  bool          type; // hex/decimal
} DataTypeFloat;

typedef struct
{
  Token*        token; // name
} DataTypeStruct;

typedef struct
{
  // return type
  // params
  // name
} DataTypeFunction;

typedef struct
{
  DataTypeType type;
  union
  {
    DataTypeInteger  integer;
    DataTypeFloat    floating_point;
    DataTypeFunction function;
  };
  int depth;
} DataType;

typedef struct
{
  String*  name;
  DataType type;
} Argument;

typedef struct
{
  DataType  return_type;
  String*   name;
  Argument* arguments;
  int       argument_count;
} FunctionNode;

typedef struct AstNode AstNode;
struct AstNode
{
  // next horizontally
  AstNode* next;
  // next down one
  AstNode*    down;
  AstNodeType type;
  union
  {
    ConstantNode constant;
    FunctionNode function;
  };
};

#endif
