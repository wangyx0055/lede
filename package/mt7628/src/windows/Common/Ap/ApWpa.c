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
    wpa.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Jan Lee     03-07-22        Initial
    Rory Chen   04-11-29        Add WPA2PSK
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x00700000

UCHAR   OUI_WPA_WEP40[4] = {0x00, 0x50, 0xF2, 0x01};
UCHAR   OUI_WPA_TKIP[4] = {0x00, 0x50, 0xF2, 0x02};
UCHAR   OUI_WPA_CCMP[4] = {0x00, 0x50, 0xF2, 0x04};
UCHAR   OUI_WPA2_WEP40[4] = {0x00, 0x0F, 0xAC, 0x01};
UCHAR   OUI_WPA2_TKIP[4] = {0x00, 0x0F, 0xAC, 0x02};
UCHAR   OUI_WPA2_CCMP[4] = {0x00, 0x0F, 0xAC, 0x04};

BOOLEAN ApWpaMsgTypeSubst(
    IN UCHAR    EAPType,
    OUT ULONG   *MsgType) 
{
    switch(EAPType)
    {
        case EAPPacket:
            *MsgType = APMT2_EAPPacket;
            break;
        case EAPOLStart:
            *MsgType = APMT2_EAPOLStart;
            break;
        case EAPOLLogoff:
            *MsgType = APMT2_EAPOLLogoff;
            break;
        case EAPOLKey:
            *MsgType = APMT2_EAPOLKey;
            break;
        case EAPOLASFAlert:
            *MsgType = APMT2_EAPOLASFAlert;
            break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaMsgTypeSubst : return FALSE; \n"));    
            return FALSE;
    }

    return TRUE;
}

/*  
    ==========================================================================
    Description: 
        association state machine init, including state transition and timer init
    Parameters: 
        S - pointer to the association state machine
    ==========================================================================
 */
VOID ApWpaStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(S, Trans, AP_MAX_WPA_PTK_STATE, AP_MAX_WPA_MSG, Drop, AP_WPA_PTK, AP_WPA_MACHINE_BASE);

    StateMachineSetAction(S, AP_WPA_PTK, APMT2_EAPPacket, ApWpaEAPPacketAction);
    StateMachineSetAction(S, AP_WPA_PTK, APMT2_EAPOLStart, ApWpaEAPOLStartAction);
    StateMachineSetAction(S, AP_WPA_PTK, APMT2_EAPOLLogoff, ApWpaEAPOLLogoffAction);
    StateMachineSetAction(S, AP_WPA_PTK, APMT2_EAPOLKey, ApWpaEAPOLKeyAction);
    StateMachineSetAction(S, AP_WPA_PTK, APMT2_EAPOLASFAlert, ApWpaEAPOLASFAlertAction);
}

/*
    ==========================================================================
    Description:
        this is state machine function. 
        When receiving EAP packets which is  for 802.1x authentication use. 
        Not use in PSK case
    Return:
    ==========================================================================
*/
VOID ApWpaEAPPacketAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}

VOID ApWpaEAPOLASFAlertAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}

VOID ApWpaEAPOLLogoffAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}

/*
    ==========================================================================
    Description:
   
    Return:
         TRUE if this is EAP frame
         FALSE otherwise
    ==========================================================================
*/
BOOLEAN ApWpaCheckWPAframe(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR           pData,
    IN ULONG            DataByteCount)
{

    if(DataByteCount < (LENGTH_802_1_H +LENGTH_EAPOL_H + 2))
        return FALSE;

    DBGPRINT(RT_DEBUG_INFO, ("ApWpaCheckWPAframe ===> \n"));    

    DBGPRINT_RAW(RT_DEBUG_INFO, ("data = %02x:%02x:%02x:%02x-%02x:%02x:%02x:%02x\n",                                        
                                                *pData, *(pData+1),*(pData+2),*(pData+3),*(pData+4),*(pData+5),*(pData+6),*(pData+7)));
                                        

    if (PlatformEqualMemory(SNAP_802_1H, pData, 6)) 
    {
        pData += 6;
    }
    else if (PlatformEqualMemory(SNAP_AMP_LLC, pData, 6))
    {
        pData += 6;
    }

    if (PlatformEqualMemory(EAPOL, pData, 2)) 
    {
        pData += 2;         
    }   
    else    
        return FALSE;

    switch (*(pData+1))     
    {   
        case EAPPacket:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAP-Packet frame, TYPE = 0 \n"));
            break;
        case EAPOLStart:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL-Start frame, TYPE = 1 \n"));
            break;
        case EAPOLLogoff:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOLLogoff frame, TYPE = 2 \n"));
            break;
        case EAPOLKey:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL-Key frame, TYPE = 3, Length =%x\n", *(pData+2)));
            break;
        case EAPOLASFAlert:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOLASFAlert frame, TYPE = 4 \n"));
            break;
        default:
            return FALSE;
    }   
    return TRUE;
}

/*
    ==========================================================================
    Description:
       Check sanity of multicast cipher selector in RSN IE.
    Return:
         TRUE if match
         FALSE otherwise
    ==========================================================================
*/
BOOLEAN ApWpaCheckMcast(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PEID_STRUCT      eid_ptr,
    IN MAC_TABLE_ENTRY  *pEntry)
{
    pEntry->AuthMode = pPort->PortCfg.AuthMode;

    if (eid_ptr->Len >= 6)
    {
        // WPA and WPA2 format not the same in RSN_IE
        if ((eid_ptr->Eid == IE_WPA) && (PlatformEqualMemory(eid_ptr->Octet, WPA_OUI, 4)))
        {
            if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2)
                pEntry->AuthMode = Ralink802_11AuthModeWPA;
            else if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK)
                pEntry->AuthMode = Ralink802_11AuthModeWPAPSK;

            if (PlatformEqualMemory(&eid_ptr->Octet[6], &pPort->SoftAP.ApCfg.RSN_IE[0][6], 4))
                return TRUE;
        }
        else if (eid_ptr->Eid == IE_WPA2)
        {
            UCHAR   IE_Idx = 0;

            // When WPA1/WPA2 mix mode, the RSN_IE is stored in different structure
            if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2) || 
                (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
                IE_Idx = 1;

            if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2)
                pEntry->AuthMode = Ralink802_11AuthModeWPA2;
            else if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK)
                pEntry->AuthMode = Ralink802_11AuthModeWPA2PSK;

            if (PlatformEqualMemory(&eid_ptr->Octet[2], &pPort->SoftAP.ApCfg.RSN_IE[IE_Idx][2], 4))
                return TRUE;
        }
    }

    DBGPRINT(RT_DEBUG_ERROR, ("ApWpaCheckMcast ==> WPAIE = %d\n", eid_ptr->Eid));

    return FALSE;
}

/*
    ==========================================================================
    Description:
       Check sanity of unicast cipher selector in RSN IE.
    Return:
         TRUE if match
         FALSE otherwise
    ==========================================================================
*/
BOOLEAN ApWpaCheckUcast(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PEID_STRUCT      eid_ptr,
    IN MAC_TABLE_ENTRY  *pEntry)
{
    pEntry->WepStatus = pPort->PortCfg.WepStatus;

    if (eid_ptr->Len >= 16)
    {
        if ((eid_ptr->Eid == IE_WPA) && (PlatformEqualMemory(eid_ptr->Octet, WPA_OUI, 4)))
        {
            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)
            {// multiple cipher (TKIP/CCMP)
                if (PlatformEqualMemory(&eid_ptr->Octet[12], &pPort->SoftAP.ApCfg.RSN_IE[0][12], 4))
                {
                    pEntry->WepStatus = Ralink802_11Encryption2Enabled;
                    return TRUE;
                }
                else if (PlatformEqualMemory(&eid_ptr->Octet[12], &pPort->SoftAP.ApCfg.RSN_IE[0][16], 4))
                {
                    pEntry->WepStatus = Ralink802_11Encryption3Enabled;
                    return TRUE;
                }
            }
            else
            {// single cipher
                if (PlatformEqualMemory(&eid_ptr->Octet[12], &pPort->SoftAP.ApCfg.RSN_IE[0][12], 4))
                    return TRUE;
            }
        }
        else if (eid_ptr->Eid == IE_WPA2)
        {
            UCHAR   IE_Idx = 0;

            // When WPA1/WPA2 mix mode, the RSN_IE is stored in different structure
            if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2) || 
                (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
                IE_Idx = 1;

            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)
            {// multiple cipher (TKIP/CCMP)
                if (PlatformEqualMemory(&eid_ptr->Octet[8], &pPort->SoftAP.ApCfg.RSN_IE[IE_Idx][8], 4))
                {
                    pEntry->WepStatus = Ralink802_11Encryption2Enabled;
                    return TRUE;
                }
                else if (PlatformEqualMemory(&eid_ptr->Octet[8], &pPort->SoftAP.ApCfg.RSN_IE[IE_Idx][12], 4))
                {
                    pEntry->WepStatus = Ralink802_11Encryption3Enabled;
                    return TRUE;
                }
            }
            else
            {
                // single cipher
                if (PlatformEqualMemory(&eid_ptr->Octet[8], &pPort->SoftAP.ApCfg.RSN_IE[IE_Idx][8], 4))
                    return TRUE;
            }
        }
    }

    DBGPRINT(RT_DEBUG_ERROR, ("ApWpaCheckUcast ==> WPAIE = %d\n", eid_ptr->Eid));

    return FALSE;
}

/*
    ==========================================================================
    Description:
       Check sanity of authentication method selector in RSN IE.
    Return:
         TRUE if match
         FALSE otherwise
    ==========================================================================
*/
BOOLEAN ApWpaCheckAuth(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PEID_STRUCT      eid_ptr,
    IN MAC_TABLE_ENTRY  *pEntry)
{
    if (eid_ptr->Len >= 16)
    {
        if ((eid_ptr->Eid == IE_WPA) && (PlatformEqualMemory(eid_ptr->Octet, WPA_OUI, 4)))
        {
            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)
            {
                if (PlatformEqualMemory(&eid_ptr->Octet[18], &pPort->SoftAP.ApCfg.RSN_IE[0][18], 4))
                    return TRUE;
                else if (PlatformEqualMemory(&eid_ptr->Octet[18], &pPort->SoftAP.ApCfg.RSN_IE[0][22], 4))
                    return TRUE;
            }
            else
            {
                if (PlatformEqualMemory(&eid_ptr->Octet[18], &pPort->SoftAP.ApCfg.RSN_IE[0][18], 4))
                    return TRUE;
            }
        }
        else if (eid_ptr->Eid == IE_WPA2)
        {
            UCHAR   IE_Idx = 0;

            // When WPA1/WPA2 mix mode, the RSN_IE is stored in different structure
            if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2) || 
                (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
                IE_Idx = 1;

            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)
            {
                if (PlatformEqualMemory(&eid_ptr->Octet[14], &pPort->SoftAP.ApCfg.RSN_IE[IE_Idx][14], 4))
                    return TRUE;
                else if (PlatformEqualMemory(&eid_ptr->Octet[14], &pPort->SoftAP.ApCfg.RSN_IE[IE_Idx][18], 4))
                    return TRUE;
            }
            else
            {
                if (PlatformEqualMemory(&eid_ptr->Octet[14], &pPort->SoftAP.ApCfg.RSN_IE[IE_Idx][14], 4))
                    return TRUE;
            }
        }
    }

    DBGPRINT(RT_DEBUG_ERROR, ("ApWpaCheckAuth ==> WPAIE=%d, WepStatus=%s\n", eid_ptr->Eid, decodeCipherAlgorithm(pEntry->WepStatus)));

    return FALSE;
}


/*
    ==========================================================================
    Description:
        Port Access Control Inquiry function. Return entry's Privacy and Wpastate.
    Return:
        pEntry 
    ==========================================================================
*/
MAC_TABLE_ENTRY *ApWpaPACInquiry(
    IN  PMP_ADAPTER               pAd, 
    IN  PUCHAR                      pAddr, 
    OUT NDIS_802_11_PRIVACY_FILTER  *Privacy,
    OUT AP_WPA_STATE                *WpaState)
{

    *Privacy = Ndis802_11PrivFilterAcceptAll;
    *WpaState = AS_NOTUSE;
    
    if (pAddr[0] & 0x01)
    {// mcast & broadcast address
        *Privacy = Ndis802_11PrivFilterAcceptAll;
        *WpaState = AS_NOTUSE;
    } 
    
    return NULL;
}

