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
    rt_main_end.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
   Shiang		Aug/21/08		Initial version for VxWorks
*/

#include "rt_config.h"
//#include "platform.h"
#include "ralink.h"

#include "taskLib.h"
#include "arch/mips/ivMips.h"

#include "mt7628End.h"

#ifdef MBLK_DEBUG
#include "cmd.h"
#endif

extern int netTaskId;

#define MAX_WLAN_UNITS 4

typedef struct _WLAN_DRV_CTRL
{
	DRV_CTRL	drvCtrl;
} WLAN_DRV_CTRL;

extern WLAN_DRV_CTRL g_wlan_drvCtrl[MAX_WLAN_UNITS];
extern NET_POOL_ID _pNetDpool;

/*
 * Declare our function table.  
 */


int MainVirtualIF_open(
	IN PNET_DEV net_dev);

int MainVirtualIF_close(
	IN PNET_DEV net_dev);

INT rt28xx_ioctl(
	IN	PNET_DEV	endDev, 
	IN	int			cmd, 
	IN	caddr_t		data);

static int rt28xx_send_packets(
	IN END_OBJ * pEndObj,
	IN M_BLK_ID     pMblk);

static inline M_BLK_ID RtmpVxNetPktCheck(
	IN END_OBJ * pEndObj,
	IN M_BLK_ID     pMblk);
	

STATUS RtmpVxNetDevMCastAdd(
	IN END_OBJ	*pEndObj,
	IN char		*pAddr);
	
STATUS RtmpVxNetDevMCastDel(
	IN END_OBJ	*pEndObj,
	IN char		*pAddr);

STATUS RtmpVxNetDevMCastGet(
	IN END_OBJ *pEndObj,
	IN MULTI_TABLE* pTable);


NET_FUNCS RtmpMAINEndFuncTable = {
    (FUNCPTR)MainVirtualIF_open,		/* STATUS (*start) (END_OBJ*); start the device. */
    (FUNCPTR)MainVirtualIF_close,		/* STATUS (*stop) (END_OBJ*); stop the device. */
    (FUNCPTR)sysRtmpEndUnLoad,		/* STATUS (*unload) (END_OBJ*); Unloading function for the driver.*/
    (FUNCPTR)rt28xx_ioctl,				/* int (*ioctl) (END_OBJ*, int, caddr_t); Ioctl function for the driver. */
    (FUNCPTR)rt28xx_send_packets,	/* STATUS (*send) (END_OBJ* , M_BLK_ID);Send function for the driver. */
    (FUNCPTR)RtmpVxNetDevMCastAdd,	/* STATUS (*mCastAddrAdd) (END_OBJ*, char*);Multicast add function.*/
    (FUNCPTR)RtmpVxNetDevMCastDel,	/* STATUS (*mCastAddrDel) (END_OBJ*, char*);Multicast delete function.  */
    (FUNCPTR)RtmpVxNetDevMCastGet,	/* STATUS (*mCastAddrGet) (END_OBJ*, MULTI_TABLE*);Multicast retrieve function . */
    NULL,								/* STATUS (*pollSend) (END_OBJ*, M_BLK_ID);Polling send function */
    NULL, 								/* STATUS (*pollRcv) (END_OBJ*, M_BLK_ID);Polling receive function */
    endEtherAddressForm,		/* M_BLK_ID (*formAddress) (); put address info into a NET_BUFFER */
    endEtherPacketDataGet,		/* STATUS (*packetDataGet) ();get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet,		/* STATUS (*addrGet) (M_BLK_ID, M_BLK_ID, M_BLK_ID, M_BLK_ID, M_BLK_ID); Get packet addresses. */
    NULL,								/* int (*endBind) (void*, void*, void*, long type); information exchange between */
									/* network service and network driver */
};

#ifdef MBLK_DEBUG
static M_CL_CONFIG	wireless_MclBlkConfig[RTMP_NETPOOL_CNT];

static char cmdWirelessPacketShowHelp[] = 
"# packet -show cblk | mblk \r\n"
"# \r\n"
"# -show          Show mblk chain or cblk chain.\r\n"
"# \r\n"
"# cblk           Cblk chain.\r\n"
"# mblk           Mblk chain.\r\n"
"# \r\n"
;
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
#ifdef VXWORKS
int MBSS_PacketSend(
	IN	PNDIS_PACKET				pPktSrc, 
	IN	PNET_DEV					pDev,
	IN	RTMP_NET_PACKET_TRANSMIT	Func)
{
    RTMP_ADAPTER     *pAd;
    BSS_STRUCT *pMbss;
    PNDIS_PACKET     pPkt = (PNDIS_PACKET)pPktSrc;
    INT              IdBss;
    int idx; 

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);

#if 1 //yiwei debug tx
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%d: MBSS_PacketSend() ==> pDev=%p\n",__LINE__,pDev));
#endif

#ifdef RALINK_ATE
    if (ATE_ON(pAd))
    {
        RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_FAILURE);
        return 0;
    } /* End of if */
#endif /* RALINK_ATE */

	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))          ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
	{
		/* wlan is scanning/disabled/reset */
		RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_FAILURE);
		return 0;
	} /* End of if */


    /* 0 is main BSS, dont handle it here */
    /* FIRST_MBSSID = 1 */
    pMbss = pAd->ApCfg.MBSSID;


	 idx = Get_Wdev_idx_From_EndObject(pDev);
	 if (idx == -1)
	{
	        RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_FAILURE);
	        return 0;
   	 }
   	 {
        /* find the device in our MBSS list */
		{
/*			NdisZeroMemory((PUCHAR)&(RTPKT_TO_OSPKT(pPktSrc))->cb[CB_OFF], 15); */
			NdisZeroMemory((PUCHAR)(GET_OS_PKT_CB(pPktSrc) + CB_OFF), 15);
           		//RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPktSrc, IdBss);
			SET_OS_PKT_NETDEV(pPktSrc, pDev); 
		 

            /* transmit the packet */
           		 return Func(pPktSrc, idx);
		}
	}

    /* can not find the BSS so discard the packet */
	RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_FAILURE);

    return 0;
} /* End of MBSS_PacketSend */

INT MBSS_VirtualIF_PacketSend(
	IN PNDIS_PACKET			pPktSrc, 
	IN PNET_DEV				pDev)
{

	MEM_DBG_PKT_ALLOC_INC(pPktSrc);

#if 1 //yiwei debug tx
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%d: MBSS_VirtualIF_PacketSend() ==> pDev=%p\n",__LINE__,pDev));
#endif

	if(!(RTMP_OS_NETDEV_STATE_RUNNING(pDev)))
	{
		/* the interface is down */
		RELEASE_NDIS_PACKET(NULL, pPktSrc, NDIS_STATUS_FAILURE);
		return 0;
	} /* End of if */

	return MBSS_PacketSend(pPktSrc, pDev, rt28xx_packet_xmit);
} /* End of MBSS_VirtualIF_PacketSend */


/*
========================================================================
Routine Description:
    IOCTL to WLAN.

Arguments:
	pDev			which WLAN network interface
	pIoCtrl			command information
	Command			command ID

Return Value:
    0: IOCTL successfully
    otherwise: IOCTL fail

Note:
    SIOCETHTOOL     8946    New drivers use this ETHTOOL interface to
                            report link failure activity.
========================================================================
*/
INT MBSS_VirtualIF_Ioctl(
	IN PNET_DEV				pDev, 
	IN OUT VOID 			*pIoCtrl, 
	IN INT 					Command)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);

	if (!pAd)
		return -EINVAL;

/*	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE)) */
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS)
		return -ENETDOWN;
	/* End of if */

	/* do real IOCTL */
	return rt28xx_ioctl(pDev, pIoCtrl, Command);
} /* End of MBSS_VirtualIF_Ioctl */
#endif /*VXWORKS*/

/*
========================================================================
Routine Description:
    Send a packet to WLAN via MBSS interface.

Arguments:
    pEndObj	points to our adapter
    pMblk		packet buffer need to delivery

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
static int RtmpOSNetDevMBSSSend(
	IN END_OBJ		*pEndObj,
	IN M_BLK_ID     pMblk)
{
	M_BLK_ID	pNewMblk;
	NDIS_STATUS		status = NDIS_STATUS_FAILURE;
	STATUS retVal;
	RTMP_ADAPTER *pAd = NULL;
	int idx = -1; 

	 pAd = RTMP_OS_NETDEV_GET_PRIV(pEndObj);
	 idx = ENDOBJ_GET_UNIT(pEndObj);

	 if (idx == -1)
	 {
	 	RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pMblk, NDIS_STATUS_FAILURE);
	 	return status;
	 }

	VX_WIRELESS_LOCK(); 
	pNewMblk = RtmpVxNetPktCheck(pEndObj, pMblk);

	if (pNewMblk)
	{
		if (taskIdSelf() != netTaskId )
		{
	       retVal = netJobAdd(MBSS_VirtualIF_PacketSend, pNewMblk, (PNET_DEV)pEndObj, 0, 0, 0);

		   if (retVal != OK)	
		   {
		   		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pNewMblk, NDIS_STATUS_FAILURE);
#ifdef DMA_DEBUG
		   		pAd->packet_netjobadd_fail_count++;
#endif /* DMA_DEBUG */
		   }
		   status=0;
	    }
		else
		{
			status = MBSS_VirtualIF_PacketSend((PNDIS_PACKET)pNewMblk, (PNET_DEV)pEndObj);
		}
		
	}
	else
	{
		status = 0;
	}

	VX_WIRELESS_UNLOCK();
	return status;
}

