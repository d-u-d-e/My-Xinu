LIB_OBJS:= $(patsubst %.c,$(TOPDIR)/compile/objects/%.o,$(wildcard *.c))

all: $(LIB_OBJS)

$(TOPDIR)/compile/objects/%.o: %.c
	@$(CC) $(CFLAGS) -o $@ $<
