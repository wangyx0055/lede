/*! \file "hal_common.c"
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

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "MtConfig.h"

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
    )
{
// TODO: Shiang-usw-win, add it here, need to move to other place!
#ifdef MT_MAC
	{
		UINT32 Value;

		HW_IO_READ32(pAd, TOP_HVR, &Value);
		pAd->HWVersion = Value;

		HW_IO_READ32(pAd, TOP_FVR, &Value);
		pAd->FWVersion = Value;

		HW_IO_READ32(pAd, TOP_HCR, &Value);
		pAd->ChipID = Value;

		if (IS_MT7603(pAd) || IS_MT76x6(pAd))
		{
			HW_IO_READ32(pAd, STRAP_STA, &Value);
			pAd->AntMode = (Value >> 24) & 0x1;
		}
	}
#endif

    if (IS_MT7603(pAd))
    {
        RegisterHalMt7603(pAd);
        pAd->chipCap.WtblHwNum = MT_WTBL_SIZE;
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalSwitchChannel)
    {
        pAd->HalCtrl.pfHalSwitchChannel(pAd, Channel);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalSwitchBandwidth)
    {
        pAd->HalCtrl.pfHalSwitchBandwidth(pAd, Bandwidth);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalEnterPm2)
    {
        pAd->HalCtrl.pfHalEnterPm2(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalEnterPm4)
    {
        pAd->HalCtrl.pfHalEnterPm4(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalExitPm4)
    {
        pAd->HalCtrl.pfHalExitPm4(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalEnterPm5)
    {
        pAd->HalCtrl.pfHalEnterPm5(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalExitPm5)
    {
        pAd->HalCtrl.pfHalExitPm5(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalEnterPm6)
    {
        pAd->HalCtrl.pfHalEnterPm6(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalExitPm6)
    {
        pAd->HalCtrl.pfHalExitPm6(pAd);
    }
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Enter PM7
*
* \param pAd pointer to MP_ADAPTER
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
HalEnterPm7(
    PMP_ADAPTER pAd
    )
{
    if (pAd->HalCtrl.pfHalEnterPm7)
    {
        pAd->HalCtrl.pfHalEnterPm7(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalSetTxRx)
    {
        pAd->HalCtrl.pfHalSetTxRx(pAd, bEnableTx, bEnableRx);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalSetTxStreams)
    {
        pAd->HalCtrl.pfHalSetTxStreams(pAd, NumOfTxStreams);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalSetRxStreams)
    {
        pAd->HalCtrl.pfHalSetRxStreams(pAd, NumOfRxStreams);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalSetDeviceMacAddress)
    {
        pAd->HalCtrl.pfHalSetDeviceMacAddress(pAd, pMacAddress);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalSetRxWtbl)
    {
        pAd->HalCtrl.pfHalSetRxWtbl(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalInitWtbl)
    {
        pAd->HalCtrl.pfHalInitWtbl(pAd);
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalReadEfuse)
    {
        return pAd->HalCtrl.pfHalReadEfuse(pAd, Offset, pData, Length);
    }
    else
    {
        return NDIS_STATUS_FAILURE;
    }
}

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
    )
{
    if (pAd->HalCtrl.pfHalWriteEfuse)
    {
        return pAd->HalCtrl.pfHalWriteEfuse(pAd, Offset, pData, Length);
    }
    else
    {
        return NDIS_STATUS_FAILURE;
    }
}


//#ifdef MT7603_FPGA
UINT32 mt_mac_cr_range[] = {
    0x60000000, 0x20000, 0x200, /* WF_CFG */
    0x60100000, 0x21000, 0x200, /* WF_TRB */
    0x60110000, 0x21200, 0x200, /* WF_AGG */
    0x60120000, 0x21400, 0x200, /* WF_ARB */
    0x60130000, 0x21600, 0x200, /* WF_TMAC */
    0x60140000, 0x21800, 0x200, /* WF_RMAC */
    0x60150000, 0x21A00, 0x200, /* WF_SEC */
    0x60160000, 0x21C00, 0x200, /* WF_DMA */
    0x60170000, 0x21E00, 0x200, /* WF_CFGOFF */
    0x60180000, 0x22000, 0x1000, /* WF_PF */
    0x60190000, 0x23000, 0x200, /* WF_WTBLOFF */
    0x601A0000, 0x23200, 0x200, /* WF_ETBF */
    
    0x60300000, 0x24000, 0x400, /* WF_LPON */
    0x60310000, 0x24400, 0x200, /* WF_INT */
    0x60320000, 0x28000, 0x4000, /* WF_WTBLON */
    0x60330000, 0x2C000, 0x200, /* WF_MIB */
    0x60400000, 0x2D000, 0x200, /* WF_AON */

    0x80020000, 0x00000, 0x2000, /* TOP_CFG */  
    0x80000000, 0x02000, 0x2000, /* MCU_CFG */
    0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
