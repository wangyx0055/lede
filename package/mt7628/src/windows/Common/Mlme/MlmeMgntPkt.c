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
    MgntPkt.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

#define DRIVER_FILE         0x00B00000

extern UCHAR   CipherWpaTemplateLen;
UCHAR   CipherWpa2TemplateLen; 
VOID
MgntPktConstructBeaconFrame(
    IN PMP_PORT  pPort,
    IN OUT PUCHAR    pOutBuffer,
    IN OUT ULONG    *pFrameLen
    )
{
    UCHAR           DsLen = 1, SsidLen, TimLen=4;
    UCHAR           RSNIe=IE_WPA, RSNIe2=IE_WPA2;
    HEADER_802_11   BcnHdr;
    LARGE_INTEGER   FakeTimestamp;
    UINT            i;
    UCHAR       tmpSupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       tmpExtRateLen = 0;
    UCHAR       SupRateLen = 0;
    PMP_ADAPTER    pAd = pPort->pAd;
    ULONG length =0;
    ULONG TmpLen;
    PUCHAR      ptr;
    UCHAR   byte0 = (UCHAR)(pPort->SoftAP.ApCfg.TimBitmap & 0x000000fe);  // skip AID#0
    UCHAR   byte1 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0x0000ff00) >> 8);
    UCHAR   byte2 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0x00ff0000) >> 16);
    UCHAR   byte3 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0xff000000) >> 24);
    UCHAR   byte4 = (UCHAR)(pPort->SoftAP.ApCfg.TimBitmap2 & 0x000000ff);
    UCHAR   byte5 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0x0000ff00) >> 8);
    UCHAR   byte6 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0x00ff0000) >> 16);
    UCHAR   byte7 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0xff000000) >> 24);
    ADD_HT_INFO_IE  AddHTInfo;  // Useful as AP.    

    if (pPort->SoftAP.ApCfg.bHideSsid)
        SsidLen = 0;
    else
        SsidLen = pPort->PortCfg.SsidLen;

    MgtMacHeaderInit(pAd,pPort, &BcnHdr, SUBTYPE_BEACON, 0, BROADCAST_ADDR, pPort->CurrentAddress);

    // In the A band should not appear 1,2,5,11
    if(pPort->Channel > 14)
    {
        tmpSupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        tmpSupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        tmpSupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        tmpSupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        tmpSupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
        tmpExtRateLen = 0;
    }
    else
    {
        PlatformMoveMemory(tmpSupRate,pPort->CommonCfg.SupRate,pPort->CommonCfg.SupRateLen);
        tmpExtRateLen = pPort->CommonCfg.ExtRateLen;
    }

    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)) 
    {
        // CH14 only supports 11b
        SupRateLen = 4;
        tmpExtRateLen = 0;
    }
    else
    {
        SupRateLen = pPort->CommonCfg.SupRateLen;
    }

    // for update framelen to TxWI later.
    MakeOutgoingFrame(pOutBuffer,                  &length,
                    sizeof(HEADER_802_11),           &BcnHdr, 
                    TIMESTAMP_LEN,                   &FakeTimestamp,
                    2,                               &pPort->CommonCfg.BeaconPeriod,
                    2,                               &pPort->SoftAP.ApCfg.CapabilityInfo,
                    1,                               &SsidIe, 
                    1,                               &SsidLen, 
                    SsidLen,                         pPort->PortCfg.Ssid,
                    1,                               &SupRateIe, 
                    1,                               &SupRateLen,
                    SupRateLen,                      tmpSupRate, 
                    1,                               &DsIe, 
                    1,                               &DsLen, 
                    1,                               &pPort->Channel,
                    END_OF_ARGS);

    if (pPort->SoftAP.ApCfg.DtimCount == 0)
        pPort->SoftAP.ApCfg.DtimCount = pPort->SoftAP.ApCfg.DtimPeriod - 1;
    else
        pPort->SoftAP.ApCfg.DtimCount -= 1;

    //
    // TIM IE(=5) order: 10
    //
   if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
   {
        //CH14 only support 11b, not to include TIM
        ;
   }
   else
   {
        ptr = pOutBuffer + length;

        *ptr = IE_TIM;
        *(ptr + 2) = pPort->SoftAP.ApCfg.DtimCount;
        *(ptr + 3) = pPort->SoftAP.ApCfg.DtimPeriod;

        if (byte0 || byte1) // there's some backlog frame for AID 1-15
        {
            *(ptr + 4) = 0;      // Virtual TIM bitmap stars from AID #0
            *(ptr + 5) = byte0;
            *(ptr + 6) = byte1;
            *(ptr + 7) = byte2;
            *(ptr + 8) = byte3;
            *(ptr + 9) = byte4;
            *(ptr + 10) = byte5;
            *(ptr + 11) = byte6;
            *(ptr + 12) = byte7;
            if (byte7)      *(ptr + 1) = 11; // IE length
            else if (byte6) *(ptr + 1) = 10; // IE length
            else if (byte5) *(ptr + 1) = 9;  // IE length
            else if (byte4) *(ptr + 1) = 8;  // IE length
            else if (byte3) *(ptr + 1) = 7;  // IE length
            else if (byte2) *(ptr + 1) = 6;  // IE length
            else if (byte1) *(ptr + 1) = 5;  // IE length
            else            *(ptr + 1) = 4;  // IE length
        }
        else if (byte2 || byte3) // there's some backlogged frame for AID 16-31
        {
            *(ptr + 4) = 2;      // Virtual TIM bitmap starts from AID #16
            *(ptr + 5) = byte2;
            *(ptr + 6) = byte3;
            *(ptr + 7) = byte4;
            *(ptr + 8) = byte5;
            *(ptr + 9) = byte6;
            *(ptr + 10) = byte7;
            if (byte7)      *(ptr + 1) = 9; // IE length
            else if (byte6) *(ptr + 1) = 8; // IE length
            else if (byte5) *(ptr + 1) = 7; // IE length
            else if (byte4) *(ptr + 1) = 6; // IE length
            else if (byte3) *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else if (byte4 || byte5) // there's some backlogged frame for AID 32-47
        {
            *(ptr + 4) = 4;      // Virtual TIM bitmap starts from AID #32
            *(ptr + 5) = byte4;
            *(ptr + 6) = byte5;
            *(ptr + 7) = byte6;
            *(ptr + 8) = byte7;
            if (byte7)      *(ptr + 1) = 7; // IE length
            else if (byte6) *(ptr + 1) = 6; // IE length
            else if (byte5) *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else if (byte6 || byte7) // there's some backlogged frame for AID 48-63
        {
            *(ptr + 4) = 6;      // Virtual TIM bitmap starts from AID #48
            *(ptr + 5) = byte6;
            *(ptr + 6) = byte7;
            if (byte7)      *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else // no backlogged frames
        {
            *(ptr + 1) = 4; // IE length
            *(ptr + 4) = 0;
            *(ptr + 5) = 0;
        }

        // bit0 means backlogged mcast/bcast
        *(ptr + 4) |= (pPort->SoftAP.ApCfg.TimBitmap & 0x01); 

        // adjust BEACON length according to the new TIM
        length += (2 + *(ptr+1));
    }    

    //
    // Channel Switch Announce order: 15
    //  
    // fill up Channel Switch Announcement Element
    //
    if ((pPort->CommonCfg.PhyMode == PHY_11A) && (pPort->CommonCfg.bIEEE80211H == 1) && (pPort->CommonCfg.RadarDetect.RDMode == RD_SWITCHING_MODE))
    {
        ptr = pOutBuffer + length;
        *ptr = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
        *(ptr + 1) = 3;
        *(ptr + 2) = 1;
        *(ptr + 3) = pPort->Channel;
        *(ptr + 4) = pPort->CommonCfg.RadarDetect.CSCount;
        ptr      += 5;
        length += 5;
    }

    //
    // ERP information(=42) order: 19
    //
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only support 11b, not to include ERP
        ;
    }
    else
    {
        // Update ERP and extended tx rate
        if ((pPort->CommonCfg.PhyMode == PHY_11BG_MIXED) || (pPort->CommonCfg.PhyMode == PHY_11G) 
            || (pPort->CommonCfg.PhyMode == PHY_11BGN_MIXED) ||(pPort->CommonCfg.PhyMode == PHY_11GN_MIXED)
            || (pPort->CommonCfg.PhyMode == PHY_11N))
        {
            //
            // fill ERP IE
            // 
            ptr = pOutBuffer + length;
            *ptr = IE_ERP;
            *(ptr + 1) = 1;
            *(ptr + 2) = pPort->SoftAP.ApCfg.ErpIeContent;
            ptr      += 3;
            length += 3;
        }
    }

    //
    // Extended Supported Rates(ID=50) order:20
    //
    if (tmpExtRateLen)
    {
        MakeOutgoingFrame(pOutBuffer+length,         &TmpLen,
                        1,                               &ExtRateIe, 
                        1,                               &tmpExtRateLen,
                        tmpExtRateLen,                   pPort->CommonCfg.ExtRate, 
                        END_OF_ARGS);
        length += TmpLen;
    }


    //
    // RSN(ID=48) order:21
     //
     if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
    {
         MakeOutgoingFrame(pOutBuffer+length,        &TmpLen,
                          1,                            &RSNIe2,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],      pPort->SoftAP.ApCfg.RSN_IE[0],
                          END_OF_ARGS);
        length += TmpLen;    
    }
    else if  ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
        MakeOutgoingFrame(pOutBuffer+length,        &TmpLen,
                          1,                            &RSNIe2,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[1],
                          pPort->SoftAP.ApCfg.RSNIE_Len[1],      pPort->SoftAP.ApCfg.RSN_IE[1],
                          END_OF_ARGS);
        length += TmpLen;
    }   

    //
    // HT capabilities(=45) order: 37
    //
    // step 5. Update HT. Since some fields might change in the same BSS.
    //
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)) 
    {
        //CH14 only supports 11B and will not carry any HT info on beacons
        ;
    }
    else
    {
        PlatformMoveMemory(&AddHTInfo, &pPort->SoftAP.ApCfg.AddHTInfoIe, sizeof(ADD_HT_INFO_IE));
        AddHTInfo.ControlChan = pPort->Channel;
    
        if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
        {
            ULONG HtLen, HtLen1;
            ULONG VhtCapIeLen, VhtOpIeLen;
            UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
            UCHAR BROADCOM2[4] = {0x0, 0x90, 0x4c, 0x34};
            HT_CAPABILITY_IE HtCapability;
            PlatformMoveMemory(&HtCapability, &pPort->SoftAP.ApCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            if (pAd->HwCfg.Antenna.field.RxPath > 1)
            {
                HtCapability.MCSSet[1] = 0xff;
            }
            else
            {
                HtCapability.MCSSet[1] = 0x00;
            }

            // add HT Capability IE 
            HtLen = sizeof(pPort->SoftAP.ApCfg.HtCapability);
            HtLen1 = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe);
            MakeOutgoingFrame(pOutBuffer+length,         &TmpLen,
                                      1,                                &HtCapIe,
                                      1,                                &HtLen,
                                     HtLen,          &HtCapability, 
                                      1,                                &AddHtInfoIe,
                                      1,                                &HtLen1,
                                     HtLen1,          &AddHTInfo, 
                              END_OF_ARGS);
            length += TmpLen;
            
            // add IEs for VHT
            if (pPort->Channel>14)
            {
                if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                {
                    VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                    MakeOutgoingFrame(pOutBuffer+length, &TmpLen,
                                        1, &VhtCapIe,
                                        1, &VhtCapIeLen,
                                        SIZE_OF_VHT_CAP_IE, &pPort->SoftAP.ApCfg.VhtCapability,
                                        END_OF_ARGS);
                    length += TmpLen;

                    // add VHT_OP_IE for VHT
                    VhtOpIeLen = SIZE_OF_VHT_OP_IE;
                    MakeOutgoingFrame(pOutBuffer+length, &TmpLen,
                                        1, &VhtOpIe,
                                        1, &VhtOpIeLen,
                                        SIZE_OF_VHT_OP_IE,  &pPort->SoftAP.ApCfg.VhtOperation,
                                        END_OF_ARGS);
                    length += TmpLen;
                }
            }
            
            {
                HtLen = SIZE_HT_CAP_IE + 4;
                HtLen1 = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe) + 4;
                MakeOutgoingFrame(pOutBuffer + length,            &TmpLen,
                              1,                                &WpaIe,
                              1,                                &HtLen,
                              4,                                &BROADCOM[0],
                             (HtLen-4),          &HtCapability, 
                              1,                                &WpaIe,
                              1,                                &HtLen1,
                              4,                                &BROADCOM2[0],
                             (HtLen1-4),          &AddHTInfo, 
                              END_OF_ARGS);
                length += TmpLen;
            }
        }
    }

    //
    // Vendor Specific(ID=221) order: Last
    //
    if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK))
    {
        MakeOutgoingFrame(pOutBuffer+length,        &TmpLen,
                          1,                            &RSNIe,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],      pPort->SoftAP.ApCfg.RSN_IE[0],
                          END_OF_ARGS);
        length += TmpLen;
    }
    else if  ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
         MakeOutgoingFrame(pOutBuffer+length,        &TmpLen,
                          1,                            &RSNIe2,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[1],
                          pPort->SoftAP.ApCfg.RSNIE_Len[1],      pPort->SoftAP.ApCfg.RSN_IE[1],
                          END_OF_ARGS);
        length += TmpLen;
    }

    // add WMM IE here
    if ((pPort->CommonCfg.bWmmCapable) || (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED))
    {
        UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 
        WmeParmIe[8] = pPort->SoftAP.ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;
        for (i=QID_AC_BE; i<=QID_AC_VO; i++)
        {
            WmeParmIe[10+ (i*4)] = (i << 5)                                         +     // b5-6 is ACI
                                   ((UCHAR)pPort->SoftAP.ApCfg.BssEdcaParm.bACM[i] << 4)     +     // b4 is ACM
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);              // b0-3 is AIFSN
            WmeParmIe[11+ (i*4)] = (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[i] << 4)           +     // b5-8 is CWMAX
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              // b0-3 is CWMIN
            WmeParmIe[12+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] & 0xff);        // low byte of TXOP
            WmeParmIe[13+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] >> 8);          // high byte of TXOP
        }

        MakeOutgoingFrame(pOutBuffer+length,         &TmpLen,
                          26,                            WmeParmIe,
                          END_OF_ARGS);
        length+= TmpLen;
    }

    if (pPort->PortCfg.AdditionalBeaconIESize > 0)
    {
        MakeOutgoingFrame(pOutBuffer+length,         &TmpLen,
                          pPort->PortCfg.AdditionalBeaconIESize ,     pPort->PortCfg.AdditionalBeaconIEData,
                          END_OF_ARGS);

        length += TmpLen;
    }

    *pFrameLen = length;

    pAd->pTxCfg->BeaconBufLen = min(length, MAX_LEN_OF_MLME_BUFFER);
    PlatformMoveMemory(pAd->pTxCfg->BeaconBuf, pOutBuffer, min(length, MAX_LEN_OF_MLME_BUFFER));
}

