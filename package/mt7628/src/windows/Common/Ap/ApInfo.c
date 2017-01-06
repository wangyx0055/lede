/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_info.c

    Abstract:
    Miniport Query information related subroutines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    08-10-2002    created
    John Chang  01-25-2003    modified as Access Point

*/
#include    "MtConfig.h"

#define ALLOWED_AUTO_CONFIG_FLAGS (DOT11_PHY_AUTO_CONFIG_ENABLED_FLAG | DOT11_MAC_AUTO_CONFIG_ENABLED_FLAG)

// Default Privacy Exemption List
DOT11_PRIVACY_EXEMPTION_LIST
DefaultPrivacyExemptionList =
{
    // Header
    {
        NDIS_OBJECT_TYPE_DEFAULT,                   // Type
        DOT11_PRIVACY_EXEMPTION_LIST_REVISION_1,    // Revision
        sizeof(DOT11_PRIVACY_EXEMPTION_LIST)        // Size
    },

    // uNumOfEntries
    0,

    // uTotalNumOfEntries
    0,

    // PrivacyExemptionEntries
    {0}
};

extern UCHAR    OUI_WPA_WEP40[];
extern UCHAR    OUI_WPA_TKIP[];
extern UCHAR    OUI_WPA_CCMP[];
extern UCHAR    OUI_WPA2_WEP40[];
extern UCHAR    OUI_WPA2_TKIP[];
extern UCHAR    OUI_WPA2_CCMP[];
extern PCHAR    DbgGetOidName( ULONG Oid);

PDOT11_PRIVACY_EXEMPTION_LIST pDefaultPrivacyExemptionList = &DefaultPrivacyExemptionList;

NDIS_STATUS
ApSetDesiredSsidList(
    IN  PMP_ADAPTER           pAd,
    IN  PMP_PORT              pPort,  
    IN  PDOT11_SSID_LIST        SsidList)
{
    NDIS_STATUS             ndisStatus = NDIS_STATUS_SUCCESS;
    BOOLEAN                 bOk = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("ApSetDesiredSsidList=>>\n"));

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    SsidList->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_SSID_LIST_REVISION_1,
                    sizeof(DOT11_SSID_LIST)
                    ))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ApSetDesiredSsidList invalid header\n"));
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }
        
        // Set is allowed only in AP INIT state
        VALIDATE_AP_INIT_STATE(pPort);

        bOk =
            (
                (1 == SsidList->uNumOfEntries) &&
                (1 == SsidList->uTotalNumOfEntries) &&
                (1 <= SsidList->SSIDs[0].uSSIDLength) &&
                (DOT11_SSID_MAX_LENGTH >= SsidList->SSIDs[0].uSSIDLength)
            ) ? TRUE : FALSE;

        if (!bOk)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ApSetDesiredSsidList NDIS_STATUS_INVALID_DATA bOK= false\n"));
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        //
        // Copy SSID
        //
        PlatformMoveMemory(&pPort->PortCfg.DesiredSSID, &SsidList->SSIDs[0], sizeof(DOT11_SSID));
        PlatformMoveMemory(pPort->PortCfg.Ssid, SsidList->SSIDs[0].ucSSID, SsidList->SSIDs[0].uSSIDLength);                 
        pPort->PortCfg.SsidLen = (UCHAR) SsidList->SSIDs[0].uSSIDLength;
        pPort->SoftAP.ApCfg.bSoftAPReady = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, ("AP mode Set::OID_DOT11_DESIRED_SSID_LIST (len=%d,SSID=%s\n",
                    pPort->PortCfg.SsidLen, pPort->PortCfg.Ssid));

    } while (FALSE);

    return ndisStatus;
}

