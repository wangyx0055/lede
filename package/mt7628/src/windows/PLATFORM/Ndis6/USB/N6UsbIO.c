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
    rtusb_io.c

    Abstract:

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    06-25-2004    created

*/
#include    "MtConfig.h"

//
// Initialize e-Fuse configuration
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID InitEFuseConfig(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

    //2 TODO: Contact the ASIC team to make sure the mapping configuration of the e-Fuse

    pAd->HwCfg.eFuseConfig.MapStart = 0x02D0;
    pAd->HwCfg.eFuseConfig.MapEnd = 0x02FC;
    pAd->HwCfg.eFuseConfig.MapSize = 45;

    DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
}

//
// The adapter supports (a) the flash installation (auto-run) and (b) 8KB firmware in the EEPROM and some EEPROM configurations are in e-fuse component
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  TRUE: The adapter supports (a) the flash installation (auto-run) and (b) 8KB firmware in the EEPROM and some EEPROM configurations are in e-fuse component
//  FALSE: Standard adapter
//
BOOLEAN FlashInstallationWith8KBFirmware(
    IN PMP_ADAPTER pAd)
{
    BOOLEAN bResult = FALSE;
    ULONG FirmwareMode = 0;

    DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

    RTUSBFirmwareOpmode(pAd,&FirmwareMode);

    if ((pAd->HwCfg.buseEfuse == FALSE) && ((FirmwareMode&0x00000003) == 2)) // CD-ROM mode (#2)
    {
        bResult = TRUE;
    }
    else
    {
        bResult = FALSE;
    }
    
    DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));

    return bResult;
}

/*
    ========================================================================
    
    Routine Description: Get current firmware operation mode (Return Value)

    Arguments:

    Return Value: 
        0 or 1 = Downloaded by host driver
        others = Driver doesn't download firmware

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBFirmwareOpmode(
    IN  PMP_ADAPTER   pAd,
    OUT PULONG          pValue)
{
    NTSTATUS    Status = NDIS_STATUS_SUCCESS;

    Status = N6UsbIoVendorRequest(
        pAd,
        USBD_TRANSFER_DIRECTION_IN,
        DEVICE_VENDOR_REQUEST_IN,
        0x1,
        0x11,
        0,
        pValue,
        4);
    return Status;
}

//
// Send USB vendor request to underlying WDM USB driver.
//
NTSTATUS    USBVendorRequest(
    IN  PMP_ADAPTER   pAd,
    IN  ULONG           Direction,
    IN  UCHAR           Request,
    IN  USHORT          Value,
    IN  USHORT          Index,
    IN  PVOID           TransferBuffer,
    IN  ULONG           TransferBufferLength)
{
    WDF_USB_CONTROL_SETUP_PACKET    controlSetupPacket;
    WDF_MEMORY_DESCRIPTOR           memDesc;
    WDF_REQUEST_SEND_OPTIONS        options;
    ULONG                           bytesTransferred;
    NTSTATUS                        ntStatus;
    UCHAR                           Number;

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeGetCurrentIrql() != PASSIVE_LEVEL, KeGetCurrentIrql() = %d\n", __FUNCTION__, KeGetCurrentIrql()));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    // Acquire Control token
    while (1)
    {
#if _WIN8_USB_SS_SUPPORTED
        NdisAcquireSpinLock(&pAd->Ss.SsLock);
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
        {
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
            return (STATUS_DEVICE_NOT_CONNECTED);
        }
#endif
        NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
        if (pAd->pHifCfg->ControlPending == FALSE)
        {
            pAd->pHifCfg->ControlPending = TRUE;
            NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
#if _WIN8_USB_SS_SUPPORTED
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif
            break;
        }
        else
        {
            NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
#if _WIN8_USB_SS_SUPPORTED
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif
            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                return (STATUS_DEVICE_NOT_CONNECTED);   
            
            NdisCommonGenericDelay(500);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - USB ControlPending=TRUE, Delay 500usec\n", __FUNCTION__));  
        }
    }
    
    // Check for correct level
    ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("IoBuildDeviceIoControlRequest Irq != PASSIVE_LEVEL. KeGetCurrentIrql()=%d\n", KeGetCurrentIrql()));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        // Release token
        NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
        pAd->pHifCfg->ControlPending = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
        return(ntStatus);
    }


    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        // Release token
        NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
        pAd->pHifCfg->ControlPending = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
        return (STATUS_DEVICE_NOT_CONNECTED);
    }

    WDF_USB_CONTROL_SETUP_PACKET_INIT_VENDOR(
                            &controlSetupPacket,
                            Direction,
                            RTUSB_RequestToDevice,
                            Request,
                            Value,
                            Index);

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
                            &memDesc, 
                            TransferBuffer, 
                            TransferBufferLength);


    WDF_REQUEST_SEND_OPTIONS_INIT(
                            &options,
                            WDF_REQUEST_SEND_OPTION_TIMEOUT);

    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
                            &options,
                            WDF_REL_TIMEOUT_IN_SEC(1));  // time out after one second

    Number = 0;

    do{

    ntStatus = WdfUsbTargetDeviceSendControlTransferSynchronously(
                            pAd->pHifCfg->UsbDevice, 
                            WDF_NO_HANDLE,              // Optional WDFREQUEST
                            &options,                       // PWDF_REQUEST_SEND_OPTIONS
                            &controlSetupPacket,
                            &memDesc,
                            &bytesTransferred);

    if(!NT_SUCCESS(ntStatus)) 
    {
            DBGPRINT(RT_DEBUG_ERROR, ("%s:WdfUsbTargetDeviceSendControlTransferSynchronously,request:0x%x, Failed - 0x%x ,Index = %x\n", __FUNCTION__, Request, ntStatus, Index));

        //
        // A device which does not exist was specified.
        //

        if ((ntStatus == STATUS_NO_SUCH_DEVICE) ||
            (ntStatus == STATUS_DEVICE_NOT_CONNECTED) ||
            (ntStatus == STATUS_DEVICE_REMOVED))
        {
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
            pAd->Counter.MTKCounters.VendorRequestFail++;
            PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_SUCCESS, TRUE, TRUE);
        }
            else if (ntStatus == STATUS_IO_TIMEOUT)
        {
            pAd->Counter.MTKCounters.VendorRequestTimeout++;
        }
    }
        else
        {
            break;
        }

        Number++;

    }while(Number < 5);

    // Release token
    NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
    pAd->pHifCfg->ControlPending = FALSE;
    NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);

    return ntStatus;
}

#if 1
/*
    ========================================================================
    
    Routine Description: Read various length data from RT2573

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBMultiRead(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    OUT PUCHAR          pData,
    IN  USHORT          length)
{
    NTSTATUS    Status;
    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("-->%s, offset%x, length%d\n", __FUNCTION__, Offset, length));  
    Status = N6UsbIoVendorRequest(
        pAd,
        USBD_TRANSFER_DIRECTION_IN ,
        DEVICE_VENDOR_REQUEST_IN,
        0x7,
        0,
        Offset,
        pData,
        length);
    
    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("<--%s, status%d\n", __FUNCTION__, Status));    
    return Status;
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBSingleWrite(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  USHORT          Value)
{
    NTSTATUS    Status = NDIS_STATUS_SUCCESS;
    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("-->%s, offset%x, Value%d\n", __FUNCTION__, Offset, Value));    
#if 0
    Status = N6UsbIoVendorRequest(pAd,
        USBD_TRANSFER_DIRECTION_OUT,
        DEVICE_VENDOR_REQUEST_OUT,
        0x2,
        Value,
        Offset,
        NULL,
        0);
    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("<--%s, status%d\n", __FUNCTION__, status));    
#endif  

    return Status;
}


#if 0
/*
    ========================================================================
    
    Routine Description: Write various length data to RT2573

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBMultiWrite(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  PUCHAR          pData,
    IN  USHORT          length)
{
    NTSTATUS    Status;
    

    Status = N6UsbIoVendorRequest(
        pAd,
        USBD_TRANSFER_DIRECTION_OUT,
        DEVICE_VENDOR_REQUEST_OUT,
        0x6,
        0,
        Offset,
        pData,
        length);
    
    return Status;
}
#else
NTSTATUS    RTUSBMultiWrite(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  PUCHAR          pData,
    IN  USHORT          length)
{
    NTSTATUS    Status;
    USHORT      index = 0,Value;
    PUCHAR      pSrc = pData;
    USHORT      resude = 0;

    resude = length % 2;
     length  += resude ;
    do
    {
        Value =(USHORT)( *pSrc  | (*(pSrc + 1) << 8));
        Status = RTUSBSingleWrite(pAd,Offset + index,Value);
        index +=2;
        length -= 2;
        pSrc = pSrc + 2;
    }while(length > 0); 

    return Status;
}
#endif

/*
    ========================================================================
    
    Routine Description: Read 32-bit MAC register

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBReadMACRegister(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    OUT PULONG          pValue)
{
    NTSTATUS    Status = NDIS_STATUS_SUCCESS;
#if 0
    Status = N6UsbIoVendorRequest(
        pAd,
        USBD_TRANSFER_DIRECTION_IN ,
        DEVICE_VENDOR_REQUEST_IN,
        0x7,
        0,
        Offset,
        pValue,
        4);
#endif  
    return Status;
}


#if 0
/*
    ========================================================================
    
    Routine Description: Write 32-bit MAC register

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBWriteMACRegister(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  ULONG           Value)
{
    NTSTATUS    Status;
    
    Status = N6UsbIoVendorRequest(
        pAd,
        USBD_TRANSFER_DIRECTION_OUT,
        DEVICE_VENDOR_REQUEST_OUT,
        0x6,
        0x00,
        Offset,
        &Value,
        4);
    
    return Status;
}
#else
NTSTATUS    RTUSBWriteMACRegister(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  ULONG           Value)
{
    NTSTATUS    Status = NDIS_STATUS_SUCCESS;
#if 0   
    Status = RTUSBSingleWrite(pAd,Offset,(USHORT)(Value & 0xffff));
    if(!NT_SUCCESS(Status)) 
        return Status;
    Status = RTUSBSingleWrite(pAd,Offset + 2,(USHORT) ((Value & 0xffff0000) >> 16));
#endif  

    return Status;
}




//
// Write four bytes data to the specified MAC register.
//
/*NTSTATUS
RTUSBWriteMACRegister(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  ULONG           Value
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    if (KeGetCurrentIrql() > PASSIVE_LEVEL)
    {
        //
        //Asyn I/O
        //
        switch (Offset)
        {
            case MAC_ADDR_DW0:
            case MAC_ADDR_DW1:
            case MAC_BSSID_DW0:
            case MAC_BSSID_DW1:
            case MAC_WCID_BASE + BSSID_WCID * HW_WCID_ENTRY_SIZE:   //WCID Search table
            case MAC_WCID_BASE + BSSID_WCID * HW_WCID_ENTRY_SIZE + 4:
            case BCN_TIME_CFG:  //MtAsicEnableIbssSync, MtAsicEnableBssSync
            case TBTT_SYNC_CFG: //MtAsicEnableIbssSync
                HwUsbSendVendorControlPacketAsync(pAd, 0x06, 0, sizeof(ULONG), &Value, Offset);
                break;

            default:
                ASSERT(FALSE);
                DBGPRINT (RT_DEBUG_TRACE, ("Async. Control Write Not supported !! OffSet[%x] Value %x \n", Offset, Value));
                break;
        }
    }
    else
    {
        Status = N6UsbIoVendorRequest(
            pAd,
            RTUSB_TRANSFER_DIRECTION_OUT,
            DEVICE_VENDOR_REQUEST_OUT,
            0x6,
            0x00,
            Offset,
            &Value,
            4);
    }
    return Status;
}*/

#endif

#endif

