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
    NdisInfo.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

#define PHY_TYPE_BUFFER_SIZE (sizeof(DOT11_SUPPORTED_PHY_TYPES) + sizeof(DOT11_PHY_TYPE) * STA_DESIRED_PHY_MAX_COUNT)
#define INVALID_PHY_ID 0x80000000U


ULONG64
MlmeInfoQueryLinkSpeed(
        IN  PMP_ADAPTER pAd,
        IN  PMP_PORT  pPort,
        IN  PPHY_CFG pPhyCfg)
{
    ULONG   rateIdx = 0;
    ULONG64 LinkSpeed;
    UCHAR   ShortGI;
    UCHAR initOffset = 16;

    if (READ_PHY_CFG_MODE(pAd, pPhyCfg) == MODE_CCK)    
    {
        switch (READ_PHY_CFG_MCS(pAd, pPhyCfg))
        {
            case RATE_1:   LinkSpeed = 10000;    break;
            case RATE_2:   LinkSpeed = 20000;    break;
            case RATE_5_5: LinkSpeed = 55000;    break;
            case RATE_11:  LinkSpeed = 110000;   break;
            default:       LinkSpeed = 10000;    break;
        }
    }
    else if (READ_PHY_CFG_MODE(pAd, pPhyCfg) == MODE_OFDM)
    {
        switch (READ_PHY_CFG_MCS(pAd, pPhyCfg))
        {
            case 0:   LinkSpeed = 60000;    break;
            case 1:   LinkSpeed = 90000;    break;
            case 2:  LinkSpeed = 120000;    break;
            case 3:  LinkSpeed = 180000;    break;
            case 4:  LinkSpeed = 240000;    break;
            case 5:  LinkSpeed = 360000;    break;
            case 6:  LinkSpeed = 480000;    break;
            case 7:  LinkSpeed = 540000;    break;
            default:       LinkSpeed = 10000;    break;
        }
    }
    else
    {
        // AP report MAX MCS = 15.  Station report currently used rate. So I fix AP report use MCS=15
        if (pPort->PortSubtype == PORTSUBTYPE_STA || (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
        {
            ShortGI = (UCHAR)READ_PHY_CFG_SHORT_GI(pAd, pPhyCfg);
            
            if (READ_PHY_CFG_MODE(pAd, pPhyCfg) == MODE_VHT)
            {
                rateIdx = GetVhtRateTableIdx(pAd, 
                                     READ_PHY_CFG_BW(pAd, pPhyCfg), 
                                     READ_PHY_CFG_NSS(pAd, pPhyCfg), 
                                     READ_PHY_CFG_SHORT_GI(pAd, pPhyCfg), 
                                     READ_PHY_CFG_MCS(pAd, pPhyCfg));
            }
            else
            {
            if((!VHT_CAPABLE(pAd)) && 
                    (pAd->StaCfg.bIdle300M == TRUE) && 
                    ((READ_PHY_CFG_MCS(pAd, pPhyCfg) == MCS_15) || 
                      (READ_PHY_CFG_MCS(pAd, pPhyCfg) == MCS_7)))
            {               
                ShortGI = GI_400;               
            }

            if (READ_PHY_CFG_MCS(pAd, pPhyCfg) > MCS_15)
            {
                    rateIdx = 64 + ((UCHAR)READ_PHY_CFG_BW(pAd, pPhyCfg) * 16) + (ShortGI * 32) + ((UCHAR)READ_PHY_CFG_MCS(pAd, pPhyCfg));
                }
                else
                {
                    rateIdx = 16 + ((UCHAR)READ_PHY_CFG_BW(pAd, pPhyCfg) * 16) + (ShortGI * 32) + ((UCHAR)READ_PHY_CFG_MCS(pAd, pPhyCfg));
                }
            }
            }
            
        if (READ_PHY_CFG_MODE(pAd, pPhyCfg) == MODE_VHT)
        {
            // Avoid getting the zero VhtRate, when MCS=9 BW=0 GI=1 NSS=0 (rateIdx=169)
            if ((rateIdx!=0)&&VhtRateTable[rateIdx]==0)
            {
                rateIdx=rateIdx-1;
            }
            LinkSpeed = VhtRateTable[rateIdx] * 5000;
        }
        else
        {
            LinkSpeed = CckOfdmHtRateTable[rateIdx] * 5000;
        }
    }

    return LinkSpeed * 100;
}

/*
    ========================================================================
    
    Routine Description:
        Add WPA key process

    Arguments:
        pAd                     Pointer to our adapter
        pBuf                            Pointer to the where the key stored

    Return Value:
        NDIS_SUCCESS                    Add key successfully

    IRQL = DISPATCH_LEVEL
    
    Note:

    ========================================================================
*/
NDIS_STATUS 
MlmeInfoWPAAddKeyProc(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PVOID           pBuf
    )
{
    PNDIS_802_11_KEY    pKey;
    ULONG               KeyIdx;

    PUCHAR              pTxMic, pRxMic;
    BOOLEAN             bTxKey;         // Set the key as transmit key
    BOOLEAN             bPairwise;      // Indicate the key is pairwise key
    BOOLEAN             bKeyRSC;        // indicate the receive  SC set by KeyRSC value.
                                // Otherwise, it will set by the NIC.
    BOOLEAN             bAuthenticator; // indicate key is set by authenticator.
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    if(pBssidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return NDIS_STATUS_INVALID_DATA;
    }

    pKey = (PNDIS_802_11_KEY) pBuf;
    KeyIdx = pKey->KeyIndex & 0xff;
    // Bit 31 of Add-key, Tx Key
    bTxKey      = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
    // Bit 30 of Add-key PairwiseKey
    bPairwise   = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;
    // Bit 29 of Add-key KeyRSC
    bKeyRSC     = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
    // Bit 28 of Add-key Authenticator
    bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoWPAAddKeyProc===> pKey->KeyIndex  = 0x%x\n",pKey->KeyIndex ));
    // 1. Check Group / Pairwise Key
    if (bPairwise)  // Pairwise Key
    {
        // 1. KeyIdx must be 0, otherwise, return NDIS_STATUS_INVALID_DATA
        if (KeyIdx != 0)
            return(NDIS_STATUS_INVALID_DATA);

        // 2. Check bTx, it must be true, otherwise, return NDIS_STATUS_INVALID_DATA
        if (bTxKey == FALSE)
            return(NDIS_STATUS_INVALID_DATA);

        // 3. If BSSID is all 0xff, return NDIS_STATUS_INVALID_DATA
        if (MAC_ADDR_EQUAL(pKey->BSSID, BROADCAST_ADDR))
            return(NDIS_STATUS_INVALID_DATA);

        // 3.1 Check Pairwise key length for TKIP key. For AES, it's always 128 bits
        if ((pPort->PortCfg.PairCipher == Ralink802_11Encryption2Enabled) && (pKey->KeyLength != LEN_TKIP_KEY))
            return(NDIS_STATUS_INVALID_DATA);

        pPort->SharedKey[BSS0][KeyIdx].Type = PAIRWISE_KEY;

        if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2)
        {
            // Send media specific event to start PMKID caching
            MlmeInfoIndicateWPA2Status(pAd, pPort);
        }
    }
    else
    {
        // 1. Check BSSID, if not current BSSID or Bcast, return NDIS_STATUS_INVALID_DATA
        if ((! MAC_ADDR_EQUAL(pKey->BSSID, BROADCAST_ADDR)) &&
            (! MAC_ADDR_EQUAL(pKey->BSSID, pPort->PortCfg.Bssid)))
            return(NDIS_STATUS_INVALID_DATA);

        // 2. Check Key index for supported Group Key
        if (KeyIdx >= GROUP_KEY_NUM + 2)
        {
            return(NDIS_STATUS_INVALID_DATA);
        }
        else if (KeyIdx >= GROUP_KEY_NUM)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("KeyIdx:%d, KeyLen:%d\n", KeyIdx, pKey->KeyLength));
            pAd->StaCfg.PmfCfg.IGTKKeyIdx = (UCHAR)KeyIdx;
            PlatformMoveMemory(pAd->StaCfg.PmfCfg.IPN[KeyIdx-4], &pKey->KeyRSC, 6);
            PlatformMoveMemory(pAd->StaCfg.PmfCfg.IGTK[KeyIdx-4], pKey->KeyMaterial, pKey->Length -12);
            return(NDIS_STATUS_SUCCESS);
        }

        // 3. Set as default Tx Key if bTxKey is TRUE
        if (bTxKey == TRUE)
            pPort->PortCfg.DefaultKeyId = (UCHAR) KeyIdx;

        pPort->SharedKey[BSS0][KeyIdx].Type = GROUP_KEY;
    }

    // 4. Select RxMic / TxMic based on Supp / Authenticator
    if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPANone)
    {
        // for WPA-None Tx, Rx MIC is the same
        pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
        pRxMic = pTxMic;
    }
    else if (bAuthenticator == TRUE)
    {
        pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
        pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
    }
    else
    {
        pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
        pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
    }

    // 6. Check RxTsc
    if (bKeyRSC == TRUE)
    {
        PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].RxTsc, &pKey->KeyRSC, 6);
        PlatformMoveMemory(pBssidMacTabEntry->PairwiseKey.RxTsc, &pKey->KeyRSC, 6);            
    }
    else
    {
        PlatformZeroMemory(pPort->SharedKey[BSS0][KeyIdx].RxTsc, 6);        
    }

    // 7. Copy information into Pairwise Key structure.
    // pKey->KeyLength will include TxMic and RxMic, therefore, we use 16 bytes hardcoded.
    pPort->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;        
    pBssidMacTabEntry->PairwiseKey.KeyLen = (UCHAR)pKey->KeyLength;
    // pKey->KeyLength might only contain 5, 13 or 16.
    // And it might reference to invliad memory if we copy fixed length as 16.
    PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].Key, &pKey->KeyMaterial, min(pKey->KeyLength,16));
    PlatformMoveMemory(pBssidMacTabEntry->PairwiseKey.Key, &pKey->KeyMaterial, min(pKey->KeyLength,16));
    if (pKey->KeyLength == LEN_TKIP_KEY)
    {
        // Only Key lenth equal to TKIP key have these
        PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].RxMic, pRxMic, 8);
        PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].TxMic, pTxMic, 8);
        PlatformMoveMemory(pBssidMacTabEntry->PairwiseKey.RxMic, pRxMic, 8);
        PlatformMoveMemory(pBssidMacTabEntry->PairwiseKey.TxMic, pTxMic, 8);
        //
        // pKey->KeyLength = 32.
        // 16: TkipKey, 8: RxMIC Key, 8: TxMIC Key
        // Update Key len here.
        //
        pPort->SharedKey[BSS0][KeyIdx].KeyLen = 16;
    }
    COPY_MAC_ADDR(pPort->SharedKey[BSS0][KeyIdx].BssId, pKey->BSSID);

    // Init TxTsc to one based on WiFi WPA specs
    pPort->SharedKey[BSS0][KeyIdx].TxTsc[0] = 1;
    pPort->SharedKey[BSS0][KeyIdx].TxTsc[1] = 0;
    pPort->SharedKey[BSS0][KeyIdx].TxTsc[2] = 0;
    pPort->SharedKey[BSS0][KeyIdx].TxTsc[3] = 0;
    pPort->SharedKey[BSS0][KeyIdx].TxTsc[4] = 0;
    pPort->SharedKey[BSS0][KeyIdx].TxTsc[5] = 0;
    // 4. Init TxTsc to one based on WiFi WPA specs
    pBssidMacTabEntry->PairwiseKey.TxTsc[0] = 1;
    pBssidMacTabEntry->PairwiseKey.TxTsc[1] = 0;
    pBssidMacTabEntry->PairwiseKey.TxTsc[2] = 0;
    pBssidMacTabEntry->PairwiseKey.TxTsc[3] = 0;
    pBssidMacTabEntry->PairwiseKey.TxTsc[4] = 0;
    pBssidMacTabEntry->PairwiseKey.TxTsc[5] = 0;

    //
    // for WPA+WEP mixed mode Group Key (WEP-40/WEP-104)
    //
    if ( !bPairwise && 
        ((pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP40)||
        (pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP104)))
    {
        if (pPort->SharedKey[BSS0][KeyIdx].KeyLen == 5)
        {
            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP64;
        }
        else if (pPort->SharedKey[BSS0][KeyIdx].KeyLen == 13)
        {
            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP128;
        }
        else
            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;

    }
    else if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {   
        pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_AES;
    }
    else if (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)
    {   
        pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_TKIP;
    }
    else if (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))
    {
        if (pPort->SharedKey[BSS0][KeyIdx].KeyLen == 5)
        {
            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP64;
        }
        else if (pPort->SharedKey[BSS0][KeyIdx].KeyLen == 13)
        {
            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP128;
        }
        else
        {
            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
        }
    }
    else
    {
        pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
    }

    if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
    {
        //
        // On WPA/WPA2, Update Group Key Cipher.
        //
        if (!bPairwise)
        {
            if (pPort->PortCfg.GroupCipher == Ralink802_11Encryption3Enabled)
                pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_AES;
            else if (pPort->PortCfg.GroupCipher == Ralink802_11Encryption2Enabled)
                pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_TKIP;
        }
    }

    if (pPort->PortSubtype == PORTSUBTYPE_STA || (pPort->PortSubtype == PORTSUBTYPE_P2PClient))  // Pairwise Key. Add BSSID to WCTable
    {
        pBssidMacTabEntry->PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][KeyIdx].CipherAlg;
       pBssidMacTabEntry->PairwiseKey.KeyLen = pPort->SharedKey[BSS0][KeyIdx].KeyLen;
    }


    DBGPRINT(RT_DEBUG_TRACE, ("pPort->SharedKey[BSS0][%d].CipherAlg = %d. DefaultKeyId = %d. \n", KeyIdx, pPort->SharedKey[BSS0][KeyIdx].CipherAlg, pPort->PortCfg.DefaultKeyId));

    if ((bTxKey) && (pPort->PortType == EXTSTA_PORT))  // Pairwise Key. Add BSSID to WCTable
    {
        UCHAR Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
        MlmeInfoAddBSSIDCipher(pAd, pPort, Wcid, pKey);
    }

    // WPA+WEP mixed mode GroupKey. 1 for WEP-40, 5 for WEP-104
    if (((pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP40) || 
        (pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP104)) &&  (!bPairwise))
    {
        AsicAddSharedKeyEntry(pAd,
                                pPort,
                                BSS0,
                                (UCHAR)KeyIdx,
                                pPort->SharedKey[BSS0][KeyIdx].CipherAlg, 
                                pPort->SharedKey[BSS0][KeyIdx].Key, 
                                NULL,
                                NULL);
    }
    else
    {
        
        AsicAddSharedKeyEntry(pAd, 
                                pPort,
                                BSS0, 
                                (UCHAR)KeyIdx, 
                                pPort->SharedKey[BSS0][KeyIdx].CipherAlg,
                                pPort->SharedKey[BSS0][KeyIdx].Key, 
                                pPort->SharedKey[BSS0][KeyIdx].TxMic,
                                pPort->SharedKey[BSS0][KeyIdx].RxMic);
    }

    if (pPort->SharedKey[BSS0][KeyIdx].Type == GROUP_KEY)
    {
        // 802.1x port control
        pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
        DBGPRINT(RT_DEBUG_ERROR,("!!WPA_802_1X_PORT_SECURED!!\n"));
    }

    return (NDIS_STATUS_SUCCESS);
}


