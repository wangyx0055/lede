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
    STA_oids_Query.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"
extern PCHAR DbgGetOidName(ULONG Oid);

static DOT11_DATA_RATE_MAPPING_ENTRY Std_abg_DataRateMappingTable[] = {
    {2, 0, 2},
    {4, 0, 4},
    {6, 0, 6},
    {9, 0, 9},
    {11, 0, 11},
    {12, 0, 12},
    {18, 0, 18},
    {22, 0, 22},
    {24, 0, 24},
    {36, 0, 36},
    {44, 0, 44},
    {48, 0, 48},
    {54, 0, 54},
    {66, 0, 66},
    {72, 0, 72},
    {96, 0, 96},
    {108, 0, 108}
};

#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER)) 

NDIS_STATUS
N6QueryOidDot11PowerMgmtModeAutoEnabled(
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
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Query oid  0x%08x %s .....\n", Oid, DbgGetOidName(Oid)));

    if (InformationBufferLength < DOT11_SIZEOF_POWER_MGMT_AUTO_MODE_ENABLE_INFO_REVISION_1)
    {
        *BytesNeeded = DOT11_SIZEOF_POWER_MGMT_AUTO_MODE_ENABLE_INFO_REVISION_1;
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
        PDOT11_POWER_MGMT_AUTO_MODE_ENABLED_INFO pPowerMgmtAutoMode = (PDOT11_POWER_MGMT_AUTO_MODE_ENABLED_INFO)InformationBuffer;

        MP_ASSIGN_NDIS_OBJECT_HEADER(pPowerMgmtAutoMode->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_POWER_MGMT_AUTO_MODE_ENABLED_REVISION_1,
                                    DOT11_SIZEOF_POWER_MGMT_AUTO_MODE_ENABLE_INFO_REVISION_1);
        pPowerMgmtAutoMode->bEnabled = pAd->StaCfg.bAutoPowerSaveEnable;
            }
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
N6QueryOidDot11PowerMgmtModeStatus(
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
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Query oid OID_DOT11_POWER_MGMT_MODE_STATUS 0x%08x %s .....\n", Oid, DbgGetOidName(Oid)));

    if (InformationBufferLength < DOT11_SIZEOF_POWER_MGMT_MODE_STATUS_INFO_REVISION_1)
    {
        *BytesNeeded = DOT11_SIZEOF_POWER_MGMT_MODE_STATUS_INFO_REVISION_1;
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
        PDOT11_POWER_MGMT_MODE_STATUSINFO pPowerMgmtModeStatus = (PDOT11_POWER_MGMT_MODE_STATUSINFO)InformationBuffer;

        MP_ASSIGN_NDIS_OBJECT_HEADER(pPowerMgmtModeStatus->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_POWER_MGMT_MODE_STATUS_INFO_REVISION_1,
                                    DOT11_SIZEOF_POWER_MGMT_MODE_STATUS_INFO_REVISION_1);
/*
    typedef enum _DOT11_POWER_MODE { 
        dot11_power_mode_unknown = 0, 
        dot11_power_mode_active = 1, 
        dot11_power_mode_powersave = 2 
    } DOT11_POWER_MODE, * PDOT11_POWER_MODE; 
*/
        pPowerMgmtModeStatus->PowerSaveMode = dot11_power_mode_powersave; //pAd->StaCfg.Psm;
/*
    #define DOT11_POWER_SAVING_NO_POWER_SAVING  0 
    #define DOT11_POWER_SAVING_FAST_PSP         8 
    #define DOT11_POWER_SAVING_MAX_PSP          16 
    #define DOT11_POWER_SAVING_MAXIMUM_LEVEL    24 

*/
        pPowerMgmtModeStatus->uPowerSaveLevel = pAd->StaCfg.WindowsPowerMode;
/*
typedef enum _ DOT11_POWER_MODE_REASON { 
 dot11_power_mode_reason_no_change         =0, // initially in this state, has not changed since 
 dot11_power_mode_reason_noncompliant_AP   =1, // AP is not compliant. As to be in CAM 
 dot11_power_mode_reason_legacy_WFD_device =2, // WFD device is legacy 
 dot11_power_mode_reason_compliant_AP      =3,// AP connected does PSM correctly, PSM can works 
 dot11_power_mode_reason_compliant_WFD_device =4, // All connected WFD device can do PSM 
 dot11_power_mode_reason_others            =5 
} DOT11_POWER_MODE_REASON;
*/
        pPowerMgmtModeStatus->Reason = dot11_power_mode_reason_others;
            }
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
N6QueryOidDot11OffloadNetworkList(
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
            DBGPRINT(RT_DEBUG_ERROR, ("Query oid  0x%08x %s .....\n", Oid, DbgGetOidName(Oid)));
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

#endif // COMPILE_WIN8_ABOVE


NDIS_STATUS
N6QueryOidPmRemoveWolPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidGenInterruptModeration(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPmRemoveProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPmAddProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPmAddWolPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPmGetProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidDot11NicPowerState(
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
        uInfo = pAd->StaCfg.bRadio;
        uInfoLen = sizeof(BOOLEAN);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_NIC_POWER_STATE (=%d)\n", uInfo));
    }
    else // EXTAP_PORT
    {
        do
        {
            *BytesWritten= 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(BOOLEAN))
            {
                *BytesNeeded = sizeof(BOOLEAN); 
                ndisStatus = NDIS_STATUS_INVALID_LENGTH; 
                break; 
            }
            *(PBOOLEAN)InformationBuffer = pAd->StaCfg.bRadio;
            *BytesWritten = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) QUERY::OID_DOT11_NIC_POWER_STATE (=%d)\n", PortNumber, *(PBOOLEAN)InformationBuffer));
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
N6QueryOidDot11DesiredPhyList(
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
        PDOT11_PHY_ID_LIST PhyIdList = (PDOT11_PHY_ID_LIST)InformationBuffer;
        ULONG TotalLen = FIELD_OFFSET(DOT11_PHY_ID_LIST, dot11PhyId) + pAd->StaCfg.DesiredPhyCount * sizeof(ULONG);
        
        if (InformationBufferLength < TotalLen)
        {
            *BytesNeeded = TotalLen;
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;

            FUNC_LEAVE_STATUS(ndisStatus);
            return ndisStatus;
        }
        PlatformZeroMemory(PhyIdList, InformationBufferLength);

        MP_ASSIGN_NDIS_OBJECT_HEADER(PhyIdList->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_PHY_ID_LIST_REVISION_1,
                                    sizeof(DOT11_PHY_ID_LIST));

        //
        // Copy the desired PHY list.
        //
        PhyIdList->uNumOfEntries = pAd->StaCfg.DesiredPhyCount;
        PlatformMoveMemory(PhyIdList->dot11PhyId, pAd->StaCfg.DesiredPhyList, pAd->StaCfg.DesiredPhyCount * sizeof(ULONG));
                
        //
        // Exit and don't copy result into InformationBuffer
        //
        *BytesWritten = TotalLen;
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DESIRED_PHY_LIST (PhyCount=%d)\n", PhyIdList->uNumOfEntries));
    }
    else // EXTAP_PORT
    {
        // break;
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
N6QueryOidDot11AutoConfigEnabled(
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
        uInfo = pAd->pNicCfg->AutoConfigEnabled;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_AUTO_CONFIG_ENABLED (=%d)\n", uInfo));
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryAutoConfigEnabled()
        do
        {
            *BytesWritten= 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG); 
                ndisStatus = NDIS_STATUS_INVALID_LENGTH; 
                break; 
            }
            *(PULONG)InformationBuffer = pPort->AutoConfigEnabled;
            *BytesWritten = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) QUERY::OID_DOT11_AUTO_CONFIG_ENABLED (=%d)\n", PortNumber, *(PULONG)InformationBuffer));
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
N6QueryOidDot11CurrentOperationMode(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_CURRENT_OPERATION_MODE))
            {
                *BytesNeeded = sizeof(DOT11_CURRENT_OPERATION_MODE);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                PDOT11_CURRENT_OPERATION_MODE pCurrentOperationMode = (PDOT11_CURRENT_OPERATION_MODE)InformationBuffer;
                
                pCurrentOperationMode->uCurrentOpMode = pPort->PortCfg.OperationMode;
                //
                // Exit and don't copy result into InformationBuffer
                //              
                *BytesWritten = sizeof(DOT11_CURRENT_OPERATION_MODE);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_OPERATION_MODE (=%d)\n", pCurrentOperationMode->uCurrentOpMode));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryCurrentOperationMode()
        do
        {
            *BytesWritten= 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(DOT11_CURRENT_OPERATION_MODE))
            {
                *BytesNeeded = sizeof(DOT11_CURRENT_OPERATION_MODE);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                PDOT11_CURRENT_OPERATION_MODE pCurrentOperationMode = (PDOT11_CURRENT_OPERATION_MODE)InformationBuffer;
                
                pCurrentOperationMode->uCurrentOpMode = pPort->PortCfg.OperationMode;
                //
                // Exit and don't copy result into InformationBuffer
                //              
                *BytesWritten = sizeof(DOT11_CURRENT_OPERATION_MODE);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_OPERATION_MODE (=%d)\n", pCurrentOperationMode->uCurrentOpMode));
            }
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
N6QueryOidDot11FlushBssList(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
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
N6QueryOidDot11DesiredSsidList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_SSID_LIST pDot11SSIDList = (PDOT11_SSID_LIST)InformationBuffer;

            if (InformationBufferLength < sizeof(DOT11_SSID_LIST))
            {
                *BytesNeeded = sizeof(DOT11_SSID_LIST);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            
            TotalLen = FIELD_OFFSET(DOT11_SSID_LIST, SSIDs) + sizeof(DOT11_SSID);
            if (InformationBufferLength < TotalLen)
            {
                pDot11SSIDList->uNumOfEntries = 0;
                pDot11SSIDList->uTotalNumOfEntries = 1;         
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            PlatformZeroMemory(pDot11SSIDList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11SSIDList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_SSID_LIST_REVISION_1,
                                        sizeof(DOT11_SSID_LIST));

            //
            // Only for one entry.
            //
            pDot11SSIDList->uNumOfEntries = 1;
            pDot11SSIDList->uTotalNumOfEntries = 1;

            PlatformMoveMemory(pDot11SSIDList->SSIDs, &pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = pDot11SSIDList->uNumOfEntries * sizeof(DOT11_SSID) + FIELD_OFFSET(DOT11_SSID_LIST, SSIDs);

            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DESIRED_SSID_LIST (SSID=%c%c%c....,Len=%d)\n", 
                pDot11SSIDList->SSIDs[0].ucSSID[0], pDot11SSIDList->SSIDs[0].ucSSID[1], pDot11SSIDList->SSIDs[0].ucSSID[2], pDot11SSIDList->SSIDs[0].uSSIDLength));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryDesiredSSIDList()
        ULONG                   TotalLen;
        PDOT11_SSID_LIST        pDot11SSIDList = NULL;
        
        do
        {
            *BytesWritten = 0;
            *BytesNeeded = 0;

            pDot11SSIDList = (PDOT11_SSID_LIST)InformationBuffer;

            if (InformationBufferLength < sizeof(DOT11_SSID_LIST))
            {
                *BytesNeeded = sizeof(DOT11_SSID_LIST);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            
            TotalLen = FIELD_OFFSET(DOT11_SSID_LIST, SSIDs) + sizeof(DOT11_SSID);
            if (InformationBufferLength < TotalLen)
            {
                pDot11SSIDList->uNumOfEntries = 0;
                pDot11SSIDList->uTotalNumOfEntries = 1;         
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            PlatformZeroMemory(pDot11SSIDList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11SSIDList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_SSID_LIST_REVISION_1,
                                        sizeof(DOT11_SSID_LIST));
            if(pPort->PortCfg.DesiredSSID.uSSIDLength == 0)
            {
                //no any entry ,clear ssid list
                pDot11SSIDList->uNumOfEntries = 0;
                pDot11SSIDList->uTotalNumOfEntries = 0;
                RtlZeroMemory(
                            &pDot11SSIDList->SSIDs[0],
                            sizeof(DOT11_SSID)
                            );
            }
            else
            {
                //
                // Only for one entry.
                //
                pDot11SSIDList->uNumOfEntries = 1;
                pDot11SSIDList->uTotalNumOfEntries = 1;
                PlatformMoveMemory(pDot11SSIDList->SSIDs, &pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
            }
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = pDot11SSIDList->uNumOfEntries * sizeof(DOT11_SSID) + FIELD_OFFSET(DOT11_SSID_LIST, SSIDs);

            DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) QUERY::OID_DOT11_DESIRED_SSID_LIST (SSID=%c%c%c....,Len=%d)\n", PortNumber, 
                pDot11SSIDList->SSIDs[0].ucSSID[0], pDot11SSIDList->SSIDs[0].ucSSID[1], pDot11SSIDList->SSIDs[0].ucSSID[2], pDot11SSIDList->SSIDs[0].uSSIDLength));
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
N6QueryOidDot11CurrentChannel(
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
        uInfo =pPort->Channel;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_CHANNEL (=%d)\n", uInfo));
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryCurrentChannel()
        do
        {
            *BytesWritten= 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG); 
                ndisStatus = NDIS_STATUS_INVALID_LENGTH; 
                break; 
            }
            *(PULONG)InformationBuffer = pPort->Channel;
            *BytesWritten = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) Query::OID_DOT11_CURRENT_CHANNEL (=%d)\n", PortNumber, *(PULONG)InformationBuffer));
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
N6QueryOidDot11SafeModeEnabled(
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
        uInfo = pPort->CommonCfg.bSafeModeEnabled;
        uInfoLen = sizeof(BOOLEAN);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SAFE_MODE_ENABLED (=%d)\n", uInfo));
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
N6QueryOidDot11CurrentPhyId(
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
        uInfo = pPort->CommonCfg.SelectedPhyId;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_INFO, ("Query::OID_DOT11_CURRENT_PHY_ID (=%d)\n", uInfo));
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
N6QueryOidPnpAddWakeUpPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPnpRemoveWakeUpPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPnpEnableWakeUp(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPnpCapabilities(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidPnpQueryPower(
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
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_PNP_QUERY_POWER \n"));
        // Status is always success for OID_PNP_SET_POWER
        ndisStatus = NDIS_STATUS_SUCCESS;
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
N6QueryOidDot11MpduMaxLength(
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
        uInfo = pPort->CommonCfg.MPDUMaxLength;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_MPDU_MAX_LENGTH (=%d)\n", uInfo));
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
N6QueryOidDot11OperationModeCapability(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_OPERATION_MODE_CAPABILITY))
            {
                *BytesNeeded = sizeof(DOT11_OPERATION_MODE_CAPABILITY);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                pInfo = (PVOID)&pPort->CommonCfg.OperationModeCapability;
                uInfoLen = sizeof(DOT11_OPERATION_MODE_CAPABILITY);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_OPERATION_MODE_CAPABILITY \n"));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryOperationModeCapability()
        do
        {
            *BytesWritten= 0;
            *BytesNeeded = 0;
            if (InformationBufferLength < sizeof(DOT11_OPERATION_MODE_CAPABILITY))
            {
                *BytesNeeded = sizeof(DOT11_OPERATION_MODE_CAPABILITY);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                PlatformMoveMemory(InformationBuffer,&pPort->CommonCfg.OperationModeCapability
                    ,sizeof(DOT11_OPERATION_MODE_CAPABILITY));
                *BytesWritten= sizeof(DOT11_OPERATION_MODE_CAPABILITY);
                DBGPRINT(RT_DEBUG_TRACE, ("AP Query::OID_DOT11_OPERATION_MODE_CAPABILITY \n"));
            }
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
N6QueryOidDot11AtimWindow(
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
        uInfo = pPort->CommonCfg.AtimWindow;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ATIM_WINDOW (=%d)\n", uInfo));
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
N6QueryOidDot11OptionalCapability(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_OPTIONAL_CAPABILITY))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(DOT11_OPTIONAL_CAPABILITY);
                break;
            }
            else
            {
                pInfo = (PVOID)&pPort->CommonCfg.OptionalCapability;
                uInfoLen = sizeof(DOT11_OPTIONAL_CAPABILITY);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_OPTIONAL_CAPABILITY\n"));
            }
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
N6QueryOidDot11CurrentOptionalCapability(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_CURRENT_OPTIONAL_CAPABILITY))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(DOT11_CURRENT_OPTIONAL_CAPABILITY);
                break;
            }
            else
            {
                pInfo = (PVOID)&pPort->CommonCfg.CurrentOptionalCapability;
                uInfoLen = sizeof(DOT11_CURRENT_OPTIONAL_CAPABILITY);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_OPTIONAL_CAPABILITY\n"));
            }
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
N6QueryOidDot11StationId(
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
        pInfo = pAd->HwCfg.PermanentAddress;
        uInfoLen = MAC_ADDR_LEN;
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_3_PERMANENT_ADDRESS (=%02x:%02x:%02x:%02x:%02x:%02x)\n",
                    pAd->HwCfg.PermanentAddress[0], pAd->HwCfg.PermanentAddress[1], pAd->HwCfg.PermanentAddress[2],
                    pAd->HwCfg.PermanentAddress[3], pAd->HwCfg.PermanentAddress[4], pAd->HwCfg.PermanentAddress[5]));
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
N6QueryOidDot11ChannelAgilityEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidDot11ChannelAgilityPresent(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidDot11OperationalRateSet(
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
        do
        {   
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_RATE_SET))
            {
                *BytesNeeded = sizeof(DOT11_RATE_SET);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                PDOT11_RATE_SET pDot11RateSet = (PDOT11_RATE_SET)InformationBuffer;
                UCHAR Index;
                ULONG PhyID = pPort->CommonCfg.SelectedPhyId;
                
                PlatformZeroMemory(pDot11RateSet, InformationBufferLength);

                for (Index = 0; Index < pAd->pNicCfg->PhyMIB[PhyID].OperationalRateSet.uRateSetLength; Index++) 
                {               
                    pDot11RateSet->ucRateSet[Index] = pAd->pNicCfg->PhyMIB[PhyID].OperationalRateSet.ucRateSet[Index];
                }

                pDot11RateSet->uRateSetLength = Index;

                //
                // Exit and don't copy result into InformationBuffer
                //              
                *BytesWritten = sizeof(DOT11_RATE_SET);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_OPERATIONAL_RATE_SET (RateLen=%d)\n", pDot11RateSet->uRateSetLength));
            }
        } while (FALSE);
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
N6QueryOidDot11MacAddress(
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
        pInfo = pPort->CurrentAddress;
        uInfoLen = MAC_ADDR_LEN;
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_3_CURRENT_ADDRESS (=%02x:%02x:%02x:%02x:%02x:%02x)\n",
                    pPort->CurrentAddress[0], pPort->CurrentAddress[1], pPort->CurrentAddress[2],
                    pPort->CurrentAddress[3], pPort->CurrentAddress[4], pPort->CurrentAddress[5]));
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
N6QueryOidDot11RtsThreshold(
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
        uInfo = pPort->CommonCfg.RtsThreshold;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_RTS_THRESHOLD (=%d)\n", uInfo));
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
N6QueryOidDot11ShortRetryLimit(
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
        uInfo = 7;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SHORT_RETRY_LIMIT (=%d)\n", uInfo));
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
N6QueryOidDot11LongRetryLimit(
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
        uInfo = 4;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_LONG_RETRY_LIMIT (=%d)\n", uInfo));
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
N6QueryOidDot11FragmentationThreshold(
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
        uInfo = pPort->CommonCfg.FragmentThreshold;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_FRAGMENTATION_THRESHOLD (=%d)\n", uInfo));
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
N6QueryOidDot11MaxTransmitMsduLifetime(
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
        uInfo = pPort->CommonCfg.MaxTxLifeTime;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::>>OID_DOT11_MAX_TRANSMIT_MSDU_LIFETIME\n"));
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
N6QueryOidDot11MaxReceiveLifetime(
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
        uInfo = pPort->CommonCfg.MaxRxLifeTime;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::>>OID_DOT11_MAX_RECEIVE_LIFETIME\n"));
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
N6QueryOidDot11SupportedPhyTypes(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < FIELD_OFFSET(DOT11_SUPPORTED_PHY_TYPES, dot11PHYType))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = FIELD_OFFSET(DOT11_SUPPORTED_PHY_TYPES, dot11PHYType);
                break;
            }
            else
            {
                //ASSERT(FALSE); //Break here
                PDOT11_SUPPORTED_PHY_TYPES pDot11SupportedPhyTypes = InformationBuffer;
                ULONG uNumMaxEntries = 0;

                do
                {
                    *BytesWritten = 0;
                    *BytesNeeded = 0;

                    if (InformationBufferLength < FIELD_OFFSET(DOT11_SUPPORTED_PHY_TYPES, dot11PHYType))
                    {
                        ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                        *BytesNeeded = FIELD_OFFSET(DOT11_SUPPORTED_PHY_TYPES, dot11PHYType);
                        break;
                    }

                    InformationBufferLength -= FIELD_OFFSET(DOT11_SUPPORTED_PHY_TYPES, dot11PHYType);
                    uNumMaxEntries = InformationBufferLength / sizeof(DOT11_PHY_TYPE);

                    ndisStatus = Ndis6CommonQueryCurrentSupportedPHYTypes(pAd, pPort, uNumMaxEntries, pDot11SupportedPhyTypes);

                    *BytesWritten = FIELD_OFFSET(DOT11_SUPPORTED_PHY_TYPES, dot11PHYType) +
                                    pDot11SupportedPhyTypes->uNumOfEntries * sizeof(DOT11_PHY_TYPE);

                    *BytesNeeded = FIELD_OFFSET(DOT11_SUPPORTED_PHY_TYPES, dot11PHYType) +
                                    pDot11SupportedPhyTypes->uTotalNumOfEntries * sizeof(DOT11_PHY_TYPE);
                } while(FALSE);
                
                DBGPRINT(RT_DEBUG_TRACE, ("Query::>>OID_DOT11_SUPPORTED_PHY_TYPES\n"));
            }
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
N6QueryOidDot11CurrentRegDomain(
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
        uInfo = DOT11_REG_DOMAIN_OTHER;
        uInfoLen = sizeof(ULONG);
        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_REG_DOMAIN (=%d)\n", uInfo));
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
N6QueryOidDot11TempType(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_TEMP_TYPE))
            {
                *BytesNeeded = sizeof(DOT11_TEMP_TYPE);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                uInfo = dot11_temp_type_1;  //we return dot11_temp_type_1 (0C - 40C).
                uInfoLen = sizeof(DOT11_TEMP_TYPE);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_TEMP_TYPE (=%d)\n", uInfo));
            }
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
N6QueryOidDot11DiversitySupport(
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
        do
        {   
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_DIVERSITY_SUPPORT))
            {
                *BytesNeeded = sizeof(DOT11_DIVERSITY_SUPPORT);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                uInfo = dot11_diversity_support_dynamic;
                uInfoLen = sizeof(DOT11_DIVERSITY_SUPPORT);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DIVERSITY_SUPPORT (=%d)\n", uInfo));
            }
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
N6QueryOidDot11SupportedPowerLevels(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_SUPPORTED_POWER_LEVELS))
            {
                *BytesNeeded = sizeof(DOT11_SUPPORTED_POWER_LEVELS);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SUPPORTED_POWER_LEVELS not Support\n"));
            }
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
N6QueryOidDot11CurrentTxPowerLevel(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = 1;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_TX_POWER_LEVEL (=%d)\n", uInfo));
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
N6QueryOidDot11CcaModeSupported(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = DOT11_CCA_MODE_CS_ONLY;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CCA_MODE_SUPPORTED (=%d)\n", uInfo));
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
N6QueryOidDot11CurrentCcaMode(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = DOT11_CCA_MODE_CS_ONLY;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_CCA_MODE (=%d)\n", uInfo));
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
N6QueryOidDot11EdThreshold(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = (ULONG) -65;
            uInfo = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ED_THRESHOLD (=%d)\n", uInfo));
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
N6QueryOidDot11RegDomainsSupportValue(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < FIELD_OFFSET(DOT11_REG_DOMAINS_SUPPORT_VALUE, dot11RegDomainValue))
            {
                *BytesNeeded = sizeof(DOT11_REG_DOMAINS_SUPPORT_VALUE);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                PDOT11_REG_DOMAINS_SUPPORT_VALUE pDot11RegDomainsSupportValue = (PDOT11_REG_DOMAINS_SUPPORT_VALUE)InformationBuffer;
                PlatformZeroMemory(InformationBuffer, InformationBufferLength);

                pDot11RegDomainsSupportValue->uNumOfEntries = 0;
                pDot11RegDomainsSupportValue->uTotalNumOfEntries = 0;

                *BytesWritten = pDot11RegDomainsSupportValue->uNumOfEntries * sizeof(DOT11_REG_DOMAIN_VALUE) +
                        FIELD_OFFSET(DOT11_REG_DOMAINS_SUPPORT_VALUE, dot11RegDomainValue);
                *BytesNeeded = pDot11RegDomainsSupportValue->uTotalNumOfEntries * sizeof(DOT11_REG_DOMAIN_VALUE) +
                        FIELD_OFFSET(DOT11_REG_DOMAINS_SUPPORT_VALUE, dot11RegDomainValue);                             
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_REG_DOMAINS_SUPPORT_VALUE (num of dot11RegDomainValue = %d)\n",
                            pDot11RegDomainsSupportValue->uNumOfEntries));
            }
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
N6QueryOidDot11SupportedTxAntenna(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < FIELD_OFFSET(DOT11_SUPPORTED_ANTENNA_LIST, dot11SupportedAntenna))
            {
                *BytesNeeded = sizeof(DOT11_SUPPORTED_ANTENNA_LIST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SUPPORTED_TX_ANTENNA not Support\n"));
            }
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
N6QueryOidDot11SupportedRxAntenna(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < FIELD_OFFSET(DOT11_SUPPORTED_ANTENNA_LIST, dot11SupportedAntenna))
            {
                *BytesNeeded = sizeof(DOT11_SUPPORTED_ANTENNA_LIST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SUPPORTED_RX_ANTENNA not Support\n"));
            }
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
N6QueryOidDot11DiversitySelectionRx(
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
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < FIELD_OFFSET(DOT11_DIVERSITY_SELECTION_RX_LIST, dot11DiversitySelectionRx))
            {
                *BytesNeeded = sizeof(DOT11_SUPPORTED_ANTENNA_LIST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DIVERSITY_SELECTION_RX not Support\n"));
            }
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
N6QueryOidDot11SupportedDataRatesValue(
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
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_SUPPORTED_DATA_RATES_VALUE_V2 pSupportedDataRatesValue = (PDOT11_SUPPORTED_DATA_RATES_VALUE_V2) InformationBuffer;
            ULONG Index;
            
            if (InformationBufferLength < sizeof(DOT11_SUPPORTED_DATA_RATES_VALUE_V2))
            {
                *BytesNeeded = sizeof(DOT11_SUPPORTED_DATA_RATES_VALUE_V2);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pSupportedDataRatesValue, InformationBufferLength);

            for (Index = 0; Index < MAX_NUM_SUPPORTED_RATES_V2; Index++)
            {
                pSupportedDataRatesValue->ucSupportedTxDataRatesValue[Index] = pAd->pNicCfg->PhyMIB[pPort->CommonCfg.SelectedPhyId].SupportedDataRatesValue.ucSupportedTxDataRatesValue[Index];
                pSupportedDataRatesValue->ucSupportedRxDataRatesValue[Index] = pAd->pNicCfg->PhyMIB[pPort->CommonCfg.SelectedPhyId].SupportedDataRatesValue.ucSupportedRxDataRatesValue[Index];
            }
            
            //
            // Exit and don't copy result into InformationBuffer
            //              
            *BytesWritten = sizeof(DOT11_SUPPORTED_DATA_RATES_VALUE_V2);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SUPPORTED_DATA_RATES_VALUE (SupRateLen=%d)\n", pPort->CommonCfg.SupRateLen));
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
N6QueryOidDot11RfUsage(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = 10;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_RF_USAGE (=%d)\n", uInfo));
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
N6QueryOidDot11RecvSensitivityList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidDot11MultiDomainCapabilityImplemented(
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
        ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
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
N6QueryOidDot11MultiDomainCapabilityEnabled(
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
        ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
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
N6QueryOidDot11CountryString(
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
        ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
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
N6QueryOidDot11CurrentFrequency(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->Channel;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CURRENT_FREQUENCY (=%d)\n", uInfo));
        } while (FALSE);
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
N6QueryOidDot11FrequencyBandsSupported(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = DOT11_FREQUENCY_BANDS_LOWER | DOT11_FREQUENCY_BANDS_MIDDLE;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_FREQUENCY_BANDS_SUPPORTED (=0x%08x)\n", uInfo));
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
N6QueryOidDot11ShortPreambleOptionImplemented(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = TRUE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SHORT_PREAMBLE_OPTION_IMPLEMENTED (=%d)\n", uInfo));
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
N6QueryOidDot11PbccOptionImplemented(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = FALSE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_PBCC_OPTION_IMPLEMENTED (=%d)\n", uInfo));
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
N6QueryOidDot11ErpPbccOptionImplemented(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = FALSE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ERP_PBCC_OPTION_IMPLEMENTED (=%d)\n", uInfo));
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
N6QueryOidDot11ErpPbccOptionEnabled(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = FALSE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ERP_PBCC_OPTION_ENABLED (=%d)\n", uInfo));
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
N6QueryOidDot11DsssOfdmOptionImplemented(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = FALSE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DSSS_OFDM_OPTION_IMPLEMENTED (=%d)\n", uInfo));
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
N6QueryOidDot11DsssOfdmOptionEnabled(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = FALSE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DSSS_OFDM_OPTION_ENABLED (=%d)\n", uInfo));
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
N6QueryOidDot11ShortSlotTimeOptionImplemented(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = TRUE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SHORT_SLOT_TIME_OPTION_IMPLEMENTED (=%d)\n", uInfo));
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
N6QueryOidDot11ShortSlotTimeOptionEnabled(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = TRUE;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SHORT_SLOT_TIME_OPTION_ENABLED (=%d)\n", uInfo));
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
N6QueryOidDot11PowerMgmtRequest(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = DOT11_POWER_SAVING_NO_POWER_SAVING;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_POWER_MGMT_REQUEST (=%d)\n", uInfo));
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
N6QueryOidDot11ExcludedMacAddressList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_MAC_ADDRESS_LIST pDot11MacAddrList = (PDOT11_MAC_ADDRESS_LIST)InformationBuffer;

            TotalLen = pAd->StaCfg.ExcludedMACAddressCount * MAC_ADDR_LEN + FIELD_OFFSET(DOT11_MAC_ADDRESS_LIST, MacAddrs);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            PlatformZeroMemory(pDot11MacAddrList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11MacAddrList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_MAC_ADDRESS_LIST_REVISION_1,
                                        sizeof(DOT11_MAC_ADDRESS_LIST));

            PlatformMoveMemory(pDot11MacAddrList->MacAddrs, pAd->StaCfg.ExcludedMACAddressList, pAd->StaCfg.ExcludedMACAddressCount * MAC_ADDR_LEN);
            pDot11MacAddrList->uNumOfEntries = pAd->StaCfg.ExcludedMACAddressCount;
            pDot11MacAddrList->uTotalNumOfEntries = pAd->StaCfg.ExcludedMACAddressCount;

            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_EXCLUDED_MAC_ADDRESS_LIST, MacCount=%d\n", pAd->StaCfg.ExcludedMACAddressCount));
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
N6QueryOidDot11ExcludeUnencrypted(
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
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->PortCfg.ExcludeUnencrypted;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_EXCLUDE_UNENCRYPTED (=%d)\n", uInfo));
        } while (FALSE);
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
N6QueryOidDot11DesiredBssidList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_BSSID_LIST pDot11BSSIDList = (PDOT11_BSSID_LIST)InformationBuffer;

            TotalLen = pPort->PortCfg.DesiredBSSIDCount * MAC_ADDR_LEN + FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            
            PlatformZeroMemory(pDot11BSSIDList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11BSSIDList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_BSSID_LIST_REVISION_1,
                                        sizeof(DOT11_BSSID_LIST));

            PlatformMoveMemory(pDot11BSSIDList->BSSIDs, pPort->PortCfg.DesiredBSSIDList, pPort->PortCfg.DesiredBSSIDCount * MAC_ADDR_LEN);

            pDot11BSSIDList->uNumOfEntries = pPort->PortCfg.DesiredBSSIDCount;
            pDot11BSSIDList->uTotalNumOfEntries = pPort->PortCfg.DesiredBSSIDCount;

            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DESIRED_BSSID_LIST, Bssid list Count=%d\n", pPort->PortCfg.DesiredBSSIDCount));
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
N6QueryOidDot11DesiredBssType(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_BSS_TYPE))
            {
                *BytesNeeded = sizeof(DOT11_BSS_TYPE);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                uInfo = pAd->StaCfg.BssType;
                if (IS_P2P_STA_GO(pAd, pPort) && (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort))) 
                {
                    uInfo = BSS_ADHOC;
                }
                uInfoLen = sizeof(DOT11_BSS_TYPE);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DESIRED_BSS_TYPE (=%d)\n", uInfo));
            }
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
N6QueryOidDot11Statistics(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_STATISTICS))
            {
                *BytesNeeded = sizeof(DOT11_STATISTICS);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_STATISTICS\n"));
                ndisStatus = HwQueryStatistics(    pAd,
                              InformationBuffer,
                              InformationBufferLength,
                              BytesWritten,
                              BytesNeeded);
            }   
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryStatistics()
        do
        {
            *BytesWritten= 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(DOT11_STATISTICS))
            {
                *BytesNeeded = sizeof(DOT11_STATISTICS);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) QUERY::OID_DOT11_STATISTICS\n", PortNumber));
                ndisStatus = HwQueryStatistics(pAd,     //may need to classified to different port yfczz_add
                                            InformationBuffer,
                                            InformationBufferLength,
                                            BytesWritten,
                                            BytesNeeded);
            }   
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
N6QueryOidDot11EnabledAuthenticationAlgorithm(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_AUTH_ALGORITHM_LIST pAuthAlgoList = (PDOT11_AUTH_ALGORITHM_LIST)InformationBuffer;

            TotalLen = FIELD_OFFSET(DOT11_AUTH_ALGORITHM_LIST, AlgorithmIds) + sizeof(DOT11_AUTH_ALGORITHM);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pAuthAlgoList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pAuthAlgoList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_AUTH_ALGORITHM_LIST_REVISION_1,
                                        sizeof(DOT11_AUTH_ALGORITHM_LIST));

            pAuthAlgoList->uNumOfEntries = 1;
            pAuthAlgoList->uTotalNumOfEntries = 1;

            if ((pAd->OpMode == OPMODE_STAP2P) && (PORTV2_P2P_ON(pAd, pPort)))
            {
                pAuthAlgoList->AlgorithmIds[0] = VPORT_DOT11_AUTH_ALGO_RSNA_PSK;
            }
            else if (IS_P2P_STA_GO(pAd, pPort) && (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort)))
            {
                pAuthAlgoList->AlgorithmIds[0] = DOT11_AUTH_ALGO_80211_OPEN;
            }
            else if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
            {
                pAuthAlgoList->AlgorithmIds[0] = pPort->SoftAP.ApCfg.ApPseudoStaAuthMode;
            }
            else
            {
                pAuthAlgoList->AlgorithmIds[0] = pPort->PortCfg.AuthMode;
            }
    
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ENABLED_AUTHENTICATION_ALGORITHM (AlgorithmIds = %x)\n", pPort->PortCfg.AuthMode));
        } while (FALSE);
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
N6QueryOidDot11SupportedUnicastAlgorithmPair(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG MaxAuthCipherPair = 0;
            PDOT11_AUTH_CIPHER_PAIR_LIST pAuthCipherList = (PDOT11_AUTH_CIPHER_PAIR_LIST)InformationBuffer;
                
            if (pAd->StaCfg.BssType == dot11_BSS_type_infrastructure)
            {
                MaxAuthCipherPair = uBSSUnicastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);
            }
            else
            {
                MaxAuthCipherPair = uIBSSUnicastAuthCipherPairsSize/ sizeof(DOT11_AUTH_CIPHER_PAIR); //Assume Adhoc
            }

            TotalLen = FIELD_OFFSET(DOT11_AUTH_CIPHER_PAIR_LIST, AuthCipherPairs) + MaxAuthCipherPair * sizeof(DOT11_AUTH_CIPHER_PAIR);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pAuthCipherList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pAuthCipherList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_AUTH_CIPHER_PAIR_LIST_REVISION_1,
                                        sizeof(DOT11_AUTH_CIPHER_PAIR_LIST));
            
            pAuthCipherList->uNumOfEntries = MaxAuthCipherPair;
            if (pAd->StaCfg.BssType == dot11_BSS_type_infrastructure)
            {
                PlatformMoveMemory(pAuthCipherList->AuthCipherPairs, BSSUnicastAuthCipherPairs, uBSSUnicastAuthCipherPairsSize);
            }
            else
            {
                PlatformMoveMemory(pAuthCipherList->AuthCipherPairs, IBSSUnicastAuthCipherPairs, uIBSSUnicastAuthCipherPairsSize);
            }
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SUPPORTED_UNICAST_ALGORITHM_PAIR (BssType=%d,NumOfAuthCipherPair=%d)\n",
                                        pAd->StaCfg.BssType, MaxAuthCipherPair));
        } while (FALSE);
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
N6QueryOidDot11SupportedMulticastAlgorithmPair(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG   MaxAuthCipherPair;
            PDOT11_AUTH_CIPHER_PAIR_LIST pAuthCipherList = (PDOT11_AUTH_CIPHER_PAIR_LIST)InformationBuffer;

            if (pAd->StaCfg.BssType == dot11_BSS_type_infrastructure)
            {
                MaxAuthCipherPair = uBSSMulticastAuthCipherPairsSize/ sizeof(DOT11_AUTH_CIPHER_PAIR);
            }
            else
            {
                MaxAuthCipherPair = uIBSSMulticastAuthCipherPairsSize/ sizeof(DOT11_AUTH_CIPHER_PAIR);  //Assume Adhoc
            }

            TotalLen = FIELD_OFFSET(DOT11_AUTH_CIPHER_PAIR_LIST, AuthCipherPairs) + MaxAuthCipherPair * sizeof(DOT11_AUTH_CIPHER_PAIR);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pAuthCipherList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pAuthCipherList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_AUTH_CIPHER_PAIR_LIST_REVISION_1,
                                        sizeof(DOT11_AUTH_CIPHER_PAIR_LIST));
            
            pAuthCipherList->uNumOfEntries = MaxAuthCipherPair;
            if (pAd->StaCfg.BssType == dot11_BSS_type_infrastructure)
            {
                PlatformMoveMemory(pAuthCipherList->AuthCipherPairs, BSSMulticastAuthCipherPairs, uBSSMulticastAuthCipherPairsSize);
            }
            else
            {
                PlatformMoveMemory(pAuthCipherList->AuthCipherPairs, IBSSMulticastAuthCipherPairs, uIBSSMulticastAuthCipherPairsSize);
            }
            
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_SUPPORTED_UNICAST_ALGORITHM_PAIR (BssType=%d,NumOfAuthCipherPair=%d)\n",
                                        pAd->StaCfg.BssType, MaxAuthCipherPair));   
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQuerySupportedMulticastCipherAlgorithm()
        ULONG MaxAuthCipherPair;
        PDOT11_AUTH_CIPHER_PAIR_LIST pAuthCipherList = (PDOT11_AUTH_CIPHER_PAIR_LIST)InformationBuffer;
        
        do
        {
            MaxAuthCipherPair = uBSSMulticastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);

            TotalLen = FIELD_OFFSET(DOT11_AUTH_CIPHER_PAIR_LIST, AuthCipherPairs) + MaxAuthCipherPair * sizeof(DOT11_AUTH_CIPHER_PAIR);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pAuthCipherList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pAuthCipherList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_AUTH_CIPHER_PAIR_LIST_REVISION_1,
                                        sizeof(DOT11_AUTH_CIPHER_PAIR_LIST));
            
            pAuthCipherList->uNumOfEntries = MaxAuthCipherPair;

            PlatformMoveMemory(pAuthCipherList->AuthCipherPairs, BSSMulticastAuthCipherPairs, uBSSMulticastAuthCipherPairsSize);
            *BytesWritten = TotalLen;
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
N6QueryOidDot11EnabledUnicastCipherAlgorithm(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_CIPHER_ALGORITHM_LIST pAuthCipherList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;

            TotalLen = FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds) + sizeof(DOT11_CIPHER_ALGORITHM);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pAuthCipherList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pAuthCipherList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_CIPHER_ALGORITHM_LIST_REVISION_1,
                                        sizeof(DOT11_CIPHER_ALGORITHM_LIST));

            pAuthCipherList->uNumOfEntries = 1;
            pAuthCipherList->uTotalNumOfEntries = 1;

            if ((pAd->OpMode == OPMODE_STAP2P) && (PORTV2_P2P_ON(pAd, pPort)))
            {
                pAuthCipherList->AlgorithmIds[0] =  VPORT_DOT11_CIPHER_ALGO_CCMP;
            }
            else if (IS_P2P_STA_GO(pAd, pPort) && (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort)))
            {
                pAuthCipherList->AlgorithmIds[0] =  DOT11_CIPHER_ALGO_NONE;
            }
            else
            {
                pAuthCipherList->AlgorithmIds[0] =  pPort->PortCfg.UnicastCipherAlgorithm;
            }
            
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ENABLED_UNICAST_CIPHER_ALGORITHM (=%x)\n", pAuthCipherList->AlgorithmIds[0]));
        } while (FALSE);
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
N6QueryOidDot11EnabledMulticastCipherAlgorithm(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_CIPHER_ALGORITHM_LIST pAuthCipherList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;

            if ((pAd->OpMode == OPMODE_STAP2P) && (PORTV2_P2P_ON(pAd, pPort)))
            {
                TotalLen = FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds) + sizeof(DOT11_CIPHER_ALGORITHM);
            }
            else
            {
                TotalLen = FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds) + 
                                pPort->PortCfg.MulticastCipherAlgorithmCount * sizeof(DOT11_CIPHER_ALGORITHM);
            }
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pAuthCipherList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pAuthCipherList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_CIPHER_ALGORITHM_LIST_REVISION_1,
                                        sizeof(DOT11_CIPHER_ALGORITHM_LIST));

            if (IS_P2P_STA_GO(pAd, pPort) && (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort)))
            {
                pPort->PortCfg.MulticastCipherAlgorithmList[0] = DOT11_CIPHER_ALGO_NONE;
                pPort->PortCfg.MulticastCipherAlgorithmCount = 1;
            }
            
            if ((pAd->OpMode == OPMODE_STAP2P) && (PORTV2_P2P_ON(pAd, pPort)))
            {
                pAuthCipherList->uNumOfEntries = 1;
                pAuthCipherList->uTotalNumOfEntries = 1;
                pAuthCipherList->AlgorithmIds[0] = VPORT_DOT11_CIPHER_ALGO_CCMP;
            }
            else
            {
                pAuthCipherList->uNumOfEntries = pPort->PortCfg.MulticastCipherAlgorithmCount;
                pAuthCipherList->uTotalNumOfEntries = pPort->PortCfg.MulticastCipherAlgorithmCount;
                PlatformMoveMemory(pAuthCipherList->AlgorithmIds,
                                pPort->PortCfg.MulticastCipherAlgorithmList,
                                pPort->PortCfg.MulticastCipherAlgorithmCount * sizeof(DOT11_CIPHER_ALGORITHM));
            }
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;               
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ENABLED_MULTICAST_CIPHER_ALGORITHM (AlgorithmCount=%d)\n", pAuthCipherList->uNumOfEntries));
        } while (FALSE);
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
N6QueryOidDot11CipherDefaultKeyId(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->PortCfg.DefaultKeyId;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_CIPHER_DEFAULT_KEY_ID (=%d)\n", uInfo));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryCipherDefaultKeyId()
        do
        {
            *BytesWritten = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG); 
                ndisStatus = NDIS_STATUS_INVALID_LENGTH; 
                break; 
            }
            *(PULONG)InformationBuffer = pPort->PortCfg.DefaultKeyId;       //yfczz_add
            *BytesWritten = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) QUERY::OID_DOT11_CIPHER_DEFAULT_KEY_ID (=%d)\n", PortNumber, *(PULONG)InformationBuffer));
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
N6QueryOidDot11CipherDefaultKey(
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
N6QueryOidDot11CipherKeyMappingKey(
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
N6QueryOidDot11EnumAssociationInfo(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG                       RemainingBytes;
            PRTMP_ADHOC_LIST            pAdhocEntry;
            PDOT11_ASSOCIATION_INFO_EX  pAssocInfo = NULL;
            PDOT11_ASSOCIATION_INFO_LIST pAssocInfoList;
            //PDOT11_ASSOCIATION_INFO_LIST pAssocInfoList = (PDOT11_ASSOCIATION_INFO_LIST)InformationBuffer;

            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ENUM_ASSOCIATION_INFO, InformationBufferLength = %d\n", InformationBufferLength));

            if (InformationBufferLength < sizeof(DOT11_ASSOCIATION_INFO_LIST))
            {
                if ( InformationBuffer == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("InformationBuffer is null\n"));
                }
                *BytesNeeded = sizeof(DOT11_ASSOCIATION_INFO_LIST);                 
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            pAssocInfoList = (PDOT11_ASSOCIATION_INFO_LIST)InformationBuffer;
            PlatformZeroMemory(pAssocInfoList, InformationBufferLength);
            
            MP_ASSIGN_NDIS_OBJECT_HEADER(pAssocInfoList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_ASSOCIATION_INFO_LIST_REVISION_1,
                                        sizeof(DOT11_ASSOCIATION_INFO_LIST));

            if (pAd->StaCfg.BssType == dot11_BSS_type_infrastructure && INFRA_ON(pPort))
            {
                pAssocInfo = &(pAssocInfoList->dot11AssocInfo[0]);
                PlatformZeroMemory(pAssocInfo, sizeof(DOT11_ASSOCIATION_INFO_EX));

                COPY_MAC_ADDR(pAssocInfo->PeerMacAddress, pPort->PortCfg.Bssid);
                COPY_MAC_ADDR(pAssocInfo->BSSID, pPort->PortCfg.Bssid);

                pAssocInfo->usCapabilityInformation = pAd->MlmeAux.CapabilityInfo;
                pAssocInfo->usListenInterval = pAd->MlmeAux.BeaconPeriod;

                pAssocInfo->usAssociationID = pAd->MlmeAux.Aid;
                pAssocInfo->dot11AssociationState = pAd->MlmeAux.AssocState;
                pAssocInfo->liAssociationUpTime.QuadPart = pAd->MlmeAux.AssociationUpTime.QuadPart;
                pAssocInfoList->uNumOfEntries = 1;
                pAssocInfoList->uTotalNumOfEntries = 1;
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ENUM_ASSOCIATION_INFO (BSSID[%02x:%02x:%02x:%02x:%02x:%02x:],AID=%d,AssocState=%d,BeaconPeriod=%d)\n",
                        pPort->PortCfg.Bssid[0],pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],
                        pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5], 
                        pAd->MlmeAux.Aid, pAssocInfo->dot11AssociationState, pAd->MlmeAux.BeaconPeriod));
            }
            else if ((pAd->StaCfg.BssType == dot11_BSS_type_independent) && (pAd->StaCfg.ActiveAdhocListQueue.Number > 0))
            {
                //In WLK 1.1 encryption_cmn test(IBSS), sometimes and very often WLK query ASSOC_INFO immediately after 
                //ASSOC_START is indicated when ADHOC_ON flag is not set. If we use ADHOC_ON flag, the query will fail,
                //and WLK will send packets immediately and there will be packet loss.
                RemainingBytes = InformationBufferLength - FIELD_OFFSET(DOT11_ASSOCIATION_INFO_LIST, dot11AssocInfo);
                pAssocInfo = &(pAssocInfoList->dot11AssocInfo[0]);
                DBGPRINT(RT_DEBUG_TRACE, ("adhoclist count =%d\n", pAd->StaCfg.ActiveAdhocListQueue.Number));
                NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
                pAdhocEntry = (PRTMP_ADHOC_LIST)pAd->StaCfg.ActiveAdhocListQueue.Head;
                while (pAdhocEntry)
                {
                    if (RemainingBytes >= sizeof(DOT11_ASSOCIATION_INFO_EX))
                    {
                        COPY_MAC_ADDR(&pAssocInfo->PeerMacAddress, pAdhocEntry->PeerMacAddress);
                        COPY_MAC_ADDR(&pAssocInfo->BSSID, pAdhocEntry->Bssid);
                        pAssocInfo->usCapabilityInformation = pAdhocEntry->CapabilityInfo;
                        pAssocInfo->usListenInterval = pAdhocEntry->BeaconInterval;

                        pAssocInfo->dot11AssociationState = pAdhocEntry->AssocState;
                        if (pAssocInfo->dot11AssociationState == dot11_assoc_state_zero)
                        {
                            pAssocInfo->dot11AssociationState = dot11_assoc_state_unauth_unassoc;
                        }
                        
                        pAssocInfoList->uNumOfEntries++;
                        pAssocInfoList->uTotalNumOfEntries++;
                        RemainingBytes -= sizeof(DOT11_ASSOCIATION_INFO_EX);
                        pAssocInfo++;
                    }
                    else
                    {
                        // Not enough space to store this entry
                        pAssocInfoList->uTotalNumOfEntries++;
                        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                        RemainingBytes = 0;
                        //
                        // We continue walking through the list to determine the total
                        // space required for this OID
                        //
                    }
                    pAdhocEntry = (PRTMP_ADHOC_LIST)pAdhocEntry->Next;
                }
                NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("return NDIS_STATUS_INVALID_STATE\n"));               
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
            }

            *BytesWritten = pAssocInfoList->uTotalNumOfEntries * sizeof(DOT11_ASSOCIATION_INFO_EX) + 
                            FIELD_OFFSET(DOT11_ASSOCIATION_INFO_LIST, dot11AssocInfo);
            if (ndisStatus == NDIS_STATUS_BUFFER_OVERFLOW)
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_ASSOCIATION_INFO_LIST, dot11AssocInfo) +
                    (sizeof(DOT11_ASSOCIATION_INFO_EX) * pAd->StaCfg.ActiveAdhocListQueue.Number);
            }
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
N6QueryOidDot11HardwarePhyState(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pAd->StaCfg.bHwRadio;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_HARDWARE_PHY_STATE (=%d)\n", uInfo));
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
N6QueryOidDot11MediaStreamingEnabled(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->CommonCfg.bMediaStreamingEnabled;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE,("Query::OID_DOT11_MEDIA_STREAMING_ENABLED (=%d)\n", uInfo));
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
N6QueryOidDot11UnreachableDetectionThreshold(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->CommonCfg.UnreachableDetectionThreshold;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_UNREACHABLE_DETECTION_THRESHOLD (=%d)\n", uInfo));
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
N6QueryOidDot11ActivePhyList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_PHY_ID_LIST PhyIdList = (PDOT11_PHY_ID_LIST)InformationBuffer;

            if (!MP_VERIFY_STATE(pPort, OP_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
            }
            
            //
            // only supports one active PHY at a time.
            //
            TotalLen = FIELD_OFFSET(DOT11_PHY_ID_LIST, dot11PhyId) + sizeof(ULONG);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(PhyIdList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(PhyIdList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_PHY_ID_LIST_REVISION_1,
                                        sizeof(DOT11_PHY_ID_LIST));
            //
            // Copy the desired PHY list.
            //
            PhyIdList->uNumOfEntries = 1;
            PhyIdList->dot11PhyId[0] = pAd->StaCfg.ActivePhyId;
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_ACTIVE_PHY_LIST (ActivePhyId=%d)\n", PhyIdList->dot11PhyId[0]));
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
N6QueryOidDot11ExtstaCapability(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_EXTSTA_CAPABILITY pDot11ExtStaCap = (PDOT11_EXTSTA_CAPABILITY)InformationBuffer;

            if (InformationBufferLength < sizeof(DOT11_EXTSTA_CAPABILITY))
            {
                *BytesNeeded = sizeof(DOT11_EXTSTA_CAPABILITY);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pDot11ExtStaCap, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11ExtStaCap->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_EXTSTA_CAPABILITY_REVISION_1,
                                        sizeof(DOT11_EXTSTA_CAPABILITY));

            pDot11ExtStaCap->uScanSSIDListSize = 4;     // minimum required.
            pDot11ExtStaCap->uDesiredBSSIDListSize = 1; // We only support one BSSID. 
            pDot11ExtStaCap->uDesiredSSIDListSize = 1;
            pDot11ExtStaCap->uExcludedMacAddressListSize = 4;
            pDot11ExtStaCap->uPrivacyExemptionListSize = 32;
            pDot11ExtStaCap->uKeyMappingTableSize = KEY_MAPPING_KEY_TABLE_SIZE;
            pDot11ExtStaCap->uDefaultKeyTableSize = DOT11_MAX_NUM_DEFAULT_KEY;
            pDot11ExtStaCap->uWEPKeyValueMaxLength = 13;  //Max for WEP104
            pDot11ExtStaCap->uPMKIDCacheSize = STA_PMKID_MAX_COUNT;
            pDot11ExtStaCap->uMaxNumPerSTADefaultKeyTables = PER_STA_KEY_TABLE_SIZE;
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = sizeof(DOT11_EXTSTA_CAPABILITY);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_EXTSTA_CAPABILITY \n"));
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
N6QueryOidDot11DataRateMappingTable(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_DATA_RATE_MAPPING_TABLE pDataRateMappingTable = (PDOT11_DATA_RATE_MAPPING_TABLE)InformationBuffer;

            TotalLen = FIELD_OFFSET(DOT11_DATA_RATE_MAPPING_TABLE, DataRateMappingEntries) + sizeof(Std_abg_DataRateMappingTable);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pDataRateMappingTable, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDataRateMappingTable->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_DATA_RATE_MAPPING_TABLE_REVISION_1,
                                        sizeof(DOT11_DATA_RATE_MAPPING_TABLE));

            pDataRateMappingTable->uDataRateMappingLength = sizeof(Std_abg_DataRateMappingTable) / sizeof(DOT11_DATA_RATE_MAPPING_ENTRY);

            //
            // Copy the standard a/b/g data rate mapping table.
            //
            PlatformMoveMemory(pDataRateMappingTable->DataRateMappingEntries, Std_abg_DataRateMappingTable, sizeof(Std_abg_DataRateMappingTable));
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_DATA_RATE_MAPPING_TABLE\n"));
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
N6QueryOidDot11MaximumListSize(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;

    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6QueryOidDot11PrivacyExemptionList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG  uNumOfEntries;
            PDOT11_PRIVACY_EXEMPTION_LIST pPrivacyExemptionList = (PDOT11_PRIVACY_EXEMPTION_LIST)InformationBuffer;

             if (InformationBufferLength < FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            if ((pPort->PortCfg.PrivacyExemptionList == NULL) || (pPort->PortCfg.PrivacyExemptionList->uNumOfEntries == 0))
            {
                uNumOfEntries = 0;
            }
            else
            {
                uNumOfEntries = pPort->PortCfg.PrivacyExemptionList->uNumOfEntries;
            }

            TotalLen = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries) + uNumOfEntries * sizeof(DOT11_PRIVACY_EXEMPTION);
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pPrivacyExemptionList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pPrivacyExemptionList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_PRIVACY_EXEMPTION_LIST_REVISION_1,
                                        sizeof(DOT11_PRIVACY_EXEMPTION_LIST));

            pPrivacyExemptionList->uNumOfEntries = uNumOfEntries;
            if (uNumOfEntries > 0)
            {
                PlatformMoveMemory(pPrivacyExemptionList->PrivacyExemptionEntries,
                                pPort->PortCfg.PrivacyExemptionList->PrivacyExemptionEntries,
                                uNumOfEntries * sizeof(DOT11_PRIVACY_EXEMPTION));
            }

            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            //*BytesWritten = sizeof(DOT11_EXTSTA_CAPABILITY);;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_PRIVACY_EXEMPTION_LIST (PrivacyExemptionListCount=%d)\n", pPrivacyExemptionList->uNumOfEntries));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidQueryPrivacyExemptionList()
        ULONG                   uNumOfEntries;
        PDOT11_PRIVACY_EXEMPTION_LIST pPrivacyExemptionList = (PDOT11_PRIVACY_EXEMPTION_LIST)InformationBuffer;

        do
        {
            *BytesWritten = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            //
            //GetRequiredListSize
            //
            if ((pPort->PortCfg.PrivacyExemptionList == NULL) || (pPort->PortCfg.PrivacyExemptionList->uNumOfEntries == 0))
            {
                uNumOfEntries = 0;
            }
            else
            {
                uNumOfEntries = pPort->PortCfg.PrivacyExemptionList->uNumOfEntries;
            }
        
            TotalLen = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries) + uNumOfEntries * sizeof(DOT11_PRIVACY_EXEMPTION);
            
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            
            PlatformZeroMemory(pPrivacyExemptionList, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pPrivacyExemptionList->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_PRIVACY_EXEMPTION_LIST_REVISION_1,
                                        sizeof(DOT11_PRIVACY_EXEMPTION_LIST));

            pPrivacyExemptionList->uNumOfEntries = uNumOfEntries;
            if ((pPort->PortCfg.PrivacyExemptionList != NULL) && (uNumOfEntries > 0))
            {
                PlatformMoveMemory(pPrivacyExemptionList->PrivacyExemptionEntries,
                    pPort->PortCfg.PrivacyExemptionList->PrivacyExemptionEntries,
                    uNumOfEntries * sizeof(DOT11_PRIVACY_EXEMPTION));
            }

            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            
            DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) QUERY::OID_DOT11_PRIVACY_EXEMPTION_LIST (PrivacyExemptionListCount=%d)\n", PortNumber, pPrivacyExemptionList->uNumOfEntries));

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
N6QueryOidDot11IbssParams(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PDOT11_IBSS_PARAMS  pDot11IBSSParams = (PDOT11_IBSS_PARAMS)InformationBuffer;

            if (InformationBufferLength < sizeof(DOT11_IBSS_PARAMS))
                {
                *BytesNeeded = sizeof(DOT11_IBSS_PARAMS);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
                }

            TotalLen = sizeof(DOT11_IBSS_PARAMS) + pPort->PortCfg.AdditionalBeaconIESize;
            if (InformationBufferLength < TotalLen)
            {
                *BytesNeeded = TotalLen;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            PlatformZeroMemory(pDot11IBSSParams, InformationBufferLength);

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11IBSSParams->Header,
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_IBSS_PARAMS_REVISION_1,
                                        sizeof(DOT11_IBSS_PARAMS));

            pDot11IBSSParams->bJoinOnly = pAd->StaCfg.IBSSJoinOnly;
            pDot11IBSSParams->uIEsLength = pPort->PortCfg.AdditionalBeaconIESize;
            pDot11IBSSParams->uIEsOffset = sizeof(DOT11_IBSS_PARAMS);

            if (pPort->PortCfg.AdditionalBeaconIESize > 0)
            {
                PlatformMoveMemory(Add2Ptr(pDot11IBSSParams, sizeof(DOT11_IBSS_PARAMS)),
                                pPort->PortCfg.AdditionalBeaconIEData,
                                pPort->PortCfg.AdditionalBeaconIESize);
            }
            //
            // Exit and don't copy result into InformationBuffer
            //
            *BytesWritten = TotalLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_IBSS_PARAMS (bJoinOnly=%d,IE-Len=%d) \n", pDot11IBSSParams->bJoinOnly, pDot11IBSSParams->uIEsLength));
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
N6QueryOidDot11PmkidList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC RTMPQueryPMKIDList()
        PDOT11_PMKID_LIST pPMKIDList;
        ULONG i = 0;

        do
        {
            *BytesWritten = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs))
            {
                DBGPRINT_ERR(("%s: NDIS_STATUS_BUFFER_OVERFLOW #1\n", __FUNCTION__));

                *BytesNeeded = FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            
            PlatformZeroMemory(InformationBuffer, InformationBufferLength);

            pPMKIDList = (PDOT11_PMKID_LIST)InformationBuffer;

            MP_ASSIGN_NDIS_OBJECT_HEADER(pPMKIDList->Header, 
                NDIS_OBJECT_TYPE_DEFAULT,
                DOT11_PMKID_LIST_REVISION_1,
                sizeof(DOT11_PMKID_LIST));

            pPMKIDList->uTotalNumOfEntries = pAd->StaCfg.PMKIDCount;
            pPMKIDList->uNumOfEntries = 0;

            // Integer overflow checking.
            if (FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs) > 
                (FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs) + 
                (pAd->StaCfg.PMKIDCount * sizeof(DOT11_PMKID_ENTRY))))
            {
                DBGPRINT_ERR(("%s: NDIS_STATUS_FAILURE\n", __FUNCTION__));
                
                return NDIS_STATUS_FAILURE;
            }

            // Verify size.
            if (InformationBufferLength < 
                (FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs) + 
                (pAd->StaCfg.PMKIDCount * sizeof(DOT11_PMKID_ENTRY))))
            {
                DBGPRINT_ERR(("%s: NDIS_STATUS_BUFFER_OVERFLOW #2\n", __FUNCTION__));

                pPMKIDList->uNumOfEntries = 0;
                pPMKIDList->uTotalNumOfEntries = pAd->StaCfg.PMKIDCount;
                *BytesWritten = 0;
                *BytesNeeded = FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs) + 
                                  (pAd->StaCfg.PMKIDCount * sizeof(DOT11_PMKID_ENTRY));
                return NDIS_STATUS_BUFFER_OVERFLOW;
            }

            // Copy the PMKID list.
            pPMKIDList->uNumOfEntries = pAd->StaCfg.PMKIDCount;
            pPMKIDList->uTotalNumOfEntries = pAd->StaCfg.PMKIDCount;
            PlatformMoveMemory(pPMKIDList->PMKIDs,
                            pAd->StaCfg.PMKIDList,
                            (pAd->StaCfg.PMKIDCount * sizeof(DOT11_PMKID_ENTRY)));

            *BytesNeeded = 0;
            *BytesWritten = (pPMKIDList->uNumOfEntries * sizeof(DOT11_PMKID_ENTRY)) + 
                                FIELD_OFFSET(DOT11_PHY_ID_LIST, dot11PhyId);

            for (i = 0; i < pAd->StaCfg.PMKIDCount; i++)
            {
                DBGPRINT(RT_DEBUG_INFO, ("%s: BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                    __FUNCTION__, 
                    pAd->StaCfg.PMKIDList[i].BSSID[0], pAd->StaCfg.PMKIDList[i].BSSID[1], pAd->StaCfg.PMKIDList[i].BSSID[2], 
                    pAd->StaCfg.PMKIDList[i].BSSID[3], pAd->StaCfg.PMKIDList[i].BSSID[4], pAd->StaCfg.PMKIDList[i].BSSID[5]));
                DBGPRINT(RT_DEBUG_INFO, ("%s: uFlags = %d; PMKID = %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n", 
                    __FUNCTION__, 
                    pAd->StaCfg.PMKIDList[i].uFlags, 
                    pAd->StaCfg.PMKIDList[i].PMKID[0], pAd->StaCfg.PMKIDList[i].PMKID[1], pAd->StaCfg.PMKIDList[i].PMKID[2], pAd->StaCfg.PMKIDList[i].PMKID[3], 
                    pAd->StaCfg.PMKIDList[i].PMKID[4], pAd->StaCfg.PMKIDList[i].PMKID[5], pAd->StaCfg.PMKIDList[i].PMKID[6], pAd->StaCfg.PMKIDList[i].PMKID[7], 
                    pAd->StaCfg.PMKIDList[i].PMKID[8], pAd->StaCfg.PMKIDList[i].PMKID[9], pAd->StaCfg.PMKIDList[i].PMKID[10], pAd->StaCfg.PMKIDList[i].PMKID[11], 
                    pAd->StaCfg.PMKIDList[i].PMKID[12], pAd->StaCfg.PMKIDList[i].PMKID[13], pAd->StaCfg.PMKIDList[i].PMKID[14], pAd->StaCfg.PMKIDList[i].PMKID[15]));
            }
        } while (FALSE);

        DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_PMKID_LIST\n"));
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
N6QueryOidDot11UnicastUseGroupEnabled(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->CommonCfg.bUnicastUseGroupEnabled;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_UNICAST_USE_GROUP_ENABLED (=%d)\n", uInfo));
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
N6QueryOidDot11HiddenNetworkEnabled(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->PortCfg.bHiddenNetworkEnabled;
            uInfoLen = sizeof(BOOLEAN);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_HIDDEN_NETWORK_ENABLED (=%d)\n", uInfo));
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
N6QueryOidDot11QosParams(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->CommonCfg.SafeModeQOS_PARAMS;
            uInfoLen = sizeof(UCHAR);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_QOS_PARAMS(=%d)\n", uInfo));   
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
N6QueryOidDot11AvailableChannelList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            //
            // Current we didn't support it.
            //
            DBGPRINT(RT_DEBUG_INFO, ("Query::OID_DOT11_AVAILABLE_CHANNEL_LIST\n"));
        } while (FALSE);
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
N6QueryOidDot11PortStateNotification(
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
N5QueryOidGenSupportedList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = (PVOID)&NICSupportedOids;
            uInfoLen = NIC_SUPPORT_OID_SIZE;
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
N5QueryOidGenHardwareStatus(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = NdisHardwareStatusReady;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_HARDWARE_STATUS (=%d)\n", uInfo));
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
N5QueryOidGenMediaSupported(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = NdisMediumNative802_11;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_MEDIA_IN_USE (=%d)\n", uInfo));
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
N5QueryOidGenMediaInUse(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = NdisMediumNative802_11;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_MEDIA_IN_USE (=%d)\n", uInfo));
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
N5QueryOidGenPhysicalMedium(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;

    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N5QueryOidGenMaximumLookahead(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;

    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N5QueryOidGenMaximumFrameSize(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = MP_802_11_MAX_FRAME_SIZE - MP_802_11_SHORT_HEADER_SIZE;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Queyr::OID_GEN_MAXIMUM_FRAME_SIZE (=%d)\n", uInfo));
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
N5QueryOidGenLinkSpeed(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            UCHAR   initOffset = 16;
            ULONG   rateIdx;
            UCHAR   ShortGI;

            if (pPort->PortSubtype == PORTSUBTYPE_STA || (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
            {
                if(READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_CCK)
                {
                    switch (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg))
                    {
                        case RATE_1:   uInfo = 10000;    break;
                        case RATE_2:   uInfo = 20000;    break;
                        case RATE_5_5: uInfo = 55000;    break;
                        case RATE_11:  uInfo = 110000;   break;
                        default:       uInfo = 10000;    break;
                    }
                }
                if(READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_OFDM)
                {
                    switch (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg))
                    {
                        case 0:   uInfo = 60000;    break;
                        case 1:   uInfo = 90000;    break;
                        case 2:  uInfo = 120000;    break;
                        case 3:  uInfo = 180000;    break;
                        case 4:  uInfo = 240000;    break;
                        case 5:  uInfo = 360000;    break;
                        case 6:  uInfo = 480000;    break;
                        case 7:  uInfo = 540000;    break;
                        default:       uInfo = 10000;    break;
                    }
                }
                else
                {
                    // AP report MAX MCS = 15.  Station report currently used rate. So I fix AP report use MCS=15                       
                    ShortGI = (UCHAR)READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg);

                    if (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_VHT)
                    {
                        rateIdx = GetVhtRateTableIdx(pAd, 
                                                    READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg),
                                                    READ_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg), 
                                                    READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg), 
                                                    READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
                    }
                    else
                    {
                    //user request show 300M when device idle and rate is 270M
                    if((!VHT_CAPABLE(pAd)) && 
                            (pAd->StaCfg.bIdle300M == TRUE) && 
                            ((READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_15) || 
                            (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_7)))
                    {
                        ShortGI = GI_400;
                    }
                    
                    if (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) > MCS_15)
                    {
                            rateIdx = 64 + ((UCHAR)READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg) * 16) + (ShortGI * 32) + ((UCHAR)READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
                        }
                        else
                        {
                            rateIdx = 16 + ((UCHAR)READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg) * 16) + (ShortGI * 32) + ((UCHAR)READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
                        }
                    }


                    if (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_VHT)
                    {
                        uInfo = VhtRateTable[rateIdx] * 5000;
                    }
                    else
                    {
                        uInfo = CckOfdmHtRateTable[rateIdx] * 5000;
                    }
                }
            }
            else
            {
                // BG legacy mode.
                if ((pPort->CommonCfg.bProhibitTKIPonHT) && 
                    ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||
                     (pPort->PortCfg.CipherAlg == CIPHER_TKIP)||
                     (pPort->PortCfg.CipherAlg == CIPHER_TKIP_NO_MIC)))
                {   
                    if (pPort->CommonCfg.PhyMode == PHY_11B)
                    {
                        // maximum 11b rate
                        uInfo = 110000;
                    }
                    else
                    {
                        // maximum 11g rate
                        uInfo = 540000; 
                    }               
                }
                else
                {
                    // Report maximize Tx Rate
                    UCHAR TXMaxMCS = 15;
                    
                    if (pAd->HwCfg.Antenna.field.TxPath == 1)
                    {
                        TXMaxMCS = 7;
                    }
                    else if (pAd->HwCfg.Antenna.field.TxPath == 2)
                    {
                        TXMaxMCS = 15;
                    }
                    else if (pAd->HwCfg.Antenna.field.TxPath == 3)
                    {
                        TXMaxMCS = MCS_23;
                        initOffset = 64;
                    }
                    else
                    {
                        ASSERT(0);
                    }
                    
                    rateIdx = initOffset + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth *16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 *32) + TXMaxMCS;
                    uInfo = CckOfdmHtRateTable[rateIdx] *5000;
                }
            }
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_INFO, ("Query::OID_GEN_LINK_SPEED (=%d)\n", uInfo));
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
N5QueryOidGenTransmitBufferSpace(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = MP_802_11_MAX_FRAME_SIZE * TX_RING_SIZE;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_TRANSMIT_BUFFER_SPACE (=%d)\n", uInfo));
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
N5QueryOidGenReceiveBufferSpace(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = MP_802_11_MAX_FRAME_SIZE * RX_RING_SIZE_END_INDEX;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_RECEIVE_BUFFER_SPACE (=%d)\n", uInfo));
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
N5QueryOidGenTransmitBlockSize(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = (ULONG) MP_802_11_MAX_FRAME_SIZE;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_MAXIMUM_TOTAL_SIZE (=%d)\n", uInfo));
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
N5QueryOidGenReceiveBlockSize(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = (ULONG) MP_802_11_MAX_FRAME_SIZE;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_MAXIMUM_TOTAL_SIZE (=%d)\n", uInfo));
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
N5QueryOidGenVendorId(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            PlatformMoveMemory(pInfo, pAd->HwCfg.PermanentAddress,  3);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_VENDOR_ID (=0x%08x)\n", uInfo));
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
N5QueryOidGenVendorDescription(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = pAd->pNicCfg->VendorDesc;
            uInfoLen = pAd->pNicCfg->VendorDescLen;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_VENDOR_DESCRIPTION (=%s)\n", (PUCHAR)pInfo));
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
N5QueryOidGenVendorDriverVersion(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = NIC_VENDOR_DRIVER_VERSION;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_VENDOR_DRIVER_VERSION (=0x%08x)\n", uInfo));
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
N5QueryOidGenCurrentPacketFilter(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = pPort->PortCfg.PacketFilter;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_CURRENT_PACKET_FILTER (=0x%08x)\n", uInfo));
        } while (FALSE);
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
N5QueryOidGenCurrentLookahead(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = MP_802_11_MAXIMUM_LOOKAHEAD;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_CURRENT_LOOKAHEAD (=%d)\n", uInfo));
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
N5QueryOidGenDriverVersion(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            USHORT usInfo = 0;
            usInfo = (USHORT) NIC_VENDOR_DRIVER_VERSION;
            uInfoLen = sizeof(USHORT);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_DRIVER_VERSION (=0x%x)\n", uInfo));
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
N5QueryOidGenMaximumTotalSize(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = (ULONG) MP_802_11_MAX_FRAME_SIZE;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_MAXIMUM_TOTAL_SIZE (=%d)\n", uInfo));
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
N5QueryOidGenProtocolOptions(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;

    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N5QueryOidGenMacOptions(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                     NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                     NDIS_MAC_OPTION_NO_LOOPBACK        |
                     NDIS_MAC_OPTION_8021P_PRIORITY;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_MAC_OPTIONS (=%d)\n", uInfo));
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
N5QueryOidGenMediaConnectStatus(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            //
            // We will always report connected. The nwifi IM driver
            // takes care of the logic of this for the miniport.
            //
            uInfo = NdisMediaStateConnected;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_MEDIA_CONNECT_STATUS (=%d)\n", uInfo));
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
N5QueryOidGenXmitOk(
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
 
    ULONG64     ullInfo64 = 0;
            
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = &ullInfo64;
            ndisStatus = Ndis6CommonQueryStatistics(
                                    pAd,
                                    Oid,
                                    &ullInfo64,
                                    BytesWritten
                                    );
            uInfoLen = *BytesWritten;
            uInfoLen = sizeof(ullInfo64);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_XMIT_OK Not Support\n"));
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
N5QueryOidGenRcvOk(
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
 
    ULONG64     ullInfo64 = 0;
            
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = &ullInfo64;
            ndisStatus = Ndis6CommonQueryStatistics(
                                    pAd,
                                    Oid,
                                    &ullInfo64,
                                    BytesWritten
                                    );
            uInfoLen = *BytesWritten;
            uInfoLen = sizeof(ullInfo64);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_XMIT_OK Not Support\n"));
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
N5QueryOidGenXmitError(
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
 
    ULONG64     ullInfo64 = 0;
            
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = &ullInfo64;
            ndisStatus = Ndis6CommonQueryStatistics(
                                    pAd,
                                    Oid,
                                    &ullInfo64,
                                    BytesWritten
                                    );
            uInfoLen = *BytesWritten;
            uInfoLen = sizeof(ullInfo64);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_XMIT_OK Not Support\n"));
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
N5QueryOidGenRcvError(
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
 
    ULONG64     ullInfo64 = 0;
            
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = &ullInfo64;
            ndisStatus = Ndis6CommonQueryStatistics(
                                    pAd,
                                    Oid,
                                    &ullInfo64,
                                    BytesWritten
                                    );
            uInfoLen = *BytesWritten;
            uInfoLen = sizeof(ullInfo64);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_XMIT_OK Not Support\n"));
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
N5QueryOidGenRcvNoBuffer(
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
 
    ULONG64     ullInfo64 = 0;
            
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = &ullInfo64;
            ndisStatus = Ndis6CommonQueryStatistics(
                                    pAd,
                                    Oid,
                                    &ullInfo64,
                                    BytesWritten
                                    );
            uInfoLen = *BytesWritten;
            uInfoLen = sizeof(ullInfo64);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_XMIT_OK Not Support\n"));
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
N5QueryOidGenRcvCrcError(
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
 
    ULONG64     ullInfo64 = 0;
            
    ULONG       TotalLen = 0;
    ULONG       uInfo = 0;
    ULONG       uInfoLen = 0;
    PVOID       pInfo = (PVOID)&uInfo;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = &ullInfo64;
            ndisStatus = Ndis6CommonQueryStatistics(
                                    pAd,
                                    Oid,
                                    &ullInfo64,
                                    BytesWritten
                                    );
            uInfoLen = *BytesWritten;
            uInfoLen = sizeof(ullInfo64);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_XMIT_OK Not Support\n"));
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
N5QueryOidGenTransmitQueueLength(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = 0;  // TODO: need to update this value.
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_TRANSMIT_QUEUE_LENGTH not Support\n"));
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
N5QueryOid8023PermanentAddress(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = pAd->HwCfg.PermanentAddress;
            uInfoLen = MAC_ADDR_LEN;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_3_PERMANENT_ADDRESS (=%02x:%02x:%02x:%02x:%02x:%02x)\n",
                        pAd->HwCfg.PermanentAddress[0], pAd->HwCfg.PermanentAddress[1], pAd->HwCfg.PermanentAddress[2],
                        pAd->HwCfg.PermanentAddress[3], pAd->HwCfg.PermanentAddress[4], pAd->HwCfg.PermanentAddress[5]));
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
N5QueryOidDot11PermanentAddress(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = pAd->HwCfg.PermanentAddress;
            uInfoLen = MAC_ADDR_LEN;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_3_PERMANENT_ADDRESS (=%02x:%02x:%02x:%02x:%02x:%02x)\n",
                        pAd->HwCfg.PermanentAddress[0], pAd->HwCfg.PermanentAddress[1], pAd->HwCfg.PermanentAddress[2],
                        pAd->HwCfg.PermanentAddress[3], pAd->HwCfg.PermanentAddress[4], pAd->HwCfg.PermanentAddress[5]));
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
N5QueryOid8023CurrentAddress(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = pPort->CurrentAddress;
            uInfoLen = MAC_ADDR_LEN;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_3_CURRENT_ADDRESS (=%02x:%02x:%02x:%02x:%02x:%02x)\n",
                        pPort->CurrentAddress[0], pPort->CurrentAddress[1], pPort->CurrentAddress[2],
                        pPort->CurrentAddress[3], pPort->CurrentAddress[4], pPort->CurrentAddress[5]));
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
N5QueryOidDot11CurrentAddress(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            pInfo = pPort->CurrentAddress;
            uInfoLen = MAC_ADDR_LEN;
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_3_CURRENT_ADDRESS (=%02x:%02x:%02x:%02x:%02x:%02x)\n",
                        pPort->CurrentAddress[0], pPort->CurrentAddress[1], pPort->CurrentAddress[2],
                        pPort->CurrentAddress[3], pPort->CurrentAddress[4], pPort->CurrentAddress[5]));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC NONE
        PlatformMoveMemory(InformationBuffer, pPort->CurrentAddress, MAC_ADDR_LEN);
        *BytesWritten  = *BytesNeeded = MAC_ADDR_LEN;
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
N5QueryOidDot11MulticastList(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfoLen = pPort->PortCfg.MCAddressCount * MAC_ADDR_LEN;
            if (InformationBufferLength < uInfoLen)
            {
                ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
                *BytesNeeded = uInfoLen;
                break;
            }
            else
            {
                pInfo = (PVOID) pPort->PortCfg.MCAddressList;               
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_DOT11_MULTICAST_LIST  (count =%d)\n", pPort->PortCfg.MCAddressCount ));
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
N5QueryOid8023MaximumListSize(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            uInfo = HW_MAX_MCAST_LIST_SIZE;
            uInfoLen = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_3_MAXIMUM_LIST_SIZE (=%d)\n", uInfo));
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
#if 0
NDIS_STATUS
N5QueryOidGenInterruptModeration(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC ()
        } while (FALSE);
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC ()
        } while (FALSE);
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
#endif
NDIS_STATUS
N5QueryOidGenLinkParameters(
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
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    FUNC_ENTER;

    if(pBssidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return NDIS_STATUS_INVALID_DATA;
    } 

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(NDIS_LINK_PARAMETERS))
            {
                *BytesNeeded = sizeof(NDIS_LINK_PARAMETERS);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                PNDIS_LINK_PARAMETERS pLinkParams = (PNDIS_LINK_PARAMETERS)InformationBuffer;
                ULONG   rateIdx;
                ULONG   LinkSpeed;

                if (READ_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg) == MODE_CCK)
                {
                    switch (READ_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg))
                    {
                        case RATE_1:   LinkSpeed = 10000;    break;
                        case RATE_2:   LinkSpeed = 20000;    break;
                        case RATE_5_5: LinkSpeed = 55000;    break;
                        case RATE_11:  LinkSpeed = 110000;   break;
                        default:       LinkSpeed = 10000;    break;
                    }
                }
                else if (READ_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg) == MODE_OFDM)
                {
                    switch (READ_PHY_CFG_MCS(pAd, &pBssidMacTabEntry->TxPhyCfg))
                    {
                        case 0:   LinkSpeed = 60000;    break;
                        case 1:   LinkSpeed = 90000;    break;
                        case 2:   LinkSpeed = 120000;   break;
                        case 3:   LinkSpeed = 180000;   break;
                        case 4:   LinkSpeed = 240000;   break;
                        case 5:   LinkSpeed = 360000;   break;
                        case 6:   LinkSpeed = 480000;   break;
                        case 7:   LinkSpeed = 540000;   break;
                        default:  LinkSpeed = 10000;    break;
                    }
                }
                else
                {
                    // AP report MAX MCS = 15.  Station report currently used rate. So I fix AP report use MCS=15
                    if (pPort->PortSubtype == PORTSUBTYPE_STA || (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
                    {               
                        if (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_VHT)
                        {
                            rateIdx = GetVhtRateTableIdx(pAd, 
                                                 READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg), 
                                                 READ_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg), 
                                                 READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg), 
                                                 READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
                        }
                        else
                        {
                            rateIdx = 16 + ((UCHAR)READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg) * 16) + ((UCHAR)READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg) * 32) + ((UCHAR)READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
                        }
                    }
                    else
                    {
                        if (VHT_CAPABLE(pAd))
                        {
                            if (pAd->HwCfg.Antenna.field.RxPath == 2) // 2x2
                            {
                                rateIdx = GetVhtRateTableIdx(pAd, 
                                                       pPort->CommonCfg.DesiredHtPhy.HtChannelWidth, 
                                                       NSS_1, 
                                                       pPort->CommonCfg.DesiredHtPhy.ShortGIfor40, 
                                                       MCS_7); //2 TODO: Future NICs may support 256 QAM.
                            }
                            else // 1x1
                            {
                                rateIdx = GetVhtRateTableIdx(pAd, 
                                                       pPort->CommonCfg.DesiredHtPhy.HtChannelWidth, 
                                                       NSS_0, 
                                                       pPort->CommonCfg.DesiredHtPhy.ShortGIfor40, 
                                                       MCS_7); //2 TODO: Future NICs may support 256 QAM.
                            }
                        }
                        else
                        {
                            if (pAd->HwCfg.Antenna.field.TxPath == 3)
                            {
                                rateIdx = 64 + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth * 16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 * 32) + MCS_23;
                            }
                            else if (pAd->HwCfg.Antenna.field.TxPath == 2)
                            {
                                rateIdx = 16 + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth * 16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 * 32) + MCS_15;
                            }
                            else
                            {
                                rateIdx = 16 + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth * 16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 * 32) + MCS_7;
                            }
                        }
                    }

                    if (VHT_CAPABLE(pAd))
                    {
                        LinkSpeed = VhtRateTable[rateIdx] * 5000;
                    }
                    else
                    {
                        LinkSpeed = CckOfdmHtRateTable[rateIdx] * 5000;
                    }
                }
                
                PlatformZeroMemory(pLinkParams, InformationBufferLength);
                MP_ASSIGN_NDIS_OBJECT_HEADER(pLinkParams->Header, 
                                            NDIS_OBJECT_TYPE_DEFAULT,
                                            NDIS_LINK_PARAMETERS_REVISION_1,
                                            NDIS_SIZEOF_LINK_PARAMETERS_REVISION_1);
//                                          sizeof(NDIS_LINK_PARAMETERS));

                pLinkParams->MediaDuplexState = MediaDuplexStateHalf;
                pLinkParams->XmitLinkSpeed = LinkSpeed * 100;
                pLinkParams->RcvLinkSpeed = LinkSpeed * 100;
                pLinkParams->PauseFunctions = NdisPauseFunctionsUnsupported;
                pLinkParams->AutoNegotiationFlags = NDIS_LINK_STATE_DUPLEX_AUTO_NEGOTIATED;

                //
                // Exit and don't copy result into InformationBuffer
                //
                *BytesWritten = sizeof(NDIS_LINK_PARAMETERS);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_GEN_LINK_PARAMETERS \n"));
            }
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
N5QueryOidPmParameters(
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
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_PM_PARAMETERS \n"));
            // Status is always success for OID_PNP_SET_POWER
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