NET_FUNCS RtmpMBSSEndFuncTable = {
    (FUNCPTR)MBSS_VirtualIF_Open,        /* Function to start the device. */
    (FUNCPTR)MBSS_VirtualIF_Close,         /* Function to stop the device. */
    (FUNCPTR)sysRtmpEndUnLoad,       /* Unloading function for the driver.*/
    (FUNCPTR)rt28xx_ioctl,       /* Ioctl function for the driver. */
    (FUNCPTR)RtmpOSNetDevMBSSSend,         /* Send function for the driver. */
    (FUNCPTR)RtmpVxNetDevMCastAdd,     /* Multicast add function.*/
    (FUNCPTR)RtmpVxNetDevMCastDel,     /* Multicast delete function.  */
    (FUNCPTR)RtmpVxNetDevMCastGet,     /* Multicast retrieve function . */
    NULL,     /* Polling send function */
    NULL,     /* Polling receive function */
    endEtherAddressForm,            /* put address info into a NET_BUFFER */
    endEtherPacketDataGet,              /* get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet,	 	/* Get packet addresses. */
    NULL                            /* Bind */
};
#endif /* MBSS_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef VXWORKS
int APC_PacketSend(
	IN	PNDIS_PACKET				skb_p, 
	IN	PNET_DEV					dev_p,
	IN	RTMP_NET_PACKET_TRANSMIT	Func)
{
	RTMP_ADAPTER *pAd;
	PAPCLI_STRUCT pApCli;
	INT apcliIndex;



	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}
#endif /* RALINK_ATE */

	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))          ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
	{
		/* wlan is scanning/disabled/reset */
		RELEASE_NDIS_PACKET(pAd, skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}


	pApCli = (PAPCLI_STRUCT)&pAd->ApCfg.ApCliTab;

	for(apcliIndex = 0; apcliIndex < MAX_APCLI_NUM; apcliIndex++)
	{
		if (pApCli[apcliIndex].Valid != TRUE)
			continue;

		/* find the device in our ApCli list */
		if (pApCli[apcliIndex].wdev.if_dev== dev_p)
		{
			/* ya! find it */
			pAd->RalinkCounters.PendingNdisPacketCount ++;
			//RTMP_SET_PACKET_SOURCE(skb_p, PKTSRC_NDIS);
			RTMP_SET_PACKET_MOREDATA(skb_p, FALSE);
			//RTMP_SET_PACKET_NET_DEVICE_APCLI(skb_p, apcliIndex);
			SET_OS_PKT_NETDEV(skb_p, dev_p/*pAd->net_dev*/);


			/* transmit the packet */
			return Func(skb_p, apcliIndex);
		}
    }

	RELEASE_NDIS_PACKET(pAd, skb_p, NDIS_STATUS_FAILURE);

	return 0;
}

/*
========================================================================
Routine Description:
    Send a packet to WLAN.

Arguments:
    skb_p           points to our adapter
    dev_p           which WLAN network interface

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
INT ApCli_VirtualIF_PacketSend(
	IN PNDIS_PACKET 	pPktSrc, 
	IN PNET_DEV			pDev)
{
	MEM_DBG_PKT_ALLOC_INC(pPktSrc);

	if(!(RTMP_OS_NETDEV_STATE_RUNNING(pDev)))
	{
		/* the interface is down */
		RELEASE_NDIS_PACKET(NULL, pPktSrc, NDIS_STATUS_FAILURE);
		return 0;
	} /* End of if */

	return APC_PacketSend(pPktSrc, pDev, rt28xx_apcli_packet_xmit);
} /* End of ApCli_VirtualIF_PacketSend */


/*
========================================================================
Routine Description:
    IOCTL to WLAN.

Arguments:
    dev_p           which WLAN network interface
    rq_p            command information
    cmd             command ID

Return Value:
    0: IOCTL successfully
    otherwise: IOCTL fail

Note:
    SIOCETHTOOL     8946    New drivers use this ETHTOOL interface to
                            report link failure activity.
========================================================================
*/
INT ApCli_VirtualIF_Ioctl(
	IN PNET_DEV				dev_p, 
	IN OUT VOID 			*rq_p, 
	IN INT 					cmd)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

/*	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE)) */
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS)
		return -ENETDOWN;

	/* do real IOCTL */
	return (rt28xx_ioctl(dev_p, rq_p, cmd));
} /* End of ApCli_VirtualIF_Ioctl */
#endif


/*
========================================================================
Routine Description:
    Send a packet to WLAN via Apcli interface.

Arguments:
    pEndObj	points to our adapter
    pMblk		packet buffer need to delivery

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
static int RtmpOSNetDevApCliSend(
	IN END_OBJ * pEndObj,
	IN M_BLK_ID     pMblk)
{
	M_BLK_ID	pNewMblk;
	NDIS_STATUS		status = NDIS_STATUS_FAILURE;
	STATUS retVal;
	RTMP_ADAPTER *pAd = NULL;
	
	pAd = RTMP_OS_NETDEV_GET_PRIV(pEndObj);


	if (pAd == NULL)
	{
	 	return 0;
	}

	VX_WIRELESS_LOCK(); 
	pNewMblk = RtmpVxNetPktCheck(pEndObj, pMblk);
	//if (pNewMblk == NULL)
	//	return -1;


	if (pNewMblk)
	{
		if (taskIdSelf() != netTaskId )
		{
	       retVal = netJobAdd(ApCli_VirtualIF_PacketSend, pNewMblk, (PNET_DEV)pEndObj, 0, 0, 0);

		  	if (retVal != OK)	
		  	{
		  		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pNewMblk, NDIS_STATUS_FAILURE);
#ifdef DMA_DEBUG
				pAd->packet_netjobadd_fail_count++;
#endif /* DMA_DEBUG */
		  	}

			status=0;
	    }
		else
		{
			status = ApCli_VirtualIF_PacketSend((PNDIS_PACKET)pNewMblk, (PNET_DEV)pEndObj);
		}
		
	}
	else
	{
		status = 0;
	}

	VX_WIRELESS_UNLOCK();

	
	return status;

}


NET_FUNCS RtmpApCliEndFuncTable = {
    (FUNCPTR)ApCli_VirtualIF_Open,        /* Function to start the device. */
    (FUNCPTR)ApCli_VirtualIF_Close,         /* Function to stop the device. */
    (FUNCPTR)sysRtmpEndUnLoad,       /* Unloading function for the driver.*/
    (FUNCPTR)rt28xx_ioctl,       /* Ioctl function for the driver. */
    (FUNCPTR)RtmpOSNetDevApCliSend,         /* Send function for the driver. */
    (FUNCPTR)RtmpVxNetDevMCastAdd,     /* Multicast add function.*/
    (FUNCPTR)RtmpVxNetDevMCastDel,     /* Multicast delete function.  */
    (FUNCPTR)RtmpVxNetDevMCastGet,     /* Multicast retrieve function . */
    NULL,     /* Polling send function */
    NULL,     /* Polling receive function */
    endEtherAddressForm,            /* put address info into a NET_BUFFER */
    endEtherPacketDataGet,              /* get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet,	 	/* Get packet addresses. */
    NULL                            /* Bind */
};
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
/*
========================================================================
Routine Description:
    Send a packet to WLAN via WDS interface.

Arguments:
    pEndObj	points to our adapter
    pMblk		packet buffer need to delivery

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
static int RtmpOSNetDevWDSSend(
	IN END_OBJ * pEndObj,
	IN M_BLK_ID     pMblk)
{
	M_BLK_ID	pNewMblk;

	pNewMblk = RtmpVxNetPktCheck(pEndObj, pMblk);
	if (pNewMblk == NULL)
		return -1;
	
	return WdsVirtualIFSendPackets((PNDIS_PACKET)pNewMblk, (PNET_DEV)pEndObj);

}


NET_FUNCS RtmpWDSEndFuncTable = {
    (FUNCPTR)WdsVirtualIF_open,        /* Function to start the device. */
    (FUNCPTR)WdsVirtualIF_close,         /* Function to stop the device. */
    (FUNCPTR)sysRtmpEndUnLoad,       /* Unloading function for the driver.*/
    (FUNCPTR)rt28xx_ioctl,       			/* Ioctl function for the driver. */
    (FUNCPTR)RtmpOSNetDevWDSSend,         /* Send function for the driver. */
    (FUNCPTR)RtmpVxNetDevMCastAdd,     /* Multicast add function.*/
    (FUNCPTR)RtmpVxNetDevMCastDel,     /* Multicast delete function.  */
    (FUNCPTR)RtmpVxNetDevMCastGet,     /* Multicast retrieve function . */
    NULL,     /* Polling send function */
    NULL,     /* Polling receive function */
    NULL,            /* put address info into a NET_BUFFER */
    NULL,              /* get pointer to data in NET_BUFFER */
    NULL,          /* Get packet addresses. */
    NULL                            /* Bind */
};
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MESH_SUPPORT
/*
========================================================================
Routine Description:
    Send a packet to WLAN via WDS interface.

Arguments:
    pEndObj	points to our adapter
    pMblk		packet buffer need to delivery

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
static int RtmpOSNetDevMeshSend(
	IN END_OBJ * pEndObj,
	IN M_BLK_ID     pMblk)
{
	M_BLK_ID	pNewMblk;

	pNewMblk = RtmpVxNetPktCheck(pEndObj, pMblk);
	if (pNewMblk == NULL)
		return -1;
	
	return Mesh_VirtualIF_PacketSend((PNDIS_PACKET)pNewMblk, (PNET_DEV)pEndObj);

}
NET_FUNCS RtmpMeshEndFuncTable = {
    (FUNCPTR)Mesh_VirtualIF_Open,        /* Function to start the device. */
    (FUNCPTR)Mesh_VirtualIF_Close,         /* Function to stop the device. */
    (FUNCPTR)sysRtmpEndUnLoad,       /* Unloading function for the driver.*/
    (FUNCPTR)rt28xx_ioctl,       /* Ioctl function for the driver. */
    (FUNCPTR)RtmpOSNetDevMeshSend,   /* Send function for the driver. */
    (FUNCPTR)RtmpVxNetDevMCastAdd,     /* Multicast add function.*/
    (FUNCPTR)RtmpVxNetDevMCastDel,     /* Multicast delete function.  */
    (FUNCPTR)RtmpVxNetDevMCastGet,     /* Multicast retrieve function . */
    NULL,     /* Polling send function */
    NULL,     /* Polling receive function */
    endEtherAddressForm,            /* put address info into a NET_BUFFER */
    endEtherPacketDataGet,              /* get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet,	 	/* Get packet addresses. */
    NULL                           				/* Bind */
};
#endif /* MESH_SUPPORT */


