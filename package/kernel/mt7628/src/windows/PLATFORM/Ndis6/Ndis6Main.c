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
    rtmp_main.c

    Abstract:
    Miniport main initialization routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    2002-08-01    created
    John Chang  2004-09-01    modified for rt2561/2661
*/
#include    "MtConfig.h"

//
//  Global static variable
//  Debug level flag
ULONG   RTDebugLevel = RT_DEBUG_TRACE; //RT_DEBUG_TRACE;

NDIS_HANDLE     G_NdisMiniportDriverHandle = NULL;
NDIS_HANDLE     G_MiniportDriverContext = NULL;

NDIS_HANDLE         G_NdisWrapperHandle = NULL;
PMP_ADAPTER       G_pAdapter  = NULL;
ULONG               G_NdisVersion = 0;

extern ULONG    G_MemoryAllocate_cnt;
extern ULONG    G_MemoryFree_cnt;
extern ULONG    G_MemoryAllocate_Len;
extern ULONG    G_MemoryFree_Len;

/*
    ========================================================================

    Routine Description:
        Driver's Entry point

    Arguments:
        pDriverObject    -   pointer to the driver object
        pRegistryPath    -   pointer to the driver registry path

    Return Value:
        NDIS_STATUS - the value returned by NdisMRegisterMiniport

    IRQL = PASSIVE_LEVEL

    Note:

    ========================================================================
*/
NDIS_STATUS DriverEntry(
    IN  PDRIVER_OBJECT      pDriverObject,
    IN  PUNICODE_STRING     pRegistryPath
    )
{
    NDIS_STATUS                             ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_DRIVER_CHARACTERISTICS    MPChar;
//  ULONG                                   ndisVersion;
    WDF_DRIVER_CONFIG                       config;
    NTSTATUS                                ntStatus;
    WDFDRIVER                               hDriver; //vm control
    
    DBGPRINT(RT_DEBUG_TRACE,("================================================\n"));
    DBGPRINT(RT_DEBUG_TRACE,("Build Date: "__DATE__" Time: "__TIME__" txwi %u\n", sizeof(TXWI_STRUC) - sizeof(ULONG)));
    DBGPRINT(RT_DEBUG_TRACE,("================================================\n"));

      WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
    //
    // Set WdfDriverInitNoDispatchOverride flag to tell the framework
    // not to provide dispatch routines for the driver. In other words,
    // the framework must not intercept IRPs that the I/O manager has
    // directed to the driver. In this case, it will be handled by NDIS
    // port driver.
    //
    config.DriverInitFlags |= WdfDriverInitNoDispatchOverride;

    ntStatus = WdfDriverCreate(pDriverObject,
                               pRegistryPath,
                               WDF_NO_OBJECT_ATTRIBUTES,
                               &config,                
                               &hDriver); //vm control
    if(!NT_SUCCESS(ntStatus)){
        DBGPRINT(RT_DEBUG_ERROR, ("WdfDriverCreate failed\n"));
        return NDIS_STATUS_FAILURE;
    }
    //
    // Make sure we are compatible with the version of NDIS supported by OS.
    //
    G_NdisVersion = NdisGetVersion();
    DBGPRINT(RT_DEBUG_TRACE,("ndisVersion =%x \n", G_NdisVersion));

    //
    // Fill in the Miniport characteristics structure with the version numbers
    // and the entry points for driver-supplied MiniportXxx
    //
    PlatformZeroMemory(&MPChar, sizeof(MPChar));

    if (G_NdisVersion <= 0x00060001)
    {
        // NDIS Version 6.1 (Vista)
        MPChar.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS,
        MPChar.Header.Size = sizeof(NDIS_MINIPORT_DRIVER_CHARACTERISTICS);
        MPChar.Header.Revision = NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_1;
        MPChar.MajorNdisVersion = RTMP_NDIS_MAJOR_VERSION;
        MPChar.MinorNdisVersion = 0;
    }
    else
    {
        // NDIS Version 6.2(win7), 6.3 (win8)
        MPChar.Header.Type      = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS;
        MPChar.Header.Size      = NDIS_SIZEOF_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
        MPChar.Header.Revision  = NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
        MPChar.MajorNdisVersion = RTMP_NDIS_MAJOR_VERSION;
#ifdef WIN8_RALINK_WFD_SUPPORT      
        // !!! We should report the current G_NdisVersion to NDIS.
        // !!! But report Ndis620 for successful virtual port creation.
        MPChar.MinorNdisVersion = 20;
#else
        MPChar.MinorNdisVersion = (G_NdisVersion & 0xff);
#endif /* WIN8_RALINK_WFD_SUPPORT */
    }
    MPChar.MajorDriverVersion           = NIC_MAJOR_DRIVER_VERSION;
    MPChar.MinorDriverVersion           = NIC_MINOR_DRIVER_VERSION;

    //
    // Init/PnP handlers
    //
    MPChar.InitializeHandlerEx      = N6Initialize;
    MPChar.RestartHandler           = N6Restart;
    MPChar.PauseHandler             = N6Pause;

    MPChar.ShutdownHandlerEx        = N6ShutdownHandler; 
    MPChar.DevicePnPEventNotifyHandler  =  N6DevicePnPEvent;
    MPChar.HaltHandlerEx            = N6Halt;
    MPChar.UnloadHandler            = N6Unload;

    //
    // Query/Set/Method requests handlers
    //
    MPChar.OidRequestHandler        = N6OIDRequest; 
    MPChar.CancelOidRequestHandler  = N6CancelOIDRequest;

    //
    // Set optional miniport services handler
    //
    MPChar.SetOptionsHandler        = N6SetOptions;

    //
    // Send/Receive handlers
    //
    MPChar.SendNetBufferListsHandler    = N6XmitSendNetBufferLists;
    MPChar.CancelSendHandler            = N6CancelSendNetBufferLists;
    MPChar.ReturnNetBufferListsHandler  = N6ReturnNetBufferLists;

    //
    // Fault handling handlers
    //
    MPChar.CheckForHangHandlerEx        = N6CheckForHang;
    MPChar.ResetHandlerEx               = N6Reset;
    MPChar.Flags = NDIS_WDM_DRIVER;      

    //
    // Direct OID request handlers
    //
    MPChar.DirectOidRequestHandler      = N6DirectOidRequest;
    MPChar.CancelDirectOidRequestHandler = N6CancelDirectOidRequest;


    //
    // Registers miniport's entry points with the NDIS library as the first
    // step in NIC driver initialization. The NDIS will call the
    // MiniportInitialize when the device is actually started by the PNP
    // manager.
    //
    ndisStatus = NdisMRegisterMiniportDriver(pDriverObject,
                                         pRegistryPath,
                                         (PNDIS_HANDLE)G_MiniportDriverContext,
                                         &MPChar,
                                         &G_NdisMiniportDriverHandle);
    
    G_NdisWrapperHandle = G_NdisMiniportDriverHandle;

    if (ndisStatus != NDIS_STATUS_SUCCESS) 
    {   
        DBGPRINT(RT_DEBUG_ERROR, ("NdisMRegisterMiniportDriver Status = 0x%08x\n", ndisStatus));
    }

    DBGPRINT(RT_DEBUG_TRACE,("<== DriverEntry, Status=0x%08x\n", ndisStatus));

    return ndisStatus;
}

NDIS_STATUS
N6Initialize(
    IN  NDIS_HANDLE                        MiniportAdapterHandle,
    IN  NDIS_HANDLE                        MiniportDriverContext,
    IN  PNDIS_MINIPORT_INIT_PARAMETERS     MiniportInitParameters
    )
{
    NDIS_STATUS     NdisStatus = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER   pAd = NULL;
    ULONG           SafelyRemoveAttribute = 0;
    BOOLEAN         bInitMLME = FALSE;
    BOOLEAN         bInitTxRx = FALSE;
    BOOLEAN         bAllocAdapter = FALSE;
    PMP_PORT      pPort = NULL;

    //INT i;
    ADAPTER_INITIAL_STAGE   InitialStage = {0};

    UNREFERENCED_PARAMETER(MiniportDriverContext);
    UNREFERENCED_PARAMETER(MiniportInitParameters);

    DBGPRINT(RT_DEBUG_TRACE, ("==> %s\n", __FUNCTION__));

    do 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Ndis6CommonAllocAdapterBlock Sizeof[ = %d]\n", sizeof(MP_ADAPTER)));

        // Allocate MP_ADAPTER miniport adapter structure
        NdisStatus = Ndis6CommonAllocAdapterBlock(MiniportAdapterHandle, &pAd);
        if ((NdisStatus != NDIS_STATUS_SUCCESS) || (pAd == NULL))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Ndis6CommonAllocAdapterBlock failed, ndisStatus[=0x%08x]\n", NdisStatus));
            break;
        }

        G_pAdapter = pAd;

        pAd->AdapterHandle = MiniportAdapterHandle;     

        pAd->pNicCfg->NdisVersion = G_NdisVersion;
        InitialStage.bAllocAdapter = TRUE;

 


        NdisStatus = N6IniMiniportAttributes (pAd);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("NdisMSetMiniportAttributes failed, ndisStatus[=0x%08x]\n", NdisStatus));
            break;
        }

        NdisStatus = N6InterfaceStart(pAd);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6InterfaceStart failed, Status[=0x%08x]\n", NdisStatus));
            break;
        }               
            
        //
        // Since 6x9x, we need to swap pipes for 6x9x,
        // get the MAC version as earlier as possible.
        //      
        pAd->HwCfg.MACVersion = 0;

        GET_ASIC_VERSION_ID(pAd);
        
        NdisCommonInitializeSWCtrl(pAd, &InitialStage);

        InitializeHWCtrl(pAd, &InitialStage);
        
         NdisStatus = NdisInitNICReadRegParameters(pAd);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT_ERR(("NdisInitNICReadRegParameters failed, Status[=0x%08x]\n", NdisStatus));
            break;
        }   

 #if 1 // InitializeHWCtrl
       NdisStatus = NdisInitAdapterBlock(pAd, &InitialStage);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT_ERR(("NdisInitAdapterBlock failed, ndisStatus[=0x%08x]\n", NdisStatus));
            break;
        }       
 #endif

        NdisStatus = MlmeInit(pAd);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT_ERR(("MlmeInit failed, Status[=0x%08x]\n", NdisStatus));       
            break;
        }
        
        InitialStage.bInitMLME = TRUE;
        
        // Start to Load FW
        DBGPRINT(RT_DEBUG_TRACE,("Start to load firmware!!\n"));