#if 0
/*
    ========================================================================
    
    Routine Description: Read 8-bit BBP register

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBReadBBPRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Id,
    IN  PUCHAR          pValue)
{
    BBP_CSR_CFG_STRUC   BbpCsr;
    ULONG           temp;
    UINT            i = 0;

    // Verify the busy condition
    do
    {
        RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &BbpCsr.word);
        if (!(BbpCsr.field.Busy == BUSY))
            break;
        i++;
    }
    while ((i < RETRY_LIMIT) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
    
    if ((i == RETRY_LIMIT) || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        //
        // Read failed then Return Default value.
        //
        *pValue = pAd->HwCfg.BbpWriteLatch[Id];
    
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
        return STATUS_UNSUCCESSFUL;
    }

    // Prepare for write material
    BbpCsr.word                 = 0;
    BbpCsr.field.fRead          = 1;
    BbpCsr.field.Busy           = 1;
    BbpCsr.field.RegNum         = Id;
    RTUSBWriteMACRegister(pAd, BBP_CSR_CFG, BbpCsr.word);

    i = 0;  
    // Verify the busy condition
    do
    {
        RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &BbpCsr.word);
        if (!(BbpCsr.field.Busy == BUSY))
        {
            *pValue = (UCHAR)BbpCsr.field.Value;
            break;
        }
        i++;
    }
    while ((i < RETRY_LIMIT) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
    
    if ((i == RETRY_LIMIT) || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        //
        // Read failed then Return Default value.
        //
        *pValue = pAd->HwCfg.BbpWriteLatch[Id];

        DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
        return STATUS_UNSUCCESSFUL;
    }
    
    return STATUS_SUCCESS;
}
#else
/*
    ========================================================================
    
    Routine Description: Read 8-bit BBP register by firmware

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBReadBBPRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Id,
    IN  PUCHAR          pValue)
{
    BBP_CSR_CFG_STRUC   BbpCsr;
    int                 i, k;
#if DBG
    ULONG uValue;
#endif

    USHORT  MacAddess;

    MacAddess = H2M_BBP_AGENT;

    //prevent wait  too long  in S5  state
    if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
        return STATUS_UNSUCCESSFUL;

    // Acquire Control token
    while (1)
    {
        NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
        if (pAd->pHifCfg->BBPControlPending == FALSE)
        {
            pAd->pHifCfg->BBPControlPending = TRUE;
            NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);
            break;
        }
        else
        {
            NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);
            
            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                return (STATUS_DEVICE_NOT_CONNECTED);   
            DBGPRINT_ERR(("****** BBP busy now**** at Line#%d\n", __LINE__));
            NdisCommonGenericDelay(10);
        }
    }
    
    BbpCsr.word = 0;
    for (i = 0; i < MAX_BUSY_COUNT; i++)
    {
        RTUSBReadMACRegister(pAd, MacAddess, &BbpCsr.word);

        if (BbpCsr.field.Busy == BUSY)
        {
            if(i >= 10)
            {
                pAd->BBPBusycnt++;
                DBGPRINT_ERR(("BBP read busy Over 10 times at Line#%d\n", __LINE__));
                //prevent wait  too long  in S5  state
                if(pAd->bInShutdown&& (pAd->Shutdowncnt == 0))
                {
                    NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
                    pAd->pHifCfg->BBPControlPending = FALSE;
                    NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);

                    return STATUS_UNSUCCESSFUL;
                }
            }
            else
            {
                continue;
            }
        }
        
        BbpCsr.word = 0;
        BbpCsr.field.fRead = 1;
        BbpCsr.field.BBP_RW_MODE = 1;
        BbpCsr.field.Busy = 1;
        BbpCsr.field.RegNum = Id;

        RTUSBWriteMACRegister(pAd, MacAddess, BbpCsr.word);
        
        AsicSendCommanToMcu(pAd, 0x80, 0xff, 0x0, 0x0);
        for (k = 0; k < MAX_BUSY_COUNT; k++)
        {
            RTUSBReadMACRegister(pAd, MacAddess, &BbpCsr.word);

            if (BbpCsr.field.Busy == IDLE)
            {
                break;
            }
            else
            {
                if(k >= 10)
                {
                    pAd->BBPBusycnt++;
                    DBGPRINT_ERR(("BBP read busy Over 10 times at Line#%d\n", __LINE__));
                    //prevent wait  too long  in S5  state
                    if(pAd->bInShutdown&& (pAd->Shutdowncnt == 0))
                    {
                        NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
                            pAd->pHifCfg->BBPControlPending = FALSE;
                        NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);

                        return STATUS_UNSUCCESSFUL;
                    }
                    break;
                }
            }
        }
        if ((BbpCsr.field.Busy == IDLE) &&
            (BbpCsr.field.RegNum == Id))
        {
            *pValue = (UCHAR)BbpCsr.field.Value;
            break;
        }
    }
    if (BbpCsr.field.Busy == BUSY)
    {
#if DBG
        RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &uValue);
        DBGPRINT_ERR(("BBP read R%d=0x%x fail 101c=0x%x\n", Id, BbpCsr.word,uValue));
#endif
        *pValue = pAd->HwCfg.BbpWriteLatch[Id];

        NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
        pAd->pHifCfg->BBPControlPending = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);

        return STATUS_UNSUCCESSFUL;
    }

    NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
    pAd->pHifCfg->BBPControlPending = FALSE;
    NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);

    return STATUS_SUCCESS;
}
#endif

#if 0
/*
    ========================================================================
    
    Routine Description: Write 8-bit BBP register

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBWriteBBPRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Id,
    IN  UCHAR           Value)
{
    BBP_CSR_CFG_STRUC   BbpCsr;
    USHORT          temp;
    UINT            i = 0;

    // Verify the busy condition
    if (Id >= MAX_NUM_OF_BBP_LATCH)
    {
        DBGPRINT_ERR(("****** BBP_Write_Latch Buffer (id=0x%d) exceeds max boundry ****** \n",Id));
        return STATUS_UNSUCCESSFUL;
    }
    
    do
    {
        RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &BbpCsr.word);
        if (!(BbpCsr.field.Busy == BUSY))
            break;
        i++;
    }
    while ((i < RETRY_LIMIT) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
    
    if ((i == RETRY_LIMIT) || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
        return STATUS_UNSUCCESSFUL;
    }

    // Prepare for write material
    BbpCsr.word                 = 0;
    BbpCsr.field.fRead          = 0;
    BbpCsr.field.Value          = Value;
    BbpCsr.field.Busy           = 1;
    BbpCsr.field.RegNum         = Id;
    RTUSBWriteMACRegister(pAd, BBP_CSR_CFG, BbpCsr.word);
    
    pAd->HwCfg.BbpWriteLatch[Id] = Value;

    return STATUS_SUCCESS;
}
#else

/*
    ========================================================================
    
    Routine Description: Write 8-bit BBP register by firmware

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/

NTSTATUS    RTUSBWriteBBPRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Id,
    IN  UCHAR           Value)

{
    BBP_CSR_CFG_STRUC   BbpCsr;
    int                 BusyCnt;
#if DBG
    ULONG               uValue;
#endif

    USHORT  MacAddess;

    MacAddess = H2M_BBP_AGENT;


    if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
        return STATUS_UNSUCCESSFUL;

    if (Id >= MAX_NUM_OF_BBP_LATCH)
    {
        DBGPRINT_ERR(("****** BBP_Write_Latch Buffer (id=0x%d) exceeds max boundry ****** \n",Id));
        return STATUS_UNSUCCESSFUL;
    }

    // Acquire Control token
    while (1)
    {
        NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
        if (pAd->pHifCfg->BBPControlPending == FALSE)
        {
            pAd->pHifCfg->BBPControlPending = TRUE;
            NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);
            break;
        }
        else
        {
            NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);
            
            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                return (STATUS_DEVICE_NOT_CONNECTED);   
            //DBGPRINT_ERR(("****** BBP busy now**** \n"));
            NdisCommonGenericDelay(10);
        }
    }

#if DBG
    //
    // for check build void warning uninitialized local variable 'BbpCsr'
    //
    BbpCsr.word = 0;
#endif

    for (BusyCnt = 0; BusyCnt < MAX_BUSY_COUNT; BusyCnt++)
    {

        // 7601 no 8051 to transfer 0x7028, so we direct access 0x101c
        RTUSBReadMACRegister(pAd, MacAddess, &BbpCsr.word);

        if (BbpCsr.field.Busy == BUSY)
        {
            if(BusyCnt >= 10)
            {
                pAd->BBPBusycnt++;
                DBGPRINT_ERR(("BBP write busy Over 10 times\n"));
                //prevent wait  too long  in S5  state
                if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
                {
                    NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
                        pAd->pHifCfg->BBPControlPending = FALSE;
                    NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);

                    return STATUS_UNSUCCESSFUL;
                }
            }
            else
            {
                continue;
            }
        }
        BbpCsr.word = 0;
        BbpCsr.field.fRead = 0;
        BbpCsr.field.BBP_RW_MODE = 1;
        BbpCsr.field.Busy = 1;
        BbpCsr.field.Value = Value;
        BbpCsr.field.RegNum = Id;

        RTUSBWriteMACRegister(pAd, MacAddess, BbpCsr.word);
        AsicSendCommanToMcu(pAd, 0x80, 0xff, 0x0, 0x0);
        pAd->HwCfg.BbpWriteLatch[Id] = Value;
        break;
    }
    if (BusyCnt == MAX_BUSY_COUNT)
    {
#if DBG
        RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &uValue);
        DBGPRINT_ERR(("BBP write R%d=0x%x fail,101c = 0x%x\n", Id, BbpCsr.word,uValue));
#endif

        NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
        pAd->pHifCfg->BBPControlPending = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);

        return STATUS_UNSUCCESSFUL;
    }


    NdisAcquireSpinLock(&pAd->pHifCfg->BBPControlLock);
    pAd->pHifCfg->BBPControlPending = FALSE;
    NdisReleaseSpinLock(&pAd->pHifCfg->BBPControlLock);

    return STATUS_SUCCESS;
}
#endif


/*
    ========================================================================
    
    Routine Description: Write RF register through MAC

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBWriteRFRegister(
    IN  PMP_ADAPTER   pAd,
    IN  ULONG           Value)
{
    PHY_CSR4_STRUC  PhyCsr4;
    UINT            i = 0;

    if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
        return STATUS_UNSUCCESSFUL;
    
    PhyCsr4.word = 0;

    do
    {
        RTUSBReadMACRegister(pAd, RF_CSR_CFG0, &PhyCsr4.word);
        if (!(PhyCsr4.field.Busy))
            break;
        i++;
    }
    while ((i < RETRY_LIMIT) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

    if ((i == RETRY_LIMIT) || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
        return STATUS_UNSUCCESSFUL;
    }

    RTUSBWriteMACRegister(pAd, RF_CSR_CFG0, Value);
    
    return STATUS_SUCCESS;
}

/*
    ========================================================================
    
    Routine Description: Write RT3070 RF register through MAC

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RT30xxWriteRFRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RegID,
    IN  ULONG           Value)
{
    RF_CSR_CFG_STRUC    rfcsr = {0};
    UINT                i = 0;
    RF_CSR_CFG_EXT_STRUC RfCsrCfgExt = {0};

#ifdef DBG

    if (RF_BANK_SELECTION_CAPABLE(pAd))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: ***************************************************\n", __FUNCTION__));
        
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NIC supports RF bank selection, MACVersion = 0x%08X\n", 
            __FUNCTION__, 
            pAd->HwCfg.MACVersion));

        DBGPRINT(RT_DEBUG_ERROR, ("%s: Call the WriteRfRegExt funciton instead\n", __FUNCTION__));

        DBGPRINT(RT_DEBUG_ERROR, ("%s: ***************************************************\n", __FUNCTION__));


        ASSERT(FALSE);

        return STATUS_UNSUCCESSFUL;
    }

#endif // DBG

        if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
            return STATUS_UNSUCCESSFUL;
        
        rfcsr.word = 0;

            ASSERT((RegID >= 0) && (RegID <= 31)); // R0~R31
            
            do
            {
                RTUSBReadMACRegister(pAd, RF_CSR_CFG, &rfcsr.word);

                if (!rfcsr.field.RF_CSR_KICK)
                {
                    break;
                }
                i++;
            }
            while ((i < RETRY_LIMIT) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
                ; // do nothing

            if ((i == RETRY_LIMIT) || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
            {
                DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
                return STATUS_UNSUCCESSFUL;
            }

            rfcsr.field.RF_CSR_WR = 1;
            rfcsr.field.RF_CSR_KICK = 1;
            rfcsr.field.TESTCSR_RFACC_REGNUM = RegID; // R0~R31
            rfcsr.field.RF_CSR_DATA = Value;
            
            RTUSBWriteMACRegister(pAd, RF_CSR_CFG, rfcsr.word);
    return STATUS_SUCCESS;
}

/*
    ========================================================================
    
    Routine Description: Read RT3070 RF register through MAC

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RT30xxReadRFRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RegID,
    IN  PUCHAR          pValue)
{
    RF_CSR_CFG_STRUC    rfcsr = {0};
    UINT                i, k = 0;
    RF_CSR_CFG_EXT_STRUC RfCsrCfgExt = {0};
    
#ifdef DBG
    
    if (RF_BANK_SELECTION_CAPABLE(pAd))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: ***************************************************\n", __FUNCTION__));
        
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NIC supports RF bank selection, MACVersion = 0x%08X\n", 
            __FUNCTION__, 
            pAd->HwCfg.MACVersion));

        DBGPRINT(RT_DEBUG_ERROR, ("%s: Call the ReadRfRegExt funciton instead\n", __FUNCTION__));

        DBGPRINT(RT_DEBUG_ERROR, ("%s: ***************************************************\n", __FUNCTION__));


        ASSERT(FALSE);

        return STATUS_UNSUCCESSFUL;
    }
    
#endif // DBG

    //2 TODO: Emulation only
    {
        if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
            return STATUS_UNSUCCESSFUL;
        
        rfcsr.word = 0;
        
        ASSERT((RegID >= 0) && (RegID <= 31)); // R0~R31
        for (i = 0; i < MAX_BUSY_COUNT; i++)
        {
            RTUSBReadMACRegister(pAd, RF_CSR_CFG, &rfcsr.word);

            if (rfcsr.field.RF_CSR_KICK == BUSY)
            {
                continue;
            }
            rfcsr.word = 0;
            rfcsr.field.RF_CSR_WR = 0;
            rfcsr.field.RF_CSR_KICK = 1;
            rfcsr.field.TESTCSR_RFACC_REGNUM = RegID;// R0~R31
            RTUSBWriteMACRegister(pAd, RF_CSR_CFG, rfcsr.word);
            for (k = 0; k < MAX_BUSY_COUNT; k++)
            {
                RTUSBReadMACRegister(pAd, RF_CSR_CFG, &rfcsr.word);

                if (rfcsr.field.RF_CSR_KICK == IDLE)
                {
                    break;
                }
            }
            if ((rfcsr.field.RF_CSR_KICK == IDLE) &&
                (rfcsr.field.TESTCSR_RFACC_REGNUM == RegID))
            {
                *pValue = (UCHAR)rfcsr.field.RF_CSR_DATA;
                break;
            }
        }
        if (rfcsr.field.RF_CSR_KICK == BUSY)
        {
            DBGPRINT_ERR(("RF read R%d=0x%x fail, i[%d], k[%d]\n", RegID, rfcsr.word,i,k));
            return STATUS_UNSUCCESSFUL;
        }
    }
    return STATUS_SUCCESS;
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID eFusePhysicalReadRegisters( 
    IN  PMP_ADAPTER   pAd, 
    IN  USHORT Offset, 
    IN  USHORT Length, 
    OUT USHORT* pData)
{
    EFUSE_CTRL_STRUC        eFuseCtrlStruc;
    int                     i;
    USHORT                  efuseDataOffset;
    ULONG                   data;

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        return;

    RTUSBReadMACRegister(pAd, EFUSE_CTRL, (PULONG) &eFuseCtrlStruc.word);

    //Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.
    eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

    //Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.
    eFuseCtrlStruc.field.EFSROM_MODE = 1;

    //Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.
    eFuseCtrlStruc.field.EFSROM_KICK = 1;

    PlatformMoveMemory(&data, &eFuseCtrlStruc.word, 4); 
    RTUSBWriteMACRegister(pAd, EFUSE_CTRL, data);   

    //Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.
    i = 0;
    while ((i < 100) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {   
        RTUSBReadMACRegister(pAd, EFUSE_CTRL, (PULONG) &eFuseCtrlStruc.word);   
        if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
            break;
        Delay_us(100);
        i++;
    }

    //Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)
    efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC)  ;  

    RTUSBReadMACRegister(pAd, efuseDataOffset, &data);

    data = data >> (8*(Offset & 0x3));

    PlatformMoveMemory(pData, &data, Length);
    
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
UCHAR eFuseReadRegisters(
    IN  PMP_ADAPTER   pAd, 
    IN  USHORT Offset, 
    IN  USHORT Length, 
    OUT USHORT* pData)
{
    EFUSE_CTRL_STRUC    eFuseCtrlStruc;
    int                 i;
    USHORT              efuseDataOffset;
    ULONG               data;
    ULONG   uEFUSE_CTRL = EFUSE_CTRL;
    USHORT  uEFUSE_DATA3 = EFUSE_DATA3;
    
    DBGPRINT(RT_DEBUG_TRACE, ("--> eFuseReadRegisters Offset = %d\n",Offset));
   
    RTUSBReadMACRegister(pAd, (USHORT)uEFUSE_CTRL, (PULONG) &eFuseCtrlStruc.word);

    //Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.
    eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

    //Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 0.
    eFuseCtrlStruc.field.EFSROM_MODE = 0;

    //Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.
    eFuseCtrlStruc.field.EFSROM_KICK = 1;
    
    PlatformMoveMemory(&data, &eFuseCtrlStruc.word, 4);
    RTUSBWriteMACRegister(pAd, (USHORT)uEFUSE_CTRL, data);

    //Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.
    i = 0;
    while ((i < 100) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {   
        RTUSBReadMACRegister(pAd, (USHORT)uEFUSE_CTRL, (PULONG) &eFuseCtrlStruc.word);
        if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
        {
            break;
        }   
        Delay_us(100);
        i++;    
    }

    //if EFSROM_AOUT is not found in physical address, write 0xffff
    if (eFuseCtrlStruc.field.EFSROM_AOUT == 0x3f)
    {
        for(i = 0; i < Length/2; i++)
            *(pData+2*i) = 0xffff;
    }
    else
    {
        efuseDataOffset =  uEFUSE_DATA3 - (Offset & 0xC);
        RTUSBReadMACRegister(pAd, efuseDataOffset, &data);
        data = data >> (8*(Offset & 0x3));      
        PlatformMoveMemory(pData, &data, Length);
    }

    return (UCHAR) eFuseCtrlStruc.field.EFSROM_AOUT;
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS eFuseRead(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    OUT PUCHAR          pData,
    IN  USHORT          Length)
{
    USHORT* pOutBuf = (USHORT*)pData;
    NTSTATUS    Status = STATUS_SUCCESS;
    UCHAR   EFSROM_AOUT;
    int i;
    
    for (i = 0; i < Length; i += 2)
    {
        EFSROM_AOUT = eFuseReadRegisters(pAd, Offset + i, 2, &pOutBuf[i / 2]);
    }
    
    return Status;
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID eFuseReadPhysical( 
    IN  PMP_ADAPTER   pAd, 
    IN  PUSHORT lpInBuffer,
    IN  ULONG nInBufferSize,
    OUT PUSHORT lpOutBuffer,
    IN  ULONG nOutBufferSize)
{
    USHORT* pInBuf = (USHORT*)lpInBuffer;
    USHORT* pOutBuf = (USHORT*)lpOutBuffer;

    USHORT Offset = pInBuf[0];                  //addr
    USHORT Length = pInBuf[1];                  //length
    int         i;
    
    for(i = 0; i < Length; i+=2)
    {
        eFusePhysicalReadRegisters(pAd,Offset+i, 2, &pOutBuf[i/2]); 
    }   
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID eFusePhysicalWriteRegisters(
    IN  PMP_ADAPTER   pAd,    
    IN  USHORT Offset, 
    IN  USHORT Length, 
    OUT USHORT* pData)
{
    EFUSE_CTRL_STRUC    eFuseCtrlStruc;
    int                 i;
    USHORT              efuseDataOffset;
    ULONG               data, eFuseDataBuffer[4];

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        return;

    //Step0. Write 16-byte of data to EFUSE_DATA0-3 (0x590-0x59C), where EFUSE_DATA0 is the LSB DW, EFUSE_DATA3 is the MSB DW.

    /////////////////////////////////////////////////////////////////
    //read current values of 16-byte block  
    RTUSBReadMACRegister(pAd, EFUSE_CTRL, (PULONG) &eFuseCtrlStruc.word);

    //Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.
    eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

    //Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.
    eFuseCtrlStruc.field.EFSROM_MODE = 1;

    //Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.
    eFuseCtrlStruc.field.EFSROM_KICK = 1;

    PlatformMoveMemory(&data, &eFuseCtrlStruc.word, 4);
    RTUSBWriteMACRegister(pAd, EFUSE_CTRL, data);   

    //Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.
    i = 0;
    while ((i < 100) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {   
        RTUSBReadMACRegister(pAd, EFUSE_CTRL, (PULONG) &eFuseCtrlStruc.word);

        if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
            break;
        Delay_us(100);
        i++;    
    }

    //Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)
    efuseDataOffset =  EFUSE_DATA3;     
    for(i = 0; i < 4; i++)
    {
        RTUSBReadMACRegister(pAd, efuseDataOffset, (PULONG) &eFuseDataBuffer[i]);
        efuseDataOffset -=  4;      
    }

    //Update the value, the offset is multiple of 2, length is 2
    efuseDataOffset = (Offset & 0xc) >> 2;
    data = pData[0] & 0xffff;
    if((Offset % 4) != 0)
    {
        eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff) | (data << 16);
    }
    else
    {
        eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff0000) | data;
    }

    efuseDataOffset =  EFUSE_DATA3;
    for(i = 0; i < 4; i++)
    {
        RTUSBWriteMACRegister(pAd, efuseDataOffset, eFuseDataBuffer[i]);            
        efuseDataOffset -= 4;       
    }
    /////////////////////////////////////////////////////////////////

    //Step1. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.
    eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

    //Step2. Write EFSROM_MODE (0x580, bit7:bit6) to 3.
    eFuseCtrlStruc.field.EFSROM_MODE = 3;
    
    //Step3. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical write procedure.
    eFuseCtrlStruc.field.EFSROM_KICK = 1;

    PlatformMoveMemory(&data, &eFuseCtrlStruc.word, 4); 
    RTUSBWriteMACRegister(pAd, EFUSE_CTRL, data);   

    //Step4. Polling EFSROM_KICK(0x580, bit30) until it become 0 again. It��s done.
    i = 0;
    while ((i < 100) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {   
        RTUSBReadMACRegister(pAd, EFUSE_CTRL, (PULONG) &eFuseCtrlStruc.word);

        if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
            break;
        
        Delay_us(100); 
        i++;    
    }
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID eFuseWritePhysical( 
    IN  PMP_ADAPTER   pAd,    
    PUSHORT lpInBuffer,
    ULONG nInBufferSize,    
    PUCHAR lpOutBuffer,     
    ULONG nOutBufferSize)   
{
    USHORT* pInBuf = (USHORT*)lpInBuffer;
    int         i;
    //USHORT* pOutBuf = (USHORT*)ioBuffer;

    USHORT Offset = pInBuf[0];                  //addr
    USHORT Length = pInBuf[1];                  //length
    USHORT* pValueX = &pInBuf[2];               //value ...     
    
    for(i=0; i<Length; i+=2)
    {
        eFusePhysicalWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);   
    }   
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS eFuseWriteRegisters(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT Offset, 
    IN  USHORT Length, 
    IN  USHORT* pData)
{
    USHORT  i, StartBlock, EndBlock;
    USHORT  eFuseData;
    USHORT  LogicalAddress, BlkNum = 0xffff;
    UCHAR   EFSROM_AOUT, NrFreeBlk;
    BOOLEAN     bUpdateBlock = FALSE;

    USHORT addr,tmpaddr, InBuf[10], tmpOffset;
    USHORT buffer[8];
    BOOLEAN     bWriteSuccess = TRUE;
    NTSTATUS    Status = STATUS_SUCCESS;
    USHORT RetryCount;

    DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters Offset=%x, pData=%x\n", Offset, *pData));

    //set start block and end block number, start from tail of mapping table
    if( (pAd->HwCfg.eFuseConfig.MapEnd % 2) != 0)
    {
        StartBlock = pAd->HwCfg.eFuseConfig.MapEnd-1; 
    }
    else
    {
        StartBlock = pAd->HwCfg.eFuseConfig.MapEnd; 
    }

    if( (pAd->HwCfg.eFuseConfig.MapStart % 2) != 0)
    {
        EndBlock = pAd->HwCfg.eFuseConfig.MapStart-1; 
    }
    else
    {
        EndBlock = pAd->HwCfg.eFuseConfig.MapStart; 
    }

    //Step 0. find the entry in the mapping table
    tmpOffset = Offset & 0xfffe;
    EFSROM_AOUT = eFuseReadRegisters(pAd, tmpOffset, 2, &eFuseData);
    if( EFSROM_AOUT == 0x3f)
    {       
        for (i=StartBlock; i >= EndBlock; i-=2)
        {
            eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);

            if(( (LogicalAddress >> 8) & 0xff) == 0)
            {
                if(i != pAd->HwCfg.eFuseConfig.MapEnd)
                {
                    BlkNum = i+1-pAd->HwCfg.eFuseConfig.MapStart;
                    break;
                }   
            }

            if( (LogicalAddress & 0xff) == 0)
            {
                if(i != (pAd->HwCfg.eFuseConfig.MapStart-1))
                {
                    BlkNum = i-pAd->HwCfg.eFuseConfig.MapStart;
                    break;
                }
            }
        }
    }
    else
    {
        BlkNum = EFSROM_AOUT;

        //We found the block, check if it is the free block number is 0. If free block number is 0 , we compare the data writen and read back from the block is equal or not. 
        //We will write to Efuse only the values are equal, otherwise return and not update the value.
        eFuseGetFreeBlockCount(pAd, &NrFreeBlk);
        if(NrFreeBlk == 0)
        {
            addr = BlkNum * 0x10 ;

            if( ((Offset % 16) == 0) && (Length == 16) )
            {
                //Efuse write by block case
                for(i = 0; i < 16; i+=2)
                {
                    InBuf[0] = addr+( (Offset+i) % 16);
                    InBuf[1] = 2;
                    InBuf[2] = 0;   

                    eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

                    if( (pData[i/2] | InBuf[2]) != pData[i/2])
                        return STATUS_UNSUCCESSFUL;
                }
            }
            else
            {
                InBuf[0] = addr+(Offset % 16);
                InBuf[1] = 2;
                InBuf[2] = 0;   

                eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

                if( (pData[0] | InBuf[2]) != pData[0])
                    return STATUS_UNSUCCESSFUL;
            }   
        }
    }   

    DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters BlkNum = %d \n", BlkNum));

    if(BlkNum == 0xffff)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
        return STATUS_UNSUCCESSFUL;
    }   

    if( ((Offset % 16) == 0) && (Length == 16) )
    {
        //Efuse write by block case
        PlatformMoveMemory(buffer, pData, 16);
    }
    else
    {
        //Step 1. Save data of this block   
        // read and save the original block data
        for(i =0; i<8; i++)
        {
            addr = BlkNum * 0x10 ;
            
            InBuf[0] = addr+2*i;
            InBuf[1] = 2;
            InBuf[2] = 0x0; 
            
            eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

            buffer[i] = InBuf[2];
        }

        //Step 2. Update the data in buffer, and write the data to Efuse
        buffer[ (Offset >> 1) % 8] = pData[0];
    }   

    RetryCount = 0;
    do
    {   
        //Step 3. Write the data to Efuse
        if( ((Offset % 16) == 0) && (Length == 16) )
        {
            DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters by block: Offset=0x%x\n", Offset));

            //Efuse write by block case
            addr = BlkNum * 0x10 ;

            InBuf[0] = addr;
            InBuf[1] = Length;
            PlatformMoveMemory(&InBuf[2], buffer, 16);
            eFuseWritePhysical(pAd, &InBuf[0], 4+16, NULL, 2);  
        }   
        else
        {
            if(!bWriteSuccess)
            {
                for(i =0; i<8; i++)
                {
                    addr = BlkNum * 0x10 ;
                    
                    InBuf[0] = addr+2*i;
                    InBuf[1] = 2;
                    InBuf[2] = buffer[i];   
                    
                    eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);     
                }
            }
            else
            {
                    addr = BlkNum * 0x10 ;
                    
                    InBuf[0] = addr+(Offset % 16);
                    InBuf[1] = 2;
                    InBuf[2] = pData[0];    
                    
                    eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2); 
            }
        }
    
        //Step 4. Write mapping table
        addr = pAd->HwCfg.eFuseConfig.MapStart+BlkNum;

        tmpaddr = addr;

        if(addr % 2 != 0)
            addr = addr -1; 
        InBuf[0] = addr;
        InBuf[1] = 2;

        //convert the address from 10 to 8 bit ( bit7, 6 = parity and bit5 ~ 0 = bit9~4), and write to logical map entry
        tmpOffset = Offset;
        tmpOffset >>= 4;
        tmpOffset |= ((~(tmpOffset & 0x01 ^ ( tmpOffset >> 1 & 0x01) ^  (tmpOffset >> 2 & 0x01) ^  (tmpOffset >> 3 & 0x01))) << 6) & 0x40;
        tmpOffset |= ((~( (tmpOffset >> 2 & 0x01) ^ (tmpOffset >> 3 & 0x01) ^ (tmpOffset >> 4 & 0x01) ^ ( tmpOffset >> 5 & 0x01))) << 7) & 0x80;

        // write the logical address
        if(tmpaddr%2 != 0)  
            InBuf[2] = tmpOffset<<8;    
        else          
            InBuf[2] = tmpOffset;

        eFuseWritePhysical(pAd,&InBuf[0], 6, NULL, 0);

        //Step 5. Compare data if not the same, invalidate the mapping entry, then re-write the data until E-fuse is exhausted
        bWriteSuccess = TRUE;
        for(i =0; i<8; i++)
        {
            addr = BlkNum * 0x10 ;
            
            InBuf[0] = addr+2*i;
            InBuf[1] = 2;
            InBuf[2] = 0x0; 
            
            eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

            if(buffer[i] != InBuf[2])
            {
                bWriteSuccess = FALSE;
                break;
            }   
        }

        //Step 6. invlidate mapping entry and find a free mapping entry if not succeed
        if (!bWriteSuccess)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess BlkNum = %d\n", BlkNum));

#if 0
            if(BlkNum >= (EFUSE_USAGE_MAP_SIZE-1))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
                return FALSE;
            }   
#endif          
            // the offset of current mapping entry
            addr = pAd->HwCfg.eFuseConfig.MapStart+BlkNum;            

            //find a new mapping entry
            BlkNum = 0xffff;
            for (i=StartBlock; i >= EndBlock; i-=2)
            {
                eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);         

                if(( (LogicalAddress >> 8) & 0xff) == 0)
                {
                    if(i != pAd->HwCfg.eFuseConfig.MapEnd)
                    {
                        BlkNum = i+1-pAd->HwCfg.eFuseConfig.MapStart;
                        break;
                    }   
                }

                if( (LogicalAddress & 0xff) == 0)
                {
                    if(i != (pAd->HwCfg.eFuseConfig.MapStart-1))
                    {
                        BlkNum = i-pAd->HwCfg.eFuseConfig.MapStart;
                        break;
                    }
                }
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess new BlkNum = %d\n", BlkNum));  
            if(BlkNum == 0xffff)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
                return STATUS_UNSUCCESSFUL;
            }

            //invalidate the original mapping entry if new entry is not found
            tmpaddr = addr;

            if(addr % 2 != 0)
                addr = addr -1; 
            InBuf[0] = addr;
            InBuf[1] = 2;       
            
            eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);             

            // write the logical address
            if(tmpaddr%2 != 0) 
            {
                // Invalidate the high byte
                for (i=8; i<15; i++)
                {
                    if( ( (InBuf[2] >> i) & 0x01) == 0)
                    {
                        InBuf[2] |= (0x1 <<i);
                        break;
                    }   
                }       
            }   
            else
            {
                // invalidate the low byte
                for (i=0; i<8; i++)
                {
                    if( ( (InBuf[2] >> i) & 0x01) == 0)
                    {
                        InBuf[2] |= (0x1 <<i);
                        break;
                    }   
                }                   
            }
            eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 0); 
        }   
        RetryCount++;
    }   
    while(!bWriteSuccess && (RetryCount < 5));  

    return Status;
}

VOID eFuseGetFreeBlockCount(  
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pFreeBlock)
{
    USHORT i, StartBlock, EndBlock;
    USHORT  LogicalAddress;
    USHORT  FirstFreeBlock = 0xffff, LastFreeBlock = 0xffff;

    // find first free block
    if( (pAd->HwCfg.eFuseConfig.MapStart % 2) != 0)
    {
        StartBlock = pAd->HwCfg.eFuseConfig.MapStart-1; 
    }
    else
    {
        StartBlock = pAd->HwCfg.eFuseConfig.MapStart; 
    }

    if( (pAd->HwCfg.eFuseConfig.MapEnd % 2) != 0)
    {
        EndBlock = pAd->HwCfg.eFuseConfig.MapEnd-1; 
    }
    else
    {
        EndBlock = pAd->HwCfg.eFuseConfig.MapEnd; 
    }

    for (i = StartBlock; i <= EndBlock; i+=2)
    {
        eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
        
        if( (LogicalAddress & 0xff) == 0)
        {
            if(i != (pAd->HwCfg.eFuseConfig.MapStart-1))
            {
                FirstFreeBlock = i;
                break;
            }
        }       

        if(( (LogicalAddress >> 8) & 0xff) == 0)
        {
            if(i != pAd->HwCfg.eFuseConfig.MapEnd)
            {
                FirstFreeBlock = i+1;
                break;
            }   
        }           
    }

    DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, FirstFreeBlock= 0x%x\n", FirstFreeBlock));

    //if not find, return free block number = 0
    if(FirstFreeBlock == 0xffff)
    {
        *pFreeBlock = 0;
        return; 
    }

    // find last free block
    if( (pAd->HwCfg.eFuseConfig.MapEnd % 2) != 0)
    {
        StartBlock = pAd->HwCfg.eFuseConfig.MapEnd-1; 
    }
    else
    {
        StartBlock = pAd->HwCfg.eFuseConfig.MapEnd; 
    }

    if( (pAd->HwCfg.eFuseConfig.MapStart % 2) != 0)
    {
        EndBlock = pAd->HwCfg.eFuseConfig.MapStart-1; 
    }
    else
    {
        EndBlock = pAd->HwCfg.eFuseConfig.MapStart; 
    }

    for (i = StartBlock; i >= EndBlock; i-=2)
    {
        eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
                
        if(( (LogicalAddress >> 8) & 0xff) == 0)
        {
            if(i != pAd->HwCfg.eFuseConfig.MapEnd)
            {
                LastFreeBlock = i+1;
                break;
            }   
        }

        if( (LogicalAddress & 0xff) == 0)
        {
            if(i != (pAd->HwCfg.eFuseConfig.MapStart-1))
            {
                LastFreeBlock = i;
                break;
            }
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, LastFreeBlock= 0x%x\n", LastFreeBlock));

    //if not find last free block, return free block number = 0, this should not happen since we have checked first free block number previously
    if(LastFreeBlock == 0xffff)
    {
        *pFreeBlock = 0;
        return; 
    }

    // return total free block number, last free block number must >= first free block number
    if(LastFreeBlock < FirstFreeBlock)
    {
        *pFreeBlock = 0;    
    }
    else
    {
        *pFreeBlock = LastFreeBlock - FirstFreeBlock + 1;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, TotalFreeBlock= %d\n", *pFreeBlock));
    return; 
}

NTSTATUS eFuseWrite(  
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  PUCHAR          pData,
    IN  USHORT          length)
{
    int i;
    NTSTATUS    Status = STATUS_SUCCESS;

    USHORT* pValueX = (PUSHORT) pData;              //value ...     
        
    for(i=0; i<length; i+=2)
    {
        eFuseWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);   
    }

    return Status;
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBReadEEPROM(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    OUT PUCHAR          pData,
    IN  USHORT          length)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    
#if DBG
    //BJBJ temp: if ((pAd->TrackInfo.bExternalEEPROMImage == TRUE) && (pAd->HwCfg.buseEfuse == TRUE))
    if (pAd->TrackInfo.bExternalEEPROMImage == TRUE)
    {
        PlatformMoveMemory(pData, &(pAd->TrackInfo.EEPROMImage[Offset]), length);
    }
    else
#endif
    if ((pAd->HwCfg.buseEfuse) || (FlashInstallationWith8KBFirmware(pAd) == TRUE))
    {
        Status = eFuseRead(pAd, Offset, pData, length);
    }
    else
    {

        // For 7650E2, toggle before R/W and after R/W EEPROM
        if (EXT_EEPROM_PCIE_USB_CONCURRENT(pAd))
        {
            MISC_CTRL_STRUC RegMiscCtrl;
            RTUSBReadMACRegister(pAd, MISC_CTRL, &RegMiscCtrl.word);
            RegMiscCtrl.field.EEPROMLoc = TRUE;  // Read WLAN scope of EEPROM
            RTUSBWriteMACRegister(pAd, MISC_CTRL, RegMiscCtrl.word);            
        }

        // ========================================
        // Start to R/W EEPROM
        // ========================================
        Status = N6UsbIoVendorRequest(
            pAd,
            USBD_TRANSFER_DIRECTION_IN ,
            DEVICE_VENDOR_REQUEST_IN,
            0x9,
            0,
            Offset,
            pData,
            length);

        // ========================================
        // End of R/W EEPROM
        // ========================================
        
        // For 7650E2, toggle before R/W and after R/W EEPROM
        if (EXT_EEPROM_PCIE_USB_CONCURRENT(pAd))
        {
            MISC_CTRL_STRUC RegMiscCtrl;
            RTUSBReadMACRegister(pAd, MISC_CTRL, &RegMiscCtrl.word);
            RegMiscCtrl.field.EEPROMLoc = FALSE;  // Read WLAN scope of EEPROM
            RTUSBWriteMACRegister(pAd, MISC_CTRL, RegMiscCtrl.word);            
        }
    }
    
    return Status;
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSBWriteEEPROM(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  PUCHAR          pData,
    IN  USHORT          length)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    
        // For 7650E2, toggle before R/W and after R/W EEPROM
        if (EXT_EEPROM_PCIE_USB_CONCURRENT(pAd))
        {
            MISC_CTRL_STRUC RegMiscCtrl;
            RTUSBReadMACRegister(pAd, MISC_CTRL, &RegMiscCtrl.word);
            RegMiscCtrl.field.EEPROMLoc = TRUE;
            RTUSBWriteMACRegister(pAd, MISC_CTRL, RegMiscCtrl.word);            
        }

        // ========================================
        // Start to R/W EEPROM
        // ======================================== 
    Status = N6UsbIoVendorRequest(
        pAd,
        USBD_TRANSFER_DIRECTION_OUT,
        DEVICE_VENDOR_REQUEST_OUT,
        0x8,
        0,
        Offset,
        pData,
        length);

        // ========================================
        // End of R/W EEPROM
        // ========================================
        
        // For 7650E2, toggle before R/W and after R/W EEPROM
        if (EXT_EEPROM_PCIE_USB_CONCURRENT(pAd))
        {
            MISC_CTRL_STRUC RegMiscCtrl;
            RTUSBReadMACRegister(pAd, MISC_CTRL, &RegMiscCtrl.word);
            RegMiscCtrl.field.EEPROMLoc = FALSE;
            RTUSBWriteMACRegister(pAd, MISC_CTRL, RegMiscCtrl.word);            
        }

    return Status;
}

VOID
FreeQueueMemory(
    IN PCmdQElmt   cmdqelmt
    )
{
    if (cmdqelmt->CmdFromNdis == TRUE)
    {
        {
            if (cmdqelmt->buffer != NULL)
                PlatformFreeMemory(cmdqelmt->buffer, cmdqelmt->bufferlength);
        }

        PlatformFreeMemory(cmdqelmt, sizeof(CmdQElmt));
    }
    else
    {
        if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
            PlatformFreeMemory(cmdqelmt->buffer, cmdqelmt->bufferlength);

        PlatformFreeMemory(cmdqelmt, sizeof(CmdQElmt));
    }               
}
/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    RTUSBDequeueCmd(
    IN  PCmdQ       cmdq,
    OUT PCmdQElmt   *pcmdqelmt)
{
    *pcmdqelmt = cmdq->head;
    
    if (*pcmdqelmt != NULL)
    {
        cmdq->head = cmdq->head->next;
        cmdq->size--;
        if (cmdq->size == 0)
            cmdq->tail = NULL;
    }
}

//
// Send USB vendor request to underlying WDM USB driver.
//
NTSTATUS
N6UsbIoVendorRequest(
   IN   PMP_ADAPTER   pAd,
    IN  ULONG           Direction,
    IN  UCHAR           ReservedBits,
    IN  UCHAR           Request,
    IN  USHORT          Value,
    IN  USHORT          Index,
    IN  PVOID           TransferBuffer,
    IN  ULONG           TransferBufferLength
    )
{
    WDF_USB_CONTROL_SETUP_PACKET    controlSetupPacket;
    WDF_MEMORY_DESCRIPTOR           memDesc;
    WDF_REQUEST_SEND_OPTIONS        options;
    ULONG                           bytesTransferred;
    NTSTATUS                        ntStatus;
    UCHAR                           Number;

    UNREFERENCED_PARAMETER(ReservedBits);
    
    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeGetCurrentIrql() != PASSIVE_LEVEL, KeGetCurrentIrql() = %d\n", __FUNCTION__, KeGetCurrentIrql()));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    // Acquire Control token
    while (1)
    {
#if _WIN8_USB_SS_SUPPORTED
        NdisAcquireSpinLock(&pAd->Ss.SsLock);
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
        {
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
            return (STATUS_DEVICE_NOT_CONNECTED);
        }
#endif
        NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
        if (pAd->pHifCfg->ControlPending == FALSE)
        {
            pAd->pHifCfg->ControlPending = TRUE;
            NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
#if _WIN8_USB_SS_SUPPORTED
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif
            break;
        }
        else
        {
            NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
#if _WIN8_USB_SS_SUPPORTED
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif
            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                return (STATUS_DEVICE_NOT_CONNECTED);   
            
            NdisCommonGenericDelay(500);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - USB ControlPending=TRUE, Delay 500usec\n", __FUNCTION__));  
        }
    }
    // Check for correct level
    ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("IoBuildDeviceIoControlRequest Irq != PASSIVE_LEVEL. KeGetCurrentIrql()=%d\n", KeGetCurrentIrql()));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        // Release token
        NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
        pAd->pHifCfg->ControlPending = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
        return(ntStatus);
    }

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        // Release token
        NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
        pAd->pHifCfg->ControlPending = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);
        return (STATUS_DEVICE_NOT_CONNECTED);
    }

    WDF_USB_CONTROL_SETUP_PACKET_INIT_VENDOR(
                            &controlSetupPacket,
                            Direction,
                            RTUSB_RequestToDevice,
                            Request,
                            Value,
                            Index);

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
                            &memDesc, 
                            TransferBuffer, 
                            TransferBufferLength);


    WDF_REQUEST_SEND_OPTIONS_INIT(
                            &options,
                            WDF_REQUEST_SEND_OPTION_TIMEOUT);

    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
                            &options,
                            WDF_REL_TIMEOUT_IN_SEC(1));  // time out after one second

    Number = 0;

    do{

    ntStatus = WdfUsbTargetDeviceSendControlTransferSynchronously(
                            pAd->pHifCfg->UsbDevice, 
                            WDF_NO_HANDLE,              // Optional WDFREQUEST
                            &options,                       // PWDF_REQUEST_SEND_OPTIONS
                            &controlSetupPacket,
                            &memDesc,
                            &bytesTransferred);

    if(!NT_SUCCESS(ntStatus)) 
    {
            DBGPRINT(RT_DEBUG_ERROR, ("%s:WdfUsbTargetDeviceSendControlTransferSynchronously,request:0x%x, Failed - 0x%x ,Index = %x\n", __FUNCTION__, Request, ntStatus, Index));

        //
        // A device which does not exist was specified.
        //

        if ((ntStatus == STATUS_NO_SUCH_DEVICE) ||
            (ntStatus == STATUS_DEVICE_NOT_CONNECTED) ||
            (ntStatus == STATUS_DEVICE_REMOVED))
        {
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
            pAd->Counter.MTKCounters.VendorRequestFail++;
            PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_SUCCESS, TRUE, TRUE);
        }
            else if (ntStatus == STATUS_IO_TIMEOUT)
        {
            pAd->Counter.MTKCounters.VendorRequestTimeout++;
        }
    }
        else
        {
            break;
        }

        Number++;

    }while(Number < 5);

    // Release token
    NdisAcquireSpinLock(&pAd->pHifCfg->ControlLock);
    pAd->pHifCfg->ControlPending = FALSE;
    NdisReleaseSpinLock(&pAd->pHifCfg->ControlLock);

    return ntStatus;
}

NTSTATUS
MakeSynchronousIoctlWithTimeOutCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PLONG lock;

    lock = (PLONG) Context;
#pragma prefast(suppress: __WARNING_INVALID_PARAM_VALUE_1, " lock should not be NULL")              
    if (InterlockedExchange((PVOID)lock, IRPLOCK_COMPLETED) == IRPLOCK_CANCE_START) {
        // 
        // Main line code has got the control of the IRP. It will
        // now take the responsibility of completing the IRP. 
        // Therefore...
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    return STATUS_SUCCESS;
}

/*
    ========================================================================
    
    Routine Description:
        Creates and submits a URB (URB_FUNCTION_RESET_PIPE) to USBD to
        reset a pipe.   If the reset pipe fails then it calls USB_ResetDevice

    Arguments:
        Adapter     Pointer to VNET_ADAPTER structure
        PipeHandle  The handle of the pipe to be reset

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSB_ResetPipe(
    IN  PMP_ADAPTER   pAd, 
    IN  UINT            pipeIndex)
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PUSB_DEVICE_CONTEXT pUsbDeviceContext;
    USB_PIPE_TYPE bulkType = BULK_TYPE_UNKNOWN;

    DBGPRINT_RAW(RT_DEBUG_ERROR, ("-->USB : URB_FUNCTION_RESET_PIPE\n"));
    
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);

    ntStatus = WdfUsbTargetPipeResetSynchronously(
                    pUsbDeviceContext->BulkOutPipeHandle[pipeIndex], 
                    WDF_NO_HANDLE,
                                            NULL
                    );
    
    return ntStatus;
}

/*
    ========================================================================
    
    Routine Description:
        Creates an IRP to submite an IOCTL_INTERNAL_USB_RESET_PORT
        synchronously. Callers of this function must be running at
        PASSIVE LEVEL.

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSB_ResetDevice(
    IN  PMP_ADAPTER   pAd)
{
    NTSTATUS        Status = STATUS_SUCCESS;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->USB_ResetDevice\n"));
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    WdfUsbTargetDeviceResetPortSynchronously(pAd->pHifCfg->UsbDevice);

    N6USBInterfaceStart(pAd);

    return Status;
}

/*
    ========================================================================
    
    Routine Description:
        This I/O request queries the status of the PDO. 

    Arguments:
        pAd          Pointer to Adpater
        PortStatus   Can be one or both of USBD_PORT_ENABLED, USBD_PORT_CONNECTED

    Return Value:

    IRQL = PASSIVE
    
    Note:
    
    ========================================================================
*/
NTSTATUS RTUSB_GetPortStatus(
    IN  PMP_ADAPTER   pAd)
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
//  PIRP                pIrp = NULL;
//  KEVENT              IoEvent;
//  IO_STATUS_BLOCK     IoStatus;
//  PIO_STACK_LOCATION  pNextStack;

    DBGPRINT(RT_DEBUG_TRACE, ("--->RTUSB_GetPortStatus\n"));

    // Check for correct level
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

