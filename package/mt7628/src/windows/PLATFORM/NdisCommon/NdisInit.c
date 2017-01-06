/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    Ndis6Init.c

    Abstract:
    Miniport main initialization routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#include    "MtConfig.h"

extern ULONG    G_MemoryAllocate_cnt;
extern ULONG    G_MemoryFree_cnt;
extern ULONG    G_MemoryAllocate_Len;
extern ULONG    G_MemoryFree_Len;

//
// Structure to first copy string values into
//
typedef struct      _LOCAL_STRINGS {
    NDIS_STRING     Ssid;               // SSID network nameFcfginit
    WCHAR           SsidBuf[64];
    NDIS_STRING     VendorDesc;         // SSID network name
    WCHAR           VendorDescBuf[128];
    NDIS_STRING     Key0;               // Key 0 in ascii format entered by user
    WCHAR           KeyBuf0[128];
    NDIS_STRING     Key1;               // Key 1 in ascii format entered by user
    WCHAR           KeyBuf1[128];
    NDIS_STRING     Key2;               // Key 2 in ascii format entered by user
    WCHAR           KeyBuf2[128];
    NDIS_STRING     Key3;               // Key 3 in ascii format entered by user
    WCHAR           KeyBuf3[128];
    NDIS_STRING     MAC;                // Local administration MAC Address
    WCHAR           LocalMacAddr[64];
    NDIS_STRING     WPSDevName;         // WPS Device Name
    WCHAR           WPSDevNameBuf[64];  
    NDIS_STRING     CountryCodeString;
    WCHAR           CountryCodeStringBuf[64];
    NDIS_STRING     BaldEagleSSID;
    WCHAR           BaldEagleSSIDBuf[64];
}   LOCAL_STRINGS, *PLOCAL_STRINGS;

LOCAL_STRINGS   LocalStrings;