#if 1
     // Before Load firmware, need to start N6UsbXmitStart & N6UsbRecvStart first.
        NdisCommonStartTxRx(pAd, &InitialStage);
        
#endif

        NdisStatus = NICLoadFirmware(pAd);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT_ERR(("NICLoadFirmware failed, ndisStatus[=0x%08x]\n", NdisStatus));
            break;
        }

        InitialStage.bLoadFw = TRUE;

        NICReadEEPROMParameters(pAd); 


         //
        // Fill the Miniport 802.11 Attributes, we can do so as soon as NIC is initialized.
        //
#if 1
        NdisStatus = N6Set80211Attributes(pAd); //MPSet80211Attributes
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s   Line(%d)Failed to set 80211 attributes\n", __LINE__, __FUNCTION__));
             break;
            //return  NdisStatus;
        }
#endif
        NdisStatus = N6SetGeneralAttributes(pAd); //MPSetGeneralAttributes
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Failed to set general attributes\n"));
             break;
            //return  NdisStatus;
        }   

        // Init mac setting
        mt_mac_init(pAd);

        // Init DMA Scheduler
        MtAsicDMASchedulerInit(pAd, DMA_SCH_HYBRID);

        // Init WLan Table
        mt_hw_tb_init(pAd, FALSE);

        InitializeProtocolCtrl(pAd);      

        pPort = pAd->PortList[FXXK_PORT_0];

        pPort->CommonCfg.InitPhyMode = pPort->CommonCfg.PhyMode;
        pPort->CommonCfg.LastPhyMode = pPort->CommonCfg.PhyMode;

        PlatformIndicateCurrentPhyPowerState(pAd,0, DOT11_PHY_ID_ANY);
        

        pAd->StaCfg.bRadio = pAd->StaCfg.bSwRadio && pAd->StaCfg.bHwRadio;

        
        // when initial state driver should be BW_20
        pPort->BBPCurrentBW = BW_20;
        
  
        //Initialize phy mode for softap
        pPort->SoftAP.ApCfg.PhyMode = pPort->CommonCfg.PhyMode;
        PlatformMoveMemory(&pPort->SoftAP.ApCfg.AddHTInfoIe, &pPort->CommonCfg.AddHTInfo, sizeof(ADD_HT_INFO_IE));
        PlatformMoveMemory(&pPort->SoftAP.ApCfg.HtCapability, &pPort->CommonCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
        PlatformMoveMemory(&pPort->SoftAP.ApCfg.TxPhyCfg, &pPort->CommonCfg.TxPhyCfg, sizeof(PHY_CFG));

        // SoftAp not support Tx Beamforming
        PlatformZeroMemory(&pPort->SoftAP.ApCfg.HtCapability.TxBFCap, sizeof(HT_BF_CAP));
        
                    
        pAd->AdapterState = NicPaused;
        MP_SET_STATE(pAd, INIT_STATE);
        MP_SET_STATE(pPort, INIT_STATE);

    } while (FALSE);

    if (pAd && (NdisStatus != NDIS_STATUS_SUCCESS))
    {
        if (InitialStage.bInitMLME)
            MlmeHalt(pAd, pAd->PortList[NDIS_DEFAULT_PORT_NUMBER]);

        if (InitialStage.bEnableXmit)
        {
             NdisCommonStopTx(pAd);
        }

        if (InitialStage.bEnableRecv)
        {
            NdisCommonStopRx(pAd);
        }

        if (InitialStage.bInitRecv)
        {
            NdisCommonFreeNicRecv(pAd);
        }

        if (InitialStage.bInitXmit)
        {
            NdisCommonFreeNICTransmit(pAd);
        }

        if (InitialStage.bAllocSpinLock)
        {
            NdisCommonDeallocateAllSpinLock(pAd);
        }
        
        NdisCommonFreeAllThreads(pAd);
        NdisCommonFreeAllWorkitems(pAd);
        
        ReleaseAdapterInternalBlock(pAd);

        if (InitialStage.bAllocAdapter)
        {
            N6FreePortMemory(pAd);
            
            PlatformFreeMemory(pAd, sizeof(MP_ADAPTER));
        }
    }
    else if (pAd)
    {
        PMP_PORT        pPort = pAd->PortList[FXXK_PORT_0];
        // Set periodic timer here.
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        //LARGE_INTEGER   dueTime;

        //dueTime = RtlConvertLongToLargeInteger((-1)*MLME_TASK_EXEC_INTV); 
        //KeSetCoalescableTimer(&pPort->Mlme.PeriodicTimer, dueTime, MLME_TASK_EXEC_INTV, 10, &pPort->Mlme.MlmeDpc);
        
#endif
        PlatformSetTimer(pPort, &pPort->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

        if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
        {
            //DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkReceive!\n" ));           
            
            //
            // Now Enable RxTx
            //
//**            RTMPEnableRxTx(pAd);

            // Initialize RF register to default value
            //AsicSwitchChannel(pAd, pPort->Channel,FALSE);


            // switch to defaul channel 1 and BW 20 for switch channel
//**            SwitchBandwidth(pAd, TRUE, pPort->Channel, 0, 0);

            //
            // Perform MT76xx power up Calibration secquence
            //
//**            PowerOnFullCalibration(pAd);

            //N6UsbXmitStart(pAd);
        }


#ifdef MT7603_FPGA // Its' for MT7603 FPGA Test

        DBGPRINT(RT_DEBUG_TRACE, ("FPGA : AutoBA = %d, Policy = %d\n", pPort->CommonCfg.BACapability.field.AutoBA, pPort->CommonCfg.BACapability.field.Policy));
        //pPort->CommonCfg.BACapability.field.AutoBA = 1;         // enable/disable BA
        //pPort->CommonCfg.BACapability.field.Policy = IMMED_BA;  // IMMED_BA(1)/DLEAY_BA(1)
        //pPort->CommonCfg.BACapability.word = 0;
        //pPort->CommonCfg.BACapability.field.Policy = BA_NOTUSE;

        // Init MT7603 CRs
        MT7603InitMAC(pAd);

        // Init WLan Table
        WTBLInit(pAd);

        // Set MAC address
        {
            UCHAR tempAddress[MAC_ADDR_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
            
            MtAsicSetDevMac(pAd, pAd->HwCfg.CurrentAddress);
            MtAsicSetBssid(pAd, tempAddress);
        }

        // Set BW
        MtAsicSetBW(pAd, BW_20);

        // Set Tx Stream
        MtAsicSetTxStream(pAd, OPMODE_STA, FALSE);

        // Set Rx Stream
        MtAsicSetRxStream(pAd, 2);

        // Enable Tx/Rx
        MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

        // Build channel
        pPort->CommonCfg.PhyMode = PHY_11BGN_MIXED;
        pAd->HwCfg.ChannelMode = ChMode_24G;
        //BuildChannelList(pAd);
//      DBGPRINT(RT_DEBUG_TRACE,("pAd->HwCfg.ChannelListNum = %d\n", pAd->HwCfg.ChannelListNum));
        pAd->HwCfg.ChannelListNum = 11;
        pAd->HwCfg.ChannelList[0].Channel = 1; // for FPGA connection
#if 1
        pAd->HwCfg.ChannelList[1].Channel = 2; // for FPGA connection
        pAd->HwCfg.ChannelList[2].Channel = 3; // for FPGA connection
        pAd->HwCfg.ChannelList[3].Channel = 4; // for FPGA connection
        pAd->HwCfg.ChannelList[4].Channel = 5; // for FPGA connection
        pAd->HwCfg.ChannelList[5].Channel = 6; // for FPGA connection
        pAd->HwCfg.ChannelList[6].Channel = 7; // for FPGA connection
        pAd->HwCfg.ChannelList[7].Channel = 8; // for FPGA connection
        pAd->HwCfg.ChannelList[8].Channel = 9; // for FPGA connection
        pAd->HwCfg.ChannelList[9].Channel = 10; // for FPGA connection
        pAd->HwCfg.ChannelList[10].Channel = 11; // for FPGA connection
#endif
#if 1  // set Guard time
        {
            ULONG MACValue;
            
            MACValue = 0x40404040;
            HW_IO_WRITE32(pAd, ARB_GTQR0, MACValue);

            MACValue = 0x00004000;
            HW_IO_WRITE32(pAd, ARB_GTQR2, MACValue);
        }

#endif

#if 1 // UDMA Rx Aggregation
        {
            U3DMA_WLCFG         U3DMAWLCFG;
            // Set DMA scheduler to Bypass mode.
            // Enable Tx/Rx
            HW_IO_READ32(pAd, UDMA_WLCFG_0, &U3DMAWLCFG.word);
            DBGPRINT(RT_DEBUG_ERROR,("1 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
            U3DMAWLCFG.field.WL_RX_AGG_EN = 1;
            U3DMAWLCFG.field.WL_RX_AGG_LMT = 0x15;
            U3DMAWLCFG.field.WL_RX_AGG_TO = 0x80;
            DBGPRINT(RT_DEBUG_ERROR,("2 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
            HW_IO_WRITE32(pAd, UDMA_WLCFG_0, U3DMAWLCFG.word);
        }
#endif

#if 0   
        {
            UDMA_STOP_DROP_EPOUT UDMAStopDropEndPoint;
        
            PlatformAccessCR(pAd, CR_READ, STOP_DROP_EPOUT, (PUCHAR)&UDMAStopDropEndPoint, sizeof(UDMA_STOP_DROP_EPOUT));
            DBGPRINT(RT_DEBUG_TRACE, ("1 UDMAStopDropEndPoint = 0x%08x\n", UDMAStopDropEndPoint));

            UDMAStopDropEndPoint.STOP_EP8OUT = 1;
            //MACValue |= 0x00000100;   // stop EP8

            DBGPRINT(RT_DEBUG_TRACE, ("2 UDMAStopDropEndPoint = 0x%08x\n", UDMAStopDropEndPoint));
            PlatformAccessCR(pAd, CR_WRITE, STOP_DROP_EPOUT, (PUCHAR)&UDMAStopDropEndPoint, sizeof(UDMA_STOP_DROP_EPOUT));
        }
#endif                      

#if 0 // DMA loopback test
        {
            U3DMA_WLCFG         U3DMAWLCFG;
            // Set DMA scheduler to Bypass mode.
            // Enable Tx/Rx
            PlatformAccessCR(pAd, CR_READ, UDMA_WLCFG_0, (PUCHAR)&U3DMAWLCFG, sizeof(U3DMA_WLCFG));
            DBGPRINT(RT_DEBUG_ERROR,("1 U3DMAWLCFG = %x\n", U3DMAWLCFG));
            U3DMAWLCFG.WL_LPK_EN = 1;
            DBGPRINT(RT_DEBUG_ERROR,("2 U3DMAWLCFG = %x\n", U3DMAWLCFG));
            PlatformAccessCR(pAd, CR_WRITE, UDMA_WLCFG_0, (PUCHAR)&U3DMAWLCFG, sizeof(U3DMA_WLCFG));
        }
#endif

#if 0 // for 3+2 test
        {
            ULONG   MACValue = 0;
            
            // Stop Queues Test
            MACValue = 0xffffffff;
            PlatformAccessCR(pAd, CR_WRITE, ARB_TQCR5, (PUCHAR)&MACValue, sizeof(ULONG));

            // Stop Queue Prediction
            MACValue = 0x50FF0020;
            PlatformAccessCR(pAd, CR_WRITE, 0x601100b4, (PUCHAR)&MACValue, sizeof(ULONG));

            // Start Queue Prediction
            MACValue = 0x40F00020;
            PlatformAccessCR(pAd, CR_WRITE, 0x601100b4, (PUCHAR)&MACValue, sizeof(ULONG));

            // Mask Rest queue priority; 0x0000000F enable Queue 0~3; 0x00001E00 enable Queue 9~12
            MACValue = 0x0000000F;
            PlatformAccessCR(pAd, CR_WRITE, 0x500005c4, (PUCHAR)&MACValue, sizeof(ULONG));

            // Endpoints configuration - 3+2
            MACValue = 0x08041040;
            PlatformAccessCR(pAd, CR_WRITE, 0x50000230, (PUCHAR)&MACValue, sizeof(ULONG));
            
        }
#endif

#if 0
        XmitSendNullFrameFor7603Lookback(pAd, 0, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 1, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 2, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 3, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 5, 0);
        Delay_us(10000);

        // Stop Endpoints Test
        if (1)
        {
            ULONG   MACValue = 0;
            PlatformAccessCR(pAd, CR_READ, STOP_DROP_EPOUT, (PUCHAR)&MACValue, sizeof(ULONG));
            DBGPRINT(RT_DEBUG_TRACE, ("1 STOP_DROP_EPOUT = 0x%08x\n", MACValue));

            MACValue |= 0x000003F0;   // stop EP4

            DBGPRINT(RT_DEBUG_TRACE, ("2 STOP_DROP_EPOUT = 0x%08x\n", MACValue));
            PlatformAccessCR(pAd, CR_WRITE, STOP_DROP_EPOUT, (PUCHAR)&MACValue, sizeof(ULONG));
        }

        // Stop Queues Test
        if (0)
        {
            ULONG   MACValue = 0;
            
            MACValue = 0x0000001F;
            PlatformAccessCR(pAd, CR_WRITE, ARB_TQCR4, (PUCHAR)&MACValue, sizeof(ULONG));
            //HW_IO_WRITE32(pAd, ARB_TQCR1, val);
            Delay_us(500);
        }

        

        XmitSendNullFrameFor7603Lookback(pAd, 0, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 1, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 2, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 3, 0);
        XmitSendNullFrameFor7603Lookback(pAd, 5, 0);
#endif      

//XmitSendNullFrameFor7603Lookback(pAd, 0, 0);

        if (0)
        {
            UCHAR nEndpoint = 0, nQueue = 0;

            //nEndpoint = 1;

            //for (nEndpoint = 0; nEndpoint < 6; nEndpoint ++)
            {
                for (nQueue = 0; nQueue < 15; nQueue ++)
                {
                    if (nQueue != 5 && nQueue != 6 && nQueue != 7 && nQueue != 8 && nQueue != 14)
                    {
                        for (nEndpoint = 0; nEndpoint < 6; nEndpoint ++)
                        {
                            XmitSendNullFrameFor7603Lookback(pAd, nEndpoint, nQueue);
                            Delay_us(500);
                        }
                    }
                }
            }
        }

        pAd->TrackInfo.PacketCountFor7603Debug = 0;
        pAd->TrackInfo.RxPacketCountFor7603Debug = 0;
#else
        // Read From Registry
        DBGPRINT(RT_DEBUG_TRACE, ("N6Initialize : AutoBA = %d, Policy = %d\n", pPort->CommonCfg.BACapability.field.AutoBA, pPort->CommonCfg.BACapability.field.Policy));
        DBGPRINT(RT_DEBUG_TRACE, ("pAd->HwCfg.ChannelMode = %d, pPort->CommonCfg.CountryRegion = %d, pPort->CommonCfg.PhyMode = %d\n", pAd->HwCfg.ChannelMode, pPort->CommonCfg.CountryRegion, pPort->CommonCfg.PhyMode));

        {
            PMP_PORT          pPort = pAd->PortList[PORT_0];
            COPY_MAC_ADDR(pPort->CurrentAddress, pAd->HwCfg.CurrentAddress);  
        }
    
        
        NdisCommonMappingPortToBssIdx(pPort, UPDATE_MAC, DO_NOTHING);

        // Set BW
        MtAsicSetBW(pAd, BW_20);
                
        // Enable Tx/Rx
        MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);
        
        MtAsicSetRxFilter(pAd);

        MlmeInfoSetPhyMode(pAd, pPort, pPort->CommonCfg.PhyMode,TRUE);

        pPort->CommonCfg.bAutoTxRateSwitch = TRUE;

        // UDMA Rx Aggregation Setting
        if (1)
        {
            U3DMA_WLCFG         U3DMAWLCFG;
            HW_IO_READ32(pAd, UDMA_WLCFG_0, (UINT32 *)&U3DMAWLCFG.word);
            DBGPRINT(RT_DEBUG_ERROR,("1 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
            U3DMAWLCFG.field.WL_RX_AGG_EN = 1;
            U3DMAWLCFG.field.WL_RX_AGG_LMT = 0x18;  // 24K
            U3DMAWLCFG.field.WL_RX_AGG_TO = 0x30;   // 48us
            DBGPRINT(RT_DEBUG_ERROR,("2 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
            HW_IO_WRITE32(pAd, UDMA_WLCFG_0, (UINT32)U3DMAWLCFG.word);
        }
#endif

        //
        // We have indicated PhyPowerState at NICInitAsicFromEEPROM(...), but OS might not 100% get 
        // the HW Radio status and keeps the Radio ON but it supposes to be Radio Off when HW was Radio off.
        // So Let PlatformIndicateCurrentPhyPowerState to indicate the HW Radio status again by change the LastHwRadio 
        // to undefined and PlatformIndicateCurrentPhyPowerState will indicate the SW/HW Radio status again.
        //
        pAd->StaCfg.LastHwRadio = 0xFFFFFFFF;

        pPort->CommonCfg.bInitializationState = TRUE;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<== %s, ndisStatus=0x%08x\n", __FUNCTION__, NdisStatus));


    return NdisStatus;
}

NDIS_STATUS
N6Restart(
    IN  NDIS_HANDLE                         MiniportAdapterContext,
    IN  PNDIS_MINIPORT_RESTART_PARAMETERS   MiniportRestartParameters
    )
{
    PMP_ADAPTER pAd = (PMP_ADAPTER) MiniportAdapterContext;
    PNDIS_RESTART_ATTRIBUTES     NdisRestartAttributes;
    PNDIS_RESTART_GENERAL_ATTRIBUTES  NdisGeneralAttributes;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;   

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6Restart\n"));

    NdisRestartAttributes = MiniportRestartParameters->RestartAttributes;

    //
    // If NdisRestartAttributes is not NULL, then miniport can modify
    // generic attributes and add new media specific info attributes
    // at the end. Otherwise, NDIS restarts the miniport because
    // of other reason, miniport should not try to modify/add attributes
    //
    if (NdisRestartAttributes != NULL)
    {
        ASSERT(NdisRestartAttributes->Oid == OID_GEN_MINIPORT_RESTART_ATTRIBUTES);

        NdisGeneralAttributes = (PNDIS_RESTART_GENERAL_ATTRIBUTES)NdisRestartAttributes->Data;

        //
        // Check to see if we need to change any attributes, for example,
        // the driver can change the current MAC address here. Or the
        // driver can add media specific info attributes.
        //      
    }
    
    NdisAcquireSpinLock(&pAd->AdapterStateLock);
    pAd->AdapterState = NicRunning;
    NdisReleaseSpinLock(&pAd->AdapterStateLock);

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6Restart\n"));
    return ndisStatus;
}

/*
    ========================================================================
    
    Routine Description:
        Miniport pause and restart are new features introduced in NDIS 6.0.
        NDIS calls a miniport driver's MiniportPause function to stop
        data flow before a Plug and Play operation, such as adding or
        removing a filter driver or binding or unbinding a protocol driver,
        is performed. The adapter remains in the Pausing state until the
        pause operation has completed.

    Arguments:
        MiniportAdapterContext      Pointer to our adapter
        MiniportPauseParameters     Not use

    Return Value:
        NDIS_STATUS_SUCCESS         Put to PAUSE state ok.
        NDIS_STATUS_PENDING         Can't put to PAUSE state at this moment


    Note:
        A miniport can't fail the pause request and
        should do the following:
        1.) Wait for all calls to the NdisMIndicateReceiveNetBufferLists
            function to return.     
        2.) Wait for NDIS to return the ownership of all NET_BUFFER_LIST
            structures from outstanding receive indications to the miniport
            driver's MiniportReturnNetBufferLists function.
        3.) Complete all outstanding send requests by calling NdisMSendNetBufferListsComplete. 
        4.) Reject all new send requests made to its MiniportSendNetBufferLists 
        
    ========================================================================
*/
NDIS_STATUS
N6Pause(
    IN  NDIS_HANDLE                         MiniportAdapterContext,
    IN  PNDIS_MINIPORT_PAUSE_PARAMETERS     MiniportPauseParameters
    )
{
    PMP_ADAPTER   pAd = (PMP_ADAPTER) MiniportAdapterContext;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(MiniportPauseParameters);
    
    DBGPRINT(RT_DEBUG_TRACE, ("==> N6Pause\n"));

    ASSERT(pAd->AdapterState == NicRunning);

    if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
    {
        LedCtrlSetLed(pAd, LED_WPS_TURN_LED_OFF);
    }

    NdisAcquireSpinLock(&pAd->AdapterStateLock);
    pAd->AdapterState = NicPausing;
    NdisReleaseSpinLock(&pAd->AdapterStateLock);

    do
    {
        //
        // Complete all the pending sends
        //
        Ndis6CommonRejectPendingPackets(pAd); 
        //ndisStatus = NDIS_STATUS_SUCCESS;

        if (pAd->pRxCfg->nIndicatedRxPkts == 0)
            ndisStatus = NDIS_STATUS_SUCCESS;       
        else
        {
            // We will not complete this IPR, so don't return NDIS_STATUS_PENDING
            ULONG   CheckIndicateNetBufferIsDoneCount = 0;
            while ((pAd->pRxCfg->nIndicatedRxPkts != 0) && CheckIndicateNetBufferIsDoneCount<=500)
            {
                NdisCommonGenericDelay(1000);
                CheckIndicateNetBufferIsDoneCount++;
                
                DBGPRINT(RT_DEBUG_ERROR, ("%s , nIndicatedRxPkts = %d, CheckIndicateNetBufferIsDoneCount = %d\n", 
                    __FUNCTION__,
                    pAd->pRxCfg->nIndicatedRxPkts,
                    CheckIndicateNetBufferIsDoneCount));
            }
            ndisStatus = NDIS_STATUS_SUCCESS;
        }
    } while (FALSE);

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6Pause, ndisStatus=0x%08x,nIndicatedRxPkts = %d\n", ndisStatus,pAd->pRxCfg->nIndicatedRxPkts));

    return ndisStatus;
}

VOID 
N6ShutdownHandler(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_SHUTDOWN_ACTION    ShutdownAction
    )
{
    PMP_ADAPTER   pAd = (PMP_ADAPTER) MiniportAdapterContext;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    ULONG           i = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6ShutdownHandler\n"));

    
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
    pAd->Shutdowncnt = SHUTDOWN_TIMEOUT;
    pAd->bInShutdown = TRUE;    
    pAd->bWaitShutdownCMD = TRUE;

    if (ShutdownAction == NdisShutdownPowerOff)
    {
        //
        // Ask command thread to do the task of RTMPAdapterShutdown
        //
        MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_RTMP_SHUTDOWN, NULL ,0);

        //
        // Wait it to be finished or exit if experience some error and can't finished within 1 mins.
        //
        i = 0;
        do 
        {
            NdisCommonGenericDelay(100 * 1000); // sleep 100 milliseconds
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Wait N6ShutdownHandler to be completed or time out %d * 0.1/ 60 seconds\n", __FUNCTION__, i));
        } while ((pAd->bWaitShutdownCMD == TRUE) && ( i++ < 600));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6ShutdownHandler\n"));
}

VOID 
N6Shutdown(
    IN  PMP_ADAPTER             pAd
    )
{
    UCHAR       RfValue = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6Shutdown\n"));

    // Turn off the LEDs.
    if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
    {
        LedCtrlSetLed(pAd, LED_WPS_TURN_LED_OFF);
    }

    LedCtrlSetLed(pAd, LED_HALT);
    //
    // Force signal strength Led to be turned off, firmware is not done it.
    //
    LedCtrlSetSignalLed(pAd, -100);
    DBGPRINT(RT_DEBUG_TRACE, ("N6Shutdown turn off led\n"));   

    //
    // According to Xtal Programming v0.3
    //
    //auto turning mode
    // R6[6] = 1
    RT30xxReadRFRegister(pAd, RF_R06, &RfValue);
    RfValue = (RfValue | 0x40); // vco_ic (VCO bias current control, 01: low)
    RT30xxWriteRFRegister(pAd, RF_R06, RfValue);

    // Xtal code
    RT30xxReadRFRegister(pAd, RF_R23, &RfValue);
    AsicSendCommanToMcu(pAd, RFTUNE_MCU_CMD, 0xff, 0, RfValue);

#if 0       // Shall Radio Off, but not verified
    // Radio Off, put at last
    if(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        AsicRadioOff(pAd);
    }   
#else   
    if(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    {
        // Disable Rx
        SendDisableRxCMD(pAd);

        // Polling Rx Empty
        if (1)
        {
            U3DMA_WLCFG         U3DMAWLCFG;
            int i = 0;
            
            do
            {
                HW_IO_READ32(pAd, UDMA_WLCFG_0, (UINT32 *)&U3DMAWLCFG.word);
                DBGPRINT(RT_DEBUG_TRACE,("%d Wait Tx/Rx to be idle, Rx = %d, Tx = %d\n", i, U3DMAWLCFG.field.WL_RX_BUSY, U3DMAWLCFG.field.WL_TX_BUSY));

                i++;
                if (i == 1000)
                    break;
            } while ((U3DMAWLCFG.field.WL_RX_BUSY == 1) || (U3DMAWLCFG.field.WL_TX_BUSY == 1));
        }
    
        SendRadioOnOffCMD(pAd, PWR_RadioOff);
    }
#endif

    //
    // Put this line at the bottom of this function.
    // It have to turn off the flag of bWaitShutdownCMD before return back. 
    //
    pAd->bWaitShutdownCMD = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6Shutdown\n"));
}

VOID
N6DevicePnPEvent(
    IN NDIS_HANDLE                  MiniportAdapterContext,
    IN PNET_DEVICE_PNP_EVENT        NetDevicePnPEvent
    )
{
    PMP_ADAPTER           pAd = (PMP_ADAPTER) MiniportAdapterContext;
    NDIS_DEVICE_PNP_EVENT   DevicePnPEvent = NetDevicePnPEvent->DevicePnPEvent;
    PVOID                   InformationBuffer = NetDevicePnPEvent->InformationBuffer;
    ULONG                   InformationBufferLength = NetDevicePnPEvent->InformationBufferLength;

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6DevicePnPEvent\n"));

    switch (DevicePnPEvent)
    {
        case NdisDevicePnPEventQueryRemoved:
            DBGPRINT(RT_DEBUG_TRACE, ("N6DevicePnPEvent: NdisDevicePnPEventQueryRemoved\n"));
            break;

        case NdisDevicePnPEventRemoved:
            DBGPRINT(RT_DEBUG_TRACE, ("N6DevicePnPEvent: NdisDevicePnPEventRemoved\n"));
            break;       

        case NdisDevicePnPEventSurpriseRemoved:
            DBGPRINT(RT_DEBUG_TRACE, ("N6DevicePnPEvent: NdisDevicePnPEventSurpriseRemoved\n"));

            Ndis6CommonRejectPendingPackets(pAd); 

            PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_SUCCESS, TRUE, TRUE);

            MT_SET_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED);
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
            NdisCommonStopRx(pAd);
#if 0           
            MpOnSurpriseRemoval(pAd);
#endif
            break;

        case NdisDevicePnPEventQueryStopped:
            DBGPRINT(RT_DEBUG_TRACE, ("N6DevicePnPEvent: NdisDevicePnPEventQueryStopped\n"));
            break;

        case NdisDevicePnPEventStopped:
            DBGPRINT(RT_DEBUG_TRACE, ("N6DevicePnPEvent: NdisDevicePnPEventStopped\n"));
            break;      

        case NdisDevicePnPEventPowerProfileChanged:
            DBGPRINT(RT_DEBUG_TRACE, ("N6DevicePnPEvent: NdisDevicePnPEventPowerProfileChanged\n"));
            break;      

        default:
            DBGPRINT(RT_DEBUG_TRACE, ("N6DevicePnPEvent: unknown PnP event %x \n", DevicePnPEvent));
            break;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6DevicePnPEvent\n")); 
}

VOID
N6Halt(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HALT_ACTION        HaltAction
    )
{
    PMP_ADAPTER   pAd = (PMP_ADAPTER) MiniportAdapterContext;
    BOOLEAN         Cancelled =FALSE;
    ULONG           TxPinCfg = 0x00050F0F; 
    PIRP pIrp = NULL;
    PMP_PORT      pPort = pAd->PortList[PORT_0];
    UCHAR               RfValue = 0;

    // Don't put any code before this line
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
    
    DBGPRINT(RT_DEBUG_TRACE, ("==> N6Halt\n"));   

    // Turn off the LEDs.
    if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED))) // LED mode 9.
    {
//**        LedCtrlSetLed(pAd, LED_WPS_TURN_LED_OFF);
    }
    
    //
    // Before set flag fRTMP_ADAPTER_HALT_IN_PROGRESS, 
    // if connecting to an AP, try to DIS-ASSOC it before leave 
    // we should send a disassoc frame to our AP.
    //    
    if ((pPort->PortSubtype == PORTSUBTYPE_STA) &&
        INFRA_ON(pPort) && 
        !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) 
    {
        // send DLS-TEAR_DOWN message, 
        if (DLS_ON(pAd))
        {
            DlsTearDown(pAd, pPort);
        }
        // send TDLS-TEAR_DOWN message, 
        if (TDLS_ON(pAd))
        {
            TdlsTearDown(pAd, TRUE);
        }
    }

    P2pShutdown(pAd);

    // free all AP related resources
    APShutdown(pAd);
     
    NdisInitFreeRepositMemory(pAd);
    
    MlmeHalt(pAd, pAd->PortList[NDIS_DEFAULT_PORT_NUMBER]);

    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED))  // Lens : I don't know who added delay time 500ms
    {
        // Delay a specfic time to finish unfinished task
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Delay 500mse start\n", __FUNCTION__));
        NdisCommonGenericDelay(500000);
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Delay 500mse end\n", __FUNCTION__));
    }

    NdisCommonCleanUpMLMEWaitQueue(pAd);
    NdisCommonCleanUpMLMEBulkOutQueue(pAd);

    // Free variable port data
    Ndis6CommonFreePortData(pAd);

//**
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
#if 0   
        if(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        {
            AsicRadioOff(pAd);
        }
#else
        if(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SendRadioOnOffCMD 1\n"));

            // Disable Rx
            SendDisableRxCMD(pAd);

            // Polling Rx Empty
            if (1)
            {
                U3DMA_WLCFG         U3DMAWLCFG;
                int i = 0;
                
                do
                {
                    HW_IO_READ32(pAd, UDMA_WLCFG_0, (UINT32 *)&U3DMAWLCFG.word);
                    DBGPRINT(RT_DEBUG_TRACE,("%d Wait Tx/Rx to be idle, Rx = %d, Tx = %d\n", i, U3DMAWLCFG.field.WL_RX_BUSY, U3DMAWLCFG.field.WL_TX_BUSY));

                    i++;
                    if (i == 1000)
                        break;
                } while ((U3DMAWLCFG.field.WL_RX_BUSY == 1) || (U3DMAWLCFG.field.WL_TX_BUSY == 1));
            }
    
            SendRadioOnOffCMD(pAd, PWR_RadioOff);
        }
        DBGPRINT(RT_DEBUG_TRACE, ("SendRadioOnOffCMD 2\n"));
#endif
    }
    DBGPRINT(RT_DEBUG_TRACE, ("SendRadioOnOffCMD 3\n"));

    NdisCommonStopTx(pAd);
    NdisCommonStopRx(pAd);

    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED))
    {
        // Sleep 50 milliseconds so pending io might finish normally
        NdisCommonGenericDelay(50000);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("Check All Pending PacketsIndicated = %d are return \n", pAd->pRxCfg->nIndicatedRxPkts));
    ASSERT(pAd->pRxCfg->nIndicatedRxPkts == 0);

    // Terminate Threads
    DBGPRINT(RT_DEBUG_TRACE, ("Terminate Threads\n"));
    NdisCommonFreeAllThreads(pAd);
    NdisCommonFreeAllWorkitems(pAd);

    MtFreeInternalCmdBuffer(pAd);

    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED))  // Lens : I don't know who added delay time 500ms
    {
        // Delay a specfic time to finish unfinished task
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Delay 500mse start\n", __FUNCTION__));
        NdisCommonGenericDelay(500000);
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Delay 500mse end\n", __FUNCTION__));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("start to wait pending request\n"));
    NdisCommonWaitPendingRequest(pAd);

    N6PortDeRegisterActivePort(pAd);
    
    N6FreePortMemory(pAd);

 #if _WIN8_USB_SS_SUPPORTED
    //
    //Halt for USB SS.
    //
    UsbSsHalt(pAd);
#endif
    
    ReleaseAdapter(pAd);
    
    DBGPRINT(RT_DEBUG_TRACE, ("<== G_MemoryAllocate_cnt   %d\n",G_MemoryAllocate_cnt));
    DBGPRINT(RT_DEBUG_TRACE, ("<== G_MemoryFree_cnt   %d\n",G_MemoryFree_cnt));
    DBGPRINT(RT_DEBUG_TRACE, ("<== G_MemoryAllocate_Len   %d\n",G_MemoryAllocate_Len));
    DBGPRINT(RT_DEBUG_TRACE, ("<== G_MemoryFree_Len   %d\n",G_MemoryFree_Len));
    DBGPRINT(RT_DEBUG_TRACE, ("<== N6Halt\n"));
}

VOID
N6Unload(
    IN  PDRIVER_OBJECT  DriverObject
    )
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> N6Unload\n"));
    
    NdisMDeregisterMiniportDriver(G_NdisMiniportDriverHandle);  

    WdfDriverMiniportUnload(WdfGetDriver());
    DBGPRINT(RT_DEBUG_TRACE, ("<== N6Unload\n"));
}

