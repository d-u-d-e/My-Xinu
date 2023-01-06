#include <ether.h>
#include <am335x_eth.h>
#include <delay.h>

struct ethcblk ethertab[1];

struct eth_a_csreg eth_a_regs;

static int32 eth_phy_reset(volatile struct eth_a_mdio * mdio, byte phyadr);
static int32 eth_phy_read(volatile struct eth_a_mdio * mdio, byte regadr, byte phyadr, uint32 * value);
static int32 eth_phy_write(volatile struct eth_a_mdio * mdio, byte regadr, byte phyadr, uint32 value);

/*-----------------------------------------------------------------------
 * ethinit - initialize the TI AM335X ethernet hardware
 *-----------------------------------------------------------------------
 */

devcall ethinit(struct dentry * devptr)
{
    /* Get the Ethernet control block address	*/
	/* from the device table entry	*/

    struct ethcblk * ethptr = &ethertab[devptr->dvminor];

    /* Store the address of CSRs in the Ethernet control block	*/

    struct eth_a_csreg * csrptr = &eth_a_regs;
    ethptr->csr = csrptr;

    /* Initialize the addresses of all the submodules */

    csrptr->ale = (struct eth_a_ale *)ETH_AM335X_ALE_ADDR;
    csrptr->cpdma = (struct eth_a_cpdma *)ETH_AM335X_CPDMA_ADDR;
    csrptr->sl = (struct eth_a_sl *)ETH_AM335X_SL1_ADDR;
    csrptr->stateram = (struct eth_a_stateram *)ETH_AM335X_STATERAM_ADDR;
    csrptr->ss = (struct eth_a_ss *)ETH_AM335X_SS_ADDR;
    csrptr->wr = (struct eth_a_wr *)ETH_AM335X_WR_ADDR;
    csrptr->mdio = (struct eth_a_mdio *)ETH_AM335X_MDIO_ADDR;

    /* Reset all the submodules */

    csrptr->cpdma->reset = 1;
    while(csrptr->cpdma->reset == 1);

    csrptr->sl->reset = 1;
    while(csrptr->sl->reset == 1);

    csrptr->wr->reset = 1;
	while(csrptr->wr->reset == 1) ;

	csrptr->ss->reset = 1;
	while(csrptr->ss->reset == 1) ;

    /* Enable MDIO	*/

    csrptr->mdio->ctrl |= ETH_AM335X_MDIOCTL_EN;

    /* Reset the PHY */

    int32 retval;
    retval = eth_phy_reset(csrptr->mdio, 0);
    if (retval == SYSERR){
        kprintf("Cannot reset Ethernet PHY\n");
        return SYSERR;
    }

    uint32 phyreg;
    retval = eth_phy_read(csrptr->mdio, ETH_PHY_CTLREG, 0, &phyreg);
    if (retval == SYSERR){
        return SYSERR;
    }

    if ((phyreg & ETH_PHY_CTLREG_SM) == ETH_PHY_10M){
        kprintf("Ethernet Link is Up. Speed is 10Mbps\n");
    }
    else if ((phyreg & ETH_PHY_CTLREG_SM) == ETH_PHY_100M){
        kprintf("Ethernet Link is Up. Speed is 100Mbps\n");
    }
    else if ((phyreg & ETH_PHY_CTLREG_SM) == ETH_PHY_1000M){
        kprintf("Ethernet Link is Up. Speed is 1000Mbps\n");
    }
    else{
        return SYSERR;
    }

	if(phyreg & ETH_PHY_CTLREG_FD) {
		kprintf("Link is Full Duplex\n");
		csrptr->sl->macctrl |= ETH_AM335X_SLCTL_FD;
	}
	else {
		kprintf("Link is Half Duplex\n");
	}

    /* Read the device MAC address */

    int32 i;
    for (i = 0; i < 2; i++){
        ethptr->devAddress[4 + i] = *((byte*)(0x44E10630 + i)); /* bytes 4 and 5 */
    }

    for(i = 0; i < 4; i++) {
		ethptr->devAddress[i] = *((byte *)(0x44E10634 + i)); /* bytes 0, 1, 2, 3 */
	}

    kprintf("MAC Address is: ");
    for(i = 0; i < 5; i++) {
		kprintf("%02X:", ethptr->devAddress[i]);
	}
    kprintf("%02X\n", ethptr->devAddress[5]);

    /* Initialize the rx ring size field */

}

