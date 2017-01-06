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
    STA_oids_Set.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"
extern PCHAR DbgGetOidName(ULONG Oid);
#define ALLOWED_AUTO_CONFIG_FLAGS (DOT11_PHY_AUTO_CONFIG_ENABLED_FLAG | DOT11_MAC_AUTO_CONFIG_ENABLED_FLAG)

#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER)) 

NDIS_STATUS        
N6SetOidDot11PowerMgmtModeAutoEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("SET oid  OID_DOT11_POWER_MGMT_MODE_AUTO_ENABLED : 0x%08x %s .....\n", Oid, DbgGetOidName(Oid)));

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
        pAd->StaCfg.bAutoPowerSaveEnable = pPowerMgmtAutoMode->bEnabled;

        DBGPRINT(RT_DEBUG_ERROR, ("OID_DOT11_POWER_MGMT_MODE_AUTO_ENABLED : pPowerMgmtAutoMode->bEnabled = %d\n", pPowerMgmtAutoMode->bEnabled));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}    


NDIS_STATUS
N6SetOidDot11PowerMgmtModeStatus(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_ERROR, ("SetInformation oid  OID_DOT11_POWER_MGMT_MODE_STATUS 0x%08x %s.....shall not  set\n", Oid, DbgGetOidName(Oid)));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}
    
NDIS_STATUS
N6SetOidDot11OffloadNetworkList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    UCHAR idxChannelList = 0, idxOffloadNetworkList = 0, idxDot11ChannelHints = 0, idxBuildHintedChannel = 0;
    BOOLEAN bFound = FALSE;
    
    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Set oid  0x%08x %s .....\n", Oid, DbgGetOidName(Oid)));

    if (InformationBufferLength < FIELD_OFFSET(DOT11_OFFLOAD_NETWORK_LIST_INFO, offloadNetworkList))
    {
        *BytesNeeded = FIELD_OFFSET(DOT11_OFFLOAD_NETWORK_LIST_INFO, offloadNetworkList);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;

        DBGPRINT(RT_DEBUG_TRACE, ("ERROR: InformationBufferLength=%d \n", InformationBufferLength));

                break;
    }
    else
    {
        PDOT11_OFFLOAD_NETWORK_LIST_INFO pDot11NLOInfo = (PDOT11_OFFLOAD_NETWORK_LIST_INFO)InformationBuffer;

        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                            pDot11NLOInfo->Header,
                            NDIS_OBJECT_TYPE_DEFAULT,
                            DOT11_OFFLOAD_NETWORK_LIST_REVISION_1,
                            sizeof(DOT11_OFFLOAD_NETWORK_LIST_INFO)
                            ))
        {
            if (pDot11NLOInfo->uNumOfEntries == 0)
            {
                // No more NLO scan should occur
                // Nor should NLO discovery be indicated.
                DBGPRINT(RT_DEBUG_TRACE, ("NLO Entry is 0, disable NLO.....\n"));
                
                pAd->StaCfg.NLOEntry.NLOEnable = FALSE;
                PlatformZeroMemory(&pAd->StaCfg.NLOEntry, sizeof (NDIS_NLO_ENTRY));
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
            else
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                DBGPRINT(RT_DEBUG_TRACE, ("Invalid Ndis NLO Info.....\n"));
            }
            
                    break;
        }

                if (pDot11NLOInfo->uNumOfEntries == 0)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("NLO Entry is 0, disable NLO.....\n"));
            pAd->StaCfg.NLOEntry.NLOEnable = FALSE;
            PlatformZeroMemory(&pAd->StaCfg.NLOEntry, sizeof (NDIS_NLO_ENTRY));
        }
        else if (pDot11NLOInfo->uNumOfEntries > MAX_NUM_OF_NLO_ENTRY)
        {
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            *BytesNeeded = MAX_NUM_OF_NLO_ENTRY * sizeof(DOT11_OFFLOAD_NETWORK);
            DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_OFFLOAD_NETWORK_LIST is too large, (uNumOfEntries=%d) we only support one BSSID\n",
                                pDot11NLOInfo->uNumOfEntries));
                    break;
        }
        else
        {
            PlatformZeroMemory(&pAd->StaCfg.NLOEntry, sizeof (NDIS_NLO_ENTRY));

            // keep NLO info from NDIS
            pAd->StaCfg.NLOEntry.FastScanPeriod = (pDot11NLOInfo->FastScanPeriod == 0) ? 60 : pDot11NLOInfo->FastScanPeriod;
            pAd->StaCfg.NLOEntry.FastScanIterations = pDot11NLOInfo->FastScanIterations;
            pAd->StaCfg.NLOEntry.SlowScanPeriod = (pDot11NLOInfo->SlowScanPeriod == 0) ? 1800: pDot11NLOInfo->SlowScanPeriod;
            pAd->StaCfg.NLOEntry.ulFlags = pDot11NLOInfo->ulFlags;

            pAd->StaCfg.NLOEntry.uNumOfEntries =
                (pDot11NLOInfo->uNumOfEntries < MAX_NUM_OF_NLO_ENTRY) ? pDot11NLOInfo->uNumOfEntries :  MAX_NUM_OF_NLO_ENTRY;

            PlatformMoveMemory(pAd->StaCfg.NLOEntry.OffloadNetwork, pDot11NLOInfo->offloadNetworkList, 
                     pAd->StaCfg.NLOEntry.uNumOfEntries * sizeof(DOT11_OFFLOAD_NETWORK));

            pAd->StaCfg.NLOEntry.NLOEnable = TRUE;
            pAd->StaCfg.NLOEntry.bNLOAfterResume = TRUE;
            pAd->StaCfg.NLOEntry.OneSecPeriodicRound = 0;

            //
            // Build NLO hinted channels
            //
            PlatformZeroMemory(pAd->StaCfg.NLOEntry.NLOHintedChannels, (sizeof (CHANNEL_TX_POWER) * MAX_NUM_OF_CHANNELS));
            pAd->StaCfg.NLOEntry.NumOfHintedChannels = 0;
            for (idxOffloadNetworkList = 0; idxOffloadNetworkList < pAd->StaCfg.NLOEntry.uNumOfEntries; idxOffloadNetworkList++)
            {                       
                for (idxDot11ChannelHints = 0; idxDot11ChannelHints < DOT11_MAX_CHANNEL_HINTS; idxDot11ChannelHints++)
                {
                    if (pAd->StaCfg.NLOEntry.OffloadNetwork[idxOffloadNetworkList].Dot11ChannelHints[idxDot11ChannelHints].uChannelNumber != DOT11_INVALID_CHANNEL_NUMBER)
                    {
                        for (idxChannelList = 0; idxChannelList < pAd->HwCfg.ChannelListNum; idxChannelList++)
                        {
                            if (pAd->HwCfg.ChannelList[idxChannelList].Channel == pAd->StaCfg.NLOEntry.OffloadNetwork[idxOffloadNetworkList].Dot11ChannelHints[idxDot11ChannelHints].uChannelNumber)
                            {
                                bFound = FALSE;
                                for (idxBuildHintedChannel = 0; idxBuildHintedChannel < pAd->StaCfg.NLOEntry.NumOfHintedChannels; idxBuildHintedChannel++)
                                {
                                    if (pAd->StaCfg.NLOEntry.NLOHintedChannels[idxBuildHintedChannel].Channel == pAd->HwCfg.ChannelList[idxChannelList].Channel)
                                    {
                                        bFound = TRUE; // Ignore this channel since it is already on the hinted channels.
                                        break;
                                    }
                                }

                                if (bFound == FALSE)
                                {
                                    PlatformMoveMemory(&pAd->StaCfg.NLOEntry.NLOHintedChannels[pAd->StaCfg.NLOEntry.NumOfHintedChannels], 
                                        &pAd->HwCfg.ChannelList[idxChannelList], 
                                        sizeof(CHANNEL_TX_POWER));
                                    ++pAd->StaCfg.NLOEntry.NumOfHintedChannels;
                                }
                            }
                        }
                    }
                }
            }


#ifdef DBG
            for (idxOffloadNetworkList = 0; idxOffloadNetworkList < pAd->StaCfg.NLOEntry.uNumOfEntries; idxOffloadNetworkList++)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s: OID_DOT11_OFFLOAD_NETWORK_LIST: SSID = %c%c%c%c%c%c..., Dot11ChannelHints.uChannelNumber = %d, %d, %d, %d\n", 
                    __FUNCTION__, 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Ssid.ucSSID[0], 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Ssid.ucSSID[1], 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Ssid.ucSSID[2], 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Ssid.ucSSID[3], 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Ssid.ucSSID[4], 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Ssid.ucSSID[5], 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Dot11ChannelHints[0].uChannelNumber, 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Dot11ChannelHints[1].uChannelNumber, 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Dot11ChannelHints[2].uChannelNumber, 
                    pDot11NLOInfo->offloadNetworkList[idxOffloadNetworkList].Dot11ChannelHints[3].uChannelNumber));
            }

            DBGPRINT(RT_DEBUG_TRACE, ("%s: OID_DOT11_OFFLOAD_NETWORK_LIST: Build NLO hinted channels, NumOfHintedChannels = %d\n", 
                __FUNCTION__, 
                pAd->StaCfg.NLOEntry.NumOfHintedChannels));

            for (idxChannelList = 0; idxChannelList < pAd->StaCfg.NLOEntry.NumOfHintedChannels; idxChannelList++)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s: OID_DOT11_OFFLOAD_NETWORK_LIST: Build NLO hinted channels, channel %d\n", 
                    __FUNCTION__, 
                    pAd->StaCfg.NLOEntry.NLOHintedChannels[idxChannelList].Channel));
            }
#endif // DBG

            switch (pAd->StaCfg.NLOEntry.ulFlags)
            {
                case DOT11_NLO_FLAG_STOP_NLO_INDICATION: 
                {
                    // No more NLO scan should occur
                    // Nor should NLO discovery be indicated.
                    pAd->StaCfg.NLOEntry.NLOEnable = FALSE;
                }
                break;

                case DOT11_NLO_FLAG_SCAN_ON_AOAC_PLATFORM: 
                {
                    // NLO should scan immediately and the schedule of fast scan and slow scan should be followed.
                    pAd->StaCfg.NLOEntry.NLOEnable = TRUE;
                }
                break;

                case DOT11_NLO_FLAG_SCAN_AT_SYSTEM_RESUME: 
                {
                    // When the system resumes, the miniport/hardware tries to connect to the previous connected network.
                    // The lists should be scan right after the effort failed or when there is no previous connected AP to reconnect to.
                    pAd->StaCfg.NLOEntry.NLOEnable = TRUE;
                }
                break;

                default: 
                {
                    // do nothing
                }
            }

            DBGPRINT(RT_DEBUG_TRACE, ("Set:: Enable NLO..... FastScaPeriod=%d, Iteration=%d, SlowScan=%d, NLO Entry is %d, ulFlags = 0x%08X\n", 
                pAd->StaCfg.NLOEntry.FastScanPeriod, pAd->StaCfg.NLOEntry.FastScanIterations, 
                pAd->StaCfg.NLOEntry.SlowScanPeriod, pAd->StaCfg.NLOEntry.uNumOfEntries, 
                pAd->StaCfg.NLOEntry.ulFlags));
                }
            *BytesRead =  FIELD_OFFSET(DOT11_OFFLOAD_NETWORK_LIST_INFO, offloadNetworkList) 
                + pAd->StaCfg.NLOEntry.uNumOfEntries * sizeof(DOT11_OFFLOAD_NETWORK);

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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}  

#endif // COMPILE_WIN8_ABOVE


NDIS_STATUS
N6SetOidPmRemoveWolPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}       

NDIS_STATUS
N6SetOidGenInterruptModeration(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}    

NDIS_STATUS
N6SetOidPmRemoveProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
} 

NDIS_STATUS
N6SetOidPmAddProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}               

NDIS_STATUS
N6SetOidPmAddWolPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
} 

NDIS_STATUS
N6SetOidPmGetProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}        

