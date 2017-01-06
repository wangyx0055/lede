#include "MtConfig.h"

NTSTATUS
N6UsbCreateInitializeUsbDevice(
    IN  PMP_ADAPTER   pAd
    )
/*++

Routine Description:

    This helper routine selects the configuration, interface and
    creates a context for every pipe (end point) in that interface.

Arguments:

    Nic - Pointer to the HW NIC structure. 

Return Value:

    NT status value

--*/
{
    WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
    NTSTATUS                            ntStatus;
    WDFUSBPIPE                          pipeHandle;
    WDF_USB_PIPE_INFORMATION            pipeInfo;
    UCHAR                               pipeIndex;
    UCHAR                               numberConfiguredPipes;
    PUSB_DEVICE_CONTEXT                 usbDeviceContext;
    WDF_OBJECT_ATTRIBUTES               attributes;
    WDFDEVICE                           WdfDevice;
    UCHAR                               BulkOutIdx = 0;
    UCHAR                               BulkInIdx = 0;  

    // temp for emulation
    //BOOLEAN Skip2ndBulkinPipe = FALSE;
    //
    // Create a USB device handle so that we can communicate with the
    // underlying USB stack. The WDFUSBDEVICE handle is used to query,
    // configure, and manage all aspects of the USB device.
    // These aspects include device properties, bus properties,
    // and I/O creation and synchronization.
    //
    //
    // 1. create USB device
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, USB_DEVICE_CONTEXT);
    WdfDevice = pAd->pHifCfg->WdfDevice;
    ntStatus = WdfUsbTargetDeviceCreate(WdfDevice,
                                &attributes,
                                &pAd->pHifCfg->UsbDevice);
    
    if (!NT_SUCCESS(ntStatus)) {
        DBGPRINT(RT_DEBUG_TRACE ,("WdfUsbTargetDeviceCreate failed with Status code %x!STATUS!\n", ntStatus));
        return ntStatus;
    }

    if(pAd->pHifCfg->UsbDevice == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE ,("WdfUsbTargetDeviceCreate failed with usbdevice null\n"));
        return ntStatus;
    }

    //
    // 2. Select USB configuration, with only one interface
    //
    WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_SINGLE_INTERFACE( &configParams);
    ntStatus = WdfUsbTargetDeviceSelectConfig(pAd->pHifCfg->UsbDevice,
                                        WDF_NO_OBJECT_ATTRIBUTES,
                                        &configParams);
    
    if(!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("WdfUsbTargetDeviceSelectConfig failed %x!STATUS!\n",
        ntStatus));
        return ntStatus;
    }

    usbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
    usbDeviceContext->UsbInterface = configParams.Types.SingleInterface.ConfiguredUsbInterface;
    numberConfiguredPipes = configParams.Types.SingleInterface.NumberConfiguredPipes;

    if (numberConfiguredPipes > MAX_PIPES ) 
    {
        numberConfiguredPipes = MAX_PIPES;
    }

    usbDeviceContext->NumberOfPipes = numberConfiguredPipes;
    DBGPRINT(RT_DEBUG_TRACE, ("N6UsbCreateInitializeUsbDevice:NumberOfPipes(IN+OUT)=%d\n", numberConfiguredPipes));


    //
    // 3. Enumerate pipes
    //
    for(pipeIndex=0; pipeIndex < numberConfiguredPipes; pipeIndex++) 
    {
        WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);

        pipeHandle  = WdfUsbInterfaceGetConfiguredPipe(usbDeviceContext->UsbInterface,
                                                pipeIndex, //PipeIndex,
                                                &pipeInfo
                                                );
        
        if (pipeHandle == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("IN %s Config Pipe Fail Pipe index = %D\n", __FUNCTION__, pipeIndex));
            ntStatus = STATUS_INVALID_DEVICE_STATE;
            return ntStatus;
        }

        //
        // Tell the framework that it's okay to read less than
        // MaximumPacketSize
        //
        WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(pipeHandle);

        if(WdfUsbPipeTypeBulk == pipeInfo.PipeType ) 
        {
            DBGPRINT(RT_DEBUG_TRACE ,("BulkInput Pipe is 0x%p direction : %s\n", 
                                        pipeHandle,
                                        WdfUsbTargetPipeIsInEndpoint(pipeHandle) ? "IN" : "OUT"));

            if (WdfUsbTargetPipeIsInEndpoint(pipeHandle)) 
            {
                usbDeviceContext->BulkInPipeHandle[BulkInIdx] = pipeHandle;
                DBGPRINT(RT_DEBUG_TRACE,("creating BulkInIdx = %d, 0x%x\n",BulkInIdx,pipeHandle));
                usbDeviceContext->BulkInMaxPacketSize = pipeInfo.MaximumPacketSize;
                pAd->pHifCfg->BulkInMaxPacketSize = (USHORT)pipeInfo.MaximumPacketSize;
                BulkInIdx++;
            } 
            else 
            {
                usbDeviceContext->BulkOutPipeHandle[BulkOutIdx] = pipeHandle;
                DBGPRINT(RT_DEBUG_TRACE,("creating BulkOutIdx = %d, 0x%x\n",BulkOutIdx,pipeHandle));
                DBGPRINT(RT_DEBUG_TRACE,("Endpoint Address = %x\n",pipeInfo.EndpointAddress));
                usbDeviceContext->BulkOutMaxPacketSize = pipeInfo.MaximumPacketSize;
                pAd->pHifCfg->BulkOutMaxPacketSize = (USHORT)pipeInfo.MaximumPacketSize;
                BulkOutIdx++;
            }
            DBGPRINT(RT_DEBUG_TRACE,("%s: Size = 0x%x", __FUNCTION__, pipeInfo.Size));
            DBGPRINT(RT_DEBUG_TRACE,("%s: MaximumPacketSize = 0x%x", __FUNCTION__, pipeInfo.MaximumPacketSize));
            DBGPRINT(RT_DEBUG_TRACE,("%s: MaximumTransferSize = 0x%x", __FUNCTION__, pipeInfo.MaximumTransferSize));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE,(" Pipe is 0x%p type %x\n", pipeHandle, pipeInfo.PipeType));
        }
    }
    pAd->pHifCfg->BulkInPipeNum = BulkInIdx;
    pAd->pHifCfg->BulkOutPipeNum = BulkOutIdx;

    DBGPRINT(RT_DEBUG_TRACE,("Total %d IN-pipes, %d OUT-pipes\n",pAd->pHifCfg->BulkInPipeNum,pAd->pHifCfg->BulkOutPipeNum));

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = pAd->pHifCfg->WdfDevice;

    ntStatus = WdfWaitLockCreate(&attributes, &usbDeviceContext->PipeStateLock);
    if (!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT(RT_DEBUG_TRACE ,("Couldn't create PipeStateLock %x!STATUS!\n",
        ntStatus));
        return ntStatus;
    }   

    return ntStatus;
}

