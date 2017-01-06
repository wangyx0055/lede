/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    sdio.c

Abstract:
Environment:

    kernel

Notes:


Revision History:


--*/
#include "sdio_config.h"


//#include "sdio.h"
//#include "sdio_read.h"
#if 0
#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT, DriverEntry)
    #pragma alloc_text (PAGE, SDIOEvtDeviceAdd)
    #pragma alloc_text (PAGE, SDIOEvtIoDeviceControl)
   // #pragma alloc_text (PAGE, ReadRxIdx) 
#endif
#endif

BOOLEAN NoisyMode = FALSE;

WDFDEVICE g_device;
NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT  DriverObject,
           IN PUNICODE_STRING RegistryPath
           )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{

	NTSTATUS            status = STATUS_SUCCESS;
	WDF_DRIVER_CONFIG    config;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (">> RegistryPath: %wZ\n", RegistryPath));
	WDF_DRIVER_CONFIG_INIT(&config,
                           SDIOEvtDeviceAdd
                          );
	config.EvtDriverUnload = DriverUnload;
	status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             WDF_NO_OBJECT_ATTRIBUTES,
                             &config,
                             WDF_NO_HANDLE
                            );

    return status;
}

NTSTATUS
DriverEntry_SDIO(
           IN PDRIVER_OBJECT  DriverObject,
           IN PUNICODE_STRING RegistryPath
           )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{

	NTSTATUS            status = STATUS_SUCCESS;
	WDF_DRIVER_CONFIG    config;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (">> RegistryPath: %wZ\n", RegistryPath));
	WDF_DRIVER_CONFIG_INIT(&config,
                           SDIOEvtDeviceAdd
                          );
	config.EvtDriverUnload = DriverUnload;
	status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             WDF_NO_OBJECT_ATTRIBUTES,
                             &config,
                             WDF_NO_HANDLE
                            );

    return status;
}

VOID 
DriverUnload (IN WDFDRIVER  Driver)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DriverUnload()\n"));
}


NTSTATUS
SDIOEvtDeviceAdd(
                IN WDFDRIVER Driver,
                IN PWDFDEVICE_INIT DeviceInit
                )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS		status = STATUS_SUCCESS;
    //PFDO_DATA		fdoData;
    RTMP_ADAPTER		*pAdapter;
    WDF_IO_QUEUE_CONFIG        queueConfig;
    WDF_OBJECT_ATTRIBUTES    fdoAttributes;
    WDFDEVICE                device;
    WDF_PNPPOWER_EVENT_CALLBACKS          pnpPowerCallbacks;
    SDBUS_INTERFACE_PARAMETERS interfaceParameters = {0};
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: -->\n", __FUNCTION__));

	//DECLARE_CONST_UNICODE_STRING(mydevname, L"\\DosDevices\\MT6620_COMN001");
    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    WdfDeviceInitSetPowerPageable(DeviceInit);

    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	
    //
    // Register PNP callbacks.
    //
    pnpPowerCallbacks.EvtDevicePrepareHardware = SDIOEvtDevicePrepareHardware;
    pnpPowerCallbacks.EvtDeviceReleaseHardware = SDIOEvtDeviceReleaseHardware;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT(&fdoAttributes);
    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&fdoAttributes, RTMP_ADAPTER);

    status = WdfDeviceCreate(&DeviceInit, &fdoAttributes, &device);
    if (!NT_SUCCESS(status)) {
        return status;
    }

	//fdoData = SDIOFdoGetData(device); //Gets device context
	pAdapter = SDIOpAdGetData(device);


    //
    // Open an interface to the SD bus driver
    //
    status = SdBusOpenInterface(WdfDeviceWdmGetPhysicalDevice (device),
                                &pAdapter->BusInterface,
                                sizeof(SDBUS_INTERFACE_STANDARD),
                                SDBUS_INTERFACE_VERSION);

    if (!NT_SUCCESS(status)) {
        return status;
    }
	pAdapter->NextDeviceObject = WdfDeviceWdmGetAttachedDevice(device);
    interfaceParameters.Size                        = sizeof(SDBUS_INTERFACE_PARAMETERS);
    interfaceParameters.TargetObject                = WdfDeviceWdmGetAttachedDevice(device);
	interfaceParameters.CallbackAtDpcLevel          = FALSE;   //run at passive level
    interfaceParameters.DeviceGeneratesInterrupts   = TRUE;    //change to true eventually
    interfaceParameters.CallbackRoutine             = SDIOEventCallback;
    interfaceParameters.CallbackRoutineContext      = pAdapter;

    status = STATUS_UNSUCCESSFUL;
    if (pAdapter->BusInterface.InitializeInterface) {
        status = (pAdapter->BusInterface.InitializeInterface)(pAdapter->BusInterface.Context,
                                                             &interfaceParameters);
    }
	if(!pAdapter->NextDeviceObject)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: @@@@@@@@@@@@@@@@@@@@2!pAdapter->NextDeviceObject@@@@@@@@@@@@@@@@@@@@@\n", __FUNCTION__));	
    if (!NT_SUCCESS(status)) {
        return status;
    }


    //
    // now fill in the function number
    //

    status = SdioGetProperty(device,
                             SDP_FUNCTION_NUMBER,
                             &pAdapter->FunctionNumber,
                             sizeof(pAdapter->FunctionNumber));
    if (!NT_SUCCESS(status)) {
        return status;
    }


    pAdapter->DriverVersion = SDBUS_DRIVER_VERSION_1;

    SdioGetProperty(device,
                    SDP_BUS_DRIVER_VERSION,
                    &pAdapter->DriverVersion,
                    sizeof(pAdapter->DriverVersion));

    pAdapter->FunctionFocus = pAdapter->FunctionNumber;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAdapter->FunctionFocus: %d\n", pAdapter->FunctionNumber));
    if (pAdapter->DriverVersion < SDBUS_DRIVER_VERSION_2) {
        pAdapter->BlockMode = 0;
    } else {
        pAdapter->BlockMode = 1;
    }
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EvtDeviceAdd: block mode: %d\n", pAdapter->BlockMode));
    //
    //    Register New device
    //
	
    status = WdfDeviceCreateDeviceInterface(device,
                                            (LPGUID) &GUID_QA_MT7603S,
                                            NULL
                                           );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    pAdapter->WdfDevice = device;

	/* register I/O queue for read/write/ioctl routines */
	//create queue to process the IO
	status = CreateQueues(pAdapter);
	if (!NT_SUCCESS(status)) 
	{ 
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CreateQueues failed with status 0x%p !\n", status));
		return status;
	} 

    // set device to g_device
    g_device = device; 

	status = InitAdapterEvent(pAdapter);
	status = InitAdapterCommon(pAdapter);
		
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <--\n", __FUNCTION__));
    return status;
}

NTSTATUS
SDIOEvtDevicePrepareHardware(
                            WDFDEVICE Device,
                            WDFCMRESLIST Resources,
                            WDFCMRESLIST ResourcesTranslated
                            )
/*++

Routine Description:

    EvtDevicePrepareHardware event callback performs operations that are necessary
    to make the driver's device operational. The framework calls the driver's
    EvtDeviceStart callback when the PnP manager sends an IRP_MN_START_DEVICE
    request to the driver stack.

Arguments:

    Device - Handle to a framework device object.

    Resources - Handle to a collection of framework resource objects.
                This collection identifies the raw (bus-relative) hardware
                resources that have been assigned to the device.

    ResourcesTranslated - Handle to a collection of framework resource objects.
                This collection identifies the translated (system-physical)
                hardware resources that have been assigned to the device.
                The resources appear from the CPU's point of view.
                Use this list of resources to map I/O space and
                device-accessible memory into virtual address space

Return Value:

    WDF status code

--*/
{
	NTSTATUS	status = STATUS_SUCCESS;
	USHORT maxBlockLength;
	USHORT hostBlockLength;
	//PFDO_DATA	fdoData;
	RTMP_ADAPTER	*pAdapter;
	UCHAR		BusWidth = 1;
	ULONG		BusClock = 0;
	UCHAR		IntStaus = 0xff;
	ULONG		u4Value = 0;


	UNREFERENCED_PARAMETER(Resources);
	UNREFERENCED_PARAMETER(ResourcesTranslated);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: -->\n", __FUNCTION__));

	//
	// Get the host buffer block size
	//

	status = SdioGetProperty(Device,
                             SDP_HOST_BLOCK_LENGTH,
                             &hostBlockLength,
                             sizeof(hostBlockLength));

	if (!NT_SUCCESS(status)) {
		return status;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("hostBlockLength = %d\n", hostBlockLength));
	maxBlockLength = 512;  // just a value for testing

	if (hostBlockLength < maxBlockLength) {
		maxBlockLength = hostBlockLength;

	}
	
	//
	// set the block count since the SDIO board may not have any
	// tuples in its FLASH
	//

	status = SdioSetProperty(Device,
                             SDP_FUNCTION_BLOCK_LENGTH,
                             &maxBlockLength,
                             sizeof(maxBlockLength));

	//set bus width
	//BusWidth = 1; //1 or 4, 1 bit mode or 4 bit mode
	BusWidth = 4; //1 or 4, 1 bit mode or 4 bit mode
	status = SdioSetProperty(Device,
                             SDP_BUS_WIDTH,
                             &BusWidth,
                             sizeof(BusWidth));
	if (!NT_SUCCESS(status)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] Set bus width failed: %d (0x%x)\n", status, status));
		return status;
	}	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SD Bus Width = %d (0x%x)\n", BusWidth, BusWidth));
	
 

	//read clock  
	status = SdioGetProperty(Device,
                             SDP_BUS_CLOCK,
                             &BusClock,
                             sizeof(BusClock));
	if (!NT_SUCCESS(status)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] Get SD bus clock failed: %d (0x%x)\n", status, status));
		return status;
	}	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SD Bus Clock = %d (0x%x)\n", BusClock, BusClock));


	
	//fdoData = SDIOFdoGetData(Device); //Gets device context
	pAdapter = SDIOpAdGetData(Device); //Gets device context

	pAdapter->BlockLength = maxBlockLength;

	status = RTMP_SDIO_READ32(pAdapter, WCIR, &u4Value);

	if (!NT_SUCCESS(status)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] WCIR got error: %d (0x%x)\n", status, status));
		return status;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WCIR: 0x%x\n", u4Value)); 
	//init
	SDIO_init(pAdapter);
	RTMP_SDIO_READ32(pAdapter, WHISR,&u4Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): MCR_WHISR1: value:%x\n", __FUNCTION__,u4Value)); 
	
