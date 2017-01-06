/*
 ***************************************************************************
 * MediaTek Inc.
 * 5F, No.5, Tai-Yuan 1st St., 
 * Chupei City, Hsinchu County 30265, Taiwan, R.O.C.
 *
 * (c) Copyright 2014-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmpFw.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
 #include "Mtconfig.h"  
 

NDIS_STATUS FirmwareCommnadRestartDownloadFW(
    IN  PMP_ADAPTER   pAd
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     	pFwPktContext = &(pAd->pHifCfg->FwPktContext); 
    FIRMWARE_TXDSCR CmdRestartDownloadFwRequest;
    LARGE_INTEGER       TimeOut1Second;

    TimeOut1Second.QuadPart = -(1000* 10000);

    // Fill setting
    PlatformZeroMemory(&CmdRestartDownloadFwRequest, sizeof(FIRMWARE_TXDSCR));
    CmdRestartDownloadFwRequest.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
    CmdRestartDownloadFwRequest.CID = 0xEF; // The ID of CMD w/ target address/length request
    CmdRestartDownloadFwRequest.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
    CmdRestartDownloadFwRequest.Length = sizeof(FIRMWARE_TXDSCR);   

    PlatformZeroMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
    
    // Set the in use bit
    pFwPktContext->InUse = TRUE;               
    pAd->pHifCfg->FwPktContext.BulkOutSize = sizeof(FIRMWARE_TXDSCR);
    PlatformMoveMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], &CmdRestartDownloadFwRequest, sizeof(FIRMWARE_TXDSCR));

    XmitSendMlmeCmdPkt(pAd, TXPKT_FW_FRAME);

    do
    {
        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadRestartDownloadFW - wait to get bulk in ###\n"));


        PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadRestartDownloadFW - got CMD response ###\n"));
    
    }while (FALSE);

    Delay_us(1000);// 1 ms
    
    return NdisStatus;
}

NDIS_STATUS FirmwareCommnadStartToLoadFW(
    IN  PMP_ADAPTER   pAd,
    IN  ULONG           ImageLength
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pFwPktContext = &(pAd->pHifCfg->FwPktContext); 
    CMD_START_TO_LOAD_FW_REQUEST    CmdStartToLoadFwRequest;
    LARGE_INTEGER       TimeOut1Second;

    TimeOut1Second.QuadPart = -(500* 10000);

    DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToLoadFW ###\n"));

    // Fill setting
    PlatformZeroMemory(&CmdStartToLoadFwRequest, sizeof(CMD_START_TO_LOAD_FW_REQUEST));
    CmdStartToLoadFwRequest.FwTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
    CmdStartToLoadFwRequest.FwTxD.CID = 0x01; // The ID of CMD w/ target address/length request
    CmdStartToLoadFwRequest.FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
    CmdStartToLoadFwRequest.FwTxD.Length = sizeof(CMD_START_TO_LOAD_FW_REQUEST);    
    CmdStartToLoadFwRequest.FwTxD.SeqNum = 0;
    CmdStartToLoadFwRequest.Address = 0x00100000;   
    CmdStartToLoadFwRequest.Length = ImageLength;  // FW    size
    CmdStartToLoadFwRequest.DataMode = 0x80000000;  

    PlatformZeroMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
    
    // Set the in use bit
    pFwPktContext->InUse = TRUE;               
    pAd->pHifCfg->FwPktContext.BulkOutSize = sizeof(CMD_START_TO_LOAD_FW_REQUEST);
    PlatformMoveMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], &CmdStartToLoadFwRequest, sizeof(CMD_START_TO_LOAD_FW_REQUEST));

    XmitSendMlmeCmdPkt(pAd, TXPKT_FW_FRAME);

    do
    {
        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToLoadFW - wait to get bulk in ###\n"));

        PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

        Delay_us(1000);// 1 ms

        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToLoadFW - got CMD response ###\n"));
        
    }while (FALSE);
    
    return NdisStatus;
}

NDIS_STATUS FirmwareScatters(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pFirmwareImage,
    IN  ULONG           StartAddress,
    IN  ULONG           ImageLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    ULONG sentLen = 0, remainLen = 0, i = 0;
    ULONG MacValue=0;
    PTX_CONTEXT pFwPktContext = &(pAd->pHifCfg->FwPktContext); 
    ULONG busyCount = 0;
    PUCHAR          pCopyPointer;
    ULONG           writeAddress;
    FIRMWARE_TXDSCR FwTxD;
    LARGE_INTEGER       TimeOut1Second;

    TimeOut1Second.QuadPart = -(500* 10000);
        
    sentLen = 0;
    remainLen = ImageLength;
    pCopyPointer = pFirmwareImage;
    writeAddress = StartAddress;

    DBGPRINT(RT_DEBUG_TRACE,("### FirmwareScatters ###\n"));


    //
    // 2. send out 14k bytes to Andes till end of firmware
    //      
    
    while (remainLen > 0)
    {
        if(remainLen > 14336)
        {
            sentLen = 14336;  // copy 14k bytes at most
            remainLen = remainLen - 14336;
        }
        else
        {
            sentLen = remainLen;
            remainLen = 0;
        }

        DBGPRINT(RT_DEBUG_TRACE,("copying the %d-th Kbytes....sentLen %d \n",++i, sentLen));
        DBGPRINT(RT_DEBUG_TRACE,("Total FileLength %d len remain %d \n", ImageLength, remainLen));
                
        PlatformZeroMemory(&FwTxD, sizeof(FIRMWARE_TXDSCR));
        FwTxD.PQ_ID = 0xC000; //P_IDX[31] = 0x1, Q_IDX[30 : 27] = 0x0
        FwTxD.CID = 0xEE; // The ID of CMD w/ target address/length request
        FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
        FwTxD.Length = sizeof(FIRMWARE_TXDSCR) + sentLen;
        FwTxD.SeqNum = 1;
            
        PlatformZeroMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // Set the in use bit
        pFwPktContext->InUse = TRUE;               
        pAd->pHifCfg->FwPktContext.BulkOutSize = sizeof(FIRMWARE_TXDSCR) + sentLen;
        PlatformMoveMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], &FwTxD, sizeof(FIRMWARE_TXDSCR));
        PlatformMoveMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0] + sizeof(FIRMWARE_TXDSCR), pCopyPointer, sentLen);

        XmitSendMlmeCmdPkt(pAd, TXPKT_FW_FRAME);

        pCopyPointer += sentLen;

        PlatformWaitEventTriggeredAndReset(&(pAd->HwCfg.LoadFWEvent), 1000);

        if (pAd->HwCfg.bLoadingFW == FALSE)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        Delay_us(1000);// 1 ms

    }
    
    return NdisStatus;
}

NDIS_STATUS FirmwareCommnadStartToRunFW(
    IN  PMP_ADAPTER   pAd
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT     pFwPktContext = &(pAd->pHifCfg->FwPktContext); 
    CMD_START_FW_REQUEST    CmdStartFwRequest;
    LARGE_INTEGER       TimeOut1Second;

    TimeOut1Second.QuadPart = -(500* 10000);

    DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToRunFW ###\n"));

    // Fill setting
    PlatformZeroMemory(&CmdStartFwRequest, sizeof(CMD_START_FW_REQUEST));
    CmdStartFwRequest.FwTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
    CmdStartFwRequest.FwTxD.CID = 0x02; // The ID of CMD w/ target address/length request
    CmdStartFwRequest.FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
    CmdStartFwRequest.FwTxD.Length = sizeof(CMD_START_FW_REQUEST);  
    CmdStartFwRequest.FwTxD.SeqNum = 2; 
    CmdStartFwRequest.Override = 0x1;
    CmdStartFwRequest.Address = 0x00100000;

    PlatformZeroMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
    
    // Set the in use bit
    pFwPktContext->InUse = TRUE;               
    pAd->pHifCfg->FwPktContext.BulkOutSize = sizeof(CMD_START_FW_REQUEST);
    PlatformMoveMemory(&pAd->pHifCfg->FwPktContext.TransferBuffer->WirelessPacket[0], &CmdStartFwRequest, sizeof(CMD_START_FW_REQUEST));
    
    XmitSendMlmeCmdPkt(pAd, TXPKT_FW_FRAME);

    do
    {
#if 0   
        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToRunFW - wait to get bulk out complete ###\n"));
        Status = KeWaitForSingleObject(&pAd->HwCfg.LoadFWEvent,
                                Executive,
                                KernelMode,
                                FALSE,
                                &TimeOut1Second);

        if (Status != STATUS_SUCCESS || pAd->HwCfg.bLoadingFW == FALSE)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToRunFW - got bulk out complete ###\n"));
#endif
        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToRunFW - wait to get bulk in ###\n"));

        PlatformWaitEventTriggeredAndReset(&pAd->HwCfg.WaitFWEvent, 1000);

        Delay_us(1000);// 1 ms

        DBGPRINT(RT_DEBUG_TRACE,("### FirmwareCommnadStartToRunFW - got CMD response ###\n"));
        
    }while (FALSE);
    
    return NdisStatus;
}

NDIS_STATUS USB_Load_ILM_DLM(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pFirmwareImage,
    IN  ULONG           StartAddress,
    IN  ULONG           ImageLength
    )
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
#if 0
#if 1  // Write 14 KB one time  (14336 Bytes)
    NTSTATUS Status = STATUS_SUCCESS;
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    ULONG sentLen = 0, remainLen = 0, i = 0;
//  TX_FCE_INFO_STRUC fceInfo = {0};
//  UCHAR bulk_buf[14340] = {0};
    ULONG MacValue=0;
    PTX_CONTEXT pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    ULONG busyCount = 0;
    PUCHAR          pCopyPointer;
    ULONG           writeAddress;
        
    sentLen = 0;
    remainLen = ImageLength;
    pCopyPointer = pFirmwareImage;
    writeAddress = StartAddress;

    FIRMWARE_TXDSCR FwTxD;

    //
    // 2. send out 14k bytes to Andes till end of firmware
    //      

    pAd->HwCfg.bLoadingFW = TRUE;
    
    while (remainLen > 0)
    {
        if(remainLen > 14336)
        {
            sentLen = 14336;  // copy 14k bytes at most
            remainLen = remainLen - 14336;
        }
        else
        {
            sentLen = remainLen;
            remainLen = 0;
        }

        DBGPRINT(RT_DEBUG_TRACE,("copying the %d-th Kbytes....sentLen %d \n",++i, sentLen));
        DBGPRINT(RT_DEBUG_TRACE,("Total FileLength %d len remain %d \n", ImageLength, remainLen));
                
        PlatformZeroMemory(&FwTxD, sizeof(FIRMWARE_TXDSCR));
        FwTxD.u2PQ_ID = 0xD000;
        FwTxD.ucPktTypeID = 0xA0;
            
//      PlatformZeroMemory(bulk_buf, sizeof(bulk_buf));
//      RtlCopyMemory(bulk_buf, &fceInfo, sizeof(TX_FCE_INFO_STRUC));
//      RtlCopyMemory(bulk_buf + sizeof(TX_FCE_INFO_STRUC), pCopyPointer, sentLen);
//      pCopyPointer += sentLen;

        //
        // write 0x400230 = firmware_start_address
        //
        RTUSBWriteMACRegisterExt1(pAd, 0x230, writeAddress);
        writeAddress += sentLen;

        //while((len%4) !=0)
        //len++;

        //
        // write 0x400234[31:16] = firmware byte count
        //
        RTUSBReadMACRegisterExt1(pAd, 0x234, &MacValue);
        MacValue &= 0xFFFF;
        MacValue |= (sentLen<<16);
        DBGPRINT(RT_DEBUG_TRACE,("write firmware segment, len = 0x%x\n", sentLen + sizeof(TX_FCE_INFO_STRUC)));
        RTUSBWriteMACRegisterExt1(pAd, 0x234, MacValue);

            
        // ========================================
#if 0
        busyCount = 0;
        while(1)
        {
            if (pPktCmdContext->InUse == FALSE)
            {
                PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
                
                // Set the in use bit
                pPktCmdContext->InUse = TRUE;               
                pAd->pHifCfg->PktCmdContext.BulkOutSize =  sizeof(TX_FCE_INFO_STRUC) + sentLen;
                PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &fceInfo, sizeof(TX_FCE_INFO_STRUC));
                PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0] + sizeof(TX_FCE_INFO_STRUC), pCopyPointer, sentLen);

                // set flag to trigger bulkout null frame
                MTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PKTCMD);

                // Kick bulk out 
                N6USBKickBulkOut(pAd);
                break;
            }
            else
            {
                DbgPrint("6x9x ERROR!!, pPktCmdContext->InUse == TRUE\n");
                busyCount++;
            }
            if(busyCount>100)
                break;
        }
#else
        PlatformZeroMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], MAX_FW_SIZE);
        
        // Set the in use bit
        pPktCmdContext->InUse = TRUE;               
        pAd->pHifCfg->PktCmdContext.BulkOutSize =  sizeof(TX_FCE_INFO_STRUC) + sentLen;
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], &fceInfo, sizeof(TX_FCE_INFO_STRUC));
        PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0] + sizeof(TX_FCE_INFO_STRUC), pCopyPointer, sentLen);

        // set flag to trigger bulkout null frame
        MTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PKTCMD);
        
        // Kick bulk out 
        N6USBKickBulkOut(pAd);
#endif
        pCopyPointer += sentLen;

        Status = KeWaitForSingleObject(&pAd->HwCfg.LoadFWEvent,
                                Executive,
                                KernelMode,
                                FALSE,
                                NULL);

        if (Status != STATUS_SUCCESS || pAd->HwCfg.bLoadingFW == FALSE)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        while(1)
        {
            NdisCommonGenericDelay(100);
            RTUSBReadMACRegisterExt1(pAd, 0x234, &MacValue);
            DBGPRINT(RT_DEBUG_TRACE,("MacValue = %08x\n", MacValue));
            if (MacValue & 0x80000000)
            {
                DBGPRINT(RT_DEBUG_TRACE,("PDMA DDONE : MacValue = %08x\n", MacValue));
                break;
            }

            i++;
            if(i > 100)
            {
                NdisStatus = NDIS_STATUS_FAILURE;
                break;
            }
        }
    }

    pAd->HwCfg.bLoadingFW = FALSE;
    
#else   // Write 4 KB one time
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG sentLen = 0, len = 0, i = 0;
    TX_FCE_INFO_STRUC fceInfo = {0};
    UCHAR bulk_buf[4120] = {0};
    ULONG MacValue=0;
    PTX_CONTEXT pPktCmdContext = &(pAd->pHifCfg->PktCmdContext); 
    ULONG busyCount = 0;
        
    sentLen = 0;

    //
    // 2. send out 14k bytes to Andes till end of firmware
    //      
    while (sentLen <= ImageLength)
    {
        if(sentLen >= ImageLength)
        {
            DBGPRINT(RT_DEBUG_TRACE,("BIN file copying completed.. %d bytes copyied.\n",sentLen));
            break;
        }

        DBGPRINT(RT_DEBUG_TRACE,("copying the %d-th Kbytes....sentLen %d \n",++i, sentLen));
            
        len = ImageLength - sentLen;

        DBGPRINT(RT_DEBUG_TRACE,("Total FileLength %d len remain %d \n",ImageLength ,len));
            
        if(len > 4096)
            len = 4096;  // copy 14k bytes at most      
                
        fceInfo.PKT_LEN = len;
        fceInfo.D_PORT = 2;
            
        PlatformZeroMemory(bulk_buf, sizeof(bulk_buf));
        RtlCopyMemory(bulk_buf, &fceInfo, sizeof(TX_FCE_INFO_STRUC));
        RtlCopyMemory(bulk_buf + sizeof(TX_FCE_INFO_STRUC), pFirmwareImage + sentLen, len);

        //
        // write 0x400230 = firmware_start_address
        //
        RTUSBWriteMACRegisterExt1(pAd, 0x230, StartAddress + sentLen);
        sentLen += len;

        //while((len%4) !=0)
        //len++;

        //
        // write 0x400234[31:16] = firmware byte count
        //
        RTUSBReadMACRegisterExt1(pAd, 0x234, &MacValue);
        MacValue &= 0xFFFF;
        MacValue |= (len<<16);
        DBGPRINT(RT_DEBUG_TRACE,("write firmware segment, len = 0x%x\n",len + sizeof(TX_FCE_INFO_STRUC)));
        RTUSBWriteMACRegisterExt1(pAd, 0x234, MacValue);

            
        // ========================================
        busyCount = 0;
        while(1)
        {
            if (pPktCmdContext->InUse == FALSE)
            {
                // Set the in use bit
                pPktCmdContext->InUse = TRUE;               
                pAd->pHifCfg->PktCmdContext.BulkOutSize =  sizeof(TX_FCE_INFO_STRUC) + len;
                PlatformMoveMemory(&pAd->pHifCfg->PktCmdContext.TransferBuffer->WirelessPacket[0], bulk_buf, pAd->pHifCfg->PktCmdContext.BulkOutSize);

                // set flag to trigger bulkout null frame
                MTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PKTCMD);

                // Kick bulk out 
                N6USBKickBulkOut(pAd);
                break;
            }
            else
            {
                DbgPrint("6x9x ERROR!!, pPktCmdContext->InUse == TRUE\n");
                busyCount++;
            }
            if(busyCount>100)
                break;
        }
        NdisCommonGenericDelay(3);
    }
#endif
#endif

    return NdisStatus;
}

#if 1
//
// Load Andes firmware
// MT7603 FPGA
//
NDIS_STATUS LoadAndesFW(
    IN PMP_ADAPTER pAd)
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    NTSTATUS CRStatus;
    NDIS_HANDLE FileHandle = NULL;
    NDIS_STRING FileName = {0};
    UINT FileLength = 0;
    NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);
    PUCHAR pFirmwareImage = NULL;
    ULONG  FWImageLength  = 0;
    BOOLEAN bLoadExternalFW = FALSE;
    PFW_BIN_INFO_STRUC pFwInfo = NULL;
    TOP_MISC_CONTROLS2 TopMiscControls2;
    U3DMA_WLCFG         U3DMAWLCFG;
    SCHEDULER_REGISTER4 SchedulerRegister4, backupSchedulerRegister4;
//  ULONG                   MACValue;
    ULONG                   BreakCount = 0;
//  UCHAR                   ReloadFWLimited;

    NdisInitializeString(&FileName,"FW_7603.bin");

    NdisOpenFile(&NdisStatus, &FileHandle, &FileLength, &FileName, HighestAcceptableMax);

    DBGPRINT(RT_DEBUG_TRACE, ("%s: FW size = %d\n", __FUNCTION__,FileLength));

    NdisFreeString(FileName);


    if ((NdisStatus != NDIS_STATUS_SUCCESS) || (FileLength == 0)) 
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NdisOpenFile() failed\n", __FUNCTION__));

#if defined(_WIN64)

        //pFirmwareImage = FW7650;
        //FileLength = FW7650_LENGTH;
        
        //pFwHeader = (PFW_BIN_HEADER_STRUC)(pFirmwareImage);
        //pAd->HwCfg.FirmwareVersion = pFwHeader->WiFiFwVersion;

        //bLoadExternalFW = FALSE;
        //NdisStatus = NDIS_STATUS_SUCCESS;

        //DBGPRINT(RT_DEBUG_TRACE, ("%s: Load default 6x9x (7650/7630/7610/7601) firmware, WiFiFwVersion = 0x%08X\n", 
        //  __FUNCTION__, 
        //  pFwHeader->WiFiFwVersion));
#else
        //return NDIS_STATUS_FAILURE;
#endif

    }
    else
    {
        bLoadExternalFW = TRUE; // Load the external firmware file

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Load the external firmware file\n", __FUNCTION__));
    }

    if (bLoadExternalFW == TRUE)
    {
        NdisMapFile(&NdisStatus, &pFirmwareImage, FileHandle);
        DBGPRINT(RT_DEBUG_TRACE, ("%s: NdisMapFile FileLength=%d\n", __FUNCTION__, FileLength));


        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR,("Ndis Map File failed!!\n"));
            NdisCloseFile(FileHandle);
            return NDIS_STATUS_FAILURE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("%s: NdisMapFile FileLength=%d\n", __FUNCTION__, FileLength));
    }
    
    if((FileLength <= FW_INFO_LENGTH) && (bLoadExternalFW == TRUE))
    {
        DBGPRINT(RT_DEBUG_ERROR,("invliad firmware, size < 64 bytes\n"));
            NdisUnmapFile(FileHandle);
            NdisCloseFile(FileHandle);
        return NDIS_STATUS_FAILURE;
    }

    pFwInfo           = (PFW_BIN_INFO_STRUC)(pFirmwareImage + (FileLength - sizeof(FW_BIN_INFO_STRUC)));
    FWImageLength    = pFwInfo->ImageLength + 4;  // 4 is CRC length

    DBGPRINT(RT_DEBUG_TRACE,("%s: FileLength = 0x%08X, FWImageLength = 0x%08X\n", __FUNCTION__, FileLength, FWImageLength));    

    // Set DMA scheduler to Bypass mode.
    // Enable Tx/Rx
    CRStatus = HW_IO_READ32(pAd, UDMA_WLCFG_0, &U3DMAWLCFG.word);
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_ERROR,("1 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
    U3DMAWLCFG.field.WL_TX_EN = 1;
    U3DMAWLCFG.field.WL_RX_EN = 1;
    U3DMAWLCFG.field.WL_RX_MPSZ_PAD0 = 1;
    DBGPRINT(RT_DEBUG_ERROR,("2 U3DMAWLCFG = %x\n", U3DMAWLCFG));
    CRStatus = HW_IO_WRITE32(pAd, UDMA_WLCFG_0, U3DMAWLCFG.word);
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }

    // Set DMA scheduler to Bypass mode.
    CRStatus = HW_IO_READ32(pAd, SCH_REG4, &SchedulerRegister4.word);
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_ERROR,("1 SchedulerRegister4 = %x\n", SchedulerRegister4.word));
    backupSchedulerRegister4.field.BypassMode = SchedulerRegister4.field.BypassMode;
    SchedulerRegister4.field.BypassMode = 1;
    SchedulerRegister4.field.ForceQid = 0;
    DBGPRINT(RT_DEBUG_ERROR,("2 SchedulerRegister4 = %x\n", SchedulerRegister4));
    CRStatus = HW_IO_WRITE32(pAd, SCH_REG4, SchedulerRegister4.word);
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    
    do
    {
        // check if need to reload FW
        CRStatus = HW_IO_READ32(pAd, TOP_MISC2, &TopMiscControls2.word);
        if (CRStatus != STATUS_SUCCESS)
        {
            return NDIS_STATUS_FAILURE;
        }

        if (TopMiscControls2.field.FwIsRunning)
        {
            NdisStatus = FirmwareCommnadRestartDownloadFW(pAd);
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {               
                return NDIS_STATUS_FAILURE;
            }
        }

        if (!TopMiscControls2.field.ReadyToLoadFW)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("!!!!! TOP is not ready to load FW. !!!!!\n"));
            return NDIS_STATUS_FAILURE;
        }

        // Send command packet to FW to set "start to load FW"
        NdisStatus = FirmwareCommnadStartToLoadFW(pAd, FWImageLength);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("!!!!! FirmwareCommnadStartToLoadFW failed. !!!!!\n"));
            return NDIS_STATUS_FAILURE;
        }

        pAd->HwCfg.bLoadingFW = TRUE;

        // Start to load FW
        NdisStatus = FirmwareScatters(pAd, pFirmwareImage, 0, FWImageLength);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("!!!!! FirmwareScatters failed. !!!!!\n"));
            return NDIS_STATUS_FAILURE;
        }

        pAd->HwCfg.bLoadingFW = FALSE;

        // FW Starting
        NdisStatus = FirmwareCommnadStartToRunFW(pAd);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("!!!!! FirmwareCommnadStartToRunFW failed. !!!!!\n"));
            return NDIS_STATUS_FAILURE;
        }

    } while(FALSE);

    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));

    //
    // close BIN file
    //
    if ((bLoadExternalFW == TRUE) && (NdisStatus == NDIS_STATUS_SUCCESS))
    {
        DBGPRINT(RT_DEBUG_ERROR,("Loading FW SUCCESS\n"));
        NdisUnmapFile(FileHandle);
        NdisCloseFile(FileHandle);
    }

    // Polling 0x80021134 [1] = 1, and then can read/write CRs
    do
    {
        HW_IO_READ32(pAd, TOP_MISC2, &TopMiscControls2.word);

        if (TopMiscControls2.field.FwIsRunning == 1)
            break;
        
        Delay_us(100000);// 1 ms
        

        BreakCount ++;
        if (BreakCount > 100)
        {
            DBGPRINT(RT_DEBUG_ERROR,("Polling FW ready bit failed.\n"));
            break;
        }
        
    } while(1);

    // Set DMA scheduler to original mode.
    SchedulerRegister4.field.BypassMode = backupSchedulerRegister4.field.BypassMode;
    CRStatus = HW_IO_WRITE32(pAd, SCH_REG4, SchedulerRegister4.word);
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }

#if 0 // patch Queue 8 detect error packet number, toggle 0x50000594[8] -->1 -->0
DBGPRINT(RT_DEBUG_ERROR,("1 --------------> \n"));
    CRStatus = PlatformAccessCR(pAd, CR_READ, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    MACValue |= 0x00000100;
    CRStatus = PlatformAccessCR(pAd, CR_WRITE, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }

    CRStatus = PlatformAccessCR(pAd, CR_READ, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    MACValue &= 0xFFFFFEFF;
    CRStatus = PlatformAccessCR(pAd, CR_WRITE, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
DBGPRINT(RT_DEBUG_ERROR,("2 --------------> \n"));
#endif

    pAd->HwCfg.LoadingFWCount ++;
    DBGPRINT(RT_DEBUG_TRACE, ("####################### pAd->HwCfg.LoadingFWCount = %d ###################\n", pAd->HwCfg.LoadingFWCount));

    return NdisStatus;
}
#else
//
// Load Andes firmware
//
NDIS_STATUS LoadAndesFW(
    IN PMP_ADAPTER pAd)
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE FileHandle = NULL;
    NDIS_STRING FileName = {0};
    NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);
    UINT FileLength = 0, i = 0;
    TX_FCE_INFO_STRUC fceInfo = {0};
    ULONG FW = {0};
    ULONG MacValue=0;
    ULONG len=0;
    ULONG sentLen=0;
    UCHAR firstbuffer[64];
    PUCHAR pFirmwareImage = NULL;   
    ULONG busyCount = 0;        
    PUCHAR              pSrc = NULL;
    //============
    USHORT Value,Index = 0;     
    UCHAR Request = 0;      
    ULONG TransferBufferLength = 64;        
    PVOID TransferBuffer = NULL;        
    ULONG TEMP;

    // 
    // Andes FW loading architecture for ILM/DLM
    // 
    PUCHAR pFirmwareImageILM = NULL;
    PUCHAR pFirmwareImageDLM = NULL;
    ULONG  FWImageILMLength  = 0;
    ULONG  FWImageDLMLength  = 0;
    ULONG  BtFwVersion;
    ULONG  WiFiFwVersion;
    ULONGLONG FwTimeStampLowPart;
    ULONG     FwTimeStampHighPart;
    BOOLEAN bLoadExternalFW = FALSE;
    NTSTATUS    Status = STATUS_SUCCESS;

    PFW_BIN_HEADER_STRUC pFwHeader = NULL;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //
    // If the firmware is activated (0x400230 = 0xFFFFFFFF), no need to reload firmware. 
    //
    Status = RTUSBReadMACRegisterExt1(pAd, 0x230, &MacValue);
    if(Status != STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE,("Maybe something wrong in WdfUsbTargetDeviceSendControlTransferSynchronously!!\n"));
        NdisStatus = NDIS_STATUS_FAILURE;
        return NdisStatus;
    }

    DBGPRINT(RT_DEBUG_TRACE,("0x400230 = 0x%x\n",MacValue));
    
    if(0)
    {
        if((MacValue & 0x1) == 0x1)
        {
            DBGPRINT(RT_DEBUG_TRACE,("Firmware is activated, no need to re-load FW.bin\n"));
            NdisStatus = NDIS_STATUS_SUCCESS;
            return NdisStatus;      
        }
    }
    else
    {
        if((MacValue & 0x1) == 0x1)
        {   
            DBGPRINT(RT_DEBUG_TRACE,("Firmware is activated, reset to re-load FW\n"));
            // Re-load FW
            RTUSBWriteMACRegisterExt1(pAd, 0x230, 0x0);
            NdisCommonGenericDelay(5000);
            RTUSBVenderReset(pAd);
            NdisCommonGenericDelay(20000);
        }
    }

    NdisOpenFile(&NdisStatus, &FileHandle, &FileLength, &FileName, HighestAcceptableMax);
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: FW size = %d\n", __FUNCTION__,FileLength));

    NdisFreeString(FileName);


    if ((NdisStatus != NDIS_STATUS_SUCCESS) || 
        (FileLength == 0)) 
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NdisOpenFile() failed\n", __FUNCTION__));

#if defined(_WIN64)

        {
            return NDIS_STATUS_SUCCESS;
        }
        
        pFwHeader = (PFW_BIN_HEADER_STRUC)(pFirmwareImage);
        pAd->HwCfg.FirmwareVersion = pFwHeader->WiFiFwVersion;

        bLoadExternalFW = FALSE;
        NdisStatus = NDIS_STATUS_SUCCESS;

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Load default 6x9x (7650/7630/7610/7601) firmware, WiFiFwVersion = 0x%08X\n", 
            __FUNCTION__, 
            pFwHeader->WiFiFwVersion));
#else
        return NDIS_STATUS_FAILURE;
#endif

    }
    else
    {
        bLoadExternalFW = TRUE; // Load the external firmware file

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Load the external firmware file\n", __FUNCTION__));
    }

    RTUSBWriteMACRegister(pAd, 0x0800, 0x00000001);  // Enable FCE
    RTUSBWriteMACRegister(pAd, 0x0238, 0x00c00000);  // Enable USB_DMA_CFG
    RTUSBWriteMACRegister(pAd, 0x09a0, 0x00400230);  // FCE tx_fs_base_ptr
    RTUSBWriteMACRegister(pAd, 0x09a4, 0x00000001);  // FCE tx_fs_max_cnt
    RTUSBWriteMACRegister(pAd, 0x09a8, 0x00000001);  // FCE tx_fs_ctx_idx
    RTUSBWriteMACRegister(pAd, 0x09c4, 0x00000044);  // FCE pdma enable
    RTUSBWriteMACRegister(pAd, 0x0a6c, 0x00000003);  // FCE skip_fs_en

/*
    //Use sys clock (CLOCK_CTL[12]) before FW loaded for speed up
    RTUSBReadMACRegisterExt1(pAd, 0x0208, &MacValue);
    MacValue |= BIT12;
    RTUSBWriteMACRegisterExt1(pAd, 0x0208, MacValue);
*/
    // ===============================================
    // Load Andes firmware
    // ===============================================
    //  Andes FW loading architecture
    //    A. ILM and DLM is continuous     DLM index start address: 0x20000 (7650E1, 7601) 
    //    B. ILM and DLM is not continuous DLM index start address: 0x80000 (7650E2 and later, 7662)

    // ===============================================
    // Load Firmware
    // ===============================================
    //  Andes FW loading architecture
    //
    //  Bin file format
    //    +--------------------------+
    //    |     FW header 32byte     |
    //    +--------------------------+
    //    |       FW ILM part        |
    //    +--------------------------+
    //    |       FW DLM part        |
    //    +--------------------------+      
    //  ILM segment to 2 parts
    //    1. iVECT part: 64byte
    //    2. Remnant part
    //    +--------------------------+
    //    | iVECT part| Remnant part |
    //    |                          |          
    //    +--------------------------+
    if (bLoadExternalFW == TRUE)
    {
        NdisMapFile(&NdisStatus, &pFirmwareImage, FileHandle);
        DBGPRINT(RT_DEBUG_TRACE, ("%s: NdisMapFile FileLength=%d\n", __FUNCTION__, FileLength));


        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR,("Ndis Map File failed!!\n"));
            NdisCloseFile(FileHandle);
            return NDIS_STATUS_FAILURE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("%s: NdisMapFile FileLength=%d\n", __FUNCTION__, FileLength));
    }
    
    pFwHeader           = (PFW_BIN_HEADER_STRUC)pFirmwareImage;
    FWImageILMLength    = pFwHeader->IlmLen;
    FWImageDLMLength    = pFwHeader->DlmLen;
    BtFwVersion         = pFwHeader->BtFwVersion;
    WiFiFwVersion       = pFwHeader->WiFiFwVersion;     
    FwTimeStampLowPart  = pFwHeader->TimeStampLowPart;
    FwTimeStampHighPart = pFwHeader->TimeStampHighPart;

    pFirmwareImageILM   = pFirmwareImage + sizeof(FW_BIN_HEADER_STRUC);
    pFirmwareImageDLM   = pFirmwareImage + sizeof(FW_BIN_HEADER_STRUC) + FWImageILMLength;

    DBGPRINT(RT_DEBUG_TRACE,("%s: WiFi Firmware Ver= 0x%08X, BT Firmware Ver= 0x%08X\n", 
                    __FUNCTION__, WiFiFwVersion, BtFwVersion));     
    DBGPRINT(RT_DEBUG_TRACE,("%s: TimeStamp LowPart= 0x%016llX, TimeStamp HighPart= 0x%08X\n", 
                    __FUNCTION__, FwTimeStampLowPart, FwTimeStampHighPart));
    DBGDumpHex(RT_DEBUG_TRACE, "Parsing Firmware TimeStamp", 
                    ( sizeof(FwTimeStampLowPart) + sizeof(FwTimeStampHighPart) ), 
                    (PUCHAR)(&pFwHeader->TimeStampLowPart), TRUE);
    DBGPRINT(RT_DEBUG_TRACE,("%s: Firmware len= 0x%08X, ILM part len= 0x%08X, DLM part len= 0x%08X\n", 
                    __FUNCTION__, FileLength, FWImageILMLength, FWImageDLMLength)); 
    //
    // 1. read first 64 bytes into tmp buffer
    //

    if(FileLength > IVECTOR_LENGTH)
    {
        PlatformZeroMemory(firstbuffer, sizeof(firstbuffer));
        RtlCopyMemory(firstbuffer, pFirmwareImageILM, IVECTOR_LENGTH);
        sentLen += IVECTOR_LENGTH;
    }
    else
    {
        if (bLoadExternalFW == TRUE)
        {

            DBGPRINT(RT_DEBUG_ERROR,("invliad firmware, size < 64 bytes\n"));
                NdisUnmapFile(FileHandle);
                NdisCloseFile(FileHandle);
            return NDIS_STATUS_FAILURE;
        }
    }

    {
        // 
        // Andes FW loading architecture for ILM/DLM
        //

        // 2. send out 4*1024bytes ILM to Andes till end of firmware
        NdisStatus = USB_Load_ILM_DLM(pAd,
                        pFirmwareImageILM + IVECTOR_LENGTH,
                        ILMFW_START_ADDRESS,
                        FWImageILMLength - IVECTOR_LENGTH);

        if ((bLoadExternalFW == TRUE) && 
            (NdisStatus != NDIS_STATUS_SUCCESS))
        {
            DBGPRINT(RT_DEBUG_ERROR,("ERROR!!! Loading ILM FAIL\n"));
            NdisUnmapFile(FileHandle);
            NdisCloseFile(FileHandle);
            return NdisStatus;
        }

        //3. send out 14*1024bytes DLM to Andes till end of firmware
        NdisStatus = USB_Load_ILM_DLM(pAd,
                        pFirmwareImageDLM,
                        DLMFW_START_ADDRESS,
                        FWImageDLMLength);

        if ((bLoadExternalFW == TRUE) && 
            (NdisStatus != NDIS_STATUS_SUCCESS))
        {
            DBGPRINT(RT_DEBUG_ERROR,("ERROR!!! Loading DLM FAIL\n"));
            NdisUnmapFile(FileHandle);
            NdisCloseFile(FileHandle);
            return NdisStatus;
        }

        //
        // close BIN file
        //
        if ((bLoadExternalFW == TRUE) && 
            (NdisStatus == NDIS_STATUS_SUCCESS))
        {
            DBGPRINT(RT_DEBUG_ERROR,("Loading ILM/DLM SUCCESS\n"));
            NdisUnmapFile(FileHandle);
            NdisCloseFile(FileHandle);
        }
    }

    //
    // 5. launch the firmware
    //
    //NdisCommonGenericDelay(100000);
    DBGPRINT(RT_DEBUG_TRACE,("launch the firmware ....\n"));
    NdisStatus = N6UsbIoVendorRequest(
                            pAd,
                            USBD_TRANSFER_DIRECTION_OUT,
                            DEVICE_VENDOR_REQUEST_OUT,
                            0x1,  //Request
                            0x12, // Value
                            0,
                            firstbuffer,
                            TransferBufferLength);
    NdisCommonGenericDelay(10000);

