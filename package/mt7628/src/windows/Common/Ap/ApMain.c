/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************
 
    Module Name:
    soft_ap.c

    Abstract:
    Access Point specific routines and MAC table maintenance routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP

 */

#include "MtConfig.h"

#define DRIVER_FILE         0x00500000

char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
    "restart access point",
    "successfully associated",
    "has disassociated",
    "has been aged-out and disassociated" ,
    "active countermeasures",
    "has disassociated with invalid PSK password"};

/*
    ==========================================================================
    Description:
        Initialize AP specific data especially the NDIS packet pool that's
        used for wireless client bridging.
    ==========================================================================
 */
NDIS_STATUS APInitialize(
    IN  PMP_ADAPTER   pAd)
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    //UCHAR   GTK[TKIP_GTK_LENGTH];

    DBGPRINT(RT_DEBUG_TRACE, ("---> APInitialize\n"));

    // Init Group key update timer, and countermeasures timer
    PlatformInitTimer(pPort, &pPort->SoftAP.ApCfg.REKEYTimer, ApWpaGREKEYPeriodicExecTimerCallback, TRUE, "GREKEYPeriodicExecTimer");
    PlatformInitTimer(pPort, &pPort->SoftAP.ApCfg.CounterMeasureTimer, ApWpaCMTimerExecTimerCallback, FALSE,"CMTimerExecTimer");

    PlatformZeroMemory(&pAd->WdsTab, sizeof(WDS_TABLE));

    // A fake BSSID
    pPort->SoftAP.ApCfg.AdhocBssid[0] = 0xF0;
    pPort->SoftAP.ApCfg.AdhocBssid[1] = 0x50;
    pPort->SoftAP.ApCfg.AdhocBssid[2] = 0x40;
    pPort->SoftAP.ApCfg.AdhocBssid[3] = 0x30;
    pPort->SoftAP.ApCfg.AdhocBssid[4] = 0x20;
    pPort->SoftAP.ApCfg.AdhocBssid[5] = 0x10;

    DBGPRINT(RT_DEBUG_TRACE, ("<--- APInitialize\n"));
    return Status;
}

VOID APIndicateConnectStatus(
    IN PMP_ADAPTER        pAd)
{
    DOT11_CONNECTION_START_PARAMETERS           connStart;
    DOT11_CONNECTION_COMPLETION_PARAMETERS      connComp;
    PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

    //
    // Indicate connection start. If the connection status is not successful, 
    // use all-zeros as the BSSID in the connection start structure.
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(connStart.Header, 
                                 NDIS_OBJECT_TYPE_DEFAULT,
                                 DOT11_CONNECTION_START_PARAMETERS_REVISION_1,
                                 sizeof(DOT11_CONNECTION_START_PARAMETERS));
    connStart.BSSType = dot11_BSS_type_independent;
    PlatformMoveMemory(connStart.AdhocBSSID, pPort->SoftAP.ApCfg.AdhocBssid, sizeof(DOT11_MAC_ADDRESS));

    PlatformMoveMemory(connStart.AdhocSSID.ucSSID, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
    connStart.AdhocSSID.uSSIDLength = pPort->PortCfg.SsidLen;

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_CONNECTION_START,
                        NULL,
                        &connStart,
                        sizeof(DOT11_CONNECTION_START_PARAMETERS));

    //
    // Indicate connection complete regardless of the connection status
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(connComp.Header, 
                                 NDIS_OBJECT_TYPE_DEFAULT,
                                 DOT11_CONNECTION_COMPLETION_PARAMETERS_REVISION_1,
                                 sizeof(DOT11_CONNECTION_COMPLETION_PARAMETERS));

    connComp.uStatus = DOT11_CONNECTION_STATUS_SUCCESS;

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_CONNECTION_COMPLETION,
                        NULL,
                        &connComp,
                        sizeof(DOT11_CONNECTION_COMPLETION_PARAMETERS));
}

/*
    ==========================================================================
    Description:
        Start AP service. If any vital AP parameter is changed, a STOP-START
        sequence is required to disassociate all STAs.

    IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
    IRQL = PASSIVE_LEVEL. (from InitializeHandler)  

    Note:
        Can't call NdisMIndicateStatus on this routine.

        RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
        Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

    ==========================================================================
 */
VOID APStartUp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort) 
{
    UCHAR   GTK[TKIP_GTK_LENGTH];
    UCHAR   TableSize = 0;
    PMP_PORT pStaPort  = MlmeSyncGetActivePort(pAd);  
    BOOLEAN  bUpperBand= FALSE,bLowerBand = FALSE;
    UCHAR i = 0;
    OID_SET_HT_PHYMODE      HTPhymode;
    PMAC_TABLE_ENTRY pMBCastMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);

    if (pMBCastMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, pMBCastMacTabEntry Entry is NULL\n", __FUNCTION__, __LINE__));
        return;
    }
    
    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    if ((pAd->MlmeAux.bNeedPlatformIndicateScanStatus == TRUE) || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
    {
        pPort->SoftAP.ApCfg.StartSoftApAfterScan = TRUE;
        pPort->SoftAP.ApCfg.SoftApPort = pPort;
        DBGPRINT(RT_DEBUG_ERROR, ("%s - STA scanning, start SoftAp after scan done.", __FUNCTION__));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("===> %s. pPort->Channel = %d, pPort->CommonCfg.PhyMode = %d, pPort->SoftAP.ApCfg.PhyMode = %d, pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1 = %d\n", __FUNCTION__, pPort->Channel, pPort->CommonCfg.PhyMode, pPort->SoftAP.ApCfg.PhyMode, pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1));

    pPort->SoftAP.ApCfg.PhyMode = pPort->CommonCfg.PhyMode;

    if (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)
    {
        HTPhymode.PhyMode = pPort->SoftAP.ApCfg.PhyMode;
        HTPhymode.TransmitNo = (UCHAR)pAd->HwCfg.Antenna.field.TxPath;
        HTPhymode.HtMode = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.HTMODE;
        APABandGet40MChannel(pPort->Channel, &HTPhymode.ExtOffset);
        HTPhymode.MCS = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.MCS;
        HTPhymode.BW = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.BW;
        HTPhymode.STBC = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.STBC;
        HTPhymode.SHORTGI = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.ShortGI;

        SetHtVht(pAd, pPort, &HTPhymode);       

        PlatformMoveMemory(&pPort->SoftAP.ApCfg.HtCapability, &pPort->CommonCfg.HtCapability, sizeof(pPort->CommonCfg.HtCapability));
        PlatformMoveMemory(&pPort->SoftAP.ApCfg.AddHTInfoIe, &pPort->CommonCfg.AddHTInfo, sizeof(pPort->CommonCfg.AddHTInfo));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("===> %s. PortSubtype = %d.\n", __FUNCTION__, pPort->PortSubtype));

    pAd->BeaconPort.pApGoPort = pPort;
    pPort->SoftAP.bAPStart = TRUE;

    // make sure Radio is On
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        AsicRadioOn(pAd);
    
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        AsicForceWakeup(pAd);

#if 0
    //DLS tear down
    if (INFRA_ON(pStaPort) && (DLS_ON(pAd)))
    {
        DlsTearDown(pAd, pPort);
    }

    //TDLS tear down
    if (INFRA_ON(pStaPort) && (TDLS_ON(pAd)))
    {
        TdlsTearDown(pAd, TRUE);
    }
#endif
     
    if (pPort->Channel == 0)
        pPort->Channel = MlmeSyncFirstChannel(pAd);

    //
    // Ssid set from OID_DOT11_DESIRED_SSID_LIST
    //
    if ((pPort->PortCfg.SsidLen <= 0) || (pPort->PortCfg.SsidLen > MAX_LEN_OF_SSID))
    {
        PlatformMoveMemory(pPort->PortCfg.Ssid, "SoftAP-MTK", 9);
        pPort->PortCfg.SsidLen = 9;
    }
    pPort->SoftAP.ApCfg.CapabilityInfo    = CAP_GENERATE(1, 0, (pPort->PortCfg.WepStatus != Ralink802_11EncryptionDisabled), 1, pPort->CommonCfg.bUseShortSlotTime);

    if ((pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable))
    {
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

            pPort->CommonCfg.APEdcaParm.Cwmax[0] = 6;
            pPort->CommonCfg.APEdcaParm.Cwmax[1] = 10;
            pPort->CommonCfg.APEdcaParm.Cwmax[2] = 4;
            pPort->CommonCfg.APEdcaParm.Cwmax[3] = 3;

            pPort->CommonCfg.APEdcaParm.Txop[0]  = 0;
            pPort->CommonCfg.APEdcaParm.Txop[1]  = 0;
            pPort->CommonCfg.APEdcaParm.Txop[2]  = AC2_DEF_TXOP;
            pPort->CommonCfg.APEdcaParm.Txop[3]  = AC3_DEF_TXOP;
        }
        MtAsicSetEdcaParm(pAd, pPort, &pPort->CommonCfg.APEdcaParm, TRUE);

        // EDCA parameters to be annouced in outgoing BEACON, used by WMM STA
        if (pPort->SoftAP.ApCfg.BssEdcaParm.bValid == FALSE)
        {
            pPort->SoftAP.ApCfg.BssEdcaParm.bValid = TRUE;
            pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[0] = 3;
            pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[1] = 7;
            pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[2] = 2;
            pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[3] = 2;

            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[0] = 4;
            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[1] = 4;
            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[2] = 3;
            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[3] = 2;

            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[0] = 10;
            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[1] = 10;
            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[2] = 4;
            pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[3] = 3;

            pPort->SoftAP.ApCfg.BssEdcaParm.Txop[0]  = 0;
            pPort->SoftAP.ApCfg.BssEdcaParm.Txop[1]  = 0;
            pPort->SoftAP.ApCfg.BssEdcaParm.Txop[2]  = AC2_DEF_TXOP;
            pPort->SoftAP.ApCfg.BssEdcaParm.Txop[3]  = AC3_DEF_TXOP;
        }
    }
    else
    {
        MtAsicSetEdcaParm(pAd, pPort, NULL, TRUE);
    }
    
    if (pPort->SoftAP.ApCfg.PhyMode < PHY_11ABGN_MIXED)
    {
        // Patch UI
        pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
    }

    COPY_MAC_ADDR(pPort->PortCfg.Bssid, pPort->CurrentAddress);
    
    DBGPRINT(RT_DEBUG_TRACE, ("Bssid = %02x:%02x:%02x:%02x:%02x:%02x\n", pPort->PortCfg.Bssid[0],pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));
    //Need to study which mac address should be set
    /************************************************************/  
    if(pPort->PortSubtype == PORTSUBTYPE_VwifiAP)
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Microsoft SOFTAP is starting ...\n",__FUNCTION__,__LINE__));
#if 0        
        //
        // If station port is already connected, after multi bssid being set, its bss index will be changed from 0 to 1.
        // Wep key has to be re-installed.
        //
        BssIdx = Ndis6CommonGetBssidIndex(pAd, pStaPort, MULTI_BSSID_MODE);
        if (INFRA_ON(pStaPort) && BssIdx)
        {
            if ((pStaPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP40) ||
                (pStaPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP104) ||
                (pStaPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP))
            {
                for (i = 0; i < 4; i++)
                {
                    if (pStaPort->SharedKey[BSS0][i].KeyLen > 0)
                    {
                        AsicAddKeyEntry(pAd, pStaPort, pMacTabEntry->wcid, BssIdx, i, &pStaPort->SharedKey[BSS0][i], FALSE, FALSE);
                    }
                }
                MlmeInfoStaOidSetWEPDefaultKeyID(pStaPort);
            }
        }