/*
    ========================================================================
    
    Routine Description:
        Read SafelyRemove Falg 

    Arguments:
        pAd                            Pointer to our adapter
        WrapperConfigurationContext    For use by NdisOpenConfiguration

    Return Value:
        TRUE     Disable SafelyRemove Icon on systemtray
        FALSE    Enable SafelyRemove Icon on systemtray
        
    IRQL <= DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
BOOLEAN 
N6UsbReadSafelyRemoveFlag(
    IN PMP_ADAPTER    pAd
    )
{
    NDIS_CONFIGURATION_OBJECT       ConfigObject;
    NDIS_HANDLE                     RegistryConfigurationHandle = NULL;
    NDIS_CONFIGURATION_PARAMETER    Parameter;
    NDIS_STRING                     RegName = NDIS_STRING_CONST("SafelyRemoveOff");
    NDIS_STATUS                     ndisStatus;
    PNDIS_CONFIGURATION_PARAMETER   pParameter = NULL;  
    BOOLEAN                         bSafelyRemove = FALSE;

    ConfigObject.Header.Type = NDIS_OBJECT_TYPE_CONFIGURATION_OBJECT;
    ConfigObject.Header.Revision = NDIS_CONFIGURATION_OBJECT_REVISION_1;
    ConfigObject.Header.Size = sizeof( NDIS_CONFIGURATION_OBJECT );
    ConfigObject.NdisHandle = pAd->AdapterHandle;
    ConfigObject.Flags = 0;
#pragma prefast(suppress: __WARNING_MEMORY_LEAK, " should not have memmory leak")   
    ndisStatus = NdisOpenConfigurationEx(
                        &ConfigObject,
                        &RegistryConfigurationHandle
                        );

    if ((ndisStatus == NDIS_STATUS_SUCCESS) && (RegistryConfigurationHandle != NULL))
    {
        pParameter = &Parameter;

        NdisReadConfiguration(
                    &ndisStatus,
                    &pParameter,
                    RegistryConfigurationHandle,
                    &RegName,
                    NdisParameterInteger
                    );
        if (ndisStatus == NDIS_STATUS_SUCCESS)
        {
            bSafelyRemove = (pParameter->ParameterData.IntegerData ? TRUE : FALSE);
        }

        //
        // Close the handle to the registry
        //
        NdisCloseConfiguration(RegistryConfigurationHandle);
    }
    else
    {
        DBGPRINT_ERR(("NdisOpenConfigurationEx failed\n"));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("N6UsbReadSafelyRemoveFlag, SafelyRemoveOff=%d\n", bSafelyRemove));

    return bSafelyRemove;
}

NDIS_STATUS 
N6USBInterfaceStart(
    IN  PMP_ADAPTER   pAd
    )
{
    NTSTATUS ntStatus;

    ULONG                   nameLength;
    WDF_OBJECT_ATTRIBUTES   attributes;

    // Get the pointer to Next Device Object
    NdisMGetDeviceProperty(
            pAd->AdapterHandle,
            &pAd->pHifCfg->Pdo,
            &pAd->pHifCfg->Fdo,
            &pAd->pHifCfg->pNextDeviceObject,
            NULL,
            NULL);

    if (pAd->pHifCfg->pNextDeviceObject == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NdisMGetDeviceProperty failed\n", __FUNCTION__));
        return NDIS_STATUS_DEVICE_FAILED;
    }       

    // Allocate AdapterDesc memory block
    if (pAd->pHifCfg->AdapterDesc != NULL)  // if from S3/S4
        PlatformFreeMemory(pAd->pHifCfg->AdapterDesc, pAd->pHifCfg->AdapterDescLen);

    PlatformAllocateMemory(pAd, &pAd->pHifCfg->AdapterDesc,NIC_RTMP_ADAPTER_NAME_SIZE );
    pAd->pHifCfg->AdapterDescLen = NIC_RTMP_ADAPTER_NAME_SIZE;
    if(pAd->pHifCfg->AdapterDesc == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory for AdapterDesc failed\n", __FUNCTION__));
        return NDIS_STATUS_RESOURCES;
    }
    
    ntStatus = IoGetDeviceProperty (pAd->pHifCfg->Pdo,
                                  DevicePropertyDeviceDescription,
                                  NIC_RTMP_ADAPTER_NAME_SIZE,
                                  pAd->pHifCfg->AdapterDesc,
                                  &nameLength);

    if((ntStatus == STATUS_BUFFER_TOO_SMALL) && (nameLength > 0))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("use nameLength = %d for  IoGetDeviceProperty failed (0x%x)\n",nameLength,ntStatus));
        PlatformFreeMemory(pAd->pHifCfg->AdapterDesc, pAd->pHifCfg->AdapterDescLen);
        // Allocate larger size buffer
       PlatformAllocateMemory(pAd, &pAd->pHifCfg->AdapterDesc,nameLength );
       pAd->pHifCfg->AdapterDescLen = nameLength;
        if(pAd->pHifCfg->AdapterDesc == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory for AdapterDesc failed\n", __FUNCTION__));
            return NDIS_STATUS_RESOURCES;
        }
    
        ntStatus = IoGetDeviceProperty (pAd->pHifCfg->Pdo,
                          DevicePropertyDeviceDescription,
                          nameLength,
                          pAd->pHifCfg->AdapterDesc,
                          &nameLength);
    }
    
    if (!NT_SUCCESS (ntStatus))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: IoGetDeviceProperty failed (0x%x)\n", __FUNCTION__,  ntStatus));
        return NDIS_STATUS_INVALID_DEVICE_REQUEST;
    }

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, WDF_DEVICE_INFO);

    ntStatus = WdfDeviceMiniportCreate(WdfGetDriver(),
                                     &attributes,
                                     pAd->pHifCfg->Fdo,
                                     pAd->pHifCfg->pNextDeviceObject,
                                     pAd->pHifCfg->Pdo,
                                     &pAd->pHifCfg->WdfDevice);
    
    if (!NT_SUCCESS (ntStatus))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: WdfDeviceMiniportCreate failed (0x%x)\n", __FUNCTION__, ntStatus));
        return NDIS_STATUS_RESOURCES;
    }

    //
    // Get WDF miniport device context.
    //
    GetWdfDeviceInfo(pAd->pHifCfg->WdfDevice)->Adapter = pAd;

    ntStatus = N6UsbCreateInitializeUsbDevice(pAd);
    if (!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: N6UsbCreateInitializeUsbDevice failed\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }
    
    return NDIS_STATUS_SUCCESS;
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    Note:
        
    ========================================================================
*/
NDIS_STATUS 
N6UsbInitTransmit(
    IN  PMP_ADAPTER   pAd
    )
{

    CCHAR               stackSize;
    UCHAR               i, acidx;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PTX_CONTEXT         pNullContext   = &(pAd->pHifCfg->NullContext);
    PTX_CONTEXT         pPsPollContext = &(pAd->pHifCfg->PsPollContext);
    PTX_CONTEXT         pRTSContext    = &(pAd->pHifCfg->RTSContext);
    PTX_CONTEXT         pPktCmdContext = &(pAd->pHifCfg->PktCmdContext);
    PTX_CONTEXT         pFwPktContext = &(pAd->pHifCfg->FwPktContext);

    PUSB_DEVICE_CONTEXT     pUsbDeviceContext;
    WDFUSBPIPE              usbPipe;
    BOOLEAN                 bfailAllocate= FALSE;
    BOOLEAN                 bAllocateStatus = FALSE;
    stackSize = pAd->pHifCfg->pNextDeviceObject->StackSize + 1;
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);

    //
    // set a timer for 5 seconds for each request. This includes both data and mgmt. frames
    //
    WDF_REQUEST_SEND_OPTIONS_INIT(
                              &G_RequestOptions,
                              0
                              );
    //
    // Timeout of less than 5 secs is less desirable.
    //
    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
                                     &G_RequestOptions,
                                     WDF_REL_TIMEOUT_IN_SEC(5)
                                     );
    DBGPRINT(RT_DEBUG_TRACE, ("--> N6UsbInitTransmit\n"));

    // Init 4 set of Tx parameters
    for (i = 0; i < 4; i++)
    {
        // Initialize all Transmit releated queues
        InitializeQueueHeader(&pAd->pTxCfg->SendTxWaitQueue[i]);
        
        pAd->pHifCfg->NextBulkOutIndex[i]    = 0;        // Next Local tx ring pointer waiting for buck out
        pAd->pHifCfg->BulkOutPending[i]      = FALSE;    // Buck Out control flag    
    }

    InitializeQueueHeader(&pAd->pP2pCtrll->TxSwNoAMgmtQueue);
#if UAPSD_AP_SUPPORT
    for (i=0; i<NUM_OF_UAPSD_CLI; i++)
    {
        InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[i][QID_AC_BE]);
        InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[i][QID_AC_BK]);
        InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[i][QID_AC_VI]);
        InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[i][QID_AC_VO]);
    }