VOID 
MgntPktSendBeacon(
    IN PMP_PORT  pPort
    ) 
{
    PUCHAR          pOutBuffer = NULL;
    PMP_ADAPTER    pAd = pPort->pAd;
    NDIS_STATUS     NStatus;   
    ULONG           FrameLen = 0;
    
    // P2P device has its own rule to update beacon
    if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO)
        ||(pPort->PortSubtype == PORTSUBTYPE_P2PClient) )
        return;

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory    

    if (NStatus == NDIS_STATUS_SUCCESS)
    {
        MgntPktConstructBeaconFrame(pPort, pOutBuffer, &FrameLen);
    }
    else
    {
        DBGPRINT(RT_DEBUG_WARN,("%s: not enough memory\n", __FUNCTION__));        
    }
    
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    DBGPRINT(RT_DEBUG_TRACE,("MgntPktSendBeacon - <--- \n"));
    
}

VOID
MgntPktConstructProbeReqFrame(
    IN PMP_PORT  pPort,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pFrameLen
    )
{
    PUCHAR          pSupRate = NULL;
    UCHAR           SupRateLen = 0;
    PUCHAR          pExtRate = NULL;
    UCHAR           ExtRateLen = 0; 
    UCHAR           ASupRate[] = {0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6C};
    UCHAR           ASupRateLen = sizeof(ASupRate)/sizeof(UCHAR);   
    HEADER_802_11   Hdr80211;  
    CHAR  ssid[MAX_LEN_OF_SSID];
    UCHAR ssidLen = 0;
    PMP_ADAPTER    pAd = pPort->pAd;
    ULONG length = 0;
    
    PlatformZeroMemory(ssid, MAX_LEN_OF_SSID);
        
    if (pPort->ScaningChannel <= 14)
    {
        if (pPort->CommonCfg.Ch14BOnly && (pPort->ScaningChannel == CHANNEL_14))
        {
            pSupRate = pPort->CommonCfg.SupRate;
            SupRateLen = 4;
            ExtRateLen = 0;
        }
        else
        {
            pSupRate = pPort->CommonCfg.SupRate;
            SupRateLen = pPort->CommonCfg.SupRateLen;
            pExtRate = pPort->CommonCfg.ExtRate;
            ExtRateLen = pPort->CommonCfg.ExtRateLen;
        }
    }
    else
    {
        //
        // Overwrite Support Rate, CCK rate are not allowed
        //
        pSupRate = ASupRate;
        SupRateLen = ASupRateLen;
        ExtRateLen = 0;
    }

    if(pPort->PortCfg.bHiddenNetworkEnabled)
    {
        ssidLen = (UCHAR)pPort->PortCfg.DesiredSSID.uSSIDLength; // pPort->PortCfg.LastSsidLen;
        PlatformMoveMemory(ssid, pPort->PortCfg.DesiredSSID.ucSSID, ssidLen);
    }
    else
    {
        ssidLen = 0;
    }
        
    if (pAd->MlmeAux.BssType == BSS_INFRA)
        MgtMacHeaderInit(pAd, pPort,&Hdr80211, SUBTYPE_PROBE_REQ, 0, pAd->MlmeAux.Bssid, pAd->MlmeAux.Bssid);
    else
        MgtMacHeaderInit(pAd, pPort,&Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);
        
    DBGPRINT(RT_DEBUG_TRACE, ("%s:%d make probe req with SsidLen:%d SSID:%s", __FUNCTION__, __LINE__, ssidLen, ssid));
    
    MakeOutgoingFrame(pOutBuffer,               &length,
                      sizeof(HEADER_802_11),    &Hdr80211,
                      1,                        &SsidIe,
                      1,                        &ssidLen,
                      ssidLen,                  &ssid,
                      1,                        &SupRateIe,
                      1,                        &SupRateLen,
                      SupRateLen,               pSupRate,
                      END_OF_ARGS);

    if (ExtRateLen)
    {
        ULONG Tmp;
        MakeOutgoingFrame(pOutBuffer + length,            &Tmp,
                          1,                                &ExtRateIe,
                          1,                                &ExtRateLen,
                          ExtRateLen,                       pExtRate,
                          END_OF_ARGS);
        length += Tmp;
    }

    //
    // add VHT_Cap IE for VHT
    //
    if(pPort->CommonCfg.PhyMode == PHY_11VHT)
    {
        ULONG Tmp;
        UCHAR VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
        MakeOutgoingFrame(pOutBuffer+length,   &Tmp,
                            1,                   &VhtCapIe,
                            1,                   &VhtCapIeLen,
                            SIZE_OF_VHT_CAP_IE,  &pPort->CommonCfg.DesiredVhtPhy.VhtCapability,
                            END_OF_ARGS);
        length += Tmp;
    }

            // Append WSC information in probe request if WSC state is running
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if (NDIS_WIN8_ABOVE(pAd) && (pPort->PortType == WFD_CLIENT_PORT) && (pPort->PortCfg.P2PAdditionalRequestIESize > 0) && (pPort->PortCfg.P2PAdditionalRequestIEData != NULL))
            {
                ULONG Tmp;
                DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] %s - Use Microsoft IE (P2PAdditionalRequestIESize =%d) \n", __FUNCTION__, pPort->PortCfg.P2PAdditionalRequestIESize));

                MakeOutgoingFrame(pOutBuffer + length,        &Tmp,
                                  pPort->PortCfg.P2PAdditionalRequestIESize,    pPort->PortCfg.P2PAdditionalRequestIEData,
                                  END_OF_ARGS);
                length += Tmp;
            }
            else
#endif
            //if ((pPort->StaCfg.WscControl.WscEnProbeReqIE) && (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
            {
                UCHAR       WscBuf[512];
                USHORT      WscIeLen;
                ULONG Tmp;
                WscMakeProbeReqIE(pAd, pPort, FALSE, &WscBuf[0], &WscIeLen);
                MakeOutgoingFrame(pOutBuffer + length, &Tmp, WscIeLen, WscBuf, END_OF_ARGS);

                length += Tmp;
            }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            // P2P IE
            if (pPort->PortType == WFD_CLIENT_PORT)
            {
                ULONG Tmp;
                P2pMakeP2pIE(pAd, pPort, PORTSUBTYPE_P2PClient, (UCHAR)SUBTYPE_PROBE_REQ, pOutBuffer + length, &Tmp);
                length += Tmp;
            }
#endif

            // send probe request twice to patch can't receive probe response frame for hidden AP test.
            //NStatus = MlmeAllocateMemory(pAd, &pAd->pNicCfg->OutBufferForSendProbeReq);

            // added connection probility
            if (pAd->pNicCfg->OutBufferForSendProbeReq != NULL)
            {
                if (length <= MAX_VIE_LEN)
                {
                    PlatformMoveMemory(pAd->pNicCfg->OutBufferForSendProbeReq, pOutBuffer, length);
                    pAd->pNicCfg->OutBufferForSendProbeReqLen = length;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("%s Buffer to small Need size [%d] Limit size [%d] \n", __FUNCTION__, length, MAX_VIE_LEN));
                }
                //NdisCommonMiniportMMRequest(pAd, pAd->pNicCfg->OutBufferForSendProbeReq, FrameLen);
                //DBGPRINT(RT_DEBUG_TRACE, ("SYNC - send ProbeReq @ channel=%d, Len=%d\n", pPort->ScaningChannel, FrameLen));
            }

    
    *pFrameLen = length;
}

VOID 
MgntPktSendProbeRequest(
    IN PMP_PORT pPort
    )
{
    NDIS_STATUS     NStatus;
    PUCHAR          pOutBuffer = NULL;
 
    ULONG           FrameLen = 0;   
    
    PMP_ADAPTER    pAd = pPort->pAd;
    
    //
    // send probe request
    //      
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    
    if(NStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s Memory Allocate failed\n",__FUNCTION__));
        return;
    }
    MgntPktConstructProbeReqFrame(pPort, pOutBuffer, &FrameLen);  
    
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

}