/*
    ==========================================================================
    Description:
       Start 4-way HS when rcv EAPOL_START which may create by our driver in assoc.c
    Return:
    ==========================================================================
*/
VOID ApWpaEAPOLStartAction(
    IN PMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem) 
{
    MAC_TABLE_ENTRY *pEntry;
    PHEADER_802_11  pHeader;
    BOOLEAN         Cancelled;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];

    DBGPRINT(RT_DEBUG_TRACE, ("ApWpaEAPOLStartAction ===> %x %x %x %x %x %x \n", Elem->Msg[0], Elem->Msg[1],Elem->Msg[2],Elem->Msg[3],Elem->Msg[4],Elem->Msg[5]));

    // Delay for avoiding the misorder of EAPOL-KEY and Assoc Rsp.
    Delay_us(200000);//200ms

    pHeader = (PHEADER_802_11)Elem->Msg;

    //For normaol PSK, we enqueue an EAPOL-Start command to trigger the process.
    if (Elem->MsgLen == 6)
        pEntry = MacTableLookup(pAd, pPort, Elem->Msg);
    else
        pEntry = MacTableLookup(pAd, pPort, pHeader->Addr2);

    if (pEntry) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - PortSecured = %d . AuthMode = %d. ()\n", pEntry->PortSecured, pEntry->AuthMode ));
        if ((pEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
            && ((pEntry->AuthMode == Ralink802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK)))
        {
            pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
            pEntry->WpaState = AS_INITPSK;
            pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
            PlatformZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
            pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;

            ApWpaStart4WayHS(pAd, pPort, pEntry);
            PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);
            PlatformSetTimer(pPort, &pEntry->RetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
        }
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - cann't find pEntry  ()\n"));

}

/*
    ==========================================================================
    Description:
        Function to handle countermeasures active attack.  Init 60-sec timer if necessary.
    Return:
    ==========================================================================
*/
VOID ApWpaHandleCounterMeasure(
    IN PMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry) 
{
    BOOLEAN     Cancelled;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;    
    PQUEUE_HEADER pHeader;
   
    PMP_PORT  pPort = pAd->PortList[PORT_0];
    if (!pEntry)
        return;

    // record which entry causes this MIC error, if this entry sends disauth/disassoc, AP doesn't need to log the CM
    pEntry->CMTimerRunning = TRUE;
    pPort->SoftAP.ApCfg.MICFailureCounter++;

    if (pPort->SoftAP.ApCfg.CMTimerRunning == TRUE)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Receive CM Attack Twice within 60 seconds ====>>> \n"));

        // renew GTK
        ApWpaGenRandom(pAd, pPort, pPort->SoftAP.ApCfg.GNonce);
        ApLogEvent(pAd, pEntry->Addr, EVENT_COUNTER_M);
        PlatformCancelTimer(&pPort->SoftAP.ApCfg.CounterMeasureTimer, &Cancelled);
        
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
                pEntry = NULL;
                continue; 
            }
            
            // happened twice within 60 sec,  AP SENDS disaccociate all associated STAs.  All STA's transition to State 2
            if (pMacEntry->ValidAsCLI == TRUE)
            {
                ApWpaDisAssocAction(pAd, pPort,pMacEntry, REASON_MIC_FAILURE);
            }
            
            pNextMacEntry = pNextMacEntry->Next;    
            pMacEntry = NULL;
        }

        // Further,  ban all Class 3 DATA transportation for  a period 0f 60 sec
        // disallow new association , too
        pPort->SoftAP.ApCfg.BANClass3Data = TRUE;        

        // check how many entry left...  should be zero
        pPort->SoftAP.ApCfg.GKeyDoneStations = pPort->MacTab.Size;
        DBGPRINT(RT_DEBUG_TRACE, ("pPort->SoftAP.ApCfg.GKeyDoneStations=%d \n", pPort->SoftAP.ApCfg.GKeyDoneStations));
    }

    PlatformSetTimer(pPort, &pPort->SoftAP.ApCfg.CounterMeasureTimer, 60 * MLME_TASK_EXEC_INTV);
    pPort->SoftAP.ApCfg.CMTimerRunning = TRUE;
    pPort->SoftAP.ApCfg.PrevaMICFailTime = pPort->SoftAP.ApCfg.aMICFailTime;
    NdisGetCurrentSystemTime(&pPort->SoftAP.ApCfg.aMICFailTime);
}

/*
    ==========================================================================
    Description:
        This is state machine function. 
        When receiving EAPOL packets which is  for 802.1x key management. 
        Use both in WPA, and WPAPSK case. 
        In this function, further dispatch to different functions according to the received packet.  3 categories are : 
          1.  normal 4-way pairwisekey and 2-way groupkey handshake
          2.  MIC error (Countermeasures attack)  report packet from STA.
          3.  Request for pairwise/group key update from STA
    Return:
    ==========================================================================
*/
VOID ApWpaEAPOLKeyAction(
    IN PMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem) 
{
    BOOLEAN             Cancelled;
    MAC_TABLE_ENTRY     *pEntry;
    PHEADER_802_11      pHeader;
    PAP_EAPOL_PACKET    pEapol_packet;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];

    DBGPRINT(RT_DEBUG_INFO, ("ApWpaEAPOLKeyAction ===>\n"));

    pHeader = (PHEADER_802_11)Elem->Msg;
    pEapol_packet = (PAP_EAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];

    do
    {
        if ((pAd->OpMode == OPMODE_AP) 
            && ((pPort->PortCfg.AuthMode < Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeAutoSwitch)))
            break;

        pEntry = MacTableLookup(pAd, pPort, pHeader->Addr2);
        if (!pEntry)
            break;

        if (((pEapol_packet->ProVer != EAPOL_VER) && (pEapol_packet->ProVer != EAPOL_VER_2)) || ((pEapol_packet->KeyDesc.Type != WPA1_KEY_DESC) && (pEapol_packet->KeyDesc.Type != WPA2_KEY_DESC)))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Key descripter does not match with WPA rule\n"));
            break;
        }

        if ((pEntry->WepStatus == Ralink802_11Encryption2Enabled) && (pEapol_packet->KeyDesc.Keyinfo.KeyDescVer != DESC_TYPE_TKIP))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(TKIP) \n"));
            break;
        }
        else if ((pEntry->WepStatus == Ralink802_11Encryption3Enabled) && (pEapol_packet->KeyDesc.Keyinfo.KeyDescVer != DESC_TYPE_AES))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(AES) \n"));
            break;
        }

        if ((pEntry->Sst == SST_ASSOC) && (pEntry->WpaState >= AS_INITPSK))
        {
            if ((pEapol_packet->KeyDesc.Keyinfo.KeyMic) && (pEapol_packet->KeyDesc.Keyinfo.Request) && (pEapol_packet->KeyDesc.Keyinfo.Error))
            {
                // KEYMIC=1, REQUEST=1, ERROR=1
                DBGPRINT(RT_DEBUG_ERROR, ("MIC, REQUEST, ERROR  are all 1, active countermeasure \n"));
                ApWpaHandleCounterMeasure(pAd, pEntry);
            }
            else if ((pEapol_packet->KeyDesc.Keyinfo.Secure) && !(pEapol_packet->KeyDesc.Keyinfo.Request) && !(pEapol_packet->KeyDesc.Keyinfo.Error))
            {
                // SECURE=1, REQUEST=0, ERROR=0
                if ((pEntry->AuthMode == Ralink802_11AuthModeWPA) || (pEntry->AuthMode == Ralink802_11AuthModeWPAPSK))
                {
                    ApWpaPeerGroupMsg2Action(pAd, pEntry, &Elem->Msg[LENGTH_802_11], (Elem->MsgLen - LENGTH_802_11));
                }
                else if (((pEntry->AuthMode == Ralink802_11AuthModeWPA2) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK)) // WPA2-XXX during rekey procedure.
                        && (pEntry->WpaState >=AS_PTKINITDONE) && (pEntry->GTKState == REKEY_NEGOTIATING))
                {
                    ApWpaPeerGroupMsg2Action(pAd, pEntry, &Elem->Msg[LENGTH_802_11], (Elem->MsgLen - LENGTH_802_11));
                }
                else if ((pEntry->AuthMode == Ralink802_11AuthModeWPA2) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK))
                {
                    ApWpa2PeerPairMsg4Action(pAd, pEntry, Elem);
                }
            }
            else if (!(pEapol_packet->KeyDesc.Keyinfo.Secure) && !(pEapol_packet->KeyDesc.Keyinfo.Request) && !(pEapol_packet->KeyDesc.Keyinfo.Error))
            {
                // SECURE=0, REQUEST=0, ERROR=0
                if (pEntry->WpaState == AS_PTKSTART)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Call ApWpaPeerPairMsg2Action.\n", __FUNCTION__));
                    ApWpaPeerPairMsg2Action(pAd, pEntry, Elem);
                }
                else if (pEntry->WpaState == AS_PTKINIT_NEGOTIATING)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Call ApWpa1PeerPairMsg4Action.\n", __FUNCTION__));
                    ApWpa1PeerPairMsg4Action(pAd, pEntry, Elem);
                }
            }
            else if ((pEapol_packet->KeyDesc.Keyinfo.Request) && !(pEapol_packet->KeyDesc.Keyinfo.Error))
            {
                // REQUEST=1, ERROR=0
                // Need to check KeyType for groupkey or pairwise key update, refer to 8021i P.114, 
                if (pEapol_packet->KeyDesc.Keyinfo.KeyType == GROUPKEY)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("REQUEST=1, ERROR=0, update group key\n"));
                    ApWpaGenRandom(pAd, pPort, pPort->SoftAP.ApCfg.GNonce);
                    pPort->SoftAP.ApCfg.WpaGTKState = SETKEYS;
                    pPort->SoftAP.ApCfg.GKeyDoneStations = pPort->MacTab.Size;

                    if (pEntry->WpaState == AS_PTKINITDONE)
                    {
                        pEntry->GTKState = REKEY_NEGOTIATING;
                        ApWpaHardTransmit(pAd, pEntry);
                        PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);
                        PlatformSetTimer(pPort, &pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
                    }
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("REQUEST=1, ERROR= 0, update pairwise key\n"));
                    pEntry->PairwiseKey.KeyLen = 0;
                    pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
                    AsicRemovePairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid); //(pEntry - &pPort->MacTab.Content[0])/sizeof(MAC_TABLE_ENTRY));
                    pEntry->Sst = SST_ASSOC;
                    if (pEntry->AuthMode == Ralink802_11AuthModeWPA)
                    {
                        pEntry->WpaState = AS_INITPMK;  
                    }
                    else if (pEntry->AuthMode == Ralink802_11AuthModeWPAPSK)
                    {
                        pEntry->WpaState = AS_INITPSK;  
                    }
                    pEntry->GTKState = REKEY_NEGOTIATING;
                    pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
                }
            }
        }
    }while(FALSE);
}

/*
    ==========================================================================
    Description:
        This is a function to initilize 4-way handshake
    Return:
     
    ==========================================================================
*/
VOID ApWpaStart4WayHS(
    IN PMP_ADAPTER    pAd, 
    IN PMP_PORT       pPort,
    IN MAC_TABLE_ENTRY  *pEntry) 
{
    UINT            i;
    UCHAR           Header802_3[14];
    AP_EAPOL_PACKET Packet;

    DBGPRINT(RT_DEBUG_TRACE, ("===> ApWpaStart4WayHS\n"));

    do
    {       
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;

        if ((pEntry->WpaState > AS_PTKSTART) || (pEntry->WpaState < AS_INITPMK))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Not expect calling  ApWpaStart4WayHS \n"));
            break;
        }

        // init header and Fill Packet
        MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pPort->CurrentAddress, EAPOL);

        PlatformZeroMemory(&Packet, sizeof(Packet));
        Packet.ProVer = EAPOL_VER;
        Packet.ProType = EAPOLKey;
        Packet.Body_Len[1] = 0x5f;

        // Increment replay counter by 1 and fill replay counter
        i = LEN_KEY_DESC_REPLAY;
        do
        {
            i--;
            pEntry->R_Counter[i]++;
            if (i == 0)
                break;
        } while (pEntry->R_Counter[i] == 0);
        PlatformMoveMemory(Packet.KeyDesc.RCounter, pEntry->R_Counter, sizeof(pEntry->R_Counter));

        // Fill key_version, keyinfo, key_len...
        Packet.KeyDesc.Keyinfo.KeyDescVer = DESC_TYPE_TKIP;
        Packet.KeyDesc.Keyinfo.KeyType = PAIRWISEKEY;
        Packet.KeyDesc.Keyinfo.KeyAck = 1;
        DBGPRINT(RT_DEBUG_TRACE, ("pEntry->AuthMode = %s\n",decodeAuthAlgorithm(pEntry->AuthMode)));
        // Deal with the differences between WPA and WPA2
        if ((pEntry->AuthMode == Ralink802_11AuthModeWPA) || (pEntry->AuthMode == Ralink802_11AuthModeWPAPSK))
        {
            Packet.KeyDesc.Type = WPA1_KEY_DESC;
            Packet.KeyDesc.DataLen[1] = 0;
        }
        else if ((pEntry->AuthMode == Ralink802_11AuthModeWPA2) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK))
        {
            Packet.KeyDesc.Type = WPA2_KEY_DESC;
            Packet.KeyDesc.DataLen[1] = 0;
        }

        // Deal with the differences between TKIP and CCMP
        if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
        {
            Packet.KeyDesc.KeyLength[1] = LEN_TKIP_KEY;
            Packet.KeyDesc.Keyinfo.KeyDescVer = DESC_TYPE_TKIP;
        }
        else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
        {
            Packet.KeyDesc.KeyLength[1] = LEN_AES_KEY;
            Packet.KeyDesc.Keyinfo.KeyDescVer = DESC_TYPE_AES;
        }

        // Fill Anonce
        ApWpaGenRandom(pAd, pPort, pEntry->ANonce);  
        PlatformMoveMemory(Packet.KeyDesc.Nonce, pEntry->ANonce, sizeof(pEntry->ANonce));
        
        // Transmit Msg1 to air
        ApWpaToWirelessSta(pAd, pPort, pEntry->Aid, Header802_3, sizeof(Header802_3), (PUCHAR)&Packet, Packet.Body_Len[1] + 4);

        pEntry->WpaState = AS_PTKSTART;

    }while(FALSE);