/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        					     */
/*---------------------------------------------------------------------*/

/* public function prototype */
int rt28xx_close(IN VOID *pNetDev);
int rt28xx_open(IN  VOID *pNetDev);


/*
========================================================================

 RtmpVxNetDevMCastAdd - add the multicast address to the multicast list of the device

 	This routine add the multicast address to the list of whatever the driver.

 RETURNS: OK or ERROR.
========================================================================
*/
STATUS RtmpVxNetDevMCastAdd(
	IN END_OBJ	*pEndObj,
	IN char		*pAddr)
{
	STATUS	retVal;

	retVal = etherMultiAdd(&pEndObj->multiList, pAddr);

	if (retVal == ENETRESET)
	{
		/* FIXME: Currently we didn't implement the multicast filter list in our driver.*/
		pEndObj->nMulti++;
		retVal = OK;
	}

	return (retVal == OK) ? OK : ERROR;
}


/*
========================================================================

 RtmpVxNetDevMCastDel - Delete the multicast address list for the device

	 This routine delete the multicast list of whatever the driver is already listening for.

 RETURNS: OK or ERROR.
========================================================================
*/
STATUS RtmpVxNetDevMCastDel(
	IN END_OBJ	*pEndObj,
	IN char		*pAddr)
{
	STATUS	retVal;

	retVal = etherMultiDel(&pEndObj->multiList, pAddr);

	if (retVal == ENETRESET)
	{
		/* FIXME: Currently we didn't implement the multicast filter list in our driver.*/
		pEndObj->nMulti--;
		retVal = OK;
	}

	return (retVal == OK) ? OK : ERROR;
}


/*
========================================================================

 RtmpVxNetDevMCastGet - get the multicast address list for the device

 This routine gets the multicast list of whatever the driver
 is already listening for.

 RETURNS: OK or ERROR.
========================================================================
*/

STATUS RtmpVxNetDevMCastGet(
	IN END_OBJ *pEndObj,
	IN MULTI_TABLE* pTable)
{
    return (etherMultiGet (&pEndObj->multiList, pTable));
}


/*
========================================================================
Routine Description:
    Close raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int MainVirtualIF_close(IN PNET_DEV net_dev)
{
	//RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	DRV_CTRL * pDrvCtrl = (DRV_CTRL *)net_dev;
	RTMP_ADAPTER *pAd = wlGetInfAdapter((END_OBJ *)&pDrvCtrl->endObj);
	//DRV_CTRL * pDrvCtrl = (DRV_CTRL *)(&g_wlan_drvCtrl[0]);

	/* Sanity check for pAd */
	if (pAd == NULL)
		return 0; /* close ok */

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].bcn_buf.bBcnSntReq = FALSE;
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* kick out all STAs behind the bss. */
		MbssKickOutStas(pAd, MAIN_MBSSID, REASON_DISASSOC_INACTIVE);
	}

	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef MESH_SUPPORT
	MeshMakeBeacon(pAd, MESH_BEACON_IDX(pAd));
	MeshUpdateBeaconFrame(pAd, MESH_BEACON_IDX(pAd));
#endif /* MESH_SUPPORT */

	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	pDrvCtrl->started = FALSE;

	return 0;
}


/*
========================================================================
Routine Description:
    Open raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int MainVirtualIF_open(IN PNET_DEV net_dev)
{
	DBGPRINT(RT_DEBUG_TRACE, ("-->MainVirtualIF_open net_dev=0x%x\n",net_dev));

	RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	//DRV_CTRL * pDrvCtrl = (DRV_CTRL *)net_dev;
	//RTMP_ADAPTER *pAd = wlGetInfAdapter((END_OBJ *)&pDrvCtrl->endObj);
	DRV_CTRL * pDrvCtrl = (DRV_CTRL *)(&g_wlan_drvCtrl[0]);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Into MainVirtualIF_open()!pAd=0x%x\n", pAd));
	/* Sanity check for pAd */
	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Into MainVirtualIF_open()!pAd == NULL\n"));
		return 0; /* close ok */
	}

#ifdef CONFIG_AP_SUPPORT
	/*pAd->ApCfg.MBSSID[MAIN_MBSSID].bcn_buf.bBcnSntReq = TRUE;*/
#endif /* CONFIG_AP_SUPPORT */
	if (VIRTUAL_IF_UP(pAd) != 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("VIRTUAL_IF_UP fail\n"));
		return -1;
	}
	pDrvCtrl->started = TRUE;

	return 0;
}


/*
========================================================================
Routine Description:
    Close raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int rt28xx_close(IN VOID *dev)
{
	END_OBJ *pEndObj = (struct end_object *)dev;
	RTMP_ADAPTER	*pAd = RTMP_OS_NETDEV_GET_PRIV(pEndObj);
	UINT32			i = 0;

#ifdef RTMP_MAC_USB
	/*DECLARE_WAIT_QUEUE_HEAD(unlink_wakeup); */
	/*DECLARE_WAITQUEUE(wait, current); */

	/*RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS); */
#endif /* RTMP_MAC_USB */


    DBGPRINT(RT_DEBUG_TRACE, ("===> rt28xx_close\n"));

	/* Sanity check for pAd */
	if (pAd == NULL)
		return 0; /* close ok */

	RTMPDrvClose(pAd, dev);

#if 0
#ifdef WMM_ACM_SUPPORT
	/* must call first */
	ACMP_Release(pAd);
#endif /* WMM_ACM_SUPPORT */

#ifdef MESH_SUPPORT
	/* close all mesh link before the interface go down. */
	if (MESH_ON(pAd))
		MeshDown(pAd, TRUE);
#endif /* MESH_SUPPORT */

#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		BOOLEAN Cancelled;
#ifdef RTMP_MAC_PCI
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_CLOSE);
#endif /* RTMP_MAC_PCI */

		/* If dirver doesn't wake up firmware here, */
		/* NICLoadFirmware will hang forever when interface is up again. */
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        {      
		    AsicForceWakeup(pAd, TRUE);
        }

#ifdef QOS_DLS_SUPPORT
		/* send DLS-TEAR_DOWN message, */
		if (pAd->CommonCfg.bDLSCapable)
		{
			UCHAR i;

			/* tear down local dls table entry */
			for (i=0; i<MAX_NUM_OF_INIT_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}

			/* tear down peer dls table entry */
			for (i=MAX_NUM_OF_INIT_DLS_ENTRY; i<MAX_NUM_OF_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}
			RTMP_MLME_HANDLER(pAd);
		}
#endif /* QOS_DLS_SUPPORT */

		if (INFRA_ON(pAd) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);
			if (Elem != NULL)
			{
				COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
				DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;

				MsgElem->Machine = ASSOC_STATE_MACHINE;
				MsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
				MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
				NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

				/* Prevent to connect AP again in STAMlmePeriodicExec */
				pAd->MlmeAux.AutoReconnectSsidLen= 32;
				NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

				pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
				MlmeDisassocReqAction(pAd, MsgElem);
				kfree(MsgElem);
			
				RTMPusecDelay(1000);
			}
		}

#ifdef RTMP_MAC_USB
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);
#endif /* RTMP_MAC_USB */

#ifdef CCX_SUPPORT
		RTMPCancelTimer(&pAd->StaCfg.LeapAuthTimer, &Cancelled);
#endif

		RTMPCancelTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, &Cancelled);
		RTMPCancelTimer(&pAd->StaCfg.WpaDisassocAndBlockAssocTimer, &Cancelled);

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		/* send wireless event to wpa_supplicant for infroming interface down. */
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */

		MlmeRadioOff(pAd);
#ifdef RTMP_MAC_PCI
		pAd->bPCIclkOff = FALSE;    
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_STA_SUPPORT */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RTMPusecDelay(1000);
		}
	}
	
#ifdef RTMP_MAC_USB
	/* ensure there are no more active urbs. */
	/*add_wait_queue (&unlink_wakeup, &wait); */
	/*pAd->wait = &unlink_wakeup; */

	/* maybe wait for deletions to finish. */
	i = 0;
	while(i < 25)
	{
		unsigned long IrqFlags;

		RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
		if (pAd->PendingRx == 0)
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
			break;
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
                RTMPusecDelay(UNLINK_TIMEOUT_MS*1000);	/*Time in microsecond */
		i++;
	}
	/*pAd->wait = NULL; */
	/*remove_wait_queue (&unlink_wakeup, &wait); */
#endif /* RTMP_MAC_USB */

#ifdef CONFIG_AP_SUPPORT

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef RTMP_MAC_USB
	  /*RTMPCancelTimer(&pAd->CommonCfg.BeaconUpdateTimer, &Cancelled); */
#endif /* RTMP_MAC_USB */

#ifdef DOT11N_DRAFT3
		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED)
		{
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}
#endif /* DOT11N_DRAFT3 */

		/* PeriodicTimer already been canceled by MlmeHalt() API. */
		/*RTMPCancelTimer(&pAd->PeriodicTimer,	&Cancelled); */
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Close kernel threads or tasklets */
	RtmpMgmtTaskExit(pAd);

	/* Stop Mlme state machine */
	MlmeHalt(pAd);

	/* Close kernel threads or tasklets */
	RtmpNetTaskExit(pAd);


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		MacTableReset(pAd);
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */

		/* Shutdown Access Point function, release all related resources */
		APShutdown(pAd);
#ifdef AUTO_CH_SELECT_ENHANCE
		/* Free BssTab & ChannelInfo tabbles. */
		AutoChBssTableDestroy(pAd);
		ChannelInfoDestroy(pAd);
#endif /* AUTO_CH_SELECT_ENHANCE */
	}
#endif /* CONFIG_AP_SUPPORT */

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);

#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		WscStop(pAd, FALSE, &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
				FALSE,
#endif /* CONFIG_AP_SUPPORT */
				&pAd->StaCfg.WscControl);
#endif /* CONFIG_STA_SUPPORT */

#ifdef OLD_DH_KEY
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	    	WSC_VFREE_KEY_MEM(pAd->ApCfg.MBSSID[0].WscControl.pPubKeyMem, pAd->ApCfg.MBSSID[0].WscControl.pSecKeyMem);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		WSC_VFREE_KEY_MEM(pAd->StaCfg.WscControl.pPubKeyMem, pAd->StaCfg.WscControl.pSecKeyMem);
#endif /* CONFIG_STA_SUPPORT */
#endif /* OLD_DH_KEY */

