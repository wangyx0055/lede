/*
 ***************************************************************************
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
    p2p.c

    Abstract:
    Peer to peer is also called Wifi Direct. This function handles P2P table management. also include persistent table that saves credential.

    Revision History:
    Who              When               What
    --------    ----------    ----------------------------------------------
    Jan Lee         2010-05-21    created for Peer-to-Peer Action frame(Wifi Direct)
*/
#include "MtConfig.h"

/*  
    ==========================================================================
    Description: 
        This is a periodic routine that check P2P Group Table's Status. One importatn task is to check if some frame
        that need transmission result is success or retry fail.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pGroupMaintain(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    PRT_P2P_CLIENT_ENTRY        pP2pEntry;
    ULONG       Data;
    BOOLEAN     bAllPsm = TRUE;
    BOOLEAN     bAllP2p = TRUE;
    BOOLEAN     bAllActive = TRUE;
    UCHAR       Sanity = 0;
    ULONG       TotalFrameLen;
    UCHAR       StatusCode = 0;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;   
    PQUEUE_HEADER pHeader;

    // Check if we need to update timestamp high byte
    if (pPort->P2PCfg.GONoASchedule.bValid == TRUE)
    {
        // Disable OppPS when NoA is ON.
        P2pStopOpPS(pAd, pPort);
        RTUSBReadMACRegister(pAd, TSF_TIMER_DW1, &Data);

        // Check if high bytes changed. If yes, need to update NoA.
        if (Data != pPort->P2PCfg.GONoASchedule.TsfHighByte)
        {
            /*
            DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain. Tsf MSB changed to %d from %d.  restart NoA . \n",Data, pPort->P2PCfg.GONoASchedule.TsfHighByte ));
            // I want to resume the NoA
            pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = TRUE;
            P2pStopNoA(pAd, NULL);
            // Ok. Now resume it.
            pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = FALSE;
            P2pGOStartNoA(pAd, FALSE);
            */
        }

    }
    // Check all mac table. Need to know if there is at least one legacy client or station. 
    // If yes, I need to stop  P2P power management, i.e. NoA, or OpPs.
    if (IS_P2P_GO_OP(pPort) && 
        ((pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1) 
        || IS_P2P_SIGMA_ON(pPort)
        || pPort->P2PCfg.GONoASchedule.bValid == TRUE))
    {
        // Mac table starts from 2.
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
            
            if(pMacEntry == NULL)
            {
                break;
            }
            
            if((pMacEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            if (pMacEntry->ValidAsCLI == FALSE)
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            if (pMacEntry->ValidAsP2P == FALSE)
            {
                bAllP2p = FALSE;
                DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain. Not all are P2P.  !!!!!!!!! \n"));
            }
            
            if (pMacEntry->PsMode == PWR_ACTIVE)
            {
                bAllPsm = FALSE;
                pPort->P2PCfg.P2pGOAllPsm = FALSE;
                if (pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1)
                    DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain. Not all are Psm. !!!!!!!!! \n"));
                Sanity |= 0x2;
            }
            else
            {
                bAllActive = FALSE;
            }
            
            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }
        
        // Since NoA is OFF, consider to enable OppPS.
        if ((bAllP2p == FALSE) && (pPort->P2PCfg.GONoASchedule.bValid == TRUE))
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain. Not all are P2P. So stop NoA!!!!!!!!! \n" ));
            P2pStopNoA(pAd, NULL);
        }

        if ((bAllPsm == TRUE) && (bAllP2p == TRUE) 
            && (pPort->MacTab.Size > 0) && (pPort->P2PCfg.GONoASchedule.bValid == FALSE))
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain2. all are P2P. All are psm. !!!!!!!!! \n" ));
            // bit 7 is OppPS bit. set 1 to enable. bit [0:6] is in unit TU.
            if ((pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1) && 
                (IS_P2P_SIGMA_ON(pPort)))
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain.[1] all are P2P. All are psm. So start  Opps!!!!!!!!! \n" ));
                pPort->P2PCfg.P2pGOAllPsm = TRUE;
                P2pStartOpPS(pAd, pPort);
            }
            else if ((pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1) && 
                ((pAd->Counter.MTKCounters.Last30SecTotalRxCount < 200)))
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain. [2] all are P2P. All are psm. So start  Opps!!!!!!!!! \n" ));
                P2pStartOpPS(pAd, pPort);
            }
            // case 2 to turn on CTWindows.  Not decide the case 2 rule yet. 2010-June
            else if (0)
            {
                pPort->P2PCfg.CTWindows = 0x8a;
            }
        }
        else if (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT) && 
            ((bAllP2p == FALSE)||(bAllActive == TRUE)))
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2pGroupMaintain. Not all are P2P. Not all psm. So stop OPPS!!!!!!!!! \n" ));
            P2pStopOpPS(pAd, pPort);
        }
    }

    if (pPort->P2PCfg.p2pidxForServiceCbReq < MAX_P2P_GROUP_SIZE)
    {
        if (pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.p2pidxForServiceCbReq].ConfigTimeOut > 0)
            pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.p2pidxForServiceCbReq].ConfigTimeOut--;
        if (pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.p2pidxForServiceCbReq].P2pClientState == P2PSTATE_SERVICE_COMEBACK_COMMAND
            && (pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.p2pidxForServiceCbReq].ConfigTimeOut == 0))
        {
            P2pActionSendComebackReq(pAd, pPort, pPort->P2PCfg.p2pidxForServiceCbReq, pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.p2pidxForServiceCbReq].addr);
            pPort->P2PCfg.p2pidxForServiceCbReq = MAX_P2P_GROUP_SIZE;
        }
    }
    
    if ((!IS_P2P_CLIENT_OP(pPort))
        && (!IS_P2P_GO_OP(pPort)))
    {
        UINT i =0;
        for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
        {       
            pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[i];
            // Add some delay to connect to Persistent GO. Because some GO like broadcom need configuration time to start GO.
            if ((pP2pEntry->P2pClientState == P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME))
            {
                if (pP2pEntry->ConfigTimeOut > 0)
                    pP2pEntry->ConfigTimeOut--;
                if (pP2pEntry->ConfigTimeOut == 0)
                {
                    pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_ENROLLEE;
                    pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
                    P2pWpsDone(pAd, pP2pEntry->addr, pPort);
                }
            }
            else if (pP2pEntry->P2pClientState == P2PSTATE_SENT_PROVISION_REQ)
            {
                if (pP2pEntry->ConfigTimeOut > 0)
                    pP2pEntry->ConfigTimeOut--;
                if (pP2pEntry->ConfigTimeOut  == 0)
                {
                    DBGPRINT(RT_DEBUG_TRACE, (" no need change client state"));
                    pP2pEntry->P2pClientState = P2PSTATE_PROVISION_COMMAND;
                }
                else if (((pP2pEntry->ConfigTimeOut %50) == 3) ||((pP2pEntry->ConfigTimeOut %50) == 2))
                {
                    // retry every 5 seconds if no response
                    pP2pEntry->P2pClientState = P2PSTATE_PROVISION_COMMAND;
                }
            }
        }
    }
    // time out case.
    if (IS_P2P_GO_OP(pPort))
    {
        UINT i =0;
        for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
        {       
            pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[i];
            if (pP2pEntry->P2pClientState == P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS)
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2P  P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS \n"));

                P2PSendDevDisRsp(pAd, pPort, P2PSTATUS_SUCCESS, pPort->P2PCfg.LatestP2pPublicFrame.Token, pPort->P2PCfg.LatestP2pPublicFrame.p80211Header.Addr2, &TotalFrameLen);
                pP2pEntry->P2pClientState = P2PSTATE_CLIENT_OPERATING;
            }
            else if (pP2pEntry->P2pClientState == P2PSTATE_WAIT_GO_DISCO_ACK)
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2P  P2PSTATE_WAIT_GO_DISCO_ACK \n"));
                if(pP2pEntry->GODevDisWaitCount == 20)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("P2P GO DISCO - Can't Find Device \n"));
                    pP2pEntry->P2pClientState = P2PSTATE_CLIENT_OPERATING;
                    P2PSendDevDisRsp(pAd, pPort, P2PSTATUS_IMCOMPA_PARM, pPort->P2PCfg.LatestP2pPublicFrame.Token, pPort->P2PCfg.LatestP2pPublicFrame.p80211Header.Addr2, &TotalFrameLen);
                }
                pP2pEntry->GODevDisWaitCount++;
            }
            else if (pP2pEntry->P2pClientState== P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS)
            {
                // Only when I am GO . I need to check the response ack success or not. 
                // doesn't check rule.  start GO right away.
                pP2pEntry->P2pClientState = P2PSTATE_CLIENT_WPS;
                
                P2pStartAutoGo(pAd, pPort);
                DBGPRINT(RT_DEBUG_TRACE,("P2P Table : idx=%d Get Invite Rsp Ask Success.  p2pState = %d.\n", i, pPort->P2PCfg.P2PConnectState));
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.Addr, pP2pEntry->addr, MAC_ADDR_LEN);
            
                // Wps Done will set PMK to me.
                P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                // this is not Auto GO by command from GUI. So set the intent index to != 16
                //  if I am auto GO and after persistent I still be auto GO
            }
        }
    }
    else if ((pPort->P2PCfg.P2PConnectState ==  P2P_DO_GO_SCAN_BEGIN)
        && (pPort->P2PCfg.P2pCounter.GoScanBeginCounter100ms > GOSCANBEGINCOUNTER_MAX))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2P_DO_GO_SCAN_BEGIN Timeout. BAck to idle. \n"));
        P2pGroupFormFailHandle(pAd);
    }
    else if ((pPort->P2PCfg.P2PConnectState ==  P2P_I_AM_CLIENT_ASSOC_AUTH)
        && (pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms > GOSCANBEGINCOUNTER_MAX))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2P_I_AM_CLIENT_ASSOC_AUTH Timeout. Back to idle. \n"));
        pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
    }
    else if (IS_P2P_GO_NEGOING(pPort) 
        ||(pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE))
    {
        UINT i =0;
        for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
        {       
            pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[i];
            if (pP2pEntry->P2pClientState == P2PSTATE_NONE)
                continue;
            if ((pP2pEntry->P2pClientState >= P2PSTATE_SENT_GO_NEG_REQ) && 
                (pP2pEntry->P2pClientState <= P2PSTATE_WAIT_GO_COMFIRM_ACK))
            {
                pP2pEntry->StateCount++;
                if (pP2pEntry->StateCount >= 1200)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("P2P Table : idx=%d Go Nego Req Timeout2. \n", i));
                    pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY;
                    P2pGroupFormFailHandle(pAd);
                    pPort->P2PCfg.P2pCounter.Counter100ms = 0;
                    pP2pEntry->StateCount = 0;
                    pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_GROUP_FORMING));
                    pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_OWNER));
                }
                else if (((pP2pEntry->StateCount % 100) == 10) 
                    && (pP2pEntry->StateCount > 200)
                    && (pP2pEntry->P2pClientState == P2PSTATE_SENT_GO_NEG_REQ))
                {
                    DBGPRINT(RT_DEBUG_TRACE,("P2P Table : idx=%d Go Nego Req Retry. \n", i));
                    pP2pEntry->P2pClientState = P2PSTATE_CONNECT_COMMAND;
                    pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
                    pPort->P2PCfg.P2pCounter.Counter100ms = 0;
                }
                else if (((pP2pEntry->StateCount % 100) == 10) 
                    && (pP2pEntry->StateCount > 200)
                    && (pP2pEntry->P2pClientState == P2PSTATE_WAIT_GO_COMFIRM_ACK))
                {
                    DBGPRINT(RT_DEBUG_TRACE,("P2P Table : idx=%d Go Nego Confirm Retry. \n", i));
                    StatusCode = 0;
                    P2PSendGoNegoConfirm(pAd, pPort, pP2pEntry->FrameToken, (UCHAR)i, pP2pEntry->addr, 0);
                }
            }
            else if (pP2pEntry->P2pClientState == P2PSTATE_GO_COMFIRM_ACK_SUCCESS)
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2P Table : idx=%d Get Confirm Ask Success.  p2pState = %d.\n", i, pPort->P2PCfg.P2PConnectState));
                // Don't leet ClientState keep in P2PSTATE_GO_COMFIRM_ACK_SUCCESS,
                // Or will keep calling P2pGoNegoDone(). 
                // ClientState will be updated when GO receiving AuthReq.
                pP2pEntry->P2pClientState = P2PSTATE_GOT_GO_COMFIRM;
                P2pGoNegoDone(pAd, pPort, pP2pEntry);
            }
            else if(pP2pEntry->P2pClientState== P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK)
            {
                if(pP2pEntry->StateCount == 0)
                {
                    pPort->P2PCfg.LockNego = FALSE;;
                    DBGPRINT(RT_DEBUG_TRACE,("wait P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK fail"));
                }
                else
                {
                    pP2pEntry->StateCount--;
                    DBGPRINT(RT_DEBUG_TRACE,("wait P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK count = %d", pP2pEntry->StateCount));
                }   
            }
            else if (pP2pEntry->P2pClientState== P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS)
            {
                // Only when I am GO . I need to check the response ack success or not. 
                // doesn't check rule.  start GO right away.
                pP2pEntry->P2pClientState = P2PSTATE_CLIENT_WPS;
                P2pStartAutoGo(pAd, pPort);
                DBGPRINT(RT_DEBUG_TRACE,("P2P Table :  Get Invite Rsp Ask Success.  p2pState = %d.\n", pPort->P2PCfg.P2PConnectState));
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.Addr, pP2pEntry->addr, MAC_ADDR_LEN);
                // Wps Done will set PMK to me.
                P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                // this is not Auto GO by command from GUI. So set the intent index to != 16
                //  if I am auto GO and after persistent I still be auto GO
                if(IS_P2P_AUTOGO(pPort))
                    pPort->P2PCfg.GoIntentIdx = 16;
                else
                    pPort->P2PCfg.GoIntentIdx = 15;
            }
        }
    }
}


