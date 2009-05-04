OBJS=peach-finder.o utils.o entry.o ui.o files.o
CFLAGS=-Wall -O2 -g

all: peach-finder

peach-finder: $(OBJS)
	g++ $(CFLAGS) -o peach-finder -lncursesw $(OBJS)

%.o: %.cc peach.h Makefile
	g++ $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) peach-finder

.PHONY: clean
