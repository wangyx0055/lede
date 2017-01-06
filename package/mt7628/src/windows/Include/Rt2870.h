/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2005, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rt2870.h

    Abstract:
    RT2860usb ASIC  related definition & structures

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Jan Lee 02-13-2006    created

*/

#ifndef __RT2870_H__
#define __RT2870_H__
///////////////////////////
// reg for RT2860usb ///////////
//////////////////////////

//
// PCI registers - base address 0x0000
//
#define PCI_CFG         0x0000
#define PCI_EECTRL          0x0004
#define PCI_MCUCTRL         0x0008 

#define OPT_14          0x114

//
// SCH/DMA registers - base address 0x0200
//
// INT_SOURCE_CSR: Interrupt source register. Write one to clear corresponding bit
//
#define DMA_CSR0      0x200
#define INT_SOURCE_CSR      0x200


//
// INT_MASK_CSR:   Interrupt MASK register.   1: the interrupt is mask OFF
//
#define INT_MASK_CSR        0x204
#ifdef BIG_ENDIAN
typedef union   _INT_MASK_CSR_STRUC {
    struct  {
        ULONG           TxCoherent:1;
        ULONG           RxCoherent:1;
        ULONG           :20;
        ULONG           MCUCommandINT:1;
        ULONG           MgmtDmaDone:1;
        ULONG           HccaDmaDone:1;
        ULONG           Ac3DmaDone:1;
        ULONG           Ac2DmaDone:1;
        ULONG           Ac1DmaDone:1;
        ULONG           Ac0DmaDone:1;
        ULONG           RxDone:1;
        ULONG           TxDelay:1;
        ULONG           RXDelay_INT_MSK:1;
    }   field;
    ULONG           word;
}INT_MASK_CSR_STRUC, *PINT_MASK_CSR_STRUC;
#else
typedef union   _INT_MASK_CSR_STRUC {
    struct  {
        ULONG           RXDelay_INT_MSK:1;
        ULONG           TxDelay:1;
        ULONG           RxDone:1;
        ULONG           Ac0DmaDone:1;
        ULONG           Ac1DmaDone:1;
        ULONG           Ac2DmaDone:1;
        ULONG           Ac3DmaDone:1;
        ULONG           HccaDmaDone:1;
        ULONG           MgmtDmaDone:1;
        ULONG           MCUCommandINT:1;
        ULONG           :20;
        ULONG           RxCoherent:1;
        ULONG           TxCoherent:1;
    }   field;
    ULONG           word;
} INT_MASK_CSR_STRUC, *PINT_MASK_CSR_STRUC;
#endif
#define WPDMA_GLO_CFG   0x208
#ifdef BIG_ENDIAN
typedef union   _WPDMA_GLO_CFG_STRUC    {
    struct  {
        ULONG           :16;
        ULONG           RXHdrScater:8;
        ULONG           BigEndian:1;
        ULONG           EnTXWriteBackDDONE:1;
        ULONG           WPDMABurstSIZE:2;
        ULONG           RxDMABusy:1;
        ULONG           EnableRxDMA:1;
        ULONG           TxDMABusy:1;
        ULONG           EnableTxDMA:1;
    }   field;
    ULONG           word;
}WPDMA_GLO_CFG_STRUC, *PWPDMA_GLO_CFG_STRUC;
#else
typedef union   _WPDMA_GLO_CFG_STRUC    {
    struct  {
        ULONG           EnableTxDMA:1;
        ULONG           TxDMABusy:1;
        ULONG           EnableRxDMA:1;
        ULONG           RxDMABusy:1;
        ULONG           WPDMABurstSIZE:2;
        ULONG           EnTXWriteBackDDONE:1;
        ULONG           BigEndian:1;
        ULONG           RXHdrScater:8;
        ULONG           :16;
    }   field;
    ULONG           word;
} WPDMA_GLO_CFG_STRUC, *PWPDMA_GLO_CFG_STRUC;
#endif
#define WPDMA_RST_IDX   0x20c
#ifdef BIG_ENDIAN
typedef union   _WPDMA_RST_IDX_STRUC    {
    struct  {
        ULONG           :15;
        ULONG           RST_DRX_IDX0:1;
        ULONG           rsv:10;
        ULONG           RST_DTX_IDX5:1;
        ULONG           RST_DTX_IDX4:1;
        ULONG           RST_DTX_IDX3:1;
        ULONG           RST_DTX_IDX2:1;
        ULONG           RST_DTX_IDX1:1;
        ULONG           RST_DTX_IDX0:1;
    }   field;
    ULONG           word;
}WPDMA_RST_IDX_STRUC, *PWPDMA_RST_IDX_STRUC;
#else
typedef union   _WPDMA_RST_IDX_STRUC    {
    struct  {
        ULONG           RST_DTX_IDX0:1;
        ULONG           RST_DTX_IDX1:1;
        ULONG           RST_DTX_IDX2:1;
        ULONG           RST_DTX_IDX3:1;
        ULONG           RST_DTX_IDX4:1;
        ULONG           RST_DTX_IDX5:1;
        ULONG           rsv:10;
        ULONG           RST_DRX_IDX0:1;
        ULONG           :15;
    }   field;
    ULONG           word;
} WPDMA_RST_IDX_STRUC, *PWPDMA_RST_IDX_STRUC;
#endif
#define DELAY_INT_CFG  0x0210
#ifdef BIG_ENDIAN
typedef union   _DELAY_INT_CFG_STRUC    {
    struct  {
        ULONG           TXDLY_INT_EN:1;
        ULONG           TXMAX_PINT:7;
        ULONG           TXMAX_PTIME:8;
        ULONG           RXDLY_INT_EN:1;
        ULONG           RXMAX_PINT:7;
        ULONG           RXMAX_PTIME:8;
    }   field;
    ULONG           word;
}DELAY_INT_CFG_STRUC, *PDELAY_INT_CFG_STRUC;
#else
typedef union   _DELAY_INT_CFG_STRUC    {
    struct  {
        ULONG           RXMAX_PTIME:8;
        ULONG           RXMAX_PINT:7;
        ULONG           RXDLY_INT_EN:1;
        ULONG           TXMAX_PTIME:8;
        ULONG           TXMAX_PINT:7;
        ULONG           TXDLY_INT_EN:1;
    }   field;
    ULONG           word;
} DELAY_INT_CFG_STRUC, *PDELAY_INT_CFG_STRUC;
#endif


//
// AC_TXOP_CSR0: AC_BK/AC_BE TXOP register
//
#define WMM_TXOP0_CFG    0x0220
#ifdef BIG_ENDIAN
typedef union   _AC_TXOP_CSR0_STRUC {
    struct  {
        USHORT  Ac1Txop;        // for AC_BE, in unit of 32us
        USHORT  Ac0Txop;        // for AC_BK, in unit of 32us
    }   field;
    ULONG           word;
}   AC_TXOP_CSR0_STRUC, *PAC_TXOP_CSR0_STRUC;
#else
typedef union   _AC_TXOP_CSR0_STRUC {
    struct  {
        USHORT  Ac0Txop;        // for AC_BK, in unit of 32us
        USHORT  Ac1Txop;        // for AC_BE, in unit of 32us
    }   field;
    ULONG           word;
}   AC_TXOP_CSR0_STRUC, *PAC_TXOP_CSR0_STRUC;
#endif

//
// AC_TXOP_CSR1: AC_VO/AC_VI TXOP register
//
#define WMM_TXOP1_CFG    0x0224
#ifdef BIG_ENDIAN
typedef union   _AC_TXOP_CSR1_STRUC {
    struct  {
        USHORT  Ac3Txop;        // for AC_VO, in unit of 32us
        USHORT  Ac2Txop;        // for AC_VI, in unit of 32us
    }   field;
    ULONG           word;
}   AC_TXOP_CSR1_STRUC, *PAC_TXOP_CSR1_STRUC;
#else
typedef union   _AC_TXOP_CSR1_STRUC {
    struct  {
        USHORT  Ac2Txop;        // for AC_VI, in unit of 32us
        USHORT  Ac3Txop;        // for AC_VO, in unit of 32us
    }   field;
    ULONG           word;
}   AC_TXOP_CSR1_STRUC, *PAC_TXOP_CSR1_STRUC;
#endif
#define RINGREG_DIFF            0x10
#define GPIO_CTRL_CFG    0x0228 //MAC_CSR13

#define TX_BASE_PTR0     0x0230 //AC_BK base address
#define TX_MAX_CNT0      0x0234
#define TX_CTX_IDX0       0x0238
#define TX_DTX_IDX0      0x023c
#define TX_BASE_PTR1     0x0240     //AC_BE base address
#define TX_MAX_CNT1      0x0244
#define TX_CTX_IDX1       0x0248
#define TX_DTX_IDX1      0x024c
#define TX_BASE_PTR2     0x0250     //AC_VI base address
#define TX_MAX_CNT2      0x0254
#define TX_CTX_IDX2       0x0258
#define TX_DTX_IDX2      0x025c
#define TX_BASE_PTR3     0x0260     //AC_VO base address
#define TX_MAX_CNT3      0x0264
#define TX_CTX_IDX3       0x0268
#define TX_DTX_IDX3      0x026c
#define TX_BASE_PTR4     0x0270     //HCCA base address
#define TX_MAX_CNT4      0x0274
#define TX_CTX_IDX4       0x0278
#define TX_DTX_IDX4      0x027c
#define TX_BASE_PTR5     0x0280     //MGMT base address
#define  TX_MAX_CNT5     0x0284
#define TX_CTX_IDX5       0x0288
#define TX_DTX_IDX5      0x028c
#define TX_MGMTMAX_CNT      TX_MAX_CNT5
#define TX_MGMTCTX_IDX       TX_CTX_IDX5
#define TX_MGMTDTX_IDX      TX_DTX_IDX5
#define RX_BASE_PTR     0x0290  //RX base address
#define RX_MAX_CNT      0x0294
#define RX_CRX_IDX       0x0298
#define RX_DRX_IDX      0x029c

#ifdef BIG_ENDIAN
typedef union   _USB_DMA_CFG_STRUC  {
    struct  {
        ULONG  TxBusy:1;    //USB DMA TX FSM busy . debug only
        ULONG  RxBusy:1;        //USB DMA RX FSM busy . debug only
        ULONG  EpoutValid:6;        //OUT endpoint data valid. debug only
        ULONG  TxBulkEn:1;        //Enable USB DMA Tx  
        ULONG  RxBulkEn:1;        //Enable USB DMA Rx  
        ULONG  RxBulkAggEn:1;        //Enable Rx Bulk Aggregation  
        ULONG  TxopHalt:1;        //Halt TXOP count down when TX buffer is full.
        ULONG  TxClear:1;        //Clear USB DMA TX path
        ULONG  rsv:2;        
        ULONG  phyclear:1;              //phy watch dog enable. write 1
        ULONG  RxBulkAggLmt:8;        //Rx Bulk Aggregation Limit  in unit of 1024 bytes
        ULONG  RxBulkAggTOut:8;        //Rx Bulk Aggregation TimeOut  in unit of 33ns
    }   field;
    ULONG           word;
}   USB_DMA_CFG_STRUC, *PUSB_DMA_CFG_STRUC;
#else
typedef union   _USB_DMA_CFG_STRUC  {
    struct  {
        ULONG  RxBulkAggTOut:8;        //Rx Bulk Aggregation TimeOut  in unit of 33ns
        ULONG  RxBulkAggLmt:8;        //Rx Bulk Aggregation Limit  in unit of 256 bytes
        ULONG  phyclear:1;              //phy watch dog enable. write 1
        ULONG  rsv:2;        
        ULONG  TxClear:1;        //Clear USB DMA TX path
        ULONG  TxopHalt:1;        //Halt TXOP count down when TX buffer is full.
        ULONG  RxBulkAggEn:1;        //Enable Rx Bulk Aggregation  
        ULONG  RxBulkEn:1;        //Enable USB DMA Rx  
        ULONG  TxBulkEn:1;        //Enable USB DMA Tx  
        ULONG  EpoutValid:6;        //OUT endpoint data valid
        ULONG  RxBusy:1;        //USB DMA RX FSM busy 
        ULONG  TxBusy:1;    //USB DMA TX FSM busy 
    }   field;
    ULONG           word;
}   USB_DMA_CFG_STRUC, *PUSB_DMA_CFG_STRUC;
#endif

#define USB_CYC_CFG      0x02a4

#ifdef BIG_ENDIAN
typedef union   _US_CYC_CNT_STRUC   {
    struct  {
        ULONG  rsv2:7;
        ULONG  TestEn:1;
        ULONG  TestSel:8;
        ULONG  rsv1:7;
        ULONG  BtModeEn:1;
        ULONG  UsCycCnt:8;
    }   field;
    ULONG           word;
}   US_CYC_CNT_STRUC, *PUS_CYC_CNT_STRUC;
#else
typedef union   _US_CYC_CNT_STRUC   {
    struct  {
        ULONG  UsCycCnt:8;
        ULONG  BtModeEn:1;
        ULONG  rsv1:7;
        ULONG  TestSel:8;
        ULONG  TestEn:1;
        ULONG  rsv2:7;
    }   field;
    ULONG           word;
}   US_CYC_CNT_STRUC, *PUS_CYC_CNT_STRUC;
#endif

#define IOT_STRUC_INTEL_4965 0x01

//
//  3  PBF  registers  
//
//
// Most are for debug. Driver doesn't touch PBF register.
#define     PBF_SYS_CTRL     0x0400

#ifdef BIG_ENDIAN
typedef union _PBF_SYS_CTRL_STRUC
{
    struct
    {
        ULONG   Reserved5:12; // Reserved
        ULONG   SHR_MSEL:1; // Shared memory access selection
        ULONG   PBF_MSEL:2; // Packet buffer memory access selection
        ULONG   HST_PM_SEL:1; // The write selection of the host program RAM
        ULONG   Reserved4:1; // Reserved
        ULONG   CAP_MODE:1; // Packet buffer capture mode
        ULONG   Reserved3:1; // Reserved
        ULONG   CLK_SEL:1; // MAC/PBF clock source selection
        ULONG   PBF_CLK_EN:1; // PBF clock enable
        ULONG   MAC_CLK_EN:1; // MAC clock enable
        ULONG   DMA_CLK_EN:1; // DMA clock enable
        ULONG   Reserved2:1; // Reserved
        ULONG   MCU_READY:1; // MCU ready
        ULONG   Reserved1:2; // Reserved
        ULONG   ASY_RESET:1; // ASYNC interface reset
        ULONG   PBF_RESET:1; // PBF hardware reset
        ULONG   MAC_RESET:1; // MAC hardware reset
        ULONG   DMA_RESET:1; // DMA hardware reset
        ULONG   MCU_RESET:1; // MCU hardware reset
    } field;

    ULONG word;
} PBF_SYS_CTRL_STRUC, *PPBF_SYS_CTRL_STRUC;
#else
typedef union _PBF_SYS_CTRL_STRUC
{
    struct
    {       
        ULONG   MCU_RESET:1; // MCU hardware reset
        ULONG   DMA_RESET:1; // DMA hardware reset
        ULONG   MAC_RESET:1; // MAC hardware reset
        ULONG   PBF_RESET:1; // PBF hardware reset
        ULONG   ASY_RESET:1; // ASYNC interface reset
        ULONG   Reserved1:2; // Reserved
        ULONG   MCU_READY:1; // MCU ready
        ULONG   Reserved2:1; // Reserved
        ULONG   DMA_CLK_EN:1; // DMA clock enable
        ULONG   MAC_CLK_EN:1; // MAC clock enable
        ULONG   PBF_CLK_EN:1; // PBF clock enable
        ULONG   CLK_SEL:1; // MAC/PBF clock source selection
        ULONG   Reserved3:1; // Reserved
        ULONG   CAP_MODE:1; // Packet buffer capture mode
        ULONG   Reserved4:1; // Reserved
        ULONG   HST_PM_SEL:1; // The write selection of the host program RAM
        ULONG   PBF_MSEL:2; // Packet buffer memory access selection
        ULONG   SHR_MSEL:1; // Shared memory access selection
        ULONG   Reserved5:12; // Reserved
    } field;

    ULONG word;
} PBF_SYS_CTRL_STRUC, *PPBF_SYS_CTRL_STRUC;
#endif


#define     PBF_MAX_PCNT    0x040C
#define     PBF_CTRL        0x0410
#define     PBF_INT_STA     0x0414
#define     PBF_INT_ENA     0x0418
#define     RXQ_STA         0x0430

#ifdef BIG_ENDIAN
typedef union   _RXQ_STA_STRUC  {
    struct  {
        ULONG       Rsv:8;
        ULONG       RX0Q_PCNT:8;
        ULONG       Rsv:8;
        ULONG       R0Q_STA:8;
    }   field;
    ULONG           word;
}RXQ_STA_STRUC , *PRXQ_STA_STRUC ;
#else
typedef union   _RXQ_STA_STRUC {
    struct  {
        ULONG       R0Q_STA:8;
        ULONG       Rsv0:8;
        ULONG       RX0Q_PCNT:8;
        ULONG       Rsv1:8;
    }   field;
    ULONG           word;
} RXQ_STA_STRUC , *PRXQ_STA_STRUC ;
#endif

#define     TXRXQ_PCNT       0x0438
#define     PBF_DBG          0x043c
#define TXRXQ_STA       0x0434
#define PBF_CAP_CTRL        0x0440

#define CMB_CTRL            0x20
#ifdef BIG_ENDIAN
typedef union   _CMB_CTRL_STRUC {
    struct  {
        ULONG           LDO0_EN:1;
        ULONG           LDO3_EN:1;
        ULONG           LDO_BGSEL:2;        //LDO band-gap selection
        ULONG           LDO_CORE_LEVEL:4;   //LDO0/3 volrage level selection
        ULONG           PLL_LD:1;           //BBP PLL lock done
        ULONG           XTAL_RDY:1;     
        ULONG           Rsv:2;
        ULONG       LDO25_FRC_ON:1;//4              
        ULONG       LDO25_LARGEA:1;     
        ULONG       LDO25_LEVEL:2;      //LDO25 voltage level selection
        ULONG       AUX_OPT_Bit15_Two_AntennaMode:1;            // or one antenna mode          
        ULONG       AUX_OPT_Bit14_TRSW1_as_GPIO:1;          // or as BT TRSW        
        ULONG       AUX_OPT_Bit13_GPIO7_as_GPIO:1;          // or as BT_PA_EN       
        ULONG       AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;  // or WLAN TRSW     
        ULONG       AUX_OPT_Bit11_Rsv:1;        
        ULONG       AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1;         
        ULONG       AUX_OPT_Bit9_GPIO3_as_GPIO:1;               // or PCIe Wake     
        ULONG       AUX_OPT_Bit8_AuxPower_Exists:1;     
        ULONG       AUX_OPT_Bit7_KeepInterfaceClk:1;            // or slow down interface clock when only BT is enabled     
        ULONG       AUX_OPT_Bit6_KeepXtal_On:1;             // or power down Xtal when all functions are off        
        ULONG       AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;  // or despite BT on or off      
        ULONG       AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;    // or despite WLAN on or off
        ULONG       AUX_OPT_Bit3_PLLOn_L1:1;
        ULONG       AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
        ULONG       AUX_OPT_Bit1_PCIePhyClkOn_L1:1;     
        ULONG       AUX_OPT_Bit0_InterfaceClk_40Mhz:1;          // or enable interface clock power saving   
        //ULONG     AUX_OPT:16;     
    }field  ;
    ULONG           word;
}   CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#else
typedef union   _CMB_CTRL_STRUC {
    struct  {
        //ULONG     AUX_OPT:16;
        ULONG       AUX_OPT_Bit0_InterfaceClk_40Mhz:1;          // or enable interface clock power saving
        ULONG       AUX_OPT_Bit1_PCIePhyClkOn_L1:1;     
        ULONG       AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
        ULONG       AUX_OPT_Bit3_PLLOn_L1:1;
        ULONG       AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;    // or despite WLAN on or off
        ULONG       AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;  // or despite BT on or off
        ULONG       AUX_OPT_Bit6_KeepXtal_On:1;             // or power down Xtal when all functions are off
        ULONG       AUX_OPT_Bit7_KeepInterfaceClk:1;            // or slow down interface clock when only BT is enabled
        ULONG       AUX_OPT_Bit8_AuxPower_Exists:1;
        ULONG       AUX_OPT_Bit9_GPIO3_as_GPIO:1;               // or PCIe Wake
        ULONG       AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1; 
        ULONG       AUX_OPT_Bit11_Rsv:1;
        ULONG       AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;  // or WLAN TRSW
        ULONG       AUX_OPT_Bit13_GPIO7_as_GPIO:1;          // or as BT_PA_EN
        ULONG       AUX_OPT_Bit14_TRSW1_as_GPIO:1;          // or as BT TRSW
        ULONG       AUX_OPT_Bit15_Two_AntennaMode:1;            // or one antenna mode      
        ULONG       LDO25_LEVEL:2;      //LDO25 voltage level selection
        ULONG       LDO25_LARGEA:1;
        ULONG       LDO25_FRC_ON:1;//4      
        ULONG           Rsv:2;
        ULONG           XTAL_RDY:1;
        ULONG           PLL_LD:1;           //BBP PLL lock done
        ULONG           LDO_CORE_LEVEL:4;   //LDO0/3 volrage level selection
        ULONG           LDO_BGSEL:2;        //LDO band-gap selection
        ULONG           LDO3_EN:1;
        ULONG           LDO0_EN:1;
    }field;
    ULONG           word;
} CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#endif

