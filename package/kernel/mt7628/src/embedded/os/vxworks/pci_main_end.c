/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    2860_main_end.c

    Abstract:
    Create and register network interface for PCI based chipsets in VxWorks platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include "rt_config.h"
#include "mt7628End.h"
/*#include "ifMgr/ifMgr_ifCtl.h"*/





/* */
/* Ralink 28xx based PCI device table, include all supported chipsets */
/* */


/* */
/* Our PCI driver structure */
/* */


/***************************************************************************
 *
 *	PCI device initialization related procedures.
 *
 ***************************************************************************/
static INT  rt2860_init_module(VOID)
{
	return 0;
}


/* */
/* Driver module unload function */
/* */
static VOID  rt2860_cleanup_module(VOID)
{
	return;
}


/* */
/* PCI device probe & initialization function */
/* */
static INT rt2860_probe()
{
	return 0;
}



static VOID  rt2860_remove_one()
{
	return;
}

#if 0
/*
========================================================================
Routine Description:
    Check the chipset vendor/product ID.

Arguments:
    _dev_p				Point to the PCI or USB device

Return Value:
    TRUE				Check ok
	FALSE				Check fail

Note:
========================================================================
*/
BOOLEAN RT28XXChipsetCheck(
	IN void *_dev_p)
{
	/* always TRUE */
	return TRUE;
}
#endif /* Unused */

typedef struct _WLAN_DRV_CTRL
{
	DRV_CTRL	drvCtrl;
} WLAN_DRV_CTRL;
#if 0
#define MAX_WLAN_UNITS 4
#define WLAN_DEV_NAME "ra"

IMPORT LIST	g_ifMgr_freeIfList; 

WLAN_DRV_CTRL apCliDryCtrl = {0};

WLAN_DRV_CTRL g_wlan_drvCtrl[MAX_WLAN_UNITS] = {0};	/* use for eth drv_ctl */

END_OBJ* RtmpVxPciEndLoad( /* RT2880 PCI */
	IN RTMP_VX_INIT_STRING *initToken)
{
	END_OBJ *pENDDev;
	ULONG						csr_addr;
	int							status;
	POS_COOKIE					handle = NULL;
	RTMP_ADAPTER				*pAd;
	/*unsigned int				dev_irq; */
	/*RTMP_OS_NETDEV_OP_HOOK	netDevHook; */

	int pci_vendor;

	DRV_CTRL 	*pDrvCtrl;
	int unitNum = 0;

	UCHAR *macAddr;
	int i;


	DBGPRINT(RT_DEBUG_TRACE, ("===> RtmpVxPciEndLoad()\n"));

	pci_vendor = pcie_read_config_word(0x0, 0x1, 0x0, 0x0);

	printk("vendor = 0x%0x\n", pci_vendor);

	
/*RtmpRaBusInit============================================ */
	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long) initToken->csrAddr;
	

/*RtmpDevInit============================================== */
	/* Allocate RTMP_ADAPTER adapter structure */
	handle = (POS_COOKIE)malloc(sizeof(struct os_cookie));
	if (!handle)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate memory for os_cookie failed!\n"));
		goto err_out;
	}
	memset((PUCHAR)handle, 0, sizeof(struct os_cookie));
	
	status = RTMPAllocAdapterBlock((PVOID)handle, &pAd);
	if (status != NDIS_STATUS_SUCCESS)
	{
		free(handle);
		goto err_out;
	}

	/* Here are the RTMP_ADAPTER structure with rbus-bus specific parameters. */
	pAd->CSRBaseAddress = (PUCHAR)(0xbf700000);
	pAd->irq_num = initToken->irqNum;

	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_PCI);
	handle->unitNum = initToken->phyNum;

	pDrvCtrl = (DRV_CTRL *)(&g_wlan_drvCtrl[unitNum]);
	bzero((char *)pDrvCtrl, sizeof(WLAN_DRV_CTRL)); 

    pDrvCtrl->unit = handle->unitNum;
	
    sprintf(pDrvCtrl->name, "%s", WLAN_DEV_NAME);
    sprintf(pDrvCtrl->fullName, "%s%d", WLAN_DEV_NAME, unitNum);
	

