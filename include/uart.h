#pragma once

#include <kernel.h>
/*
 * Control and Status Register (CSR) definintions for the 16550 UART.
 */
struct uart_csreg
{
	volatile uint32	buffer;	/* receive buffer (when read)		*/
				/*   OR transmit hold (when written)	*/
	volatile uint32	ier;	/* interrupt enable			*/
	volatile uint32	iir;	/* interrupt identification (when read)	*/
				/*   OR FIFO control (when written)	*/
	volatile uint32	lcr;	/* line control register		*/
	volatile uint32	mcr;	/* modem control register		*/
	volatile uint32	lsr;	/* line status register			*/
	volatile uint32	msr;	/* modem status register		*/
	volatile uint32	spr;	/* scratch register			*/
	volatile uint32 mdr1;
	volatile uint32 res[12];/* unused UART registers		*/
	volatile uint32 sysc;	/* system configuration register	*/
	volatile uint32 syss;	/* system status register		*/
	volatile uint32 wer;
	volatile uint32 res4;
	volatile uint32 rxfifo_lvl;
	volatile uint32 txfifo_lvl;
	volatile uint32 ier2;
	volatile uint32 isr2;
	volatile uint32 freq_sel;
	volatile uint32 res5[2];
	volatile uint32 mdr3;
	volatile uint32 tx_dma_thresh;
};

/* Alternative names for control and status registers */

#define	rhr	buffer		/* receive buffer (when read)		*/
#define	thr	buffer		/* transmit hold (when written)		*/
#define	fcr	iir		/* FIFO control (when written)		*/
#define	dll	buffer		/* divisor latch (low byte)		*/
#define	dlh	ier		/* divisor latch (high byte)		*/


/* Line status bits */
#define UART_LSR_THRE	0x20	/* Transmit-hold-register empty	*/
#define	UART_LSR_BI	0x10	/* Break interrupt indicator		*/
#define UART_LSR_DR	0x01	/* Data ready */

/* Pad control addresses and modes */
#define UART0_PADRX_ADDR	0x44E10970
#define UART0_PADTX_ADDR	0x44E10974
#define UART0_PADRX_MODE	0
#define UART0_PADTX_MODE	0

/* SYSC register bits */

#define UART_SYSC_SOFTRESET	0x00000002

/* SYSS register bits */

#define UART_SYSS_RESETDONE	0x00000001

/* Line control bits */

#define	UART_LCR_DLAB	0x80	/* Divisor latch access bit		*/
#define	UART_LCR_8N1	0x03	/* 8 bits, no parity, 1 stop		*/

/* Divisor values for baud rate at 115.38 kbps */

#define	UART_DLL	26	/* value for low byte of divisor latch	*/
#define UART_DLH	0	/* value for high byte of divisor latch	*/

/* FIFO control bits */

#define UART_FCR_EFIFO	0x01	/* Enable in and out hardware FIFOs	*/
#define UART_FCR_RRESET 0x02	/* Reset receiver FIFO			*/
#define UART_FCR_TRESET 0x04	/* Reset transmit FIFO			*/
#define UART_FCR_TRIG0	0x00	/* RCVR FIFO trigger level one char	*/
#define UART_FCR_TRIG1	0x40	/* RCVR FIFO trigger level 1/4		*/
#define UART_FCR_TRIG2	0x80	/* RCVR FIFO trigger level 2/4		*/
#define UART_FCR_TRIG3	0xC0	/* RCVR FIFO trigger level 3/4		*/

/* MDR1 bits	*/
#define UART_MDR1_16X	0x00000000

/* Interrupt enable bits */

#define UART_IER_ERBFI	0x01	/* Received data interrupt mask		*/
#define UART_IER_ETBEI	0x02	/* Transmitter buffer empty interrupt	*/

#define UART_IIR_IRQ	0x01	/* Interrupt pending bit */
#define UART_IIR_IDMASK 0x0E	/* 3-bit field for interrupt ID	*/
#define UART_IIR_MSC	0x00	/* Modem status change			*/
#define UART_IIR_THRE	0x02	/* Transmitter holding register empty	*/
#define UART_IIR_RDA	0x04	/* Receiver data available		*/
#define UART_IIR_RLSI	0x06	/* Receiver line status interrupt	*/
#define UART_IIR_RTO	0x0C	/* Receiver timed out			*/

#define	UART_FIFO_SIZE	64	/* chars in UART onboard output FIFO	*/