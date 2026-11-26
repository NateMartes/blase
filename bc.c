#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gc/gc.h>
#include "ast.h"
#include "pool.h"
#include "parser.tab.h"
#include "symtab.h"
#include "quadtab.h"
#include "quadopt.h"
#include "typecheck.h"
#include "armgen.h"

int main(int argc, char *argv[]) {
  GC_INIT();

  // Check for an argument.
  if (argc != 2) {
    fprintf(stderr, "usage: %s file\n", argv[0]);
    exit(1);
  }
  
  // Check the source file name.
  char *ext = rindex(argv[1], '.');
  if (ext == NULL || strcmp(ext, ".bl") != 0) {
    fprintf(stderr, "%s: source file name must end in .bl\n", argv[0]);
    exit(1);
  }

  // Open the source file and assign it to stdin.
  stdin = freopen(argv[1], "r", stdin);
  if (stdin == NULL) {
    fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
    exit(1);
  }

  
  // Parse the source and assign the AST to prog.
  program *prog = NULL;
  yyparse(&prog);

  // Create the symbol table and typecheck the program.
  symtab *st = symtab_new();

  // Do intermediate code generation.
  if (typecheck_prog(st, prog)) {
    quadtab *qt = quadtab_new();
    quadtab_gen(qt, st, prog);
    fprintf(stderr, "Quad Table:\n");
    quadtab_print(qt);
    printf("\nOptimizing Jumps\n\n");
    opt_jumps(qt, st);
    quadtab_print(qt);
    
    // Open the assembly file.
    int len = strlen(argv[1]);
    char *asmfile = GC_MALLOC(len);
    strncpy(asmfile, argv[1], len - 2);
    asmfile[len - 2] = 's';
    asmfile[len - 1] = '\0';
    stdout = freopen(asmfile, "w", stdout);
    if (stdout == NULL) {
      fprintf(stderr, "%s: %s: %s\n", argv[0], asmfile, strerror(errno));
    }
    
    // Generate assembly code.
    gen_program(qt, st);
  }
}