#endif
    
    pAd->pHifCfg->NextMLMEIndex         = 0;
    pAd->pHifCfg->PushMgmtIndex         = 0;
    pAd->pHifCfg->PopMgmtIndex          = 0;
    pAd->pHifCfg->MgmtQueueSize         = 0;

    pAd->pHifCfg->PrioRingFirstIndex    = 0;
    pAd->pHifCfg->PrioRingTxCnt         = 0;

    do
    {
        // Allocate BulkOut temp buffer
        for (i = 0; i < 4; i++)
        {
            Status = PlatformAllocateMemory(pAd,&(pAd->pHifCfg->BulkOutTempBuf[i]), LOCAL_TXBUF_SIZE_4K);
            if ((Status != NDIS_STATUS_SUCCESS) || (NULL == pAd->pHifCfg->BulkOutTempBuf[i]))
            {
                Status = PlatformAllocateMemory(pAd,&(pAd->pHifCfg->BulkOutTempBuf[i]), LOCAL_TXBUF_SIZE_4K);
                if ((Status != NDIS_STATUS_SUCCESS) || (NULL == pAd->pHifCfg->BulkOutTempBuf[i]))
                {           
                    Status = NDIS_STATUS_RESOURCES;
                    DBGPRINT(RT_DEBUG_ERROR, ("ERROR pAd->pHifCfg->BulkOutTempBuf[%d]\n",i));
                    goto done;
                }
            }
        }

        // Swap Endpoint : change endpoint 8 to correct index       
        {
            WDFUSBPIPE tempPipe;
            DBGPRINT(RT_DEBUG_TRACE,("pipes before swapping:\n"));

            tempPipe = pUsbDeviceContext->BulkOutPipeHandle[0];
            for(i=0;i<pAd->pHifCfg->BulkOutPipeNum - 2;i++)
            {   
                pUsbDeviceContext->BulkOutPipeHandle[i] = pUsbDeviceContext->BulkOutPipeHandle[i+1];
            }
            pUsbDeviceContext->BulkOutPipeHandle[pAd->pHifCfg->BulkOutPipeNum-2] = tempPipe;

            DBGPRINT(RT_DEBUG_TRACE,("pipes after swapping,\n"));
            for(i=0;i<pAd->pHifCfg->BulkOutPipeNum;i++)
            {
                DBGPRINT(RT_DEBUG_TRACE,("usbDeviceContext->BulkOutPipeHandle[%d] = 0x%x\n",i,pUsbDeviceContext->BulkOutPipeHandle[i]));
            }
        }

        //
        // TX_RING_SIZE
        //
        for (acidx = 0; acidx < 4; acidx++)
        {
            PHT_TX_CONTEXT  pTxContext = &(pAd->pHifCfg->TxContext[acidx]);
             usbPipe = pUsbDeviceContext->BulkOutPipeHandle[acidx]; 
            PlatformZeroMemory(pTxContext, sizeof(HT_TX_CONTEXT));              
            PlatformAllocateMemory( pAd, &pTxContext->TransferBuffer, sizeof(HTTX_BUFFER));
            if (NULL == pTxContext->TransferBuffer)
            {
                bfailAllocate =TRUE;
                Status = NDIS_STATUS_RESOURCES;
                DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX TxContext[%d] TX_BUFFER !! \n", i));                   
                goto done;
            }

            PlatformZeroMemory(pTxContext->TransferBuffer->Aggregation, 4);
            pTxContext->pAd = pAd;      
            pTxContext->IRPPending[0] = FALSE;
            pTxContext->IRPPending[1] = FALSE;
            pTxContext->NextBulkOutPosition = 0;
            pTxContext->ENextBulkOutPosition = 0;
            pTxContext->CurWritePosition = 0;
            pTxContext->BulkOutSize = 0;
            pTxContext->BulkOutPipeId = acidx;      
            pTxContext->bCopySavePad = FALSE;

            //add check if this pipe exist
            if(usbPipe != NULL)
            {
                bAllocateStatus = AllocateWriteRequestsWorker(
                            pAd,
                            usbPipe,
                            pTxContext
                        );

                if(bAllocateStatus == FALSE)
                {
                    bfailAllocate = TRUE;
                    Status = NDIS_STATUS_RESOURCES;
                    goto done;
                }
            }
        }

#if 1 // MT7603 FPGA
        {
            int i=0;

            for (i = 0; i < 6; i ++)
            {
                PTX_CONTEXT         pEPNullContext   = &(pAd->pHifCfg->EPNullContext[i]);

                usbPipe = pUsbDeviceContext->BulkOutPipeHandle[i];  

                //
                // NullContext
                //
                PlatformZeroMemory(pEPNullContext, sizeof(TX_CONTEXT)); 
                PlatformAllocateMemory( pAd, &pEPNullContext->TransferBuffer, sizeof(TX_BUFFER));
                if (NULL == pEPNullContext->TransferBuffer)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX NullContext TX_BUFFER !! \n"));            
                    Status = NDIS_STATUS_RESOURCES;
                    goto done;
                }
                pEPNullContext->pAd = pAd;  
                pEPNullContext->InUse = FALSE;
                pEPNullContext->ReadyToBulkOut = FALSE;
                pEPNullContext->IRPPending = FALSE;
                bAllocateStatus = AllocateMLMEWriteRequestsWorker(
                                    pAd,
                                    usbPipe,
                                    pEPNullContext                          
                                );

                if(bAllocateStatus == FALSE)
                {
                    bfailAllocate = TRUE;
                    Status = NDIS_STATUS_RESOURCES;
                    goto done;
                }           
            }
        }
#endif

        //for Fw packet
        {
            usbPipe = pUsbDeviceContext->BulkOutPipeHandle[ENDPOINT_OUT_4];  
        }

        if (usbPipe == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("error, BulkOutPipeHandle[ENDPOINT_OUT_4]\n"));
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }

        //
        // FwPktContext
        //
        PlatformZeroMemory(pFwPktContext, sizeof(TX_CONTEXT)); 
        PlatformAllocateMemory( pAd, &pFwPktContext->TransferBuffer, sizeof(CMD_TX_BUFFER));
        if (NULL == pFwPktContext->TransferBuffer)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX pFwPktContext TX_BUFFER !! \n"));          
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        pFwPktContext->pAd = pAd;  
        pFwPktContext->InUse = FALSE;
        pFwPktContext->ReadyToBulkOut = FALSE;
        pFwPktContext->IRPPending = FALSE;
        bAllocateStatus = AllocateMLMEWriteRequestsWorker(
                            pAd,
                            usbPipe,
                            pFwPktContext                          
                        );

        //for mlme/control frame/CMD packet
        {
            usbPipe = pUsbDeviceContext->BulkOutPipeHandle[ENDPOINT_OUT_8];  
        }

        if (usbPipe == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("error, BulkOutPipeHandle[ENDPOINT_OUT_8]\n"));
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }

        //
        // PRIO_RING_SIZE
        //      
        for (i = 0; i < PRIO_RING_SIZE; i++)
        {
            PTX_CONTEXT pMLMEContext = &(pAd->pHifCfg->MLMEContext[i]);      

            PlatformZeroMemory(pMLMEContext, sizeof(TX_CONTEXT));
#if _WIN8_USB_SS_SUPPORTED
#if DBG
            pMLMEContext->Index = i;
#endif
#endif
             PlatformAllocateMemory( pAd, &pMLMEContext->TransferBuffer, sizeof(TX_BUFFER));
            if (NULL == pMLMEContext->TransferBuffer)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX MLMEContext[%d] TX_BUFFER !! \n", i));
                bfailAllocate = TRUE;
                Status = NDIS_STATUS_RESOURCES;
                goto done;
            }
            bAllocateStatus = AllocateMLMEWriteRequestsWorker(
                            pAd,
                            usbPipe,
                            pMLMEContext                            
                        );

            pMLMEContext->pAd = pAd;        
            pMLMEContext->InUse = FALSE;
            pMLMEContext->ReadyToBulkOut = FALSE;
            pMLMEContext->IRPPending = FALSE;
            if(bAllocateStatus == FALSE)
            {
                bfailAllocate = TRUE;
                Status = NDIS_STATUS_RESOURCES;
                goto done;
                    
            }
        }
        
        //
        // NullContext
        //
        PlatformZeroMemory(pNullContext, sizeof(TX_CONTEXT));   
        PlatformAllocateMemory( pAd, &pNullContext->TransferBuffer, sizeof(TX_BUFFER));
        if (NULL == pNullContext->TransferBuffer)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX NullContext TX_BUFFER !! \n"));            
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        pNullContext->pAd = pAd;    
        pNullContext->InUse = FALSE;
        pNullContext->ReadyToBulkOut = FALSE;
        pNullContext->IRPPending = FALSE;
        bAllocateStatus = AllocateMLMEWriteRequestsWorker(
                            pAd,
                            usbPipe,
                            pNullContext                            
                        );

        if(bAllocateStatus == FALSE)
        {
            bfailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        
        //
        // RTSContext
        //
        PlatformZeroMemory(pRTSContext, sizeof(TX_CONTEXT));        
         PlatformAllocateMemory( pAd, &pRTSContext->TransferBuffer, sizeof(TX_BUFFER));
        if (NULL == pRTSContext->TransferBuffer)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX RTSContext TX_BUFFER !! \n"));         
            bfailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        pRTSContext->pAd = pAd; 
        pRTSContext->InUse = FALSE;
        pRTSContext->ReadyToBulkOut = FALSE;
        pRTSContext->IRPPending = FALSE;
        bAllocateStatus = AllocateMLMEWriteRequestsWorker(
                            pAd,
                            usbPipe,
                            pRTSContext                         
                        );

        if(bAllocateStatus == FALSE)
        {
            bfailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        
        //
        // PsPollContext
        //
        PlatformZeroMemory(pPsPollContext, sizeof(TX_CONTEXT));     
        PlatformAllocateMemory( pAd, &pPsPollContext->TransferBuffer, sizeof(TX_BUFFER));
        if (NULL == pPsPollContext->TransferBuffer)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX PsPollContext TX_BUFFER !! \n"));          
            bfailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        pPsPollContext->pAd = pAd;      
        pPsPollContext->InUse = FALSE;
        pPsPollContext->ReadyToBulkOut = FALSE;
        pPsPollContext->IRPPending = FALSE;
        pPsPollContext->bAggregatible = FALSE;
        pPsPollContext->LastOne = TRUE;
        bAllocateStatus = AllocateMLMEWriteRequestsWorker(
                            pAd,
                            usbPipe,
                            pPsPollContext                          
                        );
        if(bAllocateStatus == FALSE)
        {
            bfailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        
        //
        // PktCmdContext
        //
        PlatformZeroMemory(pPktCmdContext, sizeof(TX_CONTEXT)); 
        PlatformAllocateMemory( pAd, &pPktCmdContext->TransferBuffer, sizeof(CMD_TX_BUFFER));
        if (NULL == pPktCmdContext->TransferBuffer)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX PktCmdContext TX_BUFFER !! \n"));          
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        pPktCmdContext->pAd = pAd;  
        NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
        pPktCmdContext->InUse = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
        pPktCmdContext->ReadyToBulkOut = FALSE;
        pPktCmdContext->IRPPending = FALSE;
        bAllocateStatus = AllocateMLMEWriteRequestsWorker(
                            pAd,
                            usbPipe,
                            pPktCmdContext                          
                        );

        if(bAllocateStatus == FALSE)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("PktCmdContext failed\n"));
            bfailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }

        //for added connection probility
         PlatformAllocateMemory( pAd, &pAd->pNicCfg->OutBufferForSendProbeReq, MAX_VIE_LEN);
        if (NULL == pAd->pNicCfg->OutBufferForSendProbeReq)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in AllocpAd->pNicCfg->OutBufferForSendProbeReq !! \n"));            
            Status = NDIS_STATUS_RESOURCES;
            goto done;
        }
        
    } while (FALSE);