#if DBG
    if (pEntry)
        DBGPRINT(RT_DEBUG_TRACE, ("<=== ApWpaStart4WayHS, WpaState= %d \n", pEntry->WpaState));
#endif
}

/*
    ==========================================================================
    Description:
        When receiving the second packet of 4-way pairwisekey handshake.
    Return:
    ==========================================================================
*/
VOID ApWpaPeerPairMsg2Action(
    IN PMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem) 
{
    UCHAR               PTK[80], mac[LEN_KEY_DESC_MIC], SNonce[LEN_KEY_DESC_NONCE], digest[80];
    UCHAR               EAPOLHEAD[4] = {EAPOL_VER,  EAPOLKey,  0x00,  0x77};
    ULONG               FrameLen = 0;
    UCHAR               OutBuffer[MAX_LEN_OF_EAP_HS];
    USHORT              MICMsgLen, MICOffset;
    BOOLEAN             Cancelled;
    PHEADER_802_11      pHeader;
    AP_EAPOL_PACKET     *pEapol_Packet, Packet;
    UCHAR               Header802_3[14];
    UINT                i;
    PUCHAR              pRc4GTK = NULL, Key_Data = NULL;
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    UCHAR               idx;
    
    DBGPRINT(RT_DEBUG_TRACE, ("===>ApWpaPeerPairMsg2Action \n"));

    pHeader = (PHEADER_802_11)Elem->Msg;

    do
    {
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;

        if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(AP_KEY_DESCRIPTER) - AP_MAX_LEN_OF_RSNIE - 2))
            break;

        // check Entry in valid State
        if (pEntry->WpaState < AS_PTKSTART)
            break;

        // Save Data Length to pDesc for receiving packet,   then put in outgoing frame Data Len fields.
        pEapol_Packet = (PAP_EAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];       

        // 802.1i, p.99 - 100 ,  on Receive MSG2 
        //  1. check Replay Counter coresponds to MSG1.,  otherwise discard
        if (!PlatformEqualMemory(pEapol_Packet->KeyDesc.RCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in msg 2 of 4-way handshake! \n"));
            break;
        }

        //  2. Derive PTK, 
        MICMsgLen=(Elem->Msg[LENGTH_802_11+LENGTH_802_1_H+3]) | (Elem->Msg[LENGTH_802_11+LENGTH_802_1_H+2]<<8); 
        EAPOLHEAD[3]=(UCHAR)MICMsgLen;
        MICMsgLen+=LENGTH_EAPOL_H;
        PlatformMoveMemory(SNonce, pEapol_Packet->KeyDesc.Nonce, LEN_KEY_DESC_NONCE);

        ApWpaCountPTK(pAd, pPort->SoftAP.ApCfg.PMK,  pEntry->ANonce, pPort->CurrentAddress, SNonce, pHeader->Addr2,  PTK, LEN_PTK); 
        PlatformMoveMemory(pEntry->PTK, PTK, LEN_PTK);

        //  3. verify MSG2 MIC, If not valid, discard.
        MICOffset = LENGTH_EAPOL_H+sizeof(AP_KEY_DESCRIPTER)-AP_MAX_LEN_OF_RSNIE-2-LEN_KEY_DESC_MIC;
        PlatformMoveMemory(OutBuffer, &Elem->Msg[LENGTH_802_11+LENGTH_802_1_H], MICMsgLen);  
        PlatformZeroMemory((OutBuffer+MICOffset), LEN_KEY_DESC_MIC);

        if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
        {
            hmac_md5(PTK, LEN_EAP_MICK, OutBuffer, MICMsgLen, mac);
        }
        else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
        {
            HMAC_SHA1(OutBuffer,  MICMsgLen, PTK, LEN_EAP_MICK, digest);
            PlatformMoveMemory(mac, digest, LEN_KEY_DESC_MIC);
        }

#pragma prefast(suppress: __WARNING_USING_UNINIT_VAR, "mac will be initialized above")          
        if (!PlatformEqualMemory(&Elem->Msg[LENGTH_802_11+LENGTH_802_1_H+MICOffset], mac, LEN_KEY_DESC_MIC))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in msg 2 of 4-way handshake\n"));
            DBGPRINT(RT_DEBUG_ERROR, ("Receive : %x %x \n ",Elem->Msg[LENGTH_802_11+LENGTH_802_1_H+MICOffset], Elem->Msg[LENGTH_EAPOL_H+LENGTH_802_11+LENGTH_802_1_H+MICOffset+1]));
            DBGPRINT(RT_DEBUG_ERROR, ("PMK : %x %x \n ",pPort->SoftAP.ApCfg.PMK[0], pPort->SoftAP.ApCfg.PMK[1]));
            break;
        }
        else
            DBGPRINT(RT_DEBUG_ERROR, ("MIC VALID in msg 2 of 4-way handshake\n"));

        //  4. check RSN IE, if not match, send MLME-DEAUTHENTICATE.
        // Points  pDesc  to RSN_IE in Datafield.
        if (!PlatformEqualMemory(&pEapol_Packet->KeyDesc.Data[2], pEntry->RSN_IE, pEntry->RSNIE_Len))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("ApWpaPeerPairMsg2Action::RSN_IE Different in msg 2 of 4-way handshake!\n"));
            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaPeerPairMsg2Action::Receive     : %x %x %x %x \n", pEapol_Packet->KeyDesc.Data[2], pEapol_Packet->KeyDesc.Data[3], pEapol_Packet->KeyDesc.Data[4], pEapol_Packet->KeyDesc.Data[5]));
        
            for (idx = 0; idx < pEntry->RSNIE_Len; idx++)
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-", pEapol_Packet->KeyDesc.Data[idx+2]));
            }
            DBGPRINT(RT_DEBUG_TRACE, ("\n"));       

            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaPeerPairMsg2Action::Current(%d) : %x %x %x %x \n", pEntry->RSNIE_Len, pEntry->RSN_IE[0], pEntry->RSN_IE[1], pEntry->RSN_IE[2], pEntry->RSN_IE[3]));

            for (idx = 0; idx < pEntry->RSNIE_Len; idx++)
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-",pEntry->RSN_IE[idx]));
            }
            DBGPRINT(RT_DEBUG_TRACE, ("\n"));   
            break;
        }
        else
            DBGPRINT(RT_DEBUG_TRACE, ("RSN_IE VALID in msg 2 of 4-way handshake\n"));

        //verified  ok , change state
        PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);
        pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

        // CONSTRUCT msg3
        // Init Packet and Fill header
        PlatformZeroMemory(&Packet, sizeof(Packet));
        Packet.ProVer = EAPOL_VER;
        Packet.ProType = EAPOLKey;

        // Increment replay counter by 1  
        i = LEN_KEY_DESC_REPLAY;
        do
        {
            i--;
            pEntry->R_Counter[i]++;
            DBGPRINT(RT_DEBUG_TRACE, ("pEntry->R_Counter[%d]++=%d\n", i, pEntry->R_Counter[i]));
            if (i == 0)
            {
                break;
            }
        } while (pEntry->R_Counter[i] == 0);    
    
        // Fill replay counter
        PlatformMoveMemory(Packet.KeyDesc.RCounter, pEntry->R_Counter, sizeof(pEntry->R_Counter));

        // Fill key version, keyinfo, key len
        Packet.KeyDesc.Keyinfo.KeyMic = 1;
        Packet.KeyDesc.Keyinfo.KeyType = PAIRWISEKEY;
        Packet.KeyDesc.Keyinfo.Install = 1;
        Packet.KeyDesc.Keyinfo.KeyAck = 1;
        if ((pEntry->AuthMode == Ralink802_11AuthModeWPA) || (pEntry->AuthMode == Ralink802_11AuthModeWPAPSK))
        {
            Packet.KeyDesc.Type = WPA1_KEY_DESC;
            Packet.KeyDesc.DataLen[1] = 0;
        }
        else if ((pEntry->AuthMode == Ralink802_11AuthModeWPA2) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK))
        {
            Packet.KeyDesc.Type = WPA2_KEY_DESC;
            Packet.KeyDesc.DataLen[1] = 0;
            Packet.KeyDesc.Keyinfo.EKD_DL = 1;
            Packet.KeyDesc.Keyinfo.Secure = 1;
        }

        if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
        {
            Packet.KeyDesc.KeyLength[1] = LEN_TKIP_KEY;
        }
        else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
        {
            Packet.KeyDesc.KeyLength[1] = LEN_AES_KEY;
        }
        // Need to care about TKIP/AES both cipher mode carefully.
        Packet.KeyDesc.Keyinfo.KeyDescVer =
            (((pEntry->WepStatus == Ralink802_11Encryption3Enabled) || (pPort->PortCfg.GroupKeyWepStatus == Ralink802_11Encryption3Enabled)) ? (DESC_TYPE_AES) : (DESC_TYPE_TKIP));

        // Fill in Data, Data Len
        PlatformMoveMemory(Packet.KeyDesc.Nonce, pEntry->ANonce, sizeof(pEntry->ANonce));
        PlatformZeroMemory(Packet.KeyDesc.MIC, sizeof(Packet.KeyDesc.MIC));

        if ((pEntry->AuthMode == Ralink802_11AuthModeWPA) || (pEntry->AuthMode == Ralink802_11AuthModeWPAPSK))
        {
            Packet.KeyDesc.Data[0] = IE_WPA;
            Packet.KeyDesc.Data[1] = pPort->SoftAP.ApCfg.RSNIE_Len[0];
            Packet.KeyDesc.DataLen[1] = pPort->SoftAP.ApCfg.RSNIE_Len[0] + 2;
            Packet.Body_Len[1] = 93 + 2 + 2 + pPort->SoftAP.ApCfg.RSNIE_Len[0];

            PlatformMoveMemory(&Packet.KeyDesc.Data[2], pPort->SoftAP.ApCfg.RSN_IE[0], pPort->SoftAP.ApCfg.RSNIE_Len[0]);

            // make a frame for Counting MIC.
            MakeOutgoingFrame(OutBuffer,            &FrameLen,
                                Packet.Body_Len[1] + 4,  &Packet,
                                END_OF_ARGS);
            PlatformZeroMemory(mac, sizeof(mac));

            // count MIC
            if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
            {
                HMAC_SHA1(OutBuffer,  FrameLen, PTK, LEN_EAP_MICK, digest);
                PlatformMoveMemory(mac, digest, LEN_KEY_DESC_MIC);
            }
            else
            {
                hmac_md5(PTK,  LEN_EAP_MICK, OutBuffer, FrameLen, mac);
            }
            PlatformMoveMemory(&Packet.KeyDesc.MIC, mac, LEN_KEY_DESC_MIC);
        }
        else if ((pEntry->AuthMode == Ralink802_11AuthModeWPA2) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK))
        {
            UCHAR       GTK[TKIP_GTK_LENGTH], mic[LEN_KEY_DESC_MIC];
            UCHAR       ekey[(LEN_KEY_DESC_IV+LEN_EAP_EK)], key_length, RSNIE_Len;

            PlatformAllocateMemory(pAd,  &pRc4GTK, 512);

            if (pRc4GTK == NULL)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed for pRc4GTK\n", __FUNCTION__));
                break;
            }
            
           PlatformAllocateMemory(pAd,  &Key_Data, 512);
            if (Key_Data == NULL)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed for Key_Data\n", __FUNCTION__));
                break;
            }

            PlatformMoveMemory(Packet.KeyDesc.RSC, pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc, 6);

            ApWpaCountGTK(pAd, pPort->SoftAP.ApCfg.GMK, (UCHAR*)pPort->SoftAP.ApCfg.GNonce, pPort->CurrentAddress, GTK, TKIP_GTK_LENGTH);
            pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].KeyLen = LEN_TKIP_EK;
            PlatformMoveMemory(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].Key, GTK, LEN_TKIP_EK);
            PlatformMoveMemory(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxMic, &GTK[16], LEN_TKIP_TXMICK);
            PlatformMoveMemory(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].RxMic, &GTK[24], LEN_TKIP_RXMICK);

            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_TKIP;
            else if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_TKIP;
            else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_AES;
            else
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_NONE;

            // Fixed P2P 4 Way Key to cover STA's. Changed register offset from BSS0 to BSS3.
            if(pPort->PortNumber != PORT_0)
            {
                AsicAddKeyEntry(pAd, pPort, 0, BSS3, pPort->PortCfg.DefaultKeyId, &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId], FALSE, TRUE);
            }
            else
            {
                AsicAddKeyEntry(pAd, pPort, 0, BSS0, pPort->PortCfg.DefaultKeyId, &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId], FALSE, TRUE);
            }

            PlatformZeroMemory(Key_Data, 512);

            if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK)
            {
                RSNIE_Len = pPort->SoftAP.ApCfg.RSNIE_Len[1];
                PlatformMoveMemory(&Key_Data[2], pPort->SoftAP.ApCfg.RSN_IE[1], RSNIE_Len);
            }
            else
            {
                RSNIE_Len = pPort->SoftAP.ApCfg.RSNIE_Len[0];
                PlatformMoveMemory(&Key_Data[2], &pPort->SoftAP.ApCfg.RSN_IE[0], RSNIE_Len);
            }

            Packet.Body_Len[1] = 2+RSNIE_Len;

            Key_Data[0] = IE_WPA2;
            Key_Data[1] = RSNIE_Len;

            // Key Data Encapsulation format
            Key_Data[2+RSNIE_Len+0] = 0xDD;
            Key_Data[2+RSNIE_Len+2] = 0x00;
            Key_Data[2+RSNIE_Len+3] = 0x0F;
            Key_Data[2+RSNIE_Len+4] = 0xAC;
            Key_Data[2+RSNIE_Len+5] = 0x01;

            // GTK Key Data Encapsulation format
            Key_Data[2+RSNIE_Len+6] = (pPort->PortCfg.DefaultKeyId & 0x03);
            Key_Data[2+RSNIE_Len+7] = 0x00;

            // handle the difference between TKIP and AES-CCMP
            if (pPort->PortCfg.GroupKeyWepStatus == Ralink802_11Encryption3Enabled)
            {
                Key_Data[2+RSNIE_Len+1] = 0x16;// 4+2+16(OUI+GTK+GTKKEY)
                PlatformMoveMemory(&Key_Data[2+RSNIE_Len+8], GTK, LEN_AES_KEY);
                Packet.Body_Len[1] += 8+LEN_AES_KEY;
            }
            else
            {
                Key_Data[2+RSNIE_Len+1] = 0x26;// 4+2+32(OUI+GTK+GTKKEY)
                PlatformMoveMemory(&Key_Data[2+RSNIE_Len+8], GTK, TKIP_GTK_LENGTH);
                Packet.Body_Len[1] += 8+TKIP_GTK_LENGTH;
            }

            // Still dont know why, but if not append will occur "GTK not include in MSG3"
            // Patch for compatibility between zero config and funk
            if (pPort->PortCfg.GroupKeyWepStatus == Ralink802_11Encryption3Enabled)
            {
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH] = 0xDD;
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH+1] = 0;
                Packet.Body_Len[1] += 2;
            }
            else
            {
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH] = 0xDD;
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH+1] = 0;
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH+2] = 0;
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH+3] = 0;
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH+4] = 0;
                Key_Data[2+RSNIE_Len+8+TKIP_GTK_LENGTH+5] = 0;
                Packet.Body_Len[1] += 6;
            }

            key_length = Packet.Body_Len[1];
            Packet.KeyDesc.DataLen[1] = key_length;
            Packet.Body_Len[1] += 93 + 2; 

            PlatformZeroMemory(mic, sizeof(mic));

            // count MIC
            if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
            {
                ApWpaAesGtkkeyWrap(pAd, &pEntry->PTK[16], Key_Data, key_length, pRc4GTK);
                // AES wrap function will grow 8 bytes in length
                PlatformMoveMemory(Packet.KeyDesc.Data, pRc4GTK, (key_length + 8));
                Packet.Body_Len[1] += 8;
                Packet.KeyDesc.DataLen[1] += 8;
                MakeOutgoingFrame(OutBuffer,                &FrameLen,
                                  Packet.Body_Len[1] + 4,   &Packet,
                                  END_OF_ARGS);

                HMAC_SHA1(OutBuffer,  FrameLen, pEntry->PTK, LEN_EAP_MICK, digest);
                PlatformMoveMemory(Packet.KeyDesc.MIC, digest, LEN_KEY_DESC_MIC);
            }
            else
            {
                // PREPARE Encrypted  "Key DATA" field.  (Encrypt GTK with RC4, usinf PTK[16]->[31] as Key, IV-field as IV)
                // put TxTsc in Key RSC field
                pAd->pNicCfg->PrivateInfo.FCSCRC32 = PPPINITFCS32;   //Init crc32.

                // ekey is the contanetion of IV-field, and PTK[16]->PTK[31]
                PlatformMoveMemory(ekey, Packet.KeyDesc.IV, LEN_KEY_DESC_IV);
                PlatformMoveMemory(&ekey[LEN_KEY_DESC_IV], &pEntry->PTK[16], LEN_EAP_EK);
                ARCFOUR_INIT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, ekey, sizeof(ekey));  //INIT SBOX, KEYLEN+3(IV)
                pAd->pNicCfg->PrivateInfo.FCSCRC32 = RTMP_CALC_FCS32(pAd->pNicCfg->PrivateInfo.FCSCRC32, Key_Data, key_length);
                WPAARCFOUR_ENCRYPT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, pRc4GTK,Key_Data, key_length);
                PlatformMoveMemory(Packet.KeyDesc.Data, pRc4GTK, key_length);

                // make a frame for Countint MIC,
                MakeOutgoingFrame(OutBuffer,                &FrameLen,
                                  Packet.Body_Len[1] + 4,   &Packet,
                                  END_OF_ARGS);

                hmac_md5(pEntry->PTK, LEN_EAP_MICK, OutBuffer, FrameLen, mic);
                PlatformMoveMemory(Packet.KeyDesc.MIC, mic, LEN_KEY_DESC_MIC);
            }
        }

        // Make outgoing frame
        MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pPort->CurrentAddress, EAPOL);

        ApWpaToWirelessSta(pAd, pPort, pEntry->Aid, Header802_3, sizeof(Header802_3), (PUCHAR)&Packet, Packet.Body_Len[1] + 4);

        pEntry->ReTryCounter = PEER_MSG3_RETRY_TIMER_CTR;
        PlatformSetTimer(pPort, &pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);

        pEntry->WpaState = AS_PTKINIT_NEGOTIATING;
    }while(FALSE);  

    if (Key_Data)
        PlatformFreeMemory(Key_Data, 512);

    if (pRc4GTK)
        PlatformFreeMemory(pRc4GTK, 512);

    DBGPRINT(RT_DEBUG_TRACE, ("<=== ApWpaPeerPairMsg2Action \n"));
}

