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
    OID_RT6X9X.c

    Abstract: 
    RT6X9X features about OID(RT6590/RT6592/MT7601)

    Revision History:
    Who                     When            What
    ------------------- ----------      ----------------------------------------------
        Smith Hsu       2011/12/1       Start implementation
        Ian Tang        2012/2/6        Add HT/VHT functions
*/

#include "MtConfig.h"



/*
 **********************************************************************************************************************
 * [Section]
 *
 * Public Function Definition for OID procedure
 **********************************************************************************************************************
*/



//
// Query hardware version for different HW architecture, such as RF range and RF band selection
//
ULONG OidQueryHwVersion(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: MacVersion = %d, BbVersion = %d, RfVersion = %d, eFuseVersion = %d\n", 
        __FUNCTION__, 
        pAd->HwCfg.HwArchitectureVersion.MacVersion, 
        pAd->HwCfg.HwArchitectureVersion.BbVersion, 
        pAd->HwCfg.HwArchitectureVersion.RfVersion, 
        pAd->HwCfg.HwArchitectureVersion.eFuseVersion));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));

    return pAd->HwCfg.HwArchitectureVersion.Word;
}

//
// Report HW feature. example: VHT, HwWAPI etc.
//
BOOLEAN OidQueryHwChipCapability(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pQueryBuffer,
    IN  ULONG           QueryBufferLength,
    OUT PULONG          pBytesWritten
    )   
{
    POID_BUF_HW_CAPABILITIES   pHwCapa;

    // ==============================
    // Check the Buffer length
    // ==============================
    // The length of first Buffer Version of HwCapa is 8 byte.
    if (QueryBufferLength < sizeof(OID_BUF_HW_CAPABILITIES))
    {
        *pBytesWritten = sizeof(OID_BUF_HW_CAPABILITIES);
        return FALSE;
    }

    // ==============================
    // Start to fill data to information buffer
    // ==============================
    pHwCapa = (POID_BUF_HW_CAPABILITIES)pQueryBuffer;
    
    pHwCapa->BufVer = OID_BUF_HW_CAPA_BUFVER_1;
    pHwCapa->HwCapaInfo.UniField.VhtSupp = VHT_CAPABLE(pAd);

    // indicate BW80 is supported.
    pHwCapa->HwCapaInfo.UniField.ChannelWidth = VHT_BW_80;

    *pBytesWritten = sizeof(OID_BUF_HW_CAPABILITIES);

    // ToDo: assign more value in future
    // Capability2
    // Capability3 ...

    DBGPRINT(RT_DEBUG_TRACE, ("[OidSubFunc] %s: Report pHwCapa->BufVer = %d, pHwCapa->VhtSupp = %d, pHwCapa->ChannelWidth = %d\n", 
                __FUNCTION__, 
                pHwCapa->BufVer, 
                pHwCapa->HwCapaInfo.UniField.VhtSupp,
                pHwCapa->HwCapaInfo.UniField.ChannelWidth));

    return TRUE;
}