/*
    // Mark for S4 BSOD hit when i = 60
    // for debug only
    for(i=0;i<64;i++)
    {
        MacValue = 0;
        N6UsbIoVendorRequest(
                        pAd,
                        USBD_TRANSFER_DIRECTION_IN ,
                        DEVICE_VENDOR_REQUEST_IN,
                        0x27,
                        0,
                        i*4,
                        &MacValue,
                        4);
            DbgPrint("firmware : 0x%08x\n",MacValue);
    }
    NdisCommonGenericDelay(3000);
*/
#if 1
    //
    // 6. Release Semaphore 
    //

    MacValue = 0x00;
    Status = RTUSBReadMACRegisterExt1(pAd, 0x230, &MacValue);
    if(Status != STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE,("Maybe something wrong in WdfUsbTargetDeviceSendControlTransferSynchronously!!\n"));
        NdisStatus = NDIS_STATUS_FAILURE;
        return NdisStatus;
    }

    DBGPRINT(RT_DEBUG_TRACE,("0x400230 = 0x%x\n",MacValue));

    // For Emulation stage, only monitor the bit of MCU ready.
    if ((MacValue & 0x00000001) != 0x1)
    {
        DBGPRINT(RT_DEBUG_TRACE,("ERROR !! In%s, MCU not ready\n",__FUNCTION__));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,("In%s, MCU is ready\n",__FUNCTION__));
    }

/*
    // Reset Semaphore to 0x01 after checking or FW loaded
    RTUSBReadMACRegisterExt1(pAd, SEMAPHORE_00, &MacValue);
    MacValue |= BIT0;
    RTUSBWriteMACRegisterExt1(pAd, SEMAPHORE_00, MacValue);
    DBGPRINT(RT_DEBUG_TRACE, ("%s: FW up, Release Semaphore.\n", __FUNCTION__));
*/  
#endif

/*
    // Reset CLOCK_CTL[12] after FW loaded
    RTUSBReadMACRegisterExt1(pAd, 0x0208, &MacValue);
    MacValue &= ~(0x1000);
    RTUSBWriteMACRegisterExt1(pAd, 0x0208, MacValue);
*/
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));

    return NdisStatus;
}
#endif
