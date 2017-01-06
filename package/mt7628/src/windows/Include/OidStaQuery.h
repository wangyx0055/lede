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
    STA_oids_Query.h

    Abstract:
    

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------


*/
#ifndef __OIDSTAQUERY_H__
#define __OIDSTAQUERY_H__


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
N6QueryOidDot11PowerMgmtModeAutoEnabled(
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
N6QueryOidDot11PowerMgmtModeStatus(
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
N6QueryOidDot11OffloadNetworkList(
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
N6QueryOidPmRemoveWolPattern(
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
N6QueryOidGenInterruptModeration(
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
N6QueryOidPmRemoveProtocolOffload(
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
N6QueryOidPmAddProtocolOffload(
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
N6QueryOidPmAddWolPattern(
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
N6QueryOidPmGetProtocolOffload(
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
N6QueryOidDot11NicPowerState(
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
N6QueryOidDot11DesiredPhyList(
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
N6QueryOidDot11AutoConfigEnabled(
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
N6QueryOidDot11CurrentOperationMode(
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
N6QueryOidDot11FlushBssList(
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
N6QueryOidDot11DesiredSsidList(
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
N6QueryOidDot11CurrentChannel(
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
N6QueryOidDot11SafeModeEnabled(
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
N6QueryOidDot11CurrentPhyId(
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
N6QueryOidPnpAddWakeUpPattern(
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
N6QueryOidPnpRemoveWakeUpPattern(
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
N6QueryOidPnpEnableWakeUp(
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
N6QueryOidPnpCapabilities(
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
N6QueryOidPnpQueryPower(
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
N6QueryOidDot11MpduMaxLength(
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
N6QueryOidDot11OperationModeCapability(
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
N6QueryOidDot11AtimWindow(
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
N6QueryOidDot11OptionalCapability(
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
N6QueryOidDot11CurrentOptionalCapability(
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
N6QueryOidDot11StationId(
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
N6QueryOidDot11ChannelAgilityEnabled(
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
N6QueryOidDot11ChannelAgilityPresent(
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
N6QueryOidDot11OperationalRateSet(
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
N6QueryOidDot11MacAddress(
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
N6QueryOidDot11RtsThreshold(
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
N6QueryOidDot11ShortRetryLimit(
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
N6QueryOidDot11LongRetryLimit(
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
N6QueryOidDot11FragmentationThreshold(
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
N6QueryOidDot11MaxTransmitMsduLifetime(
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
N6QueryOidDot11MaxReceiveLifetime(
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
N6QueryOidDot11SupportedPhyTypes(
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
N6QueryOidDot11CurrentRegDomain(
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
N6QueryOidDot11TempType(
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
N6QueryOidDot11DiversitySupport(
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
N6QueryOidDot11SupportedPowerLevels(
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
N6QueryOidDot11CurrentTxPowerLevel(
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
N6QueryOidDot11CcaModeSupported(
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
N6QueryOidDot11CurrentCcaMode(
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
N6QueryOidDot11EdThreshold(
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
N6QueryOidDot11RegDomainsSupportValue(
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
N6QueryOidDot11SupportedTxAntenna(
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
N6QueryOidDot11SupportedRxAntenna(
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
N6QueryOidDot11DiversitySelectionRx(
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
N6QueryOidDot11SupportedDataRatesValue(
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
N6QueryOidDot11RfUsage(
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
N6QueryOidDot11RecvSensitivityList(
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
N6QueryOidDot11MultiDomainCapabilityImplemented(
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
N6QueryOidDot11MultiDomainCapabilityEnabled(
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
N6QueryOidDot11CountryString(
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
N6QueryOidDot11CurrentFrequency(
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
N6QueryOidDot11FrequencyBandsSupported(
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
N6QueryOidDot11ShortPreambleOptionImplemented(
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
N6QueryOidDot11PbccOptionImplemented(
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
N6QueryOidDot11ErpPbccOptionImplemented(
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
N6QueryOidDot11ErpPbccOptionEnabled(
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
N6QueryOidDot11DsssOfdmOptionImplemented(
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
N6QueryOidDot11DsssOfdmOptionEnabled(
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
N6QueryOidDot11ShortSlotTimeOptionImplemented(
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
N6QueryOidDot11ShortSlotTimeOptionEnabled(
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
N6QueryOidDot11PowerMgmtRequest(
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
N6QueryOidDot11ExcludedMacAddressList(
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
N6QueryOidDot11ExcludeUnencrypted(
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
N6QueryOidDot11DesiredBssidList(
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
N6QueryOidDot11DesiredBssType(
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
N6QueryOidDot11Statistics(
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
N6QueryOidDot11EnabledAuthenticationAlgorithm(
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
N6QueryOidDot11SupportedUnicastAlgorithmPair(
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
N6QueryOidDot11SupportedMulticastAlgorithmPair(
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
N6QueryOidDot11EnabledUnicastCipherAlgorithm(
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
N6QueryOidDot11EnabledMulticastCipherAlgorithm(
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
N6QueryOidDot11CipherDefaultKeyId(
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
N6QueryOidDot11CipherDefaultKey(
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
N6QueryOidDot11CipherKeyMappingKey(
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
N6QueryOidDot11EnumAssociationInfo(
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
N6QueryOidDot11HardwarePhyState(
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
N6QueryOidDot11MediaStreamingEnabled(
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
N6QueryOidDot11UnreachableDetectionThreshold(
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
N6QueryOidDot11ActivePhyList(
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
N6QueryOidDot11ExtstaCapability(
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
N6QueryOidDot11DataRateMappingTable(
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
N6QueryOidDot11MaximumListSize(
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
N6QueryOidDot11PrivacyExemptionList(
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
N6QueryOidDot11IbssParams(
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
N6QueryOidDot11PmkidList(
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
N6QueryOidDot11UnicastUseGroupEnabled(
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
N6QueryOidDot11HiddenNetworkEnabled(
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
N6QueryOidDot11QosParams(
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
N6QueryOidDot11AvailableChannelList(
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
N6QueryOidDot11PortStateNotification(
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
N5QueryOidGenSupportedList(
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
N5QueryOidGenHardwareStatus(
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
N5QueryOidGenMediaSupported(
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
N5QueryOidGenMediaInUse(
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
N5QueryOidGenPhysicalMedium(
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
N5QueryOidGenMaximumLookahead(
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
N5QueryOidGenMaximumFrameSize(
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
N5QueryOidGenLinkSpeed(
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
N5QueryOidGenTransmitBufferSpace(
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
N5QueryOidGenReceiveBufferSpace(
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
N5QueryOidGenTransmitBlockSize(
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
N5QueryOidGenReceiveBlockSize(
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
N5QueryOidGenVendorId(
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
N5QueryOidGenVendorDescription(
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
N5QueryOidGenVendorDriverVersion(
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
N5QueryOidGenCurrentPacketFilter(
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
N5QueryOidGenCurrentLookahead(
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
N5QueryOidGenDriverVersion(
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
N5QueryOidGenMaximumTotalSize(
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
N5QueryOidGenProtocolOptions(
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
N5QueryOidGenMacOptions(
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
N5QueryOidGenMediaConnectStatus(
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
N5QueryOidGenXmitOk(
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
N5QueryOidGenRcvOk(
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
N5QueryOidGenXmitError(
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
N5QueryOidGenRcvError(
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
N5QueryOidGenRcvNoBuffer(
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
N5QueryOidGenRcvCrcError(
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
N5QueryOidGenTransmitQueueLength(
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
N5QueryOid8023PermanentAddress(
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
N5QueryOidDot11PermanentAddress(
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
N5QueryOid8023CurrentAddress(
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
N5QueryOidDot11CurrentAddress(
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
N5QueryOidDot11MulticastList(
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
N5QueryOid8023MaximumListSize(
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
N5QueryOidGenInterruptModeration(
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
N5QueryOidGenLinkParameters(
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
N5QueryOidPmParameters(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
#endif