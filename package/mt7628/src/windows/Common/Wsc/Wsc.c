/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
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
    Paul Lin    06-08-08        Initial
    Rita Yang   07-04-19        Add Registrar
*/
#include "MtConfig.h"
#pragma warning(disable:6387)
#pragma warning(disable:6011)
#pragma warning(disable:28196)
#include <ntstrsafe.h>
#pragma warning(default:6387)
#pragma warning(default:6011)
#pragma warning(default:28196)

#define DRIVER_FILE         0x01500000

// General used field
// The last 6 bytes of UUID shall changed to MAC address
UCHAR   Wsc_Uuid[16] = {0x52, 0x61, 0x6c, 0x69, 0x6e, 0x6b, 0x57, 0x50, 0x53, 0x2d, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
UCHAR   Wsc_Manufacture[] = "Ralink Technology, Corp.";
UCHAR   Wsc_Manufacture_Len = (sizeof(Wsc_Manufacture) / sizeof(UCHAR));
UCHAR   Wsc_Model_Name[] = "Ralink Wireless Adapter";
UCHAR   Wsc_Model_Name_Len = (sizeof(Wsc_Model_Name) / sizeof(UCHAR));
UCHAR   Wsc_Model_Number[] = MODEL_NUMBER;
UCHAR   Wsc_Model_Number_Len = (sizeof(Wsc_Model_Number) / sizeof(UCHAR));
UCHAR   Wsc_Model_Serial[] = "12345678";
UCHAR   Wsc_Model_Serial_Len = (sizeof(Wsc_Model_Serial) / sizeof(UCHAR));
UCHAR   Wsc_Pri_Dev_Type[8] = {0x00, 0x01, 0x00, 0x50, 0xf2, 0x04, 0x00, 0x01};
//UCHAR Wsc_Pri_Dev_Type[8] = {0x00, 0x08, 0x00, 0x50, 0xf2, 0x04, 0x00, 0x05}; // media type
//UCHAR Wsc_Dev_Name_E[] = "Ralink Client"; // default to Registry
UCHAR   Wsc_Dev_Name_R[] = "Ralink EX-Registrar";
UCHAR   Wsc_Personal_String[] =  "Wi-Fi Easy and Secure Key Derivation";
UCHAR   EnrolleeMsg[] = "WFA-SimpleConfig-Enrollee-1-0";
UCHAR   RegistrarMsg[] = "WFA-SimpleConfig-Registrar-1-0";
UCHAR   Wsc_SMI_Code[3] = {0x00, 0x37, 0x2A};

// Global reusable buffer
UCHAR   OutMsgBuf[3000];        // buffer to create message contents
UCHAR   HmacData[4096];         // temp buffer to store encrypt output data
UCHAR   PlainText[1500];        // buffer to create plain data
UCHAR   CipherText[1500];       // buffer to create encrypted data
UCHAR   MFBuf[700];
PUCHAR  pFragData;
INT     FragRemainLen;

//
// The WPS attributes of the T Home Speedport W 303V WPS AP.
//

#define NULL_CHARACTER_LENGTH 1 // Null character ('\0').

// The model name (an ASCII string).
UCHAR THOME_303V_MODEL_NAME[] = "ARV4525PW";
#define THOME_303V_MODEL_NAME_LENGTH    ((sizeof (THOME_303V_MODEL_NAME) / sizeof(UCHAR)) - NULL_CHARACTER_LENGTH)

// The model number (an ASCII string).
UCHAR THOME_303V_MODEL_NUMBER_000[] = "1.04.000";
#define THOME_303V_MODEL_NUMBER_LENGTH_000  ((sizeof (THOME_303V_MODEL_NUMBER_000) / sizeof(UCHAR)) - NULL_CHARACTER_LENGTH)

UCHAR THOME_303V_MODEL_NUMBER_001[] = "1.04.001";
#define THOME_303V_MODEL_NUMBER_LENGTH_001  ((sizeof (THOME_303V_MODEL_NUMBER_001) / sizeof(UCHAR)) - NULL_CHARACTER_LENGTH)

/*  
    ==========================================================================
    Description: 
        association state machine init, including state transition and timer init
    Parameters: 
        S - pointer to the association state machine
    ==========================================================================
 */
VOID WscStateMachineInit(
    IN  PMP_ADAPTER       pAd, 
    IN  STATE_MACHINE       *S, 
    OUT STATE_MACHINE_FUNC  Trans[])    
{
    StateMachineInit(S, Trans, MAX_WSC_STATE, MAX_WSC_MSG, Drop, WSC_IDLE, WSC_MACHINE_BASE);
    StateMachineSetAction(S, WSC_IDLE, WSC_EAP_REQ_MSG, WscEAPAction);
    StateMachineSetAction(S, WSC_IDLE, WSC_EAP_RSP_MSG, WscEAPAction);
    StateMachineSetAction(S, WSC_IDLE, WSC_EAP_FAIL_MSG, WscEAPAction);
}

/*
    ==========================================================================
    Description:
        This is state machine function when receiving EAP packets 
        which is WPS Registration Protocol.

        There are two roles at our Station, as an 
        1. Enrollee     
        2. External Registrar

        Running Scenarios:
        -----------------------------------------------------------------
        1a. Adding an AP as an Enrollee to a station as an External Registrar (EAP)
            [External Registrar]<----EAP--->[Enrollee_AP]
        -----------------------------------------------------------------
        2a. Adding a station as an Enrollee to an AP with built-in Registrar (EAP)  
            [Registrar_AP]<----EAP--->[Enrollee_STA]
        -----------------------------------------------------------------
        3a. Adding an Enrollee with External Registrar (UPnP/EAP)   
            [External Registrar]<----UPnP--->[Proxy_AP]<---EAP--->[Enrollee_STA]  
        -----------------------------------------------------------------

    Return:
        None
    ==========================================================================
*/
VOID WscEAPAction(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   MsgType;
    UCHAR   Id;
    BOOLEAN Cancelled = FALSE;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];

    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscEAPAction\n"));

    // Receive WSC EAP packet, cancel EAP Rx timer
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);

    // Check the packet is MF or not.
    switch (WscMfNeedBuf(pAd, pPort, &Id, Elem))
    {
        case 1:
            //No Fragmented EAP packet
            DBGPRINT(RT_DEBUG_TRACE, ("%s No Fragmented packet!!\n", __FUNCTION__));
            break;
            
        case 2:
            //Collect all Fragmented EAP packets to MFBuf, use MFBuf as the WSC message to progress.
            Elem->MsgLen = pPort->StaCfg.WscControl.MFBufLen;
            PlatformZeroMemory(Elem->Msg, MGMT_DMA_BUFFER_SIZE);
            PlatformMoveMemory(Elem->Msg, MFBuf, Elem->MsgLen);
            DBGPRINT(RT_DEBUG_TRACE, ("WscEAPAction - Use MFBuf as the WSC message to progress. Length=%d\n", Elem->MsgLen));
            break;
            
        case 3:
            //Need Buffer the packets, collecting the fragmented EAP packets.
            DBGPRINT(RT_DEBUG_TRACE, ("WscEAPAction - Need Buffer Fragmented WSC EAP!! Send WSC Frag ACK. Id:%d\n", Id));
            WscSendEapFragAck(pAd, pPort, Id, Elem);
            return;

        default:
            return;
    }

    // 0. Start to process EAP message, finding the exact message type first.
    MsgType = WscRxMsgType(pAd, pPort, &Id, Elem);
    DBGPRINT(RT_DEBUG_TRACE, ("WscEAPAction: Received MsgType= 0x%02x\n", MsgType));

    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
    {
        WscEAPEnrolleeAction(pAd, pPort, MsgType, Id, Elem);
        PlatformZeroMemory(MFBuf, 700);
    }
    else if ((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR) ||
        (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP))
    {
        WscEAPRegistrarAction(pAd, pPort, MsgType, Id, Elem,  NULL, NULL);
        PlatformZeroMemory(MFBuf, 700);
    }
    else
        return;
        
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscEAPAction\n"));
}

/*
    ============================================================================
    Enrollee            Enrollee            Enrollee    
    ============================================================================    
*/
VOID    WscEAPEnrolleeAction(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  UCHAR               MsgType,
    IN  UCHAR               Id,
    IN  MLME_QUEUE_ELEM     *Elem)
{
    BOOLEAN         Cancelled = FALSE;
    PHEADER_802_11      pHeader_802_11;
    ULONG           Idx;
        
    //DBGPRINT(RT_DEBUG_TRACE, ("-----> WscEAPEnrolleeAction\n"));

    // Base on state doing the Msg, State change diagram
    switch (MsgType)
    {   
        case WSC_MSG_EAP_REQ_ID:
            // This is only valid at WSC_STATE_LINKUP
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_LINK_UP)
            {
                // call to process EAP-Req and send out EAP-Rsp
                WscSendEapRspId(pAd, pPort, Id, Elem);
                // Change the state to next one
                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_START;
            }
            // resend the rsp-id
            else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_START)
            {
                // call to process EAP-Req and send out EAP-Rsp
                WscSendEapRspId(pAd, pPort, Id, Elem);
                // Change the state to next one
                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_START;
            }

            // EAP Rx processed, set new EAP Rx timer
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
            
            break;

        case WSC_MSG_EAP_REQ_START:
            // This is only valid at WSC_STATE_WAIT_START
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_START)
            {
                // Send out M1
                WscSendMessageM1(pAd, pPort, Id, Elem);
                // Change the state to next one
                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M2;

                // Set a new timer more than 5 sec for M2. Protect against multiple M2D received
                // Once re-associate , this will be canceled
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
                PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapM2Timer, WSC_PER_MSG_TIMEOUT);
            }
            // We have to resend M1 if we receive WSC start instead of M2 or M2D
            else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M2)
            {
                // Send out M1
                WscSendMessageM1(pAd, pPort, Id, Elem);
                // Change the state to next one
                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M2;

                // Set a new timer more than 5 sec for M2. Protect against multiple M2D received
                // Once re-associate , this will be canceled
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
                PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapM2Timer, WSC_PER_MSG_TIMEOUT);
            }
            break;
    
        case WSC_MSG_M2:
            // Cancel waiting M2 timer first
            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
            // Receive M2, if we are at WSC_STATE_WAIT_M2 state, process it immediately
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M2)
            {
                // Process M2
                if (WscRecvMessageM2(pAd, pPort, Id, Elem))    // succeed
                {
                    // Send out M3
                    WscSendMessageM3(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M4;
                }
                else    // fail
                    goto err;
            }
            // We have to resend M3 if we receive M2 instead of M4
            else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M4)
            {
                // It's a retry. Use lasttx2 instead of lasttx  
                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Length = 
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length;
                PlatformMoveMemory(pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Data, 
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Data,
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length);
                
                // Process M2
                if (WscRecvMessageM2(pAd, pPort, Id, Elem))    // succeed
                {
                    // Send out M3
                    WscSendMessageM3(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M4;
                }
                else    // fail
                    goto err;
            }
            // Received delayed M2 afetr receiving M2D
            else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_RX_M2D)
            {
                // Process M2
                if (WscRecvMessageM2(pAd, pPort, Id, Elem))    // succeed
                {
                    // Send out M3
                    WscSendMessageM3(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M4;
                }
                else    // fail
                    goto err;       
            }
            
            // EAP Rx processed, set new EAP Rx timer
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);

            break;
            
        case WSC_MSG_M2D:
            // Receive M2D, if we are at WSC_STATE_WAIT_M2 state, process it immediately
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M2)
            {
                // Process M2D
                // Check our own ENonce. If mismatch, don't send Ack

                Idx = BssTableSearch(pAd, pPort, &pAd->ScanTab, pPort->StaCfg.WscControl.WscAPBssid, pPort->ScaningChannel);
                if (WscRecvMessageM2D(pAd, pPort, Id, Elem))
                {
                    // Send out EAP-Rsp-Ack
                    WscSendEapRspAck(pAd, pPort, Id, Elem);

                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_RX_M2D;    
                    
                    if(IS_P2P_ENROLLEE(pPort))
                    {
                        pPort->P2PCfg.P2pCounter.CounterAfterSetWscMode = 0;
                        PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
                        DBGPRINT(RT_DEBUG_TRACE, ("Is ENROLLEE not PlatformCancelTimer"));
                    }
                    else if(pAd->ScanTab.BssEntry[Idx].P2PDevice) // sta want to connect P2P GO
                    {
                        PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
                        DBGPRINT(RT_DEBUG_TRACE, ("not ENROLLEE PlatformCancelTimer"));
                    }
                }
            }
            // Multiple M2D
            else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_RX_M2D)
            {
                // Process M2D
                // Check our own ENonce. If mismatch, don't send Ack
                if (WscRecvMessageM2D(pAd, pPort, Id, Elem))
                {
                    // Send out EAP-Rsp-Ack
                    WscSendEapRspAck(pAd, pPort, Id, Elem);
                }
            }
            break;

        case WSC_MSG_M4:
            // Receive M4, if we are at WSC_STATE_WAIT_M4 state, process it immediately
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M4)
            {
                // Process M4
                if (WscRecvMessageM4(pAd, pPort, Id, Elem))    // succeed
                {
                    // Send out M5
                    WscSendMessageM5(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M6;
                }
                else    // fail
                    goto err;       
            }
            // We have to resend M5 if we receive M4 instead of M6
            else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M6)
            {
                // It's a retry. Use lasttx2 instead of lasttx  
                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Length = 
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length;
                PlatformMoveMemory(pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Data, 
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Data,
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length);
                
                // Process M4
                if (WscRecvMessageM4(pAd, pPort, Id, Elem))    // succeed
                {
                    // Send out M5
                    WscSendMessageM5(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M6;
                }
                else    // fail
                    goto err;
            }

            // EAP Rx processed, set new EAP Rx timer
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
            
            break;

        case WSC_MSG_M6:
            // Receive M6, if we are at WSC_STATE_WAIT_M6 state, process it immediately
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M6)
            {
                // Process M6
                if (WscRecvMessageM6(pAd, pPort, Id, Elem))    // succeed
                {            
                    // Send out M7
                    WscSendMessageM7(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M8;
                }
                else    // fail
                    goto err;
            }
            // We have to resend M7 if we receive M6 instead of M8
            else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M8)
            {
                // It's a retry. Use lasttx2 instead of lasttx  
                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Length = 
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length;
                PlatformMoveMemory(pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Data, 
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Data,
                            pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length);
                
                // Process M6
                if (WscRecvMessageM6(pAd, pPort, Id, Elem))    // succeed
                {
                    // Send out M7
                    WscSendMessageM7(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M8;
                }
                else    // fail
                    goto err;
            }

            // EAP Rx processed, set new EAP Rx timer
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
            
            break;

        case WSC_MSG_M8:
            // Receive M8, if we are at WSC_STATE_WAIT_M6 state, process it immediately
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M8)
            {
                // Process M8
                if (WscRecvMessageM8(pAd, pPort, Id, Elem))    // succeed
                {
                    // Send out EAP-Rsp-Done
                    WscSendEapRspDone(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_DISCONN;

                }
                else// fail
                    goto err;
            }
            
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_DISCONN)
            {
                // Set a new timer. timeout = Per Message Timeout on AP (15 sec)
                // Regardless of Eap-Fail, this will be configured after timeout.
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
                PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, 15000);
            }
            break;

        case WSC_MSG_EAP_FAIL:
            // Receive Eap-Fail at WSC_STATE_WAIT_DISCONN, process it and go to WSC_STATE_CONFIGURED
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_DISCONN)
            {
                
                // Backup the AP's BSSID for the UI.
                // Note that the STA supports only one profile.
                PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr,
                    pPort->StaCfg.WscControl.WscAPBssid, MAC_ADDRESS_LENGTH);
                
                // Receive Eap-Fail, cancel timer
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
                
                // call to process Eap-Fail
                WscReceiveEapFail(pAd, Id, Elem);
                
                // Update WSC status
                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;  
                
                pPort->StaCfg.WscControl.bWPSMODE9SKIPProgress = TRUE;
                
                // Change the state to next one
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;

                // Reset the scanning counter.
                pAd->StaCfg.ScanCnt = 0;

                // The protocol is finished.
                pPort->StaCfg.WscControl.bWPSLEDSetTimer = TRUE;
                LedCtrlSetBlinkMode(pAd, LED_WPS_SUCCESS);

                pHeader_802_11 = (PHEADER_802_11)&Elem->Msg[0];
                if (IS_P2P_ENROLLEE(pPort))
                {
                    P2pWpsDone(pAd, pHeader_802_11->Addr2, pPort);
                }

            }
            // Received unexpected EAP-FAIL and expected EAP-FAIL at the state of M2D, restart the association
            else if (pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_EAPFAIL)
            {
                // call to process Eap-Fail
                WscReceiveEapFail(pAd, Id, Elem);

                // re-associate immediately and change to state WSC_STATE_START
                PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, 0);
                
            }
            // Receive Eap-Fail, it indicate the EAP failed, we have to set teh correct state and staus
            else
            {
                // call to process Eap-Fail
                WscReceiveEapFail(pAd, Id, Elem);
                
                // Update WSC status
                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_FAILED;  
                DBGPRINT(RT_DEBUG_TRACE, ("!! 1 STATUS_WSC_EAP_FAILED\n"));         

                // Change the state to next one
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("WscEAPEnrolleeAction : Rx EAP-FAIL\n"));         
            break;

        case WSC_MSG_WSC_NACK:
            // Receive EAP-Req-Nack at entering lock-step (M3~Done)
            if ((pPort->StaCfg.WscControl.WscState >= WSC_STATE_WAIT_M4) && 
                (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
                WscReceiveEapNack(pAd, pPort, Id, Elem);
            
            //Send EAP Response NACK after we receive EAP NACK.
            WscSendEapRspNack(pAd, pPort, Id, Elem);
            break;

        case WSC_MSG_FRAG_ACK:
            // Receive WSC-Frag ACk
            if((IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd))&&(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
            {
                WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, Elem, FALSE);
            }   
            break;

        default:
            DBGPRINT(RT_DEBUG_TRACE, ("WscEAPEnrolleeAction : Unsupported Msg Type\n"));       
            break;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("----> WscEAPEnrolleeAction. WscState = %x. WscStatus = %x\n", pPort->StaCfg.WscControl.WscState, pPort->StaCfg.WscControl.WscStatus));
    
    //DBGPRINT(RT_DEBUG_TRACE, ("<----- WscEAPEnrolleeAction\n"));
    return;

err:
    // TODO:  Might need to send out a Nack and change the state to wait for an EAP-FAIL
    if (pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_EAPFAIL)
    {
        if(pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_PASSWORD_MISMATCH)
        {
            // set config error 18
            pPort->StaCfg.WscControl.RegData.EnrolleeInfo.ConfigError = cpu2be16(WSC_ERROR_DEV_PWD_AUTH_FAIL);
        }
        else if (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_M8_PROFILE_MAC_ERR)
        {
            // set config error 13
            pPort->StaCfg.WscControl.RegData.EnrolleeInfo.ConfigError = cpu2be16(WSC_ERROR_ROGUE_SUSPECTED);
        }
        WscSendEapRspNack(pAd, pPort, Id, Elem);
        // Change the state to next one
        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_EAPFAIL;
    }

    // For a invalid nonce, invalid authenticator and invalid length:
    // sliently ignore this message and stay at the same state
    
    // EAP Rx processed, set new EAP Rx timer
    if ((pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_EAPFAIL) && 
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_DISCONN))
        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);

    DBGPRINT(RT_DEBUG_TRACE, (" Error handling <----- WscEAPEnrolleeAction\n"));
    return;

}

/*
    ============================================================================
    Registrar           Registrar           Registrar           
    ============================================================================    
*/  
VOID    WscEAPRegistrarAction(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  UCHAR               MsgType,
    IN  UCHAR               Id,
    IN  MLME_QUEUE_ELEM     *Elem,
    IN  PWSC_EXTREG_MSG     ExtRegMsg,
    IN  PUCHAR              UserSelEnrMAC)
{
    BOOLEAN         Cancelled;
    PHEADER_802_11  pHeader_802_11;
    UCHAR NoPinCode4[4] = {0, 0, 0, 0};
    UCHAR NoPinCode8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    UCHAR index = P2P_NOT_FOUND;
    
    //DBGPRINT(RT_DEBUG_TRACE, ("-----> WscEAPRegistrarAction\n"));

    // Base on state doing the Msg, State change diagram
    switch (MsgType)
    {   
        case WSC_MSG_EAP_OL_START: //To support P2P
            // P2P GO AP Registart role need to support receiving EAPOL Start Frame.
            WscSendEapReqId(pAd, pPort, Id, Elem);
            // EAP Rx processed, set new EAP Rx timer
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer,
                        WSC_PER_MSG_TIMEOUT);
            break;
        case WSC_MSG_EAP_RSP_ID:
            {
                if (IS_P2P_REGISTRA(pPort))
                {
                    if (pPort->StaCfg.WscControl.WscState == WSC_STATE_LINK_UP)
                    {
                        // call to send out EAP-Request with WSC-Start.
                        WsceSendEapReqWscStart(pAd, pPort, Id, Elem);
                        // Change the state to next one
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                    }
                    // resend the rsp-id
                    else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M1)
                    {
                        // call to send out EAP-Request with WSC-Start.
                        WsceSendEapReqWscStart(pAd, pPort, Id, Elem);
                        // Change the state to next one
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                    }
            
                    // EAP Rx processed, set new EAP Rx timer
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer,
                                WSC_PER_MSG_TIMEOUT);
                }
            }
            break;
        case WSC_MSG_EAP_REQ_ID:
            {
                // This is only valid at WSC_STATE_LINKUP
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_LINK_UP)
                {
                    // call to process EAP-Req and send out EAP-Rsp
                    WscSendEapRspId(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                }
                // resend the rsp-id
                else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M1)
                {
                    // call to process EAP-Req and send out EAP-Rsp
                    WscSendEapRspId(pAd, pPort, Id, Elem);
                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                }

                // EAP Rx processed, set new EAP Rx timer
                PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
            }
            break;

        case WSC_MSG_M1:
            {
                // Receive M1, if we are at WSC_STATE_WAIT_M1 state, process it immediately
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M1)
                {
                    // Process M1
                    if (WscRecvMessageM1(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        if((pPort->StaCfg.WscControl.UseUserSelectEnrollee == 1) && (UserSelEnrMAC != NULL) &&
                            (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)&&
                            (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE)&&
                            (!PlatformEqualMemory(UserSelEnrMAC, pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr, 6)))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("Send M2D, User Selected Enrollee MAC is not matched with the MAC in M1\n"));
                            WscSendMessageM2D(pAd, pPort, Id, Elem);
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                        }
                        else if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)&&
                            (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE))
                        {
                            UCHAR   CntPBCEnr = 0;
                            CntPBCEnr = WscPbcEnrTabCheck(pAd, pPort, pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr);

                            if(pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EXTREG_MULTI_PBC_SESSIONS_DETECT)
                            {
                                //Send M2D with error code 12(detect too many devices)
                                DBGPRINT(RT_DEBUG_TRACE, ("Send M2D with error code 12 (Multiple PBC seesions detected!)\n"));
                                WscSendMessageM2D(pAd, pPort, Id, Elem);
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                            }
                            else if(CntPBCEnr > 1)
                            {
                                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_PBC_TOO_MANY_ENROLLEE;
                                //Send NACK since overlap
                                DBGPRINT(RT_DEBUG_TRACE, ("Send EAP-NACK since too many enrollee PBC\n"));
                                WscSendEapRspNack(pAd, pPort, Id, Elem);
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                            }
                            else if((pPort->P2PCfg.P2PConnectState == P2P_WPS_REGISTRA))
                            {
                                pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigError = WSC_ERROR_SETUP_LOCKED;
                                WscSendMessageM2D(pAd, pPort, Id, Elem);
                                WpseSendEapFail(pAd, pPort, Id, Elem);
                                
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                            }
                            else
                            {
                                // Send out M2
                                WscSendMessageM2(pAd, pPort, Id, Elem);
                                // Change the state to next one
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M3;
                            }
                            
                        }
                        else if(P2P_ON(pPort) && 
                              (IS_P2P_GO_OP(pPort) || IS_P2P_AUTOGO(pPort) ||(pPort->P2PCfg.P2PConnectState == P2P_WPS_REGISTRA)) &&
                              (pPort->StaCfg.WscControl.RegData.PINLen == 0))
                        {
                            if((pPort->StaCfg.WscControl.RegData.RegistrarInfo.DevPwdId == cpu2be16(DEV_PASS_ID_PIN)) &&
                               (pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DevPwdId == cpu2be16(DEV_PASS_ID_PIN)) )
                            {
                                    DBGPRINT(RT_DEBUG_TRACE, ("Empty PIN code, Wait AutoGo enter PIN code.\n"));

                                    DBGPRINT(RT_DEBUG_TRACE, ("DevPwdId = %x \n", pPort->StaCfg.WscControl.RegData.RegistrarInfo.DevPwdId));

                                    pHeader_802_11 = (PHEADER_802_11)&Elem->Msg[0];
                                
                                    pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigError = cpu2be16(WSC_ERROR_NO_ERROR);

                                    PlatformMoveMemory(&pPort->StaCfg.WscControl.M2ElemForP2P, Elem, sizeof(MLME_QUEUE_ELEM));
                                    pPort->StaCfg.WscControl.M2IdForP2P = Id;
                                    WscSendMessageM2D(pAd, pPort, Id, Elem);
                                    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M3;  // wait AutoGo enter PIN code and send M2 to enrolle, then enrolle will send M3 to continous.

                                    PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.GroupId, pHeader_802_11->Addr2, 6);
                                    index = P2pGroupTabSearch(pAd,  pHeader_802_11->Addr2);
                                    pPort->P2PCfg.PopUpIndex = index;
                                    pAd->pP2pCtrll->P2PTable.Client[index].ConfigMethod = CONFIG_METHOD_DISPLAY;
                                    P2pSetEvent(pAd, pPort,  P2PEVENT_CONFIRM_ENTER_PIN_FROM_WPS_M2);
                            }
                            else if((pPort->P2PCfg.P2PConnectState == P2P_WPS_REGISTRA))
                            {
                                
                                pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigError = WSC_ERROR_SETUP_LOCKED;
                                WscSendMessageM2D(pAd, pPort, Id, Elem);
                                WpseSendEapFail(pAd, pPort, Id, Elem);
                                
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                            }
                            
                        }
                        else
                        {
                            // Send out M2
                            WscSendMessageM2(pAd, pPort, Id, Elem);
                            // Change the state to next one
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M3;
                        }
                    }
                    else    // fail
                        goto err;
                }
                // We have to resend M2 if we receive M1 instead of M3
                else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M3)
                {
                    // Process M1
                    if (WscRecvMessageM1(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        if(pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EXTREG_PBC_TOO_MANY_ENROLLEE)
                        {
                            //Send M2D with error code 12(detect too many devices)
                            WscSendMessageM2D(pAd, pPort, Id, Elem);
                        }
                        else
                        {
                            // Send out M2
                            WscSendMessageM2(pAd, pPort, Id, Elem);
                            // Change the state to next one
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M3;
                        }
                    }
                    else    // fail
                        goto err;
                }
                else if(pPort->StaCfg.WscControl.WscState < WSC_STATE_WAIT_M1)
                {
                    // Process M1
                    if (WscRecvMessageM1(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Send M2D because not ready to receive M1\n"));
                        WscSendMessageM2D(pAd, pPort, Id, Elem);
                    }
                }
                else if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP) && 
                    (pPort->StaCfg.WscControl.WscState > WSC_STATE_WAIT_M5))
                {
                    // Process M1
                    if (WscRecvMessageM1(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Send M2 When wsc state > wait_M5 after receiving M1\n"));
                        WscSendMessageM2(pAd, pPort, Id, Elem);
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M3;
                    }
                }
                // EAP Rx processed, set new EAP Rx timer
                if(!(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP))
                {
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
                }
            }
            break;
        
        case WSC_MSG_M3:
            {
                // Receive M3, if we are at WSC_STATE_WAIT_M3 state, process it immediately
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M3)
                {
                    // Process M3
                    if (WscRecvMessageM3(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        // Send out M4
                        WscSendMessageM4(pAd, pPort, Id, Elem);
                        // Change the state to next one
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M5;
                    }
                    else    // fail
                        goto err;               
                }
                // We have to resend M4 if we receive M3 instead of M5
                else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M5)
                {
                    // It's a retry. Use lasttx2 instead of lasttx  
                    pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Length = 
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length;
                    PlatformMoveMemory(pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Data, 
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Data,
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length);

                    // Process M3
                    if (WscRecvMessageM3(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        // Send out M4
                        WscSendMessageM4(pAd, pPort, Id, Elem);
                        // Change the state to next one
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M5;
                    }
                    else    // fail
                        goto err;
                }

                // EAP Rx processed, set new EAP Rx timer
                if(!(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP))
                {
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
                }
            }
            break;

        case WSC_MSG_M5:
            {
                // Receive M5, if we are at WSC_STATE_WAIT_M4 state, process it immediately
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M5)
                {
                    // Process M5
                    if (WscRecvMessageM5(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        // Send out M6
                        WscSendMessageM6(pAd, pPort, Id, Elem);
                        // Change the state to next one
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M7;
                    }
                    else    // fail
                    {
                        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_M6_FAIL;
                        goto err;
                    }
                }
                // We have to resend M6 if we receive M5 instead of M7
                else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M7)
                {
                    // It's a retry. Use lasttx2 instead of lasttx  
                    pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Length = 
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length;
                    PlatformMoveMemory(pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Data, 
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Data,
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length);
            
                    // Process M5
                    if (WscRecvMessageM5(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        // Send out M6
                        WscSendMessageM6(pAd, pPort, Id, Elem);
                        // Change the state to next one
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M7;
                    }
                    else    // fail
                        goto err;
                }

                // EAP Rx processed, set new EAP Rx timer
                if(!(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP))
                {
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
                }
            }
            break;

        case WSC_MSG_M7:
            {
                // Receive M7, if we are at WSC_STATE_WAIT_M7 state, process it immediately
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_M7)
                {
                    // Process M7
                    if (WscRecvMessageM7(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        BOOLEAN     bSendM8 = TRUE;

                        if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
                        {
                            INT i;
                            PBSS_ENTRY  pInBss;
                            BOOLEAN APSetupLocked = FALSE;
                            
                            for (i = 0; i < pAd->ScanTab.BssNr; i++) 
                            {
                                pInBss  = (PBSS_ENTRY) &pAd->ScanTab.BssEntry[i];  // BSS entry for VarIE processing
                                if (MAC_ADDR_EQUAL(pInBss->Bssid, 
                                    pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr))
                                {
                                    APSetupLocked = pInBss->WSCAPSetupLocked;
                                }
                            }
                            
                            if((pAd->StaCfg.WSCVersion2 < 0x20) || 
                                (pPort->StaCfg.WscControl.RegData.EnrolleeInfo.Version2 < 0x20))
                            {
                                if(pPort->StaCfg.WscControl.bConfiguredAP == TRUE)
                                    bSendM8 = FALSE;
                                else
                                    bSendM8 = TRUE;
                            }
                            else if((pAd->StaCfg.WSCVersion2 >= 0x20) &&
                                (pPort->StaCfg.WscControl.RegData.EnrolleeInfo.Version2 >= 0x20))
                            {
                                if(pPort->StaCfg.WscControl.bConfiguredAP == TRUE)
                                {
                                    if(APSetupLocked)
                                        bSendM8 = FALSE;
                                    else
                                    {
                                        if(pPort->StaCfg.WscControl.ReconfigAPSelbyUser == 1)
                                            bSendM8 = TRUE;
                                        else
                                            bSendM8 = FALSE;
                                    }
                                }
                                else if(APSetupLocked == FALSE)
                                    bSendM8 = TRUE;
                                else
                                    bSendM8 = FALSE;
                            }
                            
                            DBGPRINT(RT_DEBUG_TRACE, (" AP'sVersion:%d, APSetupLocked:%d, ReconfigAPSelbyUser:%d, bSendM8:%d\n", 
                                pPort->StaCfg.WscControl.RegData.EnrolleeInfo.Version2, 
                                APSetupLocked, pPort->StaCfg.WscControl.ReconfigAPSelbyUser, bSendM8));
                        }
                        
                        
                        //if ((pPort->StaCfg.WscControl.bConfiguredAP == TRUE)&&(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR))
                        if( !bSendM8 )
                        {
                            
                            //
                            // Force to send NACK.
                            //
                            WscSendEapRspNack(pAd, pPort, Id, Elem);

                            //
                            // Save the profile.
                            //
                            // The peer is a configured AP.
                            if (pPort->StaCfg.WscControl.bConfiguredAP == TRUE &&
                                pPort->StaCfg.WscControl.WscM7Profile.ProfileCnt == 1)
                            {
                                // Copy M7 profile to M8
                                PlatformZeroMemory(&pPort->StaCfg.WscControl.WscProfile, sizeof(WSC_PROFILE));
                                PlatformMoveMemory( &pPort->StaCfg.WscControl.WscProfile.Profile[0], 
                                                &pPort->StaCfg.WscControl.WscM7Profile.Profile[0], 
                                                sizeof(WSC_CREDENTIAL));
                                pPort->StaCfg.WscControl.WscProfile.ProfileCnt =
                                        pPort->StaCfg.WscControl.WscM7Profile.ProfileCnt;
                            }   

                            // Backup the AP's BSSID for the UI.
                            // Note that the STA supports only one profile.
                            // The Bssid of provision must be enrollee's MAC.
                            if(pPort->StaCfg.WscControl.WscState == WSC_ConfMode_REGISTRAR_UPNP)
                            {
                                PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr,
                                    pPort->StaCfg.WscControl.ExtRegEnrMAC, MAC_ADDRESS_LENGTH);
                                DBGPRINT(RT_DEBUG_TRACE, ("Profile0MAC:%x %x %x\n", 
                                    pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[3],
                                    pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[4],
                                    pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[5]));
                            }
                            else
                            {

                                PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr,
                                    pPort->StaCfg.WscControl.WscAPBssid, MAC_ADDRESS_LENGTH);
                            }

                            // Reset the scanning counter.
                            pAd->StaCfg.ScanCnt = 0;

                            //
                            // Set to configured status.
                            //
                            // Update WSC status
                            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;  

                            // Change the state to next one
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                            pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                            pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;

                            // The protocol is finished.
                            pPort->StaCfg.WscControl.bWPSLEDSetTimer = TRUE;
                            LedCtrlSetBlinkMode(pAd, LED_WPS_SUCCESS);
                            pPort->StaCfg.WscControl.AsExtreg = TRUE;
                            DBGPRINT(RT_DEBUG_TRACE, ("WscControl.AsExtreg is TRUE."));

                        }
                        else
                        {
                            if(pPort->StaCfg.WscControl.WscConfMode== WSC_ConfMode_REGISTRAR_UPNP)
                            {
                                PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr,
                                    pPort->StaCfg.WscControl.ExtRegEnrMAC, MAC_ADDRESS_LENGTH);
                                DBGPRINT(RT_DEBUG_TRACE, ("Profile0MAC:%x %x %x\n", 
                                    pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[3],
                                    pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[4],
                                    pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[5]));
                            }

                            // Send out M8
                            WscSendMessageM8(pAd, pPort, Id, Elem);
                            // Change the state to next one
                            // Change the state to next one
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_DONE;
                            //if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
                            //{
                                //pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;    
                            //}
                        }
                    }
                    else    // fail
                        goto err;
                }
                // We have to resend M8 if we receive M7 instead of WSC_DONE
                else if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_DONE)
                {

                    // It's a retry. Use lasttx2 instead of lasttx  
                    pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Length = 
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length;
                    PlatformMoveMemory(pPort->StaCfg.WscControl.SavedWPSMessage.LastTx.Data, 
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Data,
                                pPort->StaCfg.WscControl.SavedWPSMessage.LastTx2.Length);
                
                    // Process M7
                    if (WscRecvMessageM7(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        // Send out M8
                        WscSendMessageM8(pAd, pPort, Id, Elem);
                        // Change the state to next one
                        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_DONE;
                    }
                    else    // fail
                        goto err;
                }

                if (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED)
                {
                    // do nothing.
                }
                else
                {
                    // EAP Rx processed, set new EAP Rx timer
                    if(!(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP))
                    {
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
                    }
                }
            }
            break;

        case WSC_MSG_WSC_DONE:
            {
                // Patch: Siemens WPS AP.
                if (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED)
                {
                    // do nothing.
                    return;
                }

                // This is only valid at WSC_STATE_WAIT_DONE
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_DONE)
                {
                    // Process WSC Done
                    if (WscReceiveEapDone(pAd, pPort, Id, Elem, ExtRegMsg))    // succeed
                    {
                        if (IS_P2P_REGISTRA(pPort))
                        {
                            WpseSendEapFail(pAd, pPort, Id, Elem);
                            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                            pHeader_802_11 = (PHEADER_802_11)&Elem->Msg[0];

                            // GO's is set to configured once WpsDone.
                            if (pPort->P2PCfg.bConfiguredAP != WSC_APSTATE_CONFIGURED)
                            {
                                pPort->P2PCfg.bConfiguredAP = WSC_APSTATE_CONFIGURED;    
                                DBGPRINT(RT_DEBUG_TRACE, ("!!! GO becomes configured status !!!\n"));
                            }

                            P2pWpsDone(pAd, pHeader_802_11->Addr2, pPort);
                        }
                        else
                        {
                            if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
                            {
                                // Send out EAP-Rsp-Done
                                WscSendEapRspAck(pAd, pPort, Id, Elem);
                                // Change the state to next one
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_DISCONN;

                                //Don't do auto reconnection after configuring AP since AP's configuration will be changed.
                                pAd->MlmeAux.AutoReconnectSsidLen= 32;
                                PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
                            }
                            else    //UPnP External Registrar
                            {
                                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                                pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                                pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;
                            }
                            
                            if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
                            {
                                pPort->StaCfg.WscControl.AsExtreg = TRUE;
                                DBGPRINT(RT_DEBUG_TRACE, ("WscControl.AsExtreg is TRUE."));
                            }
                        }
                    }
                    else    // fail
                        goto err;
                }
                
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_DISCONN)
                { // Set a new timer. timeout = Per Message Timeout on AP (15 sec)
                  // Regardless of Eap-Fail, this will be configured after timeout.
                    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
                    if(pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_REGISTRAR_UPNP)
                    {
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, 15000);
                    }
                }
            }   
            break;

        case WSC_MSG_EAP_FAIL:
            {
                // Patch: Siemens WPS AP.
                if (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED)
                {
                    // do nothing.
                    return;
                }

                // Receive Eap-Fail at WSC_STATE_WAIT_DISCONN, process it and go to WSC_STATE_CONFIGURED
                if (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_DISCONN)
                {
                    
                    // Backup the AP's BSSID for the UI.
                    // Note that the STA supports only one profile.
                    PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr,
                        pPort->StaCfg.WscControl.WscAPBssid, MAC_ADDRESS_LENGTH);
                    
                    // Receive Eap-Fail, cancel timer
                    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
                    
                    // call to process Eap-Fail
                    WscReceiveEapFail(pAd, Id, Elem);
                    
                    // Update WSC status
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;  

                    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
                    {
                        pPort->StaCfg.WscControl.AsExtreg = TRUE;
                        DBGPRINT(RT_DEBUG_TRACE, ("WscControl.AsExtreg is TRUE."));
                    }

                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                    pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                    pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;

                    // Reset the scanning counter.
                    pAd->StaCfg.ScanCnt = 0;

                    // The protocol is finished.
                    pPort->StaCfg.WscControl.bWPSLEDSetTimer = TRUE;
                    LedCtrlSetBlinkMode(pAd, LED_WPS_SUCCESS);
                }
                // Received unexpected EAP-FAIL, restart the association
                else if (pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_EAPFAIL)
                {
                    // call to process Eap-Fail
                    WscReceiveEapFail(pAd, Id, Elem);

                    // re-associate immediately and change to state WSC_STATE_START
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, 0);
                
                }
                // Receive Eap-Fail, it indicate the EAP failed, we have to set teh correct state and staus
                else
                {
                    // call to process Eap-Fail
                    WscReceiveEapFail(pAd, Id, Elem);
                    
                    // Update WSC status
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_FAILED;  

                    // Change the state to next one
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                    pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                    pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("WscEAPRegistrarAction : Rx EAP-FAIL\n"));
            }
            break;

        case WSC_MSG_WSC_NACK:
            {
                // Patch: Siemens WPS AP.
                if (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED)
                {
                    // do nothing.
                    return;
                }

                // Receive EAP-Req-Nack at entering lock-step (M3~Done)
                if ((pPort->StaCfg.WscControl.WscState >= WSC_STATE_WAIT_M3) && 
                    (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
                {
                    WscReceiveEapNack(pAd, pPort, Id, Elem);
                    if (IS_P2P_REGISTRA(pPort))
                        WpseSendEapFail(pAd, pPort, Id, Elem);
                    else
                    {
                        WscSendEapRspNack(pAd, pPort, Id, Elem);
                    }
                }
            }
            break;
            
        case WSC_MSG_FRAG_ACK:
            // Receive WSC-Frag ACk
            if((IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd))&&(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
            {
                WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, Elem, FALSE);
            }   
            break;

        default:
            {
                DBGPRINT(RT_DEBUG_TRACE, ("WscEAPRegistrarAction : Unsupported Msg Type\n"));
            }
            break;
    }

    //DBGPRINT(RT_DEBUG_TRACE, ("<----- WscEAPRegistrarAction\n"));
    return;

err:
    // TODO:  Might need to send out a Nack and change the state to wait for an EAP-FAIL
    if (((pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_PASSWORD_MISMATCH)||
            (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EXTREG_M6_FAIL)) &&        
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_EAPFAIL))
    {
        DBGPRINT(RT_DEBUG_TRACE,(" . 1 PIN = %x  %x. 2 PIN = %x  %x\n", *(PULONG)&pPort->P2PCfg.PinCode[0], *(PULONG)&pPort->P2PCfg.PinCode[4], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));
        // set config error
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigError = cpu2be16(WSC_ERROR_DEV_PWD_AUTH_FAIL);
        WscSendEapRspNack(pAd, pPort, Id, Elem);
        // Change the state to next one
        pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_EAPFAIL;
    }

    // For a invalid nonce, invalid authenticator and invalid length:
    // sliently ignore this message and stay at the same state
    
    // EAP Rx processed, set new EAP Rx timer
    if ((pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_EAPFAIL) && 
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_DISCONN) &&
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_CONFIGURED))
        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);

    DBGPRINT(RT_DEBUG_TRACE, (" Error handling <----- WscEAPRegistrarAction\n"));
    return;

}

