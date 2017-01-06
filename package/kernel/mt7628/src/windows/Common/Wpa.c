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
    wpa.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Jan Lee     03-07-22        Initial
    Paul Lin    03-11-28        Modify for supplicant
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x01400000

//extern UCHAR BIT8[];
UCHAR   CipherWpaPskTkip[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x02, // Multicast
        0x01, 0x00,             // Number of unicast
        0x00, 0x50, 0xf2, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x50, 0xf2, 0x02  // authentication
        };
UCHAR   CipherWpaPskTkipLen = (sizeof(CipherWpaPskTkip) / sizeof(UCHAR));

UCHAR   CipherWpaPskAes[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x04, // Multicast
        0x01, 0x00,             // Number of unicast
        0x00, 0x50, 0xf2, 0x04, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x50, 0xf2, 0x02  // authentication
        };
UCHAR   CipherWpaPskAesLen = (sizeof(CipherWpaPskAes) / sizeof(UCHAR));

UCHAR   CipherWpa2PskTkip[] = {
        0x30,                   // RSN IE
        0x14,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x0f, 0xac, 0x02, // group cipher, TKIP
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x0f, 0xac, 0x02, // authentication
        0x00, 0x00,             // RSN capability
        };

UCHAR   CipherWpa2PskTkipLen = (sizeof(CipherWpa2PskTkip) / sizeof(UCHAR));

UCHAR   CipherWpa2PskAes[] = {
        0x30,                   // RSN IE
        0x14,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x0f, 0xac, 0x04, // group cipher, AES
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x04, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x0f, 0xac, 0x02, // authentication
        0x00, 0x00,             // RSN capability
        };

UCHAR   CipherWpa2PskAesLen = (sizeof(CipherWpa2PskAes) / sizeof(UCHAR));

UCHAR   CipherSuiteCiscoCCKMCKIP[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x40, 0x96, 0x00, // Multicast
        0x01, 0x00,             // Number of uicast
        0x00, 0x40, 0x96, 0x00, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00,
        };

UCHAR   CipherSuiteCiscoCCKMCkipCmic[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x40, 0x96, 0x01, // Multicast
        0x01, 0x00,             // Number of uicast
        0x00, 0x40, 0x96, 0x01, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00  // Authentication
        };

UCHAR   CipherSuiteCiscoCCKMCmic[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x40, 0x96, 0x02, // Multicast
        0x01, 0x00,             // Number of uicast
        0x00, 0x40, 0x96, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00,     
        };

UCHAR   CipherSuiteCiscoCCKMWep40[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x01, // Multicast, WEP-104
        0x01, 0x00,             // Number of uicast
        0x00, 0x50, 0xf2, 0x01, // unicast, WEP-104
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00  // Authentication
        };

UCHAR   CipherSuiteCiscoCCKMWep104[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x05, // Multicast, WEP-104
        0x01, 0x00,             // Number of uicast
        0x00, 0x50, 0xf2, 0x05, // unicast, WEP-104
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00  // Authentication
        };

UCHAR   CipherSuiteCiscoCCKMLen = (sizeof(CipherSuiteCiscoCCKMCkipCmic) / sizeof(UCHAR));

UCHAR   CipherSuiteCiscoCCKMTkip[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x02, // Multicast
        0x01, 0x00,             // Number of uicast
        0x00, 0x50, 0xf2, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00  // Authentication
        };

UCHAR   CipherSuiteCiscoCCKMWPALen = (sizeof(CipherSuiteCiscoCCKMTkip) / sizeof(UCHAR));

UCHAR   CipherSuiteCiscoCCKMAES[] = {
        0xDD, 0x16,             // RSN IE
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x04, // Multicast
        0x01, 0x00,             // Number of uicast
        0x00, 0x50, 0xf2, 0x04, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00 // Authentication
        };

UCHAR   CipherSuiteCiscoCCKMTkip24[] = {
        0xDD, 0x18,             // RSN IE, Length
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x02, // Multicast
        0x01, 0x00,             // Number of uicast
        0x00, 0x50, 0xf2, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00,
        0x28, 0x00// Authentication
        };

UCHAR   CipherSuiteCiscoCCKMTkip24Len = (sizeof(CipherSuiteCiscoCCKMTkip24) / sizeof(UCHAR));

UCHAR   CipherSuiteCCXTkip[] = {
        0xDD, 0x16,             // RSN IE, Length
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x02, // Multicast
        0x01, 0x00,             // Number of unicast
        0x00, 0x50, 0xf2, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x50, 0xf2, 0x01  // authentication
        };

UCHAR   CipherSuiteCCXTkipLen = (sizeof(CipherSuiteCCXTkip) / sizeof(UCHAR));

UCHAR   CipherSuiteCiscoCCKMWPA2Tkip[] = {
        0x30,                   // RSN IE
        0x14,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x0f, 0xac, 0x02, // group cipher, TKIP
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00, // Authentication
        0x28, 0x00              // RSN capability       
        };

#if 0
UCHAR   CipherSuiteCiscoCCKMWPA2AES[] = {
        0x30,                   // RSN IE
        0x18,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x0f, 0xac, 0x04, // group cipher, AES
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x04, // unicast
        0x02, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00, // Authentication
        0x00, 0x0f, 0xac, 0x01, // Authentication
        0x28, 0x00              // RSN capability
        };
#else
UCHAR   CipherSuiteCiscoCCKMWPA2AES[] = {
        0x30,                   // RSN IE
        0x14,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x0f, 0xac, 0x04, // group cipher, AES
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x04, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x40, 0x96, 0x00, // Authentication
        0x28, 0x00              // RSN capability
        };