NDIS_STATUS
ProcessNdis6OIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
    USHORT  i=0;
    DBGPRINT(RT_DEBUG_LOUD, ("===>%s()\n", __FUNCTION__)); 
    for(i = 0 ; i < sizeof(MTK_SUPPORT_N6_OIDs)/sizeof(OID_LIST_ENTRY); i++)
    {
        if(NdisRequest->DATA.QUERY_INFORMATION.Oid == MTK_SUPPORT_N6_OIDs[i].OID)
        {
            if ((NdisRequest->RequestType ==NdisRequestQueryInformation) ||(NdisRequest->RequestType ==NdisRequestQueryStatistics))
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Query %s\n",MTK_SUPPORT_N6_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_N6_OIDs[i].QueryFunc(pAd, NdisRequest);
                break;
            }
            else if ((NdisRequest->RequestType ==NdisRequestSetInformation) ||(NdisRequest->RequestType ==NdisRequestMethod))   
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Set %s\n",MTK_SUPPORT_N6_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_N6_OIDs[i].SetFunc(pAd, NdisRequest);
                break;
            }
        }
    }
    DBGPRINT(RT_DEBUG_LOUD, ("<===%s(), status = 0x%x\n", __FUNCTION__, ndisStatus)); 
    return ndisStatus;
}

