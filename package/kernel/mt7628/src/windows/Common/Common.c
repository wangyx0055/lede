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
    rtmpCommon.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */


#include "MtConfig.h"  
#include "Common.h"
#include "..\tables\RateTables.h"   

//
// BBP register initialization set
//
REG_PAIR   BBPRegTable[] = {
    {BBP_R65,       0x2C},      // fix rssi issue
    {BBP_R66,       0x38},      // Also set this default value to pAd->HwCfg.BbpTuning.R66CurrentValue at initial
    {BBP_R69,       0x12},
    {BBP_R70,       0xa},
    {BBP_R73,       0x10},
    {BBP_R81,       0x37},
    {BBP_R82,       0x62},
    {BBP_R83,       0x6A},
    {BBP_R84,       0x99},
    {BBP_R86,       0x00},
    {BBP_R91,       0x04},
    {BBP_R92,       0x00},
    {BBP_R103,      0x00},      // Disable DC-Filter        
    {BBP_R105,      0x05},      // Add MLD for 2 stream case
    {BBP_R106,      0x35},      // for ShortGI throughput
};
#define NUM_BBP_REG_PARMS   (sizeof(BBPRegTable) / sizeof(REG_PAIR))

//
// Initialize HW versoin
//
VOID InitHwVersion(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    {
        pAd->HwCfg.HwArchitectureVersion.eFuseVersion = eFuseArchitectureVersion_Old;
        pAd->HwCfg.HwArchitectureVersion.RfVersion = RfArchitectureVersion_Old;
        pAd->HwCfg.HwArchitectureVersion.BbVersion = BbArchitectureVersion_Old;
        pAd->HwCfg.HwArchitectureVersion.MacVersion = MacArchitectureVersion_Old;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

ULONG GET_ASIC_VERSION_ID(
    IN  PMP_ADAPTER   pAd
    )
{   
/*
    //new chip needs to read MAC register address 0x0000 to make sure what device it is.
    RTUSBReadMACRegister(pAd, (SYS_CTRL_REG_BASE + 0x0000), &pAd->HwCfg.MACVersion);

    //MAC version should be read successfully by pervious RTUSBReadMACRegister
    //or IS_RT6X9X will not work.
    if(IS_RT6X9X(pAd))
    {
        WLAN_FUN_CTRL_STRUC WlanFunCtrl = {0};

        DBGPRINT(RT_DEBUG_TRACE, ("%s: New MAC layout, enable Wlan before register read/write\n", __FUNCTION__));

        // ShiuanWen mark for MTK_OID_N5_QUERY_USB_RESET_BULK_IN after AsicRadioOff function
        //RTUSBReadMACRegister(pAd, WLAN_FUN_CTRL, (PULONG)&WlanFunCtrl);
        //WlanFunCtrl.field.WLAN_CLK_EN = 1;
        //WlanFunCtrl.field.WLAN_EN = 0;
        //RTUSBWriteMACRegister(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);

        return (SYS_CTRL_REG_BASE + 0x0000);
    }
    else
    {
        RTUSBReadMACRegister(pAd, MAC_CSR0, &pAd->MACVersion);
    }

    return (MAC_CSR0);
*/
    return 0;   // need to implement later.
}

//
// Initialize VHT control
//
VOID VhtInitCtrl(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //2 TODO

    if (VHT_NIC(pAd))
    {       
        pPort->CommonCfg.DesiredVhtPhy.bVhtEnable = TRUE;
        pPort->SoftAP.ApCfg.bVhtEnable = TRUE;
        
        VhtInittMimoCtrlField(pAd);
        VhtInitCapIE(pAd, pPort);
        VhtInitOpIE(pAd, pPort);
        VhtInitExtBssLoadIE(pAd, pPort);

        DBGPRINT(RT_DEBUG_TRACE, ("%s: VHT NIC, enable VHT capability.\n", __FUNCTION__));
    }
    else
    {
        pPort->CommonCfg.DesiredVhtPhy.bVhtEnable = FALSE;
        pPort->SoftAP.ApCfg.bVhtEnable = FALSE;

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Non-VHT NIC, disable VHT capability.\n", __FUNCTION__));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}


//
// Set Rx antenna based on the number of Rx paths
//
VOID BbSetRxAntByRxPath(
    IN PMP_ADAPTER pAd)
{
    UCHAR BbValue = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    if (EXT_BB_CAPABLE(pAd))
    {
        //2 TODO: add
        //2 TODO: pAd->StaCfg.BBPR3???
    }
    else
    {
        RTUSBReadBBPRegister(pAd, BBP_R3, &BbValue);
        
        switch (pAd->HwCfg.Antenna.field.RxPath)
        {
            case 1: 
            {
                BbValue = ((BbValue & ~0x18) | 0x00); // 1R
            }
            break;

            case 2: 
            {
                BbValue = ((BbValue & ~0x18) | 0x08); // 2R
            }
            break;

            case 3: 
            {
                BbValue = ((BbValue & ~0x18) | 0x10); // 3R
            }
            break;

            default: 
            {
                BbValue = ((BbValue & ~0x18) | 0x00); // 1R
            }
            break;
        }

        RTUSBWriteBBPRegister(pAd, BBP_R3, BbValue);
        pAd->StaCfg.BBPR3 = BbValue;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Get the index of the VHT rate table
//
ULONG GetVhtRateTableIdx(
    IN PMP_ADAPTER pAd, 
    IN ULONG Bw, // BW_XXX
    IN ULONG Nss, // NSS_XXX
    IN ULONG ShortGI, // GI_XXX
    IN ULONG MCS) // MCS_XXX
{
    ULONG rateIdx = 0;
    
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: Nss = %d, BW = %d, GI = %d, MCS = %d, NUM_RATE_ENTRIES_OF_VHT_RATE_TABLE = %d\n", 
        __FUNCTION__, 
        Nss, 
        Bw, 
        ShortGI, 
        MCS, 
        NUM_RATE_ENTRIES_OF_VHT_RATE_TABLE));

    rateIdx = ((Bw * BW_SEG_OF_VHT_RATE_TABLE) + 
                        (Nss * NSS_SEG_OF_VHT_RATE_TABLE) + 
                        (ShortGI * GI_SEG_OF_VHT_RATE_TABLE) + 
                        (MCS));

    if (rateIdx >= NUM_RATE_ENTRIES_OF_VHT_RATE_TABLE)
    {
        rateIdx = 0;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));

    return rateIdx;
}


//
// Initialize the Rx AGC VGA tuning table
//
VOID BbInitRxAgcVgaTuningTable(
    IN PMP_ADAPTER pAd)
{
    UCHAR BbValue = 0;

#if DBG
    UCHAR idx = 0;
#endif // DBG
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));   
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

/*
    ========================================================================

    Routine Description:
        Cipher decoder used in debug

    Arguments:
        ULONG algorithm                     Algorithm type

    Return Value:
        LPSTR                               String of  cipher algorithm

    IRQL

    Note:

    ========================================================================
*/
LPSTR DecodeCipherName (ULONG algorithm)
{
    LPSTR retval = "";
    switch (algorithm) 
    {
        case CIPHER_NONE:           retval = "CIPHER_NONE";     break;
        case CIPHER_WEP64:          retval = "CIPHER_WEP64";        break;
        case CIPHER_WEP128:         retval = "CIPHER_WEP128";       break;
        case CIPHER_TKIP:           retval = "CIPHER_TKIP";     break;
        case CIPHER_AES:            retval = "CIPHER_AES";          break;
        case CIPHER_WAPI:           retval = "CIPHER_WAPI";     break;      
        case CIPHER_TKIP_NO_MIC:    retval = "CIPHER_TKIP_NO_MIC";  break;
        case CIPHER_SMS4:           retval = "CIPHER_SMS4";     break;

        default:
            DBGPRINT(RT_DEBUG_ERROR, ("**********DecodeCipherName: UNKNOWN algorithm  (algorithm=0x%x)************\n", algorithm));
            retval = "***UNKNOWN Alogorithm***";
    }
    return(retval);
}

VOID AsicRadioOn(
    IN PMP_ADAPTER pAd)
{
//  WPDMA_GLO_CFG_STRUC DmaCfg;
    ULONG                MACValue = 0;
//  BOOLEAN         brc;
    UINT                 RetryRound = 0;
    UINT                 i = 0; 

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeGetCurrentIrql() != PASSIVE_LEVEL, KeGetCurrentIrql() = %d\n", __FUNCTION__, KeGetCurrentIrql()));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE,("%s ==>\n",__FUNCTION__));
    //
    // Acquire a spinlock to make sure AsicRadioOn/AsicRadioOff would not be intercept mutually.
    //
    while (TRUE)
    {
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        {
            DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] NIC is absent !!!\n",__FUNCTION__,__LINE__));
            return;
        }
        NdisAcquireSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
        if (pAd->HwCfg.RadioStatusIsChanging == FALSE)
        {
            pAd->HwCfg.RadioStatusIsChanging = TRUE;
            NdisReleaseSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
            break;
        }
        else
        {
            NdisReleaseSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
            NdisCommonGenericDelay(500);                    
            DBGPRINT(RT_DEBUG_TRACE,("wait to AsicRadioOn. #%d",i++));          
        }
    }

    // 4. Clear idle flag
    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

    //
    // start bulkin pipes, but do not send IRP immediately 
    // because flag fRTMP_ADAPTER_IDLE_RADIO_OFF have not been cleared yet
    //
    NdisCommonStartRx(pAd);


   

//  Delay_us(10000);

    //
    // Send
    //
    //SendRadioOnOffCMD(pAd, PWR_RadioOff);
    SendPowerManagementCMD(pAd, PM5, PWR_PmOff, pAd->HwCfg.CurrentAddress, 0, 1);

//  Delay_us(200000);

    //
    // release the flag that AsicRadioOn/AsicRadioOff in another thread can continue.
    //
    NdisAcquireSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
    pAd->HwCfg.RadioStatusIsChanging = FALSE;
    NdisReleaseSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
    //
    DBGPRINT(RT_DEBUG_TRACE,("<== %s\n",__FUNCTION__));

}

VOID RTMPSetDefaultCipher(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    ULONG   index = 0;
    ULONG   AuthMode;

    // This OID will be set in VWiFi AP, STA and VWiFi STA(P2P)
    if ((pPort->PortSubtype == PORTSUBTYPE_STA) || 
        (pPort->PortSubtype == PORTSUBTYPE_P2PClient) || 
        (pPort->PortSubtype == PORTSUBTYPE_VwifiAP) ||
        (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        AuthMode = pPort->PortCfg.AuthMode;
    }
    else
    {
        AuthMode = pPort->CommonCfg.APPseudoSTAConfig.AuthMode;
    }
    
    switch (AuthMode)
    {
        case DOT11_AUTH_ALGO_80211_OPEN:
            pPort->PortCfg.UnicastCipherAlgorithm = DOT11_CIPHER_ALGO_WEP;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP104;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP40;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_NONE;
            break;

        case DOT11_AUTH_ALGO_80211_SHARED_KEY:
            ASSERT(pAd->StaCfg.BssType == dot11_BSS_type_infrastructure);
            pPort->PortCfg.UnicastCipherAlgorithm = DOT11_CIPHER_ALGO_WEP;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP104;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP40;         
            break;

        case DOT11_AUTH_ALGO_WPA:
        case DOT11_AUTH_ALGO_WPA_PSK:
        case DOT11_AUTH_ALGO_RSNA:
        case DOT11_AUTH_ALGO_RSNA_PSK:
        case Ralink802_11AuthModeWPA1WPA2:
        case Ralink802_11AuthModeWPA1PSKWPA2PSK:
            pPort->PortCfg.UnicastCipherAlgorithm = DOT11_CIPHER_ALGO_CCMP;
            pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_CCMP;

            if (pAd->StaCfg.BssType == dot11_BSS_type_infrastructure)
            {
                pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_TKIP;
                pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP104;
                pPort->PortCfg.MulticastCipherAlgorithmList[index++] = DOT11_CIPHER_ALGO_WEP40;
            }
            break;

        default:
            break;
    }

    pPort->PortCfg.MulticastCipherAlgorithmCount = index;
    if (index > 1)
        pPort->PortCfg.MulticastCipherAlgorithm = DOT11_CIPHER_ALGO_NONE;
    else
        pPort->PortCfg.MulticastCipherAlgorithm = pPort->PortCfg.MulticastCipherAlgorithmList[0];
    DBGPRINT(RT_DEBUG_TRACE, ("%s: pPort->PortCfg.MulticastCipherAlgorithmCount = %d\n", __FUNCTION__, pPort->PortCfg.MulticastCipherAlgorithmCount));
}


VOID RTMPSetDefaultAuthAlgo(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    if ((pPort->PortSubtype == PORTSUBTYPE_P2PClient) ||
        (pPort->PortSubtype == PORTSUBTYPE_STA))
    {
        if (pAd->StaCfg.BssType == dot11_BSS_type_infrastructure)
        {
            pPort->PortCfg.AuthMode = DOT11_AUTH_ALGO_RSNA;
        }
        else
        {
            pPort->PortCfg.AuthMode = DOT11_AUTH_ALGO_RSNA_PSK;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s: (pAd->OpMode == OPMODE_STA) (pPort->PortCfg.AuthMode = %s)\n", 
            __FUNCTION__, decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
    }
    else if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // do nothing.
        DBGPRINT(RT_DEBUG_TRACE, ("%s: (pAd->OpMode == OPMODE_AP) do nothing.\n", __FUNCTION__));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Other Port subtype = %d. Port number =%d \n", __FUNCTION__, pPort->PortSubtype, pPort->PortNumber));
    }


    // Set the default cipher depending on the new authentication algorithm selected.
    RTMPSetDefaultCipher(pAd,pPort);
}

/*
    ========================================================================
    
    Routine Description:
        Reset NIC from error

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    Note:
        Reset NIC from error state
        
    ========================================================================
*/
VOID    
NICResetFromError(
    IN  PMP_ADAPTER   pAd
    )
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
#if 1
    NdisCommonGenericDelay(5000);
    
    // Init MT7603 CRs
    //MT7603InitMAC(pAd);
    mt_mac_init(pAd);

    // Init WLan Table
    //WTBLInit(pAd);
    mt_hw_tb_init(pAd, FALSE);

    MtAsicSetDevMac(pAd, pAd->HwCfg.CurrentAddress, 0);

    // Set BW
    MtAsicSetBW(pAd, BW_20);

    // Set Tx Stream
    MtAsicSetTxStream(pAd, pPort->CommonCfg.TxStream);

    // Set Rx Stream
    MtAsicSetRxStream(pAd, pPort->CommonCfg.RxStream);

    // Enable Tx/Rx
    MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

    MtAsicSetRxFilter(pAd);

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
#else
    NICInitializeAsic(pAd);
//#ifdef    INIT_FROM_EEPROM
    NICInitAsicFromEEPROM(pAd);
//#endif
    
    //
    // Post-process the BBP registers based on the chip model
    // N.B. the PostBBPInitialization function should be called after the NICInitAsicFromEEPROM function.
    // (reference the pAd->HwCfg.Antenna member)
    //
    PostBBPInitialization(pAd);

    //
    // Post-process the RF registers based on the chip model
    //
    PostRFInitialization(pAd);

    //
    // Initialize the desired TSSI table
    //
    InitDesiredTSSITable(pAd);

    //
    // Initialize VHT control
    //
    VhtInitCtrl(pAd);

    //
    // Initialize PPAD
    //
    InitPPAD(pAd);

    //
    // Temperature compensation, initialize the lookup table
    //
    DBGPRINT(RT_DEBUG_TRACE, ("IS_RT5392 = %d, bAutoTxAgcG = %d\n", IS_RT5392(pAd), pAd->HwCfg.bAutoTxAgcG));
    if (IS_RT5392(pAd) && pAd->HwCfg.bAutoTxAgcG)
    {
        InitLookupTableForGBand(pAd);
    }
    else if ((IS_RT5592(pAd) || (IS_RT85592(pAd)))
            && pAd->HwCfg.bAutoTxAgcG)
    {
        InitLookupTableForGBand(pAd);
        InitLookupTableForABand(pAd);
    }

    //
    // Initialize the frequency calibration
    //
    InitFrequencyCalibration(pAd);
    
    PowerOnFullCalibration(pAd);
    
    RTUSBWriteHWMACAddress(pAd);

    //
    // Initialize the transmit beamforming control
    //
    InitTxBfCtrl(pAd);
    
#endif
}

/**
 * This routine will free memory which has been allocated under pAd
 *
 * @param pAd pointer to MP_ADAPTER structure.
 *
 * @return None
 */
VOID    ReleaseAdapterInternalBlock(
    IN  PMP_ADAPTER   pAd)
{
    if (pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd is NULL\n", __FUNCTION__));  
        return;
    }

    //free support phy type array
    if(pAd->StaCfg.pSupportedPhyTypes !=NULL)
    {
        PlatformFreeMemory(pAd->StaCfg.pSupportedPhyTypes, pAd->StaCfg.SupportedPhyTypesLen);
        pAd->StaCfg.pSupportedPhyTypes = NULL;
    }

    // free for P2PCtrl        
    if (pAd->pP2pCtrll != NULL)
    {
        PlatformFreeMemory(pAd->pP2pCtrll, sizeof(P2P_CTRL_T));
        pAd->pP2pCtrll = NULL;
    }

    // 
    // free for pRxCfg        
    //
    if (pAd->pRxCfg != NULL)
    {
        PlatformFreeMemory(pAd->pRxCfg, sizeof(RX_CONFIG));
        pAd->pRxCfg = NULL;
    }

    // 
    // free for pTxCfg        
    //
    if (pAd->pTxCfg != NULL)
    {
        PlatformFreeMemory(pAd->pTxCfg, sizeof(TX_CONFIG));
        pAd->pTxCfg = NULL;
    }

    // 
    // free for pHifCfg        
    //
    if (pAd->pHifCfg != NULL)
    {
        if(pAd->pHifCfg->AdapterDesc != NULL)
        {
            PlatformFreeMemory(pAd->pHifCfg->AdapterDesc, pAd->pHifCfg->AdapterDescLen);
        }

        PlatformFreeMemory(pAd->pHifCfg, sizeof(USB_CONFIG_T));
        pAd->pHifCfg = NULL;
    }

    // 
    // free for pNicCfg        
    //
    if (pAd->pNicCfg != NULL)
    {
        // free for added connection probility 
        if (pAd->pNicCfg->OutBufferForSendProbeReq != NULL)
        {
            PlatformFreeMemory(pAd->pNicCfg->OutBufferForSendProbeReq, MAX_VIE_LEN);
            pAd->pNicCfg->OutBufferForSendProbeReq = NULL;
        }
    
        PlatformFreeMemory(pAd->pNicCfg, sizeof(NIC_CONFIG));
        pAd->pNicCfg = NULL;
    }
}

/**
 * This routine will free Tx/Rx/SpinLock/Adapter memory resource.
 *
 * @param pAd pointer to MP_ADAPTER structure.
 *
 * @return None
 */
VOID    ReleaseAdapter(
    IN  PMP_ADAPTER   pAd)
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)

    DBGPRINT(RT_DEBUG_TRACE, ("---> ReleaseAdapter\n"));

    NdisCommonFreeNicRecv(pAd);

    NdisCommonFreeNICTransmit(pAd);
    
    NdisCommonDeallocateAllSpinLock(pAd);

    ReleaseAdapterInternalBlock(pAd);
        
    // Remove the Device Object
    PlatformFreeMemory(pAd, sizeof(MP_ADAPTER));
    DBGPRINT(RT_DEBUG_TRACE, ("<--- ReleaseAdapter\n"));
