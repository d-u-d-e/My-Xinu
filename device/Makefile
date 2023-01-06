
DEV_OBJS:= $(patsubst %.c,%.o,$(wildcard **/*.c))

all: $(DEV_OBJS)

lfs/%.o: lfs/%.c
	@$(CC) $(CFLAGS) -o $(TOPDIR)/compile/objects/$(notdir $@) $<

nam/%.o: nam/%.c
	@$(CC) $(CFLAGS) -o $(TOPDIR)/compile/objects/$(notdir $@) $<
	
ramdisk/%.o: ramdisk/%.c
	@$(CC) $(CFLAGS) -o $(TOPDIR)/compile/objects/$(notdir $@) $<
	
tty/%.o: tty/%.c
	@$(CC) $(CFLAGS) -o $(TOPDIR)/compile/objects/$(notdir $@) $<

eth/%.o: eth/%.c
	@$(CC) $(CFLAGS) -o $(TOPDIR)/compile/objects/$(notdir $@) $<
