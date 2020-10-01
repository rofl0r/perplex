#ifndef PERPLEX_LEXER
#error PERPLEX_LEXER needs to be set as a CFLAG for all input files
#endif
#include "lexer.h"
#include "y.tab.h"
#include <stdio.h>
#include <string.h>

long long int solution;
extern int yyparse();

static perplex_t scanner;

int yylex() {
	return PERPLEX_LEXER(scanner);
}

int main() {
	char buf[1024];
	while(fgets(buf, sizeof buf, stdin)) {
		int tid;
		scanner = perplexStringScanner(buf, strlen(buf));
		yyparse();
		printf("%lld\n", solution);
//		while((tid = yylex(scanner)) != YYEOF)
//			printf("%d\n", tid);
		perplexFree(scanner);
	}
}