#endif  
UCHAR   CipherSuiteCiscoCCKMWPA2Len = (sizeof(CipherSuiteCiscoCCKMWPA2AES) / sizeof(UCHAR));
/*
    ========================================================================
    
    Routine Description:
        Classify WPA EAP message type

    Arguments:
        EAPType     Value of EAP message type
        MsgType     Internal Message definition for MLME state machine
        
    Return Value:
        TRUE        Found appropriate message type
        FALSE       No appropriate message type

    IRQL = DISPATCH_LEVEL
    
    Note:
        All these constants are defined in wpa.h
        For supplicant, there is only EAPOL Key message avaliable
        
    ========================================================================
*/
BOOLEAN WpaMsgTypeSubst(
    IN  UCHAR   EAPType,
    OUT ULONG   *MsgType)   
{
    switch (EAPType)
    {
        case EAPPacket:
            *MsgType = MT2_EAPPacket;
            break;
        case EAPOLStart:
            *MsgType = MT2_EAPOLStart;
            break;
        case EAPOLLogoff:
            *MsgType = MT2_EAPOLLogoff;
            break;
        case EAPOLKey:
            *MsgType = MT2_EAPOLKey;
            break;
        case EAPOLASFAlert:
            *MsgType = MT2_EAPOLASFAlert;
            break;
        default:
            DBGPRINT(RT_DEBUG_INFO, ("WpaMsgTypeSubst : return FALSE; \n"));       
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
VOID WpaPskStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
#if(COMPILE_WIN7(CURRENT_OS_NDIS_VER))   
    StateMachineInit(S, Trans, MAX_WPA_PSK_STATE, MAX_WPA_PSK_MSG, Drop, WPA_PSK_IDLE, WPA_MACHINE_BASE);
    StateMachineSetAction(S, WPA_PSK_IDLE, MT2_EAPOLKey, WpaEAPOLKeyAction);
#endif
}

#if(COMPILE_WIN7(CURRENT_OS_NDIS_VER))     
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
VOID WpaEAPOLKeyAction(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem) 
{
    INT             MsgType;
    UCHAR           ZeroReplay[LEN_KEY_DESC_REPLAY];
    PKEY_DESCRIPTER pKeyDesc;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WpaEAPOLKeyAction\n"));
    // Get 802.11 header first
    pKeyDesc = (PKEY_DESCRIPTER) &Elem->Msg[(LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H)];

    // Sanity check, this should only happen in WPA-PSK mode
    if ((pPort->PortCfg.AuthMode != Ralink802_11AuthModeWPAPSK)&& 
        (pPort->PortCfg.AuthMode != Ndis802_11AuthModeWPA2PSK))
        return;

    // 0. Debug print all bit information
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Key Description Version %d\n", pKeyDesc->KeyInfo.KeyDescVer));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Key Type %d\n", pKeyDesc->KeyInfo.KeyType));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Key Index %d\n", pKeyDesc->KeyInfo.KeyIndex));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Install %d\n", pKeyDesc->KeyInfo.Install));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Key Ack %d\n", pKeyDesc->KeyInfo.KeyAck));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Key MIC %d\n", pKeyDesc->KeyInfo.KeyMic));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Secure %d\n", pKeyDesc->KeyInfo.Secure));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Error %d\n", pKeyDesc->KeyInfo.Error));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo Request %d\n", pKeyDesc->KeyInfo.Request));
    DBGPRINT(RT_DEBUG_INFO, ("KeyInfo DL %d\n", pKeyDesc->KeyInfo.EKD_DL));
    
    // 1. Check EAPOL frame version and type
    if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK)
    {
        if ((Elem->Msg[LENGTH_802_11+LENGTH_802_1_H] != EAPOL_VER) || (pKeyDesc->Type != WPA_KEY_DESC))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("  Key descripter does not match with WPA rule \n"));
            return;
        }
    }
    else
    {

        if ((Elem->Msg[LENGTH_802_11+LENGTH_802_1_H] != EAPOL_VER) || (pKeyDesc->Type != RSN_KEY_DESC))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Key descripter does not match with WPA2 rule\n"));
            return;
        }
    }

    // 2.Check Version for AES & TKIP
    if ((pPort->PortCfg.WepStatus ==    Ralink802_11Encryption3Enabled) && (pKeyDesc->KeyInfo.KeyDescVer != DESC_TYPE_AES))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("  Key descripter version not match AES \n"));
        return;
    }
    else if ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) && (pKeyDesc->KeyInfo.KeyDescVer != DESC_TYPE_TKIP))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("  Key descripter version not match TKIP \n"));
        return;
    }

    // First validate replay counter, only accept message with larger replay counter
    // Let equal pass, some AP start with all zero replay counter
    PlatformZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);
    if ((MtkCompareMemory(pKeyDesc->ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY) != 1) &&
        (MtkCompareMemory(pKeyDesc->ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
        return;

    // Classify message Type, either pairwise message 1, 3, or group message 1 for supplicant
    MsgType = EAPOL_MSG_INVALID;
    /*
====================================================================
        WPA2PSK     WPA2PSK         WPA2PSK     WPA2PSK
======================================================================
*/
    if (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
    {
        if ((pKeyDesc->KeyInfo.KeyType == 1) &&
            (pKeyDesc->KeyInfo.EKD_DL == 0) &&
            (pKeyDesc->KeyInfo.KeyAck == 1) &&
            (pKeyDesc->KeyInfo.KeyMic == 0) &&
            (pKeyDesc->KeyInfo.Secure == 0) &&
            (pKeyDesc->KeyInfo.Error == 0) &&
            (pKeyDesc->KeyInfo.Request == 0))
        {
            MsgType = EAPOL_PAIR_MSG_1;
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL Key Pairwise Message 1 (RSN)\n"));
        }
        else if ((pKeyDesc->KeyInfo.KeyType == 1) &&
            (pKeyDesc->KeyInfo.EKD_DL == 1) &&
            (pKeyDesc->KeyInfo.KeyAck == 1) &&
            (pKeyDesc->KeyInfo.KeyMic == 1) &&
            (pKeyDesc->KeyInfo.Secure == 1) &&
            (pKeyDesc->KeyInfo.Error == 0) &&
            (pKeyDesc->KeyInfo.Request == 0))
        {
            MsgType = EAPOL_PAIR_MSG_3;
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL Key Pairwise Message 3 (RSN)\n"));
        }
        else if ((pKeyDesc->KeyInfo.KeyType == 0) &&
            (pKeyDesc->KeyInfo.EKD_DL == 1) &&
            (pKeyDesc->KeyInfo.KeyAck == 1) &&
            (pKeyDesc->KeyInfo.KeyMic == 1) &&
            (pKeyDesc->KeyInfo.Secure == 1) &&
            (pKeyDesc->KeyInfo.Error == 0) &&
            (pKeyDesc->KeyInfo.Request == 0))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("EAP handshaks in progress, droped Group Message 1 !!!\n"));
            return;
        }

        // We will assume link is up (assoc suceess and port not secured).
        // All state has to be able to process message from previous state
        switch (pAd->StaCfg.WpaState)
        {
            case SS_START:
                if (MsgType == EAPOL_PAIR_MSG_1)
                {
                    Wpa2PairMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_MSG_3;
                }
                break;
                    
            case SS_WAIT_MSG_3:
                if (MsgType == EAPOL_PAIR_MSG_1)
                {
                    Wpa2PairMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_MSG_3;
                }
                else if (MsgType == EAPOL_PAIR_MSG_3)
                {
                    Wpa2PairMsg3Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_GROUP;
                }
                break;
                    
            case SS_WAIT_GROUP:     // When doing group key exchange
            case SS_FINISH:         // This happened when update group key
                if (MsgType == EAPOL_PAIR_MSG_1)
                {
                    Wpa2PairMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_MSG_3;
                    // Reset port secured variable
                    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                }
                else if (MsgType == EAPOL_PAIR_MSG_3)
                {
                    WpaPairMsg3Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_GROUP;
                    // Reset port secured variable
                    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                }
                else if (MsgType == EAPOL_GROUP_MSG_1)
                {
                    WpaGroupMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_FINISH;
                }
                break;
                    
            default:
                break;              
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("<----- WpaEAPOLKeyAction\n"));
    }
///*
//====================================================================
//          WPAPSK          WPAPSK          WPAPSK          WPAPSK
//======================================================================
//*/
    else if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK)
    {
        if ((pKeyDesc->KeyInfo.KeyType == 1) &&
            (pKeyDesc->KeyInfo.KeyIndex == 0) &&
            (pKeyDesc->KeyInfo.KeyAck == 1) &&
            (pKeyDesc->KeyInfo.KeyMic == 0) &&
            (pKeyDesc->KeyInfo.Secure == 0) &&
            (pKeyDesc->KeyInfo.Error == 0) &&
            (pKeyDesc->KeyInfo.Request == 0))
        {
            MsgType = EAPOL_PAIR_MSG_1;
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL Key Pairwise Message 1\n"));
        }
        else if ((pKeyDesc->KeyInfo.KeyType == 1) &&
            (pKeyDesc->KeyInfo.KeyIndex == 0) &&
            (pKeyDesc->KeyInfo.KeyAck == 1) &&
            (pKeyDesc->KeyInfo.KeyMic == 1) &&
            (pKeyDesc->KeyInfo.Secure == 0) &&
            (pKeyDesc->KeyInfo.Error == 0) &&
            (pKeyDesc->KeyInfo.Request == 0))
        {
            MsgType = EAPOL_PAIR_MSG_3;
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL Key Pairwise Message 3\n"));
        }
        else if ((pKeyDesc->KeyInfo.KeyType == 0) &&
            (pKeyDesc->KeyInfo.KeyIndex != 0) &&
            (pKeyDesc->KeyInfo.KeyAck == 1) &&
            (pKeyDesc->KeyInfo.KeyMic == 1) &&
            (pKeyDesc->KeyInfo.Secure == 1) &&
            (pKeyDesc->KeyInfo.Error == 0) &&
            (pKeyDesc->KeyInfo.Request == 0))
        {
            MsgType = EAPOL_GROUP_MSG_1;
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL Key Group Message 1\n"));
        }
        
        // We will assume link is up (assoc suceess and port not secured).
        // All state has to be able to process message from previous state
        switch (pAd->StaCfg.WpaState)
        {
            case SS_START:
                if (MsgType == EAPOL_PAIR_MSG_1)
                {
                    WpaPairMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_MSG_3;
                }
                break;
                    
            case SS_WAIT_MSG_3:
                if (MsgType == EAPOL_PAIR_MSG_1)
                {
                    WpaPairMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_MSG_3;
                }
                else if (MsgType == EAPOL_PAIR_MSG_3)
                {
                    WpaPairMsg3Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_GROUP;
                }
                break;
                    
            case SS_WAIT_GROUP:     // When doing group key exchange
            case SS_FINISH:         // This happened when update group key
                if (MsgType == EAPOL_PAIR_MSG_1)
                {
                    WpaPairMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_MSG_3;
                    // Reset port secured variable
                    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                }
                else if (MsgType == EAPOL_PAIR_MSG_3)
                {
                    WpaPairMsg3Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_WAIT_GROUP;
                    // Reset port secured variable
                    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                }
                else if (MsgType == EAPOL_GROUP_MSG_1)
                {
                    WpaGroupMsg1Action(pAd, Elem);
                    pAd->StaCfg.WpaState = SS_FINISH;
                }
                break;
                    
            default:
                break;              
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WpaEAPOLKeyAction\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Process Pairwise key 4-way handshaking

    Arguments:
        pAd Pointer to our adapter
        Elem        Message body
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/
VOID    WpaPairMsg1Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem) 
{
    PHEADER_802_11      pHeader;
    UCHAR               PTK[80];
    PUCHAR              pOutBuffer = NULL;
    HEADER_802_11       Header_802_11;
    NDIS_STATUS         NStatus;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,0x88,0x8e};
    PEAPOL_PACKET       pMsg1;
    EAPOL_PACKET        Packet;
    UCHAR               Mic[16];    

    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
       
    DBGPRINT(RT_DEBUG_TRACE, ("WpaPairMsg1Action ----->\n"));
    
    pHeader = (PHEADER_802_11) Elem->Msg;
    
    // Save Data Length to pDesc for receiving packet, then put in outgoing frame   Data Len fields.
    pMsg1 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    
    // Process message 1 from authenticator
    // Key must be Pairwise key, already verified at callee.
    // 1. Save Replay counter, it will use to verify message 3 and construct message 2
    PlatformMoveMemory(pAd->StaCfg.ReplayCounter, pMsg1->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);       

    // 2. Save ANonce
    PlatformMoveMemory(pAd->StaCfg.ANonce, pMsg1->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);
        
    // TSNonce <--- SNonce
    // Generate random SNonce
    GenRandom(pAd,  pAd->StaCfg.SNonce);  

    {
        // TPTK <--- Calc PTK(ANonce, TSNonce)
        WpaApWpaCountPTK(pAd,
                    pPort,
                    pAd->StaCfg.PskKey.Key, 
                    pAd->StaCfg.ANonce,
                    pPort->PortCfg.Bssid, 
                    pAd->StaCfg.SNonce, 
                    pAd->HwCfg.CurrentAddress,     
                    PTK, 
                    LEN_PTK);   
    }
    // Save key to PTK entry
    PlatformMoveMemory(pAd->StaCfg.PTK, PTK, LEN_PTK);
    
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence ++;
    WpaMacHeaderInit(pAd, pPort, &Header_802_11, 0, pPort->PortCfg.Bssid);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;
    
    // Zero message 2 body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLKey;
    //
    // Message 2 as  EAPOL-Key(0,1,0,0,0,P,0,SNonce,MIC,RSN IE)
    //
    Packet.KeyDesc.Type = WPA_KEY_DESC;
    // 1. Key descriptor version and appropriate RSN IE
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        Packet.KeyDesc.KeyInfo.KeyDescVer = 2;
        Packet.KeyDesc.KeyDataLen[1] = CipherWpaPskAesLen;
        PlatformMoveMemory(Packet.KeyDesc.KeyData, CipherWpaPskAes, CipherWpaPskAesLen);
    }
    else    // TKIP
    {       
            Packet.KeyDesc.KeyInfo.KeyDescVer = 1;
            Packet.KeyDesc.KeyDataLen[1] = CipherWpaPskTkipLen;
            PlatformMoveMemory(Packet.KeyDesc.KeyData, CipherWpaPskTkip, CipherWpaPskTkipLen);
    }   
    // Update packet length after decide Key data payload
    Packet.Len[1]  = sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE + Packet.KeyDesc.KeyDataLen[1];

    // Update Key length 
    Packet.KeyDesc.KeyLength[0] = pMsg1->KeyDesc.KeyLength[0];
    Packet.KeyDesc.KeyLength[1] = pMsg1->KeyDesc.KeyLength[1];  
    // 2. Key Type PeerKey
    Packet.KeyDesc.KeyInfo.KeyType = 1;

    // 3. KeyMic field presented
    Packet.KeyDesc.KeyInfo.KeyMic  = 1;

    // 4. Fill SNonce
    PlatformMoveMemory(Packet.KeyDesc.KeyNonce, pAd->StaCfg.SNonce, LEN_KEY_DESC_NONCE);

    // 5. Key Replay Count
    PlatformMoveMemory(Packet.KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY);       
    
    // Send EAPOL(0, 1, 0, 0, 0, K, 0, TSNonce, 0, MIC(TPTK), 0)
    // Out buffer for transmitting message 2        
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    // Prepare EAPOL frame for MIC calculation
    // Be careful, only EAPOL frame is counted for MIC calculation
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // 5. Prepare and Fill MIC value
    PlatformZeroMemory(Mic, sizeof(Mic));
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1(pOutBuffer, FrameLen, PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        INT i;
        DBGPRINT_RAW(RT_DEBUG_INFO, (" PMK = "));
        for (i = 0; i < 16; i++)
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%2x-", pAd->StaCfg.PskKey.Key[i]));
        
        DBGPRINT_RAW(RT_DEBUG_INFO, ("\n PTK = "));
        for (i = 0; i < 64; i++)
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%2x-", pAd->StaCfg.PTK[i]));
        DBGPRINT_RAW(RT_DEBUG_INFO, ("\n FrameLen = %d\n", FrameLen));      
        
        hmac_md5(PTK,  LEN_EAP_MICK, pOutBuffer, FrameLen, Mic);
    }
    PlatformMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);

    MakeOutgoingFrame(pOutBuffer,           &FrameLen,  
                    sizeof(HEADER_802_11),&Header_802_11,
                    sizeof(EAPHEAD),      EAPHEAD, 
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
        
    DBGPRINT(RT_DEBUG_TRACE, ("WpaPairMsg1Action <-----\n"));
}

