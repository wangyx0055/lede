/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    mt_hif.h
*/

#ifndef __MT_HIF_H__
#define __MT_HIF_H__

#define HIF_BASE            0x4000
#define MT_HIF_BASE         0x4000
#define HIF_SYS_REV         (MT_HIF_BASE + 0x0000)
#define MT_INT_SOURCE_CSR   (MT_HIF_BASE + 0x0200)
#define MT_INT_MASK_CSR     (MT_HIF_BASE + 0x0204)

#define MT_WPDMA_GLO_CFG    (MT_HIF_BASE + 0x0208)
#define TX_DMA_EN (1 << 0)
#define TX_DMA_BUSY (1 << 1)
#define RX_DMA_EN (1 << 2)
#define RX_DMA_BUSY (1 << 3)

#define WPDMA_RST_PTR       (MT_HIF_BASE + 0x020c)
#define MT_DELAY_INT_CFG    (MT_HIF_BASE + 0x0210)

#define SCH_REG4 (MT_HIF_BASE + 0x0594)
#define SCH_REG4_FORCE_QID_MASK (0x0f)
#define SCH_REG4_FORCE_QID(p) (((p) & 0x0f))
#define SCH_REG4_BYPASS_MODE_MASK (0x1 << 5)
#define SCH_REG4_BYPASS_MODE(p) (((p) & 0x1) << 5)

/* UDMA register */
#define UDMA_BASE       0x50029000
#define UDMA_WLCFG_1    (0x)
#define UDMA_RESET      (UDMA_BASE + 0x14)
#define UDMA_WLCFG_0    (UDMA_BASE + 0x18)
#define UDMA_WLCFG_0_TX_BT_SIZE_MASK (0x07 << 27)
#define UDMA_WLCFG_0_TX_BT_SIZE(p) (((p) & 0x07) << 27)
#define UDMA_WLCFG_0_RX_BT_SIZE_MASK (0x07 << 24)
#define UDMA_WLCFG_0_RX_BT_SIZE(p) (((p) & 0x07) << 24)
#define UDMA_WLCFG_0_TX_EN_MASK (0x1 << 23)
#define UDMA_WLCFG_0_TX_EN(p) (((p) & 0x1) << 23)
#define UDMA_WLCFG_0_RX_EN_MASK (0x1 << 22)
#define UDMA_WLCFG_0_RX_EN(p) (((p) & 0x1) << 22)
#define UDMA_WLCFG_0_RX_AGG_EN_MASK (0x1 << 21)
#define UDMA_WLCFG_0_RX_AGG_EN(p) (((p) & 0x1) << 21)
#define UDMA_WLCFG_0_LPK_EN_MASK (0x1 << 20)
#define UDMA_WLCFG_0_LPK_EN(p) (((p) & 0x1) << 20)
#define UDMA_WLCFG_0_RX_MPSZ_PAD0_MASK (0x1 << 18)
#define UDMA_WLCFG_0_RX_MPSZ_PAD0(p) (((p) & 0x1) << 18)
#define UDMA_WLCFG_0_RX_AGG_LMT_MASK (0xff << 8)
#define UDMA_WLCFG_0_RX_AGG_LMT(p) (((p) & 0xff) << 8)
#define UDMA_WLCFG_0_RX_AGG_TO_MASK (0xff << 0)
#define UDMA_WLCFG_0_RX_AGG_TO(p) (((p) & 0xff) << 0)

#define STOP_DROP_EPOUT (0x80)

#define MT_TXTIME_THD_0     (HIF_BASE + 0x500)
#define MT_TXTIME_THD_1     (HIF_BASE + 0x504)
#define MT_TXTIME_THD_2     (HIF_BASE + 0x508)
#define MT_TXTIME_THD_3     (HIF_BASE + 0x50c)
#define MT_TXTIME_THD_4     (HIF_BASE + 0x510)
#define MT_TXTIME_THD_5     (HIF_BASE + 0x514)
#define MT_TXTIME_THD_6     (HIF_BASE + 0x518)
#define MT_TXTIME_THD_7     (HIF_BASE + 0x51c)
#define MT_TXTIME_THD_8     (HIF_BASE + 0x520)
#define MT_TXTIME_THD_9     (HIF_BASE + 0x524)
#define MT_TXTIME_THD_10    (HIF_BASE + 0x528)
#define MT_TXTIME_THD_11    (HIF_BASE + 0x52c)
#define MT_TXTIME_THD_12    (HIF_BASE + 0x530)
#define MT_TXTIME_THD_13    (HIF_BASE + 0x534)
#define MT_TXTIME_THD_14    (HIF_BASE + 0x538)
#define MT_TXTIME_THD_15    (HIF_BASE + 0x53c)

#define MT_PAGE_CNT_0       (HIF_BASE + 0x540)
#define MT_PAGE_CNT_1       (HIF_BASE + 0x544)
#define MT_PAGE_CNT_2       (HIF_BASE + 0x548)
#define MT_PAGE_CNT_3       (HIF_BASE + 0x54c)
#define MT_PAGE_CNT_4       (HIF_BASE + 0x550)
#define MT_PAGE_CNT_5       (HIF_BASE + 0x554)
#define MT_PAGE_CNT_6       (HIF_BASE + 0x558)
#define MT_PAGE_CNT_7       (HIF_BASE + 0x55c)
#define MT_PAGE_CNT_8       (HIF_BASE + 0x560)
#define MT_PAGE_CNT_9       (HIF_BASE + 0x564)
#define MT_PAGE_CNT_10      (HIF_BASE + 0x568)
#define MT_PAGE_CNT_11      (HIF_BASE + 0x56c)
#define MT_PAGE_CNT_12      (HIF_BASE + 0x570)
#define MT_PAGE_CNT_13      (HIF_BASE + 0x574)
#define MT_PAGE_CNT_14      (HIF_BASE + 0x578)
#define MT_PAGE_CNT_15      (HIF_BASE + 0x57c)

#define MT_QUEUE_PRIORITY_1 (HIF_BASE + 0x580)
#define MT_QUEUE_PRIORITY_2 (HIF_BASE + 0x584)

#define MT_SCH_REG_1        (HIF_BASE + 0x588)
#define MT_SCH_REG_2        (HIF_BASE + 0x58c)
#define MT_SCH_REG_3        (HIF_BASE + 0x590)
#define MT_SCH_REG_4        (HIF_BASE + 0x594)

#define MT_GROUP_THD_0      (HIF_BASE + 0x598)
#define MT_GROUP_THD_1      (HIF_BASE + 0x59c)
#define MT_GROUP_THD_2      (HIF_BASE + 0x5a0)
#define MT_GROUP_THD_3      (HIF_BASE + 0x5a4)
#define MT_GROUP_THD_4      (HIF_BASE + 0x5a8)
#define MT_GROUP_THD_5      (HIF_BASE + 0x5ac)

#define MT_BMAP_0       (HIF_BASE + 0x5b0)
#define MT_BMAP_1       (HIF_BASE + 0x5b4)
#define MT_BMAP_2       (HIF_BASE + 0x5b8)

#define MT_HIGH_PRIORITY_1  (HIF_BASE + 0x5bc)
#define MT_HIGH_PRIORITY_2  (HIF_BASE + 0x5c0)
#define MT_PRIORITY_MASK    (HIF_BASE + 0x5c4)

#define MT_RSV_MAX_THD      (HIF_BASE + 0x5c8)

#endif
