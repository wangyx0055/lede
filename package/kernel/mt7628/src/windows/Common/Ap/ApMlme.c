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
    mlme.c

    Abstract:
    Major MLME state machiones here

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "MtConfig.h"
#include <stdarg.h>

/*
    ==========================================================================
    Description:
        This routine is executed every second -
        1. Decide the overall channel quality
        2. Check if need to upgrade the TX rate to any client
        3. perform MAC table maintenance, including ageout no-traffic clients, 
           and release packet buffer in PSQ is fail to TX in time.
    ==========================================================================
 */
VOID ApMlmePeriodicExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    USHORT  OldOperaionMode;
    ULONG   TxTotalCnt;

    NICUpdateRawCounters(pAd);
    
    // The time period for checking antenna is according to traffic
    if (pPort->CommonCfg.bPPAD == FALSE)
    {
        if (pAd->Mlme.bEnableAutoAntennaCheck)
        {
            TxTotalCnt = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + 
                             pAd->Counter.MTKCounters.OneSecTxRetryOkCount + 
                             pAd->Counter.MTKCounters.OneSecTxFailCount;

            if (TxTotalCnt > 50)
            {
                if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
                {
                    AsicEvaluateRxAnt(pAd,pPort);
                }
            }
            else
            {
                if (pAd->Mlme.OneSecPeriodicRound % 3 == 0)
                {
                    AsicEvaluateRxAnt(pAd,pPort);
                }
            }
        }
    }

#if 0   
    if ( (pAd->OpMode == OPMODE_AP) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        ORIBATimerTimeout(pAd,pPort);
    else
    {
        for(uPortNum = 0; uPortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; uPortNum++)
        {
            if ((pAd->PortList[uPortNum] == NULL) || (pAd->PortList[uPortNum]->bActive == FALSE))
            {
                continue;
            }

            if(pAd->PortList[uPortNum]->PortType == EXTAP_PORT)
            {
                ORIBATimerTimeout(pAd,pAd->PortList[uPortNum]);
                break;
            }
        }
    }
#else
    ORIBATimerTimeout(pAd,pPort);
#endif

    // MAC table maintenance
    MacTableMaintenance(pAd,pPort);
    APUpdateCapabilityAndErpIe(pAd);    
    if(pPort->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        GOUpdateBeaconFrame(pAd, pPort);
    }
    else if((pPort->SoftAP.ApCfg.TimBitmap !=0) || ( pPort->SoftAP.ApCfg.TimBitmap2 != 0) || pPort->SoftAP.ApCfg.bTIMIEChange || pPort->SoftAP.ApCfg.bErpIEChange)
    {   
        pPort->SoftAP.ApCfg.bTIMIEChange = FALSE;        
        pPort->SoftAP.ApCfg.bErpIEChange = FALSE;
        ApConnectUpdateBeaconFrame(pAd,pPort);             
     }
    if( pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED )
    {
        OldOperaionMode = pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode;
        APUpdateOperationMode(pAd);
        if ((OldOperaionMode != pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode) || (pPort->MacTab.fAnyBASession))
        {
            if(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
                MtAsicUpdateProtect(pAd, pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, (ALLN_SETPROTECT), TRUE, pPort->MacTab.fAnyStationNonGF);
        }           
    }
    //
    // Indicate link speed.
    // When the NIC runs in the AP mode, the miniport driver indicates
    // the highest speed of the current PHY setting.
    //
    if ((pAd->Mlme.OneSecPeriodicRound % 2) == 0)
    {
        PlatformIndicateLinkQuality(pAd,pPort->PortNumber);
        PlatformIndicateNewLinkSpeed(pAd, pPort->PortNumber, OPMODE_AP);    
    }

    {
        // When AP beacon is enabled and RTS/CTS is enabled, there is a chance that hardware MAC FSM will run into a deadlock
        // and sending CTS-to-self over and over.
        // Software Patch Solution: 
        // 1. Polling debug state register 0x10F4 every one second.
        // 2. If in 0x10F4 the ((bit29==1) && (bit7==1)) OR ((bit29==1) && (bit5==1)), it means the deadlock has occurred.
        // 3. If the deadlock occurred, reset MAC/BBP by setting 0x1004 to 0x0003 for a while then setting it back to 0x000C again.

        ULONG   MacReg;
        
        RTUSBReadMACRegister(pAd, 0x10F4, &MacReg);
        if (((MacReg & 0x20000000) && (MacReg & 0x80)) || ((MacReg & 0x20000000) && (MacReg & 0x20)))
        {
            RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0x1);
            Delay_us(1);
            RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0xC);

            DBGPRINT(RT_DEBUG_WARN,("Warning, MAC specific condition occurs \n"));
        }
    }   
}

VOID ApMlmeSelectTxRateTable(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PUCHAR               *ppTable,
    IN PUCHAR               pTableSize,
    IN PUCHAR               pInitTxRateIdx)
{
    // decide the rate table for tuning
    if (pPort->CommonCfg.TxRateTableSize > 0)
    {
        *ppTable = RateSwitchTable;
        *pTableSize = RateSwitchTable[0];
        *pInitTxRateIdx = RateSwitchTable[1];
    }
    else if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        *ppTable = RateSwitchTable11B;
        *pTableSize = RateSwitchTable11B[0];
        *pInitTxRateIdx = RateSwitchTable11B[1];
        DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: B only AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
    }
    else
    {
        if((pPort->CommonCfg.bProhibitTKIPonHT) && 
            ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||
             (pPort->PortCfg.CipherAlg == CIPHER_TKIP)||
             (pPort->PortCfg.CipherAlg == CIPHER_TKIP_NO_MIC)||
             (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)))
        {
            if (pEntry->RateLen == 4)
            {
                *ppTable = RateSwitchTable11B;
                *pTableSize = RateSwitchTable11B[0];
                *pInitTxRateIdx = RateSwitchTable11B[1];
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B only AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }
            else if (pEntry->RateLen > 8)
            {
                *ppTable = RateSwitchTable11G;
                *pTableSize = RateSwitchTable11G[0];
                *pInitTxRateIdx = RateSwitchTable11G[1];
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: A/G AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }
            else        
            {
                *ppTable = RateSwitchTable11BG;
                *pTableSize = RateSwitchTable11BG[0];
                *pInitTxRateIdx = RateSwitchTable11BG[1];
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B/G mixed AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }   
        }
        else if ((SUPPORT_AGS(pAd)) 
                && (pAd->HwCfg.Antenna.field.TxPath >= 2)             // we support 2x2
                && (pPort->SoftAP.ApCfg.bVhtEnable == TRUE)              // we support VHT
                && (pEntry->VhtPeerStaCtrl.bVhtCapable == TRUE) // the connected client support VHT
                // the connected client support VHT 2x2
                && (pEntry->VhtPeerStaCtrl.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor2ss != NOT_SUPPORTED))
        {
            *ppTable = Ags2x2VhtRateTable;
            *pTableSize = Ags2x2VhtRateTable[0];
            *pInitTxRateIdx = Ags2x2VhtRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 2x2 STA, Ags2x2VhtRateTable\n", 
                __FUNCTION__));
        }
        else if ((SUPPORT_AGS(pAd))
                 && (pAd->HwCfg.Antenna.field.TxPath >= 1)
                 && (pPort->SoftAP.ApCfg.bVhtEnable == TRUE)
                 && (pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss == MCS_0_9)
                 && (pEntry->VhtPeerStaCtrl.bVhtCapable == TRUE)
                 && (pEntry->VhtPeerStaCtrl.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss == MCS_0_9))
        {
            *ppTable = Ags1x1Vht256QAMRateTable;
            *pTableSize = Ags1x1Vht256QAMRateTable[0];
            *pInitTxRateIdx = Ags1x1Vht256QAMRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 1x1 STA, Ags1x1Vht256QAMRateTable\n", 
                __FUNCTION__));
        }
        else if ((SUPPORT_AGS(pAd))
                //&& (pAd->HwCfg.Antenna.field.TxPath >= 2)             // we support 2x2
                && (pPort->SoftAP.ApCfg.bVhtEnable == TRUE)              // we support VHT
                && (pEntry->VhtPeerStaCtrl.bVhtCapable == TRUE))// the connected client support VHT
                // the connected client support VHT 1x1
        {
            *ppTable = Ags1x1VhtRateTable;
            *pTableSize = Ags1x1VhtRateTable[0];
            *pInitTxRateIdx = Ags1x1VhtRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 1x1 STA, Ags1x1VhtRateTable\n", 
                __FUNCTION__));
        }
        else if (SUPPORT_AGS(pAd) && 
                    (pEntry->HTCapability.MCSSet[0] == 0xFF) && 
                    (pEntry->HTCapability.MCSSet[1] == 0xFF) && 
                    (pEntry->HTCapability.MCSSet[2] == 0xFF) && 
                    (pAd->HwCfg.Antenna.field.TxPath == 3))
        {// 11N 3S 
            *ppTable = AGS3x3HTRateTable;
            *pTableSize = AGS3x3HTRateTable[0];
            *pInitTxRateIdx = AGS3x3HTRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 3S STA, AGS3x3HTRateTable\n", 
                __FUNCTION__));
        }       
        else if ((pEntry->RateLen == 12) && (pEntry->HTCapability.MCSSet[0] == 0xff) && ((pEntry->HTCapability.MCSSet[1] == 0) ||
            ((pAd->HwCfg.Antenna.field.TxPath == 1) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream == 1)))
        {// 11BGN 1S AP
            if (SUPPORT_AGS(pAd))
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = AGS1x1HTRateTable;
                    *pTableSize = AGS1x1HTRateTable[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S STA, AGS1x1HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    *ppTable = AGS1x1HTRateTable5G;
                    *pTableSize = AGS1x1HTRateTable5G[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable5G[1];
                
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S STA, AGS1x1HTRateTable5G\n", 
                        __FUNCTION__));             
                }
            }
            else
            {
                *ppTable = RateSwitchTable11BGN1S;
                *pTableSize = RateSwitchTable11BGN1S[0];
                *pInitTxRateIdx = RateSwitchTable11BGN1S[1];
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11BGN 1S STA \n"));
            }
        }
        else if ((pEntry->RateLen == 12) && (pEntry->HTCapability.MCSSet[0] == 0xff) && (pEntry->HTCapability.MCSSet[1] == 0xff) &&
            (((pAd->HwCfg.Antenna.field.TxPath >= 2) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream >= 2)))
        {// 11BGN 2S AP
            if (SUPPORT_AGS(pAd))
            {
                *ppTable = AGS2x2HTRateTable;
                *pTableSize = AGS2x2HTRateTable[0];
                *pInitTxRateIdx = AGS2x2HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 2S STA, AGS2x2HTRateTable\n", 
                    __FUNCTION__));
            }
            else
            {
                if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
                {
                    *ppTable = RateSwitchTable11BGN2S;
                    *pTableSize = RateSwitchTable11BGN2S[0];
                    *pInitTxRateIdx = RateSwitchTable11BGN2S[1];
                }
                else
                {
                    *ppTable = RateSwitchTable11BGN2SForABand;
                    *pTableSize = RateSwitchTable11BGN2SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11BGN2SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11BGN 2S STA \n"));
            }
        }
        else if ((pEntry->HTCapability.MCSSet[0] == 0xff) && ((pEntry->HTCapability.MCSSet[1] == 0) ||
            ((pAd->HwCfg.Antenna.field.TxPath == 1) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream == 1)))
        {// 11N 1S AP
            if (SUPPORT_AGS(pAd))
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = AGS1x1HTRateTable;
                    *pTableSize = AGS1x1HTRateTable[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S STA, AGS1x1HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    *ppTable = AGS1x1HTRateTable5G;
                    *pTableSize = AGS1x1HTRateTable5G[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable5G[1];
                
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S STA, AGS1x1HTRateTable5G\n", 
                        __FUNCTION__));
                }
            }
            else
            {
                *ppTable = RateSwitchTable11N1S;
                *pTableSize = RateSwitchTable11N1S[0];
                *pInitTxRateIdx = RateSwitchTable11N1S[1];
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 1S STA \n"));
            }
        }
        else if ((pEntry->HTCapability.MCSSet[0] == 0xff) && (pEntry->HTCapability.MCSSet[1] == 0xff) &&
            (((pAd->HwCfg.Antenna.field.TxPath >= 2) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream >= 2)))
        {// 11N 2S AP
            if (SUPPORT_AGS(pAd))
            {
                *ppTable = AGS2x2HTRateTable;
                *pTableSize = AGS2x2HTRateTable[0];
                *pInitTxRateIdx = AGS2x2HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 2S STA, AGS2x2HTRateTable\n", 
                    __FUNCTION__));
            }
            else
            {       
                if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
                {
                    *ppTable = RateSwitchTable11N2S;
                    *pTableSize = RateSwitchTable11N2S[0];
                    *pInitTxRateIdx = RateSwitchTable11N2S[1];
                }
                else
                {
                    *ppTable = RateSwitchTable11N2SForABand;
                    *pTableSize = RateSwitchTable11N2SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 2S STA \n"));
            }
        }
        else if ((pEntry->RateLen == 4) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
        {// B only AP
            *ppTable = RateSwitchTable11B;
            *pTableSize = RateSwitchTable11B[0];
            *pInitTxRateIdx = RateSwitchTable11B[1];
            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B only STA \n"));
        }
        else if ((pEntry->RateLen > 8) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
        {// B/G  mixed AP
            *ppTable = RateSwitchTable11BG;
            *pTableSize = RateSwitchTable11BG[0];
            *pInitTxRateIdx = RateSwitchTable11BG[1];
            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B/G mixed STA \n"));
        }
        else if ((pEntry->RateLen == 8) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
        {// G only AP
            *ppTable = RateSwitchTable11G;
            *pTableSize = RateSwitchTable11G[0];
            *pInitTxRateIdx = RateSwitchTable11G[1];
            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: A/G STA \n"));
        }
        else if ((pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0) && (pEntry->HTCapability.MCSSet[2] == 0))
        {
            *ppTable = RateSwitchTable11BGAll;
            *pTableSize = RateSwitchTable11BGAll[0];
            *pInitTxRateIdx = RateSwitchTable11BGAll[1];
            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: Not 11n STA \n"));
        }       
        else
        {
            if (SUPPORT_AGS(pAd) && (pAd->HwCfg.Antenna.field.TxPath == 3))
            {
                *ppTable = AGS3x3HTRateTable;
                *pTableSize = AGS3x3HTRateTable[0];
                *pInitTxRateIdx = AGS3x3HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: Unknown STA, AGS3x3HTRateTable\n", 
                    __FUNCTION__));
            }
            else
            {
                if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
                {
                    if (pAd->HwCfg.Antenna.field.TxPath == 1)
                    {
                        *ppTable = RateSwitchTable11N1S;
                        *pTableSize = RateSwitchTable11N1S[0];
                        *pInitTxRateIdx = RateSwitchTable11N1S[1];
                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                    }
                    else
                    {
                        *ppTable = RateSwitchTable11N2S;
                        *pTableSize = RateSwitchTable11N2S[0];
                        *pInitTxRateIdx = RateSwitchTable11N2S[1];
                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));
                    }
                }
                else
                {
                    if (pAd->HwCfg.Antenna.field.TxPath == 1)
                    {
                        *ppTable = RateSwitchTable11N1S;
                        *pTableSize = RateSwitchTable11N1S[0];
                        *pInitTxRateIdx = RateSwitchTable11N1S[1];
                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                    }
                    else
                    {
                        *ppTable = RateSwitchTable11N2SForABand;
                        *pTableSize = RateSwitchTable11N2SForABand[0];
                        *pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));
                    }
                }
                
                DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode (RateLen=%d, MCSSet[0]=0x%x, MCSSet[1]=0x%x, TxStream=%d, RxStream=%d)\n",
                    pEntry->RateLen, pEntry->HTCapability.MCSSet[0], pEntry->HTCapability.MCSSet[1],
                    pAd->HwCfg.NicConfig3.field.TxStream, pAd->HwCfg.NicConfig3.field.RxStream));           
            }

        }
    }
}

