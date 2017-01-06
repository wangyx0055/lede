/*
 **********************************************************************************************
 * Ralink Technology Corporation
 * 5F, No.36, Tai-Yuen Street, Jhubei City HsinChu Hsien 302, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology Corporation
 *
 * All rights reserved. Ralink's source code is an unpublished work and the use of a copyright notice does not imply otherwise. 
 * This source code contains confidential trade secret material of Ralink Tech. Any attemp or participation in deciphering, 
 * decoding, reverse engineering or in any way altering the source code is stricitly prohibited, unless the prior written consent 
 * of Ralink Technology Corporation is obtained.
 **********************************************************************************************

    Module Name:
    tpc.c

    Abstract:
    The function definitions for Transmit Power Control (TPC) in IEEE 802.11d and IEEE 802.11h
    a) Operation in additional multi regulatory domains
    b) Spectrum and transmit power management extensions in the 5 GHz band in Europe

    Reference:
    a) IEEE 802.11d, 2001
    b) IEEE 802.11h, 2003

    Note: MDSM is short for Multi Domain and Specturm Management

    Revision History:
    Who                 When            What
    --------        ----------      ----------------------------------------------
    Ian Tang        12/24/08    Create
    Ian Tang        03/03/09    TPC

*/

#include "MtConfig.h"

//
// 802.11d+h IE code
//
UCHAR dot11dhCountry = IE_DOT11DH_COUNTRY;
UCHAR dot11dhHoppingPatternParameters = IE_DOT11DH_HOPPING_PATTERN_PARAMETERS;
UCHAR dot11dhHoppingPatternTable = IE_DOT11DH_HOPPING_PATTERN_TABLE;
UCHAR dot11dhRequest = IE_DOT11DH_REQUEST;
UCHAR dot11dhPowerConstraint = IE_DOT11DH_POWER_CONSTRAINT;
UCHAR dot11dhPowerCapability = IE_DOT11DH_POWER_CAPABILITY;
UCHAR dot11dhTPCRequest = IE_DOT11DH_TPC_REQUEST;
UCHAR dot11dhTPCReport = IE_DOT11DH_TPC_REPORT;
UCHAR dot11dhSupportedChannel = IE_DOT11DH_SUPPORTED_CHANNELS;
UCHAR dot11dhChannelSwitchAnnouncement = IE_DOT11DH_CHANNEL_SWITCH_ANNOUNCEMENT;
UCHAR dot11dhMeasurementRequest = IE_DOT11DH_MEASUREMENT_REQUEST;
UCHAR dot11dhMeasurementReport = IE_DOT11DH_MEASUREMENT_REPORT;
UCHAR dot11dhQuiet = IE_DOT11DH_QUIET;
UCHAR dot11dhIbssDFS = IE_DOT11DH_IBSS_DFS;

//
// Multi domain implementations
//

//
// Multi domain, spectrum and transmit power management initialization
//
// Parameters
//  pAd: The adapter data structure
//  bEnableMultiDomainCapability: Enable the multi domain functionality or not
//  bEnableSpectrumManagement: Enable the spectrum and transmit power management or not
//
// Return Value:
//  None
//
VOID InitMDSMCtrl(
    IN PMP_ADAPTER pAd, 
    IN BOOLEAN bEnableMultiDomainCapability, 
    IN BOOLEAN bEnableSpectrumManagement)
{
    UCHAR MaxTxPowerCapabilityInEEPROM = MSDM_ABSENT_MAX_TX_POWER_CAPABILITY;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("TPC: ---> %s\n", __FUNCTION__));

    //
    // Multi domain functionality
    //
    pAd->StaCfg.MDSMCtrl.dot11MultiDomainCapabilityEnabled = bEnableMultiDomainCapability;

    //
    // The defined TPC and DFS procedures
    //
    pAd->StaCfg.MDSMCtrl.dot11SpectrumManagementRequired = bEnableSpectrumManagement;

    //
    // Initialize the maximum/minimum transmit power with which the STA is capable of transmitting in the current channel, in units of dBm
    //
    MaxTxPowerCapabilityInEEPROM = (pPort->CommonCfg.DefineMaxTxPwr & MSDM_MAX_TX_POWE_CAPABILITY_MASK);

    if ((MaxTxPowerCapabilityInEEPROM == MSDM_ABSENT_MAX_TX_POWER_CAPABILITY) || 
         (MaxTxPowerCapabilityInEEPROM < MSDM_LOWER_BOUND_OF_MAX_TX_POWER_CAPABILITY) || 
         (MaxTxPowerCapabilityInEEPROM > MSDM_UPPER_BOUND_OF_MAX_TX_POWER_CAPABILITY))
    {
        pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MaxTxPowerCapability = MSDM_DEFAULT_MAX_TX_POWER_CAPABILITY;
        pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MinTxPowerCapability = MSDM_DEFAULT_MIN_TX_POWER_CAPABILITY;
    }
    else
    {
        pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MaxTxPowerCapability = MaxTxPowerCapabilityInEEPROM;
        pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MinTxPowerCapability = MSDM_DEFAULT_MIN_TX_POWER_CAPABILITY;
    }

    pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.RealTxPower = MSDM_DEFAULT_MAX_TX_POWER_CAPABILITY;

    DBGPRINT(RT_DEBUG_TRACE, ("TPC: %s, pAd->StaCfg.MDCtrl.dot11MultiDomainCapabilityEnabled = %d, pAd->StaCfg.MDSMCtrl.dot11SpectrumManagementRequired = %d\n", __FUNCTION__, pAd->StaCfg.MDSMCtrl.dot11MultiDomainCapabilityEnabled, pAd->StaCfg.MDSMCtrl.dot11SpectrumManagementRequired));
    DBGPRINT(RT_DEBUG_TRACE, ("TPC: %s, MaxTxPowerCapability = %d, MinTxPowerCapability = %d\n", 
        __FUNCTION__, 
        pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MaxTxPowerCapability, 
        pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MinTxPowerCapability));

    DBGPRINT(RT_DEBUG_TRACE, ("TPC: <--- %s\n", __FUNCTION__));

}