NDIS_STATUS
N6SetOidDot11EnumBssList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType           = pPort->PortType;
    NDIS_OID         Oid                = NdisRequest->DATA.METHOD_INFORMATION.Oid;
    PVOID            InformationBuffer  = NdisRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    ULONG            InputBufferLength  = NdisRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    ULONG            OutputBufferLength = NdisRequest->DATA.METHOD_INFORMATION.OutputBufferLength;
    PULONG           BytesRead          = (PULONG)&NdisRequest->DATA.METHOD_INFORMATION.BytesRead;
    PULONG           BytesWritten       = (PULONG)&NdisRequest->DATA.METHOD_INFORMATION.BytesWritten;
    PULONG           BytesNeeded        = (PULONG)&NdisRequest->DATA.METHOD_INFORMATION.BytesNeeded;

    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    *BytesWritten = 0;
    *BytesRead = 0;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        PDOT11_BYTE_ARRAY pDot11ByteArray = NULL;
        
        DBGPRINT(RT_DEBUG_TRACE, ("Query:OID_DOT11_ENUM_BSS_LIST\n"));

        //
        // Check enough space for the 3 fields of the DOT11_BYTE_ARRAY
        //
        if (OutputBufferLength < FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer))
        {
            *BytesNeeded = sizeof(DOT11_BYTE_ARRAY);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;

            FUNC_LEAVE_STATUS(ndisStatus);
            return ndisStatus;
        }

        ndisStatus = NdisInitGetBssList(pAd, InformationBuffer, OutputBufferLength);

        pDot11ByteArray = (PDOT11_BYTE_ARRAY)InformationBuffer;
        
        if ( ndisStatus == NDIS_STATUS_BUFFER_OVERFLOW)
            {
            *BytesWritten = 0;
            *BytesNeeded = pDot11ByteArray->uTotalNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
            }
            else
            {
            *BytesWritten = pDot11ByteArray->uNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
            *BytesNeeded = 0;
            }
        DBGPRINT(RT_DEBUG_TRACE, ("Query:OID_DOT11_ENUM_BSS_LIST (pDot11ByteArray->uTotalNumOfBytes = %d)\n", pDot11ByteArray->uTotalNumOfBytes));
        DBGPRINT(RT_DEBUG_TRACE, ("Query:OID_DOT11_ENUM_BSS_LIST (pDot11ByteArray->uNumOfBytes = %d)\n", pDot11ByteArray->uNumOfBytes));
        DBGPRINT(RT_DEBUG_TRACE, ("Query:OID_DOT11_ENUM_BSS_LIST (Total BSS=%d, BytesNeed=%d,BytesWritten= %d,BufferLen=%d,Status=%x)\n", pAd->ScanTab.BssNr, *BytesNeeded, *BytesWritten,OutputBufferLength,ndisStatus));

        if (pPort->PortNumber == PORT_0)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_ENUM_BSS_LIST - Record OS time.\n"));
            NdisGetCurrentSystemTime((PLARGE_INTEGER)&pPort->P2PCfg.LastEnumBssTime);
        }
    }
    else
    {
        UCHAR               i,j;
        PDOT11_BYTE_ARRAY   pDot11ByteArray = NULL;
        ULONG               dwTimeExpired;
        ULONG               dwDifferTime;
        ULONG               dwOverflow = 0xffffffff;
        UCHAR               uChannel;
        BOOLEAN             bChannelLegal;
        LARGE_INTEGER       currTime;
        BOOLEAN             bFound;
        ULONG               BSSEntrySize = 0;
        ULONG               RemainingBytes;
        ULONG               IETotalSize = 0;
        PBSS_ENTRY          pBss = NULL;
        PUCHAR              pCurrPtr;
        PDOT11_BSS_ENTRY    pDot11BSSEntry = NULL;
        UCHAR               TempBssNr = 0;
        
        pDot11ByteArray = (PDOT11_BYTE_ARRAY)InformationBuffer;

        //
        // Check enough space for the 3 fields of the DOT11_BYTE_ARRAY
        //
        if (OutputBufferLength < FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer))
        {
            *BytesWritten = 0;
            *BytesNeeded = sizeof(DOT11_BYTE_ARRAY);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
        }

        dwTimeExpired =  (pPort->CommonCfg.BssidRepositTime)* 10000000;
        DBGPRINT(RT_DEBUG_INFO,("dwTimeExpired is %d\n", dwTimeExpired));

        PlatformZeroMemory(pDot11ByteArray, OutputBufferLength);

        //
        // For Reposit bssid list to calculate how many Bssid should be added to 
        //

        // Get current system time as current time stamp
        NdisGetCurrentSystemTime(&currTime);

        // Reset the temporal Reposit Bss Table
        if(pAd->pNicCfg->RepositBssTable.pBssEntry != NULL && pAd->pNicCfg->TempRepositBssTable.pBssEntry != NULL)
    {
            // Reset the temporal Reposit Bss Table
            for(i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
            {
                PlatformZeroMemory(&pAd->pNicCfg->TempRepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
                pAd->pNicCfg->TempRepositBssTable.ulTime[i].LowPart = 0;
                pAd->pNicCfg->TempRepositBssTable.ulTime[i].HighPart = 0;
    }

            // Copy Current Scan Table to temporal Reposit Bss Table
            PlatformMoveMemory(pAd->pNicCfg->TempRepositBssTable.pBssEntry, pAd->ScanTab.BssEntry, pAd->ScanTab.BssNr*sizeof(BSS_ENTRY));
            for(i = 0; i < pAd->ScanTab.BssNr; i++)
    {
                pAd->pNicCfg->TempRepositBssTable.ulTime[i] = currTime;
            }

            TempBssNr = pAd->ScanTab.BssNr;

            pAd->pNicCfg->TempRepositBssTable.BssNr = TempBssNr;
            
            // Compare and Merge the ScanTab result with the Reposit Bss Table to temporal Reposit Bss Table

            for(i = 0; i < pAd->pNicCfg->RepositBssTable.BssNr; i++)
        {
                if(i > MAX_LEN_OF_BSS_TABLE || TempBssNr >= MAX_LEN_OF_BSS_TABLE)
            {
                    break;
            }
            
                bFound = FALSE;
            
                for(j = 0; j <pAd->ScanTab.BssNr; j++)
            {
                    if(PlatformEqualMemory(pAd->ScanTab.BssEntry[j].Bssid, pAd->pNicCfg->RepositBssTable.pBssEntry[i].Bssid, 6) == 1)
                    {
                        //for wps issue ,we need to make sure probe response WPS IE also exist in table.
                        if (pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtProbeRspLen > 0 && 
                            (pAd->pNicCfg->TempRepositBssTable.pBssEntry[j].WSCInfoAtProbeRspLen == 0))
                        {
                            pAd->pNicCfg->TempRepositBssTable.pBssEntry[j].WSCInfoAtProbeRspLen =   pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtProbeRspLen;
                            PlatformMoveMemory(&pAd->pNicCfg->TempRepositBssTable.pBssEntry[j].WSCInfoAtProbeRsp,&pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtProbeRsp,
                                        pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtProbeRspLen);

                            DBGPRINT(RT_DEBUG_INFO,("Ssid <<%s>> with psb\n", pAd->ScanTab.BssEntry[i].Ssid));
            }
                        bFound = TRUE;
                        break;
        }
    }


                if(!bFound)
                {
                    DBGPRINT(RT_DEBUG_INFO,("Not Found Ssid <<%s>>\n", pAd->pNicCfg->RepositBssTable.pBssEntry[i].Ssid));

                    if(currTime.LowPart > pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart)
                    {
                        dwDifferTime = currTime.LowPart - pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart;
                    }
                    else
                    {
                        dwDifferTime = dwOverflow - pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart + currTime.LowPart;
                    }

                    if(dwDifferTime < dwTimeExpired )
    {
                        // Check if channel is legal or not ??
                        bChannelLegal = FALSE;
                        uChannel = 0;

                        uChannel = pAd->pNicCfg->RepositBssTable.pBssEntry[i].Channel;
                        DBGPRINT(RT_DEBUG_INFO, ("Channel is = %d \n", uChannel));
                        
                        for(j = 0; j < pAd->HwCfg.ChannelListNum; j++)
        {
                            if(uChannel == pAd->HwCfg.ChannelList[j].Channel)
            {
                                bChannelLegal = TRUE;
                                DBGPRINT(RT_DEBUG_INFO, ("Channel is legal \n"));
                                break;
            }
                        }
                        
                        if(bChannelLegal)
            {
                            // for legal channel we add this bss entry to the temporal reposit bssid list
                            PlatformMoveMemory(&pAd->pNicCfg->TempRepositBssTable.pBssEntry[TempBssNr], &pAd->pNicCfg->RepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
                            pAd->pNicCfg->TempRepositBssTable.ulTime[TempBssNr] = pAd->pNicCfg->RepositBssTable.ulTime[i];
                            TempBssNr++;
                            pAd->pNicCfg->TempRepositBssTable.BssNr++;
                        }
            }
        }
    }

            // Copy the temporal reposit bss table to the Reposit bss table
            // 1.) clear Reposit Bss Table first
            // Reset the temporal Reposit Bss Table
            DBGPRINT(RT_DEBUG_INFO, ("pAd->RepositBssidList.ScanTab initialize successfully\n"));

            for(i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
            {
                PlatformZeroMemory(&pAd->pNicCfg->RepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
                pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart = 0;
                pAd->pNicCfg->RepositBssTable.ulTime[i].HighPart = 0;
            }

            // 2.) copy the temporal Reposit Bss Table to Reposit Bss Table
            for(i = 0; i < pAd->pNicCfg->TempRepositBssTable.BssNr; i++)
            {
                PlatformMoveMemory(&pAd->pNicCfg->RepositBssTable.pBssEntry[i], &pAd->pNicCfg->TempRepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
                pAd->pNicCfg->RepositBssTable.ulTime[i] = pAd->pNicCfg->TempRepositBssTable.ulTime[i];
            }
            pAd->pNicCfg->RepositBssTable.BssNr = pAd->pNicCfg->TempRepositBssTable.BssNr;
            DBGPRINT(RT_DEBUG_INFO, ("Bssid list original count = %d, merged count = %d\n",pAd->ScanTab.BssNr, pAd->pNicCfg->RepositBssTable.BssNr));
            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11ByteArray->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_BSS_ENTRY_BYTE_ARRAY_REVISION_1,
                                        sizeof(DOT11_BYTE_ARRAY));

            RemainingBytes = OutputBufferLength - FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
            pCurrPtr = pDot11ByteArray->ucBuffer;
            for (i = 0; i < pAd->pNicCfg->RepositBssTable.BssNr; i++)
    {
                pBss = &pAd->pNicCfg->RepositBssTable.pBssEntry[i];

                //because beacon do not include hidden ssid length ,call StaGetEnumBSSIELength() 
                //to get the actual IE length include hidden ssid len
                
                BSSEntrySize = FIELD_OFFSET(DOT11_BSS_ENTRY, ucBuffer)  + pBss->SsidLen + sizeof(DOT11_INFO_ELEMENT)  
                                    + pBss->VarIELen + pBss->WSCInfoAtBeaconsLen + pBss->WSCInfoAtProbeRspLen;              
                                
                pDot11ByteArray->uTotalNumOfBytes += BSSEntrySize;              
                if (RemainingBytes >= BSSEntrySize)
                {
                    pDot11BSSEntry = (PDOT11_BSS_ENTRY)pCurrPtr;

                    pDot11BSSEntry->uPhyId = MlmeInfoGetPhyIdByChannel(pAd, pBss->Channel);
                    pDot11BSSEntry->usBeaconPeriod = pBss->BeaconPeriod;
                    pDot11BSSEntry->ullTimestamp = pBss->BeaconTimestamp;
                    pDot11BSSEntry->ullHostTimestamp = pBss->HostTimestamp;
                    pDot11BSSEntry->dot11BSSType = pBss->BssType;
                    pDot11BSSEntry->usCapabilityInformation = pBss->CapabilityInfo;
                    pDot11BSSEntry->lRSSI = pBss->Rssi;

                    if ((pAd->HwCfg.BLNAGain != 0xFF) && (pAd->HwCfg.Antenna.field.RssiIndicationMode == USER_AWARENESS_MODE))
                    {
                        pDot11BSSEntry->lRSSI = pDot11BSSEntry->lRSSI + GET_LNA_GAIN(pAd); // RSSI indication by Rx LNA output

                        DBGPRINT(RT_DEBUG_TRACE, ("RTMPApCopyBssidList, RSSI, MacAddress = %02X:%02X:%02X:%02X:%02X:%02X, pDot11BSSEntry->lRSSI = %d, pAd->HwCfg.LatchRfRegs.Channel = %d\n", 
                            pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], 
                            pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5], 
                            pDot11BSSEntry->lRSSI, 
                            pAd->HwCfg.LatchRfRegs.Channel));
    }

                    pDot11BSSEntry->uLinkQuality = MlmeInfoGetLinkQuality(pAd, pDot11BSSEntry->lRSSI,FALSE);

                    MAP_CHANNEL_ID_TO_KHZ(pBss->Channel, pDot11BSSEntry->PhySpecificInfo.uChCenterFrequency);
                    pDot11BSSEntry->PhySpecificInfo.uChCenterFrequency /= 1000;

                    pDot11BSSEntry->bInRegDomain = pBss->bInRegDomain;

                    COPY_MAC_ADDR(pDot11BSSEntry->dot11BSSID, pBss->Bssid);

                    BSSEntrySize = FIELD_OFFSET(DOT11_BSS_ENTRY, ucBuffer);
                    pDot11ByteArray->uNumOfBytes += BSSEntrySize;
                    pCurrPtr+= BSSEntrySize;
                    RemainingBytes -= BSSEntrySize;                         

                    //consider hidden ssid case ,so we have to make sure SSID IE element
                    //copy content in the pBss->VarIE buffer
                    //
                    pCurrPtr[0] = DOT11_INFO_ELEMENT_ID_SSID;
                    pCurrPtr[1]  = pBss->SsidLen;
                    pCurrPtr += 2;

                    PlatformMoveMemory(pCurrPtr, pBss->Ssid, pBss->SsidLen);    
                    pCurrPtr+= pBss->SsidLen;
                    IETotalSize += ( pBss->SsidLen + 2) ;
                    
                    PlatformMoveMemory(pCurrPtr, pBss->VarIEs , pBss->VarIELen);
                    IETotalSize += pBss->VarIELen;                                      
                    pCurrPtr += pBss->VarIELen;                                     

                    if(pBss->WSCInfoAtProbeRspLen >= pBss->WSCInfoAtBeaconsLen)
    {
                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtProbeRsp, pBss->WSCInfoAtProbeRspLen);
                        IETotalSize += pBss->WSCInfoAtProbeRspLen;
                        pCurrPtr += pBss->WSCInfoAtProbeRspLen;     
                        
                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtBeacons, pBss->WSCInfoAtBeaconsLen);
                        IETotalSize += pBss->WSCInfoAtBeaconsLen;
                        pCurrPtr += pBss->WSCInfoAtBeaconsLen;                          
                    }
                    else
        {
                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtBeacons, pBss->WSCInfoAtBeaconsLen);
                        IETotalSize += pBss->WSCInfoAtBeaconsLen;
                        pCurrPtr += pBss->WSCInfoAtBeaconsLen;      
                        
                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtProbeRsp, pBss->WSCInfoAtProbeRspLen);
                        IETotalSize += pBss->WSCInfoAtProbeRspLen;
                        pCurrPtr += pBss->WSCInfoAtProbeRspLen;     
                    }
                    
#if DBG
                    if(pBss->WSCInfoAtProbeRspLen > 0)
            {
                        DBGPRINT(RT_DEBUG_INFO, (" rep WPS ssid= %s,wscbelen=%d,wscpsb=%d\n",  pBss->Ssid, pBss->WSCInfoAtBeaconsLen ,pBss->WSCInfoAtProbeRspLen));     
                        DBGPRINT(RT_DEBUG_INFO, (" %2x:%2x:%2x:%2x:%2x:%2x\n",pBss->WSCInfoAtProbeRsp[0],pBss->WSCInfoAtProbeRsp[1],pBss->WSCInfoAtProbeRsp[2],
                                pBss->WSCInfoAtProbeRsp[3],pBss->WSCInfoAtProbeRsp[4],pBss->WSCInfoAtProbeRsp[5]));
                    }
#endif
                    pDot11BSSEntry->uBufferLength = IETotalSize;
                    pDot11ByteArray->uNumOfBytes += IETotalSize;
                    RemainingBytes -= IETotalSize;      

            }
            else
            {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    RemainingBytes = 0;
            }
                IETotalSize = 0;
        }
    }
        else
        {

            MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11ByteArray->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_BSS_ENTRY_BYTE_ARRAY_REVISION_1,
                                        sizeof(DOT11_BYTE_ARRAY));

            RemainingBytes = OutputBufferLength - FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
            pCurrPtr = pDot11ByteArray->ucBuffer;
            for (i = 0; i < pAd->ScanTab.BssNr; i++)
            {
                pBss = &pAd->ScanTab.BssEntry[i];

                //because beacon do not include hidden ssid length ,call StaGetEnumBSSIELength() 
                //to get the actual IE length include hidden ssid len

                BSSEntrySize = FIELD_OFFSET(DOT11_BSS_ENTRY, ucBuffer)  + pBss->SsidLen + sizeof(DOT11_INFO_ELEMENT)  
                                    + pBss->VarIELen + pBss->WSCInfoAtBeaconsLen + pBss->WSCInfoAtProbeRspLen;          
                                
                pDot11ByteArray->uTotalNumOfBytes += BSSEntrySize;              
                if (RemainingBytes >= BSSEntrySize)
    {
                    pDot11BSSEntry = (PDOT11_BSS_ENTRY)pCurrPtr;

                    pDot11BSSEntry->uPhyId = MlmeInfoGetPhyIdByChannel(pAd, pBss->Channel);
                    pDot11BSSEntry->usBeaconPeriod = pBss->BeaconPeriod;
                    pDot11BSSEntry->ullTimestamp = pBss->BeaconTimestamp;
                    pDot11BSSEntry->ullHostTimestamp = pBss->HostTimestamp;
                    pDot11BSSEntry->dot11BSSType = pBss->BssType;
                    pDot11BSSEntry->usCapabilityInformation = pBss->CapabilityInfo;
                    pDot11BSSEntry->lRSSI = pBss->Rssi;

                    if ((pAd->HwCfg.BLNAGain != 0xFF) && (pAd->HwCfg.Antenna.field.RssiIndicationMode == USER_AWARENESS_MODE))
                    {
                        pDot11BSSEntry->lRSSI = pDot11BSSEntry->lRSSI + GET_LNA_GAIN(pAd); // RSSI indication by Rx LNA output

                        DBGPRINT(RT_DEBUG_TRACE, ("RTMPApCopyBssidList, RSSI, MacAddress = %02X:%02X:%02X:%02X:%02X:%02X, pDot11BSSEntry->lRSSI = %d, pAd->HwCfg.LatchRfRegs.Channel = %d\n", 
                            pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], 
                            pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5], 
                            pDot11BSSEntry->lRSSI, 
                            pAd->HwCfg.LatchRfRegs.Channel));
    }

                    pDot11BSSEntry->uLinkQuality = MlmeInfoGetLinkQuality(pAd, pDot11BSSEntry->lRSSI,FALSE);

                    MAP_CHANNEL_ID_TO_KHZ(pBss->Channel, pDot11BSSEntry->PhySpecificInfo.uChCenterFrequency);
                    pDot11BSSEntry->PhySpecificInfo.uChCenterFrequency /= 1000;

                    pDot11BSSEntry->bInRegDomain = pBss->bInRegDomain;

                    COPY_MAC_ADDR(pDot11BSSEntry->dot11BSSID, pBss->Bssid);

                    BSSEntrySize = FIELD_OFFSET(DOT11_BSS_ENTRY, ucBuffer);
                    pDot11ByteArray->uNumOfBytes += BSSEntrySize;
                    pCurrPtr+= BSSEntrySize;
                    RemainingBytes -= BSSEntrySize;                         

                    //consider hidden ssid case ,so we have to make sure SSID IE element
                    //copy content in the pBss->VarIE buffer
                    //
                    pCurrPtr[0] = DOT11_INFO_ELEMENT_ID_SSID;
                    pCurrPtr[1]  = pBss->SsidLen;
                    pCurrPtr += 2;

                    PlatformMoveMemory(pCurrPtr, pBss->Ssid, pBss->SsidLen);    
                    pCurrPtr+= pBss->SsidLen;
                    IETotalSize += ( pBss->SsidLen + 2) ;
                    
                    PlatformMoveMemory(pCurrPtr, pBss->VarIEs , pBss->VarIELen);
                    IETotalSize += pBss->VarIELen;              
                    pCurrPtr += pBss->VarIELen;                 

                    if(pBss->WSCInfoAtProbeRspLen >= pBss->WSCInfoAtBeaconsLen)
    {
                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtProbeRsp, pBss->WSCInfoAtProbeRspLen);
                        IETotalSize += pBss->WSCInfoAtProbeRspLen;
                        pCurrPtr += pBss->WSCInfoAtProbeRspLen;
                        
                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtBeacons, pBss->WSCInfoAtBeaconsLen);
                        IETotalSize += pBss->WSCInfoAtBeaconsLen;
                        pCurrPtr += pBss->WSCInfoAtBeaconsLen;      
                    }
                    else
        {
                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtBeacons, pBss->WSCInfoAtBeaconsLen);
                        IETotalSize += pBss->WSCInfoAtBeaconsLen;
                        pCurrPtr += pBss->WSCInfoAtBeaconsLen;

                        PlatformMoveMemory(pCurrPtr, pBss->WSCInfoAtProbeRsp, pBss->WSCInfoAtProbeRspLen);
                        IETotalSize += pBss->WSCInfoAtProbeRspLen;
                        pCurrPtr += pBss->WSCInfoAtProbeRspLen;
                    }
                    
#if DBG
                    if(pBss->WSCInfoAtBeaconsLen > 0)
            {
                        DBGPRINT(RT_DEBUG_INFO, (" WPS ssid= %s,wscbelen=%d,wscpsb=%d\n",  pBss->Ssid, pBss->WSCInfoAtBeaconsLen ,pBss->WSCInfoAtProbeRspLen));     
                    }
#endif
                    pDot11BSSEntry->uBufferLength = IETotalSize;
                    pDot11ByteArray->uNumOfBytes += IETotalSize;
                    RemainingBytes -= IETotalSize;      

            }
            else
            {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    RemainingBytes = 0;
            }
                IETotalSize = 0;
            }
        }
        
        
        *BytesWritten = pDot11ByteArray->uNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
        if ( ndisStatus == NDIS_STATUS_BUFFER_OVERFLOW)
        {
            *BytesNeeded = pDot11ByteArray->uTotalNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer)                 
                            + (BSSEntrySize - RemainingBytes);
        }
        else
        {
            *BytesNeeded = pDot11ByteArray->uTotalNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
    }
        DBGPRINT(RT_DEBUG_TRACE, ("AP Query:OID_DOT11_ENUM_BSS_LIST (Total BSS=%d, BytesNeed=%d,BytesWritten= %d,BufferLen=%d,Status=%x)\n", pAd->ScanTab.BssNr, *BytesNeeded, *BytesWritten,OutputBufferLength,ndisStatus));
    }// EXTAP_PORT

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}               

NDIS_STATUS
N6SetOidDot11NicPowerState(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    //SDV: irql_miscellaneous_function
    //
    ndisStatus = Ndis6CommonPortSetNicPowerState(pAd, pPort, NdisRequest);

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}    

NDIS_STATUS
N6SetOidDot11ResetRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType           = pPort->PortType;
    NDIS_OID         Oid                = NdisRequest->DATA.METHOD_INFORMATION.Oid;
    PVOID            InformationBuffer  = NdisRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    ULONG            InputBufferLength  = NdisRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    ULONG            OutputBufferLength = NdisRequest->DATA.METHOD_INFORMATION.OutputBufferLength;
    PULONG           BytesRead          = (PULONG)&NdisRequest->DATA.METHOD_INFORMATION.BytesRead;
    PULONG           BytesWritten       = (PULONG)&NdisRequest->DATA.METHOD_INFORMATION.BytesWritten;
    PULONG           BytesNeeded        = (PULONG)&NdisRequest->DATA.METHOD_INFORMATION.BytesNeeded;

    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    *BytesWritten = 0;
    *BytesRead = 0;
    
    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
#ifdef MULTI_CHANNEL_SUPPORT
        if (((PortType == EXTSTA_PORT) || (PortType == WFD_CLIENT_PORT))  && (NdisRequest->DATA.METHOD_INFORMATION.Oid == OID_DOT11_RESET_REQUEST))
        {
            pAd->MccCfg.PendedResetRequest = NdisRequest;
            DBGPRINT(RT_DEBUG_TRACE,("%s(%d) - Record STA/CLI OID_DOT11_RESET_REQUEST(PortNum=%d)\n", __FUNCTION__, __LINE__, NdisRequest->PortNumber));
        }
        else if ((PortType == WFD_GO_PORT)  && (NdisRequest->DATA.METHOD_INFORMATION.Oid == OID_DOT11_RESET_REQUEST))
        {
            pAd->MccCfg.PendedGoOidRequest = NdisRequest;
            DBGPRINT(RT_DEBUG_TRACE,("%s(%d) - Record GO OID_DOT11_RESET_REQUEST(PortNum=%d)\n", __FUNCTION__, __LINE__, NdisRequest->PortNumber));                     
    }

        if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pPort->PortType == WFD_GO_PORT))
    {
            // Save disconnect port
            pAd->MccCfg.pGoPort = pPort;

            // Set the flag
            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_PRE_STOP);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Set MULTI_CHANNEL_GO_PRE_STOP flag\n", __FUNCTION__));

            // Pending this OID
            ndisStatus = NDIS_STATUS_PENDING;
        }
        else if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pPort->PortType == WFD_CLIENT_PORT))
        {
            // Save reset request  port
            pAd->MccCfg.pResetRequestPort = pPort;

            // Set the flag
            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_RESET_REQ);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Set MULTI_CHANNEL_CLI_PRE_RESET_REQ flag\n", __FUNCTION__));

            // Pending this OID
            ndisStatus = NDIS_STATUS_PENDING;
        }
        else if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pPort->PortType == EXTSTA_PORT))
            {
            // Save reset request  port
            pAd->MccCfg.pResetRequestPort = pPort;

            // Set the flag
            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_RESET_REQ);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Set MULTI_CHANNEL_STA_PRE_RESET_REQ flag\n", __FUNCTION__));

            // Pending this OID
            ndisStatus = NDIS_STATUS_PENDING;
            }
            else
#endif /*MULTI_CHANNEL_SUPPORT*/                
            {
            ndisStatus = RTMPInfoResetRequest(
                                    pAd,
                                    pPort,
                                    InformationBuffer,
                                    InputBufferLength,
                                    OutputBufferLength,
                                    BytesRead,
                                    BytesWritten,
                                    BytesNeeded
                                    );  
        }

    }
    else // EXTAP_PORT
    {
        ndisStatus = ApInfoResetRequest(
                            pAd,
                            PortNumber,
                            InformationBuffer,
                            InputBufferLength,
                            OutputBufferLength,
                            BytesRead,
                            BytesWritten,
                            BytesNeeded);
    } // EXTAP_PORT

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}        

NDIS_STATUS
N6SetOidDot11DesiredPhyList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        if (!MP_VERIFY_STATE(pPort, INIT_STATE))
        {
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_DESIRED_PHY_LIST failed on Invalid state\n"));

            FUNC_LEAVE_STATUS(ndisStatus);
            return ndisStatus;
    }

        if (InformationBufferLength < sizeof(DOT11_PHY_ID_LIST))
    {
            *BytesNeeded = sizeof(DOT11_PHY_ID_LIST);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            
            FUNC_LEAVE_STATUS(ndisStatus);
            return ndisStatus;
        }
        else
        {
            ULONG                     index = 0;
            DOT11_SUPPORTED_PHY_TYPES SupportedPhyTypes;
            BOOLEAN                   AnyPhyId = FALSE;
            PDOT11_PHY_ID_LIST        PhyIdList = (PDOT11_PHY_ID_LIST)InformationBuffer;

            if (PhyIdList->dot11PhyId[0] >= pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries) //Why AP use this for check PHY ID?
            {
                //
                // Invalid PHY ID
                //
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                
                FUNC_LEAVE_STATUS(ndisStatus);
                return ndisStatus;
            }

            if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(PhyIdList->Header, 
                                                    NDIS_OBJECT_TYPE_DEFAULT,
                                                    DOT11_PHY_ID_LIST_REVISION_1,
                                                    sizeof(DOT11_PHY_ID_LIST)))
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;

                FUNC_LEAVE_STATUS(ndisStatus);
                return ndisStatus;
            }

            // Must have atleast one entry in the list
            if (PhyIdList->uNumOfEntries < 1)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                
                FUNC_LEAVE_STATUS(ndisStatus);
                return ndisStatus;
        }
            
            *BytesNeeded = PhyIdList->uNumOfEntries * sizeof(ULONG) +
                            FIELD_OFFSET(DOT11_PHY_ID_LIST, dot11PhyId);

            if (InformationBufferLength < *BytesNeeded)
            {
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    
                FUNC_LEAVE_STATUS(ndisStatus);
                return ndisStatus;
    }

            // If the list is too long or too short, simply return error.
            if (PhyIdList->uNumOfEntries > STA_DESIRED_PHY_MAX_COUNT || PhyIdList->uNumOfEntries < 1)
            {
                *BytesRead = FIELD_OFFSET(DOT11_PHY_ID_LIST, dot11PhyId);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
            }

            *BytesRead = FIELD_OFFSET(DOT11_PHY_ID_LIST, dot11PhyId) + PhyIdList->uNumOfEntries * sizeof(ULONG);

            //
            // Make sure we support all the PHY IDs in the list.
            //
            Ndis6CommonQuerySupportedPHYTypes(pAd, 0, &SupportedPhyTypes);
            for (index = 0; index < PhyIdList->uNumOfEntries; index++)
            {
                if (PhyIdList->dot11PhyId[index] == DOT11_PHY_ID_ANY)
                {
                    AnyPhyId = TRUE;
                }
                else if (PhyIdList->dot11PhyId[index] >= SupportedPhyTypes.uTotalNumOfEntries)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    
                    FUNC_LEAVE_STATUS(ndisStatus);
                    return ndisStatus;
                }
            }

            //
            // Copy the desired PHY list.
            //
            if (AnyPhyId)
            {
                pAd->StaCfg.DesiredPhyCount = 1;
                pAd->StaCfg.DesiredPhyList[0] = DOT11_PHY_ID_ANY; 
            }
            else
            {
                pAd->StaCfg.DesiredPhyCount = PhyIdList->uNumOfEntries;
                PlatformMoveMemory(pAd->StaCfg.DesiredPhyList, 
                                PhyIdList->dot11PhyId,
                                pAd->StaCfg.DesiredPhyCount * sizeof(ULONG));
            }

            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_DESIRED_PHY_LIST\n"));

            ndisStatus = Ndis6CommonSetDesiredPhyIdList(pAd, pPort, pAd->StaCfg.DesiredPhyList, pAd->StaCfg.DesiredPhyCount);
        }
    }
    else // EXTAP_PORT
    {
        ndisStatus = ApOidSetDesiredPhyList(pAd, PortNumber, InformationBuffer, InformationBufferLength, BytesRead, BytesNeeded);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}       