done:   
    if(bfailAllocate)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("bfailAllocate = TRUE\n"));
        N6UsbFreeNICTransmit(pAd);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("<-- N6UsbInitTransmit\n"));
    return Status;
}

/*
    ========================================================================
    
    Routine Description:
        Initialize receive data structures

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_RESOURCES

    Note:
        Initialize all receive releated private buffer, include those define
        in MP_ADAPTER structure and all private data structures. The mahor
        work is to allocate buffer for each packet and chain buffer to 
        NDIS packet descriptor.
        
    ========================================================================
*/
NDIS_STATUS 
N6UsbInitRecv(
    IN  PMP_ADAPTER   pAd
    )
{
    UCHAR           i;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    NTSTATUS                    ntStatus;
    INT             index, num;
    PRTMP_REORDERBUF    pReorderBuf;
    CCHAR               stackSize;
    PUSB_DEVICE_CONTEXT     pUsbDeviceContext;
    WDFUSBPIPE              usbPipe, usbPipe2;
    CHAR                    dummyBuffer[1] = {0};  //Need a buffer to pass to UsbBuildInterruptOrBulkTransferRequest  
    BOOLEAN             bFailAllocate = FALSE;
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
    usbPipe = pUsbDeviceContext->BulkInPipeHandle[ENDPOINT_IN_84];
    WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(usbPipe);
    usbPipe2 = pUsbDeviceContext->BulkInPipeHandle[ENDPOINT_IN_85];
    WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(usbPipe2);

    stackSize = pAd->pHifCfg->pNextDeviceObject->StackSize + 1;
    pAd->pHifCfg->PendingRx = 0;
    pAd->pHifCfg->NextRxBulkInReadIndex = 0; // Next Rx Read indexyy
    pAd->pHifCfg->NextRxBulkInIndex      = RX_RING_SIZE_END_INDEX -2;    // Rx Bulk pointer
    pAd->pHifCfg->NextRxBulkInPosition = 0;
    for (i = 0; i < (RX_RING_SIZE); i++)
    {
        PRX_CONTEXT  pRxContext = &(pAd->pHifCfg->RxContext[i]);

        if (i == 31)
        {
            ntStatus = WdfRequestCreate(
                        WDF_NO_OBJECT_ATTRIBUTES,
                        WdfUsbTargetPipeGetIoTarget(usbPipe2),
                        &pRxContext->Request);
        }
        else
        {
            ntStatus = WdfRequestCreate(
                        WDF_NO_OBJECT_ATTRIBUTES,
                        WdfUsbTargetPipeGetIoTarget(usbPipe),
                        &pRxContext->Request);
        }
        
        if (!NT_SUCCESS(ntStatus))
        {       
            bFailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            DBGPRINT(RT_DEBUG_ERROR, ("N6UsbInitRecv, WdfRequestCreate failed\n"));
            break;
        }

        if (i == 31)
        {
            ntStatus = WdfMemoryCreatePreallocated(
                    WDF_NO_OBJECT_ATTRIBUTES,
                    dummyBuffer,
                    LOCAL_TXBUF_SIZE,
                    //sizeof(dummyBuffer),
                    &pRxContext->RxWdfMemory);
        }
        else
        {
            ntStatus = WdfMemoryCreatePreallocated(
                    WDF_NO_OBJECT_ATTRIBUTES,
                    dummyBuffer,
                    MAX_RXBULK_SIZE,
                    //sizeof(dummyBuffer),
                    &pRxContext->RxWdfMemory);
        }

        if (!NT_SUCCESS(ntStatus))
        {   
            bFailAllocate = TRUE;
            Status = NDIS_STATUS_RESOURCES;
            DBGPRINT(RT_DEBUG_ERROR, ("N6UsbInitRecv, WdfMemoryCreatePreallocated failed\n"));
            break;
        }
        if (i == 31)
        {
            // Allocate transfer buffer
            PlatformAllocateMemory( pAd, &pRxContext->TransferBuffer, LOCAL_TXBUF_SIZE);
            if (pRxContext->TransferBuffer == NULL)
            {
                bFailAllocate = TRUE;
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            PlatformZeroMemory(pRxContext->TransferBuffer, LOCAL_TXBUF_SIZE);
        pRxContext->TransferbufferLen = LOCAL_TXBUF_SIZE;
        }
        else
        {
            // Allocate transfer buffer
            PlatformAllocateMemory(pAd, &pRxContext->TransferBuffer, MAX_RXBULK_SIZE);
            if (pRxContext->TransferBuffer == NULL)
            {
                bFailAllocate = TRUE;
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            PlatformZeroMemory(pRxContext->TransferBuffer, MAX_RXBULK_SIZE);
         pRxContext->TransferbufferLen = MAX_RXBULK_SIZE;       
        }

        NdisAllocateSpinLock(&pRxContext->RxContextLock);
        pRxContext->pAd = pAd;
        pRxContext->InUse       = FALSE;
        pRxContext->IRPPending  = FALSE;
        pRxContext->Readable    = FALSE;        
        pRxContext->ReadPosOffset = 0;
        if (i == 31)
        {
            pRxContext->pipe = usbPipe2;
            pRxContext->IoTarget = WdfUsbTargetPipeGetIoTarget(usbPipe2);
        }
        else
        {
            pRxContext->pipe = usbPipe;
            pRxContext->IoTarget = WdfUsbTargetPipeGetIoTarget(usbPipe);
        }
        
        pRxContext->UsbDevice = pAd->pHifCfg->UsbDevice; 
        pRxContext->Index = i;
    }


    for (i = 0; i < RTMP_MAX_NUMBER_OF_PORT; i ++)
    {
        pAd->pRxCfg->pLastWaitToIndicateRxPkts[i] = NULL;
        pAd->pRxCfg->pWaitToIndicateRxPkts[i] = NULL;
        pAd->pRxCfg->nWaitToIndicateRxPktsNum[i] = 0;
    }
    
    // Allocate Rx Reordering buffer.  Usb 
    //
    // Initialize BA Rx reordering Ring and associated buffer memory
    //
    for (num = 0; num < Max_BARECI_SESSION; num++)
    {
        pAd->pRxCfg->LocalRxReorderBuf[num].InUse = FALSE;
        for (index = 0; index < Max_RX_REORDERBUF; index++)
        {
            pReorderBuf = &pAd->pRxCfg->LocalRxReorderBuf[num].MAP_RXBuf[index];
            pReorderBuf->AllocSize = RX_BUFFER_NORMSIZE;    //buffer for Per MPDU. SO it's 4096bytes. Not whole Aggregated BUlkIn.
            PlatformAllocateMemory(pAd, &pReorderBuf->pBuffer, RX_BUFFER_NORMSIZE);
            if (pReorderBuf->pBuffer == NULL)
            {
                bFailAllocate = TRUE;
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            PlatformZeroMemory(pReorderBuf->pBuffer, RX_BUFFER_NORMSIZE);
            pReorderBuf->IsFull = FALSE;

        }

    }

    if(bFailAllocate)
    {
        N6UsbFreeNICRecv(pAd);
    }
    return Status;
}

VOID    
N6UsbFreeNICRecv(
    IN  PMP_ADAPTER   pAd
    )
{
    UINT                i,num,index;
    PRX_CONTEXT         pRxContext = NULL;
    PRTMP_REORDERBUF        pReorderBuf;
    PMP_LOCAL_RX_PKTS pLocalNetBufferList;

    // Free all resources for the RECEIVE buffer queue.

    PUSB_DEVICE_CONTEXT pUsbDeviceContext;

    if (pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd is NULL\n", __FUNCTION__));  
        return;
    }

    if (pAd->pHifCfg != NULL)
    {
        pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
        WdfUsbTargetPipeResetSynchronously(
        pUsbDeviceContext->BulkInPipeHandle[ENDPOINT_IN_84], 
            WDF_NO_HANDLE,
            NULL
            );

        WdfUsbTargetPipeResetSynchronously(
        pUsbDeviceContext->BulkInPipeHandle[ENDPOINT_IN_85], 
            WDF_NO_HANDLE,
            NULL
            );

        // Free all resources for the RECEIVE buffer queue.
        for (i = 0; i < (RX_RING_SIZE); i++)
        {
            pRxContext = &pAd->pHifCfg->RxContext[i];

            if (pRxContext->Request)
                WdfObjectDelete(pRxContext->Request);
            
            if (pRxContext->RxWdfMemory)
                WdfObjectDelete(pRxContext->RxWdfMemory);

            if (pRxContext->TransferBuffer != NULL)
            {
                PlatformFreeMemory(pRxContext->TransferBuffer, pRxContext->TransferbufferLen);
                pRxContext->TransferBuffer = NULL;
            }       
            NdisFreeSpinLock(&pRxContext->RxContextLock);
        }
    }

    //
    // Temporary put release LocalNetBufferList here.
    // We have to free LocalNetBufferList before free RecvPktsPool.
    // Since this LocalNetBufferList was allocated by RecvPktsPool handle.
    //
    if (pAd->pTxCfg != NULL)
    {
        for (i = 0; i < NUM_OF_LOCAL_RX_PKT_BUFFER; i++)
        {
            pLocalNetBufferList = &pAd->pTxCfg->LocalNetBufferList[i];

            if (pLocalNetBufferList->Mdl)
            {
                NdisFreeMdl(pLocalNetBufferList->Mdl);
                pLocalNetBufferList->Mdl = NULL;
            }

            if (pLocalNetBufferList->NetBufferList)
            {
                NdisFreeNetBufferList(pLocalNetBufferList->NetBufferList);
                pLocalNetBufferList->NetBufferList = NULL;
            }

            if (pLocalNetBufferList->Data)
            {
                PlatformFreeMemory(pLocalNetBufferList->Data, MAX_FRAME_SIZE);
                pLocalNetBufferList->Data = NULL;
            }
        }
    }    

    if (pAd->pRxCfg != NULL)
    {
        // Free  Rx Reordering buffer.  Usb 
        //
        for (num = 0; num < Max_BARECI_SESSION; num++)
        {
            pAd->pRxCfg->LocalRxReorderBuf[num].InUse = FALSE;
            for (index = 0; index < Max_RX_REORDERBUF; index++)
            {
                pReorderBuf = &pAd->pRxCfg->LocalRxReorderBuf[num].MAP_RXBuf[index];
                if (pReorderBuf->pBuffer != NULL)
                {
                    PlatformFreeMemory(pReorderBuf->pBuffer, RX_BUFFER_NORMSIZE);
                }
                pReorderBuf->IsFull = FALSE;
            }
        }

        for (i = 0; i < NUM_OF_LOCAL_RX_PKT_BUFFER; i++)
        {
            PMP_LOCAL_RX_PKTS pIndicateRxPkts = &pAd->pRxCfg->IndicateRxPkts[i];

            if (pIndicateRxPkts->Mdl)
            {
                NdisFreeMdl(pIndicateRxPkts->Mdl);
                pIndicateRxPkts->Mdl = NULL;
            }

            if (pIndicateRxPkts->NetBufferList)
            {
                NdisFreeNetBufferList(pIndicateRxPkts->NetBufferList);
                pIndicateRxPkts->NetBufferList = NULL;
            }

//          if (pIndicateRxPkts->Dummy)
//          {
//              PlatformFreeMemory111(pIndicateRxPkts->Dummy);
//              pIndicateRxPkts->Dummy = NULL;
//          }
        }

        if (pAd->pRxCfg->DummyForIndicatedMDL)
        {
            PlatformFreeMemory(pAd->pRxCfg->DummyForIndicatedMDL, ROUND_TO_PAGES(MAX_FRAME_SIZE) + PAGE_SIZE);
            pAd->pRxCfg->DummyForIndicatedMDL = NULL;
        }

        if (pAd->pRxCfg->RecvPktPool)
        {
            NdisFreeNetBufferPool(pAd->pRxCfg->RecvPktPool);
            pAd->pRxCfg->RecvPktPool = NULL;
        }
    
        if (pAd->pRxCfg->RecvPktsPool)
        {
            NdisFreeNetBufferListPool(pAd->pRxCfg->RecvPktsPool);
            pAd->pRxCfg->RecvPktsPool = NULL;
        }    
    }
}

VOID    
N6UsbFreeNICTransmit(
    IN PMP_ADAPTER pAd
    )
{
    UINT                i, acidx;
    PTX_CONTEXT         pNullContext;
    PTX_CONTEXT         pPsPollContext;
    PTX_CONTEXT         pRTSContext;
    PTX_CONTEXT         pPktCmdContext;
    PTX_CONTEXT         pFwPktContext;
    PTX_CONTEXT         pEPNullContext;
    PTX_CONTEXT         pMLMEContext;
    PHT_TX_CONTEXT      pTxContext;

    if (pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd is NULL\n", __FUNCTION__));  
        return;
    }

    if (pAd->pHifCfg != NULL)
    {
        // Free Bulkout temp Buffer
        for (i = 0; i < 4; i++)
        {       
            if ( NULL != pAd->pHifCfg->BulkOutTempBuf[i])
            {
                PlatformFreeMemory(pAd->pHifCfg->BulkOutTempBuf[i], LOCAL_TXBUF_SIZE_4K);
                pAd->pHifCfg->BulkOutTempBuf[i] = NULL;
            }
        }   

        // Free Fw Pkt resource
        pFwPktContext = &pAd->pHifCfg->FwPktContext;   
        if (NULL != pFwPktContext->TransferBuffer)
        {
            PlatformFreeMemory(pFwPktContext->TransferBuffer, sizeof(CMD_TX_BUFFER));
            pFwPktContext->TransferBuffer = NULL;
            if (pFwPktContext->WriteRequestArray[0] != NULL) 
            {
                WdfObjectDelete(pFwPktContext->WriteRequestArray[0]);
                pFwPktContext->WriteRequestArray[0]  = NULL;
            }
        }

        // Free Pkt Cmd resource    
        pPktCmdContext = &pAd->pHifCfg->PktCmdContext;
        if (NULL != pPktCmdContext->TransferBuffer)
        {
            PlatformFreeMemory(pPktCmdContext->TransferBuffer, sizeof(CMD_TX_BUFFER));
            pPktCmdContext->TransferBuffer = NULL;
            if (pPktCmdContext->WriteRequestArray[0] != NULL) 
            {
                WdfObjectDelete(pPktCmdContext->WriteRequestArray[0]);
                pPktCmdContext->WriteRequestArray[0]  = NULL;
            }
        }

        // Free PsPoll frame resource   
        pPsPollContext = &pAd->pHifCfg->PsPollContext;
        if (NULL != pPsPollContext->TransferBuffer)
        {
            PlatformFreeMemory(pPsPollContext->TransferBuffer, sizeof(TX_BUFFER));
            pPsPollContext->TransferBuffer = NULL;
            if (pPsPollContext->WriteRequestArray[0] != NULL) 
            {
                WdfObjectDelete(pPsPollContext->WriteRequestArray[0]);
                pPsPollContext->WriteRequestArray[0]  = NULL;
            }
        }

#if 1 // MT7603 FPGA
            
        for (i = 0; i < 6; i ++)
        {
            pEPNullContext   = &(pAd->pHifCfg->EPNullContext[i]);
            // Free EP NULL frame resource  
            if (NULL != pEPNullContext->TransferBuffer)
            {
                PlatformFreeMemory(pEPNullContext->TransferBuffer, sizeof(TX_BUFFER));
                pEPNullContext->TransferBuffer = NULL;
                if (pEPNullContext->WriteRequestArray[0] != NULL) 
                {
                    WdfObjectDelete(pEPNullContext->WriteRequestArray[0]);
                    pEPNullContext->WriteRequestArray[0]  = NULL;
                }
            }
        }

#endif
        // Free NULL frame resource 
        pNullContext   = &pAd->pHifCfg->NullContext;
        if (NULL != pNullContext->TransferBuffer)
        {
            PlatformFreeMemory(pNullContext->TransferBuffer, sizeof(TX_BUFFER));
            pNullContext->TransferBuffer = NULL;
            if (pNullContext->WriteRequestArray[0] != NULL) 
            {
                WdfObjectDelete(pNullContext->WriteRequestArray[0]);
                pNullContext->WriteRequestArray[0]  = NULL;
            }
        }

        // Free RTS frame resource  
        pRTSContext    = &pAd->pHifCfg->RTSContext;        
        if (NULL != pRTSContext->TransferBuffer)
        {
            PlatformFreeMemory(pRTSContext->TransferBuffer, sizeof(TX_BUFFER));
            pRTSContext->TransferBuffer = NULL;
            if (pRTSContext->WriteRequestArray[0] != NULL) 
            {
                WdfObjectDelete(pRTSContext->WriteRequestArray[0]);
                pRTSContext->WriteRequestArray[0]  = NULL;
            }
        }

        // Free Prio frame resource
        for (i = 0; i < PRIO_RING_SIZE; i++)
        {
            pMLMEContext = &(pAd->pHifCfg->MLMEContext[i]);
            
            if (pMLMEContext->WriteRequestArray[0] != NULL) 
            {
                WdfObjectDelete(pMLMEContext->WriteRequestArray[0]);
                pMLMEContext->WriteRequestArray[0]  = NULL;
            }
            
            if (pMLMEContext->TransferBuffer != NULL)
            {
                PlatformFreeMemory(pMLMEContext->TransferBuffer, sizeof(TX_BUFFER));
                pMLMEContext->TransferBuffer = NULL;
            }
        }
        
        // Free Tx frame resource
        for (acidx = 0; acidx < 4; acidx++)
        {
            pTxContext = &(pAd->pHifCfg->TxContext[acidx]);       

            for ( i = 0; i < MAX_WRITE_USB_REQUESTS ;i++)
            {
                if (pTxContext->WriteRequestArray[i] != NULL) 
                {
                    WdfObjectDelete(pTxContext->WriteRequestArray[i]);
                    pTxContext->WriteRequestArray[i] = NULL;
                }
            }
            
            if ( NULL != pTxContext->TransferBuffer )
            {
                PlatformFreeMemory(pTxContext->TransferBuffer,sizeof(HTTX_BUFFER));
                pTxContext->TransferBuffer = NULL;
            }
        }
    }
}

NTSTATUS 
N6UsbRxStaPacket(
    IN  PMP_ADAPTER  pAd,
    IN    BOOLEAN          bBulkReceive,
    IN  ULONG           nIndex
    )
{

    NDIS_STATUS ndisStatus = STATUS_MORE_PROCESSING_REQUIRED;
   // ULONG           DebugCount = 0;
    //BOOLEAN         bHtVhtFrame = FALSE;
    UCHAR           index = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    //PDOT11_PHY_FRAME_STATISTICS pPhyStats = &pAd->Counter.StatisticsInfo.PhyCounters[pPort->CommonCfg.DefaultPhyId];

    if ((pPort->CommonCfg.BACapability.field.AutoBA == FALSE) || (pPort->CommonCfg.PhyMode < PHY_11ABGN_MIXED))
    {
        NdisAcquireSpinLock(&pAd->pHifCfg->BulkInReadLock);
        pAd->pHifCfg->bBulkInRead = TRUE;
        //DBGPRINT(RT_DEBUG_TRACE, ("%s - pAd->pHifCfg->BulkInReadLock=TRUE, pPort->CommonCfg.PhyMode = %d, AutoBA = %d\n", __FUNCTION__, pPort->CommonCfg.PhyMode, pPort->CommonCfg.BACapability.field.AutoBA));    
    }
    
    RecvPreParseBuffer(pAd, nIndex);

    if (pAd->pHifCfg->bBulkInRead == TRUE)
    {
        pAd->pHifCfg->bBulkInRead = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->BulkInReadLock);
    }
    else
    {
        for (index = 0; index < RTMP_MAX_NUMBER_OF_PORT; index ++)
        {
            RecordWaitToIndicateRxPktsNumbyPort(pAd, index);
        }       
    }

    if (bBulkReceive == TRUE)
        N6USBBulkReceive(pAd);

    //
    // We return STATUS_MORE_PROCESSING_REQUIRED so that the completion
    // routine (IofCompleteRequest) will stop working on the irp.
    //
    return ndisStatus; 
}

/* 
    ==========================================================================
    Description:
        Send out a NULL frame to AP. The prpose is to inform AP this client 
        current PSM bit.
    NOTE:
        This routine should only be used in infrastructure mode.
    ==========================================================================
 */
VOID 
XmitSendNullFrameFor7603Lookback(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           EndpointIndex,
    IN  UCHAR           QueueId
    )
{
    PTX_CONTEXT             pEPNullContext;
#if 1
    PTXDSCR_LONG_STRUC      pLongFormatTxD;
#else
    PTXDSCR_SHORT_STRUC     pShortFormatTxD;
#endif
PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    UCHAR tempAddress[MAC_ADDR_LEN] = {0x00, 0x0C, 0x43, 0x33, 0x44, 0x55};

    UCHAR           PortSecured = pAd->StaCfg.PortSecured;
    WDFREQUEST          writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;
    HEADER_802_11   NullFrame;
    PUCHAR          ConcatedPoint;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->N6USBBulkOutNullFrame %d, Endpoint = %d, Queue = %d\n", pAd->TrackInfo.PacketCountFor7603Debug, EndpointIndex, QueueId));

    pEPNullContext = &(pAd->pHifCfg->EPNullContext[EndpointIndex]);
    if (pEPNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pEPNullContext->InUse = TRUE;
    
        PlatformZeroMemory(&NullFrame, sizeof(HEADER_802_11));
        
        NullFrame.FC.Type = BTYPE_DATA;
        NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
        
        NullFrame.FC.ToDs = 1;
        if (pPort->CommonCfg.bAPSDForcePowerSave)
        {
            NullFrame.FC.PwrMgmt = PWR_SAVE;
        }
        else
        {
            NullFrame.FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
        }
        NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcUsbDuration(pAd, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)], 14);

        pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
        NullFrame.Sequence = pAd->pTxCfg->Sequence;

        //**COPY_MAC_ADDR(pPort->NullFrame.Addr1, pPort->PortCfg.Bssid);
        //**COPY_MAC_ADDR(pPort->NullFrame.Addr2, pPort->CurrentAddress);
        //**COPY_MAC_ADDR(pPort->NullFrame.Addr3, pPort->PortCfg.Bssid);

        COPY_MAC_ADDR(NullFrame.Addr1, BROADCAST_ADDR);
        COPY_MAC_ADDR(NullFrame.Addr2, tempAddress);
        COPY_MAC_ADDR(NullFrame.Addr3, tempAddress);

        PlatformZeroMemory(&pEPNullContext->TransferBuffer->WirelessPacket[0], ONE_SEGMENT_UNIT);

        ConcatedPoint = &pEPNullContext->TransferBuffer->WirelessPacket[0];
        pEPNullContext->BulkOutSize = 0;

        {
            USHORT          i = 0, j = 0;
            UCHAR           Qindex[10] = {0, 1, 2, 3, 4, 9, 10, 11, 12, 13};
            int             PayloadLength = 0;

            for (i = 0; i < 20; i++)
            {
                PayloadLength = rand() % 1500;

                //PayloadLength = 1500;

                //if (PayloadLength > 0)
                //  NullFrame.FC.SubType = SUBTYPE_DATA;
#if 1               
                pLongFormatTxD = (PTXDSCR_LONG_STRUC)&ConcatedPoint[0];
#else
                pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&ConcatedPoint[0];
#endif

                pLongFormatTxD->TxDscrDW0.DestinationQueueID = QueueId;
                pLongFormatTxD->TxDscrDW0.DestinationPortID = 0;
#if 1               
                pLongFormatTxD->TxDscrDW0.TxByteCount = sizeof(TXDSCR_LONG_STRUC) + sizeof(NullFrame) + PayloadLength;
#else
                pShortFormatTxD->TxDscrDW0.TxByteCount = sizeof(TXDSCR_SHORT_STRUC) + sizeof(NullFrame) + PayloadLength;
#endif
                pLongFormatTxD->TxDscrDW1.WLANIndex = 0;
                pLongFormatTxD->TxDscrDW1.HeaderLength = (sizeof(HEADER_802_11) / 2); // word base
                pLongFormatTxD->TxDscrDW1.HeaderFormat = 0x2;
#if 1               
                pLongFormatTxD->TxDscrDW1.TxDescriptionFormat = TXD_LONG_FORMAT;
#else
                pShortFormatTxD->TxDscrDW1.TxDescriptionFormat = TXD_SHORT_FORMAT;
#endif
                pLongFormatTxD->TxDscrDW1.NoAck = 1;
#if 1               
                pLongFormatTxD->TxDscrDW2.BMPacket = 1;
                pLongFormatTxD->TxDscrDW2.FixedRate = 1;
                pLongFormatTxD->TxDscrDW2.BADisable = 1;
                pLongFormatTxD->TxDscrDW3.RemainingTxCount = 0x1f;
                pLongFormatTxD->TxDscrDW5.BARSSNContorl = 1;
                pLongFormatTxD->TxDscrDW6.RateToBeFixed = 0;
                pLongFormatTxD->TxDscrDW6.FixedBandwidthMode = 0x100;  // BW_20

                pLongFormatTxD->TxDscrDW6.RateToBeFixed = 0x4B;  // BW_20
                //pLongFormatTxD->TxDscrDW6.AntennaPriority = 2;  // BW_20
                //pLongFormatTxD->TxDscrDW6.SpetialExtenstionEnable = 1;  // BW_20
                ConcatedPoint += sizeof(TXDSCR_LONG_STRUC);
                pEPNullContext->BulkOutSize += sizeof(TXDSCR_LONG_STRUC);
#else
                ConcatedPoint += sizeof(TXDSCR_SHORT_STRUC);
                pEPNullContext->BulkOutSize += sizeof(TXDSCR_SHORT_STRUC);
#endif      


                PlatformMoveMemory(ConcatedPoint, &NullFrame, sizeof(NullFrame));
                ConcatedPoint += sizeof(NullFrame);
                pEPNullContext->BulkOutSize += sizeof(NullFrame);

                //PlatformMoveMemory(ConcatedPoint, Qindex, 10);
                //ConcatedPoint += 10;
                
                for (j = 0; j < PayloadLength; j++)
                {
                    ConcatedPoint[j] = QueueId + 1;
                }

                ConcatedPoint += PayloadLength;

                pEPNullContext->BulkOutSize += PayloadLength;

#if 1
                if (PayloadLength % 4 == 1)
                {
                    //pLongFormatTxD->TxDscrDW0.TxByteCount += 3;
                    ConcatedPoint += 3;
                    pEPNullContext->BulkOutSize += 3;
                }
                else if (PayloadLength % 4 == 2) 
                {
                    //pLongFormatTxD->TxDscrDW0.TxByteCount += 2;
                    ConcatedPoint += 2;
                    pEPNullContext->BulkOutSize += 2;
                }
                else if (PayloadLength % 4 == 3) 
                {
                    //pLongFormatTxD->TxDscrDW0.TxByteCount += 1;
                    ConcatedPoint += 1;
                    pEPNullContext->BulkOutSize += 1;
                }
#endif

                DBGPRINT(RT_DEBUG_TRACE, ("Total length = %d + %d = %d\n", sizeof(TXDSCR_LONG_STRUC) + sizeof(NullFrame), PayloadLength, sizeof(TXDSCR_LONG_STRUC) + sizeof(NullFrame) + PayloadLength));
                
            }
#if 0
            for (i = 0; i < pEPNullContext->BulkOutSize; i++)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%02x \n", pEPNullContext->TransferBuffer->WirelessPacket[i]));
                
            }
#endif          
        }

        DBGPRINT(RT_DEBUG_TRACE, ("Total concated length = %d\n", pEPNullContext->BulkOutSize));

        pEPNullContext->BulkOutSize += 4; // Bulk out end padding

#if 1
        if(1)
        {
            USHORT i = 0;
            PUCHAR p = &pEPNullContext->TransferBuffer->WirelessPacket[0];
            DBGPRINT(RT_DEBUG_TRACE, ("Send a NULL Frame to M2M\n"));

            DBGPRINT(RT_DEBUG_TRACE, ("TxD ===> %d\n", sizeof(TXDSCR_LONG_STRUC)));
            for(i=0; i < (USHORT)sizeof(TXDSCR_LONG_STRUC); i++)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%02x ", p[i]));
            }

            //DBGPRINT(RT_DEBUG_TRACE, ("Paload ===> %d, (pLongFormatTxD->TxDscrDW1.HeaderFormat = %d)\n", sizeof(HEADER_802_11), pLongFormatTxD->TxDscrDW1.HeaderFormat));
            //for(i=0; i < (USHORT)sizeof(HEADER_802_11); i++)
            //{
            //  DBGPRINT(RT_DEBUG_TRACE, ("%02x ", p[i+sizeof(TXDSCR_LONG_STRUC)]));
            //}
        }