//return 1 : No need buffer
//return 2 : Use MFbuf as the message
//return 3 : Need buffer
UCHAR   WscMfNeedBuf(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT   pPort,
    OUT PUCHAR              pId,
    IN PMLME_QUEUE_ELEM pElem)
{
    LONG                Length;
    PUCHAR              pData;
    PHEADER_802_11      pHeader;
    PIEEE8021X_FRAME    p802_1x;
    PEAP_FRAME          pEap, pEapUpdateId;
    PWSC_FRAME          pWsc;
    UCHAR               Ret;
    ULONG               i;

    if (pElem->MsgLen < ((LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME))))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WscMfNeedBuf - Elem Length Wrong!!\n"));
        return 0;
    }

    pHeader = (PHEADER_802_11)pElem->Msg;
    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    Length = pElem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;
    *pId = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("MFNeeddBuf - Length:%d\n", Length));

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    //Length = be2cpu16(p802_1x->Length);

    if(p802_1x->Type == EAPOL_TYPE_START)
    {
        //Due to GO will process EAPOL_START message in P2P mode, so it needs to skip it
        DBGPRINT(RT_DEBUG_TRACE, ("MfNeedBuf return 1 since EAPOL_START \n"));
        return (1);
    }
    
    pData += sizeof(IEEE8021X_FRAME);
    Length -= sizeof(IEEE8021X_FRAME);

    // Start of EAP frame
    pEap = (PEAP_FRAME) pData;
    
    *pId = (pEap->Id);
    
    // check for EAP-Fail which has length smaller than 5
    if (pEap->Code == EAP_CODE_FAIL)
    {
        // Receive EAP-Fail
        DBGPRINT(RT_DEBUG_TRACE, ("MfNeedBuf return 1 since eapfail\n"));
        return(1);
    }

    if(pEap->Type == EAP_TYPE_ID)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MfNeedBuf return 1 since id\n"));
        return (1);
    }
    // Length include EAP 5 bytes header
    pData += sizeof(EAP_FRAME);
    Length -= sizeof(EAP_FRAME);    


    // WSC messages, check for WSC Op-Code, or Messages
    pWsc = (PWSC_FRAME) pData;
    pData += sizeof(WSC_FRAME);
    Length -= sizeof(WSC_FRAME);

    if(Length < 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MfNeedBuf return 1 since Invalid Length \n"));
        return (1);
    }

    if(((pWsc->Flags & 0x01) == 0x1) && ((pPort->StaCfg.WscControl.LastWSCEAPFlags & 0x01) == 0x0))
    {
        PlatformMoveMemory(MFBuf, pElem->Msg, LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME)+ sizeof(EAP_FRAME) + sizeof(WSC_FRAME));
        pPort->StaCfg.WscControl.MFBufLen = LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME)+ sizeof(EAP_FRAME) + sizeof(WSC_FRAME);
        
        if((pWsc->Flags & 0x02) == 0x2)
        {
            pData += 2;
            Length -= 2;
            PlatformMoveMemory(MFBuf + pPort->StaCfg.WscControl.MFBufLen, pData, Length);
            pPort->StaCfg.WscControl.MFBufLen += Length;
            DBGPRINT(RT_DEBUG_TRACE, ("%s LF enabled. Length:%d\n", __FUNCTION__, Length));
        }
        else
        {
            PlatformMoveMemory(MFBuf + pPort->StaCfg.WscControl.MFBufLen, pData, Length);
            pPort->StaCfg.WscControl.MFBufLen += Length;
        }
        
        //PlatformMoveMemory(MFBuf, pElem->Msg, LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME)+ be2cpu16(p802_1x->Length));
        //pPort->StaCfg.WscControl.MFBufLen = LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME)+ be2cpu16(p802_1x->Length);
        DBGPRINT(RT_DEBUG_TRACE, ("%s Last is NOT Fragmented, This frame is Fragmented. Buffer this frame Length = %d\n", __FUNCTION__, pPort->StaCfg.WscControl.MFBufLen));
        for(i = 0; i < pPort->StaCfg.WscControl.MFBufLen; i++)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WscMfNeedBuf get - 0x%x\n", MFBuf[i]));
        }

    }
    else if((pPort->StaCfg.WscControl.LastWSCEAPFlags & 0x01) == 0x1)
    {
        PlatformMoveMemory(MFBuf + pPort->StaCfg.WscControl.MFBufLen, pData, Length);
        pPort->StaCfg.WscControl.MFBufLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, ("%s Last is Fragmented. Always Buffer this packet (Length = %d)\n", __FUNCTION__, Length));
        for(i = 0; i < (USHORT)Length; i++)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WscMfNeedBuf get - 0x%x\n", *(pData+i)));
        }
    }

    if((pWsc->Flags & 0x01) == 0x1)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MfNeedBuf return 3\n"));
        Ret = 3;
    }
    else if(((pPort->StaCfg.WscControl.LastWSCEAPFlags & 0x01) == 0x1) && ((pWsc->Flags & 0x01) == 0x0))
    {
        USHORT  IEEE8021XLength = 0;
        IEEE8021XLength = be2cpu16(pPort->StaCfg.WscControl.MFBufLen - (LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME)));
        PlatformMoveMemory(&MFBuf[32+2], &IEEE8021XLength, sizeof(USHORT));
        PlatformMoveMemory(&MFBuf[32+2+4], &IEEE8021XLength, sizeof(USHORT));
        DBGPRINT(RT_DEBUG_TRACE, ("MfNeedBuf return 2, MFBuf Len:%d\n", pPort->StaCfg.WscControl.MFBufLen));
        Ret = 2;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MfNeedBuf return 1\n"));
        Ret = 1;
    }
    
    //Update EAP Id
    pEapUpdateId = (PEAP_FRAME)&MFBuf[LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME)];
    pEapUpdateId->Id = *pId;

    //Update the LastWSCEAPFlags
    pPort->StaCfg.WscControl.LastWSCEAPFlags = pWsc->Flags;

    return Ret;

}

/*
    ========================================================================
    
    Routine Description:
        Classify EAP message type

    Arguments:
        pAd         - NIC Adapter pointer
        Elem        - The EAP packet
        
    Return Value:
        Received EAP message type

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
UCHAR   WscRxMsgType(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    OUT PUCHAR              pId,
    IN  PMLME_QUEUE_ELEM    pElem) 
{
    USHORT              Length;
    PUCHAR              pData;
    PIEEE8021X_FRAME    p802_1x;
    PEAP_FRAME          pEap;
    PWSC_FRAME          pWsc;
    USHORT              WscType, WscLen;

    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    if (p802_1x->Type == EAPOLStart)
    {
        *pId = 0;
        return(WSC_MSG_EAP_OL_START);
    }

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    if (p802_1x->Type == EAPOLStart)
    {
        *pId = 0;
        return(WSC_MSG_EAP_OL_START);
    }
    
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);

    // Start of EAP frame
    pEap = (PEAP_FRAME) pData;

    // check for EAP-Fail which has length smaller than 5
    if (pEap->Code == EAP_CODE_FAIL)
    {
        // Receive EAP-Fail
        *pId = pEap->Id;
        return(WSC_MSG_EAP_FAIL);
    }

    // check length for EAP-packet WSC type
    if (Length < sizeof(EAP_FRAME))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : EAP length is too small --> %d; \n", Length));       
        return(WSC_MSG_UNKNOWN);
    }
    // Length include EAP 5 bytes header
    pData += sizeof(EAP_FRAME);
    Length -= 5;    
    *pId = pEap->Id;
    

    switch (pEap->Code)
    {
        case EAP_CODE_REQ:
            switch (pEap->Type)
            {
                case EAP_TYPE_ID:
                    return(WSC_MSG_EAP_REQ_ID);
                    break;
                    
                case EAP_TYPE_WSC:
                    // WSC messages, check for WSC Op-Code, or Messages
                    pWsc = (PWSC_FRAME) pData;
                    pData += sizeof(WSC_FRAME);
                    Length -= sizeof(WSC_FRAME);

                    // Verify SMI first
                    if (((pWsc->SMI[0] * 256 + pWsc->SMI[1]) * 256 + pWsc->SMI[2]) != WSC_SMI)
                    {
                        // Wrong WSC SMI Vendor ID, Update WSC status
                        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_WRONG_SMI;
                        return(WSC_MSG_UNKNOWN);
                    }

                    // Verify Vendor Type
                    if (cpu2be32(pWsc->VendorType) != WSC_VENDOR_TYPE)
                    {
                        // Wrong WSC Vendor Type, Update WSC status
                        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_WRONG_VENDOR_TYPE;
                        return(WSC_MSG_UNKNOWN);
                    }

                    if (pWsc->OpCode == WSC_OPCODE_START)
                    {
                        if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
                        {
                            // Receive WSC Start
                            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_START_RECEIVED;
                            return(WSC_MSG_EAP_REQ_START);
                        }
                        else
                        {   // Wrong Opcode, ignore it
                            return(WSC_MSG_EAP_REQ_START);  
                        }
                    }
                    
                    if(pWsc->OpCode == WSC_OPCODE_FRAG_ACK)
                    {
                        return (WSC_MSG_FRAG_ACK);
                    }
                    
                    // Not Wsc Start, We have to look for WSC_IE_MSG_TYPE to classify M1 ~ M8, the remain size must large than 4
                    while (Length > 4)
                    {
                        WscType = cpu2be16(*((PUSHORT) pData));
                        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
                        if (WscType == WSC_IE_MSG_TYPE)
                        {
                            return(*(pData + 4));   // Found the message type
                        }
                        else
                        {
                            pData  += (WscLen + 4);
                            Length -= (WscLen + 4);
                        }
                    }
                    break;
                    
                default:
                    DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : Unsupported EAP Code --> %d; \n", pEap->Code));      
                    break;
            }
            break;
            
        case EAP_CODE_RSP:
            if (IS_P2P_REGISTRA(pPort))
            {
                switch (pEap->Type)
                {
                    case EAP_TYPE_ID:
                        DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : EAP-Rsp_Id\n"));
                        return(WSC_MSG_EAP_RSP_ID);
                        break;
                        
                    case EAP_TYPE_WSC:
                        // WSC messages, check for WSC Op-Code, or Messages
                        pWsc = (PWSC_FRAME) pData;
                        pData += sizeof(WSC_FRAME);
                        Length -= sizeof(WSC_FRAME);

                        // Verify SMI first
                        if (((pWsc->SMI[0] * 256 + pWsc->SMI[1]) * 256 + pWsc->SMI[2]) != WSC_SMI)
                        {
                            // Wrong WSC SMI Vendor ID, Update WSC status
                            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_WRONG_SMI;               
                            DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : WSC_SMI wrong. WSC_MSG_UNKNOWN\n"));
                            return(WSC_MSG_UNKNOWN);
                        }

                        // Verify Vendor Type
                        if (cpu2be32(pWsc->VendorType) != WSC_VENDOR_TYPE)
                        {
                            // Wrong WSC Vendor Type, Update WSC status
                            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_WRONG_VENDOR_TYPE;               
                            DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : Vendor Type wrong. WSC_MSG_UNKNOWN\n"));
                            return(WSC_MSG_UNKNOWN);
                        }

                        if (pWsc->OpCode == WSC_OPCODE_START)
                        {
                            if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
                            {
                                // Receive WSC Start
                                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_START_RECEIVED;              
                                DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType :  WPS Eap Request State \n"));
                                return(WSC_MSG_EAP_REQ_START);
                            }
                            else
                            {   // Wrong Opcode, ignore it
                                // Receive WSC Start
                                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_START_RECEIVED;              
                                DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType :  WPS Eap Request State \n"));
                                return(WSC_MSG_EAP_REQ_START);  
                            }
                        }

                        // Not Wsc Start, We have to look for WSC_IE_MSG_TYPE to classify M1 ~ M8, the remain size must large than 4
                        while (Length > 4)
                        {
                            WscType = cpu2be16(*((PUSHORT) pData));
                            WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
                            if (WscType == WSC_IE_MSG_TYPE)
                            {
                                DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : Find Message %d. \n", *(pData + 4)));
                                return(*(pData + 4));   // Found the message type
                            }
                            else
                            {
                                pData  += (WscLen + 4);
                                Length -= (WscLen + 4);
                            }
                        }                   
                    break;
                }
            
            }
            DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : Normal WPS Not supposed to receive EAP-Rsp. P2P state = %d.\n", pPort->P2PCfg.P2PConnectState));      
 
            break;
            
        case EAP_CODE_FAIL:
            // Receive EAP-Fail
            return(WSC_MSG_EAP_FAIL);
            break;

        default:
            DBGPRINT(RT_DEBUG_TRACE, ("WscRxMsgType : Unsupported EAP Code --> %d; \n", pEap->Code));      
            break;          
    }
    
    return(WSC_MSG_UNKNOWN);
}

/*
    ========================================================================
    
    Routine Description:
        Classify WSC message type

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
BOOLEAN WscMsgTypeSubst(
    IN  UCHAR   EAPType,
    IN  UCHAR   EAPCode,
    OUT ULONG   *MsgType)   
{
    switch (EAPType)
    {
        case EAPOLStart:
            *MsgType = WSC_EAP_REQ_MSG;
            DBGPRINT(RT_DEBUG_TRACE, ("%s : Received WPS EAP_START frame.\n", __FUNCTION__));
            break;

        case EAPPacket:
            switch (EAPCode)
            {
                case EAP_CODE_REQ:
                    *MsgType = WSC_EAP_REQ_MSG;
                    break;

                case EAP_CODE_RSP:
                    *MsgType = WSC_EAP_RSP_MSG;
                    break;

                case EAP_CODE_FAIL:
                    *MsgType = WSC_EAP_FAIL_MSG;
                    break;

                default:
                    DBGPRINT(RT_DEBUG_TRACE, ("WscMsgTypeSubst : unsupported EAP Code; \n"));      
                    return FALSE;       
            }
            break;
            
        default:
                    DBGPRINT(RT_DEBUG_TRACE, ("WscMsgTypeSubst : unsupported EAP Code;%d \n", EAPCode));       
            return FALSE;       
    }   
    return TRUE;
}

/*
    ========================================================================
    
    Routine Description:
        WSC connection time out

    Arguments:
        FunctionContext     NIC Adapter pointer
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID WscConnectTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    BOOLEAN     Cancelled = FALSE;

    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    // 120 seconds WPS walk time expiration.
    pPort->StaCfg.WscControl.bWPSWalkTimeExpiration = TRUE;

    // 1. cancel all timer
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscConnectTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPINTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer, &Cancelled);


    DBGPRINT(RT_DEBUG_TRACE, ("WscConnectTimeoutTimerCallback --> WscState = %d. WscStatus = %d. \n", pPort->StaCfg.WscControl.WscState, pPort->StaCfg.WscControl.WscStatus));
    // if link is up, there shall be nothing wrong
    // perhaps we will set another flag to do it
    if ((pPort->StaCfg.WscControl.bWPSSuccessandLinkup) &&
        (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) &&
        (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WscConnectTimeoutTimerCallback --> Connection OK\n"));
    }
    else
    {
    
        DBGPRINT(RT_DEBUG_TRACE, ("WscConnectTimeoutTimerCallback -->   !\n"));

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_FAILED;  
        pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
        pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
        pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;
        
        LedCtrlSetBlinkMode(pAd, LED_LINK_DOWN);

        P2PStopWpsTimerCallback(NULL, pAd, NULL, NULL);

        if ((!IS_P2P_GO_OP(pPort)) && (!IS_P2P_CLIENT_OP(pPort)))
        {
            // WPS LED mode 7 or 8 or 12 .
            if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_7) || 
                ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_8) ||
                ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_12)||
                ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15))
            {
                pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = FALSE;

                // Turn off the WPS LED modoe due to the maximum WPS processing time is expired (120 seconds).
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                if (IS_P2P_GO_OP(pPort))
                {
                    pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                }
                else
                    DBGPRINT(RT_DEBUG_TRACE, ("WscConnectTimeoutTimerCallback --> Fail to connect\n"));
            }
            else if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // WPS LED mode 9.
            {   
                if (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE) // PIN method.
                {
                    // The NIC using PIN method fails to finish the WPS handshaking within 120 seconds.
                    LedCtrlSetBlinkMode(pAd, LED_WPS_ERROR);

                    // Turn off the WPS LED after 15 seconds.
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_WPS_FAIL_LED_PATTERN_TIMEOUT);

                    // The Ralink UI would make MTK_OID_N6_SET_DISCONNECT_REQUEST request while it receive STATUS_WSC_EAP_FAILED.
                    // Allow the NIC to turn off the WPS LED after WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT seconds.
                    pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = TRUE;
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);
                    
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: The NIC using PIN method fails to finish the WPS handshaking within 120 seconds.\n", __FUNCTION__));
                }
                else if (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE) // PBC method.
                {
                    switch (pPort->StaCfg.WscControl.WscLastWarningLEDMode) // Based on last WPS warning LED mode.
                    {
                        case 0:
                        case LED_WPS_ERROR:
                            // Failed to find any partner or find partner but WPS does not success.
                            LedCtrlSetBlinkMode(pAd, LED_WPS_ERROR);

                            // Turn off the WPS LED after 15 seconds.
                            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_WPS_FAIL_LED_PATTERN_TIMEOUT);

                            // The Ralink UI would make MTK_OID_N6_SET_DISCONNECT_REQUEST request while it receive STATUS_WSC_EAP_FAILED.
                            // Allow the NIC to turn off the WPS LED after WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT seconds.
                            pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = TRUE;
                            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);
                            
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: Last WPS LED status is LED_WPS_ERROR.\n", __FUNCTION__));
                            break;              

                        default:
                            // do nothing.
                            break;
                    }
                }
                else
                {
                    // do nothing.
                }
            }
            else
            {
                // do nothing.
            }
        }
        DBGPRINT(RT_DEBUG_TRACE, ("WscConnectTimeoutTimerCallback --> Fail to connect\n"));
    }
}

/*
    ========================================================================
    
    Routine Description:
        Exec scan after scan timer expiration 

    Arguments:
        FunctionContext     NIC Adapter pointer
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID WscMlmeSyncScanTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    // call to execute the scan actions
    WscScanExec(pAd, pPort);

    // register 10 second timer for PBC or PIN connection execution
    if (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE)
    {
        // Prevent infinite loop if conncet time out didn't stop the repeat scan
        if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscPBCTimer, 15000);
    }
    else if (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_REGISTRA_SPEC)
    {
        // Prevent infinite loop if conncet time out didn't stop the repeat scan
        if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscPINTimer, 10000);
    }
    else if (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_USER_SPEC)
    {
        // Prevent infinite loop if conncet time out didn't stop the repeat scan
        if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscPINTimer, 10000);
    }
    else if (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE)
    {
        // Prevent infinite loop if conncet time out didn't stop the repeat scan
        if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscPINTimer, 10000);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("!!! WscMlmeSyncScanTimeoutTimerCallback !!!\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Exec PBC connection verification after PBC timer expiration 

    Arguments:
        FunctionContext     NIC Adapter pointer
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID WscPBCTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    DBGPRINT(RT_DEBUG_TRACE, ("!!! WscPBCTimeoutTimerCallback !!!\n"));

    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        WscExtregPBCExec(pAd, pPort);
    }
    else
    {
        WscPBCExec(pAd, pPort);
    }
    
    // call Mlme handler to execute it
    NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));

}

VOID WscPINTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    DBGPRINT(RT_DEBUG_TRACE, ("!!! WscPINTimeoutTimerCallback !!!\n"));

    WscPINExec(pAd);
}

VOID WscEAPRxTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    MLME_DISASSOC_REQ_STRUCT   DisassocReq;
    UCHAR       i;
    MAC_TABLE_ENTRY *pEntry;

    // Change to p2p port number if I connect as Enrollee in con-current mode.
    if ((pAd->PortList[pPort->P2PCfg.PortNumber] != NULL) &&
        (IS_P2P_ENROLLEE(pPort)) && 
        (IS_P2P_CON_CLI(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) &&
        (INFRA_ON(pAd->PortList[pPort->P2PCfg.PortNumber])))
    {
        pPort = (PMP_PORT)pAd->PortList[pPort->P2PCfg.PortNumber];
        DBGPRINT(RT_DEBUG_TRACE, ("change to virtual#2 (P2PCfg.PortNumber = %d) \n", pPort->P2PCfg.PortNumber));
    }

    if (pPort == NULL)
        return;
    
    // No EAP receive within WSC_PER_MSG_TIMEOUT, reset the state machine
    // and redo protocols if it's still within 2 min
    // Set WSC state to WSC_STATE_START
    pPort->StaCfg.WscControl.WscState = WSC_STATE_START;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_START_ASSOC;

    // The protocol is connecting to a partner.
    LedCtrlSetBlinkMode(pAd, LED_WPS_IN_PROCESS);

    DBGPRINT(RT_DEBUG_TRACE, ("!!! WscEAPRxTimeoutTimerCallback !!!\n"));

    if((INFRA_ON(pPort)) && (!IS_P2P_REGISTRA(pPort)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Disassociate with current WPS AP...\n"));
        MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
        MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
            sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));
    }

    P2PPrintState(pAd);
    if (IS_P2P_REGISTRA(pPort))
    {
        for (i = 0; i < MAX_P2P_GROUP_SIZE;i++)
        {
            // prevent printing too much gabbagemessage....
            if (i < 9)
                P2PPrintP2PEntry(pAd, i);
            if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_WPS)
            {
                pEntry = MacTableLookup(pAd,  pPort, pAd->pP2pCtrll->P2PTable.Client[i].InterfaceAddr);
                if (pEntry != NULL)
                {
                    MLME_DEAUTH_REQ_STRUCT      DeAuthRequest;

                    DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
                    DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTab  Disconnect  ==>..%x \n", pPort->P2PCfg.Bssid[5]));
                    COPY_MAC_ADDR(DeAuthRequest.Addr, pPort->P2PCfg.Bssid);
                    MlmeAuthDeauthReqAction(pAd, pPort->P2PCfg.PortNumber, &DeAuthRequest);
                    MacTableDeleteAndResetEntry(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pEntry->Aid, pEntry->Addr, TRUE);
                }
                pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY;
                break;
            }
        }
        P2pConnectStateUpdate(pAd, pPort);
    }
    
    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer, 1000);

    if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
        pPort->StaCfg.WscControl.WscState = WSC_STATE_LINK_UP;
}

VOID WscRxTimeOuttoScanTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    DBGPRINT(RT_DEBUG_TRACE, ("!!! WscRxTimeOuttoScanTimerCallback !!!\n"));

    if(((pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE) || 
        (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE)||
        (pPort->StaCfg.WscControl.WscMode ==WSC_PIN_MODE_USER_SPEC) ||
        (pPort->StaCfg.WscControl.WscMode ==WSC_PIN_MODE_REGISTRA_SPEC))&&
        (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE))
    {
        // Call WscScan subroutine
        WscScanExec(pAd, pPort);

        // Set 10 second timer to invoke PBC connection actions.
        if ((pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE) && (!IS_P2P_ENROLLEE(pPort)))  
        {
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscPBCTimer, 15000);
            DBGPRINT(RT_DEBUG_TRACE, ("%s set PBCTimer\n", __FUNCTION__));
        }
        
    }
}

VOID WscM8WaitEapFailTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    // It is not important for lost Eap-Fail
    // Wait Eap-Fail until timeout(WSC_PER_MSG_TIMEOUT), set WPS state to complete state
    pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;
    pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
    pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;

    // The protocol is finished.
    pPort->StaCfg.WscControl.bWPSLEDSetTimer = TRUE;
    LedCtrlSetBlinkMode(pAd, LED_WPS_SUCCESS);

    DBGPRINT(RT_DEBUG_TRACE, ("!!! WscM8WaitEapFailTimeoutTimerCallback !!!\n"));
}

VOID WscEapM2TimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    
    BOOLEAN     Cancelled = FALSE;

    /* 1. M2Timer has started while M1 sent
     * 2. Cancel it once STA re-associate. Imply that ...
     *      ->Reset EapRxTimer beyond the state of WSC_STATE_WAIT_M2
     *      ->Received Eap-Fail after Rsp-Ack   
     * 3. Excute this timeout
     */
     
    if ((pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF) &&
        (pPort->StaCfg.WscControl.WscState < WSC_STATE_WAIT_M4))
    {
        // re-associate immediately and change to state WSC_STATE_START
        PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, 0);
        
        DBGPRINT(RT_DEBUG_TRACE, ("!!! WscEapM2TimeoutTimerCallback !!!\n"));
    }
}

//
// After the NIC connects with a WPS AP or not, 
// the WscLEDTimerTimerCallback timer controls the LED behavior according to LED mode.
//
VOID WscLEDTimerTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT)FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    // WPS LED mode 7 and 8.
    if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_7) || 
        ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_8) ||
        ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_12) )
    {
        LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Turn off the WPS successful LED pattern.\n", __FUNCTION__));
    }
    else if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9)) // WPS LED mode 9.
    {
        switch (pPort->StaCfg.WscControl.WscLEDMode) // Last WPS LED state.
        {
            // Turn off the blue LED after 300 seconds.
            case LED_WPS_SUCCESS:
                // Turn off the WPS LED for one second.
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);

                pPort->StaCfg.WscControl.bIndicateConnNotTurnOff = FALSE;

                // Turn on/off the WPS success LED according to AP's encryption algorithm after one second.
                PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_WPS_TURN_OFF_LED_TIMEOUT);

                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS => LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                break;

            // After turn off the blue LED for one second.
            // AP uses an encryption algorithm:
            // a) YES: Turn on the blue LED.
            // b) NO: Turn off the blue LED.
            case LED_WPS_TURN_LED_OFF:
                if(INFRA_ON(pPort))
                {
                    if ((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) && 
                        (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED))
                    {
                        if (WscAPHasSecuritySetting(pAd, pPort) == TRUE) // The NIC connects with an AP using an encryption algorithm.
                        {
                            // Turn on WPS success LED.
                            LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_ON_BLUE_LED);
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF => LED_WPS_TURN_ON_BLUE_LED\n", __FUNCTION__));
                        }
                        else // The NIC connects with an AP using OPEN-NONE.
                        {
                            // Turn off the WPS LED.
                            LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF => LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                        }
                    }
                }
                break;
                
            // Turn off the amber LED after 15 seconds.
            case LED_WPS_ERROR:
                // Turn off the WPS LED.
                pPort->StaCfg.WscControl.bErrDeteWPSTermination = FALSE;
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_ERROR/LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                break;

            // Turn off the amber LED after ~3 seconds.
            case LED_WPS_SESSION_OVERLAP_DETECTED:
                // Turn off the WPS LED.
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                break;

            default:
                // do nothing.
                break;
        }
    }
    else if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15)) 
    {
        switch (pPort->StaCfg.WscControl.WscLEDMode) // Last WPS LED state.
        {
            // Turn off the blue LED after 300 seconds.
            case LED_WPS_SUCCESS:
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS => LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                break;
                
            // Turn off the LED after 120 seconds.
            case LED_WPS_ERROR:
                // Turn off the WPS LED.
                pPort->StaCfg.WscControl.bErrDeteWPSTermination = FALSE;
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_ERROR/LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                break;

            // Turn off the LED after ~10 seconds.
            case LED_WPS_SESSION_OVERLAP_DETECTED:
                // Turn off the WPS LED.
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                break;

            default:
                // do nothing.
                DBGPRINT(RT_DEBUG_TRACE, ("LED4200 check: %s %d ;Not Support WscLEDMode\n",__FUNCTION__,__LINE__));
                break;
        }
    }
    else
    {
        // do nothing.
    }   
}

VOID WscSkipTurnOffLEDTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    // Allow the NIC to turn off the WPS LED again.
    pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = FALSE;
    pPort->StaCfg.WscControl.bSkipWPSTurnOffLEDAfterSuccess = FALSE;
    pPort->StaCfg.WscControl.bSessionOverlap = FALSE;
    pPort->StaCfg.WscControl.bSkipWPSLEDCMD = FALSE;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: Allow the NIC to turn off the WPS LED again.\n", __FUNCTION__));
}

VOID    WscPushHWPBCDisconnectAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort)
{
    if (INFRA_ON(pPort))
    {
        MLME_DISASSOC_REQ_STRUCT    DisReq;
        
        // Set to immediately send the media disconnect event
        pAd->MlmeAux.CurrReqIsFromNdis = TRUE;

        DBGPRINT(RT_DEBUG_TRACE, ("disassociate with current AP before Hardward PBC\n"));
        MlmeCntDisassocParmFill(pAd, &DisReq, pPort->PortCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
        MlmeEnqueue(pAd, pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DisReq);

        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
    }
    else if (ADHOC_ON(pPort))
    {
        MlmeCntLinkDown(pPort, FALSE);
    }
    
    // Always disable auto reconnection.
    // Set the AutoReconnectSsid to prevent it reconnect to old SSID
    // Since calling this indicate user don't want to connect to that SSID anymore.
    pAd->MlmeAux.AutoReconnectSsidLen= 32;
    pAd->MlmeAux.AutoReconnectStatus = FALSE;
    PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
}

VOID    WscPushHWPBCStartAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort)
{
    // Reset WPS LED mode state.
    pPort->StaCfg.WscControl.WscLEDMode = 0;
    pPort->StaCfg.WscControl.WscLastWarningLEDMode = 0;

    // Support Infra mode only
    pAd->StaCfg.BssType = BSS_INFRA;

    DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!! WPS - HW PBC !!!\n"));

    // 1. Set WSC Mode to PBC Enrollee
    pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_ENROLLEE;
    pPort->StaCfg.WscControl.WscMode = WSC_PBC_MODE;

    // 2. Init WSC State
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_IDLE;
    // For WSC, scan action shall be controlled by WSC state machine.
    pAd->StaCfg.ScanCnt = 3;

    // Reset the WPS walk time.
    pPort->StaCfg.WscControl.bWPSWalkTimeExpiration = FALSE;

    if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
    {
        MlmeRestartStateMachine(pAd,pPort);                                     
        DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
    }
    
    // 3. Start Action
    // Default settings: Send WSC IE on assoc and probe-req frames
    pPort->StaCfg.WscControl.WscEnAssociateIE = TRUE;
    pPort->StaCfg.WscControl.WscEnProbeReqIE = TRUE;

    // Cleaer bWPSSuccessandLinkup.
    pPort->StaCfg.WscControl.bWPSSuccessandLinkup = FALSE;

    WscPushPBCAction(pAd, pPort);
}
/*
    ========================================================================
    
    Routine Description:
        Push PBC from HW/SW Buttton

    Arguments:
        pAd    - NIC Adapter pointer
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/

VOID    WscPushPBCAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort)
{
    BOOLEAN     Cancelled = FALSE;
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscPushPBCAction\n"));

    // 0. PBC mode, disregard the SSID information, we have to get the current AP list 
    //    and check the beacon for Push buttoned AP.

    // 1. Cancel old timer to prevent use push continuously
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscConnectTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPINTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscLEDTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer, &Cancelled);

    // Set WSC state to WSC_STATE_INIT
    pPort->StaCfg.WscControl.WscState = WSC_STATE_INIT;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_SCAN_AP;

    // Init Registrar pair structures
    WscInitRegistrarPair(pAd, pPort);

    //default is using PIN 8 digit.
    pPort->StaCfg.WscControl.RegData.PINLen = 4;
    // For PBC, the PIN is all '0'
    pPort->StaCfg.WscControl.RegData.PIN[0] = '0';
    pPort->StaCfg.WscControl.RegData.PIN[1] = '0';
    pPort->StaCfg.WscControl.RegData.PIN[2] = '0';
    pPort->StaCfg.WscControl.RegData.PIN[3] = '0';
    pPort->StaCfg.WscControl.RegData.PIN[4] = '0';
    pPort->StaCfg.WscControl.RegData.PIN[5] = '0';
    pPort->StaCfg.WscControl.RegData.PIN[6] = '0';
    pPort->StaCfg.WscControl.RegData.PIN[7] = '0';


    // 2. Set 2 min timout routine
    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscConnectTimer, WSC_REG_SESSION_TIMEOUT);
    PlatformZeroMemory(pPort->StaCfg.WscControl.ExtRegEnrMAC, 6);

    // 3. Call WscScan subroutine
    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        WscExtRegScanExec(pPort);
    }
    else
    {
        WscScanExec(pAd, pPort);
    }

    // 4. Set 10 second timer to invoke PBC connection actions.
    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscExtregPBCTimer, 20000);
    }
    else
    {
        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscPBCTimer, 10000);

        //For led mode 6, customer wants the wps led can be started as doing wps.
        if (KeGetCurrentIrql() != PASSIVE_LEVEL)
        {
            LedCtrlSetBlinkMode(pAd, LED_WPS);
        }
        else
        {
            LedCtrlSetLed(pAd, LED_WPS);
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscPushPBCAction\n"));    

}

/*
    ========================================================================
    
    Routine Description:
        Doing an active scan with empty SSID, the scanened list will
        be processed in PBCexec or PINexec routines

    Arguments:
        pAd         - NIC Adapter pointer
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    WscScanExec(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort) 
{
    MLME_SCAN_REQ_STRUCT    ScanReq;

    if (IS_P2P_ENROLLEE(pPort))  
    {
        pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    }
    
    // Make sure connect state machine is idle
    if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
    {
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MLME_RESTART_STATE_MACHINE, NULL, 0);
        DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
    }

    // Prevent infinite loop if conncet time out didn't stop the repeat scan
    if (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_FAILED)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("!!! WscScanExec !!!\n"));
    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_SCAN_ENROLLEE;
    }
    else
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_SCAN_AP;

        // The protocol is searching for a partner in PBC mode.
        LedCtrlSetBlinkMode(pAd, LED_WPS_IN_PROCESS);

        // Clean up the scan table.
        BssTableInit(&pAd->ScanTab); 
        
        // Prepare Scan parameters and enqueue the MLME command
        MlmeCntScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_WSC_ACTIVE);
        MlmeEnqueue(pAd, pPort,SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
        if (IS_P2P_ENROLLEE(pPort))  
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - CNTL_WAIT_SCAN_FOR_CONNECT for p2p enrollee \n",  __FUNCTION__));
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_SCAN_FOR_CONNECT;
        }
        else
        {
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
        }
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));
    }
}

VOID    WscExtRegScanExec(
    IN PMP_PORT    pPort)
{
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_SCAN_ENROLLEE;
    DBGPRINT(RT_DEBUG_TRACE, ("STATUS_WSC_EXTREG_SCAN_ENROLLEE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n"));
}

VOID    WscExtregCallPBCExecTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    
    WscExtregPBCExec(pAd, pPort);
}

VOID    WscExtregCallScanExecTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    
    WscExtRegScanExec(pPort);
}


VOID    WscExtregPBCExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT    pPort)
{
    DBGPRINT(RT_DEBUG_TRACE, ("-------> WscExtregPBCExec\n"));
    
    if(pPort->StaCfg.WscControl.WscPbcEnrCount == 1)
    {
        //PlatformMoveMemory(pPort->StaCfg.WscControl.ExtRegEnrMAC, pPort->StaCfg.WscControl.WscPbcEnrTab[0].MacAddr, 6);
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_SCAN_ONE_ENROLLEE;
        DBGPRINT(RT_DEBUG_TRACE, ("STATUS_WSC_EXTREG_SCAN_ONE_ENROLLEE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
    }
    else if(pPort->StaCfg.WscControl.WscPbcEnrCount == 0)
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_PBC_NO_ENROLLEE;
        DBGPRINT(RT_DEBUG_TRACE, ("STATUS_WSC_EXTREG_PBC_NO_ENROLLEE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
    }
    else
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_PBC_TOO_MANY_ENROLLEE;
        DBGPRINT(RT_DEBUG_TRACE, ("STATUS_WSC_EXTREG_PBC_TOO_MANY_ENROLLEE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
    }
    pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
}

/*
    ========================================================================
    
    Routine Description:
        Doing PBC conenction verification, it will check current BSS list
        and find the correct number of PBC AP. If only 1 exists, it will
        start to make connection. Otherwise, it will set a scan timer
        to perform another scan for next PBC connection execution.

    Arguments:
        pAd         - NIC Adapter pointer
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    WscPBCExec(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort) 
{
    //WPS-PBC attempted PBC in Ad-hoc mode
    if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15)
    {
        //WPS-PBC Error
        if (ADHOC_ON(pPort))
        {
            LedCtrlSetBlinkMode(pAd, LED_WPS_ERROR);
            DBGPRINT(RT_DEBUG_TRACE, ("LED4200 check: WPS-PBC attempted PBC in Ad-hoc mode"));
            return;
        }   

    }

    // 1. Search the qualified SSID from current SSID list
    WscPBCBssTableSort(pAd, pPort, &pPort->StaCfg.WscControl.WscBssTab);

    // 2. Check the qualified AP for connection, if more than 1 AP avaliable, report error.
    if (pPort->StaCfg.WscControl.WscBssTab.BssNr != 1)
    {
        // Set WSC state to WSC_FAIL
        pPort->StaCfg.WscControl.WscState = WSC_STATE_FAIL;
        if (pPort->StaCfg.WscControl.WscBssTab.BssNr == 0)
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_PBC_NO_AP;

            // Failed to find any partner.
            pPort->StaCfg.WscControl.bErrDeteWPSTermination = FALSE;
            LedCtrlSetBlinkMode(pAd, LED_WPS_ERROR);
            // 2.1. Set 10 second timer to invoke another scan
            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscScanTimer, 5000);
        }
        else
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_PBC_TOO_MANY_AP;         

            if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // WPS LED mode 9.
            {
                // In case of the WPS LED mode 9, the UI would abort the connection attempt by making the MTK_OID_N6_SET_WSC_WPS_STATE_MACHINE_TERMINATION request.
                DBGPRINT(RT_DEBUG_TRACE, ("%s: Skip the WPS session overlap detected LED indication.\n", __FUNCTION__));
            }
            else // Other LED mode.
            {
                // Session overlap detected.
                LedCtrlSetBlinkMode(pAd, LED_WPS_SESSION_OVERLAP_DETECTED);
                if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15)
                {
                    // Turn off the WPS successful LED pattern after 10 seconds.
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, 10000);
                } 
            }
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("WscPBCExec --> AP list is %d, wait for next time\n", pPort->StaCfg.WscControl.WscBssTab.BssNr));

        // 2.2 We have to quit for now
        return;
    }

    // 3. Now we got the intend AP, Set the WSC state and enqueue the SSID connection command   
    pAd->MlmeAux.CurrReqIsFromNdis = FALSE; 

    if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
    {
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MLME_RESTART_STATE_MACHINE, NULL, 0);
        DBGPRINT(RT_DEBUG_TRACE, ("!!! WscPBCExec --> MLME busy, reset MLME state machine !!!\n"));
    }


    // 4. Set WSC state to WSC_STATE_START
    pPort->StaCfg.WscControl.WscState = WSC_STATE_START;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_START_ASSOC;

    // The protocol is connecting to a partner.
    LedCtrlSetBlinkMode(pAd, LED_WPS_IN_PROCESS);

    // 5. Connect to the WPS PBC-enabled AP by its BSSID directly.
    if (pPort->StaCfg.WscControl.WscBssTab.BssNr == 1)
    {
        // Backup the AP's BSSID
        PlatformMoveMemory(pPort->StaCfg.WscControl.WscAPBssid, 
                        pPort->StaCfg.WscControl.WscBssTab.BssEntry[0].Bssid, 
                        (sizeof(UCHAR) * MAC_ADDR_LEN));

        // Set the channel of the desired WPS PBC-enabled AP.
        pPort->ScaningChannel = pPort->StaCfg.WscControl.WscBssTab.BssEntry[0].Channel;

        // Saved the WSC AP channel.
        // This is because some dual-band APs have the same BSSID in different bands and 
        // the Ralink UI needs this information to connect the AP with correct channel.
        pPort->StaCfg.WscControl.WscAPChannel = pPort->StaCfg.WscControl.WscBssTab.BssEntry[0].Channel;

        // Reset auto-connect limitations
        pAd->MlmeAux.ScanForConnectCnt = 0;

        MlmeEnqueue(pAd,
            pPort,
             MLME_CNTL_STATE_MACHINE,
             MTK_OID_N5_SET_BSSID,
             MAC_ADDR_LEN,
             pPort->StaCfg.WscControl.WscBssTab.BssEntry[0].Bssid);

        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));
    }
}

VOID    WscPINExec(
    IN  PMP_ADAPTER   pAd) 
{
}

/*
    ========================================================================
    
    Routine Description:
        Find WSC PBC activated AP list

    Arguments:
        pAd         - NIC Adapter pointer
        OutTab      - Qualified AP BSS table
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
        All these constants are defined in wsc.h
        
    ========================================================================
*/
VOID WscPBCBssTableSort(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    OUT BSS_TABLE       *OutTab) 
{
    INT                 i, j, Len, idx;
    UCHAR               *pData;
    PBSS_ENTRY          pInBss, pOutBss;
    PWSC_IE             pWscIE;
    USHORT              DevicePasswordID;
    BOOLEAN             Found;
    UCHAR               UUID[8][16];
    UCHAR               TmpUUID[16];
    UCHAR               Duplicate;
    UCHAR               zeros16[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    // initialize Output BSS Table
    BssTableInit(OutTab);

    for (i = 0; i < pAd->ScanTab.BssNr; i++) 
    {
        pInBss  = (PBSS_ENTRY) &pAd->ScanTab.BssEntry[i];  // BSS entry for VarIE processing
        pOutBss = (PBSS_ENTRY)&OutTab->BssEntry[OutTab->BssNr];     // avaliable entry for new BSS list
        pData   = (UCHAR *) &pInBss->WSCInfoAtBeacons;
        Found   = FALSE;
        PlatformZeroMemory(&TmpUUID[0], 16);

        // 1. Check VarIE length
        if ((pInBss->WSCInfoAtBeaconsLen <= 0)&&(pInBss->WSCInfoAtProbeRspLen<= 0))
            continue;

        Len = pInBss->WSCInfoAtBeaconsLen;

        pData += 6;
        Len   -= 6;
        
        // 2. Start to look the PBC type within WSC VarIE
        while (Len > 0)
        {
            // Check for WSC IEs
            pWscIE = (PWSC_IE) pData;

            // Check for device password ID, PBC = 0x0004
            if (be2cpu16(pWscIE->Type) == WSC_IE_DEV_PASS_ID)
            {
                // Found device password ID
                DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data));
                DBGPRINT(RT_DEBUG_TRACE, ("WscPBCBssTableSort : DevicePasswordID = 0x%04x\n", DevicePasswordID));
                if (DevicePasswordID == DEV_PASS_ID_PBC)    // Check for PBC value
                {
                    // Found matching PBC AP in current list, add it into table and add the count
                    PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                    Found = TRUE;
                        
                    DBGPRINT(RT_DEBUG_TRACE, ("PBC Found by Beacon--> \n"));
                    DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x\n", OutTab->BssNr,
                        pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5]));
                }
            }

            // UUID_E is optional for beacons, but mandatory for probe-request
            if (be2cpu16(pWscIE->Type) == WSC_IE_UUID_E)
            {
                // Avoid error UUID-E storage from PIN mode
                PlatformMoveMemory(&TmpUUID[0], (UCHAR *)(pData+4), 16);
            }
                
            // Set the offset and look for PBC information
            // Since Type and Length are both short type, we need to offset 4, not 2
            pData += (be2cpu16(pWscIE->Length) + 4);
            Len   -= (be2cpu16(pWscIE->Length) + 4);
        }

        //We also look the PBC type within WSC Probe Response WscIE from AP if we can't find that by it's beacon.
        if((!Found)&&(pInBss->WSCInfoAtProbeRspLen > 0))
        {
            UCHAR   *pDataProbRsp   = (UCHAR *) &pInBss->WSCInfoAtProbeRsp;
            INT     LenProbRsp      = pInBss->WSCInfoAtProbeRspLen;

            pDataProbRsp += 6;
            LenProbRsp   -= 6;

            while (LenProbRsp > 0)
            {
                // Check for WSC IEs
                pWscIE = (PWSC_IE) pDataProbRsp;

                // Check for device password ID, PBC = 0x0004
                if (be2cpu16(pWscIE->Type) == WSC_IE_DEV_PASS_ID)
                {
                    // Found device password ID
                    DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data));
                    DBGPRINT(RT_DEBUG_TRACE, ("WscPBCModeAPSearch : DevicePasswordID = 0x%04x\n", DevicePasswordID));
                    if (DevicePasswordID == DEV_PASS_ID_PBC)    // Check for PBC value
                    {
                        // Found matching PBC AP in current list, add it into table and add the count
                        PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                        Found = TRUE;

                        DBGPRINT(RT_DEBUG_TRACE, ("PBC Found by Probe Response--> \n"));
                        DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                                pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                                pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
                    }
                }

                // UUID_E is optional for beacons, but mandatory for probe-request
                if (be2cpu16(pWscIE->Type) == WSC_IE_UUID_E)
                {
                    // Avoid error UUID-E storage from PIN mode
                    PlatformMoveMemory(&TmpUUID[0], (UCHAR *)(pData+4), 16);
                }
                    
                // Set the offset and look for PBC information
                // Since Type and Length are both short type, we need to offset 4, not 2
                pDataProbRsp += (be2cpu16(pWscIE->Length) + 4);
                LenProbRsp   -= (be2cpu16(pWscIE->Length) + 4);
            }
        }
        
        if (Found == TRUE)
        {
            // Store UUID
            PlatformMoveMemory(&UUID[OutTab->BssNr][0], &TmpUUID[0], 16);

            DBGPRINT(RT_DEBUG_TRACE, ("UUID-E= "));
            for(idx = 0; idx < 16; idx++)
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x  ", UUID[OutTab->BssNr][idx]));
            DBGPRINT(RT_DEBUG_TRACE, ("\n"));

            OutTab->BssNr++;
        }
    }

    // Select the WPS APs with the preferred PHY type.
    switch (pPort->StaCfg.WscControl.PreferrredWPSAPPhyType)
    {
        case PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST:
            {
                BOOLEAN b2DOT4GAP = FALSE; // 2.4G AP
                UCHAR candidateAPIndex = 0, numCandidateAP = 0;
                
                DBGPRINT(RT_DEBUG_TRACE, ("%s: PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST\n", __FUNCTION__));
                
                for (i = 0; i < OutTab->BssNr; i++)
                {
                    if (OutTab->BssEntry[i].Channel <= 14)
                    {
                        b2DOT4GAP = TRUE;
                        break;
                    }
                }

                // Choice 2.4G WPS APs only
                if (b2DOT4GAP == TRUE)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Choice 2.4G WPS APs only\n", __FUNCTION__));
                    
                    // Copy 2.4G WPS APs only.
                    for (i = 0; i < OutTab->BssNr; i++)
                    {
                        if (OutTab->BssEntry[i].Channel <= 14)
                        {
                            RtlMoveMemory(&OutTab->BssEntry[candidateAPIndex], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
                            RtlMoveMemory(&UUID[candidateAPIndex][0], &UUID[i][0], 16);
                            
                            candidateAPIndex++;
                            numCandidateAP++;
                        }
                    }

                    OutTab->BssNr = numCandidateAP;
                }
                else // Choice 5G WPS APs instead or there does not exist any PBC-enaabled AP
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Choice 5G WPS APs instead or there does not exist any PBC-enabled AP\n", __FUNCTION__));
                    // do nothing.
                }
            }
            break;

        case PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST:
            {
                BOOLEAN b5GAP = FALSE; // 5G AP
                UCHAR candidateAPIndex = 0, numCandidateAP = 0;
                
                DBGPRINT(RT_DEBUG_TRACE, ("%s: PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST\n", __FUNCTION__));
                
                for (i = 0; i < OutTab->BssNr; i++)
                {
                    if (OutTab->BssEntry[i].Channel > 14)
                    {
                        b5GAP = TRUE;
                        break;
                    }
                }

                // Choice 2.4G WPS APs only
                if (b5GAP == TRUE)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Choice 5G WPS APs only\n", __FUNCTION__));
                    
                    // Copy 2.4G WPS APs only.
                    for (i = 0; i < OutTab->BssNr; i++)
                    {
                        if (OutTab->BssEntry[i].Channel > 14)
                        {
                            RtlMoveMemory(&OutTab->BssEntry[candidateAPIndex], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
                            RtlMoveMemory(&UUID[candidateAPIndex][0], &UUID[i][0], 16);
                            
                            candidateAPIndex++;
                            numCandidateAP++;
                        }
                    }

                    OutTab->BssNr = numCandidateAP;
                }
                else // Choice 5G WPS APs instead or there does not exist any PBC-enaabled AP
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Choice 2.4G WPS APs instead or there does not exist any PBC-enabled AP\n", __FUNCTION__));
                    // do nothing.
                }
            }
            break;

        case PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION:
            {
                // do nothing.
                DBGPRINT(RT_DEBUG_TRACE, ("%s: PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION\n", __FUNCTION__));
            }
            break;

        default:
            {
                // do nothing.
            }
            break;
    }

    Duplicate = 0;
    // Check for concurrent dual band AP
    if (OutTab->BssNr > 1)
    {
        for (i = 0; i < OutTab->BssNr; i++)
        {
            for (j = i + 1; j < OutTab->BssNr; j++)
            {
                if ((MtkCompareMemory(&UUID[i][0], &UUID[j][0], 16) == 0) &&
                    (MtkCompareMemory(&UUID[i][0], zeros16, 16) != 0))
                {
                    // Same UUID at different channel, indicate concurrent AP
                    // We can indicate 1 AP only.
                    Duplicate++;                    
                }
            }
        }
    }
    // Set the finakl AP found, it might not be graceful, snce we might always select 2.4G band
    OutTab->BssNr -= Duplicate;
    
    DBGPRINT(RT_DEBUG_TRACE, ("WscPBCBssTableSort : Total %d PBC AP Found\n", OutTab->BssNr));
}

VOID WscPBCModeAPSearch(
    IN  PMP_ADAPTER   pAd, 
    IN PUCHAR pBSSID, 
    IN UCHAR Channel, 
    OUT BSS_TABLE       *OutTab, 
    OUT PULONG          pScanningTableIndex) 
{
    INT                 i, j, Len, idx;
    UCHAR               *pData;
    PBSS_ENTRY          pInBss, pOutBss;
    PWSC_IE             pWscIE;
    USHORT              DevicePasswordID;
    BOOLEAN             Found = FALSE;
    BOOLEAN             PBCing = FALSE;
    UCHAR               UUID[8][16];
    UCHAR               TmpUUID[16];
    UCHAR               Duplicate;
    UCHAR               zeros16[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ULONG               scanningTableIndex = 0;
    BOOLEAN             SelectReg = FALSE;
    UCHAR               APWscVersion = 0x10;

    // initialize Output BSS Table
    BssTableInit(OutTab);

    for (i = 0; i < pAd->ScanTab.BssNr; i++) 
    {
        pInBss  = (PBSS_ENTRY) &pAd->ScanTab.BssEntry[i];  // BSS entry for VarIE processing
        pOutBss = (PBSS_ENTRY)&OutTab->BssEntry[OutTab->BssNr];     // avaliable entry for new BSS list
        pData   = (UCHAR *) &pInBss->WSCInfoAtBeacons;
        Found   = FALSE;
        PBCing  = FALSE;
        PlatformZeroMemory(&TmpUUID[0], 16);
        APWscVersion = 0x10;

        // BSSID verification.
        if (MAC_ADDR_EQUAL(pInBss->Bssid, pBSSID))
        {
            // Do nothing.
            DBGPRINT(RT_DEBUG_TRACE, ("WscPBCModeAPSearch : Found desired BSSID\n"));
        }
        else
        {
            continue;
        }

        // Check the correct channel due to some dual-band APs have the same BSSID in different channel.
        if (pInBss->Channel == Channel)
        {
            // Correct channel.
            DBGPRINT(RT_DEBUG_TRACE, ("WscPBCModeAPSearch : Correct Channel \n"));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WscPBCModeAPSearch : wrong Channel  %d %d \n", Channel, pInBss->Channel));
            continue;
        }

        // 1. Check VarIE length
        if ((pInBss->WSCInfoAtBeaconsLen <= 0)&&(pInBss->WSCInfoAtProbeRspLen<= 0))
            continue;

        Len = pInBss->WSCInfoAtBeaconsLen;
        
        // 2. WSC IE in beacons, skip 6 bytes = 1 + 1 + 4
        pData += 6;
        Len   -= 6;

        // 3. Start to look the PBC type within WSC VarIE
        while (Len > 0)
        {
            // Check for WSC IEs
            pWscIE = (PWSC_IE) pData;
            if(be2cpu16(pWscIE->Type) == WSC_IE_VENDOR_EXT)
            {
                PUCHAR  pVendorExtType = pWscIE->Data;
                USHORT  VendorExtLen = be2cpu16(pWscIE->Length);
                UCHAR   VendorExtElemLen = 0;
                if (((*(pVendorExtType) * 256 + *(pVendorExtType+1)) * 256 + (*(pVendorExtType+2))) == WSC_SMI)
                {
                    VendorExtLen -= 3;      //WSC SMI
                    pVendorExtType += 3;    //WSC SMI
                    while(VendorExtLen > 2)
                    {
                        VendorExtElemLen = *(pVendorExtType+1);
                                            
                        switch(*pVendorExtType)
                        {
                            case WSC_IE_VERSION2:
                                APWscVersion = *(pVendorExtType + 2);
                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse AP Version2 type 0x%x!!\n", __FUNCTION__, APWscVersion));
                                break;
                            default:
                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse UNKNOWN type 0x%x !!\n", __FUNCTION__, *pVendorExtType));
                                break;
                        }
                        pVendorExtType += (VendorExtElemLen + 2);
                        VendorExtLen -= (VendorExtElemLen + 2);
                    }
                }

            }
#if 0
            if (be2cpu16(pWscIE->Type) == WSC_IE_VERSION2)
            {
                APWscVersion = *((UCHAR *) pWscIE->Data);
            }
#endif
            if (be2cpu16(pWscIE->Type) == WSC_IE_SEL_REG)
            {
                SelectReg = TRUE;
            }
            
            // Check for device password ID, PBC = 0x0004
            if (be2cpu16(pWscIE->Type) == WSC_IE_DEV_PASS_ID)
            {
                // Found device password ID
                DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data));
                DBGPRINT(RT_DEBUG_TRACE, ("WscPBCModeAPSearch : DevicePasswordID = 0x%04x\n", DevicePasswordID));
                if (DevicePasswordID == DEV_PASS_ID_PBC)
                {
                    // Found matching PBC AP in current list, add it into table and add the count
                    //PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                    //Found = TRUE;
                    PBCing = TRUE;

                    // Preserve the scanning table index (the PBC-enabled AP).
                    //scanningTableIndex = i;
                        
                    //DBGPRINT(RT_DEBUG_TRACE, ("PBC Found by Beacon--> \n"));
                    //DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                    //      pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                    //      pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
                }
            }

            // UUID_E is optional for beacons, but mandatory for probe-request
            if (be2cpu16(pWscIE->Type) == WSC_IE_UUID_E)
            {
                // Avoid error UUID-E storage from PIN mode
                PlatformMoveMemory(&TmpUUID[0], (UCHAR *)(pData+4), 16);
            }
                
            // Set the offset and look for PBC information
            // Since Type and Length are both short type, we need to offset 4, not 2
            pData += (be2cpu16(pWscIE->Length) + 4);
            Len   -= (be2cpu16(pWscIE->Length) + 4);
        }

        pInBss->WSCAPVersion= APWscVersion;
        if(((pInBss->WSCAPVersion <= 0x10)||
            ((pInBss->WSCAPVersion >= 0x20) && SelectReg))
            && PBCing)
        {
            Found = TRUE;
            PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
            scanningTableIndex = i;
            DBGPRINT(RT_DEBUG_TRACE, ("PBC Found by Beacon--> version:0x%02x\n", pInBss->WSCAPVersion));
            DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                            pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                            pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
        }
        
        //We also look the PBC type within WSC Probe Response WscIE from AP if we can't find that by its beacon.
        if((!Found)&&(pInBss->WSCInfoAtProbeRspLen > 0))
        {
            UCHAR   *pDataProbRsp   = (UCHAR *) &pInBss->WSCInfoAtProbeRsp;
            INT     LenProbRsp      = pInBss->WSCInfoAtProbeRspLen;

            pDataProbRsp += 6;
            LenProbRsp   -= 6;
            
            while (LenProbRsp > 0)
            {
                // Check for WSC IEs
                pWscIE = (PWSC_IE) pDataProbRsp;

                if(be2cpu16(pWscIE->Type) == WSC_IE_VENDOR_EXT)
                {
                    PUCHAR  pVendorExtType = pWscIE->Data;
                    USHORT  VendorExtLen = be2cpu16(pWscIE->Length);
                    UCHAR   VendorExtElemLen = 0;
                    if (((*(pVendorExtType) * 256 + *(pVendorExtType+1)) * 256 + (*(pVendorExtType+2))) == WSC_SMI)
                    {
                        VendorExtLen -= 3;      //WSC SMI
                        pVendorExtType += 3;    //WSC SMI
                        while(VendorExtLen > 2)
                        {
                            VendorExtElemLen = *(pVendorExtType+1);
                                                
                            switch(*pVendorExtType)
                            {
                                case WSC_IE_VERSION2:
                                    APWscVersion = *(pVendorExtType + 2);
                                    DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse AP Version2 type 0x%x!!\n", __FUNCTION__, APWscVersion));
                                    break;
                                default:
                                    DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse UNKNOWN type 0x%x !!\n", __FUNCTION__, *pVendorExtType));
                                    break;
                            }
                            pVendorExtType += (VendorExtElemLen + 2);
                            VendorExtLen -= (VendorExtElemLen + 2);
                        }
                    }

                }

#if 0
                if (be2cpu16(pWscIE->Type) == WSC_IE_VERSION2)
                {
                    APWscVersion = *((UCHAR *) pWscIE->Data);
                }
#endif
                if (be2cpu16(pWscIE->Type) == WSC_IE_SEL_REG)
                {
                    SelectReg = TRUE;
                }

                // Check for device password ID, PBC = 0x0004
                if (be2cpu16(pWscIE->Type) == WSC_IE_DEV_PASS_ID)
                {
                    // Found device password ID
                    DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data));
                    DBGPRINT(RT_DEBUG_TRACE, ("WscPBCModeAPSearch : DevicePasswordID = 0x%04x\n", DevicePasswordID));
                    if (DevicePasswordID == DEV_PASS_ID_PBC)    // Check for PBC value
                    {
                        // Found matching PBC AP in current list, add it into table and add the count
                        //PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                        //Found = TRUE;
                        PBCing = TRUE;

                        // Preserve the scanning table index (the PBC-enabled AP).
                        //scanningTableIndex = i;
                            
                        //DBGPRINT(RT_DEBUG_TRACE, ("PBC Found by Probe Response--> \n"));
                        //DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                        //      pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                        //      pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
                    }
                }

                // UUID_E is optional for beacons, but mandatory for probe-request
                if (be2cpu16(pWscIE->Type) == WSC_IE_UUID_E)
                {
                    // Avoid error UUID-E storage from PIN mode
                    PlatformMoveMemory(&TmpUUID[0], (UCHAR *)(pData+4), 16);
                }
                    
                // Set the offset and look for PBC information
                // Since Type and Length are both short type, we need to offset 4, not 2
                pDataProbRsp += (be2cpu16(pWscIE->Length) + 4);
                LenProbRsp   -= (be2cpu16(pWscIE->Length) + 4);
            }
            pInBss->WSCAPVersion = APWscVersion;
            if(((pInBss->WSCAPVersion <= 0x10)||
                ((pInBss->WSCAPVersion >= 0x20) && SelectReg))
                && PBCing)
            {
                Found = TRUE;
                PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                scanningTableIndex = i;
                DBGPRINT(RT_DEBUG_TRACE, ("PBC Found by Beacon--> version:0x%02x\n", pInBss->WSCAPVersion));
                DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                                pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                                pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
            }
        }
        
        if (Found == TRUE)
        {
            // Store UUID
            PlatformMoveMemory(&UUID[OutTab->BssNr][0], &TmpUUID[0], 16);

            DBGPRINT(RT_DEBUG_TRACE, ("UUID-E= "));
            for(idx = 0; idx < 16; idx++)
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x  ", UUID[OutTab->BssNr][idx]));
            DBGPRINT(RT_DEBUG_TRACE, ("\n"));

            OutTab->BssNr++;
        }
    }

    Duplicate = 0;
    // Check for concurrent dual band AP
    if (OutTab->BssNr > 1)
    {
        for (i = 0; i < OutTab->BssNr; i++)
        {
            for (j = i + 1; j < OutTab->BssNr; j++)
            {
                if ((MtkCompareMemory(&UUID[i][0], &UUID[j][0], 16) == 0) &&
                    (MtkCompareMemory(&UUID[i][0], zeros16, 16) != 0))
                {
                    // Same UUID at different channel, indicate concurrent AP
                    // We can indicate 1 AP only.
                    Duplicate++;
                }
            }
        }
    }
    // Set the finakl AP found, it might not be graceful, snce we might always select 2.4G band
    OutTab->BssNr -= Duplicate;

    if (OutTab->BssNr == 1)
    {
        *pScanningTableIndex = scanningTableIndex;
    }
    else
    {
        *pScanningTableIndex = BSS_NOT_FOUND;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("WscPBCModeAPSearch : Total %d PBC AP Found\n", OutTab->BssNr));
}

//
// Search the AP with PIN mode
//
VOID WscPINModeAPSearch(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pBSSID, 
    IN UCHAR Channel, 
    OUT BSS_TABLE       *OutTab, 
    OUT PULONG          pScanningTableIndex) 
{
    INT                 i, j, Len, idx;
    UCHAR               *pData;
    PBSS_ENTRY          pInBss, pOutBss;
    PWSC_IE             pWscIE;
    USHORT              DevicePasswordID;
    BOOLEAN             Found = FALSE;
    BOOLEAN             PINing = FALSE;
    UCHAR               UUID[8][16];
    UCHAR               TmpUUID[16];
    UCHAR               AuthorizedMACs[30];
    BOOLEAN             MyAuthorizedMAC = TRUE;
    BOOLEAN             SelectRegistrar = FALSE;
    UCHAR               Duplicate;
    UCHAR               zeros16[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ULONG               scanningTableIndex = 0;
    UCHAR               APWscVersion = 0x10;

    // initialize Output BSS Table
    BssTableInit(OutTab);

    for (i = 0; i < pAd->ScanTab.BssNr; i++) 
    {
        pInBss  = (PBSS_ENTRY) &pAd->ScanTab.BssEntry[i];  // BSS entry for VarIE processing
        pOutBss = (PBSS_ENTRY)&OutTab->BssEntry[OutTab->BssNr];     // avaliable entry for new BSS list
        pData   = (UCHAR *) &pInBss->WSCInfoAtBeacons;
        Found   = FALSE;
        PINing  = FALSE;
        PlatformZeroMemory(&TmpUUID[0], 16);
        PlatformZeroMemory(&AuthorizedMACs[0], 30);
        MyAuthorizedMAC = FALSE;
        SelectRegistrar = FALSE;
        APWscVersion = 0x10;

        // BSSID verification.
        if (MAC_ADDR_EQUAL(pInBss->Bssid, pBSSID))
        {
            // Do nothing.
            DBGPRINT(RT_DEBUG_TRACE, ("WscPINModeAPSearch : Found desired BSSID\n"));
        }
        else
        {
            continue;
        }

        // Check the correct channel due to some dual-band APs have the same BSSID in different channel.
        if (pInBss->Channel == Channel)
        {
            // Correct channel.
        }
        else
        {
            continue;
        }

        // 1. Check VarIE length
        if ((pInBss->WSCInfoAtBeaconsLen <= 0)&&(pInBss->WSCInfoAtProbeRspLen<= 0))
            continue;

        Len = pInBss->WSCInfoAtBeaconsLen;
            
        // 2. WSC IE in beacons, skip 6 bytes = 1(EID) + 1(Length) + 4(OUI)
        pData += 6;
        Len   -= 6;

        // 3. Start to look the PBC type within WSC VarIE
        while (Len > 0)
        {
            // Check for WSC IEs
            pWscIE = (PWSC_IE) pData;

            if(be2cpu16(pWscIE->Type) == WSC_IE_VENDOR_EXT)
            {
                PUCHAR  pVendorExtType = pWscIE->Data;
                USHORT  VendorExtLen = be2cpu16(pWscIE->Length);
                UCHAR   VendorExtElemLen = 0;
                if (((*(pVendorExtType) * 256 + *(pVendorExtType+1)) * 256 + (*(pVendorExtType+2))) == WSC_SMI)
                {
                    VendorExtLen -= 3;      //WSC SMI
                    pVendorExtType += 3;    //WSC SMI
                    while(VendorExtLen > 2)
                    {
                        VendorExtElemLen = *(pVendorExtType+1);
                                            
                        switch(*pVendorExtType)
                        {
                            case WSC_IE_VERSION2:
                                APWscVersion = *(pVendorExtType + 2);
                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse AP Version2 type 0x%x!!\n", __FUNCTION__, APWscVersion));
                                break;
                            default:
                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse UNKNOWN type 0x%x !!\n", __FUNCTION__, *pVendorExtType));
                                break;
                        }
                        pVendorExtType += (VendorExtElemLen + 2);
                        VendorExtLen -= (VendorExtElemLen + 2);
                    }
                }

            }
#if 0
            if (be2cpu16(pWscIE->Type) == WSC_IE_VERSION2)
            {
                APWscVersion = *((UCHAR *)pWscIE->Data);
            }
#endif
            if ((be2cpu16(pWscIE->Type) == WSC_IE_AUTHORIZED_MACS) && (pWscIE->Length <= 30))
            {
                PlatformMoveMemory(&AuthorizedMACs[0], (UCHAR *)(pData+4), pWscIE->Length);
                if(APWscVersion >= 0x20)
                {
                    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
                    {
                        for(i = 0; i < (pWscIE->Length/6); i++)
                        {
                            if(PlatformEqualMemory(pPort->CurrentAddress, &AuthorizedMACs[i*6], 6))
                            {
                                MyAuthorizedMAC = TRUE;
                                DBGPRINT(RT_DEBUG_TRACE, ("WscPINModeAPSearch: AuthorizedMAC in Beacon is ours!\n"));
                            }
                        }
                    }
                }
            }

            if (be2cpu16(pWscIE->Type) == WSC_IE_SEL_REG)
            {
                SelectRegistrar = be2cpu16(*((UCHAR *) &pWscIE->Data));
            }

            // Check for device password ID, PIN = 0x0000
            if (be2cpu16(pWscIE->Type) == WSC_IE_DEV_PASS_ID)
            {
                // Found device password ID
                DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data));
                DBGPRINT(RT_DEBUG_TRACE, ("WscPINModeAPSearch : DevicePasswordID = 0x%04x\n", DevicePasswordID));
                if ((DevicePasswordID == DEV_PASS_ID_PIN)
                    || (DevicePasswordID == DEV_PASS_ID_REG)
                    || (DevicePasswordID == DEV_PASS_ID_USER))  // Check for PIN value
                {
                    // Found matching PIN AP in current list, add it into table and add the count
                    //PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                    //Found = TRUE;
                    PINing = TRUE;

                    // Preserve the scanning table index (the PBC-enabled AP).
                    //scanningTableIndex = i;
                        
                    //DBGPRINT(RT_DEBUG_TRACE, ("PIN Found by Beacon--> \n"));
                    //DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                    //      pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                    //      pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
                }
            }

            // UUID_E is optional for beacons, but mandatory for probe-request
            if (be2cpu16(pWscIE->Type) == WSC_IE_UUID_E)
            {
                // Avoid error UUID-E storage from PIN mode
                PlatformMoveMemory(&TmpUUID[0], (UCHAR *)(pData+4), 16);
            }
                
            // Set the offset and look for PBC information
            // Since Type and Length are both short type, we need to offset 4, not 2
            pData += (be2cpu16(pWscIE->Length) + 4);
            Len   -= (be2cpu16(pWscIE->Length) + 4);
        }

        pInBss->WSCAPVersion = APWscVersion;
        if(((APWscVersion <= 0x10)||
            ((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE) && (pInBss->WSCAPVersion >= 0x20) && MyAuthorizedMAC && SelectRegistrar)) && 
            PINing)
        {
            // Found matching PIN AP in current list, add it into table and add the count
            Found = TRUE;
            PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
            scanningTableIndex = i;
            DBGPRINT(RT_DEBUG_TRACE, ("PIN AP Found by Beacon--> WscVersion:0x%02x\n", pInBss->WSCAPVersion));
            DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                            pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                            pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
        }
        
        //We also look the PIN type within WSC Probe Response WscIE from AP if we can't find that by it's beacon.
        if((!Found)&&(pInBss->WSCInfoAtProbeRspLen > 0))
        {
            UCHAR   *pDataProbRsp   = (UCHAR *) &pInBss->WSCInfoAtProbeRsp;
            INT     LenProbRsp      = pInBss->WSCInfoAtProbeRspLen;

            pDataProbRsp += 6;
            LenProbRsp   -= 6;
            
            while (LenProbRsp > 0)
            {
                // Check for WSC IEs
                pWscIE = (PWSC_IE) pDataProbRsp;

                if(be2cpu16(pWscIE->Type) == WSC_IE_VENDOR_EXT)
                {
                    PUCHAR  pVendorExtType = pWscIE->Data;
                    USHORT  VendorExtLen = be2cpu16(pWscIE->Length);
                    UCHAR   VendorExtElemLen = 0;
                    if (((*(pVendorExtType) * 256 + *(pVendorExtType+1)) * 256 + (*(pVendorExtType+2))) == WSC_SMI)
                    {
                        VendorExtLen -= 3;      //WSC SMI
                        pVendorExtType += 3;    //WSC SMI
                        while(VendorExtLen > 2)
                        {
                            VendorExtElemLen = *(pVendorExtType+1);
                                                
                            switch(*pVendorExtType)
                            {
                                case WSC_IE_VERSION2:
                                    APWscVersion = *(pVendorExtType + 2);
                                    DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse AP Version2 type 0x%x!!\n", __FUNCTION__, APWscVersion));
                                    break;
                                default:
                                    DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse UNKNOWN type 0x%x !!\n", __FUNCTION__, *pVendorExtType));
                                    break;
                            }
                            pVendorExtType += (VendorExtElemLen + 2);
                            VendorExtLen -= (VendorExtElemLen + 2);
                        }
                    }

                }
#if 0
                if (be2cpu16(pWscIE->Type) == WSC_IE_VERSION2)
                {
                    APWscVersion = *((UCHAR *)pWscIE->Data);
                }
#endif
                if ((be2cpu16(pWscIE->Type) == WSC_IE_AUTHORIZED_MACS) && (pWscIE->Length <= 30))
                {
                    //AuthorizedMACsLen = pWscIE->Length;
                    PlatformMoveMemory(&AuthorizedMACs[0], (UCHAR *)(pData+4), pWscIE->Length);
                    if(pInBss->WSCAPVersion >= 0x20)
                    {
                        if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
                        {
                            for(i = 0; i < (pWscIE->Length/6); i++)
                            {
                                if(PlatformEqualMemory(pPort->CurrentAddress, &AuthorizedMACs[i*6], 6))
                                {
                                    MyAuthorizedMAC = TRUE;
                                }
                            }
                        }
                    }
                }

                if (be2cpu16(pWscIE->Type) == WSC_IE_SEL_REG)
                {
                    SelectRegistrar = be2cpu16(*((UCHAR *) &pWscIE->Data));
                }
                
                // Check for device password ID, PIN = 0x0000
                if (be2cpu16(pWscIE->Type) == WSC_IE_DEV_PASS_ID)
                {
                    // Found device password ID
                    DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data));
                    DBGPRINT(RT_DEBUG_TRACE, ("WscPINModeAPSearch : DevicePasswordID = 0x%04x\n", DevicePasswordID));
                    if ((DevicePasswordID == DEV_PASS_ID_PIN)
                        || (DevicePasswordID == DEV_PASS_ID_REG)
                        || (DevicePasswordID == DEV_PASS_ID_USER))  // Check for PIN value
                    {
                        // Found matching PBC AP in current list, add it into table and add the count
                        //PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                        //Found = TRUE;
                        PINing = TRUE;

                        // Preserve the scanning table index (the PBC-enabled AP).
                        //scanningTableIndex = i;
                            
                        //DBGPRINT(RT_DEBUG_TRACE, ("PIN Found by Probe Response--> \n"));
                        //DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                        //      pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                        //      pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
                    }
                }

                // UUID_E is optional for beacons, but mandatory for probe-request
                if (be2cpu16(pWscIE->Type) == WSC_IE_UUID_E)
                {
                    // Avoid error UUID-E storage from PIN mode
                    PlatformMoveMemory(&TmpUUID[0], (UCHAR *)(pData+4), 16);
                }
                    
                // Set the offset and look for PBC information
                // Since Type and Length are both short type, we need to offset 4, not 2
                pDataProbRsp += (be2cpu16(pWscIE->Length) + 4);
                LenProbRsp   -= (be2cpu16(pWscIE->Length) + 4);
            }
            
            pInBss->WSCAPVersion = APWscVersion;
            if(((APWscVersion <= 0x10)||
                ((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE) && (pInBss->WSCAPVersion >= 0x20) && MyAuthorizedMAC && SelectRegistrar)) && 
                PINing)
            {
                // Found matching PIN AP in current list, add it into table and add the count
                Found = TRUE;
                PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
                scanningTableIndex = i;
                DBGPRINT(RT_DEBUG_TRACE, ("PIN Found by Beacon--> \n"));
                DBGPRINT(RT_DEBUG_TRACE, ("#%d  Bssid %02x:%02x:%02x:%02x:%02x:%02x; SSID = %c%c%c...\n", OutTab->BssNr,
                                pOutBss->Bssid[0], pOutBss->Bssid[1], pOutBss->Bssid[2], pOutBss->Bssid[3], pOutBss->Bssid[4], pOutBss->Bssid[5], 
                                pOutBss->Ssid[0], pOutBss->Ssid[1], pOutBss->Ssid[2]));
            }
        }
            
        if (Found == TRUE)
        {
            // Store UUID
            PlatformMoveMemory(&UUID[OutTab->BssNr][0], &TmpUUID[0], 16);

            DBGPRINT(RT_DEBUG_INFO, ("UUID-E= "));
            for(idx = 0; idx < 16; idx++)
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x  ", UUID[OutTab->BssNr][idx]));
            DBGPRINT(RT_DEBUG_INFO, ("\n"));

            OutTab->BssNr++;
        }
    }

    Duplicate = 0;
    // Check for concurrent dual band AP
    if (OutTab->BssNr > 1)
    {
        for (i = 0; i < OutTab->BssNr; i++)
        {
            for (j = i + 1; j < OutTab->BssNr; j++)
            {
                if ((MtkCompareMemory(&UUID[i][0], &UUID[j][0], 16) == 0) &&
                    (MtkCompareMemory(&UUID[i][0], zeros16, 16) != 0))
                {
                    // Same UUID at different channel, indicate concurrent AP
                    // We can indicate 1 AP only.
                    Duplicate++;
                }
            }
        }
    }
    // Set the finakl AP found, it might not be graceful, snce we might always select 2.4G band
    OutTab->BssNr -= Duplicate;

    if (OutTab->BssNr == 1) // The AP with the same BSSID.
    {
        *pScanningTableIndex = scanningTableIndex;
    }
    else
    {
        *pScanningTableIndex = BSS_NOT_FOUND;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("WscPINModeAPSearch : Total %d PIN AP Found\n", OutTab->BssNr));
    if (OutTab->BssNr > 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("pAd->StaCfg.bAutoReconnect %d  \n", pAd->StaCfg.bAutoReconnect));
        DBGPRINT(RT_DEBUG_TRACE, (".AutoReconnectSsidLen %d => %c %c %c %c ...  \n", pAd->MlmeAux.AutoReconnectSsidLen, 
            pAd->MlmeAux.AutoReconnectSsid[0], pAd->MlmeAux.AutoReconnectSsid[1],pAd->MlmeAux.AutoReconnectSsid[2],pAd->MlmeAux.AutoReconnectSsid[3]));
        DBGPRINT(RT_DEBUG_TRACE, ("pAd->ScanTab.BssNr %d  \n", pAd->ScanTab.BssNr));
        DBGPRINT(RT_DEBUG_TRACE, ("CntlMachine.CurrState =   %d  \n", pAd->PortList[0]->Mlme.CntlMachine.CurrState));
        DBGPRINT(RT_DEBUG_TRACE, ("bRoamingAfterResume =   %d  \n", pAd->PortList[0]->bRoamingAfterResume));
        DBGPRINT(RT_DEBUG_TRACE, ("CurrentBssEntry.LastBeaconRxTime =   %d  \n", pAd->PortList[0]->CurrentBssEntry.LastBeaconRxTime));
    }
}

