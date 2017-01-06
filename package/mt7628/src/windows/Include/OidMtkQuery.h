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
    MTK_oids_Query.h

    Abstract:
    

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------


*/
#ifndef __OIDMTKQUERY_H__
#define __OIDMTKQUERY_H__
  

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryVendor(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryPhyMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryCountryRegion(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryEepromVersion(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryFirmwareVersion(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryCurrentChannelId(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryTxPowerLevel1(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryHardwareRegister(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryLinkStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryNoiseLevel(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryStatistics(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryExtraInfo(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryLastRxRate(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryRssi0(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryRssi1(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryLastTxRate(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryRssi2(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySnr0(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySnr1(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryConfiguration(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryAuthenticationMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryEncryptionStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryEventTable(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryMacTable(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryOpMode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryApConfig(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryBatable(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryImmeBaCap(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryHtPhymode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWpaRekey(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryStaConfig(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryPreamble(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryApsdSetting(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryApsdPsm(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWmm(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryDls(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryDlsParam(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWapiSupport(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryMfpControl(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryPortUsage(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryUsbVersion(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryInitializationState(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryManualChList(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryBtHciGetCmd(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryBtHciGetAclData(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryEepromBandwidth(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryP2pStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWifiDirectLinkstate(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryP2pMacInfo(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryP2pPersist(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySigmaP2pStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySigmaWpsPincode(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySigmaPsk(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryBaldEagleSsid(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryExtregEventMessage(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySuppressScan(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWifiChiptype(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryTdtPeriodic(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryTdtOneshotCalibration(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryTdtOneshotPacket(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryTdtOneshotSelectRatetableName(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryStbcRxCount(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySsNum(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryStbcTxCount(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQuerySnr2(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscVersion(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
    

/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscExtregPbcEnroMac(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryHwArchitectureVersion(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryHwCapabilities(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWtblInfo(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryTestTxNullFrame(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryRadioStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryMib(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryPowerManagement(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryRfTest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscCustomizedIeParam(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscStatus(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscProfile(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscSsid(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscHwPbc(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryPreferredWpsApPhyType(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWscApChannel(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );


/**
 * .
 *
 * @param pAd pointer to MP_ADAPTER structure.
 * @param NdisRequest pointer to NDIS_OID_REQUEST structure.
 *
 * @return If the function succeeds, the return value is NDIS_STATUS_SUCCESS.
 * Otherwise, an error code is returned.
 */
NDIS_STATUS
MtkOidQueryWsc20Testbed(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );
#endif