#endif      
        // Fill out frame length information for global Bulk out arbitor
        //pNullContext->BulkOutSize = TransferBufferLength;
        //DBGPRINT(RT_DEBUG_TRACE, ("Send a NULL Frame to M2M\n"));
    }

#if 1
    pEPNullContext->EndPointIndex = EndpointIndex;
    NdisAcquireSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
    if (pAd->pHifCfg->bEndPointUsed[pEPNullContext->EndPointIndex] == TRUE)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, (" return pAd->pHifCfg->bEndPointUsed[%d] == TRUE\n", pEPNullContext->EndPointIndex));  
        NdisReleaseSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
        return;
    }
    pAd->pHifCfg->bEndPointUsed[pEPNullContext->EndPointIndex] = TRUE;
    NdisReleaseSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
#endif

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pEPNullContext->BulkOutSize;
    
    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pEPNullContext, ENDPOINT_OUT_8, MT7603BulkOutNullFrameComplete);
    writeRequest = pEPNullContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);

    if (WdfRequestSend(writeRequest, writeContext->IoTarget, WDF_NO_SEND_OPTIONS ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket send failed\n"));     
    }
    
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---N6USBBulkOutNullFrame \n")); 
}

/* 
    ==========================================================================
    Description:
        Send out a NULL frame to AP. The prpose is to inform AP this client 
        current PSM bit.
    NOTE:
        This routine should only be used in infrastructure mode.
    ==========================================================================
 */
