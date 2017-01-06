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
     sync.c
 
     Abstract:
     Synchronization state machine related services
 
     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
     John Chang  08-04-2003    created for 11g soft-AP
 
 */

#include "MtConfig.h"

#define DRIVER_FILE         0x00600000

#define A_BAND_MAP_SIZE 11

UCHAR A_BAND_CHANNEL_MAPPING_LIST[]={36,44,52,60,100,108,116,124,132,149,157};      

/*
    ==========================================================================
    Description:
        The sync state machine, 
    Parameters:
        Sm - pointer to the state machine
    Note:
        the state machine looks like the following

                            AP_SYNC_IDLE
    APMT2_PEER_PROBE_REQ    peer_probe_req_action
    ==========================================================================
 */
VOID ApMlmeSyncStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(Sm, Trans, AP_MAX_SYNC_STATE, AP_MAX_SYNC_MSG, Drop, AP_SYNC_IDLE, AP_SYNC_MACHINE_BASE);

    StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_PROBE_REQ, ApSyncMlmeSyncPeerProbeReqAction); 
    StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_PROBE_RSP, ApSyncMlmeSyncPeerProbeRspAction); 
    StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_BEACON, ApSyncMlmeSyncPeerBeaconAction); 
}

/*
    ==========================================================================
    Description:
        Process the received ProbeRequest from clients
    Parameters:
        Elem - msg containing the ProbeReq frame
    ==========================================================================
 */
VOID ApSyncMlmeSyncPeerProbeReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR         Addr2[MAC_ADDR_LEN];
    CHAR          Ssid[MAX_LEN_OF_SSID];
    UCHAR         SsidLen; //, Rates[MAX_LEN_OF_SUPPORTED_RATES], RatesLen;
    NDIS_STATUS   NStatus;
    PUCHAR        pOutBuffer = NULL;
    ULONG         FrameLen = 0;

    PMP_PORT  pPort = pAd->PortList[Elem->PortNum]; 
    //fix AP stop but still send out probe response
    if((pPort->PortNumber == 1 && pPort->SoftAP.bAPStart == FALSE) && (pPort->PortSubtype != PORTSUBTYPE_P2PGO))
    {
        return;
    }
    
#if 0    
    // Just response in own operation channel
    // We don't know current scan state, so check both CentralChannel and CommonChannel
    if ((pAd->HwCfg.LatchRfRegs.Channel != pPort->CentralChannel) && (pAd->HwCfg.LatchRfRegs.Channel != pPort->Channel))
    {
        return;
    }
#endif
    
    if (! ApSanityPeerProbeReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, Ssid, &SsidLen)) //, Rates, &RatesLen))
    {
        return;
    }

    if (((SsidLen == 0) && (! pPort->SoftAP.ApCfg.bHideSsid)) || 
        ((SsidLen == pPort->PortCfg.SsidLen) && PlatformEqualMemory(Ssid, pPort->PortCfg.Ssid, (ULONG) SsidLen)) ||
        ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (PlatformEqualMemory(WILDP2PSSID, Ssid, WILDP2PSSIDLEN))))
    {
        // allocate and send out ProbeRsp frame
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (NStatus != NDIS_STATUS_SUCCESS)
            return;

        DBGPRINT(RT_DEBUG_INFO, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5] ));

        MgntPktConstructAPProbRspFrame(pAd, pPort, Elem, Addr2, Ssid, SsidLen, pOutBuffer, &FrameLen);
        
        // 802.11n 11.1.3.2.2 active scanning. sending probe response with MCS rate is 
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

//CleanUp:


        
    }
}

/* 
    ==========================================================================
    Description:

    NOTE:
    ==========================================================================
 */
VOID ApSyncMlmeSyncPeerProbeRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PRT_P2P_CLIENT_ENTRY pP2PBssEntry = NULL;
    PRT_P2P_DISCOVERY_ENTRY pP2PDiscoEntry = NULL;

    if (pPort->PortSubtype != PORTSUBTYPE_P2PGO)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!!!ApSyncMlmeSyncPeerProbeRspAction return. Port= %d !!! \n",  Elem->PortNum));
        return;
    }

    PeerP2pBeaconProbeRspAtScan(pAd, pPort, Elem, 0, pPort->Channel, &pP2PBssEntry, &pP2PDiscoEntry);

}

/* 
    ==========================================================================
    Description:
        parse the received BEACON

    NOTE:
        The only thing AP cares about received BEACON frames is to decide 
        if there's any overlapped legacy BSS condition (OLBC).
        If OLBC happened, this AP should set the ERP->Use_Protection bit in its 
        outgoing BEACON. The result is to tell all its clients to use RTS/CTS
        or CTS-to-self protection to protect B/G mixed traffic
    ==========================================================================
 */