//
// Calculate the maximum transmit power level
//
// Parameters
//  pAd: The adapter data structure
//  MaxTxPowerLevel: The maximum transmit power level specified for the channel in the country element, in dBm
//  LocalPowerConstraint: The local power constraint specified for the channel in the power constranit element, in dB
//
// Return Value:
//  TRUE: Success
//  FALSE: Calculation failure
//
BOOLEAN MDSMCalculateMaxTxPower(
    IN PMP_ADAPTER pAd, 
    IN UCHAR MaxTxPowerLevel, 
    IN UCHAR LocalPowerConstraint)
{
    BOOLEAN bResult = FALSE;
    UCHAR MaxTxPower = 0;

    DBGPRINT(RT_DEBUG_INFO, ("TPC: ---> %s\n", __FUNCTION__));

    if (LocalPowerConstraint == MDSM_ABSENT_POWER_CONSTRAINT_IE)
    {
        LocalPowerConstraint = 0; // No mitigation requirements
    }

    //
    // Pessimistic approach: 
    // The local maximum transmit power for a channel is defined as the maximum transmit power level specified for the channel 
    // in the country element minus the local power constraint specified for the channel in the power constraint element.
    //
    MaxTxPower = MaxTxPowerLevel - LocalPowerConstraint;

    if (MaxTxPower < 0)
    {
        bResult = FALSE;
        
        DBGPRINT(RT_DEBUG_ERROR, ("TPC: %s: Incorrect maximum transmit power (MaxTxPower = %d, MaxTxPowerLevel = %d, LocalPowerConstraint = %d)\n", 
            __FUNCTION__, 
            MaxTxPower, 
            MaxTxPowerLevel, 
            LocalPowerConstraint));
    }
    else
    {
        bResult = TRUE;
        pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.acceptableMaxTxPower = MaxTxPower;

        DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.acceptableMaxTxPower = %d, MaxTxPower = %d dBm, MaxTxPowerLevel = %d dBm, LocalPowerConstraint = %d dBm\n", 
            __FUNCTION__, 
            pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.acceptableMaxTxPower, 
            MaxTxPower, 
            MaxTxPowerLevel, 
            LocalPowerConstraint));
    }

    DBGPRINT(RT_DEBUG_INFO, ("TPC: <--- %s\n", __FUNCTION__));

    return bResult;
}

//
// Create a power capability IE
//
// Parameters
//  pAd: The adapter data structure
//  pPowerCapabilityIE: Pointer to a caller-supplied variable in which stores the power capability IE
//
// Return Value
//  None
//
VOID MDSMCreatePowerCapabilityIE(
    IN PMP_ADAPTER pAd, 
    IN OUT PPOWER_CAPABILITY_IE pPowerCapabilityIE)
{
    DBGPRINT(RT_DEBUG_INFO, ("TPC: ---> %s\n", __FUNCTION__));

    PlatformZeroMemory(pPowerCapabilityIE, sizeof(POWER_CAPABILITY_IE));

    pPowerCapabilityIE->ElementID = IE_DOT11DH_POWER_CAPABILITY;
    pPowerCapabilityIE->Length = DOT11DH_POWER_CAPABILITY_LENGTH;
    pPowerCapabilityIE->MinTxPowerCapability = pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MinTxPowerCapability;
    pPowerCapabilityIE->MaxTxPowerCapability = pAd->StaCfg.MDSMCtrl.HwTxPowerSetting.MaxTxPowerCapability;

    DBGPRINT(RT_DEBUG_INFO, ("TPC: <--- %s\n", __FUNCTION__));
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          End of File
////////////////////////////////////////////////////////////////////////////////////////////////