NDIS_STATUS
ProcessNdis5OIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
    USHORT  i=0;
    DBGPRINT(RT_DEBUG_LOUD, ("===>%s()\n", __FUNCTION__)); 
    for(i = 0 ; i < sizeof(MTK_SUPPORT_N5_OIDs)/sizeof(OID_LIST_ENTRY); i++)
    {
        if(NdisRequest->DATA.QUERY_INFORMATION.Oid == MTK_SUPPORT_N5_OIDs[i].OID)
        {
            if ((NdisRequest->RequestType ==NdisRequestQueryInformation) ||(NdisRequest->RequestType ==NdisRequestQueryStatistics))
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Query %s\n",MTK_SUPPORT_N5_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_N5_OIDs[i].QueryFunc(pAd, NdisRequest);
                break;
            }
            else if ((NdisRequest->RequestType ==NdisRequestSetInformation) ||(NdisRequest->RequestType ==NdisRequestMethod))   
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Set %s\n",MTK_SUPPORT_N5_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_N5_OIDs[i].SetFunc(pAd, NdisRequest);
                break;
            }
        }
    }
    DBGPRINT(RT_DEBUG_LOUD, ("<===%s(), status = 0x%x\n", __FUNCTION__, ndisStatus)); 
    return ndisStatus;
}