VOID
MgntPktConstructAPPeerAssocActionFrame(
    IN PMP_PORT  pPort,
    IN USHORT               CapabilityInfo,
    IN MAC_TABLE_ENTRY *pEntry,
    IN USHORT               StatusCode,
    IN  USHORT       Aid,
    IN ULONG           RalinkIe,
    IN ULONG           P2PSubelementLen,
    IN UCHAR             HTCapability_Len,
    IN BOOLEAN         indicateAssocComplete,
    IN UCHAR           errorSource,//DOT11_ASSOC_ERROR_SOURCE_OTHER;
    IN ULONG           errorStatus,    
    IN PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS assocCompletePara,
    IN ULONG assocCompleteParaSize,
    IN PUCHAR        Addr2,
    IN BOOLEAN      bAssocReq,
    IN BOOLEAN      bReAssocReq,
    IN MLME_QUEUE_ELEM *Elem,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pLen
    )
{
    PMP_ADAPTER    pAd = pPort->pAd;
    UCHAR       tmpSupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR           RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};    
    UCHAR       tmpExtRateLen = 0;
    ULONG       FrameLen =0;
    ULONG           TmpLen = 0; 
    UCHAR       SupRateLen = 0;
    UCHAR           VhtCapIeLen;
    UCHAR       i =0, VhtOpIeLen =0, HtLen= 0, AddHtLen =0;
    HEADER_802_11 Hdr80211;
    ADD_HT_INFO_IE      AddHTInfo;  // Useful as AP.
     // In the A band should not appear 1,2,5,11

     if(bAssocReq)
     {
        MgtMacHeaderInit(pAd,pPort, &Hdr80211, SUBTYPE_ASSOC_RSP, 0, Addr2, pPort->PortCfg.Bssid);
     }
     else if (bReAssocReq)
     {
        MgtMacHeaderInit(pAd,pPort, &Hdr80211, SUBTYPE_REASSOC_RSP, 0, Addr2, pPort->PortCfg.Bssid);
     }
    
     
    if(pPort->Channel > 14)
    {
        tmpSupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        tmpSupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        tmpSupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        tmpSupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        tmpSupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
        tmpExtRateLen = 0;
    }
    else
    {
        PlatformMoveMemory(tmpSupRate,pPort->CommonCfg.SupRate,pPort->CommonCfg.SupRateLen);
        tmpExtRateLen = pPort->CommonCfg.ExtRateLen;
    }
    
    // CH14 only supports 11B
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        SupRateLen = 4;
        tmpExtRateLen = 0;
    }
    else
    {
        SupRateLen = pPort->CommonCfg.SupRateLen;
    }
    
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                      sizeof(HEADER_802_11),    &Hdr80211,
                      2,                        &CapabilityInfo,
                      2,                        &StatusCode,
                      2,                        &Aid,
                      1,                        &SupRateIe,
                      1,                        &SupRateLen,
                      SupRateLen,   tmpSupRate,
                      END_OF_ARGS);

    if (tmpExtRateLen)
    {
        //
        // 802.11g spec doesn't say we can attached ERP IE.
        // If we attached this IE. WiFi phone by TI solution will fail on association.
        //
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,    
                          1,                        &ExtRateIe,
                          1,                        &tmpExtRateLen,
                          tmpExtRateLen,            pPort->CommonCfg.ExtRate,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    // add WMM IE here
    if (pPort->CommonCfg.bWmmCapable && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
    {
        UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 
        WmeParmIe[8] = pPort->SoftAP.ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;

#if UAPSD_AP_SUPPORT
        if(IS_AP_SUPPORT_UAPSD(pAd, pPort))
        {
            WmeParmIe[8] |=0x80; //the b7 in QoS Info is U-ASPD support indication
            DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==>WmeParmIe[8](OoS Info field)=%02x\n", __FUNCTION__, __LINE__, WmeParmIe[8]));
        }
#endif
    
        for (i=QID_AC_BE; i<=QID_AC_VO; i++)
        {
            WmeParmIe[10+ (i*4)] = (i << 5)                                         +     // b5-6 is ACI
                                   ((UCHAR)pPort->SoftAP.ApCfg.BssEdcaParm.bACM[i] << 4)     +     // b4 is ACM
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);              // b0-3 is AIFSN
            WmeParmIe[11+ (i*4)] = (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[i] << 4)           +     // b5-8 is CWMAX
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              // b0-3 is CWMIN
            WmeParmIe[12+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] & 0xff);        // low byte of TXOP
            WmeParmIe[13+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] >> 8);          // high byte of TXOP
        }

        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
                          26,                       WmeParmIe,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only supports 11B and will not carry any HT info, WMM
        ;
    }
    else
    {
        PlatformMoveMemory(&AddHTInfo, &pPort->SoftAP.ApCfg.AddHTInfoIe, sizeof(ADD_HT_INFO_IE));
        AddHTInfo.ControlChan = pPort->Channel;

        // HT capability in AssocRsp frame.
        if ((pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)&&(HTCapability_Len > 0))
        {
            ULONG HtLen1;
            UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
            UCHAR BROADCOM2[4] = {0x0, 0x90, 0x4c, 0x34};
            HT_CAPABILITY_IE HtCapability;
            PlatformMoveMemory(&HtCapability, &pPort->SoftAP.ApCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            if (pAd->HwCfg.Antenna.field.RxPath > 1)
            {
                HtCapability.MCSSet[1] = 0xff;
            }
            else
            {
                HtCapability.MCSSet[1] = 0x00;
            }
            // add HT Capability IE 
            HtLen1 = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe);
            MakeOutgoingFrame(pOutBuffer+FrameLen,         &TmpLen,
                                      1,                                &HtCapIe,
                                      1,                                &HTCapability_Len,
                                     HTCapability_Len,          &HtCapability, 
                                      1,                                &AddHtInfoIe,
                                      1,                                &HtLen1,
                                     HtLen1,          &AddHTInfo, 
                              END_OF_ARGS);
            FrameLen += TmpLen;

            if(pPort->Channel >14)
            {               
                if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                {
                    // add VHT_CAP_IE for VHT
                    VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                    MakeOutgoingFrame(pOutBuffer+FrameLen,  &TmpLen,
                                        1,                  &VhtCapIe,
                                        1,                  &VhtCapIeLen,
                                        SIZE_OF_VHT_CAP_IE, &pPort->SoftAP.ApCfg.VhtCapability,
                                        END_OF_ARGS);
                    FrameLen += TmpLen;

                    // add VHT_OP_IE for VHT            
                    VhtOpIeLen = SIZE_OF_VHT_OP_IE;
                    MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
                                        1,                  &VhtOpIe,
                                        1,                  &VhtOpIeLen,
                                        SIZE_OF_VHT_OP_IE,  &pPort->SoftAP.ApCfg.VhtOperation,
                                        END_OF_ARGS);
                    FrameLen += TmpLen;
                }
            }

            if ((RalinkIe) == 0)
            {
                HtLen = SIZE_HT_CAP_IE + 4;
                AddHtLen = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe) + 4;
                MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                              1,                                &WpaIe,
                              1,                                &HtLen,
                              4,                                &BROADCOM[0],
                             (HtLen-4),          &HtCapability, 
                              1,                                &WpaIe,
                              1,                                &AddHtLen,
                              4,                                &BROADCOM2[0],
                             (AddHtLen-4),          &AddHTInfo, 
                              END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }

#if 0
        //
        // Append association response IEs here
        //
        if ((pPort->PortType == WFD_GO_PORT) && 
            (pPort->PortCfg.AdditionalAssocRspIEData != NULL) &&
            (pPort->PortCfg.AdditionalAssocRspIESize > 0))
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
                pPort->PortCfg.AdditionalAssocRspIESize, pPort->PortCfg.AdditionalAssocRspIEData,
                END_OF_ARGS);

            FrameLen += TmpLen;

            pPort->PortCfg.AdditionalAssocRspIESize = 0;
        }
#endif

        if (P2PSubelementLen > 0)
        {
            PUCHAR          pData;
            pData = pOutBuffer + FrameLen;
            P2pMakeP2pIE(pAd, pPort, PORTSUBTYPE_P2PGO, SUBTYPE_ASSOC_RSP, pData, &TmpLen);
            FrameLen += TmpLen;
            if(bAssocReq)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("ASSOC RSP - Insert P2P IE \n"));
            }
            else if(bReAssocReq)    
            {
                DBGPRINT(RT_DEBUG_TRACE, ("ReASSOC RSP - Insert P2P IE \n"));
            }            
        }
    }

    // add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back, Byte0.b2 for RDG

#if 0  // Because it will have problem with BG mode, so we don't turn on Aggregation only in SoftAp mode.       
    if ( pPort->CommonCfg.bAggregationCapable && (RalinkIe&0x01) )
    {
        RalinkSpecificIe[5] |= 0x01;
    }
#endif

    if( pPort->CommonCfg.bPiggyBackCapable && (RalinkIe&0x02) )
    {
        RalinkSpecificIe[5] |= 0x02;
    }
    if( pPort->CommonCfg.bRdg && (RalinkIe&0x04) )
    {
        RalinkSpecificIe[5] |= 0x04;
    }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
    if ((pPort->PortType == WFD_GO_PORT) && (bAssocReq==TRUE))
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                        pPort->PortCfg.AdditionalAssocRspIESize, pPort->PortCfg.AdditionalAssocRspIEData,
                        9,                       RalinkSpecificIe,
                        END_OF_ARGS);
    }
    else
#endif
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                        9,                       RalinkSpecificIe,
                        END_OF_ARGS);
    }
    FrameLen += TmpLen;

#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))
    if ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortType == WFD_GO_PORT))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOCC -Virtual Port. Indicate Assoc Complete )\n"));
        //win7
        //
        // A miniport driver must make an NDIS_STATUS_DOT11_INCOMING_ASSOC_COMPLETION status indication after the following events:
        //
        // The NIC successfully sends an association response frame to a peer station on an infrastructure BSS that originally sent an association request. 
        // The association is successful and the NIC successfully sends the corresponding association response frame to the peer station that originally requested the association. 
        // The association fails, regardless of whether the response is sent successfully or not. The failure can be because the NIC or operating system reject the association request or because of a failure not related to the 802.11 framework. 
        //
        if (indicateAssocComplete)
        {
            //
            // Can be a success or failure.
            // If it is failure, it must be an error from OS
            //
            if (errorStatus != 0)
            {
                APPrepareAssocCompletePara(
                            pAd,
                            NULL,
                            Addr2,
                            errorStatus,
                            errorSource,
                            bReAssocReq,
                            NULL,
                            0,
                            NULL,
                            0,
                            assocCompletePara,
                            &assocCompleteParaSize);
            }
            else
            {
                APPrepareAssocCompletePara(
                            pAd,
                            pEntry,
                            Addr2,
                            errorStatus,
                            errorSource,
                            bReAssocReq,
                            Add2Ptr(Elem->Msg, LENGTH_802_11),
                            Elem->MsgLen - LENGTH_802_11,
                            Add2Ptr(pOutBuffer, LENGTH_802_11),
                            FrameLen - LENGTH_802_11,
                            assocCompletePara,
                            &assocCompleteParaSize);
            }

            ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_INCOMING_ASSOC_COMPLETION, NULL, assocCompletePara, assocCompleteParaSize);
        }
    }
#endif    

    *pLen = FrameLen;
}


