#include "ast_node.h"
#include "parser.h"
#include "token.h"
#include <stdlib.h>

void debug_data_type(DataType type)
{
  switch (type.type)
  {
  case DATA_TYPE_INTEGER:
  {
    if (!type.integer.signedness)
    {
      printf("unsigned ");
    }
    switch (type.integer.size)
    {
    case 1:
    {
      printf("char");
      break;
    }
    case 2:
    {
      printf("short");
      break;
    }
    case 4:
    {
      printf("int");
      break;
    }
    case 8:
    {
      printf("long");
      break;
    }
    }
  }
  case DATA_TYPE_FLOATING_POINT:
  {
  }
  case DATA_TYPE_FUNCTION:
  {
  }
  case DATA_TYPE_STRUCT:
  {
  }
  }
  for (int i = 0; i < type.depth; i++)
  {
    printf("*");
  }
  printf(" ");
}
void debug_argument(Argument argument)
{
}

void debug_declaration(DeclarationNode declaration)
{
  if (declaration.type_qualifier != 0)
  {
    if ((declaration.type_qualifier & TYPE_QUAL_CONST) != 0)
    {
      printf("const ");
    }
    if ((declaration.type_qualifier & TYPE_QUAL_RESTRICT) != 0)
    {
      printf("restrict ");
    }
    if ((declaration.type_qualifier & TYPE_QUAL_VOLATILE) != 0)
    {
      printf("volatile ");
    }
  }

  if (declaration.storage_specifier != 0)
  {
    if ((declaration.storage_specifier & STORAGE_REGISTER) != 0)
    {
      printf("register ");
    }
    else if ((declaration.storage_specifier & STORAGE_AUTO) != 0)
    {
      printf("auto ");
    }
    else if ((declaration.storage_specifier & STORAGE_STATIC) != 0)
    {
      printf("static ");
    }
    else if ((declaration.storage_specifier & STORAGE_EXTERN) != 0)
    {
      printf("extern ");
    }
  }

  debug_data_type(declaration.type);
}

static inline void print_tabs(int tabs)
{
  for (int i = 0; i < tabs; i++)
  {
    printf("\t");
  }
}

void debug_node(AstNode* node, int tabs);

void debug_block(AstNode* block, int tabs)
{
  printf("{\n");
  AstNode* curr = block;
  debug_node(curr, tabs + 1);
  print_tabs(tabs);
  printf("}\n");
}

