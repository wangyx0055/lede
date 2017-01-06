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
    RT6X9X.c

    Abstract: 
    RT6X9X features (RT6590/RT6592/MT7601)

    Revision History:
    Who                         When            What
    -------------------     ----------      ----------------------------------------------
    Ian Tang and Brenton Wu     2011/11/16      Start implementation
    Ian Tang                    2012/3/13       Add MT7650 (RT6590)
    BJ Chiang                   2014/4/10       Remove MT7650 (RT6590)    
*/

#include "MtConfig.h"

//
// Read the DoubleWord of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_DOUBLE_WORD(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    return pPhyCfg->DoubleWord;
}

//
// Read the MCS of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_MCS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.MCS;
    }
    else
    {
        return pPhyCfg->Default.MCS;
    }
}

//
// Read the BW of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_BW(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.BW;
    }
    else
    {
        return pPhyCfg->Default.BW;
    }
}

//
// Read the ShortGI of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_SHORT_GI(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.ShortGI;
    }
    else
    {
        return pPhyCfg->Default.ShortGI;
    }
}

//
// Read the STBC of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_STBC(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.STBC;
    }
    else
    {
        return pPhyCfg->Default.STBC;
    }
}

//
// Read the bITxBfPacket of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_ITX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.bITxBfPacket;
    }
    else
    {
        return pPhyCfg->Default.bITxBfPacket;
    }
}

//
// Read the bETxBfPacket of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_ETX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.bETxBfPacket;
    }
    else
    {
        return pPhyCfg->Default.bETxBfPacket;
    }
}

//
// Read the Mode of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_MODE(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.Mode;
    }
    else
    {
        return pPhyCfg->Default.Mode;
    }
}

//
// Read the Nss of the PHY_CFG
//
extern ULONG FORCEINLINE READ_PHY_CFG_NSS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg)
{
    if (EXT_MAC_CAPABLE(pAd))
    {
        return pPhyCfg->Ext.Nss;
    }
    else
    {
        return (pPhyCfg->Default.MCS / 8);
    }
}

