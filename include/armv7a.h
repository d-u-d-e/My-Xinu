#pragma once

/* Coprocessor CP15 c1 - Control Register bits */

#define ARMV7A_C1CTL_V	0x00002000	/* Exception base addr control	*/
#define ARMV7A_C1CTL_I	0x00001000	/* Instruction Cache enable	*/
#define ARMV7A_C1CTL_C	0x00000004	/* Data Cache enable		*/
#define ARMV7A_C1CTL_A	0x00000002	/* Strict alignment enable	*/
#define ARMV7A_C1CTL_M	0x00000001	/* MMU enable			*/

#define MAXADDR	0xA0000000	/* 512 MiB RAM starting from 0x80000000	*/

/* kernel is loaded by u-boot at 0x81000000, i.e. after 16 MiB */

/* Exception Vector Addresses */

/* 0x20 = 32 -> 32 / 4 = 8 handlers */

#define ARMV7A_EV_START	0x4030CE00	/* Exception vector start addr	*/
#define ARMV7A_EV_END	0x4030CE20	/* Exception vector end addr	*/
#define ARMV7A_EH_START 0x4030CE24	/* Exception handler start addr	*/
#define ARMV7A_EH_END	0x4030CE40	/* Exception handler end addr	*/
#define ARMV7A_IRQH_ADDR 0x4030CE38	/* IRQ exp handler address	*/