#endif
        //
        // Set multi-bssid
        //      
        NdisCommonMappingPortToBssIdx(pPort, UPDATE_MAC, UPDATE_MAC);
        //AsicSetMultiBssidEx(pAd, pPort->CurrentAddress, 1);
        //AsicSetMultiOwnMac(pAd, pPort->CurrentAddress, 1);
        
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] %02x %02x %02x %02x %02x %02x\n",__FUNCTION__,__LINE__, pPort->CurrentAddress[0], pPort->CurrentAddress[1], pPort->CurrentAddress[2], pPort->CurrentAddress[3], pPort->CurrentAddress[4], pPort->CurrentAddress[5]));
    }
    else // pPort->PortSubtype == PORTSUBTYPE_SoftapAP
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] RALINK SOFTAP is starting ...\n",__FUNCTION__,__LINE__));
        MtAsicSetBssid(pAd, pPort->CurrentAddress, (UINT8)pPort->PortNumber);
    }


    pMBCastMacTabEntry->Addr[0] = 0x01;

    WRITE_PHY_CFG_MODE(pAd, &pMBCastMacTabEntry->TxPhyCfg, MODE_OFDM);
    WRITE_PHY_CFG_MCS(pAd, &pMBCastMacTabEntry->TxPhyCfg, MCS_3);
    
    pPort->SoftAP.ApCfg.AddHTInfoIe.ControlChan = pPort->Channel;

    // 3x3 device will not run AsicEvaluateRxAnt
    
    //If station port is connected with 11N AP, station's HT setting will be used as softap's HT setting.
    if ((OPSTATUS_TEST_FLAG(pStaPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
        && (pAd->StaActive.SupportedHtPhy.bHtEnable))
    {
        pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = pAd->StaActive.SupportedHtPhy.HtChannelWidth;
        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = pAd->StaActive.SupportedHtPhy.ExtChanOffset;
        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = (UCHAR)pAd->StaActive.SupportedHtPhy.HtChannelWidth;
    }
    else if ((OPSTATUS_TEST_FLAG(pStaPort, fOP_STATUS_MEDIA_STATE_CONNECTED)) && (pAd->LogoTestCfg.OnTestingWHQL))  // for WLanEXTAPVWIFITest of WHQL Test. If Station connects first, the softap should follow the station's channel setting.
    {
        pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = pAd->StaActive.SupportedHtPhy.HtChannelWidth;
        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = pAd->StaActive.SupportedHtPhy.ExtChanOffset;
        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = (UCHAR)pAd->StaActive.SupportedHtPhy.HtChannelWidth;
    }
    else
    {
        if (pPort->SoftAP.ApCfg.PhyMode < PHY_11ABGN_MIXED)
        {
            pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
            pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 0;
            pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
            pPort->BBPCurrentBW = BW_20;

            // reset BBP to bandwidth to 20MHz          
            BbSetTxRxBwCtrl(pAd, BW_20);
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : current channel = %d \n",pPort->Channel));
            if (pPort->Channel > 14)
            {
                if(pAd->HwCfg.NicConfig2.field.bDisableBW40ForA)
                {
                    // BW40 for A band is NOT allowed
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 0;
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_NONE;

                    // add VHT IEs
                    if (pPort->SoftAP.ApCfg.PhyMode == PHY_11VHT)
                    {
                        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth = VHT_BW_20_40;
                    }
                }
                else
                {
                    //
                    // add information for VHT
                    //                  
                    if(pPort->CommonCfg.PhyMode == PHY_11VHT)                 
                    {
                        // 
                        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth = pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelWidth;

                        // get central channel
                        if (pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth == VHT_BW_20_40)
                        {                       
                            Vht5GCentralChannelDecision(
                                pAd, 
                                pPort->Channel, 
                                (UCHAR)pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth,
                                &pPort->CentralChannel);

                            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE%d, SoftAP is VHTcapable, PrimaryChannel=%d, Bw = %d, CentralChannel=%d\n",
                                __FUNCTION__,
                                __LINE__,
                                pPort->Channel,
                                pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth,
                                pPort->CentralChannel));
                        }
                        else if(pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth == VHT_BW_80)
                        {
                            UCHAR CentralChannelin40Mhz=0;
                            // retrive central channel
                            Vht5GCentralChannelDecision(
                                pAd, 
                                pPort->Channel,
                                BW_80,
                                &pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1);

                            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE%d, SoftAP is VHTcapable, PrimaryChannel=%d, Bw=%d, ChannelCenterFreqSeg1=%d\n",
                                __FUNCTION__,
                                __LINE__,
                                pPort->Channel,
                                BW_80,
                                pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1));
                            //
                            // several information in HT IEs need to be decided
                            //
                            // 1.
                            pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;         
                            // 2.
                            pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 1;

                            for(i=0;i<NUM_OF_VALID_VHT_CHANNEL_TABLE;i++)
                            {
                                if(pPort->Channel == ValidVhtChannelTable[i].PrimaryChannel)
                                {
                                    if(ValidVhtChannelTable[i].BandWidth == BW_40)
                                    {
                                        CentralChannelin40Mhz = ValidVhtChannelTable[i].CentralChannel;
                                        // 3.
                                        if(CentralChannelin40Mhz > pPort->Channel)
                                            pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_ABOVE;
                                        else
                                            pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_BELOW;
                                        break;
                                    }           
                                }
                            }   
                        }   
                    }
                    else // pPort->CommonCfg.PhyMode < PHY_11VHT
                    {
                        pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;         
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 1;
                        switch (pPort->Channel)
                        {
                            case 36:
                            case 44:
                            case 52:
                            case 60:
                            case 100:
                            case 108:
                            case 116:
                            case 124:
                            case 132:
                            case 149:
                            case 157:
                                pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_ABOVE;
                                break;

                            case 165:
                            case 169:
                            case 173:
                                //BW_20 in these channels
                                pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
                                pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 0;
                                pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
                                break;

                            default:
                                pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_BELOW;
                                break;
                        }
                    }

                }
            }
            else
            {

                if (pPort->CommonCfg.PhyMode==PHY_11VHT)
                {
                    pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth = VHT_BW_20_40;
                    pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1 = 0;
                    pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg2 = 0;
                }

                if ((pAd->HwCfg.NicConfig2.field.bDisableBW40ForG) || 
                    (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_20 && pPort->PortSubtype == PORTSUBTYPE_SoftapAP))
                {
                    // BW40 for 2.4G band is NOT allowed
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 0;
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
                }
                else
                {
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;         
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 1;
                    
                    bUpperBand = RTMPCheckChannel(pAd, pPort->Channel + 2, pPort->Channel);
                    if(pPort->Channel  > 2)
                    {
                        bLowerBand = RTMPCheckChannel(pAd, pPort->Channel - 2, pPort->Channel);
                    }

                    if(bUpperBand)
                    {
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_ABOVE;
                    }
                    else if(bLowerBand)
                    {
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_BELOW;
                    }
                    else
                    {                   
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
                    }   
                }
            }
            //we need to support A band  40M mode when sta  connected with 40  or 20 M A band AP

            if (pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth==VHT_BW_80)
            {
                UCHAR PrimaryChannelLocation;           
                pPort->BBPCurrentBW = BW_80;
                pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1 = pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1;
                //
                // retrive primary channel offset
                //
                GetPrimaryChannelLocation(
                        pAd,
                        pPort->Channel,
                        BW_80,
                        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1,
                        &PrimaryChannelLocation
                        );

                pPort->CentralChannel = pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1;
                    
                SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_80, PrimaryChannelLocation);

                DBGPRINT(RT_DEBUG_TRACE,("%s, channel switching to Primary channel = %d, bandwidth = %d, Central channel = %d, PrimaryLocation = %d\n",
                        __FUNCTION__,
                        pPort->Channel,
                        BW_80,
                        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1,
                        PrimaryChannelLocation
                        ));         
            }
            else if ((pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
                (pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE))
            {
                pPort->BBPCurrentBW = BW_40;
                pPort->CentralChannel = pPort->Channel + 2;
                
                SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_ABOVE);
            
                DBGPRINT(RT_DEBUG_TRACE, ("%s : ExtAbove, ChannelWidth=%d, PrimaryChannel=%d, ExtChanOffset=%d, CentralChannel=%d\n",
                    __FUNCTION__,
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth, 
                    pPort->Channel, 
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset,
                    pPort->CentralChannel));
            }
            else if ((pPort->Channel > 2) &&
                    (pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
                    (pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset == EXTCHA_BELOW))
            {
                pPort->BBPCurrentBW = BW_40;
                if (pPort->Channel == 14)
                    pPort->CentralChannel = pPort->Channel - 1;
                else
                    pPort->CentralChannel = pPort->Channel - 2;
                
                SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_BELOW);

                DBGPRINT(RT_DEBUG_TRACE, ("%s : ExtBlow, ChannelWidth=%d, PrimaryChannel=%d, ExtChanOffset=%d, CentralChannel=%d\n",
                    __FUNCTION__,
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth, 
                    pPort->Channel, 
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset,
                    pPort->CentralChannel));
            }
            else
            {
                pPort->BBPCurrentBW = BW_20;
                pPort->CentralChannel = pPort->Channel;
                
                SwitchBandwidth(pAd, FALSE, pPort->Channel, BW_20, EXTCHA_NONE);

                DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : 20MHz, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d wmm = %d\n",
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth, pPort->Channel, pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset,pPort->CommonCfg.bWmmCapable));
            }       
        }
    }

    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);  

    if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
    {
    }
    else
    {
        if (pPort->BBPCurrentBW == BW_20)
        {
            RTUSBWriteBBPRegister(pAd, BBP_R66, 0x40);
        }
        else
        {
            RTUSBWriteBBPRegister(pAd, BBP_R66, 0x48);
        }
    }

//    MlmeSetTxPreamble(pAd, (USHORT)pPort->CommonCfg.TxPreamble);
//    MlmeUpdateTxRates(pAd, pPort,FALSE);
    //MlmeUpdateHtVhtTxRates(pAd, pPort,0xff);

    pAd->StaCfg.ActivePhyId = MlmeInfoGetPhyIdByChannel(pAd, pPort->Channel);
    
    // start sending BEACON out
    pPort->bBeaconing = TRUE;
//    MlmeUpdateBeacon(pAd);
    APUpdateCapabilityAndErpIe(pAd);
    pPort->SoftAP.ApCfg.bErpIEChange = TRUE; // Always update beacon for HT
    APUpdateOperationMode(pAd);
    if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11A) && (pPort->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pPort->Channel))
    {
        pPort->CommonCfg.RadarDetect.RDMode = RD_SILENCE_MODE;
        pPort->CommonCfg.RadarDetect.RDCount = 0;
        pPort->CommonCfg.RadarDetect.InServiceMonitorCount = 0;
        RadarDetectionStart(pAd);
    }
    else
    {
        if (pPort->SoftAP.ApCfg.bSoftAPReady == TRUE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SoftAp Start and send beacon!!!"));
            MgntPktSendBeacon(pPort); // send first beacon
            //
            // TODO: To be change to MtAsicEnableBssSync
            // 
            ApAsicEnableBssSync(pPort->pAd);
            //MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
        }
    }

    // Reset Max length of MPDU and PSDU 
    pPort->SoftAP.ApCfg.MAX_PSDU_LEN = MAC_VALUE_MAX_LEN_CFG_DEFAULT(pAd);

    // Enable pre-tbtt interrupt
    //RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
    //Value |=0x1;
    //RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);

    // Set LED
    LedCtrlSetLed(pAd, LED_LINK_UP);

    // if AuthMode >= WPA,then pairwise key table must be used; 
    // if legacy WEP inused, then only shared key table is used
    if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AP AuthMode=%s, Pairwise Key Table in-used\n", decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AP AuthMode=%s, disable Pairwise Key Table\n", decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
    }

    DBGPRINT(RT_DEBUG_TRACE, (" APStartUp :  Group rekey method= %d , interval = 0x%x\n",pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod,pPort->SoftAP.ApCfg.WPAREKEY.ReKeyInterval));
    // Group rekey related
    if ((pPort->SoftAP.ApCfg.WPAREKEY.ReKeyInterval != 0) && ((pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod == TIME_REKEY) || (pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod == PKT_REKEY))) 
    {
        // Regularly check the timer
        if (pPort->SoftAP.ApCfg.REKEYTimerRunning == FALSE)
        {
            pPort->SoftAP.ApCfg.REKEYTimerRunning = TRUE;
            PlatformSetTimer(pPort, &pPort->SoftAP.ApCfg.REKEYTimer, GROUP_KEY_UPDATE_EXEC_INTV);
            pPort->SoftAP.ApCfg.REKEYCOUNTER = 0;
        }
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("Port Subtype: %d\n", pPort->PortSubtype));
    
    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // Init TKIP Group-Key-related variables
        ApWpaGenRandom(pAd, pPort, pPort->SoftAP.ApCfg.GMK);
        ApWpaGenRandom(pAd, pPort, pPort->SoftAP.ApCfg.GNonce);
        ApWpaCountGTK(pAd, pPort->SoftAP.ApCfg.GMK, (UCHAR*)pPort->SoftAP.ApCfg.GNonce, pPort->CurrentAddress, GTK, TKIP_GTK_LENGTH);
        if ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) ||
            (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled) ||
            (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled))
        {
            PCIPHER_KEY pKey = &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId];
            pKey->KeyLen = LEN_TKIP_EK;
            PlatformMoveMemory(pKey->Key, GTK, LEN_TKIP_EK);
            PlatformMoveMemory(pKey->TxMic, &GTK[16], LEN_TKIP_TXMICK);
            PlatformMoveMemory(pKey->RxMic, &GTK[24], LEN_TKIP_RXMICK);

            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
                pKey->CipherAlg = CIPHER_AES;
            else
                pKey->CipherAlg = CIPHER_TKIP;

            AsicAddKeyEntry(pAd,pPort, MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST), BSS0, pPort->PortCfg.DefaultKeyId, pKey, FALSE, TRUE);
        }
    }
    // Disable Protection first.
    MtAsicUpdateProtect(pAd, HT_NO_PROTECT,  ALLN_SETPROTECT, FALSE, FALSE);
    //ApMlmeSetPiggyBack(pAd, TRUE);

    ApLogEvent(pAd, pPort->CurrentAddress, EVENT_RESET_ACCESS_POINT);
    pAd->Mlme.PeriodicRound = 0;
    pAd->Mlme.OneSecPeriodicRound = 0;

    OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
    // ##############################################################################################
    //PlatformSetTimer(pPort,&pPort->Mlme.MlmeCntLinkUpTimer, 0);

    // Txop can only be modified when RDG is off, WMM is disable and TxBurst is enable
    if (pPort->CommonCfg.bEnableTxBurst)
    {
    	// TODO: Shiang-usw-win, should not use this function directly unless you already stop the Tx/Rx
        MtAsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x60);
    }
    else
    {
        MtAsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x0);
    }

    if (pPort->CommonCfg.bCarrierDetect)
    {
//        AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_ONE_CTS, 1000);
        pPort->CommonCfg.bCarrierDetectPhase = TRUE;
    }

