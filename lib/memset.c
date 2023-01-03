/*------------------------------------------------------------------------
 *  memset  -  Set a block ot n bytes to the same value and return a
 *			   pointer to the memory
 *------------------------------------------------------------------------
 */

void * memset(void * s, int c, int n)
{
    register int i;
    char * cp = (char *)s;

    for (i = 0; i < n; i++){
        *cp = (unsigned char)c;
        cp++;
    }
    return s;
}