VOID 
XmitSendNullFrameFor7603Test(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           EndpointIndex,
    IN  UCHAR           QueueId
    )
{
    PTX_CONTEXT             pEPNullContext;
#if 1
    PTXDSCR_LONG_STRUC      pLongFormatTxD;
#else
    PTXDSCR_SHORT_STRUC     pShortFormatTxD;
#endif
PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    UCHAR tempAddress[MAC_ADDR_LEN] = {0x00, 0x0C, 0x43, 0x33, 0x44, 0x55};

    UCHAR           PortSecured = pAd->StaCfg.PortSecured;
    WDFREQUEST          writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;
    HEADER_802_11   NullFrame;
    PUCHAR          ConcatedPoint;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->N6USBBulkOutNullFrame %d, Endpoint = %d, Queue = %d\n", pAd->TrackInfo.PacketCountFor7603Debug, EndpointIndex, QueueId));

    pEPNullContext = &(pAd->pHifCfg->EPNullContext[EndpointIndex]);
    if (pEPNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pEPNullContext->InUse = TRUE;
    
        PlatformZeroMemory(&NullFrame, sizeof(HEADER_802_11));
        
        NullFrame.FC.Type = BTYPE_DATA;
        NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
        
        NullFrame.FC.ToDs = 1;
        if (pPort->CommonCfg.bAPSDForcePowerSave)
        {
            NullFrame.FC.PwrMgmt = PWR_SAVE;
        }
        else
        {
            NullFrame.FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
        }
        NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcUsbDuration(pAd, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)], 14);

        pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
        NullFrame.Sequence = pAd->pTxCfg->Sequence;

        //**COPY_MAC_ADDR(pPort->NullFrame.Addr1, pPort->PortCfg.Bssid);
        //**COPY_MAC_ADDR(pPort->NullFrame.Addr2, pPort->CurrentAddress);
        //**COPY_MAC_ADDR(pPort->NullFrame.Addr3, pPort->PortCfg.Bssid);

        COPY_MAC_ADDR(NullFrame.Addr1, BROADCAST_ADDR);
        COPY_MAC_ADDR(NullFrame.Addr2, tempAddress);
        COPY_MAC_ADDR(NullFrame.Addr3, tempAddress);

        PlatformZeroMemory(&pEPNullContext->TransferBuffer->WirelessPacket[0], ONE_SEGMENT_UNIT);

        ConcatedPoint = &pEPNullContext->TransferBuffer->WirelessPacket[0];
        pEPNullContext->BulkOutSize = 0;

        {
            USHORT          i = 0, j = 0;
            UCHAR           Qindex[10] = {0, 1, 2, 3, 4, 9, 10, 11, 12, 13};
            int             PayloadLength = 0;

            for (i = 0; i < 1; i++)
            {
                PayloadLength = rand() % 1500;

                //PayloadLength = 1500;

                //if (PayloadLength > 0)
                //  NullFrame.FC.SubType = SUBTYPE_DATA;
#if 1               
                pLongFormatTxD = (PTXDSCR_LONG_STRUC)&ConcatedPoint[0];
#else
                pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&ConcatedPoint[0];
#endif

                pLongFormatTxD->TxDscrDW0.DestinationQueueID = QueueId;
                pLongFormatTxD->TxDscrDW0.DestinationPortID = 0;
#if 1               
                pLongFormatTxD->TxDscrDW0.TxByteCount = sizeof(TXDSCR_LONG_STRUC) + sizeof(NullFrame) + PayloadLength;
#else
                pShortFormatTxD->TxDscrDW0.TxByteCount = sizeof(TXDSCR_SHORT_STRUC) + sizeof(NullFrame) + PayloadLength;
#endif
                pLongFormatTxD->TxDscrDW1.WLANIndex = 0;
                pLongFormatTxD->TxDscrDW1.HeaderLength = (sizeof(HEADER_802_11) / 2); // word base
                pLongFormatTxD->TxDscrDW1.HeaderFormat = 0x2;
#if 1               
                pLongFormatTxD->TxDscrDW1.TxDescriptionFormat = TXD_LONG_FORMAT;
#else
                pShortFormatTxD->TxDscrDW1.TxDescriptionFormat = TXD_SHORT_FORMAT;
#endif
                pLongFormatTxD->TxDscrDW1.NoAck = 1;
#if 1               
                pLongFormatTxD->TxDscrDW2.BMPacket = 1;
                pLongFormatTxD->TxDscrDW2.FixedRate = 1;
                pLongFormatTxD->TxDscrDW2.BADisable = 1;
                pLongFormatTxD->TxDscrDW3.RemainingTxCount = 0x1f;
                pLongFormatTxD->TxDscrDW5.BARSSNContorl = 1;
                pLongFormatTxD->TxDscrDW6.RateToBeFixed = 0;
                pLongFormatTxD->TxDscrDW6.FixedBandwidthMode = 0x100;  // BW_20

                pLongFormatTxD->TxDscrDW6.RateToBeFixed = 0x8F;  // MCS_15
                //pLongFormatTxD->TxDscrDW6.AntennaPriority = 2;  // BW_20
                //pLongFormatTxD->TxDscrDW6.SpetialExtenstionEnable = 1;  // BW_20
                ConcatedPoint += sizeof(TXDSCR_LONG_STRUC);
                pEPNullContext->BulkOutSize += sizeof(TXDSCR_LONG_STRUC);
#else
                ConcatedPoint += sizeof(TXDSCR_SHORT_STRUC);
                pEPNullContext->BulkOutSize += sizeof(TXDSCR_SHORT_STRUC);
#endif      


                PlatformMoveMemory(ConcatedPoint, &NullFrame, sizeof(NullFrame));
                ConcatedPoint += sizeof(NullFrame);
                pEPNullContext->BulkOutSize += sizeof(NullFrame);

                //PlatformMoveMemory(ConcatedPoint, Qindex, 10);
                //ConcatedPoint += 10;
                
                for (j = 0; j < PayloadLength; j++)
                {
                    ConcatedPoint[j] = QueueId + 1;
                }

                ConcatedPoint += PayloadLength;

                pEPNullContext->BulkOutSize += PayloadLength;

#if 1
                if (PayloadLength % 4 == 1)
                {
                    //pLongFormatTxD->TxDscrDW0.TxByteCount += 3;
                    ConcatedPoint += 3;
                    pEPNullContext->BulkOutSize += 3;
                }
                else if (PayloadLength % 4 == 2) 
                {
                    //pLongFormatTxD->TxDscrDW0.TxByteCount += 2;
                    ConcatedPoint += 2;
                    pEPNullContext->BulkOutSize += 2;
                }
                else if (PayloadLength % 4 == 3) 
                {
                    //pLongFormatTxD->TxDscrDW0.TxByteCount += 1;
                    ConcatedPoint += 1;
                    pEPNullContext->BulkOutSize += 1;
                }
#endif

                DBGPRINT(RT_DEBUG_TRACE, ("Total length = %d + %d = %d\n", sizeof(TXDSCR_LONG_STRUC) + sizeof(NullFrame), PayloadLength, sizeof(TXDSCR_LONG_STRUC) + sizeof(NullFrame) + PayloadLength));
                
            }
#if 0
            for (i = 0; i < pEPNullContext->BulkOutSize; i++)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%02x \n", pEPNullContext->TransferBuffer->WirelessPacket[i]));
                
            }
