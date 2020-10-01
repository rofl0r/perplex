%{
#define YYSTYPE long long int
#include <stdio.h>
extern int yylex();
extern void yyerror(char*);
extern YYSTYPE solution;
//int yydebug = 1;
%}

%token NUMBER LPAREN RPAREN
%left '+' '-'
%left '*' '/'

%%

statement:
	expr			{ solution = yyval; }

expr:	expr '+' expr		{ $$ = $1 + $3; }
 |	expr '-' expr		{ $$ = $1 - $3; }
 |	expr '*' expr		{ $$ = $1 * $3; }
 |	expr '/' expr		{ $$ = $1 / $3; }
 |	NUMBER			{ $$ = $1; }
 |	LPAREN expr RPAREN	{ $$ = $2; }
 ;

%%

void yyerror(char* s) { fprintf(stderr, "%s\n", s); }

