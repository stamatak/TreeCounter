# Makefile June 2011 by Alexandros Stamatakis

CC = gcc 

CFLAGS = -O2 -fomit-frame-pointer -funroll-loops #-Wall -pedantic -Wunused-parameter -Wredundant-decls  -Wreturn-type  -Wswitch-default -Wunused-value -Wimplicit  -Wimplicit-function-declaration  -Wimplicit-int -Wimport  -Wunused  -Wunused-function  -Wunused-label -Wno-int-to-pointer-cast -Wbad-function-cast  -Wmissing-declarations -Wmissing-prototypes  -Wnested-externs  -Wold-style-definition -Wstrict-prototypes  -Wdeclaration-after-statement -Wpointer-sign -Wextra -Wredundant-decls -Wunused -Wunused-function -Wunused-parameter -Wunused-value  -Wunused-variable -Wformat  -Wformat-nonliteral -Wparentheses -Wsequence-point -Wuninitialized -Wundef -Wbad-function-cast

LIBRARIES = -lm -lgmp

RM = rm -f

objs    = treeCounter.o

all : treeCounter

GLOBAL_DEPS = axml.h globalVariables.h

treeCounter : $(objs)
	$(CC) -o treeCounter $(objs) $(LIBRARIES) 

treeCounter.o : treeCounter.c


clean : 
	$(RM) *.o treeCounter