#endif    
}

/*
    ========================================================================
    
    Routine Description:
        Load 8051 firmware RT2870.BIN file into MAC ASIC

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        NDIS_STATUS_SUCCESS         firmware image load ok
        NDIS_STATUS_FAILURE         image not found

    IRQL = PASSIVE_LEVEL
        
    ========================================================================
*/
NDIS_STATUS NICLoadFirmware(
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

#if IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
//TODO
#elif IS_BUS_TYPE_USB(DEV_BUS_TYPE)

    // Set DMA scheduler to Bypass mode.
    // Enable Tx/Rx
    CRStatus =HW_IO_READ32(pAd, UDMA_WLCFG_0, &U3DMAWLCFG.word);
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_ERROR,("1 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
    U3DMAWLCFG.field.WL_TX_EN = 1;
    U3DMAWLCFG.field.WL_RX_EN = 1;
    U3DMAWLCFG.field.WL_RX_MPSZ_PAD0 = 1;
    DBGPRINT(RT_DEBUG_ERROR,("2 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
    CRStatus = HW_IO_WRITE32(pAd, UDMA_WLCFG_0, U3DMAWLCFG.word);
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }

#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)
//TODO
#endif

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
    DBGPRINT(RT_DEBUG_ERROR,("2 SchedulerRegister4 = %x\n", SchedulerRegister4.word));
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
        {
            DBGPRINT(RT_DEBUG_ERROR,("FW SUCCESS Run\n"));
            break;
        }
        
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
    CRStatus = USBAccessCR(pAd, CR_READ, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    MACValue |= 0x00000100;
    CRStatus = USBAccessCR(pAd, CR_WRITE, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }

    CRStatus = USBAccessCR(pAd, CR_READ, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
    if (CRStatus != STATUS_SUCCESS)
    {
        return NDIS_STATUS_FAILURE;
    }
    MACValue &= 0xFFFFFEFF;
    CRStatus = USBAccessCR(pAd, CR_WRITE, 0x50000594, (PUCHAR)&MACValue, sizeof(ULONG));
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


VOID RTMPInitPhyType(IN PMP_ADAPTER pAd)
{
    UCHAR Index;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    //
    // PhyMib[0] = 802.11b
    //
    pAd->pNicCfg->PhyMIB[0].PhyID = 0;

    pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[0] = dot11_phy_type_hrdsss;
    pAd->pNicCfg->PhyMIB[0].PhyType = dot11_phy_type_hrdsss;

    pAd->pNicCfg->PhyMIB[0].OperationalRateSet.ucRateSet[0] = 2;
    pAd->pNicCfg->PhyMIB[0].OperationalRateSet.ucRateSet[1] = 4;
    pAd->pNicCfg->PhyMIB[0].OperationalRateSet.ucRateSet[2] = 11;
    pAd->pNicCfg->PhyMIB[0].OperationalRateSet.ucRateSet[3] = 22;
    pAd->pNicCfg->PhyMIB[0].OperationalRateSet.uRateSetLength = 4;
    
    pAd->pNicCfg->PhyMIB[0].ActiveRateSet.uRateSetLength = 0;

    pAd->pNicCfg->PhyMIB[0].BasicRateSet.ucRateSet[0] = 2;
    pAd->pNicCfg->PhyMIB[0].BasicRateSet.ucRateSet[1] = 4;
    pAd->pNicCfg->PhyMIB[0].BasicRateSet.ucRateSet[2] = 11;
    pAd->pNicCfg->PhyMIB[0].BasicRateSet.ucRateSet[3] = 22;
    pAd->pNicCfg->PhyMIB[0].BasicRateSet.uRateSetLength = 4;

    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedTxDataRatesValue[0] = 2;
    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedTxDataRatesValue[1] = 4;
    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedTxDataRatesValue[2] = 11;
    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedTxDataRatesValue[3] = 22;

    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedRxDataRatesValue[0] = 2;
    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedRxDataRatesValue[1] = 4;
    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedRxDataRatesValue[2] = 11;
    pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedRxDataRatesValue[3] = 22;

    for (Index = 4; Index < MAX_NUM_SUPPORTED_RATES_V2; Index++) 
    {
        pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedTxDataRatesValue[Index] = 0;
        pAd->pNicCfg->PhyMIB[0].SupportedDataRatesValue.ucSupportedRxDataRatesValue[Index] = 0;
    }
    
    if((IS_DUAL_BAND_NIC(pAd) && (pAd->HwCfg.DisableABandFromEEPROM != TRUE)))
    {

#if 0
        //
        // PhyMib[3] = 802.11n
        //
        pAd->pNicCfg->PhyMIB[3].PhyID = 3;

        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[3] = dot11_phy_type_ht;
        pAd->pNicCfg->PhyMIB[3].PhyType = dot11_phy_type_ht;

        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[5] = 18;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[6] = 24;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[7] = 36;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[8] = 48;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[9] = 72;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[10] = 96;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.ucRateSet[11] = 108;
        pAd->pNicCfg->PhyMIB[3].OperationalRateSet.uRateSetLength = 12;

        pAd->pNicCfg->PhyMIB[3].ActiveRateSet.uRateSetLength = 0;
    
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.ucRateSet[5] = 24;
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.ucRateSet[6] = 48;
        pAd->pNicCfg->PhyMIB[3].BasicRateSet.uRateSetLength = 7;
    
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[11] = 108;
    
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[11] = 108;
    
        for (Index = 12; Index < MAX_NUM_SUPPORTED_RATES_V2; Index++) 
        {
            pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedTxDataRatesValue[Index] = 0;
            pAd->pNicCfg->PhyMIB[3].SupportedDataRatesValue.ucSupportedRxDataRatesValue[Index] = 0;
        }
        pAd->pNicCfg->PhyMIB[3].Channel = 1; 
#endif
        
        //
        // PhyMib[2] = 802.11g
        //
        pAd->pNicCfg->PhyMIB[2].PhyID = 2;
    
        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[2] = dot11_phy_type_erp;
        pAd->pNicCfg->PhyMIB[2].PhyType = dot11_phy_type_erp;
    
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[5] = 18;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[6] = 24;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[7] = 36;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[8] = 48;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[9] = 72;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[10] = 96;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[11] = 108;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.uRateSetLength = 12;
        
        pAd->pNicCfg->PhyMIB[2].ActiveRateSet.uRateSetLength = 0;
    
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[5] = 24;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[6] = 48;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.uRateSetLength = 7;
    
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[11] = 108;
    
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[11] = 108;
    
        for (Index = 12; Index < MAX_NUM_SUPPORTED_RATES_V2; Index++) 
        {
            pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[Index] = 0;
            pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[Index] = 0;
        }
        pAd->pNicCfg->PhyMIB[2].Channel = 1; 
        
        //
        // PhyMib[1] = 802.11a
        //
        pAd->pNicCfg->PhyMIB[1].PhyID = 1;

        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[1] = dot11_phy_type_ofdm;
        pAd->pNicCfg->PhyMIB[1].PhyType = dot11_phy_type_ofdm;
        
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[0] = 12;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[1] = 18;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[2] = 24;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[3] = 36;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[4] = 48;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[5] = 72;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[6] = 96;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[7] = 108;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.uRateSetLength = 8;

        pAd->pNicCfg->PhyMIB[1].ActiveRateSet.uRateSetLength = 0;

        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[0] = 12;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[1] = 24;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[2] = 48;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.uRateSetLength = 3;

        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[0] = 12;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[1] = 18;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[2] = 24;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[3] = 36;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[4] = 48;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[5] = 72;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[6] = 96;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[7] = 108;

        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[0] = 12;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[1] = 18;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[2] = 24;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[3] = 36;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[4] = 48;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[5] = 72;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[6] = 96;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[7] = 108;

        for (Index = 8; Index < MAX_NUM_SUPPORTED_RATES_V2; Index++) 
        {
            pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[Index] = 0;
            pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[Index] = 0;
        }
        pAd->pNicCfg->PhyMIB[1].Channel = 36;

        pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries = 3;
        pPort->CommonCfg.DefaultPhyId = 2;
    }
    //BGN mix
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s:Support phy id bgn mix.\n", __FUNCTION__));
#if 0       
        //
        // PhyMib[2] = 802.11n
        //
        pAd->pNicCfg->PhyMIB[2].PhyID = 2;

        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[2] = dot11_phy_type_ht;
        pAd->pNicCfg->PhyMIB[2].PhyType = dot11_phy_type_ht;

        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[5] = 18;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[6] = 24;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[7] = 36;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[8] = 48;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[9] = 72;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[10] = 96;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.ucRateSet[11] = 108;
        pAd->pNicCfg->PhyMIB[2].OperationalRateSet.uRateSetLength = 12;

        pAd->pNicCfg->PhyMIB[2].ActiveRateSet.uRateSetLength = 0;
    
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[5] = 24;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.ucRateSet[6] = 48;
        pAd->pNicCfg->PhyMIB[2].BasicRateSet.uRateSetLength = 7;
    
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[11] = 108;
    
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[11] = 108;
    
        for (Index = 12; Index < MAX_NUM_SUPPORTED_RATES_V2; Index++) 
        {
            pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedTxDataRatesValue[Index] = 0;
            pAd->pNicCfg->PhyMIB[2].SupportedDataRatesValue.ucSupportedRxDataRatesValue[Index] = 0;
        }
        pAd->pNicCfg->PhyMIB[2].Channel = 1; 
#endif
        //
        // PhyMib[2] = 802.11g
        //      
        pAd->pNicCfg->PhyMIB[1].PhyID = 1;
    
        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[1] = dot11_phy_type_erp;
        pAd->pNicCfg->PhyMIB[1].PhyType = dot11_phy_type_erp;
    
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[5] = 18;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[6] = 24;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[7] = 36;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[8] = 48;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[9] = 72;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[10] = 96;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.ucRateSet[11] = 108;
        pAd->pNicCfg->PhyMIB[1].OperationalRateSet.uRateSetLength = 12;
        
        pAd->pNicCfg->PhyMIB[1].ActiveRateSet.uRateSetLength = 0;
    
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[0] = 2;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[1] = 4;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[2] = 11;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[3] = 22;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[4] = 12;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[5] = 24;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.ucRateSet[6] = 48;
        pAd->pNicCfg->PhyMIB[1].BasicRateSet.uRateSetLength = 7;
    
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[11] = 108;
    
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[0] = 2;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[1] = 4;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[2] = 11;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[3] = 22;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[4] = 12;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[5] = 18;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[6] = 24;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[7] = 36;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[8] = 48;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[9] = 72;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[10] = 96;
        pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[11] = 108;
    
        for (Index = 12; Index < MAX_NUM_SUPPORTED_RATES_V2; Index++) 
        {
            pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedTxDataRatesValue[Index] = 0;
            pAd->pNicCfg->PhyMIB[1].SupportedDataRatesValue.ucSupportedRxDataRatesValue[Index] = 0;
        }
        
        pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries = 2;
        pPort->CommonCfg.DefaultPhyId = 1;
    }
        
}


/*
    ========================================================================
    
    Routine Description:
        Initialize port configuration structure

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    Note:
        
    ========================================================================
*/
VOID    UserCfgInit(
    IN  PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit\n"));    

    //
    //  part I. intialize common configuration
    //

    DBGPRINT(RT_DEBUG_TRACE, ("-->sizeof(FRAME_ADDBA_RSP)=%d,     \n",sizeof(FRAME_ADDBA_RSP)));    
    
    pAd->pHifCfg->BulkOutComplete= 0;
    pAd->pHifCfg->BulkOutCompleteOther= 0;
    pAd->pHifCfg->BulkOutCompleteCancel = 0;
    pAd->pHifCfg->BulkInReq= 0;
    pAd->pHifCfg->BulkInComplete = 0;
    pAd->pHifCfg->BulkInCompleteFail = 0;

    pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelayFactor = 5;
    pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt = 900;
    pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveDwell = 20;    
    pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveDwell = 10;
    pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveTotalPerChannel = 200;
    pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveTotalPerChannel = 20;
    pPort->CommonCfg.ScanParameter.Dot11OBssScanActivityThre = 25;
    pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelay = (pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt * pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelayFactor);

    pAd->TrackInfo.SameRxByteCount = 0;
    pAd->TrackInfo.InfraOnSameRxByteCount = 20;
    pAd->TrackInfo.IdleOnSameRxByteCountUpper = 700;
    pAd->TrackInfo.IdleOnSameRxByteCountLower = 600;
    pAd->TrackInfo.GoOnSameRxByteCount = 10;
    if (pAd->LogoTestCfg.OnTestingWHQL)
    {
        pAd->TrackInfo.InfraOnSameRxByteCount = (20 * 5);
        pAd->TrackInfo.IdleOnSameRxByteCountUpper = (700 * 5);
        pAd->TrackInfo.IdleOnSameRxByteCountLower = (600 * 5);
        pAd->TrackInfo.GoOnSameRxByteCount = (10 * 5);
    }

    pAd->HwCfg.Antenna.word = 0; 
    pPort->BBPCurrentBW = BW_20;
    pAd->UiCfg.bPromiscuous = FALSE;
    pAd->pHifCfg->bUsbTxBulkAggre = 0;           
    pAd->HwCfg.LedCntl.word = 0;
    pAd->HwCfg.LedIndicatorStregth = 0xFF;    // init as unsed value to ensure driver will set to MCU once.
    pAd->TrackInfo.CountDowntoPsm = 0;
    
    pPort->CommonCfg.MaxPktOneTxBulk = 2;
    pPort->CommonCfg.TxBulkFactor = 1;
    pPort->CommonCfg.RxBulkFactor =1;
    pAd->HwCfg.bAutoTxAgcA = FALSE;           // Default is OFF
    pAd->HwCfg.bAutoTxAgcG = FALSE;           // Default is OFF
    pAd->HwCfg.FreqCalibrationCtrl.bEnableFrequencyCalibration = FALSE; // Off by default
    pAd->HwCfg.RfIcType = RFIC_2820;

    // Init timer for reset complete event
    pPort->Channel = 1;
    pPort->CentralChannel = 1;
    pPort->CommonCfg.bRdg = FALSE;
    pAd->pHifCfg->bForcePrintTX = FALSE;
    pAd->pHifCfg->bForcePrintRX = FALSE;
    pPort->CommonCfg.Dsifs = 10;      // in units of usec 
    pPort->CommonCfg.PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
    pPort->CommonCfg.TxPower = 100; //mW
    pPort->CommonCfg.TxPowerPercentage = 0xffffffff; // AUTO
    pPort->CommonCfg.TxPowerDefault = 0xffffffff; // AUTO
    pPort->CommonCfg.TxPreamble = Rt802_11PreambleAuto; // use Long preamble on TX by defaut
    pPort->CommonCfg.bUseZeroToDisableFragment = FALSE;
    pPort->CommonCfg.RtsThreshold = 2347;
    pPort->CommonCfg.FragmentThreshold = 2346;
    pPort->CommonCfg.UseBGProtection = 0;    // 0: AUTO
    pPort->CommonCfg.bEnableTxBurst = 1;      
    pPort->CommonCfg.PhyMode = 0xff;     // unknown
    pPort->CommonCfg.BandState = UNKNOWN_BAND;

    pPort->CommonCfg.RadarDetect.CSPeriod = 10;
    pPort->CommonCfg.RadarDetect.CSCount = 0;
    pPort->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;
    pPort->CommonCfg.bPiggyBackCapable = TRUE;    

    PlatformZeroMemory(&pPort->CommonCfg.HtCapability, sizeof(pPort->CommonCfg.HtCapability));
    PlatformZeroMemory(&pPort->CommonCfg.AddHTInfo, sizeof(pPort->CommonCfg.AddHTInfo));
    PlatformZeroMemory(&pPort->CommonCfg.IOTestParm, sizeof(pPort->CommonCfg.IOTestParm));
    pPort->CommonCfg.BACapability.field.MpduDensity = 0;
    pPort->CommonCfg.BACapability.field.Policy = IMMED_BA;
    pPort->CommonCfg.BACapability.field.RxBAWinLimit = 64;
    pPort->CommonCfg.BACapability.field.TxBAWinLimit = 64;

    WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, BW_20);
    WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg, MCS_15);
    WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_800);
    WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg, STBC_NONE);
    
    pPort->CommonCfg.TxRate = RATE_6;
    DBGPRINT(RT_DEBUG_TRACE, ("--> %s. BACapability = 0x%x\n",__FUNCTION__,pPort->CommonCfg.BACapability.word));    
    
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg, 0);
    
    pPort->CommonCfg.BACapability.field.MMPSmode = MMPS_ENABLE;       

    //
    // part II. intialize STA specific configuration
    //
    pPort->CommonCfg.HitCount = 0;
    pPort->CommonCfg.bCarrierAssert = FALSE;

    if(VHT_NIC(pAd))
        pPort->CommonCfg.BaLimit = 30;
    else
    pPort->CommonCfg.BaLimit = 14;

    pAd->StaCfg.Psm = PWR_ACTIVE;
    pPort->CommonCfg.BeaconPeriod = 100;     // in mSec

    // 
    // part III. AP configurations
    //

    pPort->SoftAP.ApCfg.TimBitmap  = 0;
    pPort->SoftAP.ApCfg.TimBitmap2 = 0;
    pPort->SoftAP.ApCfg.DtimCount  = 0;
    pPort->SoftAP.ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;
    pPort->SoftAP.ApCfg.AgeoutTime = MAC_TABLE_AGEOUT_TIME;

    // 802.1x port control
    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
    pAd->StaCfg.LastMicErrorTime = 0;
    pAd->StaCfg.MicErrCnt        = 0;
    pAd->StaCfg.bBlockAssoc      = FALSE;
    pAd->StaCfg.WpaState         = SS_NOTUSE;       // Handle by microsoft unless RaConfig changed it.

    // WSC state machine
    pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_NOTUSED;
    pPort->StaCfg.WscControl.WscEnProfile = FALSE;    
    pPort->StaCfg.WscControl.bConfiguredAP = FALSE;

    pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt = 0;
    pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt = 1;
    pAd->HwCfg.RxAnt.EvaluatePeriod = 0;

    pAd->StaCfg.RssiTrigger = 0;
    pAd->StaCfg.RssiSample.LastRssi[0] = 0;
    pAd->StaCfg.RssiSample.LastRssi[1] = 0;
    pAd->StaCfg.RssiSample.AvgRssi[0]  = 0;
    pAd->StaCfg.RssiSample.AvgRssiX8[0] = 0;
    pAd->StaCfg.RssiTriggerMode = RSSI_TRIGGERED_UPON_BELOW_THRESHOLD;
    pAd->StaCfg.AtimWin = 0;
    pAd->StaCfg.DefaultListenCount = 3;//default listen count;
    pAd->StaCfg.BssType = BSS_INFRA;  // BSS_INFRA or BSS_ADHOC
    pAd->StaCfg.AdhocJoiner  = FALSE;
    pAd->StaCfg.AdhocCreator = FALSE;

    // global variables mXXXX used in MAC protocol state machines
    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);

    // PHY specification
    pPort->CommonCfg.PhyMode = PHY_11ABG_MIXED;       // default PHY mode
    //MlmeInfoSetPhyMode(pAd, PHY_11BG_MIXED);   // default in 11BG mixed mode
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  // CCK use LONG preamble

    // user desired power mode
    pAd->StaCfg.WindowsPowerMode = DOT11_POWER_SAVING_NO_POWER_SAVING;
    pAd->StaCfg.WindowsBatteryPowerMode = DOT11_POWER_SAVING_NO_POWER_SAVING;
    pAd->StaCfg.bWindowsACCAMEnable = FALSE;
    
        // init 11w pmf control
    PlatformZeroMemory(&pAd->StaCfg.PmfCfg.PmfControl, sizeof(PMF_CONTROL));
    