//    ApMlmeSetPiggyBack(pAd, pPort->CommonCfg.bPiggyBackCapable);
    
    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        APIndicateConnectStatus(pAd);
        APIndicateAssociationStatus(pAd, pPort->SoftAP.ApCfg.AdhocBssid);
    }
    else if (pPort->PortSubtype == PORTSUBTYPE_VwifiAP
            || (IS_P2P_STA_GO(pAd, pPort)))
    {
        //for two MAC mode
//        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, APCLIENTNORMAL);
    }

    OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_SOFTAP_IS_DOWN);
    pPort->SoftAP.bAPStart = TRUE;
    pPort->SoftAP.ApCfg.ApPortNum = pPort->PortNumber;   
    pPort->SoftAP.ApCfg.StaChannel = pPort->Channel;
    pPort->SoftAP.ApCfg.StaCentralChannel = pPort->CentralChannel;
    pPort->SoftAP.ApCfg.StaHtEnable = pAd->StaActive.SupportedHtPhy.bHtEnable;

    // re-initial tx rate again,beacuse APstart call mlmeupdatetxrate reset  pPort->CommonCfg.TxRateIndex to 0
    //  20M MCS 0  not work well with Broadcom AP 
//    pEntry = pMacTabEntry;
//    MlmeSelectTxRateTable(pAd,pEntry, &pTable, &TableSize, &pMacTabEntry->CurrTxRateIndex);

    //Indicate link quality.
    pPort->LastLinkQuality = 0;
    PlatformIndicateLinkQuality(pAd, pPort->PortNumber);

    //indicate AP channel to OS
    RTMPApIndicateFrequencyAdoped(pPort);

    // Reset RTS threshold to maximum
    pPort->CommonCfg.RtsThreshold = MAX_RTS_THRESHOLD;
//    RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
//    RtsCfg.field.RtsThres = 0xFFFF;
//    RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
//    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]_Reset RtsThreshold to maximum\n",__FUNCTION__,__LINE__));  

    DBGPRINT(RT_DEBUG_TRACE, ("<=== %s\n",__FUNCTION__));
}

/*
    ==========================================================================
    Description:
        disassociate all STAs and stop AP service.
    Note:
    ==========================================================================
 */
VOID APStop(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort) 
{
    BOOLEAN     Cancelled;
    PMP_PORT pStaPort;
    
    DBGPRINT(RT_DEBUG_TRACE, ("===> APStop !!!, pPort->PortNumber = %d\n",pPort->PortNumber));  
    OPSTATUS_SET_FLAG(pPort, fOP_STATUS_SOFTAP_IS_DOWN);    

    MacTableReset(pAd,pPort->PortNumber);
    // Disable pre-tbtt interrupt
    //RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
    //Value &=0xe;
    //RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);
    // Disable piggyback
    ApMlmeSetPiggyBack(pAd, FALSE);
    MtAsicUpdateProtect(pAd, HT_NO_PROTECT,  (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        if (pPort->CommonCfg.bCarrierDetect)
        {
            AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_STOP, 0);
            pPort->CommonCfg.bCarrierDetectPhase = FALSE;
        }

        pPort->bBeaconing = FALSE;
        MlmeUpdateBeacon(pAd);

        //
        // Set RxFilter back to Normal
        //
        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, STANORMAL);

        // Set LED
        pStaPort = Ndis6CommonGetStaPort(pAd);
        if(pStaPort != NULL)
        {
            if(!OPSTATUS_TEST_FLAG(pStaPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
                LedCtrlSetLed(pAd, LED_LINK_DOWN);
        }
    }

    if(pPort->SoftAP.ApCfg.REKEYTimerRunning == TRUE)
    {
        PlatformCancelTimer(&pPort->SoftAP.ApCfg.REKEYTimer, &Cancelled);
        pPort->SoftAP.ApCfg.REKEYTimerRunning=FALSE;
    }

    //
    // Cancel the Timer, to make sure the timer was not queued.
    //
    PlatformCancelTimer(&pPort->SoftAP.ApCfg.CounterMeasureTimer, &Cancelled);
    DBGPRINT(RT_DEBUG_TRACE, ("After cancel timer\n")); 
    pAd->BeaconPort.pApGoPort = NULL;
    pPort->SoftAP.bAPStart = FALSE;
    DBGPRINT(RT_DEBUG_TRACE, ("<=== APStop !!!\n"));
}

/*
    ==========================================================================
    Description:
        Shutdown AP and free AP specific resources
    ==========================================================================
 */
VOID APShutdown(
    IN PMP_ADAPTER pAd)
{
    BOOLEAN     bStopAP = FALSE;
    ULONG       i = 0;
    PMP_PORT  pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    BOOLEAN     Cancelled = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("---> APShutdown\n"));

    for (i = 0; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
    {
        if (pAd->PortList[i] == NULL)
        {
            continue;
            }

        if (pAd->PortList[i]->bActive == FALSE)
        {
            continue;
        }

        if (pAd->PortList[i]->PortSubtype == PORTSUBTYPE_SoftapAP || pAd->PortList[i]->PortSubtype == PORTSUBTYPE_VwifiAP)
        {
            APStop(pAd,pAd->PortList[i]);
            break;
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<--- APShutdown\n"));
}

/*
    ==========================================================================
    Description:
        This routine is used to clean up a specified power-saving queue. It's
        used whenever a wireless client is deleted.
    ==========================================================================
 */
VOID 
APCleanupPsQueue(
    IN  PMP_ADAPTER   pAd,
    IN  PQUEUE_HEADER   pQueue
    )
{
    PQUEUE_ENTRY pEntry;
    PMT_XMIT_CTRL_UNIT pXcu;
    while (pQueue->Head)
    {
        pEntry = RemoveHeadQueue(pQueue);
        pXcu = (PMT_XMIT_CTRL_UNIT)pEntry;    
        
        if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
        {
            RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_FAILURE);
        }
    }
}

/*
    ==========================================================================
    Description:
        This routine is called to log a specific event into the event table.
        The table is a QUERY-n-CLEAR array that stop at full.
    ==========================================================================
 */
VOID ApLogEvent(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pAddr,
    IN USHORT   Event)
{
    if (pAd->TrackInfo.EventTab.Num < MAX_NUM_OF_EVENT)
    {
        RT_802_11_EVENT_LOG *pLog = &pAd->TrackInfo.EventTab.Log[pAd->TrackInfo.EventTab.Num];
        NdisGetCurrentSystemTime(&pLog->SystemTime);
        COPY_MAC_ADDR(pLog->Addr, pAddr);
        pLog->Event = Event;
        DBGPRINT_RAW(RT_DEBUG_TRACE,("LOG#%d %02x:%02x:%02x:%02x:%02x:%02x %s\n",
            pAd->TrackInfo.EventTab.Num, pAddr[0], pAddr[1], pAddr[2], 
            pAddr[3], pAddr[4], pAddr[5], pEventText[Event]));
        pAd->TrackInfo.EventTab.Num += 1;
    }
}

/*
    ==========================================================================
    Description:
        Operationg mode is as defined at 802.11n for how proteciton in this BSS operates. 
        Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
        802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

        Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
        40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
        As STA, this obeys the operation mode in ADDHT IE.
        As AP, update protection when setting ADDHT IE and after new STA joined.
    ==========================================================================
*/
VOID APUpdateOperationMode(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    if( pPort->SoftAP.ApCfg.PhyMode < PHY_11ABGN_MIXED )
        return;
    
    // Not reset OperaionMode = 0 here.  Reset when 1. No Station associated with me. 2. MACTableReset. 

    // If I am 40MHz BSS, and there exist HT-20MHz station. 
    // Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection.
    if ((pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode == 0)
        && (pPort->MacTab.fAnyStation20Only )
        && (pPort->CommonCfg.DesiredHtPhy.HtChannelWidth == 1))
        pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 2;

    // If I use legacy rate for transmission to station.
    if (pPort->MacTab.fAnyStationIsLegacy)
        pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 3;
            
    pPort->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = pPort->MacTab.fAnyStationNonGF;
    
}

/*
    ==========================================================================
    Description:
        Update ERP IE and CapabilityInfo based on STA association status.
        The result will be auto updated into the next outgoing BEACON in next
        TBTT interrupt service routine
    ==========================================================================
 */
VOID APUpdateCapabilityAndErpIe(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    UCHAR  ErpIeContent = 0;
    BOOLEAN ShortSlotCapable = pPort->CommonCfg.bUseShortSlotTime;
    static UCHAR lastErpIeContent;
    PMAC_TABLE_ENTRY pEntry = NULL;
    PMAC_TABLE_ENTRY pNextEntry = NULL;  
    PQUEUE_HEADER pHeader;    
    
    lastErpIeContent = pPort->SoftAP.ApCfg.ErpIeContent;
    
    if (pPort->SoftAP.ApCfg.PhyMode == PHY_11B)
        return;

    pHeader = &pPort->MacTab.MacTabList;
    pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextEntry;
        
        if ((pEntry->ValidAsCLI == FALSE) || (pEntry->Sst != SST_ASSOC))
        {
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue;
        }

        // at least one 11b client associated, turn on ERP.NonERPPresent bit
        // almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility
        if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
        {
            ShortSlotCapable = FALSE;
            ErpIeContent |= 0x01;
        }

        // at least one client can't support short slot
        if ((pEntry->CapabilityInfo & 0x0400) == 0)
            ShortSlotCapable = FALSE;
        
        pNextEntry = pNextEntry->Next;       
        pEntry = NULL;
    }

    // decide ErpIR.UseProtection bit, depending on pPort->CommonCfg.UseBGProtection
    //    AUTO (0): UseProtection = 1 if any 11b STA associated
    //    ON (1): always USE protection
    //    OFF (2): always NOT USE protection
    if (pPort->CommonCfg.UseBGProtection == 0)
    {
        ErpIeContent = (ErpIeContent)? 0x03 : 0x00;
        if ((pPort->SoftAP.ApCfg.LastOLBCDetectTime + STA_AGED_OUT_TIME) > pAd->Mlme.Now64) // legacy BSS exist within 10 sec
        {
            DBGPRINT(RT_DEBUG_INFO, ("APUpdateCapabilityAndErpIe - Legacy 802.11b BSS overlaped\n"));
            ErpIeContent |= 0x02;                                     // set Use_Protection bit
        }
    }
    else if (pPort->CommonCfg.UseBGProtection == 1)   
        ErpIeContent |= 0x02;
    else
        ;
    
    if ((pPort->SoftAP.ApCfg.ErpIeContent & 0x1) != (ErpIeContent & 0x1))        
    {
        pPort->SoftAP.ApCfg.ErpIeContent = (ErpIeContent | 0x04);
        if ((pPort->SoftAP.ApCfg.ErpIeContent & 0x1) == 0x1)     
        {
            MtAsicUpdateProtect(pAd, pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT), FALSE, pPort->MacTab.fAnyStationNonGF);
        }
        else
            MtAsicUpdateProtect(pAd, pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT), TRUE, pPort->MacTab.fAnyStationNonGF);

    }
    pPort->SoftAP.ApCfg.ErpIeContent = (ErpIeContent | 0x04);

    //
    // deicide CapabilityInfo.ShortSlotTime bit
    //
    if (ShortSlotCapable)
        pPort->SoftAP.ApCfg.CapabilityInfo |= 0x0400;
    else
        pPort->SoftAP.ApCfg.CapabilityInfo &= 0xfbff;

    if (pPort->CommonCfg.TxPreamble == Rt802_11PreambleLong)
        pPort->SoftAP.ApCfg.CapabilityInfo &= (~0x020);
    else
        pPort->SoftAP.ApCfg.CapabilityInfo |= 0x020;
    DBGPRINT(RT_DEBUG_INFO, ("APUpdateCapabilityAndErpIe - Capability= 0x%04x, ERP is 0x%02x\n", 
        pPort->SoftAP.ApCfg.CapabilityInfo, ErpIeContent));

    MtAsicSetSlotTime(pAd, ShortSlotCapable, pPort->Channel);
    
    // ERP ie changed, turn on beacon-update flag
    if ( pPort->SoftAP.ApCfg.ErpIeContent != lastErpIeContent)
        pPort->SoftAP.ApCfg.bErpIEChange = TRUE;
}

/*
    ==========================================================================
    Description:
        Check if the specified STA pass the Access Control List checking.
        If fails to pass the checking, then no authentication nor association 
        is allowed
    Return:
        MLME_SUCCESS - this STA passes ACL checking

    ==========================================================================
*/
BOOLEAN ApCheckAccessControlList(
    IN PMP_ADAPTER pAd,
    IN PUCHAR        pAddr) 
{
    BOOLEAN Result = TRUE;
    ULONG   i;
    PMP_PORT        pPort = pAd->BeaconPort.pApGoPort;

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pPort can't be NULL\n", __FUNCTION__));
        return Result;
    }
    
    if (pPort->SoftAP.AccessControlList.Policy == ACL_POLICY_DISABLE)       // ACL is disabled
        Result = TRUE;
    else
    {
        if (pPort->SoftAP.AccessControlList.Policy == ACL_POLICY_POSITIVE_LIST)   // ACL is a positive list
            Result = FALSE;
        else                                              // ACL is a negative list
            Result = TRUE;
        for (i = 0; i < pPort->SoftAP.AccessControlList.Num; i++)
        {
            if (MAC_ADDR_EQUAL(pAddr, pPort->SoftAP.AccessControlList.Entry[i].Addr))
            {
                Result = !Result;
                break;
            }
        }
    }

    if (Result == FALSE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x failed ACL checking\n", pAddr[0],pAddr[1],pAddr[2],pAddr[3],pAddr[4],pAddr[5]));
    }

    return Result;
}

