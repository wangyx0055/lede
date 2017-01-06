/***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
            ap_uapsd.c
 
    Abstract:
            Handle WMM Power Saving queues on GO/AP
 
    Revision History:

        Who         When        What
        --------        ----------    ----------------------------------------------
        Rita Yang       2010         initial for P2P WMM-PS
 */

#include "MtConfig.h"

#if UAPSD_AP_SUPPORT

/*
========================================================================
Routine Description:
    Close current Service Period.

Arguments:
    pAd             Pointer to our adapter
    pEntry          Close the SP of the entry

Return Value:
    None

Note:
========================================================================
*/
VOID ApUapsdServicePeriodClose(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN  BOOLEAN             bNoAInterrupt)
{
    INT             i;

    NdisAcquireSpinLock(&pAd->UAPSD.UAPSDEOSPLock);

    //if service period is still in active, we close service period
    if ((pEntry != NULL) && (pEntry->ValidAsCLI == TRUE)  && (pEntry->bAPSDFlagSPStart  == TRUE))
    {

        DBGPRINT(RT_DEBUG_TRACE, ("uapsd> [#%d ] Close Service Period !!! bNoAInterrupt=%d\n", pEntry->Aid, bNoAInterrupt));        
    
        /* re-init SP related parameters */
        if (bNoAInterrupt == FALSE)
        {
            /* clear all  queues in TxSwUapsdQueue[entry]*/
            pEntry->UAPSDTxNum = 0;

            if (pEntry->UAPSDQAid < NUM_OF_UAPSD_CLI)
            {
                for(i=0; i<WMM_NUM_OF_AC; i++)  
                {
                    NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[i]);
                    APCleanupPsQueue(pAd, &pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][i]);
                    NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[i]);
                }
            }

        }
        pEntry->bAPSDFlagSPStart = FALSE;
        
        // for 2870
        pEntry->UAPSDTagOffset[QID_AC_BE] = 0;
        pEntry->UAPSDTagOffset[QID_AC_BK] = 0;
        pEntry->UAPSDTagOffset[QID_AC_VI] = 0;
        pEntry->UAPSDTagOffset[QID_AC_VO] = 0;

    } 

    NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);

}

/*
========================================================================
Routine Description:
    Deliver all queued packets.

Arguments:
    pAd            Pointer to our adapter
    *pEntry        STATION

Return Value:
    None

Note:
    SMP protection by caller for packet enqueue.
========================================================================
*/
VOID ApUapsdAllPacketDeliver(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY     *pEntry)
{
    QUEUE_HEADER *pQueApsd;
    PQUEUE_ENTRY pQueEntry;
    UCHAR QueIdList[WMM_NUM_OF_AC] = { QID_AC_BE, QID_AC_BK,QID_AC_VI, QID_AC_VO };
    INT32 IdAc, QueId; /* must be signed, can not be unsigned */
    ULONG       OptPktNum = 0;  // optimal packet numbers to dequeue

    
    NdisAcquireSpinLock(&pAd->UAPSD.UAPSDEOSPLock);

    /* deliver ALL U-APSD packets from AC3 to AC0 (AC0 to AC3 is also ok) */
    for(IdAc=(WMM_NUM_OF_AC-1); IdAc>=0; IdAc--)
    {
        pQueApsd = &(pEntry->UAPSDQueue[IdAc]);
        QueId = QueIdList[IdAc];

        while(pQueApsd->Head)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("uapsd> Insert UAPSDQueue to TxSwUapsdQueue (Number before= %d)\n",pQueApsd->Number));    
            pQueEntry = RemoveHeadQueue(pQueApsd);
            NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[IdAc]);
            InsertTailQueueAc(pAd, pEntry, &pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QueId], pQueEntry);
            NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[IdAc]);        
        }
    }
    
    NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
} 


/*
========================================================================
Routine Description:
    Parse the UAPSD field in WMM element in (re)association request frame.

Arguments:
    pAd             Pointer to our adapter
    *pEntry         STATION
    *pElm           QoS information field

Return Value:
    None

Note:
    No protection is needed.

    1. Association -> TSPEC:
        use static UAPSD settings in Association
        update UAPSD settings in TSPEC

    2. Association -> TSPEC(11r) -> Reassociation:
        update UAPSD settings in TSPEC
        backup static UAPSD settings in Reassociation

    3. Association -> Reassociation:
        update UAPSD settings in TSPEC
        backup static UAPSD settings in Reassociation
========================================================================
*/
VOID ApUapsdAssocParse(
    IN  PMP_ADAPTER       pAd,
    IN PMP_PORT     pPort,
    IN  MAC_TABLE_ENTRY     *pEntry,
    IN  UCHAR               *pElm)
{
    PQBSS_STA_INFO_PARM  pQosInfo;
    INT     i;
    UCHAR   UAPSDQAid = 0xff;
   
    
    /* check if the station enables UAPSD function */
    if ((pEntry) && (pPort->CommonCfg.bAPSDCapable))
    {
        /* backup its UAPSD parameters */
        pQosInfo = (PQBSS_STA_INFO_PARM) pElm;

        pEntry->MaxSPLength = pQosInfo->MaxSPLength;

        /* use static UAPSD setting of association request frame */
        pEntry->bAPSDCapablePerAC[QID_AC_BE] = pQosInfo->UAPSD_AC_BE;
        pEntry->bAPSDCapablePerAC[QID_AC_BK] = pQosInfo->UAPSD_AC_BK;
        pEntry->bAPSDCapablePerAC[QID_AC_VI] = pQosInfo->UAPSD_AC_VI;
        pEntry->bAPSDCapablePerAC[QID_AC_VO] = pQosInfo->UAPSD_AC_VO;

        pEntry->bAPSDDeliverEnabledPerAC[QID_AC_BE] = pQosInfo->UAPSD_AC_BE;
        pEntry->bAPSDDeliverEnabledPerAC[QID_AC_BK] = pQosInfo->UAPSD_AC_BK;
        pEntry->bAPSDDeliverEnabledPerAC[QID_AC_VI] = pQosInfo->UAPSD_AC_VI;
        pEntry->bAPSDDeliverEnabledPerAC[QID_AC_VO] = pQosInfo->UAPSD_AC_VO;

        if ((pEntry->bAPSDCapablePerAC[QID_AC_BE] == 0) &&
            (pEntry->bAPSDCapablePerAC[QID_AC_BK] == 0) &&
            (pEntry->bAPSDCapablePerAC[QID_AC_VI] == 0) &&
            (pEntry->bAPSDCapablePerAC[QID_AC_VO] == 0))
        {
            CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE);
        }
        else if (pAd->UAPSD.UapsdSWQBitmap > ((1<< NUM_OF_UAPSD_CLI) -1))
        {
            /* Currently, we only support 2 clients to access TxSwUapsdQueue at the same time */            
            CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE);
            DBGPRINT(RT_DEBUG_TRACE, ("assoc uapsd> Entry Queue is not enough. So disable client apsd (pAd->UAPSD.UapsdSWQBitmap=%d)\n", pAd->UAPSD.UapsdSWQBitmap));               
        }
        else
        {
            // choose an idex for this new client.
            for (i=0; i<NUM_OF_UAPSD_CLI; i++)
            {
                if ((pAd->UAPSD.UapsdSWQBitmap) & (1 << i))
                    continue;
                
                UAPSDQAid = (UCHAR) i;
                break;
            }
            
            if (UAPSDQAid == 0xff)
            {
                CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE);
                DBGPRINT(RT_DEBUG_TRACE, ("assoc uapsd> UAPSDQ Entries are not enough ~~~ So disable uapsd\n"));
            }
            else
            {
                CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE);
                pEntry->UAPSDQAid = UAPSDQAid;
                pAd->UAPSD.UapsdSWQBitmap |= (1 << pEntry->UAPSDQAid);
                DBGPRINT(RT_DEBUG_TRACE, ("assoc uapsd> Client with apsd capable JOIN (pEntry->UAPSDQAid=%d, pAd->UAPSD.UapsdSWQBitmap=%d)\n", pEntry->UAPSDQAid, pAd->UAPSD.UapsdSWQBitmap));          
            }
        }


        if (  UAPSD_MR_IS_ENTRY_ACCESSIBLE(pEntry) &&
            (pEntry->bAPSDCapablePerAC[QID_AC_BE] == 1) &&
            (pEntry->bAPSDCapablePerAC[QID_AC_BK] == 1) &&
            (pEntry->bAPSDCapablePerAC[QID_AC_VI] == 1) &&
            (pEntry->bAPSDCapablePerAC[QID_AC_VO] == 1))
        {
            /* all AC are U-APSD */
            DBGPRINT(RT_DEBUG_TRACE, ("assoc uapsd> all AC are UAPSD\n"));
            pEntry->bAPSDAllAC = 1;
        }
        else
        {
            /* at least one AC is not U-APSD or the entry is not  accessible*/
            DBGPRINT(RT_DEBUG_TRACE, ("assoc uapsd> at least one AC is not UAPSD %d -%d -%d -%d\n",
                    pEntry->bAPSDCapablePerAC[QID_AC_BE],
                    pEntry->bAPSDCapablePerAC[QID_AC_BK],
                    pEntry->bAPSDCapablePerAC[QID_AC_VI],
                    pEntry->bAPSDCapablePerAC[QID_AC_VO]));
            pEntry->bAPSDAllAC = 0;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("assoc uapsd> MaxSPLength = %d\n", pEntry->MaxSPLength));
    }
}