/*  
    ==========================================================================
    Description: 
        Copy P2P Table's information to Mac Table when the P2P Device is in my group.
        
    Parameters: 
    Note:
    ==========================================================================
 */
VOID P2pCopyMacTabtoP2PTab(
    IN PMP_ADAPTER pAd,
    IN UCHAR        P2pindex,
    IN UCHAR        Macindex)
{
    MAC_TABLE_ENTRY  *pEntry;
    RT_P2P_CLIENT_ENTRY *pP2pEntry;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    if ((P2pindex >= MAX_P2P_GROUP_SIZE) || (Macindex >= MAX_LEN_OF_MAC_TABLE))
        return;

    pEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Macindex); 
    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,Macindex));
        return;
    }
    
    pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[P2pindex];
    pP2pEntry->CTWindow= pEntry->P2pInfo.CTWindow;
    pP2pEntry->P2pClientState = pEntry->P2pInfo.P2pClientState;
    pP2pEntry->P2pFlag = pEntry->P2pInfo.P2pFlag;
    pP2pEntry->NoAToken = pEntry->P2pInfo.NoAToken;
    pP2pEntry->GeneralToken = pEntry->P2pInfo.GeneralToken;

    pP2pEntry->DevCapability = pEntry->P2pInfo.DevCapability;
    pP2pEntry->GroupCapability = pEntry->P2pInfo.GroupCapability;
    pP2pEntry->NumSecondaryType = pEntry->P2pInfo.NumSecondaryType; 
    pP2pEntry->DeviceNameLen = pEntry->P2pInfo.DeviceNameLen;
    pP2pEntry->ConfigMethod = pEntry->P2pInfo.ConfigMethod;
    
    PlatformMoveMemory(pP2pEntry->addr, pEntry->P2pInfo.DevAddr, MAC_ADDR_LEN);
    PlatformMoveMemory(pP2pEntry->InterfaceAddr, pEntry->P2pInfo.InterfaceAddr, MAC_ADDR_LEN);
    // Save the bssid with interface address. 
    PlatformMoveMemory(pP2pEntry->bssid, pEntry->P2pInfo.InterfaceAddr, MAC_ADDR_LEN);
    PlatformMoveMemory(pP2pEntry->PrimaryDevType, pEntry->P2pInfo.PrimaryDevType, P2P_DEVICE_TYPE_LEN);
    PlatformMoveMemory(pP2pEntry->DeviceName, pEntry->P2pInfo.DeviceName, 32);
    PlatformMoveMemory(pP2pEntry->SecondaryDevType, pEntry->P2pInfo.SecondaryDevType, MAX_P2P_SECONDARY_DEVTYPE_LIST * P2P_DEVICE_TYPE_LEN);

}