/*
    ==========================================================================
    Description:
        Look up a STA MAC table. Return its Sst to decide if an incoming
        frame from this STA or an outgoing frame to this STA is permitted.
    Return:
    ==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,    
    IN  PUCHAR pAddr, 
    OUT SST   *Sst, 
    OUT UCHAR *Aid,
    OUT UCHAR *PsMode,
    OUT UCHAR *Rate) 
{
    MAC_TABLE_ENTRY *pEntry = NULL;

    if (pAddr[0] & 0x01) // mcast & broadcast address
    {
        *Sst        = SST_ASSOC;
        *Aid        = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);   // Softap supports 1 BSSID and use WCID=0 as multicast Wcid index
        *PsMode     = PWR_ACTIVE;
        *Rate       = pPort->CommonCfg.MlmeRate; 
    } 
    else // unicast address
    {
        pEntry = MacTableLookup(pAd,  pPort, pAddr);
        if (pEntry) 
        {
            *Sst        = pEntry->Sst;
            *Aid        = pEntry->Aid;
            *PsMode     = pEntry->PsMode;
            if (((pEntry->AuthMode >= Ralink802_11AuthModeWPA) && (pEntry->AuthMode != Ralink802_11AuthModeAutoSwitch))
                && (pEntry->GTKState != REKEY_ESTABLISHED))
                *Rate = pPort->CommonCfg.MlmeRate;
            else
                *Rate = pEntry->CurrTxRate;
        } 
        else 
        {       
            *Sst        = SST_NOT_AUTH;
            *Aid        = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);
            *PsMode     = PWR_ACTIVE;
            *Rate       = pPort->CommonCfg.MlmeRate; 
        }
    }
    return pEntry;
}

/*
    ==========================================================================
    Description:
        Update the station current power save mode. Calling this routine also
        prove the specified client is still alive. Otherwise AP will age-out
        this client once IdleCount exceeds a threshold.
    ==========================================================================
 */
UCHAR APPsIndicate(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort,
    IN PUCHAR pAddr, 
    IN ULONG Wcid, 
    IN UCHAR Rssi,
    IN UCHAR Psm) 
{
    MAC_TABLE_ENTRY *pEntry;
    UCHAR           old_psmode;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (USHORT)Wcid); 

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return PWR_ACTIVE;
    }

    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("APPsIndicate wcid=%x \n", Wcid));
        return PWR_ACTIVE;
    }
    
    pEntry = pWcidMacTabEntry;
    old_psmode = pEntry->PsMode;
    
    if (PlatformEqualMemory(pEntry->Addr, pAddr, MAC_ADDR_LEN)) 
    {
        if ((pEntry->PsMode == PWR_SAVE) && (Psm == PWR_ACTIVE))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("APPsIndicate - %02x:%02x:%02x:%02x:%02x:%02x wakes up, act like rx PS-POLL\n", pAddr[0],pAddr[1],pAddr[2],pAddr[3],pAddr[4],pAddr[5]));
            // Send BAR to snyc sequence
            if (pEntry->IdentifyIntelSta == IOT_STRUC_INTEL_4965)
            {           
                pPort->CommonCfg.IOTestParm.bSendBAR = TRUE;
                ORIBATimerTimeout(pAd,pPort);
            }
            // sleep station awakes, move all pending frames from PSQ to TXQ if any
#if UAPSD_AP_SUPPORT            
            ApDataHandleRxPsPoll(pAd, pAddr, pEntry->Aid, FALSE, pPort, TRUE);
#else
            ApDataHandleRxPsPoll(pAd, pAddr, pEntry->Aid, FALSE);
#endif

        }
        else if ((pEntry->PsMode != PWR_SAVE) && (Psm == PWR_SAVE))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("APPsIndicate - %02x:%02x:%02x:%02x:%02x:%02x sleeps\n", pAddr[0],pAddr[1],pAddr[2],pAddr[3],pAddr[4],pAddr[5]));

            //For Sigma 6.1.11, we need to generate a fake QoS packet to pass the test
            //When staion indicates PSM, we need to update TIM 
            if(IS_P2P_SIGMA_ON(pPort) && pPort->P2PCfg.bSigmaFakePkt)
            {
                if (Wcid >= 32)
                    pPort->SoftAP.ApCfg.TimBitmap2 |= TIM_BITMAP[Wcid-32]; 
                else
                    pPort->SoftAP.ApCfg.TimBitmap |= TIM_BITMAP[Wcid];
                MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_UPDATE_TIM, NULL, 0);
            }
        }

        pEntry->NoDataIdleCount = 0;
        pEntry->PsMode = Psm;
        pEntry->LastRssi = Rssi;
    } 
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("APPsIndicate -[%dth] not match %02x:%02x:%02x:%02x:%02x:%02x \n", Wcid, pAddr[0],pAddr[1],pAddr[2],pAddr[3],pAddr[4],pAddr[5]));
        // not in table, try to learn it ???? why bother?
    }
    return old_psmode;
}

/*
    ==========================================================================
    Description:
        This routine reset the entire MAC table. All packets pending in
        the power-saving queues are freed here.
    ==========================================================================
 */
VOID MacTableReset(
    IN  PMP_ADAPTER  pAd,
    IN NDIS_PORT_NUMBER PortNum)
{
    PMP_PORT pPort = pAd->PortList[PortNum];
    PCHAR       pOutBuffer = NULL;
    NDIS_STATUS NStatus;
    ULONG       FrameLen = 0;
    HEADER_802_11 DisassocHdr;
    USHORT      Reason = REASON_DISASSOC_INACTIVE;
    RT_SET_ASIC_WCID    SetAsicWcid;
    BOOLEAN     Cancelled;
    DOT11_DISASSOCIATION_PARAMETERS disassocPara;
    UCHAR StartIdx = 1;
    UCHAR       BTHSPeer[8] = {0};
    UCHAR       P2PPeer[MAX_LEN_OF_MAC_TABLE]= {0}; // The peer is p2p client or GO
    UCHAR       Zero[8] = {0};
    PMAC_TABLE_ENTRY  pEntry;
    MP_RW_LOCK_STATE LockState;
    MP_RW_LOCK_STATE PsQLockState;
    PQUEUE_ENTRY    pQEntry = NULL;
    PNOAMGMT_STRUC  pMgmt = NULL;
    MTK_TIMER_STRUCT backupTimer;

    PMAC_TABLE_ENTRY pNextEntry = NULL;          
    PQUEUE_HEADER pHeader;

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
    PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    DBGPRINT(RT_DEBUG_TRACE, ("McastPsQueue.Number %d...\n",pPort->MacTab.McastPsQueue.Number ));
    PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock, &PsQLockState);
    APCleanupPsQueue(pAd, &pPort->MacTab.McastPsQueue);
    PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock, &PsQLockState);
    InitializeQueueHeader(&pPort->MacTab.McastPsQueue);

    //Clear pending NoA queue
    NdisAcquireSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
    while(pAd->pP2pCtrll->TxSwNoAMgmtQueue.Head != NULL)
    {
        pQEntry = RemoveHeadQueue(&pAd->pP2pCtrll->TxSwNoAMgmtQueue);
        pMgmt = CONTAINING_RECORD(pQEntry, NOAMGMT_STRUC, NoAQEntry);   
        MlmeFreeMemory(pAd, pMgmt->NoAQEntry.pBuffer);
        pMgmt->NoAQEntry.pBuffer = NULL;
        pMgmt->NoAQEntry.Valid = FALSE;
    }
    NdisReleaseSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);

    // P2pMs will set the GO port and call InfoReset before setting PortSubtype to PORTSUBTYPE_P2PGO, 
    // we need to add PortType check for WFD_GO_PORT to avoid disconnecting connected STA
    pHeader = &pPort->MacTab.MacTabList;
    pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextEntry;

    if ((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) || (pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO) || (pPort->PortType == WFD_GO_PORT))
    {
            if((pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) || (pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
            {
                pNextEntry = pNextEntry->Next;   
                pEntry = NULL;
                continue; 
            }
            
    }
    else if (IS_P2P_CON_CLI(pAd, pPort))
    {
            if((pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) || (pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
            {
                pNextEntry = pNextEntry->Next;   
                pEntry = NULL;
                continue; 
            }
    }

// 2011-11-25 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT        
        if (INFRA_ON(pAd->PortList[PORT_0]) && (pEntry->WlanIdxRole==ROLE_WLANIDX_BSSID))
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - Bypass  ROLE_WLANIDX_BSSID sinec STA is still in operation\n", __FUNCTION__));
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) && (pEntry->WlanIdxRole==ROLE_WLANIDX_P2P_CLIENT))
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s - Bypass ROLE_WLANIDX_P2P_CLIENT(22/23) sinec MSCLI is still in operation\n", __FUNCTION__));
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue;          
        }
#endif /* MULTI_CHANNEL_SUPPORT */

        
        // Search GO's clients according to P2pClientState
        if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
        {
            if (pEntry->ValidAsCLI == FALSE)
            {
                pNextEntry = pNextEntry->Next;   
                pEntry = NULL;
                continue;      
            }
            
            // Check if I am P2P Client
            if ((pEntry->ValidAsP2P == TRUE)
                && (pEntry->WlanIdxRole==ROLE_WLANIDX_BSSID)
                && ((pEntry->P2pInfo.P2pClientState == P2PSTATE_GO_ASSOC)
                || (pEntry->P2pInfo.P2pClientState == P2PSTATE_GO_OPERATING)))
                P2PPeer[pEntry->wcid] = 1;

            // Check if I am P2P GO
            if ((pEntry->P2pInfo.P2pClientState != P2PSTATE_CLIENT_WPS)
                && (pEntry->P2pInfo.P2pClientState != P2PSTATE_CLIENT_ASSOC)
                && (pEntry->P2pInfo.P2pClientState != P2PSTATE_CLIENT_OPERATING)
                && (pEntry->P2pInfo.P2pClientState != P2PSTATE_NONP2P_PSK)
                && (pEntry->P2pInfo.P2pClientState != P2PSTATE_NONP2P_WPS)
                && (pEntry->P2pInfo.P2pClientState != P2PSTATE_NONE))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s %d pPort->MacTab.Content[i].P2pInfo.P2pClientState = %d",__FUNCTION__, __LINE__, pEntry->P2pInfo.P2pClientState));
                pNextEntry = pNextEntry->Next;   
                pEntry = NULL;
                continue;      
                
            }
            
            //The connected P2P peer is active in this entry.
            P2PPeer[pEntry->wcid] = 1;
        }

        if (pEntry->ValidAsCLI == TRUE)
        {
            PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);
            
            if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP || pPort->PortSubtype == PORTSUBTYPE_VwifiAP || pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock, &PsQLockState);
                APCleanupPsQueue(pAd, &pEntry->PsQueue);
                PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock, &PsQLockState);
#if UAPSD_AP_SUPPORT
                if (UAPSD_MR_IS_ENTRY_ACCESSIBLE(pEntry))
                {
                    NdisAcquireSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
                    ApUapsdCleanupUapsdQueue(pAd, pEntry, TRUE);
                    NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);

                    pAd->UAPSD.UapsdSWQBitmap &= (~(1 << (pEntry->UAPSDQAid)));
                    DBGPRINT(RT_DEBUG_TRACE, ("Client with apsd capable LEAVE (pEntry->UAPSDQAid= %d, pAd->UAPSD.UapsdSWQBitmap=%d)\n", pEntry->UAPSDQAid, pAd->UAPSD.UapsdSWQBitmap));      
                }   