/*
========================================================================
Routine Description:
    Enqueue a UAPSD packet.

Arguments:
    pAd             Pointer to our adapter
    *pEntry         STATION
    NetBuffer           UAPSD dnlink packet
    IdAc            UAPSD AC ID (0 ~ 3)

Return Value:
    None

Note:
========================================================================
*/
NDIS_STATUS 
ApUapsdPacketEnqueue(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY     *pEntry,
    IN  PMT_XMIT_CTRL_UNIT             pXcu,
    IN  UINT32              IdAc
    )
{
    /*
        1. the STATION is UAPSD STATION;
        2. AC ID is legal;
        3. the AC is UAPSD AC.
        so we queue the packet to its UAPSD queue
    */

    /* [0] ~ [3], QueIdx base is QID_AC_BE */
    QUEUE_HEADER *pQueUapsd;

    /* check if current queued UAPSD packet number is too much */
    if (pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("uapsd> pEntry == NULL!\n"));
        return NDIS_STATUS_FAILURE;
    } 

    pQueUapsd = &(pEntry->UAPSDQueue[IdAc]);

    DBGPRINT(RT_DEBUG_TRACE, ("uapsd> ---------------------------------------------> start ApUapsdPacketEnqueue\n"));     

    if (pQueUapsd->Number >= MAX_PACKETS_IN_UAPSD_QUEUE)
    {
        /* too much queued pkts, free (discard) the tx packet */
        DBGPRINT(RT_DEBUG_TRACE, ("uapsd> many(%ld) WCID(%d) AC(%d)\n", pQueUapsd->Number, pXcu->Wcid, IdAc));
        return NDIS_STATUS_FAILURE;
    }
    else
    {
        /* queue the tx packet to the U-APSD queue of the AC */
        NdisAcquireSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
        InsertTailQueue(pQueUapsd, pXcu);
        NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);   

        DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==>Insert ac ps Q#%d, pQueUapsd->Number(=%d)\n", __FUNCTION__, __LINE__, IdAc, pQueUapsd->Number));

        #if 0 
        if (RTMP_GET_PACKET_MGMT_PKT(pPacket) == 1)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ps> mgmt to uapsd queue...\n"));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE,
                ("ps> data (0x%08lx) (AC%d) to uapsd queue (num of pkt = %ld)...\n",
                (ULONG)pPacket, IdAc,
                pQueUapsd->Number));
        }
        #endif 

    }
    DBGPRINT(RT_DEBUG_TRACE, ("uapsd> <--------------------------------------------- end ApUapsdPacketEnqueue\n"));

    return NDIS_STATUS_SUCCESS;
    
}

