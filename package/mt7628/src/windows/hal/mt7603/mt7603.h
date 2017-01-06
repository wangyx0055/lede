/*! \file "mt7603.h"
      \brief MT7603 related functions
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

 // TODO: Shiang-usw-win, Lens, what's the purpose of this header file??
 
#ifndef MT7603_H
#define MT7603_H

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
* \brief Register chip-specific functions
*
* \param pAd pointer to MP_ADAPTER
*
* \return None
*/
/*----------------------------------------------------------------------------*/
VOID
RegisterHalMt7603(
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
HalSwitchChannelMt7603(
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
HalSwitchBandwidthMt7603(
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
HalEnterPm2Mt7603(
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
HalEnterPm4Mt7603(
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
HalExitPm4Mt7603(
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
HalEnterPm5Mt7603(
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
HalExitPm5Mt7603(
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
HalEnterPm6Mt7603(
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
HalExitPm6Mt7603(
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
HalEnterPm7Mt7603(
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
HalSetTxRxMt7603(
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
HalSetTxStreamsMt7603(
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
HalSetRxStreamsMt7603(
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
HalSetDeviceMacAddressMt7603(
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
HalSetRxWtblMt7603(
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
HalInitWtblMt7603(
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
HalReadEfuseMt7603(
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
HalWriteEfuseMt7603(
    PMP_ADAPTER pAd, 
    USHORT Offset, 
    PUCHAR pData, 
    USHORT Length
    );

#endif /* MT7603_H */