VOID 
MlmeInfoStaOidSetWEPDefaultKeyID(  
    IN  PMP_PORT      pPort
    )
{
    PMP_ADAPTER   pAd = pPort->pAd;
    UCHAR           KeyIdx = pPort->PortCfg.DefaultKeyId;
    if ((pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP40) ||
        (pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP104) ||
        (pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP) &&
        (pPort->SharedKey[BSS0][KeyIdx].KeyLen != 0))
    {
        // Default key for tx (shared key)
        UCHAR   IVEIV[8];
        ULONG   WCIDAttri, Value;
        USHORT  offset, offset2, i;
        UCHAR   CipherAlg;
        UCHAR   BssIndex = BSS0;

        if ((pAd->NumberOfPorts > 1) && (pAd->StaCfg.BssType == BSS_INFRA))
        {
            BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
        }
        else if (pAd->OpMode == OPMODE_STAP2P)
        {
            BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
        }


        CipherAlg = pPort->SharedKey[BSS0][KeyIdx].CipherAlg;

        PlatformZeroMemory(IVEIV, 8);
        // Add BSSID to WCTable. because this is Tx wep key.
        // WCID Attribute UDF:3, BSSIdx:3, Alg:3, Keytable:1=PAIRWISE KEY, BSSIdx is 0
        WCIDAttri =(BssIndex << 4) |(CipherAlg<<1)|SHAREDKEYTABLE;

        offset = MAC_WCID_ATTRIBUTE_BASE + ((MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID))* HW_WCID_ATTRI_SIZE);
        RTUSBWriteMACRegister(pAd, offset, WCIDAttri);
        // 1. IV/EIV
        // Specify key index to find shared key.
        IVEIV[3] = (UCHAR)(KeyIdx<< 6); //WEP Eiv bit off. groupkey index is not 0
        offset = PAIRWISE_IVEIV_TABLE_BASE + (MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST) * HW_IVEIV_ENTRY_SIZE);
        offset2 = PAIRWISE_IVEIV_TABLE_BASE + ((MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID))* HW_IVEIV_ENTRY_SIZE);
        if(pAd->NumberOfPorts == 1)
        {
            for (i = 0; i < 8;)
            {
                Value = IVEIV[i];
                Value += (IVEIV[i+1]<<8);
                Value += (IVEIV[i+2]<<16);
                Value += (IVEIV[i+3]<<24);
                RTUSBWriteMACRegister(pAd, offset+i, Value);
                i+=4;
            }
        }
        
        for (i = 0; i < 8;)
        {
            Value = IVEIV[i];
            Value += (IVEIV[i+1]<<8);
            Value += (IVEIV[i+2]<<16);
            Value += (IVEIV[i+3]<<24);      
            RTUSBWriteMACRegister(pAd, offset2+i, Value);
            i+=4;
        }

        //only one port exist , then we can set up broadcast wcid cipher attribute
        // if two port exist, do not set up this attribute, it will  cause SoftAP broadcast packet  not encrypt
        if(pAd->NumberOfPorts == 1)
        {
            // 2. WCID Attribute UDF:3, BSSIdx:3, Alg:3, Keytable:use share key, BSSIdx is 0
            WCIDAttri = (BssIndex << 4) | (pPort->SharedKey[BSS0][KeyIdx].CipherAlg<<1)|SHAREDKEYTABLE;
            offset = MAC_WCID_ATTRIBUTE_BASE + (MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST)* HW_WCID_ATTRI_SIZE);
            DBGPRINT(RT_DEBUG_TRACE, ("BSS0Mcast_WCID : offset = %x, WCIDAttri = %x\n", offset, WCIDAttri));
            RTUSBWriteMACRegister(pAd, offset, WCIDAttri);
        }
    }

}


NDIS_STATUS 
MlmeInfoAddKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx,
    IN  DOT11_CIPHER_ALGORITHM AlgorithmId,
    IN  ULONG           KeyLen,
    IN  PUCHAR          KeyValue,
    IN  BOOLEAN         bDelete,
    IN  BOOLEAN         bPairwise,
    IN  BOOLEAN         bTxKey
    )
{
    PDOT11_KEY_ALGO_CCMP        CCMPKey = NULL;
    PDOT11_KEY_ALGO_TKIP_MIC    TKIPKey = NULL;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    PDOT11_KEY_ALGO_BIP         BIPKey = NULL;
#endif
    UCHAR                       CipherAlg = CIPHER_NONE;
    MAC_TABLE_ENTRY             *pEntry;
    ULONGLONG                   TSC=0;
    PRTMP_ADHOC_LIST            pAdhocEntry;
    UCHAR                       BssIndex = BSS0;
    UCHAR                      Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID); // as sta default
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    if ((pAd->NumberOfPorts > 1) 
        && ((pAd->StaCfg.BssType == BSS_INFRA)
        || (pPort->PortType == WFD_CLIENT_PORT)
        || (pPort->PortType == WFD_GO_PORT)))
    {
        BssIndex = (UCHAR)pPort->PortNumber;
        
    }
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoAddKey, BssIndex = %d, BssType=%d, PortType=%d \n", BssIndex, pAd->StaCfg.BssType, pPort->PortType));

    // P2P ==============>>>
    // Select wcid, default is 1
    if (PlatformEqualMemory(MacAddr,ZERO_MAC_ADDR, MAC_ADDR_LEN) && (pPort->PortType == WFD_GO_PORT))
    {
        Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);
        DBGPRINT(RT_DEBUG_TRACE, ("Zero address for group key (bPairwise=%d, wcid=%d) \n", bPairwise, Wcid));
    }
    else if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        Wcid = P2pGetClientWcid(pAd, pPort);
        
        pWcidMacTabEntry =  MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);

        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
            return NDIS_STATUS_INVALID_DATA;
        }
        
         // Set port secured when received pairwise key
        pWcidMacTabEntry->PortSecured = WPA_802_1X_PORT_SECURED;
        DBGPRINT(RT_DEBUG_TRACE, ("It's for P2P Client (bPairwise=%d, wcid=%d) \n", bPairwise, Wcid));
    }   
    else if (pPort->PortType == WFD_GO_PORT)
    {
        pEntry = MacTableLookup(pAd,  pPort, MacAddr);
        if(pEntry != NULL)
        {               
            
            // fill wcid
            Wcid = (UCHAR)pEntry->Aid;
            // Set 802.1x port secured
            if ((pEntry->AuthMode >= DOT11_AUTH_ALGO_WPA) || (pEntry->WepStatus >= DOT11_CIPHER_ALGO_TKIP))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("[Win8] GO add pairwise key to Aid #%d. Port is secured while Auth=%d and Encr=%d \n", pEntry->Aid, pEntry->AuthMode, pEntry->WepStatus));
                pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
            }
        }
        else if (!PlatformEqualMemory(MacAddr,ZERO_MAC_ADDR, MAC_ADDR_LEN))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("don't have mac entry yet,add pairwise key \n")); 
#if 0 // don't add key if this entry is not existed         
            pEntry = MacTableInsertEntry(pAd, pPort, MacAddr, TRUE, FALSE);
            if(pEntry != NULL)
            {               
                // fill wcid
                Wcid = (UCHAR)pEntry->Aid;
                // Set 802.1x port secured
                if ((pEntry->AuthMode >= DOT11_AUTH_ALGO_WPA) || (pEntry->WepStatus >= DOT11_CIPHER_ALGO_TKIP))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[Win8] GO add pairwise key to Aid #%d. Port is secured while Auth=%d and Encr=%d \n", pEntry->Aid, pEntry->AuthMode, pEntry->WepStatus));
                    pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
                }
            }
#else
            DBGPRINT(RT_DEBUG_ERROR, ("add key failed \n"));    
            return NDIS_STATUS_FAILURE;
#endif
        }
    }
    // P2P <<<==============

    
    //ASSERT(KeyIdx < 4);
    ASSERT(KeyIdx < 6); //PMF will use KeyIdx 4 or 5 for IGTK

    if (!bDelete)
    {
        switch (AlgorithmId)
        {
            case DOT11_CIPHER_ALGO_WEP:
                if (KeyLen == 5)
                {
                    CipherAlg = CIPHER_WEP64;
                }
                else if (KeyLen == 13)
                {
                    CipherAlg = CIPHER_WEP128;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey Invalid wep key\n"));
                    return NDIS_STATUS_INVALID_DATA;
                }
                break;              
            case DOT11_CIPHER_ALGO_WEP40:
                CipherAlg = CIPHER_WEP64;
                
                if (KeyLen != 5)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey, Invalild DOT11_CIPHER_ALGO_WEP40 KeyLen=%d\n", KeyLen));
                    return NDIS_STATUS_INVALID_DATA;
                }
                break;
                
            case DOT11_CIPHER_ALGO_WEP104:
                CipherAlg = CIPHER_WEP128;

                if (KeyLen != 13)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey, Invalild DOT11_CIPHER_ALGO_WEP104 KeyLen=%d\n", KeyLen));
                    return NDIS_STATUS_INVALID_DATA;
                }
                break;
                
            case DOT11_CIPHER_ALGO_CCMP:
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))                            
                //11w use this path for IGTK insert, excepts Win8
                if (KeyIdx >=4) //IGTK
                {   
                    PDOT11_KEY_ALGO_CCMP        BIPKey = NULL;
                    // 
                    // DOT11_CIPHER_ALGO_MFPCCMP is really just DOT11_CIPHER_ALGO_CCMP.  
                    // Set flag to indicate that MFP is active.
                    // 
                    
                    DBGPRINT(RT_DEBUG_TRACE, ("MFP: cipher is BIP\n"));

                    BIPKey = (PDOT11_KEY_ALGO_CCMP)KeyValue;
                    if (BIPKey->ulCCMPKeyLength != 16)
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey, Invalid IGTK =%d\n", CCMPKey->ulCCMPKeyLength));
                        return NDIS_STATUS_INVALID_DATA;
                    }

                    DBGPRINT(RT_DEBUG_ERROR, ("MFP: KeyIdx:%d, KeyLen:%d\n", KeyIdx, BIPKey->ulCCMPKeyLength));

                    pAd->StaCfg.PmfCfg.IGTKKeyIdx = (UCHAR)KeyIdx;
                    PlatformMoveMemory(pAd->StaCfg.PmfCfg.IPN[KeyIdx-4], BIPKey->ucIV48Counter, 6);
                    PlatformMoveMemory(pAd->StaCfg.PmfCfg.IGTK[KeyIdx-4], BIPKey->ucCCMPKey, BIPKey->ulCCMPKeyLength);
                    return NDIS_STATUS_SUCCESS;
                }
