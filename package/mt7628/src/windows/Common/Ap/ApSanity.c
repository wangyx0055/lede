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
     ap_sanity.c
     
     Abstract:
     Handle association related requests either from WSTA or from local MLME
     
     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
     John Chang  08-14-2003    created for 11g soft-AP
     John Chang  12-30-2004    merge with STA driver for RT2600
*/

#include "MtConfig.h"

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
extern UCHAR    PRE_N_HT_OUI[];


BOOLEAN ApSanityPeerAssocReqSanity(
    IN PMP_ADAPTER pAd, 
    IN NDIS_PORT_NUMBER PortNumber,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pListenInterval, 
    OUT UCHAR *pSsidLen,
    OUT char *Ssid,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *RSN,
    OUT UCHAR *pRSNLen,
    OUT BOOLEAN *pbWmmCapable,
    OUT ULONG  *pRalinkIe,
    OUT ULONG *WpsLen, 
    OUT PUCHAR pWps, 
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement, 
    OUT UCHAR        *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT UCHAR            *pVhtCapabilityLen,
    OUT VHT_CAP_IE       *pVhtCapability)
{
    CHAR         *Ptr;
    PFRAME_802_11 Fr = (PFRAME_802_11)Msg;
    PEID_STRUCT  eid_ptr;
    UCHAR        Sanity=0;
    UCHAR           WPA1_OUI[3]={0x00,0x50,0xf2};
    UCHAR           WPA2_OUI[3]={0x00,0x0F,0xAC};
    MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY*)NULL;
    PMP_PORT pPort = pAd->PortList[PortNumber];
    // to prevent caller from using garbage output value
    *pRatesLen    = 0;
    *pRSNLen      = 0;
    *pbWmmCapable = FALSE;
    *pRalinkIe    = 0;
    *pHtCapabilityLen= 0;
    *P2PSubelementLen = 0;
    *WpsLen = 0;
    
    COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
    Ptr = Fr->Octet;

    PlatformMoveMemory(pCapabilityInfo, &Fr->Octet[0], 2);
    PlatformMoveMemory(pListenInterval, &Fr->Octet[2], 2);

    eid_ptr = (PEID_STRUCT) &Fr->Octet[4];

    pEntry = MacTableLookup(pAd,  pPort, pAddr2);
    
    if(!pEntry)
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] NO entry found in the software-based MAC table.\n",__FUNCTION__,__LINE__));
    }

    // get variable fields from payload and advance the pointer
    while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((UCHAR*)Fr + MsgLen))
    {
        switch(eid_ptr->Eid)
        {
            case IE_SSID:
                if (((Sanity&0x1) == 1))
                    break;
                if((eid_ptr->Len <= MAX_LEN_OF_SSID))
                {
                    Sanity |= 0x01;
                    PlatformMoveMemory(Ssid, eid_ptr->Octet, eid_ptr->Len);
                    *pSsidLen = eid_ptr->Len;
                    DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAssocReqSanity - SsidLen = %d  \n", *pSsidLen ));
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAssocReqSanity - wrong IE_SSID\n"));
                    return FALSE;
                }
                break;
                
            case IE_SUPP_RATES:
                if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) && (eid_ptr->Len > 0))
                {
                    Sanity |= 0x02;
                    PlatformMoveMemory(Rates, eid_ptr->Octet, eid_ptr->Len);
                    DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAssocReqSanity - IE_SUPP_RATES., Len=%d. Rates[0]=%x\n",eid_ptr->Len, Rates[0]));
                    DBGPRINT(RT_DEBUG_TRACE, ("Rates[1]=%x %x %x %x %x %x %x\n", Rates[1], Rates[2], Rates[3], Rates[4], Rates[5], Rates[6], Rates[7]));
                    *pRatesLen = eid_ptr->Len;
                }
                else
                {
                    // HT rate not ready yet. return true temporarily. rt2860c
                    //DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAssocReqSanity - wrong IE_SUPP_RATES\n"));
                    Sanity |= 0x02;
                    *pRatesLen = 8;
                    Rates[0] = 0x82;
                    Rates[1] = 0x84;
                    Rates[2] = 0x8b;
                    Rates[3] = 0x96;
                    Rates[4] = 0x12;
                    Rates[5] = 0x24;
                    Rates[6] = 0x48;
                    Rates[7] = 0x6c;
                    DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAssocReqSanity - wrong IE_SUPP_RATES., Len=%d\n",eid_ptr->Len));
                    //return FALSE;
                }
                break;
                
            case IE_EXT_SUPP_RATES:
                if (eid_ptr->Len + *pRatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, eid_ptr->Len);
                    *pRatesLen = (*pRatesLen) + eid_ptr->Len;
                }
                else
                {
                    PlatformMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, MAX_LEN_OF_SUPPORTED_RATES - (*pRatesLen));
                    *pRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
                }
                break;
                
            case IE_HT_CAP:
                if (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (eid_ptr->Len >= sizeof(HT_CAPABILITY_IE))
                    {
                        PlatformMoveMemory(pHtCapability, eid_ptr->Octet, SIZE_HT_CAP_IE);
                        *pHtCapabilityLen = SIZE_HT_CAP_IE;
                        Sanity |= 0x10;
                        DBGPRINT(RT_DEBUG_WARN, ("ApSanityPeerAssocReqSanity - IE_HT_CAP\n"));
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN, ("ApSanityPeerAssocReqSanity - wrong IE_HT_CAP.eid_ptr->Len = %d\n", eid_ptr->Len));
                    }
                }
                break;

            case IE_VHT_CAP:
                if (eid_ptr->Len == sizeof(VHT_CAP_IE))
                {
                    PlatformMoveMemory(pVhtCapability, eid_ptr->Octet, SIZE_OF_VHT_CAP_IE);
                    *pVhtCapabilityLen = SIZE_OF_VHT_CAP_IE;
                    DBGPRINT(RT_DEBUG_WARN, ("%s - VHT_CAP_IE\n",__FUNCTION__));
                }
                else
                {
                    DBGPRINT(RT_DEBUG_WARN, ("%s - wrong VHT_CAP_IE. eid_ptr->Len = %d\n",__FUNCTION__, eid_ptr->Len));
                }
                break;

            case IE_WPA:    // same as IE_VENDOR_SPECIFIC
            case IE_WPA2:
                if (PlatformEqualMemory(eid_ptr->Octet, P2POUIBYTE, 4) && (eid_ptr->Len >= 4))
                {
                    if (*P2PSubelementLen == 0)
                    {
                        PlatformMoveMemory(pP2pSubelement, &eid_ptr->Eid, (eid_ptr->Len+2));
                        *P2PSubelementLen = (eid_ptr->Len+2);
                    }
                    else
                    {
                        PlatformMoveMemory(pP2pSubelement + *P2PSubelementLen, &eid_ptr->Eid, (eid_ptr->Len+2));
                        *P2PSubelementLen += (eid_ptr->Len+2);
                    }

                    DBGPRINT(RT_DEBUG_TRACE, (" ! ===>P2P - ApSanityPeerAssocReqSanity  P2P IE Len becomes = %d.   %s\n", *P2PSubelementLen, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                    break;
                }
                if (PlatformEqualMemory(eid_ptr->Octet, RALINK_OUI, 3) && (eid_ptr->Len == 7))
                {
                    *pRalinkIe = eid_ptr->Octet[3];
                    
                    // Error handle, 
                    if ((pPort->CommonCfg.bAggregationCapable) && (pPort->CommonCfg.bPiggyBackCapable) && (pPort->SoftAP.ApCfg.bOneClientUsePiggyBack)
                        && ((*pRalinkIe & 0x00000001) == 1))
                    {
                        QOS_CSR0_STRUC  qos_csr0;
                        QOS_CSR1_STRUC  qos_csr1;
                        qos_csr1.word = 0;
                        qos_csr0.word = 0;

                        //RTUSBReadMACRegister(pAd, QOS_CSR0, &qos_csr0.word);
                        //RTUSBReadMACRegister(pAd, QOS_CSR1, &qos_csr1.word);

                        if ((pEntry) && (qos_csr0.field.Byte0 == pEntry->Addr[0]) && (qos_csr0.field.Byte1 == pEntry->Addr[1])
                            && (qos_csr0.field.Byte2 == pEntry->Addr[2]) && (qos_csr0.field.Byte3 == pEntry->Addr[3])
                            && (qos_csr1.field.Byte4 == pEntry->Addr[4]) && (qos_csr1.field.Byte5 == pEntry->Addr[5]))
                        {
                            ApWpaDisAssocAction(pAd,pPort, pEntry, REASON_UNSPECIFY);
                            DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_WPA2. \n", __FUNCTION__));
                            return FALSE;
                        }
                    }
                    break;
                }
                
                if (((Sanity&0x10) == 0)&& PlatformEqualMemory(eid_ptr->Octet, PRE_N_HT_OUI, 3) && (eid_ptr->Len >= 4))
                {
                    if ((eid_ptr->Octet[3] == OUI_PREN_HT_CAP) && (eid_ptr->Len >= 30))
                    {
                        PlatformMoveMemory(pHtCapability, &eid_ptr->Octet[4], sizeof(HT_CAPABILITY_IE));
                        *pHtCapabilityLen = SIZE_HT_CAP_IE; // Nnow we only support 26 bytes.
                    }
                    DBGPRINT(RT_DEBUG_WARN, ("%s - PRE_N_HT_OUI\n", __FUNCTION__));
                    break;
                }
                // WMM_IE
                if (PlatformEqualMemory(eid_ptr->Octet, WME_INFO_ELEM, 6) && (eid_ptr->Len == 7))
                {
                    *pbWmmCapable = TRUE;
#if UAPSD_AP_SUPPORT
                    DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==>bAPSDCapable(=%d), bWmmCapable(=%d)\n", __FUNCTION__, __LINE__, pPort->CommonCfg.bAPSDCapable, pPort->CommonCfg.bWmmCapable));
                    if(IS_AP_SUPPORT_UAPSD(pAd, pPort))
                    {
                        ApUapsdAssocParse(pAd, pPort, pEntry, (PUCHAR)&eid_ptr->Octet[6]);
                    }               
#endif  
                    break;
                }

                if (PlatformEqualMemory(eid_ptr->Octet, WPS_OUI, 4))
                {

                    PlatformMoveMemory(pWps, eid_ptr->Octet, eid_ptr->Len);
                    // Get WPS. Act like get tne RSNIE.
                    Sanity |= 0x80;

                    *WpsLen = eid_ptr->Len;
                    DBGPRINT(RT_DEBUG_TRACE, ("got WPS OUI !\n"));      
                    DBGPRINT(RT_DEBUG_TRACE, (" ! %s  WPS IE Len  %d.   %s\n", __FUNCTION__, *WpsLen, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                    break;
                }
                
                if ((pPort->PortCfg.AuthMode < Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeAutoSwitch))
                    break;
                
                // If this IE did not begins with 00:0x50:0xf2:0x01,  it would be proprietary.  So we ignore
                if (!PlatformEqualMemory(eid_ptr->Octet, WPA1_OUI, sizeof(WPA1_OUI))
                    && !PlatformEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI)))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Not RSN IE, maybe WMM IE!!!\n"));
                    break;                          
                }

                if (((eid_ptr->Eid == IE_WPA) && (PlatformEqualMemory(eid_ptr->Octet, WPA_OUI, 4)))
                    ||(eid_ptr->Eid == IE_WPA2))
                {
                    if ((eid_ptr->Len <= AP_MAX_LEN_OF_RSNIE) && (eid_ptr->Len > AP_MIN_LEN_OF_RSNIE))
                    {
                        if (!pEntry)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("null pEntry!!!\n"));
                            return FALSE;
                        }

                        DBGPRINT(RT_DEBUG_TRACE, ("Receive IE_WPA : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                            eid_ptr->Octet[0],eid_ptr->Octet[1],eid_ptr->Octet[2],eid_ptr->Octet[3],
                            eid_ptr->Octet[4],eid_ptr->Octet[5],eid_ptr->Octet[6],eid_ptr->Octet[7]));
                        
                        *pRSNLen=eid_ptr->Len;
                        if(PlatformEqualMemory(eid_ptr->Octet,WPS_OUI,4))
                        {                                       
                            DBGPRINT(RT_DEBUG_TRACE, ("got WPS OUI !\n"));      
                            break;
                        }
                        if (!ApWpaCheckMcast(pAd, pPort,eid_ptr, pEntry))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaCheckMcast FAILED !!!\n"));
                                ApWpaDisAssocAction( pAd,pPort, pEntry, REASON_MCIPHER_NOT_VALID);
                            return FALSE;
                        }                        
                        if (!ApWpaCheckUcast(pAd, pPort,eid_ptr, pEntry))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaCheckUcast FAILED !!!\n"));
                                ApWpaDisAssocAction( pAd, pPort,pEntry, REASON_UCIPHER_NOT_VALID);
                            return FALSE;
                        }                        
                        if (!ApWpaCheckAuth(pAd,pPort, eid_ptr, pEntry))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaCheckAuth FAILED !!!\n"));
                                ApWpaDisAssocAction( pAd, pPort,pEntry, REASON_INVALID_IE);
                            return FALSE;
                        }                        
                        
                        PlatformMoveMemory(RSN, eid_ptr->Octet, eid_ptr->Len);