//  *PortStatus = 0;

    ntStatus = WdfUsbTargetDeviceIsConnectedSynchronous(pAd->pHifCfg->UsbDevice);

#if 0
    // Initialize event 
    KeInitializeEvent(&IoEvent, NotificationEvent, FALSE);

    // Build an IRP in which to pass the URB to USBD
    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                pAd->pHifCfg->pNextDeviceObject, // Points to the next-lower
                NULL,                           // No Input
                0,
                NULL,                           // No output
                0,
                TRUE,                           // Internal use IRP_MJ_INTERNAL_DEVICE_CONTROL
                &IoEvent,
                &IoStatus);
    
    if (!pIrp)
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("RTUSB_GetPortStatus::memory alloc for irp failed\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        return(ntStatus);
    }

    // Pass the URB to the USB driver stack
    pNextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(pNextStack != NULL);
    pNextStack->Parameters.Others.Argument1 = PortStatus;

    ntStatus = IoCallDriver(pAd->pHifCfg->pNextDeviceObject, pIrp);
    if (ntStatus == STATUS_PENDING)
    {
        ntStatus = KeWaitForSingleObject(&IoEvent, Executive, KernelMode, FALSE, NULL);
        if(!NT_SUCCESS(ntStatus))
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("KeWaitForSingleObject failed\n"));
        }
    }
    else
    {
        IoStatus.Status = ntStatus;
    }
    
    // USBD maps the error code for us
    ntStatus = IoStatus.Status;

    DBGPRINT(RT_DEBUG_TRACE, ("<---RTUSB_GetPortStatus, PortStatus=0x%08x\n", *PortStatus));
#endif

    return ntStatus;
}

/*
    ========================================================================
    
    Routine Description:
        Check Bulkout endpoint usage: 

    Arguments:
        pAd         Pointer to Adpater

    Return Value:

    IRQL = ANY
    
    Note:
    
    ========================================================================
*/
NTSTATUS RTUSB_EndpointUsage(
    IN  PMP_ADAPTER   pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    if ((pAd->Mlme.OneSecPeriodicRound % 10) == 9)
    {
        UCHAR tempAC = 0x01; //Default endpoint 0 should be aborp and reset

        // Per Endpoint count down counter
        if (pAd->pHifCfg->TxContext[1].BulkoutEPCountDown > 0 )
        {
            pAd->pHifCfg->TxContext[1].BulkoutEPCountDown --;
            tempAC |=0x02;              
        }
        
        if (pAd->pHifCfg->TxContext[2].BulkoutEPCountDown > 0 )          
        {
            pAd->pHifCfg->TxContext[2].BulkoutEPCountDown --;
            tempAC |=0x04;                      
        }
        
        if (pAd->pHifCfg->TxContext[3].BulkoutEPCountDown > 0 )          
        {
            pAd->pHifCfg->TxContext[3].BulkoutEPCountDown --;
            tempAC |=0x08;                  
        }

        pAd->pHifCfg->UsedEndpoint = tempAC;

        if (pAd->pHifCfg->UsedEndpoint !=1)
            pPort->CommonCfg.bAggregationCapable = FALSE;
        else
            pPort->CommonCfg.bAggregationCapable = pAd->pHifCfg->bUsbTxBulkAggre;
        
        DBGPRINT(RT_DEBUG_TRACE, ("RTUSB_EndpointUsage -  %d:%d:%d:%d, UsedEndpoint = 0x%02X, BulkLastOneSecCount = %d\n",pAd->pHifCfg->PendingTx[0],pAd->pHifCfg->PendingTx[1],pAd->pHifCfg->PendingTx[2],pAd->pHifCfg->PendingTx[3],pAd->pHifCfg->UsedEndpoint,pAd->pHifCfg->BulkLastOneSecCount));             
    }
    return NDIS_STATUS_SUCCESS;
}

/*
    ========================================================================
    
    Routine Description:
        Handling bulkout reset: 

    Arguments:
        pAd         Pointer to Adpater
        pTxContext  Point to Tx context for Bulkout
        BulkOutPipeId   Endpoint id
        BulkFailIrpIdx  Irp index

    Return Value:

    IRQL = PASSIVE
    
    Note:
    
    ========================================================================
*/
NTSTATUS RTUSB_ReBulkOut(
    IN  PMP_ADAPTER   pAd,
    IN  PHT_TX_CONTEXT  pTxContext,
    IN  UCHAR           BulkOutPipeId,
    IN  UCHAR           BulkFailIrpIdx)
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
//  UCHAR       IrpIdx = 0;
    WDFREQUEST      writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;  

    //
    // Re-BulkOut Fail 
    //
//  NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);

    //BulkFailIrpIdx = pAd->pHifCfg->BulkFailParm.IrpIdx[BulkOutPipeId];

#if 0
    if ((pTxContext->IRPPending[0] == TRUE) && (pTxContext->IRPPending[1] == TRUE))
    {
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
        pAd->pHifCfg->BulkOutReqCount[2]++;
        DBGPRINT_RAW(RT_DEBUG_TRACE,(" %s: All IRPPending is busy\n",__FUNCTION__));
        return STATUS_UNSUCCESSFUL;
    }
    else if ((!((pTxContext->IRPPending[0] == FALSE) && (pTxContext->IRPPending[1] == FALSE)))
    && ((pAd->pHifCfg->TxContext[BulkOutPipeId].bCopySavePad == TRUE) || (pTxContext->bCurWriting == TRUE)))
    {   
        // Already one IRP bulked out. 
        // if bCurWriting=TRUE, just return here.  
        // if bCopySavePad = TRUE,   just return here.
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
            DBGPRINT_RAW(RT_DEBUG_TRACE,(" SavedPad : %x  %x  %x  %x  %x  %x  %x  %x \n",
                pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[0], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[1], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[2],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[3]
                ,pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[4], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[5], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[6],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[7]));
        return STATUS_UNSUCCESSFUL;
    }

    if (pTxContext->IRPPending[0] == FALSE)
        IrpIdx =0;
    else if (pTxContext->IRPPending[1] == FALSE)
    {
        IrpIdx =1;
        pAd->pHifCfg->BulkOutReqCount[1]++;
    }
    else
    {
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
        DBGPRINT_RAW(RT_DEBUG_TRACE,(" All pTxContext->IRPPending is pending\n"));
        return STATUS_UNSUCCESSFUL;     
    }
#endif
    
    NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
    pTxContext->IRPPending[BulkFailIrpIdx] = TRUE;

    writeRequest = pTxContext->WriteRequestArray[BulkFailIrpIdx];
    writeContext = GetWriteRequestContext(writeRequest);

    N6USBInitHTTxDesc(  pAd, 
                        pTxContext, 
                        writeRequest,
                        (PVOID)writeContext,
                        BulkOutPipeId, 
                        pAd->pHifCfg->BulkFailParm.NextBulkOutPositon[BulkOutPipeId][BulkFailIrpIdx],
                        pAd->pHifCfg->BulkFailParm.BulkOutSize[BulkOutPipeId][BulkFailIrpIdx] , 
                        BulkFailIrpIdx, 
                        N6USBBulkOutDataPacketComplete);
    pTxContext->BulkoutEPCountDown = 2; 
    NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);

    if (WdfRequestSend(writeRequest, writeContext->IoTarget, &G_RequestOptions ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("Re N6USBBulkOutDataPacket send failed\n"));      
    }                                   
    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Re N6USBBulkOutDataPacket EP#%d BulkOutSize = %d NextBulkOutPositon = %d, BulkFailIrpIdx = %d\n",BulkOutPipeId, pAd->pHifCfg->BulkFailParm.BulkOutSize[BulkOutPipeId][BulkFailIrpIdx],pAd->pHifCfg->BulkFailParm.NextBulkOutPositon[BulkOutPipeId][BulkFailIrpIdx], BulkFailIrpIdx));
    
    MT_INC_REF(&pAd->pHifCfg->PendingTx[BulkOutPipeId]);   
    return NdisStatus;
}