VOID ApMlmeSetTxRate(
    IN PMP_ADAPTER        pAd,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PRTMP_TX_RATE_SWITCH pTxRate,
    IN PUCHAR               pTable)
{
    if ((pTxRate->STBC == STBC_USE) && 
         (READ_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg) == STBC_USE) && 
         (pAd->HwCfg.Antenna.field.TxPath >= 2))
    {
        WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
    }
    else
    {
        WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
    }

    if ((pTxRate->ShortGI) && (READ_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg)))
    {
        WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_400);
    }
    else
    {
        WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
    }

    if (pTxRate->CurrMCS < MCS_AUTO)
    {
        WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pTxRate->CurrMCS);
    }

    if (pTxRate->Mode <= MODE_VHT)
    {
        WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, pTxRate->Mode);
    }

    if (pTxRate->Nss <= NSS_1)
    {
        WRITE_PHY_CFG_NSS(pAd, &pEntry->TxPhyCfg, pTxRate->Nss);
    }

    // VHT Rate table has the Rate tuning algorithm that change the Bandwidth.
    if( VHT_NIC(pAd) && CURRENT_RATETABLE_IS_VHT_TABLE(pTable) )
        WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pTxRate->BW);

    DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: ApMlmeSetTxRate - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, Nss = %d\n", 
        pEntry->CurrTxRateIndex, 
        READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_NSS(pAd, &pEntry->TxPhyCfg)));
}

/*
    ==========================================================================
    Description:
        This routine walks through the MAC table, see if TX rate change is 
        required for each associated client. 
    Output:
        pEntry->CurrTxRate - 
    NOTE:
        call this routine every second
    ==========================================================================
 */