#ifdef NDIS51_MINIPORT
    pAd->StaCfg.WindowsPowerProfile = NdisPowerProfileAcOnLine; // Ndis802_11PowerModeFast_PSP;
#endif

    // Patch for Ndtest
    pAd->StaCfg.ScanCnt = 0;

    // init to zero
    pAd->StaCfg.RssiDelta = 0;

    pAd->StaCfg.FastRoamingSensitivity = 0;
    
    pAd->StaCfg.bHwRadio  = TRUE; // Default Hardware Radio status is On
    pAd->StaCfg.bSwRadio  = TRUE; // Default Software Radio status is On
    pAd->StaCfg.bSwRadioOff = FALSE; //For Vita read registry only.
    pAd->StaCfg.bRadio    = TRUE; // bHwRadio && bSwRadio
    pAd->StaCfg.bHardwareRadio = FALSE;     // Default is OFF
    pAd->StaCfg.bHwPBC = FALSE;
    pAd->StaCfg.LastSwRadio = 0xFFFFFFFF; // Initial Last Radio status as undefined.
    pAd->StaCfg.LastHwRadio = 0xFFFFFFFF; // Initial Last Radio status as undefined.
    pAd->StaCfg.LinkQualitySetting = 0x0;

    // Nitro mode control
    pAd->StaCfg.bAutoReconnect = TRUE;

    // Save the init time as last scan time, the system should do scan after 2 seconds.
    // This patch is for driver wake up from standby mode, system will do scan right away.
    pAd->StaCfg.LastScanTime = 0;
    pAd->StaCfg.SmartScan = FALSE;
    pAd->StaCfg.LastScanTimeFromNdis = 0;

    // Default for extra information is not valid
    pAd->UiCfg.ExtraInfo = EXTRA_INFO_CLEAR;
    
    // Default Config change flag
    pAd->pNicCfg->bConfigChanged = FALSE;

    // 
    // part III. AP configurations
    //

    pPort->SoftAP.ApCfg.TimBitmap  = 0;
    pPort->SoftAP.ApCfg.TimBitmap2 = 0;
    pPort->SoftAP.ApCfg.DtimCount  = 0;
    pPort->SoftAP.ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;

    //
    // part IV. others
    //
    // dynamic BBP R66:sensibity tuning to overcome background noise
    pAd->HwCfg.BbpTuning.bEnable                = TRUE;  
    pAd->HwCfg.BbpTuning.FalseCcaLowerThreshold = 100;
    pAd->HwCfg.BbpTuning.FalseCcaUpperThreshold = 512;   
    pAd->HwCfg.BbpTuning.R66Delta               = 4;
    pAd->Mlme.bEnableAutoAntennaCheck = TRUE;
    //
    // Also initial R66CurrentValue, XmitResumeMsduTransmission might use this value.
    // if not initial this value, the default value will be 0.
    //
    pAd->HwCfg.BbpTuning.R66CurrentValue = 0x38;
    
    //
    // Use all bandwidth by default
    //

    pPort->CommonCfg.MPDUMaxLength = MAX_AGGREGATION_SIZE;
    
    //
    // OpModeCapability
    //
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if(NDIS_WIN8_ABOVE(pAd))
    {
        pPort->CommonCfg.OperationModeCapability.uOpModeCapability = DOT11_OPERATION_MODE_EXTENSIBLE_STATION |
                                                                    DOT11_OPERATION_MODE_NETWORK_MONITOR |
                                                                    DOT11_OPERATION_MODE_EXTENSIBLE_AP |
                                                                    DOT11_OPERATION_MODE_WFD_DEVICE |
                                                                    DOT11_OPERATION_MODE_WFD_GROUP_OWNER |
                                                                    DOT11_OPERATION_MODE_WFD_CLIENT;
    }
    else