/*
========================================================================
Routine Description:
    Maintenance our UAPSD PS queue.  Release all queued packet if timeout.

Arguments:
    pAd             Pointer to our adapter
    *pEntry         STATION

Return Value:
    None

Note:
    If in RT2870, pEntry can not be removed during ApUapsdQueueMaintenance()
========================================================================
*/
VOID ApUapsdQueueMaintenance(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY     *pEntry)
{
    QUEUE_HEADER *pQue, *pSwQue;
    PMP_PORT            pPort = NULL;
    ULONG IdAc;
    UCHAR FlgUapsdPkt = 0, FlgSWQUapsdPkt = 0;
    USHORT IdleCount;

    pPort = pEntry->pPort;

//  if (pEntry->PsMode == PWR_SAVE)
    {
        /* UAPSD packet only for power-save STA, not active STA */

        /* init */
        NdisAcquireSpinLock(&pAd->UAPSD.UAPSDEOSPLock);

        pQue = pEntry->UAPSDQueue;
        /* check if more than one U-APSD packets exists */
        for(IdAc=0; IdAc<WMM_NUM_OF_AC; IdAc++)
        {
            if (pQue[IdAc].Head != NULL)
            {
                //At least one U-APSD packets exists so we need to check if queued U-APSD packets are timeout.
                FlgUapsdPkt = 1;
                break;
            }
        }

        if (pEntry->UAPSDQAid < NUM_OF_UAPSD_CLI)
        {
            pSwQue = pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid];
            /* check if more than one U-APSD-TxSWQue packets exists */
            for(IdAc=0; IdAc<WMM_NUM_OF_AC; IdAc++)
            {
                if (pSwQue[IdAc].Head != NULL)
                {
                    //At least one U-APSD-TxSWQue packets exists so we need to check if queued U-APSD packets are timeout.
                    FlgSWQUapsdPkt = 1;
                    break;
                }
            }
        }

    //  IdleCount = UAPSD_WIFI_IDLE_TIME;       /* UAPSD_WIFI_IDLE_TIME = 15 seconds for WiFi WMM test */
        IdleCount = UAPSD_IDLE_TIME;    /* normal timeout value */

        if (FlgUapsdPkt || FlgSWQUapsdPkt)
        {
            pEntry->UAPSDQIdleCount ++;

            if (pEntry->UAPSDQIdleCount >=  IdleCount ) 
            {

                DBGPRINT(RT_DEBUG_TRACE, ("uapsd> UAPSD queue timeout! clean all queued frames...\n"));

                //UAPSDQIdleCount will be 0 after trigger frame is received
                // clear all U-APSD packets
                ApUapsdCleanupUapsdQueue(pAd, pEntry, FALSE);

                /* clear idle counter */
                pEntry->UAPSDQIdleCount = 0;
            }
        }
        else
        {
            /* clear idle counter */
            pEntry->UAPSDQIdleCount = 0;
        }

        NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);

    }
    
    #if 1
    /* check TIM bit */
    //Clear TIM Case 1: When AC are not all UAPSD, we must check Legacy Queue size to decide if we need to clear TIM.  
    if((pEntry->bAPSDAllAC == FALSE) && (pEntry->PsQueue.Number == 0))
    {
        DBGPRINT(RT_DEBUG_TRACE,("clear  client#%d TIM in ApUapsdQueueMaintenance :: UAPSD Case ALL AC are not UAPSD::\n",pEntry->Aid)); 
         if (pPort)
         {
            // clear corresponding TIM bit
            if (pEntry->Aid >= 32)
                pPort->SoftAP.ApCfg.TimBitmap2 &= (~TIM_BITMAP[pEntry->Aid-32]); 
            else
                pPort->SoftAP.ApCfg.TimBitmap &= (~TIM_BITMAP[pEntry->Aid]);
        }
    }
    //Clear TIM Case 2: When ALL AC are UAPSD and all UAPSD Queue are empty, we must clear TIM.
    else if ((pEntry->bAPSDAllAC == TRUE) && 
        (pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_BE].Number==0) &&
        (pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_BK].Number==0) &&
        (pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_VI].Number==0) &&
        (pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_VO].Number==0))
    {
        DBGPRINT(RT_DEBUG_TRACE,("clear  client#%d TIM in ApMlmePeriodicExec :: UAPSD Case ALL AC are UAPSD::\n",pEntry->Aid)); 

        /* all AC are U-APSD and no any U-APSD packet is queued, set TIM */
        /* clear TIM bit */
        if (pEntry->Aid >= 32 && (pEntry->PsQueue.Number == 0))
        {
            if (pPort)
                pPort->SoftAP.ApCfg.TimBitmap2 &= (~TIM_BITMAP[pEntry->Aid-32]); 
        }
        else if (pEntry->PsQueue.Number == 0)
        {
             if (pPort)
                pPort->SoftAP.ApCfg.TimBitmap &= (~TIM_BITMAP[pEntry->Aid]);
        }
    }
    #endif
    
} 

/*
========================================================================
Routine Description:
    Handle UAPSD Trigger Frame.

Arguments:
    pAd             Pointer to our adapter
    *pEntry         the source STATION
    UpOfFrame       the UP of the trigger frame

Return Value:
    None

Note:
========================================================================
*/

/*  WMM Specification V1.1 3.6.2.4

To initiate an USP to retrieve MSDUs and MMPDUs from delivery-enabled ACs, a WMM STA
shall wake up and transmit a Trigger Frame to the WMM AP. . Frames of delivery-enabled
ACs are delivered during a USP that begins after the WMM AP acknowledges the trigger frame
transmitted by the WMM STA. The WMM STA shall remain awake until it receives a QoS
Data or Null frame addressed to it, with the EOSP subfield in QoS Control field set to 1.

*/