#if 0	
	status = NICLoadRomPatch(pAdapter);
	if(status != STATUS_SUCCESS)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s, NICLoadRomPatch failed, status 0x%x\n", __FUNCTION__,status); 
	status = NICLoadFirmware(pAdapter);
	if(status != STATUS_SUCCESS)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s, NICLoadFirmware failed, status 0x%x\n", __FUNCTION__,status); 
#endif		

/*
	pAdapter->TxBuffer[0] = 0x20;
	pAdapter->TxBuffer[1] = 0x04;
	pAdapter->TxBuffer[2] = 0x00;
	pAdapter->TxBuffer[3] = 0x00;

	pAdapter->TxBuffer[4] = 0x00;
	pAdapter->TxBuffer[5] = 0xCC;
	pAdapter->TxBuffer[6] = 0x00;
	pAdapter->TxBuffer[7] = 0x00;

	pAdapter->TxBuffer[8] = 0x00;
	pAdapter->TxBuffer[9] = 0x04;
	pAdapter->TxBuffer[10] = 0x00;
	pAdapter->TxBuffer[11] = 0xa0;

	pAdapter->TxBuffer[12] = 0x00;
	pAdapter->TxBuffer[13] = 0xf8;
	pAdapter->TxBuffer[14] = 0x00;
	pAdapter->TxBuffer[15] = 0x00;

	pAdapter->TxBuffer[16] = 0x00;
	pAdapter->TxBuffer[17] = 0x00;
	pAdapter->TxBuffer[18] = 0x00;
	pAdapter->TxBuffer[19] = 0x00;

	pAdapter->TxBuffer[20] = 0x00;
	pAdapter->TxBuffer[21] = 0x15;
	pAdapter->TxBuffer[22] = 0x00;
	pAdapter->TxBuffer[23] = 0x00;

	pAdapter->TxBuffer[24] = 0x00;
	pAdapter->TxBuffer[25] = 0x0c;
	pAdapter->TxBuffer[26] = 0x30;
	pAdapter->TxBuffer[27] = 0x01;

	pAdapter->TxBuffer[28] = 0x00;
	pAdapter->TxBuffer[29] = 0x00;
	pAdapter->TxBuffer[30] = 0x00;
	pAdapter->TxBuffer[31] = 0x00;

	pAdapter->TxBuffer[32] = 0x08;
	pAdapter->TxBuffer[33] = 0x00;
	pAdapter->TxBuffer[34] = 0x00;
	pAdapter->TxBuffer[35] = 0x00;

	pAdapter->TxBuffer[36] = 0xff;
	pAdapter->TxBuffer[37] = 0xff;
	pAdapter->TxBuffer[38] = 0xff;
	pAdapter->TxBuffer[39] = 0xff;

	pAdapter->TxBuffer[40] = 0xff;
	pAdapter->TxBuffer[41] = 0xff;
	pAdapter->TxBuffer[42] = 0x00;
	pAdapter->TxBuffer[43] = 0x00;

	pAdapter->TxBuffer[44] = 0x00;
	pAdapter->TxBuffer[45] = 0x00;
	pAdapter->TxBuffer[46] = 0x00;
	pAdapter->TxBuffer[47] = 0x00;

	pAdapter->TxBuffer[48] = 0x00;
	pAdapter->TxBuffer[49] = 0x11;
	pAdapter->TxBuffer[50] = 0x22;
	pAdapter->TxBuffer[51] = 0x33;

	pAdapter->TxBuffer[52] = 0x44;
	pAdapter->TxBuffer[53] = 0x55;
	pAdapter->TxBuffer[54] = 0x00;
	pAdapter->TxBuffer[55] = 0x00;

	pAdapter->TxBufferLength = 0x420;

	
	RTMPusecDelay(1000000);
	MtAsicTOPInit(pAdapter);
	mt_hif_sys_init(pAdapter);
	//asic_set_tmac_info_template(pAdapter);
	mt7603_init_mac_cr(pAdapter);
	RTMPusecDelay(1000000);
	pAdapter->TxRemained = 3;	
	pAdapter->bLoadingFW= TRUE;
	SDIO_TxData(pAdapter);
	pAdapter->bLoadingFW= FALSE;
	*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <--\n", __FUNCTION__));
	return status;
}


NTSTATUS
SDIOEvtDeviceReleaseHardware(
                            IN  WDFDEVICE Device,
                            IN  WDFCMRESLIST ResourcesTranslated
                            )
/*++

Routine Description:

    EvtDeviceReleaseHardware is called by the framework whenever the PnP manager
    is revoking ownership of our resources.  This may be in response to either
    IRP_MN_STOP_DEVICE or IRP_MN_REMOVE_DEVICE.  The callback is made before
    passing down the IRP to the lower driver.

    In this callback, do anything necessary to free those resources.

Arguments:

    Device - Handle to a framework device object.

    ResourcesTranslated - Handle to a collection of framework resource objects.
                This collection identifies the translated (system-physical)
                hardware resources that have been assigned to the device.
                The resources appear from the CPU's point of view.
                Use this list of resources to map I/O space and
                device-accessible memory into virtual address space

Return Value:

    NTSTATUS - Failures will be logged, but not acted on.

--*/
{
    //PFDO_DATA   fdoData;
    RTMP_ADAPTER	*pAdapter;
    UNREFERENCED_PARAMETER(ResourcesTranslated);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: -->\n", __FUNCTION__));

    //fdoData = 
	pAdapter = SDIOpAdGetData(Device);
		
    if (pAdapter->BusInterface.InterfaceDereference) {

        (pAdapter->BusInterface.InterfaceDereference)(pAdapter->BusInterface.Context);
        pAdapter->BusInterface.InterfaceDereference = NULL;

    }
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <--\n", __FUNCTION__));
    return STATUS_SUCCESS;
}

//
// Io events callbacks.
//
VOID
SDIOEvtIoDeviceControl(
                      IN WDFQUEUE Queue,
                      IN WDFREQUEST Request,
                      IN size_t OutputBufferLength,
                      IN size_t InputBufferLength,
                      IN ULONG IoControlCode
                      )