#endif
                                
                CipherAlg = CIPHER_AES;
                CCMPKey = (PDOT11_KEY_ALGO_CCMP)KeyValue;

                TSC = ((ULONGLONG)CCMPKey->ucIV48Counter[0]) |
                        (((ULONGLONG)CCMPKey->ucIV48Counter[1]) << 8) |
                        (((ULONGLONG)CCMPKey->ucIV48Counter[2]) << 16) |
                        (((ULONGLONG)CCMPKey->ucIV48Counter[3]) << 24) |
                        (((ULONGLONG)CCMPKey->ucIV48Counter[4]) << 32) |
                        (((ULONGLONG)CCMPKey->ucIV48Counter[5]) << 40);

                if (CCMPKey->ulCCMPKeyLength != 16)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey, Invalid AES keylen=%d\n", CCMPKey->ulCCMPKeyLength));
                    return NDIS_STATUS_INVALID_DATA;
                }
                KeyValue = CCMPKey->ucCCMPKey;
                KeyLen = CCMPKey->ulCCMPKeyLength;
                break;
                
            case DOT11_CIPHER_ALGO_TKIP:
                CipherAlg = CIPHER_TKIP;
                TKIPKey = (PDOT11_KEY_ALGO_TKIP_MIC)KeyValue;
                if (TKIPKey->ulTKIPKeyLength != 16 || TKIPKey->ulMICKeyLength != 16)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey:: Invalid TKIP key len=%d, MIC Keylen=%d\n", 
                                    TKIPKey->ulTKIPKeyLength, TKIPKey->ulMICKeyLength));
                    return NDIS_STATUS_INVALID_DATA;
                }
                KeyValue = TKIPKey->ucTKIPMICKeys;
                KeyLen = TKIPKey->ulTKIPKeyLength;
                break;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))   //Win8 has this define, not used in Win7
            case DOT11_CIPHER_ALGO_BIP:
                // 
                // DOT11_CIPHER_ALGO_MFPCCMP is just DOT11_CIPHER_ALGO_CCMP.  
                // Set flag to indicate that MFP is active.
                // 
                DBGPRINT(RT_DEBUG_TRACE, ("MFP: DOT11_CIPHER_ALGO_BIP\n"));

                BIPKey = (PDOT11_KEY_ALGO_BIP)KeyValue;
                if (BIPKey->ulBIPKeyLength != 16)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey, Invalid IGTK =%d\n", BIPKey->ulBIPKeyLength));
                    return NDIS_STATUS_INVALID_DATA;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("MFP: KeyIdx:%d, KeyLen:%d\n", KeyIdx, BIPKey->ulBIPKeyLength));

                    if(KeyIdx < 4)
                        KeyIdx = 4; //patch MS KeyIdx is 0 for temp
                    pAd->StaCfg.PmfCfg.IGTKKeyIdx = (UCHAR)KeyIdx;
                    PlatformMoveMemory(pAd->StaCfg.PmfCfg.IPN[KeyIdx-4], BIPKey->ucIPN, 6);
                    PlatformMoveMemory(pAd->StaCfg.PmfCfg.IGTK[KeyIdx-4], BIPKey->ucBIPKey, BIPKey->ulBIPKeyLength);
                    return(NDIS_STATUS_SUCCESS);
                }
                break;
#endif

            default:
                DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey:: Unknow AlgorithmId = 0x%x\n", AlgorithmId));
                break;
        }

        if (pPort->StaCfg.WscControl.bWPSSession == FALSE)
        {
            if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
            {   
                UCHAR   i;
                BOOLEAN WepStatusEqualAP = FALSE;
                for (i = 0; i < pAd->ScanTab.BssNr; i++) 
                {
                    BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];
                    // pick up the first available vacancy
                    if ((MAC_ADDR_EQUAL(pInBss->Bssid, pPort->PortCfg.Bssid)) && (pInBss->WepStatus == pPort->PortCfg.WepStatus))
                    {
                        WepStatusEqualAP = TRUE;
                        break;
                    }
                }
                // The AP uses OPEN-NONE.
                if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeOpen) && (pPort->PortCfg.WepStatus == Ralink802_11WEPDisabled))
                {
                    LedCtrlSetLed(pAd, LED_NORMAL_CONNECTION_WITHOUT_SECURITY);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_NORMAL_CONNECTION_WITHOUT_SECURITY\n", __FUNCTION__));
                }
                else if (!WepStatusEqualAP)
                {
                    //Do nothing;
                }
                else // The AP uses an encryption algorithm.
                {
                    LedCtrlSetLed(pAd, LED_NORMAL_CONNECTION_WITH_SECURITY);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_NORMAL_CONNECTION_WITH_SECURITY\n", __FUNCTION__));
                }
            }
        }

        pPort->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR)KeyLen;
//      pPort->MacTab.Content[Wcid].PairwiseKey.KeyLen = (UCHAR)KeyLen;
        PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].Key, KeyValue, min(KeyLen, 16));
//      PlatformMoveMemory(pPort->MacTab.Content[Wcid].PairwiseKey.Key, KeyValue, min(KeyLen, 16));

        if (CipherAlg == CIPHER_TKIP)
        {
            PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].RxMic, Add2Ptr(KeyValue,KeyLen), 8);
            PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].TxMic, Add2Ptr(KeyValue,KeyLen + 8), 8);
//          PlatformMoveMemory(pPort->MacTab.Content[Wcid].PairwiseKey.RxMic, Add2Ptr(KeyValue,KeyLen), 8);
//          PlatformMoveMemory(pPort->MacTab.Content[Wcid].PairwiseKey.TxMic, Add2Ptr(KeyValue,KeyLen + 8), 8);
        }
        COPY_MAC_ADDR(pPort->SharedKey[BSS0][KeyIdx].BssId, MacAddr);
        pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CipherAlg;
            
        // Init TxTsc to one based on WiFi WPA specs
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[0] = 1;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[1] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[2] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[3] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[4] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[5] = 0;
        // 4. Init TxTsc to one based on WiFi WPA specs
//      pPort->MacTab.Content[Wcid].PairwiseKey.TxTsc[0] = 1;
//      pPort->MacTab.Content[Wcid].PairwiseKey.TxTsc[1] = 0;
//      pPort->MacTab.Content[Wcid].PairwiseKey.TxTsc[2] = 0;
//      pPort->MacTab.Content[Wcid].PairwiseKey.TxTsc[3] = 0;
//      pPort->MacTab.Content[Wcid].PairwiseKey.TxTsc[4] = 0;
//      pPort->MacTab.Content[Wcid].PairwiseKey.TxTsc[5] = 0;


        // Set pairwise related info here.
        if (bPairwise)
        {
            pWcidMacTabEntry =  MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);
            if(pWcidMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
                return NDIS_STATUS_INVALID_DATA;
            }
            
            pWcidMacTabEntry->PairwiseKey.KeyLen = (UCHAR)KeyLen;
            PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.Key, KeyValue, min(KeyLen, 16));

            pWcidMacTabEntry->PairwiseKey.KeyLen = (UCHAR)KeyLen;
            PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.Key, KeyValue, min(KeyLen, 16));
            if (CipherAlg == CIPHER_TKIP)
            {
                PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.RxMic, Add2Ptr(KeyValue,KeyLen), 8);
                PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.TxMic, Add2Ptr(KeyValue,KeyLen + 8), 8);
            }

            // 4. Init TxTsc to one based on WiFi WPA specs
            pWcidMacTabEntry->PairwiseKey.TxTsc[0] = 1;
            pWcidMacTabEntry->PairwiseKey.TxTsc[1] = 0;
            pWcidMacTabEntry->PairwiseKey.TxTsc[2] = 0;
            pWcidMacTabEntry->PairwiseKey.TxTsc[3] = 0;
            pWcidMacTabEntry->PairwiseKey.TxTsc[4] = 0;
            pWcidMacTabEntry->PairwiseKey.TxTsc[5] = 0;

            //if ((pAd->OpMode == OPMODE_STA))  // Pairwise Key. Add BSSID to WCTable
            if ((pPort->PortType == EXTSTA_PORT) || (pPort->PortType == WFD_CLIENT_PORT) || (pPort->PortType == WFD_GO_PORT))
            {
                pWcidMacTabEntry->PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][KeyIdx].CipherAlg;
                pWcidMacTabEntry->PairwiseKey.KeyLen = pPort->SharedKey[BSS0][KeyIdx].KeyLen;
            }

        }
        
        if (ADHOC_ON(pPort) && (AlgorithmId == DOT11_CIPHER_ALGO_CCMP) && (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
        {
            if (bPairwise)
            {
                pEntry = MacTableLookup(pAd,  pPort, MacAddr);
                if (pEntry == NULL)
                {
                    pEntry = MacTableInsertEntry(pAd, pPort, MacAddr, FALSE);
                    if (pEntry == NULL)
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("Should not happen, MacTableInsertEntry return NULL means out of MacTab Size\n"));
                        return NDIS_STATUS_FAILURE;
                    }                   
                    else
                    {
                        //
                        // MacTableInsertEntry here means MlmeSyncPeerBeacon doesn't getting call before add key.
                        // In this case we need to update pEntry->Sst state, otherwise In MlmeSyncPeerBeacon will never update this state
                        // since Peer MAC has been interted.
                        //
                        pEntry->Sst = SST_ASSOC;
                    }
                    AsicAddKeyEntry(pAd, pPort,pEntry->Aid, BssIndex, (UCHAR)KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], TRUE, TRUE);  
                }
                else
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("entry not NULL  , force add key\n"));    
                    AsicAddKeyEntry(pAd,pPort, pEntry->Aid, BssIndex , (UCHAR)KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], TRUE, TRUE); 
                }   
                NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
                pAdhocEntry = (PRTMP_ADHOC_LIST)pAd->StaCfg.ActiveAdhocListQueue.Head;
                while (pAdhocEntry)
                {
                    if (MAC_ADDR_EQUAL(MacAddr, pAdhocEntry->PeerMacAddress))
                    {
                        pAdhocEntry->bKeySet = TRUE;
                        break;
                    }
                    pAdhocEntry = (PRTMP_ADHOC_LIST)pAdhocEntry->Next;
                }
                NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);
            }
            else
            {
                UCHAR WlanIdx = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_ADHOC_MCAST);
                //
                // Use WCID(254) to Entrypt bradcast frame, so that we don't need to implement Software AES entrypt.
                //
                DBGPRINT(RT_DEBUG_ERROR, ("Add broadcast key\n"));          
                AsicAddKeyEntry(pAd,pPort, WlanIdx, BssIndex, (UCHAR)KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], TRUE, TRUE);               
            }
        }
        else
        {
            if ((bPairwise) && (MFP_ON(pAd, pPort)) &&
                (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled || pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled))
            {       
                ; // Do nothing
            }
            else                
            {
                if (!bPairwise)
                {   
                    Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s   group key wcid = %d \n", __FUNCTION__, Wcid));
                }
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s   pairwise key wcid = %d \n", __FUNCTION__, Wcid));
				}
                AsicAddKeyEntry(pAd, pPort, Wcid, BssIndex, (UCHAR)KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], bPairwise, bTxKey);
        }
        }

        if (!bPairwise)
        {
            
            if (MlmeSyncGetRoleTypeByWlanIdx(pPort, Wcid) != ROLE_WLANIDX_BSSID) // for concurrent client
            {
                pWcidMacTabEntry =  MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);
                if(pWcidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
                    return NDIS_STATUS_INVALID_DATA;
                }
                
                pWcidMacTabEntry->PortSecured = WPA_802_1X_PORT_SECURED;
            }
            else // for port0 and bssid_wcid
                pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
        }

        if (KeyLen >= 16)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddKey::KeyID=%d, Algo=%d, KeyLen=%d, Key=[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x]\n",
                            KeyIdx, AlgorithmId, KeyLen, KeyValue[0], KeyValue[1], KeyValue[2], KeyValue[3], KeyValue[4], KeyValue[5], KeyValue[6],
                            KeyValue[7], KeyValue[8], KeyValue[9], KeyValue[10], KeyValue[11], KeyValue[12], KeyValue[13], KeyValue[14], KeyValue[15]));
        }
        
        DBGPRINT(RT_DEBUG_ERROR, ("pPort->SharedKey[BSS0][%d].CipherAlg = %d. DefaultKeyId = %d. \n", KeyIdx, pPort->SharedKey[BSS0][KeyIdx].CipherAlg, pPort->PortCfg.DefaultKeyId));      
    }
    else
    {
        pPort->SharedKey[BSS0][KeyIdx].KeyLen = 0;
        AsicRemoveSharedKeyEntry(pAd, BSS0, KeyIdx); //remove HW key for Win8 NDIStest AddRemoveKey_cmn
    }

    return NDIS_STATUS_SUCCESS;
}

