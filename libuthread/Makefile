# Target library
lib := libuthread.a
o_file := sem.o thread.o queue.o

CC  := gcc
CFLAGS := -Wall -Wextra -Werror
DEPFLAGS = -MMD -MF $(@:.o=.d)

all: $(lib)

## TODO: Phase 1 and Phase 2

$(lib): $(o_file)
	ar rcs $@ $^
	ranlib $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

clean:
	rm -f $(lib) $(o_file)