VOID ApMlmeDynamicTxRateSwitching(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN TX_STA_CNT0_STRUC TxStaCnt0,
    IN TX_STA_CNT1_STRUC StaTx1)
{
    PUCHAR                  pTable;
    UCHAR                   TableSize = 0;
    UCHAR                   UpRateIdx, DownRateIdx, CurrRateIdx;
    CHAR                    TmpIdx;
    ULONG                   AccuTxTotalCnt, TxTotalCnt;
    ULONG                   TxErrorRatio = 0;
    MAC_TABLE_ENTRY         *pEntry;
    PRTMP_TX_RATE_SWITCH    pCurrTxRate, pNextTxRate = NULL, pTmpTxRate;
    BOOLEAN                 bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    UCHAR                   InitTxRateIdx, TrainUp, TrainDown;
    CHAR                    Rssi, RssiOffset = 0;
    ULONG                   TxRetransmit, TxSuccess, TxFailCount;
    ULONG                   MacTabSize;
    AGS_STATISTICS_INFO AGSStatisticsInfo = {0};
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;    
    PQUEUE_HEADER pHeader;

    TxRetransmit = StaTx1.field.TxRetransmit;
    TxSuccess = StaTx1.field.TxSuccess;
    TxFailCount = TxStaCnt0.field.TxFailCount;
    TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;
    
    pAd->Counter.MTKCounters.OneSecTxRetryOkCount += StaTx1.field.TxRetransmit;
    pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount += StaTx1.field.TxSuccess;
    pAd->Counter.MTKCounters.OneSecTxFailCount += TxStaCnt0.field.TxFailCount;
    pAd->Counter.WlanCounters.TransmittedFragmentCount.LowPart += StaTx1.field.TxSuccess;
    pAd->Counter.WlanCounters.RetryCount.LowPart += StaTx1.field.TxRetransmit;
    pAd->Counter.WlanCounters.FailedCount.LowPart += TxStaCnt0.field.TxFailCount;

    if (TxStaCnt0.field.TxFailCount != 0)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE,("APdyn  QuickDRS: send BAR \n"));
        pPort->CommonCfg.IOTestParm.bSendBAR = TRUE;
        ORIBATimerTimeout(pAd,pPort);
    }

    AccuTxTotalCnt = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + 
                 pAd->Counter.MTKCounters.OneSecTxRetryOkCount + 
                 pAd->Counter.MTKCounters.OneSecTxFailCount;
    
    if (TxTotalCnt)
    {
        TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
        // Patch hardware algorithm issue:
        // Soft AP can't stop retransmitting packet when receiving abnormal BA from Intel 4965 STA
        // We force to set LongRtyLimit and ShortRtyLimit to 0 to stop retransmitting packet, after a while, resoring original settings
        // (Abnormal BA means that Starting Sequence Control and BlockAck Bitmap are 0)
        if (TxErrorRatio == 100)
        {
            TX_RTY_CFG_STRUC    TxRtyCfg;
                        
            RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
            TxRtyCfg.field.LongRtyLimit = 11;
            TxRtyCfg.field.ShortRtyLimit = 0x7;
            RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

            Delay_us(300);

            // Clear the flag for later on (500ms) here to restore the TxRty Count to 1f
            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_LONG_RETRY);

        }
        else if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_LONG_RETRY))
        {
            TX_RTY_CFG_STRUC    TxRtyCfg;

            DBGPRINT(RT_DEBUG_TRACE,("%s: ReStore RxTry = 0x1f",__FUNCTION__));
            RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
            TxRtyCfg.field.LongRtyLimit = 0x1f;
            TxRtyCfg.field.ShortRtyLimit = 0x1f;
            RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

            OPSTATUS_SET_FLAG(pAd, fOP_STATUS_LONG_RETRY);
        }
        
    }

    if (pAd->HwCfg.Antenna.field.RxPath == 3)
    {
        Rssi = ((pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[1] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[2]) / 3);
    }
    else if (pAd->HwCfg.Antenna.field.RxPath == 2)
    {
        Rssi = (pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[1]) >> 1;
    }
    else
    {
        Rssi = pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0];
    }
    //
    // Gather the statistics information
    //
    AGSStatisticsInfo.RSSI = Rssi;
    AGSStatisticsInfo.TxErrorRatio = TxErrorRatio;
    AGSStatisticsInfo.AccuTxTotalCnt = AccuTxTotalCnt;
    AGSStatisticsInfo.TxTotalCnt = TxTotalCnt;
    AGSStatisticsInfo.TxSuccess = TxSuccess;
    AGSStatisticsInfo.TxRetransmit = TxRetransmit;
    AGSStatisticsInfo.TxFailCount = TxFailCount;

    //
    // walk through MAC table, see if need to change AP's TX rate toward each entry
    //
    MacTabSize = MAX_LEN_OF_MAC_TABLE;
    
    pHeader = &pPort->MacTab.MacTabList;
    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextMacEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
         
        if(pEntry == NULL)
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue;  
        }
        
        if((pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
        {
            if ((pPort->PortType == EXTSTA_PORT) && (pPort->SoftAP.bAPStart == FALSE))
            {
                if((pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
                {
                    pNextMacEntry = pNextMacEntry->Next;   
                    pEntry = NULL;
                    continue;  
                }
            }
            else
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pEntry = NULL;
                continue;  
            }
        }

        // only associated STA counts
        if ((pEntry->ValidAsCLI == FALSE) || (pEntry->Sst != SST_ASSOC))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue;  
        }

        // excluding the entry in p2p client operation
        if ((pEntry->WlanIdxRole == ROLE_WLANIDX_P2P_CLIENT) && (pEntry->ValidAsP2P == TRUE) && (pEntry->P2pInfo.P2pClientState == P2PSTATE_GO_OPERATING))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue;  
        }
        
        CurrRateIdx = pEntry->CurrTxRateIndex;  
        //
        // Initialize
        //
        UpRateIdx = CurrRateIdx;
        DownRateIdx = CurrRateIdx;
        
        ApMlmeSelectTxRateTable(pAd,pPort, pEntry, &pTable, &TableSize, &InitTxRateIdx);
        
        if (SUPPORT_AGS(pAd))
        {
            if (IS_AGS_RATE_TABLE(pTable))
            {
                //
                // The dynamic Tx rate switching for AGS(Adaptive Group Switching) in RT6x9x
                //
                ApMlmeDynamicTxRateSwitchingAGSv2(pAd, pPort, pEntry, pTable, TableSize, &AGSStatisticsInfo, InitTxRateIdx);

                pNextMacEntry = pNextMacEntry->Next;   
                pEntry = NULL;
                continue;  // Skip the remaining procedure of the old Tx rate switching
            }
        }
        
        // When switch from Fixed rate -> auto rate, the REAL TX rate might be different from pPort->CommonCfg.TxRateIndex.
        // So need to sync here.
        pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(CurrRateIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];

        
        if ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg)!= pCurrTxRate->CurrMCS) 
            && (pPort->CommonCfg.bAutoTxRateSwitch == TRUE))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: ##Fixed rate -> auto rate## CurrRateIdx = %d, InitTxRateIdx = %d.  \n", CurrRateIdx, InitTxRateIdx));
            // Need to sync Real Tx rate and our record. 
            // Then return for next DRS.
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(InitTxRateIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
            pEntry->CurrTxRateIndex = InitTxRateIdx;
            MlmeSetTxRate(pAd, pEntry, pCurrTxRate, pTable);
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }       
        
        // decide the next upgrade rate and downgrade rate, if any
        // For legacy connection, skip unsupported rates
        if (pCurrTxRate->Mode == MODE_CCK)
        {   
            TmpIdx = CurrRateIdx + 1;
            while(TmpIdx < TableSize)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupCCKMCS[pTmpTxRate->CurrMCS] == TRUE)
                {
                    UpRateIdx = TmpIdx;
                    break;
                }
                TmpIdx++;
            }

            TmpIdx = CurrRateIdx - 1;
            while(TmpIdx >= 0)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupCCKMCS[pTmpTxRate->CurrMCS] == TRUE)
                {
                    DownRateIdx = TmpIdx;
                    break;
                }
                TmpIdx--;
            }   
        }
        else if (pCurrTxRate->Mode == MODE_OFDM)
        {   
            TmpIdx = CurrRateIdx + 1;
            while(TmpIdx < TableSize)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupOFDMMCS[pTmpTxRate->CurrMCS] == TRUE)
                {   
                    UpRateIdx = TmpIdx;
                    break;
                }
                TmpIdx++;
            }

            TmpIdx = CurrRateIdx - 1;
            while(TmpIdx >= 0)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupOFDMMCS[pTmpTxRate->CurrMCS] == TRUE)
                {
                    DownRateIdx = TmpIdx;
                    break;
                }
                TmpIdx--;
            }   
        }
        else
        {
            // for 11n rates, select adjacent rate index as up and down rate index
            if ((CurrRateIdx > 0) && (CurrRateIdx < (TableSize - 1)))
            {
                UpRateIdx = CurrRateIdx + 1;
                DownRateIdx = CurrRateIdx -1;
            }
            else if (CurrRateIdx == 0)
            {
                UpRateIdx = CurrRateIdx + 1;
                DownRateIdx = CurrRateIdx;
            }
            else if (CurrRateIdx == (TableSize - 1))
            {
                UpRateIdx = CurrRateIdx;
                DownRateIdx = CurrRateIdx - 1;
            }
        }

        if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
        {
            TrainUp     = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
            TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
        }
        else
        {
            TrainUp     = pCurrTxRate->TrainUp;
            TrainDown   = pCurrTxRate->TrainDown;
        }

        pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;

        DBGPRINT_RAW(RT_DEBUG_INFO,("DRS:Aid=%d, TxRetransmit=%d, TxFailCount=%d, TxSuccess=%d \n",
            pEntry->Aid, TxRetransmit, TxFailCount, TxSuccess));

        if (pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: Fixed - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, PER=%d%% \n\n",
                                        pEntry->CurrTxRateIndex, 
                                        READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg),
                                        READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg),
                                        READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg),
                                        READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg),
                                        READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg),
                                        TxErrorRatio));
            return;
        }
        else
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: Before- CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, TrainUp=%d, TrainDown=%d, NextUp=%d, NextDown=%d, CurrMCS=%d, PER=%d%%\n",
                CurrRateIdx,
                pCurrTxRate->CurrMCS,
                pCurrTxRate->STBC,
                pCurrTxRate->ShortGI,
                pCurrTxRate->Mode,
                TrainUp,
                TrainDown,
                UpRateIdx,
                DownRateIdx,
                READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
                TxErrorRatio));
        }

        if (! OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        if (AccuTxTotalCnt <= 15)
        {
            CHAR    idx = 0;
            UCHAR   TxRateIdx;
            UCHAR   MCS0 = 0, MCS1 = 0, MCS2 = 0, MCS3 = 0, MCS4 = 0, MCS5 = 0, MCS6 = 0, MCS7 = 0, MCS12 = 0, MCS13 = 0, MCS14 = 0, MCS15 = 0;

            // check the existence and index of each needed MCS
            while (idx < pTable[0])
            {
                pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(idx+1)*SIZE_OF_RATE_TABLE_ENTRY];

                if (pCurrTxRate->CurrMCS == MCS_0)
                {
                    MCS0 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_1)
                {
                    MCS1 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_2)
                {
                    MCS2 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_3)
                {
                    MCS3 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_4)
                {
                    MCS4 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_5)
                {
                    MCS5 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_6)
                {
                    MCS6 = idx;
                }
                else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))   // prevent the highest MCS using short GI when 1T and low throughput
                {
                    MCS7 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_12)
                {
                    MCS12 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_13)
                {
                    MCS13 = idx;
                }
                else if (pCurrTxRate->CurrMCS == MCS_14)
                {
                    MCS14 = idx;
                }
                else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))
                {
                    MCS15 = idx;
                }
                
                idx ++;
            }

            if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
            {
                if (pAd->HwCfg.NicConfig2.field.ExternalLNAForG)
                {
                    RssiOffset = 2;
                }
                else
                {
                    RssiOffset = 5;
                }
            }
            else
            {
                if (pAd->HwCfg.NicConfig2.field.ExternalLNAForA)
                {
                    RssiOffset = 5;
                }
                else
                {
                    RssiOffset = 8;
                }
            }

            if ((pTable == RateSwitchTable11BGN2S) || (pTable == RateSwitchTable11BGN2SForABand) || (pTable == RateSwitchTable11N2S) || (pTable == RateSwitchTable11N2SForABand) || (pTable == RateSwitchTable))
            {// N mode with 2 stream
                if (MCS15 && (Rssi >= (-70+RssiOffset)))
                    TxRateIdx = MCS15;
                else if (MCS14 && (Rssi >= (-72+RssiOffset)))
                    TxRateIdx = MCS14;
                else if (MCS13 && (Rssi >= (-76+RssiOffset)))
                    TxRateIdx = MCS13;
                else if (MCS12 && (Rssi >= (-78+RssiOffset)))
                    TxRateIdx = MCS12;
                else if (MCS4 && (Rssi >= (-82+RssiOffset)))
                    TxRateIdx = MCS4;
                else if (MCS3 && (Rssi >= (-84+RssiOffset)))
                    TxRateIdx = MCS3;
                else if (MCS2 && (Rssi >= (-86+RssiOffset)))
                    TxRateIdx = MCS2;
                else if (MCS1 && (Rssi >= (-88+RssiOffset)))
                    TxRateIdx = MCS1;
                else
                    TxRateIdx = MCS0;
            }
            else if ((pTable == RateSwitchTable11BGN1S) || (pTable == RateSwitchTable11N1S))
            {// N mode with 1 stream
                if (MCS7 && (Rssi > (-72+RssiOffset)))
                    TxRateIdx = MCS7;
                else if (MCS6 && (Rssi > (-74+RssiOffset)))
                    TxRateIdx = MCS6;
                else if (MCS5 && (Rssi > (-77+RssiOffset)))
                    TxRateIdx = MCS5;
                else if (MCS4 && (Rssi > (-79+RssiOffset)))
                    TxRateIdx = MCS4;
                else if (MCS3 && (Rssi > (-81+RssiOffset)))
                    TxRateIdx = MCS3;
                else if (MCS2 && (Rssi > (-83+RssiOffset)))
                    TxRateIdx = MCS2;
                else if (MCS1 && (Rssi > (-86+RssiOffset)))
                    TxRateIdx = MCS1;
                else
                    TxRateIdx = MCS0;
            }
            else
            {// Legacy mode
                if (MCS7 && (Rssi > -70) && (pEntry->SupOFDMMCS[MCS_7]))
                    TxRateIdx = MCS7;
                else if (MCS6 && (Rssi > -74) && (pEntry->SupOFDMMCS[MCS_6]))
                    TxRateIdx = MCS6;
                else if (MCS5 && (Rssi > -78) && (pEntry->SupOFDMMCS[MCS_5]))
                    TxRateIdx = MCS5;
                else if (MCS4 && (Rssi > -82) && (pEntry->SupOFDMMCS[MCS_4]))
                    TxRateIdx = MCS4;
                else if ((MCS4 == 0) && (pTable == RateSwitchTable11B)) // for B-only mode
                {
                    if ( pEntry->SupCCKMCS[MCS_3])
                                           TxRateIdx = MCS3;
                      else if (pEntry->SupCCKMCS[MCS_2])
                                            TxRateIdx = MCS2;
                      else if (pEntry->SupCCKMCS[MCS_1])
                                            TxRateIdx = MCS1;
                      else if (pEntry->SupCCKMCS[MCS_0])
                                            TxRateIdx = MCS0;
                                     else
                                             TxRateIdx = MCS3;
                }
                else if (MCS3 && (Rssi > -85) && (pEntry->SupOFDMMCS[MCS_3] || pEntry->SupCCKMCS[MCS_3]))
                    TxRateIdx = MCS3;
                else if (MCS2 && (Rssi > -87) && (pEntry->SupOFDMMCS[MCS_2] || pEntry->SupCCKMCS[MCS_2]))
                    TxRateIdx = MCS2;
                else if (MCS1 && (Rssi > -90) && (pEntry->SupOFDMMCS[MCS_1] || pEntry->SupCCKMCS[MCS_1]))
                    TxRateIdx = MCS1;
                else
                    TxRateIdx = MCS0;
            }

            if (TxRateIdx != pEntry->CurrTxRateIndex)
            {
                pEntry->CurrTxRateIndex = TxRateIdx;
                pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pEntry->CurrTxRateIndex+1)*SIZE_OF_RATE_TABLE_ENTRY];
                ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
            }

            PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
            PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
            pEntry->fLastSecAccordingRSSI = TRUE;

            DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: AccuTxTotalCnt <= 15, switch MCS according to RSSI (%d), RssiOffset=%d\n", Rssi, RssiOffset));
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        if (pEntry->fLastSecAccordingRSSI == TRUE)
        {
            pEntry->fLastSecAccordingRSSI = FALSE;
            pEntry->LastSecTxRateChangeAction = 0;
            DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: MCS is according to RSSI, and ignore tuning this sec \n"));
            return;
        }

        do
        {
            BOOLEAN bTrainUpDown = FALSE;
            
            pEntry->CurrTxRateStableTime ++;

            // downgrade TX quality if PER >= Rate-Down threshold
            if (TxErrorRatio >= TrainDown)
            {
                bTrainUpDown = TRUE;
                pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;
            }
            // upgrade TX quality if PER <= Rate-Up threshold
            else if (TxErrorRatio <= TrainUp)
            {
                bTrainUpDown = TRUE;
                bUpgradeQuality = TRUE;

#pragma prefast(suppress: __WARNING_READ_OVERRUN, "pEntry->TxQuality, should not Buffer overrun")               
                if (pEntry->TxQuality[CurrRateIdx])
                    pEntry->TxQuality[CurrRateIdx] --;  // quality very good in CurrRate

                if (pEntry->TxRateUpPenalty)
                    pEntry->TxRateUpPenalty --;
                else if (pEntry->TxQuality[UpRateIdx])
                    pEntry->TxQuality[UpRateIdx] --;    // may improve next UP rate's quality
            }

#pragma prefast(suppress: __WARNING_WRITE_OVERRUN, "pEntry->PER, should not Buffer overrun")
            pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

            // perform DRS - consider TxRate Down first, then rate up.
            if (bTrainUpDown)
            {
                if ((CurrRateIdx != DownRateIdx) && (pEntry->TxQuality[CurrRateIdx] >= DRS_TX_QUALITY_WORST_BOUND))
                {
                    pEntry->CurrTxRateIndex = DownRateIdx;
                }
                else if ((CurrRateIdx != UpRateIdx) && (pEntry->TxQuality[UpRateIdx] <= 0))
                {
                    pEntry->CurrTxRateIndex = UpRateIdx;
                }
            }
        }while (FALSE);

        // if rate-up happen, clear all bad history of all TX rates
        if (pEntry->CurrTxRateIndex > CurrRateIdx)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: ++TX rate from %d to %d \n", CurrRateIdx, pEntry->CurrTxRateIndex));
            
            pEntry->CurrTxRateStableTime = 0;
            pEntry->TxRateUpPenalty = 0;
            pEntry->LastSecTxRateChangeAction = 1; // rate UP
            PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
            PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);

            //
            // For TxRate fast train up
            // 
            if (!pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning)
            {
                pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
            }
        }
        // if rate-down happen, only clear DownRate's bad history
        else if (pEntry->CurrTxRateIndex < CurrRateIdx)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: --TX rate from %d to %d \n", CurrRateIdx, pEntry->CurrTxRateIndex));
            
            pEntry->CurrTxRateStableTime = 0;
            pEntry->TxRateUpPenalty = 0;           // no penalty
            pEntry->LastSecTxRateChangeAction = 2; // rate DOWN
#pragma prefast(suppress: __WARNING_WRITE_OVERRUN, "pEntry->TxQuality, should not Buffer overrun")          
            pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
#pragma prefast(suppress: __WARNING_WRITE_OVERRUN, "pEntry->PER, should not Buffer overrun")            
            pEntry->PER[pEntry->CurrTxRateIndex] = 0;

            //
            // For TxRate fast train down
            // 
            if (!pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning)
            {
                pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
            }
        }
        else
        {
            pEntry->LastSecTxRateChangeAction = 0; // rate no change
            bTxRateChanged = FALSE;
        }

        pEntry->LastTxOkCount = TxSuccess;
        
        pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pEntry->CurrTxRateIndex+1)*SIZE_OF_RATE_TABLE_ENTRY];
        if (bTxRateChanged && pNextTxRate)
        {
            ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
        }    
        pNextMacEntry = pNextMacEntry->Next;
        pEntry = NULL;
    }
}