#endif
    {
        pPort->CommonCfg.OperationModeCapability.uOpModeCapability = DOT11_OPERATION_MODE_EXTENSIBLE_STATION |
                                                                DOT11_OPERATION_MODE_NETWORK_MONITOR |
                                                                DOT11_OPERATION_MODE_EXTENSIBLE_AP ;
    }

    pPort->CommonCfg.OperationModeCapability.uReserved = 0;
    pPort->CommonCfg.OperationModeCapability.uMajorVersion = 2;
    pPort->CommonCfg.OperationModeCapability.uMinorVersion = 0;
    pPort->CommonCfg.OperationModeCapability.uNumOfTXBuffers = 0;
    pPort->CommonCfg.OperationModeCapability.uNumOfRXBuffers = RX_RING_SIZE_END_INDEX;    
    
    N6StaCfgInit(pAd, TRUE);

    //
    // Initialize the pseudo STA configuration for soft AP mode.
    //
    InitAPPseudoSTAConfig(pAd);

    //
    // By default, the station automatically selects the desired WPS AP.
    //
    pPort->StaCfg.WscControl.PreferrredWPSAPPhyType= PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION;

    // Clear PMKID cache.
    PlatformZeroMemory(&pPort->CommonCfg.PMKIDCache, sizeof(PMKID_CACHE));
    pAd->StaCfg.PMKIDCount = 0;
    PlatformZeroMemory(pAd->StaCfg.PMKIDList, (sizeof(DOT11_PMKID_ENTRY) * STA_PMKID_MAX_COUNT));

    //
    // Watch dog for TX Stall, initial TotalSendNdisPacketCount start from 1
    // 
    pAd->Counter.MTKCounters.TotalSendNdisPacketCount = 1;

    pAd->pHifCfg->UsedEndpoint = 1;   // first bulkout endpoint will be used by default  
    pAd->BBPBusycnt = 0;
    pAd->CountDowntoRadioOff = STAY_IN_AWAKE; // decrease in STAMlmePeriodicExec()
    pAd->Shutdowncnt = 0;
    pAd->bInShutdown = FALSE;
    pAd->SleepTBTT = 2;
    pPort->StaCfg.WscControl.bWPSMODE9SKIPProgress = FALSE;

    //
    // Default set BT LED status off.
    //
    pPort->CommonCfg.BTLEDStatus = BT_LED_OFF;

    pPort->StaCfg.WscControl.WscPbcEnrCount = 0;
    pPort->StaCfg.WscControl.UseUserSelectEnrollee = 0;

    pAd->bWakeupFromS3S4 = FALSE;

    pAd->LogoTestCfg.TurnOffBeaconListen = FALSE;
    pAd->LogoTestCfg.CounterForTOBL = 0;

    pAd->LogoTestCfg.WhckFirst40Secs = FALSE;
    pAd->LogoTestCfg.WhckCpuUtilTestRunning = FALSE;
    pAd->LogoTestCfg.WhckDataCntForPast40Sec = 0;
    pAd->LogoTestCfg.WhckDataCntForPast60Sec = 0;

    pAd->pNicCfg->bUpdateBssList = FALSE;

    pAd->MlmeAux.bNeedPlatformIndicateScanStatus = FALSE;
    pAd->pNicCfg->PendedDevicePowerState = NdisDeviceStateD0;

    pAd->pRxCfg->nIndicatedRxPkts = 0;

    pAd->pHifCfg->bIRPBulkOutData = FALSE;