//
// Write the DoubleWord of PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_DOUBLE_WORD(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG DoubleWord)
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    pPhyCfg->DoubleWord = DoubleWord;

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the MCS of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_MCS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG MCS) // MCS_XXX
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.MCS = MCS;
    }
    else
    {
        pPhyCfg->Default.MCS = MCS;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the BW of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_BW(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG BW) // BW_XXX
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.BW = BW;
    }
    else
    {
        pPhyCfg->Default.BW = BW;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the ShortGI of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_SHORT_GI(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG ShortGI) // GI_XXX
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.ShortGI = ShortGI;
    }
    else
    {
        pPhyCfg->Default.ShortGI = ShortGI;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the STBC of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_STBC(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG STBC) // STBC_XXX
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.STBC = STBC;
    }
    else
    {
        pPhyCfg->Default.STBC = STBC;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the bITxBfPacket of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_ITX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG bITxBfPacket) // 1/0
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.bITxBfPacket = bITxBfPacket;
    }
    else
    {
        pPhyCfg->Default.bITxBfPacket = bITxBfPacket;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the bETxBfPacket of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_ETX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG bETxBfPacket) // 1/0
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.bETxBfPacket = bETxBfPacket;
    }
    else
    {
        pPhyCfg->Default.bETxBfPacket = bETxBfPacket;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the Mode of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_MODE(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG Mode) // MODE_XXX
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.Mode = Mode;
    }
    else
    {
        pPhyCfg->Default.Mode = Mode;
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Write the Nss of the PHY_CFG
//
extern VOID FORCEINLINE WRITE_PHY_CFG_NSS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG Nss) // NSS_XXX
{
    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pPhyCfg->Ext.Nss = Nss;
    }
    else
    {
        // Do nothing
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Print the member of the PHY_CFG
//
extern VOID FORCEINLINE PRINT_PHY_CFG(
    IN PMP_ADAPTER    pAd, 
    IN PPHY_CFG         pPhyCfg,
    IN ULONG            RTDebugLevel)
{
    if (VHT_CAPABLE(pAd))
    {
        DBGPRINT(RTDebugLevel, ("PRINT_PHY_CFG: (MODE=%d, BW=%d, MCS=%d, Nss=%d, SGI=%d, STBC=%d, ITxBf=%d, ETxBf=%d)\n", 
            READ_PHY_CFG_MODE(pAd, pPhyCfg),
            READ_PHY_CFG_BW(pAd, pPhyCfg),
            READ_PHY_CFG_MCS(pAd, pPhyCfg),
            READ_PHY_CFG_NSS(pAd, pPhyCfg),
            READ_PHY_CFG_SHORT_GI(pAd, pPhyCfg), 
            READ_PHY_CFG_STBC(pAd, pPhyCfg),
            READ_PHY_CFG_ITX_BF_PACKET(pAd, pPhyCfg),
            READ_PHY_CFG_ETX_BF_PACKET(pAd, pPhyCfg)
            ));
    }
    else
    {
        DBGPRINT(RTDebugLevel, ("PRINT_PHY_CFG: (MODE=%d, BW=%d, MCS=%d, SGI=%d, STBC=%d, ITxBf=%d, ETxBf=%d)\n", 
            READ_PHY_CFG_MODE(pAd, pPhyCfg),
            READ_PHY_CFG_BW(pAd, pPhyCfg),
            READ_PHY_CFG_MCS(pAd, pPhyCfg), 
            READ_PHY_CFG_SHORT_GI(pAd, pPhyCfg), 
            READ_PHY_CFG_STBC(pAd, pPhyCfg),
            READ_PHY_CFG_ITX_BF_PACKET(pAd, pPhyCfg),
            READ_PHY_CFG_ETX_BF_PACKET(pAd, pPhyCfg)            
            ));
    }

}

//
// Indicate rate for UI
//
VOID IndicateRateForUi(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pRatePhyCfg, 
    IN OUT PVOID pRateIndication, 
    IN OUT PULONG pRateSize)
{
    PUI_RATE_DISPLAY_INDICATION pUiRateDisplayIndication = (PUI_RATE_DISPLAY_INDICATION)(pRateIndication);
    PUI_RATE_DISPLAY_INDICATION_EXT pUiRateDisplayIndicationExt = (PUI_RATE_DISPLAY_INDICATION_EXT)(pRateIndication);

    DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));
    
    if (EXT_MAC_CAPABLE(pAd))
    {
        pUiRateDisplayIndicationExt->field.MCS = READ_PHY_CFG_MCS(pAd, pRatePhyCfg);
        pUiRateDisplayIndicationExt->field.BW = READ_PHY_CFG_BW(pAd, pRatePhyCfg);
        pUiRateDisplayIndicationExt->field.ShortGI = READ_PHY_CFG_SHORT_GI(pAd, pRatePhyCfg);
        pUiRateDisplayIndicationExt->field.STBC = READ_PHY_CFG_STBC(pAd, pRatePhyCfg);
        pUiRateDisplayIndicationExt->field.bITxBfPacket = READ_PHY_CFG_ITX_BF_PACKET(pAd, pRatePhyCfg);
        pUiRateDisplayIndicationExt->field.bETxBfPacket = READ_PHY_CFG_ETX_BF_PACKET(pAd, pRatePhyCfg);
        pUiRateDisplayIndicationExt->field.Mode = READ_PHY_CFG_MODE(pAd, pRatePhyCfg);
        pUiRateDisplayIndicationExt->field.Nss = READ_PHY_CFG_NSS(pAd, pRatePhyCfg);

        *pRateSize = sizeof(UI_RATE_DISPLAY_INDICATION_EXT);

        DBGPRINT(RT_DEBUG_INFO, ("%s: MCS = %d, BW = %d, ShortGI = %d, STBC = %d, bITBfPacket = %d, bETxBfPacket = %d, Mode = %d, Nss = %d\n", 
            __FUNCTION__, 
            pUiRateDisplayIndicationExt->field.MCS, 
            pUiRateDisplayIndicationExt->field.BW, 
            pUiRateDisplayIndicationExt->field.ShortGI, 
            pUiRateDisplayIndicationExt->field.STBC, 
            pUiRateDisplayIndicationExt->field.bITxBfPacket, 
            pUiRateDisplayIndicationExt->field.bETxBfPacket, 
            pUiRateDisplayIndicationExt->field.Mode, 
            pUiRateDisplayIndicationExt->field.Nss));
    }
    else
    {
        pUiRateDisplayIndication->field.MCS = (USHORT)(READ_PHY_CFG_MCS(pAd, pRatePhyCfg));
        pUiRateDisplayIndication->field.BW = (USHORT)(READ_PHY_CFG_BW(pAd, pRatePhyCfg));
        pUiRateDisplayIndication->field.ShortGI = (USHORT)(READ_PHY_CFG_SHORT_GI(pAd, pRatePhyCfg));
        pUiRateDisplayIndication->field.STBC = (USHORT)(READ_PHY_CFG_STBC(pAd, pRatePhyCfg));
        pUiRateDisplayIndication->field.bITxBfPacket = (USHORT)(READ_PHY_CFG_ITX_BF_PACKET(pAd, pRatePhyCfg));
        pUiRateDisplayIndication->field.bETxBfPacket = (USHORT)(READ_PHY_CFG_ETX_BF_PACKET(pAd, pRatePhyCfg));
        pUiRateDisplayIndication->field.Mode = (USHORT)(READ_PHY_CFG_MODE(pAd, pRatePhyCfg));

        *pRateSize = sizeof(UI_RATE_DISPLAY_INDICATION);

        DBGPRINT(RT_DEBUG_INFO, ("%s: MCS = %d, BW = %d, ShortGI = %d, STBC = %d, bITBfPacket = %d, bETxBfPacket = %d, Mode = %d\n", 
            __FUNCTION__, 
            pUiRateDisplayIndication->field.MCS, 
            pUiRateDisplayIndication->field.BW, 
            pUiRateDisplayIndication->field.ShortGI, 
            pUiRateDisplayIndication->field.STBC, 
            pUiRateDisplayIndication->field.bITxBfPacket, 
            pUiRateDisplayIndication->field.bETxBfPacket, 
            pUiRateDisplayIndication->field.Mode));
    }

    DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