NDIS_STATUS
ProcessMTKOIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
    USHORT  i=0;
    DBGPRINT(RT_DEBUG_LOUD, ("===>%s()\n", __FUNCTION__)); 
    for(i = 0 ; i < sizeof(MTK_SUPPORT_SELF_OIDs)/sizeof(OID_LIST_ENTRY); i++)
    {
        if(NdisRequest->DATA.QUERY_INFORMATION.Oid == MTK_SUPPORT_SELF_OIDs[i].OID)
        {
            if ((NdisRequest->DATA.QUERY_INFORMATION.Oid & MTK_TOGGLE_OID_SET) == MTK_TOGGLE_OID_SET)
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Set %s\n",MTK_SUPPORT_SELF_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_SELF_OIDs[i].SetFunc(pAd, NdisRequest);
                break;
            }
            else
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Query %s\n",MTK_SUPPORT_SELF_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_SELF_OIDs[i].QueryFunc(pAd, NdisRequest);
                break;
            }
        }
    }
    DBGPRINT(RT_DEBUG_LOUD, ("<===%s(), status = 0x%x\n", __FUNCTION__, ndisStatus)); 
    return ndisStatus;
}

#if (SUPPORT_QA_TOOL ==1)
NDIS_STATUS
ProcessMTKQAOIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
    USHORT  i=0;
    DBGPRINT(RT_DEBUG_LOUD, ("===>%s()\n", __FUNCTION__)); 
    for(i = 0 ; i < sizeof(MTK_SUPPORT_QA_OIDs)/sizeof(OID_LIST_ENTRY); i++)
    {
        if(NdisRequest->DATA.QUERY_INFORMATION.Oid == MTK_SUPPORT_QA_OIDs[i].OID)
        {
            if ((NdisRequest->RequestType ==NdisRequestQueryInformation) ||(NdisRequest->RequestType ==NdisRequestQueryStatistics))
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Query %s\n",MTK_SUPPORT_QA_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_QA_OIDs[i].QueryFunc(pAd, NdisRequest);
                break;
            }
            else if ((NdisRequest->RequestType ==NdisRequestSetInformation) ||(NdisRequest->RequestType ==NdisRequestMethod))   
            {
                DBGPRINT(RT_DEBUG_LOUD, ("Set %s\n",MTK_SUPPORT_SELF_OIDs[i].DbgOidStr)); 
                ndisStatus = MTK_SUPPORT_QA_OIDs[i].SetFunc(pAd, NdisRequest);
                break;
            }
        }
    }
    DBGPRINT(RT_DEBUG_LOUD, ("<===%s(), status = 0x%x\n", __FUNCTION__, ndisStatus)); 
    return ndisStatus;
}
#endif