/*++

Routine Description:

    This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
    requests from the system.

Arguments:

    Queue - Handle to the framework queue object that is associated
            with the I/O request.
    Request - Handle to a framework request object.

    OutputBufferLength - length of the request's output buffer,
                        if an output buffer is available.
    InputBufferLength - length of the request's input buffer,
                        if an input buffer is available.

    IoControlCode - the driver-defined or system-defined I/O control code
                    (IOCTL) that is associated with the request.
Return Value:

    VOID

--*/
{
//    PFDO_DATA               fdoData = NULL;
	RTMP_ADAPTER	*pAdapter=NULL;
    WDFDEVICE               device;
    NTSTATUS                status = STATUS_SUCCESS;
    PVOID                   inBuffer = NULL, outBuffer = NULL;
    size_t                  bytesReturned = 0;
    size_t                  size = 0;

    PAGED_CODE();

    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, ("Started SDIOEvtIODeviceControl\n"));
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s: -->\n", __FUNCTION__);
    device = WdfIoQueueGetDevice(Queue);
    //fdoData =            
    pAdapter =  SDIOpAdGetData(device);//Gets device context
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s InputBufferLength = %d, OutputBufferLength = %d, IOCTL = 0x%x\n", __FUNCTION__, InputBufferLength, OutputBufferLength, IoControlCode));
    //
    // Get the ioctl input & output buffers
    //
	if (InputBufferLength != 0) {
		status = WdfRequestRetrieveInputBuffer(Request,
                                               InputBufferLength,
                                               &inBuffer,
                                               &size);

		if (!NT_SUCCESS(status)) {
			WdfRequestComplete(Request, status);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s Retrieve InputBuffer failed, status = 0x%x\n", __FUNCTION__, status));    
			return;
		}
	}
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s Retrieve InputBuffer size = %d\n", __FUNCTION__, size));  
	if (OutputBufferLength != 0) {
		status = WdfRequestRetrieveOutputBuffer(Request,
                                                OutputBufferLength,
                                                &outBuffer,
                                                &size);
		if (!NT_SUCCESS(status)) {
			WdfRequestComplete(Request, status);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s Retrieve OutputBuffer failed, status = 0x%x\n", __FUNCTION__, status));    
			return;
		}
	}
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s Retrieve OutputBuffer size = %d\n", __FUNCTION__, size));  

    switch (IoControlCode) {
    
    case IOCTL_SDIO_GET_DRIVER_VERSION:

        if (OutputBufferLength < sizeof(USHORT)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        *(PUSHORT)outBuffer = pAdapter->DriverVersion;
        bytesReturned = sizeof(USHORT);
        break;


    case IOCTL_SDIO_GET_FUNCTION_NUMBER:

        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        *(PUCHAR)outBuffer = pAdapter->FunctionNumber;
        bytesReturned  = sizeof(UCHAR);
        break;


    case IOCTL_SDIO_GET_FUNCTION_FOCUS:

        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        *(PUCHAR)outBuffer = pAdapter->FunctionFocus;
        bytesReturned  = sizeof(UCHAR);
        break;


    case IOCTL_SDIO_SET_FUNCTION_FOCUS:

        if (InputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        pAdapter->FunctionFocus = *(PUCHAR)inBuffer;
        break;


        //---------------------------------------------------
        //
        // SDP_BUS_WIDTH
        //
        //---------------------------------------------------

    case IOCTL_SDIO_GET_BUS_WIDTH:

        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = SdioGetProperty(device, SDP_BUS_WIDTH,
                                 outBuffer, sizeof(UCHAR));

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(UCHAR);
        }
        break;


    case IOCTL_SDIO_SET_BUS_WIDTH:

        if (InputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = SdioSetProperty(device, SDP_BUS_WIDTH,
                                 inBuffer, sizeof(UCHAR));
        break;


        //---------------------------------------------------
        //
        // SDP_BUS_CLOCK
        //
        //---------------------------------------------------

    case IOCTL_SDIO_GET_BUS_CLOCK:

        if (OutputBufferLength < sizeof(ULONG)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = SdioGetProperty(device, SDP_BUS_CLOCK,
                                 outBuffer, sizeof(ULONG));

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(ULONG);
        }
        break;


    case IOCTL_SDIO_SET_BUS_CLOCK:

        if (InputBufferLength < sizeof(ULONG)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = SdioSetProperty(device, SDP_BUS_CLOCK,
                                 inBuffer, sizeof(ULONG));
        break;


        //---------------------------------------------------
        //
        // SDP_FUNCTION_BLOCK_LENGTH
        //
        //---------------------------------------------------


    case IOCTL_SDIO_GET_BLOCKLEN:

        if (OutputBufferLength < sizeof(USHORT)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioGetProperty(device, SDP_FUNCTION_BLOCK_LENGTH,
                                 outBuffer, sizeof(SHORT));

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(USHORT);
        }
        break;


    case IOCTL_SDIO_SET_BLOCKLEN:

        if (InputBufferLength < sizeof(USHORT)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioSetProperty(device, SDP_FUNCTION_BLOCK_LENGTH,
                                 inBuffer, sizeof(SHORT));
        break;


        //---------------------------------------------------
        //
        // SDP_FN0_BLOCK_LENGTH
        //
        //---------------------------------------------------


    case IOCTL_SDIO_GET_FN0_BLOCKLEN:

        if (OutputBufferLength < sizeof(USHORT)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioGetProperty(device, SDP_FN0_BLOCK_LENGTH,
                                 outBuffer, sizeof(SHORT));

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(USHORT);
        }
        break;


    case IOCTL_SDIO_SET_FN0_BLOCKLEN:

        if (InputBufferLength < sizeof(USHORT)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioSetProperty(device, SDP_FN0_BLOCK_LENGTH,
                                 inBuffer, sizeof(SHORT));
        break;


        //---------------------------------------------------
        //
        // SDP_BUS_INTERFACE_CONTROL
        //
        //---------------------------------------------------


    case IOCTL_SDIO_GET_BUS_INTERFACE_CONTROL:

        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioGetProperty(device, SDP_BUS_INTERFACE_CONTROL,
                                 outBuffer, sizeof(UCHAR));

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(UCHAR);
        }
        break;


    case IOCTL_SDIO_SET_BUS_INTERFACE_CONTROL:

        if (InputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioSetProperty(device, SDP_BUS_INTERFACE_CONTROL,
                                 inBuffer, sizeof(UCHAR));
        break;


        //---------------------------------------------------
        //
        // SDP_FUNCTION_INT_ENABLE
        //
        //---------------------------------------------------


    case IOCTL_SDIO_GET_INT_ENABLE:

        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioGetProperty(device, SDP_FUNCTION_INT_ENABLE,
                                 outBuffer, sizeof(UCHAR));

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(UCHAR);
        }
        break;


    case IOCTL_SDIO_SET_INT_ENABLE:

        if (InputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = SdioSetProperty(device, SDP_FUNCTION_INT_ENABLE,
                                 inBuffer, sizeof(UCHAR));
        break;


        //---------------------------------------------------
        //
        // READ/WRITE BYTE
        //
        //---------------------------------------------------


    case IOCTL_SDIO_READ_BYTE:

        if ((InputBufferLength < sizeof(ULONG)) || (OutputBufferLength < sizeof(UCHAR))) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = SdioReadWriteByte(device,
                                   pAdapter->FunctionFocus,
                                   (PUCHAR)outBuffer,
                                   *(PULONG)inBuffer,
                                   FALSE);

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(UCHAR);
        }

        break;


    case IOCTL_SDIO_WRITE_BYTE:

        if ((InputBufferLength < sizeof(ULONG)*2)) {//||(OutputBufferLength < sizeof(UCHAR))) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        // BUGBUG: check for output buffer length

        status = SdioReadWriteByte(device,
                                   pAdapter->FunctionFocus,
                                   (PUCHAR)(&((PULONG)inBuffer)[1]),
                                   *(PULONG)inBuffer,
                                   TRUE);

        // BUGBUG: Return the right size

        if (NT_SUCCESS(status)) {
            bytesReturned  = sizeof(UCHAR);
        }


        break;


        //---------------------------------------------------
        //
        // Mode settings
        //
        //---------------------------------------------------

    case IOCTL_SDIO_SET_TRANSFER_MODE:

        bytesReturned  = 0;

        break;

    case IOCTL_SDIO_TOGGLE_MODE:

        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if (pAdapter->DriverVersion < SDBUS_DRIVER_VERSION_2) {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        pAdapter->BlockMode = pAdapter->BlockMode ? 0 : 1;
        *(PUCHAR)outBuffer = pAdapter->BlockMode;
        bytesReturned  = sizeof(UCHAR);
        break;


    case IOCTL_SDIO_TOGGLE_NOISY:

        if (OutputBufferLength < sizeof(BOOLEAN)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        NoisyMode = NoisyMode ? 0 : 1;
        *(PBOOLEAN)outBuffer = NoisyMode;
        bytesReturned  = sizeof(BOOLEAN);

        if (NoisyMode) {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SDIO: Noisy mode\n"));
        } else {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SDIO: Quiet mode\n"));
        }
        break;


    default:
        //ASSERTMSG(FALSE, "Invalid IOCTL request\n");

        //status = STATUS_INVALID_DEVICE_REQUEST;

	status = DevIoctlHandler(pAdapter, InputBufferLength, inBuffer, OutputBufferLength, outBuffer, IoControlCode, &bytesReturned);
	if(IOCTL_GET_PACKETCMD_SEQNUM != IoControlCode)
	{
		//outBuffer = inBuffer;//tmp
	}
#if 0
	do
	{
		ULONG* pOutBuf = (ULONG*)outBuffer;
		ULONG* pInBuf = (ULONG*)inBuffer;
		ULONG In;
		ULONG Out;
		if(pInBuf)
			In = pInBuf[0];				//addr
		if(pOutBuf)
			Out =  pOutBuf[0];
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "in: 0x%x, out: 0x%x, bytesReturned = %d\n", In, Out, bytesReturned);	
	}while (FALSE);
#endif	
    }
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s: <--\n", __FUNCTION__);
	if(Request)
	{
		WdfRequestCompleteWithInformation(Request, status, bytesReturned);
	}
    return;
}

VOID
SDIOEventCallback(
   IN PVOID Context,
   IN ULONG InterruptType
   )
/*++

Routine Description:

    This routine is called by the SD bus driver when a card interrupt is
    detected. It will launch the EventWorker routine which reads the data from
    the card.

Arguments:

    Context, interrupt type are defined in the SDBUS api

Return Values:

    none

--*/
{

        //PFDO_DATA fdoData = (PFDO_DATA) Context;
        RTMP_ADAPTER *pAdapter = (PRTMP_ADAPTER) Context;
	NTSTATUS status = STATUS_SUCCESS;
	
	

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "{}enter SDIOEventCallback\n");
    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "{}interruptType = %d (0x%x)\n", InterruptType, InterruptType);
 
//	status = IoAcquireRemoveLock(&(pAdapter->io_remove_lock), NULL);
    //if(status != STATUS_SUCCESS )
	//{
	//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "IoAcquireRemoveLock failed in EvtCallback, 0x%x\n", status);
	//	return;
	//}

	//debug code snippet
	//irql = KeGetCurrentIrql();
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "{}current irql = %d (0x%x)\n", (ULONG)irql, (ULONG)irql);
/*


*/		

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Wait for 5s for test.....\n");
	//WaitMicroSecond1(1000* 1 * 1000 * 5);
 
	/* read from WHISR */
	status = SDIO_GET_INTR(pAdapter, &(pAdapter->currentIntrStatus));
	if (!NT_SUCCESS(status)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] HAL_GET_INTR got error: %d (0x%x)\n", status, status));
	} 
	else{
		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("{}intrStatus = 0x%x\n", pAdapter->currentIntrStatus.WHISRVal));
		
		/////
		DispatchAndHandleInterrupt(pAdapter);
	}

    /////do any thing before here/////

    (pAdapter->BusInterface.AcknowledgeInterrupt)(pAdapter->BusInterface.Context);
 
	//use this lock to make sure that pAdapter and BusInterface are always valid.
	//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);

   // MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "{}leave SDIOEventCallback\n");	
}