#ifndef OLD_DH_KEY
	DH_freeall();
#endif /* OLD_DH_KEY */

	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Stop(pAd);
#endif /* WSC_INCLUDED */

#ifdef RTMP_MAC_PCI
	{
			BOOLEAN brc;
			/*	ULONG			Value; */

			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			{
				RTMP_ASIC_INTERRUPT_DISABLE(pAd);
			}

			/* Receive packets to clear DMA index after disable interrupt. */
			/*RTMPHandleRxDoneInterrupt(pAd); */
			/* put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all */
			/* register access before Radio off. */


			brc=RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);
			if (brc==FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__)); 
			}
	}
	

/*
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}

	// Disable Rx, register value supposed will remain after reset 
	NICIssueReset(pAd);
*/

	/* Free IRQ */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		/* Deregister interrupt function */
		RtmpOSIRQRelease(pEndObj);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}
#endif /* RTMP_MAC_PCI */

	/* Free Ring or USB buffers */
	RTMPFreeTxRxRingMemory(pAd);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef DOT11_N_SUPPORT
	/* Free BA reorder resource */
	ba_reordering_resource_release(pAd);
#endif /* DOT11_N_SUPPORT */

#ifdef RTMP_MAC_USB
#ifdef INF_AMAZON_SE
	if (pAd->UsbVendorReqBuf)
		os_free_mem(pAd, pAd->UsbVendorReqBuf);
#endif /* INF_AMAZON_SE */
#endif /* RTMP_MAC_USB */


	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
#endif
	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt28xx_close\n"));
	
	return 0; /* close ok */
}


/*
========================================================================
Routine Description:
    Open raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
========================================================================
*/
int rt28xx_open(IN VOID *dev)
{				 
	END_OBJ *pEndObj = (END_OBJ *)dev;
	PRTMP_ADAPTER pAd;
	int retval = 0;
	// int bbpId, rfId = 0;
	// UCHAR	regBBP, regRF = 0;
	// char msg[4096];
	UCHAR Inum;
	// INT index;
	// pAd = (PRTMP_ADAPTER)pEndObj->devObject.pDevice;
	pAd=RTMP_OS_NETDEV_GET_PRIV(pEndObj);
	 DBGPRINT(RT_DEBUG_ERROR, ("-->rt28xx_open!\n"));
	 
	/* Sanity check for pAd */
	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free; So the net_dev->priv will be NULL in 2rd open */
		DBGPRINT(RT_DEBUG_ERROR, ("pAd is NULL!\n"));
		printf("pAd is NULL!\n");
		return -1;
	}

	/* Request interrupt service routine for PCI device */
	/* register the interrupt routine with the os */
#if 0
#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
		SYS_INT_CONNECT(INUM_TO_IVEC(IV_RT2880_INIC_VEC), rt2860_interrupt, pAd, retval);
	}
	else
#endif /* RTMP_RBUS_SUPPORT */
#ifdef RTMP_PCI_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCI) /* RT2880 PCI */
	{
		//SYS_INT_CONNECT(INUM_TO_IVEC(IV_PCIE_VEC), rt2860_interrupt, pAd, retval);
	}
#endif /* RTMP_PCI_SUPPORT */
#endif
	RTMP_DRIVER_MCU_SLEEP_CLEAR(pAd);
	
#if 1
	Inum=(0x50+6);
	//printk("INUM_TO_IVEC=%d\n",INUM_TO_IVEC(Inum));
	//DBGPRINT(RT_DEBUG_ERROR, ("INUM_TO_IVEC=%d\n",INUM_TO_IVEC(Inum)));
	if (intConnect(INUM_TO_IVEC(Inum), rt2860_interrupt, (int)pAd) < 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("init irq failed!\n"));
		printf("init irq failed!\n");
		goto err;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("init irq success!\n"));
	}		
#endif
	//unmask_irq(PCI_A_INT);
	
	//unmask_irq(PCI_B_INT);
	
	/* Init IRQ parameters */
	RTMP_DRIVER_IRQ_INIT(pAd);

#ifdef VXWORKS
#ifdef CONFIG_AP_SUPPORT
    if (pAd->OpMode == OPMODE_AP)
	{
		UINT32 IdMbss;
        	BSS_STRUCT *pMbss;

		for(IdMbss=0; IdMbss<HW_BEACON_MAX_NUM; IdMbss++)
			{

			pMbss = &pAd->ApCfg.MBSSID[IdMbss];
			if (pMbss) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL,DBG_LVL_ERROR, ("%s():enter wdev_bcn_buf_init[%d] !\n", __FUNCTION__,IdMbss));
				wdev_bcn_buf_init(pAd, &pMbss->bcn_buf);
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():func_dev is NULL!\n", __FUNCTION__));
				goto err4;
			}	
		}
	}
#endif
#endif
	
	/* Chip & other init */
	if (rt28xx_init(pAd, NULL, NULL) == FALSE)
	{
		printf("rt28xx_init FALSE!\n");
		goto err;
	}
	//RT28XXDMADisable(pAd);
	//RTMP_ASIC_INTERRUPT_DISABLE(pAd);
#if 1
#ifdef MBSS_SUPPORT
	/* the function can not be moved to RT2860_probe() even register_netdev()
	   is changed as register_netdevice().
	   Or in some PC, kernel will panic (Fedora 4) */
	/*RT28xx_MBSS_Init(pAd, pAd->net_dev);*/
#endif /* MBSS_SUPPORT */

#ifdef WDS_SUPPORT
	//RT28xx_WDS_Init(pAd, pAd->net_dev);
#endif /* WDS_SUPPORT */
#endif

#ifdef WSC_INCLUDED
	/* WSC hardware push button function 0811 */
	//WSC_HDR_BTN_Init(pAd);
#endif 

#ifdef APCLI_SUPPORT
	/*RT28xx_ApCli_Init(pAd, pAd->net_dev);*/
#endif /* APCLI_SUPPORT */

#ifdef MESH_SUPPORT
	RTMP_Mesh_Init(pAd, pAd->net_dev, pHostName);
#endif /* MESH_SUPPORT */	
	//goto err;

	/* Enable Interrupt */
//printk("pAd->int_enable_reg=0x%x\n",pAd->int_enable_reg);
//pAd->int_enable_reg=0x00;
	//RTMP_IRQ_ENABLE(pAd);

	/* Now Enable RxTx */
	//RTMPEnableRxTx(pAd);

	RTMPDrvOpen(pAd);

	//RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

#if 0
	{
		UINT32 reg = 0;
		RTMP_IO_READ32(pAd, 0x1300, &reg);  /* clear garbage interrupts */
		DBGPRINT(RT_DEBUG_TRACE, ("0x1300 = %08x\n", reg));
	}
#endif


/*+++Add for VxWorks Shell Cmd supprot */
#ifdef RTMP_VX_SHELL_CMD_SUPPORT
#ifdef VXWORKS_6X
	RtmpVxShellCmdArrayAdd();
#endif /* VXWORKS_6X */
#endif /* RTMP_VX_SHELL_CMD_SUPPORT */
/*---Add for VxWorks Shell Cmd supprot */
	//RTMP_IRQ_ENABLE(pAd);
	//RT28XXDMAEnable(pAd);
	pAd->bApStarted =TRUE;

	
	
	return (retval);

err:
	return (-1);
} /* End of rt28xx_open */


/*
========================================================================
Routine Description:
	Early check if the packet has enough packet headeroom for latter handling.

Arguments:
	pEndObj		the end object.
	pMblk		the pointer refer to a packet buffer
	
Return Value:
	packet buffer pointer:	if sanity check success.
	NULL:				if failed

Note:
	This function is the entry point of Tx Path for Os delivery packet to 
	our driver. You only can put OS-depened & STA/AP common handle procedures 
	in here.
========================================================================
*/
static inline M_BLK_ID RtmpVxNetPktCheck(
	IN END_OBJ *pEndObj,
	IN M_BLK_ID pMblk)
{
	M_BLK_ID		pNewMblk;
	RTMP_ADAPTER	*pAd;
    BOOLEAN			bNeedCopy = FALSE;


	pAd = RTMP_OS_NETDEV_GET_PRIV(pEndObj);
	
	if (!(pEndObj->flags & IFF_UP))
	{
		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pMblk, NDIS_STATUS_FAILURE);
		return NULL;
	}
#if 1
	if ( GET_OS_PKT_LEN(pMblk) > 1536)
	{
		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pMblk, NDIS_STATUS_FAILURE);
		return NULL;
	}
#endif	
#if 0
	/* This check not used now due to some packet from vxworks kernel didn't set this up.*/
	if ((pMblk->m_flags & M_PKTHDR) != M_PKTHDR)
	{
		DBGPRINT(RT_DEBUG_WARN, ("M_PKTHDR not set!\n"));
		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pMblk, NDIS_STATUS_FAILURE);
		return NULL;
	}
#endif

	/* Check if we need to do copy the original mBlk to a new mBlk for further access */
	if ((MBLK_TO_NET_POOL(pMblk) == _pNetDpool) || (pMblk->pClBlk->clRefCnt > 1)
		|| (pMblk->m_next) || ((pMblk->m_data - pMblk->pClBlk->clNode.pClBuf) <= CB_MAX_OFFSET)
#ifdef CACHE_DMA_SUPPORT
		||(memcmp(pMblk->pClBlk->clNode.pClBuf, CACHE_DMA_SIGNAUTURE, 4) != 0)
#endif /* CACHE_DMA_SUPPORT */
		)
	{
#if 0
		DBGPRINT(RT_DEBUG_TRACE, ("Warning!!! a packet sent with m_next(0x%x), headroom(0x%x - 0x%x = %d)\n", 
						(pMblk->m_next), pMblk->m_data, pMblk->pClBlk->clNode.pClBuf, 
						(pMblk->m_data - pMblk->pClBlk->clNode.pClBuf)));
#endif
		bNeedCopy = TRUE;
	}
	else
	{
		NdisZeroMemory(pMblk->pClBlk->clNode.pClBuf, CB_MAX_OFFSET);
	}
	
	/* Assign the pEndObj to mBlk first, or the skb_copy will failed! */
	SET_OS_PKT_NETDEV(pMblk, pEndObj);
	if (bNeedCopy)
	{
		pNewMblk = skb_copy((PNDIS_PACKET)pMblk, 0);
		RELEASE_NDIS_PACKET(pAd, pMblk, NDIS_STATUS_SUCCESS);
		return pNewMblk;
	}
		
	return pMblk;
}


