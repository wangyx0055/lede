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
    AP_oids_Query.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

#if (COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  

NDIS_STATUS
N6QueryOidDot11EnumPeerInfo(
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
 
    PDOT11_PEER_INFO_LIST pPeerInfoList;
    PDOT11_PEER_INFO pPeerInfo;
    MAC_TABLE_ENTRY *pEntry = NULL;
    ULONG staEntryCount = 0;
    ULONG requiredSize = 0;
    MP_RW_LOCK_STATE LockState;
    
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL; 
    PQUEUE_HEADER pHeader;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    // EXPAND_FUNC RTMPQueryPeerInfo()
    do
    {
        //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
        PlatformAcquireOldRWLockForRead(&pAd->MacTablePool.MacTabPoolLock,&LockState);

        staEntryCount = pPort->MacTab.Size;

        //
        // calculate the required size
        //
        if (!GetRequiredListSize(
                        FIELD_OFFSET(DOT11_PEER_INFO_LIST, PeerInfo), 
                        sizeof(DOT11_PEER_INFO), 
                        staEntryCount, 
                        &requiredSize))
        {
            // overflow should not happen
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (InformationBufferLength < requiredSize)
        {
            *BytesNeeded = requiredSize;
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        // the buffer is large enough to hold all information
        pPeerInfoList = (PDOT11_PEER_INFO_LIST)InformationBuffer;
        pPeerInfoList->uTotalNumOfEntries = staEntryCount;
        pPeerInfoList->uNumOfEntries = 0;

        // enumerate all station entries and get association info
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
            
            if(pMacEntry == NULL)
            {
                break; 
            }

            if((pMacEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            if ((pMacEntry->ValidAsCLI == TRUE) && (pMacEntry->ValidAsWDS == FALSE))
            {
                pEntry = pMacEntry;

                // Filter not conenct to GO. For example, p2p client wcid is equal to 23.
                if (pPort->PortType == WFD_GO_PORT)
                {
                    if ((pEntry->P2pInfo.P2pClientState != P2PSTATE_NONP2P_PSK)
                        && (pEntry->P2pInfo.P2pClientState != P2PSTATE_NONP2P_WPS)
                        && (pEntry->P2pInfo.P2pClientState != P2PSTATE_CLIENT_ASSOC)
                        && (pEntry->P2pInfo.P2pClientState != P2PSTATE_CLIENT_OPERATING)
                        && (pEntry->P2pInfo.P2pClientState != P2PSTATE_CLIENT_WPS))
                    {
                    pNextMacEntry = pNextMacEntry->Next;   
                    }
                    pMacEntry = NULL;
                    continue; 
                }
                
                pPeerInfo = &pPeerInfoList->PeerInfo[pPeerInfoList->uNumOfEntries];
    
                PlatformZeroMemory(pPeerInfo, sizeof(DOT11_PEER_INFO));

                // MAC address
                COPY_MAC_ADDR(pPeerInfo->MacAddress, pEntry->Addr);

                // capability information
                pPeerInfo->usCapabilityInformation = pEntry->CapabilityInfo;

                // auth/cipher
                pPeerInfo->AuthAlgo = pEntry->AuthMode;

                pPeerInfo->UnicastCipherAlgo = pEntry->WepStatus;
                pPeerInfo->MulticastCipherAlgo = pEntry->WepStatus;

                // WPS enabled
                pPeerInfo->bWpsEnabled = pEntry->bWpsEnabled;

                // listen interval
                pPeerInfo->usListenInterval = pEntry->usListenInterval;

                // supported rates
                PlatformMoveMemory(pPeerInfo->ucSupportedRates, pEntry->SupRate, pEntry->SupRateLen);

                // association ID
                pPeerInfo->usAssociationID = pEntry->Aid;

                // association state
                pPeerInfo->AssociationState = pEntry->AssocState;

                // power mode
                pPeerInfo->PowerMode = pEntry->PowerMode;

                // association up time
                pPeerInfo->liAssociationUpTime = pEntry->AssocUpTime;

                // statistics
                PlatformMoveMemory(&pPeerInfo->Statistics, &pEntry->Statistics, sizeof(DOT11_PEER_STATISTICS));

                pPeerInfoList->uNumOfEntries++;
                DBGPRINT(RT_DEBUG_ERROR, ("Num of client %d : MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", pPeerInfoList->uNumOfEntries,
            pPeerInfo->MacAddress[0],pPeerInfo->MacAddress[1],pPeerInfo->MacAddress[2],
            pPeerInfo->MacAddress[3],pPeerInfo->MacAddress[4],pPeerInfo->MacAddress[5]));
            }
            
            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }

        //ASSERT(pPeerInfoList->uNumOfEntries == pPeerInfoList->uTotalNumOfEntries);
        
        *BytesWritten = requiredSize;

        MP_ASSIGN_NDIS_OBJECT_HEADER(pPeerInfoList->Header,
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_PEER_INFO_LIST_REVISION_1,
                                    sizeof(DOT11_PEER_INFO_LIST));
    } while (FALSE);

    //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    

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


#endif


NDIS_STATUS
N6QueryOidDot11AdditionalIe(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryAdditionalIe()
        ULONG beaconIeOffset = sizeof(DOT11_ADDITIONAL_IE);
        ULONG responseIeOffset = 0;
        ULONG requiredSize = 0;
        PMP_PORT              pPort = pAd->PortList[PortNumber];
        PDOT11_ADDITIONAL_IE AdditionalIe;
        do
        {
        *BytesWritten= 0;
        *BytesNeeded = 0;
        // 
        // add required size for beacon IEs
        //
        if (RtlULongAdd(beaconIeOffset, pPort->PortCfg.AdditionalBeaconIESize, &responseIeOffset) != STATUS_SUCCESS)
        {
            // 
            // This shall not happen because we validated the IEs before
            //
            ASSERT(FALSE);
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }
        
         // 
        // add required size for response IEs
        //
        if (RtlULongAdd(responseIeOffset, pPort->PortCfg.AdditionalResponseIESize, &requiredSize) != STATUS_SUCCESS)
        {
            // 
            // This shall not happen because we validated the IEs before
            //
            ASSERT(FALSE);
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (InformationBufferLength < requiredSize)
        {
            // 
            // the buffer is not big enough
            //
            *BytesNeeded = requiredSize;
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        AdditionalIe = (PDOT11_ADDITIONAL_IE)InformationBuffer;
        // 
        // the buffer is big enough, copy the IEs
        // copy beacon IEs
        //
        RtlCopyMemory(
            (BYTE *)AdditionalIe + beaconIeOffset,
            pPort->PortCfg.AdditionalBeaconIEData,
            pPort->PortCfg.AdditionalBeaconIESize
            );

        AdditionalIe->uBeaconIEsLength = pPort->PortCfg.AdditionalBeaconIESize;
            AdditionalIe->uBeaconIEsOffset = beaconIeOffset;

        // 
        // copy response IEs
        //
        RtlCopyMemory(
            (BYTE *)AdditionalIe + responseIeOffset,
            pPort->PortCfg.AdditionalResponseIEData,
            pPort->PortCfg.AdditionalResponseIESize
            );

        AdditionalIe->uResponseIEsLength = pPort->PortCfg.AdditionalResponseIESize;
        AdditionalIe->uResponseIEsOffset = responseIeOffset;        
        *BytesWritten = requiredSize;
    }while(FALSE);
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


NDIS_STATUS
N6QueryOidDot11AssociationParams(
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
        // EXPAND_FUNC N6Dot11_QueryAssociationParam()
        PDOT11_ASSOCIATION_PARAMS   dot11AssocParams = NULL;
        
        do
        {
        *BytesWritten = 0;
        *BytesNeeded = 0;
         
             
        *BytesNeeded  = sizeof(DOT11_ASSOCIATION_PARAMS) + pPort->PortCfg.AdditionalAssocReqIESize;
            if (InformationBufferLength <*BytesNeeded )
        {
            *BytesNeeded = sizeof(DOT11_ASSOCIATION_PARAMS);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }
        dot11AssocParams = (PDOT11_ASSOCIATION_PARAMS)InformationBuffer;
        MP_ASSIGN_NDIS_OBJECT_HEADER(((PDOT11_ASSOCIATION_PARAMS)InformationBuffer)->Header,
                NDIS_OBJECT_TYPE_DEFAULT,
            DOT11_ASSOCIATION_PARAMS_REVISION_1,
            sizeof(DOT11_ASSOCIATION_PARAMS));
        PlatformMoveMemory(dot11AssocParams->BSSID, pPort->PortCfg.AssocIEBSSID, DOT11_ADDRESS_SIZE);
            dot11AssocParams->uAssocRequestIEsLength = pPort->PortCfg.AdditionalAssocReqIESize;
            dot11AssocParams->uAssocRequestIEsOffset = sizeof(DOT11_ASSOCIATION_PARAMS);

        if (pPort->PortCfg.AdditionalAssocReqIESize > 0)
        {
            PlatformMoveMemory(Add2Ptr(dot11AssocParams, sizeof(DOT11_ASSOCIATION_PARAMS)),
                pPort->PortCfg.AdditionalAssocReqIEData,
                pPort->PortCfg.AdditionalAssocReqIESize);
        }                  

        *BytesWritten  = *BytesNeeded;
    }while(FALSE);
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


NDIS_STATUS
N6QueryOidDot11StartApRequest(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC BREAK
        } while (FALSE);
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


NDIS_STATUS
N6QueryOidDot11AvailableFrequencyList(
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
            // EXPAND_FUNC NONE
    //
    // Current we didn't support it.
    //
    DBGPRINT(RT_DEBUG_INFO, ("Query::OID_DOT11_AVAILABLE_FREQUENCY_LIST\n"));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC BREAK
        } while (FALSE);
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


NDIS_STATUS
N6QueryOidDot11BeaconPeriod(
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
 
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        uInfo = pPort->CommonCfg.BeaconPeriod;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_BEACON_PERIOD (=%d)\n", uInfo));
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC BREAK
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


NDIS_STATUS
N6QueryOidDot11DtimPeriod(
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
 
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        uInfo = pPort->SoftAP.ApCfg.DtimPeriod;
        DBGPRINT(RT_DEBUG_INFO, ("Query::OID_DOT11_DTIM_PERIOD(=%d)\n", uInfo));
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC BREAK
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


NDIS_STATUS
N6QueryOidDot11IncomingAssociationDecision(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC BREAK
        } while (FALSE);
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


NDIS_STATUS
N6QueryOidDot11DisassociatePeerRequest(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC BREAK
        } while (FALSE);
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

    
NDIS_STATUS
N6QueryOidDot11WpsEnabled(
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

    // EXPAND_FUNC ApOidQueryWPSEnable()
    do
    {
        *BytesWritten = 0;
        *BytesNeeded = 0;
        if( InformationBufferLength < sizeof(BOOLEAN))
        {
            *BytesNeeded = sizeof(BOOLEAN);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }
        
        if (pPort->PortType == EXTSTA_PORT)
        {
            ndisStatus = NDIS_STATUS_FAILURE;
            break;
        }
        
        *(BOOLEAN *) InformationBuffer  = pPort->SoftAP.ApCfg.bWPSEnable ;
        *BytesWritten = sizeof(BOOLEAN);
    }while(FALSE);  
    DBGPRINT(RT_DEBUG_TRACE, ("Query bWPSEnable = %x\n",pPort->SoftAP.ApCfg.bWPSEnable));

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