/*
    ========================================================================
    
    Routine Description:
        This API will simulate a plug/unplug event and the device will be 
        removed and re-added.
        Callers of this function must be running at
        PASSIVE LEVEL.

    Arguments:

    Return Value:

    IRQL = PASSIVE LEVEL
    
    Note:
    
    ========================================================================
*/
NTSTATUS    RTUSB_Replug(
    IN  PMP_ADAPTER   pAd)
{
    NTSTATUS        ntStatus = STATUS_SUCCESS;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->USB_CYCLE_PORT\n"));
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    // must after load FW
    WdfUsbTargetDeviceCyclePortSynchronously(pAd->pHifCfg->UsbDevice);

    N6USBInterfaceStart(pAd);

    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);

    return ntStatus;
}

VOID
MTUSBCmdHander(
    IN PMP_PORT     pPort,
    IN UINT             Cmd,
    IN PUCHAR         pData,
    IN ULONG          DataLen
    )
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    NTSTATUS    ntStatus = NDIS_STATUS_SUCCESS;
//  ULONG       portStatus = 0;
    PMP_ADAPTER pAd = pPort->pAd;
    PMP_PORT  pApPort = NULL;
    ULONG       WPSLedMode10;
    BOOLEAN     bDetectHwWpsPbc = FALSE;
    ULONG       DW0,TimeTillTbtt;
    ULONG BytesReadTemp, BytesNeededTemp;

    AUTO_RSP_CFG_STRUC AutoRspCfg = {0};
    PUSHORT pPsm;


    if(Cmd == MT_CMD_PNP_SET_POWER)          
            {
                PMP_PORT pStaPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
                NDIS_DEVICE_POWER_STATE     NewDeviceState;
                PlatformMoveMemory(&NewDeviceState, pData, sizeof(NDIS_DEVICE_POWER_STATE));
                N6UsbSetPowerbyPort(pStaPort,NewDeviceState);

                N6CompletePendingOID( pAd, PENDING_PNP_SET_POWER, NDIS_STATUS_SUCCESS);
                
                DBGPRINT(RT_DEBUG_TRACE, ("[%s],%d CMD::MT_CMD_PNP_SET_POWER\n",__FUNCTION__,__LINE__));
            }
    else if (Cmd== MT_CMD_RTMP_SHUTDOWN)
            {               
                DBGPRINT(RT_DEBUG_TRACE, ("[%s],%d CMD::MT_CMD_RTMP_SHUTDOWN\n", __FUNCTION__, __LINE__));
                N6Shutdown(pAd);
            }
    else if(Cmd == MT_CMD_RESEND_RX_IRP)
            {               
                UCHAR BulkInIndex;
            
                N6UsbRecvStart(pAd);

                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS);
                for (BulkInIndex = 0; BulkInIndex < pAd->pNicCfg->NumOfBulkInIRP ; BulkInIndex++ )
                {
                    if( pAd->pHifCfg->PendingRx < pAd->pNicCfg->NumOfBulkInIRP  )
                    {
                        N6USBBulkReceive(pAd);
                        //N6UsbRxStaPacket(pAd,FALSE);
                    }
                }
                DBGPRINT(RT_DEBUG_TRACE, ("[%s],%d CMD::MTK_OID_N5_QUERY_RESEND_RX_IRP ,pendingRx=%d\n",__FUNCTION__,__LINE__,pAd->pHifCfg->PendingRx));
            }   
            else if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))    && 
                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)))
            {               
        switch (Cmd)
                            {
                    case MT_CMD_SEND_BEACON:
                       {
                            PMP_PORT    pPort = NULL;
        
                            pPort = pAd->BeaconPort.pApGoPort;
                            if ((pPort != NULL) && pPort->bActive && ((pPort->PortType == WFD_GO_PORT) || (pPort->PortType == EXTAP_PORT)))
                            {   
                                MgntPktSendBeacon(pPort);
                                break;
                            }
                            break;
                        }
                            
                    case MT_CMD_DISCONNECT_REQUEST:
                        DisconnectRequest(pAd,  pAd->PortList[pAd->ucActivePortNum], &BytesReadTemp, &BytesNeededTemp, FALSE);
                        break;

                    case MT_CMD_CTRL_PWR_BIT: // Update the power bit in control frame
                    {
                        pPsm = ((PUSHORT)(pData));
                        
                        RTUSBReadMACRegister(pAd, AUTO_RSP_CFG, &AutoRspCfg.word);
                        AutoRspCfg.field.AckCtsPsmBit = ((*pPsm == PWR_SAVE) ? 1 : 0);
                        RTUSBWriteMACRegister(pAd, AUTO_RSP_CFG, AutoRspCfg.word);

                        DBGPRINT(RT_DEBUG_INFO, ("%s: (MT_CMD_CTRL_PWR_BIT) PSM = %d\n", 
                            __FUNCTION__, 
                            *pPsm));
                    }
                    break;
                    
                    case MT_CMD_SET_TBTT_TIMER:
#ifdef MULTI_CHANNEL_SUPPORT
                        // Prepare next time.
                        RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &DW0);

                        if (DW0 <= pPort->P2PCfg.TbttNextTargetTimePoint)
                        {
                            TimeTillTbtt = pPort->P2PCfg.TbttNextTargetTimePoint - DW0;
                            TimeTillTbtt = (TimeTillTbtt >> 10)& 0xffff;

                            // Re-read tbtt timer from register and add 1 beacon interval,
                            // if the reset time is less than 20 ms
                            if (TimeTillTbtt < 20)
                            {
                                ULONG   StartTimeOffset = 0;

                                RTUSBReadMACRegister(pAd, TBTT_TIMER, &TimeTillTbtt);
                                TimeTillTbtt = TimeTillTbtt&0x1ffff;
//                              RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &DW0);

                                // start time right after beacon transmission.
                                pPort->P2PCfg.TbttNextTargetTimePoint = DW0 + TimeTillTbtt*64 + (pPort->CommonCfg.BeaconPeriod/100)*102400 + StartTimeOffset;
                                TimeTillTbtt = TimeTillTbtt*64 +  (pPort->CommonCfg.BeaconPeriod/100)*102400 + StartTimeOffset;
                                //convert TimeTillTbtt from us to ms
                                TimeTillTbtt = (TimeTillTbtt >> 10)& 0xffff;

                                // Set timer
                                PlatformSetTimer(pPort, &pPort->Mlme.SwTbttTimer, TimeTillTbtt);
                                PlatformSetTimer(pPort, &pPort->Mlme.TBTTTimer, TimeTillTbtt);
                                DBGPRINT(RT_DEBUG_INFO, (" %s(%d) : Restart SwTbttTimer\n", __FUNCTION__, __LINE__));
                            }
                            else
                            {
                                PlatformSetTimer(pPort, &pPort->Mlme.SwTbttTimer, TimeTillTbtt);
                                PlatformSetTimer(pPort, &pPort->Mlme.TBTTTimer, TimeTillTbtt);
                                // duration is one beacon period
                                pPort->P2PCfg.TbttNextTargetTimePoint += (pPort->CommonCfg.BeaconPeriod/100)*102400;
                            }
                        }
                        else
                        {
                            // Restart TBTT timer due to our GP timer's delay. 
//                          if (pPort->CommonCfg.P2pControl.field.SwBasedNoA == 1)
                            {
                                ULONG   StartTimeOffset = 0;

                                RTUSBReadMACRegister(pAd, TBTT_TIMER, &TimeTillTbtt);
                                TimeTillTbtt = TimeTillTbtt&0x1ffff;
//                              RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &DW0);

                                // start time right after beacon transmission.
                                pPort->P2PCfg.TbttNextTargetTimePoint = DW0 + TimeTillTbtt*64 + StartTimeOffset;
                                TimeTillTbtt = TimeTillTbtt*64 + StartTimeOffset;
                                //convert TimeTillTbtt from us to ms
                                TimeTillTbtt = (TimeTillTbtt >> 10)& 0xffff;
                                // Set timer
                                PlatformSetTimer(pPort, &pPort->Mlme.SwTbttTimer, TimeTillTbtt);
                                PlatformSetTimer(pPort, &pPort->Mlme.TBTTTimer, TimeTillTbtt);                      
                                DBGPRINT(RT_DEBUG_INFO, (" %s(%d) : Restart SwTbttTimer\n", __FUNCTION__, __LINE__));
                            }
                        }
                        DBGPRINT(RT_DEBUG_INFO, ("%s: (MT_CMD_SET_TBTT_TIMER) BeaconPeriod=%d, TimeTillTbtt=%d, TbttNextTargetTimePoint=%d, DW0=%d\n", 
                            __FUNCTION__, pPort->CommonCfg.BeaconPeriod, TimeTillTbtt, pPort->P2PCfg.TbttNextTargetTimePoint, DW0));

#else
                        RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &DW0);
                        //convert DW0 from us to ms 
                        DW0 = (DW0 >> 10);
                        TimeTillTbtt = ((ULONG) pPort->CommonCfg.BeaconPeriod - 
                                        (DW0 % (ULONG) pPort->CommonCfg.BeaconPeriod));
                        PlatformSetTimer(pPort, &pPort->Mlme.TBTTTimer, TimeTillTbtt);
