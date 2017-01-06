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
    sanity.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    John Chang  2004-09-01      add WMM support
*/
#include "MtConfig.h"

UCHAR   WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
UCHAR   RSN_OUI[] = {0x00, 0x0f, 0xac};
UCHAR   WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};
UCHAR   WME_INFO_ELEM[]  = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR   WME_PARM_ELEM[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR   RALINK_OUI[]  = {0x00, 0x0c, 0x43};
UCHAR   MARVELL_OUI[]  = {0x00, 0x50, 0x43};
UCHAR   EPIGRAM_OUI[]  = {0x00, 0x90, 0x4C};
UCHAR   ATHEROS_OUI[]  = {0x00, 0x03, 0x7F};
UCHAR   BROADCOM_OUI[]  = {0x00, 0x10, 0x18};
UCHAR   PRE_N_HT_OUI[]  = {0x00, 0x90, 0x4c};
UCHAR   SSIDL_OUI[] = {0x00, 0x50, 0xf2, 0x05}; // SSIDL IE
extern UCHAR    P2POUIBYTE[];
extern UCHAR OUI_WPA2_CCMP[];
extern UCHAR OUI_WPA2_TKIP[];
extern UCHAR OUI_WPA_CCMP[];
extern UCHAR OUI_WPA_TKIP[];
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeStartReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen) 
{
    MLME_START_REQ_STRUCT *Info;

    Info = (MLME_START_REQ_STRUCT *)(Msg);

    if (Info->SsidLen > MAX_LEN_OF_SSID)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqSanity fail - wrong SSID length\n"));
        return FALSE;
    }

    *pSsidLen = Info->SsidLen;
    PlatformMoveMemory(Ssid, Info->Ssid, *pSsidLen);

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAssocReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pApAddr, 
    OUT USHORT *pCapabilityInfo, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pListenIntv) 
{
    MLME_ASSOC_REQ_STRUCT *pInfo;

    pInfo = (MLME_ASSOC_REQ_STRUCT *)Msg;
    *pTimeout = pInfo->Timeout;                         // timeout
    COPY_MAC_ADDR(pApAddr, pInfo->Addr);                // AP address
    *pCapabilityInfo = pInfo->CapabilityInfo;           // capability info
    *pListenIntv = pInfo->ListenIntv;

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
BOOLEAN MlmeAuthReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pAlg) 
{
    MLME_AUTH_REQ_STRUCT *pInfo;

    pInfo  = (MLME_AUTH_REQ_STRUCT *)Msg;
    COPY_MAC_ADDR(pAddr, pInfo->Addr);
    *pTimeout = pInfo->Timeout;
    *pAlg = pInfo->Alg;

    if (((*pAlg == AUTH_OPEN_SYSTEM) ||(*pAlg == AUTH_SHARED_KEY)) && 
        ((*pAddr & 0x01) == 0)) 
    {
        return TRUE;
    } 
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAuthReqSanity fail - wrong algorithm\n"));
        return FALSE;
    }
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerAssocRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN VOID *pMsg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pStatus, 
    OUT USHORT *pAid, 
    OUT UCHAR SupRate[], 
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[], 
    OUT UCHAR *pExtRateLen,
    OUT ULONG *pP2PIeLen,
    OUT HT_CAPABILITY_IE        *pHtCapability,
    OUT VHT_CAP_IE           *pVhtCapability,
    OUT VHT_OP_IE            *pVhtOperation,
    OUT ADD_HT_INFO_IE      *pAddHtInfo,    // AP might use this additional ht info IE 
    OUT UCHAR           *pHtCapabilityLen,
    OUT UCHAR           *pAddHtInfoLen,
    OUT UCHAR                *pVhtCapable,
    OUT UCHAR           *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT PEXT_CAP_ELEMT pExtendedCapabilities) 
{
    CHAR          IeType;
    PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
    PEID_STRUCT   pEid;
    ULONG         Length = 0;
    BOOLEAN       bVhtCapabilityCarried = FALSE;
    BOOLEAN       bVhtOperationCarried = FALSE;
    
    *pNewExtChannelOffset = 0xff;
    *pHtCapabilityLen = 0;
    *pAddHtInfoLen = 0;
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    Length += LENGTH_802_11;    

    PlatformMoveMemory(pCapabilityInfo, &pFrame->Octet[0], 2);
    Length += 2;    
    PlatformMoveMemory(pStatus,         &pFrame->Octet[2], 2);
    Length += 2;    
    *pExtRateLen = 0;
    *pP2PIeLen = 0;
    pEdcaParm->bValid = FALSE;

    if (*pStatus != MLME_SUCCESS) 
        return TRUE;

    PlatformMoveMemory(pAid, &pFrame->Octet[4], 2);
    Length += 2;    
    *pAid = le2cpu16(*pAid);

    // TODO: check big endian issue &0x3fff
    *pAid = (*pAid) & 0x3fff; // AID is low 14-bit

    // -- get supported rates from payload and advance the pointer
    IeType = pFrame->Octet[6];
    *pSupRateLen = pFrame->Octet[7];
    if ((IeType != IE_SUPP_RATES) || (*pSupRateLen > MAX_LEN_OF_SUPPORTED_RATES))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s fail - wrong SupportedRates IE\n", __FUNCTION__));
        return FALSE;
    } 
    else 
        PlatformMoveMemory(SupRate, &pFrame->Octet[8], *pSupRateLen);

    Length = Length + 2 + *pSupRateLen;

    // many AP implement proprietary IEs in non-standard order, we'd better
    // tolerate mis-ordered IEs to get best compatibility
    pEid = (PEID_STRUCT) &pFrame->Octet[8 + (*pSupRateLen)];

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)
    {
        switch (pEid->Eid)
        {
            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;
                }
                break;
                
            case IE_HT_CAP:
            case IE_HT_CAP2:
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
                    {
                        PlatformMoveMemory(pHtCapability, pEid->Octet, SIZE_HT_CAP_IE);
                        *pHtCapabilityLen = SIZE_HT_CAP_IE;
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_HT_CAP.\n", __FUNCTION__));
                    }
                }
                break;

            case IE_VHT_CAP:
                if (pEid->Len == SIZE_OF_VHT_CAP_IE)
                {
                    bVhtCapabilityCarried = TRUE;
                    PlatformMoveMemory(pVhtCapability, pEid->Octet, SIZE_OF_VHT_CAP_IE);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_VHT_CAP. \n",__FUNCTION__));
                }
                break;
                
            case IE_VHT_OP:
                if (pEid->Len == SIZE_OF_VHT_OP_IE)
                {
                    bVhtOperationCarried = TRUE;
                    PlatformMoveMemory(pVhtOperation, pEid->Octet, SIZE_OF_VHT_OP_IE);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_VHT_OP. \n",__FUNCTION__));
                }
                break;

            case IE_EXT_CAPABILITY: 
                {
                    if (pEid->Len <= sizeof(EXT_CAP_ELEMT))
                    {
                        PlatformMoveMemory(pExtendedCapabilities, pEid->Octet, pEid->Len);
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("%s: IE_EXT_CAPABILITY, length is too large (%d)\n", 
                            __FUNCTION__, 
                            pEid->Len));
                    }
                }
                break;

            case IE_ADD_HT:
            case IE_ADD_HT2:
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (pEid->Len >= sizeof(ADD_HT_INFO_IE))                
                    {
                        // This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only
                        // copy first sizeof(ADD_HT_INFO_IE)
                        PlatformMoveMemory(pAddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
                        *pAddHtInfoLen = SIZE_ADD_HT_INFO_IE;
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_ADD_HT.\n", __FUNCTION__));
                    }
                }
                break;

            case IE_NEW_EXT_CHA_OFFSET:
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (pEid->Len == 1)
                    {
                        *pNewExtChannelOffset = pEid->Octet[0];
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_NEW_EXT_CHA_OFFSET.\n", __FUNCTION__));
                    }
                }
                break;

            case IE_VENDOR_SPECIFIC:
                // handle WME PARAMTER ELEMENT
                if (PlatformEqualMemory(pEid->Octet, P2POUIBYTE, 4))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Get P2P IE = %x\n", __FUNCTION__, pEid->Len));
                    *pP2PIeLen = pEid->Len;
                }
                if (PlatformEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
                {
                    PUCHAR  ptr;
                    int     i;

                    // parsing EDCA parameters
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
                    //pEdcaParm->bMoreDataAck    = FALSE; // pEid->Octet[0] & 0x80;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
                    ptr = &pEid->Octet[8];
                    for (i = 0; i < 4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }

                break;

#if 0               
            case IE_EDCA_PARAMETER:
                if (pEid->Len == 18)
                {
                    PUCHAR ptr;
                    int i;
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = pEid->Octet[0] & 0x40;
//                  pEdcaParm->bMoreDataAck    = pEid->Octet[0] & 0x80;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[0] & 0x0f;
                    ptr = &pEid->Octet[2];
                    for (i=0; i<4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }
                break;
#endif
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("%s - ignore unrecognized EID = %d\n", __FUNCTION__, pEid->Eid));
                break;
        }

        Length = Length + 2 + pEid->Len;
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
    }

    // VhtCapable = TRUE only if VHT_CAP_IE and VHT_OP_IE are present
    *pVhtCapable = bVhtCapabilityCarried & bVhtOperationCarried;

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDisassocSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason) 
{
    BOOLEAN         bResult = FALSE;
    PFRAME_802_11   pFrame = (PFRAME_802_11)Msg;
    PMP_PORT  pPort = pAd->PortList[pAd->ucActivePortNum];


    do
    {
        // Skip broadcast/multicast disassociation
        if (
            // 11w have broadcast disassoc frame
            !MFP_ON(pAd,pPort) &&
            (pFrame->Hdr.Addr1[0] & 0x01))
            break;

        // Skip no reason code
        if (MsgLen < (LENGTH_802_11 + 2))
            break;

        bResult = TRUE;

        COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
        PlatformMoveMemory(pReason, &pFrame->Octet[0], 2);

    } while (FALSE);

    return bResult;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDeauthSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason) 
{
    BOOLEAN bResult = FALSE;
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
    PMP_PORT  pPort = pAd->PortList[pAd->ucActivePortNum];

    do
    {
        // Skip broadcast/multicast disassociation
        if (
            // 11w have broadcast disassoc frame
            !MFP_ON(pAd,pPort) &&
            (pFrame->Hdr.Addr1[0] & 0x01))
            break;

        // Skip no reason code
        if (MsgLen < (LENGTH_802_11 + 2))
            break;

        bResult = TRUE;

        COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
        PlatformMoveMemory(pReason, &pFrame->Octet[0], 2);

    } while (FALSE);

    return bResult;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerAuthSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT USHORT *pAlg, 
    OUT USHORT *pSeq, 
    OUT USHORT *pStatus, 
    CHAR *pChlgText) 
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr,   pFrame->Hdr.Addr2);
    PlatformMoveMemory(pAlg,    &pFrame->Octet[0], 2);
    PlatformMoveMemory(pSeq,    &pFrame->Octet[2], 2);
    PlatformMoveMemory(pStatus, &pFrame->Octet[4], 2);

    //
    // Change to AUTH_MODE_OPEN(0) or AUTH_MODE_KEY(1)
    // We can't use Ralink802_11AuthModeOpen from now on.
    // 
    if (*pAlg == AUTH_OPEN_SYSTEM) 
    {
        if (*pSeq == 1 || *pSeq == 2) 
        {
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong Seg#(=%d)\n", *pSeq));
            return FALSE;
        }
    } 
    else if (*pAlg == AUTH_SHARED_KEY) 
    {
        if (*pSeq == 1 || *pSeq == 4) 
        {
            return TRUE;
        } 
        else if (*pSeq == 2 || *pSeq == 3) 
        {
            PlatformMoveMemory(pChlgText, &pFrame->Octet[8], CIPHER_TEXT_LEN);
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong Seg#(=%d)\n", *pSeq));
            return FALSE;
        }
    } 
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong algorithm(=%d)\n", *pAlg));
        return FALSE;
    }
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerProbeReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen)
{
    UCHAR         Idx;
    UCHAR         RateLen;
    CHAR          IeType;
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

    if ((pFrame->Octet[0] != IE_SSID) || (pFrame->Octet[1] > MAX_LEN_OF_SSID) || (pFrame->Octet[1]  < 0)) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerProbeReqSanity fail - wrong SSID IE(Type=%d,Len=%d)\n",pFrame->Octet[0],pFrame->Octet[1]));
        return FALSE;
    } 

    *pSsidLen = pFrame->Octet[1];
    PlatformMoveMemory(Ssid, &pFrame->Octet[2], *pSsidLen);

    Idx = *pSsidLen + 2;

    // -- get supported rates from payload and advance the pointer
    IeType = pFrame->Octet[Idx];
    RateLen = pFrame->Octet[Idx + 1];
    if (IeType != IE_SUPP_RATES) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerProbeReqSanity fail - wrong SupportRates IE(Type=%d,Len=%d)\n",pFrame->Octet[Idx],pFrame->Octet[Idx+1]));
        return FALSE;
    }
    else
    {
        if ((pAd->StaCfg.AdhocMode == 2) && (RateLen < 8))
            return (FALSE);
    }

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeSyncPeerBeaconAndProbeRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT PUCHAR pBssid, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT UCHAR *pBssType, 
    OUT USHORT *pBeaconPeriod, 
    OUT UCHAR *pChannel, 
    OUT UCHAR *pNewChannel,
    OUT ULONGLONG *pTimestamp, 
    OUT CF_PARM *pCfParm, 
    OUT USHORT *pAtimWin, 
    OUT USHORT *pCapabilityInfo, 
    OUT UCHAR *pErp,
    OUT UCHAR *pDtimCount, 
    OUT UCHAR *pDtimPeriod, 
    OUT UCHAR *pBcastFlag, 
    OUT UCHAR *pMessageToMe, 
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT PEDCA_PARM       pEdcaParm,
    OUT PQBSS_LOAD_PARM  pQbssLoad,
    OUT PQOS_CAPABILITY_PARM pQosCapability,
    OUT ULONG *pRalinkIe,
    OUT UCHAR        *pHtCapabilityLen,
    OUT UCHAR        *pPreNHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT BOOLEAN    *pVhtCapable,
    OUT VHT_CAP_IE *pVhtCapability,
    OUT VHT_OP_IE *pVhtOperation,
    OUT UCHAR        *AddHtInfoLen,
    OUT ADD_HT_INFO_IE *AddHtInfo,
    OUT EXT_CAP_ELEMT               *pExtCap,   // this is the extened capibility IE  
    OUT UCHAR *NewExtChannelOffset,     // Ht extension channel offset(above or below)
    OUT USHORT *WSCInfoAtBeaconsLen,
    OUT PUCHAR WSCInfoAtBeacons,
    OUT USHORT *WSCInfoAtProbeRspLen,
    OUT PUCHAR WSCInfoAtProbeRsp,
    OUT PSSIDL_IE pSSIDL_VIE,
    OUT PUCHAR pMaxTxPowerLevel, 
    OUT PUCHAR pLocalPowerConstraint,
    OUT UCHAR *bConnectedToCiscoAp,
    OUT PBOOLEAN bHasOperatingModeField,
    OUT POPERATING_MODE_FIELD pOperatingModeField,
    OUT USHORT *LengthVIE,  
    OUT PNDIS_802_11_VARIABLE_IEs pVIE) 
{
    PMLME_QUEUE_ELEM    pMlmeQueueElem = NULL;
    CHAR                *Ptr, TimLen;
    PFRAME_802_11       pFrame;
    PEID_STRUCT         pEid;
    UCHAR               SubType;
    UCHAR               Sanity;
    ULONG               Length = 0; 
    BOOLEAN             IsBeacon = FALSE;
    BOOLEAN             bSaveVariableIE = TRUE;
    BOOLEAN bHasCountryIE = FALSE, bHasDSParametSetIE = FALSE;
    PCOUNTRY_IE pCountryIE = NULL;
    BOOLEAN     bVhtCapabilityCarried = FALSE;
    BOOLEAN     bVhtOperationCarried = FALSE;

    // Add for 3 necessary EID field check
    Sanity = 0;

    *pAtimWin = 0;
    *pErp = 0;  
    *pDtimCount = 0;
    *pDtimPeriod = 0;
    *pBcastFlag = 0;
    *pMessageToMe = 0;
    *pExtRateLen = 0;
    *LengthVIE = 0;                 // Set the length of VIE to init value 0
    *pHtCapabilityLen = 0;                  // Set the length of VIE to init value 0
    *pPreNHtCapabilityLen = 0;                  // Set the length of VIE to init value 0
    *AddHtInfoLen = 0;                  // Set the length of VIE to init value 0
    *pRalinkIe = 0;
    pExtCap->DoubleWord = 0;
    *pNewChannel = 0;
    *NewExtChannelOffset = 0xff;    //Default 0xff means no such IE
    *WSCInfoAtBeaconsLen = 0;
    *WSCInfoAtProbeRspLen = 0;
    pCfParm->bValid = FALSE;        // default: no IE_CF found
    pQbssLoad->bValid = FALSE;      // default: no IE_QBSS_LOAD found
    pEdcaParm->bValid = FALSE;      // default: no IE_EDCA_PARAMETER found
    pQosCapability->bValid = FALSE; // default: no IE_QOS_CAPABILITY found
    *bConnectedToCiscoAp=FALSE;
    *pMaxTxPowerLevel = MDSM_ABSENT_COUNTRY_IE;
    *pLocalPowerConstraint = MDSM_ABSENT_POWER_CONSTRAINT_IE;
    *bHasOperatingModeField = FALSE;

    pFrame = (PFRAME_802_11)Msg;

    // get subtype from header
    SubType = (UCHAR)pFrame->Hdr.FC.SubType;
    IsBeacon = (SubType == SUBTYPE_BEACON);

    // get Addr2 and BSSID from header
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    COPY_MAC_ADDR(pBssid, pFrame->Hdr.Addr3);

    Ptr = pFrame->Octet;
    Length += LENGTH_802_11;

    // get timestamp from payload and advance the pointer
    PlatformMoveMemory(pTimestamp, Ptr, TIMESTAMP_LEN);
    Ptr += TIMESTAMP_LEN;
    Length += TIMESTAMP_LEN;

    // get beacon interval from payload and advance the pointer
    PlatformMoveMemory(pBeaconPeriod, Ptr, 2);
    Ptr += 2;
    Length += 2;

    // get capability info from payload and advance the pointer
    PlatformMoveMemory(pCapabilityInfo, Ptr, 2);
    Ptr += 2;
    Length += 2;

    if (CAP_IS_IBSS_ON(*pCapabilityInfo))
        *pBssType = BSS_ADHOC;
    else
        *pBssType = BSS_INFRA;

    pEid = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)    
    {
        //
        // Secure copy VIE to VarIE[MAX_VIE_LEN] didn't overflow.
        //
        if ((*LengthVIE + pEid->Len + 2) >= MAX_VIE_LEN)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Variable IEs out of resource [len(=%d) > MAX_VIE_LEN(=%d)]\n",
                    __FUNCTION__,
                    (*LengthVIE + pEid->Len + 2),
                    MAX_VIE_LEN));
            break;
        }
    
        switch(pEid->Eid)
        {
            case IE_SSID:
                // Already has one SSID EID in this beacon, ignore the second one
                if (Sanity & 0x1)
                    break;              
                if(pEid->Len <= MAX_LEN_OF_SSID)
                {
                    PlatformMoveMemory(Ssid, pEid->Octet, pEid->Len);
                    *pSsidLen = pEid->Len;
                    Sanity |= 0x1;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_SSID (len=%d)\n", __FUNCTION__, pEid->Len));
                    return FALSE;
                }
                break;

            case IE_SUPP_RATES:
                if(pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    Sanity |= 0x2;
                    PlatformMoveMemory(SupRate, pEid->Octet, pEid->Len);
                    *pSupRateLen = pEid->Len;

                    // TODO: 2004-09-14 not a good design here, cause it exclude extra rates 
                    // from ScanTab. We should report as is. And filter out unsupported
                    // rates in MlmeAux.
                    // Check against the supported rates
                    // RTMPCheckRates(pAd, SupRate, pSupRateLen);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_SUPP_RATES (len=%d)\n", __FUNCTION__, pEid->Len));
                    return FALSE;
                }
                break;

            case IE_HT_CAP:
            
                if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
                {
                    PlatformMoveMemory(pHtCapability, pEid->Octet, sizeof(HT_CAPABILITY_IE));
                    *pHtCapabilityLen = SIZE_HT_CAP_IE; // Nnow we only support 26 bytes.
                    *pPreNHtCapabilityLen = 0;
                    Sanity |= 0x10;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_HT_CAP. pEid->Len = %d\n", __FUNCTION__, pEid->Len));
                }           
                break;

            case IE_VHT_CAP: // length = 12, not extensible
                if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                {
                    bVhtCapabilityCarried = TRUE;
                    // check length
                    if (pEid->Len == (SIZE_OF_VHT_CAP_IE))
                    {
                        PlatformMoveMemory(pVhtCapability, pEid->Octet, sizeof(VHT_CAP_IE));
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN,("%s - wrong IE_VHT_CAP. pEid->Len = %d\n", __FUNCTION__, pEid->Len));
                    }
                }           
                break;

            case IE_VHT_OP: // length = 5, not extensible
                if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                {
                    bVhtOperationCarried = TRUE;
                    // check length
                    if (pEid->Len == (SIZE_OF_VHT_OP_IE))
                    {
                        PlatformMoveMemory(pVhtOperation, pEid->Octet, sizeof(VHT_OP_IE));
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN,("%s - wrong IE_VHT_OP. pEid->Len = %d\n", __FUNCTION__, pEid->Len));
                    }
                }
                break;

            case IE_ADD_HT:
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (pEid->Len >= sizeof(ADD_HT_INFO_IE))                
                    {
                        // This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only
                        // copy first sizeof(ADD_HT_INFO_IE)
                        PlatformMoveMemory(AddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
                        *AddHtInfoLen = SIZE_ADD_HT_INFO_IE;

                        if (pAd->HwCfg.LatchRfRegs.Channel > 14)
                        {
                            *pChannel =AddHtInfo->ControlChan;
                            Sanity |= 0x4;
                        }
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_WARN, ("%s - wrong IE_ADD_HT. \n", __FUNCTION__));
                    }
                }
                break;

            case IE_NEW_EXT_CHA_OFFSET:
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (pEid->Len == 1)
                    {
                        *NewExtChannelOffset = pEid->Octet[0];
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_INFO, ("%s - wrong IE_NEW_EXT_CHA_OFFSET. \n", __FUNCTION__));
                    }
                }
                break;

            case IE_FH_PARM:
                DBGPRINT(RT_DEBUG_TRACE, ("%s (IE_FH_PARM) \n", __FUNCTION__));
                break;

            case IE_DS_PARM:
                if(pEid->Len == 1)
                {
                    *pChannel = *pEid->Octet;
                    if (ChannelSanity(pAd, *pChannel) == 0)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_DS_PARM (ch=%d)\n", __FUNCTION__, *pChannel));
                        return FALSE;
                    }
                    Sanity |= 0x4;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_DS_PARM (len=%d)\n", __FUNCTION__, pEid->Len));
                    return FALSE;
                }
                bHasDSParametSetIE = TRUE;
                break;

            case IE_CF_PARM:
                if(pEid->Len == 6)
                {
                    pCfParm->bValid = TRUE;
                    pCfParm->CfpCount = pEid->Octet[0];
                    pCfParm->CfpPeriod = pEid->Octet[1];
                    pCfParm->CfpMaxDuration =(UCHAR) pEid->Octet[2] + 256 * (UCHAR)pEid->Octet[3];
                    pCfParm->CfpDurRemaining = (UCHAR) pEid->Octet[4] + 256 * (UCHAR)pEid->Octet[5];
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_CF_PARM\n", __FUNCTION__));
                    return FALSE;
                }
                break;

            case IE_IBSS_PARM:
                if(pEid->Len == 2)
                {
                    PlatformMoveMemory(pAtimWin, pEid->Octet, pEid->Len);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_IBSS_PARM\n", __FUNCTION__));
                    return FALSE;
                }
                break;

            case IE_TIM:
#ifdef MULTI_CHANNEL_SUPPORT
                if((INFRA_ON(pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort)) && SubType == SUBTYPE_BEACON)
                {
                    USHORT      AidValue;

                    AidValue = pPort->PsPollFrame.Aid & 0x3fff;
                    
                    // Check INFRA and P2pMs CLI if BSSID match
                    if (/*PlatformEqualMemory(pBssid, pPort->CommonCfg.Bssid, MAC_ADDR_LEN) ||*/PlatformEqualMemory(pBssid, pPort->PortCfg.Bssid, MAC_ADDR_LEN))      
                    {
                        GetTimBit((PUCHAR)pEid, AidValue, &TimLen, pBcastFlag, pDtimCount, pDtimPeriod, pMessageToMe);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: Beacon from (%x:%x:%x:%x:%x:%x) has data to me(%x:%x:%x:%x:%x:%x)\n",
                                                    __FUNCTION__,
                                                    pFrame->Hdr.Addr3[0], pFrame->Hdr.Addr3[1], pFrame->Hdr.Addr3[2], pFrame->Hdr.Addr3[3], pFrame->Hdr.Addr3[4], pFrame->Hdr.Addr3[5],
                                                    pFrame->Hdr.Addr1[0], pFrame->Hdr.Addr1[1], pFrame->Hdr.Addr1[2], pFrame->Hdr.Addr1[3], pFrame->Hdr.Addr1[4], pFrame->Hdr.Addr1[5]));
                    }
                }