#if DBG
                        // for debug
                        {
                            UCHAR CipherAlg = CIPHER_NONE;
                            if (IS_WEP_STATUS_ON(pEntry->WepStatus))
                                CipherAlg = CIPHER_WEP64;
                            else if (pEntry->WepStatus == Ralink802_11Encryption2Enabled)
                                CipherAlg = CIPHER_TKIP;
                            else if (pEntry->WepStatus == Ralink802_11Encryption3Enabled)
                                CipherAlg = CIPHER_AES;
                            DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAssocReqSanity(AID#%d WepStatus=%s)\n", pEntry->Aid, DecodeCipherName(CipherAlg)));
                        }
#endif
                    }
                    else
                    {
                        *pRSNLen=0;
                        DBGPRINT(RT_DEBUG_TRACE, ("%s - missing IE_WPA(%d)\n", __FUNCTION__,eid_ptr->Len));
                        return FALSE;
                    }               
                }               
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s 2- (%d)\n", __FUNCTION__,eid_ptr->Len));
                    break;

                }               
                break;
            default:
                break;
        }
        eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
    }

    if ((Sanity&0x3) != 0x03)    
    {
        DBGPRINT(RT_DEBUG_WARN, ("%s - missing mandatory field\n", __FUNCTION__));
        return FALSE;
    }
    else
    {
        DBGPRINT(RT_DEBUG_WARN, ("%s - success\n", __FUNCTION__));
        return TRUE;
    }
}

