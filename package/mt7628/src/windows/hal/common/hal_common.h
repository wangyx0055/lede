/*! \file "hal_common.h"
      \brief A set of routines that emulate some chip-specific detailes.
*/

/*******************************************************************************
* Copyright (c) 2013 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/

#ifndef HAL_COMMON_H
#define HAL_COMMON_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/


/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

#pragma pack(push, struct_pack1)
#pragma pack(1)

//
// Forward declaration
//
typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;

//
// HAL control
//
typedef struct _HAL_CTRL_T
{
    //
    // HW operations
    //
    VOID (*pfHalSwitchChannel)(PMP_ADAPTER pAd, UCHAR Channel);
    VOID (*pfHalSwitchBandwidth)(PMP_ADAPTER pAd, UCHAR Bandwidth);
    VOID (*pfHalEnterPm2)(PMP_ADAPTER pAd);
    VOID (*pfHalEnterPm4)(PMP_ADAPTER pAd);
    VOID (*pfHalExitPm4)(PMP_ADAPTER pAd);
    VOID (*pfHalEnterPm5)(PMP_ADAPTER pAd);
    VOID (*pfHalExitPm5)(PMP_ADAPTER pAd);
    VOID (*pfHalEnterPm6)(PMP_ADAPTER pAd);
    VOID (*pfHalExitPm6)(PMP_ADAPTER pAd);
    VOID (*pfHalEnterPm7)(PMP_ADAPTER pAd);
    VOID (*pfHalSetTxRx)(PMP_ADAPTER pAd, BOOLEAN EnableTx, BOOLEAN EnableRx);
    VOID (*pfHalSetTxStreams)(PMP_ADAPTER pAd, UCHAR NumOfTxStreams);
    VOID (*pfHalSetRxStreams)(PMP_ADAPTER pAd, UCHAR NumOfRxStreams);
    VOID (*pfHalSetDeviceMacAddress)(PMP_ADAPTER pAd, PUCHAR pMacAddress);
    VOID (*pfHalSetRxWtbl)(PMP_ADAPTER pAd);
    VOID (*pfHalInitWtbl)(PMP_ADAPTER pAd);
    NDIS_STATUS (*pfHalReadEfuse)(PMP_ADAPTER pAd, USHORT Offset, PUCHAR pData, USHORT Length);
    NDIS_STATUS (*pfHalWriteEfuse)(PMP_ADAPTER pAd, USHORT Offset, PUCHAR pData, USHORT Length);
} HAL_CTRL_T, *P_HAL_CTRL_T;

#pragma pack(pop, struct_pack1)

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/


/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/


/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* \brief Initialize the chip-specific functions based on the chip type
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalInitHalCtrl(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Switch channel
*
* \param pAd pointer to MP_ADAPTER
* \param Channel Channel number
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalSwitchChannel(
    PMP_ADAPTER pAd, 
    UCHAR Channel
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Switch bandwidth
*
* \param pAd pointer to MP_ADAPTER
* \param Bandwidth bandwidth
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalSwitchBandwidth(
    PMP_ADAPTER pAd, 
    UCHAR Bandwidth
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Enter PM2
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalEnterPm2(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Enter PM4
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalEnterPm4(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Exit PM4
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalExitPm4(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Enter PM5
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalEnterPm5(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Exit PM5
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalExitPm5(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Enter PM6
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalEnterPm6(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Exit PM6
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalExitPm6(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Entry PM7
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalEnterPm7(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Set Tx/Rx
*
* \param pAd pointer to MP_ADAPTER
* \param bEnableTx enable/disable Tx
* \param bEnableRx enable/disable Rx
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalSetTxRx(
    PMP_ADAPTER pAd, 
    BOOLEAN bEnableTx, 
    BOOLEAN bEnableRx
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Set Tx streams
*
* \param pAd pointer to MP_ADAPTER
* \param NumOfTxStream Number of Tx streams
* 
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalSetTxStreams(
    PMP_ADAPTER pAd, 
    UCHAR NumOfTxStreams
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Set Rx streams
*
* \param pAd pointer to MP_ADAPTER
* \param NumOfRxStream Number of Tx streams
* 
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalSetRxStreams(
    PMP_ADAPTER pAd, 
    UCHAR NumOfRxStreams
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Set the device's MAC address
*
* \param pAd pointer to MP_ADAPTER
* \param pMacAddress pointer to the MAC address
* 
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalSetDeviceMacAddress(
    PMP_ADAPTER pAd, 
    PUCHAR pMacAddress
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Set the Rx WTBL
*
* \param pAd pointer to MP_ADAPTER
* 
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalSetRxWtbl(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Initialize the WTBL
*
* \param pAd pointer to MP_ADAPTER
* 
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalInitWtbl(
    PMP_ADAPTER pAd
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Read e-fuse
*
* \param pAd pointer to MP_ADAPTER
* \param Offset e-fuse offset
* \param pData pointer to a caller-allocated buffer in which to return the e-fuse contents
* \param Offset how many bytes of e-fuse contents to return?
*
* \return NDIS_STATUS_SUCCESS if successful read operation
*/
/*----------------------------------------------------------------------------*/
NDIS_STATUS
HalReadEfuse(
    PMP_ADAPTER pAd, 
    USHORT Offset, 
    PUCHAR pData, 
    USHORT Length
    );

/*----------------------------------------------------------------------------*/
/*!
* \brief Write e-fuse
*
* \param pAd pointer to MP_ADAPTER
* \param Offset e-fuse offset
* \param pData pointer to a caller-allocated buffer in which to return the e-fuse contents
* \param Offset how many bytes of e-fuse contents to return?
*
* \return NDIS_STATUS_SUCCESS if successful read operation
*/
/*----------------------------------------------------------------------------*/
NDIS_STATUS
HalWriteEfuse(
    PMP_ADAPTER pAd, 
    USHORT Offset, 
    PUCHAR pData, 
    USHORT Length
    );

#endif /* HAL_COMMON_H */

