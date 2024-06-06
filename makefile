CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Wno-return-local-addr -Wunsafe-loop-optimizations -Wuninitialized -Werror -Wno-unused-parameter -Werror
PROG = thread_hash
INCLUDES = thread_hash.h
PROGS = ${PROG}
LIBS = -lcrypt

all: ${PROGS}

$(PROG): $(PROG).o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -lz

$(PROG).o: $(PROG).c $(INCLUDES)
	$(CC) $(CFLAGS) -c $<

tar:
	tar cvfa Lab4.tar.gz *.[ch] [mM]akefile

clean cls:
	rm -f $(PROGS) *.o *~ \#*