//#ifdef MT7603_FPGA
//    0xA0000000, 0x80000, 0x10000, /* PSE_CFG after remap 2 */
//#else
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
//#endif /* MT7603_FPGA */
    0x60200000, 0x10000, 0x10000, /* WF_PHY */
    
    0x0, 0x0, 0x0,
};
//#endif


#define MT7603_PSE_WTBL_2_ADDR	0xa5000000
#define MT7636_PSE_WTBL_2_ADDR	0xa8000000

static UINT32 MtUsbPhysicalAddrMap(UINT32 addr)
{
#if 1
	UINT32 global_addr = 0x0, idx = 1;
	//extern UINT32 mt_mac_cr_range[];

	if (addr < 0x2000)
		global_addr = 0x80020000 + addr;
	else if ((addr >= 0x2000) && (addr < 0x4000))
		global_addr = 0x80000000 + addr - 0x2000;
	else if ((addr >= 0x4000) && (addr < 0x8000))
		global_addr = 0x50000000 + addr - 0x4000;
	else if ((addr >= 0x8000) && (addr < 0x10000))
		global_addr = 0xa0000000 + addr - 0x8000;
	else if ((addr >= 0x10000) && (addr < 0x20000))
		global_addr = 0x60200000 + addr - 0x10000;
	else if ((addr >= 0x20000) && (addr < 0x40000))
	{
		do {
			if ((addr >= mt_mac_cr_range[idx]) && (addr < (mt_mac_cr_range[idx]+mt_mac_cr_range[idx+1]))) {
				global_addr = mt_mac_cr_range[idx-1]+(addr-mt_mac_cr_range[idx]);
				break;
			}
			idx += 3;
		}while (mt_mac_cr_range[idx] != 0);

		//      if (mt_mac_cr_range[idx] == 0)
		//      {
		//          DBGPRINT(RT_DEBUG_OFF, ("unknow addr range = %x\n", addr));
		//      }
	}
	else if ((addr >= 0x40000) && (addr < 0x80000)) //WTBL Address
	{
		global_addr = MT7603_PSE_WTBL_2_ADDR + addr - 0x40000;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr1=0x%x\n", global_addr));
	}
	else if ((addr >= 0xc0000) && (addr < 0xc0100)) //PSE Client
	{
		global_addr = 0x800c0000 + addr - 0xc0000;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr2=0x%x\n", global_addr));
	}
	else
		global_addr = addr;

	return global_addr;
#else
	return 0;
#endif
}