NDIS_STATUS
N6SetOidDot11ScanRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))              
#ifdef MULTI_CHANNEL_SUPPORT
        if ((PortType == EXTSTA_PORT) && (NdisRequest->DATA.SET_INFORMATION.Oid == OID_DOT11_SCAN_REQUEST))
        {
            pAd->pNicCfg->PendedScanRequest = NdisRequest;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) - Record OID_DOT11_SCAN_REQUEST(PortNum=%d)\n", __FUNCTION__, __LINE__, NdisRequest->PortNumber));        
    }
#endif // MULTI_CHANNEL_SUPPORT
#endif // COMPILE_WIN8_ABOVE

        do
        {
#if 1           
    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST, AP mode skip this request\n"));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
            else if (pPort->PortCfg.OperationMode == DOT11_OPERATION_MODE_NETWORK_MONITOR)
        {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST,Monitor mode skip this request\n"));
                //in network monitor mode ,do not allow scan operation
                ndisStatus = NDIS_STATUS_DOT11_MEDIA_IN_USE;
                break;
            }
            else if (pPort->PortNumber != NDIS_DEFAULT_PORT_NUMBER)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("From non-ndis-default Port (%d), ignore OID_DOT11_SCAN_REQUEST...\n", pPort->PortNumber));
                PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
                DBGPRINT(RT_DEBUG_TRACE, ("indicate scan confirm ...\n"));
                break;
            }
            else if ((pAd->pHifCfg->BulkLastOneSecCount > 30) && (!pAd->LogoTestCfg.OnTestingWHQL))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST, Heavy traffic on TX/RX ignore this request\n"));
                ndisStatus = NDIS_STATUS_DOT11_MEDIA_IN_USE;
                break;
            }
            else if(P2PIsScanAllowed(pAd, pPort) == FALSE)
            {
                if (pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE_DONE)
                    pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_ENROLLEE;

                PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST, P2PIsScanAllowed(pAd) == FALSE\n"));
                break;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS = %d, pAd->MlmeAux.bNeedPlatformIndicateScanStatus = %d\n", MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS), pAd->MlmeAux.bNeedPlatformIndicateScanStatus));
            if ((pPort->PortType == EXTAP_PORT) || MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {
                // If driver is auto scanning but not from OID ScanRequest
                // Set NeedPlatformIndicateScanStatus to confirm scan later for AutoConfig getting the complete ScanTab
                if ((pAd->MlmeAux.bNeedPlatformIndicateScanStatus == FALSE) && MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
                {
                    ndisStatus = NDIS_STATUS_SUCCESS;
                    DBGPRINT(RT_DEBUG_TRACE, ("Confirm Scan Later (Driver is auto scanning).... \n"));
                    pAd->MlmeAux.bNeedPlatformIndicateScanStatus = TRUE;
                    pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;  // avoid the MlmeCntlMachinePerformAction dispatch MT2_SCAN_CONF to wrong function.
            }
            else
            {
                    PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
                    DBGPRINT(RT_DEBUG_TRACE, ("complete this scan , it is ap port, or scan in progress, Port =%d, Type =%d\n",
                        PortNumber,
                        pPort->PortType));
            }

                break;  // not going to do futher actions
        }
            
            // this scan is not for fast-roaming, set to FALSE
            pAd->StaCfg.bFastRoamingScan = FALSE;

            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST\n"));    
#if 0           
            // if a system scan already in progress, we queue this scan confirm
            if(pAd->MlmeAux.bNeedPlatformIndicateScanStatus  &&  MT_TEST_FLAG(pAd,fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {               
                ndisStatus = NDIS_STATUS_DOT11_MEDIA_IN_USE;
                PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_SCAN_CONFIRM, pAd->pNicCfg->ScanRequestID, &ndisStatus, sizeof(NDIS_STATUS));
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST :: another is scanning , NDIS_STATUS_INVALID_STATE, return scan request\n"));
                break;
    }
#endif // if 0       

            if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
            {
                ULONGLONG       Now64;

                NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

#if 0 // To be merged
                if (pAd->WhckLastOidScanTime + 15*ONE_SECOND_TIME > Now64)
                {
                    pAd->WhckNormalScan = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set scan time as normail scan time\n"));
                }
                else
                {
                    pAd->WhckNormalScan = FALSE;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set scan time as short scan time\n"));
                }
                pAd->WhckLastOidScanTime = Now64;
#endif // if 0

                // If P2pMs is in operation, reject 1min port0 scan
                if (((Now64 - pPort->P2PCfg.LastFlushBssTime) < ONE_SECOND_TIME) || ((Now64 - pPort->P2PCfg.LastEnumBssTime) < ONE_SECOND_TIME))
                {
                    // Do 1. enum+scan: triggered by user, 2. flush+scn: triggered by WHCK
                }
                else
                {
                    if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) ||
                        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("P2pMs is in operation, ignore this 1 min periodic OID_DOT11_SCAN_REQUEST...\n"));
                        PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
                        break;                  
                    }
                }
            }

            pAd->MlmeAux.bActiveScanForHiddenAP = FALSE; // Reset to default
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST, (as %s) do this request\n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));

#ifdef MULTI_CHANNEL_SUPPORT
            if (pAd->MccCfg.MultiChannelEnable == TRUE)
            {
                // Save port
                pAd->MccCfg.pScanRequestPort = pPort;

                // Set the flag
                MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_EXT_SCAN_PRE_START);
                DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Set MULTI_CHANNEL_EXT_SCAN_PRE_START flag\n", __FUNCTION__));

                // Pending this OID
                ndisStatus = NDIS_STATUS_PENDING;
            }
            else
#endif // MULTI_CHANNEL_SUPPORT               
            {   
                ndisStatus = MpScanRequest(
                                    pAd,
                                    pPort,
                                    InformationBuffer,
                                    InformationBufferLength,
                                    BytesRead,
                                    BytesNeeded
                                    );
            }
#else // if 0
            ndisStatus = NDIS_STATUS_SUCCESS;
            PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_SCAN_CONFIRM, pAd->pNicCfg->ScanRequestID, &ndisStatus, sizeof(NDIS_STATUS));
#endif // if 1
        }while (FALSE);
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}         

NDIS_STATUS
N6SetOidDot11DisconnectRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;


    // EXPAND_FUNC NONE
#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
#ifdef MULTI_CHANNEL_SUPPORT
        if ((NdisRequest->DATA.SET_INFORMATION.Oid == OID_DOT11_DISCONNECT_REQUEST) ||
            (NdisRequest->DATA.SET_INFORMATION.Oid == OID_DOT11_WFD_DISCONNECT_FROM_GROUP_REQUEST))
        {
            pAd->MccCfg.PendedDisconnectOidRequest = NdisRequest;
        }
#endif // MULTI_CHANNEL_SUPPORT            
#endif  


    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_DISCONNECT_REQUEST (%s)\n",decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        if ((pPort->P2PCfg.P2PConnectState == P2P_I_AM_CLIENT_ASSOC_AUTH) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        {
            // When I am GO, GUI still set Disconnect_request and then set a adhoc profile, I should ignore this.
        }
        else
        {
#ifdef MULTI_CHANNEL_SUPPORT
            // For STA
            if (pAd->MccCfg.MultiChannelEnable == TRUE)
            {
                // Save disconnect port
                pAd->MccCfg.pDisconnectPort = pPort;

                // Set the flag
                MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_STOP);
                DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Set MULTI_CHANNEL_STA_PRE_STOP flag\n", __FUNCTION__));

                // Pending this OID
                ndisStatus = NDIS_STATUS_PENDING;
            }
            else
#endif /*MULTI_CHANNEL_SUPPORT*/                    
            {
                ndisStatus = DisconnectRequest(pAd, pPort, BytesRead, BytesNeeded, FALSE);              
            }
        }
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}          

NDIS_STATUS
N6SetOidDot11ConnectRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
    PMAC_TABLE_ENTRY pEntry = NULL;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
        NDIS_PORT_NUMBER    ConnectedPortNum;
        BOOLEAN             bConnected =FALSE;

            // No need to check INIT_STATE
#if 0           
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_CONNECT_REQUEST failed on Invalid state\n"));
                break;
            }
#endif          

            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CONNECT_REQUEST %s\n",decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    
            if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::GOGO OID_DOT11_CONNECT_REQUEST, Start AP mode. %s\n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                MP_SET_STATE(pPort, OP_STATE);

                //Indicate GO to OS
                P2pIndicateGo(pAd, pPort);

                // GO in sta mode: update extra information to link is up
                pAd->UiCfg.ExtraInfo = GENERAL_LINK_UP;
                // tell GUI that GO starts a p2p network.
                pPort->P2PCfg.LinkState = Ralink802_11NetworkTypeP2P;
                break;
            }

            // do not do Adhoc if doing WPS registra now.
            if (IS_P2P_REGISTRA(pPort) && (pAd->OpMode == OPMODE_STA))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CONNECT_REQUEST return, %s\n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));             
                break;
            }

            if(pAd->StaCfg.bRadio == FALSE)
            {
                break;
            }

            if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
            {
                MP_SET_STATE(pPort, OP_STATE);
                if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
            }
            else
            {
                    //APStartUp(pAd,pPort);
                    RestartAPIsRequired = TRUE;
            }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CONNECT_REQUEST, Start AP mode\n"));
                break;
        }
            // do not do Adhoc if 5G adhoc is disabled
            if ( (pAd->StaCfg.bAdhoc5GDisable) && (pAd->StaCfg.BssType == BSS_ADHOC) && (pPort->Channel>14))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CONNECT_REQUEST, Discard Connect with 5GHz Adhoc!!\n"));              
                break;
    }

            // Allow two-client mode with wlanSTA + p2pCLI
            if (pAd->OpMode != OPMODE_STAP2P)
            {
                if(pAd->NumberOfPorts > 1)
                {
                    Ndis6CommonCheckanyPortConnected(pAd, &ConnectedPortNum, &bConnected);
                    if(bConnected)
                    {
                        ndisStatus = NDIS_STATUS_DOT11_MEDIA_IN_USE;                    
                        break;
                    }
                }
            }

            // When the WPS is enabled, the STA has to prevent the connection request from the Vista Zero Config.
            if (pPort->StaCfg.WscControl.WscState > WSC_STATE_OFF)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CONNECT_REQUEST (When the WPS is enabled, the STA has to prevent the connection request from the Vista Zero Config).\n"));
                ndisStatus = NDIS_STATUS_DOT11_MEDIA_IN_USE;
                break;
            }

            if (P2P_OFF(pPort))
            {
                // Delete queued event that was set when disabling P2P.
                pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = FALSE;
            }

            pAd->MlmeAux.AutoReconnectStatus = FALSE;
            pAd->MlmeAux.AutoReconnectSsidLen= 32;
            PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

            //Update P2P connection state on P2P port
            if(pPort->PortNumber == pPort->P2PCfg.PortNumber)
                P2pConnectRequest(pAd, pPort);
            
            if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
            {               
                MlmeRestartStateMachine(pAd,pPort);
            }           
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
#ifdef MULTI_CHANNEL_SUPPORT
            // Polling another H/W Q to be empty for new connection session during multi-channel
            if (pPort->PortNumber == PORT_0)
            {
                //P2pMsStartActionFrame(pAd, pPort);
                P2pMsStartActionFrameExec(pAd, pPort);
            }
#endif /*MULTI_CHANNEL_SUPPORT*/
#endif

            pAd->MlmeAux.bNeedIndicateConnectStatus = TRUE;

            // Reset auto-connect limitations
            pAd->MlmeAux.ScanForConnectCnt = 0;

            //for Ndistest association_ext test, ndistest will connect an AP without inforeset
            pAd->MlmeAux.bStaCanRoam = FALSE;
            pAd->ucActivePortNum = pPort->PortNumber;

            MlmeSyncMacTableDumpInfo(pPort);
            
            MlmeEnqueue(pAd,
                        pPort,
                        MLME_CNTL_STATE_MACHINE,
                        OID_DOT11_CONNECT_REQUEST,
                        0,
                        NULL);
            MP_SET_STATE(pPort, OP_STATE);

            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CONNECT_REQUEST (ucActivePortNum = %d)\n", pAd->ucActivePortNum));                        
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}        

NDIS_STATUS
N6SetOidDot11AutoConfigEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
    {
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
        {
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_AUTO_CONFIG_ENABLED failed on Invalid state\n"));
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
            }

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                pAd->pNicCfg->AutoConfigEnabled = *(PULONG)InformationBuffer;
                *BytesRead = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_AUTO_CONFIG_ENABLED (=%d)\n", pAd->pNicCfg->AutoConfigEnabled));
            }
        } while (FALSE);
        }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetAutoConfigEnabled()
        ULONG AutoConfigEnabled = FALSE;
        
        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
    }

            VALIDATE_AP_INIT_STATE(pPort);
      
            AutoConfigEnabled = *(PULONG)InformationBuffer;
            pPort->AutoConfigEnabled = AutoConfigEnabled & ALLOWED_AUTO_CONFIG_FLAGS;

            *BytesRead = sizeof(ULONG);
        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}      

NDIS_STATUS
N6SetOidDot11CurrentOperationMode(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    // EXPAND_FUNC PortOidSetCurrentOperationMode()
    do
    {
        if (InformationBufferLength < sizeof(DOT11_CURRENT_OPERATION_MODE))
        {
            *BytesNeeded = sizeof(DOT11_CURRENT_OPERATION_MODE);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
    }
        else
        {
            PDOT11_CURRENT_OPERATION_MODE pCurrentOperationMode = (PDOT11_CURRENT_OPERATION_MODE)InformationBuffer;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER)) //keep watch
            if ((pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_EXTENSIBLE_STATION) &&
                (pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_NETWORK_MONITOR) &&
                (pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_EXTENSIBLE_AP) &&
                (pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_WFD_DEVICE) &&
                (pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_WFD_GROUP_OWNER) &&
                (pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_WFD_CLIENT))
#else
            if ((pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_EXTENSIBLE_STATION) &&
                (pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_NETWORK_MONITOR) &&
                (pCurrentOperationMode->uCurrentOpMode != DOT11_OPERATION_MODE_EXTENSIBLE_AP))
#endif
    {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }
            pPort->PortCfg.OperationMode = pCurrentOperationMode->uCurrentOpMode;           
            
            if (pPort->PortCfg.OperationMode == DOT11_OPERATION_MODE_EXTENSIBLE_AP)
            {
                PMAC_TABLE_ENTRY pMBCastEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);
                if (pMBCastEntry != NULL)
                {
                    MtAsicUpdateRxWCIDTable(pAd, pMBCastEntry->wcid,  BROADCAST_ADDR);
                }
            
                //
                // The operational state is INIT
                //
                ApSetState(pPort,AP_STATE_STOPPED);
                pPort->PortType = EXTAP_PORT;   
                pPort->PortSubtype = PORTSUBTYPE_VwifiAP;
                //
                // Initital default value
                //
                PlatformZeroMemory(&pPort->PortCfg.Ssid, 32); //SSID will be set on Desired SSID

                COPY_MAC_ADDR(pPort->PortCfg.Bssid, pPort->CurrentAddress);
            }
            else if(pPort->PortCfg.OperationMode == DOT11_OPERATION_MODE_NETWORK_MONITOR)
            {
                // 
                // In network monitor mode, we wont allow packets to be sent 
                // or scans to happen
                //
                
                //MlmeInfoEnablePromiscuousMode(pAd);

                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_ENABLE_PROMISCUOUS_MODE, NULL, 0);

                //
                // Disable autoconfig
                //
                pAd->pNicCfg->AutoConfigEnabled = 0;
            }
            else if(pPort->PortCfg.OperationMode == DOT11_OPERATION_MODE_EXTENSIBLE_STATION)    
            {
                // Reset AP status only at SoftAP(and VWiFi) mode.
                if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP ||pPort->PortSubtype == PORTSUBTYPE_VwifiAP)
                {
                    ApSetState(pPort,AP_STATE_INVALID);     
                }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
                if(NDIS_WIN8_ABOVE(pAd))
                {
                    // Reset wfd port number
                    if (pPort->PortType == WFD_DEVICE_PORT)
                        pPort->P2PCfg.PortNumber = PORT_0;
                    if (pPort->PortType == WFD_GO_PORT)
                        pPort->P2PCfg.GOPortNumber = PORT_0;
                    // clear multiple clients
                    if (pPort->PortType == WFD_CLIENT_PORT)
                    {
                        if (pPort->PortNumber == pPort->P2PCfg.ClientPortNumber)// #1
                        {
                            pPort->P2PCfg.ClientPortNumber = PORT_0;
                            MT_CLEAR_BIT(pPort->P2PCfg.ClientUseBitmap, P2pMs_CLIENT_1_INUSED);
                        }
                        else if (pPort->PortNumber == pPort->P2PCfg.Client2PortNumber)//#2
                        {
                            pPort->P2PCfg.Client2PortNumber = PORT_0;
                            MT_CLEAR_BIT(pPort->P2PCfg.ClientUseBitmap, P2pMs_CLIENT_2_INUSED);
                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Clear wfd port number (porttype=%d, portnum=%d(%d-%d-%d-%d), cli-bitmap=%d)\n", 
                        __FUNCTION__, pPort->PortType, pPort->PortNumber, pPort->P2PCfg.PortNumber, pPort->P2PCfg.GOPortNumber, pPort->P2PCfg.ClientPortNumber, pPort->P2PCfg.Client2PortNumber, pPort->P2PCfg.ClientUseBitmap));
            }
#endif

                MP_SET_STATE(pPort, INIT_STATE);
                pPort->PortType = EXTSTA_PORT;  
                // keep PortSubtype for Ralink-SoftAP and P2P + STA concurrent
                if ((pPort->PortSubtype != PORTSUBTYPE_SoftapAP) && (pAd->OpMode != OPMODE_STAP2P))
                {
                    pPort->PortSubtype = PORTSUBTYPE_STA;
                }

                if (pPort->PortNumber != PORT_0)
                {
                    NdisCommonMappingPortToBssIdx(pPort, DELETE_MAC, DELETE_MAC);
                }
                
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_DISABLE_PROMISCUOUS_MODE, NULL, 0);
                DBGPRINT(RT_DEBUG_TRACE, ("Port Set::DOT11_OPERATION_MODE_EXTENSIBLE_STATION\n"));
            }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))//not modify for temp
            else if (pPort->PortCfg.OperationMode == DOT11_OPERATION_MODE_WFD_DEVICE)
            {
                MP_SET_STATE(pPort, INIT_STATE);
                pPort->PortType = WFD_DEVICE_PORT;
                pPort->PortSubtype = PORTSUBTYPE_P2PDevice;
                pPort->P2PCfg.PortNumber = (UCHAR) pPort->PortNumber;
                //AsicSetMultiBssid(pAd, pPort->CurrentAddress, 0);
                MtAsicSetMultiBssidEx(pAd, pPort->CurrentAddress, (UCHAR) pPort->PortNumber);
                // enable ms p2p once initialize device port
                P2pMsEnable(pAd, pPort);

                NdisCommonMappingPortToBssIdx(pPort, UPDATE_MAC, DO_NOTHING);

                DBGPRINT(RT_DEBUG_TRACE, ("Port Set::DOT11_OPERATION_MODE_WFD_DEVICE\n"));
            }
            else if (pPort->PortCfg.OperationMode == DOT11_OPERATION_MODE_WFD_GROUP_OWNER)
            {
                PlatformZeroMemory(&pPort->PortCfg.Ssid, 32); //SSID will be set on Desired SSID

                COPY_MAC_ADDR(pPort->PortCfg.Bssid, pPort->CurrentAddress);

                MP_SET_STATE(pPort, INIT_STATE);
                pPort->PortType = WFD_GO_PORT;
                pPort->P2PCfg.GOPortNumber = (UCHAR) pPort->PortNumber;

                NdisCommonMappingPortToBssIdx(pPort, UPDATE_MAC, UPDATE_MAC);
                // PORTSUBTYPE_P2PGO will be set in P2pMsStartGo()
                DBGPRINT(RT_DEBUG_TRACE, ("Port Set::DOT11_OPERATION_MODE_WFD_GROUP_OWNER\n"));         
            }
            else if (pPort->PortCfg.OperationMode == DOT11_OPERATION_MODE_WFD_CLIENT)
            {
                //PMAC_TABLE_ENTRY pMBCastEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);
                MP_SET_STATE(pPort, INIT_STATE);
                pPort->PortType = WFD_CLIENT_PORT;
                
                if ((!MT_TEST_BIT(pPort->P2PCfg.ClientUseBitmap, P2pMs_CLIENT_1_INUSED)) || (pPort->P2PCfg.ClientPortNumber == pPort->PortNumber))
                {
                    pPort->P2PCfg.ClientPortNumber = (UCHAR) pPort->PortNumber;
                    MT_SET_BIT(pPort->P2PCfg.ClientUseBitmap, P2pMs_CLIENT_1_INUSED);
                }
                else if ((!MT_TEST_BIT(pPort->P2PCfg.ClientUseBitmap, P2pMs_CLIENT_2_INUSED)) || (pPort->P2PCfg.Client2PortNumber == pPort->PortNumber))
                {
                    pPort->P2PCfg.Client2PortNumber = (UCHAR) pPort->PortNumber;
                    MT_SET_BIT(pPort->P2PCfg.ClientUseBitmap, P2pMs_CLIENT_2_INUSED);
            }
                //DBGPRINT(RT_DEBUG_TRACE, ("AsicUpdateRxWCIDTable (portnum=%d, wcid=%d Addr = BROADCAST_ADDR )\n", pPort->PortNumber, pMBCastEntry->wcid));
                
                // [PortSubtype Usage] 
                // When client role port is created, set as PORTSUBTYPE_P2PClient
                // When it goes back to EXTSTA, reset to PORTSUBTYPE_STA 
                pPort->PortSubtype = PORTSUBTYPE_P2PClient;
                NdisCommonMappingPortToBssIdx(pPort, UPDATE_MAC, DO_NOTHING);
                DBGPRINT(RT_DEBUG_TRACE, ("Port Set::DOT11_OPERATION_MODE_WFD_CLIENT (portnum=%d, bitmap=%d)\n", pPort->PortNumber, pPort->P2PCfg.ClientUseBitmap));                  
    }
