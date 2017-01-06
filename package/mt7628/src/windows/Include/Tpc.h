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
    tpc.h

    Abstract:
    The data structures and the function prototypes for Transmit Power Control (TPC) in IEEE 802.11d and IEEE 802.11h
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

#ifndef __TPC_H__
#define __TPC_H__

#pragma pack(push, struct_pack1)
#pragma pack(1)

//
// The length of the country string field in the country IE
//
#define LENGTH_OF_COUNTRY_STRING_FIELD      3

//
// The length of the country triplet field, including the first channel number, the number of channels and the maximum transmit power level
//
#define LENGTH_OF_COUNTRY_TRIPLET_FIELD     3

//
// The minimum length of the country IE, exclude the element ID field and the length field
//
#define MINIMUM_LENGTH_OF_COUNTRY_IE        6

//
// The country triplet filed in the country IE
//
typedef struct _COUNTRY_TRIPLET
{
    UCHAR   MlmeSyncFirstChannelNumber;
    UCHAR   NumberOfChannels;
    UCHAR   MaxTxPowerLevel;
} COUNTRY_TRIPLET, *PCOUNTRY_TRIPLET;

//
// Country IE
//
typedef struct _COUNTRY_IE
{
    UCHAR           ElementID;
    UCHAR           Length;
    UCHAR           CountryString[LENGTH_OF_COUNTRY_STRING_FIELD];
    COUNTRY_TRIPLET CountryTriplet; // variable
} COUNTRY_IE, *PCOUNTRY_IE;

//
// Power constraint IE
//
typedef struct _POWER_CONSTRAINT_IE
{
    UCHAR   ElementID;
    UCHAR   Length;
    UCHAR   LocalPowerConstraint;
} POWER_CONSTRAINT_IE, *PPOWER_CONSTRAINT_IE;

//
// The length of the power capability, excluding the element ID field and the length field
//
#define DOT11DH_POWER_CAPABILITY_LENGTH 2

//
// Power capability IE
//
typedef struct _POWER_CAPABILITY_IE
{
    UCHAR   ElementID;
    UCHAR   Length;
    CHAR    MinTxPowerCapability; // in units of dBm
    CHAR    MaxTxPowerCapability; // in units of dBm
} POWER_CAPABILITY_IE, *PPOWER_CAPABILITY_IE;

//
// The beacon frame or the probe response frame does not have the country IE.
//
#define MDSM_ABSENT_COUNTRY_IE                          0xFF

//
// The beacon frame or the probe response frame does not have the power constraint IE.
//
#define MDSM_ABSENT_POWER_CONSTRAINT_IE             0xFF

//
// The beacon frame or the probe response frame does not have the Cisco cell power information IE.
// Ref: The "S31: AP Control of Client Transmit Power" of the Cisco CCX specification, V2.13
//
#define MDSM_ABSENT_CCX_CELL_POWER_INFORMATION_IE   0xFF

//
// Forward declaration
//
typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;

//
// 802.11d Information Elements and related variables
//

//
// 802.11d+h IE code
//
#define IE_DOT11DH_COUNTRY                              7
#define IE_DOT11DH_HOPPING_PATTERN_PARAMETERS           8
#define IE_DOT11DH_HOPPING_PATTERN_TABLE                9
#define IE_DOT11DH_REQUEST                              10
#define IE_DOT11DH_POWER_CONSTRAINT                 32
#define IE_DOT11DH_POWER_CAPABILITY                     33
#define IE_DOT11DH_TPC_REQUEST                          34
#define IE_DOT11DH_TPC_REPORT                           35
#define IE_DOT11DH_SUPPORTED_CHANNELS                   36
#define IE_DOT11DH_CHANNEL_SWITCH_ANNOUNCEMENT      37
#define IE_DOT11DH_MEASUREMENT_REQUEST                  38
#define IE_DOT11DH_MEASUREMENT_REPORT                   39
#define IE_DOT11DH_QUIET                                    40
#define IE_DOT11DH_IBSS_DFS                             41

