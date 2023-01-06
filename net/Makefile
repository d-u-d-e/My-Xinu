NET_OBJS:= $(patsubst %.c,$(TOPDIR)/compile/objects/%.o,$(wildcard *.c))

all: $(NET_OBJS)

$(TOPDIR)/compile/objects/%.o: %.c
	@$(CC) $(CFLAGS) -o $@ $<
    