#define OSCCTL              0x38
#ifdef BIG_ENDIAN
typedef union   _OSCCTL_STRUC   {
    struct  {
        ULONG           ROSC_EN:1;          //Ring oscilator enable
        ULONG           CAL_REQ:1;          //Ring oscilator calibration request        
        ULONG           CLK_32K_VLD:1;      //Ring oscilator 32KHz output clock valid
        ULONG       CAL_ACK:1;          //Ring oscillator calibration ack
        ULONG       CAL_CNT:12;         //Ring oscillator calibration counter result
        ULONG       Rsv:3;  
        ULONG       REF_CYCLE:13;       //Reference clock cycles to measure the clock divider's dividend
    }   field;
    ULONG           word;
}   OSCCTL_STRUC, *POSCCTL_STRUC;
#else
typedef union   _OSCCTL_STRUC   {
    struct  {
        ULONG       REF_CYCLE:13;       //Reference clock cycles to measure the clock divider's dividend
        ULONG       Rsv:3;
        ULONG       CAL_CNT:12;         //Ring oscillator calibration counter result
        ULONG       CAL_ACK:1;          //Ring oscillator calibration ack
        ULONG           CLK_32K_VLD:1;      //Ring oscilator 32KHz output clock valid
        ULONG           CAL_REQ:1;          //Ring oscilator calibration request
        ULONG           ROSC_EN:1;          //Ring oscilator enable
    }   field;
    ULONG           word;
} OSCCTL_STRUC, *POSCCTL_STRUC;
#endif

#define COEXCFG0            0x40
#ifdef BIG_ENDIAN
typedef union   _COEXCFG0_STRUC {
    struct  {
        ULONG           COEX_CFG1:8;        //Bluetooth/WiFi coexistence configuration for two antenna
        //ULONG     COEX_CFG1_Bit31_Rsv:1;
        //ULONG     COEX_CFG1_Bit30_FixWiFiBBPPwr_BTRx:1;
        //ULONG     COEX_CFG1_Bit29_Abort_LowPriorityWiFiTx_bt_rx_req_h:1;  
        //ULONG     COEX_CFG1_Bit28_LowerWiFiTxPwr_BTRx:1;
        //ULONG     COEX_CFG1_Bit27_LowerLowPriorityBTTx_WiFiRx:1;
        //ULONG     COEX_CFG1_Bit26_LowerHighPriorityBTTx_WiFiRx:1;
        //ULONG     COEX_CFG1_Bit25_BT_TRSW_BTIdle:1;   
        //ULONG     COEX_CFG1_Bit24_FixWiFiLNAGain_BTTx:1;          
        
        ULONG           COEX_CFG0:8;        //Bluetooth/WiFi coexistence configuration for one antenna
        //ULONG         COEX_CFG0_Bit23_Srv:1;
        //ULONG         COEX_CFG0_Bit22_FixWiFiBBPPwr_BTRx:1;   
        //ULONG     COEX_CFG0_Bit21_Abort_LowPriorityWiFiTx_bt_rx_req_h:1;  
        //ULONG     COEX_CFG0_Bit20_LowerWiFiTxPwr_BTRx:1;
        //ULONG     COEX_CFG0_Bit19_LowerLowPriorityBTTx_WiFiRx:1;
        //ULONG     COEX_CFG0_Bit18_LowerHighPriorityBTTx_WiFiRx:1;
        //ULONG     COEX_CFG0_Bit17_BT_TRSW_BTIdle:1;   
        //ULONG     COEX_CFG0_Bit16_FixWiFiLNAGain_BTTx:1;      
        
        ULONG           FIX_WL_RF_LNA:2;    //Fixed WLAN RF LNA gain for concurrent Tx/Rx
        ULONG       FIX_BT_H_PA:3;      //Fixed BT PA power when Tx high priority packet
        ULONG       FIX_BT_L_PA:3;      //Fixed BT PA power when Tx low priority packet
        ULONG       FIX_WL_TX_PWR:2;    //Fixed WLAN BBP Tx power
        ULONG       Rsv:3;
        ULONG       FIX_WL_ANT_EN:1;    //Enable to Force WLAN antenna
        ULONG       FIX_WL_DI_ANT:1;    //Force WLAN Rx to diversity antenna
        ULONG       COEX_ENT:1;         //Coexistence mode anable
    }field  ;
    ULONG           word;
}   COEXCFG0_STRUC, *PCOEXCFG0_STRUC;
#else
typedef union   _COEXCFG0_STRUC {
    struct  {
        ULONG       COEX_ENT:1;         //Coexistence mode anable
        ULONG       FIX_WL_DI_ANT:1;    //Force WLAN Rx to diversity antenna
        ULONG       FIX_WL_ANT_EN:1;    //Enable to Force WLAN antenna
        ULONG       Rsv:3;
        ULONG       FIX_WL_TX_PWR:2;    //Fixed WLAN BBP Tx power
        ULONG       FIX_BT_L_PA:3;      //Fixed BT PA power when Tx low priority packet
        ULONG       FIX_BT_H_PA:3;      //Fixed BT PA power when Tx high priority packet
        ULONG           FIX_WL_RF_LNA:2;    //Fixed WLAN RF LNA gain for concurrent Tx/Rx

        ULONG           COEX_CFG0:8;        //Bluetooth/WiFi coexistence configuration for one antenna
        #if 0
        ULONG       COEX_CFG0_Bit16_FixWiFiLNAGain_BTTx:1;              
        ULONG       COEX_CFG0_Bit17_BT_TRSW_BTIdle:1;           
        ULONG       COEX_CFG0_Bit18_LowerHighPriorityBTTx_WiFiRx:1;     
        ULONG       COEX_CFG0_Bit19_LowerLowPriorityBTTx_WiFiRx:1;      
        ULONG       COEX_CFG0_Bit20_LowerWiFiTxPwr_BTRx:1;      
        ULONG       COEX_CFG0_Bit21_Abort_LowPriorityWiFiTx_bt_rx_req_h:1;          
        ULONG           COEX_CFG0_Bit22_FixWiFiBBPPwr_BTRx:1;   
        ULONG           COEX_CFG0_Bit23_Srv:1;
        #endif
        
        ULONG           COEX_CFG1:8;        //Bluetooth/WiFi coexistence configuration for two antenna
        #if 0
        ULONG       COEX_CFG1_Bit24_FixWiFiLNAGain_BTTx:1;      
        ULONG       COEX_CFG1_Bit25_BT_TRSW_BTIdle:1;           
        ULONG       COEX_CFG1_Bit26_LowerHighPriorityBTTx_WiFiRx:1;     
        ULONG       COEX_CFG1_Bit27_LowerLowPriorityBTTx_WiFiRx:1;      
        ULONG       COEX_CFG1_Bit28_LowerWiFiTxPwr_BTRx:1;      
        ULONG       COEX_CFG1_Bit29_Abort_LowPriorityWiFiTx_bt_rx_req_h:1;          
        ULONG       COEX_CFG1_Bit30_FixWiFiBBPPwr_BTRx:1;       
        ULONG       COEX_CFG1_Bit31_Rsv:1;
        #endif
        
    }field  ;
    ULONG           word;
} COEXCFG0_STRUC, *PCOEXCFG0_STRUC;
#endif

#define COEXCFG1            0x44
#ifdef BIG_ENDIAN
typedef union   _COEXCFG1_STRUC {
    struct  {
        ULONG           Rsv:8;
        ULONG       DIS_WL_RF_DELY:8;   //Delay time of RF_PE when WLAN is in force Tx stop state. Unit is 1/48 us.
        ULONG       DIS_WL_PA_DELY:8;   //Delay time of RF_PA when WLAN is in force Tx stop state. Unit is 1/48 us.
        ULONG       DIS_WL_TR_DELY:8;   //Delay time of RF_TR when WLAN is in force Tx stop state. Unit is 1/48 us. 
    }   field;
    ULONG           word;
}   COEXCFG1_STRUC, *PCOEXCFG1_STRUC;
#else
typedef union   _COEXCFG1_STRUC {
    struct  {
        ULONG       DIS_WL_TR_DELY:8;   //Delay time of RF_TR when WLAN is in force Tx stop state. Unit is 1/48 us.
        ULONG       DIS_WL_PA_DELY:8;   //Delay time of RF_PA when WLAN is in force Tx stop state. Unit is 1/48 us.
        ULONG       DIS_WL_RF_DELY:8;   //Delay time of RF_PE when WLAN is in force Tx stop state. Unit is 1/48 us.     
        ULONG           Rsv:8;
    }   field;
    ULONG           word;
} COEXCFG1_STRUC, *PCOEXCFG1_STRUC;
#endif

#define COEXCFG2            0x48
#define BT_COEX_CFG2        0x1238
#define BT_COEX_CFG3        0x123C
#ifdef BIG_ENDIAN
typedef union   _COEXCFG2_STRUC {
    struct  {
        //ULONG     BT_COEX_CFG1:8; //BT coexistence configuration for two antenna mode
        ULONG       BT_COEX_CFG1_Bit31_Rsv:1;
        ULONG       BT_COEX_CFG1_Bit30_Rsv:1;   
        ULONG       BT_COEX_CFG1_Bit29_HaltHighPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG1_Bit28_HaltHighPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG1_Bit27_HaltHighPriorityTx_wl_busy:1;
        ULONG       BT_COEX_CFG1_Bit26_HaltLowPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG1_Bit25_HaltLowPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG1_Bit24_HaltLowPriorityTx_wl_busy:1;     
        
        //ULONG     BT_COEX_CFG0:8; //BT coexistence configuration for one antenna mode
        ULONG       BT_COEX_CFG0_Bit23_Rsv:1;
        ULONG       BT_COEX_CFG0_Bit22_Rsv:1;   
        ULONG       BT_COEX_CFG0_Bit21_HaltHighPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG0_Bit20_HaltHighPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG0_Bit19_HaltHighPriorityTx_wl_busy:1;
        ULONG       BT_COEX_CFG0_Bit18_HaltLowPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG0_Bit17_HaltLowPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG0_Bit16_HaltLowPriorityTx_wl_busy:1;
        
        //ULONG     WL_COEX_CFG1:8; //WLAN coexistence configuration for two antenna mode
        ULONG       WL_COEX_CFG1_Bit15_LowerTxPwr_bt_high_priority:1;
        ULONG       WL_COEX_CFG1_Bit14_Enable_Tx_free_timer:1;
        ULONG       WL_COEX_CFG1_Bit13_Disable_TxAgg_bi_high_priority:1;
        ULONG       WL_COEX_CFG1_Bit12_Disable_bt_rx_req_h:1;
        ULONG       WL_COEX_CFG1_Bit11_HaltTx_bt_tx_req_l:1;
        ULONG       WL_COEX_CFG1_Bit10_HaltTx_bt_tx_req_h:1;
        ULONG       WL_COEX_CFG1_Bit9_HaltTx_bt_rx_req_h:1; 
        ULONG       WL_COEX_CFG1_Bit8_HaltTx_bt_rx_busy:1;      
        
        //ULONG     WL_COEX_CFG0:8; //WLAN coexistence configuration for one antenna mode   
        ULONG       WL_COEX_CFG0_Bit7_LowerTxPwr_bt_high_priority:1;
        ULONG       WL_COEX_CFG0_Bit6_Enable_Tx_free_timer:1;
        ULONG       WL_COEX_CFG0_Bit5_Disable_TxAgg_bi_high_priority:1;
        ULONG       WL_COEX_CFG0_Bit4_Disable_bt_rx_req_h:1;
        ULONG       WL_COEX_CFG0_Bit3_HaltTx_bt_tx_req_l:1;
        ULONG       WL_COEX_CFG0_Bit2_HaltTx_bt_tx_req_h:1;
        ULONG       WL_COEX_CFG0_Bit1_HaltTx_bt_rx_req_h:1; 
        ULONG       WL_COEX_CFG0_Bit0_HaltTx_bt_rx_busy:1;          
    }field;
    ULONG           word;
}   COEXCFG2_STRUC, *PCOEXCFG2_STRUC;
#else
typedef union   _COEXCFG2_STRUC {
    struct  {
        //ULONG     WL_COEX_CFG0:8; //WLAN coexistence configuration for one antenna mode   
        ULONG       WL_COEX_CFG0_Bit0_HaltTx_bt_rx_busy:1;  
        ULONG       WL_COEX_CFG0_Bit1_HaltTx_bt_rx_req_h:1; 
        ULONG       WL_COEX_CFG0_Bit2_HaltTx_bt_tx_req_h:1;
        ULONG       WL_COEX_CFG0_Bit3_HaltTx_bt_tx_req_l:1;
        ULONG       WL_COEX_CFG0_Bit4_Disable_bt_rx_req_h:1;
        ULONG       WL_COEX_CFG0_Bit5_Disable_TxAgg_bi_high_priority:1;
        ULONG       WL_COEX_CFG0_Bit6_Enable_Tx_free_timer:1;       
        ULONG       WL_COEX_CFG0_Bit7_LowerTxPwr_bt_high_priority:1;

        //ULONG     WL_COEX_CFG1:8; //WLAN coexistence configuration for two antenna mode
        ULONG       WL_COEX_CFG1_Bit8_HaltTx_bt_rx_busy:1;
        ULONG       WL_COEX_CFG1_Bit9_HaltTx_bt_rx_req_h:1; 
        ULONG       WL_COEX_CFG1_Bit10_HaltTx_bt_tx_req_h:1;
        ULONG       WL_COEX_CFG1_Bit11_HaltTx_bt_tx_req_l:1;
        ULONG       WL_COEX_CFG1_Bit12_Disable_bt_rx_req_h:1;
        ULONG       WL_COEX_CFG1_Bit13_Disable_TxAgg_bi_high_priority:1;
        ULONG       WL_COEX_CFG1_Bit14_Enable_Tx_free_timer:1;      
        ULONG       WL_COEX_CFG1_Bit15_LowerTxPwr_bt_high_priority:1;
        
        //ULONG     BT_COEX_CFG0:8; //BT coexistence configuration for one antenna mode
        ULONG       BT_COEX_CFG0_Bit16_HaltLowPriorityTx_wl_busy:1;
        ULONG       BT_COEX_CFG0_Bit17_HaltLowPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG0_Bit18_HaltLowPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG0_Bit19_HaltHighPriorityTx_wl_busy:1;
        ULONG       BT_COEX_CFG0_Bit20_HaltHighPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG0_Bit21_HaltHighPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG0_Bit22_Rsv:1;   
        ULONG       BT_COEX_CFG0_Bit23_Rsv:1;
        
        //ULONG     BT_COEX_CFG1:8; //BT coexistence configuration for two antenna mode
        ULONG       BT_COEX_CFG1_Bit24_HaltLowPriorityTx_wl_busy:1;
        ULONG       BT_COEX_CFG1_Bit25_HaltLowPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG1_Bit26_HaltLowPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG1_Bit27_HaltHighPriorityTx_wl_busy:1;
        ULONG       BT_COEX_CFG1_Bit28_HaltHighPriorityTx_wl_rx_busy:1;
        ULONG       BT_COEX_CFG1_Bit29_HaltHighPriorityTx_wl_bcn_busy:1;
        ULONG       BT_COEX_CFG1_Bit30_Rsv:1;           
        ULONG       BT_COEX_CFG1_Bit31_Rsv:1;
    }field  ;
    ULONG           word;
} COEXCFG2_STRUC, *PCOEXCFG2_STRUC;
#endif

//2 TODO: Move to RT6X9X.h
#define WLAN_FUN_CTRL       0x80
#define BT_FUN_CTRL         0xC0

#ifdef BIG_ENDIAN
typedef union   _WLAN_FUN_CTRL_STRUC    {
    struct  {
        ULONG       GPIO0_OUT_OE_N:8;       //GPIO0 output enable
        ULONG       GPIO0_OUT:8;            //GPIO0 output data
        ULONG       GPIO0_IN:8;             //GPIO0 input data
        ULONG       WLAN_ACC_BT:1;          //WLAN driver access BT enable
        ULONG       INV_TR_SW0:1;           //invert TR_SW0
        ULONG       FRC_WL_ANT_SET:1;
        ULONG       PCIE_APP0_CLK_REQ:1;    //PCIE function0(WLAN) clock request
        ULONG       WLAN_RESET:1;           //Reset WLAN
        ULONG       WLAN_MCU_RESET:1;
        ULONG       WLAN_CLK_EN:1;          //WLAN clock enable
        ULONG       WLAN_EN:1;              //WLAN function enable
    } field;
    ULONG           word;
}   WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#else
typedef union   _WLAN_FUN_CTRL_STRUC    {
    struct  {
        ULONG       WLAN_EN:1;              //WLAN function enable
        ULONG       WLAN_CLK_EN:1;          //WLAN clock enable
        ULONG       WLAN_MCU_RESET:1;
        ULONG       WLAN_RESET:1;           //Reset WLAN
        ULONG       PCIE_APP0_CLK_REQ:1;    //PCIE function0(WLAN) clock request
        ULONG       FRC_WL_ANT_SET:1;
        ULONG       INV_TR_SW0:1;           //invert TR_SW0
        ULONG       WLAN_ACC_BT:1;          //WLAN driver access BT enable
        ULONG       GPIO0_IN:8;             //GPIO0 input data
        ULONG       GPIO0_OUT:8;            //GPIO0 output data     
        ULONG       GPIO0_OUT_OE_N:8;       //GPIO0 output enable
    } field;
    ULONG           word;
} WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#endif
#ifdef BIG_ENDIAN
typedef union   _TXRXQ_PCNT_STRUC   {
    struct  
    {
        ULONG   Rx0QPcnt:8;
        ULONG   Tx2QPcnt:8; // EDCA 
        ULONG   Tx1QPcnt:8; // HCCA 
        ULONG   Tx0QPcnt:8;
    }   field;
    ULONG           word;
}   TXRXQ_PCNT_STRUC, *PTXRXQ_PCNT_STRUC;       
#else
typedef union   _TXRXQ_PCNT_STRUC   {
    struct  
    {
        ULONG   Tx0QPcnt:8;
        ULONG   Tx1QPcnt:8; // HCCA 
        ULONG   Tx2QPcnt:8; // EDCA 
        ULONG   Rx0QPcnt:8;
    }   field;
    ULONG           word;
}   TXRXQ_PCNT_STRUC, *PTXRXQ_PCNT_STRUC;       
#endif