//
// The default maximum and minimum transmit power with which a STA is capable of transmitting in the current channel
//
// Note that the maximum transmit power capability is based on the CCK channel power, Tx0/Tx1=18.6 /18.3, in channel 1
//
#define MSDM_DEFAULT_MAX_TX_POWER_CAPABILITY        19 // in units of dBm
#define MSDM_DEFAULT_MIN_TX_POWER_CAPABILITY        5 // in units of dBm

//
// The EEPROM (0x4Eh~0x4Fh) does not have the maximum transmit power capability
//
#define MSDM_ABSENT_MAX_TX_POWER_CAPABILITY     0xFF

//
// The lower bound of the hardware's maximum transmit power capability, in units of dBm
//
#define MSDM_LOWER_BOUND_OF_MAX_TX_POWER_CAPABILITY 0

//
// The upper bound of the hardware's maximum transmit power capability, in units of dBm
//
#define MSDM_UPPER_BOUND_OF_MAX_TX_POWER_CAPABILITY 50

//
// The bit mask used for getting the hardware's maximum tranmsit power capability, in units of dBm
//
#define MSDM_MAX_TX_POWE_CAPABILITY_MASK            0x00FF

//
// The channel gap in the country IE
//
#define MSDM_2_4G_CHANNEL_GAP   1
#define MSDM_5G_CHANNEL_GAP 4

//
// The hardware's transmit power settings
//
// Note (without RF transmit power consideration): 
//  "real transmit power" = "MaxTxPowerCapability" - "DropTxPowerByMAC" - "DropTxPowerByBBP" 
//
typedef struct _MSDM_HW_TX_POWER_SETTING
{
    // The maximum transmit power with which the STA is capable of transmitting in the current channel, in units of dBm
    CHAR    MaxTxPowerCapability;

    // The minimum transmit power with which the STA is capable of transmitting in the current channel, in units of dBm
    CHAR    MinTxPowerCapability;

    // Real transmit power
    CHAR    RealTxPower;
} MSDM_HW_TX_POWER_SETTING, *PMSDM_HW_TX_POWER_SETTING;

//
// The multi domain and specturm management of the associated STA (AP or peer STA)
//
typedef struct _ASSOCIATED_STA_MSDM_CAPABILITY
{
    BOOLEAN bMDSMCapable;
    UCHAR   acceptableMaxTxPower; // The maximum transmit power, in dBm, allowed in the subband for the associated domain country string
} ASSOCIATED_STA_MSDM_CAPABILITY, *PASSOCIATED_STA_MSDM_CAPABILITY;

//
// Multi domain, spectrum and transmit power maintance
//
typedef struct _MULTI_DOMAIN_SPECTRUM_MANAGEMENT_CTRL
{
    BOOLEAN dot11MultiDomainCapabilityEnabled; // The capability of the station to operate in multiple regulatory domains is enabled or disabled
    BOOLEAN dot11SpectrumManagementRequired; // The defined TPC and DFS procedures are enabled or disabled.

    ASSOCIATED_STA_MSDM_CAPABILITY associatedStaMSDMCapable;

    MSDM_HW_TX_POWER_SETTING HwTxPowerSetting;

} MULTI_DOMAIN_SPECTRUM_MANAGEMENT_CTRL, *PMULTI_DOMAIN_SPECTRUM_MANAGEMENT_CTRL;

#pragma pack(pop, struct_pack1)

//
// Multi domain funtion prototypes
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
    IN BOOLEAN bEnableSpectrumManagement);

//
// Calaculate the maximum transmit power level
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
    IN UCHAR LocalPowerConstraint);

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
    IN OUT PPOWER_CAPABILITY_IE pPowerCapabilityIE);

//
// Apply the transmit power adjustment based on the STA's transmit power capability and the AP's/peer STA's capabilities, 
// such as the country IE, the power constraint IE and the maximum transmit power field in the link measurement request frame.
//
// Parameters
//  pAd: The adapter data structure
//  DeltaTxPower: The delta transmit power (non-positive number) for the specified transmit rate
//  BbpR1: BBP R1
//
// Return Value
//  Real transmit power, in units of dBm
//
CHAR MDSMApplyTxPowerAdjustment(
    IN PMP_ADAPTER pAd, 
    IN CHAR DeltaTxPower, 
    IN UCHAR BbpR1);

#endif // __DOT11DH_H__

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          End of File
////////////////////////////////////////////////////////////////////////////////////////////////