/*
    ========================================================================
    Routine Description:
        AP side, Auto TxRate faster train up timer call back function.
    
    Arguments:
        SystemSpecific1         - Not used.
        FunctionContext         - Pointer to our Adapter context.
        SystemSpecific2         - Not used.
        SystemSpecific3         - Not used.
    
    Return Value:
        None
    
    ========================================================================
*/
VOID ApMlmeQuickResponeForRateUpExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    PUCHAR                  pTable;
    UCHAR                   TableSize = 0;
    UCHAR                   UpRateIdx, DownRateIdx, CurrRateIdx;
    CHAR                    TmpIdx;
    ULONG                   AccuTxTotalCnt = 0, TxTotalCnt = 0;
    ULONG                   TxErrorRatio = 0;
    MAC_TABLE_ENTRY         *pEntry;
    PRTMP_TX_RATE_SWITCH    pCurrTxRate, pNextTxRate = NULL, pTmpTxRate;
    BOOLEAN                 bTxRateChanged = TRUE;
    UCHAR                   InitTxRateIdx, TrainUp, TrainDown;
    CHAR                    Rssi;
    ULONG                   TxRetransmit = 0, TxSuccess = 0, TxFailCount = 0;
    ULONG                   StartIdx;
    ULONG                   ratio = 0;
    AGS_STATISTICS_INFO AGSStatisticsInfo = {0};
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;    
    
    pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
    
    if (pAd->HwCfg.Antenna.field.RxPath == 3)
    {
        Rssi = ((pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[1] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[2]) / 3);
    }
    else    if (pAd->HwCfg.Antenna.field.RxPath == 2)
    {
        Rssi = (pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[1]) >> 1;
    }
    else
    {
        Rssi = pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0];
    }

    //
    // Gather the statistics information
    //
    AGSStatisticsInfo.RSSI = Rssi;
    AGSStatisticsInfo.TxErrorRatio = TxErrorRatio;
    AGSStatisticsInfo.AccuTxTotalCnt = AccuTxTotalCnt;
    AGSStatisticsInfo.TxTotalCnt = TxTotalCnt;
    AGSStatisticsInfo.TxSuccess = TxSuccess;
    AGSStatisticsInfo.TxRetransmit = TxRetransmit;
    AGSStatisticsInfo.TxFailCount = TxFailCount;

    if(pPort->PortType == EXTSTA_PORT)
        StartIdx = 1;
    else
        StartIdx = 2;
    //
    // walk through MAC table, see if need to change AP's TX rate toward each entry
    //
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
             if((pPort->PortType == EXTSTA_PORT) && (pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
             {
                pNextMacEntry = pNextMacEntry->Next;   
                pEntry = NULL;
                continue; 
             }
             else
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pEntry = NULL;
                continue; 
            }             
        }

        pEntry = pMacEntry;

        // only associated STA counts
        if ((pEntry->ValidAsCLI == FALSE) || (pEntry->Sst != SST_ASSOC))
         {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
         }

        // excluding the entry in p2p client operation
        if ((pEntry->WlanIdxRole ==ROLE_WLANIDX_P2P_CLIENT) && (pEntry->ValidAsP2P == TRUE) && (pEntry->P2pInfo.P2pClientState == P2PSTATE_GO_OPERATING))
         {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
         }

        CurrRateIdx = pEntry->CurrTxRateIndex;
        //
        // Initialize
        //
        UpRateIdx = CurrRateIdx;
        DownRateIdx = CurrRateIdx;

        // decide the rate table for tuning
        ApMlmeSelectTxRateTable(pAd, pPort,pEntry, &pTable, &TableSize, &InitTxRateIdx);

        if (SUPPORT_AGS(pAd))
        {
            if (IS_AGS_RATE_TABLE(pTable))
            {
                //
                // The dynamic Tx rate switching for AGS(Adaptive Group Switching) in RT6x9x
                //
                ApMlmeQuickResponeForRateUpExecAGSv2(pAd, pEntry, pTable, TableSize, &AGSStatisticsInfo, InitTxRateIdx);

                pNextMacEntry = pNextMacEntry->Next;   
                pEntry = NULL;
                continue;  // Skip the remaining procedure of the old Tx rate switching
            }
        }

        pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(CurrRateIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];

        // decide the next upgrade rate and downgrade rate, if any
        // For legacy connection, skip unsupported rates
        if (pCurrTxRate->Mode == MODE_CCK)
        {   
            TmpIdx = CurrRateIdx + 1;
            while(TmpIdx < TableSize)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupCCKMCS[pTmpTxRate->CurrMCS] == TRUE)
                {
                    UpRateIdx = TmpIdx;
                    break;
                }
                TmpIdx++;
            }

            TmpIdx = CurrRateIdx - 1;
            while(TmpIdx >= 0)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupCCKMCS[pTmpTxRate->CurrMCS] == TRUE)
                {
                    DownRateIdx = TmpIdx;
                    break;
                }
                TmpIdx--;
            }   
        }
        else if (pCurrTxRate->Mode == MODE_OFDM)
        {   
            TmpIdx = CurrRateIdx + 1;
            while(TmpIdx < TableSize)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupOFDMMCS[pTmpTxRate->CurrMCS] == TRUE)
                {
                    UpRateIdx = TmpIdx;
                    break;
                }
                TmpIdx++;
            }

            TmpIdx = CurrRateIdx - 1;
            while(TmpIdx >= 0)
            {
                pTmpTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(TmpIdx+1)*SIZE_OF_RATE_TABLE_ENTRY];
                if(pEntry->SupOFDMMCS[pTmpTxRate->CurrMCS] == TRUE)
                {
                    DownRateIdx = TmpIdx;
                    break;
                }
                TmpIdx--;
            }   
        }
        else
        {
            // for 11n rates, select adjacent rate index as up and down rate index
            if ((CurrRateIdx > 0) && (CurrRateIdx < (TableSize - 1)))
            {
                UpRateIdx = CurrRateIdx + 1;
                DownRateIdx = CurrRateIdx -1;
            }
            else if (CurrRateIdx == 0)
            {
                UpRateIdx = CurrRateIdx + 1;
                DownRateIdx = CurrRateIdx;
            }
            else if (CurrRateIdx == (TableSize - 1))
            {
                UpRateIdx = CurrRateIdx;
                DownRateIdx = CurrRateIdx - 1;
            }
        }

        if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
        {
            TrainUp     = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
            TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
        }
        else
        {
            TrainUp     = pCurrTxRate->TrainUp;
            TrainDown   = pCurrTxRate->TrainDown;
        }

        DBGPRINT_RAW(RT_DEBUG_INFO,("QuickDRS:Aid=%d, TxRetransmit=%d, TxFailCount=%d, TxSuccess=%d \n",
            pEntry->Aid, TxRetransmit, TxFailCount, TxSuccess));

        if (pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: Fixed - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, PER=%d%% \n\n",
                pEntry->CurrTxRateIndex, 
                READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg),
                READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg),
                READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg),
                READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg),
                READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg),
                TxErrorRatio));

            return;
        }
        else
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: Before- CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, TrainUp=%d, TrainDown=%d, NextUp=%d, NextDown=%d, CurrMCS=%d, PER=%d%%\n",
                CurrRateIdx,
                pCurrTxRate->CurrMCS,
                pCurrTxRate->STBC,
                pCurrTxRate->ShortGI,
                pCurrTxRate->Mode,
                TrainUp,
                TrainDown,
                UpRateIdx,
                DownRateIdx,
                READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg),
                TxErrorRatio));
        }

        if (! OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        if (AccuTxTotalCnt <= 15)
        {
            // perform DRS - consider TxRate Down first, then rate up.
            if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;
            }
            else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
            }
            
            pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pEntry->CurrTxRateIndex+1)*SIZE_OF_RATE_TABLE_ENTRY];
            ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
            
            DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: AccuTxTotalCnt <= 15, train back to original rate \n"));
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        do
        {
            // downgrade TX quality if PER >= Rate-Down threshold
            if (TxErrorRatio >= TrainDown)
            {
                pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;
            }

#pragma prefast(suppress: __WARNING_WRITE_OVERRUN, "pEntry->PER, should not Buffer overrun")
            pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

            // perform DRS - consider TxRate Down first, then rate up.
            if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
            {
                if (((pEntry->LastTxOkCount + 2) >= (TxSuccess * ratio)) && TxErrorRatio != 0)
                {
                    pEntry->CurrTxRateIndex = DownRateIdx;
#pragma prefast(suppress: __WARNING_WRITE_OVERRUN, "pEntry->TxQuality, should not Buffer overrun")
                    pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: (Up) bad tx ok count (L:%d, C:%d)\n", pEntry->LastTxOkCount, (TxSuccess * ratio)));
                }
                else
                {
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: (Up) keep rate-up (L:%d, C:%d)\n", pEntry->LastTxOkCount, (TxSuccess * ratio)));
                }
            }
            else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
            {
                if ((TxErrorRatio >= 50) && (TxErrorRatio >= TrainDown))
                {
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: (Down) direct train down (TxErrorRatio >= TrainDown)\n"));
                }
                else if (((pEntry->LastTxOkCount + 2) >= (TxSuccess * ratio)) && TxErrorRatio != 0)
                {
                    pEntry->CurrTxRateIndex = UpRateIdx;
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: (Down) bad tx ok count (L:%d, C:%d)\n", pEntry->LastTxOkCount, (TxSuccess * ratio)));
                }
                else
                {
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: (Down) keep rate-down (L:%d, C:%d)\n", pEntry->LastTxOkCount, (TxSuccess * ratio)));
                }
            }
        }while (FALSE);

        // if rate-up happen, clear all bad history of all TX rates
        if (pEntry->CurrTxRateIndex > CurrRateIdx)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: ++TX rate from %d to %d \n", CurrRateIdx, pEntry->CurrTxRateIndex));
            
            pEntry->TxRateUpPenalty = 0;
            PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
            PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
        }
        // if rate-down happen, only clear DownRate's bad history
        else if (pEntry->CurrTxRateIndex < CurrRateIdx)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: --TX rate from %d to %d \n", CurrRateIdx, pEntry->CurrTxRateIndex));
            
            pEntry->TxRateUpPenalty = 0;           // no penalty
            pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
            pEntry->PER[pEntry->CurrTxRateIndex] = 0;
        }
        else
        {
            bTxRateChanged = FALSE;
        }

        pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pEntry->CurrTxRateIndex+1)*SIZE_OF_RATE_TABLE_ENTRY];
        if (bTxRateChanged && pNextTxRate)
        {
            ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
        }  
        pNextMacEntry = pNextMacEntry->Next; 
        pMacEntry = NULL;
    }
    
}