#ifdef BIG_ENDIAN
typedef union   _PBF_DBG_STRUC  {
    struct  
    {
        ULONG   Rsvd:24;
        ULONG   FreePcnt:8;
    }   field;
    ULONG           word;
}   PBF_DBG_STRUC, *PPBF_DBG_STRUC;     
#else
typedef union   _PBF_DBG_STRUC  {
    struct  
    {
        ULONG   FreePcnt:8;
        ULONG   Rsvd:24;
    }   field;
    ULONG           word;
}   PBF_DBG_STRUC, *PPBF_DBG_STRUC;     
#endif

#ifdef BIG_ENDIAN
typedef union   _BCN_OFFSET_0   {
    struct  {
        ULONG   Bcn3Offset:8;
        ULONG   Bcn2Offset:8;
        ULONG   Bcn1Offset:8;
        ULONG   Bcn0Offset:8;
    }   field;
    ULONG           word;
}   BCN_OFFSET_0, *PBCN_OFFSET_0;       
#else
typedef union   _BCN_OFFSET_0   {
    struct  {
        ULONG   Bcn0Offset:8;
        ULONG   Bcn1Offset:8;
        ULONG   Bcn2Offset:8;
        ULONG   Bcn3Offset:8;
    }   field;
    ULONG           word;
}   BCN_OFFSET_0, *PBCN_OFFSET_0;       
#endif

#ifdef BIG_ENDIAN
typedef union   _BCN_OFFSET_1   {
    struct  {
        ULONG   Bcn7Offset:8;
        ULONG   Bcn6Offset:8;
        ULONG   Bcn5Offset:8;
        ULONG   Bcn4Offset:8;
    }   field;
    ULONG           word;
}   BCN_OFFSET_1, *PBCN_OFFSET_1;       
#else
typedef union   _BCN_OFFSET_1   {
    struct  {
        ULONG   Bcn4Offset:8;
        ULONG   Bcn5Offset:8;
        ULONG   Bcn6Offset:8;
        ULONG   Bcn7Offset:8;
    }   field;
    ULONG           word;
}   BCN_OFFSET_1, *PBCN_OFFSET_1;       
#endif

#define PBF_CAP_CTRL        0x0440


// eFuse registers
#define EFUSE_CTRL          ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x24) : (0x0580))

#define EFUSE_DATA0         ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x28) : (0x0590))
#define EFUSE_DATA1         ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x2c) : (0x0594))
#define EFUSE_DATA2         ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x30) : (0x0598))
#define EFUSE_DATA3         ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x34) : (0x059c))
#define EFUSE_TAG               0x2fe

#define EFUSE_CTRL_2        0x24
#define EFUSE_DATA0_2       0x28
#define EFUSE_DATA1_2       0x2c
#define EFUSE_DATA2_2       0x30
#define EFUSE_DATA3_2       0x34

//
// eFuse configuration
//
typedef struct _EFUSE_CONFIGURATION
{
    USHORT  MapStart;
    USHORT  MapEnd;
    USHORT  MapSize;
} EFUSE_CONFIGURATION, *PEFUSE_CONFIGURATION;

#ifdef BIG_ENDIAN
typedef union _EFUSE_CTRL_STRUC {
    struct {
    ULONG       SEL_EFUSE:1;
    ULONG       EFSROM_KICK:1;
    ULONG       RESERVED:4;
    ULONG       EFSROM_AIN:10;
    ULONG       EFSROM_LDO_ON_TIME:2;
    ULONG       EFSROM_LDO_OFF_TIME:6;
    ULONG       EFSROM_MODE:2;
    ULONG       EFSROM_AOUT:6;  
    } field;
    ULONG           word;
}   EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;   
#else
typedef union _EFUSE_CTRL_STRUC {
    struct {
    ULONG       EFSROM_AOUT:6;
    ULONG       EFSROM_MODE:2;
    ULONG       EFSROM_LDO_OFF_TIME:6;
    ULONG       EFSROM_LDO_ON_TIME:2;
    ULONG       EFSROM_AIN:10;
    ULONG       RESERVED:4;
    ULONG       EFSROM_KICK:1;
    ULONG       SEL_EFUSE:1;
    } field;
    ULONG           word;
}   EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#endif

#define LDO_CFG0                0x05d4
#define GPIO_SWITCH             0x05dc
#define DEBUG_INDEX             0x05E8

//
// FCE Registers
//
#define PER_PORT_TOTAL_USED_PAGE_COUNT_1        0x0a30
#define PER_PORT_TOTAL_USED_PAGE_COUNT_2        0x0a34

#ifdef BIG_ENDIAN
typedef union   _PER_PORT_TOTAL_USED_PAGE_COUNT_1_STRUC {
    struct  {
        ULONG       F1_PSE_PCNT:8;
        ULONG       F0_PSE_PCNT:8;
        ULONG       H_PSE_PCNT:8;
        ULONG       W_PSE_PCNT:8;
    }   field;
    ULONG           word;
}PER_PORT_TOTAL_USED_PAGE_COUNT_1_STURC, *PPER_PORT_TOTAL_USED_PAGE_COUNT_STRUC;
#else
typedef union   _PER_PORT_TOTAL_USED_PAGE_COUNT_1_STRUC{
    struct  {
        ULONG       W_PSE_PCNT:8;
        ULONG       H_PSE_PCNT:8;
        ULONG       F0_PSE_PCNT:8;
        ULONG       F1_PSE_PCNT:8;
    }   field;
    ULONG           word;
} PER_PORT_TOTAL_USED_PAGE_COUNT_1_STURC, *PPER_PORT_TOTAL_USED_PAGE_COUNT_STRUC;
#endif



#ifdef BIG_ENDIAN
typedef union   _PER_PORT_TOTAL_USED_PAGE_COUNT_2_STRUC {
    struct  {
        ULONG       CT_PSE_PCNT:8;
        ULONG       CR_PSE_PCNT:8;
        ULONG       VT_PSE_PCNT:8;
        ULONG       VR_PSE_PCNT:8;
    }   field;
    ULONG           word;
}PER_PORT_TOTAL_USED_PAGE_COUNT_2_STRUC, *PPER_PORT_TOTAL_USED_PAGE_COUNT_2_STRUC;
#else
typedef union   _PER_PORT_TOTAL_USED_PAGE_COUNT_2_STRUC{
    struct  {
        ULONG       VR_PSE_PCNT:8;
        ULONG       VT_PSE_PCNT:8;
        ULONG       CR_PSE_PCNT:8;
        ULONG       CT_PSE_PCNT:8;
    }   field;
    ULONG           word;
} PER_PORT_TOTAL_USED_PAGE_COUNT_2_STRUC, *PPER_PORT_TOTAL_USED_PAGE_COUNT_2STURC;
#endif

//
//  4  MAC  registers  
//
//
//  4.1 MAC SYSTEM  configuration registers (offset:0x1000)
//
#define MAC_CSR0            0x1000
#ifdef BIG_ENDIAN
typedef union   _ASIC_VER_ID_STRUC  {
    struct  {
        USHORT  ASICVer;        // version : 2860
        USHORT  ASICRev;        // reversion  : 0
    }   field;
    ULONG           word;
}   ASIC_VER_ID_STRUC, *PASIC_VER_ID_STRUC;
#else
typedef union   _ASIC_VER_ID_STRUC  {
    struct  {
        USHORT  ASICRev;        // reversion  : 0
        USHORT  ASICVer;        // version : 2860
    }   field;
    ULONG           word;
}   ASIC_VER_ID_STRUC, *PASIC_VER_ID_STRUC;
#endif
#define MAC_SYS_CTRL            0x1004      //MAC_CSR1
#define MAC_ADDR_DW0                    0x1008      // MAC ADDR DW0
#define MAC_ADDR_DW1                 0x100c     // MAC ADDR DW1

#ifdef BIG_ENDIAN
typedef union   _MAC_SYS_CTRL_STRUC {
    struct  {
        ULONG       Reserve1:28;
        ULONG       MacRxEna:1;
        ULONG       MacTxEna:1;
        ULONG       Reserve2:2;
    }   field;
    ULONG           word;
}   MAC_SYS_CTRL_STRUC, *PMAC_SYS_CTRL_STRUC;
#else
typedef union   _MAC_SYS_CTRL_STRUC {
    struct  {
        ULONG       Reserve2:2;
        ULONG       MacTxEna:1;
        ULONG       MacRxEna:1;
        ULONG       Reserve1:28;
    }   field;
    ULONG           word;
}   MAC_SYS_CTRL_STRUC, *PMAC_SYS_CTRL_STRUC;
#endif

//
// MAC_CSR2: STA MAC register 0
//
#ifdef BIG_ENDIAN
typedef union   _MAC_DW0_STRUC  {
    struct  {
        UCHAR       Byte3;      // MAC address byte 3
        UCHAR       Byte2;      // MAC address byte 2
        UCHAR       Byte1;      // MAC address byte 1
        UCHAR       Byte0;      // MAC address byte 0
    }   field;
    ULONG           word;
}   MAC_DW0_STRUC, *PMAC_DW0_STRUC;
#else
typedef union   _MAC_DW0_STRUC  {
    struct  {
        UCHAR       Byte0;      // MAC address byte 0
        UCHAR       Byte1;      // MAC address byte 1
        UCHAR       Byte2;      // MAC address byte 2
        UCHAR       Byte3;      // MAC address byte 3
    }   field;
    ULONG           word;
}   MAC_DW0_STRUC, *PMAC_DW0_STRUC;
#endif

//
// MAC_CSR3: STA MAC register 1
//
#ifdef BIG_ENDIAN
typedef union   _MAC_DW1_STRUC  {
    struct  {
        UCHAR       Rsvd1;
        UCHAR       U2MeMask;
        UCHAR       Byte5;      // MAC address byte 5
        UCHAR       Byte4;      // MAC address byte 4
    }   field;
    ULONG           word;
}   MAC_DW1_STRUC, *PMAC_DW1_STRUC;
#else
typedef union   _MAC_DW1_STRUC  {
    struct  {
        UCHAR       Byte4;      // MAC address byte 4
        UCHAR       Byte5;      // MAC address byte 5
        UCHAR       U2MeMask;
        UCHAR       Rsvd1;
    }   field;
    ULONG           word;
}   MAC_DW1_STRUC, *PMAC_DW1_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union   _MAC_DW1_EX_STRUC   {
    struct  {
        UCHAR       Rsvd1;
        UCHAR       MultiBcnNumEx:1;
        UCHAR       MultiBssidModeEx:1;
        UCHAR       NewMultiBssidMode:1;
        UCHAR       MultiBcnNum:3;
        UCHAR       MultiBssidMode:2;
        UCHAR       Byte5;      // MAC address byte 5
        UCHAR       Byte4;      // MAC address byte 4
    }   field;
    ULONG           word;
}   MAC_DW1_EX_STRUC, *PMAC_DW1_EX_STRUC;
#else
typedef union   _MAC_DW1_EX_STRUC   {
    struct  {
        UCHAR       Byte4;      // MAC address byte 4
        UCHAR       Byte5;      // MAC address byte 5
        UCHAR       MultiBssidMode:2;
        UCHAR       MultiBcnNum:3;
        UCHAR       NewMultiBssidMode:1;
        UCHAR       MultiBssidModeEx:1;
        UCHAR       MultiBcnNumEx:1;
        UCHAR       Rsvd1;
    }   field;
    ULONG           word;
}   MAC_DW1_EX_STRUC, *PMAC_DW1_EX_STRUC;
#endif
enum
{
    MULTI_BSSID_1 = 0,
    MULTI_BSSID_2,
    MULTI_BSSID_4,
    MULTI_BSSID_8
};
enum
{
    BEACON_NUMBER_1 = 0,
    BEACON_NUMBER_2,
    BEACON_NUMBER_3,
    BEACON_NUMBER_4,
    BEACON_NUMBER_5,
    BEACON_NUMBER_6,
    BEACON_NUMBER_7,
    BEACON_NUMBER_8
};
#define MAC_BSSID_DW0                   0x1010      // MAC BSSID DW0
#define MAC_BSSID_DW1                   0x1014      // MAC BSSID DW1

//
// MAC_CSR5: BSSID register 1
//
#ifdef BIG_ENDIAN
typedef union   _MAC_CSR5_STRUC {
    struct  {
        USHORT      Rsvd:11;
        USHORT      MBssBcnNum:3;
        USHORT      BssIdMode:2; // 0: one BSSID, 10: 4 BSSID,  01: 2 BSSID , 11: 8BSSID
        UCHAR       Byte5;       // BSSID byte 5
        UCHAR       Byte4;       // BSSID byte 4
    }   field;
    ULONG           word;
}   MAC_CSR5_STRUC, *PMAC_CSR5_STRUC;
#else
typedef union   _MAC_CSR5_STRUC {
    struct  {
        UCHAR       Byte4;       // BSSID byte 4
        UCHAR       Byte5;       // BSSID byte 5
        USHORT          BssIdMask:2; // 0: one BSSID, 10: 4 BSSID,  01: 2 BSSID , 11: 8BSSID
        USHORT      MBssBcnNum:3;
        USHORT      Rsvd:11;
    }   field;
    ULONG           word;
}   MAC_CSR5_STRUC, *PMAC_CSR5_STRUC;
#endif

#define MAX_LEN_CFG              0x1018     // rt2860b max 16k bytes. bit12:13 Maximum PSDU length (power factor) 0:2^13, 1:2^14, 2:2^15, 3:2^16
#define BBP_CSR_CFG                 0x101c      //  
//
// BBP_CSR_CFG: BBP serial control register
//
#ifdef BIG_ENDIAN
typedef union   _BBP_CSR_CFG_STRUC  {
    struct  {
        ULONG       :12;        
        ULONG       BBP_RW_MODE:1;      // 0: use serial mode  1:parallel
        ULONG       BBP_PAR_DUR:1;          // 0: 4 MAC clock cycles  1: 8 MAC clock cycles
        ULONG       Busy:1;             // 1: ASIC is busy execute BBP programming. 
        ULONG       fRead:1;            // 0: Write BBP, 1: Read BBP
        ULONG       RegNum:8;           // Selected BBP register
        ULONG       Value:8;            // Register value to program into BBP
    }   field;
    ULONG           word;
}   BBP_CSR_CFG_STRUC, *PBBP_CSR_CFG_STRUC;
#else
typedef union   _BBP_CSR_CFG_STRUC  {
    struct  {
        ULONG       Value:8;            // Register value to program into BBP
        ULONG       RegNum:8;           // Selected BBP register
        ULONG       fRead:1;            // 0: Write BBP, 1: Read BBP
        ULONG       Busy:1;             // 1: ASIC is busy execute BBP programming. 
        ULONG       BBP_PAR_DUR:1;           // 0: 4 MAC clock cycles  1: 8 MAC clock cycles
        ULONG       BBP_RW_MODE:1;      // 0: use serial mode  1:parallel
        ULONG       :12;
    }   field;
    ULONG           word;
}   BBP_CSR_CFG_STRUC, *PBBP_CSR_CFG_STRUC;
#endif
#define RF_CSR_CFG0                 0x1020       
//
// RF_CSR_CFG: RF control register
//
#ifdef BIG_ENDIAN
typedef union   _RF_CSR_CFG0_STRUC  {
    struct  {
        ULONG       Busy:1;         // 0: idle 1: 8busy
        ULONG       Sel:1;              // 0:RF_LE0 activate  1:RF_LE1 activate 
        ULONG       StandbyMode:1;          // 0: high when stand by 1: low when standby
        ULONG       bitwidth:5;         // Selected BBP register
        ULONG       RegIdAndContent:24;         // Register value to program into BBP
    }   field;
    ULONG           word;
}   RF_CSR_CFG0_STRUC, *PRF_CSR_CFG0_STRUC;
#else
typedef union   _RF_CSR_CFG0_STRUC  {
    struct  {
        ULONG       RegIdAndContent:24;         // Register value to program into BBP
        ULONG       bitwidth:5;         // Selected BBP register
        ULONG       StandbyMode:1;          // 0: high when stand by 1: low when standby
        ULONG       Sel:1;              // 0:RF_LE0 activate  1:RF_LE1 activate 
        ULONG       Busy:1;         // 0: idle 1: 8busy
    }   field;
    ULONG           word;
}   RF_CSR_CFG0_STRUC, *PRF_CSR_CFG0_STRUC;
#endif
#define RF_CSR_CFG1                 0x1024  
#ifdef BIG_ENDIAN
typedef union   _RF_CSR_CFG1_STRUC  {
    struct  {
        ULONG       rsv:7;          // 0: idle 1: 8busy
        ULONG       RFGap:5;            // Gap between BB_CONTROL_RF and RF_LE. 0: 3 system clock cycle (37.5usec) 1: 5 system clock cycle (62.5usec)
        ULONG       RegIdAndContent:24;         // Register value to program into BBP
    }   field;
    ULONG           word;
}   RF_CSR_CFG1_STRUC, *PRF_CSR_CFG1_STRUC;
#else
typedef union   _RF_CSR_CFG1_STRUC  {
    struct  {
        ULONG       RegIdAndContent:24;         // Register value to program into BBP
        ULONG       RFGap:5;            // Gap between BB_CONTROL_RF and RF_LE. 0: 3 system clock cycle (37.5usec) 1: 5 system clock cycle (62.5usec)
        ULONG       rsv:7;          // 0: idle 1: 8busy
    }   field;
    ULONG           word;
}   RF_CSR_CFG1_STRUC, *PRF_CSR_CFG1_STRUC;
#endif
#define RF_CSR_CFG2                 0x1028      //  
#ifdef BIG_ENDIAN
typedef union   _RF_CSR_CFG2_STRUC  {
    struct  {
        ULONG       rsv:8;          // 0: idle 1: 8busy
        ULONG       RegIdAndContent:24;         // Register value to program into BBP
    }   field;
    ULONG           word;
}   RF_CSR_CFG2_STRUC, *PRF_CSR_CFG2_STRUC;
#else
typedef union   _RF_CSR_CFG2_STRUC  {
    struct  {
        ULONG       RegIdAndContent:24;         // Register value to program into BBP
        ULONG       rsv:8;          // 0: idle 1: 8busy
    }   field;
    ULONG           word;
}   RF_CSR_CFG2_STRUC, *PRF_CSR_CFG2_STRUC;
#endif
#define LED_CFG                 0x102c      //  MAC_CSR14
#ifdef BIG_ENDIAN
typedef union   _LED_CFG_STRUC  {
    struct  {
        ULONG       :1;          
        ULONG       LedPolar:1;         // Led Polarity.  0: active low1: active high
        ULONG       YLedMode:2;         // yellow Led Mode
        ULONG       GLedMode:2;         // green Led Mode
        ULONG       RLedMode:2;         // red Led Mode    0: off1: blinking upon TX2: periodic slow blinking3: always on
        ULONG       rsv:2;          
        ULONG       SlowBlinkPeriod:6;          // slow blinking period. unit:1ms
        ULONG       OffPeriod:8;            // blinking off period unit 1ms
        ULONG       OnPeriod:8;         // blinking on period unit 1ms
    }   field;
    ULONG           word;
}   LED_CFG_STRUC, *PLED_CFG_STRUC;
#else
typedef union   _LED_CFG_STRUC  {
    struct  {
        ULONG       OnPeriod:8;         // blinking on period unit 1ms
        ULONG       OffPeriod:8;            // blinking off period unit 1ms
        ULONG       SlowBlinkPeriod:6;          // slow blinking period. unit:1ms
        ULONG       rsv:2;          
        ULONG       RLedMode:2;         // red Led Mode    0: off1: blinking upon TX2: periodic slow blinking3: always on
        ULONG       GLedMode:2;         // green Led Mode
        ULONG       YLedMode:2;         // yellow Led Mode
        ULONG       LedPolar:1;         // Led Polarity.  0: active low1: active high
        ULONG       :1;          
    }   field;
    ULONG           word;
}   LED_CFG_STRUC, *PLED_CFG_STRUC;
#endif