#endif  /* MULTI_CHANNEL_SUPPORT */
                        break;

                    case MT_CMD_UPDATE_TIM:
                        // Update TIM job move to MgntPktSendBeacon()
                        break;

                    case MT_CMD_RESET_BBP:

                        //Patch Scan no AP issue

                        //Reset BBP Registers
                        DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d set MAC 0x1004 = 0x0\n",__FUNCTION__,__LINE__));
                        RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0x0);
                        RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0x2);

                        //Restore previous BBP values
                        NICRestoreBBPValue(pAd);


                        RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0xc);

                        break;

                    case MT_CMD_FORCE_SLEEP: 
                        AsicSleepThenAutoWakeup(pAd, 1);
                        break;
                        
                    case MT_CMD_CHECK_GPIO:
                        {
                            ULONG   data = 0;
                            BOOLEAN RadioOff = FALSE;

                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!! RT_OID_CHECK_GPIO !!!\n"));
                            {
                                // Read GPIO pin2 as Hardware controlled radio state
                                RTUSBReadMACRegister(pAd, GPIO_CTRL_CFG, &data);
                                if ((data & 0x04) == 0)
                                    RadioOff = TRUE;
                            }

                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!!Radio %d !!!\n", RadioOff));

                            if ((pAd->StaCfg.bHardwareRadio == TRUE) && 
                                (pPort->CommonCfg.GpioPinSharingMode == GpioPinSharingMode_DefaultMode))
                            {

                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!! Radio Check !!!\n"));

                                if (RadioOff)
                                {
                                    pAd->StaCfg.bHwRadio = FALSE;
                                }
                                else
                                {
                                    pAd->StaCfg.bHwRadio = TRUE;
                                }

                                if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
                                {
                                    pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                                    if (pAd->StaCfg.bRadio == TRUE)
                                    {
                                        DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!! Radio On !!!\n"));

                                        MlmeRadioOn(pAd);
                                        // Update extra information
                                        pAd->UiCfg.ExtraInfo = EXTRA_INFO_CLEAR;
                                    }
                                    else
                                    {
                                        ULONG BytesReadTemp, BytesNeededTemp;
                                        
                                        DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!! Radio Off !!!\n"));

                                        // Perform a disassociation operation and make a NDIS_STATUS_DOT11_DISASSOCIATION indication.
                                        DisconnectRequest(pAd, pPort,&BytesReadTemp, &BytesNeededTemp, TRUE);

                                        MlmeRadioOff(pAd,pPort);
                                        // Update extra information
                                        pAd->UiCfg.ExtraInfo = HW_RADIO_OFF;
                                    }
                                }
                                PlatformIndicateCurrentPhyPowerState(pAd, 0,DOT11_PHY_ID_ANY);
                            }
                            // ========================================
                            // Read GPIO pin3 as WPS Push Button state
                            // ========================================

                            // TODO: Push Button must hold more than 3 sec ?
                            if (pAd->HwCfg.NicConfig2.field.EnableWPSPBC && (pAd->StaCfg.bRadio == TRUE) && (pAd->HwCfg.LedCntl.field.LedMode != LED_MODE_SIGNAL_STREGTH))
                            {
                                //
                                // Enable HW PBC functionality only if the NIC runs with Ralink UI.
                                //
                                if (pPort->StaCfg.WscControl.bCheckHWPBC == TRUE)
                                {
                                    pPort->StaCfg.WscControl.bCheckHWPBC = FALSE;

                                    if (pPort->CommonCfg.GpioPinSharingMode == GpioPinSharingMode_Mode1)
                                    {
                                        if (data & 0x04) // GPIO #2 (HW WPS PBC, active low)
                                        {
                                            bDetectHwWpsPbc = FALSE;
                                        }
                                        else
                                        {
                                            bDetectHwWpsPbc = TRUE;
                                        }
                                    }
                                    else
                                    {
                                        if (data & 0x08) // GPIO #3 (HW WPS PBC, active low)
                                        {
                                            bDetectHwWpsPbc = FALSE;
                                        }
                                        else
                                        {
                                            bDetectHwWpsPbc = TRUE;
                                        }
                                    }

                                    //
                                    // GPIO Pin#3 as WPS PBC (Push Buttom Configuration) active low.
                                    //                          
                                    if (bDetectHwWpsPbc == FALSE)
                                    {
                                        pAd->StaCfg.bHwPBC = FALSE;
                                    }
                                    else
                                    {
                                        pAd->StaCfg.bHwPBC = TRUE;
                                        //Reset WscStatus before starting HW PBC
                                        pPort->StaCfg.WscControl.WscStatus=STATUS_WSC_NOTUSED;                                    
                                    
                                        // Inform UI to start HW PBC
                                        if (pAd->StaCfg.HwPBCState == FALSE)
                                            pAd->StaCfg.HwPBCState = TRUE; 
                                    }

                                    
                                    if(pPort->StaCfg.WscControl.bCheckHWPBCCustomerStateMachine == FALSE)
                                    {
                                        if ((pAd->StaCfg.bHwPBC == TRUE) &&
                                            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
                                        {
                                            // Start WPS session.
                                            pPort->StaCfg.WscControl.bWPSSession = TRUE;
                                            

                                            if (pAd->StaCfg.WscHwPBCMode == 1)
                                            {
                                                WscPushHWPBCDisconnectAction(pAd, pPort);
                                                
                                                // Set to Scan AP first or UI won't query WSC status any more
                                                pPort->StaCfg.WscControl.WscStatus=STATUS_WSC_SCAN_AP;                                    
                                                Delay_us(1000000);//1 sec to wait UI set disconnection.

                                                WscPushHWPBCStartAction(pAd, pPort);
                                            }
                                            else if (pAd->StaCfg.WscHwPBCMode == 0)
                                            {
                                                // 0. Disassoc from current AP first
                                                if (INFRA_ON(pPort))
                                                {
                                                    MLME_DISASSOC_REQ_STRUCT    DisReq;
                                                    
                                                    // Set to immediately send the media disconnect event
                                                    pAd->MlmeAux.CurrReqIsFromNdis = TRUE;

                                                    DBGPRINT(RT_DEBUG_TRACE, ("disassociate with current AP before Hardward PBC\n"));
                                                    MlmeCntDisassocParmFill(pAd, &DisReq, pPort->PortCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
                                                    MlmeEnqueue(pAd,pPort,ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
                                                                sizeof(MLME_DISASSOC_REQ_STRUCT), &DisReq);

                                                    pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;

                                                    // Set the AutoReconnectSsid to prevent it reconnect to old SSID
                                                    // Since calling this indicate user don't want to connect to that SSID anymore.
                                                    pAd->MlmeAux.AutoReconnectSsidLen= 32;
                                                    PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
                                                }
                                                else if (ADHOC_ON(pPort))
                                                {
                                                    // Set the AutoReconnectSsid to prevent it reconnect to old SSID
                                                    // Since calling this indicate user don't want to connect to that SSID anymore.
                                                    pAd->MlmeAux.AutoReconnectSsidLen= 32;
                                                    PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);          
                                    
                                                    MlmeCntLinkDown(pPort, FALSE);
                                                }
                                                
                                                Delay_us(1000000);//1 sec to wait UI set disconnection.
                                                
                                                // Always disable auto reconnection.
                                                pAd->MlmeAux.AutoReconnectSsidLen= 32;
                                                pAd->MlmeAux.AutoReconnectStatus = FALSE;
                                                PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

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

                                                pPort = pAd->PortList[PORT_0];
                                                WscPushPBCAction(pAd, pPort);
                                            
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;

                    case MT_CMD_ENABLE_PROMISCUOUS_MODE:
                        MlmeInfoEnablePromiscuousMode(pAd);
                        break;
                    case MT_CMD_DISABLE_PROMISCUOUS_MODE:
                        MlmeInfoDisablePromiscuousMode(pAd);
                        break;                  
                    case MT_CMD_DOT11_RTS_THRESHOLD:
                        {
                            TX_RTS_CFG_STRUC RtsCfg;
                            RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                            RtsCfg.field.RtsThres = pPort->CommonCfg.RtsThreshold;
                            RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
                            DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_DOT11_RTS_THRESHOLD  (=%d)\n", pPort->CommonCfg.RtsThreshold));
                        }
                        
                        break;
                    case MT_CMD_ADD_STA_MAPPING_KEY:
                        {
                            NDIS_PORT_NUMBER PortNumber = *((NDIS_PORT_NUMBER *)pData);
                            PDOT11_BYTE_ARRAY KeyData = (PDOT11_BYTE_ARRAY)(pData + sizeof(NDIS_PORT_NUMBER));
                            PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE pKeyMappingKeys;
                            //OS build 676 fix station port at port zero
                            PMP_PORT pStaPort = pAd->PortList[PortNumber/*NDIS_DEFAULT_PORT_NUMBER*/];
                            NDIS_STATUS AddKeyStatus = NDIS_STATUS_SUCCESS;

                            pKeyMappingKeys = (PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE) KeyData->ucBuffer;
                            AddKeyStatus = MlmeInfoAddKey(pAd,
                                pStaPort,
                                pKeyMappingKeys->PeerMacAddr,
                                0,
                                pKeyMappingKeys->AlgorithmId,
                                pKeyMappingKeys->usKeyLength,
                                pKeyMappingKeys->ucKey,
                                pKeyMappingKeys->bDelete,
                                TRUE,
                                TRUE);

#ifdef MULTI_CHANNEL_SUPPORT
                            if (pStaPort->PortType == EXTSTA_PORT)
                            {
                                // This flag will assign STA data to HCCA during new session phase
                                OPSTATUS_SET_FLAG(pPort, fOP_STATUS_INFRA_OP);
                                
                                // CLI connectes to peer GO and cehck STA/GO status
                                // Defer a task to handle multi-channel state and DHCP
                                MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_NEW_CONNECTION_START);
                                PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, 100);   
                            }
#endif /*MULTI_CHANNEL_SUPPORT*/

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                            // Update P2pMs status and info during adding the pairwise key
                            if (NDIS_WIN8_ABOVE(pAd) && 
                                ((pStaPort->PortType == WFD_CLIENT_PORT) || (pStaPort->PortType == WFD_GO_PORT)) &&
                                (AddKeyStatus == NDIS_STATUS_SUCCESS))
                            {
                                P2pMsWPADone(pAd, pStaPort, pKeyMappingKeys->PeerMacAddr);
                            }
#endif
                            DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_ADD_STA_MAPPING_KEY(port=%d)\n", PortNumber));                           
                        }       
                        break;
                    case MT_CMD_ADD_STA_DEFAULT_KEY:
                        {
                            NDIS_PORT_NUMBER PortNumber = *((NDIS_PORT_NUMBER *)pData);
                            PDOT11_CIPHER_DEFAULT_KEY_VALUE pDefaultKey = (PDOT11_CIPHER_DEFAULT_KEY_VALUE)(pData + sizeof(NDIS_PORT_NUMBER));
                            //OS build 676 fix station port at port zero                            
                            PMP_PORT pStaPort = pAd->PortList[PortNumber/*NDIS_DEFAULT_PORT_NUMBER*/];
                            MlmeInfoAddKey(pAd,
                                pStaPort,
                                pDefaultKey->MacAddr,
                                (UCHAR)pDefaultKey->uKeyIndex,
                                pDefaultKey->AlgorithmId,
                                pDefaultKey->usKeyLength,
                                pDefaultKey->ucKey,
                                pDefaultKey->bDelete,
                                FALSE,
                                FALSE);
                            DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_ADD_STA_DEFAULT_KEY (port=%d)\n", PortNumber));  
                        }
                        break;
                    case MT_CMD_ADD_AP_DEFAULT_KEY:
                        {
                            //OS build 676 fix station port at port 1
                            PMP_PORT pAPPort = pAd->PortList[PORT_1];
                            PDOT11_CIPHER_DEFAULT_KEY_VALUE pDefaultKey = (PDOT11_CIPHER_DEFAULT_KEY_VALUE)pData;
                                RTMPApAddKey(pAd,
                                    pAPPort,
                                    pDefaultKey->MacAddr,
                                    (UCHAR)pDefaultKey->uKeyIndex,
                                    pDefaultKey->AlgorithmId,
                                    pDefaultKey->usKeyLength,
                                    pDefaultKey->ucKey,
                                    pDefaultKey->bDelete,
                                    FALSE,
                                    TRUE);          
                        }
                        DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_ADD_AP_DEFAULT_KEY\n"));
                        break;
                    case MT_CMD_ADD_AP_MAPPING_KEY:
                        {
                            //OS build 676 fix station port at port 1
                            PMP_PORT pAPPort = pAd->PortList[PORT_1];
                            PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE pKeyMappingKeys;
                            PDOT11_BYTE_ARRAY KeyData = (PDOT11_BYTE_ARRAY)pData;
                            pKeyMappingKeys = (PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE) KeyData->ucBuffer;  
                            RTMPApAddKey(pAd, 
                            pAPPort,
                            pKeyMappingKeys->PeerMacAddr,
                            0,
                            pKeyMappingKeys->AlgorithmId,
                            pKeyMappingKeys->usKeyLength,
                            pKeyMappingKeys->ucKey,
                            pKeyMappingKeys->bDelete,
                            TRUE,
                            TRUE);
                        }
                        DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_ADD_AP_MAPPING_KEY\n"));
                        break;
                    case MT_CMD_STA_UPDTAE_DEFAULT_ID:
                        {
                            //OS build 676 fix station port at port zero
                            NDIS_PORT_NUMBER PortNumber = *((NDIS_PORT_NUMBER *)pData);                         
                            PMP_PORT pStaPort = pAd->PortList[PortNumber/*NDIS_DEFAULT_PORT_NUMBER*/];
                            MlmeInfoStaOidSetWEPDefaultKeyID(pStaPort);
                            DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_STA_UPDTAE_DEFAULT_ID (pPort=%d)\n", PortNumber));
                        }
                        break;
                    case OID_DOT11_START_AP_REQUEST:                            
                        {
                            PMP_PORT pAPPort;
                            NDIS_PORT_NUMBER PortNumber = *((NDIS_PORT_NUMBER *)pData);
                            pAPPort = pAd->PortList[PortNumber];
                            ApStartApRequestHandler(pAd, pAPPort);
                            DBGPRINT(RT_DEBUG_TRACE, ("CMD::OID_DOT11_START_AP_REQUEST \n"));
                        }
                        break;              
                    case MT_CMD_AP_RESET_REQUEST:
                        {                           
                            PMP_PORT pAPPort;
                            NDIS_PORT_NUMBER PortNumber = *((NDIS_PORT_NUMBER *)pData);
                            pAPPort = pAd->PortList[PortNumber];
                            ApInfoResetHandler(pAPPort);
                        }
                        
                        DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_AP_RESET_REQUEST \n"));
                        break;
                    case MT_CMD_STA_RESET_REQUEST:
                        {
                            PMP_PORT pAPPort;
                            NDIS_PORT_NUMBER PortNumber = *((NDIS_PORT_NUMBER *)pData);
                            pAPPort = pAd->PortList[PortNumber];
                            NdisCommonInfoResetHandler(pAPPort);
                        }
                        break;
                    case MT_CMD_DOT11_POWER_MGMT_REQUEST:
                        MlmeInfoPnpSetPowerMgmtMode(pAd, *(PULONG)pData);
                        DBGPRINT(RT_DEBUG_TRACE, ("CMD::MT_CMD_DOT11_POWER_MGMT_REQUEST \n"));
                        break;

                    case MT_CMD_RESTART_AP_IS_REQUIRED:
                        {
                            PMP_PORT pAPPort;
                            NDIS_PORT_NUMBER PortNumber = *((NDIS_PORT_NUMBER *)pData);
                            pAPPort = pAd->PortList[PortNumber];
                            DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]CMD::MT_CMD_RESTART_AP_IS_REQUIRED, restart SoftAP @ port %d ...  \n",__FUNCTION__,__LINE__,pPort->PortNumber));
                            ApRestartAPHandler(pAd, pAPPort);
                        }
                        break;

                    case MT_CMD_PERIODIC_EXECUT:
                        {
                            ULONG i;
                            //
                            // The locking range of VCO changes with temperature. Not do this in PM5 non-associate to reduce power consumption
                            //
                            if ((((pAd->Mlme.OneSecPeriodicRound + VCO_CALIBRATION_PERIOD_SHIFT) % VCO_CALIBRATION_PERIOD) == 0) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
                            {
                                AsicVCOCalibration(pAd);
                                pAd->HwCfg.SkipFreqCaliTimer = 3; // skip frequency calibration 3 times   
                            }

                            if(pAd->HwCfg.SkipFreqCaliTimer > 0)
                                pAd->HwCfg.SkipFreqCaliTimer--;
                               
                            for(i = 0;  i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
                            {
                                pPort = pAd->PortList[i];
                                if ((pPort == NULL) || (pPort->bActive == FALSE))
                                    continue;

                                DBGPRINT(RT_DEBUG_INFO, (" %d pPort->SoftAP.bAPStart = %d, pPort->PortType = %d, pAd->OpMode = %d, pPort->PortSubtype = %d\n", i, pPort->SoftAP.bAPStart, pPort->PortType, pAd->OpMode, pPort->PortSubtype));
                                if((pPort->SoftAP.bAPStart && 
                                    ((pPort->PortNumber == PORT_0 && pAd->OpMode == OPMODE_AP && pPort->PortSubtype == PORTSUBTYPE_SoftapAP) ||
                                    (pPort->PortNumber != PORT_0 /*&& pAd->OpMode == OPMODE_APCLIENT*/ && pPort->PortSubtype == PORTSUBTYPE_VwifiAP)))  ||
                                    (pPort->P2PCfg.bGOStart &&
                                    (/*pPort->PortNumber != PORT_0 &&*/ pPort->PortSubtype  == PORTSUBTYPE_P2PGO)))
                                {
                                    // SoftapAP : port 0
                                    // VwifiAP: port 1 or port 2
                                    // GO: port 0 or port 2 or port 3 ....          
                                    ApMlmePeriodicExec(pAd, pPort);
                                }
                                else if(pPort->PortNumber == NDIS_DEFAULT_PORT_NUMBER)
                                {
                                    STAMlmePeriodicExec(pAd);
                                }
                            }
                            RTUSB_EndpointUsage(pAd);                           
                        }
                        break;

                        case MT_CMD_PERIODIC_CHECK_RX:
                        {
                            ULONG       MACValue;
                            //
                            // Watch dog for RX no responding issue.
                            //
                            if (pAd->pHifCfg->CurrentBulkInCount == pAd->pHifCfg->BulkInReq)
                            {
                                if (!MT_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
                                                        fRTMP_ADAPTER_BULKIN_RESET |
                                                        fRTMP_ADAPTER_RADIO_OFF | 
                                                        fRTMP_ADAPTER_HALT_IN_PROGRESS |
                                                        fRTMP_ADAPTER_NIC_NOT_EXIST |
                                                        fRTMP_ADAPTER_SUSPENDED)))
                                {
                                    // get page count in RxQ
                                    RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);

                                    if (pAd->pHifCfg->HWRxPCNCount == 0)
                                    {
                                        pAd->pHifCfg->HWRxPCNCount = MACValue & 0xff000000;
                                    }
                                    else 
                                    {
                                        //
                                        // RXQPCNT not zero and keep increase, means RX URB not complete for more than one second.
                                        // Cancel it then re-bulk in.
                                        //
                                        if ((MACValue & 0xff000000) && (MACValue >= pAd->pHifCfg->HWRxPCNCount))
                                        {
                                            DBGPRINT(RT_DEBUG_ERROR, ("RX URB no responding, RXQPCNT=0x%02x, will retry bulk in, BulkInCount=%d\n", (MACValue >> 24), pAd->pHifCfg->BulkInReq));
                                            pAd->pHifCfg->CurrentBulkInCount = 0;
                                            pAd->pHifCfg->HWRxPCNCount = 0;
                                        
                                            //DBGPRINT(RT_DEBUG_ERROR, ("After Cancel IRP, PendingRx=%d\n",pAd->pHifCfg->PendingRx));
                                            
                                            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
                                            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_USB_RESET_BULK_IN, NULL, 0);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                pAd->pHifCfg->CurrentBulkInCount = pAd->pHifCfg->BulkInReq;
                                pAd->pHifCfg->HWRxPCNCount = 0;
                            }                           
                        }
                        break;  
                
                    case MT_CMD_QKERIODIC_EXECUT:
                        {
                            //ULONG PortNum;
                            TX_STA_CNT0_STRUC TxStaCnt0;
                            TX_STA_CNT1_STRUC StaTx1;
                            BOOLEAN     bUpdateStaDrs = FALSE;
                            
                            pAd->Mlme.QuickPeriodicRound++;
                            NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->Mlme.Now64);

                            DBGPRINT(RT_DEBUG_TRACE, ("%s   MT_CMD_QKERIODIC_EXECUT Port(%d) PortType = %d  IDLE_ON = %d\n", __FUNCTION__, pPort->PortNumber, pPort->PortType, IDLE_ON(pPort)));

                            //for(PortNum = 0;  PortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; PortNum++)
                            {
                                //pPort = pAd->PortList[PortNum];
                                if ((pPort != NULL) && (pPort->bActive == TRUE))
                                {
                                    if((((pPort->PortType == EXTAP_PORT) || (pAd->OpMode == OPMODE_AP)) && (pPort->SoftAP.bAPStart)) 
                                        || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
                                    {
                                        if (pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning)
                                        {
                                            //MlmeUpdateDRSTimeAndStatistic(pAd, &TxStaCnt0, &StaTx1);
                                            ApMlmeQuickResponeForRateUpExec(pAd,pPort);
                                        }

                                        if (pAd->Mlme.PeriodicRound % 5 == 0)
                                        {
                                            //MlmeUpdateDRSTimeAndStatistic(pAd, &TxStaCnt0, &StaTx1);
                                            TxStaCnt0.word = 0;
                                            StaTx1.word = 0;
                                            ApMlmeDynamicTxRateSwitching(pAd,pPort, TxStaCnt0, StaTx1); 
                                        }
                                    }
                                    else if ((pPort->PortType == EXTSTA_PORT) || (pPort->PortType == WFD_CLIENT_PORT))
                                    {
                                        if ((!IDLE_ON(pPort))  && (bUpdateStaDrs == FALSE))
                                        {
                                            if (pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
                                            {
                                                //MlmeUpdateDRSTimeAndStatistic(pAd, &TxStaCnt0, &StaTx1);
                                                StaQuickResponeForRateUpExec(pAd, pPort);
                                            }

                                            if (pAd->Mlme.PeriodicRound % 5 == 0)
                                            {
                                                //MlmeUpdateDRSTimeAndStatistic(pAd, &TxStaCnt0, &StaTx1);
                                                MlmeDynamicTxRateSwitching(pAd, pPort);
                                            }
                                        }
                                    }

                                    // Patch WiFi 11n: 3 STAs IBSS network. Use AP sync mode instead of IBSS sync mode.
                                    if (ADHOC_ON(pPort))
                                    {
                                        BCN_TIME_CFG_STRUC csr9;
                                        ULONG NumEntry = pPort->MacTab.Size;
                                        if((pAd->StaCfg.IBSSync.csr9.field.bBeaconGen == TRUE) && (pAd->StaCfg.IBSSync.csr9.field.TsfSyncMode != 3))            
                                        {
                                            if (pAd->StaCfg.IBSSync.csr9.field.TsfSyncMode != 3)
                                            {
                                                RTUSBReadMACRegister(pAd, BCN_TIME_CFG, &csr9.word);
                                                pAd->StaCfg.IBSSync.csr9.word = csr9.word;
                                                pAd->StaCfg.IBSSync.csr9.field.TsfSyncMode = 3;
                                                RTUSBWriteMACRegister(pAd, BCN_TIME_CFG, pAd->StaCfg.IBSSync.csr9.word);
                                                DBGPRINT(RT_DEBUG_TRACE, ("IBSS Sync: AP mode\n"));
                                            }
                                        }
                                        else if((pAd->StaCfg.IBSSync.csr9.field.bBeaconGen == TRUE) && (pAd->StaCfg.IBSSync.csr9.field.TsfSyncMode != 2))
                                        {
                                            RTUSBReadMACRegister(pAd, BCN_TIME_CFG, &csr9.word);
                                            pAd->StaCfg.IBSSync.csr9.word = csr9.word;
                                            pAd->StaCfg.IBSSync.csr9.field.TsfSyncMode = 2;
                                            RTUSBWriteMACRegister(pAd, BCN_TIME_CFG, pAd->StaCfg.IBSSync.csr9.word);    
                                            DBGPRINT(RT_DEBUG_TRACE, ("IBSS Sync: IBSS mode\n"));
                                        }
                                    }

                                    if (pAd->Mlme.QuickPeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0)
                                    {
                                        PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

                                        
                                        // 1x1 device for 802.11n WMM Test
                                        if ((pBssidMacTabEntry != NULL) &&(pAd->HwCfg.Antenna.field.TxPath == 1) &&
                                            INFRA_ON(pPort) &&                                  
                                            (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE) && 
                                            ((pPort->CommonCfg.BACapability.field.Policy == BA_NOTUSE)||(pBssidMacTabEntry->TXAutoBAbitmap==0)))
                                        {
                                            EDCA_AC_CFG_STRUC   Ac0Cfg;
                                            EDCA_AC_CFG_STRUC   Ac2Cfg;                             
                                            RTUSBReadMACRegister(pAd, EDCA_AC2_CFG, &Ac2Cfg.word);                                  
                                            RTUSBReadMACRegister(pAd, EDCA_AC0_CFG, &Ac0Cfg.word);
                                            
                                            if ((pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_VO] == 0) &&
                                                (pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_BK] == 0) &&
                                                (pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_BE] < 50) &&                                      
                                                (pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_VI] >= 1000))
                                            {
                                                //5.2.27/28 T7: Total throughput need to ~36Mbps
                                                if (Ac2Cfg.field.Aifsn!=0xc)
                                                {
                                                    Ac2Cfg.field.Aifsn = 0xc;
                                                    RTUSBWriteMACRegister(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
                                                }
                                            }
                                            else if ((pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_VO] == 0) &&
                                                (pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_BK] == 0) &&
                                                (pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_VI] == 0) &&                                      
                                                (pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_BE] < 10))
                                            {
                                                // restore default parameter of BE
                                                if ((Ac0Cfg.field.Aifsn!=3) ||(Ac0Cfg.field.AcTxop!=0))
                                                {
                                                    if(Ac0Cfg.field.Aifsn!=3)
                                                        Ac0Cfg.field.Aifsn = 3;
                                                    if(Ac0Cfg.field.AcTxop!=0)
                                                        Ac0Cfg.field.AcTxop = 0;
                                                    RTUSBWriteMACRegister(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
                                                }

                                                // restore default parameter of VI
                                                if (Ac2Cfg.field.Aifsn!=0x3)
                                                {
                                                    Ac2Cfg.field.Aifsn = 0x3;
                                                    RTUSBWriteMACRegister(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
                                                }
                                            }
                                        }
                                        
                                    }
                                }
                                
                            }
    
                            if (pAd->Mlme.QuickPeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0)
                            {
                                //
                                // Clear Tx/Rx Traffic one second count.
                                //
                                pAd->pHifCfg->BulkLastOneSecCount = pAd->pHifCfg->BulkOutDataOneSecCount + pAd->pHifCfg->BulkInDataOneSecCount;
                                pAd->pHifCfg->BulkOutDataOneSecCount = 0;
                                pAd->pHifCfg->BulkInDataOneSecCount = 0; 

                                // 
                                pAd->Counter.MTKCounters.OneSecAggCnt = 0;
                                pAd->Counter.MTKCounters.OneSecNoAggCnt = 0;                             

                                //pAd->Counter.MTKCounters.LastOneSecRxOkDataCnt = pAd->Counter.MTKCounters.OneSecRxOkDataCnt;

                                pAd->Counter.MTKCounters.LastOneSecTxNoRetryOkCount = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount;
                                pAd->Counter.MTKCounters.LastOneSecTxRetryOkCount = pAd->Counter.MTKCounters.OneSecTxRetryOkCount;
                                pAd->Counter.MTKCounters.LastOneSecTxFailCount = pAd->Counter.MTKCounters.OneSecTxFailCount;
                                
                                // clear all OneSecxxx counters.
                                pAd->Counter.MTKCounters.OneSecBeaconSentCnt = 0;
                                pAd->Counter.MTKCounters.OneSecFalseCCACnt = 0;
                                pAd->Counter.MTKCounters.OneSecRxFcsErrCnt = 0;
                                pAd->Counter.MTKCounters.OneSecRxOkCnt = 0;
                                pAd->Counter.MTKCounters.OneSecTxFailCount = 0;
                                pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount = 0;
                                pAd->Counter.MTKCounters.OneSecTxRetryOkCount = 0;
                                pAd->Counter.MTKCounters.OneSecRxOkDataCnt = 0;
                                pAd->Counter.MTKCounters.OneSecTransmittedByteCount = 0;
                                pAd->Counter.MTKCounters.OneSecReceivedByteCount = 0;
                                pAd->Counter.MTKCounters.OneSecRxBeaconCnt = 0;


                                // TODO: for debug only. to be removed
                                pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_BE] = 0;
                                pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_BK] = 0;
                                pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_VI] = 0;
                                pAd->Counter.MTKCounters.OneSecOsTxCount[QID_AC_VO] = 0;
                                pAd->Counter.MTKCounters.OneSecDmaDoneCount[QID_AC_BE] = 0;
                                pAd->Counter.MTKCounters.OneSecDmaDoneCount[QID_AC_BK] = 0;
                                pAd->Counter.MTKCounters.OneSecDmaDoneCount[QID_AC_VI] = 0;
                                pAd->Counter.MTKCounters.OneSecDmaDoneCount[QID_AC_VO] = 0;
                                pAd->Counter.MTKCounters.OneSecTxDoneCount = 0;
                                pAd->Counter.MTKCounters.OneSecTxAggregationCount = 0;
                                pAd->Counter.MTKCounters.OneSecRxAggregationCount = 0;   
                            }           
                        }
                        break;
                        
                    case MT_CMD_MULTI_WRITE_MAC:
                        {
                            USHORT  Offset = *((PUSHORT)pData);
                            USHORT  Length = *((PUSHORT)(pData + 2));
                            DBGPRINT(RT_DEBUG_TRACE, ("MT_CMD_MULTI_WRITE_MAC : Offset = %x, Length = %d", Offset, Length));
                            RTUSBMultiWrite(pAd, Offset, pData + 4, Length);
                        }
                        break;

                    case MT_CMD_USB_RESET_BULK_OUT:
                        {
                            ULONG       MACValue = 0;
                            ULONG       Index;
                            PHT_TX_CONTEXT  pTxContext;
                            RTMP_BULKOUT_RESET_FRAME_TYPE   typeBulkOutRest = *((PRTMP_BULKOUT_RESET_FRAME_TYPE)pData);                         
                            TX_RTY_CFG_STRUC    TxRtyCfg,TxRtyCfgtmp;
                            TX_RTS_CFG_STRUC    RtsCfg,RtsCfgtmp;
//                          UCHAR               BulkOutPipeIndex;
                            PUSB_DEVICE_CONTEXT pUsbDeviceContext;
                            UCHAR               bulkResetPipeid = (typeBulkOutRest>>4);
                            UCHAR               AnotherPendingTx = 0;
                            ULONG       index = 0;                          

                            PBF_CFG_STRUC_EXT       PbfCfg = {0};

                            typeBulkOutRest &=0x0F;
                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("CmdThread : MT_CMD_USB_RESET_BULK_OUT===> PendingTx[%d] = %d\n",bulkResetPipeid,pAd->pHifCfg->PendingTx[bulkResetPipeid]));
                            DBGPRINT(RT_DEBUG_TRACE, ("MT_CMD_USB_RESET_BULK_OUT: PendingTx %d:%d:%d:%d, pAd->UsedAC = 0x%02X\n",pAd->pHifCfg->PendingTx[0],pAd->pHifCfg->PendingTx[1],pAd->pHifCfg->PendingTx[2],pAd->pHifCfg->PendingTx[3],pAd->pHifCfg->UsedEndpoint));           
                            
                            //
                            // From DDK,
                            // All transfers must be aborted or cancelled before attempting to reset the pipe.
                            //  DO we need to RESET PIPE AT BULKOUT fail ??????????  12-25-2006 Jan                     
                            //RTUSBCancelPendingBulkOutIRP(pAd);
                            // Wait 10ms to let previous packet that are already in HW FIFO to clear. by MAXLEE 12-25-2007

                            if( typeBulkOutRest == BULKOUT_RESET_TYPE_DATA )
                            {
                                pAd->pHifCfg->TxContext[0].AnotherPendingTx = 0; 
                                pAd->pHifCfg->TxContext[1].AnotherPendingTx = 0;                                 
                                pAd->pHifCfg->TxContext[2].AnotherPendingTx = 0;                                 
                                pAd->pHifCfg->TxContext[3].AnotherPendingTx = 0; 
                                AnotherPendingTx = pAd->pHifCfg->TxContext[bulkResetPipeid].AnotherPendingTx; // for print
                            }
                            
                            pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
                            WdfWaitLockAcquire(pUsbDeviceContext->PipeStateLock, NULL);

                            WdfUsbTargetPipeAbortSynchronously(
                                            pUsbDeviceContext->BulkOutPipeHandle[bulkResetPipeid], 
                                            WDF_NO_HANDLE,
                                                                    NULL
                                            );

                            WdfWaitLockRelease(pUsbDeviceContext->PipeStateLock);

                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("pAd->pHifCfg->PendingTx[0] = %d\n",pAd->pHifCfg->PendingTx[0]));

                            {
                                RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
                                TxRtyCfgtmp.word = TxRtyCfg.word;
                                TxRtyCfg.field.LongRtyLimit = 0x4;
                                TxRtyCfg.field.ShortRtyLimit = 0x7;
                                RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

                                //
                                // Update RTS retry limit
                                //
                                RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                                RtsCfgtmp.word = RtsCfg.word;
                                RtsCfg.field.AutoRtsRetryLimit = 0;
                                RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);

                                Index = 0;

                            // emulation, the TXRXQ_PCNT is set to 0 only when PBF is reset(0x0400).,
                            // why do we add this code here ? 
                            /*
                                do 
                                {
                                    RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);
                                    if ((MACValue & 0xffffff) == 0)
                                        break;
                                    Index++;
                                    NdisCommonGenericDelay(1000);
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("MTK_OID_N5_QUERY_USB_RESET_BULK_OUT: MACValue = [0x%08X]\n",MACValue));                                
                                }while((Index < 1000)&&(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)));
                            */
                                                        }

                            RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
                            TxRtyCfg.field.LongRtyLimit = TxRtyCfgtmp.field.LongRtyLimit;
                            TxRtyCfg.field.ShortRtyLimit = TxRtyCfgtmp.field.ShortRtyLimit;
                            RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

                            RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                            RtsCfg.field.AutoRtsRetryLimit = RtsCfgtmp.field.AutoRtsRetryLimit;
                            RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);    


                            
                            MACValue = 0;
                            RTUSBReadMACRegister(pAd, USB_DMA_CFG, &MACValue);
                            // To prevent Read Register error, we 2nd check the validity.
                            // enable TX, RX
                            if ((MACValue & 0xc00000) == 0)
                                RTUSBReadMACRegister(pAd, USB_DMA_CFG, &MACValue);
                            
                            // To prevent Read Register error, we 3rd check the validity.
                            if ((MACValue & 0xc00000) == 0)
                                RTUSBReadMACRegister(pAd, USB_DMA_CFG, &MACValue);

                            // clear TX path
                            MACValue |= 0x80000;
                            RTUSBWriteMACRegister(pAd, USB_DMA_CFG, MACValue);
                            
                            MACValue &= (~0x80000);
                            RTUSBWriteMACRegister(pAd, USB_DMA_CFG, MACValue);
                            
                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Set 0x2a0 bit19. Clear USB DMA TX path\n"));
                            //MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
                            // Wait 10ms to prevent next URB to bulkout before HW reset. by MAXLEE 12-25-2007                           
                            //NdisCommonGenericDelay(1000);

                            //
                            // On Vista OS, it must call RTUSB_ResetPipe to recover.
                            // 0: for Bulk In Pipe
                            // 1: for Bulk out pipe #0
                            // 2: for Bulk out pipe #1
                            // ...
                            //
                            DBGPRINT(RT_DEBUG_TRACE, ("*********** Reset Bulk Out Pipe #%d Index = %d***********\n", pAd->pHifCfg->BulkFailParm.BulkResetPipeid,Index));

                            // If the BULKOUT reset causes by DATA frame, we will re-bulkout it again.
                            if( typeBulkOutRest == BULKOUT_RESET_TYPE_DATA )
                            {
                                MT_SET_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);    
                                //RTUSB_ResetPipe(pAd, pAd->pHifCfg->BulkFailParm.BulkResetPipeid + 1);
                                
                                ntStatus = WdfUsbTargetPipeResetSynchronously(
                                                pUsbDeviceContext->BulkOutPipeHandle[bulkResetPipeid], 
                                                WDF_NO_HANDLE,
                                                                        NULL
                                                );

                                //Reset other endpoints
                                for (Index = 0; Index < pAd->pHifCfg->BulkOutPipeNum; Index++)
                                {
                                    if((Index != bulkResetPipeid) && ((pAd->pHifCfg->UsedEndpoint&(1<<Index))==(1<<Index)))
                                    {
                                        ntStatus = WdfUsbTargetPipeResetSynchronously(
                                                        pUsbDeviceContext->BulkOutPipeHandle[Index], 
                                                        WDF_NO_HANDLE,
                                                                                NULL
                                                        );
                                        pAd->pHifCfg->BulkFailParm.BulkResetPipeid &= ~(1<<Index);                                   
                                    }
                                }
                            
                                NdisCommonGenericDelay(10000);

                                DBGPRINT(RT_DEBUG_TRACE, ("**Prepare ReBulkOut the ResetPipeID = %d, AnotherPendingTx = 0x%02x\n", pAd->pHifCfg->BulkFailParm.BulkResetPipeid,AnotherPendingTx));
                                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);  