/*
    ==========================================================================
    Description:
        countermeasures active attack timer execution
    Return:
    ==========================================================================
*/
VOID ApWpaCMTimerExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    UINT           j = 0;
    PMP_PORT   pPort = (PMP_PORT)FunctionContext;
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;    
    PQUEUE_HEADER pHeader;

    pPort->SoftAP.ApCfg.BANClass3Data = FALSE;

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
        
        if ((pMacEntry->ValidAsCLI == TRUE) && (pMacEntry->CMTimerRunning == TRUE))
        {
            pMacEntry->CMTimerRunning =FALSE;
            j++;
        }
        pNextMacEntry = pNextMacEntry->Next;
        pMacEntry = NULL;
    }
    
    if (j > 1)
        DBGPRINT(RT_DEBUG_ERROR, ("Find more than one entry which generated MIC Fail ..  \n"));

    pPort->SoftAP.ApCfg.CMTimerRunning = FALSE;
}

VOID ApWpaRetryExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    IN  PMTK_TIMER_STRUCT    pTimer = (PMTK_TIMER_STRUCT)SystemSpecific2;
    MAC_TABLE_ENTRY *pEntry = NULL;


    pEntry = (PMAC_TABLE_ENTRY)pTimer->pOption;
//After refine the Mac Table, need enable again.    

    if ((pEntry) && (pEntry->ValidAsCLI == TRUE))
    {

        pEntry->ReTryCounter++;
        DBGPRINT(RT_DEBUG_TRACE, ("ApWpaRetryExecTimerCallback---> ReTryCounter=%d, WpaState=%d \n", pEntry->ReTryCounter, pEntry->WpaState));

        if(pPort->P2PCfg.P2PConnectState == P2P_GO_ASSOC_AUTH)
            pPort = pAd->PortList[pPort->P2PCfg.PortNumber];

        switch (pEntry->AuthMode)
        {
            case Ralink802_11AuthModeWPAPSK:
            case Ralink802_11AuthModeWPA2PSK:
                if (pEntry->ReTryCounter > (GROUP_MSG1_RETRY_TIMER_CTR + 1))
                {    
                    DBGPRINT(RT_DEBUG_TRACE, ("ApWpaRetryExecTimerCallback::Group Key HS exceed retry count, Disassociate client, pEntry->ReTryCounter %d\n", pEntry->ReTryCounter));
                    ApWpaDisAssocAction(pAd,pPort, pEntry, REASON_GROUP_KEY_HS_TIMEOUT);
                }
                else if (pEntry->ReTryCounter > GROUP_MSG1_RETRY_TIMER_CTR)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("ApWpaRetryExecTimerCallback::ReTry 2-way group-key Handshake \n"));
                    if (pEntry->GTKState == REKEY_NEGOTIATING)
                    {
                        ApWpaHardTransmit(pAd, pEntry);
                        PlatformSetTimer(pPort, &pEntry->RetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
                    }
                }
                else if (pEntry->ReTryCounter > (PEER_MSG1_RETRY_TIMER_CTR + 3))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("ApWpaRetryExecTimerCallback::MSG3 timeout, pEntry->ReTryCounter = %d\n", pEntry->ReTryCounter));
                    //ApWpaDisAssocAction(pAd, pEntry, REASON_4_WAY_TIMEOUT);
                    // Patch: (the SoftAP uses WPA2-PSK BOTH)
                    // Linksys STA (WUSB300N) may send the encrypted EAPOL packet during 4-way handshaking, 
                    // when it changes security algorithm between TKIP and AES.
                    // In addition, the SoftAP should send deauthentication to the Linksys STA in order to make the Linksys STA
                    // back to State 1 (unauthenticated and unassociated state).
                    MlmeAssocCls3errAction(pAd, pEntry->Addr);
                }
                else if (pEntry->ReTryCounter == (PEER_MSG1_RETRY_TIMER_CTR + 3))                
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("ApWpaRetryExecTimerCallback::Retry MSG1, the last try\n"));
                    ApWpaStart4WayHS(pAd, pPort, pEntry);
                    PlatformSetTimer(pPort, &pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
                }
                else if (pEntry->ReTryCounter < (PEER_MSG1_RETRY_TIMER_CTR + 3))
                {
                    if ((pEntry->WpaState == AS_PTKSTART) || (pEntry->WpaState == AS_INITPSK) || (pEntry->WpaState == AS_INITPMK))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("ApWpaRetryExecTimerCallback::ReTry MSG1 of 4-way Handshake\n"));
                        ApWpaStart4WayHS(pAd, pPort, pEntry);
                        PlatformSetTimer(pPort, &pEntry->RetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
                    }
                }
                break;

            default:
                break;
        }
    }
}