NDIS_STATUS
ProcessOIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )  
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
    DBGPRINT(RT_DEBUG_LOUD, ("===>%s()\n", __FUNCTION__)); 
    //Process Ndis6 related OID.
    if(ndisStatus == NDIS_STATUS_NOT_RECOGNIZED)
    {
        ndisStatus = ProcessNdis6OIDRequest(pAd, NdisRequest);
    }

    //Process Ndis5 related OID.
    if(ndisStatus == NDIS_STATUS_NOT_RECOGNIZED)
    {
        ndisStatus = ProcessNdis5OIDRequest(pAd, NdisRequest);
    }

     //Process MTK related OID.
    if(ndisStatus == NDIS_STATUS_NOT_RECOGNIZED)
    {
        ndisStatus = ProcessMTKOIDRequest(pAd, NdisRequest);
    }  

#if (SUPPORT_QA_TOOL ==1)
     //Process MTK QA related OID.
    if(ndisStatus == NDIS_STATUS_NOT_RECOGNIZED)
    {
        ndisStatus = ProcessMTKQAOIDRequest(pAd, NdisRequest);
    }      
#endif

   DBGPRINT(RT_DEBUG_LOUD, ("<===%s(), status = 0x%x\n", __FUNCTION__, ndisStatus));
    return ndisStatus;
    
}

NDIS_STATUS
N6OIDRequest(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    PMP_ADAPTER   pAd = (PMP_ADAPTER)MiniportAdapterContext;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    PMP_PORT      pPort;

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED))
    {
        ndisStatus = NDIS_STATUS_NOT_ACCEPTED;
        DBGPRINT(RT_DEBUG_ERROR, ("NdisRequest failed as surprise removal is in progress\n"));

        return ndisStatus;
    }

    if (NdisRequest->PortNumber >= RTMP_MAX_NUMBER_OF_PORT)
    {
        ndisStatus = NDIS_STATUS_FAILURE;

        DBGPRINT(RT_DEBUG_ERROR, ("N6OIDRequest:: Invalid PortNumber(=%d) Max PortNumber(=%d)\n", NdisRequest->PortNumber, pAd->NumberOfPorts));
        return ndisStatus;
    }

    //
    // Since the NdisRequest->PortNumber might be wrong, after senity check then we can get Port at here.
    //
    pPort = pAd->PortList[NdisRequest->PortNumber];
    
    if (pPort == NULL)
    {
        ndisStatus = NDIS_STATUS_FAILURE;

        DBGPRINT(RT_DEBUG_ERROR, ("PortNumber (%d) does not exist in the PortList\n",NdisRequest->PortNumber));

        return ndisStatus;
    }

    if(NdisRequest->DATA.SET_INFORMATION.Oid == OID_PNP_SET_POWER)
    {
        PlatformMoveMemory(&pAd->pNicCfg->PendedDevicePowerState, NdisRequest->DATA.SET_INFORMATION.InformationBuffer, sizeof(NDIS_DEVICE_POWER_STATE));

        if(pAd->pNicCfg->PendedDevicePowerState != NdisDeviceStateD0)
        {
            BOOLEAN             Cancelled;
                
            // This is only for WHCK testing.
            // Reason : The scan request happened before S3/S4, and the original return scan confirm 
            //               is in the MlmeRestartStateMachine() which is in the bottom of this function.
            //               The scan confirm in the MlmeRestartStateMachine is NDIS_STATUS_REQUEST_ABORTED.
            //          But the WHCK will expect the scan confirm is NDIS_STATUS_SUCCESS.
            //
            DBGPRINT(RT_DEBUG_TRACE, ("Lens : into OID_PNP_SET_POWER, OnTestingWHQL = %d, bNeedPlatformIndicateScanStatus = %d, ucScanPortNum = %d, PortNumber = %d\n", pAd->LogoTestCfg.OnTestingWHQL, pAd->MlmeAux.bNeedPlatformIndicateScanStatus, pAd->ucScanPortNum, pPort->PortNumber));

            if (pAd->MlmeAux.bNeedPlatformIndicateScanStatus || MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {
                // stop scan
                PlatformCancelTimer(&pPort->Mlme.ScanTimer, &Cancelled);
                pPort->ScaningChannel = 0;
            }

            PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_SUCCESS, TRUE, TRUE);
        }
    }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if ((NdisRequest->DATA.METHOD_INFORMATION.Oid == OID_DOT11_ENUM_BSS_LIST) && pAd->LogoTestCfg.OnTestingWHQL)  // Only for BVT Test on Win8
    {
        if(pAd->MlmeAux.bNeedPlatformIndicateScanStatus)
        {
            DBGPRINT(RT_DEBUG_TRACE,("N6OIDRequest : OID_DOT11_ENUM_BSS_LIST : NDIS_STATUS_PENDING\n"));

            DBGPRINT(RT_DEBUG_TRACE, ("N6OIDRequest, Pending this OID...\n"));
            NdisAcquireSpinLock(&pAd->pNicCfg->PendingOIDLock);
            pAd->pNicCfg->PendOidEntry.PendingNdisRequest = NdisRequest;
            NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
            NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->pNicCfg->PendOidEntry.OidPendStartTime);
            
            return NDIS_STATUS_PENDING;
        }
    }