VOID ApUapsdTriggerFrameHandle(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  MAC_TABLE_ENTRY     *pEntry,
    IN  UCHAR               UpOfFrame)
{
    QUEUE_HEADER    *pAcPsQue = NULL;
    QUEUE_HEADER    *pAcSwQue, *pLastAcSwQue;
    PQUEUE_ENTRY    pQuedEntry;
    PMT_XMIT_CTRL_UNIT    pXcu = NULL;

    UINT32      AcQueId;
    UINT32      TxPktNum, SpMaxLen;
    /* AC ID          = VO > VI > BK > BE */
    /* so we need to change BE & BK */
    /* => AC priority = VO > VI > BE > BK */
    UINT32      AcPriority[WMM_NUM_OF_AC] = { 1, 0, 2, 3 };
    UINT32      SpLenMap[4] = { 0, 2, 4, 6 }; /* 0: deliver all U-APSD packets */
    UCHAR       QueIdList[WMM_NUM_OF_AC] = { QID_AC_BE, QID_AC_BK,
                                        QID_AC_VI, QID_AC_VO };
    BOOLEAN     FlgQueEmpty;
    BOOLEAN     FlgNullSnd;
    BOOLEAN     FlgMgmtFrame;
    BOOLEAN     FlgDequeueOnly;
    BOOLEAN     FlgDequeueNeed;
    USHORT      Aid, QueId;
    INT32       IdAc; /* must be signed, can not use unsigned */
//  ULONG    FlgIrq;
    BOOLEAN     bMoreData = FALSE;
    ULONG   RemainUapsdTxNum = 0;
    
    /* sanity check for Service Period of the STATION */
    NdisAcquireSpinLock(&pAd->UAPSD.UAPSDEOSPLock);

    /* WMM Specification V1.1 3.6.1.5
            1. A Trigger Frame received by the WMM AP from a WMM STA that
            already has an USP underway shall not trigger the start of a new USP.
        
        2. Current SP for the STATION is not yet ended so the packet is
            normal DATA packet. */  
    
    if (pEntry->bAPSDFlagSPStart == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("uapsd> sorry! SP is not yet closed!\n"));        
        NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
        return;
    }
    

    if (pEntry->UAPSDQAid >= NUM_OF_UAPSD_CLI)
    {
        DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==> UAPSDQAid is invalid. \n", __FUNCTION__, __LINE__));
        NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
        return;
    }


    if (pEntry->MaxSPLength >= 4)
    {
        //fatal error, MaxSPLength should be 0 ~ 3 so reset it to 0
        pEntry->MaxSPLength = 0;
    }

    pEntry->bAPSDFlagSpRoughUse = FALSE;
    
    // sanity Check for UAPSD condition
    if (UpOfFrame >= 8)
        UpOfFrame = 1;


    /* get the AC ID of incoming packet by mapping UP of this frame to AC
    ---------------------------------------------------------
        UP      ----->  AC
        0           AC_BE
        1           AC_BK
        2           AC_BK
        3           AC_BE
        4           AC_VI
        5           AC_VI
        6           AC_VO
        7           AC_VO
    ---------------------------------------------------------*/                                     
    AcQueId = MapUserPriorityToAccessCategory[UpOfFrame];

    // check whether the AC is trigger-enabled AC
    if (pEntry->bAPSDCapablePerAC[AcQueId] == 0)
    {
        /*
            WMM Specification V1.1 Page 4
            Trigger Frame: A QoS Data or QoS Null frame from a WMM STA in
            Power Save Mode associated with an AC the WMM STA has configured
            to be a trigger-enabled AC.

            A QoS Data or QoS Null frame that indicates transition to/from
            Power Save Mode is not considered to be a Trigger Frame and the
            AP shall not respond with a QoS Null frame.
        */

        /*
            ERROR! the AC does not belong to a trigger-enabled AC or
            the ACM of the AC is set.
        */
        DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==> <<<<<<<<<<<< AcQueId=%d  is not trigger enabled AC\n", __FUNCTION__, __LINE__, AcQueId));
        NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==> <<<<<<<<<<<< AcQueId=%d is trigger-enabled AC \n", __FUNCTION__, __LINE__, AcQueId));

    /* enqueue U-APSD packets to AC software queues */

    /* init */
    FlgQueEmpty = TRUE;
    TxPktNum = 0;
    SpMaxLen = SpLenMap[pEntry->MaxSPLength];
    pAcSwQue = NULL;
    pLastAcSwQue = NULL;
    FlgMgmtFrame = FALSE;
    FlgDequeueOnly = FALSE;
    FlgDequeueNeed = FALSE;
    FlgNullSnd = FALSE; 
    /* from highest priority AC3 --> AC2 --> AC0 --> lowest priority AC1 */
    pQuedEntry = NULL;

    /* Get remaining packet in tx software queues */
    RemainUapsdTxNum = pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_BK].Number 
                    + pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_BE].Number 
                    + pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_VI].Number 
                    + pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_VO].Number;

    if (pPort->P2PCfg.GONoASchedule.bValid == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==> RemainUapsdTxNum=%d, SpMaxLen=%d \n", __FUNCTION__, __LINE__, RemainUapsdTxNum, SpMaxLen ));

        if (SpMaxLen != 0)
        {
            if (RemainUapsdTxNum >= SpMaxLen)
            {
                DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==> TxSw Queue is full, jump to dequeue pkts\n",  __FUNCTION__, __LINE__));
                FlgDequeueOnly = TRUE;
            }
            else
            {
                SpMaxLen -= RemainUapsdTxNum;
            }
        }
        else if (RemainUapsdTxNum > 0)
        {
            FlgDequeueNeed = TRUE;
        }
    }
    

    if (FlgDequeueOnly == FALSE)
    {
        for (IdAc=(WMM_NUM_OF_AC-1); IdAc>=0; IdAc--)
        {
            AcQueId = AcPriority[IdAc];

            /* check if the AC is delivery-enable AC */
            if (pEntry->bAPSDDeliverEnabledPerAC[AcQueId] == 0)
            {
                DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==>  AcQueId=%d is not Deliver Enabled\n", __FUNCTION__, __LINE__, AcQueId));
                continue;
            }

            /*
                NOTE: get U-APSD queue pointer here to speed up, do NOT use
                pEntry->UAPSDQueue[AcQueId] throughout codes because
                compiler will compile it to many assembly codes.
            */
            pAcPsQue = &pEntry->UAPSDQueue[AcQueId];

            /* check if any U-APSD packet is queued for the AC */
            if (pAcPsQue->Head == NULL)
            {
                DBGPRINT(RT_DEBUG_INFO,("Chk::%s(%d)==>  AcQueId=%d queue is NULL, continue\n", __FUNCTION__, __LINE__, AcQueId));
                continue;
            }
            /* at least one U-APSD packet exists here */

            /* get AC software queue, equal to AcQueId */
            QueId = QueIdList[AcQueId];
            pAcSwQue = &pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QueId];
            DBGPRINT(RT_DEBUG_TRACE, ("uapsd> AcQueId#%d  -- Total UAPSDQueue Number = %d\n", AcQueId, pAcPsQue->Number));

            /* put U-APSD packets to the AC software queue */
            while(pAcPsQue->Head)
            {
                /* check if Max SP Length != 0 */
                if (SpMaxLen != 0)
                {
                    /*
                        WMM Specification V1.1 3.6.1.7
                        At each USP for a WMM STA, the WMM AP shall attempt to
                        transmit at least one MSDU or MMPDU, but no more than the
                        value encoded in the Max SP Length field in the QoS Info
                        Field of a WMM Information Element from delivery-enabled
                        ACs, that are destined for the WMM STA.
                    */
                    if (TxPktNum >= SpMaxLen)
                    {
                        /*
                            Some queued U-APSD packets still exists so we will
                            not clear MoreData bit of the packet.
                        */
                        FlgQueEmpty = FALSE;
                        break;
                    } 
                }

                /* count U-APSD packet number */
                TxPktNum ++;

                /* queue last U-APSD packet */
                if (pQuedEntry != NULL)
                {
                    /* enqueue U-APSD packet to transmission software queue */

                    /*
                        WMM Specification V1.1 3.6.1.7
                        Each buffered frame shall be delivered using the access
                        parameters of its AC.
                    */
                    DBGPRINT(RT_DEBUG_TRACE, ("uapsd> 2. Insert UAPSDQueue to TxSwUapsdQueue (remaining Number = %d)\n",pAcPsQue->Number)); 
                    NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[QueId]);
                    InsertTailQueueAc(pAd, pEntry, pLastAcSwQue, pQuedEntry);
                    NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[QueId]);

                }

                /* get the U-APSD packet */
                pQuedEntry = RemoveHeadQueue(pAcPsQue);         

                pXcu = (PMT_XMIT_CTRL_UNIT)pQuedEntry; 

                //if (RTMP_GET_PACKET_MGMT_PKT(pQuedPkt) == 1)
                //  FlgMgmtFrame = 1;
                

                if (pXcu!= NULL)
                {
                    /*
                        WMM Specification V1.1 3.6.1.7
                        The More Data bit (b13) of the directed MSDU or MMPDU
                        associated with delivery-enabled ACs and destined for
                        that WMM STA indicates that more frames are buffered for
                        the delivery-enabled ACs.
                    */
                    bMoreData = TRUE;
                    pXcu->bMoreData = TRUE;

                    /* set U-APSD flag & its software queue ID */
                    pXcu->bUAPSD = TRUE;
                    pXcu->QueIdx = (UCHAR)QueId;
                }

                /* backup its software queue pointer */
                pLastAcSwQue = pAcSwQue;
            }/* End of while */ 

            if (FlgQueEmpty == FALSE)
            {
                /* FlgQueEmpty will be FALSE only when TxPktNum >= SpMaxLen */
                break;
            }
        } 
        //=================================================>>>>  /* End of for (IdAc)*/
        
        /*
            For any mamagement UAPSD frame, we use DMA to do SP check
            because no any FIFO statistics for management frame.
        */
        //if (FlgMgmtFrame)
        //  pEntry->bAPSDFlagSpRoughUse = TRUE;


        /*
            No need to protect EOSP handle code because we will be here
            only when last SP is ended.
        */
        FlgNullSnd = FALSE;

        if (TxPktNum >= 1)
        {
            if (FlgQueEmpty == TRUE)
            {
                /*
                    No any more queued U-APSD packet so clear More Data bit of
                    the last frame.
                */
                if(pXcu != NULL)
                {
                    bMoreData = FALSE;
                    pXcu->bMoreData = FALSE;
                    //DBGPRINT(RT_DEBUG_TRACE, ("uapsd> TEST::Last packet  get packet more data field=0 \n"));
                }
            } 
        } 


        /* Refer WMM Spec. Table 4 
            The number of delivered frame is 2, 4,  6 , or all.
            If TxPktNum to transmit is less than one, it mean no data buffered in UAPSDQueue.
            Refer WMM Spec. 3.6.1.7 
            If the WMM AP has no buffered MSDU or MMPDU to transmit or needs to otherwise terminate a
            USP, the WMM AP may generate an extra QoS Null frame, with the EOSP set to 1.   
        */

        if (TxPktNum <= 1)
        {

            if (TxPktNum <= 0)
            {
                /* if no data needs to tx, respond with QosNull for the trigger frame */

                FlgNullSnd = TRUE;  
                pEntry->UAPSDTxNum = 0;
                
                /*
                    We will send the QoS Null frame below.
                */
            }
            else
            {
                /* only one packet so send it directly */
                if((pXcu != NULL) && (pQuedEntry != NULL))
                {
                    UCHAR   tmpIdx = 0;

                    if (pXcu->QueIdx< WMM_NUM_OF_AC)
                        tmpIdx = pXcu->QueIdx;
                    
                    pXcu->bEosp = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("uapsd> <<<<<<<<<<<< 1. only one packet so send it directly, set EOPS flag (QId=%d)\n", tmpIdx));
                    DBGPRINT(RT_DEBUG_TRACE, ("uapsd> <<<<<<<<<<<< 2. Insert it to TxSwUapsdQueue\n"));
                    NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[tmpIdx]);
                    InsertTailQueueAc(pAd, pEntry, pLastAcSwQue, pQuedEntry);
                    NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[tmpIdx]);
                    pEntry->UAPSDTxNum = 1;

                }
            }   
        }
        else
        {
            /* more than two U-APSD packets */

            /*
                NOTE: EOSP bit !=(!MoreData) bit because Max SP Length,
                we can not use MoreData bit to decide EOSP bit.
            */

            /*
                EOSP frame:
                we will re-check the EOSP frame in HardTransmit().
            */

            // TODO: P2P NoA + Max SP Length, might be an issue
            
            if(pXcu!= NULL)
            {
                pXcu->bEosp = TRUE;
            }
            if(pEntry!=NULL)
            {
                /* include the EOSP frame */
                pEntry->UAPSDTxNum = (USHORT)TxPktNum; 
            }

            if((pXcu!= NULL) && (pEntry!=NULL))
            {
                if (pXcu->bEosp == TRUE)
                {
                    UCHAR   tmpIdx = 0;

                    if (pXcu->QueIdx < WMM_NUM_OF_AC)
                        tmpIdx = pXcu->QueIdx;

                    DBGPRINT(RT_DEBUG_TRACE, ("uapsd> 2. Insert the last packet(EOSP) to TxSwUapsdQueue\n"));
                    NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[tmpIdx]);
                    InsertTailQueueAc(pAd, pEntry, pLastAcSwQue, pQuedEntry);
                    NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[tmpIdx]);
                }
            }


        } 
    }

    /* clear corresponding TIM bit */
    #if 1
    /* get its AID for the station */
    Aid = pEntry->Aid;
    if ((pPort->P2PCfg.GONoASchedule.bValid == FALSE) && (pEntry->bAPSDAllAC == TRUE) && (FlgQueEmpty == TRUE))
    {
        /* all AC are U-APSD and no any U-APSD packet is queued, set TIM */
        /* clear TIM bit */
        if ((Aid > 0) && (Aid < MAX_LEN_OF_MAC_TABLE))
        {
            if (Aid >= 32)
                pPort->SoftAP.ApCfg.TimBitmap2 &= (~TIM_BITMAP[Aid-32]); 
            else
                pPort->SoftAP.ApCfg.TimBitmap &= (~TIM_BITMAP[Aid]);

            DBGPRINT(RT_DEBUG_TRACE, ("trigger frame >>>after clear TIM at recv trigger frame, Current bit map=%d, sent packet to ps-save client\n", pPort->SoftAP.ApCfg.TimBitmap));
            
        } 
    }
    #endif

    /* Use DMA to do SP check because no any FIFO statistics for management frame. */ 
    if (FlgNullSnd &&  (!FlgDequeueNeed)) 
        pEntry->bAPSDFlagSpRoughUse = TRUE; 

    /* reset idle timeout here whenever a trigger frame is received */
    pEntry->UAPSDQIdleCount = 0;

    /* set the SP start flag and then allow to dequeue */
    pEntry->bAPSDFlagSPStart = TRUE; 

    NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);


    /* check if NULL Frame is needed to be transmitted */
    if (FlgNullSnd &&  (!FlgDequeueNeed))
    {
        // if not in NoA absent, send a null frame.
        if ((pPort->P2PCfg.GONoASchedule.bValid == FALSE) || (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == FALSE))
        {
            /* bQosNull = bEOSP = TRUE = 1 */
            DBGPRINT(RT_DEBUG_TRACE, ("uapsd > UAPSD Queue is Emtpry, go to ApUapsdSendNullFrame\n"));
            ApUapsdSendNullFrame(pAd, pPort, pEntry->Addr, pEntry->CurrTxRate, TRUE, TRUE, UpOfFrame);

            /* Stop SP directly, not use DMA done*/
            if (pEntry->bAPSDFlagSpRoughUse == TRUE)
                ApUapsdServicePeriodClose(pAd, pEntry, FALSE);
        }   
        return;
    } 

    
    /* Dequeue outgoing frames from TxSwUapsdQueue0..3 queue and process it */
    DBGPRINT(RT_DEBUG_TRACE,("uapsd > ApUapsdTriggerFrameHandle ------------------------->Start to dequeue pkts (#%d)\n", pEntry->Aid));
    /* Dequeue from highest priority AC3, AC2, AC0 and then lowest priority AC1 */
    for (IdAc=(WMM_NUM_OF_AC-1); IdAc>=0; IdAc--)
    {
        ApUapsdDeQueueUAPSDPacket(pAd, pPort, (UCHAR)AcPriority[IdAc], pEntry);
    }
    N6USBKickBulkOut(pAd);

} 