/*
    ==========================================================================
    Description:
        When receiving the last packet of 4-way pairwisekey handshake.
        Initilize 2-way groupkey handshake following.
    Return:
    ==========================================================================
*/
VOID ApWpa1PeerPairMsg4Action(
    IN PMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem) 
{
    UCHAR               mac[LEN_KEY_DESC_MIC];
    ULONG               MICMsgLen;
    PHEADER_802_11      pHeader;
    AP_KEY_DESCRIPTER      *pKeyDesc;
    AP_EAPOL_PACKET        EAPOLPKT;
    BOOLEAN             Cancelled = FALSE;  
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    
    // ========================================================
    DBGPRINT(RT_DEBUG_TRACE, ("===> ApWpa1PeerPairMsg4Action\n"));
    PlatformZeroMemory((PUCHAR)&EAPOLPKT, sizeof(AP_EAPOL_PACKET));

    do
    {
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;
        if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(AP_KEY_DESCRIPTER) - AP_MAX_LEN_OF_RSNIE - 2 ) )
            break;
        if (pEntry->WpaState < AS_PTKINIT_NEGOTIATING)
            break;

        pKeyDesc = (PAP_KEY_DESCRIPTER)&Elem->Msg[(LENGTH_802_11+LENGTH_802_1_H+LENGTH_EAPOL_H)];
        pHeader = (PHEADER_802_11)Elem->Msg;

        // 1.check Replay Counter
        if (!PlatformEqualMemory(pKeyDesc->RCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in msg 4 of 4-way handshake!!!!!!!!!! \n"));
            DBGPRINT(RT_DEBUG_ERROR, ("Receive : %d %d %d %d\n", pKeyDesc->RCounter[0],pKeyDesc->RCounter[1],pKeyDesc->RCounter[2],pKeyDesc->RCounter[3]));
            DBGPRINT(RT_DEBUG_ERROR, ("Current : %d %d %d %d\n", pEntry->R_Counter[4],pEntry->R_Counter[5],pEntry->R_Counter[6],pEntry->R_Counter[7]));
        }

        // 2. check MIC, if not valid, discard silently
        MICMsgLen = (Elem->Msg[LENGTH_802_11+LENGTH_802_1_H+3]) | (Elem->Msg[LENGTH_802_11+LENGTH_802_1_H+2]<<8);   
        MICMsgLen += LENGTH_EAPOL_H;

        PlatformMoveMemory((PUCHAR)&EAPOLPKT, &Elem->Msg[LENGTH_802_11+LENGTH_802_1_H], MICMsgLen);  
        PlatformZeroMemory(EAPOLPKT.KeyDesc.MIC, sizeof(EAPOLPKT.KeyDesc.MIC));

        hmac_md5(pEntry->PTK, LEN_EAP_MICK, (PUCHAR)&EAPOLPKT, (MICMsgLen), mac);
        if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
        {
            hmac_md5(pEntry->PTK, LEN_EAP_MICK, (PUCHAR)&EAPOLPKT, (MICMsgLen), mac);
        }
        else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
        {
            UCHAR digest[80];

            HMAC_SHA1((PUCHAR)&EAPOLPKT,  MICMsgLen, pEntry->PTK, LEN_EAP_MICK, digest);
            PlatformMoveMemory(mac, digest, LEN_KEY_DESC_MIC);
        }
        if (!PlatformEqualMemory(pKeyDesc->MIC, mac, LEN_KEY_DESC_MIC))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("MAC Different in msg 4 of 4-way handshake!\n"));
            break;
        }
        else
            DBGPRINT(RT_DEBUG_TRACE, ("MAC Valid in msg 4 of 4-way handshake!\n"));

        // 3. uses the MLME.SETKEYS.request to configure PTK into MAC
        PlatformZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));   

        pEntry->PairwiseKey.KeyLen = LEN_TKIP_EK;
        PlatformMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[32], LEN_TKIP_EK);
        PlatformMoveMemory(pEntry->PairwiseKey.RxMic, &pEntry->PTK[TKIP_AP_RXMICK_OFFSET], LEN_TKIP_RXMICK);
        PlatformMoveMemory(pEntry->PairwiseKey.TxMic, &pEntry->PTK[TKIP_AP_TXMICK_OFFSET], LEN_TKIP_TXMICK);
        {
        pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
        if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
            pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
        else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
            pEntry->PairwiseKey.CipherAlg = CIPHER_AES;
#if 0           
        AsicAddPairwiseKeyEntry(
            pAd, 
            pEntry->Addr, 
            (UCHAR)pEntry->Aid, //(pEntry - &pPort->MacTab.Content[0])/sizeof(MAC_TABLE_ENTRY), 
            &pEntry->PairwiseKey,
            FALSE);
#endif
        AsicAddKeyEntry(pAd,pPort, pEntry->Aid, BSS0, 0, &pEntry->PairwiseKey, TRUE, TRUE);
        }

        // 4. upgrade state
        pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
        pEntry->WpaState = AS_PTKINITDONE;
        pPort->SoftAP.ApCfg.PortSecured = WPA_802_1X_PORT_SECURED;
        ApLogEvent(pAd, pEntry->Addr, EVENT_ASSOCIATED);  // 2005-02-14 log association only after 802.1x successful

        // 5. init Group 2-way handshake if necessary.
        ApWpaHardTransmit(pAd, pEntry);

        pEntry->ReTryCounter = GROUP_MSG1_RETRY_TIMER_CTR;
        PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);
        PlatformSetTimer(pPort, &pEntry->RetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
    }while(FALSE);

    DBGPRINT(RT_DEBUG_TRACE, ("<=== ApWpa1PeerPairMsg4Action\n"));
}

/*
    ==========================================================================
    Description:
        When receiving the last packet of 4-way pairwisekey handshake.
    Return:
    ==========================================================================
*/
VOID ApWpa2PeerPairMsg4Action(
    IN PMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem) 
{
    UINT            MicDataLen;
    UCHAR           pDecryp[MAX_LEN_OF_EAP_HS], mic[LEN_KEY_DESC_MIC];
    PUCHAR          pData;
    BOOLEAN         Cancelled;
    AP_KEY_DESCRIPTER  *pKeyDesc;
    VOID            *Msg = &Elem->Msg[LENGTH_802_11];
    UINT            MsgLen = Elem->MsgLen - LENGTH_802_11;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE, ("===>ApWpa2PeerPairMsg4Action\n"));

    MicDataLen = MsgLen - LENGTH_802_1_H;
    pData = (PUCHAR)Msg;
    PlatformZeroMemory(pDecryp, sizeof(pDecryp));
    PlatformZeroMemory(mic, sizeof(mic));

    do
    {
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;

        if (MsgLen < (LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(AP_KEY_DESCRIPTER) - AP_MAX_LEN_OF_RSNIE - 2))
            break;

        pKeyDesc = (PAP_KEY_DESCRIPTER)(pData + LENGTH_802_1_H + LENGTH_EAPOL_H);

        // 1. verify the Reaply counter, if not valid,
        if (!PlatformEqualMemory(pKeyDesc->RCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in msg4 of 4-way handshake!\n"));
            DBGPRINT(RT_DEBUG_TRACE, ("Receive : %d %d %d %d \n",pKeyDesc->RCounter[0],pKeyDesc->RCounter[1],pKeyDesc->RCounter[2],pKeyDesc->RCounter[3]));
            DBGPRINT(RT_DEBUG_TRACE, ("Current : %d   %d  %d   %d  \n",pEntry->R_Counter[0],pEntry->R_Counter[1],pEntry->R_Counter[2],pEntry->R_Counter[3]));
            break;
        }
        else
            DBGPRINT(RT_DEBUG_TRACE, ("Replay Counter VALID in msg4 of 4-way handshake!\n"));

        // 2. verify MIC, 
        PlatformMoveMemory(pDecryp, &pData[LENGTH_802_1_H], MicDataLen);  
        PlatformZeroMemory(&pDecryp[81], LEN_KEY_DESC_MIC);
        if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
        {
            hmac_md5(pEntry->PTK,  LEN_EAP_MICK, pDecryp, MicDataLen, mic);
        }
        else
        {
            UCHAR digest[80];

            HMAC_SHA1(pDecryp, MicDataLen, pEntry->PTK, LEN_EAP_MICK, digest);
            PlatformMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
        }
        if (!PlatformEqualMemory(pKeyDesc->MIC, mic, LEN_KEY_DESC_MIC))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in msg4 of 4-way handshake!\n"));
            break;
        }
        else
            DBGPRINT(RT_DEBUG_TRACE, ("MIC VALID in msg4 of 4-way handshake!\n"));

        PlatformZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));   

        pEntry->PairwiseKey.KeyLen = LEN_TKIP_EK;
        PlatformMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[32], LEN_TKIP_EK);
        PlatformMoveMemory(pEntry->PairwiseKey.RxMic, &pEntry->PTK[TKIP_AP_RXMICK_OFFSET], LEN_TKIP_RXMICK);
        PlatformMoveMemory(pEntry->PairwiseKey.TxMic, &pEntry->PTK[TKIP_AP_TXMICK_OFFSET], LEN_TKIP_TXMICK);
        {
            pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
            if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
                pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
            else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
                pEntry->PairwiseKey.CipherAlg = CIPHER_AES;
    #if 0           
            AsicAddPairwiseKeyEntry(
                pAd, 
                pEntry->Addr, 
                (UCHAR)pEntry->Aid, //(pEntry - &pPort->MacTab.Content[0])/sizeof(MAC_TABLE_ENTRY), 
                &pEntry->PairwiseKey,
                FALSE);
    #endif
            AsicAddKeyEntry(pAd, pPort,pEntry->Aid, BSS0, 0, &pEntry->PairwiseKey, TRUE, TRUE);
        }

        // 3. upgrade state
        pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
        pEntry->WpaState = AS_PTKINITDONE;
        pPort->SoftAP.ApCfg.PortSecured = WPA_802_1X_PORT_SECURED;
        ApLogEvent(pAd, pEntry->Addr, EVENT_ASSOCIATED);  // 2005-02-14 log association only after 802.1x successful

        PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);

        pEntry->GTKState = REKEY_ESTABLISHED;

        if (pPort->SoftAP.ApCfg.GKeyDoneStations > 0 )
            pPort->SoftAP.ApCfg.GKeyDoneStations--;

        if (pPort->SoftAP.ApCfg.GKeyDoneStations == 0)
        {
            pPort->SoftAP.ApCfg.WpaGTKState = SETKEYS_DONE;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("AP SETKEYS DONE - WPA2, AuthMode=%s, WepStatus=%s\n", decodeAuthAlgorithm(pEntry->AuthMode), decodeCipherAlgorithm(pEntry->WepStatus)));
        P2pWPADone(pAd, pPort, pEntry, TRUE);
    }while(FALSE);  
}

/*
    ==========================================================================
    Description:
        When receiving the last packet of 2-way groupkey handshake.
    Return:
    ==========================================================================
*/
VOID ApWpaPeerGroupMsg2Action(
    IN PMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN VOID             *Msg,
    IN UINT             MsgLen) 
{
    UINT                Len;
    UCHAR               pDecryp[MAX_LEN_OF_EAP_HS], mic[LEN_KEY_DESC_MIC], digest[80];
    PUCHAR              pData;
    BOOLEAN             Cancelled;
    AP_KEY_DESCRIPTER  *pKeyDesc;
    PMP_PORT            pPort = NULL;

    do
    {
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;

        pPort = pEntry->pPort;
        if (pPort == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s: pPort can't be NULL\n", __FUNCTION__));
            break;
        }

        if (MsgLen < (LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(AP_KEY_DESCRIPTER) - AP_MAX_LEN_OF_RSNIE - 2))
            break;

        if (pEntry->WpaState != AS_PTKINITDONE)
            break;

        DBGPRINT(RT_DEBUG_TRACE, ("ApWpaPeerGroupMsg2Action ====> %x %x %x %x %x %x \n",pEntry->Addr[0], pEntry->Addr[1],
            pEntry->Addr[2],pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));

        Len = MsgLen - LENGTH_802_1_H;
        pData = (PUCHAR)Msg;
        PlatformZeroMemory(pDecryp, sizeof(pDecryp));
        PlatformZeroMemory(mic, sizeof(mic));

        pKeyDesc = (PAP_KEY_DESCRIPTER)(pData + LENGTH_802_1_H + LENGTH_EAPOL_H);

        // 1. verify the Reaply counter, if not valid,
        if (!PlatformEqualMemory(pKeyDesc->RCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter  Different  in msg 2 of GROUP 2-way handshake! \n"));
            DBGPRINT(RT_DEBUG_ERROR, ("Receive: %d %d %d %d - %d %d %d %d \n",pKeyDesc->RCounter[0],pKeyDesc->RCounter[1],pKeyDesc->RCounter[2],pKeyDesc->RCounter[3],pKeyDesc->RCounter[4],pKeyDesc->RCounter[5],pKeyDesc->RCounter[6],pKeyDesc->RCounter[7]));
            DBGPRINT(RT_DEBUG_ERROR, ("Current: %d %d %d %d - %d %d %d %d \n",pEntry->R_Counter[0],pEntry->R_Counter[1],pEntry->R_Counter[2],pEntry->R_Counter[3],pEntry->R_Counter[4],pEntry->R_Counter[5],pEntry->R_Counter[6],pEntry->R_Counter[7]));
            break;
        }
        else
            DBGPRINT(RT_DEBUG_TRACE, ("Replay Counter VALID in msg 2 of GROUP 2-way handshake! \n"));

        // 2. verify MIC, 
        PlatformMoveMemory(pDecryp, &pData[LENGTH_802_1_H], Len);  
        PlatformZeroMemory(&pDecryp[81], LEN_KEY_DESC_MIC);
        if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
        {
            hmac_md5(pEntry->PTK,  LEN_EAP_MICK, pDecryp, Len, mic);
        }
        else
        {
            //encrypt_mpdu(PTK,  pnl,  pnh, 0, PAY_LEN, &LEN_HDR, (INT*)&MICMsgLen, 0, 0, OutBuffer, mac);
            HMAC_SHA1(pDecryp,  Len, pEntry->PTK, LEN_EAP_MICK, digest);
            PlatformMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
        }
        if (!PlatformEqualMemory(pKeyDesc->MIC, mic, LEN_KEY_DESC_MIC))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in msg 2 of 2-way handshake!\n"));
            DBGPRINT(RT_DEBUG_TRACE, ("::PMK = %x:%x:%x:%x:%x:%x:%x:%x \n", 
                pPort->SoftAP.ApCfg.PMK[0],pPort->SoftAP.ApCfg.PMK[1],pPort->SoftAP.ApCfg.PMK[2],pPort->SoftAP.ApCfg.PMK[3],
                pPort->SoftAP.ApCfg.PMK[4],pPort->SoftAP.ApCfg.PMK[5],pPort->SoftAP.ApCfg.PMK[6],pPort->SoftAP.ApCfg.PMK[7]));
            break;
        }
        else
            DBGPRINT(RT_DEBUG_TRACE, ("MIC VALID in msg 2 of 2-way handshake!\n"));

        // 3.  upgrade state
        PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);

        pEntry->GTKState = REKEY_ESTABLISHED;

        if (pPort->SoftAP.ApCfg.GKeyDoneStations > 0 )
            pPort->SoftAP.ApCfg.GKeyDoneStations--;

        if (pPort->SoftAP.ApCfg.GKeyDoneStations == 0)
        {
            pPort->SoftAP.ApCfg.WpaGTKState = SETKEYS_DONE;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("AP SETKEYS DONE - WPA1, AuthMode=%s, WepStatus=%s\n", decodeAuthAlgorithm(pEntry->AuthMode), decodeCipherAlgorithm(pEntry->WepStatus)));
    }while(FALSE);  
}