#endif

    if(NdisRequest->DATA.SET_INFORMATION.Oid == OID_DOT11_SCAN_REQUEST)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6OIDRequest, Oid == OID_DOT11_SCAN_REQUEST\n"));
        pAd->pNicCfg->PendedScanRequest = NdisRequest;
        if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))
        {
            DBGPRINT(RT_DEBUG_TRACE,("N6OIDRequest : OID_DOT11_SCAN_REQUEST : fRTMP_ADAPTER_RESET_IN_PROGRESS = TRUE\n"));
            return NDIS_STATUS_RESET_IN_PROGRESS;
        }

        if(pAd->MlmeAux.bNeedPlatformIndicateScanStatus || MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(pAd->AdapterState == NicPausing) || (pAd->pNicCfg->PendedDevicePowerState != NdisDeviceStateD0))
        {
            DBGPRINT(RT_DEBUG_TRACE,("N6OIDRequest : OID_DOT11_SCAN_REQUEST :Media in use!!!! (%d %d %d %d)\n", pAd->MlmeAux.bNeedPlatformIndicateScanStatus, MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS), (pAd->AdapterState == NicPausing), (pAd->pNicCfg->PendedDevicePowerState != NdisDeviceStateD0)));
            return NDIS_STATUS_DOT11_MEDIA_IN_USE;
        }
    }
    
    if((NdisRequest->DATA.METHOD_INFORMATION.Oid == OID_DOT11_RESET_REQUEST) && (pPort->PortNumber == PORT_0))
    {   
        DBGPRINT(RT_DEBUG_TRACE, ("N6OIDRequest, OID_DOT11_RESET_REQUEST OID....\n"));
        PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_REQUEST_ABORTED, TRUE, TRUE);
    }

    ndisStatus = ProcessOIDRequest(pAd, NdisRequest);

    if(ndisStatus == NDIS_STATUS_PENDING)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6OIDRequest, Pending this OID...\n"));
        NdisAcquireSpinLock(&pAd->pNicCfg->PendingOIDLock);
        pAd->pNicCfg->PendOidEntry.PendingNdisRequest = NdisRequest;
        NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
        NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->pNicCfg->PendOidEntry.OidPendStartTime);
    }
    return ndisStatus;
}

VOID
N6DUMP_OID(
    NDIS_OID Oid
)
{
    LPSTR   StrBuf = "";

    switch (Oid) 
    {
        case OID_PNP_SET_POWER        : StrBuf = "OID_PNP_SET_POWER";       break;
        case OID_DOT11_SCAN_REQUEST   : StrBuf = "OID_DOT11_SCAN_REQUEST";  break;
#if COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER)
        case OID_DOT11_CREATE_MAC   : StrBuf = "OID_DOT11_CREATE_MAC";  break;
        case OID_DOT11_DELETE_MAC   : StrBuf = "OID_DOT11_DELETE_MAC";  break;
#endif

        default:
        StrBuf = "UNKNOWN";
        break;
    }
     DBGPRINT(RT_DEBUG_TRACE, ("N6DUMP_OID()  ..... %s\n", StrBuf));
}


BOOLEAN
N6CompletePendingOID(
    IN  PMP_ADAPTER               pAd,
    IN  PENDING_OID_TYPE          OidType,
    IN  NDIS_STATUS                   ndisStatus
    )
{
    BOOLEAN                         bNeedComplete = FALSE;
    NDIS_OID                        Oid;
    PNDIS_OID_REQUEST        tmpPendedRequest = NULL;
    ULONGLONG Now64=0;

    NdisAcquireSpinLock(&pAd->pNicCfg->PendingOIDLock);
    if(pAd->pNicCfg->PendOidEntry.PendingNdisRequest == NULL)
    {
        NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
        return FALSE;
    }
  
    Oid = pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.Oid;

    switch(OidType)
    {
        case PENDING_DEFAULT:
            bNeedComplete = TRUE;
        break;

        case PENDING_SCAN_REQUEST:
            if(Oid == OID_DOT11_SCAN_REQUEST)
                bNeedComplete = TRUE;
        break;

        case PENDING_PNP_SET_POWER:
            if(Oid == OID_PNP_SET_POWER)
                bNeedComplete = TRUE;
        break;

        case PENDING_NIC_POWER_STATE:
            if(Oid == OID_DOT11_NIC_POWER_STATE)
                bNeedComplete = TRUE;
        break;

#if COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER)
        case PENDING_CREATE_DELETE_MAC:
            if(Oid == OID_DOT11_CREATE_MAC || Oid == OID_DOT11_DELETE_MAC)
                bNeedComplete = TRUE;
        break;
#endif
        default:
            break;
    }

    if(bNeedComplete)
    {
        tmpPendedRequest = pAd->pNicCfg->PendOidEntry.PendingNdisRequest;
        pAd->pNicCfg->PendOidEntry.PendingNdisRequest = NULL;
    }

    NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
    
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

    pAd->pNicCfg->PendOidEntry.OidPendingTime = (ULONG)((Now64 - pAd->pNicCfg->PendOidEntry.OidPendStartTime)/10000); // in ms.

    DBGPRINT(RT_DEBUG_TRACE, ("N6CompletePendingOID(), Pending this OID = %d, in %d  ms\n",OidType, pAd->pNicCfg->PendOidEntry.OidPendingTime));
    
    if(bNeedComplete)
    {
        NdisMOidRequestComplete(
                    pAd->AdapterHandle,
                    tmpPendedRequest,
                    ndisStatus);
                    
        tmpPendedRequest = NULL;
        N6DUMP_OID(Oid);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID N6CancelOIDRequest(
    IN NDIS_HANDLE hMiniportAdapterContext,
    IN PVOID       RequestId
    )
{
    UNREFERENCED_PARAMETER(hMiniportAdapterContext);
    UNREFERENCED_PARAMETER(RequestId);
}

NDIS_STATUS
N6SetOptions(
    IN NDIS_HANDLE  NdisMiniportDriverHandle,
    IN NDIS_HANDLE  MiniportDriverContext
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6SetOptions\n"));

    do
    {
#if _WIN8_USB_SS_SUPPORTED
        ndisStatus = N6RegisterSelectiveSuspend(NdisMiniportDriverHandle);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("[Ss]Fail to register selective suspend characteristics.\n"));
            break;
        }
#endif

    } while (0);

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6SetOptions\n"));
    return ndisStatus;
}

