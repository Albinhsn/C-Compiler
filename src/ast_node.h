#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"

typedef enum
{
  NODE_FUNCTION,
  NODE_ARRAY,
  NODE_ASSIGN,
  NODE_DO,
  NODE_FOR,
  NODE_WHILE,
  NODE_IF,
  NODE_RETURN,
  NODE_VARIABLE,
  NODE_BLOCK,
  NODE_DECLARATION,
  NODE_STRUCT,
  NODE_UNION,
  NODE_ENUM,
  NODE_DEFINE,
  NODE_INCLUDE,
  NODE_BINARY,
  NODE_CALL,
  NODE_CAST,
  NODE_COMPARISON,
  NODE_DOT,
  NODE_SWITCH,
  NODE_GROUPED,
  NODE_INDEX,
  NODE_CONSTANT,
  NODE_LOGICAL,
  NODE_POSTFIX,
  NODE_UNARY,
  NODE_BREAK,
  NODE_CONTINUE,
  NODE_IDENTIFIER,
  NODE_GOTO,
  NODE_EMPTY

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

typedef WhileNode DoNode;
typedef struct
{
  TokenType op;
  AstNode*  left;
  AstNode*  right;
} BinaryNode;

typedef BinaryNode ComparisonNode;

typedef struct
{
  String*  name;
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
  AstNode* target;
  AstNode* value;
} AssignNode;

typedef struct
{
  AstNode* value;
} ReturnNode;

typedef struct EnumValue EnumValue;

struct EnumValue
{
  String*    name;
  Token*     constant;
  EnumValue* next;
};

typedef struct
{
  EnumValue* values;
  String*    name;
} EnumNode;

typedef struct
{
  AstNode* node;
  Token*   postfix;
} PostfixNode;

typedef struct StructField StructField;
struct StructField
{
  DataType     type;
  String*      name;
  StructField* next;
};

typedef struct
{
  StructField* fields;
  String*      name;
} StructNode;
typedef StructNode    UnionNode;

typedef struct IfBlock IfBlock;
struct IfBlock{
  AstNode * condition;
  AstNode * body;
  IfBlock * next;
};

typedef struct IfNode IfNode;
struct IfNode
{
  IfBlock*  blocks;
  AstNode* else_;
};

typedef struct {
  AstNode * condition;
  AstNode * block;
} SwitchNode;

struct AstNode
{
  AstNodeType type;
  AstNode*    next;
  union
  {
    IfNode          if_;
    ConstantNode    constant;
    FunctionNode    function;
    BinaryNode      binary;
    ComparisonNode  comparison;
    DeclarationNode declaration;
    VariableNode    variable;
    ReturnNode      return_;
    BlockNode       block;
    WhileNode       while_;
    ForNode         for_;
    AssignNode      assign;
    EnumNode        enum_;
    DoNode          do_;
    PostfixNode     postfix;
    StructNode      struct_;
    UnionNode       union_;
    SwitchNode switch_;
  };
};

void debug_node(AstNode* node, int tabs);

#endif
