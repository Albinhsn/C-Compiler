
#include "common.h"
#include "files.h"
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    printf("Need filename!\n");
    return 1;
  }
  String file     = {};
  i32    size     = 4096 * 4096;
  Arena  arena    = {};
  arena.ptr       = 0;
  arena.memory    = (u64)malloc(size);
  arena.maxSize   = size;
  Scanner scanner = {};

  bool    read    = sta_read_file(&arena, &file, argv[1]);
  if (!read)
  {
    printf("Couldn't read file %s\n", argv[1]);
    return 1;
  }
  init_scanner(&scanner, &arena, &file, argv[1]);

  Parser parser = {};
  init_parser(&parser, &scanner);
  AstNode* head = parse(&parser);
  debug_node(head);

  return 0;
}