//                              N6UsbXmitStart(pAd);

                                // Re-BulkOut may cause bulkout sequence less than last indicate sequence, 
                                // and Intel 4965 STA will send abnormal BA when Re-BulkOut occurs.
                                // (Abnormal BA means that Starting Sequence Control and BlockAck Bitmap are 0)
                                if (pPort->CommonCfg.IOTestParm.bIntel4965 != TRUE)
                                {
                                    UCHAR           IrpIdx = 0, BulkOutPipeId = 0;                                  

                                    for (BulkOutPipeId = 0; BulkOutPipeId < 4; BulkOutPipeId ++)
                                    {
                                        pTxContext = &(pAd->pHifCfg->TxContext[BulkOutPipeId]);

#if 1
                                        if ((pTxContext->AnotherPendingTx&0x30) == 0x30)
                                        {
                                            XmitSendMlmeCmdPkt(pAd, TXPKT_NULL_FRAME);
                                            DBGPRINT(RT_DEBUG_TRACE, ("EP#%d. PendingIRP number = 2, AnotherPendingTx = 0x%02x, First IrpIndex = %d, Second IrpIndex = %d\n", BulkOutPipeId, pTxContext->AnotherPendingTx,(pTxContext->AnotherPendingTx&0x01),((pTxContext->AnotherPendingTx&0x02)>>1)));
                                            RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, (pTxContext->AnotherPendingTx&0x01));
                                            RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, ((pTxContext->AnotherPendingTx&0x02)>>1));                                                      
                                        }
                                        else if ((pTxContext->AnotherPendingTx&0x10) == 0x10)
                                        {
                                            XmitSendMlmeCmdPkt(pAd, TXPKT_NULL_FRAME);
                                            DBGPRINT(RT_DEBUG_TRACE, ("EP#%d. PendingIRP number = 1, AnotherPendingTx = 0x%02x, First IrpIndex = %d\n", BulkOutPipeId, pTxContext->AnotherPendingTx, (pTxContext->AnotherPendingTx&0x01)));
                                            RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, (pTxContext->AnotherPendingTx&0x01));                                                       
                                        }
#else
                                        if (pTxContext->AnotherPendingTx != 0)
                                        {
                                            //for (IrpIdx = 0; IrpIdx < MAX_WRITE_USB_REQUESTS; IrpIdx ++)
                                            {
                                                //if ((pTxContext->AnotherPendingTx & (1 << IrpIdx)) != 0)
                                                //  RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, IrpIdx);
                                                if ((pTxContext->AnotherPendingTx & 0xF0) == 0x10)
                                                {
                                                    RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, 0);
                                                }
                                                else if ((pTxContext->AnotherPendingTx & 0xF0) == 0x20)
                                                {
                                                    RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, 1);
                                                }

                                                if ((pTxContext->AnotherPendingTx & 0x0F) == 0x01)
                                                {
                                                    RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, 0);
                                                }
                                                else if ((pTxContext->AnotherPendingTx & 0x0F) == 0x02)
                                                {
                                                    RTUSB_ReBulkOut(pAd, pTxContext, BulkOutPipeId, 1);
                                                }
                                            }
                                        }
#endif
                                    }
                                }
                            }
                            else
                            {
                                //Besides DATA frame, all reset pipe 0
                                
                                //
                                // We must call WdfUsbTargetPipeResetSynchronously 
                                // Send a clear stall URB. Pipereset will clear the stall.
                                // Otherwise, any bulkout will not complete.
                                //
                                MT_SET_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);    
                                
                                ntStatus = WdfUsbTargetPipeResetSynchronously(
                                                pUsbDeviceContext->BulkOutPipeHandle[ENDPOINT_OUT_4], 
                                                WDF_NO_HANDLE,
                                                                        NULL
                                                );
                                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);  
                            }

                            

                            pAd->pHifCfg->BulkFailParm.BulkResetPipeid = 0;
                            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
                            DBGPRINT(RT_DEBUG_TRACE, ("Clear fRTMP_ADAPTER_BULKOUT_RESET!!!\n"));                           

                            

                            DBGPRINT(RT_DEBUG_TRACE, ("remove N6UsbXmitStart after Clear fRTMP_ADAPTER_BULKOUT_RESET!!!\n"));
                            //N6UsbXmitStart(pAd);
                            // In BULKOUT_RESET process, will block N6USBKickBulkOut routine. After BULKOUT_RESET, we should kick it out ASAP.
                            if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && 
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
                            {
                                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
                            }
                        }

                        DBGPRINT_RAW(RT_DEBUG_ERROR, ("CmdThread : MTK_OID_N5_QUERY_USB_RESET_BULK_OUT<===\n"));
                        break;

                    case MT_CMD_USB_RESET_BULK_IN:
                        DBGPRINT_RAW(RT_DEBUG_ERROR, ("CmdThread : MT_CMD_USB_RESET_BULK_IN === >\n"));
                        //
                        // From DDK,
                        // All transfers must be aborted or cancelled before attempting to reset the pipe.
                        //  
                        {
                            UCHAR   BulkInIndex;
                            ULONG   MACValue;
                            ULONG   i = 0;
                            PUSB_DEVICE_CONTEXT pUsbDeviceContext;
                            do
                            {
                                // To avoid Andes wakeup when sleep
                                if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
                                    break;

                                ntStatus = RTUSBReadMACRegister(pAd, (USHORT)GET_ASIC_VERSION_ID(pAd), &MACValue);
                                if ((NT_SUCCESS(ntStatus) == TRUE) ||(ntStatus == STATUS_DEVICE_NOT_CONNECTED) || 
                                     (ntStatus == STATUS_DEVICE_REMOVED) ||
                                     (ntStatus == STATUS_NO_SUCH_DEVICE) )
                                    break;
                                NdisCommonGenericDelay(1000);
                                i++;
                            }while(i < 5);

                            if ((NT_SUCCESS(ntStatus) == TRUE)&& (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
                            {
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("0x%x/0x%x/ 0x%x. ::x%x/0x%x/ 0x%x.n", pAd->pHifCfg->NextRxBulkInIndex,  pAd->pHifCfg->NextRxBulkInReadIndex, pAd->pHifCfg->NextRxBulkInPosition, pAd->pHifCfg->BulkInReq, pAd->pHifCfg->BulkInComplete, pAd->pHifCfg->BulkInCompleteFail));
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("[0] IRPPending=%d:  InUse=%d: Readable=%d: [1] IRPPending=%d:  InUse=%d: Readable=%d: [2 IRPPending=%d:  InUse=%d: Readable=%d: [3] IRPPending=%d:  InUse=%d: Readable=%d: \n"
                                    , pAd->pHifCfg->RxContext[0].IRPPending, pAd->pHifCfg->RxContext[0].InUse, pAd->pHifCfg->RxContext[0].Readable
                                    , pAd->pHifCfg->RxContext[1].IRPPending, pAd->pHifCfg->RxContext[1].InUse, pAd->pHifCfg->RxContext[1].Readable
                                    , pAd->pHifCfg->RxContext[2].IRPPending, pAd->pHifCfg->RxContext[2].InUse, pAd->pHifCfg->RxContext[2].Readable
                                    , pAd->pHifCfg->RxContext[3].IRPPending, pAd->pHifCfg->RxContext[3].InUse, pAd->pHifCfg->RxContext[3].Readable));
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("[4] IRPPending=%d:  InUse=%d: Readable=%d: [5] IRPPending=%d:  InUse=%d: Readable=%d:[6] IRPPending=%d:  InUse=%d: Readable=%d: [7] IRPPending=%d:  InUse=%d: Readable=%d: \n"
                                    , pAd->pHifCfg->RxContext[4].IRPPending, pAd->pHifCfg->RxContext[4].InUse, pAd->pHifCfg->RxContext[4].Readable
                                    , pAd->pHifCfg->RxContext[5].IRPPending, pAd->pHifCfg->RxContext[5].InUse, pAd->pHifCfg->RxContext[5].Readable
                                    , pAd->pHifCfg->RxContext[6].IRPPending, pAd->pHifCfg->RxContext[6].InUse, pAd->pHifCfg->RxContext[6].Readable
                                    , pAd->pHifCfg->RxContext[7].IRPPending, pAd->pHifCfg->RxContext[7].InUse, pAd->pHifCfg->RxContext[7].Readable));                              

                                //
                                // On Vista OS, it must call RTUSB_ResetPipe to recover.
                                // 0: for Bulk In Pipe
                                // 1: for Bulk out pipe #0
                                // 2: for Bulk out pipe #1
                                // ...
                                //
                                
                                DBGPRINT(RT_DEBUG_TRACE, ("*********** Reset BulkIn Pipe ***********\n"));
                                N6UsbRecvStop(pAd);
                                
                                //RTUSB_ResetPipe(pAd, 0);
                                                    MT_SET_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);    

                                pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
                                ntStatus = WdfUsbTargetPipeResetSynchronously(
                                                    pUsbDeviceContext->BulkInPipeHandle[ENDPOINT_IN_84], 
                                                        WDF_NO_HANDLE,
                                                        NULL
                                                        );
                                                    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);                                  

                                NdisCommonGenericDelay(10000);
                                N6UsbRecvStart(pAd);
                                                    
                                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
                                // ==========

                                for (BulkInIndex = 0; BulkInIndex < pAd->pNicCfg->NumOfBulkInIRP; BulkInIndex++ )
                                {
                                    if( pAd->pHifCfg->PendingRx < pAd->pNicCfg->NumOfBulkInIRP)
                                {
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("N6USBBulkReceive\n"));                               
                                    N6USBBulkReceive(pAd);
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("NextRxBulkInReadIndex = %d, NextRxBulkInIndex = %d \n",  pAd->pHifCfg->NextRxBulkInReadIndex,pAd->pHifCfg->NextRxBulkInIndex ));                                                                   

                                    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("N6UsbRxStaPacket\n"));                                        
                                    //N6UsbRxStaPacket(pAd, FALSE);    
                                    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("NextRxBulkInReadIndex = %d, NextRxBulkInIndex = %d, PendingRx = %d \n",  pAd->pHifCfg->NextRxBulkInReadIndex,pAd->pHifCfg->NextRxBulkInIndex,pAd->pHifCfg->PendingRx ));
                                }
                                }

                            }
                            else
                            {
                                ntStatus = RTUSB_GetPortStatus(pAd/*, &portStatus*/);
                                if (!NT_SUCCESS(ntStatus))
                                    {
                                    //if (!(portStatus & USBD_PORT_CONNECTED))
                                    //{
                                        MT_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
                                        DBGPRINT_RAW(RT_DEBUG_ERROR, ("Device has been removed\n"));    

                                        PlatformIndicateScanStatus(pAd, pPort, NDIS_STATUS_SUCCESS, TRUE, FALSE);
                                    //}
                                }
                                else
                                {
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Card exist\n"));                 
                                }

                                //if card exist ,need to reset pipe
                                if ( !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                                {
                                                
                                    //
                                    // We must call WdfUsbTargetPipeResetSynchronously 
                                    // Send a clear stall URB. Pipereset will clear the stall.
                                    // Otherwise, any bulkout will not complete.
                                    //
                                    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
                                    ntStatus = WdfUsbTargetPipeResetSynchronously(
                                        pUsbDeviceContext->BulkOutPipeHandle[ENDPOINT_OUT_4], 
                                        WDF_NO_HANDLE,
                                                                NULL
                                        );                              
                                }

                                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("Read Register Failed!\n"));
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("\n"));
                            }
                        }
                        DBGPRINT_RAW(RT_DEBUG_ERROR, ("CmdThread : MT_CMD_USB_RESET_BULK_IN <===\n"));
                    break;
                        
                    case MT_CMD_SET_ASIC_WCID:
                        // ToDo : check later : Lens
                        {
                            RT_SET_ASIC_WCID    SetAsicWcid;
     
                            SetAsicWcid = *((PRT_SET_ASIC_WCID)(pData));
                            
                            if (SetAsicWcid.WCID >= MAX_LEN_OF_MAC_TABLE)
                            {
                                DBGPRINT(RT_DEBUG_TRACE, ("Cmd : MT_CMD_SET_ASIC_WCID : Invalid WCID = %d\n", SetAsicWcid.WCID));
                                break;
                            }
                            
                            if (PlatformEqualMemory(SetAsicWcid.Addr, ZERO_MAC_ADDR, MAC_ADDR_LEN))
                            {
                                MtAsicDelWcidTab(pAd, (UCHAR)SetAsicWcid.WCID);
                            }
                            
#if 0                            
                            offset = MAC_WCID_BASE + ((UCHAR)SetAsicWcid.WCID)*HW_WCID_ENTRY_SIZE;

                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Cmd : MT_CMD_SET_ASIC_WCID : WCID = %d, SetTid  = %x, DeleteTid = %x.\n", SetAsicWcid.WCID, SetAsicWcid.SetTid, SetAsicWcid.DeleteTid));
                            MACValue = (pWcidMacTabEntry->Addr[3]<<24)+(pWcidMacTabEntry->Addr[2]<<16)+(pWcidMacTabEntry->Addr[1]<<8)+(pWcidMacTabEntry->Addr[0]);
                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("1-MACValue= %x,\n", MACValue));
                            RTUSBWriteMACRegister(pAd, offset, MACValue);
                            // Read bitmask
                            RTUSBReadMACRegister(pAd, offset+4, &MACRValue);
                            if ( SetAsicWcid.DeleteTid != 0xffffffff)
                                MACRValue &= (~SetAsicWcid.DeleteTid);
                            if (SetAsicWcid.SetTid != 0xffffffff)
                                MACRValue |= (SetAsicWcid.SetTid);
                            MACRValue &= 0xffff0000;
                        
                            MACValue = (pWcidMacTabEntry->Addr[5]<<8)+pWcidMacTabEntry->Addr[4];
                            MACValue |= MACRValue;
                            RTUSBWriteMACRegister(pAd, offset+4, MACValue);
                            
                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("2-MACValue= %x,\n", MACValue));
#endif                            
                        }
                        break;
                    case MT_CMD_SET_ASIC_WCID_CIPHER:
                        {
                            RT_SET_ASIC_WCID_ATTRI  SetAsicWcidAttri;
                            USHORT                  offset;
                            ULONG                   MACRValue = 0;
                            SHAREDKEY_MODE_STRUC    csr1;
                            UCHAR                   BssIndex = BSS0;
                            
                            SetAsicWcidAttri = *((PRT_SET_ASIC_WCID_ATTRI)(pData));
                            
                            if (SetAsicWcidAttri.WCID >= MAX_LEN_OF_MAC_TABLE)
                                break;
                            
                            if ((pAd->NumberOfPorts > 1) && (pAd->StaCfg.BssType == BSS_INFRA))
                            {
                                pPort = pAd->PortList[SetAsicWcidAttri.PortNum];
                                BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
                                DBGPRINT_RAW(RT_DEBUG_TRACE, ("Cmd : MT_CMD_SET_ASIC_WCID_CIPHER pPort->CurrentAddress[5]  = %x :\n", pPort->CurrentAddress[5] ));
                            }

                            
                            offset = MAC_WCID_ATTRIBUTE_BASE + ((UCHAR)SetAsicWcidAttri.WCID)*HW_WCID_ATTRI_SIZE;

                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Cmd : MT_CMD_SET_ASIC_WCID_CIPHER : WCID = %d, Cipher = %x.\n", SetAsicWcidAttri.WCID, SetAsicWcidAttri.Cipher));
                            // Read bitmask
                            RTUSBReadMACRegister(pAd, offset, &MACRValue);
                            MACRValue = 0;
                            MACRValue  = MACRValue |  ((UCHAR)SetAsicWcidAttri.Cipher << 1) | (BssIndex << 4) ;

                            RTUSBWriteMACRegister(pAd, offset, MACRValue);
                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("2-offset = %x , MACValue= %x,\n", offset, MACRValue));
                            
                            offset = PAIRWISE_IVEIV_TABLE_BASE + ((UCHAR)SetAsicWcidAttri.WCID)*HW_IVEIV_ENTRY_SIZE;
                            MACRValue = 0;                          
                            if ( (SetAsicWcidAttri.Cipher <= CIPHER_WEP128))
                                MACRValue |= ( pPort->PortCfg.DefaultKeyId << 30);
                            else
                                MACRValue |= (0x20000000);
                            RTUSBWriteMACRegister(pAd, offset, MACRValue);
                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("2-offset = %x , MACValue= %x,\n", offset, MACRValue));

                            //
                            // Update cipher algorithm. WSTA always use BSS0                            
                            //
                            // for adhoc mode only ,because wep status slow than add key, when use zero config
                            if (pAd->StaCfg.BssType == BSS_ADHOC )
                            {
                                offset = MAC_WCID_ATTRIBUTE_BASE;

                                RTUSBReadMACRegister(pAd, offset, &MACRValue);
                                MACRValue &= (~0xe);
                                MACRValue |= (((UCHAR)SetAsicWcidAttri.Cipher) << 1);

                                RTUSBWriteMACRegister(pAd, offset, MACRValue);

                                //Update group key cipher,,because wep status slow than add key, when use zero config
                                RTUSBReadMACRegister(pAd, SHARED_KEY_MODE_BASE+4*(0/2), &csr1.word);
                                
                                        csr1.field.Bss0Key0CipherAlg = SetAsicWcidAttri.Cipher;
                                        csr1.field.Bss0Key1CipherAlg = SetAsicWcidAttri.Cipher;
                                                                
                                RTUSBWriteMACRegister(pAd, SHARED_KEY_MODE_BASE+4*(0/2), csr1.word);
                            }
                        }
                        break;
                    case MT_CMD_SET_DLS_KEY_TABLE:
                        {
                            RT_SET_DLS_WCID_KEY  DLS_Cipher;
                            DLS_Cipher  = *((PRT_SET_DLS_WCID_KEY)(pData));
                            AsicAddKeyEntry(pAd,pPort, (USHORT)DLS_Cipher.WCID , BSS0, 0, &DLS_Cipher.CipherKey, TRUE, TRUE);   // reserve 0 for multicast, 1 for unicast
                        }
                        break;
                        
                    case MT_CMD_SET_DLS_RX_WCID_TABLE:
                        {
                            RT_SET_ASIC_WCID    SetAsicWcid;
                            SetAsicWcid = *((PRT_SET_ASIC_WCID)(pData));
                            MtAsicUpdateRxWCIDTable(pAd, (USHORT)(SetAsicWcid.WCID + 2), SetAsicWcid.Addr);
                        }
                        break;
                        
                    case MT_CMD_ASIC_ADD_SKEY:
                        pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
                        AsicAddKeyEntry(pAd,pPort, 0, BSS0, pPort->PortCfg.DefaultKeyId, &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId], FALSE, TRUE);
                        break;
                        
                    case MT_CMD_USB_VENDOR_RESET:
                        N6UsbIoVendorRequest(pAd,
                                                0,
                                                DEVICE_VENDOR_REQUEST_OUT,
                                                1,
                                                1,
                                                0,
                                                NULL,
                                                0);
                        break;

                    case MT_CMD_802_11_SET_PHY_MODE:
                        {
                    ULONG   phymode = *(ULONG *)(pData);
                            
                            // Bit 16 true -- force to build channel list
                            MlmeInfoSetPhyMode(pAd, pPort, (phymode&0xff),(phymode&0x00010000)?(TRUE):(FALSE));
                            
                            //RestartAPIsRequired = TRUE;
                            DBGPRINT(RT_DEBUG_ERROR, ("SetCMD::MTK_OID_N5_SET_PHY_MODE (=%s)\n", DecodePhyMode(phymode)));
                        }
                        break;

                    case  MT_CMD_PERFORM_SOFT_DIVERSITY:
                        // TODO: Can't get the corrent port number here??
                        if ((pAd->OpMode == OPMODE_AP) ||
                            (IS_P2P_STA_GO(pAd, pAd->PortList[PORT_0])) ||
                            (IS_P2P_CON_GO(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) ||
                            (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber])))
                            ApMlmeAsicRxAntEvalTimeout(pAd);
                        else
                            AsicRxAntEvalAction(pAd);

                        break;
                        
                    case MT_CMD_MLME_RESTART_STATE_MACHINE:
                        {
                            // Restart MLME state machine.
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: Call MlmeRestartStateMachine\n", __FUNCTION__));    
                            MlmeRestartStateMachine(pAd,pPort);
                        }
                        break;

                    //for WPS LED MODE 10 
                    case MT_CMD_LED_WPS_MODE10:
                        {
                            WPSLedMode10 = *((PULONG)(pData));
                            DBGPRINT(RT_DEBUG_TRACE, ("WPS LED MODE10--ON or Flash or OFF : %x\n", WPSLedMode10));
                            switch(WPSLedMode10){
                        
                                case LINK_STATUS_WPS_MODE10_TURN_ON:
                                    LedCtrlSetLed(pAd, LED_WPS_MODE10_TURN_ON);
                                    break;
                                
                                case LINK_STATUS_WPS_MODE10_FLASH:
                                    LedCtrlSetLed(pAd,LED_WPS_MODE10_FLASH);
                                    break;
                        
                                case LINK_STATUS_WPS_MODE10_TURN_OFF:
                                    LedCtrlSetLed(pAd, LED_WPS_MODE10_TURN_OFF);
                                    break;
                                default:
                                    DBGPRINT(RT_DEBUG_TRACE, ("WPS LED MODE 10--no this status!!!\n"));
                                    break;
                            }
                        }   
                        break;

                    case MT_CMD_TXBF_NDPANNOUNCE_GEN:
                    {
                        if(!pAd->LogoTestCfg.OnTestingWHQL)
                            TxBfNDPAnnouncementGeneration(pAd, pPort);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s MT_CMD_TXBF_NDPANNOUNCE_GEN call TxBfNDPAnnouncementGeneration().\n", __FUNCTION__)); 
                    }
                    break;

                    default:
                        DBGPRINT(RT_DEBUG_ERROR, ("--> Control Thread !! ERROR !! default ???? \n"));    

                        break;
                }  // switch (cmdqelmt->command)
            } //if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    else if (Cmd == MTK_OID_N5_SET_HARDWARE_REGISTER)
            {
#if 1 //(defined(_WIN64) || DBG)            
                NdisStatus = RTUSBSetHardWareRegister(pAd, pData);
#endif              
            }
            else
            {
                DBGPRINT(RT_DEBUG_INFO, ("Device not connect or Halt in progress\n"));
            }
                }