BOOLEAN
MlmeInfoAPFindKeyMappingKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr
    )
{
    MAC_TABLE_ENTRY *pEntry;
    pEntry = MacTableLookup(pAd,  pPort, MacAddr);

    if(pEntry != NULL)
    {
        
        if(pEntry->PairwiseKey.KeyLen != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN
MlmeInfoFindKeyMappingKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr
    )
{
    BOOLEAN             bResult = FALSE;
    INT                 KeyIdx;
    PCIPHER_KEY         pCihperKey = NULL;
    PRTMP_ADHOC_LIST    pAdhocEntry;
    
    if (ADHOC_ON(pPort) && (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
    {
        NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
        pAdhocEntry = (PRTMP_ADHOC_LIST)pAd->StaCfg.ActiveAdhocListQueue.Head;
        while (pAdhocEntry)
        {
            if (MAC_ADDR_EQUAL(MacAddr, pAdhocEntry->PeerMacAddress) && (pAdhocEntry->bKeySet))
            {
                bResult = TRUE;
                break;
            }
            pAdhocEntry = (PRTMP_ADHOC_LIST)pAdhocEntry->Next;
        }
        NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);
    }
    else
    {
        for(KeyIdx = 0; KeyIdx < 4; KeyIdx++)
        {
            if ( MAC_ADDR_EQUAL( pPort->SharedKey[BSS0][KeyIdx].BssId,MacAddr) &&
                !MAC_ADDR_EQUAL(pPort->SharedKey[BSS0][KeyIdx].BssId,ZERO_MAC_ADDR))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("find key mapping key addr2 = %2x:%2x:%2x:%2x:%2x:%2x",MacAddr[0],
                                        MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],
                                        MacAddr[5]));   
                pCihperKey = &pPort->SharedKey[BSS0][KeyIdx];
                if (pCihperKey->KeyLen !=0)
                {
                    bResult = TRUE;
                    break;
                }
            }
        }
    }
    return bResult;
}

PCIPHER_KEY
MlmeInfoFindStaDefaultKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx
    )
{
    PPER_STA_CIPHER_KEY     pPerStaCipherKey;
    PCIPHER_KEY             pCipherKey;
    ULONG                   index;
    ULONG                   Count = pPort->PerStaKeyCount;

    if (KeyIdx >= 4)
    {
        DBGPRINT(RT_DEBUG_ERROR, (" StaDefaultKey error KeyIdx = %d\n",KeyIdx));
        return NULL;
    }

    if (Count == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, (" StaDefaultKey count 0\n"));
        return NULL;
    }

    for (index = 0; index < PER_STA_KEY_TABLE_SIZE; index++)
    {
        pPerStaCipherKey = &pPort->PerStaKey[index];
        if (pPerStaCipherKey->Valid)
        {
            if (MAC_ADDR_EQUAL(pPerStaCipherKey->StaMacAddr, MacAddr))
            {
                pCipherKey = &pPerStaCipherKey->CipherKey[KeyIdx];
                DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoFindStaDefaultKey::KeyID=%d,Key=[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x]\n",
                            KeyIdx,pCipherKey->Key[0], pCipherKey->Key[1],pCipherKey->Key[2], pCipherKey->Key[3], pCipherKey->Key[4], pCipherKey->Key[5], pCipherKey->Key[6],
                            pCipherKey->Key[7], pCipherKey->Key[8], pCipherKey->Key[9], pCipherKey->Key[10], pCipherKey->Key[11], pCipherKey->Key[12], pCipherKey->Key[13], pCipherKey->Key[14], pCipherKey->Key[15]));
                return (pCipherKey->KeyLen != 0) ? pPerStaCipherKey->CipherKey : NULL;
            }

            Count--;
            if (Count == 0)
            {
                DBGPRINT(RT_DEBUG_ERROR, (" can't find StaDefaultKey \n"));
                return NULL;
            }
        }
    }

    DBGPRINT(RT_DEBUG_ERROR, (" can't find valid StaDefaultKey entry \n"));
    //
    // Should not reach here.
    //
    return NULL;
}

NDIS_STATUS
MlmeInfoAddPerStaDefaultKey(    
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx,
    IN  DOT11_CIPHER_ALGORITHM AlgorithmId,
    IN  ULONG           KeyLen,
    IN  PUCHAR          KeyValue
    )
{
    PPER_STA_CIPHER_KEY         pPerStaCipherKey;
    ULONG                       index, emptyIndex;
    PDOT11_KEY_ALGO_CCMP        CCMPKey = NULL;
    PDOT11_KEY_ALGO_TKIP_MIC    TKIPKey = NULL;
    UCHAR                       CipherAlg = CIPHER_NONE;
        
    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey:: StaMAC[%02x:%02x:%02x:%02x:%02x:%02x] KeyID=%d, algoId=%d\n",
        MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5], KeyIdx, AlgorithmId));

    if (pPort->PortCfg.AuthMode != Ralink802_11AuthModeWPA2PSK)
        return NDIS_STATUS_NOT_SUPPORTED;

    //
    // Search the per-STA key table to find either a matching MacAddr or an empty key entry.
    //
    emptyIndex = PER_STA_KEY_TABLE_SIZE;
    pPerStaCipherKey = pPort->PerStaKey;
    for (index = 0; index < PER_STA_KEY_TABLE_SIZE; index++, pPerStaCipherKey++)
    {
        if (pPerStaCipherKey->Valid && MAC_ADDR_EQUAL(pPerStaCipherKey->StaMacAddr, MacAddr))
            break;

        if (!pPerStaCipherKey->Valid && (emptyIndex == PER_STA_KEY_TABLE_SIZE))
            emptyIndex = index;
    }

    if (index == PER_STA_KEY_TABLE_SIZE)
    {
        //
        // We did not find a per-STA key with matching MacAddr. 
        //
        if (KeyLen == 0) 
        {
            //
            // If we are asked to delete a per-STA key, fail the request.
            //
            return NDIS_STATUS_INVALID_DATA;
        }
        else if (emptyIndex == PER_STA_KEY_TABLE_SIZE)
        {
            //
            // If we are asked to add a key but the table is full, fail the request.
            //
            return NDIS_STATUS_RESOURCES;
        }

        pPerStaCipherKey = &pPort->PerStaKey[emptyIndex];
        COPY_MAC_ADDR(pPerStaCipherKey->StaMacAddr, MacAddr);
    }

    if (KeyLen != 0)
    {
        switch (AlgorithmId)
        {
            case DOT11_CIPHER_ALGO_WEP:
                if (KeyLen == 5)
                {
                    CipherAlg = CIPHER_WEP64;
                }
                else if (KeyLen == 13)
                {
                    CipherAlg = CIPHER_WEP128;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey Invalid wep key\n"));
                    return NDIS_STATUS_INVALID_DATA;
                }
                break;              
            case DOT11_CIPHER_ALGO_WEP40:
                CipherAlg = CIPHER_WEP64;

                if (KeyLen != 5)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey, Invalild DOT11_CIPHER_ALGO_WEP40 KeyLen=%d\n", KeyLen));
                    return NDIS_STATUS_INVALID_DATA;
                }
                break;
                
            case DOT11_CIPHER_ALGO_WEP104:
                CipherAlg = CIPHER_WEP128;

                if (KeyLen != 13)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey, Invalild DOT11_CIPHER_ALGO_WEP104 KeyLen=%d\n", KeyLen));
                    return NDIS_STATUS_INVALID_DATA;
                }
                break;
                
            case DOT11_CIPHER_ALGO_CCMP:
                CipherAlg = CIPHER_AES;
                CCMPKey = (PDOT11_KEY_ALGO_CCMP)KeyValue;
                if (CCMPKey->ulCCMPKeyLength != 16)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey, Invalid AES keylen=%d\n", CCMPKey->ulCCMPKeyLength));
                    return NDIS_STATUS_INVALID_DATA;
                }
                KeyValue = CCMPKey->ucCCMPKey;
                KeyLen = CCMPKey->ulCCMPKeyLength;
                break;
                
            case DOT11_CIPHER_ALGO_TKIP:
                CipherAlg = CIPHER_TKIP;
                TKIPKey = (PDOT11_KEY_ALGO_TKIP_MIC)KeyValue;
                if (TKIPKey->ulTKIPKeyLength != 16 || TKIPKey->ulMICKeyLength != 16)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey:: Invalid TKIP key len=%d, MIC Keylen=%d\n", 
                                    TKIPKey->ulTKIPKeyLength, TKIPKey->ulMICKeyLength));
                    return NDIS_STATUS_INVALID_DATA;
                }
                KeyValue = TKIPKey->ucTKIPMICKeys;
                KeyLen = TKIPKey->ulTKIPKeyLength;
                break;

            default:
                DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey:: Unknow AlgorithmId = 0x%x\n", AlgorithmId));
                break;
        }

        DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoAddPerStaDefaultKey::KeyID=%d, Algo=%d, KeyLen=%d, Key=[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x]\n",
                            KeyIdx, AlgorithmId, KeyLen, KeyValue[0], KeyValue[1], KeyValue[2], KeyValue[3], KeyValue[4], KeyValue[5], KeyValue[6],
                            KeyValue[7], KeyValue[8], KeyValue[9], KeyValue[10], KeyValue[11], KeyValue[12], KeyValue[13], KeyValue[14], KeyValue[15]));
        
        pPerStaCipherKey->CipherKey[KeyIdx].KeyLen = (UCHAR) KeyLen;
        PlatformMoveMemory(pPerStaCipherKey->CipherKey[KeyIdx].Key, KeyValue, min(KeyLen, 16));
        if (CipherAlg == CIPHER_TKIP)
        {
            PlatformMoveMemory(pPerStaCipherKey->CipherKey[KeyIdx].RxMic, Add2Ptr(KeyValue,KeyLen), 8);
            PlatformMoveMemory(pPerStaCipherKey->CipherKey[KeyIdx].TxMic, Add2Ptr(KeyValue,KeyLen + 8), 8);
        }
        pPerStaCipherKey->CipherKey[KeyIdx].CipherAlg = CipherAlg;
        // Init TxTsc to one based on WiFi WPA specs
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[0] = 1;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[1] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[2] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[3] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[4] = 0;
        pPort->SharedKey[BSS0][KeyIdx].TxTsc[5] = 0;
        
        pPerStaCipherKey->Valid = TRUE;
        pPort->PerStaKeyCount++;
    }
    else
    {
        PlatformZeroMemory(pPerStaCipherKey, sizeof(PER_STA_CIPHER_KEY));
        pPort->PerStaKeyCount--;
    }

    return NDIS_STATUS_SUCCESS; 
}