RTMP_REG_ENTRY  NICRegTable[] = {
//  reg value name                      bRequired   Parameter Type          Offset in MP_ADAPTER                  Field size                          Default Value   Min         Max
    {NDIS_STRING_CONST("OpMode"),               0,  NdisParameterInteger,   REG_CFG_OFFSET(OpMode),                    REG_CFG_SIZE(OpMode),                      OPMODE_STA,  0,          1           },
    {NDIS_STRING_CONST("CountryRegion"),        0,  NdisParameterInteger,   REG_CFG_OFFSET(CountryRegion),       REG_CFG_SIZE(CountryRegion),         0,          REGION_MINIMUM_BG_BAND, REGION_MAXIMUM_BG_BAND},
    {NDIS_STRING_CONST("CountryRegionABand"),   0,  NdisParameterInteger,   REG_CFG_OFFSET(CountryRegionForABand),REG_CFG_SIZE(CountryRegionForABand),7,          REGION_MINIMUM_A_BAND, REGION_MAXIMUM_A_BAND},
    {NDIS_STRING_CONST("WirelessMode"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(PhyMode),             REG_CFG_SIZE(PhyMode),               5,          0,          11          },
    {NDIS_STRING_CONST("Channel"),              0,  NdisParameterInteger,   REG_CFG_OFFSET(Channel),             REG_CFG_SIZE(Channel),               1,          1,          255         },
    {NDIS_STRING_CONST("RtsThresh"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(RtsThreshold),        REG_CFG_SIZE(RtsThreshold),          2347,       0,          2347        },
    {NDIS_STRING_CONST("FragThresh"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(FragmentThreshold),   REG_CFG_SIZE(FragmentThreshold),     2346,       256,        2346        },
    //{NDIS_STRING_CONST("CipherMode"),         0,  NdisParameterInteger,   COMMON_CFG_OFFSET(WepStatus),           COMMON_CFG_SIZE(WepStatus),             0,          0,          7           },
    //{NDIS_STRING_CONST("AuthMode"),               0,  NdisParameterInteger,   COMMON_CFG_OFFSET(AuthMode),            COMMON_CFG_SIZE(AuthMode),              1,          1,          7           },
    //{NDIS_STRING_CONST("WEPKeyUse"),          0,  NdisParameterInteger,   COMMON_CFG_OFFSET(DefaultKeyId),        COMMON_CFG_SIZE(DefaultKeyId),          0,          1,          4           },
    {NDIS_STRING_CONST("RDG"),                  0,  NdisParameterInteger,   REG_CFG_OFFSET(bRdg),                REG_CFG_SIZE(bRdg),                  1,          0,          1           },
    {NDIS_STRING_CONST("SSID"),                 0,  NdisParameterString,    STRING_OFFSET(Ssid),                    STRING_SIZE(Ssid),                      0,          0,          1           },
    {NDIS_STRING_CONST("VendorDesc"),           0,  NdisParameterString,    STRING_OFFSET(VendorDesc),              STRING_SIZE(VendorDesc),                0,          0,          1           },
    {NDIS_STRING_CONST("WPSDevName"),           0,  NdisParameterString,    STRING_OFFSET(WPSDevName),              STRING_SIZE(WPSDevName),                0,          0,          1           },  
    {NDIS_STRING_CONST("WEPKey1"),              0,  NdisParameterString,    STRING_OFFSET(Key0),                    STRING_SIZE(Key0),                      0,          0,          1           },
    {NDIS_STRING_CONST("WEPKey2"),              0,  NdisParameterString,    STRING_OFFSET(Key1),                    STRING_SIZE(Key1),                      0,          0,          1           },
    {NDIS_STRING_CONST("WEPKey3"),              0,  NdisParameterString,    STRING_OFFSET(Key2),                    STRING_SIZE(Key2),                      0,          0,          1           },
    {NDIS_STRING_CONST("WEPKey4"),              0,  NdisParameterString,    STRING_OFFSET(Key3),                    STRING_SIZE(Key3),                      0,          0,          1           },
    {NDIS_STRING_CONST("NetworkAddress"),       0,  NdisParameterString,    STRING_OFFSET(MAC),                     STRING_SIZE(MAC),                       0,          0,          1           },

    {NDIS_STRING_CONST("RBFactor"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(RxBulkFactor),     REG_CFG_SIZE(RxBulkFactor),     0,          0,          2           },
    {NDIS_STRING_CONST("TBFactor"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(TxBulkFactor),     REG_CFG_SIZE(TxBulkFactor),     0,          0,          2           },
    {NDIS_STRING_CONST("MaxPktOneTxBulk"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(MaxPktOneTxBulk),     REG_CFG_SIZE(MaxPktOneTxBulk),       0,          0,          2           },
    {NDIS_STRING_CONST("BGProtection"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(UseBGProtection),     REG_CFG_SIZE(UseBGProtection),       0,          0,          2           },
    {NDIS_STRING_CONST("ShortSlot"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(bUseShortSlotTime),   REG_CFG_SIZE(bUseShortSlotTime),     0,          0,          1           },
//
// TODO: if we can parse aggregation frame, then enable this function
//
    {NDIS_STRING_CONST("FrameAggregation"),     0,  NdisParameterInteger,   REG_CFG_OFFSET(bAggregationCapable), REG_CFG_SIZE(bAggregationCapable),   0,          0,          1           },
    {NDIS_STRING_CONST("IEEE80211H"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(bIEEE80211H),         REG_CFG_SIZE(bIEEE80211H),           0,          0,          1           },  
    {NDIS_STRING_CONST("CarrierDetect"),        0,  NdisParameterInteger,   REG_CFG_OFFSET(bCarrierDetect),      REG_CFG_SIZE(bCarrierDetect),        0,          0,          1           },
    {NDIS_STRING_CONST("TXBurst"),              0,  NdisParameterInteger,   REG_CFG_OFFSET(bEnableTxBurst),      REG_CFG_SIZE(bEnableTxBurst),        0,          0,          1           },
    {NDIS_STRING_CONST("WMMCapable"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(bWmmCapable),         REG_CFG_SIZE(bWmmCapable),           0,          0,          1           },
    {NDIS_STRING_CONST("APSDCapable"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(bAPSDCapable),        REG_CFG_SIZE(bAPSDCapable),          0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_BE"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(bAPSDAC_BE),          REG_CFG_SIZE(bAPSDAC_BE),            0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_BK"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(bAPSDAC_BK),          REG_CFG_SIZE(bAPSDAC_BK),            0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_VI"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(bAPSDAC_VI),          REG_CFG_SIZE(bAPSDAC_VI),            0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_VO"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(bAPSDAC_VO),          REG_CFG_SIZE(bAPSDAC_VO),            0,          0,          1           },
    {NDIS_STRING_CONST("MultipleIRP"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(bMultipleIRP),        REG_CFG_SIZE(bMultipleIRP),          1,          0,          1           },
    {NDIS_STRING_CONST("BACapability"),                 0,  NdisParameterHexInteger ,   REG_CFG_OFFSET(BACapability),                REG_CFG_SIZE(BACapability),                  0x07034020,         2,          0xffffffff          },
    {NDIS_STRING_CONST("TransmitSetting"),              0,  NdisParameterInteger,   REG_CFG_OFFSET(RegTransmitSetting),              REG_CFG_SIZE(RegTransmitSetting),                    0x36a15,            2,          0xffffffff          },
    {NDIS_STRING_CONST("DLSCapable"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(bDLSCapable),         REG_CFG_SIZE(bDLSCapable),           0,          0,          1           },
    {NDIS_STRING_CONST("DirectMode"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(DirectMode),          REG_CFG_SIZE(DirectMode),            0,          0,          0xffffffff},

    {NDIS_STRING_CONST("PSPXlink"),             0,  NdisParameterInteger,   REG_CFG_OFFSET(PSPXlink),            REG_CFG_SIZE(PSPXlink),              0,          0,          1           },
    {NDIS_STRING_CONST("WifiDirectControl"),    0,  NdisParameterInteger,   REG_CFG_OFFSET(P2pControl),          REG_CFG_SIZE(P2pControl),            0x3b8b78,   0,          0xffffffff  },

    {NDIS_STRING_CONST("Radio"),                0,  NdisParameterInteger,   REG_CFG_OFFSET(bSwRadioOff),            REG_CFG_SIZE(bSwRadioOff),              0,          0,          1           },

    {NDIS_STRING_CONST("AutoReconnect"),        0,  NdisParameterInteger,   REG_CFG_OFFSET(bAutoReconnect),         REG_CFG_SIZE(bAutoReconnect),           1,          0,          1           },
    {NDIS_STRING_CONST("NetworkType"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(BssType),                REG_CFG_SIZE(BssType),                  1,          1,          2           },
    {NDIS_STRING_CONST("AdhocOfdm"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(AdhocMode),              REG_CFG_SIZE(AdhocMode),                1,          0,          10          },
    {NDIS_STRING_CONST("AdhocNMode"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(bAdhocNMode),                REG_CFG_SIZE(bAdhocNMode),              0,          0,          1           },  // AdhocN Support
    {NDIS_STRING_CONST("AdhocNForceMulticastRate"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(AdhocNForceMulticastRate),               REG_CFG_SIZE(AdhocNForceMulticastRate),                 0x00100000,         0,          0xffffffff          },  // AdhocN Support   
    {NDIS_STRING_CONST("Adhoc5GDisable"),       0,  NdisParameterInteger,   REG_CFG_OFFSET(bAdhoc5GDisable),        REG_CFG_SIZE(bAdhoc5GDisable),          0,          0,          1           },
    {NDIS_STRING_CONST("AdhocN40MHzAllowed"),   0,  NdisParameterInteger,   REG_CFG_OFFSET(bAdhocN40MHzAllowed),    REG_CFG_SIZE(bAdhocN40MHzAllowed),      0,          0,          1           },  // AdhocN Support   
    {NDIS_STRING_CONST("AdhocN14Ch40MHzAllowed"), 0,NdisParameterInteger,   REG_CFG_OFFSET(bAdhocN14Ch40MHzAllowed),REG_CFG_SIZE(bAdhocN14Ch40MHzAllowed),  0,          0,          1           },  // AdhocN Support 

    {NDIS_STRING_CONST("PSMode"),               0,  NdisParameterInteger,   REG_CFG_OFFSET(WindowsPowerMode),       REG_CFG_SIZE(WindowsPowerMode),         0,          0,          2           },

    {NDIS_STRING_CONST("FastRoamingSensitivity"), 0,NdisParameterInteger,   REG_CFG_OFFSET(FastRoamingSensitivity), REG_CFG_SIZE(FastRoamingSensitivity),   0,          0,          5           },

    {NDIS_STRING_CONST("AutoChannelSelect"),    0,  NdisParameterInteger,   REG_CFG_OFFSET(bAutoChannelAtBootup),    REG_CFG_SIZE(bAutoChannelAtBootup),      0,          0,          1           },
    {NDIS_STRING_CONST("DtimPeriod"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(DtimPeriod),              REG_CFG_SIZE(DtimPeriod),                1,          1,          5           },
    {NDIS_STRING_CONST("IdleTime"),             0,  NdisParameterInteger,   REG_CFG_OFFSET(AgeoutTime),              REG_CFG_SIZE(AgeoutTime),                300,       60,          3600        },
    {NDIS_STRING_CONST("RssiDelta"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(RssiDelta),              REG_CFG_SIZE(RssiDelta),                6,          0,          20          },
    {NDIS_STRING_CONST("SmartScan"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(SmartScan),              REG_CFG_SIZE(SmartScan),                0,          0,          1           },
    {NDIS_STRING_CONST("PSControl"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(PSControl),              REG_CFG_SIZE(PSControl),                0,          0,          0xffffffff  },
    {NDIS_STRING_CONST("KeepReconnectCnt"),     0,  NdisParameterInteger,   REG_CFG_OFFSET(KeepReconnectCnt),       REG_CFG_SIZE(KeepReconnectCnt),         3,          1,          30          },
    {NDIS_STRING_CONST("SoftAPForceMulticastRate"),         0,  NdisParameterHexInteger,    REG_CFG_OFFSET(SoftAPForceMulticastRate),            REG_CFG_SIZE(SoftAPForceMulticastRate),              0x00000000,         0,          0xffffffff          },  // SoftAPForceMulticastRate

    //LED
    {NDIS_STRING_CONST("LEDCtlBySW"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(LedControlBySW),            REG_CFG_SIZE(LedControlBySW),              0,          0,          1           },
    {NDIS_STRING_CONST("LEDMode"),              0,  NdisParameterInteger,   REG_CFG_OFFSET(INFLedCntl),                REG_CFG_SIZE(INFLedCntl),                  0,          0,          0xff        },
    {NDIS_STRING_CONST("LEDConfig"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(INFLed1),                   REG_CFG_SIZE(INFLed1),                     0,          0,          0xffff      },
    {NDIS_STRING_CONST("LEDAct"),               0,  NdisParameterInteger,   REG_CFG_OFFSET(INFLed2),                   REG_CFG_SIZE(INFLed2),                     0,          0,          0xffff      },
    {NDIS_STRING_CONST("LEDPolarity"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(INFLed3),                   REG_CFG_SIZE(INFLed3),                     0,          0,          0xffff      },
    {NDIS_STRING_CONST("TEST"),                 0,  NdisParameterInteger,   REG_CFG_OFFSET(WhqlTest),               REG_CFG_SIZE(WhqlTest),                 0,          0,          1           },      

    {NDIS_STRING_CONST("CountryTxPwr"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(CountryTxPwr),        REG_CFG_SIZE(CountryTxPwr),          0xffffffff, 0,          0xffffffff  },
    {NDIS_STRING_CONST("CountryTxPwr5G"),       0,  NdisParameterInteger,   REG_CFG_OFFSET(CountryTxPwr5G),      REG_CFG_SIZE(CountryTxPwr5G),        0xffffffff, 0,          0xffffffff  },
    {NDIS_STRING_CONST("LinkSpeedStatus"),      0,  NdisParameterInteger,   REG_CFG_OFFSET(FixLinkSpeedStatus), REG_CFG_SIZE(FixLinkSpeedStatus),       0,          0,          2           },
    //WAPI
    {NDIS_STRING_CONST("WAPI"),               0,  NdisParameterInteger,   REG_CFG_OFFSET(WAPI),                    REG_CFG_SIZE(WAPI),                      1,  0,          0xffffffff           },   

    //MFP
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    {NDIS_STRING_CONST("MFPC"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(PmfControl.MFP_Enable),      REG_CFG_SIZE(PmfControl.MFP_Enable),        0,          0,      1},
    {NDIS_STRING_CONST("MFPR"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(PmfControl.MFP_Required),        REG_CFG_SIZE(PmfControl.MFP_Required),        0,          0,      1},
#else
    {NDIS_STRING_CONST("MFPC"),                 0,  NdisParameterInteger,   REG_CFG_OFFSET(PmfControl.MFP_Enable),      REG_CFG_SIZE(PmfControl.MFP_Enable),        0,          0,          1           },
    {NDIS_STRING_CONST("MFPR"),                 0,  NdisParameterInteger,   REG_CFG_OFFSET(PmfControl.MFP_Required),    REG_CFG_SIZE(PmfControl.MFP_Required),      0,          0,          1           },
#endif

    // Reposit Bssid List
    {NDIS_STRING_CONST("BssidRepositTime"), 0,  NdisParameterInteger,   REG_CFG_OFFSET(BssidRepositTime),    REG_CFG_SIZE(BssidRepositTime),  DEFAULT_REPOSIT_TIME,   MIN_REPOSIT_TIME,   MAX_REPOSIT_TIME    },
    {NDIS_STRING_CONST("BeaconLostTime"),       0,  NdisParameterInteger,   REG_CFG_OFFSET(BeaconLostTime),         REG_CFG_SIZE(BeaconLostTime),           3,          3,          8           },
    //HT is disabled on 11n when using TKIP.
    {NDIS_STRING_CONST("ProhibitTKIPonHT"),     0,  NdisParameterInteger,   REG_CFG_OFFSET(bProhibitTKIPonHT),   REG_CFG_SIZE(bProhibitTKIPonHT), 0,  0,  1},

    // Link Quality Setting Control
    {NDIS_STRING_CONST("LinkQualitySetting"),   0,  NdisParameterInteger,   REG_CFG_OFFSET(LinkQualitySetting),         REG_CFG_SIZE(LinkQualitySetting),0x0,   0x0,    0xff},

    // Channel 14 support 11b only
    {NDIS_STRING_CONST("Ch14BOnly"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(Ch14BOnly),           REG_CFG_SIZE(Ch14BOnly),             1,          0,          1           },

    {NDIS_STRING_CONST("PALRegulatoryClass"),       0,  NdisParameterInteger,   REG_CFG_OFFSET(PALRegulatoryClass),      REG_CFG_SIZE(PALRegulatoryClass),        254,        0,          0xff},
    {NDIS_STRING_CONST("PALCapability"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(PALCapability),           REG_CFG_SIZE(PALCapability),             0,      0,          0xffffffff},
    {NDIS_STRING_CONST("PALVersionSpecifier"),      0,  NdisParameterInteger,   REG_CFG_OFFSET(PALVersionSpecifier),     REG_CFG_SIZE(PALVersionSpecifier),       0,      0,          0xff},
    {NDIS_STRING_CONST("PALCompany_ID"),        0,  NdisParameterInteger,   REG_CFG_OFFSET(PALCompany_ID),       REG_CFG_SIZE(PALCompany_ID),         0,      0,          0xffff},
    {NDIS_STRING_CONST("PALSubVersion"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(PALSubVersion),           REG_CFG_SIZE(PALSubVersion),         0,      0,          0xffff},

    // GPIO pin-sharing mode
    {NDIS_STRING_CONST("GpioPinSharingMode"),   0,  NdisParameterInteger,   REG_CFG_OFFSET(GpioPinSharingMode),  REG_CFG_SIZE(GpioPinSharingMode),    GpioPinSharingMode_DefaultMode,     GpioPinSharingMode_LowerBound,  GpioPinSharingMode_UpperBound},

    // Blad Eagle function
    {NDIS_STRING_CONST("BaldEagleSSID"),       0,   NdisParameterString,    STRING_OFFSET(BaldEagleSSID),      STRING_SIZE(BaldEagleSSID),              0,                      0,                  1                },

    //Led config
    {NDIS_STRING_CONST("LedCfg"),       0,  NdisParameterInteger,   REG_CFG_OFFSET(LedCfg),      REG_CFG_SIZE(LedCfg),        0,  0,          1   },
    
    {NDIS_STRING_CONST("Feature11n"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(Feature11n),         REG_CFG_SIZE(Feature11n),               1,          0,          0xffffffff          },

    // Can disable bEnableFrequencyCalibration
    {NDIS_STRING_CONST("AdaptiveFreq"),             0,  NdisParameterInteger,   REG_CFG_OFFSET(AdaptiveFreq),            REG_CFG_SIZE(AdaptiveFreq),              1,          0,          1           },

    // Set tx power level
    {NDIS_STRING_CONST("TxPowerDefault"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(TxPowerDefault),          REG_CFG_SIZE(TxPowerDefault),                100,            0,          100         },

    // For 11n Tx Burst timing test.
    {NDIS_STRING_CONST("11nTxBurstTimeTest"),               0,  NdisParameterInteger,   REG_CFG_OFFSET(b11nTxBurstTimeTest),     REG_CFG_SIZE(b11nTxBurstTimeTest),       0,          0,          1           },

    // For FCC Power Test
    {NDIS_STRING_CONST("AdTxPwrForFCCTest"),                0,  NdisParameterInteger,   REG_CFG_OFFSET(bAdTxPwrForFCCTest),      REG_CFG_SIZE(bAdTxPwrForFCCTest),        0,          0,          1           },

    
    // Adjust Tx power mode For Customer , 0: no use, 1: For Asus
    {NDIS_STRING_CONST("CustomizedPowerMode"),              0,  NdisParameterInteger,   REG_CFG_OFFSET(CustomizedPowerMode),     REG_CFG_SIZE(CustomizedPowerMode),       0,          0,          0xFF            },
    {NDIS_STRING_CONST("MaxTxPower"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(MaxTxPower),          REG_CFG_SIZE(MaxTxPower),                0,          0,          0xFF            },
    {NDIS_STRING_CONST("CountryTargetTxPowerMode"),             0,  NdisParameterInteger,   REG_CFG_OFFSET(CountryTargetTxPowerMode),        REG_CFG_SIZE(CountryTargetTxPowerMode),      0,          0,          0xFF            },

    // Can Enable/Disable Stream mode
    {NDIS_STRING_CONST("StreamMode"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(StreamMode),          REG_CFG_SIZE(StreamMode),                0,          0,          1           },

    // Registry control to Enable/Disable function of ChannelSwitchOffloadToFW
    {NDIS_STRING_CONST("ChannelSwitchOffloadToFW"),             0,  NdisParameterInteger,   REG_CFG_OFFSET(bChannelSwitchOffloadToFW),           REG_CFG_SIZE(bChannelSwitchOffloadToFW),             1,          0,          1           },

    // bit 0~3 is value and bit 4~7 is id for BBP setting
    {NDIS_STRING_CONST("BBPIDV"),           0,  NdisParameterInteger,   REG_CFG_OFFSET(BBPIDV),          REG_CFG_SIZE(BBPIDV),               0,           0,          0xffffffff          },
    // Check Security when setting WEP to connect AP
    {NDIS_STRING_CONST("WepConnectCheckEncry"),             0,  NdisParameterInteger,   REG_CFG_OFFSET(WepConnectCheckEncry),            REG_CFG_SIZE(WepConnectCheckEncry),             0,           0,          1           },

    {NDIS_STRING_CONST("FeatureSetting"),       0,  NdisParameterInteger,   REG_CFG_OFFSET(FeatureSetting),      REG_CFG_SIZE(FeatureSetting),        0,  0,          0xffffffff},

    // TxBF
    {NDIS_STRING_CONST("ETxBfEn"),                      0,  NdisParameterInteger,   REG_CFG_OFFSET(ETxBfEn),                 REG_CFG_SIZE(ETxBfEn),                       0,  0,  1}, 
    {NDIS_STRING_CONST("ETxBfTimeout"),                 0,  NdisParameterInteger,   REG_CFG_OFFSET(ETxBfTimeout),                REG_CFG_SIZE(ETxBfTimeout),                  0,  0,  65535}, 
    {NDIS_STRING_CONST("ETxBfNonCompressedFeedback"),   0,  NdisParameterInteger,   REG_CFG_OFFSET(ETxBfNonCompressedFeedback),  REG_CFG_SIZE(ETxBfNonCompressedFeedback),    0,  0,  1}, 

    //P2P Reinforced ACK check
    {NDIS_STRING_CONST("P2PAckCheck"),  0,  NdisParameterInteger,   REG_CFG_OFFSET(bP2pAckCheck),    REG_CFG_SIZE(bP2pAckCheck),      1,          0,          1           },  
    // P2P Video Boost switch
    {NDIS_STRING_CONST("P2PVideoBoost"),    0,  NdisParameterInteger,   REG_CFG_OFFSET(P2PVideoBoost),   REG_CFG_SIZE(P2PVideoBoost), 0,  0,  1},

    //
    // Tx/Rx configuration
    //
    {NDIS_STRING_CONST("TxRxConfigByRegistry"), 0,  NdisParameterInteger,   REG_CFG_OFFSET(TxRxConfigByRegistry),    REG_CFG_SIZE(TxRxConfigByRegistry),  TX_RX_CONFIG_BY_REGISTRY_DISABLE,   TX_RX_CONFIG_BY_REGISTRY_DISABLE,   TX_RX_CONFIG_BY_REGISTRY_MAX},
    //WSC 2.0
    {NDIS_STRING_CONST("WSCVersion2"),       0, NdisParameterInteger,   REG_CFG_OFFSET(WSCVersion2),      REG_CFG_SIZE(WSCVersion2),    0x20,   0x0,    0x20},  
    {NDIS_STRING_CONST("WSC20Testbed"),       0,    NdisParameterInteger,   REG_CFG_OFFSET(WSC20Testbed),      REG_CFG_SIZE(WSC20Testbed),  0x08,   0x0,    0xffffffff},    
    {NDIS_STRING_CONST("WscHwPBCMode"),     0,  NdisParameterInteger,   REG_CFG_OFFSET(WscHwPBCMode),       REG_CFG_SIZE(WscHwPBCMode),   0,  0,  1},

    //Multicast policy: 0-> defalult(enable "bypass RA packet") 1->disable "bypass RA packet" 2->will not clear up accepted multicast poll
    {NDIS_STRING_CONST("MPolicy"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(MPolicy),         REG_CFG_SIZE(MPolicy),               0,      0,          0xffffffff},

    {NDIS_STRING_CONST("SuppressScan"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(OzmoDeviceSuppressScan),             REG_CFG_SIZE(OzmoDeviceSuppressScan),               0,          0,          0xffffffff          },

    //
    // PPAD (per-packet antenna diversity)
    //
    {NDIS_STRING_CONST("EnablePPAD"),   0,  NdisParameterInteger,   REG_CFG_OFFSET(bPPAD),   REG_CFG_SIZE(bPPAD), 1,  0,  1}, 

    //
    // Calibration control registry
    //
    {NDIS_STRING_CONST("CalCtrl"),  0,  NdisParameterInteger,   REG_CFG_OFFSET(CalCtrlRegistry), REG_CFG_SIZE(CalCtrlRegistry),   0x00000001, 0,  0xFFFFFFFF}, 

    // Check if we are running WHQL testing or not.
    {NDIS_STRING_CONST("OnTestingWHQL"),            0,  NdisParameterInteger,   REG_CFG_OFFSET(OnTestingWHQL),            REG_CFG_SIZE(OnTestingWHQL),              0,          0,          1           },
    {NDIS_STRING_CONST("OnTestingCPUUtil"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(OnTestingCPUUtil),            REG_CFG_SIZE(OnTestingCPUUtil),              0,          0,          1           },
    {NDIS_STRING_CONST("WHQLScanTime"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(WHQLScanTime),            REG_CFG_SIZE(WHQLScanTime),              60,          10,          140           },

    // Win8 In-Box Mode
    {NDIS_STRING_CONST("ChannelMode"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(ChannelMode),            REG_CFG_SIZE(ChannelMode),              0,          0,          2           },
    {NDIS_STRING_CONST("InBoxMode"),    0,  NdisParameterInteger,   REG_CFG_OFFSET(InBoxMode),           REG_CFG_SIZE(InBoxMode), 0,  0,  1},

    //
    // Bandedge power table enable
    //
    {NDIS_STRING_CONST("BandedgePowerTableEnable"), 0, NdisParameterInteger, REG_CFG_OFFSET(BandedgePowerTableEnable), REG_CFG_SIZE(BandedgePowerTableEnable), 1, 0, 1},

    //
    // Switch Channel Offload Enable
    //
    {NDIS_STRING_CONST("SwitchChannelOffloadEnable"), 0, NdisParameterInteger, REG_CFG_OFFSET(SwitchChannelOffloadEnable), REG_CFG_SIZE(SwitchChannelOffloadEnable), 1, 0, 1},

    //
    // Full Calibration by FW with one cmd toggle
    //
    {NDIS_STRING_CONST("FullCalibrationOffloadEnable"), 0, NdisParameterInteger, REG_CFG_OFFSET(FullCalibrationOffloadEnable), REG_CFG_SIZE(FullCalibrationOffloadEnable), 0, 0, 1},  

    //
    // Thermal solution Enable
    //
    {NDIS_STRING_CONST("ThermalEnable"), 0, NdisParameterInteger, REG_CFG_OFFSET(ThermalEnable), REG_CFG_SIZE(ThermalEnable), 0, 0, 1},

    //
    // Unify Mac - FPGA
    //
    // 1. Endpint configuration
    // 2. Tx 1 EP to m Queues
    // 3. Tx m EPs to 1 Queue
    // 4. Tx m EPs to m Queues
    // 5. 1 Set WMM
    // 6. 2 Sets WMM
    //{NDIS_STRING_CONST("ThermalEnable"), 0, NdisParameterInteger, COMMON_CFG_OFFSET(ThermalEnable), COMMON_CFG_SIZE(ThermalEnable), 0, 0, 1},
};



#if 0
RTMP_REG_ENTRY  NICRegTable[] = {
//  reg value name                      bRequired   Parameter Type          Offset in MP_ADAPTER                  Field size                          Default Value   Min         Max
    {NDIS_STRING_CONST("OpMode"),               0,  NdisParameterInteger,   RTMP_OFFSET(OpMode),                    RTMP_SIZE(OpMode),                      OPMODE_STA,  0,          1           },
    {NDIS_STRING_CONST("CountryRegion"),        0,  NdisParameterInteger,   REG_CFG_OFFSET(CountryRegion),       REG_CFG_SIZE(CountryRegion),         0,          REGION_MINIMUM_BG_BAND, REGION_MAXIMUM_BG_BAND},
    {NDIS_STRING_CONST("CountryRegionABand"),   0,  NdisParameterInteger,   REG_CFG_OFFSET(CountryRegionForABand),REG_CFG_SIZE(CountryRegionForABand),7,          REGION_MINIMUM_A_BAND, REGION_MAXIMUM_A_BAND},
    {NDIS_STRING_CONST("WirelessMode"),         0,  NdisParameterInteger,   REG_CFG_OFFSET(PhyMode),             REG_CFG_SIZE(PhyMode),               5,          0,          11          },
    {NDIS_STRING_CONST("Channel"),              0,  NdisParameterInteger,   COMMON_CFG_OFFSET(Channel),             COMMON_CFG_SIZE(Channel),               1,          1,          255         },
    {NDIS_STRING_CONST("RtsThresh"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(RtsThreshold),        COMMON_CFG_SIZE(RtsThreshold),          2347,       0,          2347        },
    {NDIS_STRING_CONST("FragThresh"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(FragmentThreshold),   COMMON_CFG_SIZE(FragmentThreshold),     2346,       256,        2346        },
    //{NDIS_STRING_CONST("CipherMode"),         0,  NdisParameterInteger,   COMMON_CFG_OFFSET(WepStatus),           COMMON_CFG_SIZE(WepStatus),             0,          0,          7           },
    //{NDIS_STRING_CONST("AuthMode"),               0,  NdisParameterInteger,   COMMON_CFG_OFFSET(AuthMode),            COMMON_CFG_SIZE(AuthMode),              1,          1,          7           },
    //{NDIS_STRING_CONST("WEPKeyUse"),          0,  NdisParameterInteger,   COMMON_CFG_OFFSET(DefaultKeyId),        COMMON_CFG_SIZE(DefaultKeyId),          0,          1,          4           },
    {NDIS_STRING_CONST("RDG"),                  0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bRdg),                COMMON_CFG_SIZE(bRdg),                  1,          0,          1           },
    {NDIS_STRING_CONST("SSID"),                 0,  NdisParameterString,    STRING_OFFSET(Ssid),                    STRING_SIZE(Ssid),                      0,          0,          1           },
    {NDIS_STRING_CONST("VendorDesc"),           0,  NdisParameterString,    STRING_OFFSET(VendorDesc),              STRING_SIZE(VendorDesc),                0,          0,          1           },
    {NDIS_STRING_CONST("WPSDevName"),           0,  NdisParameterString,    STRING_OFFSET(WPSDevName),              STRING_SIZE(WPSDevName),                0,          0,          1           },  
    {NDIS_STRING_CONST("WEPKey1"),              0,  NdisParameterString,    STRING_OFFSET(Key0),                    STRING_SIZE(Key0),                      0,          0,          1           },
    {NDIS_STRING_CONST("WEPKey2"),              0,  NdisParameterString,    STRING_OFFSET(Key1),                    STRING_SIZE(Key1),                      0,          0,          1           },
    {NDIS_STRING_CONST("WEPKey3"),              0,  NdisParameterString,    STRING_OFFSET(Key2),                    STRING_SIZE(Key2),                      0,          0,          1           },
    {NDIS_STRING_CONST("WEPKey4"),              0,  NdisParameterString,    STRING_OFFSET(Key3),                    STRING_SIZE(Key3),                      0,          0,          1           },
    {NDIS_STRING_CONST("NetworkAddress"),       0,  NdisParameterString,    STRING_OFFSET(MAC),                     STRING_SIZE(MAC),                       0,          0,          1           },

    {NDIS_STRING_CONST("RBFactor"),         0,  NdisParameterInteger,   COMMON_CFG_OFFSET(RxBulkFactor),     COMMON_CFG_SIZE(RxBulkFactor),     0,          0,          2           },
    {NDIS_STRING_CONST("TBFactor"),         0,  NdisParameterInteger,   COMMON_CFG_OFFSET(TxBulkFactor),     COMMON_CFG_SIZE(TxBulkFactor),     0,          0,          2           },
    {NDIS_STRING_CONST("MaxPktOneTxBulk"),          0,  NdisParameterInteger,   COMMON_CFG_OFFSET(MaxPktOneTxBulk),     COMMON_CFG_SIZE(MaxPktOneTxBulk),       0,          0,          2           },
    {NDIS_STRING_CONST("BGProtection"),         0,  NdisParameterInteger,   COMMON_CFG_OFFSET(UseBGProtection),     COMMON_CFG_SIZE(UseBGProtection),       0,          0,          2           },
    {NDIS_STRING_CONST("ShortSlot"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bUseShortSlotTime),   COMMON_CFG_SIZE(bUseShortSlotTime),     0,          0,          1           },
//
// TODO: if we can parse aggregation frame, then enable this function
//
    {NDIS_STRING_CONST("FrameAggregation"),     0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bAggregationCapable), COMMON_CFG_SIZE(bAggregationCapable),   0,          0,          1           },
    {NDIS_STRING_CONST("IEEE80211H"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bIEEE80211H),         COMMON_CFG_SIZE(bIEEE80211H),           0,          0,          1           },  
    {NDIS_STRING_CONST("CarrierDetect"),        0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bCarrierDetect),      COMMON_CFG_SIZE(bCarrierDetect),        0,          0,          1           },
    {NDIS_STRING_CONST("TXBurst"),              0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bEnableTxBurst),      COMMON_CFG_SIZE(bEnableTxBurst),        0,          0,          1           },
    {NDIS_STRING_CONST("WMMCapable"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bWmmCapable),         COMMON_CFG_SIZE(bWmmCapable),           0,          0,          1           },
    {NDIS_STRING_CONST("APSDCapable"),          0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bAPSDCapable),        COMMON_CFG_SIZE(bAPSDCapable),          0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_BE"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bAPSDAC_BE),          COMMON_CFG_SIZE(bAPSDAC_BE),            0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_BK"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bAPSDAC_BK),          COMMON_CFG_SIZE(bAPSDAC_BK),            0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_VI"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bAPSDAC_VI),          COMMON_CFG_SIZE(bAPSDAC_VI),            0,          0,          1           },
    {NDIS_STRING_CONST("APSDAC_VO"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bAPSDAC_VO),          COMMON_CFG_SIZE(bAPSDAC_VO),            0,          0,          1           },
    {NDIS_STRING_CONST("MultipleIRP"),          0,  NdisParameterInteger,   REG_CFG_OFFSET(bMultipleIRP),        REG_CFG_SIZE(bMultipleIRP),          1,          0,          1           },
    {NDIS_STRING_CONST("BACapability"),                 0,  NdisParameterHexInteger ,   REG_CFG_OFFSET(BACapability),                REG_CFG_SIZE(BACapability),                  0x07034020,         2,          0xffffffff          },
    {NDIS_STRING_CONST("TransmitSetting"),              0,  NdisParameterInteger,   REG_CFG_OFFSET(RegTransmitSetting),              REG_CFG_SIZE(RegTransmitSetting),                    0x36a15,            2,          0xffffffff          },
    {NDIS_STRING_CONST("DLSCapable"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bDLSCapable),         COMMON_CFG_SIZE(bDLSCapable),           0,          0,          1           },
    {NDIS_STRING_CONST("DirectMode"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(DirectMode),          COMMON_CFG_SIZE(DirectMode),            0,          0,          0xffffffff},

    {NDIS_STRING_CONST("PSPXlink"),             0,  NdisParameterInteger,   COMMON_CFG_OFFSET(PSPXlink),            COMMON_CFG_SIZE(PSPXlink),              0,          0,          1           },
    {NDIS_STRING_CONST("WifiDirectControl"),    0,  NdisParameterInteger,   COMMON_CFG_OFFSET(P2pControl),          COMMON_CFG_SIZE(P2pControl),            0x3b8b78,   0,          0xffffffff  },

    {NDIS_STRING_CONST("Radio"),                0,  NdisParameterInteger,   STA_CFG_OFFSET(bSwRadioOff),            STA_CFG_SIZE(bSwRadioOff),              0,          0,          1           },

    {NDIS_STRING_CONST("AutoReconnect"),        0,  NdisParameterInteger,   STA_CFG_OFFSET(bAutoReconnect),         STA_CFG_SIZE(bAutoReconnect),           1,          0,          1           },
    {NDIS_STRING_CONST("NetworkType"),          0,  NdisParameterInteger,   STA_CFG_OFFSET(BssType),                STA_CFG_SIZE(BssType),                  1,          1,          2           },
    {NDIS_STRING_CONST("AdhocOfdm"),            0,  NdisParameterInteger,   STA_CFG_OFFSET(AdhocMode),              STA_CFG_SIZE(AdhocMode),                1,          0,          10          },
    {NDIS_STRING_CONST("AdhocNMode"),           0,  NdisParameterInteger,   STA_CFG_OFFSET(bAdhocNMode),                STA_CFG_SIZE(bAdhocNMode),              0,          0,          1           },  // AdhocN Support
    {NDIS_STRING_CONST("AdhocNForceMulticastRate"),         0,  NdisParameterInteger,   STA_CFG_OFFSET(AdhocNForceMulticastRate),               STA_CFG_SIZE(AdhocNForceMulticastRate),                 0x00100000,         0,          0xffffffff          },  // AdhocN Support   
    {NDIS_STRING_CONST("Adhoc5GDisable"),       0,  NdisParameterInteger,   STA_CFG_OFFSET(bAdhoc5GDisable),        STA_CFG_SIZE(bAdhoc5GDisable),          0,          0,          1           },
    {NDIS_STRING_CONST("AdhocN40MHzAllowed"),   0,  NdisParameterInteger,   STA_CFG_OFFSET(bAdhocN40MHzAllowed),    STA_CFG_SIZE(bAdhocN40MHzAllowed),      0,          0,          1           },  // AdhocN Support   
    {NDIS_STRING_CONST("AdhocN14Ch40MHzAllowed"), 0,NdisParameterInteger,   STA_CFG_OFFSET(bAdhocN14Ch40MHzAllowed),STA_CFG_SIZE(bAdhocN14Ch40MHzAllowed),  0,          0,          1           },  // AdhocN Support 

    {NDIS_STRING_CONST("PSMode"),               0,  NdisParameterInteger,   STA_CFG_OFFSET(WindowsPowerMode),       STA_CFG_SIZE(WindowsPowerMode),         0,          0,          2           },

    {NDIS_STRING_CONST("FastRoamingSensitivity"), 0,NdisParameterInteger,   STA_CFG_OFFSET(FastRoamingSensitivity), STA_CFG_SIZE(FastRoamingSensitivity),   0,          0,          5           },

    {NDIS_STRING_CONST("AutoChannelSelect"),    0,  NdisParameterInteger,   AP_CFG_OFFSET(bAutoChannelAtBootup),    AP_CFG_SIZE(bAutoChannelAtBootup),      0,          0,          1           },
    {NDIS_STRING_CONST("DtimPeriod"),           0,  NdisParameterInteger,   AP_CFG_OFFSET(DtimPeriod),              AP_CFG_SIZE(DtimPeriod),                1,          1,          5           },
    {NDIS_STRING_CONST("IdleTime"),             0,  NdisParameterInteger,   AP_CFG_OFFSET(AgeoutTime),              AP_CFG_SIZE(AgeoutTime),                300,       60,          3600        },
    {NDIS_STRING_CONST("RssiDelta"),            0,  NdisParameterInteger,   STA_CFG_OFFSET(RssiDelta),              STA_CFG_SIZE(RssiDelta),                6,          0,          20          },
    {NDIS_STRING_CONST("SmartScan"),            0,  NdisParameterInteger,   STA_CFG_OFFSET(SmartScan),              STA_CFG_SIZE(SmartScan),                0,          0,          1           },
    {NDIS_STRING_CONST("PSControl"),            0,  NdisParameterInteger,   STA_CFG_OFFSET(PSControl),              STA_CFG_SIZE(PSControl),                0,          0,          0xffffffff  },
    {NDIS_STRING_CONST("KeepReconnectCnt"),     0,  NdisParameterInteger,   STA_CFG_OFFSET(KeepReconnectCnt),       STA_CFG_SIZE(KeepReconnectCnt),         3,          1,          30          },
    {NDIS_STRING_CONST("SoftAPForceMulticastRate"),         0,  NdisParameterHexInteger,    AP_CFG_OFFSET(SoftAPForceMulticastRate),            AP_CFG_SIZE(SoftAPForceMulticastRate),              0x00000000,         0,          0xffffffff          },  // SoftAPForceMulticastRate

    //LED
    {NDIS_STRING_CONST("LEDCtlBySW"),           0,  NdisParameterInteger,   HW_CFG_OFFSET(LedControlBySW),            HW_CFG_SIZE(LedControlBySW),              0,          0,          1           },
    {NDIS_STRING_CONST("LEDMode"),              0,  NdisParameterInteger,   HW_CFG_OFFSET(INFLedCntl),                HW_CFG_SIZE(INFLedCntl),                  0,          0,          0xff        },
    {NDIS_STRING_CONST("LEDConfig"),            0,  NdisParameterInteger,   HW_CFG_OFFSET(INFLed1),                   HW_CFG_SIZE(INFLed1),                     0,          0,          0xffff      },
    {NDIS_STRING_CONST("LEDAct"),               0,  NdisParameterInteger,   HW_CFG_OFFSET(INFLed2),                   HW_CFG_SIZE(INFLed2),                     0,          0,          0xffff      },
    {NDIS_STRING_CONST("LEDPolarity"),          0,  NdisParameterInteger,   HW_CFG_OFFSET(INFLed3),                   HW_CFG_SIZE(INFLed3),                     0,          0,          0xffff      },
    {NDIS_STRING_CONST("TEST"),                 0,  NdisParameterInteger,   STA_CFG_OFFSET(WhqlTest),               STA_CFG_SIZE(WhqlTest),                 0,          0,          1           },      

    {NDIS_STRING_CONST("CountryTxPwr"),         0,  NdisParameterInteger,   COMMON_CFG_OFFSET(CountryTxPwr),        COMMON_CFG_SIZE(CountryTxPwr),          0xffffffff, 0,          0xffffffff  },
    {NDIS_STRING_CONST("CountryTxPwr5G"),       0,  NdisParameterInteger,   COMMON_CFG_OFFSET(CountryTxPwr5G),      COMMON_CFG_SIZE(CountryTxPwr5G),        0xffffffff, 0,          0xffffffff  },
    {NDIS_STRING_CONST("LinkSpeedStatus"),      0,  NdisParameterInteger,   STA_CFG_OFFSET(FixLinkSpeedStatus), STA_CFG_SIZE(FixLinkSpeedStatus),       0,          0,          2           },
    //WAPI
    {NDIS_STRING_CONST("WAPI"),               0,  NdisParameterInteger,   RTMP_OFFSET(WAPI),                    RTMP_SIZE(WAPI),                      1,  0,          0xffffffff           },   

    //MFP
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    {NDIS_STRING_CONST("MFPC"),         0,  NdisParameterInteger,   PMF_CFG_OFFSET(PmfControl.MFP_Enable),      PMF_CFG_SIZE(RSNCapability),        0,          0,      1},
    {NDIS_STRING_CONST("MFPR"),         0,  NdisParameterInteger,   PMF_CFG_OFFSET(PmfControl.MFP_Required),        PMF_CFG_SIZE(RSNCapability),        0,          0,      1},
#else
    {NDIS_STRING_CONST("MFPC"),                 0,  NdisParameterInteger,   PMF_CFG_OFFSET(PmfControl.MFP_Enable),      PMF_CFG_SIZE(PmfControl.MFP_Enable),        0,          0,          1           },
    {NDIS_STRING_CONST("MFPR"),                 0,  NdisParameterInteger,   PMF_CFG_OFFSET(PmfControl.MFP_Required),    PMF_CFG_SIZE(PmfControl.MFP_Required),      0,          0,          1           },
#endif

    // Reposit Bssid List
    {NDIS_STRING_CONST("BssidRepositTime"), 0,  NdisParameterInteger,   COMMON_CFG_OFFSET(BssidRepositTime),    COMMON_CFG_SIZE(BssidRepositTime),  DEFAULT_REPOSIT_TIME,   MIN_REPOSIT_TIME,   MAX_REPOSIT_TIME    },
    {NDIS_STRING_CONST("BeaconLostTime"),       0,  NdisParameterInteger,   STA_CFG_OFFSET(BeaconLostTime),         STA_CFG_SIZE(BeaconLostTime),           3,          3,          8           },
    //HT is disabled on 11n when using TKIP.
    {NDIS_STRING_CONST("ProhibitTKIPonHT"),     0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bProhibitTKIPonHT),   COMMON_CFG_SIZE(bProhibitTKIPonHT), 0,  0,  1},

    // Link Quality Setting Control
    {NDIS_STRING_CONST("LinkQualitySetting"),   0,  NdisParameterInteger,   STA_CFG_OFFSET(LinkQualitySetting),         STA_CFG_SIZE(LinkQualitySetting),0x0,   0x0,    0xff},

    // Channel 14 support 11b only
    {NDIS_STRING_CONST("Ch14BOnly"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(Ch14BOnly),           COMMON_CFG_SIZE(Ch14BOnly),             1,          0,          1           },

    {NDIS_STRING_CONST("PALRegulatoryClass"),       0,  NdisParameterInteger,   COMMON_CFG_OFFSET(PALRegulatoryClass),      COMMON_CFG_SIZE(PALRegulatoryClass),        254,        0,          0xff},
    {NDIS_STRING_CONST("PALCapability"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(PALCapability),           COMMON_CFG_SIZE(PALCapability),             0,      0,          0xffffffff},
    {NDIS_STRING_CONST("PALVersionSpecifier"),      0,  NdisParameterInteger,   COMMON_CFG_OFFSET(PALVersionSpecifier),     COMMON_CFG_SIZE(PALVersionSpecifier),       0,      0,          0xff},
    {NDIS_STRING_CONST("PALCompany_ID"),        0,  NdisParameterInteger,   COMMON_CFG_OFFSET(PALCompany_ID),       COMMON_CFG_SIZE(PALCompany_ID),         0,      0,          0xffff},
    {NDIS_STRING_CONST("PALSubVersion"),            0,  NdisParameterInteger,   COMMON_CFG_OFFSET(PALSubVersion),           COMMON_CFG_SIZE(PALSubVersion),         0,      0,          0xffff},

    // GPIO pin-sharing mode
    {NDIS_STRING_CONST("GpioPinSharingMode"),   0,  NdisParameterInteger,   COMMON_CFG_OFFSET(GpioPinSharingMode),  COMMON_CFG_SIZE(GpioPinSharingMode),    GpioPinSharingMode_DefaultMode,     GpioPinSharingMode_LowerBound,  GpioPinSharingMode_UpperBound},

    // Blad Eagle function
    {NDIS_STRING_CONST("BaldEagleSSID"),       0,   NdisParameterString,    STRING_OFFSET(BaldEagleSSID),      STRING_SIZE(BaldEagleSSID),              0,                      0,                  1                },

    //Led config
    {NDIS_STRING_CONST("LedCfg"),       0,  NdisParameterInteger,   COMMON_CFG_OFFSET(LedCfg),      COMMON_CFG_SIZE(LedCfg),        0,  0,          1   },
    
    {NDIS_STRING_CONST("Feature11n"),           0,  NdisParameterInteger,   STA_CFG_OFFSET(Feature11n),         STA_CFG_SIZE(Feature11n),               1,          0,          0xffffffff          },

    // Can disable bEnableFrequencyCalibration
    {NDIS_STRING_CONST("AdaptiveFreq"),             0,  NdisParameterInteger,   COMMON_CFG_OFFSET(AdaptiveFreq),            COMMON_CFG_SIZE(AdaptiveFreq),              1,          0,          1           },

    // Set tx power level
    {NDIS_STRING_CONST("TxPowerDefault"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(TxPowerDefault),          COMMON_CFG_SIZE(TxPowerDefault),                100,            0,          100         },

    // For 11n Tx Burst timing test.
    {NDIS_STRING_CONST("11nTxBurstTimeTest"),               0,  NdisParameterInteger,   COMMON_CFG_OFFSET(b11nTxBurstTimeTest),     COMMON_CFG_SIZE(b11nTxBurstTimeTest),       0,          0,          1           },

    // For FCC Power Test
    {NDIS_STRING_CONST("AdTxPwrForFCCTest"),                0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bAdTxPwrForFCCTest),      COMMON_CFG_SIZE(bAdTxPwrForFCCTest),        0,          0,          1           },

    
    // Adjust Tx power mode For Customer , 0: no use, 1: For Asus
    {NDIS_STRING_CONST("CustomizedPowerMode"),              0,  NdisParameterInteger,   COMMON_CFG_OFFSET(CustomizedPowerMode),     COMMON_CFG_SIZE(CustomizedPowerMode),       0,          0,          0xFF            },
    {NDIS_STRING_CONST("MaxTxPower"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(MaxTxPower),          COMMON_CFG_SIZE(MaxTxPower),                0,          0,          0xFF            },
    {NDIS_STRING_CONST("CountryTargetTxPowerMode"),             0,  NdisParameterInteger,   COMMON_CFG_OFFSET(CountryTargetTxPowerMode),        COMMON_CFG_SIZE(CountryTargetTxPowerMode),      0,          0,          0xFF            },

    // Can Enable/Disable Stream mode
    {NDIS_STRING_CONST("StreamMode"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(StreamMode),          COMMON_CFG_SIZE(StreamMode),                0,          0,          1           },

    // Registry control to Enable/Disable function of ChannelSwitchOffloadToFW
    {NDIS_STRING_CONST("ChannelSwitchOffloadToFW"),             0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bChannelSwitchOffloadToFW),           COMMON_CFG_SIZE(bChannelSwitchOffloadToFW),             1,          0,          1           },

    // bit 0~3 is value and bit 4~7 is id for BBP setting
    {NDIS_STRING_CONST("BBPIDV"),           0,  NdisParameterInteger,   COMMON_CFG_OFFSET(BBPIDV),          COMMON_CFG_SIZE(BBPIDV),               0,           0,          0xffffffff          },
    // Check Security when setting WEP to connect AP
    {NDIS_STRING_CONST("WepConnectCheckEncry"),             0,  NdisParameterInteger,   COMMON_CFG_OFFSET(WepConnectCheckEncry),            COMMON_CFG_SIZE(WepConnectCheckEncry),             0,           0,          1           },

    {NDIS_STRING_CONST("FeatureSetting"),       0,  NdisParameterInteger,   COMMON_CFG_OFFSET(FeatureSetting),      COMMON_CFG_SIZE(FeatureSetting),        0,  0,          0xffffffff},

    // TxBF
    {NDIS_STRING_CONST("ETxBfEn"),                      0,  NdisParameterInteger,   TX_BF_CTRL_OFFSET(ETxBfEn),                 TX_BF_CTRL_SIZE(ETxBfEn),                       0,  0,  1}, 
    {NDIS_STRING_CONST("ETxBfTimeout"),                 0,  NdisParameterInteger,   TX_BF_CTRL_OFFSET(ETxBfTimeout),                TX_BF_CTRL_SIZE(ETxBfTimeout),                  0,  0,  65535}, 
    {NDIS_STRING_CONST("ETxBfNonCompressedFeedback"),   0,  NdisParameterInteger,   TX_BF_CTRL_OFFSET(ETxBfNonCompressedFeedback),  TX_BF_CTRL_SIZE(ETxBfNonCompressedFeedback),    0,  0,  1}, 

    //P2P Reinforced ACK check
    {NDIS_STRING_CONST("P2PAckCheck"),  0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bP2pAckCheck),    COMMON_CFG_SIZE(bP2pAckCheck),      1,          0,          1           },  
    // P2P Video Boost switch
    {NDIS_STRING_CONST("P2PVideoBoost"),    0,  NdisParameterInteger,   COMMON_CFG_OFFSET(P2PVideoBoost),   COMMON_CFG_SIZE(P2PVideoBoost), 0,  0,  1},

    //
    // Tx/Rx configuration
    //
    {NDIS_STRING_CONST("TxRxConfigByRegistry"), 0,  NdisParameterInteger,   COMMON_CFG_OFFSET(TxRxConfigByRegistry),    COMMON_CFG_SIZE(TxRxConfigByRegistry),  TX_RX_CONFIG_BY_REGISTRY_DISABLE,   TX_RX_CONFIG_BY_REGISTRY_DISABLE,   TX_RX_CONFIG_BY_REGISTRY_MAX},
    //WSC 2.0
    {NDIS_STRING_CONST("WSCVersion2"),       0, NdisParameterInteger,   STA_CFG_OFFSET(WSCVersion2),      STA_CFG_SIZE(WSCVersion2),    0x20,   0x0,    0x20},  
    {NDIS_STRING_CONST("WSC20Testbed"),       0,    NdisParameterInteger,   STA_CFG_OFFSET(WSC20Testbed),      STA_CFG_SIZE(WSC20Testbed),  0x08,   0x0,    0xffffffff},    
    {NDIS_STRING_CONST("WscHwPBCMode"),     0,  NdisParameterInteger,   STA_CFG_OFFSET(WscHwPBCMode),       STA_CFG_OFFSET(WscHwPBCMode),   0,  0,  1},

    //Multicast policy: 0-> defalult(enable "bypass RA packet") 1->disable "bypass RA packet" 2->will not clear up accepted multicast poll
    {NDIS_STRING_CONST("MPolicy"),          0,  NdisParameterInteger,   COMMON_CFG_OFFSET(MPolicy),         COMMON_CFG_SIZE(MPolicy),               0,      0,          0xffffffff},

    {NDIS_STRING_CONST("SuppressScan"),         0,  NdisParameterInteger,   STA_CFG_OFFSET(OzmoDeviceSuppressScan),             STA_CFG_SIZE(OzmoDeviceSuppressScan),               0,          0,          0xffffffff          },

    //
    // PPAD (per-packet antenna diversity)
    //
    {NDIS_STRING_CONST("EnablePPAD"),   0,  NdisParameterInteger,   COMMON_CFG_OFFSET(bPPAD),   COMMON_CFG_SIZE(bPPAD), 1,  0,  1}, 

    //
    // Calibration control registry
    //
    {NDIS_STRING_CONST("CalCtrl"),  0,  NdisParameterInteger,   COMMON_CFG_OFFSET(CalCtrlRegistry), COMMON_CFG_SIZE(CalCtrlRegistry),   0x00000001, 0,  0xFFFFFFFF}, 

    // Check if we are running WHQL testing or not.
    {NDIS_STRING_CONST("OnTestingWHQL"),            0,  NdisParameterInteger,   LOGO_TEST_CFG_OFFSET(OnTestingWHQL),            LOGO_TEST_CFG_SIZE(OnTestingWHQL),              0,          0,          1           },
    {NDIS_STRING_CONST("OnTestingCPUUtil"),         0,  NdisParameterInteger,   LOGO_TEST_CFG_OFFSET(OnTestingCPUUtil),            LOGO_TEST_CFG_SIZE(OnTestingCPUUtil),              0,          0,          1           },
    {NDIS_STRING_CONST("WHQLScanTime"),         0,  NdisParameterInteger,   LOGO_TEST_CFG_OFFSET(WHQLScanTime),            LOGO_TEST_CFG_SIZE(WHQLScanTime),              60,          10,          140           },

    // Win8 In-Box Mode
    {NDIS_STRING_CONST("ChannelMode"),          0,  NdisParameterInteger,   HW_CFG_OFFSET(ChannelMode),            HW_CFG_SIZE(ChannelMode),              0,          0,          2           },
    {NDIS_STRING_CONST("InBoxMode"),    0,  NdisParameterInteger,   COMMON_CFG_OFFSET(InBoxMode),           COMMON_CFG_SIZE(InBoxMode), 0,  0,  1},

    //
    // Bandedge power table enable
    //
    {NDIS_STRING_CONST("BandedgePowerTableEnable"), 0, NdisParameterInteger, COMMON_CFG_OFFSET(BandedgePowerTableEnable), COMMON_CFG_SIZE(BandedgePowerTableEnable), 1, 0, 1},

    //
    // Switch Channel Offload Enable
    //
    {NDIS_STRING_CONST("SwitchChannelOffloadEnable"), 0, NdisParameterInteger, COMMON_CFG_OFFSET(SwitchChannelOffloadEnable), COMMON_CFG_SIZE(SwitchChannelOffloadEnable), 1, 0, 1},

    //
    // Full Calibration by FW with one cmd toggle
    //
    {NDIS_STRING_CONST("FullCalibrationOffloadEnable"), 0, NdisParameterInteger, COMMON_CFG_OFFSET(FullCalibrationOffloadEnable), COMMON_CFG_SIZE(FullCalibrationOffloadEnable), 0, 0, 1},  

    //
    // Thermal solution Enable
    //
    {NDIS_STRING_CONST("ThermalEnable"), 0, NdisParameterInteger, COMMON_CFG_OFFSET(ThermalEnable), COMMON_CFG_SIZE(ThermalEnable), 0, 0, 1},

    //
    // Unify Mac - FPGA
    //
    // 1. Endpint configuration
    // 2. Tx 1 EP to m Queues
    // 3. Tx m EPs to 1 Queue
    // 4. Tx m EPs to m Queues
    // 5. 1 Set WMM
    // 6. 2 Sets WMM
    //{NDIS_STRING_CONST("ThermalEnable"), 0, NdisParameterInteger, COMMON_CFG_OFFSET(ThermalEnable), COMMON_CFG_SIZE(ThermalEnable), 0, 0, 1},
};
#endif
#define NIC_NUM_REG_PARAMS  (sizeof (NICRegTable) / sizeof(RTMP_REG_ENTRY))

//
// RF register initialization set
//
REG_PAIR   RFRegTable[] = {
    {RF_R04,        0x40},
    {RF_R05,        0x03},
    {RF_R06,        0x02},
    {RF_R07,        0x70},
    {RF_R09,        0x0F},
    {RF_R10,        0x41},
    {RF_R11,        0x21},
    {RF_R12,        0x7B},
    {RF_R14,        0x90},
    {RF_R15,        0x58},
    {RF_R16,        0xB3},
    {RF_R17,        0x92},
    {RF_R18,        0x2C},
    {RF_R19,        0x02},
    {RF_R20,        0xBA},
    {RF_R21,        0xDB},
    {RF_R24,        0x16},
    {RF_R25,        0x01},
    {RF_R27,        0x03},
    {RF_R29,        0x1F},
};
#define NUM_RF_REG_PARMS    (sizeof(RFRegTable) / sizeof(REG_PAIR))

TX_POWER_CONTROL_EXT_OVER_MAC_WITH_BANDEDGE TxPwrTableForFCC =
{
  /*** BW20Over2Dot4G ***/
  0x10101111,
  0x10101111,
  0x10110e0f,
  0x10110e0f,
  0x0e0e0e0f,
  0x0e0e0e0f,
  0x10110e0e,
  0x10110e0e,
  0x00000e0f,
  0x00000e0f,
  0x0e0e0e0e,
  0x0e0e0e0e,
  0x0e0e0e0e,
  0x0e0e0e0e,
  0x0000000e,
 /*** BW40Over2Dot4G ***/
  0x10101111,
  0x10101111,
  0x0f0f0e0f,
  0x0f0f0e0f,
  0x0e0e0e0f,
  0x0e0e0e0f,
  0x0f0f0e0e,
  0x0f0f0e0e,
  0x00000e0f,
  0x00000e0f,
  0x0e0e0e0e,
  0x0e0e0e0e,
  0x0e0e0e0e,
  0x0e0e0e0e,
  0x0000000e,
 /*** BW20Over2Dot4GBandEdge ***/
  0x10101111,
  0x10101111,
  0x0d0d0e0f,
  0x0d0d0e0f,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0e0e,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW40Over2Dot4GBandEdge ***/
  0x10101111,
  0x10101111,
  0x0d0d0e0f,
  0x0d0d0e0f,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0e0e,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW20Over5G ***/
  0x0f100000,
  0x0f100000,
  0x0f100d0e,
  0x0f100d0e,
  0x0d0d0d0e,
  0x0d0d0d0e,
  0x0f100d0d,
  0x0f100d0d,
  0x00000d0e,
  0x00000d0e,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW40Over5G ***/
  0x0f100000,
  0x0f100000,
  0x0f0f0d0e,
  0x0f0f0d0e,
  0x0d0d0d0e,
  0x0d0d0d0e,
  0x0f0f0d0d,
  0x0f0f0d0d,
  0x00000d0e,
  0x00000d0e,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW20Over5GBandEdge ***/
  0x0e0e0000,
  0x0e0e0000,
  0x0a0a0e0e,
  0x0a0a0e0e,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x00000a0a,
  0x00000a0a,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x0a0a0e0e,
  0x0a0a0a0a,
  0x0000000a,
 /*** BW40Over5GBandEdge ***/
  0x0e0e0000,
  0x0e0e0000,
  0x0b0b0e0e,
  0x0b0b0e0e,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x00000b0b,
  0x00000b0b,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x0b0b0e0e,
  0x0b0b0b0b,
  0x0000000b,
};

TX_POWER_CONTROL_EXT_OVER_MAC_WITH_BANDEDGE TxPwrTableForCE =
{
  /*** BW20Over2Dot4G ***/         
  0x10101111,                      
  0x10101111,                      
  0x0d0d0e0f,                      
  0x0d0d0e0f,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0e0e,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
  /*** BW40Over2Dot4G ***/         
  0x10101111,                      
  0x10101111,                      
  0x0d0d0e0f,                      
  0x0d0d0e0f,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0e0e,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
  /*** BW20Over2Dot4GBandEdge ***/ 
  0x10101111,                      
  0x10101111,                      
  0x0d0d0e0f,                      
  0x0d0d0e0f,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0e0e,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
  /*** BW40Over2Dot4GBandEdge ***/ 
  0x10101111,                      
  0x10101111,                      
  0x0d0d0e0f,                      
  0x0d0d0e0f,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0e0e,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
  /*** BW20Over5G ***/             
  0x0f100000,                      
  0x0f100000,                      
  0x0d0d0d0e,                      
  0x0d0d0d0e,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
  /*** BW40Over5G ***/             
  0x0f100000,                      
  0x0f100000,                      
  0x0d0d0d0e,                      
  0x0d0d0d0e,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
  /*** BW20Over5GBandEdge ***/     
  0x0f100000,                      
  0x0f100000,                      
  0x0d0d0d0e,                      
  0x0d0d0d0e,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
  /*** BW40Over5GBandEdge ***/     
  0x0f100000,                      
  0x0f100000,                      
  0x0d0d0d0e,                      
  0x0d0d0d0e,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x00000d0d,                      
  0x00000d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0d0d0d0d,                      
  0x0000000d,                      
    
};

TX_POWER_CONTROL_EXT_OVER_MAC_WITH_BANDEDGE TxPwrTableForAS =
{
 /*** BW20Over2Dot4GBand ***/
  0x10101111,
  0x10101111,
  0x0d0d0e0f,
  0x0d0d0e0f,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0e0e,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW40Over2Dot4GBand ***/
  0x10101111,
  0x10101111,
  0x0d0d0e0f,
  0x0d0d0e0f,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0e0e,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW20Over2Dot4GBandEdge ***/
  0x10101111,
  0x10101111,
  0x0d0d0e0f,
  0x0d0d0e0f,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0e0e,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW40Over2Dot4GBandEdge ***/
  0x10101111,
  0x10101111,
  0x0d0d0e0f,
  0x0d0d0e0f,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0e0e,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW20Over5GBand ***/
  0x0f100000,
  0x0f100000,
  0x0d0d0d0e,
  0x0d0d0d0e,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW40Over5GBand ***/
  0x0f100000,
  0x0f100000,
  0x0d0d0d0e,
  0x0d0d0d0e,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x00000d0d,
  0x00000d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0d0d0d0d,
  0x0000000d,
 /*** BW20Over5GBandEdge ***/
  0x0e0e0000,
  0x0e0e0000,
  0x0a0a0d0e,
  0x0a0a0d0e,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x00000a0a,
  0x00000a0a,
  0x0a0a0a0a,
  0x0a0a0a0a,
  0x0a0a0d0d,
  0x0a0a0a0a,
  0x0000000a,
 /*** BW40Over5GBandEdge ***/
  0x0e0e0000,
  0x0e0e0000,
  0x0b0b0d0e,
  0x0b0b0d0e,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x00000b0b,
  0x00000b0b,
  0x0b0b0b0b,
  0x0b0b0b0b,
  0x0b0b0d0d,
  0x0b0b0b0b,
  0x0000000b,
    
};

//
// ASIC register initialization sets
//

RTMP_REG_PAIR   MACRegTable[] = {
    {LEGACY_BASIC_RATE,     0x0000013f}, //  Basic rate set bitmap
    {HT_BASIC_RATE,     0x00008003}, // Basic HT rate set , 20M, MCS=3, MM. Format is the same as in TXWI.
    {MAC_SYS_CTRL,      0x00}, // 0x1004, , default Disable RX
    {RX_FILTR_CFG,      0x17f97}, //0x1400  , RX filter control,  
    {BKOFF_SLOT_CFG,        0x209}, // default set short slot time,  
    {TX_SW_CFG0,        0x40a06}, // Gary,2006-08-23 
    {TX_SW_CFG1,        0x80606}, // Gary,2006-08-23 
    {TX_LINK_CFG,       0x1020}, // Gary,2006-08-23 
    {TX_TIMEOUT_CFG,        0x000a2090},    // CCK has some problem. So increase timieout value. 2006-10-09. DecreaeTXOP_timeout to 0xa 2007-07-26
    {MAX_LEN_CFG,       0x000E2fff}, // 0x3018, MAX frame length. Max PSDU = 16kbytes. Change MIN_MPDU_LEN from A to E.(including FCS 4 bytes)
    {LED_CFG,       0x7f031e46}, // Gary, 2006-08-23
    {WMM_AIFSN_CFG,     0x00002273},
    {WMM_CWMIN_CFG,     0x00002344},
    {WMM_CWMAX_CFG,     0x000034aa},
    {PBF_MAX_PCNT,          0x1F3fbf9f},        //Jan, 2006/04/20
    {AUTO_RSP_CFG,          0x13},
    {TX_RTY_CFG,            0x47d01f1f},        //Jan, 2006/10/06
    {0x408, 0xf4001E}, //  Enable Queue 0,1 and 2
    {CCK_PROT_CFG,      0x01700003},    // Initial Auto_Responder, because QA will turn off Auto-Responder
    {OFDM_PROT_CFG,     0x01700003},    // Initial Auto_Responder, because QA will turn off Auto-Responder
    {GF20_PROT_CFG,     0x01744005},    // Initial Auto_Responder, because QA will turn off Auto-Responder;use 36Mbps by default
    {GF40_PROT_CFG,     0x03F44085},    // Initial Auto_Responder, because QA will turn off Auto-Responder;use 36Mbps by default
    {MM20_PROT_CFG,     0x01744005},    // Initial Auto_Responder, because QA will turn off Auto-Responder;use 36Mbps by default
    {MM40_PROT_CFG,     0x03F44085},    // Initial Auto_Responder, because QA will turn off Auto-Responder;use 36Mbps by default
    {WCID_RESERVED, 0x1}, //ff for mlme packet
    {TXOP_CTRL_CFG,     0x0000243f},    //Extension channel backoff. bit7=0 if sense extension CCA before 40MHz TX. 2007-07-13 Jerry
    {TX_RTS_CFG,        0x01092b20},    // Rts Retry limit set to 0x20. 2006-12-27 Jan; Enable HW RTS fallback but the bit0 of MCS can't be zero.(ASIC bug)
    {WPDMA_GLO_CFG,     0x00000030},
    {TXOP_HLDR_ET,      0x00000002},
    {PWR_PIN_CFG,       0x00000003},    // RT2872 (USB) Default is 0x0A (Power Save), need to set to 0x03   
    {EXP_ACK_TIME,      0x002C00DC},    // Jerry 2010-11    
};

//
// RT2870 Firmware Spec only used 1 oct for version expression
//
#if defined(_WIN64)
#define FIRMWARE_MINOR_VERSION      35
#else
#define FIRMWARE_MINOR_VERSION      34
#endif

//
// The verification of the Tx power per rate
//
// Parameters
//  pAd: The adapter data structure
//  pTxPwr: Point to a Tx power per rate
//
// Return Value
//  None
//
VOID RTMPVerifyTxPwrPerRateExt(
    IN PMP_ADAPTER pAd, 
    IN OUT PUCHAR pTxPwr)
{
    DBGPRINT(RT_DEBUG_INFO, ("--> %s\n", __FUNCTION__));

    if (*pTxPwr > 0xC)
    {
        *pTxPwr = 0xC;
    }
    
    DBGPRINT(RT_DEBUG_INFO, ("<-- %s\n", __FUNCTION__));
}

/*
    ========================================================================
    
    Routine Description:
        Read initial Tx power per MCS and BW from EEPROM
        
    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    Note:
        
    ========================================================================
*/
VOID    RTMPReadTxPwrPerRate(
    IN  PMP_ADAPTER   pAd)
{
    ULONG       data, Adata40M, Gdata40M;
    USHORT      i, value, value2;
    INT         Apwrdelta, Gpwrdelta;
    UCHAR       t1,t2,t3,t4;
    BOOLEAN     bApwrdeltaMinus = FALSE, bGpwrdeltaMinus = FALSE;

    if ((pAd->HwCfg.buseEfuse == TRUE && pAd->HwCfg.HwArchitectureVersion.eFuseVersion == eFuseArchitectureVersion_EfuseSizeReduction))
    {

    }
    else
    {
        //
        // Get power delta for 20MHz and 40MHz.
        //
        DBGPRINT(RT_DEBUG_TRACE, ("Txpower per Rate\n"));
        RTUSBReadEEPROM(pAd, EEPROM_TXPOWER_DELTA, (PUCHAR)&value2, 2);
        Apwrdelta = 0;
        Gpwrdelta = 0;

        if ((value2 & 0xff) != 0xff)
        {
            if (value2 & 0x80)
                Gpwrdelta = (value2&0xf);
            
            if (value2 & 0x40)
                bGpwrdeltaMinus = FALSE;
            else
                bGpwrdeltaMinus = TRUE;
        }
        if ((value2 & 0xff00) != 0xff00)
        {
            if (value2 & 0x8000)
                Apwrdelta = ((value2&0xf00)>>8);

            if (value2 & 0x4000)
                bApwrdeltaMinus = FALSE;
            else
                bApwrdeltaMinus = TRUE;
        }   
        DBGPRINT(RT_DEBUG_TRACE, ("Gpwrdelta = %x, Apwrdelta = %x .\n", Gpwrdelta, Apwrdelta));
        
        for (i = 0; i < 5; i++)
        {
            RTUSBReadEEPROM(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4, (PUCHAR)&value, 2);
            data = value;
            if (bApwrdeltaMinus == FALSE)
            {
                t1 = (value&0xf)+(Apwrdelta);
                if (t1 > 0xf)
                    t1 = 0xf;
                t2 = ((value&0xf0)>>4)+(Apwrdelta);
                if (t2 > 0xf)
                    t2 = 0xf;
                t3 = ((value&0xf00)>>8)+(Apwrdelta);
                if (t3 > 0xf)
                    t3 = 0xf;
                t4 = ((value&0xf000)>>12)+(Apwrdelta);
                if (t4 > 0xf)
                    t4 = 0xf;
            }
            else
            {
                if ((value&0xf) > Apwrdelta)
                    t1 = (value&0xf)-(Apwrdelta);
                else
                    t1 = 0;
                if (((value&0xf0)>>4) > Apwrdelta)
                    t2 = ((value&0xf0)>>4)-(Apwrdelta);
                else
                    t2 = 0;
                if (((value&0xf00)>>8) > Apwrdelta)
                    t3 = ((value&0xf00)>>8)-(Apwrdelta);
                else
                    t3 = 0;
                if (((value&0xf000)>>12) > Apwrdelta)
                    t4 = ((value&0xf000)>>12)-(Apwrdelta);
                else
                    t4 = 0;
            }               
            Adata40M = t1 + (t2<<4) + (t3<<8) + (t4<<12);
            if (bGpwrdeltaMinus == FALSE)
            {
                t1 = (value&0xf)+(Gpwrdelta);
                if (t1 > 0xf)
                    t1 = 0xf;
                t2 = ((value&0xf0)>>4)+(Gpwrdelta);
                if (t2 > 0xf)
                    t2 = 0xf;
                t3 = ((value&0xf00)>>8)+(Gpwrdelta);
                if (t3 > 0xf)
                    t3 = 0xf;
                t4 = ((value&0xf000)>>12)+(Gpwrdelta);
                if (t4 > 0xf)
                    t4 = 0xf;
            }
            else
            {
                if ((value&0xf) > Gpwrdelta)
                    t1 = (value&0xf)-(Gpwrdelta);
                else
                    t1 = 0;
                if (((value&0xf0)>>4) > Gpwrdelta)
                    t2 = ((value&0xf0)>>4)-(Gpwrdelta);
                else
                    t2 = 0;
                if (((value&0xf00)>>8) > Gpwrdelta)
                    t3 = ((value&0xf00)>>8)-(Gpwrdelta);
                else
                    t3 = 0;
                if (((value&0xf000)>>12) > Gpwrdelta)
                    t4 = ((value&0xf000)>>12)-(Gpwrdelta);
                else
                    t4 = 0;
            }               
            Gdata40M = t1 + (t2<<4) + (t3<<8) + (t4<<12);
            
            RTUSBReadEEPROM(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4 + 2, (PUCHAR)&value, 2);
            if (bApwrdeltaMinus == FALSE)
            {
                t1 = (value&0xf)+(Apwrdelta);
                if (t1 > 0xf)
                    t1 = 0xf;
                t2 = ((value&0xf0)>>4)+(Apwrdelta);
                if (t2 > 0xf)
                    t2 = 0xf;
                t3 = ((value&0xf00)>>8)+(Apwrdelta);
                if (t3 > 0xf)
                    t3 = 0xf;
                t4 = ((value&0xf000)>>12)+(Apwrdelta);
                if (t4 > 0xf)
                    t4 = 0xf;
            }
            else
            {
                if ((value&0xf) > Apwrdelta)
                    t1 = (value&0xf)-(Apwrdelta);
                else
                    t1 = 0;
                if (((value&0xf0)>>4) > Apwrdelta)
                    t2 = ((value&0xf0)>>4)-(Apwrdelta);
                else
                    t2 = 0;
                if (((value&0xf00)>>8) > Apwrdelta)
                    t3 = ((value&0xf00)>>8)-(Apwrdelta);
                else
                    t3 = 0;
                if (((value&0xf000)>>12) > Apwrdelta)
                    t4 = ((value&0xf000)>>12)-(Apwrdelta);
                else
                    t4 = 0;
            }               
            Adata40M |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
            if (bGpwrdeltaMinus == FALSE)
            {
                t1 = (value&0xf)+(Gpwrdelta);
                if (t1 > 0xf)
                    t1 = 0xf;
                t2 = ((value&0xf0)>>4)+(Gpwrdelta);
                if (t2 > 0xf)
                    t2 = 0xf;
                t3 = ((value&0xf00)>>8)+(Gpwrdelta);
                if (t3 > 0xf)
                    t3 = 0xf;
                t4 = ((value&0xf000)>>12)+(Gpwrdelta);
                if (t4 > 0xf)
                    t4 = 0xf;
            }
            else
            {
                if ((value&0xf) > Gpwrdelta)
                    t1 = (value&0xf)-(Gpwrdelta);
                else
                    t1 = 0;
                if (((value&0xf0)>>4) > Gpwrdelta)
                    t2 = ((value&0xf0)>>4)-(Gpwrdelta);
                else
                    t2 = 0;
                if (((value&0xf00)>>8) > Gpwrdelta)
                    t3 = ((value&0xf00)>>8)-(Gpwrdelta);
                else
                    t3 = 0;
                if (((value&0xf000)>>12) > Gpwrdelta)
                    t4 = ((value&0xf000)>>12)-(Gpwrdelta);
                else
                    t4 = 0;
            }               
            Gdata40M |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
            data |= (value<<16);
            
            pAd->Tx20MPwrCfgABand[i] = pAd->Tx20MPwrCfgGBand[i] = data;
            pAd->Tx40MPwrCfgABand[i] = Adata40M;
            pAd->Tx40MPwrCfgGBand[i] = Gdata40M;
            
            if (data != 0xffffffff)
                RTUSBWriteMACRegister(pAd, TX_PWR_CFG_0 + i*4, data);
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%x,  Adata40M = %x,  Gdata40M = %x \n", data, Adata40M, Gdata40M));
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPReadTxPwrPerRate\n"));
}

BOOLEAN IsRalinkEncStr(TCHAR *szIn)
{
    BOOLEAN bResult = FALSE;
    
    if(strlen(szIn) > 6)
    {
        if((strlen(szIn) % 2) == 0)
        {
            if(szIn[0] == 'R' && szIn[1] == 'A')
            {
                int nLen = (int) strlen(szIn);
                UCHAR nRKey, nChksum;
                TCHAR szRKey[4], szChkSum[4];
                
                szRKey[0] = szIn[2];
                szRKey[1] = szIn[nLen-3];
                szRKey[2] = '\0';
                AtoH(szRKey, &nRKey, 1);

                szChkSum[0] = szIn[nLen-2];
                szChkSum[1] = szIn[nLen-1];
                szChkSum[2] = '\0';
                AtoH(szChkSum, &nChksum, 1);

                if((nRKey + nChksum) == 0xFF)
                {
                    bResult = TRUE;
                }
            }
        }
    }
    return bResult; 
}

int Decrypt(TCHAR *szIn, TCHAR *szOut, int nLen)
{
    TCHAR   szKey[4], szTmp[4];
    UCHAR   nRKey;
    UCHAR   byteTmp;
    int     i;

    if(!IsRalinkEncStr(szIn))
    {
#pragma prefast(suppress: __WARNING_BANNED_API_USAGE, " should not use this api")               
        strcpy(szOut, szIn);
        return ((int)strlen(szOut));
    }

    //Get RKey
    szKey[0] = szIn[2];
    szKey[1] = szIn[nLen-3];
    szKey[2] = '\0';
    AtoH(szKey, &nRKey, 1);

    for(i = 0;i < (nLen-6)/2;i++)
    {
        szTmp[0] = szIn[(i+1)*2+1];
        szTmp[1] = szIn[(i+1)*2+2];
        szTmp[2] = '\0';
        AtoH(szTmp, &byteTmp, 1);
        
        szOut[i] = byteTmp ^ (nRKey + ((i+1) * 13));
    }
    szOut[i] = '\0';

    return ((int)strlen(szOut));
}

VOID CpoyRegistryToDriverDefaultSetting (
       IN  PMP_ADAPTER       pAd
       )
{
    PlatformMoveMemory(&pAd->HwCfg.HwSupportCapabilities.MaxRegTransmitSetting, &pAd->RegistryCfg.RegTransmitSetting, sizeof(REG_TRANSMIT_SETTING));
    pAd->HwCfg.HwSupportCapabilities.MaxPhyMode = pAd->RegistryCfg.PhyMode;
    pAd->HwCfg.HwSupportCapabilities.MaxBACapability.word = pAd->RegistryCfg.BACapability.word;
    pAd->HwCfg.HwSupportCapabilities.bWmmCapable = pAd->RegistryCfg.bWmmCapable;
    
    pAd->pNicCfg->bMultipleIRP = pAd->RegistryCfg.bMultipleIRP;

    pAd->StaCfg.bAdhocNMode = pAd->RegistryCfg.bAdhocNMode;
    pAd->HwCfg.ChannelMode = pAd->RegistryCfg.ChannelMode;
    pAd->StaCfg.SmartScan = pAd->RegistryCfg.SmartScan;
    pAd->StaCfg.bSwRadioOff = pAd->RegistryCfg.bSwRadioOff;
    pAd->StaCfg.FastRoamingSensitivity = pAd->RegistryCfg.FastRoamingSensitivity;

    pAd->StaCfg.WSCVersion2 = pAd->RegistryCfg.WSCVersion2;
    pAd->StaCfg.WscHwPBCMode = pAd->RegistryCfg.WscHwPBCMode;
    pAd->StaCfg.WSC20Testbed = pAd->RegistryCfg.WSC20Testbed;
    pAd->StaCfg.BeaconLostTime = pAd->RegistryCfg.BeaconLostTime;

    pAd->HwCfg.bRdg = pAd->RegistryCfg.bRdg;
}

VOID UpdatePortDefaultSetting (
       IN  PMP_PORT       pPort
       )
{
    PMP_ADAPTER pAd = pPort->pAd;
    
    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]\n", __FUNCTION__, pPort->PortNumber));
    
    PlatformMoveMemory(&pPort->CommonCfg.RegTransmitSetting, &pAd->HwCfg.HwSupportCapabilities.MaxRegTransmitSetting, sizeof(REG_TRANSMIT_SETTING));
    pPort->CommonCfg.PhyMode = pAd->HwCfg.HwSupportCapabilities.MaxPhyMode;
    pPort->CommonCfg.BACapability.word = pAd->HwCfg.HwSupportCapabilities.MaxBACapability.word;
    pPort->CommonCfg.bWmmCapable = pAd->HwCfg.HwSupportCapabilities.bWmmCapable;

    pPort->CommonCfg.InitPhyMode = pPort->CommonCfg.PhyMode;
    pPort->CommonCfg.LastPhyMode = pPort->CommonCfg.PhyMode;

    pPort->CommonCfg.CountryRegion = pAd->RegistryCfg.CountryRegion;

    FUNC_LEAVE;
}


/*
    ========================================================================
    
    Routine Description:
        Read the following from the registry
        1. All the parameters
        2. NetworkAddres

    Arguments:
        Adapter                     Pointer to our adapter
        WrapperConfigurationContext For use by NdisOpenConfiguration

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_FAILURE
        NDIS_STATUS_RESOURCES

    IRQL = PASSIVE_LEVEL

    Note:
    
    ========================================================================
*/
NDIS_STATUS NdisInitNICReadRegParameters(
    IN  PMP_ADAPTER       pAd
    )
{
    NDIS_STATUS                     ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE                     ConfigurationHandle = NULL;
    NDIS_CONFIGURATION_OBJECT       ConfigObject;
    PRTMP_REG_ENTRY                 pRegEntry;
    NDIS_PARAMETER_TYPE             ParamType;
    NDIS_CONFIGURATION_PARAMETER    NdisConfigurationParameter; 
    PNDIS_CONFIGURATION_PARAMETER   pReturnedValue;
    UCHAR                           i,j;
    UINT                            value;
    UINT                            Length;
    CHAR                            AnsiBuf[MAX_LEN_OF_VENDOR_DESC + 1]; // NULL Terminated
    CHAR                            AnsiBuf_1[MAX_LEN_OF_VENDOR_DESC + 1]; // NULL Terminated
    ANSI_STRING                     AnsiStr;
    ANSI_STRING                     AnsiStr_BAK;
    PUCHAR                          pointer;
    BOOLEAN                         bTxKey = FALSE;
    //ULONG                         IV;
    UCHAR                           CurrentAddress[MAC_ADDR_LEN];      // User changed MAC address
    PUSB_DEVICE_CONTEXT                 usbDeviceContext;
    PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    DBGPRINT(RT_DEBUG_TRACE, ("==> %s \n",__FUNCTION__));
    
    ConfigObject.Header.Type = NDIS_OBJECT_TYPE_CONFIGURATION_OBJECT;
    ConfigObject.Header.Revision = NDIS_CONFIGURATION_OBJECT_REVISION_1;
    ConfigObject.Header.Size = sizeof( NDIS_CONFIGURATION_OBJECT );
    ConfigObject.NdisHandle = pAd->AdapterHandle;
    ConfigObject.Flags = 0;

    pReturnedValue = &NdisConfigurationParameter;
#pragma prefast(suppress: __WARNING_MEMORY_LEAK, " should not have memmory leak")   
    ndisStatus = NdisOpenConfigurationEx(&ConfigObject, &ConfigurationHandle);

    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT_ERR(("NdisOpenConfigurationEx failed\n"));
        return ndisStatus;
    }

    if ((ndisStatus == NDIS_STATUS_SUCCESS) && (ConfigurationHandle != NULL))
    {
        // Init local strings buffer
        INIT_NDIS_STR(LocalStrings.Ssid,        &LocalStrings.SsidBuf[0],       64);
        INIT_NDIS_STR(LocalStrings.VendorDesc,  &LocalStrings.VendorDescBuf[0], 128);
        INIT_NDIS_STR(LocalStrings.Key0,        &LocalStrings.KeyBuf0[0],       128);
        INIT_NDIS_STR(LocalStrings.Key1,        &LocalStrings.KeyBuf1[0],       128);
        INIT_NDIS_STR(LocalStrings.Key2,        &LocalStrings.KeyBuf2[0],       128);
        INIT_NDIS_STR(LocalStrings.Key3,        &LocalStrings.KeyBuf3[0],       128);
        INIT_NDIS_STR(LocalStrings.MAC,         &LocalStrings.LocalMacAddr[0],  64);
        INIT_NDIS_STR(LocalStrings.WPSDevName,  &LocalStrings.WPSDevNameBuf[0], 64);
        INIT_NDIS_STR(LocalStrings.CountryCodeString,   &LocalStrings.CountryCodeStringBuf[0],  64);
        INIT_NDIS_STR(LocalStrings.BaldEagleSSID, &LocalStrings.BaldEagleSSIDBuf[0],    64);
    
        // Read all registry values
        for (i = 0, pRegEntry = NICRegTable; i < NIC_NUM_REG_PARAMS; i++, pRegEntry++)
        {
            // Get the configuration value for a specific parameter.  Under NT the
            // parameters are all read in as DWORDs.
            ParamType = (NDIS_PARAMETER_TYPE) pRegEntry->Type;
    
            if (ParamType == NdisParameterString)
            {
                // If it is string, we have to copy to local strings, and convert to ansi strings.
                pointer = ((PUCHAR) &LocalStrings) + pRegEntry->FieldOffset;
            }
            else
            {
                pointer = (PUCHAR) pAd + pRegEntry->FieldOffset;
            }
    
            //
            // pReturnedValue will not be updated while there were failure readed.
            // So we should reset the value before reading.
            //
            PlatformZeroMemory(pReturnedValue, sizeof(NDIS_CONFIGURATION_PARAMETER));
    
            NdisReadConfiguration(
                        &ndisStatus,
                        &pReturnedValue,
                        ConfigurationHandle,
                        &pRegEntry->RegName,
                        ParamType);
    
            //
            // Right now, only interger type supported,
            // Support for string and other types will be added later
            //
            // If the parameter was present, then check its value for validity.
            //
            if (ndisStatus == NDIS_STATUS_SUCCESS)
            {
                // Check that param value is not too small or too large
                if (pReturnedValue->ParameterData.IntegerData < pRegEntry->Min ||
                    pReturnedValue->ParameterData.IntegerData > pRegEntry->Max)
                {
                    value = pRegEntry->Default;
                }
                else
                {
                    value = pReturnedValue->ParameterData.IntegerData;
                }
            }
            else if (pRegEntry->bRequired)
            {
                DBGPRINT_RAW(RT_DEBUG_ERROR, (" -- failed\n"));
    
                ASSERT(FALSE);
    
                ndisStatus = NDIS_STATUS_FAILURE;
                break;
            }
            else
            {
                value = pRegEntry->Default;
                DBGPRINT_RAW(RT_DEBUG_INFO, ("= 0x%x (default)\n", value));
                ndisStatus = NDIS_STATUS_SUCCESS;
            }
    
            // Store the value in the adapter structure.
            switch(pRegEntry->FieldSize)
            {
                case 1:
                    *((PUCHAR) pointer) = (UCHAR) value;
                    break;
    
                case 2:
                    *((PUSHORT) pointer) = (USHORT) value;
                    break;
    
                case 4:
                    *((PULONG) pointer) = (ULONG) value;
                    break;
    
                default:
                    if (ParamType == NdisParameterString)
                        RtlCopyUnicodeString((PNDIS_STRING) pointer, &pReturnedValue->ParameterData.StringData);
                    else
                        DBGPRINT(RT_DEBUG_ERROR, ("Bogus field size %d\n", pRegEntry->FieldSize));
                    break;
            }
        }

        CpoyRegistryToDriverDefaultSetting(pAd);

        UpdatePortDefaultSetting(pAd->PortList[FXXK_PORT_0]);
        
        
        //A patch to fix: Un-updated RaCoInst.dll file, and wirelessmode will not updated (wirelessmode=0).
        if (pPort->CommonCfg.PhyMode == PHY_11BG_MIXED)
        {
            if(IS_DUAL_BAND_NIC(pAd))
            {
                if(VHT_NIC(pAd))
                {
                    pPort->CommonCfg.PhyMode = PHY_11VHT;
                }
                else
                {
                    pPort->CommonCfg.PhyMode = PHY_11ABGN_MIXED;
                }
            }
            else
            {
                pPort->CommonCfg.PhyMode = PHY_11BGN_MIXED;
            }
            
        }
        
        if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
        {
            pPort->CommonCfg.DesiredHtPhy.bHtEnable = TRUE;
            
            if (pPort->CommonCfg.PhyMode == PHY_11VHT)
                pPort->CommonCfg.DesiredVhtPhy.bVhtEnable = TRUE;
        }
    
        DBGPRINT(RT_DEBUG_TRACE, ("Registry: WSCVersion2: 0x%x\n", pAd->StaCfg.WSCVersion2));
        DBGPRINT(RT_DEBUG_TRACE, ("Registry: WSC20Testbed: 0x%08x\n", pAd->StaCfg.WSC20Testbed));
    
        if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("We are testing WHQL now!!!\n Set BA Capability as BA-not use, and AutoBA = 0"));
            pPort->CommonCfg.BACapability.field.AutoBA = 1;           // enable/disable BA
            pPort->CommonCfg.BACapability.field.Policy = IMMED_BA;    // IMMED_BA(1)/DLEAY_BA(1)
        }
        
        pAd->StaCfg.WSCVersion2Tmp = pAd->StaCfg.WSCVersion2;
        if(IS_ENABLE_WSC20TB_Version57(pAd))
        {
            pAd->StaCfg.WSCVersion2 = 0x57;
            DBGPRINT(RT_DEBUG_TRACE, ("WscTestbed enable version 57, WSC Version2 = 0x%x\n", pAd->StaCfg.WSCVersion2));
        }
        if(pAd->StaCfg.WSCVersion2 < 0x20)
        {
            pAd->StaCfg.WSC20Testbed = pAd->StaCfg.WSC20Testbed | (0x00000004);
        }
    
        DBGPRINT(RT_DEBUG_TRACE, (("P2pControl = %x\n"), pPort->CommonCfg.P2pControl.word));
    
        pPort->CommonCfg.SupportedHtPhy.MpduDensity = (UCHAR)pPort->CommonCfg.BACapability.field.MpduDensity;
        if (pPort->CommonCfg.BACapability.field.RxBAWinLimit > Max_RX_REORDERBUF)
            pPort->CommonCfg.BACapability.field.RxBAWinLimit = Max_RX_REORDERBUF;
    
        DBGPRINT(RT_DEBUG_TRACE, ((" PhyMode=%d, BW =%d  MCS =%d  ShortGI =%d  STBC =%d\n"), 
            pPort->CommonCfg.PhyMode, 
            READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg), 
            READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg),  
            READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg), 
            READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg)));
    
        DBGPRINT(RT_DEBUG_TRACE, ("Registry: BACapability = 0x%x. MpduDensity=%d, RxBAWinLimit= 0x%x. TxBAWinLimit= 0x%x, b2040CoexistScanSup = %d\n", 
                pPort->CommonCfg.BACapability.word,
                pPort->CommonCfg.BACapability.field.MpduDensity,
                pPort->CommonCfg.BACapability.field.RxBAWinLimit,
                pPort->CommonCfg.BACapability.field.TxBAWinLimit,
                pPort->CommonCfg.BACapability.field.b2040CoexistScanSup));    
    
        DBGPRINT(RT_DEBUG_TRACE, ("Registry: TransmitSetting = 0x%x. PhyMode= 0x%x, MCS= 0x%x, BW= 0x%x, SHORTGI= 0x%x, STBC= 0x%x, TRANSNO= 0x%x,  HTMODE= 0x%x. Extension channel= 0x%x.\n", 
                pPort->CommonCfg.RegTransmitSetting.word,
                pPort->CommonCfg.RegTransmitSetting.field.PhyMode,
                pPort->CommonCfg.RegTransmitSetting.field.MCS,
                pPort->CommonCfg.RegTransmitSetting.field.BW,
                pPort->CommonCfg.RegTransmitSetting.field.ShortGI,
                pPort->CommonCfg.RegTransmitSetting.field.STBC,
                pPort->CommonCfg.RegTransmitSetting.field.TRANSNO,
                pPort->CommonCfg.RegTransmitSetting.field.HTMODE,
                pPort->CommonCfg.RegTransmitSetting.field.EXTCHA));
    
        DBGPRINT(RT_DEBUG_TRACE, ("MaxPktOneTxBulk=%d, TxBulkFactor=%d, RxBulkFactor=%d,\n",
            pPort->CommonCfg.MaxPktOneTxBulk, pPort->CommonCfg.TxBulkFactor ,pPort->CommonCfg.RxBulkFactor)); 
    

        // Thermal Solution Enable
        DBGPRINT(RT_DEBUG_TRACE, ("Registry: ThermalEnable = %d\n", pPort->CommonCfg.ThermalEnable));

        //
        // For Vista, default we don't want to reconnect to any SSID
        //
        pAd->MlmeAux.AutoReconnectSsidLen= 32;
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
    
    
        AnsiStr.Length        = 0;
        AnsiStr.MaximumLength = MAX_LEN_OF_VENDOR_DESC + 1; // NULL terminated
        AnsiStr.Buffer        = AnsiBuf;
    
        ndisStatus =  NdisUnicodeStringToAnsiString(&AnsiStr, &LocalStrings.VendorDesc);
        if (AnsiStr.Length > 0 && (ndisStatus == NDIS_STATUS_SUCCESS))
        {
            PlatformZeroMemory(pAd->pNicCfg->VendorDesc, MAX_LEN_OF_VENDOR_DESC);  // NOT NULL-terminated
            pAd->pNicCfg->VendorDescLen = (UCHAR) AnsiStr.Length;
            PlatformMoveMemory(pAd->pNicCfg->VendorDesc, &AnsiStr.Buffer[0], AnsiStr.Length);
        }
        else    // Use default
        {
            pAd->pNicCfg->VendorDescLen = (UCHAR) NIC_VENDOR_DESC_LEN;
            PlatformMoveMemory(pAd->pNicCfg->VendorDesc, NIC_VENDOR_DESC, NIC_VENDOR_DESC_LEN);
        }
        DBGPRINT(RT_DEBUG_TRACE, ("Vendor Description (Len=%d, VendorDesc=%s...)\n", pAd->pNicCfg->VendorDescLen, pAd->pNicCfg->VendorDesc));
    
        //
        // Update bSwRadioOff to bSwRadio.
        // From now on, we will use pAd->StaCfg.bSwRadio to determine SwRadio status.
        //
        pAd->StaCfg.bSwRadio = pAd->StaCfg.bSwRadioOff;
        
        // Warning, invert the read register result because the definition of Radio is
        // 0: Enable, 1: disable
        if(pAd->OpMode == OPMODE_AP)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Soft AP Mode, Software Radio is always on\n"));
            pAd->StaCfg.bSwRadio = TRUE;
        }
        else
        {
            if (pAd->StaCfg.bSwRadio == 1)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Software Radio is off\n"));
                pAd->StaCfg.bSwRadio = FALSE;
                
                // Update extra information to link is up
                pAd->UiCfg.ExtraInfo = SW_RADIO_OFF;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Software Radio is on\n"));
                pAd->StaCfg.bSwRadio = TRUE;
            }
        }
    
        AnsiStr_BAK.Length        = 0;
        AnsiStr_BAK.MaximumLength = MAX_LEN_OF_VENDOR_DESC + 1; // NULL terminated
        AnsiStr_BAK.Buffer        = AnsiBuf_1;
    
    
        NdisUnicodeStringToAnsiString(&AnsiStr_BAK, &LocalStrings.MAC);
        NdisUnicodeStringToAnsiString(&AnsiStr, &LocalStrings.MAC);
        // MAC assress at least 12 byte chars 
        //filter out '-' char for ndits test (WLK 1.2)
        j = 0;
        if( AnsiStr_BAK.Length >= 12)
        {   
            for(i = 0; i < AnsiStr_BAK.Length; i++)
            {
                if(isxdigit((unsigned char)AnsiStr_BAK.Buffer[i]))
                {
                    AnsiStr.Buffer[j] = AnsiStr_BAK.Buffer[i];
                    j++;    
                }
            }
        }
        AnsiStr.Length = j;
        // Read NetworkAddress
        Length = (UCHAR) AnsiStr.Length / 2;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("MAC string len=%d\n",AnsiStr.Length));       
    
        if (Length == MAC_ADDR_LEN)
        {
            AtoH(&AnsiStr.Buffer[0], CurrentAddress, MAC_ADDR_LEN);
            //do not set broadcast address amd grobal administrator address
            //byte[0] in MAC address : bit 0: 1 multicast address
            //                       bit 1:  0: global administrator address
            //                                              1: local administrator address
             
            if( (CurrentAddress[0] & 0x1) || ((CurrentAddress[0] & 0x2) == 0))
            {
                pAd->HwCfg.bLocalAdminMAC = FALSE;    
            }
            else
            {
                pAd->HwCfg.bLocalAdminMAC = TRUE;
                PlatformMoveMemory(pAd->HwCfg.CurrentAddress,CurrentAddress,MAC_ADDR_LEN);
            }
            
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Local MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                pAd->HwCfg.CurrentAddress[0], pAd->HwCfg.CurrentAddress[1], pAd->HwCfg.CurrentAddress[2],
                pAd->HwCfg.CurrentAddress[3], pAd->HwCfg.CurrentAddress[4], pAd->HwCfg.CurrentAddress[5]));       
        }
    
        // 
        // Read and parse Bald Eagle SSID
        //
        AnsiStr.MaximumLength = MAX_LEN_OF_SSID + 1; // NULL terminated, should be 33
        AnsiStr.Buffer        = AnsiBuf;
        
        ndisStatus =  NdisUnicodeStringToAnsiString(&AnsiStr, &LocalStrings.BaldEagleSSID);
    
        // Registry BaldEagleSSID is valid, this STA can only connect to a specific SSID which is set in registry "BaldEagleSSID".
        if ((AnsiStr.Length > 0) &&
            (AnsiStr.Length <= MAX_LEN_OF_SSID) &&
            (ndisStatus == NDIS_STATUS_SUCCESS))
        {
            pAd->StaCfg.BaldEagle.EnableBladEagle = TRUE;
            pAd->StaCfg.BaldEagle.SsidLen = (UCHAR) AnsiStr.Length;
            PlatformMoveMemory(pAd->StaCfg.BaldEagle.Ssid, &AnsiStr.Buffer[0], AnsiStr.Length);
            DBGPRINT(RT_DEBUG_TRACE,("Bald eagle mechanism is ENABLED, the STA can only connect to AP called \"%s\",length = %d\n"
                ,pAd->StaCfg.BaldEagle.Ssid, pAd->StaCfg.BaldEagle.SsidLen));
        }
        else
        {
            pAd->StaCfg.BaldEagle.EnableBladEagle = FALSE;
            DBGPRINT(RT_DEBUG_TRACE,("Bald Eagle mechanism is not activated since registry \"BaldEagleSSID\" is not exist or the value is empty\n"));
        }
    
        //
        // Read WPS Device Name
        //
        AnsiStr.Length        = 0;
        AnsiStr.MaximumLength = 32 + 1; // NULL terminated
        AnsiStr.Buffer        = AnsiBuf;
    
        NdisUnicodeStringToAnsiString(&AnsiStr, &LocalStrings.WPSDevName);
        if (AnsiStr.Length > 0)
        {
            PlatformZeroMemory(pAd->WpsCfg.WPSDevName, 32);  // NOT NULL-terminated
            pAd->WpsCfg.WPSDevNameLen = (UCHAR) AnsiStr.Length;
            PlatformMoveMemory(pAd->WpsCfg.WPSDevName, &AnsiStr.Buffer[0], AnsiStr.Length);
        }
        else    // Use default
        {
            pAd->WpsCfg.WPSDevNameLen = (UCHAR)NIC_WPS_DESC_LEN;
            PlatformMoveMemory(pAd->WpsCfg.WPSDevName, NIC_WPS_DESC, pAd->WpsCfg.WPSDevNameLen);
        }
        DBGPRINT(RT_DEBUG_TRACE, ("WPS Device Name (Len=%d, WPSDevName=%s...)\n", pAd->WpsCfg.WPSDevNameLen, pAd->WpsCfg.WPSDevName));
    
        //
        // Disable WMM if we can't support on this device.
        //
        if (pPort->CommonCfg.bWmmCapable)
        {
            //
            // For supporting WMM, the NumberOfPipes must large than 5
            // 1: for BulkIn, 4: for VI,VE,BE,BK Bulk out.
            //
            usbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
            if ( usbDeviceContext->NumberOfPipes < 5)
                pPort->CommonCfg.bWmmCapable = FALSE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("Registry: WmmCapable=%d. RDG=%d. \n", pPort->CommonCfg.bWmmCapable, pPort->CommonCfg.bRdg));
    
        //Init Ba CApability parameters.
        pPort->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pPort->CommonCfg.BACapability.field.MpduDensity;
        pPort->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pPort->CommonCfg.BACapability.field.AmsduEnable;
        pPort->CommonCfg.DesiredHtPhy.AmsduSize= (USHORT)pPort->CommonCfg.BACapability.field.AmsduSize;
        pPort->CommonCfg.DesiredHtPhy.MimoPs= (USHORT)pPort->CommonCfg.BACapability.field.MMPSmode;
    
        // UPdata to HT IE
        pPort->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pPort->CommonCfg.BACapability.field.MMPSmode;
        pPort->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pPort->CommonCfg.BACapability.field.AmsduSize;
        pPort->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pPort->CommonCfg.BACapability.field.MpduDensity;
    
        pPort->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 2;
    
        if (pAd->pNicCfg->bMultipleIRP)
            pAd->pNicCfg->NumOfBulkInIRP = MULTIPLE_BULKIN_NUM;
        else
            pAd->pNicCfg->NumOfBulkInIRP = 1;

            
        DBGPRINT(RT_DEBUG_TRACE, ("NumOfBulkInIRP=%d, MpduDensity = %d\n", pAd->pNicCfg->NumOfBulkInIRP, pPort->CommonCfg.HtCapability.HtCapParm.MpduDensity));
    
        pPort->CommonCfg.REGBACapability.word = pPort->CommonCfg.BACapability.word;

        DBGPRINT(RT_DEBUG_TRACE, ("bAdhocNMode Support = [%d]\n",pAd->StaCfg.bAdhocNMode));
    
        pAd->StaCfg.bInitAdhocNMode = pAd->StaCfg.bAdhocNMode;
        
        // USB 1.1 will disable Ralink Tx Aggregation
        if (pAd->pHifCfg->BulkInMaxPacketSize == 64)
            pPort->CommonCfg.bAggregationCapable = FALSE;
    
        // Save it. Linkdown function will use it. We will disable RalinkTxAggregation when having Priority-Packets in RTXmitSendPacket.
        pAd->pHifCfg->bUsbTxBulkAggre = pPort->CommonCfg.bAggregationCapable;  
        
       DBGPRINT(RT_DEBUG_TRACE, ("bAggregationCapable = [%d]\n", pPort->CommonCfg.bAggregationCapable));
        // Close the registry
        
        NdisCloseConfiguration(ConfigurationHandle);

        DBGPRINT(RT_DEBUG_TRACE, ("%s SmartScan = [%d]\n", __FUNCTION__, pAd->StaCfg.SmartScan));
        DBGPRINT(RT_DEBUG_TRACE, ("%s FastRoamingSensitivity = [%d]\n", __FUNCTION__, pAd->StaCfg.FastRoamingSensitivity));
        
        //
        // correspond the sensitivity degree to dBm value to get the fast-roaming threshold
        //
        switch (pAd->StaCfg.FastRoamingSensitivity)
        {
            case LOWEST_SENSITIVITY:
                pAd->StaCfg.CCXControl.field.FastRoamEnable = 1;
                pAd->StaCfg.CCXControl.field.dBmToRoam = LOWEST_SENSITIVITY_DBM;
                break;
    
            case MEDIUM_LOW_SENSITIVITY:
                pAd->StaCfg.CCXControl.field.FastRoamEnable = 1;
                pAd->StaCfg.CCXControl.field.dBmToRoam = MEDIUM_LOW_SENSITIVITY_DBM;
                break;
    
            case MEDIUM_SENSITIVITY:
                pAd->StaCfg.CCXControl.field.FastRoamEnable = 1;
                pAd->StaCfg.CCXControl.field.dBmToRoam = MEDIUM_SENSITIVITY_DBM;
                break;
    
            case MEDIUM_HIGH_SENSITIVITY:
                pAd->StaCfg.CCXControl.field.FastRoamEnable = 1;
                pAd->StaCfg.CCXControl.field.dBmToRoam = MEDIUM_HIGH_SENSITIVITY_DBM;
                break;
    
            case HIGHEST_SENSITIVITY:
                pAd->StaCfg.CCXControl.field.FastRoamEnable = 1;
                pAd->StaCfg.CCXControl.field.dBmToRoam = HIGHEST_SENSITIVITY_DBM;
                break;
    
            default:
                pAd->StaCfg.CCXControl.field.FastRoamEnable = 0;
        }
    
        DBGPRINT(RT_DEBUG_TRACE, ("%s: pPort->CommonCfg.GpioPinSharingMode = %d\n", 
            __FUNCTION__, 
            pPort->CommonCfg.GpioPinSharingMode));
    
        DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->StaCfg.PSControl.word = 0x%X\n", 
            __FUNCTION__, 
            pAd->StaCfg.PSControl.word));
    
        DBGPRINT(RT_DEBUG_TRACE, ("%s: bPPAD = %d\n", 
            __FUNCTION__, 
            pPort->CommonCfg.bPPAD));
    
        DBGPRINT(RT_DEBUG_TRACE, ("%s: CalCtrlRegistry = 0x%08X, CalCtrlRegistry.EnableInternalAlc = %d, CalCtrlRegistry.EnableCoefficientAlcRefValue = %d\n", 
            __FUNCTION__, 
            pPort->CommonCfg.CalCtrlRegistry, 
            pPort->CommonCfg.CalCtrlRegistry.EnableInternalAlc, 
            pPort->CommonCfg.CalCtrlRegistry.EnableCoefficientAlcRefValue));
    
        DBGPRINT(RT_DEBUG_TRACE, ("<== %s . bEnableTxBurst= %d. \n", __FUNCTION__, pPort->CommonCfg.bEnableTxBurst));
    
        DBGPRINT(RT_DEBUG_TRACE, ("%s: BandedgePowerTableEnable = %d\n", 
            __FUNCTION__, 
            pPort->CommonCfg.BandedgePowerTableEnable));
        
    }
    else
    {
        DBGPRINT_ERR(("NdisOpenConfigurationEx failed\n"));
    }
    
    return ndisStatus;
}
 
NDIS_STATUS RTUSBWriteHWMACAddress(
    IN  PMP_ADAPTER       pAd)
{
    MAC_DW0_STRUC       StaMacReg0;
    MAC_DW1_STRUC       StaMacReg1;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    LARGE_INTEGER       NOW;
    PMP_PORT          pPort = pAd->PortList[PORT_0];
    // initialize the random number generator
    NdisGetCurrentSystemTime(&NOW);
    
    if (pAd->HwCfg.bLocalAdminMAC != TRUE)
    {
        pAd->HwCfg.CurrentAddress[0] = pAd->HwCfg.PermanentAddress[0];
        pAd->HwCfg.CurrentAddress[1] = pAd->HwCfg.PermanentAddress[1];
        pAd->HwCfg.CurrentAddress[2] = pAd->HwCfg.PermanentAddress[2];
        pAd->HwCfg.CurrentAddress[3] = pAd->HwCfg.PermanentAddress[3];
        pAd->HwCfg.CurrentAddress[4] = pAd->HwCfg.PermanentAddress[4];
        pAd->HwCfg.CurrentAddress[5] = pAd->HwCfg.PermanentAddress[5];
    }
    COPY_MAC_ADDR(pPort->CurrentAddress, pAd->HwCfg.CurrentAddress);
    // Write New MAC address to MAC_CSR2 & MAC_CSR3 & let ASIC know our new MAC
    StaMacReg0.field.Byte0 = pAd->HwCfg.CurrentAddress[0];
    StaMacReg0.field.Byte1 = pAd->HwCfg.CurrentAddress[1];
    StaMacReg0.field.Byte2 = pAd->HwCfg.CurrentAddress[2];
    StaMacReg0.field.Byte3 = pAd->HwCfg.CurrentAddress[3];
    StaMacReg1.field.Byte4 = pAd->HwCfg.CurrentAddress[4];
    StaMacReg1.field.Byte5 = pAd->HwCfg.CurrentAddress[5];
    StaMacReg1.field.U2MeMask = 0x1;
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("Local MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
            pAd->HwCfg.CurrentAddress[0], pAd->HwCfg.CurrentAddress[1], pAd->HwCfg.CurrentAddress[2],
            pAd->HwCfg.CurrentAddress[3], pAd->HwCfg.CurrentAddress[4], pAd->HwCfg.CurrentAddress[5]));

    RTUSBWriteMACRegister(pAd, MAC_ADDR_DW0, StaMacReg0.word);
    RTUSBWriteMACRegister(pAd, MAC_ADDR_DW1, StaMacReg1.word);

    RTUSBWriteMACRegister(pAd, MAC_BSSID_DW0, StaMacReg0.word);
    RTUSBWriteMACRegister(pAd, MAC_BSSID_DW1,  StaMacReg1.word);

    return Status;
}

/*
    ========================================================================
    
    Routine Description:
        Read initial parameters from EEPROM
        
    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    Note:
        
    ========================================================================
*/
VOID    NICReadEEPROMParameters(
    IN  PMP_ADAPTER   pAd)
{
    UINT32  Offset;
    UINT8   Value[16];
    UINT8   Byte34;
    UINT8   Byte42;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    // need to do this step by host driver. After loading FW, FW will not initialize setting from EFuse.
    // It is used for co-exist with buffer mode for embedded system.
    SendEFuseBufferModeCMD(pAd, EEPROM_MODE_EFUSE);

    // byte 0~1 : Chip ID   byte 2~3 : EEPROM Version   byte 4~9 : WLAN Mac Address  
    // byte a~b : Device ID  byte c~d : Vendor ID  byte e~f : Reserved
    Offset = 0x0;
    EFUSE_IO_READ128(pAd, Offset, (UINT8 *)&Value);
    //DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Value = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, Value[0], Value[1], Value[2], Value[3], Value[4], Value[5], Value[6], Value[7], Value[8], Value[9], Value[10], Value[11], Value[12], Value[13], Value[14], Value[15]));   

    pAd->HwCfg.PermanentAddress[0] = Value[4];
    pAd->HwCfg.PermanentAddress[1] = Value[5];
    pAd->HwCfg.PermanentAddress[2] = Value[6];
    pAd->HwCfg.PermanentAddress[3] = Value[7];
    pAd->HwCfg.PermanentAddress[4] = Value[8];
    pAd->HwCfg.PermanentAddress[5] = Value[9];

    pAd->HwCfg.CurrentAddress[0] = Value[4];
    pAd->HwCfg.CurrentAddress[1] = Value[5];
    pAd->HwCfg.CurrentAddress[2] = Value[6];
    pAd->HwCfg.CurrentAddress[3] = Value[7];
    pAd->HwCfg.CurrentAddress[4] = Value[8];
    pAd->HwCfg.CurrentAddress[5] = Value[9];

    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Chip ID = %02x%02x\n", __FUNCTION__, Value[1], Value[0]));    
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, EEPROM Version = %d.%d\n", __FUNCTION__, Value[3], Value[2]));    
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Mac Address = %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, Value[4], Value[5], Value[6], Value[7], Value[8], Value[9]));   

    // byte 39 : Country Region
    Offset = 0x30;
    EFUSE_IO_READ128(pAd, Offset, (UINT8 *)&Value);
   // DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Value = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, Value[0], Value[1], Value[2], Value[3], Value[4], Value[5], Value[6], Value[7], Value[8], Value[9], Value[10], Value[11], Value[12], Value[13], Value[14], Value[15]));   

    // Country Regsion init order : 1. from Registry.  2. from EFuse
    if (Value[9] != 0xFF)
    {
        pPort->CommonCfg.CountryRegion = Value[9];
    }

    // use for later if need
    Byte34 = Value[4];
    pAd->HwCfg.Antenna.field.TxPath = ((Byte34 & 0xf0) >> 4);
    pAd->HwCfg.Antenna.field.RxPath = (Byte34 & 0x0f);
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, pPort->CommonCfg.CountryRegion = %d\n", __FUNCTION__, pPort->CommonCfg.CountryRegion));   

    // byte 42 : number of Tx/Rx stream
    Offset = 0x40;
    EFUSE_IO_READ128(pAd, Offset, (UINT8 *)&Value);
    //DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Value = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, Value[0], Value[1], Value[2], Value[3], Value[4], Value[5], Value[6], Value[7], Value[8], Value[9], Value[10], Value[11], Value[12], Value[13], Value[14], Value[15]));   

    Byte42 = Value[2];
    if (Byte42 == 0xFF) // 0xFF means that based on the path setting byte 34
    {
        pPort->CommonCfg.TxStream = (UCHAR)pAd->HwCfg.Antenna.field.TxPath;
        pPort->CommonCfg.RxStream = (UCHAR)pAd->HwCfg.Antenna.field.RxPath;

        pAd->HwCfg.NicConfig3.field.TxStream = pAd->HwCfg.Antenna.field.TxPath;
        pAd->HwCfg.NicConfig3.field.RxStream = pAd->HwCfg.Antenna.field.RxPath;
    }
    else
    {
        pPort->CommonCfg.TxStream = ((Byte42 & 0xf0) >> 4);
        pPort->CommonCfg.RxStream = (Byte42 & 0x0f);

        pAd->HwCfg.NicConfig3.word = Byte42;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Tx Stream = %d, Rx Stream = %d, Tx Antenna=%d, Rx Antenna=%d\n", __FUNCTION__, pAd->HwCfg.NicConfig3.field.TxStream, pAd->HwCfg.NicConfig3.field.RxStream, pAd->HwCfg.Antenna.field.TxPath, pAd->HwCfg.Antenna.field.RxPath));  

    // Set Tx Stream
    MtAsicSetTxStream(pAd, pPort->CommonCfg.TxStream);

    // Set Rx Stream
    MtAsicSetRxStream(pAd, pPort->CommonCfg.RxStream);
    
/*
    Offset = 0x10;
    EFUSE_IO_READ128(pAd, Offset, (UINT8 *)&Value);
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Value = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, Value[0], Value[1], Value[2], Value[3], Value[4], Value[5], Value[6], Value[7], Value[8], Value[9], Value[10], Value[11], Value[12], Value[13], Value[14], Value[15])); 

    Offset = 0x20;
    EFUSE_IO_READ128(pAd, Offset, (UINT8 *)&Value);
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Value = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, Value[0], Value[1], Value[2], Value[3], Value[4], Value[5], Value[6], Value[7], Value[8], Value[9], Value[10], Value[11], Value[12], Value[13], Value[14], Value[15])); 

    Offset = 0x30;
    EFUSE_IO_READ128(pAd, Offset, (UINT8 *)&Value);
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Value = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, Value[0], Value[1], Value[2], Value[3], Value[4], Value[5], Value[6], Value[7], Value[8], Value[9], Value[10], Value[11], Value[12], Value[13], Value[14], Value[15])); 
*/
}

VOID NicReadEepromForIqCompensation(
    IN PMP_ADAPTER pAd)
{
}

/*
    ========================================================================
    
    Routine Description:
        Set default value from EEPROM
        
    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    NICInitAsicFromEEPROM(
    IN  PMP_ADAPTER   pAd)
{
    ULONG                   data;
    USHORT                  i;
    EEPROM_NIC_CONFIG2_STRUC    NicConfig2;
    //UCHAR BBPR1, BBPR3;
    USHORT  MacAddess;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitAsicFromEEPROM\n"));

    for(i = EEPROM_BBP_ARRAY_OFFSET; i < NUM_EEPROM_BBP_PARMS; i++)
    {
        UCHAR BbpRegIdx, BbpValue;
    
        if ((pAd->HwCfg.EEPROMDefaultValue[i] != 0xFFFF) && (pAd->HwCfg.EEPROMDefaultValue[i] != 0))
        {
            BbpRegIdx = (UCHAR)(pAd->HwCfg.EEPROMDefaultValue[i] >> 8);
            BbpValue  = (UCHAR)(pAd->HwCfg.EEPROMDefaultValue[i] & 0xff);
            RTUSBWriteBBPRegister(pAd, BbpRegIdx, BbpValue);
        }
    }

    NicConfig2.word = pAd->HwCfg.EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];
    if ((NicConfig2.word & 0x00ff) == 0xff)
    {
        NicConfig2.word &= 0xff00;
    }

    if ((NicConfig2.word >> 8) == 0xff)
    {
        NicConfig2.word &= 0x00ff;
    }

    // Save the antenna for future use
    pAd->HwCfg.NicConfig2.word = NicConfig2.word;
    
    if (pAd->HwCfg.LedCntl.word  == 0xff)
    {
        pAd->HwCfg.LedCntl.word = 01;
        pAd->HwCfg.Led1 = 0x5555;
        pAd->HwCfg.Led2 = 0x2221;
        pAd->HwCfg.Led3 = 0x5627;
    }

    //
    // LED Control by INF file.
    //
    if (pAd->HwCfg.LedControlBySW == TRUE)
    {
        pAd->HwCfg.LedCntl.word = pAd->HwCfg.INFLedCntl;
        pAd->HwCfg.Led1 = pAd->HwCfg.INFLed1;
        pAd->HwCfg.Led2 = pAd->HwCfg.INFLed2;
        pAd->HwCfg.Led3 = pAd->HwCfg.INFLed3;
    }
    AsicSendCommanToMcu(pAd, 0x52, 0xff, (UCHAR)pAd->HwCfg.Led1, (UCHAR)(pAd->HwCfg.Led1 >> 8));
    AsicSendCommanToMcu(pAd, 0x53, 0xff, (UCHAR)pAd->HwCfg.Led2, (UCHAR)(pAd->HwCfg.Led2 >> 8));
    AsicSendCommanToMcu(pAd, 0x54, 0xff, (UCHAR)pAd->HwCfg.Led3, (UCHAR)(pAd->HwCfg.Led3 >> 8));
    AsicSendCommanToMcu(pAd, 0x51, 0xff, 0, pAd->HwCfg.LedCntl.field.Polarity);

    pAd->HwCfg.LedIndicatorStregth = 0xFF;
    LedCtrlSetSignalLed(pAd, -100);    // Force signal strength Led to be turned off, before link up

    pPort->StaCfg.WscControl.ConfigMethods = 0x008C;
    if(NicConfig2.field.EnableWPSPBC)
    {
        pPort->StaCfg.WscControl.ConfigMethods  = cpu2be16(0x268c);   // Label, Display, Virtul/Physical PBC, Virtual Display PIN
    }
    else
    {
        pPort->StaCfg.WscControl.ConfigMethods  = cpu2be16(0x228c);   // Label, Display, Virtul/Physical PBC, Virtual Display PIN
    }
    
    // Prepare carrier detection material
    if (pPort->CommonCfg.bCarrierDetect)
    {
        MacAddess = H2M_BBP_AGENT;
        
        RTUSBWriteMACRegister(pAd, MacAddess, 0);   // initialize BBP R/W access agent
        RTUSBWriteMACRegister(pAd, H2M_MAILBOX_CSR, 0);
        RTUSBWriteMACRegister(pAd, H2M_INT_SRC, 0);
        AsicSendCommanToMcu(pAd, 0x72, 0, 0, 0);

        RTMPPrepareCTSFrame(pAd, pAd->HwCfg.CurrentAddress, 35000/*25000*/, 0, 0, 0); // prepare more time for protect because of polling architecture
        RTUSBWriteMACRegister(pAd, CARRIER_R66_VALUE, 0x40305040);
    }
    
    // Read Hardware controlled Radio state enable bit
    if (NicConfig2.field.HardwareRadioControl == 1)
    {
        BOOLEAN RadioOff = FALSE;
        pAd->StaCfg.bHardwareRadio = TRUE;
        
        DBGPRINT(RT_DEBUG_TRACE, ("HwRadioControl Radio\n"));
        
        {
            // Read GPIO pin2 as Hardware controlled radio state
            RTUSBReadMACRegister(pAd, GPIO_CTRL_CFG, &data);
            if ((data & 0x04) == 0)
                RadioOff = TRUE;
        }

        //
        // The GPIO pin2 default is 1:Pull-High, means HW Radio Enable.
        // When the value is 0, means HW Radio disable.
        //
        if (RadioOff)
        {
            pAd->StaCfg.bHwRadio = FALSE;
            // Update extra information to link is up
            pAd->UiCfg.ExtraInfo = HW_RADIO_OFF;
        }
    }
    else
        pAd->StaCfg.bHardwareRadio = FALSE;     
    
    pAd->StaCfg.bRadio = pAd->StaCfg.bSwRadio && pAd->StaCfg.bHwRadio;
    
    if (pAd->StaCfg.bRadio == FALSE)
    {       
        MT_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

        AsicSendCommanToMcu(pAd, SLEEP_MCU_CMD, 0xff, 0xff, 0x02);
        LedCtrlSetLed(pAd, LED_RADIO_OFF);
    }
    else
    {
        AsicSendCommanToMcu(pAd, SLEEP_MCU_CMD, 0xff, 0xff, 0x02);
        AsicSendCommanToMcu(pAd, WAKE_MCU_CMD, 0xff, 0x00, 0x02);

        LedCtrlSetLed(pAd, LED_RADIO_ON);
    }

//
    // Not indicate PhyPowerState at NICInitAsicFromEEPROM.
    // This will cause MlmeRadio->NicResetForError->NICInitAsicFromEEPROM to indicate this event too.
    // And cause SoftAP_OIDs_ext failed. Because PhyPowerState will be calling twice. 
    // (one at OID_DOT11_NIC_POWER_STATE, and the other at MlmeRadioOn if OID_DOT11_NIC_POWER_STATE set to power on).
    // 
    //
    // PlatformIndicateCurrentPhyPowerState(pAd,0, DOT11_PHY_ID_ANY);
    //

    //
    // log if the chipset support TX ALC (auto level control)
    //
    if (NicConfig2.field.DynamicTxAgcControl == 1)
        pAd->HwCfg.bAutoTxAgcA = pAd->HwCfg.bAutoTxAgcG = TRUE;
    else
        pAd->HwCfg.bAutoTxAgcA = pAd->HwCfg.bAutoTxAgcG = FALSE;        

    // disable AntennaDiversity, FreqCali when testing WHQL
    if (pAd->LogoTestCfg.OnTestingWHQL)
    {
        pAd->HwCfg.FreqCalibrationCtrl.bEnableFrequencyCalibration = FALSE;
        pAd->HwCfg.NicConfig2.field.AntDiv = 0;
    }

    pPort->CommonCfg.BandState = UNKNOWN_BAND;

    BbSetRxAntByRxPath(pAd);

    BbSetTxDacCtrlByTxPath(pAd);
    
    // Set Aetenna if bit 12 is set, Fixed Antenna Selection is applied
    if (pAd->HwCfg.NicConfig2.field.AntDiv == ANT_DIV_CONFIG_MAIN_ANT)
    {
        AsicSetRxAnt(pAd, 0);   // main ant.
    }
    else if (pAd->HwCfg.NicConfig2.field.AntDiv == ANT_DIV_CONFIG_AUX_ANT)
    {
        AsicSetRxAnt(pAd, 1);   // aux ant.
    }
    else
    {
        // do nothing
    }

    DBGPRINT(RT_DEBUG_TRACE,("Use Hw Radio Control Pin=%d; if used Pin=%d;\n",
        pAd->StaCfg.bHardwareRadio, pAd->StaCfg.bHardwareRadio));
    
    DBGPRINT(RT_DEBUG_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d, LED mode=%x\n", pAd->HwCfg.Antenna.field.TxPath, pAd->HwCfg.Antenna.field.RxPath, pAd->HwCfg.RfIcType, pAd->HwCfg.LedCntl.word));

    DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}

/*
    ========================================================================
    
    Routine Description:
        Initialize NIC hardware

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    Note:
        
    ========================================================================
*/
NDIS_STATUS NICInitializeAsic(
    IN  PMP_ADAPTER   pAd)
{
    // TODO: Add
    return NDIS_STATUS_SUCCESS;

}

//
// Post-process the BBP registers based on the chip model
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID PostBBPInitialization(
    IN PMP_ADAPTER pAd)
{
    BBP_R105_STRUC BBPR105 = {0};
    BBP_R106_STRUC BBPR106 = {0};
    UCHAR BbpReg = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    //ULONG ulBbValue;
    
    DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

    RTUSBReadBBPRegister(pAd, BBP_R105, &BBPR105.byte);

    BBPR105.field.EnableSIGRemodulation = 0;
    BBPR105.field.ADAPTIVE_FEQ_ENABLE = 0;
    BBPR105.field.ADAPTIVE_FEQ_STEP = 0;
        
    //
    // Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)
    //
    if (pAd->HwCfg.Antenna.field.RxPath == 1) // Single RX
    {
        BbSetMldFor2Stream(pAd, FALSE);
    }
    else
    {
        BbSetMldFor2Stream(pAd, TRUE);
    }

    RTUSBWriteBBPRegister(pAd, BBP_R105, BBPR105.byte);

    DBGPRINT(RT_DEBUG_TRACE, ("%s: BBP_R105: BBPR105.field.EnableSIGRemodulation = %d, BBPR105.field.ADAPTIVE_FEQ_ENABLE = %d, BBPR105.field.ADAPTIVE_FEQ_STEP = %d\n", 
        __FUNCTION__, 
        BBPR105.field.EnableSIGRemodulation, 
        BBPR105.field.ADAPTIVE_FEQ_ENABLE, 
        BBPR105.field.ADAPTIVE_FEQ_STEP));

    // Write specific BBP value from registry
    if (pPort->CommonCfg.BBPIDV != 0)
    {
        UCHAR RegIdx, RegValue;
        RegIdx = (UCHAR)((pPort->CommonCfg.BBPIDV & 0xFF00) >> 8);
        RegValue  = (UCHAR)(pPort->CommonCfg.BBPIDV & 0xFF);  
        RTUSBWriteBBPRegister(pAd, RegIdx, RegValue);   
        DBGPRINT(RT_DEBUG_TRACE, ("%s: BBPIDV: RegIdx = %d, RegValue = %d\n", 
            __FUNCTION__, 
            RegIdx, 
            RegValue));     
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}

//
// Post-process the RF registers based on the chip model
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID PostRFInitialization(
    IN PMP_ADAPTER pAd)
{    
    DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));
    DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}

#if DBG

//
// Load the external EEPROM image (RT3xUsbEeprom.bin)
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  NDIS_STATUS_SUCCESS: The image is valid
//  NDIS_STATUS_FAILURE: Cannot open the external image
//
NDIS_STATUS LoadExternalEEPROMImage(
    IN PMP_ADAPTER pAd)
{
    NDIS_STATUS Status;
    NDIS_STRING FileName;
    NDIS_HANDLE FileHandle;
    UINT FileLength = 0;
    PUCHAR pFileImage;
    NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);
    ULONG offset = 0;
    
    NdisInitializeString(&FileName,"RT3xUsbEeprom.bin");
    NdisOpenFile(&Status, &FileHandle, &FileLength, &FileName, HighestAcceptableMax);
    NdisFreeString(FileName);

    if ((Status != NDIS_STATUS_SUCCESS) /*|| (FileLength != MAX_EXTERNAL_EEPROM_IMAGE_SIZE)*/ )
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: NdisOpenFile (RT3xUsbEeprom.bin) failed\n", __FUNCTION__));

        pAd->TrackInfo.bExternalEEPROMImage = FALSE;
        
        return NDIS_STATUS_FAILURE;
    }
    
    NdisMapFile(&Status, &pFileImage, FileHandle);
    
    PlatformMoveMemory(pAd->TrackInfo.EEPROMImage, pFileImage, FileLength);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisUnmapFile(FileHandle);
        NdisCloseFile(FileHandle);
    }

    pAd->TrackInfo.bExternalEEPROMImage = TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: *************************************************************************\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: FileLength = %d\n", __FUNCTION__, FileLength));

    for (offset = 0; offset < MAX_EXTERNAL_EEPROM_IMAGE_SIZE; offset++)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: EEPROM[0x%02X] = 0x%02X\n", __FUNCTION__, offset, pAd->TrackInfo.EEPROMImage[offset]));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: *************************************************************************\n", __FUNCTION__));

    return NDIS_STATUS_SUCCESS;
}

#endif // DBG

//
// Initialize the pseudo STA configuration that used for soft AP mode.
//
VOID InitAPPseudoSTAConfig(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    pPort->CommonCfg.APPseudoSTAConfig.AuthMode = Ralink802_11AuthModeOpen;
}

// IRQL = PASSIVE_LEVEL
UCHAR BtoH(char ch)
{
    if (ch >= '0' && ch <= '9') return (ch - '0');        // Handle numerals
    if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  // Handle capitol hex digits
    if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  // Handle small hex digits
    return(255);
}

//
//  FUNCTION: AtoH(char *, UCHAR *, int)
//
//  PURPOSE:  Converts ascii string to network order hex
//
//  PARAMETERS:
//    src    - pointer to input ascii string
//    dest   - pointer to output hex
//    destlen - size of dest
//
//  COMMENTS:
//
//    2 ascii bytes make a hex byte so must put 1st ascii byte of pair
//    into upper nibble and 2nd ascii byte of pair into lower nibble.
//
// IRQL = PASSIVE_LEVEL

void AtoH(char * src, UCHAR * dest, int destlen)
{
    char * srcptr;
    PUCHAR destTemp;

    srcptr = src;   
    destTemp = (PUCHAR) dest; 

    while(destlen--)
    {
        *destTemp = BtoH(*srcptr++) << 4;    // Put 1st ascii byte in upper nibble.
        *destTemp += BtoH(*srcptr++);      // Add 2nd ascii byte to above.
        destTemp++;
    }
}


/*
    ========================================================================
    
    Routine Description:
        Enable RX 

    Arguments:
        pAd                     Pointer to our adapter

    Return Value:
        None

    IRQL <= DISPATCH_LEVEL
    
    Note:
        Before Enable RX, make sure you have enabled Interrupt.
    ========================================================================
*/
VOID RTMPEnableRxTx(
    IN PMP_ADAPTER    pAd)
{
//  WPDMA_GLO_CFG_STRUC GloCfg;
    USB_DMA_CFG_STRUC   UsbCfg;
    ULONG   i = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPEnableRxTx\n"));
    
    RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0x4);

#if 0
    do
    {
        RTUSBReadMACRegister(pAd, WPDMA_GLO_CFG, &GloCfg.word);
        if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
            break;
        
        Delay_us(50);
        i++;
    }while ( i <1);
    DBGPRINT(RT_DEBUG_TRACE, ("<== READ DMA offset 0x208 = 0x%x\n", GloCfg.word));  
    
    {
        Delay_us(50);
        GloCfg.field.EnTXWriteBackDDONE = 1;
        GloCfg.field.EnableRxDMA = 1;
        GloCfg.field.EnableTxDMA = 1;
        DBGPRINT(RT_DEBUG_TRACE, ("<== WRITE DMA offset 0x208 = 0x%x\n", GloCfg.word)); 
        RTUSBWriteMACRegister(pAd, WPDMA_GLO_CFG, GloCfg.word);

    }
#endif

    {
        UsbCfg.word = 0;
        // for last packet, PBF might use more than limited, so minus 2 to prevent from error
        // If A-MSDU's frame size is about 3k, it will hit our buffer boundary. Change RX bulk aggregation limit from 21k to 19k. 
        UsbCfg.field.RxBulkAggLmt = (MAX_RXBULK_SIZE /1024)-4;  
        UsbCfg.field.phyclear = 0;
        // OS version is 2k and usb version is 1.1,do not use bulk in aggregation
        if ( pAd->pHifCfg->BulkInMaxPacketSize == 512)
            UsbCfg.field.RxBulkAggEn = 1;

        UsbCfg.field.RxBulkEn = 1;
        UsbCfg.field.TxBulkEn = 1;
        UsbCfg.field.RxBulkAggTOut = 0x18;  // 2006-10-18
        RTUSBWriteMACRegister(pAd, USB_DMA_CFG, UsbCfg.word);

    }
    // enable RX of MAC block
    if ((pAd->OpMode == OPMODE_AP) || 
        ((pAd->OpMode == OPMODE_STA) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)))
    {
        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, APNORMAL);     // enable RX of DMA block
    }
    else
    {
        if (pPort->CommonCfg.PSPXlink)
            RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, PSPXLINK);
        else
            RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, STANORMAL);
    }
    
    RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0xc); //default: MAC TX/RX Enable 

    DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPEnableRxTx\n")); 
}

VOID    RTMPPrepareCTSFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA,
    IN  ULONG           Duration,
    IN  UCHAR           RTSRate,
    IN  UCHAR           CTSIdx,
    IN  UCHAR           FrameGap)
{
    RTS_FRAME           RtsFrame;
    PRTS_FRAME          pRtsFrame;
    TXWI_STRUC          TxWI;
    PUCHAR              ptr;
    ULONG               longptr;
    UINT                i;
    //HTTRANSMIT_SETTING    BeaconTransmit;
    PHY_CFG BeaconPhyCfg = {0};
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];


    
    UNREFERENCED_PARAMETER(FrameGap);
    UNREFERENCED_PARAMETER(CTSIdx);
    UNREFERENCED_PARAMETER(RTSRate);    
    UNREFERENCED_PARAMETER(pDA);    
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPPrepareCTSFrame\n"));    

    PlatformZeroMemory(&TxWI, sizeof(TxWI));
    PlatformZeroMemory(&RtsFrame, sizeof(RTS_FRAME));

    pRtsFrame = &RtsFrame;
                
    pRtsFrame->FC.Type  = BTYPE_CNTL;
    pRtsFrame->FC.SubType = SUBTYPE_CTS;
    pRtsFrame->Duration = (USHORT)Duration;
    
    COPY_MAC_ADDR(pRtsFrame->Addr1, pAd->HwCfg.PermanentAddress);

    if (pPort->Channel <= 14)
    {
        WRITE_PHY_CFG_MODE(pAd, &BeaconPhyCfg, MODE_CCK);
    }
    else
    {
        WRITE_PHY_CFG_MODE(pAd, &BeaconPhyCfg, MODE_OFDM);
    }

    XmitWriteTxWI(pAd, pAd->PortList[PORT_0], &TxWI, FALSE, FALSE, FALSE, FALSE, FALSE, 0, RESERVED_WCID, 
                10, PID_CTS_TO_SELF, 0, IFS_HTTXOP, FALSE, BeaconPhyCfg, FALSE, TRUE, FALSE);

    //
    // Move TXWI and frame content to on-chip memory
    //
    ptr = (PUCHAR)&TxWI;
    for (i=0; i<pAd->HwCfg.TXWI_Length;)  // 24-byte TXINFO field
    {
        longptr = *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, HW_NULL_BASE + i, longptr);
        ptr += 4;
        i+=4;
    }   

    ptr = (PUCHAR)&RtsFrame;
    for (i=0; i< 10;)
    {
        longptr = *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, HW_NULL_BASE + pAd->HwCfg.TXWI_Length + i, longptr);
        ptr += 4;
        i+=4;
    }
}
    
// IRQL = PASSIVE_LEVEL


/*
    ========================================================================
    
    Routine Description:
        Enable Wlan function. this action will enable wlan clock so that chip can accept command. So MUST put in the 
        very beginning of Initialization. And put in the very LAST in the Halt function.

    Arguments:
        pAd                     Pointer to our adapter

    Return Value:
        None

    IRQL <= DISPATCH_LEVEL
    
    Note:
        Before Enable RX, make sure you have enabled Interrupt.
    ========================================================================
*/
VOID RTMPEnableWlan(
    IN PMP_ADAPTER    pAd,
    IN BOOLEAN bOn,
    IN BOOLEAN bResetWLAN)
{
    WLAN_FUN_CTRL_STRUC WlanFunCtrl = {0};
    CMB_CTRL_STRUC      CmbCtrl;
    WPDMA_GLO_CFG_STRUC GloCfg={0};
    UCHAR       index;
    UCHAR       PatchPLLCount = 0;
    BOOLEAN     PassiveLevel = TRUE;    

    //NdisAcquireSpinLock(&pAd->WlanEnLock);

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
        PassiveLevel = FALSE;   //use for delay function    

    DBGPRINT(RT_DEBUG_ERROR, ("==> RTMPWlan %d\n", bOn));
    RTUSBReadMACRegister(pAd, WLAN_FUN_CTRL, (PULONG)&WlanFunCtrl);

    if (bResetWLAN == TRUE)
    {
        WlanFunCtrl.field.GPIO0_OUT_OE_N = 0xFF;
        WlanFunCtrl.field.FRC_WL_ANT_SET = FALSE;
    }
    
    DBGPRINT(RT_DEBUG_ERROR, ("==> RTMPWlan %d, pAd->WlanFunCtrl.word = 0x%x\n", bOn, WlanFunCtrl.word));
    
    if (bOn == TRUE)
    {  
        if ((WlanFunCtrl.field.WLAN_EN == 1) && (bResetWLAN == FALSE))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("<== RTMPWlan, Exit because WLAN EN is already ON \n"));          
            //NdisReleaseSpinLock(&pAd->WlanEnLock);
            return;
        }
        WlanFunCtrl.field.WLAN_CLK_EN = 0;
        WlanFunCtrl.field.WLAN_EN = 1;
    }
    else
    {
        WlanFunCtrl.field.WLAN_EN = 0;
        WlanFunCtrl.field.WLAN_CLK_EN = 0; //clear it here, after PLL_LD write it to TRUE           
        //WlanFunCtrl.field.PCIE_APP0_CLK_REQ = 0;
    }

    DBGPRINT(RT_DEBUG_ERROR, ("==> RTMPWlan %d, WlanFunCtrl.word = 0x%x\n", bOn, WlanFunCtrl.word));
    RTUSBWriteMACRegister(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);

    if (PassiveLevel)
        NdisCommonGenericDelay(2);
    else
        Delay_us(2);

    do
    {
        index = 0;
        CmbCtrl.word = 0;
        if (bOn == TRUE)
        {
            do 
            {
                RTUSBReadMACRegister(pAd, CMB_CTRL, &CmbCtrl.word);

                // HW issue: Must check PLL_LD&XTAL_RDY when setting EEP to disable PLL power down
                if ((CmbCtrl.field.PLL_LD == 1) && (CmbCtrl.field.XTAL_RDY == 1))
                    break;

                if (PassiveLevel)
                    NdisCommonGenericDelay(20);
                else
                    Delay_us(20);
            } while (index++ < 100);

            DBGPRINT(RT_DEBUG_TRACE, (" [CMB_CTRL =0x%08x]. index = %d.\n", CmbCtrl.word, index));
        
            //WlanFunCtrl.field.PCIE_APP0_CLK_REQ = 1;
            WlanFunCtrl.field.WLAN_CLK_EN = 1;

            if(bResetWLAN)
            {
                RTUSBWriteMACRegister(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word | 0x8);  

                if (PassiveLevel)
                    NdisCommonGenericDelay(2);
                else
                    Delay_us(2);
            }


            if (bResetWLAN)
            {
                WlanFunCtrl.word &= (~0x8);
                RTUSBWriteMACRegister(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);        
            }
            else
            {
                RTUSBWriteMACRegister(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
            }
        }
    }while(PatchPLLCount != 0);

/*
    do
    {
        if (bOn == TRUE)
        {
            //WlanFunCtrl.field.PCIE_APP0_CLK_REQ = 1;
            WlanFunCtrl.field.WLAN_CLK_EN = 1;

            if (bResetWLAN) // CD team: 6x9x doesn't reset WLAN
            {
                RTUSBWriteMACRegister(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word | 0x8);  

                if (PassiveLevel)
                    NdisCommonGenericDelay(2);
                else
                    Delay_us(2);
            }
            else
            {
                RTUSBWriteMACRegister(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
            }           
        }
    }while(FALSE);
*/
    //NdisReleaseSpinLock(&pAd->WlanEnLock);    
    DBGPRINT(RT_DEBUG_ERROR, ("<== RTMPWlan %d  pAd->WlanFunCtrl.word = 0x%x\n", bOn, WlanFunCtrl.word));   
}

/*
    ========================================================================
    
    Routine Description:
        Allocate DMA memory blocks for send, receive

    Arguments:
        Adapter     Pointer to our adapter

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_FAILURE
        NDIS_STATUS_RESOURCES

    IRQL = PASSIVE_LEVEL

    Note:
    
    ========================================================================
*/
NDIS_STATUS 
NdisInitAdapterBlock(
    IN  PMP_ADAPTER   pAd,
    OUT PADAPTER_INITIAL_STAGE   pInitialStage
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    NET_BUFFER_LIST_POOL_PARAMETERS ListPoolParameters;
    NET_BUFFER_POOL_PARAMETERS      BufferPoolParameters;
#endif    
    UCHAR                           index;
    ULONG                           size = 0;
    ULONG                           DataLength;
    
    FUNC_ENTER;
    
    pAd->Counter.WlanCounters.TransmittedFragmentCount.LowPart =  0;
    pAd->Counter.WlanCounters.MulticastTransmittedFrameCount.LowPart =0;
    pAd->Counter.WlanCounters.FailedCount.LowPart =0;
    pAd->Counter.WlanCounters.NoRetryCount.LowPart =0;
    pAd->Counter.WlanCounters.RetryCount.LowPart =0;
    pAd->Counter.WlanCounters.MultipleRetryCount.LowPart =0;
    pAd->Counter.WlanCounters.RTSSuccessCount.LowPart =0;
    pAd->Counter.WlanCounters.RTSFailureCount.LowPart =0;
    pAd->Counter.WlanCounters.ACKFailureCount.LowPart =0;
    pAd->Counter.WlanCounters.FrameDuplicateCount.LowPart =0;
    pAd->Counter.WlanCounters.ReceivedFragmentCount.LowPart =0;
    pAd->Counter.WlanCounters.MulticastReceivedFrameCount.LowPart =0;
    pAd->Counter.WlanCounters.FCSErrorCount.LowPart =0;

    pAd->Counter.WlanCounters.TransmittedFragmentCount.HighPart =  0;
    pAd->Counter.WlanCounters.MulticastTransmittedFrameCount.HighPart =0;
    pAd->Counter.WlanCounters.FailedCount.HighPart =0;
    pAd->Counter.WlanCounters.NoRetryCount.HighPart =0;
    pAd->Counter.WlanCounters.RetryCount.HighPart =0;
    pAd->Counter.WlanCounters.MultipleRetryCount.HighPart =0;
    pAd->Counter.WlanCounters.RTSSuccessCount.HighPart =0;
    pAd->Counter.WlanCounters.RTSFailureCount.HighPart =0;
    pAd->Counter.WlanCounters.ACKFailureCount.HighPart =0;
    pAd->Counter.WlanCounters.FrameDuplicateCount.HighPart =0;
    pAd->Counter.WlanCounters.ReceivedFragmentCount.HighPart =0;
    pAd->Counter.WlanCounters.MulticastReceivedFrameCount.HighPart =0;
    pAd->Counter.WlanCounters.FCSErrorCount.HighPart =0;

    
    for (index = 0; index < HW_MAX_SUPPORT_MBSSID; index++)
    {
        pAd->MBssidTable[index] = FALSE;
    }
    
    do
    {
        MTInitializeCmdQ(&pAd->pHifCfg->CmdQ);
        MTInitializeCmdQ(&pAd->pP2pCtrll->P2PCmdQ);
        MTInitializeCmdQ(&pAd->MccCfg.MultiChannelCmdQ);

        NdisCommonAllocateAllSpinLock(pAd);

        pInitialStage->bAllocSpinLock = TRUE;
        

#if _WIN8_USB_SS_SUPPORTED
        //
        //Initialization for USB SS.
        //
        if (FALSE == UsbSsInit(pAd))
        {
            ndisStatus = NDIS_STATUS_FAILURE;
            DBGPRINT(RT_DEBUG_ERROR, ("Initialization failed for USB SS.\n"));
        }
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }
#endif

        //
        // Init send data structures and related parameters
        //
        ndisStatus = NdisCommonInitTransmit(pAd);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pInitialStage->bInitXmit = TRUE;

        //
        // Init receive data structures and related parameters
        //
        ndisStatus = NdisCommonInitRecv(pAd);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pInitialStage->bInitRecv = TRUE;
 #if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))       
        PlatformZeroMemory(&ListPoolParameters, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));

        ListPoolParameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
        ListPoolParameters.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
        ListPoolParameters.Header.Size = sizeof(ListPoolParameters);
        ListPoolParameters.ProtocolId = 0;
        ListPoolParameters.ContextSize = 0;
        ListPoolParameters.fAllocateNetBuffer = TRUE;
        ListPoolParameters.PoolTag = NIC_TAG_RX;

        pAd->pRxCfg->RecvPktsPool = NdisAllocateNetBufferListPool(pAd->AdapterHandle, &ListPoolParameters);
        if (pAd->pRxCfg->RecvPktsPool == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("NdisAllocateNetBufferListPool for Recv failed\n"));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        PlatformZeroMemory(&BufferPoolParameters, sizeof(NET_BUFFER_POOL_PARAMETERS));
        BufferPoolParameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
        BufferPoolParameters.Header.Revision = NET_BUFFER_POOL_PARAMETERS_REVISION_1;
        BufferPoolParameters.Header.Size = sizeof(BufferPoolParameters);
        BufferPoolParameters.PoolTag = NIC_TAG_RX;

        pAd->pRxCfg->RecvPktPool = NdisAllocateNetBufferPool(pAd->AdapterHandle, &BufferPoolParameters);
        if (pAd->pRxCfg->RecvPktPool == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("NdisAllocateNetBufferPool for Recv failed\n"));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
#endif

#if UAPSD_AP_SUPPORT
        for (index=0; index<NUM_OF_UAPSD_CLI; index++)
        {
            InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[index][QID_AC_BE]);
            InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[index][QID_AC_BK]);
            InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[index][QID_AC_VI]);
            InitializeQueueHeader(&pAd->UAPSD.TxSwUapsdQueue[index][QID_AC_VO]);
        }
#endif

        //
        // Create an MDL to describe the data buffer portion
        //
        PlatformAllocateMemory(pAd, &pAd->pRxCfg->DummyForIndicatedMDL, ROUND_TO_PAGES(MAX_FRAME_SIZE) + PAGE_SIZE);
        if (pAd->pRxCfg->DummyForIndicatedMDL == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Allocate memory for pAd->pRxCfg->DummyForIndicatedMDL failed\n"));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        InitializeQueueHeader(&pAd->pRxCfg->IndicateRxPktsQueue);
        InitializeQueueHeader(&pAd->pTxCfg->LocalNetBufferListQueue);

  #if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))           
        for (index = 0; index < NUM_OF_LOCAL_RX_PKT_BUFFER; index++)
        {
            PMP_LOCAL_RX_PKTS pIndicateRxPkts = &pAd->pRxCfg->IndicateRxPkts[index];

            
            // we don't know the actual Virtual address, the size of the buffer could be 'n' pages but it can span
            // 'n+1' pages because the Virtual Address can have an offset into the first Page.
            // DummySize is defined in rtmp.h
            // 
            pIndicateRxPkts->Mdl = NdisAllocateMdl(pAd->AdapterHandle,
                                                        (PVOID)pAd->pRxCfg->DummyForIndicatedMDL,
                                                        ROUND_TO_PAGES(MAX_FRAME_SIZE) + PAGE_SIZE);            

            DataLength = MmGetMdlByteCount(pIndicateRxPkts->Mdl);
            
            pIndicateRxPkts->NetBufferList = NdisAllocateNetBufferAndNetBufferList(
                                                        pAd->pRxCfg->RecvPktsPool,
                                                        0,
                                                        0,
                                                        pIndicateRxPkts->Mdl,
                                                        0,
                                                        DataLength);
            
            if (pIndicateRxPkts->NetBufferList == NULL)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("NdisAllocateNetBufferAndNetBufferList failed\n"));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            pIndicateRxPkts->NetBuffer = NET_BUFFER_LIST_FIRST_NB(pIndicateRxPkts->NetBufferList);

            MT_SET_NETBUFFERLIST_IN_NBL(pIndicateRxPkts->NetBufferList, pIndicateRxPkts);
                
            InsertTailQueue(&pAd->pRxCfg->IndicateRxPktsQueue, pIndicateRxPkts);
        }

        
        for (index = 0; index < NUM_OF_LOCAL_RX_PKT_BUFFER; index++)
        {
            PMP_LOCAL_RX_PKTS pLocalNetBufferList = &pAd->pTxCfg->LocalNetBufferList[index];

            //
            // Create an MDL to describe the data buffer portion, 
            // We can't use Dummy data here, on SoftAP mode may need it for Bridge.
            //
            PlatformAllocateMemory(pAd, &pLocalNetBufferList->Data, MAX_FRAME_SIZE);
            if (pLocalNetBufferList->Data == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Allocate memory for pLocalNetBufferList->Data failed\n"));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            //
            // Create an MDL to describe the data buffer portion
            //
            pLocalNetBufferList->Mdl = NdisAllocateMdl(pAd->AdapterHandle,
                                                        (PVOID)pLocalNetBufferList->Data,
                                                        MAX_FRAME_SIZE);            

            pLocalNetBufferList->NetBufferList = NdisAllocateNetBufferAndNetBufferList(
                                                        pAd->pRxCfg->RecvPktsPool,
                                                        0,
                                                        0,
                                                        pLocalNetBufferList->Mdl,
                                                        0,
                                                        MAX_FRAME_SIZE);
            
            if (pLocalNetBufferList->NetBufferList == NULL)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("NdisAllocateNetBufferAndNetBufferList failed\n"));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            pLocalNetBufferList->NetBuffer = NET_BUFFER_LIST_FIRST_NB(pLocalNetBufferList->NetBufferList);

            MT_SET_NETBUFFERLIST_IN_NBL(pLocalNetBufferList->NetBufferList, pLocalNetBufferList);
                
            InsertTailQueue(&pAd->pTxCfg->LocalNetBufferListQueue, pLocalNetBufferList);
        }
#endif
        //Allocate Local buffer for Tcb.
        {
            USHORT  i =0;

            InitializeQueueHeader(&pAd->pTxCfg->XcuIdelQueue);
            
            for(i =0;i<NUM_OF_MP_XMIT_CTRL_UNIT_NUMBER;i++)
            {
                PVOID   Ptr = NULL;
                
                PlatformZeroMemory(&pAd->pTxCfg->XcuArray[i], sizeof(MT_XMIT_CTRL_UNIT));
                pAd->pTxCfg->XcuArray[i].CurrentXcuidx = i;

                InsertTailQueue(&pAd->pTxCfg->XcuIdelQueue, &pAd->pTxCfg->XcuArray[i]);
            }
        }
      

        //
        // Initial AdhocList for OID_DOT11_ENUM_ASSOCIATION_INFO (Adhoc connect list).
        //
        InitializeQueueHeader(&pAd->StaCfg.FreeAdhocListQueue);
        InitializeQueueHeader(&pAd->StaCfg.ActiveAdhocListQueue);
        for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++)
        {
            pAd->StaCfg.AdhocList[index].AssocState = dot11_assoc_state_zero;
            InsertTailQueue(&pAd->StaCfg.FreeAdhocListQueue, &pAd->StaCfg.AdhocList[index]);
        }   
    } while (FALSE);
    
    
    
    FUNC_LEAVE_STATUS(ndisStatus);

    return ndisStatus;
}