#endif
            else
            {
                DBGPRINT_ERR(("[%s][LINE_%d] Unknown Operation Mode (%x)!!\n",__FUNCTION__,__LINE__, pPort->PortCfg.OperationMode));
            }
            //APRemoveAdditionalIE(pPort);
            Ndis6CommonPortFreeVariableIE(pPort);
            DBGPRINT(RT_DEBUG_TRACE, ("Port Set::OID_DOT11_CURRENT_OPERATION_MODE PortNum =%d ,(mode=0x%08x) (%x:%x:%x:%x:%x:%x)\n", 
                pPort->PortNumber,pPort->PortCfg.OperationMode, pPort->CurrentAddress[0],pPort->CurrentAddress[1],pPort->CurrentAddress[2],pPort->CurrentAddress[3],pPort->CurrentAddress[4],pPort->CurrentAddress[5]));
        }

        *BytesRead = sizeof(DOT11_CURRENT_OPERATION_MODE);
        *BytesNeeded = sizeof(DOT11_CURRENT_OPERATION_MODE);
    }while(FALSE);

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}      

NDIS_STATUS
N6SetOidDot11FlushBssList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG       BssIdx = BSS_NOT_FOUND;
            PBSS_ENTRY  pCurrBss = NULL;
            
            if ((pAd->StaCfg.SmartScan && (INFRA_ON(pPort) || ADHOC_ON(pPort) || (pPort->SoftAP.bAPStart == TRUE)))||
                (pAd->StaCfg.OzmoDeviceSuppressScan != 0) ||
                (pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE))

    {
                ULONGLONG Now;

                NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now);
                Now = Now/10000;    // convert as ms

                if ((((Now - pAd->StaCfg.LastScanTimeFromNdis) > 55*1000) && ((Now - pAd->StaCfg.LastScanTimeFromNdis) < 65*1000)) ||
                    (((Now - pAd->StaCfg.LastScanTimeFromNdis) > 115*1000) && ((Now - pAd->StaCfg.LastScanTimeFromNdis) < 125*1000)) ||
                    (((Now - pAd->StaCfg.LastScanTimeFromNdis) > 175*1000) && ((Now - pAd->StaCfg.LastScanTimeFromNdis) < 185*1000)))
        {
                    DBGPRINT(RT_DEBUG_TRACE, ("Ignore OID_DOT11_FLUSH_BSS_LIST, due to periodic scan, LastScanTimeFromNdis=%I64d, Now=%I64d, diff=%I64d \n",
                        pAd->StaCfg.LastScanTimeFromNdis, Now, (Now - pAd->StaCfg.LastScanTimeFromNdis)));
                    break;
                }
                else if (pAd->StaCfg.OzmoDeviceSuppressScan == 2)
            {
                    DBGPRINT(RT_DEBUG_TRACE, ("Ignore OID_DOT11_FLUSH_BSS_LIST, due to OzmoDevice, LastScanTimeFromNdis=%I64d, Now=%I64d, diff=%I64d \n",
                        pAd->StaCfg.LastScanTimeFromNdis, Now, (Now - pAd->StaCfg.LastScanTimeFromNdis)));
                    break;
            }
                else if (pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE)
            {
                    DBGPRINT(RT_DEBUG_TRACE, ("Ignore OID_DOT11_FLUSH_BSS_LIST, due to Cpu Util\n"));
                    break;
            }
        }
            
            if (pAd->pHifCfg->BulkLastOneSecCount > 30)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_FLUSH_BSS_LIST, Heavy traffic on TX/RX ignore this set::OID_DOT11_FLUSH_BSS_LIST\n"));
                break;
    }

            if(pPort->bRoamingAfterResume )
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_FLUSH_BSS_LIST, ignore this set ,due to bRoamingAfterResume \n"));
                break;
            }

            if(P2PIsScanAllowed(pAd, pPort) == FALSE)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_FLUSH_BSS_LIST, ignore this set ,due to P2P in progress \n"));
                break;
            }

            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_FLUSH_BSS_LIST (LastBulkCount=%d\n", pAd->pHifCfg->BulkLastOneSecCount));

            //
            // To tell OID_DOT11_CONNECT_REQUEST that we clear all the scan table.
            // So we can't skip OID_DOT11_CONNECT_REQUEST if NDIS asks for it.
            //
            pAd->pNicCfg->bIsClearScanTab = TRUE;

            PlatformAllocateMemory(pAd,  &pCurrBss, sizeof(BSS_ENTRY));
            if (pCurrBss != NULL)
            {
                PlatformZeroMemory(pCurrBss, sizeof(BSS_ENTRY));
                if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
                {
                    BssIdx = BssSsidTableSearch(&pAd->ScanTab, pPort->PortCfg.Bssid, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen, pPort->Channel);
                    if (BssIdx != BSS_NOT_FOUND)
                    {
                        PlatformMoveMemory(pCurrBss, &pAd->ScanTab.BssEntry[BssIdx], sizeof(BSS_ENTRY));
                    }
                }
                
                // clean up previous SCAN result, add current BSS back to table if any
                BssTableInit(&pAd->ScanTab); 

                //reset Reposit Bss Table
                ResetRepositBssTable(pAd);

                if (BssIdx != BSS_NOT_FOUND)
                {
                    // Restore entries that are in use
                    PlatformMoveMemory(&pAd->ScanTab.BssEntry[0], pCurrBss, sizeof(BSS_ENTRY));
                    pAd->ScanTab.BssNr = 1;
                }

                PlatformFreeMemory(pCurrBss, sizeof(BSS_ENTRY));
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed for BSS_ENTRY\n", __FUNCTION__));
            }

            // Record timestamp for rejecting 1min port0 scan during P2pMs connection PING test
            if (pPort->PortNumber == PORT_0)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_FLUSH_BSS_LIST - Record OS time.\n"));
                NdisGetCurrentSystemTime((PLARGE_INTEGER)&pPort->P2PCfg.LastFlushBssTime);
            }  
        } while(FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC BREAK
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}       

NDIS_STATUS
N6SetOidDot11DesiredSsidList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        if(PortType == EXTSTA_PORT)
        {
            // EXPAND_FUNC NONE
            ULONG               i = 0;
            UCHAR               ssid[33];
            PDOT11_SSID_LIST    ssidList = (PDOT11_SSID_LIST)InformationBuffer;
            DBGPRINT(RT_DEBUG_INFO, ("OID_DOT11_DESIRED_SSID_LIST: uNumOfEntries       = %d\n", ssidList->uNumOfEntries));
            DBGPRINT(RT_DEBUG_INFO, ("uTotalNumOfEntries  = %d\n", ssidList->uTotalNumOfEntries));
            memset(ssid, 0, 33);
            for (i = 0; i < ssidList->uNumOfEntries; i++) 
            {
                memcpy(ssid, ssidList->SSIDs[i].ucSSID, ssidList->SSIDs[i].uSSIDLength); 
                DBGPRINT(RT_DEBUG_INFO, ("%-2d) %s\n", i, ssid));
            }
    }

        do
    {
            // EXPAND_FUNC NONE
            PDOT11_SSID_LIST pDot11SSIDList = (PDOT11_SSID_LIST)InformationBuffer;
            DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Set OID_DOT11_DESIRED_SSID_LIST\n",__FUNCTION__,__LINE__));
// No need to check INIT_STATE
#if 0               
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
        {
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_DESIRED_SSID_LIST failed on Invalid state\n"));
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
            }
#endif              

            //
            // Only support Single SSID
            //
            *BytesNeeded =  FIELD_OFFSET(DOT11_SSID_LIST, SSIDs) + sizeof(DOT11_SSID);

            if (InformationBufferLength < (*BytesNeeded))
            {
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                DBGPRINT(RT_DEBUG_WARN, ("ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW\n"));
                break;
            }

            if (pDot11SSIDList->uNumOfEntries < 1)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                DBGPRINT(RT_DEBUG_WARN, ("ndisStatus = NDIS_STATUS_INVALID_DATA\n"));
                break;
            }

            if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
            {   
                PlatformZeroMemory(pPort->PortCfg.Ssid, MAX_LEN_OF_SSID);
                PlatformMoveMemory(pPort->PortCfg.Ssid, pDot11SSIDList->SSIDs[0].ucSSID, pDot11SSIDList->SSIDs[0].uSSIDLength);
                pPort->PortCfg.SsidLen = (UCHAR) pDot11SSIDList->SSIDs[0].uSSIDLength;
                PlatformZeroMemory(&pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
                PlatformMoveMemory(&pPort->PortCfg.DesiredSSID, &pDot11SSIDList->SSIDs[0], sizeof(DOT11_SSID));
                //RestartAPIsRequired = TRUE;

                // The SoftAP should broadcast beacon packets untill the ApUI makes the OID_DOT11_DESIRED_SSID_LIST request.
                pPort->SoftAP.ApCfg.bSoftAPReady = TRUE;
            }
            else if(pPort->PortType == EXTAP_PORT)
            {
                PlatformZeroMemory(pPort->PortCfg.Ssid, MAX_LEN_OF_SSID);
                PlatformMoveMemory(pPort->PortCfg.Ssid, pDot11SSIDList->SSIDs[0].ucSSID, pDot11SSIDList->SSIDs[0].uSSIDLength);                 
                pPort->PortCfg.SsidLen = (UCHAR) pDot11SSIDList->SSIDs[0].uSSIDLength;
                PlatformZeroMemory(&pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
                PlatformMoveMemory(&pPort->PortCfg.DesiredSSID, &pDot11SSIDList->SSIDs[0], sizeof(DOT11_SSID));
                DBGPRINT(RT_DEBUG_TRACE, ("AP mode Set::OID_DOT11_DESIRED_SSID_LIST (len=%d,SSID=%s\n",
                    pPort->PortCfg.SsidLen, pPort->PortCfg.Ssid));

            }
            else
            {
                PlatformZeroMemory(&pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
                PlatformMoveMemory(&pPort->PortCfg.DesiredSSID, &pDot11SSIDList->SSIDs[0], sizeof(DOT11_SSID));
                DBGPRINT(RT_DEBUG_TRACE, ("sta mode Set::OID_DOT11_DESIRED_SSID_LIST (len=%d,SSID=%s port=%d\n",
                    pPort->PortCfg.DesiredSSID.uSSIDLength, pPort->PortCfg.DesiredSSID.ucSSID, pPort->PortNumber));

            }
            
            DBGPRINT(RT_DEBUG_TRACE, ("\n"));
        }while (FALSE);
        }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetDesiredSsidList()
        ULONG               requiredListSize = 0;
        PDOT11_SSID_LIST    SsidList = NULL;
        DBGPRINT(RT_DEBUG_TRACE, ("ApOidSetDesiredSsidList=>>\n"));
        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(DOT11_SSID_LIST))
            {
                *BytesNeeded = sizeof(DOT11_SSID_LIST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                DBGPRINT(RT_DEBUG_TRACE, ("ApOidSetDesiredSsidList: NDIS_STATUS_INVALID_LENGTH \n"));
                break;
    }

            SsidList = (PDOT11_SSID_LIST)InformationBuffer;

            if (!GetRequiredListSize(
                            FIELD_OFFSET(DOT11_SSID_LIST, SSIDs),
                            sizeof(DOT11_SSID),
                            SsidList->uNumOfEntries,
                            &requiredListSize))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("ApOidSetDesiredSsidList NDIS_STATUS_INVALID_DATA\n"));
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            if (InformationBufferLength < requiredListSize)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("ApOidSetDesiredSsidList NDIS_STATUS_INVALID_LENGTH\n"));
                *BytesNeeded = requiredListSize;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            ndisStatus = ApSetDesiredSsidList(pAd, pAd->PortList[PortNumber], SsidList);
            if (ndisStatus == NDIS_STATUS_SUCCESS)
            {
                *BytesRead = requiredListSize;
            }
        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CurrentChannel(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                DBGPRINT(RT_DEBUG_ERROR,("Set::OID_DOT11_CURRENT_CHANNEL failed on Invalid state\n"));
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
    }

            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    {
                DBGPRINT(RT_DEBUG_ERROR,("Set::OID_DOT11_CURRENT_CHANNEL failed on Invalid power state\n"));
                ndisStatus = NDIS_STATUS_DOT11_POWER_STATE_INVALID;
                break;
            }

#if 0
            //
            // A scan operation is in progress. Setting the channel could interfere
            // with the scanning process. We will not honor this request.
            //
            if (MP_DOT11_SCAN_IN_PROGRESS(pAd))
            {
                ndisStatus = NDIS_STATUS_DOT11_MEDIA_IN_USE;
                break;
            }
