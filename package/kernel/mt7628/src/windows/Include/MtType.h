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
    rtmp_type.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Paul Lin    1-2-2004
*/
#ifndef __MTTYPE_H__
#define __MTTYPE_H__

// Compiler Option macro
#define COMPILE_WIN7(_v)                ((_v) == WIN7_NDIS_VER)        // for Win7
#define COMPILE_VISTA_ABOVE(_v)   ((_v) >= VISTA_NDIS_VER)    // for VISTA later version
#define COMPILE_WIN7_ABOVE(_v)   ((_v) >= WIN7_NDIS_VER)        // for Win7 later version
#define COMPILE_WIN8_ABOVE(_v)   ((_v) >= WIN8_NDIS_VER)        // for Win8 later version
#define COMPILE_WIN81_ABOVE(_v)    ((_v) >= WIN81_NDIS_VER)    // for Win8.1 later version

// Run-time NDIS Version macro
#define NDIS_WIN7_BELOW(_V)         (((_V)->pNicCfg->NdisVersion)<RTMP_NDIS620_VERSION)
#define NDIS_WIN7(_pAD)                   (((_pAD)->pNicCfg->NdisVersion) == RTMP_NDIS620_VERSION)
#define NDIS_WIN8(_pAD)                   (((_pAD)->pNicCfg->NdisVersion) == RTMP_NDIS630_VERSION)
#define NDIS_WIN81(_pAD)                (((_pAD)->pNicCfg->NdisVersion) == RTMP_NDIS640_VERSION)
#define NDIS_WIN8_ABOVE(_pAD)      (((_pAD)->pNicCfg->NdisVersion) >= RTMP_NDIS630_VERSION)
#define NDIS_WIN81_ABOVE(_pAD)    (((_pAD)->pNicCfg->NdisVersion) >= RTMP_NDIS640_VERSION)

#define IS_BUS_TYPE_PCI(_V) ( (_V)== BUS_TYPE_PCI)
#define IS_BUS_TYPE_USB(_V) ( (_V)== BUS_TYPE_USB)
#define IS_BUS_TYPE_SDIO(_V) ( (_V)== BUS_TYPE_SDIO)


#define IN
#define OUT

#define VOID void
#define PVOID void * //for Source insight keyword highlight.

#ifndef TRUE
    #define TRUE    1
#endif

#ifndef FALSE
    #define FALSE   0
#endif



#ifndef Win32
#define Win32

#undef  BIG_ENDIAN          // Only little endian for WIN32 system

//#ifdef NDIS50_MINIPORT



//#endif //#ifdef NDIS50_MINIPORT

#endif //#ifndef Win32

#define GNU_PACKED

typedef unsigned short      UINT16;
//typedef unsigned long       UINT32;
typedef unsigned __int64    UINT64;

// Endian byte swapping codes
#define SWAP16(x) \
    ((UINT16)( \
    (((UINT16)(x) & (UINT16) 0x00ffU) << 8) | \
    (((UINT16)(x) & (UINT16) 0xff00U) >> 8) ))

#define SWAP32(x) \
    ((UINT32)( \
    (((UINT32)(x) & (UINT32) 0x000000ffUL) << 24) | \
    (((UINT32)(x) & (UINT32) 0x0000ff00UL) <<  8) | \
    (((UINT32)(x) & (UINT32) 0x00ff0000UL) >>  8) | \
    (((UINT32)(x) & (UINT32) 0xff000000UL) >> 24) ))

#define SWAP64(x) \
    ((UINT64)( \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000000000ffULL) << 56) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000000000ff00ULL) << 40) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000000000ff0000ULL) << 24) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000ff000000ULL) <<  8) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000ff00000000ULL) >>  8) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000ff0000000000ULL) >> 24) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00ff000000000000ULL) >> 40) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56) ))

#ifdef BIG_ENDIAN