VOID DispatchAndHandleInterrupt(RTMP_ADAPTER *pAdapter)
{
	size_t fifoFreeCnt=0;
	NTSTATUS status = STATUS_SUCCESS;
	UINT32 IntStatus = pAdapter->currentIntrStatus.WHISRVal;
	if (IntStatus & ABNORMAL_INT)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ABNORMAL_INT\n", __FUNCTION__));
		MTSDIOProcessAbnormalInterrupt(pAdapter);
	}
	//if ((IntStatus & RX0_DONE_INT) || (IntStatus & RX1_DONE_INT))
	if (IntStatus & (RX0_DONE_INT|  RX1_DONE_INT))
	{
	//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s, RX0_DONE_INT | RX1_DONE_INT\n", __FUNCTION__);
		MTSDIOProcessRxInterrupt(pAdapter);
	}	
	if (IntStatus & TX_DONE_INT)
	{
	//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s, TX_DONE_INT\n", __FUNCTION__);		
		MTSDIOProcessTxInterrupt(pAdapter);
		
	}

	return;
}
/*
NTSTATUS
SdioReadWriteBuffer(
                   IN WDFDEVICE Device,
                   IN ULONG Function,
                   IN PMDL Mdl,
                   IN ULONG Address,
                   IN ULONG Length,
                   IN BOOLEAN WriteToDevice,
                   OUT PULONG BytesRead
                   )
{
*/
NTSTATUS
SdioReadWriteBuffer(
                   IN WDFDEVICE Device,
                   IN ULONG Function,
                   IN PMDL Mdl,
                   IN ULONG Address,
                   IN ULONG Length,
                   IN BOOLEAN FIFOMode,
                   IN BOOLEAN WriteToDevice,
                   OUT PULONG BytesRead
                   )
{
	NTSTATUS   status=0;
//	PFDO_DATA   fdoData;
	RTMP_ADAPTER *pAdapter;
	BOOLEAN BlockMode; 

    	pAdapter = SDIOpAdGetData(Device);
    	BlockMode = (pAdapter->BlockMode==1) ? TRUE : FALSE;

	if(WriteToDevice)
	{
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "-->-->-->-->-->-->-->-->-->-->write to device data buffer:\n");
		//printMDL(Mdl, 1); //for debug
	}

	
    	status = SdioReadWriteBuffer_BlockMode(Device, Function, Mdl, Address, 
		                     Length, FIFOMode, WriteToDevice, BytesRead, BlockMode);
    	if (!NT_SUCCESS(status)) 
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] SdioReadWriteBuffer_BlockMode return failed: %d (0x%x), blockmode=%d\n", status, status, BlockMode));
        		return status;
    	}	
	return status;

}
NTSTATUS
SdioReadWriteBuffer_BlockMode(
                   IN WDFDEVICE Device,
                   IN ULONG Function,
                   IN PMDL Mdl,
                   IN ULONG Address,
                   IN ULONG Length,
                   IN BOOLEAN FIFOMode,
                   IN BOOLEAN WriteToDevice,
                   OUT PULONG BytesRead,
	     IN BOOLEAN BlockMode
                   )
{
//    PFDO_DATA   fdoData;
	RTMP_ADAPTER *pAdapter;
    SDBUS_REQUEST_PACKET sdrp;
    SD_RW_EXTENDED_ARGUMENT extendedArgument;
    NTSTATUS                 status;
    const SDCMD_DESCRIPTOR ReadIoExtendedDesc =
    {SDCMD_IO_RW_EXTENDED, SDCC_STANDARD, SDTD_READ, SDTT_SINGLE_BLOCK, SDRT_5};

    const SDCMD_DESCRIPTOR WriteIoExtendedDesc =
    {SDCMD_IO_RW_EXTENDED, SDCC_STANDARD, SDTD_WRITE, SDTT_SINGLE_BLOCK, SDRT_5};

    PAGED_CODE();
    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "enter SdioReadWriteBuffer()\n");  //to construct CMD53

    //fdoData = 
    pAdapter = SDIOpAdGetData(Device);

    RtlZeroMemory(&sdrp, sizeof(SDBUS_REQUEST_PACKET));

    sdrp.RequestFunction = SDRF_DEVICE_COMMAND;
    sdrp.Parameters.DeviceCommand.Mdl = Mdl;

    extendedArgument.u.AsULONG = 0;
    extendedArgument.u.bits.Function = Function;
    if(FIFOMode == FALSE)
        extendedArgument.u.bits.OpCode = 1;             // increment address
    else
        extendedArgument.u.bits.OpCode = 0;             // FIFO read

    extendedArgument.u.bits.BlockMode = BlockMode;
    extendedArgument.u.bits.Address = Address;

    if (WriteToDevice) {
        extendedArgument.u.bits.WriteToDevice = 1;
        sdrp.Parameters.DeviceCommand.CmdDesc  = WriteIoExtendedDesc;
    } else {
        sdrp.Parameters.DeviceCommand.CmdDesc  = ReadIoExtendedDesc;
    }

    //if (pAdapter->firmwareinfo.BlockMode == 1) {
    if(BlockMode){
        sdrp.Parameters.DeviceCommand.CmdDesc.TransferType = SDTT_MULTI_BLOCK_NO_CMD12;
    }
	else
	{
		sdrp.Parameters.DeviceCommand.CmdDesc.TransferType = SDTT_SINGLE_BLOCK;
	}

    sdrp.Parameters.DeviceCommand.Argument = extendedArgument.u.AsULONG;
    sdrp.Parameters.DeviceCommand.Length   = Length;
    /*
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Function = 0x%x\n", Function);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "FIFOMode = 0x%x\n", FIFOMode);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Address = 0x%x\n", Address);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "WriteToDevice = 0x%x\n", WriteToDevice);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "BlockMode = 0x%x\n", BlockMode);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Argument = 0x%x\n", extendedArgument.u.AsULONG);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Length = 0x%x\n", Length);
*/	
    //
    // Send the IO request down to the bus driver
    //
    status = SdBusSubmitRequest(pAdapter->BusInterface.Context, &sdrp);
    *BytesRead = (ULONG)sdrp.Information;
      
    return status;
}

NTSTATUS
SdioReadWriteByte(
                 IN WDFDEVICE Device,
                 IN ULONG Function,
                 IN PUCHAR Data,
                 IN ULONG Address,
                 IN BOOLEAN WriteToDevice
                 )
/*++


--*/

{
//    PFDO_DATA                    fdoData;
	RTMP_ADAPTER	*pAdapter;
    NTSTATUS                    status;
    SDBUS_REQUEST_PACKET        sdrp;
    SD_RW_DIRECT_ARGUMENT        directArgument;

    const SDCMD_DESCRIPTOR ReadIoDirectDesc =
    {SDCMD_IO_RW_DIRECT, SDCC_STANDARD, SDTD_READ, SDTT_CMD_ONLY, SDRT_5};

    const SDCMD_DESCRIPTOR WriteIoDirectDesc =
    {SDCMD_IO_RW_DIRECT, SDCC_STANDARD, SDTD_WRITE, SDTT_CMD_ONLY, SDRT_5};
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: -->\n", __FUNCTION__));
    //
    // get an SD request packet
    //

//    fdoData = 
	pAdapter = SDIOpAdGetData(Device);

    RtlZeroMemory(&sdrp, sizeof(SDBUS_REQUEST_PACKET));

    sdrp.RequestFunction = SDRF_DEVICE_COMMAND;

    directArgument.u.AsULONG = 0;
    directArgument.u.bits.Function = Function;
    directArgument.u.bits.Address = Address;


    if (WriteToDevice) {
        directArgument.u.bits.WriteToDevice = 1;
        directArgument.u.bits.Data = *Data;
        sdrp.Parameters.DeviceCommand.CmdDesc  = WriteIoDirectDesc;
    } else {
        sdrp.Parameters.DeviceCommand.CmdDesc  = ReadIoDirectDesc;
    }

    sdrp.Parameters.DeviceCommand.Argument = directArgument.u.AsULONG;

    //
    // Send the IO request down to the bus driver
    //

    status = SdBusSubmitRequest(pAdapter->BusInterface.Context, &sdrp);    

    if (NT_SUCCESS(status) && !WriteToDevice) {
        *Data = sdrp.ResponseData.AsUCHAR[0];
    }
    else if (!NT_SUCCESS(status)) {       
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] SdBusSubmitRequest:  fail: %d (0x%x)\n", status, status));
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave SdioReadWriteByte()\n");
		return status;
    }
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <--\n", __FUNCTION__));
    return status;
}