//
// The dynamic Tx rate switching for AGS in VHT(Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//
// Return Value:
//  None
//
VOID ApMlmeDynamicTxRateSwitchingAGSv2(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH pNextTxRate = NULL;
    BOOLEAN bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    UCHAR TrainUp = 0, TrainDown = 0;
    CHAR RssiOffsetForAgs = 0;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: ---> %s\n", __FUNCTION__));

    if (! OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("AGS: %s: Disable Tx rate switching for AGS, OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) = %d\n", 
            __FUNCTION__, 
            OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt = %d, TxSuccess = %d, TxRetransmit = %d, TxFailCount = %d, TxErrorRatio = %d\n",
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));

    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    //
    // Select the next upgrade rate and the next downgrade rate, if any
    //
    do 
    {
        if ((pTable == AGS3x3HTRateTable) && 
             (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs3, 
                pEntry->TxQuality[pCurrTxRate->upMcs3], 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));

            //
            // 3x3 STA
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy (different MCS group)
                {
                    UpRateIdx = pCurrTxRate->upMcs3;

                    // MCS group #2 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && 
                         (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs2;
                    }

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 3:
                {
                    UpRateIdx = pCurrTxRate->upMcs3;
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [3x3 STA], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }           
            
            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo)) || 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [3x3 STA, Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs3, 
                    pEntry->TxQuality[pCurrTxRate->upMcs3], 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                     (InitTxRateIdx == AGS2x2HTRateTable[1])) ||
                     ((pTable == Ags2x2VhtRateTable) && 
                     (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));
            
            //
            // 2x2 STA
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy
                {
                    UpRateIdx = pCurrTxRate->upMcs2;

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [2x2 STA], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   

            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs2] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [2x2 STA], Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else 
        {
            //
            // 1x1 STA
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 1:
                case 0:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [1x1 STA], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   
        }

        //
        // The STA uses the best Tx rate at this moment.
        //
        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
            break;
        }
        
        if ((pEntry->TxQuality[UpRateIdx] > 0) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
        }
        else
        {
            break;
        }
    } while (1);

    DownRateIdx = pCurrTxRate->downMcs;

    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }

    //
    // Keep the TxRateChangeAction status
    //
    pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;     

    if (pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
    {
        // force update back to the fixed mode
        if (pPort->CommonCfg.RegTransmitSetting.field.HTMODE >= HTMODE_11B)
        {
            WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.MCS);
            WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.BW);
            WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.ShortGI);
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.STBC);
            
            switch (pPort->CommonCfg.RegTransmitSetting.field.HTMODE)
            {
                case HTMODE_MM:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTMIX);
                    break;
                    
                case HTMODE_GF:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTGREENFIELD);
                    break;

                case HTMODE_11B:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_CCK);
                    break;

                case HTMODE_11G:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_OFDM);
                    break;
            }
        }

        if ((READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg) == STBC_USE) && 
             (pAd->HwCfg.Antenna.field.TxPath >= 2))
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
        }
        else
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        // Double confirm STBC not use if (MCS >7). Although we already make correct rate table.
        // RT2860 PHY just support STBC=1 when MCS<=7(1S)   
        if (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) > MCS_7)
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE,("%s: AGS: Fixed - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, PER=%d%%, Retry=%d, NoRetry=%d\n\n", 
            __FUNCTION__, pEntry->CurrTxRateIndex, 
            READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg), 
            pAGSStatisticsInfo->TxErrorRatio, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxSuccess));

        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: AGS: Rssi = %d, TxSuccess = %lu, TxRetransmit = %lu, TxFailCount = %lu, TxErrorRatio = %lu\n", 
        __FUNCTION__, 
        pAGSStatisticsInfo->RSSI, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: Before - CurrTxRateIdx = %d, MCS = %d, STBC = %d, ShortGI = %d, Mode = %d, TrainUp = %d, TrainDown = %d, NextUp = %d, NextDown = %d, CurrMCS = %d, pEntry->AGSCtrl.MCSGroup = %d, PER = %lu%%, Retry = %lu, NoRetry = %lu\n", 
        __FUNCTION__, 
        CurrRateIdx, 
        pCurrTxRate->CurrMCS, 
        pCurrTxRate->STBC, 
        pCurrTxRate->ShortGI, 
        pCurrTxRate->Mode, 
        TrainUp, 
        TrainDown, 
        UpRateIdx, 
        DownRateIdx, 
        READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
        pEntry->AGSCtrl.MCSGroup, 
        pAGSStatisticsInfo->TxErrorRatio, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxSuccess));

    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        CHAR idx = 0;
        UCHAR TxRateIdx;
        UCHAR MCS0 = 0, MCS1 = 0, MCS2 = 0, MCS3 = 0, MCS4 = 0,  MCS5 =0, MCS6 = 0, MCS7 = 0;   
        UCHAR MCS8 = 0, MCS9 = 0, MCS10 = 0, MCS11 = 0, MCS12 = 0, MCS13 = 0, MCS14 = 0, MCS15 = 0;
        UCHAR MCS16 = 0, MCS17 = 0, MCS18 = 0, MCS19 = 0, MCS20 = 0, MCS21 = 0, MCS22 = 0, MCS23 = 0;
        UCHAR Mcs8Vht1x1 = 0, Mcs9Vht1x1 = 0; // VHT 1x1 MCS 8/9 (256QAM)
        UCHAR CCKMCS0 = 0, CCKMCS1 = 0, CCKMCS2 = 0;


        //
        // Check the existence and index of each needed MCS
        //
        while (idx < pTable[0])
        {
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(idx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

            if (IS_VHT_1X1_256QAM_RATE_TABLE(pTable) &&
                AGS_VHT_NSS0_MCS9(pTable, pCurrTxRate) &&
                VHT_1X1_256QAM_CAPABLE_ON_PEER_STA(pEntry))
            {
                Mcs9Vht1x1 = idx;
            }
            else if (IS_VHT_1X1_256QAM_RATE_TABLE(pTable) &&
                AGS_VHT_NSS0_MCS8(pTable, pCurrTxRate) &&
                VHT_1X1_256QAM_CAPABLE_ON_PEER_STA(pEntry))
            {
                Mcs8Vht1x1 = idx;
            }
            else if(AGS_CCK_MCS0(pTable, pCurrTxRate))
            {
                CCKMCS0 = idx;
            }
            else if(AGS_CCK_MCS1(pTable, pCurrTxRate))
            {
                CCKMCS1 = idx;
            }
            else if(AGS_CCK_MCS2(pTable, pCurrTxRate))
            {
                CCKMCS2 = idx;
            }           
            else if (AGS_HT_MCS0(pTable, pCurrTxRate) ||
                AGS_VHT_NSS0_MCS0(pTable, pCurrTxRate))
            {
                MCS0 = idx;
            }
            else if (AGS_HT_MCS1(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS1(pTable, pCurrTxRate))
            {
                MCS1 = idx;
            }
            else if (AGS_HT_MCS2(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS2(pTable, pCurrTxRate))
            {
                MCS2 = idx;
            }
            else if (AGS_HT_MCS3(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS3(pTable, pCurrTxRate))
            {
                MCS3 = idx;
            }
            else if (AGS_HT_MCS4(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS4(pTable, pCurrTxRate))
            {
                MCS4 = idx;
            }
            else if (AGS_HT_MCS5(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS5(pTable, pCurrTxRate))
            {
                MCS5 = idx;
            }
            else if (AGS_HT_MCS6(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS6(pTable, pCurrTxRate))
            {
                MCS6 = idx;
            }
            else if (AGS_HT_MCS7(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS7(pTable, pCurrTxRate))
            {
                MCS7 = idx;
            }
            else if (AGS_HT_MCS8(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS0(pTable, pCurrTxRate))
            {
                MCS8 = idx;
            }
            else if (AGS_HT_MCS9(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS1(pTable, pCurrTxRate))
            {
                MCS9 = idx;
            }
            else if (AGS_HT_MCS10(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS2(pTable, pCurrTxRate))
            {
                MCS10 = idx;
            }
            else if (AGS_HT_MCS11(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS3(pTable, pCurrTxRate))
            {
                MCS11 = idx;
            }
            else if (AGS_HT_MCS12(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS4(pTable, pCurrTxRate))
            {
                MCS12 = idx;
            }
            else if (AGS_HT_MCS13(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS5(pTable, pCurrTxRate))
            {
                MCS13 = idx;
            }
            else if (AGS_HT_MCS14(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS6(pTable, pCurrTxRate))
            {
                MCS14 = idx;
            }
            else if (AGS_HT_MCS15(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS7(pTable, pCurrTxRate))
            {
                MCS15 = idx;
            }
            else if (AGS_HT_MCS16(pTable, pCurrTxRate))
            {
                MCS16 = idx;
            }
            else if (AGS_HT_MCS17(pTable, pCurrTxRate))
            {
                MCS17 = idx;
            }
            else if (AGS_HT_MCS18(pTable, pCurrTxRate))
            {
                MCS18 = idx;
            }
            else if (AGS_HT_MCS19(pTable, pCurrTxRate))
            {
                MCS19 = idx;
            }
            else if (AGS_HT_MCS20(pTable, pCurrTxRate))
            {
                MCS20 = idx;
            }
            else if (AGS_HT_MCS21(pTable, pCurrTxRate))
            {
                MCS21 = idx;
            }
            else if (AGS_HT_MCS22(pTable, pCurrTxRate))
            {
                MCS22 = idx;
            }
            else if (AGS_HT_MCS23(pTable, pCurrTxRate))
            {
                MCS23 = idx;
            }
            
            idx++;
        }


        if ((pTable == AGS3x3HTRateTable) && 
             (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            RssiOffsetForAgs = AgsRssiOffsetTable[2][pPort->BBPCurrentBW];
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: 2*2, RssiOffsetForAgs=%d\n", __FUNCTION__, RssiOffsetForAgs));

            //
            // 3x3 STA
            //
            if (MCS23 && (pAGSStatisticsInfo->RSSI > (-67 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS23;
            }
            else if (MCS22 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS22;
            }
            else if (MCS21 && (pAGSStatisticsInfo->RSSI > (-72 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS21;
            }
            else if (MCS20 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS20;
            }
            else if (MCS19 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS19;
            }
            else if (MCS18 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS18;
            }
            else if (MCS17 && (pAGSStatisticsInfo->RSSI > (-85 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS17;
            }
            else
            {
                TxRateIdx = MCS16;
            }
            
            pEntry->AGSCtrl.MCSGroup = 3;
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                    (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                    ((pTable == Ags2x2VhtRateTable) && 
                      (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            RssiOffsetForAgs = AgsRssiOffsetTable[1][pPort->BBPCurrentBW];
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: 2*2, RssiOffsetForAgs=%d\n", __FUNCTION__, RssiOffsetForAgs));

            //
            // 2x2 STA
            //
            if (MCS15 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS15;
            }
            else if (MCS14 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS14;
            }
            else if (MCS13 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS13;
            }
            else if (MCS12 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS12;
            }
            else if (MCS11 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS11;
            }
            else if (MCS10 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS10;
            }
            else if (MCS9 && (pAGSStatisticsInfo->RSSI > (-87 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS9;
            }
            else
            {
                TxRateIdx = MCS8;
            }
            
            pEntry->AGSCtrl.MCSGroup = 2;
        } 
        else
        {
            RssiOffsetForAgs = AgsRssiOffsetTable[0][pPort->BBPCurrentBW];
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: 1*1, RssiOffsetForAgs=%d\n", __FUNCTION__, RssiOffsetForAgs));

            //
            // 1x1 STA
            //
            if (Mcs9Vht1x1 && (pAGSStatisticsInfo->RSSI > (-67 + RssiOffsetForAgs)))
            {
                TxRateIdx = Mcs9Vht1x1;
            }
            else if (Mcs8Vht1x1 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffsetForAgs)))
            {
                TxRateIdx = Mcs8Vht1x1;
            }
            else if (MCS7 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS7;
            }
            else if (MCS6 && (pAGSStatisticsInfo->RSSI > (-73 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS6;
            }
            else if (MCS5 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS5;
            }
            else if (MCS4 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS4;
            }
            else if (MCS3 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS3;
            }
            else if (MCS2 && (pAGSStatisticsInfo->RSSI > (-84 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS2;
            }
            else if (MCS1 && (pAGSStatisticsInfo->RSSI > (-89 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS1;
            }
            else if(MCS0 && (pAGSStatisticsInfo->RSSI > (-91 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS0;
            }
            else if (CCKMCS2 && (pAGSStatisticsInfo->RSSI > (-92 + RssiOffsetForAgs)))
            {
                TxRateIdx = CCKMCS2;
            }               
            else if (CCKMCS1 && (pAGSStatisticsInfo->RSSI > (-94 + RssiOffsetForAgs)))
            {
                TxRateIdx = CCKMCS1;
            }
            else
            {
                TxRateIdx = CCKMCS0;
            }

            pEntry->AGSCtrl.MCSGroup = 1;
        }

        pEntry->AGSCtrl.lastRateIdx = pEntry->CurrTxRateIndex;
        pEntry->CurrTxRateIndex = TxRateIdx;
        pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

        ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);

        PlatformZeroMemory(pEntry->TxQuality, (sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH));
        PlatformZeroMemory(pEntry->PER, (sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH));
        
        pEntry->fLastSecAccordingRSSI = TRUE;           

        return;
    }

    //
    // The MCS selection is based on the RSSI and skips the rate tuning this time.
    //
    if (pEntry->fLastSecAccordingRSSI  == TRUE)
    {
        pEntry->fLastSecAccordingRSSI = FALSE;
        pEntry->LastSecTxRateChangeAction = 0;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: The MCS selection is based on the RSSI, and skips the rate tuning this time.\n", 
            __FUNCTION__));

        return;
    }

    do
    {
        BOOLEAN bTrainUpDown = FALSE;
        
        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            bTrainUpDown = TRUE;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }   
        else if (pAGSStatisticsInfo->TxErrorRatio <= TrainUp) // Good quality
        {
            bTrainUpDown = TRUE;
            bUpgradeQuality = TRUE;
            
            if (pEntry->TxQuality[CurrRateIdx])
            {
                pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
            }

            if (pEntry->TxRateUpPenalty)
            {
                pEntry->TxRateUpPenalty--;
            }
            else
            {
                if (pEntry->TxQuality[pCurrTxRate->upMcs3] && (pCurrTxRate->upMcs3 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs3]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs2] && (pCurrTxRate->upMcs2 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs2]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs1] && (pCurrTxRate->upMcs1 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs1]--;
                }               
            }
        }
        else if (pEntry->AGSCtrl.MCSGroup > 0) //even if TxErrorRatio > TrainUp
        {
            if (UpRateIdx != 0)
            {
                bTrainUpDown = FALSE;
                
                if (pEntry->TxQuality[CurrRateIdx])
                {
                    pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
                }

                 if (pEntry->TxQuality[UpRateIdx])
                 {
                    pEntry->TxQuality[UpRateIdx]--; // It may improve next train-up Tx rate's quality
                }
            }
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        //
        // Update the current Tx rate
        //
        if (bTrainUpDown)
        {
            // Downgrade Tx rate
            if ((CurrRateIdx != DownRateIdx) && 
                 (pEntry->TxQuality[CurrRateIdx] >= AGS_TX_QUALITY_WORST_BOUND))
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
            }
            else if ((CurrRateIdx != UpRateIdx) && 
                        (pEntry->TxQuality[UpRateIdx] <= 0)) // Upgrade Tx rate
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
            }
        }
    } while (FALSE);

    // Tx rate up
    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 1))
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train up
        //
        if (!pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning)
        {
            pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                 (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train down
        // 
        if (!pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning)
        {
            pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else // Tx rate remains unchanged.
    {
        pEntry->LastSecTxRateChangeAction = 0; // Tx rate remains unchanged.
        bTxRateChanged = FALSE;
    }

    pEntry->LastTxOkCount = pAGSStatisticsInfo->TxSuccess;

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {
        ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    //
    // RDG threshold control
    //
    if ((pPort->CommonCfg.bRdg == TRUE) && (AreAllStaRdgCapable(pAd) == TRUE)) // RDG capable
    {
        TXOP_THRESHOLD_CFG_STRUC TxopThCfg = {0};
        TX_LINK_CFG_STRUC TxLinkCfg = {0};
        
        if ((pAd->Counter.MTKCounters.OneSecReceivedByteCount > (pAd->Counter.MTKCounters.OneSecTransmittedByteCount * 5)) && 
             (pNextTxRate->CurrMCS != MCS_23) && 
             ((pAd->Counter.MTKCounters.OneSecReceivedByteCount + pAd->Counter.MTKCounters.OneSecTransmittedByteCount) >= (50 * 1024)))
        {
            RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
            TxLinkCfg.field.TxRDGEn = 0;
            RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

            RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
            TxopThCfg.field.RDG_IN_THRES = 0xFF; // Similar to diable Rx RDG
            TxopThCfg.field.RDG_OUT_THRES = 0x00;
            RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

            DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0xFF\n", __FUNCTION__));
        }
        else
        {
            RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
            TxLinkCfg.field.TxRDGEn = 1;
            RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

            RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
            TxopThCfg.field.RDG_IN_THRES = 0x00;
            TxopThCfg.field.RDG_OUT_THRES = 0x00;
            RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

            DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0x00\n", __FUNCTION__));
        }
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: <--- %s\n", __FUNCTION__));
}

//
// The dynamic Tx rate switching for AGS (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//  InitTxRateIdx: The initial Tx rate index (3x3: 0x18, 2x2: 0x0F and 1x1: 0x07)
//
// Return Value:
//  None
//
VOID ApMlmeDynamicTxRateSwitchingAGS(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH pNextTxRate = NULL;
    BOOLEAN bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    UCHAR TrainUp = 0, TrainDown = 0;
    CHAR RssiOffset = 0;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: ---> %s\n", __FUNCTION__));

    if (! OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("AGS: %s: Disable Tx rate switching for AGS, OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) = %d\n", 
            __FUNCTION__, 
            OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt = %d, TxSuccess = %d, TxRetransmit = %d, TxFailCount = %d, TxErrorRatio = %d\n",
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));

    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    //
    // Select the next upgrade rate and the next downgrade rate, if any
    //
    do 
    {
        if ((pTable == AGS3x3HTRateTable) && 
             (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs3, 
                pEntry->TxQuality[pCurrTxRate->upMcs3], 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));

            //
            // 3x3 STA
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy (different MCS group)
                {
                    UpRateIdx = pCurrTxRate->upMcs3;

                    // MCS group #2 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && 
                         (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs2;
                    }

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 3:
                {
                    UpRateIdx = pCurrTxRate->upMcs3;
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [3x3 STA], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }           
            
            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo)) || 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [3x3 STA, Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs3, 
                    pEntry->TxQuality[pCurrTxRate->upMcs3], 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                     (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                     ((pTable == Ags2x2VhtRateTable) && 
                     (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));
            
            //
            // 2x2 STA
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy
                {
                    UpRateIdx = pCurrTxRate->upMcs2;

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [2x2 STA], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   

            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs2] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [2x2 STA], Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else 
        {
            //
            // 1x1 STA
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 1:
                case 0:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [1x1 STA], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   
        }

        //
        // The STA uses the best Tx rate at this moment.
        //
        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
            break;
        }
        
        if ((pEntry->TxQuality[UpRateIdx] > 0) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
        }
        else
        {
            break;
        }
    } while (1);

    DownRateIdx = pCurrTxRate->downMcs;

    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }

    //
    // Keep the TxRateChangeAction status
    //
    pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;     

    if (pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
    {
        // force update back to the fixed mode
        if (pPort->CommonCfg.RegTransmitSetting.field.HTMODE >= HTMODE_11B)
        {
            WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.MCS);
            WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.BW);
            WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.ShortGI);
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.STBC);
            
            switch (pPort->CommonCfg.RegTransmitSetting.field.HTMODE)
            {
                case HTMODE_MM:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTMIX);
                    break;
                    
                case HTMODE_GF:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTGREENFIELD);
                    break;

                case HTMODE_11B:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_CCK);
                    break;

                case HTMODE_11G:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_OFDM);
                    break;
            }
        }

        if ((READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg) == STBC_USE) && 
             (pAd->HwCfg.Antenna.field.TxPath >= 2))
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
        }
        else
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        // Double confirm STBC not use if (MCS >7). Although we already make correct rate table.
        // RT2860 PHY just support STBC=1 when MCS<=7(1S)   
        if (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) > MCS_7)
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE,("%s: AGS: Fixed - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, PER=%d%%, Retry=%d, NoRetry=%d\n\n",
            __FUNCTION__, pEntry->CurrTxRateIndex, 
            READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg), 
            pAGSStatisticsInfo->TxErrorRatio, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxSuccess));

        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: AGS: Rssi = %d, TxSuccess = %lu, TxRetransmit = %lu, TxFailCount = %lu, TxErrorRatio = %lu\n", 
        __FUNCTION__, 
        pAGSStatisticsInfo->RSSI, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: Before - CurrTxRateIdx = %d, MCS = %d, STBC = %d, ShortGI = %d, Mode = %d, TrainUp = %d, TrainDown = %d, NextUp = %d, NextDown = %d, CurrMCS = %d, pEntry->AGSCtrl.MCSGroup = %d, PER = %lu%%, Retry = %lu, NoRetry = %lu\n", 
        __FUNCTION__, 
        CurrRateIdx, 
        pCurrTxRate->CurrMCS, 
        pCurrTxRate->STBC, 
        pCurrTxRate->ShortGI, 
        pCurrTxRate->Mode, 
        TrainUp, 
        TrainDown, 
        UpRateIdx, 
        DownRateIdx, 
        READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
        pEntry->AGSCtrl.MCSGroup, 
        pAGSStatisticsInfo->TxErrorRatio, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxSuccess));

    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        CHAR idx = 0;
        UCHAR TxRateIdx;
        UCHAR MCS0 = 0, MCS1 = 0, MCS2 = 0, MCS3 = 0, MCS4 = 0,  MCS5 =0, MCS6 = 0, MCS7 = 0;   
        UCHAR MCS8 = 0, MCS9 = 0, MCS10 = 0, MCS11 = 0, MCS12 = 0, MCS13 = 0, MCS14 = 0, MCS15 = 0;
        UCHAR MCS16 = 0, MCS17 = 0, MCS18 = 0, MCS19 = 0, MCS20 = 0, MCS21 = 0, MCS22 = 0, MCS23 = 0;

        //
        // Check the existence and index of each needed MCS
        //
        while (idx < pTable[0])
        {
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(idx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

            if (AGS_HT_MCS0(pTable, pCurrTxRate) || 
                AGS_VHT_NSS0_MCS0(pTable, pCurrTxRate))
            {
                MCS0 = idx;
            }
            else if (AGS_HT_MCS1(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS1(pTable, pCurrTxRate))
            {
                MCS1 = idx;
            }
            else if (AGS_HT_MCS2(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS2(pTable, pCurrTxRate))
            {
                MCS2 = idx;
            }
            else if (AGS_HT_MCS3(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS3(pTable, pCurrTxRate))
            {
                MCS3 = idx;
            }
            else if (AGS_HT_MCS4(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS4(pTable, pCurrTxRate))
            {
                MCS4 = idx;
            }
            else if (AGS_HT_MCS5(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS5(pTable, pCurrTxRate))
            {
                MCS5 = idx;
            }
            else if (AGS_HT_MCS6(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS6(pTable, pCurrTxRate))
            {
                MCS6 = idx;
            }
            else if (AGS_HT_MCS7(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS7(pTable, pCurrTxRate))
            {
                MCS7 = idx;
            }
            else if (AGS_HT_MCS8(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS0(pTable, pCurrTxRate))
            {
                MCS8 = idx;
            }
            else if (AGS_HT_MCS9(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS1(pTable, pCurrTxRate))
            {
                MCS9 = idx;
            }
            else if (AGS_HT_MCS10(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS2(pTable, pCurrTxRate))
            {
                MCS10 = idx;
            }
            else if (AGS_HT_MCS11(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS3(pTable, pCurrTxRate))
            {
                MCS11 = idx;
            }
            else if (AGS_HT_MCS12(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS4(pTable, pCurrTxRate))
            {
                MCS12 = idx;
            }
            else if (AGS_HT_MCS13(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS5(pTable, pCurrTxRate))
            {
                MCS13 = idx;
            }
            else if (AGS_HT_MCS14(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS6(pTable, pCurrTxRate))
            {
                MCS14 = idx;
            }
            else if (AGS_HT_MCS15(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS7(pTable, pCurrTxRate))
            {
                MCS15 = idx;
            }
            else if (AGS_HT_MCS16(pTable, pCurrTxRate))
            {
                MCS16 = idx;
            }
            else if (AGS_HT_MCS17(pTable, pCurrTxRate))
            {
                MCS17 = idx;
            }
            else if (AGS_HT_MCS18(pTable, pCurrTxRate))
            {
                MCS18 = idx;
            }
            else if (AGS_HT_MCS19(pTable, pCurrTxRate))
            {
                MCS19 = idx;
            }
            else if (AGS_HT_MCS20(pTable, pCurrTxRate))
            {
                MCS20 = idx;
            }
            else if (AGS_HT_MCS21(pTable, pCurrTxRate))
            {
                MCS21 = idx;
            }
            else if (AGS_HT_MCS22(pTable, pCurrTxRate))
            {
                MCS22 = idx;
            }
            else if (AGS_HT_MCS23(pTable, pCurrTxRate))
            {
                MCS23 = idx;
            }
            
            idx++;
        }
        
        RssiOffset = 0;

        if ((pTable == AGS3x3HTRateTable) && 
             (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            //
            // 3x3 STA
            //
            if (MCS23 && (pAGSStatisticsInfo->RSSI > (-67 + RssiOffset)))
            {
                TxRateIdx = MCS23;
            }
            else if (MCS22 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffset)))
            {
                TxRateIdx = MCS22;
            }
            else if (MCS21 && (pAGSStatisticsInfo->RSSI > (-72 + RssiOffset)))
            {
                TxRateIdx = MCS21;
            }
            else if (MCS20 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffset)))
            {
                TxRateIdx = MCS20;
            }
            else if (MCS19 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffset)))
            {
                TxRateIdx = MCS19;
            }
            else if (MCS18 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffset)))
            {
                TxRateIdx = MCS18;
            }
            else if (MCS17 && (pAGSStatisticsInfo->RSSI > (-85 + RssiOffset)))
            {
                TxRateIdx = MCS17;
            }
            else
            {
                TxRateIdx = MCS16;
            }
            
            pEntry->AGSCtrl.MCSGroup = 3;
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                    (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                    ((pTable == Ags2x2VhtRateTable) && 
                      (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            //
            // 2x2 STA
            //
            if (MCS15 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffset)))
            {
                TxRateIdx = MCS15;
            }
            else if (MCS14 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffset)))
            {
                TxRateIdx = MCS14;
            }
            else if (MCS13 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffset)))
            {
                TxRateIdx = MCS13;
            }
            else if (MCS12 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffset)))
            {
                TxRateIdx = MCS12;
            }
            else if (MCS11 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffset)))
            {
                TxRateIdx = MCS11;
            }
            else if (MCS10 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffset)))
            {
                TxRateIdx = MCS10;
            }
            else if (MCS9 && (pAGSStatisticsInfo->RSSI > (-87 + RssiOffset)))
            {
                TxRateIdx = MCS9;
            }
            else
            {
                TxRateIdx = MCS8;
            }
            
            pEntry->AGSCtrl.MCSGroup = 2;
        } 
        else 
        {
            //
            // 1x1 STA
            //
            if (MCS7 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffset)))
            {
                TxRateIdx = MCS7;
            }
            else if (MCS6 && (pAGSStatisticsInfo->RSSI > (-73 + RssiOffset)))
            {
                TxRateIdx = MCS6;
            }
            else if (MCS5 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffset)))
            {
                TxRateIdx = MCS5;
            }
            else if (MCS4 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffset)))
            {
                TxRateIdx = MCS4;
            }
            else if (MCS3 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffset)))
            {
                TxRateIdx = MCS3;
            }
            else if (MCS2 && (pAGSStatisticsInfo->RSSI > (-84 + RssiOffset)))
            {
                TxRateIdx = MCS2;
            }
            else if (MCS1 && (pAGSStatisticsInfo->RSSI > (-89 + RssiOffset)))
            {
                TxRateIdx = MCS1;
            }
            else
            {
                TxRateIdx = MCS0;
            }
            
            pEntry->AGSCtrl.MCSGroup = 1;
        }

        pEntry->AGSCtrl.lastRateIdx = pEntry->CurrTxRateIndex;
        pEntry->CurrTxRateIndex = TxRateIdx;
        pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

        ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);

        PlatformZeroMemory(pEntry->TxQuality, (sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH));
        PlatformZeroMemory(pEntry->PER, (sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH));
        
        pEntry->fLastSecAccordingRSSI = TRUE;           

        return;
    }

    //
    // The MCS selection is based on the RSSI and skips the rate tuning this time.
    //
    if (pEntry->fLastSecAccordingRSSI  == TRUE)
    {
        pEntry->fLastSecAccordingRSSI = FALSE;
        pEntry->LastSecTxRateChangeAction = 0;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: The MCS selection is based on the RSSI, and skips the rate tuning this time.\n", 
            __FUNCTION__));

        return;
    }

    do
    {
        BOOLEAN bTrainUpDown = FALSE;
        
        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            bTrainUpDown = TRUE;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }   
        else if (pAGSStatisticsInfo->TxErrorRatio <= TrainUp) // Good quality
        {
            bTrainUpDown = TRUE;
            bUpgradeQuality = TRUE;
            
            if (pEntry->TxQuality[CurrRateIdx])
            {
                pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
            }

            if (pEntry->TxRateUpPenalty)
            {
                pEntry->TxRateUpPenalty--;
            }
            else
            {
                if (pEntry->TxQuality[pCurrTxRate->upMcs3] && (pCurrTxRate->upMcs3 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs3]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs2] && (pCurrTxRate->upMcs2 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs2]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs1] && (pCurrTxRate->upMcs1 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs1]--;
                }               
            }
        }
        else if (pEntry->AGSCtrl.MCSGroup > 0) //even if TxErrorRatio > TrainUp
        {
            if (UpRateIdx != 0)
            {
                bTrainUpDown = TRUE;
                
                if (pEntry->TxQuality[CurrRateIdx])
                {
                    pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
                }

                 if (pEntry->TxQuality[UpRateIdx])
                 {
                    pEntry->TxQuality[UpRateIdx]--; // It may improve next train-up Tx rate's quality
                }
            }
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        //
        // Update the current Tx rate
        //
        if (bTrainUpDown)
        {
            // Downgrade Tx rate
            if ((CurrRateIdx != DownRateIdx) && 
                 (pEntry->TxQuality[CurrRateIdx] >= AGS_TX_QUALITY_WORST_BOUND))
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
            }
            else if ((CurrRateIdx != UpRateIdx) && 
                        (pEntry->TxQuality[UpRateIdx] <= 0)) // Upgrade Tx rate
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
            }
        }
    } while (FALSE);

    // Tx rate up
    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 1))
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train up
        //
        if (!pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning)
        {
            pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                 (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train down
        // 
        if (!pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning)
        {
            pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else // Tx rate remains unchanged.
    {
        pEntry->LastSecTxRateChangeAction = 0; // Tx rate remains unchanged.
        bTxRateChanged = FALSE;
    }

    pEntry->LastTxOkCount = pAGSStatisticsInfo->TxSuccess;

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {
        ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    //
    // RDG threshold control
    //
    if ((pPort->CommonCfg.bRdg == TRUE) && (AreAllStaRdgCapable(pAd) == TRUE)) // RDG capable
    {
        TXOP_THRESHOLD_CFG_STRUC TxopThCfg = {0};
        TX_LINK_CFG_STRUC TxLinkCfg = {0};
        
        if ((pAd->Counter.MTKCounters.OneSecReceivedByteCount > (pAd->Counter.MTKCounters.OneSecTransmittedByteCount * 5)) && 
             (pNextTxRate->CurrMCS != MCS_23) && 
             ((pAd->Counter.MTKCounters.OneSecReceivedByteCount + pAd->Counter.MTKCounters.OneSecTransmittedByteCount) >= (50 * 1024)))
        {
            RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
            TxLinkCfg.field.TxRDGEn = 0;
            RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

            RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
            TxopThCfg.field.RDG_IN_THRES = 0xFF; // Similar to diable Rx RDG
            TxopThCfg.field.RDG_OUT_THRES = 0x00;
            RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

            DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0xFF\n", __FUNCTION__));
        }
        else
        {
            RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
            TxLinkCfg.field.TxRDGEn = 1;
            RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

            RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
            TxopThCfg.field.RDG_IN_THRES = 0x00;
            TxopThCfg.field.RDG_OUT_THRES = 0x00;
            RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

            DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0x00\n", __FUNCTION__));
        }
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: <--- %s\n", __FUNCTION__));
}

//
// Auto Tx rate faster train up/down for AGS (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//
// Return Value:
//  None
//
VOID ApMlmeQuickResponeForRateUpExecAGSv2(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH    pNextTxRate = NULL;
    BOOLEAN bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    UCHAR TrainUp = 0, TrainDown = 0;
    CHAR ratio = 0;
    ULONG OneSecTxNoRetryOKRationCount = 0;
    PMP_PORT        pPort = pEntry->pPort;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: ---> %s\n", __FUNCTION__));

    if (pPort)
        pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt = %d, TxSuccess = %d, TxRetransmit = %d, TxFailCount = %d, TxErrorRatio = %d\n",
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));
    
    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    UpRateIdx = DownRateIdx = pEntry->AGSCtrl.lastRateIdx;

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }

    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);

        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            pEntry->CurrTxRateIndex = DownRateIdx;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
        {
            pEntry->CurrTxRateIndex = UpRateIdx;
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt <= 15, train back to original rate\n", 
            __FUNCTION__));
        
        return;
    }

    do
    {
        if (pEntry->LastTimeTxRateChangeAction == 0)
        {
            ratio = 5;
        }
        else
        {
            ratio = 4;
        }

        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        OneSecTxNoRetryOKRationCount = (pAGSStatisticsInfo->TxSuccess * ratio);
        
        // Tx rate down
        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            // Change Auto Rate tuning rule 1: Change from tx_ok to PER
            if (pAGSStatisticsInfo->TxErrorRatio > TrainDown)  // Poor quality
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) bad Tx ok count (Current PER:%d, NewMcs's TrainDown:%d)\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));
            }
            else // Good quality
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) keep rate-up (Current PER:%d, NewMcs's TrainDown:%d)\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));

                PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);

                if (pEntry->AGSCtrl.MCSGroup == 0)
                {
                    if ((pTable == AGS3x3HTRateTable) && 
                         (InitTxRateIdx == AGS3x3HTRateTable[1]))
                    {
                        //
                        // 3x3 STA
                        //
                        pEntry->AGSCtrl.MCSGroup = 3;
                    }
                    else if (((pTable == AGS2x2HTRateTable) && 
                                 (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                                 ((pTable == Ags2x2VhtRateTable) &&
                                   (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                    {
                        //
                        // 2x2 STA
                        //
                        pEntry->AGSCtrl.MCSGroup = 2;
                    }
                    else
                    {
                        pEntry->AGSCtrl.MCSGroup = 1;
                    }
                }
            }
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx)) // Tx rate up
        {
// Don't quick check within train down case
#if 0
            // Change Auto Rate tuning rule 1: Change from tx_ok to PER
            if (pAGSStatisticsInfo->TxErrorRatio > TrainDown)  // Poor quality
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (DOWN) bad tx ok count (Current PER:%d, NewMcs's TrainDown:%d)\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));
            }
            else
            {
                if ((pTable == AGS3x3HTRateTable) && 
                     (InitTxRateIdx == AGS3x3HTRateTable[1]))
                {
                    //
                    // 3x3 STA
                    //
                    pEntry->AGSCtrl.MCSGroup = 3;
                }
                else if (((pTable == AGS2x2HTRateTable) && 
                             (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                             ((pTable == Ags2x2VhtRateTable) && 
                               (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                {
                    //
                    // 2x2 STA
                    //
                    pEntry->AGSCtrl.MCSGroup = 2;
                }
                else
                {
                    pEntry->AGSCtrl.MCSGroup = 1;
                }

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (Down) keep rate-down (Current PER:%d, NewMcs's TrainDown:%d)\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));
            }
#endif          
        }
    }while (FALSE);

    //
    // Last action is rate-up
    //
    if (pEntry->LastSecTxRateChangeAction == 1) 
    {
        // looking for the next group with valid MCS
        if ((pEntry->CurrTxRateIndex != CurrRateIdx) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(DownRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
        }
        
        // UpRateIdx is for temp use in this section
        switch (pEntry->AGSCtrl.MCSGroup)
        {
            case 3: 
            {
                UpRateIdx = pCurrTxRate->upMcs3;
            }
            break;
            
            case 2: 
            {
                UpRateIdx = pCurrTxRate->upMcs2;
            }
            break;
            
            case 1: 
            {
                UpRateIdx = pCurrTxRate->upMcs1;
            }
            break;
            
            case 0: 
            {
                UpRateIdx = CurrRateIdx;
            }
            break;
            
            default: 
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup));
            }
            break;
        }

        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
        }
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: next MCS group,  pEntry->AGSCtrl.MCSGroup = %d\n", 
            __FUNCTION__, 
            pEntry->AGSCtrl.MCSGroup));
        
    }

    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate up
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));  
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0; //restore the TxQuality from max to 0
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                (pEntry->LastSecTxRateChangeAction == 1)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
    }
    else
    {
        bTxRateChanged = FALSE;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: rate is not changed\n", 
            __FUNCTION__));
    }

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {
        ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: <--- %s\n", __FUNCTION__));
}