#else
                if(INFRA_ON(pPort) && SubType == SUBTYPE_BEACON)
                {
                    USHORT      AidValue;

                    // For checking the Aid value, we need to bypass the first two bits of the Aid value. Just in case, we forget to remove the first two bits of Aid.
                    AidValue = pPort->PsPollFrame.Aid & 0x3fff;
                
                    GetTimBit((PUCHAR)pEid, AidValue, &TimLen, pBcastFlag, pDtimCount, pDtimPeriod, pMessageToMe);
                }
#endif /*MULTI_CHANNEL_SUPPORT*/
                break;

            case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
                if(pEid->Len == 3)
                {
                    *pNewChannel = pEid->Octet[1];  //extract new channel number
                }
                break;

            // New for WPA
            // CCX has the same IE, we need to parse that too
            // Wifi WMM use the same IE vale, need to parse that too
            // case IE_WPA:
            case IE_VENDOR_SPECIFIC:
                // Check the OUI version, filter out non-standard usage
                if (PlatformEqualMemory(pEid->Octet, RALINK_OUI, 3) && (pEid->Len == 7))
                {
                    if (pEid->Octet[3] != 0)
                        *pRalinkIe = pEid->Octet[3];
                    else
                        *pRalinkIe = 0xf0000000; // Set to non-zero value (can't set bit0-2) to represent this is Ralink Chip. So at linkup, we will set ralinkchip flag.
                }
                // Check Broadcom/Atheros 802.11n OUI version, for HT Capability IE. 
                // This HT IE is before IEEE draft set HT IE value.2006-09-28 by Jan. 
                else if ( (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && PlatformEqualMemory(pEid->Octet, PRE_N_HT_OUI, 3) && (pEid->Len >= 4))
                {
                    if ((pEid->Octet[3] == OUI_PREN_HT_CAP) && (pEid->Len >= 30) && (*pHtCapabilityLen == 0))
                    {
                        PlatformMoveMemory(pHtCapability, &pEid->Octet[4], sizeof(HT_CAPABILITY_IE));
                        *pPreNHtCapabilityLen = SIZE_HT_CAP_IE; // Nnow we only support 26 bytes.
                    }
                    if ((pEid->Octet[3] == OUI_PREN_ADD_HT) && (pEid->Len >= 26) && (*AddHtInfoLen == 0))
                    {
                        PlatformMoveMemory(AddHtInfo, &pEid->Octet[4], sizeof(ADD_HT_INFO_IE));
                        *AddHtInfoLen = SIZE_ADD_HT_INFO_IE;    // Nnow we only support 26 bytes.
                    }
                }
                else if (PlatformEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
                {
                    PUCHAR ptr;
                    int i;

                    // parsing EDCA parameters
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
                    ptr = &pEid->Octet[8];
                    for (i = 0; i < 4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }
                else if (PlatformEqualMemory(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7))
                {
                    // parsing EDCA parameters
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;

                    // use default EDCA parameter
                    pEdcaParm->bACM[QID_AC_BE]  = 0;
                    pEdcaParm->Aifsn[QID_AC_BE] = 3;
                    pEdcaParm->Cwmin[QID_AC_BE] = CW_MIN_IN_BITS;
                    pEdcaParm->Cwmax[QID_AC_BE] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_BE]  = 0;

                    pEdcaParm->bACM[QID_AC_BK]  = 0;
                    pEdcaParm->Aifsn[QID_AC_BK] = 7;
                    pEdcaParm->Cwmin[QID_AC_BK] = CW_MIN_IN_BITS;
                    pEdcaParm->Cwmax[QID_AC_BK] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_BK]  = 0;

                    pEdcaParm->bACM[QID_AC_VI]  = 0;
                    pEdcaParm->Aifsn[QID_AC_VI] = 2;
                    pEdcaParm->Cwmin[QID_AC_VI] = CW_MIN_IN_BITS-1;
                    pEdcaParm->Cwmax[QID_AC_VI] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_VI]  = AC2_DEF_TXOP; // AC_VI: 96*32us ~= 3ms

                    pEdcaParm->bACM[QID_AC_VO]  = 0;
                    pEdcaParm->Aifsn[QID_AC_VO] = 2;
                    pEdcaParm->Cwmin[QID_AC_VO] = CW_MIN_IN_BITS-2;
                    pEdcaParm->Cwmax[QID_AC_VO] = CW_MAX_IN_BITS-1;
                    pEdcaParm->Txop[QID_AC_VO]  = AC3_DEF_TXOP; // AC_VO: 48*32us ~= 1.5ms
                }
                else if (PlatformEqualMemory(pEid->Octet, SSIDL_OUI, 4))
                {
                    if ((pEid->Len==5) || (pEid->Octet[5] == 0))    // Count of the SSIDL
                    {
                        // ignore this SSIDL, since there's no SSID Elem included
                    }
                    else
                    {
                        if (pEid->Len >= 254)   // max is 256
                            pEid->Len = 236;

                        // copy the SSIDL VIE (for MlmeSyncMlmeSyncPeerBeaconAtScanAction) for doing BssTableSetEntry
                        PlatformMoveMemory(&pSSIDL_VIE->ElementID, &pEid->Eid, pEid->Len + 2);
                    }
                }
                else if (PlatformEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len >= 4) && (pEid->Len < MAX_VIE_LEN))
                {
                    //
                    // Save WPS IE from Probe Request And Beacons
                    //
                    if (IsBeacon)
                    {
                        // WSCInfoAtBeacons can't be NULL
                        if (WSCInfoAtBeacons != NULL)
                        {
                            if(*WSCInfoAtBeaconsLen == 0)
                            {
                                PlatformMoveMemory(WSCInfoAtBeacons, &pEid->Eid, pEid->Len + 2);
                                *WSCInfoAtBeaconsLen = pEid->Len + 2;
                            }
                            else
                            {
                                PlatformMoveMemory(WSCInfoAtBeacons + *WSCInfoAtBeaconsLen, &pEid->Octet + 4, pEid->Len - 4);
                                *WSCInfoAtBeaconsLen += (pEid->Len - 4);
                            }
                        }
                    }
                    else
                    {
                        // WSCInfoAtProbeRsp can't be NULL
                        if (WSCInfoAtProbeRsp != NULL)
                        {
                            if(*WSCInfoAtProbeRspLen == 0)
                            {
                                PlatformMoveMemory(WSCInfoAtProbeRsp, &pEid->Eid, pEid->Len + 2);
                                *WSCInfoAtProbeRspLen = pEid->Len + 2;
                            }
                            else
                            {
                                PlatformMoveMemory(WSCInfoAtProbeRsp + *WSCInfoAtProbeRspLen, &pEid->Octet + 4, pEid->Len - 4);
                                *WSCInfoAtProbeRspLen += (pEid->Len - 4);
                            }
                        }
                    }
                    bSaveVariableIE = FALSE;
                }

                DBGPRINT(RT_DEBUG_INFO, ("%s - Receive IE_WPA\n", __FUNCTION__));
                break;

            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;

                    // TODO: 2004-09-14 not a good design here, cause it exclude extra rates 
                    // from ScanTab. We should report as is. And filter out unsupported
                    // rates in MlmeAux.
                    // Check against the supported rates
                    // RTMPCheckRates(pAd, ExtRate, pExtRateLen);
                }
                break;

            case IE_EXT_CAPABILITY:
                if (pEid->Len >= 1)
                {
                    PlatformMoveMemory(pExtCap, pEid->Octet, pEid->Len);
                }
                break;
        
            case IE_ERP:
                if (pEid->Len == 1)
                {
                    *pErp = (UCHAR)pEid->Octet[0];
                }
                break;
        
            case IE_VHT_OPERATING_MODE_NOTIFICATION: 
                {
                    if (pEid->Len == sizeof (OPERATING_MODE_FIELD))
                    {
                        *bHasOperatingModeField = TRUE;
                        PlatformMoveMemory(pOperatingModeField, pEid->Octet, sizeof(OPERATING_MODE_FIELD));
                    }
                }
                break;

                
            // WPA2 & 802.11i RSN
            case IE_RSN:
                // Parse RSN if necessary.
                DBGPRINT(RT_DEBUG_INFO, ("IE_RSN length = %d\n", pEid->Len));
                break;              

            case IE_QBSS_LOAD:
                if (pEid->Len == 5)
                {
                    pQbssLoad->bValid = TRUE;
                    pQbssLoad->StaNum = (UCHAR)pEid->Octet[0] + (UCHAR)pEid->Octet[1] * 256;
                    pQbssLoad->ChannelUtilization = pEid->Octet[2];
                    pQbssLoad->RemainingAdmissionControl = (UCHAR)pEid->Octet[3] + (UCHAR)pEid->Octet[4] * 256;
                    DBGPRINT(RT_DEBUG_INFO, ("[%02x-%02x-%02x-%02x-%02x-%02x] QBSS - Load, StaNum = %d, CHUtil=%d, AC=%d \n", 
                            pBssid[0], pBssid[1], pBssid[2], pBssid[3], pBssid[4], pBssid[5], 
                            pQbssLoad->StaNum, pQbssLoad->ChannelUtilization, pQbssLoad->RemainingAdmissionControl));
                }
                break;
            case IE_DOT11DH_COUNTRY:
                {
                if (pEid->Len < MINIMUM_LENGTH_OF_COUNTRY_IE)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("TPC: %s: Country IE is too short, pEid->Length = %d\n", __FUNCTION__, pEid->Len));
                }
                else
                {       
                    bHasCountryIE = TRUE;
                    pCountryIE = (PCOUNTRY_IE)(pEid);

                    DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: Country IE, pCountryIE->CountryString = %c%c%c\n", 
                        __FUNCTION__, 
                        pCountryIE->CountryString[0], pCountryIE->CountryString[1], pCountryIE->CountryString[2]));
                }
                }
                break;

            case IE_DOT11DH_POWER_CONSTRAINT:
                {
                    PPOWER_CONSTRAINT_IE pPowerConstraintIE = (PPOWER_CONSTRAINT_IE)(pEid);

                    *pLocalPowerConstraint = pPowerConstraintIE->LocalPowerConstraint;
                    
                    DBGPRINT(RT_DEBUG_INFO, ("@@: TPC: %s: Power constraint IE, pPowerConstraintIE->LocalPowerConstraint = %d dB\n", __FUNCTION__, pPowerConstraintIE->LocalPowerConstraint));
                }
                break;

            default:
                DBGPRINT(RT_DEBUG_INFO, ("%s - unrecognized EID = %d\n", __FUNCTION__, pEid->Eid));
                break;
        }
        
        //
        *pVhtCapable = bVhtCapabilityCarried & bVhtOperationCarried;

        //
        // We don't wan to backup SSID
        //
        if ((pVIE != NULL) && (pEid->Eid != IE_SSID) && bSaveVariableIE)
        {
            PlatformMoveMemory(Add2Ptr(pVIE, *LengthVIE) /*Ptr + *LengthVIE*/, &pEid->Eid, pEid->Len + 2);
            *LengthVIE += (pEid->Len + 2);
        }

        Length = Length + 2 + pEid->Len;  // Eid[1] + Len[1]+ content[Len]  
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
        bSaveVariableIE = TRUE;
    }

    //
    // Search the maximum transmit power level for the desired channel
    //
    if ((bHasDSParametSetIE == TRUE) && (bHasCountryIE == TRUE))
    {
        UCHAR index = 0;
        UCHAR lowestChannel = 0, highestChannel = 0;
        PCOUNTRY_TRIPLET pCountryTriplet = NULL;
        UCHAR NumberOfSubbands = ((pCountryIE->Length - LENGTH_OF_COUNTRY_STRING_FIELD) / LENGTH_OF_COUNTRY_TRIPLET_FIELD);
        PUCHAR pData = NULL;

        DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: Search the maximum transmit power level for the desired channel\n", __FUNCTION__));

        pData = (PUCHAR)(&pCountryIE->CountryTriplet); // First country triplet
        for (index = 0; index < NumberOfSubbands; index++, pCountryTriplet += 1)
        {
            pCountryTriplet = (PCOUNTRY_TRIPLET)(pData);
            lowestChannel = pCountryTriplet->MlmeSyncFirstChannelNumber;

            if (pCountryTriplet->MlmeSyncFirstChannelNumber <= 14) // 2.4 GHz
            {
                highestChannel = pCountryTriplet->MlmeSyncFirstChannelNumber + ((pCountryTriplet->NumberOfChannels - 1) * MSDM_2_4G_CHANNEL_GAP);
            }
            else // 5 GHz
            {
                highestChannel = pCountryTriplet->MlmeSyncFirstChannelNumber + ((pCountryTriplet->NumberOfChannels - 1) * MSDM_5G_CHANNEL_GAP);
            }
            
            DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: (BSSID = %02X:%02X:%02X:%02X:%02X:%02X) lowestChannel = %d, highestChannel = %d, *pChannel = %d\n", 
                __FUNCTION__, 
                pBssid[0], pBssid[1], pBssid[2], pBssid[3], pBssid[4], pBssid[5], 
                lowestChannel, highestChannel, *pChannel));

            // Channel boundary verification
            if ((*pChannel >= lowestChannel) && (*pChannel <= highestChannel))
            {
                *pMaxTxPowerLevel = pCountryTriplet->MaxTxPowerLevel;
                DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: The maximum transmit power level = %d dBm (channel = %d)\n", __FUNCTION__, pCountryTriplet->MaxTxPowerLevel, *pChannel));
                break;
            }

            if (index < (NumberOfSubbands - 1))
            {
                pData += sizeof(COUNTRY_TRIPLET); // Shift to the next country triplet
            }
        }
    }
    
    // For some 11a AP. it did not have the channel EID, patch here
    if ((pAd->HwCfg.LatchRfRegs.Channel > 14) && ((Sanity & 0x04) == 0))
    {
        pMlmeQueueElem = CONTAINING_RECORD(Msg, MLME_QUEUE_ELEM, Msg);
        if(pPort->SoftAP.bAPStart || pPort->P2PCfg.bGOStart)
        {           
            if((pPort->BBPCurrentBW == BW_40) &&  (pMlmeQueueElem->Channel == pPort->CentralChannel)) 
            {
                *pChannel = pPort->Channel; 
            }
            else
            {
                *pChannel = pMlmeQueueElem->Channel;    
            }
        }
        else
        {
        
            *pChannel = pMlmeQueueElem->Channel;
        }
        Sanity |= 0x4;      
    }
    
    // Ignore 5G AdHoc beacon if 5G Adhoc is disabled
    if ((pAd->StaCfg.bAdhoc5GDisable) && (*pBssType == BSS_ADHOC))
    {
        if (IS_P2P_FINDING(pPort))
        {
            // P2P's probe response may carry Bsstype=Adhoc. should ignore the channel ban bcz it's not adhoc.
        }
        else if ((*pChannel > 14) || (*pNewChannel > 14))
            return FALSE;
    }

    if ((Sanity&0x7) != 0x7)
    {
        DBGPRINT(RT_DEBUG_WARN, ("%s - missing field, Sanity=0x%02x\n", __FUNCTION__, Sanity));
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
        MlmeSyncPeerBeaconAndProbeRspSanity function is too long. So the 2nd Sanity check function is for all IE that will be added in the future
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeSyncPeerBeaconAndProbeRspSanity2(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    IN OVERLAP_BSS_SCAN_IE *BssScan,
    OUT UCHAR   *RegClass) 
{
    CHAR                *Ptr;
    PFRAME_802_11       pFrame;
    PEID_STRUCT         pEid;
    ULONG               Length = 0; 
    BOOLEAN             brc;

    pFrame = (PFRAME_802_11)Msg;

    *RegClass = 0;
    Ptr = pFrame->Octet;
    Length += LENGTH_802_11;

    // get timestamp from payload and advance the pointer
    Ptr += TIMESTAMP_LEN;
    Length += TIMESTAMP_LEN;

    // get beacon interval from payload and advance the pointer
    Ptr += 2;
    Length += 2;

    // get capability info from payload and advance the pointer
    Ptr += 2;
    Length += 2;

    pEid = (PEID_STRUCT) Ptr;
    brc = FALSE;
    
    PlatformZeroMemory(BssScan, sizeof(OVERLAP_BSS_SCAN_IE));
    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)    
    {   
        switch(pEid->Eid)
        {
            case IE_SUPP_REG_CLASS:
                if(pEid->Len > 0)
                {
                    *RegClass = *pEid->Octet;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_SUPP_REG_CLASS (len=%d)\n", __FUNCTION__, pEid->Len));
                }
                break;
            case IE_OVERLAPBSS_SCAN_PARM:
                if (pEid->Len == sizeof(OVERLAP_BSS_SCAN_IE))
                {
                    brc = TRUE;
                    PlatformMoveMemory(BssScan, pEid->Octet, sizeof(OVERLAP_BSS_SCAN_IE));
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - wrong IE_OVERLAPBSS_SCAN_PARM (len=%d)\n", __FUNCTION__, pEid->Len));
                }
                break;
            case IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT:
                DBGPRINT(RT_DEBUG_TRACE, ("%s - IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT\n", __FUNCTION__));
                break;              
        }
        Length = Length + 2 + pEid->Len;  // Eid[1] + Len[1]+ content[Len]  
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }

    return brc;
}