NDIS_STATUS
ApOidSetDesiredPhyList(
    IN  PMP_ADAPTER           pAd,
    IN    NDIS_PORT_NUMBER          PortNumber,    
    IN    PVOID                     InformationBuffer,
    IN    ULONG                     InformationBufferLength,
    OUT   PULONG                    BytesRead,
    OUT   PULONG                    BytesNeeded)
{
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_PHY_ID_LIST  PhyIdList;
    ULONG               requiredListSize = 0;
    PMP_PORT          pPort;
    BOOLEAN             anyPhyId = FALSE;
    ULONG               index;

    do
    {
        *BytesRead = 0;
        *BytesNeeded = 0;

        if (InformationBufferLength < sizeof(DOT11_PHY_ID_LIST))
        {
            *BytesNeeded = sizeof(DOT11_PHY_ID_LIST);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;            
            break;
        }

        PhyIdList = (PDOT11_PHY_ID_LIST)InformationBuffer;

        if (!GetRequiredListSize(
                    FIELD_OFFSET(DOT11_PHY_ID_LIST, dot11PhyId), 
                    sizeof(ULONG), 
                    PhyIdList->uNumOfEntries, 
                    &requiredListSize))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (InformationBufferLength < requiredListSize)
        {
            *BytesNeeded = requiredListSize;
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    PhyIdList->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_PHY_ID_LIST_REVISION_1,
                    sizeof(DOT11_PHY_ID_LIST)
                    ))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        pPort = pAd->PortList[PortNumber];

        //
        // Set is allowed only in AP INIT state
        //
        VALIDATE_AP_INIT_STATE(pPort);

        if (PhyIdList->uNumOfEntries == 0)
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        //
        // only support up to AP_DESIRED_PHY_MAX_COUNT PHY types
        //
        if (PhyIdList->uNumOfEntries > AP_DESIRED_PHY_MAX_COUNT)
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        for (index = 0; index < PhyIdList->uNumOfEntries; index++)
        {
            if (PhyIdList->dot11PhyId[index] == DOT11_PHY_ID_ANY)
            {
                anyPhyId = TRUE;
            }
            else if (PhyIdList->dot11PhyId[index] >= pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries) 
            {
                //
                // Invalid PHY ID
                //
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }       
        }

        //
        // TODO: Program HW with this PHY ID?
        //

        //
        // Copy the desired PHY list.
        //
        pPort->DesiredPhyCount = 1;
        
        if (anyPhyId)
        {
            pPort->DesiredPhyList[0] = DOT11_PHY_ID_ANY;
        }
        else
        {
            //pPort->DesiredPhyList[0] = PhyIdList->dot11PhyId;
        }

        *BytesRead = requiredListSize;

    } while (FALSE);
    

    return ndisStatus;
}

BOOLEAN
ApValidateUnicastAuthCipherPair(
    IN DOT11_AUTH_ALGORITHM AuthAlgo,
    IN DOT11_CIPHER_ALGORITHM   CipherAlgo)
{
    BOOLEAN         ValidPair = FALSE;

    switch (AuthAlgo)
    {
        case DOT11_AUTH_ALGO_80211_OPEN:
            ValidPair = (BOOLEAN)((CipherAlgo == DOT11_CIPHER_ALGO_WEP) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP104) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP40) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_NONE));
            break;
            
        case DOT11_AUTH_ALGO_80211_SHARED_KEY:
            ValidPair = (BOOLEAN)((CipherAlgo == DOT11_CIPHER_ALGO_WEP) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP104) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP40));
            break;

        case DOT11_AUTH_ALGO_WPA:
        case DOT11_AUTH_ALGO_WPA_PSK:
        case DOT11_AUTH_ALGO_RSNA:
        case DOT11_AUTH_ALGO_RSNA_PSK:
            ValidPair = (BOOLEAN)((CipherAlgo == DOT11_CIPHER_ALGO_TKIP) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_CCMP));
            break;

        default:
            break;
    }

    return ValidPair;
}

BOOLEAN
ApValidateMulticastAuthCipherPair(
    IN DOT11_AUTH_ALGORITHM AuthAlgo,
    IN DOT11_CIPHER_ALGORITHM CipherAlgo)
{
    BOOLEAN         ValidPair = FALSE;

    switch (AuthAlgo)
    {
        case DOT11_AUTH_ALGO_80211_OPEN:
            ValidPair = (BOOLEAN)((CipherAlgo == DOT11_CIPHER_ALGO_WEP) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP104) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP40) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_NONE));
            break;

        case DOT11_AUTH_ALGO_80211_SHARED_KEY:
            ValidPair = (BOOLEAN)((CipherAlgo == DOT11_CIPHER_ALGO_WEP) || 
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP104) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP40));
            break;

        case DOT11_AUTH_ALGO_WPA:
        case DOT11_AUTH_ALGO_WPA_PSK:
        case DOT11_AUTH_ALGO_RSNA:
        case DOT11_AUTH_ALGO_RSNA_PSK:
            ValidPair = (BOOLEAN)((CipherAlgo == DOT11_CIPHER_ALGO_TKIP) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_CCMP) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP104) ||
                                    (CipherAlgo == DOT11_CIPHER_ALGO_WEP40));
            break;

        default:
            break;
    }

    return ValidPair;
}