//
// Auto Tx rate faster train up/down for AGS (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//  InitTxRateIdx: The initial Tx rate index (3x3: 0x18, 2x2: 0x0F and 1x1: 0x07)
//
// Return Value:
//  None
//
VOID ApMlmeQuickResponeForRateUpExecAGS(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo, 
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH    pNextTxRate = NULL;
    BOOLEAN bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    UCHAR TrainUp = 0, TrainDown = 0;
    CHAR ratio = 0;
    ULONG OneSecTxNoRetryOKRationCount = 0;
    PMP_PORT        pPort = pEntry->pPort;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: ---> %s\n", __FUNCTION__));

    if (pPort)
        pPort->SoftAP.ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt = %d, TxSuccess = %d, TxRetransmit = %d, TxFailCount = %d, TxErrorRatio = %d\n",
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));
    
    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    UpRateIdx = DownRateIdx = pEntry->AGSCtrl.lastRateIdx;

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }

    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);

        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            pEntry->CurrTxRateIndex = DownRateIdx;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
        {
            pEntry->CurrTxRateIndex = UpRateIdx;
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt <= 15, train back to original rate\n", 
            __FUNCTION__));
        
        return;
    }

    do
    {
        if (pEntry->LastTimeTxRateChangeAction == 0)
        {
            ratio = 5;
        }
        else
        {
            ratio = 4;
        }

        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        OneSecTxNoRetryOKRationCount = (pAGSStatisticsInfo->TxSuccess * ratio);
        
        // Tx rate down
        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount) // Poor quality
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) bad Tx ok count (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));
            }
            else // Good quality
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) keep rate-up (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));

                PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);

                if (pEntry->AGSCtrl.MCSGroup == 0)
                {
                    if ((pTable == AGS3x3HTRateTable) && 
                         (InitTxRateIdx == AGS3x3HTRateTable[1]))
                    {
                        //
                        // 3x3 STA
                        //
                        pEntry->AGSCtrl.MCSGroup = 3;
                    }
                    else if (((pTable == AGS2x2HTRateTable) && 
                                 (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                                 ((pTable == Ags2x2VhtRateTable) &&
                                   (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                    {
                        //
                        // 2x2 STA
                        //
                        pEntry->AGSCtrl.MCSGroup = 2;
                    }
                    else
                    {
                        pEntry->AGSCtrl.MCSGroup = 1;
                    }
                }
            }
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx)) // Tx rate up
        {
            if ((pAGSStatisticsInfo->TxErrorRatio >= 50) || (pAGSStatisticsInfo->TxErrorRatio >= TrainDown)) // Poor quality
            {
                if ((pTable == AGS3x3HTRateTable) && 
                                     (InitTxRateIdx == AGS3x3HTRateTable[1]))
                {
                    //
                    // 3x3 STA
                    //
                    pEntry->AGSCtrl.MCSGroup = 3;
                }
                else if (((pTable == AGS2x2HTRateTable) && 
                            (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                            ((pTable == Ags2x2VhtRateTable) && 
                              (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                {
                    //
                    // 2x2 STA
                    //
                    pEntry->AGSCtrl.MCSGroup = 2;
                }
                else
                {
                    pEntry->AGSCtrl.MCSGroup = 1;
                }

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (DOWN) direct train down (TxErrorRatio[%d] >= TrainDown[%d])\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio, 
                    TrainDown));
            }
            else if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (DOWN) bad tx ok count (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));
            }
            else
            {
                if ((pTable == AGS3x3HTRateTable) && 
                     (InitTxRateIdx == AGS3x3HTRateTable[1]))
                {
                    //
                    // 3x3 STA
                    //
                    pEntry->AGSCtrl.MCSGroup = 3;
                }
                else if (((pTable == AGS2x2HTRateTable) && 
                             (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                             ((pTable == Ags2x2VhtRateTable) && 
                               (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                {
                    //
                    // 2x2 STA
                    //
                    pEntry->AGSCtrl.MCSGroup = 2;
                }
                else
                {
                    pEntry->AGSCtrl.MCSGroup = 1;
                }

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (Down) keep rate-down (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));
            }
        }
    }while (FALSE);

    //
    // Last action is rate-up
    //
    if (pEntry->LastSecTxRateChangeAction == 1) 
    {
        // looking for the next group with valid MCS
        if ((pEntry->CurrTxRateIndex != CurrRateIdx) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(DownRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
        }
        
        // UpRateIdx is for temp use in this section
        switch (pEntry->AGSCtrl.MCSGroup)
        {
            case 3: 
            {
                UpRateIdx = pCurrTxRate->upMcs3;
            }
            break;
            
            case 2: 
            {
                UpRateIdx = pCurrTxRate->upMcs2;
            }
            break;
            
            case 1: 
            {
                UpRateIdx = pCurrTxRate->upMcs1;
            }
            break;
            
            case 0: 
            {
                UpRateIdx = CurrRateIdx;
            }
            break;
            
            default: 
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup));
            }
            break;
        }

        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
        }
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: next MCS group,  pEntry->AGSCtrl.MCSGroup = %d\n", 
            __FUNCTION__, 
            pEntry->AGSCtrl.MCSGroup));
        
    }

    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate up
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));  
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0; //restore the TxQuality from max to 0
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                (pEntry->LastSecTxRateChangeAction == 1)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
    }
    else
    {
        bTxRateChanged = FALSE;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: rate is not changed\n", 
            __FUNCTION__));
    }

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {       
        ApMlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: <--- %s\n", __FUNCTION__));
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN ApMlmeMsgTypeSubst(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT    pPort,
    IN PFRAME_802_11 pFrame, 
    OUT INT *Machine, 
    OUT INT *MsgType) 
{
    USHORT Seq;
    UCHAR  EAPType; 
    UCHAR   EAPCode;
//TODO:
// only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, 
// ignore this frame
    
    // wpa EAPOL PACKET
    if (pFrame->Hdr.FC.Type == BTYPE_DATA) 
    {
        if (pPort->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP)
        {
            *Machine = WSC_STATE_MACHINE;
            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
            EAPCode = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
            return(WscMsgTypeSubst(EAPType, EAPCode, MsgType));     
        }
        else
        {
            *Machine = AP_WPA_STATE_MACHINE;
            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
            return ApWpaMsgTypeSubst(EAPType, MsgType);
        }
    }

    if (pFrame->Hdr.FC.Type != BTYPE_MGMT)
        return FALSE;

    // Legacy mode will not handle ACTON frame
    if( (pPort->SoftAP.ApCfg.PhyMode < PHY_11ABGN_MIXED) && (pFrame->Hdr.FC.SubType >=SUBTYPE_ACTION))
        return FALSE;

    // Do not process any incoming packets if InfoReset is in progress
    
    switch (pFrame->Hdr.FC.SubType) 
    {
        case SUBTYPE_ASSOC_REQ:
            *Machine = AP_ASSOC_STATE_MACHINE;
            *MsgType = APMT2_PEER_ASSOC_REQ;
            DBGPRINT_RAW(RT_DEBUG_TRACE,("SUBTYPE_ASSOC_REQ\n"));
            break;
//      case SUBTYPE_ASSOC_RSP:
//          *Machine = AP_ASSOC_STATE_MACHINE;
//          *MsgType = APMT2_PEER_ASSOC_RSP;
//          break;
        case SUBTYPE_REASSOC_REQ:
            *Machine = AP_ASSOC_STATE_MACHINE;
            *MsgType = APMT2_PEER_REASSOC_REQ;
            break;
//      case SUBTYPE_REASSOC_RSP:
//          *Machine = AP_ASSOC_STATE_MACHINE;
//          *MsgType = APMT2_PEER_REASSOC_RSP;
//          break;
        case SUBTYPE_PROBE_REQ:
            *Machine = AP_SYNC_STATE_MACHINE;
            *MsgType = APMT2_PEER_PROBE_REQ;
            break;
        case SUBTYPE_PROBE_RSP:
            *Machine = AP_SYNC_STATE_MACHINE;
            *MsgType = APMT2_PEER_PROBE_RSP;
            if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
                && P2P_ON(pPort))
            {
                *Machine = SYNC_STATE_MACHINE;
                *MsgType = MT2_PEER_PROBE_RSP;
            }
            break;
        case SUBTYPE_BEACON:
            *Machine = AP_SYNC_STATE_MACHINE;
            *MsgType = APMT2_PEER_BEACON;
            if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
                && P2P_ON(pPort))
            {
                *Machine = SYNC_STATE_MACHINE;
                *MsgType = MT2_PEER_BEACON;
            }
            break;
//      case SUBTYPE_ATIM:
//          *Machine = AP_SYNC_STATE_MACHINE;
//          *MsgType = APMT2_PEER_ATIM;
//          break;
        case SUBTYPE_DISASSOC:
            *Machine = AP_ASSOC_STATE_MACHINE;
            *MsgType = APMT2_PEER_DISASSOC_REQ;
            break;
        case SUBTYPE_AUTH:
            // get the sequence number from payload 24 Mac Header + 2 bytes algorithm
            PlatformMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
            DBGPRINT(RT_DEBUG_INFO,("AUTH seq=%d Octet=%02x %02x %02x %02x %02x %02x %02x %02x\n", Seq,
                pFrame->Octet[0], pFrame->Octet[1], pFrame->Octet[2], pFrame->Octet[3], 
                pFrame->Octet[4], pFrame->Octet[5], pFrame->Octet[6], pFrame->Octet[7]));
            if (Seq == 1 || Seq == 3) 
            {
                *Machine = AP_AUTH_RSP_STATE_MACHINE;
                *MsgType = APMT2_PEER_AUTH_ODD;
            } 
//          else if (Seq == 2 || Seq == 4) 
//          {
//              *Machine = AP_AUTH_STATE_MACHINE;
//              *MsgType = APMT2_PEER_AUTH_EVEN;
//          } 
            else 
            {
                DBGPRINT(RT_DEBUG_TRACE,("wrong AUTH seq=%d Octet=%02x %02x %02x %02x %02x %02x %02x %02x\n", Seq,
                    pFrame->Octet[0], pFrame->Octet[1], pFrame->Octet[2], pFrame->Octet[3], 
                    pFrame->Octet[4], pFrame->Octet[5], pFrame->Octet[6], pFrame->Octet[7]));
                return FALSE;
            }
            break;
        case SUBTYPE_DEAUTH:
            *Machine = AP_AUTH_RSP_STATE_MACHINE;
            *MsgType = APMT2_PEER_DEAUTH;
            break;
        case SUBTYPE_ACTION:
        case SUBTYPE_ACTION_NO_ACK: 
            *Machine = ACTION_STATE_MACHINE;
            //  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support
            //  Vendor specific usage.
            if ((pFrame->Octet[0] & 0x7F) == MT2_ACT_VENDOR) //  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Vendor Action frame OUI= 0x%x\n", *(PULONG)&pFrame->Octet[1]));
                // Now support WFA P2P
                if (PlatformEqualMemory(&pFrame->Octet[1], P2POUIBYTE, 4) && 
                    (IS_P2P_GO_OP(pPort) || IS_P2P_AUTOGO(pPort) || MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)))

                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Vendor Action frame P2P OUI= 0x%x\n", *(PULONG)&pFrame->Octet[1]));
                    *Machine = P2P_STATE_MACHINE;
                    if ((pFrame->Octet[5] >= 0) && (pFrame->Octet[5] <= MT2_MAX_PEER_SUPPORT))
                    {
                        *MsgType = pFrame->Octet[5]; // subtype. 
                    }
                    else
                        return FALSE;
                }
                else
                {
                    return FALSE;
                }
            } 
            else if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG) 
            {
                *MsgType = MT2_ACT_INVALID;
                DBGPRINT(RT_DEBUG_TRACE, ("Invalid Action frame Category= %d\n", *MsgType));
            } 
            else
            {
                *MsgType = (pFrame->Octet[0]&0x7F);
                DBGPRINT(RT_DEBUG_TRACE, (" Action frame Category= %d\n", *MsgType));
            } 
            break;
        default:
            return FALSE;
            break;
    }

    return TRUE;
}

