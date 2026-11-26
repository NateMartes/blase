%{
  #include <stdlib.h>
  #include <gc/gc.h>
  #include "ast.h"
  #include "pool.h"
  #include "parser.tab.h"

  #define malloc GC_MALLOC
%}

%option noyywrap
%option yylineno

whitespace             [ \t\n]+
int_const               [0-9]+
identifier             [a-zA-Z][a-zA-Z0-9]*

%%

{whitespace}  { }
div           { return DIV; }
mod           { return MOD; }
and           { return AND; }
or            { return OR; }
not           { return NOT; }
true          { return TRUE; }
false         { return FALSE; }
write         { return WRITE; }
writeln       { return WRITELN; }
if            { return IF; }
then          { return THEN; }
else          { return ELSE; }
while         { return WHILE; }
do            { return DO; }
begin         { return BEEGIN; }
end           { return END; }
program       { return PROGRAM; }
integer       { return INTEGER; }
boolean       { return BOOLEAN; }
var           { return VAR; }
":="          { return ASSIGN; }
"<="          { return LE; }
">="          { return GE; }
"<>"          { return NE; }
{identifier}  { yylval.string = intern(yytext); return IDENTIFIER; }
{int_const}   { yylval.constant = atol(yytext); return INT_CONST; }
[+-;:()*<>=.] { return *yytext; }
.             { fprintf(stderr, "Unrecognized character: %c\n", *yytext); }

%%
