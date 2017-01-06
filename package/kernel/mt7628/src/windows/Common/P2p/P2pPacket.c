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
    p2p_packet.c

    Abstract:
    Peer to peer is also called Wifi Direct. P2P is a Task Group of WFA. this file contains all functions that handles p2p packets 

    Revision History:
    Who              When               What
    --------    ----------    ----------------------------------------------
    Jan Lee         2010-03-08    created for Peer-to-Peer(Wifi Direct)
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x01000000

extern UCHAR    OutMsgBuf[];        // buffer to create message contents
extern UCHAR    WIFIDIRECT_OUI[];


/*  
    ==========================================================================

    Description: 
        Receive Public Action frame that set to  verdor specific and with OUI type = WFA P2P
    ==========================================================================
 */
VOID P2PPublicAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Subtype = Elem->Msg[LENGTH_802_11+6];
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];

    DBGPRINT(RT_DEBUG_TRACE,("Public action frames with Vendor specific OUI = WFAP2P.Subtype = %d. Port= %d.  \n", Subtype, Elem->PortNum));

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (IS_P2P_MS_DEV(pAd, pAd->PortList[Elem->PortNum]))
    {
        // Indicate the previous packet sent before indicate packet received if  completedflag is not clear. 
        //
        do
        {
            PP2P_PUBLIC_FRAME pSendOutFrame = (PP2P_PUBLIC_FRAME) pPort->P2PCfg.pSendOutBufferedFrame;

            if ((pPort->P2PCfg.pSendOutBufferedFrame) && (pPort->P2PCfg.SendOutBufferedSize > FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID)))
            {
                if ((pSendOutFrame->Subtype == GO_NEGOCIATION_REQ) && (Subtype == GO_NEGOCIATION_RSP) &&
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ))
                {
                }
                else if ((pSendOutFrame->Subtype == GO_NEGOCIATION_RSP) && (Subtype == GO_NEGOCIATION_CONFIRM) &&
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_RSP))
                {
                }
                else if ((pSendOutFrame->Subtype == P2P_INVITE_REQ) && (Subtype == P2P_INVITE_RSP) &&
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ))
                {
                }
                else if ((pSendOutFrame->Subtype == P2P_PROVISION_REQ) && (Subtype == P2P_PROVISION_RSP) &&
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ))
                {
                }
                else // do nothing
                    break;

                // send below command to queue to avoid race condition with ACK handle mechanism
                if (pPort->P2PCfg.WaitForMSCompletedFlags != P2pMs_WAIT_COMPLETE_NOUSE)
                {                   
                    if (NDIS_STATUS_SUCCESS == MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2pMs_SEND_COMPLETE_IND, NULL, 0))
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s::Send MT_CMD_P2pMs_SEND_COMPLETE_IND(%d) to USB cmd queue\n", __FUNCTION__, pPort->P2PCfg.WaitForMSCompletedFlags));
                    }
                    else
                    {
                        // disable periodic ACK check
                        pPort->P2PCfg.bP2pAckReq = FALSE;
                        
                        DBGPRINT(RT_DEBUG_TRACE,("%s::Fail to send MT_CMD_P2pMs_SEND_COMPLETE_IND(%d) to USB cmd queue\n", __FUNCTION__, pPort->P2PCfg.WaitForMSCompletedFlags));
                    }
                }
            }   
        } while (FALSE);

        if (NDIS_STATUS_SUCCESS == MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_RCV, &Elem->Msg[0], Elem->MsgLen))
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s::Send MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_RCV(%d) to USB cmd queue\n", __FUNCTION__, Subtype));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s::Fail to send MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_RCV(%d) to USB cmd queue\n", __FUNCTION__, Subtype));
        }
    }
    else
#endif
    {
        switch(Subtype)
        {
            case GO_NEGOCIATION_REQ:
            P2pReceGoNegoReqAction(pAd, Elem);
            break;
            case GO_NEGOCIATION_RSP:
                P2pReceGoNegoRspAction(pAd, Elem);
                break;
            case GO_NEGOCIATION_CONFIRM:
                P2pReceGoNegoConfirmAction(pAd, Elem);
                break;
            case P2P_INVITE_REQ:
                P2pReceInviteReqAction(pAd, Elem);
                break;
            case P2P_INVITE_RSP:
                P2pReceInviteRspAction(pAd, Elem);
                break;
            case P2P_DEV_DIS_REQ:
                P2pReceDevDisReqAction(pAd, Elem);
                break;
            case P2P_DEV_DIS_RSP:
                P2pReceDevDisRspAction(pAd, Elem);
                break;
            case P2P_PROVISION_REQ:
                P2pReceProvisionReqAction(pAd, Elem);
                break;
            case P2P_PROVISION_RSP:
                P2pReceProvisionRspAction(pAd, Elem);
                break;
            default:
                DBGPRINT(RT_DEBUG_ERROR,("Unknown Public action frames with Vendor specific OUI = WFAP2P.Subtype = %d \n", Subtype));
                break;
        }
    }
}


/*  
    ==========================================================================
    Description: 
        Insert P2P subelement P2P Group Info format in Probe Response. it contains device information of 
        P2P Clients that are members of my P2P group.  
        
    Parameters: 
         pInBuffer : pointer to data that contains data to put in
         pOutBuffer : pointer to buffer that should put data to.
    Note:
         
    ==========================================================================
 */
ULONG InsertP2PGroupInfoTlv(
    IN PMP_ADAPTER pAd,
    IN PUCHAR       pOutBuffer)
{
    PUCHAR  pDest, pContent;
    UCHAR   Length;
    UCHAR   NoOfClient = 0;
    P2P_CLIENT_INFO_DESC        ClientInfo;
    UCHAR       ZeroType[P2P_DEVICE_TYPE_LEN];
    UCHAR       DevCapability;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;   
    PQUEUE_HEADER pHeader;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];

    pDest = pOutBuffer;
    PlatformZeroMemory(pDest, 255);
    PlatformZeroMemory(ZeroType, P2P_DEVICE_TYPE_LEN);
    *pDest = SUBID_P2P_GROUP_INFO;
    *(pDest + 2) = 0;       // Set length to 0 first.  Need to update to real length in the end of this function.
    pContent = pDest + 3;   // pContent points to payload.
    Length = 0;

    pHeader = &pPort->MacTab.MacTabList;
    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextMacEntry != NULL)
    {
        pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
        
        if(pMacEntry == NULL)
        {
            break;
        }
        
        if (pMacEntry->ValidAsCLI && pMacEntry->ValidAsP2P && (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_OPERATING))
        {
            NoOfClient++;
            PlatformZeroMemory(&ClientInfo, sizeof(ClientInfo));
            PlatformMoveMemory(ClientInfo.DevAddr, pMacEntry->P2pInfo.DevAddr, MAC_ADDR_LEN);
            DBGPRINT(RT_DEBUG_INFO, ("  -- InsertP2PGroupInfoTlv( ) (Mac  %x %x  %x.\n", pMacEntry->P2pInfo.DevAddr[3], pMacEntry->P2pInfo.DevAddr[4], pMacEntry->P2pInfo.DevAddr[5]));
            PlatformMoveMemory(ClientInfo.InterfaceAddr, pMacEntry->P2pInfo.InterfaceAddr, MAC_ADDR_LEN);
            DevCapability = pMacEntry->P2pInfo.DevCapability;
            if (MT_TEST_BIT(pMacEntry->P2pInfo.P2pFlag, P2PFLAG_DEVICE_DISCOVERABLE))
            {
                DBGPRINT(RT_DEBUG_INFO, ("  - (Mac  %x %x  %x Support Client Discovery.\n", pMacEntry->P2pInfo.DevAddr[3], pMacEntry->P2pInfo.DevAddr[4], pMacEntry->P2pInfo.DevAddr[5]));
                DevCapability |= DEVCAP_CLIENT_DISCOVER;
            }

            ClientInfo.Capability = DevCapability;
            PlatformMoveMemory(ClientInfo.ConfigMethod, &pMacEntry->P2pInfo.ConfigMethod, 2);
            PlatformMoveMemory(ClientInfo.PrimaryDevType, pMacEntry->P2pInfo.PrimaryDevType, P2P_DEVICE_TYPE_LEN);
            ClientInfo.NumSecondaryType = pMacEntry->P2pInfo.NumSecondaryType;
            // Set NumSecondaryType to zero first. will add soon.
            ClientInfo.NumSecondaryType = 0;
            PlatformMoveMemory(ClientInfo.PrimaryDevType, pMacEntry->P2pInfo.PrimaryDevType, P2P_DEVICE_TYPE_LEN);
            ClientInfo.Length = SIZE_OF_FIXED_CLIENT_INFO_DESC + pMacEntry->P2pInfo.DeviceNameLen + 4; // default no size of secondary types
            PlatformMoveMemory(pContent, &ClientInfo, SIZE_OF_FIXED_CLIENT_INFO_DESC);
            pContent += SIZE_OF_FIXED_CLIENT_INFO_DESC;
            // Length is accumulated length for this attirbute.
            Length += SIZE_OF_FIXED_CLIENT_INFO_DESC;
            
            // Insert Secondary device types
            if (pMacEntry->P2pInfo.NumSecondaryType > 0)
            {
                ClientInfo.NumSecondaryType = min(pMacEntry->P2pInfo.NumSecondaryType, MAX_P2P_SECONDARY_DEVTYPE_LIST);
                PlatformMoveMemory(pContent, pMacEntry->P2pInfo.SecondaryDevType, ClientInfo.NumSecondaryType * P2P_DEVICE_TYPE_LEN);
                // Length is accumulated length for this attirbute.
                Length += (ClientInfo.NumSecondaryType * P2P_DEVICE_TYPE_LEN);
                ClientInfo.Length += (ClientInfo.NumSecondaryType * P2P_DEVICE_TYPE_LEN);
                pContent += (ClientInfo.NumSecondaryType * P2P_DEVICE_TYPE_LEN);
            }

            // Insert WPS Device Name TLV
            *((PUSHORT) pContent) = cpu2be16(WSC_IE_DEV_NAME);
            *((PUSHORT) (pContent + 2)) = cpu2be16(pMacEntry->P2pInfo.DeviceNameLen);  
            PlatformMoveMemory(pContent + 4, pMacEntry->P2pInfo.DeviceName, pMacEntry->P2pInfo.DeviceNameLen);
            Length += (UCHAR)pMacEntry->P2pInfo.DeviceNameLen + 4;
            pContent += (pMacEntry->P2pInfo.DeviceNameLen + 4);
            DBGPRINT(RT_DEBUG_INFO, (" ----- InsertP2PGroupInfoTlv(%d) (Total Len now = %d) DevNameLen = %d.\n", pMacEntry->wcid, Length, pMacEntry->P2pInfo.DeviceNameLen));
        }
        pNextMacEntry = pNextMacEntry->Next;   
        pMacEntry = NULL;
    }


    // Because the length field doesn't count itself. So when update Attribute length, need to add number of client descriptor. 
    if (Length <= 0xff)
    {
        *(pDest + 1) = Length;      // Set  to real length
    }
    else
    {
        *(pDest + 1) = Length%255;      // Set  to real length
        *(pDest + 2) = Length/256;      // Set  to real length
    }

    return (Length + 3);

}
    
/*  
    ==========================================================================
    Description: 
        P2P Subelement was renamed to P2P attribute in final verison spec. 
        At first, it's called Subelement.
        So the function is named to InsertP2PSubelmtTlv. 
        Used to insert P2P attribute TLV format.
        
    Parameters: 
         pInBuffer : pointer to data that contains data to put in
         pOutBuffer : pointer to buffer that should put data to.
    Note:
         
    ==========================================================================
 */
ULONG InsertP2PSubelmtTlv(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,  
    IN UCHAR            SubId,
    IN PUCHAR       pInBuffer,
    IN PUCHAR       pOutBuffer)
{
    PUCHAR  pDest;
    ULONG   Length;

    pDest = pOutBuffer;
    PlatformZeroMemory(pDest, 255);
    *pDest = SubId;
    pDest += 1;
    Length = 0;
    switch(SubId)
    {
        case SUBID_P2P_INVITE_FLAG:
            *pDest = 1;
            *(pDest + 1) = 0;
            *(pDest + 2) = *pInBuffer;

            Length = 4;
            break;
        case SUBID_P2P_STATUS:  // 0
            *pDest = 1;
            *(pDest + 1) = 0;
            *(pDest + 2) = *pInBuffer;

            Length = 4;
            break;
        case SUBID_P2P_MINOR_REASON:    // 0
            break;
        case SUBID_P2P_CAP:     // 2
            *pDest = 2;
            *(pDest + 1) = 0;
            PlatformMoveMemory(pDest + 2, pInBuffer, 2);
            Length = 5;
            break;
        case SUBID_P2P_DEVICE_ID:   // 3 this is Device Address!
        case SUBID_P2P_GROUP_BSSID: // 6 group Bssid
        case SUBID_P2P_INTERFACE_ADDR:  //9
            *pDest = 6;
            *(pDest + 1) = 0;
            PlatformMoveMemory(pDest + 2, pInBuffer, MAC_ADDR_LEN);
            // DEvice Type
            //PlatformMoveMemory(pDest + 7, pAd->HwCfg.CurrentAddress, 8);

            Length = 9;
            break;
        case SUBID_P2P_OWNER_INTENT:    // 4
            *pDest = 1;
            *(pDest + 1) = 0;
            *(pDest + 2) = *pInBuffer;
            Length = 4;
            break;

        case SUBID_P2P_CONFIG_TIMEOUT:  // 5
            *pDest = 2;
            *(pDest + 1) = 0;
            PlatformMoveMemory(pDest + 2, pInBuffer, 2);
            Length = 5;
            break;
        case SUBID_P2P_CHANNEL_LIST:    // 11
            *(pDest + 1) = 0;
//country string . Two ASCII +  one more byte
            *(pDest + 2) = 0x47;
            *(pDest + 3) = 0x4c;
            *(pDest + 4) = 0x04;
            InsertP2pChannelList(pAd, pPort, pInBuffer, &Length, (pDest + 5));
            *pDest = Length + 3;
            Length += 6;
            break;
        case SUBID_P2P_OP_CHANNEL:
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if ((pPort->PortType == WFD_DEVICE_PORT) || (pPort->PortType == WFD_GO_PORT) ||  (pPort->PortType == WFD_CLIENT_PORT))
            {
                *pDest = 5;
                *(pDest + 1) = 0;
                PlatformMoveMemory(pDest + 2, pInBuffer, sizeof(DOT11_WFD_CHANNEL));
                Length = 8;
            }
            else
#endif
            {
                *pDest = 5;
                *(pDest + 1) = 0;
//Annex J. 802.11ERVmb_D3/0.pdf
                *(pDest + 2) = 0x47;
                *(pDest + 3) = 0x4c;
                *(pDest + 4) = 0x04;
                *(pDest + 5) = ChannelToClass(*pInBuffer, COUNTRY_GLOBAL);
                *(pDest + 6) = *pInBuffer;
                Length = 8;
            }
            break;
        case SUBID_P2P_LISTEN_CHANNEL:  // 6
            *pDest = 5;
            *(pDest + 1) = 0;
//Annex J. 802.11ERVmb_D3/0.pdf
            *(pDest + 2) = 0x47;
            *(pDest + 3) = 0x4c;
            *(pDest + 4) = 0x04;
            *(pDest + 5) = ChannelToClass(*pInBuffer, COUNTRY_GLOBAL);
            *(pDest + 6) = *pInBuffer;
            Length = 8;
            break;
        case SUBID_P2P_EXT_LISTEN_TIMING:   //8
            *pDest = 4;
            *(pDest + 1) = 0;
            *((PUSHORT) (pDest + 2)) = (P2P_EXT_LISTEN_PERIOD);
            *((PUSHORT) (pDest + 4)) = (P2P_EXT_LISTEN_INTERVAL);
            Length = 7;
            break;
        case SUBID_P2P_MANAGEABILITY:   //10
            break;
        case SUBID_P2P_NOA: //12
            break;
        case SUBID_P2P_DEVICE_INFO: // 13
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if ((pPort->PortType == WFD_DEVICE_PORT) || (pPort->PortType == WFD_GO_PORT) ||  (pPort->PortType == WFD_CLIENT_PORT))
            {
                // Length[0]: Low byte
                // Length[1]: High byte
                if ((pPort->PortCfg.WFDNumOfSecondaryDevice > 0) && (pPort->PortCfg.pWFDSecondaryDeviceTypeList != NULL))
                {
                    *pDest = 17 + pPort->P2PCfg.DeviceNameLen + 4 + pPort->PortCfg.WFDSecondaryDeviceTypeLen;
                    *(pDest + 1) = 0;
                }
                else
                {
                    *pDest = 17 + pPort->P2PCfg.DeviceNameLen + 4;                // Length[0]: Low byte
                    *(pDest + 1) = 0;                                           // Length[1]: High byte
                }
                // device address
                PlatformMoveMemory(pDest + 2, pInBuffer, MAC_ADDR_LEN);
                // config method                    
                *((PUSHORT) (pDest + 8)) = cpu2be16(pPort->P2PCfg.ConfigMethod);
                // Device Type  
                PlatformMoveMemory(pDest + 10, &pPort->P2PCfg.PriDeviceType[0], 8);
                
                // Insert MS P2P desired secondary device
                if ((pPort->PortCfg.WFDNumOfSecondaryDevice > 0) && (pPort->PortCfg.pWFDSecondaryDeviceTypeList != NULL))
                {
                    *(pDest + 18) = (UCHAR)(pPort->PortCfg.WFDNumOfSecondaryDevice & 0xff);
                    pDest += 19;
                    PlatformMoveMemory(pDest, pPort->PortCfg.pWFDSecondaryDeviceTypeList, pPort->PortCfg.WFDSecondaryDeviceTypeLen);
                    pDest += (pPort->PortCfg.WFDSecondaryDeviceTypeLen);    // 1 is for Number of secondary device types P.87
                }
                else
                {
                    *(pDest + 18) = 0;
                    pDest += 19;
                }           
                // Insert device name
                *((PUSHORT) pDest) = cpu2be16(WSC_IE_DEV_NAME);
                *((PUSHORT) (pDest + 2)) = cpu2be16(pPort->P2PCfg.DeviceNameLen);
                PlatformMoveMemory(pDest + 4, &pPort->P2PCfg.DeviceName[0], pPort->P2PCfg.DeviceNameLen);
                if ((pPort->PortCfg.WFDNumOfSecondaryDevice > 0) && (pPort->PortCfg.pWFDSecondaryDeviceTypeList != NULL))
                {
                    Length = 20 + pPort->P2PCfg.DeviceNameLen + 4 + pPort->PortCfg.WFDSecondaryDeviceTypeLen;
                }
                else
                {
                    Length = 20 + pPort->P2PCfg.DeviceNameLen + 4;
                }
                break;
            }
            else    
#endif
            {
                // Length[0]: Low byte
                // Length[1]: High byte
                *pDest = 17 + pPort->P2PCfg.DeviceNameLen + 4;
                *(pDest + 1) = 0;
                // device address
                PlatformMoveMemory(pDest + 2, pInBuffer, MAC_ADDR_LEN);
                // config method
                *((PUSHORT) (pDest + 8)) = cpu2be16(pPort->P2PCfg.ConfigMethod);
                // Device Type
                if(pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
                    PlatformMoveMemory(pDest + 10, &pPort->StaCfg.WscControl.RegData.RegistrarInfo.PriDeviceType[0], 8);
                else
                    PlatformMoveMemory(pDest + 10, &pPort->StaCfg.WscControl.RegData.EnrolleeInfo.PriDeviceType[0], 8);
                *(pDest + 18) = 0;
                *(pDest + 19) = 0;
                pDest += 19;
                // device name
                *((PUSHORT) pDest) = cpu2be16(WSC_IE_DEV_NAME);
                *((PUSHORT) (pDest + 2)) = cpu2be16(pPort->P2PCfg.DeviceNameLen);
                PlatformMoveMemory(pDest + 4, &pPort->P2PCfg.DeviceName[0], pPort->P2PCfg.DeviceNameLen);
                Length = 20 + pPort->P2PCfg.DeviceNameLen + 4;
                break;
            }
            
        case SUBID_P2P_GROUP_INFO:  //14
            break;
        case SUBID_P2P_GROUP_ID:    //15
            *pDest = 6 + pPort->P2PCfg.SSIDLen;
            *(pDest + 1) = 0;
            PlatformMoveMemory(pDest + 2, pInBuffer, MAC_ADDR_LEN);
            PlatformMoveMemory(pDest + 8, &pPort->P2PCfg.SSID[0], pPort->P2PCfg.SSIDLen);
            Length = 9 + pPort->P2PCfg.SSIDLen;
            DBGPRINT(RT_DEBUG_TRACE, (" -----Insert SUBID_P2P_GROUP_ID (Len = %d) \n", Length));
            break;
        case SUBID_P2P_INTERFACE:   //16
            *pDest = 6 + 1 + 6;
            *(pDest + 1) = 0;
            // We use the same device addr and interface addr. So All use pInBuffer to copy to thie interface addr list.
            PlatformMoveMemory(pDest + 2, pInBuffer, MAC_ADDR_LEN);
            *(pDest + 8) = 1;
            PlatformMoveMemory(pDest + 9, pInBuffer, MAC_ADDR_LEN);
            Length = 16;
            break;
        default:
            *pDest = 0;
            Length = 0;
            break;
    }

    return Length;
}


/*  
    ==========================================================================
    Description: 
        Used to insert P2P Channel list attribute.
        
    Parameters: 
        ChannelListLen : output for total length.
         pDest : pointer to buffer that should put data to.
    Note:
         
    ==========================================================================
 */
VOID InsertP2pChannelList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,  
    IN PUCHAR    ChannelList,
    OUT ULONG    *ChannelListLen,
    OUT PUCHAR  pDest)
{
    UCHAR       i, pos;
    UCHAR       LastRegClass = 0xff, CurRegClass;
    PUCHAR      pLastLenPos;
    UCHAR       LastNum = 0;
    PP2P_CHANNEL_ENTRY_LIST pCommcomSetChannelList = (PP2P_CHANNEL_ENTRY_LIST)ChannelList;
    UCHAR       ChannelNr;

    pLastLenPos = pDest + 1;
    pos = 2;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (pPort->PortType == WFD_DEVICE_PORT)
    {
        ChannelNr = pCommcomSetChannelList->ChannelNr;
    }
    else
#endif      
    {
        ChannelNr = pAd->HwCfg.ChannelListNum;
    }
    
    for (i = 0; i < ChannelNr; i++)
    {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if (pPort->PortType == WFD_DEVICE_PORT)
            CurRegClass = ChannelToClass(pCommcomSetChannelList->Channel[i], COUNTRY_GLOBAL);
        else
#endif      
            CurRegClass = ChannelToClass(pAd->HwCfg.ChannelList[i].Channel, COUNTRY_USA);
        
        // 0. Decide current regulatory class. 11y
        // Insert RegClass if necessary
        if (LastRegClass == 0xff)
        {
            // case 0 : initilize
            LastRegClass = CurRegClass;
            *(pDest) = CurRegClass;
        }
        else if ((CurRegClass != LastRegClass) && (LastRegClass != 0xff))
        {
            // case 1 : change regulatory class
            *(pDest + pos) = CurRegClass;
            *pLastLenPos = LastNum;

            LastNum = 0;
            LastRegClass = CurRegClass;
            pLastLenPos = pDest + pos + 1;
            pos = pos + 2;
        }

        // Insert
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if (pPort->PortType == WFD_DEVICE_PORT)
            *(pDest + pos) = pCommcomSetChannelList->Channel[i];
        else
#endif
            *(pDest + pos) = pAd->HwCfg.ChannelList[i].Channel;

        LastNum++;
        pos++;

        // Last item. Update Num.
        if (i == (ChannelNr -1))
        {
            *pLastLenPos = LastNum;
        }
    }

    *ChannelListLen = pos;

    DBGPRINT(RT_DEBUG_TRACE,("Channel List ==> output length is %d \n", *ChannelListLen));
    for (i = 0; i <*ChannelListLen; i = i++ )
    {
        DBGPRINT(RT_DEBUG_TRACE,(": %d \n", *(pDest+i)));
        
    }
}


/*  
    ==========================================================================
    Description: 
        The routine make  Device Discovery Request Action Frame Packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PSendDevDisReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN PUCHAR       Addr1,
    IN PUCHAR       Bssid,
    IN PUCHAR       ClientAddr1,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    //PMP_PORT  pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    ULONG   P2pIeLen = 0;
    PUCHAR          pDest;
    ULONG   Length;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    UCHAR       i;

    *pTotalFrameLen = 0;
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    pDest = pOutBuffer;
    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, Bssid);
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevDisReq  TO %x %x %x %x %x %x. \n", Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevDisReq  Bssid %x %x %x %x %x %x. \n", Bssid[0], Bssid[1], Bssid[2],Bssid[3],Bssid[4],Bssid[5]));
    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_DEV_DIS_REQ;

    pPort->P2PCfg.Token++;
    pFrame->Token = pPort->P2PCfg.Token;
    
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = 38;
    
    pDest = &pFrame->Octet[0];
    // attach subelementID = 3.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_ID, ClientAddr1, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    pFrame->Length = (UCHAR)P2pIeLen;
    pDest = (PUCHAR)pOutBuffer;
    for (i = 0; i <*pTotalFrameLen; )
    {
        DBGPRINT(RT_DEBUG_TRACE,(": %x %x %x %x %x %x %x %x %x \n", *(pOutBuffer+i), *(pOutBuffer+i+1), *(pOutBuffer+i+2), 
        *(pOutBuffer+i+3), *(pOutBuffer+i+4), *(pOutBuffer+i+5), *(pOutBuffer+i+6), *(pOutBuffer+i+7), *(pOutBuffer+i+8)));
        i = i + 9;
    }
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, *pTotalFrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);

    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevice Discovery Req  . TO %x %x %x %x %x %x.  *pTotalFrameLen = %d. \n", Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5], *pTotalFrameLen));
}


/*  
    ==========================================================================
    Description: 
        The routine make  Device Discovery Response Action Frame Packet .
        Send by GO. So Bssid is my GO addr
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PSendDevDisRsp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR        RspStatus,
    IN UCHAR            Token,
    IN PUCHAR       Addr1,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    //PMP_PORT  pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    ULONG   P2pIeLen = 0;
    PUCHAR          pDest;
    ULONG   Length;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    UCHAR       i;

    *pTotalFrameLen = 0;
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    pDest = pOutBuffer;
    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, pPort->CurrentAddress);
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevDisRsp[ Status : %d] TO %x %x %x %x %x %x. \n", RspStatus, Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5]));
    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_DEV_DIS_RSP;

    pFrame->Token = Token;
    
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = 38;
    
    pDest = &pFrame->Octet[0];
    // attach subelementID = 3.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, &RspStatus, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    pFrame->Length = (UCHAR)P2pIeLen;
    for (i = 0; i <*pTotalFrameLen; )
    {
        DBGPRINT(RT_DEBUG_TRACE,(": %x %x %x %x %x %x %x %x %x \n", *(pOutBuffer+i), *(pOutBuffer+i+1), *(pOutBuffer+i+2), 
        *(pOutBuffer+i+3), *(pOutBuffer+i+4), *(pOutBuffer+i+5), *(pOutBuffer+i+6), *(pOutBuffer+i+7), *(pOutBuffer+i+8)));
        i = i + 9;
    }
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, *pTotalFrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);

    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevice Discovery Response. TO %x %x %x %x %x %x.   \n", Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5]));
}


/*  
    ==========================================================================
    Description: 
        The routine send additional Probe Request when being in P2P Search State..
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pSendProbeReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort,
    IN MP_PORT_SUBTYPE      PortSubtype) 
{
    PUCHAR      pOutBuffer;
    ULONG       FrameLen;
    MLME_QUEUE_ELEM Elem;
    NDIS_STATUS   NStatus;

    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    // Always use device port to send out a probe request
    P2PMakeProbe(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], &Elem, PortSubtype, pPort->ScaningChannel, SUBTYPE_PROBE_REQ, pOutBuffer, &FrameLen);
    if (FrameLen > 0)
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    else if (pOutBuffer != NULL)
        MlmeFreeMemory(pAd, pOutBuffer);

}


/*  
    ==========================================================================
    Description: 
        The routine make Invitiation Response Action Frame Packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PSendProvisionReq(
    IN PMP_ADAPTER pAd,
    IN USHORT       ConfigMethod,
    IN UCHAR            Token,
    IN PUCHAR       Addr1,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    PMP_PORT  pPort = pAd->PortList[FXXK_PORT_0];
    ULONG   WpsIeLen = 0;
    PUCHAR          pDest;
    PUCHAR          pP2PIeLenDest;
    ULONG   Length;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    UCHAR           p2pindex = P2P_NOT_FOUND;

    *pTotalFrameLen = 0;
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendProvisionReq  . TO %x %x %x %x %x %x.  ConfigMethod = %s \n", Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5], decodeConfigMethod(ConfigMethod)));
    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    // Bssid = DA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, Addr1);
    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_PROVISION_REQ;
    pFrame->Token = Token;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), WPS_OUI, 4);
    WpsIeLen = 4;
    *pTotalFrameLen = 38;
    pDest = pFrame->Octet;
    
    // attach wsc version
    *((PUSHORT) pDest) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pDest + 2)) = cpu2be16(0x0001);
    *(pDest + 4) = WSC_VERSION;
    pDest += 5;
    WpsIeLen   += 5;
    *pTotalFrameLen += 5;
    
    // attach config method .
    *((PUSHORT) pDest) = cpu2be16(WSC_IE_CONF_MTHD);
    *((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pDest + 4)) = cpu2be16(ConfigMethod);      // Label, Display, PBC
    pDest += 6;
    WpsIeLen   += 6;
    *pTotalFrameLen += 6;

    // attach wsc version 2.0 in the end of wsc ie
    // announce we have WPS2.0 before message
    if(pAd->StaCfg.WSCVersion2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pDest) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pDest + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pDest + 2));

        pDest += 4;
        WpsIeLen   += 4;
        *pTotalFrameLen += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pDest, Wsc_SMI_Code, 3);
        pDest += 3;
        WpsIeLen   += 3;
        VendorExtLen += 3;
        *pTotalFrameLen += 3;

        // Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pDest) = (WSC_IE_VERSION2);
        *((PUSHORT) (pDest + 1)) = (0x01);
        *(pDest + 2) = pAd->StaCfg.WSCVersion2;
        pDest += 3;
        WpsIeLen   += 3;
        VendorExtLen += 3;
        *pTotalFrameLen += 3;

        *pVendorExtLen += cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pDest, TBV57, 12);
            pDest += 12;
            WpsIeLen   += 12;
            *pTotalFrameLen += 12;
        }
    }
    pFrame->Length = (UCHAR)WpsIeLen;

    // Insert P2P IE===>
    *pDest = IE_VENDOR_SPECIFIC;
    pP2PIeLenDest = (pDest+1);
    PlatformMoveMemory(pDest+2, P2POUIBYTE, 4);
    WpsIeLen = 4;
    pDest += 6;
    *pTotalFrameLen += 6;

    // attach capability
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP,  pPort->P2PCfg.P2pCapability, pDest);
    pDest += Length;
    WpsIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 13.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO,  pPort->CurrentAddress, pDest);
    pDest += Length;
    WpsIeLen += Length;
    *pP2PIeLenDest = (UCHAR)WpsIeLen;
    *pTotalFrameLen += Length;

    // attach Group ID
    p2pindex = P2pGroupTabSearch(pAd, Addr1);
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        if (pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule == P2P_IS_GO)
        {
            // Update peer's ssid (GO) because UI might get wrong ssid at the beginning
            if ((pPort->P2PCfg.SSIDLen != pAd->pP2pCtrll->P2PTable.Client[p2pindex].SsidLen) || 
                (PlatformEqualMemory(pPort->P2PCfg.SSID, pAd->pP2pCtrll->P2PTable.Client[p2pindex].Ssid, pAd->pP2pCtrll->P2PTable.Client[p2pindex].SsidLen)))
            {
                pPort->P2PCfg.SSIDLen = pAd->pP2pCtrll->P2PTable.Client[p2pindex].SsidLen;
                PlatformMoveMemory(pPort->P2PCfg.SSID, pAd->pP2pCtrll->P2PTable.Client[p2pindex].Ssid, pAd->pP2pCtrll->P2PTable.Client[p2pindex].SsidLen);
            }

            Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr, pDest);
            pDest += Length;
            WpsIeLen += Length;
            *pP2PIeLenDest = (UCHAR)WpsIeLen;
            *pTotalFrameLen += Length;

            DBGPRINT(RT_DEBUG_TRACE, ("%s - %s, Flag 0x%x, Group ID %x %x %x %x %x %x,  SsidLen %d %d\n", 
                    __FUNCTION__, 
                    decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState), pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pFlag, 
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr[0], pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr[1], 
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr[2], pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr[3],
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr[4],pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr[5],
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].SsidLen, pPort->P2PCfg.SSIDLen));

        }
    }
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, *pTotalFrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
}

/*  
    ==========================================================================
    Description: 
        The routine make Invitiation Response Action Frame Packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PSendProvisionRsp(
    IN PMP_ADAPTER pAd,
    IN USHORT       ConfigMethod,
    IN UCHAR        Token,
    IN PUCHAR       Addr1,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    PMP_PORT  pPort = pAd->PortList[FXXK_PORT_0];
    UCHAR   WpsIeLen = 0;
    PUCHAR          pDest;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS   NStatus;

    *pTotalFrameLen = 0;
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendProvisionRsp  . TO %x %x %x %x %x %x.  ConfigMethod = %x \n", Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5], ConfigMethod));
    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    // Bssid = SA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, pPort->CurrentAddress);
    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_PROVISION_RSP;
    pFrame->Token = Token;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), WPS_OUI, 4);
    WpsIeLen = 4;
    *pTotalFrameLen = 38;
    pDest = pFrame->Octet;
    if (ConfigMethod > 0)
    {
        // attach wsc version
        *((PUSHORT) pDest) = cpu2be16(WSC_IE_VERSION);
        *((PUSHORT) (pDest + 2)) = cpu2be16(0x0001);
        *(pDest + 4) = WSC_VERSION;
        pDest += 5;
        WpsIeLen   += 5;
        *pTotalFrameLen += 5;
        
        // attach config method .
        *((PUSHORT) pDest) = cpu2be16(WSC_IE_CONF_MTHD);
        *((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pDest + 4)) = cpu2be16(ConfigMethod);      // Label, Display, PBC
        pDest += 6;
        WpsIeLen   += 6;
        *pTotalFrameLen += 6;

        // attach wsc version 2.0 in the end of wsc ie
        // announce we have WPS2.0 before message
        if(pAd->StaCfg.WSCVersion2 >= 0x20)
        {
            PUSHORT     pVendorExtLen;
            USHORT      VendorExtLen = 0;
            
            *((PUSHORT) pDest) = cpu2be16(WSC_IE_VENDOR_EXT);
            *((PUSHORT) (pDest + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
            pVendorExtLen = ((PUSHORT) (pDest + 2));

            pDest += 4;
            WpsIeLen   += 4;
            *pTotalFrameLen += 4;

            //WSC SMI code as Vendor ID
            PlatformMoveMemory(pDest, Wsc_SMI_Code, 3);
            pDest += 3;
            WpsIeLen   += 3;
            VendorExtLen += 3;
            *pTotalFrameLen += 3;

            // Version2 if WSC version is highter than 2.0.
            *((PUSHORT) pDest) = (WSC_IE_VERSION2);
            *((PUSHORT) (pDest + 1)) = (0x01);
            *(pDest + 2) = pAd->StaCfg.WSCVersion2;
            pDest += 3;
            WpsIeLen   += 3;
            VendorExtLen += 3;
            *pTotalFrameLen += 3;

            *pVendorExtLen += cpu2be16(VendorExtLen);

            if(IS_ENABLE_WSC20TB_Version57(pAd))
            {
                //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
                UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
                DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

                PlatformMoveMemory(pDest, TBV57, 12);
                pDest += 12;
                WpsIeLen   += 12;
                *pTotalFrameLen += 12;
            }
        }           
    }
    pFrame->Length = WpsIeLen;
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, *pTotalFrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
}


/*  
    ==========================================================================
    Description: 
        The routine send Go Negociation Confirm packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PSendGoNegoConfirm(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR            Token,
    IN UCHAR            idx,
    IN PUCHAR       Addr1,
    IN UCHAR        StatusCode)
{
    ULONG       FrameLen;
    ULONG       TempLen;
    PRT_P2P_CLIENT_ENTRY pP2pEntry;
    NDIS_STATUS   NStatus;
    PUCHAR          pOutBuffer = NULL, pDest;
    UCHAR       Channel;
    PP2P_PUBLIC_FRAME   pFrame;
    ULONG       P2pLen = 0;
    UCHAR       P2pCapability[2];
    
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[idx];
    pDest = pOutBuffer;
    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    P2PMakeGoNegoConfirm(pAd, pPort, Addr1, Token, pDest, &TempLen);
    FrameLen = TempLen;
    pDest += TempLen;
    P2pLen = 4;
    // status
    TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, &StatusCode, pDest);
    pDest += TempLen;
    FrameLen += TempLen;
    P2pLen +=TempLen;
    // p2p cap
    P2pCapability[0] = pPort->P2PCfg.P2pCapability[0];
    P2pCapability[1] = pPort->P2PCfg.P2pCapability[1];
    if ((StatusCode == 0) && (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO))
    {
        P2pCapability[1] |= GRPCAP_OWNER;
        P2pCapability[1] |= GRPCAP_GROUP_FORMING;
        pPort->P2PCfg.P2pCapability[1] |= (GRPCAP_GROUP_FORMING);
    }
    else
    {
        P2pCapability[1] = 0;
        pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_GROUP_FORMING));
    }   
    TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, P2pCapability/*pPort->P2PCfg.P2pCapability*/, pDest);
    FrameLen += TempLen;
    pDest += TempLen;
    P2pLen +=TempLen;
    // If I am Go, Use my channel to set in the Go Nego Rsp.
    if (pP2pEntry->Rule == P2P_IS_CLIENT)
    {
        Channel = pPort->P2PCfg.GroupChannel;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, &Channel, pDest);
        FrameLen += TempLen;
        pDest += TempLen;
        P2pLen +=TempLen;
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, &Channel, pDest);
        FrameLen += TempLen;
        pDest += TempLen;
        P2pLen +=TempLen;
        // Shall include Group ID if I am GO
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pPort->CurrentAddress, pDest);
        FrameLen += TempLen;
        pDest += TempLen;
        P2pLen +=TempLen;

    }
    else
    {
        Channel = pP2pEntry->OpChannel;
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, &Channel, pDest);
        FrameLen += TempLen;
        pDest += TempLen;
        P2pLen +=TempLen;
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, &Channel, pDest);
        FrameLen += TempLen;
        pDest += TempLen;
        P2pLen +=TempLen;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become ENROLLEE!!   ENROLLEE.!!\n"));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - Opchannel is %d \n", Channel));
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2pLen is %d \n", P2pLen));

    pFrame->Length = (UCHAR)P2pLen;
    pP2pEntry->P2pClientState = P2PSTATE_WAIT_GO_COMFIRM_ACK;
    // PFP2P
    pP2pEntry->StateCount = 0;
    //P2pSetWps(pAd, pPort);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);

}



/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving Go Negociation Confirm packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceGoNegoConfirmAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    UCHAR       DevType[8], DevAddr[6], IfAddr[6], Channel, OpChannel, Intent, index, Ssid[32];
    UCHAR       GroupCap, DeviceCap, DeviceNameLen;
    UCHAR       StatusCode = 12, SsidLen = 0;
    USHORT      Dpid, ConfigMethod;

    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Recieve Confirm Confirm Confirm. ->PortNum = %d", Elem->PortNum));
    pPort = pAd->PortList[Elem->PortNum];
    PlatformZeroMemory(Ssid, 32);
    // Get Request content capability
    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen + 7 - sizeof(P2P_PUBLIC_FRAME)), 
        FALSE, &Dpid, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, DevAddr, IfAddr, NULL, &SsidLen, Ssid, &ConfigMethod, NULL, DevType, NULL, NULL, &Channel, &OpChannel, NULL, &Intent, &StatusCode, NULL);
    // confirm doesn't attach device addr in subelement. So copy from SA.
    PlatformMoveMemory(DevAddr, pFrame->p80211Header.Addr2, MAC_ADDR_LEN);
    DBGPRINT(RT_DEBUG_TRACE, ("GroupCap = %x., DeviceCap = %x. DevAddr = %x %x %x %x %x %x  \n", GroupCap, DeviceCap, DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("Channel = %d . OpChannel = %d   \n",  Channel, OpChannel));
    
    // Check StatusCode.
    if (StatusCode != 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Ignore Go Negociation Confirm Status Code not Success. = %d \n", StatusCode));
        P2pGroupFormFailHandle(pAd);
        if (StatusCode == P2PSTATUS_REJECT_BY_USER)
        {
            pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_REJECTBYUSER;
            pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2P_ERRCODE_REJECTBYUSER  \n"));
        }
        else if (StatusCode == P2PSTATUS_NO_CHANNEL)
        {
            pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_CHANNELDISMATCH;
            pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2PSTATUS_NO_CHANNEL  \n"));
        }
        else if (StatusCode == P2PSTATUS_LIMIT)
        {
            pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_REACHLIMIT;
            pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2PSTATUS_LIMIT  \n"));
        }
        return;
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Recieve Confirm Confirm Confirm Success"));

    // Check Peer is in the valid state to receive Go Negociation Response.
    index = P2pGroupTabSearch(pAd, DevAddr);
    if (index == P2P_NOT_FOUND)
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Ignore Go Negociation Confirm from Unknown device. \n"));
        P2PPrintP2PEntry(pAd, 0);
        return;
    }
    else 
    {
        // Existing peer stay in another state. Doesn't need respond the Go Negociation Request.
        if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) && (pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState != P2PSTATE_WAIT_GO_COMFIRM))
        {
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - Existing peer stay in another state. = %d. return.\n", pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState));
            return;
        }
    }

    if (index < MAX_P2P_GROUP_SIZE)
    {
        PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[index];
        DBGPRINT(RT_DEBUG_TRACE, (" pP2pEntry[%d]->rule = %d. GoIntent = %d. My intent is %d. \n",  index, pP2pEntry->Rule, pP2pEntry->GoIntent, pPort->P2PCfg.GoIntentIdx));
        DBGPRINT(RT_DEBUG_TRACE, ("---->P2P pPort->P2PCfg.SsidLength = %d. [%x %x %x..]\n", pPort->P2PCfg.SSIDLen, 
        pPort->P2PCfg.SSID[0], pPort->P2PCfg.SSID[1], pPort->P2PCfg.SSID[2]));
        if (SsidLen > 0)
        {
            PlatformMoveMemory(pP2pEntry->Ssid, Ssid, 32);
            pP2pEntry->SsidLen = SsidLen;
        }
        
        if (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT)
        {   
            // I become Client
            pP2pEntry->Rule = P2P_IS_GO;
            pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
            pP2pEntry->OpChannel= OpChannel;
            pPort->P2PCfg.GroupChannel = OpChannel;
            pPort->P2PCfg.GroupOpChannel = OpChannel;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become ENROLLEE!!GOGO Go's Bssid = %x %x %x %x %x %x!!\n", pPort->P2PCfg.Bssid[0], pPort->P2PCfg.Bssid[1],pPort->P2PCfg.Bssid[2],pPort->P2PCfg.Bssid[3],pPort->P2PCfg.Bssid[4],pPort->P2PCfg.Bssid[5]));
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become ENROLLEE!!GOGO Go's DevAddr = %x %x %x %x %x %x!!\n", pPort->P2PCfg.GroupDevAddr[0], pPort->P2PCfg.GroupDevAddr[1],pPort->P2PCfg.GroupDevAddr[2],pPort->P2PCfg.GroupDevAddr[3],pPort->P2PCfg.GroupDevAddr[4],pPort->P2PCfg.GroupDevAddr[5]));
            PlatformMoveMemory(pPort->P2PCfg.SSID, pP2pEntry->Ssid, 32);
            pPort->P2PCfg.SSIDLen = pP2pEntry->SsidLen;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become ENROLLEE!GOGO Go's SsidLen = %d.!!\n", pP2pEntry->SsidLen)); 
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become ENROLLEE!!GOGO Go's SSID = %x %x %x %x %x %x!!\n", pPort->P2PCfg.SSID[0], pPort->P2PCfg.SSID[1],pPort->P2PCfg.SSID[2],pPort->P2PCfg.SSID[3],pPort->P2PCfg.SSID[4],pPort->P2PCfg.SSID[5]));
            P2pGoNegoDone(pAd,  pPort, pP2pEntry);
        }
        else
        {
            // I become GO
            pP2pEntry->Rule = P2P_IS_CLIENT;
            pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
            pP2pEntry->P2pClientState = P2PSTATE_GOT_GO_COMFIRM;
            P2pGoNegoDone(pAd,  pPort, pP2pEntry);
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
        }

    }

    P2PPrintState(pAd);
}
/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving Go Negociation Response packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceGoNegoRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)&Elem->Msg[0];
    PMP_PORT  pPort;
    UCHAR       DevType[8], Ssid[32], DevAddr[6], IfAddr[6], Channel, OpChannel, Intent, index;
    UCHAR       GroupCap, DeviceCap, DeviceName[32], DeviceNameLen;
    UCHAR       SsidLen = 0;
    PRT_P2P_CLIENT_ENTRY pP2pEntry;
    UCHAR       StatusCode = 8, RspStatus = 0;
    UCHAR       TempIntent;
    USHORT      Dpid, ConfigMethod;
    UCHAR       Index = 0;
    BOOLEAN     StopGroup = FALSE;
    UCHAR       ChannelList[MAX_NUM_OF_CHANNELS];

    DBGPRINT(RT_DEBUG_TRACE, (" P2P -  GOGOGO Recieve Response.Response MsgLen = %d.", Elem->MsgLen));
    pPort = pAd->PortList[Elem->PortNum];
    PlatformZeroMemory(ChannelList, sizeof(ChannelList));

    // Get Request content capability
    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8),  
        FALSE, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, 
        DevAddr, IfAddr, NULL, &SsidLen, Ssid, &ConfigMethod, NULL, DevType, NULL, NULL, &Channel, &OpChannel, ChannelList, &TempIntent, &StatusCode, NULL);

    Intent = TempIntent >>1;
    DBGPRINT(RT_DEBUG_TRACE, (" Dev Addr = %x %x %x %x %x %x. Intent = %d. My Intent = %d. \n",  DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5], Intent, pPort->P2PCfg.GoIntentIdx));
    DBGPRINT(RT_DEBUG_TRACE, ("   interface addr = %x %x %x %x %x %x  \n",  IfAddr[0], IfAddr[1],IfAddr[2],IfAddr[3],IfAddr[4],IfAddr[5]));

    DBGPRINT(RT_DEBUG_TRACE, (" P2P -StatusCode = %d. PeerIntent = %d . ListenChannel = %d.OpChannel = %d. My Intent = %d.\n", StatusCode, Intent, Channel, OpChannel, pPort->P2PCfg.GoIntentIdx));

    //Check whether the selected channel is valid
    P2pCheckAndUpdateGroupChannel(pAd, pPort, ChannelList);

    // Check StatusCode.
    if (StatusCode == P2PSTATUS_BOTH_INTENT15)
    {
        // since peer return that info unknown, then stop resend GO Negociation Req.
        // To prepare for retry GO Nego REQ.
        index = P2pGroupTabSearch(pAd, DevAddr);
        if (index < MAX_P2P_GROUP_SIZE)
        {
            // since peer return that info unknown, then stop resend GO Negociation Req.
            // To prepare for retry GO Nego REQ.
            pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY;
            // Update P2PConnectState.
            if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT)||(pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO))
                pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P -  = %s \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        }
        // Report both intent15 to Sigma
        pPort->P2PCfg.SigmaQueryStatus.ErrorCode = P2PSTATUS_BOTH_INTENT15;
        pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_BOTHINTENT15;
        pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Receive Status Code that both Go Intent Value is 15.  = %x \n", StatusCode));
        return;
    }
    else if (StatusCode != 0)
    {
        pPort->P2PCfg.SigmaQueryStatus.ErrorCode = StatusCode;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P -Go Negociation Response Status Code PASSED or being rejected  = %d \n", StatusCode));
        index = P2pGroupTabSearch(pAd, DevAddr);
        if (index < MAX_P2P_GROUP_SIZE)
        {
            // since peer return that info unknown, then stop resend GO Negociation Req.
            // doesn't need to prepare for retry GO Nego REQ.
            pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY;
            // Update P2PConnectState.
            if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT)||(pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO))
                pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
            if (StatusCode == P2PSTATUS_REJECT_BY_USER)
            {
                pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_REJECTBYUSER;
                pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2P_ERRCODE_REJECTBYUSER  \n"));
            }
            else if (StatusCode == P2PSTATUS_NO_CHANNEL)
            {
                pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_CHANNELDISMATCH;
                pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2PSTATUS_NO_CHANNEL  \n"));
            }
            else if (StatusCode == P2PSTATUS_LIMIT)
            {
                pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_REACHLIMIT;
                pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2PSTATUS_LIMIT  \n"));
            }
            else if((StatusCode == P2PSTATUS_PASSED) &&
                (pPort->P2PCfg.Dpid == DEV_PASS_ID_USER) &&
                (pPort->P2PCfg.ConfigMethod == CONFIG_METHOD_KEYPAD))
            {
                
                pPort->P2PCfg.PopUpIndex = index;
                DBGPRINT(RT_DEBUG_TRACE, ("pop window. %d\n", pPort->P2PCfg.PopUpIndex));
                pAd->pP2pCtrll->P2PTable.Client[index].ConfigMethod = CONFIG_METHOD_DISPLAY;
                P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS);
                pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY;
                // for group formation actively using enter pin method, we need send nego resp with status 1 to wait user enter pin
                pPort->P2PCfg.LockNego = TRUE;
            }
            
            pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD;
            //AsicSwitchChannel(pAd,pPort->P2PCfg.ListenChannel, FALSE);
            AsicSwitchChannel(pPort, pPort->P2PCfg.ListenChannel, pPort->P2PCfg.ListenChannel, BW_20, FALSE);
            DBGPRINT(RT_DEBUG_TRACE, ("stay the channel %d to wait Nego Req", pPort->ScaningChannel));
            DBGPRINT(RT_DEBUG_TRACE, (" P2P -  = %s \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        }
        return;
    }

    // Check Peer is in the valid state to receive Go Negociation Response.
    index = P2pGroupTabSearch(pAd, DevAddr);
    if (index == P2P_NOT_FOUND)
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Ignore Go Negociation Response from Unknown device. \n"));
        return;
    }
    else 
    {
    }

    if (index < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[index];
        // Check peer capability
        pP2pEntry->ListenChannel = Channel;
        pP2pEntry->OpChannel = OpChannel;
        pP2pEntry->GoIntent = Intent;
        pP2pEntry->StateCount = 0;
        if (SsidLen > 0)
        {
            PlatformMoveMemory(pP2pEntry->Ssid, Ssid, 32);
            pP2pEntry->SsidLen = SsidLen;
        }
        P2PPrintP2PEntry(pAd, index);
        // If this peer is provistioned, dpid should follows spec's assignment on page 33
        if (P2P_TEST_FLAG(pP2pEntry, P2PFLAG_PROVISIONED))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("P2P provisioned -dpid= %x. ConfigMethod = %s.\n", Dpid, decodeConfigMethod(pP2pEntry->ConfigMethod)));
            switch(pP2pEntry->ConfigMethod)
            {
                case CONFIG_METHOD_DISPLAY:
                    if ((Dpid != DEV_PASS_ID_REG) && (Dpid != DEV_PASS_ID_PIN))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, (" P2P -1 Ignore Go Negociation Response with wrong dpid \n"));
                        return;
                    }
                    break;
                case CONFIG_METHOD_LABEL:
                    if ((Dpid != DEV_PASS_ID_REG) && (Dpid != DEV_PASS_ID_PIN))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, (" P2P -2 Ignore Go Negociation Response with wrong dpid \n"));
                        return;
                    }
                    break;
                case CONFIG_METHOD_KEYPAD:
                    if (Dpid != DEV_PASS_ID_USER)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, (" P2P -3 Ignore Go Negociation Response with wrong dpid \n"));
                        return;
                    }
                    break;
                default:
                    break;
            }
        }
        P2pSetRule(pAd, index, pPort, IfAddr, TempIntent, OpChannel, DevAddr);

        if (pP2pEntry->Rule == P2P_IS_CLIENT )
        {
                
            if (pPort->CommonCfg.P2pControl.field.EnableIntraBss == 1)
            {
                pPort->P2PCfg.P2pCapability[1] |= GRPCAP_INTRA_BSS;
                pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = FALSE;

                DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Intra BSS. \n"));
            }
            else
            {
                pPort->P2PCfg.P2pCapability[1] &=  (~GRPCAP_INTRA_BSS);
                pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Disable Intra BSS. \n"));
            }
                
        }       
        else if(pP2pEntry->Rule == P2P_IS_GO) // i am GO
        {
                pPort->P2PCfg.P2pCapability[1] &=  (~GRPCAP_INTRA_BSS);
                pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = TRUE;
        }
        
        if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) && (pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState != P2PSTATE_SENT_GO_NEG_REQ))
        {
            DBGPRINT(RT_DEBUG_TRACE, (" P2P -  Existing peer stay in another state. = %d. \n", pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState));
            return;
        }
        
        // If Port0 connected to WlanAP and its channel is not the same with Port2, resume Port2 group.
        if (((pAd->OpMode == OPMODE_STAP2P) && (PORTV2_P2P_ON(pAd, pPort)) && INFRA_ON(pAd->PortList[PORT_0])) &&
            ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT) && (pP2pEntry->Rule == P2P_IS_GO) && (pPort->P2PCfg.GroupOpChannel != pPort->Channel)))
        {
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - Error!!! Resume group due to incompatible channel as Port0.(ch=%d %d) \n", pPort->P2PCfg.GroupOpChannel, pPort->Channel));
            RspStatus = P2PSTATUS_NO_CHANNEL;
            StopGroup = TRUE;
        }

        pP2pEntry->FrameToken = pFrame->Token;
        P2PSendGoNegoConfirm(pAd, pPort, pFrame->Token, index, pP2pEntry->addr, RspStatus);
    }
    P2PPrintState(pAd);

    if (StopGroup == TRUE)
    {
        pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_CHANNELDISMATCH;
        pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
        P2pDown(pAd, pPort);
        // Delete P2P Device that I previously tried to connect.
        for (Index = 0; Index < MAX_P2P_GROUP_SIZE; Index++)
        {
            if (pAd->pP2pCtrll->P2PTable.Client[Index].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                P2pGroupTabDelete(pAd, Index, pAd->pP2pCtrll->P2PTable.Client[Index].addr);
        }
        P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
        return;
    }
}

/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving Go Negociation Request packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceGoNegoReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort;
    UCHAR       DevType[8], DevAddr[6], BssidAddr[6], Channel, OpChannel, Intent, index, StatusCode;
    UCHAR       GroupCap, DeviceCap, DeviceName[32], DeviceNameLen;
    ULONG       FrameLen;
    ULONG       TempLen;
    UCHAR       SsidLen = 0;
    RT_P2P_CLIENT_ENTRY *pP2pEntry;
    NDIS_STATUS NStatus;
    PUCHAR      pOutBuffer = NULL;
    PUCHAR      pDest;
    UCHAR       RspStatus = P2PSTATUS_SUCCESS;
    UCHAR       TempIntent;
    USHORT      Dpid, SentDpid, ConfigMethod = 0;
    UCHAR       ChannelList[MAX_NUM_OF_CHANNELS];
    BOOLEAN     StopGroup = FALSE;
    UCHAR       Index = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("GOGOGOGO P2P - receive Go Neg Request. Request PortNum = %d. MsgLen = %d ", Elem->PortNum, Elem->MsgLen));
    PlatformZeroMemory(ChannelList, sizeof(ChannelList));
    pPort = pAd->PortList[Elem->PortNum];
    pP2pEntry = NULL;
    // Didn't allow new req when doing provisioning.
    if (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_REGISTRA(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" GOGOGOGO P2P - receive Go Neg Request. return on %s", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        return;
    }
    // Get Request content capability
    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8),
        FALSE, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, 
        BssidAddr, NULL, &SsidLen, NULL, &ConfigMethod, NULL, DevType, NULL, NULL, &Channel, &OpChannel, ChannelList, &TempIntent, &StatusCode, NULL);

    Intent = TempIntent >>1;
    DBGPRINT(RT_DEBUG_TRACE, (" DevAddr in P2P IE = %x %x %x %x %x %x  \n",  DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, (" Addr2 = %x %x %x %x %x %x  \n",  pFrame->p80211Header.Addr2[0], pFrame->p80211Header.Addr2[1],pFrame->p80211Header.Addr2[2],pFrame->p80211Header.Addr2[3],pFrame->p80211Header.Addr2[4],pFrame->p80211Header.Addr2[5]));
    DBGPRINT(RT_DEBUG_TRACE, (" its bssid = %x %x %x %x %x %x  \n",  BssidAddr[0], BssidAddr[1],BssidAddr[2],BssidAddr[3],BssidAddr[4],BssidAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, (" Listen channel = %d. Dpid = %x ,%s My Dpid = %x LockNego = %d\n", Channel, Dpid, decodeDpid(Dpid), pPort->P2PCfg.Dpid, pPort->P2PCfg.LockNego));

    //Check whether the selected channel is valid
    P2pCheckAndUpdateGroupChannel(pAd, pPort, ChannelList);

    index = P2pGroupTabSearch(pAd, DevAddr);
     
    DBGPRINT(RT_DEBUG_TRACE, ("P2P -Peer[%d] Intent = %x . OpChannel = %x. My Intent = %x. TempIntent = %x\n", index, Intent, Channel, pPort->P2PCfg.GoIntentIdx, TempIntent));
    if (index == P2P_NOT_FOUND)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2P -1  insert\n"));
        index = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY, NULL, 0);
        if (index < MAX_P2P_GROUP_SIZE)
        {
            pPort->P2PCfg.PopUpIndex = index;
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].DeviceName, DeviceName, 32);
            DBGPRINT(RT_DEBUG_TRACE, ("From a unknown peer. Pop up setting windows. %d\n", pPort->P2PCfg.PopUpIndex));
            pAd->pP2pCtrll->P2PTable.Client[index].DeviceNameLen = DeviceNameLen;
            pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[index];
            pP2pEntry->ConfigMethod = ConfigMethod;
        }
    }
    
    if (index < MAX_P2P_GROUP_SIZE)
    {
        P2PPrintP2PEntry(pAd, index);
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[index];

        // If peer is provisioned, don't update. We treat him only support this provisioned configmethod.
        if (!P2P_TEST_FLAG(pP2pEntry, P2PFLAG_PROVISIONED) && (ConfigMethod != 0))
            pP2pEntry->ConfigMethod = ConfigMethod;
        
        DBGPRINT(RT_DEBUG_TRACE, ("P2P  Dpid = [%x]  %s \n", pPort->P2PCfg.Dpid, decodeP2PClientState(pP2pEntry->P2pClientState)));
        if (IS_P2P_PEER_CLIENT_OP(pP2pEntry) || IS_P2P_PEER_WPAPSK(pP2pEntry) || IS_P2P_PEER_PROVISIONING(pP2pEntry))
        {
            // Receive Go Neg Req when this peer's state is operating or doing provisioning. Delete this peer.
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - Existing peer  state is %d. %s,.  Delete it.\n", pP2pEntry->P2pClientState, decodeP2PClientState(pP2pEntry->P2pClientState)));

            return;
        }
        if (pP2pEntry->P2pClientState > P2PSTATE_GO_DONE)
        {
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - Existing peer  state is %d . %s,. return.\n", pP2pEntry->P2pClientState, decodeP2PClientState(pP2pEntry->P2pClientState)));
            return;
        }
        else if (pP2pEntry->P2pClientState == P2PSTATE_SENT_GO_NEG_REQ)
        {
            if (IsP2pFirstMacSmaller(pP2pEntry->addr, pPort->CurrentAddress))
            {
                DBGPRINT(RT_DEBUG_TRACE, (" P2P - Dual GO Req. Existing peer  state is %s. \n", decodeP2PClientState(pP2pEntry->P2pClientState)));
                return;
            }
        }
        if (IS_P2P_GO_OP(pPort))
        {
            // I am GO . Don't need go through GO NEgo process. So return fail.
            RspStatus = P2PSTATUS_INVALID_PARM;
        }
        else if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_NOSPEC) || (pPort->P2PCfg.LockNego == TRUE))
        {
            pPort->P2PCfg.PopUpIndex = index;
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].DeviceName, DeviceName, 32);
            DBGPRINT(RT_DEBUG_TRACE, ("Need setting first. Pop up setting windows. %d\n", pPort->P2PCfg.PopUpIndex));
            pAd->pP2pCtrll->P2PTable.Client[index].DeviceNameLen = DeviceNameLen;
            if(pPort->P2PCfg.P2pEventQueue.LastSetEvent != P2PEVENT_POPUP_SETTING_WINDOWS)
            {
                if(FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS))
                    pPort->P2PCfg.P2pEventQueue.bPopWindow= TRUE;
            }
            pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[index];
            DBGPRINT(RT_DEBUG_TRACE, ("Peer Capability. %x %x \n", DeviceCap, GroupCap));
            if (MT_TEST_BIT(GroupCap, 0x1))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Error sending me Own bit ON. in Go Nego Req..  \n"));
            }
            if (pP2pEntry->P2pClientState == P2PSTATE_DISCOVERY_GO)
            {
                pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY;
                pP2pEntry->Rule = P2P_IS_CLIENT;
                DBGPRINT(RT_DEBUG_TRACE, ("Change peer state from P2PSTATE_DISCOVERY_GO to P2PSTATE_DISCOVERY \n"));
            }
            RspStatus = P2PSTATUS_PASSED; 
            P2P_SET_FLAG(pP2pEntry, P2PFLAG_PASSED_NEGO_REQ);
            // do not need to ban sending Go Nego now.
            pPort->P2PCfg.SigmaSetting.bWaitGoNegReq = FALSE;
            // The following is for Sigma Test. I need to automatically start go nego after I get the WPS credential setting.
            // 
            PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*MAC_ADDR_LEN);
            PlatformMoveMemory(&pPort->P2PCfg.ConnectingMAC[0][0], pP2pEntry->addr, MAC_ADDR_LEN);
            // Set to 0xff means to connect to first Connecting MAC
            pPort->P2PCfg.ConnectingIndex = 0xff;
        }
    }
    // Ban Request from the same GoIntent index = 15.
    if ((Intent == pPort->P2PCfg.GoIntentIdx) && (Intent == 15))
    {
        RspStatus = P2PSTATUS_BOTH_INTENT15;
        // Report both intent15 to Sigma
        pPort->P2PCfg.SigmaQueryStatus.ErrorCode = P2PSTATUS_BOTH_INTENT15;
        DBGPRINT(RT_DEBUG_TRACE, (" P2pReceGoNegoReqAction -Receive a peer that has the same Go Intent index as mine. \n"));
    }   
    else if (FALSE == P2pCheckChannelList(pAd, ChannelList))
    {
        RspStatus = P2PSTATUS_NO_CHANNEL;
        DBGPRINT(RT_DEBUG_TRACE, (" P2pReceGoNegoReqAction -Receive a peer that has no channel as mine. \n"));
    }
    else if (P2P_OFF(pPort))
    {
        RspStatus = P2PSTATUS_REJECT_BY_USER;
        DBGPRINT(RT_DEBUG_TRACE, (" P2pReceGoNegoReqAction -   P2P Off. \n"));
    }
    
        
    // allocate and send out GoNegoRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory  failf -  . \n"));
        return;
    }

    pDest = pOutBuffer;
    if ((RspStatus == P2PSTATUS_SUCCESS) && (pP2pEntry != NULL))
    {
        pP2pEntry->P2pClientState = P2PSTATE_WAIT_GO_COMFIRM;
        pP2pEntry->StateCount = 0;
        P2pStopScan(pAd, pPort);
        // Save  peer capability
        pP2pEntry->ListenChannel = Channel;
        pP2pEntry->OpChannel = OpChannel;
        P2pSetRule(pAd, index, pPort, BssidAddr, TempIntent, OpChannel, DevAddr);

        // After P2P Role is decided, check if OpChannel is valid.      
        // If Port0 is connected and its channel is not the same with Port2, resume Port2 group.
        if (((pAd->OpMode == OPMODE_STAP2P) && (PORTV2_P2P_ON(pAd, pPort)) && INFRA_ON(pAd->PortList[PORT_0])) &&
            ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT) && (pP2pEntry->Rule == P2P_IS_GO) && (pPort->P2PCfg.GroupOpChannel != pPort->Channel)))
        {
            RspStatus = P2PSTATUS_NO_CHANNEL;
            DBGPRINT(RT_DEBUG_TRACE, ("P2pReceGoNegoReqAction -Receive a peer that has the incompatible op channel as Port0.(ch=%d %d) \n", pPort->P2PCfg.GroupOpChannel, pPort->Channel));
            // Stop this group
            StopGroup = TRUE;
        }
        else
        {
            // If I am Go, Use my channel to set in the Go Nego Rsp.
            if (pP2pEntry->Rule == P2P_IS_CLIENT )
            {
                
                if (pPort->CommonCfg.P2pControl.field.EnableIntraBss == 1)
                {
                    pPort->P2PCfg.P2pCapability[1] |= GRPCAP_INTRA_BSS;
                    pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = FALSE;

                    DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Intra BSS. \n"));
                }
                else
                {
                    pPort->P2PCfg.P2pCapability[1] &=  (~GRPCAP_INTRA_BSS);
                    pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Disable Intra BSS. \n"));
                }
                            OpChannel = pPort->P2PCfg.GroupChannel;
            }
                
            else if(pP2pEntry->Rule == P2P_IS_GO) // I am Client
            {
                pPort->P2PCfg.P2pCapability[1] &=  (~GRPCAP_INTRA_BSS);
                pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = TRUE;
            }
            
            // Change beacon content
            pPort->P2PCfg.P2pCapability[1] |= (GRPCAP_GROUP_FORMING);
        }
    }
            
    SentDpid = DEV_PASS_ID_PIN;
    if (P2P_TEST_FLAG(pP2pEntry, P2PFLAG_PROVISIONED))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P -   Go Negociation Responce to provisioned  \n"));
        if (pPort->P2PCfg.ConfigMethod == CONFIG_METHOD_DISPLAY)
            SentDpid = DEV_PASS_ID_REG;
        else if (pPort->P2PCfg.ConfigMethod == CONFIG_METHOD_LABEL)
            SentDpid = DEV_PASS_ID_PIN;
        else if  (pPort->P2PCfg.ConfigMethod == CONFIG_METHOD_KEYPAD)
            SentDpid = DEV_PASS_ID_USER;
        else if (pPort->P2PCfg.ConfigMethod == CONFIG_METHOD_PUSHBUTTON)
            SentDpid = DEV_PASS_ID_PBC;
    }
    else
    {
        if (Dpid == DEV_PASS_ID_PIN)
            SentDpid = DEV_PASS_ID_PIN;
        else if (Dpid == DEV_PASS_ID_USER)
            SentDpid = DEV_PASS_ID_REG;
        else if  (Dpid == DEV_PASS_ID_REG)
            SentDpid = DEV_PASS_ID_USER;
        else if (Dpid == DEV_PASS_ID_PBC)
            SentDpid = DEV_PASS_ID_PBC;
    }

    DBGPRINT(RT_DEBUG_TRACE, (" P2P - ReceDpid = %x. SentDpid %x \n", (Dpid), (SentDpid)));
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - ReceDpid = %s. SentDpid %s \n", decodeDpid(Dpid), decodeDpid(SentDpid)));
    P2PMakeGoNegoRsp(pAd, pPort, DevAddr, SentDpid, pFrame->Token, TempIntent, OpChannel, RspStatus, pOutBuffer, &TempLen);
    FrameLen = TempLen;
    pDest += TempLen;
    // : copy req to rsp first.

    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Make Go Negociation Responce Length = %d.RspStatus = %d \n", FrameLen, RspStatus));
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
    P2PPrintState(pAd);
    // else ignore this Go Negociation Request because P2P table reach maximum.

    if (StopGroup == TRUE)
    {
        pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_CHANNELDISMATCH;
        pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
        P2pDown(pAd, pPort);
        // Delete P2P Device that I previously tried to connect.
        for (Index = 0; Index < MAX_P2P_GROUP_SIZE; Index++)
        {
            if (pAd->pP2pCtrll->P2PTable.Client[Index].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                P2pGroupTabDelete(pAd, Index, pAd->pP2pCtrll->P2PTable.Client[Index].addr);
        }
        P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
        return;
    }       
}


/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving Client Discovery Request Public Action Frame packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceDevDisReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort;
    UCHAR       devAddr[6], BssidAddr[6], OpChannel, StatusCode;
    UCHAR       GroupCap, DeviceCap, DeviceNameLen;
    UCHAR       SsidLen;
    MLME_P2P_ACTION_STRUCT  P2PActReq;  
    ULONG           TotalFrameLen;
    RT_P2P_CLIENT_ENTRY     *pEntry = NULL;
    UCHAR       p2pindex;
    BOOLEAN     SigmaFlag = TRUE;
    
    pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2pReceDevDisReqAction = %d ", Elem->MsgLen));

    // Get Request content capability
    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
        FALSE, NULL, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, devAddr, NULL, 
        BssidAddr, &SsidLen, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &OpChannel, NULL, NULL, &StatusCode, NULL);
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - ask for p2p client = %x %x %x  ", devAddr[3], devAddr[4], devAddr[5]));
    // Use TX to decide who is sending.
    p2pindex = P2pGroupTabSearch(pAd, devAddr);

    // Cheat for Sigma-6.1.10, Atheros AP sometimes fail to send disassociation packet to STA and we may fail this item easily.
    // So we force send fail reply after receiving second DevDisReq packet.
    if (IS_P2P_SIGMA_ON(pPort) && pPort->P2PCfg.DevDisReqCount >= 1)
        SigmaFlag = FALSE;

    if (((p2pindex < MAX_P2P_GROUP_SIZE) && (IS_P2P_PEER_CLIENT_OP(&pAd->pP2pCtrll->P2PTable.Client[p2pindex])))
        && (SigmaFlag))
    {
        pEntry = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
        DBGPRINT(RT_DEBUG_TRACE, ("P2P -p2p client = %s  ", decodeP2PClientState(pEntry->P2pClientState)));
        PlatformZeroMemory(&P2PActReq, sizeof(P2PActReq));
        DBGPRINT(RT_DEBUG_TRACE, ("to interface Addr = %x %x %x %x %x %x  \n",  pEntry->InterfaceAddr[0], pEntry->InterfaceAddr[1],pEntry->InterfaceAddr[2],pEntry->InterfaceAddr[3],pEntry->InterfaceAddr[4],pEntry->InterfaceAddr[5]));
        COPY_MAC_ADDR(P2PActReq.Addr, pEntry->InterfaceAddr);
        P2PActReq.TabIndex = p2pindex;
        MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_STATE_MACHINE, MT2_MLME_P2P_GO_DIS_REQ, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq);
        // Device Discvoery  Response is delayed until I get the GO Discovery Request Frame's Ack.
        //Cheat for Sigma-6.1.10 --> Assume always success.  Send back response.
        pPort->P2PCfg.DevDisReqCount++;
        PlatformMoveMemory(&pPort->P2PCfg.LatestP2pPublicFrame, pFrame, sizeof(P2P_SAVED_PUBLIC_FRAME));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2P -can't find p2p client .Status fail"));
        PlatformMoveMemory(&pPort->P2PCfg.LatestP2pPublicFrame, pFrame, sizeof(P2P_SAVED_PUBLIC_FRAME));
        P2PSendDevDisRsp(pAd, pPort, P2PSTATUS_IMCOMPA_PARM, pPort->P2PCfg.LatestP2pPublicFrame.Token, pPort->P2PCfg.LatestP2pPublicFrame.p80211Header.Addr2, &TotalFrameLen);
    }
}


/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving Client Discovery Response Public Action Frame packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceDevDisRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort;
    UCHAR       DevType[8], DevAddr[6], BssidAddr[6], Channel, OpChannel, index, StatusCode;
    UCHAR       GroupCap, DeviceCap, DeviceNameLen, TempIntent;
    UCHAR       SsidLen = 0;
    RT_P2P_CLIENT_ENTRY *pP2pEntry;
    PUCHAR          pOutBuffer = NULL;
    USHORT      Dpid, ConfigMethod;
    UCHAR       ClientP2PIndex = P2P_NOT_FOUND, i;
    
    DBGPRINT(RT_DEBUG_TRACE, ("GOGO  P2P - P2pReceDevDisRspAction MsgLen = %d ", Elem->MsgLen));
    pPort = pAd->PortList[Elem->PortNum];
    pP2pEntry = NULL;

    index = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
    if (index == P2P_NOT_FOUND)
        return;

    pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[index];

    DBGPRINT(RT_DEBUG_TRACE, ("P2pReceDevDisRspAction %s ", decodeP2PClientState(pP2pEntry->P2pClientState)));
    if (pP2pEntry->P2pClientState != P2PSTATE_GO_DISCO_COMMAND)
        return;
    // Change State back to P2PSTATE_DISCOVERY_GO
    // Because P2PSTATE_GO_DISCO_COMMAND is from P2PSTATE_DISCOVERY_GO
    pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY_GO;
    // Get Request content capability
    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8),
        FALSE, &Dpid, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, DevAddr, 
        BssidAddr, NULL, &SsidLen, NULL, &ConfigMethod, NULL, DevType, NULL, NULL, &Channel, &OpChannel, NULL, &TempIntent, &StatusCode, NULL);

    if (StatusCode == P2PSTATUS_SUCCESS)
    {
        for (i = index; i < MAX_P2P_GROUP_SIZE;i++)
        {
            if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_DISCO_COMMAND)
                ClientP2PIndex = i;
        }
        if (ClientP2PIndex == P2P_NOT_FOUND)
            return;

        if (IS_P2P_SIGMA_ON(pPort))
        {
            // Don't want to start group forming right now.
            pAd->pP2pCtrll->P2PTable.Client[ClientP2PIndex].P2pClientState = P2PSTATE_CONNECT_COMMAND;
            P2pStartGroupForm(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[ClientP2PIndex].addr, ClientP2PIndex);
        }
    }
    else
    {
        for (i = index; i < MAX_P2P_GROUP_SIZE;i++)
        {
            if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_DISCO_COMMAND)
                pAd->pP2pCtrll->P2PTable.Client[ClientP2PIndex].P2pClientState = P2PSTATE_DISCOVERY_CLIENT;
        }

    }
            
}

/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving Invitation Request Action Frame packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceInviteReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort;
    UCHAR       DevType[8], Ssid[32], DevAddr[6], BssidAddr[6], OpChannel, index, StatusCode;
    UCHAR       GroupCap, DeviceCap, DeviceNameLen;
    ULONG       FrameLen;
    UCHAR       ChannelList[MAX_NUM_OF_CHANNELS];
    UCHAR       SsidLen, p2pindex;
    UCHAR       RspStatus = P2PSTATUS_INVALID_PARM;
    UCHAR       MyRule = P2P_IS_GO;
    USHORT      ConfigMethod;
    UCHAR       InviteFlag;
    BOOLEAN     bReinvoke = FALSE;
    BOOLEAN     bAct = FALSE;
    MAC_TABLE_ENTRY     *pEntry;
    
    pPort = pAd->PortList[Elem->PortNum];

    PlatformZeroMemory(ChannelList, MAX_NUM_OF_CHANNELS);
    PlatformZeroMemory(BssidAddr, MAC_ADDR_LEN);
    OpChannel = 0;
    // Get Request content capability
    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
        FALSE, NULL, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, DevAddr, NULL, 
        BssidAddr, &SsidLen, Ssid, &ConfigMethod, NULL, DevType, NULL, NULL, NULL, &OpChannel, ChannelList, NULL, &StatusCode, &InviteFlag);
    DBGPRINT(RT_DEBUG_TRACE, ("P2pReceInviteReqAction ==>InviteFlag = %d.  %s \n", InviteFlag, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    // Use TX to decide who is sending.
    PlatformMoveMemory(DevAddr, pFrame->p80211Header.Addr2, MAC_ADDR_LEN);
    DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.Dpid = %d \n", pPort->P2PCfg.Dpid));

    DBGPRINT(RT_DEBUG_TRACE, ("OpChannel = %d   InviteFlag = %x  \n",  OpChannel, InviteFlag));
    DBGPRINT(RT_DEBUG_TRACE, ("BssidAddr = %x %x %x %x %x %x  \n",  BssidAddr[0], BssidAddr[1],BssidAddr[2],BssidAddr[3],BssidAddr[4],BssidAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("DevAddr = %x %x %x %x %x %x  \n",  DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("Ssid = %c %c %c %c %c %c %c %c %c  \n",  Ssid[0], Ssid[1],Ssid[2],Ssid[3],Ssid[4],Ssid[5],Ssid[6],Ssid[7],Ssid[8]));
    index = P2pPerstTabSearch(pAd, DevAddr);
    DBGPRINT(RT_DEBUG_TRACE, ("perst index = %d.  \n", index));
    DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.Dpid = %d \n", pPort->P2PCfg.Dpid));

    // Reflash this flag on the next Invitation Request or DPID setting.
    pPort->P2PCfg.SigmaQueryStatus.bInvitationSucced = FALSE;

    if ((index < MAX_P2P_TABLE_SIZE) && ((InviteFlag & P2P_INVITE_FLAG_REINVOKE) == P2P_INVITE_FLAG_REINVOKE))
        bReinvoke = TRUE;
            
    // case 1: Reinvoke case:
    // If I have credential and both enable persistent. 
    if (bReinvoke == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Invite: Reinvoke  \n"));
        P2pCheckInviteReq(pAd, (pAd->pP2pCtrll->P2PTable.PerstEntry[index].MyRule == P2P_IS_GO), index, ChannelList, DevAddr, OpChannel, Ssid, SsidLen, &RspStatus);
        if (RspStatus == P2PSTATUS_SUCCESS)
        {
            // Find peer,. So set Status = success.
            if ((pAd->pP2pCtrll->P2PTable.PerstEntry[index].MyRule == P2P_IS_GO) 
                && (pPort->P2PCfg.P2PConnectState == P2P_INVITE || pPort->P2PCfg.P2PConnectState == P2P_GO_ASSOC_AUTH  || pPort->P2PCfg.LockNego))
            {
                RspStatus = P2PSTATUS_PASSED;
            }
            else if(pAd->pP2pCtrll->P2PTable.PerstEntry[index].MyRule == P2P_IS_GO)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("2 P2PConnectState = %d", pPort->P2PCfg.P2PConnectState));
                pPort->P2PCfg.LockNego = TRUE;
                pPort->P2PCfg.P2pCounter.CounterP2PAcceptReinvoke = 50; //reserve  secs to let client do persistent
                MyRule = P2P_IS_GO;
                OpChannel = pPort->P2PCfg.GroupChannel;
                pPort->P2PCfg.GroupOpChannel = OpChannel;
                pPort->StaCfg.WscControl.WscAPChannel = OpChannel;
                P2pCopyPerstParmToCfg(pAd, pPort, index);
                DBGPRINT(RT_DEBUG_TRACE, ("Invite: Decide to use OpChannel = %d for group \n", OpChannel));
                p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_CLIENT_WPS, Ssid, SsidLen);
                // update the device type and UI icon will correct
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].PrimaryDevType, DevType, 8);

                pAd->pP2pCtrll->P2PTable.Client[p2pindex].StateCount =5;
                if (p2pindex < MAX_P2P_GROUP_SIZE)
                {
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK;

                    // In order to do Auth, when p2p auto reconnect and i'm GO, we need to change the port security state
                    pEntry = MacTableLookup(pAd,  pPort, BssidAddr);
                    if (pEntry) 
                    {
                        pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;  
                    }
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Invite: reach my limit. Send back RspStatus = %d.  \n", RspStatus));
                    RspStatus = P2PSTATUS_LIMIT;
                }
            }
            else
            {
                MyRule = P2P_IS_CLIENT;
                pPort->P2PCfg.GroupChannel = OpChannel;
                pPort->P2PCfg.GroupOpChannel = OpChannel;
                P2pCopyPerstParmToCfg(pAd, pPort, index);
                DBGPRINT(RT_DEBUG_TRACE, ("Invite: Reinvoke as client for group \n"));
                pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
                // Update the Client state and SSID.
                p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_GO_WPS, Ssid, SsidLen);
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].PrimaryDevType, DevType, 8);
                //P2pGoNegoDone(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], &pAd->pP2pCtrll->P2PTable.Client[index]);
                pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_ENROLLEE;
                P2pWpsDone(pAd, DevAddr, pPort);
            }
        }
    }
    // case 2: abNormal invitation cases:...
    //  
    else if ((InviteFlag & P2P_INVITE_FLAG_REINVOKE) == P2P_INVITE_FLAG_REINVOKE)
    {
        // Peer ask me to reinvoke. But I can't find my record. Send back error code.
        DBGPRINT(RT_DEBUG_TRACE, ("Invite:  pPort->CommonCfg.P2pControl.field.EnablePresistent = %d    \n", pPort->CommonCfg.P2pControl.field.EnablePresistent));
        DBGPRINT(RT_DEBUG_TRACE, ("Invite:  Can't find Credential.    \n"));
    }

    // case 3: Normal invitation cases:
    //  
    else
    {
        if (PlatformEqualMemory(BssidAddr, ZERO_MAC_ADDR, MAC_ADDR_LEN))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Invite Req carry allzero Bssid, \n"));
        }
        else if (IS_P2P_CONNECT_IDLE(pPort) || (pPort->P2PCfg.P2PConnectState == P2P_INVITE))
        {
            P2pCheckInviteReqFromExisting(pAd, ChannelList, DevAddr, OpChannel, Ssid, SsidLen, &RspStatus);
            DBGPRINT(RT_DEBUG_TRACE, ("Invite: Got invitation from P2P  . RspStatus = %d \n", RspStatus));
            if (RspStatus == P2PSTATUS_SUCCESS)
            {
                if (IS_P2P_SIGMA_OFF(pPort))
                {
                    // =====>> Change =====>> Get DevAddr from Group ID
                    // It ought to use DevAddr to insert a P2P Table Client entry. But in this case, I can't get Dev addr. 
                    // so still use bssid addr to insert. Will copy bssid and InterfaceAddr after insert.
                    p2pindex = P2pGroupTabInsert(pAd, /*DevAddr*/BssidAddr, P2PSTATE_DISCOVERY_GO, Ssid, SsidLen);
                    if (p2pindex < MAX_P2P_GROUP_SIZE)
                    {
                        if (ConfigMethod != 0)
                            pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = ConfigMethod;
                        P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_GO_INVITE_ME);
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid, BssidAddr, MAC_ADDR_LEN);
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].InterfaceAddr, BssidAddr, MAC_ADDR_LEN);
                        pPort->P2PCfg.PopUpIndex = p2pindex;
                        P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS);
                    }
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Invite:  Update connecting MAC for Sigma testing.\n"));
                    PlatformMoveMemory(&pPort->P2PCfg.ConnectingMAC[0][0], BssidAddr, MAC_ADDR_LEN);
                    pPort->P2PCfg.ConnectingIndex = 0xff;
                    P2pConnect(pAd);
                }
                PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Bssid, BssidAddr, MAC_ADDR_LEN);
                PlatformMoveMemory(pPort->P2PCfg.Bssid, BssidAddr, MAC_ADDR_LEN);
                PlatformMoveMemory(pPort->P2PCfg.GroupDevAddr, DevAddr, MAC_ADDR_LEN);
                PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, Ssid, 32);
                pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen = SsidLen;
                pPort->P2PCfg.GroupOpChannel = OpChannel;

                // Save the desired connecting mac address, and then GO will connect to DEV once DPID has set down by Sigma. 
                pPort->P2PCfg.SigmaQueryStatus.bInvitationSucced = TRUE;
                PlatformMoveMemory(pPort->P2PCfg.SigmaQueryStatus.InviteConnectingMAC, DevAddr, 6);
                // Set MyRule for SigmaQry because DPID is not ready.
                pPort->P2PCfg.SigmaQueryStatus.MyRule = P2P_IS_CLIENT;
                MyRule = P2P_IS_CLIENT;
            }
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("P2p Send Invite Rsp RspStatus = %d.  \n", RspStatus));
    P2PMakeInviteRsp(pAd, pPort, MyRule, pFrame->Token, pFrame->p80211Header.Addr2, pPort->CurrentAddress, &OpChannel, &RspStatus, &FrameLen);
}

/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving Invitation Response Action Frame packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceInviteRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort;
    UCHAR       DevType[8], devAddr[6], BssidAddr[6], OpChannel, index, p2pindex, StatusCode;
    UCHAR       GroupCap, DeviceCap, DeviceNameLen;
    USHORT      ConfigMethod;
    UCHAR       SsidLen, ChannelList[MAX_NUM_OF_CHANNELS];
    pPort = pAd->PortList[Elem->PortNum];

    PlatformZeroMemory(ChannelList, 14);
    PlatformZeroMemory(BssidAddr, 6);
    // Get Request content capability
    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
        FALSE, NULL, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, devAddr, NULL, 
        BssidAddr, &SsidLen, NULL, &ConfigMethod, NULL, DevType, NULL, NULL, NULL, &OpChannel, ChannelList, NULL, &StatusCode, NULL);

    DBGPRINT(RT_DEBUG_TRACE, ("P2pReceInviteRspAction ==> StatusCode = %d. \n", StatusCode));
    DBGPRINT(RT_DEBUG_TRACE, ("TA = %x %x %x %x %x %x  \n",  pFrame->p80211Header.Addr2[0], pFrame->p80211Header.Addr2[1],pFrame->p80211Header.Addr2[2],pFrame->p80211Header.Addr2[3],pFrame->p80211Header.Addr2[4],pFrame->p80211Header.Addr2[5]));

    // Reflash this flag on the next Invitation Response or DPID setting.
    pPort->P2PCfg.SigmaQueryStatus.bInvitationSucced = FALSE;
    
    if ((StatusCode == P2PSTATUS_SUCCESS) /*|| (StatusCode == P2PSTATUS_PASSED)*/)
    {
        index = P2pPerstTabSearch(pAd, pFrame->p80211Header.Addr2);
        DBGPRINT(RT_DEBUG_TRACE, ("P2pPerstTabSearch ==> index = %d. \n", index));
        if ((index < MAX_P2P_TABLE_SIZE) 
            && ((pPort->P2PCfg.LastSentInviteFlag & P2P_INVITE_FLAG_REINVOKE) == P2P_INVITE_FLAG_REINVOKE))
        {
            // this is a persistent connection.
            pPort->P2PCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
            pPort->P2PCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
            // Find peer,. So set Status = success.
            if (pAd->pP2pCtrll->P2PTable.PerstEntry[index].MyRule == P2P_IS_GO)
            {
                OpChannel = pPort->P2PCfg.GroupChannel;
                pPort->P2PCfg.GroupOpChannel = OpChannel;
                P2pCopyPerstParmToCfg(pAd, pPort, index);
                DBGPRINT(RT_DEBUG_TRACE, ("Decide to use OpChannel = %d for group \n", OpChannel));
                p2pindex = P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_CLIENT_WPS, NULL, SsidLen);
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].PrimaryDevType, DevType, 8);
                P2pStartAutoGo(pAd, pPort);
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.Addr, pFrame->p80211Header.Addr2, MAC_ADDR_LEN);
                // Wps Done will set PMK to me.
                if (pAd->OpMode == OPMODE_STA)
                    P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                // this is not Auto GO by command from GUI. So set the intent index to != 16
                //  if I am auto GO and after persistent I still be auto GO
                if(IS_P2P_AUTOGO(pPort))
                    pPort->P2PCfg.GoIntentIdx = 16;
                else
                         pPort->P2PCfg.GoIntentIdx = 15;
            }
            else
            {
                pPort->P2PCfg.GroupChannel = OpChannel;
                pPort->P2PCfg.GroupOpChannel = OpChannel;
                pPort->StaCfg.WscControl.WscAPChannel = OpChannel;
                P2pCopyPerstParmToCfg(pAd, pPort, index);
                DBGPRINT(RT_DEBUG_TRACE, ("Reinvoke as client for group. OpChannel= %d\n", OpChannel));
                pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_ENROLLEE;
                // Add some delay to connect GO
                p2pindex = P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME, NULL, SsidLen);
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].PrimaryDevType, DevType, 8);
                if (index < MAX_P2P_GROUP_SIZE)
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigTimeOut = 25;
                
            }
        }
        else
        {
            index = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
            if (index < MAX_P2P_GROUP_SIZE)
            {
                if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
                {
                    // Last this P2P Device might start associate with me. Set its state to "finish Group Forming."
                    pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_GOT_GO_COMFIRM;
                    // stop search because I become GO.
                    pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
                    P2pSetListenIntBias(pAd, 1);
                    DBGPRINT(RT_DEBUG_ERROR, ("P2pReceInviteRspAction Success.  I am GO. \n"));
                }
                else if (IS_P2P_CLIENT_OP(pPort))
                    DBGPRINT(RT_DEBUG_ERROR, ("P2pReceInviteRspAction Success.  I am client. \n"));
            }
        }

        // Save the desired connecting mac address when received the invitation response.
        pPort->P2PCfg.SigmaQueryStatus.bInvitationSucced = TRUE;
        PlatformMoveMemory(pPort->P2PCfg.SigmaQueryStatus.InviteConnectingMAC, devAddr, 6);
    }
    else if (StatusCode == P2PSTATUS_NO_CHANNEL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pReceInviteRspAction ==> No common channel...Stop. \n"));
        pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_CHANNELDISMATCH;
        pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
    }
    else if(StatusCode == P2PSTATUS_INVALID_PARM)
    {
        P2pDisconnectOneConnection(pAd, pPort, MAC_ADDRESS_LENGTH, (VOID *)pFrame->p80211Header.Addr2);
        index = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
        if (index < MAX_P2P_GROUP_SIZE)
            pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY;
        pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_PEER_WITHOUT_PERSISTENT_TABLE;
            pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2P_ERRCODE_PEER_WITHOUT_PERSISTENT_TABLE  \n"));
    }
}


/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving provistion request Action Frame packet.
        the Element field contains a single WPS IE.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceProvisionReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort;
    USHORT      WpsConfigMethod, WpsConfigMethodForPassiveGo = 0;
    ULONG       FrameLen;
    UCHAR       p2pindex;
    UCHAR       DeviceName[32];
    UCHAR       GroupCap, DeviceCap, DeviceNameLen, DevAddr[MAC_ADDR_LEN], BssidAddr[MAC_ADDR_LEN],SsidLen, OpChannel;

    pPort = pAd->PortList[Elem->PortNum];

    DBGPRINT(RT_DEBUG_TRACE, ("P2pRece Provision ReqAction ==> My Dpid = %d \n", pPort->P2PCfg.Dpid));

    P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
        FALSE, NULL, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, 
        BssidAddr, &SsidLen, NULL, NULL, &WpsConfigMethod, NULL, NULL, NULL, NULL, &OpChannel, NULL, NULL, NULL, NULL);

    DBGPRINT(RT_DEBUG_TRACE, ("P2pRece Provision ReqAction ==>ConfigMethod = %x %s DeviceName = %s\n", WpsConfigMethod, decodeConfigMethod(WpsConfigMethod), DeviceName));

    // Patch for other vendor that might send multiple config... IN spec, the config method in Provision
    // Discovery req should carry config method that indicate SINGLE method.
    if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        if (WpsConfigMethod == CONFIG_METHOD_PUSHBUTTON
            && (pPort->P2PCfg.Dpid == DEV_PASS_ID_PBC))
        {
            // Correct. Keep going. 
        }
        else if (WpsConfigMethod == CONFIG_METHOD_DISPLAY
            && (pPort->P2PCfg.Dpid == DEV_PASS_ID_REG))
        {
            // Correct. Keep going. 
        }
        else if (WpsConfigMethod == CONFIG_METHOD_KEYPAD
            && (pPort->P2PCfg.Dpid == DEV_PASS_ID_USER))
        {
            // Correct. Keep going. 
        }
        else if (WpsConfigMethod == CONFIG_METHOD_LABEL
            && (pPort->P2PCfg.Dpid == DEV_PASS_ID_PIN))
        {
            // Correct. Keep going. 
        }
        else if (WpsConfigMethod == CONFIG_METHOD_PUSHBUTTON
            ||WpsConfigMethod == CONFIG_METHOD_KEYPAD
            ||WpsConfigMethod == CONFIG_METHOD_DISPLAY)
        {
            // Update my config method. and later I will let RaUI to pop up WPS setting page
            WpsConfigMethodForPassiveGo = WpsConfigMethod;
            WpsConfigMethod = 0;
        }
    }
    // I only support those 4 method.
    if ((WpsConfigMethod == CONFIG_METHOD_LABEL) 
        || (WpsConfigMethod == CONFIG_METHOD_DISPLAY)
        || (WpsConfigMethod == CONFIG_METHOD_PUSHBUTTON) 
        || (WpsConfigMethod == CONFIG_METHOD_KEYPAD)
        || (WpsConfigMethod == 0x18c))
    {
        p2pindex = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);

        DBGPRINT(RT_DEBUG_TRACE, ("I receive provision Req and I have Dpid = 0x%x", pPort->P2PCfg.Dpid));
        
        if (p2pindex >= MAX_P2P_GROUP_SIZE)
        {
            p2pindex = P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_DISCOVERY, NULL, 0);
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceName, DeviceName, DeviceNameLen);
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceNameLen = DeviceNameLen;
            if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE))
                pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
        }
        if (p2pindex < MAX_P2P_GROUP_SIZE)
        {
            // Set Event to GUI to display PIN digit.
            if ((WpsConfigMethod == CONFIG_METHOD_LABEL)||(WpsConfigMethod == CONFIG_METHOD_DISPLAY))
            {
                
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.DisplayPin.MacAddr, pFrame->p80211Header.Addr2, 6);
                if ((pPort->P2PCfg.P2pEvent != NULL) && (IS_P2P_SIGMA_OFF(pPort))
                    && (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
                    && (pPort->P2PCfg.Dpid == DEV_PASS_ID_NOSPEC))
                {
                    pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_DISPLAY_PIN;
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent 13. Display Pin \n"));         
                    if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_DISPLAY_PIN))
                        pPort->P2PCfg.P2pEventQueue.bShowPinCode= TRUE;
                    
                }
                else if ((pPort->P2PCfg.P2pEvent != NULL) && (IS_P2P_SIGMA_OFF(pPort)))
                {
                    pPort->P2PCfg.PopUpIndex = p2pindex;
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_DISPLAY;
                    if ((WpsConfigMethod == CONFIG_METHOD_DISPLAY) || (WpsConfigMethod == CONFIG_METHOD_LABEL))
                    {
                        pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_KEYPAD;
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Need setting first. Pop up setting windows. %d\n", pPort->P2PCfg.PopUpIndex));
                    // peer enter pin ask to me display, provision or nego first receive first pop
                    if( (pPort->P2PCfg.Dpid == DEV_PASS_ID_NOSPEC)
                        && (pPort->P2PCfg.P2pEventQueue.LastSetEvent != P2PEVENT_POPUP_SETTING_WINDOWS))
                    {
                        if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS))
                            pPort->P2PCfg.P2pEventQueue.bPopWindow= TRUE;
                    }
                }
            }
            pPort->P2PCfg.ConfigMethod = WpsConfigMethod;
            DBGPRINT(RT_DEBUG_TRACE, ("Update My Config Method to  %s \n", decodeConfigMethod(pPort->P2PCfg.ConfigMethod )));         
            if (WpsConfigMethod == CONFIG_METHOD_KEYPAD)
            {
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_DISPLAY;
                if(pPort->PortSubtype == PORTSUBTYPE_P2PGO)
                {
                    //pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_DISPLAY;
                    pPort->P2PCfg.PopUpIndex = p2pindex;
                    if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS))
                            pPort->P2PCfg.P2pEventQueue.bPopWindow= TRUE;
                    
                    DBGPRINT(RT_DEBUG_TRACE, ("ConfigMethod = %d PopUpIndex = %d pinLen = %d PinCode = %x %x %x %x %x %x %x %x \n", pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod, pPort->P2PCfg.PopUpIndex, pPort->P2PCfg.PINLen, pPort->P2PCfg.PinCode[0], 
                        pPort->P2PCfg.PinCode[1], pPort->P2PCfg.PinCode[2], pPort->P2PCfg.PinCode[3], pPort->P2PCfg.PinCode[4], pPort->P2PCfg.PinCode[5], pPort->P2PCfg.PinCode[6], pPort->P2PCfg.PinCode[7]));
                }   
                //pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_LABEL;
                // I need user specified. So set to DEV_PASS_ID_NOSPEC. 
                // And wait for manual GUI setting.
                // And wait for manual GUI setting.
            }
            else if (WpsConfigMethod == CONFIG_METHOD_PUSHBUTTON)
            {
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
                //pPort->P2PCfg.Dpid = DEV_PASS_ID_PBC;
                if((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pPort->P2PCfg.Dpid == DEV_PASS_ID_NOSPEC))
                {
                    pPort->P2PCfg.PopUpIndex = p2pindex;
                    
                    DBGPRINT(RT_DEBUG_TRACE, ("Need setting first 2. Pop up setting windows. Index = %d, ConfigMethod = %d\n", pPort->P2PCfg.PopUpIndex, pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod));
                    if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO))
                        pPort->P2PCfg.P2pEventQueue.bPopWindowForGO= TRUE;
                }
            }
            else
            {
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_KEYPAD;             

                if (IS_P2P_CONNECT_IDLE(pPort))
                {
                    //Because I am not connected. So Maybe will receive Go Nego Req. So need to set some necessary parameters first.
                    PlatformMoveMemory(pPort->P2PCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
                    pPort->P2PCfg.SSID[WILDP2PSSIDLEN] = 'b';
                    pPort->P2PCfg.SSID[WILDP2PSSIDLEN+1] = 'b';
                    pPort->P2PCfg.SSIDLen = WILDP2PSSIDLEN + 2;

                    // Prepare WPA2PSK AES IE.
                    RTMPAPMakeRSNIE(pAd, pPort, Ralink802_11AuthModeWPA2PSK, Ralink802_11Encryption3Enabled);
                }
            }

            // Update Sigma.ConfigMethod after finished provision procedure.
            pPort->P2PCfg.SigmaQueryStatus.ConfigMethod =  pPort->P2PCfg.ConfigMethod;

            P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_PROVISIONED);
            // This peer is provistioned, So need to set P2pflag. So tell GUI. so Set topology update event.
            pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;            
            
            P2PSendProvisionRsp(pAd, WpsConfigMethod, pFrame->Token, pFrame->p80211Header.Addr2, &FrameLen);
            DBGPRINT(RT_DEBUG_TRACE, ("Accept Provision Req ==>ConfigMethod = %s \n", decodeConfigMethod(WpsConfigMethod)));
        }
    }
    else
    {
        P2PSendProvisionRsp(pAd, WpsConfigMethodForPassiveGo, pFrame->Token, pFrame->p80211Header.Addr2, &FrameLen);

        p2pindex = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
        if (p2pindex >= MAX_P2P_GROUP_SIZE)
        {
            p2pindex = P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_DISCOVERY, NULL, 0);
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceName, DeviceName, DeviceNameLen);
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceNameLen = DeviceNameLen;
            if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE))
                pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
        }
        if (p2pindex < MAX_P2P_GROUP_SIZE)
        {
            
            if ((pPort->P2PCfg.P2pEvent != NULL) && (IS_P2P_SIGMA_OFF(pPort)) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            {
                if ((WpsConfigMethodForPassiveGo == CONFIG_METHOD_LABEL) ||(WpsConfigMethodForPassiveGo == CONFIG_METHOD_DISPLAY))
                {
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_KEYPAD;
                }
                else if(WpsConfigMethodForPassiveGo == CONFIG_METHOD_KEYPAD)
                {
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_DISPLAY;
                }
                else if(WpsConfigMethodForPassiveGo == CONFIG_METHOD_PUSHBUTTON)
                {
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
                }

                pPort->P2PCfg.PopUpIndex = p2pindex;

                DBGPRINT(RT_DEBUG_TRACE, ("Need setting first. Pop up setting windows. Index = %d, ConfigMethod = %d\n", pPort->P2PCfg.PopUpIndex, pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod));
                if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO))
                    pPort->P2PCfg.P2pEventQueue.bPopWindowForGO= TRUE;
            }
        }
    }
}

/*  
    ==========================================================================
    Description: 
        The routine that is called when receiving provistion request Action Frame packet.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceProvisionRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
    PMP_PORT  pPort;
    USHORT      ConfigMethod = 0;
    UCHAR       p2pindex;
    
    pPort = pAd->PortList[Elem->PortNum];

    p2pindex = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
    if (p2pindex >= MAX_P2P_GROUP_SIZE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pReceProvisionRspAction from unknown device ==> \n"));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("P2pReceProvisionRspAction from %d ==> \n", p2pindex));
    
    if (PlatformEqualMemory(&pFrame->OUI2[0], WPS_OUI, 4))
    {
        P2PParseWPSIE(&pFrame->ElementID, (pFrame->Length +2), NULL, &ConfigMethod, NULL, NULL, NULL, NULL, NULL);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("P2pRece Provision RspAction ==>ConfigMethod = %x. %s \n", ConfigMethod, decodeConfigMethod(ConfigMethod)));
    DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.Dpid = %d \n", pPort->P2PCfg.Dpid));
    // Set Event to GUI to display PIN digit.
    // But if peer is GO.. Don't display again.
    /*if ((ConfigMethod == CONFIG_METHOD_KEYPAD) && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule == P2P_IS_CLIENT))
    {
        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.DisplayPin.MacAddr, pFrame->p80211Header.Addr2, 6);
        if ((pPort->P2PCfg.P2pEvent != NULL) && (IS_P2P_SIGMA_OFF(pPort)))
        {
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_DISPLAY_PIN;
            DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent 13. Show PIN now\n"));         
            KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
            pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
        }
    }*/

    DBGPRINT(RT_DEBUG_TRACE, ("P2pClientState = %s PeerRule = %d Test(P2PFLAG_PROVISIONED) = %d", decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState), pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule,
    P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_PROVISIONED)));

    // p.s spec said null indicates failure. So check ConfigMethod to decide if should go on following actions.
    if ((ConfigMethod == CONFIG_METHOD_LABEL) 
        || (ConfigMethod == CONFIG_METHOD_DISPLAY)
        || (ConfigMethod == CONFIG_METHOD_KEYPAD)
        || (ConfigMethod == CONFIG_METHOD_PUSHBUTTON))
    {

        // Provistion request may be sent to AutoGO.
        if (pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState == P2PSTATE_SENT_PROVISION_REQ)
        {
            if (pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule == P2P_IS_GO)
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY_GO;
            else
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
        }
        pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = ConfigMethod;
        if ((ConfigMethod == CONFIG_METHOD_LABEL) 
                || (ConfigMethod == CONFIG_METHOD_DISPLAY))
        {
            // I use user specified. Need GUI set PIN. So set to NOSPEC now.
            // But if peer is GO. I will connect now. So don't delete previous Dpid setting.
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_KEYPAD;
        }
        else if (ConfigMethod == CONFIG_METHOD_KEYPAD)
        {
            // I use user specified. Need GUI set PIN. So set to NOSPEC now.
            // But if peer is GO. I will connect now. So don't delete previous Dpid setting.
            if (pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule != P2P_IS_GO)
                pPort->P2PCfg.Dpid = DEV_PASS_ID_PIN;
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_LABEL;
        }
        else if (ConfigMethod == CONFIG_METHOD_PUSHBUTTON)
        {
            // I use user specified. Need GUI set PIN. So set to NOSPEC now.
            // But if peer is GO. I will connect now. So don't delete previous Dpid setting.
            pPort->P2PCfg.Dpid = DEV_PASS_ID_PBC;
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
        }

        
        if(pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule == P2P_IS_GO)
        {
            // Peer use Label or Display. If peer is alreayd GO, I alreayd have WPS information to connect
            // So doesn't need to pop up a setting windows.
            //In I KeyPad case : if GO invite me , I just pop once
            // If I actiove to connect to GO need pop twice
            if (((ConfigMethod == CONFIG_METHOD_LABEL) || (ConfigMethod == CONFIG_METHOD_DISPLAY)) && (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_GO_INVITE_ME)))
            {
                pPort->P2PCfg.PopUpIndex = p2pindex;
                DBGPRINT(RT_DEBUG_TRACE, ("Need setting first. Pop up setting windows. %d\n", pPort->P2PCfg.PopUpIndex));
                P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS);
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Peer %d ConfigMethod = %s \n", p2pindex, decodeConfigMethod(ConfigMethod)));
            if ((pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE || pPort->P2PCfg.P2PConnectState == P2P_NEGOTIATION))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Decide to Join P2p group? when I am %s \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE || pPort->P2PCfg.P2PConnectState == P2P_NEGOTIATION)
                {
                    // If Bssid field is empty, copy it from src addr.
                    if (PlatformEqualMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid, ZeroSsid, MAC_ADDR_LEN))
                    {
                        // Addr2 is peer's device address, not GO interface address
                        //PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid, pFrame->p80211Header.Addr2, MAC_ADDR_LEN);
                    }
                    DBGPRINT(RT_DEBUG_TRACE,("P2p :connecting to GO with Bssid   %x.  %x. %x.  %x. %x.  %x. \n", pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid[0], pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid[1],pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid[2],pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid[3],pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid[4],pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid[5]));
                    DBGPRINT(RT_DEBUG_TRACE,("P2p : its GroupCapability= %x.  DevCapability= %x. \n", pAd->pP2pCtrll->P2PTable.Client[p2pindex].GroupCapability, pAd->pP2pCtrll->P2PTable.Client[p2pindex].DevCapability));
                    pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_GO_WPS;
                    PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Bssid, pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid, MAC_ADDR_LEN);
                    PlatformMoveMemory(pPort->P2PCfg.Bssid, pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid, MAC_ADDR_LEN);
                    PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, pAd->pP2pCtrll->P2PTable.Client[p2pindex].Ssid, 32);
                    pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen = pAd->pP2pCtrll->P2PTable.Client[p2pindex].SsidLen;
                    pPort->P2PCfg.GroupOpChannel = pAd->pP2pCtrll->P2PTable.Client[p2pindex].OpChannel;
                    P2pGoNegoDone(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], &pAd->pP2pCtrll->P2PTable.Client[p2pindex]);
                }
            }
            P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_PROVISIONED);
        }
        else if(pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule == P2P_IS_CLIENT)
        {
            if ((pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE  || pPort->P2PCfg.P2PConnectState == P2P_NEGOTIATION)
                && (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_PROVISIONED))
                &&(IS_P2P_SIGMA_OFF(pPort)))
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s receprovision response and send nego Req right now", __FUNCTION__));
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_CONNECT_COMMAND;
                PlatformZeroMemory(&pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], MAC_ADDR_LEN);
                P2pConnectAction(pAd, pPort, FALSE, p2pindex);
        }

                P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_PROVISIONED);
        }
        

    }
    else
    {
        // Don't need to update P2P Client State. we will retry later.
    }
//  else
//      P2pSetEvent(pAd, pPort,  P2PEVENT_STOP_PROGRESSBAR_NOW);
}

/*  
    ==========================================================================
    Description: 
        Call this function when receiving WPS EAP Nack frame. Most of time is because incorrect PIN.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pReceiveEapNack(
    IN PMP_ADAPTER pAd,
    IN  PMLME_QUEUE_ELEM    pElem)
{
    PHEADER_802_11  pHeader_802_11;
    UCHAR   addr[6];
    MAC_TABLE_ENTRY *pEntry = NULL;
    PMP_PORT  pPort = pAd->PortList[pElem->PortNum];

    if ((!IS_P2P_ENROLLEE(pPort) ) && (!IS_P2P_REGISTRA(pPort)))
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pReceiveEapNack : WscState = %d. %s \n", pPort->StaCfg.WscControl.WscState, decodeP2PState(pPort->P2PCfg.P2PConnectState)));           
    
    pHeader_802_11 = (PHEADER_802_11)&pElem->Msg[0];
    PlatformMoveMemory(addr, pHeader_802_11->Addr2, MAC_ADDR_LEN);

    if (IS_P2P_ENROLLEE(pPort))
    {
        MlmeCntLinkDown(pAd->PortList[pPort->P2PCfg.PortNumber], FALSE);
        pEntry = MacTableLookup(pAd,  pPort, addr);
        if (pEntry != NULL)
            MacTableDeleteAndResetEntry(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pEntry->Aid, pEntry->Addr, TRUE);

        P2pGroupTabDelete(pAd, 0xff, addr); 
        // P2P link down should disable autoreconnect.
        pAd->MlmeAux.AutoReconnectSsidLen= 32;
        pAd->MlmeAux.AutoReconnectStatus = FALSE;
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
        P2pGotoIdle(pAd, pPort);
        P2pConnectStateUpdate(pAd, pPort);
    }
    if (IS_P2P_REGISTRA(pPort))
    {
        pEntry = MacTableLookup(pAd,  pPort, addr);
        if (pEntry != NULL)
        {
            MLME_DEAUTH_REQ_STRUCT      DeAuthRequest;

            DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
            DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTab  Disconnect  ==>..%x \n", pPort->P2PCfg.Bssid[5]));
            COPY_MAC_ADDR(DeAuthRequest.Addr, pPort->P2PCfg.Bssid);
            MlmeAuthDeauthReqAction(pAd, pPort->P2PCfg.PortNumber, &DeAuthRequest);
            MacTableDeleteAndResetEntry(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pEntry->Aid, pEntry->Addr, TRUE);
        }

        P2pGroupTabDelete(pAd, 0xff, addr); 
        P2pConnectStateUpdate(pAd, pPort);
        if (IS_P2P_SIGMA_OFF(pPort))
            P2pRefreshTableThenScan(pAd, 0);
    }
    pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_WPSNACK;
    pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
    P2PDefaultConfigM(pAd, pPort);
}


/*  
    ==========================================================================
    Description: 
        Make P2P beacon frame and save to  BeaconBuf[]
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pMakeBssBeacon(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort) 
{
    UCHAR           DsLen = 1, TimLen = 4, IbssLen = 2, SsidLen, CountryLen;
    HEADER_802_11   BcnHdr;
    LARGE_INTEGER   FakeTimestamp;
    ULONG           FrameLen = 0, longptr;
    PTXWI_STRUC     pTxWI = &pAd->pTxCfg->BeaconTxWI;
    PUCHAR          pBeaconFrame = pAd->pTxCfg->BeaconBuf;
    UCHAR           *ptr;
    UINT            i;
    //HTTRANSMIT_SETTING    BeaconTransmit;   // MGMT frame PHY rate setting when operatin at Ht rate.
    PHY_CFG BeaconPhyCfg = {0};
    UCHAR       tmpSupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR   RSN_IE[22]={ 0x00,0x50,0xf2,0x01,0x01,0x00,0x00,0x50,0xf2,0x04,0x01,0x00,0x00,0x50,0xf2,0x04,0x01,0x00,0x00,0x50,0xf2,0x02};
    UCHAR   RSNIe=IE_WPA2, RSNIe2=IE_WPA2, RSN_Len=22, WscIe = 0xdd;
    UCHAR           P2pIEFixed[6] = {0xdd, 0x12, 0x50, 0x6f, 0x9a, 0x09};   // length will modify later
    ULONG TmpLen;
    UCHAR       ThisSsid[32];
    UCHAR       SupRateLen;
    ULONG   LastWscLen;
    UCHAR   wcid =0;

    // copy P2P Parameters to local pPort.
    // GO in Win8 uses port cfg
    if (pPort->PortType != WFD_GO_PORT)
    {
        pPort->PortCfg.SsidLen = pPort->P2PCfg.SSIDLen;
        PlatformMoveMemory(pPort->PortCfg.Ssid, pPort->P2PCfg.SSID, 32);
    }

    SsidLen = pPort->PortCfg.SsidLen;
    MgtMacHeaderInit(pAd, pPort, &BcnHdr, SUBTYPE_BEACON, 0, BROADCAST_ADDR, pPort->PortCfg.Bssid);

    RSNIe = IE_WPA2;

    {
        SupRateLen = 8;
        tmpSupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        tmpSupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        tmpSupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        tmpSupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        tmpSupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
    }

    pPort->SoftAP.ApCfg.CapabilityInfo = CAP_GENERATE(1, 0, 1, 0, 0);
    
#if UAPSD_AP_SUPPORT
    if (IS_AP_SUPPORT_UAPSD(pAd, pPort))
    {
        /* QAPs set the APSD subfield to 1 within the Capability Information
        field when the MIB attribute dot11APSDOptionImplemented is true
        and set it to 0 otherwise. STAs always set this subfield to 0. */
        pPort->SoftAP.ApCfg.CapabilityInfo |= 0x0800;
    }
#endif
        
    // for update framelen to TxWI later.
    MakeOutgoingFrame(pBeaconFrame,                  &FrameLen,
                    sizeof(HEADER_802_11),           &BcnHdr, 
                    TIMESTAMP_LEN,                   &FakeTimestamp,
                    2,                               &pPort->CommonCfg.BeaconPeriod,
                    2,                               &pPort->SoftAP.ApCfg.CapabilityInfo,
                    1,                               &SsidIe, 
                    1,                               &SsidLen, 
                    SsidLen,                         pPort->PortCfg.Ssid,
                    1,                               &SupRateIe, 
                    1,                               &SupRateLen,
                    SupRateLen,       tmpSupRate, 
                    END_OF_ARGS);

    if (!MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_COEXIST_OPT_BIT)
        && (pPort->P2PCfg.P2pManagedParm.CountryContent[0] == 0x55)
        && (pPort->P2PCfg.P2pManagedParm.CountryContent[1] == 0x53)
        && (pAd->OpMode == OPMODE_STAP2P))
    {
        UCHAR       CountryIe = IE_COUNTRY;
        UCHAR       CountryContent[6];
        CountryLen = 6;
        PlatformMoveMemory(&CountryContent[0], &pPort->P2PCfg.P2pManagedParm.CountryContent[0], 6);
        
        MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                    1,                               &CountryIe, 
                    1,                               &CountryLen, 
                    CountryLen,                               &CountryContent[0],
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    // add WMM IE here
    if (pPort->Channel <= 14)
    {

        MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                    1,                               &DsIe, 
                    1,                               &DsLen, 
                    1,                               &pPort->Channel,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
    // add WMM IE here
    //if ((pPort->CommonCfg.bWmmCapable) || (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED))
    {
        UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 
        WmeParmIe[8] = pPort->SoftAP.ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;
#if UAPSD_AP_SUPPORT
        if(IS_AP_SUPPORT_UAPSD(pAd, pPort))
        {
            WmeParmIe[8] |=0x80; //the b7 in QoS Info is U-ASPD support indication
            DBGPRINT(RT_DEBUG_INFO,("Chk::%s(%d)==>WmeParmIe[8](OoS Info field)=%02x\n", __FUNCTION__, __LINE__, WmeParmIe[8]));
        }
#endif      
        for (i=QID_AC_BE; i<=QID_AC_VO; i++)
        {
            WmeParmIe[10+ (i*4)] = (i << 5)                                         +     // b5-6 is ACI
                                   ((UCHAR)pPort->SoftAP.ApCfg.BssEdcaParm.bACM[i] << 4)     +     // b4 is ACM
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);              // b0-3 is AIFSN
            WmeParmIe[11+ (i*4)] = (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[i] << 4)           +     // b5-8 is CWMAX
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              // b0-3 is CWMIN
            WmeParmIe[12+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] & 0xff);        // low byte of TXOP
            WmeParmIe[13+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] >> 8);          // high byte of TXOP
        }

        MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                          26,                            WmeParmIe,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    // Append RSN_IE when  WPA OR WPAPSK, 
    {
        MakeOutgoingFrame(pBeaconFrame+FrameLen,        &TmpLen,
                          1,                            &RSNIe,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],      &pPort->SoftAP.ApCfg.RSN_IE[0],
                          END_OF_ARGS);
        FrameLen += TmpLen;

        DBGPRINT(RT_DEBUG_TRACE, ("%s:  Append RSN_IE when  Ralink802_11AuthModeWPA2PSK\n", __FUNCTION__));
    }

    /*pPort->P2PCfg.P2pBcnOffset.IpBitmapBcnOffset = FrameLen + 6;
    RaIpIe.ElementID = IE_VENDOR_SPECIFIC;
    RaIpIe.Length = 5;
    PlatformMoveMemory(RaIpIe.OUI, RALINK_OUI, 3);
    RaIpIe.OUIMode = RALINKOUIMODE_IPREQ;
    // Default request all four Aid (1~4) 's ip.
    RaIpIe.Octet[0] = 0x1e;
    {
        MakeOutgoingFrame(pBeaconFrame+FrameLen,        &TmpLen,
                          sizeof(RALINKIP_IE),                            &RaIpIe,
                          END_OF_ARGS);
        FrameLen += TmpLen;

        DBGPRINT(RT_DEBUG_INFO, ("%s:  Append RalinkIP_IE \n", __FUNCTION__));
    }
    */

    // Set TimIELocationInBeacon before P2P IE. 
    // Because following P2P IE want to put to GOUpateBeacon() function too.
    pPort->SoftAP.ApCfg.TimIELocationInBeacon = FrameLen; 

    // Append WSC IE
    //
    LastWscLen = pPort->P2PCfg.GOBeaconBufWscLen;

    if ((pPort->PortType == WFD_GO_PORT) && (pPort->PortCfg.AdditionalBeaconIESize > 0) && (pPort->PortCfg.AdditionalBeaconIEData != NULL))
    {
        MakeOutgoingFrame(pBeaconFrame + FrameLen,      &TmpLen,
                            pPort->PortCfg.AdditionalBeaconIESize,  pPort->PortCfg.AdditionalBeaconIEData,
                            END_OF_ARGS);
    }
    /*else if ((pPort->PortType == WFD_GO_PORT) && (IS_P2P_MS_DEV(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]))
        && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIESize > 0) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIEData != NULL))
    {
        MakeOutgoingFrame(pBeaconFrame + FrameLen,      &TmpLen,
                            pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIESize,  pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIEData,
                            END_OF_ARGS);
    }*/
    else
    {
        P2pMakeBeaconWSCIE(pAd, pPort, pBeaconFrame + FrameLen, &TmpLen);
    }
    pPort->P2PCfg.GOBeaconBufWscLen = TmpLen; 

    if (LastWscLen != pPort->P2PCfg.GOBeaconBufWscLen)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE,  (" WSCIE Length Changing = %d ", TmpLen));
        DumpFrameMessage(&pAd->pTxCfg->BeaconBuf[FrameLen], TmpLen, ("WSCIE in beacon\n"));
    }
    // save wsc ie length
    FrameLen += TmpLen;

    // ========================================>
    // Save P2P IE Lenght offset.
    PlatformMoveMemory(pBeaconFrame + FrameLen, &P2pIEFixed[0], 6);
    FrameLen += 6;
    pPort->P2PCfg.P2pBcnOffset.P2pCapOffset = FrameLen + 3;
    TmpLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, (pPort->PortType == WFD_GO_PORT) ? (pPort->PortCfg.P2pCapability) : (pPort->P2PCfg.P2pCapability), pBeaconFrame + FrameLen);
    FrameLen += TmpLen;
    // Device ID in device port is mandatory in beacon
    TmpLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_ID, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pBeaconFrame + FrameLen);
    FrameLen += TmpLen;

    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &BeaconPhyCfg, 0);
    WRITE_PHY_CFG_MODE(pAd, &BeaconPhyCfg, MODE_OFDM);

    wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);
    XmitWriteTxWI(pAd, pPort, pTxWI, FALSE, TRUE, FALSE, FALSE, TRUE, 0, wcid,
    FrameLen, PID_MGMT, 0, IFS_HTTXOP, FALSE, BeaconPhyCfg, FALSE, TRUE, FALSE);


    //
    // step 6. move BEACON TXD and frame content to on-chip memory
    //
    ptr = (PUCHAR)&pAd->pTxCfg->BeaconTxWI;
    for (i=0; i<pAd->HwCfg.TXWI_Length; )  // 24-byte TXINFO field
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + i, longptr);   //Beacon buffer of AP is always in 0x7900.
        ptr += 4;
        i+=4;
    }

    // update BEACON frame content. start right after the 24-byte TXINFO field
    ptr = pAd->pTxCfg->BeaconBuf;
    for (i=0; i< FrameLen;)
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + pAd->HwCfg.TXWI_Length + i, longptr);
        ptr += 4;
        i+=4;
    }

    pAd->pTxCfg->BeaconBufLen = FrameLen;
    pPort->SoftAP.ApCfg.CapabilityInfoLocationInBeacon = sizeof(HEADER_802_11) + TIMESTAMP_LEN + 2;

//==============================================>
    if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortNumber == PORT_0))// GO on Port0
    {
        // Make fake beacon for indicating connection in GO mode.
        pPort->SoftAP.ApCfg.AdhocBssid[0] = 0xF0;
        pPort->SoftAP.ApCfg.AdhocBssid[1] = 0x50;
        pPort->SoftAP.ApCfg.AdhocBssid[2] = 0x40;
        pPort->SoftAP.ApCfg.AdhocBssid[3] = 0x30;
        pPort->SoftAP.ApCfg.AdhocBssid[4] = 0x20;
        pPort->SoftAP.ApCfg.AdhocBssid[5] = 0x10;
        MgtMacHeaderInit(pAd, pPort, &BcnHdr, SUBTYPE_BEACON, 0, BROADCAST_ADDR, pPort->SoftAP.ApCfg.AdhocBssid);
        // In the A band should not appear 1,2,5,11
        {
            tmpSupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
            tmpSupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
            tmpSupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
            tmpSupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
            tmpSupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
            tmpSupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
            tmpSupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
            tmpSupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
        }

        pPort->SoftAP.ApCfg.CapabilityInfo = CAP_GENERATE(0, 1, 0, 0, 0);

        pBeaconFrame = pAd->pP2pCtrll->GOBeaconBuf;

        SsidLen = pPort->P2PCfg.SSIDLen;
        PlatformMoveMemory(ThisSsid, pPort->P2PCfg.SSID, 32);

        // for update framelen to TxWI later.
        MakeOutgoingFrame(pBeaconFrame,                  &FrameLen,
                        sizeof(HEADER_802_11),           &BcnHdr,
                        TIMESTAMP_LEN,                   &FakeTimestamp,
                        2,                               &pPort->CommonCfg.BeaconPeriod,
                        2,                               &pPort->SoftAP.ApCfg.CapabilityInfo,
                        1,                               &SsidIe,
                        1,                               &SsidLen,
                        SsidLen,                         ThisSsid,
                        1,                               &SupRateIe,
                        1,                               &pPort->CommonCfg.SupRateLen,
                        pPort->CommonCfg.SupRateLen,       tmpSupRate,
                        1,                               &DsIe,
                        1,                               &DsLen,
                        1,                               &pPort->P2PCfg.GroupOpChannel,
                          1,                               &IbssIe,
                          1,                               &IbssLen,
                          2,                               &pAd->StaActive.AtimWin,
                        END_OF_ARGS);

        pPort->SoftAP.ApCfg.CapabilityInfo = CAP_GENERATE(1, 0, 1, 0, 0);

#if UAPSD_AP_SUPPORT
        if (IS_AP_SUPPORT_UAPSD(pAd, pPort))
        {
            /* QAPs set the APSD subfield to 1 within the Capability Information
            field when the MIB attribute dot11APSDOptionImplemented is true
            and set it to 0 otherwise. STAs always set this subfield to 0. */
            pPort->SoftAP.ApCfg.CapabilityInfo |= 0x0800;
        }
#endif

        // add WMM IE here
        if ((pPort->CommonCfg.bWmmCapable) || (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 
            WmeParmIe[8] = pPort->SoftAP.ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;
#if UAPSD_AP_SUPPORT
            if(IS_AP_SUPPORT_UAPSD(pAd, pPort))
            {
                WmeParmIe[8] |=0x80; //the b7 in QoS Info is U-ASPD support indication
                DBGPRINT(RT_DEBUG_INFO,("Chk::%s(%d)==>WmeParmIe[8](OoS Info field)=%02x\n", __FUNCTION__, __LINE__, WmeParmIe[8]));
            }
#endif
            for (i=QID_AC_BE; i<=QID_AC_VO; i++)
            {
                WmeParmIe[10+ (i*4)] = (i << 5)                                         +     // b5-6 is ACI
                                       ((UCHAR)pPort->SoftAP.ApCfg.BssEdcaParm.bACM[i] << 4)     +     // b4 is ACM
                                       (pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);              // b0-3 is AIFSN
                WmeParmIe[11+ (i*4)] = (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[i] << 4)           +     // b5-8 is CWMAX
                                       (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              // b0-3 is CWMIN
                WmeParmIe[12+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] & 0xff);        // low byte of TXOP
                WmeParmIe[13+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] >> 8);          // high byte of TXOP
            }

            MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                              26,                            WmeParmIe,
                              END_OF_ARGS);
            FrameLen += TmpLen;
        }

        pAd->pP2pCtrll->GOBeaconBufLen = FrameLen;
    }

}


/*  
    ==========================================================================
    Description: 
        Prepare Probe reqeust or response frame when opeartin as P2P DEvice.
        
    Parameters: 
        pDest : buffer to put frame content.
        pFrameLen : frame length.
    Note:
         
    ==========================================================================
 */
VOID P2PMakeProbe(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN MLME_QUEUE_ELEM *Elem, 
    IN MP_PORT_SUBTYPE      PortSubtype,
    IN UCHAR        DsChannel,
    IN USHORT   SubType,
    OUT PUCHAR pDest,
    OUT ULONG *pFrameLen) 
{
    PUCHAR      pOutBuffer = pDest;
    UCHAR       Addr2[6];
    PHEADER_802_11  pHeader;
    HEADER_802_11 ProbeRspHdr;
    UCHAR   RSNIe=IE_WPA2, RSNIe2=IE_WPA2, RSN_Len=22;
    ULONG       beaconInterval;
    UCHAR       tmpSupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       tmpExtRateLen;
    LARGE_INTEGER FakeTimestamp;
    UCHAR       DsLen = 1, SsidLen = 0;
    ULONG       TmpLen;
    ULONG       FrameLen = 0;
    UCHAR       ErpIeLen = 1;
    UCHAR           P2pIEFixed[6] = {0xdd, 0x08, 0x50, 0x6f, 0x9a, 0x09};   // length will modify later
    USHORT      CapabilityInfo;
    UCHAR       SupRateLen;
    PUCHAR  ptr;
    UCHAR   Ssid[MAX_LEN_OF_SSID];

    pHeader = (PHEADER_802_11) &Elem->Msg[0];
    PlatformMoveMemory(Addr2, pHeader->Addr2, 6);
    PlatformMoveMemory(&P2pIEFixed[2], P2POUIBYTE, 4);

    DBGPRINT(RT_DEBUG_TRACE, ("%s   port %d", __FUNCTION__, pPort->PortNumber));

    if (SubType== SUBTYPE_PROBE_RSP)
    {
        DBGPRINT(RT_DEBUG_INFO, ("SYNC - P2PMakeProbeRsp. Addr2 = %02x:%02x:%02x:%02x:%02x:%02x...\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5] ));
        MgtMacHeaderInit(pAd, pPort, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, Addr2, pPort->CurrentAddress);
    }
    else
        MgtMacHeaderInit(pAd, pPort, &ProbeRspHdr, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);

    PlatformMoveMemory(tmpSupRate,pPort->CommonCfg.SupRate,pPort->CommonCfg.SupRateLen);
    tmpExtRateLen = pPort->CommonCfg.ExtRateLen;

    // P2P device's probe response need to set both ess and ibss bit to zero.
    CapabilityInfo = CAP_GENERATE(0, 0, 1, 0, 0);
    if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
        CapabilityInfo = CAP_GENERATE(1, 0, 1, 0, 0);
        

#if UAPSD_AP_SUPPORT
    if (IS_AP_SUPPORT_UAPSD(pAd, pPort))
    {
        /* QAPs set the APSD subfield to 1 within the Capability Information
        field when the MIB attribute dot11APSDOptionImplemented is true
        and set it to 0 otherwise. STAs always set this subfield to 0. */
        CapabilityInfo |= 0x0800;
    }
#endif
                
    tmpSupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
    tmpSupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
    tmpSupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
    tmpSupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
    tmpSupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
    tmpSupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
    tmpSupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
    tmpSupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
    SupRateLen  = 8;
    tmpExtRateLen = pPort->CommonCfg.ExtRateLen;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP)
        || MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)
        || INFRA_ON(pAd->pP2pCtrll->pInfraPort)))
        DsChannel = pAd->pP2pCtrll->pInfraPort->Channel;
    else
#endif      
    if (NDIS_WIN7(pAd) && (IS_P2P_OP(pPort) || INFRA_ON(pAd->pP2pCtrll->pInfraPort)))
        DsChannel = pPort->Channel;
    else if ((pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE)
        ||(pPort->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY)
        || (pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH_COMPLETE)
        || (pPort->P2PCfg.P2PDiscoProvState == P2P_LISTEN))
        DsChannel = pPort->P2PCfg.ListenChannel;
    else if (IS_P2P_SEARCHING(pPort))
        DsChannel = pPort->ScaningChannel;
    else
        DsChannel = pAd->HwCfg.LatchRfRegs.Channel;

    beaconInterval = (pPort->CommonCfg.BeaconPeriod == 0) ? 100 : pPort->CommonCfg.BeaconPeriod;

    //DsChannel = pAd->HwCfg.LatchRfRegs.Channel;
    if (DsChannel == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("SYNC - P2PMakeProbeRsp. DsChannel = is 0 !!!!!ComChannel = %d.  mlmeaux cha= %d. %s\n",pPort->Channel, pPort->ScaningChannel, decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
        DsChannel = pPort->Channel;
    }
    if (SubType== SUBTYPE_PROBE_RSP)
    {
        SsidLen = WILDP2PSSIDLEN;
        PlatformZeroMemory(Ssid, MAX_LEN_OF_SSID);
        PlatformMoveMemory(Ssid, &WILDP2PSSID[0], WILDP2PSSIDLEN);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if (NDIS_WIN8_ABOVE(pAd) && pPort->PortType == WFD_GO_PORT)
        {
            SsidLen = pPort->PortCfg.SsidLen;
            PlatformMoveMemory(Ssid, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
        }   
        else
#endif
        {
            if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                SsidLen = pPort->P2PCfg.SSIDLen;
                PlatformMoveMemory(Ssid, pPort->P2PCfg.SSID, pPort->P2PCfg.SSIDLen);
            }
        }
        MakeOutgoingFrame(pOutBuffer,                   &FrameLen,
                          sizeof(HEADER_802_11),        &ProbeRspHdr,
                          TIMESTAMP_LEN,                &FakeTimestamp,
                          2,                            &beaconInterval,
                          2,                            &CapabilityInfo,
                          1,                            &SsidIe,
                          1,                            &SsidLen,
                          SsidLen,                      Ssid,
                          1,                            &SupRateIe,
                          1,                            &SupRateLen,
                          SupRateLen,                   tmpSupRate,
                          END_OF_ARGS);
    }
    else
    {
        SsidLen = WILDP2PSSIDLEN;
        MakeOutgoingFrame(pOutBuffer,                   &FrameLen,
                          sizeof(HEADER_802_11),        &ProbeRspHdr,
                          1,                            &SsidIe,
                          1,                            &SsidLen,
                          SsidLen,                      &WILDP2PSSID[0],
                          1,                            &SupRateIe,
                          1,                            &SupRateLen,
                          SupRateLen,                   tmpSupRate,
                          END_OF_ARGS);

    }
    
    // Test Plan doesn't ask Probe request to carry DS IE.
    if (SubType != SUBTYPE_PROBE_REQ)
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen, 
                          1,                        &DsIe,
                          1,                        &DsLen,
                          1,                        &DsChannel,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
    
    // Add this IE after I already become GO.
/*  if (tmpExtRateLen && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen, 
                          1,                        &ErpIe,
                          1,                        &ErpIeLen,
                          1,                        &pPort->SoftAP.ApCfg.ErpIeContent,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }*/

    // Msut append RSN_IE because P2P uses WPA2PSK. 
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen, 
                          1,                        &RSNIe,
                          1,                        &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],  pPort->SoftAP.ApCfg.RSN_IE[0],
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

/*          

        // add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back
        if (pPort->CommonCfg.bAggregationCapable)
        {
            if ((pPort->CommonCfg.bPiggyBackCapable))
            {
                UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x07, 0x00, 0x00, 0x00};
                MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                                  9,                         RalinkSpecificIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
            else
            {
                UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x05, 0x00, 0x00, 0x00}; 
                MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                                  9,                         RalinkSpecificIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
        else
        {
            UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00}; 
            MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                              9,                         RalinkSpecificIe,
                              END_OF_ARGS);
            FrameLen += TmpLen;
        }

        if ((pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY))
        {           
            UCHAR   HtLen, AddHtLen, NewExtLen;
            ADD_HT_INFO_IE      AddHTInfo;  // Useful as AP.

            HT_CAPABILITY_IE HtCapability;
            PlatformMoveMemory(&HtCapability, &pPort->SoftAP.ApCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            if (pAd->HwCfg.Antenna.field.RxPath > 1)
            {
                HtCapability.MCSSet[1] = 0xff;
            }
            else
            {
                HtCapability.MCSSet[1] = 0x00;
            }

            HtLen = sizeof(pPort->SoftAP.ApCfg.HtCapability);
            AddHtLen = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe);
            NewExtLen = 1;

            PlatformMoveMemory(&AddHTInfo, &pPort->SoftAP.ApCfg.AddHTInfoIe, sizeof(ADD_HT_INFO_IE));

            //New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame
            MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                              1,                                &HtCapIe,
                              1,                                &HtLen,
                             sizeof(HT_CAPABILITY_IE),          &HtCapability, 
                              1,                                &AddHtInfoIe,
                              1,                                &AddHtLen,
                             sizeof(ADD_HT_INFO_IE),            &AddHTInfo, 
                              1,                                &NewExtChanIe,
                              1,                                &NewExtLen,
                             sizeof(NEW_EXT_CHAN_IE),           &pPort->CommonCfg.NewExtChanOffset, 
                              END_OF_ARGS);
            FrameLen += TmpLen;
        }
    */  
    //New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame

    // Append WSC information
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && (SubType == SUBTYPE_PROBE_RSP) && (pPort->PortCfg.AdditionalResponseIESize > 0) && (pPort->PortCfg.AdditionalResponseIEData != NULL))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s - Use Microsoft IE (AdditionalResponseIESize = %d)\n", __FUNCTION__, pPort->PortCfg.AdditionalResponseIESize));

        MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
                          pPort->PortCfg.AdditionalResponseIESize,  pPort->PortCfg.AdditionalResponseIEData,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
    else if (NDIS_WIN8_ABOVE(pAd) && (SubType == SUBTYPE_PROBE_REQ) && (pPort->P2PCfg.IsValidProprietaryIE == TRUE) && (pPort->P2PCfg.WFDProprietaryIEsLen > 0))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s - Use Microsoft IE (WFDProprietaryIEsLen = %d)\n", __FUNCTION__, pPort->P2PCfg.WFDProprietaryIEsLen));

        MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
                          pPort->P2PCfg.WFDProprietaryIEsLen, pPort->P2PCfg.WFDProprietaryIEs,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
    else if (NDIS_WIN8_ABOVE(pAd) && (SubType == SUBTYPE_PROBE_REQ) && (pPort->P2PCfg.WFDProprietaryIEsLen == 0) && (pPort->PortCfg.P2PAdditionalRequestIESize > 0) && (pPort->PortCfg.P2PAdditionalRequestIEData != NULL))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s - Use Microsoft IE (P2PAdditionalRequestIESize = %d)\n", __FUNCTION__, pPort->PortCfg.P2PAdditionalRequestIESize));

        MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
                          pPort->PortCfg.P2PAdditionalRequestIESize,    pPort->PortCfg.P2PAdditionalRequestIEData,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
    else
#endif
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s - Use Ralink IE\n", __FUNCTION__));
        P2pMakeProbeRspWSCIE(pAd, pPort, SubType, pOutBuffer + FrameLen, &TmpLen);
        FrameLen += TmpLen;
    }

    // P2P IE
    ptr = pOutBuffer + FrameLen;
    P2pMakeP2pIE(pAd, pPort, PortSubtype, (UCHAR)SubType, ptr, &TmpLen);
    FrameLen += TmpLen;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // [Win8] Put Additional IEs to the end because we don't know what's the data structure.
    if (NDIS_WIN8_ABOVE(pAd) && (SubType == SUBTYPE_PROBE_REQ) && (pPort->P2PCfg.IsValidProprietaryIE == FALSE) && (pPort->P2PCfg.WFDProprietaryIEsLen > 0))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s - Use Microsoft IE (WFDProprietaryIEsLen = %d)\n", __FUNCTION__, pPort->P2PCfg.WFDProprietaryIEsLen));

        MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
                          pPort->P2PCfg.WFDProprietaryIEsLen, pPort->P2PCfg.WFDProprietaryIEs,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
#endif
    
    *pFrameLen = FrameLen;

}



/*  
    ==========================================================================
    Description: 
        The routine make Invitiation Request Action Frame Packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PMakeInviteReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR        MyRule,
    IN UCHAR        InviteFlag,
    IN PUCHAR       Addr1,
    IN PUCHAR       Bssid,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    //PMP_PORT      pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    ULONG           P2pIeLen = 0;
    PUCHAR          pDest;
    ULONG           Length;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    UCHAR           ConfigTimeout[2];

    *pTotalFrameLen = 0;
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    // Bssid = DA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, Addr1);
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PMakeInviteReq  TO %x %x %x %x %x %x. \n", Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5]));
    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_INVITE_REQ;

    pPort->P2PCfg.Token++;
    pFrame->Token = pPort->P2PCfg.Token;
    
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = 38;
    pDest = pFrame->Octet;
    // attach subelementID= 5.
    ConfigTimeout[0] = pPort->P2PCfg.ConfigTimeout[0];
    ConfigTimeout[1] = pPort->P2PCfg.ConfigTimeout[1];
    // I am AutoGO. No need config time. So set zero.
    if ((IS_P2P_AUTOGO(pPort)) || (IS_P2P_GO_OP(pPort)))
        ConfigTimeout[0] = 0;
    
    if (IS_P2P_CLIENT_OP(pPort))
        ConfigTimeout[1] = 0;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 11.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_INVITE_FLAG, &InviteFlag, pDest);
    pPort->P2PCfg.LastSentInviteFlag = InviteFlag;
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 17.
    if (IS_P2P_CLIENT_OP(pPort))
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, &pPort->Channel, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PMakeInviteReq  pPort->Channel = %d. \n", pPort->Channel));
    }
    else if ((MyRule == P2P_IS_GO))
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, &pPort->P2PCfg.GroupOpChannel, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PMakeInviteReq  pPort->P2PCfg.GroupOpChannel = %d. \n", pPort->P2PCfg.GroupOpChannel));
    }

    // attach subelementID= 7.
    if ((MyRule == P2P_IS_CLIENT) && (InviteFlag == P2P_INVITE_FLAG_REINVOKE))
    {
        // Reinvoked as Client shall not attach Group Bssid 
    }
    else if (IS_P2P_CLIENT_OP(pPort))
    {
        // attach subelementID= 7.
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_BSSID, pPort->P2PCfg.Bssid, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }
    else
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_BSSID, &pPort->CurrentAddress[0], pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }
    // attach subelementID= 11.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, &pPort->P2PCfg.GroupOpChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 11.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pPort->P2PCfg.GroupDevAddr, pDest);
            
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 11.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pPort->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;


    pFrame->Length = (UCHAR)P2pIeLen;
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, *pTotalFrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
}


/*  
    ==========================================================================
    Description: 
        The routine make Invitiation Response Action Frame Packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PMakeInviteRsp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR        MyRule,
    IN UCHAR        Token,
    IN PUCHAR       Addr1,
    IN PUCHAR       Bssid,
    IN PUCHAR       OpChannel,
    IN PUCHAR       Status,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    //PMP_PORT  pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    ULONG   P2pIeLen = 0;
    PUCHAR          pDest;
    ULONG   Length;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS   NStatus;

    *pTotalFrameLen = 0;
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    // Bssid = SA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, pPort->CurrentAddress);
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PMakeInviteRsp MyRule = %d. TO %x %x %x %x %x %x. \n", MyRule, Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("P2P - Bssid %x %x %x %x %x %x. \n", Bssid[0], Bssid[1], Bssid[2],Bssid[3],Bssid[4],Bssid[5]));
    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_INVITE_RSP;
    pFrame->Token = Token;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = 38;
    pDest = pFrame->Octet;
    // attach subelementID= 0.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, Status, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 5.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &pPort->P2PCfg.ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 17.
    if ((MyRule == P2P_IS_GO) && (*Status == P2PSTATUS_SUCCESS))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - OpChannel =  %d \n", pPort->P2PCfg.GroupChannel));
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, OpChannel, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }
    // attach subelementID= 7.
    if ((MyRule == P2P_IS_GO) && (*Status == P2PSTATUS_SUCCESS))
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_BSSID, Bssid, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }
    // attach subelementID= 7.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, OpChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    // attach subelementID= 17.
    *pTotalFrameLen += Length;
    pFrame->Length = (UCHAR)P2pIeLen;

    NdisCommonMiniportMMRequest(pAd, pOutBuffer, *pTotalFrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
}

/*  
    ==========================================================================
    Description: 
        The routine prepares Go Negociation Confirm packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PMakeGoNegoConfirm(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR       Addr1,
    IN UCHAR            Token,
    IN PUCHAR       pOutBuffer,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;

    // Bssid = DA
    ActHeaderInit(pAd, pPort, (PHEADER_802_11)pOutBuffer, Addr1, Addr1);

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = GO_NEGOCIATION_CONFIRM;
    pFrame->Token = Token;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);

    *pTotalFrameLen = 38;
        
}


/*  
    ==========================================================================
    Description: 
        The routine make Go Negociation Response packet .
        
    Parameters:
        TempIntent is directly from Go Req. Still need to parse this.
    Note:
         
    ==========================================================================
 */
VOID P2PMakeGoNegoRsp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR       Addr1,
    IN USHORT           SentDpid,
    IN UCHAR            Token,
    IN UCHAR            TempIntent,
    IN UCHAR            Channel,
    IN UCHAR            Status,
    IN PUCHAR       pOutBuffer,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    PUCHAR          pDest;
    ULONG   Length;
    ULONG   P2pIeLen = 0;
    UCHAR   RealIntent;
    UCHAR           WscIEFixed[] = {0xdd, 0x0f, 0x00, 0x50, 0xf2, 0x04};    // length will modify later
    PUCHAR  pWscIeLen;
    UCHAR   P2pCapability[2];

    // Bssid = SA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, pPort->CurrentAddress);

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = GO_NEGOCIATION_RSP;
    pFrame->Token = Token;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = sizeof(P2P_PUBLIC_FRAME) - 1;
    pDest = pFrame->Octet;
    
    // Start attach subelement.
    // attach subelementID= 0.
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Status %d \n", Status));
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, &Status, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    P2pCapability[0] = pPort->P2PCfg.P2pCapability[0];
    P2pCapability[1] = pPort->P2PCfg.P2pCapability[1];
    if ((Status == 0) && (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO))
    {
        P2pCapability[1] |= GRPCAP_OWNER;
        P2pCapability[1] |= GRPCAP_GROUP_FORMING;
        pPort->P2PCfg.P2pCapability[1] |= (GRPCAP_GROUP_FORMING);     
    }
    else
    {
        P2pCapability[1] = 0;
        pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_GROUP_FORMING));      
    }
    
    // attach subelementID= 2.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP,  P2pCapability/*pPort->P2PCfg.P2pCapability*/, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    
    // attach subelementID= 4.
    RealIntent = TempIntent>>1;
    if (((TempIntent & 1) == 1) && (RealIntent == pPort->P2PCfg.GoIntentIdx))
    {
        RealIntent = TempIntent&0xfe;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P -    1  %d \n", pPort->P2PCfg.GoIntentIdx));
    }
    else if (((TempIntent & 1) == 0) && (RealIntent == pPort->P2PCfg.GoIntentIdx))
    {
        RealIntent = TempIntent|0x1;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P -    2  %d \n", pPort->P2PCfg.GoIntentIdx));
    }
    else
        RealIntent = pPort->P2PCfg.GoIntentIdx << 1;

    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OWNER_INTENT, &RealIntent, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 4.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &pPort->P2PCfg.ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 17.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, &pPort->P2PCfg.GroupChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    // attach subelementID= 6.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_LISTEN_CHANNEL, &pPort->P2PCfg.ListenChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    //  attach subelementID= 9.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_INTERFACE_ADDR, pPort->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    //  attach channel List= 11.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, &pPort->P2PCfg.GroupChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    //  attach subelementID= 13.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pPort->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    //  attach subelementID= 15.
    if (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO)
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pPort->CurrentAddress, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }
    
    pFrame->Length = (UCHAR)P2pIeLen;

    // 0. WSC fixed IE
    PlatformMoveMemory(pDest, &WscIEFixed[0], 6);
    pWscIeLen = (pDest + 1);
    *pWscIeLen = 4;
    pDest += 6;
    *pTotalFrameLen += 6;

    // 1. Version
    *((PUSHORT) pDest) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pDest + 2)) = cpu2be16(0x0001);
    *(pDest + 4) = WSC_VERSION;
    pDest += 5;
    *pWscIeLen += 5;
    *pTotalFrameLen   += 5;
    // 9. Device password ID. According to Table.1 in P2P Spec.

    *((PUSHORT) pDest) = cpu2be16(WSC_IE_DEV_PASS_ID);
    *((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
    *((PUSHORT) (pDest + 4)) = cpu2be16(SentDpid);
    pDest += 6;
    *pWscIeLen += 6;
    *pTotalFrameLen   += 6;

    // attach wsc version 2.0
    // announce we have WPS2.0 before message
    if(pAd->StaCfg.WSCVersion2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pDest) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pDest + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pDest + 2));

        pDest += 4;
        *pWscIeLen += 4;
        *pTotalFrameLen += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pDest, Wsc_SMI_Code, 3);
        pDest += 3;
        *pWscIeLen += 3;
        *pTotalFrameLen += 3;
        VendorExtLen += 3;

        // Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pDest) = (WSC_IE_VERSION2);
        *((PUSHORT) (pDest + 1)) = (0x01);
        *(pDest + 2) = pAd->StaCfg.WSCVersion2;
        pDest += 3;
        *pWscIeLen += 3;
        *pTotalFrameLen += 3;
        VendorExtLen += 3;

        *pVendorExtLen += cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pDest, TBV57, 12);
            pDest += 12;
            *pWscIeLen += 12;
            *pTotalFrameLen += 12;
        }
    }   
}


/*  
    ==========================================================================
    Description: 
        The routine make Go Negociation Request packet .
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PMakeGoNegoReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR            index,
    IN PUCHAR       Addr1,
    IN PUCHAR       pOutBuffer,
    OUT PULONG      pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    //PMP_PORT  pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    PUCHAR          pDest;
    ULONG   Length;
    ULONG   P2pIeLen = 0;
    UCHAR   FilledIntent;
    UCHAR   RandomB;
    UCHAR           WscIEFixed[] = {0xdd, 0x0f, 0x00, 0x50, 0xf2, 0x04};    // length will modify later
    PUCHAR      pWscIeLen;
    USHORT      SentDpid;
    UCHAR       P2pCapability[2];
    
    if (index >= MAX_P2P_GROUP_SIZE)
    {
        return;
    }
    *pTotalFrameLen = 0;
    //PlatformMoveMemory(pPort->PortCfg.Bssid, Addr1, 6);
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, Addr1, Addr1);
    DBGPRINT(RT_DEBUG_TRACE, (" P2P %d - P2PMakeGoNegoReq  TO %x %x %x %x %x %x. \n", pPort->P2PCfg.PortNumber, Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5]));

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = GO_NEGOCIATION_REQ;
    pFrame->Token = 1;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = 38;
    pDest = pFrame->Octet;
    
    // Start attach subelement.
    // attach subelementID= 2.

    P2pCapability[0] = pPort->P2PCfg.P2pCapability[0];
    P2pCapability[1] = pPort->P2PCfg.P2pCapability[1];
    if (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO)
    {
        P2pCapability[1] |= GRPCAP_OWNER;       
    }
    else
    {
        P2pCapability[1] &= (~(GRPCAP_OWNER));  
    }
    P2pCapability[1] |= GRPCAP_GROUP_FORMING;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, P2pCapability/*pPort->P2PCfg.P2pCapability*/, pDest);
    
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    //  attach subelementID= 4.
    FilledIntent = pPort->P2PCfg.GoIntentIdx<<1;
    RandomB = RandomByte(pAd);
    // If Intent is not 15, set tie breaker bit to 1 randomly.
    if (((RandomB %2) == 0) && (pPort->P2PCfg.GoIntentIdx != 15))
        FilledIntent |= 1;

    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OWNER_INTENT, &FilledIntent, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    //  attach subelementID= 7.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &pPort->P2PCfg.ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    //  attach subelementID= 6.

    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_LISTEN_CHANNEL, &pPort->P2PCfg.ListenChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    //  attach subelementID= 9.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_INTERFACE_ADDR, pPort->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    //  attach channel List= 11.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, &pPort->P2PCfg.GroupChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    //  attach subelementID= 13.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pPort->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    //  attach subelementID= 17.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, &pPort->P2PCfg.GroupChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;

    DBGPRINT(RT_DEBUG_TRACE, (" P2P   - P2PMakeGoNegoReq ExtListenPeriod = %d. \n", pPort->P2PCfg.ExtListenPeriod));
    if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - insert SUBID_P2P_EXT_LISTEN_TIMING. \n"));
        //  attach subelementID= 17.
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_EXT_LISTEN_TIMING, NULL, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }

    pFrame->Length = (UCHAR)P2pIeLen;

    // 0. WSC fixed IE

    PlatformMoveMemory(pDest, &WscIEFixed[0], 6);
    pWscIeLen = pDest + 1;
    pDest += 6;
    *pTotalFrameLen += 6;
    *pWscIeLen = 4;
    // 1. Version
    *((PUSHORT) pDest) = cpu2be16(WSC_IE_VERSION);
    *((PUSHORT) (pDest + 2)) = cpu2be16(0x0001);
    *(pDest + 4) = WSC_VERSION;
    pDest += 5;
    *pTotalFrameLen   += 5;
    *pWscIeLen += 5;
    // 9. Device password ID
    *((PUSHORT) pDest) = cpu2be16(WSC_IE_DEV_PASS_ID);
    *((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
    SentDpid = pPort->P2PCfg.Dpid;
    if (P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PROVISIONED))
    {
        switch(pAd->pP2pCtrll->P2PTable.Client[index].ConfigMethod)
        {
            case CONFIG_METHOD_DISPLAY:
            case CONFIG_METHOD_LABEL:
                SentDpid = DEV_PASS_ID_USER;
                break;
            case CONFIG_METHOD_PUSHBUTTON:
                SentDpid = DEV_PASS_ID_PBC;
                break;
            default :
                SentDpid = DEV_PASS_ID_PIN;
                break;
        }
    }
    *((PUSHORT) (pDest + 4)) = cpu2be16(SentDpid);
    pDest += 6;
    *pTotalFrameLen   += 6;
    *pWscIeLen += 6;

    // attach wsc version 2.0 in the end of wsc ie
    // announce we have WPS2.0 before message
    if(pAd->StaCfg.WSCVersion2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pDest) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pDest + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pDest + 2));

        pDest += 4;
        *pWscIeLen += 4;
        *pTotalFrameLen += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pDest, Wsc_SMI_Code, 3);
        pDest += 3;
        *pWscIeLen += 3;
        *pTotalFrameLen += 3;
        VendorExtLen += 3;

        // Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pDest) = (WSC_IE_VERSION2);
        *((PUSHORT) (pDest + 1)) = (0x01);
        *(pDest + 2) = pAd->StaCfg.WSCVersion2;
        pDest += 3;
        *pWscIeLen += 3;
        *pTotalFrameLen += 3;
        VendorExtLen += 3;

        *pVendorExtLen += cpu2be16(VendorExtLen);

        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            //Wsc 2.0 test bed, additional for version 5.7 is: IE 0x5241, Len 8, content: V5.7TEST
            UCHAR   TBV57[12] = {0x10, 0xFE, 0x00, 0x08, 0x56, 0x35, 0x2E, 0x37, 0x54, 0x45, 0x53, 0x54};
            DBGPRINT(RT_DEBUG_TRACE, ("Use WPS 2.0 Version V5.7Test \n"));  

            PlatformMoveMemory(pDest, TBV57, 12);
            pDest += 12;
            *pWscIeLen += 12;
            *pTotalFrameLen += 12;
        }
    }

}


/*  
    ==========================================================================
    Description: 
        Make P2P IE.
        
    Parameters: 
        reutrn IE lenght and buffer.
    Note:
         
    ==========================================================================
 */
VOID P2pMakeP2pIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,  
    IN  MP_PORT_SUBTYPE     PortSubtype,
    IN  UCHAR           PacketType,
    OUT PUCHAR          pOutBuf,
    OUT PULONG          pIeLen)
{
    UCHAR           P2pIEFixed[6] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x09};   // length will modify later
    ULONG           Len;
    PUCHAR          pData;
    ULONG           TempLen;
    UCHAR           Status;
    UCHAR               P2pCapability[2];
    PlatformMoveMemory(&P2pIEFixed[2], P2POUIBYTE, 4);

    pData = pOutBuf;
    Len = 0;
    *pIeLen = 0;
    // 0. P2P fixed IE
    PlatformMoveMemory(pData, &P2pIEFixed[0], 6);
    pData += 6;
    Len += 6;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (pPort->PortType >= WFD_DEVICE_PORT)
    {
        P2pCapability[0] = pPort->PortCfg.P2pCapability[0];
        P2pCapability[1] = pPort->PortCfg.P2pCapability[1];
    }
    else
#endif
    {
        P2pCapability[0] = pPort->P2PCfg.P2pCapability[0];
        P2pCapability[1] = pPort->P2PCfg.P2pCapability[1];
    }
    
    // :  .
    if (PacketType == SUBTYPE_ASSOC_RSP)
    {
        Status = P2PSTATUS_SUCCESS;

#if 0
        // Ths status attribute shall be present in p2p ie to provide status informaion when association request is denied.
        if (Status != P2PSTATUS_SUCCESS)
        {
            TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, &Status, pData);
            Len += TempLen;
            pData += TempLen;
        }
#endif

        // Microsoft WFD doesn't support Extend-Listen
        if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort) && (pPort->PortType < WFD_DEVICE_PORT))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("P2pMakeP2pIE (PacketType = %d)  insert SUBID_P2P_EXT_LISTEN_TIMING  .\n", PacketType));
            TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_EXT_LISTEN_TIMING, NULL, pData);
            Len += TempLen;
            pData += TempLen;
        }

        // this is managed infra STA connect to Infra AP. So Add P2P Interface. then this's all. return here.
        DBGPRINT(RT_DEBUG_TRACE, ("<-----AssocRsp  . (Len = %d) bExtendListen = %d. \n", Len, pPort->CommonCfg.P2pControl.field.ExtendListen));
        *(pOutBuf+1) = (Len-2);
        *pIeLen = Len;
        return;
    }
    
    if ((PortSubtype == PORTSUBTYPE_STA) && (PacketType == SUBTYPE_ASSOC_REQ))
    {
        // Always support Managed when connecting using Assoc Req.
        if (NDIS_WIN7(pAd))
            P2pCapability[0] |= DEVCAP_INFRA_MANAGED;

        // If the cross connect is enabled, check whether we need to turn off it because Managed AP Asks us to do so.
        // If already turned off, no need to check.
        if ((!MT_TEST_BIT(P2pCapability[1], GRPCAP_CROSS_CONNECT))
            && PlatformEqualMemory(pAd->PortList[PORT_0]->PortCfg.Bssid, pPort->P2PCfg.P2pManagedParm.ManageAPBSsid, MAC_ADDR_LEN))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.P2pManagedParm.APP2pManageability = %x \n",  pPort->P2PCfg.P2pManagedParm.APP2pManageability));
            // If This is the 1st Managed AP that I just want to connect to, but now I don't turn off the cross connect bit yet.
            // I have to turn off this bit when connecting to this AP.
            if ((pPort->P2PCfg.P2pManagedParm.APP2pManageability != 0xff)
                && (MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ENABLE_BIT))
                && (!MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ICS_ENABLE_BIT)))
            {
                if (NDIS_WIN7(pAd))
                {
                    P2pCapability[1] &= (~GRPCAP_CROSS_CONNECT);
                    DBGPRINT(RT_DEBUG_TRACE, ("Turn off Corss Conenct bit in Assoc Req. %x \n",  (~GRPCAP_CROSS_CONNECT)));
                }
            }
        }

        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, P2pCapability, pData);
        Len += TempLen;
        pData += TempLen;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if (pPort->PortType >= WFD_DEVICE_PORT)
        {
            TempLen = InsertP2PSubelmtTlv(pAd, 
                                            pPort, 
                                            SUBID_P2P_INTERFACE, 
                                            pPort->CurrentAddress, 
                                            pData);
        }
        else
#endif
        {
            TempLen = InsertP2PSubelmtTlv(pAd, 
                                            pPort, 
                                            SUBID_P2P_INTERFACE, 
                                            pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, 
                                            pData);
        }
        Len += TempLen;
        pData += TempLen;
        // this is managed infra STA connect to Infra AP. So Add P2P Interface. then this's all. return here.
        DBGPRINT(RT_DEBUG_TRACE, ("<-----P2pMakeP2pIE For managed STA. (Len = %d PacketType = %d.) \n", Len, PacketType));
        *(pOutBuf+1) = (Len-2);
        *pIeLen = Len;
        return;
    }


/* 
    [WiFi-Direct Spec v1.0]
    The P2P Capability subelement shall be included in Beacon frames, Probe Request frames, 
    Probe Response frames, GO Negotiation Request frames, GO Negotiation Response frames,
    GO Negotiation Confirmation frames, and (Re) Association Request frames. 

    The Group Capability Bitmap field in the P2P Capability subelement shall be
    reserved if Probe Request or Probe Response frames are transmitted by a P2P Device
    that is not a Group Owner. The Group Capability Bitmap field in the P2P Capability
    subelement shall always be reserved when the P2P Capability subelement is included in
    the (Re) Association Request frames.
*/

    if ((pPort->PortType < WFD_DEVICE_PORT) && (pPort->CommonCfg.P2pControl.field.ClientDiscovery != 0))
    {
        P2pCapability[0] |= DEVCAP_CLIENT_DISCOVER;
    }
    if ((PacketType == SUBTYPE_PROBE_REQ) || (PacketType == SUBTYPE_ASSOC_REQ))
    {
        // Probe Request Group Capability bit is reserved. (TestPlan 4.1.1)
        P2pCapability[1] = 0;
    }
    TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, P2pCapability, pData);
    Len += TempLen;
    pData += TempLen;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // If dev id specificed in discovery request, driver must use this in probe request
    if ((pPort->PortType >= WFD_DEVICE_PORT) &&
        (PacketType == SUBTYPE_PROBE_REQ) && MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID) && (pPort->P2PCfg.uNumDeviceFilters > 0))
    {
        //TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_ID, (PUCHAR)&pPort->P2PCfg.FilterDevId[0][0], pData);
        // OID V1.42 might proivde two device filter lists, which one should be used in Probe req? Currently, use 1st one.
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_ID, (PUCHAR)(pPort->P2PCfg.DeviceFilterList[0].DeviceID), pData);
        Len += TempLen;
        pData += TempLen;
    }
    else
#endif
    {
        if ((PortSubtype == PORTSUBTYPE_P2PClient) 
            || (PacketType == SUBTYPE_PROBE_REQ))
        {
            // Doesn't need to specify who I am searching for. So delete
        }
        else
        {
            TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_ID, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pData);
            Len += TempLen;
            pData += TempLen;
        }
    }

    if (PacketType == SUBTYPE_PROBE_REQ)
    {
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_LISTEN_CHANNEL, &pPort->P2PCfg.ListenChannel, pData);
        Len += TempLen;
        pData += TempLen;
        if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
        {
            TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, &pPort->P2PCfg.GroupChannel, pData);
            Len += TempLen;
            pData += TempLen;
        }

    }
    if ((PacketType == SUBTYPE_PROBE_RSP) || (PacketType == SUBTYPE_ASSOC_REQ) || (PacketType == SUBTYPE_REASSOC_REQ))
    {
        if (PacketType == SUBTYPE_ASSOC_REQ)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("  P2pMakeP2pIE (PacketType = %d)  insert SUBID_P2P_DEVICE_INFO. DeviceNameLen = %d.\n", PacketType, pPort->P2PCfg.DeviceNameLen));
        }
        //  used to uniquely reference to device address
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pData);
        Len += TempLen;
        pData += TempLen;
    }
    
    // Microsoft WFD doesn't support Extend-Listen
    if (((PacketType == SUBTYPE_PROBE_RSP) || (PacketType == SUBTYPE_PROBE_REQ)  || (PacketType == SUBTYPE_ASSOC_RSP))
        && (pPort->CommonCfg.P2pControl.field.ExtendListen == 1)
        && (pPort->PortType < WFD_DEVICE_PORT))
    {
        DBGPRINT(RT_DEBUG_INFO, ("P2pMakeP2pIE (PacketType = %d)  insert SUBID_P2P_EXT_LISTEN_TIMING  .\n", PacketType));
        TempLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_EXT_LISTEN_TIMING, NULL, pData);
        Len += TempLen;
        pData += TempLen;
    }

    if ((IS_P2P_GO_OP(pPort) ||
        (IS_P2P_MS_GO(pAd, pPort) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO)/*MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)*/)) && 
        (PacketType == SUBTYPE_PROBE_RSP))  
    {
        // If I am GO, must insert Group Info in my probe response to Probe Request that has P2P IE.
        TempLen = InsertP2PGroupInfoTlv(pAd, pData);
        Len += TempLen;
        pData += TempLen;
    }

    // The NoA has its own P2P IE. So NoA Attribute lenght doesn't count here.
    *(pOutBuf+1) = (Len-2);
    
    if ((IS_P2P_GO_OP(pPort) ||
        (IS_P2P_MS_GO(pAd, pPort) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP))) &&
        (pPort->P2PCfg.GONoASchedule.bValid == TRUE))
    {
        TempLen = P2pUpdateNoAProbeRsp(pAd, pPort, pData);
        Len += TempLen;
        pData += TempLen;
    }
    else if ((IS_P2P_GO_OP(pPort) ||
        (IS_P2P_MS_GO(pAd, pPort) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP))) &&
        (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT)))
    {
        TempLen = P2pUpdateNoAProbeRsp(pAd, pPort, pData);
        Len += TempLen;
        pData += TempLen;
    }
    DBGPRINT(RT_DEBUG_INFO, ("<----- P2pMakeP2pIE (Len = %d) \n", Len));
    *pIeLen = Len;
}


VOID P2pMakeBeaconWSCIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT        pPort,
    OUT PUCHAR          pOutBuf,
    OUT PULONG          pIeLen)
{
    UCHAR           WscIEFixed[] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x04};    // length will modify later
    ULONG           Len;
    PUCHAR          pData;
    PWSC_REG_DATA   pReg;
    PWSC_DEV_INFO   pDevInfo;   
//  UCHAR           PhyMode;  

    // Role play as Registrar used by GO
    pReg = (PWSC_REG_DATA) &pPort->StaCfg.WscControl.RegData;
    pDevInfo = (PWSC_DEV_INFO) &pReg->RegistrarInfo;

    pData = pOutBuf;
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

    // 2. Wi-Fi Protected Setup State
    *((PUSHORT) pData) = cpu2be16(WSC_IE_STATE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = WSC_APSTATE_CONFIGURED;//(pPort->P2PCfg.bConfiguredAP ? WSC_APSTATE_CONFIGURED : WSC_APSTATE_UNCONFIGURED);
    pData += 5;
    Len   += 5;

    if (pPort->P2PCfg.Dpid  != DEV_PASS_ID_NOSPEC)
    {   
        // 3. Selected Registrar
        *((PUSHORT) pData) = cpu2be16(WSC_IE_SEL_REG);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        *(pData + 4) = 1;//pDevInfo->SelReg;
        pData += 5;
        Len   += 5;

        // 4. Device Password ID
        *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_PASS_ID);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);        
        *((PUSHORT) (pData + 4)) = cpu2be16(pPort->P2PCfg.Dpid);

        // change to default pin if not PBC mode
        //if (pPort->P2PCfg.Dpid != DEV_PASS_ID_PBC) 
        //  *((PUSHORT) (pData + 4)) = cpu2be16(DEV_PASS_ID_PIN);
        pData += 6;
        Len   += 6;

        // 5. Selected Registrar Config Methods
        *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_CFG_MTHD);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        //*((PUSHORT) (pData + 4)) = cpu2be16(0x188);   // Support All : PBC, Keypad, Display   
        *((PUSHORT) (pData + 4)) = cpu2be16(0x108); // for sigma 4.2.2 test: GO can not support PBC so Support Keypad, Display
        pData += 6;
        Len   += 6;     

        DBGPRINT(RT_DEBUG_INFO, ("P2pMakeBeaconWSCIE: SelReg=1 \n"));
    }
    
    // 6. UUID-E
    *((PUSHORT) pData) = cpu2be16(WSC_IE_UUID_E);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
    PlatformMoveMemory((pData + 4), pDevInfo->Uuid, 16);
    pData += 20;
    Len   += 20;

    // 7. RF band, shall change based on current channel
    *((PUSHORT) pData) = cpu2be16(WSC_IE_RF_BAND);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
    *(pData + 4) = pDevInfo->RfBand;
    pData += 5;
    Len   += 5;

    // 8. Primary device type
    *((PUSHORT) pData) = cpu2be16(WSC_IE_PRI_DEV_TYPE);
    *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
    PlatformMoveMemory((pData + 4), pDevInfo->PriDeviceType, 8);
    pData += 12;
    Len   += 12;
    
    // 9. Primary device name
    *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
    *((PUSHORT) (pData + 2)) = cpu2be16(pPort->P2PCfg.DeviceNameLen); 
    PlatformMoveMemory((pData + 4), pPort->P2PCfg.DeviceName, pPort->P2PCfg.DeviceNameLen);
    pData += pPort->P2PCfg.DeviceNameLen + 4;
    Len   += pPort->P2PCfg.DeviceNameLen + 4;

    // attach wsc version 2.0 in the end of wsc ie
    // announce we have WPS2.0 before connection
    if(pAd->StaCfg.WSCVersion2 >= 0x20)
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

        // Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData + 1)) = (0x01);
        *(pData + 2) = pAd->StaCfg.WSCVersion2;
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
            pData+= 12;
            Len   += 12;
        }
    }

    // update the total length in vendor specific IE
    *(pOutBuf+1) = Len - 2;

    // fill in output buffer
    *pIeLen = Len;
    
}


VOID P2pMakeProbeRspWSCIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  USHORT          SubType,
    OUT PUCHAR          pOutBuf,
    OUT PULONG          pIeLen)
{
    UCHAR           WscIEFixed[] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x04};    // length will modify later
    ULONG           Len;
    PUCHAR          pData;
    PWSC_REG_DATA   pReg;
    PWSC_DEV_INFO   pDevInfo;   
    WSC_DEV_INFO    LocalDevInfo;
    
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
    
    pData = pOutBuf;
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

    if (SubType == SUBTYPE_PROBE_RSP)
    {
        // 2. Wi-Fi Protected Setup State
        *((PUSHORT) pData) = cpu2be16(WSC_IE_STATE);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        *(pData + 4) = WSC_APSTATE_CONFIGURED;//(pPort->P2PCfg.bConfiguredAP ? WSC_APSTATE_CONFIGURED : WSC_APSTATE_UNCONFIGURED);
        pData += 5;
        Len   += 5;

        // Win7: GO port and Device port use the same MAC address, response to Probe Request.
        // So need to turn ON SelReg of all Probe Response during WPS
        if (pPort->P2PCfg.Dpid != DEV_PASS_ID_NOSPEC)
        {
            if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) || IS_P2P_REGISTRA(pPort) || IS_P2P_GO_OP(pPort) || (IS_P2P_AUTOGO(pPort)))
            {
                // 3. Selected Registrar
                *((PUSHORT) pData) = cpu2be16(WSC_IE_SEL_REG);
                *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
                *(pData + 4) = 1;//pDevInfo->SelReg;
                pData += 5;
                Len   += 5;
            
                // 4. DPID shall be a required attribute if Credentials are available and ready for immediate use.
                *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_PASS_ID);
                *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
                *((PUSHORT) (pData + 4)) = cpu2be16(pPort->P2PCfg.Dpid);

                // change to default pin if not PBC mode
                //if (pPort->P2PCfg.Dpid != DEV_PASS_ID_PBC) 
                //  *((PUSHORT) (pData + 4)) = cpu2be16(DEV_PASS_ID_PIN);
                pData += 6;
                Len   += 6;
                                
                // 4. Selected Registrar Config Methods
                *((PUSHORT) pData) = cpu2be16(WSC_IE_REG_CFG_MTHD);
                *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
                //*((PUSHORT) (pData + 4)) = cpu2be16(0x188);   // Support All : PBC, Keypad, Display   
                *((PUSHORT) (pData + 4)) = cpu2be16(0x108); // for sigma 4.2.2 test: GO can not support PBC so Support Keypad, Display
                pData += 6;
                Len   += 6;
                
                DBGPRINT(RT_DEBUG_INFO, ("P2pMakeProbeRspWSCIE: SelReg=1 \n"));

            }
        }       
        
        // 5. Response Type WSC_ID_RESP_TYPE
        *((PUSHORT) pData) = cpu2be16(WSC_IE_RSP_TYPE);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) || IS_P2P_REGISTRA(pPort) || IS_P2P_GO_OP(pPort) || (IS_P2P_AUTOGO(pPort)))
            *(pData + 4) = WSC_MSGTYPE_REGISTRAR;
        else
            *(pData + 4) = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;
        pData += 5;
        Len   += 5;

        // 6. UUID-E
        *((PUSHORT) pData) = cpu2be16(WSC_IE_UUID_E);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
        PlatformMoveMemory((pData + 4), pDevInfo->Uuid, 16);
        pData += 20;
        Len   += 20;

        // 7. RF band, shall change based on current channel
        *((PUSHORT) pData) = cpu2be16(WSC_IE_RF_BAND);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        *(pData + 4) = pDevInfo->RfBand;
        pData += 5;
        Len   += 5;     

        // 8. Manufacture
        *((PUSHORT) pData) = cpu2be16(WSC_IE_MANUFACTURER);
        *((PUSHORT) (pData + 2)) = cpu2be16((strlen(Wsc_Manufacture))); 
        PlatformMoveMemory((pData + 4), pDevInfo->Manufacturer, (ULONG)strlen(Wsc_Manufacture));
        pData += strlen(Wsc_Manufacture) + 4;
        Len   += (ULONG)strlen(Wsc_Manufacture) + 4;

        // 9. Model Name
        *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NAME);
        *((PUSHORT) (pData + 2)) = cpu2be16((strlen(Wsc_Model_Name)));  
        PlatformMoveMemory((pData + 4), pDevInfo->ModelName, (ULONG)strlen(Wsc_Model_Name)); 
        pData += strlen(Wsc_Model_Name) + 4;
        Len   += (ULONG)strlen(Wsc_Model_Name) + 4;
        
        // 10 .Model Number
        *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NO);
        *((PUSHORT) (pData + 2)) = cpu2be16((strlen(Wsc_Model_Number)));    
        PlatformMoveMemory((pData + 4), pDevInfo->ModelNumber, (ULONG)strlen(Wsc_Model_Number));
        pData += strlen(Wsc_Model_Number) + 4;
        Len   += (ULONG)strlen(Wsc_Model_Number) + 4;
        
        // 11. Serial Number
        *((PUSHORT) pData) = cpu2be16(WSC_IE_SERIAL);
        *((PUSHORT) (pData + 2)) = cpu2be16((strlen(Wsc_Model_Serial)));
        PlatformMoveMemory((pData + 4), pDevInfo->SerialNumber, (ULONG)strlen(Wsc_Model_Serial));
        pData += strlen(Wsc_Model_Serial) + 4;
        Len   += (ULONG)strlen(Wsc_Model_Serial) + 4;

        // 12. Primary device type
        *((PUSHORT) pData) = cpu2be16(WSC_IE_PRI_DEV_TYPE);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
        PlatformMoveMemory((pData + 4), pDevInfo->PriDeviceType, 8);
        pData += 12;
        Len   += 12;
        
        // 13. Primary device name
        *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
        *((PUSHORT) (pData + 2)) = cpu2be16(pPort->P2PCfg.DeviceNameLen); 
        PlatformMoveMemory((pData + 4), pPort->P2PCfg.DeviceName, pPort->P2PCfg.DeviceNameLen);
        pData += pPort->P2PCfg.DeviceNameLen + 4;
        Len   += pPort->P2PCfg.DeviceNameLen + 4;
        
        // 14. Config Method
        *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            *((PUSHORT) (pData + 4)) = cpu2be16(0x108); // excluding PBC
        else
            *((PUSHORT) (pData + 4)) = cpu2be16(0x188); // Support All : PBC, Keypad, Display   
        pData += 6;
        Len   += 6;

    }
    else    // Probe Request
    {
        // 2. Request Type
        *((PUSHORT) pData) = cpu2be16(WSC_IE_REQ_TYPE);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        *(pData + 4) = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;
        pData += 5;
        Len   += 5;
    
        // Config Method
        *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_MTHD);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = cpu2be16(0x188); // Support All : PBC, Keypad, Display   
        pData += 6;
        Len   += 6;
        
        // UUID-E
        *((PUSHORT) pData) = cpu2be16(WSC_IE_UUID_E);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0010);    
        PlatformMoveMemory((pData + 4), pDevInfo->Uuid, 16);
        pData += 20;
        Len   += 20;

        // Primary device type
        *((PUSHORT) pData) = cpu2be16(WSC_IE_PRI_DEV_TYPE);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0008);    
        PlatformMoveMemory((pData + 4), pDevInfo->PriDeviceType, 8);
        pData += 12;
        Len   += 12;

        // RF band, shall change based on current channel
        *((PUSHORT) pData) = cpu2be16(WSC_IE_RF_BAND);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
        *(pData + 4) = pDevInfo->RfBand;
        pData += 5;
        Len   += 5;

        // Associate state
        *((PUSHORT) pData) = cpu2be16(WSC_IE_ASSOC_STATE);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = pDevInfo->AssocState;    // Not associated
        pData += 6;
        Len   += 6;
                    
        // Config error
        *((PUSHORT) pData) = cpu2be16(WSC_IE_CONF_ERROR);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = pDevInfo->ConfigError;   // No error
        pData += 6;
        Len   += 6;

        // Device password ID
        *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_PASS_ID);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
        *((PUSHORT) (pData + 4)) = cpu2be16(pPort->P2PCfg.Dpid);

        // change to default pin if not PBC mode
        //if (pPort->P2PCfg.Dpid != DEV_PASS_ID_PBC) 
        //  *((PUSHORT) (pData + 4)) = cpu2be16(DEV_PASS_ID_PIN);
        pData += 6;
        Len   += 6;

        // Primary device name
        *((PUSHORT) pData) = cpu2be16(WSC_IE_DEV_NAME);
        *((PUSHORT) (pData + 2)) = cpu2be16(pPort->P2PCfg.DeviceNameLen); 
        PlatformMoveMemory((pData + 4), pPort->P2PCfg.DeviceName, pPort->P2PCfg.DeviceNameLen);
        pData += pPort->P2PCfg.DeviceNameLen + 4;
        Len   += pPort->P2PCfg.DeviceNameLen + 4;

        if(pAd->StaCfg.WSCVersion2 >= 0x20)
        {
            // Manufacture
            *((PUSHORT) pData) = cpu2be16(WSC_IE_MANUFACTURER);
            *((PUSHORT) (pData + 2)) = cpu2be16((strlen(Wsc_Manufacture))); 
            PlatformMoveMemory((pData + 4), pDevInfo->Manufacturer, (ULONG)strlen(Wsc_Manufacture));
            pData += strlen(Wsc_Manufacture) + 4;
            Len   += (ULONG)strlen(Wsc_Manufacture) + 4;

            // Model Name
            *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NAME);
            *((PUSHORT) (pData + 2)) = cpu2be16((strlen(Wsc_Model_Name)));  
            PlatformMoveMemory((pData + 4), pDevInfo->ModelName, (ULONG)strlen(Wsc_Model_Name)); 
            pData += strlen(Wsc_Model_Name) + 4;
            Len   += (ULONG)strlen(Wsc_Model_Name) + 4;
            
            // Model Number
            *((PUSHORT) pData) = cpu2be16(WSC_IE_MODEL_NO);
            *((PUSHORT) (pData + 2)) = cpu2be16((strlen(Wsc_Model_Number)));    
            PlatformMoveMemory((pData + 4), pDevInfo->ModelNumber, (ULONG)strlen(Wsc_Model_Number));
            pData += strlen(Wsc_Model_Number) + 4;
            Len   += (ULONG)strlen(Wsc_Model_Number) + 4;
        }
        
    }
        
    // attach wsc version 2.0 in the end of wsc ie
    // announce we have WPS2.0 before connection
    if(pAd->StaCfg.WSCVersion2 >= 0x20)
    {
        PUSHORT     pVendorExtLen;
        USHORT      VendorExtLen = 0;
        
        *((PUSHORT) pData) = cpu2be16(WSC_IE_VENDOR_EXT);
        *((PUSHORT) (pData + 2)) = cpu2be16(0x0000);    //Initial length to 0, update length after fill the contents.
        pVendorExtLen = ((PUSHORT) (pData + 2));

        pData+= 4;
        Len   += 4;

        //WSC SMI code as Vendor ID
        PlatformMoveMemory(pData, Wsc_SMI_Code, 3);
        pData += 3;
        Len   += 3;
        VendorExtLen += 3;

        // Version2 if WSC version is highter than 2.0.
        *((PUSHORT) pData) = (WSC_IE_VERSION2);
        *((PUSHORT) (pData+ 1)) = (0x01);
        *(pData + 2) = pAd->StaCfg.WSCVersion2;
        pData+= 3;
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
    
    // update the total length in vendor specific IE
    *(pOutBuf+1) = Len - 2;

    // fill in output buffer
    *pIeLen = Len;
    
}

/*  
    ==========================================================================
    Description: 
        Make a P2P Fake NoA Attribute to trigger myself to restart NoA. The Start time is changed. Duration and Interval and Count
        is the same as GO's beacon
        
    Parameters: 
         StartTime : A new Start time.
         pOutBuffer : pointer to buffer that should put data to.
    Note:
         
    ==========================================================================
 */
VOID P2PMakeFakeNoATlv(
    IN PMP_ADAPTER pAd,
     IN  PMP_PORT        pPort,
    IN ULONG     StartTime,
    IN PUCHAR       pOutBuffer)
{
    PUCHAR      pDest;
        
    pDest = pOutBuffer;

    *(pDest) = SUBID_P2P_NOA;
    // Length is 13*n + 2 = 15 when n = 1
    *(pDest+1) = 15;
    // Lenght 2nd byte
    *(pDest+2) = 0;
    // Index.
    *(pDest+3) = pPort->P2PCfg.GONoASchedule.Token;
    // CT Windows and OppPS parm. Don't turn on both. So Set CTWindows = 0
    *(pDest+4) = 0;
    // Count.  Test Plan set to 255.
    *(pDest+5) = pPort->P2PCfg.GONoASchedule.Count;
    // Duration
    PlatformMoveMemory((pDest+6), &pPort->P2PCfg.GONoASchedule.Duration, 4);
    // Interval
    PlatformMoveMemory((pDest+10), &pPort->P2PCfg.GONoASchedule.Interval, 4);
    PlatformMoveMemory((pDest+14), &StartTime, 4);

}


/*  
    ==========================================================================
    Description: 
        Update P2P beacon frame for P2P IE Group Info Attribute.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
ULONG P2pUpdateGroupBeacon(
    IN PMP_ADAPTER pAd,
    IN ULONG    StartPosition) 
{
    UCHAR       P2PIEFixed[] = {0xdd, 0x16, 0x50, 0x6f, 0x9a, 0x09};    // length will modify later
    PUCHAR      pDest;
    ULONG       GroupInfoLen = 0;

    pDest = &pAd->pTxCfg->BeaconBuf[StartPosition + 6];
    GroupInfoLen = InsertP2PGroupInfoTlv(pAd, pDest);
    if ((GroupInfoLen > 3) && (GroupInfoLen < 250))
    {
        P2PIEFixed[1] = GroupInfoLen + 4;
        pDest = &pAd->pTxCfg->BeaconBuf[StartPosition];
        PlatformMoveMemory(pDest, P2PIEFixed, 6);
        return (GroupInfoLen+6);
    }
    else if (GroupInfoLen >= 250)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pUpdateGroupBeacon error- GroupInfoLen = %d. too large. \n", GroupInfoLen));

    }
    return 0;
}



/*  
    ==========================================================================
    Description: 
        Update P2P beacon frame and save to  BeaconBuf[].
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pUpdateBeaconP2pCap(
    IN PMP_ADAPTER pAd,
    IN  PUCHAR   pCapability) 
{
}

/*  
    ==========================================================================
    Description: 
        Update P2P beacon frame for P2P IE NoA Attribute. When I am GO.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
ULONG P2pUpdateNoABeacon(
    IN PMP_ADAPTER pAd,
    IN ULONG    StartPosition) 
{
    PUCHAR  pDest;
    UCHAR   P2PIEFixed[6] = {0xdd, 0x16, 0x50, 0x6f, 0x9a, 0x09};   // length will modify later
    UCHAR   i;
    PUCHAR  ptr;
    ULONG   longptr;
    UCHAR       CTWindow;
    PMP_PORT pPort = NULL;
    PTXWI_STRUC pTxWI = &pAd->pTxCfg->BeaconTxWI;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]))
        pPort = pAd->PortList[pPort->P2PCfg.GOPortNumber];
    else
#endif
        pPort = pAd->PortList[pPort->P2PCfg.PortNumber];

    if (pPort->P2PCfg.GONoASchedule.bValid == TRUE
        ||(pPort->P2PCfg.P2PChannelState == P2P_NOATHEN_GOTOSCAN_STATE)
        ||(pPort->P2PCfg.P2PChannelState == P2P_ENTER_GOTOSCAN)
        ||(pPort->P2PCfg.P2PChannelState == P2P_PRE_NOATHEN_GOTOSCAN_STATE))
    {
        pDest = &pAd->pTxCfg->BeaconBuf[StartPosition];
        // Always support attach one NoA.. So.. length is fixed to 0x16. :)  
        PlatformMoveMemory(pDest, P2PIEFixed, 6);

        *(pDest+6) = SUBID_P2P_NOA;
        // Length is 13*n + 2 = 15 when n = 1
        *(pDest+7) = 15;
        // Lenght 2nd byte
        *(pDest+8) = 0;
        // Index.
        *(pDest+9) = pPort->P2PCfg.GONoASchedule.Token;
        // CT Windows and OppPS parm. Don't turn on both. So Set CTWindows = 0
        *(pDest+10) = 0;
        // Count.  Test Plan set to 255.
        *(pDest+11) = pPort->P2PCfg.GONoASchedule.Count;
        // Duration
        PlatformMoveMemory((pDest+12), &pPort->P2PCfg.GONoASchedule.Duration, 4);
        // Interval
        PlatformMoveMemory((pDest+16), &pPort->P2PCfg.GONoASchedule.Interval, 4);
        PlatformMoveMemory((pDest+20), &pPort->P2PCfg.GONoASchedule.StartTime, 4);
        pAd->pP2pCtrll->GOBeaconBufNoALen = 24; 
        for (i= 0; i < 24; )
        {
            ptr = (PUCHAR) &pAd->pTxCfg->BeaconBuf[StartPosition+i];
            longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + pAd->HwCfg.TXWI_Length + StartPosition +i, longptr);
            i += 4;
        }

        DBGPRINT_RAW(RT_DEBUG_INFO, ("StartPosition = %d ", StartPosition));
        for (i = 0; i < (24);)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-", 
                 (pAd->pTxCfg->BeaconBuf[StartPosition+i]), pAd->pTxCfg->BeaconBuf[StartPosition+i+1], pAd->pTxCfg->BeaconBuf[StartPosition+i+2], pAd->pTxCfg->BeaconBuf[StartPosition+i+3]
                ,pAd->pTxCfg->BeaconBuf[StartPosition+i+4] ,pAd->pTxCfg->BeaconBuf[StartPosition+i+5] ,pAd->pTxCfg->BeaconBuf[StartPosition+i+6],pAd->pTxCfg->BeaconBuf[StartPosition+i+7]
                ,pAd->pTxCfg->BeaconBuf[StartPosition+i+8],pAd->pTxCfg->BeaconBuf[StartPosition+i+9],pAd->pTxCfg->BeaconBuf[StartPosition+i+10],pAd->pTxCfg->BeaconBuf[StartPosition+i+11]));
             i = i + 12;
        }
        //
        // step 6. move BEACON TXD and frame content to on-chip memory
        //
        WRITE_TXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pTxWI, (StartPosition + pAd->pP2pCtrll->GOBeaconBufNoALen));
        WRITE_TXWI_PHY_MODE(pAd, pTxWI, MODE_OFDM);

        ptr = (PUCHAR)&pAd->pTxCfg->BeaconTxWI;
        for (i=0; i<pAd->HwCfg.TXWI_Length; )  // 24-byte TXINFO field
        {
            longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + i, longptr);
            ptr += 4;
            i += 4;
        }
    }
    else if (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT)
        || (pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1))
    {
        pAd->pP2pCtrll->GOBeaconBufNoALen = 11; 
        P2PIEFixed[1] = 0x9;
        pDest = &pAd->pTxCfg->BeaconBuf[StartPosition];
        PlatformMoveMemory(pDest, P2PIEFixed, 6);
        *(pDest+6) = SUBID_P2P_NOA;
        // Length is 13*n + 2 = 15 when n = 1
        *(pDest+7) = 2;
        // Lenght 2nd byte
        *(pDest+8) = 0;
        // Index.
        *(pDest+9) = pPort->P2PCfg.GONoASchedule.Token;
        // CT Windows and OppPS parm
        // According to TP, we should set OpPS bit on. it's used as capability bit.
        // but our code ues pPort->P2PCfg.CTWindows as enable/disable bit.
        // So for beacon transmission, we set the bit on just right before beacon transmission.

        if (pPort->StaCfg.WscControl.WscState > WSC_STATE_INIT
            || (pAd->StaCfg.WpaState != SS_NOTUSE)
            || IS_P2P_REGISTRA(pPort))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("don't announce OpPS when doing WPS or before 4-way   %d  %d\n", pAd->StaCfg.WpaState, pPort->StaCfg.WscControl.WscState));
            CTWindow = 0;
        }
        else if ((pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1) 
            && (pPort->P2PCfg.SigmaSetting.CTWindow >= 5 ))
                CTWindow = 0x80 + pPort->P2PCfg.SigmaSetting.CTWindow;
        else
            CTWindow = pPort->P2PCfg.CTWindows;
        
        *(pDest+10) = CTWindow;

        for (i= 0; i < 11; )
        {
            ptr = (PUCHAR)&pAd->pTxCfg->BeaconBuf[StartPosition+i];
            longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + pAd->HwCfg.TXWI_Length +StartPosition +i, longptr);
            i += 4;
        }
        WRITE_TXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pTxWI, (StartPosition + 11));
        WRITE_TXWI_PHY_MODE(pAd, pTxWI, MODE_OFDM);
        
        ptr = (PUCHAR)&pAd->pTxCfg->BeaconTxWI;
        for (i = 0; i < pAd->HwCfg.TXWI_Length; )  // 24-byte TXINFO field
        {
            longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + i, longptr);
            ptr += 4;
            i += 4;
        }
    }
    else
    {
        WRITE_TXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pTxWI, StartPosition);
        WRITE_TXWI_PHY_MODE(pAd, pTxWI, MODE_OFDM);

        pAd->pP2pCtrll->GOBeaconBufNoALen = 0;
        ptr = (PUCHAR)&pAd->pTxCfg->BeaconTxWI;
        for (i = 0; i < pAd->HwCfg.TXWI_Length; )  // 24-byte TXINFO field
        {
            longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + i, longptr);
            ptr += 4;
            i += 4;
        }
        for (i= 0; i < 24; i++)
        {
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + pAd->HwCfg.TXWI_Length +StartPosition +i, 0x0);
        }
    }
    

    return pAd->pP2pCtrll->GOBeaconBufNoALen;
}



/*  
    ==========================================================================
    Description: 
        Update P2P beacon frame for P2P IE NoA Attribute. When I am GO.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
ULONG P2pUpdateNoAProbeRsp(
    IN PMP_ADAPTER pAd,
     IN  PMP_PORT        pPort,
    IN PUCHAR   pInbuffer) 
{
    PUCHAR  pDest;
    UCHAR   P2PIEFixed[6] = {0xdd, 0x16, 0x00, 0x50, 0xf2, 0x09};   // length will modify later
    
    PlatformMoveMemory(&P2PIEFixed[2], P2POUIBYTE, 4);
    
    if (pPort->P2PCfg.GONoASchedule.bValid == TRUE)
    {
        pDest = pInbuffer;
        // Always support attach one NoA.. So.. length is fixed to 0x16. :)  
        PlatformMoveMemory(pDest, P2PIEFixed, 6);

        *(pDest+6) = SUBID_P2P_NOA;
        // Length is 13*n + 2 = 15 when n = 1
        *(pDest+7) = 15;
        // Lenght 2nd byte
        *(pDest+8) = 0;
        // Index.
        *(pDest+9) = pPort->P2PCfg.GONoASchedule.Token;
        // CT Windows and OppPS parm. Don't turn on both. So Set CTWindows = 0
        *(pDest+10) = 0;
        // Count.  Test Plan set to 255.
        *(pDest+11) = pPort->P2PCfg.GONoASchedule.Count;
        // Duration
        PlatformMoveMemory((pDest+12), &pPort->P2PCfg.GONoASchedule.Duration, 4);
        // Interval
        PlatformMoveMemory((pDest+16), &pPort->P2PCfg.GONoASchedule.Interval, 4);
        PlatformMoveMemory((pDest+20), &pPort->P2PCfg.GONoASchedule.StartTime, 4);

        return 24;
    }
    else if (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT))
    {
        P2PIEFixed[1] = 0x9;
        pDest = pInbuffer;
        PlatformMoveMemory(pDest, P2PIEFixed, 6);
        *(pDest+6) = SUBID_P2P_NOA;
        // Length is 13*n + 2 = 15 when n = 1
        *(pDest+7) = 2;
        // Lenght 2nd byte
        *(pDest+8) = 0;
        // Index.
        *(pDest+9) = pPort->P2PCfg.GONoASchedule.Token;
        // CT Windows and OppPS parm
        *(pDest+10) = pPort->P2PCfg.CTWindows;
        return 11;
    }
    else
    {
        return 0;
    }
    

}

/*  
    ==========================================================================
    Description: 
        Parse P2P Service Discovery Request.
        
    Parameters: 
        return : TRUE if .... We don't support Service discovery yet. So no rule now. Always TRUE first.
    Note:
         
    ==========================================================================
 */
BOOLEAN P2PParseServiceDiscoReq(
    IN PMP_ADAPTER pAd, 
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        *ServiceTransaction) 
{
    PUCHAR      pData;

    pData = pFrame->Octet;
    do 
    {
        if (*pData != CATEGORY_PUBLIC)
        {
            DBGPRINT(RT_DEBUG_TRACE,("GASIntialReq = Not Public action. Return \n"));
            return FALSE;
        }
        if (*(pData+1) != GAS_INITIAL_REQ)
        {
            DBGPRINT(RT_DEBUG_TRACE,("GASIntialReq = Not GAS Initial Req. Return \n"));
            return FALSE;
        }
        pData += 3;
        if (*pData != 0x6c)
        {
            DBGPRINT(RT_DEBUG_TRACE,("GASIntialReq = wrong Advertisement protocol information element Return \n"));
            return FALSE;
        }
        pData += 4;
        pData += 2;
        if (*pData != 0xdd)
        {
            DBGPRINT(RT_DEBUG_TRACE,("GASIntialReq = wrong   Return \n"));
            return FALSE;
        }
        if (!PlatformEqualMemory((pData + 4), P2POUIBYTE, 4))
        {
            DBGPRINT(RT_DEBUG_TRACE,("GASIntialReq = wrong OUI Return \n"));
            return FALSE;
        }
        *ServiceTransaction = *(pData+13);
        DBGPRINT(RT_DEBUG_TRACE,("GASIntialReq = ServiceTransac is %d. \n", *ServiceTransaction));
    }while(FALSE);

     return TRUE;
}


/*  
    ==========================================================================
    Description: 
        Parse P2P Service Discovery Response.
        
    Parameters: 
        return : TRUE if .... We don't support Service discovery yet. So no rule now. Always TRUE first.
    Note:
         
    ==========================================================================
 */
BOOLEAN P2PParseServiceDiscoRsp(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT        pPort,
    IN PRT_P2P_CLIENT_ENTRY pP2pEntry,
    IN UCHAR        pP2pidx,
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        ServiceTransaction) 
{
    PUCHAR      pData;

    pData = pFrame->Octet;
    do 
    {
        if (*pData != CATEGORY_PUBLIC)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = Not Public action. Return \n"));
            return FALSE;
        }
        if (*(pData+1) != GAS_INITIAL_RSP)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = Not GAS Initial RSP. Return \n"));
            return FALSE;
        }
        
        if (*(PUSHORT)(pData+3) != 0)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = Status is 0x%x.  Return \n", *(PUSHORT)(pData+3)));
            return FALSE;
        }
        if (*(PUSHORT)(pData+5) != 0)
        {
            pP2pEntry->P2pClientState = P2PSTATE_SERVICE_COMEBACK_COMMAND;
            pP2pEntry->GeneralToken = *(pData+2);
            pPort->P2PCfg.p2pidxForServiceCbReq = pP2pidx;
            pP2pEntry->ConfigTimeOut = (*(PUSHORT)(pData+5))/100;
            if (pP2pEntry->ConfigTimeOut == 0)
                pP2pEntry->ConfigTimeOut = 1;
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = comeback delay is 0x%x.   \n", *(PUSHORT)(pData+5)));
            DBGPRINT(RT_DEBUG_TRACE,("Set GeneralToken %d.  Return \n", pP2pEntry->GeneralToken));
            return FALSE;
        }
        pData += 7;
        
        if (*pData != 0x6c)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = wrong Advertisement protocol information element Return \n"));
            return FALSE;
        }
        pData += 4;
        pData += 2;
        if (*pData != 0xdd)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = wrong   Return \n"));
            return FALSE;
        }
        if (!PlatformEqualMemory((pData + 4), P2POUIBYTE, 4))
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = wrong OUI Return \n"));
            return FALSE;
        }
        DBGPRINT(RT_DEBUG_TRACE,("P2PParseServiceDiscoRsp = ServiceTransac is %d. \n", ServiceTransaction));
    }while(FALSE);

     return TRUE;
}


/*  
    ==========================================================================
    Description: 
        Parse P2P Service Comeback Request.
        
    Parameters: 
        return : TRUE if .... We don't support Service discovery yet. So no rule now. Always TRUE first.
    Note:
         
    ==========================================================================
 */
BOOLEAN P2PParseComebackReq(
    IN PMP_ADAPTER pAd, 
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        *ServiceTransaction) 
{
    PUCHAR      pData;

    pData = pFrame->Octet;
    do 
    {
        if (*pData != CATEGORY_PUBLIC)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseComebackReq = Not Public action. Return \n"));
            return FALSE;
        }
        if (*(pData+1) != GAS_COMEBACK_REQ)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseComebackReq = Not GAS_COMEBACK_REQ Return \n"));
            return FALSE;
        }
    }while(FALSE);

     return TRUE;
}

/*  
    ==========================================================================
    Description: 
        Processing WSC IE and put to OUTPUT buffer.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2PParseWPSIE(
    IN PUCHAR   pWpsData,
    IN USHORT       WpsLen,
    OUT PUSHORT Dpid,
    OUT PUSHORT ConfigMethod,
    OUT PUCHAR  DeviceName,
    OUT UCHAR   *DeviceNameLen,
    OUT PUCHAR  pSelectedRegistrar,
    OUT PUCHAR  pNumRequestedType,  
    OUT PUCHAR  pDevType)
{
    USHORT              AccuIeLen = 0;
    PUCHAR              pData;
    USHORT              WscType, WscLen;

    if (DeviceNameLen != NULL)
        *DeviceNameLen = 0;

    if (pNumRequestedType != NULL)
        *pNumRequestedType = 0;
    
    if ((pWpsData == NULL) || (WpsLen < 6))
        return FALSE;

    if ( ((*pWpsData) != IE_VENDOR_SPECIFIC) || (!PlatformEqualMemory(pWpsData+2, WPS_OUI, 4)))
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s :  No WPS OUI  ..%x %x %x %x %x %x .\n", __FUNCTION__, *(pWpsData+0),*(pWpsData+1),*(pWpsData+2), *(pWpsData+3), *(pWpsData+4), *(pWpsData+5)));
        return FALSE;   
    }

    pData = pWpsData+6;
    AccuIeLen += 6;

    if (pData == NULL)
        return TRUE;
    
    // Start to process WSC IEs
    while (AccuIeLen <= (WpsLen - 4))
    {
        WscType = cpu2be16(*((PUSHORT) pData));
        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
        pData  += 4;

        // Parse M1 WSC type and store to RegData structure
        switch (WscType)
        {
            case WSC_IE_DEV_NAME:       /* 1 */
                if (DeviceName !=NULL)
                {
                    PlatformMoveMemory(DeviceName, pData, 32);
                    if (DeviceNameLen !=NULL)
                        *DeviceNameLen = (UCHAR)WscLen;
                    DBGPRINT(RT_DEBUG_INFO,("%s :  DeviceName = %c%c%c%c%c ...\n", __FUNCTION__, DeviceName[0], DeviceName[1], DeviceName[2],DeviceName[3],DeviceName[4]));
                }
                break;
            case WSC_IE_DEV_PASS_ID:        /* 2 */
                if (Dpid !=NULL)
                {
                    *Dpid = be2cpu16(*((USHORT *) pData));
                    //*Dpid = *((PUSHORT) pData);
                    DBGPRINT(RT_DEBUG_INFO,("%s :  Dpid = %s  .\n", __FUNCTION__, decodeDpid(*Dpid)));
                }
                break;
            case WSC_IE_CONF_MTHD:      /* 3 */
                if (ConfigMethod !=NULL)
                {
                    *ConfigMethod = be2cpu16(*((PUSHORT) pData));
                    DBGPRINT(RT_DEBUG_INFO,(" Config = %x  ..\n", *ConfigMethod));
                }
                break;
            case WSC_IE_SEL_REG:
                if (pSelectedRegistrar)
                {
                    *pSelectedRegistrar = *(pData);     /*Bool*/
                    if (*pSelectedRegistrar == 1)
                        DBGPRINT(RT_DEBUG_TRACE,(" SelectedRegistrar = %x  ..\n", *pSelectedRegistrar));                        
                }
                break;
            case WSC_IE_REQUESTED_DEVICE_TYPE:  /*Only present in the WPS IE in the Probe Request frame*/
                if ((pNumRequestedType != NULL) && (pDevType != NULL) && (WscLen == 8))
                {
                    if ((*pNumRequestedType) > MAX_P2P_FILTER_LIST)
                        break;
                    
                    PlatformMoveMemory(pDevType + (*pNumRequestedType) * 8, pData, 8);
                    (*pNumRequestedType) ++;
                    
                    DBGPRINT(RT_DEBUG_TRACE,("%s (Type = 0x%04x):  RequestedDeviceType = %x-%x-%x-%x-%x-%x-%x-%x. NumRequestedType = %d\n", 
                            __FUNCTION__, WscType, pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7], *pNumRequestedType));
                }
                break;
            case WSC_IE_PRI_DEV_TYPE:
                if ((pNumRequestedType == NULL) && (pDevType != NULL) && (WscLen == 8))
                {
                    PlatformMoveMemory(pDevType, pData, 8);
                    DBGPRINT(RT_DEBUG_TRACE,("%s (Type = 0x%04x):  PrimaryDeviceType = %x-%x-%x-%x-%x-%x-%x-%x\n", 
                            __FUNCTION__, WscType, pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7]));
                }
                break;
                
            default:
                if ((WscType & 0xff00) != 0x1000)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s --> Invaild IE 0x%04x !!! EXIT !!!\n", __FUNCTION__, WscType));
                    AccuIeLen = WpsLen; // Invalid format, stop and exit
                }
                break;
        }

        // Offset to net WSC Ie
        pData  += WscLen;
        AccuIeLen += (4 + WscLen);

    }

    return TRUE;
}

/*  
    ==========================================================================
    Description: 
        Parse P2P Service Comeback Response.
        
    Parameters: 
        return : TRUE if .... We don't support Service discovery yet. So no rule now. Always TRUE first.
    Note:
         
    ==========================================================================
 */
BOOLEAN P2PParseComebackRsp(
    IN PMP_ADAPTER pAd, 
    IN PRT_P2P_CLIENT_ENTRY pP2pEntry,
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        *ServiceTransaction) 
{
    PUCHAR      pData;

    pData = pFrame->Octet;
    do 
    {
        if (*pData != CATEGORY_PUBLIC)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseComebackRsp = Not Public action. Return \n"));
            return FALSE;
        }
        if (*(pData+1) != GAS_COMEBACK_RSP)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseComebackRsp = Not GAS_COMEBACK_REQ Return \n"));
            return FALSE;
        }
        // +2 : Token
        // +3, +4 : Status
        // +5 : More Frag
        DBGPRINT(RT_DEBUG_TRACE,("P2PParseComebackRsp = More Frag = %d. \n", *(pData+5)));
        if ((*(pData+5) &0x80) == 0x80 )
        {
            pP2pEntry->P2pClientState = P2PSTATE_SERVICE_COMEBACK_COMMAND;
            DBGPRINT(RT_DEBUG_TRACE,("P2PParseComebackRsp = More Frag set to one. \n"));
            
             return TRUE;
        }
        // +6 +7  : Comeback Delay
        // +8  : ADvertisement Potocol IE
        // +9 +10 : Query Response Len
        
        return FALSE;
        
    }while(FALSE);

}
/*  


/*  
    ==========================================================================
    Description: 
        Parse P2P subelement content to fill into the correct output buffer
        
    Parameters: 
        return : TRUE if the pSearchAddr is in GroupInfoAttribute.
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pParseGroupInfoAttribute(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT        pPort,
    IN UCHAR P2pindex, 
    IN VOID *Msg, 
    IN ULONG MsgLen) 
{
    PEID_STRUCT   pEid;
    ULONG               Length = 0;
    LONG        GroupInfoLen;
    UCHAR       i;
    UCHAR       idx;
    PUCHAR      pData;
    UCHAR       ThisDescLen;
    BOOLEAN     brc = TRUE;
    P2P_CLIENT_INFO_DESC    *pClient;
    USHORT          WscType, WscLen;
    ULONG           LeftLength;
    PP2PEID_STRUCT  pP2pEid;
    ULONG           AttriLen;
    UCHAR           SmallerP2Pidx;

    pEid = (PEID_STRUCT) Msg;

    Length = 0;
    LeftLength = MsgLen; 
    pEid = (PEID_STRUCT)Msg;
    while ((ULONG)(pEid->Len + 2) <= LeftLength)
    {
        // might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE.
        // To check Octet[1] is because Ralink add one byte itself for P2P IE. So the IE content shift one byte afterward.
        if (PlatformEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
        {
            // To check Octet[5] for first P2PEid
            // is because Ralink add one byte itself for P2P IE. So the IE content shift one byte afterward.
            pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[5];
            AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            Length = 0;
            while ((Length + 3 + AttriLen) <= pEid->Len)    
            {
                switch(pP2pEid->Eid)
                {
                    case SUBID_P2P_GROUP_INFO:
                        GroupInfoLen = AttriLen;
                        pData = &pP2pEid->Octet[0];
                        DBGPRINT(RT_DEBUG_INFO, ("SUBID_P2P_GROUP_INFO - Go index=%d\n",  P2pindex));
                        while(GroupInfoLen > 0)
                        {   
                            pClient = (P2P_CLIENT_INFO_DESC*)pData;
                            ThisDescLen = pClient->Length;
                            if ((ThisDescLen < 23) || (ThisDescLen > GroupInfoLen))
                            {
                                DBGPRINT(RT_DEBUG_ERROR, ("Error parsing P2P IE group info attribute. This Client -%d/%d\n", ThisDescLen, GroupInfoLen));
                                break;
                            }
                            idx = P2pGroupTabSearch(pAd, pClient->DevAddr);
                            if ((idx < MAX_P2P_GROUP_SIZE) 
                                && (pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState != P2PSTATE_DISCOVERY_CLIENT)
                                && (pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState != P2PSTATE_CLIENT_DISCO_COMMAND))
                            {
                                // P2P topology changed. Reset the P2P Table and rescan.
                                if (idx > P2pindex)
                                    SmallerP2Pidx = P2pindex + 1;   // don't delete GO
                                else
                                    SmallerP2Pidx = idx;        // this method also delete GO. :(   
                                DBGPRINT(RT_DEBUG_ERROR, ("!P2P topology changed[P2pClientState = %d] when parsing P2P IE group info attribute. Delete from index : %d\n", pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState, SmallerP2Pidx ));
                                P2PPrintP2PEntry(pAd, idx);
                                for (i = (SmallerP2Pidx); i < MAX_P2P_GROUP_SIZE;i++)
                                {

                                    if ((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_PROVISION_COMMAND)
                                        || (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CONNECT_COMMAND)
                                        || (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState > P2PSTATE_DISCOVERY_CLIENT))
                                    {
                                        DBGPRINT(RT_DEBUG_ERROR, ("!break right away because we have another connect command to continue. update topology is not so important to do right now.\n" ));
                                        break;
                                    }
                                    else
                                    {
                                        P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
                                    }
                                    // Don't update this now.
                                    if (pAd->pP2pCtrll->P2PTable.ClientNumber == SmallerP2Pidx)
                                        return FALSE;
                                }
                                //  because we check idx in following code, 
                                // So need to Search again after topology changed
                                idx = P2pGroupTabSearch(pAd, pClient->DevAddr);
                            }
                            
                            if ((idx == P2P_NOT_FOUND) && ( pAd->pP2pCtrll->P2PTable.ClientNumber < (MAX_P2P_GROUP_SIZE - 1))
                                && (!PlatformEqualMemory(pClient->DevAddr, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, MAC_ADDR_LEN)))
                            {
                                idx = P2pGroupTabInsert(pAd, pClient->DevAddr, P2PSTATE_DISCOVERY_CLIENT, NULL, 0);
                                
                                DBGPRINT(RT_DEBUG_TRACE, ("Insert a P2P Client to P2P table [%d]  .\n", idx));
                                // Insert a P2P Client followi ng the GO that we received. Need to delete ALL following client that is already in P2P table.
                                // Because the number of client that is in the p2p group might be changed.
                            }
                            if (idx < MAX_P2P_GROUP_SIZE)
                            {
                                pAd->pP2pCtrll->P2PTable.Client[idx].DevCapability = pClient->Capability;
                                // Don't update state when it's in P2PSTATE_CLIENT_DISCO_COMMAND
                                if (pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState != P2PSTATE_CLIENT_DISCO_COMMAND)
                                    pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState = P2PSTATE_DISCOVERY_CLIENT;
                                pAd->pP2pCtrll->P2PTable.Client[idx].ConfigMethod = *((PUSHORT) pClient->ConfigMethod);
                                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[idx].PrimaryDevType, pClient->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
                                pAd->pP2pCtrll->P2PTable.Client[idx].NumSecondaryType = pClient->NumSecondaryType;
                                WscType = cpu2be16(*((PUSHORT) &pClient->Octet[pClient->NumSecondaryType*P2P_DEVICE_TYPE_LEN]));
                                WscLen  = cpu2be16(*((PUSHORT) (&pClient->Octet[2 + pClient->NumSecondaryType*P2P_DEVICE_TYPE_LEN])));
                                PlatformMoveMemory(&pAd->pP2pCtrll->P2PTable.Client[idx].DeviceName[0], &pClient->Octet[4+(pClient->NumSecondaryType*P2P_DEVICE_TYPE_LEN)], 32);
                                if (WscLen <= 32)
                                    pAd->pP2pCtrll->P2PTable.Client[idx].DeviceNameLen = WscLen;
                                pAd->pP2pCtrll->P2PTable.Client[idx].MyGOIndex = P2pindex;
                                // Assign client's opchannel to its GO's opchannel.
                                pAd->pP2pCtrll->P2PTable.Client[idx].OpChannel = pAd->pP2pCtrll->P2PTable.Client[P2pindex].OpChannel;
                                if (pPort->P2PCfg.ConnectingIndex < MAX_P2P_GROUP_SIZE)
                                {
                                    if (PlatformEqualMemory(pAd->pP2pCtrll->P2PTable.Client[idx].addr, &pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], MAC_ADDR_LEN))
                                        P2pConnectAfterScan(pAd, pPort, FALSE, idx);
                                }
                            }
                            GroupInfoLen -= (ThisDescLen + 1);
                            pData += (ThisDescLen + 1);
                        };
                        break;
                    default:
                        break;
                        
                }
                Length = Length + 3 + AttriLen;  // Eid[1] + Len[1]+ content[Len]
                pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
                AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            }
            // We already get what we want. so break.
            break;
        }
        LeftLength = LeftLength - pEid->Len - 2;
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }
    return brc;
}


/*  
    ==========================================================================
    Description: 
        Parse P2P subelement content to fill into the correct output buffer
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pParseManageSubElmt(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT        pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT UCHAR *pChannel,
    OUT UCHAR *pNumOfP2pOtherAttribute,
    OUT UCHAR *pTotalNumOfP2pAttribute,
    OUT UCHAR *pMamageablity,
    OUT UCHAR *pMinorReason)
{
    PP2PEID_STRUCT  pP2pEid;
    PEID_STRUCT     pEid;
    ULONG           Length;
    ULONG           AttriLen;
    ULONG           LeftLength;

    //DBGPRINT(RT_DEBUG_TRACE, ("P2pParseManageSubElmt  MsgLen = %d.  \n", MsgLen));
    if (pNumOfP2pOtherAttribute != NULL)
        *pNumOfP2pOtherAttribute = 0;

    if (pTotalNumOfP2pAttribute != NULL)
        *pTotalNumOfP2pAttribute = 0;

    LeftLength = MsgLen; 
    pEid = (PEID_STRUCT)Msg;
    while ((ULONG)(pEid->Len + 2) <= LeftLength)
    {
        if (pEid->Eid == IE_CHANNEL_USAGE)
        {
            *pChannel = pEid->Octet[2];
            DBGPRINT(RT_DEBUG_TRACE, ("IE_CHANNEL_USAGE  = %x %x %x %x [ch=]%x.  \n", pEid->Eid, pEid->Len, pEid->Octet[0], pEid->Octet[1], pEid->Octet[2]));
        }
        if (pEid->Eid == IE_COUNTRY)
        {
            PlatformMoveMemory(&pPort->P2PCfg.P2pManagedParm.CountryContent[0], &pEid->Octet[0], 6);
            DBGPRINT(RT_DEBUG_TRACE, ("IE_COUNTRY  = %x %x %x %x %x. %x %x %x  \n", pEid->Eid, pEid->Len, pEid->Octet[0], pEid->Octet[1], pEid->Octet[2], pEid->Octet[3], pEid->Octet[4], pEid->Octet[5]));
        }
        // might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE.
        if (PlatformEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
        {
            // Get Request content capability
            pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
            AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            Length = 0;
            while ((Length + 3 + AttriLen) <= pEid->Len)    
            {
                switch(pP2pEid->Eid)
                {
                    case SUBID_P2P_MINOR_REASON:
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC -  Has P2P SUBID_P2P_MINOR_REASON IE Minor Reason = %d.\n", pP2pEid->Octet[0]));
                        if (pTotalNumOfP2pAttribute != NULL)
                            *pTotalNumOfP2pAttribute = *pTotalNumOfP2pAttribute+1;
                        if (pNumOfP2pOtherAttribute != NULL)
                            *pNumOfP2pOtherAttribute = *pNumOfP2pOtherAttribute+1;
                        if (pMinorReason != NULL)
                            *pMinorReason = pP2pEid->Octet[0];
                        break;
                    case SUBID_P2P_MANAGEABILITY:
                        if (pTotalNumOfP2pAttribute != NULL)
                            *pTotalNumOfP2pAttribute = *pTotalNumOfP2pAttribute+1;
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC -Ap Has P2P Manageability IE . Total P2P IE count is %d \n", *pTotalNumOfP2pAttribute));
                        if (pMamageablity != NULL)
                            *pMamageablity = pP2pEid->Octet[0];
                        break;
                    default:
                        if (pTotalNumOfP2pAttribute != NULL)
                            *pTotalNumOfP2pAttribute = *pTotalNumOfP2pAttribute+1;
                        if (pNumOfP2pOtherAttribute != NULL)
                            *pNumOfP2pOtherAttribute = *pNumOfP2pOtherAttribute+1;
                        break;
                        
                }
                Length = Length + 3 + AttriLen;  // Eid[1] + Len[1]+ content[Len]
                pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
                AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            }

        }
        LeftLength = LeftLength - pEid->Len - 2;
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }

}

/*  
    ==========================================================================
    Description: 
        Parse P2P NoA subelement content to make appropriate action for NoA schedule.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pParseNoASubElmt(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    IN UCHAR  wcidindex) 
{
    ULONG               Length = 0;
    PP2PEID_STRUCT  pP2pEid;
    ULONG           AttriLen;
    ULONG           LeftLength;
    PEID_STRUCT     pEid;
    UCHAR           ChannelListLeft = MAX_NUM_OF_CHANNELS;
    BOOLEAN         brc;
    BOOLEAN         bNoAAttriExist = FALSE;
    PUCHAR          pData;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, wcidindex);  

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,wcidindex));
        return;
    }
   
    // Intel sends multiple P2P IE... So I can't give each input a default value..
    if (MsgLen == 0)
        return;

    LeftLength = MsgLen; 
    pEid = (PEID_STRUCT)Msg;
    while ((ULONG)(pEid->Len + 2) <= LeftLength)
    {
        // might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE.
        if (PlatformEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
        {
            // Get Request content capability
            pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
            AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            Length = 0;
            while ((Length + 3 + AttriLen) <= pEid->Len)    
            {
                switch(pP2pEid->Eid)
                {
                    case SUBID_P2P_NOA:
                        pData = &pEid->Octet[0];
                        DBGPRINT(RT_DEBUG_INFO,("Get NoA Attr: %x %x %x %x %x %x %x %x %x \n", *(pData+0), *(pData+1), *(pData+2), 
                        *(pData+3), *(pData+4), *(pData+5), *(pData+6), *(pData+7), *(pData+8)));
                        bNoAAttriExist = TRUE;
                        brc = P2pHandleNoAAttri(pAd, pPort, pWcidMacTabEntry,  &pP2pEid->Eid);
                        // Got a NoA Attribute from this p2pindex. In fact, This should be GO.
                        if (brc == TRUE)
                        {
                            pPort->P2PCfg.NoAIndex = wcidindex;
                            if((pPort->P2PCfg.bNoAOn == FALSE) && (!MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT)))
                            {
                                pPort->P2PCfg.bNoAOn = TRUE;
                                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_SET_PSM_CFG, NULL, 0);
                            }
                        }
                        break;
                    default:
                        break;
                        
                }
                Length = Length + 3 + AttriLen;  // Eid[1] + Len[1]+ content[Len]
                pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
                AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            }

        }
        LeftLength = LeftLength - pEid->Len - 2;
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }

    if (bNoAAttriExist == FALSE)
    {
        if (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT))
        {
            DBGPRINT(RT_DEBUG_TRACE,("Beacon and no NoA Attribute! \n"));
            P2pStopOpPS(pAd, pPort);
        }
        if ((pWcidMacTabEntry->P2pInfo.NoADesc[0].bValid == TRUE))
        {
            DBGPRINT(RT_DEBUG_TRACE,("Beacon and no NoA Attribute!Stop active NoA \n"));
            P2pStopNoA(pAd, pWcidMacTabEntry);
        }
        pPort->P2PCfg.bKeepSlient = FALSE;
        pPort->P2PCfg.bPreKeepSlient = FALSE;
    }
}



/*  
    ==========================================================================
    Description: 
        Parse P2P subelement content to fill into the correct output buffer. May contain multiple P2P IE and WPS IE.
        So need while loop to find all IE.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pParseSubElmt(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    IN BOOLEAN  bBeacon, 
    OUT USHORT *pDpid,
    OUT UCHAR *pGroupCap,
    OUT UCHAR *pDeviceCap,
    OUT UCHAR *pDeviceName,
    OUT UCHAR *pDeviceNameLen,
    OUT UCHAR *pDevAddr,
    OUT UCHAR *pInterFAddr,
    OUT UCHAR *pBssidAddr,
    OUT UCHAR *pSsidLen,
    OUT UCHAR *pSsid,
    OUT USHORT *pConfigMethod,
    OUT USHORT *pWpsConfigMethod,
    OUT UCHAR *pDevType,
    OUT UCHAR *pNumSecondaryType,
    OUT UCHAR *pSecondarDevTypes,
    OUT UCHAR *pListenChannel,
    OUT UCHAR *pOpChannel,
    OUT UCHAR *pChannelList,
    OUT UCHAR *pIntent,
    OUT UCHAR *pStatusCode,
    OUT UCHAR *pInviteFlag) 
{
    ULONG               Length = 0;
    PP2PEID_STRUCT  pP2pEid;
    ULONG           AttriLen;
    USHORT          WscType, WscLen;
    UCHAR           offset;
    PEID_STRUCT     pEid;
    UCHAR           ChannelListLeft = MAX_NUM_OF_CHANNELS;
    ULONG           AccuP2PIELen;
    ULONG           AccuIeLen = 0;
    UCHAR           NumOfChannel, ChannelListSize = 0, i;
    BOOLEAN         bFillDevId = FALSE;
    UCHAR           SelectedRegistrar = 0;
    
    // Intel sends multiple P2P IE... So I can't give each input a default value..
    if (MsgLen == 0)
        return;
    
    pEid = (PEID_STRUCT)Msg;
    
    AccuIeLen = pEid->Len + 2;
    if (PlatformEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
    {
        AccuIeLen = pEid->Len + pEid->Octet[0]*256 + 3;
        DBGPRINT(RT_DEBUG_TRACE, ("P2pParseSubElmt for proprietary foramt.  AccuIeLen = %d.  .. \n", AccuIeLen));
    }
    
    while ((AccuIeLen) <= MsgLen)
    {
        if (PlatformEqualMemory(&pEid->Octet[0], WPS_OUI, 4))
        {
            if (bBeacon == TRUE)
                P2PParseWPSIE(&pEid->Eid, (pEid->Len + 2), pDpid, pWpsConfigMethod, pDeviceName, pDeviceNameLen, &SelectedRegistrar, NULL, NULL);
            else
                P2PParseWPSIE(&pEid->Eid, (pEid->Len + 2), pDpid, pWpsConfigMethod, NULL, NULL, &SelectedRegistrar, NULL, NULL);
        }
        // might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE.
        else if ((PlatformEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
            ||(PlatformEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4)))
        {
            // Get Request content capability
            if (PlatformEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
            {
                pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[5];
                AccuP2PIELen = pEid->Len + pEid->Octet[0]*256;
            }
            else
            {
                pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
                AccuP2PIELen = pEid->Len;
            }
            AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            Length = 4; // add 4-byte p2p oui
            while ((Length + 3 + AttriLen) <= AccuP2PIELen)    
            {
                switch(pP2pEid->Eid)
                {                       
                    case SUBID_P2P_EXT_LISTEN_TIMING:
                        break;
                    case SUBID_P2P_INVITE_FLAG:
                        if ((pInviteFlag != NULL) && (pP2pEid->Len[0] == 1) && (pP2pEid->Len[1] == 0))
                        {
                            *pInviteFlag = pP2pEid->Octet[0];
                        }
                        break;
                    case SUBID_P2P_MANAGEABILITY:
                        break;
                    case SUBID_P2P_CAP:
                        if ((pGroupCap != NULL) && (pP2pEid->Len[0] == 2) && (pP2pEid->Len[1] == 0))
                        {
                            *pGroupCap = pP2pEid->Octet[1]; 
                            *pDeviceCap = pP2pEid->Octet[0]; 
                        }   
                        break;
                    case SUBID_P2P_OWNER_INTENT:
                        if ((pIntent != NULL) && (pP2pEid->Len[0] == 1) && (pP2pEid->Len[1] == 0))
                        {
                            *pIntent = pP2pEid->Octet[0];
                        }
                        break;
                    case SUBID_P2P_CHANNEL_LIST:
                        if(pP2pEid->Len[0] > 5 && pChannelList != NULL)
                        {
                            int tempEntryLen = AttriLen - 3; // Minux Country String
                            PCHAR pStartEntry = &pP2pEid->Octet[0] + 3; // Across Country String
                            
                            while(tempEntryLen > 0)
                            {
                                NumOfChannel = *(pStartEntry + 1);

                                pStartEntry = pStartEntry + 2;

                                for(i = 0; i < NumOfChannel; i++)
                                {
                                    *(pChannelList + i) = *(pStartEntry + i);
                                    DBGPRINT(RT_DEBUG_TRACE, ("ChannelList = %d", *(pChannelList + i) ));
                                }
                                
                                tempEntryLen = tempEntryLen - 2 - NumOfChannel;
                                if(tempEntryLen > 2)
                                {
                                    pChannelList = pChannelList + NumOfChannel;
                                    pStartEntry = pStartEntry + NumOfChannel;
                                }

                            }
                        }
                        break;
                    case SUBID_P2P_OP_CHANNEL:
                        if ((pOpChannel != NULL) && (pP2pEid->Len[0] == 5) && (pP2pEid->Len[1] == 0))
                        {
                            *pOpChannel = pP2pEid->Octet[4];    // Octet[1] is regulatory 
                        }   
                        break;
                    case SUBID_P2P_LISTEN_CHANNEL:
                        if ((pListenChannel != NULL) && (pP2pEid->Len[0] == 5) && (pP2pEid->Len[1] == 0))
                        {
                            *pListenChannel = pP2pEid->Octet[4];    // Octet[1] is regulatory 
                        }
                        break;
                    case SUBID_P2P_GROUP_BSSID: // group Bssid
                        if ((pBssidAddr != NULL) && (pP2pEid->Len[0] == MAC_ADDR_LEN) && (pP2pEid->Len[1] == 0))
                        {
                            PlatformMoveMemory(pBssidAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
                        }
                        break;
                    case SUBID_P2P_INTERFACE_ADDR:
                        if ((pInterFAddr != NULL) && (pP2pEid->Len[0] == MAC_ADDR_LEN) && (pP2pEid->Len[1] == 0))
                        {
                            PlatformMoveMemory(pInterFAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
                        }
                        break;
                    case SUBID_P2P_DEVICE_ID:
                        // Beacon has this field.
                        if ((pDevAddr != NULL) && (pP2pEid->Len[0] == MAC_ADDR_LEN) && (pP2pEid->Len[1] == 0))
                        {
                            PlatformMoveMemory(pDevAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
                            bFillDevId = TRUE;
                        }
                        break;
                    case SUBID_P2P_GROUP_ID:
                        
                        if ((pSsid != NULL) && (pP2pEid->Len[0]) > 6 && (pP2pEid->Len[0] <= 38))
                        {
                            PlatformMoveMemory(pSsid, &pP2pEid->Octet[6], (pP2pEid->Len[0] - 6));
                            if (pSsidLen != NULL)
                                *pSsidLen = pP2pEid->Len[0] - 6;
                            DBGPRINT(RT_DEBUG_TRACE, (" SUBID_P2P_GROUP_ID - SSID ( Len %d)= %c %c %c %c %c %c %c %c %c.. \n",  *pSsidLen, pSsid[0], pSsid[1],pSsid[2],pSsid[3],pSsid[4],pSsid[5],pSsid[6],pSsid[7],pSsid[8]));

                            if ((pDevAddr != NULL) && (bFillDevId == FALSE))
                            {
                                PlatformMoveMemory(pDevAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
                                DBGPRINT(RT_DEBUG_TRACE, (" SUBID_P2P_GROUP_ID - DevAddr = %x %x %x %x %x %x.\n",  pDevAddr[0], pDevAddr[1], pDevAddr[2], pDevAddr[3], pDevAddr[4], pDevAddr[5]));
                            }
                        }
                        break;
                    case SUBID_P2P_DEVICE_INFO:
                        if (pDevAddr != NULL)
                            PlatformMoveMemory(pDevAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
                        if (pConfigMethod != NULL) // big-endian?
                            *pConfigMethod = be2cpu16(*(PUSHORT)&pP2pEid->Octet[6]);
                        if (pDevType != NULL) // big-endian?
                            PlatformMoveMemory(pDevType, &pP2pEid->Octet[8], 8);
                        // Maybe this is an error device type.. print it out
                        if (pP2pEid->Octet[9] == 0)
                            DBGPRINT(RT_DEBUG_ERROR,("SUBID_P2P_DEVICE_INFO - Receive DevType : %x %x  = %x %x %x %x = %x %x  \n", pP2pEid->Octet[8], pP2pEid->Octet[9],
                            pP2pEid->Octet[10], pP2pEid->Octet[11], pP2pEid->Octet[12], pP2pEid->Octet[13], pP2pEid->Octet[14], pP2pEid->Octet[15]));

                        // Secondary Types.
                        if (pNumSecondaryType != NULL)  // big-endian?
                        {
                            if (pP2pEid->Octet[16] > 0)
                            {
                                *pNumSecondaryType = min(pP2pEid->Octet[16], MAX_P2P_SECONDARY_DEVTYPE_LIST);
                                DBGPRINT(RT_DEBUG_TRACE,("SUBID_P2P_DEVICE_INFO - NumSecondaryType =%d \n", (*pNumSecondaryType)));

                                if (pSecondarDevTypes != NULL)
                                    PlatformMoveMemory(pSecondarDevTypes, &pP2pEid->Octet[17], (*pNumSecondaryType) * 8);
                            }
                        }
                        
                        // Count the device name offset.
                        offset = 17 + pP2pEid->Octet[16] * 8;
                        WscType = cpu2be16(*((PUSHORT) &pP2pEid->Octet[offset]));
                        WscLen  = cpu2be16(*((PUSHORT) (&pP2pEid->Octet[offset+2])));
                        if ((WscType == WSC_IE_DEV_NAME) && (WscLen <= 32)) // big-endian?
                        {
                            if ((pDeviceName != NULL))
                            {
                                *pDeviceNameLen = (UCHAR)WscLen;
                                PlatformMoveMemory(pDeviceName, &pP2pEid->Octet[21 + pP2pEid->Octet[16]*8], WscLen);
                                DBGPRINT(RT_DEBUG_INFO, ("SUBID_P2P_DEVICE_INFO Device Name= %c %c %c %c %c %c \n",  pDeviceName[0], pDeviceName[1],pDeviceName[2],pDeviceName[3],pDeviceName[4],pDeviceName[5]));
                            }
                        }

                        break;
                    case SUBID_P2P_STATUS:
                        if ((pStatusCode != NULL) && (pP2pEid->Len[0] == 1) && (pP2pEid->Len[1] == 0))
                        {
                            *pStatusCode = pP2pEid->Octet[0];
                            DBGPRINT(RT_DEBUG_INFO, (" SUBID_P2P_STATUS    Eid = %x \n", *pStatusCode));
                        }
                        
                        break;
                    case SUBID_P2P_GROUP_INFO:
                        
                        break;
                    case SUBID_P2P_NOA:
                        
                        break;
                    case SUBID_P2P_CONFIG_TIMEOUT:
                        break;
                    default:
                        DBGPRINT(RT_DEBUG_ERROR, (" SUBID_P2P_ unknown  Eid = %x \n", pP2pEid->Eid));
                        break;
                        
                }
                Length = Length + 3 + AttriLen;  // Eid[1] + Len[1]+ content[Len]
                if (Length >= AccuP2PIELen)
                    break;
                pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
                AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
            }

        }

        // already reach the last IE. Stop finding next Eid.
        if (AccuIeLen >= MsgLen)
            break;
        
        // Forward buffer to next pEid
        if (PlatformEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
        {
            pEid = (PEID_STRUCT)((UCHAR*)pEid + (pEid->Len + pEid->Octet[0]*256 + 3)); 
            DBGPRINT(RT_DEBUG_ERROR, ("We already accumul ate all P2P IE. don't need to search next P2P IE \n"));
            // We already accumul ate all P2P IE. don't need to search next P2P IE
            break;
        }
        else
        {
            pEid = (PEID_STRUCT)((UCHAR*)pEid + pEid->Len + 2);    
        }
        
        // Since we get the next pEid, 
        // Predict the accumulated IeLen after adding the next pEid's length.   
        // The accumulated IeLen is for checking length.
        if (PlatformEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
        {
            AccuIeLen += (pEid->Len + pEid->Octet[0]*256 + 3);
        }
        else
        {
            AccuIeLen += (pEid->Len + 2);
        }       

    }

}

BOOLEAN P2pMlmeSyncMlmeSyncPeerBeaconAtJoinAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem,
    IN PUCHAR       Bssid,
    IN UCHAR        SelectedRegistrar)
{
    UCHAR       P2pManageability = 0xff;
    UCHAR       NumOfOtherP2pAttri = 1;
    UCHAR       Channel = 1;
    UCHAR       TotalNumOfP2pAttribute = 1;
    UCHAR       GroupFormation = 0, GroupCap = 0, DevCap = 0;
    BOOLEAN     ReadyToConnect = TRUE;
    UCHAR       discoindex = P2P_NOT_FOUND;
    PFRAME_802_11   pFrame = (PFRAME_802_11) Elem->Msg;
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    
    if (Elem->MsgLen <= (LENGTH_802_11 + 12))
        return ReadyToConnect;
    
    P2pParseManageSubElmt(pAd, 
                     pPort,
                    &Elem->Msg[LENGTH_802_11 + 12], 
                    (Elem->MsgLen - LENGTH_802_11 - 12), 
                    &Channel,
                    &NumOfOtherP2pAttri,
                    &TotalNumOfP2pAttribute,
                    &P2pManageability,
                    NULL);

    //DBGPRINT(RT_DEBUG_TRACE, ("P2pParseManageSubElmt  TotalNumOfP2pAttribute = %d.  \n", TotalNumOfP2pAttribute));
    // If this AP carries Managed Attribute. Update to our ManageAPBSsid
    pPort->P2PCfg.P2pManagedParm.APP2pManageability = 0xff;
    if (P2pManageability != 0xff)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Receive desired BEACON with valid P2pManageability= %x\n", P2pManageability));
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC -NumOfOtherP2pAttri %d . Channel = %d. \n", NumOfOtherP2pAttri, Channel));
        PlatformMoveMemory(pPort->P2PCfg.P2pManagedParm.ManageAPBSsid, Bssid, MAC_ADDR_LEN);

        // Reset Minor Reason when connecting to Managed AP.
        pPort->P2PCfg.P2pManagedParm.APP2pMinorReason = 0;
        pPort->P2PCfg.P2pManagedParm.APUsageChannel = Channel ;
        // If this is the latest managed AP that I connected to. but this AP turn off the managed function.
        // I should clear my record.
        
        // If this is a manged AP. update to the ManageAPBSsid.
        if ((MT_TEST_BIT(P2pManageability, P2PMANAGED_ENABLE_BIT)) && (NumOfOtherP2pAttri == 0))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC -I am connecting to a Managed AP. Save this Bssid. %x %x %x \n", Bssid[3],Bssid[4],Bssid[5]));
        }
        
        pPort->P2PCfg.P2pManagedParm.APP2pManageability = P2pManageability;
    }
    pPort->P2PCfg.P2pManagedParm.TotalNumOfP2pAttribute = TotalNumOfP2pAttribute;

    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BEACON P2pManageability= %x. TotalNumOfP2pAttribute = %d. OtherP2pAttri = %d. \n", P2pManageability, TotalNumOfP2pAttribute, NumOfOtherP2pAttri));

    //
    // When the driver gets a CONNECT_TO_GROUP_REQUEST, the driver should not
    // attempts to connect until it sees the required values in GO's beacon/probe response.
    //
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Bssid))
    {
        // Get GroupFormation bit in GroupCapability attribute of P2P IE 
        P2pParseSubElmt(pAd, &Elem->Msg[LENGTH_802_11 + 12], (Elem->MsgLen - LENGTH_802_11 - 12),  
                        ((pFrame->Hdr.FC.SubType == SUBTYPE_BEACON) ? TRUE : FALSE), NULL, &GroupCap, &DevCap, NULL, NULL, 
                        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        if (MT_TEST_BIT(GroupCap, GRPCAP_GROUP_FORMING))
            GroupFormation = 1;

        if (IS_P2P_MS_CLI(pAd, pAd->PortList[Elem->PortNum]) || IS_P2P_MS_CLI2(pAd, pAd->PortList[Elem->PortNum]))
        {
            // If this probe response didn't carry P2P IE
            // We check GroupFormation bit again from discovery entry
            if (GroupFormation == 0)
            {
                discoindex = P2pDiscoTabSearch(pAd, Bssid, NULL, NULL); 
                if (discoindex < MAX_P2P_DISCOVERY_SIZE)
                {
                    if (MT_TEST_BIT(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].GroupCapability, GRPCAP_GROUP_FORMING))
                        GroupFormation = 1;                  
                }
            }
            
            if ((pPort->P2PCfg.InGroupFormation) && (pPort->P2PCfg.WaitForWPSReady))
            {
                if (GroupFormation && SelectedRegistrar)
                {
                    pPort->P2PCfg.ReadyToConnect = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("[Win8] %s: ReadyToConnect to GO with GroupFormation and SelectedRegistrar\n", __FUNCTION__));
                }
                else
                    pPort->P2PCfg.ReadyToConnect = FALSE;
            }
            else if (pPort->P2PCfg.WaitForWPSReady)
            {
                if (SelectedRegistrar)
                {
                    pPort->P2PCfg.ReadyToConnect = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("[Win8] %s: ReadyToConnect to GO with SelectedRegistrar\n", __FUNCTION__));
                }
                else
                    pPort->P2PCfg.ReadyToConnect = FALSE;
            }
            else
                pPort->P2PCfg.ReadyToConnect = TRUE;

            // Indicate ConnectStart for this wps connection request.
            if ((pPort->P2PCfg.ReadyToConnect == TRUE) && (pPort->P2PCfg.WaitForWPSReady == TRUE))
                PlatformIndicateConnectionStart(pAd, pAd->PortList[Elem->PortNum], pAd->StaCfg.BssType, NULL, NULL, 0);

        }
        else
        {
            // Not Win8 Client Port, set to true
            pPort->P2PCfg.ReadyToConnect = TRUE;
        }
        
        ReadyToConnect = pPort->P2PCfg.ReadyToConnect;
        DBGPRINT(RT_DEBUG_TRACE, ("[Win8] %s: ReadyToConnect = %d \n", __FUNCTION__, ReadyToConnect));

    }
#endif

    return ReadyToConnect;
}


/*  
    ==========================================================================
    Description: 
        The routine check whether we need to check this frame's transmission result. Success ? or Retry fail?.
        If found retry fail, we may need to resend this frame again. 
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pAckRequiredCheck(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN PP2P_PUBLIC_FRAME    pFrame,
    OUT         UCHAR   *TempPid)
{
    UCHAR               EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
    PUCHAR      pDest;
    PFRAME_P2P_ACTION       pP2pActFrame;
    PUCHAR      ptr = NULL;

    pP2pActFrame = (PFRAME_P2P_ACTION)pFrame;
    ptr = (PUCHAR)&pFrame->OUI[0];
    
    // check Go Neg Confirm Frame.
    if ((pFrame->Subtype == GO_NEGOCIATION_CONFIRM) && (PlatformEqualMemory(ptr, P2POUIBYTE, 4)) && (pFrame->Action == P2P_ACTION) && (pFrame->Category == CATEGORY_PUBLIC))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a Confirm Frame. Pid = RequireACK !!!\n"));
        *TempPid = PID_REQUIRE_ACK;
        return;
    }

    // check Intivation Response.
    if ((pFrame->Subtype == P2P_INVITE_REQ) && (PlatformEqualMemory(ptr, P2POUIBYTE, 4)) && (pFrame->Action == P2P_ACTION) && (pFrame->Category == CATEGORY_PUBLIC))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a Invite Req Frame. Pid = RequireACK !!!\n"));
        *TempPid = PID_REQUIRE_ACK;
        return;
    }

    // check Intivation Response.
    if ((pFrame->Subtype == P2P_INVITE_RSP) && (PlatformEqualMemory(ptr, P2POUIBYTE, 4)) && (pFrame->Action == P2P_ACTION) && (pFrame->Category == CATEGORY_PUBLIC))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a Invite Rsp Frame. Pid = RequireACK !!!\n"));
        *TempPid = PID_REQUIRE_ACK;
        // for GO romaining with persistent bit on we need let peer reinvoke consequencely
        if((pPort->CommonCfg.P2pControl.field.EnablePresistent == 1) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO))
            pPort->P2PCfg.P2PConnectState = P2P_INVITE;
        return;
    }

    if ((pP2pActFrame->Category == MT2_ACT_VENDOR) 
        && (pP2pActFrame->OUISubType == P2PACT_GO_DISCOVER_REQ) 
        && (pP2pActFrame->OUIType == 0x9))  
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a GO Discoverbility. Pid = RequireACK !!!\n"));
        *TempPid = PID_REQUIRE_ACK;
        return;
    }
    // As Go, need to check EAP fail's ACK
    // JANTEMP . not add this check yet.
    if ((IS_P2P_REGISTRA(pPort)) && (pFrame->p80211Header.FC.Type == BTYPE_DATA))
    {
        pDest = &pFrame->Category;
        if (PlatformEqualMemory(EAPHEAD, pDest, 8))
        {
            pDest += 8;
            if ((*pDest == EAPOL_VER) && (*(pDest+1) == EAPPacket) && (*(pDest + 3) == EAP_CODE_FAIL))
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a EAP FailFrame.   Pid = RequireACK!!!\n"));
                *TempPid = PID_REQUIRE_ACK;
            }
        }

    }
}


/*  
    ==========================================================================
    Description: 
        Updating client state after transmitting packets
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pClientStateUpdate(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    ULONG i;

    for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
    {
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_WAIT_GO_COMFIRM_ACK)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("GO Negociation Confirm Sent!!!! \n"));
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_GO_COMFIRM_ACK_SUCCESS;
        }
        else if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Reinvoke presistent Invite Rsp Sent!!!! \n"));
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS;
        }
        else if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_WAIT_GO_DISCO_ACK)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("GO Discoveribility Action Frame Sent!!!! \n"));
            // Only when Client state is CLIENT_OPERATING, will driver set to P2PSTATE_WAIT_GO_DISCO_ACK state.
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS;
        }
        else if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_SENT_INVITE_REQ)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Invite Req Frame Sent!!!! \n"));
            // Because Invitation Req might be sent when I am P2P Client or when I am GO. 
            // Because after I send invitation req, I stay in that channel to wait for Invite response, 
            // So once the invitation packet transmission is done, I may go back to Op Channel. 
            P2pBackToOpChannel(pAd, pPort);
            
            // Very difficult to implement in USB, there ignore ....
            /*
            if (0)
            {
                pAd->pP2pCtrll->P2PTable.Client[j].P2pClientState = P2PSTATE_INVITE_COMMAND;
                P2pSetListenIntBias(pAd, 3);
                pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
                P2pGotoScan(pAd);
            }
            */
        }
    }
}

/*  
    ==========================================================================
    Description: 
        Processing the Probe Response frame when operating as a P2P Device. 
        Only called from STA's state machine that is in scanning.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN PeerP2pBeaconProbeRspAtScan(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort,
    IN MLME_QUEUE_ELEM *Elem,
    IN USHORT CapabilityInfo,
    IN UCHAR    WorkingChannel,
    OUT PRT_P2P_CLIENT_ENTRY *pP2PBssEntry,
    OUT PRT_P2P_DISCOVERY_ENTRY *pP2PDiscoEntry)
{
    BOOLEAN     bBeacon = FALSE;
    BOOLEAN     bresult;
    BOOLEAN     bChangeState = FALSE;
    ULONG       P2PSubelementLen;
    PUCHAR      P2pSubelement = NULL;
    UCHAR       index = P2P_NOT_FOUND;
    UCHAR       discoindex = P2P_NOT_FOUND;
    UCHAR       Addr2[6], SsidLen;
    UCHAR       Ssid[32];
    ULONG       Peerip;
    UCHAR       DevType[8], DevAddr[6], Channel = 0, OpChannel = 0, Intent;
    UCHAR       GroupCap = 0, DeviceCap, StatusCode;
    USHORT      Dpid;
    PHEADER_802_11  pHeader;
    USHORT      ConfigMethod = 0xffff;
    UCHAR       DeviceNameLen = 0, DeviceName[32];
    UCHAR       SavedP2PTableNum;
    BOOLEAN     containP2P = FALSE;
    UCHAR           NumSecondaryType = 0, SecondarDevTypes[MAX_P2P_SECONDARY_DEVTYPE_LIST][P2P_DEVICE_TYPE_LEN];
    
    // If there is already BSS or IBSS and only one port, no need to parse Probe response for P2P discovery feature
    // INFRA on includes case that I am P2P client. in this case, doesn't need to parse P2P IE in Probe Response either??
    // Init P2pSubelement
    PlatformAllocateMemory(pAd,  &P2pSubelement, MAX_VIE_LEN);
    if (P2pSubelement == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("3 PeerP2pBeaconProbeRspAtScan::1Allocate memory size(=1024) failed\n"));
        goto CleanUp;
    }

    PlatformZeroMemory(Ssid, 32);
    PlatformZeroMemory(DevAddr, 6);
    // P2P Device's probe response doesn't set ess bit on. 
    pHeader = (PHEADER_802_11)&Elem->Msg[0];
    // In P2P spec, the P2P IE in probe response and beacon are different. 
    // So set a boolean to check if this is a beacon frame.
    if (pHeader->FC.SubType == SUBTYPE_BEACON)
        bBeacon = TRUE;
    
    // Intel put P2P IE into two separate IE
    // Sanity check
    if (PeerP2pProbeRspSanity(pAd, 
                                pPort,
                                Elem->Msg, 
                                Elem->MsgLen, 
                                Addr2, 
                                Ssid, 
                                &SsidLen, 
                                &Peerip,
                                &P2PSubelementLen,
                                P2pSubelement))
    {
        containP2P = TRUE;
        
        // Check P2P's Probe Response validatity.
        if (!(PlatformEqualMemory(Ssid, &WILDP2PSSID[0], WILDP2PSSIDLEN)))
        {
            DBGPRINT(RT_DEBUG_INFO, ("PeerP2pBeaconProbeRspAtScan= %c %c %c %c %c %c%c \n",  Ssid[0], Ssid[1],Ssid[2],Ssid[3],Ssid[4],Ssid[5],Ssid[6]));
            //goto CleanUp;
        }

        // Step 1:  Parse P2P attribute
        // If this peer is provisioned, don't update Config
        if (bBeacon == FALSE)
        {
            P2pParseSubElmt(pAd, (PVOID)P2pSubelement, P2PSubelementLen, 
                bBeacon, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, NULL, NULL, NULL, &ConfigMethod, NULL, DevType, &NumSecondaryType, (PUCHAR)&SecondarDevTypes[0][0], &Channel, &OpChannel, NULL, &Intent, &StatusCode, NULL);
        }
        else
            P2pParseSubElmt(pAd, (PVOID)P2pSubelement, P2PSubelementLen, 
                bBeacon, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, NULL, NULL, NULL, NULL, NULL, DevType, &NumSecondaryType, (PUCHAR)&SecondarDevTypes[0][0], &Channel, &OpChannel, NULL, &Intent, &StatusCode, NULL);

        ////////////////////////////////////////////////////////////////////////////////////
        // [WIN8] Process beacon and probe response and build up Discovery Table
        if (pAd->NumberOfPorts > 1)
        {
            do
            {
                // Insert to Discovery Table when do win8 discovery scanning.
                discoindex = P2pDiscoTabSearch(pAd, Addr2, NULL, NULL);
                if (discoindex == P2P_NOT_FOUND)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[Win8] New From Addr2 = %x %x %x %x %x %x. bBeacon = %d . GroupCap = %x. SsidLen=%d \n",  Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5], bBeacon, GroupCap, SsidLen));
                    DBGPRINT(RT_DEBUG_TRACE, ("[Win8] New From DevAddr = %x %x %x %x %x %x. ConfigMethod = %x \n",  DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5], ConfigMethod));
                    discoindex = P2pDiscoTabInsert(pAd, Addr2, DevAddr, Ssid, SsidLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("[Win8] Insert to table[%d]\n", discoindex));
                }

                // Update discovery table according to this is beacon or probe response. 
                // Beacon and probe response carries different attribute. 
                if (discoindex < MAX_P2P_DISCOVERY_SIZE)
                {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))                                 
                    // debug        
                    if ((MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID)) && 
                        (PlatformEqualMemory(pPort->P2PCfg.DeviceFilterList[0].DeviceID, DevAddr, 6)) && 
                        ((!bBeacon)))
                    {
                        pPort->P2PCfg.SetPobeReqTimer = FALSE;
                        DBGPRINT(RT_DEBUG_TRACE, ("[Win8] Update ProbeRsp/Beacon with filtering device address, Addr2 = %x %x %x %x %x %x. bBeacon= %d\n", Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5], bBeacon));
                    }
#endif

                    // This is p2p device
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].bLegacyNetworks = FALSE;

                    // If this is beacon. It must use correct Bsid and Ssid.
                    if (bBeacon == TRUE)
                    {
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].Ssid, Ssid, MAX_LEN_OF_SSID);
                        pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].SsidLen = SsidLen;
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].Bssid, Addr2, MAC_ADDR_LEN);
                        pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].OpChannel = WorkingChannel;
                    }
                    else if (bBeacon == FALSE)
                    {
                        // If this is GO.                   
                        if ((GroupCap & GRPCAP_OWNER) == GRPCAP_OWNER)
                        {
                            // Probe Response in social channels may not necessarily the operating channel of a GO. So... don't update operating channnel.
                            // I always want to use beacon's channel as operating channel. So.. Only update Opchannel if not get beacon yet.
                            if ((pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].OpChannel == 0) && (WorkingChannel != 1) && (WorkingChannel != 6) && (WorkingChannel != 11))
                            {
                                pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].OpChannel = WorkingChannel;
                            }
                            if (CAP_IS_ESS_ON(CapabilityInfo))
                            {
                                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].Bssid, Addr2, MAC_ADDR_LEN);
                                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].Ssid, Ssid, MAX_LEN_OF_SSID);
                                pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].SsidLen = SsidLen;
                            }
                        }
                        else if (!CAP_IS_ESS_ON(CapabilityInfo))
                        {
                            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].Bssid, Addr2, MAC_ADDR_LEN);
                        }
                        
                        // P2P PrimaryDevType only appears in Probe Response. not in beacon.
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].PrimaryDevType, DevType, P2P_DEVICE_TYPE_LEN);
                        // Save secondary device types
                        if (NumSecondaryType > 0)
                        {
                            pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].NumSecondaryType = NumSecondaryType;
                            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].PrimaryDevType, SecondarDevTypes, NumSecondaryType * P2P_DEVICE_TYPE_LEN);
                        }
                        // Can I just set this channel that I got Probe response as listent channel??
                        // Use DS parameter as peer listen if DS parameter exits or use driver's channel setting as peer listen channel
                        if (WorkingChannel != 0xff)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("%s::Set P2P Listen Ch from DS Para Ch(%d)\n", __FUNCTION__, WorkingChannel));                        
                            if(WorkingChannel != Elem->Channel )
                            {
                                DBGPRINT(RT_DEBUG_TRACE, ("%s::P2P Listen Ch Mismatch, DS Para Ch(%d), DriverCh(%d)\n", __FUNCTION__, WorkingChannel, Elem->Channel));
                            }
                            pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].ListenChannel = WorkingChannel;
                        }
                        else
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("%s::Set P2P Listen Ch from Driver Ch(%d)\n", __FUNCTION__, Elem->Channel));
                            pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].ListenChannel = Elem->Channel;
                        }
                        DBGPRINT(RT_DEBUG_TRACE, ("%s::pAd->pP2pCtrll->P2PTable.DiscoEntry[%d].ListenChannel=%d\n", __FUNCTION__, discoindex, pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].ListenChannel));
                    }

                    // update P2P Capability
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].DevCapability = DeviceCap;
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].GroupCapability = GroupCap;

                    // update life time = 5 min
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].LifeTime = 3000;//unit 100ms

                    //return relevant BssEntry to caller, the caller then updates information in this BssEntry
                    *pP2PDiscoEntry = &pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex];

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                    //
                    // Take some action when the peer is found in this chaannel
                    // Send out the  buffering of PD request/NEGO request/Invite request frame
                    //
                    if ((PlatformEqualMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].TransmitterAddr, &pPort->P2PCfg.ConnectingMAC[0][0], MAC_ADDR_LEN))
                        && (pPort->P2PCfg.pRequesterBufferedFrame)                
                        && (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED)
                        ||  MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED)
                        ||  MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED)))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("%s - TO %x %x %x %x %x %x. \n",
                                    __FUNCTION__, pPort->P2PCfg.ConnectingMAC[0][0], pPort->P2PCfg.ConnectingMAC[0][1], pPort->P2PCfg.ConnectingMAC[0][2],
                                    pPort->P2PCfg.ConnectingMAC[0][3], pPort->P2PCfg.ConnectingMAC[0][4], pPort->P2PCfg.ConnectingMAC[0][5]));

                            // Update flags
                            if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED))
                            {
                                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED);
                                MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ);
                                DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Provision Request (len = %d, flags = %d) \n",__FUNCTION__, pPort->P2PCfg.RequesterBufferedSize, pPort->P2PCfg.WaitForMSCompletedFlags ));
                            }
                            else if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED))
                            {
                                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED);
                                MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ);
                                DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Invitation Request (len = %d, flags = %d) \n",__FUNCTION__, pPort->P2PCfg.RequesterBufferedSize, pPort->P2PCfg.WaitForMSCompletedFlags ));
                            }
                            else if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED))
                            {
                                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED);
                                MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ);
                                DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Nego Request (len = %d, flags = %d) \n",__FUNCTION__, pPort->P2PCfg.RequesterBufferedSize, pPort->P2PCfg.WaitForMSCompletedFlags ));
                            }
                            
#ifdef MULTI_CHANNEL_SUPPORT
                            MlmeSyncForceToTriggerScanComplete(pAd);
#endif /*MULTI_CHANNEL_SUPPORT*/
                                
                            // Kickoff TX
                            NdisCommonMiniportMMRequest(pAd, pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize);

                            //Free Buffered Data
                            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);
                        }
#endif          
                }
            }while(FALSE);
        }
        ////////////////////////////////////////////////////////////////////////////////////


        // Step 2:
        // when I am connected.  My P2P table MUST starts from my GO.
        // this makes the task that driver notify GUI My p2p table easier.
        if (IS_P2P_CLIENT_OP(pPort))
        {
            index = P2pGroupTabSearch(pAd, pPort->P2PCfg.Bssid);
            if ((index == P2P_NOT_FOUND) && (!PlatformEqualMemory(pPort->P2PCfg.Bssid, Addr2, MAC_ADDR_LEN)))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("1 From Addr2 = %x %x %x %x %x %x  \n",  Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
                DBGPRINT(RT_DEBUG_TRACE, ("1 pPort->P2PCfg.Bssid = %x %x %x %x %x %x  return\n",  pPort->P2PCfg.Bssid[0], pPort->P2PCfg.Bssid[1],pPort->P2PCfg.Bssid[2],pPort->P2PCfg.Bssid[3],pPort->P2PCfg.Bssid[4],pPort->P2PCfg.Bssid[5]));
                goto CleanUp;
            }
        }

        // Step 3:   Insert.
        SavedP2PTableNum = pAd->pP2pCtrll->P2PTable.ClientNumber;
        index = P2pGroupTabSearch(pAd, DevAddr);
        if (index == P2P_NOT_FOUND)
        {
            // Because only Probe Response carrys group info, 
            // We always want to use Probe Response to add to my P2P table structure. 
            // If this is beacon ... return.
            if (bBeacon == TRUE)
                goto CleanUp;
            
            DBGPRINT(RT_DEBUG_TRACE, ("2 New From Addr2 = %x %x %x %x %x %x. bBeacon = %d . GroupCap = %x \n",  Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5], bBeacon, GroupCap));
            DBGPRINT(RT_DEBUG_TRACE, ("2 New From DevAddr = %x %x %x %x %x %x. ConfigMethod = %x \n",  DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5], ConfigMethod));
            if (IS_P2P_CLIENT_OP(pPort) && PlatformEqualMemory(pPort->P2PCfg.Bssid, Addr2, MAC_ADDR_LEN))
            {
                index = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_GO_OPERATING, Ssid, SsidLen);
            }
            else if ((bBeacon == TRUE) || ((GroupCap&GRPCAP_OWNER) == GRPCAP_OWNER))
            {
                index = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY_GO, Ssid, SsidLen);
            }
            // P2P client can't send probe response. So the probe response must be from P2P device.
            else
            {
                index = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY, Ssid, SsidLen);
            }
            // update the device type to UI
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].PrimaryDevType, DevType, P2P_DEVICE_TYPE_LEN);
        }
            
        // No matter the index is from existing table, or newly added, always update Ssid Information If this peer is not connected.
        // Step 4:   Update table.
        if (index < MAX_P2P_GROUP_SIZE)
        {
            pAd->pP2pCtrll->P2PTable.Client[index].DevCapability = DeviceCap;
            pAd->pP2pCtrll->P2PTable.Client[index].GroupCapability = GroupCap;
            // Update Rule according to latest Probe Response or Beacon that I received.
            if ((bBeacon == TRUE) && 
                ((pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_DISCOVERY_CLIENT)||(pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_DISCOVERY)))
            {
                // If total P2P device number > 10, don't want keep update topology if topology change.
                if ((pAd->pP2pCtrll->P2PTable.ClientNumber < 10)
                    || (((pAd->pP2pCtrll->P2PTable.ClientNumber%4) == 3)&&(pAd->pP2pCtrll->P2PTable.ClientNumber >= 10)))
                bChangeState = TRUE;
                pAd->pP2pCtrll->P2PTable.Client[index].Rule = P2P_IS_GO;
                pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY_GO;
                DBGPRINT(RT_DEBUG_TRACE, ("3 From DevAddr = %x %x %x %x %x %x.  Change rule to GO GroupCap = %x \n",  DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5], GroupCap));
            }
            else if ((pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_DISCOVERY_GO)
                && ((GroupCap&GRPCAP_OWNER) != GRPCAP_OWNER))
            {
                if ((pAd->pP2pCtrll->P2PTable.ClientNumber < 10)
                    || (((pAd->pP2pCtrll->P2PTable.ClientNumber%4) == 3)&&(pAd->pP2pCtrll->P2PTable.ClientNumber >= 10)))
                bChangeState = TRUE;
                pAd->pP2pCtrll->P2PTable.Client[index].Rule = P2P_IS_CLIENT;
                pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY;
                DBGPRINT(RT_DEBUG_TRACE, ("4 From DevAddr = %x %x %x %x %x %x.  Change rule to Device. GroupCap = %x \n",  DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5], GroupCap));
            }
            
            // If peer is provisioned, don't update. We treat him only support this provisioned configmethod.
            if (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PROVISIONED) && (ConfigMethod != 0xffff))
                pAd->pP2pCtrll->P2PTable.Client[index].ConfigMethod = ConfigMethod;

            // Always Update Device Name
            if (DeviceNameLen != 0)
            {
                // device name changed. update GUI
                if (pAd->pP2pCtrll->P2PTable.Client[index].DeviceName[0] != DeviceName[0])
                    bChangeState = TRUE;
                PlatformMoveMemory(&pAd->pP2pCtrll->P2PTable.Client[index].DeviceName[0], DeviceName, 32);
                pAd->pP2pCtrll->P2PTable.Client[index].DeviceNameLen = DeviceNameLen;
            }

            // If this is beacon. It must use correct opchannel, ssid, interface addr, and SSID. So update.
            // Step 4 - 1:   Update table according to this is beacon or probe response. Beacon and probe response carries
            // different attribute. 
            if (bBeacon == TRUE)
            {
                pAd->pP2pCtrll->P2PTable.Client[index].Rule = P2P_IS_GO;
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].Ssid, Ssid, 32);
                pAd->pP2pCtrll->P2PTable.Client[index].SsidLen = SsidLen;
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].bssid, Addr2, MAC_ADDR_LEN);
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].InterfaceAddr, Addr2, MAC_ADDR_LEN);
                pAd->pP2pCtrll->P2PTable.Client[index].OpChannel = WorkingChannel;
                // The peer is GO. So set its state to GO_WPS directly. No need for Group forming procedure.
                // Search this beacon's group info, and insert its client to my p2p table too.
                if ((GroupCap & GRPCAP_OWNER) != GRPCAP_OWNER)
                {
                    // Print for debug.
                    DBGPRINT(RT_DEBUG_TRACE,("P2p : One P2P device[%d] send out beacom. But group owner %x bit not set ? \n", GroupCap));
                }
            }
            // Make it more readable to use "else if". Group info only appears in Probe Response.
            else if (bBeacon == FALSE)
            {
                // If this is GO.                   
                if ((GroupCap & GRPCAP_OWNER) == GRPCAP_OWNER)
                {
                    // The peer is GO. So set its state to GO_WPS directly. No need for Group forming procedure.
                    pAd->pP2pCtrll->P2PTable.Client[index].Rule = P2P_IS_GO;
                    // Probe Response in social channels may not necessarily the operating channel of a GO. So... don't update operating channnel.
                    // I always want to use beacon's channel as operating channel. So.. Only update Opchannel if not get beacon yet.
                    if ((pAd->pP2pCtrll->P2PTable.Client[index].OpChannel == 0) && (WorkingChannel != 1) && (WorkingChannel != 6) && (WorkingChannel != 11))
                    {
                        pAd->pP2pCtrll->P2PTable.Client[index].OpChannel = WorkingChannel;
                    }
                    if (CAP_IS_ESS_ON(CapabilityInfo))
                    {
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].bssid, Addr2, MAC_ADDR_LEN);
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].InterfaceAddr, Addr2, MAC_ADDR_LEN);
                        PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].Ssid, Ssid, 32);
                        pAd->pP2pCtrll->P2PTable.Client[index].SsidLen = SsidLen;
                    }
                }
                else
                {
                    // Not GO anymore, update State.
                    if (IS_P2P_PEER_DISCOVERY(&pAd->pP2pCtrll->P2PTable.Client[index]))
                        pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY;
                    pAd->pP2pCtrll->P2PTable.Client[index].Rule = P2P_IS_CLIENT;
                }
                // P2P PrimaryDevType only appears in Probe Response. not in beacon.
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[index].PrimaryDevType, DevType, 8);
                // Can I just set this channel that I got Probe response as listent channel??
                pAd->pP2pCtrll->P2PTable.Client[index].ListenChannel = Elem->Channel;
                bresult = P2pParseGroupInfoAttribute(pAd, pPort, index, P2pSubelement, P2PSubelementLen);
                if (bresult == FALSE)
                    goto CleanUp;
            }

            // Step 5: Take Some action when the peer is 
            // Decide to connect? or Provision ? or Service discovery ?
            if ((pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState <= P2PSTATE_GO_DONE))
            {
                if ((DeviceCap & DEVCAP_INVITE) == DEVCAP_INVITE)
                    P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_INVITE_ENABLED);
                
                DBGPRINT(RT_DEBUG_INFO,("P2p : GrpCap=%x. DevCap=%x. ConfigMethod= %x   DeviceNameLen = %d.\n", GroupCap, DeviceCap, pAd->pP2pCtrll->P2PTable.Client[index].ConfigMethod, pAd->pP2pCtrll->P2PTable.Client[index].DeviceNameLen));
                DBGPRINT(RT_DEBUG_INFO,("P2p : DevAddr   %x.  %x. %x.  %x. %x.  %x. \n", DevAddr[0], DevAddr[1],DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5]));
                DBGPRINT(RT_DEBUG_TRACE, (" ConnectingMAC = %x.  %x. %x.  %x. %x.  %x.\n", pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][1], pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][2],
                    pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][3], pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][4], pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][5]));
                DBGPRINT(RT_DEBUG_TRACE, ("Addr2 = %x %x %x %x %x %x", Addr2[0], Addr2[1], Addr2[2], Addr2[3], Addr2[4], Addr2[5]));
                DBGPRINT(RT_DEBUG_TRACE, ("pAd->pP2pCtrll->P2PTable.Client[index].bssid %x.  %x. %x.  %x. %x.  %x. \n", pAd->pP2pCtrll->P2PTable.Client[index].bssid[0], pAd->pP2pCtrll->P2PTable.Client[index].bssid[1], pAd->pP2pCtrll->P2PTable.Client[index].bssid[2], pAd->pP2pCtrll->P2PTable.Client[index].bssid[3], pAd->pP2pCtrll->P2PTable.Client[index].bssid[4], pAd->pP2pCtrll->P2PTable.Client[index].bssid[5]));
                DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.ConnectingIndex = %d \n", pPort->P2PCfg.ConnectingIndex));
                
                if (pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_SERVICE_DISCO_COMMAND)
                {
                    P2pActionSendServiceReqCmd(pAd, pPort, index, pAd->pP2pCtrll->P2PTable.Client[index].addr);

                    P2pStopScan(pAd, pPort);
                    pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY;
                }
                else if (pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_GO_DISCO_COMMAND)
                {
                    bresult = P2pClientDiscovery(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[index].addr, index);
                    P2pStopScan(pAd, pPort);
                }
                else if ((pPort->P2PCfg.ConnectingIndex < MAX_P2P_GROUP_SIZE) &&
                    (!PlatformEqualMemory(ZeroSsid, &pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], MAC_ADDR_LEN)))
                {   
                    // Try GO's Bssid because Atheros's/Broadcom's DevAddr is different to MacAddr.
                    if ((pAd->pP2pCtrll->P2PTable.Client[index].Rule == P2P_IS_GO) &&
                        PlatformEqualMemory(pAd->pP2pCtrll->P2PTable.Client[index].bssid, &pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], MAC_ADDR_LEN))
                    {
                        P2pConnectAfterScan(pAd, pPort, bBeacon, index);
                    }
                    else if (PlatformEqualMemory(pAd->pP2pCtrll->P2PTable.Client[index].addr, &pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], MAC_ADDR_LEN))
                    {
                        P2pConnectAfterScan(pAd, pPort, bBeacon, index);
                    }
                }
            }

            // table number changed. Decide whether to Notify GUI to update 
            if ((SavedP2PTableNum != pAd->pP2pCtrll->P2PTable.ClientNumber) || (bChangeState == TRUE))
            {
                pPort->P2PCfg.P2pCounter.TopologyUpdateCounter = 0;
            }

        }
    }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // Fill legacy networks to discovery table during active legacy-network-scan  
    else if (NDIS_WIN8_ABOVE(pAd) && 
            (MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_FORCE_LEGACY_NETWORK)) &&
            (SsidLen > 0) && (!(PlatformEqualMemory(Ssid, &WILDP2PSSID[0], WILDP2PSSIDLEN))) && (P2PSubelementLen == 0))
        {
            ////////////////////////////////////////////////////////////////////////////////////
            // [WIN8] Process beacon and probe response and build up Discovery Table for legacy networks
            if (pAd->NumberOfPorts > 1)
            {
                // Insert to Discovery Table when do win8 discovery scanning.
                discoindex = P2pDiscoTabSearch(pAd, Addr2, NULL, NULL);
                if (discoindex == P2P_NOT_FOUND)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[Win8] New From Addr2 for legacy network= %x %x %x %x %x %x. bBeacon = %d.\n",  Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5], bBeacon));
                    discoindex = P2pDiscoTabInsert(pAd, Addr2, NULL, Ssid, SsidLen);
                }

                // Update discovery table 
                if (discoindex < MAX_P2P_DISCOVERY_SIZE)
                {
                    // This is legacy network
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].bLegacyNetworks = TRUE;

                    // Bsid, Ssid
                    PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].Ssid, Ssid, MAX_LEN_OF_SSID);
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].SsidLen = SsidLen;
                    PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].Bssid, Addr2, MAC_ADDR_LEN);
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].OpChannel = WorkingChannel;
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].ListenChannel = WorkingChannel;

                    // update life time = 5 min
                    pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].LifeTime = 3000;//unit 100ms

                    //return relevant BssEntry to caller, the caller then updates information in this BssEntry
                    *pP2PDiscoEntry = &pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex];
                }
            }
            ////////////////////////////////////////////////////////////////////////////////////

        }
#endif

CleanUp:
    if ((discoindex!= P2P_NOT_FOUND) && (discoindex < MAX_P2P_DISCOVERY_SIZE))
    {
        pAd->pP2pCtrll->P2PTable.DiscoEntry[discoindex].bValid = TRUE;
        DBGPRINT(RT_DEBUG_INFO, ("Win8 - Finish inerting Discovery table entry and set the valid status TRUE here.\n"));    
    }

    if (P2pSubelement)
        PlatformFreeMemory(P2pSubelement, MAX_VIE_LEN);

    return containP2P;
}



/*  
    ==========================================================================
    Description: 
        Processing the Beacon frame when operating as a P2P client. 
        Only called from STA's state machine that is in idle.
        this function can support NoA and show Ralink IP.
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID PeerP2pBeacon(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort,
    IN PUCHAR   pAddr2,
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Addr2[6], SsidLen;
    UCHAR   Ssid[32];
    ULONG       Peerip;
    ULONG   P2PSubelementLen;
    PUCHAR  P2pSubelement = NULL;
    PFRAME_802_11       pFrame;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid);      
    pFrame = (PFRAME_802_11)Elem->Msg;

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,Elem->Wcid));
        return;
    }
    
    // Only check beacon .
    if (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP)
        return;

    if ((MlmeSyncGetRoleTypeByWlanIdx(pPort, Elem->Wcid) !=ROLE_WLANIDX_BSSID) && (!IS_P2P_MS_CLI_WCID(pPort, Elem->Wcid)))
    {
        DBGPRINT(RT_DEBUG_INFO, ("0 PeerP2pBeaconProbeRspAtScan failed : can't find wcid = %d \n", Elem->Wcid));
        return;
    }
    if (pWcidMacTabEntry->ValidAsP2P == FALSE)
    {
        DBGPRINT(RT_DEBUG_INFO, ("1 PeerP2pBeaconProbeRspAtScan failed :  wcid = %d. not ValidAsP2P. Bug!please check. \n", Elem->Wcid));
        return;
    }
    // Init P2pSubelement
    PlatformAllocateMemory(pAd,  &P2pSubelement, MAX_VIE_LEN);
    if (P2pSubelement == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("3 PeerP2pBeacon::1Allocate memory size(=1024) failed\n"));
        goto CleanUp;
    }

    if (PeerP2pBeaconSanity(pAd, 
                                pPort,
                                Elem->Msg, 
                                Elem->MsgLen, 
                                Addr2, 
                                Ssid, 
                                &SsidLen, 
                                &Peerip,
                                &P2PSubelementLen,
                                P2pSubelement))
    {

        // Parse the power managemenr parameters in here.
        P2pParseNoASubElmt(pAd, P2pSubelement, P2PSubelementLen, Elem->Wcid);
        // Since we get beacon, check if GO enable and OppPS.
        if (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT))
        {
            if(pPort->P2PCfg.bOppsOn == FALSE)
            {
                pPort->P2PCfg.bOppsOn = TRUE;
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_SET_PSM_CFG, NULL, 0);
            }
            
            DBGPRINT(RT_DEBUG_ERROR, ("OpPS ON \n"));
            pPort->P2PCfg.bKeepSlient = FALSE;
            MTDeQueueNoAMgmtPacket(pAd);
            NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
            if (((pPort->P2PCfg.CTWindows&0x7f) > 0) && ((pPort->P2PCfg.CTWindows&0x7f) < 90))
                PlatformSetTimer(pPort, &pPort->P2PCfg.P2pCTWindowTimer, (pPort->P2PCfg.CTWindows&0x7f));
        }
        else if(pPort->P2PCfg.bOppsOn)
        {
            pPort->P2PCfg.bOppsOn = FALSE;
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_SET_PSM_CFG, NULL, 0);
        }
    }

CleanUp:
    if (P2pSubelement)
        PlatformFreeMemory(P2pSubelement,MAX_VIE_LEN);
}


/*  
    ==========================================================================
    Description: 
        Sanity check of the Probe Response frame when operating as a P2P Device. 
        Only called from STA's state machine that is in scanning.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN PeerP2pBeaconSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT ULONG *Peerip,
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement) 
{
    PFRAME_802_11       pFrame;
    PEID_STRUCT         pEid;
    ULONG               Length = 0;
    BOOLEAN             brc = FALSE;
    PUCHAR              Ptr;
    BOOLEAN             bFirstP2pOUI = TRUE;
    
    pFrame = (PFRAME_802_11)Msg;
    Length += LENGTH_802_11;

    *P2PSubelementLen = 0;
    *pSsidLen = 0;
    *Peerip = 0;
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

    Ptr = pFrame->Octet;

    // get timestamp from payload and advance the pointer
    Ptr += TIMESTAMP_LEN;
    Length += TIMESTAMP_LEN;

    // get beacon interval from payload and advance the pointer
    Ptr += 2;
    Length += 2;

    // get capability info from payload and advance the pointer
    Ptr += 2;
    Length += 2;

    pEid = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)    
    {
        switch(pEid->Eid)
        {           
            case IE_SSID:
                if(pEid->Len <= MAX_LEN_OF_SSID)
                {
                    PlatformMoveMemory(Ssid, pEid->Octet, pEid->Len);
                    *pSsidLen = pEid->Len;
                }
                break;
            case IE_VENDOR_SPECIFIC:
                // Check the OUI version, filter out non-standard usage
                if (PlatformEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len >= 4))
                {
                    if (*P2PSubelementLen == 0)
                    {
                        PlatformMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
                        *P2PSubelementLen = pEid->Len +2;
                    }
                    else if (*P2PSubelementLen > 0)
                    {
                        PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
                        *P2PSubelementLen += (pEid->Len+2);
                    }
                }
                else if (PlatformEqualMemory(pEid->Octet, P2POUIBYTE, 4) && (pEid->Len >= 4))
                {
                    // If this is the first P2P OUI. Then also append P2P OUI.
                    // Beacon 's P2P attribute doesn't exceed 256 bytes. So not use acumulcated form.
                    if (bFirstP2pOUI == TRUE)
                    {
                        if (*P2PSubelementLen == 0)
                        {
                            PlatformMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
                            *P2PSubelementLen = (pEid->Len +2);
                            brc = TRUE;
                        }
                        else if (*P2PSubelementLen > 0)
                        {
                            PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
                            *P2PSubelementLen += (pEid->Len+2);
                            brc = TRUE;
                        }
                        bFirstP2pOUI = FALSE;
                    }
                    else
                    {
                        // If this is not the first P2P OUI. Then don't append P2P OUI.
                        // because our parse function doesn't need so many P2P OUI.
                        if ((*P2PSubelementLen > 0) && (pEid->Len > 4))
                        {
                            PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
                            *P2PSubelementLen += (pEid->Len+2);
                            brc = TRUE;
                        }
                    }
                }
                break;
        }
        Length = Length + 2 + pEid->Len;  // Eid[1] + Len[1]+ content[Len]
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    
    }
    return brc;
}

/*  
    ==========================================================================
    Description: 
        Processing Probe Request frame when operating as a P2P Device. 
        Can be called from both as AP's state machine or as STA( that is doing P2P search)'s state machine
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
#if DBG 
VOID PeerP2pProbeReq(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem,
    IN BOOLEAN  bSendRsp,
    ULONG Line)
#else
VOID PeerP2pProbeReq(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem,
    IN BOOLEAN  bSendRsp)
#endif  
{
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    ULONG           P2PSubelementLen, WpsIELen;
    UCHAR           *P2pSubelement;
    UCHAR           *WpsIE;
    NDIS_STATUS   NStatus;
    PUCHAR        pOutBuffer = NULL;
    ULONG         FrameLen = 0;
    UCHAR          Addr2[MAC_ADDR_LEN];
    UCHAR           Ssid[MAX_LEN_OF_SSID] = {0}, SsidLen = 0, DevAddr[MAC_ADDR_LEN], DevType[P2P_DEVICE_TYPE_LEN];
    UCHAR       index;
    BOOLEAN     bNewlyAdd = FALSE;
    UCHAR   NumRequestedType = 0,  RequestedDeviceType[MAX_P2P_FILTER_LIST][P2P_DEVICE_TYPE_LEN];
#if DBG 
    PFRAME_802_11   pFrame;
    PHEADER_802_11   pHeader;

    pFrame = (PFRAME_802_11) Elem->Msg;
    pHeader = &pFrame->Hdr;
#endif

    if (P2P_OFF(pPort))
        return;

#if DBG
    // Got Probe req
    DBGPRINT(RT_DEBUG_INFO, ("%s [%d], Addr1(%x %x %x %x %x %x)Addr2(%x %x %x %x %x %x)Addr3(%x %x %x %x %x %x), LatchRfRegs.Channel(%d)\n", 
                                __FUNCTION__,
                                __LINE__,
                                pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5],
                                pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5],
                                pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5],
                                pAd->HwCfg.LatchRfRegs.Channel));
    
    // Debug for P2pMs listen channel change issue
    if (bSendRsp)
    {
        DBGPRINT(RT_DEBUG_INFO, ("[WFDDBG] %s : ToPort(%d), Last Latch ch(%d),  Listen ch(%d), Line(%d)\n", __FUNCTION__, Elem->PortNum, pAd->HwCfg.LatchRfRegs.Channel, pPort->P2PCfg.ListenChannel, Line));
    }
#endif

    // Only Device port and GO port can send p2p probe response
    if ((pPort->PortType != WFD_DEVICE_PORT) && (pPort->PortType != WFD_GO_PORT)
        && ((pPort->PortType != EXTSTA_PORT) || (pPort->PortSubtype != PORTSUBTYPE_P2PClient)))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : return by PortNum(%d), PortType(%d), PortSubtype(%d) \n", __FUNCTION__, pPort->PortNumber, pPort->PortType, pPort->PortSubtype));
        return;
    }

    // MS P2P: Handle GO's response in func of ApSyncMlmeSyncPeerProbeReqAction
    if (pPort->PortType == WFD_GO_PORT)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : Should not get from GO's port here !\n", __FUNCTION__));
        return;
    }
    // MS P2P: Do not send probe response if device is not discoverable
    if ((pPort->PortType == WFD_DEVICE_PORT) && (pPort->P2PCfg.bAvailability == FALSE))
    {
        DBGPRINT(RT_DEBUG_INFO, ("%s : return by PortNum(%d), PortType(%d), bAvailability(%d)\n",
                                    __FUNCTION__,
                                    pPort->PortNumber,
                                    pPort->PortType,
                                    pPort->P2PCfg.bAvailability));
        return;
    }

    // Ralink P2P: When I am a P2P Client , can't send probe response.
    if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortSubtype == PORTSUBTYPE_P2PClient) && 
        (IS_P2P_CLIENT_OP(pPort)))
        return;

    //Don't send out Probe Response if  I become P2P client after group formation
    if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortSubtype == PORTSUBTYPE_P2PClient) && 
        (pPort->P2PCfg.P2PConnectState <= P2P_DO_GO_SCAN_DONE) && (pPort->P2PCfg.P2PConnectState >= P2P_ANY_IN_FORMATION_AS_CLIENT))
        return;

    P2pSubelement = NULL;
    WpsIE = NULL;
    PlatformAllocateMemory(pAd,  &P2pSubelement, MAX_VIE_LEN);
    PlatformAllocateMemory(pAd,  &WpsIE, MAX_VIE_LEN);

    if ((P2pSubelement == NULL) || (WpsIE == NULL))
        goto CleanUp;
        
    if (PeerP2pProbeReqSanity(pAd, 
                                pPort,
                                Elem->Msg, 
                                Elem->MsgLen, 
                                Addr2, 
                                Ssid, 
                                &SsidLen, 
                                &P2PSubelementLen,
                                P2pSubelement,
                                &WpsIELen,
                                WpsIE))
    {
        PlatformZeroMemory(DevAddr, MAC_ADDR_LEN);
        PlatformZeroMemory(DevType, P2P_DEVICE_TYPE_LEN);
        P2pParseSubElmt(pAd, (PVOID)P2pSubelement, P2PSubelementLen, 
                        FALSE, NULL, NULL, NULL, 0, NULL, DevAddr, NULL, NULL, NULL, NULL, NULL, NULL, DevType, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        // Check if carried requested device types in probe request
        PlatformZeroMemory(RequestedDeviceType, sizeof(RequestedDeviceType));
        if (WpsIELen > 0)
            P2PParseWPSIE(WpsIE, (USHORT)WpsIELen, NULL, NULL, NULL, NULL, NULL, &NumRequestedType, (PUCHAR)&RequestedDeviceType[0][0]);

        if (FALSE == P2PDeviceMatch(pAd, DevAddr, NULL, 0))
            goto CleanUp;
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if (NDIS_WIN8_ABOVE(pAd))
        {
            // TODO: [ndtest_8018] Device address and device types might be appended from P2PAdditionalRequestIEData regardless of  filter scan
            if (0)
            /*if (FALSE == P2pMsDeviceAddrAndTypeMatch(
                            pAd, 
                            pAd->PortList[pPort->P2PCfg.PortNumber],
                            DevAddr,
                            NumRequestedType,
                            (PUCHAR)&RequestedDeviceType[0][0],
                            TRUE, 
                            NULL, 
                            NULL,
                            0, 
                            NULL,
                            P2P_IS_DEVICE,
                            NULL))*/
            {
                goto CleanUp;
            }
        }
#endif

        index = P2pGroupTabSearch(pAd, Addr2);

        if (bSendRsp == TRUE)
        {
            // allocate and send out ProbeRsp frame
            NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
            if (NStatus != NDIS_STATUS_SUCCESS)
                goto CleanUp;

            if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
                P2PMakeProbe(pAd, pPort, Elem, PORTSUBTYPE_P2PGO, Elem->Channel, SUBTYPE_PROBE_RSP, pOutBuffer, &FrameLen);
            else
                P2PMakeProbe(pAd, pPort, Elem, PORTSUBTYPE_P2PClient, Elem->Channel, SUBTYPE_PROBE_RSP, pOutBuffer, &FrameLen);

#if DBG
            // Debug for P2pMs listen channel change issue
            if (bSendRsp)
            {
                pFrame = (PFRAME_802_11) pOutBuffer;
                pHeader = &pFrame->Hdr;
                
                DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s(%d) :PortType(%d) Send ProbeRsp PKT Addr1(%x %x %x %x %x %x), Addr2(%x %x %x %x %x %x), Addr3(%x %x %x %x %x %x), Last Latch ch(%d),  Listen ch(%d), Line(%d)\n", 
                                            __FUNCTION__, 
                                            __LINE__,
                                            pPort->PortType,
                                            pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5],
                                            pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5],
                                            pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5],
                                            pAd->HwCfg.LatchRfRegs.Channel, 
                                            pPort->P2PCfg.ListenChannel, 
                                            Line));
            }
#endif

            if (FrameLen > 0)
                NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
            else if (pOutBuffer != NULL)
                MlmeFreeMemory(pAd, pOutBuffer);
        }
        // Check already in table ?
        // If currently there is no activated P2P profile, we can still check if there is matching peer that is
        // in Persistent table send probe response.  If there is one, maybe we can try to connect to it.
    }
CleanUp:

    if (P2pSubelement)
        PlatformFreeMemory(P2pSubelement, MAX_VIE_LEN);
    if (WpsIE)
        PlatformFreeMemory(WpsIE, MAX_VIE_LEN);
}

    
/*  
    ==========================================================================
    Description: 
        Sanity check of the Probe Request frame when operating as a P2P Device. 
        Can be called from both as AP's state machine or as STA( that is doing P2P search)'s state machine
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN PeerP2pProbeReqSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement, 
    OUT ULONG *WpsIELen, 
    OUT PUCHAR pWpsIE) 
{
    PFRAME_802_11       pFrame;
    PEID_STRUCT             pEid;
    ULONG               Length = 0;
    BOOLEAN             brc = FALSE;
    UCHAR           RateLen;
    BOOLEAN         bOnlybRate = TRUE;
    UCHAR           i;
    
    pFrame = (PFRAME_802_11)Msg;
    Length += LENGTH_802_11;

    *P2PSubelementLen = 0;
    *WpsIELen = 0;
    *pSsidLen = 0;
    RateLen = 0;
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

    pEid = (PEID_STRUCT) pFrame->Octet;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)    
    {
        switch(pEid->Eid)
        {
            case IE_EXT_SUPP_RATES:
                // concatenate all extended rates to Rates[] and RateLen
                RateLen = (RateLen) + pEid->Len;
                for (i = 0; i < RateLen;i++)
                {
                    // Has at least 6Mbps. not 11b-only.  set bOnlybRate to FALSE
                    // 11M 
                    if (((pEid->Octet[i]&0x7f) == 0x2)
                        ||((pEid->Octet[i]&0x7f) == 0x4) 
                        ||((pEid->Octet[i]&0x7f) == 0x16)
                        ||((pEid->Octet[i]&0x7f) == 0xb))
                    {
                    }
                    else
                        bOnlybRate = FALSE;
                }
                break;
            case IE_SUPP_RATES:
                RateLen = (RateLen) + pEid->Len;
                for (i = 0; i < RateLen;i++)
                {
                    if (((pEid->Octet[i]&0x7f) == 0x2)
                        ||((pEid->Octet[i]&0x7f) == 0x4) 
                        ||((pEid->Octet[i]&0x7f) == 0x16)
                        ||((pEid->Octet[i]&0x7f) == 0xb))
                    {
                    }
                    else
                        bOnlybRate = FALSE;
                }
                break;
            case IE_SSID:
                if(pEid->Len <= MAX_LEN_OF_SSID)
                {
                    PlatformMoveMemory(Ssid, pEid->Octet, pEid->Len);
                    *pSsidLen = pEid->Len;
                }
                break;
            case IE_VENDOR_SPECIFIC:
                // Check the OUI version, filter out non-standard usage
                if (PlatformEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len > 4))
                {
                    // might get multiple WSC IE
                    if (pWpsIE != NULL)
                    {
                        if (*WpsIELen == 0)
                        {
                            PlatformMoveMemory(pWpsIE, &pEid->Eid, pEid->Len +2);
                            *WpsIELen = pEid->Len + 2;
                        }
                        else
                        {
                            PlatformMoveMemory(pWpsIE + *WpsIELen, &pEid->Octet + 4, pEid->Len - 4);
                            *WpsIELen += (pEid->Len - 4);
                        }
                    }                   
                }
                if (PlatformEqualMemory(pEid->Octet, P2POUIBYTE, 4) && (pEid->Len > 4))
                {
                    if (*P2PSubelementLen == 0)
                    {
                        if (pP2pSubelement != NULL)
                            PlatformMoveMemory(pP2pSubelement, pEid->Octet, pEid->Len);
                        *P2PSubelementLen = pEid->Len;
                        brc = TRUE;
                    }
                    else if (*P2PSubelementLen > 0)
                    {
                        if (pP2pSubelement != NULL)
                            PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Octet[4], pEid->Len - 4);
                        *P2PSubelementLen += (pEid->Len - 4);
                        brc = TRUE;
                    }

                }
                break;
        }
        Length = Length + 2 + pEid->Len;  // Eid[1] + Len[1]+ content[Len]
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    
    }

    // Doesn't parse probe request that only support 11b. So return FALSE.
    if (bOnlybRate == TRUE)
    {
        DBGPRINT(RT_DEBUG_INFO, ("Ignore Probe Request that is 11b only. from %x %x %x\n", *(pAddr2+3), *(pAddr2+4), *(pAddr2+5)));
        brc = FALSE;
    }
    // Check P2P's Probe Response validatity.
    if (*P2PSubelementLen <= 4)
    {
        brc = FALSE;
    }
    if (!(PlatformEqualMemory(Ssid, &WILDP2PSSID[0], WILDP2PSSIDLEN)))
    {
        brc = FALSE;
        DBGPRINT(RT_DEBUG_INFO, ("Ignore Probe Request that had invalid SSID Name. from %x %x %x \n", *(pAddr2+3), *(pAddr2+4), *(pAddr2+5)));
    }
    
    return brc;
}

/*  
    ==========================================================================
    Description: 
        Sanity check of the Probe Response frame when operating as a P2P Device. 
        Only called from STA's state machine that is in scanning.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN PeerP2pProbeRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT ULONG *Peerip,
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement) 
{
    PFRAME_802_11       pFrame;
    PEID_STRUCT         pEid;
    ULONG               Length = 0;
    BOOLEAN             brc = FALSE;
    PUCHAR              Ptr;
    BOOLEAN             bFirstP2pOUI = TRUE;
    BOOLEAN             bLastIsP2pOUI = FALSE;
    PUCHAR              pP2PIeConLen = NULL;    // pointer to 2 bytes to indicate Contenated length of all P2P IE
    ULONG               P2PIeConLen = 0;    //  Contenated length of all P2P IE
    ULONG           idx;

    pFrame = (PFRAME_802_11)Msg;
    Length = LENGTH_802_11;

    *P2PSubelementLen = 0;
    *pSsidLen = 0;
    *Peerip = 0;
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    
    Ptr = pFrame->Octet;

    // get timestamp from payload and advance the pointer
    Ptr += TIMESTAMP_LEN;
    Length += TIMESTAMP_LEN;

    // get beacon interval from payload and advance the pointer
    Ptr += 2;
    Length += 2;

    // get capability info from payload and advance the pointer
    Ptr += 2;
    Length += 2;

    pEid = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)    
    {
        switch(pEid->Eid)
        {           
            case IE_SSID:
                bLastIsP2pOUI = FALSE;
                if(pEid->Len <= MAX_LEN_OF_SSID)
                {
                    PlatformMoveMemory(Ssid, pEid->Octet, pEid->Len);
                    *pSsidLen = pEid->Len;
                }
                break;
            case IE_VENDOR_SPECIFIC:
                bLastIsP2pOUI = FALSE;
                // Check the OUI version, filter out non-standard usage
                if (PlatformEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len >= 4))
                {
                    if (*P2PSubelementLen == 0)
                    {
                        PlatformMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
                        *P2PSubelementLen = pEid->Len +2;
                    }
                    else if (*P2PSubelementLen > 0)
                    {
                        PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
                        *P2PSubelementLen += (pEid->Len+2);
                    }
                }
                else if (PlatformEqualMemory(pEid->Octet, P2POUIBYTE, 4) && (pEid->Len >= 4))
                {
                    brc = TRUE;
                    bLastIsP2pOUI = TRUE;
                    // If this is the first P2P OUI. Then also append P2P OUI.
                    if (bFirstP2pOUI == TRUE)
                    {
                        // Althought this is first P2P IE.
                        // still need to Check *P2PSubelementLen, because *P2PSubelementLen also includes WPS IE.
                        if (*P2PSubelementLen == 0)
                        {
                            PlatformMoveMemory(pP2pSubelement, &pEid->Eid, 2);
                            *(pP2pSubelement + 2) = 0;
                            // Make one more byte for P2P accumulated length.
                            PlatformMoveMemory(pP2pSubelement + 3, &pEid->Octet[0], pEid->Len);
                            pP2PIeConLen = pP2pSubelement + *P2PSubelementLen + 1;
                            *P2PSubelementLen = (pEid->Len + 3);
                            P2PIeConLen = pEid->Len;    // Real P2P IE length is Len.
                            DBGPRINT(RT_DEBUG_INFO, ("SYNC -1-1 P2PIeConLen  = %d\n", P2PIeConLen));
                        }
                        else if (*P2PSubelementLen > 0)
                        {
                            PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, 2);
                            *(pP2pSubelement + *P2PSubelementLen + 2) = 0;
                            // Make one more byte for P2P accumulated length.
                            PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen + 3, &pEid->Octet[0], pEid->Len);
                            pP2PIeConLen = pP2pSubelement + *P2PSubelementLen + 1;
                            *P2PSubelementLen += (pEid->Len+3);
                            // bFirstP2pOUI is TURE. So use =  
                            P2PIeConLen = pEid->Len;
                            DBGPRINT(RT_DEBUG_INFO, (" -1-2 P2PIeConLen  = %d\n", P2PIeConLen));
                        }
                        bFirstP2pOUI = FALSE;
                    }
                    else if (bLastIsP2pOUI == TRUE)
                    {
                        // If this is not the first P2P OUI. Then don't append P2P OUI.
                        // because our parse function doesn't need so many P2P OUI.
                        if ((*P2PSubelementLen > 0) && (pEid->Len > 4))
                        {
                            PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Octet[4], pEid->Len-4);
                            *P2PSubelementLen += (pEid->Len-4);
                            P2PIeConLen += (pEid->Len - 4);
                        }
                    }
                    else if (bLastIsP2pOUI == FALSE)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Wrong P2P IE !!!!!! P2PIeConLen  = %d\n", P2PIeConLen));
                    }
                }
                break;
            default : 
                bLastIsP2pOUI = FALSE;
                break;

        }
        Length = Length + 2 + pEid->Len;  // Eid[1] + Len[1]+ content[Len]
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    
    }
    if ((P2PIeConLen != 0) && (pP2PIeConLen != NULL))
    {
        *pP2PIeConLen = (UCHAR)(P2PIeConLen%256);
        *(pP2PIeConLen+1) = (UCHAR)(P2PIeConLen/256);
        DBGPRINT(RT_DEBUG_INFO, ("  - 3 P2PIeConLen  = %d. /256 = %d. *P2PSubelementLen = %d \n", P2PIeConLen, (P2PIeConLen/256), *P2PSubelementLen));
        DBGPRINT(RT_DEBUG_INFO, ("  -  %x %x \n", *pP2PIeConLen,  *(pP2PIeConLen+1) ));
        for (idx = 0; idx < (*P2PSubelementLen);)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-", 
                *(pP2pSubelement+idx), *(pP2pSubelement+idx+1), *(pP2pSubelement+idx+2), *(pP2pSubelement+idx+3)
                ,*(pP2pSubelement+idx+4) ,*(pP2pSubelement+idx+5) ,*(pP2pSubelement+idx+6),*(pP2pSubelement+idx+7)
                ,*(pP2pSubelement+idx+8),*(pP2pSubelement+idx+9),*(pP2pSubelement+idx+10),*(pP2pSubelement+idx+11)));
            
             idx = idx + 12;
        }
        
    }

    return brc;
}



/*  
    ==========================================================================
    Description: 
        Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID PeerGASIntialReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    UCHAR   p2pindex;
    UCHAR   Addr2[6];
    PFRAME_802_11   pFrame;
    PRT_P2P_CLIENT_ENTRY    pP2pEntry;
    MLME_P2P_ACTION_STRUCT  P2PActReq;
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    
    pFrame = (PFRAME_802_11)Elem->Msg;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pPeer  GASIntialReqAction = %d, \n", Elem->MsgLen));
    COPY_MAC_ADDR(Addr2, pFrame->Hdr.Addr2);
    DBGPRINT(RT_DEBUG_TRACE, ("From Addr2 = %x %x %x %x %x %x  \n", Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
    p2pindex = P2pGroupTabSearch(pAd, Addr2);
    if (p2pindex == P2P_NOT_FOUND)
    {
        p2pindex = P2pGroupTabInsert(pAd, Addr2, P2PSTATE_DISCOVERY, NULL, 0);
    }
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
        
        P2PParseServiceDiscoReq(pAd, pFrame, &pPort->P2PCfg.ServiceTransac);
        PlatformZeroMemory(&P2PActReq, sizeof(P2PActReq));
        COPY_MAC_ADDR(P2PActReq.Addr, pP2pEntry->addr);
        DBGPRINT(RT_DEBUG_TRACE, (" = %x %x %x %x %x %x  \n",  P2PActReq.Addr[0], P2PActReq.Addr[1],P2PActReq.Addr[2],P2PActReq.Addr[3],P2PActReq.Addr[4],P2PActReq.Addr[5]));
        
        P2PActReq.TabIndex = p2pindex;
        MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_STATE_MACHINE, MT2_MLME_P2P_GAS_INT_RSP, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq);
    }
}


VOID PeerGASIntialRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    UCHAR   p2pindex;
    UCHAR   Addr2[6];
    ULONG   i;
    PUCHAR  pDest;
    PRT_P2P_CLIENT_ENTRY    pP2pEntry;
    PFRAME_802_11   pFrame;
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    
    pFrame = (PFRAME_802_11)Elem->Msg;
    DBGPRINT(RT_DEBUG_TRACE,("P2pPeer  PeerGASIntialRspAction = %d, \n", Elem->MsgLen));
    COPY_MAC_ADDR(Addr2, pFrame->Hdr.Addr2);
    DBGPRINT(RT_DEBUG_TRACE, ("From Addr2 = %x %x %x %x %x %x  \n", Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
    pDest = &Elem->Msg[0];
    for (i = 0; i <Elem->MsgLen; )
    {
        DBGPRINT(RT_DEBUG_INFO,(": %x %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2),
        *(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8), *(pDest+i+9)));
        i = i + 10;
    }
    p2pindex = P2pGroupTabSearch(pAd, Addr2);
    if (p2pindex == P2P_NOT_FOUND)
    {
        p2pindex = P2pGroupTabInsert(pAd, Addr2, P2PSTATE_DISCOVERY, NULL, 0);
    }
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
        
        P2PParseServiceDiscoRsp(pAd, pPort, pP2pEntry, p2pindex, pFrame, pPort->P2PCfg.ServiceTransac);
    }
}

/*  
    ==========================================================================
    Description: 
        Publiac action frame. But with ACtion is GAS_COMEBACK_REQ (11).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID PeerGASComebackReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    UCHAR   p2pindex;
    UCHAR   Addr2[6];
    PRT_P2P_CLIENT_ENTRY    pP2pEntry;
    PFRAME_802_11   pFrame;
    MLME_P2P_ACTION_STRUCT  P2PActReq; 
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    
    pFrame = (PFRAME_802_11)Elem->Msg;
    DBGPRINT(RT_DEBUG_TRACE,("p2p  PeerGASComebackReqAction = %d, \n", Elem->MsgLen));

    COPY_MAC_ADDR(Addr2, pFrame->Hdr.Addr2);
    DBGPRINT(RT_DEBUG_TRACE, ("From Addr2 = %x %x %x %x %x %x  \n", Addr2[0], Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
    p2pindex = P2pGroupTabSearch(pAd, Addr2);
    if (p2pindex == P2P_NOT_FOUND)
    {
        p2pindex = P2pGroupTabInsert(pAd, Addr2, P2PSTATE_DISCOVERY, NULL, 0);
    }
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
        
        if (TRUE == P2PParseComebackReq(pAd, pFrame, &pPort->P2PCfg.ServiceTransac))
        {
            PlatformZeroMemory(&P2PActReq, sizeof(P2PActReq));
            COPY_MAC_ADDR(P2PActReq.Addr, pP2pEntry->addr);
            DBGPRINT(RT_DEBUG_TRACE, (" = %x %x %x %x %x %x  \n",  P2PActReq.Addr[0], P2PActReq.Addr[1],P2PActReq.Addr[2],P2PActReq.Addr[3],P2PActReq.Addr[4],P2PActReq.Addr[5]));
            
            P2PActReq.TabIndex = p2pindex;
            MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_STATE_MACHINE, MT2_MLME_P2P_GAS_CB_RSP, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq);
        }
    }

}

/*  
    ==========================================================================
    Description: 
        Publiac action frame. But with ACtion is GAS_COMEBACK_Rsp(11).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID PeerGASComebackRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PFRAME_802_11   pFrame;
    UCHAR   p2pindex;
    UCHAR   Addr2[6];
    PRT_P2P_CLIENT_ENTRY    pP2pEntry;
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    
    pFrame = (PFRAME_802_11)Elem->Msg;
    COPY_MAC_ADDR(Addr2, pFrame->Hdr.Addr2);
    p2pindex = P2pGroupTabSearch(pAd, Addr2);

    DBGPRINT(RT_DEBUG_TRACE,("p2p  PeerGASComebackRspAction = %d, \n", Elem->MsgLen));
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
        
        if (TRUE == P2PParseComebackRsp(pAd, pP2pEntry, pFrame, &pPort->P2PCfg.ServiceTransac))
            pPort->P2PCfg.p2pidxForServiceCbReq = p2pindex;
    }
}


/*
    ========================================================================
    
    Routine Description:
        Check REinvoke's invitation Request frame .

    Arguments:
            - NIC Adapter pointer
        
    Return Value:
        FALSE - None of channel in ChannelList Match any channel in pAd->HwCfg.ChannelList[] array

    IRQL = DISPATCH_LEVEL
    
    Note:
    ========================================================================

*/
VOID P2pCheckInviteReq(
    IN PMP_ADAPTER pAd,
    IN BOOLEAN      bIAmGO,
    IN UCHAR        index,
    IN PUCHAR   ChannelList,
    IN PUCHAR   BssidAddr,
    IN UCHAR        OpChannel,
    IN PUCHAR   Ssid,
    IN UCHAR    SsidLen,
    IN UCHAR    *pRspStatus)
{
    
    *pRspStatus = P2PSTATUS_SUCCESS;
    // Check if have Common Channels.
    if (FALSE == P2pCheckChannelList(pAd, ChannelList))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - no common channel = %d...\n", *ChannelList));
        *pRspStatus = P2PSTATUS_NO_CHANNEL;
        return;
    }
    // Invite Req from a CLient doesn't includes group_bssid in the request. So doesn't need check.
    // Check Bssid is correct.
    if (!PlatformEqualMemory(BssidAddr, pAd->pP2pCtrll->P2PTable.PerstEntry[index].Addr, MAC_ADDR_LEN)
        && (bIAmGO == FALSE))
    {
        *pRspStatus = P2PSTATUS_INVALID_PARM;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - MAc addr invalid  .\n"));
        return;
    }

    // invite Req from Owner include OpChannel. from Client, doesn't include.
    /*if (bIAmGO == FALSE)
    {
        // Check Oopchannel is correct.
        for (i = 0;i < pAd->HwCfg.ChannelListNum;i++)
        {
            if (pAd->HwCfg.ChannelList[i].Channel == OpChannel)
            {
                break;
            }
        }
        if ( i == pAd->HwCfg.ChannelListNum)
        {
            *pRspStatus = P2PSTATUS_NO_CHANNEL;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - 2 P2PSTATUS_NO_CHANNEL  .\n"));
            return;
        }
    }*/
    // Check SSID is correct.
    if ((SsidLen > 0) && (!PlatformEqualMemory(pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid, Ssid, SsidLen)))
    {
    DBGPRINT(RT_DEBUG_TRACE, ("Ssid1 = %c%c%c%c%c%c%c%c%c%c%c%c%c%c  \n",  Ssid[0], Ssid[1],Ssid[2],Ssid[3],Ssid[4],Ssid[5],Ssid[6],Ssid[7],Ssid[8],Ssid[9],Ssid[10],Ssid[11],Ssid[12],Ssid[13]));
    DBGPRINT(RT_DEBUG_TRACE, ("Ssid2 = %c%c%c%c%c%c%c%c%c%c%c%c%c%c  \n",  
        pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[0], pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[1],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[2],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[3],
        pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[4],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[5],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[6],
        pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[7],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[8],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[9],
        pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[10],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[11],pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[12],
        pAd->pP2pCtrll->P2PTable.PerstEntry[index].Profile.SSID.Ssid[13]));
        *pRspStatus = P2PSTATUS_INVALID_PARM;
        return;
    }
}



/*
    ========================================================================
    
    Routine Description:
        Check REinvoke's invitation Request frame .

    Arguments:
            - NIC Adapter pointer
        
    Return Value:
        FALSE - None of channel in ChannelList Match any channel in pAd->HwCfg.ChannelList[] array

    IRQL = DISPATCH_LEVEL
    
    Note:
    ========================================================================

*/
VOID P2pCheckInviteReqFromExisting(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   ChannelList,
    IN PUCHAR   BssidAddr,
    IN UCHAR        OpChannel,
    IN PUCHAR   Ssid,
    IN UCHAR    SsidLen,
    IN UCHAR    *pRspStatus)
{
    UCHAR       i;
    *pRspStatus = P2PSTATUS_SUCCESS;
    // Check if have Common Channels.
    if (FALSE == P2pCheckChannelList(pAd, ChannelList))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - no common channel. Channel list is : %d...\n", ChannelList[0]));
        *pRspStatus = P2PSTATUS_NO_CHANNEL;
        return;
    }

    // invite Req from Owner include OpChannel. from Client, doesn't include.
    if (OpChannel != 0) 
    {
        // Check Oopchannel is correct.
        for (i = 0;i < pAd->HwCfg.ChannelListNum;i++)
        {
            if (pAd->HwCfg.ChannelList[i].Channel == OpChannel)
            {
                break;
            }
        }
        if ( i == pAd->HwCfg.ChannelListNum)
        {
            *pRspStatus = P2PSTATUS_NO_CHANNEL;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - No channel = %d...\n", OpChannel));
            return;
        }
    }
    // Check SSID is correct.
    if ((SsidLen == 0))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Ssidlen Zero = %d...\n", SsidLen));
        *pRspStatus = P2PSTATUS_INVALID_PARM;
        return;
    }
}
    
/*  
    ==========================================================================
    Description: 
        Go PeerDisassocReq Action.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID GoPeerDisassocReq(
    IN PMP_ADAPTER pAd,
    IN PMAC_TABLE_ENTRY pEntry,
    IN PUCHAR   Addr2)
{
    BOOLEAN     Cancelled = FALSE;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("GoPeerDisassocReq - 1 receive DIS-ASSOC request \n"));

    if (pEntry != NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("GoPeerDisassocReq - Aid = %d.   = State = %d \n", pEntry->Aid, pEntry->P2pInfo.P2pClientState));
        // If this peer is in operating mode, reset this peer's state
        if ((pEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_ASSOC)
            ||(pEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_OPERATING)
            ||(pEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_PSK)
            || (pEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_WPS)
            || (pEntry->P2pInfo.P2pClientState == P2PSTATE_NONE))
        {
            // Set to a temporary state.  If thie device connect within 25 seconds. he may use WPS to connect.
            P2pGroupTabDelete(pAd, P2P_NOT_FOUND, pEntry->Addr);

            // Don't Stop GO immediately. Give some time for this client to reconnect with 5 seconds. 
            // when StopGo timer expired, 
            DBGPRINT(RT_DEBUG_TRACE, (" have entry GoPeerDisassocReq: P2pStopGoTimer -  5 sec\n"));
            PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
            PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopGoTimer, 1000);
        }
        pAd->StaCfg.WpaState = SS_NOTUSE;
    }
    else
    {
        P2pGroupTabDelete(pAd, P2P_NOT_FOUND, Addr2);
        DBGPRINT(RT_DEBUG_TRACE, ("no entry GoPeerDisassocReq: Check P2pStopGoTimer after 5 sec \n"));
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
        PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopGoTimer, 1000);
        
    }

}


VOID GOUpdateBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort) 
{
    PTXWI_STRUC    pTxWI = &pAd->pTxCfg->BeaconTxWI;
    PUCHAR        pBeaconFrame = pAd->pTxCfg->BeaconBuf;
    UCHAR  *ptr;
    ULONG FrameLen = pPort->SoftAP.ApCfg.TimIELocationInBeacon;
    ULONG UpdatePos = pPort->SoftAP.ApCfg.CapabilityInfoLocationInBeacon;
    ULONG longptr;
    
    UCHAR byte0 = (UCHAR)(pPort->SoftAP.ApCfg.TimBitmap & 0x000000fe);  // skip AID#0
    UCHAR byte1 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0x0000ff00) >> 8);
    UCHAR byte2 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0x00ff0000) >> 16);
    UCHAR byte3 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0xff000000) >> 24);
    UCHAR byte4 = (UCHAR)(pPort->SoftAP.ApCfg.TimBitmap2 & 0x000000ff);
    UCHAR byte5 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0x0000ff00) >> 8);
    UCHAR byte6 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0x00ff0000) >> 16);
    UCHAR byte7 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0xff000000) >> 24);
    UINT  i;
    UCHAR   P2pIEFixed[6] = {0xdd, 0x12, 0x50, 0x6f, 0x9a, 0x09};   // length will modify later
    ULONG   TmpLen;
    ULONG   LastWscLen;

    if ((pPort == NULL) || (pPort->PortSubtype != PORTSUBTYPE_P2PGO) || (pPort->P2PCfg.bGOStart == FALSE))
        return;

    if (pPort->HwBeaconBase == 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("beacon base should not be 0000000000000000"));
        return;
    }
    
    //
    // step 1 - update BEACON's Capability
    //
    ptr = pBeaconFrame + pPort->SoftAP.ApCfg.CapabilityInfoLocationInBeacon;
    *ptr = (UCHAR)(pPort->SoftAP.ApCfg.CapabilityInfo & 0x00ff);
    *(ptr+1) = (UCHAR)((pPort->SoftAP.ApCfg.CapabilityInfo & 0xff00) >> 8);

    //
    // step 2 - update TIM IE
    // TODO: enlarge TIM bitmap to support up to 64 STAs
    // TODO: re-measure if RT2600 TBTT interrupt happens faster than BEACON sent out time
    //
    if (pPort->SoftAP.ApCfg.DtimCount == 0)
        pPort->SoftAP.ApCfg.DtimCount = pPort->SoftAP.ApCfg.DtimPeriod - 1;
    else
        pPort->SoftAP.ApCfg.DtimCount -= 1;

    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only support 11b, not to include TIM
    }
    else
    {
        ptr = pBeaconFrame + pPort->SoftAP.ApCfg.TimIELocationInBeacon;
        *ptr = IE_TIM;
        *(ptr + 2) = pPort->SoftAP.ApCfg.DtimCount;
        *(ptr + 3) = pPort->SoftAP.ApCfg.DtimPeriod;

        if (byte0 || byte1) // there's some backlog frame for AID 1-15
        {
            *(ptr + 4) = 0;      // Virtual TIM bitmap stars from AID #0
            *(ptr + 5) = byte0;
            *(ptr + 6) = byte1;
            *(ptr + 7) = byte2;
            *(ptr + 8) = byte3;
            *(ptr + 9) = byte4;
            *(ptr + 10) = byte5;
            *(ptr + 11) = byte6;
            *(ptr + 12) = byte7;
            if (byte7)      *(ptr + 1) = 11; // IE length
            else if (byte6) *(ptr + 1) = 10; // IE length
            else if (byte5) *(ptr + 1) = 9;  // IE length
            else if (byte4) *(ptr + 1) = 8;  // IE length
            else if (byte3) *(ptr + 1) = 7;  // IE length
            else if (byte2) *(ptr + 1) = 6;  // IE length
            else if (byte1) *(ptr + 1) = 5;  // IE length
            else            *(ptr + 1) = 4;  // IE length
        }
        else if (byte2 || byte3) // there's some backlogged frame for AID 16-31
        {
            *(ptr + 4) = 2;      // Virtual TIM bitmap starts from AID #16
            *(ptr + 5) = byte2;
            *(ptr + 6) = byte3;
            *(ptr + 7) = byte4;
            *(ptr + 8) = byte5;
            *(ptr + 9) = byte6;
            *(ptr + 10) = byte7;
            if (byte7)      *(ptr + 1) = 9; // IE length
            else if (byte6) *(ptr + 1) = 8; // IE length
            else if (byte5) *(ptr + 1) = 7; // IE length
            else if (byte4) *(ptr + 1) = 6; // IE length
            else if (byte3) *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else if (byte4 || byte5) // there's some backlogged frame for AID 32-47
        {
            *(ptr + 4) = 4;      // Virtual TIM bitmap starts from AID #32
            *(ptr + 5) = byte4;
            *(ptr + 6) = byte5;
            *(ptr + 7) = byte6;
            *(ptr + 8) = byte7;
            if (byte7)      *(ptr + 1) = 7; // IE length
            else if (byte6) *(ptr + 1) = 6; // IE length
            else if (byte5) *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else if (byte6 || byte7) // there's some backlogged frame for AID 48-63
        {
            *(ptr + 4) = 6;      // Virtual TIM bitmap starts from AID #48
            *(ptr + 5) = byte6;
            *(ptr + 6) = byte7;
            if (byte7)      *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else // no backlogged frames
        {
            *(ptr + 1) = 4; // IE length
            *(ptr + 4) = 0;
            *(ptr + 5) = 0;
        }

        // bit0 means backlogged mcast/bcast
        *(ptr + 4) |= (pPort->SoftAP.ApCfg.TimBitmap & 0x01); 

        // adjust BEACON length according to the new TIM
        FrameLen += (2 + *(ptr+1)); 
    }
    //
    // fill up Channel Switch Announcement Element
    //
    if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11A) && (pPort->CommonCfg.bIEEE80211H == 1) && (pPort->CommonCfg.RadarDetect.RDMode == RD_SWITCHING_MODE))
    {
        ptr = pBeaconFrame + FrameLen;
        *ptr = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
        *(ptr + 1) = 3;
        *(ptr + 2) = 1;
        *(ptr + 3) = pPort->Channel;
        *(ptr + 4) = pPort->CommonCfg.RadarDetect.CSCount;
        ptr      += 5;
        FrameLen += 5;
    }
    
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only support 11b, not to include ERP
        ;
    }
    else
    {
        // Update ERP and extended tx rate
        if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11BG_MIXED) || (pPort->SoftAP.ApCfg.PhyMode == PHY_11G) || (pPort->SoftAP.ApCfg.PhyMode == PHY_11BGN_MIXED))
        {
            //
            // fill ERP IE
            // 
            ptr = pBeaconFrame + FrameLen;
            *ptr = IE_ERP;
            *(ptr + 1) = 1;
            *(ptr + 2) = pPort->SoftAP.ApCfg.ErpIeContent;
            ptr      += 3;
            FrameLen += 3;
        }
    }

    //
    // step 5. Update HT. Since some fields might change in the same BSS.
    //
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)) 
    {
        //CH14 only supports 11B and will not carry any HT info on beacons
        ;
    }
    else
    {
        if (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY)
        {
            ULONG HtLen, HtLen1;

            // add HT Capability IE 
            HtLen = sizeof(pPort->SoftAP.ApCfg.HtCapability);
            HtLen1 = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe);
            MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                                      1,                                &HtCapIe,
                                      1,                                &HtLen,
                                     HtLen,          &pPort->SoftAP.ApCfg.HtCapability, 
                                      1,                                &AddHtInfoIe,
                                      1,                                &HtLen1,
                                     HtLen1,          &pPort->SoftAP.ApCfg.AddHTInfoIe, 
                              END_OF_ARGS);
            FrameLen += TmpLen;
        }
    }

    // Append WSC IE
    //
    LastWscLen = pPort->P2PCfg.GOBeaconBufWscLen;

    if ((pPort->PortType == WFD_GO_PORT) && (pPort->PortCfg.AdditionalBeaconIESize > 0) && (pPort->PortCfg.AdditionalBeaconIEData != NULL))
    {
        MakeOutgoingFrame(pBeaconFrame + FrameLen,      &TmpLen,
                            pPort->PortCfg.AdditionalBeaconIESize,  pPort->PortCfg.AdditionalBeaconIEData,
                            END_OF_ARGS);
    }
    /*else if ((pPort->PortType == WFD_GO_PORT) && (IS_P2P_MS_DEV(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]))
        && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIESize > 0) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIEData != NULL))
    {
        MakeOutgoingFrame(pBeaconFrame + FrameLen,      &TmpLen,
                            pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIESize,  pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalBeaconIEData,
                            END_OF_ARGS);
    }*/
    else
    {
        P2pMakeBeaconWSCIE(pAd, pPort, pBeaconFrame + FrameLen, &TmpLen);
    }

    pPort->P2PCfg.GOBeaconBufWscLen = TmpLen; 
    if (LastWscLen != pPort->P2PCfg.GOBeaconBufWscLen)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE,  (" WSCIE Length Changing = %d ", TmpLen));
        DumpFrameMessage(&pAd->pTxCfg->BeaconBuf[FrameLen], TmpLen, ("WSCIE in beacon\n"));
    }
    // save wsc ie length
    FrameLen += TmpLen;

    // ========================================>

    // Save P2P IE Lenght offset.
    PlatformMoveMemory(pBeaconFrame + FrameLen, &P2pIEFixed[0], 6);
    FrameLen += 6;
    pPort->P2PCfg.P2pBcnOffset.P2pCapOffset = FrameLen + 3;
    TmpLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, (pPort->PortType == WFD_GO_PORT) ? (pPort->PortCfg.P2pCapability) : (pPort->P2PCfg.P2pCapability), pBeaconFrame + FrameLen);
    FrameLen += TmpLen;
    // Device ID in device port is mandatory in beacon
    TmpLen = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_ID, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pBeaconFrame + FrameLen);
    FrameLen += TmpLen;

    pAd->pTxCfg->BeaconBufLen = FrameLen;

    //
    // step 6. Since FrameLen may change, update TXWI.
    //
    // Update in real buffer
    // Update sw copy.  

    WRITE_TXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pTxWI, FrameLen);
    WRITE_TXWI_PHY_MODE(pAd, pTxWI, MODE_OFDM);
    
    //
    // step 6. move BEACON TXWI and frame content to on-chip memory
    //
    ptr = (PUCHAR)&pAd->pTxCfg->BeaconTxWI;
    for (i=0; i<pAd->HwCfg.TXWI_Length; )  // 24-byte TXINFO field
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + i, longptr);   //Beacon buffer of AP is always in 0x7900.
        ptr += 4;
        i+=4;
    }   

    ptr = pAd->pTxCfg->BeaconBuf;
    if (FrameLen > UpdatePos)
    {
        for (i = 0; i < FrameLen; )
        {
            longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + pAd->HwCfg.TXWI_Length + i, longptr);
            ptr += 4;
            i+=4;
        }
    }

    if (IS_P2P_GO_OP(pPort) || 
        (IS_P2P_AUTOGO(pPort)) ||
        (IS_P2P_MS_GO(pAd, pPort) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)))
    {
        P2pUpdateNoABeacon(pAd, FrameLen);
    }
}


UCHAR ChannelToClass(
    IN UCHAR        Channel,
    IN UCHAR        Country)
{
    UCHAR       ReturnClass = 1;
    if (Country == COUNTRY_GLOBAL)
    {
        // 0. Decide current regulatory class. P802.11REVmb_D3.0.pdf. TableJ-4
        if (Channel <= 11)
        {
            ReturnClass = 81;
        }
        else if (Channel == 14)
        {
            ReturnClass = 82;   // 
        }           
        else if (Channel <= 48)
        {
            ReturnClass = 115;  // 
        }           
        else if (Channel <= 64)
        {
            ReturnClass = 118;
        }
        else if (Channel  <= 140)
        {
            ReturnClass = 121;
        }
        else if ((Channel  == 165) || (Channel  == 169))
        {
            ReturnClass = 125;
        }
        else
        {
            // 3  when channels are 149.153. 157. 161
            ReturnClass = 124;
        }
    }   
    else if (Country == COUNTRY_USA)
    {
        // 0. Decide current regulatory class. P802.11REVmb_D3.0.pdf. TableJ-4
        if (Channel <= 11)
        {
            ReturnClass = 81;
        }
        else if (Channel <= 48)
        {
            ReturnClass = 115;  // 
        }           
        else if (Channel <= 64)
        {
            ReturnClass = 118;
        }
        else if (Channel  <= 140)
        {
            ReturnClass = 121;
        }
        else if ((Channel  == 165) || (Channel  == 169))
        {
            ReturnClass = 125;
        }
        else
        {
            // 3  when channels are 149.153. 157. 161
            ReturnClass = 124;
        }
    }
    return ReturnClass;
}


