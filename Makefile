CC=gcc
CFLAGS=-Wall

calc: calc.l calc.y calc.h
	bison -d -Wcounterexamples calc.y
	flex calc.l
	$(CC) $(CFLAGS) -o $@ calc.tab.c lex.yy.c calc.c -lfl
	
.PHONY clean:
	rm -f calc 
	rm -f calc.tab* 
	rm -f lex.yy.c