VOID    Wpa2PairMsg1Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem)
{
    PHEADER_802_11      pHeader;
    UCHAR               PTK[80];
    PUCHAR              pOutBuffer = NULL;
    HEADER_802_11       Header_802_11;
    NDIS_STATUS         NStatus;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,0x88,0x8e};
    PEAPOL_PACKET       pMsg1;
    EAPOL_PACKET        Packet;
    UCHAR               Mic[16];
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];

       
    DBGPRINT(RT_DEBUG_TRACE, ("Wpa2PairMsg1Action ----->\n"));
    
    pHeader = (PHEADER_802_11) Elem->Msg;
    
    // Save Data Length to pDesc for receiving packet, then put in outgoing frame   Data Len fields.
    pMsg1 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    
    // Process message 1 from authenticator
    // Key must be Pairwise key, already verified at callee.
    // 1. Save Replay counter, it will use to verify message 3 and construct message 2
    PlatformMoveMemory(pAd->StaCfg.ReplayCounter, pMsg1->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);       

    // 2. Save ANonce
    PlatformMoveMemory(pAd->StaCfg.ANonce, pMsg1->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);
        
    // TSNonce <--- SNonce
    // Generate random SNonce
    GenRandom(pAd,  pAd->StaCfg.SNonce);  

    {
        // TPTK <--- Calc PTK(ANonce, TSNonce)
        WpaApWpaCountPTK(pAd,
                    pPort,
                    pAd->StaCfg.PskKey.Key, 
                    pAd->StaCfg.ANonce,
                    pPort->CommonCfg.Bssid, 
                    pAd->StaCfg.SNonce, 
                    pAd->HwCfg.CurrentAddress,     
                    PTK, 
                    LEN_PTK);   
    }
    // Save key to PTK entry
    PlatformMoveMemory(pAd->StaCfg.PTK, PTK, LEN_PTK);
    
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    WpaMacHeaderInit(pAd, pPort, &Header_802_11, 0, pPort->CommonCfg.Bssid);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;
    
    // Zero message 2 body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLKey;
    //
    // Message 2 as  EAPOL-Key(0,1,0,0,0,P,0,SNonce,MIC,RSN IE)
    //
    Packet.KeyDesc.Type = RSN_KEY_DESC;
    // 1. Key descriptor version and appropriate RSN IE
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeyDescVer = AES",__FUNCTION__));
        Packet.KeyDesc.KeyInfo.KeyDescVer = 2;
        Packet.KeyDesc.KeyDataLen[1] = CipherWpa2PskAesLen;
        PlatformMoveMemory(Packet.KeyDesc.KeyData, CipherWpa2PskAes, CipherWpa2PskAesLen);
    }
    else    // TKIP
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeyDescVer = TKIP",__FUNCTION__));
        Packet.KeyDesc.KeyInfo.KeyDescVer = 1;
        Packet.KeyDesc.KeyDataLen[1] = CipherWpa2PskTkipLen;
        PlatformMoveMemory(Packet.KeyDesc.KeyData, CipherWpa2PskTkip, CipherWpa2PskTkipLen);
    }
    // Update packet length after decide Key data payload
    Packet.Len[1]  = sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE + Packet.KeyDesc.KeyDataLen[1];

    // Update Key length 
    Packet.KeyDesc.KeyLength[0] = pMsg1->KeyDesc.KeyLength[0];
    Packet.KeyDesc.KeyLength[1] = pMsg1->KeyDesc.KeyLength[1];  
    // 2. Key Type PeerKey
    Packet.KeyDesc.KeyInfo.KeyType = 1;

    // 3. KeyMic field presented
    Packet.KeyDesc.KeyInfo.KeyMic  = 1;

    // 4. Fill SNonce
    PlatformMoveMemory(Packet.KeyDesc.KeyNonce, pAd->StaCfg.SNonce, LEN_KEY_DESC_NONCE);

    // 5. Key Replay Count
    PlatformMoveMemory(Packet.KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY);       
    
    // Send EAPOL(0, 1, 0, 0, 0, K, 0, TSNonce, 0, MIC(TPTK), 0)
    // Out buffer for transmitting message 2        
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    // Prepare EAPOL frame for MIC calculation
    // Be careful, only EAPOL frame is counted for MIC calculation
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // 5. Prepare and Fill MIC value
    PlatformZeroMemory(Mic, sizeof(Mic));
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1(pOutBuffer, FrameLen, PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        INT i;
        DBGPRINT_RAW(RT_DEBUG_TRACE, (" PMK = "));
        for (i = 0; i < 16; i++)
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%2x-", pAd->StaCfg.PskKey.Key[i]));
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n PTK = "));
        for (i = 0; i < 64; i++)
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%2x-", pAd->StaCfg.PTK[i]));
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n FrameLen = %d\n", FrameLen));
        
        hmac_md5(PTK,  LEN_EAP_MICK, pOutBuffer, FrameLen, Mic);
    }
    PlatformMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);

    MakeOutgoingFrame(pOutBuffer,           &FrameLen,  
                    sizeof(HEADER_802_11),&Header_802_11,
                    sizeof(EAPHEAD),      EAPHEAD, 
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
        
    DBGPRINT(RT_DEBUG_TRACE, ("Wpa2PairMsg1Action <-----\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Process Pairwise key 4-way handshaking

    Arguments:
        pAd Pointer to our adapter
        Elem        Message body
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/
VOID    WpaPairMsg3Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem) 
{
    NDIS_STATUS         Status;
    PHEADER_802_11      pHeader;
    PUCHAR              pOutBuffer = NULL;
    HEADER_802_11       Header_802_11;
    NDIS_STATUS         NStatus;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,0x88,0x8e};
    EAPOL_PACKET        Packet;
    PEAPOL_PACKET       pMsg3;
    PUCHAR              pTmp = NULL;
    UCHAR               Mic[16], OldMic[16];    
    PNDIS_802_11_KEY    pPeerKey;
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    
       
    DBGPRINT(RT_DEBUG_TRACE, ("WpaPairMsg3Action ----->\n"));
    
    pHeader = (PHEADER_802_11) Elem->Msg;
    
    // Process message 3 frame.
    pMsg3 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];

    // 1. Verify RSN IE & cipher type match
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        if (pMsg3->KeyDesc.KeyInfo.KeyDescVer != 2)
            return;
        pTmp = (PUCHAR) &CipherWpaPskAes;
    }
    else    // TKIP
    {
        if (pMsg3->KeyDesc.KeyInfo.KeyDescVer != 1)
            return;
    
        pTmp = (PUCHAR) &CipherWpaPskTkip;
    }

    // Fix compatibility issue, when AP append nonsense data after auth mode with different size.
    // We should qualify this kind of RSN as acceptable
    if ((pTmp != NULL) && (!PlatformEqualMemory((PUCHAR) &pMsg3->KeyDesc.KeyData[2], pTmp + 2, CipherWpaPskTkipLen - 2)))
    {
        DBGPRINT(RT_DEBUG_ERROR, (" RSN IE mismatched msg 3 of 4-way handshake!!!!!!!!!! \n"));
        return;
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" RSN IE matched in msg 3 of 4-way handshake!!!!!!!!!! \n"));
    
    // 2. Check MIC value
    // Save the MIC and replace with zero
    PlatformMoveMemory(OldMic, pMsg3->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
    PlatformZeroMemory(pMsg3->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1((PUCHAR) pMsg3, pMsg3->Len[1] + 4, pAd->StaCfg.PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        hmac_md5(pAd->StaCfg.PTK, LEN_EAP_MICK, (PUCHAR) pMsg3, pMsg3->Len[1] + 4, Mic);
    }
    
    if (!PlatformEqualMemory(OldMic, Mic, LEN_KEY_DESC_MIC))
    {
        DBGPRINT(RT_DEBUG_ERROR, (" MIC Different in msg 3 of 4-way handshake!!!!!!!!!! \n"));
        return;
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" MIC VALID in msg 3 of 4-way handshake!!!!!!!!!! \n"));

    // 3. Check Replay Counter, it has to be larger than last one. No need to be exact one larger
    if (MtkCompareMemory(pMsg3->KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY) != 1)
        return;

    // Update new replay counter
    PlatformMoveMemory(pAd->StaCfg.ReplayCounter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);       

    // 4. Double check ANonce
    if (!PlatformEqualMemory(pAd->StaCfg.ANonce, pMsg3->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
        return;
    
    // 5. Construct Message 4
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence ++;
    WpaMacHeaderInit(pAd, pPort, &Header_802_11, 0, pPort->PortCfg.Bssid);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;
    
    // Zero message 4 body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLKey;
    Packet.Len[1]  = sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE;     // No data field
    
    //
    // Message 4 as  EAPOL-Key(0,1,0,0,0,P,0,0,MIC,0)
    //
    Packet.KeyDesc.Type = RSN_KEY_DESC;
    
    // Key descriptor version and appropriate RSN IE
    Packet.KeyDesc.KeyInfo.KeyDescVer = pMsg3->KeyDesc.KeyInfo.KeyDescVer;

    // Update Key Length
    Packet.KeyDesc.KeyLength[0] = pMsg3->KeyDesc.KeyLength[0];
    Packet.KeyDesc.KeyLength[1] = pMsg3->KeyDesc.KeyLength[1];
    
    // Key Type PeerKey
    Packet.KeyDesc.KeyInfo.KeyType = 1;

    // KeyMic field presented
    Packet.KeyDesc.KeyInfo.KeyMic  = 1;

    // Key Replay count 
    PlatformMoveMemory(Packet.KeyDesc.ReplayCounter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);        

    // Out buffer for transmitting message 4        
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    // Prepare EAPOL frame for MIC calculation
    // Be careful, only EAPOL frame is counted for MIC calculation
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // Prepare and Fill MIC value
    PlatformZeroMemory(Mic, sizeof(Mic));
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1(pOutBuffer, FrameLen, pAd->StaCfg.PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        hmac_md5(pAd->StaCfg.PTK, LEN_EAP_MICK, pOutBuffer, FrameLen, Mic);
    }
    PlatformMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);

    // Make  Transmitting frame
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                    sizeof(HEADER_802_11),&Header_802_11,
                    sizeof(EAPHEAD),      EAPHEAD, 
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // 6. Send Message 4 to authenticator
    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    // 7. Update PTK
    Status = MlmeAllocateMemory(pAd, &pPeerKey);  //Get an unused nonpaged memory
    if (Status != NDIS_STATUS_SUCCESS) 
        return;
    
    PlatformZeroMemory(pPeerKey, sizeof(NDIS_802_11_KEY) + LEN_EAP_KEY);
    pPeerKey->Length    = sizeof(NDIS_802_11_KEY) + LEN_EAP_KEY;
    pPeerKey->KeyIndex  = 0xe0000000;
    pPeerKey->KeyLength = pMsg3->KeyDesc.KeyLength[0] * 256 + pMsg3->KeyDesc.KeyLength[1];

    COPY_MAC_ADDR(pPeerKey->BSSID, pPort->PortCfg.Bssid);
    PlatformMoveMemory(&pPeerKey->KeyRSC, pMsg3->KeyDesc.KeyRsc, LEN_KEY_DESC_RSC);
    PlatformMoveMemory(pPeerKey->KeyMaterial, &pAd->StaCfg.PTK[32], LEN_EAP_KEY);
    // Call Add peer key function
    MlmeInfoWPAAddKeyProc(pAd, pPort, pPeerKey);
    MlmeFreeMemory(pAd, pPeerKey);
    
    DBGPRINT(RT_DEBUG_TRACE, ("WpaPairMsg3Action <-----\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Process Pairwise key 4-way handshaking

    Arguments:
        pAd Pointer to our adapter
        Elem        Message body
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/
VOID    Wpa2PairMsg3Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem)
{
    NDIS_STATUS             Status;
    PHEADER_802_11      pHeader;
    PUCHAR              pOutBuffer = NULL;
    HEADER_802_11       Header_802_11;
    NDIS_STATUS         NStatus;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,0x88,0x8e};
    EAPOL_PACKET        Packet;
    PEAPOL_PACKET       pMsg3;
    PUCHAR              pTmp;
    UCHAR               Mic[16], OldMic[16];
    PNDIS_802_11_KEY    pPeerKey;
    PUCHAR              pKeyData;
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    
       
    DBGPRINT(RT_DEBUG_TRACE, ("Wpa2PairMsg3Action ----->\n"));
    
    pHeader = (PHEADER_802_11) Elem->Msg;
    
    // Process message 3 frame.
    pMsg3 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];

    // 1. Verify RSN IE & cipher type match
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {
        if (pMsg3->KeyDesc.KeyInfo.KeyDescVer != 2)
            return;
        pTmp = (PUCHAR) &CipherWpa2PskAes;
    }
    else    // TKIP
    {
        if (pMsg3->KeyDesc.KeyInfo.KeyDescVer != 1)
            return;

        pTmp = (PUCHAR) &CipherWpa2PskTkip;
    }

    // Fix compatibility issue, when AP append nonsense data after auth mode with different size.
    // We should qualify this kind of RSN as acceptable

    pKeyData = &pMsg3->KeyDesc.KeyData[0];

     DBGPRINT(RT_DEBUG_TRACE, ("%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x",
        pKeyData[0],pKeyData[1],pKeyData[2],pKeyData[3],pKeyData[4],pKeyData[5],pKeyData[6],pKeyData[7],pKeyData[8],pKeyData[9],pKeyData[10]));
     DBGPRINT(RT_DEBUG_TRACE, ("%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x",
        pKeyData[11],pKeyData[12],pKeyData[13],pKeyData[14],pKeyData[15],pKeyData[16],pKeyData[17],pKeyData[18],pKeyData[19],pKeyData[20],pKeyData[21]));           

/*  
    if (!PlatformEqualMemory((PUCHAR) &pMsg3->KeyDesc.KeyData[2], pTmp + 2, CipherWpa2PskTkipLen - 2))
    {
        DBGPRINT(RT_DEBUG_ERROR, (" RSN IE mismatched msg 3 of 4-way handshake!!!!!!!!!! \n"));
        return;
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" RSN IE matched in msg 3 of 4-way handshake!!!!!!!!!! \n"));
*/  
    // 2. Check MIC value
    // Save the MIC and replace with zero
    PlatformZeroMemory(OldMic, LEN_KEY_DESC_MIC);
    PlatformZeroMemory(Mic, LEN_KEY_DESC_MIC);
    PlatformMoveMemory(OldMic, pMsg3->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
    PlatformZeroMemory(pMsg3->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1((PUCHAR) pMsg3, pMsg3->Len[1] + 4, pAd->StaCfg.PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        hmac_md5(pAd->StaCfg.PTK, LEN_EAP_MICK, (PUCHAR) pMsg3, pMsg3->Len[1] + 4, Mic);
    }
    
    if (!PlatformEqualMemory(OldMic, Mic, LEN_KEY_DESC_MIC))
    {
        DBGPRINT(RT_DEBUG_ERROR, (" MIC Different in msg 3 of 4-way handshake!!!!!!!!!! \n"));
        return;
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" MIC VALID in msg 3 of 4-way handshake!!!!!!!!!! \n"));

    // 3. Check Replay Counter, it has to be larger than last one. No need to be exact one larger
    if (MtkCompareMemory(pMsg3->KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY) != 1)
        return;

    // Update new replay counter
    PlatformMoveMemory(pAd->StaCfg.ReplayCounter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);       

    // 4. Double check ANonce
    if (!PlatformEqualMemory(pAd->StaCfg.ANonce, pMsg3->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
        return;

    //
    // Obtain GTK
    // For BT30 we discard the GTK here !!
    //
    
    // 5. Construct Message 4
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    WpaMacHeaderInit(pAd, pPort, &Header_802_11, 0, pPort->CommonCfg.Bssid);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;
    
    // Zero message 4 body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLKey;
    Packet.Len[1]  = sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE;     // No data field
    
    //
    // Message 4 as  EAPOL-Key(0,1,0,0,0,P,0,0,MIC,0)
    //
    Packet.KeyDesc.Type = RSN_KEY_DESC;
    
    // Key descriptor version and appropriate RSN IE
    Packet.KeyDesc.KeyInfo.KeyDescVer = pMsg3->KeyDesc.KeyInfo.KeyDescVer;

    // Update Key Length
    Packet.KeyDesc.KeyLength[0] = pMsg3->KeyDesc.KeyLength[0];
    Packet.KeyDesc.KeyLength[1] = pMsg3->KeyDesc.KeyLength[1];
    
    // Key Type PeerKey
    Packet.KeyDesc.KeyInfo.KeyType = 1;

    // KeyMic field presented
    Packet.KeyDesc.KeyInfo.KeyMic  = 1;
    // No group key HS needed
    Packet.KeyDesc.KeyInfo.Secure = 1;

    // Key Replay count 
    PlatformMoveMemory(Packet.KeyDesc.ReplayCounter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);        

    // Out buffer for transmitting message 4        
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    // Prepare EAPOL frame for MIC calculation
    // Be careful, only EAPOL frame is counted for MIC calculation
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // Prepare and Fill MIC value
    PlatformZeroMemory(Mic, sizeof(Mic));
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1(pOutBuffer, FrameLen, pAd->StaCfg.PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        hmac_md5(pAd->StaCfg.PTK, LEN_EAP_MICK, pOutBuffer, FrameLen, Mic);
    }
    PlatformMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);

    // Make  Transmitting frame
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                    sizeof(HEADER_802_11),&Header_802_11,
                    sizeof(EAPHEAD),      EAPHEAD, 
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // 6. Send Message 4 to authenticator
    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    // 7. Update PTK
    Status = MlmeAllocateMemory(pAd, &pPeerKey);  //Get an unused nonpaged memory
    if (Status != NDIS_STATUS_SUCCESS) 
        return;
    
    PlatformZeroMemory(pPeerKey, sizeof(NDIS_802_11_KEY) + LEN_EAP_KEY);
    pPeerKey->Length    = sizeof(NDIS_802_11_KEY) + LEN_EAP_KEY;
    pPeerKey->KeyIndex  = 0xe0000000;
    pPeerKey->KeyLength = pMsg3->KeyDesc.KeyLength[0] * 256 + pMsg3->KeyDesc.KeyLength[1];

    COPY_MAC_ADDR(pPeerKey->BSSID, pPort->CommonCfg.Bssid);
    PlatformMoveMemory(&pPeerKey->KeyRSC, pMsg3->KeyDesc.KeyRsc, LEN_KEY_DESC_RSC);
    PlatformMoveMemory(pPeerKey->KeyMaterial, &pAd->StaCfg.PTK[32], LEN_EAP_KEY);
    // Call Add peer key function
    MlmeInfoWPAAddKeyProc(pAd, pPort, pPeerKey);
    MlmeFreeMemory(pAd, pPeerKey);

    // Bluetooth BT30 Authen success
    //MlmeEnqueue(pAd, pPort, BT_STATE_MACHINE, Four_Way_Handshake_Success, 0, NULL);
    
    DBGPRINT(RT_DEBUG_TRACE, ("Wpa2PairMsg3Action <-----\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Process Group key 2-way handshaking

    Arguments:
        pAd Pointer to our adapter
        Elem        Message body
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/
VOID    WpaGroupMsg1Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem) 
{
    NDIS_STATUS         Status;
    PHEADER_802_11      pHeader;
    PUCHAR              pOutBuffer = NULL;
    HEADER_802_11       Header_802_11;
    NDIS_STATUS         NStatus;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,0x88,0x8e};
    EAPOL_PACKET        Packet;
    PEAPOL_PACKET       pGroup;
    UCHAR               Mic[16], OldMic[16];
    UCHAR               GTK[32], Key[32];
    PNDIS_802_11_KEY    pGroupKey;  
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
       
    DBGPRINT(RT_DEBUG_TRACE, ("WpaGroupMsg1Action ----->\n"));
    
    pHeader = (PHEADER_802_11) Elem->Msg;
    
    // Process Group message 1 frame.
    pGroup = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];

    // 1. Verify Replay counter
    //    Check Replay Counter, it has to be larger than last one. No need to be exact one larger
    if (MtkCompareMemory(pGroup->KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY) != 1)
        return;

    // Update new replay counter
    PlatformMoveMemory(pAd->StaCfg.ReplayCounter, pGroup->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);      

    // 2. Verify MIC is valid
    // Save the MIC and replace with zero
    PlatformMoveMemory(OldMic, pGroup->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
    PlatformZeroMemory(pGroup->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1((PUCHAR) pGroup, pGroup->Len[1] + 4, pAd->StaCfg.PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        hmac_md5(pAd->StaCfg.PTK, LEN_EAP_MICK, (PUCHAR) pGroup, pGroup->Len[1] + 4, Mic);
    }
    
    if (!PlatformEqualMemory(OldMic, Mic, LEN_KEY_DESC_MIC))
    {
        DBGPRINT(RT_DEBUG_ERROR, (" MIC Different in group msg 1 of 2-way handshake!!!!!!!!!! \n"));
        return;
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" MIC VALID in group msg 1 of 2-way handshake!!!!!!!!!! \n"));

    // 3. Decrypt GTK from Key Data
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        if (pGroup->KeyDesc.KeyInfo.KeyDescVer != 2)
            return;
        // Decrypt AES GTK
        AES_GTK_KEY_UNWRAP(&pAd->StaCfg.PTK[16], GTK, pGroup->KeyDesc.KeyData);     
    }
    else    // TKIP
    {
        INT i;
        
        if (pGroup->KeyDesc.KeyInfo.KeyDescVer != 1)
            return;
        // Decrypt TKIP GTK
        // Construct 32 bytes RC4 Key
        PlatformMoveMemory(Key, pGroup->KeyDesc.KeyIv, 16);
        PlatformMoveMemory(&Key[16], &pAd->StaCfg.PTK[16], 16);
        ARCFOUR_INIT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, Key, 32);
        //discard first 256 bytes
        for (i = 0; i < 256; i++)
            ARCFOUR_BYTE(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT);
        // Decrypt GTK. Becareful, there is no ICV to check the result is correct or not
        ARCFOUR_DECRYPT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, GTK, pGroup->KeyDesc.KeyData, 32);        
    }
    
    // 4. Construct Group Message 2
    pAd->pTxCfg->Sequence ++;
    WpaMacHeaderInit(pAd, pPort, &Header_802_11, 1, pPort->PortCfg.Bssid);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd,  AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;
    
    // Zero Group message 1 body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLKey;
    Packet.Len[1]  = sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE;     // No data field
    
    //
    // Group Message 2 as  EAPOL-Key(1,0,0,0,G,0,0,MIC,0)
    //
    Packet.KeyDesc.Type = RSN_KEY_DESC;
    
    // Key descriptor version and appropriate RSN IE
    Packet.KeyDesc.KeyInfo.KeyDescVer = pGroup->KeyDesc.KeyInfo.KeyDescVer;

    // Update Key Length and Key Index
    Packet.KeyDesc.KeyInfo.KeyIndex = pGroup->KeyDesc.KeyInfo.KeyIndex;
    Packet.KeyDesc.KeyLength[0] = pGroup->KeyDesc.KeyLength[0];
    Packet.KeyDesc.KeyLength[1] = pGroup->KeyDesc.KeyLength[1]; 

    // Key Type Group key
    Packet.KeyDesc.KeyInfo.KeyType = 0;

    // KeyMic field presented
    Packet.KeyDesc.KeyInfo.KeyMic  = 1;

    // Secure bit is 1
    Packet.KeyDesc.KeyInfo.Secure  = 1;
    
    // Key Replay count 
    PlatformMoveMemory(Packet.KeyDesc.ReplayCounter, pGroup->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);       

    // Out buffer for transmitting group message 2      
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    // Prepare EAPOL frame for MIC calculation
    // Be careful, only EAPOL frame is counted for MIC calculation
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // Prepare and Fill MIC value
    PlatformZeroMemory(Mic, sizeof(Mic));
    if (pPort->PortCfg.WepStatus    == Ralink802_11Encryption3Enabled)
    {
        // AES
        UCHAR digest[80];
            
        HMAC_SHA1(pOutBuffer, FrameLen, pAd->StaCfg.PTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        INT i;
        DBGPRINT_RAW(RT_DEBUG_INFO, ("PTK = "));
        for (i = 0; i < 64; i++)
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%2x-", pAd->StaCfg.PTK[i]));
        DBGPRINT_RAW(RT_DEBUG_INFO, ("\n FrameLen = %d\n", FrameLen));
            
        hmac_md5(pAd->StaCfg.PTK, LEN_EAP_MICK, pOutBuffer, FrameLen, Mic);
    }
    PlatformMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);

    // Make Transmitting frame
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,  
                    sizeof(HEADER_802_11),&Header_802_11,
                    sizeof(EAPHEAD),      EAPHEAD, 
                    Packet.Len[1] + 4,    &Packet,
                    END_OF_ARGS);

    // 5. Copy frame to Tx ring and prepare for encryption
    WpaHardTransmit(pAd, pPort, pOutBuffer, FrameLen);

    // 6 Free allocated memory
    MlmeFreeMemory(pAd, pOutBuffer);
    
    // 6. Update GTK
    Status = MlmeAllocateMemory(pAd, &pGroupKey);  //Get an unused nonpaged memory
    if (Status != NDIS_STATUS_SUCCESS) 
        return;
    
    PlatformZeroMemory(pGroupKey, sizeof(NDIS_802_11_KEY) + LEN_EAP_KEY);
    pGroupKey->Length    = sizeof(NDIS_802_11_KEY) + LEN_EAP_KEY;
    pGroupKey->KeyIndex  = 0x20000000 | pGroup->KeyDesc.KeyInfo.KeyIndex;
    pGroupKey->KeyLength = pGroup->KeyDesc.KeyLength[0] * 256 + pGroup->KeyDesc.KeyLength[1];

    COPY_MAC_ADDR(pGroupKey->BSSID, pPort->PortCfg.Bssid);
    PlatformMoveMemory(pGroupKey->KeyMaterial, GTK, LEN_EAP_KEY);   
    
    // Call Add peer key function   
    MlmeInfoWPAAddKeyProc(pAd, pPort, pGroupKey);
    MlmeFreeMemory(pAd, pGroupKey);
    
    DBGPRINT(RT_DEBUG_TRACE, ("WpaGroupMsg1Action <-----\n"));
}
#endif //#if defined(NDIS620_MINIPORT)