NDIS_STATUS RTMPApAddKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx,
    IN  DOT11_CIPHER_ALGORITHM AlgorithmId,
    IN  ULONG           KeyLen,
    IN  PUCHAR          KeyValue,
    IN  BOOLEAN         bDelete,
    IN  BOOLEAN         bPairwise,
    IN  BOOLEAN         bTxKey)
{
    PDOT11_KEY_ALGO_CCMP        CCMPKey = NULL;
    PDOT11_KEY_ALGO_TKIP_MIC    TKIPKey = NULL;
    UCHAR   CipherAlg = CIPHER_NONE;
    PMAC_TABLE_ENTRY pEntry;
    ULONGLONG TSC=0;
    //UCHAR BssIndex = (pPort->CurrentAddress[5] & 0x01);
    UCHAR BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
    PMAC_TABLE_ENTRY pMacTabEntry;

    ASSERT(KeyIdx < 4);
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

            default:
                DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddMappingKey:: Unknow AlgorithmId = 0x%x\n", AlgorithmId));
                break;
        }


        pPort->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR)KeyLen;
        PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].Key, KeyValue, min(KeyLen, 16));

        if (CipherAlg == CIPHER_TKIP)
        {
            PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].RxMic, Add2Ptr(KeyValue,KeyLen), 8);
            PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].TxMic, Add2Ptr(KeyValue,KeyLen + 8), 8);          
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
        

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("this  key addr = %2x:%2x:%2x:%2x:%2x:%2x\n",MacAddr[0],
                                    MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],
                                            MacAddr[5]));   
        if(PlatformEqualMemory(MacAddr,ZERO_MAC_ADDR, MAC_ADDR_LEN))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("add group key \n"));     

            pEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);
            if (pEntry != NULL)
            {
              AsicAddKeyEntry(pAd, pPort, pEntry->wcid, BssIndex, (UCHAR)KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], FALSE, bTxKey);  
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR,("%s(), Line:%d, Mac Entry is NULL for add group key\n", __FUNCTION__, __LINE__));
            }
        }
        else
        {           
            pEntry = MacTableLookup(pAd,  pPort, MacAddr);
            if(pEntry != NULL)
            {               
                DBGPRINT(RT_DEBUG_ERROR, ("add pairwise key \n"));      

                pMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pEntry->Aid); 

                if(pMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pEntry->Aid));
                    return NDIS_STATUS_INVALID_DATA;
                }
                
                    // 4. Init TxTsc to one based on WiFi WPA specs
                pMacTabEntry->PairwiseKey.TxTsc[0] = 1;
                pMacTabEntry->PairwiseKey.TxTsc[1] = 0;
                pMacTabEntry->PairwiseKey.TxTsc[2] = 0;
                pMacTabEntry->PairwiseKey.TxTsc[3] = 0;
                pMacTabEntry->PairwiseKey.TxTsc[4] = 0;
                pMacTabEntry->PairwiseKey.TxTsc[5] = 0;

                pMacTabEntry->PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][KeyIdx].CipherAlg;
                pMacTabEntry->PairwiseKey.KeyLen = pPort->SharedKey[BSS0][KeyIdx].KeyLen;
                PlatformMoveMemory(pMacTabEntry->PairwiseKey.Key, KeyValue, min(KeyLen, 16));
                if(CipherAlg == CIPHER_TKIP)
                {
                    PlatformMoveMemory(pMacTabEntry->PairwiseKey.RxMic, Add2Ptr(KeyValue,KeyLen), 8);
                    PlatformMoveMemory(pMacTabEntry->PairwiseKey.TxMic, Add2Ptr(KeyValue,KeyLen + 8), 8);
                }
                AsicAddKeyEntry(pAd,pPort, pEntry->Aid, BssIndex, (UCHAR)KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], TRUE, bTxKey);
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR, ("don't have mac entry yet,add pairwise key \n"));     
                pEntry = MacTableInsertEntry(pAd, pPort, MacAddr, TRUE);   
                pEntry->WlanIdxRole = ROLE_WLANIDX_CLIENT;
                
                if (pEntry)
                {
                    AsicAddKeyEntry(pAd, pPort, pEntry->Aid, BssIndex, (UCHAR)KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], TRUE, bTxKey);
                }
                
            }
        }

        //if (bPairwise)
            pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;

        if (KeyLen >= 16)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("RTMPApAddKey::KeyID=%d, Algo=%d, KeyLen=%d, Key=[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x]\n",
                            KeyIdx, AlgorithmId, KeyLen, KeyValue[0], KeyValue[1], KeyValue[2], KeyValue[3], KeyValue[4], KeyValue[5], KeyValue[6],
                            KeyValue[7], KeyValue[8], KeyValue[9], KeyValue[10], KeyValue[11], KeyValue[12], KeyValue[13], KeyValue[14], KeyValue[15]));
        }
        
        DBGPRINT(RT_DEBUG_ERROR, ("pPort->SharedKey[BSS0][%d].CipherAlg = %d. DefaultKeyId = %d. \n", KeyIdx, pPort->SharedKey[BSS0][KeyIdx].CipherAlg, pPort->PortCfg.DefaultKeyId));      
    }
    else
    {
        pPort->SharedKey[BSS0][KeyIdx].KeyLen = 0;  
        pEntry = MacTableLookup(pAd,  pPort, MacAddr);  
        if(pEntry)
        {
            pMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pEntry->Aid);  
            if(pMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pEntry->Aid));
                return NDIS_STATUS_INVALID_DATA;
            }
            
            pMacTabEntry->PairwiseKey.KeyLen  = 0;
            DBGPRINT(RT_DEBUG_ERROR, ("pEntry->Aid = %d. delete key mapping key\n", pEntry->Aid));      
        }       
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS ApOidDisassociatePeerRequest(
    IN  PMP_ADAPTER       pAd,
    IN    NDIS_PORT_NUMBER  PortNumber,    
    IN    PVOID             InformationBuffer,
    IN    ULONG             InformationBufferLength,
    OUT   PULONG            BytesRead,
    OUT   PULONG            BytesNeeded)
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_DISASSOCIATE_PEER_REQUEST DisassociateRequest;
    PMP_PORT  pPort = pAd->PortList[PortNumber];
    PMAC_TABLE_ENTRY pEntry; 
    MLME_DISASSOC_REQ_STRUCT DisassocReq;
    DOT11_DISASSOCIATION_PARAMETERS disassocPara;

    DisassociateRequest =  (PDOT11_DISASSOCIATE_PEER_REQUEST)InformationBuffer;

    do
    {
        if (InformationBufferLength < sizeof(DOT11_DISASSOCIATE_PEER_REQUEST))
        {
            *BytesNeeded =  sizeof(DOT11_DISASSOCIATE_PEER_REQUEST);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (pPort->PortType == EXTAP_PORT)
        {
            if(pAd->LogoTestCfg.OnTestingWHQL)  // for WLanEXTAPVWIFITest of WHQL Test.
            {
                if (ApGetState(pPort) != AP_STATE_STARTED)
                {
                    ndisStatus = NDIS_STATUS_INVALID_STATE;
                }
            }
            else
            {
                VALIDATE_AP_OP_STATE(pPort);
            }
        }

        if ((pPort->PortType == WFD_GO_PORT) && (!MP_VERIFY_STATE(pPort, OP_STATE)))
        {
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            break;
        }   


        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
            DisassociateRequest->Header,
            NDIS_OBJECT_TYPE_DEFAULT,
            DOT11_DISASSOCIATE_PEER_REQUEST_REVISION_1,
            sizeof(DOT11_DISASSOCIATE_PEER_REQUEST)
            ))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        //disconnect all peer station
        if( ETH_IS_BROADCAST(DisassociateRequest->PeerMacAddr))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Disconnect all peer station..................\n"));
            MacTableReset(pAd, PortNumber);                     
        }
        else
        {
            pEntry = MacTableLookup( pAd,  pPort, DisassociateRequest->PeerMacAddr);
            if((pEntry != NULL) && (pEntry->Aid != 0))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Disconnect  peer station %2x:%2x:%2x:%2x:%2x:%2x\n",
                    DisassociateRequest->PeerMacAddr[0],DisassociateRequest->PeerMacAddr[1],
                    DisassociateRequest->PeerMacAddr[2],DisassociateRequest->PeerMacAddr[3],
                    DisassociateRequest->PeerMacAddr[4],DisassociateRequest->PeerMacAddr[5]));
                if(pEntry->Sst == SST_ASSOC)
                {
                
                    if ((IS_P2P_MS_GO(pAd, pPort) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC)))
                    {
                        // Maintain p2p group table and p2p connected status once disassoc the peers
                        GoPeerDisassocReq(pAd, pEntry, pEntry->Addr);
                    }
                
                    //indicate disassoc event to os
                    APPrepareDisassocPara(pAd, pEntry->Addr, DOT11_DISASSOC_REASON_OS, &disassocPara);

                    //
                    // Send NDIS indication
                    //
                    ApPlatformIndicateDot11Status(pAd,PortNumber, NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocPara, sizeof(DOT11_DISASSOCIATION_PARAMETERS));
                    //delete mac entry here to pass ndistest SOftAP_connectOID case.
                    MacTableDeleteAndResetEntry(pAd,pPort, pEntry->Aid, DisassociateRequest->PeerMacAddr, TRUE);
                     COPY_MAC_ADDR(DisassocReq.Addr,DisassociateRequest->PeerMacAddr);      
                     DisassocReq.Reason = REASON_DISASSOC_INACTIVE;
                     MlmeEnqueue(pAd, pPort,  AP_ASSOC_STATE_MACHINE, APMT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT),&DisassocReq);
                }
            }
        }
    }while(FALSE);

    if (NDIS_STATUS_SUCCESS == ndisStatus)
    {
        *BytesRead = sizeof(DOT11_DISASSOCIATE_PEER_REQUEST);
    }
    return ndisStatus;
}


VOID RTMPAPMakeRSNIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UINT            AuthMode,
    IN  UINT            WepStatus)
{
    DBGPRINT(RT_DEBUG_TRACE,("RTMPAPMakeRSNIE : AuthMode = %s ,WepStatus =%s \n",decodeAuthAlgorithm(AuthMode),decodeCipherAlgorithm(WepStatus)));
    if (WepStatus == Ralink802_11Encryption4Enabled)
        pPort->PortCfg.GroupKeyWepStatus = Ralink802_11Encryption2Enabled;
    else
        pPort->PortCfg.GroupKeyWepStatus = WepStatus;

    if ((AuthMode != Ralink802_11AuthModeWPA) && (AuthMode != Ralink802_11AuthModeWPAPSK)
        && (AuthMode != Ralink802_11AuthModeWPA2) && (AuthMode != Ralink802_11AuthModeWPA2PSK)
        && (AuthMode != Ralink802_11AuthModeWPA1WPA2) && (AuthMode != Ralink802_11AuthModeWPA1PSKWPA2PSK))
        return;

    pPort->SoftAP.ApCfg.RSNIE_Len[0] = 0;
    pPort->SoftAP.ApCfg.RSNIE_Len[1] = 0;
    PlatformZeroMemory(pPort->SoftAP.ApCfg.RSN_IE[0], AP_MAX_LEN_OF_RSNIE);
    PlatformZeroMemory(pPort->SoftAP.ApCfg.RSN_IE[1], AP_MAX_LEN_OF_RSNIE);

    // For WPA1, RSN_IE=221
    if ((AuthMode == Ralink802_11AuthModeWPA) || (AuthMode == Ralink802_11AuthModeWPAPSK)
        || (AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
        RSNIE               *pRsnie;
        RSNIE_AUTH          *pRsnie_auth;
        RSN_CAPABILITIES    *pRSN_Cap;
        UCHAR               Rsnie_size = 0;

        pRsnie = (RSNIE*)pPort->SoftAP.ApCfg.RSN_IE[0];
        PlatformMoveMemory(pRsnie->oui, OUI_WPA_WEP40, 4);
        pRsnie->version = 1;

        switch (WepStatus)
        {
            case Ralink802_11Encryption2Enabled:
                PlatformMoveMemory(pRsnie->mcast, OUI_WPA_TKIP, 4);
                pRsnie->ucount = 1;
                PlatformMoveMemory(pRsnie->ucast[0].oui, OUI_WPA_TKIP, 4);
                Rsnie_size = sizeof(RSNIE);
                break;

            case Ralink802_11Encryption3Enabled:
                PlatformMoveMemory(pRsnie->mcast, OUI_WPA_CCMP, 4);
                pRsnie->ucount = 1;
                PlatformMoveMemory(pRsnie->ucast[0].oui, OUI_WPA_CCMP, 4);
                Rsnie_size = sizeof(RSNIE);
                break;

            case Ralink802_11Encryption4Enabled:
                PlatformMoveMemory(pRsnie->mcast, OUI_WPA_TKIP, 4);
                pRsnie->ucount = 2;
                PlatformMoveMemory(pRsnie->ucast[0].oui, OUI_WPA_TKIP, 4);
                PlatformMoveMemory(pRsnie->ucast[0].oui + 4, OUI_WPA_CCMP, 4);
                Rsnie_size = sizeof(RSNIE) + 4;
                break;
        }

        pRsnie_auth = (RSNIE_AUTH*)((PUCHAR)pRsnie + Rsnie_size);

        switch (AuthMode)
        {
            case Ralink802_11AuthModeWPA:
            case Ralink802_11AuthModeWPA1WPA2:
                pRsnie_auth->acount = 1;
                PlatformMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_WEP40, 4);
                break;

            case Ralink802_11AuthModeWPAPSK:
            case Ralink802_11AuthModeWPA1PSKWPA2PSK:
                pRsnie_auth->acount = 1;
                PlatformMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_TKIP, 4);
                break;
        }

        pRSN_Cap = (RSN_CAPABILITIES*)((PUCHAR)pRsnie_auth + sizeof(RSNIE_AUTH));

        pPort->SoftAP.ApCfg.RSNIE_Len[0] = Rsnie_size + sizeof(RSNIE_AUTH) + sizeof(RSN_CAPABILITIES);
    }

    // For WPA2, RSN_IE=48, if WPA1WPA2/WPAPSKWPA2PSK mix mode, we store RSN_IE in RSN_IE[1] else RSNIE[0]
    if ((AuthMode == Ralink802_11AuthModeWPA2) || (AuthMode == Ralink802_11AuthModeWPA2PSK)
        || (AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
    {
        RSNIE2              *pRsnie2;
        RSNIE_AUTH          *pRsnie_auth2;
        RSN_CAPABILITIES    *pRSN_Cap;
        UCHAR               Rsnie_size = 0;
        DBGPRINT(RT_DEBUG_TRACE,("RTMPAPMakeRSNIE : add wpa2 ie \n"));
        if ((AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
            pRsnie2 = (RSNIE2*)pPort->SoftAP.ApCfg.RSN_IE[1];
        else
            pRsnie2 = (RSNIE2*)pPort->SoftAP.ApCfg.RSN_IE[0];

        pRsnie2->version = 1;

        switch (WepStatus)
        {
            case Ralink802_11Encryption2Enabled:
                PlatformMoveMemory(pRsnie2->mcast, OUI_WPA2_TKIP, 4);
                pRsnie2->ucount = 1;
                PlatformMoveMemory(pRsnie2->ucast[0].oui, OUI_WPA2_TKIP, 4);
                Rsnie_size = sizeof(RSNIE2);
                break;

            case Ralink802_11Encryption3Enabled:
                PlatformMoveMemory(pRsnie2->mcast, OUI_WPA2_CCMP, 4);
                pRsnie2->ucount = 1;
                PlatformMoveMemory(pRsnie2->ucast[0].oui, OUI_WPA2_CCMP, 4);
                Rsnie_size = sizeof(RSNIE2);
                break;

            case Ralink802_11Encryption4Enabled:
                PlatformMoveMemory(pRsnie2->mcast, OUI_WPA2_TKIP, 4);
                pRsnie2->ucount = 2;
                PlatformMoveMemory(pRsnie2->ucast[0].oui, OUI_WPA2_TKIP, 4);
                PlatformMoveMemory(pRsnie2->ucast[0].oui + 4, OUI_WPA2_CCMP, 4);
                Rsnie_size = sizeof(RSNIE2) + 4;
                break;
        }

        pRsnie_auth2 = (RSNIE_AUTH*)((PUCHAR)pRsnie2 + Rsnie_size);

        switch (AuthMode)
        {
            case Ralink802_11AuthModeWPA2:
            case Ralink802_11AuthModeWPA1WPA2:
                pRsnie_auth2->acount = 1;
                PlatformMoveMemory(pRsnie_auth2->auth[0].oui, OUI_WPA2_WEP40, 4);
                break;

            case Ralink802_11AuthModeWPA2PSK:
            case Ralink802_11AuthModeWPA1PSKWPA2PSK:
                pRsnie_auth2->acount = 1;
                PlatformMoveMemory(pRsnie_auth2->auth[0].oui, OUI_WPA2_TKIP, 4);
                break;
        }

        pRSN_Cap = (RSN_CAPABILITIES*)((PUCHAR)pRsnie_auth2 + sizeof(RSNIE_AUTH));
        pRSN_Cap->field.Pre_Auth = (pPort->SoftAP.ApCfg.bPreAuth == TRUE) ? 1 : 0;

        if ((AuthMode == Ralink802_11AuthModeWPA1WPA2) || (AuthMode == Ralink802_11AuthModeWPA1PSKWPA2PSK))
            pPort->SoftAP.ApCfg.RSNIE_Len[1] = Rsnie_size + sizeof(RSNIE_AUTH) + sizeof(RSN_CAPABILITIES);
        else
            pPort->SoftAP.ApCfg.RSNIE_Len[0] = Rsnie_size + sizeof(RSNIE_AUTH) + sizeof(RSN_CAPABILITIES);
    }

    DBGPRINT(RT_DEBUG_TRACE,("RTMPAPMakeRSNIE : RSNIE_Len = %d\n", pPort->SoftAP.ApCfg.RSNIE_Len[0]));
}

VOID  ApInfoResetHandler(PMP_PORT pPort)
{
    PMP_ADAPTER   pAd = pPort->pAd;
    //UCHAR     BBPValue;
    PMP_PORT  pStaPort = NULL;

    // we should disable WPS Feature when AP Reset
    pPort->SoftAP.ApCfg.bWPSEnable = FALSE;

    // Reset Additional IE data
    //APRemoveAdditionalIE(pPort);
    Ndis6CommonPortFreeVariableIE(pPort);

    //Clear the desired SSID
    PlatformZeroMemory(&pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));

    // Clear the desired BSSID list
    pPort->PortCfg.DesiredBSSIDList[0][0] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][1] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][2] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][3] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][4] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][5] = 0xFF;
    pPort->PortCfg.DesiredBSSIDCount = 1;
    pPort->PortCfg.AcceptAnyBSSID = TRUE;
    DBGPRINT(RT_DEBUG_TRACE, ("%s(%d): Reset DesiredBSSIDList and AcceptAnyBSSID.\n", __FUNCTION__, pPort->PortNumber));

    //APCleanupPrivacyExemptionList(pAd);
    Ndis6CommonPortFreePrivacyExemptionList(pPort);

    // 
    // Stop AP
    //
    if (AP_STATE_STARTED == pPort->ApState || AP_STATE_STARTING == pPort->ApState)
    {       
        ApSetState(pPort,AP_STATE_STOPPING);
        //stop AP
        APStop(pAd,pPort);
    }

    //
    // Reset Stuff here.
    //
    //\\//\\TODO:
    //change the NIC state to ExtAP INIT state
    //cancel any outstanding operations such as beacon, probe request/response, association and scan
    //cleanup TX and RX queue
    
    MP_SET_STATE(pPort, INIT_STATE);

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        pPort->PortCfg.DefaultKeyId = 0;    
        MlmeInfoWPARemoveAllKeys(pAd,pPort);
    }

    //add check if AP started, this will caue adhoc disconnected
    //if(pPort->SoftAP.bAPStart)
    //  MtAsicDisableSync(pAd);
    
    ApSetState(pPort,AP_STATE_STOPPED);
    pPort->SoftAP.bAPStart = FALSE;

    //RESET BACK TO 20 mhz when sta port already connect and connected to legacy AP
    pStaPort = Ndis6CommonGetStaPort(pAd);
    if ((pPort->BBPCurrentBW == BW_40) && 
        (pStaPort) && (INFRA_ON(pStaPort)) && 
        (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE))
    {
        pPort->BBPCurrentBW = BW_20;
        pPort->CentralChannel = pPort->Channel;
        SwitchBandwidth(pAd, TRUE, pPort->Channel, BW_20, EXTCHA_NONE);
        DBGPRINT(RT_DEBUG_TRACE, ("Change back to 20 MHz for legacy AP\n" ));

        // Update GO's HT settings again
        if (P2P_ON(pPort) && (pAd->OpMode == OPMODE_STAP2P) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        {
            P2pSetDefaultGOHt(pAd, pPort);
        }
    }
}


