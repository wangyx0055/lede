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

NDIS_STATUS
MtkOidQueryVendor(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = 0x00430C00;
            DBGPRINT(RT_DEBUG_TRACE, ("Query:RT_OID_VERDOR (=0x%08x)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }        

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryPhyMode(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = (ULONG) pPort->CommonCfg.PhyMode;
            DBGPRINT(RT_DEBUG_TRACE, ("Query:MTK_OID_N6_QUERY_PHY_MODE (=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryCountryRegion(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = ((UI_MAX_REGION_CODE_IN_A_BAND << 24) | 
                        (UI_MAX_REGION_CODE_IN_BG_BAND << 16) |
                        (pPort->CommonCfg.CountryRegionForABand << 8) | 
                        pPort->CommonCfg.CountryRegion);

            uInfoLen = sizeof(ULONG);

            // Return value format (ULONG):
            // Byte 3 = The maximum country region code in A band that the Ralink UI can display.
            // Byte 2 = The maximum country region code in BG band that the Ralink UI can display.
            // Byte 1 = Country region code in A band.
            // Byte 0 = Country region code in BG band.

            DBGPRINT(RT_DEBUG_TRACE, ("Query:MTK_OID_N6_QUERY_COUNTRY_REGION (UI_MAX_REGION_CODE_IN_A_BAND = %d, UI_MAX_REGION_CODE_IN_BG_BAND = %d, A=%d, BG=%d, 0x%04x)\n", 
                UI_MAX_REGION_CODE_IN_A_BAND, 
                UI_MAX_REGION_CODE_IN_BG_BAND, 
                pPort->CommonCfg.CountryRegionForABand, 
                pPort->CommonCfg.CountryRegion, 
                uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }        

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryEepromVersion(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pAd->HwCfg.EepromVersion;
            DBGPRINT(RT_DEBUG_INFO, ("Query:MTK_OID_N6_QUERY_EEPROM_VERSION (=0x%08x)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryFirmwareVersion(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pAd->HwCfg.FirmwareVersion;
            DBGPRINT(RT_DEBUG_INFO, ("Query:MTK_OID_N6_QUERY_FIRMWARE_VERSION (=0x%08x)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryCurrentChannelId(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = (ULONG) pPort->Channel;
                DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_CURRENT_CHANNEL_ID (=%d)\n", uInfo));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryTxPowerLevel1(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->CommonCfg.TxPowerPercentage;
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_TX_POWER_LEVEL_1 (=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryHardwareRegister(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    RT_802_11_CR_ACCESS  HardwareRegister  = {0};

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_HARDWARE_REGISTER\n"));
            if (InformationBufferLength != sizeof(RT_802_11_CR_ACCESS))
            {
                *BytesNeeded = sizeof(RT_802_11_CR_ACCESS);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {   
                PRT_802_11_CR_ACCESS pHWRegisterFromUI = (PRT_802_11_CR_ACCESS)(pInformationBuffer);

                *BytesWritten = sizeof(RT_802_11_CR_ACCESS);
                *BytesNeeded = sizeof(RT_802_11_CR_ACCESS);
                uInfoLen = sizeof(RT_802_11_CR_ACCESS);

                HardwareRegister.HardwareType = pHWRegisterFromUI->HardwareType;
                HardwareRegister.SetOrQuery = pHWRegisterFromUI->SetOrQuery;
                HardwareRegister.CR = pHWRegisterFromUI->CR;

                pInfo = (PVOID)(&HardwareRegister); // Copy to pInformationBuffer at the end of this function.
#if 1 //(defined(_WIN64) || DBG)        
                ndisStatus = QueryHardWareRegister(pAd, &HardwareRegister);
#endif              
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryLinkStatus(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    RT_802_11_LINK_STATUS LinkStatus;
                    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(RT_802_11_LINK_STATUS))
            {
                *BytesNeeded = sizeof(RT_802_11_LINK_STATUS);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                LinkStatus.CurrTxRate = RateIdTo500Kbps[pPort->CommonCfg.TxRate];   // unit : 500 kbps
                if(P2P_ON(pPort) && IS_P2P_GO_OP(pPort) && pAd->OpMode == OPMODE_STA)
                {
                    LinkStatus.ChannelQuality = 100;
                }
                else
                {
                    LinkStatus.ChannelQuality = pPort->Mlme.ChannelQuality;
                }
                LinkStatus.RxByteCount = pAd->Counter.MTKCounters.ReceivedByteCount;
                LinkStatus.TxByteCount = pAd->Counter.MTKCounters.TransmittedByteCount;
                LinkStatus.CentralChannel = pPort->CentralChannel;
                pInfo = (void *)&LinkStatus;
                uInfoLen = sizeof(RT_802_11_LINK_STATUS);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_LINK_STATUS (CentralChannel=%d)\n", pPort->CentralChannel));
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryNoiseLevel(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = (ULONG) pAd->HwCfg.BbpWriteLatch[17];
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_NOISE_LEVEL (=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryStatistics(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    RT_802_11_STATISTICS Statistics; // Use the NDIS-allocated memory
                    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(RT_802_11_STATISTICS))
            {
                *BytesNeeded = sizeof(RT_802_11_STATISTICS);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                NICUpdateRawCounters(pAd);

                if (pAd->Counter.WlanCounters.TransmittedFragmentCount.QuadPart < pAd->Counter.WlanCounters.RetryCount.QuadPart)
                {
                    pAd->Counter.WlanCounters.TransmittedFragmentCount = pAd->Counter.WlanCounters.RetryCount;
                }

                Statistics.TransmittedFragmentCount = pAd->Counter.WlanCounters.TransmittedFragmentCount;
                Statistics.MulticastTransmittedFrameCount = pAd->Counter.WlanCounters.MulticastTransmittedFrameCount;
                Statistics.FailedCount = pAd->Counter.WlanCounters.FailedCount;
                Statistics.RetryCount = pAd->Counter.WlanCounters.RetryCount;
                Statistics.MultipleRetryCount = pAd->Counter.WlanCounters.MultipleRetryCount;
                Statistics.RTSSuccessCount = pAd->Counter.WlanCounters.RTSSuccessCount;
                Statistics.RTSFailureCount = pAd->Counter.WlanCounters.RTSFailureCount;
                Statistics.ACKFailureCount = pAd->Counter.WlanCounters.ACKFailureCount;
                Statistics.FrameDuplicateCount = pAd->Counter.WlanCounters.FrameDuplicateCount;
                Statistics.ReceivedFragmentCount = pAd->Counter.WlanCounters.ReceivedFragmentCount;
                
                // Use 802.3 packet count instead. (ReceivedFragmentCount include MGMT frame counts)
                //Statistics.RcvSucFrame = pAd->Counter.WlanCounters.ReceivedFragmentCount;
                Statistics.RcvSucFrame = pAd->Counter.Counters8023.GoodReceives;
                Statistics.RxNoBuffer = pAd->Counter.Counters8023.RxNoBuffer;
                
                Statistics.MulticastReceivedFrameCount = pAd->Counter.WlanCounters.MulticastReceivedFrameCount;
                Statistics.FCSErrorCount = pAd->Counter.MTKCounters.RealFcsErrCount;
                Statistics.FCSErrorCount = pAd->Counter.WlanCounters.FCSErrorCount;
                Statistics.FrameDuplicateCount.LowPart = pAd->Counter.WlanCounters.FrameDuplicateCount.LowPart / 100;

                
                Statistics.TxAggRange1Count = pAd->Counter.WlanCounters.TxAggRange1Count;
                Statistics.TxAggRange2Count = pAd->Counter.WlanCounters.TxAggRange2Count;
                Statistics.TxAggRange3Count = pAd->Counter.WlanCounters.TxAggRange3Count;
                Statistics.TxAggRange4Count = pAd->Counter.WlanCounters.TxAggRange4Count;
                // STATS_COUNT_SUPPORT
                Statistics.AmpduSuccessCount =  pAd->Counter.WlanCounters.AmpduSuccessCount;
                Statistics.AmpduFailCount =  pAd->Counter.WlanCounters.AmpduFailCount;


                uInfoLen = sizeof(RT_802_11_STATISTICS);
                pInfo = (PVOID)&Statistics;
                //*BytesWritten = sizeof(RT_802_11_STATISTICS); // Set to the number of bytes it is returning at pInformationBuffer
                DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_STATISTICS \n"));                             
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryExtraInfo(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pAd->UiCfg.ExtraInfo;
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_EXTRA_INFO (=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryLastRxRate(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            //uInfo = pPort->LastRxRate;
            IndicateRateForUi(pAd, &pPort->LastRxRatePhyCfg, pInfo, &uInfoLen);
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_LAST_RX_RATE (=0x%x)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryRssi0(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if ((pAd->HwCfg.BLNAGain != 0xFF) && (pAd->HwCfg.Antenna.field.RssiIndicationMode == USER_AWARENESS_MODE))
            {
                uInfo = pAd->StaCfg.RssiSample.LastRssi[0] + GET_LNA_GAIN(pAd); // RSSI indication by Rx LNA output
        
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_RSSI_0, USER_AWARENESS_MODE, RSSI = %d\n", uInfo));
            }
            else if(P2P_ON(pPort) && IS_P2P_GO_OP(pPort) && pAd->OpMode == OPMODE_STA)
            {
                uInfo = pPort->SoftAP.ApCfg.RssiSample.LastRssi[0]; 
            }
            else
            {
                uInfo = pAd->StaCfg.RssiSample.LastRssi[0]; 
            }
            DBGPRINT(RT_DEBUG_INFO, ("Query::RT_OID_QUERY_RSSI0(=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryRssi1(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (pAd->HwCfg.Antenna.field.RxPath > 1)
            {
                if ((pAd->HwCfg.BLNAGain != 0xFF) && (pAd->HwCfg.Antenna.field.RssiIndicationMode == USER_AWARENESS_MODE))
                {
                    uInfo = pAd->StaCfg.RssiSample.LastRssi[1] + GET_LNA_GAIN(pAd); // RSSI indication by Rx LNA output

                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_RSSI_1, USER_AWARENESS_MODE, RSSI = %d\n", uInfo));
                }
                else if(P2P_ON(pPort) && IS_P2P_GO_OP(pPort) && pAd->OpMode == OPMODE_STA)
                {
                    uInfo = pPort->SoftAP.ApCfg.RssiSample.LastRssi[1]; 
                }
                else
                {
                    uInfo = pAd->StaCfg.RssiSample.LastRssi[1];
                }

                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N5_QUERY_RSSI_1(=%d)\n", uInfo));
            }
            else
            {
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryLastTxRate(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            PHY_CFG RatePhyCfg = {0};           
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &RatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg));
            
            if((!VHT_CAPABLE(pAd)) && 
                (pAd->StaCfg.bIdle300M == TRUE) && 
                ((READ_PHY_CFG_MCS(pAd, &RatePhyCfg) == MCS_15) || 
                  (READ_PHY_CFG_MCS(pAd, &RatePhyCfg) == MCS_7)))
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &RatePhyCfg, GI_400);                   
            }
            

            // ===============================================
            // Ui Tool Feature: Fixed Tx rate in MACVer2 later
            // ===============================================
            // Replace the phy cfg from "RatePhyCfg" to "UI configuration" if "Fixed Tx rate" feature is enable
            // This feature is caused by OID "MTK_OID_N6_SET_FIXED_RATE" and only supported in MACVer2 later.
            if(EXT_MAC_CAPABLE(pAd) && pAd->UiCfg.UtfFixedTxRateEn) 
            {
                WRITE_PHY_CFG_STBC(pAd, &RatePhyCfg, READ_PHY_CFG_STBC(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
                WRITE_PHY_CFG_SHORT_GI(pAd, &RatePhyCfg, READ_PHY_CFG_SHORT_GI(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
                WRITE_PHY_CFG_MCS(pAd, &RatePhyCfg, READ_PHY_CFG_MCS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
                WRITE_PHY_CFG_MODE(pAd, &RatePhyCfg, READ_PHY_CFG_MODE(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
                WRITE_PHY_CFG_BW(pAd, &RatePhyCfg, READ_PHY_CFG_BW(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
                WRITE_PHY_CFG_NSS(pAd, &RatePhyCfg, READ_PHY_CFG_NSS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg)); 
            }

            IndicateRateForUi(pAd, &RatePhyCfg, pInfo, &uInfoLen);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_LAST_TX_RATE (=%x)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryRssi2(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (pAd->HwCfg.Antenna.field.RxPath > 2)
            {
                if ((pAd->HwCfg.BLNAGain != 0xFF) && (pAd->HwCfg.Antenna.field.RssiIndicationMode == USER_AWARENESS_MODE))
                {
                    uInfo = pAd->StaCfg.RssiSample.LastRssi[2] + GET_LNA_GAIN(pAd); // RSSI indication by Rx LNA output

                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_RSSI_2, USER_AWARENESS_MODE, RSSI = %d\n", uInfo));
                }
                else if(P2P_ON(pPort) && IS_P2P_GO_OP(pPort) && pAd->OpMode == OPMODE_STA)
                {
                    uInfo = pPort->SoftAP.ApCfg.RssiSample.LastRssi[2]; 
                }
                else
                {
                    uInfo = pAd->StaCfg.RssiSample.LastRssi[2];
                }

                DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_RSSI_2(=%d)\n", uInfo));
            }
            else
            {
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySnr0(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = ((0xeb - pAd->StaCfg.LastSNR0) * 3) / 16 ;
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_SNR_0(=0x%x)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySnr1(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if ((pAd->HwCfg.Antenna.field.RxPath > 1))
            {
                uInfo = ((0xeb - pAd->StaCfg.LastSNR1) * 3) / 16 ;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_SNR_1(0x=%x)\n", uInfo));
            }
            else
            {
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryConfiguration(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    NDIS_802_11_CONFIGURATION Config;
    NDIS_802_11_CONFIGURATION Configuration;
                
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(NDIS_802_11_CONFIGURATION))
                {
                    *BytesNeeded = sizeof(NDIS_802_11_CONFIGURATION);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    Config.Length = sizeof(NDIS_802_11_CONFIGURATION);
                    Config.BeaconPeriod = pPort->CommonCfg.BeaconPeriod;
                    Config.ATIMWindow = pPort->CommonCfg.AtimWindow;
                    MAP_CHANNEL_ID_TO_KHZ(pPort->Channel, Config.DSConfig);
                    
                    DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_CONFIGURATION (BeacnPeriod=%d,AtimW=%d,Ch=%d)\n",
                            Config.BeaconPeriod, Config.ATIMWindow, pPort->Channel));

                    pInfo = (VOID *)&Config;
                    uInfoLen = sizeof(NDIS_802_11_CONFIGURATION);
                }
            } while (FALSE);
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                Configuration.Length = sizeof(NDIS_802_11_CONFIGURATION);
                Configuration.BeaconPeriod = pPort->CommonCfg.BeaconPeriod;
                Configuration.ATIMWindow = pAd->StaActive.AtimWin;
                MAP_CHANNEL_ID_TO_KHZ(pPort->Channel, Configuration.DSConfig);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CONFIGURATION(BeaconPeriod=%d,AtimW=%d,Channel=%d) \n", 
                         Configuration.BeaconPeriod, Configuration.ATIMWindow, pPort->Channel));
                if (pAd->UiCfg.bPromiscuous == TRUE)
                {
                    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                }
                pInfo = (VOID *)&Configuration;
                uInfoLen = sizeof(NDIS_802_11_CONFIGURATION);
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryAuthenticationMode(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = pPort->PortCfg.AuthMode;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_AUTHENTICATION_MODE(=%d)\n", uInfo));
            } while (FALSE);
        }
        else
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryEncryptionStatus(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = pPort->PortCfg.WepStatus;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_ENCRYPTION_STATUS(=%d)\n", uInfo));
            } while (FALSE);
        }
        else
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryEventTable(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(RT_802_11_EVENT_TABLE))
                {
                    *BytesNeeded = sizeof(RT_802_11_EVENT_TABLE);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    pInfo = (UCHAR *)&pAd->TrackInfo.EventTab;
                    uInfoLen = 8 + pAd->TrackInfo.EventTab.Num * sizeof(RT_802_11_EVENT_LOG);
                }
                DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_EVENT_TABLE \n"));
            } while (FALSE);
        }
        else
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryMacTable(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength < sizeof(RT_802_11_MAC_TABLE))
                {
                    *BytesNeeded = sizeof(RT_802_11_MAC_TABLE);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    PRT_802_11_MAC_TABLE pMacTab = (PRT_802_11_MAC_TABLE)(pInformationBuffer); // Use the NDIS-allocated memory
                    PMAC_TABLE_ENTRY pMacEntry = NULL;
                    PMAC_TABLE_ENTRY pNextMacEntry = NULL;                    
                    PQUEUE_HEADER pHeader;           
                    pMacTab->Num=0;

                    pHeader = &pPort->MacTab.MacTabList;
                    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                    while (pNextMacEntry != NULL)
                    {
                        pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                        
                        if(pMacEntry == NULL)
                        {
                            break; 
                        }
                       
                        if (pMacEntry->ValidAsCLI && (pMacEntry->Sst == SST_ASSOC))
                        {
                            COPY_MAC_ADDR(pMacTab->Entry[pMacTab->Num].Addr, pMacEntry->Addr);
                            pMacTab->Entry[pMacTab->Num].Aid = (UCHAR)pMacEntry->Aid;
                            pMacTab->Entry[pMacTab->Num].Psm = pMacEntry->PsMode;
                            pMacTab->Entry[pMacTab->Num].TxRate.DoubleWord = READ_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->TxPhyCfg);
                            pMacTab->Num += 1;
                        }
                        
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                    }

                    uInfoLen = 0; // Skip the memory movement at the end of this function
                    *BytesWritten = (sizeof(pMacTab->Num) + (pMacTab->Num * sizeof(RT_802_11_MAC_ENTRY))); // Set to the number of bytes it is returning at pInformationBuffer
                    DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_MAC_TABLE (Num=%d)\n", pMacTab->Num));
                }
            } while (FALSE);
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength < sizeof(RT_802_11_MAC_TABLE))
                {
                    *BytesNeeded = sizeof(RT_802_11_MAC_TABLE);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    PRT_802_11_MAC_TABLE pMacTab = (PRT_802_11_MAC_TABLE)(pInformationBuffer); // Use the NDIS-allocated memory
                    PMAC_TABLE_ENTRY pMacEntry = NULL;
                    PMAC_TABLE_ENTRY pNextMacEntry = NULL;      
                    PQUEUE_HEADER pHeader;
                    
                    pMacTab->Num=0;

                    pHeader = &pPort->MacTab.MacTabList;
                    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                    while (pNextMacEntry != NULL)
                    {
                        pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                        
                        if(pMacEntry == NULL)
                        {
                            break; 
                        }
                        
                        if (pMacEntry->ValidAsCLI && (pMacEntry->Sst == SST_ASSOC))
                        {
                            COPY_MAC_ADDR(pMacTab->Entry[pMacTab->Num].Addr, pMacEntry->Addr);
                            pMacTab->Entry[pMacTab->Num].Aid = (UCHAR)pMacEntry->Aid;
                            pMacTab->Entry[pMacTab->Num].Psm = pMacEntry->PsMode;
                            pMacTab->Entry[pMacTab->Num].TxRate.DoubleWord = READ_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->TxPhyCfg);
                            pMacTab->Num += 1;
                        }
                        
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                    }

                    uInfoLen = 0; // Skip the memory movement at the end of this function
                    *BytesWritten = (sizeof(pMacTab->Num) + (pMacTab->Num * sizeof(RT_802_11_MAC_ENTRY))); // Set to the number of bytes it is returning at pInformationBuffer
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_MAC_TABLE (Num=%d)\n", pMacTab->Num));
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryOpMode(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pAd->OpMode;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_OP_MODE(=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryApConfig(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    RT_802_11_AP_CONFIG ApConfig;
                    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(RT_802_11_AP_CONFIG))
                {
                    *BytesNeeded = sizeof(RT_802_11_AP_CONFIG);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N5_QUERY_AP_CONFIG\n"));
                    ApConfig.EnableTxBurst = pPort->CommonCfg.bEnableTxBurst;
                    ApConfig.EnableTurboRate = 0;
                    ApConfig.IsolateInterStaTraffic = pPort->SoftAP.ApCfg.bIsolateInterStaTraffic;
                    ApConfig.HideSsid = pPort->SoftAP.ApCfg.bHideSsid;
                    ApConfig.UseBGProtection = pPort->CommonCfg.UseBGProtection;
                    ApConfig.UseShortSlotTime = pPort->CommonCfg.bUseShortSlotTime;
                    ApConfig.AgeoutTime = pPort->SoftAP.ApCfg.AgeoutTime;
                    ApConfig.Rsv1 = 0;
                    ApConfig.SystemErrorBitmap = pAd->UiCfg.SystemErrorBitmap;

                    pInfo = &ApConfig;
                    uInfoLen = sizeof(RT_802_11_AP_CONFIG);
                }
            } while (FALSE);
        }
        else
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryBatable(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    PQUERYBA_TABLE pBAT = NULL;
                
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            UCHAR   TotalEntry,j, index;

            PlatformAllocateMemory(pAd,  &pBAT, sizeof(QUERYBA_TABLE));
            if (pBAT != NULL)
            {
                PMAC_TABLE_ENTRY pMacEntry = NULL;
                PMAC_TABLE_ENTRY pNextMacEntry = NULL;                    
                PQUEUE_HEADER pHeader;
                
                PlatformZeroMemory(pBAT, sizeof(QUERYBA_TABLE));
                //
                TotalEntry = pPort->MacTab.Size;
                index = 0;

                pHeader = &pPort->MacTab.MacTabList;
                pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                while (pNextMacEntry != NULL)
                {
                    pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                    
                    if(pMacEntry == NULL)
                    {
                        break; 
                    }
                    
                    if ((pMacEntry->ValidAsCLI == TRUE) && (pMacEntry->TXBAbitmap))
                    {
                        PlatformMoveMemory(pBAT->BAOriEntry[index].MACAddr, pMacEntry->Addr, 6);
                        pBAT->BAOriEntry[index].BaBitmap = (UCHAR)pMacEntry->TXBAbitmap;
                        for (j = 0;j < 8;j++)
                        {
                            pBAT->BAOriEntry[index].BufSize[j] = pPort->BATable.BAOriEntry[pMacEntry->BAOriWcidArray[j]].BAWinSize;
                            pBAT->BAOriEntry[index].ORI_BA_Status[j] = pPort->BATable.BAOriEntry[pMacEntry->BAOriWcidArray[j]].ORI_BA_Status;
                            DBGPRINT(RT_DEBUG_TRACE,("BAT.BAOriEntry[%d].BufSize[%d]  = %d \n", index, j, pBAT->BAOriEntry[index].BufSize[j]));
                        }
                        // roughly total number.
                        DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_BATABLE OriNum =%x : %x : %x: %x:%x:%x  )\n", 
                            pBAT->BAOriEntry[index].MACAddr[0],pBAT->BAOriEntry[index].MACAddr[1],pBAT->BAOriEntry[index].MACAddr[2],
                            pBAT->BAOriEntry[index].MACAddr[3],pBAT->BAOriEntry[index].MACAddr[4],pBAT->BAOriEntry[index].MACAddr[5]));
                        //DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_BATABLE OriStatus =%x : %x : %x: %x:%x:%x\n", 
                            //pBAT->BAOriEntry[i].ORI_BA_Status[0],pBAT->BAOriEntry[i].ORI_BA_Status[1],pBAT->BAOriEntry[i].ORI_BA_Status[2],
                            //pBAT->BAOriEntry[i].ORI_BA_Status[3],pBAT->BAOriEntry[i].ORI_BA_Status[4],pBAT->BAOriEntry[i].ORI_BA_Status[5]));
                        TotalEntry--;
                        index++;
                        pBAT->OriNum++;
                    }
                    
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                }

                // 
                TotalEntry = pPort->MacTab.Size;
                index = 0;

                pHeader = &pPort->MacTab.MacTabList;
                pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                while (pNextMacEntry != NULL)
                {
                    pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                    
                    if(pMacEntry == NULL)
                    {
                        break; 
                    }
                    
                    if ((pMacEntry->ValidAsCLI == TRUE) && (pMacEntry->RXBAbitmap))
                    {
                        PlatformMoveMemory(pBAT->BARecEntry[index].MACAddr, pMacEntry->Addr, 6);
                        pBAT->BARecEntry[index].BaBitmap = (UCHAR)pMacEntry->RXBAbitmap;
                        for (j = 0;j < 8;j++)
                        {
                            pBAT->BARecEntry[index].BufSize[j] = pPort->BATable.BARecEntry[pMacEntry->BARecWcidArray[j]].BAWinSize;
                            if (pBAT->BARecEntry[index].BufSize[j] >64 )
                            {
                                pBAT->BARecEntry[index].BufSize[j] = 64;
                            }
                            if (pMacEntry->RXBAbitmap & (1<<j))
                            {
                                pBAT->BARecEntry[index].REC_BA_Status[j] = Recipient_Accept;
                            }

                            DBGPRINT(RT_DEBUG_TRACE,("BAT.BARecEntry[%d].BufSize[%d]  = %d \n", index, j, pBAT->BARecEntry[index].BufSize[j]));                         
                        }
                        // roughly total number.
                        TotalEntry--;
                        index++;
                        pBAT->RecNum++;
                    }
                    
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                }
                
                pInfo = (VOID *)pBAT;
                uInfoLen = sizeof(QUERYBA_TABLE);
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed for MTK_OID_N6_QUERY_BATABLE\n", __FUNCTION__));
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    if (pBAT != NULL)
    {
        PlatformFreeMemory(pBAT, sizeof(QUERYBA_TABLE));
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryImmeBaCap(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    OID_BACAP_STRUC BACap;
                
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            BACap.TxBAWinLimit = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;
            BACap.RxBAWinLimit = (UCHAR)pPort->CommonCfg.BACapability.field.RxBAWinLimit;
            BACap.AutoBA = (BOOLEAN)pPort->CommonCfg.BACapability.field.AutoBA;
            BACap.Policy = (UCHAR)pPort->CommonCfg.BACapability.field.Policy;
            BACap.MpduDensity= (UCHAR)pPort->CommonCfg.BACapability.field.MpduDensity;
            BACap.AmsduSize = (UCHAR)pPort->CommonCfg.HtCapability.HtCapInfo.AMsduSize;
            BACap.AmsduEnable = (UCHAR)pPort->CommonCfg.DesiredHtPhy.AmsduEnable;
            BACap.MMPSmode = (UCHAR)pPort->CommonCfg.HtCapability.HtCapInfo.MimoPs;
            pInfo = (VOID *)&BACap;
            uInfoLen = sizeof(BACap);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_IMME_BA_CAP (Policy=%d)\n", BACap.Policy));
            DBGPRINT(RT_DEBUG_TRACE, (" (TxBAWinLimit=%d) (RxBAWinLimit=%d) (MpduDensity=%d)\n", BACap.TxBAWinLimit, BACap.RxBAWinLimit, BACap.MpduDensity));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryHtPhymode(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    OID_SET_HT_PHYMODE HTPhyMode;  
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

    if(pBssidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    } 
                
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            HTPhyMode.PhyMode   = pPort->CommonCfg.PhyMode;
            HTPhyMode.HtMode    = (UCHAR)READ_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg);
            HTPhyMode.BW        = (UCHAR)READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg);
            
            if(HTPhyMode.PhyMode == PHY_11VHT)
            {
                HTPhyMode.MCS = ((UCHAR)READ_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg)) & 0x0F;
                HTPhyMode.MCS |= ((UCHAR)READ_PHY_CFG_NSS(pAd, &pBssidMacTabEntry->TxPhyCfg)) << 4;
            }
            else
            {
                HTPhyMode.MCS = (UCHAR)READ_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg);
            }
            
            HTPhyMode.SHORTGI            = (UCHAR)READ_PHY_CFG_SHORT_GI(pAd, &pBssidMacTabEntry->TxPhyCfg);
            HTPhyMode.STBC               = (UCHAR)READ_PHY_CFG_STBC(pAd, &pBssidMacTabEntry->TxPhyCfg);
            HTPhyMode.ExtOffset          = ((pPort->CentralChannel < pPort->Channel) ? (EXTCHA_BELOW) : (EXTCHA_ABOVE));
            HTPhyMode.CentralChannelSeg1 = pPort->CentralChannel;
            
            pInfo = (VOID *)&HTPhyMode;
            uInfoLen = sizeof(OID_SET_HT_PHYMODE);

            DBGPRINT(RT_DEBUG_TRACE, ("Query:: MTK_OID_N6_SET_HT_PHYMODE -- OidBuf(PhyMode=%d, TransmitNo=%d, HtMode=%d, ExtOffset=%d, MCS index=%d, <Nss in VHT=%d>, BW=%d, STBC=%d, SHORTGI=%d, <VhtCentralChannel in VHT=%d>) \n", 
                                HTPhyMode.PhyMode, 
                                HTPhyMode.TransmitNo, 
                                HTPhyMode.HtMode, 
                                HTPhyMode.ExtOffset,
                                ( (HTPhyMode.PhyMode == PHY_11VHT)? (HTPhyMode.MCS & 0x0F):(HTPhyMode.MCS) ),
                                ( (HTPhyMode.PhyMode == PHY_11VHT)? (HTPhyMode.MCS & 0xF0):(0) ),
                                HTPhyMode.BW, 
                                HTPhyMode.STBC, 
                                HTPhyMode.SHORTGI,
                                HTPhyMode.CentralChannelSeg1
                                ));

            DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (.word = %x ) RegTransmitSetting.field.STBC = %d, RegTransmitSetting.field.ShortGI = %d\n", 
                                        READ_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->TxPhyCfg),
                                        READ_PHY_CFG_STBC(pAd, &pBssidMacTabEntry->TxPhyCfg),
                                        READ_PHY_CFG_SHORT_GI(pAd, &pBssidMacTabEntry->TxPhyCfg)
                                        ));
            
            DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (.word = %x )\n", 
                READ_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->TxPhyCfg)));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWpaRekey(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength < sizeof(RT_802_11_WPA_REKEY))
                {
                    *BytesNeeded = sizeof(RT_802_11_WPA_REKEY);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    pInfo = &pPort->SoftAP.ApCfg.WPAREKEY;
                    uInfoLen = sizeof(RT_802_11_WPA_REKEY);
                }
                DBGPRINT(RT_DEBUG_TRACE, (" Query :: MTK_OID_N6_QUERY_WPA_REKEY\n"));
            } while (FALSE);
        }
        else
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryStaConfig(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    RT_802_11_STA_CONFIG StaConfig;
                    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(RT_802_11_STA_CONFIG))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    StaConfig.bEnableTxBurst = pPort->CommonCfg.bEnableTxBurst;
                    StaConfig.EnableTurboRate = 0;
                    StaConfig.UseBGProtection = pPort->CommonCfg.UseBGProtection;
                    StaConfig.UseShortSlotTime = pPort->CommonCfg.bUseShortSlotTime;
                    StaConfig.AdhocMode = pAd->StaCfg.AdhocMode;
                    StaConfig.HwRadioStatus = (pAd->StaCfg.bHwRadio == TRUE) ? 1 : 0;
                    StaConfig.Rsv1 = (pAd->UiCfg.bPromiscuous == TRUE) ? 1 : 0;
                    StaConfig.SystemErrorBitmap = pAd->UiCfg.SystemErrorBitmap;
                    pInfo = &StaConfig;
                    uInfoLen = sizeof(RT_802_11_STA_CONFIG);
                    DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_STA_CONFIG\n"));
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryPreamble(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->CommonCfg.TxPreamble;
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_PREAMBLE(=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryApsdSetting(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                ULONG apsd = 0;
                apsd = (pPort->CommonCfg.bAPSDCapable | (pPort->CommonCfg.bAPSDAC_BE << 1) | (pPort->CommonCfg.bAPSDAC_BK << 2)
                    | (pPort->CommonCfg.bAPSDAC_VI << 3) | (pPort->CommonCfg.bAPSDAC_VO << 4) | (pPort->CommonCfg.MaxSPLength << 5));

                uInfo = *(ULONG *)&apsd;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_APSD_SETTING (=0x%x, APSDCap=%d, AC_BE=%d, AC_BK=%d, AC_VI=%d, AC_VO=%d, MAXSPLen=%d)\n", 
                    uInfo,
                    pPort->CommonCfg.bAPSDCapable,
                    pPort->CommonCfg.bAPSDAC_BE,
                    pPort->CommonCfg.bAPSDAC_BK,
                    pPort->CommonCfg.bAPSDAC_VI,
                    pPort->CommonCfg.bAPSDAC_VO,
                    pPort->CommonCfg.MaxSPLength));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryApsdPsm(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = (ULONG) pPort->CommonCfg.bAPSDForcePowerSave;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_APSD_PSM (=%d)\n", uInfo));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWmm(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = *(BOOLEAN *)&pPort->CommonCfg.bWmmCapable;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_QUERY_WMM (=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryDls(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = pPort->CommonCfg.bDLSCapable;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_DLS(=%d)\n", uInfo));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryDlsParam(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    RT_802_11_DLS_UI Dls[MAX_NUM_OF_DLS_ENTRY];
                
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != (sizeof(RT_802_11_DLS_UI) * MAX_NUM_OF_DLS_ENTRY))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    for (i = 0; i < MAX_NUM_OF_DLS_ENTRY; i++)
                    {
                        if (TDLS_ON(pAd))
                        {
                            PlatformMoveMemory(&Dls[i], &pAd->StaCfg.TDLSEntry[i], sizeof(RT_802_11_DLS_UI));
                            // TDLS fits to DLS status
                            if (Dls[i].Valid && Dls[i].Status == TDLS_MODE_CONNECTED)
                            {
                                Dls[i].Status = DLS_FINISH;
                        }
                        }
                        else
                        {
                            PlatformMoveMemory(&Dls[i], &pAd->StaCfg.DLSEntry[i], sizeof(RT_802_11_DLS_UI));
                        }
                    }
                    pInfo = Dls;
                    uInfoLen = (sizeof(RT_802_11_DLS_UI) * MAX_NUM_OF_DLS_ENTRY);
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_DLS_PARAM\n"));
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryMfpControl(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_ERROR, ("Query::MTK_OID_N6_QUERY_MFP_CONTROL \n"));
            pInfo = &(pAd->StaCfg.PmfCfg.PmfControl);
            *BytesWritten = uInfoLen = sizeof(PMF_CONTROL);
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryPortUsage(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_INFO, ("Qy::MTK_OID_N6_QUERY_PORT_USAGE!!!  \n"));
            if (InformationBufferLength != sizeof(ULONG))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {   
                /*              
                    0x00000001              Port0_Enabled
                    0x00000002              Port1_Enabled
                    0x00000004              Port2_Enabled
                        :                           :
                    the first byte is for specific features 
                    0x40000000              Support p2p mac info query in Win8                  
                    0x80000000              Support Vista Soft AP feature in Win7               
                */
                    
                uInfo  = 0x00000001;    //  Port0 enabled
                uInfo |= 0x80000000;    // Support SoftAP
                for(i = 1; i < RTMP_MAX_NUMBER_OF_PORT; i++)
                {
                    if ((pAd->PortList[i] != NULL) && (pAd->PortList[i]->bActive == TRUE))
                    {
                        uInfo |= (1 << i);  //  Port#i enabled
                }
                }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                if (NDIS_WIN8_ABOVE(pAd))
                {
                    uInfo |= 0x40000000;    // Support p2p mac info query
                    uInfo |= 0xA0000000;  //0xA0000000              not support Ralink P2P, it only happened in USB Win8 driver 
                }
#endif
    
                DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_QUERY_PORT_USAGE : %x \n", uInfo));
                
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryUsbVersion(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                //check usb max bulk out packet size to judge if usb version is 1.1 or 2.0
                if ( pAd->pHifCfg->BulkOutMaxPacketSize < 512)
                {
                    uInfo = 1;
                }
                else
                {
                    uInfo = 2;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_USB_VERSION(=%d)\n", uInfo));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryInitializationState(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = *(PBOOLEAN)&pPort->CommonCfg.bInitializationState;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_INITIALIZATION_STATE(=%d)\n", pPort->CommonCfg.bInitializationState));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryAironetipInfo(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryCcxRxAirone(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWmmAcmCmd(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryManualChList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != 3*sizeof(USHORT))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                pInfo =  (PVOID)pAd->HwCfg.EEPROMManualChList;
                
                uInfoLen = 3*sizeof(USHORT);
    
                DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_QUERY_MANUAL_CH_LIST : %04x-%04x-%04x\n",
                    *((PUSHORT)pInfo),
                    *((PUSHORT)pInfo + 1),
                    *((PUSHORT)pInfo + 2)));
                
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryBtHciGetCmd(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryBtHciGetAclData(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryEepromBandwidth(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            do
            {
                // EXPAND_FUNC NONE
#if DBG
                PUCHAR DbgMsg;
#endif
                // Based on EEPROM 0x37, return BW40 is supported or not.
                // 0x0: BW40 NOT allowed in 2.4G and 5G
                // 0x1: BW40 allowed in 2.4G, NOT allowed in 5G.
                // 0x2: BW40 NOT allowed in 2.4G, allowed in 5G.
                // 0x3: BW40 allowed in 2.4G and 5G
                uInfo = pAd->HwCfg.NicConfig2.field.bDisableBW40ForG | (pAd->HwCfg.NicConfig2.field.bDisableBW40ForA << 1);
                uInfo = ~(0xFFFFFFFC | uInfo);
#if DBG
                switch(uInfo)
                {
                    case 0x0: 
                        DbgMsg = "BW40 is NOT allowed in both 2.4G and 5G.\n";
                        break;
                    case 0x1: 
                        DbgMsg = "BW40 is allowed in 2.4G, but NOT in 5G.\n";
                        break;
                    case 0x2: 
                        DbgMsg = "BW40 is allowed in 5G only.\n";
                        break;
                    case 0x3: 
                        DbgMsg = "BW40 is allowed in both 2.4G and 5G.\n";
                        break;
        
                    default:
                        DbgMsg = "WARNING!! Invalid value return to MTK_OID_N6_QUERY_EEPROM_BANDWIDTH!!\n";
                }           
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_EEPROM_BANDWIDTH(=0x%08x), %s\n", uInfo, DbgMsg));
#endif
            } while (FALSE);
        }
        else
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryP2pStatus(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(RT_OID_QUERY_P2P_STATUS_STRUCT))
            {
                *BytesNeeded = sizeof(RT_OID_QUERY_P2P_STATUS_STRUCT);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                DBGPRINT(RT_DEBUG_TRACE, ("Qy::RT_OID_Q_P2P_STATUS. Len too short.(=%d). Need =%d\n", InformationBufferLength, sizeof(RT_OID_QUERY_P2P_STATUS_STRUCT)));
            }
            else 
            {
                PUCHAR              ptemp;
                PWSC_CREDENTIAL     pProfile;
                UCHAR               i;
                OID_DISCOVERYENTRY *pDiscovery;
                
                pInfo =  (PVOID)&pPort->P2PCfg.P2PQueryStatusOid;
                ptemp = (PUCHAR)pInfo;
                *BytesWritten = uInfoLen = sizeof(RT_OID_QUERY_P2P_STATUS_STRUCT);
                pProfile = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.Profile;
                DBGPRINT(RT_DEBUG_INFO, ("Qy::Buffer ==>  \n") );
                for (i=0;i<128;)
                {
                    DBGPRINT(RT_DEBUG_INFO, ("  0x%x  0x%x 0x%x 0x%x\n", *ptemp, *(ptemp+1), *(ptemp+2), *(ptemp+3)));
                    i = i +4;
                    ptemp+= 4;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_STATUS(P2PStatus=%d)\n", pPort->P2PCfg.P2PQueryStatusOid.P2PStatus));
                if (pPort->P2PCfg.P2PQueryStatusOid.P2PStatus == P2PEVENT_STATUS_UPDATE_DISCOVERY)
                {
                    DBGPRINT(RT_DEBUG_INFO, ("Qy::MTK_OID_N6_QUERY_P2P_STATUS(TotalNumber =%d)\n", pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.TotalNumber));
                    for (i=0;i<3;i++)
                    {
                        pDiscovery = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[i];
                        DBGPRINT(RT_DEBUG_INFO, ("   %d  = %c%c...\n", pDiscovery->DeviceNameLen,pDiscovery->DeviceName[0],pDiscovery->DeviceName[1]));
                    }
                        
                }
                else if (pPort->P2PCfg.P2PQueryStatusOid.P2PStatus == P2PEVENT_POPUP_SETTING_WINDOWS)
                {
    
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::(  =%x)\n", pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.ConfigMethod));
                        
                }
                else if (pPort->P2PCfg.P2PQueryStatusOid.P2PStatus == P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_STATUS(NumberOfPeer =%d)\n", pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer));
                    
                }
                else if (pPort->P2PCfg.P2PQueryStatusOid.P2PStatus == P2PEVENT_GEN_SEUDO_ADHOC_PROFILE)
                {
                    NDIS_802_11_SSID    *uSsid;
                    uSsid = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.SeudoAdhoc.uSsid;
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_STATUS(SsidLength =%d)\n", pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.SeudoAdhoc.uSsid.SsidLength));
                    DBGPRINT(RT_DEBUG_TRACE, ("Ssid = %x. %x %x %x %x %x  \n",uSsid->Ssid[0], uSsid->Ssid[1],uSsid->Ssid[2],uSsid->Ssid[3],uSsid->Ssid[4],uSsid->Ssid[5]));         
                        
                }
                else if (pPort->P2PCfg.P2PQueryStatusOid.P2PStatus == P2PEVENT_STATUS_WPS_DONE)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::bIsPersistent = %d\n",pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.bIsPersistent));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::SsidLen = %d\n",pProfile->SSID.SsidLength));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::Ssid = %s \n",pProfile->SSID.Ssid));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::AuthType = 0x%x\n",pProfile->AuthType));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::EncrType = %d\n",pProfile->EncrType));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::KeyIndex = %d\n",pProfile->KeyIndex));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::KeyLength = %d\n",pProfile->KeyLength));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::Key ==>\n"));
                    for (i=0;i<16;)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("  0x%x  0x%x 0x%x 0x%x\n",pProfile->Key[i], pProfile->Key[i+1], pProfile->Key[i+2],pProfile->Key[i+3]));
                        i = i+4;
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::<===Key =\n"));
                    DBGPRINT(RT_DEBUG_TRACE, ("Qy::MacAddr = %x %x %x %x %x %x\n",pProfile->MacAddr[0], pProfile->MacAddr[1], pProfile->MacAddr[2],pProfile->MacAddr[3],pProfile->MacAddr[4],pProfile->MacAddr[5]));
                }
                else
                {
                }
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_STATUS(=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWifiDirectLinkstate(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if(pAd->OpMode == OPMODE_STAP2P)
            {
                ndisStatus = NDIS_STATUS_FAILURE;
            }
            else
            {
                uInfo = *(ULONG *)&pPort->P2PCfg.LinkState;
                DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_WIFI_DIRECT_LINKSTATE (=%d)\n", uInfo));
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryP2pMacInfo(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    RT_OID_QUERY_P2P_MAC_INFO_STRUCT    P2pMacInfo;
                    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(RT_OID_QUERY_P2P_MAC_INFO_STRUCT))
            {
                *BytesNeeded = sizeof(RT_OID_QUERY_P2P_MAC_INFO_STRUCT);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_MAC_INFO. Len too short.(=%d). Need =%d\n", InformationBufferLength, sizeof(RT_OID_QUERY_P2P_MAC_INFO_STRUCT)));
            }
            else
            {
                PlatformZeroMemory(&P2pMacInfo, sizeof(RT_OID_QUERY_P2P_MAC_INFO_STRUCT));
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                if (NDIS_WIN8_ABOVE(pAd))
                {
                    if (pAd->MacAddressUsed[MAX_NUM_OF_TOTAL_WFD_ROLE - 1] == TRUE)
                    {
                        // start from port#1
                        for (i=1; i < RTMP_MAX_NUMBER_OF_PORT; i++)
                        {
                            if ((pAd->PortList[i] == NULL) || (pAd->PortList[i]->bActive == FALSE))
                            {
                                continue;
                            }

                            if (MAC_ADDR_EQUAL(pAd->PortList[i]->CurrentAddress, pAd->MacAddressList[MAX_NUM_OF_TOTAL_WFD_ROLE - 1]))
                            {
                                P2pMacInfo.bActive = TRUE;
                                P2pMacInfo.PortNumber = (ULONG)pAd->PortList[i]->PortNumber;
                                PlatformMoveMemory(&P2pMacInfo.MacAddr, pAd->PortList[i]->CurrentAddress, MAC_ADDRESS_LENGTH);
                                DBGPRINT(RT_DEBUG_TRACE, ("[win8]Qy::bActive= %d\n", P2pMacInfo.bActive));
                                DBGPRINT(RT_DEBUG_TRACE, ("[win8]Qy::PortNumber= %d, %d\n", P2pMacInfo.PortNumber, i));
                                DBGPRINT(RT_DEBUG_TRACE, ("[win8]Qy::MAC= %x %x %x %x %x %x \n", 
                                        P2pMacInfo.MacAddr[0], P2pMacInfo.MacAddr[1], P2pMacInfo.MacAddr[2],
                                        P2pMacInfo.MacAddr[3], P2pMacInfo.MacAddr[4], P2pMacInfo.MacAddr[5]));
                                break;
                            }
                        }
                    }
                }
                else
#endif
                {
                    // win7: p2p virtual port is default to port 2
                    if (PORTV2_P2P_ON(pAd, pAd->PortList[PORT_2]))
                    {
                        P2pMacInfo.bActive = TRUE;
                        P2pMacInfo.PortNumber = PORT_2;
                        PlatformMoveMemory(&P2pMacInfo.MacAddr, pAd->PortList[PORT_2]->CurrentAddress, MAC_ADDRESS_LENGTH);
                        DBGPRINT(RT_DEBUG_TRACE, ("[win7]Qy::bActive= %d\n", P2pMacInfo.bActive));
                        DBGPRINT(RT_DEBUG_TRACE, ("[win7]Qy::PortNumber= %d, %d\n", P2pMacInfo.PortNumber, PORT_2));
                        DBGPRINT(RT_DEBUG_TRACE, ("[win7]Qy::MAC= %x %x %x %x %x %x \n", 
                                P2pMacInfo.MacAddr[0], P2pMacInfo.MacAddr[1], P2pMacInfo.MacAddr[2],
                                P2pMacInfo.MacAddr[3], P2pMacInfo.MacAddr[4], P2pMacInfo.MacAddr[5]));
                    }
                }
    
                pInfo = (PVOID)&P2pMacInfo;
                *BytesWritten = uInfoLen = sizeof(RT_OID_QUERY_P2P_MAC_INFO_STRUCT);                    
                ndisStatus = NDIS_STATUS_SUCCESS;
                DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_MAC_INFO (size=%d)", sizeof(RT_OID_QUERY_P2P_MAC_INFO_STRUCT)));
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryP2pPersist(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_PERSIST. Len too short.(=%d). Need =%d\n", InformationBufferLength, sizeof(ULONG)));
            }
            else
            {
                uInfo = pPort->CommonCfg.P2pControl.field.EnablePresistent;
                DBGPRINT(RT_DEBUG_TRACE, ("Qy::MTK_OID_N6_QUERY_P2P_PERSIST EnablePresistent = %d", pPort->CommonCfg.P2pControl.field.EnablePresistent));
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySigmaP2pStatus(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT);
                DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_SIGMA_P2P_STATUS. Size isn't correct.(=%d  %d)\n", InformationBufferLength, sizeof(RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT)));
            }
            else
            {                   
                pInfo =  (PVOID)&pPort->P2PCfg.SigmaQueryStatus;
    
                *BytesWritten = uInfoLen = sizeof(RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT);                    
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_SIGMA_P2P_STATUS (size=%d)\n", sizeof(RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT)));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySigmaWpsPincode(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != 8*sizeof(UCHAR))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded =  8*sizeof(UCHAR);
                DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_SIGMA_WPS_PINCODE. Size isn't correct.(=%d  %d)\n", InformationBufferLength, 8*sizeof(ULONG)));
            }
            else
            {                   
                pInfo =  (PVOID)pPort->P2PCfg.PinCode;
                //pInfo =  (PVOID)pPort->StaCfg.WscControl.RegData.PIN;
                DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_SIGMA_WPS_PINCODE. (=%x %x )\n", *(PULONG)&pPort->P2PCfg.PinCode[0], *(PULONG)&pPort->P2PCfg.PinCode[3]));
    
                *BytesWritten = uInfoLen = 8*sizeof(UCHAR);                 
                ndisStatus = NDIS_STATUS_SUCCESS;
            }     
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySigmaPsk(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != 64*sizeof(UCHAR))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = 64*sizeof(UCHAR);
                DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_SIGMA_PSK. Size isn't correct.(=%d  %d)\n", InformationBufferLength, 64*sizeof(ULONG)));
            }
            else
            {   
                if (P2P_ON(pPort))        // P2P + WPS
                {
                    pInfo =  (PVOID)pPort->P2PCfg.PhraseKey;
                }
                else        // WPS
                {
                    pInfo =  (PVOID)pPort->StaCfg.WscControl.WscProfile.Profile[0].Key;
                }
    
                DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_SIGMA_PSK. (key=%s , len=%d)\n", 
                        pPort->StaCfg.WscControl.WscProfile.Profile[0].Key, pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyLength));
                
                *BytesWritten = uInfoLen = 64*sizeof(UCHAR);                    
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryBaldEagleSsid(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    NDIS_802_11_SSID TempSSID;
                
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(NDIS_802_11_SSID))
                {
                    *BytesNeeded = sizeof(NDIS_802_11_SSID);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_BALD_EAGLE_SSID, ndisStatus = NDIS_STATUS_INVALID_LENGTH\n"));
                }
                else
                {               
                    PlatformZeroMemory(&TempSSID, sizeof(NDIS_802_11_SSID));
                    pInfo = &TempSSID;
                    uInfoLen = sizeof(NDIS_802_11_SSID);
                    if(pAd->StaCfg.BaldEagle.EnableBladEagle)
                    {
                        TempSSID.SsidLength = pAd->StaCfg.BaldEagle.SsidLen;
                        PlatformMoveMemory(TempSSID.Ssid, pAd->StaCfg.BaldEagle.Ssid, pAd->StaCfg.BaldEagle.SsidLen);
                        DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_BALD_EAGLE_SSID(=%s)\n", TempSSID.Ssid));
                    }
                    else
                    {   
                        TempSSID.SsidLength = 0;
                        PlatformZeroMemory(TempSSID.Ssid, sizeof(TempSSID.Ssid));
                        DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_BALD_EAGLE_SSID, but it's disabled.\n"));
                    }               
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryExtregEventMessage(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (pPort->CommonCfg.RTMessageEvent.Lock == TRUE)
                {
                    ndisStatus = NDIS_STATUS_RESOURCES;
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_EXTREG_EVENT_MESSAGE(=%08x)\n", ndisStatus));
                }
                else if (InformationBufferLength < pPort->CommonCfg.RTMessageEvent.MessageLogLength)
                {
                    *BytesNeeded = pPort->CommonCfg.RTMessageEvent.MessageLogLength;
                    ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_EXTREG_EVENT_MESSAGE(=%08x), MsgLen:%d\n", ndisStatus, pPort->CommonCfg.RTMessageEvent.MessageLogLength));              
                }
                else
                {
                    pInfo = pPort->CommonCfg.RTMessageEvent.MessageLog;
                    uInfoLen = pPort->CommonCfg.RTMessageEvent.MessageLogLength;
                    pPort->CommonCfg.RTMessageEvent.Reset = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_EXTREG_EVENT_MESSAGE, size = %d\n", uInfoLen));
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySuppressScan(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pAd->StaCfg.OzmoDeviceSuppressScan;
            DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_SUPPRESS_SCAN(=%d)\n", uInfo));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWifiChiptype(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryTdtPeriodic(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryTdtOneshotCalibration(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryTdtOneshotPacket(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryTdtOneshotSelectRatetableName(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryStbcRxCount(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryStbcTxCount(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySsNum(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->CommonCfg.NumberOfSS;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N5_QUERY_SS_NUM(=0x%08x)\n", uInfo)); 
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQuerySnr2(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscVersion(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = (ULONG)pAd->StaCfg.WSCVersion2;
                uInfoLen = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_VERSION = %d, InformationBufferLength:%d\n", uInfo, InformationBufferLength));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscExtregPbcEnroMac(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    UCHAR Zero[6] = {0};
    EXTREG_PBC_ENRO_MAC PBCEnro;
    PBCEnro.MACs  = NULL;
    PBCEnro.count = 0;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                //EXTREG_PBC_ENRO_MAC   PBCEnro;
                if(PlatformEqualMemory(pPort->StaCfg.WscControl.ExtRegEnrMAC, Zero, 6))
                {
                    PBCEnro.count = 0;
        }
        else
        {
                    PBCEnro.count = 1;
                    PlatformAllocateMemory(pAd,  &PBCEnro.MACs, 6*PBCEnro.count);
                    if(PBCEnro.MACs == NULL)
                {
                        ndisStatus = NDIS_STATUS_RESOURCES;
                        DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_EXTREG_PBC_ENRO_MAC, NDIS_STATUS_RESOURCES\n"));
                        break;
                }
                    PlatformMoveMemory(PBCEnro.MACs, pPort->StaCfg.WscControl.ExtRegEnrMAC, 6*PBCEnro.count); 

                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_EXTREG_PBC_ENRO_MAC, count:%d, MAC:%x:%x:%x:%x:%x:%x\n",
                    PBCEnro.count, *(PBCEnro.MACs), *(PBCEnro.MACs+1), *(PBCEnro.MACs+2), *(PBCEnro.MACs+3), *(PBCEnro.MACs+4), *(PBCEnro.MACs+5)));
                }
                
                pInfo = &PBCEnro;
                uInfoLen = sizeof(EXTREG_PBC_ENRO_MAC);
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    if(PBCEnro.MACs != NULL)
    {
        PlatformFreeMemory(PBCEnro.MACs, (PBCEnro.count * 6));
    }
        
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryHwArchitectureVersion(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(ULONG))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(ULONG);
                
                DBGPRINT(RT_DEBUG_ERROR, ("%s (MTK_OID_N6_QUERY_HW_ARCHITECTURE_VERSION): InformationBufferLength = %d, *BytesNeeded = %d\n", 
                    __FUNCTION__, 
                    InformationBufferLength, 
                    *BytesNeeded));
            }
            else
            {
                uInfo = OidQueryHwVersion(pAd);
                *BytesWritten = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_SUCCESS;
    
                DBGPRINT(RT_DEBUG_TRACE, ("%s (MTK_OID_N6_QUERY_HW_ARCHITECTURE_VERSION): HW architecture version = 0x%08X\n", 
                    __FUNCTION__, 
                    uInfo));
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryHwCapabilities(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    BOOLEAN OidSubFunStatus;                    // The return status of OID sub function
    PVOID   pLocalInfoBufForSubFunc = NULL;     // The local buffer to restore the information for reply.
    ULONG   LenReturnedFormSubFunc = 0;         // The length of SubFunction write to pLocalInfoBuf
    PlatformAllocateMemory(pAd,  &pLocalInfoBufForSubFunc, OID_BUF_QUERY_MAX_LENGTH);
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
    
            // Confirm Local buffer is available
            if(pLocalInfoBufForSubFunc == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query: Allocate LocalInfoBuf fail (%s)\n", __FUNCTION__));
                break;
            }
            else
            {
                PlatformZeroMemory(pLocalInfoBufForSubFunc, OID_BUF_QUERY_MAX_LENGTH);
            }

            // Start to perform action
            OidSubFunStatus = OidQueryHwChipCapability(
                                    pAd,
                                    pLocalInfoBufForSubFunc,
                                    InformationBufferLength,
                                    &LenReturnedFormSubFunc
                                    );  
            if(OidSubFunStatus)
            {
                uInfoLen = LenReturnedFormSubFunc;
                pInfo = pLocalInfoBufForSubFunc;
            }
            else
            {
                if(LenReturnedFormSubFunc)
                {
                    *BytesNeeded = LenReturnedFormSubFunc;
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                ndisStatus = NDIS_STATUS_FAILURE;                
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Query:MTK_OID_N6_QUERY_HW_CAPABILITIES, status=0x%08x\n", ndisStatus));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    if (pLocalInfoBufForSubFunc != NULL)
    {
        PlatformFreeMemory(pLocalInfoBufForSubFunc, OID_BUF_QUERY_MAX_LENGTH);
    }
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWtblInfo(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
#if 1           
            if (InformationBufferLength < sizeof(struct wtbl_entry))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(struct wtbl_entry);
                DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_WTBL_INFO. Size isn't correct.(=%d  %d)\n", InformationBufferLength, sizeof(struct wtbl_entry)));
            }
            else
            {
                struct wtbl_entry *pWTBLEntry = (struct wtbl_entry *)(pInformationBuffer);
    
                DumpWTBLInfo(pAd, pWTBLEntry->wtbl_idx, pWTBLEntry);
                //dump_wtbl_info(pAd, )

                pInfo = (PVOID)pWTBLEntry;

                *BytesWritten = uInfoLen = sizeof(struct wtbl_entry);
                DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_QUERY_WTBL_INFO (*BytesWritten=%d)\n", *BytesWritten));

            }
#endif       
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryTestTxNullFrame(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryRadioStatus(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryMib(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(MIB_DATA))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(MIB_DATA);
                DBGPRINT(RT_DEBUG_TRACE, ("QUERY::MTK_OID_N6_QUERY_MIB. Size isn't correct.(=%d  %d)\n", InformationBufferLength, sizeof(MIB_DATA)));
            }
            else
            {
                PMIB_DATA pMibData = (PMIB_DATA)(pInformationBuffer);
    
                show_mib_proc(pAd, pMibData);

                show_mib_proc2(pAd);

                pInfo = (PVOID)pMibData;
    
                *BytesWritten = uInfoLen = sizeof(MIB_DATA);
                DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_QUERY_MIB (*BytesWritten=%d)\n", *BytesWritten));
            }
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryPowerManagement(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryRfTest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscCustomizedIeParam(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscStatus(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = (ULONG) (pPort->StaCfg.WscControl.WscStatus | pPort->StaCfg.WscControl.WscMode); 
                if(pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_PBC_TOO_MANY_AP)
                {
                    WscStop(pAd, pPort);
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_STATUS Stop Wsc since PBC overlap!\n"));
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_STATUS (=0x%08x)\n", uInfo));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscProfile(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(WSC_PROFILE))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    pInfo = &pPort->StaCfg.WscControl.WscProfile;
                    uInfoLen = sizeof(WSC_PROFILE);
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_PROFILE\n"));
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscSsid(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    NDIS_802_11_SSID TempSSID;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if ((InformationBufferLength <= 0) ||(InformationBufferLength > sizeof(NDIS_802_11_SSID)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    if((OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED)) && (pPort->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP ))
                    {
                        PlatformZeroMemory(&TempSSID, sizeof(NDIS_802_11_SSID));
                        pInfo = &TempSSID;
                        uInfoLen = sizeof(NDIS_802_11_SSID);
                        TempSSID.SsidLength = pPort->PortCfg.SsidLen;
                        PlatformMoveMemory(TempSSID.Ssid, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
                        DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_SSID = %c, %c, %c \n", pPort->PortCfg.Ssid[0], pPort->PortCfg.Ssid[1], pPort->PortCfg.Ssid[2]));
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_SSID: Media status: %d, .WscControl.WscState= %d \n", 
                                OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED), pPort->StaCfg.WscControl.WscState) );
                    } 
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscHwPbc(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = pAd->StaCfg.HwPBCState;
                // reset to disable
                if (pAd->StaCfg.HwPBCState)
                {
                    pAd->StaCfg.HwPBCState = FALSE;
                }
        
                pPort->StaCfg.WscControl.bCheckHWPBC = TRUE;
                DBGPRINT(RT_DEBUG_INFO, ("Query::MTK_OID_N6_QUERY_WSC_HW_PBC (=%d)\n", uInfo));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryPreferredWpsApPhyType(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(ULONG))
                {
                    *BytesNeeded = sizeof(ULONG);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_PREFERRED_WPS_AP_PHY_TYPE: NDIS_STATUS_INVALID_LENGTH "  
                        "(InformationBufferLength = %d; *BytesNeeded = %d)\n", InformationBufferLength, *BytesNeeded));
                }
                else
                {
                    uInfo = pPort->StaCfg.WscControl.PreferrredWPSAPPhyType;
                    uInfoLen = sizeof(ULONG);
                    ndisStatus = NDIS_STATUS_SUCCESS;
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_PREFERRED_WPS_AP_PHY_TYPE (=%d)\n", uInfo));
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWscApChannel(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(ULONG))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_AP_CHANNEL (NDIS_STATUS_INVALID_LENGTH)\n"));
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    uInfo = pPort->StaCfg.WscControl.WscAPChannel; // The WSC AP channel that the station successfully got the WSC profile from it.
                    uInfoLen = sizeof(ULONG);
                    DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC_AP_CHANNEL (channel = %d)\n", uInfo));
                }
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidQueryWsc20Testbed(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
 
    ULONG       i          = 0; 
    ULONG       uInfo      = 0;  
    ULONG       uInfoLen   = sizeof(uInfo);
    PVOID       pInfo      = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                uInfo = (ULONG)pAd->StaCfg.WSC20Testbed;
                uInfoLen = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::MTK_OID_N6_QUERY_WSC20_TESTBED = 0x%08x, InformationBufferLength:%d\n", uInfo, InformationBufferLength));
            } while (FALSE);
        }
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        if (uInfoLen <= InformationBufferLength)
        {
            if(uInfoLen)
            {
                // Copy result into InformationBuffer
                *BytesWritten = uInfoLen;
                PlatformMoveMemory(pInformationBuffer, pInfo, uInfoLen);
            }
            else 
            {
                // the result is either zero length of has been copied by the OID specific handler
                // Dont overwrite the BytesWritten and BytesNeeded as they are also appropriately set
            }
        }
        else
        {
            // The buffer length is not sufficient
            *BytesNeeded = uInfoLen;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT; 
        }

        if (Oid == MTK_OID_N6_QUERY_EVENT_TABLE && &pAd->TrackInfo.EventTab != NULL)
        {
            PlatformZeroMemory(&pAd->TrackInfo.EventTab, sizeof(RT_802_11_EVENT_TABLE));
        } 
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