/*
    ========================================================================
    
    Routine Description:
        Init WPA MAC header

    Arguments:
        pAd Pointer to our adapter
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/
VOID    WpaMacHeaderInit(
    IN      PMP_ADAPTER   pAd, 
    IN      PMP_PORT      pPort,
    IN OUT  PHEADER_802_11  pHdr80211, 
    IN      UCHAR           wep, 
    IN      PUCHAR          pAddr1) 
{
    PlatformZeroMemory(pHdr80211, sizeof(HEADER_802_11));
    pHdr80211->FC.Type  = BTYPE_DATA;   
    pHdr80211->FC.ToDs  = 1;
    if (wep == 1)
        pHdr80211->FC.Wep = 1;
    
     // Addr1: DA, Addr2: BSSID, Addr3: SA
    COPY_MAC_ADDR(pHdr80211->Addr1, pAddr1);
    COPY_MAC_ADDR(pHdr80211->Addr2, pAd->HwCfg.CurrentAddress);
    COPY_MAC_ADDR(pHdr80211->Addr3, pPort->PortCfg.Bssid);
    pHdr80211->Sequence =   pAd->pTxCfg->Sequence;      
}

#if(COMPILE_WIN7(CURRENT_OS_NDIS_VER))   
/*
    ========================================================================

    Routine Description:
        Copy frame from waiting queue into relative ring buffer and set 
    appropriate ASIC register to kick hardware encryption before really
    sent out to air.
        
    Arguments:
        pAd     Pointer to our adapter
        PNDIS_PACKET    Pointer to outgoing Ndis frame
        NumberOfFrag    Number of fragment required
        
    Return Value:
        None

    Note:
    
    ========================================================================
*/
VOID    WpaHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pFrame,
    IN  ULONG           FrameLen)
{
    UCHAR           *OutBuffer = NULL;
    PUCHAR          pSrc, pDest;
    ULONG           Iv16;
    ULONG           Iv32;
    PCIPHER_KEY     pWpaKey;
    NDIS_STATUS     NStatus;
    UCHAR           i;
    UCHAR           RC4Key[16];
    UINT            p1k[5]; //for mix_key;
    ULONG           pnl;/* Least significant 16 bits of PN */
    ULONG           pnh;/* Most significant 32 bits of PN */ 
    
    NStatus = MlmeAllocateMemory(pAd, (PVOID)&OutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("ASSOC - MlmeAssocReqAction() allocate memory failed \n"));
        pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        MlmeCntlConfirm(pAd, MT2_ASSOC_CONF, MLME_FAIL_NO_RESOURCE);
        return;
    }
    pDest = (PUCHAR)OutBuffer;
    pSrc = pFrame;
    
    // outgoing frame always wakeup PHY to prevent frame lost and 
    // turn off PSM bit to improve performance
    if (pAd->StaCfg.Psm == PWR_SAVE)
    {
        MlmeSetPsm(pAd, PWR_ACTIVE);
    }
    
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        AsicForceWakeup(pAd);
    
    pWpaKey = (PCIPHER_KEY) &pPort->SharedKey[BSS0][0];
        
    if (pWpaKey->KeyLen == 0)
    {
        return;
    }

    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)
    {
        i = 0;

        // Calculate MIC value
        RTMPInitMICEngine(
            pAd,
            pWpaKey->Key,
            pSrc + 4,
            pSrc + 10,
            0,
            pWpaKey->TxMic);

        PlatformMoveMemory(pDest, pSrc, LENGTH_802_11);
        pDest += LENGTH_802_11;
        pSrc += LENGTH_802_11;
        RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Tx, pSrc, (FrameLen - LENGTH_802_11));
        RTMPTkipGetMIC(&pAd->pNicCfg->PrivateInfo.Tx);

        RTMPInitTkipEngine(
            pAd,
            pWpaKey->Key,
            0,
            pAd->HwCfg.CurrentAddress,
            pWpaKey->TxMic,
            pWpaKey->TxTsc,
            &Iv16,
            &Iv32);

        pnl = (ULONG) *(pWpaKey->TxTsc) + (((ULONG) *(pWpaKey->TxTsc + 1)) << 8);   //TSC[0]:TSC[1]
        pnh = *(PULONG)(pWpaKey->TxTsc + 2);

        pAd->pNicCfg->PrivateInfo.FCSCRC32 = PPPINITFCS32;   //Init crc32.
        RTMPTkipMixKey(pWpaKey->Key, pAd->HwCfg.CurrentAddress, pnl, pnh, RC4Key, p1k);

        // Copy 8 bytes encapsulation into Tx ring
        PlatformMoveMemory(pDest, &Iv16, 4);
        pDest += 4;
        PlatformMoveMemory(pDest, &Iv32, 4);
        pDest += 4;

        // Init RC4 encyption engine
        ARCFOUR_INIT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, RC4Key, 16);

        // Increase TxTsc value for next transmission
        while (++pWpaKey->TxTsc[i] == 0x0)
        {
            i++;
            if (i == 6)
                break;
        }
            
        if (i == 6)
        {
            // TODO: TSC has done one full cycle, do re-keying stuff follow specs
            // Should send a special event microsoft defined to request re-key
        }
        
        RTMPEncryptData(pAd, pSrc, pDest, (FrameLen - LENGTH_802_11));
        pDest +=  (FrameLen - LENGTH_802_11);

        RTMPEncryptData(pAd, pAd->pNicCfg->PrivateInfo.Tx.MIC, pDest, 8);
        pDest +=  8;
        FrameLen += 8;
        RTMPSetICV(pAd, pDest);
        FrameLen += 12;

        NdisCommonMiniportMMRequest(pAd, OutBuffer, FrameLen);
    }
}
#endif //#if defined(NDIS620_MINIPORT)