VOID  
NdisInitFreeRepositMemory(
    IN  PMP_ADAPTER   pAd
    )
{
    // free Reposit memory
    if(pAd->pNicCfg->RepositBssTable.pBssEntry != NULL)
    {
        PlatformFreeMemory(pAd->pNicCfg->RepositBssTable.pBssEntry,MAX_LEN_OF_BSS_TABLE*sizeof(BSS_ENTRY));
        DBGPRINT(RT_DEBUG_TRACE, ("pAd->pNicCfg->RepositBssTable.pBssEntry free memory \n"));
    }

    // free TempReposit memory
    if(pAd->pNicCfg->TempRepositBssTable.pBssEntry != NULL)
    {
        PlatformFreeMemory(pAd->pNicCfg->TempRepositBssTable.pBssEntry,MAX_LEN_OF_BSS_TABLE*sizeof(BSS_ENTRY));
        DBGPRINT(RT_DEBUG_TRACE, ("pAd->pNicCfg->TempRepositBssTable.pBssEntry free memory \n"));
    }
}

NDIS_STATUS 
NdisInitGetBssList(
    IN  PMP_ADAPTER   pAd,
    OUT PVOID           InformationBuffer,
    IN  ULONG           OutputBufferLength
    )
{

    UCHAR               i,j;
    PBSS_ENTRY          pBss = NULL;
    PUCHAR              pCurrPtr;
    PDOT11_BSS_ENTRY    pDot11BSSEntry = NULL;
    PDOT11_BYTE_ARRAY   pDot11ByteArray = NULL;
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
    ULONG               BSSEntrySize = 0;
    ULONG               RemainingBytes;
    ULONG               IETotalSize = 0;
    UCHAR               TempBssNr = 0;
    LARGE_INTEGER       currTime;
    BOOLEAN             bFound;
    ULONG               dwTimeExpired;
    ULONG               dwDifferTime;
    ULONG               dwOverflow = 0xffffffff;
    UCHAR               uChannel;
    BOOLEAN             bChannelLegal;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    pDot11ByteArray = (PDOT11_BYTE_ARRAY)InformationBuffer;

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
        //if(pAd->pNicCfg->bUpdateBssList)
        if (1)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Have newly Bss List!!!!!!!!!!!\n"));

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

                        //for wps issue ,we need to make sure probe response WPS IE also exist in table.
                        if (pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtBeaconsLen > 0 && 
                            (pAd->pNicCfg->TempRepositBssTable.pBssEntry[j].WSCInfoAtBeaconsLen == 0))
                        {
                            pAd->pNicCfg->TempRepositBssTable.pBssEntry[j].WSCInfoAtBeaconsLen =   pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtBeaconsLen;
                            PlatformMoveMemory(&pAd->pNicCfg->TempRepositBssTable.pBssEntry[j].WSCInfoAtBeacons,&pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtBeacons,
                                        pAd->pNicCfg->RepositBssTable.pBssEntry[i].WSCInfoAtBeaconsLen);

                            DBGPRINT(RT_DEBUG_INFO,("Ssid <<%s>> with bea\n", pAd->ScanTab.BssEntry[i].Ssid));
                        }

                        bFound = TRUE;
                        break;
                    }
                }

                
                if(!bFound)
                {
                    DBGPRINT(RT_DEBUG_INFO,("Not Found Ssid <<%s>>\n", pAd->pNicCfg->RepositBssTable.pBssEntry[i].Ssid));
                    
                    if(currTime.LowPart > pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart)
                        dwDifferTime = currTime.LowPart - pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart;
                    else
                        dwDifferTime = dwOverflow - pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart + currTime.LowPart;
                
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
        
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if (pAd->LogoTestCfg.OnTestingWHQL)  // Only for BVT Test on Win8
            {
                pAd->pNicCfg->bUpdateBssList = FALSE;
            }
#endif
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Old Bss List!!!!!!!!!!!\n"));
        }

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