/*
    ==========================================================================
    Description:
        Only for sending the first packet of 2-way groupkey handshake
    Return:
    ==========================================================================
*/
NDIS_STATUS ApWpaHardTransmit(
    IN PMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry)
{
    UCHAR               GTK[TKIP_GTK_LENGTH], pRc4GTK[TKIP_GTK_LENGTH + 8], mic[LEN_KEY_DESC_MIC], digest[80];
    UCHAR               ekey[LEN_KEY_DESC_IV + LEN_EAP_EK];
    ULONG               FrameLen = 0;
    PUCHAR              pOutBuffer = NULL;
    UCHAR               Header802_3[14], Key_Data[512];
    AP_EAPOL_PACKET     Packet;
    UINT                i;
    NDIS_STATUS         NStatus;
    PMP_PORT          pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    PlatformZeroMemory(ekey, sizeof(ekey));
    PlatformZeroMemory(pRc4GTK, sizeof(pRc4GTK));

    do
    {
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;

        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (NStatus != NDIS_STATUS_SUCCESS) 
            break;

        // Increment replay counter by 1  
        i = LEN_KEY_DESC_REPLAY;
        do
        {
            i--;
            pEntry->R_Counter[i]++;
            if (i == 0)
            {
                break;
            }
        }
        while (pEntry->R_Counter[i] == 0);

        // 0. init Packet and Fill header
        PlatformZeroMemory(&Packet, sizeof(Packet));
        Packet.ProVer = EAPOL_VER;
        Packet.ProType = EAPOLKey;
        Packet.Body_Len[1] = LEN_MSG1_2WAY;

        // 2, Fill key version, keyinfo, key len       
        if ((pEntry->AuthMode == Ralink802_11AuthModeWPA2) || 
            (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK))
        {
            Packet.KeyDesc.Type = WPA2_KEY_DESC;
            Packet.KeyDesc.Keyinfo.EKD_DL= 1;
        }
        else
            Packet.KeyDesc.Type = WPA1_KEY_DESC;
    
        Packet.KeyDesc.Keyinfo.KeyMic = 1;
        Packet.KeyDesc.Keyinfo.Secure = 1;
        Packet.KeyDesc.Keyinfo.KeyDescVer = DESC_TYPE_TKIP;
        Packet.KeyDesc.Keyinfo.KeyType = GROUPKEY;
        Packet.KeyDesc.Keyinfo.KeyIndex = pPort->PortCfg.DefaultKeyId;
        Packet.KeyDesc.Keyinfo.KeyAck = 1;
        Packet.KeyDesc.KeyLength[1] = TKIP_GTK_LENGTH;
        Packet.KeyDesc.DataLen[1] = TKIP_GTK_LENGTH;
        if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
        {
            Packet.Body_Len[1] -= 8;
            Packet.KeyDesc.KeyLength[1] = LEN_AES_KEY;
            Packet.KeyDesc.DataLen[1] = LEN_AES_KEY + 8;
            Packet.KeyDesc.Keyinfo.KeyDescVer = DESC_TYPE_AES;
        }

        ApWpaCountGTK(pAd, pPort->SoftAP.ApCfg.GMK, (UCHAR*)pPort->SoftAP.ApCfg.GNonce, pAd->HwCfg.CurrentAddress, GTK, TKIP_GTK_LENGTH);
        pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].KeyLen = LEN_TKIP_EK;
        PlatformMoveMemory(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].Key, GTK, LEN_TKIP_EK);
        PlatformMoveMemory(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxMic, &GTK[16], LEN_TKIP_TXMICK);
        PlatformMoveMemory(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].RxMic, &GTK[24], LEN_TKIP_RXMICK);            
        if (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)
            pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_TKIP;
        else if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
            pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_TKIP;
        else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
            pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_AES;
        else
            pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg = CIPHER_NONE;

        //AsicAddKeyEntry(pAd, 0, BSS0, pPort->CommonCfg.DefaultKeyId, &pPort->SharedKey[BSS0][pPort->CommonCfg.DefaultKeyId], FALSE, TRUE);
        MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_ASIC_ADD_SKEY, NULL, 0);
        if ((pEntry->AuthMode == Ralink802_11AuthModeWPA2) || 
            (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK))
        {
            UCHAR  key_length ;
            MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pAd->HwCfg.CurrentAddress, EAPOL);
            PlatformZeroMemory(Key_Data, 512); //sizeof(Key_Data));
            PlatformZeroMemory(Packet.KeyDesc.MIC, sizeof(Packet.KeyDesc.MIC));

            Packet.Body_Len[1] = 0;
            // Key Data Encapsulation format
            Key_Data[0] = 0xDD;
            Key_Data[2] = 0x00;
            Key_Data[3] = 0x0F;
            Key_Data[4] = 0xAC;
            Key_Data[5] = 0x01;

            // GTK Key Data Encapsulation format
            Key_Data[6] = (pPort->PortCfg.DefaultKeyId & 0x03);
            Key_Data[7] = 0x00;

            // handle the difference between TKIP and AES-CCMP
            if (pPort->PortCfg.GroupKeyWepStatus == Ralink802_11Encryption3Enabled)
            {
                Key_Data[1] = 0x16;// 4+2+16(OUI+GTK+GTKKEY)
                PlatformMoveMemory(&Key_Data[8], GTK, LEN_AES_KEY);
                Packet.Body_Len[1] += 8+LEN_AES_KEY;
            }
            else
            {
                Key_Data[1] = 0x26;// 4+2+32(OUI+GTK+GTKKEY)
                PlatformMoveMemory(&Key_Data[8], GTK, TKIP_GTK_LENGTH);
                Packet.Body_Len[1] += 8+TKIP_GTK_LENGTH;
            }
            // Do not apply the following patch if it is going to perform rekey procedure of the group key in WPA2-XXX AES.
            if (((pEntry->AuthMode == Ralink802_11AuthModeWPA2) ||(pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK)) && 
                (pEntry->GTKState == REKEY_NEGOTIATING) && 
            (pPort->PortCfg.GroupKeyWepStatus == Ralink802_11Encryption3Enabled))
            {
                // do nothing.
            }
            // Do not apply the following patch if it is going to perform rekey procedure of the group key in WPA2-PSK BOTH and the peer STA uses AES.
            else if ((pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK) && 
                (pEntry->GTKState == REKEY_NEGOTIATING) && 
                (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled) &&
                (pEntry->PairwiseKey.CipherAlg == CIPHER_AES))
            {
                // do nothing.
            }
            else
            {
                // Patch for compatibility between zero config and funk
                if (pPort->PortCfg.GroupKeyWepStatus == Ralink802_11Encryption3Enabled)
                {
                    Key_Data[8+TKIP_GTK_LENGTH] = 0xDD;
                    Key_Data[8+TKIP_GTK_LENGTH+1] = 0;
                    Packet.Body_Len[1] += 2;
                }
                else
                {
                    Key_Data[8+TKIP_GTK_LENGTH] = 0xDD;
                    Key_Data[8+TKIP_GTK_LENGTH+1] = 0;
                    Key_Data[8+TKIP_GTK_LENGTH+2] = 0;
                    Key_Data[8+TKIP_GTK_LENGTH+3] = 0;
                    Key_Data[8+TKIP_GTK_LENGTH+4] = 0;
                    Key_Data[8+TKIP_GTK_LENGTH+5] = 0;
                    Packet.Body_Len[1] += 6;
                }
            }

            key_length = Packet.Body_Len[1];
            Packet.KeyDesc.DataLen[1] = key_length;
            Packet.Body_Len[1] +=93 +2;

            PlatformZeroMemory(mic,LEN_KEY_DESC_MIC );

            PlatformMoveMemory(Packet.KeyDesc.RSC, pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc, 6);

            PlatformMoveMemory(Packet.KeyDesc.RCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);
            PlatformZeroMemory(Packet.KeyDesc.Nonce, LEN_KEY_DESC_NONCE);//WPA2 0;
            PlatformZeroMemory(Packet.KeyDesc.IV,sizeof(LEN_KEY_DESC_IV));//WPA2 0; WPA1 random
            // count MIC
            if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
            {
                ApWpaAesGtkkeyWrap(pAd, &pEntry->PTK[16], Key_Data, key_length, pRc4GTK);
                // AES wrap function will grow 8 bytes in length
                PlatformMoveMemory(Packet.KeyDesc.Data, pRc4GTK, (key_length + 8));
                Packet.Body_Len[1] += 8;
                Packet.KeyDesc.DataLen[1] += 8;
                MakeOutgoingFrame(pOutBuffer,            &FrameLen,
                    Packet.Body_Len[1] + 4, &Packet,
                    END_OF_ARGS);

                HMAC_SHA1(pOutBuffer, FrameLen, pEntry->PTK, LEN_EAP_MICK, digest);
                PlatformMoveMemory(Packet.KeyDesc.MIC, digest, LEN_KEY_DESC_MIC);
            }
            else
            {
                // PREPARE Encrypted  "Key DATA" field.  (Encrypt GTK with RC4, usinf PTK[16]->[31] as Key, IV-field as IV)
                // put TxTsc in Key RSC field
                pAd->pNicCfg->PrivateInfo.FCSCRC32 = PPPINITFCS32;   //Init crc32.

                // ekey is the contanetion of IV-field, and PTK[16]->PTK[31]
                PlatformMoveMemory(ekey, Packet.KeyDesc.IV, LEN_KEY_DESC_IV);
                PlatformMoveMemory(&ekey[LEN_KEY_DESC_IV], &pEntry->PTK[16], LEN_EAP_EK);
                ARCFOUR_INIT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, ekey, (LEN_KEY_DESC_IV+LEN_EAP_EK));  //INIT SBOX, KEYLEN+3(IV)
                pAd->pNicCfg->PrivateInfo.FCSCRC32 = RTMP_CALC_FCS32(pAd->pNicCfg->PrivateInfo.FCSCRC32, Key_Data, key_length);
                WPAARCFOUR_ENCRYPT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, pRc4GTK,Key_Data, key_length);
#pragma prefast(suppress: __WARNING_READ_OVERRUN, "pRc4GTK, should not be invalid data")                
                PlatformMoveMemory(Packet.KeyDesc.Data, pRc4GTK, key_length);

                // make a frame for Countint MIC,
                MakeOutgoingFrame(pOutBuffer,            &FrameLen,
                Packet.Body_Len[1] + 4, &Packet,
                END_OF_ARGS);

                hmac_md5(pEntry->PTK, LEN_EAP_MICK, pOutBuffer, FrameLen, mic);
                PlatformMoveMemory(Packet.KeyDesc.MIC, mic, LEN_KEY_DESC_MIC);
            }

            MakeOutgoingFrame(pOutBuffer,            &FrameLen,
                            LENGTH_802_3,            Header802_3,
                            Packet.Body_Len[1] + 4,  &Packet,
                            END_OF_ARGS);

#pragma prefast(suppress: __WARNING_READ_OVERRUN, "Header802_3, should not Buffer overrun")
            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaToWirelessSta : ETHTYPE = %x %x FrameLen = %d! \n",Header802_3[12],Header802_3[13],FrameLen));

            ApWpaToWirelessSta(pAd, pPort, pEntry->Aid, Header802_3, LENGTH_802_3, (PUCHAR) &Packet, Packet.Body_Len[1] + 4);
    
        }
        else
        {

            MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pAd->HwCfg.CurrentAddress, EAPOL);

            PlatformMoveMemory(Packet.KeyDesc.RCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);
            PlatformMoveMemory(Packet.KeyDesc.Nonce, pPort->SoftAP.ApCfg.GNonce, LEN_KEY_DESC_NONCE);
            PlatformMoveMemory(Packet.KeyDesc.IV, &pPort->SoftAP.ApCfg.GNonce[16], sizeof(LEN_KEY_DESC_IV));

            // Suggest IV be random number plus some number,
            Packet.KeyDesc.IV[15] += 2;
            PlatformMoveMemory(Packet.KeyDesc.RSC, pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc, 6);
            // Count EAPOL MIC , and encrypt DATA field before Send,   DATA fields includes the encrypted GTK
            if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
            {
                if (pPort->PortCfg.GroupKeyWepStatus == Ralink802_11Encryption3Enabled)
                {
                    ApWpaAesGtkkeyWrap(pAd, &pEntry->PTK[16], GTK, 16, pRc4GTK);
                    PlatformMoveMemory(Packet.KeyDesc.Data, pRc4GTK, (LEN_AES_KEY +8));
                }
                else
                {
                    ApWpaAesGtkkeyWrap(pAd, &pEntry->PTK[16], GTK, TKIP_GTK_LENGTH, pRc4GTK);
                    PlatformMoveMemory(Packet.KeyDesc.Data, pRc4GTK, (TKIP_GTK_LENGTH +8));
                    Packet.KeyDesc.KeyLength[1] += 16;
                    Packet.KeyDesc.DataLen[1] += 16;
                    Packet.Body_Len[1] += 16;
                }

                // First make a frame  for Countint MIC,
                MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                    Packet.Body_Len[1] + 4,   &Packet,
                    END_OF_ARGS);

                HMAC_SHA1(pOutBuffer,  FrameLen, pEntry->PTK, LEN_EAP_MICK, digest);
                PlatformMoveMemory(&Packet.KeyDesc.MIC, digest, LEN_KEY_DESC_MIC);
            }
            else
            {
                // PREPARE Encrypted  "Key DATA" field.  (Encrypt GTK with RC4, usinf PTK[16]->[31] as Key, IV-field as IV)
                // put TxTsc in Key RSC field
                pAd->pNicCfg->PrivateInfo.FCSCRC32 = PPPINITFCS32;   //Init crc32.

                // ekey is the contanetion of IV-field, and PTK[16]->PTK[31]
                PlatformMoveMemory(ekey, Packet.KeyDesc.IV, LEN_KEY_DESC_IV);
                PlatformMoveMemory(&ekey[LEN_KEY_DESC_IV], &pEntry->PTK[16], LEN_EAP_EK);
                ARCFOUR_INIT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, ekey, sizeof(ekey));  //INIT SBOX, KEYLEN+3(IV)
                pAd->pNicCfg->PrivateInfo.FCSCRC32 = RTMP_CALC_FCS32(pAd->pNicCfg->PrivateInfo.FCSCRC32, GTK, TKIP_GTK_LENGTH);
                WPAARCFOUR_ENCRYPT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, pRc4GTK,GTK,  TKIP_GTK_LENGTH);
                PlatformMoveMemory(Packet.KeyDesc.Data,  pRc4GTK, TKIP_GTK_LENGTH);

                // make a frame for Countint MIC,
                MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                                Packet.Body_Len[1] + 4,   &Packet,
                                END_OF_ARGS);

                hmac_md5(pEntry->PTK, LEN_EAP_MICK, pOutBuffer, FrameLen, mic);
                PlatformMoveMemory(Packet.KeyDesc.MIC, mic, LEN_KEY_DESC_MIC);
            }

            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaHardTransmit : FrameLen = %d \n", Packet.Body_Len[1] + 4));
            ApWpaToWirelessSta(pAd, pPort, pEntry->Aid, Header802_3, LENGTH_802_3, (PUCHAR) &Packet, Packet.Body_Len[1] + 4);
        }
    }while (FALSE);

    MlmeFreeMemory(pAd, pOutBuffer);

    return (NDIS_STATUS_SUCCESS);
}