/*
    ========================================================================
    
    Routine Description:
        SHA1 function 

    Arguments:
        
    Return Value:

    Note:
        
    ========================================================================
*/
VOID    HMAC_SHA1(
    IN  UCHAR   *text,
    IN  UINT    text_len,
    IN  UCHAR   *key,
    IN  UINT    key_len,
    IN  UCHAR   *digest)
{
    SHA_CTX context;
    UCHAR   k_ipad[65]; /* inner padding - key XORd with ipad   */
    UCHAR   k_opad[65]; /* outer padding - key XORd with opad   */
    INT     i;

    // if key is longer than 64 bytes reset it to key=SHA1(key) 
    if (key_len > 64) 
    {
        SHA_CTX      tctx;
        SHAInit(&tctx);
        SHAUpdate(&tctx, key, key_len);
        SHAFinal(&tctx, key);
        key_len = 20;
    }
    PlatformZeroMemory(k_ipad, sizeof(k_ipad));
    PlatformZeroMemory(k_opad, sizeof(k_opad));
    PlatformMoveMemory(k_ipad, key, key_len);
    PlatformMoveMemory(k_opad, key, key_len);

    // XOR key with ipad and opad values  
    for (i = 0; i < 64; i++) 
    {   
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    // perform inner SHA1 
    SHAInit(&context);                      /* init context for 1st pass */
    SHAUpdate(&context, k_ipad, 64);        /*  start with inner pad */
    SHAUpdate(&context, text, text_len);    /*  then text of datagram */
    SHAFinal(&context, digest);             /* finish up 1st pass */

    //perform outer SHA1  
    SHAInit(&context);                  /* init context for 2nd pass */
    SHAUpdate(&context, k_opad, 64);    /*  start with outer pad */
    SHAUpdate(&context, digest, 20);    /*  then results of 1st hash */
    SHAFinal(&context, digest);         /* finish up 2nd pass */
}

/*
    ========================================================================
    
    Routine Description:
        PRF function 

    Arguments:
        
    Return Value:

    Note:
        802.1i  Annex F.9
        
    ========================================================================
*/
VOID    PRF(
    IN  PMP_ADAPTER pAd,
    IN  UCHAR   *key,
    IN  INT     key_len,
    IN  UCHAR   *prefix,
    IN  INT     prefix_len,
    IN  UCHAR   *data,
    IN  INT     data_len,
    OUT UCHAR   *output,
    IN  INT     len)
{
    INT     i;
    PUCHAR  input = NULL;
    INT     currentindex = 0;
    INT     total_len;
    
    PlatformAllocateMemory(pAd, &input, 1024);
    if (input == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("PRF::Allocate memory size(=1024) failed\n"));
        return;
    }
    
    PlatformMoveMemory(input, prefix, prefix_len);
    input[prefix_len] = 0;
    PlatformMoveMemory(&input[prefix_len + 1], data, data_len);
    total_len = prefix_len + 1 + data_len;
    input[total_len] = 0;
    total_len++;
    for (i = 0; i < (len + 19) / 20; i++)
    {
        HMAC_SHA1(input, total_len, key, key_len, &output[currentindex]);
        currentindex += 20;
        input[total_len - 1]++;
    }   

    if (input)
        PlatformFreeMemory(input, 1024);
}