//
// Determine this is WPS AP or not.
//
VOID WscCheckWPSAP(
    IN OUT PBSS_ENTRY pBssEntry) 
{
    INT Len;
    PUCHAR pData = (PUCHAR)&pBssEntry->VarIEs;
    PBEACON_EID_STRUCT pEid;
    PWSC_IE pWscIE = NULL;
    BOOLEAN bTHome303VModelName = FALSE, bTHome303VModelNumber = FALSE;
    BOOLEAN bTHome303VInConfiguredState = FALSE, bTHome303VInPINMode = FALSE;

    pBssEntry->bWPSAP = FALSE;

    pBssEntry->WSCAPVersion = 0x10;
    if (pBssEntry->VarIELen == 0)
    {
        return;
    }

    // WPS IE - 0xdd xx 00 50 f2 04
    Len = pBssEntry->VarIELen;
    while (Len > 0)
    {
        pEid = (PBEACON_EID_STRUCT) pData;
        
        // Skip this IE if the Element ID is not 0xDD.
        if (pEid->Eid != IE_WFA_WSC)
        {
            // Examine next IE.
            pData += (pEid->Len + 2);
            Len -= (pEid->Len + 2);
            continue;
        }
        else
        {
            // This is a WPS AP.
            if (be2cpu32(*(ULONG *)&pEid->Octet) == WSC_OUI)
            {
                DBGPRINT(RT_DEBUG_INFO, ("%s: This is a WPS AP (BSSID = %02X:%02X:%02X:%02X:%02X:%02X; SSID = %c%c%c)\n", 
                    __FUNCTION__, 
                    pBssEntry->Bssid[0], pBssEntry->Bssid[1], pBssEntry->Bssid[2], 
                    pBssEntry->Bssid[3], pBssEntry->Bssid[4], pBssEntry->Bssid[5], 
                    pBssEntry->Ssid[0], pBssEntry->Ssid[1], pBssEntry->Ssid[2]));

                pBssEntry->bWPSAP = TRUE;

                // This is a WPS AP.
                // Skip the first six bytes, that is Element ID (0xDD), Length (1 byte) and OUI (0x00, 0x50, 0xF2 and 0x04).
                pData += 6;
                Len -= 6;

                bTHome303VModelName = FALSE;
                bTHome303VModelNumber = FALSE;
                bTHome303VInConfiguredState = FALSE;
                bTHome303VInPINMode = FALSE;
    
                // Parse the WPS attributes within the WPS IE.
                while (Len > 0)
                {
                    // Parse each WPS attribute.
                    pWscIE = (PWSC_IE)(pData);
                    
                    if(be2cpu16(pWscIE->Type) == WSC_IE_VENDOR_EXT)
                    {
                        PUCHAR  pVendorExtType = pWscIE->Data;
                        USHORT  VendorExtLen = be2cpu16(pWscIE->Length);
                        UCHAR   VendorExtElemLen = 0;
                        if (((*(pVendorExtType) * 256 + *(pVendorExtType+1)) * 256 + (*(pVendorExtType+2))) == WSC_SMI)
                        {
                            VendorExtLen -= 3;      //WSC SMI
                            pVendorExtType += 3;    //WSC SMI
                            while(VendorExtLen > 2)
                            {
                                VendorExtElemLen = *(pVendorExtType+1);
                                                    
                                switch(*pVendorExtType)
                                {
                                    case WSC_IE_VERSION2:
                                        pBssEntry->WSCAPVersion = *(pVendorExtType + 2);
                                        DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse AP Version2 type 0x%x!!\n", __FUNCTION__, pBssEntry->WSCAPVersion));
                                        break;
                                    default:
                                        DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse UNKNOWN type 0x%x !!\n", __FUNCTION__, *pVendorExtType));
                                        break;
                                }
                                pVendorExtType += (VendorExtElemLen + 2);
                                VendorExtLen -= (VendorExtElemLen + 2);
                            }
                        }
                    }
                
                    switch (be2cpu16(pWscIE->Type)) // WPS attibute.
                    {
                        // T-Home 303V model name?
                        case WSC_IE_MODEL_NAME: // Model name.
                        {                   
                            if (be2cpu16(pWscIE->Length) == THOME_303V_MODEL_NAME_LENGTH)
                            {                           
                                if (PlatformEqualMemory((PVOID)(pWscIE->Data), 
                                                 (PVOID)(THOME_303V_MODEL_NAME), 
                                                 THOME_303V_MODEL_NAME_LENGTH) == TRUE)
                                {
                                    bTHome303VModelName = TRUE;
                                    DBGPRINT(RT_DEBUG_INFO, ("THOME_303V_MODEL_NAME\n"));
                                }
                            }

                            break;
                        }

                        // T-Home 303V model number?
                        case WSC_IE_MODEL_NO: // Model number.
                        {
                            // Model number = 1.04.000
                            if (be2cpu16(pWscIE->Length) == THOME_303V_MODEL_NUMBER_LENGTH_000)
                            {
                                if (PlatformEqualMemory((PVOID)(pWscIE->Data), 
                                                 (PVOID)(THOME_303V_MODEL_NUMBER_000), 
                                                 THOME_303V_MODEL_NUMBER_LENGTH_000) == TRUE)
                                {
                                    bTHome303VModelNumber= TRUE;
                                }
                            }

                            // Model number = 1.04.001
                            if (be2cpu16(pWscIE->Length) == THOME_303V_MODEL_NUMBER_LENGTH_001)
                            {
                                if (PlatformEqualMemory((PVOID)(pWscIE->Data), 
                                                 (PVOID)(THOME_303V_MODEL_NUMBER_001), 
                                                 THOME_303V_MODEL_NUMBER_LENGTH_001) == TRUE)
                                {
                                    bTHome303VModelNumber= TRUE;
                                }
                            }

                            break;
                        }

                        // T-Home 303V WPS state?
                        case WSC_IE_STATE: // WPS state.
                        {
                            if ((UCHAR)(*pWscIE->Data) == WSC_APSTATE_CONFIGURED) // Configured state.
                            {
                                bTHome303VInConfiguredState = TRUE;
                                DBGPRINT(RT_DEBUG_INFO, ("WSC_APSTATE_CONFIGURED\n"));
                            }
                            break;
                        }

                        // Use PIN mode?
                        case WSC_IE_DEV_PASS_ID: // Device password ID.
                        {
                            if (be2cpu16(*((USHORT *) &pWscIE->Data)) == DEV_PASS_ID_PIN)
                            {
                                bTHome303VInPINMode = TRUE;
                                DBGPRINT(RT_DEBUG_INFO, ("WSC_IE_DEV_PASS_ID\n"));
                            }

                            break;
                        }                       

                        default:
                        {
                            // Not important WPS attribute and skip them.
                            break;
                        }
                    }

                    // Parse another WPS attibutes.
                    // Note that the WPS attibute type and the WPS attribute length are both short type (2 + 2 = 4 bytes).
                    pData += (be2cpu16(pWscIE->Length) + 4);
                    Len -= (be2cpu16(pWscIE->Length) + 4);
                }

                // This is the T-Home W 303V WPS AP and it uses PIN mode in the configured state.
                if ((bTHome303VModelName == TRUE) && 
                    (bTHome303VModelNumber == TRUE) && 
                    (bTHome303VInConfiguredState == TRUE) && 
                    (bTHome303VInPINMode == TRUE))
                {
                    pBssEntry->bTHome303V_ConfiguredPINMode = TRUE;

                    DBGPRINT(RT_DEBUG_INFO, ("%s: This is the T-Home W 303V WPS AP and it uses PIN mode in the configured state. (BSSID = %02X:%02X:%02X:%02X:%02X:%02X; SSID = %c%c%c)\n", 
                        __FUNCTION__, 
                        pBssEntry->Bssid[0], pBssEntry->Bssid[1], pBssEntry->Bssid[2], 
                        pBssEntry->Bssid[3], pBssEntry->Bssid[4], pBssEntry->Bssid[5], 
                        pBssEntry->Ssid[0], pBssEntry->Ssid[1], pBssEntry->Ssid[2]));
                }
                
                break;
            }
            else
            {
                // Examine next IE.
                pData += (pEid->Len + 2);
                Len -= (pEid->Len + 2);
                continue;
            }
        }
    }   
}

/*
    ========================================================================
    
    Routine Description:
        Init WSC MAC header

    Arguments:
        pAd    Pointer to our adapter
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/
VOID    WscMacHeaderInit(
    IN      PMP_ADAPTER   pAd, 
    IN      PMP_PORT      pPort,
    IN OUT  PHEADER_802_11  Hdr, 
    IN      PUCHAR          pAddr1) 
{
    PlatformZeroMemory(Hdr, sizeof(HEADER_802_11));
    Hdr->FC.Type = BTYPE_DATA;
    Hdr->FC.ToDs = 1;
    
     // Addr1: DA, Addr2: BSSID, Addr3: SA
    
    COPY_MAC_ADDR(&Hdr->Addr1, pAddr1);
    COPY_MAC_ADDR(&Hdr->Addr2, &pPort->CurrentAddress);
    COPY_MAC_ADDR(&Hdr->Addr3, &pPort->PortCfg.Bssid);
    Hdr->Sequence = pAd->pTxCfg->Sequence; 
    if (IS_P2P_REGISTRA(pPort))
    {
        Hdr->FC.ToDs = 0;
        Hdr->FC.FrDs = 1;
    }
}

/*
    ========================================================================
    
    Routine Description:
        WSC_STATE_START action

    Arguments:
        pAd         - NIC Adapter pointer
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after link up
        1. Change the correct parameters
        2. Send EAPOL - START
        
    ========================================================================
*/
VOID WscStartAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort)
{
    HEADER_802_11       Header_802_11;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0;
    IEEE8021X_FRAME     Packet;
    UCHAR               *OutBuffer = NULL;
    NDIS_STATUS         NStatus;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,0x88,0x8e};
    BOOLEAN             Cancelled;
    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscStartAction\n"));

    // 1. Change the authentication to open and encryption to none if necessary.

    // 2. Send EAPOL start
    
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    WscMacHeaderInit(pAd, pPort, &Header_802_11, pPort->PortCfg.Bssid);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;
    
    // Zero message 2 body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLStart;
    Packet.Length  = cpu2be16(0);
    
    // Out buffer for transmitting message 2        
    NStatus = MlmeAllocateMemory(pAd, (PVOID)&OutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    FrameLen = 0;
    // Make  Transmitting frame
    MakeOutgoingFrame(OutBuffer, &FrameLen, sizeof(HEADER_802_11), &Header_802_11,
        sizeof(EAPHEAD), EAPHEAD, 
        4, &Packet,
        END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, OutBuffer, FrameLen);
    DBGPRINT(RT_DEBUG_TRACE, ("Send EAPOL-START size = %d\n", FrameLen));

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAPOL_START_SENT;

    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
    DBGPRINT(RT_DEBUG_TRACE, ("Startaction cancel PBCTimer\n"));

    // set new EAP Rx timer
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WSC_PER_MSG_TIMEOUT);
    DBGPRINT(RT_DEBUG_TRACE, ("Startaction set RxTimer\n"));
    
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscStartAction\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Initialize WSC registrar pair data structure

    Arguments:
        pAd    - NIC Adapter pointer
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    WscInitRegistrarPair(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort)
{
    PWSC_REG_DATA       pReg;
    PWSC_DEV_INFO       pDevInfo;       
    INT                 idx, DH_Len;
    BOOLEAN             bCustomizeSet = FALSE;
    UCHAR               AllZeros40[40];
    EEPROM_NIC_CONFIG2_STRUC    NicConfig2;
    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscInitRegistrarPair\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    //0. Zero the data structure first
    PlatformZeroMemory(&pPort->StaCfg.WscControl.RegData, sizeof(WSC_REG_DATA));

    if(pPort->StaCfg.WscControl.AsExtreg == FALSE)
    {
        PlatformZeroMemory(&pPort->StaCfg.WscControl.WscPbcEnrTab, sizeof(WSCPBCENROLLEE)*PBC_ENR_TAB_SIZE);
        pPort->StaCfg.WscControl.WscPbcEnrCount = 0;
    }

    // Role play, Enrollee or Registrar
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
        pDevInfo = (PWSC_DEV_INFO) &pReg->EnrolleeInfo;
    else if ((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR) ||
             (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP))
        pDevInfo = (PWSC_DEV_INFO) &pReg->RegistrarInfo;
    else
        return;

    // Get WPS device information from UI
    PlatformZeroMemory(AllZeros40, 40);
    if (!PlatformEqualMemory(&pPort->StaCfg.WscControl.CustomizeDevInfo, AllZeros40, 40))
    {
        bCustomizeSet = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, ("This is Customized Info!!!!\n"));
    }

    // Init EAP token
    pPort->StaCfg.WscControl.EapId = 1;

    //
    // Fill all the device information.
    //

    // 1. Version
    pDevInfo->Version = WSC_VERSION;
    DBGPRINT(RT_DEBUG_TRACE, ("%s WSCVersion2:0x%x\n", __FUNCTION__, pAd->StaCfg.WSCVersion2));
    if(pAd->StaCfg.WSCVersion2 >= 0x20)
    {
        pDevInfo->Version2 = pAd->StaCfg.WSCVersion2;  //WSC version after 2.0.
    }
    else
    {
        pDevInfo->Version2 = WSC_VERSION;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("Use WSC Version:0x%x\n", pDevInfo->Version2));   

    // 2. UUID : "RalinkWPS-"+ MACAddr
    if (bCustomizeSet)
        PlatformMoveMemory(pDevInfo->Uuid, pPort->StaCfg.WscControl.CustomizeDevInfo.Uuid, 16);
    else
    {
        PlatformMoveMemory(&pDevInfo->Uuid[0], Wsc_Uuid, 10);
        PlatformMoveMemory(&pDevInfo->Uuid[10], pPort->CurrentAddress, MAC_ADDR_LEN);
    }

    
    // 3. MAC address
    PlatformMoveMemory(pDevInfo->MacAddr, pPort->CurrentAddress, 6);

    // 4. Device Name
    if (bCustomizeSet)
        PlatformMoveMemory(pDevInfo->DeviceName, pPort->StaCfg.WscControl.CustomizeDevInfo.DeviceName, 32);
    else if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
    {
        pDevInfo->DeviceNameLen = pAd->WpsCfg.WPSDevNameLen;
        PlatformMoveMemory(pDevInfo->DeviceName, pAd->WpsCfg.WPSDevName, pAd->WpsCfg.WPSDevNameLen);
    }
    else
    {
        pDevInfo->DeviceNameLen = sizeof(Wsc_Dev_Name_R);
        PlatformMoveMemory(pDevInfo->DeviceName, Wsc_Dev_Name_R, sizeof(Wsc_Dev_Name_R));
    }
    
    // 5. Manufacture
    PlatformMoveMemory(pDevInfo->Manufacturer, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.Manufacturer : Wsc_Manufacture), 
        (bCustomizeSet ? 64 : sizeof(Wsc_Manufacture)));

    // 6. Model Name
    PlatformMoveMemory(pDevInfo->ModelName, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.ModelName : Wsc_Model_Name), 
        (bCustomizeSet ? 32 : sizeof(Wsc_Model_Name)));
    
    // 7. Model Number
    PlatformMoveMemory(pDevInfo->ModelNumber, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.ModelNumber : Wsc_Model_Number), 
        (bCustomizeSet ? 32 : sizeof(Wsc_Model_Number)));   
    
    // 8. Serial Number
    PlatformMoveMemory(pDevInfo->SerialNumber, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.SerialNumber : Wsc_Model_Serial), 
        (bCustomizeSet ? 32 : sizeof(Wsc_Model_Serial)));   
    
    // 9. Authentication Type Flags
    pDevInfo->AuthTypeFlags = cpu2be16(0x0023); // Open(=0x1), WPAPSK(=0x2), WPA2PSK(=0x20)
    
    // 10. Encryption Type Flags
    pDevInfo->EncrTypeFlags = cpu2be16(0x0009); // None(=0x1), WEP(=0x2), TKIP(=0x4), AES(=0x8) 

    // 11. Connection Type Flag
    pDevInfo->ConnTypeFlags = 0x01;             // ESS

    // 12. Associate state
    pDevInfo->AssocState = cpu2be16(0x0000);        // Not associated

    // 13. Configure Error
    pDevInfo->ConfigError = cpu2be16(WSC_ERROR_NO_ERROR);   // No error

    // 14. OS Version
    pDevInfo->OsVersion  = cpu2be32(0x80000000);    // first bit must be 1
    
    // 15. RF Band
    pDevInfo->RfBand = 0x00;
    if ((pPort->CommonCfg.PhyMode == PHY_11A) || (pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED) ||
        (pPort->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || (pPort->CommonCfg.PhyMode == PHY_11AN_MIXED) ||
        (pPort->CommonCfg.PhyMode == PHY_11AGN_MIXED) || (pPort->CommonCfg.PhyMode == PHY_11VHT))
        pDevInfo->RfBand |= 0x02;           // 5.0G
    if ((pPort->CommonCfg.PhyMode != PHY_11A) && (pPort->CommonCfg.PhyMode != PHY_11AN_MIXED))
        pDevInfo->RfBand |= 0x01;           // 2.4G
    
    // 16. Config Method
    // TODO: Might need to differ between PBC and PIN mode
    pDevInfo->ConfigMethods  = cpu2be16(0x008c);    // Label, Display, PBC
    NicConfig2.word = pAd->HwCfg.EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];
    if(NicConfig2.field.EnableWPSPBC)
    {
        pDevInfo->ConfigMethods  = cpu2be16(0x268c);    // Label, Display, Virtul/Physical PBC, Virtual Display PIN
    }
    else
    {
        pDevInfo->ConfigMethods  = cpu2be16(0x228c);    // Label, Display, Virtul/Physical PBC, Virtual Display PIN
    }

    // 17. Wi-Fi Protected Setup State
    pDevInfo->ScState  = 0x01;      // Not Configured, used on AP's beacon and probe response

    // 18. Device Password ID
    if  (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE)   // PBC
    {
        pDevInfo->DevPwdId = cpu2be16(DEV_PASS_ID_PBC);
    }
    else if  (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_USER_SPEC)    // PBC
    {
        pDevInfo->DevPwdId  = cpu2be16(DEV_PASS_ID_USER);       
    }
    else if  (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_REGISTRA_SPEC)    // PBC
    {
        pDevInfo->DevPwdId  = cpu2be16(DEV_PASS_ID_REG);        
    }
    else    // PIN mode
    {
        pDevInfo->DevPwdId  = cpu2be16(DEV_PASS_ID_PIN);
    }


    // 19. SSID
#pragma prefast(suppress: __WARNING_UNVALIDATED_PARAM, "pPort not possible on buffer overrun")   
    PlatformMoveMemory(pDevInfo->Ssid, pPort->PortCfg.Ssid, min(pPort->PortCfg.SsidLen, 32));

    // 20. Primary Device Type
    PlatformMoveMemory(pReg->EnrolleeInfo.PriDeviceType, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.PriDeviceType : Wsc_Pri_Dev_Type), 8);
    PlatformMoveMemory(pReg->RegistrarInfo.PriDeviceType, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.PriDeviceType : Wsc_Pri_Dev_Type), 8);
    
    // 21. Wi-Fi Protected Setup State
    if (IS_P2P_REGISTRA(pPort) || pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO)
        pDevInfo->ScState = (pPort->P2PCfg.bConfiguredAP ? WSC_APSTATE_CONFIGURED : WSC_APSTATE_UNCONFIGURED);

    //
    // Other enrollee/registrar controlled data in RegData structure ==============
    
    //  192 random bytes for DH key generation
    for (idx = 0; idx < 192; idx++)
        pReg->DHRandom[idx] = RandomByte(pAd);
    
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
    {
        // Enrollee Nonce, first generate and save to Wsc Control Block
        for (idx = 0; idx < 16; idx++)
            pReg->EnrolleeNonce[idx] = RandomByte(pAd);
        
        // Generate DH Public Key
        GenerateDHPublicKey(&pReg->DHRandom[0], 192, &pReg->Pke[0], &DH_Len);
    
        // Enrollee 16 byte E-S1 generation
        for (idx = 0; idx < 16; idx++)
            pReg->Es1[idx] = RandomByte(pAd);
    
        // Enrollee 16 byte E-S2 generation
        for (idx = 0; idx < 16; idx++)
            pReg->Es2[idx] = RandomByte(pAd);
    
    }
    else    // Registrar
    {
        // Registrar Nonce, first generate and save to Wsc Control Block
        for (idx = 0; idx < 16; idx++)
            pReg->RegistrarNonce[idx] = RandomByte(pAd);
    
        // Generate DH Public Key
        GenerateDHPublicKey(&pReg->DHRandom[0], 192, &pReg->Pkr[0], &DH_Len);
    
        // Enrollee 16 byte R-S1 generation
        for (idx = 0; idx < 16; idx++)
            pReg->Rs1[idx] = RandomByte(pAd);
    
        // Enrollee 16 byte R-S2 generation
        for (idx = 0; idx < 16; idx++)
            pReg->Rs2[idx] = RandomByte(pAd);

        // Reset flags
        pPort->StaCfg.WscControl.bConfiguredAP = FALSE;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscInitRegistrarPair\n"));    
}

VOID    WscSetDevInfo(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT      pPort,
    IN  PWSC_DEV_INFO   pDevInfo)
{
    BOOLEAN             bCustomizeSet = FALSE;
    UCHAR               AllZeros40[40];
    
    DBGPRINT(RT_DEBUG_INFO, ("-----> WscSetDevInfo\n"));

    // Get WPS device information from UI
    PlatformZeroMemory(AllZeros40, 40);
    if (!PlatformEqualMemory(&pPort->StaCfg.WscControl.CustomizeDevInfo, AllZeros40, 40))
        bCustomizeSet = TRUE;

    //
    // Fill all the device information.
    //

    // 1. Version
    pDevInfo->Version = WSC_VERSION;

    // 2. UUID : "RalinkWPS-"+ MACAddr
    if (bCustomizeSet)
        PlatformMoveMemory(pDevInfo->Uuid, pPort->StaCfg.WscControl.CustomizeDevInfo.Uuid, 16);
    else
    {
        PlatformMoveMemory(&pDevInfo->Uuid[0], Wsc_Uuid, 10);
        PlatformMoveMemory(&pDevInfo->Uuid[10], pPort->CurrentAddress, MAC_ADDR_LEN);
    }

    
    // 3. MAC address
    PlatformMoveMemory(pDevInfo->MacAddr, pPort->CurrentAddress, 6);

    // 4. Device Name
    if (bCustomizeSet)
        PlatformMoveMemory(pDevInfo->DeviceName, pPort->StaCfg.WscControl.CustomizeDevInfo.DeviceName, 32);
    else if (P2P_ON(pPort))
        PlatformMoveMemory(pDevInfo->DeviceName, pPort->P2PCfg.DeviceName, 32);  
    else if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
        PlatformMoveMemory(pDevInfo->DeviceName, pAd->WpsCfg.WPSDevName, pAd->WpsCfg.WPSDevNameLen);
    else
        PlatformMoveMemory(pDevInfo->DeviceName, Wsc_Dev_Name_R, sizeof(Wsc_Dev_Name_R));
    
    // 5. Manufacture
    PlatformMoveMemory(pDevInfo->Manufacturer, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.Manufacturer : Wsc_Manufacture), 
        (bCustomizeSet ? 64 : sizeof(Wsc_Manufacture)));

    // 6. Model Name
    PlatformMoveMemory(pDevInfo->ModelName, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.ModelName : Wsc_Model_Name), 
        (bCustomizeSet ? 32 : sizeof(Wsc_Model_Name)));
    
    // 7. Model Number
    PlatformMoveMemory(pDevInfo->ModelNumber, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.ModelNumber : Wsc_Model_Number), 
        (bCustomizeSet ? 32 : sizeof(Wsc_Model_Number)));   
    
    // 8. Serial Number
    PlatformMoveMemory(pDevInfo->SerialNumber, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.SerialNumber : Wsc_Model_Serial), 
        (bCustomizeSet ? 32 : sizeof(Wsc_Model_Serial)));   
/*  
    // 9. Authentication Type Flags
    pDevInfo->AuthTypeFlags = cpu2be16(0x0023); // Open(=0x1), WPAPSK(=0x2), WPA2PSK(=0x20)
    
    // 10. Encryption Type Flags
    pDevInfo->EncrTypeFlags = cpu2be16(0x000F); // None(=0x1), WEP(=0x2), TKIP(=0x4), AES(=0x8) 

    // 11. Connection Type Flag
    pDevInfo->ConnTypeFlags = 0x01;             // ESS

    // 12. Associate state
    pDevInfo->AssocState = cpu2be16(0x0000);        // Not associated

    // 13. Configure Error
    pDevInfo->ConfigError = cpu2be16(WSC_ERROR_NO_ERROR);   // No error

    // 14. OS Version
    pDevInfo->OsVersion  = cpu2be32(0x80000000);    // first bit must be 1
*/  
    // 15. RF Band
    pDevInfo->RfBand = 0x00;
    if ((pPort->CommonCfg.PhyMode == PHY_11A) || (pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED) ||
        (pPort->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || (pPort->CommonCfg.PhyMode == PHY_11AN_MIXED) ||
        (pPort->CommonCfg.PhyMode == PHY_11AGN_MIXED) || (pPort->CommonCfg.PhyMode == PHY_11VHT))
        pDevInfo->RfBand |= 0x02;           // 5.0G
    if ((pPort->CommonCfg.PhyMode != PHY_11A) && (pPort->CommonCfg.PhyMode != PHY_11AN_MIXED))
        pDevInfo->RfBand |= 0x01;           // 2.4G
/*  
    // 16. Config Method
    // TODO: Might need to differ between PBC and PIN mode
    pDevInfo->ConfigMethods  = cpu2be16(0x18c); // Label, Display, PBC

    // 17. Wi-Fi Protected Setup State
    pDevInfo->ScState  = 0x01;      // Not Configured, used on AP's beacon and probe response

    // 18. Device Password ID
    if  (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE)   // PBC
    {
        pDevInfo->DevPwdId = cpu2be16(DEV_PASS_ID_PBC);
    }
    else if  (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_USER_SPEC)    // PBC
    {
        pDevInfo->DevPwdId  = cpu2be16(DEV_PASS_ID_USER);       
    }
    else    // PIN mode
    {
        pDevInfo->DevPwdId  = cpu2be16(DEV_PASS_ID_PIN);        
    }


    // 19. SSID
#pragma prefast(suppress: __WARNING_UNVALIDATED_PARAM, "pPort not possible on buffer overrun")   
    PlatformMoveMemory(pDevInfo->Ssid, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
*/
    // 20. Primary Device Type
    PlatformMoveMemory(pDevInfo->PriDeviceType, 
        (bCustomizeSet ? pPort->StaCfg.WscControl.CustomizeDevInfo.PriDeviceType : Wsc_Pri_Dev_Type), 8);
/*
    // 21. Wi-Fi Protected Setup State
    if (IS_P2P_REGISTRA(pPort))
        pDevInfo->ScState = (pPort->P2PCfg.bConfiguredAP ? WSC_APSTATE_CONFIGURED : WSC_APSTATE_UNCONFIGURED);
*/
    
    DBGPRINT(RT_DEBUG_INFO, ("<----- WscSetDevInfo\n"));    
}

VOID    WscFragmentedEAP(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  UCHAR               OpCode,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  BOOLEAN         bFirstFrag)
{   
    DBGPRINT(RT_DEBUG_TRACE, ("------> WscFragmentedEAP, FragRemainLen:%d\n", FragRemainLen));  
    
    if(FragRemainLen <= 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WscFragmentedEAP -- No Fragmented Packets need to send\n")); 
        return;
    }
    else if(FragRemainLen > 128)
    {
        UCHAR   Cut = 128;
        if(bFirstFrag)
        {
            Cut -= 2;
        }
        WscMakeEAPFrame(pAd, pPort, Id, OpCode, pFragData, Cut, pElem, TRUE, bFirstFrag);
        
        pFragData += Cut;
        FragRemainLen -= Cut;
        DBGPRINT(RT_DEBUG_TRACE, ("WscFragmentedEAP -- Send One Fragmented Packets (Not Last one)! FragRemain:%d\n", FragRemainLen));   
    }
    else
    {
        WscMakeEAPFrame(pAd, pPort, Id, OpCode, pFragData, FragRemainLen, pElem, FALSE, FALSE);
        FragRemainLen = 0;
        pFragData += FragRemainLen;
        DBGPRINT(RT_DEBUG_TRACE, ("WscFragmentedEAP -- Send Last one Fragmented Packet!\n"));   
    }
}

