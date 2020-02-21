# Target library
lib := libuthread.a
o_file := sem.o tps.o
compiled_ofile := thread.o queue.o 

CC  := gcc
CFLAGS := -Wall -Wextra -Werror

# Include dependencies
deps := $(patsubst %.o,%.d,$(o_file))
-include $(deps)

DEPFLAGS = -MMD -MF $(@:.o=.d)

all: $(lib)

$(lib): $(o_file) $(compiled_ofile)
	ar rcs $@ $^

%.o: %.c
	@echo "CC    $@"
	@$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

clean:
	@echo "CLEAN"
	@rm -f $(lib) $(o_file) $(deps)

