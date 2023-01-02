TOPDIR:= ..
COMPILER_ROOT:= cross/bin/arm-none-eabi-
PNAME:= bbb
PLAT:= Platform_$(PNAME)

CC:=		$(COMPILER_ROOT)gcc
LD:=		${COMPILER_ROOT}ld
OBJCOPY:=	${COMPILER_ROOT}objcopy

MY_XINU:= $(TOPDIR)/compile/my_xinu.elf
MY_XINUBIN:= $(TOPDIR)/compile/my_xinu.bin

VERSIONFILE:= version
LDSCRIPT:= ld.script

INCLUDE:= -I$(TOPDIR)/include

DEFS:= -DVERSION=\""`cat $(VERSIONFILE)`"\"

KERNEL_OBJS:= $(patsubst %.S,%.o,$(wildcard $(TOPDIR)/kernel/*.S)) $(patsubst %.c,%.o,$(wildcard $(TOPDIR)/kernel/*.c)) 
LIB_OBJS:= $(patsubst %.c,%.o,$(wildcard $(TOPDIR)/lib/*.c))
DEV_OBJS:= $(patsubst %.c,%.o,$(wildcard $(TOPDIR)/device/**/*.c))
OBJ_FILES:= $(KERNEL_OBJS) $(LIB_OBJS) $(DEV_OBJS)

#--------------------------------------------------------------------------------
# Add files that require special rules and place start.o at the front
# The linker searches from left to right, and notes unresolved symbols as it goes
#--------------------------------------------------------------------------------
LD_LIST:= $(TOPDIR)/kernel/start.o $(filter-out $(TOPDIR)/kernel/start.o,$(OBJ_FILES))

# Compiler flags
CFLAGS:= -mcpu=cortex-a8 -mno-unaligned-access -marm -ffreestanding -fno-stack-protector -nostdlib -c -Wall -fcompare-debug-second -O $(DEFS) $(INCLUDE)

LDFLAGS:= -dn -Map=my_xinu.map -T ld.script

export

all: my_xinu

my_xinu: Makefile $(LD_LIST)
	@$(LD) $(LDFLAGS) $(LD_LIST) -Lcross/lib/gcc/arm-none-eabi/12.2.1 -lgcc -o $(MY_XINU)
	@$(OBJCOPY) -O binary $(MY_XINU) $(MY_XINUBIN)

$(TOPDIR)/kernel/%.o: $(TOPDIR)/kernel/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(TOPDIR)/kernel/%.o: $(TOPDIR)/kernel/%.S
	$(CC) ${CFLAGS} -o $@ $<

$(TOPDIR)/lib/%.o: $(TOPDIR)/lib/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(TOPDIR)/device/%.o: $(TOPDIR)/lib/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TOPDIR)/kernel/*.o
	rm -f $(TOPDIR)/lib/*.o
	rm -f $(TOPDIR)/device/*.o