#ifdef RTMP_INTERNAL_TX_ALC
    pAd->HwCfg.TxALCData.TssiSlope = 0;
    pAd->HwCfg.TxALCData.TssiDC0 = 0;
    pAd->HwCfg.TxALCData.TssiDC0_HVGA = 0;
    pAd->HwCfg.TxALCData.TSSI_USE_HVGA = 0;
    pAd->HwCfg.TxALCData.PowerDiffPre = 100;
#endif /* RTMP_INTERNAL_TX_ALC */

    pAd->HwCfg.bLoadingFW = FALSE;
    pAd->HwCfg.LoadingFWCount = 0;
    pPort->bStartJoin = FALSE;
    pAd->pNicCfg->OutBufferForSendProbeReqLen = 0;
    pAd->pHifCfg->bBulkInRead = FALSE;

    pPort->SoftAP.ApCfg.StartSoftApAfterScan = FALSE;
    pPort->SoftAP.ApCfg.SoftApPort = NULL;

    pAd->TrackInfo.bTxNULLFrameTest = FALSE;
    pAd->TrackInfo.bStopCMDEventReceive = FALSE;

    pPort->CommonCfg.lowTrafficThrd = 2;
    pPort->CommonCfg.TrainUpRule = 2; // 1;
    pPort->CommonCfg.TrainUpRuleRSSI = -70; // 0;
    pPort->CommonCfg.TrainUpLowThrd = 90;
    pPort->CommonCfg.TrainUpHighThrd = 110;

    /* Frequency for rate adaptation */
    pAd->ra_interval = DEF_RA_TIME_INTRVAL;
    pAd->ra_fast_interval = DEF_QUICK_RA_TIME_INTERVAL;

    DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n",__FUNCTION__));
}