#endif
            }
            DBGPRINT(RT_DEBUG_TRACE, (" PsQueue.Number %d...\n", pEntry->PsQueue.Number ));
            BATableTearRECEntry(pAd,pPort, 0, (UCHAR)pEntry->wcid, TRUE);
            BATableTearORIEntry(pAd, pPort,0, (UCHAR)pEntry->wcid, TRUE, TRUE);
            pEntry->ValidAsCLI = FALSE;

            if(pPort->MacTab.Size == 0)
            {
                DBGPRINT(RT_DEBUG_TRACE, (" mac table size 0\n"));
            }
            pPort->MacTab.Size--;
            
                
            // Before reset MacTable, send disassociation packet to client (except concurrent GO)
             if ((pEntry->Sst == SST_ASSOC) && (!IS_P2P_CON_GO(pAd,pPort)))
            {
                //indicate disassoc event to os
                APPrepareDisassocPara(pAd, pEntry->Addr,
                DOT11_DISASSOC_REASON_PEER_DISASSOCIATED | Reason, &disassocPara);

                //
                // Send NDIS indication
                //
                ApPlatformIndicateDot11Status(pAd,PortNum, NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocPara, sizeof(DOT11_DISASSOCIATION_PARAMETERS));

                // if STA+AP channel changed, there is no need to send DISASSOC frames to STAs
                if ((pPort->SoftAP.ApCfg.StaChannel == pPort->Channel) ||
                    (pPort->SoftAP.ApCfg.StaCentralChannel == pPort->CentralChannel))
                {

                    //  send out a DISASSOC request frame
                    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
                    if (NStatus != NDIS_STATUS_SUCCESS) 
                    {
                        PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
                        return;
                    }

                    Reason = REASON_DISASSOC_INACTIVE;
                    DBGPRINT(RT_DEBUG_ERROR, ("mac table reset ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",Reason,pEntry->Addr[0],
                        pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));
                    MgtMacHeaderInit(pAd,pPort, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pPort->PortCfg.Bssid);
                    MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
                    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);               

                    Delay_us(2000);                
                }   
            }
            //pPort->MacTab.Hash[HashIdx] = pPort->MacTab.Content[i].pNext;

                    
            PlatformZeroMemory(pEntry->Addr,6);
            SetAsicWcid.WCID = pEntry->wcid;
            SetAsicWcid.SetTid = 0xffffffff;
            SetAsicWcid.DeleteTid = 0xffffffff;
            SetAsicWcid.DeleteTid = 0xffffffff;
            PlatformZeroMemory(SetAsicWcid.Addr,MAC_ADDR_LEN);
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_ASIC_WCID, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));
            
            //MtAsicDelWcidTab(pAd, i);
        }     
        pNextEntry = pNextEntry->Next;  
        pEntry = NULL;
    }

    // Reset all entries starts from 2 
    // idx#0 reserved for multicast; idx#1 reserved for wlan sta
    pHeader = &pPort->MacTab.MacTabList;
    pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextEntry;

        if ((pPort->PortSubtype != PORTSUBTYPE_P2PGO) || (P2PPeer[pEntry->wcid] == 0))
        {

            UCHAR Wcid = pEntry->wcid;
            USHORT   WlanRole = pEntry->WlanIdxRole;
            PMAC_TABLE_ENTRY TmpNextEntry = pEntry->Next;
            if(pEntry->wcid <MAX_LEN_OF_MAC_TABLE)
            {
                PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);

                PlatformMoveMemory(&backupTimer, &pEntry->RetryTimer, sizeof(MTK_TIMER_STRUCT));
                PlatformZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
                PlatformMoveMemory(&pEntry->RetryTimer, &backupTimer, sizeof(MTK_TIMER_STRUCT));
            }
            pEntry->wcid = Wcid;
            pEntry->Aid= Wcid;
            pEntry->WlanIdxRole = WlanRole;
            pEntry->pPort = pPort;
            pEntry->Next = TmpNextEntry;
        
        }   

        pNextEntry = pNextEntry->Next;      
        pEntry = NULL;
    }

    //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);

    if (pPort->MacTab.Size == 0)
    {
        pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
    }
    return;

}

USHORT
GetWlanRoleType(
    IN PMP_PORT        pPort
    )
{
    USHORT WlanRole = ROLE_WLANIDX_UNKNOW;
    if(pPort->PortType== EXTSTA_PORT)
    {
        if(ADHOC_ON(pPort))
        {
            WlanRole = ROLE_WLANIDX_CLIENT;
        }
        else
        {
            WlanRole = ROLE_WLANIDX_BSSID;        
        }

    }
    else if(pPort->PortType == WFD_CLIENT_PORT)
    {
        WlanRole = ROLE_WLANIDX_P2P_CLIENT;
    }
    else if ((pPort->PortType == WFD_DEVICE_PORT))
    {
        WlanRole = ROLE_WLANIDX_P2P_DEVICE;
    }
    else if ((pPort->PortType == WFD_GO_PORT) || (pPort->PortType == EXTAP_PORT))
    {
        WlanRole = ROLE_WLANIDX_CLIENT;
    }
    else 
    {
        WlanRole = ROLE_WLANIDX_UNKNOW;
        DBGPRINT(RT_DEBUG_TRACE, ("%s at Line(%d), the WlanIdxRole = ROLE_WLANIDX_UNKNOW !!!!!!!!!!!!!!\n", __FUNCTION__, __LINE__));
    } 

    return WlanRole;
}

VOID
MacTableSetToInitState(
    IN PMP_ADAPTER   pAd, 
    IN PMP_PORT        pPort,
    IN PMAC_TABLE_ENTRY pEntry,
    IN BOOLEAN  ResetAll
    )
{
    MTK_TIMER_STRUCT backupTimer;
    BOOLEAN Cancelled;
    UCHAR Wcid = pEntry->wcid;
    PMAC_TABLE_ENTRY NextEntry = pEntry->Next;
    USHORT RoleType = pEntry->WlanIdxRole;

    if(ResetAll)
    {
        if(pEntry->wcid <MAX_LEN_OF_MAC_TABLE)
        {
            PlatformCancelTimer(&pEntry->RetryTimer, &Cancelled);

            PlatformMoveMemory(&backupTimer, &pEntry->RetryTimer, sizeof(MTK_TIMER_STRUCT));
            PlatformZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
            PlatformMoveMemory(&pEntry->RetryTimer, &backupTimer, sizeof(MTK_TIMER_STRUCT));
        }
    }
    
    pEntry->wcid = Wcid;
    pEntry->Aid = Wcid;
    pEntry->Next = NextEntry;

    pEntry->WlanIdxRole = RoleType;
    
    pEntry->ValidAsWDS = FALSE;

    pEntry->pAd = pAd;
    pEntry->pPort = pPort;
    pEntry->CMTimerRunning = FALSE;
    pEntry->RSNIE_Len = 0;
    PlatformZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
    pEntry->AuthMode = pPort->PortCfg.AuthMode;
    pEntry->WepStatus = pPort->PortCfg.WepStatus;

    if ((pEntry->AuthMode < Ralink802_11AuthModeWPA) || (pEntry->AuthMode == Ralink802_11AuthModeAutoSwitch))
    pEntry->WpaState = AS_NOTUSE;
    else
    pEntry->WpaState = AS_INITIALIZE;
    pEntry->GTKState = REKEY_NEGOTIATING;
    pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
    pEntry->PairwiseKey.KeyLen = 0;
    pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
    pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
    AsicRemovePairwiseKeyEntry(pAd, (UCHAR)pEntry->wcid);
    pEntry->Sst = SST_NOT_AUTH;
    pEntry->AuthState = AS_NOT_AUTH;
    pEntry->Aid = (USHORT)pEntry->wcid;  //0;
    pEntry->CapabilityInfo = 0;
    pEntry->PsMode = PWR_ACTIVE;
    pEntry->MaxSupportedRate = RATE_11;
    pEntry->CurrTxRate = RATE_11;
    pEntry->PsQIdleCount = 0;
    pEntry->NoDataIdleCount = 0;
    // For rate tuning record -------------->
    if ((pEntry->HTCapability.MCSSet[0] == 0xff) &&
    (pEntry->HTCapability.MCSSet[1] == 0xff) &&
    (pPort->CommonCfg.TxStream > 1) &&
    ((pPort->CommonCfg.TxStream == 2) || (pEntry->HTCapability.MCSSet[2] == 0x0)))
    {
    pEntry->mcsGroup = 2;
    }
    else
    {
    pEntry->mcsGroup = 1;
    }

    pEntry->lastRateIdx = 0xFF;
    pEntry->lowTrafficCount = 0;
    pEntry->perThrdAdj = PER_THRD_ADJ;

    pEntry->fLastSecAccordingRSSI = FALSE;
    pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
    pEntry->CurrTxRateIndex = 0;
    pEntry->CurrTxRateStableTime = 0;
    pEntry->TxRateUpPenalty = 0;
    // For rate tuning record --------------<

    InitializeQueueHeader(&pEntry->PsQueue);
#if UAPSD_AP_SUPPORT
    UAPSD_MR_ENTRY_INIT(pEntry);
#endif    
}

/*
    ==========================================================================
    Description:
        Add and new entry into MAC table
    ==========================================================================
 */
MAC_TABLE_ENTRY *MacTableInsertEntry(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT        pPort,
    IN  PUCHAR pAddr,
    IN BOOLEAN  CleanAll
    ) 
{
    PMAC_TABLE_ENTRY pEntry = NULL;
    MP_RW_LOCK_STATE LockState;
    BOOLEAN IsEntryExist = FALSE;
    USHORT   RoleType =0;
    
    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return NULL;
    }

//    RoleType = GetWlanRoleType(pPort);
    // if FULL, return

    // allocate one MAC entry
    //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
    PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    // Both AP & STA co-exist and share the same MAC Table.
    // So we need to distingulish bssid entry from other entries like direect link.
    
    // If insert with forward, start to insert mac entry from the beginning.
    // If insert with backward, start to insert mac entry from the end.
        // pick up the first available vacancy

    pEntry = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&pAd->MacTablePool.MacTabPoolList); 

    if(pEntry ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s pEntry = NULL at Line(%d)\n", __FUNCTION__, __LINE__));
        return NULL;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s at Line(%d) get Entry with wcid = %d, wlanRole = %d \n", __FUNCTION__, __LINE__, pEntry->wcid, pEntry->WlanIdxRole));

        
    MacTableSetToInitState(pAd, pPort, pEntry, CleanAll);

    COPY_MAC_ADDR(pEntry->Addr, pAddr);
    pPort->MacTab.Size ++;      
    pEntry->WlanIdxRole = RoleType;
        
            // Add this pAddr into ASIC WCID Table Outside This function.
            // Because Spin Lock will raise to > PASSIVE_LEVEL, so that VenderRequest runs only at PASSIVE_LEVEL.
            //MTEnqueueInternalCmd(pAd, pPort, MTK_OID_N5_SET_CLIENT_MAC_ENTRY, pEntry, sizeof(MAC_TABLE_ENTRY));           
            // Will add this MTK_OID_N5_SET_CLIENT_MAC_ENTRY request outside of caller.         
            DBGPRINT(RT_DEBUG_TRACE, ("MacTableInsertEntry aid =%d \n",pEntry->Aid));

    // add this MAC entry into HASH table
    if (pEntry && IsEntryExist ==FALSE)
    {
        InsertTailQueue(&pPort->MacTab.MacTabList, pEntry);  
    }

    MlmeSyncMacTableDumpInfo(pPort);

    //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    return pEntry;
}

/*
    ==========================================================================
    Description:
        Delete a specified client from MAC table
    ==========================================================================
 */
    
BOOLEAN 
MacTableDeleteAndResetEntry(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN UCHAR wcid,
    IN PUCHAR pAddr,
    IN BOOLEAN bForcedDelete
    )
{
    MAC_TABLE_ENTRY *pEntry;
    RT_SET_ASIC_WCID    SetAsicWcid;
    MP_RW_LOCK_STATE    LockState;
    MP_RW_LOCK_STATE PsQLockState;

    FUNC_ENTER;
    
    if (wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        return FALSE;
    }
    PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);

    pEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, wcid);

    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, wcid));
        PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
        return FALSE;
    }
    
    if (pEntry /*&& (pEntry->ValidAsCLI ||pEntry->ValidAsWDS)*/)
    {
        
        if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
        {
            USHORT Wcid = pEntry->wcid;
            USHORT   WlanRole = pEntry->WlanIdxRole;
            PMAC_TABLE_ENTRY  TmpNextEntry = pEntry->Next;
            
            DBGPRINT(RT_DEBUG_TRACE,("MacTableDeleteEntry1 Aid= 0x%x \n",  pEntry->Aid));

            
            BATableTearRECEntry(pAd, pPort,0, (UCHAR)pEntry->Aid, TRUE);
            BATableTearORIEntry(pAd,pPort, 0, (UCHAR)pEntry->Aid, TRUE,TRUE);
            PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock, &PsQLockState);       
            APCleanupPsQueue(pAd, &pEntry->PsQueue); // return all NDIS packet in PSQ
            PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock, &PsQLockState);
#if UAPSD_AP_SUPPORT
            if (UAPSD_MR_IS_ENTRY_ACCESSIBLE(pEntry))
            {
                NdisAcquireSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
                ApUapsdCleanupUapsdQueue(pAd, pEntry, FALSE);
                NdisReleaseSpinLock(&pAd->UAPSD.UAPSDEOSPLock);   

                pAd->UAPSD.UapsdSWQBitmap &= (~(1 << pEntry->UAPSDQAid));
                DBGPRINT(RT_DEBUG_TRACE, ("Client with apsd capable LEAVE (pEntry->UAPSDQAid= %d, pAd->UAPSD.UapsdSWQBitmap=%d)\n", pEntry->UAPSDQAid, pAd->UAPSD.UapsdSWQBitmap));         
            }           
#endif

            //Resue it, so we need set it to init state.
            if(bForcedDelete == FALSE)
            {
                MacTableSetToInitState(pAd, pPort, pEntry, TRUE);
            }
            
            if (VHT_NIC(pAd))
            {
                pEntry->VhtPeerStaCtrl.MaxBW = BW_ALL;
            }

            if(pPort->MacTab.Size == 0)
                DBGPRINT(RT_DEBUG_TRACE,("MacTableDeleteAndResetEntry1 MacTab.Size = 0"));  
            
            if(pPort->MacTab.Size > 0)
                pPort->MacTab.Size --;

            // Delete this pAddr from ASIC on-chip WCID Table
            SetAsicWcid.WCID = wcid;
            SetAsicWcid.SetTid = 0xffffffff;
            SetAsicWcid.DeleteTid = 0xffffffff;
            PlatformZeroMemory(SetAsicWcid.Addr,MAC_ADDR_LEN);
            if(bForcedDelete)
            {
            MlmeSyncRemoveMacTab(pPort, pEntry);
            }

            MlmeSyncMacTableDumpInfo(pPort);
            
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_SET_ASIC_WCID, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));

            DBGPRINT(RT_DEBUG_TRACE, ("MacTableDeleteAndResetEntry1 - Total= %d\n", pPort->MacTab.Size));
        }
    }

    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    if (pPort->MacTab.Size == 0)
    {
        pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
        MtAsicUpdateProtect(pAd, pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, (ALLN_SETPROTECT), TRUE, pPort->MacTab.fAnyStationNonGF);
    }
    FUNC_LEAVE;
    return TRUE;
}