// For MT7603 /////////////////////////
NTSTATUS HW_IO_READ32(PMP_ADAPTER pAd, UINT32 Offset, UINT32 *pValue)
{
    NTSTATUS    Status;
    UINT32      CRAddress;
    USHORT      BaseAddress;
    USHORT      OffsetValue;

    CRAddress = MtUsbPhysicalAddrMap(Offset);

    BaseAddress = (CRAddress >> 16) & 0xffff;
    OffsetValue = CRAddress & 0xffff;

    // 0x5000_2345
    DBGPRINT_RAW(RT_DEBUG_INFO, ("-->%s, BaseAddress = 0x%x, offset = 0x%x, OffsetValue = %d, pValue = %02x %02x %02x %02x\n", __FUNCTION__, BaseAddress, Offset, OffsetValue, pValue[0], pValue[1], pValue[2], pValue[3]));   
    Status = USBVendorRequest(
            pAd,
            USBD_TRANSFER_DIRECTION_IN,
            VENDOR_REQUEST_READ,
            BaseAddress, // 0x5000
            OffsetValue, // 0x2345
            pValue,
            sizeof(ULONG));
    
    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("<--%s, status%d\n", __FUNCTION__, Status));    
    return Status;
}


NTSTATUS HW_IO_WRITE32(PMP_ADAPTER pAd, UINT32 Offset, UINT32 Value)
{
    NTSTATUS    Status;
    UINT32      CRAddress;
    USHORT      BaseAddress;
    USHORT      OffsetValue;

    CRAddress = MtUsbPhysicalAddrMap(Offset);

    BaseAddress = (CRAddress >> 16) & 0xffff;
    OffsetValue = CRAddress & 0xffff;

    // 0x5000_2345
    DBGPRINT_RAW(RT_DEBUG_INFO, ("-->%s, BaseAddress = 0x%x, offset = 0x%x, OffsetValue = 0x%x, Value = %x\n", __FUNCTION__, BaseAddress, Offset, OffsetValue, Value));    
    Status = USBVendorRequest(
            pAd,
            USBD_TRANSFER_DIRECTION_OUT,
            VENDOR_REQUEST_WRITE,
            BaseAddress, // 0x5000
            OffsetValue, // 0x2345
            (PVOID)&Value,
            sizeof(ULONG));
    
    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("<--%s, status%d\n", __FUNCTION__, Status));    
    return Status;
}

NTSTATUS EFUSE_IO_READ128(PMP_ADAPTER pAd, UINT32 Offset, UINT8 *pValue)
{
    NTSTATUS    Status;
    USHORT      BaseAddress;
    USHORT      OffsetValue;

    BaseAddress = (Offset >> 16) & 0xffff;
    OffsetValue = Offset & 0xffff;

    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, offset = 0x%x\n", __FUNCTION__, Offset)); 

    Status = USBVendorRequest(
            pAd,
            USBD_TRANSFER_DIRECTION_IN,
            VENDOR_REQUEST_READ_EFUSE,
            BaseAddress, // 0; not use
            OffsetValue, // EFuse offset : 0x00  0x10  0x20 ..... (base : 16 bytes)
            pValue,
            16);
    
    return Status;
}

NTSTATUS EFUSE_IO_WRITE128(PMP_ADAPTER pAd, UINT32 Offset, UINT8 *pValue)
{
    NTSTATUS    Status;
    USHORT      BaseAddress;
    USHORT      OffsetValue;

    BaseAddress = (Offset >> 16) & 0xffff;
    OffsetValue = Offset & 0xffff;

    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, offset = 0x%x\n", __FUNCTION__, Offset)); 
    DBGPRINT(RT_DEBUG_TRACE, ("-->%s, Value = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, pValue[0], pValue[1], pValue[2], pValue[3], pValue[4], pValue[5], pValue[6], pValue[7], pValue[8], pValue[9], pValue[10], pValue[11], pValue[12], pValue[13], pValue[14], pValue[15])); 

    Status = USBVendorRequest(
            pAd,
            USBD_TRANSFER_DIRECTION_OUT,
            VENDOR_REQUEST_WRITE_EFUSE,
            BaseAddress, // 0; not use
            OffsetValue, // EFuse offset : 0x00  0x10  0x20 ..... (base : 16 bytes)
            pValue,
            16);
    
    return Status;
}