/*
    ========================================================================
    
    Routine Description:
        Remove WPA Key process

    Arguments:
        pAd                     Pointer to our adapter
        pBuf                            Pointer to the where the key stored

    Return Value:
        NDIS_SUCCESS                    Add key successfully

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
NDIS_STATUS 
MlmeInfoWPARemoveKeyProc(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PVOID           pBuf
    )
{
    PNDIS_802_11_REMOVE_KEY pKey;
    ULONG                   KeyIdx;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    BOOLEAN                 bTxKey;         // Set the key as transmit key
    BOOLEAN                 bPairwise;      // Indicate the key is pairwise key
    BOOLEAN                 bKeyRSC;        // indicate the receive  SC set by KeyRSC value.
                                // Otherwise, it will set by the NIC.
    BOOLEAN                 bAuthenticator; // indicate key is set by authenticator.
    INT                     i;


    DBGPRINT(RT_DEBUG_TRACE,("---> MlmeInfoWPARemoveKeyProc\n"));
    
    pKey = (PNDIS_802_11_REMOVE_KEY) pBuf;
    KeyIdx = pKey->KeyIndex & 0xff;
    // Bit 31 of Add-key, Tx Key
    bTxKey         = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
    // Bit 30 of Add-key PairwiseKey
    bPairwise      = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;
    // Bit 29 of Add-key KeyRSC
    bKeyRSC        = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
    // Bit 28 of Add-key Authenticator
    bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;

    // 1. If bTx is TRUE, return failure information
    if (bTxKey == TRUE)
        return(NDIS_STATUS_INVALID_DATA);

    // 2. Check Pairwise Key
    if (bPairwise)
    {
        // a. If BSSID is broadcast, remove all pairwise keys.
        // b. If not broadcast, remove the pairwise specified by BSSID
        for (i = 0; i < SHARE_KEY_NUM; i++)
        {
            if (MAC_ADDR_EQUAL(pPort->SharedKey[BSS0][i].BssId, pKey->BSSID))
            {
                DBGPRINT(RT_DEBUG_TRACE,("MlmeInfoWPARemoveKeyProc(KeyIdx=%d)\n", i));
                pPort->SharedKey[BSS0][i].KeyLen = 0;
                pPort->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
                AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)i);
                Status = NDIS_STATUS_SUCCESS;
                break;
            }
        }
    }
    // 3. Group Key
    else
    {
        // a. If BSSID is broadcast, remove all group keys indexed
        // b. If BSSID matched, delete the group key indexed.
        DBGPRINT(RT_DEBUG_TRACE,("MlmeInfoWPARemoveKeyProc(KeyIdx=%d)\n", KeyIdx));
        pPort->SharedKey[BSS0][KeyIdx].KeyLen = 0;
        pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
        AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)KeyIdx);
        Status = NDIS_STATUS_SUCCESS;
    }
    return (Status);
}


/*
    ========================================================================
    
    Routine Description:
        Remove All WPA Keys

    Arguments:
        pAd                     Pointer to our adapter

    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:

    ========================================================================
*/
VOID    
MlmeInfoWPARemoveAllKeys(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    )
{
    PMP_PORT pStaPort = pAd->PortList[PORT_0];
    INT     i;
    UCHAR   BssIndex = BSS0;
    USHORT   Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);

    DBGPRINT(RT_DEBUG_TRACE,("MlmeInfoWPARemoveAllKeys(AuthMode=%s, WepStatus=%s)\n"
        , decodeAuthAlgorithm(pPort->PortCfg.AuthMode), decodeCipherAlgorithm(pPort->PortCfg.WepStatus)));
    if (pAd->NumberOfPorts > 1)
    {
        BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
    }

#ifdef MULTI_CHANNEL_SUPPORT
    // Cover for WFD_Concurrent_ext.htm, Test Tool triggers connection but does not trigger 4 way, driver might fail to handle connection status
    // This make driver fail to handle disconnection.
    if (INFRA_ON(pStaPort) && (pPort->PortCfg.AuthMode == 7) && (pPort->PortCfg.WepStatus == 4) && (pPort->PortType == WFD_CLIENT_PORT))
    {
        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) == FALSE)
        {
            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_OUT_NEW_CONNECTION_START);
            // win8 OS might take 350ms for DHCP exchange, keep for 400ms for this.
            PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, 10/*1300*/);    

            // Set connected status as client operation
            MT_SET_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP);
            DBGPRINT(RT_DEBUG_TRACE,("Cover for WFD_Concurrent_ext.htm CLI connection issue\n"));
        }
    }
#endif /*MULTI_CHANNEL_SUPPORT*/ 

    // For WEP/CKIP, there is no need to remove it, since WinXP won't set it again after
    // Link up. And it will be replaced if user changed it.
    //if (pPort->CommonCfg.AuthMode < Ralink802_11AuthModeWPA)
    //  return;

    // For WPA-None, there is no need to remove it, since WinXP won't set it again after
    // Link up. And it will be replaced if user changed it.
    //if (pPort->CommonCfg.AuthMode == Ralink802_11AuthModeWPANone)
    //  return;

    for (i = 0; i < SHARE_KEY_NUM; i++)
    {
        DBGPRINT(RT_DEBUG_TRACE,("MlmeInfoWPARemoveAllKeys : remove %s key #%d\n", DecodeCipherName(pPort->SharedKey[BSS0][i].CipherAlg), i));
        pPort->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
        pPort->SharedKey[BSS0][i].KeyLen = 0;
        AsicRemoveSharedKeyEntry(pAd, BssIndex, (UCHAR)i);  
    }

    if(pPort->PortNumber == PORT_0) //skip key remove if port is not for STA
    {
        //Remove IGTK
        pAd->StaCfg.PmfCfg.IGTKLen = 0;
        for (i = 0; i < 2; i++)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoWPARemoveAllKeys : Rmove Integrity Group Key (KeyIdx=%d)\n", i+4));
            PlatformZeroMemory(pAd->StaCfg.PmfCfg.IPN[i], 6);
            PlatformZeroMemory(pAd->StaCfg.PmfCfg.IGTK[i], 16);
        }
    }

}

/*
    ========================================================================
    
    Routine Description:
        Construct and indicate WPA2 Media Specific Status

    Arguments:
        pAd Pointer to our adapter

    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    
MlmeInfoIndicateWPA2Status(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    )
{
    struct
    {
        NDIS_802_11_STATUS_TYPE             Status;
        NDIS_802_11_PMKID_CANDIDATE_LIST    List;
    }   Candidate;

    Candidate.Status = Ndis802_11StatusType_PMKID_CandidateList;
    Candidate.List.Version = 1;
    // This might need to be fixed to accomadate with current saved PKMIDs
    Candidate.List.NumCandidates = 1;
    PlatformMoveMemory(&Candidate.List.CandidateList[0].BSSID, pPort->PortCfg.Bssid, 6);
    Candidate.List.CandidateList[0].Flags = 0;
#if 0   
    NdisMIndicateStatus(pAd->AdapterHandle, NDIS_STATUS_MEDIA_SPECIFIC_INDICATION, &Candidate, sizeof(Candidate));  
#endif
    DBGPRINT(RT_DEBUG_TRACE,("MlmeInfoIndicateWPA2Status\n"));
}

/*
    ========================================================================
    Routine Description:
        Change NIC PHY mode. Re-association may be necessary. possible settings
        include - PHY_11B, PHY_11BG_MIXED, PHY_11A, and PHY_11ABG_MIXED 

        [Argument#3]
        We will let phymode to 0xff for rebuild channel list. But it may impack on our logic checking.
        like: pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED to do some 11n feature.
        So, we should never set phymode to 0xff and use bForceUpdaetChList(Argument#3) instead.     

    Arguments:
        pAd - Pointer to our adapter
        phymode  - 
        bForceUpdaetChList - will upate channel list. Omit phymode change check     

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    ========================================================================
*/
VOID    
MlmeInfoSetPhyMode(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT        pPort,
    IN  ULONG           phymode,
    IN  BOOLEAN         bForceUpdaetChList
    )
{
    INT                 i;
    OID_SET_HT_PHYMODE  HTPhymode;

    DBGPRINT(RT_DEBUG_TRACE,("%s ===> %d",__FUNCTION__, phymode));

    phymode&= 0xff;
    
    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s: pPort can't be NULL",__FUNCTION__));
        return;
    }
    
    //
    // check the validity of VHT
    // 
    if((pPort->CommonCfg.DesiredVhtPhy.bVhtEnable== FALSE) && (phymode == PHY_11VHT))
    {
        DBGPRINT(RT_DEBUG_WARN,("PHY_11VHT is not supported on this NIC, downgrade to PHY_11ABGN_MIXED.\n"));
        phymode = PHY_11ABGN_MIXED;
    }
    
    if (phymode >PHY_11MAX)
    {
        DBGPRINT(RT_DEBUG_TRACE,("MlmeInfoSetPhyMode Phymode(=%s). return; \n",  DecodePhyMode(phymode)));
        phymode = PHY_11BGN_MIXED;
    }
    //if TKIP, change to legacy mode.
    if( ((pPort->PortCfg.ExcludeUnencrypted) || (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)) &&
        (pPort->CommonCfg.bProhibitTKIPonHT) && 
        ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||(pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)))
    {
        if(phymode >= PHY_11ABGN_MIXED)
        {
            switch(phymode)
            {
                case PHY_11ABGN_MIXED:
                case PHY_11VHT:
                    phymode = PHY_11ABG_MIXED;
                    break;
                case PHY_11N:
                    phymode = PHY_11G;
                    break;
                case PHY_11GN_MIXED:
                    phymode = PHY_11G;
                    break;
                case PHY_11AN_MIXED:
                    phymode = PHY_11A;
                    break;
                case PHY_11BGN_MIXED:
                    phymode = PHY_11BG_MIXED;
                    break;
                case PHY_11AGN_MIXED:
                    phymode = PHY_11G;
                    break;
                default:
                    break;
            }
        }
    }

    // if no change and not bForceUpdaetChList, do nothing
    if ((pPort->CommonCfg.PhyMode == phymode) && (bForceUpdaetChList == FALSE))
        return;
    
    DBGPRINT(RT_DEBUG_TRACE,("MlmeInfoSetPhyMode(=%s)\n", DecodePhyMode(phymode)));
    pPort->CommonCfg.PhyMode = (UCHAR)phymode;

    //map phy mode to adhoc mode
    if (phymode < ADHOC_11ABGN_MIXED)
    {
        if (phymode == PHY_11BG_MIXED) pAd->StaCfg.AdhocMode = ADHOC_11BG_MIXED;
        else if (phymode == PHY_11B) pAd->StaCfg.AdhocMode = ADHOC_11B;
        else if (phymode == PHY_11A) pAd->StaCfg.AdhocMode = ADHOC_11A;
        else if (phymode == PHY_11ABG_MIXED) pAd->StaCfg.AdhocMode = ADHOC_11ABG_MIXED;
        else if (phymode == PHY_11G) pAd->StaCfg.AdhocMode = ADHOC_11G;
    }
    else
    {
        pAd->StaCfg.AdhocMode = pPort->CommonCfg.PhyMode;
    }

    if(pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        pPort->SoftAP.ApCfg.PhyMode = (UCHAR)phymode;
    }
    else
    {
        pPort->SoftAP.ApCfg.PhyMode = pPort->CommonCfg.InitPhyMode;
    }
    DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Set pPort->SoftAP.ApCfg.PhyMode = %s\n",__FUNCTION__,__LINE__,DecodePhyMode(pPort->SoftAP.ApCfg.PhyMode)));

    BuildChannelList(pAd, pPort);

    // sanity check user setting in Registry
    for (i = 0; i < pAd->HwCfg.ChannelListNum; i++)
    {
        if (pPort->Channel == pAd->HwCfg.ChannelList[i].Channel)
            break;
    }
    if (i == pAd->HwCfg.ChannelListNum)
        pPort->Channel = MlmeSyncFirstChannel(pAd);         

    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoSetPhyMode->Channel= %d, PhyMode = %s\n", pPort->Channel, DecodePhyMode(pPort->CommonCfg.PhyMode)));

    PlatformZeroMemory(pPort->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
    PlatformZeroMemory(pPort->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
    PlatformZeroMemory(pPort->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
    switch (pPort->CommonCfg.PhyMode) {
        case PHY_11B:
            pPort->CommonCfg.SupRate[0]  = 0x82;    // 1 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[1]  = 0x84;    // 2 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[2]  = 0x8B;    // 5.5 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[3]  = 0x96;    // 11 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRateLen  = 4;
            pPort->CommonCfg.ExtRateLen  = 0;
            pPort->CommonCfg.DesireRate[0]  = 2;     // 1 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[1]  = 4;     // 2 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[2]  = 11;    // 5.5 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[3]  = 22;    // 11 mbps, in units of 0.5 Mbps
            WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_CCK); // This MODE is only FYI. not use
            break;

        case PHY_11G:
        case PHY_11N:
        case PHY_11BG_MIXED:
        case PHY_11ABG_MIXED:
        case PHY_11ABGN_MIXED:
        case PHY_11BGN_MIXED:
        case PHY_11GN_MIXED:
        case PHY_11VHT:
            pPort->CommonCfg.SupRate[0]  = 0x82;    // 1 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[1]  = 0x84;    // 2 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[2]  = 0x8B;    // 5.5 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[3]  = 0x96;    // 11 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[4]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRate[5]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRate[6]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRateLen  = 8;
            pPort->CommonCfg.ExtRate[0]  = 0x0C;    // 6 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.ExtRate[1]  = 0x18;    // 12 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.ExtRate[2]  = 0x30;    // 24 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.ExtRate[3]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.ExtRateLen  = 4;
            pPort->CommonCfg.DesireRate[0]  = 2;     // 1 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[1]  = 4;     // 2 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[2]  = 11;    // 5.5 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[3]  = 22;    // 11 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[4]  = 12;    // 6 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[5]  = 18;    // 9 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[6]  = 24;    // 12 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[7]  = 36;    // 18 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[8]  = 48;    // 24 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[9]  = 72;    // 36 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[10] = 96;    // 48 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[11] = 108;   // 54 mbps, in units of 0.5 Mbps
            break;

        case PHY_11A:
        case PHY_11AN_MIXED:
        case PHY_11AGN_MIXED:
            pPort->CommonCfg.SupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
            pPort->CommonCfg.SupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.SupRateLen  = 8;
            pPort->CommonCfg.ExtRateLen  = 0;
            pPort->CommonCfg.DesireRate[0]  = 12;    // 6 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[1]  = 18;    // 9 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[2]  = 24;    // 12 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[3]  = 36;    // 18 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[4]  = 48;    // 24 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[5]  = 72;    // 36 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[6]  = 96;    // 48 mbps, in units of 0.5 Mbps
            pPort->CommonCfg.DesireRate[7]  = 108;   // 54 mbps, in units of 0.5 Mbps
            WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_OFDM); // This MODE is only FYI. not use
            break;

        default:
            break;
    }

#if 1
    HTPhymode.PhyMode = pPort->CommonCfg.PhyMode;
    HTPhymode.TransmitNo = (UCHAR)pAd->HwCfg.Antenna.field.TxPath;
    HTPhymode.HtMode = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.HTMODE;
    HTPhymode.ExtOffset = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.EXTCHA;
    HTPhymode.MCS = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.MCS;
    HTPhymode.BW = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.BW;
    HTPhymode.STBC = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.STBC;
    HTPhymode.SHORTGI = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.ShortGI;
    // Don't need to store HTPhymode.VhtCentralChannel here, SetHtVht will decide it.
#else
    // for test hard code : Lens
    HTPhymode.PhyMode = pPort->CommonCfg.PhyMode;
    HTPhymode.TransmitNo = 2;
    HTPhymode.HtMode = HTMODE_MM;
    HTPhymode.ExtOffset = 0;
    HTPhymode.MCS = MCS_15;
    HTPhymode.BW = BW_40;
    HTPhymode.STBC = 0;
    HTPhymode.SHORTGI = 1;
    // Don't need to store HTPhymode.VhtCentralChannel here, SetHtVht will decide it.
#endif
    if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
        SetHtVht(pAd,pPort, &HTPhymode);

    //MlmeUpdateTxRates(pAd, pPort,FALSE);
    //MlmeUpdateHtVhtTxRates(pAd, pPort, 0xff);
    //MtAsicSetSlotTime(pAd, TRUE); //FALSE);

    pPort->CommonCfg.BandState = UNKNOWN_BAND;
//  MgntPktConstructIBSSBeaconFrame(pAd);    // supported rates may change
}