NDIS_STATUS
ApInfoResetRequest(
    IN  PMP_ADAPTER       pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  OUT PVOID           InformationBuffer,
    IN  ULONG               InputBufferLength,
    IN  ULONG               OutputBufferLength,
    OUT PULONG              BytesRead,
    OUT PULONG              BytesWritten,
    OUT PULONG              BytesNeeded)
{
    
    NDIS_STATUS         ResetStatus = NDIS_STATUS_SUCCESS;
    PDOT11_RESET_REQUEST pDot11ResetRequest = InformationBuffer;
    PDOT11_STATUS_INDICATION pDot11StatusIndication = InformationBuffer;
    PMP_PORT          pPort = pAd->PortList[PortNumber];  
    
    do
    {
        // init output parameters
        *BytesRead = 0;
        *BytesWritten = 0;

        //
        // First make sure the input buffer is large enough to
        // hold a RESET_CONFIRM
        //
        if (OutputBufferLength < sizeof(DOT11_STATUS_INDICATION))
        {
            *BytesNeeded = sizeof(DOT11_STATUS_INDICATION);
            return NDIS_STATUS_INVALID_LENGTH;
        }

        //
        // Validate the buffer length
        //
        if (InputBufferLength < sizeof(DOT11_RESET_REQUEST))
        {
            *BytesNeeded = sizeof(DOT11_RESET_REQUEST);
            return NDIS_STATUS_INVALID_LENGTH;
        }

        *BytesRead = sizeof(DOT11_RESET_REQUEST);

        //
        //Reset the AP
        //
        if (pDot11ResetRequest->dot11ResetType != dot11_reset_type_phy_and_mac)
        {
            ResetStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if(KeGetCurrentIrql() == DISPATCH_LEVEL)
        {
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_AP_RESET_REQUEST,&PortNumber,sizeof(NDIS_PORT_NUMBER)); 
        }   
        else
        {
            ApInfoResetHandler(pPort);
        }
        //
        // Complete the reset request with appropriate status to NDIS
        //
        pDot11StatusIndication->uStatusType = DOT11_STATUS_RESET_CONFIRM;
        pDot11StatusIndication->ndisStatus = ResetStatus;

        // Clear PMKID cache.
        PlatformZeroMemory(&pPort->CommonCfg.PMKIDCache, sizeof(PMKID_CACHE));    
        pAd->StaCfg.PMKIDCount = 0;
        PlatformZeroMemory(pAd->StaCfg.PMKIDList, (sizeof(DOT11_PMKID_ENTRY) * STA_PMKID_MAX_COUNT));
        pPort->PortCfg.ExcludeUnencrypted  = FALSE;

        // 2->will not clear up accepted multicast poll
        if (pPort->CommonCfg.MPolicy != 2)
        {
            //cleasr multicast list table
            pPort->PortCfg.MCAddressCount  = 0;
            PlatformZeroMemory(pPort->PortCfg.MCAddressList, DOT11_ADDRESS_SIZE * HW_MAX_MCAST_LIST_SIZE);
        }
        
        // Safety, clear it.
        MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS); 
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_SOFTAP_IS_DOWN);
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);

        DBGPRINT(RT_DEBUG_TRACE, ("<== ApInfoResetRequest\n"));
        ResetStatus = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    return ResetStatus;
}

