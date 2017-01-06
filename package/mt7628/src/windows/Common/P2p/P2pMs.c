/*
 ***************************************************************************
 * Ralink Technology, Corp.
 * 5F, No.5, Tai-Yuan 1st Street
 * Jhubei City, Hsinchu County 30265
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2011, Ralink Technology, Corp.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
        ms_p2p.c

    Abstract:
        Wi-Fi Direct implementation based on Microsoft Windows 8 WLAN OID
        Specification
*/

/* INCLUDE FILES */
#include "MtConfig.h"

#define DRIVER_FILE         0x00F00000

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  

/*
    ==========================================================================
    Description:
        Be called when driver is ready in Win8 WFD.

    Parameters:

    Note:

    ==========================================================================
 */
VOID P2pMsEnable(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort
    )
{
#if 1
    UCHAR       module, i, j;
    UCHAR       ListenArray[3];
    UCHAR       RanVar[32];
    US_CYC_CNT_STRUC    UsCycCnt = {0};
    PBF_CFG_STRUC_EXT       PbfCfg = {0};

    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]", __FUNCTION__, pPort->PortNumber));

    pPort->P2PCfg.P2PMode = P2P_MODE_WIN8;

#ifdef MULTI_CHANNEL_SUPPORT
    pAd->MccCfg.MultiChannelEnable = FALSE;
    pAd->pP2pCtrll->MultiChannelStartOrEndProcess = 0;
    pPort->P2PCfg.STAHwQSEL = FIFO_EDCA;
    pPort->P2PCfg.CLIHwQSEL = FIFO_EDCA;
    pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
    pPort->P2PCfg.SwitchChTime = 300;     // default 400 mse
    pPort->P2PCfg.PwrSaveDelayTime = 20;  // unit: msec, default 20 ms
    pAd->MccCfg.SwQSelect = NUM_OF_TX_RING;//QID_AC_BE;
    pAd->MccCfg.PendedDisconnectOidRequest = NULL;
    pAd->MccCfg.PendedGoOidRequest = NULL;
    pAd->MccCfg.PendedResetRequest = NULL;
    pAd->pNicCfg->PendedScanRequest = NULL;
    pAd->MccCfg.pDisconnectPort = NULL;
    pAd->MccCfg.pGoPort = NULL;
    pAd->MccCfg.pResetRequestPort = NULL;
    pAd->MccCfg.pScanRequestPort = NULL;
    pAd->MccCfg.TxOp = IFS_HTTXOP;
    // 40M
    // Central channel
    pPort->P2PCfg.InfraCentralChannel = CHANNEL_OFF;
    pPort->P2PCfg.P2PCentralChannel = CHANNEL_OFF;
    pPort->P2PCfg.P2PGOCentralChannel = CHANNEL_OFF;
    // Seconday channel offset (1: above primary ch, 3: below primary ch, 0xff: deault)
    pPort->P2PCfg.InfraExtChanOffset = EXTCHA_NONE;
    pPort->P2PCfg.P2PExtChanOffset = EXTCHA_NONE;
    pPort->P2PCfg.P2PGOExtChanOffset = EXTCHA_NONE;

    // Use to control MlmeDynamicTxRateSwitching (23->1: ok) (1->23: NG)
    pPort->P2PCfg.CurrentWcid = RESERVED_WCID;

    RTUSBReadMACRegister(pAd, USB_CYC_CFG, &UsCycCnt.word);
    UsCycCnt.field.UsCycCnt = 0;
    RTUSBWriteMACRegister(pAd, USB_CYC_CFG, UsCycCnt.word);

   // RTUSBReadMACRegister(pAd, PBF_CFG(pAd), &PbfCfg.word);
    //WRITE_PBF_CFG_TX2QENABLE(pAd, &PbfCfg, 1);
    //WRITE_PBF_CFG_TX1QENABLE(pAd, &PbfCfg, 0);
   // WRITE_PBF_CFG_TX0QENABLE(pAd, &PbfCfg, 0);
    //PbfCfg.field.Tx2QEnable = 1;      // EDCA
    //PbfCfg.field.Tx1QEnable = 0;      // HCCA
    //PbfCfg.field.Tx0QEnable = 0;      // Mgt
   // RTUSBWriteMACRegister(pAd, PBF_CFG(pAd), PbfCfg.word);

    NdisGetCurrentSystemTime((PLARGE_INTEGER)&pPort->P2PCfg.LastFlushBssTime);
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&pPort->P2PCfg.LastEnumBssTime);
#endif /*MULTI_CHANNEL_SUPPORT*/

#ifdef WFD_NEW_PUBLIC_ACTION
    pPort->P2PCfg.PrevState = P2P_NULL_STATE;
    pPort->P2PCfg.CurrState = P2P_NULL_STATE;
    pPort->P2PCfg.Signal = P2P_NULL_STATE;
    pPort->P2PCfg.LostAckRetryCounter = 0;
    pPort->P2PCfg.LostRspRetryCounter = 0;
    pPort->P2PCfg.pP2PPublicActionFrameData = NULL;
    pPort->P2PCfg.P2PPublicActionFrameDataLen = 0;
    pPort->P2PCfg.PrevSentFrame = (P2P_PROVISION_RSP + 1);
#endif /*WFD_NEW_PUBLIC_ACTION*/

    // ==============>>
    // Init parameters with WIN8 interactions
    pPort->P2PCfg.bAvailability = FALSE;
    pPort->P2PCfg.DeviceDiscoverable = DOT11_WFD_DEVICE_AUTO_AVAILABILITY;
    pPort->P2PCfg.WFDStatus = P2PSTATUS_SUCCESS;

    pPort->P2PCfg.MSDialogToken = 1;
    pPort->P2PCfg.WaitForMSCompletedFlags = P2pMs_WAIT_COMPLETE_NOUSE;

    // Handle ready to connection on client side.
    pPort->P2PCfg.InGroupFormation = FALSE;
    pPort->P2PCfg.WaitForWPSReady = FALSE;
    pPort->P2PCfg.ReadyToConnect = TRUE;

    pPort->P2PCfg.WFDProprietaryIEsLen = 0;
    PlatformZeroMemory(pPort->P2PCfg.WFDProprietaryIEs, sizeof(pPort->P2PCfg.WFDProprietaryIEs));
    pPort->P2PCfg.IsValidProprietaryIE = FALSE;

    // <<==============

    // Test bits
    // If you want to randomize your listen channel, please turn on the following flag
    pPort->P2PCfg.RandomListenChTestBitOn = FALSE;
    pPort->P2PCfg.SwitchChannelInProgress = FALSE;
    pPort->P2PCfg.RestoreChannelInProgress = FALSE;

    //
    // WPS
    //

    // Driver hard fixed to PC.
    PlatformMoveMemory(pPort->P2PCfg.PriDeviceType, Wsc_Pri_Dev_Type, 8);
    // Disable WscConfig
    pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_DISABLE;
    // init wps supported version from registry
    if (pAd->StaCfg.WSCVersion2 >= 0x20)
        pPort->P2PCfg.WPSVersionsEnabled = 2;
    else
        pPort->P2PCfg.WPSVersionsEnabled = 1;

    //
    // Counters set to specific values to aviod to trigger
    //
    pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD;
    pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = WFD_BLOCKING_SCAN_TIMEOUT;
    pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = WFD_BLOCKING_SCAN_TIMEOUT;
    pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = WFD_COMPLETE_SEND_TIMEOUT;
    pPort->P2PCfg.P2pCounter.SwitchingChannelCounter = WFD_COMPLETE_SEND_TIMEOUT;
    pPort->P2PCfg.P2pCounter.StartRetryCounter = 300;
    pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail = 0xffffffff;
    pPort->P2PCfg.P2pCounter.CounterP2PAcceptReinvoke = 0xff;
    pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 10;

    // Construct AP RSN_IE with WPA2PSK-AES. (regardless of pPort parameter)
    RTMPAPMakeRSNIE(pAd, pAd->PortList[PORT_0], Ralink802_11AuthModeWPA2PSK, Ralink802_11Encryption3Enabled);

    pPort->P2PCfg.ConfigTimeout[0] = 40;
    pPort->P2PCfg.ConfigTimeout[1] = 30;

    // init discovery table
    P2pDiscoTabInit(pAd);

    // others
    pPort->P2PCfg.bGroupsScanOnce = FALSE;
    pPort->P2PCfg.ClientUseBitmap = 0;
    pPort->P2PCfg.GoIOpChannel = 0;
    PlatformZeroMemory(&pPort->P2PCfg.GoNOpChannel, sizeof(DOT11_WFD_CHANNEL));

    //
    // UPDATE STATE
    //

    // init states used in Win8
    pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
    pAd->pP2pCtrll->P2pMsConnectedStatus = P2pMs_CONNECTED_STATUS_OFF;
    pPort->P2PCfg.DiscoverTriggerType = P2pMs_DISCOVER_BY_OS;


    // DiscoProvState goes to Idle State
    P2pGotoIdle(pAd, pPort);


    //
    // Select my listen channel
    //

    if (MlmeSyncIsValidChannel(pAd, P2P_DEFAULT_LISTEN_CHANNEL))
        pPort->P2PCfg.ListenChannel = P2P_DEFAULT_LISTEN_CHANNEL;
    else if ((P2P_DEFAULT_LISTEN_CHANNEL != 1) && MlmeSyncIsValidChannel(pAd, 1))
        pPort->P2PCfg.ListenChannel = 1;
    else if ((P2P_DEFAULT_LISTEN_CHANNEL != 11) && MlmeSyncIsValidChannel(pAd, 11))
        pPort->P2PCfg.ListenChannel = 11;
    else if ((P2P_DEFAULT_LISTEN_CHANNEL != 6) && MlmeSyncIsValidChannel(pAd, 6))
        pPort->P2PCfg.ListenChannel = 6;
    else
        pPort->P2PCfg.ListenChannel = P2P_DEFAULT_LISTEN_CHANNEL;

    // randomize my listen channel
    if ((pPort->P2PCfg.RandomListenChTestBitOn == TRUE) && (pAd->HwCfg.ChannelListNum > 0))
    {
        ListenArray[0] = 1;
        ListenArray[1] = 6;
        ListenArray[2] = 11;

        j = 0;
        // Add 1, 6, 11 to ListenArray Array.
        for (i = 0;i < pAd->HwCfg.ChannelListNum;i++)
        {
            if (IS_SOCIAL_CHANNEL(pAd->HwCfg.ChannelList[i].Channel))
            {
                ListenArray[j] = pAd->HwCfg.ChannelList[i].Channel;
                j++;
            }
        }
        DBGPRINT(RT_DEBUG_TRACE, ("%s: how many social channels are in our supported channel ? Ans : %d !!! \n", __FUNCTION__, j));

        // generate 32-byte random byte
        GenRandom(pAd, RanVar);

        if ( j > 0)
        {
            //module = RandomByte(pAd)%j;
            module = RanVar[16]%j;

            if (module < 3)
                pPort->P2PCfg.ListenChannel = ListenArray[module];
            DBGPRINT(RT_DEBUG_TRACE, ("1: pPort->P2PCfg.ListenChannel: %d !!! \n", pPort->P2PCfg.ListenChannel));
        }
        else
        {
            //module = RandomByte(pAd)%3;
            module = RanVar[16]%3;

            if (module < 3)
                pPort->P2PCfg.ListenChannel = ListenArray[module];
            DBGPRINT(RT_DEBUG_TRACE, ("2: pPort->P2PCfg.ListenChannel: %d !!! \n", pPort->P2PCfg.ListenChannel));
        }
    }

    pAd->pP2pCtrll->pWfdDevicePort = pPort;

    P2PMSSyncListenChannelToPortChannel(pAd, pPort);

        

// 2011-11-29 For P2pMs multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
    pPort->P2PCfg.P2PChannel = pPort->P2PCfg.ListenChannel;
    pPort->P2PCfg.InfraChannel = pPort->P2PCfg.ListenChannel; 
#endif /* MULTI_CHANNEL_SUPPORT */

    FUNC_LEAVE;
#endif
}

/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon completion of P2P Discovery along
        with a list of devices

    Arguments:
        pAd                     Pointer to our adapter
        pPort                   Pointer to operating port

    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateDiscoverComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort)
{
    PDOT11_WFD_DISCOVER_COMPLETE_PARAMETERS pDiscoCompleteParam;
    PUCHAR      pBuffer;
    USHORT      BufferLength = 0;
    UCHAR       cliIdx;
    ULONG       BytesNeeded, BytesWritten, TotalBytesWritten;
    UCHAR       TotalDeviceNumber = 0;

    BufferLength = sizeof(DOT11_WFD_DISCOVER_COMPLETE_PARAMETERS);
    DBGPRINT(RT_DEBUG_ERROR, ("%s::Header(=%d)\n", __FUNCTION__, BufferLength));

    //Calculate the size of device list
    for (cliIdx = 0; cliIdx < MAX_P2P_DISCOVERY_SIZE/*pAd->pP2pCtrll->P2PTable.DiscoNumber*/; cliIdx++)
    {
        if (pAd->pP2pCtrll->P2PTable.DiscoEntry[cliIdx].bValid == FALSE)
            continue;

        BufferLength += (sizeof(DOT11_WFD_DEVICE_ENTRY) +
                        pAd->pP2pCtrll->P2PTable.DiscoEntry[cliIdx].BeaconFrameLen +
                        pAd->pP2pCtrll->P2PTable.DiscoEntry[cliIdx].ProbeRspFrameLen);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("%s::DeviceEntry total buffer Len(=%d), Header(%d)\n", __FUNCTION__, BufferLength, sizeof(DOT11_WFD_DISCOVER_COMPLETE_PARAMETERS)));

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd,  &pDiscoCompleteParam, BufferLength);
    if (pDiscoCompleteParam != NULL)
    {
        //
        // Fill the dicovery complete status indication parameters
        //
        PlatformZeroMemory(pDiscoCompleteParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pDiscoCompleteParam->Header,
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_WFD_DISCOVER_COMPLETE_PARAMETERS_REVISION_1,
                                        DOT11_SIZEOF_WFD_DISCOVER_COMPLETE_PARAMETERS_REVISION_1);


        //parse device list into buffer
        pBuffer = (PUCHAR) pDiscoCompleteParam + sizeof(DOT11_WFD_DISCOVER_COMPLETE_PARAMETERS);
        P2pMsBuildDeviceList(pAd, pPort, pBuffer, BufferLength, &BytesWritten, &BytesNeeded, &TotalDeviceNumber);

        DBGPRINT(RT_DEBUG_TRACE, ("%s::RemainingBytes(%d), tmpWritten(%d), tmpNeeded(%d), TotalDeviceNumber(%d)\n",
                                    __FUNCTION__,
                                    BufferLength,
                                    BytesWritten,
                                    BytesNeeded,
                                    TotalDeviceNumber));

        pDiscoCompleteParam->Status             = NDIS_STATUS_SUCCESS;
        pDiscoCompleteParam->uListOffset        = sizeof(DOT11_WFD_DISCOVER_COMPLETE_PARAMETERS);
        pDiscoCompleteParam->uNumOfEntries      = TotalDeviceNumber;
        pDiscoCompleteParam->uTotalNumOfEntries = TotalDeviceNumber;
        pDiscoCompleteParam->uListLength        = BytesWritten;

        TotalBytesWritten = sizeof(DOT11_WFD_DISCOVER_COMPLETE_PARAMETERS) + pDiscoCompleteParam->uListLength;
        if(BytesNeeded != 0)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s ==> need more buffer size (=%d)\n", __FUNCTION__, BytesNeeded));
        }
        else
        {
            PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_DISCOVER_COMPLETE, NULL, pDiscoCompleteParam, TotalBytesWritten);
            DBGPRINT(RT_DEBUG_TRACE, ("%s: NDIS_STATUS_DOT11_WFD_DISCOVER_COMPLETE, BytesWritten=%d, BytesNeeded=%d, TotalBytesWritten=%d \n",__FUNCTION__, BytesWritten, BytesNeeded, TotalBytesWritten));
        }
        PlatformFreeMemory(pDiscoCompleteParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon successful transmission of GO
        negotiation request packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pOutBuffer          Pointer to tranmitted GO negotiation request packet
        OutBufferLen        Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateGONegoReqSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus)
{
    PDOT11_GO_NEGOTIATION_REQUEST_SEND_COMPLETE_PARAMETERS pNegoReqParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);

    BufferLength = sizeof(DOT11_GO_NEGOTIATION_REQUEST_SEND_COMPLETE_PARAMETERS)
                    + OutBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
   PlatformAllocateMemory(pAd,  &pNegoReqParam, BufferLength);
    if (pNegoReqParam != NULL)
    {
        //
        // Fill the GO nego req send status indication parameters
        //
        PlatformZeroMemory(pNegoReqParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pNegoReqParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_GO_NEGOTIATION_REQUEST_SEND_COMPLETE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_GO_NEGOTIATION_REQUEST_SEND_COMPLETE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pNegoReqParam->PeerDeviceAddress, pFrame->p80211Header.Addr1);
        pNegoReqParam->DialogToken = pFrame->Token;
        pNegoReqParam->Status = NdisStaus;
        pNegoReqParam->uIEsOffset = sizeof(DOT11_GO_NEGOTIATION_REQUEST_SEND_COMPLETE_PARAMETERS);
        pNegoReqParam->uIEsLength = OutBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pNegoReqParam, pNegoReqParam->uIEsOffset),
                        Add2Ptr(pOutBuffer, FrameIEOffset),
                        pNegoReqParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_GO_NEGOTIATION_REQUEST_SEND_COMPLETE,
                            NULL, pNegoReqParam, BufferLength);

        PlatformFreeMemory(pNegoReqParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon transmission of GO negotiation response
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pOutBuffer          Pointer to transmitted GO negotiation response packet
        OutBufferLen        Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateGONegoRspSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus)
{
    PDOT11_GO_NEGOTIATION_RESPONSE_SEND_COMPLETE_PARAMETERS pNegoRspParam = NULL;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
//  UCHAR               idx;
//  PCHAR               ptr;
    BufferLength = sizeof(DOT11_GO_NEGOTIATION_RESPONSE_SEND_COMPLETE_PARAMETERS)
                    + OutBufferLen - FrameIEOffset;

    DBGPRINT(RT_DEBUG_ERROR, ("%s ==> bufferLen=%d Length=%d\n", __FUNCTION__,BufferLength,pFrame->Length));

    //Allocate memory for indicating dicovery complete
  PlatformAllocateMemory(pAd,  &pNegoRspParam, BufferLength);
    if (pNegoRspParam != NULL)
    {
        //
        // Fill the GO nego req send status indication parameters
        //
        PlatformZeroMemory(pNegoRspParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pNegoRspParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_GO_NEGOTIATION_RESPONSE_SEND_COMPLETE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_GO_NEGOTIATION_RESPONSE_SEND_COMPLETE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pNegoRspParam->PeerDeviceAddress, pFrame->p80211Header.Addr1);
        pNegoRspParam->DialogToken = pFrame->Token;
        pNegoRspParam->Status = NdisStaus;
        pNegoRspParam->uIEsOffset = sizeof(DOT11_GO_NEGOTIATION_RESPONSE_SEND_COMPLETE_PARAMETERS);
        pNegoRspParam->uIEsLength = OutBufferLen - FrameIEOffset;

        PlatformMoveMemory(Add2Ptr((PUCHAR)pNegoRspParam, pNegoRspParam->uIEsOffset),
                        Add2Ptr(pOutBuffer, FrameIEOffset),
                        pNegoRspParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_GO_NEGOTIATION_RESPONSE_SEND_COMPLETE,
                            NULL, pNegoRspParam, BufferLength);

        PlatformFreeMemory(pNegoRspParam,BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon transmission of GO negotiation
        confirmation packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pOutBuffer          Pointer to transmitted GO negotiation confirmation
        OutBufferLen        Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateGONegoConfirmSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus)
{
    PDOT11_GO_NEGOTIATION_CONFIRMATION_SEND_COMPLETE_PARAMETERS pNegoConfirmParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);

    BufferLength = sizeof(DOT11_GO_NEGOTIATION_CONFIRMATION_SEND_COMPLETE_PARAMETERS)
                    + OutBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd, &pNegoConfirmParam, BufferLength);
    if (pNegoConfirmParam != NULL)
    {
        //
        // Fill the GO nego req send status indication parameters
        //
        PlatformZeroMemory(pNegoConfirmParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pNegoConfirmParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_GO_NEGOTIATION_CONFIRMATION_SEND_COMPLETE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_GO_NEGOTIATION_CONFIRMATION_SEND_COMPLETE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pNegoConfirmParam->PeerDeviceAddress, pFrame->p80211Header.Addr1);
        pNegoConfirmParam->DialogToken = pFrame->Token;
        pNegoConfirmParam->Status = NdisStaus;
        pNegoConfirmParam->uIEsOffset = sizeof(DOT11_GO_NEGOTIATION_CONFIRMATION_SEND_COMPLETE_PARAMETERS);
        pNegoConfirmParam->uIEsLength = OutBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pNegoConfirmParam, pNegoConfirmParam->uIEsOffset),
                        Add2Ptr(pOutBuffer, FrameIEOffset),
                        pNegoConfirmParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_GO_NEGOTIATION_CONFIRMATION_SEND_COMPLETE,
                            NULL, pNegoConfirmParam, BufferLength);

        PlatformFreeMemory(pNegoConfirmParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
#ifdef WFD_NEW_PUBLIC_ACTION

    if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
        (pAd->HwCfg.LatchRfRegs.Channel != pPort->Channel))
    {
        P2pMsRestoreChannel(pAd, pPort,  /*pPort->Channel*/pAd->HwCfg.LatchRfRegs.Channel);
    }
#endif /*WFD_NEW_PUBLIC_ACTION*/            
}

/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon successful transmission of invite
        request packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pOutBuffer          Pointer to tranmitted GO negotiation request packet
        OutBufferLen        Size of invite request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateInviteReqSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus)
{
    PDOT11_INVITATION_REQUEST_SEND_COMPLETE_PARAMETERS pInviteReqParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);

    BufferLength = sizeof(DOT11_INVITATION_REQUEST_SEND_COMPLETE_PARAMETERS)
                    + OutBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd,  &pInviteReqParam, BufferLength);
    if (pInviteReqParam != NULL)
    {
        //
        // Fill the GO nego req send status indication parameters
        //
        PlatformZeroMemory(pInviteReqParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pInviteReqParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_INVITATION_REQUEST_SEND_COMPLETE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_INVITATION_REQUEST_SEND_COMPLETE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pInviteReqParam->PeerDeviceAddress, pFrame->p80211Header.Addr1);
        COPY_MAC_ADDR(pInviteReqParam->ReceiverAddress, pFrame->p80211Header.Addr1);
        pInviteReqParam->DialogToken = pFrame->Token;
        pInviteReqParam->Status = NdisStaus;
        pInviteReqParam->uIEsOffset = sizeof(DOT11_INVITATION_REQUEST_SEND_COMPLETE_PARAMETERS);
        pInviteReqParam->uIEsLength = OutBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pInviteReqParam, pInviteReqParam->uIEsOffset),
                        Add2Ptr(pOutBuffer, FrameIEOffset),
                        pInviteReqParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_INVITATION_REQUEST_SEND_COMPLETE,
                            NULL, pInviteReqParam, BufferLength);

        PlatformFreeMemory(pInviteReqParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}

/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon successful transmission of invite
        response packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pOutBuffer          Pointer to tranmitted GO negotiation request packet
        OutBufferLen        Size of invite request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateInviteRspSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus)
{
    PDOT11_INVITATION_RESPONSE_SEND_COMPLETE_PARAMETERS pInviteRspParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);

    BufferLength = sizeof(DOT11_INVITATION_RESPONSE_SEND_COMPLETE_PARAMETERS)
                    + OutBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
   PlatformAllocateMemory(pAd,  &pInviteRspParam, BufferLength);
    if (pInviteRspParam != NULL)
    {
        // Flush Acceptor's discovery table once indicate send-complete Invitation Response.
        // OS will set discover request in the next step.
        if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_INVITATION))
        {
            //Remove all entries from P2P group table
            P2pGroupTabInit(pAd); // Ralink P2P
            P2pDiscoTabInit(pAd); // WIN8 P2P
        }
    
        //
        // Fill the GO nego req send status indication parameters
        //
        PlatformZeroMemory(pInviteRspParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pInviteRspParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_INVITATION_RESPONSE_SEND_COMPLETE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_INVITATION_RESPONSE_SEND_COMPLETE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pInviteRspParam->ReceiverDeviceAddress, pFrame->p80211Header.Addr1);
        pInviteRspParam->DialogToken = pFrame->Token;
        pInviteRspParam->Status = NdisStaus;
        pInviteRspParam->uIEsOffset = sizeof(DOT11_INVITATION_RESPONSE_SEND_COMPLETE_PARAMETERS);
        pInviteRspParam->uIEsLength = OutBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pInviteRspParam, pInviteRspParam->uIEsOffset),
                        Add2Ptr(pOutBuffer, FrameIEOffset),
                        pInviteRspParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_INVITATION_RESPONSE_SEND_COMPLETE,
                            NULL, pInviteRspParam, BufferLength);

        PlatformFreeMemory(pInviteRspParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}



/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon successful transmission of provision
        discovery request packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pOutBuffer          Pointer to tranmitted GO negotiation request packet
        OutBufferLen        Size of invite request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateProvDiscoReqSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus)
{
    PDOT11_PROVISION_DISCOVERY_REQUEST_SEND_COMPLETE_PARAMETERS pProvDiscoReqParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);

    BufferLength = sizeof(DOT11_PROVISION_DISCOVERY_REQUEST_SEND_COMPLETE_PARAMETERS)
                    + OutBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
   PlatformAllocateMemory(pAd,  &pProvDiscoReqParam, BufferLength);
    if (pProvDiscoReqParam != NULL)
    {
        //
        // Fill the GO nego req send status indication parameters
        //
        PlatformZeroMemory(pProvDiscoReqParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pProvDiscoReqParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_PROVISION_DISCOVERY_REQUEST_SEND_COMPLETE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_PROVISION_DISCOVERY_REQUEST_SEND_COMPLETE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pProvDiscoReqParam->PeerDeviceAddress, pFrame->p80211Header.Addr1);
        COPY_MAC_ADDR(pProvDiscoReqParam->ReceiverAddress, pFrame->p80211Header.Addr1);
        pProvDiscoReqParam->DialogToken = pFrame->Token;
        pProvDiscoReqParam->Status = NdisStaus;
        pProvDiscoReqParam->uIEsOffset = sizeof(DOT11_PROVISION_DISCOVERY_REQUEST_SEND_COMPLETE_PARAMETERS);
        pProvDiscoReqParam->uIEsLength = OutBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pProvDiscoReqParam, pProvDiscoReqParam->uIEsOffset),
                        Add2Ptr(pOutBuffer, FrameIEOffset),
                        pProvDiscoReqParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_PROVISION_DISCOVERY_REQUEST_SEND_COMPLETE,
                            NULL, pProvDiscoReqParam, BufferLength);

        PlatformFreeMemory(pProvDiscoReqParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon successful transmission of provision
        discovery response packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pOutBuffer          Pointer to tranmitted GO negotiation request packet
        OutBufferLen        Size of invite request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/VOID
P2pMsIndicateProvDiscoRspSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus)
{
    PDOT11_PROVISION_DISCOVERY_RESPONSE_SEND_COMPLETE_PARAMETERS pProvDiscoRspParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);

    FUNC_ENTER;

    BufferLength = sizeof(DOT11_PROVISION_DISCOVERY_RESPONSE_SEND_COMPLETE_PARAMETERS)
                    + OutBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd,  &pProvDiscoRspParam, BufferLength);
    if (pProvDiscoRspParam != NULL)
    {
        // Flush Acceptor's discovery table once indicate send-complete PD Response.
        // OS will set discover request in the next step.
        if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_START_PROGRESS))
        {
            //Remove all entries from P2P group table
            P2pGroupTabInit(pAd); // Ralink P2P
            P2pDiscoTabInit(pAd); // WIN8 P2P
        }
    
        //
        // Fill the GO nego req send status indication parameters
        //
        PlatformZeroMemory(pProvDiscoRspParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pProvDiscoRspParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_PROVISION_DISCOVERY_RESPONSE_SEND_COMPLETE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_PROVISION_DISCOVERY_RESPONSE_SEND_COMPLETE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pProvDiscoRspParam->ReceiverDeviceAddress, pFrame->p80211Header.Addr1);
        pProvDiscoRspParam->DialogToken = pFrame->Token;
        pProvDiscoRspParam->Status = NdisStaus;
        pProvDiscoRspParam->uIEsOffset = sizeof(DOT11_PROVISION_DISCOVERY_RESPONSE_SEND_COMPLETE_PARAMETERS);
        pProvDiscoRspParam->uIEsLength = OutBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pProvDiscoRspParam, pProvDiscoRspParam->uIEsOffset),
                        Add2Ptr(pOutBuffer, FrameIEOffset),
                        pProvDiscoRspParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_PROVISION_DISCOVERY_RESPONSE_SEND_COMPLETE,
                            NULL, pProvDiscoRspParam, BufferLength);

        PlatformFreeMemory(pProvDiscoRspParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }

    FUNC_LEAVE;
    
}



/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon reception of GO negotiation request
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateGONegoReqRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PDOT11_RECEIVED_GO_NEGOTIATION_REQUEST_PARAMETERS pNegoReqParam;
    ULONG               BufferLength;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
    UCHAR               OpChannel = 0, Intent = 0, ListenChannel = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Processing GO Negotiation Request\n", __FUNCTION__));

    BufferLength = sizeof(DOT11_RECEIVED_GO_NEGOTIATION_REQUEST_PARAMETERS) +
                    InBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd,  &pNegoReqParam, BufferLength);
    if (pNegoReqParam != NULL)
    {
        // First, parsing Rx data
        P2pParseSubElmt(pAd, &pFrame->ElementID, InBufferLen - FrameIEOffset,
                    FALSE, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &ListenChannel, &OpChannel, NULL, &Intent, NULL, NULL);

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Parsing Rx: ListenChannel = %d. OpChannel = %d. Intent = %d. TieBreaker = %d. \n",
                __FUNCTION__, ListenChannel, OpChannel, (Intent >> 1), (Intent & 1)));
        //
        // Fill the GO nego req recv status indication parameters
        //
        PlatformZeroMemory(pNegoReqParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pNegoReqParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_RECEIVED_GO_NEGOTIATION_REQUEST_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_RECEIVED_GO_NEGOTIATION_REQUEST_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pNegoReqParam->PeerDeviceAddress, pFrame->p80211Header.Addr2);
        pNegoReqParam->DialogToken = pFrame->Token;
        pNegoReqParam->RequestContext = NULL;
        pNegoReqParam->uIEsOffset = sizeof(DOT11_RECEIVED_GO_NEGOTIATION_REQUEST_PARAMETERS);
        pNegoReqParam->uIEsLength = InBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pNegoReqParam, pNegoReqParam->uIEsOffset),
                        Add2Ptr(pInBuffer, FrameIEOffset),
                        pNegoReqParam->uIEsLength);

        // Save common set channel list
        P2pMsSaveCommonSetChannelList(pAd, pPort, pInBuffer, InBufferLen);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_RECEIVED_GO_NEGOTIATION_REQUEST,
                            NULL, pNegoReqParam, BufferLength);

        PlatformFreeMemory(pNegoReqParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon reception of GO negotiation response
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation response packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateGONegoRspRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PDOT11_RECEIVED_GO_NEGOTIATION_RESPONSE_PARAMETERS pNegoRspParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
    UCHAR               StatusCode = P2PSTATUS_SUCCESS, OpChannel = 0, Intent = 0, ListenChannel = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Processing GO Negotiation Response\n", __FUNCTION__));

    BufferLength = sizeof(DOT11_RECEIVED_GO_NEGOTIATION_RESPONSE_PARAMETERS)
                    + InBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
   PlatformAllocateMemory(pAd,  &pNegoRspParam, BufferLength);
    if (pNegoRspParam != NULL)
    {
        // First, parsing Rx data
        P2pParseSubElmt(pAd, &pFrame->ElementID, InBufferLen - FrameIEOffset,
                    FALSE, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &ListenChannel, &OpChannel, NULL, &Intent, &StatusCode, NULL);

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Parsing Rx: StatusCode = %d. ListenChannel = %d. OpChannel = %d. Intent = %d. TieBreaker = %d. \n",
                __FUNCTION__, StatusCode, ListenChannel, OpChannel, (Intent >>1), (Intent & 1)));

        // save the peer's opchannel
        if (OpChannel != 0)
        {
            UCHAR       p2pindex = P2P_NOT_FOUND;

            p2pindex = P2pDiscoTabSearch(pAd, pFrame->p80211Header.Addr2, NULL, NULL);
            if (p2pindex < MAX_P2P_DISCOVERY_SIZE)
            {
                pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].GroupChannel = OpChannel;
            }
        }


        //
        // Fill the GO nego req recv status indication parameters
        //
        PlatformZeroMemory(pNegoRspParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pNegoRspParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_RECEIVED_GO_NEGOTIATION_RESPONSE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_RECEIVED_GO_NEGOTIATION_RESPONSE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pNegoRspParam->PeerDeviceAddress, pFrame->p80211Header.Addr2);
        pNegoRspParam->DialogToken = pFrame->Token;
        pNegoRspParam->ResponseContext = NULL;
        pNegoRspParam->uIEsOffset = sizeof(DOT11_RECEIVED_GO_NEGOTIATION_RESPONSE_PARAMETERS);
        pNegoRspParam->uIEsLength = InBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pNegoRspParam, pNegoRspParam->uIEsOffset),
                        Add2Ptr(pInBuffer, FrameIEOffset),
                        pNegoRspParam->uIEsLength);

        // Save common set channel list
        P2pMsSaveCommonSetChannelList(pAd, pPort, pInBuffer, InBufferLen);

        //
        // Error handling
        //

        // if got error, go back to idle state
        if ((StatusCode != P2PSTATUS_SUCCESS) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_FORMATION))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - !!!ERROR!!! state = %s. listen-ch = %d. common-ch= %d. latch-ch = %d. \n",
                    __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase), pPort->P2PCfg.ListenChannel, pPort->Channel, pAd->HwCfg.LatchRfRegs.Channel));

            if (StatusCode != P2PSTATUS_PASSED)
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;

            // Go back to my listen channel right now.
            // Assume my listen ch is equal to common channel whether port 0 is connected or not.
            if (/*(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&*/
//              (pPort->P2PCfg.RandomListenChTestBitOn == TRUE) &&
                (pAd->HwCfg.LatchRfRegs.Channel != pPort->P2PCfg.ListenChannel) /*&&
                (pPort->P2PCfg.ListenChannel == pPort->Channel)*/)
            {
                P2pMsRestoreChannel(pAd,  pPort, pAd->HwCfg.LatchRfRegs.Channel);
            }
        }

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_RECEIVED_GO_NEGOTIATION_RESPONSE,
                            NULL, pNegoRspParam, BufferLength);

        PlatformFreeMemory(pNegoRspParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon reception of GO negotiation confirmation
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation response packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateGONegoConfirmRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PDOT11_RECEIVED_GO_NEGOTIATION_CONFIRMATION_PARAMETERS pNegoConfirmParam;
    ULONG               BufferLength = 0;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
    UCHAR               StatusCode = P2PSTATUS_SUCCESS, OpChannel = 0, ListenChannel = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Processing GO Negotiation confirmation\n", __FUNCTION__));

    BufferLength = sizeof(DOT11_RECEIVED_GO_NEGOTIATION_CONFIRMATION_PARAMETERS)
                    + InBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd,  &pNegoConfirmParam, BufferLength);
    if (pNegoConfirmParam != NULL)
    {
        // First, parsing Rx data
        P2pParseSubElmt(pAd, &pFrame->ElementID, InBufferLen - FrameIEOffset,
                    FALSE, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &ListenChannel, &OpChannel, NULL, NULL, &StatusCode, NULL);

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Parsing Rx: StatusCode = %d. ListenChannel = %d. OpChannel = %d. \n",
                __FUNCTION__, StatusCode, ListenChannel, OpChannel));


        //
        // Fill the GO nego req recv status indication parameters
        //
        PlatformZeroMemory(pNegoConfirmParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pNegoConfirmParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_RECEIVED_GO_NEGOTIATION_CONFIRMATION_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_RECEIVED_GO_NEGOTIATION_CONFIRMATION_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pNegoConfirmParam->PeerDeviceAddress, pFrame->p80211Header.Addr2);
        pNegoConfirmParam->DialogToken = pFrame->Token;
        pNegoConfirmParam->uIEsOffset = sizeof(DOT11_RECEIVED_GO_NEGOTIATION_CONFIRMATION_PARAMETERS);
        pNegoConfirmParam->uIEsLength = InBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pNegoConfirmParam, pNegoConfirmParam->uIEsOffset),
                        Add2Ptr(pInBuffer, FrameIEOffset),
                        pNegoConfirmParam->uIEsLength);

#ifdef WFD_NEW_PUBLIC_ACTION
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
            (pAd->HwCfg.LatchRfRegs.Channel != pPort->Channel))
        {
            P2pMsRestoreChannel(pAd, pPort,  /*pPort->Channel*/pAd->HwCfg.LatchRfRegs.Channel);
        }
#else
        //
        // Error handling
        //

        // if got error, go back to idle state
        if ((StatusCode != P2PSTATUS_SUCCESS) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_FORMATION))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - !!!ERROR!!! state = %s. listen-ch = %d. common-ch= %d. latch-ch = %d. \n",
                    __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase), pPort->P2PCfg.ListenChannel, pPort->Channel, pAd->HwCfg.LatchRfRegs.Channel));

            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;

            // Go back to my listen channel right now.
            // Assume my listen ch is equal to common channel whether port 0 is connected or not.
            if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
//              (pPort->P2PCfg.RandomListenChTestBitOn == TRUE) &&
                (pAd->HwCfg.LatchRfRegs.Channel != pPort->P2PCfg.ListenChannel) &&
                (pPort->P2PCfg.ListenChannel == pPort->Channel))
            {
                P2pMsRestoreChannel(pAd, pPort,  pAd->HwCfg.LatchRfRegs.Channel);
            }
        }
#endif /*WFD_NEW_PUBLIC_ACTION*/
        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_RECEIVED_GO_NEGOTIATION_CONFIRMATION,
                            NULL, pNegoConfirmParam, BufferLength);
        
        PlatformFreeMemory(pNegoConfirmParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon reception of invite request
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateInviteReqRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PDOT11_RECEIVED_INVITATION_REQUEST_PARAMETERS pInviteReqParam;
    ULONG               BufferLength;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
    UCHAR               OpChannel = 0, InviteFlag = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Processing Invite Request\n", __FUNCTION__));

    BufferLength = sizeof(DOT11_RECEIVED_INVITATION_REQUEST_PARAMETERS) +
                    InBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
   PlatformAllocateMemory(pAd,  &pInviteReqParam, BufferLength);
    if (pInviteReqParam != NULL)
    {
        // First, parsing Rx data
        P2pParseSubElmt(pAd, &pFrame->ElementID, InBufferLen - FrameIEOffset,
                    FALSE, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &OpChannel, NULL, NULL, NULL, &InviteFlag);

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Parsing Rx: InviteFlag = %d. OpChannel = %d. \n",
                __FUNCTION__, InviteFlag, OpChannel));

        // Update op channel got from Invitation Request
        pPort->P2PCfg.GoIOpChannel = OpChannel;
        
        //
        // Fill the GO nego req recv status indication parameters
        //
        PlatformZeroMemory(pInviteReqParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pInviteReqParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_RECEIVED_INVITATION_REQUEST_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_RECEIVED_INVITATION_REQUEST_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pInviteReqParam->TransmitterDeviceAddress, pFrame->p80211Header.Addr2);
        COPY_MAC_ADDR(pInviteReqParam->BSSID, pFrame->p80211Header.Addr3);
        pInviteReqParam->DialogToken = pFrame->Token;
        pInviteReqParam->RequestContext = NULL;
        pInviteReqParam->uIEsOffset = sizeof(DOT11_RECEIVED_INVITATION_REQUEST_PARAMETERS);
        pInviteReqParam->uIEsLength = InBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pInviteReqParam, pInviteReqParam->uIEsOffset),
                        Add2Ptr(pInBuffer, FrameIEOffset),
                        pInviteReqParam->uIEsLength);

        // Save common set channel list
        P2pMsSaveCommonSetChannelList(pAd, pPort, pInBuffer, InBufferLen);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_RECEIVED_INVITATION_REQUEST,
                            NULL, pInviteReqParam, BufferLength);

        PlatformFreeMemory(pInviteReqParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}



/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon reception of invite response
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateInviteRspRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PDOT11_RECEIVED_INVITATION_RESPONSE_PARAMETERS pInviteRspParam;
    ULONG               BufferLength;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
    UCHAR               StatusCode = P2PSTATUS_SUCCESS, OpChannel = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Processing Invite Response\n", __FUNCTION__));

    BufferLength = sizeof(DOT11_RECEIVED_INVITATION_RESPONSE_PARAMETERS)    +
                    InBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd,  &pInviteRspParam, BufferLength);
    if (pInviteRspParam != NULL)
    {
        // First, parsing Rx data
        P2pParseSubElmt(pAd, &pFrame->ElementID, InBufferLen - FrameIEOffset,
                    FALSE, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &OpChannel, NULL, NULL, &StatusCode, NULL);

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Parsing Rx: StatusCode = %d. OpChannel = %d.\n",
                __FUNCTION__, StatusCode, OpChannel));


        //
        // Fill the Invite req recv status indication parameters
        //
        PlatformZeroMemory(pInviteRspParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pInviteRspParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_RECEIVED_INVITATION_RESPONSE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_RECEIVED_INVITATION_RESPONSE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pInviteRspParam->TransmitterDeviceAddress, pFrame->p80211Header.Addr2);
        COPY_MAC_ADDR(pInviteRspParam->BSSID, pFrame->p80211Header.Addr3);
        pInviteRspParam->DialogToken = pFrame->Token;
        pInviteRspParam->uIEsOffset = sizeof(DOT11_RECEIVED_INVITATION_RESPONSE_PARAMETERS);
        pInviteRspParam->uIEsLength = InBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pInviteRspParam, pInviteRspParam->uIEsOffset),
                        Add2Ptr(pInBuffer, FrameIEOffset),
                        pInviteRspParam->uIEsLength);

        //
        // Error handling
        //

        // if got error, go back to idle state
        // TODO: Always go back to my operation channel because PD request shall be sent on the operation channel.
        if (/*(StatusCode != P2PSTATUS_SUCCESS) &&*/ (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_INVITATION))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - !!!ERROR!!! state = %s. listen-ch = %d. common-ch= %d. latch-ch = %d. \n",
                    __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase), pPort->P2PCfg.ListenChannel, pPort->Channel, pAd->HwCfg.LatchRfRegs.Channel));

            if ((StatusCode != P2PSTATUS_PASSED) && (StatusCode != P2PSTATUS_SUCCESS))
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;

            // Go back to my listen channel right now.
            // Assume my listen ch is equal to common channel whether port 0 is connected or not.
            if (/*(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&*/
//              (pPort->P2PCfg.RandomListenChTestBitOn == TRUE) &&
                (pAd->HwCfg.LatchRfRegs.Channel != pPort->P2PCfg.ListenChannel) /*&&
                (pPort->P2PCfg.ListenChannel == pPort->Channel)*/)
            {
                P2pMsRestoreChannel(pAd, pPort,  pAd->HwCfg.LatchRfRegs.Channel);
            }
        }

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_RECEIVED_INVITATION_RESPONSE,
                            NULL, pInviteRspParam, BufferLength);
        
        PlatformFreeMemory(pInviteRspParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon reception of provision discovery
        request packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateProvDiscoReqRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PDOT11_RECEIVED_PROVISION_DISCOVERY_REQUEST_PARAMETERS pProvDiscoReqParam;
    ULONG               BufferLength;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
    USHORT              WpsConfigMethod = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Processing Provision Discovery Request\n", __FUNCTION__));

    BufferLength = sizeof(DOT11_RECEIVED_PROVISION_DISCOVERY_REQUEST_PARAMETERS) +
                    InBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
   PlatformAllocateMemory(pAd,  &pProvDiscoReqParam, BufferLength);
    if (pProvDiscoReqParam != NULL)
    {
        // First, parsing Rx data
        P2pParseSubElmt(pAd, &pFrame->ElementID, InBufferLen - FrameIEOffset,
                    FALSE, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL, &WpsConfigMethod, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Parsing Rx: WpsConfigMethod = 0x%x.(%s) \n",
                __FUNCTION__, WpsConfigMethod, decodeConfigMethod(WpsConfigMethod)));


        //
        // Fill the Provision Discovery req recv status indication parameters
        //
        PlatformZeroMemory(pProvDiscoReqParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pProvDiscoReqParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_RECEIVED_PROVISION_DISCOVERY_REQUEST_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_RECEIVED_PROVISION_DISCOVERY_REQUEST_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pProvDiscoReqParam->TransmitterDeviceAddress, pFrame->p80211Header.Addr2);
        COPY_MAC_ADDR(pProvDiscoReqParam->BSSID, pFrame->p80211Header.Addr3);
        pProvDiscoReqParam->DialogToken = pFrame->Token;
        pProvDiscoReqParam->RequestContext = NULL;
        pProvDiscoReqParam->uIEsOffset = sizeof(DOT11_RECEIVED_PROVISION_DISCOVERY_REQUEST_PARAMETERS);
        pProvDiscoReqParam->uIEsLength = InBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pProvDiscoReqParam, pProvDiscoReqParam->uIEsOffset),
                        Add2Ptr(pInBuffer, FrameIEOffset),
                        pProvDiscoReqParam->uIEsLength);

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_RECEIVED_PROVISION_DISCOVERY_REQUEST,
                            NULL, pProvDiscoReqParam, BufferLength);

        PlatformFreeMemory(pProvDiscoReqParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}


/*
    ========================================================================
    Routine Description:
        This function indicates to OS upon reception of provision discovery
        response packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicateProvDiscoRspRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PDOT11_RECEIVED_PROVISION_DISCOVERY_RESPONSE_PARAMETERS pProvDiscoRspParam = NULL;
    ULONG               BufferLength;
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    UINT                FrameIEOffset = (UINT)FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
    USHORT              WpsConfigMethod = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Processing Provision Discovery Response\n", __FUNCTION__));
    //DBGPRINT(RT_DEBUG_TRACE, ("%s ==> OUI = %x, %s, %x, %x\n", __FUNCTION__, pFrame->OUI[0], pFrame->OUI[1], pFrame->OUI[2], pFrame->OUIType));

    BufferLength = sizeof(DOT11_RECEIVED_PROVISION_DISCOVERY_RESPONSE_PARAMETERS) +
                    InBufferLen - FrameIEOffset;

    //Allocate memory for indicating dicovery complete
    PlatformAllocateMemory(pAd,  &pProvDiscoRspParam, BufferLength);
    if (pProvDiscoRspParam != NULL)
    {
        // First, parsing Rx data
        P2pParseSubElmt(pAd, &pFrame->ElementID, InBufferLen - FrameIEOffset,
                    FALSE, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL, &WpsConfigMethod, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Parsing Rx: WpsConfigMethod = 0x%x.(%s) \n",
                __FUNCTION__, WpsConfigMethod, decodeConfigMethod(WpsConfigMethod)));

        //
        // Fill the Provision nego req recv status indication parameters
        //
        PlatformZeroMemory(pProvDiscoRspParam, BufferLength);
        MP_ASSIGN_NDIS_OBJECT_HEADER(pProvDiscoRspParam->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_RECEIVED_PROVISION_DISCOVERY_RESPONSE_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_RECEIVED_PROVISION_DISCOVERY_RESPONSE_PARAMETERS_REVISION_1);

        COPY_MAC_ADDR(pProvDiscoRspParam->TransmitterDeviceAddress, pFrame->p80211Header.Addr2);
        COPY_MAC_ADDR(pProvDiscoRspParam->BSSID, pFrame->p80211Header.Addr3);
        pProvDiscoRspParam->DialogToken = pFrame->Token;
        pProvDiscoRspParam->uIEsOffset = sizeof(DOT11_RECEIVED_PROVISION_DISCOVERY_RESPONSE_PARAMETERS);
        pProvDiscoRspParam->uIEsLength = InBufferLen - FrameIEOffset;
        PlatformMoveMemory(Add2Ptr((PUCHAR)pProvDiscoRspParam, pProvDiscoRspParam->uIEsOffset),
                        Add2Ptr(pInBuffer, FrameIEOffset),
                        pProvDiscoRspParam->uIEsLength);
        DBGPRINT(RT_DEBUG_TRACE,("%s::TxAddr=%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
                                      __FUNCTION__,
                                      pFrame->p80211Header.Addr2[0],
                                      pFrame->p80211Header.Addr2[1],
                                      pFrame->p80211Header.Addr2[2],
                                      pFrame->p80211Header.Addr2[3],
                                      pFrame->p80211Header.Addr2[4],
                                      pFrame->p80211Header.Addr2[5]));

        DBGPRINT(RT_DEBUG_TRACE,("%s::PortNumber=%d\n", __FUNCTION__, pPort->PortNumber));

        if (pAd->HwCfg.LatchRfRegs.Channel != pPort->P2PCfg.ListenChannel)
        {
            P2pMsRestoreChannel(pAd, pPort,  pAd->HwCfg.LatchRfRegs.Channel);
        }       

        //Indicate to OS
        PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_WFD_RECEIVED_PROVISION_DISCOVERY_RESPONSE,
                            NULL, pProvDiscoRspParam, BufferLength);
                            
        PlatformFreeMemory(pProvDiscoRspParam, BufferLength);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Fail to allocate memory for indication\n", __FUNCTION__));
    }
}

/*
    ========================================================================
    Routine Description:
        This function indicates the received public action framr to OS

    Arguments:
        InformationBuffer           Pointer to 802.11 message
        InformationBufferLength Length of 802.11 message

    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsIndicatePublicActionFrameRecv(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR  InformationBuffer,
    IN  ULONG   InformationBufferLength)
{
    UCHAR   Subtype = InformationBuffer[LENGTH_802_11+6];

    switch(Subtype)
    {
        case GO_NEGOCIATION_REQ:
            P2pMsIndicateGONegoReqRecv(pAd, pPort, InformationBuffer, InformationBufferLength);
            break;
        case GO_NEGOCIATION_RSP:
            P2pMsIndicateGONegoRspRecv(pAd, pPort, InformationBuffer, InformationBufferLength);
            break;
        case GO_NEGOCIATION_CONFIRM:
            P2pMsIndicateGONegoConfirmRecv(pAd, pPort, InformationBuffer, InformationBufferLength);
            break;
        case P2P_INVITE_REQ:
            P2pMsIndicateInviteReqRecv(pAd, pPort, InformationBuffer, InformationBufferLength);
            break;
        case P2P_INVITE_RSP:
            P2pMsIndicateInviteRspRecv(pAd, pPort, InformationBuffer, InformationBufferLength);
            break;
/*
        case P2P_DEV_DIS_REQ:
            P2pReceDevDisReqAction(pAd, Elem);
            break;
        case P2P_DEV_DIS_RSP:
            P2pReceDevDisRspAction(pAd, Elem);
            break;
*/
        case P2P_PROVISION_REQ:
            P2pMsIndicateProvDiscoReqRecv(pAd, pPort, InformationBuffer, InformationBufferLength);
            break;
        case P2P_PROVISION_RSP:
            P2pMsIndicateProvDiscoRspRecv(pAd, pPort, InformationBuffer, InformationBufferLength);
            break;
        default:
            DBGPRINT(RT_DEBUG_ERROR,("Unknown Public action frames with Vendor specific OUI = WFAP2P.Subtype = %d \n", Subtype));
            break;

    }
}

/*
    ========================================================================
    Routine Description:
        This function generates GO negotiation request packet to peer device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSendGONegoReq(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PUCHAR  pOutBuffer = NULL;
    ULONG   FrameLen = 0;
    ULONG   tmpByte = 0;
    UCHAR   p2pindex = P2P_NOT_FOUND;
    BOOLEAN bScanInProgress = FALSE;
    
    PDOT11_SEND_GO_NEGOTIATION_REQUEST_PARAMETERS   pWFDSendGONegoReq =
        (PDOT11_SEND_GO_NEGOTIATION_REQUEST_PARAMETERS) InformationBuffer;


    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSendGONegoReq->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SEND_GO_NEGOTIATION_REQUEST_PARAMETERS_REVISION_1,
                    DOT11_SIZEOF_SEND_GO_NEGOTIATION_REQUEST_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (MlmeAllocateMemory(pAd, &pOutBuffer) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

#ifdef MULTI_CHANNEL_SUPPORT
        // Stop Infra session if necessary
        P2pMsStartActionFrame(pAd, pPort);
#endif /*MULTI_CHANNEL_SUPPORT*/

        // Stay in formation state
        pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_FORMATION;

        // Start blocking periodic scan timer
        pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;

        // Start sending timer...
        // if no ack received for this sending frame after a given timeout, we will indicate a failure to NDIS
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = 0;

        // Copy MS Send Timeout to pAdapter and use below method to send action frame
        // 1. Stop sending action frame when receives ACK or acftion frame response and indicate success status to OS
        // 2. Stop sending action frame and indicate failure status to OS after timeout
        pPort->P2PCfg.MSSendTimeout = (pWFDSendGONegoReq->uSendTimeout /100) - WFD_COMPLETE_SEND_TIMEOUT;

        //Update Configuration Timeout
        pPort->P2PCfg.ConfigTimeout[0] = pWFDSendGONegoReq->MinimumConfigTimeout.GOTimeout;
        pPort->P2PCfg.ConfigTimeout[1] = pWFDSendGONegoReq->MinimumConfigTimeout.ClientTimeout;

        //Update GO Capability
//      P2pMsUpdateGOCapability(pAd, pPort, pWFDSendGONegoReq->GroupCapability);
        DBGPRINT(RT_DEBUG_TRACE,("%s:: GroupCap=%d. GRPCAP_OWNER=%d. GRPCAP_PERSISTENT=%d. GRPCAP_INTRA_BSS=%d. \n\t GRPCAP_CROSS_CONNECT=%d. GRPCAP_PERSISTENT_RECONNECT=%d. GRPCAP_GROUP_FORMING=%d. GRPCAP_LIMIT=%d.\n",
                __FUNCTION__, pWFDSendGONegoReq->GroupCapability,
                MT_TEST_BIT(pWFDSendGONegoReq->GroupCapability, GRPCAP_OWNER),
                MT_TEST_BIT(pWFDSendGONegoReq->GroupCapability, GRPCAP_PERSISTENT),
                MT_TEST_BIT(pWFDSendGONegoReq->GroupCapability, GRPCAP_INTRA_BSS),
                MT_TEST_BIT(pWFDSendGONegoReq->GroupCapability, GRPCAP_CROSS_CONNECT),
                MT_TEST_BIT(pWFDSendGONegoReq->GroupCapability, GRPCAP_PERSISTENT_RECONNECT),
                MT_TEST_BIT(pWFDSendGONegoReq->GroupCapability, GRPCAP_GROUP_FORMING),
                MT_TEST_BIT(pWFDSendGONegoReq->GroupCapability, GRPCAP_LIMIT)));

        P2pMsMakeGONegoReq(pAd,
                            pPort,
                            pWFDSendGONegoReq->PeerDeviceAddress,
                            pWFDSendGONegoReq->DialogToken,
                            pWFDSendGONegoReq->GroupOwnerIntent,
                            pWFDSendGONegoReq->IntendedInterfaceAddress,
                            pWFDSendGONegoReq->GroupCapability,
                            pOutBuffer,
                            &FrameLen);


        //Additional IE
        PlatformMoveMemory(Add2Ptr(pOutBuffer, FrameLen),
                    Add2Ptr(pWFDSendGONegoReq, pWFDSendGONegoReq->uIEsOffset),
                    pWFDSendGONegoReq->uIEsLength);
        FrameLen += pWFDSendGONegoReq->uIEsLength;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Go Negociation Request Length = %d. timeout = %d ms. LatchCh = %d. PeerDeviceAddress = %x:%x:%x:%x:%x:%x \n",
                __FUNCTION__, FrameLen, pWFDSendGONegoReq->uSendTimeout, pAd->HwCfg.LatchRfRegs.Channel,
                pWFDSendGONegoReq->PeerDeviceAddress[0], pWFDSendGONegoReq->PeerDeviceAddress[1], pWFDSendGONegoReq->PeerDeviceAddress[2],
                pWFDSendGONegoReq->PeerDeviceAddress[3], pWFDSendGONegoReq->PeerDeviceAddress[4], pWFDSendGONegoReq->PeerDeviceAddress[5]));

        // ======================================>
        // Update connecting MAC address to the first array
        PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
        PlatformMoveMemory(pPort->P2PCfg.ConnectingMAC, pWFDSendGONegoReq->PeerDeviceAddress, MAC_ADDR_LEN);
        pPort->P2PCfg.ConnectingIndex = 0xff;  // Win7 might use not 0xff
        // <======================================

        //
        // If the peer's listen channel is not equal to my listen channel,
        // Buffer this frame and send out when I arrive the peer listen channel
        //
        p2pindex = P2pDiscoTabSearch(pAd, pWFDSendGONegoReq->PeerDeviceAddress, NULL, NULL);

        // If scan in progress, stop p2p scan in the next 100ms
        if (pPort->P2PCfg.P2PDiscoProvState != P2P_ENABLE_LISTEN_ONLY)
        {
            bScanInProgress = TRUE;

            // 1. scan in progress
            // 2. p2p idle state
            // 3. scan complete and wait to go back to p2p listen only state
            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
                pPort->P2PCfg.ScanPeriod = pPort->P2PCfg.P2pCounter.CounterAftrScanButton + 1;
            else if ((pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE) && (pPort->P2PCfg.ScanPeriod > pPort->P2PCfg.P2pCounter.CounterAftrScanButton))
                pPort->P2PCfg.ScanPeriod = pPort->P2PCfg.P2pCounter.CounterAftrScanButton + 1;
            else if (pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE)
                ;
            else
                bScanInProgress = FALSE;
            
            if (bScanInProgress)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Stop p2p scan in the next 100ms (%d %d %s)\n", 
                    __FUNCTION__, pPort->P2PCfg.ScanPeriod, pPort->P2PCfg.P2pCounter.CounterAftrScanButton, decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
            }
        }

        // My listen channel is not equal to the peer listen channel        
        if (
//          (pPort->P2PCfg.RandomListenChTestBitOn == TRUE) &&
            (p2pindex < MAX_P2P_DISCOVERY_SIZE) &&
            (pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel != pPort->P2PCfg.ListenChannel) &&
            (MlmeSyncIsValidChannel(pAd, pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel)) &&
            (pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel != pAd->HwCfg.LatchRfRegs.Channel) &&
            (pPort->P2PCfg.SwitchChannelInProgress == FALSE))
        {
            //Free Old Buffered Data
            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

            // Allocate new buffers
           PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
	    pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
            if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
            {
                //Cache new Buffer
                PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
                pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;           
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Go to the peer's[%d] listen channel(%d) to send the packet \n", __FUNCTION__, p2pindex, pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel));

#ifdef WFD_NEW_PUBLIC_ACTION
#ifdef MULTI_CHANNEL_SUPPORT
                //MlmeP2pMsSwitchCh(pAd, pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel);
                AsicSwitchChannel(pPort, pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel, pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel, BW_20, FALSE);
#else
                //AsicSwitchChannel(pAd, pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel, FALSE);
                //AsicLockChannel(pAd, pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel);
#endif /*MULTI_CHANNEL_SUPPORT*/                
#else
                // Go to the peer's listen channel
                pPort->P2PCfg.P2pCounter.SwitchingChannelCounter = 0;
                pPort->P2PCfg.SwitchChannelInProgress = TRUE;
                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ);
                MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED);
#endif /*WFD_NEW_PUBLIC_ACTION*/
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Alloc memory fail\n", __FUNCTION__));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
        }
        else    // My listen channel is equal to the peer listen channel
        {
            // I might not stay in my listen channel now because of scanning
            // When I go beack to my common channel with scan complete, send this action frame directly.
            if (bScanInProgress == TRUE)
            {
                //Free Old Buffered Data
                P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

                // Allocate new buffers
                PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
                 pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
                if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
                {
                    //Cache new Buffer
                    PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
                    pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;           

                    // Go to the peer's listen channel 
                    /*pPort->P2PCfg.P2pCounter.SwitchingChannelCounter = 0;
                    pPort->P2PCfg.SwitchChannelInProgress = TRUE;*/
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ);
                    MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED_AFTER_SCAN);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - Go to the peer's listen channel(%d) to send the packet. [scan=%d, status=0x%x]\n", 
                            __FUNCTION__, pPort->P2PCfg.ListenChannel, bScanInProgress, pPort->P2PCfg.WaitForMSCompletedFlags));
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s-%d - Alloc memory fail\n", __FUNCTION__, __LINE__));
                    ndisStatus = NDIS_STATUS_RESOURCES;
                    break;
                }
                
            }
            else
            {
#ifdef WFD_NEW_PUBLIC_ACTION
                //Free Old Buffered Data
                P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

                // Allocate new buffers
               PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
                pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
                
                if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
                {
                    //Cache new Buffer
                    PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
                }
#else                               
                // Kickoff TX   
                NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
#endif /*WFD_NEW_PUBLIC_ACTION*/
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Stay[%d] in the current channel to send the packet (%d - %d) \n", __FUNCTION__, p2pindex, pPort->P2PCfg.ListenChannel, pAd->HwCfg.LatchRfRegs.Channel));            
            }
        }
    }while(FALSE);

    if (pOutBuffer)
        MlmeFreeMemory(pAd, pOutBuffer);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_SEND_GO_NEGOTIATION_REQUEST_PARAMETERS) + pWFDSendGONegoReq->uIEsLength;
    }

#ifdef WFD_NEW_PUBLIC_ACTION
    MlmeP2pMsPublicActionFrameSend(pAd, pPort, GO_NEGOCIATION_REQ);
#endif /*WFD_NEW_PUBLIC_ACTION*/

    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function generates GO negotiation response packet to peer device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSendGONegoRsp(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PUCHAR  pOutBuffer = NULL;
    ULONG   FrameLen = 0;
    ULONG   tmpByte = 0;
    PDOT11_SEND_GO_NEGOTIATION_RESPONSE_PARAMETERS  pWFDSendGONegoRsp =
        (PDOT11_SEND_GO_NEGOTIATION_RESPONSE_PARAMETERS) InformationBuffer;

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSendGONegoRsp->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SEND_GO_NEGOTIATION_RESPONSE_PARAMETERS_REVISION_1,
                    DOT11_SIZEOF_SEND_GO_NEGOTIATION_RESPONSE_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (MlmeAllocateMemory(pAd, &pOutBuffer) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

#ifdef MULTI_CHANNEL_SUPPORT
        // Stop Infra session if necessary
        P2pMsStartActionFrame(pAd, pPort);
#endif /*MULTI_CHANNEL_SUPPORT*/

        //Update Configuration Timeout
        pPort->P2PCfg.ConfigTimeout[0] = pWFDSendGONegoRsp->MinimumConfigTimeout.GOTimeout;
        pPort->P2PCfg.ConfigTimeout[1] = pWFDSendGONegoRsp->MinimumConfigTimeout.ClientTimeout;

        //Update GO Capability
//      P2pMsUpdateGOCapability(pAd, pPort, pWFDSendGONegoRsp->GroupCapability);
        DBGPRINT(RT_DEBUG_TRACE,("%s:: GroupCap=%d. GRPCAP_OWNER=%d. GRPCAP_PERSISTENT=%d. GRPCAP_INTRA_BSS=%d. \n\t GRPCAP_CROSS_CONNECT=%d. GRPCAP_PERSISTENT_RECONNECT=%d. GRPCAP_GROUP_FORMING=%d. GRPCAP_LIMIT=%d.\n",
                __FUNCTION__, pWFDSendGONegoRsp->GroupCapability,
                MT_TEST_BIT(pWFDSendGONegoRsp->GroupCapability, GRPCAP_OWNER),
                MT_TEST_BIT(pWFDSendGONegoRsp->GroupCapability, GRPCAP_PERSISTENT),
                MT_TEST_BIT(pWFDSendGONegoRsp->GroupCapability, GRPCAP_INTRA_BSS),
                MT_TEST_BIT(pWFDSendGONegoRsp->GroupCapability, GRPCAP_CROSS_CONNECT),
                MT_TEST_BIT(pWFDSendGONegoRsp->GroupCapability, GRPCAP_PERSISTENT_RECONNECT),
                MT_TEST_BIT(pWFDSendGONegoRsp->GroupCapability, GRPCAP_GROUP_FORMING),
                MT_TEST_BIT(pWFDSendGONegoRsp->GroupCapability, GRPCAP_LIMIT)));

        //Update Group ID
        if (pWFDSendGONegoRsp->bUseGroupID)
        {
            PlatformMoveMemory(pPort->P2PCfg.DeviceAddress, pWFDSendGONegoRsp->GroupID.DeviceAddress, MAC_ADDR_LEN);
            pPort->P2PCfg.SSIDLen = (UCHAR) pWFDSendGONegoRsp->GroupID.SSID.uSSIDLength;
            PlatformMoveMemory(pPort->P2PCfg.SSID, pWFDSendGONegoRsp->GroupID.SSID.ucSSID, pPort->P2PCfg.SSIDLen);
        }
        //Create GO Nego Response
        P2pMsMakeGONegoRsp(pAd,
                        pPort,
                        pWFDSendGONegoRsp->PeerDeviceAddress,
                        pWFDSendGONegoRsp->DialogToken,
                        pWFDSendGONegoRsp->Status,
                        pWFDSendGONegoRsp->GroupOwnerIntent,
                        pWFDSendGONegoRsp->IntendedInterfaceAddress,
                        pWFDSendGONegoRsp->bUseGroupID,
                        &pWFDSendGONegoRsp->GroupID,
                        pWFDSendGONegoRsp->GroupCapability,
                        pOutBuffer,
                        &FrameLen);

        // Stay in formation state if status is no failure
        if (pWFDSendGONegoRsp->Status)
        {
            // Taken action on the peer to accept pairing (e.g. user prompt)
            if (pWFDSendGONegoRsp->Status != P2PSTATUS_PASSED)
            {
            // reset ConnectingMAC
            PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
                pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
            }
            else
            {
                pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_FORMATION;
                // Start blocking periodic scan timer
                pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
            }
            // Wehn I sent GoNego/Invite/Provision response frame, I still stay in my listen channel.
            // So no need to back to my listen channel
        }
        else
        {
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_FORMATION;
        // Start blocking periodic scan timer
        pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
        }

        // Start sending timer...
        // if no ack received for this sending frame after a given timeout, we will indicate a failure to NDIS
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = 0;

        // Copy MS Send Timeout to pAdapter and use below method to send action frame
        // 1. Stop sending action frame when receives ACK or acftion frame response and indicate success status to OS
        // 2. Stop sending action frame and indicate failure status to OS after timeout
        pPort->P2PCfg.MSSendTimeout = (pWFDSendGONegoRsp->uSendTimeout /100) - WFD_COMPLETE_SEND_TIMEOUT;

        //Additional IE
        PlatformMoveMemory(Add2Ptr(pOutBuffer, FrameLen),
                    Add2Ptr(pWFDSendGONegoRsp, pWFDSendGONegoRsp->uIEsOffset),
                    pWFDSendGONegoRsp->uIEsLength);
        FrameLen += pWFDSendGONegoRsp->uIEsLength;


        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Go Negociation Responce Length = %d. RspStatus = %d. timeout = %d . LatchCh = %d. PeerDeviceAddress = %x:%x:%x:%x:%x:%x \n",
                __FUNCTION__, FrameLen, pWFDSendGONegoRsp->Status, pWFDSendGONegoRsp->uSendTimeout, pAd->HwCfg.LatchRfRegs.Channel,
                pWFDSendGONegoRsp->PeerDeviceAddress[0], pWFDSendGONegoRsp->PeerDeviceAddress[1], pWFDSendGONegoRsp->PeerDeviceAddress[2],
                pWFDSendGONegoRsp->PeerDeviceAddress[3], pWFDSendGONegoRsp->PeerDeviceAddress[4], pWFDSendGONegoRsp->PeerDeviceAddress[5]));

#ifdef WFD_NEW_PUBLIC_ACTION
        //Free Old Buffered Data
        P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

        // Allocate new buffers
        PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
        pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
        
        if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
        {
            //Cache new Buffer
            PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
        }
#else
        // Kickoff TX. Direct OID
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
#endif /*WFD_NEW_PUBLIC_ACTION*/
    }while(FALSE);

    if (pOutBuffer)
        MlmeFreeMemory(pAd, pOutBuffer);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_SEND_GO_NEGOTIATION_RESPONSE_PARAMETERS) + pWFDSendGONegoRsp->uIEsLength;
    }

#ifdef WFD_NEW_PUBLIC_ACTION
    MlmeP2pMsPublicActionFrameSend(pAd, pPort, GO_NEGOCIATION_RSP);
#endif /*WFD_NEW_PUBLIC_ACTION*/

    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function generates GO negotiation confirmation packet to peer device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSendGONegoConfirm(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PUCHAR  pOutBuffer = NULL;
    ULONG   FrameLen = 0;
    ULONG   tmpByte = 0;
    PDOT11_SEND_GO_NEGOTIATION_CONFIRMATION_PARAMETERS  pWFDSendGONegoConfirm =
        (PDOT11_SEND_GO_NEGOTIATION_CONFIRMATION_PARAMETERS) InformationBuffer;

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSendGONegoConfirm->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SEND_GO_NEGOTIATION_CONFIRMATION_PARAMETERS_REVISION_1,
                    DOT11_SIZEOF_SEND_GO_NEGOTIATION_CONFIRMATION_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (MlmeAllocateMemory(pAd, &pOutBuffer) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        // Stay in formation state if status is no failure
        if (pWFDSendGONegoConfirm->Status)
        {
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;

            // reset ConnectingMAC
            PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);

            // Go back to my listen channel
            // Assume my listen ch is equal to common channel whether port 0 is connected or not.
            if (
//              (pPort->P2PCfg.RandomListenChTestBitOn == TRUE) &&
                (pAd->HwCfg.LatchRfRegs.Channel != pPort->P2PCfg.ListenChannel) &&
                (pPort->P2PCfg.ListenChannel == pPort->Channel) &&
                (pPort->P2PCfg.RestoreChannelInProgress == FALSE) &&
                (pPort->P2PCfg.SwitchChannelInProgress == FALSE))
            {
                pPort->P2PCfg.P2pCounter.SwitchingChannelCounter = 0;
                pPort->P2PCfg.RestoreChannelInProgress = TRUE;
            }
        }
        else
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_FORMATION;

        // Start sending timer...
        // if no ack received for this sending frame after a given timeout, we will indicate a failure to NDIS
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = 0;

        // Copy MS Send Timeout to pAdapter and use below method to send action frame
        // 1. Stop sending action frame when receives ACK or acftion frame response and indicate success status to OS
        // 2. Stop sending action frame and indicate failure status to OS after timeout
        pPort->P2PCfg.MSSendTimeout = (pWFDSendGONegoConfirm->uSendTimeout /100) - WFD_COMPLETE_SEND_TIMEOUT;

        //Update GO Capability
//      P2pMsUpdateGOCapability(pAd, pPort, pWFDSendGONegoConfirm->GroupCapability);
        DBGPRINT(RT_DEBUG_TRACE,("%s:: GroupCap=%d. GRPCAP_OWNER=%d. GRPCAP_PERSISTENT=%d. GRPCAP_INTRA_BSS=%d. \n\t GRPCAP_CROSS_CONNECT=%d. GRPCAP_PERSISTENT_RECONNECT=%d. GRPCAP_GROUP_FORMING=%d. GRPCAP_LIMIT=%d.\n",
                __FUNCTION__, pWFDSendGONegoConfirm->GroupCapability,
                MT_TEST_BIT(pWFDSendGONegoConfirm->GroupCapability, GRPCAP_OWNER),
                MT_TEST_BIT(pWFDSendGONegoConfirm->GroupCapability, GRPCAP_PERSISTENT),
                MT_TEST_BIT(pWFDSendGONegoConfirm->GroupCapability, GRPCAP_INTRA_BSS),
                MT_TEST_BIT(pWFDSendGONegoConfirm->GroupCapability, GRPCAP_CROSS_CONNECT),
                MT_TEST_BIT(pWFDSendGONegoConfirm->GroupCapability, GRPCAP_PERSISTENT_RECONNECT),
                MT_TEST_BIT(pWFDSendGONegoConfirm->GroupCapability, GRPCAP_GROUP_FORMING),
                MT_TEST_BIT(pWFDSendGONegoConfirm->GroupCapability, GRPCAP_LIMIT)));

        //Update Group ID
        if (pWFDSendGONegoConfirm->bUseGroupID)
        {
            PlatformMoveMemory(pPort->P2PCfg.DeviceAddress, pWFDSendGONegoConfirm->GroupID.DeviceAddress, MAC_ADDR_LEN);
            pPort->P2PCfg.SSIDLen = (UCHAR) pWFDSendGONegoConfirm->GroupID.SSID.uSSIDLength;
            PlatformMoveMemory(pPort->P2PCfg.SSID,pWFDSendGONegoConfirm->GroupID.SSID.ucSSID, pPort->P2PCfg.SSIDLen);
        }
        //Create GO Nego Confirmation
        P2pMsMakeGONegoConfirm(pAd,
                        pPort,
                        pWFDSendGONegoConfirm->PeerDeviceAddress,
                        pWFDSendGONegoConfirm->DialogToken,
                        pWFDSendGONegoConfirm->Status,
                        pWFDSendGONegoConfirm->bUseGroupID,
                        &pWFDSendGONegoConfirm->GroupID,
                        pWFDSendGONegoConfirm->GroupCapability,
                        pOutBuffer,
                        &FrameLen);


        //Additional IE
        PlatformMoveMemory(Add2Ptr(pOutBuffer, FrameLen),
                    Add2Ptr(pWFDSendGONegoConfirm, pWFDSendGONegoConfirm->uIEsOffset),
                    pWFDSendGONegoConfirm->uIEsLength);
        FrameLen += pWFDSendGONegoConfirm->uIEsLength;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Go Negociation Confirm Length = %d. RspStatus = %d. timeout = %d . LatchCh = %d. PeerDeviceAddress = %x:%x:%x:%x:%x:%x \n",
                __FUNCTION__, FrameLen, pWFDSendGONegoConfirm->Status, pWFDSendGONegoConfirm->uSendTimeout, pAd->HwCfg.LatchRfRegs.Channel,
                pWFDSendGONegoConfirm->PeerDeviceAddress[0], pWFDSendGONegoConfirm->PeerDeviceAddress[1], pWFDSendGONegoConfirm->PeerDeviceAddress[2],
                pWFDSendGONegoConfirm->PeerDeviceAddress[3], pWFDSendGONegoConfirm->PeerDeviceAddress[4], pWFDSendGONegoConfirm->PeerDeviceAddress[5]));

#ifdef WFD_NEW_PUBLIC_ACTION
        //Free Old Buffered Data
        P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

        // Allocate new buffers
        PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
        pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
        
        if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
        {
            //Cache new Buffer
            PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
        }
#else
        // Kickoff TX. Direct OID
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
#endif /*WFD_NEW_PUBLIC_ACTION*/
    }while(FALSE);

    if (pOutBuffer)
        MlmeFreeMemory(pAd, pOutBuffer);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_SEND_GO_NEGOTIATION_CONFIRMATION_PARAMETERS) + pWFDSendGONegoConfirm->uIEsLength;
    }

#ifdef WFD_NEW_PUBLIC_ACTION
    MlmeP2pMsPublicActionFrameSend(pAd, pPort, GO_NEGOCIATION_CONFIRM);
#endif /*WFD_NEW_PUBLIC_ACTION*/

    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function generates invite request packet to peer device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSendInviteReq(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PUCHAR  pOutBuffer = NULL;
    ULONG   FrameLen = 0;
    ULONG   tmpByte = 0;
    UCHAR   p2pindex = P2P_NOT_FOUND, PeerChannel = 0, DevIndexArray[6];
    UCHAR   i;
    PDOT11_SEND_INVITATION_REQUEST_PARAMETERS   pWFDSendInviteReq =
        (PDOT11_SEND_INVITATION_REQUEST_PARAMETERS) InformationBuffer;

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSendInviteReq->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SEND_INVITATION_REQUEST_PARAMETERS_REVISION_1,
                    DOT11_SIZEOF_SEND_INVITATION_REQUEST_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (MlmeAllocateMemory(pAd, &pOutBuffer) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

#ifdef MULTI_CHANNEL_SUPPORT
        // Stop Infra session if necessary
        P2pMsStartActionFrame(pAd, pPort);
#endif /*MULTI_CHANNEL_SUPPORT*/
        
        // Stay in invitation state
        pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_INVITATION;
        // Start blocking periodic scan timer
        pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
        // Start sending timer...
        // if no ack received for this sending frame after a given timeout, we will indicate a failure to NDIS
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = 0;

        // Copy MS Send Timeout to pAdapter and use below method to send action frame
        // 1. Stop sending action frame when receives ACK or acftion frame response and indicate success status to OS
        // 2. Stop sending action frame and indicate failure status to OS after timeout
        pPort->P2PCfg.MSSendTimeout = (pWFDSendInviteReq->uSendTimeout / 100) - WFD_COMPLETE_SEND_TIMEOUT;
        
        //Update Group ID
        PlatformMoveMemory(pPort->P2PCfg.DeviceAddress, pWFDSendInviteReq->GroupID.DeviceAddress, MAC_ADDR_LEN);
        pPort->P2PCfg.SSIDLen = (UCHAR) pWFDSendInviteReq->GroupID.SSID.uSSIDLength;
        PlatformMoveMemory(pPort->P2PCfg.SSID, pWFDSendInviteReq->GroupID.SSID.ucSSID, pPort->P2PCfg.SSIDLen);

        //Create invite request packet
        P2pMsMakeInviteReq(pAd,
                        pPort,
                        pWFDSendInviteReq->PeerDeviceAddress,
                        pWFDSendInviteReq->DialogToken,
                        pWFDSendInviteReq->InvitationFlags,
                        pWFDSendInviteReq->bUseSpecifiedOperatingChannel,
                        pWFDSendInviteReq->OperatingChannel,
                        pWFDSendInviteReq->bUseGroupBSSID,
                        pWFDSendInviteReq->GroupBSSID,
                        &pWFDSendInviteReq->GroupID,
                        pOutBuffer,
                        &FrameLen);

        //Additional IE
        PlatformMoveMemory(Add2Ptr(pOutBuffer, FrameLen),
                    Add2Ptr(pWFDSendInviteReq, pWFDSendInviteReq->uIEsOffset),
                    pWFDSendInviteReq->uIEsLength);
        FrameLen += pWFDSendInviteReq->uIEsLength;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Go Invite Request Length = %d. timeout= %d . LatchCh = %d. PeerDeviceAddress = %x:%x:%x:%x:%x:%x \n",
                __FUNCTION__, FrameLen, pWFDSendInviteReq->uSendTimeout, pAd->HwCfg.LatchRfRegs.Channel,
                pWFDSendInviteReq->PeerDeviceAddress[0], pWFDSendInviteReq->PeerDeviceAddress[1], pWFDSendInviteReq->PeerDeviceAddress[2],
                pWFDSendInviteReq->PeerDeviceAddress[3], pWFDSendInviteReq->PeerDeviceAddress[4], pWFDSendInviteReq->PeerDeviceAddress[5]));

        // ======================================>
        // Update connecting MAC address to the first array
        PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
        PlatformMoveMemory(pPort->P2PCfg.ConnectingMAC, pWFDSendInviteReq->PeerDeviceAddress, MAC_ADDR_LEN);
        pPort->P2PCfg.ConnectingIndex = 0xff;  // Win7 might use not 0xff
        // <======================================

        //
        // If the peer's listen channel is not equal to my listen channel,
        // Buffer this frame and send out when I arrive the peer listen channel
        //

        // If I become Client in Persistent Group, the INV request frame shall be sent to the
        // P2P device address of the P2P Group Owner and on the operating channel of the P2P Group

        if ((pWFDSendInviteReq->bUseGroupBSSID == FALSE) && (pWFDSendInviteReq->InvitationFlags.InvitationType == 1))
        {
            // Search by device address. Select the index direct to Group Owner.

            DevIndexArray[0] = P2P_NOT_FOUND;
            DevIndexArray[1] = P2P_NOT_FOUND;
            DevIndexArray[2] = P2P_NOT_FOUND;
            DevIndexArray[3] = P2P_NOT_FOUND;
            DevIndexArray[4] = P2P_NOT_FOUND;
            DevIndexArray[5] = P2P_NOT_FOUND;

            p2pindex = P2pDiscoTabSearch(pAd, NULL, pWFDSendInviteReq->PeerDeviceAddress, DevIndexArray);
            if (p2pindex < MAX_P2P_DISCOVERY_SIZE)
            {
                for (i = 0; i < sizeof(DevIndexArray); i++)
                {
                    if (DevIndexArray[i] == P2P_NOT_FOUND)
                        break;
                    if (pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].BeaconFrameLen > 0)
                    {
                        PeerChannel = pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].OpChannel;
                        // Use GO's interface address instaed!
                        PlatformMoveMemory(pPort->P2PCfg.ConnectingMAC, pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].TransmitterAddr, MAC_ADDR_LEN);
                        DBGPRINT(RT_DEBUG_ERROR, ("%s - I become Client in Persistent Group. Send out on the operating channel (DiscoEntry#%d - %x:%x:%x:%x:%x:%x) \n",
                                    __FUNCTION__, DevIndexArray[i],
                                    pPort->P2PCfg.ConnectingMAC[0][0], pPort->P2PCfg.ConnectingMAC[0][1], pPort->P2PCfg.ConnectingMAC[0][2],
                                    pPort->P2PCfg.ConnectingMAC[0][3], pPort->P2PCfg.ConnectingMAC[0][4], pPort->P2PCfg.ConnectingMAC[0][5]));

                        break;
                    }
                }
            }
        }
        else
        {
            p2pindex = P2pDiscoTabSearch(pAd, pWFDSendInviteReq->PeerDeviceAddress, NULL, NULL);
            if (p2pindex < MAX_P2P_DISCOVERY_SIZE)
            {
                PeerChannel = pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel;
            }
        }

        if (
//          (pPort->P2PCfg.RandomListenChTestBitOn == TRUE) &&
            (PeerChannel != 0) &&
            (PeerChannel != pPort->P2PCfg.ListenChannel) &&
            (MlmeSyncIsValidChannel(pAd, PeerChannel)) &&
            (PeerChannel != pAd->HwCfg.LatchRfRegs.Channel) &&
            (pPort->P2PCfg.SwitchChannelInProgress == FALSE))
        {
            //Free Old Buffered Data
            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

            // Allocate new buffers
            PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
            pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
			
            if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
            {
                //Cache new Buffer
                PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
                pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Go to the peer's listen channel(%d) to send the packet \n", __FUNCTION__, PeerChannel));

#ifdef WFD_NEW_PUBLIC_ACTION
#ifdef MULTI_CHANNEL_SUPPORT
                MlmeP2pMsSwitchCh(pAd, PeerChannel);
#else
                AsicSwitchChannel(pAd, PeerChannel, FALSE);
                //AsicLockChannel(pAd, PeerChannel);
#endif /*MULTI_CHANNEL_SUPPORT*/                
#else
                // Go to the peer's listen channel
                pPort->P2PCfg.P2pCounter.SwitchingChannelCounter = 0;
                pPort->P2PCfg.SwitchChannelInProgress = TRUE;
                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ);
                MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED);
#endif /*WFD_NEW_PUBLIC_ACTION*/    
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Alloc memory fail\n", __FUNCTION__));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
        }
        else
        {
#ifdef WFD_NEW_PUBLIC_ACTION
            //Free Old Buffered Data
            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

            // Allocate new buffers
            PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
            pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
            
            if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
            {
                //Cache new Buffer
                PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
            }
#else                   
            // Kickoff TX
            NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Stay in the current channel to send the packet (%d - %d) \n", __FUNCTION__, pPort->P2PCfg.ListenChannel, pAd->HwCfg.LatchRfRegs.Channel));
#endif /*WFD_NEW_PUBLIC_ACTION*/
        }
    }while(FALSE);

    if (pOutBuffer)
        MlmeFreeMemory(pAd, pOutBuffer);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_SEND_INVITATION_REQUEST_PARAMETERS) + pWFDSendInviteReq->uIEsLength;
    }

#ifdef WFD_NEW_PUBLIC_ACTION
    MlmeP2pMsPublicActionFrameSend(pAd, pPort, P2P_INVITE_REQ);
#endif /*WFD_NEW_PUBLIC_ACTION*/

    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function generates invite response packet to peer device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSendInviteRsp(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PUCHAR  pOutBuffer = NULL;
    ULONG   FrameLen = 0;
    ULONG   tmpByte = 0;
    PDOT11_SEND_INVITATION_RESPONSE_PARAMETERS  pWFDSendInviteRsp =
        (PDOT11_SEND_INVITATION_RESPONSE_PARAMETERS) InformationBuffer;

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSendInviteRsp->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SEND_INVITATION_RESPONSE_PARAMETERS_REVISION_1,
                    DOT11_SIZEOF_SEND_INVITATION_RESPONSE_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (MlmeAllocateMemory(pAd, &pOutBuffer) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

#ifdef MULTI_CHANNEL_SUPPORT
        // Stop Infra session if necessary
        P2pMsStartActionFrame(pAd, pPort);
#endif /*MULTI_CHANNEL_SUPPORT*/

        // Stay in invitation state if status is no failure
        if (pWFDSendInviteRsp->Status)
        {
            // Taken action on the peer to accept pairing (e.g. user prompt)
            if (pWFDSendInviteRsp->Status != P2PSTATUS_PASSED)
            {
                // reset ConnectingMAC
                PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
                pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
            }
            else
            {
                pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_INVITATION;
                // Start blocking periodic scan timer
                pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;      
            }
            
            // Wehn I send GoNego/Invite/Provision response frame, I still stay in my listen channel.
            // So no need to back to my listen channel
        }
        else
        {
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_INVITATION;
            // Start blocking periodic scan timer
            pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
        }

        // Start sending timer...
        // if no ack received for this sending frame after a given timeout, we will indicate a failure to NDIS
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = 0;

        // Copy MS Send Timeout to pAdapter and use below method to send action frame
        // 1. Stop sending action frame when receives ACK or acftion frame response and indicate success status to OS
        // 2. Stop sending action frame and indicate failure status to OS after timeout
        pPort->P2PCfg.MSSendTimeout = (pWFDSendInviteRsp->uSendTimeout /100) - WFD_COMPLETE_SEND_TIMEOUT;

        //Create invite response packet
        P2pMsMakeInviteRsp(pAd,
                        pPort,
                        pWFDSendInviteRsp->ReceiverDeviceAddress,
                        pWFDSendInviteRsp->DialogToken,
                        pWFDSendInviteRsp->Status,
                        pWFDSendInviteRsp->bUseSpecifiedOperatingChannel,
                        pWFDSendInviteRsp->OperatingChannel,
                        pWFDSendInviteRsp->bUseGroupBSSID,
                        pWFDSendInviteRsp->GroupBSSID,
                        pOutBuffer,
                        &FrameLen);

        //Additional IE
        PlatformMoveMemory(Add2Ptr(pOutBuffer, FrameLen),
                    Add2Ptr(pWFDSendInviteRsp, pWFDSendInviteRsp->uIEsOffset),
                    pWFDSendInviteRsp->uIEsLength);
        FrameLen += pWFDSendInviteRsp->uIEsLength;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Go Invite Response Length = %d. timeout= %d . LatchCh = % d. PeerDeviceAddress = %x:%x:%x:%x:%x:%x \n",
                __FUNCTION__, FrameLen, pWFDSendInviteRsp->uSendTimeout, pAd->HwCfg.LatchRfRegs.Channel,
                pWFDSendInviteRsp->ReceiverDeviceAddress[0], pWFDSendInviteRsp->ReceiverDeviceAddress[1], pWFDSendInviteRsp->ReceiverDeviceAddress[2],
                pWFDSendInviteRsp->ReceiverDeviceAddress[3], pWFDSendInviteRsp->ReceiverDeviceAddress[4], pWFDSendInviteRsp->ReceiverDeviceAddress[5]));

#ifdef WFD_NEW_PUBLIC_ACTION
        //Free Old Buffered Data
        P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

        // Allocate new buffers
        PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
        pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
        
        if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
        {
            //Cache new Buffer
            PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
        }
#else
        // Kickoff TX. Direct OID
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
#endif /*WFD_NEW_PUBLIC_ACTION*/
    }while(FALSE);

    if (pOutBuffer)
        MlmeFreeMemory(pAd, pOutBuffer);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_SEND_INVITATION_REQUEST_PARAMETERS) + pWFDSendInviteRsp->uIEsLength;
    }

#ifdef WFD_NEW_PUBLIC_ACTION
    MlmeP2pMsPublicActionFrameSend(pAd, pPort, P2P_INVITE_RSP);
#endif /*#ifdef WFD_NEW_PUBLIC_ACTION*/

    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function generates provision discovery request packet to peer device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSendProvDiscoReq(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PUCHAR  pOutBuffer = NULL;
    ULONG   FrameLen = 0;
    ULONG   tmpByte = 0;
    UCHAR   p2pindex = P2P_NOT_FOUND, PeerChannel = 0, DevIndexArray[6];
    UCHAR   GroupCapability = 0;
    UCHAR   i;

    PDOT11_SEND_PROVISION_DISCOVERY_REQUEST_PARAMETERS  pWFDSendProvDiscoReq =
        (PDOT11_SEND_PROVISION_DISCOVERY_REQUEST_PARAMETERS) InformationBuffer;


    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSendProvDiscoReq->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SEND_PROVISION_DISCOVERY_REQUEST_PARAMETERS_REVISION_1,
                    DOT11_SIZEOF_SEND_PROVISION_DISCOVERY_REQUEST_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (MlmeAllocateMemory(pAd, &pOutBuffer) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

#ifdef MULTI_CHANNEL_SUPPORT
        // Stop Infra session if necessary
        P2pMsStartActionFrame(pAd, pPort);
#endif /*MULTI_CHANNEL_SUPPORT*/

        if ((pWFDSendProvDiscoReq->bUseGroupID == FALSE) || (pPort->P2PCfg.P2pMsSatetPhase != P2pMs_STATE_INVITATION))
        {
            // Stay in start state
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_START_PROGRESS;
            // Start blocking periodic scan timer
            pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
        }

        // Start sending timer...
        // if no ack received for this sending frame after a given timeout, we will indicate a failure to NDIS
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = 0;

        // Copy MS Send Timeout to pAdapter and use below method to send action frame
        // 1. Stop sending action frame when receives ACK or acftion frame response and indicate success status to OS
        // 2. Stop sending action frame and indicate failure status to OS after timeout
        pPort->P2PCfg.MSSendTimeout = (pWFDSendProvDiscoReq->uSendTimeout / 100) - WFD_COMPLETE_SEND_TIMEOUT;


        //Update GO Capability
//      P2pMsUpdateGOCapability(pAd, pPort, pWFDSendProvDiscoReq->GroupCapability);
        DBGPRINT(RT_DEBUG_TRACE,("%s:: GroupCap=%d. GRPCAP_OWNER=%d. GRPCAP_PERSISTENT=%d. GRPCAP_INTRA_BSS=%d. \n\t GRPCAP_CROSS_CONNECT=%d. GRPCAP_PERSISTENT_RECONNECT=%d. GRPCAP_GROUP_FORMING=%d. GRPCAP_LIMIT=%d.\n",
                __FUNCTION__, pWFDSendProvDiscoReq->GroupCapability,
                MT_TEST_BIT(pWFDSendProvDiscoReq->GroupCapability, GRPCAP_OWNER),
                MT_TEST_BIT(pWFDSendProvDiscoReq->GroupCapability, GRPCAP_PERSISTENT),
                MT_TEST_BIT(pWFDSendProvDiscoReq->GroupCapability, GRPCAP_INTRA_BSS),
                MT_TEST_BIT(pWFDSendProvDiscoReq->GroupCapability, GRPCAP_CROSS_CONNECT),
                MT_TEST_BIT(pWFDSendProvDiscoReq->GroupCapability, GRPCAP_PERSISTENT_RECONNECT),
                MT_TEST_BIT(pWFDSendProvDiscoReq->GroupCapability, GRPCAP_GROUP_FORMING),
                MT_TEST_BIT(pWFDSendProvDiscoReq->GroupCapability, GRPCAP_LIMIT)));

        //Update Group ID
        if (pWFDSendProvDiscoReq->bUseGroupID)
        {
            PlatformMoveMemory(pPort->P2PCfg.DeviceAddress, pWFDSendProvDiscoReq->GroupID.DeviceAddress, MAC_ADDR_LEN);
            pPort->P2PCfg.SSIDLen = (UCHAR) pWFDSendProvDiscoReq->GroupID.SSID.uSSIDLength;
            PlatformMoveMemory(pPort->P2PCfg.SSID, pWFDSendProvDiscoReq->GroupID.SSID.ucSSID, pPort->P2PCfg.SSIDLen);
        }

        //Create Provision Discovery Request packet
        P2pMsMakeProvDiscoReq(pAd,
                        pPort,
                        pWFDSendProvDiscoReq->PeerDeviceAddress,
                        pWFDSendProvDiscoReq->DialogToken,
                        pWFDSendProvDiscoReq->bUseGroupID,
                        &pWFDSendProvDiscoReq->GroupID,
                        pWFDSendProvDiscoReq->GroupCapability,
                        pOutBuffer,
                        &FrameLen);

        //Additional IE
        PlatformMoveMemory(Add2Ptr(pOutBuffer, FrameLen),
                    Add2Ptr(pWFDSendProvDiscoReq, pWFDSendProvDiscoReq->uIEsOffset),
                    pWFDSendProvDiscoReq->uIEsLength);
        FrameLen += pWFDSendProvDiscoReq->uIEsLength;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Provision Discovery Request Length = %d. timeout = %d . LatchCh = %d. PeerDeviceAddress = %x:%x:%x:%x:%x:%x \n",
                __FUNCTION__, FrameLen, pWFDSendProvDiscoReq->uSendTimeout, pAd->HwCfg.LatchRfRegs.Channel,
                pWFDSendProvDiscoReq->PeerDeviceAddress[0], pWFDSendProvDiscoReq->PeerDeviceAddress[1], pWFDSendProvDiscoReq->PeerDeviceAddress[2],
                pWFDSendProvDiscoReq->PeerDeviceAddress[3], pWFDSendProvDiscoReq->PeerDeviceAddress[4], pWFDSendProvDiscoReq->PeerDeviceAddress[5]));

        // ======================================>
        // Update connecting MAC address to the first array
        PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
        PlatformMoveMemory(pPort->P2PCfg.ConnectingMAC, pWFDSendProvDiscoReq->PeerDeviceAddress, MAC_ADDR_LEN);
        pPort->P2PCfg.ConnectingIndex = 0xff; // Win7 might use not 0xff
        // <======================================

        //
        // If the peer's listen channel is not equal to my listen channel,
        // Buffer this frame and send out when I arrive the peer listen channel
        //

        // If connecting to a P2P Group, the PD request frame shall be sent to the
        // P2P device address of the P2P Group Owner and on the operating channel of the P2P Group
        if (pWFDSendProvDiscoReq->bUseGroupID)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - bUseGroupID(%d), DeviceId(%x %x %x %x %x %x), SSID(%s)\n", 
                                        __FUNCTION__,
                                        pWFDSendProvDiscoReq->bUseGroupID,
                                        pWFDSendProvDiscoReq->GroupID.DeviceAddress[0],
                                        pWFDSendProvDiscoReq->GroupID.DeviceAddress[1],
                                        pWFDSendProvDiscoReq->GroupID.DeviceAddress[2],
                                        pWFDSendProvDiscoReq->GroupID.DeviceAddress[3],
                                        pWFDSendProvDiscoReq->GroupID.DeviceAddress[4],
                                        pWFDSendProvDiscoReq->GroupID.DeviceAddress[5],
                                        pWFDSendProvDiscoReq->GroupID.SSID.ucSSID));
        }

        if (pWFDSendProvDiscoReq->bUseGroupID)
        {
            // Search by device address. Select the index direct to Group Owner.

            DevIndexArray[0] = P2P_NOT_FOUND;
            DevIndexArray[1] = P2P_NOT_FOUND;
            DevIndexArray[2] = P2P_NOT_FOUND;
            DevIndexArray[3] = P2P_NOT_FOUND;
            DevIndexArray[4] = P2P_NOT_FOUND;
            DevIndexArray[5] = P2P_NOT_FOUND;

            p2pindex = P2pDiscoTabSearch(pAd, NULL, pWFDSendProvDiscoReq->PeerDeviceAddress, DevIndexArray);
            
            if (p2pindex < MAX_P2P_DISCOVERY_SIZE)
            {
                for (i = 0; i < sizeof(DevIndexArray); i++)
                {
                    // Check if found
                    if (DevIndexArray[i] == P2P_NOT_FOUND)
                        continue;

                    DBGPRINT(RT_DEBUG_TRACE, ("DevIndexArray[%d]=%d - DeviceId(%x %x %x %x %x %x), SSID(%s)\n", 
                                                i, DevIndexArray[i],
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].DeviceId[0],
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].DeviceId[1],
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].DeviceId[2],
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].DeviceId[3],
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].DeviceId[4],
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].DeviceId[5],
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].Ssid));

                    //Check if GroupID match
                    if ((MAC_ADDR_EQUAL(pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].DeviceId, pWFDSendProvDiscoReq->GroupID.DeviceAddress) == FALSE) ||
                        (SSID_EQUAL(pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].Ssid, pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].SsidLen, pWFDSendProvDiscoReq->GroupID.SSID.ucSSID, pWFDSendProvDiscoReq->GroupID.SSID.uSSIDLength) == FALSE))
                    {
                        continue;
                    }
                    
                    if ((pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].BeaconFrameLen > 0) ||
                        (pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].ProbeRspFrameLen > 0))
                    {
                        PeerChannel = pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].OpChannel;
                        GroupCapability = pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].GroupCapability;

                        if ((PeerChannel == 0) && (GroupCapability & GRPCAP_OWNER))
                        { 
                            // Didn't get Beacon form GO, use it's listen channel as op channel
                            PeerChannel = pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].ListenChannel;
                        }
                        
                        // Use GO's interface address instaed!
                        PlatformMoveMemory(pPort->P2PCfg.ConnectingMAC, pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].TransmitterAddr, MAC_ADDR_LEN);
                        DBGPRINT(RT_DEBUG_ERROR, ("%s - Connecting to a P2P Group(%d). Send out on the operating channel (ch=%d) (DiscoEntry#%d - %x:%x:%x:%x:%x:%x) \n", 
                                    __FUNCTION__, pWFDSendProvDiscoReq->bUseGroupID, PeerChannel, DevIndexArray[i],
                                    pPort->P2PCfg.ConnectingMAC[0][0], pPort->P2PCfg.ConnectingMAC[0][1], pPort->P2PCfg.ConnectingMAC[0][2],
                                    pPort->P2PCfg.ConnectingMAC[0][3], pPort->P2PCfg.ConnectingMAC[0][4], pPort->P2PCfg.ConnectingMAC[0][5]));

                        break;
                    }
                }
            }
        }
        else
        {
            p2pindex = P2pDiscoTabSearch(pAd, pWFDSendProvDiscoReq->PeerDeviceAddress, NULL, NULL);
            if (p2pindex < MAX_P2P_DISCOVERY_SIZE)
            {
                PeerChannel = pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel;
            }
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s - PeerChannel(%d), MyListenCh(%d), MyLatchRfCh(%d), SwitchChInProgress(%d)\n", 
                                    __FUNCTION__,
                                    PeerChannel,
                                    pPort->P2PCfg.ListenChannel,
                                    pAd->HwCfg.LatchRfRegs.Channel,
                                    (pPort->P2PCfg.SwitchChannelInProgress == TRUE)));

        if (
//          (pPort->P2PCfg.RandomListenChTestBitOn == TRUE) &&
            (PeerChannel != 0) &&
            //(PeerChannel != pPort->P2PCfg.ListenChannel) && // CLI(11) then STA(8), CLI disconenct and connect again, this will reject channel switch
            (MlmeSyncIsValidChannel(pAd, PeerChannel)) &&
            (PeerChannel != pAd->HwCfg.LatchRfRegs.Channel) &&
            (pPort->P2PCfg.SwitchChannelInProgress == FALSE))
        {
            //Free Old Buffered Data
            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

            // Allocate new buffers
            PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
	     pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
            if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
            {
                //Cache new Buffer
                PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
                pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Go to the peer's listen channel(%d) to send the packet \n", __FUNCTION__, PeerChannel));

#ifdef WFD_NEW_PUBLIC_ACTION
#ifdef MULTI_CHANNEL_SUPPORT
                AsicSwitchChannel(pPort, PeerChannel, PeerChannel, BW_20, FALSE);
#else
                //AsicSwitchChannel(pAd, PeerChannel, FALSE);
                //AsicLockChannel(pAd, PeerChannel);
#endif /*MULTI_CHANNEL_SUPPORT*/                
#else
                // Go to the peer's listen channel
                pPort->P2PCfg.P2pCounter.SwitchingChannelCounter = 0;
                pPort->P2PCfg.SwitchChannelInProgress = TRUE;
                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ);
                MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED);
#endif /*WFD_NEW_PUBLIC_ACTION*/        
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Alloc memory fail\n", __FUNCTION__));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
        }
        else
        {
#ifdef WFD_NEW_PUBLIC_ACTION
            //Free Old Buffered Data
            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

            // Allocate new buffers
            PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
            pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
            
            if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
            {
                //Cache new Buffer
                PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
            }
#else           
            // Kickoff TX
            NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Stay in the current channel to send the packet (%d - %d - %d - %d) \n", 
                                        __FUNCTION__, 
                                        pPort->P2PCfg.ListenChannel, 
                                        pAd->HwCfg.LatchRfRegs.Channel,
                                        PeerChannel,
                                        pPort->P2PCfg.SwitchChannelInProgress));      
#endif /*WFD_NEW_PUBLIC_ACTION*/
        }
    }while(FALSE);

    if (pOutBuffer)
        MlmeFreeMemory(pAd, pOutBuffer);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_SEND_PROVISION_DISCOVERY_REQUEST_PARAMETERS) + pWFDSendProvDiscoReq->uIEsLength;
    }

#ifdef WFD_NEW_PUBLIC_ACTION
    MlmeP2pMsPublicActionFrameSend(pAd, pPort, P2P_PROVISION_REQ);
#endif /*WFD_NEW_PUBLIC_ACTION*/

    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function generates provision discovery response packet to peer device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSendProvDiscoRsp(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PUCHAR  pOutBuffer = NULL;
    ULONG   FrameLen = 0;
    ULONG   tmpByte = 0;
    PDOT11_SEND_PROVISION_DISCOVERY_RESPONSE_PARAMETERS pWFDSendProvDiscoRsp =
        (PDOT11_SEND_PROVISION_DISCOVERY_RESPONSE_PARAMETERS) InformationBuffer;

    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]", __FUNCTION__, pPort->PortNumber));

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSendProvDiscoRsp->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SEND_PROVISION_DISCOVERY_RESPONSE_PARAMETERS_REVISION_1,
                    DOT11_SIZEOF_SEND_PROVISION_DISCOVERY_RESPONSE_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (MlmeAllocateMemory(pAd, &pOutBuffer) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

#ifdef MULTI_CHANNEL_SUPPORT
        // Stop Infra session if necessary
        P2pMsStartActionFrame(pAd, pPort);
#endif /*MULTI_CHANNEL_SUPPORT*/

        if (pPort->P2PCfg.P2pMsSatetPhase != P2pMs_STATE_INVITATION)
        {
            // Stay in start state
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_START_PROGRESS;
            // Start blocking periodic scan timer
            pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
        }

        // Start sending timer...
        // if no ack received for this sending frame after a given timeout, we will indicate a failure to NDIS
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = 0;

        // Copy MS Send Timeout to pAdapter and use below method to send action frame
        // 1. Stop sending action frame when receives ACK or acftion frame response and indicate success status to OS
        // 2. Stop sending action frame and indicate failure status to OS after timeout
        pPort->P2PCfg.MSSendTimeout = (pWFDSendProvDiscoRsp->uSendTimeout /100) - WFD_COMPLETE_SEND_TIMEOUT;

        //Create Provision Discovery response packet
        P2pMsMakeProvDiscoRsp(pAd,
                        pPort,
                        pWFDSendProvDiscoRsp->ReceiverDeviceAddress,
                        pWFDSendProvDiscoRsp->DialogToken,
                        pOutBuffer,
                        &FrameLen);

        //Additional IE
        PlatformMoveMemory(Add2Ptr(pOutBuffer, FrameLen),
                    Add2Ptr(pWFDSendProvDiscoRsp, pWFDSendProvDiscoRsp->uIEsOffset),
                    pWFDSendProvDiscoRsp->uIEsLength);
        FrameLen += pWFDSendProvDiscoRsp->uIEsLength;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Provision Discovery Response Length = %d, timeout =% d . LatchCh = %d. PeerDeviceAddress = %x:%x:%x:%x:%x:%x \n",
                __FUNCTION__, FrameLen, pWFDSendProvDiscoRsp->uSendTimeout, pAd->HwCfg.LatchRfRegs.Channel,
                pWFDSendProvDiscoRsp->ReceiverDeviceAddress[0], pWFDSendProvDiscoRsp->ReceiverDeviceAddress[1], pWFDSendProvDiscoRsp->ReceiverDeviceAddress[2],
                pWFDSendProvDiscoRsp->ReceiverDeviceAddress[3], pWFDSendProvDiscoRsp->ReceiverDeviceAddress[4], pWFDSendProvDiscoRsp->ReceiverDeviceAddress[5]));
#ifdef WFD_NEW_PUBLIC_ACTION
        //Free Old Buffered Data
        P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);

        // Allocate new buffers
       PlatformAllocateMemory(pAd,  &pPort->P2PCfg.pRequesterBufferedFrame, FrameLen);
        pPort->P2PCfg.RequesterBufferedSize = (USHORT)FrameLen;
        
        if(pPort->P2PCfg.pRequesterBufferedFrame != NULL)
        {
            //Cache new Buffer
            PlatformMoveMemory(pPort->P2PCfg.pRequesterBufferedFrame, pOutBuffer, FrameLen);
        }
#else
        // Kickoff TX. Direct OID
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
#endif /*WFD_NEW_PUBLIC_ACTION*/
    }while(FALSE);

    if (pOutBuffer)
        MlmeFreeMemory(pAd, pOutBuffer);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = DOT11_SIZEOF_SEND_PROVISION_DISCOVERY_RESPONSE_PARAMETERS_REVISION_1 + pWFDSendProvDiscoRsp->uIEsLength;
    }

#ifdef WFD_NEW_PUBLIC_ACTION
    MlmeP2pMsPublicActionFrameSend(pAd, pPort, P2P_PROVISION_RSP);
#endif /*WFD_NEW_PUBLIC_ACTION*/
    
    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function parse relevant information elements for GO negotiation
        request packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        PeerAddr            Peer device address
        GOIntent            GO intent
        InterfaceAddr       Interface Address of the Device
        pOutBuffer          Pointer to outgoing packet
        pTotalFrameLen      Total length of outgoing packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsMakeGONegoReq(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_GO_INTENT  GOIntent,
    IN DOT11_MAC_ADDRESS    InterfaceAddr,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    PUCHAR  pDest;
    ULONG   Length;
    ULONG   P2pIeLen = 0;
    UCHAR   Intent;
    DOT11_WFD_CHANNEL   tmpChannel;
    UCHAR   P2pCapability[2];
    UCHAR   i= 0;

    // BSSID = DA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, PeerAddr);

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = GO_NEGOCIATION_REQ;

    //pFrame->Token = pPort->P2PCfg.Token++;
    pFrame->Token = Token;

    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = sizeof(P2P_PUBLIC_FRAME) - 1;
    pDest = pFrame->Octet;

    // attach subelementID= 2
    P2pCapability[0] = pPort->PortCfg.P2pCapability[0];
    P2pCapability[1] = GroupCapability;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, P2pCapability, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Capability\n"));

    // attach subelementID= 4
    Intent = GOIntent.Intent;
    Intent <<= 1;
    Intent |= 0x00 | GOIntent.TieBreaker;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OWNER_INTENT, &Intent, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Intent = %d, TieBreaker = %d \n", GOIntent.Intent, GOIntent.TieBreaker));

    // attach subelementID= 5
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &pPort->P2PCfg.ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Configuration Timeout %d %d\n", pPort->P2PCfg.ConfigTimeout[0], pPort->P2PCfg.ConfigTimeout[1]));

    //  attach subelementID= 6.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_LISTEN_CHANNEL, &pPort->P2PCfg.ListenChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Listen Channel %d\n", pPort->P2PCfg.ListenChannel));

    // Microsoft WFD doesn't support Extend-Listen
/*  if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - insert SUBID_P2P_EXT_LISTEN_TIMING. \n"));
        //  attach subelementID= 17.
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_EXT_LISTEN_TIMING, NULL, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }
*/
    // attach subelementID= 9
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_INTERFACE_ADDR, InterfaceAddr, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Intented Interface Address = %x:%x:%x:%x:%x:%x \n", InterfaceAddr[0], InterfaceAddr[1], InterfaceAddr[2], InterfaceAddr[3], InterfaceAddr[4], InterfaceAddr[5]));

    // attach subelementID= 11
    //Copy the default channel list to CommcomSetChannelList
    pPort->P2PCfg.CommcomSetChannelList.ChannelNr = pAd->HwCfg.ChannelListNum;
    for (i = 0; i < (pPort->P2PCfg.CommcomSetChannelList.ChannelNr); i++)
    {
        pPort->P2PCfg.CommcomSetChannelList.Channel[i] = pAd->HwCfg.ChannelList[i].Channel;
    }

    // if ExtSTA had connected and P2pMs had not connected yet, use ExtSTA channel
    if (INFRA_ON(pAd->PortList[PORT_0]) &&  (pAd->pP2pCtrll->P2pMsConnectedStatus == P2pMs_CONNECTED_STATUS_OFF))
    {
        pPort->P2PCfg.CommcomSetChannelList.ChannelNr = 1;
        pPort->P2PCfg.CommcomSetChannelList.Channel[0] = pPort->P2PCfg.GroupChannel/*pPort->Channel*/;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List. ExtSTA had connected, use ExtSTA's (ch=%d) as Channel List\n", pPort->P2PCfg.CommcomSetChannelList.Channel[0]));
    }
    
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, (PUCHAR)&pPort->P2PCfg.CommcomSetChannelList, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List Nr = %d\n", pPort->P2PCfg.CommcomSetChannelList.ChannelNr));

    //  attach subelementID= 13.
    //  used to uniquely reference to device address
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Device Info\n"));

    // attach subelementID= 17
    //Let Miniport decide operating channel
    tmpChannel.CountryRegionString[0] = 0x47;
    tmpChannel.CountryRegionString[1] = 0x4c;
    tmpChannel.CountryRegionString[2] = 0x04;
    tmpChannel.OperatingClass = ChannelToClass(pPort->P2PCfg.GroupChannel, COUNTRY_GLOBAL);
    tmpChannel.ChannelNumber = pPort->P2PCfg.GroupChannel;

    // pPort->P2PCfg.GroupChannel will be updated to STA ch if STA is linking up without any MS P2P connection.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &tmpChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - ChannelNumber = %d \n", tmpChannel.ChannelNumber));

    pFrame->Length = (UCHAR)P2pIeLen;
}


/*
    ========================================================================
    Routine Description:
        This function parse relevant information elements for GO negotiation
        response packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        PeerAddr            Peer device address
        Token               Dialog Token
        Status              GO negotiation status
        GOIntent            GO intent
        InterfaceAddr       Interface Address
        bUseGroupID         Use group ID indicator
        pGroupID            Pointer to group ID
        pOutBuffer          Pointer to outgoing packet
        pTotalFrameLen      Total length of outgoing packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsMakeGONegoRsp(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_STATUS_CODE Status,
    IN DOT11_WFD_GO_INTENT  GOIntent,
    IN DOT11_MAC_ADDRESS    InterfaceAddr,
    IN BOOLEAN              bUseGroupID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    PUCHAR  pDest;
    ULONG   Length;
    ULONG   P2pIeLen = 0;
    UCHAR   Intent = 0;
    DOT11_WFD_CHANNEL   tmpChannel;
    UCHAR   ConfigTimeout[2];
    UCHAR   P2pCapability[2];
    UCHAR   i= 0;

#if (USE_P2P_SPEC == 1)
    // P2P Spec. section2.4.3 p.23
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, pPort->CurrentAddress);
#else
    // BSSID = SA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, (bUseGroupID ? pPort->P2PCfg.DeviceAddress : pPort->CurrentAddress));
#endif

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

    // attach subelementID= 0
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, &Status, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Status %d \n", Status));

    // attach subelementID= 2
    P2pCapability[0] = pPort->PortCfg.P2pCapability[0];
    P2pCapability[1] = GroupCapability;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP,  P2pCapability, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Capability %d \n", Status));

    // attach subelementID= 4
    Intent = GOIntent.Intent;
    Intent <<= 1;
    Intent |= 0x00 | GOIntent.TieBreaker;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OWNER_INTENT, &Intent, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Intent = %d, TieBreaker = %d \n", GOIntent.Intent, GOIntent.TieBreaker));

    // attach subelementID= 5
    ConfigTimeout[0] = pPort->P2PCfg.ConfigTimeout[0];
    ConfigTimeout[1] = pPort->P2PCfg.ConfigTimeout[1];

#if 0 // For NDIS WFD_Group_ext.htm
    if (bUseGroupID)
        ConfigTimeout[1] = 0; // set client configuration timeout to zero
    else
        ConfigTimeout[0] = 0;// set go configuration timeout to zero
#endif

    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Configuration Timeout %d %d\n", ConfigTimeout[0], ConfigTimeout[1]));

    // attach subelementID= 17
    //Let Miniport decide operating channel
    tmpChannel.CountryRegionString[0] = 0x47;
    tmpChannel.CountryRegionString[1] = 0x4c;
    tmpChannel.CountryRegionString[2] = 0x04;
    tmpChannel.OperatingClass = ChannelToClass(pPort->P2PCfg.GroupChannel, COUNTRY_GLOBAL);
    tmpChannel.ChannelNumber = pPort->P2PCfg.GroupChannel;

    // pPort->P2PCfg.GroupChannel will be updated to STA ch if STA is linking up without any MS P2P connection.
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &tmpChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - ChannelNumber = %d \n", tmpChannel.ChannelNumber));

    // attach subelementID= 9
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_INTERFACE_ADDR, InterfaceAddr, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Intented Interface Address = %x:%x:%x:%x:%x:%x \n", InterfaceAddr[0], InterfaceAddr[1], InterfaceAddr[2], InterfaceAddr[3], InterfaceAddr[4], InterfaceAddr[5]));

    // attach subelementID= 11
    //Copy the default channel list to CommcomSetChannelList
    pPort->P2PCfg.CommcomSetChannelList.ChannelNr = pAd->HwCfg.ChannelListNum;
    for (i = 0; i < (pPort->P2PCfg.CommcomSetChannelList.ChannelNr); i++)
    {
        pPort->P2PCfg.CommcomSetChannelList.Channel[i] = pAd->HwCfg.ChannelList[i].Channel;
    }
    
    // if ExtSTA had connected and P2pMs had not connected yet, use ExtSTA channel
    if (INFRA_ON(pAd->PortList[PORT_0]) &&  (pAd->pP2pCtrll->P2pMsConnectedStatus == P2pMs_CONNECTED_STATUS_OFF))
    {
        pPort->P2PCfg.CommcomSetChannelList.ChannelNr = 1;
        pPort->P2PCfg.CommcomSetChannelList.Channel[0] = pPort->P2PCfg.GroupChannel/*pPort->Channel*/;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List. ExtSTA had connected, use ExtSTA's (ch=%d) as Channel List\n", pPort->P2PCfg.CommcomSetChannelList.Channel[0]));
    }
    
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, (PUCHAR)&pPort->P2PCfg.CommcomSetChannelList, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List Nr = %d\n", pPort->P2PCfg.CommcomSetChannelList.ChannelNr));

    //  attach subelementID= 13.
    //  used to uniquely reference to device address
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Device Info\n"));

    // attach subelementID= 15
    if(bUseGroupID)
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pGroupID->DeviceAddress, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;

        // Succeed and I become GO. Update my operting channel here!
        if ((Status == P2PSTATUS_SUCCESS) && (tmpChannel.ChannelNumber != 0))
        {
            pPort->P2PCfg.GroupOpChannel = tmpChannel.ChannelNumber;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become GO !! GroupOpChannel = %d \n", pPort->P2PCfg.GroupOpChannel));
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Group ID = %x:%x:%x  \n", pGroupID->DeviceAddress[3], pGroupID->DeviceAddress[4], pGroupID->DeviceAddress[5]));

    pFrame->Length = (UCHAR)P2pIeLen;
}


/*
    ========================================================================
    Routine Description:
        This function parse relevant information elements for GO negotiation
        confirmation packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        PeerAddr            Peer device address
        Token               Dialog token
        Status              GO negotiation status
        bUseGroupID         Use group ID indicator
        pGroupID            Pointer to group ID
        pOutBuffer          Pointer to outgoing packet
        pTotalFrameLen      Total length of outgoing packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsMakeGONegoConfirm(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_STATUS_CODE Status,
    IN BOOLEAN              bUseGroupID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
    PUCHAR  pDest;
    ULONG   Length;
    ULONG   P2pIeLen = 0;
    UCHAR   Intent = 0;
    DOT11_WFD_CHANNEL   tmpChannel;
    UCHAR   P2pCapability[2];
    UCHAR   p2pindex = P2P_NOT_FOUND;
    UCHAR   i= 0;

#if (USE_P2P_SPEC == 1)
    // P2P Spec. section2.4.3 p.23
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, PeerAddr);
#else
    // BSSID = DA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, (bUseGroupID ? pPort->P2PCfg.DeviceAddress : PeerAddr));
#endif

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = GO_NEGOCIATION_CONFIRM;
    pFrame->Token = Token;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = sizeof(P2P_PUBLIC_FRAME) - 1;
    pDest = pFrame->Octet;

    // attach subelementID= 0
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, &Status, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Status %d \n", Status));

    // attach subelementID= 2
    P2pCapability[0] = pPort->PortCfg.P2pCapability[0];
    P2pCapability[1] = GroupCapability;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP,  P2pCapability, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Capability \n"));


    // attach subelementID= 17
    //Let Miniport decide operating channel
    tmpChannel.CountryRegionString[0] = 0x47;
    tmpChannel.CountryRegionString[1] = 0x4c;
    tmpChannel.CountryRegionString[2] = 0x04;
    // assign new op channel after GoN
#if 0   
    if (pPort->P2PCfg.CommcomSetChannelList.ChannelNr == 1)
    {
        pPort->P2PCfg.GroupChannel = pPort->P2PCfg.CommcomSetChannelList.Channel[0];
        pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
    }
#else
    if (((pPort->P2PCfg.CommcomSetChannelList.ChannelNr == 1) && (INFRA_ON(pAd->PortList[PORT_0]) == FALSE) && (pAd->pP2pCtrll->P2pMsConnectedStatus == P2pMs_CONNECTED_STATUS_OFF)) ||
        ((pPort->P2PCfg.CommcomSetChannelList.ChannelNr == 1) && (pPort->P2PCfg.CommcomSetChannelList.Channel[0] == pPort->Channel)))
    {
        pPort->P2PCfg.GroupChannel = pPort->P2PCfg.CommcomSetChannelList.Channel[0];
        pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
        DBGPRINT(RT_DEBUG_TRACE, ("Infra on(%d), P2P conn sts(%d), ch list's ch (%d), common ch(%d)\n", 
                                    INFRA_ON(pAd->PortList[PORT_0]) == TRUE ? 1 : 0,
                                    pAd->pP2pCtrll->P2pMsConnectedStatus,
                                    pPort->P2PCfg.CommcomSetChannelList.Channel[0],
                                    pPort->Channel));
    }
#endif
    tmpChannel.OperatingClass = ChannelToClass(pPort->P2PCfg.GroupChannel, COUNTRY_GLOBAL);
    tmpChannel.ChannelNumber = pPort->P2PCfg.GroupChannel;
    // If I am Client, use GO's channel that set in the Go Nego Rsp.
    p2pindex = P2pDiscoTabSearch(pAd, PeerAddr, NULL, NULL);
    if (p2pindex < MAX_P2P_DISCOVERY_SIZE)
    {
        if ((bUseGroupID == FALSE) && (pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].GroupChannel != 0))
        {
            tmpChannel.ChannelNumber = pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].GroupChannel;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Client !!\n"));
        }
    }
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &tmpChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - ChannelNumber = %d \n", tmpChannel.ChannelNumber));

    // attach subelementID= 11
    //Copy the default channel list to CommcomSetChannelList
    pPort->P2PCfg.CommcomSetChannelList.ChannelNr = pAd->HwCfg.ChannelListNum;
    for (i = 0; i < (pPort->P2PCfg.CommcomSetChannelList.ChannelNr); i++)
    {
        pPort->P2PCfg.CommcomSetChannelList.Channel[i] = pAd->HwCfg.ChannelList[i].Channel;
    }
    
    // if ExtSTA had connected and P2pMs had not connected yet, use ExtSTA channel
    if (INFRA_ON(pAd->PortList[PORT_0]) &&  (pAd->pP2pCtrll->P2pMsConnectedStatus == P2pMs_CONNECTED_STATUS_OFF))
    {
        pPort->P2PCfg.CommcomSetChannelList.ChannelNr = 1;
        pPort->P2PCfg.CommcomSetChannelList.Channel[0] = pPort->P2PCfg.GroupChannel;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List. ExtSTA had connected, use ExtSTA's (ch=%d) as Channel List\n", pPort->P2PCfg.CommcomSetChannelList.Channel[0]));
    }
    
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, (PUCHAR)&pPort->P2PCfg.CommcomSetChannelList, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List Nr = %d\n", pPort->P2PCfg.CommcomSetChannelList.ChannelNr));

    // attach subelementID= 15
    if(bUseGroupID)
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pGroupID->DeviceAddress, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;

        // Succeed and I become GO. Update my operting channel here!
        if ((Status == P2PSTATUS_SUCCESS) && (tmpChannel.ChannelNumber != 0))
        {
            pPort->P2PCfg.GroupOpChannel = tmpChannel.ChannelNumber;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become GO !! GroupOpChannel = %d \n", pPort->P2PCfg.GroupOpChannel));
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Group ID = %x:%x:%x:%x:%x:%x  \n",
                                  pGroupID->DeviceAddress[0],
                                  pGroupID->DeviceAddress[1],
                                  pGroupID->DeviceAddress[2],
                                  pGroupID->DeviceAddress[3],
                                  pGroupID->DeviceAddress[4],
                                  pGroupID->DeviceAddress[5]));

    pFrame->Length = (UCHAR)P2pIeLen;
}


/*
    ========================================================================
    Routine Description:
        This function parse relevant information elements for invite request
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        PeerAddr            Peer device address
        InviteFlag          Invitation flag
        bUseSpecifiedOperatingChannel   Use OS defined OP channel indicator
        OperatingChannel    OP channel
        bUseGroupBSSID      Use group BSSID indicator
        GroupBSSID          Group BSSID
        pGroupID            Pointer to group ID
        pOutBuffer          Pointer to outgoing packet
        pTotalFrameLen      Total length of outgoing packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsMakeInviteReq(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_INVITATION_FLAGS InviteFlag,
    IN BOOLEAN              bUseSpecifiedOperatingChannel,
    IN DOT11_WFD_CHANNEL    OperatingChannel,
    IN BOOLEAN              bUseGroupBSSID,
    IN DOT11_MAC_ADDRESS    GroupBSSID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    ULONG           P2pIeLen = 0;
    PUCHAR          pDest;
    ULONG           Length;
    UCHAR           tmpFlag;
    DOT11_WFD_CHANNEL   tmpChannel;
    UCHAR           ConfigTimeout[2];
    PMP_PORT      pStaPort = NULL, pClientPort = NULL, pClient2Port = NULL;
    BOOLEAN         IsWorkingNetwork = FALSE;
    UCHAR           i = 0;

    pFrame = (PP2P_PUBLIC_FRAME) pOutBuffer;

    ConfigTimeout[0] = pPort->P2PCfg.ConfigTimeout[0];
    ConfigTimeout[1] = pPort->P2PCfg.ConfigTimeout[1];

#if (USE_P2P_SPEC == 1)
    // P2P Spec. section2.4.3 p.23
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, PeerAddr);
#else
    // BSSID = DA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, (bUseGroupBSSID ? GroupBSSID : PeerAddr));
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s - TO %x %x %x %x %x %x. \n",
                __FUNCTION__, PeerAddr[0], PeerAddr[1], PeerAddr[2],PeerAddr[3],PeerAddr[4],PeerAddr[5]));

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_INVITE_REQ;

    //pFrame->Token = pPort->P2PCfg.Token++;
    pFrame->Token = Token;

    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = sizeof(P2P_PUBLIC_FRAME) - 1;
    pDest = pFrame->Octet;

    // attach subelementID= 5
    // I am GO or Client. No need config time. So set zero.
#if 0 // For NDIS WFD_Group_ext.htm
    if ((InviteFlag.InvitationType == 0) && (bUseGroupBSSID))
    {
        ConfigTimeout[0] = 0;
        ConfigTimeout[1] = 0;
    }
#endif

    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Configuration Timeout %d %d\n", ConfigTimeout[0], ConfigTimeout[1]));

    // attach subelementID= 18.
    tmpFlag = 0x00 | InviteFlag.InvitationType;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_INVITE_FLAG, &tmpFlag, pDest);
    pPort->P2PCfg.LastSentInviteFlag = tmpFlag;
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Invite Flag  %d\n", tmpFlag));

    // Found out all of clients
    pStaPort = pAd->PortList[PORT_0];
    pClientPort = MlmeSyncGetP2pClientPort(pAd, 0);
    pClient2Port= MlmeSyncGetP2pClientPort(pAd, 1);
    // Check if another active wireless network
    if (INFRA_ON(pStaPort) ||
        (pPort->P2PCfg.bGOStart) ||
        (pClientPort && INFRA_ON(pClientPort)) ||
        (pClient2Port && INFRA_ON(pClient2Port)))
    {
        IsWorkingNetwork = TRUE;
    }

    // attach subelementID= 17
    // My GO shall carry in operation channel
    // My Client may carry in operation channel
    if((bUseSpecifiedOperatingChannel) &&
        (!PlatformEqualMemory(&OperatingChannel, ZERO_MAC_ADDR, 5)) &&
        (MlmeSyncIsValidChannel(pAd, OperatingChannel.ChannelNumber)) &&
        (IsWorkingNetwork == FALSE))
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &OperatingChannel, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;

        // update operation channel to local
        pPort->P2PCfg.GroupOpChannel  = OperatingChannel.ChannelNumber;
        pPort->P2PCfg.GroupChannel    = OperatingChannel.ChannelNumber;

        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Operating Channel = %d.\n", OperatingChannel.ChannelNumber));
    }
    else
    {
        //Let Miniport decide operating channel
        tmpChannel.CountryRegionString[0] = 0x47;
        tmpChannel.CountryRegionString[1] = 0x4c;
        tmpChannel.CountryRegionString[2] = 0x04;

        // P2P network already operates in common channel
        if (IsWorkingNetwork && (pPort->P2PCfg.GroupOpChannel != pPort->Channel))
        {
            pPort->P2PCfg.GroupOpChannel  = pPort->Channel;
            pPort->P2PCfg.GroupChannel    = pPort->Channel;
        }

        tmpChannel.OperatingClass = ChannelToClass(pPort->P2PCfg.GroupOpChannel, COUNTRY_GLOBAL);
        tmpChannel.ChannelNumber = pPort->P2PCfg.GroupOpChannel;

        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &tmpChannel, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Operating Channel = %d. GroupOpChannel = %d. \n", tmpChannel.ChannelNumber, pPort->P2PCfg.GroupOpChannel));
    }

    // attach subelementID= 7.
    // My GO shall carry in group bssid
    // My Client may carry in group bssid
    if(bUseGroupBSSID)
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_BSSID, GroupBSSID, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Group BSSID = %x:%x:%x:%x:%x:%x \n",
            GroupBSSID[0], GroupBSSID[1], GroupBSSID[2],GroupBSSID[3],GroupBSSID[4],GroupBSSID[5]));
    }

    // attach subelementID= 11
    //Copy the default channel list to CommcomSetChannelList
    pPort->P2PCfg.CommcomSetChannelList.ChannelNr = pAd->HwCfg.ChannelListNum;
    for (i = 0; i < pAd->HwCfg.ChannelListNum; i++)
    {
        pPort->P2PCfg.CommcomSetChannelList.Channel[i] = pAd->HwCfg.ChannelList[i].Channel;
    }
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, (PUCHAR)&pPort->P2PCfg.CommcomSetChannelList, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List Nr = %d\n", pPort->P2PCfg.CommcomSetChannelList.ChannelNr));

    // attach subelementID= 15
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pGroupID->DeviceAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Group ID = %x:%x:%x  \n", pGroupID->DeviceAddress[3], pGroupID->DeviceAddress[4], pGroupID->DeviceAddress[5]));

    //  attach subelementID= 13.
    //  used to uniquely reference to device address
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Device Info\n"));

    // Set total p2p IE Length
    pFrame->Length = (UCHAR)P2pIeLen;
}


/*
    ========================================================================
    Routine Description:
        This function parse relevant information elements for invite response
        packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        PeerAddr            Peer device address
        Token               Dialog token
        Status              Invitation status
        bUseSpecifiedOperatingChannel   Use OS defined OP channel indicator
        OperatingChannel    OP channel
        bUseGroupBSSID      Use group BSSID indicator
        GroupBSSID          Group BSSID
        pOutBuffer          Pointer to outgoing packet
        pTotalFrameLen      Total length of outgoing packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsMakeInviteRsp(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_STATUS_CODE Status,
    IN BOOLEAN              bUseSpecifiedOperatingChannel,
    IN DOT11_WFD_CHANNEL    OperatingChannel,
    IN BOOLEAN              bUseGroupBSSID,
    IN DOT11_MAC_ADDRESS    GroupBSSID,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    ULONG           P2pIeLen = 0;
    PUCHAR          pDest;
    ULONG           Length;
    DOT11_WFD_CHANNEL   tmpChannel;
    PMP_PORT      pStaPort = NULL, pClientPort = NULL, pClient2Port = NULL;
    BOOLEAN         IsWorkingNetwork = FALSE;


    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;

#if (USE_P2P_SPEC == 1)
    // P2P Spec. section2.4.3 p.23
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, pPort->CurrentAddress);
#else
    // BSSID = SA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, (bUseGroupBSSID ? GroupBSSID : pPort->CurrentAddress));
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s -  TO %x %x %x %x %x %x. \n",
                __FUNCTION__, PeerAddr[0], PeerAddr[1], PeerAddr[2],PeerAddr[3],PeerAddr[4],PeerAddr[5]));

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_INVITE_RSP;
    pFrame->Token = Token;
    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = sizeof(P2P_PUBLIC_FRAME) - 1;
    pDest = pFrame->Octet;

    // attach subelementID= 0
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_STATUS, &Status, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Status %d \n", Status));


    // attach subelementID= 5
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CONFIG_TIMEOUT, &pPort->P2PCfg.ConfigTimeout[0], pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Configuration Timeout %d %d\n", pPort->P2PCfg.ConfigTimeout[0], pPort->P2PCfg.ConfigTimeout[1]));

#if 1
    if(Status == DOT11_WFD_STATUS_SUCCESS)
    {
        // Found out all of clients
        pStaPort = pAd->PortList[PORT_0];
        pClientPort = MlmeSyncGetP2pClientPort(pAd, 0);
        pClient2Port= MlmeSyncGetP2pClientPort(pAd, 1);
        // Check if another active wireless network
        if (INFRA_ON(pStaPort) ||
            (pPort->P2PCfg.bGOStart) ||
            (pClientPort && INFRA_ON(pClientPort)) ||
            (pClient2Port && INFRA_ON(pClient2Port)))
        {
            IsWorkingNetwork = TRUE;
        }

        // attach subelementID= 17
        // Shall carry in operation channel if the peer is GO
        // May carry in operation channel if the peer is Client
        if((bUseSpecifiedOperatingChannel) &&
            (!PlatformEqualMemory(&OperatingChannel, ZERO_MAC_ADDR, 5)) &&
            (MlmeSyncIsValidChannel(pAd, OperatingChannel.ChannelNumber)) &&
            (IsWorkingNetwork == FALSE))
        {
            //Parse operating channel information from OS
            Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &OperatingChannel, pDest);
            pDest += Length;
            P2pIeLen += Length;
            *pTotalFrameLen += Length;

            // update operation channel to local
            pPort->P2PCfg.GroupOpChannel  = OperatingChannel.ChannelNumber;
            pPort->P2PCfg.GroupChannel    = OperatingChannel.ChannelNumber;

            DBGPRINT(RT_DEBUG_TRACE, (" P2P - Operating Channel = %d.\n", OperatingChannel.ChannelNumber));
        }
        else
        {
            //Let Miniport decide operating channel
            tmpChannel.CountryRegionString[0] = 0x47;
            tmpChannel.CountryRegionString[1] = 0x4c;
            tmpChannel.CountryRegionString[2] = 0x04;

            // P2P network already operates in common channel
            if (IsWorkingNetwork && (pPort->P2PCfg.GroupOpChannel != pPort->Channel))
            {
                pPort->P2PCfg.GroupOpChannel  = pPort->Channel;
                pPort->P2PCfg.GroupChannel    = pPort->Channel;
            }

            tmpChannel.OperatingClass = ChannelToClass(pPort->P2PCfg.GroupOpChannel, COUNTRY_GLOBAL);
            tmpChannel.ChannelNumber = pPort->P2PCfg.GroupOpChannel;

            Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &tmpChannel, pDest);
            pDest += Length;
            P2pIeLen += Length;
            *pTotalFrameLen += Length;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - Operating Channel = %d. GroupOpChannel = %d. \n", tmpChannel.ChannelNumber, pPort->P2PCfg.GroupOpChannel));
        }

        // attach subelementID= 7.
        // GO shall carry in group bssid under success status
        // Client may carry in group bssid under success status
        if(bUseGroupBSSID)
        {
            Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_BSSID, GroupBSSID, pDest);
            pDest += Length;
            P2pIeLen += Length;
            *pTotalFrameLen += Length;
            DBGPRINT(RT_DEBUG_TRACE, (" P2P - Group BSSID = %x:%x:%x:%x:%x:%x \n",
                GroupBSSID[0], GroupBSSID[1], GroupBSSID[2],GroupBSSID[3],GroupBSSID[4],GroupBSSID[5]));
        }

        // attach subelementID= 11
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, (PUCHAR)&pPort->P2PCfg.CommcomSetChannelList, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List Nr = %d\n", pPort->P2PCfg.CommcomSetChannelList.ChannelNr));

    }
#else   // TODO: wait to remove
    // attach subelementID= 17
    if(bUseSpecifiedOperatingChannel)
    {
        //Parse operating channel information from OS
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &OperatingChannel, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Operating Channel = %d. P2PCfg.GroupChannel = %d.\n", OperatingChannel.ChannelNumber, pPort->P2PCfg.GroupChannel));
    }
    else if(Status == DOT11_WFD_STATUS_SUCCESS)
    {
        //Let Miniport decide operating channel
        tmpChannel.CountryRegionString[0] = 0x47;
        tmpChannel.CountryRegionString[1] = 0x4c;
        tmpChannel.CountryRegionString[2] = 0x04;
        tmpChannel.OperatingClass = ChannelToClass(pPort->P2PCfg.GroupChannel, COUNTRY_GLOBAL);
        tmpChannel.ChannelNumber = pPort->P2PCfg.GroupChannel;

        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_OP_CHANNEL, (PUCHAR) &tmpChannel, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Operating Channel = %d \n", tmpChannel.ChannelNumber));
    }

    // attach subelementID= 7.
    if(bUseGroupBSSID)
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_BSSID, GroupBSSID, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - Group BSSID = %x:%x:%x:%x:%x \n",
            GroupBSSID[0], GroupBSSID[1], GroupBSSID[2],GroupBSSID[3],GroupBSSID[4],GroupBSSID[5]));
    }

    // attach subelementID= 11
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CHANNEL_LIST, &pPort->P2PCfg.GroupChannel, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Channel List\n"));
#endif

    // Set total p2p IE Length
    pFrame->Length = (UCHAR)P2pIeLen;
}


/*
    ========================================================================
    Routine Description:
        This function parse relevant information elements for provision
        discovery request packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        PeerAddr            Peer device address
        bUseGroupID         Use group ID indicator
        pGroupID            Pointer to group ID
        pOutBuffer          Pointer to outgoing packet
        pTotalFrameLen      Total length of outgoing packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsMakeProvDiscoReq(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN BOOLEAN              bUseGroupID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    ULONG           P2pIeLen = 0;
    PUCHAR          pDest;
    ULONG           Length;
    UCHAR           P2pCapability[2];

    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;

#if (USE_P2P_SPEC == 1)
    // P2P Spec. section2.4.3 p.23
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, PeerAddr);
#else
    // BSSID = DA or SA?
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, (bUseGroupID ? pGroupID->DeviceAddress : PeerAddr));
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s - TO %x %x %x %x %x %x. \n",
                __FUNCTION__, PeerAddr[0], PeerAddr[1], PeerAddr[2],PeerAddr[3],PeerAddr[4],PeerAddr[5]));

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_PROVISION_REQ;

    //pFrame->Token = pPort->P2PCfg.Token++;
    pFrame->Token = Token;

    pFrame->ElementID = IE_VENDOR_SPECIFIC;
    PlatformMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);
    P2pIeLen = 4;
    *pTotalFrameLen = sizeof(P2P_PUBLIC_FRAME) - 1;
    pDest = pFrame->Octet;

    // attach subelementID= 2
    P2pCapability[0] = pPort->PortCfg.P2pCapability[0];
    P2pCapability[1] = GroupCapability;
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_CAP, P2pCapability, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Capability\n"));

    //  attach subelementID= 13.
    //  used to uniquely reference to device address
    Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_DEVICE_INFO, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, pDest);
    pDest += Length;
    P2pIeLen += Length;
    *pTotalFrameLen += Length;
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Device Info\n"));

    // attach subelementID= 15
    if(bUseGroupID)
    {
        Length = InsertP2PSubelmtTlv(pAd, pPort, SUBID_P2P_GROUP_ID, pGroupID->DeviceAddress, pDest);
        pDest += Length;
        P2pIeLen += Length;
        *pTotalFrameLen += Length;
    }
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - Group ID = %x:%x:%x  \n", pGroupID->DeviceAddress[3], pGroupID->DeviceAddress[4], pGroupID->DeviceAddress[5]));

    // Set total p2p IE Length
    pFrame->Length = (UCHAR)P2pIeLen;
}


/*
    ========================================================================
    Routine Description:
        This function parse relevant information elements for provision
        discovery response packet

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        PeerAddr            Peer device address
        Token               Dialog token
        pOutBuffer          Pointer to outgoing packet
        pTotalFrameLen      Total length of outgoing packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsMakeProvDiscoRsp(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen)
{
    PP2P_PUBLIC_FRAME   pFrame;
    ULONG           P2pIeLen = 0;
    //PUCHAR            pDest;
    //ULONG         Length;

    pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;

#if (USE_P2P_SPEC == 1)
    // P2P Spec. section2.4.3 p.23
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, pPort->CurrentAddress);
#else
    // BSSID = DA
    ActHeaderInit(pAd, pPort, &pFrame->p80211Header, PeerAddr, PeerAddr);
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s - TO %x %x %x %x %x %x. \n",
                __FUNCTION__, PeerAddr[0], PeerAddr[1], PeerAddr[2],PeerAddr[3],PeerAddr[4],PeerAddr[5]));

    pFrame->Category = CATEGORY_PUBLIC;
    pFrame->Action = P2P_ACTION;
    PlatformMoveMemory((pFrame->OUI), P2POUIBYTE, 4);
    // OUISubtype
    pFrame->Subtype = P2P_PROVISION_RSP;
    pFrame->Token = Token;

    *pTotalFrameLen = FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID);
}


/*
    ========================================================================
    Routine Description:
        This function returns a list of discovered devices to OS when
        OID_DOT11_WFD_ENUM_DEVICE_LIST is called

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation response packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:
        ndisStatus          NDIS Status
    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsEnumerateDeviceList(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID InformationBuffer,
    IN  ULONG OutputBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_BYTE_ARRAY   pdot11ByteArray;
    ULONG               RemainingBytes;
    ULONG               tmpNeeded, tmpWritten;
    PUCHAR              pCurrPtr;
    UCHAR               TotalDeviceNumber = 0;
    do
    {
        *BytesWritten = 0;
        *BytesNeeded = 0;

        //
        // Check enough space for the 3 fields of the DOT11_BYTE_ARRAY
        //
        if (OutputBufferLength < FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer))
        {
            *BytesNeeded = sizeof(DOT11_BYTE_ARRAY);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        pdot11ByteArray = (PDOT11_BYTE_ARRAY)InformationBuffer;
        PlatformZeroMemory(pdot11ByteArray, OutputBufferLength);

        pdot11ByteArray->uNumOfBytes = 0;
        pdot11ByteArray->uTotalNumOfBytes = 0;

        MP_ASSIGN_NDIS_OBJECT_HEADER(pdot11ByteArray->Header,
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_DEVICE_ENTRY_BYTE_ARRAY_REVISION_1,
                                    sizeof(DOT11_BYTE_ARRAY));

        RemainingBytes = OutputBufferLength - FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
        pCurrPtr = pdot11ByteArray->ucBuffer;

        DBGPRINT(RT_DEBUG_TRACE, ("%s(1) OutputBufferLength(%d), RemainingBytes(%d), Header(%d)\n",
                                    __FUNCTION__,
                                    OutputBufferLength,
                                    RemainingBytes,
                                    FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer)));

        P2pMsBuildDeviceList(pAd, pPort, pCurrPtr, RemainingBytes, &tmpWritten, &tmpNeeded, &TotalDeviceNumber);


        DBGPRINT(RT_DEBUG_TRACE, ("%s(2) RemainingBytes(%d), tmpWritten(%d), tmpNeeded(%d), TotalDeviceNumber(%d)\n",
                                    __FUNCTION__,
                                    RemainingBytes,
                                    tmpWritten,
                                    tmpNeeded,
                                    TotalDeviceNumber));

        if(tmpNeeded != 0)
        {
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            pdot11ByteArray->uTotalNumOfBytes = RemainingBytes + tmpNeeded;
            pdot11ByteArray->uNumOfBytes = tmpWritten;
            *BytesWritten = pdot11ByteArray->uNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
            *BytesNeeded = pdot11ByteArray->uTotalNumOfBytes +  FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
            DBGPRINT(RT_DEBUG_ERROR, ("%s::Error::OS(%d), uNumOfBytes(%d), uTotalNumOfBytes(%d)\n", __FUNCTION__, OutputBufferLength,  *BytesWritten, *BytesNeeded));
            break;
        }
        pdot11ByteArray->uNumOfBytes = tmpWritten;
        pdot11ByteArray->uTotalNumOfBytes = tmpWritten;
        *BytesWritten = pdot11ByteArray->uNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
        *BytesNeeded  = pdot11ByteArray->uTotalNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
        DBGPRINT(RT_DEBUG_TRACE, ("%s::Ok::OS(%d), uNumOfBytes(%d), uTotalNumOfBytes(%d)\n", __FUNCTION__, OutputBufferLength,  *BytesWritten, *BytesNeeded));

    }while(FALSE);

    DBGPRINT(RT_DEBUG_ERROR, ("%s ==> OutputBufferLength(=%ld)\n", __FUNCTION__, OutputBufferLength));

    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function reads device capability from buffer and sets Wi-Fi
        Direct configs when OID_DOT11_WFD_DEVICE_CAPABILITY is called

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation response packet
        InBufferLen         Size of GO negotiation request packet
        BytesRead           Number of bytes read from buffer
        BytesNeed           Number of bytes needed from buffer
    Return Value:
        ndisStatus          NDIS Status
    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetDeviceCapability(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_DEVICE_CAPABILITY_CONFIG pWFDDevCapConfig =
        (PDOT11_WFD_DEVICE_CAPABILITY_CONFIG) InformationBuffer;
    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDDevCapConfig->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_WFD_DEVICE_CAPABILITY_CONFIG_REVISION_1,
                    DOT11_SIZEOF_WFD_DEVICE_CAPABILITY_CONFIG_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        // All bits of P2pCapability[0] are including. So we can init it here.
        pPort->PortCfg.P2pCapability[0] = 0;

        /*
            Service Discovery Bit
            The Service Discovery field shall be set to 1 if the P2P
            Device supports Service Discovery, and is set to 0
            otherwise.
        */
        if(pWFDDevCapConfig->bServiceDiscoveryEnabled == TRUE)
            MT_SET_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_SD);

        /*
            P2P Client Discoverability Bit
            Within a P2P Group Info subelement and a (re)
            association request frame the P2P Client Discoverability
            field shall be set to 1 when the P2P Device supports
            P2P Client Discoverability, and is set to 0 otherwise.
            This field shall be reserved and set to 0 in all other
            frames or uses.
        */
        if(pWFDDevCapConfig->bClientDiscoverabilityEnabled == TRUE)
            MT_SET_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_CLIENT_DISCOVER);

        /*
            Concurrent Operation Bit
            The Concurrent Operation field shall be set to 1 when
            the P2P Device supports Concurrent Operation with
            WLAN, and is set to 0 otherwise.
        */
        if(pWFDDevCapConfig->bConcurrentOperationSupported == TRUE)
            MT_SET_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_CLIENT_CONCURRENT);

        /*
            P2P Infrastructure Managed Bit
            The P2P Infrastructure Managed field shall be set to 1
            when the P2P interface of the P2P Device is capable of
            being managed by the WLAN (infrastructure network)
            based on P2P Coexistence Parameters, and set to 0
            otherwise.
        */
        if(pWFDDevCapConfig->bInfrastructureManagementEnabled == TRUE)
            MT_SET_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_INFRA_MANAGED);
        
        /*
            P2P Device Limit Bit
            The P2P Device Limit field shall be set to 1 when the
            P2P Device is unable to participate in additional P2P
            Groups, and set to 0 otherwise.
        */
        if(pWFDDevCapConfig->bDeviceLimitReached == TRUE)
            MT_SET_BIT(pPort->PortCfg.P2pCapability[0],DEVCAP_DEVICE_LIMIT);
    
        /*
            P2P Invitation
            Procedure
            The P2P Invitation Procedure field shall be set to 1 if the
            P2P Device is capable of processing P2P Invitation
            Procedure signaling, and set to 0 otherwise.
        */
        if(pWFDDevCapConfig->bInvitationProcedureEnabled == TRUE)
            MT_SET_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_INVITE);
        
        // update wps supported version
        if (pWFDDevCapConfig->WPSVersionsEnabled == DOT11_WPS_VERSION_2_0)
            pPort->P2PCfg.WPSVersionsEnabled = DOT11_WPS_VERSION_2_0;// 0x02
        else
            pPort->P2PCfg.WPSVersionsEnabled = DOT11_WPS_VERSION_1_0;// 0x01

        // For USB only, USB is late to update beacon and fails to pass WFD TC
        // Update GO beacon if GO is started
        if ((pPort->PortType == WFD_GO_PORT) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        {
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_UPDATE_BEACON, NULL, 0);
        }
        
        DBGPRINT(RT_DEBUG_TRACE,("%s:: DEVCAP_SD=%d. DEVCAP_CLIENT_DISCOVER=%d. DEVCAP_CLIENT_CONCURRENT=%d. \n\t DEVCAP_INFRA_MANAGED=%d. DEVCAP_DEVICE_LIMIT=%d. DEVCAP_INVITE=%d \n", 
                __FUNCTION__,
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_SD), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_CLIENT_DISCOVER),
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_CLIENT_CONCURRENT), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_INFRA_MANAGED), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_DEVICE_LIMIT), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[0], DEVCAP_INVITE)));
        
    }while(FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_DEVICE_CAPABILITY_CONFIG);
    }
    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function reads group capability from buffer and sets Wi-Fi
        Direct configs when OID_DOT11_WFD_GROUP_OWNER_CAPABILITY is called

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation response packet
        InBufferLen         Size of GO negotiation request packet
        BytesRead           Number of bytes read from buffer
        BytesNeed           Number of bytes needed from buffer
    Return Value:
        ndisStatus          NDIS Status
    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetGOCapability(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_GROUP_OWNER_CAPABILITY_CONFIG pWFDGOCapConfig =
        (PDOT11_WFD_GROUP_OWNER_CAPABILITY_CONFIG) InformationBuffer;
    UCHAR   GroupCap;
    
    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDGOCapConfig->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_WFD_GROUP_OWNER_CAPABILITY_CONFIG_REVISION_1,
                    DOT11_SIZEOF_WFD_GROUP_CAPABILITY_CONFIG_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        GroupCap = 0;

        /*
            P2P Group Owner Bit
            The P2P Group Owner field shall be set to 1 when the
            P2P Device is operating as a Group Owner, and set to 0
            otherwise.
        */
        // Driver must maintain P2P Group Owner Bit by itself.

        /*
            Persistent P2P Group Bit
            The Persistent P2P Group field shall be set to 1 when
            the P2P Device is hosting, or intends to host, a
            persistent P2P Group, and set to 0 otherwise.
        */
        if(pWFDGOCapConfig->bPersistentGroupEnabled == TRUE)
            MT_SET_BIT(GroupCap, GRPCAP_PERSISTENT);
        
        /*
            Intra-BSS Bit
            Distribution
            The Intra-BSS Distribution field shall be set to 1 if the
            P2P Device is a P2P Group Owner and provides a data
            distribution service between Clients in the P2P Group.
            The Intra-BSS Distribution field shall be set to 0, if the
            P2P Device is not a P2P Group Owner, or is not
            providing such a data distribution service.
        */  
        if(pWFDGOCapConfig->bIntraBSSDistributionSupported == TRUE)
            MT_SET_BIT(GroupCap, GRPCAP_INTRA_BSS);

        /*
            Cross Connection Bit
            The Cross Connection field shall be set to 1 if the P2P
            Device is a P2P Group Owner and provides cross
            connection between the P2P Group and a WLAN. The
            Cross Connection field shall be set to 0 if the P2P
            Device is not a P2P Group Owner, or is not providing a
            cross connection service.
        */
        if(pWFDGOCapConfig->bCrossConnectionSupported == TRUE)
            MT_SET_BIT(GroupCap, GRPCAP_CROSS_CONNECT);
        
        /*
            Persistent Reconnect Bit
            The Persistent Reconnect field shall be set to 1 when
            the P2P Device is hosting, or intends to host, a
            persistent P2P Group that allows reconnection without
            user intervention, and set to 0 otherwise.
        */
        if(pWFDGOCapConfig->bPersistentReconnectSupported == TRUE)
            MT_SET_BIT(GroupCap, GRPCAP_PERSISTENT_RECONNECT);

        /*
            Group Formation Bit
            The Group Formation field shall be set to 1 when the
            P2P Device is operating as a Group Owner in the
            Provisioning phase of Group Formation, and set to 0
            otherwise 
        */
        if(pWFDGOCapConfig->bGroupFormationEnabled == TRUE)
            MT_SET_BIT(GroupCap, GRPCAP_GROUP_FORMING);
        
        /*
            P2P Group Limit Bit
            The P2P Group Limit field shall be set to 1 when the
            P2P Group Owner is unable to add additional Clients to
            its P2P Group, and set to 0 otherwise.
        */
        if (pWFDGOCapConfig->uMaximumGroupLimit == 0)
            MT_SET_BIT(GroupCap, GRPCAP_LIMIT);
        DBGPRINT(RT_DEBUG_TRACE,("%s:: WFDGOCapConfig->uMaximumGroupLimit=%d.\n", __FUNCTION__, pWFDGOCapConfig->uMaximumGroupLimit));

        // Save max of group limit on GO port
        if (pPort->PortType == WFD_GO_PORT)
            pPort->P2PCfg.MaximumGroupLimit = min(pWFDGOCapConfig->uMaximumGroupLimit, MAX_P2P_GO_TABLE_SIZE);
        
        // Turn on Group Owner Bit in GO port when start GO?
        if (pPort->PortType == WFD_GO_PORT)
            MT_SET_BIT(GroupCap, GRPCAP_OWNER);

        // Update P2pCapability
        pPort->PortCfg.P2pCapability[1] = GroupCap;
        
        // For USB only, USB is late to update beacon and fails to pass WFD TC
        // Update GO beacon if GO is started
        if ((pPort->PortType == WFD_GO_PORT) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        {
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_UPDATE_BEACON, NULL, 0);
        }

        DBGPRINT(RT_DEBUG_TRACE,("%s:: GRPCAP_OWNER=%d. GRPCAP_PERSISTENT=%d. GRPCAP_INTRA_BSS=%d. GRPCAP_CROSS_CONNECT=%d. \n\t GRPCAP_PERSISTENT_RECONNECT=%d. GRPCAP_GROUP_FORMING=%d. GRPCAP_LIMIT=%d.\n", 
                __FUNCTION__,
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_OWNER), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_PERSISTENT),
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_INTRA_BSS), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_CROSS_CONNECT), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_PERSISTENT_RECONNECT), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_GROUP_FORMING), 
                MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_LIMIT)));
    }while(FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_GROUP_OWNER_CAPABILITY_CONFIG);
    }
    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function update group capability from Action Frames

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation response packet

    Return Value:
        ndisStatus          NDIS Status
    IRQL =

    Note:
    ========================================================================
*/
VOID P2pMsUpdateGOCapability(
    IN PMP_ADAPTER                pAd,
    IN PMP_PORT                   pPort,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCap)
{
    pPort->PortCfg.P2pCapability[1] = GroupCap;
    
    DBGPRINT(RT_DEBUG_TRACE,("%s:: GroupCap=%d. GRPCAP_OWNER=%d. GRPCAP_PERSISTENT=%d. GRPCAP_INTRA_BSS=%d. \n\t GRPCAP_CROSS_CONNECT=%d. GRPCAP_PERSISTENT_RECONNECT=%d. GRPCAP_GROUP_FORMING=%d. GRPCAP_LIMIT=%d.\n", 
            __FUNCTION__, GroupCap,
            MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_OWNER), 
            MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_PERSISTENT),
            MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_INTRA_BSS), 
            MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_CROSS_CONNECT), 
            MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_PERSISTENT_RECONNECT), 
            MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_GROUP_FORMING), 
            MT_TEST_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_LIMIT)));

}

// Not currently used. Allows us more control
#if 0 
/*
    ========================================================================
    Routine Description:
        This function query/set P2P listen channel

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer               Pointer to P2P listen channel request
        InBufferLen         Size of P2P listen channel request
        BytesRead           Number of bytes read from buffer
        BytesNeed           Number of bytes needed from buffer
    Return Value:
        ndisStatus          NDIS Status
    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetListenChannel(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_DEVICE_LISTEN_CHANNEL pWFDListenChInfo =
        (PDOT11_WFD_DEVICE_LISTEN_CHANNEL) InformationBuffer;
    
    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDListenChInfo->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_WFD_DEVICE_LISTEN_CHANNEL_REVISION_1,
                    DOT11_SIZEOF_WFD_DEVICE_LISTEN_CHANNEL_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        // TODO: [hm] 2011-10-14 set the listen channel
        
        DBGPRINT(RT_DEBUG_TRACE,("%s::OID_DOT11_WFD_DEVICE_LISTEN_CHANNEL(%d)\n", __FUNCTION__));
    } while (FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_DEVICE_INFO);
    }
    
    return ndisStatus;
}
#endif
/*
    ========================================================================
    Routine Description:
        This function reads P2P scan parameters and triggers scan procedure

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to P2P device discovery request
        InBufferLen         Size of P2P device discovery request
        BytesRead           Number of bytes read from buffer
        BytesNeed           Number of bytes needed from buffer
    Return Value:
        ndisStatus          NDIS Status
    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetDeviceDiscovery(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
    PDOT11_WFD_DISCOVER_REQUEST pWFDDisReq =
        (PDOT11_WFD_DISCOVER_REQUEST)InformationBuffer;
    PUCHAR  pDevId = NULL;
    ULONG   StartScanRound = 1; // as default
    UCHAR   NumRequestedType = 0,  RequestedDeviceType[MAX_P2P_FILTER_LIST][P2P_DEVICE_TYPE_LEN];
    BOOLEAN IsWscIE = FALSE;
    UCHAR   i = 0;

    // For WHCK8224 JoinExsiting GO case, WHCK didn't set port type GO side triggered scan and P2P device side failed to reply probe rsp
    pPort->PortType = WFD_DEVICE_PORT;
    pPort->PortSubtype = PORTSUBTYPE_P2PDevice;

    // For WHCK8224 case2, Next TC will report previous scan result
    //P2pDiscoTabInit(pAd);

    if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
    {
        P2PExitWhckCpuCheck(pAd);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s DT=%x, ST=%d, TO=%d, DevFilterListOffset=%d, NumOfDevFilter=%d, IeOffset=%d, IeLen=%d, Legacy=%d\n",
                                  __FUNCTION__,
                                  pWFDDisReq->DiscoverType,
                                  pWFDDisReq->ScanType,
                                  pWFDDisReq->uDiscoverTimeout,
                                  //pWFDDisReq->uDeviceIDListOffset,
                                  //pWFDDisReq->uNumOfDeviceIDs,
                                  pWFDDisReq->uDeviceFilterListOffset,
                                  pWFDDisReq->uNumDeviceFilters,
                                  pWFDDisReq->uIEsOffset,
                                  pWFDDisReq->uIEsLength,
                                  pWFDDisReq->bForceScanLegacyNetworks));
    do
    {
        pPort->P2PCfg.DiscoverTriggerType = P2pMs_DISCOVER_BY_OS;
        
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDDisReq->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_WFD_DISCOVER_REQUEST_REVISION_1,
                    DOT11_SIZEOF_WFD_DISCOVER_REQUEST_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        // Don't go to non-force SCAN during p2p connection 
        if ((pWFDDisReq->DiscoverType & dot11_wfd_discover_type_forced) == 0)
        {
            // 1. Not in idle state
            // 2. Too short scan time (< 1se)
            if((pPort->P2PCfg.P2PDiscoProvState > P2P_ENABLE_LISTEN_ONLY) ||
                (pPort->P2PCfg.P2pMsSatetPhase > P2pMs_STATE_IDLE) ||
                (pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand < WFD_BLOCKING_SCAN_TIMEOUT) ||
                ((pWFDDisReq->uDiscoverTimeout / 100) < 10) ||
                (pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms < 300))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - DO quick report for non-force scan during p2p connection(state= %s, msstate= %s, counter1=%d, counter2=%d) \n", 
                        __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2PDiscoProvState), decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase), pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand, pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms));

                ndisStatus = NDIS_STATUS_INDICATION_REQUIRED;
                *BytesRead = sizeof(DOT11_WFD_DISCOVER_REQUEST)
                            + pWFDDisReq->uIEsLength
                            + (sizeof(DOT11_WFD_DISCOVER_DEVICE_FILTER) *  pWFDDisReq->uNumDeviceFilters);

                // quick discovery report after 1 sec
                pPort->P2PCfg.P2pCounter.QuickDiscoveryListUpdateCounter = 10;// unit: 100 ms
                MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_QUICK_DISCO_REQ);
                break;
            }
        }

        //
        // init scan filter
        //
        pPort->P2PCfg.ScanWithFilter = P2pMs_SCAN_FILTER_NULL;
        
        //Update Probe Request IEs
        if ((pWFDDisReq->uIEsLength > 0) && (pWFDDisReq->uIEsLength <= sizeof(pPort->P2PCfg.WFDProprietaryIEs)))
        {
            //
            // copy IEs
            //
            PlatformMoveMemory(pPort->P2PCfg.WFDProprietaryIEs, 
                            (BYTE *)pWFDDisReq + pWFDDisReq->uIEsOffset, 
                            pWFDDisReq->uIEsLength);
            pPort->P2PCfg.WFDProprietaryIEsLen = (USHORT) pWFDDisReq->uIEsLength;

    
            if (pPort->P2PCfg.WFDProprietaryIEsLen > 0)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] WFDProprietaryIEsLen = %d\n",  pPort->P2PCfg.WFDProprietaryIEsLen));
                DumpFrameMessage((PUCHAR) pPort->P2PCfg.WFDProprietaryIEs,  pPort->P2PCfg.WFDProprietaryIEsLen, ("WFDProprietaryIEs\n"));

                // Search one or more requested device types from WSC IE
                // {10 6A 00 08 __ __(ID) __ __ __ __ (OUI) __ __ (SubID)}
                PlatformZeroMemory(RequestedDeviceType, sizeof(RequestedDeviceType));
                IsWscIE = P2PParseWPSIE(pPort->P2PCfg.WFDProprietaryIEs, pPort->P2PCfg.WFDProprietaryIEsLen, 
                                NULL, NULL, NULL, NULL, NULL, &NumRequestedType, (PUCHAR)&RequestedDeviceType[0][0]);

                if (NumRequestedType > 0)
                {
                    MT_SET_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_TYPE);
                    PlatformMoveMemory(pPort->P2PCfg.FilterDevType, RequestedDeviceType, NumRequestedType * 8);
                    pPort->P2PCfg.NumberOfFilterDevType = NumRequestedType;
                }

                if (IsWscIE == TRUE)
                    pPort->P2PCfg.IsValidProprietaryIE = TRUE;
                else
                    pPort->P2PCfg.IsValidProprietaryIE = FALSE;
            }

        }
        else
        {
            //
            // not used, free old IEs
            //
            if (pPort->P2PCfg.WFDProprietaryIEsLen > 0)
            {
                PlatformZeroMemory(pPort->P2PCfg.WFDProprietaryIEs, sizeof(pPort->P2PCfg.WFDProprietaryIEs));
                pPort->P2PCfg.WFDProprietaryIEsLen = 0;
                pPort->P2PCfg.IsValidProprietaryIE = FALSE;
            }
        }

        // Copy device filter list and handle in probe req and discovery complete
        if ((pWFDDisReq->uNumDeviceFilters > 0) && (pWFDDisReq->uDeviceFilterListOffset > 0))
        {
#if 0           
            MT_SET_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID);
            pDevId = Add2Ptr(pWFDDisReq, pWFDDisReq->uDeviceIDListOffset);
            // non-broadcost address, use this DevId of P2P IE probe request
            if ( ((*pDevId) & 0x01) == 0)
            {
                pPort->P2PCfg.NumberOfFilterDevId = 1; // only one specific device Id
                PlatformMoveMemory(pPort->P2PCfg.FilterDevId, pDevId, MAC_ADDRESS_LENGTH);
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Get Device ID %x %x %x %x %x %x. \n",
                            __FUNCTION__, pPort->P2PCfg.FilterDevId[0][0], pPort->P2PCfg.FilterDevId[0][1], 
                            pPort->P2PCfg.FilterDevId[0][2], pPort->P2PCfg.FilterDevId[0][3],
                            pPort->P2PCfg.FilterDevId[0][4], pPort->P2PCfg.FilterDevId[0][5]));

                // Delay StartScanRound. Let the peer goes beck to its listen channel
                StartScanRound = 4;// 400 ms
            }
#else
            MT_SET_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID);
            pPort->P2PCfg.uNumDeviceFilters = min(pWFDDisReq->uNumDeviceFilters, MAX_P2P_DISCOVERY_FILTER_NUM);

            PlatformZeroMemory(pPort->P2PCfg.DeviceFilterList, 
                        sizeof(DOT11_WFD_DISCOVER_DEVICE_FILTER) *  MAX_P2P_DISCOVERY_FILTER_NUM);
            
            PlatformMoveMemory(pPort->P2PCfg.DeviceFilterList,
                        Add2Ptr(pWFDDisReq, pWFDDisReq->uDeviceFilterListOffset),
                        sizeof(DOT11_WFD_DISCOVER_DEVICE_FILTER) *  pPort->P2PCfg.uNumDeviceFilters);

            for (i = 0; i < (pPort->P2PCfg.uNumDeviceFilters); i++)
            {
                if (pPort->P2PCfg.DeviceFilterList[i].ucBitmask == 0)
                {
                    // Win8 OS deson't provide valid value and cover it
                    pPort->P2PCfg.DeviceFilterList[i].ucBitmask = 1;
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - DeviceFilterList[%d], Bitmask(0) --> Cover by default value Bitmask(%d)\n", __FUNCTION__, i, pPort->P2PCfg.DeviceFilterList[i].ucBitmask));
                }
                DBGPRINT(RT_DEBUG_TRACE, ("%s - DeviceFilterList[%d] Bitmask(1:DEV, 2:GO, F:ANY)(= %d) \n", __FUNCTION__, i, pPort->P2PCfg.DeviceFilterList[i].ucBitmask));
                DBGPRINT(RT_DEBUG_TRACE, ("%s - DeviceFilterList[%d] DeviceID = %x %x %x %x %x %x. \n",
                                            __FUNCTION__,
                                            i, 
                                            pPort->P2PCfg.DeviceFilterList[i].DeviceID[0],
                                            pPort->P2PCfg.DeviceFilterList[i].DeviceID[1],
                                            pPort->P2PCfg.DeviceFilterList[i].DeviceID[2],
                                            pPort->P2PCfg.DeviceFilterList[i].DeviceID[3],
                                            pPort->P2PCfg.DeviceFilterList[i].DeviceID[4],
                                            pPort->P2PCfg.DeviceFilterList[i].DeviceID[5]));
                DBGPRINT(RT_DEBUG_TRACE, ("%s - DeviceFilterList[%d] GroupSSID = %s \n", __FUNCTION__, i, pPort->P2PCfg.DeviceFilterList[i].GroupSSID.ucSSID));               
            }
            
            // Delay StartScanRound. Let the peer goes back to its listen channel (default is 100 ms)
            // For quck p2p GONego connection
            if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pPort->P2PCfg.DeviceFilterList[0].ucBitmask == DISCOVERY_FILTER_BITMASK_DEVICE) && ((pWFDDisReq->uDiscoverTimeout / 100) > 2))
            {
                StartScanRound = 2;// 200 ms
            }
            // For quck p2p invitation/join connection
            else if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pPort->P2PCfg.DeviceFilterList[0].ucBitmask != DISCOVERY_FILTER_BITMASK_DEVICE) && ((pWFDDisReq->uDiscoverTimeout / 100) > 1))
            {
                StartScanRound = 2;// 200 ms
            }
            else if ((pWFDDisReq->uDiscoverTimeout / 100) > 4)
            {
                StartScanRound = 2;// 400 ms
            }

#endif
        }

        // Check if force to scan legacy networks
        // use legacy-wildcard-ssid to MlmeSyncScanMlmeSyncNextChannel at the first probe request and 
        // use p2p-wildcard-ssid to p2pperiodexcute at the second and third probe request)
        if (pWFDDisReq->bForceScanLegacyNetworks == TRUE)
            MT_SET_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_FORCE_LEGACY_NETWORK);
        
        // Reset filter content if not used
        if (!MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_TYPE))
        {
            pPort->P2PCfg.NumberOfFilterDevType = 0; 
            PlatformZeroMemory(pPort->P2PCfg.FilterDevType, MAX_P2P_FILTER_LIST * P2P_DEVICE_TYPE_LEN);
        }
        if (!MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID))
        {
            //pPort->P2PCfg.NumberOfFilterDevId = 0; 
            //PlatformZeroMemory(pPort->P2PCfg.FilterDevId, MAX_P2P_FILTER_LIST * MAC_ADDRESS_LENGTH);
            pPort->P2PCfg.uNumDeviceFilters = 0;

            PlatformZeroMemory(pPort->P2PCfg.DeviceFilterList, 
                        sizeof(DOT11_WFD_DISCOVER_DEVICE_FILTER) *  MAX_P2P_DISCOVERY_FILTER_NUM);          
        }

        // Assign both normal and filter scan with the same value, but filter scan will update this value later
#if 1
        /* ndis set DiscoverTimeout as 250ms, 500ms, 1sec, 10sec... etc */
         // 1) more than 3 sec, shorten scan complete time with 500ms
        if ((pWFDDisReq->uDiscoverTimeout / 100) >= 30)
        {
            pPort->P2PCfg.ScanPeriod = (pWFDDisReq->uDiscoverTimeout / 100) - max(5, StartScanRound);
        }
        // 2)for 500ms ~ 1 sec, one round scantime = 30ms*11(channels)+200ms(startround) = 530ms
        //  usb needs more switching time = (30+20)ms*11(channels)+200ms(startround) = 750ms
        else if (((pWFDDisReq->uDiscoverTimeout / 100) > 5) && ((pWFDDisReq->uDiscoverTimeout / 100) <= 10))
        {
            pPort->P2PCfg.ScanPeriod = (pWFDDisReq->uDiscoverTimeout / 100) - 1;
//          pPort->P2PCfg.ScanPeriod = (pWFDDisReq->uDiscoverTimeout / 100) - max(2, StartScanRound);
        }
        else
        {
            pPort->P2PCfg.ScanPeriod = (pWFDDisReq->uDiscoverTimeout / 100);
        }
        DBGPRINT(RT_DEBUG_TRACE, ("%s - ScanPeriod = %d, StartScanRound=%d  (unit:100ms)\n", __FUNCTION__, pPort->P2PCfg.ScanPeriod, StartScanRound));
#else
        if ((pWFDDisReq->uDiscoverTimeout / 100) <= 5) // less than 500 ms
            pPort->P2PCfg.ScanPeriod = (pWFDDisReq->uDiscoverTimeout / 100);
        else
            pPort->P2PCfg.ScanPeriod = (pWFDDisReq->uDiscoverTimeout / 100) - 5;
#endif

        switch(pWFDDisReq->ScanType)
        {
            case dot11_wfd_scan_type_active:
                pPort->P2PCfg.ScanType = P2P_SCAN_TYPE_ACTIVE;
                break;
            case dot11_wfd_scan_type_passive:
                pPort->P2PCfg.ScanType = P2P_SCAN_TYPE_PASSIVE;
                break;
            case dot11_wfd_scan_type_auto:
                pPort->P2PCfg.ScanType = P2P_SCAN_TYPE_AUTO;
                break;
            default:
                DBGPRINT(RT_DEBUG_ERROR, ("Undefined P2P scan type\n"));
//              ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
        }


        //P2pScanChannelDefault(pAd, pPort);
        P2pSetListenIntBias(pAd, 1);

        // GO to Scan ===> ===> ===> ===>       
        pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
        MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ);
        P2pGotoScan(pAd, pPort, pWFDDisReq->DiscoverType, StartScanRound);
    } while (FALSE);

    if (NDIS_STATUS_INDICATION_REQUIRED == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_DISCOVER_REQUEST)
                    + pWFDDisReq->uIEsLength
                    + (sizeof(DOT11_WFD_DISCOVER_DEVICE_FILTER) *  pWFDDisReq->uNumDeviceFilters);
    }
    
    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function stop P2P scan

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer               
        InBufferLen         
        BytesRead           Number of bytes read from buffer
        BytesNeed           Number of bytes needed from buffer
    Return Value:
        ndisStatus          NDIS Status
    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsStopDeviceDiscovery(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    do
    {
        *BytesRead = 0;
        *BytesNeeded = 0;

        if ((pPort->P2PCfg.P2pCounter.CounterAftrScanButton + 2) < pPort->P2PCfg.ScanPeriod)
        {
            // Assign shoter timeout to timeout immediatedly after 200 ms during the ongoing discovery
            pPort->P2PCfg.ScanPeriod = (pPort->P2PCfg.P2pCounter.CounterAftrScanButton + 2);
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s::OID_DOT11_WFD_STOP_DISCOVERY at (%d)\n", __FUNCTION__, pPort->P2PCfg.ScanPeriod));     
    } while(FALSE);

    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function configures the discoverability of the device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/

NDIS_STATUS
P2pMsSetAvailability (
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PULONG  pDiscoState = (PULONG) InformationBuffer;

    switch(*pDiscoState)
    {
        case DOT11_WFD_DEVICE_NOT_DISCOVERABLE:
            pPort->P2PCfg.bAvailability = FALSE;
            pPort->P2PCfg.DeviceDiscoverable = DOT11_WFD_DEVICE_NOT_DISCOVERABLE;
            pPort->P2PCfg.P2pCounter.ScanIntervalBias2 = 0;
            break;

        case DOT11_WFD_DEVICE_AUTO_AVAILABILITY:
            if (pAd->LogoTestCfg.OnTestingWHQL == FALSE)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2pMs_RADIO_ON, NULL, 0);
            }
            pPort->P2PCfg.bAvailability = TRUE;
            pPort->P2PCfg.DeviceDiscoverable = DOT11_WFD_DEVICE_AUTO_AVAILABILITY;
            pPort->P2PCfg.P2pCounter.ScanIntervalBias2 = 0;
            break;

        case DOT11_WFD_DEVICE_HIGH_AVAILABILITY:
            if (pAd->LogoTestCfg.OnTestingWHQL == FALSE)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2pMs_RADIO_ON, NULL, 0);
            }
            pPort->P2PCfg.bAvailability = TRUE;
            pPort->P2PCfg.DeviceDiscoverable = DOT11_WFD_DEVICE_HIGH_AVAILABILITY;
            //extend interval between two scan round and allow
            //device to stay in listen channel longer
            pPort->P2PCfg.P2pCounter.ScanIntervalBias2 = 20;
            break;

        default:
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
    }

    DBGPRINT(RT_DEBUG_TRACE,("%s: DiscoState = %d\n", __FUNCTION__, *pDiscoState));

    if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pPort->P2PCfg.DeviceDiscoverable != DOT11_WFD_DEVICE_NOT_DISCOVERABLE))
    {
        P2PExitWhckCpuCheck(pAd);
    }

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(ULONG);
    }
    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function configures device info of the WFD device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetDeviceInfo(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_DEVICE_INFO pWFDDevInfo =
        (PDOT11_WFD_DEVICE_INFO) InformationBuffer;
    PUCHAR  pPriDeviceType = NULL;
    
    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDDevInfo->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_WFD_DEVICE_INFO_REVISION_1,
                    DOT11_SIZEOF_WFD_DEVICE_INFO_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (!PlatformEqualMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, &pWFDDevInfo->DeviceAddress, MAC_ADDR_LEN))
            DBGPRINT(RT_DEBUG_ERROR,("%s: Device Address is different to device port address (PortNumber %d)", __FUNCTION__, pPort->PortNumber));

        pPort->P2PCfg.ConfigMethod = pWFDDevInfo->ConfigMethods;

        // primary device type
        pPriDeviceType = &pPort->P2PCfg.PriDeviceType[0];
        *((PUSHORT) pPriDeviceType) = cpu2be16(pWFDDevInfo->PrimaryDeviceType.CategoryID);
        PlatformMoveMemory(pPriDeviceType + 2, &pWFDDevInfo->PrimaryDeviceType.OUI[0], 4);
        *((PUSHORT) (pPriDeviceType + 6)) = cpu2be16(pWFDDevInfo->PrimaryDeviceType.SubCategoryID);

        // Fix for MS WFD NDIS Test tool provide wrong length
        pWFDDevInfo->DeviceName.uDeviceNameLength = (ULONG) strlen(pWFDDevInfo->DeviceName.ucDeviceName);
        pPort->P2PCfg.DeviceNameLen = (USHORT)pWFDDevInfo->DeviceName.uDeviceNameLength;
        PlatformMoveMemory(pPort->P2PCfg.DeviceName, pWFDDevInfo->DeviceName.ucDeviceName, 32);
        DBGPRINT(RT_DEBUG_TRACE,("DeviceName.uDeviceNameLength(%d)\n", pWFDDevInfo->DeviceName.uDeviceNameLength));
    } while (FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_DEVICE_INFO);
    }
    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function set the seconday device types that are advertised by wifi direct devices

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer               Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetSecondaryDeviceTypeList(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS     ndisStatus  = NDIS_STATUS_SUCCESS;
    PUCHAR          pNewData    = NULL;
    PDOT11_WFD_SECONDARY_DEVICE_TYPE_LIST pWFDSecondaryDeviceTypeList = (PDOT11_WFD_SECONDARY_DEVICE_TYPE_LIST) InformationBuffer;

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDSecondaryDeviceTypeList->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_WFD_SECONDARY_DEVICE_TYPE_LIST_REVISION_1,
                    DOT11_SIZEOF_WFD_SECONDARY_DEVICE_TYPE_LIST_REVISION_1))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s::NDIS_STATUS_INVALID_DATA\n", __FUNCTION__));
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

#if 0   // debug
            pWFDSecondaryDeviceTypeList->uNumOfEntries = 2;
            pWFDSecondaryDeviceTypeList->uTotalNumOfEntries = 2;
#endif

        if (pWFDSecondaryDeviceTypeList->uNumOfEntries >0)
        {
            ULONG   SecondaryDeviceTypeLen = 0;
#if 0   // debug
            UCHAR SecondaryDeviceList[16] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x50, 0xf2, 0x04, 0x01, 0x00, 0x01, 0x00, 0x00, 0x50, 0xf2, 0x04};
#endif
            // Copy the OS setting and insert to P2P IE, P.87 P2P spec.
            SecondaryDeviceTypeLen = (pWFDSecondaryDeviceTypeList->uNumOfEntries) * sizeof(DOT11_WFD_DEVICE_TYPE);

            P2pMsAllocateIeData(pAd,
                                (PUCHAR)pWFDSecondaryDeviceTypeList->SecondaryDeviceTypes,
                                SecondaryDeviceTypeLen,
                                &pNewData);

            if(pNewData == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Alloc seconday device type list buffer fail\n"));
                 ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
            else
            {
                UCHAR                       Index               = 0;
                PDOT11_WFD_DEVICE_TYPE      pSecondaryDevice    = NULL;
                PUCHAR                      pNewDataTemp        = NULL;
                //
                // Fill seconday device type list
                //
#if 0   // debug
                pSecondaryDevice = (PDOT11_WFD_DEVICE_TYPE)SecondaryDeviceList;
#else
                pSecondaryDevice = pWFDSecondaryDeviceTypeList->SecondaryDeviceTypes;
#endif
                pNewDataTemp = pNewData;

                for (Index = 0; Index < pWFDSecondaryDeviceTypeList->uNumOfEntries; Index++)
                {
                    *((PUSHORT) pNewDataTemp) = cpu2be16(pSecondaryDevice->CategoryID);
                    PlatformMoveMemory(pNewDataTemp + 2, &pSecondaryDevice->OUI[0], 4);
                    *((PUSHORT) (pNewDataTemp + 6)) = cpu2be16(pSecondaryDevice->SubCategoryID);
                    pNewDataTemp += sizeof(DOT11_WFD_DEVICE_TYPE);
                    pSecondaryDevice++;
                }
                
                //Free previous Old IEs
                P2pMsFreeIeMemory(&pPort->PortCfg.pWFDSecondaryDeviceTypeList, &pPort->PortCfg.WFDSecondaryDeviceTypeLen);
                                
                //Cache current new IEs
                pPort->PortCfg.pWFDSecondaryDeviceTypeList = pNewData;
                pPort->PortCfg.WFDSecondaryDeviceTypeLen = SecondaryDeviceTypeLen;
                pPort->PortCfg.WFDNumOfSecondaryDevice = pWFDSecondaryDeviceTypeList->uNumOfEntries;
                
                pNewData = NULL;        
                // TODO: hm needs to add this attribute to defaultRequestIe (WSC IE) and refine the IE length as well               
            }
        }

        DBGPRINT(RT_DEBUG_TRACE, ("P2pMsSetSecondaryDeviceType (%d)(%d)(%d)\n", 
                                    pWFDSecondaryDeviceTypeList->uNumOfEntries, 
                                    pWFDSecondaryDeviceTypeList->uTotalNumOfEntries,
                                    pPort->PortCfg.WFDNumOfSecondaryDevice));
        DumpFrameMessage(pPort->PortCfg.pWFDSecondaryDeviceTypeList, pPort->PortCfg.WFDNumOfSecondaryDevice, ("SecondaryDeviceIEs\n"));

    } while (FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_SECONDARY_DEVICE_TYPE_LIST);
    }
    return ndisStatus;

}

/*
    ========================================================================
    Routine Description:
        This function configures additional IEs for the device

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetAdditionalIe(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   BufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_ADDITIONAL_IE pWFDAddIe =
        (PDOT11_WFD_ADDITIONAL_IE) InformationBuffer;
    PUCHAR newBeaconIeData = NULL;
    PUCHAR newResponseIeData = NULL;
    PUCHAR newDefaultIeData = NULL;

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    pWFDAddIe->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_WFD_ADDITIONAL_IE_REVISION_1,
                    DOT11_SIZEOF_WFD_ADDITIONAL_IE_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        //Beacon IE
        if (pWFDAddIe->uBeaconIEsLength > 0)
        {
            //Allocate new IEs buffers
            P2pMsAllocateIeData(pAd,
                                (PUCHAR)((PUCHAR)(pWFDAddIe) + pWFDAddIe->uBeaconIEsOffset),
                                pWFDAddIe->uBeaconIEsLength,
                                &newBeaconIeData);
            if(newBeaconIeData == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Alloc AdditionalIe BeaconIEs buffer fail\n"));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            //Free Old IEs
            P2pMsFreeIeMemory(&pPort->PortCfg.AdditionalBeaconIEData, &pPort->PortCfg.AdditionalBeaconIESize);

            //Cache new IEs
            pPort->PortCfg.AdditionalBeaconIEData = newBeaconIeData;
            pPort->PortCfg.AdditionalBeaconIESize = pWFDAddIe->uBeaconIEsLength;
            newBeaconIeData = NULL;

            // Update GO beacon if GO is started
            if ((pPort->PortType == WFD_GO_PORT) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_UPDATE_BEACON, NULL, 0);
        }

        //ProbeRsp IE
        if (pWFDAddIe->uProbeResponseIEsLength > 0)
        {
            //Allocate new IEs buffers
            P2pMsAllocateIeData(pAd,
                                (PUCHAR)((PUCHAR)(pWFDAddIe) + pWFDAddIe->uProbeResponseIEsOffset),
                                pWFDAddIe->uProbeResponseIEsLength,
                                &newResponseIeData);
            if(newResponseIeData == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Alloc AdditionalIe ResponseIE buffer fai\n"));
                 ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            //Free Old IEs
            P2pMsFreeIeMemory(&pPort->PortCfg.AdditionalResponseIEData, &pPort->PortCfg.AdditionalResponseIESize);

            //Cache new IEs
            pPort->PortCfg.AdditionalResponseIEData = newResponseIeData;
            pPort->PortCfg.AdditionalResponseIESize = pWFDAddIe->uProbeResponseIEsLength;
            newResponseIeData = NULL;

        }

        //Default IE
        if (pWFDAddIe->uDefaultRequestIEsLength > 0)
        {
            //Allocate new IEs buffers
            P2pMsAllocateIeData(pAd,
                                (PUCHAR)((PUCHAR)(pWFDAddIe) + pWFDAddIe->uDefaultRequestIEsOffset),
                                pWFDAddIe->uDefaultRequestIEsLength,
                                &newDefaultIeData);
            if(newDefaultIeData == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Alloc AdditionalIe DefaultIEs buffer fail\n"));
                 ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            //Free Old IEs
            P2pMsFreeIeMemory(&pPort->PortCfg.P2PAdditionalRequestIEData, &pPort->PortCfg.P2PAdditionalRequestIESize);

            //Cache new IEs
            pPort->PortCfg.P2PAdditionalRequestIEData = newDefaultIeData;
            pPort->PortCfg.P2PAdditionalRequestIESize = pWFDAddIe->uDefaultRequestIEsLength;
            newDefaultIeData = NULL;

        }
    } while (FALSE);


    if (pPort->PortCfg.AdditionalBeaconIESize > 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] AdditionalBeaconIESize = %d\n",  pPort->PortCfg.AdditionalBeaconIESize));
        DumpFrameMessage((PUCHAR) pPort->PortCfg.AdditionalBeaconIEData, pPort->PortCfg.AdditionalBeaconIESize, ("AdditionalBeaconIEData\n"));
    }
    if (pPort->PortCfg.AdditionalResponseIESize > 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] AdditionalResponseIESize = %d\n",  pPort->PortCfg.AdditionalResponseIESize));
        DumpFrameMessage((PUCHAR) pPort->PortCfg.AdditionalResponseIEData, pPort->PortCfg.AdditionalResponseIESize, ("AdditionalResponseIEData\n"));
    }
    if (pPort->PortCfg.P2PAdditionalRequestIESize > 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] P2PAdditionalRequestIESize = %d\n",  pPort->PortCfg.P2PAdditionalRequestIESize));
        DumpFrameMessage((PUCHAR) pPort->PortCfg.P2PAdditionalRequestIEData, pPort->PortCfg.P2PAdditionalRequestIESize, ("P2PAdditionalRequestIEData\n"));
    }

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_ADDITIONAL_IE) + pWFDAddIe->uBeaconIEsLength
                        + pWFDAddIe->uProbeResponseIEsLength + pWFDAddIe->uDefaultRequestIEsLength;
    }

    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function set Group ID

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer               Pointer to received Desired Group ID request packet
        InBufferLen         Size of Desired Group ID request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsSetDesiredGroupId(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS             ndisStatus              = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_GROUP_ID pWFDDesiredGroupId  = (PDOT11_WFD_GROUP_ID)InformationBuffer;

    do
    {
        *BytesRead = 0;
        *BytesNeeded = 0;

        /*
        Verifying error code for set on OID_DOT11_WFD_DESIRED_GROUP_ID, in GO-OP mode 
        76092 Failed to receive the expected error code(NDIS_STATUS_INVALID_STATE ). Received: NDIS_STATUS_SUCCESS
        */
        if (MP_VERIFY_STATE(pPort, OP_STATE))
        {
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            DBGPRINT(RT_DEBUG_ERROR, ("%s::OID_DOT11_WFD_DESIRED_GROUP_ID failed on Invalid state\n", __FUNCTION__));
            break;
        }   
        

        if((pWFDDesiredGroupId->SSID.uSSIDLength  == 0) || 
            (pWFDDesiredGroupId->SSID.uSSIDLength > DOT11_SSID_MAX_LENGTH))
        {
            // TODO: Actually, NDIS must set the length more than 7 ? ("DIRECT-")
            DBGPRINT(RT_DEBUG_ERROR, ("%s - Faild on invalid SSID length \n",__FUNCTION__));
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

#if 0 //prepare for Ndistest negative test.
        if((pWFDDesiredGroupId->SSID.uSSIDLength > WILDP2PSSIDLEN) &&
                !PlatformEqualMemory(pWFDDesiredGroupId->SSID.ucSSID, WILDP2PSSID, WILDP2PSSIDLEN))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s - Faild on invalid GO SSID\n", __FUNCTION__));
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }
#endif

        //
        // Update GroupID
        //
        PlatformMoveMemory(pPort->P2PCfg.DeviceAddress, pWFDDesiredGroupId->DeviceAddress, MAC_ADDR_LEN);
        pPort->P2PCfg.SSIDLen = (UCHAR)pWFDDesiredGroupId->SSID.uSSIDLength;
        PlatformMoveMemory(pPort->P2PCfg.SSID, pWFDDesiredGroupId->SSID.ucSSID, pPort->P2PCfg.SSIDLen);

        // Win8 set SSID of WFD ports through OID_DOT11_DESIRED_SSID_LIST   
        if (pPort->PortType == WFD_GO_PORT)
        {
            pPort->PortCfg.SsidLen = (UCHAR)pWFDDesiredGroupId->SSID.uSSIDLength;
            PlatformMoveMemory(pPort->PortCfg.Ssid, pWFDDesiredGroupId->SSID.ucSSID, pWFDDesiredGroupId->SSID.uSSIDLength);
            PlatformMoveMemory(&pPort->PortCfg.DesiredSSID, &pWFDDesiredGroupId->SSID, sizeof(DOT11_SSID));
        }
        else if (pPort->PortType == WFD_CLIENT_PORT)
        {
            // Use Normal connection SSID, not use auto-reconnection SSID (MlmeAux.AutoReconnectSsid)
            PlatformMoveMemory(&pPort->PortCfg.DesiredSSID, &pWFDDesiredGroupId->SSID, sizeof(DOT11_SSID));
        }
        
        DBGPRINT(RT_DEBUG_TRACE,("%s::GroupID-DeviceAddress=", __FUNCTION__));
        DumpAddress(pPort->P2PCfg.DeviceAddress);
        DBGPRINT(RT_DEBUG_TRACE,("%s::GroupID-SSID=%s(%d)", __FUNCTION__,pWFDDesiredGroupId->SSID.ucSSID,pPort->P2PCfg.SSIDLen));
        DumpFrame(pWFDDesiredGroupId->SSID.ucSSID, pPort->P2PCfg.SSIDLen);
    } while(FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_WFD_GROUP_ID);
    }

    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function provides startup parameters to the wifi direct GO

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer               Pointer to received Group join parameters packet
        InBufferLen         Size of Group join parameters packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/

NDIS_STATUS
P2pMsSetGroupStartParameters(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,   
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS                             ndisStatus                      = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_GROUP_START_PARAMETERS   pDot11GroupStartParameters = (PDOT11_WFD_GROUP_START_PARAMETERS)InformationBuffer;
    do
    {
        *BytesRead = 0;
        *BytesNeeded = 0;

        if (pPort->PortType != WFD_GO_PORT)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Failed on Invalid state (porttype=%d, portnumber=%d)\n", __FUNCTION__, pPort->PortType, pPort->PortNumber));
            return NDIS_STATUS_INVALID_STATE;
        }
    
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                                                        pDot11GroupStartParameters->Header,
                                                           NDIS_OBJECT_TYPE_DEFAULT,
                                                           DOT11_WFD_GROUP_START_PARAMETERS_REVISION_1,
                                                           DOT11_SIZEOF_WFD_GROUP_START_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        //
        // Update operting channel set in GO Nego or Invitation phase
        //
//      pPort->P2PCfg.GroupOpChannel = pDot11GroupStartParameters->AdvertisedOperatingChannel.ChannelNumber;

        DBGPRINT(RT_DEBUG_TRACE, ("DOT11_WFD_GROUP_START_PARAMETERS: GroupOpChannel = %d. ChannelNumber = %d. OperatingClass=0x%x\n", 
            pPort->P2PCfg.GroupOpChannel,
            pDot11GroupStartParameters->AdvertisedOperatingChannel.ChannelNumber,
            pDot11GroupStartParameters->AdvertisedOperatingChannel.OperatingClass));
        
    } while(FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead =  sizeof(DOT11_WFD_GROUP_START_PARAMETERS);
    }
    
    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function provides startup parameters to the wifi direct client

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer               Pointer to received Group join parameters packet
        InBufferLen         Size of Group join parameters packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/

NDIS_STATUS
P2pMsSetGroupJoinParameters(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,   
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS                         ndisStatus                      = NDIS_STATUS_SUCCESS;
    PDOT11_WFD_GROUP_JOIN_PARAMETERS    pDot11GroupJoinParameters   = (PDOT11_WFD_GROUP_JOIN_PARAMETERS)InformationBuffer;
    do
    {
        if (pPort->PortType != WFD_CLIENT_PORT)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Failed on Invalid state (porttype=%d, portnumber=%d)\n", __FUNCTION__, pPort->PortType, pPort->PortNumber));
            return NDIS_STATUS_INVALID_STATE;
        }

        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                                                        pDot11GroupJoinParameters->Header,
                                                           NDIS_OBJECT_TYPE_DEFAULT,
                                                           DOT11_WFD_GROUP_JOIN_PARAMETERS_REVISION_1,
                                                           DOT11_SIZEOF_WFD_GROUP_JOIN_PARAMETERS_REVISION_1))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }
#if 0   // Sometimes OS gives a wrong operating channel. We don't store this value because Client will do scan to search the desired GO later.
        //
        // Update operting channel set in GO Nego or Invitation phase
        //
        if ((pDot11GroupJoinParameters->GOOperatingChannel.ChannelNumber != 0) &&
            MlmeSyncIsValidChannel(pAd, pDot11GroupJoinParameters->GOOperatingChannel.ChannelNumber))
        {
            pPort->P2PCfg.GroupOpChannel = pDot11GroupJoinParameters->GOOperatingChannel.ChannelNumber;
        }
#endif      
        //
        // Update GO configuration time, unit : 10 ms
        //
        pPort->PortCfg.P2pConfigTimeout[0] = (UCHAR)pDot11GroupJoinParameters->GOConfigTime;

        //
        // Update InGroupFormation bit
        // The miniport must not attempt to connect until it received a probe response or beacon from GO 
        // with the Group Formation field set to 1 
        pPort->P2PCfg.InGroupFormation = pDot11GroupJoinParameters->bInGroupFormation;
        
        // sync to device port
        if (pDot11GroupJoinParameters->bInGroupFormation)
        {
            if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortType == WFD_DEVICE_PORT)
            {
                MT_SET_BIT(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2pCapability[1], GRPCAP_GROUP_FORMING);
            }
        }
        else
        {
            if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortType == WFD_DEVICE_PORT)
            {
                MT_CLEAR_BIT(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2pCapability[1], GRPCAP_GROUP_FORMING);
            }
        }

        //
        // Update WaitForWPSReady
        // The miniport must not attempt to connect until it received a probe response or beacon from GO 
        // with the Selected Registrar WPS attribute set to 1 and the desired Group Formation bit
        pPort->P2PCfg.WaitForWPSReady = pDot11GroupJoinParameters->bWaitForWPSReady;

        // If this is for wps connection request, we must wait for GO ready.
        if (pPort->P2PCfg.WaitForWPSReady == TRUE)
            pPort->P2PCfg.ReadyToConnect = FALSE;

        //
        // Go to provisioning state when Client start WPS engine
        //
        if ((pDot11GroupJoinParameters->bWaitForWPSReady == TRUE) &&
            ((pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_IDLE) ||
            (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_INVITATION) ||
            (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_FORMATION) ||
            (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_START_PROGRESS)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - change the state from %s to P2pMs_STATE_PROVISIONING\n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_PROVISIONING;
            // Start blocking periodic scan timer
            pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
        }
        else if (pDot11GroupJoinParameters->bWaitForWPSReady == FALSE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - change the state from %s to P2pMs_STATE_IDLE\n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));
            // go back to idle
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
        }

        // Update op channel got from GoN/Invitation before
        pPort->P2PCfg.GoNOpChannel = pDot11GroupJoinParameters->GOOperatingChannel;

        DBGPRINT(RT_DEBUG_TRACE, ("P2pMsSetGroupJoinParameters = ChannelNumber(%d), GOConfigTime(%d), bInGroupFormation(%d), bWaitForWPSReady(%d).  GroupOpChannel=%d \n",
                                pDot11GroupJoinParameters->GOOperatingChannel.ChannelNumber,
                                pDot11GroupJoinParameters->GOConfigTime,
                                pDot11GroupJoinParameters->bInGroupFormation,
                                pDot11GroupJoinParameters->bWaitForWPSReady,
                                pPort->P2PCfg.GroupOpChannel));
    } while(FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead =  sizeof(DOT11_WFD_GROUP_JOIN_PARAMETERS);
    }
    
    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function get Dialog Token

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer               Pointer to received Desired Group ID request packet
        InBufferLen         Size of Desired Group ID request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
NDIS_STATUS
P2pMsGetDialogToken(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID InformationBuffer,
    IN  ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    do
    {
        *BytesWritten = 0;
        *BytesNeeded = 0;

        if (InformationBufferLength < sizeof(UCHAR))
        {
            *BytesNeeded = sizeof(UCHAR);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        *(PUCHAR)InformationBuffer = pPort->P2PCfg.MSDialogToken;

        // In Ndis test, Dialog Token should be a non-zero value
        if (pPort->P2PCfg.MSDialogToken == 255)
        {
            pPort->P2PCfg.MSDialogToken = 1;
        }
        else
        {
            pPort->P2PCfg.MSDialogToken = (pPort->P2PCfg.MSDialogToken + 1);
        }
            
        *BytesWritten = sizeof(UCHAR);
        DBGPRINT(RT_DEBUG_TRACE, ("%s::OID_DOT11_WFD_GET_DIALOG_TOKEN (=%d, %d)\n", __FUNCTION__, *(PUCHAR)InformationBuffer, pPort->P2PCfg.MSDialogToken));
    } while(FALSE);

    return ndisStatus;
}

/*
    ========================================================================
    Routine Description:
        This function start GO request. The port should use the configured information to create beacons and start beaconong
        and responding to probe requests.

    Arguments:
        pAd                 Pointer to our adapter
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/

NDIS_STATUS
P2pMsStartGOReq(
    IN PMP_ADAPTER pAd,
    IN      PMP_PORT pPort)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    BOOLEAN TimerCancelled = FALSE;

    do
    {
        ndisStatus = P2pMsStartGo(pAd, pPort);
        DBGPRINT(RT_DEBUG_TRACE, ("%s:: PortNumber=%d. PortType=%d.\n", __FUNCTION__, pPort->PortNumber, pPort->PortType));
    } while(FALSE);

    return ndisStatus;
}

NDIS_STATUS
P2pMsStartGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort)
{
    ULONG       Value;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
#ifdef MULTI_CHANNEL_SUPPORT    
    MULTI_CH_DEFER_OID_STRUCT   MultiChDeferOid;
#endif /*MULTI_CHANNEL_SUPPORT*/

    if (!MP_VERIFY_STATE(pPort, INIT_STATE))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s::OID_DOT11_WFD_START_GO_REQUEST failed on Invalid state\n", __FUNCTION__));
        return NDIS_STATUS_INVALID_STATE;
    }   

    if (pPort->PortType != WFD_GO_PORT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Failed on Invalid state (porttype=%d, portnumber=%d)\n", __FUNCTION__, pPort->PortType, pPort->PortNumber));
        return NDIS_STATUS_INVALID_STATE;
    }

    if(pAd->StaCfg.bRadio == FALSE) //for WFD NDIStest error handling, 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s : Radio is OFF when start GO! \n", __FUNCTION__));
        return NDIS_STATUS_DOT11_POWER_STATE_INVALID;
    }
    
    // BSSID is set on current_operation_mode oid
    // SSID is set on Desired GroupId
    // Auth and Cipher algorithm are set on related OIDs.

    // Bssid would be reset in inforeset before startGO
    // So we need to set Bssid again
    COPY_MAC_ADDR(pPort->PortCfg.Bssid, pPort->CurrentAddress);

    pPort->PortCfg.DefaultKeyId = 1;

    // TODO:: [hm] OS will set go capability after Setting OID: 0x0e060102 OID_DOT11_WFD_START_GO_REQUEST port=2, need to check the setting of this bit
    // Set Group-Owner-Bit
    MT_SET_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_OWNER);
    DBGPRINT(RT_DEBUG_TRACE, ("%s : group owner bit is true when start GO! \n", __FUNCTION__));

    P2pSetDefaultGOHt(pAd, pPort);

    //the following 3 variable used to tell if the softap port needs to disconnect all
    //connected stations after the station port gets reconnected with an AP.
    pPort->SoftAP.ApCfg.StaChannel = pPort->Channel;
    pPort->SoftAP.ApCfg.StaCentralChannel = pPort->CentralChannel;
    pPort->SoftAP.ApCfg.StaHtEnable = pAd->StaActive.SupportedHtPhy.bHtEnable;

    //if (MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_COEXIST_OPT_BIT))
        P2pEdcaDefault(pAd, pPort);

    // [PortSubtype Usage] 
    // When GO starts to send beacon, set as PORTSUBTYPE_P2PGO
    // When it goes back to EXTSTA, reset to PORTSUBTYPE_STA 
    // PORTSUBTYPE_P2PGO in Win8 means to start GO's beacon.
    pPort->PortSubtype = PORTSUBTYPE_P2PGO;

    P2pStopScan(pAd, pPort);

    // [Win8 WFD]
    // Because it's hard to maintain different listen channel and working channel on STA+GO+Clients
    // So we change listen channel to this working channel once connection is active.
    // And reset listen channel to default after linkdown
    if (pPort->P2PCfg.ListenChannel != pPort->Channel)
    {
// 2011-11-28 For P2pMs GO multi-channel
//#ifdef MULTI_CHANNEL_SUPPORT
        // In multi-channel case, GO will use 1. STA+CLI: CLI ch 2. STA: STA ch 3. CLI: CLI ch
        DBGPRINT(RT_DEBUG_TRACE, ("%s - ListenChannel(%d), Channel(%d)\n", __FUNCTION__, pPort->P2PCfg.ListenChannel, pPort->Channel));
//#else             
        pPort->P2PCfg.ListenChannel = pPort->Channel;
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Follow common channel setting.becomes  ListenChannel = %d. \n", __FUNCTION__, pPort->P2PCfg.ListenChannel));
//#endif /* MULTI_CHANNEL_SUPPORT */        
    }
    
    P2pGoSwitchChannel(pAd, pPort);

#if UAPSD_AP_SUPPORT
    pAd->UAPSD.UapsdSWQBitmap = 0;
#endif

    // In AP mode,  First WCID Table in ASIC will never be used. To prevent it's 0xff-ff-ff-ff-ff-ff, Write 0 here.
    // p.s ASIC use all 0xff as termination of WCID table search.
    RTUSBWriteMACRegister(pAd, MAC_WCID_BASE, 0x00);
    RTUSBWriteMACRegister(pAd, MAC_WCID_BASE+4, 0x0);


    //////////////////////////////////////////////////////////
    // Prepare GO's Beacon
    
    //RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, APNORMAL);     // enable RX of DMA block
    //for two MAC mode, ps-poll will not be dropped.
    RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, APCLIENTNORMAL);
    pPort->bBeaconing = TRUE;
    MlmeUpdateBeacon(pAd);
    MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
    // Pre-tbtt interrupt setting.
    RTUSBReadMACRegister(pAd, INT_TIMER_CFG, &Value);
    Value &= 0xffff0000;
    Value |= 6 << 4; // Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
    RTUSBWriteMACRegister(pAd, INT_TIMER_CFG, Value);
    // Enable pre-tbtt interrupt
    RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
    Value |=0x1;
    RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);
    
    //////////////////////////////////////////////////////////

    // It control TX/RX data path
    pPort->SoftAP.ApCfg.bSoftAPReady = TRUE;

    // like bAPStart, maintain the coexistence between AP and Client.
    pPort->P2PCfg.bGOStart = TRUE;
    
    MP_SET_STATE(pPort, OP_STATE);
    MT_SET_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON);

    DBGPRINT(RT_DEBUG_TRACE, ("--->P2pMsStartGo  Channel = %d. CentralChannel = %d.\n", pPort->Channel, pPort->CentralChannel));
    DBGPRINT(RT_DEBUG_TRACE, ("!!GroupOpChannel = %d.  PortSubtype = %d. \n", pPort->P2PCfg.GroupOpChannel, pPort->PortSubtype));

#ifdef MULTI_CHANNEL_SUPPORT
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_PRE_START))
    {
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_NEW_CONNECTION_START);
        MultiChDeferOid.Type = PENDED_START_GO_OID_REQUEST;
        MultiChDeferOid.ndisStatus = ndisStatus;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SWITCH_EXEC, &MultiChDeferOid, sizeof(MULTI_CH_DEFER_OID_STRUCT));        
    }
    else
    {
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_NEW_CONNECTION_START);
        PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, 100);   
    }

    // Prepare TBTT timer by Software beased timer
    // Software based timer means don't use GP interrupt to get precise timer calculation. 
    // So need to check time offset caused by software timer.   
    if (pPort->CommonCfg.P2pControl.field.SwBasedNoA == 1)
    {
        ULONG   TimeTillTbtt, GPDiff;
        ULONG   StartTimeOffset = 0;

        RTUSBReadMACRegister(pAd, TBTT_TIMER, &TimeTillTbtt);
        TimeTillTbtt = TimeTillTbtt&0x1ffff;
        RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &Value);
        DBGPRINT(RT_DEBUG_TRACE,(" %s : TimeTillTbtt(%d). Current Tsf LSB(%d). Tsf LSB + TimeTillTbtt(%d).\n", __FUNCTION__, TimeTillTbtt, Value,  TimeTillTbtt*64+Value));

        // Wait five beacons 0x7d000 for 5 beacon interval.
        // start time right after beacon transmission.
        pPort->P2PCfg.TbttNextTargetTimePoint = Value + TimeTillTbtt*64 + DEFAULT_HOWMANY_BI*102400 + StartTimeOffset;
        GPDiff = TimeTillTbtt*64 + DEFAULT_HOWMANY_BI*102400 + StartTimeOffset;
        //convert TimeTillTbtt from us to ms
        GPDiff = (GPDiff >> 10)& 0xffff;
        // Set timer
        PlatformSetTimer(pPort, &pPort->Mlme.SwTbttTimer, GPDiff);
            
        DBGPRINT(RT_DEBUG_TRACE, (" %s : Start SwTbttTimer: BeaconPeriod=%d Intervals=%d, GPDiff=%d, TbttNextTargetTimePoint =%d\n",
                __FUNCTION__, pPort->CommonCfg.BeaconPeriod, DEFAULT_HOWMANY_BI, GPDiff, pPort->P2PCfg.TbttNextTargetTimePoint));
    }

#endif /* MULTI_CHANNEL_SUPPORT */

    return ndisStatus;
}


/*  
    ==========================================================================
    Description: 
        GO stop function
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID 
P2pMsStopGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort) 
{
    ULONG       Value;
    UCHAR       i;
    BOOLEAN     Cancelled;
    
    if (pPort->PortType != WFD_GO_PORT)
    {
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s ==> \n", __FUNCTION__));
    
    pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = FALSE;
    pPort->P2PCfg.P2PChannelState = P2P_DIFF_CHANNEL_OFF;
    

    // Clear GO's connected status
    MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC);
    MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP);

    // Disable beacon
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        // Clear Group-Owner-Bit
        MT_CLEAR_BIT(pPort->PortCfg.P2pCapability[1], GRPCAP_OWNER);

        pPort->bBeaconing = FALSE;
        MlmeUpdateBeacon(pAd);

        // Pre-tbtt interrupt setting.
        // Disable pre-tbtt interrupt
        RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
        Value &= 0xfffffffe;
        RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);
        pPort->PortSubtype = PORTSUBTYPE_P2PClient;
        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, STANORMAL);     // enable RX of DMA block
        DBGPRINT(RT_DEBUG_TRACE, ("%s - disable beacon, portsubtype =%d \n", __FUNCTION__, pPort->PortSubtype));
    }
        
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopWpsTimer, &Cancelled);
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer,  &Cancelled);
    PlatformCancelTimer(&pPort->Mlme.SwTbttTimer, &Cancelled);
    

    pPort->P2PCfg.PhraseKeyChanged = FALSE;
    pPort->P2PCfg.P2pGOAllPsm = FALSE;

    // Delete P2P Clients that connect to me previously.
    for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
    {
        if ((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_WPS)
            || (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_ASSOC)
            || (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_OPERATING))
        {
            P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
        }
    }

#if 0 // need to check how to merge from PCI
    // Cancel the AP related timers
    if(pPort->SoftAP.ApCfg.REKEYTimerRunning == TRUE)
    {
        PlatformCancelTimer(&pPort->SoftAP.ApCfg.REKEYTimer, &Cancelled);
        pPort->SoftAP.ApCfg.REKEYTimerRunning=FALSE;
    }
    PlatformCancelTimer(&pPort->SoftAP.ApCfg.CounterMeasureTimer, &Cancelled);
    PlatformCancelTimer(&pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);
#endif

    // Remove Bssid/Ssid after group dissolve
    PlatformZeroMemory(pPort->PortCfg.Bssid, MAC_ADDR_LEN);
    PlatformZeroMemory(pPort->PortCfg.Ssid, MAX_LEN_OF_SSID);
    pPort->PortCfg.SsidLen = 0;

    // It control TX/RX data path
    pPort->SoftAP.ApCfg.bSoftAPReady = FALSE;

    // Reset bGOStart if GO disappeared.
    // Stop to maintain the coexistence between AP and Client
    pPort->P2PCfg.bGOStart = FALSE;

    MP_SET_STATE(pPort, INIT_STATE);

// 2011-11-28 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
    // GO stop case
    if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
    {
        MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON);
    }
#endif /* MULTI_CHANNEL_SUPPORT */

    
}

/*
    ========================================================================
    Routine Description:
        This function will request the wifi direct client to join a wifi direct group. The port should use the configured
        information to search for the group owner and associate it

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pStateMachineTouched  Pointer to StateMachineTouched
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/

NDIS_STATUS
P2pMsConnectToGroupReq(
    IN  PMP_ADAPTER pAd,
    IN      PMP_PORT pPort)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    do
    {
        if (pPort->PortType != WFD_CLIENT_PORT)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Failed on Invalid state (porttype=%d, portnumber=%d)\n", __FUNCTION__, pPort->PortType, pPort->PortNumber));
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            break;
        }
        if (!MP_VERIFY_STATE(pPort, INIT_STATE))
        {
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_WFD_CONNECT_TO_GROUP_REQUEST failed on Invalid state\n"));
            break;
        }

        // Reset Auto reconnect params
        pAd->MlmeAux.AutoReconnectStatus = FALSE;
        pAd->MlmeAux.AutoReconnectSsidLen= 32;
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

        if ((pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)))
        {
            MlmeRestartStateMachine(pAd, pPort);
            DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
        }

        // Reset this counter to zero when I am connecting to GO
        pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = 0;

        //in case if first time no SSID exist in scan table
        pPort->P2PCfg.bGroupsScanOnce = TRUE;

        // Reset allowed scan retries
        pAd->StaCfg.ScanCnt = 0;

        pAd->MlmeAux.bNeedIndicateConnectStatus = TRUE;
        
        // Reset auto-connect limitations
        pAd->MlmeAux.ScanForConnectCnt = 0;

        //for Ndistest association_ext test, ndistest will connect an AP without inforeset
        pAd->MlmeAux.bStaCanRoam = FALSE;

        pAd->ucActivePortNum = pPort->PortNumber;
        MlmeEnqueue(pAd,
                    pPort,
                    MLME_CNTL_STATE_MACHINE,
                    OID_DOT11_WFD_CONNECT_TO_GROUP_REQUEST,
                    0,
                    NULL);

        MP_SET_STATE(pPort, OP_STATE);
    }while(FALSE);
    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_WFD_CONNECT_TO_GROUP_REQUEST (state= %s)\n", decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));

    return ndisStatus;
}


NDIS_STATUS
    P2pMsDisconnectFromGroupReq(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesRead = 0;
    *BytesNeeded = 0;

    if (pPort->PortType != WFD_CLIENT_PORT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Failed on Invalid state (porttype=%d, portnumber=%d)\n", __FUNCTION__, pPort->PortType, pPort->PortNumber));
        return NDIS_STATUS_INVALID_STATE;
    }

#ifdef MULTI_CHANNEL_SUPPORT
    // For P2pMs CLI
    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        // Save disconnect port
        pAd->MccCfg.pDisconnectPort = pPort;

        // Set the flag
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_STOP);
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Set MULTI_CHANNEL_CLI_PRE_STOP flag\n", __FUNCTION__));

        // Pending this OID
        ndisStatus = NDIS_STATUS_PENDING;
    }
    else
#endif /*MULTI_CHANNEL_SUPPORT*/        
    {
        ndisStatus = DisconnectRequest(pAd, pPort, BytesRead, BytesNeeded, FALSE);
    }

    return ndisStatus;
}


/*
    ========================================================================
    Routine Description:
        This function allocates IEs data to corresponding location

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsAllocateIeData(
    IN  PMP_ADAPTER pAd,
    IN  PUCHAR  pIeSrc,
    IN  ULONG   IeLength,
    OUT PUCHAR  *pIeMemory)
{
    PVOID newIeData = NULL;

    PlatformAllocateMemory(pAd,  &newIeData, IeLength);

    if(newIeData == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Alloc AdditionalIe BeaconIEs buffer fail\n"));
        *pIeMemory = NULL;
        return;
    }

    //
    // copy IEs Data
    PlatformMoveMemory(newIeData, pIeSrc, IeLength);
    *pIeMemory = newIeData;
    DBGPRINT(RT_DEBUG_TRACE, ("Allocate Address(%p)\n", *pIeMemory));
}

/*
    ========================================================================
    Routine Description:
        This function frees the memory allocated for IEs

    Arguments:
        pAd                 Pointer to our adapter
        pPort               Pointer to operating port
        pInBuffer           Pointer to received GO negotiation request packet
        InBufferLen         Size of GO negotiation request packet
    Return Value:

    IRQL =

    Note:
    ========================================================================
*/
VOID
P2pMsFreeIeMemory(
    IN PUCHAR  *pIeMemory,
    IN PLONG   pIeLength)
{
    if (*pIeMemory != NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Free Address(%p)\n", *pIeMemory));
        PlatformFreeMemory(*pIeMemory, *pIeLength);
        *pIeMemory = NULL;
    }
    *pIeLength = 0;
}

VOID
P2PMSSyncListenChannelToPortChannel(
    IN PMP_ADAPTER  pAd,
    IN PMP_PORT     pPort
)
{
    PMP_PORT    pInfraPort = pAd->pP2pCtrll->pInfraPort;
    
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s   ==> Port(%d)\n", __FUNCTION__, pPort->PortNumber));
     //Follow INFRA channel when PORT_0 is already connected
    if (INFRA_ON(pInfraPort))
    {
        pPort->P2PCfg.ListenChannel = pInfraPort->Channel;
        pPort->P2PCfg.GroupChannel = pInfraPort->Channel;
        pPort->P2PCfg.GroupOpChannel = pInfraPort->Channel;      
        DBGPRINT(RT_DEBUG_TRACE, (" !!Concurrent Mode!! ListenChannel and GroupOpChannel follow CommonChannel (%d) !!! \n", pPort->P2PCfg.ListenChannel));
    }
    else
    {
        pInfraPort->Channel = pPort->P2PCfg.ListenChannel;
        pInfraPort->CentralChannel = pPort->P2PCfg.ListenChannel;
        pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.ListenChannel;
        pPort->P2PCfg.GroupChannel = pPort->P2PCfg.GroupOpChannel;
        
        DBGPRINT(RT_DEBUG_TRACE, (" !!Non-concurrent Mode!! CommonChannel and GroupOpChannel follow ListenChannel (%d) !!! \n", pPort->P2PCfg.ListenChannel));
    }

    FUNC_LEAVE;
}
#ifdef DBG
VOID
P2pMsIndicateFrameSendComplete(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR pOutBuffer,
    IN ULONG OutBufferLen,
    IN NDIS_STATUS NdisStaus,
    IN ULONG Line)
#else
VOID
P2pMsIndicateFrameSendComplete(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR pOutBuffer,
    IN ULONG OutBufferLen,
    IN NDIS_STATUS NdisStaus)
#endif  
{
    PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME) pOutBuffer;

    if ((pPort == NULL) || (pPort->PortType != WFD_DEVICE_PORT))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Invalid Port !!! \n", __FUNCTION__));
        return;
    }

    FUNC_ENTER;
    
    if ((pFrame != NULL) && 
        (OutBufferLen > FIELD_OFFSET(P2P_PUBLIC_FRAME, ElementID)) &&
        (pPort->P2PCfg.WaitForMSCompletedFlags != P2pMs_WAIT_COMPLETE_NOUSE))
    {
        // disable periodic ACK check
        pPort->P2PCfg.bP2pAckReq = FALSE;
        
        DBGPRINT(RT_DEBUG_TRACE, ("%s(Line:%d) - 1  status = %d. frame type=%d. flags=0x%x. \n", __FUNCTION__, Line, NdisStaus, pFrame->Subtype, pPort->P2PCfg.WaitForMSCompletedFlags));

        switch(pFrame->Subtype)
        {
            case GO_NEGOCIATION_REQ:
                if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ) ||
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED) ||
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED_AFTER_SCAN))
                {
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ);
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED);
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED_AFTER_SCAN);              
                    P2pMsIndicateGONegoReqSendComplete(pAd, pPort, pOutBuffer, OutBufferLen, NdisStaus);
                }
                break;
            case GO_NEGOCIATION_RSP:
                if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_RSP))
                {
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_RSP);
                    P2pMsIndicateGONegoRspSendComplete(pAd, pPort, pOutBuffer, OutBufferLen, NdisStaus);
                }
                break;
            case GO_NEGOCIATION_CONFIRM:
                if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_COM))
                {
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_COM);
                    P2pMsIndicateGONegoConfirmSendComplete(pAd, pPort, pOutBuffer, OutBufferLen, NdisStaus);
                }
                break;
            case P2P_INVITE_REQ:
                if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ) ||
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED))
                {
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ);
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED);
                    P2pMsIndicateInviteReqSendComplete(pAd, pPort, pOutBuffer, OutBufferLen, NdisStaus);
                }
                break;
            case P2P_INVITE_RSP:
                if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_RSP))
                {
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_RSP);
                    P2pMsIndicateInviteRspSendComplete(pAd, pPort, pOutBuffer, OutBufferLen, NdisStaus);
                }
                break;
            case P2P_PROVISION_REQ:
                if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ) ||
                    MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED))
                {
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ);
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED);
                    P2pMsIndicateProvDiscoReqSendComplete(pAd, pPort, pOutBuffer, OutBufferLen, NdisStaus);
                }
                break;
            case P2P_PROVISION_RSP:
                if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_RSP))
                {
                    MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_RSP);
                    P2pMsIndicateProvDiscoRspSendComplete(pAd, pPort, pOutBuffer, OutBufferLen, NdisStaus);
                }
                break;
            default:
                DBGPRINT(RT_DEBUG_ERROR,("No indication required for this frame (=%d) \n", pFrame->Subtype));
                break;

        }

        if (NdisStaus != NDIS_STATUS_SUCCESS)
        {
            // If no ack received (timeout), go back to idle state
            if (((pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_FORMATION) ||
                 (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_INVITATION) ||
                (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_START_PROGRESS)) &&
                ((pFrame->Subtype == GO_NEGOCIATION_REQ) ||
                (pFrame->Subtype == GO_NEGOCIATION_RSP) ||
                (pFrame->Subtype == GO_NEGOCIATION_CONFIRM) ||
                (pFrame->Subtype == P2P_INVITE_REQ) ||
                (pFrame->Subtype == P2P_INVITE_RSP) ||
                (pFrame->Subtype == P2P_PROVISION_REQ) ||
                (pFrame->Subtype == P2P_PROVISION_RSP)))
            {
                pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
            }
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s(Line:%d) - 2  status = %d. frame type=%d. flags=0x%x. \n", __FUNCTION__, Line, NdisStaus, pFrame->Subtype, pPort->P2PCfg.WaitForMSCompletedFlags));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - WaitForMSCompletedFlags(%d) \n", __FUNCTION__, pPort->P2PCfg.WaitForMSCompletedFlags));
    }

    FUNC_LEAVE;
    
}


/*
    ========================================================================
    Routine Description:
        This function copies P2P group entries in DOT11_WFD_DEVICE_ENTRY
        format

    Arguments:
        pAd                     Pointer to our adapter
        pBuffer                 Pointer to buffer
        BufferLength            Length of allocated buffer

    Return Value:
        remaining buffer size

    IRQL =

    Note:
        the remaining *must* be 0, otherwise there might be some errors in
        the code
    ========================================================================
*/
VOID
P2pMsBuildDeviceList(
    IN  PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR      pBuffer,
    IN  ULONG       BufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded,
    OUT PUCHAR      pTotalDeviceNumber)
{
    PDOT11_WFD_DEVICE_ENTRY pDeviceEntry;
    PRT_P2P_DISCOVERY_ENTRY     pP2PEntry;
    UCHAR                   cliIdx;
    ULONG                   EntrySize;
    ULONG                   tmpBufferLength = BufferLength;
    UCHAR                   ZeroDevType[8] = {0};

    *BytesWritten = 0;
    *BytesNeeded = 0;
    *pTotalDeviceNumber = 0;
    
    pDeviceEntry = (PDOT11_WFD_DEVICE_ENTRY) pBuffer;

    //Build WFD device entry list from P2P Table
    for (cliIdx = 0; cliIdx < MAX_P2P_DISCOVERY_SIZE/*pAd->pP2pCtrll->P2PTable.DiscoNumber*/; cliIdx++)
    {
        pP2PEntry = &pAd->pP2pCtrll->P2PTable.DiscoEntry[cliIdx];
        
        // This entry is not available.
        if (pP2PEntry->bValid == FALSE)
            continue;
        
        EntrySize = sizeof(DOT11_WFD_DEVICE_ENTRY) +  (ULONG)pP2PEntry->BeaconFrameLen + (ULONG)pP2PEntry->ProbeRspFrameLen;

        if ((pP2PEntry->BeaconFrameLen == 0) && (pP2PEntry->ProbeRspFrameLen == 0))
        {
            DBGPRINT(RT_DEBUG_ERROR,("%s - Can't report all zeros on BeaconIe and ProbeRspIe. (cliIdx=%d)\n", __FUNCTION__, cliIdx));
            continue;
        }

        //
        // Report matched filters because we don't make sure the peer is following specification to response.
        //
        if (pPort->P2PCfg.ScanWithFilter != P2pMs_SCAN_FILTER_NULL)
        {
            // 1. mismatching device id, device type
            if (FALSE == P2pMsDeviceAddrAndTypeMatch(
                            pAd, 
                            pAd->PortList[pPort->P2PCfg.PortNumber],
                            //(MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID) ? (PUCHAR)&pPort->P2PCfg.FilterDevId[0][0] : NULL),
                            (MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID) ? (PUCHAR)pPort->P2PCfg.DeviceFilterList : NULL),
                            (MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_TYPE) ? (ULONG)pPort->P2PCfg.NumberOfFilterDevType : 0),
                            (MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_TYPE) ? (PUCHAR)&pPort->P2PCfg.FilterDevType[0][0] : NULL),
                            FALSE, 
                            pP2PEntry->DeviceId, 
                            pP2PEntry->Ssid,
                            pP2PEntry->SsidLen,
                            pP2PEntry->PrimaryDevType,
                            pP2PEntry->NumSecondaryType, 
                            (PUCHAR)&pP2PEntry->SecondaryDevType[0][0],
                            P2P_IS_DEVICE,
                            NULL))
            {
                continue;
            }

            // 2. exclude legacy networks if not turn on force-legacy-network-scan filter. 
            if ((!MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_FORCE_LEGACY_NETWORK)) &&
                (pP2PEntry->bLegacyNetworks == TRUE))
            {
                continue;
            }
        }
        
        //If buffer is overflowed, we need to work out the buffer required
        if (tmpBufferLength == 0)
        {       
            *BytesNeeded += EntrySize;
            continue;
        }
        else if(tmpBufferLength < EntrySize)
        {       
            *BytesNeeded = (EntrySize - tmpBufferLength);
            tmpBufferLength = 0;        
            continue;
        }

        //parse device information to buffer
        
        // It's GO
        if (pP2PEntry->BeaconFrameLen > 0)
        {
            pDeviceEntry->uPhyId = MlmeInfoGetPhyIdByChannel(pAd, pP2PEntry->OpChannel);
            MAP_CHANNEL_ID_TO_KHZ(pP2PEntry->OpChannel, pDeviceEntry->PhySpecificInfo.uChCenterFrequency);
            pDeviceEntry->PhySpecificInfo.uChCenterFrequency /= 1000;

        }
        else
        {
            pDeviceEntry->uPhyId = MlmeInfoGetPhyIdByChannel(pAd, pP2PEntry->ListenChannel);
            MAP_CHANNEL_ID_TO_KHZ(pP2PEntry->ListenChannel, pDeviceEntry->PhySpecificInfo.uChCenterFrequency);
            pDeviceEntry->PhySpecificInfo.uChCenterFrequency /= 1000;
        }
        
        PlatformMoveMemory(pDeviceEntry->dot11BSSID, pP2PEntry->Bssid, MAC_ADDRESS_LENGTH);
        pDeviceEntry->dot11BSSType = pP2PEntry->BssType;
        PlatformMoveMemory(pDeviceEntry->TransmitterAddress, pP2PEntry->TransmitterAddr, MAC_ADDRESS_LENGTH);
        pDeviceEntry->lRSSI                     = pP2PEntry->Rssi;
        pDeviceEntry->uLinkQuality              = MlmeInfoGetLinkQuality(pAd, pP2PEntry->Rssi, FALSE);
        pDeviceEntry->usBeaconPeriod            = pP2PEntry->BeaconPeriod;
        pDeviceEntry->ullTimestamp              = max(pP2PEntry->BeaconTimestamp, pP2PEntry->ProbeRspTimestamp);
        pDeviceEntry->ullBeaconHostTimestamp    = pP2PEntry->BeaconTimestamp;
        pDeviceEntry->ullProbeResponseHostTimestamp = pP2PEntry->ProbeRspTimestamp;
        pDeviceEntry->usCapabilityInformation   = pP2PEntry->CapabilityInfo;
        pDeviceEntry->uBeaconIEsOffset          = sizeof(DOT11_WFD_DEVICE_ENTRY);
        pDeviceEntry->uBeaconIEsLength          = pP2PEntry->BeaconFrameLen;
        pDeviceEntry->uProbeResponseIEsOffset   = sizeof(DOT11_WFD_DEVICE_ENTRY) + pP2PEntry->BeaconFrameLen;
        pDeviceEntry->uProbeResponseIEsLength   = pP2PEntry->ProbeRspFrameLen;

        //Copy Beacon IEs to buffer
        PlatformMoveMemory(((PUCHAR) pDeviceEntry + pDeviceEntry->uBeaconIEsOffset),
                        pP2PEntry->BeaconFrame,
                        pP2PEntry->BeaconFrameLen);

        //Copy Probe Response IEs to buffer
        PlatformMoveMemory(((PUCHAR) pDeviceEntry + pDeviceEntry->uProbeResponseIEsOffset),
                        pP2PEntry->ProbeRspFrame,
                        pP2PEntry->ProbeRspFrameLen);

        //Update remaining size;
        tmpBufferLength -= EntrySize;
        *BytesWritten += EntrySize;

        // increase actual device numbers
        (*pTotalDeviceNumber) ++;

        // print its contents
        P2PPrintDiscoEntry(pAd, cliIdx);

        //Update pointer to next device entry
        pDeviceEntry = (PDOT11_WFD_DEVICE_ENTRY)((PUCHAR) pDeviceEntry + EntrySize);

    }

    DBGPRINT(RT_DEBUG_ERROR,("%s==>DiscoNumber = %d, TotalDeviceNumber = %d\n", __FUNCTION__, pAd->pP2pCtrll->P2PTable.DiscoNumber, *pTotalDeviceNumber));
}

NDIS_STATUS
P2pMsIncomingAssociationDecisionV2(
    IN  PMP_ADAPTER   pAd,
    IN  NDIS_PORT_NUMBER          PortNumber,
    IN  PVOID           InformationBuffer,
    IN  ULONG           InformationBufferLength,
    IN  PULONG          pulBytesNeeded,
    IN  PULONG          pulBytesRead
    )
{
    NDIS_STATUS                             ndisStatus                      = NDIS_STATUS_SUCCESS;
    PDOT11_INCOMING_ASSOC_DECISION_V2   pIncomingAssociationDecisionV2;
    PMAC_TABLE_ENTRY                        pEntry;
    PMP_PORT                              pPort                           = pAd->PortList[PortNumber];
    PUCHAR                                  newResponseIeData               = NULL;

    do
    {
        *pulBytesNeeded = 0;
        *pulBytesRead = 0;

        if (InformationBufferLength < sizeof(DOT11_INCOMING_ASSOC_DECISION_V2))
        {
            *pulBytesNeeded = sizeof(DOT11_INCOMING_ASSOC_DECISION_V2);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        pIncomingAssociationDecisionV2 = (PDOT11_INCOMING_ASSOC_DECISION_V2) InformationBuffer;

        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                            pIncomingAssociationDecisionV2->Header,
                            NDIS_OBJECT_TYPE_DEFAULT,
                            DOT11_INCOMING_ASSOC_DECISION_REVISION_2,
                            DOT11_SIZEOF_INCOMING_ASSOC_DECISION_REVISION_2))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }
        //AssocRsp IE, OID V1.4 P.83
        if (pIncomingAssociationDecisionV2->uAssocResponseIEsLength > 0)
        {
            //Allocate new IEs buffers
            P2pMsAllocateIeData(pAd,
                                (PUCHAR)pIncomingAssociationDecisionV2 + pIncomingAssociationDecisionV2->uAssocResponseIEsOffset,
                                pIncomingAssociationDecisionV2->uAssocResponseIEsLength,
                                &newResponseIeData);
            if(newResponseIeData == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Alloc AdditionalIe Associate buffer fail\n"));
                 ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            //Free Old IEs
            P2pMsFreeIeMemory(&pPort->PortCfg.AdditionalAssocRspIEData, &pPort->PortCfg.AdditionalAssocRspIESize);

            //Cache new IEs
            pPort->PortCfg.AdditionalAssocRspIEData = newResponseIeData;
            pPort->PortCfg.AdditionalAssocRspIESize = pIncomingAssociationDecisionV2->uAssocResponseIEsLength;
            newResponseIeData = NULL;
            
        }

        pPort->P2PCfg.WFDStatus = pIncomingAssociationDecisionV2->WFDStatus;

        //
        // Set is allowed only in AP OP state
        //
        pEntry = MacTableLookup(pAd,  pPort, pIncomingAssociationDecisionV2->PeerMacAddr);
        if (!pEntry)
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        pEntry->AcceptAssoc = pIncomingAssociationDecisionV2->bAccept;
        pEntry->Reason = pIncomingAssociationDecisionV2->usReasonCode; // TODO: hm, not used in code
        *pulBytesRead = sizeof(DOT11_INCOMING_ASSOC_DECISION_V2);

        DBGPRINT(RT_DEBUG_TRACE, ("N6APDot11_SetIncomingAssociationDecision :Aid=%d, AcceptAssoc=%x, Reason=%d WFDStatus(%d), AdditionalAssocRspIESize(%d)\n", 
                                    pEntry->Aid, 
                                    pEntry->AcceptAssoc, 
                                    pEntry->Reason,
                                    pPort->P2PCfg.WFDStatus,
                                    pPort->PortCfg.AdditionalAssocRspIESize));
    } while (FALSE);

    return ndisStatus;

}

/*
    ==========================================================================
    Description:
        The routine check whether we need to check this frame's transmission result.
        If found success, we need to indicate Send-Complete to NDIS.

    Parameters:
    Note:

    ==========================================================================
 */
VOID 
P2pMsAckRequiredCheck(
    IN  PMP_ADAPTER       pAd,
    IN PMP_PORT     pPort,
    IN  PP2P_PUBLIC_FRAME   pFrame,
    OUT UCHAR       *TempPid)
{
    PFRAME_P2P_ACTION   pActFrame;
    PUCHAR              ptr = NULL;

    *TempPid = 0;
    pActFrame = (PFRAME_P2P_ACTION)pFrame;
    ptr = (PUCHAR)&pFrame->OUI[0];

    // P2P Public Action Frame
    if ((pFrame->Category == CATEGORY_PUBLIC) && (pFrame->Action == P2P_ACTION) && (PlatformEqualMemory(ptr, P2POUIBYTE, 4)))
    {
        // update flag to wait a sending complete
        DBGPRINT(RT_DEBUG_TRACE, ("%s - the previous WaitForMSCompletedFlags = %d \n", __FUNCTION__, pPort->P2PCfg.WaitForMSCompletedFlags));

        // Check Go Neg Frame
        if (pFrame->Subtype == GO_NEGOCIATION_REQ)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - GoNego Req Frame.\n", __FUNCTION__));
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ);
            *TempPid = PID_P2pMs_INDICATE;
            return;
        }
        if (pFrame->Subtype == GO_NEGOCIATION_RSP)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - GoNego Rsp Frame.\n", __FUNCTION__));
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_RSP);
            *TempPid = PID_P2pMs_INDICATE;
            return;
        }
        if (pFrame->Subtype == GO_NEGOCIATION_CONFIRM)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - GoNego Confirm Frame.\n", __FUNCTION__));
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_COM);
            *TempPid = PID_P2pMs_INDICATE;
            return;
        }

        // Check Intivation Frame
        if (pFrame->Subtype == P2P_INVITE_REQ)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - Invite Req Frame.\n", __FUNCTION__));
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ);
            *TempPid = PID_P2pMs_INDICATE;
            return;
        }
        if (pFrame->Subtype == P2P_INVITE_RSP)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - Invite Resp Frame.\n", __FUNCTION__));
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_RSP);
            *TempPid = PID_P2pMs_INDICATE;
            return;
        }

        // Check Provision Discovery Frame
        if (pFrame->Subtype == P2P_PROVISION_REQ)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - Provision Req Frame.\n", __FUNCTION__));
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ);
            *TempPid = PID_P2pMs_INDICATE;
            return;
        }
        if (pFrame->Subtype == P2P_PROVISION_RSP)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - Provision Resp Frame.\n", __FUNCTION__));
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_RSP);
            *TempPid = PID_P2pMs_INDICATE;
            return;
        }
    }
}

/*
    ==========================================================================
    Description:
        The routine called by OID_DOT11_RESET_REQUEST. Reset parameters to default.
        If it is operating as GO, it must stop the group and disconnect all its clients.
        If it is operating as Client, it must leave the group.
    Parameters:
    Note:

    ==========================================================================
 */
VOID    
P2pMsInfoResetRequest(  
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort)
{
    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    // p2p actions are not in WFD port, return.
    if ((pPort->PortType != WFD_DEVICE_PORT) &&
        (pPort->PortType != WFD_CLIENT_PORT) && 
        (pPort->PortType != WFD_GO_PORT))
    {
        return;
    }   

    DBGPRINT(RT_DEBUG_TRACE,("=====> %s \n", __FUNCTION__));

    // We often use GoIntentIdx == 16 to know I was set to be AutoGo. 
    // Since this is a disconnet comand for GO port. I must set GoIntentIdx to a vlaue !=16.
    if (pPort->PortType == WFD_GO_PORT)
        pPort->P2PCfg.GoIntentIdx = 5;

    // Dissolve p2p group
    P2pMsDown(pAd, pPort);
    
    DBGPRINT(RT_DEBUG_TRACE,("<===== %s \n", __FUNCTION__));

}

/*
    ==========================================================================
    Description:
        Reset P2P state machine. Not Disable.
    Return:

    NOTE:

    ==========================================================================
*/
VOID
P2pMsDown(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
//  PMAC_TABLE_ENTRY    pP2pEntry;
    UCHAR               i;
    MLME_DISASSOC_REQ_STRUCT    DisReq;
    MLME_DEAUTH_REQ_STRUCT      DeAuthRequest;
    BOOLEAN             TimerCancelled = FALSE;


    if ((pPort->PortType != WFD_DEVICE_PORT) &&
        (pPort->PortType != WFD_CLIENT_PORT) && 
        (pPort->PortType != WFD_GO_PORT))
    {
        return;
    }   

    //
    // 1. Send disassociate frame to all of peers.
    //
    if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
    {
        // I am Client.
        if ((pPort->PortType == WFD_CLIENT_PORT) && (INFRA_ON(pPort)))
        {       
            DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
            DBGPRINT(RT_DEBUG_TRACE,("%s:  Disconnect  GO ==>..%x \n", __FUNCTION__, pPort->PortCfg.Bssid[5]));
            COPY_MAC_ADDR(DeAuthRequest.Addr, pPort->PortCfg.Bssid);
            MlmeAuthDeauthReqAction(pAd, pPort->PortNumber, &DeAuthRequest);

            // Delay for safety de-auth frame sent out on the original channel
            // Because the common channel will be changed to my listen channel after linkdown.
//          Delay_us(5000); // 5 ms

            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;

            //Set this flag and in MlmeCntLinkDown, the bssid will be deleted from scan table and
            //the station will not keep trying to connect to the old bssid.
            pAd->MlmeAux.CurrReqIsFromNdis = FALSE;
            MlmeCntLinkDown(pPort, FALSE);
                        
            MtAsicUpdateRxWCIDTable(pAd, P2pGetClientWcid(pAd, pPort) , ZERO_MAC_ADDR);           
            //
            // Indicate disconnect status
            //
            pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
            PlatformIndicateDisassociation(pAd, pPort, DisReq.Addr, DOT11_DISASSOC_REASON_OS);
        }

#if 0   // Implement on MacTableReset()
        // I am GO
        if (pPort->PortType == WFD_GO_PORT)
        {
            for (i = 2; i < MAX_P2P_GROUP_SIZE; i++)
            {
                pP2pEntry = &pPort->MacTab.Content[i];
                // Search GO's clients according to P2pClientState
                if ((pP2pEntry->ValidAsCLI)
                    && ((pP2pEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_WPS)
                    ||  (pP2pEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_ASSOC)
                    ||  (pP2pEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_OPERATING)
                    ||  (pP2pEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_PSK)
                    ||  (pP2pEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_WPS)))
                {
                    // this Client is associated, need to send Disassociate frame disconnect.
                    //
                    PlatformZeroMemory(&DeAuthRequest, sizeof(DeAuthRequest));
                    DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
                    PlatformMoveMemory(DeAuthRequest.Addr, pP2pEntry->Addr, 6);

                    //Send dis_assoc & de_auth
                    if (!PlatformEqualMemory(DeAuthRequest.Addr, ZeroSsid, 6))
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s:  Disconnect CLI (%d)==> \n", __FUNCTION__, i));
                        ApAuthGoDeauthReqAction(pAd, pPort->PortNumber, &DeAuthRequest);
                    }
                }
            }
        }
#endif      
    }   

    //
    // 2. Stop p2p functionalities
    //
    
    if (pPort->PortType == WFD_GO_PORT)
    {
        // Stop p2p power saving
        P2pStopNoA(pAd, NULL);
        P2pStopOpPS(pAd, pPort);

        // Just clear GO's entries. 
        // Client entry with id#23 has deleted in MlmeCntLinkDown.
        MacTableReset(pAd, pPort->PortNumber);
        
        // Stop GO functionalities and disable beacon
        P2pMsStopGo(pAd, pPort);

        // TODO:  Rita, need to further check
        // Rest to BW=20MHz, Channel = ListenChannel
//      P2pMsDefaultListenChannel(pAd);

        // we should disable WPS Feature when GO Reset
        pPort->SoftAP.ApCfg.bWPSEnable = FALSE;

        pPort->P2PCfg.MaximumGroupLimit = 0;
            
        // If I am still in provisioning state, stop it
        if (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_PROVISIONING)
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
    }
    else if (pPort->PortType == WFD_CLIENT_PORT)
    {
        // Stop p2p power saving
        P2pStopNoA(pAd, NULL);
        P2pStopOpPS(pAd, pPort);

        // Handle ready to connection on client side.
        pPort->P2PCfg.InGroupFormation = FALSE;
        pPort->P2PCfg.WaitForWPSReady = FALSE;
        pPort->P2PCfg.ReadyToConnect = TRUE;

        // clear scan-once flag for searching new GO
        pPort->P2PCfg.bGroupsScanOnce = FALSE;

        // If I am still in provisioning state, stop it
        if (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_PROVISIONING)
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;

        // Reset autoreconnection and ActivePort. These parameters are setup in connect request
        if (pAd->ucActivePortNum == pPort->PortNumber)
        {
            //ULONG i = 0;
            BOOLEAN FoundCandidatedActivePort = FALSE;

            pAd->MlmeAux.AutoReconnectSsidLen= 32;
            pAd->MlmeAux.AutoReconnectStatus = FALSE;
            PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

            // Try to find the Active Port Number or set the Active Port Number to default port         
            for (i = 1; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
            {
                if ((pAd->PortList[i] != NULL) &&
                    (pAd->PortList[i]->bActive == TRUE) &&
                    ((pAd->PortList[i]->PortType == EXTSTA_PORT) || (pAd->PortList[i]->PortType == WFD_CLIENT_PORT)) &&
                    (!IDLE_ON(pAd->PortList[i])))
                {
                    pAd->ucActivePortNum = i;
                    FoundCandidatedActivePort = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - Reset ucActivePortNum to Port(%d)\n", __FUNCTION__, i));
                    break;
                }
            }

            if (!FoundCandidatedActivePort)
            {           
                pAd->ucActivePortNum = NDIS_DEFAULT_PORT_NUMBER;
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Reset ucActivePortNum to NDIS_DEFAULT_PORT_NUMBER\n", __FUNCTION__));
            }

        }
    }
    else if (pPort->PortType == WFD_DEVICE_PORT)
    {
        // Turn Off MS P2P State
        pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_NOUSE;
        
        // Reset the scan/find state to idles
        pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY; 

        // remove sending buffer
        if (pPort->P2PCfg.pSendOutBufferedFrame != NULL)
        {
            PlatformFreeMemory(pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize);
            pPort->P2PCfg.pSendOutBufferedFrame = NULL;
            pPort->P2PCfg.SendOutBufferedSize = 0;
        }
        if (pPort->P2PCfg.pRequesterBufferedFrame != NULL)
        {
            PlatformFreeMemory(pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize);
            pPort->P2PCfg.pRequesterBufferedFrame = NULL;
            pPort->P2PCfg.RequesterBufferedSize = 0;
        }

        // Set to 0xff means to connect to first Connecting MAC
        PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
        pPort->P2PCfg.ConnectingIndex = 0xff;
        
        // remove proprietary IEs
        pPort->P2PCfg.WFDProprietaryIEsLen = 0;
        PlatformZeroMemory(pPort->P2PCfg.WFDProprietaryIEs, sizeof(pPort->P2PCfg.WFDProprietaryIEs));

        // Reset Scan Counter to prevent go to P2P scan during inforeset.
        // Stop do P2P search.
        pPort->P2PCfg.P2pCounter.NextScanRound = 60;
        pPort->P2PCfg.P2pCounter.Counter100ms = 0;
        pPort->P2PCfg.P2pCounter.CounterAftrScanButton = pPort->P2PCfg.ScanPeriod;

        // Clear Client/Discovery Table
        P2pGroupTabInit(pAd); // Ralink P2P
        P2pDiscoTabInit(pAd); // WIN8 P2P

        // clear own filter lists
        pPort->P2PCfg.ScanWithFilter = P2pMs_SCAN_FILTER_NULL;
        //pPort->P2PCfg.NumberOfFilterDevId       = 0;
        pPort->P2PCfg.uNumDeviceFilters = 0;
        pPort->P2PCfg.NumberOfFilterDevType   = 0;
        pPort->P2PCfg.NumberOfFilterSsid      = 0;
        //PlatformZeroMemory(pPort->P2PCfg.FilterDevId,       MAX_P2P_FILTER_LIST * MAC_ADDRESS_LENGTH);
        PlatformZeroMemory(pPort->P2PCfg.DeviceFilterList, 
                    sizeof(DOT11_WFD_DISCOVER_DEVICE_FILTER) *  MAX_P2P_DISCOVERY_FILTER_NUM);                  
        PlatformZeroMemory(pPort->P2PCfg.FilterDevType,   MAX_P2P_FILTER_LIST * P2P_DEVICE_TYPE_LEN);
        PlatformZeroMemory(pPort->P2PCfg.FilterSsid,      MAX_P2P_FILTER_LIST * MAX_LEN_OF_SSID);
        
    
        // Handle ready to connection on client side.
        pPort->P2PCfg.InGroupFormation = FALSE;
        pPort->P2PCfg.WaitForWPSReady = FALSE;
        pPort->P2PCfg.ReadyToConnect = TRUE;
        
        // clear scan-once flag for searching new GO
        pPort->P2PCfg.bGroupsScanOnce = FALSE;

        // init wps supported version from registry
        if (pAd->StaCfg.WSCVersion2 >= 0x20)
            pPort->P2PCfg.WPSVersionsEnabled = DOT11_WPS_VERSION_2_0;
        else
            pPort->P2PCfg.WPSVersionsEnabled = DOT11_WPS_VERSION_1_0;


        // Clear channel switching flags
        pPort->P2PCfg.SwitchChannelInProgress = FALSE;
        pPort->P2PCfg.RestoreChannelInProgress = FALSE;
        
        //
        // Counters set to specific values to aviod to trigger
        //
        pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD; 
        pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = WFD_BLOCKING_SCAN_TIMEOUT;
        pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = WFD_BLOCKING_SCAN_TIMEOUT;
        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand = WFD_COMPLETE_SEND_TIMEOUT;
        pPort->P2PCfg.P2pCounter.SwitchingChannelCounter = WFD_COMPLETE_SEND_TIMEOUT;

        // Cancel SwBased Tbtt Timer
        PlatformCancelTimer(&pPort->Mlme.SwTbttTimer, &TimerCancelled);
        
    }

    //
    // 3. Clear status/flags
    //
    if (pPort->PortType == WFD_GO_PORT)
    {
        MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC);
        MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP);
    }
    else if (pPort->PortType == WFD_CLIENT_PORT)
    {
        PMP_PORT      pCheckPort2 = NULL, pTmpPort = NULL;
        
        for (i = 0; i < NUM_OF_CONCURRENT_CLIENT_ROLE; i++)
        {
            pTmpPort = MlmeSyncGetP2pClientPort(pAd, i);
            // this port is not the current port, found it
            if (pTmpPort && (pTmpPort->PortNumber != pPort->PortNumber))
                break;
            else
                pTmpPort = NULL;
        }
        pCheckPort2 = pTmpPort;

        // Disable status if another client port is not connected
        if ((!pCheckPort2) || (pCheckPort2 && IDLE_ON(pCheckPort2)))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_ASSOC);
            MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP);
        }
    }
    else if (pPort->PortType == WFD_DEVICE_PORT)
    {
#if 0   
        pAd->pP2pCtrll->P2pMsConnectedStatus = P2pMs_CONNECTED_STATUS_OFF;
        pPort->P2PCfg.WaitForMSCompletedFlags = P2pMs_WAIT_COMPLETE_NOUSE;
#else
        P2pMsEnable(pAd, pPort);
#endif
    }

    //
    // Init PortCfg parameters
    //
    
    //Free WFDSecondaryDeviceTypeList
    P2pMsFreeIeMemory(&pPort->PortCfg.pWFDSecondaryDeviceTypeList,  &pPort->PortCfg.WFDNumOfSecondaryDevice);   

    // reset p2p capability
    pPort->PortCfg.P2pCapability[0] = 0;
    pPort->PortCfg.P2pCapability[1] = 0;
    
    // reset p2p configtimeout
    pPort->PortCfg.P2pConfigTimeout[0] = 0;
    pPort->PortCfg.P2pConfigTimeout[1] = 0;
    
    // reset auth and cipher algorithm
    pPort->PortCfg.AuthMode = Ralink802_11AuthModeOpen;
    pPort->PortCfg.WepStatus = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.OrigWepStatus = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.PairCipher = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.GroupCipher = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.bMixCipher = FALSE;  
    pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s - portnum=%d. porttype=%d. subtype=%d. connectedstatus=%d. connectstate= %s. ProvDiscState= %s \n",
        __FUNCTION__, pPort->PortNumber, pPort->PortType, pPort->PortSubtype, pAd->pP2pCtrll->P2pMsConnectedStatus, decodeP2PState(pPort->P2PCfg.P2PConnectState), decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));


}

 /*
    ==========================================================================
    Description:
        Call after a successful association with p2p client or legacy sta.

    Return:
    ==========================================================================
 */
VOID 
P2pMsApAssocGoBuildAssociation(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN MAC_TABLE_ENTRY *pEntry,
    IN UCHAR         *RSN,
    IN UCHAR         *pRSNLen,
    IN UCHAR        *pP2pElement,
    IN UCHAR         P2pElementLen,
    IN UCHAR        *pWpsElement,
    IN UCHAR         WpsElementLen,
    IN HT_CAPABILITY_IE     *pHtCapability,
    IN UCHAR         HtCapabilityLen)
{
    UCHAR           p2pindex = P2P_NOT_FOUND;
    UCHAR           GroupCap = 0xff,DeviceCap = 0xff, DevAddr[6], InterfaceAddr[6], DeviceType[8], DeviceName[32], DeviceNameLen = 0;
    USHORT          Dpid, ConfigMethod;
    UCHAR           StatusCode;
    UCHAR           NumSecondaryType = 0, SecondarDevTypes[MAX_P2P_SECONDARY_DEVTYPE_LIST][P2P_DEVICE_TYPE_LEN];


    DBGPRINT(RT_DEBUG_TRACE,("=====> %s \n", __FUNCTION__));

    //  [Win8]
    //  Please update P2pClientState when the client joins to GO.
    //  We will use its state to handle disassociation in P2PStopGoTimerTimerCallback(). 

    
    // Case 1. P2P Device that connects to me 
    if (P2pElementLen != 0)
    {
        // == This Station is using P2P protocol to connect to me ===
        // Carry in P2PCap, P2PDeviceInfo(device address, config method, primary device type, secondary device list, device name)
        PlatformZeroMemory(DevAddr, 6);
        PlatformZeroMemory(InterfaceAddr, 6);
        PlatformZeroMemory(DeviceType, 8);
        PlatformZeroMemory(SecondarDevTypes, sizeof(SecondarDevTypes));
            
        // TODO: Interface address should be more than one list?
        P2pParseSubElmt(pAd, (PVOID)pP2pElement, P2pElementLen, 
            FALSE, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, NULL, NULL, NULL, &ConfigMethod, &ConfigMethod, DeviceType, &NumSecondaryType, (PUCHAR)&SecondarDevTypes[0][0], NULL, NULL, NULL, NULL, &StatusCode, NULL);
    
        DBGPRINT(RT_DEBUG_TRACE, ("[Win8] DevAddr = %x %x %x %x %x %x\n", DevAddr[0], DevAddr[1], DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5]));
        DBGPRINT(RT_DEBUG_TRACE, ("[Win8] DeviceNameLen = %d, DeviceName = %c %c %c %c %c %c %c %c\n", DeviceNameLen , DeviceName[0], DeviceName[1], DeviceName[2],DeviceName[3],DeviceName[4],DeviceName[5],DeviceName[6],DeviceName[7]));
        DBGPRINT(RT_DEBUG_TRACE, ("[Win8] DeviceCap = %x, ConfigMethod = %x, DeviceType = %x %x %x %x %x %x %x %x\n", DeviceCap, ConfigMethod , DeviceType[0], DeviceType[1], DeviceType[2],DeviceType[3],DeviceType[4],DeviceType[5],DeviceType[6],DeviceType[7]));

        if (!PlatformEqualMemory(pEntry->Addr, InterfaceAddr, 6))
        {
            // Save Interface Address here and update again in WPA done
            PlatformMoveMemory(pEntry->P2pInfo.InterfaceAddr, pEntry->Addr, 6);
            DBGPRINT(RT_DEBUG_TRACE, ("[Win8] InterfaceAddr = %x %x %x %x %x %x.\n", InterfaceAddr[0], InterfaceAddr[1], InterfaceAddr[2], InterfaceAddr[3], InterfaceAddr[4], InterfaceAddr[5]));
        }
        
        p2pindex = P2pGroupTabSearch(pAd, pEntry->Addr);
        // Add flag to show this support P2P 
        if (p2pindex < MAX_P2P_GROUP_SIZE)
        {           
            // Turn Off WSC and WPS state machine
            pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
            pAd->StaCfg.WpaState = SS_NOTUSE;
            
            //***********************************************************************
            // MS Client should use open/wep to associate with me, do WPS handshakes
            // MS Client should use wpa2psk to associate with me, do WPA handshakes
            //***********************************************************************
        
            if ((pEntry->RSNIE_Len > 0) && (pPort->SoftAP.ApCfg.bWPSEnable == FALSE))
            //if ((pEntry->RSNIE_Len > 0) && (WpsElementLen == 0))
            {
                DBGPRINT(RT_DEBUG_TRACE,("case 2-1 : Support P2P, use WPA2PSK \n"));
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_CLIENT_ASSOC;
                pEntry->P2pInfo.P2pClientState = P2PSTATE_CLIENT_ASSOC;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("case 2-2 : Support P2P, use WPS \n"));
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_CLIENT_WPS;
                pEntry->P2pInfo.P2pClientState = P2PSTATE_CLIENT_WPS;
            }
        }
        else 
        {
            // Turn Off WSC and WPS state machine
            pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
            pAd->StaCfg.WpaState = SS_NOTUSE;
            
            if ((pEntry->RSNIE_Len > 0) && (pPort->SoftAP.ApCfg.bWPSEnable == FALSE))
            //if ((pEntry->RSNIE_Len > 0) && (WpsElementLen == 0))
            {
                DBGPRINT(RT_DEBUG_TRACE,("case 1-1 : Support P2P, direct connect. use WPA2PSK \n"));
                p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_CLIENT_ASSOC, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
                pEntry->P2pInfo.P2pClientState = P2PSTATE_CLIENT_ASSOC;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("case 1-2 : Support P2P, direct connect. use WPS \n"));
                p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_CLIENT_WPS, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
                pEntry->P2pInfo.P2pClientState = P2PSTATE_CLIENT_WPS;
            }
        }
        
        // Set flag that this entry support P2P.
        pEntry->ValidAsP2P = TRUE;
        
        if (p2pindex < MAX_P2P_GROUP_SIZE)
        {
            if ((DeviceCap&DEVCAP_CLIENT_DISCOVER) == DEVCAP_CLIENT_DISCOVER)
            {
                DBGPRINT(RT_DEBUG_TRACE,("This Device is discoverable. \n"));
                P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_DEVICE_DISCOVERABLE);
            }
            if (DeviceCap != 0xff)
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].DevCapability = DeviceCap;
            if (GroupCap != 0xff)
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].GroupCapability= GroupCap;
            if (DeviceNameLen > 0)
            {
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceName, DeviceName, 32);
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceNameLen = DeviceNameLen;
                
            }

            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].InterfaceAddr, pEntry->Addr, MAC_ADDR_LEN);
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].PrimaryDevType, DeviceType, 8);
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].NumSecondaryType = NumSecondaryType;
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].SecondaryDevType, SecondarDevTypes, NumSecondaryType * 8);
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = ConfigMethod;
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].Dpid = Dpid;
        }
    }
    // Case 2. This is legacy device connection.  P2PIELen = 0
    else
    {
        // Set flag that this entry support P2P.
        pEntry->ValidAsP2P = FALSE;
            
        // Turn Off WSC and WPS state machine
        pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
        pAd->StaCfg.WpaState = SS_NOTUSE;

        // No WPA IE. must use WPS provisioning.
        // Case 2: this entry is first connected, WPS IE Len is 0. must be WPA2PSK

        if ((pEntry->RSNIE_Len > 0) && (pPort->SoftAP.ApCfg.bWPSEnable == FALSE))    
        //if (((pEntry->RSNIE_Len > 0)) && (WpsElementLen == 0))
        {
            DBGPRINT(RT_DEBUG_TRACE,("case 3-1 : Legacy STA,  use WPA2PSK \n"));
            pEntry->P2pInfo.P2pClientState = P2PSTATE_NONP2P_PSK;
            
            // This Station has ever been in my group table.
            // And now it connects as legacy sta.
            p2pindex = P2pGroupTabSearch(pAd, pEntry->Addr);
            // If it uses legacy connection, we shall delete it here.
            if (p2pindex < MAX_P2P_GROUP_SIZE)
            {
                DBGPRINT(RT_DEBUG_TRACE, (" %s. Delete it from Group table due to legacy connection\n", __FUNCTION__));
                P2pGroupTabDelete(pAd, p2pindex, pEntry->Addr);
            }
        }
        else
        {
            pEntry->P2pInfo.P2pClientState = P2PSTATE_NONP2P_WPS;
            DBGPRINT(RT_DEBUG_TRACE,("case 3-2 : Legacy STA,  use WPS \n"));
        }
    }
    
    // Update to assoc status
    if (pEntry->P2pInfo.P2pClientState != P2PSTATE_NONE)
        MT_SET_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC);

    DBGPRINT(RT_DEBUG_TRACE,("P2pClientState in MacEntry is %s  (p2pindex= %d )\n", decodeP2PClientState(pEntry->P2pInfo.P2pClientState), p2pindex));
    DBGPRINT(RT_DEBUG_TRACE,("<===== %s \n", __FUNCTION__));

}

/*  
    ==========================================================================
    Description: 
        Called by OID_DOT11_CIPHER_DEFAULT_KEY after a successful WPA2PSK AES 4-way handshake.
        
    Parameters: 
    Note:
    ==========================================================================
 */
VOID 
P2pMsWPADone(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR       MacAddr)
{

    UCHAR               index  = P2P_NOT_FOUND;
    //BOOLEAN               Cancelled;
    PMAC_TABLE_ENTRY    pEntry = NULL;

    if (P2P_OFF(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s return. P2P is OFF  !!! \n", __FUNCTION__));
        return;
    }
 
    DBGPRINT(RT_DEBUG_TRACE, ("%s: this key addr = %x,%x,%x,%x,%x,%x\n",
        __FUNCTION__, MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]));

    // This address is interface address.
    pEntry = MacTableLookup(pAd,  pPort, MacAddr);
    if (pEntry != NULL)
    {
        index = P2pGroupTabSearch(pAd, pEntry->Addr);
        if (pPort->PortType == WFD_CLIENT_PORT)
        {
            // =========== CLIENT Case ==============
            if (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
            {
                // Copy p2p devinfo to MacTab
                if (index < MAX_P2P_GROUP_SIZE)
                {
                    pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_GO_OPERATING;
                    P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PROVISIONED);
                    P2pMsCopyP2PTabtoMacTab(pAd, index, (UCHAR)pEntry->Aid, P2P_IS_CLIENT);
                    //PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);
                }
#ifdef MULTI_CHANNEL_SUPPORT
                // CLI connectes to peer GO and cehck STA/GO status
                // Defer a task to handle multi-channel state and DHCP
                if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) == FALSE)
                {
                    MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_OUT_NEW_CONNECTION_START);
                    // win8 OS might take 350ms for DHCP exchange, keep for 400ms for this.
                    PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, /*400*/1300);   

                    // Set connected status as client operation
                    MT_SET_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP);
                }
#endif /*MULTI_CHANNEL_SUPPORT*/
                
                DBGPRINT(RT_DEBUG_TRACE, ("%s- I am P2P Client. P2pindex = %d. Aid=%d. ValidAsP2P=%d. P2pMsConnectedStatus=%d. \n", __FUNCTION__, index, pEntry->Aid, pEntry->ValidAsP2P, pAd->pP2pCtrll->P2pMsConnectedStatus));
            }
        }
        else if ( pPort->PortType == WFD_GO_PORT)
        {
#ifdef MULTI_CHANNEL_SUPPORT
            // GO is connected by peer CLI and cehck STA/CLI status
            // Defer a task to handle multi-channel state and DHCP
            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_IN_NEW_CONNECTION_START);
            PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, /*400*/700);    
#endif /*MULTI_CHANNEL_SUPPORT*/
            
            // =========== GO Case ==============
            if (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
            {
                // Later we will set dhcp, don't do scan during this 30 sec
                pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = 0;
                
                // Copy p2p devinfo to MacTab
                if (index < MAX_P2P_GROUP_SIZE)
                {
                    pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_CLIENT_OPERATING;
                    P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PROVISIONED);          
                    P2pMsCopyP2PTabtoMacTab(pAd, index, (UCHAR)pEntry->Aid, P2P_IS_GO);
                    //PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
                }
                else                
                {
                    // If can't find this peer in P2Ptable. this must be legacy client that only in MACtab.
                    // And its client state is already stored in MacEntry when associate.
                }
                
                // When start GO by OID_DOT11_WFD_START_GO_REQUEST, 
                // Win8 should start to itself dhcp server, driver doesn't need to maintain dhcp server any more.

                // Set connected status as group owner operation
                MT_SET_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP);
                
                DBGPRINT(RT_DEBUG_TRACE, ("%s- I am P2P GO. P2pindex = %d. Aid=%d. ValidAsP2P=%d. GRPCAP_OWNER=%d. P2pMsConnectedStatus=%d.\n", 
                    __FUNCTION__, index, pEntry->Aid, pEntry->ValidAsP2P, (pPort->PortCfg.P2pCapability[1] & GRPCAP_OWNER), pAd->pP2pCtrll->P2pMsConnectedStatus));
            }

            P2pMsUpdateGroupLimitCap(pAd, pPort);
        }
        
        // Reset the P2pMsSatetPhase to IDLE after successful 4-way handshaking
        pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
        pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = WFD_BLOCKING_SCAN_TIMEOUT; 
    }
    
}

VOID 
P2pMsMlmeCntLinkUp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    ULONG       MACValue;
    ULONG       MACValue2;
    UCHAR       Wcid = 127;
    UCHAR       Index = 0;
    UCHAR       StopGroup = 0;
    ULONG       GroupFormErrCode = 0;
    BOOLEAN     IsP2PShutdown = FALSE;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    PMP_PORT pWfdDevicePort = pAd->pP2pCtrll->pWfdDevicePort;
    
    if (P2P_OFF(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s return. P2P is OFF  !!! \n", __FUNCTION__));
        return;
    }

   if (pWfdDevicePort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s return. pWfdDevicePort is NULL  !!! \n", __FUNCTION__));
        return;
    }

    if (pPort->PortType == EXTSTA_PORT)
        {

        // if P2pMs (CLI or GO) has not connection, set the P2pMs listen/op channel to Ext_STA
        if (P2PMSCheckAnyWFDLinkUp (pAd) == FALSE)
            {
            if (pWfdDevicePort->P2PCfg.ListenChannel != pPort->Channel)
                {
                pWfdDevicePort->P2PCfg.GroupChannel = pPort->Channel;
                // P2pMs use below item 
                pWfdDevicePort->P2PCfg.GroupOpChannel = pPort->Channel;

                pWfdDevicePort->P2PCfg.ListenChannel = pPort->Channel;
                DBGPRINT(RT_DEBUG_TRACE, ("%s - ExtSTA use (ch=%d) and P2pMs has no connection, set M2P2P listen/op to (ch=%d). CHANGE\n", __FUNCTION__, pPort->Channel, pPort->P2PCfg.ListenChannel));
                }
            
        }
        else if (pWfdDevicePort->P2PCfg.ListenChannel != pPort->Channel)
                {
            // Currently (2011/10/20), ralink driver might not support ExtSTA/CLI/GO use different channel
            DBGPRINT(RT_DEBUG_TRACE, ("%s - ExtSTA use ch(%d) and P2pMs had connected. M2P2P listen/op(ch=%d) is different form ExtSTA. !!!TAKE CARE OF THIS!!!\n", __FUNCTION__, pAd->HwCfg.LatchRfRegs.Channel, pPort->P2PCfg.ListenChannel));
                }

        DBGPRINT(RT_DEBUG_TRACE, ("%s - P2pMsConnectedStatus(%d)\n", __FUNCTION__, pAd->pP2pCtrll->P2pMsConnectedStatus));
            }
    else if (pPort->PortType == WFD_CLIENT_PORT)
    {
        if ((!INFRA_ON(pAd->pP2pCtrll->pInfraPort)) )
            {
            PMP_PORT pInfraPort = pAd->pP2pCtrll->pInfraPort;
           

            pWfdDevicePort->P2PCfg.ListenChannel = pPort->Channel;

            pInfraPort->Channel = pPort->Channel;
            pInfraPort->CentralChannel = pPort->CentralChannel;
            pInfraPort->BBPCurrentBW = pPort->BBPCurrentBW;

        }
    }   
    

    // Case 1: P2P network is earlier than WlanSta network
    //
    if ((pPort->PortNumber == PORT_0) &&
        (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) ||
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)))
    {
        // If WlanSta start an adhoc network, need to stop Group.
        if (ADHOC_ON(pPort) && (pAd->StaCfg.BssType == BSS_ADHOC))
        {
            StopGroup = 1;
            GroupFormErrCode = P2P_ERRCODE_ADHOCON;
        }
        // If WlanSta and P2pCli/P2pGO use different channels, need to stop Group.
        else if (pPort->P2PCfg.GroupOpChannel != pPort->Channel)
        {
#ifdef MULTI_CHANNEL_SUPPORT
            DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] - MultiChSts, CLI op ch(%d) is different from STA ch(%d), keep alive for this conenction\n", __FUNCTION__, __LINE__, pPort->P2PCfg.GroupOpChannel, pPort->Channel));
#else
        
            StopGroup = 2;
            GroupFormErrCode = P2P_ERRCODE_CHANNELCONFLICT;
#endif /* MULTI_CHANNEL_SUPPORT */      
        }
        // If WlanSta and P2pCli use different PhyMode, need to stop Group.
        // GO can coexist between 11N-20MHz and BG mode.
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) &&
                (pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_LEGACY_ONLY) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE))
        {
// 2011-11-23 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
            DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] - MultiChSts, Bypass StopGroup(3) P2pPhyMode(%d), bHtEnable(%d)\n", __FUNCTION__, __LINE__, pPort->P2PCfg.P2pPhyMode, pAd->StaActive.SupportedHtPhy.bHtEnable));
#else
            StopGroup = 3;
            GroupFormErrCode = P2P_ERRCODE_PHYMODECONFLICT;
#endif  /* MULTI_CHANNEL_SUPPORT */
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) &&
                (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY) && (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE))
        {
// 2011-11-23 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
            DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] - MultiChSts, Bypass StopGroup(4) P2pPhyMode(%d), bHtEnable(%d)\n", __FUNCTION__, __LINE__, pPort->P2PCfg.P2pPhyMode, pAd->StaActive.SupportedHtPhy.bHtEnable));
#else
            StopGroup = 4;
            GroupFormErrCode = P2P_ERRCODE_PHYMODECONFLICT;
#endif          
        }
        // If WlanSta and P2pCli/P2pGO use different central channels, need to stop Group.
        else if ((pPort->P2PCfg.CentralChannel != pPort->CentralChannel) && (pPort->P2PCfg.CentralChannel != 0))
        {
// 2011-11-27 For multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
#else
            StopGroup = 5;
            GroupFormErrCode = P2P_ERRCODE_CHANNELCONFLICT;
#endif  /* MULTI_CHANNEL_SUPPORT */         
        }
        // The control channel is the same.
        else if (pPort->P2PCfg.GroupOpChannel == pPort->Channel)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Same control channel  = %d.!!!!!!!!! \n", pPort->P2PCfg.GroupOpChannel));
            StopGroup = 0;
            if (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]))
            {
                if ((pAd->StaActive.SupportedHtPhy.HtChannelWidth == 1) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE))
                {
                    if ((pAd->StaActive.SupportedHtPhy.ExtChanOffset == pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("GO keeps as 40MHz !ExtChanOffset = %d \n", pAd->StaActive.SupportedHtPhy.ExtChanOffset ));
                    }
                    else if (pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth == 0)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("INFRA use 40MHz. GO keeps as 20MHz ! \n"));
                    }
                    else
                    {   // Might not happen, we already filter out different BW before.
                        DBGPRINT(RT_DEBUG_TRACE, ("(Might not happen) 2nd channel not match.\n"));
                    }
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("INFRA use 20MHz. So GO Use 20MHz too !! \n"));
                }
            }
        }

        if (StopGroup)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Case 1(%d): Resume group due to different channel or phymode or adhoc(control=%d %d. central=%d %d. P2pPhyMode =%d. bHtEnable=%d. adhoc=%d, GroupFormErrCode=%d)\n",
                                        StopGroup, 
                                        pPort->P2PCfg.GroupOpChannel, 
                                        pPort->Channel, 
                                        pPort->P2PCfg.CentralChannel, 
                                        pPort->CentralChannel, 
                                        pPort->P2PCfg.P2pPhyMode, 
                                        pAd->StaActive.SupportedHtPhy.bHtEnable, 
                                        ADHOC_ON(pPort), GroupFormErrCode));

            // If I am autoGo and no clinet connects to me, resume my group without clients
            if (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]) &&
                (INFRA_ON(pAd->PortList[PORT_0])) &&
                (pPort->MacTab.Size <= 1) &&
                (GroupFormErrCode != P2P_ERRCODE_ADHOCON))
            {
                GroupFormErrCode = 0;
                IsP2PShutdown = TRUE;
                P2PResumeGroup(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber], P2P_IS_GO, FALSE);
            }
            // Dissolve my group
            else
            {
                // Client #1
                if (IS_P2P_MS_CLI(pAd,  pAd->PortList[pPort->P2PCfg.ClientPortNumber])&&
                    INFRA_ON(pAd->PortList[pPort->P2PCfg.ClientPortNumber]))
                {
                    P2pMsDown(pAd, pAd->PortList[pPort->P2PCfg.ClientPortNumber]);
                    IsP2PShutdown = TRUE;
                }
                // Client #2
                if (IS_P2P_MS_CLI2(pAd,  pAd->PortList[pPort->P2PCfg.Client2PortNumber])&&
                    INFRA_ON(pAd->PortList[pPort->P2PCfg.Client2PortNumber]))
                {
                    P2pMsDown(pAd, pAd->PortList[pPort->P2PCfg.Client2PortNumber]);
                    IsP2PShutdown = TRUE;
                }

                if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP) &&
                    IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]))
                {
                    P2pMsDown(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]);
                    IsP2PShutdown = TRUE;
                }
                
                // Delete P2P Device that I previously tried to connect.
                if (IsP2PShutdown == TRUE)
                {
                    for (Index = 0; Index < MAX_P2P_GROUP_SIZE; Index++)
                    {
                        if (pAd->pP2pCtrll->P2PTable.Client[Index].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                            P2pGroupTabDelete(pAd, Index, pAd->pP2pCtrll->P2PTable.Client[Index].addr);
                    }
                    P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_AUTO, 2);
                }
            }

            return;
        }

    }

    // Druing STA connection, if there is no AP available, STA will trigger once scan sceanrio and pasue Beacon
    // Resume beacon here if necessary
    if (StopGroup == FALSE)
    {
        AsicResumeBssSync(pAd);
    }

    if ((pPort->PortNumber == PORT_0) || 
        ((pPort->P2PCfg.ClientPortNumber != pPort->PortNumber) &&
        (pPort->P2PCfg.Client2PortNumber != pPort->PortNumber)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2P link up return. connectedstatus=%d. port number=(%d  %d  %d)\n", pAd->pP2pCtrll->P2pMsConnectedStatus, pPort->P2PCfg.ClientPortNumber, pPort->P2PCfg.Client2PortNumber, pPort->PortNumber));
        return;
    }

    // [Win8 WFD]
    // Because it's hard to maintain different listen channel and working channel on STA+GO+Clients
    // So we change listen channel to this working channel once connection is active.
    // And reset listen channel to default after linkdown
    if (pPort->P2PCfg.ListenChannel != pPort->Channel)
    {
        pPort->P2PCfg.ListenChannel = pPort->Channel;
        DBGPRINT(RT_DEBUG_TRACE, ("2    P2pMsMlmeCntLinkUp - Follow common channel setting.becomes  ListenChannel = %d. \n", pPort->P2PCfg.ListenChannel));
    }
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));  
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, P2pGetClientWcid(pAd, pPort)));
        return;
    }
    
    // Use the owner of P2P wcid in concurrent Client if this entry has inserted
    if ((pPort->PortType == WFD_CLIENT_PORT) && (pWcidMacTabEntry->ValidAsCLI))
    {
        Wcid = P2pGetClientWcid(pAd, pPort);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s - Use Wcid=%d !!! \n", __FUNCTION__, Wcid));

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,Wcid));
        return;
    }
    
    // Update status as p2p client association
    pWcidMacTabEntry->ValidAsP2P = TRUE;
    MT_SET_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_ASSOC);
    pWcidMacTabEntry->P2pInfo.P2pClientState = P2PSTATE_GO_ASSOC;   // mean I assoc to GO.
    
    pPort->PortCfg.P2pCapability[0] &= (~DEVCAP_DEVICE_LIMIT);
    pPort->P2PCfg.MyGOwcid = Wcid;

    // Default is 11N-20, update p2p phy mode once link-up
    if ((pPort->Channel != pPort->CentralChannel) &&
        (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_ENABLE_11N_40))
    {
        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
    }
    else if((pPort->Channel == pPort->CentralChannel) &&
        (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE) &&
        (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY))
    {
        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_LEGACY_ONLY;
    }

    P2pEdcaDefault(pAd, pPort);

    // Save the current central channel as P2P's Port linkup
    if (pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_ENABLE_11N_40)
    {
        pPort->P2PCfg.CentralChannel = pPort->CentralChannel;
    }
    else
    {
        pPort->P2PCfg.CentralChannel = pPort->Channel;
#ifdef MULTI_CHANNEL_SUPPORT
        pPort->P2PCfg.P2PChannel = pPort->Channel;
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : 20M, CentralChannel(%d), P2PChannel(%d)\n", __FUNCTION__, pPort->P2PCfg.CentralChannel, pPort->P2PCfg.P2PChannel));
#endif /*MULTI_CHANNEL_SUPPORT*/
    }

    DBGPRINT(RT_DEBUG_TRACE, ("!!!P2pMsMlmeCntLinkUp. ConnectedState = %d !!!WscState = %d. P2pPhyMode = %d\n", pAd->pP2pCtrll->P2pMsConnectedStatus, pPort->StaCfg.WscControl.WscState, pPort->P2PCfg.P2pPhyMode));
    //DBGPRINT(RT_DEBUG_TRACE, ("ClientFlags = [1]  = %x. [2] = %x. [3] = %x  \n", pPort->MacTab.Content[1].ClientStatusFlags,  pPort->MacTab.Content[2].ClientStatusFlags,  pPort->MacTab.Content[3].ClientStatusFlags));
    DBGPRINT(RT_DEBUG_TRACE, ("!!!pAd Flags = %x !!!GroupOpChannel = %d.  CommonCfg.Channel = %d. P2PCfg.CentralChannel=%d\n", pAd->Flags, pPort->P2PCfg.GroupOpChannel, pPort->Channel, pPort->P2PCfg.CentralChannel));
    P2PPrintState(pAd);

    RTUSBReadMACRegister(pAd, 0x1808, &MACValue);
    RTUSBReadMACRegister(pAd, 0x180c, &MACValue2);
    DBGPRINT(RT_DEBUG_TRACE, ("!!!0x1808/0c = %x %x !!! \n", MACValue, MACValue2));
    RTUSBReadMACRegister(pAd, 0x1810, &MACValue);
    RTUSBReadMACRegister(pAd, 0x1814, &MACValue2);
    DBGPRINT(RT_DEBUG_TRACE, ("!!!0x1810/14 = %x %x !!! \n", MACValue, MACValue2));
}

VOID 
P2pMsMlmeCntLinkDown(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN  BOOLEAN         IsReqFromAP)
{
    UCHAR       p2pindex = P2P_NOT_FOUND;
    PMP_PORT  pCheckPort = NULL,  pCheckPort2 = NULL, pTmpPort = NULL;
    UCHAR       i;
    
    if (P2P_OFF(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s return. P2P is OFF  !!! \n", __FUNCTION__));
        return;
    }

    PlatformZeroMemory(&pPort->P2PCfg.GoNOpChannel, sizeof(DOT11_WFD_CHANNEL));

    if ((pPort->PortNumber == PORT_0) || 
        ((pPort->P2PCfg.ClientPortNumber != pPort->PortNumber) &&
        (pPort->P2PCfg.Client2PortNumber != pPort->PortNumber)))
    {
        // Restore ListenChannel to a social channel when role ports are not operating
        if((!MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP)) &&
            (!MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)))
        {
            // 2012-07-05 When Win8 WHOL WFD performance test, WHCK tool will test STA-AP conncetion firstly, 
            // let MS P2P GO ch follow STA's ch if STA had ever connected to AP before,
            // this can avoid GO side multichannel and increase the pass rate.
            if (pAd->LogoTestCfg.OnTestingWHQL == FALSE)
            {
                // Rest to BW=20MHz, Channel = ListenChannel
                P2pMsDefaultListenChannel(pAd, pPort);
            }
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("P2pMsMlmeCntLinkDown return. Different port number (%d %d %d) \n", pPort->P2PCfg.ClientPortNumber, pPort->P2PCfg.Client2PortNumber, pPort->PortNumber));
        return;
    }


    P2pStopNoA(pAd, NULL);
    P2pStopOpPS(pAd, pPort);
    
    // Stop to maintain Opps flag here
    // Retry limit has also restored when called linkdown()
    pPort->P2PCfg.bOppsOn = FALSE;

    if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
    {
        if (IsReqFromAP == TRUE)
        {
            // PortCfg.LastBssid instead of PortCfg.Bssid
            p2pindex = P2pGroupTabSearch(pAd, pPort->PortCfg.LastBssid);
            if (p2pindex < MAX_P2P_GROUP_SIZE)
            {
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule = P2P_IS_CLIENT;
            }
        }
        P2pGroupTabDelete(pAd, MAX_P2P_GROUP_SIZE, pPort->PortCfg.LastBssid);
    }

    // Rest to BW=20MHz, Channel = ListenChannel
    P2pMsDefaultListenChannel(pAd, pPort);


    pPort->P2PCfg.CentralChannel = 0;// not used
    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;


    // === Multiple STA+Clients: Found out all possible CheckPort(s) =======
    pCheckPort = Ndis6CommonGetStaPort(pAd);

    for (i = 0; i < NUM_OF_CONCURRENT_CLIENT_ROLE; i++)
    {
        pTmpPort = MlmeSyncGetP2pClientPort(pAd, i);
        // this port is not the current port, found it
        if (pTmpPort && (pTmpPort->PortNumber != pPort->PortNumber))
            break;
        else
            pTmpPort = NULL;
    }
    pCheckPort2 = pTmpPort;


    // Upgrade the Txop parameter. to make a fair discoverable chance.
    // Make sure probe response is tx on a fair paramter.
    if (pCheckPort && (!IDLE_ON(pCheckPort)))
        ;
    else if (pCheckPort2 && (!IDLE_ON(pCheckPort2)))
        ;
    else
    {
        RTUSBWriteMACRegister(pAd, EDCA_AC0_CFG, 0xa4300); 
    }

#if UAPSD_AP_SUPPORT
    // Record from WMM-PS GO, reset them
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_BE]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_BK]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_VI]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_VO]   = FALSE;
#endif

    // Disable status if another client port is not connected
    if ((!pCheckPort2) || (pCheckPort2 && IDLE_ON(pCheckPort2)))
    {
        MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_ASSOC);
        MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("!!!P2pMsMlmeCntLinkDown. ConnectedState = %d \n", pAd->pP2pCtrll->P2pMsConnectedStatus));

}

VOID
    P2pMsTimeoutAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort
    )
{
    UCHAR       p2pindex = P2P_NOT_FOUND;
    UCHAR       i;
    PRT_P2P_DISCOVERY_ENTRY pP2pEntry = NULL;
    UCHAR       PeerChannel = 0;
    
#ifdef WFD_NEW_PUBLIC_ACTION

    // If any non-idle state change, kick a counter to block normal scan.
    // After WFD_BLOCKING_SCAN_TIMEOUT, go back to idle state.
    // Once any error status is carried in p2p action frame by RX, go back to idle state.
    if ((pPort->P2PCfg.P2pMsSatetPhase > P2pMs_STATE_IDLE) && 
        (pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand > WFD_BLOCKING_SCAN_TIMEOUT))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - WFD_BLOCKING_SCAN_TIMEOUT, change the state from %s to P2pMs_STATE_IDLE\n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));
        pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
        
        // reset states related to new public action frame
        pPort->P2PCfg.PrevState = P2P_NULL_STATE;
        pPort->P2PCfg.CurrState = P2P_NULL_STATE;
        pPort->P2PCfg.Signal = P2P_NULL_STATE;
        pPort->P2PCfg.LostAckRetryCounter = 0;
        pPort->P2PCfg.LostRspRetryCounter = 0;
    }
#else

    // If any non-idle state change, kick a counter to block normal scan.
    // After WFD_BLOCKING_SCAN_TIMEOUT, go back to idle state.
    // Once any error status is carried in p2p action frame by RX, go back to idle state.
    if ((pPort->P2PCfg.P2pMsSatetPhase > P2pMs_STATE_IDLE) && 
        (pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand > WFD_BLOCKING_SCAN_TIMEOUT))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - WFD_BLOCKING_SCAN_TIMEOUT, change the state from %s to P2pMs_STATE_IDLE\n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));
        pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
    }

    // Retry to search the peer's listench for buffered frame
    if ((pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand < pPort->P2PCfg.MSSendTimeout) &&
        (pPort->P2PCfg.WaitForMSCompletedFlags > P2pMs_WAIT_COMPLETE_NOUSE) &&
        (pPort->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY) &&
        (pPort->P2PCfg.P2pCounter.StartRetryCounter > 5) &&
        (pPort->P2PCfg.RestoreChannelInProgress == FALSE) &&
        (pPort->P2PCfg.P2pCounter.SwitchingChannelCounter > (WFD_CHANNEL_SWITCH_TIMEOUT + 1)) &&
        (pPort->P2PCfg.pRequesterBufferedFrame))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Failed to find peer's listen ch for MS P2P buffer frame, CurTime(%d)ms, Timeout(%d)ms, try again after 500 ms\n", 
                                    __FUNCTION__,
                                    pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand * 100,
                                    pPort->P2PCfg.MSSendTimeout * 100));

        // After 500 ms, re-transmit action frame
        // Start retry counter
        pPort->P2PCfg.P2pCounter.StartRetryCounter = 0;
        
    }
    else if ((pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand < pPort->P2PCfg.MSSendTimeout) &&
            (pPort->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY) &&
            (pPort->P2PCfg.P2pCounter.StartRetryCounter == 5) &&
            (pPort->P2PCfg.RestoreChannelInProgress == FALSE) &&
            (pPort->P2PCfg.P2pCounter.SwitchingChannelCounter > (WFD_CHANNEL_SWITCH_TIMEOUT + 1)) &&
            (pPort->P2PCfg.pRequesterBufferedFrame))
    {
        if (pPort->P2PCfg.SwitchChannelInProgress == FALSE)
        {
            pPort->P2PCfg.SwitchChannelInProgress = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - StartRetryCounter(%d)\n",__FUNCTION__, pPort->P2PCfg.P2pCounter.StartRetryCounter));
        }
    }

    
    // Go to the peer listen channel when start up handshakes with invitation/provision/nego request.
    if ((pPort->P2PCfg.SwitchChannelInProgress == TRUE) &&
        (pPort->P2PCfg.P2pCounter.SwitchingChannelCounter > WFD_CHANNEL_SWITCH_TIMEOUT) &&
        (pPort->P2PCfg.pRequesterBufferedFrame) &&
        (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED) ||
         MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED) ||
         MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED)))
    {
        pPort->P2PCfg.SwitchChannelInProgress = FALSE;

        //
        // Start go to the peer's listen channel for 500ms
        //
        p2pindex = P2pDiscoTabSearch(pAd, &pPort->P2PCfg.ConnectingMAC[0][0], NULL, NULL);
        // if connecting to GO, use operating channel instead.
        if ((pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].BeaconFrameLen > 0) && (pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].OpChannel > 0))
        {
            PeerChannel = pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].OpChannel;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P  connect to GO by ch (%d), CutTime(%d)ms \n", __FUNCTION__, PeerChannel, pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand * 100));      
        }
        else
        {
            PeerChannel = pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].ListenChannel;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P  connect to peer by ch (%d), CurTime(%d)ms \n", __FUNCTION__, PeerChannel, pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand * 100));        
        }
    
        if ((p2pindex < MAX_P2P_GROUP_SIZE) && MlmeSyncIsValidChannel(pAd, PeerChannel) && (pPort->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY))
        {
            pPort->P2PCfg.P2pProprietary.ListenChanel[0] = PeerChannel;
            pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
            pPort->P2PCfg.P2pProprietary.ListenChanelCount = 1;
            pPort->P2PCfg.P2pCounter.ListenIntervalBias = WFD_PEER_CHANNEL_ACTION_TIMEOUT;
            pPort->P2PCfg.P2pCounter.NextScanRound = 1;
            pPort->P2PCfg.P2pCounter.ListenInterval = (UCHAR)pPort->P2PCfg.P2pCounter.ListenIntervalBias;
            pPort->P2PCfg.ScanPeriod = (UCHAR)(pPort->P2PCfg.P2pCounter.ListenInterval + pPort->P2PCfg.P2pCounter.NextScanRound);
            pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;   // After scan period, go back to listen idle (counter reset to zero)
            pPort->P2PCfg.P2PDiscoProvState = P2P_SEARCH;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P  Go to the peer's listen channel (%d) for a while = %d, scanperiod = %d \n", __FUNCTION__, PeerChannel, pPort->P2PCfg.P2pCounter.ListenInterval, pPort->P2PCfg.ScanPeriod));        
            P2pScan(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
        }
        else    
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Not found the peer, indicate failure to MS (index=%d, state=%s)\n", __FUNCTION__, p2pindex, decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));        

            // not found, indicate MS with failure 
            if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Not found the peer, indicate failure to MS \n", __FUNCTION__))
#ifdef DBG              
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE, __LINE__);
#else
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE);
#endif
                //Free Buffered Data
                P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);
                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED);
            }
            else if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED))
            {
#ifdef DBG              
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE, __LINE__);
#else
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE);
#endif
                //Free Buffered Data
                P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);
                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED);
            }
            else if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED))
            {
#ifdef DBG              
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE, __LINE__);
#else
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE);
#endif
                //Free Buffered Data
                P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);
                MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED);
            }

        }
    }

    // Go back to my listen/common channel when end up handshakes with error status.
    if ((pPort->P2PCfg.RestoreChannelInProgress == TRUE)  && 
        (pPort->P2PCfg.P2pCounter.SwitchingChannelCounter > WFD_CHANNEL_SWITCH_TIMEOUT) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))     
    {
        pPort->P2PCfg.RestoreChannelInProgress = FALSE;
        P2pMsRestoreChannel(pAd, pPort,  pAd->HwCfg.LatchRfRegs.Channel);
    }

    if ((pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand > /*WFD_COMPLETE_SEND_TIMEOUT*/pPort->P2PCfg.MSSendTimeout) &&
        (pPort->P2PCfg.WaitForMSCompletedFlags > P2pMs_WAIT_COMPLETE_NOUSE))
    {
        // case 1: send to the air, but not received any ack
        if((pPort->P2PCfg.pSendOutBufferedFrame) &&
            (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ) ||
            MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_RSP) ||
            MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ) ||
            MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_RSP) ||
            MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_COM) ||
            MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ) ||
            MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_RSP)))
        {
#ifdef DBG          
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize, NDIS_STATUS_FAILURE, __LINE__);
#else
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize, NDIS_STATUS_FAILURE);
#endif

            PlatformFreeMemory(pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize);
            pPort->P2PCfg.pSendOutBufferedFrame = NULL;
            pPort->P2PCfg.SendOutBufferedSize = 0;
        }
        // case 2: Not send to the air because we can't search the peer in p2p_search state.
        else if ((pPort->P2PCfg.pRequesterBufferedFrame) &&
                (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED) ||
                 MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED) ||
                 MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED) ||
                 MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED_AFTER_SCAN)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Failed to find peer's listen ch for MS P2P buffer frame, CurTime(%d)ms, Timeout(%d)ms, indicate error to os\n", 
                                        __FUNCTION__,
                                        pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand * 100,
                                        pPort->P2PCfg.MSSendTimeout * 100));

#ifdef DBG  
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE, __LINE__);
#else
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE);
#endif

            //Free Buffered Data
            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);
        }
    }
#endif /*WFD_NEW_PUBLIC_ACTION*/

    // == Discovery Table maintain ==
    // Devices which have not been visible for more than 5 minutes must not be reported to the OS.
    for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
    {       
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.DiscoEntry[i];
        if (pP2pEntry->bValid == TRUE)
        {
            if (pP2pEntry->LifeTime > 0)
                pP2pEntry->LifeTime--;
            
            if (pP2pEntry->LifeTime == 0)
            {
                P2pDiscoTabDelete(pAd, i, NULL);
            }
        }
    }

    // Indicate to discovery complete quickly to OS during p2p handshakes
    if (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_QUICK_DISCO_REQ) && 
        (pPort->P2PCfg.P2pCounter.QuickDiscoveryListUpdateCounter == 0))
    {
        P2pMsIndicateDiscoverComplete(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
        MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_QUICK_DISCO_REQ);
    }
}

/*
    ========================================================================
    Routine Description:
        Restore to the original Central/Control Channel

    Arguments:
        pAd                 Pointer to our adapter
        Channel             current channel I stayed in

    Note:
    ========================================================================
*/
VOID
P2pMsRestoreChannel(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR    Channel)
{
    UCHAR   Value, PortNum = 0;
    ULONG   Data;
    BOOLEAN bConcurrent = FALSE;
    

    DBGPRINT(RT_DEBUG_TRACE, ("%s.  ==> \n", __FUNCTION__));
    DBGPRINT(RT_DEBUG_TRACE, ("BW = %d. Cha = %d . CenCha = %d. current Ch = %d . CntlState = %d. SyncState = %d \n",  
        pPort->BBPCurrentBW, pPort->Channel, pPort->CentralChannel, Channel,
        pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState, pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState));

//  if (pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState == CNTL_IDLE &&
//      (pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState == SYNC_IDLE))
    if (1)
    {
        
        for (PortNum = 0 ; PortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/ ; PortNum ++)
        {
            if ((pAd->PortList[PortNum] == NULL) || (pAd->PortList[PortNum]->bActive == FALSE))
                continue;

            if (OPSTATUS_TEST_FLAG(pAd->PortList[PortNum], fOP_STATUS_MEDIA_STATE_CONNECTED))
                bConcurrent = TRUE;
        }

#if 0
        if ((bConcurrent == FALSE) && (pPort->CentralChannel != pPort->Channel))
        {
            pPort->CentralChannel = pPort->Channel;
            DBGPRINT(RT_DEBUG_TRACE, ("Let CentralChannel follow ControlChannel if no connection \n"));
        }
#endif
        
        // Change to my current channel
        //if ((pPort->CentralChannel > pPort->Channel) && (Channel != pPort->CentralChannel))
        //if (pPort->BBPCurrentBW == BW_40)
        if (pPort->P2PCfg.P2PGOCentralChannel != CHANNEL_OFF)
        {   
            //if ((pAd->StaCfg.Feature11n&0x3) != 0x03) // 0x3 means we have switch to HT40
            if (pPort->P2PCfg.P2PGOExtChanOffset == EXTCHA_ABOVE)
            {
                // Must using 40MHz.
                pPort->BBPCurrentBW = BW_40;
                AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                
                
                RTUSBReadBBPRegister(pAd, BBP_R4, &Value);
                Value &= (~0x18);
                Value |= 0x10;
                RTUSBWriteBBPRegister(pAd, BBP_R4, Value);
                
                //  RX : control channel at lower 
                RTUSBReadBBPRegister(pAd, BBP_R3, &Value);
                Value &= (~0x20);
                RTUSBWriteBBPRegister(pAd, BBP_R3, Value);

                RTUSBReadMACRegister(pAd, TX_BAND_CFG, &Data);
                Data &= 0xfffffffe;
                RTUSBWriteMACRegister(pAd, TX_BAND_CFG, Data);

                if (pAd->HwCfg.MACVersion == 0x28600100)
                {
                    RTUSBWriteBBPRegister(pAd, BBP_R69, 0x1A);
                    RTUSBWriteBBPRegister(pAd, BBP_R70, 0x0A);
                    RTUSBWriteBBPRegister(pAd, BBP_R73, 0x16);
                    DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
                }

                DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower Band !!! Control Channel at Below. Central = %d \n", pPort->CentralChannel));
            }
        //}
        //else if ((pPort->CentralChannel < pPort->Channel) && (pPort->BBPCurrentBW == BW_40)/*(Channel != pPort->CentralChannel)*/)
        //{
            //else if ((pAd->StaCfg.Feature11n&0x3) != 0x03) // 0x3 means we have switch to HT40
            else if (pPort->P2PCfg.P2PGOExtChanOffset == EXTCHA_BELOW)
            {
                // Must using 40MHz.
                pPort->BBPCurrentBW = BW_40;
                AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                
                RTUSBReadBBPRegister(pAd, BBP_R4, &Value);
                Value &= (~0x18);
                Value |= 0x10;
                RTUSBWriteBBPRegister(pAd, BBP_R4, Value);
                
                RTUSBReadMACRegister(pAd, TX_BAND_CFG, &Data);
                Data &= 0xfe;
                Data |= 0x1;
                RTUSBWriteMACRegister(pAd, TX_BAND_CFG, Data);
                
                RTUSBReadBBPRegister(pAd, BBP_R3, &Value);
                Value |= (0x20);
                RTUSBWriteBBPRegister(pAd, BBP_R3, Value);

                if (pAd->HwCfg.MACVersion == 0x28600100)
                {
                    RTUSBWriteBBPRegister(pAd, BBP_R69, 0x1A);
                    RTUSBWriteBBPRegister(pAd, BBP_R70, 0x0A);
                    RTUSBWriteBBPRegister(pAd, BBP_R73, 0x16);
                    DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
                }

                DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Upper Band !!! Control Channel at UpperCentral = %d \n", pPort->CentralChannel));
            }
        }
        else if (Channel != pPort->Channel)
        {
            pPort->BBPCurrentBW = BW_20;
            AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
            
            RTUSBReadBBPRegister(pAd, BBP_R4, &Value);
            Value &= (~0x18);
            RTUSBWriteBBPRegister(pAd, BBP_R4, Value);
            
            RTUSBReadMACRegister(pAd, TX_BAND_CFG, &Data);
            Data &= 0xfffffffe;
            RTUSBWriteMACRegister(pAd, TX_BAND_CFG, Data);
            
            RTUSBReadBBPRegister(pAd, BBP_R3, &Value);
            Value &= (~0x20);
            RTUSBWriteBBPRegister(pAd, BBP_R3, Value);
            
            if (pAd->HwCfg.MACVersion == 0x28600100)
            {
                RTUSBWriteBBPRegister(pAd, BBP_R69, 0x16);
                RTUSBWriteBBPRegister(pAd, BBP_R70, 0x08);
                RTUSBWriteBBPRegister(pAd, BBP_R73, 0x11);
                DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
            }
            
            DBGPRINT(RT_DEBUG_TRACE, ("!!!20MHz !!! \n" ));
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Feature11n = 0x%0x, reset bit1\n",__FUNCTION__ ,pAd->StaCfg.Feature11n));
        pAd->StaCfg.Feature11n &= (~0x2);       
        
        DBGPRINT(RT_DEBUG_TRACE, ("<== BW = %d. Cha = %d . CenCha = %d. GroupOpChannel = %d. ListenChannel= %d. \n",  pPort->BBPCurrentBW, pPort->Channel, pPort->CentralChannel, pPort->P2PCfg.GroupOpChannel, pPort->P2PCfg.ListenChannel));
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - Faield to restore channel. Port0_CntlCurState(%d), Port0_SyncCurState(%d)\n",  
                                    __FUNCTION__,
                                    pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState,
                                    pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState));
    }

    // Resume beacon after restore channel
    if (pPort->P2PCfg.bGOStart)
    {
        AsicResumeBssSync(pAd);
    }
}

VOID
P2pMsUpdateGroupLimitCap(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    PMP_PORT      pGoPort = NULL;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    UCHAR           ConnectedNum = 0, SecuredNum = 0;
    PQUEUE_HEADER pHeader;

    if (P2P_OFF(pPort))
        return;
    
    // Only support one Group Owner
    if (!IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]))
        return;

    pGoPort = pAd->PortList[pPort->P2PCfg.GOPortNumber];

    // no any client associate to GO
    if ((pPort->P2PCfg.MaximumGroupLimit == 0) ||
        (!MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus,  P2pMs_CONNECTED_STATUS_GO_ASSOC)))
        return;

    pHeader = &pGoPort->MacTab.MacTabList;
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

        if ((pMacEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_PSK)
            || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_WPS)
            || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_ASSOC && pMacEntry->ValidAsP2P == TRUE)
            || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_OPERATING && pMacEntry->ValidAsP2P == TRUE)
            || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_WPS && pMacEntry->ValidAsP2P == TRUE))
        {
            ConnectedNum ++;

            if (pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED)
                SecuredNum ++;
        }
        pNextMacEntry = pNextMacEntry->Next;   
        pMacEntry = NULL;
    }

    if (ConnectedNum >= pPort->P2PCfg.MaximumGroupLimit)
    {
        MT_SET_BIT(pGoPort->PortCfg.P2pCapability[1], GRPCAP_LIMIT);
    }
    else if (MT_TEST_BIT(pGoPort->PortCfg.P2pCapability[1], GRPCAP_LIMIT))
    {
        MT_CLEAR_BIT(pGoPort->PortCfg.P2pCapability[1], GRPCAP_LIMIT);
    }
        

    DBGPRINT(RT_DEBUG_TRACE, ("%s - MaxGroupLimitNum=%d, connectedNum=%d, SecuredNum=%d \n", __FUNCTION__, pPort->P2PCfg.MaximumGroupLimit, ConnectedNum, SecuredNum));

}

/*  
    ==========================================================================
    Description: 
        Copy P2P Table's information to Mac Table when the P2P Device is in my group.
        All of role ports are updated to the same p2p entry with different interface address 
        and group owner capability. We need to save correct p2pinfo to MacTable.
        
    Parameters:
    Note:
    ==========================================================================
 */
VOID 
P2pMsCopyP2PTabtoMacTab(
    IN PMP_ADAPTER pAd,
    IN UCHAR        P2pindex,
    IN UCHAR        Macindex,
    IN UCHAR        MyRule)
{
    PMAC_TABLE_ENTRY        pEntry;
    PRT_P2P_CLIENT_ENTRY    pP2pEntry;
    PMP_PORT    pPort= pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Macindex);   

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,Macindex));
        return;
    }
    
    if ((P2pindex >= MAX_P2P_GROUP_SIZE) || (Macindex >= MAX_LEN_OF_MAC_TABLE))
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("%s ===> \n", __FUNCTION__));

    pEntry = pWcidMacTabEntry;
    pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[P2pindex];
    pEntry->P2pInfo.CTWindow = pP2pEntry->CTWindow;
    pEntry->P2pInfo.P2pClientState = pP2pEntry->P2pClientState;
    pEntry->P2pInfo.P2pFlag = pP2pEntry->P2pFlag;
    pEntry->P2pInfo.NoAToken = pP2pEntry->NoAToken;
    pEntry->P2pInfo.GeneralToken = pP2pEntry->GeneralToken;
    pEntry->P2pInfo.ConfigMethod = pP2pEntry->ConfigMethod;
    pEntry->P2pInfo.DevCapability = pP2pEntry->DevCapability;
    if (MyRule == P2P_IS_CLIENT)
        pEntry->P2pInfo.GroupCapability = 0;
    else
        pEntry->P2pInfo.GroupCapability = pP2pEntry->GroupCapability;

    PlatformMoveMemory(pEntry->P2pInfo.DevAddr, pP2pEntry->addr, MAC_ADDR_LEN);
    PlatformMoveMemory(pEntry->P2pInfo.InterfaceAddr, pWcidMacTabEntry->Addr, MAC_ADDR_LEN);
    PlatformMoveMemory(pEntry->P2pInfo.PrimaryDevType, pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);

    pEntry->P2pInfo.DeviceNameLen = pP2pEntry->DeviceNameLen;
    PlatformMoveMemory(pEntry->P2pInfo.DeviceName, pP2pEntry->DeviceName, 32);

    pEntry->P2pInfo.NumSecondaryType = pP2pEntry->NumSecondaryType;
    PlatformMoveMemory(pEntry->P2pInfo.SecondaryDevType, pP2pEntry->SecondaryDevType, MAX_P2P_SECONDARY_DEVTYPE_LIST * P2P_DEVICE_TYPE_LEN);

    if (!PlatformEqualMemory(pP2pEntry->InterfaceAddr, pWcidMacTabEntry->Addr, MAC_ADDR_LEN))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[Win8]P2pEntry InterfaceAddr =  %x %x %x %x %x %x . \n", 
                pP2pEntry->InterfaceAddr[0], pP2pEntry->InterfaceAddr[1], pP2pEntry->InterfaceAddr[2], 
                pP2pEntry->InterfaceAddr[3], pP2pEntry->InterfaceAddr[4], pP2pEntry->InterfaceAddr[5]));
    }
    
    P2PPrintMac(pAd, Macindex);
}

BOOLEAN
P2pMsDeviceAddrAndTypeMatch(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pDesiredAddr,
    IN  ULONG           NumDesiredTypes,    
    IN  PUCHAR          pDesiredTypes,
    IN  BOOLEAN         UseDevInfo,
    IN  PUCHAR          pMyAddr,                // Device ID
    IN  PUCHAR          pSsid,                  // SSID
    IN  UCHAR           SsidLen,                // Length of SSID           
    IN  PUCHAR          pMyPriType,
    IN  ULONG           NumMySecTypes,
    IN  PUCHAR          pMySecTypes,
    IN  UCHAR           MyRule,
    OUT PUCHAR          pMacIdx)
{
    PUCHAR          pTargetAddr = NULL, pTargetPriType = NULL, pTargetSecTypes = NULL;
    ULONG           NumTargetSecTypes = 0, TargetSecTypesTotalSize = 0;
    UCHAR           i = 0, j = 0;
    BOOLEAN         FoundType = FALSE, FoundClientDevId = FALSE, FoundClientType = FALSE, MatchedDeviceIDOrGroupSSID = FALSE;
    UCHAR           ZeroDevType[8] = {0};
    PUCHAR          pCurrReqType = NULL, pSecType = NULL;
    PDOT11_WFD_DISCOVER_DEVICE_FILTER       pDeviceFilter = NULL;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;
    
    if (pMacIdx != NULL)
        *pMacIdx = P2P_NOT_FOUND;
    
    if ((pPort == NULL) || (pPort->bActive == FALSE))
        return FALSE;

    // Compare with my device info
    if ((UseDevInfo == TRUE) && (pAd->PortList[pPort->P2PCfg.PortNumber]))
    {
        pTargetAddr = pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress;
        pTargetPriType = pPort->P2PCfg.PriDeviceType;
        NumTargetSecTypes = pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.WFDNumOfSecondaryDevice;
        pTargetSecTypes = pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.pWFDSecondaryDeviceTypeList;
        TargetSecTypesTotalSize = pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.WFDSecondaryDeviceTypeLen;
    }
    else
    {
        pTargetAddr = pMyAddr;
        pTargetPriType = pMyPriType;
        NumTargetSecTypes = NumMySecTypes;
        pTargetSecTypes = pMySecTypes;
        TargetSecTypesTotalSize = NumTargetSecTypes * 8;
    }

#if 0   
    // 1. Matched desired device adress
    if ((pDesiredAddr != NULL) && (!MAC_ADDR_EQUAL(pDesiredAddr, ZERO_MAC_ADDR)))
    {
        if (pTargetAddr == NULL)
            return FALSE;
        
        if (!MAC_ADDR_EQUAL(pDesiredAddr, pTargetAddr))
        {
            // I am GO and search device address within my clients
            if ((MyRule == P2P_IS_GO) && (IS_P2P_MS_GO(pAd, pPort)))
            {
                for (k = 2;k < MAX_LEN_OF_MAC_TABLE;k++)
                {
                    pEntry = &pPort->MacTab.Content[k];
                    if ((pEntry->ValidAsCLI) && (pEntry->ValidAsP2P) && (pEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_OPERATING))
                    {
                        if (MAC_ADDR_EQUAL(pDesiredAddr, pEntry->P2pInfo.DevAddr))
                        {
                            FoundClientDevId = TRUE;
                            if (pMacIdx != NULL)
                                *pMacIdx = k;
                            break;
                        }
                    }
                }

                if (FoundClientDevId == FALSE)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s -  Device ID is not equal to me and my clients\n", __FUNCTION__));
                    return FALSE;
                }   
                    
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s -  Device ID is not equal to me\n", __FUNCTION__));
                return FALSE;
            }   
        }
    }
#endif

    // 1. Matched DeviceID and GroupSSID (OID V1.42)
    if ((pDesiredAddr != NULL) && MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID))
    {
        if (pTargetPriType == NULL)
            return FALSE;

        pDeviceFilter = (PDOT11_WFD_DISCOVER_DEVICE_FILTER)(pDesiredAddr);
        
        for (i = 0; i < (pPort->P2PCfg.uNumDeviceFilters); i++)
        {
            // 1.a Matched DeviceID
            if (MT_TEST_BITS(pDeviceFilter->ucBitmask, DISCOVERY_FILTER_BITMASK_DEVICE) ||
                MT_TEST_BITS(pDeviceFilter->ucBitmask, DISCOVERY_FILTER_BITMASK_ANY))
            {
                if(MAC_ADDR_EQUAL(pTargetAddr, pDeviceFilter->DeviceID))
                {
                    MatchedDeviceIDOrGroupSSID = TRUE;
                    break;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(%d) - Not Matched device addr(BITMASK_DEVICE) (%x %x %x %x %x %x)\n",
                                                __FUNCTION__,
                                                __LINE__,
                                                *(pTargetAddr + 0),
                                                *(pTargetAddr + 1),
                                                *(pTargetAddr + 2),
                                                *(pTargetAddr + 3),
                                                *(pTargetAddr + 4),
                                                *(pTargetAddr + 5))); 
                }               
            }

            // 1.b Matched GroupSSID
            if (MT_TEST_BITS(pDeviceFilter->ucBitmask, DISCOVERY_FILTER_BITMASK_GO) ||
                MT_TEST_BITS(pDeviceFilter->ucBitmask, DISCOVERY_FILTER_BITMASK_ANY))
            {               
                // GroupSSID is P2P wildcard SSID = DIRECT-, the local P2P device discovers all groups with 
                // P2P Group ID matching the desired devie ID.
                //if (SSID_EQUAL(pSsid, SsidLen, WILDP2PSSID, WILDP2PSSIDLEN))
                if (SSID_EQUAL(pDeviceFilter->GroupSSID.ucSSID, pDeviceFilter->GroupSSID.uSSIDLength, WILDP2PSSID, WILDP2PSSIDLEN))
                {
                    // Check the device address of Group ID with the desired device ID
                    if(MAC_ADDR_EQUAL(pTargetAddr, pDeviceFilter->DeviceID))
                    {
                        MatchedDeviceIDOrGroupSSID = TRUE;
                        break;
                    }                   
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s(%d) -1 Not Matched device addr(BITMASK_GO) (%x %x %x %x %x %x)\n",
                                                    __FUNCTION__,
                                                    __LINE__,
                                                    *(pTargetAddr + 0),
                                                    *(pTargetAddr + 1),
                                                    *(pTargetAddr + 2),
                                                    *(pTargetAddr + 3),
                                                    *(pTargetAddr + 4),
                                                    *(pTargetAddr + 5))); 
                    }                       
                }
                // GroupSSID is not a zero-length SSID, this member specifies the SSID of a P2P group 
                // that the local P2P device discovers
                else if ((SsidLen != 0) && 
                    MAC_ADDR_EQUAL(pTargetAddr, pDeviceFilter->DeviceID) &&
                    (SSID_EQUAL(pSsid, SsidLen, pDeviceFilter->GroupSSID.ucSSID, pDeviceFilter->GroupSSID.uSSIDLength)))
                {
                    MatchedDeviceIDOrGroupSSID = TRUE;
                    break;
                }
                else
                {
                    UCHAR TempSsid[33];

                    PlatformZeroMemory(TempSsid, 33);
                    PlatformMoveMemory(TempSsid, pSsid, SsidLen);
                    
                    DBGPRINT(RT_DEBUG_TRACE,("%s(%d) -2 Not Matched device addr(BITMASK_GO) (%x %x %x %x %x %x), Ssid1(%s), len1(%d), Ssid2(%s), len2(%d)\n",
                                                __FUNCTION__,
                                                __LINE__,
                                                *(pTargetAddr + 0),
                                                *(pTargetAddr + 1),
                                                *(pTargetAddr + 2),
                                                *(pTargetAddr + 3),
                                                *(pTargetAddr + 4),
                                                *(pTargetAddr + 5),
                                                TempSsid,
                                                SsidLen,
                                                pDeviceFilter->GroupSSID.ucSSID,
                                                pDeviceFilter->GroupSSID.uSSIDLength));
                }               
            }

            // Index next device filter
            if (i < (pPort->P2PCfg.uNumDeviceFilters))
            {
                pDeviceFilter++;
            }
        }
    }

    // 2. Matched Device Types
    if ((NumDesiredTypes > 0) && (pDesiredTypes != NULL))
    {       
        if (pTargetPriType == NULL)
            return FALSE;

        /* 
            Both the Category ID and Sub Category ID can be used as a filter. 
            If only looking for devices with a certain Category ID, 
            the OUI and Sub Category ID fields will have to be set to zero
        */
        pCurrReqType = (PUCHAR) pDesiredTypes;
        for (i = 0 ; i < min(NumDesiredTypes, MAX_P2P_FILTER_LIST); i++)
        {
            if (PlatformEqualMemory(pCurrReqType, ZeroDevType, 8))
            {
                // Offset to next Requested Type
                pCurrReqType += 8;
                continue;
            }
            
            // 2.1 Search with matched a specific Category ID
            if (PlatformEqualMemory(pCurrReqType + 2, &ZeroDevType[2], 6))
            {
                // matched with my primary device type
                if (PlatformEqualMemory(pCurrReqType, pTargetPriType, 2))
                {
                    FoundType = TRUE;
                }
                // matched with my secondary device list
                else if ((NumTargetSecTypes > 0) && (pTargetSecTypes != NULL) && (TargetSecTypesTotalSize == NumTargetSecTypes *8))
                {
                    pSecType = (PUCHAR) pTargetSecTypes;
                    for (j = 0 ; j < min(NumTargetSecTypes, MAX_P2P_SECONDARY_DEVTYPE_LIST); j++)
                    {
                        if (PlatformEqualMemory(pCurrReqType, pSecType, 2))
                        {
                            FoundType = TRUE;
                            break;
                        }

                        // Offset to next Secondary Type
                        pSecType += 8;
                    }
                }
            }
            // 2.2 Search with matched both Category ID and Sub Category ID
            else
            {
                // matched with my primary device type
                if (PlatformEqualMemory(pCurrReqType, pTargetPriType, 8))
                {
                    FoundType = TRUE;
                }
                // matched with my secondary device list
                else if ((NumTargetSecTypes > 0) && (pTargetSecTypes != NULL) && (TargetSecTypesTotalSize == NumTargetSecTypes *8))
                {
                    pSecType = (PUCHAR) pTargetSecTypes;
                    for (j = 0 ; j < min(NumTargetSecTypes, MAX_P2P_SECONDARY_DEVTYPE_LIST); j++)
                    {
                        if (PlatformEqualMemory(pCurrReqType, pSecType, 8))
                        {
                            FoundType = TRUE;
                            break;
                        }

                        // Offset to next Secondary Type
                        pSecType += 8;
                    }
                }
            }

            // if found the desired device type, stop to search
            if (FoundType == TRUE)
                break;
            
            // Offset to next Requested Type
            pCurrReqType += 8;
            
        }


        if (FoundType == FALSE)
        {
            // I am GO and search device types within my clients
            if ((MyRule == P2P_IS_GO) && (IS_P2P_MS_GO(pAd, pPort)))
            {
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

                    if ((pMacEntry->ValidAsCLI) && (pMacEntry->ValidAsP2P) && (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_OPERATING))
                    {
                        /* 
                            Both the Category ID and Sub Category ID can be used as a filter. 
                            If only looking for devices with a certain Category ID, 
                            the OUI and Sub Category ID fields will have to be set to zero
                        */
                        pCurrReqType = (PUCHAR) pDesiredTypes;
                        for (i = 0 ; i < min(NumDesiredTypes, MAX_P2P_FILTER_LIST); i++)
                        {
                            if (PlatformEqualMemory(pCurrReqType, ZeroDevType, 8))
                            {
                                // Offset to next Requested Type
                                pCurrReqType += 8;
                                pNextMacEntry = pNextMacEntry->Next;   
                                pMacEntry = NULL;
                                continue; 
                            }
                            
                            // 2.3 Search with matched a specific Category ID from my clients
                            if (PlatformEqualMemory(pCurrReqType + 2, &ZeroDevType[2], 6))
                            {
                                // matched with my primary device type
                                if (PlatformEqualMemory(pCurrReqType, pMacEntry->P2pInfo.PrimaryDevType, 2))
                                {
                                    FoundClientType = TRUE;
                                    
                                }
                                // matched with my secondary device list
                                else if (pMacEntry->P2pInfo.NumSecondaryType > 0)
                                {
                                    for (j = 0 ; j < min(pMacEntry->P2pInfo.NumSecondaryType, MAX_P2P_SECONDARY_DEVTYPE_LIST); j++)
                                    {
                                        pSecType = (PUCHAR) &pMacEntry->P2pInfo.SecondaryDevType[j][0];
                                        if (PlatformEqualMemory(pCurrReqType, pSecType, 2))
                                        {
                                            FoundClientType = TRUE;
                                            break;
                                        }
                                    }
                                }
                            }
                            // 2.4 Search with matched both Category ID and Sub Category ID from my clients
                            else
                            {
                                // matched with my primary device type
                                if (PlatformEqualMemory(pCurrReqType, pMacEntry->P2pInfo.PrimaryDevType, 8))
                                {
                                    FoundClientType = TRUE;
                                }
                                // matched with my secondary device list
                                else if (pMacEntry->P2pInfo.NumSecondaryType > 0)
                                {
                                    for (j = 0 ; j < min(pMacEntry->P2pInfo.NumSecondaryType, MAX_P2P_SECONDARY_DEVTYPE_LIST); j++)
                                    {
                                        pSecType = (PUCHAR) &pMacEntry->P2pInfo.SecondaryDevType[j][0];
                                        if (PlatformEqualMemory(pCurrReqType, pSecType, 8))
                                        {
                                            FoundClientType = TRUE;
                                            break;
                                        }
                                    }
                                }
                            }

                            // if found the desired device type, stop to search
                            if (FoundClientType == TRUE)
                            {
                                if (pMacIdx != NULL)
                                    *pMacIdx = (UCHAR)pMacEntry->wcid;
                                break;
                            }
                            
                            // Offset to next Requested Type
                            pCurrReqType += 8;
                            
                        }
                    }
                    
                    // if found the desired device type, stop to search
                    if (FoundClientType == TRUE)
                        break;
                    
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                }

                if (FoundClientType == FALSE)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s -  Device Type is not equal to me and my clients\n", __FUNCTION__));
                    return FALSE;
                }   
                    
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s -  Device Type is not equal to me\n", __FUNCTION__));
                return FALSE;
            }   
        }   
        
    }

    if ((pDesiredAddr != NULL) && (!MAC_ADDR_EQUAL(pDesiredAddr, ZERO_MAC_ADDR)))
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s - Matched device address (%x %x %x %x %x %x) \n", 
                __FUNCTION__, *pDesiredAddr, *(pDesiredAddr+1), *(pDesiredAddr+2), *(pDesiredAddr+3), *(pDesiredAddr+4), *(pDesiredAddr+5)));
    }
    if ((NumDesiredTypes > 0) && (pDesiredTypes != NULL))
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s - Matched device type (%x %x %x %x %x %x %x %x)\n", 
                __FUNCTION__, *pDesiredTypes, *(pDesiredTypes+1), *(pDesiredTypes+2), *(pDesiredTypes+3), *(pDesiredTypes+4), *(pDesiredTypes+5), *(pDesiredTypes+6), *(pDesiredTypes+7)));
    }

    if (MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID)) 
    {
        return MatchedDeviceIDOrGroupSSID;
    }
    else
    {
        return TRUE;
    }       
}

/*  
    ==========================================================================
    Description: 
        If no infrastructure connect and not GO, set Channel and CentralChannel to P2P's listen channel. This can 
        let P2P device stays most of time in listen state.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pMsDefaultListenChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    PMP_PORT  pStaPort = NULL, pDevPort = NULL, pGOPort = NULL, pClientPort = NULL, pClient2Port = NULL;

    if (P2P_OFF(pPort))
        return;

    pStaPort = pAd->PortList[PORT_0];

    if (IS_P2P_MS_DEV(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]))
        pDevPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    if (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]) && (pPort->P2PCfg.bGOStart))
        pGOPort = pAd->PortList[pPort->P2PCfg.GOPortNumber];

    pClientPort = MlmeSyncGetP2pClientPort(pAd, 0);
    pClient2Port= MlmeSyncGetP2pClientPort(pAd, 1);


    // I can set Channel and CentralChannel to any value. 
    // Because they are not "working channel" yet.
    // Set to Listen channel can improve the discovery time.
    if ((IDLE_ON(pStaPort))
        && (pDevPort)
        && ((!pClientPort) || ((pClientPort) && IDLE_ON(pClientPort)))
        && ((!pClient2Port) || ((pClient2Port) && IDLE_ON(pClient2Port)))
        && (!pGOPort))
    {
        // TODO: temp solution, set to default listen channel ??
        if (MlmeSyncIsValidChannel(pAd, P2P_DEFAULT_LISTEN_CHANNEL))
            pPort->P2PCfg.ListenChannel = P2P_DEFAULT_LISTEN_CHANNEL;
        else if ((P2P_DEFAULT_LISTEN_CHANNEL != 1) && MlmeSyncIsValidChannel(pAd, 1))
            pPort->P2PCfg.ListenChannel = 1;
        else if ((P2P_DEFAULT_LISTEN_CHANNEL != 11) && MlmeSyncIsValidChannel(pAd, 11))
            pPort->P2PCfg.ListenChannel = 11;
        else if ((P2P_DEFAULT_LISTEN_CHANNEL != 6) && MlmeSyncIsValidChannel(pAd, 6))
            pPort->P2PCfg.ListenChannel = 6;  
        else
            pPort->P2PCfg.ListenChannel = P2P_DEFAULT_LISTEN_CHANNEL;

        pPort->Channel = pPort->P2PCfg.ListenChannel;
        pPort->CentralChannel = pPort->P2PCfg.ListenChannel;
        pPort->P2PCfg.GroupChannel = pPort->P2PCfg.ListenChannel;
        pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.ListenChannel;

        // We want use 20MHz for the same Listen Channel. So set BW_20.
        pPort->BBPCurrentBW = BW_20;
        DBGPRINT(RT_DEBUG_TRACE, ("%s - ListenChannel = %d. GroupOpChannel = %d !!!\n",__FUNCTION__, pPort->P2PCfg.ListenChannel, pPort->P2PCfg.GroupOpChannel));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("%s - Now becomes CentralChannel = %d. Channel  = %d. !!! \n", __FUNCTION__, pPort->CentralChannel,pPort->Channel));

}

/*  
    ==========================================================================
    Description: 
        Get common set channel list from GoN Req/Rsp Inv Req and apply common set channel list in GoN Rsp/Cfm and Inv Rsp
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pMsSaveCommonSetChannelList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen)
{
    PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)Msg;
    UCHAR               PeerChannelList[MAX_NUM_OF_CHANNELS];
    UCHAR               PeerChannelIndex = 0;
    UCHAR               ChannelIndex = 0;

    // Reset CommcomSetChannelList
    PlatformZeroMemory(&pPort->P2PCfg.CommcomSetChannelList, sizeof(P2P_CHANNEL_ENTRY_LIST));
    
    // Get channel list from GoN Req/Rsp and Inv Req
    PlatformZeroMemory(PeerChannelList, MAX_NUM_OF_CHANNELS);
    P2pParseSubElmt(pAd, &pFrame->ElementID, (MsgLen - LENGTH_802_11 - 8),
                      FALSE, NULL, NULL, NULL, NULL, NULL, NULL, 
                      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, PeerChannelList, NULL, NULL, NULL);

    // Find the common channel list information 
    while((PeerChannelList[PeerChannelIndex] != 0) && (PeerChannelIndex < MAX_NUM_OF_CHANNELS))
    {
        for (ChannelIndex = 0; ChannelIndex < pAd->HwCfg.ChannelListNum; ChannelIndex++)
        {
            if (pAd->HwCfg.ChannelList[ChannelIndex].Channel == PeerChannelList[PeerChannelIndex])
            {
                pPort->P2PCfg.CommcomSetChannelList.Channel[pPort->P2PCfg.CommcomSetChannelList.ChannelNr] = PeerChannelList[PeerChannelIndex];
                pPort->P2PCfg.CommcomSetChannelList.ChannelNr++;
                break;
            }
        }
        PeerChannelIndex++;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - ChannelListNum  = %d.\n", __FUNCTION__, pPort->P2PCfg.CommcomSetChannelList.ChannelNr));
}

/*  
    ==========================================================================
    Description: 
        Fill the op channel scan parameters for P2pMs
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pMsScanOpChannelDefault(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    UCHAR   i = 0;
    UCHAR   ScanTimes = OP_CHANNEL_SCAN_TIMES;

    if (pPort->P2PCfg.GoNOpChannel.ChannelNumber == 0)
    {
        return;
    }
    // TODO: 5, 12, 20?
    // for quck p2p connectopn
    if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
        ScanTimes = 11;

    for (i = 0; i < ScanTimes; i++)
    {
        pPort->P2PCfg.P2pProprietary.ListenChanel[i] = pPort->P2PCfg.GoNOpChannel.ChannelNumber;
    }
    
    pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
    pPort->P2PCfg.P2pProprietary.ListenChanelCount = ScanTimes;

    pPort->ScaningChannel = pPort->P2PCfg.GoNOpChannel.ChannelNumber;

    DBGPRINT(RT_DEBUG_TRACE, ("%s - OpChannel(%d), ScanTimes(%d)\n", __FUNCTION__, pPort->P2PCfg.GoNOpChannel.ChannelNumber, ScanTimes));
}

#ifdef MULTI_CHANNEL_SUPPORT
/*  
    ==========================================================================
    Description: 
        This function will send out PWR=1 during P2pMs session when STA is connected.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pMsStartActionFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    
    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_P2pMs_START_NEW_SESSION, NULL, 0);
    //P2pMsStartActionFrameExec(pAd, pPort);
}

/*  
    ==========================================================================
    Description: 
        This function will send out PWR=1 at the start of new session.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pMsStartActionFrameExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    BOOLEAN     TimerCancelled = FALSE;
    UINT32      Value1 = 0;
    UINT32      Value2 = 0;
    UINT32      Count = 0;
    UCHAR       HwQSEL = pPort->P2PCfg.STAHwQSEL;
    // P2P session = 13sec, STA session = 5sec
    UINT32      RestoreChTimeout = RSETORE_CH_TIMEOUT;
    PMP_PORT    pInfraPort = pAd->pP2pCtrll->pInfraPort;

    DBGPRINT(RT_DEBUG_TRACE, ("%s - flag(%d), Infra(%d), P2PCLI(%d)\n",
                                __FUNCTION__,
                                MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION),
                                INFRA_ON(pInfraPort),
                                MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP)));   

    MultiChannelSwitchStop(pAd);
    
    // Send PWR save to AP/GO at the start of new session
    if ((MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) == FALSE) &&
        (INFRA_ON(pInfraPort) || 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) ||
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - New session start ==> \n", __FUNCTION__));

        // Set flag
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION);
        
        // Suspend MSDU transmission here
        //RTMPSuspendMsduTransmission(pAd);
        MultiChannelSuspendMsduTransmission(pAd);

        // Display current h/w queue status
        RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &Value1);
        RTUSBReadMACRegister(pAd, PBF_DBG, &Value2);
        DBGPRINT(RT_DEBUG_TRACE, ("TXRXQ_PCNT_1_(R=%d, EDCA=%d, HCCA=%d), PBF_DBG(%d)\n", 
                                        (Value1 >> 24) & 0xff,
                                        (Value1 >> 16) & 0xff,
                                        (Value1 >> 8) & 0xff,
                                        Value2 & 0xff));
    
        // Polling EDCA queue empty
        while (Count < 500)
        {
            RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &Value1);
            if (((Value1 >> 16) & 0x1f) == 0)   
            {
                break;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("[%d] Polling EDCA(%d)\n", Count, ((Value1 >> 16) & 0x1f)));

            // 1mse
            NdisCommonGenericDelay(100);

            Count++;
        }

        // Reset all the timers
        PlatformCancelTimer(&pPort->Mlme.BeaconTimer, &TimerCancelled);
        PlatformCancelTimer(&pPort->Mlme.ScanTimer, &TimerCancelled);

        // No this part in USB
        // Disallow go to sleep before scan.
        //RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);

        // No this part in USB
        // BBP and RF are not accessible in PS mode, we has to wake them up first
        //if (RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND) || MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        //  AsicForceWakeup(pAd, FROM_TX);

        if (INFRA_ON(pInfraPort) || 
            MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
        {
            if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
            {
                // TODO: 2012-04-24 Might need to consider 2nd CLI
                pPort = pAd->PortList[pPort->P2PCfg.ClientPortNumber];
                HwQSEL = pPort->P2PCfg.CLIHwQSEL;
                RestoreChTimeout -= 10000;
            }

            DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Null frame from Port(%d)\n", __FUNCTION__, pPort->PortNumber));    

            // Send out NULL frame
            XmitSendNullFrameForChSwitch(pAd, 
                                pPort,
                                pPort->CommonCfg.TxRate,
                                FALSE,
                                PWR_SAVE);
            //MultiChannelWaitTxRingEmpty(pAd, MultiChannelGetSwQ(HwQSEL));
            MultiChannelWaitTxRingEmpty(pAd);
            MultiChannelWaitHwQEmpty(pAd, HwQSEL, 0);

            // Delay for 20ms
            NdisCommonGenericDelay(20000);
        }

        // Trigger channel restore timer
        PlatformCancelTimer(&pPort->Mlme.ChannelRestoreTimer, &TimerCancelled);
        PlatformSetTimer(pPort, &pPort->Mlme.ChannelRestoreTimer, RestoreChTimeout);    

        DBGPRINT(RT_DEBUG_TRACE, ("%s - New session End (RestoreChTimeout=%d) <== \n", __FUNCTION__, RestoreChTimeout));
    }   
}

/*  
    ==========================================================================
    Description: 
        Restore channel to Infra's channel after P2pMs session. Only failure case triggers restore channel or it will be handled by multi-cahnnel
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */

VOID P2pMsChannelRestoreTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_P2pMs_CH_RESTORE, NULL, 0);
    //P2pMsChannelRestoreExec(pAd);
}

VOID P2pMsChannelRestoreExec(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT      pPort = pAd->PortList[PORT_0];
    UCHAR           RestoreChannel = pPort->P2PCfg.InfraChannel;

    DBGPRINT(RT_DEBUG_TRACE, ("%s - Restore to INFRA's channel ==> \n", __FUNCTION__));

    if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
    {
        pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
        RestoreChannel = pPort->P2PCfg.P2PChannel;        
    }

    // Restore channel
    P2pMsRestoreChannel(pAd, pPort,  RestoreChannel);

    // Send out NULL frame to inform AP/GO PWR=0 state
    XmitSendNullFrameForChSwitch(pAd, 
                        pPort,
                        pPort->CommonCfg.TxRate,
                        FALSE,
                        PWR_ACTIVE);
    //MultiChannelWaitTxRingEmpty(pAd, MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL));
    MultiChannelWaitTxRingEmpty(pAd);
    MultiChannelWaitHwQEmpty(pAd, pPort->P2PCfg.STAHwQSEL, 0);

    // Resume MSDU TX
    //RTMPResumeMsduTransmission(pAd, pPort);
    MultiChannelResumeMsduTransmission(pAd);

    // Reset flag
    MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION);
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - Restore to INFRA's channel <== \n", __FUNCTION__)); 
}
#endif /*MULTI_CHANNEL_SUPPORT*/

BOOLEAN
P2PMSCheckAnyWFDLinkUp(
    IN PMP_ADAPTER  pAd
)
{
    UCHAR   i = 0;
    PMP_PORT    pPort;
    BOOLEAN     AnyPortLinkUp = FALSE;

    for(i = PORT_1; ((pAd->PortList[i] != NULL) && (i < RTMP_MAX_NUMBER_OF_PORT)); i++)
    {
        pPort = pAd->PortList[i];
        if (INFRA_ON(pPort))
        {
            AnyPortLinkUp = TRUE;
            return AnyPortLinkUp;
        }
    }

    return AnyPortLinkUp;
}

VOID P2pMsScanDone(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    PMP_PORT  pActivePort;
    UCHAR       BBPValue;
    USHORT      Status;
    ULONGLONG       Now64;
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.bNLOAfterResume == TRUE))
    {
        pAd->StaCfg.NLOEntry.bNLOAfterResume = FALSE;
    }
#endif
    //change back to active port
    pActivePort = MlmeSyncGetActivePort(pAd);

    if ((IS_P2P_GROUP_FORMING(pPort) || IS_P2P_INVITING(pPort)) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype != PORTSUBTYPE_P2PGO))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, keep the same Channel. So don't need make a channel switch again. PortSubtype = %d\n", pPort->PortSubtype));
    }
    else if (pPort->BBPCurrentBW == BW_40 && (INFRA_ON(pActivePort) || pPort->SoftAP.bAPStart || pPort->P2PCfg.bGOStart || ADHOC_ON(pActivePort)))
    {
        RTUSBReadBBPRegister(pAd, BBP_R4, &BBPValue);
        BBPValue &= (~0x18);            
        BBPValue |= 0x10;
        RTUSBWriteBBPRegister(pAd, BBP_R4, BBPValue);
        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 40MHz channel %d, Total BSS[%02d]\n",pPort->CentralChannel, pAd->ScanTab.BssNr));
    }
    else if ((!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))&&(pPort->BBPCurrentBW == BW_40))
    {
            // if not linkup; then fallback to BW20
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntlChannelWidth: SYNC - End of SCAN: If not linkup, then fallback to BW20. restore to 20MHz channel %d\n",pPort->Channel));     
        MlmeCntlChannelWidth(pAd, pPort->Channel, pPort->Channel, BW_20,EXTCHA_NONE);
    }
    else
    {   
        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 20MHz channel %d, Total BSS[%02d]\n",pPort->Channel, pAd->ScanTab.BssNr));
    }

    pAd->CountDowntoRadioOff = STAY_IN_AWAKE;
    
    // increase concurrent SoftAp or GO beacon quantity.
    if ((pPort->SoftAP.bAPStart) && (pAd->PortList[pPort->SoftAP.ApCfg.ApPortNum]) && (pAd->PortList[pPort->SoftAP.ApCfg.ApPortNum]->PortSubtype == PORTSUBTYPE_VwifiAP))
    {
        MgntPktSendBeacon(pAd->PortList[pPort->SoftAP.ApCfg.ApPortNum]);
    }
            
    //
    // To prevent data lost.
    // Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
    // Now, we need to send an NULL data with turned PSM bit off to AP, when scan progress done 
    //
    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pPort)))
    {
        AsicSendNullFrame(pAd, pPort,pPort->Channel, (pAd->StaCfg.Psm == PWR_SAVE));
    }           

    //resume beacon after scan
    if ((pPort->SoftAP.bAPStart) || (pPort->P2PCfg.bGOStart))
    {
        AsicResumeBssSync(pAd);
    }

    if ((pPort->PortCfg.AdditionalRequestIEData != NULL) && (pPort->PortCfg.AdditionalRequestIESize > 0))
    {
        DBGPRINT(RT_DEBUG_TRACE,("Clear AdditionalIEData buffer!!\n"));
        PlatformFreeMemory(pPort->PortCfg.AdditionalRequestIEData, pPort->PortCfg.AdditionalRequestIESize);
        pPort->PortCfg.AdditionalRequestIEData = NULL;
        pPort->PortCfg.AdditionalRequestIESize = 0;
    }
    
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

    DBGPRINT(RT_DEBUG_TRACE, ("jesse Scan Complete port 2 rece beacon %d %d %d", INFRA_ON(pAd->PortList[pPort->P2PCfg.PortNumber]), (IS_P2P_CON_CLI(pAd, pAd->PortList[PORT_2])), pAd->PortList[pPort->P2PCfg.PortNumber]->bActive));
        
    if ((IS_P2P_CON_CLI(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) && (INFRA_ON(pAd->PortList[pPort->P2PCfg.PortNumber])))
    {
        PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pAd->PortList[pPort->P2PCfg.PortNumber], ROLE_WLANIDX_P2P_CLIENT);
        if(pEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  \n", __FUNCTION__, __LINE__));
            return;
        }
        
        pEntry->LastBeaconRxTime = Now64;
        DBGPRINT(RT_DEBUG_TRACE, ("jesse Scan Complete port 2 rece beacon"));
        
    }
    if(INFRA_ON(pAd->PortList[PORT_0]))
    {
        pAd->StaCfg.LastBeaconRxTime = Now64;
        DBGPRINT(RT_DEBUG_TRACE, ("jesse Scan Complete port 0 rece beacon"));
    }
    
    pAd->MlmeAux.ScanForConnectCnt++;
    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;      
    Status = MLME_SUCCESS;

    PlatformIndicateScanStatus(pAd, pPort, NDIS_STATUS_SUCCESS, TRUE, FALSE);

    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
    MlmeHandler(pAd);
}
#else
VOID DummyFunction(
    IN PMP_ADAPTER pAd)
{
    
}
#endif
