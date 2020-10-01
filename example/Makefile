GEN = y.tab.c y.tab.h lexer.re lexer.c lexer.h

all: calc

y.tab.c: parser.y
	yacc -d parser.y

y.tab.h: y.tab.c

lexer.re: lexer.ppl
	../perplex -t ../perplex_template.c -i lexer.h -o $@ $<

lexer.h: lexer.re

lexer.c: lexer.re
	re2c $< > $@

calc: calc.c y.tab.c lexer.c
	$(CC) $(CFLAGS) -DPERPLEX_LEXER=ppl_yylex $^ -o $@

clean:
	rm -f calc $(GEN)

.PHONY: all clean