/*
========================================================================
Routine Description:
    The entry point for Linux kernel sent packet to our driver.

Arguments:
    sk_buff *skb		the pointer refer to a sk_buffer.

Return Value:
    0					

Note:
	This function is the entry point of Tx Path for Os delivery packet to 
	our driver. You only can put OS-depened & STA/AP common handle procedures 
	in here.
========================================================================
*/
STATUS rt28xx_packet_xmit(M_BLK_ID pMblk, int wdev_idx)
{
	int status = 0;
	struct end_object *pEndObj;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket, pNewPacket;
	struct wifi_dev *wdev;
	
	
	pEndObj = GET_OS_PKT_NETDEV(pMblk);
	pAd =  (RTMP_ADAPTER *)(RTMP_OS_NETDEV_GET_PRIV(pEndObj));
	pPacket= (PNDIS_PACKET) pMblk;

	//DBGPRINT(RT_DEBUG_TRACE, ("-->rt28xx_packet_xmit\n"));
	wdev = &pAd->ApCfg.MBSSID[wdev_idx].wdev;

#if 1 //yiwei debug tx
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%d: rt28xx_packet_xmit() ==> wdev=%p, wdev_idx=%d\n",__LINE__,wdev,wdev_idx));
#endif

	/* RT2870STA does this in RTMPSendPackets() */
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
		return 0;
	}
#endif /* RALINK_ATE */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Drop send request since we are in monitor mode */
		if (MONITOR_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			goto done;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

        /* EapolStart size is 18 */
	if ( GET_OS_PKT_LEN(pMblk) < 14)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("bad packet size: %d\n", GET_OS_PKT_LEN(pMblk)));
//		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pMblk), GET_OS_PKT_LEN(pMblk));//Carter comment
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		goto done;
	}

	/* Check if we need to copy the packet */
	if (GET_OS_PKT_HEADROOM(pPacket) < CB_MAX_OFFSET)
	{
		printf("\n we need to copy the packet \n");
		pNewPacket = skb_copy(pPacket, 0);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		if (pNewPacket == NULL)
		{
			goto done;
		}
		pPacket = pNewPacket;
	}
	RTMP_SET_PACKET_5VT(pPacket, 0);

/* TODO: For vxWorks, shall we add this features?? */
#if 0
#ifdef RT2880
#if !defined(CONFIG_RA_NAT_NONE)
/* bruce+
 */
	unsigned int flags;
	if(ra_sw_nat_hook_tx!= NULL)
	{
		spin_lock_irqsave(pAd->page_lock, flags);
		ra_sw_nat_hook_tx(pPacket);
		spin_unlock_irqrestore(pAd->page_lock, flags);
	}
#endif
#endif /* RT2880 */
#endif

#if 0
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APSendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);
#endif /* CONFIG_AP_SUPPORT */
#endif

	return wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1, wdev);
	status = 0;

done:
			   
	return status;
}


STATUS rt28xx_apcli_packet_xmit(M_BLK_ID pMblk, int wdev_idx)
{
	int status = 0;
	struct end_object *pEndObj;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket, pNewPacket;
	struct wifi_dev *wdev;
	PAPCLI_STRUCT pApCliEntry = NULL;

	
	pEndObj = GET_OS_PKT_NETDEV(pMblk);
	pAd =  (RTMP_ADAPTER *)(RTMP_OS_NETDEV_GET_PRIV(pEndObj));
	pPacket= (PNDIS_PACKET) pMblk;

	pApCliEntry = &pAd->ApCfg.ApCliTab[0];
	wdev = &pApCliEntry->wdev;
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
		return 0;
	}
#endif /* RALINK_ATE */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Drop send request since we are in monitor mode */
		if (MONITOR_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			goto done;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

        /* EapolStart size is 18 */
	if ( GET_OS_PKT_LEN(pMblk) < 14)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("bad packet size: %d\n", GET_OS_PKT_LEN(pMblk)));
//		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pMblk), GET_OS_PKT_LEN(pMblk));//Carter comment
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		goto done;
	}

	/* Check if we need to copy the packet */
	if (GET_OS_PKT_HEADROOM(pPacket) < CB_MAX_OFFSET)
	{
		printf("\n we need to copy the packet \n");
		pNewPacket = skb_copy(pPacket, 0);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		if (pNewPacket == NULL)
		{
			goto done;
		}
		pPacket = pNewPacket;
	}
	RTMP_SET_PACKET_5VT(pPacket, 0);

/* TODO: For vxWorks, shall we add this features?? */
#if 0
#ifdef RT2880
#if !defined(CONFIG_RA_NAT_NONE)
/* bruce+
 */
	unsigned int flags;
	if(ra_sw_nat_hook_tx!= NULL)
	{
		spin_lock_irqsave(pAd->page_lock, flags);
		ra_sw_nat_hook_tx(pPacket);
		spin_unlock_irqrestore(pAd->page_lock, flags);
	}
#endif
#endif /* RT2880 */
#endif

#if 0
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APSendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);
#endif /* CONFIG_AP_SUPPORT */
#endif

	return wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1, wdev);
	status = 0;

done:
			   
	return status;
}
/*
========================================================================
Routine Description:
    Send a packet to WLAN.

Arguments:
    skb_p           points to our adapter
    dev_p           which WLAN network interface

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
static int rt28xx_send_packets(
	IN END_OBJ * pEndObj,
	IN M_BLK_ID     pMblk)
{
	M_BLK_ID     pNewMblk;
	NDIS_STATUS		status = NDIS_STATUS_FAILURE;
	RTMP_ADAPTER *pAd = NULL;
	int idx = -1; 
	STATUS retVal;

	 pAd = RTMP_OS_NETDEV_GET_PRIV(pEndObj);
	 idx = ENDOBJ_GET_UNIT(pEndObj);

#if 1 //yiwei debug tx
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%d: rt28xx_send_packets() ==> idx=%d\n",__LINE__,idx));

	if ((DebugCategory == DBG_CAT_TX) &&  RTDebugLevel == DBG_LVL_INFO) 
	{
		hex_dump_woody("rt28xx_send_packets() raw pkt  dump ==>",GET_OS_PKT_DATAPTR((PNDIS_PACKET)pMblk),GET_OS_PKT_LEN((PNDIS_PACKET)pMblk));		
	}
	
#endif

	 if (idx == -1)
	 {
	 	RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pMblk, NDIS_STATUS_FAILURE);
	 	return status;
	 }

	VX_WIRELESS_LOCK();

	pNewMblk = RtmpVxNetPktCheck(pEndObj, pMblk);

#if 1 //yiwei debug tx
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%d: rt28xx_send_packets() ==> pNewMblk=%p\n",__LINE__,pNewMblk));
#endif

	if (pNewMblk)
	{
		if (taskIdSelf() != netTaskId )
		{
	       retVal = netJobAdd(rt28xx_packet_xmit, pNewMblk, idx, 0, 0, 0);

		   if (retVal != OK)
		   {
#ifdef DMA_DEBUG
		   		pAd->packet_netjobadd_fail_count++; 
#endif /* DMA_DEBUG */
				RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)pNewMblk, NDIS_STATUS_FAILURE);
		   }

		   status=NDIS_STATUS_SUCCESS;
	    }
		else
		{
			status = rt28xx_packet_xmit(pNewMblk, idx);
		}
		
	}
	else
	{
		status = 0;
	}

	VX_WIRELESS_UNLOCK();

	return status;
}


/******************************************************************************
*
* RtmpVxIoctlAddrFilterSet - set the address filter for multicast addresses
*
* This routine goes through all of the multicast addresses on the list
* of addresses (added with the endAddrAdd() routine) and sets the
* device's filter correctly.
*
* RETURNS: N/A.
*/

LOCAL void RtmpVxIoctlAddrFilterSet(
    IN END_OBJ *pEndObj	/* device to be updated */
)
{
	ETHER_MULTI* pCurr = END_MULTI_LST_FIRST (pEndObj);

	while (pCurr != NULL)
	{
		/* TODO - set up the multicast list */
		pCurr = END_MULTI_LST_NEXT(pCurr);
	}
	/* TODO - update the device filter list */
	
}


/******************************************************************************
*
* RtmpVxIoctlConfig - reconfigure the interface under us.
*
* Reconfigure the interface setting promiscuous mode, and changing the
* multicast interface list.
*
* RETURNS: N/A.
*/

void RtmpVxIoctlConfig(
	IN END_OBJ *pEndObj)
{

	/* Set promiscuous mode if it's asked for. */

	if (END_FLAGS_GET(pEndObj) & IFF_PROMISC)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():promiscuous mode on!\n", __FUNCTION__));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():promiscuous mode off!\n", __FUNCTION__));
	}

	/* Set up address filter for multicasting. */

	if (END_MULTI_LST_CNT(pEndObj) > 0)
	{
		RtmpVxIoctlAddrFilterSet(pEndObj);
	}

	/* TODO - shutdown device completely */

	/* TODO - reset all device counters/pointers, etc. */

	/* TODO - initialize the hardware according to flags */

	return;
}