//
// Read the IQ compensation configuration from eFuse
//
VOID ReadIQCompensationConfiguraiton(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Set the Tx/Rx bandwidth control
//
VOID BbSetTxRxBwCtrl(
    IN PMP_ADAPTER pAd, 
    IN UCHAR TxRxBwCtrl) // BW_XXX
{
    // Unify ToDo
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Switch channel bandwidth
//
VOID SwitchBandwidth(
    IN PMP_ADAPTER pAd, 
    IN BOOLEAN bSwitchChannel, 
    IN UCHAR Channel, 
    IN UCHAR Bw,                        // BW_XXX
    IN UCHAR ExtensionChannelOffset)    // EXTCHA_XXX
{
#if 0
    UCHAR BbValue = 0;
    ULONG ExtBbValue = 0;    
    ULONG MacValue = 0;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: --> switching to Channel = %d, bw = %d,ExtensionChannelOffset = %d\n", 
            __FUNCTION__,
            Channel,
            Bw,
            ExtensionChannelOffset));

    if (EXT_BB_CAPABLE(pAd) && EXT_MAC_CAPABLE(pAd))
    {
        SwitchBandwidth6x9x(pAd, bSwitchChannel, Channel, Bw, ExtensionChannelOffset);
        return;
    }

    if (Bw == BW_20)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: BW20\n", __FUNCTION__));

        //
        // Tx band selection
        //
        // 0: use lower 40MHz band in 20MHz Tx
        // 1: use upper 40MHz band in 20MHz Tx
        //
        RTUSBReadMACRegister(pAd, TX_BAND_CFG, &MacValue);
        MacValue = ((MacValue & ~0x00000001) | 0x00000000);
        RTUSBWriteMACRegister(pAd, TX_BAND_CFG, MacValue);

        //
        // Tx/Rx bandwidth control
        //
        // 0: 20MHz
        // 1: 10MHz (11J)
        // 2: 40MHz
        //
        RTUSBReadBBPRegister(pAd, BBP_R4, &BbValue);
        BbValue = ((BbValue & ~0x18) | 0x00); // Tx/Rx bandwidth control
        RTUSBWriteBBPRegister(pAd, BBP_R4, BbValue);

        //
        // Primary channel selectoin
        //
        // 0: Primary channel - lower frequency channel
        // 1: Primary channel - higher frequency channel
        //
        RTUSBReadBBPRegister(pAd, BBP_R3, &BbValue);
        BbValue = ((BbValue & ~0x20) | 0x00);
        RTUSBWriteBBPRegister(pAd, BBP_R3, BbValue);
        pAd->StaCfg.BBPR3 = BbValue;

        if (pAd->HwCfg.MACVersion == 0x28600100)
        {
            RTUSBWriteBBPRegister(pAd, BBP_R69, 0x16);
            RTUSBWriteBBPRegister(pAd, BBP_R70, 0x08);
            RTUSBWriteBBPRegister(pAd, BBP_R73, 0x11);
        }

        if (bSwitchChannel == TRUE)
        {
            AsicSwitchChannel(pAd, Channel, FALSE);
        }
    }
    else if (Bw == BW_40)
    {
        if (ExtensionChannelOffset == EXTCHA_ABOVE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: BW40, EXTCHA_ABOVE, Channel = %d\n", __FUNCTION__, Channel));
            
            //
            // Tx band selection
            //
            // 0: use lower 40MHz band in 20MHz Tx
            // 1: use upper 40MHz band in 20MHz Tx
            //
            RTUSBReadMACRegister(pAd, TX_BAND_CFG, &MacValue);
            MacValue = ((MacValue & ~0x00000001) | 0x00000000);
            RTUSBWriteMACRegister(pAd, TX_BAND_CFG, MacValue);

            //
            // Tx/Rx bandwidth control
            //
            // 0: 20MHz
            // 1: 10MHz (11J)
            // 2: 40MHz
            //
            RTUSBReadBBPRegister(pAd, BBP_R4, &BbValue);
            BbValue = ((BbValue & ~0x18) | 0x10);
            RTUSBWriteBBPRegister(pAd, BBP_R4, BbValue);

            //
            // Primary channel selectoin
            //
            // 0: Primary channel - lower frequency channel
            // 1: Primary channel - higher frequency channel
            //
            RTUSBReadBBPRegister(pAd, BBP_R3, &BbValue);
            BbValue = ((BbValue & ~0x20) | 0x00);
            RTUSBWriteBBPRegister(pAd, BBP_R3, BbValue);
            pAd->StaCfg.BBPR3 = BbValue;

            if (bSwitchChannel == TRUE)
            {
                AsicSwitchChannel(pAd, Channel, FALSE);
            }

            if (pAd->HwCfg.MACVersion == 0x28600100)
            {
                RTUSBWriteBBPRegister(pAd, BBP_R69, 0x1A);
                RTUSBWriteBBPRegister(pAd, BBP_R70, 0x0A);
                RTUSBWriteBBPRegister(pAd, BBP_R73, 0x16);
            }
        }
        else if (ExtensionChannelOffset == EXTCHA_BELOW)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: BW40, EXTCHA_BELOW, Channel = %d\n", __FUNCTION__, Channel));
            
            //
            // Tx band selection
            //
            // 0: use lower 40MHz band in 20MHz Tx
            // 1: use upper 40MHz band in 20MHz Tx
            //
            RTUSBReadMACRegister(pAd, TX_BAND_CFG, &MacValue);
            MacValue = ((MacValue & ~0x00000001) | 0x00000001);
            RTUSBWriteMACRegister(pAd, TX_BAND_CFG, MacValue);

            //
            // Tx/Rx bandwidth control
            //
            // 0: 20MHz
            // 1: 10MHz (11J)
            // 2: 40MHz
            //
            RTUSBReadBBPRegister(pAd, BBP_R4, &BbValue);
            BbValue = ((BbValue & ~0x18) | 0x10);
            RTUSBWriteBBPRegister(pAd, BBP_R4, BbValue);

            //
            // Primary channel selectoin
            //
            // 0: Primary channel - lower frequency channel
            // 1: Primary channel - higher frequency channel
            //
            RTUSBReadBBPRegister(pAd, BBP_R3, &BbValue);
            BbValue = ((BbValue & ~0x20) | 0x20);
            RTUSBWriteBBPRegister(pAd, BBP_R3, BbValue);
            pAd->StaCfg.BBPR3 = BbValue;

            if (bSwitchChannel == TRUE)
            {
                AsicSwitchChannel(pAd, Channel, FALSE);
            }

            if (pAd->HwCfg.MACVersion == 0x28600100)
            {
                RTUSBWriteBBPRegister(pAd, BBP_R69, 0x1A);
                RTUSBWriteBBPRegister(pAd, BBP_R70, 0x0A);
                RTUSBWriteBBPRegister(pAd, BBP_R73, 0x16);
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Invalid ExtensionChannelOffset = %d\n", __FUNCTION__, ExtensionChannelOffset));
            ASSERT(FALSE);
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Invalid Bw = %d\n", __FUNCTION__, Bw));
        ASSERT(FALSE);
    }   
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
#endif  
}