NDIS_STATUS
ApSetAdditionalIe(
    IN  PMP_PORT pPort,
    IN  PDOT11_ADDITIONAL_IE AdditionalIe)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PVOID newBeaconIeData = NULL; 
    PVOID newResponseIeData = NULL;
    PMP_ADAPTER   pAd = pPort->pAd;

    do
    {
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    AdditionalIe->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_ADDITIONAL_IE_REVISION_1,
                    sizeof(DOT11_ADDITIONAL_IE)))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }
        if (AdditionalIe->uBeaconIEsLength > 0)
        {
             PlatformAllocateMemory(pAd,
                                &newBeaconIeData, 
                                AdditionalIe->uBeaconIEsLength);

            if(newBeaconIeData == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Alloc AdditionalIe BeaconIEs buffer fail\n"));
                 ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }           

            // 
            // copy IEs
            //
            RtlCopyMemory(
                        newBeaconIeData,
                        (BYTE *)AdditionalIe + AdditionalIe->uBeaconIEsOffset,
                        AdditionalIe->uBeaconIEsLength
                        );
        }

        if (AdditionalIe->uResponseIEsLength > 0)
        {
           PlatformAllocateMemory(pAd,
            &newResponseIeData, 
            AdditionalIe->uResponseIEsLength);

            if(newResponseIeData == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Alloc AdditionalIe ResponseIE buffer fail\n"));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
            
            // 
            // copy IEs
            //
            RtlCopyMemory(
                        newResponseIeData,
                        (BYTE *)AdditionalIe + AdditionalIe->uResponseIEsOffset,
                        AdditionalIe->uResponseIEsLength
                        );
         }

        // 
        // free the old IEs
        //
        if (pPort->PortCfg.AdditionalBeaconIEData != NULL)
        {
            PlatformFreeMemory(pPort->PortCfg.AdditionalBeaconIEData,  pPort->PortCfg.AdditionalBeaconIESize);                
        }
        
        if (pPort->PortCfg.AdditionalResponseIEData != NULL)
        {
            PlatformFreeMemory(pPort->PortCfg.AdditionalResponseIEData, pPort->PortCfg.AdditionalResponseIESize);              
        }

        // 
        // cache new IEs
        //
        pPort->PortCfg.AdditionalBeaconIEData = newBeaconIeData;
        pPort->PortCfg.AdditionalBeaconIESize = AdditionalIe->uBeaconIEsLength;
        newBeaconIeData = NULL;

        pPort->PortCfg.AdditionalResponseIEData = newResponseIeData;
        pPort->PortCfg.AdditionalResponseIESize = AdditionalIe->uResponseIEsLength;
        newResponseIeData = NULL;       
    } while (FALSE);

    AsicPauseBssSync(pAd);
    ApConnectUpdateBeaconFrame(pAd,pPort);
    if(pPort->SoftAP.bAPStart)
    {
        AsicResumeBssSync(pAd);
    }

    return ndisStatus;
}