/*
    ==========================================================================
    Description:
        Look up the MAC address in the MAC table. Return NULL if not found.
    Return:
        pEntry - pointer to the MAC entry; NULL is not found
    ==========================================================================
*/
MAC_TABLE_ENTRY *MacTableLookup(
    IN  PMP_ADAPTER pAd, 
    IN  PMP_PORT    pPort,
    IN  PUCHAR  pAddr
    )
{
    PMAC_TABLE_ENTRY pEntry = NULL;
    MP_RW_LOCK_STATE LockState;
    //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
    PlatformAcquireOldRWLockForRead(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    pEntry = MlmeSyncMacTabMatchedBssid(pPort, pAddr);

    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
    }   
    //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    return pEntry;
}


/*
    ==========================================================================
    Description:
        This routine is called by ApMlmePeriodicExec() every second to check if
        1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
           out in DTIM only
        2. any client being idle for too long and should be aged-out from MAC table
        3. garbage collect PSQ
    ==========================================================================
*/
VOID MacTableMaintenance(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    USHORT        Reason = REASON_DISASSOC_STA_LEAVING;
    DOT11_DISASSOCIATION_PARAMETERS disassocPara;
    BOOLEAN     bHasAnIntel4965STA = FALSE;
    ULONG MinimumAMPDUSize = pPort->CommonCfg.DesiredHtPhy.MaxRAmpduFactor; //Default set minimum AMPDU Size to 2, i.e. 32K   
    BOOLEAN bThreeStreamsAP = TRUE;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;    
    PQUEUE_HEADER pHeader;    
    MP_RW_LOCK_STATE PsQLockState;
    
    pPort->MacTab.fAnyStationInPsm = FALSE;
    pPort->MacTab.fAnyStationNonGF = FALSE;
    pPort->MacTab.fAnyStation20Only  = FALSE;
    pPort->MacTab.fAnyStationIsLegacy = FALSE;
    pPort->MacTab.fAnyStationBadAtheros = FALSE;
    pPort->MacTab.fAnyBASession = FALSE;
    
    pHeader = &pPort->MacTab.MacTabList;
    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextMacEntry != NULL)
    {
        PMAC_TABLE_ENTRY pEntry = (PMAC_TABLE_ENTRY)pNextMacEntry; 
    
        if(pEntry == NULL)
        {
            break;     
        }
        
        if (pEntry->ValidAsCLI == FALSE)
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue;  
        }

        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue;  
        }
        
        // Check whether exist Intel 4965 STA in the BSS
        if (pEntry->IdentifyIntelSta == IOT_STRUC_INTEL_4965)
            bHasAnIntel4965STA = TRUE;

        pEntry->NoDataIdleCount ++;  
        // 0. Check existence of BA Originator session.
        if ((pEntry->TXBAbitmap !=0))
            pPort->MacTab.fAnyBASession = TRUE;

        // 0. STA failed to complete association should be removed to save MAC table space.
        if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= MAC_TABLE_ASSOC_TIMEOUT))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x fail to complete ASSOC in %d sec\n",
                    pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
                    pEntry->Addr[4],pEntry->Addr[5],MAC_TABLE_ASSOC_TIMEOUT));
            APPrepareDisassocPara(pAd, pEntry->Addr, DOT11_DISASSOC_REASON_PEER_DISASSOCIATED | Reason, &disassocPara);

            MacTableDeleteAndResetEntry(pAd, pPort, (USHORT)pEntry->wcid, pEntry->Addr, TRUE);
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue;  
        }

        // 1. check if there's any associated STA in power-save mode. this affects outgoing
        //    MCAST/BCAST frames should be stored in PSQ till DtimCount=0
        if (pEntry->PsMode == PWR_SAVE)
            pPort->MacTab.fAnyStationInPsm = TRUE;

        if (READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_20)
            pPort->MacTab.fAnyStation20Only  = TRUE;

        if (READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg) != MODE_HTGREENFIELD)
            pPort->MacTab.fAnyStationNonGF = TRUE;

        if ((READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg) == MODE_OFDM) || (READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg) == MODE_CCK))
        {
            pPort->MacTab.fAnyStationIsLegacy = TRUE;
        }
        
        // Get minimum AMPDU size from STA      
        if (MinimumAMPDUSize > pEntry->MaxRAmpduFactor) 
        {
            MinimumAMPDUSize = pEntry->MaxRAmpduFactor;                     
        }
        
        if ((pEntry->bIAmBadAtheros) )
        {
            pPort->MacTab.fAnyStationBadAtheros = TRUE;
            if (pPort->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
                MtAsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, pPort->MacTab.fAnyStationNonGF);
        }

        // 2. delete those MAC entry that has been idle for a long time
        if (((pEntry->NoDataIdleCount >= pPort->SoftAP.ApCfg.AgeoutTime) && (pPort->SoftAP.ApCfg.AgeoutTime != 0)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
                    pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
                    pEntry->Addr[4],pEntry->Addr[5],pPort->SoftAP.ApCfg.AgeoutTime));
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
            if (NDIS_WIN8_ABOVE(pAd) && (IS_P2P_GO_OP(pPort) || IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_REGISTRA(pPort)
                || (IS_P2P_MS_GO(pAd, pPort) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC))))
            {
                // Peer ageout act like receive disassoc req action. So call this function to 
                GoPeerDisassocReq(pAd, pEntry, pEntry->Addr);
            }
#endif
            ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
            APPrepareDisassocPara(pAd, pEntry->Addr, DOT11_DISASSOC_REASON_PEER_DISASSOCIATED | Reason, &disassocPara);

            //
            // Send NDIS indication
            //
            ApPlatformIndicateDot11Status(pAd, pPort->PortNumber, NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocPara, sizeof(DOT11_DISASSOCIATION_PARAMETERS));
            MacTableDeleteAndResetEntry(pAd, pPort,pEntry->Aid, pEntry->Addr, TRUE);
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue;  
        }

        // 3. garbage collect the PsQueue if the STA has being idle for a while
        if (pEntry->PsQueue.Head)
        {
            pEntry->PsQIdleCount ++;  
            if (pEntry->PsQIdleCount > 2) 
            {
                //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
                PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
                APCleanupPsQueue(pAd, &pEntry->PsQueue);
                PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
                //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
                pEntry->PsQIdleCount = 0;
            }
        }
        else
        {
            pEntry->PsQIdleCount = 0;
        }

#if UAPSD_AP_SUPPORT
        if(IS_AP_SUPPORT_UAPSD(pAd, pPort) && (pEntry))
        {
            ApUapsdQueueMaintenance(pAd, pEntry);
        }
#endif

        if (READ_PHY_CFG_NSS(pAd, &pEntry->MaxPhyCfg) != NSS_2)
        {
            bThreeStreamsAP = FALSE;
        }
            
        pNextMacEntry = pNextMacEntry->Next;  
    }
    
    if(VHT_NIC(pAd))
        pPort->CommonCfg.BaLimit = 30;
    else if ((pAd->HwCfg.MACVersion&0xffff0000) != 0x28600000)
    {
        pPort->CommonCfg.BaLimit = 14;
    }
    else
    {
        pPort->CommonCfg.BaLimit = 7;
    }

    // Write minimum AMPDU size to MAC register
    if(VHT_NIC(pAd))
        MinimumAMPDUSize = (0x003E0FFF | (MinimumAMPDUSize << 12));
    else
        MinimumAMPDUSize = (0x000E0FFF | (MinimumAMPDUSize << 12));
    
    if(pPort->SoftAP.ApCfg.MAX_PSDU_LEN != MinimumAMPDUSize)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Write 0x%x to MAC register MAX_LEN_CFG (offset: 0x%x)\n", MinimumAMPDUSize, MAX_LEN_CFG));
        RTUSBWriteMACRegister(pAd, MAX_LEN_CFG, MinimumAMPDUSize);      
        pPort->SoftAP.ApCfg.MAX_PSDU_LEN = MinimumAMPDUSize;
    }

    // Update Intel 4965 STA information for reset bulk out mechanism
    pPort->CommonCfg.IOTestParm.bIntel4965 = bHasAnIntel4965STA;
    
    // 4. garbage collect pPort->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
    //    stale in queue. Since MCAST/BCAST frames always been sent out whenever 
    //    DtimCount==0, the only case to let them stale is surprise removal of the NIC,
    //    so that ASIC-based Tbcn interrupt stops and DtimCount dead.
    if (pPort->MacTab.McastPsQueue.Head)
    {
        pPort->MacTab.PsQIdleCount ++;
        if (pPort->MacTab.PsQIdleCount > 1)
        {
            //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
            PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
            APCleanupPsQueue(pAd, &pPort->MacTab.McastPsQueue);
            PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
            //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
            pPort->MacTab.PsQIdleCount = 0;
        }
    }
    else
        pPort->MacTab.PsQIdleCount = 0;
}


//
// Are all STAs RDG capable
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  TRUE: All STAs RDG capable.
//  FALSE: At least one STA is not RDG capable.
//
BOOLEAN AreAllStaRdgCapable(
    IN PMP_ADAPTER pAd)
{
    ULONG i = 0;
    PMAC_TABLE_ENTRY pEntry = NULL;
    BOOLEAN bRDG = FALSE;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];

    PMAC_TABLE_ENTRY pNextEntry = NULL;      
    PQUEUE_HEADER pHeader;

    pHeader = &pPort->MacTab.MacTabList;
    pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextEntry;

        if(pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
        {
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue;  
        }

        if (pEntry->ValidAsCLI == FALSE)
        {
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue;  
        }

        if (pEntry->HTCapability.ExtHtCapInfo.RDGSupport)
        {
            bRDG = TRUE;
            pNextEntry = pNextEntry->Next; 
            pEntry = NULL;
        }
        else
        {
            bRDG = FALSE;
            break;
        }

        pNextEntry = pNextEntry->Next;      
        pEntry = NULL;
    }

    return bRDG;
}

VOID ApUpdateAccessControlList(
    IN PMP_ADAPTER pAd)
{
    USHORT   AclIdx;
    BOOLEAN  Matched;
    HEADER_802_11 DisassocHdr;
    USHORT      Reason;
    PCHAR       pOutBuffer = NULL;
    NDIS_STATUS NStatus;
    ULONG       FrameLen = 0;
    PMAC_TABLE_ENTRY pEntry = NULL;
    PMAC_TABLE_ENTRY pNextEntry = NULL;   
    PQUEUE_HEADER pHeader;
    
    PMP_PORT pPort = pAd->PortList[PORT_0];
    // ACL is disabled. do nothing about the MAC table
    if (pPort->SoftAP.AccessControlList.Policy == ACL_POLICY_DISABLE)
        return;

    pHeader = &pPort->MacTab.MacTabList;
    pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextEntry;

        if((pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) || (pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))       
        {
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue;  
        }

       if (! pEntry->ValidAsCLI) 
        {
            pNextEntry = pNextEntry->Next;   
            pEntry = NULL;
            continue;  
        }

        Matched = FALSE;
        
        for (AclIdx = 0; AclIdx < pPort->SoftAP.AccessControlList.Num; AclIdx++)
        {
            if (MAC_ADDR_EQUAL(pEntry->Addr, pPort->SoftAP.AccessControlList.Entry[AclIdx].Addr))
            {
                Matched = TRUE;
                break;
            }
        }

        if ((Matched == FALSE) && (pPort->SoftAP.AccessControlList.Policy == ACL_POLICY_POSITIVE_LIST))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("STA not on positive ACL. remove it...\n"));
            // Before reset the MacTable, send disassociation packet to the client.
            if (pEntry->Sst == SST_ASSOC)
            {
                //  send out a DISASSOC request frame
                NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
                if (NStatus != NDIS_STATUS_SUCCESS) 
                {
                    MacTableDeleteAndResetEntry(pAd,pPort, pEntry->Aid, pEntry->Addr, TRUE);
                    return;
                }

                Reason = REASON_NO_LONGER_VALID;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: ASSOC - Send DISASSOC; Reason = %d frame to %x %x %x %x %x %x \n", 
                    __FUNCTION__, Reason, 
                    pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], 
                    pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));

                MgtMacHeaderInit(pAd,pPort, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pPort->PortCfg.Bssid);
                MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
                NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);               

                Delay_us(2000);
            }
            MacTableDeleteAndResetEntry(pAd, pPort,pEntry->Aid, pEntry->Addr, TRUE);
        }
        else if ((Matched == TRUE) && (pPort->SoftAP.AccessControlList.Policy == ACL_POLICY_NEGATIVE_LIST))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("STA on negative ACL. remove it...\n"));
            // Before reset the MacTable, send disassociation packet to the client.
            if (pEntry->Sst == SST_ASSOC)
            {
                //  send out a DISASSOC request frame
                NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
                if (NStatus != NDIS_STATUS_SUCCESS) 
                {
                    MacTableDeleteAndResetEntry(pAd, pPort,pEntry->Aid, pEntry->Addr, TRUE);
                    return;
                }

                Reason = REASON_NO_LONGER_VALID;
                DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",Reason,pEntry->Addr[0],
                    pEntry->Addr[1],pEntry->Addr[2],
                    pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));
                MgtMacHeaderInit(pAd, pPort,&DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pPort->PortCfg.Bssid);
                MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
                NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);               

                Delay_us(2000);
            }
            MacTableDeleteAndResetEntry(pAd, pPort,pEntry->Aid, pEntry->Addr, TRUE);
        }

        pNextEntry = pNextEntry->Next;
        pEntry = NULL;
    }
}

