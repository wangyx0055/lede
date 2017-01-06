/*
    ***************************************************************************
    * Ralink Technology, Corp.
    * 5F., No. 5, Tai-Yuan 1st St., Jhubei City, Hsinchu County 30265, Taiwan
    *
    * (c) Copyright 2002-2011, Ralink Technology, Corp.
    *
    * All rights reserved. Ralink's source code is an unpublished work and the use of a copyright notice 
    * does not imply otherwise. This source code contains confidential trade secret material of 
    * Ralink Technology, Corp. Any attempt or participation in deciphering, decoding, reverse engineering 
    * or in any way altering the source code is stricitly prohibited, unless the prior written consent of 
    * Ralink Technology, Corp. is obtained.
    ***************************************************************************

    Module Name: 
    PktCmdRspEvt.c

    Abstract: 
    Andes packet commands and response events

    Revision History:
    Who                         When            What
    -------------------     ----------      ----------------------------------------------
    Ian Tang                        2011/11/7       Start implementation
*/

#include "MtConfig.h"

//
// Transmit the packet command
//
VOID TxPktCmd(
    IN PMP_ADAPTER pAd, 
    IN UCHAR PktCmdType, // Packet command type (PKT_CMD_TYPE_XXX)
    IN UCHAR PktCmdSeq, // Packet command sequence (NO_PKT_CMD_RSP_EVENT or 1~15 command sequence)
    IN PVOID pPktCmdParameters, // Packet command parameters
    IN USHORT PktCmdParametersLength, // Length of packet command
    IN UINT MicrosecondsToStall) // The number of microseconds to delay. A driver should specify no more than 50 microseconds.
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    PUCHAR pOutBuffer = NULL;
    PUCHAR pPayload = NULL;

    if (!VALID_PKT_CMD_TYPE(PktCmdType))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect packet command type (PktCmdType = %d)\n", 
            __FUNCTION__, 
            PktCmdType));
        return;
    }

    if (!VALID_PKT_CMD_SEQ(PktCmdSeq))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect packet command sequence (PktCmdSeq = %d)\n", 
            __FUNCTION__, 
            PktCmdSeq));
        return;
    }

    if (((pPktCmdParameters == NULL) && (PktCmdParametersLength != 0)) || 
        ((pPktCmdParameters != NULL) && (PktCmdParametersLength == 0)))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect combination of pointer and length (PktCmdParametersLength = %d)\n", 
            __FUNCTION__, 
            PktCmdParametersLength));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));


    NdisStatus =PlatformAllocateMemory(pAd,  &pOutBuffer, PktCmdParametersLength);
    
    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        PlatformZeroMemory(pOutBuffer, PktCmdParametersLength);

        //
        // Fill parameters if any
        //
        if (PktCmdParametersLength != 0)
        {
            pPayload = (PUCHAR)(pOutBuffer);
            
            PlatformMoveMemory(pPayload, pPktCmdParameters, PktCmdParametersLength);
        }       

        TxPktCmdRst(pAd, QUEUE_IDX_PKT_CMD, pOutBuffer, PktCmdType, PktCmdSeq, PktCmdParametersLength);     

        PlatformFreeMemory(pOutBuffer, PktCmdParametersLength);

        if (MicrosecondsToStall <= 50)
        {
            NdisCommonGenericDelay(MicrosecondsToStall); // Stall the caller on the current processor for a given interval

            DBGPRINT(RT_DEBUG_TRACE, ("%s: MicrosecondsToStall = %d microseconds\n", 
                __FUNCTION__, 
                MicrosecondsToStall));

            //2 TODO: Print the information of the Andes response event
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Memory allocation failure, PktCmdType = %d, PktCmdSeq = %d, PktCmdParametersLength = %d\n", 
            __FUNCTION__, 
            PktCmdType, 
            PktCmdSeq, 
            PktCmdParametersLength));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