BOOLEAN PeerAddBAReqActionSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2)
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
    PFRAME_ADDBA_REQ pAddFrame;
    
    pAddFrame = (PFRAME_ADDBA_REQ)(pMsg);
    if (MsgLen < (sizeof(FRAME_ADDBA_REQ)))
    {
        DBGPRINT(RT_DEBUG_ERROR,("MlmeEnqueueForRecv: ADDBA Request frame length size = %d incorrect\n", MsgLen));
        return FALSE;
    }
    // we support immediate BA.
    if (pAddFrame->BaParm.BAPolicy != IMMED_BA)
    {
        DBGPRINT(RT_DEBUG_ERROR,("MlmeEnqueueForRecv: ADDBA Request Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy));
        DBGPRINT(RT_DEBUG_ERROR,("ADDBA Request. tid=%x, Bufsize=%x, AMSDU=%x \n", pAddFrame->BaParm.TID, pAddFrame->BaParm.BufSize, pAddFrame->BaParm.AMSDU));
        return FALSE;
    }

    if (pAddFrame->BaParm.TID &0xfff0)
    {
        DBGPRINT(RT_DEBUG_ERROR,("MlmeEnqueueForRecv: ADDBA Request incorrect TID = %d\n", pAddFrame->BaParm.TID));
        return FALSE;
    }
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    return TRUE;
}

BOOLEAN PeerAddBARspActionSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen)
{
    PFRAME_ADDBA_RSP pAddFrame;
    
    pAddFrame = (PFRAME_ADDBA_RSP)(pMsg);
    if (MsgLen < (sizeof(FRAME_ADDBA_RSP)))
    {
        DBGPRINT(RT_DEBUG_ERROR,("PeerAddBARspActionSanity: ADDBA Response frame length size = %d incorrect\n", MsgLen));
        return FALSE;
    }
    // we support immediate BA.
    if (pAddFrame->BaParm.BAPolicy != IMMED_BA)
    {
        DBGPRINT(RT_DEBUG_ERROR,("MlmeEnqueueForRecv: ADDBA Response Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy));
        return FALSE;
    }

    // we support immediate BA.
    if (pAddFrame->BaParm.TID &0xfff0)
    {
        DBGPRINT(RT_DEBUG_ERROR,("PeerAddBARspActionSanity: ADDBA Response incorrect TID = %d\n", pAddFrame->BaParm.TID));
        return FALSE;
    }
    return TRUE;

}

BOOLEAN PeerDelBAActionSanity(
    IN PMP_ADAPTER pAd, 
    IN UCHAR Wcid, 
    IN VOID *pMsg, 
    IN ULONG MsgLen)
{
    PFRAME_DELBA_REQ  pDelFrame;
    if (MsgLen != (sizeof(FRAME_DELBA_REQ)))
        return FALSE;
    
    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
        return FALSE;
    
    pDelFrame = (PFRAME_DELBA_REQ)(pMsg);
    if (pDelFrame->DelbaParm.TID &0xfff0)
        return FALSE;
    
    return TRUE;
}

/* 
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN GetTimBit(
    IN  CHAR    *Ptr, 
    IN  USHORT  Aid, 
    OUT UCHAR   *TimLen, 
    OUT UCHAR   *BcastFlag, 
    OUT UCHAR   *DtimCount, 
    OUT UCHAR   *DtimPeriod,
    OUT UCHAR   *MessageToMe) 
{
    UCHAR   BitCntl, N1, N2, MyByte, MyBit;
    CHAR    *IdxPtr;

    IdxPtr = Ptr;

    IdxPtr ++;
    *TimLen = *IdxPtr;

    // get DTIM Count from TIM element
    IdxPtr ++;
    *DtimCount = *IdxPtr;

    // get DTIM Period from TIM element
    IdxPtr++;
    *DtimPeriod = *IdxPtr;

    // get Bitmap Control from TIM element
    IdxPtr++;
    BitCntl = *IdxPtr;

    if ((*DtimCount == 0) && (BitCntl & 0x01)) 
        *BcastFlag = TRUE;
    else 
        *BcastFlag = FALSE;

    // Parse Partial Virtual Bitmap from TIM element
    N1 = BitCntl & 0xfe;    // N1 is the first bitmap byte#
    N2 = *TimLen - 4 + N1;  // N2 is the last bitmap byte#

    if ((Aid < (N1 << 3)) || (Aid >= ((N2 + 1) << 3)))
        *MessageToMe = FALSE;
    else
    {
        MyByte = (Aid >> 3) - N1;                       // my byte position in the bitmap byte-stream
        MyBit = Aid % 16 - ((MyByte & 0x01)? 8:0);

        IdxPtr += (MyByte + 1);

        if (*IdxPtr & (0x01 << MyBit))
        {
            //DBGPRINT(RT_DEBUG_WARN, ("TIM bitmap = 0x%02x, MyBit = 0x%02x\n", *IdxPtr, MyBit));
            *MessageToMe = TRUE;
        }
        else 
            *MessageToMe = FALSE;
    }

    return TRUE;
}

// IRQL = DISPATCH_LEVEL
UCHAR ChannelSanity(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel)
{
    int i;

    for (i = 0; i < pAd->HwCfg.ChannelListNum; i ++)
    {
        if (channel == pAd->HwCfg.ChannelList[i].Channel)
            return 1;
    }
    return 0;
}

/*
    ========================================================================
    Routine Description:
        Get Peer TX phy mode(CCK or OFDM)
    Arguments:
        Channel             Current Channel
        SupRate             Peer's Supported Rate Buffer
        SupRateLen          Peer's Supported Rate Length
        ExtRate             Peer's Extended Rate Buffer
        ExtRateLen          Peer's Extended Rate Length
        
    Return Value:
            1 - CCK
            2 - OFDM
            3 - CCK+OFDM
    IRQL <= DISPATCH_LEVEL
    ========================================================================
*/
UCHAR PeerTxTypeInUseSanity(
    IN UCHAR  Channel,
    IN UCHAR  SupRate[],
    IN UCHAR  SupRateLen,
    IN UCHAR  ExtRate[],
    IN UCHAR  ExtRateLen)
{
    UCHAR   rate, i;
    UCHAR   Type = 0;

    if (Channel <= 14)
    {
        //
        // First check support Rate.
        //
        for (i = 0; i < SupRateLen; i++)
        {
            rate = SupRate[i] & 0x7f; // Mask out basic rate set bit
            if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
            {
                Type |= 0x01;   //CCK
                continue;
            }
            else
            {
                //
                // Otherwise (even rate > 108) means Ndis802_11OFDM24
                //
                Type |= 0x02;   // OFDM
                break;
            }   
        }

        //
        // Second check Extend Rate.
        // Maybe OFDM rate store on Extend Rate. 
        //
        if ((Type & 0x02) == 0)
        {
            for (i = 0; i < ExtRateLen; i++)
            {
                rate = ExtRate[i] & 0x7f; // Mask out basic rate set bit
                if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
                {
                    continue;
                }
                else
                {
                    //
                    // Otherwise (even rate > 108) means Ndis802_11OFDM24
                    //
                    Type |= 0x02;  //OFDM
                    break;
                }
            }
        }
    }
    else
    {
        Type |= 0x02;  //OFDM
    }

    return Type;
}   

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAddBAReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2) 
{
    PMLME_ADDBA_REQ_STRUCT   pInfo;
    
    pInfo = (MLME_ADDBA_REQ_STRUCT *)Msg;

    if ((MsgLen != sizeof(MLME_ADDBA_REQ_STRUCT)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - message lenght not correct.\n"));
        return FALSE;
    }   
    
    if ((pInfo->Wcid >= MAX_LEN_OF_MAC_TABLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - The peer Mac is not associated yet.\n"));
        return FALSE;
    }   

    if ((pInfo->BaBufSize > Max_RX_REORDERBUF) || (pInfo->BaBufSize < 2))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - Rx Reordering buffer too big or too small/ = %d\n", pInfo->BaBufSize));
        return FALSE;
    }   

    if ((pInfo->pAddr[0]&0x01) == 0x01)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - broadcast address not support BA\n"));
        return FALSE;
    }
    return TRUE;
}
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeDelBAReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen) 
{
    MLME_DELBA_REQ_STRUCT   *pInfo;
    pInfo = (MLME_DELBA_REQ_STRUCT *)Msg;

    if ((MsgLen != sizeof(MLME_DELBA_REQ_STRUCT)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeDelBAReqSanity fail - message lenght not correct.\n"));
        return FALSE;
    }   
    
    if ((pInfo->Wcid >= MAX_LEN_OF_MAC_TABLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeDelBAReqSanity fail - The peer Mac is not associated yet.\n"));
        return FALSE;
    }   

    if ((pInfo->TID & 0xf0))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeDelBAReqSanity fail - The peer TID is incorrect.\n"));
        return FALSE;
    }   
    
    return TRUE;
}

BOOLEAN MlmeDlsReqSanity(
    IN  PMP_ADAPTER   pAd, 
    IN  VOID            *Msg, 
    IN  ULONG           MsgLen,
    OUT PRT_802_11_DLS  *pDLS,
    OUT PUSHORT         pReason)
{
    MLME_DLS_REQ_STRUCT *pInfo;

    pInfo = (MLME_DLS_REQ_STRUCT *)Msg;

    *pDLS = pInfo->pDLS;
    *pReason = pInfo->Reason;

    return TRUE;
}


BOOLEAN PeerDlsReqSanity(
    IN  PMP_ADAPTER   pAd, 
    IN  VOID            *Msg, 
    IN  ULONG           MsgLen,
    OUT PUCHAR          pDA,    
    OUT PUCHAR          pSA,
    OUT USHORT          *pCapabilityInfo, 
    OUT USHORT *pDlsTimeout,
    OUT PHT_CAPABILITY_IE pHtCap,
    OUT PULONG pHtCapLen)
{
    CHAR            *Ptr;
    PFRAME_802_11   Fr = (PFRAME_802_11)Msg;
    int RemainLen = (int)MsgLen;

    // to prevent caller from using garbage output value
    *pCapabilityInfo    = 0;
    *pDlsTimeout    = 0;

    Ptr = Fr->Octet;
    RemainLen -= sizeof(HEADER_802_11);

    // offset to destination MAC address (Category and Action field)
    Ptr += 2;
    RemainLen -= 2;

    // get DA from payload and advance the pointer
    PlatformMoveMemory(pDA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;
    RemainLen -= MAC_ADDR_LEN;

    // get SA from payload and advance the pointer
    PlatformMoveMemory(pSA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;
    RemainLen -= MAC_ADDR_LEN;

    // get capability info from payload and advance the pointer
    PlatformMoveMemory(pCapabilityInfo, Ptr, 2);
    Ptr += 2;
    RemainLen -= 2;

    // get capability info from payload and advance the pointer
    PlatformMoveMemory(pDlsTimeout, Ptr, 2);
    Ptr += 2;
    RemainLen -= 2;

    //Supported rates
    if (Ptr[0] == IE_SUPP_RATES)
    {
        Ptr += (2 + Ptr[1]);
        RemainLen -= (2 + Ptr[1]);
    }

    //Ext supported rate
    if (Ptr[0] == IE_EXT_SUPP_RATES)
    {
        Ptr += (2 + Ptr[1]);
        RemainLen -= (2 + Ptr[1]);
    }

    //HT capability
    *pHtCapLen = 0;
    if (RemainLen > 2)
    {
        if (Ptr[0] == IE_HT_CAP)
        {
            if (Ptr[1] <= sizeof(HT_CAPABILITY_IE))
            {
                *pHtCapLen = Ptr[1];
                PlatformMoveMemory(pHtCap, Ptr + 2, *pHtCapLen);
            }
            Ptr += (2 + Ptr[1]);
        }
    }

    return TRUE;
}


BOOLEAN PeerDlsRspSanity(
    IN  PMP_ADAPTER   pAd, 
    IN  VOID            *Msg, 
    IN  ULONG           MsgLen,
    OUT PUCHAR          pDA,
    OUT PUCHAR          pSA,
    OUT USHORT          *pCapabilityInfo, 
    OUT PHT_CAPABILITY_IE pHtCap,
    OUT PULONG pHtCapLen,
    OUT USHORT          *pStatus) 
{
    CHAR            *Ptr;
    PFRAME_802_11   Fr = (PFRAME_802_11)Msg;
    int RemainLen = (int)MsgLen;

    // to prevent caller from using garbage output value
    *pStatus        = 0;
    *pCapabilityInfo    = 0;

    Ptr = Fr->Octet;
    RemainLen -= sizeof(HEADER_802_11);

    // offset to destination MAC address (Category and Action field)
    Ptr += 2;
    RemainLen -= 2;

    // get status code from payload and advance the pointer
    PlatformMoveMemory(pStatus, Ptr, 2);
    Ptr += 2;
    RemainLen -= 2;

    // get DA from payload and advance the pointer
    PlatformMoveMemory(pDA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;
    RemainLen -= MAC_ADDR_LEN;

    // get SA from payload and advance the pointer
    PlatformMoveMemory(pSA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;
    RemainLen -= MAC_ADDR_LEN;

    // get capability info from payload and advance the pointer
    PlatformMoveMemory(pCapabilityInfo, Ptr, 2);
    Ptr += 2;
    RemainLen -= 2;

    //Supported rates
    if (Ptr[0] == IE_SUPP_RATES)
    {
        Ptr += (2 + Ptr[1]);
        RemainLen -= (2 + Ptr[1]);
    }

    //Ext supported rate
    if (Ptr[0] == IE_EXT_SUPP_RATES)
    {
        Ptr += (2 + Ptr[1]);
        RemainLen -= (2 + Ptr[1]);
    }

    //HT capability
    *pHtCapLen = 0;
    if (RemainLen > 2)
    {
        if (Ptr[0] == IE_HT_CAP)
        {
            if (Ptr[1] <= sizeof(HT_CAPABILITY_IE))
            {
                *pHtCapLen = Ptr[1];
                PlatformMoveMemory(pHtCap, Ptr + 2, *pHtCapLen);
            }
            Ptr += (2 + Ptr[1]);
        }
    }

    return TRUE;
}

BOOLEAN PeerDlsTearDownSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pReason) 
{
    CHAR            *Ptr;
    PFRAME_802_11   Fr = (PFRAME_802_11)Msg;

    // to prevent caller from using garbage output value
    *pReason    = 0;

    Ptr = Fr->Octet;

    // offset to destination MAC address (Category and Action field)
    Ptr += 2;

    // get DA from payload and advance the pointer
    PlatformMoveMemory(pDA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

    // get SA from payload and advance the pointer
    PlatformMoveMemory(pSA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

    // get reason code from payload and advance the pointer
    PlatformMoveMemory(pReason, Ptr, 2);
    Ptr += 2;

    return TRUE;
}

BOOLEAN MlmeTdlsReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PRT_802_11_TDLS *pTDLS,
    OUT PUSHORT pReason)
{
    MLME_TDLS_REQ_STRUCT *pInfo;

    pInfo = (MLME_TDLS_REQ_STRUCT *)Msg;
    
    *pTDLS = pInfo->pTDLS;
    *pReason = pInfo->Reason;

    return TRUE;
}

BOOLEAN PeerTdlsSetupReqSanity(
    IN PMP_ADAPTER    pAd,
    PMP_PORT          pPort,
    IN VOID     *Msg, 
    IN ULONG    MsgLen,
    OUT UCHAR   *pToken,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,
    OUT USHORT  *pCapabilityInfo,
    OUT UCHAR   *pSupRateLen,   
    OUT UCHAR   SupRate[],
    OUT UCHAR   *pExtRateLen,
    OUT UCHAR   ExtRate[],
    OUT BOOLEAN *pbWmmCapable,  
    OUT UCHAR   *pQosCapability,
    OUT UCHAR   *pHtCapLen,
    OUT HT_CAPABILITY_IE    *pHtCap,
    OUT EXT_CAP_ELEMT   *pExtCap,
    OUT UCHAR   *pRsnLen,
    OUT UCHAR   RsnIe[],
    OUT UCHAR   *pFTLen,
    OUT UCHAR   FTIe[],
    OUT UCHAR   *pTILen,
    OUT UCHAR   TIIe[])
{
    ULONG           RemainLen = MsgLen;
    CHAR            *Ptr =(CHAR *)Msg;
    PEID_STRUCT     pEid;
    ULONG           Length = 0; 
    PHEADER_802_11  pHeader80211;

    // Init output parameters
    *pSupRateLen = 0;
    *pExtRateLen = 0;
    *pCapabilityInfo = 0;
    *pHtCapLen = 0;
    pExtCap->DoubleWord = 0;
    *pbWmmCapable = FALSE;
    *pQosCapability = 0;
    *pRsnLen = 0;
    *pFTLen = 0;
    *pTILen = 0;
    
    // Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
    if(RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER))) 
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (action header) \n"));
        return FALSE;   
    }

    // Point to Source Address
    pHeader80211 = (PHEADER_802_11)Msg;
    PlatformMoveMemory(pSA, pHeader80211->Addr3, MAC_ADDR_LEN);

    // Offset to Dialog Token
    Ptr += (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));
    RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));

    // Get the value of token from payload and advance the pointer
    if(RemainLen < 1)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (dialog token) \n"));
        return FALSE;
    }   
    *pToken = *Ptr;
    
    // Offset to Link Identifier
    Ptr += 1;
    RemainLen -= 1;

    // Get BSSID, SA and DA from payload and advance the pointer
    if(RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_ID || Ptr[1] != 18)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (link identifier) \n"));
        return FALSE;
    }
    if (!MAC_ADDR_EQUAL(Ptr+2, pPort->PortCfg.Bssid))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> It's not my BSSID\n"));
        return FALSE;
    }   
    else if (!MAC_ADDR_EQUAL(Ptr+14, pAd->HwCfg.CurrentAddress))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> It's not my MAC address\n"));
        return FALSE;
    }   

    PlatformMoveMemory(pSA2, Ptr+8, MAC_ADDR_LEN);


    // Offset to Capability
    Ptr += 20;
    RemainLen -= 20;

    // Get capability info from payload and advance the pointer
    if(RemainLen < 2) 
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (capability) \n"));
        return FALSE;
    }   
    PlatformMoveMemory((PUCHAR)pCapabilityInfo, Ptr, 2);

    // Offset to other elements
    Ptr += 2;
    RemainLen -= 2;


    pEid = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= RemainLen)     
    {   
        switch(pEid->Eid)
        {
            case IE_SUPP_RATES:
                if(pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(SupRate, pEid->Octet, pEid->Len);
                    *pSupRateLen = pEid->Len;
                }
                else
                    return FALSE;

                break;
            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;
                }
                break;

            case IE_QOS_CAPABILITY:
                if (pEid->Len == 1)
                {
                    *pQosCapability = (UCHAR)pEid->Octet[0];
                    *pbWmmCapable = TRUE;
                }
                break;

            case IE_HT_CAP:
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
                    {
                        PlatformMoveMemory(pHtCap, pEid->Octet, sizeof(HT_CAPABILITY_IE));
                        *pHtCapLen = SIZE_HT_CAP_IE;    // Nnow we only support 26 bytes.
                    }
                }
                break;
                
            case IE_2040_BSS_COEXIST:
                break;

            case IE_EXT_CAPABILITY:
                if (pEid->Len == 1)
                {
                    PlatformMoveMemory(pExtCap, pEid->Octet, pEid->Len);
                }
                break;

            case IE_RSN:
                if ((pEid->Len + 2) < 64)
                {
                    PlatformMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
                    *pRsnLen = pEid->Len + 2;
                }
                break;

            case IE_FAST_BSS_TRANSITION:
                if ((pEid->Len) == (sizeof(FT_IE) - 256))
                {
                    PlatformMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
                    *pFTLen = pEid->Len + 2;
                }   
                break;
                
            case IE_TIMEOUT_INTERVAL:
                if ((pEid->Len + 2) == 7)
                {
                    PlatformMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
                    *pTILen = pEid->Len + 2;
                }
                break;
                
            default:
                // Unknown IE, we have to pass it as variable IEs
                DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity - unrecognized EID = %d\n", pEid->Eid));
                break;
        }

        Length = Length + 2 + pEid->Len;    
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }


    // Process in succeed
    return TRUE;


}