/*
    ========================================================================
    Routine Description:
        switch driver to the operation mode according to  pAd->OpMode

    Arguments:
        pAd - Pointer to our adapter

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    ========================================================================
*/
VOID    
MlmeInfoOPModeSwitching(
    IN  PMP_ADAPTER   pAd
    )
{
    PMP_PORT  pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

    DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Switch to pAd->OpMode = %d\n",__FUNCTION__,__LINE__,pAd->OpMode));

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd))
    {
        // if win8 create a virtual port for WFD Device port, we can't change p2p port number (for exclusive mode)
    }
    else
#endif
    {
    pPort->P2PCfg.PortNumber = PORT_0;
    }

    // if next op mode is OPMODE_AP, do disassoc, linkdown and APStartUp 
    // if next op mode is OPMODE_STA, do APStop, periodic timer will do auto connect
    if (pAd->OpMode == OPMODE_AP)
    {
        // always wake up when switch to Ap mode. 
        if ( OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_DOZE) ||  MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
            AsicForceWakeup(pAd);
        
        if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
        {
            if (INFRA_ON(pPort) && DLS_ON(pAd))
            {
                DlsTearDown(pAd, pPort);
            }
            if (INFRA_ON(pPort) && TDLS_ON(pAd))
            {
                TdlsTearDown(pAd, TRUE);
            }

            MlmeAssocCls3errAction(pAd, pPort->PortCfg.Bssid);
        }
        MlmeCntLinkDown(pPort, FALSE);
        // We have to tell OS we are EXTSTA_PORT but we start our own SoftAP and use STA path
        pAd->PortList[PORT_0]->PortType = EXTSTA_PORT;
        pAd->PortList[PORT_0]->PortSubtype = PORTSUBTYPE_SoftapAP;

        // No P2P function on Port2
        if (PORTV2_P2P_ON(pAd, pAd->PortList[PORT_2]) && (pAd->PortList[PORT_2]->PortType != WFD_DEVICE_PORT))
        {
            pAd->PortList[PORT_2]->PortType = EXTSTA_PORT;
            pAd->PortList[PORT_2]->PortSubtype = PORTSUBTYPE_STA;
        }

        MlmeRestartStateMachine(pAd,pPort);             
    }
    else if (pAd->OpMode == OPMODE_STA)
    {
        // Stop softAP 
        if(pAd->PortList[PORT_0]->PortSubtype == PORTSUBTYPE_SoftapAP)
        {   
            DBGPRINT(RT_DEBUG_TRACE,("Stop Ralink-SoftAP\n"));
            APStop(pAd, pPort);
        }
        else    if (pAd->PortList[PORT_1] != NULL)
        {
            if(pAd->PortList[PORT_1]->PortSubtype == PORTSUBTYPE_VwifiAP)
                // for stopping Vwifi will take care by setting OID_DOT11_RESET_REQUEST
                DBGPRINT(RT_DEBUG_TRACE,("Postpone stopping Microsoft-SoftAP until OID_DOT11_RESET_REQUEST is set\n"));
        }
        pPort->SoftAP.ApCfg.bSoftAPReady = FALSE; // SoftAP => STA
        
        //"if (pAd->OpMode == OPMODE_STA)" is used in a lot of case...
        pAd->PortList[PORT_0]->PortType = EXTSTA_PORT;
        pAd->PortList[PORT_0]->PortSubtype = PORTSUBTYPE_STA;       
        
        // Reset security default value
        pAd->PortList[PORT_0]->PortCfg.WepStatus = Ralink802_11WEPDisabled; 
    }
    else if (pAd->OpMode == OPMODE_APCLIENT)
    {   
        // Stop softAP
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] OpMode = APCLIENT, Stop SoftAP ...\n",__FUNCTION__,__LINE__));
        APStop(pAd, pPort);
        pPort->SoftAP.ApCfg.bSoftAPReady = FALSE; // SoftAP => STA
        
        // No P2P function on Port0
        pAd->PortList[PORT_0]->PortType = EXTSTA_PORT;
        pAd->PortList[PORT_0]->PortSubtype = PORTSUBTYPE_STA;
        
        // No P2P function on Port2
        if (PORTV2_P2P_ON(pAd, pAd->PortList[PORT_2]) && (pAd->PortList[PORT_2]->PortType != WFD_DEVICE_PORT))
        {
            pAd->PortList[PORT_2]->PortType = EXTSTA_PORT;
            pAd->PortList[PORT_2]->PortSubtype = PORTSUBTYPE_STA;
        }   
        
        // Reset security default value
        pAd->PortList[PORT_0]->PortCfg.WepStatus = Ralink802_11WEPDisabled; 

        if ((pAd->PortList[PORT_1] != NULL) && (pAd->PortList[PORT_1]->PortType != WFD_DEVICE_PORT))
        {
            pAd->PortList[PORT_1]->PortType = EXTAP_PORT;
            pAd->PortList[PORT_1]->PortSubtype = PORTSUBTYPE_VwifiAP;
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR, (" Start AP without new VWiFi Port\n"));
        }
    }
    else
    {   
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_SOFTAP_IS_DOWN);
        pPort->SoftAP.ApCfg.bSoftAPReady = FALSE; // SoftAP ==> STA      
    }

    DBGPRINT(RT_DEBUG_TRACE,("Switch to (=%d) mode\n", pAd->OpMode));
}

/*
    ========================================================================
    
    Routine Description:
        As STA's BSSID is a WC too, it uses shared key table.
        This function write correct unicast TX key to ASIC WCID.
        And we still make a copy in our MacTab.Content[BSSID_WCID].PairwiseKey.
        Caller guarantee TKIP/AES always has keyidx = 0. (pairwise key)
        Caller guarantee WEP calls this function when set Txkey,  default key index=0~3.
        
    Arguments:
        pAd                     Pointer to our adapter
        pKey                            Pointer to the where the key stored

    Return Value:
        NDIS_SUCCESS                    Add key successfully

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/

VOID    
MlmeInfoAddBSSIDCipher(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR   Aid,
    IN  PNDIS_802_11_KEY    pKey
    )
{
    PUCHAR      pTxMic, pRxMic;
    BOOLEAN     bKeyRSC, bAuthenticator;        // indicate the receive  SC set by KeyRSC value.
    UCHAR   CipherAlg, i;
    ULONG       WCIDAttri;
    USHORT      offset;
    UCHAR   KeyIdx, IVEIV[8];
    ULONG       Value;
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Aid);       
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoAddBSSIDCipher==> Aid = %d\n",Aid));

    if(pWcidMacTabEntry == NULL || pBssidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Aid));
        return;
    }
    
    // Bit 29 of Add-key KeyRSC
    bKeyRSC        = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
    // Bit 28 of Add-key Authenticator
    bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;
    KeyIdx = (UCHAR)pKey->KeyIndex&0xff;

    if (KeyIdx > 4)
        return;

    if (pWcidMacTabEntry->PairwiseKey.CipherAlg == CIPHER_TKIP)  
    {   if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPANone)
        {
            // for WPA-None Tx, Rx MIC is the same
            pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
            pRxMic = pTxMic;
        }
        else if (bAuthenticator == TRUE)
        {
            pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
            pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
        }
        else
        {
            pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
            pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
        }
        
        offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE) + 0x10;
        for (i = 0; i < 8; )
        {
            Value = *(pTxMic+i);
            Value += (*(pTxMic + i + 1)<<8);
            Value += (*(pTxMic + i + 2)<<16);
            Value += (*(pTxMic + i + 3)<<24);
            RTUSBWriteMACRegister(pAd, offset+i, Value);
            i += 4;
        }
        offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE) + 0x18;
        for (i = 0; i < 8; )
        {
            Value = *(pRxMic+i);
            Value += (*(pRxMic + i + 1)<<8);
            Value += (*(pRxMic + i + 2)<<16);
            Value += (*(pRxMic + i + 3)<<24);
            RTUSBWriteMACRegister(pAd, offset+i, Value);
            i += 4;
        }
        // Only Key lenth equal to TKIP key have these
        PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.RxMic, pRxMic, 8);
        PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.TxMic, pTxMic, 8);
        
        DBGPRINT(RT_DEBUG_TRACE,("  TxMIC  = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3], pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
        DBGPRINT(RT_DEBUG_TRACE,("  RxMic = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3], pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));

    }
    // 2. Record Security Key.
    pBssidMacTabEntry->PairwiseKey.KeyLen= (UCHAR)pKey->KeyLength;
    PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.Key, &pKey->KeyMaterial, pKey->KeyLength);
    // 3. Check RxTsc. And used to init to ASIC IV. 
    if (bKeyRSC == TRUE)
        PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.RxTsc, &pKey->KeyRSC, 6);          
    else
        PlatformZeroMemory(pWcidMacTabEntry->PairwiseKey.RxTsc, 6);     

    // 4. Init TxTsc to one based on WiFi WPA specs
    pWcidMacTabEntry->PairwiseKey.TxTsc[0] = 1;
    pWcidMacTabEntry->PairwiseKey.TxTsc[1] = 0;
    pWcidMacTabEntry->PairwiseKey.TxTsc[2] = 0;
    pWcidMacTabEntry->PairwiseKey.TxTsc[3] = 0;
    pWcidMacTabEntry->PairwiseKey.TxTsc[4] = 0;
    pWcidMacTabEntry->PairwiseKey.TxTsc[5] = 0;
    

    CipherAlg = pWcidMacTabEntry->PairwiseKey.CipherAlg;
    offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE);   
    RTUSBMultiWrite(pAd, (USHORT) offset, pKey->KeyMaterial, (USHORT)pKey->KeyLength);
    
    offset = SHARED_KEY_TABLE_BASE + (KeyIdx * HW_KEY_ENTRY_SIZE);  
    RTUSBMultiWrite(pAd, (USHORT) offset, pKey->KeyMaterial, (USHORT)pKey->KeyLength);
    
    offset = PAIRWISE_IVEIV_TABLE_BASE + (Aid * HW_IVEIV_ENTRY_SIZE);
    PlatformZeroMemory(IVEIV, 8);
    // IV/EIV
    if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
    {
        IVEIV[3] = 0x20;    // Eiv bit on. keyid always 0 for pairwise key

    }
    // default key idx needs to set.   in TKIP/AES KeyIdx = 0 , WEP KeyIdx is default tx key.  
    else
    {
        IVEIV[3] |= (KeyIdx<< 6);   
    }
    RTUSBMultiWrite(pAd, (USHORT) offset, IVEIV, 8);

    // WCID Attribute UDF:3, BSSIdx:3, Alg:3, Keytable:1=PAIRWISE KEY, BSSIdx is 0
    if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
        WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;
    else
        WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;
    
    offset = MAC_WCID_ATTRIBUTE_BASE + (Aid* HW_WCID_ATTRI_SIZE);
    RTUSBWriteMACRegister(pAd, offset, WCIDAttri);
    RTUSBReadMACRegister(pAd, offset, &Value);
    DBGPRINT(RT_DEBUG_TRACE, ("BSSID_WCID : offset = %x, WCIDAttri = %x\n", offset, WCIDAttri));
    
    // pAddr
    // Add Bssid mac address at linkup. not here.  check!
    /*offset = MAC_WCID_BASE + (BSSID_WCID * HW_WCID_ENTRY_SIZE);   
    *for (i=0; i<MAC_ADDR_LEN; i++)
    {
        RTMP_IO_WRITE8(pAd, offset+i, pKey->BSSID[i]);
    }
    */
    
    DBGPRINT(RT_DEBUG_ERROR,("AddBSSIDasWCIDEntry: Alg=%s, KeyLength = %d\n", DecodeCipherName(CipherAlg), pKey->KeyLength));
    DBGPRINT(RT_DEBUG_TRACE,("Key [idx=%x] [KeyLen = %d]\n", pKey->KeyIndex, pKey->KeyLength));
    for (i = 0; i < pKey->KeyLength; i++)
        DBGPRINT_RAW(RT_DEBUG_TRACE,(" %x:", pKey->KeyMaterial[i]));
    DBGPRINT(RT_DEBUG_TRACE,("   \n"));

}

