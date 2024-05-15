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

void debug_node(AstNode* node)
{
  if (node == 0)
  {
    printf("DEBUG 0 NODE\n");
    return;
  }
  switch (node->type)
  {
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
    printf("){\n");
    AstNode* block = node->down;
    while (block != 0)
    {
      debug_node(block);
      block = block->next;
    }
    printf("}\n");
    break;
  }
  case NODE_ARRAY:
  {
  }
  case NODE_ASSIGN:
  {
  }
  case NODE_EXPRESSION:
  {
  }
  case NODE_FOR:
  {
  }
  case NODE_WHILE:
  {
  }
  case NODE_IF:
  {
  }
  case NODE_RETURN:
  {
    printf("return ");
    debug_node(node->down);
    printf(";\n");
    break;
  }
  case NODE_VARIABLE:
  {
    String literal = node->variable.name->literal;
    printf("%.*s", (i32)literal.len, literal.buffer);
    if (node->down!= 0)
    {
      printf(" = ");
      debug_node(node->down);
    }
    break;
  }
  case NODE_DECLARATION:
  {
    debug_declaration(node->declaration);
    if (node->down)
    {
      AstNode* down = node->down;
      while (true)
      {
        if (!down->next)
        {
          break;
        }
        down = down->next;
      }

      while (true)
      {
        debug_node(down);
        down = down->prev;
        if (down)
        {
          printf(", ");
        }
        else
        {
          break;
        }
      }
    }
    printf(";\n");
    break;
  }
  case NODE_STRUCT:
  {
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
    debug_node(node->down);
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
    debug_node(node->down->next);
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
}
