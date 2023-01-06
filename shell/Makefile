SHELL_OBJS:= $(patsubst %.c,$(TOPDIR)/compile/objects/%.o,$(wildcard *.c))

all: $(SHELL_OBJS)

$(TOPDIR)/compile/objects/%.o: %.c
	@$(CC) $(CFLAGS) -o $@ $<
    