void tbtt_tasklet(unsigned long data)
{
/*#define MAX_TX_IN_TBTT		(16) */

#ifdef CONFIG_AP_SUPPORT
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;

#ifdef RTMP_MAC_PCI
	if (pAd->OpMode == OPMODE_AP)
	{
#ifdef AP_QLOAD_SUPPORT
		/* update channel utilization */
		QBSS_LoadUpdate(pAd, 0);
#endif /* AP_QLOAD_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
		RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	}
#endif /* RTMP_MAC_PCI */

	if (pAd->OpMode == OPMODE_AP)
	{
		/* */
		/* step 7 - if DTIM, then move backlogged bcast/mcast frames from PSQ to TXQ whenever DtimCount==0 */
#if 0    
		/* NOTE: This updated BEACON frame will be sent at "next" TBTT instead of at cureent TBTT. The reason is */
		/*       because ASIC already fetch the BEACON content down to TX FIFO before driver can make any */
		/*       modification. To compenstate this effect, the actual time to deilver PSQ frames will be */
		/*       at the time that we wrapping around DtimCount from 0 to DtimPeriod-1 */
		if ((pAd->ApCfg.DtimCount + 1) == pAd->ApCfg.DtimPeriod)
#else
		if (pAd->ApCfg.DtimCount == 0)
#endif
		{
			PQUEUE_ENTRY    pEntry;
			BOOLEAN			bPS = FALSE;
			UINT 			count = 0;
			unsigned long 		IrqFlags;

/*			NdisAcquireSpinLock(&pAd->MacTabLock); */
/*			NdisAcquireSpinLock(&pAd->TxSwQueueLock); */

#ifdef MT_MAC

#ifdef USE_BMC
			//MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("use BMC\n"));
#endif
			UINT apidx = 0, mac_val = 0, deq_cnt = 0;
			
			if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->MacTab.fAnyStationInPsm == TRUE))
			{
#ifdef USE_BMC
				#define MAX_BMCCNT 16
				int fcnt = 0, max_bss_cnt = 0;

				/* BMC Flush */
				mac_val = 0x7fff0001;
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR1, mac_val);
				
				for (fcnt=0;fcnt<100;fcnt++)
				{
				RTMP_IO_READ32(pAd, ARB_BMCQCR1, &mac_val);
					if (mac_val == 0)
						break;
				}
				
				if (fcnt == 100)
				{
					MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: flush not complete, flush cnt=%d\n", __FUNCTION__, fcnt));
					return;
				}
				
				if ((pAd->ApCfg.BssidNum == 0) || (pAd->ApCfg.BssidNum == 1))
				{
					max_bss_cnt = 0xf;
				}
				else
				{
					max_bss_cnt = MAX_BMCCNT / pAd->ApCfg.BssidNum;
				}
#endif
				for(apidx=0;apidx<pAd->ApCfg.BssidNum;apidx++)		
            	{
	                BSS_STRUCT *pMbss;
					UINT wcid = 0, PseFcnt = 0, cnt = 0, bmc_cnt = 0;
					STA_TR_ENTRY *tr_entry = NULL;
			
					pMbss = &pAd->ApCfg.MBSSID[apidx];
				
					wcid = pMbss->wdev.tr_tb_idx;
					tr_entry = &pAd->MacTab.tr_entry[wcid]; 
				
					if (tr_entry->tx_queue[QID_AC_BE].Head != NULL)
					{
#ifdef USE_BMC
						if ((apidx >= 0) && (apidx <= 4))
						{
							RTMP_IO_READ32(pAd, ARB_BMCQCR2, &mac_val);
							if (apidx == 0)
								bmc_cnt = mac_val & 0xf;
							else 
								bmc_cnt = (mac_val >> (12+ (4*apidx))) & 0xf;
						}
						else if ((apidx >= 5) && (apidx <= 12))
						{
							RTMP_IO_READ32(pAd, ARB_BMCQCR3, &mac_val);
							bmc_cnt = (mac_val >> (4*(apidx-5))) & 0xf;
						}
						else if ((apidx >=13) && (apidx <= 15))
						{
							RTMP_IO_READ32(pAd, ARB_BMCQCR3, &mac_val);
							bmc_cnt = (mac_val >> (4*(apidx-13))) & 0xf;
						}
						else
						{
							MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: apidx(%d) not support\n", __FUNCTION__, apidx));
		                    return;
						}	
						
						if (bmc_cnt >= max_bss_cnt)
							deq_cnt = 0;
						else
							deq_cnt = max_bss_cnt - bmc_cnt;
						
						if (tr_entry->tx_queue[QID_AC_BE].Number <= deq_cnt)
#endif /* USE_BMC */
							deq_cnt = tr_entry->tx_queue[QID_AC_BE].Number;
					
						MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: bss:%d, deq_cnt = %d\n", __FUNCTION__, apidx, deq_cnt));
						RTMPDeQueuePacket(pAd, FALSE, QID_AC_BE, wcid, deq_cnt); 
			
						MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: bss:%d, deq_cnt = %d\n", __FUNCTION__, apidx, deq_cnt));
					}
			
					if (WLAN_MR_TIM_BCMC_GET(apidx) == 0x01)
					{
						if  ( (tr_entry->tx_queue[QID_AC_BE].Head == NULL) && 
							(tr_entry->EntryType == ENTRY_CAT_MCAST))
						{
							WLAN_MR_TIM_BCMC_CLEAR(tr_entry->func_tb_idx);	/* clear MCAST/BCAST TIM bit */
							MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL, DBG_LVL_WARN | DBG_FUNC_UAPSD, ("%s: clear MCAST/BCAST TIM bit \n", __FUNCTION__));
						}
					}
				}
#ifdef USE_BMC				
				/* BMC start */
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR0, 0x7fff0001);
#endif				
			}
#else /* MT_MAC */		
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->MacTab.McastPsQueue.Head)
			{
				bPS = TRUE;
				if (pAd->TxSwQueue[QID_AC_BE].Number <= (pAd->TxSwQMaxLen + MAX_PACKETS_IN_MCAST_PS_QUEUE))
				{
					pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
					/*if(pAd->MacTab.McastPsQueue.Number) */
					if (count)
					{
						RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);
					}
					InsertHeadQueue(&pAd->TxSwQueue[QID_AC_BE], pEntry);
					count++;
				}
				else
				{
					break;
				}
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			
#ifdef RELEASE_EXCLUDE
			DBGPRINT(RT_DEBUG_INFO, ("DTIM=%d/%d, tx mcast/bcast out...\n",pAd->ApCfg.DtimCount,pAd->ApCfg.DtimPeriod));
#endif /* RELEASE_EXCLUDE */			
			
/*			NdisReleaseSpinLock(&pAd->TxSwQueueLock); */
/*			NdisReleaseSpinLock(&pAd->MacTabLock); */
			if (pAd->MacTab.McastPsQueue.Number == 0)
			{			
                UINT bss_index;

                /* clear MCAST/BCAST backlog bit for all BSS */
                for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
					WLAN_MR_TIM_BCMC_CLEAR(bss_index);
                /* End of for */
			}
			pAd->MacTab.PsQIdleCount = 0;

			/* Dequeue outgoing framea from TxSwQueue0..3 queue and process it */
            if (bPS == TRUE) 
			{
				RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			}
#endif /* !MT_MAC */			
		}
	}

		
#endif /* CONFIG_AP_SUPPORT */
}


INT rt28xx_ioctl(
	IN	PNET_DEV	endDev, 
	IN	int			cmd, 
	IN	caddr_t		data)
{
	RTMP_ADAPTER	*pAd = NULL;
	INT				ret = 0;

	
	pAd = (RTMP_ADAPTER *)endDev->devObject.pDevice;
	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifdef VXWORKS_6X
	/* The cmd format : ((unsigned long)(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))) */
	DBGPRINT(RT_DEBUG_INFO, ("Cmd=0x%x, IN/OUT=0x%x, Group=%d, Number=%d, Len=%d! IOCBASECMD=0x%x!\n", 
			cmd, cmd & 0xff000000, IOCGROUP(cmd), (cmd & 0xff), IOCPARM_LEN(cmd), IOCBASECMD(cmd)));
#endif /* VXWORKS_6X */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		ret = rt28xx_ap_ioctl(endDev, cmd, data);
	}
#endif /* CONFIG_AP_SUPPORT */

#if 0
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		ret = rt28xx_sta_ioctl(endDev, cmd, data);
	}
#endif /* CONFIG_STA_SUPPORT */
#endif
	return ret;
}


/*
========================================================================
Routine Description:
    Allocate memory for adapter control block.

Arguments:
    pAd					Pointer to our adapter

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE
	NDIS_STATUS_RESOURCES

Note:
========================================================================
*/
NDIS_STATUS AdapterBlockAllocateMemory(
	IN PVOID	handle,
	OUT	PVOID	*ppAd,
	IN UINT32	SizeOfpAd)
{
	*ppAd = (PVOID)kmalloc(sizeof(RTMP_ADAPTER), 0); /*pci_alloc_consistent(pci_dev, sizeof(RTMP_ADAPTER), phy_addr); */
    
	if (*ppAd) 
	{
		NdisZeroMemory(*ppAd, sizeof(RTMP_ADAPTER));
		((PRTMP_ADAPTER)*ppAd)->OS_Cookie = handle;
		return (NDIS_STATUS_SUCCESS);
	}
	else
	{
		return (NDIS_STATUS_FAILURE);
	}
}


NDIS_STATUS RtmpVxNetPoolFree(struct os_cookie *pObj)
{
	int	clIdx;
	
	
	for (clIdx=0; clIdx < RTMP_NETPOOL_CNT; clIdx++)
	{
		/* free cl buffer */
		if (pObj->pClMemPtr[clIdx])
#ifdef CACHE_DMA_SUPPORT
			cacheDmaFree(pObj->pClMemPtr[clIdx]);
#else
			free(pObj->pClMemPtr[clIdx]);
#endif /* CACHE_DMA_SUPPORT */
	
		/* free mclBlk buffer */
		if (pObj->pMclMemPtr[clIdx])
		free(pObj->pMclMemPtr[clIdx]);
	
	}

	if(*pObj->pNetPool)
		free(*pObj->pNetPool);	

	return NDIS_STATUS_SUCCESS;
	
}