NTSTATUS
SdioGetProperty(
               IN WDFDEVICE Device,
               IN SDBUS_PROPERTY Property,
               IN PVOID Buffer,
               IN ULONG Length
               )
{
//	PFDO_DATA fdoData;
	RTMP_ADAPTER *pAdapter;
	SDBUS_REQUEST_PACKET sdrp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: -->\n", __FUNCTION__));
	//fdoData = 
	pAdapter = SDIOpAdGetData(Device);
	RtlZeroMemory(&sdrp, sizeof(SDBUS_REQUEST_PACKET));

	sdrp.RequestFunction = SDRF_GET_PROPERTY;
	sdrp.Parameters.GetSetProperty.Property = Property;
	sdrp.Parameters.GetSetProperty.Buffer = Buffer;
	sdrp.Parameters.GetSetProperty.Length = Length;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <--\n", __FUNCTION__));
	return SdBusSubmitRequest(pAdapter->BusInterface.Context, &sdrp);
}


NTSTATUS
SdioSetProperty(
               IN WDFDEVICE Device,
               IN SDBUS_PROPERTY Property,
               IN PVOID Buffer,
               IN ULONG Length
               )
{
	//PFDO_DATA fdoData;
	RTMP_ADAPTER *pAdapter;
	SDBUS_REQUEST_PACKET sdrp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: -->\n", __FUNCTION__));
	//fdoData = SDIOFdoGetData(Device);
	pAdapter = SDIOpAdGetData(Device);
	RtlZeroMemory(&sdrp, sizeof(SDBUS_REQUEST_PACKET));

	sdrp.RequestFunction = SDRF_SET_PROPERTY;
	sdrp.Parameters.GetSetProperty.Property = Property;
	sdrp.Parameters.GetSetProperty.Buffer = Buffer;
	sdrp.Parameters.GetSetProperty.Length = Length;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <--\n", __FUNCTION__));
	return SdBusSubmitRequest(pAdapter->BusInterface.Context, &sdrp);
}

VOID WaitMicroSecond1(ULONG ulMicroSecond)
{
    KEVENT kEvent;
	LARGE_INTEGER timeout;
    NTSTATUS    status;

	KeInitializeEvent(&kEvent, NotificationEvent , FALSE);
 
    timeout.QuadPart = (-10LL) * ulMicroSecond;

	status = KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, &timeout);
 
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WaitMicroSecond1: KeWaitForSingleObject: (0x%x)\n", status));
	 
	KeClearEvent(&kEvent);
}
 

/*
len:  the one who needs to be align
nBytesAlign: how many bytes align, for example, nBytesAlign==4,  is four bytes align

return: align result
*/ 
ULONG NBytesAlign(ULONG len, ULONG nBytesAlign)
{
	//if(oldLen & 0x3){ // not aligned with 4-bytes, padding
	//	newLen += (4-oldLen%4); 
	//}

	if(nBytesAlign > 0xFFFF)
	{
		//Block size too large?
		//should report error?
		return 0xFFFFFFFF;
	}

    if( len%nBytesAlign == 0 )
	{
		return len;
	}

	len += nBytesAlign - len%nBytesAlign;

	return len;
}
/**
* @Routine Description:
*			
*			Create all the queue
*
* @param pDeviceExtContext : device extension
*
* @return NTSTATUS.
**/
NTSTATUS CreateQueues(IN RTMP_ADAPTER *pAdapter)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDFQUEUE wdfDefaultQueue; 
	WDF_IO_QUEUE_CONFIG     wdfIOQueueConfig;
	
	PAGED_CODE();

	//Create a default queue 
	///
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&wdfIOQueueConfig,  WdfIoQueueDispatchSequential);//
	wdfIOQueueConfig.EvtIoRead = SDIOEvtIoRead;
    //wdfIOQueueConfig.EvtIoWrite = SDIOEvtIoWrite; //write is in Write queue
	wdfIOQueueConfig.EvtIoDeviceControl = SDIOEvtIoDeviceControl;
	wdfIOQueueConfig.EvtIoStop = DftEvtIoStop;
	//wdfIOQueueConfig.EvtIoCanceledOnQueue = EvtIoCanceledOnQueue;
	//.....

	status = WdfIoQueueCreate(pAdapter->WdfDevice, &wdfIOQueueConfig, WDF_NO_OBJECT_ATTRIBUTES,
                              &pAdapter->wdfDefaultQueue );	 
	if(!NT_SUCCESS(status))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Create default queue failed with status : 0x%p.\n",status)); 
		return status;
	}
 
	//write queue
	WDF_IO_QUEUE_CONFIG_INIT(&wdfIOQueueConfig,WdfIoQueueDispatchSequential); //must be sequential
	wdfIOQueueConfig.EvtIoWrite				= SDIOEvtIoWrite;
	wdfIOQueueConfig.EvtIoStop			    = EvtIoStopWrite;
	//wdfIOQueueConfig.EvtIoCanceledOnQueue	= EvtIoCanceledOnQueue;

	status = WdfIoQueueCreate(pAdapter->WdfDevice, &wdfIOQueueConfig, WDF_NO_OBJECT_ATTRIBUTES,
		                      &pAdapter->wdfWriteQueue);
	if(!NT_SUCCESS(status))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Create write queue failed with status : 0x%p\n",status)); 
		return status;
	}
	//The WdfDeviceConfigureRequestDispatching method 
	//causes the framework to queue a specified type of I/O requests to a specified I/O queue
	status  = WdfDeviceConfigureRequestDispatching(pAdapter->WdfDevice, 
		                                             pAdapter->wdfWriteQueue, 
													 WdfRequestTypeWrite);
	if(!NT_SUCCESS(status))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Configure Request Dispatching failed (0x%p)\n",status));
		return status;
	}
  
	return status;
}

