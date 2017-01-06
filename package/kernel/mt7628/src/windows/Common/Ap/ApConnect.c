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
 
    Module Name:
    connect.c

    Abstract:
    Routines to deal Link UP/DOWN and build/update BEACON frame contents

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "MtConfig.h"

/*
    ==========================================================================
    Description:
        Update the BEACON frame in the shared memory. Because TIM IE is variable
        length. other IEs after TIM has to shift and total frame length may change
        for each BEACON period.
    Output:
        pPort->SoftAP.ApCfg.CapabilityInfo
        pPort->SoftAP.ApCfg.ErpIeContent
    ==========================================================================
*/
VOID ApConnectUpdateBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort) 
{
    PUCHAR  pBeaconFrame = pAd->pTxCfg->BeaconBuf;
    UCHAR   *ptr;
    ULONG   FrameLen = pPort->SoftAP.ApCfg.TimIELocationInBeacon;
    ULONG   longptr;
    ULONG   UpdatePos = pPort->SoftAP.ApCfg.CapabilityInfoLocationInBeacon;

    UCHAR   byte0 = (UCHAR)(pPort->SoftAP.ApCfg.TimBitmap & 0x000000fe);  // skip AID#0
    UCHAR   byte1 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0x0000ff00) >> 8);
    UCHAR   byte2 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0x00ff0000) >> 16);
    UCHAR   byte3 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap & 0xff000000) >> 24);
    UCHAR   byte4 = (UCHAR)(pPort->SoftAP.ApCfg.TimBitmap2 & 0x000000ff);
    UCHAR   byte5 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0x0000ff00) >> 8);
    UCHAR   byte6 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0x00ff0000) >> 16);
    UCHAR   byte7 = (UCHAR)((pPort->SoftAP.ApCfg.TimBitmap2 & 0xff000000) >> 24);
    UINT    i;
    //HTTRANSMIT_SETTING    BeaconTransmit;   // MGMT frame PHY rate setting when operatin at Ht rate.
    PHY_CFG BeaconPhyCfg = {0};
    UCHAR   RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};       
    ULONG   TmpLen; 
    ADD_HT_INFO_IE  AddHTInfo;  // Useful as AP.    

    DBGPRINT(RT_DEBUG_INFO,("%s=>\n",__FUNCTION__));
    DBGPRINT(RT_DEBUG_INFO, ("%s base = %x\n",__FUNCTION__,pPort->HwBeaconBase));
    if (pPort->HwBeaconBase == 0)
    {
        return;
    }

    
    PlatformMoveMemory(&AddHTInfo, &pPort->SoftAP.ApCfg.AddHTInfoIe, sizeof(ADD_HT_INFO_IE));
    AddHTInfo.ControlChan = pPort->Channel;
    
    //
    // step 1 - update BEACON's Capability
    //
    ptr = pBeaconFrame + pPort->SoftAP.ApCfg.CapabilityInfoLocationInBeacon;
    *ptr = (UCHAR)(pPort->SoftAP.ApCfg.CapabilityInfo & 0x00ff);
    *(ptr+1) = (UCHAR)((pPort->SoftAP.ApCfg.CapabilityInfo & 0xff00) >> 8);

    //
    // TIM IE order: 10
    //
    // step 2 - update TIM IE
    // TODO: enlarge TIM bitmap to support up to 64 STAs
    // TODO: re-measure if RT2600 TBTT interrupt happens faster than BEACON sent out time
    //
    if (pPort->SoftAP.ApCfg.DtimCount == 0)
        pPort->SoftAP.ApCfg.DtimCount = pPort->SoftAP.ApCfg.DtimPeriod - 1;
    else
        pPort->SoftAP.ApCfg.DtimCount -= 1;

    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only support 11b, not to include TIM
        ;
    }
    else
    {
        ptr = pBeaconFrame + pPort->SoftAP.ApCfg.TimIELocationInBeacon;
        *ptr = IE_TIM;
        *(ptr + 2) = pPort->SoftAP.ApCfg.DtimCount;
        *(ptr + 3) = pPort->SoftAP.ApCfg.DtimPeriod;

        if (byte0 || byte1) // there's some backlog frame for AID 1-15
        {
            *(ptr + 4) = 0;      // Virtual TIM bitmap stars from AID #0
            *(ptr + 5) = byte0;
            *(ptr + 6) = byte1;
            *(ptr + 7) = byte2;
            *(ptr + 8) = byte3;
            *(ptr + 9) = byte4;
            *(ptr + 10) = byte5;
            *(ptr + 11) = byte6;
            *(ptr + 12) = byte7;
            if (byte7)      *(ptr + 1) = 11; // IE length
            else if (byte6) *(ptr + 1) = 10; // IE length
            else if (byte5) *(ptr + 1) = 9;  // IE length
            else if (byte4) *(ptr + 1) = 8;  // IE length
            else if (byte3) *(ptr + 1) = 7;  // IE length
            else if (byte2) *(ptr + 1) = 6;  // IE length
            else if (byte1) *(ptr + 1) = 5;  // IE length
            else            *(ptr + 1) = 4;  // IE length
        }
        else if (byte2 || byte3) // there's some backlogged frame for AID 16-31
        {
            *(ptr + 4) = 2;      // Virtual TIM bitmap starts from AID #16
            *(ptr + 5) = byte2;
            *(ptr + 6) = byte3;
            *(ptr + 7) = byte4;
            *(ptr + 8) = byte5;
            *(ptr + 9) = byte6;
            *(ptr + 10) = byte7;
            if (byte7)      *(ptr + 1) = 9; // IE length
            else if (byte6) *(ptr + 1) = 8; // IE length
            else if (byte5) *(ptr + 1) = 7; // IE length
            else if (byte4) *(ptr + 1) = 6; // IE length
            else if (byte3) *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else if (byte4 || byte5) // there's some backlogged frame for AID 32-47
        {
            *(ptr + 4) = 4;      // Virtual TIM bitmap starts from AID #32
            *(ptr + 5) = byte4;
            *(ptr + 6) = byte5;
            *(ptr + 7) = byte6;
            *(ptr + 8) = byte7;
            if (byte7)      *(ptr + 1) = 7; // IE length
            else if (byte6) *(ptr + 1) = 6; // IE length
            else if (byte5) *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else if (byte6 || byte7) // there's some backlogged frame for AID 48-63
        {
            *(ptr + 4) = 6;      // Virtual TIM bitmap starts from AID #48
            *(ptr + 5) = byte6;
            *(ptr + 6) = byte7;
            if (byte7)      *(ptr + 1) = 5; // IE length
            else            *(ptr + 1) = 4; // IE length
        }
        else // no backlogged frames
        {
            *(ptr + 1) = 4; // IE length
            *(ptr + 4) = 0;
            *(ptr + 5) = 0;
        }

        // bit0 means backlogged mcast/bcast
        *(ptr + 4) |= (pPort->SoftAP.ApCfg.TimBitmap & 0x01); 

        // adjust BEACON length according to the new TIM
        FrameLen += (2 + *(ptr+1));
    }
    
    //
    // Channel Switch Announce order: 15
    //  
    // fill up Channel Switch Announcement Element
    //
    if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11A) && (pPort->CommonCfg.bIEEE80211H == 1) && (pPort->CommonCfg.RadarDetect.RDMode == RD_SWITCHING_MODE))
    {
        ptr = pBeaconFrame + FrameLen;
        *ptr = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
        *(ptr + 1) = 3;
        *(ptr + 2) = 1;
        *(ptr + 3) = pPort->Channel;
        *(ptr + 4) = pPort->CommonCfg.RadarDetect.CSCount;
        ptr      += 5;
        FrameLen += 5;
    }

    //
    // ERP information order: 19
    //
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        //CH14 only support 11b, not to include ERP
        ;
    }
    else
    {
        // Update ERP and extended tx rate
        if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11BG_MIXED) || (pPort->SoftAP.ApCfg.PhyMode == PHY_11G) 
            || (pPort->SoftAP.ApCfg.PhyMode == PHY_11BGN_MIXED) ||(pPort->SoftAP.ApCfg.PhyMode == PHY_11GN_MIXED)
            || (pPort->SoftAP.ApCfg.PhyMode == PHY_11N))
        {
            //
            // fill ERP IE
            // 
            ptr = pBeaconFrame + FrameLen;
            *ptr = IE_ERP;
            *(ptr + 1) = 1;
            *(ptr + 2) = pPort->SoftAP.ApCfg.ErpIeContent;
            ptr      += 3;
            FrameLen += 3;
        }
    }

    //
    // add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back, Byte0.b2 for RDG
    if (pPort->CommonCfg.bAggregationCapable)
    {
        RalinkSpecificIe[5] |= 0x01;
    }
    if( pPort->CommonCfg.bPiggyBackCapable  )
    {
        RalinkSpecificIe[5] |= 0x02;        
    }
    if( pPort->CommonCfg.bRdg  )
    {
        RalinkSpecificIe[5] |= 0x04;        
    }   
    
    MakeOutgoingFrame(pBeaconFrame+FrameLen,    &TmpLen,
                      9,                         RalinkSpecificIe,
                      END_OF_ARGS);
    FrameLen += TmpLen; 

    //
    // HT capabilities order: 37
    //
    // step 5. Update HT. Since some fields might change in the same BSS.
    //
    if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)) 
    {
        //CH14 only supports 11B and will not carry any HT info on beacons
        ;
    }
    else
    {
        if (pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED)
        {
            ULONG HtLen, HtLen1;
            ULONG VhtCapIeLen, VhtOpIeLen;
            UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
            UCHAR BROADCOM2[4] = {0x0, 0x90, 0x4c, 0x34};
            HT_CAPABILITY_IE HtCapability;
            PlatformMoveMemory(&HtCapability, &pPort->SoftAP.ApCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            if (pAd->HwCfg.Antenna.field.RxPath > 1)
            {
                HtCapability.MCSSet[1] = 0xff;
            }
            else
            {
                HtCapability.MCSSet[1] = 0x00;
            }

            // add HT Capability IE 
            HtLen = sizeof(pPort->SoftAP.ApCfg.HtCapability);
            HtLen1 = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe);
            MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                                      1,                                &HtCapIe,
                                      1,                                &HtLen,
                                     HtLen,          &HtCapability, 
                                      1,                                &AddHtInfoIe,
                                      1,                                &HtLen1,
                                     HtLen1,          &AddHTInfo, 
                              END_OF_ARGS);
            FrameLen += TmpLen;
            
            // add IEs for VHT
            if (pPort->Channel>14)
            {
                if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                {
                    ULONG TmpLen;
                    VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                    MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
                                        1, &VhtCapIe,
                                        1, &VhtCapIeLen,
                                        SIZE_OF_VHT_CAP_IE, &pPort->SoftAP.ApCfg.VhtCapability,
                                        END_OF_ARGS);
                    FrameLen += TmpLen;

                    // add VHT_OP_IE for VHT
                    VhtOpIeLen = SIZE_OF_VHT_OP_IE;
                    MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
                                        1, &VhtOpIe,
                                        1, &VhtOpIeLen,
                                        SIZE_OF_VHT_OP_IE,  &pPort->SoftAP.ApCfg.VhtOperation,
                                        END_OF_ARGS);
                    FrameLen += TmpLen;
                }
            }
            
            {
                HtLen = SIZE_HT_CAP_IE + 4;
                HtLen1 = sizeof(pPort->SoftAP.ApCfg.AddHTInfoIe) + 4;
                MakeOutgoingFrame(pBeaconFrame + FrameLen,            &TmpLen,
                              1,                                &WpaIe,
                              1,                                &HtLen,
                              4,                                &BROADCOM[0],
                             (HtLen-4),          &HtCapability, 
                              1,                                &WpaIe,
                              1,                                &HtLen1,
                              4,                                &BROADCOM2[0],
                             (HtLen1-4),          &AddHTInfo, 
                              END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
    }
    
    if(pPort->PortCfg.AdditionalBeaconIESize > 0)
    {
        MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                          pPort->PortCfg.AdditionalBeaconIESize ,     pPort->PortCfg.AdditionalBeaconIEData,
                          END_OF_ARGS);

        FrameLen += TmpLen;
    }

    pAd->pTxCfg->BeaconBufLen = FrameLen;
    //
    // step 6. Since FrameLen may change, update TXWI.
    //
    // Update in real buffer
    // Update sw copy.  
    if (pPort->Channel <= 14)
    {
        WRITE_PHY_CFG_MODE(pAd, &BeaconPhyCfg, MODE_CCK);
    }       
    else
    {
        WRITE_PHY_CFG_MODE(pAd, &BeaconPhyCfg, MODE_OFDM);
    }
    
    XmitWriteTxWI(pAd, pPort, &pAd->pTxCfg->BeaconTxWI, FALSE, TRUE, FALSE, FALSE, TRUE, 0, RESERVED_WCID, 
        FrameLen, PID_MGMT, 0, IFS_HTTXOP, FALSE, BeaconPhyCfg, FALSE, TRUE, FALSE);
    //
    // step 6. move BEACON TXWI and frame content to on-chip memory
    //

    ptr = (PUCHAR)&pAd->pTxCfg->BeaconTxWI;
    for (i = 0; i < pAd->HwCfg.TXWI_Length; )  // 24-byte TXINFO field
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + i, longptr);
        ptr += 4;
        i+=4;
    }   

    ptr = pAd->pTxCfg->BeaconBuf;
    if (FrameLen > UpdatePos)
    {
        for (i = 0; i < FrameLen; )
        {
            longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
            RTUSBWriteMACRegister(pAd, pPort->HwBeaconBase + pAd->HwCfg.TXWI_Length + i, longptr);
            ptr += 4;
            i+=4;
        }
    }
}