VOID 
MlmeInfoInitCipherAttrib(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pAddr
    )   
{
    MAC_TABLE_ENTRY *pEntry;
    UCHAR           BssIndex = BSS0;
    
       FUNC_ENTER;
        
    if ((pAd->NumberOfPorts > 1) && (pAd->StaCfg.BssType == BSS_INFRA))
    {
        BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
    }
        
    pEntry = MlmeSyncMacTabMatchedBssid(pPort, pAddr);
    if (pEntry)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("pEntry->AuthMode = %s\n",decodeAuthAlgorithm(pEntry->AuthMode)));
        if ((pEntry->AuthMode <= Ralink802_11AuthModeAutoSwitch) && IS_WEP_STATUS_ON(pEntry->WepStatus))
        {
            ULONG   uIV = 0;
            PUCHAR  ptr;

            ptr = (PUCHAR) &uIV;
            *(ptr + 3) = (pPort->PortCfg.DefaultKeyId << 6);
            AsicUpdateWCIDIVEIV(pAd, pEntry->wcid, uIV, 0);
            AsicUpdateWCIDAttribute(pAd, pPort,pEntry->wcid , BssIndex, pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg, FALSE);
            
            DBGPRINT(RT_DEBUG_TRACE, ("Update wep cipher\n"));
        }
        else if (pEntry->AuthMode == Ralink802_11AuthModeWPANone)
        {
            ULONG   uIV = 0;
            PUCHAR  ptr;

            ptr = (PUCHAR) &uIV;
            *(ptr + 3) = ((pPort->PortCfg.DefaultKeyId << 6) | 0x20);
            AsicUpdateWCIDIVEIV(pAd, pEntry->wcid, uIV, 0);
            AsicUpdateWCIDAttribute(pAd,pPort,pEntry->wcid, BssIndex, pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg, FALSE);
        }
        else
        {
            //
            // Other case, disable engine.
            // Don't worry WPA key, we will add WPA Key after 4-Way handshaking.
            //
            AsicRemoveKeyEntry(pAd, pEntry->wcid, 0, TRUE);
            DBGPRINT(RT_DEBUG_TRACE, ("Remove for wpa\n"));
        }
        MtAsicUpdateRxWCIDTable(pAd, pEntry->wcid , pEntry->Addr);
    }
    else
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] NO entry been found in the software-based MAC table.\n",__FUNCTION__,__LINE__));     


    FUNC_LEAVE;
}


/*
    ========================================================================
    Routine Description:
        Update HT IE from our capability.
        
    Arguments:
        Send all HT IE in beacon/probe rsp/assoc rsp/action frame.
        
    
    ========================================================================
*/
VOID    
MlmeInfoUpdateHtIe(
    IN  RT_HT_CAPABILITY    *pRtHt,
    OUT HT_CAPABILITY_IE    *pHtCapability,
    OUT ADD_HT_INFO_IE      *pAddHtInfo
    )
{

    DBGPRINT(RT_DEBUG_TRACE,("%s ==> \n",__FUNCTION__));
    // =====================================
    // Update for HT (11n)
    // =====================================

    PlatformZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
    PlatformZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
    
    pHtCapability->HtCapInfo.ChannelWidth = pRtHt->HtChannelWidth;
    pHtCapability->HtCapInfo.MimoPs = pRtHt->MimoPs;
    pHtCapability->HtCapInfo.GF = pRtHt->GF;
    pHtCapability->HtCapInfo.ShortGIfor20 = pRtHt->ShortGIfor20;
    pHtCapability->HtCapInfo.ShortGIfor40 = pRtHt->ShortGIfor40;
    pHtCapability->HtCapInfo.TxSTBC = pRtHt->TxSTBC;
    pHtCapability->HtCapInfo.RxSTBC = pRtHt->RxSTBC;
    pHtCapability->HtCapParm.MaxRAmpduFactor = pRtHt->MaxRAmpduFactor;
    pHtCapability->HtCapParm.MpduDensity = pRtHt->MpduDensity;
    
    pAddHtInfo->AddHtInfo.ExtChanOffset = pRtHt->ExtChanOffset ;
    pAddHtInfo->AddHtInfo.RecomWidth = pRtHt->RecomWidth;
    pAddHtInfo->AddHtInfo2.OperaionMode = pRtHt->OperaionMode;
    pAddHtInfo->AddHtInfo2.NonGfPresent = pRtHt->NonGfPresent;

    PlatformMoveMemory(pAddHtInfo->MCSSet, pRtHt->MCSSet, 4); // rt2860 only support MCS max=32, no need to copy all 16 uchar.

    DBGPRINT(RT_DEBUG_TRACE,("%s <== \n", __FUNCTION__));
}

VOID    
MlmeInfoUpdateVhtIe(
    IN    RT_VHT_CAPABILITY *pRtVht,
    OUT   VHT_CAP_IE          *pVhtCapaIe,
    OUT   VHT_OP_IE           *pVhtOperIe
    )
{
    DBGPRINT(RT_DEBUG_TRACE,("%s ==> \n",__FUNCTION__));
    
    pRtVht->bVhtEnable = TRUE;
    // =====================================
    // Update for VHT (11ac)
    // =====================================
    PlatformZeroMemory(pVhtCapaIe, sizeof(VHT_CAP_IE));
    PlatformZeroMemory(pVhtOperIe, sizeof(VHT_OP_IE));

    // ------------------------------
    // IE: VHT Capabilities Element
    // ------------------------------

    // Vht Capabilities info
    pVhtCapaIe->VhtCapInfo.MaxMpduLength            = pRtVht->VhtCapability.VhtCapInfo.MaxMpduLength;
    pVhtCapaIe->VhtCapInfo.SupportedChannelWidthSet = pRtVht->VhtCapability.VhtCapInfo.SupportedChannelWidthSet;
    pVhtCapaIe->VhtCapInfo.RxLdpc                   = pRtVht->VhtCapability.VhtCapInfo.RxLdpc;
    pVhtCapaIe->VhtCapInfo.ShortGIfor80Mhz          = pRtVht->VhtCapability.VhtCapInfo.ShortGIfor80Mhz;
    pVhtCapaIe->VhtCapInfo.ShortGIfor160Mhz         = pRtVht->VhtCapability.VhtCapInfo.ShortGIfor160Mhz;
    pVhtCapaIe->VhtCapInfo.TxStbc                   = pRtVht->VhtCapability.VhtCapInfo.TxStbc;
    pVhtCapaIe->VhtCapInfo.RxStbc                   = pRtVht->VhtCapability.VhtCapInfo.RxStbc;
    pVhtCapaIe->VhtCapInfo.SuBeamformerCapable      = pRtVht->VhtCapability.VhtCapInfo.SuBeamformerCapable;
    pVhtCapaIe->VhtCapInfo.SuBeamformeeCapable      = pRtVht->VhtCapability.VhtCapInfo.SuBeamformeeCapable;
    pVhtCapaIe->VhtCapInfo.CompressedSteeringNumber = pRtVht->VhtCapability.VhtCapInfo.SuBeamformeeCapable;
    pVhtCapaIe->VhtCapInfo.SoundingDimensionsNum    = pRtVht->VhtCapability.VhtCapInfo.SoundingDimensionsNum;
    pVhtCapaIe->VhtCapInfo.MuBeamformerCapable      = pRtVht->VhtCapability.VhtCapInfo.MuBeamformerCapable;
    pVhtCapaIe->VhtCapInfo.MuBeamformeeCapable      = pRtVht->VhtCapability.VhtCapInfo.MuBeamformeeCapable;
    pVhtCapaIe->VhtCapInfo.VhtTxopPs                = pRtVht->VhtCapability.VhtCapInfo.VhtTxopPs;
    pVhtCapaIe->VhtCapInfo.HtcVhtCapable            = pRtVht->VhtCapability.VhtCapInfo.HtcVhtCapable;
    pVhtCapaIe->VhtCapInfo.MaxAmpduLenExp           = pRtVht->VhtCapability.VhtCapInfo.MaxAmpduLenExp;
    pVhtCapaIe->VhtCapInfo.VhtLinkAdaptationCapble  = pRtVht->VhtCapability.VhtCapInfo.VhtLinkAdaptationCapble;
    pVhtCapaIe->VhtCapInfo.RxAntPatternConsistency  = pRtVht->VhtCapability.VhtCapInfo.RxAntPatternConsistency;
    pVhtCapaIe->VhtCapInfo.TxAntPatternConsistency  = pRtVht->VhtCapability.VhtCapInfo.TxAntPatternConsistency;

    // Vht supported MCS set
    PlatformMoveMemory(&pVhtCapaIe->VhtSupportMcsSet.RxMcsMap, &pRtVht->VhtCapability.VhtSupportMcsSet.RxMcsMap, sizeof(MCS_MAP));
    pVhtCapaIe->VhtSupportMcsSet.RxHighSupportedDataRate = pRtVht->VhtCapability.VhtSupportMcsSet.RxHighSupportedDataRate;
    PlatformMoveMemory(&pVhtCapaIe->VhtSupportMcsSet.TxMcsMap, &pRtVht->VhtCapability.VhtSupportMcsSet.TxMcsMap, sizeof(MCS_MAP));
    pVhtCapaIe->VhtSupportMcsSet.TxHighSupportedDataRate = pRtVht->VhtCapability.VhtSupportMcsSet.TxHighSupportedDataRate;


    // ------------------------------
    //IE: VHT Operation Element
    // ------------------------------

    // Vht Operation Information
    pVhtOperIe->VhtOpInfo.ChannelWidth         = pRtVht->VhtOperation.VhtOpInfo.ChannelWidth;
    pVhtOperIe->VhtOpInfo.ChannelCenterFreqSeg1= pRtVht->VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1;
    pVhtOperIe->VhtOpInfo.ChannelCenterFreqSeg2= pRtVht->VhtOperation.VhtOpInfo.ChannelCenterFreqSeg2;

    // Basic MCS Set
    PlatformMoveMemory(&pVhtOperIe->VhtBasicMcsSet, &pRtVht->VhtOperation.VhtBasicMcsSet, sizeof(MCS_MAP));


    DBGPRINT(RT_DEBUG_TRACE,("%s <== \n",__FUNCTION__));
}