INT SDIO_init(RTMP_ADAPTER *pAd)
{
	UINT32 Value = 0;
	UINT32 counter=0;

	RTMP_SDIO_READ32(pAd,WCIR, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WCIR = 0x%x\n",__FUNCTION__, Value));
	pAd->ChipID = Value;
	//Poll W_FUNC for FW own back
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Request FW-Own back\n",__FUNCTION__));

	//RTMP_SDIO_READ32(pAd, WHLPCR, &Value);
	
	while(!(0x100 & Value )) {
		
		RTMP_SDIO_WRITE32(pAd, WHLPCR, W_FW_OWN_REQ_CLR);
		KeStallExecutionProcessor(6);
		RTMP_SDIO_READ32(pAd, WHLPCR, &Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Request FW-Own processing: 0x%x\n",__FUNCTION__,Value));
		counter++;
		if(counter >50){
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  FW-Own back Faiure\n",__FUNCTION__));
			break;
		}
		if((0x100 & Value) == 1)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  Value bit8 = 1\n",__FUNCTION__));
		}
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  Value bit8 = 0\n",__FUNCTION__));
		
		KeStallExecutionProcessor(2);
	}	
	KeStallExecutionProcessor(20);
	RTMP_SDIO_WRITE32(pAd, WHLPCR, W_INT_EN_CLR);	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  dbg1\n",__FUNCTION__));
	RTMP_SDIO_WRITE32(pAd, WHIER, 0x47);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  dbg2\n",__FUNCTION__));
	//RTMP_SDIO_WRITE32(pAd, WHIER, (RX0_DONE_INT_EN |RX1_DONE_INT_EN));
	RTMP_SDIO_WRITE32(pAd, WHLPCR, W_INT_EN_SET);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  dbg3\n",__FUNCTION__));

			
	RTMP_SDIO_READ32(pAd, WHLPCR, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): MCR_WHLPCR: Value:%x\n", __FUNCTION__, Value));
		
//	RTMP_SDIO_WRITE32(pAd, WHIER, 0x46);
	RTMP_SDIO_READ32(pAd, WHIER, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): MCR_WHIER: Value:%x\n", __FUNCTION__, Value));
	RTMP_SDIO_READ32(pAd, WHISR, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): MCR_WHISR: Value:%x\n", __FUNCTION__, Value));
	RTMP_SDIO_READ32(pAd, WCIR, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): MCR_WCIR: Value:%x\n", __FUNCTION__, Value));

	RTMP_SDIO_READ32(pAd, WSDIOCSR, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WSDIOCSR: Value:%x\n", __FUNCTION__, Value));

//	RTMP_SDIO_WRITE32(pAd, WHIER, WHIER_DEFAULT);
	RTMP_SDIO_READ32(pAd, WHIER,&Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): MCR_WHIER: value:%x\n", __FUNCTION__,Value));
	RTMP_SDIO_READ32(pAd, WHISR,&Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): MCR_WHISR1: value:%x\n", __FUNCTION__,Value)); 
//	RTMP_SDIO_WRITE32(pAd, WHLPCR, W_INT_EN_SET);

	//RTMP_SDIO_WRITE32(pAd, WHIER, FW_OWN_BACK_INT_EN);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): <==\n", __FUNCTION__));
//#endif
	return 0;

}

// 1) low level access
NTSTATUS 
RTMP_SDIO_READ32(
    RTMP_ADAPTER *pAdapter,
    UINT32 u4Offset,
    PUINT32 pu4Value) 
{
    ULONG dwWordRead;
    PMDL prMdl= (PMDL)NULL;
    
    NTSTATUS  status = STATUS_SUCCESS;
 
//    ASSERT(pAdapter);

    prMdl = IoAllocateMdl(pu4Value, sizeof(UINT32), FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool(prMdl);

    status = SdioReadWriteBuffer(pAdapter->WdfDevice,
            pAdapter->FunctionNumber,
            prMdl,
            u4Offset,
            sizeof(UINT32),
            FALSE,
            FALSE,
            &dwWordRead);
 
    IoFreeMdl(prMdl);
 
	return status;
}

NTSTATUS 
RTMP_SDIO_WRITE32(
	RTMP_ADAPTER *pAdapter,
	UINT32 u4Offset,
	UINT32 u4Value) 
{
    ULONG dwWordWrite;
    PMDL prMdl= (PMDL)NULL;
    //PFDO_DATA fdoData;
    NTSTATUS  status = STATUS_SUCCESS;
    UINT32 Value = u4Value;
     
    //ASSERT(pAdapter);

    prMdl = IoAllocateMdl(&Value, sizeof(UINT32), FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool(prMdl);

    status = SdioReadWriteBuffer(pAdapter->WdfDevice,
            pAdapter->FunctionNumber,
            prMdl,
            u4Offset,
            sizeof(UINT32),
            FALSE,
            TRUE,
            &dwWordWrite);

    IoFreeMdl(prMdl);
 
	return status;
}

NTSTATUS 
MTSDIOProcessRxInterrupt(RTMP_ADAPTER *pAdapter){
	UINT32 Value = 0;
	UINT32 RX0PacketLen = 0, RX1PacketLen = 0;
	NTSTATUS  status = STATUS_SUCCESS;

	ASSERT(pAdapter);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s(): ==>\n", __FUNCTION__);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s(): ==>\n", __FUNCTION__));
	//RTMPusecDelay(500);
	RTMP_SDIO_READ32(pAdapter, WRPLR, &Value);
	RX0PacketLen = GET_RX0_PACKET_LENGTH(Value);
	RX1PacketLen = GET_RX1_PACKET_LENGTH(Value);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, " Rx 0 len = %d\n", RX0PacketLen);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, " Rx 1 len = %d\n", RX1PacketLen);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, RX@@ Rx 0 Len = %d\n", __FUNCTION__, RX0PacketLen));
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, RX@@ Rx 1 Len = %d\n", __FUNCTION__, RX1PacketLen));
	
	if (RX0PacketLen != 0) {
		status = MTSDIOSingleProcessRx0(pAdapter, RX0PacketLen);
	} 
	if (RX1PacketLen != 0) {
		status = MTSDIOSingleProcessRx1(pAdapter, RX1PacketLen);
	}
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s(): <==\n", __FUNCTION__);
	return status;

}

NTSTATUS MTSDIOSingleProcessRx0(RTMP_ADAPTER *pAdapter, INT32 Length){
	NTSTATUS  status = STATUS_SUCCESS;
	//UINT32 i;
	//PUCHAR buf;
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s(): ==>\n", __FUNCTION__);

	ReadRxIdx(pAdapter, Length, WRDR0);
#if 0
	if (Length >= pAd->BlockSize)
		Length = (Length + ((-Length) & (pAd->BlockSize - 1)));
	pAd->SDIORxPacket = RTMP_AllocateFragPacketBuffer(pAd, Length);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: =============>\n", __FUNCTION__));

	if (!pAd->SDIORxPacket)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: allocate net packet fail\n", 
														__FUNCTION__));

		return;
	}
	
	MTSDIODataRx0(pAd, GET_OS_PKT_DATAPTR(pAd->SDIORxPacket), Length);


	SET_OS_PKT_LEN(pAd->SDIORxPacket, Length);
	buf = GET_OS_PKT_DATAPTR(pAd->SDIORxPacket);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ================len = %x ===\n", __FUNCTION__, Length));
	for(i=0;i<Length;i++){
		printk("%x,",*(buf+i));
	}
	printk("\n");

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | 
									fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMPFreeNdisPacket(pAd, pAd->SDIORxPacket);
		return;
	}

	rtmp_rx_done_handle(pAd);
#endif
//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "%s(): <==\n", __FUNCTION__);
	return status;
}
	

NTSTATUS MTSDIOSingleProcessRx1(RTMP_ADAPTER *pAdapter, INT32 Length){
	NTSTATUS  status = STATUS_SUCCESS;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ==>\n", __FUNCTION__));

	ReadRxIdx(pAdapter, Length, WRDR1);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): <==\n", __FUNCTION__));

	return status;
}

NTSTATUS 
MTSDIOProcessAbnormalInterrupt(RTMP_ADAPTER *pAdapter){
	UINT32 WASRValue = 0;
	BOOLEAN Tx1Overflow= FALSE, Rx0Underflow = FALSE, Rx1Underflow = FALSE, FwOwnInvalidAccess = FALSE;
	NTSTATUS  status = STATUS_SUCCESS;

	ASSERT(pAdapter);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ==>\n", __FUNCTION__));

	RTMP_SDIO_READ32(pAdapter, WASR, &WASRValue);
	Tx1Overflow = (WASRValue & TX1_OVERFLOW) ? TRUE : FALSE;
	Rx0Underflow = (WASRValue & RX0_UNDERFLOW) ? TRUE : FALSE;
	Rx1Underflow = (WASRValue & RX1_UNDERFLOW) ? TRUE : FALSE;
	FwOwnInvalidAccess = (WASRValue & FW_OWN_INVALID_ACCESS) ? TRUE : FALSE;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Tx1Overflow: %d\n", __FUNCTION__, Tx1Overflow));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Rx0Underflow: %d\n", __FUNCTION__, Rx0Underflow));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Rx1Underflow: %d\n", __FUNCTION__, Rx1Underflow));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): FwOwnInvalidAccess: %d\n", __FUNCTION__, FwOwnInvalidAccess));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): <==\n", __FUNCTION__));
	return status;

}

NTSTATUS 
MTSDIOProcessTxInterrupt(RTMP_ADAPTER *pAdapter){
	NTSTATUS  status = STATUS_SUCCESS;
	UINT32 TxCount[8] = {0};
	UINT32 u4Cnt = 0;
	UINT32 idx = 0;

	ASSERT(pAdapter);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ==>\n", __FUNCTION__));
	//RTMP_SDIO_WRITE32(pAdapter, WHLPCR, W_INT_EN_CLR);
	//RTMP_SDIO_WRITE32(pAdapter, WHLPCR, W_INT_EN_SET);	

	RTMP_SDIO_READ32(pAdapter, WTQCR0, &TxCount[0]);
	RTMP_SDIO_READ32(pAdapter, WTQCR1, &TxCount[1]);
	RTMP_SDIO_READ32(pAdapter, WTQCR2, &TxCount[2]);
	RTMP_SDIO_READ32(pAdapter, WTQCR3, &TxCount[3]);
	RTMP_SDIO_READ32(pAdapter, WTQCR4, &TxCount[4]);
	RTMP_SDIO_READ32(pAdapter, WTQCR5, &TxCount[5]);
	RTMP_SDIO_READ32(pAdapter, WTQCR6, &TxCount[6]);
	RTMP_SDIO_READ32(pAdapter, WTQCR7, &TxCount[7]);
	if(TxCount[0]>0)
	{
		UINT32 TxPage = pAdapter->TxDataBufferLength/TX_PAGE_SIZE;
		if(pAdapter->TxDataBufferLength%TX_PAGE_SIZE != 0)
			TxPage++;
		pAdapter->TransmittedCount++;
		pAdapter->OtherCounters.Ac0TxedCount+= ((TxCount[0]&0xffff)/TxPage);
		//pAdapter->OtherCounters.Ac0TxedCount++;
	}
	//pAdapter->OtherCounters.Ac0TxedCount += TxCount[0];
	//MTSDIORead32(pAdapter, WHCR, &u4Cnt);
	RTMP_SDIO_READ32(pAdapter, WHCR, &u4Cnt);
	
	for(idx = 0; idx<8; idx++)
	{
/*	
		if(pAdapter->FWMode == FWRAM)
		{
			pAdapter->TxPageCount+=TxCount[idx]&0xffff;
			//pAdapter->TxPageCount+=TxCount[idx]>>16;
		}
*/		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, ==== TC[%d] %d, %d===\n", __FUNCTION__, idx, TxCount[idx]&0xffff,  TxCount[idx] >> 16));
	}

//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s: TxPageCount %d\n", __FUNCTION__, pAdapter->TxPageCount));
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s: <==\n", __FUNCTION__));
	return status;

}

   // interrupt status inquiry