/*
    ==========================================================================
    Description:
        Pre-build a BEACON frame in the shared memory
    ==========================================================================
*/
VOID MgntPktConstructAPBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT  pPort
    ) 
{
    UCHAR           DsLen = 1, SsidLen;
    UCHAR           RSNIe=IE_WPA, RSNIe2=IE_WPA2;
    HEADER_802_11   BcnHdr;
    LARGE_INTEGER   FakeTimestamp;
    ULONG           FrameLen = 0, longptr;
    PTXWI_STRUC     pTxWI = &pAd->pTxCfg->BeaconTxWI;
    PUCHAR          pBeaconFrame = pAd->pTxCfg->BeaconBuf;
    UCHAR           *ptr;
    UINT            i;
    //HTTRANSMIT_SETTING    BeaconTransmit;   // MGMT frame PHY rate setting when operatin at Ht rate.
    PHY_CFG BeaconPhyCfg = {0};
    UCHAR       tmpSupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       tmpExtRateLen = 0;
    UCHAR       SupRateLen = 0;
    UCHAR       Wcid =0;

    // P2P device has its own rule to update beacon
    if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO)
        ||(pPort->PortSubtype == PORTSUBTYPE_P2PClient) )
        return;
    if (pPort->SoftAP.ApCfg.bHideSsid)
        SsidLen = 0;
    else
        SsidLen = pPort->PortCfg.SsidLen;

    MgtMacHeaderInit(pAd,pPort, &BcnHdr, SUBTYPE_BEACON, 0, BROADCAST_ADDR, pPort->PortCfg.Bssid);

    if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK))
        RSNIe = IE_WPA;
    else if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
        RSNIe = IE_WPA2;
    
    // In the A band should not appear 1,2,5,11
    if(pPort->Channel > 14)
    {
        tmpSupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        tmpSupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        tmpSupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        tmpSupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        tmpSupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
        tmpExtRateLen = 0;
    }
    else
    {
        PlatformMoveMemory(tmpSupRate,pPort->CommonCfg.SupRate,pPort->CommonCfg.SupRateLen);
        tmpExtRateLen = pPort->CommonCfg.ExtRateLen;
    }
    
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)) 
    {
        // CH14 only supports 11b
        SupRateLen = 4;
        tmpExtRateLen = 0;
    }
    else
    {
        SupRateLen = pPort->CommonCfg.SupRateLen;
    }

    // for update framelen to TxWI later.
    MakeOutgoingFrame(pBeaconFrame,                  &FrameLen,
                    sizeof(HEADER_802_11),           &BcnHdr, 
                    TIMESTAMP_LEN,                   &FakeTimestamp,
                    2,                               &pPort->CommonCfg.BeaconPeriod,
                    2,                               &pPort->SoftAP.ApCfg.CapabilityInfo,
                    1,                               &SsidIe, 
                    1,                               &SsidLen, 
                    SsidLen,                         pPort->PortCfg.Ssid,
                    1,                               &SupRateIe, 
                    1,                               &SupRateLen,
                    SupRateLen,                      tmpSupRate, 
                    1,                               &DsIe, 
                    1,                               &DsLen, 
                    1,                               &pPort->Channel,
                    END_OF_ARGS);

    if (tmpExtRateLen)
    {
        ULONG TmpLen;
        MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                        1,                               &ExtRateIe, 
                        1,                               &tmpExtRateLen,
                        tmpExtRateLen,                   pPort->CommonCfg.ExtRate, 
                        END_OF_ARGS);
        FrameLen += TmpLen;
    }

    //
    // RSN Order: 21
    //
    // Append RSN_IE when  WPA OR WPAPSK, 
    if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
        ULONG TmpLen;
        MakeOutgoingFrame(pBeaconFrame+FrameLen,        &TmpLen,
                          1,                            &RSNIe,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],      pPort->SoftAP.ApCfg.RSN_IE[0],
                          1,                            &RSNIe2,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[1],
                          pPort->SoftAP.ApCfg.RSNIE_Len[1],      pPort->SoftAP.ApCfg.RSN_IE[1],
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }   
    else if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
    {
        ULONG TmpLen;   
        MakeOutgoingFrame(pBeaconFrame+FrameLen,        &TmpLen,
                          1,                            &RSNIe,
                          1,                            &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],      pPort->SoftAP.ApCfg.RSN_IE[0],
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    //
    // EDCA Parameter Set Order: 23
    //  
    // add WMM IE here
    if ((pPort->CommonCfg.bWmmCapable) || (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED))
    {
        ULONG TmpLen;
        UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 
        WmeParmIe[8] = pPort->SoftAP.ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;
        for (i=QID_AC_BE; i<=QID_AC_VO; i++)
        {
            WmeParmIe[10+ (i*4)] = (i << 5)                                         +     // b5-6 is ACI
                                   ((UCHAR)pPort->SoftAP.ApCfg.BssEdcaParm.bACM[i] << 4)     +     // b4 is ACM
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);              // b0-3 is AIFSN
            WmeParmIe[11+ (i*4)] = (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[i] << 4)           +     // b5-8 is CWMAX
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              // b0-3 is CWMIN
            WmeParmIe[12+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] & 0xff);        // low byte of TXOP
            WmeParmIe[13+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] >> 8);          // high byte of TXOP
        }

        MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                          26,                            WmeParmIe,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &BeaconPhyCfg, 0);

    Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);
    XmitWriteTxWI(pAd, pPort, pTxWI, FALSE,  TRUE, FALSE, FALSE, TRUE, 0, Wcid, 
        FrameLen, PID_MGMT, 0, IFS_HTTXOP, FALSE, BeaconPhyCfg, FALSE, TRUE, FALSE);
    //
    // step 6. move BEACON TXD and frame content to on-chip memory
    //
    
    
    ptr = (PUCHAR)&pAd->pTxCfg->BeaconTxWI;
    for (i=0; i<pAd->HwCfg.TXWI_Length; )  // 24-byte TXINFO field
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + i, longptr);   //Beacon buffer of AP is always in 0x7900.
        ptr += 4;
        i+=4;
    }   

    // update BEACON frame content. start right after the 24-byte TXINFO field
    ptr = pAd->pTxCfg->BeaconBuf;
    for (i=0; i< FrameLen;)
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + pAd->HwCfg.TXWI_Length + i, longptr);
        ptr += 4;
        i+=4;
    }
    
    pAd->pTxCfg->BeaconBufLen = FrameLen;
    pPort->SoftAP.ApCfg.TimIELocationInBeacon = FrameLen; 
    pPort->SoftAP.ApCfg.CapabilityInfoLocationInBeacon = sizeof(HEADER_802_11) + TIMESTAMP_LEN + 2;
}

VOID
MgntPktConstructAPProbRspFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT  pPort,
    
    IN MLME_QUEUE_ELEM *Elem,
    IN PUCHAR Addr2, 
    IN CHAR                 *Ssid,
    IN UCHAR                 SsidLen,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pLen    
    ) 
{
    ADD_HT_INFO_IE      AddHTInfo;  // Useful as AP. 
    UCHAR       tmpExtRateLen = 0;
    UCHAR       SupRateLen = 0;
    ULONG       P2PSubelementLen, WpsIELen, TmpLen;
    UCHAR       tmpSupRate[MAX_LEN_OF_SUPPORTED_RATES];
    LARGE_INTEGER FakeTimestamp;
    UCHAR         DsLen = 1;
    UCHAR   ErpIeLen = 1;
    UCHAR   RSN_IE[22]={ 0x00,0x50,0xf2,0x01,0x01,0x00,0x00,0x50,0xf2,0x02,0x01,0x00,0x00,0x50,0xf2,0x02,0x01,0x00,0x00,0x50,0xf2,0x02};
    UCHAR   RSNIe=IE_WPA, RSNIe2=IE_WPA2;
    UCHAR  RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00}; 
    UCHAR       DevAddr[MAC_ADDR_LEN], DevType[P2P_DEVICE_TYPE_LEN];
    UCHAR       NumRequestedType = 0,  RequestedDeviceType[MAX_P2P_FILTER_LIST][P2P_DEVICE_TYPE_LEN];
    UCHAR       MacIdx = 0xff;
    PFRAME_802_11   pFrame;
    PHEADER_802_11   pHeader;
    ULONG FrameLen;
    UCHAR       *P2pSubelement = NULL, *WpsIE = NULL;  
    HEADER_802_11 Hdr;

    MgtMacHeaderInit(pAd,pPort, &Hdr, SUBTYPE_PROBE_RSP, 0, Addr2, pPort->PortCfg.Bssid);
    
     if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK))
        RSNIe = IE_WPA;
    else if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
        RSNIe = IE_WPA2;

    // In the A band should not appear 1,2,5,11
    if((pPort->Channel > 14) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        tmpSupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        tmpSupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        tmpSupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
        tmpSupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        tmpSupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        tmpSupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
        tmpExtRateLen = 0;
        SupRateLen = pPort->CommonCfg.SupRateLen;
    }
    else
    {
        PlatformMoveMemory(tmpSupRate,pPort->CommonCfg.SupRate,pPort->CommonCfg.SupRateLen);
        tmpExtRateLen = pPort->CommonCfg.ExtRateLen;
        SupRateLen = pPort->CommonCfg.SupRateLen;
    }
    
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only supports 11b
        SupRateLen = 4;
        tmpExtRateLen = 0;
    }
    else
    {
        SupRateLen = pPort->CommonCfg.SupRateLen;
    }
    pPort->SoftAP.ApCfg.CapabilityInfo |= 1;
    MakeOutgoingFrame(pOutBuffer,                 &FrameLen, 
                      sizeof(HEADER_802_11),      &Hdr, 
                      TIMESTAMP_LEN,              &FakeTimestamp,
                      2,                          &pPort->CommonCfg.BeaconPeriod,
                      2,                          &pPort->SoftAP.ApCfg.CapabilityInfo,
                      1,                          &SsidIe, 
                      1,                          &pPort->PortCfg.SsidLen, 
                      pPort->PortCfg.SsidLen,     pPort->PortCfg.Ssid,
                      1,                          &SupRateIe, 
                      1,                          &SupRateLen,
                      SupRateLen,                 tmpSupRate, 
                      1,                          &DsIe, 
                      1,                          &DsLen, 
                      1,                          &pPort->Channel,
                      END_OF_ARGS);

    if (tmpExtRateLen)
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen, 
                          1,                        &ErpIe,
                          1,                        &ErpIeLen,
                          1,                        &pPort->SoftAP.ApCfg.ErpIeContent,
                          1,                        &ExtRateIe,
                          1,                        &tmpExtRateLen,
                          tmpExtRateLen,            pPort->CommonCfg.ExtRate,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    // Append RSN_IE when  WPA OR WPAPSK, 
    if ((pPort->PortCfg.AuthMode < Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeAutoSwitch))
        ; // enough information
    else if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1WPA2) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen, 
                          1,                        &RSNIe,
                          1,                        &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],  pPort->SoftAP.ApCfg.RSN_IE[0],
                          1,                        &RSNIe2,
                          1,                        &pPort->SoftAP.ApCfg.RSNIE_Len[1],
                          pPort->SoftAP.ApCfg.RSNIE_Len[1],  pPort->SoftAP.ApCfg.RSN_IE[1],
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
    else
    {
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen, 
                          1,                        &RSNIe,
                          1,                        &pPort->SoftAP.ApCfg.RSNIE_Len[0],
                          pPort->SoftAP.ApCfg.RSNIE_Len[0],  pPort->SoftAP.ApCfg.RSN_IE[0],
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    // add WMM IE here
    if (pPort->CommonCfg.bWmmCapable)
    {
        UCHAR i;
        UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 
        WmeParmIe[8] = pPort->SoftAP.ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;
#if UAPSD_AP_SUPPORT
        if(IS_AP_SUPPORT_UAPSD(pAd, pPort))
        {
            WmeParmIe[8]|=0x80; //the b7 in QoS Info is U-ASPD support indication
            DBGPRINT(RT_DEBUG_INFO,("Chk::%s(%d)==>WmeParmIe[8](OoS Info field)=%02x\n", __FUNCTION__, __LINE__, WmeParmIe[8]));
        }
#endif
        for (i=QID_AC_BE; i<=QID_AC_VO; i++)
        {
            WmeParmIe[10+ (i*4)] = (i << 5)                                         +     // b5-6 is ACI
                                   ((UCHAR)pPort->SoftAP.ApCfg.BssEdcaParm.bACM[i] << 4)     +     // b4 is ACM
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);              // b0-3 is AIFSN
            WmeParmIe[11+ (i*4)] = (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[i] << 4)           +     // b5-8 is CWMAX
                                   (pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              // b0-3 is CWMIN
            WmeParmIe[12+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] & 0xff);        // low byte of TXOP
            WmeParmIe[13+ (i*4)] = (UCHAR)(pPort->SoftAP.ApCfg.BssEdcaParm.Txop[i] >> 8);          // high byte of TXOP
        }

        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
                          26,                       WmeParmIe,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    // add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back, Byte0.b2 for RDG

#if 0  // Because it will have problem with BG mode, so we don't turn on Aggregation only in SoftAp mode.       
    if (pPort->CommonCfg.bAggregationCapable)
    {
        RalinkSpecificIe[5] |= 0x01;
    }
#endif

    if( pPort->CommonCfg.bPiggyBackCapable )
    {
        RalinkSpecificIe[5] |= 0x02;
    }
    if( pPort->CommonCfg.bRdg )
    {
        RalinkSpecificIe[5] |= 0x04;
    }

    MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                      9,                         RalinkSpecificIe,
                      END_OF_ARGS);
    FrameLen += TmpLen;     

    // add Channel switch announcement IE
    if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11A) && (pPort->CommonCfg.bIEEE80211H == 1) && (pPort->CommonCfg.RadarDetect.RDMode == RD_SWITCHING_MODE))
    {
        UCHAR CSAIe=IE_CHANNEL_SWITCH_ANNOUNCEMENT;
        UCHAR CSALen=3;
        UCHAR CSAMode=1;

        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
                          1,                        &CSAIe,
                          1,                        &CSALen,
                          1,                        &CSAMode,
                          1,                        &pPort->Channel,
                          1,                        &pPort->CommonCfg.RadarDetect.CSCount,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }
    
    PlatformMoveMemory(&AddHTInfo, &pPort->SoftAP.ApCfg.AddHTInfoIe, sizeof(ADD_HT_INFO_IE));
    AddHTInfo.ControlChan = pPort->Channel;     

    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only supports 11B and will not carry any HT info
        ;
    }
    else
    {
        if (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)
        {           
            UCHAR   HtLen, AddHtLen, NewExtLen;
            UCHAR   VhtCapIeLen, VhtOpIeLen;
            UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
            UCHAR BROADCOM2[4] = {0x0, 0x90, 0x4c, 0x34};
            HT_CAPABILITY_IE HtCapability;
            PlatformMoveMemory(&HtCapability, &pPort->SoftAP.ApCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            if (pAd->HwCfg.Antenna.field.RxPath > 1)
            {
                HtCapability.MCSSet[1] = 0xff;
            }
            else
            {
                HtCapability.MCSSet[1] = 0x00;
            }

            HtLen = sizeof(pPort->SoftAP.ApCfg.HtCapability);
            AddHtLen = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe);          

            NewExtLen = 1;
            //New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame
            MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                              1,                                &HtCapIe,
                              1,                                &HtLen,
                             sizeof(HT_CAPABILITY_IE),          &HtCapability, 
                              1,                                &AddHtInfoIe,
                              1,                                &AddHtLen,
                             sizeof(ADD_HT_INFO_IE),          &AddHTInfo, 
                              1,                                &NewExtChanIe,
                              1,                                &NewExtLen,
                             sizeof(NEW_EXT_CHAN_IE),          &pPort->CommonCfg.NewExtChanOffset, 
                              END_OF_ARGS);
            FrameLen += TmpLen;

            // add IEs for VHT
            if (pPort->Channel>14)
            {
                // add VHT_CAP_IE for VHT
                if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                {
                    VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                    MakeOutgoingFrame(pOutBuffer+FrameLen,  &TmpLen,
                                        1,                  &VhtCapIe,
                                        1,                  &VhtCapIeLen,
                                        SIZE_OF_VHT_CAP_IE, &pPort->SoftAP.ApCfg.VhtCapability,
                                        END_OF_ARGS);
                    FrameLen += TmpLen;
                

                    // add VHT_OP_IE for VHT
                    VhtOpIeLen = SIZE_OF_VHT_OP_IE;
                    MakeOutgoingFrame(pOutBuffer+FrameLen,  &TmpLen,
                                        1,                  &VhtOpIe,
                                        1,                  &VhtOpIeLen,
                                        SIZE_OF_VHT_OP_IE,  &pPort->SoftAP.ApCfg.VhtOperation,
                                        END_OF_ARGS);
                    FrameLen += TmpLen;
                }
            }

            // don't carry this very old IE for WiFi Direct.
            if ((pPort->PortSubtype != PORTSUBTYPE_P2PGO))
            {
                HtLen = SIZE_HT_CAP_IE + 4;
                AddHtLen = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe) + 4;
                MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                              1,                                &WpaIe,
                              1,                                &HtLen,
                              4,                                &BROADCOM[0],
                             (HtLen-4),          &HtCapability, 
                              1,                                &WpaIe,
                              1,                                &AddHtLen,
                              4,                                &BROADCOM2[0],
                             (AddHtLen-4),          &AddHTInfo, 
                              END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
    }

    // AP: Append additional response ie
    if((pPort->PortType == EXTAP_PORT) && (pPort->PortCfg.AdditionalResponseIESize > 0))
    {
        MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                        pPort->PortCfg.AdditionalResponseIESize, pPort->PortCfg.AdditionalResponseIEData,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }

    // I am GO
    if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        P2pSubelement = NULL;
        WpsIE = NULL;
        PlatformAllocateMemory(pAd, &P2pSubelement, MAX_VIE_LEN);
         PlatformAllocateMemory(pAd, &WpsIE, MAX_VIE_LEN);

        if (PeerP2pProbeReqSanity(pAd, 
                                    pPort,
                                    Elem->Msg, 
                                    Elem->MsgLen, 
                                    Addr2, 
                                    Ssid, 
                                    &SsidLen, 
                                    &P2PSubelementLen,
                                    P2pSubelement,
                                    &WpsIELen,
                                    WpsIE))
        {

            PlatformZeroMemory(DevAddr, MAC_ADDR_LEN);
            PlatformZeroMemory(DevType, P2P_DEVICE_TYPE_LEN);
            P2pParseSubElmt(pAd, (PVOID)P2pSubelement, P2PSubelementLen, 
                            FALSE, NULL, NULL, NULL, 0, NULL, DevAddr, NULL, NULL, NULL, NULL, NULL, NULL, DevType, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

            // Check if carried requested device types in probe request
            PlatformZeroMemory(RequestedDeviceType, sizeof(RequestedDeviceType));
            if (WpsIELen > 0)
                P2PParseWPSIE(WpsIE, (USHORT)WpsIELen, NULL, NULL, NULL, NULL, NULL, &NumRequestedType, (PUCHAR)&RequestedDeviceType[0][0]);

            // The P2P Group Owner may filter the P2P Group Info returned 
            // in the Probe Response frame to include only devices with matching device address or primary or secondary device type values              
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
            //if (pAd->pNicCfg->NdisVersion >= RTMP_NDIS630_VERSION)
            {
                // TODO: [ndtest_8018] Device address and device types might be appended from P2PAdditionalRequestIEData regardless of  filter scan
                if (0)
                /*if (FALSE == P2pMsDeviceAddrAndTypeMatch(
                                pAd, 
                                pAd->PortList[pPort->P2PCfg.GOPortNumber],
                                DevAddr,
                                NumRequestedType,
                                (PUCHAR)&RequestedDeviceType[0][0],
                                TRUE, 
                                NULL, 
                                NULL,
                                0, 
                                NULL,
                                P2P_IS_GO,
                                &MacIdx))*/
                {
                    //goto CleanUp;
                }
            }
#endif              
        }

        // Append additional response ie (wsc ie) 
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
        if (pPort->PortCfg.AdditionalResponseIESize > 0)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] %s - Use Microsoft IE (AdditionalResponseIESize = %d)\n", __FUNCTION__, pPort->PortCfg.AdditionalResponseIESize));

            MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
                              pPort->PortCfg.AdditionalResponseIESize,  pPort->PortCfg.AdditionalResponseIEData,
                              END_OF_ARGS);
            FrameLen += TmpLen;
        }
        /*else if ( (IS_P2P_MS_DEV(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]))
            && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalResponseIESize > 0) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalResponseIEData != NULL))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] %s - Use Microsoft IE (device-port, AdditionalResponseIESize = %d)\n", __FUNCTION__, pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalResponseIESize));

            MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
                                pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalResponseIESize,    pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AdditionalResponseIEData,
                                END_OF_ARGS);
        }*/
        else