VOID ApSyncMlmeSyncPeerBeaconAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR           Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
    CHAR            Ssid[MAX_LEN_OF_SSID];
    UCHAR           SsidLen, BssType, Channel, Rates[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR           RatesLen;
    USHORT          CapabilityInfo, BeaconPeriod;
    LARGE_INTEGER   TimeStamp;
    BOOLEAN         ExtendedRateIeExist;
    BOOLEAN         LegacyBssExist;
    UCHAR           Erp;
    UCHAR           HtCapabilityLen, VHtCapabilityLen;
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    
    if (ApSanityMlmeSyncPeerBeaconAndProbeRspSanity(pAd, 
                                pPort,
                                Elem->Msg, 
                                Elem->MsgLen, 
                                Addr2, 
                                Bssid, 
                                Ssid, 
                                &SsidLen, 
                                &BssType, 
                                &BeaconPeriod, 
                                &Channel, 
                                &TimeStamp, 
                                &CapabilityInfo, 
                                Rates, 
                                &RatesLen,
                                &HtCapabilityLen,
                                &VHtCapabilityLen,
                                &ExtendedRateIeExist,
                                &Erp))
    {
        if (Channel == pPort->SoftAP.ApCfg.AutoChannel_Channel)
        {
            /* CHAR RealRssi = MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_NO_1, Elem->AntSel, BW_20);
            // record the max RSSI of any received BEACON frames. APStartUp phase will
            // use this information to help select a less interference channel
            // TODO: 2005-03-04 this is a dirty patch. we should change all RSSI related variables to UNSIGNED SHORT for easy reading and calculation
            if ((RealRssi + pAd->HwCfg.BbpRssiToDbmDelta) > pPort->SoftAP.ApCfg.AutoChannel_MaxRssi)
                pPort->SoftAP.ApCfg.AutoChannel_MaxRssi = RealRssi + pAd->HwCfg.BbpRssiToDbmDelta;*/
        }

        // ignore BEACON not in this channel
        if (Channel != pPort->Channel)
            return;

        if ((Erp & 0x01) || (RatesLen <= 4))
            LegacyBssExist = TRUE;
        else
            LegacyBssExist = FALSE;

        if (LegacyBssExist)
        {
            pPort->SoftAP.ApCfg.LastOLBCDetectTime = pAd->Mlme.Now64;
            DBGPRINT(RT_DEBUG_INFO, ("%02x:%02x:%02x:%02x:%02x:%02x is a legacy BSS (rate# =%d, ERP=%d), set Use_Protection bit\n", 
                Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], RatesLen, Erp));
        }

        if ((pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED) && (HtCapabilityLen == 0) && (Elem->Rssi < 0x38) && ((pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode %2) == 0))
        {
            //turn on Protection because there exists legacy AP in my control channel 
            //and this AP's beacon has larger dbm ~> -80dbm, So I consider it will has overlapping issue.. 
            pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 1;
            MtAsicUpdateProtect(pAd, pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, TRUE);
        }

        //comment out except we need it
        /*
        if ((VHtCapabilityLen == 0) && (Elem->Rssi0 < 0x38) && ((pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode%2) == 0))
        {
            //turn on Protection because there exists legacy AP in my control channel 
            //and this AP's beacon has larger dbm ~> -80dbm, So I consider it will has overlapping issue.. 
            pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 1;
            MtAsicUpdateProtect(pAd, pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, TRUE);
            DBGPRINT(RT_DEBUG_TRACE, ("ApSyncMlmeSyncPeerBeaconAction - Find Leacy beacon in my BSS. OperaionMode = %d \n",   pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode));
        }
        */
    }
    // sanity check fail, ignore this frame
}

VOID APABandGet40MChannel( IN UCHAR inChannel,IN UCHAR *ExtOffset)
{
    INT     i;
    for(i = 0; i < A_BAND_MAP_SIZE; i++)
    {
        if( inChannel == A_BAND_CHANNEL_MAPPING_LIST[i])
        {
            *ExtOffset = EXTCHA_ABOVE;
            break;
        }
        else if(( inChannel  == (A_BAND_CHANNEL_MAPPING_LIST[i] + 4)) && (inChannel != 165))
        {
            *ExtOffset = EXTCHA_BELOW;
            break;
        }
        else if(inChannel == 165)
        {
            *ExtOffset = EXTCHA_NONE;
            break;
        }
    }
}