/*  
    ==========================================================================
    Description: 
        Copy P2P Table's information to Mac Table when the P2P Device is in my group.
        
    Parameters: 
    Note:
    ==========================================================================
 */
VOID P2pCopyP2PTabtoMacTab(
    IN PMP_ADAPTER pAd,
    IN UCHAR        P2pindex,
    IN UCHAR        Macindex)
{
    MAC_TABLE_ENTRY  *pEntry;
    RT_P2P_CLIENT_ENTRY *pP2pEntry;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    
    pEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Macindex);   

    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,Macindex));
        return;
    }
    
    if ((P2pindex >= MAX_P2P_GROUP_SIZE) || (Macindex >= MAX_LEN_OF_MAC_TABLE))
        return;

    pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[P2pindex];
    pEntry->P2pInfo.CTWindow = pP2pEntry->CTWindow;
    pEntry->P2pInfo.P2pClientState = pP2pEntry->P2pClientState;
    pEntry->P2pInfo.P2pFlag = pP2pEntry->P2pFlag;
    pEntry->P2pInfo.NoAToken = pP2pEntry->NoAToken;
    pEntry->P2pInfo.GeneralToken = pP2pEntry->GeneralToken;
    DBGPRINT(RT_DEBUG_TRACE, ("MacTab Add = %s. \n", decodeP2PClientState(pEntry->P2pInfo.P2pClientState)));
    pEntry->P2pInfo.ConfigMethod = pP2pEntry->ConfigMethod;

    pEntry->P2pInfo.DevCapability = pP2pEntry->DevCapability;
    pEntry->P2pInfo.GroupCapability = pP2pEntry->GroupCapability;
    pEntry->P2pInfo.NumSecondaryType = pP2pEntry->NumSecondaryType; 
    pEntry->P2pInfo.DeviceNameLen = pP2pEntry->DeviceNameLen;
    
    PlatformMoveMemory(pEntry->P2pInfo.DevAddr, pP2pEntry->addr, MAC_ADDR_LEN);
    PlatformMoveMemory(pEntry->P2pInfo.InterfaceAddr, pP2pEntry->InterfaceAddr, MAC_ADDR_LEN);
    DBGPRINT(RT_DEBUG_TRACE, ("MacTab InterfaceAddr = %x %x %x . \n", pP2pEntry->InterfaceAddr[3], pP2pEntry->InterfaceAddr[4], pP2pEntry->InterfaceAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("MacTab DevAddr = %x %x %x  %x %x %x. \n", pP2pEntry->addr[0],pP2pEntry->addr[1],pP2pEntry->addr[2],pP2pEntry->addr[3], pP2pEntry->addr[4], pP2pEntry->addr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("MacTab  DeviceNameLen = %d . \n", pEntry->P2pInfo.DeviceNameLen));
    PlatformMoveMemory(pEntry->P2pInfo.PrimaryDevType, pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
    PlatformMoveMemory(pEntry->P2pInfo.DeviceName, pP2pEntry->DeviceName, 32);
    PlatformMoveMemory(pEntry->P2pInfo.SecondaryDevType, pP2pEntry->SecondaryDevType, MAX_P2P_SECONDARY_DEVTYPE_LIST * P2P_DEVICE_TYPE_LEN);
    P2PPrintMac(pAd, Macindex);
}


/*  
    ==========================================================================
    Description: 
        Init P2P Group Table.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pGroupTabInit(
    IN PMP_ADAPTER pAd) 
{

    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR       i;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTabInit .  \n"));
    for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
    {       
        Tab->Client[i].P2pClientState = P2PSTATE_NONE;
        Tab->Client[i].Rule = P2P_IS_CLIENT;
        Tab->Client[i].DevCapability = 0;
        Tab->Client[i].GroupCapability = 0;
        PlatformZeroMemory(Tab->Client[i].addr, MAC_ADDR_LEN);
        PlatformZeroMemory(Tab->Client[i].bssid, MAC_ADDR_LEN);
        PlatformZeroMemory(Tab->Client[i].InterfaceAddr, MAC_ADDR_LEN);
        PlatformZeroMemory(Tab->Client[i].Ssid, MAX_LEN_OF_SSID);
        PlatformZeroMemory(Tab->Client[i].PrimaryDevType, P2P_DEVICE_TYPE_LEN);
        PlatformZeroMemory(Tab->Client[i].SecondaryDevType, MAX_P2P_SECONDARY_DEVTYPE_LIST * P2P_DEVICE_TYPE_LEN);
        PlatformZeroMemory(Tab->Client[i].DeviceName, P2P_DEVICE_NAME_LEN);
        Tab->Client[i].DeviceNameLen = 0;
        Tab->Client[i].SsidLen = 0;
        Tab->Client[i].GoIntent = 0;
        Tab->Client[i].OpChannel = 0;
        Tab->Client[i].ListenChannel = 0;
        Tab->Client[i].ConfigMethod = 0;
        Tab->Client[i].ChannelNr = 0;
        Tab->Client[i].NumSecondaryType = 0;

    }
    Tab->ClientNumber = 0;
    return;
}


/*  
    ==========================================================================
    Description: 
        Send Disassociation/Disconnect to peer clients or GO

    Parameters:
        DeleteClass     0       Delete all, including all of peers and my GO
                        1       GO send disconnect to p2p clients
                        2       GO send disconnect to wps clients
                        3       GO send disconnect to wpa clients
    Note:
         
    ==========================================================================
 */

VOID P2pGroupMacTabDisconnect(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT        pPort,
    IN  UCHAR           DeleteClass)    

{

    PMAC_TABLE_ENTRY        pP2pEntry;
    MLME_DEAUTH_REQ_STRUCT      DeAuthRequest;

    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;
   
    // Halt Handler alreayd handle sending disassociation packet
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) || 
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTab  Disconnect All==> \n"));

    if (DeleteClass == 0)
    {
        if (IS_P2P_CLIENT_OP(pPort))
        {
            DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
            DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTab  Disconnect  ==>..%x \n", pPort->P2PCfg.Bssid[5]));
            COPY_MAC_ADDR(DeAuthRequest.Addr, pPort->P2PCfg.Bssid);
            DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTab  AssocState=%d \n", pAd->PortList[pPort->P2PCfg.PortNumber]->Mlme.AssocMachine.CurrState));
            MlmeEnqueue(pAd,
                        pAd->PortList[pPort->P2PCfg.PortNumber],
                        ASSOC_STATE_MACHINE,
                        MT2_MLME_DISASSOC_REQ,
                        sizeof(MLME_DEAUTH_REQ_STRUCT),
                        &DeAuthRequest);
            // don't go to wait-oid-disassoc sate
            return;
        }
    }
    pHeader = &pPort->MacTab.MacTabList;
    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextMacEntry != NULL)
    {
        pP2pEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
    
        if(pP2pEntry == NULL)
        {
            break;
        }
        
        if((pP2pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pP2pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pP2pEntry = NULL;
            continue; 
        }
        
        if ((pP2pEntry->ValidAsCLI) && (pP2pEntry->P2pInfo.P2pClientState >= P2PSTATE_CLIENT_OPERATING))
        {
            // this Client is operating, need to send Disassociate frame disconnect.
            //
            PlatformZeroMemory(&DeAuthRequest, sizeof(DeAuthRequest));
            if((IS_P2P_SIGMA_OFF(pPort)) && 
                (pPort->CommonCfg.P2pControl.field.EnablePresistent == TRUE) &&
                (pPort->P2PCfg.LastConfigMode == CONFIG_MODE_DISCONNECT_P2P))
            {
                DeAuthRequest.Reason = REASON_P2P_PEER_MANUAL_DEAUTH;
                DBGPRINT(RT_DEBUG_TRACE,("REASON_P2P_PEER_MANUAL_DEAUTH"));
            }   
            else
            {
            DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
                DBGPRINT(RT_DEBUG_TRACE,("REASON_DEAUTH_STA_LEAVING"));
            }
            
            // DeleteClass=0, 1==> p2p clients
            // DeleteClass=2    ==> wps clients
            // DeleteClass=3    ==> wpa clients
            if (((DeleteClass == 0) || (DeleteClass == 1)) && (pP2pEntry->ValidAsP2P))
                PlatformMoveMemory(DeAuthRequest.Addr, pP2pEntry->P2pInfo.InterfaceAddr, 6);
            else if (((DeleteClass == 0) || (DeleteClass == 2)) && (pP2pEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_WPS))
                PlatformMoveMemory(DeAuthRequest.Addr, pP2pEntry->Addr, 6);
            else if (((DeleteClass == 0) || (DeleteClass == 3)) && (pP2pEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_PSK))
                PlatformMoveMemory(DeAuthRequest.Addr, pP2pEntry->Addr, 6);

            //Send dis_assoc & de_auth
            if ((!PlatformEqualMemory(DeAuthRequest.Addr, ZeroSsid, 6)))
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTab  Disconnect CLI (%d)==> \n", pP2pEntry->wcid));
                ApAuthGoDeauthReqAction(pAd,pPort->P2PCfg.PortNumber, &DeAuthRequest);
            }
        }
        pNextMacEntry = pNextMacEntry->Next;   
        pP2pEntry = NULL;
    }
    
    return;
}


