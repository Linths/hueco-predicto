CFLAGS= -g -Wall -D OSX
PHOENIX_OBS=  dict.o dm.o exec.o frames.o grammar.o match.o parse.o phoenix.o print_structs.o util.o functions.o rules.o rules_grammar.o rules_grammar.o rules_stack.o

all: libparse.a compile_grammar parse_stdin dm_stdin

libparse.a:  $(PHOENIX_OBS)
	ar rv $@ $?
	ranlib $@

dict.o:   dict.c rules_grammar.o rules_stack.o rules.o
	gcc $(CFLAGS) -c dict.c 

dm.o:   dm.c dm.h 
	gcc $(CFLAGS) -c dm.c

exec.o:   exec.c phoenix.h 
	gcc $(CFLAGS) -c exec.c

frames.o:   frames.c
	gcc $(CFLAGS) -c frames.c

functions.o:   functions.c
	gcc $(CFLAGS) -c functions.c

grammar.o:   grammar.c phoenix.h
	gcc $(CFLAGS) -c grammar.c 

match.o:   match.c phoenix.h 
	gcc $(CFLAGS) -c match.c 

parse.o:   parse.c phoenix.h
	gcc $(CFLAGS) -c parse.c 

phoenix.o:   phoenix.c
	gcc $(CFLAGS) -c phoenix.c 

print_structs.o:   print_structs.c phoenix.h
	gcc $(CFLAGS) -c print_structs.c 

util.o:   util.c
	gcc $(CFLAGS) -c util.c 

rules.o:  rules.c rules.h 
	gcc $(CFLAGS) -c rules.c

rules_stack.o: rules_stack.c rules_stack.h
	gcc $(CFLAGS) -c rules_stack.c

rules_grammar.o:  rules_grammar.c rules_grammar.h rules.o rules_stack.o
	gcc $(CFLAGS) -c rules_grammar.c 

compile_grammar:  compile_grammar.o concept_leaf.o libparse.a
	gcc  -o compile_grammar compile_grammar.o concept_leaf.o libparse.a

compile_grammar.o:   compile_grammar.c 
	gcc $(CFLAGS) -c compile_grammar.c 

concept_leaf.o:   concept_leaf.c
	gcc $(CFLAGS) -c concept_leaf.c 

parse_stdin:  parse_stdin.o libparse.a
	gcc $(CFLAGS)  -o parse_stdin parse_stdin.o libparse.a

parse_stdin.o:   parse_stdin.c phoenix.h
	gcc $(CFLAGS) -c parse_stdin.c 

dm_stdin:  dm_stdin.o libparse.a
	gcc $(CFLAGS)  -o dm_stdin dm_stdin.o libparse.a

dm_stdin.o:   dm_stdin.c phoenix.h
	gcc $(CFLAGS) -c dm_stdin.c 

clean:
	rm *.o libparse.a parse_stdin compile_grammar dm_stdin
