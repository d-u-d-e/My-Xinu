/*------------------------------------------------------------------------
 *  memcpy  -  Copy a block of memory from src to dst, and return a
 *			  pointer to the destination
 *------------------------------------------------------------------------
 */

void * memcpy(void * dst, const void * src, int n)
{
    register int i;
    char * d = (char *)dst;
    const char * s = (const char *)src;

    for (i = 0; i < n; i++){
        *d++ = *s++;
    }
    return dst;
}