VOID MTUSBCmdThread(
    IN OUT PMP_ADAPTER pAd
    )
            {
    PCmdQElmt   cmdqelmt;
    PUCHAR      pData;
    ULONG       DataLen = 0;
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    NTSTATUS    ntStatus = NDIS_STATUS_SUCCESS;

    DBGPRINT(RT_DEBUG_TRACE, ("Command Thread Triggered\n"));
    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    while (!pAd->TerminateThreads)
        {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pHifCfg->hControlThread)),0);   
            
        DBGPRINT(RT_DEBUG_INFO, ("Command Thread Triggered\n"));
        while (pAd->pHifCfg->CmdQ.size > 0)
            {
            NdisAcquireSpinLock(&pAd->pHifCfg->CmdQLock);
            RTUSBDequeueCmd(&pAd->pHifCfg->CmdQ, &cmdqelmt);
            NdisReleaseSpinLock(&pAd->pHifCfg->CmdQLock);
            if (cmdqelmt == NULL)
                {
                break;
                }
                
                pData = cmdqelmt->buffer;
                DataLen = cmdqelmt->bufferlength;
                
                MTUSBCmdHander(pAd->PortList[cmdqelmt->PortNum], cmdqelmt->command, pData, DataLen);
              
            MtReturnInternalCmdBuffer(pAd, cmdqelmt);
            
        }// while (pAd->CmdQ.size > 0)
        DBGPRINT(RT_DEBUG_INFO, ("Command Thread Finished\n"));//steven:for debug

    }//while (!Adapter->TerminateThreads)

    DBGPRINT_RAW(RT_DEBUG_INFO, ("+ + + + Control Thread Terminated + + + + \n"));

    //DecrementIoCount(pAd);
}

#if 1 //(defined(_WIN64) || DBG)

NDIS_STATUS QueryHardWareRegister(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuf)
{
    PRT_802_11_CR_ACCESS            pCRAccess;
    NDIS_STATUS                     Status = NDIS_STATUS_SUCCESS;   

    pCRAccess = (PRT_802_11_CR_ACCESS) pBuf;

    if (pCRAccess->HardwareType == HARDWARE_MAC)
    {
        HW_IO_READ32(pAd, pCRAccess->CR, &pCRAccess->Data);
        DBGPRINT(RT_DEBUG_TRACE, ("Query MAC : QueryHardWareRegister : CR[0x%x]=[0x%x]\n", pCRAccess->CR, pCRAccess->Data));
    }
    else if (pCRAccess->HardwareType == HARDWARE_RF)
    {
        ULONG   RFAddr, PollingValue, QueryValue;
        ULONG   CRAddress1 = 0x60200E04; // Set RF Address
        ULONG   CRAddress2 = 0x60200E10; // polling
        ULONG   CRAddress3 = 0x60200E0C; // Get RF Value

        RFAddr = (0x00010000) | (pCRAccess->CR & 0x00020FFF);
        HW_IO_WRITE32(pAd, CRAddress1, RFAddr);

        do
        {
            HW_IO_READ32(pAd, CRAddress2, &PollingValue);
            if ((PollingValue & 0x00002000) == 0)
                break;
            
        } while (TRUE);

        HW_IO_READ32(pAd, CRAddress3, &QueryValue);

        pCRAccess->Data = QueryValue;
        
        
        DBGPRINT(RT_DEBUG_TRACE, ("Query RF : QueryHardWareRegister : CR[0x%x]=[0x%x]\n", RFAddr, QueryValue));
    }
    else if (pCRAccess->HardwareType == HARDWARE_BBP)
    {
        //USBAccessCR(pAd, CR_WRITE, pCRAccess->CR, (PUCHAR)&pCRAccess->Data, sizeof(ULONG));
        //DBGPRINT(RT_DEBUG_TRACE, ("Set BBP : SetHardWareRegister : CR[0x%x]=[0x%x]\n", pCRAccess->CR, pCRAccess->Data));
    }
    else if (pCRAccess->HardwareType == HARDWARE_EEP)
    {
        //USBAccessCR(pAd, CR_WRITE, pCRAccess->CR, (PUCHAR)&pCRAccess->Data, sizeof(ULONG));
        //DBGPRINT(RT_DEBUG_TRACE, ("Set BBP : SetHardWareRegister : CR[0x%x]=[0x%x]\n", pCRAccess->CR, pCRAccess->Data));
    }

    return Status;
}
NDIS_STATUS SetHardWareRegister(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuf)
{
    PRT_802_11_CR_ACCESS            pCRAccess;
    NDIS_STATUS                     Status = NDIS_STATUS_SUCCESS;   
    
    pCRAccess = (PRT_802_11_CR_ACCESS) pBuf;
    
    if (pCRAccess->HardwareType == HARDWARE_MAC)
    {
        HW_IO_WRITE32(pAd, pCRAccess->CR, pCRAccess->Data);
        DBGPRINT(RT_DEBUG_TRACE, ("Set MAC : SetHardWareRegister : CR[0x%x]=[0x%x]\n", pCRAccess->CR, pCRAccess->Data));
    }
    else if (pCRAccess->HardwareType == HARDWARE_RF)
    {
        ULONG   RFAddr, PollingValue, SetValue;
        ULONG   CRAddress1 = 0x60200E00; // Set RF Value
        ULONG   CRAddress2 = 0x60200E04; // Set RF Address
        ULONG   CRAddress3 = 0x60200E10; // polling

        RFAddr = (0x00011000) | (pCRAccess->CR & 0x00020FFF);
        SetValue = pCRAccess->Data;
        HW_IO_WRITE32(pAd, CRAddress1, SetValue);
        HW_IO_WRITE32(pAd, CRAddress2, RFAddr);

        do
        {
            HW_IO_READ32(pAd, CRAddress3, &PollingValue);
            if ((PollingValue & 0x00002000) == 0)
                break;
            
        } while (TRUE);
        
        
        DBGPRINT(RT_DEBUG_TRACE, ("Set RF : SetHardWareRegister : CR[0x%x]=[0x%x]\n", RFAddr, SetValue));
    }
    else if (pCRAccess->HardwareType == HARDWARE_BBP)
    {
        //USBAccessCR(pAd, CR_WRITE, pCRAccess->CR, (PUCHAR)&pCRAccess->Data, sizeof(ULONG));
        //DBGPRINT(RT_DEBUG_TRACE, ("Set BBP : SetHardWareRegister : CR[0x%x]=[0x%x]\n", pCRAccess->CR, pCRAccess->Data));
    }
    else if (pCRAccess->HardwareType == HARDWARE_EEP)
    {
        //USBAccessCR(pAd, CR_WRITE, pCRAccess->CR, (PUCHAR)&pCRAccess->Data, sizeof(ULONG));
        //DBGPRINT(RT_DEBUG_TRACE, ("Set BBP : SetHardWareRegister : CR[0x%x]=[0x%x]\n", pCRAccess->CR, pCRAccess->Data));
    }

    return Status;
}