//                      pDot11BSSEntry->uPhyId = MlmeInfoGetPhyIdByChannel(pAd, pBss->Channel);
                if(pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                {
                    pDot11BSSEntry->uPhyId = dot11_phy_type_ht;
                }
                else
                {
                    if (pBss->Channel > 14)
                    {
                        pDot11BSSEntry->uPhyId = dot11_phy_type_ofdm;
                    }
                    else
                    {
                        pDot11BSSEntry->uPhyId = dot11_phy_type_erp;
                    }
                    if((pAd->MlmeAux.SupRateLen <= 4) && (pAd->MlmeAux.ExtRateLen == 0))
                        pDot11BSSEntry->uPhyId = dot11_phy_type_hrdsss;
                }                       

                pDot11BSSEntry->usBeaconPeriod = pBss->BeaconPeriod;
                pDot11BSSEntry->ullTimestamp = pBss->BeaconTimestamp;
                pDot11BSSEntry->ullHostTimestamp = pBss->HostTimestamp;
                pDot11BSSEntry->dot11BSSType = pBss->BssType;
                pDot11BSSEntry->usCapabilityInformation = pBss->CapabilityInfo;
                pDot11BSSEntry->lRSSI = pBss->Rssi;

                if ((pAd->HwCfg.BLNAGain != 0xFF) && (pAd->HwCfg.Antenna.field.RssiIndicationMode == USER_AWARENESS_MODE))
                {
                    pDot11BSSEntry->lRSSI = pDot11BSSEntry->lRSSI + GET_LNA_GAIN(pAd); // RSSI indication by Rx LNA output

                    DBGPRINT(RT_DEBUG_TRACE, ("OID_DOT11_ENUM_BSS_LIST, RSSI, MacAddress = %02X:%02X:%02X:%02X:%02X:%02X, pDot11BSSEntry->lRSSI = %d, pAd->HwCfg.LatchRfRegs.Channel = %d\n", 
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

                DBGPRINT(RT_DEBUG_TRACE, ("SSID : %s, Channel : %d(%d), PhyId = %d(%d), BSSTYPE = %d\n", pBss->Ssid, pBss->Channel, pDot11BSSEntry->PhySpecificInfo.uChCenterFrequency, pDot11BSSEntry->uPhyId, dot11_phy_type_ht , pDot11BSSEntry->dot11BSSType));

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

                    DBGPRINT(RT_DEBUG_TRACE, ("OID_DOT11_ENUM_BSS_LIST, RSSI, MacAddress = %02X:%02X:%02X:%02X:%02X:%02X, pDot11BSSEntry->lRSSI = %d, pAd->HwCfg.LatchRfRegs.Channel = %d\n", 
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

    return ndisStatus;
}

