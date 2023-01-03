#include <lib.h>
#include <lfs.h>
#include <conf.h>
#include <process.h>

extern umsg32 recvclr(void);
extern process shell(did32 dev);

process main(void)
{
    pid32 shpid; /* Shell process ID */
    
    printf("\n\n");

    /* Create a local file system on the RAM disk */

    lfscreate(RAM0, 40, 20480); // 20480/512 = 40 sectors in disk (20 MiB); 6 disk sectors used for i-blocks
    /* with 40 i-blocks we could address 40 * 8192 bytes = 320 MiB of data, however we have only 20 MiB of space */

    /* Run the Xinu shell */
    
    recvclr();
    resume(shpid = create(shell, 8192, 50, "shell", 1, CONSOLE));

    /* Wait for shell to exit and recreate it */

    while(TRUE){
    
        ; //TODO
    }
}