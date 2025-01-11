CC=gcc
CFLAGS=-Wall

proiect: proiect_flex.l proiect_bison.y astNode.h
	bison -d -Wcounterexamples proiect_bison.y
	flex proiect_flex.l
	$(CC) $(CFLAGS) -o $@ proiect_bison.tab.c lex.yy.c  astNode.c interface.c
	
.PHONY clean:
	rm -f proiect
	rm -f proiect_bison.tab* 
	rm -f lex.yy.c