#endif
        {
            DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] %s - Use Ralink IE\n", __FUNCTION__));

            // Ralink Proprietary feature for IP 
            P2pMakeProbeRspWSCIE(pAd, pPort, SUBTYPE_PROBE_RSP,  pOutBuffer + FrameLen, &TmpLen);
            FrameLen += TmpLen;
        }

        // ApSyncMlmeSyncPeerProbeReqAction() is called when I am already GO. So doesn't use Is_P2P_on to check whether need to add P2P IE in response.
        if (P2PSubelementLen > 0)
        {
            ULONG   P2PIeLen;
            PUCHAR  ptr;

            ptr = pOutBuffer + FrameLen;
            P2pMakeP2pIE(pAd, pPort, PORTSUBTYPE_P2PGO, SUBTYPE_PROBE_RSP, ptr, &P2PIeLen);
            FrameLen += P2PIeLen;
        }
    }

    pFrame = (PFRAME_802_11) pOutBuffer;
    pHeader = &pFrame->Hdr;

    DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s(%d) :PortType(%d) Send ProbeRsp PKT Addr1(%x %x %x %x %x %x), Addr2(%x %x %x %x %x %x), Addr3(%x %x %x %x %x %x), Last Latch ch(%d),  Listen ch(%d)\n", 
                                __FUNCTION__, 
                                __LINE__,
                                pPort->PortType,
                                pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5],
                                pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5],
                                pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5],
                                pAd->HwCfg.LatchRfRegs.Channel, 
                                pPort->P2PCfg.ListenChannel));

   *pLen = FrameLen;

    if (P2pSubelement)
        PlatformFreeMemory(P2pSubelement, MAX_VIE_LEN);
    if (WpsIE)
            PlatformFreeMemory(WpsIE, MAX_VIE_LEN);   
}