/*
    Reset MAC entry UAPSD parameters;
    purpose: clean all UAPSD PS queue; release the EOSP frame if exists;
            reset control parameters
*/
VOID ApUapsdCleanupUapsdQueue(
    IN  PMP_ADAPTER       pAd,
    IN  PMAC_TABLE_ENTRY    pMacEntry,
    IN  BOOLEAN             bResetAll)                              
{                                       
        UINT32              IdAc, i;                                                        

        DBGPRINT(RT_DEBUG_TRACE, ("clean uapsd> --------------------------------------------->  start ApUapsdCleanupUapsdQueue\n")); 
                                                
        /* clear all U-APSD queues, where these queues are from the upper layer*/                                   
        for(IdAc=0; IdAc<WMM_NUM_OF_AC; IdAc++)                     
            APCleanupPsQueue(pAd, &pMacEntry->UAPSDQueue[IdAc]);
        
        DBGPRINT(RT_DEBUG_TRACE, ("clean uapsd>size of UAPSDQueue= %d-%d-%d-%d\n", 
            pMacEntry->UAPSDQueue[0].Number, 
            pMacEntry->UAPSDQueue[1].Number, 
            pMacEntry->UAPSDQueue[2].Number, 
            pMacEntry->UAPSDQueue[3].Number));              

        /* clear UAPSD TxCount  */
        pMacEntry->UAPSDTxNum = 0;                                                                                              

        pMacEntry->bAPSDFlagSPStart = FALSE;                                        

        if (bResetAll)  //  clear all  queues in TxSwUapsdQueue[i]
        {
            DBGPRINT(RT_DEBUG_TRACE, ("clean uapsd>#%d  Reset all UAPSD SWQ (max NUM_OF_UAPSD_CLI=%d)\n", pMacEntry->Aid, NUM_OF_UAPSD_CLI));
            for (i=0; i<NUM_OF_UAPSD_CLI; i++)
            {
                for(IdAc=0; IdAc<WMM_NUM_OF_AC; IdAc++)
                {
                    NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[IdAc]);
                    APCleanupPsQueue(pAd, &pAd->UAPSD.TxSwUapsdQueue[i][IdAc]);
                    NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[IdAc]);
                }
            }
        }
        else                // clear  queues in TxSwUapsdQueue[UAPSDQAid] 
        {   
            if (pMacEntry->UAPSDQAid < NUM_OF_UAPSD_CLI)
            {
                for(IdAc=0; IdAc<WMM_NUM_OF_AC; IdAc++) 
                {
                    NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[IdAc]);
                    APCleanupPsQueue(pAd, &pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][IdAc]);
                    NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[IdAc]);
                }
                DBGPRINT(RT_DEBUG_TRACE, ("clean uapsd>size of TxSwUapsdQueue= %d-%d-%d-%d\n", 
                    pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][0].Number, 
                    pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][1].Number, 
                    pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][2].Number, 
                    pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][3].Number));              
                
            }
        }

        DBGPRINT(RT_DEBUG_TRACE, ("clean uapsd> <---------------------------------------------  end ApUapsdCleanupUapsdQueue\n"));   


}