//
// Set the Tx DAC control based on the number of Tx paths
//
VOID BbSetTxDacCtrlByTxPath(
    IN PMP_ADAPTER pAd)
{
}

//
// Set the Tx DAC control
//
VOID BbSetTxDacCtrl(
    IN PMP_ADAPTER pAd, 
    IN UCHAR DacCtrl) // DAC_XXX
{
    UCHAR BbValue = 0;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    if (EXT_BB_CAPABLE(pAd))
    {
        ULONG ExtBbValue = 0;

        // DAC setting use the value of Block_IBI Offset_R9 bit7 of BBP register to decide:
        //   For non-perPavket case (bit7=0), Set configure to BBP_TXB_R5
        //   For perPavket case (bit7=1), Set configure to TxStreamMode of TxWI
        BB_READ32(pAd, BB_BLOCK_TX_BACKEND, BB_OFFSET_R5, &ExtBbValue);
        switch (DacCtrl)
        {
      
            case DAC_0: 
            {
                BB_BITWISE_WRITE(BbValue, (BIT1 + BIT0), 0x00); // DAC 0
            }
            break;

            case DAC_1: 
            {
                BB_BITWISE_WRITE(BbValue, (BIT1 + BIT0), BIT0); // DAC 1 
            }
            break;

            case DAC_0_AND_DAC_1: 
            {
                BB_BITWISE_WRITE(BbValue, (BIT1 + BIT0), BIT1 + BIT0); // Both DAC 0 and DAC 1
            }
            break;

            default: 
            {
                BB_BITWISE_WRITE(BbValue, (BIT1 + BIT0), 0x00); // DAC 0
            }
            break;
        }
        BB_WRITE32(pAd, BB_BLOCK_TX_BACKEND, BB_OFFSET_R5, ExtBbValue);          
    }
    else
    {
        RTUSBReadBBPRegister(pAd, BBP_R1, &BbValue);
        
        switch (DacCtrl)
        {
            case DAC_0: 
            {
                BbValue = ((BbValue & ~0x18) | 0x00); // DAC 0
            }
            break;

            case DAC_1: 
            {
                BbValue = ((BbValue & ~0x18) | 0x08); // DAC 1
            }
            break;

            case DAC_0_AND_DAC_1: 
            {
                BbValue = ((BbValue & ~0x18) | 0x10); // Both DAC 0 and DAC 1
            }
            break;

            default: 
            {
                BbValue = ((BbValue & ~0x18) | 0x00); // DAC 0
            }
            break;
        }

        RTUSBWriteBBPRegister(pAd, BBP_R1, BbValue);
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Set Rx antenna
//
VOID BbSetRxAnt(
    IN PMP_ADAPTER pAd, 
    IN UCHAR RxAntNumber) // RX_ANT_XXX
{
    UCHAR BbValue = 0;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    if (EXT_BB_CAPABLE(pAd))
    {
        ULONG ExtBbValue;

        BB_READ32(pAd, BB_BLOCK_AGC, BB_OFFSET_R0, &ExtBbValue);
        switch (pAd->HwCfg.Antenna.field.RxPath)
        {
            case 1: 
            {
                BB_BITWISE_WRITE(ExtBbValue, (BIT4 + BIT3), 0x00);  // 1R
            }
            break;

            case 2: 
            {
                BB_BITWISE_WRITE(ExtBbValue, (BIT4 + BIT3), BIT3);  // 2R
            }
            break;

            case 3: 
            {
                BB_BITWISE_WRITE(ExtBbValue, (BIT4 + BIT3), BIT4);  // 3R
            }
            break;

            default: 
            {
                BB_BITWISE_WRITE(ExtBbValue, (BIT4 + BIT3), 0x00);  // 1R
            }
            break;
        }
        BB_WRITE32(pAd, BB_BLOCK_AGC, BB_OFFSET_R0, ExtBbValue);
        pAd->StaCfg.BBPR3 = BB_GET_BYTE0(ExtBbValue);
    }
    else
    {
        RTUSBReadBBPRegister(pAd, BBP_R3, &BbValue);
        
        switch (RxAntNumber)
        {                               
            case RX_ANT_1R: 
            {
                BbValue = ((BbValue & ~0x18) | 0x00); // 1R
            }
            break;

            case RX_ANT_2R: 
            {
                BbValue = ((BbValue & ~0x18) | 0x08); // 2R
            }
            break;

            case RX_ANT_3R: 
            {
                BbValue = ((BbValue & ~0x18) | 0x10); // 3R
            }
            break;

            default: 
            {
                BbValue = ((BbValue & ~0x18) | 0x00); // 1R
            }
            break;
        }

        RTUSBWriteBBPRegister(pAd, BBP_R3, BbValue);
        pAd->StaCfg.BBPR3 = BbValue;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Set Rx ADC
//
VOID BbSetRxAdc(
    IN PMP_ADAPTER pAd, 
    IN UCHAR AdcCtrl) // ADC_XXX
{
    UCHAR BbValue = 0;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    if (EXT_BB_CAPABLE(pAd))
    {
        //2 TODO: add
        //2 TODO: pAd->StaCfg.BBPR3???
    }
    else
    {
        RTUSBReadBBPRegister(pAd, BBP_R3, &BbValue);
        
        switch (AdcCtrl)
        {                               
            case ADC_0: 
            {
                BbValue = ((BbValue & ~0x03) | 0x00); // ADC 0
            }
            break;

            case ADC_1: 
            {
                BbValue = ((BbValue & ~0x03) | 0x01); // ADC 1
            }
            break;

            default: 
            {
                BbValue = ((BbValue & ~0x03) | 0x00); // ADC 0
            }
            break;
        }

        RTUSBWriteBBPRegister(pAd, BBP_R3, BbValue);
        pAd->StaCfg.BBPR3 = BbValue;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Update Rx AGC VGA value for long distance
//
VOID BbUpdateRxAgcVgaForLongDistance(
    IN PMP_ADAPTER pAd) // Update for long distance
{
    UCHAR VgaValue = 0;

    if (pAd->HwCfg.VhtRxAgcVgaTuningCtrl.bEnableVhtRxAgcVgaTuning == FALSE)
    {
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// Turn on MLD for 2x2 NIC
//
VOID BbSetMldFor2Stream(
    IN PMP_ADAPTER pAd, 
    IN BOOLEAN bEnable)
{
    BBP_R105_STRUC BbR105Value = {0};
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    if (EXT_BB_CAPABLE(pAd))
    {
        ULONG   BbValue = 0x00; 

        BB_READ32(pAd, BB_BLOCK_RX_OFDM, BB_OFFSET_R15, &BbValue);

        if (bEnable == TRUE)
        {
            BB_BITWISE_WRITE(BbValue, (BIT2), (BIT2));
        }
        else
        {
            BB_BITWISE_WRITE(BbValue, (BIT2), (0x00));
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s: MLDFor2Stream = %d\n", 
                    __FUNCTION__, 
                    BB_BITMASK_READ(BbValue, (BIT2)) ));
        
        BB_WRITE32(pAd, BB_BLOCK_RX_OFDM, BB_OFFSET_R15, BbValue);

         
    }
    else
    {
        RTUSBReadBBPRegister(pAd, BBP_R105, &BbR105Value.byte);

        if (bEnable == TRUE)
        {
            BbR105Value.field.MLDFor2Stream = 1;
        }
        else
        {
            BbR105Value.field.MLDFor2Stream = 0;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s: MLDFor2Stream = %d\n", __FUNCTION__, BbR105Value.field.MLDFor2Stream));

        RTUSBWriteBBPRegister(pAd, BBP_R105, BbR105Value.byte);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

//
// In VHT, use primary ch, BW, VhtCentralChannel to calculate Primary channel location
//
VOID GetPrimaryChannelLocation(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           PrimaryChannel,     // Primary Channel Index
    IN  UCHAR           BW,                 // Bandwidth
    IN  UCHAR           CentralChannel,     // Central Channel Index
    OUT PUCHAR          pPrimaryChLocation)
{
    DBGPRINT(RT_DEBUG_LOUD, ("%s: -->\n", __FUNCTION__));

    if(BW == BW_40)
    {
        if(CentralChannel - PrimaryChannel == 2)
            *pPrimaryChLocation = PRI_CH_LOWER_20;
        else if(CentralChannel - PrimaryChannel == -2)
            *pPrimaryChLocation = PRI_CH_HIGHER_20;            
    }
    else if(BW == BW_80)
    {       
        if(CentralChannel - PrimaryChannel == -6)
            *pPrimaryChLocation = PRI_CH_HIGHER_30;
        else if(CentralChannel - PrimaryChannel == -2)
            *pPrimaryChLocation = PRI_CH_HIGHER_10;            
        else if(CentralChannel - PrimaryChannel == 2)
            *pPrimaryChLocation = PRI_CH_LOWER_10;
        else if(CentralChannel - PrimaryChannel == 6)
            *pPrimaryChLocation = PRI_CH_LOWER_30; 
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, ERROR!! invalid BW(=%d)\n",__FUNCTION__,__LINE__,BW));
    }
    
    DBGPRINT(RT_DEBUG_LOUD, ("%s: <--\n", __FUNCTION__));
}

//////////////////////////////////// End of File ////////////////////////////////////