VOID APIndicateAssociationStatus(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR           pAddr)
{
    DOT11_ASSOCIATION_START_PARAMETERS          assocStart;
    ULONG                                       assocCompSize;
    PDOT11_ASSOCIATION_COMPLETION_PARAMETERS    assocComp;
    PUCHAR              pTempPtr = NULL;
    PHEADER_802_11      pHdr80211;
    PMP_PORT pPort = pAd->PortList[PORT_0];
    //
    // Allocate enough memory for ASSOCIATION_COMPLETE indication. If allocation fails,
    // we skip this beaconing station.
    //
    assocCompSize = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS) +
                    pAd->pTxCfg->BeaconBufLen + // for beacon
                    sizeof(ULONG);                  // for single entry PHY list

    PlatformAllocateMemory(pAd, &assocComp, assocCompSize);
    
    if (assocComp == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("APIndicateAssociationStatus, Allocate assocComp for indication failed\n"));
        return;
    }

    //
    // Indicate ASSOCIATION_START
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(assocStart.Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_ASSOCIATION_START_PARAMETERS_REVISION_1,
                                sizeof(DOT11_ASSOCIATION_START_PARAMETERS));

    assocStart.uIHVDataOffset = 0;
    assocStart.uIHVDataSize = 0;

    PlatformMoveMemory(&assocStart.SSID, &pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
    COPY_MAC_ADDR(&assocStart.MacAddr, pAddr);

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_ASSOCIATION_START,
                        NULL,
                        &assocStart,
                        sizeof(DOT11_ASSOCIATION_START_PARAMETERS));


    DBGPRINT(RT_DEBUG_TRACE, ("APIndicateAssociationStatus: NDIS_STATUS_DOT11_ASSOCIATION_START, MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    pAddr[0], pAddr[1], pAddr[2],
                    pAddr[3], pAddr[4], pAddr[5]));
    //
    // Indicate ASSOCIATION_COMPLETE
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(assocComp->Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_ASSOCIATION_COMPLETION_PARAMETERS_REVISION_1,
                                sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));

    COPY_MAC_ADDR(&assocComp->MacAddr, pAddr);
    assocComp->uStatus = 0;

    assocComp->bReAssocReq = FALSE;
    assocComp->bReAssocResp = FALSE;
    assocComp->uAssocReqOffset = 0;
    assocComp->uAssocReqSize = 0;
    assocComp->uAssocRespOffset = 0;
    assocComp->uAssocRespSize = 0;

    //
    // Append the beacon information of this beaconing station. 
    //
    pTempPtr = Add2Ptr(assocComp, sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));
    PlatformMoveMemory(pTempPtr, pAd->pTxCfg->BeaconBuf, pAd->pTxCfg->BeaconBufLen);

    // Update TA on Beacon frame.
    pHdr80211 = (PHEADER_802_11) pTempPtr;
    COPY_MAC_ADDR(pHdr80211->Addr2, pAddr);
    assocComp->uBeaconOffset = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS);
    assocComp->uBeaconSize = pAd->pTxCfg->BeaconBufLen;

    assocComp->uIHVDataOffset = 0;
    assocComp->uIHVDataSize = 0;

    //
    // Set the auth and cipher algorithm.
    //
    assocComp->AuthAlgo = pPort->PortCfg.AuthMode;
    assocComp->UnicastCipher = pPort->PortCfg.WepStatus; 
    assocComp->MulticastCipher = pPort->PortCfg.GroupCipher; 
    
    //
    // Set the PHY list. It just contains our active phy id.
    //
    assocComp->uActivePhyListOffset = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS) + 
                                        pAd->pTxCfg->BeaconBufLen;
    assocComp->uActivePhyListSize = sizeof(ULONG);
    *((ULONG UNALIGNED *)Add2Ptr(assocComp, assocComp->uActivePhyListOffset)) = pAd->StaCfg.ActivePhyId;

    assocComp->bFourAddressSupported = FALSE;
    assocComp->bPortAuthorized = FALSE;
    assocComp->DSInfo = DOT11_DS_UNKNOWN;

    assocComp->uEncapTableOffset = 0;
    assocComp->uEncapTableSize = 0;

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION,
                        NULL,
                        assocComp,
                        assocCompSize);

    DBGPRINT(RT_DEBUG_TRACE, ("PlatformIndicateAdhocAssociation: NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION, MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    pAddr[0], pAddr[1], pAddr[2],
                    pAddr[3], pAddr[4], pAddr[5]));
    //
    // Free the preallocated ASSOCIATION_COMPLETE indication structure.
    //
    PlatformFreeMemory(assocComp,assocCompSize);
}