/*
    ========================================================================
    
    Routine Description:
        Send out Message with WSC EAP(Extensible Authentication Protocol)

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        OpCode      - WSC Operation Code
        pData       - pointer to EAP Packet Body
        Len         - Length of EAP body
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    WscMakeEAPFrame(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  UCHAR               OpCode,
    IN  PUCHAR              pData,
    IN  INT                 Len,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  BOOLEAN             bFrag,
    IN  BOOLEAN             bFirstFrag)
{   

    // Inb-EAP Message
    HEADER_802_11       Header_802_11;
    USHORT              Length;
    IEEE8021X_FRAME     Ieee_8021x;
    EAP_FRAME           EapFrame;
    WSC_FRAME           WscFrame;
    UCHAR               *OutBuffer = NULL;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0;
    NDIS_STATUS         NStatus;
    PHEADER_802_11      pHeader_802_11;
    UCHAR               addr2[6];
    UCHAR               LF = 0;
    UCHAR               LF2Byte[2] ={0};
    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscMakeEAPFrame\n"));

    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    pHeader_802_11 = (PHEADER_802_11)&pElem->Msg[0];
    PlatformMoveMemory(addr2, pHeader_802_11->Addr2, 6);
    // Send back to requester. Not necessarliy it's my bssid. because I might be GO acting like a AP.. 
    WscMacHeaderInit(pAd, pPort, &Header_802_11, addr2);


    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;

    // Length = EAP + WSC_Frame + Payload
    Length = (USHORT)(sizeof(EAP_FRAME) + sizeof(WSC_FRAME) + Len);
    
    // Zero 802.1x body
    PlatformZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
    Ieee_8021x.Version = EAPOL_VER;
    Ieee_8021x.Type    = EAPPacket;
    Ieee_8021x.Length  = cpu2be16(Length);
    if(bFirstFrag)
    {
        Length +=2;
    }

    // Zero EAP frame
    PlatformZeroMemory(&EapFrame, sizeof(EapFrame));
    EapFrame.Code   = EAP_CODE_RSP;
    EapFrame.Id     = Id;
    EapFrame.Length = cpu2be16(Length);
    EapFrame.Type   = EAP_TYPE_WSC;

    // As Rigistra, use Reuqest frame.
    if (IS_P2P_REGISTRA(pPort))
        EapFrame.Code   = EAP_CODE_REQ;
    // Counting Id by AP as Registrar
    if (EapFrame.Code == EAP_CODE_REQ)
        EapFrame.Id = (pPort->StaCfg.WscControl.EapId)++;

    // Zero WSC Frame
    PlatformZeroMemory(&WscFrame, sizeof(WscFrame));
    WscFrame.SMI[0] = 0x00;
    WscFrame.SMI[1] = 0x37;
    WscFrame.SMI[2] = 0x2A;
    WscFrame.VendorType = cpu2be32(WSC_VENDOR_TYPE);
    WscFrame.OpCode = OpCode;
    if(bFrag)
        WscFrame.Flags  = (WscFrame.Flags | 0x01);

    if(bFirstFrag)
    {
        //Need LF on and include 2 bytes Length in the First fragment packet
        WscFrame.Flags  = (WscFrame.Flags | 0x02);
        LF = 2;
        (*(PUSHORT)LF2Byte) = cpu2be16((SHORT)FragRemainLen);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("WscFrame Flags = 0x%x, FragRemainLen:%d\n", WscFrame.Flags, FragRemainLen));
    
    // Out buffer for transmitting message      
    NStatus = MlmeAllocateMemory(pAd, (PVOID)&OutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    FrameLen = 0;
    
    // Make  Transmitting frame
    if (pData && (Len > 0))
        MakeOutgoingFrame(OutBuffer, &FrameLen, sizeof(HEADER_802_11), &Header_802_11,
            sizeof(EAPHEAD), EAPHEAD, sizeof(IEEE8021X_FRAME), &Ieee_8021x,
            sizeof(EapFrame), &EapFrame, sizeof(WscFrame), &WscFrame, LF, LF2Byte, Len, pData,
            END_OF_ARGS);
    else
        MakeOutgoingFrame(OutBuffer, &FrameLen, sizeof(HEADER_802_11), &Header_802_11,
            sizeof(EAPHEAD), EAPHEAD, sizeof(IEEE8021X_FRAME), &Ieee_8021x,
            sizeof(EapFrame), &EapFrame, sizeof(WscFrame), &WscFrame, LF, LF2Byte, END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, OutBuffer, FrameLen);

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscMakeEAPFrame\n")); 
}

VOID    WsceSendEapReqWscStart(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    HEADER_802_11       Header_802_11;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0, Length;
    IEEE8021X_FRAME     Ieee_8021x;
    EAP_FRAME           EapFrame;
    UCHAR               *OutBuffer = NULL;
    NDIS_STATUS         NStatus;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
    WSC_FRAME           WscFrame;
    UCHAR               SMI[] = {0x00, 0x37, 0x2A};
    UCHAR               Addr2[6];
    PHEADER_802_11      pHeader_802_11;
    
    // 1. Send EAP-Rsp Id
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WpseSendEapReq WscStart\n"));
    
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    pHeader_802_11 = (PHEADER_802_11)&pElem->Msg[0];
    PlatformMoveMemory(Addr2, pHeader_802_11->Addr2, MAC_ADDR_LEN);
    
    WscMacHeaderInit(pAd, pPort, &Header_802_11, Addr2);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;

    // Length, -1 NULL pointer of string
    Length = sizeof(EAP_FRAME) + sizeof(WscFrame);
    
    // Zero 802.1x body
    PlatformZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
    Ieee_8021x.Version = EAPOL_VER_2;
    Ieee_8021x.Type    = EAPPacket;
    Ieee_8021x.Length  = cpu2be16(Length);

    // Zero EAP frame
    PlatformZeroMemory(&EapFrame, sizeof(EapFrame));
    EapFrame.Code   = EAP_CODE_REQ;
    EapFrame.Id = (pPort->StaCfg.WscControl.EapId)++;
    EapFrame.Length = cpu2be16(Length);
    EapFrame.Type   = EAP_TYPE_WSC;

    //Wsc Frame
    PlatformZeroMemory(&WscFrame, sizeof(WscFrame));
    PlatformMoveMemory(&WscFrame.SMI, &SMI, 3);
    WscFrame.VendorType = cpu2be32(WSC_VENDOR_TYPE);
    WscFrame.OpCode = WSC_OPCODE_START;
    WscFrame.Flags = 0x00;

    
    // Out buffer for transmitting message      
    NStatus = MlmeAllocateMemory(pAd, (PVOID)&OutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    FrameLen = 0;
    
    // Make  Transmitting frame
    MakeOutgoingFrame(OutBuffer, &FrameLen, 
                        sizeof(HEADER_802_11), &Header_802_11,
                        sizeof(EAPHEAD), EAPHEAD, 
                        sizeof(IEEE8021X_FRAME), &Ieee_8021x,
                        sizeof(EapFrame), &EapFrame,
                        sizeof(WscFrame), &WscFrame,
                        END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, OutBuffer, FrameLen);

    // Update WSC status
//  pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_ID_SENT;             

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WsceSendEapReqWscStart\n"));  

}


/*
    ========================================================================
    
    Routine Description:
        WSC state actions after receiveing EAP-Req(ID) at MlmeCntLinkUp state

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx EAP-Req(ID)
        1. Change the correct parameters
        2. Send EAP-Rsp(ID)
        
    ========================================================================
*/
VOID    WscSendEapRspId(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    HEADER_802_11       Header_802_11;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0, Length;
    IEEE8021X_FRAME     Ieee_8021x;
    EAP_FRAME           EapFrame;
    UCHAR               *OutBuffer = NULL;
    NDIS_STATUS         NStatus;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
    UCHAR               addr[6];
    PUCHAR              pIdentityStr = NULL;
    USHORT              IdentityStrLen = 0;
    PHEADER_802_11      pHeader_802_11;

    // 1. Send EAP-Rsp Id
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendEapRspId\n"));

    // Role play, Enrollee or Registrar
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
    {
        pIdentityStr    = EnrolleeMsg;
        IdentityStrLen  = sizeof(EnrolleeMsg);
    }
    else    // As a Registrar
    {
        pIdentityStr    = RegistrarMsg;
        IdentityStrLen  = sizeof(RegistrarMsg);
    }   
        
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    pHeader_802_11 = (PHEADER_802_11)&pElem->Msg[0];
    PlatformMoveMemory(addr, pHeader_802_11->Addr2, MAC_ADDR_LEN);
    WscMacHeaderInit(pAd, pPort, &Header_802_11, pPort->PortCfg.Bssid);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;

    // Length, -1 NULL pointer of string
    Length = sizeof(EAP_FRAME) + IdentityStrLen - 1;
    
    // Zero 802.1x body
    PlatformZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
    Ieee_8021x.Version = EAPOL_VER;
    Ieee_8021x.Type    = EAPPacket;
    Ieee_8021x.Length  = cpu2be16(Length);

    // Zero EAP frame
    PlatformZeroMemory(&EapFrame, sizeof(EapFrame));
    EapFrame.Code   = EAP_CODE_RSP;
    EapFrame.Id     = Id;
    EapFrame.Length = cpu2be16(Length);
    EapFrame.Type   = EAP_TYPE_ID;

    
    // Out buffer for transmitting message      
    NStatus = MlmeAllocateMemory(pAd, (PVOID)&OutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;

    FrameLen = 0;
    
    // Make  Transmitting frame
    MakeOutgoingFrame(OutBuffer, &FrameLen, sizeof(HEADER_802_11), &Header_802_11,
        sizeof(EAPHEAD), EAPHEAD, sizeof(IEEE8021X_FRAME), &Ieee_8021x,
        sizeof(EapFrame), &EapFrame, (IdentityStrLen - 1), pIdentityStr,
        END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, OutBuffer, FrameLen);

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_ID_SENT;

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendEapRspId\n")); 
}

/*
    ========================================================================
    
    Routine Description:
        WSC state actions after receiveing EAP-Req(ID) at MlmeCntLinkUp state

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx EAP-Req(ID)
        1. Change the correct parameters
        2. Send EAP-Rsp(ID)
        
    ========================================================================
*/
VOID    WscSendEapReqId(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    HEADER_802_11       Header_802_11;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0, Length;
    IEEE8021X_FRAME     Ieee_8021x;
    EAP_FRAME           EapFrame;
    UCHAR               *OutBuffer = NULL;
    NDIS_STATUS         NStatus;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
    PHEADER_802_11      pHeader_802_11;
    UCHAR               addr[6];
    
    // 1. Send EAP-Req Id
    
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    pHeader_802_11 = (PHEADER_802_11)&pElem->Msg[0];
    PlatformMoveMemory(addr, pHeader_802_11->Addr2, MAC_ADDR_LEN);
    WscMacHeaderInit(pAd, pPort, &Header_802_11, addr);
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendEapReqId to %x %x %x %x %x %x\n", addr[0], addr[1],addr[2],addr[3],addr[4],addr[5]));

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;

    // Length, -1 NULL pointer of string
    Length = sizeof(EAP_FRAME);
    
    // Zero 802.1x body
    PlatformZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
    Ieee_8021x.Version = EAPOL_VER;
    Ieee_8021x.Type    = EAPPacket;
    Ieee_8021x.Length  = cpu2be16(Length);

    // Zero EAP frame
    PlatformZeroMemory(&EapFrame, sizeof(EapFrame));
    EapFrame.Code   = EAP_CODE_REQ;
    EapFrame.Id = (pPort->StaCfg.WscControl.EapId)++;
    EapFrame.Length = cpu2be16(Length);
    EapFrame.Type   = EAP_TYPE_ID;

    
    // Out buffer for transmitting message      
    NStatus = MlmeAllocateMemory(pAd, (PVOID)&OutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    FrameLen = 0;
    
    // Make  Transmitting frame
    MakeOutgoingFrame(OutBuffer, &FrameLen, sizeof(HEADER_802_11), &Header_802_11,
        sizeof(EAPHEAD), EAPHEAD, sizeof(IEEE8021X_FRAME), &Ieee_8021x,
        sizeof(EapFrame), &EapFrame, END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, OutBuffer, FrameLen);
    //MlmeFreeMemory(pAd, OutBuffer);

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_ID_SENT;             

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendEapReqId\n")); 
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M1 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx EAP-Req(WSC_Start)
        1. Change the correct parameters
        2. Send M1
        
    ========================================================================
*/
VOID    WscSendMessageM1(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;   
    PUCHAR              pData;
    INT                 Len;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    
    // Send M1 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM1\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M1 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M1
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M1;
    pData += 5;
    Len   += 5;

    // 3. UUID_E, last 6 bytes use MAC
    *((PUSHORT) pData) = cpu2be16(WSC_IE_UUID_E);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.Uuid, 16);
    pData += 20;
    Len   += 20;

    // 4. MAC address
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MAC_ADDR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0006);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.MacAddr, 6);
    pData += 10;
    Len   += 10;

    // 5. Enrollee Nonce, first generate and save to Wsc Control Block
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;

    // 6. Public Key, 192 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PUBLIC_KEY);
    *((PUSHORT) (pData + 2)) = cpu2be16(192);
    PlatformMoveMemory((pData + 4), pReg->Pke, 192);
    pData += 196;
    Len   += 196;

    // 7. Authentication Type Flags
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTH_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->EnrolleeInfo.AuthTypeFlags;        // Open, WPAPSK, Shared, WPA2PSK
    pData += 6;
    Len   += 6;
    
    // 8. Encryption Type Flags
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->EnrolleeInfo.EncrTypeFlags;        // None, WEP, TKIP, AES
    pData += 6;
    Len   += 6;
    
    // 9. Connection Type Flag
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONN_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.ConnTypeFlags;                    // ESS
    pData += 5;
    Len   += 5;

    // 10. Config Method
    // TODO: Might need to differ between PBC and PIN mode
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->EnrolleeInfo.ConfigMethods;        // Label, Display, PBC
    pData += 6;
    Len   += 6;
    
    // 11. Wi-Fi Protected Setup State
    *((PUSHORT) pData) = cpu2be16(WSC_IE_STATE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.ScState;                      // Not Configured
    pData += 5;
    Len   += 5;

    // 12. Manufacture
    PlatformZeroMemory(pData, sizeof(Wsc_Manufacture) - 1 + 4);
    *((PUSHORT)pData) = cpu2be16(WSC_IE_MANUFACTURER);
    *((PUSHORT)(pData + 2)) = cpu2be16((sizeof(Wsc_Manufacture) - 1));//cpu2be16(0x0040);   
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.Manufacturer, sizeof(Wsc_Manufacture) - 1);
    pData += sizeof(Wsc_Manufacture) - 1 + 4;
    Len   += sizeof(Wsc_Manufacture) - 1 + 4;

    // 13. Model Name
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Name) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NAME);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Name) - 1));//cpu2be16(0x0020);   
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.ModelName, sizeof(Wsc_Model_Name) - 1); 
    pData += sizeof(Wsc_Model_Name) - 1 + 4;
    Len   += sizeof(Wsc_Model_Name) - 1 + 4;
    
    // 14. Model Number
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Number) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NO);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Number) - 1));//cpu2be16(0x0020); 
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.ModelNumber, sizeof(Wsc_Model_Number) - 1);
    pData += sizeof(Wsc_Model_Number) - 1 + 4;
    Len   += sizeof(Wsc_Model_Number) - 1 + 4;
    
    // 15. Serial Number
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Serial) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_SERIAL);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Serial)- 1));//cpu2be16(0x0020);  
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.SerialNumber, sizeof(Wsc_Model_Serial)- 1);
    pData += sizeof(Wsc_Model_Serial) - 1 + 4;
    Len   += sizeof(Wsc_Model_Serial) - 1 + 4;
    
    // 16. Primary Device Type
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PRI_DEV_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.PriDeviceType, 8);
    pData += 12;
    Len   += 12;
    
    // 17. Device Name
    PlatformZeroMemory(pData, pAd->WpsCfg.WPSDevNameLen + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
    *((PUSHORT) (pData + 2)) = cpu2be16((pAd->WpsCfg.WPSDevNameLen));//cpu2be16(0x0020);  
    PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.DeviceName, pAd->WpsCfg.WPSDevNameLen);
    pData += pAd->WpsCfg.WPSDevNameLen + 4;
    Len   += pAd->WpsCfg.WPSDevNameLen + 4;
    
    // 18. RF Bands
    *((PUSHORT) pData) = cpu2be16(WSC_IE_RF_BAND);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.RfBand;
    pData += 5;
    Len   += 5;

    // 19. Associate state
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ASSOC_STATE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->EnrolleeInfo.AssocState;   // Not associated
    pData += 6;
    Len   += 6;

    // 20. Device Password ID
    *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_PASS_ID);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->EnrolleeInfo.DevPwdId;
    pData += 6;
    Len   += 6;

    // 21. Configure Error
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_ERROR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->EnrolleeInfo.ConfigError;
    pData += 6;
    Len   += 6;

    // 22. OS Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_OS_VER);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0004);
    *((PULONG) (pData + 4)) = pReg->EnrolleeInfo.OsVersion; 
    pData += 8;
    Len   += 8;

    if(pReg->EnrolleeInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        DBGPRINT(RT_DEBUG_TRACE, ("Use WSC Version:0x20 \n"));  
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 23. Version2 if WSC version is highter than 2.0.
        *((PUCHAR) pData) = (WSC_IE_VERSION2);
        *((PUCHAR) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->EnrolleeInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }

    // Init the last two Tx
    pSavedWPSMessage->LastTx2.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, OutMsgBuf, Len);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);


    //
    //  ********* Start inband EAP transport *********
    //
    if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
    {
        pFragData = &OutMsgBuf[0];
        FragRemainLen = Len;
        WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
    }
    else
    {
        WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M1_SENT; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM1\n"));    
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M2 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M1
        1. Change the correct parameters
        2. Send M2
        
    ========================================================================
*/
VOID    WscSendMessageM2(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;   
    PUCHAR              pData, pAuth;
    INT                 Len;
    INT                 DH_Len, idx;
    UCHAR               DHKey[32], KDK[32], KdkInput[38], KdfKey[80], Tmp[128];
    ULONG               HmacLen;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    // Send M2 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM2\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M2 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M2
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M2;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);
    pData += 20;
    Len   += 20;

    // 5. UUID_R, last 6 bytes use MAC
    *((PUSHORT) pData) = cpu2be16(WSC_IE_UUID_R);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.Uuid, 16);
    pData += 20;
    Len   += 20;

    // 6. Public Key, 192 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PUBLIC_KEY);
    *((PUSHORT) (pData + 2)) = cpu2be16(192);
    PlatformMoveMemory((pData + 4), pReg->Pkr, 192);
    pData += 196;
    Len   += 196;

    // 7. Authentication Type Flags
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTH_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.AuthTypeFlags;       // Open, WPAPSK, Shared, WPA2PSK
    pData += 6;
    Len   += 6;
    
    // 8. Encryption Type Flags
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.EncrTypeFlags;       // None, WEP, TKIP, AES
    pData += 6;
    Len   += 6;
    
    // 9. Connection Type Flag
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONN_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.ConnTypeFlags;                   // ESS
    pData += 5;
    Len   += 5;

    // 10. Config Method
    // TODO: Might need to differ between PBC and PIN mode
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.ConfigMethods;       // Label, Display, PBC
    pData += 6;
    Len   += 6;

    // 11. Manufacture
    PlatformZeroMemory(pData, sizeof(Wsc_Manufacture) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MANUFACTURER);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Manufacture)- 1));//cpu2be16(0x0040);   
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.Manufacturer, sizeof(Wsc_Manufacture)-1);
    pData += sizeof(Wsc_Manufacture) - 1 + 4;
    Len   += sizeof(Wsc_Manufacture) - 1 + 4;

    // 12. Model Name
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Name) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NAME);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Name)- 1));//cpu2be16(0x0020);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.ModelName, sizeof(Wsc_Model_Name)-1); 
    pData += sizeof(Wsc_Model_Name) - 1 + 4;
    Len   += sizeof(Wsc_Model_Name) - 1 + 4;
    
    // 13. Model Number
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Number) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NO);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Number)- 1));//cpu2be16(0x0020);  
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.ModelNumber, sizeof(Wsc_Model_Number)- 1);
    pData += sizeof(Wsc_Model_Number) - 1 + 4;
    Len   += sizeof(Wsc_Model_Number) - 1 + 4;
    
    // 14. Serial Number
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Serial) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_SERIAL);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Serial)- 1));//cpu2be16(0x0020);  
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.SerialNumber, sizeof(Wsc_Model_Serial)- 1);
    pData += sizeof(Wsc_Model_Serial) - 1 + 4;
    Len   += sizeof(Wsc_Model_Serial) - 1 + 4;
    
    // 15. Primary Device Type
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PRI_DEV_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.PriDeviceType, 8);
    pData += 12;
    Len   += 12;
    
    // 16. Device Name
    PlatformZeroMemory(pData, sizeof(Wsc_Dev_Name_R) - 1 + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
    *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Dev_Name_R)- 1));//cpu2be16(0x0020);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.DeviceName, sizeof(Wsc_Dev_Name_R)- 1);
    pData += sizeof(Wsc_Dev_Name_R) - 1 + 4;
    Len   += sizeof(Wsc_Dev_Name_R) - 1 + 4;
    
    // 17. RF Bands
    *((PUSHORT) pData) = cpu2be16(WSC_IE_RF_BAND);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.RfBand;
    pData += 5;
    Len   += 5;

    // 18. Associate state
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ASSOC_STATE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.AssocState;  // Not associated
    pData += 6;
    Len   += 6;

    // 19. Configure Error
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_ERROR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.ConfigError;
    pData += 6;
    Len   += 6;

    // 20. Device Password ID
    *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_PASS_ID);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.DevPwdId;
    pData += 6;
    Len   += 6;

    // 21. OS Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_OS_VER);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0004);
    *((PULONG) (pData + 4)) = pReg->RegistrarInfo.OsVersion;    
    pData += 8;
    Len   += 8;
    if(pReg->RegistrarInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);
        
        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }

    }

    // Let's start to generate keys
    // Generate Secret Key 
    //==========================================================================
    GenerateDHSecreteKey(&pReg->DHRandom[0], 192, &pReg->Pke[0], 192, &pReg->SecretKey[0], &DH_Len);

    // Compute the DHKey based on the DH secret
    sha256(&DHKey[0], &pReg->SecretKey[0], 192);

    // Create KDK input data
    PlatformMoveMemory(&KdkInput[0], pReg->EnrolleeNonce, 16);
        
    PlatformMoveMemory(&KdkInput[16], pReg->EnrolleeInfo.MacAddr, 6);
        
    PlatformMoveMemory(&KdkInput[22], pReg->RegistrarNonce, 16);
    
    // Generate the KDK
    hmac_sha(DHKey, 32,  KdkInput, 38, KDK, 32);

    // KDF
    WscDeriveKey(KDK, 32, Wsc_Personal_String, (sizeof(Wsc_Personal_String) - 1), KdfKey, 640);

    // Assign Key from KDF
    PlatformMoveMemory(pReg->AuthKey, &KdfKey[0], 32);
    PlatformMoveMemory(pReg->KeyWrapKey, &KdfKey[32], 16);
    PlatformMoveMemory(pReg->Emsk, &KdfKey[48], 32);
    // End of Key Generation====================================================

    // Init the last two Tx
    pSavedWPSMessage->LastTx2.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, OutMsgBuf, Len);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    // 22. Generate authenticator
    // Combine last TX & RX message contents and validate the HMAC
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length);
    pAuth += pSavedWPSMessage->LastRx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);

    // Validate HMAC, reuse KDK buffer
    DBGPRINT(RT_DEBUG_TRACE, ("HmacLen = %d\n", HmacLen));
    DBGPRINT(RT_DEBUG_INFO, ("HmacData --> "));
    
    for (idx = 0; idx < 64; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", HmacData[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));        

    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTHENTICATOR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), Tmp, 8);

    // Append the authenticator to last tx buffer for future HMAC calculation
    PlatformMoveMemory(&pSavedWPSMessage->LastTx.Data[pSavedWPSMessage->LastTx.Length], pData, 12);
    pSavedWPSMessage->LastTx.Length += 12;
    pData += 12;
    Len   += 12;


    //
    //  ********* Start transport *********
    //
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
    {
        // Use Inb-EAP transport
        if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
        {
            pFragData = &OutMsgBuf[0];
            FragRemainLen = Len;
            WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
        }
        else
            WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);
    }
    else    // WSC_ConfMode_REGISTRAR_UPNP
    {
        // Use Inb-UPnP transport
        DBGPRINT(RT_DEBUG_TRACE, ("ExtReg Set M2 Event!!\n"));
        RTMPSetExtRegMessageEvent(pAd, EXTREG_MESSAGE_EVENT_ID, OutMsgBuf, Len);
        
        for (idx = 0; idx < Len; idx++)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-", OutMsgBuf[idx]));
        }
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M2_SENT; 

    //Record the enrollee MAC as we are external registrar.
    if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP) /*&& (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE)*/)
    {
        PlatformMoveMemory(pPort->StaCfg.WscControl.ExtRegEnrMAC, pReg->EnrolleeInfo.MacAddr, 6);
        DBGPRINT(RT_DEBUG_TRACE, ("ExtRegEnrMAC:%x %x %x\n", pPort->StaCfg.WscControl.ExtRegEnrMAC[3], pPort->StaCfg.WscControl.ExtRegEnrMAC[4], pPort->StaCfg.WscControl.ExtRegEnrMAC[5]));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM2\n"));
}

VOID    WscSendMessageM2D(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;   
    PUCHAR              pData;
    INT                 Len;
    INT                 idx;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    // Send M2 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM2D\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M2 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M2
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M2D;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);
    pData += 20;
    Len   += 20;

    // 5. UUID_R, last 6 bytes use MAC
    *((PUSHORT) pData) = cpu2be16(WSC_IE_UUID_R);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.Uuid, 16);
    pData += 20;
    Len   += 20;

/*  // 6. Public Key, 192 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PUBLIC_KEY);
    *((PUSHORT) (pData + 2)) = cpu2be16(192);
    PlatformMoveMemory((pData + 4), pReg->Pkr, 192);
    pData += 196;
    Len   += 196;
*/
    // 7. Authentication Type Flags
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTH_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.AuthTypeFlags;       // Open, WPAPSK, Shared, WPA2PSK
    pData += 6;
    Len   += 6;
    
    // 8. Encryption Type Flags
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.EncrTypeFlags;       // None, WEP, TKIP, AES
    pData += 6;
    Len   += 6;
    
    // 9. Connection Type Flag
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONN_TYPE_FLAG);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.ConnTypeFlags;                   // ESS
    pData += 5;
    Len   += 5;

    // 10. Config Method
    // TODO: Might need to differ between PBC and PIN mode
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.ConfigMethods;       // Label, Display, PBC
    pData += 6;
    Len   += 6;

    // 11. Manufacture
    PlatformZeroMemory(pData, sizeof(Wsc_Manufacture) + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MANUFACTURER);
    *((PUSHORT) (pData + 2)) = cpu2be16(sizeof(Wsc_Manufacture));//cpu2be16(0x0040);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.Manufacturer, sizeof(Wsc_Manufacture));
    pData += sizeof(Wsc_Manufacture) + 4;
    Len   += sizeof(Wsc_Manufacture) + 4;

    // 12. Model Name
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Name) + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NAME);
    *((PUSHORT) (pData + 2)) = cpu2be16(sizeof(Wsc_Model_Name));//cpu2be16(0x0020); 
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.ModelName, sizeof(Wsc_Model_Name)); 
    pData += sizeof(Wsc_Model_Name) + 4;
    Len   += sizeof(Wsc_Model_Name) + 4;
    
    // 13. Model Number
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Number) + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NO);
    *((PUSHORT) (pData + 2)) = cpu2be16(sizeof(Wsc_Model_Number));//cpu2be16(0x0020);   
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.ModelNumber, sizeof(Wsc_Model_Number));
    pData += sizeof(Wsc_Model_Number) + 4;
    Len   += sizeof(Wsc_Model_Number) + 4;
    
    // 14. Serial Number
    PlatformZeroMemory(pData, sizeof(Wsc_Model_Serial) + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_SERIAL);
    *((PUSHORT) (pData + 2)) = cpu2be16(sizeof(Wsc_Model_Serial));//cpu2be16(0x0020);   
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.SerialNumber, sizeof(Wsc_Model_Serial));
    pData += sizeof(Wsc_Model_Serial) + 4;
    Len   += sizeof(Wsc_Model_Serial) + 4;
    
    // 15. Primary Device Type
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PRI_DEV_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.PriDeviceType, 8);
    pData += 12;
    Len   += 12;
    
    // 16. Device Name
    PlatformZeroMemory(pData, sizeof(Wsc_Dev_Name_R) + 4);
    *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
    *((PUSHORT) (pData + 2)) = cpu2be16(sizeof(Wsc_Dev_Name_R));//cpu2be16(0x0020); 
    PlatformMoveMemory((pData + 4), pReg->RegistrarInfo.DeviceName, sizeof(Wsc_Dev_Name_R));
    pData += sizeof(Wsc_Dev_Name_R) + 4;
    Len   += sizeof(Wsc_Dev_Name_R) + 4;
    
    // 17. RF Bands
    *((PUSHORT) pData) = cpu2be16(WSC_IE_RF_BAND);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.RfBand;
    pData += 5;
    Len   += 5;

    // 18. Associate state
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ASSOC_STATE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.AssocState;  // Not associated
    pData += 6;
    Len   += 6;

    // 19. Configure Error
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_ERROR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP) && 
        (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EXTREG_MULTI_PBC_SESSIONS_DETECT))
    {
        *((PUSHORT) (pData + 4)) = cpu2be16(WSC_ERROR_MULTI_PBC_DETECTED);
    }
    else
    {
        *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.ConfigError;
    }
    pData += 6;
    Len   += 6;

    // 20. Device Password ID
    *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_PASS_ID);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pReg->RegistrarInfo.DevPwdId;
    pData += 6;
    Len   += 6;

    // 21. OS Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_OS_VER);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0004);
    *((PULONG) (pData + 4)) = pReg->RegistrarInfo.OsVersion;    
    pData += 8;
    Len   += 8;

    if(pReg->RegistrarInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 1) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }

    }

    // Init the last two Tx
    pSavedWPSMessage->LastTx2.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, OutMsgBuf, Len);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    //
    //  ********* Start transport *********
    //
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Sent M2D .!!\n"));
        // Use Inb-EAP transport
        if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
        {
            pFragData = &OutMsgBuf[0];
            FragRemainLen = Len;
            WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
        }
        else
            WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);
    }
    //if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    else    // WSC_ConfMode_REGISTRAR_UPNP
    {
        // Use Inb-UPnP transport
        DBGPRINT(RT_DEBUG_TRACE, ("ExtReg Set M2D Event!!\n"));
        RTMPSetExtRegMessageEvent(pAd, EXTREG_MESSAGE_EVENT_ID, OutMsgBuf, Len);

        for (idx = 0; idx < Len; idx++)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-", OutMsgBuf[idx]));
        }
    }


    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M2D_SENT;    


    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM2D\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M3 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M2
        1. Change the correct parameters
        2. Send M3
        
    ========================================================================
*/
VOID    WscSendMessageM3(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    UCHAR               PSKBuf[512];
    PUCHAR              pData, pAuth;
    ULONG               HmacLen;
    INT                 Len, idx;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    
    // Send M3 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM3\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M3 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M3
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M3;
    pData += 5;
    Len   += 5;

    // 3. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. E-Hash1
    //
    // Generate PSK1
    hmac_sha(pReg->AuthKey, 32, pReg->PIN, pReg->PINLen, PSKBuf, 32);

    // Copy first 16 bytes to PSK1
    PlatformMoveMemory(pReg->Psk1, PSKBuf, 16);

    // Create input for E-Hash1
    PlatformMoveMemory(PSKBuf, pReg->Es1, 16);
    PlatformMoveMemory(&PSKBuf[16], pReg->Psk1, 16);
    PlatformMoveMemory(&PSKBuf[32], pReg->Pke, 192);
    PlatformMoveMemory(&PSKBuf[224], pReg->Pkr, 192);
    
    // Generate E-Hash1
    hmac_sha(pReg->AuthKey, 32, PSKBuf, 416, pReg->EHash1, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_E_HASH_1);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0020);    
    PlatformMoveMemory((pData + 4), pReg->EHash1, 32);
    pData += 36;
    Len   += 36;

    // 5. E-Hash2
    //
    // Generate PSK2
    if(pReg->PINLen == 4)   //8 PIN
    {
        hmac_sha(pReg->AuthKey, 32, &pReg->PIN[4], pReg->PINLen, PSKBuf, 32);
    }
    else if(pReg->PINLen == 2)  //4 PIN
    {
        hmac_sha(pReg->AuthKey, 32, &pReg->PIN[2], pReg->PINLen, PSKBuf, 32);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Error: PIN code Length(%d) is wrong!!!!!!!!!\n", pReg->PINLen));
    }

    // Copy first 16 bytes to PSK2
    PlatformMoveMemory(pReg->Psk2, PSKBuf, 16);
        
    // Create input for E-Hash2
    PlatformMoveMemory(PSKBuf, pReg->Es2, 16);
    PlatformMoveMemory(&PSKBuf[16], pReg->Psk2, 16);
    PlatformMoveMemory(&PSKBuf[32], pReg->Pke, 192);
    PlatformMoveMemory(&PSKBuf[224], pReg->Pkr, 192);
    
    // Generate E-Hash2
    hmac_sha(pReg->AuthKey, 32, PSKBuf, 416, pReg->EHash2, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_E_HASH_2);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0020);    
    PlatformMoveMemory((pData + 4), pReg->EHash2, 32);
    pData += 36;
    Len   += 36;
    if(pReg->EnrolleeInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->EnrolleeInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen += cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }

    // Save the last two Tx for the peeer retransmission
    pSavedWPSMessage->LastTx2.Length = pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    // 6. Generate authenticator
    // Combine last TX & RX message contents and validate the HMAC
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length);
    pAuth += pSavedWPSMessage->LastRx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);

    // Validate HMAC, reuse KDK buffer
    DBGPRINT(RT_DEBUG_TRACE, ("HmacLen = %d\n", HmacLen));
    DBGPRINT(RT_DEBUG_TRACE, ("HmacData --> "));
    
    for (idx = 0; idx < 64; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-", HmacData[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));        

    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, PSKBuf, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTHENTICATOR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), PSKBuf, 8);

    // Append the authenticator to last tx buffer for future HMAC calculation
    PlatformMoveMemory(&pSavedWPSMessage->LastTx.Data[pSavedWPSMessage->LastTx.Length], pData, 12);
    pSavedWPSMessage->LastTx.Length += 12;
    pData += 12;
    Len   += 12;
    

    //
    //  ********* Start inband EAP transport *********
    //
    if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
    {
        pFragData = &OutMsgBuf[0];
        FragRemainLen = Len;
        WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
    }
    else
        WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);


    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M3_SENT; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM3\n"));    
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M4 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M3
        1. Change the correct parameters
        2. Send M4
        
    ========================================================================
*/
VOID    WscSendMessageM4(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    UCHAR               PSKBuf[512];
    PUCHAR              pData, pAuth, pPlain;
    ULONG               HmacLen;
    UCHAR               Plain[128], Tmp[128], IV[16], EncrData[128];
    INT                 Len, idx, PlainLen, EncrLen;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
  
    // Send M4 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM4\n"));
    DBGPRINT(RT_DEBUG_TRACE,("RegData   PIN = %x  %x\n", *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M4 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M4
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M4;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. R-Hash1
    //
    // Generate PSK1
    hmac_sha(pReg->AuthKey, 32, pReg->PIN, pReg->PINLen, PSKBuf, 32);

    // Copy first 16 bytes to PSK1
    PlatformMoveMemory(pReg->Psk1, PSKBuf, 16);

    // Create input for R-Hash1
    PlatformMoveMemory(PSKBuf, pReg->Rs1, 16);
    PlatformMoveMemory(&PSKBuf[16], pReg->Psk1, 16);
    PlatformMoveMemory(&PSKBuf[32], pReg->Pke, 192);
    PlatformMoveMemory(&PSKBuf[224], pReg->Pkr, 192);
    
    // Generate R-Hash1
    hmac_sha(pReg->AuthKey, 32, PSKBuf, 416, pReg->RHash1, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_R_HASH_1);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0020);    
    PlatformMoveMemory((pData + 4), pReg->RHash1, 32);
    pData += 36;
    Len   += 36;

    // 5. R-Hash2
    //
    // Generate PSK2
    if(pReg->PINLen == 4)
    {
        hmac_sha(pReg->AuthKey, 32, &pReg->PIN[4], pReg->PINLen, PSKBuf, 32);
    }
    else if(pReg->PINLen == 2)
    {
        hmac_sha(pReg->AuthKey, 32, &pReg->PIN[2], pReg->PINLen, PSKBuf, 32);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Error: PIN code Length(%d) is wrong!!!!!!!!!\n", pReg->PINLen));
    }

    // Copy first 16 bytes to PSK2
    PlatformMoveMemory(pReg->Psk2, PSKBuf, 16);
        
    // Create input for R-Hash2
    PlatformMoveMemory(PSKBuf, pReg->Rs2, 16);
    PlatformMoveMemory(&PSKBuf[16], pReg->Psk2, 16);
    PlatformMoveMemory(&PSKBuf[32], pReg->Pke, 192);
    PlatformMoveMemory(&PSKBuf[224], pReg->Pkr, 192);
    
    // Generate R-Hash2
    hmac_sha(pReg->AuthKey, 32, PSKBuf, 416, pReg->RHash2, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_R_HASH_2);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0020);    
    PlatformMoveMemory((pData + 4), pReg->RHash2, 32);
    pData += 36;
    Len   += 36;


    // 6. Encrypted R-S1
    //    ================================================================================
    //    Prepare plain text
    pPlain = (PUCHAR) Plain;
    PlainLen = 0;
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_R_SNONCE_1);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0010);   
    PlatformMoveMemory((pPlain + 4), pReg->Rs1, 16);
    pPlain += 20;
    PlainLen += 20;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text --> "));  
    for (idx = 0; idx < PlainLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Plain[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));

    // Generate HMAC
    hmac_sha(pReg->AuthKey, 32, Plain, PlainLen, Tmp, 32);
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_KWRAP_AUTH);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x008);    
    PlatformMoveMemory((pPlain + 4), Tmp, 8);
    pPlain += 12;
    PlainLen += 12;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text Hmac --> ")); 
    for (idx = 0; idx < 8; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Tmp[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));

    // generate a random IV with 16 bytes
    for (idx = 0; idx < 16; idx++)
        IV[idx] = RandomByte(pAd);

    // Encrypt data
    WscEncryptData(pAd, Plain, PlainLen, pReg->KeyWrapKey, IV, EncrData, &EncrLen);

    //    ================================================================================
    //    End of Encrypted Setting


    // 6. Encrypted Settings
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_SETTING);
    *((PUSHORT) (pData + 2)) = cpu2be16(16 + EncrLen);  
    PlatformMoveMemory((pData + 4), IV, 16);
    PlatformMoveMemory((pData + 20), EncrData, EncrLen);
    pData += (20 + EncrLen);
    Len   += (20 + EncrLen);

    if(pReg->RegistrarInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen += cpu2be16(VendorExtLen);
        
        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }
    // Save the last two Tx for the peeer retransmission
    pSavedWPSMessage->LastTx2.Length = pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    // 7. Generate authenticator
    // Combine last TX & RX message contents and validate the HMAC
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length);
    pAuth += pSavedWPSMessage->LastRx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);

    // Validate HMAC, reuse KDK buffer
    DBGPRINT(RT_DEBUG_TRACE, ("HmacLen = %d\n", HmacLen));
    DBGPRINT(RT_DEBUG_TRACE, ("HmacData --> "));
    
    for (idx = 0; idx < 64; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", HmacData[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));       

    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, PSKBuf, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTHENTICATOR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), PSKBuf, 8);

    // Append the authenticator to last tx buffer for future HMAC calculation
    PlatformMoveMemory(&pSavedWPSMessage->LastTx.Data[pSavedWPSMessage->LastTx.Length], pData, 12);
    pSavedWPSMessage->LastTx.Length += 12;
    pData += 12;
    Len   += 12;


    //
    //  ********* Start transport *********
    //
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
    {
        // Use Inb-EAP transport
        if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
        {
            pFragData = &OutMsgBuf[0];
            FragRemainLen = Len;
            WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
        }
        else
            WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);
    }
    else    // WSC_ConfMode_REGISTRAR_UPNP
    {
        // Use Inb-UPnP transport
        DBGPRINT(RT_DEBUG_TRACE, ("ExtReg Set M4 Event!!\n"));
        RTMPSetExtRegMessageEvent(pAd, EXTREG_MESSAGE_EVENT_ID, OutMsgBuf, Len);
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M4_SENT; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM4\n"));    

}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M5 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M4
        1. Change the correct parameters
        2. Send M5
        
    ========================================================================
*/
VOID    WscSendMessageM5(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    PUCHAR              pData, pAuth, pPlain;
    UCHAR               Plain[128], Tmp[128], IV[16], EncrData[128];
    ULONG               HmacLen;
    INT                 Len, idx, PlainLen, EncrLen;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    
    // Send M5 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM5\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M5 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M5
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M5;
    pData += 5;
    Len   += 5;

    // 3. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. Encrypted E-S1
    //    ================================================================================
    //    Prepare plain text
    pPlain = (PUCHAR) Plain;
    PlainLen = 0;
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_E_SNONCE_1);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0010);   
    PlatformMoveMemory((pPlain + 4), pReg->Es1, 16);
    pPlain += 20;
    PlainLen += 20;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text --> "));  
    for (idx = 0; idx < PlainLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Plain[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));

    // Generate HMAC
    hmac_sha(pReg->AuthKey, 32, Plain, PlainLen, Tmp, 32);
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_KWRAP_AUTH);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x008);    
    PlatformMoveMemory((pPlain + 4), Tmp, 8);
    pPlain += 12;
    PlainLen += 12;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text Hmac --> ")); 
    for (idx = 0; idx < 8; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Tmp[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));

    // generate a random IV with 16 bytes
    for (idx = 0; idx < 16; idx++)
        IV[idx] = RandomByte(pAd);

    // Encrypt data
    WscEncryptData(pAd, Plain, PlainLen, pReg->KeyWrapKey, IV, EncrData, &EncrLen);

    //    ================================================================================
    //    End of Encrypted Setting

    // 5. Encrypted Settings
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_SETTING);
    *((PUSHORT) (pData + 2)) = cpu2be16(16 + EncrLen);  
    PlatformMoveMemory((pData + 4), IV, 16);
    PlatformMoveMemory((pData + 20), EncrData, EncrLen);
    pData += (20 + EncrLen);
    Len   += (20 + EncrLen);

    if(pReg->EnrolleeInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->EnrolleeInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        *pVendorExtLen += cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }

    // Save the last two Tx for the peeer retransmission
    pSavedWPSMessage->LastTx2.Length = pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    // 6. Generate authenticator
    // Combine last TX & RX message contents and validate the HMAC
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length);
    pAuth += pSavedWPSMessage->LastRx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);

    // Validate HMAC

    DBGPRINT(RT_DEBUG_TRACE, ("HmacLen = %d\n", HmacLen));
    DBGPRINT(RT_DEBUG_TRACE, ("HmacData --> "));    
    for (idx = 0; idx < (INT) HmacLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", HmacData[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));
    
    DBGPRINT(RT_DEBUG_TRACE, ("AuthKey --> ")); 
    for (idx = 0; idx < 32; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->AuthKey[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));
    
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTHENTICATOR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), Tmp, 8);

    // Append the authenticator to last tx buffer for future HMAC calculation
    PlatformMoveMemory(&pSavedWPSMessage->LastTx.Data[pSavedWPSMessage->LastTx.Length], pData, 12);
    pSavedWPSMessage->LastTx.Length += 12;
    pData += 12;
    Len   += 12;
    

    //
    //  ********* Start inband EAP transport *********
    //
    if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
    {
        pFragData = &OutMsgBuf[0];
        FragRemainLen = Len;
        WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
    }
    else
        WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);


    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M5_SENT; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM5\n"));    
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M6 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M5
        1. Change the correct parameters
        2. Send M6
        
    ========================================================================
*/
VOID    WscSendMessageM6(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    PUCHAR              pData, pAuth, pPlain;
    UCHAR               Plain[128], Tmp[128], IV[16], EncrData[128];
    ULONG               HmacLen;
    INT                 Len, idx, PlainLen, EncrLen;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
    {
        pPort = pAd->PortList[pElem->PortNum];
    }

    // Send M6 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM6\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M6 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M6
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M6;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. Encrypted R-S2
    //    ================================================================================
    //    Prepare plain text
    pPlain = (PUCHAR) Plain;
    PlainLen = 0;
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_R_SNONCE_2);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0010);   
    PlatformMoveMemory((pPlain + 4), pReg->Rs2, 16);
    pPlain += 20;
    PlainLen += 20;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text --> "));  
    for (idx = 0; idx < PlainLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Plain[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));

    // Generate HMAC
    hmac_sha(pReg->AuthKey, 32, Plain, PlainLen, Tmp, 32);
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_KWRAP_AUTH);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x008);    
    PlatformMoveMemory((pPlain + 4), Tmp, 8);
    pPlain += 12;
    PlainLen += 12;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text Hmac --> ")); 
    for (idx = 0; idx < 8; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Tmp[idx]));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));

    // generate a random IV with 16 bytes
    for (idx = 0; idx < 16; idx++)
        IV[idx] = RandomByte(pAd);

    // Encrypt data
    WscEncryptData(pAd, Plain, PlainLen, pReg->KeyWrapKey, IV, EncrData, &EncrLen);

    //    ================================================================================
    //    End of Encrypted Setting

    // 4. Encrypted Settings
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_SETTING);
    *((PUSHORT) (pData + 2)) = cpu2be16(16 + EncrLen);  
    PlatformMoveMemory((pData + 4), IV, 16);
    PlatformMoveMemory((pData + 20), EncrData, EncrLen);
    pData += (20 + EncrLen);
    Len   += (20 + EncrLen);

    if(pReg->RegistrarInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen += cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }

    // Save the last two Tx for the peeer retransmission
    pSavedWPSMessage->LastTx2.Length = pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    // 5. Generate authenticator
    // Combine last TX & RX message contents and validate the HMAC
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length);
    pAuth += pSavedWPSMessage->LastRx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);

    // Validate HMAC

    DBGPRINT(RT_DEBUG_TRACE, ("HmacLen = %d\n", HmacLen));
    DBGPRINT(RT_DEBUG_TRACE, ("HmacData --> "));    
    for (idx = 0; idx < (INT) HmacLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", HmacData[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));
    
    DBGPRINT(RT_DEBUG_TRACE, ("AuthKey --> ")); 
    for (idx = 0; idx < 32; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->AuthKey[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));
    
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTHENTICATOR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), Tmp, 8);

    // Append the authenticator to last tx buffer for future HMAC calculation
    PlatformMoveMemory(&pSavedWPSMessage->LastTx.Data[pSavedWPSMessage->LastTx.Length], pData, 12);
    pSavedWPSMessage->LastTx.Length += 12;
    pData += 12;
    Len   += 12;


    //
    //  ********* Start transport *********
    //
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
    {
        // Use Inb-EAP transport
        if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
        {
            pFragData = &OutMsgBuf[0];
            FragRemainLen = Len;
            WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
        }
        else
            WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);
    }
    else    // WSC_ConfMode_REGISTRAR_UPNP
    {
        // Use Inb-UPnP transport
        DBGPRINT(RT_DEBUG_TRACE, ("ExtReg Set M6 Event!!\n"));
        RTMPSetExtRegMessageEvent(pAd, EXTREG_MESSAGE_EVENT_ID, OutMsgBuf, Len);
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M6_SENT; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM6\n"));    

}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M7 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M6
        1. Change the correct parameters
        2. Send M7
        
    ========================================================================
