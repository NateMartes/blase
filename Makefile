CFLAGS=-g -Wall -Wno-unused-function
INCLUDES=addr.h armgen.h ast.h list.h parser.tab.h pool.h quad.h quadtab.h symbol.h symtab.h typecheck.h type.h quadopt.h
SOURCE=addr.c armgen.c ast.c bc.c lex.yy.c list.c parser.tab.c pool.c quad.c quadtab.c symbol.c symtab.c type.c typecheck.c quadopt.c

bc : bc.c $(INCLUDES) $(SOURCE)
	$(CC) $(CFLAGS) -o bc $(SOURCE) -lgc

parser.tab.c parser.tab.h : parser.y
	bison -d parser.y

lex.yy.c : lexer.lex parser.tab.h
	flex lexer.lex

testparser : testparser.c parser.tab.c parser.tab.h lex.yy.c ast.h ast.c pool.h pool.c list.h list.c
	$(CC) -g -o testparser testparser.c parser.tab.c lex.yy.c ast.c pool.c list.c -lgc

testlexer : testlexer.c parser.tab.h lex.yy.c ast.h ast.c pool.h pool.c
	$(CC) -g -o testlexer testlexer.c lex.yy.c ast.c pool.c list.c parser.tab.c -lgc