#endif

            // If autoconfig is enabled, we dont allow this to be set
            if (pAd->pNicCfg->AutoConfigEnabled & DOT11_PHY_AUTO_CONFIG_ENABLED_FLAG)
            {
                ndisStatus = NDIS_STATUS_DOT11_AUTO_CONFIG_ENABLED;
                break;
            }

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
        {
                ULONG Channel = *(PULONG)InformationBuffer;

            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
            }
            else
            {
                    //AsicSwitchChannel(pAd, (UCHAR)Channel, FALSE);
                    AsicSwitchChannel(pPort, (UCHAR)Channel, (UCHAR)Channel, BW_20, FALSE);
                }

                *BytesRead = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_CURRENT_CHANNEL (=%d)\n", Channel));
            }
        } while (FALSE);
        }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC NONE
        if(KeGetCurrentIrql() == DISPATCH_LEVEL)
        {
    }
        else
        {
            // EXPAND_FUNC ApOidSetCurrentChannel()
            PMP_PORT pStaPort = NULL;
            ULONG    CurrentChannel;

            do
            {
                *BytesRead = 0;
                *BytesNeeded = 0;

                if (InformationBufferLength < sizeof(ULONG))
                {
                    *BytesNeeded = sizeof(ULONG);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    break;
                }

                pPort = pAd->PortList[PortNumber];
                
                //
                // If autoconfig is enabled, we dont allow this to be set
                //
                if (pPort->AutoConfigEnabled & DOT11_PHY_AUTO_CONFIG_ENABLED_FLAG)
                {
                    ndisStatus = NDIS_STATUS_DOT11_AUTO_CONFIG_ENABLED;
                    break;
                }

                CurrentChannel = *(PULONG)InformationBuffer;
                //
                // validate channel
                // TODO: channel shall be in the available channel list
                //
                if (CurrentChannel < 1 || CurrentChannel > 14)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                //
                // Save this channel for later use on starting SofAP.
                //
                //if already one station connected then do not siwtch channel
                pStaPort = Ndis6CommonGetStaPort(pAd);
                if(pStaPort != NULL)
                {
                    if( (pPort->Channel  != CurrentChannel) &&  OPSTATUS_TEST_FLAG(pStaPort, fOP_STATUS_MEDIA_STATE_CONNECTED))         
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("sta connected Port(%u) remain stay at ch %d)\n", PortNumber,pPort->Channel));
                    }
                    else
                    {
                        pPort->Channel = (UCHAR)CurrentChannel;             
                        DBGPRINT(RT_DEBUG_TRACE, ("no sta connected Port(%u) change to ch %d)\n", PortNumber,pPort->Channel  ));
                    }
                }
                else
                {
                    pPort->Channel = (UCHAR)CurrentChannel;             
                    DBGPRINT(RT_DEBUG_TRACE, ("only one AP Port(%u) change to ch %d)\n", PortNumber,pPort->Channel  ));
                }
                pPort->CommonCfg.AddHTInfo.ControlChan = pPort->Channel;
                *BytesRead = sizeof(ULONG);
            } while (FALSE);
        }
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11SafeModeEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pAd, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_SAFE_MODE_ENABLED failed on Invalid state\n"));
                break;
    }

            if (InformationBufferLength < sizeof(BOOLEAN))
    {
                *BytesNeeded = sizeof(BOOLEAN);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;          
            }
            else
        {
                pPort->CommonCfg.bSafeModeEnabled = *(PBOOLEAN)InformationBuffer;
                *BytesRead = sizeof(BOOLEAN);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::>>OID_DOT11_SAFE_MODE_ENABLED [%d]\n",pPort->CommonCfg.bSafeModeEnabled));

                //Force to remove N mode. Vista SP1's FIPS cannot support QoS data.
                if( (pPort->CommonCfg.bSafeModeEnabled == 1) && (pPort->CommonCfg.SafeModeQOS_PARAMS == 0) )
            {
                    pPort->CommonCfg.SafeModeRestorePhyMode = pPort->CommonCfg.PhyMode;
                    if( pPort->CommonCfg.PhyMode == PHY_11ABGN_MIXED  )
                    {
                        MlmeInfoSetPhyMode(pAd, pPort, PHY_11ABG_MIXED,FALSE);
                    }
                    else if( pPort->CommonCfg.PhyMode == PHY_11BGN_MIXED  )
                    {
                        MlmeInfoSetPhyMode(pAd, pPort, PHY_11BG_MIXED,FALSE);
                    }
                    else if( pPort->CommonCfg.PhyMode == PHY_11GN_MIXED  )
                    {
                        MlmeInfoSetPhyMode(pAd, pPort, PHY_11G,FALSE);
                    }   
                    else if( pPort->CommonCfg.PhyMode == PHY_11AN_MIXED  )
                    {
                        MlmeInfoSetPhyMode(pAd, pPort, PHY_11A,FALSE);
            }
            else
            {
                        MlmeInfoSetPhyMode(pAd, pPort, PHY_11ABG_MIXED,FALSE);
            }
        }
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CurrentPhyId(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC MpSetCurrentPhyId()
        ULONG PhyId;

        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            *BytesRead = sizeof(ULONG);
            PhyId = *((PULONG)InformationBuffer);

            if (PhyId >= pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("MpSetCurrentPhyId, Invalued PhyId(=%d), Max(=%d)\n", 
                            PhyId, pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries - 1));
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }
            pPort->CommonCfg.SelectedPhyId = PhyId;

            DBGPRINT(RT_DEBUG_TRACE, ("MpSetCurrentPhyId, PhyId=%d\n", PhyId));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetCurrentPhyId()
        ULONG CurrentPhyId;
        
        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            CurrentPhyId = *(PULONG)InformationBuffer;

            if (CurrentPhyId >= pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries) 
            {
                //
                // Invalid PHY ID
                //
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pPort = pAd->PortList[PortNumber];
            pPort->CommonCfg.SelectedPhyId = CurrentPhyId;

            *BytesRead = sizeof(ULONG);
        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidPnpAddWakeUpPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidPnpRemoveWakeUpPattern(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidPnpEnableWakeUp(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidPnpCapabilities(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidPnpSetPower(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    // EXPAND_FUNC N6UsbPnpSetPower()
    do
    {
        NDIS_DEVICE_POWER_STATE NewDeviceState;

        //
        // Store new power state and succeed the request.
        //
        *BytesNeeded = sizeof(NDIS_DEVICE_POWER_STATE);
        if (InformationBufferLength < *BytesNeeded)
        {
            //
            // Too little data. Bail out.
            //
            DBGPRINT(RT_DEBUG_ERROR, ("Bad length received for the SetPower request\n"));
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        PlatformMoveMemory(&NewDeviceState, InformationBuffer, sizeof(NDIS_DEVICE_POWER_STATE));
        *BytesNeeded = sizeof(NDIS_DEVICE_POWER_STATE);

        if (NewDeviceState <= NdisDeviceStateUnspecified || NewDeviceState >= NdisDeviceStateMaximum)
        {
            //
            // An invalid device state has been specified. Bail out.
            //
            DBGPRINT(RT_DEBUG_ERROR, ("Invalid data received in Set Power request\n"));
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        if (NewDeviceState == pAd->DevicePowerState)
        {
            //
            // We are already in the device state being set. Succeed the call.
            //
            DBGPRINT(RT_DEBUG_INFO, ("Device is already in power state D%d\n", NewDeviceState));
            ndisStatus = NDIS_STATUS_SUCCESS;
            break;
        }

        if(NewDeviceState == NdisDeviceStateD0 )
        {
            pAd->bInShutdown = FALSE;
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_PNP_SET_POWER, InformationBuffer, InformationBufferLength);    
            ndisStatus = NDIS_STATUS_PENDING;
        }           
        else
        {
            pAd->Shutdowncnt = SHUTDOWN_TIMEOUT;
            pAd->bInShutdown = TRUE;
            ndisStatus = N6UsbSetPowerbyPort(pPort,NewDeviceState);
        }
    } while (FALSE);

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11AtimWindow(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                ULONG AtimWindow = *(PULONG)InformationBuffer;
                
                pPort->CommonCfg.AtimWindow = (USHORT)AtimWindow;
                LIMIT_BEACON_PERIOD(pPort->CommonCfg.AtimWindow);
                *BytesRead = sizeof(ULONG);
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11ChannelAgilityEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11ChannelAgilityPresent(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11OperationalRateSet(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC MpSetOperationalRateSet()
        PDOT11_RATE_SET pDot11RateSet = NULL;
        ULONG           dwRequiredSize = 0;

        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("MpSetOperationalRateSet failed on invalid state\n"));  
                break;
            }

            dwRequiredSize = FIELD_OFFSET(DOT11_RATE_SET, ucRateSet);
            if (InformationBufferLength < dwRequiredSize)
            {
                *BytesNeeded = dwRequiredSize;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            pDot11RateSet = InformationBuffer;

            if (pDot11RateSet->uRateSetLength > DOT11_RATE_SET_MAX_LENGTH ||
                pDot11RateSet->uRateSetLength == 0)
            {
                *BytesNeeded = dwRequiredSize;
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            dwRequiredSize += pDot11RateSet->uRateSetLength;
            if (InformationBufferLength < dwRequiredSize)
            {
                *BytesNeeded = dwRequiredSize;
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            *BytesRead = dwRequiredSize;
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC BREAK
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11RtsThreshold(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
    }
            // If autoconfig is enabled, we dont allow this to be set
            if (pAd->pNicCfg->AutoConfigEnabled & DOT11_MAC_AUTO_CONFIG_ENABLED_FLAG)
    {
                ndisStatus = NDIS_STATUS_DOT11_AUTO_CONFIG_ENABLED;
                break;
            }
            else
        {
                ULONG RTS_Threshold = *(PULONG)InformationBuffer;
                TX_RTS_CFG_STRUC RtsCfg;

                if (RTS_Threshold > MAX_RTS_THRESHOLD)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                *BytesRead = sizeof(ULONG);

                // Config ASIC RTS threshold register
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_DOT11_RTS_THRESHOLD, NULL, 0);
                }

                RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                if (pPort->SoftAP.bAPStart || 
                    pPort->PortSubtype == PORTSUBTYPE_P2PGO ||
                    pPort->CommonCfg.RtsThreshold >= MAX_RTS_THRESHOLD)
                {
                    // SoftAP started, reset RTS threshold to maximum
                    pPort->CommonCfg.RtsThreshold = MAX_RTS_THRESHOLD;
                    RtsCfg.field.RtsThres = 0xFFFF;
            }
            else
            {
                    pPort->CommonCfg.RtsThreshold = (USHORT)RTS_Threshold;
                    RtsCfg.field.RtsThres = pPort->CommonCfg.RtsThreshold;
            }
                RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
                
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_RTS_THRESHOLD  (=%d)\n", pPort->CommonCfg.RtsThreshold));
        }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC BREAK
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11ShortRetryLimit(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11LongRetryLimit(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11FragmentationThreshold(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
    }

            // If autoconfig is enabled, we dont allow this to be set
            if (pAd->pNicCfg->AutoConfigEnabled & DOT11_MAC_AUTO_CONFIG_ENABLED_FLAG)
    {
                ndisStatus = NDIS_STATUS_DOT11_AUTO_CONFIG_ENABLED;
                break;
            }
            else
        {
                ULONG Frag_Threshold = *(PULONG)InformationBuffer;
                if (Frag_Threshold == 0)
            {
                    Frag_Threshold = MAX_FRAG_THRESHOLD;
            }
                
                if (Frag_Threshold < 256 || Frag_Threshold > 2346)
            {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
            }
                pPort->CommonCfg.FragmentThreshold = (USHORT)Frag_Threshold;
                *BytesRead = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_FRAGMENTATION_THRESHOLD (=%d)\n", Frag_Threshold)); 
        }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC BREAK
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CurrentRegDomain(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                DBGPRINT(RT_DEBUG_ERROR,("Set::OID_DOT11_CURRENT_REG_DOMAIN failed on Invalid state\n"));
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
            }

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                ULONG RegDomain = *(PULONG)InformationBuffer;
                *BytesRead = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CURRENT_REG_DOMAIN (=%d)\n", RegDomain));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11RecvSensitivityList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11MultiDomainCapabilityEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CountryString(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CurrentFrequency(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                DBGPRINT(RT_DEBUG_ERROR,("Set::OID_DOT11_CURRENT_FREQUENCY failed on Invalid state\n"));
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
    }

            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    {
                DBGPRINT(RT_DEBUG_ERROR,("Set::OID_DOT11_CURRENT_FREQUENCY failed on Invalid state\n"));
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
            }

#if 0
            //
            // A scan operation is in progress. Setting the channel could interfere
            // with the scanning process. We will not honor this request.
            //
            if (MP_DOT11_SCAN_IN_PROGRESS(pAd))
            {
                ndisStatus = NDIS_STATUS_DOT11_MEDIA_IN_USE;
                break;
            }
#endif

            // If autoconfig is enabled, we dont allow this to be set
            if (pAd->pNicCfg->AutoConfigEnabled & DOT11_PHY_AUTO_CONFIG_ENABLED_FLAG)
            {
                ndisStatus = NDIS_STATUS_DOT11_AUTO_CONFIG_ENABLED;
                break;
            }

            if (InformationBufferLength < sizeof(ULONG))
        {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                ULONG OfdmFrequency = *(PULONG)InformationBuffer;
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
            }
            else
            {
                    //AsicSwitchChannel(pAd, (UCHAR)OfdmFrequency, FALSE);
                    AsicSwitchChannel(pPort, (UCHAR)OfdmFrequency, (UCHAR)OfdmFrequency, BW_20, FALSE);
            }
                
                *BytesRead = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CURRENT_FREQUENCY (=%d)\n", OfdmFrequency));
            }
        } while (FALSE);
        }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC NONE
        if(KeGetCurrentIrql() == DISPATCH_LEVEL)
        {
    }
        else
        {
            // EXPAND_FUNC ApOidSetCurrentFrequency()
            PMP_PORT           pStaPort = NULL;
            ULONG              CurrentFreqency;
            BOOLEAN            RestartAPIsRequired = FALSE;
            OID_SET_HT_PHYMODE HTPhymode;

            do
            {
                *BytesRead = 0;
                *BytesNeeded = 0;

                if (InformationBufferLength < sizeof(ULONG))
                {
                    *BytesNeeded = sizeof(ULONG);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    break;
                }

                pPort = pAd->PortList[PortNumber];

                //
                // If autoconfig is enabled, we dont allow this to be set
                //
                if (pPort->AutoConfigEnabled & DOT11_PHY_AUTO_CONFIG_ENABLED_FLAG)
    {
                    ndisStatus = NDIS_STATUS_DOT11_AUTO_CONFIG_ENABLED;
                    break;
    }

                CurrentFreqency = *(PULONG)InformationBuffer;
            
                //
                // validate channel
                // TODO: channel shall be in the available channel list
                //
                if (CurrentFreqency < 5180 || CurrentFreqency > 5805) //Min Ch36, Max Ch161
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                //
                // Save this channel for later use on starting SofAP.
                //
                // if already one station connected then do not siwtch channel
                pStaPort = Ndis6CommonGetStaPort(pAd);
                if(pStaPort != NULL)
                {
                    if( (pPort->Channel != CurrentFreqency) && OPSTATUS_TEST_FLAG(pStaPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("sta connected Port(%u) remain stay at ch %d)\n", PortNumber,pPort->Channel));
                    }
                    else
                    {
                        MAP_MHZ_TO_CHANNEL_ID(CurrentFreqency, pPort->Channel);
                        //RestartAPIsRequired = TRUE;
                        DBGPRINT(RT_DEBUG_TRACE, ("no sta connected Port(%u) change to ch %d)\n", PortNumber,pPort->Channel ));
                    }
                }
                else
    {
                    MAP_MHZ_TO_CHANNEL_ID(CurrentFreqency, pPort->Channel);
                    //RestartAPIsRequired = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("no sta connected Port(%u) change to ch %d)\n", PortNumber,pPort->Channel));
                }
                pPort->CommonCfg.AddHTInfo.ControlChan = pPort->Channel;
                *BytesRead = sizeof(ULONG);
                
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
                    if ((pPort->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pPort->Channel))
            {
                        if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
                        {
                            RadarDetectionStop(pAd);
                        }
                        pPort->CommonCfg.RadarDetect.RDMode = RD_SWITCHING_MODE;
                        pPort->CommonCfg.RadarDetect.CSCount = 0;
                        DBGPRINT(RT_DEBUG_TRACE, ("APSetInformation: Channel change to ch%d, will do Channel-Switch-Announcement\n", pPort->Channel));
            }
            else
            {
                        if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
                        {
                            RadarDetectionStop(pAd);
            }
                        pPort->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;
                        pPort->CommonCfg.RadarDetect.CSCount = 0;
                        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters are changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                        APStop(pAd, pPort);
                        APStartUp(pAd, pPort);
        }
    }

                if (pPort->SoftAP.ApCfg.PhyMode == PHY_11VHT)
                {
                    DBGPRINT(RT_DEBUG_WARN,("%s, LINE_%d, to be implement !!\n",__FUNCTION__,__LINE__));
                }

                if (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    HTPhymode.PhyMode = pPort->CommonCfg.PhyMode;
                    HTPhymode.TransmitNo = (UCHAR)pAd->HwCfg.Antenna.field.TxPath;
                    HTPhymode.HtMode = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.HTMODE;
                    APABandGet40MChannel(pPort->Channel, &HTPhymode.ExtOffset);
                    HTPhymode.MCS = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.MCS;
                    HTPhymode.BW = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.BW;
                    HTPhymode.STBC = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.STBC;
                    HTPhymode.SHORTGI = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.ShortGI;

                    SetHtVht(pAd, pPort, &HTPhymode);       
                }

            } while (FALSE);
        }
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11PowerMgmtRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            if (*(PULONG)InformationBuffer > DOT11_POWER_SAVING_MAXIMUM_LEVEL)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            if((pPort->SoftAP.bAPStart == FALSE) &&
              (pAd->PortList[PORT_0]->PortSubtype != PORTSUBTYPE_P2PGO))
            {
                if(KeGetCurrentIrql() == DISPATCH_LEVEL)
                {                   
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_DOT11_POWER_MGMT_REQUEST, InformationBuffer, InformationBufferLength); 
                }
                else
                {
                    MlmeInfoPnpSetPowerMgmtMode(pAd, *(PULONG)InformationBuffer);
                }
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_POWER_MGMT_REQUEST(%u)\n", *(PULONG)InformationBuffer));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11ExcludedMacAddressList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        if (PortType == EXTSTA_PORT)
        {
            PDOT11_MAC_ADDRESS_LIST macList = (PDOT11_MAC_ADDRESS_LIST)InformationBuffer;
            DBGPRINT(RT_DEBUG_INFO, ("OID_DOT11_EXCLUDED_MAC_ADDRESS_LIST: uNumOfEntries       = %d\n", macList->uNumOfEntries));
            DBGPRINT(RT_DEBUG_INFO, ("uTotalNumOfEntries  = %d\n", macList->uTotalNumOfEntries));
    }

        do
    {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < FIELD_OFFSET(DOT11_MAC_ADDRESS_LIST, MacAddrs))
        {
                *BytesNeeded = FIELD_OFFSET(DOT11_MAC_ADDRESS_LIST, MacAddrs);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                ULONG i;
                BOOLEAN bDisconnect;
                PDOT11_MAC_ADDRESS_LIST pDot11MacAddrList = (PDOT11_MAC_ADDRESS_LIST)InformationBuffer;

                if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(pDot11MacAddrList->Header,  
                                                        NDIS_OBJECT_TYPE_DEFAULT,
                                                        DOT11_MAC_ADDRESS_LIST_REVISION_1,
                                                        sizeof(DOT11_MAC_ADDRESS_LIST)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                if (pDot11MacAddrList->uNumOfEntries > 0)
                {
                    //
                    // Ensure enough space for all the entries
                    //
                    *BytesRead =  FIELD_OFFSET(DOT11_MAC_ADDRESS_LIST, MacAddrs) + pDot11MacAddrList->uNumOfEntries * sizeof(DOT11_MAC_ADDRESS);
                    if (InformationBufferLength < (*BytesNeeded))
                    {
                        ndisStatus = NDIS_STATUS_INVALID_STATE;
                        break;
                    }
                }

                //
                // Can only support STA_EXCLUDED_MAC_ADDRESS_MAX_COUNT MAC addresses
                //
                if (pDot11MacAddrList->uNumOfEntries > STA_EXCLUDED_MAC_ADDRESS_MAX_COUNT)
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    break;
                }

                pAd->StaCfg.IgnoreAllMACAddresses = FALSE;
                PlatformMoveMemory(&pAd->StaCfg.ExcludedMACAddressList, &pDot11MacAddrList->MacAddrs[0], pDot11MacAddrList->uNumOfEntries * sizeof(DOT11_MAC_ADDRESS));
                pAd->StaCfg.ExcludedMACAddressCount = pDot11MacAddrList->uNumOfEntries;

                //
                // If only the broadcast MAC is present, ignore all MAC addresses
                //
                if ((pDot11MacAddrList->uNumOfEntries == 1) &&
                    ETH_IS_BROADCAST(pDot11MacAddrList->MacAddrs[0]))
                {
                    pAd->StaCfg.IgnoreAllMACAddresses = TRUE;
                }

                //
                // If the access point we are currently associated matches this MAC address, begin disassociation process
                //
                if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
                {
                    //
                    // Check if our current BSSID is in the new excluded MAC address list
                    //
                    if (pAd->StaCfg.IgnoreAllMACAddresses)
                    {
                        bDisconnect = TRUE;
            }
            else
            {
                        bDisconnect = FALSE;

                        // Walk through the excluded MAC address list
                        for (i = 0; i < pAd->StaCfg.ExcludedMACAddressCount; i++)
                        {
                            if (MAC_ADDR_EQUAL(pPort->PortCfg.Bssid, pAd->StaCfg.ExcludedMACAddressList[i]))
                            {
                                bDisconnect = TRUE;
            }
        }
    }

                    if (bDisconnect && (pAd->StaCfg.MicErrCnt < 2))
                    {

                        MlmeCntLinkDown(pPort, FALSE);

                        pAd->MlmeAux.AutoReconnectStatus = FALSE;
                        pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
                        PlatformIndicateDisassociation(pAd,pPort, pPort->PortCfg.Bssid, DOT11_DISASSOC_REASON_OS);
                        DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  OID_DOT11_EXCLUDED_MAC_ADDRESS_LIST, PlatformIndicateDisassociation Reason=0x%08x, [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                                    DOT11_ASSOC_STATUS_UNREACHABLE, pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], 
                                    pPort->PortCfg.Bssid[2], pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4], pPort->PortCfg.Bssid[5]));           
                    }
                }
                
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_EXCLUDED_MAC_ADDRESS_LIST \n"));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11ExcludeUnencrypted(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        if (PortType == EXTSTA_PORT)
        {
            DBGPRINT(RT_DEBUG_INFO, ("OID_DOT11_EXCLUDE_UNENCRYPTED\n"));
            DBGPRINT(RT_DEBUG_INFO, ("Exclude unencrypted = %d\n", *((PBOOLEAN)InformationBuffer)));
        }
        
        do
        {
            // EXPAND_FUNC NONE
            ULONG tmpPhymode = 0;
            
            // No need to check INIT_STATE
#if 0           
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_EXCLUDE_UNENCRYPTED failed on Invalid state\n"));
                break;
            }
#endif          

            if (InformationBufferLength < sizeof(BOOLEAN))
            {
                *BytesNeeded = sizeof(BOOLEAN);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else if ((pPort->PortType == WFD_GO_PORT) || (pPort->PortType == WFD_DEVICE_PORT))
            {           
                //GO always uses WPA2PSK+AES
                pPort->PortCfg.ExcludeUnencrypted = *(PBOOLEAN)InformationBuffer;
                *BytesRead = sizeof(BOOLEAN);
                DBGPRINT(RT_DEBUG_TRACE, ("GO mode Set::OID_DOT11_EXCLUDE_UNENCRYPTED (=%d)\n", pPort->PortCfg.ExcludeUnencrypted));
                break;
            }
            else
            {
                pPort->PortCfg.ExcludeUnencrypted = *(PBOOLEAN)InformationBuffer;

                if (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus) && (pPort->PortCfg.ExcludeUnencrypted == FALSE))
                {   
                    tmpPhymode = pPort->CommonCfg.InitPhyMode;
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_802_11_SET_PHY_MODE, &tmpPhymode, sizeof(ULONG));
                }
                
                *BytesRead = sizeof(BOOLEAN);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_EXCLUDE_UNENCRYPTED (=%s) wep =%d\n"
                    ,decodeCipherAlgorithm(pPort->PortCfg.WepStatus),pPort->PortCfg.ExcludeUnencrypted));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetExcludeUnEncrypt()
        pPort->PortCfg.ExcludeUnencrypted = *(PBOOLEAN)InformationBuffer;
        *BytesRead = sizeof(BOOLEAN);
        DBGPRINT(RT_DEBUG_TRACE, ("AP mode Set::OID_DOT11_EXCLUDE_UNENCRYPTED (=%d)\n", pPort->PortCfg.ExcludeUnencrypted));
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11DesiredBssidList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_DESIRED_BSSID_LIST failed on Invalid state\n"));
                break;
            }

            if (InformationBufferLength < FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                PDOT11_BSSID_LIST pDot11BSSIDList = (PDOT11_BSSID_LIST)InformationBuffer;

                if (pDot11BSSIDList->uNumOfEntries > 1)
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_DESIRED_BSSID_LIST failed, (uNumOfEntries=%d) we only support one BSSID\n",
                                        pDot11BSSIDList->uNumOfEntries));
                    break;
            }
                else
                {
                    //
                    // Ensure enough space for all the entries
                    //
                    *BytesNeeded = pDot11BSSIDList->uNumOfEntries * sizeof(DOT11_MAC_ADDRESS) + FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs);
                    if (InformationBufferLength < (*BytesNeeded))
                    {
                        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                        break;
        }

                    pPort->PortCfg.AcceptAnyBSSID = FALSE;
                    PlatformMoveMemory(pPort->PortCfg.DesiredBSSIDList, pDot11BSSIDList->BSSIDs, pDot11BSSIDList->uNumOfEntries * sizeof(DOT11_MAC_ADDRESS));
                    pPort->PortCfg.DesiredBSSIDCount = pDot11BSSIDList->uNumOfEntries;

                    //
                    // If only the broadcast MAC is present, accept any BSSID
                    //
                    if ((pDot11BSSIDList->uNumOfEntries == 1) && (ETH_IS_BROADCAST(pDot11BSSIDList->BSSIDs[0]) == TRUE))
                    {
                        pPort->PortCfg.AcceptAnyBSSID = TRUE;
    }
                    *BytesRead =  FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs) + pDot11BSSIDList->uNumOfEntries * sizeof(DOT11_MAC_ADDRESS);                  

                    // This set request is from the NDIS subsystem.
                    pAd->StaCfg.bSetDesiredBSSIDListFromRalinkUI = FALSE;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_DESIRED_BSSID_LIST (AcceptAnyBSSID=%d,NumofBSSID=%d,[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                        pPort->PortCfg.AcceptAnyBSSID, pPort->PortCfg.DesiredBSSIDCount, 
                        pPort->PortCfg.DesiredBSSIDList[0][0], pPort->PortCfg.DesiredBSSIDList[0][1], pPort->PortCfg.DesiredBSSIDList[0][2],
                        pPort->PortCfg.DesiredBSSIDList[0][3], pPort->PortCfg.DesiredBSSIDList[0][4], pPort->PortCfg.DesiredBSSIDList[0][5]));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11DesiredBssType(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
    {
            // EXPAND_FUNC NONE
            // No need to check INIT_STATE
#if 0           
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
        {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_DESIRED_BSS_TYPE failed on Invalid state\n"));
                break;
            }
#endif
            // When my ruel is GO and this is single P2P group mode, I am forced to use adhoc profile.  So return here.
            if (IS_P2P_STA_GO(pAd, pPort) && (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort)))
            {
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_DESIRED_BSS_TYPE return on Single P2P group as GO state\n"));

                break;
            }

            if (InformationBufferLength < sizeof(DOT11_BSS_TYPE))
            {
                *BytesNeeded = sizeof(DOT11_BSS_TYPE);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                DOT11_BSS_TYPE BSSType = *(PDOT11_BSS_TYPE)InformationBuffer;

                // the BSS type any corresponds to
                // infrastructure
                if (BSSType > dot11_BSS_type_any)
                {
                    *BytesNeeded = sizeof(DOT11_BSS_TYPE);
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
            }
                else if (BSSType == dot11_BSS_type_any)
                {
                    BSSType = dot11_BSS_type_infrastructure;
                }

                if (pAd->StaCfg.BssType != BSSType)
                {
                    pAd->StaCfg.BssType = BSSType;

                    // AdhocN Support:
                    // In case of Ad Hoc mode, the NIC uses BW 20 only.
                    // Note that the Ralink UI makes the MTK_OID_N6_SET_HT_PHYMODE request before 
                    // the OID_DOT11_DESIRED_BSS_TYPE request. Therefore, set BW 20 here.
                    if ((pAd->StaCfg.bAdhocNMode) &&
                        (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
                        (pPort->PortType == EXTSTA_PORT) && 
                            (pAd->StaCfg.BssType == dot11_BSS_type_independent))
                    {
                        // Vista:
                        // Banddwidth to HT setting is incorrect before BssType setup.
                        // Here add the constrain to BW by calling SetHtVht func.
                        OID_SET_HT_PHYMODE  HTPhyMode;

                        PlatformZeroMemory(&HTPhyMode, sizeof(OID_SET_HT_PHYMODE));
                        HTPhyMode.PhyMode = pPort->CommonCfg.PhyMode;
                        HTPhyMode.TransmitNo = (UCHAR)pAd->HwCfg.Antenna.field.TxPath;
                        HTPhyMode.HtMode = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.HTMODE;
                        HTPhyMode.ExtOffset = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.EXTCHA;
                        HTPhyMode.MCS = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.MCS;
                        HTPhyMode.BW = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.BW;
                        HTPhyMode.STBC = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.STBC;
                        HTPhyMode.SHORTGI = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.ShortGI;
                        // Don't need to store HTPhymode.VhtCentralChannel here, SetHtVht will decide it.

                        SetHtVht(pAd, pPort, &HTPhyMode);

        }

                    //
                    // Set default auth and cipher algorithms based on the new bss type
                    //
                    RTMPSetDefaultAuthAlgo(pAd,pPort);
    }

                *BytesRead = sizeof(DOT11_BSS_TYPE);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_DESIRED_BSS_TYPE (=%d)\n", BSSType));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11Statistics(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11EnabledAuthenticationAlgorithm(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        if(PortType == EXTSTA_PORT)
        {
            ULONG i = 0;
            PDOT11_AUTH_ALGORITHM_LIST  authAlgList = (PDOT11_AUTH_ALGORITHM_LIST)InformationBuffer;
            DBGPRINT(RT_DEBUG_INFO, ("OID_DOT11_ENABLED_AUTHENTICATION_ALGORITHM: uNumOfEntries       = %d\n", authAlgList->uNumOfEntries));
            DBGPRINT(RT_DEBUG_INFO, ("uTotalNumOfEntries  = %d\n", authAlgList->uTotalNumOfEntries));
            for (i= 0; i < authAlgList->uNumOfEntries; i++) 
            {
                DBGPRINT(RT_DEBUG_INFO, ("%-2d) %s\n", i, decodeAuthAlgorithm(authAlgList->AlgorithmIds[i])));
            }
    }

        do
    {
            // EXPAND_FUNC NONE
            PDOT11_AUTH_ALGORITHM_LIST pAuthAlgoList = (PDOT11_AUTH_ALGORITHM_LIST)InformationBuffer;
            DBGPRINT(RT_DEBUG_TRACE, ("Set::Enable Auth  (=%d)\n", pAuthAlgoList->AlgorithmIds[0]));                    
            if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortSubtype  == PORTSUBTYPE_P2PGO))
            {
                break;
            }

            if (InformationBufferLength < (FIELD_OFFSET(DOT11_AUTH_ALGORITHM_LIST, AlgorithmIds) + sizeof(DOT11_AUTH_ALGORITHM)))
        {
                *BytesNeeded = FIELD_OFFSET(DOT11_AUTH_ALGORITHM_LIST, AlgorithmIds);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(pAuthAlgoList->Header, 
                                          NDIS_OBJECT_TYPE_DEFAULT,
                                          DOT11_AUTH_ALGORITHM_LIST_REVISION_1,
                                          sizeof(DOT11_AUTH_ALGORITHM_LIST)))
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            // Must have atleast one entry in the list
            if (pAuthAlgoList->uNumOfEntries < 1)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }
            if (pPort->PortSubtype == PORTSUBTYPE_STA || 
                (pPort->PortSubtype == PORTSUBTYPE_P2PClient) ||
                (pPort->PortType == WFD_DEVICE_PORT) ||
                (pPort->PortType == WFD_GO_PORT) ||
                (pPort->PortType == WFD_CLIENT_PORT))
            {
                if (pPort->PortCfg.AuthMode != (ULONG)(pAuthAlgoList->AlgorithmIds[0]))
                {
                    if (pAuthAlgoList->AlgorithmIds[0] == VPORT_DOT11_AUTH_ALGO_RSNA_PSK)
                    {
                        pPort->PortCfg.AuthMode = Ralink802_11AuthModeWPA2PSK;
            }
                    else
                    {
                        pPort->PortCfg.AuthMode = pAuthAlgoList->AlgorithmIds[0];
        }
                    // reload enabled unicast and multicast cipher based on current bss type and auth algo.
                    RTMPSetDefaultCipher(pAd, pPort);
    }

                *BytesRead = FIELD_OFFSET(DOT11_AUTH_ALGORITHM_LIST, AlgorithmIds) + sizeof(DOT11_AUTH_ALGORITHM);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_ENABLED_AUTHENTICATION_ALGORITHM (pAd->OpMode == OPMODE_STA) (=%s)\n", decodeAuthAlgorithm(pAuthAlgoList->AlgorithmIds[0])));
            }
            else if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
            {

                if (pPort->SoftAP.ApCfg.ApPseudoStaAuthMode != (ULONG)(pAuthAlgoList->AlgorithmIds[0]))
                {
                    pPort->SoftAP.ApCfg.ApPseudoStaAuthMode = pAuthAlgoList->AlgorithmIds[0];
                    // reload enabled unicast and multicast cipher based on current bss type and auth algo.
                    RTMPSetDefaultCipher(pAd, pPort);
                }                           

                *BytesRead = FIELD_OFFSET(DOT11_AUTH_ALGORITHM_LIST, AlgorithmIds) + sizeof(DOT11_AUTH_ALGORITHM);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_ENABLED_AUTHENTICATION_ALGORITHM (pAd->OpMode == OPMODE_AP) (=%s)\n", decodeAuthAlgorithm(pAuthAlgoList->AlgorithmIds[0])));
            }
            else
    {
                DBGPRINT(RT_DEBUG_WARN, ("Set::OID_DOT11_ENABLED_AUTHENTICATION_ALGORITHM in incorrect type %d (=%s)\n",pPort->PortSubtype, decodeAuthAlgorithm(pAuthAlgoList->AlgorithmIds[0])));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetEnabledAuthenticationAlgorithm()
        PDOT11_AUTH_ALGORITHM_LIST  AuthAlgorithmList;
        ULONG                       requiredListSize = 0;
        DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledAuthenticationAlgorithm==>"));
        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(DOT11_AUTH_ALGORITHM_LIST))
    {
                DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledAuthenticationAlgorithm:invalid buffer size \n"));
                *BytesNeeded = sizeof(DOT11_AUTH_ALGORITHM_LIST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            AuthAlgorithmList = (PDOT11_AUTH_ALGORITHM_LIST)InformationBuffer;

            if (!GetRequiredListSize(
                            FIELD_OFFSET(DOT11_AUTH_ALGORITHM_LIST, AlgorithmIds), 
                            sizeof(DOT11_AUTH_ALGORITHM), 
                            AuthAlgorithmList->uNumOfEntries, 
                            &requiredListSize
                            ))
        {
                DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledAuthenticationAlgorithm:Invalid list size \n"));
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
                                AuthAlgorithmList->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_AUTH_ALGORITHM_LIST_REVISION_1,
                                sizeof(DOT11_AUTH_ALGORITHM_LIST)
                                ))
            {
                DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledAuthenticationAlgorithm:Invalid header \n"));
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            // Set is allowed only in AP INIT state
            VALIDATE_AP_INIT_STATE(pPort);

            //
            // Only supports one auth algorithm
            //
            if (AuthAlgorithmList->uNumOfEntries != 1)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledAuthenticationAlgorithm:Invalid uNumOfEntries \n"));
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
        }

            //
            // Validate auth algorithm
            //
            if ((AuthAlgorithmList->AlgorithmIds[0] != DOT11_AUTH_ALGO_80211_OPEN) &&
                (AuthAlgorithmList->AlgorithmIds[0] != DOT11_AUTH_ALGO_RSNA_PSK))
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledAuthenticationAlgorithm:Invalid auth algorithm (=%x)\n", AuthAlgorithmList->AlgorithmIds[0]));
                break;
    }

            pPort->PortCfg.AuthMode = AuthAlgorithmList->AlgorithmIds[0];
            DBGPRINT(RT_DEBUG_TRACE,("SET Auth Mode = %s\n", decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));

            //
            // Set default ciphers
            //
            APSetDefaultCipher(pPort);

            *BytesRead = requiredListSize;

        } while (FALSE);

        if(ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledAuthenticationAlgorithm:Invalid state = %d \n",pPort->ApState));
        }
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11SupportedUnicastAlgorithmPair(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11SupportedMulticastAlgorithmPair(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11EnabledUnicastCipherAlgorithm(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        if(PortType == EXTSTA_PORT)
        {
            ULONG i;
            PDOT11_CIPHER_ALGORITHM_LIST    ciphList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;
            DBGPRINT(RT_DEBUG_INFO, ("OID_DOT11_ENABLED_MULTICAST_CIPHER_ALGORITHM: uNumOfEntries       = %d\n", ciphList->uNumOfEntries));
            DBGPRINT(RT_DEBUG_INFO, ("uTotalNumOfEntries  = %d\n", ciphList->uTotalNumOfEntries));
            for (i= 0; i < ciphList->uNumOfEntries; i++) 
            {
                DBGPRINT(RT_DEBUG_INFO, ("%-2d) %s\n", i, decodeCipherAlgorithm(ciphList->AlgorithmIds[i])));
            }
        }

        do
        {
            // EXPAND_FUNC NONE
            ULONG tmpPhymode = 0;
            
            // No need to check INIT_STATE
#if 0           
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_ENABLED_UNICAST_CIPHER_ALGORITHM failed on Invalid state\n"));
                break;
            }
#endif          

            if (InformationBufferLength < FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                if (pPort->PortSubtype == PORTSUBTYPE_STA || 
                    (pPort->PortSubtype == PORTSUBTYPE_P2PClient) ||
                    (pPort->PortType == WFD_DEVICE_PORT) ||
                    (pPort->PortType == WFD_GO_PORT) ||
                    (pPort->PortType == WFD_CLIENT_PORT))
                {
                    PDOT11_CIPHER_ALGORITHM_LIST pCipherAlgoList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;

                    if (pCipherAlgoList->AlgorithmIds[0] == VPORT_DOT11_CIPHER_ALGO_CCMP)
                    {
                        pPort->PortCfg.WepStatus = Ralink802_11Encryption3Enabled;
                        pPort->PortCfg.OrigWepStatus = Ralink802_11Encryption3Enabled;
                        pPort->PortCfg.PairCipher    = Ralink802_11Encryption3Enabled;
                        pPort->PortCfg.GroupCipher   = Ralink802_11Encryption3Enabled;
                    }
                    else
                    {
                        pPort->PortCfg.WepStatus = pCipherAlgoList->AlgorithmIds[0];
                        pPort->PortCfg.OrigWepStatus = pCipherAlgoList->AlgorithmIds[0];
                        pPort->PortCfg.PairCipher    = pCipherAlgoList->AlgorithmIds[0];
                        pPort->PortCfg.GroupCipher   = pCipherAlgoList->AlgorithmIds[0];
                    }
                    pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;

                    if (pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_CCMP)
                    {
                        //SetAsicWcidAttri.Cipher= CIPHER_AES;
                        pPort->PortCfg.CipherAlg = CIPHER_AES;                      
                    }
                    else if (pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_TKIP)
                    {
                        //SetAsicWcidAttri.Cipher = CIPHER_TKIP;
                        pPort->PortCfg.CipherAlg = CIPHER_TKIP;
                    }
                    else if (pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP40)
                    {
                        //SetAsicWcidAttri.Cipher = CIPHER_WEP64;
                        pPort->PortCfg.CipherAlg = CIPHER_WEP64;
                    }
                    else if (pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP104)
                    {
                        //SetAsicWcidAttri.Cipher = CIPHER_WEP128;
                        pPort->PortCfg.CipherAlg = CIPHER_WEP128;
                    }
                    else
                    {
                        pPort->PortCfg.CipherAlg = CIPHER_NONE;
                        //SetAsicWcidAttri.Cipher = CIPHER_NONE;
                    }

                    *BytesRead = FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds) + sizeof(DOT11_CIPHER_ALGORITHM);

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_ENABLED_UNICAST_CIPHER_ALGORITHM (WepStatus=%s)\n", decodeCipherAlgorithm(pPort->PortCfg.WepStatus)));

                    // GO always uses WPA2PSK+AES, no need to change phymode to legacy mode.
                    if ((pPort->PortType == WFD_GO_PORT) || (pPort->PortType == WFD_DEVICE_PORT))
                    {
                        break;
                    }

                    //If we want to prohibit TKIP algo. on 11n, change phymode to legacy mode.
                    if ((pPort->CommonCfg.bProhibitTKIPonHT)&&(MlmeValidateSSID(pPort->PortCfg.DesiredSSID.ucSSID, (UCHAR)pPort->PortCfg.DesiredSSID.uSSIDLength)))
                    {
                        if ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||
                            (pPort->PortCfg.CipherAlg == CIPHER_TKIP)||
                            (pPort->PortCfg.CipherAlg == CIPHER_TKIP_NO_MIC))
                        {
                            //If we use TKIP encryption algo. and the value of "ProhibitTKIPonHT" registry is true, disable Adhoc N Mode.
                            if (pAd->StaCfg.BssType == BSS_ADHOC)
                            {
                                pAd->StaCfg.bAdhocNMode = ADHOCNMODE_DISABLE;
                            }

                            if (pPort->CommonCfg.PhyMode < PHY_11ABGN_MIXED)
                            {
                                //Keep PhyMode.
                            }
                            else
                            {
                                switch (pPort->CommonCfg.PhyMode)
                                {
                                    case PHY_11ABGN_MIXED:
                                    case PHY_11VHT: 
                                        tmpPhymode = PHY_11ABG_MIXED;
                                        break;
                                    case PHY_11N:
                                        tmpPhymode = PHY_11G;
                                        break;
                                    case PHY_11GN_MIXED:
                                        tmpPhymode = PHY_11G;
                                        break;
                                    case PHY_11AN_MIXED:
                                        tmpPhymode = PHY_11A;
                                        break;
                                    case PHY_11BGN_MIXED:
                                        tmpPhymode = PHY_11BG_MIXED;
                                        break;
                                    case PHY_11AGN_MIXED:
                                        tmpPhymode = PHY_11ABG_MIXED;
                                        break;
                                }
                                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_802_11_SET_PHY_MODE, &tmpPhymode, sizeof(ULONG));
                            }
                        }
                        else
                        {
                            tmpPhymode = pPort->CommonCfg.InitPhyMode;
                            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_802_11_SET_PHY_MODE, &tmpPhymode, sizeof(ULONG));

                            //Using any encryption algo. except TKIP, we must set the bAdhocNMode to the value of "AdhocNMode" registry.
                            if (pAd->StaCfg.BssType == BSS_ADHOC)
                            {
                                pAd->StaCfg.bAdhocNMode = pAd->StaCfg.bInitAdhocNMode;
                            }
                        }
                    }
                }
                else
                {
                    // do nothing.
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_ENABLED_UNICAST_CIPHER_ALGORITHM (pAd->OpMode == OPMODE_AP) (do nothing)\n"));
                }
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetEnabledUnicastCipherAlgorithm()
        PDOT11_CIPHER_ALGORITHM_LIST    CipherAlgorithmList;
        ULONG                           requiredListSize = 0;

        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(DOT11_CIPHER_ALGORITHM_LIST))
            {
                *BytesNeeded = sizeof(DOT11_CIPHER_ALGORITHM_LIST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            CipherAlgorithmList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;
            if (!GetRequiredListSize(
                        FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds), 
                        sizeof(DOT11_CIPHER_ALGORITHM), 
                        CipherAlgorithmList->uNumOfEntries, 
                        &requiredListSize
                        ))
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

            CipherAlgorithmList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;

            if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                            CipherAlgorithmList->Header,
                            NDIS_OBJECT_TYPE_DEFAULT,
                            DOT11_CIPHER_ALGORITHM_LIST_REVISION_1,
                            sizeof(DOT11_CIPHER_ALGORITHM_LIST)
                            ))
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pPort = pAd->PortList[PortNumber];
            
            // Set is allowed only in AP INIT state
            VALIDATE_AP_INIT_STATE(pPort);

            //
            // Only supports one cipher algorithm
            //
            if (CipherAlgorithmList->uNumOfEntries != 1)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            //
            // Validate auth/cipher pair
            //
            if (!ApValidateUnicastAuthCipherPair(pPort->PortCfg.AuthMode, CipherAlgorithmList->AlgorithmIds[0]))
            {
                //
                // Invalid cipher algorithm
                //
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                DBGPRINT(RT_DEBUG_ERROR, ("Invalid unicast cipher algorithm (0x%08x) for auth algorithm (0x%08x). \n",
                                    CipherAlgorithmList->AlgorithmIds[0], pPort->PortCfg.AuthMode));
                break;
            }

            pPort->PortCfg.UnicastCipherAlgorithm = CipherAlgorithmList->AlgorithmIds[0];
            pPort->PortCfg.WepStatus = CipherAlgorithmList->AlgorithmIds[0];
            DBGPRINT(RT_DEBUG_TRACE,("SET UnicastCipherAlgorithm = WepStatus = %s\n", decodeCipherAlgorithm(CipherAlgorithmList->AlgorithmIds[0])));
            
            *BytesRead = requiredListSize;
            
            // Avoid double settings on AP RSN_IE
            if ((P2P_OFF(pPort)) ||  (pAd->OpMode != OPMODE_STAP2P))
                RTMPAPMakeRSNIE(pAd,pPort,pPort->PortCfg.AuthMode,pPort->PortCfg.WepStatus);
        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11EnabledMulticastCipherAlgorithm(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        if (PortType == EXTSTA_PORT)
        {
            ULONG i = 0;
            PDOT11_CIPHER_ALGORITHM_LIST    ciphList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;
            DBGPRINT(RT_DEBUG_INFO, ("OID_DOT11_ENABLED_MULTICAST_CIPHER_ALGORITHM: uNumOfEntries       = %d\n", ciphList->uNumOfEntries));
            DBGPRINT(RT_DEBUG_INFO, ("uTotalNumOfEntries  = %d\n", ciphList->uTotalNumOfEntries));
            for (i= 0; i < ciphList->uNumOfEntries; i++) 
            {
                DBGPRINT(RT_DEBUG_INFO, ("%-2d) %s\n", i, decodeCipherAlgorithm(ciphList->AlgorithmIds[i])));
            }
    }

        do
    {
            // EXPAND_FUNC NONE
            ULONG index = 0;
            
            // No need to check INIT_STATE
#if 0           
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_ENABLED_MULTICAST_CIPHER_ALGORITHM failed on Invalid state\n"));
                break;
            }
#endif
            if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            {
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_ENABLED_MULTICAST_CIPHER_ALGORITHM return in p2p mode.\n"));
                break;
            }

            if (InformationBufferLength < FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
        }
            else
            {
                PDOT11_CIPHER_ALGORITHM_LIST pCipherAlgoList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;

                if (InformationBufferLength < FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds))
                {
                    *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds);
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
    }

                if (pCipherAlgoList->uNumOfEntries < 1)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                *BytesNeeded = pCipherAlgoList->uNumOfEntries * sizeof(DOT11_CIPHER_ALGORITHM) +
                                    FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds);

                if (InformationBufferLength < *BytesNeeded)
                {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
                }

                if (pCipherAlgoList->uNumOfEntries > STA_MULTICAST_CIPHER_MAX_COUNT)
    {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    break;
    }

                if (pCipherAlgoList->uNumOfEntries == 1)
    {
                    pPort->PortCfg.MulticastCipherAlgorithm = pCipherAlgoList->AlgorithmIds[0];
                }
                else
        {
                    pPort->PortCfg.MulticastCipherAlgorithm = DOT11_CIPHER_ALGO_NONE;
                }

                pPort->PortCfg.MulticastCipherAlgorithmCount = pCipherAlgoList->uNumOfEntries;
                for (index = 0; index < pCipherAlgoList->uNumOfEntries; index++)
            {
                    pPort->PortCfg.MulticastCipherAlgorithmList[index] = pCipherAlgoList->AlgorithmIds[index];
            }

                if (pPort->PortCfg.MulticastCipherAlgorithm == VPORT_DOT11_CIPHER_ALGO_CCMP)
            {
                    pPort->PortCfg.MulticastCipherAlgorithm = Ralink802_11Encryption3Enabled;
            }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_ENABLED_MULTICAST_CIPHER_ALGORITHM (=%d)\n", pPort->PortCfg.MulticastCipherAlgorithm));
        }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetEnabledMulticastCipherAlgorithm()
        PDOT11_CIPHER_ALGORITHM_LIST    CipherAlgorithmList;
        ULONG                           requiredListSize = 0;

        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(DOT11_CIPHER_ALGORITHM_LIST))
            {
                *BytesNeeded = sizeof(DOT11_CIPHER_ALGORITHM_LIST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            CipherAlgorithmList = (PDOT11_CIPHER_ALGORITHM_LIST)InformationBuffer;

            if (!GetRequiredListSize(
                        FIELD_OFFSET(DOT11_CIPHER_ALGORITHM_LIST, AlgorithmIds), 
                        sizeof(DOT11_CIPHER_ALGORITHM), 
                        CipherAlgorithmList->uNumOfEntries, 
                        &requiredListSize
                        ))
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
                        CipherAlgorithmList->Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_CIPHER_ALGORITHM_LIST_REVISION_1,
                        sizeof(DOT11_CIPHER_ALGORITHM_LIST)
                        ))
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
    }

            pPort = pAd->PortList[PortNumber];

            // Set is allowed only in AP INIT state
            VALIDATE_AP_INIT_STATE(pPort);

            //
            // Only supports one cipher algorithm
            //
            if (CipherAlgorithmList->uNumOfEntries != 1)
    {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
    }

            //
            // Validate auth/cipher pair
            //
            if (!ApValidateMulticastAuthCipherPair(pPort->PortCfg.AuthMode, CipherAlgorithmList->AlgorithmIds[0]))
            {
                //
                // Invalid cipher algorithm
                //
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetEnabledMulticastCipherAlgorithm:Invalid multicast cipher algorithm (0x%08x) for auth algorithm (0x%08x). \n",
                                CipherAlgorithmList->AlgorithmIds[0], pPort->PortCfg.AuthMode));
                break;
    }

            pPort->PortCfg.MulticastCipherAlgorithm = CipherAlgorithmList->AlgorithmIds[0];
            pPort->PortCfg.GroupKeyWepStatus = CipherAlgorithmList->AlgorithmIds[0];
            DBGPRINT(RT_DEBUG_TRACE,("SET MulticastCipherAlgorithm = GroupKeyWepStatus = %s\n", decodeCipherAlgorithm(CipherAlgorithmList->AlgorithmIds[0])));
            *BytesRead = requiredListSize;      

        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CipherDefaultKeyId(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                ULONG KeyIdx = *((PULONG)InformationBuffer);
                
                if (KeyIdx >= 4)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    DBGPRINT(RT_DEBUG_ERROR, ("OID_DOT11_CIPHER_DEFAULT_KEY_ID, Inalid KeyIdx(=%d)\n", KeyIdx));
                    break;
                }

                pPort->PortCfg.DefaultKeyId = (UCHAR) KeyIdx;
                if (IS_P2P_GO_WPA2PSKING(pPort)  && (pAd->OpMode == OPMODE_STA))
                    pPort->PortCfg.DefaultKeyId = 1;

                if(KeGetCurrentIrql() == DISPATCH_LEVEL)
                {
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_STA_UPDTAE_DEFAULT_ID,&PortNumber,sizeof(NDIS_PORT_NUMBER));    
                }
                else
                {
                    MlmeInfoStaOidSetWEPDefaultKeyID(pPort);                    
                }
    
                *BytesRead = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CIPHER_DEFAULT_KEY_ID (=%d) \n", KeyIdx));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC ApOidSetDefaultKeyID()
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                ULONG KeyIdx = *((PULONG)InformationBuffer);
                
                if (KeyIdx >= 4)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    DBGPRINT(RT_DEBUG_ERROR, ("OID_DOT11_CIPHER_DEFAULT_KEY_ID, Inalid KeyIdx(=%d)\n", KeyIdx));
                    break;
                }

                pPort->PortCfg.DefaultKeyId = (UCHAR) KeyIdx;
                
                *BytesRead = sizeof(ULONG);
                *BytesNeeded = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_ERROR, ("AP Set::OID_DOT11_CIPHER_DEFAULT_KEY_ID (=%d) \n", KeyIdx));
            }
        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CipherDefaultKey(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        if(PortType == EXTSTA_PORT)
        {
            DBGPRINT(RT_DEBUG_INFO, ("*** OID_DOT11_CIPHER_DEFAULT_KEY ***\n"));
    }

        do
    {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_CIPHER_DEFAULT_KEY, Invalid keyLen=%d, need=%d\n", InformationBufferLength, *BytesNeeded));
                break;
            }
            else
            {
                PDOT11_CIPHER_DEFAULT_KEY_VALUE pDefaultKey = (PDOT11_CIPHER_DEFAULT_KEY_VALUE)InformationBuffer;

                *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey) + pDefaultKey->usKeyLength;
                if (InformationBufferLength < *BytesNeeded)
                {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_CIPHER_DEFAULT_KEY, Invalid keyLen=%d, need=%d\n", InformationBufferLength, *BytesNeeded));       
                    break;
        }

                // It is hard to synchronize security key between STA and AP especially for session timeout(EAPOL frame is been encrypted).
                // In heavy traffic, we cannot make sure next packets should use old key or new key.
                // So, just block SendPackets(just bypass EAPOL frame) till HW updates new key.
                DBGPRINT(RT_DEBUG_TRACE, ("OID_DOT11_CIPHER_DEFAULT_KEY: Set fRTMP_ADAPTER_ADDKEY_IN_PROGRESS flag\n"));
                MT_SET_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);

                if (ADHOC_ON(pPort) && (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) && !MAC_ADDR_EQUAL(pDefaultKey->MacAddr, ZERO_MAC_ADDR))
                {
                    //
                    // Use SW to Decrypt & Encrypt for DefaultKey(GroupKey)
                    //
                    MlmeInfoAddPerStaDefaultKey(pAd,
                                            pPort,
                                            pDefaultKey->MacAddr,
                                            (UCHAR)pDefaultKey->uKeyIndex,
                                            pDefaultKey->AlgorithmId,
                                            pDefaultKey->usKeyLength,
                                            pDefaultKey->ucKey);
    }
                else
                {
                    if(KeGetCurrentIrql() == DISPATCH_LEVEL)
                    {
                        UCHAR   InBuffer[256] = {0};
                        ULONG   InBufferLen = 0;

                        // Fill inbuffer with port number and InformationBuffer
                        PlatformZeroMemory(InBuffer, sizeof(InBuffer));
                        InBufferLen = 0;
                        PlatformMoveMemory(&InBuffer[0], &PortNumber, sizeof(NDIS_PORT_NUMBER));
                        InBufferLen += sizeof(NDIS_PORT_NUMBER);
                        PlatformMoveMemory(&InBuffer[sizeof(NDIS_PORT_NUMBER)], InformationBuffer, InformationBufferLength);
                        InBufferLen += InformationBufferLength;                 
                        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_ADD_STA_DEFAULT_KEY, InBuffer/*InformationBuffer*/, InBufferLen/*InformationBufferLength*/);
                    }   
                    else
                    {
                        PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
               
                        MlmeInfoAddKey(pAd,
                                pPort,
                                pDefaultKey->MacAddr,
                                (UCHAR)pDefaultKey->uKeyIndex,
                                pDefaultKey->AlgorithmId,
                                pDefaultKey->usKeyLength,
                                pDefaultKey->ucKey,
                                pDefaultKey->bDelete,
                                FALSE,
                                (pPort->PortType == WFD_GO_PORT));

                        if(pBssidMacTabEntry != NULL)
                        {
                            P2pWPADone(pAd, pPort, pBssidMacTabEntry, FALSE);
                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("auth mode = %s\n",decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
                    DBGPRINT(RT_DEBUG_TRACE, ("MAC = %x,%x,%x,%x,%x,%x\n",pDefaultKey->MacAddr[0]
                        ,pDefaultKey->MacAddr[1],pDefaultKey->MacAddr[2],pDefaultKey->MacAddr[3]
                        ,pDefaultKey->MacAddr[4],pDefaultKey->MacAddr[5]));
                }

                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);             
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CIPHER_DEFAULT_KEY\n"));

                // If Addkey in progress, we will block send packet. So, we may have packets in SendTxWaitQueue.
                DBGPRINT(RT_DEBUG_TRACE, ("SendTxWaitQueue Num[%d]\n",pAd->pTxCfg->SendTxWaitQueue[0].Number));
                if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && 
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
    {
                    NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetDefaultKey()
        do
        {
            if (InformationBufferLength < FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_CIPHER_DEFAULT_KEY, Invalid keyLen=%d, need=%d\n", InformationBufferLength, *BytesNeeded));
                break;
            }
            else
            {
                PDOT11_CIPHER_DEFAULT_KEY_VALUE pDefaultKey = (PDOT11_CIPHER_DEFAULT_KEY_VALUE)InformationBuffer;

                *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey) + pDefaultKey->usKeyLength;
                if (InformationBufferLength < *BytesNeeded)
                {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_CIPHER_DEFAULT_KEY, Invalid keyLen=%d, need=%d\n", InformationBufferLength, *BytesNeeded));       
                    break;
            }

                // It is hard to synchronize security key between STA and AP especially for session timeout(EAPOL frame is been encrypted).
                // In heavy traffic, we cannot make sure next packets should use old key or new key.
                // So, just block SendPackets(just bypass EAPOL frame) till HW updates new key.
                DBGPRINT(RT_DEBUG_TRACE, ("OID_DOT11_CIPHER_DEFAULT_KEY: Set fRTMP_ADAPTER_ADDKEY_IN_PROGRESS flag\n"));
                MT_SET_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);
                if(KeGetCurrentIrql() == DISPATCH_LEVEL)
                {
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_ADD_AP_DEFAULT_KEY, InformationBuffer, InformationBufferLength);    

        }
                else
                {
                    RTMPApAddKey(pAd,
                            pPort,
                            pDefaultKey->MacAddr,
                            (UCHAR)pDefaultKey->uKeyIndex,
                            pDefaultKey->AlgorithmId,
                            pDefaultKey->usKeyLength,
                            pDefaultKey->ucKey,
                            pDefaultKey->bDelete,
                            FALSE,
                            TRUE);                  
    }
                    DBGPRINT(RT_DEBUG_TRACE, ("auth mode = %s\n",decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
                    DBGPRINT(RT_DEBUG_TRACE, ("MAC = %x,%x,%x,%x,%x,%x\n",pDefaultKey->MacAddr[0]
                        ,pDefaultKey->MacAddr[1],pDefaultKey->MacAddr[2],pDefaultKey->MacAddr[3]
                        ,pDefaultKey->MacAddr[4],pDefaultKey->MacAddr[5]));


                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);         
                *BytesNeeded = *BytesRead;
                DBGPRINT(RT_DEBUG_TRACE, ("AP Set::OID_DOT11_CIPHER_DEFAULT_KEY\n"));
            }
        }while(FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11CipherKeyMappingKey(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        if(PortType == EXTSTA_PORT)
        {
            DBGPRINT(RT_DEBUG_INFO, ("*** OID_DOT11_CIPHER_KEY_MAPPING_KEY ***\n"));
    }

        do
    {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_BYTE_ARRAY))
            {
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                ULONG   TotalLength;
                ULONG   size;
                PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE pKeyMappingKeys;
                PDOT11_BYTE_ARRAY KeyData = (PDOT11_BYTE_ARRAY)InformationBuffer;
                NDIS_STATUS         AddKeyStatus = NDIS_STATUS_SUCCESS;

                *BytesRead = FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer) + KeyData->uNumOfBytes;

                if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(KeyData->Header, 
                                                        NDIS_OBJECT_TYPE_DEFAULT,
                                                        DOT11_CIPHER_KEY_MAPPING_KEY_VALUE_BYTE_ARRAY_REVISION_1,
                                                        sizeof(DOT11_BYTE_ARRAY)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                if (InformationBufferLength < *BytesNeeded)
    {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
    }

                // It is hard to synchronize security key between STA and AP especially for session timeout(EAPOL frame is been encrypted).
                // In heavy traffic, we cannot make sure next packets should use old key or new key.
                // So, just block SendPackets(just bypass EAPOL frame) till HW updates new key.
                DBGPRINT(RT_DEBUG_TRACE, ("OID_DOT11_CIPHER_KEY_MAPPING_KEY: Set fRTMP_ADAPTER_ADDKEY_IN_PROGRESS flag\n"));
                MT_SET_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);

                TotalLength = KeyData->uNumOfBytes;
                pKeyMappingKeys = (PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE) KeyData->ucBuffer;
                while (TotalLength  >= FIELD_OFFSET(DOT11_CIPHER_KEY_MAPPING_KEY_VALUE, ucKey))
    {
                    size = FIELD_OFFSET(DOT11_CIPHER_KEY_MAPPING_KEY_VALUE, ucKey) + pKeyMappingKeys->usKeyLength;
                    if (TotalLength < size)
        {
                        ndisStatus = NDIS_STATUS_INVALID_DATA;
                        break;
                    }

                    DBGPRINT_RAW(RT_DEBUG_TRACE, ("PeerMacAddr = %2x:%2x:%2x:%2x:%2x:%2x",pKeyMappingKeys->PeerMacAddr[0],
                                    pKeyMappingKeys->PeerMacAddr[1],pKeyMappingKeys->PeerMacAddr[2],pKeyMappingKeys->PeerMacAddr[3],pKeyMappingKeys->PeerMacAddr[4],
                                    pKeyMappingKeys->PeerMacAddr[5]));                      
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                        UCHAR   InBuffer[256] = {0};
                        ULONG   InBufferLen = 0;

                        // Fill inbuffer with port number and InformationBuffer
                        PlatformZeroMemory(InBuffer, sizeof(InBuffer));
                        InBufferLen = 0;
                        PlatformMoveMemory(&InBuffer[0], &PortNumber, sizeof(NDIS_PORT_NUMBER));
                        InBufferLen += sizeof(NDIS_PORT_NUMBER);
                        PlatformMoveMemory(&InBuffer[sizeof(NDIS_PORT_NUMBER)], InformationBuffer, InformationBufferLength);
                        InBufferLen += InformationBufferLength;
                        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_ADD_STA_MAPPING_KEY, InBuffer/*InformationBuffer*/, InBufferLen/*InformationBufferLength*/);
            }
            else
            {

                        AddKeyStatus = MlmeInfoAddKey(pAd, 
                                pPort,
                                pKeyMappingKeys->PeerMacAddr,
                                0,
                                pKeyMappingKeys->AlgorithmId,
                                pKeyMappingKeys->usKeyLength,
                                pKeyMappingKeys->ucKey,
                                pKeyMappingKeys->bDelete,
                                TRUE,
                                TRUE);
#ifdef MULTI_CHANNEL_SUPPORT
                        if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortNumber == PORT_0))
                        {
                            // This flag will assign STA data to HCCA during new session phase
                            OPSTATUS_SET_FLAG(pPort, fOP_STATUS_INFRA_OP);
                            
                            // CLI connectes to peer GO and cehck STA/GO status
                            // Defer a task to handle multi-channel state and DHCP
                            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_NEW_CONNECTION_START);
                            PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, 100);   
                        }