BOOLEAN PeerTdlsSetupRspSanity(
    IN PMP_ADAPTER    pAd,
    PMP_PORT          pPort,
    IN VOID     *Msg, 
    IN ULONG    MsgLen,
    OUT UCHAR   *pToken,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,
    OUT USHORT  *pCapabilityInfo,
    OUT UCHAR   *pSupRateLen,   
    OUT UCHAR   SupRate[],
    OUT UCHAR   *pExtRateLen,
    OUT UCHAR   ExtRate[],
    OUT BOOLEAN *pbWmmCapable,  
    OUT UCHAR   *pQosCapability,
    OUT UCHAR   *pHtCapLen,
    OUT HT_CAPABILITY_IE    *pHtCap,
    OUT EXT_CAP_ELEMT   *pExtCap,
    OUT USHORT  *pStatusCode,
    OUT UCHAR   *pRsnLen,
    OUT UCHAR   RsnIe[],
    OUT UCHAR   *pFTLen,
    OUT UCHAR   FTIe[],
    OUT UCHAR   *pTILen,
    OUT UCHAR   TIIe[])
{
    ULONG           RemainLen = MsgLen;
    CHAR            *Ptr =(CHAR *)Msg;
    PEID_STRUCT     pEid;
    ULONG           Length = 0; 
    PHEADER_802_11  pHeader80211;

    // Init output parameters
    *pSupRateLen = 0;
    *pExtRateLen = 0;
    *pCapabilityInfo = 0;
    *pHtCapLen = 0;
    pExtCap->DoubleWord = 0;
    *pbWmmCapable = FALSE;  
    *pQosCapability = 0;
    *pStatusCode = MLME_SUCCESS;
    *pRsnLen = 0;
    *pFTLen = 0;
    *pTILen = 0;

    
    // Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
    if(RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER)))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (action header) \n"));
        return FALSE;   
    }

    // Point to Source Address
    pHeader80211 = (PHEADER_802_11)Msg;
    PlatformMoveMemory(pSA, pHeader80211->Addr3, MAC_ADDR_LEN);

    // Offset to Status Code
    Ptr += (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));
    RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));
    
    // Get the value of Status Code from payload and advance the pointer
    if(RemainLen < 2)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (status code) \n"));
        return FALSE;
    }   
    *pStatusCode = (*((PUSHORT) Ptr));

    // Offset to Dialog Token
    Ptr += 2;
    RemainLen -= 2;

    // Get the value of token from payload and advance the pointer
    if(RemainLen < 1)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (dialog token) \n"));
        return FALSE;
    }   
    *pToken = *Ptr;

    if (*pStatusCode != MLME_SUCCESS)
        return TRUE;    // in the end of Setup Response frame
        
    // Offset to Link Identifier
    Ptr += 1;
    RemainLen -= 1;

    // Get BSSID, SA and DA from payload and advance the pointer
    if(RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_ID || Ptr[1] != 18) 
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (link identifier) \n"));
        return FALSE;
    }
    if (!MAC_ADDR_EQUAL(Ptr+2, pPort->PortCfg.Bssid))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> It's not my BSSID\n"));
        return FALSE;
    }   
    else if (!MAC_ADDR_EQUAL(Ptr+8, pAd->HwCfg.CurrentAddress))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> It's not my MAC address\n"));
        return FALSE;
    }   

    PlatformMoveMemory(pSA2, Ptr+14, MAC_ADDR_LEN);

    // Offset to Capability
    Ptr += 20;
    RemainLen -= 20;

    // Get capability info from payload and advance the pointer
    if(RemainLen < 2) 
        return FALSE;
    PlatformMoveMemory(pCapabilityInfo, Ptr, 2);

    // Offset to other elements
    Ptr += 2;
    RemainLen -= 2;


    pEid = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= RemainLen)     
    {   
        switch(pEid->Eid)
        {
            case IE_SUPP_RATES:
                if(pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(SupRate, pEid->Octet, pEid->Len);
                    *pSupRateLen = pEid->Len;
                }
                else
                    return FALSE;

                break;
            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    PlatformMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;
                }
                break;

            case IE_QOS_CAPABILITY:
                if (pEid->Len == 1)
                {
                    *pQosCapability = (UCHAR)pEid->Octet[0];
                    *pbWmmCapable = TRUE;
                }
                break;

            case IE_HT_CAP:
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
                    {
                        PlatformMoveMemory(pHtCap, pEid->Octet, sizeof(HT_CAPABILITY_IE));
                        *pHtCapLen = SIZE_HT_CAP_IE;    // Nnow we only support 26 bytes.
                    }
                }
                
                break;
                
            case IE_2040_BSS_COEXIST:
                break;

            case IE_EXT_CAPABILITY:
                if (pEid->Len == 1)
                {
                    PlatformMoveMemory(pExtCap, pEid->Octet, pEid->Len);
                }
                break;

            case IE_RSN:
                if ((pEid->Len + 2) < 64)
                {
                    PlatformMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
                    *pRsnLen = pEid->Len + 2;
                }
                break;

            case IE_FAST_BSS_TRANSITION:
                if ((pEid->Len) == (sizeof(FT_IE) - 256))
                {
                    PlatformMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
                    *pFTLen = pEid->Len + 2;
                }   
                break;
                
            case IE_TIMEOUT_INTERVAL:
                if ((pEid->Len + 2) == 7)
                {
                    PlatformMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
                    *pTILen = pEid->Len + 2;
                }
                break;
                
            default:
                // Unknown IE, we have to pass it as variable IEs
                DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity - unrecognized EID = %d\n", pEid->Eid));
                break;
        }

        Length = Length + 2 + pEid->Len;    
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }


    // Process in succeed
    *pStatusCode = MLME_SUCCESS;
    return TRUE;

}