NDIS_STATUS
RTMPSetRegistrySwRadioOff(
    IN PMP_ADAPTER    pAd,
    IN BOOLEAN          SwRadioOff)
{
    NDIS_CONFIGURATION_OBJECT       ConfigObject;
    NDIS_HANDLE                     RegistryConfigurationHandle = NULL;
    NDIS_CONFIGURATION_PARAMETER    Parameter;
    NDIS_STRING                     RegName = NDIS_STRING_CONST("Radio");
    NDIS_STRING                     RegNameOn = NDIS_STRING_CONST("1");
    NDIS_STRING                     RegNameOff = NDIS_STRING_CONST("0");    
    NDIS_STATUS                     ndisStatus;

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
        PlatformZeroMemory(&Parameter, sizeof(NDIS_CONFIGURATION_PARAMETER));

        Parameter.ParameterData.StringData = (SwRadioOff ?RegNameOn :RegNameOff);
        Parameter.ParameterType = NdisParameterString;

        NdisWriteConfiguration(&ndisStatus,
                                RegistryConfigurationHandle,
                                &RegName,
                                &Parameter
                                );

        //
        // Close the handle to the registry
        //
        NdisCloseConfiguration(RegistryConfigurationHandle);
    }
    else
    {
        DBGPRINT_ERR(("NdisOpenConfigurationEx failed\n"));
    }

    return ndisStatus;
}

VOID NICRestoreBBPValue(
    IN PMP_ADAPTER pAd) 
{
    UCHAR       index;
    UCHAR       Value = 0;
    ULONG       Data = 0;
    PMP_PORT  pPort = MlmeSyncGetActivePort(pAd);

    DBGPRINT(RT_DEBUG_TRACE, ("--->  NICRestoreBBPValue !!!!!!!!!!!!!!!!!!!!!!!  \n"));


    // Initialize BBP register to default value (rtmp_init.c)
    for (index = 0; index < NUM_BBP_REG_PARMS; index++)
    {
        RTUSBWriteBBPRegister(pAd, BBPRegTable[index].Register, BBPRegTable[index].Value);
    }

    // copy from (rtmp_init.c)
    if (pAd->HwCfg.MACVersion == 0x28600100)
    {
        RTUSBWriteBBPRegister(pAd, BBP_R69, 0x16);
        RTUSBWriteBBPRegister(pAd, BBP_R73, 0x12);
    }
    
    //
    // Post-process the BBP registers based on the chip model
    //
    PostBBPInitialization(pAd);

    //
    // Initialize VHT control
    //
    /// TODO: need to reset VHT IEs ?
    //VhtInitCtrl(pAd);

    //
    // Initialize PPAD
    //
    InitPPAD(pAd);

    //
    // Initialize the frequency calibration
    //
    InitFrequencyCalibration(pAd);
        
    // copy from (connect.c MlmeCntLinkUp function)
    if (INFRA_ON(pPort))
    {
        // Change to AP channel
        if(pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth==VHT_BW_80)
        {   
            UCHAR PrimaryChannelLocation=0;
            UCHAR bandwidth = BW_80;
            UCHAR CentralChannel = pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1;
            
            pPort->BBPCurrentBW = BW_80;
            GetPrimaryChannelLocation(
                    pAd,
                    pPort->Channel,
                    bandwidth,
                    CentralChannel,
                    &PrimaryChannelLocation
                    );          

            SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, bandwidth, PrimaryChannelLocation);

            DBGPRINT(RT_DEBUG_TRACE, ("%s !!!80MHz LINK UP !!! PrimaryChannelLocation =%d, Central = %d \n", 
                    __FUNCTION__,
                    PrimaryChannelLocation,
                    CentralChannel));
            }
        else if ((pPort->CentralChannel > pPort->Channel) && (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
        {   
            // Must using 40MHz.
            pPort->BBPCurrentBW = BW_40;
            SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, EXTCHA_ABOVE);

            DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower LINK UP !!! Control Channel at Below. Central = %d \n", pPort->CentralChannel ));
        }
        else if ((pPort->CentralChannel < pPort->Channel) && (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
        {   
            // Must using 40MHz.
            pPort->BBPCurrentBW = BW_40;
            SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, EXTCHA_BELOW);

            DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Upper LINK UP !!! Control Channel at UpperCentral = %d \n", pPort->CentralChannel ));
        }
        else
        {
            pPort->BBPCurrentBW = BW_20;
            SwitchBandwidth(pAd, TRUE, pPort->Channel, BW_20, EXTCHA_NONE);
            
            DBGPRINT(RT_DEBUG_TRACE, ("!!!20MHz LINK UP !!! \n" ));
        }
    }
    else if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        P2pGoSwitchChannel(pAd, pPort);
    }
 
    DBGPRINT(RT_DEBUG_TRACE, ("<---  NICRestoreBBPValue !!!!!!!!!!!!!!!!!!!!!!!  \n"));
}

