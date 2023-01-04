#pragma once
#include <kernel.h>
#include <conf.h>

#define	NM_MAXLEN	256		/* Maximum size of a file name	*/


status naminit(void);
devcall namopen(struct dentry * devptr, char * name, char * mode);