VOID
MgntPktConstructAsscReqFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT  pPort,
    IN BOOLEAN      bReassoc,
    IN USHORT  CapabilityInfo, 
    IN PUCHAR   ApAddr,
    IN USHORT          ListenIntv,
    IN USHORT           VarIesOffset,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pLen        
    )
{
    ULONG   FrameLen =0;
    ULONG           tmp, idx = 0;
    UCHAR           WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, STA_QOS_CAPABILITY};
    UCHAR           CipherTmp[64];
    UCHAR           CipherTmpLen = 0; 
    //WAPI
    UCHAR           IEWAPI =IE_WAPI;
    UCHAR           BKIDCount[]={0x00,0x00};
    UCHAR           WAPIIeLen = 0x16;   
    UCHAR           WmmTSpecIe[] = {IE_VENDOR_SPECIFIC, 0x3d, 0x00, 0x50, 0xf2, 0x02, 0x02, 0x01};
    
    UCHAR           VhtCapIeLen = 0, OperatingModeNotificationLen = 0;
    OPERATING_MODE_FIELD OperatingModeNotificationField = {0};
    BOOLEAN         FoundPMK = FALSE;   
    UCHAR           RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00}; 
    // WSC fixed IE for Version and Request type
    UCHAR           WscBuf[32] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x04, 0x10, 0x4a, 0x00, 0x01, 0x10, 0x10, 0x3a, 0x00, 0x01, 0x01};
    UCHAR   CcxIEVer[] = { 0x00, 0x40, 0x96, 0x03, 0x05};
    HEADER_802_11 AssocHdr;
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    
   if (bReassoc)
    {
         MgtMacHeaderInit(pAd, pPort,&AssocHdr, SUBTYPE_REASSOC_REQ, 0, ApAddr, ApAddr);
        // Build basic frame first
        MakeOutgoingFrame(pOutBuffer,                 &FrameLen,
                            sizeof(HEADER_802_11),    &AssocHdr,
                            2,                        &CapabilityInfo,
                            2,                        &ListenIntv,
                            MAC_ADDR_LEN,    ApAddr,
                            1,                        &SsidIe,
                            1,                        &pAd->MlmeAux.SsidLen, 
                            pAd->MlmeAux.SsidLen,     pAd->MlmeAux.Ssid, 
                            1,                        &SupRateIe,
                            1,                        &pAd->MlmeAux.SupRateLen,
                            pAd->MlmeAux.SupRateLen,  pAd->MlmeAux.SupRate,
                            END_OF_ARGS);
    }
    else
    {
         MgtMacHeaderInit(pAd, pPort,&AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr, ApAddr);
        // Build basic frame first
        MakeOutgoingFrame(pOutBuffer,                 &FrameLen,
                            sizeof(HEADER_802_11),    &AssocHdr,
                            2,                        &CapabilityInfo,
                            2,                        &ListenIntv,
                            1,                        &SsidIe,
                            1,                        &pAd->MlmeAux.SsidLen, 
                            pAd->MlmeAux.SsidLen,     pAd->MlmeAux.Ssid,
                            1,                        &SupRateIe,
                            1,                        &pAd->MlmeAux.SupRateLen,
                            pAd->MlmeAux.SupRateLen,  pAd->MlmeAux.SupRate,
                            END_OF_ARGS);
    }

  if (pAd->MlmeAux.ExtRateLen != 0)
    {
        MakeOutgoingFrame(pOutBuffer + FrameLen,      &tmp,
                            1,                        &ExtRateIe,
                            1,                        &pAd->MlmeAux.ExtRateLen,
                            pAd->MlmeAux.ExtRateLen,  pAd->MlmeAux.ExtRate,
                            END_OF_ARGS);
        FrameLen += tmp;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - pAd->PortCfg.AuthMode [%s]\n",decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));

    //
    // not including HT and RalinkIE for CCX Diag
    // CH14 only supports 11b and will not also include HT info
    //

    if ((!(pPort->CommonCfg.Ch14BOnly && (pPort->ScaningChannel == CHANNEL_14))))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - pPort->CommonCfg.InitPhyMode [0x%08X],AUX htcap= %d,bWCN =%x\n",pPort->CommonCfg.InitPhyMode,
        pAd->MlmeAux.HtCapabilityLen,pAd->MlmeAux.bWCN));
        // HT
        if (((pAd->MlmeAux.HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))  ||
            ((pAd->MlmeAux.HtCapabilityLen > 0) && (pPort->CommonCfg.InitPhyMode >= PHY_11ABGN_MIXED) && pAd->MlmeAux.bWCN))
        {
            UCHAR HtLen;
            UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
            if (pAd->StaActive.SupportedHtPhy.bPreNHt == TRUE)
            {
                HtLen = SIZE_HT_CAP_IE + 4;
                MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                    1,                              &WpaIe,
                                    1,                              &HtLen,
                                    4,                              &BROADCOM[0],
                                    pAd->MlmeAux.HtCapabilityLen,   &pAd->MlmeAux.HtCapability, 
                                    END_OF_ARGS);
            }
            else
            {
                MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                    1,                              &HtCapIe,
                                    1,                              &pAd->MlmeAux.HtCapabilityLen,
                                    pAd->MlmeAux.HtCapabilityLen,   &pAd->MlmeAux.HtCapability, 
                                    END_OF_ARGS);
            }
            FrameLen += tmp;

        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - %02x %02x %02x %02x\n", pAd->MlmeAux.HtCapability.MCSSet[0], pAd->MlmeAux.HtCapability.MCSSet[1], pAd->MlmeAux.HtCapability.MCSSet[2], pAd->MlmeAux.HtCapability.MCSSet[3]));
            

            if ((pPort->ScaningChannel <= 14) && (pPort->CommonCfg.BACapability.field.b2040CoexistScanSup == 1))
            {
                HtLen = sizeof(EXT_CAP_ELEMT);
                MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                    1,                              &ExtHtCapIe,
                                    1,                              &HtLen,
                                    HtLen,                          &pPort->CommonCfg.ExtCapIE.DoubleWord, 
                                    END_OF_ARGS);

                FrameLen += tmp;
            }
            else if ((pPort->ScaningChannel > 14) && (VHT_CAPABLE(pAd)))
            {
                EXT_CAP_ELEMT ExtendedCapabilitiesElement = {0};

                ExtendedCapabilitiesElement.DoubleWord = pPort->CommonCfg.ExtCapIE.DoubleWord;
                ExtendedCapabilitiesElement.field.BssCoexstSup = 0; // Not support in 5GHz
                
                HtLen = sizeof(EXT_CAP_ELEMT);
                
                MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                    1,                              &ExtHtCapIe,
                                    1,                              &HtLen,
                    HtLen, &ExtendedCapabilitiesElement.DoubleWord, 
                                    END_OF_ARGS);

                FrameLen += tmp;
            }
        }

        //
        // add VHT_Cap IE for VHT
        //
        if ((pPort->CommonCfg.PhyMode == PHY_11VHT)
            && (pPort->ScaningChannel>14))
        {
            ULONG Tmp;
            VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
            MakeOutgoingFrame(pOutBuffer+FrameLen,  &Tmp,
                                1,                  &VhtCapIe,
                                1,                  &VhtCapIeLen,                                   
                                SIZE_OF_VHT_CAP_IE, &pPort->CommonCfg.DesiredVhtPhy.VhtCapability,
                                END_OF_ARGS);
            FrameLen += Tmp;

            //
            // The Operating Mode Notification for Associatoin Request
            //
            if (VHT_CAPABLE(pAd) && 
                (pAd->LogoTestCfg.SigmaCfg.bAddOperatingModeNotificationInAssociationRequest == TRUE))
            {
                OperatingModeNotificationField.ChannelWidth = CH_WIDTH_BW20;
                OperatingModeNotificationField.RxNss = RX_NSS_1;
                OperatingModeNotificationField.RxNssType = RX_NSS_TYPE_MAX_RX_NSS_NORMAL;
            
                OperatingModeNotificationLen = SIZE_OF_VHT_OPERATING_MODE_NOTIFICATION_IE;
                MakeOutgoingFrame((pOutBuffer + FrameLen), &Tmp, 
                    1, &VhtOperatingModeNotification, 
                    1, &OperatingModeNotificationLen, 
                    OperatingModeNotificationLen, &OperatingModeNotificationField, 
                    END_OF_ARGS);
                FrameLen += Tmp;
                if(pBssidMacTabEntry != NULL)
                {
                    pBssidMacTabEntry->VhtPeerStaCtrl.MaxBW = CH_WIDTH_BW20;
                    pBssidMacTabEntry->VhtPeerStaCtrl.MaxNss = RX_NSS_1;
                }
            }
        }

        //
        // add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION/PiggyBack/RDG, only when
        // 1. user enable
        // 2. AP annouces in BEACON
        //      
        if (pPort->CommonCfg.bAggregationCapable && (pAd->MlmeAux.APRalinkIe & 0x00000001))
            RalinkIe[5] |= 0x01;
        if (pPort->CommonCfg.bPiggyBackCapable && (pAd->MlmeAux.APRalinkIe & 0x00000002))
            RalinkIe[5] |= 0x02;
        if (pPort->CommonCfg.bRdg && (pAd->MlmeAux.APRalinkIe & 0x00000004))
            RalinkIe[5] |= 0x04;

        // Ralink IE
        MakeOutgoingFrame(pOutBuffer+FrameLen,           &tmp,
                            9,                           RalinkIe,
                            END_OF_ARGS);
        FrameLen += tmp;
    }

    // APSD
    if ((pPort->CommonCfg.bWmmCapable == TRUE) && (pAd->MlmeAux.APEdcaParm.bValid == TRUE))            
    {
        if (pPort->CommonCfg.bAPSDCapable && pAd->MlmeAux.APEdcaParm.bAPSDCapable)
        {
            QBSS_STA_INFO_PARM QosInfo;

            PlatformZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
            QosInfo.UAPSD_AC_BE = pPort->CommonCfg.bAPSDAC_BE;
            QosInfo.UAPSD_AC_BK = pPort->CommonCfg.bAPSDAC_BK;
            QosInfo.UAPSD_AC_VI = pPort->CommonCfg.bAPSDAC_VI;
            QosInfo.UAPSD_AC_VO = pPort->CommonCfg.bAPSDAC_VO;
            QosInfo.MaxSPLength = pPort->CommonCfg.MaxSPLength;
            WmeIe[8] |= *(PUCHAR)&QosInfo;
        }
        else
        {
            // The Parameter Set Count is set to ¡§0¡¨ in the association request frames
            // WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f);
        }

        MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                            9,                          &WmeIe[0],
                            END_OF_ARGS);
        FrameLen += tmp;
    }
    
    // The power capability IE
    if (pAd->StaCfg.MDSMCtrl.dot11SpectrumManagementRequired == TRUE)
    {
        POWER_CAPABILITY_IE PowerCapabilityIE = {0};

        MDSMCreatePowerCapabilityIE(pAd, &PowerCapabilityIE);

        MakeOutgoingFrame(pOutBuffer + FrameLen,         &tmp,
                    sizeof(POWER_CAPABILITY_IE),         &PowerCapabilityIE, 
                    END_OF_ARGS);
        FrameLen += tmp;
    }
    
    // init the CipherTmp
    PlatformZeroMemory(CipherTmp, sizeof(CipherTmp));

    //
    // Let WPA(#221) Element ID on the end of this association frame.
    // Otherwise some AP will fail on parsing Element ID and set status fail on Assoc Rsp.
    // For example: Put Vendor Specific IE on the front of WPA IE.
    // This happens on AP (Model No:Linksys WRK54G)
    //      
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK)) &&
        (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)) // WPS ignore SSNIE
    {
        // Copy WPA template to buffer
        CipherTmpLen = CipherWpaTemplateLen;
        PlatformMoveMemory(CipherTmp, CipherWpaTemplate, CipherTmpLen);
        
        // Modify Group cipher, WPA+WEP mixed mode is possible
        // should use WepStatus?
        // CCX5 may use (pPort->CommonCfg.WepStatus) for S71, review this
        switch (pPort->PortCfg.GroupCipher)
        {
            case DOT11_CIPHER_ALGO_WEP40:
            case DOT11_CIPHER_ALGO_WEP104:
            case DOT11_CIPHER_ALGO_WEP:
                CipherTmp[11] = ((pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP40) ? 0x1 : 0x5);
                break;
            case Ralink802_11Encryption2Enabled:  // TKIP
                CipherTmp[11] = 0x2;
                break;
            default: // otherwise AES
                CipherTmp[11] = 0x4;
                break;
        }
        // Modify Pairwise cipher
        CipherTmp[17] = ((pPort->PortCfg.PairCipher == Ralink802_11Encryption2Enabled) ? 0x2 : 0x4);
        // Modify AKM
        CipherTmp[23] = ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) ? 0x1 : 0x2);
        // Make outgoing frame
        MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                            CipherTmpLen,           &CipherTmp[0],
                            END_OF_ARGS);
        FrameLen += tmp;
        
        // Append Variable IE
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, CipherTmp, CipherTmpLen);
        VarIesOffset += CipherTmpLen;

        // Set Variable IEs Length
        pAd->StaCfg.ReqVarIELen = VarIesOffset;
        pAd->StaCfg.AssocInfo.RequestIELength = VarIesOffset;
        // OffsetResponseIEs follow ReqVarIE
        pAd->StaCfg.AssocInfo.OffsetResponseIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION) + pAd->StaCfg.ReqVarIELen;
    }
    // For WPA2 / WPA2-PSK
    else if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || 
                (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) &&
             ((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)    ||
             ((pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)&&(!IS_DISABLE_WSC20TB_RSNIE(pAd))))) // WPS ignore RSNIE
    {
        // Copy WPA2 template to buffer
        CipherTmpLen = CipherWpa2TemplateLen;
        PlatformMoveMemory(CipherTmp, CipherWpa2Template, CipherTmpLen);

        // ccx may use (pPort->CommonCfg.WepStatus) for S71, review this
        switch (pPort->PortCfg.GroupCipher)
        {
            case DOT11_CIPHER_ALGO_WEP40:
            case DOT11_CIPHER_ALGO_WEP104:
            case DOT11_CIPHER_ALGO_WEP:
                CipherTmp[7] = ((pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP40) ? 0x1 : 0x5);
                break;
            case Ralink802_11Encryption2Enabled:  // TKIP
                CipherTmp[7] = 0x2;
                break;
#if 0
            case DOT11_CIPHER_ALGO_BIP:
                DBGPRINT(RT_DEBUG_TRACE,("MFP: set cipher algo BIP \n"));
                CipherTmp[7] = 0x6;
                break;
#endif
            default: // otherwise AES
                CipherTmp[7] = 0x4;
                break;
        }
        // Modify Pairwise cipher
        CipherTmp[13] = ((pPort->PortCfg.PairCipher == Ralink802_11Encryption2Enabled) ? 0x2 : 0x4);
        // Modify AKM
        CipherTmp[19] = ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ? 0x1 : 0x2);
        // Check for WPA PMK cache list
        if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2)
        {
            // Search chched PMKID, append it if existed
            for (idx = 0; idx < pAd->StaCfg.PMKIDCount; idx++)
            {
                if (PlatformEqualMemory(ApAddr, &pAd->StaCfg.PMKIDList[idx].BSSID, 6))
                {
                    FoundPMK = TRUE; // Found a cached PMKID for the desired AP.
                    break;
                }
            }
        }
        
        // Found a cached PMKID for the desired AP.
        if (FoundPMK)
        {
            // Update length within RSN IE
            CipherTmp[1] += 18; // PMKID Counter (2 octets) + PMKID List (16 octects)

            // PMKID Count.
            *(PUSHORT) &CipherTmp[CipherTmpLen] = 1;

            // PMKID value.
            PlatformMoveMemory(&CipherTmp[CipherTmpLen + 2], &pAd->StaCfg.PMKIDList[idx].PMKID, 16);
                
            CipherTmpLen += 18;
        }
        else
        {
                  //add PMKID list as 0 for fit IE formate
                CipherTmp[1] += 2;
                 *(UNALIGNED PUSHORT) &CipherTmp[CipherTmpLen] = 0;
                 CipherTmpLen += 2;
        }

        if((MFP_ON(pAd, pPort) && pPort->PortCfg.bAKMwSHA256) || pAd->StaCfg.PmfCfg.PmfControl.AKMwSHA256)
        {
                    
            CipherTmp[19] = ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ? 0x5 : 0x6);
            DBGPRINT(RT_DEBUG_ERROR, ("MFP: AP SHA256 = 0x%x, STA force SHA256 = 0x%x\n",pPort->PortCfg.bAKMwSHA256, pAd->StaCfg.PmfCfg.PmfControl.AKMwSHA256));
        }

        if(MFP_ON(pAd, pPort))
        {
                    //add BIP cipher suite OUI
                    CipherTmp[1] += 4;
                    PlatformMoveMemory(&CipherTmp[CipherTmpLen], OUI_PMF_BIP_CIPHER, 4);
                    CipherTmpLen += 4;
        }
        
                DBGPRINT(RT_DEBUG_ERROR, ("MFP: Outgoing RSNCapability: 0x%x\n",pAd->StaCfg.PmfCfg.RSNCapability));

        CipherTmp[20] = (UCHAR)pAd->StaCfg.PmfCfg.RSNCapability;
        //CipherTmp[21] = (UCHAR)pAd->StaCfg.PmfCfg.RSNCapability >> 8;

        // Make outgoing frame
        MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                          CipherTmpLen,         &CipherTmp[0],
                          END_OF_ARGS);
        FrameLen += tmp;

        // Append Variable IE
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, CipherTmp, CipherTmpLen);
        VarIesOffset += CipherTmpLen;       
    }
    else
    {
        // Do nothing
        ;
    }

    // Set Variable IEs Length
    pAd->StaCfg.ReqVarIELen = VarIesOffset;
    pAd->StaCfg.AssocInfo.RequestIELength = VarIesOffset;

    // OffsetResponseIEs follow ReqVarIE
    pAd->StaCfg.AssocInfo.OffsetResponseIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION) + pAd->StaCfg.ReqVarIELen;

    // Add WSC IE if we are connecting to WSC AP
    DBGPRINT(RT_DEBUG_TRACE, ("%s: - WscState = %d, WscEnAssociateIE= %d, WscCustomizedIESize= %d, WscCustomizedIEData =%x\n", __FUNCTION__, 
        pPort->StaCfg.WscControl.WscState, pPort->StaCfg.WscControl.WscEnAssociateIE, pPort->StaCfg.WscControl.WscCustomizedIESize, pPort->StaCfg.WscControl.WscCustomizedIEData));
    if (((pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)&& (IS_ENABLE_WSC20TB_WSCIE(pAd))) ||
         ((pPort->StaCfg.WscControl.WscEnAssociateIE == TRUE) && (pPort->StaCfg.WscControl.WscCustomizedIESize != 0) && (pPort->StaCfg.WscControl.WscCustomizedIEData != NULL)))
    {
        UCHAR       WscBuf[64], WscIeLen;
            
        if ((pPort->StaCfg.WscControl.WscEnAssociateIE == TRUE) &&
             (pPort->StaCfg.WscControl.WscCustomizedIESize != 0) && (pPort->StaCfg.WscControl.WscCustomizedIEData != NULL))
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                    pPort->StaCfg.WscControl.WscCustomizedIESize, pPort->StaCfg.WscControl.WscCustomizedIEData,
                    END_OF_ARGS);           
            DBGPRINT(RT_DEBUG_TRACE, ("%s: - Using costomized WSC IE, WscState = %d, WscEnAssociateIE= %d, WscCustomizedIESize= %d, WscCustomizedIEData =%x\n", __FUNCTION__, 
                pPort->StaCfg.WscControl.WscState, pPort->StaCfg.WscControl.WscEnAssociateIE, pPort->StaCfg.WscControl.WscCustomizedIESize, pPort->StaCfg.WscControl.WscCustomizedIEData));
        }
        else
        {
            WscMakeAssociateReqIE(pAd, pPort, &WscBuf[0], &WscIeLen);

            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                    WscIeLen,               WscBuf,
                    END_OF_ARGS);
        }
        FrameLen += tmp;
    }
    
    //
    // Add the association params IE (might be WSC IE)
    //
    if (pPort->PortCfg.AdditionalAssocReqIESize > 0)
    {       
        // Check that BSSID matches
        if (ETH_IS_BROADCAST(pPort->PortCfg.AssocIEBSSID) ||
            MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, pPort->PortCfg.AssocIEBSSID))
        {           
            // BSSID matches, add the IE
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                        pPort->PortCfg.AdditionalAssocReqIESize,    pPort->PortCfg.AdditionalAssocReqIEData,
                        END_OF_ARGS);
            FrameLen += tmp;
        }
    }

    // P2P IE behind WSC IE
    P2PPrintStateOnly(pAd, pPort);
    if (P2P_ON(pPort))
    {
        if ((pPort->PortType == WFD_CLIENT_PORT))
        {
            PUCHAR  ptr;
            ptr = pOutBuffer + FrameLen;
            P2pMakeP2pIE(pAd, pPort, PORTSUBTYPE_P2PClient, SUBTYPE_ASSOC_REQ, ptr, &tmp);
            FrameLen += tmp;
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOCREQ - Insert P2P IE \n"));
        }
        // Check the total P2P Attribue IE in this AP.s
        else if (pPort->P2PCfg.P2pManagedParm.TotalNumOfP2pAttribute > 0)
        {
            {
                PUCHAR  ptr;
                DBGPRINT(RT_DEBUG_TRACE, ("ASSOCREQ - Insert P2P IE in case for managed AP to recognize me\n"));
                ptr = pOutBuffer + FrameLen;
                P2pMakeP2pIE(pAd, pPort, PORTSUBTYPE_STA, SUBTYPE_ASSOC_REQ, ptr, &tmp);
                FrameLen += tmp;
            }
        }
    }

    //For USB device, it may need to queue EAP-Identify under WPS to patch Marvell testbed issue.
    pPort->StaCfg.WscControl.bQueueEAPOL =TRUE;

    //
    // Save this Association Request frame for NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION
    //
    PlatformMoveMemory(pAd->StaCfg.AssocRequest, pOutBuffer, FrameLen);
    pAd->StaCfg.AssocRequestLength = (USHORT)FrameLen;

    *pLen = FrameLen; 
}