//
// Set Tx rate to Fixed rate for testing.
//
BOOLEAN OidSetTxFixedRate(
    IN  PMP_ADAPTER       pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PVOID               pSetBuffer,
    IN  ULONG               SetBufferLength,
    OUT PULONG              pBytesNeeded
    )
{
    PMP_PORT          pPort = pAd->PortList[PortNumber];
    POID_BUF_FIXED_RATE pOidFixedRateSetting;
    //ULONG               Index;

    // ==============================
    // Check the Buffer length
    // ==============================
    // The length of first Buffer Version of VhtCapa is 8 byte.
    if (SetBufferLength < sizeof(OID_BUF_FIXED_RATE))
    {
        *pBytesNeeded = sizeof(OID_BUF_FIXED_RATE);
        return FALSE;
    }

    pOidFixedRateSetting = (POID_BUF_FIXED_RATE)pSetBuffer;
    DBGPRINT(RT_DEBUG_TRACE, ("[OidSubFunc] %s: OID buffer - FixedRateEnabled=%d\n", 
                __FUNCTION__,
                pOidFixedRateSetting->FixedRateEnabled));
    DBGPRINT(RT_DEBUG_TRACE, ("[OidSubFunc] %s: OID buffer - STBC=%d, GI=%d, MCS=%d, PhyMode=%d, BW=%d, Nss=%d\n", 
                __FUNCTION__, 
                pOidFixedRateSetting->STBC,
                pOidFixedRateSetting->SHORTGI,
                pOidFixedRateSetting->MCS,
                pOidFixedRateSetting->PhyMode,
                pOidFixedRateSetting->BW,
                pOidFixedRateSetting->Nss));


    // ==============================
    // Store UI configuration
    // ==============================
    
    // Enable or disable Tx fixed rate
    if(pOidFixedRateSetting->FixedRateEnabled)
    {
        pPort->CommonCfg.bAutoTxRateSwitch = FALSE;
        pAd->UiCfg.UtfFixedTxRateEn = TRUE;
    }
    else
    {
        pPort->CommonCfg.bAutoTxRateSwitch = TRUE;
        pAd->UiCfg.UtfFixedTxRateEn = FALSE;
        return TRUE;
    }

    // Store configuration
    WRITE_PHY_CFG_STBC(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, pOidFixedRateSetting->STBC);
    WRITE_PHY_CFG_SHORT_GI(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, pOidFixedRateSetting->SHORTGI);
    WRITE_PHY_CFG_MCS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, pOidFixedRateSetting->MCS);
    WRITE_PHY_CFG_MODE(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, pOidFixedRateSetting->PhyMode);
    WRITE_PHY_CFG_BW(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, pOidFixedRateSetting->BW);
    WRITE_PHY_CFG_NSS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, pOidFixedRateSetting->Nss);


    // ==============================
    // Check configuration and replace if it not reasonable
    // ==============================
    if (READ_PHY_CFG_MODE(pAd, &pAd->UiCfg.UtfFixedTxRateCfg) <= MODE_OFDM)
    {
        WRITE_PHY_CFG_STBC(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, 0);
        WRITE_PHY_CFG_SHORT_GI(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, 0);
        WRITE_PHY_CFG_BW(pAd, &pAd->UiCfg.UtfFixedTxRateCfg, 0);
    }
    
    
    DBGPRINT(RT_DEBUG_TRACE, ("[OidSubFunc] %s: Set FixedRateEnabled=%d\n", 
                __FUNCTION__,
                pOidFixedRateSetting->FixedRateEnabled));
    DBGPRINT(RT_DEBUG_TRACE, ("[OidSubFunc] %s: Set STBC=%d, GI=%d, MCS=%d, PhyMode=%d, BW=%d, Nss=%d\n", 
                __FUNCTION__, 
                READ_PHY_CFG_STBC(pAd, &pAd->UiCfg.UtfFixedTxRateCfg),
                READ_PHY_CFG_SHORT_GI(pAd, &pAd->UiCfg.UtfFixedTxRateCfg),
                READ_PHY_CFG_MCS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg),
                READ_PHY_CFG_MODE(pAd, &pAd->UiCfg.UtfFixedTxRateCfg),
                READ_PHY_CFG_BW(pAd, &pAd->UiCfg.UtfFixedTxRateCfg),
                READ_PHY_CFG_NSS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg)));
    
    return TRUE;    
}