/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter

    Arguments:
        pAd         - Adapter pointer
        bPiggyBack  - Enable / Disable Piggy-Back

    Return Value:
        None

    ========================================================================
*/
VOID ApMlmeSetPiggyBack(
    IN PMP_ADAPTER    pAd,
    IN BOOLEAN          bPiggyBack)
{
    TX_LINK_CFG_STRUC  TxLinkCfg;   
    RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
    if (bPiggyBack == TRUE)
        TxLinkCfg.field.TxCFAckEn = 1;
    else
        TxLinkCfg.field.TxCFAckEn = 0;
    RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);
}

/*
    ========================================================================
    Routine Description:
        Periodic evaluate antenna link status

    Arguments:
        pAd         - Adapter pointer

    Return Value:
        None

    ========================================================================
*/
VOID ApMlmeAsicEvaluateRxAnt(
    IN PMP_ADAPTER    pAd)
{
    ULONG   TxTotalCnt;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    DBGPRINT(RT_DEBUG_INFO, ("AsicEvaluateRxAnt : RealRxPath=%d \n", pAd->Mlme.RealRxPath));
    
    BbSetRxAntByRxPath(pAd);

    TxTotalCnt = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + 
                    pAd->Counter.MTKCounters.OneSecTxRetryOkCount + 
                    pAd->Counter.MTKCounters.OneSecTxFailCount;

    if (TxTotalCnt > 50)
    {
        PlatformSetTimer(pPort,&pPort->Mlme.RxAntEvalTimer, 20);
        pAd->Mlme.bLowThroughput = FALSE;
    }
    else
    {
        PlatformSetTimer(pPort,&pPort->Mlme.RxAntEvalTimer, 300);
        pAd->Mlme.bLowThroughput = TRUE;
    }
}