/*
    ========================================================================
    
    Routine Description:
        Cisco CCKM PRF function 

    Arguments:
        key             Cisco Base Transient Key (BTK)
        key_len         The key length of the BTK
        data            Ruquest Number(RN) + BSSID
        data_len        The length of the data
        output          Store for PTK(Pairwise transient keys)
        len             The length of the output
    Return Value:
        None

    Note:
        802.1i  Annex F.9
        
    ========================================================================
*/
VOID    CCKMPRF(
    IN  PMP_ADAPTER pAd,
    IN  UCHAR   *key,
    IN  INT     key_len,
    IN  UCHAR   *data,
    IN  INT     data_len,
    OUT UCHAR   *output,
    IN  INT     len)
{
    INT     i;
    PUCHAR  input = NULL;
    INT     currentindex = 0;
    INT     total_len;
    
    PlatformAllocateMemory(pAd, &input, 1024);
    if (input == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("CCKMPRF::Allocate memory size(=1024) failed\n"));
        return;
    }

    PlatformMoveMemory(input, data, data_len);
    total_len = data_len;
    input[total_len] = 0;
    total_len++;
    for (i = 0; i < (len + 19) / 20; i++)
    {
        HMAC_SHA1(input, total_len, key, key_len, &output[currentindex]);
        currentindex += 20;
        input[total_len - 1]++;
    }   

    if (input)
        PlatformFreeMemory(input, 1024);
}