#define LED_CFG_MODE_OFF                0
#define LED_CFG_MODE_BLINK_UPON_TX      1
#define LED_CFG_MODE_PERIODIC_BLINK     2
#define LED_CFG_MODE_ON                 3

#define LED_GPIO4_ON  0
#define LED_GPIO4_OFF 1


//
// AMPDU MAX LEN
//
#define AMPDU_MAX_LEN_20M1S     0x1030
#define AMPDU_MAX_LEN_20M2S     0x1034
#define AMPDU_MAX_LEN_40M1S     0x1038
#define AMPDU_MAX_LEN_40M2S     0x103C
#define AMPDU_MAX_LEN_20M3S     0x1064
#define AMPDU_MAX_LEN_40M3S     0x1068


//
// The number of the Tx chains
//
#define NUM_OF_TX_CHAIN     4

#define TX_CHAIN_ADDR0_L        0x1044

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR0_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr0L_Byte3; // Destination MAC address of Tx chain0 (byte 3)
        UCHAR   TxChainAddr0L_Byte2; // Destination MAC address of Tx chain0 (byte 2)
        UCHAR   TxChainAddr0L_Byte1; // Destination MAC address of Tx chain0 (byte 1)
        UCHAR   TxChainAddr0L_Byte0; // Destination MAC address of Tx chain0 (byte 0)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR0_L_STRUC, *PTX_CHAIN_ADDR0_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR0_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr0L_Byte0; // Destination MAC address of Tx chain0 (byte 0)
        UCHAR   TxChainAddr0L_Byte1; // Destination MAC address of Tx chain0 (byte 1)
        UCHAR   TxChainAddr0L_Byte2; // Destination MAC address of Tx chain0 (byte 2)
        UCHAR   TxChainAddr0L_Byte3; // Destination MAC address of Tx chain0 (byte 3)
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR0_L_STRUC, *PTX_CHAIN_ADDR0_L_STRUC;
#endif

