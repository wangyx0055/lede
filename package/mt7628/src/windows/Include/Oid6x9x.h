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
    OID_RT6X9X.h

    Abstract: 
    RT6X9X features about OID(RT6590/RT6592/MT7601)

    Revision History:
    Who                     When            What
    ------------------- ----------      ----------------------------------------------
        Smith Hsu       2011/12/1           Start implementation
        Ian Tang        2012/2/6        Add HT/VHT functions
*/

#ifndef __OID6X9X_H__
#define __OID6X9X_H__

#include    "MtConfig.h"

typedef struct _CHANNEL_PARAMS {
    UCHAR   PrimaryChannel;
    UCHAR   CentralChannel;
    UCHAR   BandWidth;
} CHANNEL_PARAMS, *PCHANNEL_PARAMS;

static CHANNEL_PARAMS ValidVhtChannelTable[] =
{   
    //-PrimaryChannel---CentralChannel-----Bandwidth---

    // BW20
            { 36,               36,             BW_20},
            { 40,               40,             BW_20},
            { 44,               44,             BW_20},
            { 48,               48,             BW_20},
            { 52,               52,             BW_20},
            { 56,               56,             BW_20},
            { 60,               60,             BW_20},
            { 64,               64,             BW_20},
            {100,              100,             BW_20},
            {104,              104,             BW_20},
            {108,              108,             BW_20},
            {112,              112,             BW_20},
            {116,              116,             BW_20},
            {120,              120,             BW_20},
            {124,              124,             BW_20},
            {128,              128,             BW_20},
            {132,              132,             BW_20},
            {136,              136,             BW_20},
            {140,              140,             BW_20},
            {149,              149,             BW_20},
            {153,              153,             BW_20},
            {157,              157,             BW_20},
            {161,              161,             BW_20},
            {165,              165,             BW_20},
            {169,              165,             BW_20},
            {173,              165,             BW_20},
    // BW40       
            { 36,               38,             BW_40},
            { 40,               38,             BW_40},
            { 44,               46,             BW_40},
            { 48,               46,             BW_40},
            { 52,               54,             BW_40},
            { 56,               54,             BW_40},
            { 60,               62,             BW_40},
            { 64,               62,             BW_40},
            {100,              102,             BW_40},
            {104,              102,             BW_40},
            {108,              110,             BW_40},
            {112,              110,             BW_40},
            {116,              118,             BW_40},
            {120,              118,             BW_40},
            {124,              126,             BW_40},
            {128,              126,             BW_40},
            {132,              134,             BW_40},
            {136,              134,             BW_40},
            {149,              151,             BW_40},
            {153,              151,             BW_40},
            {157,              159,             BW_40},
            {161,              159,             BW_40},
            {169,              171,             BW_40},
            {173,              171,             BW_40},
    // BW80
            { 36,               42,             BW_80},
            { 40,               42,             BW_80},
            { 44,               42,             BW_80},
            { 48,               42,             BW_80},
            { 52,               58,             BW_80},
            { 56,               58,             BW_80},
            { 60,               58,             BW_80},
            { 64,               58,             BW_80},
            {100,              106,             BW_80},
            {104,              106,             BW_80},
            {108,              106,             BW_80},
            {112,              106,             BW_80},
            {116,              122,             BW_80},
            {120,              122,             BW_80},
            {124,              122,             BW_80},
            {128,              122,             BW_80},
            {149,              155,             BW_80},
            {153,              155,             BW_80},
            {157,              155,             BW_80},
            {161,              155,             BW_80}
};
#define NUM_OF_VALID_VHT_CHANNEL_TABLE   (sizeof(ValidVhtChannelTable) / sizeof(CHANNEL_PARAMS))

/*
 **********************************************************************************************************************
 * [Section]
 *
 * The Macro Definition
 **********************************************************************************************************************
*/ 
#define OID_BUF_QUERY_MAX_LENGTH  256    // The maxium length requirement of OID buffer




/*
 **********************************************************************************************************************
 * [Section]
 *
 * The Data structure Prototype for OID buffer
 **********************************************************************************************************************
*/

//
// Forward type definition
//
typedef struct _MP_PORT MP_PORT, *PMP_PORT;

//
// Report HW feature. example: VHT, HwWAPI etc.
//
typedef struct 
{
    //
    // Buffer Version History
    // ----------------------
    // BufVer=1, Member= ULONG BufVer; UniField ULONG VhtSupp:1; UniField ULONG Reserve:31;
    // BufVer=2, Member= ... 
    //
    ULONG       BufVer;         // Version of Capability Buffer
    union
    {
        struct
        {
            ULONG   VhtSupp:1;      // Whether Device support VHT
            ULONG   ChannelWidth:8; // same definition as "ChannelWidth" in VHT_Operation IE
                                    // 0: 20 or 40 Mhz
                                    // 1: 80 Mhz
                                    // 2: 160 Mhz
                                    // 3: 80+80 Mhz
                                    // 4~255: reserved

            ULONG   reserved:23; 
            // Capability2, add it in future
            // Capability3, add it in future
        } UniField;

        ULONG   UniValue;
    } HwCapaInfo;

} OID_BUF_HW_CAPABILITIES, *POID_BUF_HW_CAPABILITIES;

// Macro for OID_BUF_HW_CAPABILITIES, Buffer version
#define OID_BUF_HW_CAPA_BUFVER_1  1 
#define OID_BUF_HW_CAPA_BUFVER_2  2


//
// Tx Fixed Rate feature for UI
//
typedef struct 
{ 
    UCHAR   FixedRateEnabled;   // If true: driver use UI setting as below field. If false: driver ignore below setting.
    UCHAR   PhyMode;            // MODE_CCK, MODE_OFDM, MODE_HTMIX, MODE_HTGREENFIELD, MODE_VHT
    UCHAR   STBC;               // Enable or disable STBC
    UCHAR   SHORTGI;            // Enable or disable short GI
    UCHAR   BW;                 // Channel bandwidth
    UCHAR   MCS;                // MCS index, 0~32 in 11n, 0~9 in 11ac
    UCHAR   Nss;                // Number of Spatial Stream
} OID_BUF_FIXED_RATE, *POID_BUF_FIXED_RATE;




/*
 **********************************************************************************************************************
 * [Section]
 *
 * Public Function Prototype for OID procedure
 **********************************************************************************************************************
*/


//
// Query hardware version for different HW architecture, such as RF range and RF band selection
//
ULONG OidQueryHwVersion(
    IN PMP_ADAPTER pAd
    );

//
// Report HW feature. example: VHT, HwWAPI etc.
//
BOOLEAN OidQueryHwChipCapability(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pQueryBuffer,
    IN  ULONG           QueryBufferLength,
    OUT PULONG          pBytesWritten       // Return TRUE: The length of returned buffer; Return FALSE: Length of Buffer required.
    );


//
// Set Tx rate to Fixed rate for testing.
//
BOOLEAN OidSetTxFixedRate(
    IN  PMP_ADAPTER       pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PVOID               pSetBuffer,
    IN  ULONG               SetBufferLength,
    OUT PULONG              pBytesNeeded
    );

//
// Set HT/VHT configurations
//
VOID SetHtVht(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  OID_SET_HT_PHYMODE *pHTPhyMode);

#endif // __OID_RT6X9X_H__

//////////////////////////////////// End of File ////////////////////////////////////