VOID ApWpaDisAssocAction(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT pPort,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN USHORT           Reason)
{
    PUCHAR          pOutBuffer = NULL;
    ULONG           FrameLen = 0;
    HEADER_802_11   DisassocHdr;
    NDIS_STATUS     NStatus;

    if (pEntry)
    {
        //  send out a DISASSOC request frame
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (NStatus != NDIS_STATUS_SUCCESS)
            return;

        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send DISASSOC Reason = %d frame to %x %x %x %x %x %x \n",Reason,pEntry->Addr[0],
            pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));

        MgtMacHeaderInit(pAd, pPort, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pPort->PortCfg.Bssid);
        MakeOutgoingFrame(pOutBuffer,               &FrameLen, 
                          sizeof(HEADER_802_11),    &DisassocHdr,
                          2,                        &Reason,
                          END_OF_ARGS);
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);     


        MacTableDeleteAndResetEntry(pAd, pPort ,pEntry->Aid, pEntry->Addr, TRUE);
    }
}

/*
     ==========================================================================
     Description:
        Timer execution function for periodically updating group key.
    Return:
    ==========================================================================
*/  
VOID ApWpaGREKEYPeriodicExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    ULONG           temp_counter = 0;
    BOOLEAN         Cancelled = FALSE;  
    PMP_PORT   pPort = (PMP_PORT)FunctionContext;
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL; 
    PQUEUE_HEADER pHeader;
    
    DBGPRINT(RT_DEBUG_INFO, ("GROUP REKEY PeriodicExec ==>> \n"));

    if((pPort->PortCfg.AuthMode < Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeAutoSwitch))
    {
        PlatformCancelTimer(&pPort->SoftAP.ApCfg.REKEYTimer, &Cancelled);
        pPort->SoftAP.ApCfg.REKEYTimerRunning = FALSE;
    }

    if ((pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod == TIME_REKEY) && (pPort->SoftAP.ApCfg.REKEYCOUNTER < 0xffffffff))
        temp_counter = (++pPort->SoftAP.ApCfg.REKEYCOUNTER);
    // REKEYCOUNTER is incremented every TX_RING_SIZE packets transmitted, 
    // But the unit of Rekeyinterval is 1K packets
    else if (pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod == PKT_REKEY )
        temp_counter = pPort->SoftAP.ApCfg.REKEYCOUNTER / 1000; // in 1000s of packets.
    else
    {
        PlatformCancelTimer(&pPort->SoftAP.ApCfg.REKEYTimer, &Cancelled);
        pPort->SoftAP.ApCfg.REKEYTimerRunning = FALSE;
    }

    if (temp_counter >= (pPort->SoftAP.ApCfg.WPAREKEY.ReKeyInterval))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: pPort->SoftAP.ApCfg.REKEYCOUNTER = %d\n", __FUNCTION__, pPort->SoftAP.ApCfg.REKEYCOUNTER));
        pPort->SoftAP.ApCfg.REKEYCOUNTER = 0;

        DBGPRINT(RT_DEBUG_TRACE, ("Rekey Interval Excess, GKeyDoneStations=%d\n", pPort->MacTab.Size));
        pPort->SoftAP.ApCfg.WpaGTKState = SETKEYS;

        // take turn updating different groupkey index, 
        if ((pPort->SoftAP.ApCfg.GKeyDoneStations = pPort->MacTab.Size) > 0)
        {
            pPort->PortCfg.DefaultKeyId = (pPort->PortCfg.DefaultKeyId == 1) ? 2 : 1;         
            ApWpaGenRandom(pAd, pPort, pPort->SoftAP.ApCfg.GNonce);

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
                
                if ((pMacEntry->ValidAsCLI == TRUE) && (pMacEntry->WpaState == AS_PTKINITDONE))
                {
                    pMacEntry->GTKState = REKEY_NEGOTIATING;
                    PlatformZeroMemory(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc, sizeof(pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc));
                    ApWpaHardTransmit(pAd, pMacEntry);
                    DBGPRINT(RT_DEBUG_TRACE, ("Rekey interval excess, Update Group Key for  %x %x %x  %x %x %x , DefaultKeyId= %x \n",\
                        pMacEntry->Addr[0],pMacEntry->Addr[1],\
                        pMacEntry->Addr[2],pMacEntry->Addr[3],\
                        pMacEntry->Addr[4],pMacEntry->Addr[5],\
                        pPort->PortCfg.DefaultKeyId));
                }
                
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
            }
            
        }
    }
}

VOID ApWpaCountPTK(
    IN  PMP_ADAPTER pAd,
    IN  UCHAR   *PMK,
    IN  UCHAR   *ANonce,
    IN  UCHAR   *AA,
    IN  UCHAR   *SNonce,
    IN  UCHAR   *SA,
    OUT UCHAR   *output,
    IN  UINT    len)
{
    UCHAR   concatenation[76];
    UINT    CurrPos=0;
    UCHAR   Prefix[22];
    UCHAR   temp[32];

    PlatformZeroMemory(temp, sizeof(temp));

    ApWpaGetSmall(SA, AA, temp, 6);
    PlatformMoveMemory(concatenation, temp, 6);
    CurrPos += 6;

    ApWpaGetLarge(SA, AA, temp, 6);
    PlatformMoveMemory(&concatenation[CurrPos], temp, 6);
    CurrPos += 6;

    ApWpaGetSmall(ANonce, SNonce, temp, 32);
    PlatformMoveMemory(&concatenation[CurrPos], temp, 32);
    CurrPos += 32;

    ApWpaGetLarge(ANonce, SNonce, temp, 32);
    PlatformMoveMemory(&concatenation[CurrPos], temp, 32);
    CurrPos += 32;

    Prefix[0] = 'P';
    Prefix[1] = 'a';
    Prefix[2] = 'i';
    Prefix[3] = 'r';
    Prefix[4] = 'w';
    Prefix[5] = 'i';
    Prefix[6] = 's';
    Prefix[7] = 'e';
    Prefix[8] = ' ';
    Prefix[9] = 'k';
    Prefix[10] = 'e';
    Prefix[11] = 'y';
    Prefix[12] = ' ';
    Prefix[13] = 'e';
    Prefix[14] = 'x';
    Prefix[15] = 'p';
    Prefix[16] = 'a';
    Prefix[17] = 'n';
    Prefix[18] = 's';
    Prefix[19] = 'i';
    Prefix[20] = 'o';
    Prefix[21] = 'n';
    PRF(pAd, PMK, PMK_LEN, Prefix,  22, concatenation, 76 , output, len);
}

VOID ApWpaCountGTK(
    IN  PMP_ADAPTER pAd,
    IN  UCHAR   *GMK,
    IN  UCHAR   *GNonce,
    IN  UCHAR   *AA,
    OUT UCHAR   *output,
    IN  UINT    len)
{
    UCHAR   concatenation[76];
    UINT    CurrPos=0;
    UCHAR   Prefix[19];
    UCHAR   temp[80];   

    PlatformMoveMemory(&concatenation[CurrPos], AA, 6);
    CurrPos += 6;

    PlatformMoveMemory(&concatenation[CurrPos], GNonce , 32);
    CurrPos += 32;

    Prefix[0] = 'G';
    Prefix[1] = 'r';
    Prefix[2] = 'o';
    Prefix[3] = 'u';
    Prefix[4] = 'p';
    Prefix[5] = ' ';
    Prefix[6] = 'k';
    Prefix[7] = 'e';
    Prefix[8] = 'y';
    Prefix[9] = ' ';
    Prefix[10] = 'e';
    Prefix[11] = 'x';
    Prefix[12] = 'p';
    Prefix[13] = 'a';
    Prefix[14] = 'n';
    Prefix[15] = 's';
    Prefix[16] = 'i';
    Prefix[17] = 'o';
    Prefix[18] = 'n';

    PRF(pAd, GMK, PMK_LEN, Prefix,  19, concatenation, 38 , temp, len);
    PlatformMoveMemory(output, temp, len);
}

VOID ApWpaGetSmall(
    IN  PVOID   pSrc1,
    IN  PVOID   pSrc2,
    OUT PUCHAR  pOut,
    IN  ULONG   Length)
{
    PUCHAR  pMem1;
    PUCHAR  pMem2;
    ULONG   Index = 0;

    pMem1 = (PUCHAR) pSrc1;
    pMem2 = (PUCHAR) pSrc2;

    for (Index = 0; Index < Length; Index++)
    {
        if (pMem1[Index] != pMem2[Index])
        {
            if (pMem1[Index] > pMem2[Index])        
                PlatformMoveMemory(pOut, pSrc2, Length);
            else
                PlatformMoveMemory(pOut, pSrc1, Length);             

            break;
        }
    }
}

VOID ApWpaGetLarge(
    IN  PVOID   pSrc1,
    IN  PVOID   pSrc2,
    OUT PUCHAR  pOut,
    IN  ULONG   Length)
{
    PUCHAR  pMem1;
    PUCHAR  pMem2;
    ULONG   Index = 0;

    pMem1 = (PUCHAR) pSrc1;
    pMem2 = (PUCHAR) pSrc2;

    for (Index = 0; Index < Length; Index++)
    {
        if (pMem1[Index] != pMem2[Index])
        {
            if (pMem1[Index] > pMem2[Index])        
                PlatformMoveMemory(pOut, pSrc1, Length);
            else
                PlatformMoveMemory(pOut, pSrc2, Length);             

            break;
        }
    }
}

// 802.1i  Annex F.9
VOID ApWpaGenRandom(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    OUT UCHAR       *random)
{
    INT             i, curr;
    UCHAR           local[80];
    UCHAR           result[80];
    LARGE_INTEGER   CurrentTime;
    UCHAR           prefix[] = {'I', 'n', 'i', 't', ' ', 'C', 'o', 'u', 'n', 't', 'e', 'r'};

    if ((pAd == NULL) || (pPort == NULL))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd or pPort can't be NULL\n", __FUNCTION__));
        return;
    }
    
    PlatformZeroMemory(result, 80);
    PlatformZeroMemory(local, 80);
    PlatformMoveMemory(local, pAd->HwCfg.CurrentAddress, MAC_ADDR_LEN);

    for (i = 0; i < 32; i++)
    {       
        curr =  MAC_ADDR_LEN;
        NdisGetCurrentSystemTime(&CurrentTime);
        PlatformMoveMemory(local,  pAd->HwCfg.CurrentAddress, MAC_ADDR_LEN);
        curr += MAC_ADDR_LEN;
        PlatformMoveMemory(&local[curr],  &CurrentTime, sizeof(CurrentTime));
        curr += sizeof(CurrentTime);
        PlatformMoveMemory(&local[curr],  result, 32);
        curr += 32;
        PlatformMoveMemory(&local[curr],  &i,  2);      
        curr += 2;
        PRF(pAd, pPort->SoftAP.ApCfg.Key_Counter, 32, prefix,12, local, curr, result, 32); 
    }

    for (i = 32; i > 0; i--)
    {   
        if (pPort->SoftAP.ApCfg.Key_Counter[i-1] == 0xff)
        {
            pPort->SoftAP.ApCfg.Key_Counter[i-1] = 0;
        }
        else
        {
            pPort->SoftAP.ApCfg.Key_Counter[i-1]++;
            break;
        }
    }
    PlatformMoveMemory(random, result,  32);
}