/*  
    ==========================================================================
    Description: 
        Calculate a appropriate length to send to dequeue engine during a P2P NoA presence period while WMM-PS is ON.
        
    Parameters: 
        pAd         Pointer to our adapter
        pMacEntry   Client Entry
    Return:
        The length of packets in byte for the next dequeue process
    ==========================================================================
 */
ULONG ApUapsdGetNextPktNum(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT            pPort,
    IN  PMAC_TABLE_ENTRY    pMacEntry)                              
{

    ULONG   Value, GPDiff = 0;
    ULONG   PktNum = MAX_TX_PROCESS;
    UCHAR   DefaultPktNumInNoA = 1;// one packet dequeue

    if (pMacEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, (" uapsd> ApUapsdGetNextPktNum=0 due to null entry\n"));
        return 0;
    }

    // IN P2P NoA schedule
    if (pPort->P2PCfg.GONoASchedule.bValid == TRUE)
    {
        // If WMM-PS client is in conjunction with NoA, 
        // dequeue one packet each time and do the next queue once received ack.
        //
        // get difference time
        if(KeGetCurrentIrql() != PASSIVE_LEVEL)
            Value = 0;
        else
            RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &Value);
        
        GPDiff = pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting - Value;
    
        if ((pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE) 
            || (pPort->P2PCfg.bKeepSlient == TRUE)) // sleeping
        {
            PktNum = 0;
        }   
        else if (GPDiff > WMM_USP_STOP_PRIOR_TO_NOA_MARGIN) // awake
        {
            PktNum = DefaultPktNumInNoA;
            DBGPRINT(RT_DEBUG_TRACE, (" uapsd> ApUapsdGetNextPktNum=%d within NoA Schedule\n", PktNum));           
        }
        else        // in the margin of WMM_USP_STOP_PRIOR_TO_NOA_MARGIN
        {
            PktNum = 0;
        }

    }
    else        // NoA OFF
    {
        PktNum = MAX_TX_PROCESS;
    }

    return PktNum;
    
}