/*
    ========================================================================
    
    Routine Description:
        Count TPTK from PMK

    Arguments:
        
    Return Value:
        Output      Store the TPTK

    Note:
        
    ========================================================================
*/
VOID WpaApWpaCountPTK(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    IN  UCHAR   *PMK,
    IN  UCHAR   *ANonce,
    IN  UCHAR   *AA,
    IN  UCHAR   *SNonce,
    IN  UCHAR   *SA,
    OUT UCHAR   *output,
    IN  UINT    len)
{   
    UCHAR   concatenation[76];
    UINT    CurrPos = 0;
    UCHAR   temp[32];
    UCHAR   Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ', 
                        'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};
    UCHAR   CCKMPrefix[] = {'F', 'a', 's', 't', '-', 'R', 'o', 'a', 'm', ' ', 'G', 'e', 'n', 
                        'e', 'r', 'a', 't', 'e', ' ', 'B', 'a', 's', 'e', ' ', 'K', 'e', 'y'};

    PlatformZeroMemory(temp, sizeof(temp));

    {
        // Get smaller address
        if (MtkCompareMemory(SA, AA, 6) == 1)
            PlatformMoveMemory(concatenation, AA, 6);
        else
            PlatformMoveMemory(concatenation, SA, 6);
        CurrPos += 6;

        // Get larger address
        if (MtkCompareMemory(SA, AA, 6) == 1)
            PlatformMoveMemory(&concatenation[CurrPos], SA, 6);
        else
            PlatformMoveMemory(&concatenation[CurrPos], AA, 6);
        CurrPos += 6;

        // Get smaller address
        if (MtkCompareMemory(ANonce, SNonce, 32) == 1)
            PlatformMoveMemory(&concatenation[CurrPos], SNonce, 32);
        else
            PlatformMoveMemory(&concatenation[CurrPos], ANonce, 32);
        CurrPos += 32;

        // Get larger address
        if (MtkCompareMemory(ANonce, SNonce, 32) == 1)
            PlatformMoveMemory(&concatenation[CurrPos], ANonce, 32);
        else
            PlatformMoveMemory(&concatenation[CurrPos], SNonce, 32);
        CurrPos += 32;
        
        PRF(pAd, PMK, LEN_MASTER_KEY, Prefix, 22, concatenation, 76, output, len);
    }
}