*/
VOID    WscSendMessageM7(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    PUCHAR              pData, pAuth, pPlain;
    UCHAR               Plain[128], Tmp[128], IV[16], EncrData[128];
    ULONG               HmacLen;
    INT                 Len, idx, PlainLen, EncrLen;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    
    // Send M7 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM7\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M7 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M7;
    pData += 5;
    Len   += 5;

    // 3. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. Encrypted E-S2
    //    ================================================================================
    //    Prepare plain text
    pPlain = (PUCHAR) Plain;
    PlainLen = 0;
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_E_SNONCE_2);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0010);   
    PlatformMoveMemory((pPlain + 4), pReg->Es2, 16);
    pPlain += 20;
    PlainLen += 20;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text --> "));  
    for (idx = 0; idx < PlainLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Plain[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));

    // Generate HMAC
    hmac_sha(pReg->AuthKey, 32, Plain, PlainLen, Tmp, 32);
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_KWRAP_AUTH);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x008);    
    PlatformMoveMemory((pPlain + 4), Tmp, 8);
    pPlain += 12;
    PlainLen += 12;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text Hmac --> ")); 
    for (idx = 0; idx < 8; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Tmp[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));

    // generate a random IV with 16 bytes
    for (idx = 0; idx < 16; idx++)
        IV[idx] = RandomByte(pAd);

    // Encrypt data
    WscEncryptData(pAd, Plain, PlainLen, pReg->KeyWrapKey, IV, EncrData, &EncrLen);

    //    ================================================================================
    //    End of Encrypted Setting

    // 4. Encrypted Settings
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_SETTING);
    *((PUSHORT) (pData + 2)) = cpu2be16(16 + EncrLen);  
    PlatformMoveMemory((pData + 4), IV, 16);
    PlatformMoveMemory((pData + 20), EncrData, EncrLen);
    pData += (20 + EncrLen);
    Len   += (20 + EncrLen);

    if(pReg->EnrolleeInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->EnrolleeInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }

    // Save the last two Tx for the peeer retransmission
    pSavedWPSMessage->LastTx2.Length = pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    // 5. Generate authenticator
    // Combine last TX & RX message contents and validate the HMAC
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length);
    pAuth += pSavedWPSMessage->LastRx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);

    // Validate HMAC

    DBGPRINT(RT_DEBUG_TRACE, ("HmacLen = %d\n", HmacLen));
    DBGPRINT(RT_DEBUG_TRACE, ("HmacData --> "));    
    for (idx = 0; idx < (INT) HmacLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", HmacData[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));
    
    DBGPRINT(RT_DEBUG_TRACE, ("AuthKey --> ")); 
    for (idx = 0; idx < 32; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->AuthKey[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));
    
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTHENTICATOR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), Tmp, 8);

    // Append the authenticator to last tx buffer for future HMAC calculation
    PlatformMoveMemory(&pSavedWPSMessage->LastTx.Data[pSavedWPSMessage->LastTx.Length], pData, 12);
    pSavedWPSMessage->LastTx.Length += 12;
    pData += 12;
    Len   += 12;
    

    //
    //  ********* Start inband EAP transport *********
    //
    if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
    {
        pFragData = &OutMsgBuf[0];
        FragRemainLen = Len;
        WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
    }
    else
        WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M7_SENT; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM7\n"));    
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC M8 Message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M7
        1. Change the correct parameters
        2. Send M8
        
    ========================================================================
*/
VOID    WscSendMessageM8(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    PUCHAR              pData, pAuth, pPlain;
    UCHAR               Tmp[128], IV[16];//, EncrData[128];
    ULONG               HmacLen;
    INT                 Len, idx, PlainLen, EncrLen;
    PWSC_CREDENTIAL     pProfileCred;
    PSAVED_WPS_MESSAGE  pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    ULONG               CredentialLen = 0;
    PUCHAR              LenPtr = NULL;
    
    // Send M8 message
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendMessageM8\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M8 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->RegistrarInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_M8;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. Encrypted wireless settings for Enrollee
    //    ================================================================================
    //    Prepare plain text
    pPlain = (PUCHAR) &PlainText[0];
    PlainLen = 0;

    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        //WSC_IE_CREDENTIAL
        pPlain += 4;
        PlainLen += 4;  
    }
    
    // Enrollee_AP.
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR) 
    {
        // The peer is a configured AP.
        if (IS_P2P_REGISTRA(pPort))  
        {   //  Auto-generate a profile
            WscAutoGenProfile(pAd, pPort);
        }
        else if (pPort->StaCfg.WscControl.bConfiguredAP == TRUE &&
            pPort->StaCfg.WscControl.WscM7Profile.ProfileCnt == 1)
        {
            // Copy M7 profile to M8
/*          PlatformZeroMemory(&pPort->StaCfg.WscControl.WscProfile, sizeof(WSC_PROFILE));
            PlatformMoveMemory( &pPort->StaCfg.WscControl.WscProfile.Profile[0], 
                            &pPort->StaCfg.WscControl.WscM7Profile.Profile[0], 
                            sizeof(WSC_CREDENTIAL));
            pPort->StaCfg.WscControl.WscProfile.ProfileCnt =
                    pPort->StaCfg.WscControl.WscM7Profile.ProfileCnt;*/
        }
        else if (pPort->StaCfg.WscControl.WscProfile.ProfileCnt == 0)    
        {   //  Auto-generate a profile
            WscAutoGenProfile(pAd, pPort);
        }
    
    }
    
    // Fill ApEncrSettings/StaEncrSettings
    for (idx = 0; idx < (INT)pPort->StaCfg.WscControl.WscProfile.ProfileCnt; idx++)
    {
        // Support up to 8 profiles
        if (idx >= 8)
            break;
            
        pProfileCred = (PWSC_CREDENTIAL) &pPort->StaCfg.WscControl.WscProfile.Profile[idx];

        // 4.1 Network Index, 1 byte (optional for ApEncrSettings)
        // If this attribute is omitted, defaults to 1.
        if (pProfileCred->NetIndex == 0)
            pProfileCred->NetIndex = 1;
        if (IS_P2P_REGISTRA(pPort))
        {
            LenPtr = pPlain;
            *((PUSHORT) pPlain) = cpu2be16(WSC_IE_CREDENTIAL);
            *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0000);   
            pPlain += 4;
            PlainLen += 4;
        }
        *((PUSHORT) pPlain) = cpu2be16(WSC_IE_NETWORK_IDX);
        *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0001);   
        *((pPlain + 4)) = 1;//pProfileCred->NetIndex; //Always 1 in 2.0.s
        pPlain += 5;
        PlainLen += 5;
        CredentialLen += 5;

        // 4.2 SSID
        // Length sanity check
        if (pProfileCred->SSID.SsidLength > 32)
            pProfileCred->SSID.SsidLength = 32;
        *((PUSHORT) pPlain) = cpu2be16(WSC_IE_SSID);
        *((PUSHORT) (pPlain + 2)) = cpu2be16(pProfileCred->SSID.SsidLength);    
        PlatformMoveMemory((pPlain + 4), pProfileCred->SSID.Ssid, pProfileCred->SSID.SsidLength);
        pPlain += (4 + pProfileCred->SSID.SsidLength);
        PlainLen += (4 + pProfileCred->SSID.SsidLength);
        CredentialLen += (4 + pProfileCred->SSID.SsidLength);

        // 4.3 Authentication Type, 2 bytes
        *((PUSHORT) pPlain) = cpu2be16(WSC_IE_AUTH_TYPE);
        *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0002);   
        *((PUSHORT) (pPlain + 4)) = cpu2be16(pProfileCred->AuthType);   
        pPlain += 6;
        PlainLen += 6;
        CredentialLen += 6;

        // 4.4 Encryption Type, 2 bytes
        *((PUSHORT) pPlain) = cpu2be16(WSC_IE_ENCR_TYPE);
        *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0002);   
        *((PUSHORT) (pPlain + 4)) = cpu2be16(pProfileCred->EncrType);   
        pPlain += 6;
        PlainLen += 6;
        CredentialLen += 6;

        // 4.5 Network Key Index, 1 byte (optional for StaEncrSettings)
        *((PUSHORT) pPlain) = cpu2be16(WSC_IE_NETWORK_KEY_IDX);
        *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0001);
        // KeyId sanity check
        if (pProfileCred->KeyIndex > 3)
            pProfileCred->KeyIndex = 0;
        // Our key index start from 0 internally    
        *((pPlain + 4)) = 1;//(UCHAR)(pProfileCred->KeyIndex + 1);  //Always 1 in 2.0.
        pPlain += 5;
        PlainLen += 5;
        CredentialLen += 5;

        // 4.6 Network Key
        // Length sanity check
        if (pProfileCred->KeyLength > 64)
            pProfileCred->KeyLength = 64;
        *((PUSHORT) pPlain) = cpu2be16(WSC_IE_NETWORK_KEY);
        *((PUSHORT) (pPlain + 2)) = cpu2be16(pProfileCred->KeyLength);  
        PlatformMoveMemory((pPlain + 4), pProfileCred->Key, pProfileCred->KeyLength);
        pPlain += (4 + pProfileCred->KeyLength);
        PlainLen += (4 + pProfileCred->KeyLength);
        CredentialLen += (4 + pProfileCred->KeyLength);

        // 4.7 MAC Address, 6 bytes
        *((PUSHORT) pPlain) = cpu2be16(WSC_IE_MAC_ADDR);
        *((PUSHORT) (pPlain + 2)) = cpu2be16(0x0006);
        if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
        {
            PlatformMoveMemory((pPlain + 4), pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr, 6);
        }
        else
        {
            PlatformMoveMemory((pPlain + 4), pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr, 6);
        }
        pPlain += 10;
        PlainLen += 10;
        CredentialLen += 10;
        // store enrollee macAddr to the profile
        PlatformMoveMemory(pProfileCred->MacAddr, pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr, 6);
        if (LenPtr != NULL)
            *((PUSHORT) (LenPtr + 2)) = cpu2be16(CredentialLen);    
    }   // max size(136 x 8) = 1088 bytes
    
    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {

        *((PUSHORT) (&PlainText[0])) = cpu2be16(WSC_IE_CREDENTIAL);
        *((PUSHORT) (&PlainText[2])) = cpu2be16(PlainLen - 4);
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("Plain text --> "));  
    for (idx = 0; idx < (PlainLen+4);)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-%02x-%02x-%02x-", PlainText[idx], PlainText[idx+1], PlainText[idx+2], PlainText[idx+3]));
         idx = idx + 4;
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));

    // Generate HMAC
    hmac_sha(pReg->AuthKey, 32, PlainText, PlainLen, Tmp, 32);
    *((PUSHORT) pPlain) = cpu2be16(WSC_IE_KWRAP_AUTH);
    *((PUSHORT) (pPlain + 2)) = cpu2be16(0x008);    
    PlatformMoveMemory((pPlain + 4), Tmp, 8);
    pPlain += 12;
    PlainLen += 12;

    DBGPRINT(RT_DEBUG_TRACE, ("Plain text Hmac --> ")); 
    for (idx = 0; idx < 8; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", Tmp[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));

    // generate a random IV with 16 bytes
    for (idx = 0; idx < 16; idx++)
        IV[idx] = RandomByte(pAd);

    // Encrypt data
    WscEncryptData(pAd, PlainText, PlainLen, pReg->KeyWrapKey, IV, CipherText, &EncrLen);

    //    ================================================================================
    //    End of Encrypted Setting

    // 4. Encrypted Settings
    *((PUSHORT) pData) = cpu2be16(WSC_IE_ENCR_SETTING);
    *((PUSHORT) (pData + 2)) = cpu2be16(16 + EncrLen);  
    PlatformMoveMemory((pData + 4), IV, 16);
    PlatformMoveMemory((pData + 20), CipherText, EncrLen);
    pData += (20 + EncrLen);
    Len   += (20 + EncrLen);

    if(pReg->RegistrarInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }   

    // Save the last two Tx for the peeer retransmission
    pSavedWPSMessage->LastTx2.Length = pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pSavedWPSMessage->LastTx2.Data, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    // Copy the content to Regdata for lasttx information
    pSavedWPSMessage->LastTx.Length = Len;
    PlatformMoveMemory(pSavedWPSMessage->LastTx.Data, OutMsgBuf, Len);

    // 5. Generate authenticator
    // Combine last TX & RX message contents and validate the HMAC
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length);
    pAuth += pSavedWPSMessage->LastRx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);

    // Validate HMAC

    DBGPRINT(RT_DEBUG_TRACE, ("HmacLen = %d\n", HmacLen));
    DBGPRINT(RT_DEBUG_TRACE, ("HmacData --> "));    
    for (idx = 0; idx < (INT) HmacLen; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", HmacData[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));
    
    DBGPRINT(RT_DEBUG_TRACE, ("AuthKey --> ")); 
    for (idx = 0; idx < 32; idx++)
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->AuthKey[idx]));
    }
    DBGPRINT(RT_DEBUG_INFO, ("\n"));
    
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    *((PUSHORT) pData) = cpu2be16(WSC_IE_AUTHENTICATOR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), Tmp, 8);

    // Append the authenticator to last tx buffer for future HMAC calculation
    PlatformMoveMemory(&pSavedWPSMessage->LastTx.Data[pSavedWPSMessage->LastTx.Length], pData, 12);
    pSavedWPSMessage->LastTx.Length += 12;
    pData += 12;
    Len   += 12;


    //  
    //  ********* Start transport *********
    //
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
    {
        // Use Inb-EAP transport
        if(IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(pAd) && (Len > 128))
        {
            pFragData = &OutMsgBuf[0];
            FragRemainLen = Len;
            WscFragmentedEAP(pAd, pPort, Id, WSC_OPCODE_MSG, pElem, TRUE);
        }
        else
            WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_MSG, OutMsgBuf, Len, pElem, FALSE, FALSE);
    }
    else    // WSC_ConfMode_REGISTRAR_UPNP
    {
        // Use Inb-UPnP transport
        DBGPRINT(RT_DEBUG_TRACE, ("ExtReg Set M8 Event!!\n"));
        RTMPSetExtRegMessageEvent(pAd, EXTREG_MESSAGE_EVENT_ID, OutMsgBuf, Len);
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M8_SENT; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendMessageM8\n"));

}

/*
    ========================================================================
    
    Routine Description:
        Send WSC Eap-Rsp-Ack

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M2D(Enrollee) and WSC_Done(Registrar)
        1. Change the correct parameters
        2. Send Eap-Rsp-Ack
        
    ========================================================================
*/
VOID    WscSendEapRspAck(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    PUCHAR              pData;
    INT                 Len;
    PWSC_DEV_INFO       pDevInfo;   
    // Send EAPOL-Response-Ack
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendEapRspAck\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;
    

    // Role play, Enrollee or Registrar
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
        pDevInfo = (PWSC_DEV_INFO) &pReg->EnrolleeInfo;
    else
        pDevInfo = (PWSC_DEV_INFO) &pReg->RegistrarInfo;

    // Prepare ACK message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;

    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pDevInfo->Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_WSC_ACK;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;

    // 4. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
        PlatformMoveMemory((pData + 4), pReg->M2DRegistrarNonce, 16);
    else
        PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);  
    pData += 20;
    Len   += 20;

    if(pReg->RegistrarInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = 0x01;
        *(pData + 2) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }   

    //
    //  ********* Start inband EAP transport *********
    //
    WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_ACK, OutMsgBuf, Len, pElem, FALSE, FALSE);

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_ACK_SENT;    

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendEapRspAck\n"));    
}

VOID    WscSendEapFragAck(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    // Send EAPOL-Response-Ack
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendEapFragAck\n"));

    //
    //  ********* Start inband EAP transport *********
    //
    WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_FRAG_ACK, NULL, 0, pElem, FALSE, FALSE);

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_ACK_SENT;    

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendEapRspAck\n"));    
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC Eap-Rsp-Nack

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        If the supplicant encounters an error authenticating or 
        processing a message, send Eap-Rsp-Nack to the authenticator
        
    ========================================================================
*/
VOID    WscSendEapRspNack(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    PUCHAR              pData;
    INT                 Len;
    PWSC_DEV_INFO       pDevInfo;
    
    // Send EAPOL-Response-Nack
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendEapRspNack\n"));
    DBGPRINT(RT_DEBUG_TRACE,(" . 1 PIN = %x  %x. 2 PIN = %x  %x\n", *(PULONG)&pPort->P2PCfg.PinCode[0], *(PULONG)&pPort->P2PCfg.PinCode[4], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;
    
    
    // Role play, Enrollee or Registrar
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
        pDevInfo = (PWSC_DEV_INFO) &pReg->EnrolleeInfo;
    else
        pDevInfo = (PWSC_DEV_INFO) &pReg->RegistrarInfo;


    // Prepare the M5 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pDevInfo->Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_WSC_NACK;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;
    
    // 4. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);
    pData += 20;
    Len   += 20;

    // 5. Configure Error
    *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_ERROR);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pData + 4)) = pDevInfo->ConfigError;   // send error code to the peer
    pData += 6;
    Len   += 6;

    if(pReg->RegistrarInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }   

    if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR) ||
        (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE))
    {
        //
        //  ********* Start inband EAP transport *********
        //
        WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_NACK, OutMsgBuf, Len, pElem, FALSE, FALSE);
    }
    else    // WSC_ConfMode_REGISTRAR_UPNP
    {
        // Use Inb-UPnP transport
        DBGPRINT(RT_DEBUG_TRACE, ("ExtReg Send Eap NAK!!\n"));
        RTMPSetExtRegMessageEvent(pAd, EXTREG_MESSAGE_EVENT_ID, OutMsgBuf, Len);
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_NACK_SENT;   
    if (pPort->StaCfg.WscControl.WscConfMode!= WSC_ConfMode_REGISTRAR_UPNP)
        P2pReceiveEapNack(pAd, pElem);
    
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendEapRspNack\n"));   
}

/*
    ========================================================================
    
    Routine Description:
        Send WSC Eap-Rsp-Done

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx M8
        1. Change the correct parameters
        2. Send Eap-Rsp-Done
        
    ========================================================================
*/
VOID    WscSendEapRspDone(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PWSC_REG_DATA       pReg;
    PUCHAR              pData;
    INT                 Len;
    
    // Send EAPOL-Response-Done
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscSendEapRspDone\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Prepare the M5 message body after WSC Frame header
    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pReg->EnrolleeInfo.Version;
    pData += 5;
    Len   += 5;

    // 2. Message Type, M
    *((PUSHORT) pData) = cpu2be16(WSC_IE_MSG_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_MSG_WSC_DONE;
    pData += 5;
    Len   += 5;

    // 3. Enrollee Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_EN_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->EnrolleeNonce, 16);
    pData += 20;
    Len   += 20;
    
    // 4. Registrar Nonce, 16 bytes
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_NONCE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pReg->RegistrarNonce, 16);
    pData += 20;
    Len   += 20;

    if(pReg->EnrolleeInfo.Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        // 23. Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = 0x01;
        *(pData + 2) = pReg->RegistrarInfo.Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;


        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }   

    //
    //  ********* Start inband EAP transport *********
    //
    WscMakeEAPFrame(pAd, pPort, Id, WSC_OPCODE_DONE, OutMsgBuf, Len, pElem, FALSE, FALSE);

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_DONE_SENT;   
    
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscSendEapRspDone\n"));   
}

/*
    ========================================================================
    
    Routine Description:
        Receive M1 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build M2
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM1(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg)
{
    USHORT              Length;
    PUCHAR              pData;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    USHORT              WscType, WscLen;
    PSAVED_WPS_MESSAGE  pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    PUCHAR              pVendorExtType;
    UCHAR               VendorExtElemLen = 0;
    USHORT              VendorExtLen = 0;
    INT                 i;


    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM1\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;
    
    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        if(pExtRegMsg)
        {
            pData = pExtRegMsg->MsgData;
            Length = pExtRegMsg->Length;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WSC External Registrar -- No ExtReg M1 in WscRecvMessageM1\n"));
            return FALSE;
        }
    }
    else
    {
        // Skip all the EAP headers since we already parse the correct M1 type earlier
        // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

        // Skip the EAP LLC header check since it's been checked many times.
        pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
        pData += sizeof(IEEE8021X_FRAME);

        // start of 802.1x frame header
        p802_1x = (PIEEE8021X_FRAME) pData;
        Length = be2cpu16(p802_1x->Length);
        
        // Length include EAP 5 bytes header
        Length -= sizeof(EAP_FRAME);

        // Skip WSC Frame fixed header
        Length -= sizeof(WSC_FRAME);
        pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

        // Copy the content to Regdata for lastRx information
        // Length must include authenticator IE size
        if (Length >= WSC_MAX_RX_BUFFER)
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
            DBGPRINT_ERR(("WscRecvMessageM1 --> M1 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
            return FALSE;
        }
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    //Receive M1
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M1_RECEIVED;

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M1 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:        /* 1 */
                pReg->EnrolleeInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:       /* 2 */
                // Do nothing since we already parse it before 
                break;
    
            case WSC_IE_UUID_E:         /* 3 */
                PlatformMoveMemory(pReg->EnrolleeInfo.Uuid, pData, 16);
                break;
                                
            case WSC_IE_MAC_ADDR:       /* 4 */
                PlatformMoveMemory(pReg->EnrolleeInfo.MacAddr, pData, 6);
                break;
    
            case WSC_IE_EN_NONCE:       /* 5 */
                PlatformMoveMemory(pReg->EnrolleeNonce, pData, 16);
                break;
                                
            case WSC_IE_PUBLIC_KEY:     /* 6 */
                // Get Enrollee Public Key
                PlatformMoveMemory(pReg->Pke, pData, 192);
                break;
                
            case WSC_IE_AUTH_TYPE_FLAG: /* 7 */
                pReg->EnrolleeInfo.AuthTypeFlags = *((PUSHORT) pData);
                break;
                
            case WSC_IE_ENCR_TYPE_FLAG: /* 8 */
                pReg->EnrolleeInfo.EncrTypeFlags = *((PUSHORT) pData);
                break;
                
            case WSC_IE_CONN_TYPE_FLAG: /* 9 */
                pReg->EnrolleeInfo.ConnTypeFlags = *pData;
                break;
                
            case WSC_IE_CONF_MTHD:      /* 10 */
                pReg->EnrolleeInfo.ConfigMethods = *((PUSHORT) pData);
                break;

            case WSC_IE_STATE:          /* 11 */
                pReg->EnrolleeInfo.ScState = *pData;
                if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR &&
                    pReg->EnrolleeInfo.ScState == WSC_APSTATE_CONFIGURED)
                {
                    if (!PORT_P2P_ON(pPort))
                    {   
                        // Don't overwrite the credential of M7 received from AP when this flag is TRUE!
                        pPort->StaCfg.WscControl.bConfiguredAP = TRUE;
                        DBGPRINT(RT_DEBUG_TRACE, (" !!!!! The peer is a configured AP as an Enrollee !!!!!! \n"));
                    }
                    else
                        DBGPRINT(RT_DEBUG_TRACE, (" !!!!!P2P doesn't enable configured AP as an Enrollee !!!!!! \n"));
                }
                break;
    
            case WSC_IE_MANUFACTURER:   /* 12 */
                PlatformMoveMemory(pReg->EnrolleeInfo.Manufacturer, pData, WscLen);
                break;
                
            case WSC_IE_MODEL_NAME:     /* 13 */
                PlatformMoveMemory(pReg->EnrolleeInfo.ModelName, pData, WscLen);
                break;
                
            case WSC_IE_MODEL_NO:       /* 14 */
                PlatformMoveMemory(pReg->EnrolleeInfo.ModelNumber, pData, WscLen);
                break;
                
            case WSC_IE_SERIAL:         /* 15 */
                PlatformMoveMemory(pReg->EnrolleeInfo.SerialNumber, pData, WscLen);
                break;
                
            case WSC_IE_PRI_DEV_TYPE:   /* 16 */
                PlatformMoveMemory(pReg->EnrolleeInfo.PriDeviceType, pData, 8);
                break;
                
            case WSC_IE_DEV_NAME:       /* 17 */
                PlatformMoveMemory(pReg->EnrolleeInfo.DeviceName, pData, WscLen);
                break;
                
            case WSC_IE_RF_BAND:        /* 18 */
                pReg->EnrolleeInfo.RfBand = *pData;
                break;
                
            case WSC_IE_ASSOC_STATE:    /* 19 */
                pReg->EnrolleeInfo.AssocState = *((PUSHORT) pData);
                break;
                
            case WSC_IE_DEV_PASS_ID:    /* 20 */
                pReg->EnrolleeInfo.DevPwdId= *((PUSHORT) pData);
                if (pReg->EnrolleeInfo.DevPwdId != pReg->RegistrarInfo.DevPwdId)
                    DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM1 --> Device Password ID mismatched !!!"));
                break;
                
            case WSC_IE_CONF_ERROR:     /* 21 */
                pReg->EnrolleeInfo.ConfigError = *((PUSHORT) pData);
                if (pReg->EnrolleeInfo.ConfigError != cpu2be16(WSC_ERROR_NO_ERROR))
                    DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM1 --> ConfigError 0x%04x\n", pReg->EnrolleeInfo.ConfigError));       
                break;
                
            case WSC_IE_OS_VER:         /* 22 */
                pReg->EnrolleeInfo.OsVersion = *((PULONG) pData);
                break;
            case WSC_IE_VENDOR_EXT:
                VendorExtLen = WscLen;
                if (((*(pData) * 256 + *(pData+1)) * 256 + (*(pData+2))) == WSC_SMI)
                {
                    VendorExtLen -= 3;  //WSC SMI
                    pVendorExtType = pData;
                    pVendorExtType += 3;
                    while(VendorExtLen > 2)
                    {
                        VendorExtElemLen = *(pVendorExtType+1);
                                            
                        switch(*pVendorExtType)
                        {
                            case WSC_IE_VERSION2:
                                pReg->EnrolleeInfo.Version2 = *(pVendorExtType+2);
                                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_VENDOR_EXT Parse Version2 type 0x%x!!\n", pReg->EnrolleeInfo.Version));
                                break;
                            default:
                                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_VENDOR_EXT Parse UNKNOWN type!!\n"));
                                break;
                        }
                        pVendorExtType += (VendorExtElemLen + 2);
                        VendorExtLen -= (VendorExtElemLen + 2);
                    }
                }
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM1 --> Unknown IE 0x%04x\n", WscType));       
                break;              
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP) && (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE))
    {
        INT IdxPbcEnr = 0;
        for(i = 0; i < PBC_ENR_TAB_SIZE; i++)
        {
            if((pPort->StaCfg.WscControl.WscPbcEnrTab[i].Valid) &&
                (PlatformEqualMemory(pReg->EnrolleeInfo.MacAddr, pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr, MAC_ADDR_LEN)))
            {
                IdxPbcEnr = i;
                break;
            }
        }
        if(!PlatformEqualMemory(pReg->EnrolleeInfo.Uuid, pPort->StaCfg.WscControl.WscPbcEnrTab[IdxPbcEnr].UUID_E, 16))
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EXTREG_MULTI_PBC_SESSIONS_DETECT;
            DBGPRINT(RT_DEBUG_TRACE, ("%s UUID-E of ProbReq and M1 are NOT match !! ProbReq:%x %x %x.., M1:%x %x %x..\n", __FUNCTION__,
                pPort->StaCfg.WscControl.WscPbcEnrTab[0].UUID_E[0], pPort->StaCfg.WscControl.WscPbcEnrTab[0].UUID_E[1], pPort->StaCfg.WscControl.WscPbcEnrTab[0].UUID_E[2],
                pReg->EnrolleeInfo.Uuid[0], pReg->EnrolleeInfo.Uuid[1], pReg->EnrolleeInfo.Uuid[2]));
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM1\n"));        
    return TRUE;

}

/*
    ========================================================================
    
    Routine Description:
        Receive M2 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build M3
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM2(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    USHORT              Length;
    ULONG               HmacLen;
    PUCHAR              pData, pAuth;
    UCHAR               Hmac[8], DHKey[32], KDK[32], KdkInput[38], KdfKey[80];
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    USHORT              WscType, WscLen;
    INT                 DH_Len;
    UCHAR               version2 = 0x10;
    PUCHAR              pVendorExtType;
    UCHAR               VendorExtElemLen = 0;
    USHORT              VendorExtLen = 0;

    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM2\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Skip all the EAP headers since we already parse the correct M2 type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);
    
    // Length include EAP 5 bytes header
    Length -= sizeof(EAP_FRAME);

    // Skip WSC Frame fixed header
    Length -= sizeof(WSC_FRAME);
    pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

    // Copy the content to Regdata for lastRx information
    // Length must include authenticator IE size
    if (Length >= WSC_MAX_RX_BUFFER)
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
        DBGPRINT_ERR(("WscRecvMessageM2 --> M2 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
        return FALSE;
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M2 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:        /* 1 */
                pReg->RegistrarInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:       /* 2 */
                // Do nothing since we already parse it before 
                break;

            case WSC_IE_EN_NONCE:       /* 3 */
                // for verification with our enrollee nonce
                if (MtkCompareMemory(&pReg->EnrolleeNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M2 Compare enrollee nonce mismatched \n"));      
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;  
                    return FALSE;
                }
                break;

            case WSC_IE_REG_NONCE:      /* 4 */
                PlatformMoveMemory(pReg->RegistrarNonce, pData, 16);
                break;

            case WSC_IE_UUID_R:         /* 5 */
                PlatformMoveMemory(pReg->RegistrarInfo.Uuid, pData, 16);
                break;

            case WSC_IE_PUBLIC_KEY:     /* 6*/
                // Get Registrar Public Key
                PlatformMoveMemory(pReg->Pkr, pData, 192);
                break;

            case WSC_IE_AUTH_TYPE_FLAG: /* 7 */
                pReg->RegistrarInfo.AuthTypeFlags = *((PUSHORT) pData);
                break;

            case WSC_IE_ENCR_TYPE_FLAG: /* 8 */
                pReg->RegistrarInfo.EncrTypeFlags = *((PUSHORT) pData);
                break;

            case WSC_IE_CONN_TYPE_FLAG: /* 9 */
                pReg->RegistrarInfo.ConnTypeFlags = *pData;
                break;

            case WSC_IE_CONF_MTHD:      /* 10 */
                pReg->RegistrarInfo.ConfigMethods = *((PUSHORT) pData);
                break;

            case WSC_IE_MANUFACTURER:   /* 11 */
                PlatformMoveMemory(pReg->RegistrarInfo.Manufacturer, pData, WscLen);
                break;

            case WSC_IE_MODEL_NAME:     /* 12 */
                PlatformMoveMemory(pReg->RegistrarInfo.ModelName, pData, WscLen);
                break;

            case WSC_IE_MODEL_NO:       /* 13 */
                PlatformMoveMemory(pReg->RegistrarInfo.ModelNumber, pData, WscLen);
                break;

            case WSC_IE_SERIAL:         /* 14 */
                PlatformMoveMemory(pReg->RegistrarInfo.SerialNumber, pData, WscLen);
                break;

            case WSC_IE_PRI_DEV_TYPE:   /* 15 */
                PlatformMoveMemory(pReg->RegistrarInfo.PriDeviceType, pData, 8);
                break;

            case WSC_IE_DEV_NAME:       /* 16 */
                PlatformMoveMemory(pReg->RegistrarInfo.DeviceName, pData, WscLen);
                break;

            case WSC_IE_RF_BAND:        /* 17 */
                pReg->RegistrarInfo.RfBand = *pData;
                break;

            case WSC_IE_ASSOC_STATE:    /* 18 */
                pReg->RegistrarInfo.AssocState = *((PUSHORT) pData);
                break;

            case WSC_IE_CONF_ERROR:     /* 19 */
                pReg->RegistrarInfo.ConfigError = *((PUSHORT) pData);
                if (pReg->RegistrarInfo.ConfigError != cpu2be16(WSC_ERROR_NO_ERROR))
                    DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM2 --> ConfigError 0x%04x\n", pReg->RegistrarInfo.ConfigError));      
                break;

            case WSC_IE_DEV_PASS_ID:    /* 20 */
                pReg->RegistrarInfo.DevPwdId= *((PUSHORT) pData);
                if (pReg->EnrolleeInfo.DevPwdId != pReg->RegistrarInfo.DevPwdId)
                    DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM2 --> Device Password ID mismatched !!!"));
                break;

            case WSC_IE_OS_VER:         /* 21 */
                pReg->RegistrarInfo.OsVersion = *((PULONG) pData);
                break;

            case WSC_IE_AUTHENTICATOR:  /* 22 */
                PlatformMoveMemory(Hmac, pData, 8);
                break;

            case WSC_IE_VENDOR_EXT:
                VendorExtLen = WscLen;
                if (((*(pData) * 256 + *(pData+1)) * 256 + (*(pData+2))) == WSC_SMI)
                {
                    VendorExtLen -= 3;  //WSC SMI
                    pVendorExtType = pData;
                    pVendorExtType += 3;
                    while(VendorExtLen > 2)
                    {
                        VendorExtElemLen = *(pVendorExtType+1);
                                            
                        switch(*pVendorExtType)
                        {
                            case WSC_IE_VERSION2:
                                pPort->StaCfg.WscControl.WSCRegistrarVersion = *(pVendorExtType+2);
                                pReg->RegistrarInfo.Version2 = *(pVendorExtType+2);
                                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_VENDOR_EXT Parse Version2 type 0x%x!!\n", pReg->EnrolleeInfo.Version));
                                break;
                            default:
                                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_VENDOR_EXT Parse UNKNOWN type!!\n"));
                                break;
                        }
                        pVendorExtType += (VendorExtElemLen + 2);
                        VendorExtLen -= (VendorExtElemLen + 2);
                    }
                }
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM2 --> Unknown IE 0x%04x\n", WscType));       
                break;
        }

        // Offset to next WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Let's start to generate keys and verify the data payload
    // Generate Secret Key
    //==========================================================================
    GenerateDHSecreteKey(&pReg->DHRandom[0], 192, &pReg->Pkr[0], 192, &pReg->SecretKey[0], &DH_Len);

    // Compute the DHKey based on the DH secret
    sha256(&DHKey[0], &pReg->SecretKey[0], 192);

    // Create KDK input data
    PlatformMoveMemory(&KdkInput[0], pReg->EnrolleeNonce, 16);
        
    PlatformMoveMemory(&KdkInput[16], pReg->EnrolleeInfo.MacAddr, 6);
        
    PlatformMoveMemory(&KdkInput[22], pReg->RegistrarNonce, 16);
    
    // Generate the KDK
    hmac_sha(DHKey, 32,  KdkInput, 38, KDK, 32);

    // KDF
    WscDeriveKey(KDK, 32, Wsc_Personal_String, (sizeof(Wsc_Personal_String) - 1), KdfKey, 640);

    // Assign Key from KDF
    PlatformMoveMemory(pReg->AuthKey, &KdfKey[0], 32);
    PlatformMoveMemory(pReg->KeyWrapKey, &KdfKey[32], 16);
    PlatformMoveMemory(pReg->Emsk, &KdfKey[48], 32);
    // End of Key Generation====================================================

    // Combine last TX & RX message contents and validate the HMAC
    // We have to exclude last 12 bytes from last receive since it's authenticator value
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length - 12;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    pAuth += pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length - 12);

    // Validate HMAC, reuse KDK buffer
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, KDK, 32);

    if (PlatformEqualMemory(Hmac, KDK, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM2 --> HMAC not match\n"));    
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &KDK[0])), cpu2be32(*((PULONG) &KDK[4]))));   
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;    
        return FALSE;
    }
    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M2_RECEIVED; 

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM2\n"));
    return TRUE;
    
}

/*
    ========================================================================
    
    Routine Description:
        Receive M2D message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build Eap-Rsp-Ack
        3. Wait EAP-Fail to re-enter start state
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM2D(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    USHORT              Length;
    PUCHAR              pData;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    USHORT              WscType, WscLen;

    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM2D\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Skip all the EAP headers since we already parse the correct M2 type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);
    
    // Length include EAP 5 bytes header
    Length -= sizeof(EAP_FRAME);

    // Skip WSC Frame fixed header
    Length -= sizeof(WSC_FRAME);
    pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M2D WSC type and store to RegData structure
        switch (WscType)
        {       
            case WSC_IE_EN_NONCE:
                // for verification with our enrollee nonce
                if (MtkCompareMemory(&pReg->EnrolleeNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M2D: It's not our own Enrollee Nonce!!! \n"));
                    return FALSE;
                }
                break;
                
            case WSC_IE_REG_NONCE:
                PlatformMoveMemory(pReg->M2DRegistrarNonce, pData, 16);
                break;
                
            case WSC_IE_MSG_TYPE:
                // Do nothing since we already parse it before 
                break;
                
            // M2D might be not the candidate Registrar we want to connect
            // Parse them on M2 later !!
            case WSC_IE_VERSION:
            case WSC_IE_UUID_R:
            case WSC_IE_AUTH_TYPE_FLAG:
            case WSC_IE_ENCR_TYPE_FLAG:
            case WSC_IE_CONN_TYPE_FLAG:
            case WSC_IE_CONF_MTHD:
            case WSC_IE_MANUFACTURER:
            case WSC_IE_MODEL_NAME:
            case WSC_IE_MODEL_NO:
            case WSC_IE_SERIAL:
            case WSC_IE_PRI_DEV_TYPE:
            case WSC_IE_DEV_NAME:
            case WSC_IE_RF_BAND:
            case WSC_IE_ASSOC_STATE:
            case WSC_IE_CONF_ERROR:
            case WSC_IE_DEV_PASS_ID:
            case WSC_IE_OS_VER:
                break;
                
            case WSC_IE_PUBLIC_KEY:
                // There shall be no Public transmitted in M2D
                break;
                
            case WSC_IE_AUTHENTICATOR:
                // No authenticator in M2D
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM2D --> Unknown IE 0x%04x\n", WscType));      
                break;
                
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M2D_RECEIVED;    

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM2D\n"));       

    return TRUE;
}

/*
    ========================================================================
    
    Routine Description:
        Receive M3 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build M4
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM3(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg)
{
    USHORT              Length;
    ULONG               HmacLen;
    PUCHAR              pData, pAuth;
    UCHAR               Hmac[8], Tmp[512];
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    USHORT              WscType, WscLen;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM3\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        if(pExtRegMsg)
        {
            pData = pExtRegMsg->MsgData;
            Length = pExtRegMsg->Length;
            if (Length >= WSC_MAX_RX_BUFFER)
            {
                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                DBGPRINT_ERR(("WscRecvMessageM3 --> M3 is too long (=%d)\n", Length));
                return FALSE;
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WSC External Registrar -- No ExtReg M3 in WscRecvMessageM1\n"));
            return FALSE;
        }
    }
    else
    {
        // Skip all the EAP headers since we already parse the correct M2 type earlier
        // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

        // Skip the EAP LLC header check since it's been checked many times.
        pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
        pData += sizeof(IEEE8021X_FRAME);

        // start of 802.1x frame header
        p802_1x = (PIEEE8021X_FRAME) pData;
        Length = be2cpu16(p802_1x->Length);
        
        // Length include EAP 5 bytes header
        Length -= sizeof(EAP_FRAME);

        // Skip WSC Frame fixed header
        Length -= sizeof(WSC_FRAME);
        pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

        // Copy the content to Regdata for lastRx information
        // Length must include authenticator IE size
        if (Length >= WSC_MAX_RX_BUFFER)
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
            DBGPRINT_ERR(("WscRecvMessageM3 --> M3 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
            return FALSE;
        }
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M3 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:        /* 1 */
                pReg->EnrolleeInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:       /* 2 */
                // Do nothing since we already parse it before 
                break;

            case WSC_IE_REG_NONCE:      /* 3 */
                // for verification with our registrar nonce
                if (MtkCompareMemory(&pReg->RegistrarNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M3 Compare registrar nonce mismatched \n"));     
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;
                    return FALSE;
                }
                break;
                
            case WSC_IE_E_HASH_1:       /* 4 */
                // Verify it in Received M5
                PlatformMoveMemory(pReg->EHash1, pData, 32);
                break;
                
            case WSC_IE_E_HASH_2:       /* 5 */
                // Verify it in Received M7
                PlatformMoveMemory(pReg->EHash2, pData, 32);
                break;
                
            case WSC_IE_AUTHENTICATOR:  /* 6 */
                PlatformMoveMemory(Hmac, pData, 8);
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM3 --> Unknown IE 0x%04x\n", WscType));       
                break;              
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }


    // Combine last TX & RX message contents and validate the HMAC
    // We have to exclude last 12 bytes from last receive since it's authenticator value
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length - 12;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    pAuth += pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length - 12);

    // Validate HMAC, reuse KDK buffer
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);

    if (PlatformEqualMemory(Hmac, Tmp, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM3 --> HMAC not match\n"));    
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Tmp[0])), cpu2be32(*((PULONG) &Tmp[4]))));   
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));         

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
        return FALSE;
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M3_RECEIVED;
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM3\n"));
    return TRUE;

}