VOID
APPlatformIndicateDisassociationStatus(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PUCHAR           pAddr,
    IN ULONG            Reason)
{
    DOT11_DISASSOCIATION_PARAMETERS  disassocParam = {0};

    //
    // Indicate DISASSOCIATION
    //
    APPrepareDisassocPara(pAd, pAddr, Reason, &disassocParam);
    PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocParam, sizeof(DOT11_DISASSOCIATION_PARAMETERS));

    DBGPRINT(RT_DEBUG_TRACE, ("Indicate NDIS_STATUS_DOT11_DISASSOCIATION [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
}


VOID
ApPlatformIndicateDot11Status(
    IN  PMP_ADAPTER   pAd,
    IN  NDIS_PORT_NUMBER          PortNumber, 
    IN  NDIS_STATUS     StatusCode,
    IN  PVOID           RequestID,
    IN  PVOID           pStatusBuffer,
    IN  ULONG           StatusBufferSize)
{
    NDIS_STATUS_INDICATION statusIndication;

    PlatformZeroMemory(&statusIndication, sizeof(NDIS_STATUS_INDICATION));

    //
    // Fill in object header
    //
    statusIndication.Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;
    statusIndication.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
    statusIndication.Header.Size = sizeof(NDIS_STATUS_INDICATION);

    // fill in the correct port number
    statusIndication.PortNumber = PortNumber;
    
    //
    // Fill in the rest of the field
    //
    statusIndication.StatusCode = StatusCode;
    statusIndication.SourceHandle = pAd->AdapterHandle;
    statusIndication.DestinationHandle = NULL;
    statusIndication.RequestId = RequestID;

    statusIndication.StatusBuffer = pStatusBuffer;
    statusIndication.StatusBufferSize = StatusBufferSize;

    //
    // Indicate the status to NDIS
    //
    //NdisMIndicateStatusEx(pAd->AdapterHandle, &statusIndication);
    PlatformIndicateStatusToNdis(pAd, &statusIndication);
}

VOID
APPrepareAssocCompletePara(
    IN  PMP_ADAPTER   pAd,
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  PUCHAR pAddr,
    IN  ULONG Status,
    IN  UCHAR ErrorSource,
    IN  BOOLEAN Reassociation,
    OUT PUCHAR RequestFrame,
    IN  USHORT RequestFrameSize,
    OUT PUCHAR ResponseFrame,
    IN  USHORT ResponseFrameSize,
    OUT PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS AssocCompletePara,
    OUT PULONG AssocCompleteParaSize)
{
    ULONG requiredAssocCompleteParaSize = 0;
    USHORT activePhyListSize;
    PDOT11_PHY_TYPE phyType;
    PUCHAR pBeaconIEStart;
    ULONG BeaconHeaderLen,BeaconIELength;
    PUCHAR  pBeaconFrame = pAd->pTxCfg->BeaconBuf;

    //ASSERT(pEntry != NULL);
    
    do
    {
        activePhyListSize = sizeof(ULONG);
        BeaconHeaderLen = sizeof(HEADER_802_11);
        BeaconIELength =  pAd->pTxCfg->BeaconBufLen - BeaconHeaderLen;

        //
        // Calculate the size of the association completion parameters
        //
        requiredAssocCompleteParaSize = sizeof(DOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS) + 
                                RequestFrameSize + 
                                ResponseFrameSize +
                                activePhyListSize +
                                BeaconIELength;

        ASSERT(*AssocCompleteParaSize >= requiredAssocCompleteParaSize);

        //
        // Store the pointer in station entry
        //
        *AssocCompleteParaSize = requiredAssocCompleteParaSize;

        //
        // Clear everything
        //
        PlatformZeroMemory(AssocCompletePara, requiredAssocCompleteParaSize);
        
        //
        // Fill in information
        //
        MP_ASSIGN_NDIS_OBJECT_HEADER(
                        AssocCompletePara->Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS_REVISION_1,
                        sizeof(DOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS));

        //
        // Status
        //
        AssocCompletePara->uStatus = Status;

        //
        // Error source
        //
        AssocCompletePara->ucErrorSource = ErrorSource;

        //
        // Station Mac address
        //
        COPY_MAC_ADDR(AssocCompletePara->PeerMacAddr, pAddr);

        //
        // Reassociation?
        //
        AssocCompletePara->bReAssocReq = AssocCompletePara->bReAssocResp = Reassociation;

        //
        // The following only applies to an successful association
        //
        //ASSERT(Status == 0);

        //
        // Request frame follows the data structure immediately
        //
        // ASSERT(RequestFrameSize != 0);
        if ((RequestFrameSize != 0) && (RequestFrame != NULL))
        {
            AssocCompletePara->uAssocReqSize = RequestFrameSize;
            AssocCompletePara->uAssocReqOffset = sizeof(DOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS);

            PlatformMoveMemory(Add2Ptr(AssocCompletePara, AssocCompletePara->uAssocReqOffset), RequestFrame, RequestFrameSize);
        }

        //
        // Response frame follows request frame
        //
        // ASSERT(ResponseFrameSize != 0);
        if ((ResponseFrameSize != 0) && (ResponseFrame != NULL))
        {
            AssocCompletePara->uAssocRespSize = ResponseFrameSize;
            AssocCompletePara->uAssocRespOffset = AssocCompletePara->uAssocReqOffset + RequestFrameSize;

            PlatformMoveMemory(Add2Ptr(AssocCompletePara, AssocCompletePara->uAssocRespOffset), ResponseFrame, ResponseFrameSize);
        }

        //
        // Active PHY list follows response frame
        //
        AssocCompletePara->uActivePhyListSize = activePhyListSize;
        AssocCompletePara->uActivePhyListOffset = AssocCompletePara->uAssocRespOffset + ResponseFrameSize;

        phyType = (PDOT11_PHY_TYPE)Add2Ptr(AssocCompletePara, AssocCompletePara->uActivePhyListOffset);
        *phyType = (DOT11_PHY_TYPE)pAd->StaCfg.ActivePhyId;
        //*phyType = 0;

        DBGPRINT(RT_DEBUG_TRACE, ("PrepareAssocCompletePara: pAd->StaCfg.ActivePhyId = %d", pAd->StaCfg.ActivePhyId));
        
        AssocCompletePara->uBeaconSize = BeaconIELength;                                                        
        AssocCompletePara->uBeaconOffset = AssocCompletePara->uActivePhyListOffset  + activePhyListSize;

        pBeaconIEStart =pBeaconFrame + BeaconHeaderLen ;
        PlatformMoveMemory(Add2Ptr(AssocCompletePara, AssocCompletePara->uBeaconOffset), pBeaconIEStart, AssocCompletePara->uBeaconSize);
        if (pEntry)
        {           
            DBGPRINT(RT_DEBUG_TRACE, ("PrepareAssocCompletePara - AuthMode =%s,UnicastCipher = %s) beaconlen = %d...\n"
                , decodeAuthAlgorithm(pEntry->AuthMode),decodeCipherAlgorithm(pEntry->WepStatus), AssocCompletePara->uBeaconSize    ));
            
            AssocCompletePara->AuthAlgo = pEntry->AuthMode;
            AssocCompletePara->UnicastCipher = pEntry->WepStatus;
            //
            // TODO: Do we need to seperate WepStatus to MulticastCipher;
            // 
            AssocCompletePara->MulticastCipher = pEntry->WepStatus;
        }
    } while (FALSE);
}

VOID
APPrepareAssocStartPara(
    IN PUCHAR pAddr,
    OUT PDOT11_INCOMING_ASSOC_STARTED_PARAMETERS AssocStartPara
    )
{
    //
    // NDIS header
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(
            AssocStartPara->Header,
            NDIS_OBJECT_TYPE_DEFAULT,
            DOT11_INCOMING_ASSOC_STARTED_PARAMETERS_REVISION_1,
            sizeof(DOT11_INCOMING_ASSOC_STARTED_PARAMETERS));

    //
    // Copy Mac address
    //
    COPY_MAC_ADDR(AssocStartPara->PeerMacAddr, pAddr);
}

NDIS_STATUS
APAllocAssocCompletePara(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          AssocReqFrameSize,
    IN  USHORT          AssocRespFrameSize,
    OUT PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS *AssocCompletePara,
    OUT PULONG          AssocCompleteParaSize
    )
{
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
    ULONG               requiredSize = 0;

    requiredSize = sizeof(DOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS)       // data structure itself
                    + AssocReqFrameSize                                     // association request frame size, can be zero
                    + AssocRespFrameSize                                    // association response frame size, can be zero
                    + sizeof(ULONG);                                        // active PHY

    PlatformAllocateMemory(pAd, AssocCompletePara, requiredSize);

    if (*AssocCompletePara == NULL)
    {
        ndisStatus = NDIS_STATUS_RESOURCES;
        DBGPRINT(RT_DEBUG_ERROR, ("APAllocAssocCompletePara:Unable to allocate %u bytes for association complete parameters.\n", requiredSize));
    }
    else
    {
        PlatformZeroMemory(*AssocCompletePara, requiredSize);
        *AssocCompleteParaSize = requiredSize;
    }

    return ndisStatus;
}

VOID
APPrepareDisassocPara(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR  pAddr,
    IN  ULONG   Reason,
    OUT PDOT11_DISASSOCIATION_PARAMETERS    DisassocPara
    )
{
    //
    // NDIS header
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(
                DisassocPara->Header,
                NDIS_OBJECT_TYPE_DEFAULT,
                DOT11_DISASSOCIATION_PARAMETERS_REVISION_1,
                sizeof(DOT11_DISASSOCIATION_PARAMETERS));

    COPY_MAC_ADDR(DisassocPara->MacAddr, pAddr);

    //
    // Reason
    //
    DisassocPara->uReason = Reason;

    //
    // IHV specific
    //
    DisassocPara->uIHVDataSize = DisassocPara->uIHVDataOffset = 0;
}

VOID
APSetDefaultCipher(
    IN PMP_PORT   pPort)
{
    switch (pPort->PortCfg.AuthMode)
    {
        case DOT11_AUTH_ALGO_80211_OPEN:
            pPort->UnicastCipherAlgorithm = DOT11_CIPHER_ALGO_WEP;
            pPort->MulticastCipherAlgorithm = DOT11_CIPHER_ALGO_WEP;
            break;
            
        case DOT11_AUTH_ALGO_WPA:
        case DOT11_AUTH_ALGO_WPA_PSK:
        case DOT11_AUTH_ALGO_RSNA:
        case DOT11_AUTH_ALGO_RSNA_PSK:
            pPort->UnicastCipherAlgorithm = DOT11_CIPHER_ALGO_CCMP;
            pPort->MulticastCipherAlgorithm = DOT11_CIPHER_ALGO_CCMP;
            break;
        default:
            DBGPRINT(RT_DEBUG_ERROR, ("APSetDefaultCipher: Invalud AuthMode(=%s)\n", decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
            ASSERT(FALSE);
            break;
    }
}

VOID 
ApRestartAPHandler(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort)
{
    if ((pPort->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pPort->Channel))
    {
        if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
        {
            RadarDetectionStop(pAd);
        }
        pPort->CommonCfg.RadarDetect.RDMode = RD_SWITCHING_MODE;
        pPort->CommonCfg.RadarDetect.CSCount = 0;
        DBGPRINT(RT_DEBUG_TRACE, ("APSetInformation: Channel change to ch%d, will do Channel-Switch-Announcement\n", pPort->Channel));
    }
    else
    {
        if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
        {
            RadarDetectionStop(pAd);
        }
        pPort->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;
        pPort->CommonCfg.RadarDetect.CSCount = 0;
        
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
        APStop(pAd,pPort);
        APStartUp(pAd,pPort);
    }
}

VOID 
ApStartApRequestHandler(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort)
{
    ApSetState(pPort, AP_STATE_STARTING);
    APStartUp(pAd,pPort);
    pPort->LastIndicateRate = 0;
    PlatformIndicateNewLinkSpeed(pAd, pPort->PortNumber, OPMODE_AP);
    ApSetState(pPort, AP_STATE_STARTED);
    if(pPort->SoftAP.bAPStart)
    {
        if (pPort->PortNumber == PORT_0 && pPort->PortSubtype == PORTSUBTYPE_SoftapAP) 
            pAd->OpMode = OPMODE_AP;         
        if (pPort->PortNumber == PORT_1 && pPort->PortSubtype == PORTSUBTYPE_VwifiAP)
            pAd->OpMode = OPMODE_APCLIENT;
        DBGPRINT(RT_DEBUG_TRACE, ("ApStartApRequestHandler:pAd->OpMode(%d) radio off\n",pAd->OpMode));            
            
    }
}

//
// TODO: To be Removed.
//
VOID ApAsicEnableBssSync(MP_ADAPTER *pAd) 
{
#define WF_LPON_TOP_BASE        0x24000
#define LPON_T2TPCR          (WF_LPON_TOP_BASE + 0x108)  /* 0x24108 */
#define LPON_MPTCR2          (WF_LPON_TOP_BASE + 0x110)  /* 0x24110 */

    UINT32 bitmask = 0;
    UINT32 Value;
    INT bss_idx = 0; // TODO: this index may carried by parameters!
    PMP_PORT    pPort = pAd->BeaconPort.pApGoPort;
    ULONG       TbttTimeControlRegister = LPON_T0TPCR;
    ULONG       MacTimerCtrlRegValue; //ValueMacPeriodicTimerControlRegisterValue = 0x19; //PCI/USB has its different value
    ULONG       MacTimerCtrlRegValue2; //ValueMacPeriodicTimerControlRegisterValue = 0x19; //PCI/USB has its different value

    DBGPRINT(RT_DEBUG_TRACE, ("--->%s():\n", __FUNCTION__));

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: AP/GO Port could not be NULL\n", __FUNCTION__));    
        return;
    }

    //
    // Needs to check HW multiple BSSID capability.
    // MT7603 supports only 0 ~ 2 multiple BSSID
    //
    if (pPort->PortNumber > 2)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: AP/GO Port not support on port number(=%d) \n", __FUNCTION__, pPort->PortNumber));    
        return;
    }

if (pPort->CommonCfg.BeaconPeriod == 0)
    pPort->CommonCfg.BeaconPeriod = 100;
    
    bss_idx = pPort->OwnMACAddressIdx;
    switch (bss_idx)
    {
        case 0:
            TbttTimeControlRegister = LPON_T0TPCR;
            MacTimerCtrlRegValue = 0x19;

            /* Configure Beacon interval */
            HW_IO_READ32(pAd, LPON_T0TPCR, &Value);
            Value = 0;
            Value &= ~BEACONPERIODn_MASK;
            Value |= BEACONPERIODn(pPort->CommonCfg.BeaconPeriod);
            Value |= TBTTn_CAL_EN;
            HW_IO_WRITE32(pAd, LPON_T0TPCR, Value);

            // Eanble TBTT periodic time comparsison for setting of BSS0
//            HW_IO_WRITE32(pAd, LPON_MPTCR0, 0x19);
            HW_IO_WRITE32(pAd, LPON_MPTCR0, 0x09);

#if 0
            bitmask = 0xff << (bss_idx * 8);
            HW_IO_READ32(pAd, LPON_PISR, &Value);
            Value &= (~bitmask);
            Value |= (0x50 << (bss_idx * 8));
            //mac_val =0x50505050;
            HW_IO_WRITE32(pAd, LPON_PISR, Value);
#endif

            HW_IO_READ32(pAd, ARB_SCR, &Value);
#ifdef RELEASE_EXCLUDE
            /*
            *   20140103
            *
            *   1. when AC0 has large volume traffic to send,
            *   Bcn pkt has chance to stuck between HIF and PSE,
            *   it will take long time to arrive PSE.
            *   which will cause ARB has no bcn to send,
            *   when encounter this case, if BIT30 is 1, HW will block other AC traffic to send out.
            *
            *   2. ARB_SCR BIT28 has bug at present, it cannot abort the Bcn Tx in some case.
            *   when it cannot abort, it will block other AC's transmisstion.
            *
            *   TODO: Carter, check these CR settings after the above case solved.
            */
#endif
            Value &= (~(1<<30)); // work-around to make BCN need to content with other ACs
            Value &= (~(1<<31)); // work-around to make BMC need to content with other ACs - 20140109 discussion.
            Value |= (0x1 << (bss_idx*2));
            //Value = Value & 0xefffffff;
            HW_IO_WRITE32(pAd, ARB_SCR, Value); 

            /* Start Beacon Queue */
            HW_IO_READ32(pAd, ARB_BCNQCR0, &Value);
            Value |= (0x1 << bss_idx) ;
            HW_IO_WRITE32(pAd, ARB_BCNQCR0, Value);
            
            break;

        case 1:
            TbttTimeControlRegister = LPON_T1TPCR;
            MacTimerCtrlRegValue = 0x1900;

            /* Configure Beacon interval */
            HW_IO_READ32(pAd, LPON_T1TPCR, &Value);
            Value = 0;
            Value &= ~BEACONPERIODn_MASK;
            Value |= BEACONPERIODn(pPort->CommonCfg.BeaconPeriod);
            Value |= TBTTn_CAL_EN;
            HW_IO_WRITE32(pAd, LPON_T1TPCR, Value);

//            HW_IO_WRITE32(pAd, LPON_MPTCR0, 0x1900);
            HW_IO_WRITE32(pAd, LPON_MPTCR0, 0x0900);

#if 0
            bitmask = 0xff << (bss_idx * 8);
            HW_IO_READ32(pAd, LPON_PISR, &Value);
            Value &= (~bitmask);
            Value |= (0x50 << (bss_idx * 8));
            //mac_val =0x50505050;
            HW_IO_WRITE32(pAd, LPON_PISR, Value);
#endif
            HW_IO_READ32(pAd, ARB_SCR, &Value);
#ifdef RELEASE_EXCLUDE
            /*
            *   20140103
            *
            *   1. when AC0 has large volume traffic to send,
            *   Bcn pkt has chance to stuck between HIF and PSE,
            *   it will take long time to arrive PSE.
            *   which will cause ARB has no bcn to send,
            *   when encounter this case, if BIT30 is 1, HW will block other AC traffic to send out.
            *
            *   2. ARB_SCR BIT28 has bug at present, it cannot abort the Bcn Tx in some case.
            *   when it cannot abort, it will block other AC's transmisstion.
            *
            *   TODO: Carter, check these CR settings after the above case solved.
            */
#endif
            Value &= (~(1<<30)); // work-around to make BCN need to content with other ACs
            Value &= (~(1<<31)); // work-around to make BMC need to content with other ACs - 20140109 discussion.
            Value |= (0x1 << (bss_idx*2));
            //Value = Value & 0xefffffff;
            HW_IO_WRITE32(pAd, ARB_SCR, Value);            

            /* Start Beacon Queue */
            HW_IO_READ32(pAd, ARB_BCNQCR0, &Value);
            Value |= (0x1 << bss_idx) ;
            HW_IO_WRITE32(pAd, ARB_BCNQCR0, Value);
            
            break;
            
        case 2:
            TbttTimeControlRegister = LPON_T2TPCR;
            MacTimerCtrlRegValue2 = 0x19;

            /* Configure Beacon interval */
            HW_IO_READ32(pAd, LPON_T2TPCR, &Value);
            Value = 0;
            Value &= ~BEACONPERIODn_MASK;
            Value |= BEACONPERIODn(pPort->CommonCfg.BeaconPeriod);
            Value |= TBTTn_CAL_EN;
            HW_IO_WRITE32(pAd, LPON_T2TPCR, Value);

//            HW_IO_WRITE32(pAd, LPON_MPTCR2, 0x19);
            HW_IO_WRITE32(pAd, LPON_MPTCR2, 0x09);

#if 0
            bitmask = 0xff << (bss_idx * 8);
            HW_IO_READ32(pAd, LPON_PISR, &Value);
            Value &= (~bitmask);
            Value |= (0x50 << (bss_idx * 8));
            //mac_val =0x50505050;
            HW_IO_WRITE32(pAd, LPON_PISR, Value);
#endif

            HW_IO_READ32(pAd, ARB_SCR, &Value);
#ifdef RELEASE_EXCLUDE
            /*
            *   20140103
            *
            *   1. when AC0 has large volume traffic to send,
            *   Bcn pkt has chance to stuck between HIF and PSE,
            *   it will take long time to arrive PSE.
            *   which will cause ARB has no bcn to send,
            *   when encounter this case, if BIT30 is 1, HW will block other AC traffic to send out.
            *
            *   2. ARB_SCR BIT28 has bug at present, it cannot abort the Bcn Tx in some case.
            *   when it cannot abort, it will block other AC's transmisstion.
            *
            *   TODO: Carter, check these CR settings after the above case solved.
            */
#endif
            Value &= (~(1<<30)); // work-around to make BCN need to content with other ACs
            Value &= (~(1<<31)); // work-around to make BMC need to content with other ACs - 20140109 discussion.
            Value |= (0x1 << (bss_idx*2));
            //Value = Value & 0xefffffff;
            HW_IO_WRITE32(pAd, ARB_SCR, Value);            

            /* Start Beacon Queue */
            HW_IO_READ32(pAd, ARB_BCNQCR0, &Value);
            Value |= (0x1 << bss_idx) ;
            HW_IO_WRITE32(pAd, ARB_BCNQCR0, Value);
            
            break;
        
        default:
            TbttTimeControlRegister = LPON_T0TPCR;
            break;
    }
}