/*
    ==========================================================================
    Description:
        Pre-build a BEACON frame in the shared memory

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
ULONG MgntPktConstructIBSSBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    ) 
{
    UCHAR         DsLen = 1, IbssLen = 2;
//  UCHAR         SupRateLen, ExtRateLen;
    UCHAR         LocalErpIe[3] = {IE_ERP, 1, 0x04}, ZERO[4] = {0,0,0,0};
    HEADER_802_11 BcnHdr;
    USHORT        CapabilityInfo;
    LARGE_INTEGER FakeTimestamp;
    ULONG         FrameLen = 0;
    PTXWI_STRUC   pTxWI = &pAd->pTxCfg->BeaconTxWI;
    CHAR         *pBeaconFrame = pAd->pTxCfg->BeaconBuf;
    BOOLEAN       Privacy;
    UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR         SupRateLen = 0;
    UCHAR         ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR         ExtRateLen = 0;
    ULONG         tmpLen;
    PHY_CFG       BeaconPhyCfg = {0};
    PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);

    if(pMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return 0;
    }
    
    if (((pAd->StaCfg.AdhocMode == ADHOC_11B) && (pPort->Channel <= 14)) ||
        (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)))
    {
        //
        // CH14 also supports 11B only
        //
        SupRate[0] = 0x82; // 1 mbps
        SupRate[1] = 0x84; // 2 mbps
        SupRate[2] = 0x8b; // 5.5 mbps
        SupRate[3] = 0x96; // 11 mbps
        SupRateLen = 4;
        ExtRateLen = 0;

        //
        // Also Update MlmeRate & RtsRate for B only
        //
        pPort->CommonCfg.MlmeRate = RATE_1;
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_CCK);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, RATE_1);

    }
    else if (pPort->Channel > 14)
    {
        SupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
        SupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        SupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
        SupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        SupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
        SupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        SupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        SupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
        SupRateLen  = 8;
        ExtRateLen  = 0;

        //
        // Also Update MlmeRate & RtsRate for G only & A only
        //
        pPort->CommonCfg.MlmeRate = RATE_6;
        pPort->CommonCfg.RtsRate = RATE_6;
        
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, OfdmRateToRxwiMCS[pPort->CommonCfg.MlmeRate]);
    }
    else
    {
        SupRate[0] = 0x82; // 1 mbps
        SupRate[1] = 0x84; // 2 mbps
        SupRate[2] = 0x8b; // 5.5 mbps
        SupRate[3] = 0x96; // 11 mbps
        SupRateLen = 4;

        ExtRate[0]  = 0x0C;    // 6 mbps, in units of 0.5 Mbps,
        ExtRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        ExtRate[2]  = 0x18;    // 12 mbps, in units of 0.5 Mbps,
        ExtRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        ExtRate[4]  = 0x30;    // 24 mbps, in units of 0.5 Mbps,
        ExtRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        ExtRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        ExtRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps     
        ExtRateLen  = 8;

        //
        // Also Update MlmeRate & RtsRate for B/G MIXED
        //
        pPort->CommonCfg.MlmeRate = RATE_1;
        pPort->CommonCfg.RtsRate = RATE_11;

        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_CCK);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, RATE_1);
    }


    //
    // Keep Basic Mlme Rate.
    //
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacTabEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg));   

    if (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg) == MODE_OFDM)
    {
        WRITE_PHY_CFG_MCS(pAd, &pMacTabEntry->TxPhyCfg, OfdmRateToRxwiMCS[RATE_24]);
    }
    else
    {
        WRITE_PHY_CFG_MCS(pAd, &pMacTabEntry->TxPhyCfg, RATE_1);
    }

    pPort->CommonCfg.BasicMlmeRate = pPort->CommonCfg.MlmeRate;

    // update active supported rate
    pAd->StaActive.SupRateLen = SupRateLen;
    PlatformMoveMemory(pAd->StaActive.SupRate, SupRate, SupRateLen);
    pAd->StaActive.ExtRateLen = ExtRateLen;
#pragma prefast(suppress: __WARNING_USING_UNINIT_VAR, "ExtRate memory referenced depends on ExtRateLen")    
    PlatformMoveMemory(pAd->StaActive.ExtRate, ExtRate, ExtRateLen);

    // compose IBSS beacon frame
    MgtMacHeaderInit(pAd,pPort, &BcnHdr, SUBTYPE_BEACON, 0, BROADCAST_ADDR, pPort->PortCfg.Bssid);
    Privacy = (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus)) || 
              (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || 
              (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled);
    CapabilityInfo = CAP_GENERATE(0, 1, Privacy, (pPort->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0);

    MakeOutgoingFrame(pBeaconFrame,                &FrameLen,
                    sizeof(HEADER_802_11),           &BcnHdr, 
                    TIMESTAMP_LEN,                   &FakeTimestamp,
                    2,                               &pPort->CommonCfg.BeaconPeriod,
                    2,                               &CapabilityInfo,
                    1,                               &SsidIe, 
                    1,                               &pPort->PortCfg.SsidLen, 
                    pPort->PortCfg.SsidLen,          pPort->PortCfg.Ssid,
                    1,                               &SupRateIe, 
                    1,                               &SupRateLen,
                    SupRateLen,                      SupRate,
                    1,                               &DsIe, 
                    1,                               &DsLen, 
                    1,                               &pPort->Channel,
                    1,                               &IbssIe, 
                    1,                               &IbssLen, 
                    2,                               &pAd->StaActive.AtimWin,
                    END_OF_ARGS);

    // add ERP_IE and EXT_RAE IE of in 802.11g
    if (ExtRateLen)
    {
        ULONG   tmp2;

        MakeOutgoingFrame(pBeaconFrame + FrameLen,         &tmp2,
                        3,                               LocalErpIe,
                        1,                               &ExtRateIe,
                        1,                               &ExtRateLen,
                        ExtRateLen,                      ExtRate,
                        END_OF_ARGS);
        FrameLen += tmp2;
    }

    // If adhoc secruity is set for WPA-None, append the cipher suite IE
    if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPANone)
    {       
        if (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)     // Tkip
        {
            MakeOutgoingFrame(pBeaconFrame + FrameLen,      &tmpLen,
                            1,                          &WpaIe,
                            1,                          &CipherSuiteWpaNoneTkipLen,
                            CipherSuiteWpaNoneTkipLen,  &CipherSuiteWpaNoneTkip[0],
                            END_OF_ARGS);
            FrameLen += tmpLen;
        }
        else if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)    // Aes
        {
            MakeOutgoingFrame(pBeaconFrame + FrameLen,    &tmpLen,
                            1,                        &WpaIe,
                            1,                        &CipherSuiteWpaNoneAesLen,
                            CipherSuiteWpaNoneAesLen,   &CipherSuiteWpaNoneAes[0],
                            END_OF_ARGS);
            FrameLen += tmpLen;
        }
    }

    if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) && (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled))
    {
        MakeOutgoingFrame(pBeaconFrame + FrameLen,    &tmpLen,
                        1,                        &Wpa2Ie,
                        1,                        &CipherSuiteAdHocWpa2AesLen,
                        CipherSuiteAdHocWpa2AesLen,   &CipherSuiteAdHocWpa2Aes[0],
                        END_OF_ARGS);
        FrameLen += tmpLen;
    }

    //add AdditionalIE from OID_DOT11_IBSS_PARAMS
    if(pPort->PortCfg.AdditionalBeaconIESize > 0)
    {
        MakeOutgoingFrame(pBeaconFrame + FrameLen,    &tmpLen,
                        pPort->PortCfg.AdditionalBeaconIESize ,   pPort->PortCfg.AdditionalBeaconIEData,
                        END_OF_ARGS);
        FrameLen += tmpLen;
    }

    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only supports 11B and will not carry any HT info on beacons
        ;
    }
    else
    {
        // AdhocN Support
        if ((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            ULONG TmpLen;
            UCHAR   HtLen = SIZE_HT_CAP_IE, HtLen2 = SIZE_ADD_HT_INFO_IE;
            HT_CAPABILITY_IE    HtCapability;

            // No support GF
            pPort->CommonCfg.HtCapability.HtCapInfo.GF = 0;
            pPort->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;

            PlatformZeroMemory(&HtCapability, sizeof(HT_CAPABILITY_IE));
            PlatformMoveMemory(&HtCapability, &pPort->CommonCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            // Change MCSSet to TX Rate (DesiredHtPhy.MCSSet for TX, HtCapability.MCSSet for RX)
            PlatformMoveMemory(&HtCapability.MCSSet[0], &pPort->CommonCfg.DesiredHtPhy.MCSSet[0], 16);
            MakeOutgoingFrame(pBeaconFrame + FrameLen,              &TmpLen,
                                1,                                  &HtCapIe,
                                1,                                  &HtLen,
                                HtLen,                              &HtCapability, 
                                1,                                  &AddHtInfoIe,
                                1,                                  &HtLen2,
                                HtLen2,                             &pPort->CommonCfg.AddHTInfo,
                                END_OF_ARGS);
            FrameLen += TmpLen;
        }

        // VHT
        if ((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode == PHY_11VHT))
        {
             DBGPRINT(RT_DEBUG_WARN,("%s,LINE_%d, to be implement!!\n",__FUNCTION__,__LINE__));
        }
    }

    if (pPort->Channel > 14)
    {
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, TRUE, FALSE, FALSE, TRUE, 0, RESERVED_WCID, FrameLen, 
                0, 0, IFS_HTTXOP, FALSE, pPort->CommonCfg.MgmtPhyCfg, FALSE, TRUE, FALSE);
    }
    else
    {
        // Set to use 1Mbps for Adhoc beacon.
        //Transmit.word = 0;
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &BeaconPhyCfg, 0);
        XmitWriteTxWI(pAd, pPort, pTxWI, FALSE, TRUE, FALSE, FALSE, TRUE, 0, RESERVED_WCID, FrameLen, 
            0, 0, IFS_HTTXOP, FALSE, BeaconPhyCfg, FALSE, TRUE, FALSE);
    }

    //
    // For WiFi four stations beacon fair generation issue.
    // Modify Beacon's TxD cwmin. Recommend by Jerry.
    // 1.) Modify MAC register TXRX_CSR10 Bit 7:0
    // or 
    // 2.) Modify cwmin.
    //

    DBGPRINT(RT_DEBUG_TRACE, ("MgntPktConstructIBSSBeaconFrame (len=%d), SupRateLen=%d, ExtRateLen=%d, Channel=%d, AdhocMode=%d, PhyMode=%d, bAdhocNMode=%d, ExtChanOffset = %d\n", 
                    FrameLen, SupRateLen, ExtRateLen, pPort->Channel, pAd->StaCfg.AdhocMode, pPort->CommonCfg.PhyMode,pAd->StaCfg.bAdhocNMode,pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset ));    

    return FrameLen;
}

VOID
MgntPktConstructIBSSProbRspPktFrame(
    IN PMP_PORT  pPort,
    IN PUCHAR         Addr2,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pFrameLen
    )
{
    PMP_ADAPTER    pAd = pPort->pAd;
    HEADER_802_11   ProbeRspHdr;
    BOOLEAN         Privacy;
    USHORT          CapabilityInfo;
    LARGE_INTEGER   FakeTimestamp;
    UCHAR           DsLen = 1, IbssLen = 2;
    UCHAR           LocalErpIe[3] = {IE_ERP, 1, 0};
    UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR         SupRateLen = 0;
    UCHAR         ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR         ExtRateLen = 0;
    ULONG         FrameLen =0;
    UCHAR         HtLen,AddHtLen, NewExtLen;

    if (((pAd->StaCfg.AdhocMode == ADHOC_11B) && (pPort->Channel <= 14)) ||
        (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)))
    {
        //
        //CH14 also supports 11B only
        //
        SupRate[0] = 0x82; // 1 mbps
        SupRate[1] = 0x84; // 2 mbps
        SupRate[2] = 0x8b; // 5.5 mbps
        SupRate[3] = 0x96; // 11 mbps
        SupRateLen = 4;
        ExtRateLen = 0;
    }
    else if (pPort->Channel > 14)
    {
        SupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
        SupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        SupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
        SupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        SupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
        SupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        SupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        SupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
        SupRateLen  = 8;
        ExtRateLen  = 0;
    }
    else
    {
        SupRate[0] = 0x82; // 1 mbps
        SupRate[1] = 0x84; // 2 mbps
        SupRate[2] = 0x8b; // 5.5 mbps
        SupRate[3] = 0x96; // 11 mbps
        SupRateLen = 4;

        ExtRate[0]  = 0x0C;    // 6 mbps, in units of 0.5 Mbps,
        ExtRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
        ExtRate[2]  = 0x18;    // 12 mbps, in units of 0.5 Mbps,
        ExtRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
        ExtRate[4]  = 0x30;    // 24 mbps, in units of 0.5 Mbps,
        ExtRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
        ExtRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
        ExtRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps     
        ExtRateLen  = 8;
    }
    
    MgtMacHeaderInit(pAd,pPort, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, Addr2, pPort->PortCfg.Bssid);

    Privacy = (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus)) || 
              (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || 
                (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled);

    CapabilityInfo = CAP_GENERATE(0, 1, Privacy, (pPort->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0);

    MakeOutgoingFrame(pOutBuffer,                   &FrameLen, 
                      sizeof(HEADER_802_11),        &ProbeRspHdr, 
                      TIMESTAMP_LEN,                &FakeTimestamp,
                      2,                            &pPort->CommonCfg.BeaconPeriod,
                      2,                            &CapabilityInfo,
                      1,                            &SsidIe, 
                      1,                            &pPort->PortCfg.SsidLen, 
                      pPort->PortCfg.SsidLen,       pPort->PortCfg.Ssid,
                      1,                            &SupRateIe, 
                      1,                            &SupRateLen,
                      SupRateLen,           SupRate, 
                      1,                            &DsIe, 
                      1,                            &DsLen, 
                      1,                            &pPort->Channel,
                      1,                            &IbssIe, 
                      1,                            &IbssLen, 
                      2,                            &pAd->StaActive.AtimWin,
                      END_OF_ARGS);

    if (ExtRateLen)
    {
        ULONG tmp;
        MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp, 
                          3,                            LocalErpIe,
                          1,                            &ExtRateIe,
                          1,                            &ExtRateLen,
                          ExtRateLen,           ExtRate,
                          END_OF_ARGS);
        FrameLen += tmp;
    }

    // If adhoc secruity is set for WPA-None, append the cipher suite IE
    if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPANone)
    {
        ULONG   tmp;
        
        if (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)     // Tkip
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                                1,                          &WpaIe,
                                1,                          &CipherSuiteWpaNoneTkipLen,
                                CipherSuiteWpaNoneTkipLen,  &CipherSuiteWpaNoneTkip[0],
                                END_OF_ARGS);
            FrameLen += tmp;
        }
        else if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)    // Aes
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                                1,                          &WpaIe,
                                1,                          &CipherSuiteWpaNoneAesLen,
                                CipherSuiteWpaNoneAesLen,       &CipherSuiteWpaNoneAes[0],
                                END_OF_ARGS);
            FrameLen += tmp;
        }
    }

    if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) && (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled))
    {
        ULONG tmp;

        MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                            1,                        &Wpa2Ie,
                            1,                        &CipherSuiteAdHocWpa2AesLen,
                            CipherSuiteAdHocWpa2AesLen,   &CipherSuiteAdHocWpa2Aes[0],
                            END_OF_ARGS);
        FrameLen += tmp;
    }

    //add AdditionalIE from OID_DOT11_IBSS_PARAMS
    if(pPort->PortCfg.AdditionalBeaconIESize > 0)
    {
        ULONG tmp;

        MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                            pPort->PortCfg.AdditionalBeaconIESize ,   pPort->PortCfg.AdditionalBeaconIEData,
                            END_OF_ARGS);
        FrameLen += tmp;
    }

    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only supports 11B and will not carry any HT info on beacons
        ;
    }
    else
    {
        if ((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            ULONG TmpLen;
            HT_CAPABILITY_IE    HtCapability;
            HtLen = sizeof(pPort->CommonCfg.HtCapability);
            AddHtLen = sizeof(pPort->CommonCfg.AddHTInfo);
            NewExtLen = 1;
            
            // No support GF
            pPort->CommonCfg.HtCapability.HtCapInfo.GF = 0;
            pPort->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;

            PlatformZeroMemory(&HtCapability, sizeof(HT_CAPABILITY_IE));
            PlatformMoveMemory(&HtCapability, &pPort->CommonCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            // Change MCSSet to TX Rate (DesiredHtPhy.MCSSet for TX, HtCapability.MCSSet for RX)
            PlatformMoveMemory(&HtCapability.MCSSet[0], &pPort->CommonCfg.DesiredHtPhy.MCSSet[0], 16);

            pPort->CommonCfg.NewExtChanOffset.NewExtChanOffset = pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset;
            
            //New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame
            MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                                1,                                &HtCapIe,
                                1,                                &HtLen,
                                 sizeof(HT_CAPABILITY_IE),          &HtCapability, 
                                1,                                &AddHtInfoIe,
                                1,                                &AddHtLen,
                                sizeof(ADD_HT_INFO_IE),          &pPort->CommonCfg.AddHTInfo, 
                                1,                                &NewExtChanIe,
                                1,                                &NewExtLen,
                                sizeof(NEW_EXT_CHAN_IE),          &pPort->CommonCfg.NewExtChanOffset, 
                                END_OF_ARGS);
            FrameLen += TmpLen;
        }

        // add IEs for VHT
        if (pPort->Channel>14)
        {
            if(pPort->CommonCfg.PhyMode == PHY_11VHT)
            {
                ULONG TmpLen;
                UCHAR VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                UCHAR VhtOpIeLen = SIZE_OF_VHT_OP_IE;
                MakeOutgoingFrame(pOutBuffer+FrameLen,  &TmpLen,
                                    1,                  &VhtCapIe,
                                    1,                  &VhtCapIeLen,                                         
                                    SIZE_OF_VHT_CAP_IE, &pPort->CommonCfg.DesiredVhtPhy.VhtCapability,
                                    END_OF_ARGS);
                FrameLen += TmpLen;

                // add VHT_OP_IE for VHT                        
                MakeOutgoingFrame(pOutBuffer+FrameLen,  &TmpLen,
                                    1,                  &VhtOpIe,
                                    1,                  &VhtOpIeLen,
                                    SIZE_OF_VHT_OP_IE,  &pPort->CommonCfg.DesiredVhtPhy.VhtOperation,
                                    END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
    }

    *pFrameLen = FrameLen;
}