/*
    ========================================================================
    
    Routine Description:
        Receive M4 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build M5
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM4(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    USHORT              Length;
    ULONG               HmacLen;
    PUCHAR              pData, pAuth;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    UCHAR               Hmac[8], Tmp[512], RHash[32];
    INT                 EncrLen, idx;
    USHORT              WscType, WscLen;
    INT                 Status;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM4\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Skip all the EAP headers since we already parse the correct M2 type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);
    
    // Length include EAP 5 bytes header
    Length -= sizeof(EAP_FRAME);

    // Skip WSC Frame fixed header
    Length -= sizeof(WSC_FRAME);
    pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

    // Copy the content to Regdata for lastRx information
    // Length must include authenticator IE size
    if (Length >= WSC_MAX_RX_BUFFER)
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
        DBGPRINT_ERR(("WscRecvMessageM4 --> M4 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
        return FALSE;
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M4 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:        /* 1 */
                pReg->RegistrarInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:       /* 2 */
                // Do nothing since we already parse it before 
                break;
    
            case WSC_IE_EN_NONCE:       /* 3 */
                // for verification with our enrollee nonce
                if (MtkCompareMemory(&pReg->EnrolleeNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M4 Compare enrollee nonce mismatched \n"));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;
                    // return after password check
                }
                break;
                
            case WSC_IE_R_HASH_1:       /* 4 */
                // Verify it in Received M4
                PlatformMoveMemory(pReg->RHash1, pData, 32);
                break;
                
            case WSC_IE_R_HASH_2:       /* 5 */
                // Verify it in Received M6
                PlatformMoveMemory(pReg->RHash2, pData, 32);
                break;

            case WSC_IE_ENCR_SETTING:   /* 6 */
                // There shall have smoe kind of length check
                if ((WscLen <= 16) || (WscLen > 1024))
                {
                    // ApEncrSetting is not enough
                    DBGPRINT(RT_DEBUG_ERROR, ("EncrSetting length is out of range(16~1024), require %d\n", WscLen));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                    break;
                }

                WscDecryptData(pAd, (pData + 16), (WscLen - 16), pReg->KeyWrapKey, pData, pReg->ApEncrSettings, &EncrLen);
                DBGPRINT(RT_DEBUG_TRACE, ("ApEncrSettings len = %d, data --> ", EncrLen));
                
                for (idx = 0; idx < EncrLen; idx++)
                {
                    DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->ApEncrSettings[idx]));
                }
                DBGPRINT(RT_DEBUG_INFO, ("\n"));        

                // Parse encryption settings
                Status = WscParseEncrSettings(pAd, pPort, pReg->ApEncrSettings, EncrLen);
                if (Status == FALSE)
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;

                break;
                
            case WSC_IE_AUTHENTICATOR:  /* 7 */
                PlatformMoveMemory(Hmac, pData, 8);
                break;

                
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM4 --> Unknown IE 0x%04x\n", WscType));       
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Verify R-Hash1
    // Create input for R-Hash1
    PlatformMoveMemory(Tmp, pReg->Rs1, 16);
    PlatformMoveMemory(&Tmp[16], pReg->Psk1, 16);
    PlatformMoveMemory(&Tmp[32], pReg->Pke, 192);
    PlatformMoveMemory(&Tmp[224], pReg->Pkr, 192);
    
    // Generate R-Hash1
    hmac_sha(pReg->AuthKey, 32, Tmp, 416, RHash, 32);
    
    if (MtkCompareMemory(pReg->RHash1, RHash, 32) != 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM4 --> RHash1 not matched\n"));       

        // Error Psk1 (device password related)  
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_PASSWORD_MISMATCH;
        return FALSE;
    }
    
    // Combine last TX & RX message contents and validate the HMAC
    // We have to exclude last 12 bytes from last receive since it's authenticator value
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length - 12;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    pAuth += pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length - 12);

    // Validate HMAC, reuse Tmp buffer
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    if (PlatformEqualMemory(Hmac, Tmp, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM4 --> HMAC not match\n"));    
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Tmp[0])), cpu2be32(*((PULONG) &Tmp[4]))));   
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));         

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
    }
    
    // err status
    if (pPort->StaCfg.WscControl.WscStatus & 0x0F00)
        return FALSE;
    else
    {
        // Update WSC status
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M4_RECEIVED;
        DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM4\n"));
        return TRUE;
    }

}

/*
    ========================================================================
    
    Routine Description:
        Receive M5 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build M6
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM5(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg)
{
    USHORT              Length;
    ULONG               HmacLen;
    PUCHAR              pData, pAuth;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    UCHAR               Hmac[8], Tmp[512], EHash[32];
    INT                 EncrLen, idx;
    USHORT              WscType, WscLen;
    INT                 Status = 0;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM5\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;
    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        if(pExtRegMsg)
        {
            pData = pExtRegMsg->MsgData;
            Length = pExtRegMsg->Length;
            if (Length >= WSC_MAX_RX_BUFFER)
            {
                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                DBGPRINT_ERR(("WscRecvMessageM5 --> M5 is too long (=%d)\n", Length));
                return FALSE;
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WSC External Registrar -- No ExtReg M5 in WscRecvMessageM5\n"));
            return FALSE;
        }
    }
    else
    {
        // Skip all the EAP headers since we already parse the correct M2 type earlier
        // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

        // Skip the EAP LLC header check since it's been checked many times.
        pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
        pData += sizeof(IEEE8021X_FRAME);

        // start of 802.1x frame header
        p802_1x = (PIEEE8021X_FRAME) pData;
        Length = be2cpu16(p802_1x->Length);
        
        // Length include EAP 5 bytes header
        Length -= sizeof(EAP_FRAME);

        // Skip WSC Frame fixed header
        Length -= sizeof(WSC_FRAME);
        pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

        // Copy the content to Regdata for lastRx information
        // Length must include authenticator IE size
        if (Length >= WSC_MAX_RX_BUFFER)
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
            DBGPRINT_ERR(("WscRecvMessageM5 --> M5 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
            return FALSE;
        }
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M5 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:        /* 1 */
                pReg->EnrolleeInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:       /* 2 */
                // Do nothing since we already parse it before 
                break;
    
            case WSC_IE_REG_NONCE:      /* 3 */
                // for verification with our registrar nonce
                if (MtkCompareMemory(&pReg->RegistrarNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M5 Compare registrar nonce mismatched \n"));     
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;
                    // return after password check
                }
                break;
                
            case WSC_IE_ENCR_SETTING:   /* 4 */
                // There shall have smoe kind of length check
                if ((WscLen <= 16) || (WscLen > 1024))
                {
                    // ApEncrSetting is not enough
                    DBGPRINT(RT_DEBUG_ERROR, ("EncrSetting length is out of range(16~1024), require %d\n", WscLen));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                    break;
                }
    
                WscDecryptData(pAd, (pData + 16), (WscLen - 16), pReg->KeyWrapKey, pData, pReg->ApEncrSettings, &EncrLen);
                DBGPRINT(RT_DEBUG_INFO, ("EncrSettings len = %d, data --> ", EncrLen));
                
                for (idx = 0; idx < EncrLen; idx++)
                {
                    DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->ApEncrSettings[idx]));
                }
                DBGPRINT(RT_DEBUG_INFO, ("\n"));        

                // Parse encryption settings
                Status = WscParseEncrSettings(pAd, pPort, pReg->ApEncrSettings, EncrLen);
                if (Status == FALSE)
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;

                break;
                
            case WSC_IE_AUTHENTICATOR:  /* 5 */
                PlatformMoveMemory(Hmac, pData, 8);
                break;
    
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM5 --> Unknown IE 0x%04x\n", WscType));
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Verify E-Hash1
    // Create input for E-Hash1
    PlatformMoveMemory(Tmp, pReg->Es1, 16);
    PlatformMoveMemory(&Tmp[16], pReg->Psk1, 16);
    PlatformMoveMemory(&Tmp[32], pReg->Pke, 192);
    PlatformMoveMemory(&Tmp[224], pReg->Pkr, 192);
    
    // Generate E-Hash1
    hmac_sha(pReg->AuthKey, 32, Tmp, 416, EHash, 32);
    
    if (MtkCompareMemory(pReg->EHash1, EHash, 32) != 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM5 --> EHash1 not matched\n"));       

        // Error Psk1 (device password related)  
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_PASSWORD_MISMATCH;
        return FALSE;
    }
    
    // Combine last TX & RX message contents and validate the HMAC
    // We have to exclude last 12 bytes from last receive since it's authenticator value
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length - 12;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    pAuth += pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length - 12);

    // Validate HMAC, reuse Tmp buffer
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    if (PlatformEqualMemory(Hmac, Tmp, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM5 --> HMAC not match\n"));    
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Tmp[0])), cpu2be32(*((PULONG) &Tmp[4]))));   
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
    }
    
    // err status
    if (pPort->StaCfg.WscControl.WscStatus & 0x0F00)
        return FALSE;
    else
    {
        // Update WSC status
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M5_RECEIVED;
        DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM5\n"));
        return TRUE;
    }

}

/*
    ========================================================================
    
    Routine Description:
        Receive M6 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build M7
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM6(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    USHORT              Length;
    PUCHAR              pData, pAuth;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    INT                 EncrLen, idx;
    UCHAR               Hmac[8], Tmp[512], RHash[32];
    ULONG               HmacLen;
    USHORT              WscType, WscLen;
    INT                 Status = 0;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM6\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Skip all the EAP headers since we already parse the correct M2 type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);
    
    // Length include EAP 5 bytes header
    Length -= sizeof(EAP_FRAME);

    // Skip WSC Frame fixed header
    Length -= sizeof(WSC_FRAME);
    pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

    // Copy the content to Regdata for lastRx information
    // Length must include authenticator IE size
    if (Length >= WSC_MAX_RX_BUFFER)
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
        DBGPRINT_ERR(("WscRecvMessageM6 --> M6 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
        return FALSE;
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M6 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:        /* 1 */
                pReg->RegistrarInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:       /* 2 */
                // Do nothing since we already parse it before 
                break;
    
            case WSC_IE_EN_NONCE:       /* 3 */
                // for verification with our enrollee nonce
                if (MtkCompareMemory(&pReg->EnrolleeNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M6 Compare enrollee nonce mismatched \n"));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;
                    // return after password check
                }
                break;
                
            case WSC_IE_ENCR_SETTING:   /* 4 */
                // There shall have smoe kind of length check
                if ((WscLen <= 16) || (WscLen > 1024))
                {
                    // ApEncrSetting is not enough
                    DBGPRINT(RT_DEBUG_ERROR, ("EncrSetting length is out of range(16~1024), require %d\n", WscLen));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                    break;
                }

                WscDecryptData(pAd, (pData + 16), (WscLen - 16), pReg->KeyWrapKey, pData, pReg->ApEncrSettings, &EncrLen);
                DBGPRINT(RT_DEBUG_TRACE, ("ApEncrSettings len = %d, data --> ", EncrLen));
                
                for (idx = 0; idx < EncrLen; idx++)
                {
                    DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->ApEncrSettings[idx]));
                }
                DBGPRINT(RT_DEBUG_INFO, ("\n"));        

                // Parse encryption settings
                Status = WscParseEncrSettings(pAd, pPort, pReg->ApEncrSettings, EncrLen);
                if (Status == FALSE)
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                
                break;

            case WSC_IE_AUTHENTICATOR:  /* 5 */
                PlatformMoveMemory(Hmac, pData, 8);
                break;
                
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM6 --> Unknown IE 0x%04x\n", WscType));
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Verify R-Hash2
    // Create input for R-Hash2
    PlatformMoveMemory(Tmp, pReg->Rs2, 16);
    PlatformMoveMemory(&Tmp[16], pReg->Psk2, 16);
    PlatformMoveMemory(&Tmp[32], pReg->Pke, 192);
    PlatformMoveMemory(&Tmp[224], pReg->Pkr, 192);
    
    // Generate R-Hash2
    hmac_sha(pReg->AuthKey, 32, Tmp, 416, RHash, 32);
    
    if (MtkCompareMemory(pReg->RHash2, RHash, 32) != 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM6 --> RHash2 not matched\n"));

        // Error Psk2 (device password related)
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_PASSWORD_MISMATCH;
        return FALSE;
    }
    
    // Combine last TX & RX message contents and validate the HMAC
    // We have to exclude last 12 bytes from last receive since it's authenticator value
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length - 12;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    pAuth += pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length - 12);

    // Validate HMAC, reuse Tmp buffer
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    if (PlatformEqualMemory(Hmac, Tmp, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM6 --> HMAC not match\n"));    
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Tmp[0])), cpu2be32(*((PULONG) &Tmp[4]))));   
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
    }
    

    // err status
    if (pPort->StaCfg.WscControl.WscStatus & 0x0F00)
        return FALSE;
    else
    {   
        // Update WSC status
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M6_RECEIVED;
        DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM6\n"));
        return TRUE;
    }

}

/*
    ========================================================================
    
    Routine Description:
        Receive M7 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build M8
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM7(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg)
{
    USHORT              Length;
    PUCHAR              pData, pAuth;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    INT                 EncrLen, idx;
    UCHAR               Hmac[8], Tmp[512], EHash[32];
    ULONG               HmacLen;
    USHORT              WscType, WscLen;
    INT                 Status = 0;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);
    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM7\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        if(pExtRegMsg)
        {
            pData = pExtRegMsg->MsgData;
            Length = pExtRegMsg->Length;
            if (Length >= WSC_MAX_RX_BUFFER)
            {
                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                DBGPRINT_ERR(("WscRecvMessageM7 --> M7 is too long (=%d)\n", Length));
                return FALSE;
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WSC External Registrar -- No ExtReg M7 in WscRecvMessageM1\n"));
            return FALSE;
        }
    }
    else
    {
        // Skip all the EAP headers since we already parse the correct M2 type earlier
        // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

        // Skip the EAP LLC header check since it's been checked many times.
        pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
        pData += sizeof(IEEE8021X_FRAME);

        // start of 802.1x frame header
        p802_1x = (PIEEE8021X_FRAME) pData;
        Length = be2cpu16(p802_1x->Length);
        
        // Length include EAP 5 bytes header
        Length -= sizeof(EAP_FRAME);

        // Skip WSC Frame fixed header
        Length -= sizeof(WSC_FRAME);
        pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

        // Copy the content to Regdata for lastRx information
        // Length must include authenticator IE size
        if (Length >= WSC_MAX_RX_BUFFER)
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
            DBGPRINT_ERR(("WscRecvMessageM7 --> M7 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
            return FALSE;
        }
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M7 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:        /* 1 */
                pReg->EnrolleeInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:       /* 2 */
                // Do nothing since we already parse it before 
                break;
    
            case WSC_IE_REG_NONCE:      /* 3 */
                // for verification with our registrar nonce
                if (MtkCompareMemory(&pReg->RegistrarNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M7 Compare registrar nonce mismatched \n"));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;
                    // return after password check
                }
                break;
                
            case WSC_IE_ENCR_SETTING:   /* 4 */
                // There shall have smoe kind of length check
                if ((WscLen <= 16) || (WscLen > 1024))
                {
                    // ApEncrSetting is not enough
                    DBGPRINT(RT_DEBUG_ERROR, ("EncrSetting length is out of range(16~1024), require %d\n", WscLen));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                    break;
                }
                
                WscDecryptData(pAd, (pData + 16), (WscLen - 16), pReg->KeyWrapKey, pData, pReg->ApEncrSettings, &EncrLen);
                DBGPRINT(RT_DEBUG_TRACE, ("ApEncrSettings len = %d, data --> ", EncrLen));
                
                for (idx = 0; idx < EncrLen; idx++)
                {
                    DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->ApEncrSettings[idx]));
                }
                DBGPRINT(RT_DEBUG_INFO, ("\n"));

                // Cleanup Old M7 Profile contents
                PlatformZeroMemory(&pPort->StaCfg.WscControl.WscM7Profile, sizeof(WSC_PROFILE));

                // Parse encryption settings
                Status = WscParseEncrSettings(pAd, pPort, pReg->ApEncrSettings, EncrLen);
                if (Status == FALSE)
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;

                break;
                
            case WSC_IE_AUTHENTICATOR:  /* 5 */
                PlatformMoveMemory(Hmac, pData, 8);
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM7 --> Unknown IE 0x%04x\n", WscType));
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Verify E-Hash2
    // Create input for E-Hash2
    PlatformMoveMemory(Tmp, pReg->Es2, 16);
    PlatformMoveMemory(&Tmp[16], pReg->Psk2, 16);
    PlatformMoveMemory(&Tmp[32], pReg->Pke, 192);
    PlatformMoveMemory(&Tmp[224], pReg->Pkr, 192);
    
    // Generate E-Hash2
    hmac_sha(pReg->AuthKey, 32, Tmp, 416, EHash, 32);
    
    if (MtkCompareMemory(pReg->EHash2, EHash, 32) != 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM7 --> EHash2 not matched\n"));

        // Error Psk2 (device password related)
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_PASSWORD_MISMATCH;
        return FALSE;
    }
    
    // Combine last TX & RX message contents and validate the HMAC
    // We have to exclude last 12 bytes from last receive since it's authenticator value
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length - 12;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    pAuth += pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length - 12);

    // Validate HMAC, reuse Tmp buffer
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    if (PlatformEqualMemory(Hmac, Tmp, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM7 --> HMAC not match\n"));    
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Tmp[0])), cpu2be32(*((PULONG) &Tmp[4]))));   
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
    }
    

    // err status
    if (pPort->StaCfg.WscControl.WscStatus & 0x0F00)
        return FALSE;
    else
    {   
        // Update WSC status
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M7_RECEIVED;
        DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM7\n"));
        return TRUE;
    }


}

/*
    ========================================================================
    
    Routine Description:
        Receive M8 message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        TRUE/FALSE

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Prepare to build EAP-Rsp(Done)
        
    ========================================================================
*/
BOOLEAN WscRecvMessageM8(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    USHORT              Length;
    PUCHAR              pData, pAuth;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    INT                 EncrLen, idx;
    UCHAR               Hmac[8], Tmp[512];
    ULONG               HmacLen;
    USHORT              WscType, WscLen;
    BOOLEAN             MatchMAC = FALSE;
    UCHAR               i;
    PSAVED_WPS_MESSAGE pSavedWPSMessage = (PSAVED_WPS_MESSAGE)(&pPort->StaCfg.WscControl.SavedWPSMessage);

    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscRecvMessageM8\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Skip all the EAP headers since we already parse the correct M2 type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);
    
    // Length include EAP 5 bytes header
    Length -= sizeof(EAP_FRAME);

    // Skip WSC Frame fixed header
    Length -= sizeof(WSC_FRAME);
    pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

    // Copy the content to Regdata for lastRx information
    // Length must include authenticator IE size
    if (Length >= WSC_MAX_RX_BUFFER)
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
        DBGPRINT_ERR(("WscRecvMessageM8 --> M8 is too long (=%d)\n", be2cpu16(p802_1x->Length)));
        return FALSE;
    }
    pSavedWPSMessage->LastRx.Length = Length;       
    PlatformMoveMemory(pSavedWPSMessage->LastRx.Data, pData, Length);

    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse M8 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_VERSION:    /* 1 */
                pReg->RegistrarInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:   /* 2 */
                // Do nothing since we already parse it before 
                break;

            case WSC_IE_EN_NONCE:   /* 3 */
                // for verification with our enrollee nonce
                if (MtkCompareMemory(&pReg->EnrolleeNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("Rx M8 Compare enrollee nonce mismatched \n"));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;  
                    return FALSE;
                }
                break;
                
            case WSC_IE_ENCR_SETTING:   /* 4 */
                // There shall have smoe kind of length check
                if ((WscLen <= 16) || (WscLen > 1024))
                {
                    // ApEncrSetting is not enough
                    DBGPRINT(RT_DEBUG_ERROR, ("EncrSetting length is out of range(16~1024), require %d\n", WscLen));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
                    return FALSE;
                }
                
                WscDecryptData(pAd, (pData + 16), (WscLen - 16), pReg->KeyWrapKey, pData, pReg->ApEncrSettings, &EncrLen);
                DBGPRINT(RT_DEBUG_TRACE, ("ApEncrSettings len = %d, data --> ", EncrLen));
                
                for (idx = 0; idx < EncrLen; idx++)
                {
                    DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x-", pReg->ApEncrSettings[idx]));
                }
                DBGPRINT(RT_DEBUG_INFO, ("\n"));        

                // Parse encryption settings
                WscProcessCredential(pAd, pPort, pReg->ApEncrSettings, EncrLen);
                break;
                
            case WSC_IE_AUTHENTICATOR:  /* 5 */
                PlatformMoveMemory(Hmac, pData, 8);
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM8 --> Unknown IE 0x%04x\n", WscType));
                break;
        }
        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    if((pAd->StaCfg.WSCVersion2 >= 0x20)&&
        (pPort->StaCfg.WscControl.WscProfile.Profile[0].EncrType == 0x0002)) //WEP profile

    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM8 WEP (0x%04x) profile, Reject the profile and send NACK!!!!\n",
            pPort->StaCfg.WscControl.WscProfile.Profile[0].EncrType));
        return FALSE;       
    }


    //In WSC 2.0, MAC of profile should matchs with our own MAC. 
    if(pPort->StaCfg.WscControl.WSCRegistrarVersion >= 0x20)
    {
        for(i = 0; i < 8; i++)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ProfileMAC:%x %x %x %x %x %x\n", pPort->StaCfg.WscControl.WscProfile.Profile[i].MacAddr[0], pPort->StaCfg.WscControl.WscProfile.Profile[i].MacAddr[1],
                pPort->StaCfg.WscControl.WscProfile.Profile[i].MacAddr[2], pPort->StaCfg.WscControl.WscProfile.Profile[i].MacAddr[3],
                pPort->StaCfg.WscControl.WscProfile.Profile[i].MacAddr[4], pPort->StaCfg.WscControl.WscProfile.Profile[i].MacAddr[5]));
            if(PlatformEqualMemory(pPort->CurrentAddress, pPort->StaCfg.WscControl.WscProfile.Profile[i].MacAddr, MAC_ADDR_LEN))
            {
                MatchMAC = TRUE;
                break;
            }
        }
        
        if(!MatchMAC)
        {
            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_M8_PROFILE_MAC_ERR;
            DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM8 --> MAC in Credential of M8 is mismatch!!!!\n"));
            return FALSE;
        }
    }

    // Combine last TX & RX message contents and validate the HMAC
    // We have to exclude last 12 bytes from last receive since it's authenticator value
    HmacLen = pSavedWPSMessage->LastTx.Length + pSavedWPSMessage->LastRx.Length - 12;
    pAuth = (PUCHAR) &HmacData[0];
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastTx.Data, pSavedWPSMessage->LastTx.Length);
    pAuth += pSavedWPSMessage->LastTx.Length;
    PlatformMoveMemory(pAuth, pSavedWPSMessage->LastRx.Data, pSavedWPSMessage->LastRx.Length - 12);

    // Validate HMAC, reuse Tmp buffer
    hmac_sha(pReg->AuthKey, 32, HmacData, HmacLen, Tmp, 32);
    
    if (PlatformEqualMemory(Hmac, Tmp, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscRecvMessageM8 --> HMAC not match\n"));
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Tmp[0])), cpu2be32(*((PULONG) &Tmp[4]))));
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));

        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_INVALID_DATA;
        return FALSE;
    }
    
    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_M8_RECEIVED;

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscRecvMessageM8\n"));

    return TRUE;
}

/*
    ========================================================================
    
    Routine Description:
        Receive Eap Fail frame

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        Actions after Rx EAP-Fail
        1. Drop the connection
        2. Save the config file.
        3. Set the correct parameters
        4. Use Auto reconnect to make connection.
        
    ========================================================================
*/
VOID    WscReceiveEapFail(
    IN  PMP_ADAPTER       pAd, 
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscReceiveEapFail\n"));
//    DBGPRINT(RT_DEBUG_TRACE,(" . 1 PIN = %x  %x. 2 PIN = %x  %x\n", *(PULONG)&pPort->P2PCfg.PinCode[0], *(PULONG)&pPort->P2PCfg.PinCode[4], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));

    // Skip all the EAP headers since we already parse the correct M2 type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

    // Update WSC status
    // pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_FAIL_RECEIVED;    

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscReceiveEapFail\n"));   
}

/*
    ========================================================================
    
    Routine Description:
        Receive Nack message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Error handling
        
    ========================================================================
*/
VOID WscReceiveEapNack(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    USHORT              Length;
    PUCHAR              pData;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    USHORT              WscType, WscLen;
    USHORT              ConfigError;
    PWSC_DEV_INFO       pDevInfo;
    UCHAR               EnrNonceTmp[16] = {0};
    UCHAR               RegNonceTmp[16] = {0};

    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscReceiveEapNack\n"));

    DBGPRINT(RT_DEBUG_TRACE,(" . 1 PIN = %x  %x. 2 PIN = %x  %x\n", *(PULONG)&pPort->P2PCfg.PinCode[0], *(PULONG)&pPort->P2PCfg.PinCode[4], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));
    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Role play, Enrollee or Registrar
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
        pDevInfo = (PWSC_DEV_INFO) &pReg->EnrolleeInfo;
    else
        pDevInfo = (PWSC_DEV_INFO) &pReg->RegistrarInfo;

    // Skip all the EAP headers since we already parse the correct M2 type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first

    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);
    
    // Length include EAP 5 bytes header
    Length -= sizeof(EAP_FRAME);

    // Skip WSC Frame fixed header
    Length -= sizeof(WSC_FRAME);
    pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));

    // Init ConfigError to zero
    ConfigError = cpu2be16(0x0000);
    
    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        switch (WscType)
        {
            case WSC_IE_VERSION:    /* 1 */
                pDevInfo->Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:   /* 2 */
                // Do nothing since we already parse it before 
                break;
                
            case WSC_IE_EN_NONCE:   /* 3 */
                if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
                {
                    // for verification with our enrollee nonce
                    if (MtkCompareMemory(&pReg->EnrolleeNonce, pData, 16) != 0)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Rx Nack Compare enrollee nonce mismatched \n")); 
                        return;
                    }
                }
                else
                {
                    //PlatformMoveMemory(pReg->EnrolleeNonce, pData, 16);
                    PlatformMoveMemory(EnrNonceTmp, pData, 16);
                }
                break;
                
            case WSC_IE_REG_NONCE:  /* 4 */
                if (pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_ENROLLEE)
                {
                    // for verification with our registrar nonce
                    if (MtkCompareMemory(&pReg->RegistrarNonce, pData, 16) != 0)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Rx Nack Compare registrar nonce mismatched \n"));    
                        return;
                    }
                }
                else
                {
                    //PlatformMoveMemory(pReg->RegistrarNonce, pData, 16);
                    PlatformMoveMemory(RegNonceTmp, pData, 16);
                }
                break;
    
            case WSC_IE_CONF_ERROR: /* 5 */
                pDevInfo->ConfigError = *((PUSHORT) pData);
                ConfigError = cpu2be16(*((PUSHORT) pData));
                DBGPRINT(RT_DEBUG_TRACE, ("WscReceiveEapNack --> ConfigError 0x%04x\n", ConfigError));
                break;
                
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscReceiveEapNack --> Unknown IE 0x%04x\n", WscType));      
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
    {
        if(MtkCompareMemory(pReg->EnrolleeNonce, EnrNonceTmp, 16) == 0)
        {
            PlatformMoveMemory(pReg->RegistrarNonce, RegNonceTmp, 16);
        }
    }
    else
    {
        if(MtkCompareMemory(pReg->RegistrarNonce, RegNonceTmp, 16) == 0)
        {
            PlatformMoveMemory(pReg->EnrolleeNonce, EnrNonceTmp, 16);
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("RegistrarNonce:0x%x 0x%x 0x%x ..\n", pReg->RegistrarNonce[0], pReg->RegistrarNonce[1], pReg->RegistrarNonce[2]));

    //  Error handling
    switch(ConfigError)
    {
        // connection timeout at the peer           
        case WSC_ERROR_REG_SESSION_TIMEOUT:
            if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
            {
                BOOLEAN     Cancelled;
                
                // cancel all timer
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscConnectTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscScanTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPINTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregScanTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregPBCTimer, &Cancelled);
                PlatformCancelTimer(&pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer, &Cancelled);

                DBGPRINT(RT_DEBUG_TRACE, ("!! 4 STATUS_WSC_EAP_FAILED\n"));     
                
                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_FAILED;  
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;
            }   
            break;
            
        case WSC_ERROR_DEV_PWD_AUTH_FAIL:
            // Change the state to wait EAP-FAIL
            if (pPort->StaCfg.WscControl.WscState != WSC_STATE_WAIT_EAPFAIL)
                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_EAPFAIL;
            break;
            
        default:
            break;
    }
    
    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_NACK_RECEIVED;

    P2pReceiveEapNack(pAd, pElem);
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscReceiveEapNack\n"));   
}

VOID    WpseSendEapFail(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    HEADER_802_11       Header_802_11;
    UCHAR               AckRate = RATE_2;
    USHORT              AckDuration = 0, Length;
    IEEE8021X_FRAME     Ieee_8021x;
    EXTENSIBLE_AUTH     ExtAuth;
    UCHAR               *OutBuffer = NULL;
    NDIS_STATUS         NStatus;
    ULONG               FrameLen = 0;
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
    UCHAR               Addr2[6];
    PHEADER_802_11      pHeader_802_11;
    
    // 1. Send EAP-Rsp Id
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WpseSendEapFail\n"));
    // =====================================
    // Use Priority Ring & NdisCommonMiniportMMRequest
    // =====================================
    pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    pHeader_802_11 = (PHEADER_802_11)&pElem->Msg[0];
    PlatformMoveMemory(Addr2, pHeader_802_11->Addr2, MAC_ADDR_LEN);
    WscMacHeaderInit(pAd, pPort,&Header_802_11, Addr2);

    // ACK size is 14 include CRC, and its rate is based on real time information
    AckRate = pPort->CommonCfg.ExpectedACKRate[pPort->CommonCfg.TxRate];
    AckDuration = XmitCalcDuration(pAd, AckRate, 14);
    Header_802_11.Duration = pPort->CommonCfg.Dsifs + AckDuration;

    // Length, -1 NULL pointer of string
    Length = sizeof(ExtAuth);
    
    // Zero 802.1x body
    PlatformZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
    Ieee_8021x.Version = EAPOL_VER;
    Ieee_8021x.Type    = EAPPacket;
    Ieee_8021x.Length  = cpu2be16(Length);

    // Zero EAP frame
    PlatformZeroMemory(&ExtAuth, sizeof(ExtAuth));
    ExtAuth.Code   = EAP_CODE_FAIL;
    ExtAuth.Id     = Id;
    ExtAuth.Length = cpu2be16(Length);

    
    // Out buffer for transmitting message      
    NStatus = MlmeAllocateMemory(pAd, (PVOID)&OutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;                 

    FrameLen = 0;
    
    // Make  Transmitting frame
    MakeOutgoingFrame(OutBuffer, &FrameLen, 
                        sizeof(HEADER_802_11), &Header_802_11,
                        sizeof(EAPHEAD), EAPHEAD, 
                        sizeof(IEEE8021X_FRAME), &Ieee_8021x,
                        sizeof(ExtAuth), &ExtAuth,
                        END_OF_ARGS);

    // Send using priority queue
    NdisCommonMiniportMMRequest(pAd, OutBuffer, FrameLen);

    // Update WSC status
//  pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_RSP_ID_SENT;             

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WpseSendEapFail\n")); 

}


/*
    ========================================================================
    
    Routine Description:
        Receive Done message

    Arguments:
        pAd    - NIC Adapter pointer
        Id          - ID between EAP-Req and EAP-Rsp pair
        Elem        - The EAP packet
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Verify the contents
        2. Error handling
        
    ========================================================================
*/
BOOLEAN WscReceiveEapDone(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg)
{
    USHORT              Length;
    PUCHAR              pData;
    PIEEE8021X_FRAME    p802_1x;
    PWSC_REG_DATA       pReg;
    USHORT              WscType, WscLen;
    USHORT              ConfigError;
    
    DBGPRINT(RT_DEBUG_TRACE, ("-----> WscReceiveEapDone\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Skip all the EAP headers since we already parse the correct WSC_DONE type earlier
    // Offset the pointer to start of WSC IEs, but we have to store the EAP length first
    if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
    {
        if(pExtRegMsg)
        {
            pData = pExtRegMsg->MsgData;
            Length = pExtRegMsg->Length;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WSC External Registrar -- No ExtReg M1 in WscRecvMessageM1\n"));
            return FALSE;
        }
    }
    else
    {
    // Skip the EAP LLC header check since it's been checked many times.
    pData = (PUCHAR) &pElem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
    pData += sizeof(IEEE8021X_FRAME);

    // start of 802.1x frame header
    p802_1x = (PIEEE8021X_FRAME) pData;
    Length = be2cpu16(p802_1x->Length);
    
    // Length include EAP 5 bytes header
    Length -= sizeof(EAP_FRAME);

    // Skip WSC Frame fixed header
    Length -= sizeof(WSC_FRAME);
    pData += (sizeof(EAP_FRAME) + sizeof(WSC_FRAME));
    }

    // Init ConfigError to zero
    ConfigError = cpu2be16(0x0000);
    
    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        switch (WscType)
        {
            case WSC_IE_VERSION:    /* 1 */
                pReg->RegistrarInfo.Version = *pData;
                break;
                
            case WSC_IE_MSG_TYPE:   /* 2 */
                // Do nothing since we already parse it before 
                break;
                
            case WSC_IE_EN_NONCE:   /* 3 */
                PlatformMoveMemory(pReg->EnrolleeNonce, pData, 16);
                break;
                
            case WSC_IE_REG_NONCE:  /* 4 */
                // for verification with our registrar nonce
                if (MtkCompareMemory(&pReg->RegistrarNonce, pData, 16) != 0)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Rx Done Compare registrar nonce mismatched \n"));
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_NONCE_MISMATCH;  
                    return FALSE;
                }   
                break;
                
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscReceiveEapDone --> Unknown IE 0x%04x\n", WscType));      
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Update WSC status
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_REQ_DONE_RECEIVED;   

    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscReceiveEapDone\n"));
    return TRUE;
}

/*
    ========================================================================
    
    Routine Description:
        Process elements within encryption settings

    Arguments:
        pAd    - NIC Adapter pointer
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        1. Rx M4 M5 M6 M7
        
    ========================================================================
*/
BOOLEAN WscParseEncrSettings(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  PUCHAR              pPlainData,
    IN  INT                 PlainLength)
{
    USHORT  WscType, WscLen, Length, HmacLen;
    PUCHAR  pData, pTmp;
    UCHAR   Hmac[8], Temp[32];
    PWSC_PROFILE    pProfile;
    USHORT  Idx;
    

    // Point to  M7 Profile
    pProfile = (PWSC_PROFILE) &pPort->StaCfg.WscControl.WscM7Profile;

    Length = (USHORT) PlainLength;
    HmacLen = Length - 12;
    pData  = pPlainData;
    
    // Start to process WSC IEs
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse Encrypted Settings
        switch (WscType)
        {
            case WSC_IE_E_SNONCE_1:     /* (1) */
                // for verification with our E-Hash1 calculation
                PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.Es1, pData, 16);
                break;

            case WSC_IE_E_SNONCE_2:     /* (1) */
                // for verification with our E-Hash2 calculation
                PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.Es2, pData, 16);
                break;

            case WSC_IE_R_SNONCE_1:     /* (1) */
                // for verification with our R-Hash1 calculation
                PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.Rs1, pData, 16);
                break;
                                
            case WSC_IE_R_SNONCE_2:     /* (1) */
                // for verification with our R-Hash2 calculation
                PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.Rs2, pData, 16);
                break;
                                
            case WSC_IE_KWRAP_AUTH:     /* (2) or (8) */
                PlatformMoveMemory(Hmac, pData, 8);
                break;

            //
            // Parse AP Settings in M7 if the peer is configured AP.
            //
            case WSC_IE_SSID:               /* (2) */
                if ((pPort->StaCfg.WscControl.bConfiguredAP == FALSE) ||
                    (pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_REGISTRAR))
                    break;  // ingore AP settings

                // Find the exact length of SSID without null terminator
                pTmp = pData;
                for (Idx = 0; Idx < WscLen; Idx++)
                {
                    if (*(pTmp++) == 0x0)
                        break;
                }
                pProfile->Profile[0].SSID.SsidLength = Idx;
                PlatformMoveMemory(pProfile->Profile[0].SSID.Ssid, pData, pProfile->Profile[0].SSID.SsidLength);
                // Svae the total number, always get the first profile
                pProfile->ProfileCnt = 1;
                break;

            case WSC_IE_MAC_ADDR:           /* (3) */
                if ((pPort->StaCfg.WscControl.bConfiguredAP == FALSE) ||
                    (pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_REGISTRAR))
                    break;  // ingore AP settings

                if (!MAC_ADDR_EQUAL(pData, pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr))
                    DBGPRINT(RT_DEBUG_TRACE, ("WscParseEncrSettings --> Enrollee macAddr not match\n"));
                PlatformMoveMemory(pProfile->Profile[0].MacAddr, pData, 6);             
                break;
                        
            case WSC_IE_AUTH_TYPE:          /* (4) */
                if ((pPort->StaCfg.WscControl.bConfiguredAP == FALSE) ||
                    (pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_REGISTRAR))
                    break;  // ingore AP settings

                pProfile->Profile[0].AuthType = cpu2be16(*((PUSHORT) pData));
                break;
                                
            case WSC_IE_ENCR_TYPE:          /* (5) */
                if ((pPort->StaCfg.WscControl.bConfiguredAP == FALSE) ||
                    (pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_REGISTRAR))
                    break;  // ingore AP settings

                pProfile->Profile[0].EncrType = cpu2be16(*((PUSHORT) pData));
                break;

            case WSC_IE_NETWORK_KEY_IDX:    /* (6) */
                if ((pPort->StaCfg.WscControl.bConfiguredAP == FALSE) ||
                    (pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_REGISTRAR))
                    break;  // ingore AP settings

                // Our key index start from 0 internally
                pProfile->Profile[0].KeyIndex = (*pData) - 1;
                break;
            
            case WSC_IE_NETWORK_KEY:        /* (7) */
                if ((pPort->StaCfg.WscControl.bConfiguredAP == FALSE) ||
                    (pPort->StaCfg.WscControl.WscConfMode != WSC_ConfMode_REGISTRAR))
                    break;  // ingore AP settings

                pProfile->Profile[0].KeyLength = WscLen;
                PlatformMoveMemory(pProfile->Profile[0].Key, pData, pProfile->Profile[0].KeyLength);
                break;
                
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscParseEncrSettings --> Unknown IE 0x%04x\n", WscType));       
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }
    // Validate HMAC, reuse KDK buffer
    hmac_sha(pPort->StaCfg.WscControl.RegData.AuthKey, 32, pPlainData, HmacLen, Temp, 32);
    
    if (PlatformEqualMemory(Hmac, Temp, 8) != 1)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WscParseEncrSettings --> HMAC not match\n"));    
        DBGPRINT(RT_DEBUG_TRACE, ("MD --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Temp[0])), cpu2be32(*((PULONG) &Temp[4])))); 
        DBGPRINT(RT_DEBUG_TRACE, ("calculated --> 0x%08x-%08x\n", cpu2be32(*((PULONG) &Hmac[0])), cpu2be32(*((PULONG) &Hmac[4]))));         

        return FALSE;   // invalid data
    }

    return TRUE;
}