/*NetDevInit============================================== */
	/*it's time to register the net device to vxworks kernel. */
#if 0
	pENDDev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (pENDDev == NULL)
	{
		printf("RtmpOSNetDevCreate() failed for main physical net device!\n");
		goto err_out_free_radev;
	}
#else
	//pENDDev = (END_OBJ *)malloc(sizeof(END_OBJ));
	pENDDev = (END_OBJ *)&pDrvCtrl->endObj;
	if (pENDDev == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, (" malloc failed for main physical net device!\n"));
		goto err_out_free_radev;
	}
	memset((PUCHAR)pENDDev, 0, sizeof(END_OBJ));
	pAd->net_dev = pENDDev;
	handle->pci_dev = pENDDev;
	status = RtmpVxNetPoolInit(handle);
	if (status == NDIS_STATUS_FAILURE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate NetPool failed!\n"));
		goto err_out_free_netdev;
	}
	/* We only assign the Tx pool for netdevice due to we use the Rx pool internal. */
	pENDDev->pNetPool = handle->pNetPool[RTMP_NETPOOL_TX];
	
#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = pENDDev;
#endif /* CONFIG_AP_SUPPORT */
	
    //pDrvCtrl->endObj.pNetPool = pENDDev->pNetPool;
	/*pDrvCtrl->attached = TRUE;   */
	pDrvCtrl->loaded = TRUE;   
    pDrvCtrl->started  = FALSE;

	if ((END_OBJ_INIT(pENDDev, (DEV_OBJ *)pAd, &initToken->devName[0],
	              initToken->phyNum, &RtmpMAINEndFuncTable, "ra1") == ERROR)) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not init net device %s!\n", "ra1"));
		goto err_out_free_netpool;
	}

	if ((END_MIB_INIT(pENDDev, M2_ifType_ethernet_csmacd,
	              &ZERO_MAC_ADDR[0],  6, ETHERMTU, END_SPEED) == ERROR))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not init MIB of net device %s!\n", "ra1"));
		goto err_out_free_netpool;
	}

	bcopy ((char *)pDrvCtrl->endObj.mib2Tbl.ifPhysAddress.phyAddress, 
		(char *)pDrvCtrl->enetAddr, pDrvCtrl->endObj.mib2Tbl.ifPhysAddress.addrLength);

	/*END_OBJ_READY(pENDDev, IFF_NOTRAILERS |IFF_SIMPLEX |IFF_BROADCAST |IFF_ALLMULTI); */
	END_OBJ_READY(pENDDev, IFF_NOTRAILERS |IFF_BROADCAST |IFF_MULTICAST);

	/* 
	 * filling pDrvCtrl for later interface management.
	 */
	pDrvCtrl->ifType = IF_TYPE_WLAN;
 	pDrvCtrl->pOwner = NULL;		/* not belong to any bridge or bridge 
									by default */
	/* 
	 * add to g_ifCtl_freeIfList by default 
	 */

									
	lstAdd(&g_ifMgr_freeIfList, &pDrvCtrl->node);	

	DBGPRINT(RT_DEBUG_TRACE, ("RtmpVxRbusEndLoad(): Allocate END object success!\n"));

#endif

	DBGPRINT(RT_DEBUG_TRACE, ("%s: at CSR addr 0x%1x\n", pENDDev->devObject.name, (ULONG)csr_addr));

	printk("%s: at CSR addr 0x%1x\n", pENDDev->devObject.name, (ULONG)csr_addr);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2880_probe\n"));

	return pENDDev;

err_out_free_netpool:
	RtmpVxNetPoolFree(handle);
	
err_out_free_netdev:
	if (pENDDev)
	RtmpOSNetDevFree(pENDDev);

err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	if (pAd)
	RTMPFreeAdapter(pAd);
		
err_out:
	return NULL;
}
#endif