NDIS_STATUS RTUSBSetHardWareRegister(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuf)
{
    PRT_802_11_HARDWARE_REGISTER    pHardwareRegister;
    ULONG                           Value;
    USHORT                          Offset;
    UCHAR                           bbpValue, rfValue = 0;
    UCHAR                           bbpID, rfID = 0;
    USHORT                          eepID;
    USHORT                          eepValue;
    NDIS_STATUS                     Status = NDIS_STATUS_SUCCESS;   
    NTSTATUS                        ntStatus = STATUS_SUCCESS;
//  ULONG                           portStatus = 0;
    PMP_PORT                      pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    PUSB_DEVICE_CONTEXT                 usbDeviceContext;
    BOOLEAN                         bP2PSigmaPos = FALSE;
    ULONG MacValue = 0;
    ULONG TotalPacktCount =0;
    //ULONG i;
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    PMAC_TABLE_ENTRY pMbcastMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);
    
    pHardwareRegister = (PRT_802_11_HARDWARE_REGISTER) pBuf;

    if (((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ))
             return NDIS_STATUS_INVALID_DATA;

    
    if (pHardwareRegister->HardwareType == HARDWARE_MAC)
    {

        //2 TODO: Remove, this is only for IC verify
        if(EXT_MAC_CAPABLE(pAd))
        {
            if ( (pHardwareRegister->Offset & 0xffff) == 0xA7F2 )
            {
                UCHAR Value;
                ULONG ExtBbValue = 0;
                DBGPRINT(RT_DEBUG_TRACE,("[6x9x VHT backdoor] RegisterDump --> \n"));
                        
                BB_READ32(pAd, BB_BLOCK_CORE, BB_OFFSET_R1, &ExtBbValue);
                DBGPRINT(RT_DEBUG_TRACE,("[6x9x VHT backdoor] RegisterDump, BBPReg(BB_BLOCK_CORE, BB_OFFSET_R1)=0x%08x\n",
                            ExtBbValue));
                        
                BB_READ32(pAd, BB_BLOCK_AGC, BB_OFFSET_R0, &ExtBbValue);
                DBGPRINT(RT_DEBUG_TRACE,("[6x9x VHT backdoor] RegisterDump, BBPReg(BB_BLOCK_AGC, BB_OFFSET_R0)=0x%08x\n",
                            ExtBbValue));
                        
                BB_READ32(pAd, BB_BLOCK_TX_BACKEND, BB_OFFSET_R0, &ExtBbValue);
                DBGPRINT(RT_DEBUG_TRACE,("[6x9x VHT backdoor] RegisterDump, BBPReg(BB_BLOCK_TX_BACKEND, BB_OFFSET_R0)=0x%08x\n",
                            ExtBbValue));

                RT30xxReadRFRegister(pAd, RF_R17, &Value);
                DBGPRINT(RT_DEBUG_TRACE,("[6x9x VHT backdoor] RegisterDump, RF_R17=0x%02x\n",Value));

                RT30xxReadRFRegister(pAd, RF_R22, &Value);
                DBGPRINT(RT_DEBUG_TRACE,("[6x9x VHT backdoor] RegisterDump, RF_R22=0x%02x\n",Value));
                        
                        
                DBGPRINT(RT_DEBUG_TRACE,("[6x9x VHT backdoor] RegisterDump <-- \n"));
                return Status;
            }
            else if ((pHardwareRegister->Offset&0xffff) == 0x77f1)
            {
                PHY_CFG PhyCfg;
                PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                if(pBssidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                    return NDIS_STATUS_INVALID_DATA;
                }  
                
                PhyCfg.DoubleWord = pHardwareRegister->Data;
                WRITE_PHY_CFG_STBC(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_STBC(pAd, &PhyCfg));
                WRITE_PHY_CFG_SHORT_GI(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_SHORT_GI(pAd, &PhyCfg));
                WRITE_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_MCS(pAd, &PhyCfg));
                WRITE_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_MODE(pAd, &PhyCfg));
                WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_BW(pAd, &PhyCfg));
                pPort->CommonCfg.bAutoTxRateSwitch = FALSE;
                return Status;
            }
        }
            
        if (((pHardwareRegister->Offset&0xffff) == 0x0000) && ((pHardwareRegister->Data&0xffff) == 0x0))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("USB vender reset\n"));
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_USB_VENDOR_RESET, NULL, 0);
        }

        if ((pHardwareRegister->Offset&0xffff) == 0x7770 || (pHardwareRegister->Offset&0xffff) == 0x77f0)
            bP2PSigmaPos = TRUE;

        Offset = (USHORT) pHardwareRegister->Offset;
        Value = (ULONG) pHardwareRegister->Data;
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) && 
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
                RTUSBWriteMACRegister(pAd, Offset, Value);      
        DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::MTK_OID_N5_SET_HARDWARE_REGISTER (MAC offset=0x%08x, data=0x%08x)\n", pHardwareRegister->Offset, pHardwareRegister->Data));
        // 2004-11-08 a special 16-byte on-chip memory is used for RaConfig to pass debugging parameters to driver
        // for debug-tuning only
        if ((pHardwareRegister->Offset >= HW_DEBUG_SETTING_BASE))
        {
            // just for emulation, to be removed later
            if ((pHardwareRegister->Offset&0xffff) == 0xA7f0 && (pHardwareRegister->Data == 0x77))
            {               
                static ULONG RegValue = 0x12345566;
                DBGPRINT(RT_DEBUG_TRACE,("6x9x, BurstWrite\n"));
                BurstWrite(pAd, 0x418000, RegValue++, 1);   
                
            }
            else if ((pHardwareRegister->Offset&0xffff) == 0xA7f0 && (pHardwareRegister->Data == 0x88))
            {   
                DBGPRINT(RT_DEBUG_TRACE,("6x9x, BurstRead\n"));
                BurstRead(pAd, (0x418000), 1);              
            }
            else if ((pHardwareRegister->Offset&0xffff) == 0xA7f0 && (pHardwareRegister->Data == 0x1a))
            {
                DBGPRINT(RT_DEBUG_TRACE,("6x9x, pPort->CommonCfg.bWmmCapable = %d\n",pPort->CommonCfg.bWmmCapable));
            }          
            else
            // 0x2bf0: test power-saving feature
            if (pHardwareRegister->Offset == HW_DEBUG_SETTING_BASE)
            {
#if 0           
                ULONG isr, imr, gimr;
                USHORT tbtt = 3;

                RTUSBReadMACRegister(pAd, MCU_INT_SOURCE_CSR, &isr);
                RTUSBReadMACRegister(pAd, MCU_INT_MASK_CSR, &imr);
                RTUSBReadMACRegister(pAd, INT_MASK_CSR, &gimr);
                DBGPRINT(RT_DEBUG_TRACE, ("Sleep %d TBTT, 8051 IMR=%08x, ISR=%08x, MAC IMR=%08x\n", tbtt, imr, isr, gimr));
                AsicSleepThenAutoWakeup(pAd, tbtt); 
#endif              
                if (pHardwareRegister->Data == 0x8)
                {
                    pAd->pHifCfg->bForcePrintTX = TRUE;
                    DBGPRINT(RT_DEBUG_ERROR, ("bForcePrintTX = %d \n", pAd->pHifCfg->bForcePrintTX));
                }
                else if (pHardwareRegister->Data == 0x9)
                {
                    pAd->pHifCfg->bForcePrintTX = FALSE;
                    DBGPRINT(RT_DEBUG_ERROR, ("bForcePrintTX = %d \n", pAd->pHifCfg->bForcePrintTX));
                }
                else if (pHardwareRegister->Data == 0xa)
                {
                    pAd->StaCfg.LinkQualitySetting = 0x0;
                }
                else if (pHardwareRegister->Data == 0xb)
                {
                    pAd->StaCfg.LinkQualitySetting = 0x51;
                }
                else if (pHardwareRegister->Data == 0xe)
                {
                    pAd->pHifCfg->bForcePrintRX = TRUE;
                    DBGPRINT(RT_DEBUG_ERROR, ("bForcePrintRX = %d \n", pAd->pHifCfg->bForcePrintRX));
                }
                else if (pHardwareRegister->Data == 0xf)
                {
                    pAd->pHifCfg->bForcePrintRX = FALSE;
                    DBGPRINT(RT_DEBUG_ERROR, ("bForcePrintRX = %d \n", pAd->pHifCfg->bForcePrintRX));
                }
                else if (pHardwareRegister->Data == 0x31)
                {
                    UCHAR   i;
                    PTX_CONTEXT     pTxContext;
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("==========================================\n"));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("pAd->Flags=0x%x, pAd->BulkFlags=0x%x\n", pAd->Flags, pAd->pHifCfg->BulkFlags));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("pAd->pHifCfg->PrioRingFirstIndex = %d, PopMgmtIndex = %d, PushMgmtIndex = %d\n", pAd->pHifCfg->PrioRingFirstIndex,  pAd->pHifCfg->PopMgmtIndex, pAd->pHifCfg->PushMgmtIndex));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("=============MLME==================\n"));
                    for (i = 0;i < TX_RING_SIZE;i++)
                    {
                        pTxContext = (PTX_CONTEXT)&pAd->pHifCfg->MLMEContext[i];
                        //DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] IRPPending=%d:  InUse=%d: bWaitingBulkOut=%d: BulkOutSize=%05d: bFullForBulkOut=%d \n " , i, pTxContext->IRPPending, pTxContext->InUse, pTxContext->bWaitingBulkOut, pTxContext->BulkOutSize, pTxContext->bFullForBulkOut));
                    }
                }
                else if (pHardwareRegister->Data == 0x32)
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("==========================================\n"));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("pAd->Flags=0x%x, pAd->BulkFlags=0x%x\n", pAd->Flags, pAd->pHifCfg->BulkFlags));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, (">>BulkOutComplete=0x%x\n", pAd->pHifCfg->BulkOutComplete));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, (">>BulkOutCompleteCancel=0x%x, BulkOutCompleteOther=0x%x\n", pAd->pHifCfg->BulkOutCompleteCancel, pAd->pHifCfg->BulkOutCompleteOther));
                    DBGPRINT(RT_DEBUG_TRACE,(" --> CurWritePosition = %d.  NextBulkOutPosition = %d. \n", pAd->pHifCfg->TxContext[0].CurWritePosition, pAd->pHifCfg->TxContext[0].NextBulkOutPosition));
                    DBGPRINT(RT_DEBUG_TRACE,(" --> bCopySavePad = %d. IRPPending = %d  / %d. \n", pAd->pHifCfg->TxContext[0].bCopySavePad,pAd->pHifCfg->TxContext[0].IRPPending[0], pAd->pHifCfg->TxContext[0].IRPPending[1]));
                    
                }
                else if ((pHardwareRegister->Data&0xff) == 0x33)
                {
                    UCHAR       i;
                    PRX_CONTEXT pRxContext;
                    
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("==========================================\n"));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("NextRxBulkInIndex = 0x%x, NextRxBulkInReadIndex =0x%x, pAd->pHifCfg->NextRxBulkInPosition = 0x%x. \n", pAd->pHifCfg->NextRxBulkInIndex,  pAd->pHifCfg->NextRxBulkInReadIndex, pAd->pHifCfg->NextRxBulkInPosition));
                    DBGPRINT_RAW(RT_DEBUG_ERROR, (">>BulkInReq=0x%x, BulkInComplete=0x%x, BulkInCompleteFail=%x\n", pAd->pHifCfg->BulkInReq, pAd->pHifCfg->BulkInComplete, pAd->pHifCfg->BulkInCompleteFail));
                    for (i = 0;i < RX_RING_SIZE;i++)
                    {
                        pRxContext= (PRX_CONTEXT)&pAd->pHifCfg->RxContext[i];
                        DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] IRPPending=%d:  InUse=%d: Readable=%d: \n "
                            , i, pRxContext->IRPPending, pRxContext->InUse, pRxContext->Readable));
                    }
                    
                }
                else if ((pHardwareRegister->Data&0xff) == 0x34)
                {
                    ULONG       value;
                    
                    value = (SLEEPCID<<16)+(OWNERMCU<<24)+ (0x40<<8)+1;
                    RTUSBWriteMACRegister(pAd, 0x7010, value);
                    RTUSBWriteMACRegister(pAd, 0x404, 0x30);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Sleep Mailbox testvalue %x\n", value));
                    
                }
                else if ((pHardwareRegister->Data&0xff) == 0x35)
                {
                    ULONG       value;
                    
                    value = (WAKECID<<16)+(OWNERMCU<<24);
                    RTUSBWriteMACRegister(pAd, 0x7010, value);
                    RTUSBWriteMACRegister(pAd, 0x404, 0x31);
            
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Wakeup Mailbox testvalue%x\n", value));
                    
                }
                else if ((pHardwareRegister->Data&0xff) == 0x36)
                {
                    ULONG       value, ledcs;
                    
                    ledcs = (pHardwareRegister->Data&0xffff00)>>8;
                    value = (OWNERMCU<<24)+(LEDMODECID<<16)+ledcs;
                    RTUSBWriteMACRegister(pAd, 0x7010, value);
                    RTUSBWriteMACRegister(pAd, 0x404, 0x50);
            
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("LED Mailbox test value %x\n", value));
                    
                }
                else if ((pHardwareRegister->Data&0xff) == 0x37)
                {
                    ULONG       value, stren;
                    
                    stren = (pHardwareRegister->Data&0xffff00)>>8;
                    value = (OWNERMCU<<24)+(SIGSTRENCID<<16)+stren;
                    RTUSBWriteMACRegister(pAd, 0x7010, value);
                    RTUSBWriteMACRegister(pAd, 0x404, 0x51);
            
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("SignalStr Mailbox test value %x\n", value));
                    
                }
                else if ((pHardwareRegister->Data&0xff) == 0x38)
                {
                    ULONG   commandid, returncode;
                    
                    RTUSBReadMACRegister( pAd, 0x7014, &commandid);
                    RTUSBReadMACRegister( pAd, 0x701c, &returncode);
                    DBGPRINT(RT_DEBUG_ERROR, ("MailBoxTOExec read 0x7014 command id : 0x%x\n", commandid));
                    DBGPRINT(RT_DEBUG_ERROR, ("MailBoxTOExec read 0x701c code : 0x%x\n",returncode));
                    if (commandid != 0xffffffff)
                        RTUSBWriteMACRegister( pAd, 0x7014, 0xffffffff);
            
                    DBGPRINT_RAW(RT_DEBUG_ERROR, (" Clear Command\n"));
                    
                }
                else if ((pHardwareRegister->Data&0xff) == 0x39)
                {
                    ULONG       value;
                    ULONG       commandid, returncode;
                    value = (QUERYPOWERCID<<16)+(OWNERMCU<<24)+ (0x40<<8)+1;
                    {

                        RTUSBReadMACRegister( pAd, 0x7014, &commandid);
                        RTUSBReadMACRegister( pAd, 0x701c, &returncode);
                        DBGPRINT(RT_DEBUG_ERROR, ("MailBoxTOExec read 0x7014 command id : 0x%x\n", commandid));
                        DBGPRINT(RT_DEBUG_ERROR, ("MailBoxTOExec read 0x701c code : 0x%x\n",returncode));
                    }
                    RTUSBWriteMACRegister( pAd, 0x7014, 0xffffffff);
                    RTUSBWriteMACRegister(pAd, 0x7010, value);
                    RTUSBWriteMACRegister(pAd, 0x404, 0x32);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("QUERY POWER Mailbox testvalue %x\n", value));
                    /*RTUSBReadMACRegister( pAd, 0x7014, &commandid);
                    RTUSBReadMACRegister( pAd, 0x701c, &returncode);
                    DBGPRINT(RT_DEBUG_ERROR, ("MailBoxTOExec read 0x7014 command id : 0x%x\n", commandid));
                    DBGPRINT(RT_DEBUG_ERROR, ("MailBoxTOExec read 0x701c code : 0x%x\n",returncode));
                    RTUSBWriteMACRegister( pAd, 0x7014, 0xffffffff);
                    Status = N6UsbIoVendorRequest(
                                                pAd,
                                                (USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK),
                                                DEVICE_VENDOR_REQUEST_IN,
                                                0x1,
                                                0x12,
                                                0,
                                                &returncode,
                                                4);*/
                }
                else if ((pHardwareRegister->Data&0xff) == 0x40)
                {
                    ULONG       TbttNumToNextWakeUp;
                    AUTO_WAKEUP_STRUC   WakeupCfg;
                    ULONG       value;
                    TbttNumToNextWakeUp = (pHardwareRegister->Data&0xff00) >> 8;

                    value = (SLEEPCID<<16)+(OWNERMCU<<24)+ (0x40<<8)+1;
                    RTUSBWriteMACRegister(pAd, 0x7010, value);
                    RTUSBWriteMACRegister(pAd, 0x404, 0x30);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Sleep Mailbox testvalue %x\n", value));
                    if (TbttNumToNextWakeUp > 0)
                    {
                        WakeupCfg.field.EnableAutoWakeup = 1;
                        WakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp-1;
                        // lead time should set to minimum HW wakeuptime. use 7 first. The correct time needs measurement.
                        WakeupCfg.field.AutoLeadTime = 0xa;
                        RTUSBWriteMACRegister(pAd, AUTO_WAKEUP_CFG, WakeupCfg.word);
                    }
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("MlmeSet TbttNumToNextWakeUp=%d. Data = %x\n", TbttNumToNextWakeUp, pHardwareRegister->Data));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x41)
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("pPort->MacTab.Content[1].TxPhyCfg = %x. \n", READ_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->TxPhyCfg)));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x44)
                {
                    XmitSendPsPollFrame(pAd, pPort);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Send a Ps-Poll frame. \n"));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x45)
                {
                    XmitSendMlmeCmdPkt(pAd, TXPKT_NULL_FRAME);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Send a NULL frame. \n"));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x48)
                {
                    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
                    N6USBBulkReceive(pAd);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("N6USBBulkReceive \n"));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x4c)
                {
                    DBGPRINT(RT_DEBUG_ERROR, (" MacTab.Content[1].word = 0x%x \n",READ_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->TxPhyCfg)));
                    DBGPRINT(RT_DEBUG_ERROR, (" MacTab.Content[0].word = 0x%x \n",READ_PHY_CFG_DOUBLE_WORD(pAd, &pMbcastMacTabEntry->TxPhyCfg)));
                    DBGPRINT(RT_DEBUG_ERROR, (" mlme.word = 0x%x \n",READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg)));
                }
                else if(((pHardwareRegister->Data&0xff) >= 0x60)  &&  ((pHardwareRegister->Data&0xff) <= 0x6e))
                {
                    //-------- (0x60~0x6f) Reserved by P2P Plugfest 2010 ---------
                    P2PSetHwReg(pAd, pHardwareRegister);
                }
                else if ((pHardwareRegister->Data&0xff) == 0x6f)
                {
                    P2PDebugUseCmd(pAd, pHardwareRegister);
                }
                else if ((pHardwareRegister->Data&0xf000) != 0)
                {
                    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                    if(pBssidMacTabEntry == NULL)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                        return NDIS_STATUS_INVALID_DATA;
                    } 
                    
                    {
                        UCHAR   MCS;
                        
                        MCS = (UCHAR)(pHardwareRegister->Data)&0xf;

                        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg, MCS);
                        WRITE_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg, MCS);

                        if (((UCHAR)(pHardwareRegister->Data) & 0xff) == MCS_AUTO)
                        {
                            PRTMP_TX_RATE_SWITCH    pCurrTxRate;

                            pPort->CommonCfg.TxRateIndex = RateSwitchTable[1];
                            pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &RateSwitchTable[(pPort->CommonCfg.TxRateIndex+1)*SIZE_OF_RATE_TABLE_ENTRY];
                            
                            WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg, pCurrTxRate->CurrMCS);
                            
                            WRITE_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg, pCurrTxRate->CurrMCS);
                                
                            pPort->CommonCfg.bAutoTxRateSwitch = TRUE;
                        }
                        else
                        {
                            // bit10 for BW control
                            if ((pHardwareRegister->Data & 0x0400) == 0x400)
                            {
                                UCHAR   BBPValue;
                                ULONG   MACValue;
                                
                                WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, BW_40);
                                    
                                if (ADHOC_ON(pPort) && (pPort->Channel <3))
                                {
                                    RTUSBReadMACRegister(pAd, TX_BAND_CFG, &MACValue);
                                    MACValue &= 0xfe;
                                    MACValue |= 0x1;
                                    RTUSBWriteMACRegister(pAd, TX_BAND_CFG, MACValue);
                                    RTUSBReadBBPRegister(pAd, BBP_R4, &BBPValue);
                                    BBPValue&= (~0x18);
                                    BBPValue|= (0x10);
                                    RTUSBWriteBBPRegister(pAd, BBP_R4, BBPValue);
                                    RTUSBReadBBPRegister(pAd, BBP_R3, &BBPValue);
                                    BBPValue&= (~0x20);
                                    RTUSBWriteBBPRegister(pAd, BBP_R3, BBPValue);
                                    pPort->BBPCurrentBW = BW_40;
                                    pPort->CentralChannel = pPort->Channel +2;
                                    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                                }
                                if (ADHOC_ON(pPort) && (pPort->Channel >=3))
                                {
                                    RTUSBReadMACRegister(pAd, TX_BAND_CFG, &MACValue);
                                    MACValue &= 0xfe;
                                    RTUSBWriteMACRegister(pAd, TX_BAND_CFG, MACValue);
                                    RTUSBReadBBPRegister(pAd, BBP_R4, &BBPValue);
                                    BBPValue&= (~0x18);
                                    BBPValue|= (0x10);
                                    RTUSBWriteBBPRegister(pAd, BBP_R4, BBPValue);
                                    RTUSBReadBBPRegister(pAd, BBP_R3, &BBPValue);
                                    BBPValue&= (~0x20);
                                    RTUSBWriteBBPRegister(pAd, BBP_R3, BBPValue);
                                    pPort->BBPCurrentBW = BW_40;
                                    pPort->CentralChannel = pPort->Channel -2;
                                    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                                }
                            }
                            else if ((pHardwareRegister->Data & 0xF800) == 0xF800)
                            {
                                UCHAR CurrMCS = 0, NextMCS = 0;
                                DBGPRINT(RT_DEBUG_TRACE, (" %s:  Data %d   \n", __FUNCTION__, pHardwareRegister->Data));
                                CurrMCS = (UCHAR)((pHardwareRegister->Data & 0xFF000000) >> 24);
                                NextMCS = (UCHAR)((pHardwareRegister->Data & 0x00FF0000) >> 16);
                                AsicCfgPAPEByStreams(pAd, CurrMCS, NextMCS);
                            }
                            else
                            {
                                UCHAR   BBPValue;
                                if (ADHOC_ON(pPort))
                                {
                                    RTUSBReadBBPRegister(pAd, BBP_R4, &BBPValue);
                                    BBPValue &= (~0x08);
                                    RTUSBWriteBBPRegister(pAd, BBP_R4, BBPValue);
                                }
                                pPort->BBPCurrentBW = BW_20;
                                WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, BW_20);
                            }
                            // bit8, 9 for MODE control
                            WRITE_PHY_CFG_MODE(pAd,&pPort->CommonCfg.TxPhyCfg, (UCHAR)((pHardwareRegister->Data & 0x0300) >> 8));
                        
                            // bit7 for STBC control
                            if ((pHardwareRegister->Data & 0x0080) == 0x0080)
                            {
                                WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg, STBC_USE);
                            }
                            else
                            {
                                WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg, STBC_NONE);
                            }

                            // bit6 for ShortGI control
                            if ((pHardwareRegister->Data & 0x0040) == 0x0040)
                            {
                                WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_400);
                            }
                            else
                            {
                                WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_800);
                            }

                            // bit0~5 for MCS control
                            WRITE_PHY_CFG_STBC(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg));
                            WRITE_PHY_CFG_SHORT_GI(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg));
                            WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg));
                            WRITE_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg));

                            pPort->CommonCfg.bAutoTxRateSwitch = FALSE;

                            DBGPRINT(RT_DEBUG_TRACE, ("DRS: STBC=%d, ShortGI=%d, MODE=%d, BW=%d \n",
                                                READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg),
                                                READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg),
                                                READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg),
                                                READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg)));
                                                
                        }
                    }
                    DBGPRINT(RT_DEBUG_ERROR, (" HTPhyMode.MODE = 0x%d \n",READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg)));
                    DBGPRINT(RT_DEBUG_ERROR, (" HTPhyMode.MCS = 0x%d \n",READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg)));
                    DBGPRINT(RT_DEBUG_ERROR, (" HTPhyMode.BW = 0x%d \n",READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg)));
                    DBGPRINT(RT_DEBUG_ERROR, (" HTPhyMode.ShortGI = 0x%d \n",READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg)));
                    DBGPRINT(RT_DEBUG_ERROR, (" BSSID_WCID.MpduDensity = 0x%d \n",pBssidMacTabEntry->MpduDensity));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x99)
                {

                    MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("USB_RESET_BULK_IN\n"));
                    //
                    // From DDK,
                    // All transfers must be aborted or cancelled before attempting to reset the pipe.
                    //  
                    while ((pAd->pHifCfg->PendingRx > 0) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) 
                    {
                        if (pAd->pHifCfg->PendingRx > 0)
                        {
                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("BulkIn IRP Pending, pAd->pHifCfg->PendingRx=%d!!!\n", pAd->pHifCfg->PendingRx));
                            ntStatus = N6UsbIoVendorRequest(pAd,
                                                0,
                                                DEVICE_VENDOR_REQUEST_OUT,
                                                0x0C,
                                                0x0,
                                                0x0,
                                                NULL,
                                                0);
                            if (!NT_SUCCESS(ntStatus))
                            {
                                DBGPRINT_RAW(RT_DEBUG_TRACE, ("Stop RX failed, ntStatus=0x%08x\n", ntStatus));
                                break;
                            }
                        }
                    }

                    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                    {
                        UCHAR   i = 0;
                        //To distinguish it's really pipe error or fake error which might because temporary miss.
                        do
                        {
                            ntStatus = RTUSB_ResetPipe(pAd, 0);
                            i++;
                        }
                        while ((i < 1) && (NT_SUCCESS(ntStatus) != TRUE));
                        
                        if (NT_SUCCESS(ntStatus) == TRUE)
                        {
                            UCHAR   index;
                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk In Reset Successed\n"));

                            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
                            if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
                                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
                            {                       
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("NumOfBulkInIRP=%d\n",pAd->pNicCfg->NumOfBulkInIRP));
                                for (index = 0; index < pAd->pNicCfg->NumOfBulkInIRP; index ++)
                                {
                                    N6USBBulkReceive(pAd);
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("N6USBBulkReceive\n"));
                                }
                            }
                        }
                        else
                        {
                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk In Reset Failed!Status=0x%x\n", ntStatus));
                            ntStatus = RTUSB_GetPortStatus(pAd/*, &portStatus*/);
                            if (!NT_SUCCESS(ntStatus))
                                {
                                //if (!(portStatus & USBD_PORT_CONNECTED))
                                //{
                                    MT_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Device has been removed\n"));        
                                //}
                            }
                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("\n"));
                        }
                    }
                }
                else if ((pHardwareRegister->Data&0xff) == 0x97)
                {
                    MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("USB_RESET_BULK_OUT\n"));
                    //
                    // From DDK,
                    // All transfers must be aborted or cancelled before attempting to reset the pipe.
                    //                          
                    Ndis6CommonRejectPendingPackets(pAd); //reject all NDIS packets waiting in TX queue
                    N6UsbXmitStop(pAd);
                    N6UsbRecvStop(pAd);
                    
                    N6UsbRecvStart( pAd);
                    N6UsbXmitStart( pAd);
                    N6USBCleanUpDataBulkOutQueue(pAd);

                    usbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
                    do
                    {
                        UCHAR   Index;
                        

                        for (Index=0; Index<pAd->pHifCfg->BulkOutPipeNum; Index++)
                        {
                            if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                            {
                                ntStatus = RTUSB_ResetPipe(pAd, Index);
                                if (NT_SUCCESS(ntStatus) == TRUE)
                                {
                                    DBGPRINT(RT_DEBUG_TRACE, ("Bulk Out Reset on Pipe[=%d] Successed\n", Index));
                                }
                                else
                                {
                                    DBGPRINT(RT_DEBUG_TRACE, ("Bulk Out Reset on Pipe[=%d] Failed, Status=0x%x\n", Index, ntStatus));
                                }
                            }
                        }
                            
                        if (NT_SUCCESS(ntStatus) == TRUE)
                        {
                            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
                            NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
                        }
                    } while (FALSE);
                }           
            }
            // 0x2bf4: test H2M_MAILBOX. byte3: Host command, byte2: token, byte1-0: arguments
            else if (pHardwareRegister->Offset == (HW_DEBUG_SETTING_BASE + 4))
            {
                if ((pHardwareRegister->Data&0xff) == 0x31)
                {
                    pAd->Mlme.bEnableAutoAntennaCheck = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("bEnableAutoAntennaCheck = TRUE \n"));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x32)
                {
                    pAd->Mlme.bEnableAutoAntennaCheck = FALSE;
                    DBGPRINT(RT_DEBUG_TRACE, ("bEnableAutoAntennaCheck = FALSE \n"));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x33)
                {
                    pAd->HwCfg.BbpTuning.bEnable = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("pAd->HwCfg.BbpTuning.bEnable = TRUE \n"));
                }
                else if ((pHardwareRegister->Data&0xff) == 0x34)
                {
                    pAd->HwCfg.BbpTuning.bEnable = FALSE;
                    DBGPRINT(RT_DEBUG_TRACE, ("pAd->HwCfg.BbpTuning.bEnable = FALSE \n"));
                }
            }
            // 0x2bf8: test ACK policy and QOS format in ADHOC mode
            else if (pHardwareRegister->Offset == (HW_DEBUG_SETTING_BASE + 8))
            {
                PUCHAR pAckStr[4] = {"NORMAL", "NO-ACK", "NO-EXPLICIT-ACK", "BLOCK-ACK"};
                EDCA_PARM DefaultEdcaParm;

                // byte0 b1-0 means ACK POLICY - 0: normal ACK, 1: no ACK, 2:no explicit ACK, 3:BA
                pPort->CommonCfg.AckPolicy[0] = ((UCHAR)pHardwareRegister->Data & 0x02) << 5;
                pPort->CommonCfg.AckPolicy[1] = ((UCHAR)pHardwareRegister->Data & 0x02) << 5;
                pPort->CommonCfg.AckPolicy[2] = ((UCHAR)pHardwareRegister->Data & 0x02) << 5;
                pPort->CommonCfg.AckPolicy[3] = ((UCHAR)pHardwareRegister->Data & 0x02) << 5;
                DBGPRINT(RT_DEBUG_EMU, ("ACK policy = %s\n", pAckStr[(UCHAR)pHardwareRegister->Data & 0x02]));

                // any non-ZERO value in byte1 turn on EDCA & QOS format
                if (pHardwareRegister->Data & 0x0000ff00) 
                {
                    PlatformZeroMemory(&DefaultEdcaParm, sizeof(EDCA_PARM));
                    DefaultEdcaParm.bValid = TRUE;
                    DefaultEdcaParm.Aifsn[0] = 3;
                    DefaultEdcaParm.Aifsn[1] = 7;
                    DefaultEdcaParm.Aifsn[2] = 2;
                    DefaultEdcaParm.Aifsn[3] = 2;

                    DefaultEdcaParm.Cwmin[0] = 4;
                    DefaultEdcaParm.Cwmin[1] = 4;
                    DefaultEdcaParm.Cwmin[2] = 3;
                    DefaultEdcaParm.Cwmin[3] = 2;

                    DefaultEdcaParm.Cwmax[0] = 10;
                    DefaultEdcaParm.Cwmax[1] = 10;
                    DefaultEdcaParm.Cwmax[2] = 4;
                    DefaultEdcaParm.Cwmax[3] = 3;

                    DefaultEdcaParm.Txop[0]  = 0;
                    DefaultEdcaParm.Txop[0]  = (USHORT)(pHardwareRegister->Data&0xff);
                    DefaultEdcaParm.Txop[1]  = 0;
                    DefaultEdcaParm.Txop[2]  = AC2_DEF_TXOP;
                    DefaultEdcaParm.Txop[3]  = AC3_DEF_TXOP;
                    MtAsicSetEdcaParm(pAd, pPort, &DefaultEdcaParm, TRUE);
                }
                else
                    MtAsicSetEdcaParm(pAd, pPort, NULL, TRUE);
            }
            // 0x2bfc: turn ON/OFF TX aggregation
            else if (pHardwareRegister->Offset == (HW_DEBUG_SETTING_BASE + 12))
            {
                if (pHardwareRegister->Data == 0xff000000)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Force to scan...\n"));
                    MlmeAutoScan(pAd);
                }

                //
                // 0x77fc [0x11~0x40] reserve range for 2009 TGn Plugfest
                //          
                if (pHardwareRegister->Data)
                    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
                else
                    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
                DBGPRINT(RT_DEBUG_TRACE, ("AGGREGATION = %d\n", OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED)));
            }
#ifdef MULTI_CHANNEL_SUPPORT
            // multi-channel debug
            // 7801(hex) = 77f4(hex) + 13(dec)
            else if (pHardwareRegister->Offset == (ULONG)(HW_DEBUG_SETTING_BASE + 13))
            {
                pPort->P2PCfg.SwitchChTime = pHardwareRegister->Data;
                DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.SwitchChTime(%d)\n", pHardwareRegister->Data));
            }
            // 7802(hex) = 77f4(hex) + 14(dec)
            else if (pHardwareRegister->Offset == (ULONG)(HW_DEBUG_SETTING_BASE + 14))
            {
                pPort->P2PCfg.PwrSaveDelayTime = pHardwareRegister->Data;
                DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.PwrSaveDelayTime(%d)\n", pHardwareRegister->Data));              
            }
#endif /*MULTI_CHANNEL_SUPPORT*/            
            else
                Status = NDIS_STATUS_NOT_SUPPORTED;
        }
    }
    else if (pHardwareRegister->HardwareType == HARDWARE_BBP)
    {
        bbpID = (UCHAR) pHardwareRegister->Offset;
        bbpValue = (UCHAR) pHardwareRegister->Data;
        RTUSBWriteBBPRegister(pAd, bbpID, bbpValue);
        DBGPRINT(RT_DEBUG_TRACE, ("BBP:ID[0x%02x]=[0x%02x]\n", bbpID, bbpValue));
    }
    else if (pHardwareRegister->HardwareType == HARDWARE_RF)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("RF:ID[0x%02x]=[0x%02x]\n", rfID, rfValue));
    }   
    else if (pHardwareRegister->HardwareType == HARDWARE_EEP)
    {
        eepID = (USHORT) pHardwareRegister->Offset;
        eepValue= (USHORT) pHardwareRegister->Data;

        if ((pAd->HwCfg.buseEfuse) || (FlashInstallationWith8KBFirmware(pAd) == TRUE))
        {
#if DBG
            if ((pAd->TrackInfo.bExternalEEPROMImage == TRUE) && (pAd->HwCfg.buseEfuse == TRUE))
            {
                // Skip the EEPROM write operation
                DBGPRINT(RT_DEBUG_TRACE, ("%s: Skip the EEPROM write operation\n", __FUNCTION__));
            }
            else
#endif
            {
                eFuseWrite(pAd, eepID, (PUCHAR)&eepValue, 2);
            }
        }
        else
        {
            RTUSBWriteEEPROM(pAd, eepID, (PUCHAR)&eepValue, 2);
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("EEP:ID[0x%04x]=[0x%04x]\n", eepID, eepValue));
    }
    Status = NDIS_STATUS_SUCCESS;
    return Status;
}
#endif