/*
//
// Request the Tx packet command
//
VOID TxPktCmdRst(
    IN PMP_ADAPTER pAd, 
    IN UCHAR QueueIdx, // Queue index
    IN PUCHAR pPktCmd, // Point to a packet command
    IN UCHAR PktCmdType, // Packet command type (PKT_CMD_TYPE_XXX)
    IN UCHAR PktCmdSeq, // Packet command sequence (NO_PKT_CMD_RSP_EVENT or 1~15 command sequence)
    IN USHORT PktCmdParametersLength) // Length of packet command
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pNullContext;
    PTXINFO_STRUC   pTxInfo;

    
    if (!VALID_QUEUE_IDX(QueueIdx))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid queue index (QueueIdx = %d)\n", 
            __FUNCTION__, 
            QueueIdx));
        return;
    }

    if (pPktCmd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NULL pointer (pPktCmd)\n", __FUNCTION__));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    pNullContext = &(pAd->pHifCfg->NullContext);
    if (pNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pNullContext->InUse = TRUE;

        // clean the buffer
        PlatformZeroMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], 100);

        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0];

        //
        // Fill Tx FCE info
        //
        WRITE_TX_INFO_INFO_TYPE(pAd, pTxInfo, 1);
        WRITE_TX_INFO_DPORT(pAd, pTxInfo, PORT_TYPE_CPU_TX_PORT);
        WRITE_TX_INFO_PKT_CMD_TYPE(pAd, pTxInfo, PktCmdType);
        WRITE_TX_INFO_PKT_CMD_SEQ(pAd, pTxInfo, PktCmdSeq);
        WRITE_TX_INFO_PKT_CMD_PARAMETERS_LENGTH(pAd, pTxInfo, PktCmdParametersLength);

        DBGPRINT(RT_DEBUG_TRACE, ("%s: TXFCE Info = 0x%08X\n", 
            __FUNCTION__, 
            (pTxInfo->word)));

        //
        //
        //
        PlatformMoveMemory( &pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[TXINFO_SIZE],
                        pPktCmd,
                        PktCmdParametersLength);

        // set the total length ig this command
        pAd->pHifCfg->NullContext.BulkOutSize = PktCmdParametersLength+TXINFO_SIZE;

        //DbgPrint("pAd->pHifCfg->NullContext.BulkOutSize = %d\n",pAd->pHifCfg->NullContext.BulkOutSize);
        
        //DumpFrame(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], pAd->pHifCfg->NullContext.BulkOutSize);

        MTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);
        
        // Kick bulk out 
        N6USBKickBulkOut(pAd);
        }
        else
        {
        DBGPRINT(RT_DEBUG_ERROR,("Send Command packet failed !!!\n"));
        }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}
*/
//
// Request the Tx packet command
//
VOID TxPktCmdRst(
    IN PMP_ADAPTER pAd, 
    IN UCHAR QueueIdx, // Queue index
    IN PUCHAR pPktCmd, // Point to a packet command
    IN UCHAR PktCmdType, // Packet command type (PKT_CMD_TYPE_XXX)
    IN UCHAR PktCmdSeq, // Packet command sequence (NO_PKT_CMD_RSP_EVENT or 1~15 command sequence)
    IN USHORT PktCmdParametersLength) // Length of packet command
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext;
    PTXINFO_STRUC   pTxInfo;


    if (!VALID_QUEUE_IDX(QueueIdx))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid queue index (QueueIdx = %d)\n", 
            __FUNCTION__, 
            QueueIdx));
        return;
    }

    if (pPktCmd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NULL pointer (pPktCmd)\n", __FUNCTION__));
        return;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));

    pPktCmdContext = &(pAd->pHifCfg->PktCmdContext);
    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        // clean the buffer
        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], 100);

        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0];

        //
        // Fill Tx FCE info
        //
        WRITE_TX_INFO_INFO_TYPE(pAd, pTxInfo, 1);
        WRITE_TX_INFO_DPORT(pAd, pTxInfo, PORT_TYPE_CPU_TX_PORT);
        WRITE_TX_INFO_PKT_CMD_TYPE(pAd, pTxInfo, PktCmdType);
        WRITE_TX_INFO_PKT_CMD_SEQ(pAd, pTxInfo, PktCmdSeq);
        WRITE_TX_INFO_PKT_CMD_PARAMETERS_LENGTH(pAd, pTxInfo, PktCmdParametersLength);

        DBGPRINT(RT_DEBUG_INFO, ("%s: TXFCE Info = 0x%08X\n", 
            __FUNCTION__, 
            (pTxInfo->word)));
        
        //
        //
        //
        PlatformMoveMemory( &pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[TXINFO_SIZE],
                        pPktCmd,
                        PktCmdParametersLength);

        // set the total length ig this command
        pAd->pHifCfg->PktCmdContext.BulkOutSize = PktCmdParametersLength+TXINFO_SIZE;

        //DbgPrint("pAd->pHifCfg->PktCmdContext.BulkOutSize = %d\n",pAd->pHifCfg->PktCmdContext.BulkOutSize);
        
        //DumpFrame(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], pAd->pHifCfg->PktCmdContext.BulkOutSize);

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        DBGPRINT(RT_DEBUG_ERROR,("%s:Send Command packet failed !!!\n", __FUNCTION__));
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Burst write (single or multiple registers)
//
VOID BurstWrite(
    IN PMP_ADAPTER pAd, 
    IN ULONG StartAddress, // Start address
    IN ULONG RegValue, // The register value
    IN ULONG NumOfRegValues) // Number of the register values to be written
        {
    BURST_WRITE_PKT_CMD_PARAMETERS BurstWritePktCmdParameters = {0};
    USHORT CmdParametersLength = 0;
    ULONG idx = 0;
        
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    BurstWritePktCmdParameters.StartAddress = StartAddress;
    BurstWritePktCmdParameters.RegValue = RegValue;

    CmdParametersLength = sizeof(BurstWritePktCmdParameters);
            
    DBGPRINT(RT_DEBUG_TRACE, ("%s: StartAddress = 0x%X, RegValue = 0x%X, CmdParametersLength = %d\n", 
        __FUNCTION__, 
        StartAddress, 
        RegValue, 
        CmdParametersLength));
    
    TxPktCmd(pAd, 
            PKT_CMD_TYPE_BURST_WRITE, 
            NO_PKT_CMD_RSP_EVENT, 
            &BurstWritePktCmdParameters, 
            CmdParametersLength,
            0);

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Burst read (single or multiple registers)
//
VOID BurstRead(
    IN PMP_ADAPTER pAd, 
    IN ULONG StartAddress, // Start address
    IN ULONG NumOfRegValues) // Number of the register values to be read
{
    BURST_READ_PKT_CMD_PARAMETERS BurstReadPktCmdParameters = {0};
    USHORT CmdParametersLength = 0;
    ULONG idx = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    BurstReadPktCmdParameters.StartAddress = StartAddress;
    BurstReadPktCmdParameters.NumOfRegValues = NumOfRegValues;

    CmdParametersLength = sizeof(BurstReadPktCmdParameters);

    DBGPRINT(RT_DEBUG_TRACE, ("%s: StartAddress = 0x%X, NumOfRegValues = %d, CmdParametersLength = %d\n", 
            __FUNCTION__, 
        StartAddress, 
        NumOfRegValues, 
        CmdParametersLength));

    TxPktCmd(pAd, PKT_CMD_TYPE_BURST_READ, NO_PKT_CMD_RSP_EVENT, 
        &BurstReadPktCmdParameters, CmdParametersLength, 0);

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendEFuseBufferModeCMD(
    IN PMP_ADAPTER    pAd,
    IN UINT8                EepromType)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    CMD_TXDSCR          CmdTxD;
    EXT_CMD_EFUSE_BUFFER_MODE_T ExtCmdEfuseBufferMode;
    LARGE_INTEGER       TimeOut1Second;

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock); 
        
        TimeOut1Second.QuadPart = -(500* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(EXT_CMD_EFUSE_BUFFER_MODE_T);   
        CmdTxD.SetQuery = 1; // 0 : Query   1 : Set
        CmdTxD.ExtendCID = 0x21;
        CmdTxD.ucExtCmdOption = 1; // need response from FW.

        // Filled RF Reg Access CMD setting
        ExtCmdEfuseBufferMode.ucSourceMode = EepromType;
        if (EepromType == EEPROM_MODE_EFUSE)
        {
                ExtCmdEfuseBufferMode.ucCount = 0;
        }
        else
        {
                // Bufffer Mode is used for embedded system.
        }
        
        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy RF Test - RFIQ
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(EXT_CMD_EFUSE_BUFFER_MODE_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &ExtCmdEfuseBufferMode, sizeof(EXT_CMD_EFUSE_BUFFER_MODE_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);

        do
        {
            DBGPRINT(RT_DEBUG_TRACE,("### SendEFuseBufferModeCMD - wait to get bulk in ###\n"));


            PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

            DBGPRINT(RT_DEBUG_TRACE,("### SendEFuseBufferModeCMD - got CMD response ###\n"));
        
        }while (FALSE);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendRFRegAccessCMD(
    IN PMP_ADAPTER    pAd,
    IN UINT32           WifiStream,
    IN UINT32           Address) 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    CMD_TXDSCR          CmdTxD;
    CMD_RF_REG_ACCESS_T   ExtRFRegAccessCMD;
    LARGE_INTEGER       TimeOut1Second;

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock); 

        TimeOut1Second.QuadPart = -(500* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(CMD_RF_REG_ACCESS_T); 
        CmdTxD.SetQuery = 1; // 0 : Query   1 : Set
        CmdTxD.ExtendCID = 0x4;
        CmdTxD.ucExtCmdOption = 1; // need response from FW.

        // Filled RF Reg Access CMD setting
        ExtRFRegAccessCMD.WiFiStream = WifiStream;
        ExtRFRegAccessCMD.Address = Address;
        //ExtRFRegAccessCMD.Data = 0;
        
        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);

        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy RF Test - RFIQ
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(CMD_RF_REG_ACCESS_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &ExtRFRegAccessCMD, sizeof(CMD_RF_REG_ACCESS_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);

        do
        {
            DBGPRINT(RT_DEBUG_TRACE,("### SendRFRegAccessCMD - wait to get bulk in ###\n"));


            PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

            DBGPRINT(RT_DEBUG_TRACE,("### SendRFRegAccessCMD - got CMD response ###\n"));
        
        }while (FALSE);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);   
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendRFTestCMD(
    IN PMP_ADAPTER    pAd,
    IN UINT32               Action,
    IN UINT32               Mode,
    IN UINT32               CalibId) 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    CMD_TXDSCR          CmdTxD;
    CMD_RF_TEST_CTRL_T  RFTestCMD;
    LARGE_INTEGER       TimeOut1Second;

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;    
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);    
        
        TimeOut1Second.QuadPart = -(500* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(CMD_RF_TEST_CTRL_T);    
        CmdTxD.SetQuery = 1; // 0 : Query   1 : Set
        CmdTxD.ExtendCID = 0x4;
        CmdTxD.ucExtCmdOption = 1; // need response from FW.

        // Filled RF Test CMD setting
        RFTestCMD.Action = (UINT8)Action;
        RFTestCMD.u.OpMode = Mode;
        
         if (Action == ACTION_IN_RFTEST)
         {
                RFTestCMD.u.RfATInfo.u4FuncIndex = 1;
                RFTestCMD.u.RfATInfo.u4FuncData = CalibId;
        }

        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy RF Test - RFIQ
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(CMD_RF_TEST_CTRL_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &RFTestCMD, sizeof(CMD_RF_TEST_CTRL_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);

        do
        {
            DBGPRINT(RT_DEBUG_TRACE,("### SendRFTestCMD - wait to get bulk in ###\n"));


            PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

            DBGPRINT(RT_DEBUG_TRACE,("### SendRFTestCMD - got CMD response ###\n"));
        
        }while (FALSE);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock); 
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendPowerManagementCMD(
    IN PMP_ADAPTER    pAd, 
    IN UINT8                PmNumber,
    IN UINT8                PmState,
    IN UINT8                *Bssid,
    IN UINT8                DtimPeriod,
    IN UINT8                BeaconInterval) 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    CMD_TXDSCR          CmdTxD;
    EXT_CMD_PM_STATE_CTRL_T PmCMD;
    LARGE_INTEGER       TimeOut1Second;

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        TimeOut1Second.QuadPart = -(500* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(EXT_CMD_PM_STATE_CTRL_T);   
        CmdTxD.SetQuery = 1;
        CmdTxD.ExtendCID = 0x7;
        CmdTxD.ucExtCmdOption = 1; // need response from FW.

        // Filled Power Management CMD setting
        PmCMD.ucPmNumber = PmNumber;
        PmCMD.ucPmState = PmState;
        PlatformMoveMemory(PmCMD.aucBssid, Bssid, 6);
        DBGPRINT(RT_DEBUG_TRACE,("### SendPowerManagementCMD - %02x %02x %02x %02x %02x %02x ###\n", PmCMD.aucBssid[0], PmCMD.aucBssid[1], PmCMD.aucBssid[2], PmCMD.aucBssid[3], PmCMD.aucBssid[4], PmCMD.aucBssid[5]));
        PmCMD.ucDtimPeriod = DtimPeriod;
        PmCMD.u2BcnInterval = BeaconInterval;
        if (PmNumber == PM4 && PmState == PWR_PmOn)
        {
            PmCMD.u4Aid = 1;
            PmCMD.u4RxFilter = 0xFEF0B;
        }
        else if (PmNumber == PM4 && PmState == PWR_PmOff)
        {
            PmCMD.u4Aid = 1;
            PmCMD.u4RxFilter = 0x9E70B;
        }

        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy PmState
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(EXT_CMD_PM_STATE_CTRL_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &PmCMD, sizeof(EXT_CMD_PM_STATE_CTRL_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);

        do
        {
            DBGPRINT(RT_DEBUG_TRACE,("### SendPowerManagementCMD - wait to get bulk in ###\n"));

            PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

            DBGPRINT(RT_DEBUG_TRACE,("### SendPowerManagementCMD - got CMD response ###\n"));
        
        }while (FALSE);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);  
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendRadioOnOffCMD(
    IN PMP_ADAPTER    pAd, 
    IN UINT8                bRadio) 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    CMD_TXDSCR          CmdTxD;
    EXT_CMD_RADIO_ON_OFF_CTRL_T CmdRadio;
    LARGE_INTEGER       TimeOut1Second;

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        TimeOut1Second.QuadPart = -(1000* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(EXT_CMD_RADIO_ON_OFF_CTRL_T);   
        CmdTxD.SetQuery = 1;
        CmdTxD.ExtendCID = 0x5;
        CmdTxD.ucExtCmdOption = 1; // need response from FW.

        // Filled Radio CMD setting
        CmdRadio.ucWiFiRadioCtrl = bRadio;

        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy Radio
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(EXT_CMD_RADIO_ON_OFF_CTRL_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &CmdRadio, sizeof(EXT_CMD_RADIO_ON_OFF_CTRL_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);

        do
        {
            DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadRestartDownloadFW - wait to get bulk in ###\n"));


            PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

            DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadRestartDownloadFW - got CMD response ###\n"));
        
        }while (FALSE);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock); 
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendDisableRxCMD(
    IN PMP_ADAPTER    pAd) 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    CMD_TXDSCR          CmdTxD;
    EXT_CMD_WIFI_RX_DISABLE_T   CmdDisableRx;
    LARGE_INTEGER       TimeOut1Second;

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        return;

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);    
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        TimeOut1Second.QuadPart = -(1000* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(EXT_CMD_WIFI_RX_DISABLE_T); 
        CmdTxD.SetQuery = 1;
        CmdTxD.ExtendCID = 0x6;
        CmdTxD.ucExtCmdOption = 1; // need response from FW.

        // Filled Radio CMD setting
        CmdDisableRx.ucWiFiRxDisableCtrl = 1;

        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
      
        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy Radio
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(EXT_CMD_WIFI_RX_DISABLE_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &CmdDisableRx, sizeof(EXT_CMD_WIFI_RX_DISABLE_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);

        do
        {
            DBGPRINT(RT_DEBUG_TRACE,("### SendDisableRxCMD - wait to get bulk in ###\n"));

            PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

            DBGPRINT(RT_DEBUG_TRACE,("### SendDisableRxCMD - got CMD response ###\n"));
        
        }while (FALSE);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock); 
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendSwitchChannelCMD(
    IN PMP_ADAPTER pAd, 
    IN EXT_CMD_CHAN_SWITCH_T    CmdChSwitch) 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    CMD_TXDSCR          CmdTxD;
    //EXT_CMD_CHAN_SWITCH_T CmdChSwitch;
    UCHAR   i = 0;
    LARGE_INTEGER       TimeOut1Second;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        TimeOut1Second.QuadPart = -(1000* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(EXT_CMD_CHAN_SWITCH_T); 
        CmdTxD.SetQuery = 1;
        CmdTxD.ExtendCID = 0x8;
        CmdTxD.ucExtCmdOption = 1;

#if 0
        if (bScan)
        {
            CmdChSwitch.CtrlCH = pPort->ScaningChannel;
            CmdChSwitch.CentralCH = pPort->ScaningChannel;
            CmdChSwitch.BW = BW_20;
        }
        else
        {
        CmdChSwitch.CtrlCH = Channel;
            CmdChSwitch.CentralCH = pPort->CentralChannel;
            CmdChSwitch.BW = pPort->BBPCurrentBW;            
        }
#endif

        CmdChSwitch.ucRxStreamNum = pPort->CommonCfg.RxStream;
        CmdChSwitch.ucTxStreamNum = pPort->CommonCfg.TxStream;

        DBGPRINT(RT_DEBUG_TRACE, ("%s   BBPCurrentBW = %d   CentralChannel %d", __FUNCTION__, pPort->BBPCurrentBW, pPort->CentralChannel));
        
        for(i=0;i<SKU_SIZE;i++)
        {
            CmdChSwitch.aucTxPowerSKU[i]=0xff;
        }

        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy Channel Privilege
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(EXT_CMD_CHAN_SWITCH_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &CmdChSwitch, sizeof(EXT_CMD_CHAN_SWITCH_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);


        do
        {
            DBGPRINT(RT_DEBUG_TRACE,("### SendSwitchChannelCMD - wait to get bulk in ###\n"));


            PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

            DBGPRINT(RT_DEBUG_TRACE,("### SendSwitchChannelCMD - got CMD response ###\n"));
        
        }while (FALSE);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);  
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }
}

VOID SendLEDCmd(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT   pPort,
    IN ULONG    LEDNumber,
    IN ULONG    LEDBehavior)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT         pPktCmdContext = &(pAd->pHifCfg->PktCmdContext);
    CMD_TXDSCR          CmdTxD;
    EXT_CMD_ID_LED_T    ExtLedCMD;
    LARGE_INTEGER       TimeOut1Second;

    if(pPort->StaCfg.WscControl.bSkipWPSLEDCMD) //Skip LED CMD since some LED CMD wont be bother.
    {
        return;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("%s: --> LEDNumber: %x, LEDBehavior: %d\n", __FUNCTION__, LEDNumber, LEDBehavior));

    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    if (pPktCmdContext->InUse == FALSE)
    {
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        TimeOut1Second.QuadPart = -(500* 10000);

        // Fill TxD setting
        PlatformZeroMemory(&CmdTxD, sizeof(CMD_TXDSCR));
        CmdTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
        CmdTxD.CID = 0xED; // The ID of CMD w/ target address/length request
        CmdTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        CmdTxD.Length = sizeof(CMD_TXDSCR) + sizeof(EXT_CMD_ID_LED_T);  
        CmdTxD.SetQuery = 1; // 0 : Query   1 : Set
        CmdTxD.ExtendCID = 0x17;
        CmdTxD.ucExtCmdOption = 0; // LED mode no need response from FW.

        // Filled RF Reg Access CMD setting
        ExtLedCMD.u4LedNo = LEDNumber;
        ExtLedCMD.u4LedCtrl = LEDBehavior;
        
        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // copy CMD TxD
        pAd->pHifCfg->PktCmdContext.BulkOutSize = sizeof(CMD_TXDSCR);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &CmdTxD, sizeof(CMD_TXDSCR));

        // copy RF Test - RFIQ
        pAd->pHifCfg->PktCmdContext.BulkOutSize += sizeof(EXT_CMD_ID_LED_T);
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[sizeof(CMD_TXDSCR)], &ExtLedCMD, sizeof(EXT_CMD_ID_LED_T));

        XmitSendMlmeCmdPkt(pAd, TXPKT_CMD_FRAME);
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);    
        DBGPRINT(RT_DEBUG_ERROR,("%s: Send Command packet failed !!!\n", __FUNCTION__));
    }
}

