all: argshell

argshell: lex.yy.c
	 cc -o argshell argshell.c lex.yy.c -lfl

lex.yy.c:
	flex shell.l

clean:
	rm argshell lex.yy.c
