/*
    ***************************************************************************
    * Ralink Technology, Corp.
    * 5F., No. 5, Tai-Yuan 1st St., Jhubei City, Hsinchu County 30265, Taiwan
    *
    * (c) Copyright 2002-2011, Ralink Technology, Corp.
    *
    * All rights reserved. Ralink's source code is an unpublished work and the use of a copyright notice 
    * does not imply otherwise. This source code contains confidential trade secret material of 
    * Ralink Technology, Corp. Any attempt or participation in deciphering, decoding, reverse engineering 
    * or in any way altering the source code is stricitly prohibited, unless the prior written consent of 
    * Ralink Technology, Corp. is obtained.
    ***************************************************************************

    Module Name: 
    vht.c

    Abstract: 
    VHT protocol control

    Revision History:
    Who                     When            What
    -------------------     ----------      ----------------------------------------------
    Ian Tang                    2011/9/5        Start implementation
*/

#include "MtConfig.h"

//
// Initialize VHT MIMO control field
//
VOID VhtInittMimoCtrlField(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Initialize VHT capabilities IE, ID=191
//
VOID VhtInitCapIE(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    PlatformZeroMemory(&pPort->CommonCfg.DesiredVhtPhy.VhtCapability, SIZE_OF_VHT_CAP_IE);
    PlatformZeroMemory(&pPort->SoftAP.ApCfg.VhtCapability, SIZE_OF_VHT_CAP_IE);

    if (VHT_NIC(pAd))
    {
        //
        // Initialize the VHT Capabilities IE
        //
        
        // 
        //    ElementID + Length + VHTCapabilitiesInfo + VHTSupportedMcsSet
        //    (1)              (1)         (4)                           (8)
        //

        //
        // Initialize the VHT Capabilities Info field (STA)
        //
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.MaxMpduLength = MAX_AMPDU_11454_OCTETS; // 11,454 octets in HW
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.SupportedChannelWidthSet = 0; // Not support either 160MHz or 80+80MHz
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.RxLdpc = 0; // Not support Rx LDPC
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.ShortGIfor80Mhz = 1; // Support Short GI with 80MHz
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.ShortGIfor160Mhz = 0; // Not support Short GI with 160MHz
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.TxStbc = 0; // Not support Tx STBC
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.RxStbc = 1; // Support Rx STBC
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.SuBeamformerCapable = 0; // Not support SU Beamformer

        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.SuBeamformeeCapable = 1; // Indicates support for operation as an SU Beamformee
        
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.CompressedSteeringNumber = 0; // Not support SU Beamformee
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.SoundingDimensionsNum = 0; // Not support sounding packets
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.MuBeamformerCapable = 0; // Not support MU Beamformer
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.MuBeamformeeCapable = 0; // Not support MU Beamformee
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.VhtTxopPs = 0; // Not support VHT TXOP power save
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.HtcVhtCapable = 0; // Not support receiving a VHT variant HT Control field
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.MaxAmpduLenExp = 7; // 7 (2^(13+7)-1 octets)
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.VhtLinkAdaptationCapble = 0; // Not support link adaptation using VHT variant HT Control field
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.RxAntPatternConsistency = 1; // Rx antenna pattern does not change during association; no HW antenna diversity for this purpose
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.TxAntPatternConsistency = 1; // Tx antenna pattern does not change during association; no HW antenna diversity for this purpose

        //
        // Initialize the VHT Supported MCS Set field (STA)
        //

        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss = MCS_0_9; // Support Rx MCS 0-9 for 1 SS
        
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor2ss = NOT_SUPPORTED; // Not support 2 SS Rx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor3ss = NOT_SUPPORTED; // Not support 3 SS Rx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor4ss = NOT_SUPPORTED; // Not support 4 SS Rx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor5ss = NOT_SUPPORTED; // Not support 5 SS Rx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor6ss = NOT_SUPPORTED; // Not support 6 SS Rx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor7ss = NOT_SUPPORTED; // Not support 7 SS Rx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor8ss = NOT_SUPPORTED; // Not support 8 SS Rx
        

        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxHighSupportedDataRate = 0X186; // 390Mbps; 1x1, MCS9, long GI, BW80

        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss = MCS_0_9; // Support Tx MCS 0-9 for 1 SS
   
        
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor2ss = NOT_SUPPORTED; // Not supoort 2 SS Tx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor3ss = NOT_SUPPORTED; // Not support 3 SS Tx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor4ss = NOT_SUPPORTED; // Not support 4 SS Tx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor5ss = NOT_SUPPORTED; // Not support 5 SS Tx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor6ss = NOT_SUPPORTED; // Not support 6 SS Tx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor7ss = NOT_SUPPORTED; // Not support 7 SS Tx
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor8ss = NOT_SUPPORTED; // Not support 8 SS Tx
        
        pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxHighSupportedDataRate = 0x186; // 390Mbps; 1x1, MCS9, long GI, BW80

        //
        // Initialize the VHT Capabilities Info field (SoftAP)
        //
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.MaxMpduLength = MAX_AMPDU_11454_OCTETS; // 11,454 octets in HW
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.SupportedChannelWidthSet = 0; // Not support either 160MHz or 80+80MHz
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.RxLdpc = 0; // Not support Rx LDPC
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.ShortGIfor80Mhz = 1; // Support Short GI with 80MHz
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.ShortGIfor160Mhz = 0; // Not support Short GI with 160MHz
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.TxStbc = 0; // Not support Tx STBC
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.RxStbc = 1; // Support Rx STBC
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.SuBeamformerCapable = 0; // Not support SU Beamformer

        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.SuBeamformeeCapable = 1; // Indicates support for operation as an SU Beamformee

        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.CompressedSteeringNumber = 0; // Not support SU Beamformee
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.SoundingDimensionsNum = 0; // Not support sounding packets
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.MuBeamformerCapable = 0; // Not support MU Beamformer
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.MuBeamformeeCapable = 0; // Not support MU Beamformee
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.VhtTxopPs = 0; // Not support VHT TXOP power save
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.HtcVhtCapable = 0; // Not support receiving a VHT variant HT Control field
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.MaxAmpduLenExp = 7; // 7 (2^(13+7)-1 octets)
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.VhtLinkAdaptationCapble = 0; // Not support link adaptation using VHT variant HT Control field
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.RxAntPatternConsistency = 1; // Rx antenna pattern does not change during association; no HW antenna diversity for this purpose
        pPort->SoftAP.ApCfg.VhtCapability.VhtCapInfo.TxAntPatternConsistency = 1; // Tx antenna pattern does not change during association; no HW antenna diversity for this purpose

        //
        // Initialize the VHT Supported MCS Set field (SoftAP)
        //
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss = MCS_0_9; // Support Rx MCS 0-9 for 1 SS
        
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor2ss = NOT_SUPPORTED; // Not supoort 2 SS Rx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor3ss = NOT_SUPPORTED; // Not support 3 SS Rx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor4ss = NOT_SUPPORTED; // Not support 4 SS Rx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor5ss = NOT_SUPPORTED; // Not support 5 SS Rx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor6ss = NOT_SUPPORTED; // Not support 6 SS Rx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor7ss = NOT_SUPPORTED; // Not support 7 SS Rx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor8ss = NOT_SUPPORTED; // Not support 8 SS Rx
        
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.RxHighSupportedDataRate = 0x186; // 390Mbps; 1x1, MCS9, long GI, BW80

        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss = MCS_0_9; // Support Tx MCS 0-9 for 1 SS

        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor2ss = NOT_SUPPORTED; // Not support 2 SS Tx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor3ss = NOT_SUPPORTED; // Not support 3 SS Tx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor4ss = NOT_SUPPORTED; // Not support 4 SS Tx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor5ss = NOT_SUPPORTED; // Not support 5 SS Tx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor6ss = NOT_SUPPORTED; // Not support 6 SS Tx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor7ss = NOT_SUPPORTED; // Not support 7 SS Tx
        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor8ss = NOT_SUPPORTED; // Not support 8 SS Tx

        pPort->SoftAP.ApCfg.VhtCapability.VhtSupportMcsSet.TxHighSupportedDataRate = 0x186; // 390Mbps; 1x1, MCS9, long GI, BW80

    }
    else
    {
        //2 TODO: Add other VHT NICs
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Initialize VHT operation IE, ID=192
//
VOID VhtInitOpIE(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    PlatformZeroMemory(&pPort->CommonCfg.DesiredVhtPhy.VhtOperation, SIZE_OF_VHT_OP_IE);
    PlatformZeroMemory(&pPort->SoftAP.ApCfg.VhtOperation, SIZE_OF_VHT_OP_IE);

    if (VHT_NIC(pAd))
    {
        //
        // Initialize the VHT Operation IE
        //
        
        // 
        // ElementID + Length + VHTOperationInfo + BasicMcsSet
        // (1)              (1)         (3)                         (2)
        //

        //
        // Initialize the VHT Operation Info field (STA)
        //
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelWidth = VHT_BW_80;
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1 = 0; // Based on the channel center frequency the VHT NIC used
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg2 = 0; // Not support 80+80MHz VHT BSS

        //
        // Initialize the VHT Basic MCS Set (STA)
        //
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss = MCS_0_9; // Support MCS 0-9 for 1 SS
        
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss = NOT_SUPPORTED; // Not support 2 SS
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor3ss = NOT_SUPPORTED; // Not support 3 SS
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor4ss = NOT_SUPPORTED; // Not support 4 SS
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor5ss = NOT_SUPPORTED; // Not support 5 SS
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor6ss = NOT_SUPPORTED; // Not support 6 SS
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor7ss = NOT_SUPPORTED; // Not support 7 SS
        pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor8ss = NOT_SUPPORTED; // Not support 8 SS

        //
        // Initialize the VHT Operation Info field (SoftAP)
        //
        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth = VHT_BW_80;
        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1 = 0; // Based on the channel center frequency the VHT NIC used
        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg2 = 0; // Not support 80+80MHz VHT BSS

        //
        // Initialize the VHT Basic MCS Set (SoftAP)
        //
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss = MCS_0_9; // Supporot MCS 0-9 for 1 SS
        
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss = NOT_SUPPORTED; // Not support 2 SS
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor3ss = NOT_SUPPORTED; // Not support 3 SS
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor4ss = NOT_SUPPORTED; // Not support 4 SS
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor5ss = NOT_SUPPORTED; // Not support 5 SS
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor6ss = NOT_SUPPORTED; // Not support 6 SS
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor7ss = NOT_SUPPORTED; // Not support 7 SS
        pPort->SoftAP.ApCfg.VhtOperation.VhtBasicMcsSet.MaxMcsFor8ss = NOT_SUPPORTED; // Not support 8 SS
    }
    else
    {
        //2 TODO: Add other VHT NICs
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Initialize VHT Ext BSS load IE, ID=193
//
VOID VhtInitExtBssLoadIE(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)    
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));
    
    PlatformZeroMemory(&pPort->SoftAP.ApCfg.VhtExtBssLoad, SIZE_OF_VHT_EXT_BSS_LOAD_IE);

    //2 TODO: need more modifications later
    if (VHT_NIC(pAd))
    {
        // initial VhtInitOpIE of STA
        // 
        //    ElementID + Length + MuMimoCapableStaCount + SpatialStreamUnderutilization        
        //       (1)        (1)           (1)                          (1)
        //
        //    + Vht40MhzUtilization + Vht80MhzUtilization + Vht160MhzUtilization 
        //              (1)                   (1)                  (1)
        // initial VhtExtBssLoadIE of STA       
        /*
        pAd->VhtCtrl.StaVhtIEs.VhtExtBssLoadIE.MuMimoCapableStaCount = 0;
        pAd->VhtCtrl.StaVhtIEs.VhtExtBssLoadIE.SpatialStreamUnderutilization = 0;
        pAd->VhtCtrl.StaVhtIEs.VhtExtBssLoadIE.Vht40MhzUtilization = 0;
        pAd->VhtCtrl.StaVhtIEs.VhtExtBssLoadIE.Vht80MhzUtilization = 0;
        pAd->VhtCtrl.StaVhtIEs.VhtExtBssLoadIE.Vht160MhzUtilization = 0;
        */

        // initial VhtExtBssLoadIE of SoftAP
        pPort->SoftAP.ApCfg.VhtExtBssLoad.MuMimoCapableStaCount = 0;
        pPort->SoftAP.ApCfg.VhtExtBssLoad.SpatialStreamUnderutilization = 0;
        pPort->SoftAP.ApCfg.VhtExtBssLoad.Vht40MhzUtilization = 0;
        pPort->SoftAP.ApCfg.VhtExtBssLoad.Vht80MhzUtilization = 0;
        pPort->SoftAP.ApCfg.VhtExtBssLoad.Vht160MhzUtilization = 0;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Initialize VHT WIDE BW Channel Switch IE, ID=194
//
VOID VhtInitBideBWChannelSwitchIE(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //2 TODO
    if (VHT_NIC(pAd))
    {
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Initialize VHT TxPower Envelope IE, ID=195
//
VOID VhtInitTxPowerEnvelopeIE(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //2 TODO
    if (VHT_NIC(pAd))
    {
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}


//
// Initialize VHT Ext Power Contraint IE, ID=196
//
VOID VhtInitExtPowerConstraintIE(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //2 TODO
    if (VHT_NIC(pAd))
    {
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}


//
// Initialize VHT AID IE, ID=197
//
VOID VhtInitAidIE(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //2 TODO
    if (VHT_NIC(pAd))
    {
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}


//
// Initialize VHT Quiet Channel IE, ID=198
//
VOID VhtInitQuietChannelIE(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //2 TODO
    if (VHT_NIC(pAd))
    {
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// In VHT, use PrimaryChannel and BW to decide VhtCentralChannel
//
VOID Vht5GCentralChannelDecision(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           PrimaryChannel,
    IN  UCHAR           BW,
    OUT PUCHAR          pCentralChannel)
{
    ULONG i;
    *pCentralChannel = 0;

    if (PrimaryChannel<=14)
        return;
    
    for(i=0;i<NUM_OF_VALID_VHT_CHANNEL_TABLE;i++)
    {
        if(PrimaryChannel == ValidVhtChannelTable[i].PrimaryChannel)
        {
            if(BW == ValidVhtChannelTable[i].BandWidth)
            {
                *pCentralChannel = ValidVhtChannelTable[i].CentralChannel;              
                DBGPRINT(RT_DEBUG_TRACE, ("Central Channel = %d\n",*pCentralChannel));
                return;
            }           
        }
    }   
    DBGPRINT(RT_DEBUG_ERROR,("Invalid combination of PrimaryChannel(=%d) and BW(=%d)!!!\n",PrimaryChannel,BW));
}

VOID VhtDecideTheMaxMCSForSpatialStream(PMCS_MAP Result,PMCS_MAP STA1,PMCS_MAP STA2)
{
    if(STA1->MaxMcsFor1ss==NOT_SUPPORTED || STA2->MaxMcsFor1ss==NOT_SUPPORTED)
        Result->MaxMcsFor1ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor1ss = (STA1->MaxMcsFor1ss < STA2->MaxMcsFor1ss)?(STA1->MaxMcsFor1ss):(STA2->MaxMcsFor1ss);

    if(STA1->MaxMcsFor2ss==NOT_SUPPORTED || STA2->MaxMcsFor2ss==NOT_SUPPORTED)
        Result->MaxMcsFor2ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor2ss = (STA1->MaxMcsFor2ss < STA2->MaxMcsFor2ss)?(STA1->MaxMcsFor2ss):(STA2->MaxMcsFor2ss);

    if(STA1->MaxMcsFor3ss==NOT_SUPPORTED || STA2->MaxMcsFor3ss==NOT_SUPPORTED)
        Result->MaxMcsFor3ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor3ss = (STA1->MaxMcsFor3ss < STA2->MaxMcsFor3ss)?(STA1->MaxMcsFor3ss):(STA2->MaxMcsFor3ss);

    if(STA1->MaxMcsFor4ss==NOT_SUPPORTED || STA2->MaxMcsFor4ss==NOT_SUPPORTED)
        Result->MaxMcsFor4ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor4ss = (STA1->MaxMcsFor4ss < STA2->MaxMcsFor4ss)?(STA1->MaxMcsFor4ss):(STA2->MaxMcsFor4ss);

    if(STA1->MaxMcsFor5ss==NOT_SUPPORTED || STA2->MaxMcsFor5ss==NOT_SUPPORTED)
        Result->MaxMcsFor5ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor5ss = (STA1->MaxMcsFor5ss < STA2->MaxMcsFor5ss)?(STA1->MaxMcsFor5ss):(STA2->MaxMcsFor5ss);

    if(STA1->MaxMcsFor6ss==NOT_SUPPORTED || STA2->MaxMcsFor6ss==NOT_SUPPORTED)
        Result->MaxMcsFor6ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor6ss = (STA1->MaxMcsFor6ss < STA2->MaxMcsFor6ss)?(STA1->MaxMcsFor6ss):(STA2->MaxMcsFor6ss);

    if(STA1->MaxMcsFor7ss==NOT_SUPPORTED || STA2->MaxMcsFor7ss==NOT_SUPPORTED)
        Result->MaxMcsFor7ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor7ss = (STA1->MaxMcsFor7ss < STA2->MaxMcsFor7ss)?(STA1->MaxMcsFor7ss):(STA2->MaxMcsFor7ss);

    if(STA1->MaxMcsFor8ss==NOT_SUPPORTED || STA2->MaxMcsFor8ss==NOT_SUPPORTED)
        Result->MaxMcsFor8ss = NOT_SUPPORTED;
    else
        Result->MaxMcsFor8ss = (STA1->MaxMcsFor8ss < STA2->MaxMcsFor8ss)?(STA1->MaxMcsFor8ss):(STA2->MaxMcsFor8ss); 
}
//////////////////////////////////// End of File ////////////////////////////////////