BOOLEAN PeerTdlsSetupConfSanity(
    IN PMP_ADAPTER    pAd,
    PMP_PORT          pPort,
    IN VOID     *Msg, 
    IN ULONG    MsgLen,
    OUT UCHAR   *pToken,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,
    OUT USHORT  *pCapabilityInfo,
    OUT EDCA_PARM   *pEdcaParm,
    OUT USHORT  *pStatusCode,
    OUT UCHAR   *pRsnLen,
    OUT UCHAR   RsnIe[],
    OUT UCHAR   *pFTLen,
    OUT UCHAR   FTIe[],
    OUT UCHAR   *pTILen,
    OUT UCHAR   TIIe[])
{
    ULONG           RemainLen = MsgLen;
    CHAR            *Ptr =(CHAR *)Msg;
    PEID_STRUCT     pEid;
    ULONG           Length = 0; 
    PHEADER_802_11  pHeader80211;

    // Init output parameters
    *pCapabilityInfo = 0;
    *pStatusCode = MLME_REQUEST_DECLINED;
     pEdcaParm->bValid = FALSE;      // default: no IE_EDCA_PARAMETER found
    *pRsnLen = 0;
    *pFTLen = 0;
    *pTILen = 0;
    
    // Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
    if(RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER)))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (action header) \n"));
        return FALSE;   
    }

    // Point to Source Address
    pHeader80211 = (PHEADER_802_11)Msg;
    PlatformMoveMemory(pSA, pHeader80211->Addr3, MAC_ADDR_LEN);

    // Offset to Status Code
    Ptr += (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));
    RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));
    
    // Get the value of Status Code from payload and advance the pointer
    if(RemainLen < 2)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (status code) \n"));
        return FALSE;
    }   
    *pStatusCode = (*((PUSHORT) Ptr));


    // Offset to Dialog Token
    Ptr += 2;
    RemainLen -= 2;

    // Get the value of token from payload and advance the pointer
    if(RemainLen < 1)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (dialog token) \n"));
        return FALSE;
    }   
    *pToken = *Ptr;

        
    // Offset to Link Identifier
    Ptr += 1;
    RemainLen -= 1;

    // Get BSSID, SA and DA from payload and advance the pointer
    if(RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_ID || Ptr[1] != 18)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (link identifier) \n"));
        return FALSE;
    }
    if (!MAC_ADDR_EQUAL(Ptr+2, pPort->PortCfg.Bssid))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> It's not my BSSID\n"));
        return FALSE;
    }   
    else if (!MAC_ADDR_EQUAL(Ptr+14, pAd->HwCfg.CurrentAddress))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> It's not my MAC address\n"));
        return FALSE;
    }   

    PlatformMoveMemory(pSA2, Ptr+8, MAC_ADDR_LEN);

    if (*pStatusCode != MLME_SUCCESS)
        return TRUE;    // end of Setup Confirm frame

    // Offset to other elements
    Ptr += 20;
    RemainLen -= 20;

    pEid = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= RemainLen)     
    {   
        switch(pEid->Eid)
        {
            case IE_VENDOR_SPECIFIC:
                // handle WME PARAMTER ELEMENT
                if (PlatformEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
                {
                    PUCHAR ptr;
                    int i;

                    // parsing EDCA parameters
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
                    //pEdcaParm->bMoreDataAck    = FALSE; // pEid->Octet[0] & 0x80;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
                    ptr = &pEid->Octet[8];
                    for (i=0; i<4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }
                break;
                
            case IE_ADD_HT:
                break;
                
            case IE_RSN:
                if ((pEid->Len + 2) < 64)
                {
                    PlatformMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
                    *pRsnLen = pEid->Len + 2;
                }
                break;
                
            case IE_FAST_BSS_TRANSITION:
                if ((pEid->Len) == (sizeof(FT_IE) - 256))
                {
                    PlatformMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
                    *pFTLen = pEid->Len + 2;
                }   
                break;
                
            case IE_TIMEOUT_INTERVAL:
                if ((pEid->Len + 2) == 7)
                {
                    PlatformMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
                    *pTILen = pEid->Len + 2;
                }
                break;
                
            default:
                // Unknown IE, we have to pass it as variable IEs
                DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity - unrecognized EID = %d\n", pEid->Eid));
                break;
        }

        Length = Length + 2 + pEid->Len;    
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }


    // Process in succeed
    *pStatusCode = MLME_SUCCESS;
    return TRUE;

}

BOOLEAN PeerTdlsTearDownSanity(
    IN PMP_ADAPTER pAd,
    PMP_PORT      pPort,    
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,
    OUT BOOLEAN *pIsInitator,
    OUT USHORT *pReasonCode,
    OUT UCHAR   *pFTLen,    
    OUT UCHAR   FTIe[])
{
    ULONG           RemainLen = MsgLen;
    CHAR            *Ptr =(CHAR *)Msg;
    PEID_STRUCT     pEid;
    ULONG           Length = 0; 
    PHEADER_802_11  pHeader80211;

    // Init output parameters
    *pReasonCode = 0;
    *pFTLen = 0 ;

    // Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
    if(RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER)))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> Invaild packet length - (cation header) \n"));
        return FALSE;   
    }

    // Point to Source Address
    pHeader80211 = (PHEADER_802_11)Msg;
    PlatformMoveMemory(pSA, pHeader80211->Addr3, MAC_ADDR_LEN);
    
    // Offset to Reason Code
    Ptr += (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));
    RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER));
    
    // Get the value of Reason Code from payload and advance the pointer
    if(RemainLen < 2) 
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> Invaild packet length - (reason code) \n"));
        return FALSE;   
    }
    *pReasonCode = (*((PUSHORT) Ptr));


    // Offset to Link Identifier
    Ptr += 2;
    RemainLen -= 2;

    // Get BSSID, SA and DA from payload and advance the pointer
    if(RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_ID || Ptr[1] != 18) 
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> Invaild packet length - (link identifier) \n"));
        return FALSE;   
    }
    // It's not my BSSID
    if (!MAC_ADDR_EQUAL(Ptr+2, pPort->PortCfg.Bssid))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> It's not my BSSID\n"));
        return FALSE;
    }   

    // Check if my MAC address and then find out SA
    if (!MAC_ADDR_EQUAL(pAd->HwCfg.CurrentAddress, Ptr+8))
    {
        if (!MAC_ADDR_EQUAL(pAd->HwCfg.CurrentAddress, Ptr+14))
            return FALSE;
        else
        {
            *pIsInitator = FALSE;   // I am not Initator.
            PlatformMoveMemory(pSA2, Ptr+8, MAC_ADDR_LEN);
        }
    }
    else
    {
        *pIsInitator = TRUE;    // I am Initator.
        PlatformMoveMemory(pSA2, Ptr+14, MAC_ADDR_LEN);
    }
    
    
    // Offset to other elements
    Ptr += 20;
    RemainLen -= 20;

    pEid = (PEID_STRUCT) Ptr;

    // TPK Handshake if RSNA Enabled
    if ((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
    {
        if(RemainLen < ((UCHAR)(pEid->Len + 2)) || (pEid->Eid != IE_FAST_BSS_TRANSITION))
            return FALSE;

        if ((pEid->Len) == (sizeof(FT_IE) - 256))
        {
            PlatformMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
            *pFTLen = pEid->Len + 2;
        }       
    }

    return TRUE;
}

BOOLEAN SAQuerySanity(
    IN PMP_ADAPTER pAd, 
    IN UCHAR Wcid, 
    IN VOID *pMsg, 
    IN ULONG MsgLen)
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;

    if (MsgLen < (LENGTH_802_11+4+sizeof(PMF_SA_QUERY_ACTION)))
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("MFP: Sanity failed, MsgLen%d < %d\n",MsgLen, (LENGTH_802_11+4+sizeof(PMF_SA_QUERY_ACTION))));
        return FALSE;
    }
        
    
    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("MFP: Sanity failed, Wcid:%d\n",Wcid));
        return FALSE;
    }

    if (pFrame->Octet[0] != CATEGORY_SAQUERY)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN, ("MFP: Sanity failed, category:%d\n",pFrame->Octet[0]));
        return FALSE;
    }
    
    return TRUE;
}
