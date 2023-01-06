KERNEL_OBJS:= $(patsubst %.c,$(TOPDIR)/compile/objects/%.o,$(wildcard *.c)) $(patsubst %.S,$(TOPDIR)/compile/objects/%.o,$(wildcard *.S))

all: $(KERNEL_OBJS)

$(TOPDIR)/compile/objects/%.o: %.c
	@$(CC) $(CFLAGS) -o $@ $<
	
$(TOPDIR)/compile/objects/%.o: %.S
	@$(CC) $(CFLAGS) -o $@ $<
    