#endif /*MULTI_CHANNEL_SUPPORT*/

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                        // Update P2pMs status and info during adding the pairwise key
                        if (NDIS_WIN8_ABOVE(pAd) && 
                            ((pPort->PortType == WFD_CLIENT_PORT) || (pPort->PortType == WFD_GO_PORT)) &&
                            (AddKeyStatus == NDIS_STATUS_SUCCESS))
                        {
                            P2pMsWPADone(pAd, pPort, pKeyMappingKeys->PeerMacAddr);
                        }
#endif
            }

                    TotalLength -= size;
                    pKeyMappingKeys = Add2Ptr(pKeyMappingKeys, size);
        }
                *BytesRead = *BytesNeeded;

                // Indicate to request pairwise master key identifiers (PMKIDs) for basic service set (BSS) identifiers (BSSIDs) 
                // that the 802.11 station can potentially roam to.
                MlmeCntCheckPMKIDCandidate(pPort);

                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);
 
#if 0
                if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && 
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
                {
                    UCHAR   Index;
                    for (Index = 0; Index < 4; Index++)
                    {
                        if(pAd->pTxCfg->SendTxWaitQueue[Index].Number > 0)
                        {
                            NdisCommonDeQueuePacket(pAd, Index);
    }
                    }

                    // Kick bulk out
                    N6USBKickBulkOut(pAd);
                }       