void debug_node(AstNode* node, int tabs)
{
  print_tabs(tabs);
  if (node == 0)
  {
    printf("DEBUG 0 NODE\n");
    return;
  }
  switch (node->type)
  {
  case NODE_DO:
  {
    printf("do\n");
    debug_node(node->do_.body, tabs);
    print_tabs(tabs);
    printf("while(");
    debug_node(node->do_.condition, 0);
    printf(");\n");
    break;
  }
  case NODE_ENUM:
  {
    EnumNode* enum_node = &node->enum_;
    printf("enum {\n");
    EnumValue* value = enum_node->values;
    while (value != NULL)
    {
      print_tabs(tabs + 1);
      printf("%.*s", (i32)value->name->len, value->name->buffer);
      if (value->constant != 0)
      {

        printf(" = ");
        debug_token(value->constant);
      }
      if (value->next)
      {
        printf(",");
      }
      printf("\n");

      value = value->next;
    }
    printf("};\n");
    break;
  }
  case NODE_EMPTY:
  {
    printf(";\n");
    break;
  }
  case NODE_BLOCK:
  {
    debug_block(node->block.nodes, tabs);
    break;
  }
  case NODE_FUNCTION:
  {
    FunctionNode* func = &node->function;
    debug_data_type(func->return_type);
    printf(" %.*s(", (i32)func->name->len, func->name->buffer);
    for (int i = 0; i < func->argument_count; i++)
    {
      debug_argument(func->arguments[i]);
      if (i < func->argument_count - 1)
      {
        printf(", ");
      }
    }
    printf(")");
    debug_node(node->function.block, tabs);
    break;
  }
  case NODE_ARRAY:
  {
  }
  case NODE_ASSIGN:
  {
    AssignNode* assign = &node->assign;
    debug_node(assign->target, 0);
    printf(" = ");
    debug_node(assign->value, 0);
    if (assign->value->type != NODE_ASSIGN)
    {
      printf(";\n");
    }
    break;
  }
  case NODE_FOR:
  {
    ForNode* for_node = &node->for_;
    printf("for(");
    if (for_node->init)
    {
      debug_node(for_node->init, 0);
    }
    printf(";");
    if (for_node->condition)
    {
      printf(" ");
      debug_node(for_node->condition, 0);
    }
    printf(";");
    if (for_node->update)
    {
      printf(" ");
      debug_node(for_node->update, 0);
    }
    printf(")\n");
    debug_node(for_node->body, tabs);
    break;
  }
  case NODE_WHILE:
  {
    printf("while(");
    debug_node(node->do_.condition, 0);
    printf(")");
    debug_node(node->do_.body, tabs);
    break;
  }
  case NODE_IF:
  {
  }
  case NODE_RETURN:
  {
    printf("return ");
    debug_node(node->return_.value, tabs);
    printf(";\n");
    break;
  }
  case NODE_VARIABLE:
  {
    String literal = *node->variable.name;
    printf("%.*s", (i32)literal.len, literal.buffer);
    if (node->variable.value != NULL)
    {
      printf(" = ");
      debug_node(node->variable.value, 0);
    }
    break;
  }
  case NODE_DECLARATION:
  {
    debug_declaration(node->declaration);
    if (node->declaration.variables)
    {
      AstNode* down = node->declaration.variables;
      debug_node(down, 0);
      printf(";\n");
      break;
    }
  }
  case NODE_UNION:
  {
    UnionNode* strukt = &node->union_;
    printf("union %.*s {\n", (i32)strukt->name->len, strukt->name->buffer);
    StructField* field = strukt->fields;
    while (field)
    {
      print_tabs(tabs + 1);
      debug_data_type(field->type);
      printf(" %.*s;", (i32)field->name->len, field->name->buffer);
      field = field->next;

      printf("\n");
    }
    printf("};\n");
    break;
  }
  case NODE_STRUCT:
  {
    StructNode* strukt = &node->struct_;
    printf("struct %.*s {\n", (i32)strukt->name->len, strukt->name->buffer);
    StructField* field = strukt->fields;
    while (field)
    {
      print_tabs(tabs + 1);
      debug_data_type(field->type);
      printf(" %.*s;", (i32)field->name->len, field->name->buffer);
      field = field->next;

      printf("\n");
    }
    printf("};\n");
    break;
  }
  case NODE_DEFINE:
  {
  }
  case NODE_INCLUDE:
  {
  }
  case NODE_BINARY:
  {
    printf("(");
    debug_node(node->binary.left, 0);
    switch (node->binary.op)
    {
    case TOKEN_PLUS:
    {
      printf(" + ");
      break;
    }
    case TOKEN_MINUS:
    {
      printf(" - ");
      break;
    }
    case TOKEN_STAR:
    {
      printf(" * ");
      break;
    }
    case TOKEN_SLASH:
    {
      printf(" / ");
      break;
    }
    default:
    {
      printf("Unknown token %s\n", get_token_type_string(node->binary.op));
      exit(1);
    }
    }
    debug_node(node->binary.right, 0);
    printf(")");
    break;
  }
  case NODE_CALL:
  {
  }
  case NODE_CAST:
  {
  }
  case NODE_COMPARISON:
  {

    printf("(");
    debug_node(node->comparison.left, 0);
    switch (node->comparison.op)
    {
    case TOKEN_LESS:
    {
      printf(" < ");
      break;
    }
    case TOKEN_LESS_EQUAL:
    {
      printf(" <= ");
      break;
    }
    case TOKEN_GREATER:
    {
      printf(" > ");
      break;
    }
    case TOKEN_GREATER_EQUAL:
    {
      printf(" >= ");
      break;
    }
    case TOKEN_EQUAL_EQUAL:
    {
      printf(" == ");
      break;
    }
    case TOKEN_BANG_EQUAL:
    {
      printf(" != ");
      break;
    }
    default:
    {
      printf("Unknown token %s\n", get_token_type_string(node->binary.op));
      exit(1);
    }
    }
    debug_node(node->comparison.right, 0);
    printf(")");
    break;
  }
  case NODE_DOT:
  {
  }
  case NODE_GROUPED:
  {
  }
  case NODE_INDEX:
  {
  }
  case NODE_CONSTANT:
  {
    String literal = node->constant.token->literal;
    printf("%.*s", (i32)literal.len, literal.buffer);
    break;
  }
  case NODE_LOGICAL:
  {
  }
  case NODE_POSTFIX:
  {
    PostfixNode* postfix = &node->postfix;
    debug_node(postfix->node, 0);
    debug_token(postfix->postfix);
    break;
  }
  case NODE_UNARY:
  {
  }
  case NODE_BREAK:
  {
  }
  case NODE_CONTINUE:
  {
  }
  case NODE_IDENTIFIER:
  {
  }
  case NODE_GOTO:
  {
  }
  }

  if (node->next)
  {
    debug_node(node->next, tabs);
  }
}
