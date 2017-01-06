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
    AP_oids_Set.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

NDIS_STATUS
MtkOidSetPhyMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                ULONG phymode = *(ULONG *) pInformationBuffer;
                PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
                
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_PHY_MODE (=%s)\n", DecodePhyMode(phymode)));
                
                // Init BssTable to avoid reporting wrong channel APs 
                BssTableInit(&pAd->ScanTab);
                ResetRepositBssTable(pAd);

                if(pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
                {
                    // Since TKIP would restrict the phymode to BG, reset the cipher mode to prevent mis-setting of phymode.
                    DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Reset Cipher to Open-NONE\n",__FUNCTION__,__LINE__));
                    pPort->PortCfg.WepStatus = Ralink802_11EncryptionDisabled;
                }               
                pPort->CommonCfg.InitPhyMode = (UCHAR)phymode;
                MlmeInfoSetPhyMode(pAd, pPort, phymode,FALSE);

                // Note that when the NIC supports the N mode, the Ralink UI firstly sets
                // the MTK_OID_N6_SET_PHY_MODE and then MTK_OID_N6_SET_HT_PHYMODE.
                // Therefore, the NIC should only restarts the SoftAP during it receives the MTK_OID_N6_SET_HT_PHYMODE.
                // On other hand, if the NIC does not support N mode, it should restart the SoftAP during
                // it receives the MTK_OID_N6_SET_PHY_MODE (the Ralink UI does not set MTK_OID_N6_SET_HT_PHYMODE).
                if ((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) &&
                      (phymode < PHY_11ABGN_MIXED))
                {
                    //RestartAPIsRequired = TRUE;
                }
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetCountryRegion(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);   
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                UCHAR   TmpPhy;
                ULONG   country = *(ULONG*)pInformationBuffer;
                
                // The multi-phase channel scanning method does not clean the scan table at once.
                // The miniport driver has to clean up the scan table when the country region is changed.
                // Note that the Ralink UI always makes the MTK_OID_N5_SET_COUNTRY_REGION object identifier before the BSSID connection.
                // Therefore, the miniport driver should clean the scan table only if the Ralink UI intends to change the country region.
                // In addition, when the user changes the wireless mode, the Ralink UI makes the MTK_OID_N5_SET_COUNTRY_REGION object identifier with the same country region, 
                // the miniport driver also has to clean up the scan table if the PHY mode is changed (see MTK_OID_N5_SET_PHY_MODE).
                if ((pPort->CommonCfg.CountryRegion != (UCHAR)(country & 0x000000FF)) || 
                     (pPort->CommonCfg.CountryRegionForABand != (UCHAR)((country >> 8) & 0x000000FF)))
                {
                    // Init BssTable to avoid reporting wrong channel APs 
                    BssTableInit(&pAd->ScanTab);
                    ResetRepositBssTable(pAd);
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N5_SET_COUNTRY_REGION (Clean the scan table)\n"));
                }

                pPort->CommonCfg.CountryRegion = (UCHAR)(country & 0x000000FF);
                pPort->CommonCfg.CountryRegionForABand = (UCHAR)((country >> 8) & 0x000000FF);
                TmpPhy = pPort->CommonCfg.PhyMode;

                // Never set phymode to 0xff. If you want to update channel list,  turn on paramater3 in MlmeInfoSetPhyMode routine. -- MlmeInfoSetPhyMode(xxx, xxx,TRUE)
                // pPort->CommonCfg.PhyMode = 0xff;
                // Build all corresponding channel information
                MlmeInfoSetPhyMode(pAd, pPort, TmpPhy, TRUE);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_COUNTRY_REGION (A:%d  B/G:%d)\n",pPort->CommonCfg.CountryRegionForABand, pPort->CommonCfg.CountryRegion));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTxPowerLevel1(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);   
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                ULONG PowerTemp = *(ULONG *) pInformationBuffer;
                if (PowerTemp > 100)
                    PowerTemp = 0xffffffff;  // AUTO

                pPort->CommonCfg.TxPowerDefault = PowerTemp;
                pPort->CommonCfg.TxPowerPercentage = PowerTemp;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_TX_POWER_LEVEL_1 (=0x%08x)\n", PowerTemp));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetHardwareRegister(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(RT_802_11_CR_ACCESS))
            {
                *BytesNeeded = sizeof(RT_802_11_CR_ACCESS); 
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
#if (defined(_WIN64) || DBG)            
                ndisStatus = SetHardWareRegister(pAd, pInformationBuffer);
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetResetCounters(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            PlatformZeroMemory(&pAd->Counter.WlanCounters, sizeof(COUNTER_802_11));
            PlatformZeroMemory(&pAd->Counter.Counters8023, sizeof(COUNTER_802_3));
            PlatformZeroMemory(&pAd->Counter.MTKCounters, sizeof(COUNTER_MTK));
            
            pPort->CommonCfg.RxSTBCCount = pPort->CommonCfg.TxSTBCCount = 0;

            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_RESET_COUNTERS \n"));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetConfiguration(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
                    PNDIS_802_11_CONFIGURATION  pConfig = (PNDIS_802_11_CONFIGURATION)pInformationBuffer;

                    pPort->CommonCfg.BeaconPeriod = (USHORT) pConfig->BeaconPeriod;
                    MAP_KHZ_TO_CHANNEL_ID(pConfig->DSConfig, pPort->Channel);
                    pPort->CentralChannel = pPort->Channel;

                    //RestartAPIsRequired = TRUE;
                    
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_CONFIGURATION (BeacnPeriod=%d,AtimW=%d,Ch=%d)\n",
                                        pConfig->BeaconPeriod, pConfig->ATIMWindow, pPort->Channel));
                }
            } while (FALSE);
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(NDIS_802_11_CONFIGURATION))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    PNDIS_802_11_CONFIGURATION pConfig = (PNDIS_802_11_CONFIGURATION)pInformationBuffer;
                    UCHAR       OriginalChannel = pPort->Channel;

                    // Both the MTK_OID_N6_SET_CONFIGURATION OID and channel scannng functionality modify the pPort->ScaningChannel.
                    if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
                        MlmeRestartStateMachine(pAd,pPort);
                        
                    }
        
                    if ((pConfig->BeaconPeriod >= 20) && (pConfig->BeaconPeriod <= 400))
                        pPort->CommonCfg.BeaconPeriod = (USHORT) pConfig->BeaconPeriod;
                    pAd->StaCfg.AtimWin = (USHORT) pConfig->ATIMWindow;
                    MAP_KHZ_TO_CHANNEL_ID(pConfig->DSConfig, pPort->Channel);
                    
                    //Make sure the select channel is a valid channel
                    if(!MlmeSyncIsValidChannel(pAd, pPort->Channel))
                        pPort->Channel = MlmeSyncFirstChannel(pAd);
                  
                    //
                    // Save the channel on MlmeAux for MlmeCntlOidRTBssidProc used.
                    //
                    pPort->ScaningChannel = pPort->Channel;

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_CONFIGURATION (BeacnPeriod=%d,AtimW=%d,Ch=%d)\n",
                        pConfig->BeaconPeriod, pConfig->ATIMWindow, pPort->Channel));

                    // Config has changed
                    pAd->pNicCfg->bConfigChanged = TRUE;

                    if ((pAd->UiCfg.bPromiscuous == TRUE) && (pPort->BBPCurrentBW == BW_40))
                    {
                        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                    }
                    else if ((pAd->UiCfg.bPromiscuous == TRUE) && (pPort->BBPCurrentBW == BW_20))
                    {
                        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetAuthenticationMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(DOT11_AUTH_ALGORITHM)) 
                {
                    *BytesNeeded = sizeof(DOT11_AUTH_ALGORITHM);    
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_AUTHENTICATION_MODE (Invalid information buffer length)\n"));
        }
        else
        {
                    DOT11_AUTH_ALGORITHM AuthMode = *(PDOT11_AUTH_ALGORITHM)pInformationBuffer;
                    switch (AuthMode)
            {
                        case Ralink802_11AuthModeOpen:
                        case Ralink802_11AuthModeShared:
                        case Ralink802_11AuthModeAutoSwitch:
                        case Ralink802_11AuthModeWPANone:
                        case Ralink802_11AuthModeWPA:
                        case Ralink802_11AuthModeWPAPSK:
                        case Ralink802_11AuthModeWPA2:
                        case Ralink802_11AuthModeWPA2PSK:
                        case Ralink802_11AuthModeWPA1WPA2:
                        case Ralink802_11AuthModeWPA1PSKWPA2PSK:
                        pPort->PortCfg.AuthMode = AuthMode;
                            break;
                        default:
                            ndisStatus = NDIS_STATUS_INVALID_DATA;      
                            break;
                    }
                }
        
                pHeader = &pPort->MacTab.MacTabList;
                pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                while (pNextMacEntry != NULL)
                {
                    pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                    
                    if(pMacEntry == NULL)
                    {
                        break; 
                    }
                    
                    if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
                    {
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                        continue; 
                    }

                    if (pMacEntry->ValidAsCLI)
                    {
                        pMacEntry->PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
                    }
                    
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                }

                pPort->PortCfg.DefaultKeyId = 0;
                if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
                {
                    pPort->SoftAP.ApCfg.WpaGTKState = SETKEYS;
                    pPort->SoftAP.ApCfg.GKeyDoneStations = pPort->MacTab.Size;
                    pPort->PortCfg.DefaultKeyId = 1;
                }
                else
                {
                    // do nothing.
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_AUTHENTICATION_MODE (=%s) (pPort->PortCfg.DefaultKeyId=%d) \n", 
                    decodeAuthAlgorithm(pPort->PortCfg.AuthMode), pPort->PortCfg.DefaultKeyId));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetEncryptionStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    ULONG tmpPhymode = 0;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(DOT11_CIPHER_ALGORITHM))
                {
                    *BytesNeeded = sizeof(DOT11_CIPHER_ALGORITHM);  
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ENCRYPTION_STATUS (Invalid information buffer length)\n"));
                }
                else
                {
                    DOT11_CIPHER_ALGORITHM  WepStatus = *(PDOT11_CIPHER_ALGORITHM)pInformationBuffer;

                    // Since TKIP, AES, WEP are all supported. It should not have any invalid setting.
                    if (WepStatus >= Ralink802_11Encryption2Enabled)
                    {
                        pPort->PortCfg.DefaultKeyId = 1;                   
                    }
                    if (WepStatus <= Ralink802_11Encryption4Enabled)
                    {
                        pPort->PortCfg.WepStatus = WepStatus;
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ENCRYPTION_STATUS: pPort->PortCfg.WepStatus = %s\n", decodeCipherAlgorithm(pPort->PortCfg.WepStatus)));
                    }
                    else
                    {   
                                ndisStatus = NDIS_STATUS_INVALID_DATA;
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ENCRYPTION_STATUS: (NDIS_STATUS_INVALID_DATA)\n"));
                                break;
                    }
                }
                if(pPort->CommonCfg.bProhibitTKIPonHT)
                {
                    if ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||
                        (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)||
                        ( pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled))
                    {
                        if(pPort->CommonCfg.PhyMode < PHY_11ABGN_MIXED)
                        {
                            //Keep PhyMode.
                        }
                        else
                        {
                            switch(pPort->CommonCfg.PhyMode)
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
                                default:
                                    tmpPhymode = pPort->CommonCfg.PhyMode;
                            }
                            DBGPRINT(RT_DEBUG_TRACE,("Enqueue MTK_OID_N5_SET_PHY_MODE(phymode = %s)",DecodePhyMode(tmpPhymode)));
                            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_802_11_SET_PHY_MODE, &tmpPhymode, sizeof(ULONG));
                        }
                    }
                    else
                    {
                        tmpPhymode = pPort->CommonCfg.InitPhyMode;
                        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_802_11_SET_PHY_MODE, &tmpPhymode, sizeof(ULONG));
                    }
                }

                // modify RSN IE
                ApWpaMakeRSNIE(pAd,  pPort->PortCfg.AuthMode, pPort->PortCfg.WepStatus);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ENCRYPTION_STATUS (=%s)\n", decodeCipherAlgorithm(pPort->PortCfg.WepStatus)));
                
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetRemoveKey(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    PNDIS_802_11_REMOVE_KEY pRemoveKey;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            do
            {
                // EXPAND_FUNC NONE
                if(pInformationBuffer == NULL)
                {
                    DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
                    return NDIS_STATUS_INVALID_DATA;
                }
                pRemoveKey = (PNDIS_802_11_REMOVE_KEY) pInformationBuffer;
                if (InformationBufferLength != pRemoveKey->Length)
                {
                    *BytesNeeded = pRemoveKey->Length;  
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    ULONG   KeyIdx = pRemoveKey->KeyIndex;
        
                    if (KeyIdx & 0x80000000)
                    {
                        // Should never set default bit when remove key
                        ndisStatus = NDIS_STATUS_INVALID_DATA;
                    }
                    else
                    {
                        KeyIdx = KeyIdx & 0x0fffffff;
                        if (KeyIdx >= 4)
                        {
                            ndisStatus = NDIS_STATUS_INVALID_DATA;
                        }
                        else
                        {
                            pPort->SharedKey[BSS0][KeyIdx].KeyLen = 0;
                            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
                            AsicRemoveSharedKeyMode(pAd, BSS0, (UCHAR) KeyIdx);
                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_REMOVE_KEY (id=0x%x, Len=%d-byte)\n", pRemoveKey->KeyIndex, pRemoveKey->Length));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetAddKey(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    PNDIS_802_11_KEY pKey;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            do
            {
                // EXPAND_FUNC NONE
                if(pInformationBuffer == NULL)
                {
                    DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
                    return NDIS_STATUS_INVALID_DATA;
                }
                pKey = (PNDIS_802_11_KEY) pInformationBuffer;
                if (InformationBufferLength != pKey->Length)
                {
                    *BytesNeeded = pKey->Length;
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
                    {
                        if(pKey->KeyLength == 32)
                        {
                            PlatformMoveMemory(pPort->SoftAP.ApCfg.PMK, pKey->KeyMaterial, 32);
                            DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_ADD_KEY::PMK = %x:%x:%x:%x:%x:%x:%x:%x \n", 
                                pPort->SoftAP.ApCfg.PMK[0],pPort->SoftAP.ApCfg.PMK[1],pPort->SoftAP.ApCfg.PMK[2],pPort->SoftAP.ApCfg.PMK[3],
                                pPort->SoftAP.ApCfg.PMK[4],pPort->SoftAP.ApCfg.PMK[5],pPort->SoftAP.ApCfg.PMK[6],pPort->SoftAP.ApCfg.PMK[7]));
                        }
                        else
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ADD_KEY ERROR \n"));
                        }
                    }
                    else // Old WEP stuff
                    {
                        ULONG   KeyIdx = pKey->KeyIndex & 0x0fffffff;

                        // it is a shared key
                        if ((KeyIdx >= 4) || ((pKey->KeyLength != 5) && (pKey->KeyLength != 13)))
                            ndisStatus = NDIS_STATUS_INVALID_DATA;
                        else
                        {
                            UCHAR CipherAlg = (pKey->KeyLength == 5)? CIPHER_WEP64 : CIPHER_WEP128;
                            BOOLEAN  bTxKey = FALSE;

                            PlatformMoveMemory(pPort->SharedKey[BSS0][KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);
                            pPort->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
                            pPort->SharedKey[BSS0][KeyIdx].CipherAlg = CipherAlg;
                            if (pKey->KeyIndex & 0x80000000)
                            {
                                // Default key for tx (shared key)
                                pPort->PortCfg.DefaultKeyId = (UCHAR) KeyIdx;
                                bTxKey = TRUE;
                            }
                            AsicAddKeyEntry(pAd,pPort, 0, BSS0, (UCHAR) KeyIdx, &pPort->SharedKey[BSS0][KeyIdx], FALSE, bTxKey);
                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ADD_KEY (info-len=%d, id=0x%x, Len=%d-byte)\n", InformationBufferLength, pKey->KeyIndex, pKey->KeyLength));
                }
            } while (FALSE);
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                pKey = (PNDIS_802_11_KEY) pInformationBuffer;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ADD_KEY )\n"));
                if (InformationBufferLength != pKey->Length)
                {
                    *BytesNeeded = pKey->Length;
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ADD_KEY (Invalid information buffer length)\n"));
                }
                else
                {
                    if (pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA)
                    {
                        if(pKey->KeyLength == 32)
                        {
                            PlatformMoveMemory(pPort->SoftAP.ApCfg.PMK, pKey->KeyMaterial, 32);
                            DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_ADD_KEY::PMK = %x:%x:%x:%x:%x:%x:%x:%x \n", 
                                pPort->SoftAP.ApCfg.PMK[0],pPort->SoftAP.ApCfg.PMK[1],pPort->SoftAP.ApCfg.PMK[2],pPort->SoftAP.ApCfg.PMK[3],
                                pPort->SoftAP.ApCfg.PMK[4],pPort->SoftAP.ApCfg.PMK[5],pPort->SoftAP.ApCfg.PMK[6],pPort->SoftAP.ApCfg.PMK[7]));
                            DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_ADD_KEY::PMK = %x:%x:%x:%x:%x:%x:%x:%x \n", 
                                pPort->SoftAP.ApCfg.PMK[8],pPort->SoftAP.ApCfg.PMK[9],pPort->SoftAP.ApCfg.PMK[10],pPort->SoftAP.ApCfg.PMK[11],
                                pPort->SoftAP.ApCfg.PMK[12],pPort->SoftAP.ApCfg.PMK[13],pPort->SoftAP.ApCfg.PMK[14],pPort->SoftAP.ApCfg.PMK[15]));
                            DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_ADD_KEY::PMK = %x:%x:%x:%x:%x:%x:%x:%x \n", 
                                pPort->SoftAP.ApCfg.PMK[16],pPort->SoftAP.ApCfg.PMK[17],pPort->SoftAP.ApCfg.PMK[18],pPort->SoftAP.ApCfg.PMK[19],
                                pPort->SoftAP.ApCfg.PMK[20],pPort->SoftAP.ApCfg.PMK[21],pPort->SoftAP.ApCfg.PMK[22],pPort->SoftAP.ApCfg.PMK[23]));
                            DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_ADD_KEY::PMK = %x:%x:%x:%x:%x:%x:%x:%x \n", 
                                pPort->SoftAP.ApCfg.PMK[24],pPort->SoftAP.ApCfg.PMK[25],pPort->SoftAP.ApCfg.PMK[26],pPort->SoftAP.ApCfg.PMK[27],
                                pPort->SoftAP.ApCfg.PMK[28],pPort->SoftAP.ApCfg.PMK[29],pPort->SoftAP.ApCfg.PMK[30],pPort->SoftAP.ApCfg.PMK[31]));

                            // [GUI allows key change in single mode, but not con-current mode ]
                            // if KEYS have changed, GO should disconnect all of wpa clients due to the expired pmk.
                            // This flag is turned on from MTK_OID_N6_SET_P2P_GO_PASSPHRASE.
                            if (pPort->P2PCfg.PhraseKeyChanged == TRUE)
                            {
                                pPort->P2PCfg.PhraseKeyChanged = FALSE;
                                // Send disassociate frame to wpa clients.
                                P2pGroupMacTabDisconnect(pAd, pPort, 3);
                            }
                        }
                        else
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ADD_KEY ERROR \n"));
                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ADD_KEY (info-len=%d, id=0x%x, Len=%d-byte)\n", InformationBufferLength, pKey->KeyIndex, pKey->KeyLength));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
        else
        {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetAcl(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength < sizeof(RT_802_11_ACL))
                {
                    *BytesNeeded = sizeof(RT_802_11_ACL);   
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    PlatformMoveMemory(&pPort->SoftAP.AccessControlList, pInformationBuffer, sizeof(RT_802_11_ACL));
                    if (pPort->SoftAP.AccessControlList.Num > MAX_LEN_OF_MAC_TABLE)
                        pPort->SoftAP.AccessControlList.Num = MAX_LEN_OF_MAC_TABLE;

                    // check if ACL change affects any existent associtions
                    ApUpdateAccessControlList(pAd);
        
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_ACL (Policy=%d, Entry#=%d)\n",
                        pPort->SoftAP.AccessControlList.Policy, pPort->SoftAP.AccessControlList.Num));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetOpMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

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
            }
            else
            {
                UCHAR OpMode = *(UCHAR *)pInformationBuffer;
                pAd->OpMode = OpMode;
    
                //
                // The RX filter configuration is based upon the NIC is running on the station mode or the SoftAP mode.
                //
                MlmeInfoDisablePromiscuousMode(pAd);
                MlmeInfoOPModeSwitching(pAd);

                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_OP_MODE (=%d)\n", OpMode));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetApConfig(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
                    RT_802_11_AP_CONFIG *pApConfig = (RT_802_11_AP_CONFIG *)pInformationBuffer;
                    pPort->CommonCfg.bEnableTxBurst           = (BOOLEAN)(pApConfig->EnableTxBurst == 1);
                    pPort->SoftAP.ApCfg.bIsolateInterStaTraffic      = (BOOLEAN)(pApConfig->IsolateInterStaTraffic == 1);
                    pPort->CommonCfg.UseBGProtection          = (BOOLEAN)pApConfig->UseBGProtection;
                    
                    pPort->CommonCfg.bUseShortSlotTime        = (BOOLEAN)(pApConfig->UseShortSlotTime == 1);
                    pPort->SoftAP.ApCfg.AgeoutTime                   = pApConfig->AgeoutTime;
                    if (pPort->SoftAP.ApCfg.bHideSsid != (pApConfig->HideSsid == 1))
                    {
                        pPort->SoftAP.ApCfg.bHideSsid = (BOOLEAN)(pApConfig->HideSsid == 1);
                        // re-built BEACON frame format
                        MtAsicDisableSync(pAd);
                        MgntPktConstructAPBeaconFrame(pAd,pPort);
                        ApConnectUpdateBeaconFrame(pAd,pPort);
                        MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
                    }
                    APUpdateCapabilityAndErpIe(pAd);
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_AP_CONFIG (Burst=%d,Hotspot=%d,HideSsid=%d,BGprot=%d,ShortSlot=%d,AgeoutTime=%d\n",
                        pApConfig->EnableTxBurst,
                        pApConfig->IsolateInterStaTraffic,
                        pApConfig->HideSsid,
                        pApConfig->UseBGProtection,
                        pApConfig->UseShortSlotTime,
                        pApConfig->AgeoutTime));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetImmeBaCap(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(OID_BACAP_STRUC))
            {
                *BytesNeeded = sizeof(OID_BACAP_STRUC); 
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else 
            {
                OID_BACAP_STRUC Orde = *(OID_BACAP_STRUC *)pInformationBuffer;
    
                if (Orde.Policy > BA_NOTUSE)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                }
                else if (Orde.Policy == BA_NOTUSE)
                {
                    pPort->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
                    // Delete all existing BA Originator session. Becasue we disable using MPDU aggregation
                    pHeader = &pPort->MacTab.MacTabList;
                    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                    while (pNextMacEntry != NULL)
                    {
                        if (pPort->MacTab.Size == 0)
                            break;

                        pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                        
                        if(pMacEntry == NULL)
                        {
                            break; 
                        }
                        
                        if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
                        {
                            pNextMacEntry = pNextMacEntry->Next;   
                            pMacEntry = NULL;
                            continue; 
                        }

                        if (pMacEntry->ValidAsCLI == TRUE)
                        {
                            BATableTearORIEntry(pAd, pPort, 0x0, (UCHAR)pMacEntry->wcid,TRUE, TRUE);
                        }
                        
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                    }

                }
                else   
                {
                    // If Policy changed, always check Manual<->Auto status
                    if (pPort->CommonCfg.BACapability.field.Policy == BA_NOTUSE)
                    {
                        pPort->CommonCfg.BACapability.field.Policy = IMMED_BA; 
                        pPort->CommonCfg.BACapability.field.AutoBA = Orde.AutoBA;
                        BaAutoManSwitch(pAd, pPort);
                    }
                    else if (Orde.AutoBA == TRUE)
                    {
                        // If Policy doesn't changed, check Manual<->Auto switch
                        if (pPort->CommonCfg.BACapability.field.AutoBA == 0)
                        {
                            pPort->CommonCfg.BACapability.field.AutoBA= 1;
                            BaAutoManSwitch(pAd, pPort);
                        }
                    }
                    else
                    {
                        // If Policy doesn't changed, check Manual<->Auto switch
                        if (pPort->CommonCfg.BACapability.field.AutoBA == 1)
                        {
                            pPort->CommonCfg.BACapability.field.AutoBA= 0;
                            BaAutoManSwitch(pAd, pPort);
                        }
                    }
                    pPort->CommonCfg.BACapability.field.Policy = IMMED_BA; // we only support immediate BA. So always set to Immediate BA
                
                    if (pPort->CommonCfg.BACapability.field.RxBAWinLimit > Max_RX_REORDERBUF)
                        pPort->CommonCfg.BACapability.field.RxBAWinLimit = Max_RX_REORDERBUF;
                }
                
                // Updata to HT IE
                pPort->CommonCfg.HtCapability.HtCapInfo.MimoPs = Orde.MMPSmode;
                //if (pPort->CommonCfg.DesiredHtPhy.MimoPs != Orde.MMPSmode) // Always send it out for WiFi CLI command
                {
                    pPort->CommonCfg.DesiredHtPhy.MimoPs= Orde.MMPSmode;
                    pPort->CommonCfg.BACapability.field.MMPSmode = Orde.MMPSmode;

                    pHeader = &pPort->MacTab.MacTabList;
                    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                    while (pNextMacEntry != NULL)
                    {
                        if (pPort->MacTab.Size == 0)
                            break;
                    
                        pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                        
                        if(pMacEntry == NULL)
                        {
                            break; 
                        }
                        
                        if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
                        {
                            pNextMacEntry = pNextMacEntry->Next;   
                            pMacEntry = NULL;
                            continue; 
                        }

                        if (pMacEntry->ValidAsCLI == FALSE)
                        {
                            pNextMacEntry = pNextMacEntry->Next;   
                            pMacEntry = NULL;
                            continue; 
                        }
                        SendSMPSAction(pAd, pPort, (UCHAR)pMacEntry->wcid, Orde.MMPSmode);
                        DBGPRINT(RT_DEBUG_TRACE, ("(Entry[%d]. MmpsMode = %d ))\n", i, pMacEntry->MmpsMode));
                        
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                    }

                    DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_IMME_BA_CAP::Switch (MimoPs = %d))\n", pPort->CommonCfg.DesiredHtPhy.MimoPs));
                }
                
                if (pPort->CommonCfg.DesiredHtPhy.AmsduEnable != Orde.AmsduEnable)
                {
                    pPort->CommonCfg.DesiredHtPhy.AmsduEnable = Orde.AmsduEnable;

                    pHeader = &pPort->MacTab.MacTabList;
                    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                    while (pNextMacEntry != NULL)
                    {
                        if (pPort->MacTab.Size == 0)
                            break;
                    
                        pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                        
                        if(pMacEntry == NULL)
                        {
                            break; 
                        }
                        
                        if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
                        {
                            pNextMacEntry = pNextMacEntry->Next;   
                            pMacEntry = NULL;
                            continue; 
                        }
                        
                        if (pMacEntry->ValidAsCLI == FALSE)
                        {
                            pNextMacEntry = pNextMacEntry->Next;   
                            pMacEntry = NULL;
                            continue; 
                        }
                        if (pPort->CommonCfg.DesiredHtPhy.AmsduEnable)
                            CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);
                        else
                            CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);
                        DBGPRINT(RT_DEBUG_TRACE, ("(Entry[%d]. Flag %x))\n", pMacEntry->wcid, pMacEntry->ClientStatusFlags));
                        
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                    }

                    DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_IMME_BA_CAP::Switch (A-MSDU = %d))\n", pPort->CommonCfg.DesiredHtPhy.AmsduEnable));
                }
                pPort->CommonCfg.BACapability.field.MMPSmode = Orde.MMPSmode;
                pPort->CommonCfg.HtCapability.HtCapInfo.AMsduSize = Orde.AmsduSize;
                pPort->CommonCfg.HtCapability.HtCapParm.MpduDensity = Orde.MpduDensity;
                pPort->CommonCfg.DesiredHtPhy.AmsduSize= Orde.AmsduSize;
                pPort->CommonCfg.BACapability.field.MpduDensity = Orde.MpduDensity;
                pPort->CommonCfg.DesiredHtPhy.MpduDensity = Orde.MpduDensity;
                pPort->CommonCfg.REGBACapability.word = pPort->CommonCfg.BACapability.word;
                DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_IMME_BA_CAP::(Orde.AutoBA = %d) (Policy=%d)(ReBAWinLimit=%d)(TxBAWinLimit=%d)(AutoMode=%d)\n",Orde.AutoBA, pPort->CommonCfg.BACapability.field.Policy,
                    pPort->CommonCfg.BACapability.field.RxBAWinLimit,pPort->CommonCfg.BACapability.field.TxBAWinLimit, pPort->CommonCfg.BACapability.field.AutoBA));
                DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_IMME_BA_CAP::(MimoPs = %d)(AmsduEnable = %d) (AmsduSize=%d)(MpduDensity=%d)\n", pPort->CommonCfg.DesiredHtPhy.MimoPs, pPort->CommonCfg.DesiredHtPhy.AmsduEnable,
                    pPort->CommonCfg.DesiredHtPhy.AmsduSize, pPort->CommonCfg.DesiredHtPhy.MpduDensity));
                DBGPRINT(RT_DEBUG_INFO, ("MTK_OID_N6_SET_IMME_BA_CAP::MTK_OID_N5_SET_IMME_BA_CAP (=%d)\n", Orde.Policy));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetAddImmeBa(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                DBGPRINT(RT_DEBUG_TRACE, (" Set :: MTK_OID_N5_SET_ADD_IMME_BA. InformationBufferLength = %d,  sizeof = %d\n", InformationBufferLength, sizeof(OID_ADD_BA_ENTRY)));
                if (InformationBufferLength != sizeof(OID_ADD_BA_ENTRY))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                else 
                {
                    OID_ADD_BA_ENTRY BA = *(OID_ADD_BA_ENTRY *)pInformationBuffer;
                    UCHAR       index;
                    MAC_TABLE_ENTRY *pEntry;

                    DBGPRINT(RT_DEBUG_TRACE, (" Set :: IsRecipient = %d, BA.nMSDU = %d, BA.TID = %d \n", BA.IsRecipient, BA.nMSDU, BA.TID));
                    if (BA.TID > 15)
                    {               
                        ndisStatus = NDIS_STATUS_INVALID_DATA;
                        break;
                    }
                    {               
                        //BATableInsertEntry
                        //As ad-hoc mode, BA pair is not limited to only BSSID. so add via OID. 
                        index = BA.TID;
                        // in ad hoc mode, when adding BA pair, we should insert this entry into MACEntry too
                        pEntry = MacTableLookup(pAd,  pPort, BA.MACAddr);
                        if (!pEntry)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N5_SET_ADD_IMME_BA. break on no connection.----:%x:%x\n", BA.MACAddr[4], BA.MACAddr[5]));
                            break;
                        }
                        if (BA.IsRecipient == FALSE)
                        {
                            MLME_ADDBA_REQ_STRUCT    AddbaReq;  
                            PlatformZeroMemory(&AddbaReq, sizeof(AddbaReq));
                            if (pEntry->bIAmBadAtheros == TRUE)
                                pEntry->BaSizeInUse = 0x10;
                            if (pPort->CommonCfg.IOTestParm.bNextDisableRxBA == TRUE)
                            {
                                
                                if ((pEntry->bIAmBadAtheros == TRUE))
                                {
                                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);
                                }
                                else
                                {
                                    BATableInsertEntry(pAd, pPort, pEntry->Aid, 0, 0x0, BA.TID, BA.nMSDU, Originator_WaitRes,FALSE);
                                    AddbaReq.BaBufSize = BA.nMSDU;
                                    DBGPRINT(RT_DEBUG_TRACE, (" BA.nMSDU : %d\n", BA.nMSDU));
                                }
                                    
                            }
                            else if ((BA.nMSDU != pEntry->BaSizeInUse) && (BA.nMSDU <= 64))
                            {
                                BATableInsertEntry(pAd, pPort, pEntry->Aid, 0, 0x0, BA.TID, BA.nMSDU, Originator_WaitRes, FALSE);
                                AddbaReq.BaBufSize = BA.nMSDU;
        }
        else
        {
                                BATableInsertEntry(pAd, pPort, pEntry->Aid, 0, 0x0, BA.TID, pEntry->BaSizeInUse, Originator_WaitRes, FALSE);
                                AddbaReq.BaBufSize = pEntry->BaSizeInUse;
                            }
                            COPY_MAC_ADDR(AddbaReq.pAddr, BA.MACAddr);
                            AddbaReq.Wcid = (UCHAR)(pEntry->Aid);
                            AddbaReq.TID = BA.TID;
                            AddbaReq.TimeOutValue = BA.TimeOut;
                            AddbaReq.Token = 1;
                            MlmeEnqueue(pAd, pPort, ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq);
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N5_SET_ADD_IMME_BA. Rec = %d. Mac = %x:%x:%x:%x:%x:%x . BaBufSize = %d \n", 
                                BA.IsRecipient, BA.MACAddr[0], BA.MACAddr[1], BA.MACAddr[2], BA.MACAddr[2]
                                , BA.MACAddr[4], BA.MACAddr[5], pEntry->BaSizeInUse));
                        }
                        else 
            {
                        }
        
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTearImmeBa(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                DBGPRINT(RT_DEBUG_TRACE, (" Set :: MTK_OID_N5_SET_TEAR_IMME_BA InformationBufferLength = %d, sizeof(OID_ADD_BA_ENTRY) = %d\n", InformationBufferLength, sizeof(OID_ADD_BA_ENTRY)));
                if (InformationBufferLength != sizeof(OID_ADD_BA_ENTRY))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                else 
                {
                    OID_ADD_BA_ENTRY BA = *(OID_ADD_BA_ENTRY *)pInformationBuffer;
                    MAC_TABLE_ENTRY *pEntry;
                    DBGPRINT(RT_DEBUG_TRACE, (" Set :: MTK_OID_N5_SET_TEAR_IMME_BA(TID = %d)\n", BA.TID));
                    if ((BA.TID > 15) && (BA.bAllTid == FALSE))
                    {               
                        ndisStatus = NDIS_STATUS_INVALID_DATA;
                        break;
                    }
                    if (BA.IsRecipient == FALSE)
                    {
                        pEntry = MacTableLookup(pAd,  pPort, BA.MACAddr);
                        if (pEntry && (pEntry->Sst == SST_ASSOC))
                        {
                            BATableTearORIEntry( pAd, pPort, BA.TID, (UCHAR)pEntry->Aid, FALSE, BA.bAllTid);
                        }
                        else if (INFRA_ON(pPort) && (pPort->PortType == EXTSTA_PORT))
                        {
                            BATableTearORIEntry( pAd, pPort, BA.TID, (UCHAR)pEntry->Aid, FALSE, BA.bAllTid);
                        }
                        else if (pEntry)
                        {
                            BATableTearORIEntry( pAd, pPort, BA.TID, (UCHAR)pEntry->Aid, TRUE, BA.bAllTid);
                        }
                        else
                            DBGPRINT(RT_DEBUG_TRACE, (" Set :: Nnot found pEntry \n"));
                    }
                    else
                    {
                        pEntry = MacTableLookup(pAd,  pPort, BA.MACAddr);
                        if (pEntry)
                        {
                            BATableTearRECEntry( pAd, pPort, BA.TID, (UCHAR)pEntry->Aid, BA.bAllTid);
                        }
                        else
                            DBGPRINT(RT_DEBUG_TRACE, (" Set :: Nnot found pEntry \n"));
                    }
                }
        
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N5_SET_TEAR_IMME_BA  \n"));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetHtPhymode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(OID_SET_HT_PHYMODE))
            {
                *BytesNeeded = sizeof(OID_SET_HT_PHYMODE);  
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                POID_SET_HT_PHYMODE pHTPhyMode = (POID_SET_HT_PHYMODE)pInformationBuffer;

                DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_HT_PHYMODE -- OidBuf(PhyMode=%d, TransmitNo=%d, HtMode=%d, ExtOffset=%d,\n", 
                                pHTPhyMode->PhyMode, 
                                pHTPhyMode->TransmitNo,
                                pHTPhyMode->HtMode,
                                pHTPhyMode->ExtOffset,
                                ( (pHTPhyMode->PhyMode == PHY_11VHT)? (pHTPhyMode->MCS & 0x0F):(pHTPhyMode->MCS) )
                                ));
                DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_HT_PHYMODE -- OidBuf(<Nss in VHT=%d>, BW=%d, STBC=%d, SHORTGI=%d, <VhtCentralChannel in VHT=%d>) \n",
                                ( (pHTPhyMode->PhyMode == PHY_11VHT)? (pHTPhyMode->MCS & 0xF0):(0) ),
                                pHTPhyMode->BW,
                                pHTPhyMode->STBC,
                                pHTPhyMode->SHORTGI,
                                pHTPhyMode->CentralChannelSeg1
                                ));

                if (pPort->PortType == EXTSTA_PORT) 
                {
                    pHTPhyMode->TransmitNo = (UCHAR)(pAd->HwCfg.Antenna.field.TxPath);
                }

                if(pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
                {
                    // Since TKIP would restrict the phymode to BG, reset the cipher mode to prevent mis-setting of phymode.
                    DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Reset Cipher to Open-NONE\n",__FUNCTION__,__LINE__));
                    pPort->PortCfg.WepStatus = Ralink802_11EncryptionDisabled;
                }
    
                if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    SetHtVht(pAd, pPort, pHTPhyMode);
                }

                // Note that when the NIC supports the N mode, the Ralink UI firstly sets
                // the MTK_OID_N6_SET_PHY_MODE and then MTK_OID_N6_SET_HT_PHYMODE.
                // Therefore, the NIC should only restarts the SoftAP during it receives the MTK_OID_N6_SET_HT_PHYMODE.
                // On other hand, if the NIC does not support N mode, it should restart the SoftAP during
                // it receives the MTK_OID_N6_SET_PHY_MODE (the Ralink UI does not set MTK_OID_N6_SET_HT_PHYMODE).
                if ((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) && 
                     (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
                {
                    //RestartAPIsRequired = TRUE;
                }

                PRINT_PHY_CFG(pAd, &pPort->CommonCfg.TxPhyCfg, RT_DEBUG_TRACE);
                DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_HT_PHYMODE, Setting complete"));                 
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetDesiredBssidList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength < FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs))
                {
                    *BytesNeeded = FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs);
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
                }
                else
                {
                    PDOT11_BSSID_LIST pDot11BSSIDList = (PDOT11_BSSID_LIST)pInformationBuffer;

                    if (pDot11BSSIDList->uNumOfEntries > 1)
                    {
                        ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                        DBGPRINT(RT_DEBUG_ERROR, ("Set::MTK_OID_N6_SET_DESIRED_BSSID_LIST failed, (uNumOfEntries=%d) we only support one BSSID\n",
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
                        *BytesWritten =  FIELD_OFFSET(DOT11_BSSID_LIST, BSSIDs) + pDot11BSSIDList->uNumOfEntries * sizeof(DOT11_MAC_ADDRESS);
        
                        // This set request is from the Ralink UI.
                        pAd->StaCfg.bSetDesiredBSSIDListFromRalinkUI = TRUE;
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_DESIRED_BSSID_LIST (AcceptAnyBSSID=%d,NumofBSSID=%d,[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                            pPort->PortCfg.AcceptAnyBSSID, pPort->PortCfg.DesiredBSSIDCount, 
                            pPort->PortCfg.DesiredBSSIDList[0][0], pPort->PortCfg.DesiredBSSIDList[0][1], pPort->PortCfg.DesiredBSSIDList[0][2],
                            pPort->PortCfg.DesiredBSSIDList[0][3], pPort->PortCfg.DesiredBSSIDList[0][4], pPort->PortCfg.DesiredBSSIDList[0][5]));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetDisconnectRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG BytesReadTemp, BytesNeededTemp;
    
            // Reset the desired BSSID list that set by the MTK_OID_N6_SET_DESIRED_BSSID_LIST or OID_DOT11_DESIRED_BSSID_LIST.
            // In this way, other connection request is allowed again.
            pPort->PortCfg.DesiredBSSIDList[0][0] = 0xFF;
            pPort->PortCfg.DesiredBSSIDList[0][1] = 0xFF;
            pPort->PortCfg.DesiredBSSIDList[0][2] = 0xFF;
            pPort->PortCfg.DesiredBSSIDList[0][3] = 0xFF;
            pPort->PortCfg.DesiredBSSIDList[0][4] = 0xFF;
            pPort->PortCfg.DesiredBSSIDList[0][5] = 0xFF;
            pPort->PortCfg.DesiredBSSIDCount = 1;
            pPort->PortCfg.AcceptAnyBSSID = TRUE;

            // Reset to FALSE.
            pAd->StaCfg.bSetDesiredBSSIDListFromRalinkUI = FALSE;

            DBGPRINT(RT_DEBUG_TRACE, ("%s: Clear the desired BSSID list and reset the bSetDesiredBSSIDListFromRalinkUI.\n", 
                __FUNCTION__));             
        
            DBGPRINT(RT_DEBUG_TRACE, ("%s: MTK_OID_N6_SET_DISCONNECT_REQUEST\n", __FUNCTION__));
            
            ndisStatus = DisconnectRequest(pAd, pPort,&BytesReadTemp, &BytesNeededTemp, FALSE);
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWpaRekey(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    PRT_WPA_REKEY pWPA_REKEY = NULL;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            do
            {
                // EXPAND_FUNC NONE
                if ((pPort->PortCfg.AuthMode < Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeAutoSwitch))
                {
                    if(pPort->SoftAP.ApCfg.REKEYTimerRunning == TRUE)
                    {
                        BOOLEAN     Cancelled;
                        PlatformCancelTimer(&pPort->SoftAP.ApCfg.REKEYTimer, &Cancelled);
                        pPort->SoftAP.ApCfg.REKEYTimerRunning = FALSE;
                    }
                    break;
                }
                if(pInformationBuffer == NULL)
                {
                    DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
                    return NDIS_STATUS_INVALID_DATA;
                }
                pWPA_REKEY = (PRT_WPA_REKEY) pInformationBuffer;
                
                // mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based
                if (pWPA_REKEY->ReKeyMethod >MAX_REKEY)
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else if (pWPA_REKEY->ReKeyInterval > MAX_REKEY_INTER)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                }
                else if ((pWPA_REKEY->ReKeyMethod == DISABLE_REKEY) ||(pWPA_REKEY->ReKeyInterval == 0))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WPA_REKEY: DISABLE WPA_REKEY\n"));
                    pPort->SoftAP.ApCfg.WPAREKEY.ReKeyInterval = pWPA_REKEY->ReKeyInterval;
                    pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod = DISABLE_REKEY;

                    if(pPort->SoftAP.ApCfg.REKEYTimerRunning == TRUE)
                    {
                        BOOLEAN     Cancelled;
                        PlatformCancelTimer(&pPort->SoftAP.ApCfg.REKEYTimer, &Cancelled);
                        pPort->SoftAP.ApCfg.REKEYTimerRunning = FALSE;
                    }
                }
                else if ((pWPA_REKEY->ReKeyMethod == PKT_REKEY)    || (pWPA_REKEY->ReKeyMethod == TIME_REKEY))  //packet-based
                {
                    if(pPort->SoftAP.ApCfg.REKEYTimerRunning == FALSE)
                    {
                        PlatformSetTimer(pPort, &pPort->SoftAP.ApCfg.REKEYTimer, GROUP_KEY_UPDATE_EXEC_INTV);
                        pPort->SoftAP.ApCfg.REKEYTimerRunning = TRUE;
                        pPort->SoftAP.ApCfg.REKEYCOUNTER = 0;
                    }
        
                    pPort->SoftAP.ApCfg.WPAREKEY.ReKeyInterval = pWPA_REKEY->ReKeyInterval;
                    pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod = pWPA_REKEY->ReKeyMethod;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WPA_REKEY , ReKeyMethod= %x, interval= %x\n", pWPA_REKEY->ReKeyMethod,pWPA_REKEY->ReKeyInterval));  
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetFlushBssList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_FLUSH_BSS_LIST\n"));
        
                //
                // To tell OID_DOT11_CONNECT_REQUEST that we clear all the scan table.
                // So we can't skip OID_DOT11_CONNECT_REQUEST if NDIS asks for it.
                //
                pAd->pNicCfg->bIsClearScanTab = TRUE;
                
                // clean up previous SCAN result, add current BSS back to table if any
                BssTableInit(&pAd->ScanTab); 
                ResetRepositBssTable(pAd);
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetStaConfig(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(RT_802_11_STA_CONFIG))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                else 
                {
                    RT_802_11_STA_CONFIG *pStaConfig = (RT_802_11_STA_CONFIG *)pInformationBuffer;
                    if (pStaConfig->bEnableTxBurst != pPort->CommonCfg.bEnableTxBurst)
                    {
                        pPort->CommonCfg.bEnableTxBurst = !(pStaConfig->bEnableTxBurst == 0);
                        //Currently Tx burst mode is only implemented in infrastructure mode.
                        if (INFRA_ON(pPort))
                        {
                            if (pPort->CommonCfg.bEnableTxBurst)
                            {
                                //Extend slot time if any encryption method is used to give ASIC more time to do encryption/decryption during Tx burst mode.
                                if (pPort->PortCfg.WepStatus != Ralink802_11EncryptionDisabled)
                                {
                                }
                                //Set CWmin/CWmax to 0.
                            }
                            else
                            {
                                if (pPort->PortCfg.WepStatus != Ralink802_11EncryptionDisabled)
                                    MtAsicSetSlotTime(pAd, (BOOLEAN)pPort->CommonCfg.bUseShortSlotTime, pPort->Channel);
                            }
                        }
                    }
                    pPort->CommonCfg.UseBGProtection = pStaConfig->UseBGProtection;
                    pPort->CommonCfg.bUseShortSlotTime = 1; // 2003-10-30 always SHORT SLOT capable
                    if (pAd->StaCfg.AdhocMode != pStaConfig->AdhocMode)
                    {
                        // allow dynamic change of "USE OFDM rate or not" in ADHOC mode
                        // if setting changed, need to reset current TX rate as well as BEACON frame format
                        pAd->StaCfg.AdhocMode = pStaConfig->AdhocMode;
                        if (pAd->StaCfg.BssType == BSS_ADHOC)
                        {
                            MlmeUpdateTxRates(pAd, pPort,FALSE);
                            MgntPktConstructIBSSBeaconFrame(pAd,pPort);
                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_STA_CONFIG (Burst=%d,BGprot=%d,ShortSlot=%d,Adhoc=%d\n",
                        pStaConfig->bEnableTxBurst,
                        pStaConfig->UseBGProtection,
                        pStaConfig->UseShortSlotTime,
                        pStaConfig->AdhocMode));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetCcx20Info(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                DBGPRINT(RT_DEBUG_TRACE, ("Set::Not Support MTK_OID_N6_SET_CCX20_INFO\n"));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetPreamble(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(ULONG))
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            else 
            {
                ULONG Preamble = *(ULONG *)pInformationBuffer;
                if (Preamble == Rt802_11PreambleShort)
                {
                    pPort->CommonCfg.TxPreamble = (USHORT)Preamble;
                    MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
                }
                else if ((Preamble == Rt802_11PreambleLong) || (Preamble == Rt802_11PreambleAuto))
                {
                    // if user wants AUTO, initialize to LONG here, then change according to AP's
                    // capability upon association.
                    pPort->CommonCfg.TxPreamble = (USHORT)Preamble;
                    MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
                }
                else
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_PREAMBLE (=%d)(pPort->CommonCfg.TxPreamble= %d)\n", Preamble, pPort->CommonCfg.TxPreamble));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetApsdSetting(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(ULONG))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                else 
                {
                    ULONG apsd = *(ULONG *)pInformationBuffer;

                    /*-------------------------------------------------------------------
                    |B31~B7 |   B6~B5    |   B4  |   B3  |  B2   |  B1   |     B0       |
                    ---------------------------------------------------------------------
                    | Rsvd  | Max SP Len | AC_VO | AC_VI | AC_BK | AC_BE | APSD Capable |
                    ---------------------------------------------------------------------*/
                    pPort->CommonCfg.bAPSDCapable = (apsd & 0x00000001) ? TRUE : FALSE;
                    pPort->CommonCfg.bAPSDAC_BE = ((apsd & 0x00000002) >> 1) ? TRUE : FALSE;
                    pPort->CommonCfg.bAPSDAC_BK = ((apsd & 0x00000004) >> 2) ? TRUE : FALSE;
                    pPort->CommonCfg.bAPSDAC_VI = ((apsd & 0x00000008) >> 3) ? TRUE : FALSE;
                    pPort->CommonCfg.bAPSDAC_VO = ((apsd & 0x00000010) >> 4) ? TRUE : FALSE;
                    pPort->CommonCfg.MaxSPLength = (UCHAR)((apsd & 0x00000060) >> 5);
        
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_APSD_SETTING (apsd=0x%x, APSDCap=%d, [BE,BK,VI,VO]=[%d/%d/%d/%d], MaxSPLen=%d)\n", apsd, pPort->CommonCfg.bAPSDCapable,
                        pPort->CommonCfg.bAPSDAC_BE, pPort->CommonCfg.bAPSDAC_BK, pPort->CommonCfg.bAPSDAC_VI, pPort->CommonCfg.bAPSDAC_VO, pPort->CommonCfg.MaxSPLength));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetApsdPsm(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(ULONG))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                else 
                {
                    pPort->CommonCfg.bAPSDForcePowerSave = *(BOOLEAN*)pInformationBuffer;
                    if (pPort->CommonCfg.bAPSDForcePowerSave != pAd->StaCfg.Psm)
                    {
                        MlmeSetPsm(pAd, pPort->CommonCfg.bAPSDForcePowerSave);
        
                        // Driver needs to notify AP when PSM changes
                        XmitSendNullFrame(pAd,pPort, pPort->CommonCfg.TxRate, TRUE);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_APSD_PSM (%d)\n", pPort->CommonCfg.bAPSDForcePowerSave));
                    break;
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWmm(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != sizeof(BOOLEAN))
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            else 
            {
                pPort->CommonCfg.bWmmCapable = *(BOOLEAN*)pInformationBuffer;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_SET_WMM (=%d) \n", pPort->CommonCfg.bWmmCapable));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetDls(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(ULONG))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                // Currently, 'DirectMode' is changed only from Registry.
                else 
                {
                    BOOLEAN     oldvalue = pPort->CommonCfg.bDLSCapable, newvalue;
                    newvalue =  *(UCHAR *)pInformationBuffer;

                    if (pPort->CommonCfg.DirectMode == DIRECT_LINK_MODE_TDLS)
                    {
                        if (oldvalue && !newvalue)
                        {
        
                            TdlsTearDown(pAd, TRUE);
                        }
                    }
                    else    // DLS direct mode
                    {
                        if (oldvalue && !newvalue)
                        {
                            DlsTearDown(pAd, pPort);
                        }
                    }
                    // update DLSCapable
                    pPort->CommonCfg.bDLSCapable = newvalue;
                    DBGPRINT(RT_DEBUG_TRACE,("Set::MTK_OID_N6_SET_DLS (=%d), DirectMode=%d\n", pPort->CommonCfg.bDLSCapable, pPort->CommonCfg.DirectMode));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetDlsParam(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(RT_802_11_DLS_UI))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                else 
                {
                    if (pPort->SoftAP.bAPStart == FALSE)
                    {
                        RT_802_11_DLS   Dls;
        
                        PlatformMoveMemory(&Dls, pInformationBuffer, sizeof(RT_802_11_DLS_UI));
                        MlmeEnqueue(pAd,
                                    pPort,
                                    MLME_CNTL_STATE_MACHINE, 
                                    MTK_OID_N6_SET_DLS_PARAM, 
                                    sizeof(RT_802_11_DLS), 
                                    &Dls);
                        DBGPRINT(RT_DEBUG_TRACE,("Set::MTK_OID_N6_SET_DLS_PARAM \n"));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetCipherKeyMappingKey(
    IN PMP_ADAPTER       pAd,
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

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if ( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n", Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) ||
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
                DBGPRINT(RT_DEBUG_TRACE, ("standard OIDs set by UI...%08x\n", Oid));

            // EXPAND_FUNC RTMPSetInformation()
            if (InformationBufferLength < sizeof(DOT11_BYTE_ARRAY))
            {
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
            }
            else
            {
                ULONG                               TotalLength;
                ULONG                               size;
                PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE pKeyMappingKeys;
                PDOT11_BYTE_ARRAY                   KeyData      = (PDOT11_BYTE_ARRAY)pInformationBuffer;
                NDIS_STATUS                         AddKeyStatus = NDIS_STATUS_SUCCESS;

                *BytesWritten = FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer) + KeyData->uNumOfBytes;

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

                TotalLength     = KeyData->uNumOfBytes;
                pKeyMappingKeys = (PDOT11_CIPHER_KEY_MAPPING_KEY_VALUE) KeyData->ucBuffer;
                while (TotalLength >= FIELD_OFFSET(DOT11_CIPHER_KEY_MAPPING_KEY_VALUE, ucKey))
                {
                    size = FIELD_OFFSET(DOT11_CIPHER_KEY_MAPPING_KEY_VALUE, ucKey) + pKeyMappingKeys->usKeyLength;
                    if (TotalLength < size)
                    {
                        ndisStatus = NDIS_STATUS_INVALID_DATA;
                        break;
                    }

                    DBGPRINT_RAW(RT_DEBUG_TRACE, ("PeerMacAddr = %2x:%2x:%2x:%2x:%2x:%2x", pKeyMappingKeys->PeerMacAddr[0],
                                                  pKeyMappingKeys->PeerMacAddr[1], pKeyMappingKeys->PeerMacAddr[2], pKeyMappingKeys->PeerMacAddr[3], pKeyMappingKeys->PeerMacAddr[4],
                                                  pKeyMappingKeys->PeerMacAddr[5]));
                    if (KeGetCurrentIrql() == DISPATCH_LEVEL)
                    {
                        UCHAR InBuffer[256] = {0};
                        ULONG InBufferLen   = 0;

                        // Fill inbuffer with port number and InformationBuffer
                        PlatformZeroMemory(InBuffer, sizeof(InBuffer));
                        InBufferLen = 0;
                        PlatformMoveMemory(&InBuffer[0],                        &PortNumber,        sizeof(NDIS_PORT_NUMBER));
                        InBufferLen += sizeof(NDIS_PORT_NUMBER);
                        PlatformMoveMemory(&InBuffer[sizeof(NDIS_PORT_NUMBER)], pInformationBuffer, InformationBufferLength);
                        InBufferLen += InformationBufferLength;
                        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_ADD_STA_MAPPING_KEY, InBuffer /*InformationBuffer*/, InBufferLen /*InformationBufferLength*/);
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

#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
                        // Update P2pMs status and info during adding the pairwise key
                        if (NDIS_WIN8_ABOVE(pAd) &&
                            ((pPort->PortType == WFD_CLIENT_PORT) || (pPort->PortType == WFD_GO_PORT)) &&
                            (AddKeyStatus == NDIS_STATUS_SUCCESS))
                        {
                            P2pMsWPADone(pAd, pPort, pKeyMappingKeys->PeerMacAddr);
                        }
#endif
                    }

                    TotalLength    -= size;
                    pKeyMappingKeys = Add2Ptr(pKeyMappingKeys, size);
                }
                *BytesWritten = *BytesNeeded;

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
                    UCHAR Index;
                    for (Index = 0; Index < 4; Index++)
                    {
                        if (pAd->pTxCfg->SendTxWaitQueue[Index].Number > 0)
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
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
                    DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n", __FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired)
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) &&
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd, pPort);
                APStartUp(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
MtkOidSetCipherDefaultKey(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if(pBssidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return NDIS_STATUS_INVALID_DATA;
    } 

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("standard OIDs set by UI...%08x\n", Oid));
    
            // EXPAND_FUNC RTMPSetInformation()
            if (InformationBufferLength < FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey))
            {
                *BytesNeeded = FIELD_OFFSET(DOT11_CIPHER_DEFAULT_KEY_VALUE, ucKey);
                ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                DBGPRINT(RT_DEBUG_ERROR, ("Set::OID_DOT11_CIPHER_DEFAULT_KEY, Invalid keyLen=%d, need=%d\n", InformationBufferLength, *BytesNeeded));
                break;
            }
            else
            {
                PDOT11_CIPHER_DEFAULT_KEY_VALUE pDefaultKey = (PDOT11_CIPHER_DEFAULT_KEY_VALUE)pInformationBuffer;

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
                        PlatformMoveMemory(&InBuffer[sizeof(NDIS_PORT_NUMBER)], pInformationBuffer, InformationBufferLength);
                        InBufferLen += InformationBufferLength;                 
                        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_ADD_STA_DEFAULT_KEY, InBuffer/*InformationBuffer*/, InBufferLen/*InformationBufferLength*/);
                    }
                    else
                    {
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

                        P2pWPADone(pAd, pPort, pBssidMacTabEntry, FALSE);
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
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetMfpControl(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_MFP_CONTROL\n"));
                if (InformationBufferLength != sizeof(PMF_CONTROL))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("sizeof(PMF_CONTROL) = %d Len = %d\n", sizeof(PMF_CONTROL),InformationBufferLength));
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    PlatformMoveMemory(&(pAd->StaCfg.PmfCfg.PmfControl), pInformationBuffer, sizeof(PMF_CONTROL));
                    DumpFrame((PPMF_CONTROL)pInformationBuffer, sizeof(PMF_CONTROL))
                    pAd->StaCfg.PmfCfg.MFPC = pAd->StaCfg.PmfCfg.PmfControl.MFP_Enable;
                    pAd->StaCfg.PmfCfg.MFPR = pAd->StaCfg.PmfCfg.PmfControl.MFP_Required;

                    if(pAd->StaCfg.PmfCfg.PmfControl.InjectPacket)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("MFP: Inject Pacekt(s)\n"));                              

                        PmfInjectPckets(pAd, pPort);
                    }
        
                    if(pAd->StaCfg.PmfCfg.PmfControl.SilentMlmeCntLinkDown)
                    {
                        pAd->StaCfg.PmfCfg.PmfControl.SilentMlmeCntLinkDown = 0;
                        MlmeCntLinkDown(pPort, FALSE);
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetFragmentationThreshold(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("standard OIDs set by UI...%08x\n", Oid));
    
            // EXPAND_FUNC RTMPSetInformation()
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
                ULONG Frag_Threshold = *(PULONG)pInformationBuffer;
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
                *BytesWritten = sizeof(ULONG);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_FRAGMENTATION_THRESHOLD (=%d)\n", Frag_Threshold)); 
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetRtsThreshold(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("standard OIDs set by UI...%08x\n", Oid));
    
            // EXPAND_FUNC RTMPSetInformation()
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
                ULONG RTS_Threshold = *(PULONG)pInformationBuffer;
                TX_RTS_CFG_STRUC RtsCfg;

                if (RTS_Threshold > MAX_RTS_THRESHOLD)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }

                *BytesWritten = sizeof(ULONG);

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
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetPowerMgmtRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                ULONG   PwrSaveLevel;
                if (InformationBufferLength < sizeof(ULONG))
                {
                    *BytesNeeded = sizeof(ULONG);
                    ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
                }

                PwrSaveLevel = *(PULONG)pInformationBuffer;
                if (PwrSaveLevel > DOT11_POWER_SAVING_MAXIMUM_LEVEL)
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }
                
                if((pPort->SoftAP.bAPStart == FALSE) &&
                  (pAd->PortList[PORT_0]->PortSubtype != PORTSUBTYPE_P2PGO))
                {
                    MlmeInfoPnpSetPowerMgmtMode(pAd, PwrSaveLevel);
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_POWER_MGMT_REQUEST(%u)\n", *(PULONG)pInformationBuffer));
        
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetLedWpsMode10(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    ULONG WPSLedMode10 = 0; 
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if(!(pAd->StaCfg.bRadio))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    break;
                }
        
                WPSLedMode10 = *(PULONG)pInformationBuffer;
                if(InformationBufferLength != sizeof(ULONG))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    DBGPRINT(RT_DEBUG_TRACE, ("WPS LED MODE10--parameter wrong type!!\n"));
                    break;
                }
                if((WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_ON) && 
                    (WPSLedMode10 != LINK_STATUS_WPS_MODE10_FLASH) && 
                    (WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_OFF))
                {
                    ndisStatus = NDIS_STATUS_INVALID_DATA;
                    DBGPRINT(RT_DEBUG_TRACE, ("WPS LED MODE10--Parameter of LED Mode 10 must be 0x00, 0x01, 0x02\n"));
                }
                else
                { 
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_LED_WPS_MODE10, &WPSLedMode10, sizeof(WPSLedMode10));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscHwPbcCustomerVendorStateMachine(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                pPort->StaCfg.WscControl.bCheckHWPBCCustomerStateMachine = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, ("SET::MTK_OID_N6_SET_WSC_HW_PBC_CUSTOMER_VENDOR_STATE_MACHINE.\n"));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscPinCode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != 8)
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_PIN_CODE Failed since Invalid Length!!\n"));
                }
                else 
                {

                    PUCHAR P2PPinCode = NULL;
                    P2PPinCode = pInformationBuffer;
                    
                    //PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, pInformationBuffer, 8);
                    //PlatformMoveMemory(pPort->P2PCfg.PinCode, pInformationBuffer, 8);
                    if((P2PPinCode[0] == 0xff) && (P2PPinCode[1] == 0xff) &&
                        (P2PPinCode[2] == 0xff) && (P2PPinCode[3] == 0xff))
                    {
                        // 4 digits
                        PlatformMoveMemory(pPort->P2PCfg.PinCode, &P2PPinCode[4], 4);
                        PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, pPort->P2PCfg.PinCode, 4);
                        pPort->StaCfg.WscControl.RegData.PINLen = 2;
                    }
                    else
                    {
                        // 8 digits
                        PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, P2PPinCode, 8);
                        pPort->StaCfg.WscControl.RegData.PINLen = 4;
                    }

                    // After get PIN code, Send M2 out.
                    WscSendMessageM2(pAd, pPort, pPort->StaCfg.WscControl.M2IdForP2P, &pPort->StaCfg.WscControl.M2ElemForP2P);
        
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_PIN_CODE -> PINLen = %d, PinCode = %02x %02x %02x %02x %02x %02x %02x %02x\n", 
                        pPort->P2PCfg.PINLen, pPort->P2PCfg.PinCode[0], pPort->P2PCfg.PinCode[1], pPort->P2PCfg.PinCode[2], pPort->P2PCfg.PinCode[3], pPort->P2PCfg.PinCode[4], pPort->P2PCfg.PinCode[5], pPort->P2PCfg.PinCode[6], pPort->P2PCfg.PinCode[7]));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetCcxMfp(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetCcxTxAironet(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWmmAcmCmd(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetUsbCyclePort(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
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
                ULONG CyclePort = *(PULONG)pInformationBuffer;
    
                if (CyclePort == TRUE)
                {
                    RTUSB_Replug(pAd);          
                }
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetBtHciSendCmd(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetBtHciSendAclData(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pConfig(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_P2P_CONFIG. %d port(%d)\n", InformationBufferLength, PortNumber));
            if (InformationBufferLength != sizeof(RT_OID_SET_P2P_STRUCT))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_CONFIG. Size isn't correct.(=%d  %d)\n", InformationBufferLength, sizeof(RT_OID_SET_P2P_STRUCT)));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                ndisStatus = P2pSetProfile(pAd, pPort, InformationBufferLength, pInformationBuffer);
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pPerstTab(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_P2P_PERST_TAB. %d\n", InformationBufferLength));
            if (InformationBufferLength != sizeof(OID_P2P_PERSISTENT_TABLE))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_PERST_TAB. Size isn't correct.(=%d  %d)\n", InformationBufferLength, sizeof(OID_P2P_PERSISTENT_TABLE)));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                if (IS_P2P_SIGMA_OFF(pPort))
                    P2pSetPerstTable(pAd, pInformationBuffer);
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pEvent(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    PSET_EVENT  pDSEvent;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            // X64 platform HANDLE size is 8 bytes. But Raui generate 4-byte. So ignore size check
//          if (InformationBufferLength < sizeof(SET_EVENT))
//              ndisStatus = NDIS_STATUS_INVALID_LENGTH;
//          else
            {
                HANDLE p2pHandle; 

                // Do not create P2P in SoftAP mode
                if(pAd->OpMode == OPMODE_AP || pAd->OpMode == OPMODE_APCLIENT)
                    break;

                // Determine OPMODE for WIN7 two client mode
                if (pPort->CommonCfg.P2pControl.field.AdvP2PMode == 0)
                {
                    // Set station mode on port0 when AdvP2PMode bit is off
                    pAd->OpMode = OPMODE_STA;
                    pPort->P2PCfg.PortNumber = PORT_0;
                }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                else if (pAd->MacAddressUsed[MAX_NUM_OF_TOTAL_WFD_ROLE - 1] == TRUE)
                {
                    // start from port#1
                    for (i=1; i < RTMP_MAX_NUMBER_OF_PORT; i++)
                    {
                        if ((pAd->PortList[i] == NULL) || (pAd->PortList[i]->bActive == FALSE))
                            continue;

                        if (MAC_ADDR_EQUAL(pAd->PortList[i]->CurrentAddress, pAd->MacAddressList[MAX_NUM_OF_TOTAL_WFD_ROLE - 1]))
                        {
                            // When AdvP2PMode bit is on and virtual port has ever created, set to p2p concurrent mode on P2PCfg.PortNumber
                            pAd->OpMode = OPMODE_STAP2P;
                            pPort->P2PCfg.PortNumber = (UCHAR)pAd->PortList[i]->PortNumber;

                            // It might be turned-on by the previous of MS WFD.
                            // Need back to the disabled state and run init settings again by p2penable()
                            if (P2P_ON(pPort))
                            {
                                pPort->P2PCfg.P2PDiscoProvState = P2P_DISABLE;
                            }
                            
                            DBGPRINT(RT_DEBUG_TRACE,("Switch to STA+P2P. P2P PortNumber = %d.!!!\n", pPort->P2PCfg.PortNumber));
                            break;
                        }
                    }
                }
                else
                {
                    // return fail when virtual staion port is not ready
                    ndisStatus = NDIS_STATUS_INVALID_OID;
                    DBGPRINT(RT_DEBUG_TRACE,("Unable to reference event object due to Virtual Port not ready!!!\n"));
                    break;
                }
#else
                else if ((pAd->PortList[PORT_2] != NULL) && (pAd->PortList[PORT_2]->bActive == TRUE) && (pPort->CommonCfg.P2pControl.field.AdvP2PMode == 1))
                {
                    // When AdvP2PMode bit is on and port2 has ever created, set to p2p concurrent mode on Port2
                    pAd->OpMode = OPMODE_STAP2P;
                    pPort->P2PCfg.PortNumber = PORT_2;
                    DBGPRINT(RT_DEBUG_TRACE,("Switch to STA+P2P. P2P PortNumber = %d.!!!\n", pPort->P2PCfg.PortNumber));
                }
                else
                {
                    // return fail when port2 is not ready
                    ndisStatus = NDIS_STATUS_INVALID_OID;
                    DBGPRINT(RT_DEBUG_TRACE,("Unable to reference event object due to PORT2 not ready!!!\n"));
                    break;
                }
#endif

                //GUI set event handle to use.
                pDSEvent = (PSET_EVENT)pInformationBuffer;
                p2pHandle = (VOID*POINTER_32) pDSEvent->hEvent;
                DBGPRINT(RT_DEBUG_TRACE, ("pInformationBuffer %x\n", pInformationBuffer) );
                DBGPRINT(RT_DEBUG_TRACE, ("pDSEvent %x\n", p2pHandle) );
                ndisStatus = ObReferenceObjectByHandle( p2pHandle,
                                SYNCHRONIZE,
                                *ExEventObjectType,
                                UserMode ,
                                &pPort->P2PCfg.P2pEvent,
                                NULL
                                );
                DBGPRINT(RT_DEBUG_TRACE, ("After User mode Reference %x\n", pDSEvent->hEvent) );

                if ( !NT_SUCCESS(ndisStatus) ) 
                {
                    DBGPRINT( RT_DEBUG_TRACE, ("Real Error Code = 0x%x\n", ndisStatus) );
                    pPort->P2PCfg.P2pEvent = NULL;
                    // Need use NDIS_STATUS_INVALID_OID to notify GUI that this OID fails.
                    ndisStatus = NDIS_STATUS_INVALID_OID;
                    DBGPRINT( RT_DEBUG_TRACE, ("Unable to reference   Event object, Error = 0x%x\n", ndisStatus) );
                } 
                else
                {
                    // Init state to idle.
                    P2pSetEvent(pAd, pPort, P2PEVENT_STATUS_IDLE);
                    DBGPRINT( RT_DEBUG_TRACE, ("User -mode HANDLE = 0x%x\n",  pPort->P2PCfg.P2pEvent));
                }
    
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_EVENT (=%x)\n", ndisStatus));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pDisconnectPerConnection(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength != MAC_ADDRESS_LENGTH)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("set:: MTK_OID_N6_SET_P2P_DISCONNECT_PER_CONNECTION InformationBufferLength = %d", InformationBufferLength));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                UCHAR addr[6];
                
                PlatformMoveMemory(&addr[0], pInformationBuffer, 6);
                DBGPRINT(RT_DEBUG_TRACE, ("set:: MTK_OID_N6_SET_P2P_DISCONNECT_PER_CONNECTION pInformationBuffer = %x %x %x %x %x %x"
                    , *((UCHAR *)pInformationBuffer), *((UCHAR *)pInformationBuffer+1), 
                    *((UCHAR *)pInformationBuffer +2), *((UCHAR *)pInformationBuffer +3), *((UCHAR *)pInformationBuffer +4),*((UCHAR *)pInformationBuffer +5)));
    
                DBGPRINT(RT_DEBUG_TRACE, ("set:: MTK_OID_N6_SET_P2P_DISCONNECT_PER_CONNECTION addr = %x %x %x %x %x %x"
                    ,addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]));
                ndisStatus = P2pDisconnectOneConnection(pAd, pPort, InformationBufferLength, pInformationBuffer);
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pEventReady(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_EVENT_READY. Size isn't correct.(=%d  %d)\n", InformationBufferLength, sizeof(RT_OID_SET_P2P_STRUCT)));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                ULONG EventReady = *(ULONG *) pInformationBuffer;
                if (EventReady == 1)
                {
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent  = 0xffffffff;
                    //P2pSetEvent(pAd, pPort, P2PEVENT_STATUS_IDLE);
                    pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_STATUS_IDLE;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_EVENT_READY. Success (=%d   )\n", EventReady));
                    if(pPort->P2PCfg.P2pEventQueue.LastSetEvent == P2PEVENT_POPUP_SETTING_WINDOWS || pPort->P2PCfg.P2pEventQueue.LastSetEvent == P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO)
                    { 
                        pPort->P2PCfg.LockNego = FALSE;
                        pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_STATUS_IDLE;
                    }
                    else if (pPort->P2PCfg.P2pEventQueue.LastSetEvent == P2PEVENT_CONNECT_COMPLETE)
                    {
                        pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail = 0xffffffff;
                    }
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_EVENT_READY.  (=%d  !!!!!)\n", EventReady));
                }
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pIp(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG MyIp = *(ULONG *) pInformationBuffer;
    
            if(P2P_OFF(pPort))
                break;

            DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_P2P_IP (My ip is =%x)\n", MyIp));               
            if (InformationBufferLength < sizeof(ULONG))
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            else
            {
                pPort->P2PCfg.MyIp = MyIp;
                pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_IP (My Ip =%x)\n", MyIp));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pGoPassphrase(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    P2P_GO_KEY  *pGoKey;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            pGoKey = (P2P_GO_KEY *) pInformationBuffer;
            DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_P2P_GO_PASSPHRASE (InformationBufferLength =%d)\n", InformationBufferLength));
            if (InformationBufferLength < sizeof(P2P_GO_KEY))
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            else
            {
                if ((pGoKey->KeyLength > 0) && (pGoKey->KeyLength <= 64))
                {
                    // Mark key changed here and then disconnect wpa client later(not include persistent key)
                    // If SSID is also changed, it means this connect is different to the previous one. We don't need to call "disconnect".
                    if ((IS_P2P_GO_OP(pPort) || IS_P2P_GO_WPA2PSKING(pPort))  &&
                        ((pPort->P2PCfg.LegacyPSKLen != (UCHAR)pGoKey->KeyLength) ||
                        (!PlatformEqualMemory(pPort->P2PCfg.LegacyPSK, pGoKey->KeyPassphase, min(pPort->P2PCfg.LegacyPSKLen, pGoKey->KeyLength)))))
                    {
                        pPort->P2PCfg.PhraseKeyChanged = TRUE;
                    }

                    // Long storage in legacy part. And PhraseKey is temporary used.
                    pPort->P2PCfg.LegacyPSKLen = (UCHAR)pGoKey->KeyLength;
                    PlatformMoveMemory(pPort->P2PCfg.LegacyPSK, pGoKey->KeyPassphase, 64);
                    
                    pPort->P2PCfg.PhraseKeyLen= (UCHAR)pGoKey->KeyLength;
                    PlatformMoveMemory(pPort->P2PCfg.PhraseKey, pGoKey->KeyPassphase, 64);
                    if ((IS_P2P_GO_OP(pPort) || IS_P2P_AUTOGO(pPort)) && (pPort->P2PCfg.Dpid == DEV_PASS_ID_NOSPEC))
                    {
                        PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.Ssid, pPort->P2PCfg.SSID, 32);
                        pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.SsidLength = pPort->P2PCfg.SSIDLen;
    
                        P2pSetEvent(pAd, pPort, P2PEVENT_DRIVER_INTERNAL_USE);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_P2P_GO_PASSPHRASE (PhraseKeyLen =%d, PhraseKeyChanged=%d)\n", pPort->P2PCfg.PhraseKeyLen, pPort->P2PCfg.PhraseKeyChanged));
                    DBGPRINT(RT_DEBUG_TRACE,(" Key = %x  %x %x\n", *(PULONG)&pPort->P2PCfg.PhraseKey[0], *(PULONG)&pPort->P2PCfg.PhraseKey[4], *(PULONG)&pPort->P2PCfg.PhraseKey[8]));
                }
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetP2pPersist(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            else
            {
                    pPort->CommonCfg.P2pControl.field.EnablePresistent = *(ULONG *)pInformationBuffer;
                    pPort->P2PCfg.LockNego = FALSE; // even throught enable or disable should be FALSE
    

                    P2PUpdateCapability(pAd, pPort);
                    DBGPRINT(RT_DEBUG_TRACE, ("set:: MTK_OID_N6_SET_P2P_PERSIST EnablePresistent = %d", pPort->CommonCfg.P2pControl.field.EnablePresistent));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetBaldEagleSsid(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT_ERR(("SET::MTK_OID_N6_QUERY_BALD_EAGLE_SSID, InformationBufferLength > 32\n"));
                }
                else
                {
                    ULONG BytesReadTemp, BytesNeededTemp;
                    PNDIS_802_11_SSID pBaldEagleSSID = (PNDIS_802_11_SSID)pInformationBuffer;
        
                    // if STA is conncted to an AP, disconnect first.
                    if(!IDLE_ON(pPort))
                    {
                        pAd->MlmeAux.CurrReqIsFromNdis = FALSE; 
                        ndisStatus = DisconnectRequest(pAd, pPort,&BytesReadTemp, &BytesNeededTemp, FALSE);
                    }
                    //
                    // Process MTK_OID_N6_SET_BALD_EAGLE_SSID
                    //
                    if (pBaldEagleSSID->SsidLength == 0)
                    {
                        pAd->StaCfg.BaldEagle.EnableBladEagle = FALSE;
                        DBGPRINT(RT_DEBUG_TRACE, ("SET::MTK_OID_N6_QUERY_BALD_EAGLE_SSID, length of SSID == 0, disable bald eagle mechanism.\n"));
                    }
                    else if(pBaldEagleSSID->SsidLength > MAX_LEN_OF_SSID)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("SET::MTK_OID_N6_SET_BALD_EAGLE_SSID failed, length of SSID > 32!!\n"));
                        ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    }           
                    else // this STA can only connect to a specific AP.
                    {
                        pAd->StaCfg.BaldEagle.EnableBladEagle = TRUE;
                        pAd->StaCfg.BaldEagle.SsidLen = pBaldEagleSSID->SsidLength;
                        PlatformMoveMemory(pAd->StaCfg.BaldEagle.Ssid, pBaldEagleSSID->Ssid, pBaldEagleSSID->SsidLength);
                        DBGPRINT(RT_DEBUG_TRACE, ("SET::MTK_OID_N6_QUERY_BALD_EAGLE_SSID(=%s)\n", pBaldEagleSSID->Ssid));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscExtregMesssage(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE\n"));

                //if(pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
                if(pPort->StaCfg.WscControl.AsExtreg)
                {               
                    if((pInformationBuffer == NULL)||(InformationBufferLength != sizeof(WSC_EXTREG_MSG)))
                    {
                        ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                        
                        DBGPRINT(RT_DEBUG_TRACE, ("size of (WSC_EXTREG_MSG) = %d\n", sizeof(WSC_EXTREG_MSG)));
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE NULL:%d or Invalid Length:%d\n", 
                            (pInformationBuffer == NULL), InformationBufferLength));
                        break;
                    }
                    else
                    {
                        PWSC_EXTREG_MSG pWscExtRegMsg = (PWSC_EXTREG_MSG) pInformationBuffer;
                        SHORT   DataLen;
                        PUCHAR  pData;
                        USHORT  WscType, WscLen;
                        UCHAR   MsgType = 0;
                        UCHAR   Id = 0;
                        SHORT   j;
                        USHORT  VendorExtLen = 0;
                        UCHAR   VendorExtElemLen = 0;
                        PUCHAR  pVendorExtType;

                        //if(pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)
                        //{
                        //  ndisStatus = NDIS_STATUS_INVALID_DATA;
                        //  DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE Invalid State!!!\n"));
                        //  break;
                        //}
                            
                        
                        DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE ExtReg mode\n"));  
                        
                        DataLen = pWscExtRegMsg->Length;
                        pData = pWscExtRegMsg->MsgData;

                        DBGPRINT(RT_DEBUG_TRACE, ("DataLen:%d, ##ProbReq##:%d, UseSelEnr:%x:%x:%x:%x:%x:%x\n", DataLen, pWscExtRegMsg->bProbReq,
                            pWscExtRegMsg->UserSelEnrMAC[0], pWscExtRegMsg->UserSelEnrMAC[1], pWscExtRegMsg->UserSelEnrMAC[2],
                            pWscExtRegMsg->UserSelEnrMAC[3], pWscExtRegMsg->UserSelEnrMAC[4], pWscExtRegMsg->UserSelEnrMAC[5]));

                        DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE ExtReg Msg:\n"));
                        for(j = 0; j < DataLen; j++)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("%02x ", *(pData+j)));
                        }

                        if((pWscExtRegMsg->bProbReq == FALSE) && 
                            (pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP) &&
                            (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
                        {
                            while (DataLen > 4)
                        {
                            WscType = cpu2be16(*((PUSHORT) pData));
                            WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
                            pData  += 4;
                            DataLen -= 4;

                            // Parse M1 WSC type and store to RegData structure
                            switch (WscType)
                            {
                        
                                case WSC_IE_MSG_TYPE:       
                                    MsgType = *pData;
                                    break;
            
                                default:
                                    break;              
                            }

                            if(MsgType != 0)
                            {
                                break;
                            }
                            // Offset to net WSC Ie
                            pData  += WscLen;
                            DataLen -= WscLen;
                        }   
                            
                            if((MsgType == 0x04)&&
                                (pPort->StaCfg.WscControl.WscState >= WSC_STATE_WAIT_M5) &&
                                (PlatformEqualMemory(pPort->StaCfg.WscControl.RegData.EnrolleeInfo.MacAddr, pWscExtRegMsg->EnrolleeMAC, 6)))
                            {
                                DBGPRINT(RT_DEBUG_TRACE, ("ExtReg receive M1 after send M4 and set WscState to WAIT_M1!!!!!!!!\n"));
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                            }
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE MsgType 0x%x!!! put to WscEAPRegistrarAction()\n", MsgType));

                            WscEAPRegistrarAction(pAd, pPort, MsgType, Id, NULL, pWscExtRegMsg, pWscExtRegMsg->UserSelEnrMAC);                      
        
                            //WscPbcEnrTabUpdate(pAd, pWscExtRegMsg->EnrolleeMAC,mVersion, mUUID_E, mDevicePassID, mProbReqLastTime);
                            
                            DataLen = pWscExtRegMsg->Length;
                            pData = pWscExtRegMsg->MsgData;
                        }
                        
                        if((pWscExtRegMsg->bProbReq) || (MsgType == 0x04))
                        {
                            //SHORT j;
                            if(pWscExtRegMsg->bProbReq)
                            {
                                //If the message is probe req, the formate will be 00 50 f2 04 10 4a 00 01 10..
                                //Hence we shift 4 byte(00 50 f2 04) first if the message is probe req.
                                //DataLen -= 4;
                                //pData += 4;
                            }
                            DBGPRINT(RT_DEBUG_TRACE, ("receive ProbReq or M1 to record the enrollee MAC:..%x:%x!!\n", pWscExtRegMsg->EnrolleeMAC[4], pWscExtRegMsg->EnrolleeMAC[5]));
                            //Set Probe Request to external registrar
                            //if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR_UPNP)
                            //  &&(pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE)
                            //  &&(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
                            //  /*&&(pPort->StaCfg.WscControl.WscState < WSC_STATE_WAIT_M1)*/)
                            {
                                UCHAR   mVersion = 0x10;
                                UCHAR   mUUID_E[16] = {0};
                                USHORT  mDevicePassID = 0xff;
                                ULONGLONG   mProbReqLastTime = 0;

                                if(pPort->StaCfg.WscControl.WscPbcEnrCount >= PBC_ENR_TAB_SIZE)
                                {
                                    DBGPRINT(RT_DEBUG_TRACE, ("Invalid WscPbcEnrCount !!!\n"));
                                    break;
                                }
                                else
                                {
                                    for(j = 0; j < DataLen; j++)
                                    {
                                        DBGPRINT(RT_DEBUG_TRACE, ("%x\n", *(pData+j)));
                                    }
                                    
                                    //Skip WPS OUI if it is included.
                                    if(PlatformEqualMemory(pData, WPS_OUI, 4))
                                    {
                                        pData  += 4;
                                        DataLen -= 4;
                                    }

                                    while (DataLen > 4)
                                    {
                                        WscType = cpu2be16(*((PUSHORT) pData));
                                        WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
                                        pData  += 4;
                                        DataLen -= 4;

                                        // Parse M1 WSC type and store to RegData structure
                                        switch (WscType)
                                        {
                                            case WSC_IE_UUID_E:
                                                PlatformMoveMemory(mUUID_E, pData, 16);
                                                DBGPRINT(RT_DEBUG_TRACE, ("uuid-e: %x %x %x %x...\n", *(pData+0), *(pData+1), *(pData+2), *(pData+3)));
                                                break;
                                                
                                            case WSC_IE_DEV_PASS_ID:
                                                mDevicePassID = cpu2be16(*((PUSHORT) pData));
                                                DBGPRINT(RT_DEBUG_TRACE, ("DEV_PASS_ID:0x%02x\n", mDevicePassID));
                                                break;

                                            case WSC_IE_VENDOR_EXT:
                                                VendorExtLen = WscLen;
                                                if (((*(pData) * 256 + *(pData+1)) * 256 + (*(pData+2))) == WSC_SMI)
                                                {
                                                    VendorExtLen -= 3;  //WSC SMI
                                                    pVendorExtType = pData;
                                                    pVendorExtType += 3;
                                                    while(VendorExtLen > 2)
                                                    {
                                                        VendorExtElemLen = *(pVendorExtType+1);
                                                                            
                                                        switch(*pVendorExtType)
                                                        {
                                                            case WSC_IE_VERSION2:
                                                                mVersion = *(pVendorExtType+2);
                                                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse Version2 type 0x%x!!\n", __FUNCTION__, mVersion));
                                                                break;
                                                            default:
                                                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse UNKNOWN type!! 0x%02x\n", __FUNCTION__, *pVendorExtType));
                                                                break;
                                                        }
                                                        pVendorExtType += (VendorExtElemLen + 2);
                                                        VendorExtLen -= (VendorExtElemLen + 2);
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                        pData  += WscLen;
                                        DataLen -= WscLen;
                                    }
                                    
                                    NdisGetCurrentSystemTime((PLARGE_INTEGER)&mProbReqLastTime);

                                    //PBC device, need to update the PBCEnrTab
                                    if((mDevicePassID == 0x04) && (!MAC_ADDR_EQUAL(pWscExtRegMsg->EnrolleeMAC, pPort->CurrentAddress))) 
                                    {
                                        WscPbcEnrTabUpdate(pAd, pPort, pWscExtRegMsg->EnrolleeMAC, mVersion, mUUID_E, mDevicePassID, mProbReqLastTime);
                                    }
                                    DBGPRINT(RT_DEBUG_TRACE, ("WscPbcEnrCount = %d\n",pPort->StaCfg.WscControl.WscPbcEnrCount));
                                }
                            }
                        }
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetSuppressScan(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            ULONG SuppressScanMode = *(ULONG *) pInformationBuffer;
            DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_SUPPRESS_SCAN (=0x%x)\n", SuppressScanMode));
    
            if (InformationBufferLength < sizeof(ULONG))
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            else
            {
                pAd->StaCfg.OzmoDeviceSuppressScan = SuppressScanMode;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_SUPPRESS_SCAN (=0x%x)\n", pAd->StaCfg.OzmoDeviceSuppressScan));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTdtEvent(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTdtEventEnd(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTdtDisable(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTdtEnable(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_INVALID_OID;
    FUNC_ENTER;

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetCustomerBtRadioStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

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
                break;
            }
            else
            {
                ULONG BTLEDStatus = *(PULONG)pInformationBuffer;
    
                if (pPort->CommonCfg.BTLEDStatus != BTLEDStatus)
                {
                    pPort->CommonCfg.BTLEDStatus = BTLEDStatus;
                }
                LedCtrlSetBlinkMode(pAd, pAd->HwCfg.BackupLedStatus);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_CUSTOMER_BT_RADIO_STATUS(=%d), LastLEDStatus=%d\n", pPort->CommonCfg.BTLEDStatus, pAd->HwCfg.BackupLedStatus));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetSsNum(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


MtkOidSetTxbfEtxBfTimeout(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: MTK_OID_N5_SET_TXBF_ETX_BF_TIMEOUT\n", __FUNCTION__));

            if (isEnableETxBf(pAd))
            {
                ndisStatus = TxBfOidSetETxBfTimeout(pAd, pInformationBuffer, InformationBufferLength);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: MTK_OID_N5_SET_TXBF_ETX_BF_TIMEOUT: TxBf is disabled.\n", __FUNCTION__));
    
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTxbfEtxBfEn(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: MTK_OID_N5_SET_TXBF_ETX_BF_EN\n", __FUNCTION__));
            ndisStatus = TxBfOidSetETxEn(pAd, pInformationBuffer, InformationBufferLength);
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTxbfEtxBfNonCompressedFeedback(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: MTK_OID_N5_SET_TXBF_ETX_BF_NON_COMPRESSED_FEEDBACK\n", __FUNCTION__));
            ndisStatus = TxBfOidSetETxBfNonCompressedFeedback(pAd, pInformationBuffer, InformationBufferLength);
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetReconfigAp(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != (sizeof(ULONG)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_RECONFIG_AP Failed since Invalid Length!!\n"));
                }
                else 
                {
                    pPort->StaCfg.WscControl.ReconfigAPSelbyUser = (BOOLEAN)*((PULONG) pInformationBuffer);
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_RECONFIG_AP ReconfigAPSelbyUser = %d\n", pPort->StaCfg.WscControl.ReconfigAPSelbyUser));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetDisableAntennaDiversity(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Set::Disable antenna diversity\n"));
            pAd->HwCfg.NicConfig2.field.AntDiv = ANT_DIV_CONFIG_DISABLE;
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetFixedMainAntenna(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_FIXED_MAIN_ANTENNA\n"));
            pAd->HwCfg.NicConfig2.field.AntDiv = ANT_DIV_CONFIG_ENABLE;
            AsicSetRxAnt(pAd, 0);
            pAd->HwCfg.NicConfig2.field.AntDiv = ANT_DIV_CONFIG_DISABLE;
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetFixedAuxAntenna(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_FIXED_AUX_ANTENNA\n"));
            pAd->HwCfg.NicConfig2.field.AntDiv = ANT_DIV_CONFIG_ENABLE;
            AsicSetRxAnt(pAd, 1);
            pAd->HwCfg.NicConfig2.field.AntDiv = ANT_DIV_CONFIG_DISABLE;
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetFixedRate(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    BOOLEAN         OidSubFunStatus;                    // The return status of OID sub function
    ULONG           LenReturnedFormSubFunc = 0;         // The length of SubFunction needed
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            OidSubFunStatus = OidSetTxFixedRate(
                                    pAd,
                                    PortNumber,
                                    pInformationBuffer,
                                    InformationBufferLength,
                                    &LenReturnedFormSubFunc
                                    );  
            if(!OidSubFunStatus)
            {
                if(LenReturnedFormSubFunc)
                {
                    *BytesNeeded = LenReturnedFormSubFunc;
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                ndisStatus = NDIS_STATUS_FAILURE;                
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set: MTK_OID_N6_SET_FIXED_RATE, status=0x%08x\n", ndisStatus));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetTestTxNullFrame(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(BOOLEAN))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_TEST_TX_NULL_FRAME. Size isn't correct.(%d =! %d)\n", InformationBufferLength, sizeof(ULONG)));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                pAd->TrackInfo.bTxNULLFrameTest = *(BOOLEAN *) pInformationBuffer;
                //XmitSendNullFrameFor7603Test(pAd, 0, 0);
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_TEST_TX_NULL_FRAME (=%x)\n", ndisStatus));
        } while (FALSE);
    }
    else
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetRadio(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(BOOLEAN))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_RADIO. Size isn't correct.(%d =! %d)\n", InformationBufferLength, sizeof(BOOLEAN)));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                BOOLEAN     bRadio = *(BOOLEAN *) pInformationBuffer;

                SendRadioOnOffCMD(pAd, (bRadio ? PWR_RadioOn : PWR_RadioOff));
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_RADIO (bRadio=%d)\n", bRadio));
                if (bRadio == PWR_RadioOff)
                {
                    pAd->TrackInfo.bStopCMDEventReceive = TRUE;
                }
                else if (bRadio == PWR_RadioOn)
                {
                    NdisCommonGenericDelay(5000);
                    
                    mt_mac_init(pAd);

                    // Init WLan Table
                    mt_hw_tb_init(pAd, FALSE);

                    {
                        PMP_PORT  pPort = pAd->PortList[PORT_0];
                        COPY_MAC_ADDR(pPort->CurrentAddress, pAd->HwCfg.CurrentAddress);    
                    }

                    NdisCommonMappingPortToBssIdx(pPort, UPDATE_MAC, DO_NOTHING);

                    // Set BW
                    MtAsicSetBW(pAd, BW_20);

                    // Set Tx Stream
                    MtAsicSetTxStream(pAd, pAd->HwCfg.Antenna.field.TxPath);

                    // Set Rx Stream
                    MtAsicSetRxStream(pAd, pAd->HwCfg.Antenna.field.RxPath);
                    
                    // CCA workaround
                    if (1)
                    {
                        ULONG MACValue;

                        HW_IO_READ32(pAd, TMAC_TRCR, (UINT32 *)&MACValue);
                        DBGPRINT(RT_DEBUG_TRACE, ("TMAC_TRCR : MACValue = %x\n", MACValue));
                        MACValue |= (0x8 << 28);
                        DBGPRINT(RT_DEBUG_TRACE, ("TMAC_TRCR : MACValue = %x\n", MACValue));
                        HW_IO_WRITE32(pAd, TMAC_TRCR, (UINT32)MACValue);
                    }

                    // increase Tx A-MPDU timeout
                    if (0)
                    {
                        ULONG MACValue;
                        MACValue = 0x005c0028;
                        HW_IO_WRITE32(pAd, 0x60130098, (UINT32)MACValue);
                    }

                    // Rx Throughput workaround
                    if (1)
                    {
                        ULONG MACValue;

                        HW_IO_READ32(pAd, 0x60204600, (UINT32 *)&MACValue);
                        MACValue |= (1 << 23);
                        MACValue &= ~(1 << 7);
                        HW_IO_WRITE32(pAd, 0x60204600, (UINT32)MACValue);
                    }
                    
                    // Tx under-run
                    if (1)
                    {
                        ULONG MACValue;

                        HW_IO_READ32(pAd, 0x60204200, (UINT32 *)&MACValue);
                        MACValue |= (3 << 8);
                        MACValue |= (8 << 0);
                        HW_IO_WRITE32(pAd, 0x60204200, (UINT32)MACValue);
                    }

                    // Enable Tx/Rx
                    MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

                    MlmeInfoSetPhyMode(pAd, pPort, pPort->CommonCfg.PhyMode,TRUE);

                    pPort->CommonCfg.bAutoTxRateSwitch = TRUE;

                    // set Guard time
                    if (0)
                    {
                        ULONG MACValue;
                        
                        MACValue = 0x40404040;
                        HW_IO_WRITE32(pAd, ARB_GTQR0, MACValue);
                    
                        MACValue = 0x00004000;
                        HW_IO_WRITE32(pAd, ARB_GTQR2, MACValue);
                    }

                    if (1)
                    {
                        ULONG MACValue;
                        
                        MACValue = 0xfb89c0;
                        HW_IO_WRITE32(pAd, 0x21244, (UINT32)MACValue);
                        
                    }

                    // UDMA Rx Aggregation Setting
                    if (1)
                    {
                        U3DMA_WLCFG         U3DMAWLCFG;
                        HW_IO_READ32(pAd, UDMA_WLCFG_0, (UINT32 *)&U3DMAWLCFG.word);
                        DBGPRINT(RT_DEBUG_ERROR,("1 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
                        U3DMAWLCFG.field.WL_RX_AGG_EN = 1;
                        U3DMAWLCFG.field.WL_RX_AGG_LMT = 0x18;  // 24K
                        U3DMAWLCFG.field.WL_RX_AGG_TO = 0x30;   // 48us
                        DBGPRINT(RT_DEBUG_ERROR,("2 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
                        HW_IO_WRITE32(pAd, UDMA_WLCFG_0, (UINT32)U3DMAWLCFG.word);
                    }
                    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                    //AsicSwitchChannel(pAd, 1, FALSE);
                }
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetPowerManagement(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(ULONG))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_POWER_MANAGEMENT. Size isn't correct.(%d =! %d)\n", InformationBufferLength, sizeof(ULONG)));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            }
            else
            {
                ULONG PmState = *(ULONG *)pInformationBuffer;
    
                if (PmState == 0x41)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_POWER_MANAGEMENT PM4 On, just to test FW CMD\n"));
                    SendPowerManagementCMD(pAd, PM4, PWR_PmOn, pAd->HwCfg.CurrentAddress, 1, 1);
                }
                else if (PmState == 0x42)
                {
                    SendPowerManagementCMD(pAd, PM4, PWR_PmOff, pAd->HwCfg.CurrentAddress, 1, 1);
                }
                else if (PmState == 0x51)
                {
                    //DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_POWER_MANAGEMENT PM5 On\n"));
                    //SendPowerManagementCMD(pAd, PM5, PWR_PmOn, pAd->HwCfg.CurrentAddress, 0, 1);
                    //pAd->TrackInfo.bStopCMDEventReceive = TRUE;
                }
                else if (PmState == 0x52)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_POWER_MANAGEMENT PM5 Off\n"));
                    SendPowerManagementCMD(pAd, PM5, PWR_PmOff, pAd->HwCfg.CurrentAddress, 0, 1);
                    pAd->TrackInfo.bStopCMDEventReceive = FALSE;
                }
                else if (PmState == 0x61)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_POWER_MANAGEMENT PM6 On\n"));
                    SendRadioOnOffCMD(pAd,PWR_RadioOff);
                    pAd->TrackInfo.bStopCMDEventReceive = TRUE;
                    //SendPowerManagementCMD(pAd, PM5, PWR_PmOff, pAd->HwCfg.CurrentAddress, 0, 1);
                }
                else if (PmState == 0x62)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_POWER_MANAGEMENT PM6 Off\n"));
                    SendRadioOnOffCMD(pAd,PWR_RadioOn);
                    pAd->TrackInfo.bStopCMDEventReceive = FALSE;
                    //SendPowerManagementCMD(pAd, PM5, PWR_PmOff, pAd->HwCfg.CurrentAddress, 0, 1);
                }
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetRfTest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    BOOLEAN Cancelled;
    
    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != (sizeof(WSC_MODE)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    PWSC_MODE pWscMode = (PWSC_MODE) pInformationBuffer;
                    ULONG BSSEntryIndex = BSS_NOT_FOUND;
                    PBSS_ENTRY pBSSEntry = NULL;
                    BOOLEAN     bP2pPBCAsEnrollee = FALSE;
                    PMP_PORT  pWscPort = (PMP_PORT) pPort; // UI set wsc mode via Port0
                    UCHAR   AllZeroPin4[4] = {0, 0, 0, 0};
                    UCHAR   AllZeroPin8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                    
                    if((pWscMode->WscConfigMode == WSC_ConfMode_REGISTRAR_UPNP)&&(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Ignore OID MTK_OID_N6_SET_WSC_MODE with ER mode when doing WPS\n"));
                        break;
                    }

                    // Reply the first M1 by M2D, reply M1 by M2 after M2D.
                    //pPort->StaCfg.WscControl.ERM1Count = 0;

                    // Reset WPS LED mode state.
                    pPort->StaCfg.WscControl.WscLEDMode = 0;
                    pPort->StaCfg.WscControl.WscLastWarningLEDMode = 0;

                    // Save WSC ConfigMode
                    //TODO: WSC_ConfMode_REGISTRAR_UPNP is unavailable !!!
                    pPort->StaCfg.WscControl.WscConfMode = (ULONG)pWscMode->WscConfigMode;

                    // Save WSC Mode inforamtion
                    pPort->StaCfg.WscControl.WscMode = (pWscMode->WscMode & 0xf0000000);

                    // Disable HwPBC status
                    pAd->StaCfg.HwPBCState = FALSE;
                    
                    // For WSC, scan action shall be controlled by WSC state machine.
                    pAd->StaCfg.ScanCnt = 3;
                    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_IDLE;

                    // Reset AP's BSSID.
                    PlatformZeroMemory(pPort->StaCfg.WscControl.WscAPBssid, (sizeof(UCHAR) * MAC_ADDR_LEN));

                    // Reset the WPS walk time.
                    pPort->StaCfg.WscControl.bWPSWalkTimeExpiration = FALSE;

                    //Clear the WPS bWPSSuccessandLinkup
                    pPort->StaCfg.WscControl.bWPSSuccessandLinkup = FALSE;

                    if(pWscMode->WscConfigMode != WSC_ConfMode_REGISTRAR_UPNP)
                    {
                        pPort->StaCfg.WscControl.AsExtreg = FALSE;
                        DBGPRINT(RT_DEBUG_TRACE, ("WscControl.AsExtreg is FALSE.\n"));
                    }
                    
                    if((pWscMode->WscConfigMode == WSC_ConfMode_REGISTRAR_UPNP) &&
                        (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)  &&
                        (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Ignore OID MTK_OID_N6_SET_WSC_MODE with ER mode when doing WPS\n"));
                        break;
                    }
                    
                    //Initialize the WSC AP we will connect before doing WSC.
                    pPort->StaCfg.WscControl.WSCRegistrarVersion = 0x0;
                    // TODO: We might to disable autoconnect here, and reenable it after getting the config file

                    // Skip the detection of pbc session overlapping if P2P Client.
                    if (IS_P2P_ENROLLEE(pPort) && (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE))
                    {
                        bP2pPBCAsEnrollee = TRUE;
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE Skip the detection of pbc session overlapping(bP2pPBCAsEnrollee=%d) \n", bP2pPBCAsEnrollee));
                    }

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE P2P State= %s) \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));

                    // Change the port from GUI to p2p port number when p2p wps is in progess.
                    if (IS_P2P_ENROLLEE(pPort))
                    {
                        pWscPort = pAd->PortList[pPort->P2PCfg.PortNumber];
                        
                        // Blocking auto-reconnection during this period. (10 sec)
                        if (pAd->OpMode == OPMODE_STAP2P)
                            pPort->P2PCfg.P2pCounter.CounterAfterSetWscMode = 100;

                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE Use P2P PortNumber= %d) \n", pPort->P2PCfg.PortNumber));
                    }
                    
                    // Check for WSC connection mode
                    if ((pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_REGISTRA_SPEC) || (pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_USER_SPEC) ||(pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE)
                        || (bP2pPBCAsEnrollee == TRUE))
                    {
                        //WPS External Registrar using PIN.
                        /*if((pWscMode->WscConfigMode == WSC_ConfMode_REGISTRAR_UPNP))
                        {
                            
                            if(!(INFRA_ON(pPort)))
                            {
                                ndisStatus = NDIS_STATUS_INVALID_DATA;
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_INVALID_SSID;
                                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_WSC_SET_MODE No Connection as External Registrar!!!\n"));
                                break;
                            }
                            
                        }*/

                        // In the viewpoint of the user and the Ralink UI, 
                        // the STA runs as a registrar using WPS PIN mode.
                        if ((pWscMode->WscConfigMode == WSC_ConfMode_REGISTRAR) && 
                            ((pWscMode->WscMode & 0xf0000000) == WSC_PIN_MODE))
                        {
                            // Find the desired WPS AP.
                            BSSEntryIndex = BssTableSearch(pAd, pWscPort, &pAd->ScanTab, pWscMode->Bssid, pWscMode->Channel);   
                            if (BSSEntryIndex == BSS_NOT_FOUND) // Not found.
                            {
                                // do nothing.
                            }
                            else // Found.
                            {
                                pBSSEntry = &pAd->ScanTab.BssEntry[BSSEntryIndex];

                                // The desired AP is the T Home Speedport W 303V WPS AP using the PIN mode in the configured state.
                                if (pBSSEntry->bTHome303V_ConfiguredPINMode == TRUE)
                                {
                                    // Patch: The STA runs as an enrollee.
                                    // When the desired WPS AP is the T Home Speedport W 303V WPS AP and it is configured as an enrollee in the configured state, 
                                    // it cannot receive an EAP-Response/Identity using the name "WFA-SimpleConfig-Registrar-1-0".
                                    // Intead, the STA's EAP-Response/Identity should use the name "WFA-SimpleConfig-Enrollee-1-0" and 
                                    // should run as an enrollee.
                                    pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_ENROLLEE;

                                    // The PIN code would be set in the following code fragment (from the Ralink UI).

                                    DBGPRINT(RT_DEBUG_TRACE, ("%s (SET::MTK_OID_N6_SET_WSC_MODE): (Simulation) The STA runs as an enrollee for the T Home Speedport W 303V WPS AP.\n", __FUNCTION__));
                                }
                                else // Normal WPS AP.
                                {
                                    // do nothing.
                                    DBGPRINT(RT_DEBUG_TRACE, ("%s (SET::MTK_OID_N6_SET_WSC_MODE): Normal WPS AP.\n", __FUNCTION__));
                                }
                            }
                        }

                        // 1. PIN mode, verify SSID sanity
                        //    UI must set a SSID, no "any" allowed
                        if (sizeof(pWscMode->Bssid) != sizeof(NDIS_802_11_MAC_ADDRESS))
                        {
                            ndisStatus = NDIS_STATUS_INVALID_DATA;
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                            pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;
                            pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE;
                            pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_INVALID_SSID;
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE Invalid PIN Mode BSSID\n"));
                            break;
                        }
                        else 
                        {
                            ULONG BytesReadTemp, BytesNeededTemp;
                
                            // tell CNTL state machine to call NdisMSetInformationComplete() after completing
                            // this request, because this request is initiated by NDIS.
                            pAd->MlmeAux.CurrReqIsFromNdis = FALSE; 

                            // Disconnect the AP which the STA is connected.
                            if(!(pWscMode->WscConfigMode == WSC_ConfMode_REGISTRAR_UPNP))
                            {
                                DisconnectRequest(pAd, pWscPort, &BytesReadTemp, &BytesNeededTemp, FALSE);
                            }

                            // Start WPS seesion.
                            pPort->StaCfg.WscControl.bWPSSession = TRUE;
                            // Backup the AP's BSSID
                            if(pWscMode->WscConfigMode != WSC_ConfMode_REGISTRAR_UPNP)
                            {   
                                PlatformMoveMemory(pPort->StaCfg.WscControl.WscAPBssid, pWscMode->Bssid, (sizeof(UCHAR) * MAC_ADDR_LEN));
                            }

                            // Reset allowed scan retries
                            // For WSC, this AP already in SSID table, scan shall not be allowed
                            pAd->StaCfg.ScanCnt = 3;


                            // 1. Cancel old timer to prevent use push continuously
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscConnectTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscScanTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPINTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscLEDTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregPBCTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscExtregScanTimer, &Cancelled);
                            PlatformCancelTimer(&pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer, &Cancelled);

                            // Set WSC state to WSC_STATE_INIT
                            pPort->StaCfg.WscControl.WscState = WSC_STATE_INIT;
                            
                            // Init Registrar pair structures
                            WscInitRegistrarPair(pAd, pWscPort);

                            // Make sure PIN haas been copied
                            if (bP2pPBCAsEnrollee == TRUE) 
                            {
                                //default is using PIN 8 digit.
                                pPort->StaCfg.WscControl.RegData.PINLen = 4;
                            
                                // For PBC, the PIN is all '0'
                                pPort->StaCfg.WscControl.RegData.PIN[0] = '0';
                                pPort->StaCfg.WscControl.RegData.PIN[1] = '0';
                                pPort->StaCfg.WscControl.RegData.PIN[2] = '0';
                                pPort->StaCfg.WscControl.RegData.PIN[3] = '0';
                                pPort->StaCfg.WscControl.RegData.PIN[4] = '0';
                                pPort->StaCfg.WscControl.RegData.PIN[5] = '0';
                                pPort->StaCfg.WscControl.RegData.PIN[6] = '0';
                                pPort->StaCfg.WscControl.RegData.PIN[7] = '0';
                            }
                            else    
                            {
                                if((pWscMode->PIN[0] == 0xff) && (pWscMode->PIN[1] == 0xff) &&
                                    (pWscMode->PIN[2] == 0xff) && (pWscMode->PIN[3] == 0xff))
                                {
                                    if(!PlatformEqualMemory(AllZeroPin4, &pWscMode->PIN[4], 4))
                                    {
                                        PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, &pWscMode->PIN[4], 4);
                                        pPort->StaCfg.WscControl.RegData.PINLen = 2;
                                        DBGPRINT(RT_DEBUG_TRACE, ("Wsc_Set_Mode PIN is 4 digit!!!\n"));
                                    }
                                    else
                                    {
                                        pPort->StaCfg.WscControl.RegData.PINLen = 0;
                                    }
                                }
                                else
                                {
                                    if(!PlatformEqualMemory(AllZeroPin8, &pWscMode->PIN[0], 8))
                                    {
                                        PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, pWscMode->PIN, 8);
                                        pPort->StaCfg.WscControl.RegData.PINLen = 4;
                                        DBGPRINT(RT_DEBUG_TRACE, ("Wsc_Set_Mode PIN is 8 digit!!!\n"));
                                    }
                                    else
                                    {
                                        pPort->StaCfg.WscControl.RegData.PINLen = 0;
                                    }
                                }
                                DBGPRINT(RT_DEBUG_TRACE, ("PIN code: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                                    pWscMode->PIN[0], pWscMode->PIN[1], pWscMode->PIN[2], pWscMode->PIN[3],
                                    pWscMode->PIN[4], pWscMode->PIN[5], pWscMode->PIN[6], pWscMode->PIN[7]));
                            }

                            // 2. Set 2 min timout routine
                            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscConnectTimer, WSC_REG_SESSION_TIMEOUT);

                            
                            // Set WSC state to WSC_STATE_START
                            if(pWscMode->WscConfigMode == WSC_ConfMode_REGISTRAR_UPNP)
                            {
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_WAIT_M1;
                                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_LINK_UP;
                            }
                            else
                            {
                                pPort->StaCfg.WscControl.WscState = WSC_STATE_START;
                                pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_START_ASSOC;

                                //For mode 6, customer wants wps led can be started as WPS is staring.
                                LedCtrlSetBlinkMode(pAd, LED_WPS);
                            }
                            
                            if(pWscMode->WscConfigMode != WSC_ConfMode_REGISTRAR_UPNP)
                            {
                                pPort->StaCfg.WscControl.bIndicateConnNotTurnOff = FALSE;

                                // The protocol is connecting to a partner.
                                LedCtrlSetBlinkMode(pAd, LED_WPS_IN_PROCESS);

                                if (pWscPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
                                {
                                    DBGPRINT(RT_DEBUG_TRACE, ("%s: !!! MLME busy, reset MLME state machine !!!\n", __FUNCTION__));
                                    MlmeRestartStateMachine(pAd, pWscPort);
                                    
                                }
        
                                // Store channel for BssidTable sort
                                pPort->ScaningChannel = pWscMode->Channel;

                                // Saved the WSC AP channel.
                                // This is because some dual-band APs have the same BSSID in different bands and 
                                // the Ralink UI needs this information to connect the AP with correct channel.
                                pPort->StaCfg.WscControl.WscAPChannel = pWscMode->Channel;

                                // Reset auto-connect limitations
                                pAd->MlmeAux.ScanForConnectCnt = 0;

                                // Enqueue BSSID connection command
                                MlmeEnqueue(pAd, 
                                            pWscPort,
                                            MLME_CNTL_STATE_MACHINE, 
                                            MTK_OID_N5_SET_BSSID,
                                            sizeof(NDIS_802_11_MAC_ADDRESS),
                                            (PVOID) &pWscMode->Bssid);

                                ndisStatus = NDIS_STATUS_SUCCESS;
                            }
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE (SsidLen=%d,Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x)\n", 
                                    pWscMode->WscSSID.SsidLength, pWscMode->WscSSID.Ssid, 
                                    pWscMode->Bssid[0], pWscMode->Bssid[1], pWscMode->Bssid[2], pWscMode->Bssid[3], pWscMode->Bssid[4], pWscMode->Bssid[5]));
                        
                        }
                        
                    }
                    else if (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE)
                    {
                        ULONG BytesReadTemp, BytesNeededTemp;
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE SW PBC\n"));
                        
                        if(pWscMode->WscConfigMode != WSC_ConfMode_REGISTRAR_UPNP)
                        {
                            // Disconnect the AP which the STA is connected.
                            DisconnectRequest(pAd,pWscPort, &BytesReadTemp, &BytesNeededTemp, FALSE);
                        }

                        // Start WPS session.
                        pPort->StaCfg.WscControl.bWPSSession = TRUE;

                        // WPS - SW PBC
                        WscPushPBCAction(pAd, pWscPort);

                        // Set status, remmeber to change StateMachineTouched to TRUE
                        ndisStatus = NDIS_STATUS_SUCCESS;

                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE SW PBC\n"));

                    }
                    else
                    {
                        // Not supported WSC connection mode, set error message
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_MODE Invalid mode %d\n", pWscMode->WscMode));
                        ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscAssociateIe(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != (sizeof(ULONG)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    if (*((PULONG) pInformationBuffer) == 1)
                        pPort->StaCfg.WscControl.WscEnAssociateIE = TRUE; 
                    else
                        pPort->StaCfg.WscControl.WscEnAssociateIE = FALSE;    
        
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_ASSOCIATE_IE %d\n", pPort->StaCfg.WscControl.WscEnAssociateIE));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscProbereqIe(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != (sizeof(ULONG)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    if (*((PULONG) pInformationBuffer) == 1)
                        pPort->StaCfg.WscControl.WscEnProbeReqIE = TRUE;  
                    else
                        pPort->StaCfg.WscControl.WscEnProbeReqIE = FALSE; 
        
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_PROBEREQ_IE %d\n", pPort->StaCfg.WscControl.WscEnProbeReqIE));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscCustomizedIeParam(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC WscSetCustomizedIEParam()
                PUCHAR pParam;
                PVOID tmpBuf = NULL;
                do
                {
                    *BytesWritten = 0;
                    *BytesNeeded = 0;

                    pParam = pInformationBuffer;

                
                    //
                    // Verify IE blob length
                    //
                    if (InformationBufferLength > 0)
                    {
                        PlatformAllocateMemory(pAd,
                        &tmpBuf,  
                        InformationBufferLength);

                        if (tmpBuf == NULL) 
                        {
                            *BytesWritten = InformationBufferLength;
                            ndisStatus = NDIS_STATUS_RESOURCES;
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: NDIS_STATUS_RESOURCES size=%d\n" ,__FUNCTION__, pPort->StaCfg.WscControl.WscCustomizedIESize));
                            break;
                        }
        
                        PlatformMoveMemory(tmpBuf, 
                        pParam,
                        InformationBufferLength);
                    }

                        if (pPort->StaCfg.WscControl.WscCustomizedIEData)
                        {
                         PlatformFreeMemory(pPort->StaCfg.WscControl.WscCustomizedIEData,  pPort->StaCfg.WscControl.WscCustomizedIESize);
                        }

                      // Save the parameters
                    pPort->StaCfg.WscControl.WscCustomizedIESize = InformationBufferLength;
                    pPort->StaCfg.WscControl.WscCustomizedIEData = tmpBuf;

                }while(FALSE);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: size=%d\n" ,__FUNCTION__, pPort->StaCfg.WscControl.WscCustomizedIESize));
                
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_CUSTOMIZED_IE_PARAM ndisStatus = %d \n", ndisStatus));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscProfile(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
                    PlatformMoveMemory(&pPort->StaCfg.WscControl.WscProfile, pInformationBuffer, sizeof(WSC_PROFILE));
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_PROFILE\n"));
                    DBGPRINT(RT_DEBUG_TRACE, ("Profile0 MAC: %x %x %x #### Profile1:%x %x %x\n", pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[3],
                        pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[4],pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr[5],
                        pPort->StaCfg.WscControl.WscProfile.Profile[1].MacAddr[3],
                        pPort->StaCfg.WscControl.WscProfile.Profile[1].MacAddr[4],
                        pPort->StaCfg.WscControl.WscProfile.Profile[1].MacAddr[5]));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscActiveProfile(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != (sizeof(ULONG)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    if (*((PULONG) pInformationBuffer) == 1)
                        pPort->StaCfg.WscControl.WscEnProfile = TRUE; 
                    else
                        pPort->StaCfg.WscControl.WscEnProfile = FALSE;    
        
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_ACTIVE_PROFILE %d\n", pPort->StaCfg.WscControl.WscEnProfile));  
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscCustomizeDevinfo(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(WPS_CUSTOMIZE_DEVICE_INFO))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                }
                else 
                {
                    PlatformMoveMemory(&pPort->StaCfg.WscControl.CustomizeDevInfo, pInformationBuffer, sizeof(WPS_CUSTOMIZE_DEVICE_INFO));
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_CUSTOMIZE_DEVINFO, DeviceName=%s, Manufacturer=%s, ModelName=%s\n", 
                        pPort->StaCfg.WscControl.CustomizeDevInfo.DeviceName, pPort->StaCfg.WscControl.CustomizeDevInfo.Manufacturer, pPort->StaCfg.WscControl.CustomizeDevInfo.ModelName));
        
                    DBGPRINT(RT_DEBUG_TRACE, ("Customized Device Info(in Hex):\n"));
                    for (i = 0; i < sizeof(WPS_CUSTOMIZE_DEVICE_INFO); i++)
                    {
                        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x-", *((PUCHAR)pInformationBuffer+i)));
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("\n")); 
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetPreferredWpsApPhyType(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_PREFERRED_WPS_AP_PHY_TYPE: NDIS_STATUS_INVALID_LENGTH "  
                        "(InformationBufferLength = %d; *BytesNeeded = %d)\n", InformationBufferLength, *BytesNeeded));
                    break;
                }
                else
                {
                    ULONG preferredWPSAPPhyType = (*(PULONG)( pInformationBuffer));
        
                    if (preferredWPSAPPhyType >= PREFERRED_WPS_AP_PHY_TYPE_MAXIMUM)
                    {
                        ndisStatus = NDIS_STATUS_INVALID_DATA;
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_PREFERRED_WPS_AP_PHY_TYPE: NDIS_STATUS_INVALID_DATA (%d)", preferredWPSAPPhyType));
                        break;
                    }
                    else
                    {
                        pPort->StaCfg.WscControl.PreferrredWPSAPPhyType = preferredWPSAPPhyType;
                        ndisStatus = NDIS_STATUS_SUCCESS;
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_PREFERRED_WPS_AP_PHY_TYPE (%d)", preferredWPSAPPhyType));
                        break;
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscWpsStateMachineTermination(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                ULONG BytesReadTemp = 0, BytesNeededTemp = 0;
                if (InformationBufferLength != sizeof(ULONG))
                {
                    *BytesNeeded = sizeof(ULONG);
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_WPS_STATE_MACHINE_TERMINATION: NDIS_STATUS_INVALID_LENGTH "  
                        "(InformationBufferLength = %d; *BytesNeeded = %d)\n", InformationBufferLength, *BytesNeeded));
                    break;
                }
                else
                {
                    ULONG WPSTerminationType = (*(PULONG)( pInformationBuffer));
                    // Disconnect and stop WPS state machine.
                    DisconnectRequest(pAd, pPort, &BytesReadTemp, &BytesNeededTemp, FALSE);

                    // Complete WPS session.
                    pPort->StaCfg.WscControl.bWPSSession = FALSE;
                    if(WPSTerminationType == 0x00)
                    {
                        // Session overlap detected LED indication.
                        pPort->StaCfg.WscControl.bSessionOverlap = TRUE;
                        LedCtrlSetBlinkMode(pAd, LED_WPS_SESSION_OVERLAP_DETECTED);
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_AFTER_SESSION_OVERLAP);
                        if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("LED4200 check: %s %d \n",__FUNCTION__,__LINE__));
                            // Turn off the WPS successful LED pattern after 10 seconds.
                            PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, 10000);
                        } 
                        else
                        {
                        // Turn off the WPS LED after 15 seconds.
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_WPS_SESSION_OVERLAP_DETECTED_TIMEOUT);
                    }
                    }
                    if(WPSTerminationType == 0x01)
                    {
                        pPort->StaCfg.WscControl.bErrDeteWPSTermination = TRUE;
                        // Gemtek  UI Err detection, turn on WPS error LED.
                        //NdisCommonGenericDelay(50000);    //0.005 sec to skip the LED turn off by OS disconnect req.
                        LedCtrlSetBlinkMode(pAd, LED_WPS_ERROR);
                        // Turn off the WPS LED after 15 seconds.
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_WPS_FAIL_LED_PATTERN_TIMEOUT);
                    }
        
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_WPS_STATE_MACHINE_TERMINATION\n"));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscGetprofileComplete(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if(pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED)
                {
                    pPort->StaCfg.WscControl.bWPSLEDSetTimer = FALSE;
                    LedCtrlSetBlinkMode(pAd, LED_WPS_SUCCESS);
                    pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = TRUE;
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_SKIP_TURN_LED_OFF_AFTER_GETPROFILE);
                }   
                DBGPRINT(RT_DEBUG_TRACE, ("SET::MTK_OID_N5_SET_WSC_GETPROFILE_COMPLETE.\n"));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWscVersion2(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

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
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_TRACE, ("MTK_OID_N6_SET_WSC_VERSION2 InformationBufferLength:%d\n", InformationBufferLength));
                }
                else
                {
                    PULONG  tempVersion2;
                    tempVersion2 = (PULONG)pInformationBuffer;
                    pAd->StaCfg.WSCVersion2 = (UCHAR)(*tempVersion2);
                    if(pAd->StaCfg.WSCVersion2 < 0x20)
                    {
                        pAd->StaCfg.WSC20Testbed = pAd->StaCfg.WSC20Testbed | (0x00000004);
                    }
        
                    if((pAd->StaCfg.WSCVersion2 == 0x10) ||(pAd->StaCfg.WSCVersion2 == 0x20))
                        pAd->StaCfg.WSCVersion2Tmp = pAd->StaCfg.WSCVersion2;

                    if (pAd->StaCfg.WSCVersion2 >= 20)
                        pPort->P2PCfg.WPSVersionsEnabled = 2;
                    else
                        pPort->P2PCfg.WPSVersionsEnabled = 1;
             
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_WSC_VERSION2 = 0x%x tmp:0x%08x\n", pAd->StaCfg.WSCVersion2, pAd->StaCfg.WSCVersion2Tmp));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetUseSelEnroMac(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != sizeof(INT))
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                else
                {
                    PINT tempINT;
                    tempINT = (PINT)pInformationBuffer;
                    pPort->StaCfg.WscControl.UseUserSelectEnrollee = (UCHAR)*tempINT;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::MTK_OID_N6_SET_USE_SEL_ENRO_MAC(%d)\n", pPort->StaCfg.WscControl.UseUserSelectEnrollee));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

NDIS_STATUS
MtkOidSetWsc20Testbed(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                = pPort->PortType;
    NDIS_OID         Oid                     = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    PVOID            pInformationBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PULONG           BytesWritten            = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

    ULONG       i                   = 0;
    BOOLEAN     RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus          = NDIS_STATUS_SUCCESS;

    *BytesWritten = 0;
    *BytesNeeded  = 0;
    FUNC_ENTER;

    if( (pInformationBuffer == NULL) && (InformationBufferLength != 0))
    {
        DBGPRINT_ERR(("ERROR !! pInformationBuffer == NULL, OID = 0x%x\n",Oid));
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) || 
        (!(PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT) && (Oid == MTK_OID_N6_QUERY_HARDWARE_REGISTER || Oid == MTK_OID_N6_SET_HARDWARE_REGISTER || Oid == MTK_OID_N6_QUERY_SUPPRESS_SCAN)))
    {
        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
        {
            pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

            ndisStatus = NDIS_STATUS_INVALID_OID;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        }
        else
        {
            do
            {
                // EXPAND_FUNC NONE
                if (InformationBufferLength != (sizeof(ULONG)))
                {
                    ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_WSC20_TESTBED Failed since Invalid Length!!\n"));
                }
                else 
                {
                    pAd->StaCfg.WSC20Testbed = *((PULONG) pInformationBuffer);
                    if(IS_ENABLE_WSC20TB_Version57(pAd))
                    {
                        pAd->StaCfg.WSCVersion2 = 0x57;
                    }
                    else
                    {
                        pAd->StaCfg.WSCVersion2 = pAd->StaCfg.WSCVersion2Tmp;
                    }
                    
                    if (pAd->StaCfg.WSCVersion2 >= 20)
                        pPort->P2PCfg.WPSVersionsEnabled = 2;
                    else
                        pPort->P2PCfg.WPSVersionsEnabled = 1;
        
                    DBGPRINT(RT_DEBUG_TRACE, ("Set:: MTK_OID_N6_SET_WSC20_TESTBED = 0x%08x tmp:0x%08x\n", pAd->StaCfg.WSC20Testbed, pAd->StaCfg.WSCVersion2Tmp));
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
        *BytesWritten = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                RadarDetectionStop(pAd);
            }

            if ((pPort->CommonCfg.bIEEE80211H == 1) && 
                RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_SWITCHING_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Channel change to ch-%d, will do Channel-Switch-Announcement\n", __FUNCTION__, __LINE__, pPort->Channel));
            }
            else
            {
                pPort->CommonCfg.RadarDetect.RDMode  = RD_NORMAL_MODE;
                pPort->CommonCfg.RadarDetect.CSCount = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:LINE_%d:: Parameters are changed, restart SoftAP @ port %d\n", __FUNCTION__, __LINE__, pPort->PortNumber));

                APStop(pAd,pPort);
                APStartUp(pAd,pPort);
            }
        }
    } 

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}