NDIS_STATUS RtmpVxNetPoolInit(struct os_cookie *pObj)
{
#ifdef MBLK_DEBUG
#else
	M_CL_CONFIG		mclBlkConfig;
#endif
	CL_DESC			clConfig;
	int				poolIdx, clSize;
	UCHAR			*memPtr;
	NET_POOL_ID		pNetPool;
	DBGPRINT(RT_DEBUG_TRACE, (" ==> %s\n", __FUNCTION__));
	/*
		This is how we would set up an END netPool using netBufLib(1).
	 	This code is pretty generic.

		1. Due to we need to reserve space for the cb, so we need to allocate two net pools and 
			each with one cluster:
				clConfig with cluster size 1536, used for the Mgmt/Tx Ring.
				clConfig with cluster size 3840, used for the Rx Ring. (AMSUD/RalinkAgg).
		2. Allocate netpool/mBlk/Cluster blocks from normal memory. Every mBlk and cluster is 
			prepended by a 4-byte header (which is why the size calculations for clusters and 
			mBlk structures contained an extra sizeof(long).
	  */

	/* Allocate memory for the net pools. one for Tx, one for Rx. */
	memPtr = malloc(sizeof(NET_POOL) * RTMP_NETPOOL_CNT);
	if (memPtr == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,("malloc for NET_POOL failed!\n"));
	    return NDIS_STATUS_FAILURE;
	}
	memset(memPtr, 0, sizeof(NET_POOL) * RTMP_NETPOOL_CNT);
	pObj->pNetPool[RTMP_NETPOOL_TX] = (NET_POOL_ID)memPtr;
	pObj->pNetPool[RTMP_NETPOOL_RX] = (NET_POOL_ID)((ULONG)memPtr + sizeof(NET_POOL));


	for (poolIdx = 0; poolIdx < RTMP_NETPOOL_CNT; poolIdx++)
	{
		pNetPool = pObj->pNetPool[poolIdx]; 
		clSize = 0;

	/*
			Initialize the CL_CONFIG, need provide four values:
				clSize - the size of a cluster in this cluster pool
				clNum - the number of clusters in this cluster pool
				memArea - pointer to an area of memory that can contain all the clusters
				memSize - the size of that memory area
	  */
		memset(&clConfig, 0, sizeof(clConfig));
		if (poolIdx == RTMP_NETPOOL_TX)
		{
#ifdef DBG
			clConfig.clNum = (TX_RING_SIZE + MGMT_RING_SIZE + MGMT_RING_SIZE);
#else
			clConfig.clNum = (TX_RING_SIZE + MGMT_RING_SIZE + MGMT_RING_SIZE) * 2;
#endif
			clSize = MGMT_DMA_BUFFER_SIZE;
			/* clConfig.clSize = ROUND_UP(MGMT_DMA_BUFFER_SIZE + sizeof(long),  _CACHE_ALIGN_SIZE)  - sizeof(long); */
		}
		else if (poolIdx == RTMP_NETPOOL_RX)
		{	
			clConfig.clNum = (RX_RING_SIZE * 4)+64;//Carter 6 to 5
			clSize = RX_BUFFER_NORMSIZE;
			/*clConfig.clSize = ROUND_UP(RX_BUFFER_NORMSIZE + sizeof(long), _CACHE_ALIGN_SIZE) - sizeof(long); */
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s:wrong poolIdx(%d)!\n", __FUNCTION__, poolIdx));
			goto err_free_mem;
		}
#ifdef VXWORKS_5X
		clSize += NETBUF_LEADING_SPACE;
#endif /* VXWORKS_5X */
		clConfig.clSize = ROUND_UP(clSize + sizeof(long), _CACHE_ALIGN_SIZE) - sizeof(long);

		clConfig.memSize = clConfig.clNum * (clConfig.clSize + sizeof(long));
#ifdef CACHE_DMA_SUPPORT
		memPtr = (char *)cacheDmaMalloc(clConfig.memSize + _CACHE_ALIGN_SIZE);
#else
		memPtr = (char *)malloc(clConfig.memSize + _CACHE_ALIGN_SIZE);
#endif /* CACHE_DMA_SUPPORT */
		if (memPtr == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s(): malloc for clConfig[%d] with size(%d) failed!\n", 
						__FUNCTION__, poolIdx, clConfig.memSize));
			goto err_free_mem;
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s(): malloc for clConfig[%d] with size(%d)!\n", 
						__FUNCTION__, poolIdx, clConfig.memSize+ _CACHE_ALIGN_SIZE));
		}
		clConfig.memArea = (char *)ROUND_UP(memPtr + sizeof(long),  _CACHE_ALIGN_SIZE) - sizeof(long);
		pObj->pClMemPtr[poolIdx] = memPtr;	

	
	/*
		Setup mBlk/cluster block pool with more mbufs than clBlks
		Calculate the total memory for all the M-Blks and CL-Blks.
			mclBlkConfig need to provide four parameters:
				mBlkNum - a count of `mBlk' structures, set as twice of the number of clusters
				clBlkNum - a count of `clBlk' structures
				memSize - the size of that memory area
				memArea - pointer to an area of memory that can contain all the 'mBlk' and 'clBlk' structures
	  */
#ifdef MBLK_DEBUG
		memset(&wireless_MclBlkConfig[poolIdx], 0, sizeof(wireless_MclBlkConfig[poolIdx]));
		wireless_MclBlkConfig[poolIdx].mBlkNum = clConfig.clNum * 2;
		wireless_MclBlkConfig[poolIdx].clBlkNum = clConfig.clNum;
		wireless_MclBlkConfig[poolIdx].memSize = (wireless_MclBlkConfig[poolIdx].mBlkNum * (M_BLK_SZ + sizeof (void *))) +
						(wireless_MclBlkConfig[poolIdx].clBlkNum *(CL_BLK_SZ + sizeof(void *)));
		wireless_MclBlkConfig[poolIdx].memArea = memalign (sizeof(long), wireless_MclBlkConfig[poolIdx].memSize);
		if (wireless_MclBlkConfig[poolIdx].memArea == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s() malloc for mclBlkConfig failed!\n", __FUNCTION__));
			goto err_free_mem;
		}

		pObj->pMclMemPtr[poolIdx] = wireless_MclBlkConfig[poolIdx].memArea;

			
		/*
			call kernel function to initialize the netpool
		*/
		if (netPoolInit(pNetPool, &wireless_MclBlkConfig[poolIdx], &clConfig, 1, NULL) == -1)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): netPoolInit(%d) failed!\n", __FUNCTION__, poolIdx));
			goto err_free_mem;
		}
		
		DBGPRINT(RT_DEBUG_ERROR, ("wireless_MclBlkConfig[%d].memArea = 0x%p\n", poolIdx, wireless_MclBlkConfig[poolIdx].memArea));
#else
		memset(&mclBlkConfig, 0, sizeof(mclBlkConfig));
		mclBlkConfig.mBlkNum = clConfig.clNum * 2;
		mclBlkConfig.clBlkNum = clConfig.clNum;
		mclBlkConfig.memSize = (mclBlkConfig.mBlkNum * (M_BLK_SZ + sizeof (void *))) +
						(mclBlkConfig.clBlkNum *(CL_BLK_SZ + sizeof(void *)));
		mclBlkConfig.memArea = memalign (sizeof(long), mclBlkConfig.memSize);
		if (mclBlkConfig.memArea == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s() malloc for mclBlkConfig failed!\n", __FUNCTION__));
			goto err_free_mem;
		}
		pObj->pMclMemPtr[poolIdx] = mclBlkConfig.memArea;

			
		/*
			call kernel function to initialize the netpool
		*/
		if (netPoolInit(pNetPool, &mclBlkConfig, &clConfig, 1, NULL) == -1)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): netPoolInit(%d) failed!\n", __FUNCTION__, poolIdx));
			goto err_free_mem;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("pNetPool[%d]=0x%x!\n", poolIdx, pObj->pNetPool[poolIdx]));
#endif	/* MBLK_DEBUG */
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n", __FUNCTION__));
	return NDIS_STATUS_SUCCESS;


err_free_mem:
	RtmpVxNetPoolFree(pObj);

	return NDIS_STATUS_FAILURE;
	
}


PNET_DEV RtmpPhyNetDevInit(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_OS_NETDEV_OP_HOOK *pNetDevHook)
{
	 END_OBJ	*pEndObj = NULL;
	
	pEndObj = (END_OBJ *)malloc(sizeof(END_OBJ));
	if (pEndObj == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RtmpPhyNetDevInit(): creation failed for main physical net device!\n"));
		return NULL;
	}

	NdisZeroMemory((unsigned char *)pNetDevHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	pNetDevHook->open = MainVirtualIF_open;
	pNetDevHook->stop = MainVirtualIF_close;
	pNetDevHook->xmit = rt28xx_send_packets;
	pNetDevHook->ioctl = rt28xx_ioctl;
	pNetDevHook->priv_flags = INT_MAIN;

	pNetDevHook->needProtcted = FALSE;
	memcpy(&pNetDevHook->devName[0],  "ra0", strlen("ra0"));
	pAd->net_dev = pEndObj;

	/* double-check if pAd is associated with the net_dev */
	if (RTMP_OS_NETDEV_GET_PRIV(pEndObj) == NULL)
	{
		RtmpOSNetDevFree(pEndObj);
		return NULL;
	}
	
#ifdef CONFIG_AP_SUPPORT
	//pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = pEndObj;
#endif /* CONFIG_AP_SUPPORT */

	if ((END_OBJ_INIT(pEndObj, (DEV_OBJ *)pAd, &pNetDevHook->devName[0],
	              0, &RtmpMAINEndFuncTable, &pNetDevHook->devName[0]) == ERROR)) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not init net device %s!\n", &pNetDevHook->devName[0]));
		return NULL;
	}
	
	if ((END_MIB_INIT(pEndObj, M2_ifType_ethernet_csmacd,
	              &pNetDevHook->devAddr[0],  6, ETHERMTU, END_SPEED) == ERROR))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not init MIB of net device %s!\n", &pNetDevHook->devName[0]));
		return NULL;
	}

	END_OBJ_READY(pEndObj, IFF_NOTRAILERS |IFF_BROADCAST |IFF_MULTICAST);
	
	DBGPRINT(RT_DEBUG_ERROR, ("RtmpPhyNetDevInit(): Allocate END object success!\n"));

	return pEndObj;
	
}