NTSTATUS 
SDIO_GET_INTR(
    RTMP_ADAPTER *pAdapter, P_INTR_STATUS_T ret)
{

	UINT32 u4Value;
	NTSTATUS status = STATUS_SUCCESS;

	//ASSERT(prAdapter);

	status = RTMP_SDIO_READ32(pAdapter, WHISR, &u4Value);
	if (!NT_SUCCESS(status)) {
		return status;
	}
#if 0	
    (*ret).          = (UINT16) ((u4Value >> 16) & 0xffff);
    (*ret).ucFwInt             = (UINT8)  ((u4Value >> 9) & 0x7f);
    (*ret).bTxFifoOverflow     = u4Value & BIT(8) ? TRUE : FALSE;
    (*ret).bFwInt              = u4Value & BIT(7) ? TRUE : FALSE;
    (*ret).ucTxCompleteCount   = (UINT8)  ((u4Value >> 4) & 0x7);
    (*ret).bTxFifoUnderThold   = u4Value & BIT(3) ? TRUE : FALSE;
    (*ret).bTxEmpty            = u4Value & BIT(2) ? TRUE : FALSE;
    (*ret).bRxDoneInt          = u4Value & BIT(1) ? TRUE : FALSE;
    (*ret).bFwOwnBackInt       = u4Value & BIT(0) ? TRUE : FALSE;
#endif
    //(*ret).u4CHISR = u4Value;    

	ret->WHISRVal = u4Value;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("currentIntrStatus 0x%x\n", pAdapter->currentIntrStatus.WHISRVal));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("TxDoneInt 0x%x\n", pAdapter->currentIntrStatus.field.TxDoneInt ));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Rx0DoneInt 0x%x\n", pAdapter->currentIntrStatus.field.Rx0DoneInt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Rx1DoneInt 0x%x\n", pAdapter->currentIntrStatus.field.Rx1DoneInt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("AbnormalInt 0x%x\n", pAdapter->currentIntrStatus.field.AbnormalInt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("FwOwnBackInt 0x%x\n", pAdapter->currentIntrStatus.field.FwOwnBackInt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s()<<<\n",__FUNCTION__));
    //return ret;
	return status;
}

   NTSTATUS
SdioReadWriteBuffer_BlockMode_Async(
                   IN WDFDEVICE Device,
                   IN ULONG Function,
                   IN PMDL Mdl,
                   IN ULONG Address,
                   IN ULONG Length,
                   IN BOOLEAN FIFOMode,
                   IN BOOLEAN WriteToDevice,
                   OUT PULONG BytesRead,
				   IN BOOLEAN BlockMode
                   )
{
//    PFDO_DATA   fdoData;
	RTMP_ADAPTER *pAdapter;
    SDBUS_REQUEST_PACKET sdrp;
    SD_RW_EXTENDED_ARGUMENT extendedArgument;
    NTSTATUS                 status = STATUS_SUCCESS;
    const SDCMD_DESCRIPTOR ReadIoExtendedDesc =
    {SDCMD_IO_RW_EXTENDED, SDCC_STANDARD, SDTD_READ, SDTT_SINGLE_BLOCK, SDRT_5};

    const SDCMD_DESCRIPTOR WriteIoExtendedDesc =
    {SDCMD_IO_RW_EXTENDED, SDCC_STANDARD, SDTD_WRITE, SDTT_SINGLE_BLOCK, SDRT_5};

	PIO_STACK_LOCATION      stack;	
	PIO_COMPLETION_ROUTINE CompletionRoutine;
	KIRQL Irql;
	LARGE_INTEGER       TimeOut1Second;	
	TimeOut1Second.QuadPart = 0;

    PAGED_CODE();
    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "enter SdioReadWriteBuffer()\n");  //to construct CMD53

    //fdoData = 
    pAdapter = SDIOpAdGetData(Device);

    	//stack = IoGetNextIrpStackLocation(pAdapter->pSDIrp);
	//stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;	
	//CompletionRoutine = SdioReadWriteBuffer_BlockMode_Async_Complete;
	//IoSetCompletionRoutine(pAdapter->pSDIrp,
                  //         CompletionRoutine,//UsbDoBulkOutTransfer0Complete,
                      //     pAdapter,
                          // TRUE,           // call on success
                           //TRUE,           // call on error
                           //TRUE);          // call on cancel
    RtlZeroMemory(&sdrp, sizeof(SDBUS_REQUEST_PACKET));

    sdrp.RequestFunction = SDRF_DEVICE_COMMAND;
    sdrp.Parameters.DeviceCommand.Mdl = Mdl;

    extendedArgument.u.AsULONG = 0;
    extendedArgument.u.bits.Function = Function;
    if(FIFOMode == FALSE)
        extendedArgument.u.bits.OpCode = 1;             // increment address
    else
        extendedArgument.u.bits.OpCode = 0;             // FIFO read

    extendedArgument.u.bits.BlockMode = BlockMode;
    extendedArgument.u.bits.Address = Address;

    if (WriteToDevice) {
        extendedArgument.u.bits.WriteToDevice = 1;
        sdrp.Parameters.DeviceCommand.CmdDesc  = WriteIoExtendedDesc;
    } else {
        sdrp.Parameters.DeviceCommand.CmdDesc  = ReadIoExtendedDesc;
    }

    //if (pAdapter->firmwareinfo.BlockMode == 1) {
    if(BlockMode){
        sdrp.Parameters.DeviceCommand.CmdDesc.TransferType = SDTT_MULTI_BLOCK_NO_CMD12;
    }
	else
	{
		sdrp.Parameters.DeviceCommand.CmdDesc.TransferType = SDTT_SINGLE_BLOCK;
	}

    sdrp.Parameters.DeviceCommand.Argument = extendedArgument.u.AsULONG;
    sdrp.Parameters.DeviceCommand.Length   = Length;
    /*
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Function = 0x%x\n", Function);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "FIFOMode = 0x%x\n", FIFOMode);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Address = 0x%x\n", Address);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "WriteToDevice = 0x%x\n", WriteToDevice);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "BlockMode = 0x%x\n", BlockMode);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Argument = 0x%x\n", extendedArgument.u.AsULONG);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Length = 0x%x\n", Length);
*/	
	KeAcquireSpinLock(&pAdapter->TxLock, &Irql);	
	if(pAdapter->TxSDRunning == TRUE)
	{
		KeReleaseSpinLock(&pAdapter->TxLock, Irql);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("[TxQQ] %s,  TxSD running return\n", __FUNCTION__));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	pAdapter->TxSDRunning = TRUE;
	KeReleaseSpinLock(&pAdapter->TxLock, Irql);
		
		
		

	//
	// Set the IRP
	//
	pAdapter->pSDIrp  = IoAllocateIrp(pAdapter->NextDeviceObject->StackSize + 1, FALSE);

    	IoSetCompletionRoutine(pAdapter->pSDIrp ,
                           SdioReadWriteBuffer_BlockMode_Async_Complete,
                           pAdapter,
                           TRUE,           // call on success
                           TRUE,           // call on error
                           TRUE);          // call on cancel

    	
    //
    // Send the IO request down to the bus driver
    //

    status = SdBusSubmitRequestAsync (pAdapter->BusInterface.Context, &sdrp, pAdapter->pSDIrp, SdioReadWriteBuffer_BlockMode_Async_Complete, pAdapter);
	if(STATUS_PENDING == status)
	{	
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("return STATUS_PENDING\n"));
	}
	else if(STATUS_SUCCESS == status)
	{
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("return STATUS_SUCCESS\n"));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("return error 0x%x\n", status));
	}
	//Event_Wait(pAdapter, &pAdapter->LoadFWEvent,TimeOut1Second);
    *BytesRead = (ULONG)sdrp.Information;
      RTMPusecDelay(2000);// 1 ms
      MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("[TxQQ] %s <--\n", __FUNCTION__));
    return status;
}