#endif          
        }

        DBGPRINT(RT_DEBUG_TRACE, ("Total concated length = %d\n", pEPNullContext->BulkOutSize));

        pEPNullContext->BulkOutSize += 4; // Bulk out end padding

#if 1
        if(1)
        {
            USHORT i = 0;
            PUCHAR p = &pEPNullContext->TransferBuffer->WirelessPacket[0];
            DBGPRINT(RT_DEBUG_TRACE, ("Send a NULL Frame to M2M\n"));

            DBGPRINT(RT_DEBUG_TRACE, ("TxD ===> %d\n", sizeof(TXDSCR_LONG_STRUC)));
            for(i=0; i < (USHORT)sizeof(TXDSCR_LONG_STRUC); i++)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%02x ", p[i]));
            }

            //DBGPRINT(RT_DEBUG_TRACE, ("Paload ===> %d, (pLongFormatTxD->TxDscrDW1.HeaderFormat = %d)\n", sizeof(HEADER_802_11), pLongFormatTxD->TxDscrDW1.HeaderFormat));
            //for(i=0; i < (USHORT)sizeof(HEADER_802_11); i++)
            //{
            //  DBGPRINT(RT_DEBUG_TRACE, ("%02x ", p[i+sizeof(TXDSCR_LONG_STRUC)]));
            //}
        }
#endif      
        // Fill out frame length information for global Bulk out arbitor
        //pNullContext->BulkOutSize = TransferBufferLength;
        //DBGPRINT(RT_DEBUG_TRACE, ("Send a NULL Frame to M2M\n"));
    }

#if 1
    pEPNullContext->EndPointIndex = EndpointIndex;
    NdisAcquireSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
    if (pAd->pHifCfg->bEndPointUsed[pEPNullContext->EndPointIndex] == TRUE)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, (" return pAd->pHifCfg->bEndPointUsed[%d] == TRUE\n", pEPNullContext->EndPointIndex));  
        NdisReleaseSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
        return;
    }
    pAd->pHifCfg->bEndPointUsed[pEPNullContext->EndPointIndex] = TRUE;
    NdisReleaseSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
#endif

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pEPNullContext->BulkOutSize;
    
    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pEPNullContext, ENDPOINT_OUT_8, MT7603BulkOutNullFrameComplete);
    writeRequest = pEPNullContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);

    if (WdfRequestSend(writeRequest, writeContext->IoTarget, WDF_NO_SEND_OPTIONS ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket send failed\n"));     
    }
    
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---N6USBBulkOutNullFrame \n")); 
}