BOOLEAN RtmpPhyNetDevExit(
	IN RTMP_ADAPTER *pAd, 
	IN PNET_DEV net_dev)
{
	END_OBJ *pEndDev;

	pEndDev = (END_OBJ *)net_dev;

#ifdef MESH_SUPPORT
	RTMP_Mesh_Remove(pAd);
#endif /* MESH_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	/* remove all AP-client virtual interfaces. */
	RT28xx_ApCli_Remove(pAd);
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
	/* remove all WDS virtual interfaces. */
	RT28xx_WDS_Remove(pAd);
#endif /* WDS_SUPPORT */

#ifdef MBSS_SUPPORT
	RT28xx_MBSS_Remove(pAd);
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/* Unregister network device */
	if (net_dev != NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RtmpOSNetDevDetach(): RtmpOSNetDeviceDetach(), dev->name=%s!\n", 
					net_dev->devObject.name));
		RtmpOSNetDevDetach(net_dev);
	}

	return TRUE;
}

#ifdef MBLK_DEBUG
int cmdWpktParser(char* param)
{
	char* start = param;
	char command[MAX_CMD_LEN] = {0};
	UINT32 index = 0, cblkCnt = 0, mblkCnt = 0;
	char* pMblkBuf = NULL;
	char* pMblkBuf1 = NULL;
	char* pMblkBuf2 = NULL;
	char* pMblkBuf3 = NULL;
	
	pMblkBuf = wireless_MclBlkConfig[RTMP_NETPOOL_TX].memArea;
	//pMblkBuf1 = wireless_MclBlkConfig[RTMP_NETPOOL_SKB_COPY].memArea;
	//pMblkBuf2 = wireless_MclBlkConfig[RTMP_NETPOOL_SKB_COPY_1].memArea;
	pMblkBuf3 = wireless_MclBlkConfig[RTMP_NETPOOL_RX].memArea;
	
	memset(command, 0, MAX_CMD_LEN);
	start = spliter(start, CONVENE, command);
	if (('\0' == command[0]) || ISHELP(command))
	{
		goto cmdPacketShowHelp;
	}
	
	/* Show all arp entries. */
	if (0 == strcmp("-show", command))
	{
		start = spliter(start, CONVENE, command);
		if (('\0' == command[0]) || ISHELP(command))
		{
			goto cmdPacketShowHelp;
		}

		/* Show cblk chain. */
		if (0 == strcmp("cblk", command))
		{
			CL_BLK	*pClBlk = NULL;
			my_printf("CLBLK Info\n");
			my_printf("index\tclblk\t\tclbuf\t\trefcnt\n");
			
			for (index = 0; index < wireless_MclBlkConfig[RTMP_NETPOOL_TX].clBlkNum; index++)
			{
				pClBlk = (CL_BLK *)(pMblkBuf + (wireless_MclBlkConfig[RTMP_NETPOOL_TX].mBlkNum * 
					(M_BLK_SZ + sizeof(long))) + (index * CL_BLK_SZ));
				
				if (0 != pClBlk->clRefCnt)
				{
					cblkCnt++;
					my_printf("%d:\t0x%x\t0x%x\t0x%x\n", index, (UINT32)pClBlk, 
						(UINT32)pClBlk->clNode.pClBuf, pClBlk->clRefCnt);
				}
			}

			printf("CLCBLK %d.\n", cblkCnt);
		
			return OK;
		}

		if (0 == strcmp("mblk", command))
		{
			M_BLK_ID pMblk = NULL;
			UINT32 errorCnt = 0, freeCnt = 0;
			BOOL mblkFree = TRUE;
			
			my_printf("MBLK Info\n");
			my_printf("index\tmblk\t\tclblk\t\tclbuf\t\tdata\t\tlen\trefcnt\tflag\tstatus\n");
			for (index = 0; index < wireless_MclBlkConfig[RTMP_NETPOOL_TX].mBlkNum; index++)
			{
				pMblk = (M_BLK_ID)(pMblkBuf + (index * (M_BLK_SZ + sizeof(long))) + sizeof(long));
				mblkCnt++;
				
				if (MT_FREE != pMblk->mBlkHdr.mType)
				{
					if (NULL != pMblk->pClBlk)
					{
						my_printf("%d:\t0x%x\t0x%x\t0x%x\t0x%x\t%u\t%d\t0x%x\t%d\n", index, pMblk, 
							(UINT32)pMblk->pClBlk, (UINT32)pMblk->pClBlk->clNode.pClBuf, 
							(UINT32)pMblk->mBlkHdr.mData, pMblk->mBlkHdr.mLen, pMblk->pClBlk->clRefCnt, 
							pMblk->mBlkHdr.mFlags, pMblk->pClBlk->clFreeArg1
							);
					}
					else
					{
						errorCnt++;
					}
				}
				else
				{
					freeCnt++;
				}
			}


			my_printf("MBLK %d FREE %d ERROR %d \n", mblkCnt, freeCnt, errorCnt);
			return OK;
		}

#if 0
		if (0 == strcmp("mblk1", command))
		{
			M_BLK_ID pMblk = NULL;
			UINT32 errorCnt = 0, freeCnt = 0;
			BOOL mblkFree = TRUE;
			
			my_printf("MBLK 1 Info\n");
			my_printf("index\tmblk\t\tclblk\t\tclbuf\t\tdata\t\tlen\trefcnt\tflag\tstatus\n");
#if 1
			for (index = 0; index < wireless_MclBlkConfig[RTMP_NETPOOL_SKB_COPY].mBlkNum; index++)
			{

				if (pMblkBuf1 == NULL)
				{
					my_printf("pMblkBuf1 == NULL\n");
				} else
				{
					pMblk = (M_BLK_ID)(pMblkBuf1 + (index * (M_BLK_SZ + sizeof(long))) + sizeof(long));

					if (pMblk == NULL)
					{
						my_printf("pMblk == NULL\n");
					} else
					{
						mblkCnt++;
						
						if (MT_FREE != pMblk->mBlkHdr.mType)
						{
							if (NULL != pMblk->pClBlk)
							{
								my_printf("%d:\t0x%x\t0x%x\t0x%x\t0x%x\t%u\t%d\t0x%x\t%d\n", index, pMblk, 
									(UINT32)pMblk->pClBlk, (UINT32)pMblk->pClBlk->clNode.pClBuf, 
									(UINT32)pMblk->mBlkHdr.mData, pMblk->mBlkHdr.mLen, pMblk->pClBlk->clRefCnt, 
									pMblk->mBlkHdr.mFlags, pMblk->pClBlk->clFreeArg1
									);
							}
							else
							{
								errorCnt++;
							}
						}
						else
						{
							freeCnt++;
						}
					}
				}
			}
#endif

			my_printf("MBLK %d FREE %d ERROR %d \n", mblkCnt, freeCnt, errorCnt);
			return OK;
		}	

		if (0 == strcmp("mblk2", command))
		{
			M_BLK_ID pMblk = NULL;
			UINT32 errorCnt = 0, freeCnt = 0;
			BOOL mblkFree = TRUE;
			
			my_printf("MBLK 2 Info\n");
			my_printf("index\tmblk\t\tclblk\t\tclbuf\t\tdata\t\tlen\trefcnt\tflag\tstatus\n");
			for (index = 0; index < wireless_MclBlkConfig[RTMP_NETPOOL_SKB_COPY_1].mBlkNum; index++)
			{
				pMblk = (M_BLK_ID)(pMblkBuf2 + (index * (M_BLK_SZ + sizeof(long))) + sizeof(long));
				mblkCnt++;
				
				if (MT_FREE != pMblk->mBlkHdr.mType)
				{
					if (NULL != pMblk->pClBlk)
					{
						my_printf("%d:\t0x%x\t0x%x\t0x%x\t0x%x\t%u\t%d\t0x%x\t%d\n", index, pMblk, 
							(UINT32)pMblk->pClBlk, (UINT32)pMblk->pClBlk->clNode.pClBuf, 
							(UINT32)pMblk->mBlkHdr.mData, pMblk->mBlkHdr.mLen, pMblk->pClBlk->clRefCnt, 
							pMblk->mBlkHdr.mFlags, pMblk->pClBlk->clFreeArg1
							);
					}
					else
					{
						errorCnt++;
					}
				}
				else
				{
					freeCnt++;
				}
			}


			my_printf("MBLK %d FREE %d ERROR %d \n", mblkCnt, freeCnt, errorCnt);
			return OK;
		}	
#endif
		if (0 == strcmp("mblk3", command))
		{
					M_BLK_ID pMblk = NULL;
					UINT32 errorCnt = 0, freeCnt = 0;
					BOOL mblkFree = TRUE;
					
					my_printf("MBLK RX Info\n");
					my_printf("index\tmblk\t\tclblk\t\tclbuf\t\tdata\t\tlen\trefcnt\tflag\tstatus\n");
					for (index = 0; index < wireless_MclBlkConfig[RTMP_NETPOOL_RX].mBlkNum; index++)
					{
						pMblk = (M_BLK_ID)(pMblkBuf3 + (index * (M_BLK_SZ + sizeof(long))) + sizeof(long));
						mblkCnt++;
						
						if (MT_FREE != pMblk->mBlkHdr.mType)
						{
							if (NULL != pMblk->pClBlk)
							{
								my_printf("%d:\t0x%x\t0x%x\t0x%x\t0x%x\t%u\t%d\t0x%x\t%d\n", index, pMblk, 
									(UINT32)pMblk->pClBlk, (UINT32)pMblk->pClBlk->clNode.pClBuf, 
									(UINT32)pMblk->mBlkHdr.mData, pMblk->mBlkHdr.mLen, pMblk->pClBlk->clRefCnt, 
									pMblk->mBlkHdr.mFlags, pMblk->pClBlk->clFreeArg1
									);
							}
							else
							{
								errorCnt++;
							}
						}
						else
						{
							freeCnt++;
						}
					}


					my_printf("MBLK RX %d FREE %d ERROR %d \n", mblkCnt, freeCnt, errorCnt);
					return OK;
				}		
	}

cmdPacketShowHelp:
	printf("\r\n%s", cmdWirelessPacketShowHelp);
	return OK;
}
#endif