/*
    ========================================================================
    
    Routine Description:
        Misc function to Generate random number

    Arguments:
        
    Return Value:

    Note:
        802.1i  Annex F.9
        
    ========================================================================
*/
VOID    GenRandom(
    IN  PMP_ADAPTER   pAd, 
    OUT UCHAR           *random)
{   
    INT     i, curr;
    UCHAR   local[80], KeyCounter[32];
    UCHAR   result[80];
    ULONGLONG   CurrentTime;
    UCHAR   prefix[] = {'I', 'n', 'i', 't', ' ', 'C', 'o', 'u', 'n', 't', 'e', 'r'};

    PlatformZeroMemory(result, 80);
    PlatformZeroMemory(local, 80);
    PlatformZeroMemory(KeyCounter, 32);
    COPY_MAC_ADDR(local, pAd->HwCfg.CurrentAddress);
    
    for (i = 0; i < 32; i++)
    {       
        curr =  MAC_ADDR_LEN;
        NdisGetCurrentSystemTime((PLARGE_INTEGER)&CurrentTime);
        COPY_MAC_ADDR(local,  pAd->HwCfg.CurrentAddress);
        curr += MAC_ADDR_LEN;
        PlatformMoveMemory(&local[curr],  &CurrentTime, sizeof(CurrentTime));
        curr += sizeof(CurrentTime);
        PlatformMoveMemory(&local[curr],  result, 32);
        curr += 32;
        PlatformMoveMemory(&local[curr],  &i,  2);      
        curr += 2;
        PRF(pAd, KeyCounter, 32, prefix,12, local,  curr, result, 32); 
    }
    PlatformMoveMemory(random, result,  32);    
}

/*
    ========================================================================
    
    Routine Description:
        Misc function to decrypt AES body
    
    Arguments:
            
    Return Value:
    
    Note:
        This function references to RFC 3394 for aes key unwrap algorithm.
            
    ========================================================================
*/
VOID    AES_GTK_KEY_UNWRAP( 
    IN  UCHAR   *key,
    OUT UCHAR   *plaintext,
    IN  UCHAR   *ciphertext)
{
    UCHAR       A[8],   BIN[16], BOUT[16];
    UCHAR       R1[8],R2[8];
    UCHAR       xor;
    INT         num_blocks = 2;
    INT         j;
    aes_context aesctx;
    
    // Initialize
    // A = C[0]
    PlatformMoveMemory(A, ciphertext, 8);
    // R1 = C1
    PlatformMoveMemory(R1, &ciphertext[8], 8);
    // R2 = C2
    PlatformMoveMemory(R2, &ciphertext[16], 8);

    aes_set_key(&aesctx, key, 128);
    
    for (j = 5; j >= 0; j--)
    {
        xor = num_blocks * j + 2;
        PlatformMoveMemory(BIN, A, 8);
        BIN[7] = A[7] ^ xor;
        PlatformMoveMemory(&BIN[8], R2, 8);
        aes_decrypt(&aesctx, BIN, BOUT);
        PlatformMoveMemory(A, &BOUT[0], 8);
        PlatformMoveMemory(R2, &BOUT[8], 8);
        
        xor = num_blocks * j + 1;
        PlatformMoveMemory(BIN, A, 8);
        BIN[7] = A[7] ^ xor;
        PlatformMoveMemory(&BIN[8], R1, 8);
        aes_decrypt(&aesctx, BIN, BOUT);
        PlatformMoveMemory(A, &BOUT[0], 8);
        PlatformMoveMemory(R1, &BOUT[8], 8);
    }

    // OUTPUT
    PlatformMoveMemory(&plaintext[0], R1, 8);
    PlatformMoveMemory(&plaintext[8], R2, 8);
}

/*
    ========================================================================
Routine Description:
    HMAC using SHA256 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes   
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID HMAC_SHA256 (
    IN  const UCHAR Key[], 
    IN  UINT KeyLen, 
    IN  const UCHAR Message[], 
    IN  UINT MessageLen, 
    OUT UCHAR MAC[],
    IN  UINT MACLen)
{
    SHA256_CTX_STRUC sha_ctx1;
    SHA256_CTX_STRUC sha_ctx2;
    UINT8 K0[SHA256_BLOCK_SIZE];
    UINT8 Digest[SHA256_DIGEST_SIZE];
    UINT index;
 
    PlatformZeroMemory(&sha_ctx1, sizeof(SHA256_CTX_STRUC));
    PlatformZeroMemory(&sha_ctx2, sizeof(SHA256_CTX_STRUC));
    /*
     * If the length of K = B(Block size): K0 = K.
     * If the length of K > B: hash K to obtain an L byte string, 
     * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
     * If the length of K < B: append zeros to the end of K to create a B-byte string K0
     */
    PlatformZeroMemory(K0, SHA256_BLOCK_SIZE);
    if (KeyLen <= SHA256_BLOCK_SIZE) {
        PlatformMoveMemory(K0, (PVOID)Key, KeyLen);
    } else {
        SHA256(Key, KeyLen, K0);
    }

    /* Exclusive-Or K0 with ipad */
    /* ipad: Inner pad; the byte x¡¦36¡¦ repeated B times. */
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        /* End of for */
        
    SHA256_Init(&sha_ctx1);
    /* H(K0^ipad) */
    SHA256_Append(&sha_ctx1, K0, sizeof(K0));
    /* H((K0^ipad)||text) */
    SHA256_Append(&sha_ctx1, Message, MessageLen);  
    SHA256_End(&sha_ctx1, Digest);

    /* Exclusive-Or K0 with opad and remove ipad */
    /* opad: Outer pad; the byte x¡¦5c¡¦ repeated B times. */
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        /* End of for */
        
    SHA256_Init(&sha_ctx2);
    /* H(K0^opad) */
    SHA256_Append(&sha_ctx2, K0, sizeof(K0));
    /* H( (K0^opad) || H((K0^ipad)||text) ) */
    SHA256_Append(&sha_ctx2, Digest, SHA256_DIGEST_SIZE);
    SHA256_End(&sha_ctx2, Digest);

    if (MACLen > SHA256_DIGEST_SIZE)
        PlatformMoveMemory(MAC, Digest,SHA256_DIGEST_SIZE);
    else
        PlatformMoveMemory(MAC, Digest, MACLen);
    
} /* End of HMAC_SHA256 */
/*
    ========================================================================
    
    Routine Description:
        The key derivation function(KDF) for the FT key hierarchy.
        (IEEE 802.11r/D9.0, 8.5.1.5.2)

    Arguments:

    Return Value:

    Note:
        Output ¡ö KDF-Length (K, label, Context) where
        Input:    K, a 256-bit key derivation key
                  label, a string identifying the purpose of the keys derived using this KDF
                  Context, a bit string that provides context to identify the derived key
                  Length, the length of the derived key in bits
        Output: a Length-bit derived key

        result ¡ö ""
        iterations ¡ö (Length+255)/256 
        do i = 1 to iterations
            result ¡ö result || HMAC-SHA256(K, i || label || Context || Length)
        od
        return first Length bits of result, and securely delete all unused bits

        In this algorithm, i and Length are encoded as 16-bit unsigned integers.

    ========================================================================
*/
VOID    FT_KDF(
    IN  PUCHAR  key,
    IN  INT     key_len,
    IN  PUCHAR  label,
    IN  INT     label_len,
    IN  PUCHAR  data,
    IN  UINT        data_len,
    OUT PUCHAR  output,
    IN  UINT        len)
{
    USHORT  i;
    UCHAR   input[1024];
    INT     currentindex = 0;
    INT     total_len;
    UINT    len_in_bits = (len << 3);

    PlatformZeroMemory(input, 1024);
    
    // Initial concatenated value (i || label || Context || Length)
    // concatenate 16-bit unsigned integer, its initial value is 1. 
    input[0] = 1;   // Todo AlbertY : start at 0 or 1 ?
    input[1] = 0;   
    total_len = 2;

    // concatenate a prefix string
    PlatformMoveMemory(&input[total_len], label, label_len);
    total_len += label_len;

    // concatenate the context
    PlatformMoveMemory(&input[total_len], data, data_len);
    total_len += data_len;

    // concatenate the length in bits (16-bit unsigned integer)
    input[total_len] = (len_in_bits & 0xFF);
    input[total_len + 1] = (len_in_bits & 0xFF00) >> 8; 
    total_len += 2;
     
    for (i = 1; i <= ((len_in_bits + 255) / 256); i++)
    {
        // HMAC-SHA256 derives output 
        HMAC_SHA256((UCHAR *)key, key_len, input, total_len, (UCHAR *)&output[currentindex], 32);       

        currentindex += 32; // next concatenation location
        input[0]++;         // increment octet count

    }           
}

