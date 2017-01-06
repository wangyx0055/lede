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
    rbus_main_end.c

    Abstract:
    Create and register network interface for RBUS based chipsets in VxWorks platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include "rt_config.h"
#include "mt7628End.h"

typedef struct _WLAN_DRV_CTRL
{
	DRV_CTRL	drvCtrl;
} WLAN_DRV_CTRL;

#define MAX_WLAN_UNITS 4
#define WLAN_DEV_NAME "ra"

WLAN_DRV_CTRL apCliDryCtrl = {0};

WLAN_DRV_CTRL g_wlan_drvCtrl[MAX_WLAN_UNITS] = {0};	/* use for eth drv_ctl */

SEM_ID VX_Wireless_LOCK;


#ifdef RTMP_RBUS_SUPPORT
END_OBJ* RtmpVxRbusEndLoad(
	IN RTMP_VX_INIT_STRING *initToken)
{
	END_OBJ *pENDDev;
	ULONG						csr_addr;
	int							status;
	POS_COOKIE					handle = NULL;
	RTMP_ADAPTER				*pAd;
	/*unsigned int				dev_irq; */
	/*RTMP_OS_NETDEV_OP_HOOK	netDevHook; */
	DRV_CTRL 	*pDrvCtrl;
	int unitNum = 0;
	UINT32 Value;
	UCHAR mac[MAC_ADDR_SIZE] = {0x2, 0x3, 0x4, 0x5, 0x7, 0x8};
	int i;	
	struct mt_dev_priv	*priv_info = NULL;
	
	DBGPRINT(RT_DEBUG_TRACE, ("===> RtmpVxRbusEndLoad()\n"));

	VX_Wireless_LOCK = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
/*RtmpRaBusInit============================================ */
	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long) RTMP_MAC_CSR_ADDR;
	

/*RtmpDevInit============================================== */
	/* Allocate RTMP_ADAPTER adapter structure */
	handle = (POS_COOKIE)malloc(sizeof(struct os_cookie));
	if (!handle)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate memory for os_cookie failed!\n"));
		goto err_out;
	} else
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate memory for os_cookie success!\n"));
	
	memset((PUCHAR)handle, 0, sizeof(struct os_cookie));

	status = RTMPAllocAdapterBlock((PVOID)handle, &pAd);
	pAd->CSRBaseAddress = (PUCHAR)csr_addr;
	if (status != NDIS_STATUS_SUCCESS)
	{
		free(handle);
		goto err_out;
	} 
	else
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate memory for AdapterBlock success!(0x%x)\n",pAd));	

#ifdef MT7628
	pAd->VirtualIfCnt =0;
	pAd->MACVersion = 0x76280000;
	pAd->chipCap.hif_type = HIF_MT;
	pAd->infType = RTMP_DEV_INF_RBUS;

	    RTMP_IO_READ32(pAd, TOP_HVR, &Value);
	    pAd->HWVersion = Value;
	DBGPRINT(RT_DEBUG_ERROR, ("pAd->HWVersion = 0x%x\n",pAd->HWVersion));	
	

	    RTMP_IO_READ32(pAd, TOP_FVR, &Value);
	    pAd->FWVersion = Value;
	DBGPRINT(RT_DEBUG_ERROR, ("pAd->FWVersion = 0x%x\n",pAd->FWVersion));	

	    RTMP_IO_READ32(pAd, TOP_HCR, &Value);
	    pAd->ChipID = Value;
	DBGPRINT(RT_DEBUG_ERROR, ("pAd->ChipID = 0x%x\n",pAd->ChipID));	


	    if (IS_MT7628(pAd))
	    {
	        RTMP_IO_READ32(pAd, STRAP_STA, &Value);
	        pAd->AntMode = (Value >> 24) & 0x1;
	    }
#endif
	
	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_RBUS);
	handle->unitNum = initToken->phyNum;
	/* Here are the RTMP_ADAPTER structure with rbus-bus specific parameters. */
	
	if (rtmp_cfg_init(pAd, NULL) != TRUE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("rtmp cfg init failed!\n"));
		rtmp_cfg_exit(pAd);
	}	
	
	pDrvCtrl = (DRV_CTRL *)(&g_wlan_drvCtrl[unitNum]);
	bzero((char *)pDrvCtrl, sizeof(WLAN_DRV_CTRL)); 

      pDrvCtrl->unit = handle->unitNum;
	
    sprintf(pDrvCtrl->name, "%s", WLAN_DEV_NAME);
    sprintf(pDrvCtrl->fullName, "%s%d", WLAN_DEV_NAME, unitNum);
/*NetDevInit============================================== */
	/*it's time to register the net device to vxworks kernel. */
	//pENDDev = (END_OBJ *)malloc(sizeof(END_OBJ));
	pENDDev = (END_OBJ *)&pDrvCtrl->endObj;
	if (pENDDev == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, (" malloc failed for main physical net device!\n"));
		goto err_out_free_radev;
	} 
	else
		DBGPRINT(RT_DEBUG_TRACE, (" malloc for main physical net device success!\n"));

	memset((PUCHAR)pENDDev, 0, sizeof(END_OBJ));

	
	status = RtmpVxNetPoolInit(handle);
	if (status == NDIS_STATUS_FAILURE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate NetPool failed!\n"));
		goto err_out_free_netdev;
	}
	/* We only assign the Tx pool for netdevice due to we use the Rx pool internal. */
	/* but rx pool shows more info, so we change to use rx */
	pENDDev->pNetPool = handle->pNetPool[RTMP_NETPOOL_RX];