NTSTATUS
SdioReadWriteBuffer_BlockMode_Async_Complete(
                   IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context
                   )
{

	RTMP_ADAPTER *pAdapter = (RTMP_ADAPTER *)Context;

	NTSTATUS	status = STATUS_SUCCESS;
	KIRQL		Irql;
	
	KeAcquireSpinLock(&pAdapter->TxLock, &Irql);	
	pAdapter->TxSDRunning = FALSE;
	KeReleaseSpinLock(&pAdapter->TxLock, Irql);
	if (pIrp->IoStatus.Status == STATUS_CANCELLED)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("sdsubmit Canceled\n"));	
	}
	else if (pIrp->IoStatus.Status == STATUS_SUCCESS)
      		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("sdsubmit SUCCESS, TxProcessing %d\n", pAdapter->TxProcessing));
	if(pAdapter->bLoadingFW)
		KeSetEvent(&pAdapter->LoadFWEvent, 0, FALSE);
	if(pAdapter->TxProcessing)
	{
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, set TxDataEvent\n", __FUNCTION__));
		KeSetEvent(&pAdapter->TxDataEvent, 0, FALSE);
	}
	IoFreeIrp(pAdapter->pSDIrp);
	pAdapter->pSDIrp = NULL;
	if(pAdapter->LoopBackWaitRx)
		KeSetEvent(&pAdapter->LoopBackSDIOTxEvent, 0, FALSE);
	//pAdapter->bIsWaitFW = FALSE;
	return STATUS_MORE_PROCESSING_REQUIRED;
}
NTSTATUS SDIOGetChipID(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	UINT32 		Value = 0;
	ntStatus = RTMP_SDIO_READ32(pAd,WCIR, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s(): WCIR = 0x%x\n",__FUNCTION__, Value));	
	pAd->ChipID = Value & 0x0000ffff;
	return ntStatus;
}

NTSTATUS SDIOInitAdapter(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	pAd->TxSDRunning = FALSE;
	pAd->TxPageCount = TX_PAGE_NUM;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s(): TxPageCount = %d\n",__FUNCTION__, pAd->TxPageCount));	
	return ntStatus;
}

NTSTATUS SDIOInitInterface(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;	

	return ntStatus;
}

NTSTATUS SDIOSetupTxPacket(RTMP_ADAPTER *pAd, UCHAR *pData, UINT32 Length, UINT32 BufferNum)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;

	if(Length >BUFFER_SIZE)
		Length = BUFFER_SIZE -4;
	if(BufferNum & 0x0002)
	{
		pAd->TxCmdBufferLength  = Length +4;
		RtlZeroMemory(pAd->TxCmdBuffer,  BUFFER_SIZE);
		RtlMoveMemory(pAd->TxCmdBuffer, pData, Length);
		//pAd->bMinLen = 44;
		//pAd->bCurrlength= pAd->bMinLen-1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("TxCmdBufferLength = %d",pAd->TxCmdBufferLength));

	}
	else
	{
		pAd->TxDataBufferLength  = Length +4;
		RtlZeroMemory(pAd->TxDataBuffer,  BUFFER_SIZE);
		RtlMoveMemory(pAd->TxDataBuffer, pData, Length);
		//pAd->bMinLen = 44;
		//pAd->bCurrlength= pAd->bMinLen-1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("TxDataBufferLength = %d",pAd->TxDataBufferLength));
	}

	return ntStatus;
}

NTSTATUS SDIOStartTxPacket(RTMP_ADAPTER *pAd, UINT32 txBufferNum, UINT32 TxRemained)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;

	if(txBufferNum & 0x0002)
	{
		ntStatus = FWTxCmd(pAd, &pAd->TxCmdBuffer, pAd->TxCmdBufferLength, NULL);
		if(ntStatus != STATUS_SUCCESS){
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, (" sent Fw cmd failed\n"));		
		}
	}
	else
	{	
		pAd->TxRemained[TX_DATA_BUFEER_NUM] = TxRemained;	
		if(pAd->TxSDRunning)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN,("%s Tx is under process\n",__FUNCTION__));
			ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s TxRemained = %d\n",__FUNCTION__, pAd->TxRemained[TX_DATA_BUFEER_NUM]));
			
			RtlZeroMemory(&pAd->TransmittedCount, sizeof(UINT32));				
			ntStatus = SDIO_TxData(pAd);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s TransmittedCount = %d\n",__FUNCTION__, pAd->TransmittedCount));
		}
	}

	return ntStatus;
}
NTSTATUS SDIOStopTxPacket(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	pAd->TxRemained[TX_DATA_BUFEER_NUM] = 0;
	pAd->TxProcessing= FALSE;
	return ntStatus;
}

NTSTATUS SDIOCheckFWROMWiFiSysOn(IN  RTMP_ADAPTER   *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	UINT32 Value =0;
	UINT32 counter = 0;

	RTMP_SDIO_READ32(pAd, WCIR, &Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WCIR: 0x%x Value & W_FUNC_RDY = 0x%x\n",__FUNCTION__,Value, Value & W_FUNC_RDY));
	
	while(Value & W_FUNC_RDY) {		
		RTMP_SDIO_READ32(pAd, WCIR, &Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WCIR: 0x%x\n",__FUNCTION__,Value));
		counter++;
		if(counter >100){
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("W_FUNC_RDY != .\n"));
			return STATUS_UNSUCCESSFUL;
			break;
		}
			
		RTMPusecDelay(30000);// 30ms			
	}
	counter = 0;
	while(!(0x100 & Value )) {
		
		RTMP_SDIO_WRITE32(pAd, WHLPCR, W_FW_OWN_REQ_CLR);

		RTMP_SDIO_READ32(pAd, WHLPCR, &Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Request FW-Own processing: 0x%x\n",__FUNCTION__,Value));
		counter++;
		if(counter >50){
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  FW-Own back Faiure\n",__FUNCTION__));
			ntStatus = STATUS_UNSUCCESSFUL;
			break;
		}		
		KeStallExecutionProcessor(2);
	}	

	return ntStatus;
}
void SDIOLoopBack_Run(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting, UINT32 length)
{
	UINT32 count = 0;
	TX_WI_STRUC pTxWI;
	PUCHAR ptr;
	UINT8 alignment = 0;
	LARGE_INTEGER       TimeOut1Second;		
	TimeOut1Second.QuadPart = 0;
#if 1
	//length = 1024;
	if(pSetting->IsDefaultPattern)
	{		
		RtlZeroMemory(&pAd->TxDataBuffer, BUFFER_SIZE);
		RtlZeroMemory(&pTxWI, sizeof(TX_WI_STRUC));
		ptr = &pAd->TxDataBuffer[sizeof(TX_WI_STRUC)];	
		for(count = 0; count<length - sizeof(TX_WI_STRUC); count++)
		{
			ptr[count] = count%16;	
		}
		//set pkt content
		pTxWI.P_IDX= 1;
		pTxWI.Q_IDX= 0;		
		pTxWI.TXByteCount =  length;
		pTxWI.FT = 1;
		pTxWI.HF = 1;
		RtlCopyMemory(&pAd->TxDataBuffer, &pTxWI, sizeof(TX_WI_STRUC));		
		RtlCopyMemory(&pAd->LoopBackTxRaw, &pAd->TxDataBuffer, LOOPBACK_SIZE);
		pAd->LoopBackTxRawLen = length;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, length = %d\n",__FUNCTION__, length));		
		if(length > 32)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("===buffer===\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x 0x%x 0x%x 0x%x\n", pAd->TxDataBuffer[3], pAd->TxDataBuffer[2], pAd->TxDataBuffer[1], pAd->TxDataBuffer[0]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x 0x%x 0x%x 0x%x\n", pAd->TxDataBuffer[7], pAd->TxDataBuffer[6], pAd->TxDataBuffer[5], pAd->TxDataBuffer[4]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x 0x%x 0x%x 0x%x\n", pAd->TxDataBuffer[11], pAd->TxDataBuffer[10], pAd->TxDataBuffer[9], pAd->TxDataBuffer[8]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x 0x%x 0x%x 0x%x\n", pAd->TxDataBuffer[15], pAd->TxDataBuffer[14], pAd->TxDataBuffer[13], pAd->TxDataBuffer[12]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x 0x%x 0x%x 0x%x\n", pAd->TxDataBuffer[19], pAd->TxDataBuffer[18], pAd->TxDataBuffer[17], pAd->TxDataBuffer[16]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x 0x%x 0x%x 0x%x\n", pAd->TxDataBuffer[23], pAd->TxDataBuffer[22], pAd->TxDataBuffer[21], pAd->TxDataBuffer[20]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x 0x%x 0x%x 0x%x\n", pAd->TxDataBuffer[27], pAd->TxDataBuffer[26], pAd->TxDataBuffer[25], pAd->TxDataBuffer[24]));
		}
		if(length % 4 != 0)
			alignment = 4 - (length % 4);

		pAd->TxDataBufferLength = length + alignment + 4 ;
		
	}

	else
	{
		RtlCopyMemory(&pAd->TxDataBuffer, &pAd->LoopBackExpectTx, LOOPBACK_SIZE);
		RtlZeroMemory(&pAd->TxDataBuffer + length , LOOPBACK_SIZE - length); 
		RtlCopyMemory(&pAd->LoopBackTxRaw, &pAd->TxDataBuffer, length);
		pAd->LoopBackTxRawLen = pAd->LoopBackExpectTxLen;
		//pAd->LoopBackExpectRxLen = length;
	}
	
#endif	
		
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LP length = %d, alignment = %d\n",__FUNCTION__, length, alignment));
		
	pAd->LoopBackResult.TxByteCount += length;//pAd->TxBufferLength[pipenum] - 4;
	pAd->LoopBackResult.TxPktCount ++;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, RxPktCount = %d\n",__FUNCTION__, pAd->LoopBackResult.RxPktCount));
	StartTx(pAd, 1, 0);	
	if(STATUS_SUCCESS != Event_Wait(pAd, &pAd->LoopBackSDIOTxEvent,TimeOut1Second))
	{		
		LoopBack_Fail(pAd, TX_TIMEOUT);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### TX_TIMEOUT ####################3\n",__FUNCTION__));
	}

}