BOOLEAN ApSanityPeerReassocReqSanity(
    IN  PMP_ADAPTER   pAd, 
    IN NDIS_PORT_NUMBER PortNumber,
    IN  VOID *Msg, 
    IN  ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pListenInterval, 
    OUT PUCHAR pApAddr,
    OUT UCHAR *pSsidLen,
    OUT char *Ssid,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *RSN,
    OUT UCHAR *pRSNLen,
    OUT BOOLEAN *pbWmmCapable,
    OUT ULONG *pRalinkIe,
    OUT ULONG   *WpsLen, 
    OUT PUCHAR  pWps, 
    OUT ULONG   *P2PSubelementLen, 
    OUT PUCHAR  pP2pSubelement, 
    OUT UCHAR        *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT UCHAR            *pVhtCapabilityLen,
    OUT VHT_CAP_IE *pVhtCapability)
{
    CHAR            *Ptr;
    PFRAME_802_11   Fr = (PFRAME_802_11)Msg;
    PEID_STRUCT     eid_ptr;
    UCHAR           Sanity=0;
    UCHAR           WPA1_OUI[3]={0x00,0x50,0xf2};
    UCHAR           WPA2_OUI[3]={0x00,0x0F,0xAC};
    MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY*)NULL;
    PMP_PORT      pPort= pAd->PortList[PortNumber];

    // to prevent caller from using garbage output value
    *pSsidLen     = 0;
    *pRatesLen    = 0;
    *pRSNLen      = 0;
    *pbWmmCapable = FALSE;
    *pRalinkIe    = 0;
    *P2PSubelementLen = 0;
    *WpsLen = 0;
    
    COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
    Ptr = Fr->Octet;

    PlatformMoveMemory(pCapabilityInfo, &Fr->Octet[0], 2);
    PlatformMoveMemory(pListenInterval, &Fr->Octet[2], 2);
    PlatformMoveMemory(pApAddr, &Fr->Octet[4], 6);

    eid_ptr = (PEID_STRUCT) &Fr->Octet[10];

    pEntry = MacTableLookup(pAd,  pPort, pAddr2);

    if(!pEntry)
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] NO entry been found in the software-based MAC table.\n",__FUNCTION__,__LINE__));
    }

    // get variable fields from payload and advance the pointer
    while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((UCHAR*)Fr + MsgLen))
    {
        switch(eid_ptr->Eid)
        {
            case IE_SSID:
                if(eid_ptr->Len <= MAX_LEN_OF_SSID)
                {
                    Sanity |= 0x01;
                    PlatformMoveMemory(Ssid, eid_ptr->Octet, eid_ptr->Len);
                    *pSsidLen = eid_ptr->Len;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_SSID (len=%d)\n", __FUNCTION__, eid_ptr->Len));
                    return FALSE;
                }
                break;
                
            case IE_SUPP_RATES:
                if (eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    Sanity |= 0x02;
                    PlatformMoveMemory(Rates, eid_ptr->Octet, eid_ptr->Len);
                    *pRatesLen = eid_ptr->Len;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_SUPP_RATES (len=%d)\n", __FUNCTION__, eid_ptr->Len));
                    return FALSE;
                }
                break;
                
            case IE_EXT_SUPP_RATES:
                if (eid_ptr->Len + *pRatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, eid_ptr->Len);
                    *pRatesLen = (*pRatesLen) + eid_ptr->Len;
                }
                else
                {
                    PlatformMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, MAX_LEN_OF_SUPPORTED_RATES - (*pRatesLen));
                    *pRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
                }
                break;

            case IE_HT_CAP:
                DBGPRINT(RT_DEBUG_WARN, ("%s - IE_HT_CAP. \n", __FUNCTION__));
                if (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (eid_ptr->Len >= sizeof(HT_CAPABILITY_IE))
                    {
                        PlatformMoveMemory(pHtCapability, eid_ptr->Octet, SIZE_HT_CAP_IE);
                        *pHtCapabilityLen = SIZE_HT_CAP_IE;
                        Sanity |= 0x10;
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN, ("ApSanityPeerAssocReqSanity - wrong IE_HT_CAP. \n"));
                    }                   
                }
                break;

            case IE_VHT_CAP:
                DBGPRINT(RT_DEBUG_WARN, ("%s - IE_VHT_CAP. \n",__FUNCTION__));
                if (eid_ptr->Len == SIZE_OF_VHT_CAP_IE)
                {
                    *pVhtCapabilityLen = SIZE_OF_VHT_CAP_IE;
                    PlatformMoveMemory(pVhtCapability, eid_ptr->Octet, SIZE_OF_VHT_CAP_IE);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_VHT_CAP. \n",__FUNCTION__));
                }
                break;

            case IE_WPA: // same as IE_VENDOR_SPECIFIC
            case IE_WPA2:
                if (PlatformEqualMemory(eid_ptr->Octet, P2POUIBYTE, 4) && (eid_ptr->Len > 4))
                {
                    PlatformMoveMemory(pP2pSubelement, eid_ptr->Octet, eid_ptr->Len);
                    *P2PSubelementLen = eid_ptr->Len;

                    DBGPRINT(RT_DEBUG_TRACE, (" ! ===>P2P - ApSanityPeerAssocReqSanity  P2P IE Len = %d.   %s\n", *P2PSubelementLen, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                    break;
                }
                if (PlatformEqualMemory(eid_ptr->Octet, RALINK_OUI, 3) && (eid_ptr->Len == 7))
                {
                    if (eid_ptr->Octet[3] != 0)
                        *pRalinkIe = eid_ptr->Octet[3];
                    else
                        *pRalinkIe = 0xf0000000; // Set to non-zero value (can't set bit0-2) to represent this is Ralink Chip. So at linkup, we will set ralinkchip flag.

                    // Error handle, 
                    if ((pPort->CommonCfg.bAggregationCapable) && (pPort->CommonCfg.bPiggyBackCapable) && (pPort->SoftAP.ApCfg.bOneClientUsePiggyBack)
                        && ((*pRalinkIe & 0x00000001) == 1))
                    {
                        QOS_CSR0_STRUC  qos_csr0;
                        QOS_CSR1_STRUC  qos_csr1;
                        qos_csr0.word = 0;
                        qos_csr1.word = 0;
                        //RTUSBReadMACRegister(pAd, QOS_CSR0, &qos_csr0.word);
                        //RTUSBReadMACRegister(pAd, QOS_CSR1, &qos_csr1.word);

                        if ((pEntry) && (qos_csr0.field.Byte0 == pEntry->Addr[0]) && (qos_csr0.field.Byte1 == pEntry->Addr[1])
                            && (qos_csr0.field.Byte2 == pEntry->Addr[2]) && (qos_csr0.field.Byte3 == pEntry->Addr[3])
                            && (qos_csr1.field.Byte4 == pEntry->Addr[4]) && (qos_csr1.field.Byte5 == pEntry->Addr[5]))
                        {
                            ApWpaDisAssocAction(pAd,pPort, pEntry, REASON_UNSPECIFY);
                            return FALSE;
                        }
                    }
                    break;
                }
                
                if (((Sanity&0x10) == 0)&& PlatformEqualMemory(eid_ptr->Octet, PRE_N_HT_OUI, 3) && (eid_ptr->Len >= 4))
                {
                    if ((eid_ptr->Octet[3] == OUI_PREN_HT_CAP) && (eid_ptr->Len >= 30))
                    {
                        PlatformMoveMemory(pHtCapability, &eid_ptr->Octet[4], sizeof(HT_CAPABILITY_IE));
                        *pHtCapabilityLen = SIZE_HT_CAP_IE; // Nnow we only support 26 bytes.
                    }
                    DBGPRINT(RT_DEBUG_WARN, ("%s - PRE_N_HT_OUI\n", __FUNCTION__));
                    break;
                }
                // WMM_IE
                if (PlatformEqualMemory(eid_ptr->Octet, WME_INFO_ELEM, 6) && (eid_ptr->Len == 7))
                {
                    *pbWmmCapable = TRUE;
#if UAPSD_AP_SUPPORT
                    DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==>bAPSDCapable(=%d), bWmmCapable(=%d)\n", __FUNCTION__, __LINE__, pPort->CommonCfg.bAPSDCapable, pPort->CommonCfg.bWmmCapable));
                    if(IS_AP_SUPPORT_UAPSD(pAd, pPort))
                    {
                        ApUapsdAssocParse(pAd, pPort, pEntry, (PUCHAR)&eid_ptr->Octet[6]);
                    }               
#endif                  
                    break;
                }

                if (PlatformEqualMemory(eid_ptr->Octet, WPS_OUI, 4))
                {
                    PlatformMoveMemory(pWps, eid_ptr->Octet, eid_ptr->Len);
                    // Get WPS. Act like get tne RSNIE. So set 0x80 for later check criteria.
                    Sanity |= 0x80;

                    *WpsLen = eid_ptr->Len;
                    DBGPRINT(RT_DEBUG_TRACE, ("got WPS OUI !\n"));      
                    DBGPRINT(RT_DEBUG_TRACE, (" ! %s  WPS IE   %d.   %s\n", __FUNCTION__, *P2PSubelementLen, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                    break;
                }
                
                if ((pPort->PortCfg.AuthMode < Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeAutoSwitch))
                    break;

                // If this IE did not begins with 00:0x50:0xf2:0x01,  it would be proprietary.  So we ignore
                if (!PlatformEqualMemory(eid_ptr->Octet, WPA1_OUI, sizeof(WPA1_OUI))
                    && !PlatformEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI)))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Not RSN IE, maybe WMM IE!!!\n"));
                    break;                          
                }
                    
                if (((eid_ptr->Eid == IE_WPA) && (PlatformEqualMemory(eid_ptr->Octet, WPA_OUI, 4)))
                    ||(eid_ptr->Eid == IE_WPA2))
                {
                    if ((eid_ptr->Len <= AP_MAX_LEN_OF_RSNIE)  && (eid_ptr->Len > AP_MIN_LEN_OF_RSNIE))
                    {
                        if (!pEntry)
                            return FALSE;

                        DBGPRINT(RT_DEBUG_TRACE, ("Receive IE_WPA : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                            eid_ptr->Octet[0],eid_ptr->Octet[1],eid_ptr->Octet[2],eid_ptr->Octet[3],
                            eid_ptr->Octet[4],eid_ptr->Octet[5],eid_ptr->Octet[6],eid_ptr->Octet[7]));
                        
                        *pRSNLen=eid_ptr->Len;
                        if (!ApWpaCheckMcast(pAd, pPort,eid_ptr, pEntry))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaCheckMcast FAILED !!!\n"));
                                ApWpaDisAssocAction( pAd,pPort, pEntry, REASON_MCIPHER_NOT_VALID);
                            return FALSE;
                        }                        
                        if (!ApWpaCheckUcast(pAd, pPort,eid_ptr, pEntry))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaCheckUcast FAILED !!!\n"));
                                ApWpaDisAssocAction( pAd, pPort,pEntry, REASON_UCIPHER_NOT_VALID);
                            return FALSE;
                        }          
                        if (!ApWpaCheckAuth(pAd, pPort,eid_ptr, pEntry))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("ApWpaCheckAuth FAILED !!!\n"));
                                ApWpaDisAssocAction( pAd, pPort,pEntry, REASON_INVALID_IE);
                            return FALSE;
                        }                        

                        PlatformMoveMemory(RSN, eid_ptr->Octet, eid_ptr->Len);
                    }
                    else
                    {
                        *pRSNLen=0;
                        DBGPRINT(RT_DEBUG_TRACE, ("%s - missing IE_WPA\n", __FUNCTION__));
                        return FALSE;
                    }
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s 2- (%d)\n", __FUNCTION__, eid_ptr->Len));
                    break;
                }
                break;
                
            default:
                break;
        }
        eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
    }

    if ((Sanity&0x3) != 0x03)    
    {
        DBGPRINT(RT_DEBUG_WARN, ("%s - missing mandatory field\n", __FUNCTION__));
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN ApSanityPeerDisassocReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *Reason) 
{
    BOOLEAN         bResult = FALSE;
    PFRAME_802_11   Fr = (PFRAME_802_11)Msg;

    do
    {
        // Skip broadcast/multicast DisAssoc
        if (Fr->Hdr.Addr1[0] & 0x01)
            break;

        // Skip no reason code
        if (MsgLen < (LENGTH_802_11 + 2))
            break;
        
        bResult = TRUE;
        
        COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
        PlatformMoveMemory(Reason, &Fr->Octet[0], 2);
    }while(FALSE);

    return bResult;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN ApSanityPeerDeauthReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *Reason) 
{
    BOOLEAN         bResult = FALSE;
    PFRAME_802_11   Fr = (PFRAME_802_11)Msg;

    do 
    {
        // Skip broadcast/multicast deauth
        if (Fr->Hdr.Addr1[0] & 0x01)
            break;

        // Skip no reason code
        if (MsgLen < (LENGTH_802_11 + 2))
            break;

        bResult = TRUE;

        COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
        PlatformMoveMemory(Reason, &Fr->Octet[0], 2);
    }while(FALSE);

    return bResult;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN ApSanityPeerAuthSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT USHORT *Alg, 
    OUT USHORT *Seq, 
    OUT USHORT *Status, 
    CHAR *ChlgText) 
{
    PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr, &Fr->Hdr.Addr2);
    PlatformMoveMemory(Alg, &Fr->Octet[0], 2);
    PlatformMoveMemory(Seq, &Fr->Octet[2], 2);
    PlatformMoveMemory(Status, &Fr->Octet[4], 2);

    if (*Alg == AUTH_OPEN_SYSTEM) 
    {
        if (*Seq == 1 || *Seq == 2) 
        {
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAuthSanity fail - wrong Seg# (=%d)\n", *Seq));
            return FALSE;
        }
    } 
    else if (*Alg == AUTH_SHARED_KEY) 
    {
        if (*Seq == 1 || *Seq == 4) 
        {
            return TRUE;
        } 
        else if (*Seq == 2 || *Seq == 3) 
        {
            PlatformMoveMemory(ChlgText, &Fr->Octet[8], CIPHER_TEXT_LEN);
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAuthSanity fail - wrong Seg# (=%d)\n", *Seq));
            return FALSE;
        }
    } 
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerAuthSanity fail - wrong algorithm (=%d)\n", *Alg));
        return FALSE;
    }
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN ApSanityPeerProbeReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *SsidLen) 
{
    PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

    // to prevent caller from using garbage output value
    *SsidLen = 0;

    COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);

    if (Fr->Octet[0] != IE_SSID || Fr->Octet[1] > MAX_LEN_OF_SSID || ( Fr->Octet[1] < 0)) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ApSanityPeerProbeReqSanity fail - wrong SSID IE\n"));
        return FALSE;
    } 
    
    *SsidLen = Fr->Octet[1];
    PlatformMoveMemory(Ssid, &Fr->Octet[2], *SsidLen);

    return TRUE;
}