/*
    ========================================================================
    
    Routine Description:
        Process credentials within AP encryption settings

    Arguments:
        pAd    - NIC Adapter pointer
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    WscProcessCredential(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  PUCHAR              pPlainData,
    IN  INT                 PlainLength)
{
    USHORT          WscType, WscLen, Length, Cnt, CurrentIdx = 0, Idx;
    PUCHAR          pData, pTmp;
    PWSC_PROFILE    pProfile;
    CHAR            NullTerminate = 0;
    UCHAR           idx;
    PULONG          pKey;
    UCHAR           i;

    Length = (USHORT) PlainLength;
    pData  = pPlainData;

    // Cleanup Old contents
    PlatformZeroMemory(&pPort->StaCfg.WscControl.WscProfile, sizeof(WSC_PROFILE));
    
    pProfile = (PWSC_PROFILE) &pPort->StaCfg.WscControl.WscProfile;
    
    // Init Profile number
    Cnt = 0;
    
    // Start to process WSC IEs within credential
    while (Length > 4)
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;
        Length -= 4;

        // Parse WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_NETWORK_IDX:
                // A new profile, add the cnt and save to database
                CurrentIdx = Cnt;   // since the index start from 0, we have to minus 1
                Cnt++;              
                break;

            case WSC_IE_SSID:
                // Find the exact length of SSID without null terminator
                pTmp = pData;
                for (Idx = 0; Idx < WscLen; Idx++)
                {
                    if (*(pTmp++) == 0x0)
                        break;
                }
                //The maximum ssid length is 32.
                if(Idx > 32)
                {
                    Idx = 32;
                }
                pProfile->Profile[CurrentIdx].SSID.SsidLength = Idx;
                PlatformMoveMemory(pProfile->Profile[CurrentIdx].SSID.Ssid, pData, pProfile->Profile[CurrentIdx].SSID.SsidLength);
                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_SSID -->pProfile->Profile[CurrentIdx].SSID.SsidLength 0x%x\n", pProfile->Profile[CurrentIdx].SSID.SsidLength));      
                for (idx = 0; idx < Idx; idx++)
                {
                    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-%02x-", pProfile->Profile[CurrentIdx].SSID.Ssid[idx]
                        , pProfile->Profile[CurrentIdx].SSID.Ssid[idx+1]));
                     idx++;
                }
                break;
                                
            case WSC_IE_AUTH_TYPE:
                pProfile->Profile[CurrentIdx].AuthType = cpu2be16(*((PUSHORT) pData));
                break;
                                
            case WSC_IE_ENCR_TYPE:
                pProfile->Profile[CurrentIdx].EncrType = cpu2be16(*((PUSHORT) pData));
                break;

            case WSC_IE_NETWORK_KEY_IDX:
                // Our key index start from 0 internally
                pProfile->Profile[CurrentIdx].KeyIndex = (*pData) - 1;
                break;
                
            case WSC_IE_NETWORK_KEY:
                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_NETWORK_KEY -->WscLen 0x%x\n", WscLen));     
                if(WscLen < 0)
                    break;
                if(WscLen > 0)
                {
                    while((WscLen -1 - NullTerminate >= 0)&&(*(pData + WscLen -1 - NullTerminate) == 0))
                    {
                        NullTerminate++;
                    }
                }
                pProfile->Profile[CurrentIdx].KeyLength = WscLen - NullTerminate;
                PlatformMoveMemory(pProfile->Profile[CurrentIdx].Key, pData, pProfile->Profile[CurrentIdx].KeyLength);
                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_NETWORK_KEY[%d] Len= %d, data --> ",CurrentIdx, pProfile->Profile[CurrentIdx].KeyLength));
                
                for (idx = 0; idx < pProfile->Profile[CurrentIdx].KeyLength; )
                {
                    pKey = (PULONG)&pProfile->Profile[CurrentIdx].Key[idx];
                    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%x  %x", *pKey, *(pKey+1)));
                    idx += 8;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("\n"));       
                break;
                
            case WSC_IE_MAC_ADDR:
                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_MAC_ADDR -->WscLen 0x%x\n", WscLen));    
                PlatformMoveMemory(pProfile->Profile[CurrentIdx].MacAddr, pData, 6);
                for (idx = 0; idx < MAC_ADDR_LEN; idx++)
                {
                    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-%02x-", pProfile->Profile[CurrentIdx].MacAddr[idx]
                        , pProfile->Profile[CurrentIdx].MacAddr[idx+1]));
                     idx++;
                }
                break;
                
            case WSC_IE_KWRAP_AUTH:
                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_KWRAP_AUTH -->WscLen 0x%x\n", WscLen));      
                // Not used here, since we already verify it at decryption
                break;

            case WSC_IE_CREDENTIAL:
                // Credential IE, The WscLen include all length within profile, we need to modify it
                // to be able to parse all profile fields
                DBGPRINT(RT_DEBUG_TRACE, ("WSC_IE_CREDENTIAL -->WscLen 0x%x\n", WscLen));      
                for (i = 0;i <WscLen;)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("0x%x 0x%x 0x%x 0x%x \n", *(pData+i), *(pData+i+1), *(pData+i+2), *(pData+i+3)));    
                    i+=4;
                }
                WscLen = 0;
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("WscProcessCredential --> Unknown IE 0x%04x\n", WscType));
                break;
        }
        // Offset to net WSC Ie
        pData  += WscLen;
        Length -= WscLen;
    }

    // Svae the total number
    pProfile->ProfileCnt = Cnt;
    DBGPRINT(RT_DEBUG_TRACE, ("WscProcessCredential --> %d profile retrieved from credential\n", Cnt));
}

/*
    ========================================================================
    
    Routine Description:
        Make WSC IE for the ProbeReq frame

    Arguments:
        pAd    - NIC Adapter pointer
        IsP2pScan   - scanning for P2P Spec requirement
        pOutBuf     - all of WSC IE field 
        pIeLen      - length
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        None
        
    ========================================================================
*/
VOID WscMakeProbeReqIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  BOOLEAN         IsP2pScan,
    OUT PUCHAR          pOutBuf,
    OUT PUSHORT         pIeLen)
{
    UCHAR           WscIEFixed[] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x04};    // length will modify later
    UCHAR           WscIEVersion[5] = {0x10, 0x4a, 0x00, 0x01, 0x10}; 
    USHORT          Len;
    PUCHAR          pData;
    PWSC_REG_DATA   pReg;
    PWSC_DEV_INFO   pDevInfo;
    WSC_DEV_INFO    LocalDevInfo;

    DBGPRINT(RT_DEBUG_INFO, ("-----> WscMakeProbeReqIE\n"));

    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    *pIeLen = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: WscState = %d, WscEnProbeReqIE= %d, WscCustomizedIESize= %d, WscCustomizedIEData= %x\n", __FUNCTION__, 
        pPort->StaCfg.WscControl.WscState, pPort->StaCfg.WscControl.WscEnProbeReqIE, pPort->StaCfg.WscControl.WscCustomizedIESize, pPort->StaCfg.WscControl.WscCustomizedIEData));
    if((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) && 
//      (pAd->StaCfg.WSCVersion2 < 0x20) &&
        (pPort->P2PCfg.WPSVersionsEnabled < 2) &&
        (IsP2pScan == FALSE))
    {
        DBGPRINT(RT_DEBUG_INFO, ("WscMakeProbeReqIE RETURN since WSC 1.0 and not doing WPS and not P2P scan\n"));
        return;
    }
    else    if((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) && 
            (pPort->StaCfg.WscControl.WscEnProbeReqIE == TRUE) &&
            (pPort->StaCfg.WscControl.WscCustomizedIESize != 0) && (pPort->StaCfg.WscControl.WscCustomizedIEData != NULL))
    {   
        *pIeLen = (USHORT)pPort->StaCfg.WscControl.WscCustomizedIESize;
        PlatformMoveMemory(pOutBuf, pPort->StaCfg.WscControl.WscCustomizedIEData, *pIeLen);
        DBGPRINT(RT_DEBUG_TRACE, ("%s: - Using costomized WSC IE, WscState = %d, WscEnProbeReqIE= %d, WscCustomizedIESize= %d, WscCustomizedIEData= %x\n", __FUNCTION__, 
            pPort->StaCfg.WscControl.WscState, pPort->StaCfg.WscControl.WscEnProbeReqIE, pPort->StaCfg.WscControl.WscCustomizedIESize, pPort->StaCfg.WscControl.WscCustomizedIEData));
        return;
    }
    
    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Role play, Enrollee or Registrar
    if(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
    {
        if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
            pDevInfo = (PWSC_DEV_INFO) &pReg->EnrolleeInfo;
        else if ((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR) || (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP))
            pDevInfo = (PWSC_DEV_INFO) &pReg->RegistrarInfo;
        else
        {
            PlatformZeroMemory(&LocalDevInfo, sizeof(WSC_DEV_INFO));
            WscSetDevInfo(pAd, pPort, &LocalDevInfo);
            pDevInfo = (PWSC_DEV_INFO) &LocalDevInfo;
        }
    }
    else
    {
        PlatformZeroMemory(&LocalDevInfo, sizeof(WSC_DEV_INFO));
        WscSetDevInfo(pAd, pPort, &LocalDevInfo);
        pDevInfo = (PWSC_DEV_INFO) &LocalDevInfo;
    }
    
    // 0. WSC fixed IE
    PlatformMoveMemory(pData, &WscIEFixed[0], 6);
    pData += 6;
    Len += 6;

    if((pAd->StaCfg.WSCVersion2 >= 0x20) && (IS_ENABLE_WSC20TB_2_WSCIEs_IN_ProbReq(pAd)))
    {
        OutMsgBuf[1] = 0x05;
        *pData = WscIEVersion[0];
        pData += 1;
        
        Len = 0;
        PlatformMoveMemory(pData, &WscIEFixed[0], 6);
        pData += 6;
        Len += 6;
        PlatformMoveMemory(pData, &WscIEVersion[1], 4);
        pData += 4;
        Len += 4;
    }
    else
    {
        // 1. Version
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        *(pData + 4) = pDevInfo->Version;
        pData += 5;
        Len   += 5;
    }
    
    // 2. Request Type
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REQ_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_DISABLE) || (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) || (IsP2pScan == TRUE))
    {
        *(pData + 4) = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;
    }
    else
    {
        *(pData + 4) = ((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE) ? WSC_MSGTYPE_ENROLLEE_OPEN_8021X : WSC_MSGTYPE_AP_WLAN_MGR);
    }
    pData += 5;
    Len   += 5;

    // 3. Config method
    if ((IsP2pScan == TRUE) || (IS_P2P_ENROLLEE(pPort)))
    {
        *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = cpu2be16(0x2288);    // Display, PBC, KEYPAD, Virtul PBC (WSC2.0), Virtual Display PIN (WSC2.0)
        pData += 6;
        Len   += 6;
    }
    else if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)                      
    {
        *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = pDevInfo->ConfigMethods; 
        pData += 6;
        Len   += 6;
    }
    else
    {
        *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = pPort->StaCfg.WscControl.ConfigMethods;
        pData += 6;
        Len   += 6;
    }

    // 4. UUID-(E or R)
    if(IsP2pScan == TRUE)
    {
        *((PUSHORT) pData) = cpu2be16(WSC_IE_UUID_E);
    }
    else
    {
        *((PUSHORT) pData) = (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)? cpu2be16(WSC_IE_UUID_E) : cpu2be16(WSC_IE_UUID_R);
    }
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pDevInfo->Uuid, 16);
    pData += 20;
    Len   += 20;

    // 5. Primary device type
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PRI_DEV_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);
    if (NDIS_WIN8_ABOVE(pAd) && (IsP2pScan == TRUE))
        PlatformMoveMemory((pData + 4), pPort->P2PCfg.PriDeviceType, 8);
    else
        PlatformMoveMemory((pData + 4), pDevInfo->PriDeviceType, 8);
    pData += 12;
    Len   += 12;

    // For WSC_ON or WSC2.0
    if (IsP2pScan == FALSE)
    {
        // 6. RF band, shall change based on current channel
        *((PUSHORT) pData) = cpu2be16(WSC_IE_RF_BAND);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        *(pData + 4) = pDevInfo->RfBand;
        pData += 5;
        Len   += 5;

        // 7. Associate state
        *((PUSHORT) pData) = cpu2be16(WSC_IE_ASSOC_STATE);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = pDevInfo->AssocState;    // Not associated
        pData += 6;
        Len   += 6;
                
        // 8. Config error
        *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_ERROR);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = pDevInfo->ConfigError;   // No error
        pData += 6;
        Len   += 6;

        // 9. Device password ID
        *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_PASS_ID);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        if(pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)
        {
            *((PUSHORT) (pData + 4)) = 0x0; //Default PIN
        }
        else
        {
            *((PUSHORT) (pData + 4)) = pDevInfo->DevPwdId;
        }
        pData += 6;
        Len   += 6;

        // 10. Select registrar set true if our role is registrar.
        if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)
        {
            *((PUSHORT) pData) = cpu2be16(WSC_IE_SEL_REG);
            *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
            *(pData + 4) = 1;
            pData += 5;
            Len   += 5;
        }
    }

    // 14. Device Name
    if ((IsP2pScan == TRUE) || (IS_P2P_ENROLLEE(pPort)))
    {
        // Use P2P Device Name
        // TODO: P2P device name might be confused wtih WSC2.0 ?
        *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
        *((PUSHORT) (pData + 2)) = cpu2be16(pPort->P2PCfg.DeviceNameLen);
        PlatformMoveMemory((pData + 4), pPort->P2PCfg.DeviceName, pPort->P2PCfg.DeviceNameLen);
        pData += (pPort->P2PCfg.DeviceNameLen + 4);
        Len   += (pPort->P2PCfg.DeviceNameLen + 4);
        DBGPRINT(RT_DEBUG_INFO, ("%s - Device Name Line[%d]\n", __FUNCTION__, __LINE__));
    }
    else if(pAd->StaCfg.WSCVersion2 >= 0x20)
    {
        if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)                       
        {
            PlatformZeroMemory(pData, sizeof(Wsc_Dev_Name_R) -1 + 4);
            *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
            *((PUSHORT) (pData + 2)) = cpu2be16(sizeof(Wsc_Dev_Name_R) - 1);//cpu2be16(0x0020); 
            //PlatformMoveMemory((pData + 4), pReg->EnrolleeInfo.DeviceName, pDevInfo->DeviceNameLen);
            PlatformMoveMemory((pData + 4), Wsc_Dev_Name_R, sizeof(Wsc_Dev_Name_R) - 1);
            pData += (sizeof(Wsc_Dev_Name_R) - 1 + 4);
            Len   += (sizeof(Wsc_Dev_Name_R) - 1 + 4);
            DBGPRINT(RT_DEBUG_INFO, ("%s - Device Name Line[%d]\n", __FUNCTION__, __LINE__));
        }
        else
        {
            PlatformZeroMemory(pData, pAd->WpsCfg.WPSDevNameLen+ 4);
            *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
            *((PUSHORT) (pData + 2)) = cpu2be16((pAd->WpsCfg.WPSDevNameLen));//cpu2be16(0x0020);   
            PlatformMoveMemory((pData + 4), pAd->WpsCfg.WPSDevName, pAd->WpsCfg.WPSDevNameLen);
            pData += (pAd->WpsCfg.WPSDevNameLen + 4);
            Len   += (pAd->WpsCfg.WPSDevNameLen + 4);          
            DBGPRINT(RT_DEBUG_INFO, ("%s - Device Name Line[%d]\n", __FUNCTION__, __LINE__));
        }
    }

    // For WSC2.0   
    if(pAd->StaCfg.WSCVersion2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;

        // 11. Manufacture
        PlatformZeroMemory(pData, sizeof(Wsc_Manufacture) -1 + 4);
        *((PUSHORT) pData) = cpu2be16(WSC_IE_MANUFACTURER);
        *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Manufacture)-1));//cpu2be16(0x0040);    
        PlatformMoveMemory((pData + 4), Wsc_Manufacture, sizeof(Wsc_Manufacture)-1);
        pData += sizeof(Wsc_Manufacture) -1 + 4;
        Len   += sizeof(Wsc_Manufacture) -1 + 4;
        
        // 12. Model Name
        PlatformZeroMemory(pData, sizeof(Wsc_Model_Name) -1 + 4);
        *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NAME);
        *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Name)-1));//cpu2be16(0x0020); 
        PlatformMoveMemory((pData + 4), Wsc_Model_Name, sizeof(Wsc_Model_Name)-1); 
        pData += sizeof(Wsc_Model_Name) -1 + 4;
        Len   += sizeof(Wsc_Model_Name) -1 + 4;
        
        // 13. Model Number
        PlatformZeroMemory(pData, sizeof(Wsc_Model_Number) -1 + 4);
        *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NO);
        *((PUSHORT) (pData + 2)) = cpu2be16((sizeof(Wsc_Model_Number)-1));//cpu2be16(0x0020);   
        PlatformMoveMemory((pData + 4), Wsc_Model_Number, sizeof(Wsc_Model_Number)-1);
        pData += sizeof(Wsc_Model_Number) -1 + 4;
        Len   += sizeof(Wsc_Model_Number) -1 + 4;
        
        // 15. Vendor Extension for additional attribute on 2.0 Spec.
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        // 15-1. Version
        *((PUCHAR) pData) = (WSC_IE_VERSION2);
        *((PUCHAR) (pData + 1)) = (0x01);
        *(pData + 2) = pAd->StaCfg.WSCVersion2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;
        
        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len   += 12;
        }
    }
        
    // update the total length in vendor specific IE
    //OutMsgBuf[1] = Len - 2;

    // fill in output buffer
    if((pAd->StaCfg.WSCVersion2 >= 0x20) && (IS_ENABLE_WSC20TB_2_WSCIEs_IN_ProbReq(pAd)))
    {
        // update the total length in vendor specific IE
        OutMsgBuf[8] = (UCHAR)Len - 2;  // 2 = 1(WPA IE DD)+ 1(WPA IE LEN). The second WSC IE length
        // fill in output buffer
        *pIeLen = Len + 7;  //7 is for the first WPS IE.
    }
    else
    {
        // update the total length in vendor specific IE
        OutMsgBuf[1] = (UCHAR)Len - 2; //Skip two bytes of DD and the length
        // fill in output buffer
        *pIeLen = Len;

    }
    PlatformMoveMemory(pOutBuf, &OutMsgBuf[0], *pIeLen);
    
    DBGPRINT(RT_DEBUG_TRACE, ("<----- WscMakeProbeReqIE %d\n", Len));
}

VOID WscMakeAssociateReqIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort,
    OUT PUCHAR          pOutBuf,
    OUT PUCHAR          pIeLen)
{
    UCHAR           WscIEFixed[] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x04};    // length will modify later
    UCHAR           Len;
    PUCHAR          pData;
    PWSC_REG_DATA   pReg;
    PWSC_DEV_INFO   pDevInfo;
    
    //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
    UCHAR               TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};

    DBGPRINT(RT_DEBUG_INFO, ("-----> WscMakeAssociateReqIE\n"));

    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;

    // Role play, Enrollee or Registrar
    if (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE)
        pDevInfo = (PWSC_DEV_INFO) &pReg->EnrolleeInfo;
    else
        pDevInfo = (PWSC_DEV_INFO) &pReg->RegistrarInfo;

    pData = (PUCHAR) &OutMsgBuf[0];
    Len = 0;
    *pIeLen = 0;
    
    // 0. WSC fixed IE
    PlatformMoveMemory(pData, &WscIEFixed[0], 6);
    pData += 6;
    Len += 6;
                
    // 1. Version
    *((PUSHORT) pData) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pDevInfo->Version;
    pData += 5;
    Len   += 5;

    // 2. Request Type
    *((PUSHORT) pData) = cpu2be16(WSC_IE_REQ_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = ((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_ENROLLEE) ? WSC_MSGTYPE_ENROLLEE_OPEN_8021X : WSC_MSGTYPE_AP_WLAN_MGR);
    pData += 5;
    Len   += 5;

    if(pDevInfo->Version2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;

        // 15. Vendor Extension for additional attribute on 2.0 Spec.
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData += 4;
        Len   += 4;
        
        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        // 3. Version
        *((PUCHAR) pData) = (WSC_IE_VERSION2);
        *((PUCHAR) (pData + 1)) = (0x01);
        *(pData + 2) = pDevInfo->Version2;
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        *pVendorExtLen = cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            PlatformMoveMemory(pData, TBV57, 12);
            pData += 12;
            Len += 12;
        }
    }
    
    
    // update the total length in vendor specific IE
    OutMsgBuf[1] = Len - 2;

    // fill in output buffer
    *pIeLen = Len;
    PlatformMoveMemory(pOutBuf, &OutMsgBuf[0], *pIeLen);
    
    DBGPRINT(RT_DEBUG_INFO, ("<----- WscMakeAssociateReqIE\n"));

}

VOID    
WscStop(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT    pPort
    )
{
    BOOLEAN     Cancelled = FALSE;
    UCHAR i =0;

    //
    // Stop all of WPS actions including timer, EAP exchange and scanning
    //
    
    // Clear all of timer
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscConnectTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPINTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscLEDTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer, &Cancelled);

    // Reset the WPS walk time.
    pPort->StaCfg.WscControl.bWPSWalkTimeExpiration = FALSE;
    pPort->StaCfg.WscControl.bErrDeteWPSTermination = FALSE;
    pPort->StaCfg.WscControl.bIndicateConnNotTurnOff = FALSE;


    // Reset state and status
    pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_NOTUSED;
    pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
    pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;
    
    // Reset EAP token
    pPort->StaCfg.WscControl.EapId = 1;

    // Reset AP's BSSID.
    //PlatformZeroMemory(pPort->StaCfg.WscControl.WscAPBssid, (sizeof(UCHAR) * MAC_ADDR_LEN));

    // Reset the AP's MAC address that is used for the UI.
    PlatformZeroMemory(pPort->StaCfg.WscControl.WscProfile.Profile, sizeof(WSC_PROFILE));

    //TODO: Make sure Aegis starting!

    // Reset the scanning counter.
    pPort->StaCfg.ScanCnt = 0;

    // Reset the desired BSSID list that set by the Ralink Vista UI with MTK_OID_N6_SET_DESIRED_BSSID_LIST.
    // In this way, other connection request is allowed again.
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][0] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][1] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][2] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][3] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][4] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][5] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDCount = 1;
    pAd->PortList[PORT_0]->PortCfg.AcceptAnyBSSID = TRUE;

    pPort->StaCfg.WscControl.WscPbcEnrCount = 0;
    PlatformZeroMemory(pPort->StaCfg.WscControl.WscPbcEnrTab, sizeof(WSCPBCENROLLEE));


    // Reset to FALSE.
    pPort->StaCfg.bSetDesiredBSSIDListFromRalinkUI = FALSE;


    if((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_6)
    {
        LedCtrlSetBlinkMode(pAd, LED_LINK_DOWN);
    }

    //pPort->StaCfg.WscControl.AsExtreg = FALSE;
    DBGPRINT(RT_DEBUG_TRACE, ("WscControl.AsExtreg is FALSE\n"));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: Clear the desired BSSID list and reset the bSetDesiredBSSIDListFromRalinkUI.\n", 
            __FUNCTION__));             

    DBGPRINT(RT_DEBUG_ERROR, ("!!!  WscStop !!! \n"));

}

VOID    
WscTimerStop(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort
    )
{
    //
    // Stop all of WPS actions including timer, EAP exchange and scanning
    //

    // Free all of timer

    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscConnectTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscScanTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscPBCTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscPINTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscEapRxTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscEapM2Timer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscLEDTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscExtregScanTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscExtregPBCTimer);
    PlatformFreeTimer(&pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer);

    DBGPRINT(RT_DEBUG_ERROR, ("!!!  %s !!! \n", __FUNCTION__));

}


VOID WscAutoGenProfile( 
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT      pPort)
{
    CHAR    pszSsid[13+1];  // add null terminal
    LPCSTR  pszFormat1 ="ExRegNW%02X%02X%02X";
    UCHAR   random[32];
    UCHAR   pszKey[64+1];   // add null terminal
    LPCSTR  pszFormat2="%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X";
    NTSTATUS Status;
    INT     idx;
    PWSC_CREDENTIAL pProfile = &pPort->StaCfg.WscControl.WscProfile.Profile[0];

    DBGPRINT(RT_DEBUG_TRACE, ("Start WscAutoGenProfile **************** \n"));
    P2PPrintState(pAd);
    PlatformZeroMemory(&pPort->StaCfg.WscControl.WscProfile, sizeof(WSC_PROFILE));

    // Set one profile.
    pPort->StaCfg.WscControl.WscProfile.ProfileCnt = 1;
    pPort->StaCfg.WscControl.WscProfile.Profile[0].NetIndex = 1;

    // SSID
    Status = RtlStringCbPrintfA(pszSsid, sizeof(pszSsid), pszFormat1, 
                                pAd->HwCfg.CurrentAddress[3], 
                                pAd->HwCfg.CurrentAddress[4], 
                                pAd->HwCfg.CurrentAddress[5]);

    PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.Ssid, pszSsid, sizeof(pszSsid) - 1);
    pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.SsidLength = sizeof(pszSsid) - 1;

    // WPA2PSK
    pPort->StaCfg.WscControl.WscProfile.Profile[0].AuthType = 0x20;
    // AES
    pPort->StaCfg.WscControl.WscProfile.Profile[0].EncrType = 0x08;
    if (IS_P2P_REGISTRA(pPort))
    {
        PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.Ssid, pPort->P2PCfg.SSID, 32);
        pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.SsidLength = pPort->P2PCfg.SSIDLen;
        // WPA2PSK
        pPort->StaCfg.WscControl.WscProfile.Profile[0].AuthType = 0x20;
        // AES
        pPort->StaCfg.WscControl.WscProfile.Profile[0].EncrType = 0x08;
        PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr, pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr, MAC_ADDR_LEN);
        DBGPRINT(RT_DEBUG_TRACE, ("WscAutoGenProfile: P2P SSID= %s  ", pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.Ssid));
        DBGPRINT(RT_DEBUG_TRACE, ("WscAutoGenProfile:MacAddr  ", pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.Ssid));
        for (idx = 0; idx < MAC_ADDR_LEN; idx++)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-%02x-", pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[idx]
                , pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[idx+1]));
             idx++;
        }
    }

    // Our key index start from 0 internally
    pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyIndex = 0;

    // If P2PGroupLimit is zero. It means in concurrent mode, P2P GO also want to 
    // acoomondate more than one client. Under this requirement, I need to use fixed PassPhrase Key
    // to send to WPS client (station). Because I don't want to let Win7 host to restart Virtual wifi AP
    // again for allow new association. REstarting VwifiAP might cause existing station to disassociate
    // Because the PMK changed.
#if 0
    if ((pAd->OpMode == OPMODE_STAP2P) && IS_P2P_REGISTRA(pPort)
        && pPort->CommonCfg.P2pControl.field.P2PGroupLimit == 0
        && (IS_P2P_SIGMA_OFF(pPort)))
    {
        PlatformZeroMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].Key, 64);
        pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyLength = pPort->P2PCfg.PhraseKeyLen;
        DBGPRINT(RT_DEBUG_TRACE, ("WscAutoGenProfile use PMK from P2P Profile,  "));
        DBGPRINT(RT_DEBUG_TRACE, ("WscAutoGenProfile Len= %d,  ", pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyLength));
        for (idx = 0; idx < (INT)pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyLength; idx++)        
        {
            pPort->StaCfg.WscControl.WscProfile.Profile[0].Key[idx] = pPort->P2PCfg.PhraseKey[idx];
        }
        for (idx = 0; idx < (INT)pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyLength + 2; idx++)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-%02x-", pPort->StaCfg.WscControl.WscProfile.Profile[0].Key[idx]
                , pPort->StaCfg.WscControl.WscProfile.Profile[0].Key[idx+1]));
             idx++;
        }
    }
    else
#endif
    {

        
        // WPAPSK Key, 64 in Hex
        for (idx = 0; idx < 32; idx++)
            random[idx]= RandomByte(pAd);

        Status = RtlStringCbPrintfA(pszKey, sizeof(pszKey), pszFormat2, 
                random[0], random[1], random[2], random[3], random[4], random[5],random[6],random[7],
                random[8], random[9], random[10], random[11], random[12], random[13],random[14],random[15],
                random[16], random[17], random[18], random[19], random[20], random[21],random[22],random[23],
                random[24], random[25], random[26], random[27], random[28], random[29],random[30],random[31]);

        PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].Key, pszKey, sizeof(pszKey)-1);
        pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyLength = sizeof(pszKey)-1;
        DBGPRINT(RT_DEBUG_TRACE, ("WscAutoGenProfile Random Key  KeyLength = %d.  ", sizeof(pszKey)-1));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("WscAutoGenProfile Len= %d, data --> ", pProfile->KeyLength));
    

}

//
// Support WPS LED mode (mode 7, mode 8 and mode 9).
// Ref: User Feedback (page 80, WPS specification 1.0)
//
BOOLEAN WscSupportWPSLEDMode(
    IN PMP_ADAPTER pAd)
{
    if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_7) || 
        ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_8)    || 
        ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) ||
        ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_12)||
        ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Support WPS LED mode (The WPS LED mode = %d).\n", 
            __FUNCTION__, (pAd->HwCfg.LedCntl.field.LedMode & LED_MODE)));
        return TRUE; // Support WPS LED mode.
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Not support WPS LED mode (The WPS LED mode = %d).\n", 
            __FUNCTION__, (pAd->HwCfg.LedCntl.field.LedMode & LED_MODE)));
        return FALSE; // Not support WPS LED mode.
    }
}

BOOLEAN WscSupportWPSLEDMode10(
    IN PMP_ADAPTER pAd)
{
    if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_10))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Support WPS LED mode (The WPS LED mode = %d).\n", 
            __FUNCTION__, (pAd->HwCfg.LedCntl.field.LedMode & LED_MODE)));
        return TRUE; // Support WPS LED mode.
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Not support WPS LED mode (The WPS LED mode = %d).\n", 
            __FUNCTION__, (pAd->HwCfg.LedCntl.field.LedMode & LED_MODE)));
        return FALSE; // Not support WPS LED mode.
    }
}

// WPS encryption types
#define WSC_ENCRYPTION_TYPE_OPEN_NONE   0x0001  // Open-None
#define WSC_ENCRYPTION_TYPE_WEP     0x0002  // WEP
#define WSC_ENCRYPTION_TYPE_TKIP    0x0004  // TKIP
#define WSC_ENCRYPTION_TYPE_AES     0x0008  // AES

//
// Whether the WPS AP has security setting or not.
// Note that this function is valid only after the WPS handshaking.
//
BOOLEAN WscAPHasSecuritySetting(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    BOOLEAN bAPHasSecuritySetting = FALSE;
    
    switch (pPort->StaCfg.WscControl.WscProfile.Profile[0].EncrType)
    {
        case WSC_ENCRYPTION_TYPE_OPEN_NONE:
        {
            bAPHasSecuritySetting = FALSE;
            break;
        }

        case WSC_ENCRYPTION_TYPE_WEP:
        case WSC_ENCRYPTION_TYPE_TKIP:
        case WSC_ENCRYPTION_TYPE_AES:
        {
            bAPHasSecuritySetting = TRUE;
            break;
        }

        default:
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Incorrect encryption types (%d)\n", 
                __FUNCTION__, pPort->StaCfg.WscControl.WscProfile.Profile[0].EncrType));
            ASSERT(FALSE);
            break;
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: WSC Entryption Type = %d\n", 
        __FUNCTION__, pPort->StaCfg.WscControl.WscProfile.Profile[0].EncrType));

    return bAPHasSecuritySetting;
}

VOID WscPbcEnrTabUpdate(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          Macaddr,
    IN  UCHAR           Version,
    IN  PUCHAR          UUID_E,
    IN  USHORT          DevicePassID,
    IN  ULONGLONG       ProbReqLastTime)
{
    UCHAR   i;
    UCHAR   idx = 0xff;
    ULONGLONG   Now64;

    if(DevicePassID != 0x04)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WscPbcEnrTabUpdate Return since DevicePassID is not PBC\n"));
        return;
    }
    
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

    //Check all entries in PBCEnrTab: clear the enrollss's record that the last time of ProbReq received is over 120 sec.
    for(i = 0 ; i < PBC_ENR_TAB_SIZE; i++)
    {
        if((pPort->StaCfg.WscControl.WscPbcEnrTab[i].Valid) && 
            (pPort->StaCfg.WscControl.WscPbcEnrTab[i].ProbReqLastTime+ (120 * ONE_SECOND_TIME) < Now64))
        {
            pPort->StaCfg.WscControl.WscPbcEnrTab[i].Valid = FALSE;
            PlatformZeroMemory(&pPort->StaCfg.WscControl.WscPbcEnrTab[i], sizeof(WSCPBCENROLLEE));
            pPort->StaCfg.WscControl.WscPbcEnrCount--;
            DBGPRINT(RT_DEBUG_TRACE, ("WscPbcEnrTabUpdate -- clear entry %d, Mac:..:%x:%x. PBCEnrTab Count is %d\n", 
                i, 
                pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr[4], pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr[5],
                pPort->StaCfg.WscControl.WscPbcEnrCount));
        }
    }

    for(i = 0 ; i < PBC_ENR_TAB_SIZE; i++)
    {
        if((pPort->StaCfg.WscControl.WscPbcEnrTab[i].Valid) && 
            (PlatformEqualMemory(&pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr, Macaddr, 6)))
        {
            idx = i;
            break;
        }
    }

    //This Mac is not in PBCEnrTab, add this to first empty entry.
    if(idx == 0xff)
    {
        for(i = 0 ; i < PBC_ENR_TAB_SIZE; i++)
        {
            if(!pPort->StaCfg.WscControl.WscPbcEnrTab[i].Valid)
            {
                idx = i;
                break;
            }
        }
        pPort->StaCfg.WscControl.WscPbcEnrCount++;
        DBGPRINT(RT_DEBUG_TRACE, ("WscPbcEnrTabUpdate -- Need Add Mac ..:%x:%x to a NEW entry %d, PbcEnrTab Count is :%d\n", 
            Macaddr[4], Macaddr[5],
            idx, pPort->StaCfg.WscControl.WscPbcEnrCount));
    }

    if(idx == 0xff)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("WscPbcEnrTabUpdate -- Can't find a appropriable entry!!!!\n"));
        return;
    }
    
    pPort->StaCfg.WscControl.WscPbcEnrTab[idx].Valid = TRUE;
    PlatformMoveMemory(pPort->StaCfg.WscControl.WscPbcEnrTab[idx].MacAddr, Macaddr, 6);
    pPort->StaCfg.WscControl.WscPbcEnrTab[idx].Version = Version; 
    PlatformMoveMemory(pPort->StaCfg.WscControl.WscPbcEnrTab[idx].UUID_E, UUID_E, 16);
    pPort->StaCfg.WscControl.WscPbcEnrTab[idx].DevicePassID = DevicePassID;
    pPort->StaCfg.WscControl.WscPbcEnrTab[idx].ProbReqLastTime = ProbReqLastTime;

    DBGPRINT(RT_DEBUG_TRACE, ("WscPbcEnrTabUpdate entry %d -- Mac:..:%x:%x, ver:%d, uuid-e:%x %x %x.., DP:%d, time:%lld\n",
        pPort->StaCfg.WscControl.WscPbcEnrTab[idx].MacAddr[4], pPort->StaCfg.WscControl.WscPbcEnrTab[idx].MacAddr[5],
        pPort->StaCfg.WscControl.WscPbcEnrTab[idx].Version,
        pPort->StaCfg.WscControl.WscPbcEnrTab[idx].UUID_E[0], pPort->StaCfg.WscControl.WscPbcEnrTab[idx].UUID_E[1], pPort->StaCfg.WscControl.WscPbcEnrTab[idx].UUID_E[2],
        pPort->StaCfg.WscControl.WscPbcEnrTab[idx].DevicePassID,
        pPort->StaCfg.WscControl.WscPbcEnrTab[idx].ProbReqLastTime));

    DBGPRINT(RT_DEBUG_TRACE, ("[#i] Valid Mac DP    (total:%d)\n", pPort->StaCfg.WscControl.WscPbcEnrCount));
    for(i = 0; i < 8 ; i++)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%d] %d\t..:%x:%x\t%d\n", 
            i, 
            pPort->StaCfg.WscControl.WscPbcEnrTab[i].Valid,
            pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr[4],
            pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr[5],
            pPort->StaCfg.WscControl.WscPbcEnrTab[i].DevicePassID));
    }
}

//return the number of devices that is detected under PBC within 2 mins.
UCHAR WscPbcEnrTabCheck(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  Mac)
{
    ULONGLONG   Now64;
    UCHAR       CntPBCEnr = 0;
    CHAR        i;
    BOOLEAN     AlreadyInPBCTab = FALSE;

    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

    WscPbcEnrTabUpdate(pAd, pPort, Mac,
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.Version2,
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.Uuid,
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DevPwdId,
        Now64);
    

    for(i = 0; i < PBC_ENR_TAB_SIZE; i++)
    {
        if(pPort->StaCfg.WscControl.WscPbcEnrTab[i].Valid)
        {
            if(pPort->StaCfg.WscControl.WscPbcEnrTab[i].ProbReqLastTime+ (120 * ONE_SECOND_TIME) >= Now64)
            {
                CntPBCEnr++;
                DBGPRINT(RT_DEBUG_TRACE, ("Entry %d (Mac:..:%x:%x) is PBC within 2 mins.\n", i, 
                    pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr[4], 
                    pPort->StaCfg.WscControl.WscPbcEnrTab[i].MacAddr[5]));
            }
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("CntPBCEnr:%d\n", CntPBCEnr));
    return CntPBCEnr;
}
