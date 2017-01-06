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
    Port_oids_Query.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

//
// Virtual station auth/cipher algorithms
//
const DOT11_AUTH_CIPHER_PAIR VStationBSSUnicastAuthCipherPairs[] = {
    {DOT11_AUTH_ALGO_80211_OPEN,        DOT11_CIPHER_ALGO_NONE},
    {VPORT_DOT11_AUTH_ALGO_RSNA_PSK,    VPORT_DOT11_CIPHER_ALGO_CCMP},
};

const DOT11_AUTH_CIPHER_PAIR VStationBSSMulticastAuthCipherPairs[] = {
    {DOT11_AUTH_ALGO_80211_OPEN,        DOT11_CIPHER_ALGO_NONE},
    {VPORT_DOT11_AUTH_ALGO_RSNA_PSK,    VPORT_DOT11_CIPHER_ALGO_CCMP},
};


NDIS_STATUS
N6QueryOidDot11VirtualStationCapability(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            InformationBuffer       = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            PUCHAR pBuf;
            PDOT11_EXTSTA_ATTRIBUTES pDot11ExtStaCap = (PDOT11_EXTSTA_ATTRIBUTES)InformationBuffer;
            
            // EXPAND_FUNC NONE
    DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_VIRTUAL_STATION_CAPABILITY, InformationBufferLength = %u\n", InformationBufferLength));

    *BytesNeeded = sizeof(DOT11_EXTSTA_ATTRIBUTES) +
            sizeof(VStationBSSUnicastAuthCipherPairs) +
            sizeof(VStationBSSMulticastAuthCipherPairs);

            if (InformationBufferLength < *BytesNeeded)
            {
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            // EXPAND_FUNC N6Dot11_QueryVirtualStationCapability()
        DBGPRINT(RT_DEBUG_TRACE, ("N6Dot11_QueryVirtualStationCapability...\n"));

        PlatformZeroMemory(InformationBuffer, InformationBufferLength);

        MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11ExtStaCap->Header, 
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_EXTSTA_ATTRIBUTES_REVISION_2,
                    sizeof(DOT11_EXTSTA_ATTRIBUTES));

        pDot11ExtStaCap->uScanSSIDListSize = STA_MAX_SCAN_SSID_LIST_COUNT;
        pDot11ExtStaCap->uDesiredBSSIDListSize = 1;
        pDot11ExtStaCap->uDesiredSSIDListSize = 1;
        pDot11ExtStaCap->uExcludedMacAddressListSize = 4;
        pDot11ExtStaCap->uPrivacyExemptionListSize = 32;
        pDot11ExtStaCap->uKeyMappingTableSize = (16 - DOT11_MAX_NUM_DEFAULT_KEY);
        pDot11ExtStaCap->uDefaultKeyTableSize = DOT11_MAX_NUM_DEFAULT_KEY;
        pDot11ExtStaCap->uWEPKeyValueMaxLength = 104 / 8;
        pDot11ExtStaCap->uPMKIDCacheSize = 0;//STA_PMKID_MAX_COUNT;
        pDot11ExtStaCap->uMaxNumPerSTADefaultKeyTables = 16 - DOT11_MAX_NUM_DEFAULT_KEY;
        pDot11ExtStaCap->bStrictlyOrderedServiceClassImplemented = FALSE;

                //
        // Qos protocol support
                //
        pDot11ExtStaCap->ucSupportedQoSProtocolFlags = DOT11_QOS_PROTOCOL_FLAG_WMM;

                //
        // Safe mode enabled
            //
        pDot11ExtStaCap->bSafeModeImplemented = FALSE;

            //
        // 11d stuff.
        //
        pDot11ExtStaCap->uNumSupportedCountryOrRegionStrings = 0;
        pDot11ExtStaCap->pSupportedCountryOrRegionStrings = NULL;

        //
        pBuf = (PUCHAR)&pDot11ExtStaCap[1];

        pDot11ExtStaCap->uInfraNumSupportedUcastAlgoPairs = sizeof(VStationBSSUnicastAuthCipherPairs) / sizeof(DOT11_AUTH_CIPHER_PAIR);
        PlatformMoveMemory(pBuf, (PVOID)VStationBSSUnicastAuthCipherPairs, sizeof(VStationBSSUnicastAuthCipherPairs));
        pDot11ExtStaCap->pInfraSupportedUcastAlgoPairs = (PDOT11_AUTH_CIPHER_PAIR)pBuf;
        pBuf += sizeof(VStationBSSUnicastAuthCipherPairs);

        pDot11ExtStaCap->uInfraNumSupportedMcastAlgoPairs = sizeof(VStationBSSMulticastAuthCipherPairs) / sizeof(DOT11_AUTH_CIPHER_PAIR);
        PlatformMoveMemory(pBuf, (PVOID)VStationBSSMulticastAuthCipherPairs, sizeof(VStationBSSMulticastAuthCipherPairs));
        pDot11ExtStaCap->pInfraSupportedMcastAlgoPairs = (PDOT11_AUTH_CIPHER_PAIR)pBuf;
        pBuf += sizeof(VStationBSSMulticastAuthCipherPairs);
 
        *BytesWritten = InformationBufferLength;
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(InformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                //
                // the result is either zero length of has been copied
                // by the OID specific handler
                //
                // Dont overwrite the BytesWritten and BytesNeeded
                // as they are also appropriately set
                //
            }
        }
        else
        {
            //
            // The buffer length is not sufficient
            //
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}
