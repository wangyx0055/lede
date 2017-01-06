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
    p2pWfd_oids_Query.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER)) 

NDIS_STATUS
N6QueryOidDot11WfdEnumDeviceList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber         = NdisRequest->PortNumber;
    PMP_PORT         pPort              = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            InformationBuffer  = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten       = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded        = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
    
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
        // EXPAND_FUNC NONE
        ndisStatus = P2pMsEnumerateDeviceList(pAd,
                                              pPort,
                                              InformationBuffer,
                                    InformationBufferLength,
                                              BytesWritten,
                                              BytesNeeded);
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
N6QueryOidDot11WfdListenStateDiscoverability(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber         = NdisRequest->PortNumber;
    PMP_PORT         pPort              = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            InformationBuffer  = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten       = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded        = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded = 0;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    uInfo = pPort->P2PCfg.DeviceDiscoverable;
    uInfoLen = sizeof(ULONG);
    DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_WFD_LISTEN_STATE_DISCOVERABILITY (=%d)\n", uInfo));
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


NDIS_STATUS
N6QueryOidDot11WfdGetDialogToken(
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
        // EXPAND_FUNC NONE
        ndisStatus = P2pMsGetDialogToken(pAd,
                                         pPort,
                                         InformationBuffer,
                                         InformationBufferLength,
                                         BytesWritten,
                                         BytesNeeded);  
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


#endif // COMPILE_WIN8_ABOVE