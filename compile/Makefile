TOPDIR:= /home/marco/Scrivania/my_xinu_bbb
COMPILER_ROOT:= $(TOPDIR)/compile/cross/bin/arm-none-eabi-
PNAME:= bbb
PLAT:= Platform_$(PNAME)

CC:=		$(COMPILER_ROOT)gcc
LD:=		${COMPILER_ROOT}ld
OBJCOPY:=	${COMPILER_ROOT}objcopy

MY_XINU:= $(TOPDIR)/compile/my_xinu.elf
MY_XINUBIN:= $(TOPDIR)/compile/my_xinu.bin

VERSIONFILE:= $(TOPDIR)/compile/version

INCLUDE:= -I$(TOPDIR)/include

DEFS:= -DVERSION=\""`cat $(VERSIONFILE)`"\"

MAKEFLAGS += --no-builtin-rules

# Compiler flags
CFLAGS:= -mcpu=cortex-a8 -mno-unaligned-access -marm -ffreestanding -fno-stack-protector -nostdlib -c -Wall -fcompare-debug-second -O $(DEFS) $(INCLUDE)

export

LDSCRIPT:= ld.script
LDFLAGS:= -dn -Map=my_xinu.map -T ld.script

all: my_xinu

#--------------------------------------------------------------------------------
# Add files that require special rules and place start.o at the front
# The linker searches from left to right, and notes unresolved symbols as it goes
#--------------------------------------------------------------------------------

my_xinu: Makefile device kernel lib shell net
	$(eval LD_LIST := $(TOPDIR)/compile/objects/start.o $(filter-out $(TOPDIR)/compile/objects/start.o,$(wildcard $(TOPDIR)/compile/objects/*.o)))
	@$(LD) $(LDFLAGS) $(LD_LIST) -Lcross/lib/gcc/arm-none-eabi/12.2.1 -lgcc -o $(MY_XINU)
	@$(OBJCOPY) -O binary $(MY_XINU) $(MY_XINUBIN)

objects_dir:
	@mkdir -p objects

kernel: objects_dir
	make -C $(TOPDIR)/kernel
	
device: objects_dir
	make -C $(TOPDIR)/device
	
lib: objects_dir
	make -C $(TOPDIR)/lib
	
shell: objects_dir
	make -C $(TOPDIR)/shell
	
net: objects_dir
	make -C $(TOPDIR)/net

clean:
	rm -f -r $(TOPDIR)/compile/objects
	rm -f $(MY_XINU) $(MY_XINUBIN)