#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((UINT64)(x))
#define be2cpu64(x) ((UINT64)(x))
#define cpu2be32(x) ((UINT32)(x))
#define be2cpu32(x) ((UINT32)(x))
#define cpu2be16(x) ((UINT16)(x))
#define be2cpu16(x) ((UINT16)(x))

#else   // Little_Endian

#define cpu2le64(x) ((UINT64)(x))
#define le2cpu64(x) ((UINT64)(x))
#define cpu2le32(x) ((UINT32)(x))
#define le2cpu32(x) ((UINT32)(x))
#define cpu2le16(x) ((UINT16)(x))
#define le2cpu16(x) ((UINT16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))

#endif  // BIG_ENDIAN

#if 0
#if IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
#define PlatformAccessCR(_PAD, _AccessType, _CRAddr, _pData, _Length)
#define HW_IO_READ32(_pAd, _Offset, _pValue)
#define HW_IO_WRITE32(_pAd, _Offset, _Value)

#elif IS_BUS_TYPE_USB(DEV_BUS_TYPE)

#define PlatformAccessCR(_PAD, _AccessType, _CRAddr, _pData, _Length)   USBAccessCR(_PAD, _AccessType, _CRAddr, _pData, _Length)
#define HW_IO_READ32(_pAd, _Offset, _pValue)  Usb_HW_IO_READ32(_pAd, _Offset, _pValue)
#define HW_IO_WRITE32(_pAd, _Offset, _Value)  Usb_HW_IO_WRITE32(_pAd, _Offset, _Value)        

#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)  
#define PlatformAccessCR(_PAD, _AccessType, _CRAddr, _pData, _Length)
#define HW_IO_READ32(_pAd, _Offset, _pValue)
#define HW_IO_WRITE32(_pAd, _Offset, _Value)
#endif
#endif

typedef enum _MLME_PKT_TYPE {
    TXPKT_FW_FRAME         = 0,
    TXPKT_CMD_FRAME         = 1,
    TXPKT_MLME_FRAME        =2,
    TXPKT_NULL_FRAME        = 3,
    TXPKT_PS_POLL_FRAME   =4  
}MLME_PKT_TYPE, * pMLME_PKT_TYPE;

typedef struct  _RSN_IE_HEADER_STRUCT   {
    UCHAR       Eid;
    UCHAR       Length;
    USHORT      Version;    // Little endian format
}   RSN_IE_HEADER_STRUCT, *PRSN_IE_HEADER_STRUCT;

// Cipher suite selector types
typedef struct _CIPHER_SUITE_STRUCT {
    UCHAR       Oui[3];
    UCHAR       Type;
}   CIPHER_SUITE_STRUCT, *PCIPHER_SUITE_STRUCT;

// Authentication and Key Management suite selector
typedef struct _AKM_SUITE_STRUCT    {
    UCHAR       Oui[3];
    UCHAR       Type;
}   AKM_SUITE_STRUCT, *PAKM_SUITE_STRUCT;

// RSN capability
typedef struct  _RSN_CAPABILITY {
    USHORT      Rsv:10;
    USHORT      GTKSAReplayCnt:2;
    USHORT      PTKSAReplayCnt:2;
    USHORT      NoPairwise:1;
    USHORT      PreAuth:1;
}   RSN_CAPABILITY, *PRSN_CAPABILITY;

#define    WLANIDX_MBCAST_PORT0 0
#define    WLANINDEX_MAX 0xff

#define    ROLE_WLANIDX_MBCAST             0
#define    ROLE_WLANIDX_BSSID                1
#define    ROLE_WLANIDX_P2P_CLIENT       2
#define    ROLE_WLANIDX_P2P_DEVICE       3
#define    ROLE_WLANIDX_VWF_BSSID        4
#define    ROLE_WLANIDX_APCLI_MCAST    5
#define    ROLE_WLANIDX_AP_MBCAST       6
#define    ROLE_WLANIDX_ADHOC_MCAST  7
#define    ROLE_WLANIDX_CLIENT              8
#define    ROLE_WLANIDX_UNKNOW           99

#endif  // __RTMP_TYPE_H__