/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN ApSanityMlmeSyncPeerBeaconAndProbeRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT    pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT PUCHAR pBssid, 
    OUT CHAR Ssid[], 
    OUT UCHAR *SsidLen, 
    OUT UCHAR *BssType, 
    OUT USHORT *BeaconPeriod, 
    OUT UCHAR *Channel, 
    OUT LARGE_INTEGER *Timestamp, 
    OUT USHORT *CapabilityInfo, 
    OUT UCHAR Rate[], 
    OUT UCHAR *RateLen,
    OUT UCHAR    *pHtCapabilityLen,
    OUT UCHAR   *pVHtCapabilityLen,
    OUT BOOLEAN *ExtendedRateIeExist,
    OUT UCHAR *Erp)
{
    CHAR                *Ptr;
    PFRAME_802_11       Fr;
    PEID_STRUCT         eid_ptr;
    UCHAR               SubType;
    UCHAR               Sanity;

    // to prevent caller from using garbage output value
    *RateLen = 0;
    *ExtendedRateIeExist = FALSE;
    *pHtCapabilityLen = 0, *pVHtCapabilityLen = 0;
    *Erp = 0;
    
    // Add for 3 necessary EID field check
    Sanity = 0;
    
    Fr = (PFRAME_802_11)Msg;
    
    // get subtype from header
    SubType = (UCHAR)Fr->Hdr.FC.SubType;

    // get Addr2 and BSSID from header
    COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
    COPY_MAC_ADDR(pBssid, &Fr->Hdr.Addr3);
    
    Ptr = Fr->Octet;
    
    // get timestamp from payload and advance the pointer
    PlatformMoveMemory(Timestamp, Ptr, TIMESTAMP_LEN);
    Ptr += TIMESTAMP_LEN;

    // get beacon interval from payload and advance the pointer
    PlatformMoveMemory(BeaconPeriod, Ptr, 2);
    Ptr += 2;

    // get capability info from payload and advance the pointer
    PlatformMoveMemory(CapabilityInfo, Ptr, 2);
    Ptr += 2;
    if (CAP_IS_ESS_ON(*CapabilityInfo)) 
    {
        *BssType = BSS_INFRA;
    } 
    else 
    {
        *BssType = BSS_ADHOC;
    }

    eid_ptr = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while(((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((UCHAR*)Fr + MsgLen))
    {
        switch(eid_ptr->Eid)
        {
            case IE_SSID:
                if(eid_ptr->Len <= MAX_LEN_OF_SSID)
                {
                    PlatformMoveMemory(Ssid, eid_ptr->Octet, eid_ptr->Len);
                    *SsidLen = eid_ptr->Len;
                    Sanity |= 0x1;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_SSID (len=%d)\n", __FUNCTION__, eid_ptr->Len));
                    return FALSE;
                }
                break;

            case IE_SUPP_RATES:
                if(eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(Rate, eid_ptr->Octet, eid_ptr->Len);
                    *RateLen = eid_ptr->Len;
                    Sanity |= 0x2;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_SUPP_RATES (len=%d)\n", __FUNCTION__, eid_ptr->Len));
                    return FALSE;
                }
                break;

            case IE_DS_PARM:
                if(eid_ptr->Len == 1)
                {
                    *Channel = *eid_ptr->Octet;                    
                    Sanity |= 0x4;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_DS_PARM (len=%d)\n", __FUNCTION__, eid_ptr->Len));
                    return FALSE;
                }
                break;
            case IE_HT_CAP:
                if (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (eid_ptr->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
                    {
                        *pHtCapabilityLen = SIZE_HT_CAP_IE; // Nnow we only support 26 bytes.

                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_HT_CAP. pEid->Len = %d\n", __FUNCTION__, eid_ptr->Len));
                    }
                }
                break;

            case IE_VHT_CAP:
                if (eid_ptr->Len >= SIZE_OF_VHT_CAP_IE)
                {
                    *pVHtCapabilityLen = SIZE_OF_VHT_CAP_IE;
                    // todo:
                    //PlatformMoveMemory(pVhtCapability, eid_ptr->Octet, SIZE_OF_VHT_CAP_IE);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_VHT_CAP. \n",__FUNCTION__));
                }
                break;

            case IE_FH_PARM:
            case IE_CF_PARM:
            case IE_IBSS_PARM:
            case IE_TIM:
            case IE_WPA:
                break;

            case IE_EXT_SUPP_RATES:
                // concatenate all extended rates to Rates[] and RateLen
                *ExtendedRateIeExist = TRUE;
                if (eid_ptr->Len + *RateLen <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(&Rate[*RateLen], eid_ptr->Octet, eid_ptr->Len);
                    *RateLen = (*RateLen) + eid_ptr->Len;
                }
                else
                {
                    PlatformMoveMemory(&Rate[*RateLen], eid_ptr->Octet, MAX_LEN_OF_SUPPORTED_RATES - (*RateLen));
                    *RateLen = MAX_LEN_OF_SUPPORTED_RATES;
                }
                break;

            case IE_ERP:
                if (eid_ptr->Len == 1)
                {
                    *Erp = (UCHAR)eid_ptr->Octet[0];
                }
                break;
                
            default:
                DBGPRINT(RT_DEBUG_INFO, ("%s - unrecognized EID = %d\n", __FUNCTION__, eid_ptr->Eid));
                break;
        }
        
        eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
    }

    // For some 11a AP. it did not have the channel EID, patch here
    if (pAd->HwCfg.LatchRfRegs.Channel > 14)
    {
        *Channel = pAd->HwCfg.LatchRfRegs.Channel;
        Sanity |= 0x4;      
    }

    if ((Sanity&0x7) != 0x7)
    {
        DBGPRINT(RT_DEBUG_WARN, ("%s - missing mandatory field)\n", __FUNCTION__));
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}