#endif
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CIPHER_KEY_MAPPING_KEY\n"));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetKeyMappingKey()
        do
        {
            if (InformationBufferLength < sizeof(DOT11_BYTE_ARRAY))
            {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
            }
            else
            {
                ULONG TotalLength;
                ULONG size;
                PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE pKeyMappingKeys;
                PDOT11_BYTE_ARRAY KeyData = (PDOT11_BYTE_ARRAY)InformationBuffer;

                *BytesRead = FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer) + KeyData->uNumOfBytes;

                if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(KeyData->Header, 
                                                        NDIS_OBJECT_TYPE_DEFAULT,
                                                        DOT11_CIPHER_KEY_MAPPING_KEY_VALUE_BYTE_ARRAY_REVISION_1,
                                                        sizeof(DOT11_BYTE_ARRAY)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                if (InformationBufferLength < *BytesNeeded)
    {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
    }

                // It is hard to synchronize security key between STA and AP especially for session timeout(EAPOL frame is been encrypted).
                // In heavy traffic, we cannot make sure next packets should use old key or new key.
                // So, just block SendPackets(just bypass EAPOL frame) till HW updates new key.
                DBGPRINT(RT_DEBUG_TRACE, ("OID_DOT11_CIPHER_KEY_MAPPING_KEY: Set fRTMP_ADAPTER_ADDKEY_IN_PROGRESS flag\n"));
                MT_SET_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);

                TotalLength = KeyData->uNumOfBytes;
                pKeyMappingKeys = (PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE) KeyData->ucBuffer;
                while (TotalLength  >= FIELD_OFFSET(DOT11_CIPHER_KEY_MAPPING_KEY_VALUE, ucKey))
    {
                    size = FIELD_OFFSET(DOT11_CIPHER_KEY_MAPPING_KEY_VALUE, ucKey) + pKeyMappingKeys->usKeyLength;
                    if (TotalLength < size)
        {
                        ndisStatus = NDIS_STATUS_INVALID_DATA;
                        break;
                    }

                    DBGPRINT_RAW(RT_DEBUG_TRACE, ("PeerMacAddr = %2x:%2x:%2x:%2x:%2x:%2x",pKeyMappingKeys->PeerMacAddr[0],
                                    pKeyMappingKeys->PeerMacAddr[1],pKeyMappingKeys->PeerMacAddr[2],pKeyMappingKeys->PeerMacAddr[3],pKeyMappingKeys->PeerMacAddr[4],
                                    pKeyMappingKeys->PeerMacAddr[5]));                      
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_ADD_AP_MAPPING_KEY, InformationBuffer, InformationBufferLength);    
            }
            else
            {
                        RTMPApAddKey(pAd, 
                                pPort,
                                pKeyMappingKeys->PeerMacAddr,
                                0,
                                pKeyMappingKeys->AlgorithmId,
                                pKeyMappingKeys->usKeyLength,
                                pKeyMappingKeys->ucKey,
                                pKeyMappingKeys->bDelete,
                                TRUE,
                                TRUE);
    }

                    TotalLength -= size;
                    pKeyMappingKeys = Add2Ptr(pKeyMappingKeys, size);
                }
                *BytesRead = *BytesNeeded;

                // Indicate to request pairwise master key identifiers (PMKIDs) for basic service set (BSS) identifiers (BSSIDs) 
                // that the 802.11 station can potentially roam to.
                //NEED TO  IMPLEMENT FOR AP MODE
                //MlmeCntCheckPMKIDCandidate(pAd);

                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS);             
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_CIPHER_KEY_MAPPING_KEY\n"));
            }
        }while(FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11MediaStreamingEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(BOOLEAN))
            {
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                pPort->CommonCfg.bMediaStreamingEnabled = *(PBOOLEAN)InformationBuffer;
                *BytesRead = sizeof(BOOLEAN);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_MEDIA_STREAMING_ENABLED (=%d)\n", pPort->CommonCfg.bMediaStreamingEnabled));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11UnreachableDetectionThreshold(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            }
            else
            {
                pPort->CommonCfg.UnreachableDetectionThreshold = *(PULONG)InformationBuffer;
                *BytesRead = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_UNREACHABLE_DETECTION_THRESHOLD (=%d)\n", pPort->CommonCfg.UnreachableDetectionThreshold));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11MaximumListSize(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
N6SetOidDot11PrivacyExemptionList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
    {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
        {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_PRIVACY_EXEMPTION_LIST failed on Invalid state\n"));
                break;
            }

            if (InformationBufferLength < FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                PDOT11_PRIVACY_EXEMPTION_LIST ExemptionList = (PDOT11_PRIVACY_EXEMPTION_LIST)InformationBuffer;

                if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(ExemptionList->Header, 
                                                        NDIS_OBJECT_TYPE_DEFAULT,
                                                        DOT11_PRIVACY_EXEMPTION_LIST_REVISION_1,
                                                        sizeof(DOT11_PRIVACY_EXEMPTION_LIST)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
            }

                *BytesNeeded = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries) +
                                ExemptionList->uNumOfEntries * sizeof(DOT11_PRIVACY_EXEMPTION);

                if (InformationBufferLength < (*BytesNeeded))
                {
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
        }
                    
                if (pPort->PortCfg.PrivacyExemptionList)
                {
                    PlatformFreeMemory(pPort->PortCfg.PrivacyExemptionList, pPort->PortCfg.PrivacyExemptionListLen);
                    pPort->PortCfg.PrivacyExemptionList = NULL;
            pPort->PortCfg.PrivacyExemptionListLen=0;
    }

                PlatformAllocateMemory(pAd,  &pPort->PortCfg.PrivacyExemptionList, InformationBufferLength);

                pPort->PortCfg.PrivacyExemptionList->uTotalNumOfEntries = ExemptionList->uTotalNumOfEntries;
                pPort->PortCfg.PrivacyExemptionList->uNumOfEntries = ExemptionList->uNumOfEntries;
        pPort->PortCfg.PrivacyExemptionListLen = InformationBufferLength;           
                if (ExemptionList->uNumOfEntries)
            {
                    PlatformMoveMemory(pPort->PortCfg.PrivacyExemptionList->PrivacyExemptionEntries,
                                    ExemptionList->PrivacyExemptionEntries,
                                    ExemptionList->uNumOfEntries * sizeof(DOT11_PRIVACY_EXEMPTION));
        }

                *BytesRead = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries) + ExemptionList->uNumOfEntries * sizeof(DOT11_PRIVACY_EXEMPTION);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_PRIVACY_EXEMPTION_LIST (NumEntry=%d,Type=0x%x,Action=%d,Packet=%d)\n",
                            ExemptionList->uNumOfEntries, ExemptionList->PrivacyExemptionEntries[0].usEtherType, 
                            ExemptionList->PrivacyExemptionEntries[0].usExemptionActionType, ExemptionList->PrivacyExemptionEntries[0].usExemptionPacketType));
    }
        } while (FALSE);
            }
    else // EXTAP_PORT
            {
        // EXPAND_FUNC ApOidSetPrivacyExemptionList()
        PDOT11_PRIVACY_EXEMPTION_LIST   PrivacyExemptionList;
        ULONG                           requiredListSize = 0;
        PDOT11_PRIVACY_EXEMPTION_LIST   tmpExemptionList = NULL;

        do
    {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
    }

            PrivacyExemptionList = (PDOT11_PRIVACY_EXEMPTION_LIST)InformationBuffer;

            if (!GetRequiredListSize(
                        FIELD_OFFSET(DOT11_PRIVACY_EXEMPTION_LIST, PrivacyExemptionEntries), 
                        sizeof(DOT11_PRIVACY_EXEMPTION), 
                        PrivacyExemptionList->uNumOfEntries, 
                        &requiredListSize
                        ))
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
                        PrivacyExemptionList->Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_PRIVACY_EXEMPTION_LIST_REVISION_1,
                        sizeof(DOT11_PRIVACY_EXEMPTION_LIST)
                        ))
        {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pPort = pAd->PortList[PortNumber];

            if (pPort->PortType == EXTAP_PORT)
            {
                // Set is allowed only in AP INIT state
                VALIDATE_AP_INIT_STATE(pPort);
        }

            //
            // Allocate memory for the new list
            //
             PlatformAllocateMemory(pAd, &tmpExemptionList, requiredListSize);
        pPort->PortCfg.PrivacyExemptionListLen = requiredListSize;  
            if (tmpExemptionList == NULL)
    {
                ndisStatus = NDIS_STATUS_RESOURCES;
                DBGPRINT(RT_DEBUG_ERROR, ("ApOidSetPrivacyExemptionList:Failed to allocate %d bytes for the exemption list.\n", requiredListSize));                 
                break;
    }

            PlatformMoveMemory(tmpExemptionList, PrivacyExemptionList, requiredListSize);

            //APCleanupPrivacyExemptionList(pAd);
            Ndis6CommonPortFreePrivacyExemptionList(pPort);

            pPort->PortCfg.PrivacyExemptionList = tmpExemptionList;

            *BytesRead = requiredListSize;

        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N6SetOidDot11IbssParams(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_IBSS_PARAMS))
            {
                *BytesNeeded = sizeof(DOT11_IBSS_PARAMS);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                PVOID           pIBSSParams = NULL;
                PDOT11_IBSS_PARAMS pDot11IBSSParams = (PDOT11_IBSS_PARAMS)InformationBuffer;

                if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(pDot11IBSSParams->Header, 
                                                        NDIS_OBJECT_TYPE_DEFAULT,
                                                        DOT11_IBSS_PARAMS_REVISION_1,
                                                        sizeof(DOT11_IBSS_PARAMS)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                //
                // Verify IE blob length
                //
                if ((pDot11IBSSParams->uIEsOffset + pDot11IBSSParams->uIEsLength) > InformationBufferLength)
                {
                    *BytesNeeded = pDot11IBSSParams->uIEsOffset + pDot11IBSSParams->uIEsLength;
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
                }

                if (pDot11IBSSParams->uIEsLength > 0)
                {
                    PlatformAllocateMemory(pAd,  &pIBSSParams, pDot11IBSSParams->uIEsLength);
                    if (pIBSSParams == NULL)
                    {
                        *BytesRead = sizeof(DOT11_IBSS_PARAMS);
                        ndisStatus = NDIS_STATUS_RESOURCES;
                        break;
                    }

                    PlatformMoveMemory(pIBSSParams, Add2Ptr(pDot11IBSSParams, pDot11IBSSParams->uIEsOffset), pDot11IBSSParams->uIEsLength);

                    if (pPort->PortCfg.AdditionalBeaconIEData)
                    {
                        PlatformFreeMemory(pPort->PortCfg.AdditionalBeaconIEData, pPort->PortCfg.AdditionalBeaconIESize );
                        pPort->PortCfg.AdditionalBeaconIEData = NULL;
                        pPort->PortCfg.AdditionalBeaconIESize = 0;
                    }
                    
                    pPort->PortCfg.AdditionalBeaconIEData = pIBSSParams;
                    pPort->PortCfg.AdditionalBeaconIESize = pDot11IBSSParams->uIEsLength;
                }

                //always set this flag
                pAd->StaCfg.IBSSJoinOnly = pDot11IBSSParams->bJoinOnly;
                if (pAd->StaCfg.IBSSJoinOnly == FALSE)
                {
                    //in case if first time no SSID exist in scan table
                    pAd->StaCfg.bIbssScanOnce = TRUE;
                }
                
                *BytesRead = pDot11IBSSParams->uIEsLength + pDot11IBSSParams->uIEsOffset;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_IBSS_PARAMS (Size=%d),JoinOnly=%d\n", 
                    pDot11IBSSParams->uIEsLength,pDot11IBSSParams->bJoinOnly));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N6SetOidDot11PmkidList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC RTMPSetPMKIDList()
        PDOT11_PMKID_LIST pPMKIDList = NULL;
        ULONG index1 = 0, index2 = 0;
        ULONG i = 0;

        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs))
            {
                DBGPRINT_ERR(("%s: NDIS_STATUS_BUFFER_OVERFLOW #1\n", __FUNCTION__));
                
                *BytesNeeded = FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            pPMKIDList = (PDOT11_PMKID_LIST)InformationBuffer;

            if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(pPMKIDList->Header, 
                      NDIS_OBJECT_TYPE_DEFAULT,
                      DOT11_PMKID_LIST_REVISION_1,
                      sizeof(DOT11_PMKID_LIST)))
            {
                DBGPRINT_ERR(("%s: NDIS_STATUS_INVALID_DATA #1\n", __FUNCTION__));
                
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            if (pPMKIDList->uNumOfEntries < 1)
            {
                DBGPRINT_ERR(("%s: NDIS_STATUS_INVALID_DATA #2\n", __FUNCTION__));
                
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            // Verify size.
            *BytesNeeded = (pPMKIDList->uNumOfEntries * sizeof(DOT11_PMKID_ENTRY)) +
                            FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs);

            if (InformationBufferLength < *BytesNeeded)
            {
                DBGPRINT_ERR(("%s:NDIS_STATUS_BUFFER_OVERFLOW #2 \n", __FUNCTION__));
                
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            // The value of the uNumOfEntries member cannot be greater than the value of uPMKIDCacheSize 
            // that the driver previously returned through a query of OID_DOT11_EXTSTA_CAPABILITY.
            if ((pPMKIDList->uNumOfEntries > STA_PMKID_MAX_COUNT) || (pPMKIDList->uNumOfEntries < 1))
            {
                DBGPRINT_ERR(("%s: NDIS_STATUS_INVALID_LENGTH\n", __FUNCTION__));
                
                *BytesRead = FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            *BytesRead = FIELD_OFFSET(DOT11_PMKID_LIST, PMKIDs) + 
                                  (pPMKIDList->uNumOfEntries * sizeof(DOT11_PMKID_ENTRY));

            // Clear PMKID cache first.
            pAd->StaCfg.PMKIDCount = 0;
            PlatformZeroMemory(pAd->StaCfg.PMKIDList, (sizeof(DOT11_PMKID_ENTRY) * STA_PMKID_MAX_COUNT));

            // Copy the PMKID list.
            pAd->StaCfg.PMKIDCount = pPMKIDList->uNumOfEntries;
            PlatformMoveMemory(pAd->StaCfg.PMKIDList,
                            pPMKIDList->PMKIDs,
                            (pAd->StaCfg.PMKIDCount * sizeof(DOT11_PMKID_ENTRY)));

            // All BSSIDs have to be in the desired BSSID list.
            if (!pPort->PortCfg.AcceptAnyBSSID)
            {
                BOOLEAN bMatchInDesiredBSSIDList = FALSE;
                
                for (index1 = 0; index1 < pAd->StaCfg.PMKIDCount; index1++)
                {
                    for (index2 = 0; index2 < pAd->StaCfg.PMKIDCount; index2++)
                    {
                        if (MAC_ADDR_EQUAL(pAd->StaCfg.PMKIDList[index1].BSSID,
                            pPort->PortCfg.DesiredBSSIDList[index2]))
                        {
                            // This BSSID is in the desired BSSID list.
                            bMatchInDesiredBSSIDList = TRUE;
                            break;
                        }
                    }

                    // This BSSID is not in the desired BSSID list.
                    if (index2 == pPort->PortCfg.DesiredBSSIDCount)
                    {
                        if (index1 != (pAd->StaCfg.PMKIDCount - 1))
                        {
                            // Replaced by the last PMKID candidate.
                            PlatformMoveMemory((pAd->StaCfg.PMKIDList + index1),
                                            (pAd->StaCfg.PMKIDList +pAd->StaCfg.PMKIDCount - 1),
                                            sizeof(DOT11_PMKID_ENTRY));

                            // Check the current index again (the last PMKID candidate)
                            index1--;
                        }

                        pAd->StaCfg.PMKIDCount--;
                    }
                }

                // None of the entries in the PMKID list has a BSSID member that matches an entry 
                // in the miniport driver's desired BSSID list.
                // (support single desired BSSID now.)
                if ((bMatchInDesiredBSSIDList == FALSE) && (pPort->PortCfg.DesiredBSSIDCount == 1))
                {
                    DBGPRINT_ERR(("%s: NDIS_STATUS_INVALID_DATA #3\n", __FUNCTION__));
                    
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }
            }

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
        } while(FALSE);
        
        DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_PMKID_LIST\n"));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N6SetOidDot11UnicastUseGroupEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_UNICAST_USE_GROUP_ENABLED failed on Invalid state\n"));
                break;
            }

            if (InformationBufferLength < sizeof(BOOLEAN))
            {
                *BytesNeeded = sizeof(BOOLEAN);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;          
            }
            else
            {
                pPort->CommonCfg.bUnicastUseGroupEnabled = *(PBOOLEAN)InformationBuffer;
                *BytesRead = sizeof(BOOLEAN);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_UNICAST_USE_GROUP_ENABLED (=%d)\n", pPort->CommonCfg.bUnicastUseGroupEnabled));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N6SetOidDot11HiddenNetworkEnabled(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_HIDDEN_NETWORK_ENABLED failed on Invalid state\n"));
                break;
            }

            if (InformationBufferLength < sizeof(BOOLEAN))
            {
                *BytesNeeded = sizeof(BOOLEAN);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                pPort->PortCfg.bHiddenNetworkEnabled = *((PBOOLEAN)InformationBuffer);
                *BytesRead = sizeof(BOOLEAN);
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N6SetOidDot11QosParams(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (!MP_VERIFY_STATE(pPort, INIT_STATE))
            {
                ndisStatus = NDIS_STATUS_INVALID_STATE;
                break;
            }
            
            if (InformationBufferLength < sizeof(DOT11_QOS_PARAMS))
            {
                *BytesNeeded = sizeof(DOT11_QOS_PARAMS);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                PDOT11_QOS_PARAMS PQosParams = (PDOT11_QOS_PARAMS)InformationBuffer;
                if( PQosParams != NULL )
                {                       
                    if( PQosParams->ucEnabledQoSProtocolFlags&DOT11_QOS_PROTOCOL_FLAG_WMM )
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_QOS_PARAMS[DOT11_QOS_PROTOCOL_FLAG_WMM]\n"));
                        pPort->CommonCfg.SafeModeQOS_PARAMS |= DOT11_QOS_PROTOCOL_FLAG_WMM;
                    }
                    if( PQosParams->ucEnabledQoSProtocolFlags&DOT11_QOS_PROTOCOL_FLAG_11E )
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_QOS_PARAMS[DOT11_QOS_PROTOCOL_FLAG_11E]\n")); 
                        pPort->CommonCfg.SafeModeQOS_PARAMS |= DOT11_QOS_PROTOCOL_FLAG_11E;                       
                    }
                    if( PQosParams->ucEnabledQoSProtocolFlags == 0 )
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_QOS_PARAMS[Disable Qos]\n"));
                        pPort->CommonCfg.SafeModeQOS_PARAMS = 0;
                    }
                    *BytesRead = sizeof(DOT11_QOS_PARAMS);
                }
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N6SetOidDot11AvailableChannelList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N6SetOidDot11PortStateNotification(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    // EXPAND_FUNC NONE
    if (PortType == EXTSTA_PORT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("RTMPEventSetInformation:: ************** OID_DOT11_PORT_STATE_NOTIFICATION ***, InformationBufferLength=%d\n", InformationBufferLength));
    }
    
    // EXPAND_FUNC ApOidSetPortStateNotification()
    //\\//\\Optional (Not Finished)
    do
    {
        PDOT11_PORT_STATE_NOTIFICATION PortStateNotification = NULL;
        *BytesRead = 0;
        *BytesNeeded = 0;

        if (InformationBufferLength < sizeof(DOT11_PORT_STATE_NOTIFICATION))
        {
            *BytesNeeded = sizeof(DOT11_PORT_STATE_NOTIFICATION);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        PortStateNotification = (PDOT11_PORT_STATE_NOTIFICATION)InformationBuffer;
        
        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                    PortStateNotification->Header,
                    NDIS_OBJECT_TYPE_DEFAULT,
                    DOT11_PORT_STATE_NOTIFICATION_REVISION_1,
                    sizeof(DOT11_PORT_STATE_NOTIFICATION)))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        //VALIDATE_AP_OP_STATE(pPort);

        
        //ApSetPortStateNotification
        //\\//\\Need Implement
        
        if (NDIS_STATUS_SUCCESS == ndisStatus)
        {
            *BytesRead = sizeof(DOT11_PORT_STATE_NOTIFICATION);

            DBGPRINT(RT_DEBUG_TRACE, ("Port(%u) Set::OID_DOT11_PORT_STATE_NOTIFICATION (bOpen=%d, PeerMac = %2x:%2x:%2x:%2x:%2x:%2x)\n", 
                PortNumber, PortStateNotification->bOpen, 
                PortStateNotification->PeerMac[0], PortStateNotification->PeerMac[1],
                PortStateNotification->PeerMac[2], PortStateNotification->PeerMac[3], 
                PortStateNotification->PeerMac[4], PortStateNotification->PeerMac[5]));
        }

    } while (FALSE);
    

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidGenPhysicalMedium(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidGenMaximumLookahead(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidGenCurrentPacketFilter(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(ULONG))
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(ULONG);
                break;
            }
            else
            {
                ULONG PacketFilter = *(PULONG)InformationBuffer;
                pPort->PortCfg.PacketFilter = PacketFilter;             
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_GEN_CURRENT_PACKET_FILTER(=0x%08x\n", PacketFilter));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetPacketFilter()
        ULONG PacketFilter;
        
        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            PacketFilter = *(PULONG)InformationBuffer;
            //
            // Any bits not supported?
            //
            if (PacketFilter & ~NDIS_PACKET_TYPE_ALL_802_11_FILTERS)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pPort->PortCfg.PacketFilter = PacketFilter;

            *BytesRead = sizeof(ULONG);

        } while (FALSE);
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidGenCurrentLookahead(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof (ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                pAd->pNicCfg->LookAhead = *(PULONG)InformationBuffer;
                *BytesRead = sizeof(ULONG);
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidGenProtocolOptions(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOid8023MulticastList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC N6Dot11_SetMulticastList()
        //
        // Initialize.
        //
        *BytesNeeded = sizeof(DOT11_MAC_ADDRESS);
        *BytesRead = 0;

        do 
        {
            //if size equal zero then clear mcastlist table
            if( InformationBufferLength == 0)
            {               
                pPort->PortCfg.MCAddressCount  = 0;
                PlatformZeroMemory(pPort->PortCfg.MCAddressList, DOT11_ADDRESS_SIZE * HW_MAX_MCAST_LIST_SIZE);
                break;
            }
            
            if (InformationBufferLength % sizeof(DOT11_MAC_ADDRESS)) 
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            //
            // Verify that we can hold the multicast list
            //
            if (InformationBufferLength > (HW_MAX_MCAST_LIST_SIZE * sizeof(DOT11_MAC_ADDRESS))) 
            {
                ndisStatus = NDIS_STATUS_MULTICAST_FULL;
                *BytesNeeded = HW_MAX_MCAST_LIST_SIZE * sizeof(DOT11_MAC_ADDRESS);
                break;
            }

            *BytesRead = InformationBufferLength;

            if (InformationBufferLength > 0)
            {
                pPort->PortCfg.MCAddressCount = InformationBufferLength/DOT11_ADDRESS_SIZE;

                PlatformMoveMemory(pPort->PortCfg.MCAddressList, InformationBuffer, InformationBufferLength);
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidDot11MulticastList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC N6Dot11_SetMulticastList()
        //
        // Initialize.
        //
        *BytesNeeded = sizeof(DOT11_MAC_ADDRESS);
        *BytesRead = 0;

        do 
        {
            //if size equal zero then clear mcastlist table
            if( InformationBufferLength == 0)
            {               
                pPort->PortCfg.MCAddressCount  = 0;
                PlatformZeroMemory(pPort->PortCfg.MCAddressList, DOT11_ADDRESS_SIZE * HW_MAX_MCAST_LIST_SIZE);
                break;
            }
            
            if (InformationBufferLength % sizeof(DOT11_MAC_ADDRESS)) 
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            //
            // Verify that we can hold the multicast list
            //
            if (InformationBufferLength > (HW_MAX_MCAST_LIST_SIZE * sizeof(DOT11_MAC_ADDRESS))) 
            {
                ndisStatus = NDIS_STATUS_MULTICAST_FULL;
                *BytesNeeded = HW_MAX_MCAST_LIST_SIZE * sizeof(DOT11_MAC_ADDRESS);
                break;
            }

            *BytesRead = InformationBufferLength;

            if (InformationBufferLength > 0)
            {
                pPort->PortCfg.MCAddressCount = InformationBufferLength/DOT11_ADDRESS_SIZE;

                PlatformMoveMemory(pPort->PortCfg.MCAddressList, InformationBuffer, InformationBufferLength);
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidGenInterruptModeration(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidGenLinkParameters(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC MpSetLinkParameters()
        PNDIS_LINK_PARAMETERS   pLinkParams;

        do
        {
            *BytesRead = 0;
            *BytesNeeded = 0;

            if (InformationBufferLength < sizeof(NDIS_LINK_PARAMETERS))
            {
                *BytesNeeded = sizeof(NDIS_LINK_PARAMETERS);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }

            pLinkParams = (PNDIS_LINK_PARAMETERS)InformationBuffer;

            if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(pLinkParams->Header, 
                                              NDIS_OBJECT_TYPE_DEFAULT,
                                              NDIS_LINK_PARAMETERS_REVISION_1,
                                              NDIS_SIZEOF_LINK_PARAMETERS_REVISION_1))
    //                                        sizeof(NDIS_LINK_PARAMETERS)))
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            //
            // TODO: HwPlatformIndicateNewLinkSpeed
            //

            DBGPRINT(RT_DEBUG_TRACE, ("==> !!!MpSetLinkParameters\n"));
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

NDIS_STATUS
N5SetOidPmParameters(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            ndisStatus = NDIS_STATUS_SUCCESS;
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
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}   

