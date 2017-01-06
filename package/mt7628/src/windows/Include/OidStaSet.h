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
    STA_oids_Set.h

    Abstract:
    

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------


*/
#ifndef __OIDSTASET_H__
#define __OIDSTASET_H__


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS        
N6SetOidDot11PowerMgmtModeAutoEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11PowerMgmtModeStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11OffloadNetworkList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPmRemoveWolPattern(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    ); 


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidGenInterruptModeration(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPmRemoveProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPmAddProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );          


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPmAddWolPattern(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPmGetProtocolOffload(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );  


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11EnumBssList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11NicPowerState(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ResetRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11DesiredPhyList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );   


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ScanRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */ 
NDIS_STATUS
N6SetOidDot11DisconnectRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ConnectRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );   


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11AutoConfigEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );  


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CurrentOperationMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11FlushBssList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11DesiredSsidList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CurrentChannel(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );    


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11SafeModeEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CurrentPhyId(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPnpAddWakeUpPattern(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    ); 


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPnpRemoveWakeUpPattern(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    ); 


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPnpEnableWakeUp(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPnpCapabilities(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );   


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidPnpSetPower(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );  


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11AtimWindow(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ChannelAgilityEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ChannelAgilityPresent(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11OperationalRateSet(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11RtsThreshold(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ShortRetryLimit(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11LongRetryLimit(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11FragmentationThreshold(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CurrentRegDomain(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11RecvSensitivityList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11MultiDomainCapabilityEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CountryString(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CurrentFrequency(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11PowerMgmtRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ExcludedMacAddressList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11ExcludeUnencrypted(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11DesiredBssidList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11DesiredBssType(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11Statistics(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11EnabledAuthenticationAlgorithm(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11SupportedUnicastAlgorithmPair(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11SupportedMulticastAlgorithmPair(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11EnabledUnicastCipherAlgorithm(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11EnabledMulticastCipherAlgorithm(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CipherDefaultKeyId(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CipherDefaultKey(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11CipherKeyMappingKey(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11MediaStreamingEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11UnreachableDetectionThreshold(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11MaximumListSize(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11PrivacyExemptionList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11IbssParams(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11PmkidList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11UnicastUseGroupEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11HiddenNetworkEnabled(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11QosParams(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11AvailableChannelList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N6SetOidDot11PortStateNotification(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );   


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidGenPhysicalMedium(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidGenMaximumLookahead(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    ); 


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidGenCurrentPacketFilter(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    ); 


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidGenCurrentLookahead(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidGenProtocolOptions(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOid8023MulticastList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidDot11MulticastList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    ); 


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidGenInterruptModeration(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidGenLinkParameters(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to RTMP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
N5SetOidPmParameters(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    
#endif