/*
    ==========================================================================
    Description:
        ENCRYPT AES GTK before sending in EAPOL frame.
        AES GTK length = 128 bit,  so fix blocks for aes-key-wrap as 2 in this function.
        This function references to RFC 3394 for aes key wrap algorithm.
    Return:
    ==========================================================================
*/
VOID ApWpaAesGtkkeyWrap( 
    IN PMP_ADAPTER pAd,
    IN UCHAR    *key,
    IN UCHAR    *plaintext,
    IN UCHAR    p_len,
    OUT UCHAR   *ciphertext)
{
    UCHAR       A[8], BIN[16], BOUT[16];
    PUCHAR      R = NULL;
    INT         num_blocks = p_len/8;   // unit:64bits
    INT         i, j;
    aes_context aesctx;
    UCHAR       xor;

   PlatformAllocateMemory(pAd,  &R, 512);
    if (R == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory\n", __FUNCTION__));
        return;
    }
    aes_set_key(&aesctx, key, 128);

    // Init IA
    for (i = 0; i < 8; i++)
        A[i] = 0xa6;

    //Input plaintext
    for (i = 0; i < num_blocks; i++)
    {
        for (j = 0 ; j < 8; j++)
            R[8 * (i + 1) + j] = plaintext[8 * i + j];
    }

    // Key Mix
    for (j = 0; j < 6; j++)
    {
        for(i = 1; i <= num_blocks; i++)
        {
            //phase 1
            memcpy(BIN, A, 8);
            memcpy(&BIN[8], &R[8 * i], 8);
            aes_encrypt(&aesctx, BIN, BOUT);

            memcpy(A, &BOUT[0], 8);
            xor = num_blocks * j + i;
            A[7] = BOUT[7] ^ xor;
            memcpy(&R[8 * i], &BOUT[8], 8);
        }
    }

    // Output ciphertext
    memcpy(ciphertext, A, 8);

    for (i = 1; i <= num_blocks; i++)
    {
        for (j = 0 ; j < 8; j++)
            ciphertext[8 * i + j] = R[8 * i + j];
    }

    if (R)
    {
        PlatformFreeMemory(R, 512);
    }
}

VOID ApWpaMakeRSNIE(
    IN  PMP_ADAPTER   pAd,    
    IN  UINT            AuthMode,
    IN  UINT            WepStatus)
{
    PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    if (WepStatus == Ralink802_11Encryption4Enabled)
        pPort->PortCfg.GroupKeyWepStatus = Ralink802_11Encryption2Enabled;
    else
        pPort->PortCfg.GroupKeyWepStatus = WepStatus;

    if ((AuthMode != Ralink802_11AuthModeWPA) && (AuthMode != Ralink802_11AuthModeWPAPSK)
        && (AuthMode != Ralink802_11AuthModeWPA2) && (AuthMode != Ralink802_11AuthModeWPA2PSK)
        && (AuthMode != Ralink802_11AuthModeWPA1WPA2) && (AuthMode != Ralink802_11AuthModeWPA1PSKWPA2PSK))
        return;

    pPort->SoftAP.ApCfg.RSNIE_Len[0] = 0;
    pPort->SoftAP.ApCfg.RSNIE_Len[1] = 0;
    PlatformZeroMemory(pPort->SoftAP.ApCfg.RSN_IE[0], AP_MAX_LEN_OF_RSNIE);
    PlatformZeroMemory(pPort->SoftAP.ApCfg.RSN_IE[1], AP_MAX_LEN_OF_RSNIE);

    // For WPA1, RSN_IE=221
    if ((AuthMode == Ralink802_11AuthModeWPA) || (AuthMode == Ralink802_11AuthModeWPAPSK)
        || (AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
        RSNIE               *pRsnie;
        RSNIE_AUTH          *pRsnie_auth;
        RSN_CAPABILITIES    *pRSN_Cap;
        UCHAR               Rsnie_size = 0;

        pRsnie = (RSNIE*)pPort->SoftAP.ApCfg.RSN_IE[0];
        PlatformMoveMemory(pRsnie->oui, OUI_WPA_WEP40, 4);
        pRsnie->version = 1;

        switch (WepStatus)
        {
            case Ralink802_11Encryption2Enabled:
                PlatformMoveMemory(pRsnie->mcast, OUI_WPA_TKIP, 4);
                pRsnie->ucount = 1;
                PlatformMoveMemory(pRsnie->ucast[0].oui, OUI_WPA_TKIP, 4);
                Rsnie_size = sizeof(RSNIE);
                break;

            case Ralink802_11Encryption3Enabled:
                PlatformMoveMemory(pRsnie->mcast, OUI_WPA_CCMP, 4);
                pRsnie->ucount = 1;
                PlatformMoveMemory(pRsnie->ucast[0].oui, OUI_WPA_CCMP, 4);
                Rsnie_size = sizeof(RSNIE);
                break;

            case Ralink802_11Encryption4Enabled:
                PlatformMoveMemory(pRsnie->mcast, OUI_WPA_TKIP, 4);
                pRsnie->ucount = 2;
                PlatformMoveMemory(pRsnie->ucast[0].oui, OUI_WPA_TKIP, 4);
                PlatformMoveMemory(pRsnie->ucast[0].oui + 4, OUI_WPA_CCMP, 4);
                Rsnie_size = sizeof(RSNIE) + 4;
                break;
        }

        pRsnie_auth = (RSNIE_AUTH*)((PUCHAR)pRsnie + Rsnie_size);

        switch (AuthMode)
        {
            case Ralink802_11AuthModeWPA:
            case Ralink802_11AuthModeWPA1WPA2:
                pRsnie_auth->acount = 1;
                PlatformMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_WEP40, 4);
                break;

            case Ralink802_11AuthModeWPAPSK:
            case Ralink802_11AuthModeWPA1PSKWPA2PSK:
                pRsnie_auth->acount = 1;
                PlatformMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_TKIP, 4);
                break;
        }

        pRSN_Cap = (RSN_CAPABILITIES*)((PUCHAR)pRsnie_auth + sizeof(RSNIE_AUTH));

        pPort->SoftAP.ApCfg.RSNIE_Len[0] = Rsnie_size + sizeof(RSNIE_AUTH) + sizeof(RSN_CAPABILITIES);
    }

    // For WPA2, RSN_IE=48, if WPA1WPA2/WPAPSKWPA2PSK mix mode, we store RSN_IE in RSN_IE[1] else RSNIE[0]
    if ((AuthMode == Ralink802_11AuthModeWPA2) || (AuthMode == Ralink802_11AuthModeWPA2PSK)
        || (AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
        RSNIE2              *pRsnie2;
        RSNIE_AUTH          *pRsnie_auth2;
        RSN_CAPABILITIES    *pRSN_Cap;
        UCHAR               Rsnie_size = 0;

        if ((AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
            pRsnie2 = (RSNIE2*)pPort->SoftAP.ApCfg.RSN_IE[1];
        else
            pRsnie2 = (RSNIE2*)pPort->SoftAP.ApCfg.RSN_IE[0];

        pRsnie2->version = 1;

        switch (WepStatus)
        {
            case Ralink802_11Encryption2Enabled:
                PlatformMoveMemory(pRsnie2->mcast, OUI_WPA2_TKIP, 4);
                pRsnie2->ucount = 1;
                PlatformMoveMemory(pRsnie2->ucast[0].oui, OUI_WPA2_TKIP, 4);
                Rsnie_size = sizeof(RSNIE2);
                break;

            case Ralink802_11Encryption3Enabled:
                PlatformMoveMemory(pRsnie2->mcast, OUI_WPA2_CCMP, 4);
                pRsnie2->ucount = 1;
                PlatformMoveMemory(pRsnie2->ucast[0].oui, OUI_WPA2_CCMP, 4);
                Rsnie_size = sizeof(RSNIE2);
                break;

            case Ralink802_11Encryption4Enabled:
                PlatformMoveMemory(pRsnie2->mcast, OUI_WPA2_TKIP, 4);
                pRsnie2->ucount = 2;
                PlatformMoveMemory(pRsnie2->ucast[0].oui, OUI_WPA2_TKIP, 4);
                PlatformMoveMemory(pRsnie2->ucast[0].oui + 4, OUI_WPA2_CCMP, 4);
                Rsnie_size = sizeof(RSNIE2) + 4;
                break;
        }

        pRsnie_auth2 = (RSNIE_AUTH*)((PUCHAR)pRsnie2 + Rsnie_size);

        switch (AuthMode)
        {
            case Ralink802_11AuthModeWPA2:
            case Ralink802_11AuthModeWPA1WPA2:
                pRsnie_auth2->acount = 1;
                PlatformMoveMemory(pRsnie_auth2->auth[0].oui, OUI_WPA2_WEP40, 4);
                break;

            case Ralink802_11AuthModeWPA2PSK:
            case Ralink802_11AuthModeWPA1PSKWPA2PSK:
                pRsnie_auth2->acount = 1;
                PlatformMoveMemory(pRsnie_auth2->auth[0].oui, OUI_WPA2_TKIP, 4);
                break;
        }

        pRSN_Cap = (RSN_CAPABILITIES*)((PUCHAR)pRsnie_auth2 + sizeof(RSNIE_AUTH));
        pRSN_Cap->field.Pre_Auth = (pPort->SoftAP.ApCfg.bPreAuth == TRUE) ? 1 : 0;

        if ((AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
            pPort->SoftAP.ApCfg.RSNIE_Len[1] = Rsnie_size + sizeof(RSNIE_AUTH) + sizeof(RSN_CAPABILITIES);
        else
            pPort->SoftAP.ApCfg.RSNIE_Len[0] = Rsnie_size + sizeof(RSNIE_AUTH) + sizeof(RSN_CAPABILITIES);
    }

    DBGPRINT(RT_DEBUG_TRACE,("ApWpaMakeRSNIE : RSNIE_Len = %d\n", pPort->SoftAP.ApCfg.RSNIE_Len[0]));
}

VOID    ApWpaToWirelessSta(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  USHORT          Aid,
    IN  PUCHAR          pHeader802_3,
    IN  UINT            HdrLen,
    IN  PUCHAR          pData,
    IN  UINT            DataLen)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PHEADER_802_11 p80211Header = NULL;
    PUCHAR pOutBuffer = NULL;
    ULONG packetSize = 0; // 802.11 header + EAPOL_LLC_SNAP + data size.
    PMT_XMIT_CTRL_UNIT pXcu = NULL;

    do {
        DBGPRINT(RT_DEBUG_TRACE,("==> %s\n", __FUNCTION__));

        // Allocate packet memory.
        ndisStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR,("%s: Memory allocation failure (ndisStatus = %d).\n", 
                __FUNCTION__, ndisStatus));
            break;
        }

        p80211Header = (PHEADER_802_11)(pOutBuffer);
        PlatformZeroMemory(p80211Header, sizeof(HEADER_802_11));        

        // Create partial header.
        p80211Header->FC.FrDs = 1;
        p80211Header->FC.ToDs = 0;
        p80211Header->FC.Type = DOT11_FRAME_TYPE_DATA;
        p80211Header->FC.SubType = DOT11_DATA_SUBTYPE_DATA;
        p80211Header->FC.MoreData = 0; //TODO
        p80211Header->FC.MoreFrag = 0; // TODO
        p80211Header->FC.PwrMgmt = 0; // TODO
        p80211Header->FC.Retry = 0; // TODO
        COPY_MAC_ADDR(p80211Header->Addr1, pHeader802_3); // DA
        COPY_MAC_ADDR(p80211Header->Addr2, pPort->PortCfg.Bssid); // BSSID
        COPY_MAC_ADDR(p80211Header->Addr3, (pHeader802_3 + MAC_ADDR_LEN)); // SA

        // Add EAPOL_LLC_SNAP and the data part.
        PlatformMoveMemory((pOutBuffer + DOT11_DATA_SHORT_HEADER_SIZE), EAPOL_LLC_SNAP, sizeof(EAPOL_LLC_SNAP));
        PlatformMoveMemory((pOutBuffer + DOT11_DATA_SHORT_HEADER_SIZE + sizeof(EAPOL_LLC_SNAP)), pData, DataLen);
        packetSize = (ULONG) (DOT11_DATA_SHORT_HEADER_SIZE + sizeof(EAPOL_LLC_SNAP) + DataLen);
            
        // Allocate NET_BUFFER_LIST.
        pXcu = Ndis6CommonAllocateNdisNetBufferList(pAd,  pPort->PortNumber,(PUCHAR)(pOutBuffer), packetSize);
        MlmeFreeMemory(pAd, pOutBuffer); // The content of the pOutBuffer has been copied to NET_BUFFER_LIST.

        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR,("%s: Cannot allocate NET_BUFFER_LIST (ndisStatus = %d).\n", 
                __FUNCTION__, ndisStatus));
            break;
        }

        // Send this packet.
        DBGPRINT(RT_DEBUG_TRACE,(" %s to Aid = %d \n", __FUNCTION__, Aid));
        pXcu->Aid = (UCHAR)Aid;
        pXcu->PktSource = PKTSRC_INTERNAL;
        ApDataSendPacket(pAd, pPort->PortNumber, pXcu);
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));

        DBGPRINT(RT_DEBUG_TRACE,("<== %s\n", __FUNCTION__));
    } while (FALSE);

    return;
}