NDIS_STATUS
ValiateAdditionalIeSize(
    IN  PDOT11_ADDITIONAL_IE    IeData,
    IN  ULONG                   IeSize,
    OUT PULONG                  SizeNeeded)
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_INVALID_LENGTH;
    ULONG                       requiredBeaconIeSize = 0;
    ULONG                       requiredResponseIeSize = 0;

    do
    {
        if (IeSize < sizeof(DOT11_ADDITIONAL_IE))
        {
            *SizeNeeded = sizeof(DOT11_ADDITIONAL_IE);
            break;
        }

        // check the size for the beacon IEs
        if (RtlULongAdd(IeData->uBeaconIEsOffset, IeData->uBeaconIEsLength, &requiredBeaconIeSize) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        // check the size for the probe response IEs
        if (RtlULongAdd(IeData->uResponseIEsOffset, IeData->uResponseIEsLength, &requiredResponseIeSize) != STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        // the size needed is the larger one
        *SizeNeeded = (requiredBeaconIeSize > requiredResponseIeSize) ? requiredBeaconIeSize : requiredResponseIeSize;

        if (IeSize >= *SizeNeeded)
        {
            ndisStatus = NDIS_STATUS_SUCCESS;
        }
    } while (FALSE);

    return ndisStatus;
}

VOID 
RTMPApIndicateFrequencyAdoped(
    IN PMP_PORT ApPort
    )
{
    PMP_PORT    pPort = ApPort;
    PMP_ADAPTER pAd = ApPort->pAd;
    DOT11_PHY_FREQUENCY_ADOPTED_PARAMETERS params;

    // send out the NDIS_STATUS_DOT11_PHY_FREQUENCY_ADOPTED status indication
    PlatformZeroMemory(&params, sizeof(DOT11_PHY_FREQUENCY_ADOPTED_PARAMETERS));

    MP_ASSIGN_NDIS_OBJECT_HEADER(
        params.Header,
        NDIS_OBJECT_TYPE_DEFAULT,
        DOT11_PHY_FREQUENCY_ADOPTED_PARAMETERS_REVISION_1,
        sizeof(DOT11_PHY_FREQUENCY_ADOPTED_PARAMETERS)
        );

    params.ulChannel = pPort->Channel;
    params.ulPhyId = MlmeInfoGetPhyIdByChannel(pAd, pPort->Channel);    
    
    ApPlatformIndicateDot11Status(
        ApPort->pAd, 
        ApPort->PortNumber,
        NDIS_STATUS_DOT11_PHY_FREQUENCY_ADOPTED, 
        NULL,                   // no request ID
        &params, 
        sizeof(params)
        );

    DBGPRINT(RT_DEBUG_TRACE, ("IndicateFrequencyAdoped phy id = %d,Channel = %d\n",params.ulPhyId,pPort->Channel));
}

