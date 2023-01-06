/*
 * Delay units are in microseconds. //core is 2000 MIPS
 */
#define	DELAY(n)                   \
{                                  \
	volatile long N = 2*n;         \
							       \
	while(N > 0) {				   \
		N--;					   \
	}						       \
}