ULONG
MlmeInfoGetLinkQuality(
    IN  PMP_ADAPTER   pAd,
    IN  LONG  Rssi,
    IN  BOOLEAN bPeriodicIndicate
    )
{
    ULONG LinkQuality = 0;

    //MSFT MSDN: wlanSignalQuality

    if (Rssi >= -50)
        LinkQuality = 100;

    //MSFT MSDN: wlanSignalQuality
    LinkQuality = (Rssi - (-100))*2;

    if (LinkQuality > 100)
        LinkQuality = 100;
    return LinkQuality; //0-100;

}

ULONG
MlmeInfoGetLinkQualityLevel(
    IN  ULONG Quality
    )
{
    ULONG QualityLevel = 0;
    if (Quality != 0)
    {
        QualityLevel = Quality / 20 + 1;
    }
    return QualityLevel;
}

ULONG
MlmeInfoGetPhyIdByChannel(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel
    )
{
    ULONG index;
    PDOT11_SUPPORTED_PHY_TYPES  supportedPhyTypes = NULL;
    UCHAR buffer[PHY_TYPE_BUFFER_SIZE] = {0};
    DOT11_PHY_TYPE PhyType;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    if (pPort->SoftAP.bAPStart == TRUE)
    {
        if ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pPort->CommonCfg.DesiredHtPhy.bHtEnable == TRUE))
            PhyType = dot11_phy_type_ht;
        else
        {
            if  (channel > 14)
            {
                PhyType = dot11_phy_type_ofdm;
            }
            else
            {
                PhyType = dot11_phy_type_erp;
            }
                if ((pPort->CommonCfg.SupRateLen <= 4) && (pPort->CommonCfg.ExtRateLen == 0))
                PhyType = dot11_phy_type_hrdsss;
        }
    }
    else
    {
        if (((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE)) ||
            ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && pAd->LogoTestCfg.OnTestingWHQL))
        {
            PhyType = dot11_phy_type_ht;
        }
        else
        {
            if  (channel > 14)
            {
                PhyType = dot11_phy_type_ofdm;
            }
            else
            {
                PhyType = dot11_phy_type_erp;
            }
            if ((pAd->MlmeAux.SupRateLen <= 4) && (pAd->MlmeAux.ExtRateLen == 0))
                PhyType = dot11_phy_type_hrdsss;
        }
    }
    //
    // Get supported PHY types.
    //
    supportedPhyTypes = (PDOT11_SUPPORTED_PHY_TYPES) buffer;
    supportedPhyTypes->uNumOfEntries = 0;
    Ndis6CommonQuerySupportedPHYTypes(pAd, STA_DESIRED_PHY_MAX_COUNT, supportedPhyTypes);

    //
    // Go through the list to find the matching type
    //
    for (index = 0; index < supportedPhyTypes->uNumOfEntries; index++)
    {
        if (PhyType == supportedPhyTypes->dot11PHYType[index])
        {
            return (index);
        }
    }

    // 
    // No match, return an invalid PhyId value.
    //
    return INVALID_PHY_ID;
}

void
MlmeInfoPnpSetPowerMgmtMode(
    IN PMP_ADAPTER pAd, 
    IN ULONG PowerMode
    )
{
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoPnpSetPowerMgmtMode, PowerMode = %u\n", PowerMode));
    
    // save user's policy here, but not change StaCfg.Psm immediately
    if ((P2P_ON(pPort)) &&
        ((IS_P2P_CON_GO(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) ||
         (IS_P2P_CON_CLI(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]) && 
          INFRA_ON(pAd->PortList[PORT_0]) && INFRA_ON(pAd->PortList[pPort->P2PCfg.PortNumber]))))
    {
        // Always use pwr_active when
        // 1. both port0 and port2 connected
        // 2. concurrent GO
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoPnpSetPowerMgmtMode, use PWR_ACTIVE in STA+P2P mode.\n"));  
        // clear PSM bit immediately
        MlmeSetPsm(pAd, PWR_ACTIVE);

        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM); 
        if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
            pAd->StaCfg.WindowsPowerMode = DOT11_POWER_SAVING_NO_POWER_SAVING;
        pAd->StaCfg.WindowsBatteryPowerMode = DOT11_POWER_SAVING_NO_POWER_SAVING;
    }   
    else if (PowerMode == DOT11_POWER_SAVING_NO_POWER_SAVING) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoPnpSetPowerMgmtMode, DOT11_POWER_SAVING_NO_POWER_SAVING\n"));

        // clear PSM bit immediately
        MlmeSetPsm(pAd, PWR_ACTIVE);

        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM); 
        if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
            pAd->StaCfg.WindowsPowerMode = PowerMode;
        pAd->StaCfg.WindowsBatteryPowerMode = PowerMode;
    } 
    else if (PowerMode == DOT11_POWER_SAVING_MAX_PSP) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoPnpSetPowerMgmtMode, DOT11_POWER_SAVING_MAX_PSP\n"));
        // do NOT turn on PSM bit here, wait until MlmeCheckPsmChange()
        // to exclude certain situations.
        //     MlmeSetPsmBit(pAd, PWR_SAVE);
        if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
            pAd->StaCfg.WindowsPowerMode = PowerMode;
        pAd->StaCfg.WindowsBatteryPowerMode = PowerMode;
        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM); 
        pAd->StaCfg.DefaultListenCount = 5;
    } 
    else if (PowerMode == DOT11_POWER_SAVING_FAST_PSP) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoPnpSetPowerMgmtMode, DOT11_POWER_SAVING_FAST_PSP\n"));
        // do NOT turn on PSM bit here, wait until MlmeCheckPsmChange()
        // to exclude certain situations.
        //     MlmeSetPsmBit(pAd, PWR_SAVE);
        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
        if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
            pAd->StaCfg.WindowsPowerMode = PowerMode;
        pAd->StaCfg.WindowsBatteryPowerMode = PowerMode;
        pAd->StaCfg.DefaultListenCount = 3;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoPnpSetPowerMgmtMode, PWR_ACTIVE\n"));
        // clear PSM bit immediately
        MlmeSetPsm(pAd, PWR_ACTIVE);

        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM); 
        if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
            pAd->StaCfg.WindowsPowerMode = PowerMode;
        pAd->StaCfg.WindowsBatteryPowerMode = PowerMode;
    }
}

VOID 
MlmeInfoMantainAdhcoList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR Addr2
    )
{
    //PRTMP_ADHOC_LIST          pAdhocList;
    MAC_TABLE_ENTRY *pEntry;
    //PCIPHER_KEY pCipheyKey;
    PRTMP_ADHOC_LIST pAdhocEntry;
    BOOLEAN bRemove;
    QUEUE_HEADER            TempQueue;


    DBGPRINT(RT_DEBUG_TRACE, ("RTMPMantainAdhocList active count = %d\n", pAd->StaCfg.ActiveAdhocListQueue.Number));
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPMantainAdhocList free count = %d\n", pAd->StaCfg.FreeAdhocListQueue.Number));

    //init temp queue
    InitializeQueueHeader(&TempQueue);

    //
    // Mantain Adhoc to AdhocList
    //
    NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);

    //copy to temp queue
    TempQueue.Head = pAd->StaCfg.ActiveAdhocListQueue.Head;
    TempQueue.Tail = pAd->StaCfg.ActiveAdhocListQueue.Tail;
    TempQueue.Number = pAd->StaCfg.ActiveAdhocListQueue.Number;
    InitializeQueueHeader(&pAd->StaCfg.ActiveAdhocListQueue);

    pAdhocEntry = (PRTMP_ADHOC_LIST)RemoveHeadQueue(&TempQueue);
    while (pAdhocEntry)
    {       
        bRemove = FALSE;

        if (Addr2)
        {
            if (MAC_ADDR_EQUAL(Addr2, pAdhocEntry->PeerMacAddress))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Deauth received from adhoc entry...\n"));
                bRemove = TRUE;
            }
        }

        //If beacon loss reach 6 seconds, we try to probe inactive station first, if beacon
        //loss more than 10 seconds, we disassociate it.
        if ((pAdhocEntry->ullHostTimeStamp + TEN_SECOND < pAd->Mlme.Now64))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("no beacon...\n"));
            bRemove = TRUE;
        }
        else
        {
            if (pAdhocEntry->ullHostTimeStamp + SIX_SECOND < pAd->Mlme.Now64)
            {
                //probe inactive station
                MlmeSyncProbeInactiveStation(pAd, pPort, pAdhocEntry->PeerMacAddress);
            }
        }

        if (bRemove)
        {
            pEntry = MacTableLookup(pAd,  pPort, pAdhocEntry->PeerMacAddress);
            if (pEntry !=NULL)
            {   
                DBGPRINT(RT_DEBUG_TRACE, ("MlmeInfoMantainAdhcoList indicate disassoc %02x %02x %02x %02x %02x %02x\n",
                pAdhocEntry->PeerMacAddress[0],
                pAdhocEntry->PeerMacAddress[1],
                pAdhocEntry->PeerMacAddress[2],
                pAdhocEntry->PeerMacAddress[3],
                pAdhocEntry->PeerMacAddress[4],
                pAdhocEntry->PeerMacAddress[5]));
                
                //need to indicate disassociation for pre-station
                MacTableDeleteAndResetEntry(pAd, pPort, pEntry->Aid, pAdhocEntry->PeerMacAddress, TRUE);
            }
            PlatformIndicateDisassociation(pAd, pPort,pAdhocEntry->PeerMacAddress, DOT11_DISASSOC_REASON_PEER_UNREACHABLE);         
            InsertTailQueue(&pAd->StaCfg.FreeAdhocListQueue, pAdhocEntry)
        }
        else
        {
            InsertTailQueue(&pAd->StaCfg.ActiveAdhocListQueue, pAdhocEntry);
        }
        pAdhocEntry = (PRTMP_ADHOC_LIST)RemoveHeadQueue(&TempQueue);
    }
    NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);
}


VOID 
MlmeInfoUpdateAdhocList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR Addr2
    )
{
    PRTMP_ADHOC_LIST pAdhocEntry;

    //
    //Update adhoc list time stamp,to prevent n adhoc station leave without send de-auth frame
    //
    NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);    
    pAdhocEntry = (PRTMP_ADHOC_LIST)pAd->StaCfg.ActiveAdhocListQueue.Head;
    while (pAdhocEntry)
    {
        if (MAC_ADDR_EQUAL(&pAdhocEntry->PeerMacAddress, Addr2))
        {
            NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAdhocEntry->ullHostTimeStamp);
            DBGPRINT(RT_DEBUG_TRACE, ("update adhoc time stampe\n"));
            break;
        }
        pAdhocEntry = (PRTMP_ADHOC_LIST)pAdhocEntry->Next;
    }
    NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);

}

VOID MlmeInfoEnablePromiscuousMode(IN PMP_ADAPTER pAd)
{
    //
    // make sure fRTMP_ADAPTER_IDLE_RADIO_OFF is cleared
    //
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        AsicRadioOn(pAd);
    }
    RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, 0x00000002);    // do not pass phy error to upper layer, it will assert        //Enable ASIC promiscuous mode  
    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoEnablePromiscuousMode -->\n"));
}

VOID 
MlmeInfoDisablePromiscuousMode(
    IN PMP_ADAPTER pAd
    )
{
//**
#if 0
    //
    // The RX filter configuration is based upon the NIC is running on the station mode or the SoftAP mode.
    //
    if (pAd->OpMode == OPMODE_AP)
    {
        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, APNORMAL);     // enable RX of DMA block
        DBGPRINT(RT_DEBUG_TRACE, ("%s: RX_FILTR_CFG = APNORMAL\n", __FUNCTION__));
    }
    else if (pAd->OpMode == OPMODE_APCLIENT)
    {
        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, APCLIENTNORMAL);     // enable RX of DMA block
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Set: MTK_OID_N6_SET_OP_MODE = OPMODE_AP, RX_FILTR_CFG = APCLIENTNORMAL\n", __FUNCTION__));
    }
    else if ((pPort->SoftAP.bAPStart == FALSE) && (pPort->P2PCfg.bGOStart == FALSE))   // If SoftAP or AP has started, ignore this!
    {
        if (pPort->CommonCfg.PSPXlink)
        {
            RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, PSPXLINK);
            DBGPRINT(RT_DEBUG_TRACE, ("%s: RX_FILTR_CFG = PSPXLINK\n", __FUNCTION__));
        }
        else
        {
            RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, STANORMAL);     // Staion not drop control frame will fail WiFi Certification.
            DBGPRINT(RT_DEBUG_TRACE, ("%s: RX_FILTR_CFG = STANORMAL\n", __FUNCTION__));
        }
    }
    DBGPRINT(RT_DEBUG_ERROR, ("MlmeInfoDisablePromiscuousMode -->\n"));
#endif  
}