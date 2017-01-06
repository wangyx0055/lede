/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    auth_rsp.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    John        2004-10-1       copy from RT2560
*/
#include "MtConfig.h"

/*
    ==========================================================================
    Description:
        authentication state machine init procedure
    Parameters:
        Sm - the state machine

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID MlmeAuthRspStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN PSTATE_MACHINE Sm, 
    IN STATE_MACHINE_FUNC Trans[]) 
{
    LARGE_INTEGER        NOW;

    StateMachineInit(Sm, Trans, MAX_AUTH_RSP_STATE, MAX_AUTH_RSP_MSG, Drop, AUTH_RSP_IDLE, AUTH_RSP_MACHINE_BASE);

    // column 1
    StateMachineSetAction(Sm, AUTH_RSP_IDLE, MT2_PEER_DEAUTH, MlmeAuthRspPeerDeauthAction);

    // column 2
    StateMachineSetAction(Sm, AUTH_RSP_WAIT_CHAL, MT2_PEER_DEAUTH, MlmeAuthRspPeerDeauthAction);

    // initialize the random number generator
    NdisGetCurrentSystemTime(&NOW);
    LfsrInit(pAd, NOW.LowPart);
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL

    ==========================================================================
*/
VOID MlmeAuthRspPeerDeauthAction(
    IN PMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem) 
{
    UCHAR       Addr2[MAC_ADDR_LEN];
    USHORT      Reason;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    UCHAR       P2pMinorReason = 0xff;
    UCHAR       Channel;

    if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason)) 
    {
        if (P2P_ON(pPort))
        {
            P2pParseManageSubElmt(pAd, pPort, &Elem->Msg[LENGTH_802_11+2], (Elem->MsgLen-LENGTH_802_11-2), &Channel, NULL, NULL, NULL, &P2pMinorReason);
            // If minor reason is valid, save minor reason.
            if (P2pMinorReason != 0xff)
            {
                pPort->P2PCfg.P2pManagedParm.APP2pMinorReason = P2pMinorReason; 
                if ((P2pMinorReason == MINOR_REASON_BCZ_MANAGED_BIT_ZERO) 
                    || (P2pMinorReason == MINOR_REASON_OUTSIDE_IT_DEFINED))
                    pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter = 150;
            }

            if(/*P2P_ON(pPort) &&*/ (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1) && (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
            pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 10;
            
            DBGPRINT(RT_DEBUG_TRACE,("AUTH_RSP - receive DE-AUTH from AP (P2pMinorReason = %d) (PortNum=%d)\n", P2pMinorReason, Elem->PortNum));
        }

        pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
        if (INFRA_ON(pPort) && MAC_ADDR_EQUAL(Addr2, pPort->PortCfg.Bssid)) 
        {
            DBGPRINT(RT_DEBUG_TRACE,("AUTH_RSP - receive DE-AUTH from our AP (Reason=%d)(PortNum=%d)\n", Reason, Elem->PortNum));

            if((Reason == REASON_P2P_PEER_MANUAL_DEAUTH) && IS_P2P_SIGMA_OFF(pPort) && P2P_ON(pPort) && (pPort->CommonCfg.P2pControl.field.EnablePresistent == TRUE))
            {
                
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_PEER_MANUAL_DISCONNECTION;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {   
                    PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.DisplayPin.MacAddr, Addr2, 6);
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_PEER_MANUAL_DISCONNECTION;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    DBGPRINT(RT_DEBUG_TRACE,("P2pKeSetEvent %d", P2PEVENT_PEER_MANUAL_DISCONNECTION));
                    
                }
                
            }

            if((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) &&
                (pPort->StaCfg.WscControl.bSkipWPSTurnOffLEDAfterSuccess == FALSE))
            {
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                pPort->StaCfg.WscControl.PeerDisconnect = TRUE;
            }

            if(((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_6) &&
                (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
            {
                LedCtrlSetBlinkMode(pAd, LED_LINK_DOWN);
                pPort->StaCfg.WscControl.PeerDisconnect = TRUE;
            }

            //pAd->MlmeAux.AutoReconnectSsidLen= 32;
            //PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

            pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
            PlatformIndicateDisassociation(pAd, pPort,Addr2, DOT11_ASSOC_STATUS_PEER_DEAUTHENTICATED_START | Reason);
            DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  MlmeAuthRspPeerDeauthAction, PlatformIndicateDisassociation Reason=0x%08x, [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    Reason | DOT11_ASSOC_STATUS_PEER_DISASSOCIATED_START, Addr2[0], Addr2[1], 
                    Addr2[2], Addr2[3], Addr2[4], Addr2[5]));       

            if (FALSE == MlmeCheckForRoaming(pAd,pPort, pAd->Mlme.Now64))
            {           
                DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - receive DE-AUTH from our AP (Reason=%d)\n", Reason));
                MlmeCntLinkDown(pPort, TRUE);

                NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->Mlme.Now64);             
            }   
            pPort->StaCfg.WscControl.PeerDisconnect = FALSE;
        }
        else if(ADHOC_ON(pPort))
        {
            MlmeInfoMantainAdhcoList(pAd, pPort, Addr2);
            //2 need to check if needed for Ndtest ?? 
#if 0
            DBGPRINT(RT_DEBUG_TRACE,("IN Adhoc AUTH_RSP - MlmeAuthRspPeerDeauthAction()\n"));
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeAuthRspPeerDeauthAction, addr2 =[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    Addr2[0], Addr2[1], 
                    Addr2[2], Addr2[3], Addr2[4], Addr2[5]));       

            MlmeInfoMantainAdhcoList(pAd,Addr2);
#endif
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,("AUTH_RSP - MlmeAuthRspPeerDeauthAction() sanity check fail\n"));
    }
}