VOID
N6CancelSendNetBufferLists(
    IN    NDIS_HANDLE              MiniportAdapterContext,
    IN    PVOID                    CancelId
    )
{
    PMP_ADAPTER       pAd = (PMP_ADAPTER)MiniportAdapterContext;
    PNET_BUFFER_LIST    NetBufferList;
    PNET_BUFFER_LIST    CancelHeadNetBufferList = NULL;
    PNET_BUFFER_LIST    CancelTailNetBufferList = NULL;
    PVOID               NetBufferListId;
    UCHAR               Index;
    PMT_XMIT_CTRL_UNIT pXcu = NULL;


    PQUEUE_HEADER   pQueue;
    PQUEUE_ENTRY    pEntry;
    
    DBGPRINT(RT_DEBUG_TRACE, ("==> N6CancelSendNetBufferLists\n"));

    for (Index = 0; Index < 4; Index++)
    {
        NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[Index]);
        while (pAd->pTxCfg->SendTxWaitQueue[Index].Head != NULL)
        {
            pQueue = (PQUEUE_HEADER) &(pAd->pTxCfg->SendTxWaitQueue[Index]);
            pEntry = RemoveHeadQueue(pQueue);
            // Retrieve Ndis NET BUFFER pointer from MiniportReserved field
            pXcu = (PMT_XMIT_CTRL_UNIT)pEntry;

            NetBufferList = pXcu->Reserve1;
            NetBufferListId = NDIS_GET_NET_BUFFER_LIST_CANCEL_ID(NetBufferList);

            if (NetBufferListId == CancelId)
            {
                NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_REQUEST_ABORTED;

                //
                // Queue this NetBufferList for cancellation
                //
                if (CancelHeadNetBufferList == NULL)
                {
                    CancelHeadNetBufferList = NetBufferList;
                    CancelTailNetBufferList = NetBufferList;
                }
                else
                {
                    NET_BUFFER_LIST_NEXT_NBL(CancelTailNetBufferList) = NetBufferList;
                    CancelTailNetBufferList = NetBufferList;
                }
            }
            if(pXcu != NULL)
            {
                InsertTailQueue(&pAd->pTxCfg->XcuIdelQueue, pXcu);                
            }
        }
        NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[Index]);
    }

    //
    // Get the packets from SendCancelQueue and complete them if any
    //
    if (CancelHeadNetBufferList != NULL)
    {
        NET_BUFFER_LIST_NEXT_NBL(CancelTailNetBufferList) = NULL;

        NdisMSendNetBufferListsComplete(
                    pAd->AdapterHandle,
                    CancelHeadNetBufferList,
                    0);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6CancelSendNetBufferLists\n"));

    return;
}

VOID
N6ReturnNetBufferLists(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNET_BUFFER_LIST    NetBufferLists,
    IN  ULONG               ReturnFlags
    )
{
    PMP_ADAPTER               pAd = (PMP_ADAPTER) MiniportAdapterContext;
    PMP_LOCAL_RX_PKTS  pIndicateRxPkts = NULL;
    PNET_BUFFER_LIST            NetBufList;
    PNET_BUFFER_LIST            NextNetBufList;
    
    DBGPRINT(RT_DEBUG_INFO, ("==> N6ReturnNetBufferLists\n"));

    for (NetBufList = NetBufferLists; NetBufList != NULL; NetBufList = NextNetBufList)
    {
        NdisAcquireSpinLock(&pAd->pRxCfg->IndicateRxPktsQueueLock);
        
        NextNetBufList = NET_BUFFER_LIST_NEXT_NBL(NetBufList);
        pIndicateRxPkts = MT_GET_NETBUFFERLIST_FROM_NBL(NetBufList);

        //ASSERT(NET_BUFFER_LIST_NEXT_NBL(NetBufList) == NULL);
        
        if (pIndicateRxPkts != NULL)
        {
            InsertTailQueue(&pAd->pRxCfg->IndicateRxPktsQueue, pIndicateRxPkts);
            MT_DEC_REF(&pAd->pRxCfg->nIndicatedRxPkts);
        }
        NdisReleaseSpinLock(&pAd->pRxCfg->IndicateRxPktsQueueLock);
    }   

    DBGPRINT(RT_DEBUG_INFO, ("N6ReturnNetBufferLists, pAd->pRxCfg->IndicateRxPktsQueue = %d, pAd->pRxCfg->nIndicatedRxPkts = %d\n", pAd->pRxCfg->IndicateRxPktsQueue.Number, pAd->pRxCfg->nIndicatedRxPkts));
    
    DBGPRINT(RT_DEBUG_INFO, ("<== N6ReturnNetBufferLists\n"));
}

BOOLEAN
N6CheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PMP_ADAPTER       pAd = (PMP_ADAPTER) MiniportAdapterContext;
    
    DBGPRINT(RT_DEBUG_TRACE, ("==> %s, PendingPacket=%d,StalledSendTicks=%d,TotalSend=%d,SnapShot=%d\n",
        __FUNCTION__,
        pAd->Counter.MTKCounters.PendingNdisPacketCount,
        pAd->Counter.MTKCounters.NumStalledSendTicks,
        pAd->Counter.MTKCounters.TotalSendNdisPacketCount,
        pAd->Counter.MTKCounters.TotalSendNdisPacketSnapShot));

    //
    // Check if pending packet stalled.
    //
    if (pAd->Counter.MTKCounters.PendingNdisPacketCount > 0)
    {
        if (pAd->Counter.MTKCounters.TotalSendNdisPacketSnapShot == 0)
        {
            pAd->Counter.MTKCounters.TotalSendNdisPacketSnapShot = pAd->Counter.MTKCounters.TotalSendNdisPacketCount;
        }
        else if (pAd->Counter.MTKCounters.TotalSendNdisPacketSnapShot == pAd->Counter.MTKCounters.TotalSendNdisPacketCount)
        {
            pAd->Counter.MTKCounters.NumStalledSendTicks++;

            DBGPRINT(RT_DEBUG_ERROR, ("%s::Pending packet not beeing sent for a while, PendingPacket(=%d), StallSendTicks(=%d)\n", 
                                    __FUNCTION__, pAd->Counter.MTKCounters.PendingNdisPacketCount, pAd->Counter.MTKCounters.NumStalledSendTicks));

            if (pAd->Counter.MTKCounters.NumStalledSendTicks >= 10)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s::TX seems to be stalled. \n", __FUNCTION__));

                //pAd->Counter.MTKCounters.TXHungCount++;

                //
                // return all the pending packets.
                //
                Ndis6CommonRejectPendingPackets(pAd); 

                pAd->Counter.MTKCounters.TotalSendNdisPacketSnapShot = 0;
                pAd->Counter.MTKCounters.NumStalledSendTicks = 0;
                //for SDV issue ,must have an code path return TRUE in checkfor hang routine.
                return TRUE;
            }
        }
        else
        {
            pAd->Counter.MTKCounters.TotalSendNdisPacketSnapShot = 0;
            pAd->Counter.MTKCounters.NumStalledSendTicks = 0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("<== N6CheckForHang\n"));
    return FALSE;
}

NDIS_STATUS
N6Reset(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    OUT PBOOLEAN        AddressingReset
    )
{
    PMP_ADAPTER   pAd = (PMP_ADAPTER)MiniportAdapterContext;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6Reset\n"));

    PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_SUCCESS, TRUE, TRUE);

    if ((pPort->CommonCfg.TgnControl.word&0x3800) != 0)
    {
        pPort->CommonCfg.TgnControl.word &= (~0x3800);
    }

    *AddressingReset = TRUE;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // Disable Win8 NLO
    if(NDIS_WIN8_ABOVE(pAd))
    pAd->StaCfg.NLOEntry.NLOEnable = FALSE;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6Reset\n"));

    return ndisStatus;
}

VOID 
N6CancelDirectOidRequest(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   RequestId
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(RequestId);
}

NDIS_STATUS
N6DirectOidRequest(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    PMP_ADAPTER   pAd = (PMP_ADAPTER)MiniportAdapterContext;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_OID        Oid;
    ULONG           PortType = 0;

    // Depending on the OID, hand it to the appropriate component for processing
    Oid = NdisRequest->DATA.QUERY_INFORMATION.Oid; // Oid is at same offset for all RequestTypes

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED))
    {
        ndisStatus = NDIS_STATUS_NOT_ACCEPTED;
        DBGPRINT(RT_DEBUG_ERROR, ("NdisRequest failed as surprise removal is in progress\n"));

        return ndisStatus;
    }

    PortType = pAd->PortList[NdisRequest->PortNumber]->PortType;

#ifdef MULTI_CHANNEL_SUPPORT    
    if (((PortType == EXTSTA_PORT) || (PortType == WFD_CLIENT_PORT))  && (NdisRequest->DATA.METHOD_INFORMATION.Oid == OID_DOT11_RESET_REQUEST))
    {
        pAd->MccCfg.PendedResetRequest = NdisRequest;
        DBGPRINT(RT_DEBUG_TRACE,("%s(%d) - Record STA/CLI OID_DOT11_RESET_REQUEST(PortNum=%d)\n", __FUNCTION__, __LINE__, NdisRequest->PortNumber));
    }
    else if ((PortType == WFD_GO_PORT)  && (NdisRequest->DATA.METHOD_INFORMATION.Oid == OID_DOT11_RESET_REQUEST))
    {
        pAd->MccCfg.PendedGoOidRequest = NdisRequest;
        DBGPRINT(RT_DEBUG_TRACE,("%s(%d) - Record GO OID_DOT11_RESET_REQUEST(PortNum=%d)\n", __FUNCTION__, __LINE__, NdisRequest->PortNumber));                     
    }
#endif /*MULTI_CHANNEL_SUPPORT*/    

    ndisStatus = ProcessOIDRequest(pAd, NdisRequest);

    if(ndisStatus ==NDIS_STATUS_PENDING)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6OIDRequest, Pending this OID...\n"));
        NdisAcquireSpinLock(&pAd->pNicCfg->PendingOIDLock);
        pAd->pNicCfg->PendOidEntry.PendingNdisRequest = NdisRequest;
        NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
        NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->pNicCfg->PendOidEntry.OidPendStartTime);
    }

    return ndisStatus;
}


