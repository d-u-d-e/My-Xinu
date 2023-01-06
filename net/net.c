#include <net.h>
#include <lib.h>

/*------------------------------------------------------------------------
 * net_init  -  Initialize network data structures and processes
 *------------------------------------------------------------------------
 */

struct network NetData;
ibid32 netbufpool;
uint32 netportseed;

/*------------------------------------------------------------------------
 * net_init  -  Initialize network data structures and processes
 *------------------------------------------------------------------------
 */

void net_init(void)
{
    int32 nbufs; /* Total no of buffers	*/

    memset(&NetData, NULLCH, sizeof(struct network));

    /* Obtain the Ethernet MAC address */

    control(ETHER0, ETH_CTRL_GET_MAC, (int32)NetData.ethucast, 0);
    memset((char *)NetData.ethbcast, 0xFF, ETH_ADDR_LEN);

    /* Initialize the random port seed */



}