VOID AsicRadioOff(
    IN PMP_ADAPTER pAd)
{
    ULONG               i = 0;

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeGetCurrentIrql() != PASSIVE_LEVEL, KeGetCurrentIrql() = %d\n", __FUNCTION__, KeGetCurrentIrql()));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s ==>\n",__FUNCTION__));

    // 
    // Acquire a spinlock to make sure AsicRadioOn/AsicRadioOff would not be intercept mutually.
    //
    while (TRUE)
    {
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        {
            DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] NIC is absent !!!\n",__FUNCTION__,__LINE__));
            return;
        }   
        
        NdisAcquireSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
        if (pAd->HwCfg.RadioStatusIsChanging == FALSE)
        {
            pAd->HwCfg.RadioStatusIsChanging = TRUE;
            NdisReleaseSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
            break;
        }
        else
        {
            NdisReleaseSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
            NdisCommonGenericDelay(500);
            #if DBG         
            DBGPRINT(RT_DEBUG_TRACE,("wait to AsicRadioOn. #%d",i++));
            #endif
        }
    }

    MT_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
    
    //SendRadioOnOffCMD(pAd, PWR_RadioOff);
    SendPowerManagementCMD(pAd, PM5, PWR_PmOn, pAd->HwCfg.CurrentAddress, 0, 1);

    Delay_us(200000);

    // Stop bulkin pipe
    NdisCommonStopRx(pAd);

    //
    // release the flag that AsicRadioOn/AsicRadioOff in another thread can continue.
    //
    NdisAcquireSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
    pAd->HwCfg.RadioStatusIsChanging = FALSE;
    NdisReleaseSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
    //
    DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n",__FUNCTION__));    
}

// IRQL = PASSIVE_LEVEL
VOID AsicRFOn(
    IN PMP_ADAPTER pAd)
{
    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeGetCurrentIrql() != PASSIVE_LEVEL, KeGetCurrentIrql() = %d\n", __FUNCTION__, KeGetCurrentIrql()));
        return;
    }  
}   

VOID RTMPSetExtRegMessageEvent(
    IN PMP_ADAPTER    pAd,
    IN ULONG            EventId,
    IN PUCHAR           Msg,
    IN ULONG            Length)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    pPort->CommonCfg.RTMessageEvent.Lock = TRUE;

    if (pPort->CommonCfg.RTMessageEvent.Reset == TRUE)
    {
        PlatformZeroMemory(pPort->CommonCfg.RTMessageEvent.MessageLog, MAX_MESSAGE_EVENT_LENGTH);
        pPort->CommonCfg.RTMessageEvent.MessageLogLength = 0;
    }

    if ((Length + sizeof(ULONG)) > MAX_MESSAGE_EVENT_LENGTH)
    {
        DBGPRINT(RT_DEBUG_WARN,("RTMPSetExtRegMessageEvent, Message full\n"));
        pPort->CommonCfg.RTMessageEvent.Reset = FALSE;
        pPort->CommonCfg.RTMessageEvent.Lock = FALSE;
            
        return;
    }
    
    pPort->CommonCfg.RTMessageEvent.MessageLogLength= Length;
    if (pPort->CommonCfg.RTMessageEvent.MessageLogLength != 0)
    {
        PlatformMoveMemory(pPort->CommonCfg.RTMessageEvent.MessageLog, Msg, Length);
    }
    else
    {
        PlatformZeroMemory(pPort->CommonCfg.RTMessageEvent.MessageLog, MAX_MESSAGE_EVENT_LENGTH);
    }
        
    pPort->CommonCfg.RTMessageEvent.Reset = FALSE;
    pPort->CommonCfg.RTMessageEvent.Lock = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetExtRegMessageEvent, Indicate Event id=%d ...... \n", EventId));
        
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
VOID    
MTInitializeCmdQ(
    IN  PCmdQ   cmdq
    )
{
    cmdq->head = NULL;
    cmdq->tail = NULL;
    cmdq->size = 0;
}

VOID
MtInitInternalCmdBuffer(
    IN PMP_ADAPTER    pAd
    )
{
    PCmdQElmt   cmdqelmt = NULL;
    PVOID pBuffer =NULL;
    UCHAR   i=0;

    InitializeQueueHeader(&pAd->Mlme.MlmeInternalCmdBufHead);
    
    for( i = 0 ; i < INTERNAL_CMD_NUM; i++ )
    {
        cmdqelmt = NULL;
        pBuffer =NULL;
        
        PlatformAllocateMemory(pAd, &cmdqelmt, sizeof(CmdQElmt));

        if (cmdqelmt == NULL)
            return;

        PlatformZeroMemory(cmdqelmt, sizeof(CmdQElmt));

        PlatformAllocateMemory(pAd, &pBuffer, INTERNAL_CMD_BUFFER_LEN);

        if (pBuffer == NULL)
        {
            PlatformFreeMemory(cmdqelmt, sizeof(CmdQElmt));
            return;
        }
        cmdqelmt->buffer = pBuffer;
        cmdqelmt->Seq =i;
        NdisAcquireSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock);
        InsertTailQueue(&pAd->Mlme.MlmeInternalCmdBufHead, cmdqelmt);     
        NdisReleaseSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock);     
    }

}

VOID
MtFreeInternalCmdBuffer(
    IN PMP_ADAPTER    pAd
    )
{
    PCmdQElmt   cmdqelmt = NULL;
    UCHAR   i=0;
    for( i = 0 ; i < INTERNAL_CMD_NUM ; i++ )
    {
        cmdqelmt = (PCmdQElmt)RemoveHeadQueue(&pAd->Mlme.MlmeInternalCmdBufHead);   
        PlatformFreeMemory(cmdqelmt->buffer, INTERNAL_CMD_BUFFER_LEN);
        PlatformFreeMemory(cmdqelmt, sizeof(CmdQElmt));
        
    }

}

PCmdQElmt
MtGetInternalCmdBuffer(
    IN PMP_ADAPTER    pAd
    )
{
    PCmdQElmt   cmdqelmt = NULL;

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
    {
        return NULL;
    }
    
    NdisAcquireSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock);
    cmdqelmt = (PCmdQElmt)RemoveHeadQueue(&pAd->Mlme.MlmeInternalCmdBufHead);    
    NdisReleaseSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock); 
    DBGPRINT(RT_DEBUG_TRACE, ("%s - %d\n", __FUNCTION__, pAd->Mlme.MlmeInternalCmdBufHead.Number));    
    return cmdqelmt;
}