VOID    ApUapsdDeQueueUAPSDPacket(
    IN  PMP_ADAPTER   pAd,  
    IN  PMP_PORT        pPort,
    IN  UCHAR           BulkOutPipeId,
    PMAC_TABLE_ENTRY    pMacEntry)
{
    PQUEUE_ENTRY    pEntry;
    UCHAR           FragmentRequired;
    NDIS_STATUS     ndisStatus;
    UCHAR           Count = 0;
    PQUEUE_HEADER   pQueue;
    UCHAR           QueIdx;
    UCHAR           OpMode = OPMODE_STA;
    UCHAR           PortNumber = 0;
    //PMP_PORT      pPort;
    ULONG           PktNum = 0;
    PMT_XMIT_CTRL_UNIT    pXcu = NULL;

    if ((NdisCommonFreeDescriptorRequest(pAd, TX_RING, BulkOutPipeId, 1) != NDIS_STATUS_SUCCESS)) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("--ApUapsdDeQueueUAPSDPacket %d queue full !!  early return\n", BulkOutPipeId));
        return;
    }

    //Calculate a appropriate length to send to dequeue engine. Default value is MAX_TX_PROCESS.
    PktNum = ApUapsdGetNextPktNum(pAd,  pPort, pMacEntry);
    if (PktNum == 0)
        return;
    
    NdisAcquireSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
    if (pAd->pTxCfg->DeQueueRunning[BulkOutPipeId])
    {
        NdisReleaseSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
        return;
    }
    else
    {
        pAd->pTxCfg->DeQueueRunning[BulkOutPipeId] = TRUE;
        NdisReleaseSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
    }

    QueIdx = BulkOutPipeId;

    // Make sure SendTxWait queue resource won't be used by other threads
    NdisAcquireSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[BulkOutPipeId]);
    // Select Queue
    pQueue = &pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][BulkOutPipeId];

    pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

    // Check queue before dequeue
    while ((pQueue->Head != NULL) && (Count < MAX_TX_PROCESS))
    {
        // Reset is in progress, stop immediately
        if (IS_P2P_GO_OP(pPort))
        {
            if ((pAd->HwCfg.LatchRfRegs.Channel  != pPort->CentralChannel)    ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))              ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))      ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))       ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                || (IS_P2P_ABSENCE(pPort)) 
                || (pPort->P2PCfg.P2PChannelState == P2P_ENTER_GOTOSCAN) 
                || ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE)))
            {
                DBGPRINT(RT_DEBUG_ERROR,("--ApUapsdDeQueueUAPSDPacket %d reset-in-progress pAd->flags = %x!!--\n", BulkOutPipeId,pAd->Flags));
                DBGPRINT(RT_DEBUG_ERROR,("%d  %d  %d %d !!--\n", pPort->P2PCfg.bPreKeepSlient, pPort->P2PCfg.bKeepSlient, pPort->P2PCfg.GONoASchedule.bValid, pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent));
                break;
            }
        }
        else if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        {
            break;
        }

        // Not Support RaTxAggregation in WMM-PS

        // Dequeue the first entry from head of queue list
        pEntry = RemoveHeadQueue(pQueue);

        // Retrieve Ndis NET BUFFER pointer from MiniportReserved field
        pXcu = (PMT_XMIT_CTRL_UNIT)pEntry;
        
        //DBGPRINT(RT_DEBUG_INFO, ("DeQueue: NetBufer %p List %p, Total=%d\n", NetBuffer, MT_GET_NETBUFFER_LIST(NetBuffer), NET_BUFFER_DATA_LENGTH(NetBuffer)));
        
        // RTS or CTS-to-self for B/G protection mode has been set already.
        // There is no need to re-do it here. 
        // Total fragment required = number of fragment + RST if required
        FragmentRequired = pXcu->FragmentRequired;

        // Suppport RaTxAggregation
        // Get next NDIS Packet from SWQ
        OpMode = OPMODE_STA;
        OpMode = pXcu->OpMode;
        PortNumber = (UCHAR)pXcu->PortNumber;

        if ((NdisCommonFreeDescriptorRequest(pAd, TX_RING, BulkOutPipeId, FragmentRequired) == NDIS_STATUS_SUCCESS)) 
        {
            // Avaliable ring descriptors are enough for this frame
            // Call hard transmit
            // Nitro mode / Normal mode selection

            // only for AP pkt
            if (OpMode == OPMODE_AP )
                ndisStatus = ApDataHardTransmit(pAd, pXcu);
            else
            {
                DBGPRINT(RT_DEBUG_ERROR,("%s ==> Unknown OpMode [%d]\n", __FUNCTION__,OpMode)); 
                ndisStatus = NDIS_STATUS_FAILURE;
            }

            // Make sure TxSwUapsd queue resource won't be used by other threads
            if (ndisStatus == NDIS_STATUS_FAILURE)
            {
                if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
                {
                    RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_FAILURE);
                }
                break;
            }
            else if (ndisStatus == NDIS_STATUS_RESOURCES)
            {
                // Not enough free tx ring, it might happen due to free descriptor inquery might be not correct
                // It also might change to NDIS_STATUS_FAILURE to simply drop the frame
                // Put the frame back into head of queue
                //DBGPRINT(RT_DEBUG_ERROR,("RESOURSE NOT ENOUGH BulkOutPipeId = %d, NetBuffer = %x\n", BulkOutPipeId, NetBuffer));               
                InsertHeadQueue(pQueue, pXcu);
                break;
            }
            else if (ndisStatus == NDIS_STATUS_ADAPTER_NOT_READY)
            {
                // If fRTMP_ADAPTER_ADDKEY_IN_PROGRESS happens, we blocks packets except EAPOL frame.
                // We must put it into TailQueue because it is possible that EAPOL frame is in next position.
                DBGPRINT(RT_DEBUG_ERROR,("NDIS_STATUS_ADAPTER_NOT_READY\n"));
                InsertTailQueue(pQueue, pXcu);            
                
                break;
            }
            else if( ndisStatus == NDIS_STATUS_NOT_ACCEPTED)
            {
                //no key install ,fail this netbuffer list
                // In this case, we keep continuing to dequeue the NetBuffer, 
                // Otherwise will happen packet not completing issue
                //
                if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
                {
                    RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_FAILURE);
                }
            }
            else if( ndisStatus == NDIS_STATUS_NOT_COPIED )
            {
                // Support RaTxAggregation: If we can't do aggregation in HardTransmit function, put it into SendTxWaitQueue
                DBGPRINT(RT_DEBUG_TRACE,("NDIS_STATUS_NOT_COPIED\n"));              
                Count++;    
            }
            else
            {
                Count++;
            }
        }
        else
        {
            //DBGPRINT(RT_DEBUG_ERROR,("--ApUapsdDeQueueUAPSDPacket %d queue full !!  NetBuffer = %x\n", BulkOutPipeId, NetBuffer));        
            InsertHeadQueue(pQueue, pXcu);
            break;
        }
    }

    // Release TxSwUapsdQueue resources
    NdisReleaseSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[BulkOutPipeId]);

    if ((pMacEntry->bAPSDAllAC == TRUE) &&
        (pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_BE].Number==0) &&
        (pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_BK].Number==0) &&
        (pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_VI].Number==0) &&
        (pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_VO].Number==0))
    {
        /* all AC are U-APSD and no any U-APSD packet is queued, set TIM */
        /* clear TIM bit */
        if ((pMacEntry->Aid > 0) && (pMacEntry->Aid < MAX_LEN_OF_MAC_TABLE))
        {
            if (pMacEntry->Aid >= 32)
                pPort->SoftAP.ApCfg.TimBitmap2 &= (~TIM_BITMAP[pMacEntry->Aid-32]);  
            else
                pPort->SoftAP.ApCfg.TimBitmap &= (~TIM_BITMAP[pMacEntry->Aid]);

            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_UPDATE_TIM, NULL, 0);
            DBGPRINT(RT_DEBUG_TRACE, (" %s : Clear TIM, Current bit map=%d \n", __FUNCTION__, pPort->SoftAP.ApCfg.TimBitmap));
            
        } 
    }

    NdisAcquireSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
    pAd->pTxCfg->DeQueueRunning[BulkOutPipeId] = FALSE;
    NdisReleaseSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
}

/*
========================================================================
Routine Description:
    Tag current offset of the AC in USB URB tx buffer.

Arguments:
    pAd             Pointer to our adapter
    NetBuffer           the tx packet
    Wcid            destination entry id
    PktOffset       USB tx buffer offset

Return Value:
    None

Note:
    Only for RT2870.
========================================================================
*/

BOOLEAN 
ApUapsdTagFrame(
    IN  MP_ADAPTER        *pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu,
    IN  UCHAR               Wcid,
    IN  UINT32              PktOffset
    )
{
    UCHAR AcQueId;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pPort, MlmeSyncGetRoleTypeByWlanIdx(pPort, Wcid));
    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return FALSE;
    }
    
    if ((Wcid == MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST)) )
        return FALSE; /* the frame is broadcast/multicast frame */
    if (pXcu->bUAPSD== FALSE)
        return FALSE; /* not uapsd data*/
    
    DBGPRINT(RT_DEBUG_INFO, ("uapsd> --------------------------------------------->  start UAPSD_TagFrame\n")); 

    /* mark the latest USB tx buffer offset for the priority */
    AcQueId = pXcu->QueIdx;
    pEntry->UAPSDTagOffset[AcQueId] = PktOffset;

    DBGPRINT(RT_DEBUG_TRACE, ("uapsd> tag offset = %d, AcQueId=%d\n", PktOffset, AcQueId));

    DBGPRINT(RT_DEBUG_INFO, ("uapsd> <--------------------------------------------- end UAPSD_TagFrame\n"));    

    return TRUE;
}