/*-----------------------------------------------------------------------
 * eth_phy_reset - Reset an Ethernet PHY
 *-----------------------------------------------------------------------
 */

int32 eth_phy_reset(volatile struct eth_a_mdio * mdio, byte phyadr)
{
    uint32	phyreg;	/* Variable to hold ETH PHY register value	*/
    int32 retval = eth_phy_read(mdio, ETH_PHY_CTLREG, phyadr, &phyreg);
    if (retval == SYSERR){
        return SYSERR;
    }

    /* Set the Reset bit and write the register */

    phyreg |= ETH_PHY_CTLREG_RESET;
    eth_phy_write(mdio, ETH_PHY_CTLREG, phyadr, phyreg);

    /* Check if Reset operation is complete */

    int32 retries;

    for (retries = 0; retries < 10; retries++){
        if(eth_phy_read(mdio, ETH_PHY_CTLREG, phyadr, &phyreg) == SYSERR){
            return SYSERR;
        }
        if ((phyreg & ETH_PHY_CTLREG_RESET) == 0){
            break;
        }
        else{
            retries++;
            DELAY(ETH_AM335X_INIT_DELAY);
            continue;
        }
    }
    if(retries >= 3){
        return SYSERR;
    }
    /* Check if the Link is established */

    for (retries = 0; retries < 10; retries++){
        if(eth_phy_read(mdio, ETH_PHY_STATREG, phyadr, &phyreg) == SYSERR){
            return SYSERR;
        }
        if ((phyreg & ETH_PHY_STATREG_LINK)){ /* link is up */
            break;
        }
        else{
            retries++;
            DELAY(ETH_AM335X_INIT_DELAY);
            continue;
        }
    }
    if(retries >= 3){
        return SYSERR;
    }
    return OK;
}

/*-----------------------------------------------------------------------
 * eth_phy_read - read an Ethernet PHY register
 *-----------------------------------------------------------------------
 */

int32 eth_phy_read(volatile struct eth_a_mdio * mdio, byte regadr, byte phyadr, uint32 * value)
{   
    /* Ethernet PHY has only 32 registers and only 32 possible PHY addresses */

    if (regadr > 31 || phyadr > 31){
        return SYSERR;
    }

    /* Wait for the previous access to complete */

    while ((mdio->useraccess0 & ETH_AM335X_MDIOUA_GO) != 0);

    /* Start the access */

    mdio->useraccess0 = (ETH_AM335X_MDIOUA_GO) | (regadr << 21) | (phyadr << 16);

    /* Check if the access was successful */

    if ((mdio->useraccess0 & ETH_AM335X_MDIOUA_ACK) == 0){
        return SYSERR;
    }

    /* Copy the value read */

    (*value) = mdio->useraccess0 & ETH_AM335X_MDIOUA_DM;
    return OK;
}

/*-----------------------------------------------------------------------
 * eth_phy_write - write an Ethernet PHY register
 *-----------------------------------------------------------------------
 */

int32 eth_phy_write(volatile struct eth_a_mdio * mdio, byte regadr, byte phyadr, uint32 value)
{   
    /* Ethernet PHY has only 32 registers and only 32 possible PHY addresses */

    if (regadr > 31 || phyadr > 31){
        return SYSERR;
    }

    /* Wait for the previous access to complete */

    while ((mdio->useraccess0 & ETH_AM335X_MDIOUA_GO) != 0);

    /* Start the access */

    mdio->useraccess0 = (ETH_AM335X_MDIOUA_GO) | (ETH_AM335X_MDIOUA_WR) | 
        (regadr << 21) | (phyadr << 16) | (value & 0xFFFF);


    /* Wait for the access to complete */

    while((mdio->useraccess0 & ETH_AM335X_MDIOUA_GO) != 0);
    return OK;
}