#define TX_CHAIN_ADDR0_H    0x1048

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR0_H_STRUC
{
    struct
    {
        USHORT  Reserved:12; // Reserved
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        UCHAR   TxChainAddr0H_Byte5; // Destination MAC address of Tx chain0 (byte 5)
        UCHAR   TxChainAddr0H_Byte4; // Destination MAC address of Tx chain0 (byte 4)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR0_H_STRUC, *PTX_CHAIN_ADDR0_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR0_H_STRUC
{
    struct
    {
        UCHAR   TxChainAddr0H_Byte4; // Destination MAC address of Tx chain0 (byte 4)
        UCHAR   TxChainAddr0H_Byte5; // Destination MAC address of Tx chain0 (byte 5)       
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        USHORT  Reserved:12; // Reserved
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR0_H_STRUC, *PTX_CHAIN_ADDR0_HA_STRUC;
#endif

#define TX_CHAIN_ADDR1_L        0x104C

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR1_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr1L_Byte3; // Destination MAC address of Tx chain1 (byte 3)
        UCHAR   TxChainAddr1L_Byte2; // Destination MAC address of Tx chain1 (byte 2)
        UCHAR   TxChainAddr1L_Byte1; // Destination MAC address of Tx chain1 (byte 1)
        UCHAR   TxChainAddr1L_Byte0; // Destination MAC address of Tx chain1 (byte 0)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR1_L_STRUC, *PTX_CHAIN_ADDR1_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR1_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr1L_Byte0; // Destination MAC address of Tx chain1 (byte 0)
        UCHAR   TxChainAddr1L_Byte1; // Destination MAC address of Tx chain1 (byte 1)
        UCHAR   TxChainAddr1L_Byte2; // Destination MAC address of Tx chain1 (byte 2)
        UCHAR   TxChainAddr1L_Byte3; // Destination MAC address of Tx chain1 (byte 3)
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR1_L_STRUC, *PTX_CHAIN_ADDR1_L_STRUC;
#endif

#define TX_CHAIN_ADDR1_H    0x1050

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR1_H_STRUC
{
    struct
    {
        USHORT  Reserved:12; // Reserved
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        UCHAR   TxChainAddr1H_Byte5; // Destination MAC address of Tx chain1 (byte 5)
        UCHAR   TxChainAddr1H_Byte4; // Destination MAC address of Tx chain1 (byte 4)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR1_H_STRUC, *PTX_CHAIN_ADDR1_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR1_H_STRUC
{
    struct
    {
        UCHAR   TxChainAddr1H_Byte4; // Destination MAC address of Tx chain1 (byte 4)
        UCHAR   TxChainAddr1H_Byte5; // Destination MAC address of Tx chain1 (byte 5)       
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        USHORT  Reserved:12; // Reserved
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR1_H_STRUC, *PTX_CHAIN_ADDR1_HA_STRUC;
#endif

#define TX_CHAIN_ADDR2_L        0x1054

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR2_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr2L_Byte3; // Destination MAC address of Tx chain2 (byte 3)
        UCHAR   TxChainAddr2L_Byte2; // Destination MAC address of Tx chain2 (byte 2)
        UCHAR   TxChainAddr2L_Byte1; // Destination MAC address of Tx chain2 (byte 1)
        UCHAR   TxChainAddr2L_Byte0; // Destination MAC address of Tx chain2 (byte 0)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR2_L_STRUC, *PTX_CHAIN_ADDR2_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR2_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr2L_Byte0; // Destination MAC address of Tx chain2 (byte 0)
        UCHAR   TxChainAddr2L_Byte1; // Destination MAC address of Tx chain2 (byte 1)
        UCHAR   TxChainAddr2L_Byte2; // Destination MAC address of Tx chain2 (byte 2)
        UCHAR   TxChainAddr2L_Byte3; // Destination MAC address of Tx chain2 (byte 3)
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR2_L_STRUC, *PTX_CHAIN_ADDR2_L_STRUC;
#endif

#define TX_CHAIN_ADDR2_H    0x1058

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR2_H_STRUC
{
    struct
    {
        USHORT  Reserved:12; // Reserved
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        UCHAR   TxChainAddr2H_Byte5; // Destination MAC address of Tx chain2 (byte 5)
        UCHAR   TxChainAddr2H_Byte4; // Destination MAC address of Tx chain2 (byte 4)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR2_H_STRUC, *PTX_CHAIN_ADDR2_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR2_H_STRUC
{
    struct
    {
        UCHAR   TxChainAddr2H_Byte4; // Destination MAC address of Tx chain2 (byte 4)
        UCHAR   TxChainAddr2H_Byte5; // Destination MAC address of Tx chain2 (byte 5)       
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        USHORT  Reserved:12; // Reserved
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR2_H_STRUC, *PTX_CHAIN_ADDR2_HA_STRUC;
#endif

#define TX_CHAIN_ADDR3_L        0x105C

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR3_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr3L_Byte3; // Destination MAC address of Tx chain3 (byte 3)
        UCHAR   TxChainAddr3L_Byte2; // Destination MAC address of Tx chain3 (byte 2)
        UCHAR   TxChainAddr3L_Byte1; // Destination MAC address of Tx chain3 (byte 1)
        UCHAR   TxChainAddr3L_Byte0; // Destination MAC address of Tx chain3 (byte 0)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR3_L_STRUC, *PTX_CHAIN_ADDR3_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR3_L_STRUC
{
    struct
    {
        UCHAR   TxChainAddr3L_Byte0; // Destination MAC address of Tx chain3 (byte 0)
        UCHAR   TxChainAddr3L_Byte1; // Destination MAC address of Tx chain3 (byte 1)
        UCHAR   TxChainAddr3L_Byte2; // Destination MAC address of Tx chain3 (byte 2)
        UCHAR   TxChainAddr3L_Byte3; // Destination MAC address of Tx chain3 (byte 3)
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR3_L_STRUC, *PTX_CHAIN_ADDR3_L_STRUC;
#endif

#define TX_CHAIN_ADDR3_H    0x1060

#ifdef BIG_ENDIAN
typedef union _TX_CHAIN_ADDR3_H_STRUC
{
    struct
    {
        USHORT  Reserved:12; // Reserved
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        UCHAR   TxChainAddr3H_Byte5; // Destination MAC address of Tx chain3 (byte 5)
        UCHAR   TxChainAddr3H_Byte4; // Destination MAC address of Tx chain3 (byte 4)
    } field;
    
    ULONG word;
} TX_CHAIN_ADDR3_H_STRUC, *PTX_CHAIN_ADDR3_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR3_H_STRUC
{
    struct
    {
        UCHAR   TxChainAddr3H_Byte4; // Destination MAC address of Tx chain3 (byte 4)
        UCHAR   TxChainAddr3H_Byte5; // Destination MAC address of Tx chain3 (byte 5)       
        USHORT  TxChainSel0:4; // Selection value of Tx chain0
        USHORT  Reserved:12; // Reserved
    } field;
    
    ULONG word;
}   TX_CHAIN_ADDR3_H_STRUC, *PTX_CHAIN_ADDR3_HA_STRUC;
#endif


//
//  4.2 MAC TIMING  configuration registers (offset:0x1100)
//
#define XIFS_TIME_CFG             0x1100         // MAC_CSR8  MAC_CSR9   
#ifdef BIG_ENDIAN
typedef union   _IFS_SLOT_CFG_STRUC {
    struct  {
        ULONG  rsv:2;         
        ULONG  BBRxendEnable:1;        //  reference RXEND signal to begin XIFS defer
        ULONG  EIFS:9;        //  unit 1us
        ULONG  OfdmXifsTime:4;        //OFDM SIFS. unit 1us. Applied after OFDM RX when MAC doesn't reference BBP signal BBRXEND
        ULONG  OfdmSifsTime:8;        //  unit 1us. Applied after OFDM RX/TX
        ULONG  CckmSifsTime:8;        //  unit 1us. Applied after CCK RX/TX
    }   field;
    ULONG           word;
}   IFS_SLOT_CFG_STRUC, *PIFS_SLOT_CFG_STRUC;
#else
typedef union   _IFS_SLOT_CFG_STRUC {
    struct  {
        ULONG  CckmSifsTime:8;        //  unit 1us. Applied after CCK RX/TX
        ULONG  OfdmSifsTime:8;        //  unit 1us. Applied after OFDM RX/TX
        ULONG  OfdmXifsTime:4;        //OFDM SIFS. unit 1us. Applied after OFDM RX when MAC doesn't reference BBP signal BBRXEND
        ULONG  EIFS:9;        //  unit 1us
        ULONG  BBRxendEnable:1;        //  reference RXEND signal to begin XIFS defer
        ULONG  rsv:2;         
    }   field;
    ULONG           word;
}   IFS_SLOT_CFG_STRUC, *PIFS_SLOT_CFG_STRUC;
#endif

#define BKOFF_SLOT_CFG             0x1104        //  mac_csr9 last 8 bits
//
// BKOFF_SLOT_CFG : Timing control register
//
#ifdef BIG_ENDIAN
typedef union   _BKOFF_SLOT_CFG_STRUC   {
    struct  {
        ULONG   Rsv:20;
        ULONG   ChannelClearDelay:4;    // specified the TX guard time after channel is clear
        ULONG   SlotTime:8;         // specified the slot boundary after deferring SIFS time
    }   field;
    ULONG           word;
}   BKOFF_SLOT_CFG_STRUC, *PBKOFF_SLOT_CFG_STRUC;
#else
typedef union   _BKOFF_SLOT_CFG_STRUC   {
    struct  {
        ULONG   SlotTime:8;         // specified the slot boundary after deferring SIFS time
        ULONG   ChannelClearDelay:4;    // specified the TX guard time after channel is clear
        ULONG   Rsv:20;
    }   field;
    ULONG           word;
}   BKOFF_SLOT_CFG_STRUC, *PBKOFF_SLOT_CFG_STRUC;
#endif

#define NAV_TIME_CFG             0x1108      // NAV  (MAC_CSR15)
#define CH_TIME_CFG             0x110C          // Count as channel busy 
#define PBF_LIFE_TIMER             0x1110        //TX/RX MPDU timestamp timer (free run)Unit: 1us  
#define BCN_TIME_CFG             0x1114      // TXRX_CSR9
//
// BCN_TIME_CFG : Synchronization control register
//
#ifdef BIG_ENDIAN
typedef union   _BCN_TIME_CFG_STRUC {
    struct  {
        ULONG       TxTimestampCompensate:8;
        ULONG       :3;
        ULONG       bBeaconGen:1;       // Enable beacon generator
        ULONG       bTBTTEnable:1;
        ULONG       TsfSyncMode:2;      // Enable TSF sync, 00: disable, 01: infra mode, 10: ad-hoc mode
        ULONG       bTsfTicking:1;      // Enable TSF auto counting
        ULONG       BeaconInterval:16;  // in unit of 1/16 TU
    }   field;
    ULONG           word;
}   BCN_TIME_CFG_STRUC, *PBCN_TIME_CFG_STRUC;
#else
typedef union   _BCN_TIME_CFG_STRUC {
    struct  {
        ULONG       BeaconInterval:16;  // in unit of 1/16 TU
        ULONG       bTsfTicking:1;      // Enable TSF auto counting
        ULONG       TsfSyncMode:2;      // Enable TSF sync, 00: disable, 01: infra mode, 10: ad-hoc mode
        ULONG       bTBTTEnable:1;
        ULONG       bBeaconGen:1;       // Enable beacon generator
        ULONG       :3;
        ULONG       TxTimestampCompensate:8;
    }   field;
    ULONG           word;
}   BCN_TIME_CFG_STRUC, *PBCN_TIME_CFG_STRUC;
#endif
#define TBTT_SYNC_CFG            0x1118         // txrx_csr10
#define TSF_TIMER_DW0             0x111C        // Local TSF timer lsb 32 bits. Read-only
#define TSF_TIMER_DW1             0x1120        // msb 32 bits. Read-only.
#define TBTT_TIMER              0x1124          // TImer remains till next TBTT. Read-only.  TXRX_CSR14
#define INT_TIMER_CFG               0x1128          //  
#define INT_TIMER_EN                0x112c          //  GP-timer and pre-tbtt Int enable
#define CH_IDLE_STA                 0x1130          //  channel idle time
#define CH_BUSY_STA                 0x1134          //  channle busy time

#define BBP_IPI_TIMER           0x113C

#ifdef BIG_ENDIAN
typedef union _BBP_IPI_TIMER_STRUC
{
    struct
    {
        ULONG   Reserved:15;
        ULONG   BBP_IPI_KICK:1; // (a) Write 1: Kick-off the BBP IPI measurement, (b) Read 1: BBP IPI is enabled., (c) Read 0: BBP IPI is disabled.
        ULONG   BBP_IPI_Period:16; // The measurement period of the BBP IPI, in unit of 1.024 ms
    } field;
    
    ULONG       word;
} BBP_IPI_TIMER_STRUC, *PBBP_IPI_TIMER_STRUC;
#else
typedef union _BBP_IPI_TIMER_STRUC
{
    struct
    {
        ULONG   BBP_IPI_Period:16; // The measurement period of the BBP IPI, in unit of 1.024 ms
        ULONG   BBP_IPI_KICK:1; // (a) Write 1: Kick-off the BBP IPI measurement, (b) Read 1: BBP IPI is enabled., (c) Read 0: BBP IPI is disabled.
        ULONG   Reserved:15;
    } field;

    ULONG       word;
} BBP_IPI_TIMER_STRUC, *PBBP_IPI_TIMER_STRUC;
#endif

//
//  4.2 MAC POWER  configuration registers (offset:0x1200)
//
#define MAC_STATUS_CFG             0x1200        // old MAC_CSR12
#define PWR_PIN_CFG             0x1204       // old MAC_CSR12
#define AUTO_WAKEUP_CFG             0x1208       // old MAC_CSR10
//
// AUTO_WAKEUP_CFG: Manual power control / status register 
//
#ifdef BIG_ENDIAN
typedef union   _AUTO_WAKEUP_STRUC  {
    struct  {
        ULONG       :16;
        ULONG       EnableAutoWakeup:1; // 0:sleep, 1:awake
        ULONG       NumofSleepingTbtt:7;          // ForceWake has high privilege than PutToSleep when both set
        ULONG       AutoLeadTime:8;
    }   field;
    ULONG           word;
}   PAUTO_WAKEUP_STRUC, *PAUTO_WAKEUP_STRUC;
#else
typedef union   _AUTO_WAKEUP_STRUC  {
    struct  {
        ULONG       AutoLeadTime:8;
        ULONG       NumofSleepingTbtt:7;          // ForceWake has high privilege than PutToSleep when both set
        ULONG       EnableAutoWakeup:1; // 0:sleep, 1:awake
        ULONG       :16;
    }   field;
    ULONG           word;
}   AUTO_WAKEUP_STRUC, *PAUTO_WAKEUP_STRUC;
#endif

//
//  4.3 MAC TX  configuration registers (offset:0x1300)
//       

#define EDCA_AC0_CFG    0x1300      //AC_TXOP_CSR0 0x3474
#define EDCA_AC1_CFG    0x1304
#define EDCA_AC2_CFG    0x1308
#define EDCA_AC3_CFG    0x130c
#ifdef BIG_ENDIAN
typedef union   _EDCA_AC_CFG_STRUC  {
    struct  {
        ULONG  :12;        // 
        ULONG  Cwmax:4;        //unit power of 2
        ULONG  Cwmin:4;        // 
        ULONG  Aifsn:4;        // # of slot time
        ULONG  AcTxop:8;        //  in unit of 32us
    }   field;
    ULONG           word;
}   EDCA_AC_CFG_STRUC, *PEDCA_AC_CFG_STRUC;
#else
typedef union   _EDCA_AC_CFG_STRUC  {
    struct  {
        ULONG  AcTxop:8;        //  in unit of 32us
        ULONG  Aifsn:4;        // # of slot time
        ULONG  Cwmin:4;        // 
        ULONG  Cwmax:4;        //unit power of 2
        ULONG  :12;       // 
    }   field;
    ULONG           word;
}   EDCA_AC_CFG_STRUC, *PEDCA_AC_CFG_STRUC;
#endif

#define EDCA_TID_AC_MAP 0x1310

//
// Default Tx power
//
#define DEFAULT_TX_POWER    0x6

//
// Tx power control over MAC
//
#define TX_PWR_CFG_0    0x1314
#define TX_PWR_CFG_0_EXT    0x1390
#define TX_PWR_CFG_1    0x1318
#define TX_PWR_CFG_1_EXT    0x1394
#define TX_PWR_CFG_2    0x131C
#define TX_PWR_CFG_2_EXT    0x1398
#define TX_PWR_CFG_3    0x1320
#define TX_PWR_CFG_3_EXT    0x139C
#define TX_PWR_CFG_4    0x1324
#define TX_PWR_CFG_4_EXT    0x13A0
#define TX_PWR_CFG_5            0x1384
#define TX_PWR_CFG_6            0x1388
#define TX_PWR_CFG_7            0x13D4
#define TX_PWR_CFG_8            0x13D8
#define TX_PWR_CFG_9            0x13DC

#ifdef BIG_ENDIAN
typedef union   _TX_PWR_CFG_STRUC   {
    struct  {
        ULONG       Byte3:8;     
        ULONG       Byte2:8;     
        ULONG       Byte1:8;     
        ULONG       Byte0:8;     
    }   field;
    ULONG           word;
}   TX_PWR_CFG_STRUC, *PTX_PWR_CFG_STRUC;
#else
typedef union   _TX_PWR_CFG_STRUC   {
    struct  {
        ULONG       Byte0:8;     
        ULONG       Byte1:8;     
        ULONG       Byte2:8;     
        ULONG       Byte3:8;     
    }   field;
    ULONG           word;
}   TX_PWR_CFG_STRUC, *PTX_PWR_CFG_STRUC;
#endif

#define TX_PIN_CFG      0x1328       
#define TX_BAND_CFG 0x132c      // 0x1 use upper 20MHz. 0 juse lower 20MHz
#define TX_SW_CFG0      0x1330
#define TX_SW_CFG1      0x1334
#define TX_SW_CFG2      0x1338
#define TXOP_THRES_CFG      0x133c

#ifdef BIG_ENDIAN
typedef union _TXOP_THRESHOLD_CFG_STRUC
{
    struct
    {
        ULONG   TXOP_REM_THRES:8; // Remaining TXOP threshold (unit: 32us)
        ULONG   CF_END_THRES:8; // CF-END threshold (unit: 32us)
        ULONG   RDG_IN_THRES:8; // Rx RDG threshold (unit: 32us)
        ULONG   RDG_OUT_THRES:8; // Tx RDG threshold (unit: 32us)
    } field;
    
    ULONG       word;
} TXOP_THRESHOLD_CFG_STRUC, *PTXOP_THRESHOLD_CFG_STRUC;
#else
typedef union _TXOP_THRESHOLD_CFG_STRUC
{
    struct
    {
        ULONG   RDG_OUT_THRES:8; // Tx RDG threshold (unit: 32us)
        ULONG   RDG_IN_THRES:8; // Rx RDG threshold (unit: 32us)
        ULONG   CF_END_THRES:8; // CF-END threshold (unit: 32us)
        ULONG   TXOP_REM_THRES:8; // Remaining TXOP threshold (unit: 32us)
    } field;
    
    ULONG       word;
} TXOP_THRESHOLD_CFG_STRUC, *PTXOP_THRESHOLD_CFG_STRUC;
#endif

#define TXOP_CTRL_CFG       0x1340
#define TX_RTS_CFG      0x1344
#ifdef BIG_ENDIAN
typedef union   _TX_RTS_CFG_STRUC   {
    struct  {
        ULONG       rsv:7;     
        ULONG       RtsFbkEn:1;    // enable rts rate fallback
        ULONG       RtsThres:16;    // unit:byte
        ULONG       AutoRtsRetryLimit:8;
    }   field;
    ULONG           word;
}   TX_RTS_CFG_STRUC, *PTX_RTS_CFG_STRUC;
#else
typedef union   _TX_RTS_CFG_STRUC   {
    struct  {
        ULONG       AutoRtsRetryLimit:8;
        ULONG       RtsThres:16;    // unit:byte
        ULONG       RtsFbkEn:1;    // enable rts rate fallback
        ULONG       rsv:7;     // 1: HT non-STBC control frame enable
    }   field;
    ULONG           word;
}   TX_RTS_CFG_STRUC, *PTX_RTS_CFG_STRUC;
#endif
#define TX_TIMEOUT_CFG  0x1348
#ifdef BIG_ENDIAN
typedef union   _TX_TIMEOUT_CFG_STRUC   {
    struct  {
        ULONG       rsv2:8;     
        ULONG       TxopTimeout:8;  //TXOP timeout value for TXOP truncation.  It is recommended that (SLOT_TIME) > (TX_OP_TIMEOUT) > (RX_ACK_TIMEOUT)
        ULONG       RxAckTimeout:8; // unit:slot. Used for TX precedure
        ULONG       MpduLifeTime:4;    //  expiration time = 2^(9+MPDU LIFE TIME)  us
        ULONG       rsv:4;     
    }   field;
    ULONG           word;
}   TX_TIMEOUT_CFG_STRUC, *PTX_TIMEOUT_CFG_STRUC;
#else
typedef union   _TX_TIMEOUT_CFG_STRUC   {
    struct  {
        ULONG       rsv:4;     
        ULONG       MpduLifeTime:4;    //  expiration time = 2^(9+MPDU LIFE TIME)  us
        ULONG       RxAckTimeout:8; // unit:slot. Used for TX precedure
        ULONG       TxopTimeout:8;  //TXOP timeout value for TXOP truncation.  It is recommended that (SLOT_TIME) > (TX_OP_TIMEOUT) > (RX_ACK_TIMEOUT)
        ULONG       rsv2:8;     // 1: HT non-STBC control frame enable
    }   field;
    ULONG           word;
}   TX_TIMEOUT_CFG_STRUC, *PTX_TIMEOUT_CFG_STRUC;
#endif
#define TX_RTY_CFG  0x134c
#ifdef BIG_ENDIAN
typedef union   _TX_RTY_CFG_STRUC   {
    struct  {
        ULONG       rsv:1;     
        ULONG       TxautoFBEnable:1;    // Tx retry PHY rate auto fallback enable
        ULONG       AggRtyMode:1;   // Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer
        ULONG       NonAggRtyMode:1;    // Non-Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer
        ULONG       LongRtyThre:12; // Long retry threshoold
        ULONG       LongRtyLimit:8; //long retry limit
        ULONG       ShortRtyLimit:8;    //  short retry limit
        
    }   field;
    ULONG           word;
}   TX_RTY_CFG_STRUC, *PTX_RTY_CFG_STRUC;
#else
typedef union   _TX_RTY_CFG_STRUC   {
    struct  {
        ULONG       ShortRtyLimit:8;    //  short retry limit
        ULONG       LongRtyLimit:8; //long retry limit
        ULONG       LongRtyThre:12; // Long retry threshoold
        ULONG       NonAggRtyMode:1;    // Non-Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer
        ULONG       AggRtyMode:1;   // Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer
        ULONG       TxautoFBEnable:1;    // Tx retry PHY rate auto fallback enable
        ULONG       rsv:1;     // 1: HT non-STBC control frame enable
    }   field;
    ULONG           word;
}   TX_RTY_CFG_STRUC, *PTX_RTY_CFG_STRUC;
#endif
#define TX_LINK_CFG 0x1350
#ifdef BIG_ENDIAN
typedef union   _TX_LINK_CFG_STRUC  {
    struct  {
        ULONG       RemotMFS:8; //remote MCS feedback sequence number     
        ULONG       RemotMFB:8;    //  remote MCS feedback
        ULONG       rsv:3;  //  
        ULONG       TxCFAckEn:1;    //   Piggyback CF-ACK enable
        ULONG       TxRDGEn:1;  // RDG TX enable 
        ULONG       TxMRQEn:1;  //  MCS request TX enable
        ULONG       RemoteUMFSEnable:1; //  remote unsolicit  MFB enable.  0: not apply remote remote unsolicit (MFS=7)
        ULONG       MFBEnable:1;    //  TX apply remote MFB 1:enable
        ULONG       RemoteMFBLifeTime:8;    //remote MFB life time. unit : 32us
    }   field;
    ULONG           word;
}   TX_LINK_CFG_STRUC, *PTX_LINK_CFG_STRUC;
#else
typedef union   _TX_LINK_CFG_STRUC  {
    struct  {
        ULONG       RemoteMFBLifeTime:8;    //remote MFB life time. unit : 32us
        ULONG       MFBEnable:1;    //  TX apply remote MFB 1:enable
        ULONG       RemoteUMFSEnable:1; //  remote unsolicit  MFB enable.  0: not apply remote remote unsolicit (MFS=7)
        ULONG       TxMRQEn:1;  //  MCS request TX enable
        ULONG       TxRDGEn:1;  // RDG TX enable 
        ULONG       TxCFAckEn:1;    //   Piggyback CF-ACK enable
        ULONG       rsv:3;  //  
        ULONG       RemotMFB:8;    //  remote MCS feedback
        ULONG       RemotMFS:8; //remote MCS feedback sequence number     
    }   field;
    ULONG           word;
}   TX_LINK_CFG_STRUC, *PTX_LINK_CFG_STRUC;
#endif

#define LG_FBK_CFG0 0x135c
#ifdef BIG_ENDIAN
typedef union   _LG_FBK_CFG0_STRUC  {
    struct  {
        ULONG       OFDMMCS7FBK:4;  //initial value is 6
        ULONG       OFDMMCS6FBK:4;  //initial value is 5
        ULONG       OFDMMCS5FBK:4;  //initial value is 4
        ULONG       OFDMMCS4FBK:4;  //initial value is 3
        ULONG       OFDMMCS3FBK:4;  //initial value is 2
        ULONG       OFDMMCS2FBK:4;  //initial value is 1
        ULONG       OFDMMCS1FBK:4;  //initial value is 0
        ULONG       OFDMMCS0FBK:4;  //initial value is 0
    }   field;
    ULONG           word;
}   LG_FBK_CFG0_STRUC, *PLG_FBK_CFG0_STRUC;
#else
typedef union   _LG_FBK_CFG0_STRUC  {
    struct  {
        ULONG       OFDMMCS0FBK:4;  //initial value is 0
        ULONG       OFDMMCS1FBK:4;  //initial value is 0
        ULONG       OFDMMCS2FBK:4;  //initial value is 1
        ULONG       OFDMMCS3FBK:4;  //initial value is 2
        ULONG       OFDMMCS4FBK:4;  //initial value is 3
        ULONG       OFDMMCS5FBK:4;  //initial value is 4
        ULONG       OFDMMCS6FBK:4;  //initial value is 5
        ULONG       OFDMMCS7FBK:4;  //initial value is 6
    }   field;
    ULONG           word;
}   LG_FBK_CFG0_STRUC, *PLG_FBK_CFG0_STRUC;
#endif

//=======================================================
//================ Protection Paramater================================
//=======================================================
#define CCK_PROT_CFG    0x1364      //CCK Protection
#define ASIC_SHORTNAV       1
#define ASIC_LONGNAV        2
#define ASIC_RTS        1
#define ASIC_CTS        2
#ifdef BIG_ENDIAN
typedef union   _PROT_CFG_STRUC {
    struct  {
        ULONG       TxopAllowVht80:1;   // (VHT)TXOP allowance.0:disallow.
        ULONG       TxopAllowVht40:1;   // (VHT)TXOP allowance.0:disallow.
        ULONG       TxopAllowVht20:1;   // (VHT)TXOP allowance.0:disallow.
        ULONG       DynCbw:1;           // (VHT)RTS use dynamic channel bandwidth when Tx signaling mode is turned on.
        ULONG       RtsTaSignal:1;      // (VHT)RTS TA signaling mode
        ULONG       RTSThEn:1;  //RTS threshold enable on CCK TX
        ULONG       TxopAllowGF40:1;    //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowGF20:1;    //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowMM40:1;    //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowMM20:1;    //CCK TXOP allowance. 0:disallow.
        ULONG       TxopAllowOfdm:1;    //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowCck:1; //CCK TXOP allowance.0:disallow.
        ULONG       ProtectNav:2;   //TXOP protection type for CCK TX. 0:None, 1:ShortNAVprotect,  2:LongNAVProtect, 3:rsv
        ULONG       ProtectCtrl:2;  //Protection control frame type for CCK TX. 1:RTS/CTS, 2:CTS-to-self, 0:None, 3:rsv
        ULONG       ProtectRate:16; //Protection control frame rate for CCK TX(RTS/CTS/CFEnd). 
    }   field;
    ULONG           word;
}   PROT_CFG_STRUC, *PPROT_CFG_STRUC;
#else
typedef union   _PROT_CFG_STRUC {
    struct  {
        ULONG       ProtectRate:16; //Protection control frame rate for CCK TX(RTS/CTS/CFEnd). 
        ULONG       ProtectCtrl:2;  //Protection control frame type for CCK TX. 1:RTS/CTS, 2:CTS-to-self, 0:None, 3:rsv
        ULONG       ProtectNav:2;   //TXOP protection type for CCK TX. 0:None, 1:ShortNAVprotect,  2:LongNAVProtect, 3:rsv
        ULONG       TxopAllowCck:1; //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowOfdm:1;    //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowMM20:1;    //CCK TXOP allowance. 0:disallow.
        ULONG       TxopAllowMM40:1;    //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowGF20:1;    //CCK TXOP allowance.0:disallow.
        ULONG       TxopAllowGF40:1;    //CCK TXOP allowance.0:disallow.
        ULONG       RTSThEn:1;  //RTS threshold enable on CCK TX
        ULONG       RtsTaSignal:1;      // (VHT)RTS TA signaling mode
        ULONG       DynCbw:1;           // (VHT)RTS use dynamic channel bandwidth when Tx signaling mode is turned on.
        ULONG       TxopAllowVht20:1;   // (VHT)TXOP allowance.0:disallow.
        ULONG       TxopAllowVht40:1;   // (VHT)TXOP allowance.0:disallow.      
        ULONG       TxopAllowVht80:1;   // (VHT)TXOP allowance.0:disallow.
    }   field;
    ULONG           word;
}   PROT_CFG_STRUC, *PPROT_CFG_STRUC;
#endif

#define OFDM_PROT_CFG   0x1368      //OFDM Protection
#define MM20_PROT_CFG   0x136C      //MM20 Protection
#define MM40_PROT_CFG   0x1370      //MM40 Protection
#define GF20_PROT_CFG   0x1374      //GF20 Protection
#define GF40_PROT_CFG   0x1378      //GR40 Protection
#define EXP_CTS_TIME    0x137C      // 
#define EXP_ACK_TIME    0x1380      // 
#define HT_FBK_TO_LEGACY    0x1384
#define TX_MPDU_ADJ_INT 0x1388

#define TX_TXBF_CFG_0       0x138C
#define TX_TXBF_CFG_1       0x13A4
#define TX_TXBF_CFG_2       0x13A8
#define TX_TXBF_CFG_3       0x13AC

#define HT_FBK_3SS_CFG0 0x13C4

#ifdef BIG_ENDIAN
typedef union _HT_FBK_3SS_CFG0_STRUC
{
    struct
    {
        ULONG   HTMCS19FBK:5;
        ULONG   rsv3:3;
        ULONG   HTMCS18FBK:5;
        ULONG   rsv2:3;
        ULONG   HTMCS17BK:5;
        ULONG   rsv1:3;
        ULONG   HTMCS16FBK:5;
        ULONG   rsv0:3;
    } field;
    
    ULONG       word;
} HT_FBK_3SS_CFG0_STRUC, *PHT_FBK_3SS_CFG0_STRUC;
#else
typedef union _HT_FBK_3SS_CFG0_STRUC
{
    struct
    {
        ULONG   HTMCS16FBK:5;
        ULONG   rsv0:3;
        ULONG   HTMCS17FBK:5;
        ULONG   rsv1:3;
        ULONG   HTMCS18FBK:5;
        ULONG   rsv2:3;
        ULONG   HTMCS19FBK:5;
        ULONG   rsv3:3;
    } field;
    
    ULONG       word;
} HT_FBK_3SS_CFG0_STRUC, *PHT_FBK_3SS_CFG0_STRUC;
#endif

#define HT_FBK_3SS_CFG1 0x13C8

#ifdef BIG_ENDIAN
typedef union _HT_FBK_3SS_CFG1_STRUC
{
    struct
    {
        ULONG   HTMCS23FBK:5;
        ULONG   rsv3:3;
        ULONG   HTMCS22FBK:5;
        ULONG   rsv2:3;
        ULONG   HTMCS21BK:5;
        ULONG   rsv1:3;
        ULONG   HTMCS20FBK:5;
        ULONG   rsv0:3;
    }   field;

    ULONG       word;
} HT_FBK_3SS_CFG1_STRUC, *PHT_FBK_3SS_CFG1_STRUC;
#else
typedef union _HT_FBK_3SS_CFG1_STRUC
{
    struct
    {
        ULONG   HTMCS20FBK:5;
        ULONG   rsv0:3;
        ULONG   HTMCS21FBK:5;
        ULONG   rsv1:3;
        ULONG   HTMCS22FBK:5;
        ULONG   rsv2:3;
        ULONG   HTMCS23FBK:5;
        ULONG   rsv3:3;
    }   field;

    ULONG       word;
} HT_FBK_3SS_CFG1_STRUC, *PHT_FBK_3SS_CFG1_STRUC;
#endif

//
//  4.4 MAC RX configuration registers (offset:0x1400)
//
#define RX_FILTR_CFG    0x1400          //TXRX_CSR0
#define AUTO_RSP_CFG    0x1404          //TXRX_CSR4
//
// TXRX_CSR4: Auto-Responder/
//
#ifdef BIG_ENDIAN
typedef union   _AUTO_RSP_CFG_STRUC {
    struct  {
        ULONG   :24;             
        ULONG   AckCtsPsmBit:1;         // Power bit value in conrtrol frame
        ULONG   DualCTSEn:1;            // Power bit value in conrtrol frame
        ULONG   rsv:1;          // Power bit value in conrtrol frame
        ULONG   AutoResponderPreamble:1;    // 0:long, 1:short preamble
        ULONG   CTS40MRef:1;        // Response CTS 40MHz duplicate mode
        ULONG   CTS40MMode:1;       // Response CTS 40MHz duplicate mode
        ULONG   BACAckPolicyEnable:1;    // 0:long, 1:short preamble
        ULONG   AutoResponderEnable:1;
    }   field;
    ULONG           word;
}   AUTO_RSP_CFG_STRUC, *PAUTO_RSP_CFG_STRUC;
#else
typedef union   _AUTO_RSP_CFG_STRUC {
    struct  {
        ULONG   AutoResponderEnable:1;
        ULONG   BACAckPolicyEnable:1;    // 0:long, 1:short preamble
        ULONG   CTS40MMode:1;       // Response CTS 40MHz duplicate mode
        ULONG   CTS40MRef:1;        // Response CTS 40MHz duplicate mode
        ULONG   AutoResponderPreamble:1;    // 0:long, 1:short preamble
        ULONG   rsv:1;          // Power bit value in conrtrol frame
        ULONG   DualCTSEn:1;            // Power bit value in conrtrol frame
        ULONG   AckCtsPsmBit:1;         // Power bit value in conrtrol frame
        ULONG        :24;            
    }   field;
    ULONG           word;
}   AUTO_RSP_CFG_STRUC, *PAUTO_RSP_CFG_STRUC;
#endif
#define LEGACY_BASIC_RATE   0x1408  //  TXRX_CSR5           0x3054
#define HT_BASIC_RATE       0x140c
#define HT_CTRL_CFG         0x1410
#define SIFS_COST_CFG       0x1414
#define RX_PARSER_CFG       0x1418  //Set NAV for all received frames
#define EXT_CCA_CFG         0x141C

//
//  4.5 MAC Security configuration (offset:0x1500)
//
#define TX_SEC_CNT0     0x1500      //
#define RX_SEC_CNT0     0x1504      //
#define CCMP_FC_MUTE        0x1508      //

#define RX_PN_PADDING_CFG 0x150C

#ifdef BIG_ENDIAN
typedef union _RX_PN_PADDING_CFG_STRUC
{
    struct
    {
        ULONG   Reserved:31;
        ULONG   RxPnPadEn:1; // Enable Rx IV/EIV/PN padding
    } field;

    ULONG       word;
} RX_PN_PADDING_CFG_STRUC, *PRX_PN_PADDING_CFG_STRUC;
#else
typedef union _RX_PN_PADDING_CFG_STRUC
{
    struct
    {
        ULONG   RxPnPadEn:1; // Enable Rx IV/EIV/PN padding
        ULONG   Reserved:31;
    } field;

    ULONG       word;
} RX_PN_PADDING_CFG_STRUC, *PRX_PN_PADDING_CFG_STRUC;
#endif

//
//  4.6 HCCA/PSMP (offset:0x1600)
//
#define TXOP_HLDR_ADDR0     0x1600       
#define TXOP_HLDR_ADDR1     0x1604       
#define TXOP_HLDR_ET        0x1608       
#define QOS_CFPOLL_RA_DW0       0x160c
#define QOS_CFPOLL_A1_DW1       0x1610
#define QOS_CFPOLL_QC       0x1614
//
//  4.7 MAC Statistis registers (offset:0x1700)
//
#define RX_STA_CNT0     0x1700      //
#define RX_STA_CNT1     0x1704      //
#define RX_STA_CNT2     0x1708      //

//
// RX_STA_CNT0_STRUC: RX PLCP error count & RX CRC error count
//
#ifdef BIG_ENDIAN
typedef union   _RX_STA_CNT0_STRUC  {
    struct  {
        USHORT  PhyErr;
        USHORT  CrcErr;
    }   field;
    ULONG           word;
}   RX_STA_CNT0_STRUC, *PRX_STA_CNT0_STRUC;
#else
typedef union   _RX_STA_CNT0_STRUC  {
    struct  {
        USHORT  CrcErr;
        USHORT  PhyErr;
    }   field;
    ULONG           word;
}   RX_STA_CNT0_STRUC, *PRX_STA_CNT0_STRUC;
#endif

//
// RX_STA_CNT1_STRUC: RX False CCA count & RX LONG frame count
//
#ifdef BIG_ENDIAN
typedef union   _RX_STA_CNT1_STRUC  {
    struct  {
        USHORT  PlcpErr;
        USHORT  FalseCca;
    }   field;
    ULONG           word;
}   RX_STA_CNT1_STRUC, *PRX_STA_CNT1_STRUC;
#else
typedef union   _RX_STA_CNT1_STRUC  {
    struct  {
        USHORT  FalseCca;
        USHORT  PlcpErr;
    }   field;
    ULONG           word;
}   RX_STA_CNT1_STRUC, *PRX_STA_CNT1_STRUC;
#endif

//
// RX_STA_CNT2_STRUC: 
//
#ifdef BIG_ENDIAN
typedef union   _RX_STA_CNT2_STRUC  {
    struct  {
        USHORT  RxFifoOverflowCount;
        USHORT  RxDupliCount;
    }   field;
    ULONG           word;
}   RX_STA_CNT2_STRUC, *PRX_STA_CNT2_STRUC;
#else
typedef union   _RX_STA_CNT2_STRUC  {
    struct  {
        USHORT  RxDupliCount;
        USHORT  RxFifoOverflowCount;
    }   field;
    ULONG           word;
}   RX_STA_CNT2_STRUC, *PRX_STA_CNT2_STRUC;
#endif

//
// TX_STA_CNT2: TX tx count
//
#ifdef BIG_ENDIAN
typedef union   _TX_STA_CNT2_STRUC  {
    struct  {
        USHORT  TxUnderFlowCount;
        USHORT  TxZeroLenCount;
    }   field;
    ULONG           word;
}   TX_STA_CNT2_STRUC, *PTX_STA_CNT2_STRUC;
#else
typedef union   _TX_STA_CNT2_STRUC  {
    struct  {
        USHORT  TxZeroLenCount;
        USHORT  TxUnderFlowCount;
    }   field;
    ULONG           word;
}   TX_STA_CNT2_STRUC, *PTX_STA_CNT2_STRUC;
#endif

//
// The extension of TX_STAT_FIFO (read before reading 0x1718)
//
#define TX_STAT_FIFO_EXT (0x1798)

typedef union _TX_STAT_FIFO_EXT_STRUC   {
    struct
    {
        ULONG TX_RTY_CNT:8; // Tx retry count
        ULONG TX_PKT_ID:8; // Tx packet ID (copied from per-packet TXWI)
        ULONG Reserved:16;
    } field;

    ULONG word;
} TX_STAT_FIFO_EXT_STRUC, *PTX_STAT_FIFO_EXT_STRUC;

#define TX_STA_FIFO     0x1718      //
//
// TX_STA_FIFO_STRUC: TX Result for specific PID status fifo register
//
#ifdef BIG_ENDIAN
typedef union   _TX_STA_FIFO_STRUC  {
    struct  {
        ULONG       Reserve:2; // Reserve
        ULONG       ITxBf:1; // Implicit TxBF
        ULONG       Sounding:1; // Sounding
        ULONG       ETxBf:1; // Explicit TxBF
        ULONG           SuccessRate:11; //include MCS, mode ,shortGI, BW settingSame format as TXWI Word 0 Bit 31-16. 
        ULONG       wcid:8;     //wireless client index
        ULONG           TxAckRequired:1;    // ack required
        ULONG           TxAggre:1;    // Tx is aggregated
        ULONG           TxSuccess:1;   // Tx success. whether success or not
        ULONG           PidType:4;
        ULONG       bValid:1;   // 1:This register contains a valid TX result
    }   field;
    ULONG           word;
}   TX_STA_FIFO_STRUC, *PTX_STA_FIFO_STRUC;
#else
typedef union   _TX_STA_FIFO_STRUC  {
    struct  {
        ULONG       bValid:1;   // 1:This register contains a valid TX result
        ULONG           PidType:4;
        ULONG           TxSuccess:1;   // Tx No retry success
        ULONG           TxAggre:1;    // Tx Retry Success
        ULONG           TxAckRequired:1;    // Tx fail
        ULONG       wcid:8;     //wireless client index
        ULONG       SuccessRate:11; //include MCS, mode ,shortGI, BW settingSame format as TXWI Word 0 Bit 31-16. 
        ULONG       ETxBf:1; // Explicit TxBF
        ULONG       Sounding:1; // Sounding
        ULONG       ITxBf:1; // Implicit TxBF
        ULONG       Reserve:2; // Reserve
    }   field;
    ULONG           word;
}   TX_STA_FIFO_STRUC, *PTX_STA_FIFO_STRUC;
#endif
// Debug counter
#define TX_AGG_CNT  0x171c
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT_STRUC   {
    struct  {
        USHORT  AggTxCount;
        USHORT  NonAggTxCount;
    }   field;
    ULONG           word;
}   TX_AGG_CNT_STRUC, *PTX_AGG_CNT_STRUC;
#else
typedef union   _TX_AGG_CNT_STRUC   {
    struct  {
        USHORT  NonAggTxCount;
        USHORT  AggTxCount;
    }   field;
    ULONG           word;
}   TX_AGG_CNT_STRUC, *PTX_AGG_CNT_STRUC;
#endif
// Debug counter
#define TX_AGG_CNT0 0x1720
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT0_STRUC  {
    struct  {
        USHORT  AggSize2Count;
        USHORT  AggSize1Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT0_STRUC, *PTX_AGG_CNT0_STRUC;
#else
typedef union   _TX_AGG_CNT0_STRUC  {
    struct  {
        USHORT  AggSize1Count;
        USHORT  AggSize2Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT0_STRUC, *PTX_AGG_CNT0_STRUC;
#endif
// Debug counter
#define TX_AGG_CNT1 0x1724
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT1_STRUC  {
    struct  {
        USHORT  AggSize4Count;
        USHORT  AggSize3Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT1_STRUC, *PTX_AGG_CNT1_STRUC;
#else
typedef union   _TX_AGG_CNT1_STRUC  {
    struct  {
        USHORT  AggSize3Count;
        USHORT  AggSize4Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT1_STRUC, *PTX_AGG_CNT1_STRUC;
#endif
#define TX_AGG_CNT2 0x1728
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT2_STRUC  {
    struct  {
        USHORT  AggSize6Count;
        USHORT  AggSize5Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT2_STRUC, *PTX_AGG_CNT2_STRUC;
#else
typedef union   _TX_AGG_CNT2_STRUC  {
    struct  {
        USHORT  AggSize5Count;
        USHORT  AggSize6Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT2_STRUC, *PTX_AGG_CNT2_STRUC;
#endif
// Debug counter
#define TX_AGG_CNT3 0x172c
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT3_STRUC  {
    struct  {
        USHORT  AggSize8Count;
        USHORT  AggSize7Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT3_STRUC, *PTX_AGG_CNT3_STRUC;
#else
typedef union   _TX_AGG_CNT3_STRUC  {
    struct  {
        USHORT  AggSize7Count;
        USHORT  AggSize8Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT3_STRUC, *PTX_AGG_CNT3_STRUC;
#endif
// Debug counter
#define TX_AGG_CNT4 0x1730
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT4_STRUC  {
    struct  {
        USHORT  AggSize10Count;
        USHORT  AggSize9Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT4_STRUC, *PTX_AGG_CNT4_STRUC;
#else
typedef union   _TX_AGG_CNT4_STRUC  {
    struct  {
        USHORT  AggSize9Count;
        USHORT  AggSize10Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT4_STRUC, *PTX_AGG_CNT4_STRUC;
#endif
#define TX_AGG_CNT5 0x1734
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT5_STRUC  {
    struct  {
        USHORT  AggSize12Count;
        USHORT  AggSize11Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT5_STRUC, *PTX_AGG_CNT5_STRUC;
#else
typedef union   _TX_AGG_CNT5_STRUC  {
    struct  {
        USHORT  AggSize11Count;
        USHORT  AggSize12Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT5_STRUC, *PTX_AGG_CNT5_STRUC;
#endif
#define TX_AGG_CNT6     0x1738
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT6_STRUC  {
    struct  {
        USHORT  AggSize14Count;
        USHORT  AggSize13Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT6_STRUC, *PTX_AGG_CNT6_STRUC;
#else
typedef union   _TX_AGG_CNT6_STRUC  {
    struct  {
        USHORT  AggSize13Count;
        USHORT  AggSize14Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT6_STRUC, *PTX_AGG_CNT6_STRUC;
#endif
#define TX_AGG_CNT7     0x173c
#ifdef BIG_ENDIAN
typedef union   _TX_AGG_CNT7_STRUC  {
    struct  {
        USHORT  AggSize164Count;
        USHORT  AggSize15Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT7_STRUC, *PTX_AGG_CNT7_STRUC;
#else
typedef union   _TX_AGG_CNT7_STRUC  {
    struct  {
        USHORT  AggSize15Count;
        USHORT  AggSize16Count;
    }   field;
    ULONG           word;
}   TX_AGG_CNT7_STRUC, *PTX_AGG_CNT7_STRUC;
#endif
#define MPDU_DENSITY_CNT        0x1740
#ifdef BIG_ENDIAN
typedef union   _MPDU_DEN_CNT_STRUC {
    struct  {
        USHORT  RXZeroDelCount; //RX zero length delimiter count
        USHORT  TXZeroDelCount; //TX zero length delimiter count
    }   field;
    ULONG           word;
}   MPDU_DEN_CNT_STRUC, *PMPDU_DEN_CNT_STRUC;
#else
typedef union   _MPDU_DEN_CNT_STRUC {
    struct  {
        USHORT  TXZeroDelCount; //TX zero length delimiter count
        USHORT  RXZeroDelCount; //RX zero length delimiter count
    }   field;
    ULONG           word;
}   MPDU_DEN_CNT_STRUC, *PMPDU_DEN_CNT_STRUC;
#endif
//
// TXRX control registers - base address 0x3000
//
// rt2860b  UNKNOWN reg use R/O Reg Addr 0x77d0 first..
#define TXRX_CSR1           0x77d0

//
// Security key table memory, base address = 0x1000
//
#define MAC_WCID_BASE       0x1800 //8-bytes(use only 6-bytes) * 256 entry = 
#define HW_WCID_ENTRY_SIZE   8

#define HW_KEY_ENTRY_SIZE           0x20
#define HW_IVEIV_ENTRY_SIZE   8
#define HW_WCID_ATTRI_SIZE   4
#define WCID_RESERVED          0x6bfc 
#define HW_SHARED_KEY_MODE_SIZE   4
#define SHAREDKEYTABLE          0
#define PAIRWISEKEYTABLE            1
#define HW_WAPI_PN_ENTRY_SIZE   8
#define BT_ACT_CNT 0x7030

#ifdef BIG_ENDIAN
typedef union   _SHAREDKEY_MODE_STRUC   {
    struct  {
        ULONG       Bss1Key3CipherAlg:4;
        ULONG       Bss1Key2CipherAlg:4;
        ULONG       Bss1Key1CipherAlg:4;
        ULONG       Bss1Key0CipherAlg:4;
        ULONG       Bss0Key3CipherAlg:4;
        ULONG       Bss0Key2CipherAlg:4;
        ULONG       Bss0Key1CipherAlg:4;
        ULONG       Bss0Key0CipherAlg:4;
    }   field;
    ULONG           word;
}   SHAREDKEY_MODE_STRUC, *PSHAREDKEY_MODE_STRUC;
#else
typedef union   _SHAREDKEY_MODE_STRUC   {
    struct  {
        ULONG       Bss0Key0CipherAlg:4;
        ULONG       Bss0Key1CipherAlg:4;
        ULONG       Bss0Key2CipherAlg:4;
        ULONG       Bss0Key3CipherAlg:4;
        ULONG       Bss1Key0CipherAlg:4;
        ULONG       Bss1Key1CipherAlg:4;
        ULONG       Bss1Key2CipherAlg:4;
        ULONG       Bss1Key3CipherAlg:4;
    }   field;
    ULONG           word;
}   SHAREDKEY_MODE_STRUC, *PSHAREDKEY_MODE_STRUC;
#endif
// 64-entry for pairwise key table
typedef struct _HW_WCID_ENTRY {  // 8-byte per entry
    UCHAR   Address[6];
    UCHAR   Rsv[2];
} HW_WCID_ENTRY, PHW_WCID_ENTRY;


// ================================================================
// Tx / Rx / Mgmt ring descriptor definition
// ================================================================

// the following PID values are used to mark outgoing frame type in TXD->PID so that
// proper TX statistics can be collected based on these categories
// b3-2 of PID field -
#define PID_MGMT            0x05
#define PID_BEACON            0xc0 //0x0c
#define PID_DATA_NORMALUCAST        0x02
#define PID_DATA_AMPDU      0x04
#define PID_DATA_NO_ACK     0x06
#define PID_DATA_NOT_NORM_ACK       0x03
#define PID_CTS_TO_SELF     0x08
#define PID_REQUIRE_ACK     0x7
#define PID_BTHS            0x0a

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
#define PID_P2pMs_INDICATE      0x0d
#endif

//
// Other on-chip shared memory space, base = 0x2000
//

// CIS space - base address = 0x2000
#define HW_CIS_BASE             0x2000

// on-chip NULL frame sapace - base address = 0x7700
// here stores QOS-NULL frame . 
//PBF_CFG(0x408) bit15 use Qos-NULL to enable STA mode early terminate txop.
#define HW_NULL_BASE            0x7700
// 2nd NULL FRAME put CF-END frame. 
//PBF_CFG(0x408) bit14 to enable AP mode early terminate txop requires CF-END
#define HW_NULL2_BASE            0x7780


//
// Higher 8KB shared memory
//
#define HW_BEACON_BASE0_REDIRECTION 0x4000
#define HW_BEACON_BASE1_REDIRECTION 0x4200
#define HW_BEACON_BASE2_REDIRECTION 0x4400
#define HW_BEACON_BASE3_REDIRECTION 0x4600
#define HW_BEACON_BASE4_REDIRECTION 0x4800
#define HW_BEACON_BASE5_REDIRECTION 0x4A00
#define HW_BEACON_BASE6_REDIRECTION 0x4C00
#define HW_BEACON_BASE7_REDIRECTION 0x4E00

// HOST-MCU shared memory - base address = 0x2100
#define HOST_CMD_CSR        0x404
//#define HOST_CMD_CSR      ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x234) : (0x404))

#define H2M_MAILBOX_CSR         0x7010
#define H2M_MAILBOX_CID         0x7014
#define H2M_INT_SRC             0x7024
#define H2M_MAILBOX_STATUS      0x701c
#define M2H_CMD_DONE_CSR        0x000c
#define H2M_BBP_AGENT           0x7028
#define CARRIER_R66_VALUE       0x702c      // firmware will choose a value according a/g band
#define MCU_TXOP_ARRAY_BASE     0x000c   // TODO: to be provided by Albert
#define MCU_TXOP_ENTRY_SIZE     32       // TODO: to be provided by Albert
#define MAX_NUM_OF_TXOP_ENTRY   16       // TODO: must be same with 8051 firmware
#define MCU_MBOX_VERSION        0x01     // TODO: to be confirmed by Albert
#define MCU_MBOX_VERSION_OFFSET 5        // TODO: to be provided by Albert

//Power saving 
#define PowerSafeCID        1
#define PowerRadioOffCID    2
#define PowerWakeCID        3
#define BBPCID              4
#define CID0MASK        0x000000ff
#define CID1MASK        0x0000ff00
#define CID2MASK        0x00ff0000
#define CID3MASK        0xff000000

//
// Host DMA registers - base address 0x200 .  TX0-3=EDCAQid0-3, TX4=HCCA, TX5=MGMT,
//
//
//  DMA RING DESCRIPTOR
//
#define E2PROM_CSR          0x0004
#define IO_CNTL_CSR         0x77d0

// 8051 firmware image for usb - use last-half base address = 0x3000
//#define FIRMWARE_IMAGE_BASE     0x3000
//#define MAX_FIRMWARE_IMAGE_SIZE 0x2000    // 8kbyte
#define DEFAULT_FIRMWARE_SIZE   0x1000    // 4kbtye

// TODO: ????? old RT2560 registers. to keep them or remove them?
//#define MCAST0                  0x0178  // multicast filter register 0
//#define MCAST1                  0x017c  // multicast filter register 1


// ================================================================
// Tx / Rx / Mgmt ring descriptor definition
// ================================================================



// value domain of pTxD->HostQId (4-bit: 0~15)
#define QID_AC_BK               1   // meet ACI definition in 802.11e
#define QID_AC_BE               0   // meet ACI definition in 802.11e
#define QID_AC_VI               2
#define QID_AC_VO               3
#define QID_HCCA                4
//#define NUM_OF_TX_RING          5
#define QID_MGMT                13
#define QID_RX                  14
#define QID_OTHER               15


// ------------------------------------------------------
// BBP & RF definition
// ------------------------------------------------------
#define BUSY        1
#define IDLE        0

#define BBP_R0                      0  // version
#define BBP_R1                      1  // TSSI

//
// The bit mask for getting the static transmit power control in the BBP R1
//
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK    0x03

#define BBP_R2                      2  // TX configure
#define BBP_R3                      3
#define BBP_R4                      4
#define BBP_R5                      5
#define BBP_R6                      6
#define BBP_R14                     14 // RX configure
#define BBP_R16                     16
#define BBP_R17                     17 // RX sensibility
#define BBP_R18                     18
#define BBP_R20                     20
#define BBP_R21                     21
#define BBP_R22                     22
#define BBP_R23                     23
#define BBP_R24                     24
#define BBP_R25                     25
#define BBP_R26                     26
#define BBP_R27                     27
#define BBP_R31                     31
#define BBP_R47                     47 // ADC6/TSSI control
#define BBP_R49                     49 //TSSI
#define BBP_R55                     55
#define BBP_R57                     57 // Rx OFDM/CCK frequency offset report
#define BBP_R58                     58
#define BBP_R60                     60
#define BBP_R62                     62 // Rx SQ0 Threshold HIGH
#define BBP_R63                     63
#define BBP_R64                     64
#define BBP_R65                     65
#define BBP_R66                     66
#define BBP_R67                     67
#define BBP_R68                     68
#define BBP_R69                     69
#define BBP_R70                     70 // Japan filter
#define BBP_R73                     73
#define BBP_R74                     74
#define BBP_R75                     75
#define BBP_R76                     76
#define BBP_R77                     77
#define BBP_R79                     79
#define BBP_R80                     80
#define BBP_R81                     81
#define BBP_R82                     82
#define BBP_R83                     83
#define BBP_R84                     84
#define BBP_R85                     85
#define BBP_R86                     86
#define BBP_R88                     88
#define BBP_R91                     91
#define BBP_R92                     92
#define BBP_R94                     94 // Tx Gain Control
#define BBP_R95                     95
#define BBP_R98                     98
#define BBP_R99                     99
#define BBP_R101                    101
#define BBP_R103                    103
#define BBP_R104                    104
#define BBP_R105                    105
#define BBP_R106                    106
#define BBP_R109                    109
#define BBP_R110                    110
#define BBP_R120                    120
#define BBP_R128                    128
#define BBP_R129                    129
#define BBP_R130                    130
#define BBP_R131                    131
#define BBP_R132                    132
#define BBP_R133                    133
#define BBP_R134                    134
#define BBP_R135                    135
#define BBP_R136                    136
#define BBP_R137                    137
#define BBP_R138                    138
#define BBP_R139                    139
#define BBP_R140                    140
#define BBP_R141                    141
#define BBP_R142                    142 // Inband configuration index register
#define BBP_R143                    143 // Inband configuration data register
#define BBP_R144                    144 // 802.11k noise histogram measurement (NHM) control
#define BBP_R145                    145 // 802.11k noise histogram measurement (NHM) IPI value
#define BBP_R146                    146
#define BBP_R147                    147
#define BBP_R148                    148
#define BBP_R149                    149
#define BBP_R150                    150
#define BBP_R151                    151
#define BBP_R152                    152
#define BBP_R153                    153
#define BBP_R154                    154
#define BBP_R155                    155
#define BBP_R156                    156
#define BBP_R157                    157
#define BBP_R158                    158
#define BBP_R159                    159
#define BBP_R160                    160
#define BBP_R161                    161
#define BBP_R162                    162
#define BBP_R163                    163
#define BBP_R164                    164
#define BBP_R165                    165
#define BBP_R166                    166
#define BBP_R167                    167
#define BBP_R168                    168
#define BBP_R169                    169
#define BBP_R170                    170
#define BBP_R171                    171
#define BBP_R172                    172
#define BBP_R173                    173
#define BBP_R174                    174
#define BBP_R175                    175
#define BBP_R176                    176
#define BBP_R177                    177
#define BBP_R178                    178
#define BBP_R179                    179 // Test config #1
#define BBP_R180                    180 // Test config #2
#define BBP_R181                    181
#define BBP_R182                    182
#define BBP_R183                    183
#define BBP_R184                    184
#define BBP_R185                    185
#define BBP_R186                    186
#define BBP_R187                    187
#define BBP_R188                    188
#define BBP_R189                    189
#define BBP_R190                    190
#define BBP_R191                    191
#define BBP_R192                    192
#define BBP_R193                    193
#define BBP_R194                    194
#define BBP_R195                    195
#define BBP_R196                    196
#define BBP_R197                    197
#define BBP_R198                    198
#define BBP_R199                    199
#define BBP_R200                    200
#define BBP_R201                    201
#define BBP_R202                    202
#define BBP_R203                    203
#define BBP_R204                    204
#define BBP_R205                    205
#define BBP_R206                    206
#define BBP_R207                    207
#define BBP_R208                    208
#define BBP_R209                    209
#define BBP_R210                    210
#define BBP_R211                    211
#define BBP_R241                    241
#define BBP_R244                    244
#define BBP_R253                    253
#define BBP_R254                    254
#define MAX_NUM_OF_BBP_LATCH        255

#define BBPR94_DEFAULT              0x06 // Add 1 value will gain 1db

#define BBP_REG_BF      BBP_R163 // TxBf control
#define BBP_REG_SNR0        BBP_R160 // TxBf SNR report (stream 0)
#define BBP_REG_SNR1        BBP_R161 // TxBf SNR report (stream 1)
#define BBP_REG_SNR2        BBP_R162 // TxBf SNR report (stream 2)

#define RSSI_FOR_VERY_LOW_SENSIBILITY -35
#define RSSI_FOR_LOW_SENSIBILITY    -58
#define RSSI_FOR_MID_LOW_SENSIBILITY  -65
#define RSSI_FOR_MID_SENSIBILITY    -90

//
// BBP R49 TSSI (Transmit Signal Strength Indicator)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R49_STRUC {
    struct
    {
        UCHAR   adc5_in_sel:1; // 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390)
        UCHAR   bypassTSSIAverage:1; // 0: the average TSSI (the average of the 16 samples), 1: the current TSSI
        UCHAR   Reserved:1; // Reserved field
        UCHAR   TSSI:5; // TSSI value
    } field;

    UCHAR       byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#else
typedef union _BBP_R49_STRUC {
    struct
    {
        UCHAR   TSSI:5; // TSSI value
        UCHAR   Reserved:1; // Reserved field
        UCHAR   bypassTSSIAverage:1; // 0: the average TSSI (the average of the 16 samples), 1: the current TSSI
        UCHAR   adc5_in_sel:1; // 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390)
    } field;
    
    UCHAR       byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#endif

//
// BBP R105 (FEQ control, MLD control and SIG remodulation)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R105_STRUC {
    struct
    {
        UCHAR   Reserve1:1; // Reserved field
        UCHAR   ADAPTIVE_FEQ_STEP:2; // The step size for the adaptive FEQ
        UCHAR   ADAPTIVE_FEQ_ENABLE:1; // Enable/disable the adaptive FEQ
        UCHAR   EnableSIGRemodulation:1; // Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols.
        UCHAR   MLDFor2Stream:1; // Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)
        UCHAR   IndependentFeedForwardCompensation:1; // Apply independent feed-forward compensation for independent stream.
        UCHAR   DetectSIGOnPrimaryChannelOnly:1; // Under 40 MHz band, detect SIG on primary channel only.
    } field;

    UCHAR       byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#else
typedef union _BBP_R105_STRUC {
    struct
    {
        UCHAR   DetectSIGOnPrimaryChannelOnly:1; // Under 40 MHz band, detect SIG on primary channel only.
        UCHAR   IndependentFeedForwardCompensation:1; // Apply independent feed-forward compensation for independent stream.
        UCHAR   MLDFor2Stream:1; // Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)
        UCHAR   EnableSIGRemodulation:1; // Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols.
        UCHAR   ADAPTIVE_FEQ_ENABLE:1; // Enable/disable the adaptive FEQ
        UCHAR   ADAPTIVE_FEQ_STEP:2; // The step size for the adaptive FEQ
        UCHAR   Reserve1:1; // Reserved field
    } field;
    
    UCHAR       byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#endif

//
// BBP R106 (GI remover)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R106_STRUC {
    struct
    {
        UCHAR   EnableLowPowerFSD:1; // enable/disable the low power FSD
        UCHAR   ShortGI_Offset40:4; // Delay GI remover when the short GI is detected in 40MHz band (40M sampling rate)
        UCHAR   ShortGI_Offset20:3; // Delay GI remover when the short GI is detected in 20MHz band (20M sampling rate)
    } field;

    UCHAR       byte;
} BBP_R106_STRUC, *PBBP_R106_STRUC;
#else
typedef union _BBP_R106_STRUC {
    struct
    {
        UCHAR   ShortGI_Offset20:3; // Delay GI remover when the short GI is detected in 20MHz band (20M sampling rate)
        UCHAR   ShortGI_Offset40:4; // Delay GI remover when the short GI is detected in 40MHz band (40M sampling rate)
        UCHAR   EnableLowPowerFSD:1; // enable/disable the low power FSD
    } field;
    
    UCHAR       byte;
} BBP_R106_STRUC, *PBBP_R106_STRUC;
#endif

//
// BBP R109 (Tx power control in 0.1dB step)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R109_STRUC
{
    struct
    {
        UCHAR   Tx1PowerCtrl:4; // Tx1 power control in 0.1dB step (valid: 0~10)
        UCHAR   Tx0PowerCtrl:4; // Tx0 power control in 0.1dB step (valid: 0~10)
    } field;

    UCHAR       byte;
} BBP_R109_STRUC, *PBBP_R109_STRUC;
#else
typedef union _BBP_R109_STRUC
{
    struct
    {
        UCHAR   Tx0PowerCtrl:4; // Tx0 power control in 0.1dB step (valid: 0~10)
        UCHAR   Tx1PowerCtrl:4; // Tx0 power control in 0.1dB step (valid: 0~10)
    } field;
    
    UCHAR       byte;
} BBP_R109_STRUC, *PBBP_R109_STRUC;
#endif

//
// BBP R110 (Tx power control in 0.1dB step)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R110_STRUC
{
    struct
    {
        UCHAR   Tx2PowerCtrl:4; // Tx2 power control in 0.1dB step (valid: 0~10)
        UCHAR   AllTxPowerCtrl:4; // All transmitters' fine power control in 0.1dB (valid: 0~10)
    } field;

    UCHAR       byte;
} BBP_R110_STRUC, *PBBP_R110_STRUC;
#else
typedef union _BBP_R110_STRUC
{
    struct
    {
        UCHAR   AllTxPowerCtrl:4; // All transmitters' fine power control in 0.1dB (valid: 0~10)
        UCHAR   Tx2PowerCtrl:4; // Tx2 power control in 0.1dB step (valid: 0~10)
    } field;
    
    UCHAR       byte;
} BBP_R110_STRUC, *PBBP_R110_STRUC;
#endif

//
// BBP R144 (802.11k noise histogram measurement control)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R144_STRUC {
    struct
    {
        UCHAR   NHMExternGain:6; // Extern LNA + channel filter loss + antenna switch loss, -31 dB ~ +31 dB
        UCHAR   NHMPeriod:2; // Noise histogram measurement period (0: 1us, 1: 2us, 2: 3us, 3: 4us)
    } field;

    UCHAR       byte;
} BBP_R144_STRUC, *PBBP_R144_STRUC;
#else
typedef union _BBP_R144_STRUC {
    struct
    {
        UCHAR   NHMPeriod:2; // Noise histogram measurement period (0: 1us, 1: 2us, 2: 3us, 3: 4us)
        UCHAR   NHMExternGain:6; // Extern LNA + channel filter loss + antenna switch loss, -31 dB ~ +31 dB
    } field;
    
    UCHAR       byte;
} BBP_R144_STRUC, *PBBP_R144_STRUC;
#endif

//
// BBP R179 (Test config #1)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R179_STRUC {
    struct
    {
        UCHAR   DataIndex1:8; // Data index #1
    } field;

    UCHAR       byte;
} BBP_R179_STRUC, *PBBP_R179_STRUC;
#else
typedef union _BBP_R179_STRUC {
    struct
    {
        UCHAR   DataIndex1:8; // Data index #1
    } field;
    
    UCHAR       byte;
} BBP_R179_STRUC, *PBBP_R179_STRUC;
#endif

//
// BBP R180 (Test config #2)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R180_STRUC {
    struct
    {
        UCHAR   DataIndex2:8; // Data index #2
    } field;

    UCHAR       byte;
} BBP_R180_STRUC, *PBBP_R180_STRUC;
#else
typedef union _BBP_R180_STRUC {
    struct
    {
        UCHAR   DataIndex2:8; // Data index #2
    } field;
    
    UCHAR       byte;
} BBP_R180_STRUC, *PBBP_R180_STRUC;
#endif

//
// BBP R182 (Test data port)
//
#ifdef BIG_ENDIAN
typedef union _BBP_R182_STRUC {
    struct
    {
        UCHAR   DataArray:8; // Data array indexed by BBP R179 and R180
    } field;

    UCHAR       byte;
} BBP_R182_STRUC, *PBBP_R182_STRUC;
#else
typedef union _BBP_R182_STRUC {
    struct
    {
        UCHAR   DataArray:8; // Data array indexed by BBP R179 and R180
    } field;
    
    UCHAR       byte;
} BBP_R182_STRUC, *PBBP_R182_STRUC;
#endif

// =================================================================================
// WCID  format
// =================================================================================
//7.1   WCID  ENTRY  format  : 8bytes
#ifdef BIG_ENDIAN
typedef struct  _WCID_ENTRY_STRUC {
}   WCID_ENTRY_STRUC, *PWCID_ENTRY_STRUC;
#else
typedef struct  _WCID_ENTRY_STRUC {
    UCHAR       RXBABitmap7;    // bit0 for TID8, bit7 for TID 15
    UCHAR       RXBABitmap0;    // bit0 for TID0, bit7 for TID 7
    UCHAR       MAC[6]; // 0 for shared key table.  1 for pairwise key table
}   WCID_ENTRY_STRUC, *PWCID_ENTRY_STRUC;
#endif
//8.1.1 SECURITY  KEY  format  : 8DW
// 32-byte per entry, total 16-entry for shared key table, 64-entry for pairwise key table
typedef struct _HW_KEY_ENTRY {          // 32-byte per entry
    UCHAR   Key[16];
    UCHAR   TxMic[8];
    UCHAR   RxMic[8];
} HW_KEY_ENTRY, *PHW_KEY_ENTRY; 

//8.1.2 IV/EIV  format  : 2DW

//8.1.3 RX attribute entry format  : 1DW
#ifdef BIG_ENDIAN
typedef struct  _MAC_ATTRIBUTE_STRUC {
}   MAC_ATTRIBUTE_STRUC, *PMAC_ATTRIBUTE_STRUC;
#else
typedef struct  _MAC_ATTRIBUTE_STRUC {
    ULONG       KeyTab:1;   // 0 for shared key table.  1 for pairwise key table
    ULONG       PairKeyMode:3;
    ULONG       BSSIDIdx:3; //multipleBSS index for the WCID
    ULONG       RXWIUDF:3;
    ULONG       rsv:22;
}   MAC_ATTRIBUTE_STRUC, *PMAC_ATTRIBUTE_STRUC;
#endif


// =================================================================================
// TX / RX ring descriptor format
// =================================================================================

// the first 24-byte in TXD is called TXINFO and will be DMAed to MAC block through TXFIFO.
// MAC block use this TXINFO to control the transmission behavior of this frame.
#define TXINFO_SIZE                 4
#define FIFO_MGMT                 0
#define FIFO_HCCA                 1
#define FIFO_EDCA                 2


//
// TXINFO : SwUsepad & SwUseAMSDU are for 802.11n AMSDU. 
//
/*
#ifdef BIG_ENDIAN
typedef struct  _TXINFO_STRUC {
}   TXINFO_STRUC, *PTXINFO_STRUC;
#else
typedef struct  _TXINFO_STRUC {
    // Word 0
    ULONG       USBDMATxPktLen:16;  //used ONLY in USB bulk Aggregation,  Total byte counts of all sub-frame.   
    ULONG       SwUseSegIdx:6;
    ULONG       SwUsepad:2;
    ULONG       WIV:1;  // Wireless Info Valid. 1 if Driver already fill WI,  o if DMA needs to copy WI to correctposition
    ULONG       QSEL:2; // select on-chip FIFO ID for 2nd-stage output scheduler.0:MGMT, 1:HCCA 2:EDCA
    ULONG       SwUseSegmentEnd:1; // Software use to indicate this is last packet, Next packet need to be rounded.
    ULONG       SwUseAMSDU:1; // Software use only.  This RA is under AMSUD policy. So make AMSDU at BulkOutData
    ULONG       SwUseNonQoS:1;  // Software use only.  Indicate this is DHCP or EAPOL frame
    ULONG       USBDMANextVLD:1;    //used ONLY in USB bulk Aggregation, NextValid  
    ULONG       USBDMATxburst:1;//used ONLY in USB bulk Aggre. Force USB DMA transmit frame from current selected endpoint
}   TXINFO_STRUC, *PTXINFO_STRUC;
#endif
*/

// =================================================================================
// HOST-MCU communication data structure
// =================================================================================

//
// H2M_MAILBOX_CSR: Host-to-MCU Mailbox
//
#ifdef BIG_ENDIAN
typedef union  _H2M_MAILBOX_STRUC {
    struct {
        ULONG       Owner:8;
        ULONG       CmdToken:8;    // 0xff tells MCU not to report CmdDoneInt after excuting the command
        ULONG       HighByte:8;
        ULONG       LowByte:8;
    }   field;
    ULONG           word;
} H2M_MAILBOX_STRUC, *PH2M_MAILBOX_STRUC;
#else
typedef union  _H2M_MAILBOX_STRUC {
    struct {
        ULONG       LowByte:8;
        ULONG       HighByte:8;
        ULONG       CmdToken:8;
        ULONG       Owner:8;
    }   field;
    ULONG           word;
} H2M_MAILBOX_STRUC, *PH2M_MAILBOX_STRUC;
#endif

//
// M2H_CMD_DONE_CSR: MCU-to-Host command complete indication
//
#ifdef BIG_ENDIAN
typedef union _M2H_CMD_DONE_STRUC {
    struct  {
        ULONG       CmdToken3;
        ULONG       CmdToken2;
        ULONG       CmdToken1;
        ULONG       CmdToken0;
    } field;
    ULONG           word;
} M2H_CMD_DONE_STRUC, *PM2H_CMD_DONE_STRUC;
#else
typedef union _M2H_CMD_DONE_STRUC {
    struct  {
        ULONG       CmdToken0;
        ULONG       CmdToken1;
        ULONG       CmdToken2;
        ULONG       CmdToken3;
    } field;
    ULONG           word;
} M2H_CMD_DONE_STRUC, *PM2H_CMD_DONE_STRUC;
#endif



//
// MCU_LEDCS: MCU LED Control Setting.
//
#ifdef BIG_ENDIAN
typedef union  _MCU_LEDCS_STRUC {
    struct  {
        UCHAR       Polarity:1;
        UCHAR       LedMode:7;      
    } field;
    UCHAR               word;
} MCU_LEDCS_STRUC, *PMCU_LEDCS_STRUC;
#else
typedef union  _MCU_LEDCS_STRUC {
    struct  {
        UCHAR       LedMode:7;      
        UCHAR       Polarity:1;
    } field;
    UCHAR           word;
} MCU_LEDCS_STRUC, *PMCU_LEDCS_STRUC;
#endif
// =================================================================================
// Register format
// =================================================================================



//
// PHY_CSR3: BBP serial control register
//
#ifdef BIG_ENDIAN
typedef union   _PHY_CSR3_STRUC {
    struct  {
        ULONG       :15;        
        ULONG       Busy:1;             // 1: ASIC is busy execute BBP programming. 
        ULONG       fRead:1;            // 0: Write BBP, 1: Read BBP
        ULONG       RegNum:7;           // Selected BBP register
        ULONG       Value:8;            // Register value to program into BBP
    }   field;
    ULONG           word;
}   PHY_CSR3_STRUC, *PPHY_CSR3_STRUC;
#else
typedef union   _PHY_CSR3_STRUC {
    struct  {
        ULONG       Value:8;            // Register value to program into BBP
        ULONG       RegNum:7;           // Selected BBP register
        ULONG       fRead:1;            // 0: Write BBP, 1: Read BBP
        ULONG       Busy:1;             // 1: ASIC is busy execute BBP programming. 
        ULONG       :15;
    }   field;
    ULONG           word;
}   PHY_CSR3_STRUC, *PPHY_CSR3_STRUC;
#endif

//
// PHY_CSR4: RF serial control register
//
#ifdef BIG_ENDIAN
typedef union   _PHY_CSR4_STRUC {
    struct  {
        ULONG       Busy:1;             // 1: ASIC is busy execute RF programming.      
        ULONG       PLL_LD:1;           // RF PLL_LD status
        ULONG       IFSelect:1;         // 1: select IF to program, 0: select RF to program
        ULONG       NumberOfBits:5;     // Number of bits used in RFRegValue (I:20, RFMD:22)
        ULONG       RFRegValue:24;      // Register value (include register id) serial out to RF/IF chip.
    }   field;
    ULONG           word;
}   PHY_CSR4_STRUC, *PPHY_CSR4_STRUC;
#else
typedef union   _PHY_CSR4_STRUC {
    struct  {
        ULONG       RFRegValue:24;      // Register value (include register id) serial out to RF/IF chip.
        ULONG       NumberOfBits:5;     // Number of bits used in RFRegValue (I:20, RFMD:22)
        ULONG       IFSelect:1;         // 1: select IF to program, 0: select RF to program
        ULONG       PLL_LD:1;           // RF PLL_LD status
        ULONG       Busy:1;             // 1: ASIC is busy execute RF programming.
    }   field;
    ULONG           word;
}   PHY_CSR4_STRUC, *PPHY_CSR4_STRUC;
#endif


//
// SEC_CSR5: shared key table security mode register
//
#ifdef BIG_ENDIAN
typedef union   _SEC_CSR5_STRUC {
    struct  {
        ULONG       :1;
        ULONG       Bss3Key3CipherAlg:3;
        ULONG       :1;
        ULONG       Bss3Key2CipherAlg:3;
        ULONG       :1;
        ULONG       Bss3Key1CipherAlg:3;
        ULONG       :1;
        ULONG       Bss3Key0CipherAlg:3;
        ULONG       :1;
        ULONG       Bss2Key3CipherAlg:3;
        ULONG       :1;
        ULONG       Bss2Key2CipherAlg:3;
        ULONG       :1;
        ULONG       Bss2Key1CipherAlg:3;
        ULONG       :1;
        ULONG       Bss2Key0CipherAlg:3;
    }   field;
    ULONG           word;
}   SEC_CSR5_STRUC, *PSEC_CSR5_STRUC;
#else
typedef union   _SEC_CSR5_STRUC {
    struct  {
        ULONG       Bss2Key0CipherAlg:3;
        ULONG       :1;
        ULONG       Bss2Key1CipherAlg:3;
        ULONG       :1;
        ULONG       Bss2Key2CipherAlg:3;
        ULONG       :1;
        ULONG       Bss2Key3CipherAlg:3;
        ULONG       :1;
        ULONG       Bss3Key0CipherAlg:3;
        ULONG       :1;
        ULONG       Bss3Key1CipherAlg:3;
        ULONG       :1;
        ULONG       Bss3Key2CipherAlg:3;
        ULONG       :1;
        ULONG       Bss3Key3CipherAlg:3;
        ULONG       :1;
    }   field;
    ULONG           word;
}   SEC_CSR5_STRUC, *PSEC_CSR5_STRUC;
#endif


//
// HOST_CMD_CSR: For HOST to interrupt embedded processor
//
#ifdef BIG_ENDIAN
typedef union   _HOST_CMD_CSR_STRUC {
    struct  {
        ULONG   Rsv:24;
        ULONG   HostCommand:8;
    }   field;
    ULONG           word;
}   HOST_CMD_CSR_STRUC, *PHOST_CMD_CSR_STRUC;
#else
typedef union   _HOST_CMD_CSR_STRUC {
    struct  {
        ULONG   HostCommand:8;
        ULONG   Rsv:24;
    }   field;
    ULONG           word;
}   HOST_CMD_CSR_STRUC, *PHOST_CMD_CSR_STRUC;
#endif

//
// AIFSN_CSR: AIFSN for each EDCA AC
//



//
// E2PROM_CSR: EEPROM control register
//
#ifdef BIG_ENDIAN
typedef union   _E2PROM_CSR_STRUC   {
    struct  {
        ULONG       Rsvd:25;
        ULONG       LoadStatus:1;   // 1:loading, 0:done
        ULONG       Type:1;         // 1: 93C46, 0:93C66
        ULONG       EepromDO:1;
        ULONG       EepromDI:1;
        ULONG       EepromCS:1;
        ULONG       EepromSK:1;
        ULONG       Reload:1;       // Reload EEPROM content, write one to reload, self-cleared.
    }   field;
    ULONG           word;
}   E2PROM_CSR_STRUC, *PE2PROM_CSR_STRUC;
#else
typedef union   _E2PROM_CSR_STRUC   {
    struct  {
        ULONG       Reload:1;       // Reload EEPROM content, write one to reload, self-cleared.
        ULONG       EepromSK:1;
        ULONG       EepromCS:1;
        ULONG       EepromDI:1;
        ULONG       EepromDO:1;
        ULONG       Type:1;         // 1: 93C46, 0:93C66
        ULONG       LoadStatus:1;   // 1:loading, 0:done
        ULONG       Rsvd:25;
    }   field;
    ULONG           word;
}   E2PROM_CSR_STRUC, *PE2PROM_CSR_STRUC;
#endif


// -------------------------------------------------------------------
//  E2PROM data layout
// -------------------------------------------------------------------

//
// RSSI indication mode
//
#define NORMAL_MODE             0 // Normal mode (RSSI indication by Rx LNA input)
#define USER_AWARENESS_MODE 1 // User awareness mode (RSSI indication by Rx LNA output)

//
// Board type
//
#define BOARD_TYPE_MINI_CARD    0 // Mini card
#define BOARD_TYPE_USB_PEN      1 // USB pen

//
// EEPROM antenna select format
//
#ifdef BIG_ENDIAN
typedef union   _EEPROM_ANTENNA_STRUC   {
    struct  {
        USHORT RssiIndicationMode:1; // RSSI indication mode
        USHORT      Rsv:1;
        USHORT BoardType:2; // 0: mini card; 1: USB pen
        USHORT      RfIcType:4;             // see E2PROM document      
        USHORT      TxPath:4;   // 1: Hardware controlled radio enabled, Read GPIO0 required.
        USHORT      RxPath:4;   // 1: Hardware controlled radio enabled, Read GPIO0 required.
    }   field;
    USHORT          word;
}   EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#else
typedef union   _EEPROM_ANTENNA_STRUC   {
    struct  {
        USHORT      RxPath:4;   // 1: Hardware controlled radio enabled, Read GPIO0 required.
        USHORT      TxPath:4;   // 1: Hardware controlled radio enabled, Read GPIO0 required.
        USHORT      RfIcType:4;             // see E2PROM document      
        USHORT BoardType:2; // 0: mini card; 1: USB pen
        USHORT      Rsv:1;
        USHORT RssiIndicationMode:1; // RSSI indication mode
    }   field;
    USHORT          word;
}   EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#endif

//
// The configurations of antenna diversity
//
#define ANT_DIV_CONFIG_DISABLE  0 // disable antenna diversity
#define ANT_DIV_CONFIG_ENABLE   1 // enable antenna diversity
#define ANT_DIV_CONFIG_MAIN_ANT 2 // use main antenna
#define ANT_DIV_CONFIG_AUX_ANT  3 // use aux. antenna

#ifdef BIG_ENDIAN
typedef union   _EEPROM_NIC_CINFIG2_STRUC   {
    struct  {
        USHORT      DACTestBit:1;               // control if driver should patch the DAC issue
        USHORT      BTCoexist:1;                    // must be 0
        USHORT      bInternalTxALC:1; // Internal Tx ALC
        USHORT      AntDiv:2; // antenna diversity (ANT_DIV_CONFIG_XXX) (b'00: disable antenna diversity, b'01: enable antenna diversity, b'10: use main antenna, b'11: use aux. antenna)
        USHORT      Rsv1:1;                 // must be 0
        USHORT      bDisableBW40ForA:1;     // 0:enable, 1:disable
        USHORT      bDisableBW40ForG:1;     // 0:enable, 1:disable
        USHORT      EnableWPSPBC:1;                     // 0:enable, 1:disable for WPSPBC
        USHORT      BW40MSidebandForA:1;
        USHORT      BW40MSidebandForG:1;
        USHORT      CardbusAcceleration:1;  // !!! NOTE: 0 - enable, 1 - disable        
        USHORT      ExternalLNAForA:1;          // external LNA enable for 5G       
        USHORT      ExternalLNAForG:1;          // external LNA enable for 2.4G
        USHORT      DynamicTxAgcControl:1; // External Tx ALC
        USHORT      HardwareRadioControl:1; // Whether RF is controlled by driver or HW. 1:enable hw control, 0:disable
    }   field;
    USHORT          word;
}   EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#else
typedef union   _EEPROM_NIC_CINFIG2_STRUC   {
    struct  {
        USHORT      HardwareRadioControl:1; // 1:enable, 0:disable
        USHORT      DynamicTxAgcControl:1; // External Tx ALC
        USHORT      ExternalLNAForG:1;              //
        USHORT      ExternalLNAForA:1;          // external LNA enable for 2.4G
        USHORT      CardbusAcceleration:1;  // !!! NOTE: 0 - enable, 1 - disable        
        USHORT      BW40MSidebandForG:1;
        USHORT      BW40MSidebandForA:1;
        USHORT      EnableWPSPBC:1;                     // 0:enable, 1:disable for WPSPBC

        USHORT      bDisableBW40ForG:1;     // 0: BW40 allowed , 1: BW40 denied
        USHORT      bDisableBW40ForA:1;     // 0: BW40 allowed , 1: BW40 denied
        USHORT      ExtLNA:1;                   // must be 0
        USHORT      AntDiv:2; // antenna diversity (ANT_DIV_CONFIG_XXX) (b'00: disable antenna diversity, b'01: enable antenna diversity, b'10: use main antenna, b'11: use aux. antenna)
        USHORT      bInternalTxALC:1; // Internal Tx ALC
        USHORT      BTCoexist:1;                    // must be 0
        USHORT      DACTestBit:1;               // control if driver should patch the DAC issue
    }   field;
    USHORT          word;
}   EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union   _EEPROM_NIC_CINFIG3_STRUC   {
    struct  {
        USHORT      Rsv1:4;             // must be 0
        USHORT      RxTempComp:1;       // Rx Temperature Compensation
        USHORT      Rsv2:2;             // must be 0
        USHORT      BTCoexistenceMethod:1;  // 0=>2 wire, 1=>3 wire
        USHORT      TxStream:4;         // Number of Tx stream
        USHORT      RxStream:4;         // Number of rx stream
    }   field;
    USHORT          word;
}   EEPROM_NIC_CONFIG3_STRUC, *PEEPROM_NIC_CONFIG3_STRUC;
#else
typedef union   _EEPROM_NIC_CINFIG3_STRUC   {
    struct  {
        USHORT      RxStream:4;         // Number of rx stream
        USHORT      TxStream:4;         // Number of Tx stream
        USHORT      BTCoexistenceMethod:1;  // 0=>2 wire, 1=>3 wire
        USHORT      Rsv2:2;             // must be 0
        USHORT      RxTempComp:1;       // Rx Temperature Compensation
        USHORT      Rsv1:4;             // must be 0
    }   field;
    USHORT          word;
}   EEPROM_NIC_CONFIG3_STRUC, *PEEPROM_NIC_CONFIG3_STRUC;
#endif

//
// EEPROM Tx power offset for the extended TSSI mode
//
#ifdef BIG_ENDIAN
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
    struct
    {
        UCHAR   Byte1;  // High Byte
        UCHAR   Byte0;  // Low Byte
    } field;
    
    USHORT      word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#else
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
    struct
    {
        UCHAR   Byte0;  // Low Byte
        UCHAR   Byte1;  // High Byte
    } field;
    
    USHORT      word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union   _EEPROM_TX_PWR_STRUC    {
    struct  {
        CHAR    Byte1;              // High Byte
        CHAR    Byte0;              // Low Byte
    }   field;
    USHORT  word;
}   EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#else
typedef union   _EEPROM_TX_PWR_STRUC    {
    struct  {
        CHAR    Byte0;              // Low Byte
        CHAR    Byte1;              // High Byte
    }   field;
    USHORT  word;
}   EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union   _EEPROM_VERSION_STRUC   {
    struct  {
        UCHAR   Version;            // High Byte
        UCHAR   FaeReleaseNumber;   // Low Byte
    }   field;
    USHORT  word;
}   EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#else
typedef union   _EEPROM_VERSION_STRUC   {
    struct  {
        UCHAR   FaeReleaseNumber;   // Low Byte
        UCHAR   Version;            // High Byte
    }   field;
    USHORT  word;
}   EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union   _EEPROM_LED_STRUC   {
    struct  {
        USHORT  Rsvd:3;             // Reserved
        USHORT  LedMode:5;          // Led mode.
        USHORT  PolarityGPIO_4:1;   // Polarity GPIO#4 setting.
        USHORT  PolarityGPIO_3:1;   // Polarity GPIO#3 setting.
        USHORT  PolarityGPIO_2:1;   // Polarity GPIO#2 setting.
        USHORT  PolarityGPIO_1:1;   // Polarity GPIO#1 setting.
        USHORT  PolarityGPIO_0:1;   // Polarity GPIO#0 setting.
        USHORT  PolarityACT:1;      // Polarity ACT setting.
        USHORT  PolarityRDY_A:1;        // Polarity RDY_A setting.
        USHORT  PolarityRDY_G:1;        // Polarity RDY_G setting.
    }   field;
    USHORT  word;
}   EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#else
typedef union   _EEPROM_LED_STRUC   {
    struct  {
        USHORT  PolarityRDY_G:1;        // Polarity RDY_G setting.
        USHORT  PolarityRDY_A:1;        // Polarity RDY_A setting.
        USHORT  PolarityACT:1;      // Polarity ACT setting.
        USHORT  PolarityGPIO_0:1;   // Polarity GPIO#0 setting.
        USHORT  PolarityGPIO_1:1;   // Polarity GPIO#1 setting.
        USHORT  PolarityGPIO_2:1;   // Polarity GPIO#2 setting.
        USHORT  PolarityGPIO_3:1;   // Polarity GPIO#3 setting.
        USHORT  PolarityGPIO_4:1;   // Polarity GPIO#4 setting.
        USHORT  LedMode:5;          // Led mode.
        USHORT  Rsvd:3;             // Reserved     
    }   field;
    USHORT  word;
}   EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#endif

//
// QOS_CSR0: TXOP holder address0 register
//
#ifdef BIG_ENDIAN
typedef union   _QOS_CSR0_STRUC {
    struct  {
        UCHAR       Byte3;      // MAC address byte 3
        UCHAR       Byte2;      // MAC address byte 2
        UCHAR       Byte1;      // MAC address byte 1
        UCHAR       Byte0;      // MAC address byte 0
    }   field;
    ULONG           word;
}   QOS_CSR0_STRUC, *PQOS_CSR0_STRUC;
#else
typedef union   _QOS_CSR0_STRUC {
    struct  {
        UCHAR       Byte0;      // MAC address byte 0
        UCHAR       Byte1;      // MAC address byte 1
        UCHAR       Byte2;      // MAC address byte 2
        UCHAR       Byte3;      // MAC address byte 3
    }   field;
    ULONG           word;
}   QOS_CSR0_STRUC, *PQOS_CSR0_STRUC;
#endif

//
// QOS_CSR1: TXOP holder address1 register
//
#ifdef BIG_ENDIAN
typedef union   _QOS_CSR1_STRUC {
    struct  {
        UCHAR       Rsvd1;
        UCHAR       Rsvd0;
        UCHAR       Byte5;      // MAC address byte 5
        UCHAR       Byte4;      // MAC address byte 4
    }   field;
    ULONG           word;
}   QOS_CSR1_STRUC, *PQOS_CSR1_STRUC;
#else
typedef union   _QOS_CSR1_STRUC {
    struct  {
        UCHAR       Byte4;      // MAC address byte 4
        UCHAR       Byte5;      // MAC address byte 5
        UCHAR       Rsvd0;
        UCHAR       Rsvd1;
    }   field;
    ULONG           word;
}   QOS_CSR1_STRUC, *PQOS_CSR1_STRUC;
#endif

#define RF_CSR_CFG  0x500
#ifdef BIG_ENDIAN
typedef union   _RF_CSR_CFG_STRUC   {
    struct  {
        ULONG   Rsvd1:14;               // Reserved
        ULONG   RF_CSR_KICK:1;          // kick RF register read/write
        ULONG   RF_CSR_WR:1;            // 0: read  1: write
        ULONG   Rsvd2:3;                // Reserved
        ULONG   TESTCSR_RFACC_REGNUM:5; // RF register ID
        ULONG   RF_CSR_DATA:8;          // DATA
    }   field;
    ULONG   word;
}   RF_CSR_CFG_STRUC, *PRF_CSR_CFG_STRUC;
#else
typedef union   _RF_CSR_CFG_STRUC   {
    struct  {
        ULONG   RF_CSR_DATA:8;          // DATA 
        ULONG   TESTCSR_RFACC_REGNUM:5; // RF register ID
        ULONG   Rsvd2:3;                // Reserved
        ULONG   RF_CSR_WR:1;            // 0: read  1: write
        ULONG   RF_CSR_KICK:1;          // kick RF register read/write
        ULONG   Rsvd1:14;               // Reserved
    }   field;
    ULONG   word;
}   RF_CSR_CFG_STRUC, *PRF_CSR_CFG_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union _RF_CSR_CFG_EXT_STRUC
{
    struct
    {
        ULONG   Rsvd1:14; // Reserved
        ULONG   RF_CSR_KICK:1; // Kick RF register read/write
        ULONG   RF_CSR_WR:1; // 0: read  1: write
        ULONG   Rsvd2:2; // Reserved
        ULONG   TESTCSR_RFACC_REGNUM:6; // RF register ID (R0~R63)
        ULONG   RF_CSR_DATA:8; // Data
    } field;

    ULONG word;
} RF_CSR_CFG_EXT_STRUC, *PRF_CSR_CFG_EXT_STRUC;
#else
typedef union _RF_CSR_CFG_EXT_STRUC
{
    struct
    {
        ULONG   RF_CSR_DATA:8; // Data
        ULONG   TESTCSR_RFACC_REGNUM:6; // RF register ID (R0~R63)
        ULONG   Rsvd2:2; // Reserved
        ULONG   RF_CSR_WR:1; // 0: read  1: write
        ULONG   RF_CSR_KICK:1; // Kick RF register read/write
        ULONG   Rsvd1:14; // Reserved
    } field;
    
    ULONG word;
} RF_CSR_CFG_EXT_STRUC, *PRF_CSR_CFG_EXT_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union   _EEPROM_WORD_STRUC  {
    struct  {
        UCHAR   Byte1;              // High Byte
        UCHAR   Byte0;              // Low Byte
    }   field;
    USHORT  word;
}   EEPROM_WORD_STRUC, *PEEPROM_WORD_STRUC;
#else
typedef union   _EEPROM_WORD_STRUC  {
    struct  {
        UCHAR   Byte0;              // Low Byte
        UCHAR   Byte1;              // High Byte
    }   field;
    USHORT  word;
}   EEPROM_WORD_STRUC, *PEEPROM_WORD_STRUC;
#endif

#endif  // __RT2870_H__
