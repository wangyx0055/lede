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

    Abstract:


 */

#ifndef __PMF_H
#define __PMF_H

VOID    PmfReqTimeoutActionTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3);

VOID    PmfConfirmTimeoutActionTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3);

VOID PmfInjectPckets(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID PmfSendSAQuery(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN UCHAR Action,
    IN PUSHORT pTranscationID);

VOID PmfSAQueryConfirmAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PmfSAQueryReqAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PmfSAQueryRspAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PmfPeerSAQueryRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PMF_PeerAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PmfDeriveIGTK(
    IN  PMP_ADAPTER   pAd,
    OUT UCHAR           *output);

INT PmfExtractBIPAction(
    IN      PMP_ADAPTER   pAd,
    IN OUT  PUCHAR          pMgmtFrame,
    IN      UINT            mgmt_len);

BOOLEAN PmfExtractIGTKKDE(
    IN  PMP_ADAPTER       pAd,
    IN  PUCHAR              pBuf,
    IN  INT                 buf_len);

#endif // __PMF_H //