VOID SendPowerOperationCMDToAndes(
    IN PMP_ADAPTER pAd, 
    IN ULONG PowerOperation,
    IN ULONG PowerLevel ) 
{
    ANDES_POWER_SAVING_PACKET andesPowerSavingPacket ={0};
    USHORT CmdParametersLength = 0;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    andesPowerSavingPacket.PowerOperation = PowerOperation;
    andesPowerSavingPacket.PowerLevel = PowerLevel;
    //add this 2 line becasue FCE bug
    andesPowerSavingPacket.InitialTBTT = 0;
    andesPowerSavingPacket.PreTBTT = 0;
    //7650E1 FCE bug
    //CmdParametersLength = sizeof(andesPowerSavingPacket);
    CmdParametersLength = 128;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: PSM: PowerOperation = 0x%X, PowerLevel = 0x%X, CmdParametersLength = %d\n", 
        __FUNCTION__, 
        PowerOperation, 
        PowerLevel, 
        CmdParametersLength));

    TxPktCmd(pAd, PKT_CMD_TYPE_PWR_SAVE, NO_PKT_CMD_RSP_EVENT, 
        &andesPowerSavingPacket, CmdParametersLength, 0);

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

VOID SendSwitchChannelOperationCMDToAndes(
    IN PMP_ADAPTER pAd, 
    IN SWITCH_CHANNEL_CMD_PARAMETERS SwitchChannelPacket) 
{
    //SWITCH_CHANNEL_CMD_PARAMETERS SwitchChannelPacket ={0};
    USHORT CmdParametersLength =  sizeof(SWITCH_CHANNEL_CMD_PARAMETERS);
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: CmdParametersLength = %d\n", 
        __FUNCTION__,
        CmdParametersLength));

    TxPktCmd(pAd, PKT_CMD_TYPE_SWITCH_CHANNEL, PKT_CMD_RSP_SWITCH_CH,
        &SwitchChannelPacket, CmdParametersLength, 0);

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Rx response event handle
//
VOID RxRspEvtHandle(
    IN PMP_ADAPTER pAd, 
    IN PRX_FCE_INFO_STRUC pRxFceInfo, // Response event
    IN ULONG RxRingIdx) // Rx ring index
    {
    //PUCHAR pRxRspEvtPayload = RX_RSP_EVT_PAYLOAD_PTR(pAd, RxRingIdx);
    PUCHAR pRxRspEvtPayload = (PUCHAR)pRxFceInfo + TXINFO_SIZE;
    PULONG pDW = NULL;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: READ_RX_FCE_INFO_PKT_LENGTH = %d\n",__FUNCTION__,READ_RX_FCE_INFO_PKT_LENGTH(pAd, pRxFceInfo)));
    DBGPRINT(RT_DEBUG_TRACE, ("%s: READ_RX_FCE_INFO_SELF_GEN_RSP_EVT = %d\n",__FUNCTION__,READ_RX_FCE_INFO_SELF_GEN_RSP_EVT(pAd, pRxFceInfo)));
    DBGPRINT(RT_DEBUG_TRACE, ("%s: READ_RX_FCE_INFO_CMD_SEQ = %d\n",__FUNCTION__,READ_RX_FCE_INFO_CMD_SEQ(pAd, pRxFceInfo)));
    DBGPRINT(RT_DEBUG_TRACE, ("%s: READ_RX_FCE_INFO_RSP_EVT_TYPE = %d\n",__FUNCTION__,READ_RX_FCE_INFO_RSP_EVT_TYPE(pAd, pRxFceInfo)));
    DBGPRINT(RT_DEBUG_TRACE, ("%s: READ_RX_FCE_INFO_SPORT = %d\n",__FUNCTION__,READ_RX_FCE_INFO_SPORT(pAd, pRxFceInfo)));
    DBGPRINT(RT_DEBUG_TRACE, ("%s: READ_RX_FCE_INFO_INFO_TYPE = %d\n",__FUNCTION__,READ_RX_FCE_INFO_INFO_TYPE(pAd, pRxFceInfo)));

    if (READ_RX_FCE_INFO_SELF_GEN_RSP_EVT(pAd, pRxFceInfo))
    {
        //2 TODO: Handle self generated response event

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Handle self-generated response event\n", __FUNCTION__));
    }
    else
    {
        switch (READ_RX_FCE_INFO_RSP_EVT_TYPE(pAd, pRxFceInfo))
        {
            case RSP_EVT_TYPE_CMD_DONE: 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s: RSP_EVT_TYPE_CMD_DONE, RspEvtType = %d\n", 
                    __FUNCTION__, 
                    READ_RX_FCE_INFO_RSP_EVT_TYPE(pAd, pRxFceInfo)));

                if(READ_RX_FCE_INFO_CMD_SEQ(pAd, pRxFceInfo) == PKT_CMD_RSP_SWITCH_CH) //since no CMD_ID in the Rx FCE Info, sequence number 1 means response from switch channel command.
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: SET Freq Offset,  RFValue= 0x%x, PreXoCode = 0x%x\n", __FUNCTION__, pRxRspEvtPayload[0], pRxRspEvtPayload[4]));
                    
                    pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset = (CHAR)pRxRspEvtPayload[8]; // Keep modified xo_cxo value
                }
        
                pDW = (PULONG)(&pRxRspEvtPayload[0]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[4]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[8]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[12]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[16]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[20]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[24]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[28]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));
                pDW = (PULONG)(&pRxRspEvtPayload[32]);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x%X\n", 
                    __FUNCTION__, 
                    *pDW));

                DBGPRINT(RT_DEBUG_TRACE, ("%s: %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X\n", 
                    __FUNCTION__, 
                    pRxRspEvtPayload[0], 
                    pRxRspEvtPayload[1], 
                    pRxRspEvtPayload[2], 
                    pRxRspEvtPayload[3], 
                    pRxRspEvtPayload[4], 
                    pRxRspEvtPayload[5], 
                    pRxRspEvtPayload[6], 
                    pRxRspEvtPayload[7], 
                    pRxRspEvtPayload[8], 
                    pRxRspEvtPayload[9], 
                    pRxRspEvtPayload[10], 
                    pRxRspEvtPayload[11], 
                    pRxRspEvtPayload[12], 
                    pRxRspEvtPayload[13], 
                    pRxRspEvtPayload[14], 
                    pRxRspEvtPayload[15]));
                DBGPRINT(RT_DEBUG_TRACE, ("%s: %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X\n", 
                    __FUNCTION__, 
                    pRxRspEvtPayload[16], 
                    pRxRspEvtPayload[17], 
                    pRxRspEvtPayload[18], 
                    pRxRspEvtPayload[19], 
                    pRxRspEvtPayload[20], 
                    pRxRspEvtPayload[21], 
                    pRxRspEvtPayload[22], 
                    pRxRspEvtPayload[23], 
                    pRxRspEvtPayload[24], 
                    pRxRspEvtPayload[25], 
                    pRxRspEvtPayload[26], 
                    pRxRspEvtPayload[27], 
                    pRxRspEvtPayload[28], 
                    pRxRspEvtPayload[29], 
                    pRxRspEvtPayload[30], 
                    pRxRspEvtPayload[31]));
    }
            break;

            case RSP_EVT_TYPE_CMD_ERROR: 
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s: RSP_EVT_TYPE_CMD_ERROR, RspEvtType = %d\n", 
                    __FUNCTION__, 
                    READ_RX_FCE_INFO_RSP_EVT_TYPE(pAd, pRxFceInfo)));

                //2 TODO: Handle error
            }
            break;

            case RSP_EVT_TYPE_CMD_RETRY: 
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s: RSP_EVT_TYPE_CMD_RETRY, RspEvtType = %d\n", 
                    __FUNCTION__, 
                    READ_RX_FCE_INFO_RSP_EVT_TYPE(pAd, pRxFceInfo)));

                //2 TODO: Handle retry
            }
            break;

            default: 
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknown response event type (%d)\n", 
                    __FUNCTION__, 
                    READ_RX_FCE_INFO_RSP_EVT_TYPE(pAd, pRxFceInfo)));
            }
            break;
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//////////////////////////////////// End of File ////////////////////////////////////

