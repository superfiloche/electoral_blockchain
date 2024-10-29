CCFLAGS = gcc -g -fsanitize=address -Wall -Wextra -c
COFLAGS = gcc -g -fsanitize=address -Wall -Wextra -o
#CCFLAGS = gcc -g -Wall -Wextra -c
#COFLAGS = gcc -g -Wall -Wextra -o

#le flag -fsanitize=address est une alternative à valgrind, très puissant pour trouver les Segfault sans gdb
#il donne aussi un rapport pour les leaks
#il suffit d'éxecuter le programme normalement (se placer dans le répertoire de l'éxectuable et "./main" dans le terminal)
#conclusion : fsanitize broken

all : main

rsa.o : sources/rsa.c
	$(CCFLAGS) sources/rsa.c -o sources/bin/rsa.o

key.o : sources/headers/rsa.h sources/key.c
	$(CCFLAGS) sources/key.c -o sources/bin/key.o

signature.o : sources/headers/key.h sources/headers/rsa.h sources/signature.c
	$(CCFLAGS) sources/signature.c -o sources/bin/signature.o

cellkey.o : sources/headers/key.h sources/cellkey.c
	$(CCFLAGS) sources/cellkey.c -o sources/bin/cellkey.o

cellprotected.o : sources/headers/key.h sources/headers/signature.h sources/cellprotected.c
	$(CCFLAGS) sources/cellprotected.c -o sources/bin/cellprotected.o

hash.o : sources/headers/cellprotected.h sources/headers/cellkey.h sources/hash.c
	$(CCFLAGS) sources/hash.c -o sources/bin/hash.o

block.o : sources/headers/hash.h sources/block.c
	$(CCFLAGS) sources/block.c -o sources/bin/block.o

tree.o : sources/headers/tree.h sources/tree.c
	$(CCFLAGS) sources/tree.c -o sources/bin/tree.o

vote.o : sources/headers/vote.h sources/vote.c
	$(CCFLAGS) sources/vote.c -o sources/bin/vote.o

main.o : sources/headers/rsa.h sources/headers/signature.h sources/headers/key.h sources/headers/cellkey.h sources/headers/cellprotected.h sources/headers/cellprotected.h sources/headers/block.h sources/headers/tree.h sources/headers/vote.h sources/main.c
	$(CCFLAGS) sources/main.c -o sources/bin/main.o

main : rsa.o key.o signature.o cellkey.o cellprotected.o hash.o block.o tree.o vote.o main.o
	$(COFLAGS) main sources/bin/*.o -lm -lssl -lcrypto

clean :
	rm -f sources/bin/*.o
	rm main    