/*  
    ==========================================================================
    Description: 
        insert a peer to P2P Group Table. Because this Peer contains P2P IE and P2P Wildwork SSID to indicate that it support P2P
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pGroupTabInsert(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr,
    IN P2P_CLIENT_STATE State,
    IN CHAR Ssid[], 
    IN UCHAR SsidLen)
{

    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR       i;

    if (PlatformEqualMemory(ZeroSsid, Addr, 6))
    {
        DBGPRINT(RT_DEBUG_ERROR,("P2pGroupTabInsert . Addr all zero Error. \n"));
        return P2P_NOT_FOUND;
    }
    if ((Addr[0] & 0x1) == 0x1)
    {
        DBGPRINT(RT_DEBUG_ERROR,("P2pGroupTabInsert . Insert mcast Addr Error. \n"));
        return P2P_NOT_FOUND;
    }
    
    for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
    {       
        // This peer already exist, so only update state.
        if ((Tab->Client[i].P2pClientState != P2PSTATE_NONE) 
            && (PlatformEqualMemory(Tab->Client[i].addr, Addr, MAC_ADDR_LEN)))
        {
            if (State != P2PSTATE_NONE)
                Tab->Client[i].P2pClientState = State;
            if ((SsidLen > 0) && (Ssid != NULL))
                PlatformMoveMemory(Tab->Client[i].Ssid, Ssid, 32);
            Tab->Client[i].SsidLen = SsidLen;
            return i;
        }
        else if (Tab->Client[i].P2pClientState == P2PSTATE_NONE)
        {
            Tab->ClientNumber++;
            PlatformMoveMemory(Tab->Client[i].addr, Addr, 6);
            
            DBGPRINT(RT_DEBUG_INFO,("P2pGroupTabInsert . Arrd[5] = 0x%x. Update State = %s \n", Addr[5], decodeP2PClientState(State)));
            Tab->Client[i].P2pClientState = State;
            if ((SsidLen > 0) && (Ssid != NULL))
                PlatformMoveMemory(Tab->Client[i].Ssid, Ssid, 32);
            Tab->Client[i].SsidLen = SsidLen;
            pAd->pP2pCtrll->P2PTable.Client[i].GoIntent = 0;
            pAd->pP2pCtrll->P2PTable.Client[i].MyGOIndex = 0xff;
            pAd->pP2pCtrll->P2PTable.Client[i].Peerip = 0;
            pAd->pP2pCtrll->P2PTable.Client[i].ConfigTimeOut = 0;
            pAd->pP2pCtrll->P2PTable.Client[i].OpChannel = 0;
            pAd->pP2pCtrll->P2PTable.Client[i].ListenChannel = 0;
            pAd->pP2pCtrll->P2PTable.Client[i].GeneralToken = RandomByte(pAd);
            if ((pAd->pP2pCtrll->P2PTable.Client[i].GeneralToken == 0)
                 || (pAd->pP2pCtrll->P2PTable.Client[i].GeneralToken > 245))
                 pAd->pP2pCtrll->P2PTable.Client[i].GeneralToken = 6;
            pAd->pP2pCtrll->P2PTable.Client[i].Dpid = DEV_PASS_ID_NOSPEC;
            pAd->pP2pCtrll->P2PTable.Client[i].P2pFlag = 0;
            if (State == P2PSTATE_DISCOVERY_GO)
                pAd->pP2pCtrll->P2PTable.Client[i].Rule = P2P_IS_GO;
            else
                pAd->pP2pCtrll->P2PTable.Client[i].Rule = P2P_IS_CLIENT;

            return i;
        }
    }
    return P2P_NOT_FOUND;
}


/*  
    ==========================================================================
    Description: 
        Delete a peer in P2P Group Table.  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pGroupTabDelete(
    IN PMP_ADAPTER pAd, 
    IN UCHAR    p2pindex, 
    IN PUCHAR    Addr) 
{

    UCHAR   index = 0xff;
    PRT_P2P_CLIENT_ENTRY        pP2pEntry;

    if ((p2pindex >= MAX_P2P_GROUP_SIZE) && (Addr != NULL))
        index = P2pGroupTabSearch(pAd, Addr);
    else
        index = p2pindex;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTabDelete . index = %d. \n", index));
    if (index < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[index];
        // Before connected, there is WPS provisioning process.
        // So maybe receive disassoc frame. but we can't delete p2p client entry .
        // So need to check P2pClientState is connected, then we can delete the entry.
        DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTabDelete  index %d.  search addr[3~5] is %x %x %x\n", index, Addr[3],Addr[4],Addr[5]));
        
        PlatformZeroMemory(pP2pEntry->addr, MAC_ADDR_LEN);
        PlatformZeroMemory(pP2pEntry->bssid, MAC_ADDR_LEN);
        PlatformZeroMemory(pP2pEntry->InterfaceAddr, MAC_ADDR_LEN);
        PlatformZeroMemory(pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
        PlatformZeroMemory(pP2pEntry->SecondaryDevType, MAX_P2P_SECONDARY_DEVTYPE_LIST * P2P_DEVICE_TYPE_LEN);
        PlatformZeroMemory(pP2pEntry->DeviceName, P2P_DEVICE_NAME_LEN);
        pP2pEntry->NumSecondaryType = 0;
        pP2pEntry->DeviceNameLen = 0;
        pP2pEntry->ConfigMethod = 0;
        pP2pEntry->OpChannel = 0;
        pP2pEntry->ListenChannel = 0;
        pP2pEntry->Dpid = DEV_PASS_ID_NOSPEC;
        pP2pEntry->MyGOIndex = 0xff;
        pP2pEntry->Peerip = 0;
        pP2pEntry->ConfigTimeOut = 0;
        pP2pEntry->Rule = P2P_IS_CLIENT;

        PlatformZeroMemory(pP2pEntry->Ssid, MAX_LEN_OF_SSID);
        pP2pEntry->SsidLen = 0;
        pP2pEntry->GoIntent = 0;
        if ((pAd->pP2pCtrll->P2PTable.ClientNumber > 0) && (pP2pEntry->P2pClientState != P2PSTATE_NONE))
            pAd->pP2pCtrll->P2PTable.ClientNumber--;
        
        pP2pEntry->P2pClientState = P2PSTATE_NONE;
    }

    return index;
}


/*  
    ==========================================================================
    Description: 
        Search a peer in P2P Group Table by the same MAc Addr..  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pGroupTabSearch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr) 
{

    UCHAR   i;
    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR   index = P2P_NOT_FOUND;
    UCHAR           Allff[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // find addr can't be 1. multicast, 2. all zero, 3. all 0xff 
    if ((Addr[0] & 0x1) == 0x1)
        return index;
    
    if (PlatformEqualMemory(Allff, Addr, 6))
        return index;

    if (PlatformEqualMemory(ZeroSsid, Addr, 6))
        return index;

    for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
    {       
        // If addr format is all zero or all 0xff or multicast, return before. So doesn't need to 
        // check invalid match here.
        if (((PlatformEqualMemory(Tab->Client[i].bssid, Addr, 6)) 
            || (PlatformEqualMemory(Tab->Client[i].InterfaceAddr, Addr, 6))
            || (PlatformEqualMemory(Tab->Client[i].addr, Addr, 6)))
            && (Tab->Client[i].P2pClientState > P2PSTATE_NONE))
            index = i;
    }
    
    return index;
}



/*  
    ==========================================================================
    Description: 
        GUI needs to update whole Persistent table to driver after reload of driver. 
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pSetPerstTable(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInformationBuffer) 
{
    POID_P2P_PERSISTENT_TABLE       pP2pPerstTab;
    PMP_PORT          pPort;
    UCHAR       i;
    UCHAR ZeroMacAddr[6] = { 0, 0, 0, 0, 0, 0};
    
    pPort = pAd->PortList[FXXK_PORT_0];
    
    pP2pPerstTab = (POID_P2P_PERSISTENT_TABLE)pInformationBuffer;
    DBGPRINT(RT_DEBUG_TRACE,("UI set P2pSetPerstTable   Num = %d \n", pP2pPerstTab->PerstNumber));
    pAd->pP2pCtrll->P2PTable.PerstNumber = 0;
    
    PlatformZeroMemory(pAd->pP2pCtrll->P2PTable.PerstEntry, sizeof(RT_P2P_PERSISTENT_ENTRY)*8);
    
    for (i = 0; i < pP2pPerstTab->PerstNumber; i++)
    {
        if(!PlatformEqualMemory(&pP2pPerstTab->PerstEntry[i].Addr[0], &ZeroMacAddr[0], MAC_ADDRESS_LENGTH)) 
        {
            
            PlatformMoveMemory(&pAd->pP2pCtrll->P2PTable.PerstEntry[pAd->pP2pCtrll->P2PTable.PerstNumber], &pP2pPerstTab->PerstEntry[i], sizeof(RT_P2P_PERSISTENT_ENTRY));
            pAd->pP2pCtrll->P2PTable.PerstNumber++;
            DBGPRINT(RT_DEBUG_TRACE, ("use this profile P2PTable.PerstNumber =%d=====>", pAd->pP2pCtrll->P2PTable.PerstNumber));
            P2PPrintP2PPerstEntry(pAd,i);
        }
        else
        {
            // for Debug
            DBGPRINT(RT_DEBUG_TRACE, ("not use this profile =====>", pAd->pP2pCtrll->P2PTable.PerstNumber));
            P2PPrintP2PPerstEntry(pAd,i);
        
        }
        
    }
}


/*  
    ==========================================================================
    Description: 
        Clean  in P2P Persistent Table.  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pPerstTabClean(
    IN PMP_ADAPTER pAd) 
{

    PRT_P2P_PERSISTENT_ENTRY        pP2pPerstEntry;
    UCHAR       i;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pPerstTabClean .  \n"));

    for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
    {       
        pP2pPerstEntry = &pAd->pP2pCtrll->P2PTable.PerstEntry[i];
        PlatformZeroMemory(pP2pPerstEntry, sizeof(RT_P2P_PERSISTENT_ENTRY));
        pP2pPerstEntry->bValid = FALSE;
    }
    pAd->pP2pCtrll->P2PTable.PerstNumber = 0;
    return;
}

/*  
    ==========================================================================
    Description: 
        Insert a peer into P2P Persistent Table.  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pPerstTabInsert(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pAddr) 
{
    UCHAR       index = 0;
#if 0
    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR       i, j;
    UCHAR       index;
    WSC_CREDENTIAL  *pProfile;
    
    index = P2pPerstTabSearch(pAd, pAddr);
    // Doesn't have this entry. Add a new one.
    if (index == P2P_NOT_FOUND)
    {
        for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
        {       
            if (Tab->PerstEntry[i].bValid == FALSE)
            {
                Tab->PerstEntry[i].bValid = TRUE;
                Tab->PerstNumber++;
                if (IS_P2P_GO_OP(pPort))
                    Tab->PerstEntry[i].MyRule = P2P_IS_GO;
                else
                    Tab->PerstEntry[i].MyRule = P2P_IS_CLIENT;
                
                PlatformMoveMemory(Tab->PerstEntry[i].Addr, pAddr, MAC_ADDR_LEN);
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::Registra MacAddr = %x %x %x %x %x %x\n",Tab->PerstEntry[i].Addr[0], Tab->PerstEntry[i].Addr[1], Tab->PerstEntry[i].Addr[2],Tab->PerstEntry[i].Addr[3],Tab->PerstEntry[i].Addr[4],Tab->PerstEntry[i].Addr[5]));
                PlatformMoveMemory(&Tab->PerstEntry[i].Profile, &pPort->StaCfg.WscControl.WscProfile.Profile[0], sizeof(WSC_CREDENTIAL));
                pProfile = &Tab->PerstEntry[i].Profile;
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::SsidLen = %d\n",pProfile->SSID.SsidLength));
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::Ssid = %c%c%c%c%c%c%c \n",pProfile->SSID.Ssid[0],pProfile->SSID.Ssid[1],pProfile->SSID.Ssid[2],pProfile->SSID.Ssid[3],pProfile->SSID.Ssid[4],pProfile->SSID.Ssid[5],pProfile->SSID.Ssid[6]));
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::MacAddr = %x %x %x %x %x %x   \n",pProfile->MacAddr[0],pProfile->MacAddr[1],pProfile->MacAddr[2],pProfile->MacAddr[3],pProfile->MacAddr[4],pProfile->MacAddr[5]));
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::AuthType = 0x%x. EncrType = %d\n",pProfile->AuthType,pProfile->EncrType));
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::KeyIndex = %d\n",pProfile->KeyIndex));
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::KeyLength = %d\n",pProfile->KeyLength));
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::Key ==>\n"));
                for (j=0;j<16;)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("  0x%x  0x%x 0x%x 0x%x\n",pProfile->Key[j], pProfile->Key[j+1], pProfile->Key[j+2],pProfile->Key[j+3]));
                    j = j+4;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Perst::<===Key =\n"));

                DBGPRINT(RT_DEBUG_ERROR, (" P2P -P2pPerstTabInsert to index = %x. Rule = %d.\n", i, Tab->PerstEntry[i].MyRule));
                return i;
            }
        }
    }
    else if (index < MAX_P2P_TABLE_SIZE)
    {

        i = index;
        Tab->PerstEntry[i].bValid = TRUE;
        if (IS_P2P_GO_OP(pPort))
            Tab->PerstEntry[i].MyRule = P2P_IS_GO;
        else
            Tab->PerstEntry[i].MyRule = P2P_IS_CLIENT;
        PlatformMoveMemory(Tab->PerstEntry[i].Addr, pAddr, MAC_ADDR_LEN);
        PlatformMoveMemory(&Tab->PerstEntry[i].Profile, &pPort->StaCfg.WscControl.WscProfile.Profile[0], sizeof(WSC_CREDENTIAL));
        pProfile = &Tab->PerstEntry[i].Profile;
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::SsidLen = %d\n",pProfile->SSID.SsidLength));
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::Ssid = %c%c%c%c%c%c%c \n",pProfile->SSID.Ssid[0],pProfile->SSID.Ssid[1],pProfile->SSID.Ssid[2],pProfile->SSID.Ssid[3],pProfile->SSID.Ssid[4],pProfile->SSID.Ssid[5],pProfile->SSID.Ssid[6]));
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::AuthType = 0x%x. EncrType = %d\n",pProfile->AuthType,pProfile->EncrType));
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::KeyIndex = %d\n",pProfile->KeyIndex));
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::KeyLength = %d\n",pProfile->KeyLength));
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::Key ==>\n"));
        for (j=0;j<16;)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("  0x%x  0x%x 0x%x 0x%x\n",pProfile->Key[j], pProfile->Key[j+1], pProfile->Key[j+2],pProfile->Key[j+3]));
            j = j+4;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::<===Key =\n"));
        DBGPRINT(RT_DEBUG_TRACE, ("Perst::MacAddr = %x %x %x %x %x %x\n",pProfile->MacAddr[0], pProfile->MacAddr[1], pProfile->MacAddr[2],pProfile->MacAddr[3],pProfile->MacAddr[4],pProfile->MacAddr[5]));
        DBGPRINT(RT_DEBUG_ERROR, (" P2P -P2pPerstTabInsert update to index = %x.\n", i));
        return i;
    }
    DBGPRINT(RT_DEBUG_ERROR, ("P2P -P2pPerstTabInsert . PerstNumber = %d.\n", Tab->PerstNumber));
    return index;
    #endif
    return index;
}

/*  
    ==========================================================================
    Description: 
        Delete an entry  in P2P Persistent Table.  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pPerstTabDelete(
    IN PMP_ADAPTER pAd,
    IN UCHAR    *pMacList) 
{

    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR       i;
    
    for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
    {       
        if (Tab->PerstEntry[i].bValid == TRUE)
        {
            if (PlatformEqualMemory(Tab->PerstEntry[i].Addr, pMacList, 6))
            {
                PlatformZeroMemory(&Tab->PerstEntry[i], sizeof(RT_P2P_PERSISTENT_ENTRY));
                Tab->PerstEntry[i].bValid = FALSE;

                if (Tab->PerstNumber > 0)
                {
                    Tab->PerstNumber--;
                }
                else
                    DBGPRINT(RT_DEBUG_ERROR, (" P2P - Persistent table count error. \n"));

                DBGPRINT(RT_DEBUG_ERROR, (" P2P - Delete a Persistent Entry .Table Number = %d. \n",  Tab->PerstNumber));
            }
                
            return i;
        }
    }
    return 0xff;
}

/*  
    ==========================================================================
    Description: 
        Search an entry  in P2P Persistent Table.  All parameters must the same.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pPerstTabSearch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr) 
{

    UCHAR   index = 0;
    #if 0
    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR   index = P2P_NOT_FOUND;
    UCHAR           Allff[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    UCHAR           AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    if ((Addr[0] & 0x1) == 0x1)
        return index;
    
    if (PlatformEqualMemory(Allff, Addr, MAC_ADDR_LEN))
        return index;

    if (PlatformEqualMemory(AllZero, Addr, MAC_ADDR_LEN))
        return index;

    for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
    {       
        if ((Tab->PerstEntry[i].bValid == TRUE) 
            && PlatformEqualMemory(&Tab->PerstEntry[i].Addr, Addr, MAC_ADDR_LEN))
        {
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - i = %d. \n", i));
            // If My rule is client, it must be perst profile when I am in staion opmode.
            if((Tab->PerstEntry[i].MyRule == P2P_IS_CLIENT) && (pAd->OpMode == OPMODE_STA || PORTV2_P2P_ON(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])))            
            {
                index = i;
            }
            else if ((Tab->PerstEntry[i].MyRule == P2P_IS_GO))
            {
                index = i;
            }
        }
    }
    #endif
    return index;
}


/*  
    ==========================================================================
    Description: 
        Clean  in P2P Persistent Table.  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pCrednTabClean(
    IN PMP_ADAPTER pAd) 
{

    UCHAR       i;
    RT_GO_CREDENTIAL_ENTRY  *pCrednEntry;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pCrednTabClean .  \n"));

    for (i = 0; i < MAX_P2P_SAVECREDN_SIZE; i++)
    {       
        pCrednEntry = &pAd->pP2pCtrll->P2PTable.TempCredential[i];
        PlatformZeroMemory(pCrednEntry, sizeof(RT_GO_CREDENTIAL_ENTRY));
        pCrednEntry->bValid = FALSE;
    }
    return;
}

/*  
    ==========================================================================
    Description: 
        Insert a peer into P2P Persistent Table.  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pCrednTabInsert(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pAddr,
    IN WSC_CREDENTIAL   *pProfile) 
{

    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR       i = 0;
    UCHAR       index;
    BOOLEAN         bExist;

    bExist = P2pCrednEntrySearch(pAd, pAddr, &index);
    DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednTabInsert \n"));
    if (bExist == FALSE)
    {
        i = (RandomByte(pAd))%2;
        if (i >= MAX_P2P_SAVECREDN_SIZE)
            i = 0;
        
        for ( i = 0; i < MAX_P2P_SAVECREDN_SIZE;i++)
        {
            DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednTabInsert %d\n", i));
            Tab->TempCredential[i].bValid = TRUE;
            PlatformMoveMemory(&Tab->TempCredential[i].Profile, pProfile, sizeof(WSC_CREDENTIAL));
            PlatformMoveMemory(&Tab->TempCredential[i].InterAddr, pAddr, MAC_ADDR_LEN);
        }
    }
    else if (index < MAX_P2P_SAVECREDN_SIZE)
    {
        DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednTabInsert Update Existing %d\n", index ));
        Tab->TempCredential[index].bValid = TRUE;
        PlatformMoveMemory(&Tab->TempCredential[index].Profile, pProfile, sizeof(WSC_CREDENTIAL));
        PlatformMoveMemory(&Tab->TempCredential[index].InterAddr, pAddr, MAC_ADDR_LEN);
    }
}

/*  
    ==========================================================================
    Description: 
        SEarch an entry  in P2P Persistent Table.  All parameters must the same.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pCrednEntrySearch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr,
    IN PUCHAR   ResultIndex) 
{

    UCHAR   i;
    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    BOOLEAN     bFind = FALSE;
    UCHAR           Allff[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    UCHAR           AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    if ((Addr[0] & 0x1) == 0x1)
        return bFind;
    
    if (PlatformEqualMemory(Allff, Addr, MAC_ADDR_LEN))
        return bFind;

    if (PlatformEqualMemory(AllZero, Addr, MAC_ADDR_LEN))
        return bFind;

    DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednEntrySearch \n"));
    for (i = 0; i < MAX_P2P_SAVECREDN_SIZE; i++)
    {       
        if ((Tab->TempCredential[i].bValid == TRUE) 
            && PlatformEqualMemory(&Tab->TempCredential[i].InterAddr, Addr, MAC_ADDR_LEN))
        {
            DBGPRINT(RT_DEBUG_TRACE, (" Find Credential Entry - i = %d. \n", i));
            bFind = TRUE;
            *ResultIndex = i;
        }
    }
    
    return bFind;
}

/*  
    ==========================================================================
    Description: 
        [Win8] Init P2P Discovery Table.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pDiscoTabInit(
    IN PMP_ADAPTER pAd) 
{

    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR       i;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pDiscoTabInit .  \n"));
    for (i = 0; i < MAX_P2P_DISCOVERY_SIZE; i++)
    {
        Tab->DiscoEntry[i].bValid = FALSE;
        PlatformZeroMemory(Tab->DiscoEntry[i].Bssid, MAC_ADDR_LEN);
        Tab->DiscoEntry[i].BssType = 0;
        PlatformZeroMemory(Tab->DiscoEntry[i].TransmitterAddr, MAC_ADDR_LEN);
        Tab->DiscoEntry[i].Rssi = 0;
        Tab->DiscoEntry[i].BeaconPeriod = 0;
        Tab->DiscoEntry[i].BeaconTimestamp = 0;
        Tab->DiscoEntry[i].ProbeRspTimestamp = 0;
        Tab->DiscoEntry[i].CapabilityInfo = 0;
        PlatformZeroMemory(Tab->DiscoEntry[i].BeaconFrame, MAX_LEN_OF_MLME_BUFFER);
        Tab->DiscoEntry[i].BeaconFrameLen = 0;
        PlatformZeroMemory(Tab->DiscoEntry[i].ProbeRspFrame, MAX_LEN_OF_MLME_BUFFER);
        Tab->DiscoEntry[i].ProbeRspFrameLen = 0;
        Tab->DiscoEntry[i].ListenChannel = 0;   
        Tab->DiscoEntry[i].OpChannel = 0;
        PlatformZeroMemory(Tab->DiscoEntry[i].DeviceId, MAC_ADDR_LEN);
        PlatformZeroMemory(Tab->DiscoEntry[i].Ssid, MAX_LEN_OF_SSID);
        Tab->DiscoEntry[i].SsidLen = 0;
        PlatformZeroMemory(Tab->DiscoEntry[i].PrimaryDevType, P2P_DEVICE_TYPE_LEN);
        Tab->DiscoEntry[i].NumSecondaryType = 0;
        PlatformZeroMemory(Tab->DiscoEntry[i].SecondaryDevType, MAX_P2P_SECONDARY_DEVTYPE_LIST * P2P_DEVICE_TYPE_LEN);      
        Tab->DiscoEntry[i].LifeTime = 0;
        Tab->DiscoEntry[i].bLegacyNetworks = FALSE;
        Tab->DiscoEntry[i].GroupChannel = 0;
        Tab->DiscoEntry[i].DevCapability = 0;
        Tab->DiscoEntry[i].GroupCapability = 0;     
    }
    Tab->DiscoNumber = 0;
    return;
}


/*  
    ==========================================================================
    Description: 
        Search a peer in P2P Discovery Table by the specific filter  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pDiscoTabSearch(
    IN PMP_ADAPTER    pAd, 
    IN PUCHAR           TxAddr,
    IN PUCHAR           DevAddr,
    OUT UCHAR           DevIndexArray[]) 
{

    UCHAR           i;
    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR           index = P2P_NOT_FOUND;
    UCHAR           Allff[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    UCHAR           FoundDeviceCount = 0;
    
    // Searching by transmitter address
    if (TxAddr != NULL)
    {
        // find addr can't be 1. multicast, 2. all zero, 3. all 0xff 
        if ((TxAddr[0] & 0x1) == 0x1)
            return index;
        
        if (PlatformEqualMemory(Allff, TxAddr, MAC_ADDR_LEN))
            return index;

        if (PlatformEqualMemory(ZERO_MAC_ADDR, TxAddr, MAC_ADDR_LEN))
            return index;

        for (i = 0; i < MAX_P2P_DISCOVERY_SIZE; i++)
        {       
            if ((Tab->DiscoEntry[i].bValid == TRUE) && 
                (PlatformEqualMemory(Tab->DiscoEntry[i].TransmitterAddr, TxAddr, MAC_ADDR_LEN)))
                index = i;
        }
        
        return index;
    }

    // Searching by device id
    if (DevAddr != NULL)
    {
        // find addr can't be 1. multicast, 2. all zero, 3. all 0xff 
        if ((DevAddr[0] & 0x1) == 0x1)
            return index;
        
        if (PlatformEqualMemory(Allff, DevAddr, MAC_ADDR_LEN))
            return index;

        if (PlatformEqualMemory(ZERO_MAC_ADDR, DevAddr, MAC_ADDR_LEN))
            return index;

        FoundDeviceCount = 0;
        for (i = 0; i < MAX_P2P_DISCOVERY_SIZE; i++)
        {       
            if ((Tab->DiscoEntry[i].bValid == TRUE) && 
                (PlatformEqualMemory(Tab->DiscoEntry[i].DeviceId, DevAddr, MAC_ADDR_LEN)))
            {
                index = i;
                // max support num = 6
                if (DevIndexArray != NULL)
                {
                    if ((FoundDeviceCount ++) > 6)
                        break;
                    
                    DevIndexArray[FoundDeviceCount-1] = i;          
                }   
            }
        }
        
        return index;
    }

    return index;
}

/*  
    ==========================================================================
    Description: 
        Delete a peer in P2P Discovery Table.  
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pDiscoTabDelete(
    IN PMP_ADAPTER pAd, 
    IN UCHAR        p2pindex, 
    IN PUCHAR       Addr) 
{

    UCHAR                       index = 0xff;
    PRT_P2P_DISCOVERY_ENTRY     pP2pEntry;

    if ((p2pindex >= MAX_P2P_DISCOVERY_SIZE) && (Addr != NULL))
        index = P2pDiscoTabSearch(pAd, Addr, NULL, NULL);
    else
        index = p2pindex;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pDiscoTabDelete . index = %d. \n", index));
    if (index < MAX_P2P_DISCOVERY_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.DiscoEntry[index];

        DBGPRINT(RT_DEBUG_TRACE,("P2pDiscoTabDelete  index %d.  addr is %x %x %x %x %x %x\n", index, 
                pP2pEntry->TransmitterAddr[0], pP2pEntry->TransmitterAddr[1], pP2pEntry->TransmitterAddr[2], 
                pP2pEntry->TransmitterAddr[3], pP2pEntry->TransmitterAddr[4], pP2pEntry->TransmitterAddr[5]));

        PlatformZeroMemory(pP2pEntry->Bssid, MAC_ADDR_LEN);
        pP2pEntry->BssType = 0;
        PlatformZeroMemory(pP2pEntry->TransmitterAddr, MAC_ADDR_LEN);
        pP2pEntry->Rssi = 0;
        pP2pEntry->BeaconPeriod = 0;
        pP2pEntry->BeaconTimestamp = 0;
        pP2pEntry->ProbeRspTimestamp = 0;
        pP2pEntry->CapabilityInfo = 0;
        PlatformZeroMemory(pP2pEntry->BeaconFrame, MAX_LEN_OF_MLME_BUFFER);
        pP2pEntry->BeaconFrameLen = 0;
        PlatformZeroMemory(pP2pEntry->ProbeRspFrame, MAX_LEN_OF_MLME_BUFFER);
        pP2pEntry->ProbeRspFrameLen = 0;
        pP2pEntry->ListenChannel = 0;
        pP2pEntry->OpChannel = 0;   
        PlatformZeroMemory(pP2pEntry->DeviceId, MAC_ADDR_LEN);
        PlatformZeroMemory(pP2pEntry->Ssid, MAX_LEN_OF_SSID);
        pP2pEntry->SsidLen = 0;
        PlatformZeroMemory(pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
        pP2pEntry->NumSecondaryType = 0;
        PlatformZeroMemory(pP2pEntry->SecondaryDevType, MAX_P2P_SECONDARY_DEVTYPE_LIST * P2P_DEVICE_TYPE_LEN);      
        pP2pEntry->LifeTime = 0;
        pP2pEntry->bLegacyNetworks = FALSE;
        pP2pEntry->GroupChannel = 0;
        pP2pEntry->DevCapability = 0;
        pP2pEntry->GroupCapability = 0;     
        
        if ((pAd->pP2pCtrll->P2PTable.DiscoNumber > 0) && (pP2pEntry->bValid == TRUE))
            pAd->pP2pCtrll->P2PTable.DiscoNumber--;
        
        pP2pEntry->bValid = FALSE;
    }

    return index;
}


/*  
    ==========================================================================
    Description: 
        Insert/Update a peer to P2P Discovery Table by its transmitter address when do discovery scan. 
        Because this Peer contains P2P IE and P2P Wildwork SSID to indicate that it support P2P.        
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
UCHAR P2pDiscoTabInsert(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    TxAddr,
    IN PUCHAR    DevAddr,
    IN UCHAR Ssid[], 
    IN UCHAR SsidLen)
{

    PRT_P2P_TABLE   Tab = &pAd->pP2pCtrll->P2PTable;
    UCHAR       i;
    
    if (PlatformEqualMemory(ZERO_MAC_ADDR, TxAddr, MAC_ADDR_LEN))
    {
        DBGPRINT(RT_DEBUG_ERROR,("P2pDiscoTabInsert . Addr all zero Error. \n"));
        return P2P_NOT_FOUND;
    }
    if ((TxAddr[0] & 0x1) == 0x1)
    {
        DBGPRINT(RT_DEBUG_ERROR,("P2pDiscoTabInsert . Insert mcast Addr Error. \n"));
        return P2P_NOT_FOUND;
    }
    
    for (i = 0; i < MAX_P2P_DISCOVERY_SIZE; i++)
    {       
        // This peer already exist, so only update state.
        if ((Tab->DiscoEntry[i].bValid == TRUE) 
            && (PlatformEqualMemory(Tab->DiscoEntry[i].TransmitterAddr, TxAddr, MAC_ADDR_LEN)))
        {
            if (DevAddr != NULL)
            {
                if (!PlatformEqualMemory(Tab->DiscoEntry[i].DeviceId, DevAddr, MAC_ADDR_LEN))
                {
                    DBGPRINT(RT_DEBUG_TRACE,("P2pDiscoTabInsert . Device address changes!!! \n"));
                    PlatformMoveMemory(Tab->DiscoEntry[i].DeviceId, DevAddr, MAC_ADDR_LEN);
                }   
            }   

            if ((SsidLen > 0) && (Ssid != NULL))
                PlatformMoveMemory(Tab->DiscoEntry[i].Ssid, Ssid, MAX_LEN_OF_SSID);
            Tab->DiscoEntry[i].SsidLen = SsidLen;
            
            return i;
        }
        // Insert new one
        else if (Tab->DiscoEntry[i].bValid == FALSE)
        {
            Tab->DiscoNumber++;
            PlatformMoveMemory(Tab->DiscoEntry[i].TransmitterAddr, TxAddr, MAC_ADDR_LEN);
            
            DBGPRINT(RT_DEBUG_INFO,("P2pDiscoTabInsert . Arrd[5] = 0x%x. \n", TxAddr[5]));          

            if (DevAddr != NULL)
                PlatformMoveMemory(Tab->DiscoEntry[i].DeviceId, DevAddr, MAC_ADDR_LEN);
            
            if ((SsidLen > 0) && (Ssid != NULL))
                PlatformMoveMemory(Tab->DiscoEntry[i].Ssid, Ssid, MAX_LEN_OF_SSID);
            Tab->DiscoEntry[i].SsidLen = SsidLen;

            return i;
        }
    }
    return P2P_NOT_FOUND;
}

