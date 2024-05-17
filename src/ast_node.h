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
  NODE_BLOCK,
  NODE_DECLARATION,
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
  unsigned char size;       // bitfield
  bool          signedness; // signed/unsigned
} DataTypeInteger;

typedef struct
{
  unsigned char size;
} DataTypeFloat;

typedef struct
{
  Token* token; // name
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

typedef struct AstNode AstNode;

typedef struct
{
  AstNode* nodes;
} BlockNode;

typedef struct
{
  DataType  return_type;
  String*   name;
  Argument* arguments;
  int       argument_count;
  AstNode*  block;
} FunctionNode;

typedef struct
{
  AstNode* init;
  AstNode* condition;
  AstNode* update;
  AstNode* body;
} ForNode;

typedef struct
{
  AstNode* condition;
  AstNode* body;
} WhileNode;

typedef struct
{
  TokenType op;
  AstNode*  left;
  AstNode*  right;
} BinaryNode;

typedef struct
{
  Token*   name;
  AstNode* value;
} VariableNode;

typedef struct
{
  DataType type;
  char     type_qualifier;
  char     storage_specifier;
  AstNode* variables;

} DeclarationNode;

typedef struct
{
  AstNode* value;
} ReturnNode;

struct AstNode
{
  AstNodeType type;
  AstNode*    next;
  union
  {
    ConstantNode    constant;
    FunctionNode    function;
    BinaryNode      binary;
    DeclarationNode declaration;
    VariableNode    variable;
    ReturnNode      return_;
    BlockNode       block;
    WhileNode       while_;
    ForNode         for_;
  };
};

void debug_node(AstNode* node);

#endif
