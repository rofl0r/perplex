OFILES = parser.o perplex.o scanner.o
HOSTCC = $(CC)

GENF = parser.c parser.h scanner.c

all: perplex

clean:
	rm -f $(OFILES) $(GENF) lemon perplex

lemon: lemon.c lempar.c
	$(HOSTCC) $< -o $@

perplex: $(OFILES)
scanner.c: scanner.re
	re2c -c -o $@ $<

scanner.o: scanner.c parser.h
parser.c: parser.y
	./lemon $<

parser.h: parser.c
parser.y: lemon
perplex.o: perplex.c perplex.h parser.h

.PHONY: all clean