/*
    ========================================================================
    Routine Description:
        After evaluation, check antenna link status

    Arguments:
        pAd         - Adapter pointer

    Return Value:
        None

    ========================================================================
*/
VOID ApMlmeAsicRxAntEvalTimeout(
    PMP_ADAPTER   pAd) 
{
    //UCHAR         BBPR3;
    CHAR            larger = -100, rssi0=0, rssi1=0, rssi2=0;
    PMP_PORT        pPort = pAd->BeaconPort.pApGoPort;

    if (pPort != NULL)
    {
        // if the traffic is low, use average rssi as the criteria
        if (pAd->Mlme.bLowThroughput == TRUE)
        {
            rssi0 = pPort->SoftAP.ApCfg.RssiSample.LastRssi[0];
            rssi1 = pPort->SoftAP.ApCfg.RssiSample.LastRssi[1];
            rssi2 = pPort->SoftAP.ApCfg.RssiSample.LastRssi[2];
        }
        else
        {
            rssi0 = (UCHAR)pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0];
            rssi1 = (UCHAR)pPort->SoftAP.ApCfg.RssiSample.AvgRssi[1];
            rssi2 = (UCHAR)pPort->SoftAP.ApCfg.RssiSample.AvgRssi[2];
        }
    }
    //
    // Rx antenna swtich is not for 3x3 NIC
    //
    if((pAd->HwCfg.Antenna.field.TxPath == 2) && (pAd->HwCfg.Antenna.field.RxPath == 3))
    {
        larger = max(rssi0, rssi1);

        if (larger > (rssi2 + 20))
            pAd->Mlme.RealRxPath = 2;
        else
            pAd->Mlme.RealRxPath = 3;
    }
#if 0
    else if(pAd->HwCfg.Antenna.field.RxPath == 2)
    {
        if (rssi0 > (rssi1 + 20))
            pAd->Mlme.RealRxPath = 1;
        else
            pAd->Mlme.RealRxPath = 2;
    }
#endif

    BbSetRxAntByRxPath(pAd);

    DBGPRINT(RT_DEBUG_TRACE, ("ApMlmeAsicRxAntEvalTimeout : RealRxPath=%d, AvgRssi0=%d, AvgRssi1=%d, AvgRssi2=%d \n",
    pAd->Mlme.RealRxPath, rssi0, rssi1, rssi2));
}

VOID ApMlmeSetSupportedLegacyMCS(
    IN PMAC_TABLE_ENTRY     pEntry)
{
    UCHAR idx; 

    //Clear Supported MCS Table
    PlatformZeroMemory(pEntry->SupCCKMCS, MAX_LEN_OF_CCK_RATES);
    PlatformZeroMemory(pEntry->SupOFDMMCS, MAX_LEN_OF_OFDM_RATES);

    for(idx = 0 ; idx < pEntry->RateLen; idx ++)
    {
        switch((pEntry->SupRate[idx]&0x7F)*5)
        {
            case 10:
                pEntry->SupCCKMCS[MCS_0] = TRUE;
                break;
            case 20:
                pEntry->SupCCKMCS[MCS_1] = TRUE;
                break;
            case 55:
                pEntry->SupCCKMCS[MCS_2] = TRUE;
                break;
            case 110:
                pEntry->SupCCKMCS[MCS_3] = TRUE;
                break;                      
            case 60:
                pEntry->SupOFDMMCS[MCS_0] = TRUE;
                break;
            case 90:
                pEntry->SupOFDMMCS[MCS_1] = TRUE;
                break;
            case 120:
                pEntry->SupOFDMMCS[MCS_2] = TRUE;
                break;
            case 180:
                pEntry->SupOFDMMCS[MCS_3] = TRUE;
                break;
            case 240:
                pEntry->SupOFDMMCS[MCS_4] = TRUE;
                break;                      
            case 360:
                pEntry->SupOFDMMCS[MCS_5] = TRUE;
                break;  
            case 480:
                pEntry->SupOFDMMCS[MCS_6] = TRUE;
                break;
            case 540:
                pEntry->SupOFDMMCS[MCS_7] = TRUE;
                break;                      
        }
    }
}