#ifdef CONFIG_AP_SUPPORT
	//pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = pENDDev;
#endif /* CONFIG_AP_SUPPORT */
	pDrvCtrl->loaded = TRUE;   
    pDrvCtrl->started  = FALSE;

	if (OK != macGet(mac, MAC_ADDR_SIZE, OS_LAN_UNIT))
	{
		DBG_ERR("lac mac get failure\n");
	}

	if ((END_OBJ_INIT(pENDDev, (DEV_OBJ *)pAd, &initToken->devName[0],
	              initToken->phyNum, &RtmpMAINEndFuncTable, "ra0") == ERROR)) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not init net device %s!\n", "ra0"));
		goto err_out_free_netpool;
	}

	if ((END_MIB_INIT(pENDDev, M2_ifType_ethernet_csmacd,
	              mac, 6, ETHERMTU, END_SPEED) == ERROR))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not init MIB of net device %s!\n", "ra0"));
		goto err_out_free_netpool;
	}

	bcopy ((char *)pDrvCtrl->endObj.mib2Tbl.ifPhysAddress.phyAddress, 
		(char *)pDrvCtrl->enetAddr, pDrvCtrl->endObj.mib2Tbl.ifPhysAddress.addrLength);
	/*END_OBJ_READY(pENDDev, IFF_NOTRAILERS |IFF_SIMPLEX |IFF_BROADCAST |IFF_ALLMULTI); */
	END_OBJ_READY(pENDDev, IFF_NOTRAILERS |IFF_BROADCAST |IFF_MULTICAST);

	/*pDrvCtrl->ifType = IF_TYPE_WLAN;*/
 	/*pDrvCtrl->pOwner = NULL;*/		/* not belong to any bridge or bridge 
									by default */

	priv_info = (struct mt_dev_priv *) kmalloc(sizeof(struct mt_dev_priv), GFP_ATOMIC);
	((pENDDev)->devObject.pDevice)= priv_info;	
	//RTMP_OS_NETDEV_SET_PRIV(pENDDev, priv_info);
	RTMP_OS_NETDEV_SET_PRIV(pENDDev, pAd);
	pAd->net_dev = pENDDev;


	/* double-check if pAd is associated with the net_dev */
	if (RTMP_OS_NETDEV_GET_PRIV(pENDDev) == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, (" RTMP_OS_NETDEV_GET_PRIV failed free pENDDev\n"));
		RtmpOSNetDevFree(pENDDev);
		goto err_out_free_netdev;
	} else
		DBGPRINT(RT_DEBUG_TRACE, (" RTMP_OS_NETDEV_GET_PRIV (0x%x)\n",RTMP_OS_NETDEV_GET_PRIV(pENDDev)));

	//DBGPRINT(RT_DEBUG_TRACE, (" RTMP_DRIVER_NET_DEV_SET (pAd=0x%x)\n",pAd));
	RTMP_DRIVER_NET_DEV_SET(pAd, pENDDev);
	//DBGPRINT(RT_DEBUG_TRACE, (" RTMP_DRIVER_NET_DEV_SET (pAd=0x%x)\n",pAd));
	DBGPRINT(RT_DEBUG_TRACE, (" RTMP_OS_NETDEV_GET_PRIV (0x%x)\n",RTMP_OS_NETDEV_GET_PRIV(pENDDev)));
		
#if 0
#ifdef CONFIG_AP_SUPPORT
    if (pAd->OpMode == OPMODE_AP)
	{
        	BSS_STRUCT *pMbss;
		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		if (pMbss) {
			MTWF_LOG(DBG_CAT_ALL, DBG_LVL_ERROR, ("%s():enter wdev_bcn_buf_init !\n", __FUNCTION__));
			wdev_bcn_buf_init(pAd, &pMbss->bcn_buf);
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_LVL_ERROR, ("%s():func_dev is NULL!\n", __FUNCTION__));
			return NULL;
		}
	}
#endif
#endif
	
	DBGPRINT(RT_DEBUG_TRACE, ("RtmpVxRbusEndLoad(): Create END Object(%s) success with CSR addr(0x%lx)\n", 
								pENDDev->devObject.name, (ULONG)csr_addr));

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2880_probe\n"));
	DBGPRINT(RT_DEBUG_TRACE, (" RTMP_OS_NETDEV_GET_PRIV (0x%x)\n",RTMP_OS_NETDEV_GET_PRIV(pENDDev)));
	//RTMP_OS_NETDEV_SET_PRIV(pENDDev, pAd);
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


VOID rt2880_END_Unload(END_OBJ *endObj)
{
	RTMP_ADAPTER *pAd;
	BOOLEAN  Cancelled;

	if (endObj == NULL)
		return;
	
	pAd = endObj->devObject.pDevice;
	if (pAd != NULL)
	{
#if defined (AP_LED) || defined (STA_LED)
#ifdef CONFIG_SWMCU_SUPPORT
	{
		PSWMCU_LED_CONTROL pSWMCULedCntl = &pAd->LedCntl.SWMCULedCntl;
		pSWMCULedCntl->bWlanLed = TRUE;
		RTMPCancelTimer(&pSWMCULedCntl->LedCheckTimer, &Cancelled);

	}
#endif
#endif /* (AP_LED) || (STA_LED) */

		RtmpPhyNetDevExit(pAd, endObj);

		RtmpRaDevCtrlExit(pAd);
	}
	else
	{
		RtmpOSNetDevDetach(endObj);
	}
	
	/* Free the root net_device. */
	RtmpOSNetDevFree(endObj);
	
}
#endif