//
// Set HT/VHT configurations
//
VOID SetHtVht(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  OID_SET_HT_PHYMODE *pHTPhyMode)
{
    ULONG   *pmcs;
    ULONG   Value = 0;
    UCHAR   BBPValue = 0;
    UCHAR   BBP3Value = 0;
    UCHAR BWSetting;
    UCHAR   RealTxAntNum = 1;
    UCHAR   RealRxAntNum = 1;
    UCHAR   PrimaryChannelLocation;
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

    
    //
    // debug messages
    //
    DBGPRINT(RT_DEBUG_TRACE,("%s, try to set PhyMode = %d\n",__FUNCTION__,pHTPhyMode->PhyMode));
    DBGPRINT(RT_DEBUG_TRACE,("%s, try to set TransmitNo = %d\n",__FUNCTION__,pHTPhyMode->TransmitNo));
    DBGPRINT(RT_DEBUG_TRACE,("%s, try to set ExtOffset = %d\n",__FUNCTION__,pHTPhyMode->ExtOffset));
    DBGPRINT(RT_DEBUG_TRACE,("%s, try to set MCS = %d\n",__FUNCTION__,pHTPhyMode->MCS));
    DBGPRINT(RT_DEBUG_TRACE,("%s, try to set BW = %d\n",__FUNCTION__,pHTPhyMode->BW));
    DBGPRINT(RT_DEBUG_TRACE,("%s, try to set STBC = %d\n",__FUNCTION__,pHTPhyMode->STBC));
    DBGPRINT(RT_DEBUG_TRACE,("%s, try to set SHORTGI = %d\n",__FUNCTION__,pHTPhyMode->SHORTGI));
    
    // Don't zerp supportedHyPhy structure.
    PlatformZeroMemory(&pPort->CommonCfg.HtCapability, sizeof(pPort->CommonCfg.HtCapability));
    PlatformZeroMemory(&pPort->CommonCfg.AddHTInfo, sizeof(pPort->CommonCfg.AddHTInfo));
    PlatformZeroMemory(&pPort->CommonCfg.NewExtChanOffset, sizeof(pPort->CommonCfg.NewExtChanOffset));
    PlatformZeroMemory(&pPort->CommonCfg.ExtCapIE, sizeof(pPort->CommonCfg.ExtCapIE));

    UpdateTxBfCapabilitiesField(pAd); // Update the TxBf capabilities field

    if (pPort->CommonCfg.BACapability.field.b2040CoexistScanSup == 1)
        pPort->CommonCfg.ExtCapIE.field.BssCoexstSup = 1;
    
    if (VHT_CAPABLE(pAd))
    {
        pPort->CommonCfg.ExtCapIE.field.OperatingModeNotification = 1;
    }
    
    pPort->CommonCfg.HtCapability.ExtHtCapInfo.PlusHTC = 1;

    if (pAd->HwCfg.bRdg)
    {
        pPort->CommonCfg.HtCapability.ExtHtCapInfo.RDGSupport = 1;
    }   
    // Fix MaxRAmpduFactor = 1. 2006-12-14 Jan
    pPort->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 2;
    pPort->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 2;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s -(RxBAWinLimit= %d), (MaxRAmpduFactor=%d)...\n",__FUNCTION__,pPort->CommonCfg.BACapability.field.RxBAWinLimit, pPort->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor));

    if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
    {
        pPort->CommonCfg.DesiredHtPhy.bHtEnable = TRUE;
        
        if (pPort->CommonCfg.PhyMode == PHY_11VHT)
            pPort->CommonCfg.DesiredVhtPhy.bVhtEnable = TRUE;
    }
                
    // Mimo power save, A-MSDU size, 
    pPort->CommonCfg.DesiredHtPhy.AmsduSize = (UCHAR)pPort->CommonCfg.BACapability.field.AmsduSize;
    pPort->CommonCfg.DesiredHtPhy.MimoPs = (UCHAR)pPort->CommonCfg.BACapability.field.MMPSmode;
    pPort->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pPort->CommonCfg.BACapability.field.MpduDensity;

    pPort->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pPort->CommonCfg.BACapability.field.AmsduSize;
    pPort->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pPort->CommonCfg.BACapability.field.MMPSmode;
    pPort->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pPort->CommonCfg.BACapability.field.MpduDensity;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s-----> AMsduSize = %d (MimoPs = %d) (RxBAWinLimit = %d) (TransmitNo = %d) \n",__FUNCTION__, pPort->CommonCfg.DesiredHtPhy.AmsduSize, pPort->CommonCfg.HtCapability.HtCapInfo.MimoPs, pPort->CommonCfg.BACapability.field.RxBAWinLimit, pHTPhyMode->TransmitNo));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: (PhyMode=%d, TransmitNo=%d, HtMode=%d, ExtOffset=%d, MCS index=%d, \n", 
                                __FUNCTION__,        
                                pHTPhyMode->PhyMode, 
                                pHTPhyMode->TransmitNo, 
                                pHTPhyMode->HtMode, 
                                pHTPhyMode->ExtOffset,
                                ( (pHTPhyMode->PhyMode == PHY_11VHT)? (pHTPhyMode->MCS & 0x0F):(pHTPhyMode->MCS) )
                                ));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: (<Nss when VHT=%d>, BW=%d, STBC=%d, SHORTGI=%d, <VhtCentralChannel in VHT=%d>) \n", 
                                __FUNCTION__,                                       
                                ( (pHTPhyMode->PhyMode == PHY_11VHT)? (pHTPhyMode->MCS & 0xF0):(0) ),
                                pHTPhyMode->BW, 
                                pHTPhyMode->STBC, 
                                pHTPhyMode->SHORTGI,
                                pHTPhyMode->CentralChannelSeg1
                                ));
    
    if(pHTPhyMode->HtMode == HTMODE_GF)
    {
        pPort->CommonCfg.HtCapability.HtCapInfo.GF = 1;
        pPort->CommonCfg.DesiredHtPhy.GF = 1;
    }
    else
    {
        pPort->CommonCfg.HtCapability.HtCapInfo.GF = 0;
        pPort->CommonCfg.DesiredHtPhy.GF = 0;
    }

    // keep the UI setting
    BWSetting = pHTPhyMode->BW;

    // %%%% Adhoc mode constrain %%%% 
    if ((pAd->StaCfg.BssType == BSS_ADHOC) && (pPort->PortSubtype == PORTSUBTYPE_STA))
    {   
        // Azureware requests to fix as 20MHz BW and long GI
        if (pAd->StaCfg.FixLinkSpeedStatus == 1)
        {
            pHTPhyMode->BW = BW_20;
            pHTPhyMode->SHORTGI = GI_800;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Adhoc constrain to 20MHz and Long GI under bFixLinkSpeedStatus=TRUE\n",__FUNCTION__));
        }
        else 
        {
            if (pAd->StaCfg.bAdhocN40MHzAllowed == FALSE)
            {
                pHTPhyMode->BW = BW_20;
                pHTPhyMode->SHORTGI = GI_800;
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Adhoc constrain to 20MHz and Long GI under bAdhocN40MHzAllowed=FALSE\n",__FUNCTION__));
            }   
        }

        // Not support GF
        pPort->CommonCfg.HtCapability.HtCapInfo.GF = 0;
        pPort->CommonCfg.DesiredHtPhy.GF = 0;
    }
    
    // Set desired rate according to GUI setting,  SUpported 
    //===========================================================>
    pPort->CommonCfg.DesiredHtPhy.MCSSet[4]= 0x1;

    // EEPROM TxStream setting has higher priority than Antenna.field.TxPath. So check TxStream first
    if (pAd->HwCfg.NicConfig3.field.TxStream == 2)
    {
        RealTxAntNum = 2;
    }
    else if (pAd->HwCfg.NicConfig3.field.TxStream == 3)
    {
        RealTxAntNum = 3;
    }
    else if (pAd->HwCfg.NicConfig3.field.TxStream == 1)
    {
        RealTxAntNum = 1;
    }
    else if (pAd->HwCfg.Antenna.field.TxPath == 2)
    {
        RealTxAntNum = 2;
    }
    else if (pAd->HwCfg.Antenna.field.TxPath == 3)
    {
        RealTxAntNum = 3;
    }
     
    // Set Corrent  DesiredHtPhy based on Tx Antenna Number setting
    switch (RealTxAntNum)
    {
        case 2:
            // 2T
            if ( (pHTPhyMode->PhyMode != PHY_11VHT) && (pHTPhyMode->MCS < 16) ||
                 (pHTPhyMode->PhyMode == PHY_11VHT) && (pHTPhyMode->MCS < 16)    // Currently, VHT only support MCS 0~7 (Nss=1).
                )
            {
                pmcs = (ULONG*)&(pPort->CommonCfg.DesiredHtPhy.MCSSet[0]);
                *pmcs= 1<<(pHTPhyMode->MCS);
            }
            else if (pHTPhyMode->MCS == 32)
            {
                pPort->CommonCfg.DesiredHtPhy.MCSSet[0]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[1]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[2]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[4]= 0x01;
            }
            else
            {
                pPort->CommonCfg.DesiredHtPhy.MCSSet[0]= 0xff;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[1]= 0xff;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[2]= 0x00;
            }
            break;
        case 3:
            // 3T
            if ( (pHTPhyMode->PhyMode != PHY_11VHT) && (pHTPhyMode->MCS < 24) ||
                 (pHTPhyMode->PhyMode == PHY_11VHT) && (pHTPhyMode->MCS < 24)    // Currently, VHT only support MCS 0~7 (Nss=1).
                )
            {
                pmcs = (ULONG*)&(pPort->CommonCfg.DesiredHtPhy.MCSSet[0]);
                *pmcs= 1<<(pHTPhyMode->MCS);
            }
            else if (pHTPhyMode->MCS == 32)
            {
                pPort->CommonCfg.DesiredHtPhy.MCSSet[0]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[1]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[2]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[4]= 0x01;
            }
            else
            {
                pPort->CommonCfg.DesiredHtPhy.MCSSet[0]= 0xff;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[1]= 0xff;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[2]= 0xff;
            }
            break;
    
        default:
            // 1T or wrong setting
            if ( (pHTPhyMode->PhyMode != PHY_11VHT) && (pHTPhyMode->MCS < 8) ||
                 (pHTPhyMode->PhyMode == PHY_11VHT) && (pHTPhyMode->MCS < 10)    // Currently, VHT only support MCS 0~7 (Nss=1).
                )
            {
                pPort->CommonCfg.DesiredHtPhy.MCSSet[0]= 1<<pHTPhyMode->MCS;
            }
            else if (pHTPhyMode->MCS == 32)
            {
                pPort->CommonCfg.DesiredHtPhy.MCSSet[0]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[1]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[2]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[4]= 0x01;
            }
            else
            {
                pPort->CommonCfg.DesiredHtPhy.MCSSet[0]= 0xff;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[1]= 0x00;
                pPort->CommonCfg.DesiredHtPhy.MCSSet[2]= 0x00;
            }
            break;

    }
    
    // EEPROM RxStream setting has higher priority than Antenna.field.RxPath. So check RxStream first
    if (pAd->HwCfg.NicConfig3.field.RxStream == 2)
    {
        RealRxAntNum = 2;
    }
    else if (pAd->HwCfg.NicConfig3.field.RxStream == 3)
    {
        RealRxAntNum = 3;
    }
    else if (pAd->HwCfg.NicConfig3.field.RxStream == 1)
    {
        RealRxAntNum = 1;
    }
    else if (pAd->HwCfg.Antenna.field.RxPath == 2)
    {
        RealRxAntNum = 2;
    }
    else if (pAd->HwCfg.Antenna.field.RxPath == 3)
    {
        RealRxAntNum = 3;
    }

    // The HtCapability broadcast in mgmt frames should be based on our Rx Antenna Number setting    
    switch (RealRxAntNum)
    {
        case 2:
            pPort->CommonCfg.HtCapability.MCSSet[0] = 0xff;
            pPort->CommonCfg.HtCapability.MCSSet[1] = 0xff;
            pPort->CommonCfg.HtCapability.MCSSet[2] = 0x00;
            break;
        case 3:
            pPort->CommonCfg.HtCapability.MCSSet[0] = 0xff;
            pPort->CommonCfg.HtCapability.MCSSet[1] = 0xff;
            pPort->CommonCfg.HtCapability.MCSSet[2] = 0xff;
            break;
        default:
            pPort->CommonCfg.HtCapability.MCSSet[0] = 0xff;
            pPort->CommonCfg.HtCapability.MCSSet[1] = 0x00;
            pPort->CommonCfg.HtCapability.MCSSet[2] = 0x00;
            break;
    }
    //<============================================================

    if(pHTPhyMode->BW == BW_80)
    {       
        pPort->BBPCurrentBW = BW_80;
        //
        if (pPort->Channel<=14)
            DBGPRINT(RT_DEBUG_ERROR,("%s, LINE%d, ERROR!! pPort->Channel(=%d) <=14\n",__FUNCTION__,__LINE__,pPort->Channel));
        if (pPort->CommonCfg.PhyMode<PHY_11VHT)
            DBGPRINT(RT_DEBUG_ERROR,("%s, LINE%d, ERROR!! pPort->CommonCfg.PhyMode(=%d) <=PHY_11VHT\n",__FUNCTION__,__LINE__,pPort->CommonCfg.PhyMode));

        // save information for HT
        pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;     
        pPort->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = BW_40;      
        pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = (pHTPhyMode->ExtOffset == EXTCHA_BELOW) ? (EXTCHA_BELOW) : (EXTCHA_ABOVE);
        pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_40;
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelWidth= VHT_BW_80;        

        Vht5GCentralChannelDecision(pAd, pPort->Channel, BW_80, &pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1);

        pPort->CentralChannel = pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1;
            
        GetPrimaryChannelLocation(
            pAd, 
            pPort->Channel, 
            BW_80, 
            pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1, 
            &PrimaryChannelLocation);
        
        SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_80, PrimaryChannelLocation);      

        DBGPRINT(RT_DEBUG_TRACE,("%s, PrimaryChannel = %d\n",__FUNCTION__,pPort->Channel));
        DBGPRINT(RT_DEBUG_TRACE,("%s, BW= %d\n",__FUNCTION__,BW_80));
        DBGPRINT(RT_DEBUG_TRACE,("%s, CentralChannelSeg1 = %d\n",__FUNCTION__,pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1));
        DBGPRINT(RT_DEBUG_TRACE,("%s, PriChannelLocation = %d\n",__FUNCTION__,PrimaryChannelLocation));
    }
    else if(pHTPhyMode->BW == BW_40)
    {
        pPort->BBPCurrentBW = BW_40;

        pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;
        pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_40;
        pPort->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = BW_40;      
        pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = (pHTPhyMode->ExtOffset == EXTCHA_BELOW) ? (EXTCHA_BELOW) : (EXTCHA_ABOVE);

        if(pHTPhyMode->PhyMode == PHY_11VHT)
        {
            // VHT default is BW80 capable;
            pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelWidth=VHT_BW_80;

            // get VHT central channel
            if (pPort->Channel>14)
            {
                Vht5GCentralChannelDecision(pAd, pPort->Channel, BW_40, &pPort->CentralChannel);
            }
            else // g band
            {
                if (pHTPhyMode->ExtOffset == EXTCHA_BELOW)
                {
                    pPort->CentralChannel = pPort->Channel-2;
                }
                else if (pHTPhyMode->ExtOffset == EXTCHA_ABOVE)
                {
                    pPort->CentralChannel = pPort->Channel+2;
                }
                else
                DBGPRINT(RT_DEBUG_ERROR,("%s, LINE%d, invalid pHTPhyMode->ExtOffset(=%d)\n",__FUNCTION__,__LINE__,pHTPhyMode->ExtOffset));
            }           
            //
            // get ExtChanOffset for HT IEs
            //
            GetPrimaryChannelLocation(pAd, pPort->Channel, BW_40, pPort->CentralChannel, &PrimaryChannelLocation);

            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE%d, PhyMode =VHT, PrimaryChannel = %d, CentralChannel = %d\n",
                    __FUNCTION__,
                    __LINE__,
                    pPort->Channel,
                    pPort->CentralChannel));

            DBGPRINT(RT_DEBUG_TRACE,("BW=%d, Primary ChannelOffset = %d, ExtChanOffset=%d\n",
                    BW_40,              
                    PrimaryChannelLocation,
                    pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
        }

        // Set Regsiter for extension channel position.
        // Turn on BBP 40MHz mode now only as AP . 
        // Sta can turn on BBP 40MHz after connection with 40MHz AP. Sta only broadcast 40MHz capability before connection.
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP || ADHOC_ON(pPort))
        {
            if (pHTPhyMode->PhyMode == PHY_11VHT)
            {
                SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset);
            }
            if ((pHTPhyMode->ExtOffset == EXTCHA_BELOW))
            {
                SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_BELOW);
            }
            else if ((pHTPhyMode->ExtOffset == EXTCHA_ABOVE))
            {
                SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_ABOVE);
            } 

            pPort->BBPCurrentBW = BW_40;
        }
    }
    else if(pHTPhyMode->BW == BW_20)
    {
        pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
        pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_20;
        pPort->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = BW_20;
        pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = EXTCHA_NONE;

        if(pHTPhyMode->PhyMode == PHY_11VHT)
        {
            pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelWidth=VHT_BW_20_40;
        }
        // 0. As AP
        // Turn on BBP 20MHz mode by request here.
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP || ADHOC_ON(pPort))
        {
            pPort->BBPCurrentBW = BW_20;

            BbSetTxRxBwCtrl(pAd, BW_20);
        }
        // 1. As STA
        // Don't change channel during linkup.  If set here, fixed rate setting will fail.
        if (!INFRA_ON(pPort))
            pPort->CentralChannel = pPort->Channel;             
    }
        
    if(pHTPhyMode->STBC == STBC_USE)
    {
        //TxSTBC
        //Set to 0 if not supported, Set to 1 if supported  
        if (pAd->HwCfg.Antenna.field.TxPath >= 2)
        {
            pPort->CommonCfg.HtCapability.HtCapInfo.TxSTBC = 1;
            pPort->CommonCfg.DesiredHtPhy.TxSTBC = 1;
        }
        else
        {
            pPort->CommonCfg.HtCapability.HtCapInfo.TxSTBC = 0;
            pPort->CommonCfg.DesiredHtPhy.TxSTBC = 0;     
        }

        //RxSTBC
        //Set to 0 for no support,                          Set to 1 for support of one spatial stream
        //Set to 2 for support of one and two spatial streams,  Set to 3 for support of one, two and three spatial streams
        if( (pAd->HwCfg.Antenna.field.RxPath >= 1 ) && (pPort->CommonCfg.TgnControl.field.STBCRXDisable == FALSE) )
        {
            pPort->CommonCfg.HtCapability.HtCapInfo.RxSTBC = 1;
            pPort->CommonCfg.DesiredHtPhy.RxSTBC = 1;
        }
        else
        {
            pPort->CommonCfg.HtCapability.HtCapInfo.RxSTBC = 0; 
            pPort->CommonCfg.DesiredHtPhy.RxSTBC = 0;     
        }
    }
    else
    {
        pPort->CommonCfg.DesiredHtPhy.TxSTBC = 0;
        pPort->CommonCfg.DesiredHtPhy.RxSTBC = 0;
    }

    if(pHTPhyMode->SHORTGI == GI_400)
    {
        pPort->CommonCfg.HtCapability.HtCapInfo.ShortGIfor20 = 1;
        pPort->CommonCfg.HtCapability.HtCapInfo.ShortGIfor40 = 1;
        pPort->CommonCfg.DesiredHtPhy.ShortGIfor20 = 1;
        pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 = 1;
    }
    else
    {
        pPort->CommonCfg.DesiredHtPhy.ShortGIfor20 = 0;
        pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 = 0;
    }
    
    // We support link adaptation for unsolicit MCS feedback, set to 2.
    pPort->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_UNSOLICIT;
    pPort->CommonCfg.AddHTInfo.ControlChan = pPort->Channel;
    // 1, the extension channel above the control channel. 
    
    // Only for updating Registry transmit setting. However, real transmit setting is from OID setting or via negociation.
    pPort->CommonCfg.RegTransmitSetting.field.EXTCHA = pHTPhyMode->ExtOffset;
    pPort->CommonCfg.RegTransmitSetting.field.BW = BWSetting;
    pPort->CommonCfg.RegTransmitSetting.field.ShortGI = pHTPhyMode->SHORTGI;
    pPort->CommonCfg.RegTransmitSetting.field.STBC = pHTPhyMode->STBC;
    pPort->CommonCfg.RegTransmitSetting.field.HTMODE = pHTPhyMode->HtMode;
    pPort->CommonCfg.RegTransmitSetting.field.MCS = pHTPhyMode->MCS;

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        pPort->CommonCfg.bWmmCapable = TRUE;

        // EDCA parameters used for AP's own transmission
        if (pPort->CommonCfg.APEdcaParm.bValid == FALSE)
        {
            pPort->CommonCfg.APEdcaParm.bValid = TRUE;
            pPort->CommonCfg.APEdcaParm.Aifsn[0] = 3;
            pPort->CommonCfg.APEdcaParm.Aifsn[1] = 7;
            pPort->CommonCfg.APEdcaParm.Aifsn[2] = 1;
            pPort->CommonCfg.APEdcaParm.Aifsn[3] = 1;

            pPort->CommonCfg.APEdcaParm.Cwmin[0] = 4;
            pPort->CommonCfg.APEdcaParm.Cwmin[1] = 4;
            pPort->CommonCfg.APEdcaParm.Cwmin[2] = 3;
            pPort->CommonCfg.APEdcaParm.Cwmin[3] = 2;

            pPort->CommonCfg.APEdcaParm.Cwmax[0] = 10;
            pPort->CommonCfg.APEdcaParm.Cwmax[1] = 6;
            pPort->CommonCfg.APEdcaParm.Cwmax[2] = 4;
            pPort->CommonCfg.APEdcaParm.Cwmax[3] = 3;

            pPort->CommonCfg.APEdcaParm.Txop[0]  = 0;
            pPort->CommonCfg.APEdcaParm.Txop[1]  = 0;
            pPort->CommonCfg.APEdcaParm.Txop[2]  = AC2_DEF_TXOP;
            pPort->CommonCfg.APEdcaParm.Txop[3]  = AC3_DEF_TXOP;
        }
        MtAsicSetEdcaParm(pAd, pPort, &pPort->CommonCfg.APEdcaParm, TRUE);
    }

    //MlmeUpdateHtVhtTxRates(pAd, pPort,0xff);

    if (pHTPhyMode->MCS != MCS_AUTO && (pHTPhyMode->MCS != MCS_AUTO_SGI_GF))
    {
        pPort->CommonCfg.bAutoTxRateSwitch = FALSE;

        AsicCfgPAPEByStreams(pAd, (UCHAR)READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg), (UCHAR)pHTPhyMode->MCS);
        //
        // Store Configuration to CommonCfg
        //
        WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg, pHTPhyMode->STBC);
        WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, pHTPhyMode->SHORTGI);
        if(pHTPhyMode->PhyMode == PHY_11VHT)
        {
            WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg, (pHTPhyMode->MCS & 0x0F));
            WRITE_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg, (pHTPhyMode->MCS & 0xF0));            
        }
        else
        {
            WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg, (pHTPhyMode->MCS));
            WRITE_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg, ((pHTPhyMode->MCS / 8) - 1) ); 
        }
        
        switch (pHTPhyMode->HtMode)
        {
            case HTMODE_VHT:
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_VHT);
                break;
                        
            case HTMODE_MM:
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_HTMIX);
                break;

            case HTMODE_GF:
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_HTGREENFIELD);
                break;

            case HTMODE_11B:
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_CCK);
                break;

            case HTMODE_11G:
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_OFDM);
                break;
        }
        
        WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, pHTPhyMode->BW);

            //
            // Store Configuration to Mac Table within STA mode
            //
        // As a station, update fixed rate here.  AP decides station's rate at buildassociation.
        if ((pPort->PortSubtype == PORTSUBTYPE_STA) && pBssidMacTabEntry !=NULL)
        {
            WRITE_PHY_CFG_STBC(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg));
            WRITE_PHY_CFG_SHORT_GI(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg));
            WRITE_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
            WRITE_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg));
            WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg));
            WRITE_PHY_CFG_NSS(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg));
        }  
    }
    else
    {
        // SoftAP should not use StaActive variable and overwrite MacTab.Content[BSSID_WCID]'s data here.
        if ((pPort->PortSubtype == PORTSUBTYPE_STA || (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
            && (pAd->StaCfg.BssType == BSS_INFRA))
        {       
            // If auto mode, bandwidth falls back to linkup setting.
            //  MCS, shortGI, STBC, MODE will change periodcaly in DRS.
            if (pAd->StaActive.SupportedHtPhy.HtChannelWidth == BW_20)
            {
                WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, pAd->StaActive.SupportedHtPhy.HtChannelWidth);
            }
            else
            {
                WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, pHTPhyMode->BW);
            }
            if(pBssidMacTabEntry !=NULL)
            {
                WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg));
            }
        }
        else
        {
            WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, pHTPhyMode->BW);
        }
        pPort->CommonCfg.bAutoTxRateSwitch = TRUE;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("%s -(CommonCfg.bAutoTxRateSwitch= %d), CommonCfg.TxPhyCfg.field.MODE =%d, MCS=%d ..\n", 
        __FUNCTION__,
        pPort->CommonCfg.bAutoTxRateSwitch, 
        READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg), 
        READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg)));
}
//////////////////////////////////// End of File ////////////////////////////////////