/*
========================================================================
Routine Description:
    Check if UAPSD packets are tx ok.

Arguments:
    pAd             Pointer to our adapter
    AcQueId         TX completion for the AC (0 ~ 3)
    bulkStartPos
    bulkEnPos

Return Value:
    None

Note:
    Only for RT2870.
========================================================================
*/
VOID ApUapsdUnTagFrame(
    IN  MP_ADAPTER    *pAd,
    IN  UCHAR           AcQueId,
    IN  UINT32          bulkStartPos,
    IN  UINT32          bulkEnPos)
{
    MAC_TABLE_ENTRY *pEntry;
    UINT32 TxPktTagOffset;
    PMAC_TABLE_ENTRY pNextEntry = NULL;   
    PQUEUE_HEADER pHeader;    
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    // Check if WMM-PS Service Period shall be stoped.
    // P2P WMM-PS operation with P2P Group Owner NoA
    if (pPort->P2PCfg.GONoASchedule.bValid == TRUE)
    {
        ULONG   Value, GPDiff;
        
        RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &Value);

        if ((Value < pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting) &&
            ((pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting -Value) >= WMM_USP_STOP_PRIOR_TO_NOA_MARGIN) )
        {
            //if (UAPSD_MR_IN_P2P_PRESENT(pAd, pEntry))
            //{
            //}
        }
        else if ((Value < pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting) &&
                ((pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting - Value) < WMM_USP_STOP_PRIOR_TO_NOA_MARGIN))
        {
            // A USP shall end if a P2P Group Owner absence period occurs prior to signaling the end of the USP. 
            // When this occurs, the End of Service Period (EOSP) bit in the last frame delivered prior to the absence is 0 and the More bit is 1, 
            // indicating that buffered data remains at the P2P Group Owner. 

            if (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == FALSE)
            {
                pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent = TRUE;
                GPDiff = pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting - Value;
                //DBGPRINT(RT_DEBUG_TRACE, ("uapsd> close SP in NICUpdateFifoStaCounters(), GPDiff = %d,  bWMMPSInAbsent=%d\n", GPDiff, pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent));
                // Force to close SPs of ALL Entry once entered NoA Absent, starIdx=2
                pHeader = &pPort->MacTab.MacTabList;
                pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                while (pNextEntry != NULL)
                {
                    pEntry = (PMAC_TABLE_ENTRY)pNextEntry;
                    
                    if(pEntry == NULL)
                    {
                        break;
                    }
                    
                    if((pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
                    {
                        pNextEntry = pNextEntry->Next;   
                        pEntry = NULL;
                        continue;
                    }
                    
                    ApUapsdServicePeriodClose(pAd, pEntry, TRUE);
               
                    pNextEntry = pNextEntry->Next;   
                    pEntry = NULL;
                }
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_INFO, ("ApUapsdUnTagFrame: NextTimePointForWMMPSCounting= %d, CurrentTime= %d\n", pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting, Value));
        }
    }

    /* loop for all entries to check whether we need to close their SP */
    pHeader = &pPort->MacTab.MacTabList;
    pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextEntry;

        if(pEntry == NULL)
        {
            break;
        }
        
        if((pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
        {
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue;
        }
        
        if ((pEntry->ValidAsCLI == TRUE) &&
            (pEntry->bAPSDFlagSPStart == TRUE) &&
            (pEntry->UAPSDTagOffset[AcQueId] != 0)) 
        {
            DBGPRINT(RT_DEBUG_INFO, ("uapsd> untag - bulkStartPos = %d\n", bulkStartPos));
            DBGPRINT(RT_DEBUG_INFO, ("uapsd> untag - bulkEnPos = %d\n", bulkEnPos));
            DBGPRINT(RT_DEBUG_TRACE, ("uapsd> untag - record offset = %d ( QId=%d )\n", AcQueId, pEntry->UAPSDTagOffset[AcQueId]));

            /*
                1. tx tag is in [bulkStartPos, bulkEnPos];
                2. when bulkEnPos < bulkStartPos
            */
            TxPktTagOffset = pEntry->UAPSDTagOffset[AcQueId];

            if (((TxPktTagOffset >= bulkStartPos) &&
                    (TxPktTagOffset <= bulkEnPos)) ||
                ((bulkEnPos < bulkStartPos) &&
                    (TxPktTagOffset >= bulkStartPos)) ||
                ((bulkEnPos < bulkStartPos) &&
                    (TxPktTagOffset <= bulkEnPos))||
                ((bulkStartPos < bulkEnPos)  && (TxPktTagOffset <= bulkStartPos)))// I suppose start pos > txPktTagOffset, it hint the packet is sent out.
            {
                /* ok, some UAPSD frames of the AC are transmitted */
                pEntry->UAPSDTagOffset[AcQueId] = 0;

                if ((pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_BK].Number == 0) &&
                    (pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_BE].Number == 0) && 
                    (pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_VI].Number == 0) &&
                    (pAd->UAPSD.TxSwUapsdQueue[pEntry->UAPSDQAid][QID_AC_VO].Number == 0) )

                {
                    /* ok, all UAPSD frames are transmitted */
                    ApUapsdServicePeriodClose(pAd, pEntry, FALSE);
                    pNextEntry = pNextEntry->Next;   
                    pEntry = NULL;
                    continue; /* check next station */
                } 
            } 
        }

        pNextEntry = pNextEntry->Next;    
        pEntry = NULL;
    }
}

VOID 
ApUapsdChangeFlagInNoA(
    IN  PMP_ADAPTER       pAd,
    IN  ULONG               RestTxNum,
    IN  PMT_XMIT_CTRL_UNIT     pXcu
    )
{
    // first Netbuffer
    if (pXcu->bUAPSD)
    {
        if (RestTxNum == 0)
        {
            //It's the last packet from UAPSD queue, set EOSP on.
            if (pXcu->bEosp == FALSE)
            {
                pXcu->bEosp = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, ("uapsd>[#1] HardTX: the LAST pkt, change Eosp to 1 (Num=%d)\n", RestTxNum));
            }
        }
    }

}
/* 
    ==========================================================================
    Description:
        Send out a NULL frame to STA. The prpose is to inform STA the service period is end.    
    ==========================================================================
 */
VOID ApUapsdSendNullFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,  
    IN  PUCHAR              pAddr,
    IN  UCHAR           TxRate, 
    IN  BOOLEAN         bQosNull,
    IN      BOOLEAN         bEOSP,
    IN  UCHAR           OldUP)
{
    UCHAR           QueIdx ;
    UCHAR   NullFrame[48];
    ULONG   Length;
    PHEADER_802_11  pHeader_802_11;
    UCHAR *qos_p;
    
    QueIdx =QID_AC_BE;

    // not sending NULL frame if it is not idle at last second
    // When APSD is on, we should send null packet to inform AP that STA is in PS mode.
    
    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }

    PlatformZeroMemory(NullFrame, 48);
    Length = sizeof(HEADER_802_11);

    pHeader_802_11 = (PHEADER_802_11) NullFrame;
    
    pHeader_802_11->FC.Type = BTYPE_DATA;
    pHeader_802_11->FC.SubType = SUBTYPE_NULL_FUNC;
    pHeader_802_11->FC.FrDs = 1;
    pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;

    COPY_MAC_ADDR(pHeader_802_11->Addr1, pAddr); // sta mac
    COPY_MAC_ADDR(pHeader_802_11->Addr2, pPort->PortCfg.Bssid); // ap mac   
    COPY_MAC_ADDR(pHeader_802_11->Addr3, pPort->PortCfg.Bssid); // ap mac

    pHeader_802_11->Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, TxRate, 14);

    pHeader_802_11->Sequence = pAd->pTxCfg->Sequence;
    pAd->pTxCfg->Sequence++;
    pAd->pTxCfg->Sequence &= MAXSEQ;
    
    DBGPRINT(RT_DEBUG_TRACE, ("uapsd > In ap send null frame, bQosNull=%d, and bEOSP=%d\n", bQosNull, bEOSP));

    // Prepare QosNull function frame
    if (bQosNull)
    {
        qos_p = ((UCHAR *)&NullFrame[0]) + Length;
        pHeader_802_11->FC.SubType = SUBTYPE_QOS_NULL;
        
        // copy QOS control bytes
        qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
        qos_p[1] = 0;
        Length += 2;
    }
    
    NdisCommonMiniportMMRequest(pAd, NullFrame, Length);
}
#endif