VOID
MtReturnInternalCmdBuffer(
    IN PMP_ADAPTER    pAd,
    IN PCmdQElmt   cmdqelmt
    )
{  
    PlatformZeroMemory(cmdqelmt->buffer, INTERNAL_CMD_BUFFER_LEN);
    //PlatformZeroMemory(cmdqelmt, sizeof(CmdQElmt));
    cmdqelmt->bufferlength =0;
    cmdqelmt->CmdFromNdis =FALSE;
    cmdqelmt->PortNum =0;
    cmdqelmt->SetOperation =FALSE;
    cmdqelmt->command = 0;
    
    NdisAcquireSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock);
    InsertTailQueue(&pAd->Mlme.MlmeInternalCmdBufHead, cmdqelmt);     
    NdisReleaseSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock);    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - %d\n", __FUNCTION__, pAd->Mlme.MlmeInternalCmdBufHead.Number));   
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
NDIS_STATUS 
MTEnqueueInternalCmd(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT      pPort,
    IN LONG             Cmd,
    IN PVOID            Buffer,
    IN ULONG            BufferLen
    )    
{
    PCmdQElmt   cmdqelmt = NULL;

    // break this command queue if thread is terminated
    if ((pAd != NULL) &&  (pAd->TerminateThreads == TRUE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Thread is terminated. ERROR!\n", __FUNCTION__));    
        return NDIS_STATUS_FAILURE;
    }

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pPort == NULL. ERROR!\n", __FUNCTION__));    
        return NDIS_STATUS_FAILURE;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s   cmd = %x PortNum = %d", __FUNCTION__, Cmd, pPort->PortNumber));

    cmdqelmt = MtGetInternalCmdBuffer(pAd);

    
    if(cmdqelmt ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Halt in progress\n", __FUNCTION__));    
        return NDIS_STATUS_FAILURE;
    }
    cmdqelmt->command = Cmd;
    cmdqelmt->CmdFromNdis = FALSE;
    cmdqelmt->PortNum = pPort->PortNumber;

    DBGPRINT(RT_DEBUG_TRACE, ("--->MTEnqueueInternalCmd,   Cmd = 0x%x, PortNum = %d, CmdSeq = %d\n", Cmd, cmdqelmt->PortNum, cmdqelmt->Seq));

    if(BufferLen != 0)
    {
        if(BufferLen>INTERNAL_CMD_BUFFER_LEN)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - BufferLen>INTERNAL_CMD_BUFFER_LEN\n", __FUNCTION__));    
            MtReturnInternalCmdBuffer(pAd, cmdqelmt);
            return NDIS_STATUS_FAILURE;
        }    
        
        PlatformMoveMemory(cmdqelmt->buffer, Buffer, BufferLen);
        cmdqelmt->bufferlength = BufferLen;
    }

    if (cmdqelmt != NULL)
    {
        if(cmdqelmt->command & NORMAL_CMD_MASK)  // Normal cmd.
        {
        NdisAcquireSpinLock(&pAd->pHifCfg->CmdQLock);
        EnqueueCmd((&pAd->pHifCfg->CmdQ), cmdqelmt);
        NdisReleaseSpinLock(&pAd->pHifCfg->CmdQLock);
        
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hControlThread)));
    }
        else if(cmdqelmt->command & P2P_CMD_MASK) //P2P related Cmd
        {
            NdisAcquireSpinLock(&pAd->pP2pCtrll->P2PCmdQLock);
            EnqueueCmd((&pAd->pP2pCtrll->P2PCmdQ), cmdqelmt);
            NdisReleaseSpinLock(&pAd->pP2pCtrll->P2PCmdQLock);
            
            NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pP2pCtrll->hP2PThread)));
        }
        else if(cmdqelmt->command & MULTICHANNEL_CMD_MASK) // MUltiChannel related Cmd
        {
            NdisAcquireSpinLock(&pAd->MccCfg.MultiChannelCmdQLock);
            EnqueueCmd((&pAd->MccCfg.MultiChannelCmdQ), cmdqelmt);
            NdisReleaseSpinLock(&pAd->MccCfg.MultiChannelCmdQLock);
            
            NdisSetEvent(&(GET_THREAD_EVENT(&pAd->MccCfg.hMultiChannelThread)));
        }
        else
        {
            DBGPRINT_ERR(("%s Cmd 0x%x, not match any case   !!!!!\n",__FUNCTION__, cmdqelmt->command));
        }
    }

    DBGPRINT(RT_DEBUG_INFO, ("<---MTEnqueueInternalCmd\n"));
    return(NDIS_STATUS_SUCCESS);
}


NDIS_STATUS 
MTDeQueueNoAMgmtPacket(
    IN  PMP_ADAPTER   pAd
    )
{
    PQUEUE_ENTRY    pQEntry = NULL;
    PNOAMGMT_STRUC  pMgmt = NULL;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    UCHAR           Count = 0;
    UCHAR           TxPacketNum;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    if (IS_P2P_GO_OP(pPort) || IS_P2P_CLIENT_OP(pPort))
    {
        if ((pAd->HwCfg.LatchRfRegs.Channel  != pPort->CentralChannel)    ||
            (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))              ||
            (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))      ||
            (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))       ||
            (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))          || 
            (IS_P2P_ABSENCE(pPort))                                       ||
            (pPort->P2PCfg.P2PChannelState == P2P_ENTER_GOTOSCAN)         ||
            ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE)))
        {
            return NDIS_STATUS_FAILURE;
        }
    }
    else if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))  ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))              ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))      ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))       ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return NDIS_STATUS_FAILURE;
    }

    // check if there is pending NULL frame to send
    /*if(pPort->P2PCfg.bPendingNullFrame)
    {
        XmitSendNullFrame(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pPort->CommonCfg.TxRate, TRUE);
        pPort->P2PCfg.bPendingNullFrame = FALSE;
    }*/

    NdisAcquireSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);

    TxPacketNum = (pPort->P2PCfg.bOppsOn) ? ((pPort->P2PCfg.CTWindows & 0x7f) / 10) : (MAX_TX_PROCESS);
    
    while(pAd->pP2pCtrll->TxSwNoAMgmtQueue.Head != NULL && Count < TxPacketNum) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("MTDeQueueNoAMgmtPacket = %d\n", pAd->pP2pCtrll->TxSwNoAMgmtQueue.Number));
        // Check NoA Period in advance . So doesn't need to consider to InsertHeadQueue after dequeue.
        if (IS_P2P_ABSENCE(pPort))
        {
            DBGPRINT(RT_DEBUG_TRACE,("bKeepSlient becomes %d. Stop dequeue Management frame.\n", pPort->P2PCfg.bKeepSlient));
            break;
        }
        // Dequeue the first entry from head of queue list
        pQEntry = RemoveHeadQueue(&pAd->pP2pCtrll->TxSwNoAMgmtQueue);
        if (pQEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("MTDeQueueNoAMgmtPacket = pNetBuffer NULL . !\n"));
            break;
        }
        
        Status = NdisCommonFreeDescriptorRequest(pAd, PRIO_RING, 0, 1);
        if (Status == NDIS_STATUS_SUCCESS)
        {
            pMgmt = CONTAINING_RECORD(pQEntry, NOAMGMT_STRUC, NoAQEntry);   

            //Make sure queued item contains data
            if(pMgmt->NoAQEntry.pBuffer == NULL)
                continue;

            NdisCommonMlmeHardTransmit(pAd, &pMgmt->NoAQEntry);

            MlmeFreeMemory(pAd, pMgmt->NoAQEntry.pBuffer);
            pMgmt->NoAQEntry.pBuffer = NULL;
            pMgmt->NoAQEntry.Valid = FALSE;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%S(%d): not enough space in PrioRing\n",__FUNCTION__,__LINE__));
            break;
        }
        Count++;
    }
        
    NdisReleaseSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
    return Status;
}

/*
    ========================================================================

    Routine Description:
        Suspend MSDU transmission
        
    Arguments:
        pAd     Pointer to our adapter
        
    Return Value:
        None
        
    Note:
    
    ========================================================================
*/
VOID    
MTSuspendMsduTransmission(
    IN  PMP_ADAPTER   pAd
    )
{
//  TX_RTS_CFG_STRUC RtsCfg;

    DBGPRINT(RT_DEBUG_TRACE,("SCANNING, suspend MSDU transmission ...\n"));

    MT_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
    DBGPRINT(RT_DEBUG_TRACE,("SCANNING, MT_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)\n"));

    pAd->Mlme.ChannelStayCount = 0; 
}

UCHAR 
MTDecideSegmentEnd(
    IN    ULONG             CurWritePosition
    )
{
    UCHAR   SegIdx = SEGMENT_TOTAL + 1;
    ULONG   i;
    
    if (CurWritePosition > MAX_TXBULK_LIMIT)
    {
        SegIdx = SEGMENT_TOTAL;
    }
    else
    {
        for ( i = 1; i < SEGMENT_TOTAL; i++)
        {
            if ((CurWritePosition > (ONE_SEGMENT_UNIT* (SEGMENT_TOTAL - i) - LOCAL_TXBUF_SIZE_4K))
                && (CurWritePosition < (ONE_SEGMENT_UNIT * (SEGMENT_TOTAL - i))))
            {
                SegIdx = SEGMENT_TOTAL - i;
                break;
            }
        }
    }

    return  SegIdx;
}

NDIS_STATUS
InitializeHWCtrl (
    IN  PMP_ADAPTER pAd,
    OUT PADAPTER_INITIAL_STAGE  pInitialStage
    )
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;

    //
    // Initialize HW version
    //
    InitHwVersion(pAd);

    HalInitHalCtrl(pAd);

    // Initialize the supported PHY type array.
    RTMPInitPhyType(pAd);

     //
    // Define length of TXWI and RXWI
    //
    pAd->HwCfg.TXWI_Length = TXWI_SIZE_16_BYTES;
    pAd->HwCfg.RXWI_Length = RXWI_SIZE_16_BYTES;

    DBGPRINT(RT_DEBUG_TRACE,("pAd->HwCfg.TXWI_Length = %d\n",pAd->HwCfg.TXWI_Length));
    DBGPRINT(RT_DEBUG_TRACE,("pAd->HwCfg.RXWI_Length = %d\n",pAd->HwCfg.RXWI_Length));

    DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, pAd->MACVersion = 0x%x\n",__FUNCTION__,__LINE__,pAd->HwCfg.MACVersion));

     return  NdisStatus;

}

NDIS_STATUS
InitializeProtocolCtrl (
    IN  PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     NdisStatus = NDIS_STATUS_SUCCESS;

    RTMPInitP2P(pAd);

    //
    // Initialize VHT control
    //
    VhtInitCtrl(pAd);

    return   NdisStatus;
}
