/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
 	Driver.c
 
	Abstract:
    Central header file to maintain all include files for all NDIS
    miniport driver routines.

	Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Rory Chen   08-21-2002    created

*/


#define INITGUID		// initialize PCIDUMPR_GUID in this module

#include "config.h"
//#include "FW7603.h"
//#include "FW7636RomPatch.h"
//#include "FW7636.h"

#include <Ntstrsafe.h>
#ifdef RTMP_USB_SUPPORT
#include "bulkusr.h"
#endif

ULONG	RTDebugLevel = DBG_LVL_TRACE;
#define RETRY_INTERVAL    900*1000 //5s
BOOLEAN					g_bDam4kBoundaryCheck = FALSE;
ULONG	TXWI_SIZE;
//ULONG g_chPattern[10];//
//extern UCHAR	g_Pattern1[52];
//extern UCHAR	g_Pattern2[4000];
UCHAR g_FWSeqUI 		= 0;//fw download
UCHAR g_FWSeqMCU 		= 0;//fw download
UCHAR g_FWRspStatus 	= 0;//fw download

//UCHAR g_PacketCMDSeqUI		= 0;//fw download
UCHAR g_PacketCMDSeqMCU 		= 0;//fw download
UCHAR g_PacketCMDRspData[PKTCMD_EVENT_BUFFER];

extern UCHAR	g_TxDPattern[32];
extern UCHAR	g_PayloadPattern[4000];
extern ULONG    g_ulTXPacketLength;
#if 0
UCHAR FirmwareImage4K[] = {
    0x02, 0x0f, 0x1c, 0x02, 0x0e, 0x0f, 0x8f, 0x50, 0xd2, 0x59, 0x22, 0x02, 0x0c, 0x31, 0x8f, 0x54, 
    0xd2, 0x58, 0x22, 0x02, 0x0b, 0x23, 0x02, 0x0e, 0x31, 0xc3, 0x22, 0x02, 0x0b, 0xe2, 0x90, 0x04, 
    0x14, 0xe0, 0x20, 0xe7, 0x03, 0x02, 0x04, 0x91, 0x90, 0x70, 0x12, 0xe0, 0xf5, 0x56, 0x90, 0x04, 
    0x04, 0xe0, 0x12, 0x0d, 0xc5, 0x00, 0xc8, 0x30, 0x00, 0x9f, 0x31, 0x00, 0x81, 0x35, 0x00, 0x78, 
    0x36, 0x00, 0xd5, 0x40, 0x00, 0xec, 0x41, 0x01, 0x03, 0x50, 0x01, 0x48, 0x51, 0x01, 0x51, 0x52, 
    0x01, 0x51, 0x53, 0x01, 0x51, 0x54, 0x01, 0x8d, 0x55, 0x01, 0xea, 0x56, 0x02, 0x3d, 0x70, 0x02, 
    0x63, 0x71, 0x02, 0x8c, 0x72, 0x03, 0x37, 0x73, 0x03, 0x5b, 0x74, 0x04, 0x05, 0x80, 0x04, 0x75, 
    0x83, 0x04, 0x2c, 0x91, 0x00, 0x00, 0x04, 0x91, 0x90, 0x70, 0x11, 0xe0, 0xf5, 0x3c, 0x02, 0x04, 
    0x8b, 0xe5, 0x55, 0xb4, 0x02, 0x0f, 0xe5, 0x58, 0x30, 0xe0, 0x06, 0x90, 0x01, 0x0d, 0x74, 0x08, 
    0xf0, 0x7d, 0x01, 0x80, 0x02, 0x7d, 0x02, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x02, 0x04, 0x8b, 0x20, 
    0x02, 0x03, 0x30, 0x03, 0x0a, 0x7d, 0x02, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x02, 0x04, 0x8b, 0xe5, 
    0x25, 0xd3, 0x94, 0x01, 0x40, 0x0c, 0x90, 0x01, 0x0c, 0xe0, 0x44, 0x02, 0xf0, 0xa3, 0xe0, 0x44, 
    0x04, 0xf0, 0x85, 0x56, 0x41, 0xd2, 0x02, 0x22, 0x90, 0x70, 0x11, 0xe0, 0xb4, 0x5a, 0x03, 0xc2, 
    0x4f, 0x22, 0xd2, 0x4f, 0x22, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x50, 0x03, 0x02, 0x04, 0x91, 0x90, 
    0x01, 0x0c, 0xe0, 0x44, 0x02, 0xf0, 0xa3, 0xe0, 0x44, 0x04, 0xf0, 0x22, 0xe5, 0x25, 0xd3, 0x94, 
    0x01, 0x50, 0x03, 0x02, 0x04, 0x91, 0x90, 0x01, 0x0c, 0xe0, 0x54, 0xfd, 0xf0, 0xa3, 0xe0, 0x54, 
    0xfb, 0xf0, 0x22, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x40, 0x07, 0xe5, 0x55, 0x60, 0x03, 0x02, 0x04, 
    0x91, 0x90, 0x70, 0x10, 0xe0, 0x54, 0x7f, 0xff, 0xbf, 0x0a, 0x0d, 0x90, 0x70, 0x11, 0xe0, 0xb4, 
    0x08, 0x06, 0x75, 0x4e, 0x01, 0x75, 0x4f, 0x84, 0x90, 0x70, 0x10, 0xe0, 0x54, 0x7f, 0xff, 0xbf, 
    0x02, 0x12, 0x90, 0x70, 0x11, 0xe0, 0x64, 0x08, 0x60, 0x04, 0xe0, 0xb4, 0x20, 0x06, 0x75, 0x4e, 
    0x03, 0x75, 0x4f, 0x20, 0xe4, 0xf5, 0x27, 0x22, 0x90, 0x70, 0x11, 0xe0, 0x24, 0xff, 0x92, 0x47, 
    0x22, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x40, 0x07, 0xe5, 0x55, 0x60, 0x03, 0x02, 0x03, 0x42, 0x90, 
    0x04, 0x04, 0xe0, 0x25, 0xe0, 0x24, 0x5d, 0xf5, 0x57, 0x90, 0x70, 0x10, 0xe0, 0xff, 0x74, 0x47, 
    0x25, 0x57, 0xf8, 0xc6, 0xef, 0xc6, 0x90, 0x70, 0x11, 0xe0, 0xff, 0x74, 0x48, 0x25, 0x57, 0xf8, 
    0xc6, 0xef, 0xc6, 0xe4, 0xfd, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x02, 0x04, 0x8b, 0xe5, 0x25, 0xd3, 
    0x94, 0x01, 0x40, 0x07, 0xe5, 0x55, 0x60, 0x03, 0x02, 0x03, 0x42, 0xe5, 0x47, 0x64, 0x07, 0x60, 
    0x1d, 0xe5, 0x47, 0x64, 0x08, 0x60, 0x17, 0xe5, 0x47, 0x64, 0x09, 0x60, 0x11, 0xe5, 0x47, 0x64, 
    0x0a, 0x60, 0x0b, 0xe5, 0x47, 0x64, 0x0b, 0x60, 0x05, 0xe5, 0x47, 0xb4, 0x0c, 0x08, 0x90, 0x70, 
    0x11, 0xe0, 0x54, 0x0f, 0xf5, 0x3a, 0xe5, 0x47, 0xb4, 0x09, 0x08, 0xe5, 0x3a, 0xb4, 0x03, 0x03, 
    0xe4, 0xf5, 0x46, 0xe5, 0x47, 0xb4, 0x0a, 0x08, 0xe5, 0x3a, 0xb4, 0x01, 0x03, 0xe4, 0xf5, 0x46, 
    0xe4, 0xfd, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0xd2, 0x04, 0x22, 0x90, 0x70, 0x11, 0xe0, 0xf4, 0xff, 
    0x90, 0x70, 0x10, 0xe0, 0x5f, 0xff, 0x90, 0x70, 0x11, 0xe0, 0x55, 0x27, 0x4f, 0x90, 0x70, 0x18, 
    0xf0, 0x90, 0x70, 0x11, 0xe0, 0x90, 0x70, 0x19, 0xf0, 0xe4, 0xfd, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 
    0x30, 0x15, 0x03, 0xd2, 0x14, 0x22, 0x90, 0x70, 0x18, 0xe0, 0xf5, 0x27, 0x90, 0x02, 0x29, 0xe0, 
    0xff, 0x90, 0x70, 0x19, 0xe0, 0xfe, 0xef, 0x5e, 0x90, 0x02, 0x29, 0xf0, 0x30, 0x47, 0x04, 0xaf, 
    0x27, 0x80, 0x04, 0xe5, 0x27, 0xf4, 0xff, 0x90, 0x02, 0x28, 0xef, 0xf0, 0x22, 0xe5, 0x25, 0xd3, 
    0x94, 0x01, 0x40, 0x07, 0xe5, 0x55, 0x60, 0x03, 0x02, 0x03, 0x42, 0x90, 0x70, 0x10, 0xe0, 0xfe, 
    0x90, 0x70, 0x11, 0xe0, 0xfd, 0xed, 0xf8, 0xe6, 0xf5, 0x57, 0xfd, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 
    0x02, 0x04, 0x8b, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x40, 0x07, 0xe5, 0x55, 0x60, 0x03, 0x02, 0x03, 
    0x42, 0x90, 0x70, 0x10, 0xe0, 0xfe, 0x90, 0x70, 0x11, 0xe0, 0xfd, 0xed, 0xf5, 0x82, 0x8e, 0x83, 
    0xe0, 0xf5, 0x57, 0xfd, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x02, 0x04, 0x8b, 0x90, 0x10, 0x00, 0xe0, 
    0xf5, 0x57, 0xe4, 0xf5, 0x58, 0xf5, 0x59, 0x90, 0x10, 0x03, 0xe0, 0xb4, 0x28, 0x05, 0x75, 0x58, 
    0x01, 0x80, 0x3c, 0x90, 0x10, 0x03, 0xe0, 0xb4, 0x30, 0x05, 0x75, 0x58, 0x02, 0x80, 0x30, 0x90, 
    0x10, 0x03, 0xe0, 0xb4, 0x33, 0x05, 0x75, 0x58, 0x04, 0x80, 0x24, 0x90, 0x10, 0x03, 0xe0, 0xb4, 
    0x35, 0x0c, 0x90, 0x10, 0x02, 0xe0, 0xb4, 0x72, 0x05, 0x75, 0x58, 0x08, 0x80, 0x11, 0x90, 0x10, 
    0x03, 0xe0, 0xb4, 0x35, 0x0a, 0x90, 0x10, 0x02, 0xe0, 0xb4, 0x93, 0x03, 0x75, 0x58, 0x10, 0xe5, 
    0x58, 0x30, 0xe1, 0x19, 0x90, 0x05, 0x08, 0xe0, 0x44, 0x01, 0xf0, 0xfd, 0x90, 0x05, 0x05, 0xe0, 
    0x54, 0xfb, 0xf0, 0x44, 0x04, 0xf0, 0xed, 0x54, 0xfe, 0x90, 0x05, 0x08, 0xf0, 0xe4, 0xf5, 0x4e, 
    0xf5, 0x4f, 0x75, 0x3a, 0xff, 0xf5, 0x25, 0x90, 0x05, 0xa4, 0x74, 0x11, 0xf0, 0xa3, 0x74, 0xff, 
    0xf0, 0xa3, 0x74, 0x03, 0xf0, 0xd2, 0x4f, 0x90, 0x01, 0x0d, 0xe0, 0x44, 0x40, 0xf0, 0x75, 0x3c, 
    0xff, 0xad, 0x57, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x90, 0x70, 0x34, 0x74, 0x31, 0xf0, 0xa3, 0x74, 
    0x26, 0xf0, 0xc2, 0x17, 0x02, 0x04, 0x8b, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x40, 0x0b, 0xe5, 0x55, 
    0x60, 0x07, 0x7d, 0x03, 0xaf, 0x56, 0x02, 0x0d, 0xeb, 0x90, 0x70, 0x10, 0xe0, 0x24, 0xff, 0x92, 
    0x93, 0xe4, 0xfd, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x02, 0x04, 0x8b, 0x90, 0x10, 0x00, 0xe0, 0x90, 
    0x10, 0x2c, 0xf0, 0x90, 0x10, 0x2f, 0x74, 0x40, 0xf0, 0x90, 0x70, 0x11, 0xe0, 0xf5, 0x57, 0xe0, 
    0x54, 0x80, 0x90, 0x70, 0x32, 0xf0, 0x90, 0x70, 0x10, 0xe0, 0xff, 0x90, 0x70, 0x11, 0xe0, 0xd3, 
    0x9f, 0x40, 0x35, 0x90, 0x70, 0x33, 0xe5, 0x57, 0xf0, 0x90, 0x70, 0x10, 0xe0, 0xff, 0x90, 0x70, 
    0x33, 0xe0, 0xc3, 0x9f, 0x40, 0x57, 0xe0, 0xff, 0x90, 0x70, 0x32, 0xe0, 0x4f, 0x90, 0x05, 0x00, 
    0xf0, 0xa3, 0x74, 0x11, 0xf0, 0xa3, 0x74, 0x01, 0xf0, 0x74, 0x03, 0xf0, 0xff, 0x12, 0x0e, 0xf8, 
    0x90, 0x70, 0x33, 0xe0, 0x14, 0xf0, 0x80, 0xd1, 0x90, 0x70, 0x33, 0xe5, 0x57, 0xf0, 0x90, 0x70, 
    0x10, 0xe0, 0xff, 0x90, 0x70, 0x33, 0xe0, 0xd3, 0x9f, 0x50, 0x22, 0xe0, 0xff, 0x90, 0x70, 0x32, 
    0xe0, 0x4f, 0x90, 0x05, 0x00, 0xf0, 0xa3, 0x74, 0x11, 0xf0, 0xa3, 0x74, 0x01, 0xf0, 0x74, 0x03, 
    0xf0, 0xff, 0x12, 0x0e, 0xf8, 0x90, 0x70, 0x33, 0xe0, 0x04, 0xf0, 0x80, 0xd1, 0x90, 0x10, 0x00, 
    0xe0, 0x90, 0x10, 0x2c, 0xf0, 0x90, 0x10, 0x2f, 0x74, 0x7f, 0xf0, 0xe4, 0xfd, 0xaf, 0x56, 0x12, 
    0x0d, 0xeb, 0x02, 0x04, 0x8b, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x40, 0x0d, 0xe5, 0x55, 0x60, 0x09, 
    0x7d, 0x03, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x80, 0x72, 0x90, 0x70, 0x10, 0xe0, 0x24, 0xff, 0x92, 
    0x4a, 0xd2, 0x05, 0xad, 0x57, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x80, 0x5f, 0x90, 0x70, 0x11, 0xe0, 
    0x24, 0xff, 0x92, 0x17, 0x90, 0x70, 0x10, 0xe0, 0xf5, 0x5d, 0xad, 0x57, 0xaf, 0x56, 0x12, 0x0d, 
    0xeb, 0x90, 0x04, 0x14, 0x74, 0x80, 0xf0, 0x30, 0x17, 0x13, 0x90, 0x10, 0x00, 0xe0, 0x90, 0x10, 
    0x2c, 0xf0, 0x90, 0x10, 0x2f, 0xe0, 0x54, 0xf0, 0xf5, 0x57, 0x45, 0x5d, 0xf0, 0xe4, 0x90, 0x70, 
    0x13, 0xf0, 0xe5, 0x56, 0xf4, 0x60, 0x2a, 0x90, 0x70, 0x25, 0xe0, 0x44, 0x01, 0xf0, 0x90, 0x02, 
    0x2c, 0x74, 0xff, 0xf0, 0x22, 0xe4, 0xf5, 0x25, 0xd2, 0x4f, 0x90, 0x70, 0x10, 0xe0, 0xf4, 0x60, 
    0x03, 0xe0, 0xf5, 0x25, 0xad, 0x57, 0xaf, 0x56, 0x12, 0x0d, 0xeb, 0x90, 0x04, 0x14, 0x74, 0x80, 
    0xf0, 0x22, 0xc2, 0xaf, 0x90, 0x04, 0x14, 0xe0, 0x54, 0x0e, 0x60, 0x04, 0xd2, 0x18, 0x80, 0x08, 
    0xe5, 0x4e, 0x45, 0x4f, 0x24, 0xff, 0x92, 0x18, 0xd2, 0xaf, 0x90, 0x04, 0x14, 0xe0, 0xa2, 0xe4, 
    0x92, 0x19, 0x74, 0x1e, 0xf0, 0xe5, 0x5f, 0x54, 0x0f, 0xf5, 0x2d, 0xe5, 0x2a, 0x70, 0x13, 0x30, 
    0x18, 0x05, 0xe5, 0x5f, 0x20, 0xe5, 0x0b, 0x30, 0x19, 0x19, 0xe5, 0x5f, 0x54, 0x30, 0xff, 0xbf, 
    0x30, 0x11, 0xe5, 0x2a, 0x70, 0x05, 0x75, 0x2a, 0x0c, 0x80, 0x02, 0x15, 0x2a, 0xd2, 0x6c, 0xd2, 
    0x6d, 0x80, 0x0f, 0xe5, 0x5f, 0x30, 0xe6, 0x06, 0xc2, 0x6c, 0xd2, 0x6d, 0x80, 0x04, 0xd2, 0x6c, 
    0xc2, 0x6d, 0xe5, 0x47, 0x64, 0x03, 0x70, 0x21, 0x30, 0x4b, 0x06, 0xc2, 0x6c, 0xd2, 0x6d, 0x80, 
    0x18, 0xe5, 0x2a, 0x70, 0x03, 0x30, 0x4c, 0x11, 0xc2, 0x4c, 0xe5, 0x2a, 0x70, 0x05, 0x75, 0x2a, 
    0x07, 0x80, 0x02, 0x15, 0x2a, 0xd2, 0x6c, 0xd2, 0x6d, 0xe5, 0x47, 0xb4, 0x09, 0x14, 0xe5, 0x44, 
    0x20, 0xe3, 0x0b, 0xe5, 0x3a, 0x64, 0x02, 0x60, 0x05, 0xe5, 0x3a, 0xb4, 0x03, 0x04, 0xc2, 0x6c, 
    0xd2, 0x6d, 0xe5, 0x47, 0xb4, 0x0a, 0x13, 0xe5, 0x3a, 0xb4, 0x01, 0x06, 0xc2, 0x6c, 0xd2, 0x6d, 
    0x80, 0x08, 0xe5, 0x3a, 0x70, 0x04, 0xd2, 0x6c, 0xc2, 0x6d, 0x20, 0x69, 0x07, 0xe5, 0x5e, 0x20, 
    0xe0, 0x02, 0xb2, 0x68, 0x20, 0x6b, 0x07, 0xe5, 0x5e, 0x20, 0xe1, 0x02, 0xb2, 0x6a, 0x20, 0x6d, 
    0x07, 0xe5, 0x5e, 0x20, 0xe2, 0x02, 0xb2, 0x6c, 0x75, 0x2e, 0x40, 0x20, 0x69, 0x04, 0xa2, 0x68, 
    0x80, 0x26, 0x30, 0x68, 0x06, 0xe5, 0x46, 0xa2, 0xe2, 0x80, 0x1d, 0xe5, 0x5e, 0x20, 0xe0, 0x04, 
    0x7f, 0x01, 0x80, 0x02, 0x7f, 0x00, 0xe5, 0x46, 0x54, 0xf0, 0xfe, 0xbe, 0xf0, 0x04, 0x7e, 0x01, 
    0x80, 0x02, 0x7e, 0x00, 0xee, 0x6f, 0x24, 0xff, 0x92, 0x73, 0x92, 0x72, 0x20, 0x6b, 0x04, 0xa2, 
    0x6a, 0x80, 0x26, 0x30, 0x6a, 0x06, 0xe5, 0x46, 0xa2, 0xe2, 0x80, 0x1d, 0xe5, 0x5e, 0x20, 0xe1, 
    0x04, 0x7f, 0x01, 0x80, 0x02, 0x7f, 0x00, 0xe5, 0x46, 0x54, 0xf0, 0xfe, 0xbe, 0xf0, 0x04, 0x7e, 
    0x01, 0x80, 0x02, 0x7e, 0x00, 0xee, 0x6f, 0x24, 0xff, 0x92, 0x75, 0x92, 0x74, 0x20, 0x6d, 0x04, 
    0xa2, 0x6c, 0x80, 0x26, 0xe5, 0x47, 0x64, 0x0a, 0x70, 0x22, 0x30, 0x6c, 0x06, 0xe5, 0x46, 0xa2, 
    0xe3, 0x80, 0x17, 0xe5, 0x3a, 0xb4, 0x01, 0x06, 0xe5, 0x46, 0xa2, 0xe3, 0x80, 0x34, 0xe5, 0x46, 
    0x20, 0xe4, 0x03, 0x30, 0xe5, 0x03, 0xd3, 0x80, 0x01, 0xc3, 0x80, 0x26, 0x30, 0x6c, 0x06, 0xe5, 
    0x46, 0xa2, 0xe2, 0x80, 0x1d, 0xe5, 0x5e, 0x20, 0xe2, 0x04, 0x7f, 0x01, 0x80, 0x02, 0x7f, 0x00, 
    0xe5, 0x46, 0x54, 0xf0, 0xfe, 0xbe, 0xf0, 0x04, 0x7e, 0x01, 0x80, 0x02, 0x7e, 0x00, 0xee, 0x6f, 
    0x24, 0xff, 0x92, 0x71, 0x92, 0x70, 0x90, 0x10, 0x00, 0xe0, 0x90, 0x10, 0x2c, 0xf0, 0x90, 0x10, 
    0x03, 0xe0, 0xc3, 0x94, 0x30, 0x40, 0x19, 0xe0, 0x64, 0x32, 0x60, 0x14, 0xa2, 0x71, 0x92, 0x77, 
    0xa2, 0x70, 0x92, 0x76, 0xe5, 0x2e, 0x13, 0x13, 0x54, 0x3f, 0xf5, 0x2e, 0xc2, 0x77, 0xd2, 0x76, 
    0x30, 0x17, 0x0d, 0x53, 0x2e, 0xf0, 0xe5, 0x2e, 0x45, 0x5d, 0x90, 0x10, 0x2f, 0xf0, 0x80, 0x06, 
    0x90, 0x10, 0x2f, 0xe5, 0x2e, 0xf0, 0xe5, 0x47, 0x64, 0x06, 0x70, 0x47, 0x90, 0x02, 0x28, 0xe0, 
    0x30, 0x47, 0x03, 0xff, 0x80, 0x02, 0xf4, 0xff, 0x8f, 0x27, 0x90, 0x02, 0x29, 0xe0, 0x54, 0xfe, 
    0xf0, 0xe5, 0x43, 0xc4, 0x54, 0x0f, 0x14, 0x60, 0x0c, 0x24, 0xfe, 0x60, 0x0c, 0x24, 0x03, 0x70, 
    0x13, 0xc2, 0x38, 0x80, 0x0f, 0xd2, 0x38, 0x80, 0x0b, 0xe5, 0x46, 0x30, 0xe2, 0x03, 0xd3, 0x80, 
    0x01, 0xc3, 0x92, 0x38, 0x30, 0x47, 0x05, 0xaf, 0x27, 0x02, 0x07, 0xe8, 0xe5, 0x27, 0xf4, 0xff, 
    0x02, 0x07, 0xe8, 0xe5, 0x47, 0x64, 0x07, 0x60, 0x0f, 0xe5, 0x47, 0x64, 0x08, 0x60, 0x09, 0xe5, 
    0x47, 0x64, 0x09, 0x60, 0x03, 0x02, 0x07, 0x56, 0x90, 0x02, 0x28, 0xe0, 0x30, 0x47, 0x03, 0xff, 
    0x80, 0x02, 0xf4, 0xff, 0x8f, 0x27, 0x90, 0x02, 0x29, 0xe0, 0x54, 0xfc, 0xf0, 0xe5, 0x3a, 0x14, 
    0x60, 0x22, 0x14, 0x60, 0x25, 0x14, 0x60, 0x2d, 0x24, 0xfc, 0x60, 0x49, 0x24, 0xf9, 0x60, 0x14, 
    0x24, 0x0e, 0x70, 0x50, 0xe5, 0x46, 0x13, 0x13, 0x54, 0x3f, 0x75, 0xf0, 0x03, 0x84, 0xe5, 0xf0, 
    0x24, 0xff, 0x80, 0x3a, 0xd2, 0x39, 0xc2, 0x38, 0x80, 0x3e, 0xe5, 0x46, 0x30, 0xe2, 0x03, 0xd3, 
    0x80, 0x1d, 0xc3, 0x80, 0x1a, 0xe5, 0x46, 0x30, 0xe2, 0x0d, 0x54, 0x38, 0xc3, 0x94, 0x30, 0x50, 
    0x06, 0x7e, 0x00, 0x7f, 0x01, 0x80, 0x04, 0x7e, 0x00, 0x7f, 0x00, 0xee, 0x4f, 0x24, 0xff, 0x92, 
    0x38, 0xc2, 0x39, 0x80, 0x13, 0xe5, 0x46, 0x30, 0xe2, 0x03, 0xd3, 0x80, 0x01, 0xc3, 0x92, 0x39, 
    0xc2, 0x38, 0x80, 0x04, 0xc2, 0x38, 0xc2, 0x39, 0x30, 0x47, 0x04, 0xaf, 0x27, 0x80, 0x04, 0xe5, 
    0x27, 0xf4, 0xff, 0x02, 0x07, 0xe8, 0xe5, 0x47, 0x64, 0x0c, 0x60, 0x09, 0xe5, 0x47, 0x64, 0x0b, 
    0x60, 0x03, 0x02, 0x07, 0xed, 0x90, 0x02, 0x28, 0xe0, 0x30, 0x47, 0x03, 0xff, 0x80, 0x02, 0xf4, 
    0xff, 0x8f, 0x27, 0x90, 0x02, 0x29, 0xe0, 0x54, 0xfd, 0xf0, 0xe5, 0x3a, 0x14, 0x60, 0x20, 0x14, 
    0x60, 0x21, 0x14, 0x60, 0x2b, 0x24, 0xfc, 0x60, 0x45, 0x24, 0xf9, 0x60, 0x12, 0x24, 0x0e, 0x70, 
    0x4a, 0xe5, 0x46, 0x13, 0x13, 0x54, 0x3f, 0x75, 0xf0, 0x03, 0x84, 0xe5, 0xf0, 0x80, 0x29, 0xd2, 
    0x39, 0x80, 0x3a, 0xe5, 0x46, 0x30, 0xe2, 0x03, 0xd3, 0x80, 0x01, 0xc3, 0x92, 0x39, 0x80, 0x2d, 
    0xe5, 0x46, 0x30, 0xe2, 0x0d, 0x54, 0x38, 0xc3, 0x94, 0x30, 0x50, 0x06, 0x7e, 0x00, 0x7f, 0x01, 
    0x80, 0x04, 0x7e, 0x00, 0x7f, 0x00, 0xee, 0x4f, 0x24, 0xff, 0x92, 0x39, 0x80, 0x0f, 0xe5, 0x46, 
    0x30, 0xe2, 0x03, 0xd3, 0x80, 0x01, 0xc3, 0x92, 0x39, 0x80, 0x02, 0xc2, 0x39, 0x30, 0x47, 0x04, 
    0xaf, 0x27, 0x80, 0x04, 0xe5, 0x27, 0xf4, 0xff, 0x90, 0x02, 0x28, 0xef, 0xf0, 0x22, 0xe4, 0xf5, 
    0x30, 0xc2, 0xaf, 0xe5, 0x51, 0x14, 0x60, 0x47, 0x14, 0x60, 0x65, 0x24, 0x02, 0x60, 0x03, 0x02, 
    0x09, 0x49, 0xd2, 0x59, 0x75, 0x55, 0x01, 0x90, 0x02, 0x08, 0xe0, 0x54, 0xfe, 0xf0, 0xe0, 0x20, 
    0xe1, 0x23, 0x90, 0x04, 0x34, 0xe0, 0xb4, 0x02, 0x1c, 0xa3, 0xe0, 0xb4, 0x02, 0x17, 0xa3, 0xe0, 
    0xb4, 0x02, 0x12, 0x7f, 0x20, 0x12, 0x00, 0x06, 0x90, 0x10, 0x04, 0xe0, 0x54, 0xf3, 0xf0, 0x75, 
    0x51, 0x01, 0x02, 0x09, 0x49, 0xe5, 0x50, 0x70, 0x06, 0x75, 0x30, 0x03, 0x02, 0x09, 0x49, 0x90, 
    0x12, 0x00, 0xe0, 0x54, 0x03, 0x70, 0x12, 0x7f, 0x20, 0x12, 0x00, 0x06, 0x90, 0x02, 0x08, 0xe0, 
    0x54, 0xfb, 0xf0, 0x75, 0x51, 0x02, 0x02, 0x09, 0x49, 0xe5, 0x50, 0x70, 0x03, 0x02, 0x09, 0x44, 
    0x90, 0x02, 0x08, 0xe0, 0x30, 0xe3, 0x03, 0x02, 0x09, 0x40, 0x90, 0x04, 0x37, 0xe0, 0x64, 0x22, 
    0x60, 0x03, 0x02, 0x09, 0x40, 0x90, 0x12, 0x04, 0x74, 0x0a, 0xf0, 0xe5, 0x58, 0x30, 0xe3, 0x1c, 
    0x90, 0x00, 0x02, 0xe0, 0x30, 0xe0, 0x15, 0xe4, 0x90, 0x05, 0x00, 0xf0, 0xa3, 0x74, 0x08, 0xf0, 
    0xa3, 0x74, 0x01, 0xf0, 0x74, 0x03, 0xf0, 0x7f, 0x01, 0x12, 0x0e, 0xe9, 0x90, 0x13, 0x28, 0xe0, 
    0x90, 0x70, 0x1a, 0xf0, 0x90, 0x13, 0x29, 0xe0, 0x90, 0x70, 0x1b, 0xf0, 0x90, 0x13, 0x2b, 0xe0, 
    0x90, 0x70, 0x22, 0xf0, 0x90, 0x13, 0x28, 0xe0, 0x54, 0xf0, 0xf0, 0xa3, 0xe0, 0x54, 0xf0, 0xf0, 
    0x90, 0x13, 0x2b, 0xe0, 0x54, 0xcc, 0xf0, 0xe5, 0x58, 0x30, 0xe3, 0x17, 0xe5, 0x25, 0x70, 0x13, 
    0xe5, 0x3c, 0xf4, 0x90, 0x13, 0x2a, 0x60, 0x05, 0xe0, 0x54, 0xf3, 0x80, 0x11, 0xe0, 0x54, 0xfb, 
    0xf0, 0x80, 0x14, 0xe5, 0x3c, 0xf4, 0x90, 0x13, 0x2a, 0x60, 0x08, 0xe0, 0x54, 0xf2, 0x45, 0x3c, 
    0xf0, 0x80, 0x04, 0xe0, 0x54, 0xfa, 0xf0, 0x90, 0x04, 0x01, 0xe0, 0x44, 0x10, 0xf0, 0x75, 0x8c, 
    0x80, 0xe0, 0x54, 0xfd, 0xf0, 0x90, 0x12, 0x04, 0xe0, 0x44, 0x04, 0xf0, 0xe5, 0x58, 0x30, 0xe0, 
    0x06, 0x90, 0x01, 0x0d, 0xe0, 0xf5, 0x24, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x40, 0x17, 0x20, 0x02, 
    0x14, 0x20, 0x03, 0x11, 0x30, 0x4f, 0x0e, 0x90, 0x01, 0x0d, 0xe0, 0x54, 0xfb, 0xf0, 0x90, 0x01, 
    0x0c, 0xe0, 0x54, 0xfd, 0xf0, 0x75, 0x30, 0x01, 0x75, 0x55, 0x02, 0xe4, 0xf5, 0x51, 0x80, 0x09, 
    0xe5, 0x50, 0x70, 0x05, 0x75, 0x30, 0x03, 0xf5, 0x51, 0xe5, 0x30, 0x60, 0x15, 0xc2, 0x01, 0xe4, 
    0xf5, 0x51, 0xc2, 0x59, 0xad, 0x30, 0xaf, 0x40, 0x12, 0x0d, 0x42, 0xe5, 0x30, 0xb4, 0x03, 0x02, 
    0xd2, 0x03, 0xd2, 0xaf, 0x22, 0xc2, 0xaf, 0x30, 0x01, 0x0e, 0xe4, 0xf5, 0x51, 0xc2, 0x59, 0xc2, 
    0x01, 0x7d, 0x02, 0xaf, 0x40, 0x12, 0x0d, 0x42, 0xe5, 0x52, 0x14, 0x60, 0x48, 0x14, 0x60, 0x25, 
    0x24, 0x02, 0x60, 0x03, 0x02, 0x0a, 0x5b, 0xe5, 0x25, 0xd3, 0x94, 0x01, 0x40, 0x11, 0x90, 0x01, 
    0x0c, 0xe0, 0x44, 0x02, 0xf0, 0xa3, 0xe0, 0x44, 0x04, 0xf0, 0x7f, 0x0a, 0x12, 0x0e, 0xe9, 0x75, 
    0x52, 0x02, 0x75, 0x55, 0x03, 0xe5, 0x58, 0x30, 0xe0, 0x06, 0x90, 0x01, 0x0d, 0xe5, 0x24, 0xf0, 
    0x90, 0x12, 0x04, 0xe0, 0x54, 0xfb, 0xf0, 0x7f, 0x20, 0x12, 0x00, 0x0e, 0x75, 0x52, 0x01, 0x75, 
    0x55, 0x03, 0x02, 0x0a, 0x5b, 0xe5, 0x54, 0x60, 0x03, 0x02, 0x0a, 0x5b, 0x90, 0x04, 0x01, 0xe0, 
    0x44, 0x0e, 0xf0, 0xe0, 0x54, 0xef, 0xf0, 0xe4, 0xf5, 0x8c, 0xe5, 0x58, 0x54, 0x18, 0x60, 0x1e, 
    0x90, 0x70, 0x1a, 0xe0, 0x90, 0x13, 0x28, 0xf0, 0x90, 0x70, 0x1b, 0xe0, 0x90, 0x13, 0x29, 0xf0, 
    0xa3, 0x74, 0x05, 0xf0, 0x90, 0x70, 0x22, 0xe0, 0x90, 0x13, 0x2b, 0xf0, 0x80, 0x11, 0x90, 0x13, 
    0x28, 0xe0, 0x44, 0x0f, 0xf0, 0xa3, 0xe0, 0x44, 0x0f, 0xf0, 0xa3, 0xe0, 0x44, 0x05, 0xf0, 0x90, 
    0x12, 0x04, 0x74, 0x03, 0xf0, 0xe5, 0x58, 0x30, 0xe3, 0x1d, 0x90, 0x00, 0x02, 0xe0, 0x30, 0xe0, 
    0x16, 0x90, 0x05, 0x00, 0x74, 0xe2, 0xf0, 0xa3, 0x74, 0x08, 0xf0, 0xa3, 0x74, 0x01, 0xf0, 0x74, 
    0x03, 0xf0, 0x7f, 0x01, 0x12, 0x0e, 0xe9, 0x90, 0x02, 0x08, 0xe0, 0x44, 0x05, 0xf0, 0x90, 0x10, 
    0x04, 0xe0, 0x44, 0x0c, 0xf0, 0xe4, 0xf5, 0x52, 0xf5, 0x55, 0x30, 0x02, 0x09, 0xc2, 0x02, 0x7d, 
    0x01, 0xaf, 0x41, 0x12, 0x0d, 0x42, 0x30, 0x03, 0x02, 0xc2, 0x03, 0xd2, 0xaf, 0x22, 0xc2, 0x4b, 
    0xc2, 0x4c, 0xe5, 0x44, 0x12, 0x0d, 0xc5, 0x0a, 0x80, 0x00, 0x0b, 0x0e, 0x04, 0x0b, 0x0a, 0x08, 
    0x0a, 0xea, 0x10, 0x0a, 0x94, 0x20, 0x0a, 0xb4, 0x60, 0x0a, 0xc5, 0xa0, 0x00, 0x00, 0x0b, 0x10, 
    0x85, 0x48, 0x43, 0x85, 0x4a, 0x42, 0x85, 0x4c, 0x5e, 0xe5, 0x47, 0x64, 0x06, 0x60, 0x03, 0x02, 
    0x0b, 0x10, 0x80, 0x1b, 0xe5, 0x48, 0xc4, 0x54, 0x0f, 0xf5, 0x43, 0xe5, 0x4a, 0xc4, 0x54, 0x0f, 
    0xf5, 0x42, 0xe5, 0x4c, 0xc4, 0x54, 0x0f, 0xf5, 0x5e, 0xe5, 0x47, 0x64, 0x06, 0x70, 0x61, 0x53, 
    0x43, 0x0f, 0x80, 0x5c, 0x85, 0x49, 0x43, 0x85, 0x4b, 0x42, 0x85, 0x4d, 0x5e, 0xe5, 0x47, 0x64, 
    0x06, 0x70, 0x4d, 0x80, 0x1b, 0xe5, 0x49, 0xc4, 0x54, 0x0f, 0xf5, 0x43, 0xe5, 0x4b, 0xc4, 0x54, 
    0x0f, 0xf5, 0x42, 0xe5, 0x4d, 0xc4, 0x54, 0x0f, 0xf5, 0x5e, 0xe5, 0x47, 0x64, 0x06, 0x70, 0x30, 
    0xe5, 0x43, 0x54, 0x0f, 0x44, 0x10, 0xf5, 0x43, 0x80, 0x26, 0xe5, 0x47, 0x64, 0x04, 0x60, 0x05, 
    0xe5, 0x47, 0xb4, 0x05, 0x06, 0x43, 0x5e, 0x04, 0x75, 0x42, 0x09, 0xe5, 0x47, 0xb4, 0x06, 0x10, 
    0xe5, 0x43, 0x54, 0x0f, 0x44, 0x30, 0xf5, 0x43, 0x80, 0x06, 0xd2, 0x4b, 0x80, 0x02, 0xd2, 0x4c, 
    0xe4, 0xf5, 0x2a, 0xe5, 0x42, 0xc4, 0x54, 0xf0, 0xff, 0xe5, 0x43, 0x54, 0x0f, 0x4f, 0xf5, 0x5f, 
    0xd2, 0x60, 0x22, 0xc0, 0xe0, 0xc0, 0xf0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0xd0, 0x75, 0xd0, 0x18, 
    0xc2, 0xaf, 0x30, 0x45, 0x03, 0x12, 0x0f, 0x57, 0x90, 0x04, 0x16, 0xe0, 0x30, 0xe3, 0x03, 0x74, 
    0x08, 0xf0, 0x90, 0x04, 0x14, 0xe0, 0x20, 0xe7, 0x03, 0x02, 0x0b, 0xd0, 0x74, 0x80, 0xf0, 0x90, 
    0x70, 0x12, 0xe0, 0xf5, 0x2f, 0x90, 0x04, 0x04, 0xe0, 0x24, 0xcf, 0x60, 0x30, 0x14, 0x60, 0x42, 
    0x24, 0xe2, 0x60, 0x47, 0x14, 0x60, 0x55, 0x24, 0x21, 0x70, 0x60, 0xe5, 0x55, 0x24, 0xfe, 0x60, 
    0x07, 0x14, 0x60, 0x08, 0x24, 0x02, 0x70, 0x08, 0x7d, 0x01, 0x80, 0x28, 0x7d, 0x02, 0x80, 0x24, 
    0x90, 0x70, 0x10, 0xe0, 0xf5, 0x50, 0x85, 0x2f, 0x40, 0xd2, 0x01, 0x80, 0x3e, 0xe5, 0x55, 0x64, 
    0x03, 0x60, 0x04, 0xe5, 0x55, 0x70, 0x04, 0x7d, 0x02, 0x80, 0x09, 0x85, 0x2f, 0x41, 0xd2, 0x02, 
    0x80, 0x29, 0xad, 0x55, 0xaf, 0x2f, 0x12, 0x0d, 0xeb, 0x80, 0x20, 0x90, 0x70, 0x10, 0xe0, 0xf5, 
    0x47, 0x90, 0x70, 0x11, 0xe0, 0xf5, 0x44, 0x12, 0x0f, 0x48, 0x80, 0x06, 0x90, 0x70, 0x10, 0xe0, 
    0xf5, 0x45, 0xe4, 0xfd, 0xaf, 0x2f, 0x12, 0x0d, 0xeb, 0xd2, 0x04, 0x90, 0x70, 0x13, 0xe4, 0xf0, 
    0x90, 0x70, 0x13, 0xe4, 0xf0, 0xd2, 0xaf, 0xd0, 0xd0, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xf0, 0xd0, 
    0xe0, 0x32, 0xc0, 0xe0, 0xc0, 0xf0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0xd0, 0xe8, 0xc0, 0xe0, 0xe9, 
    0xc0, 0xe0, 0xea, 0xc0, 0xe0, 0xeb, 0xc0, 0xe0, 0xec, 0xc0, 0xe0, 0xed, 0xc0, 0xe0, 0xee, 0xc0, 
    0xe0, 0xef, 0xc0, 0xe0, 0xc2, 0xaf, 0x30, 0x45, 0x03, 0x12, 0x0f, 0x60, 0xd2, 0xaf, 0xd0, 0xe0, 
    0xff, 0xd0, 0xe0, 0xfe, 0xd0, 0xe0, 0xfd, 0xd0, 0xe0, 0xfc, 0xd0, 0xe0, 0xfb, 0xd0, 0xe0, 0xfa, 
    0xd0, 0xe0, 0xf9, 0xd0, 0xe0, 0xf8, 0xd0, 0xd0, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xf0, 0xd0, 0xe0, 
    0x32, 0xc0, 0xe0, 0xc0, 0xf0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0xd0, 0x75, 0xd0, 0x10, 0xc2, 0xaf, 
    0x30, 0x45, 0x03, 0x12, 0x0f, 0x5a, 0x30, 0x58, 0x0a, 0xe5, 0x54, 0x60, 0x04, 0x15, 0x54, 0x80, 
    0x02, 0xc2, 0x58, 0x30, 0x59, 0x0a, 0xe5, 0x50, 0x60, 0x04, 0x15, 0x50, 0x80, 0x02, 0xc2, 0x59, 
    0xd5, 0x53, 0x07, 0x30, 0x60, 0x04, 0x15, 0x46, 0xd2, 0x04, 0x30, 0x45, 0x03, 0x12, 0x0f, 0x5d, 
    0xc2, 0x8d, 0xd2, 0xaf, 0xd0, 0xd0, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xf0, 0xd0, 0xe0, 0x32, 0x90, 
    0x70, 0x2a, 0xe0, 0x30, 0xe1, 0x43, 0xc2, 0xaf, 0x90, 0x70, 0x28, 0xe0, 0x90, 0x10, 0x1c, 0xf0, 
    0x90, 0x70, 0x29, 0xe0, 0x90, 0x10, 0x1d, 0xf0, 0x90, 0x70, 0x2a, 0xe0, 0x90, 0x10, 0x1e, 0xf0, 
    0x90, 0x10, 0x1c, 0xe0, 0xf5, 0x30, 0x90, 0x10, 0x1e, 0xe0, 0x20, 0xe1, 0xf3, 0x90, 0x10, 0x1c, 
    0xe0, 0x90, 0x70, 0x28, 0xf0, 0x90, 0x10, 0x1d, 0xe0, 0x90, 0x70, 0x29, 0xf0, 0x90, 0x10, 0x1e, 
    0xe0, 0x90, 0x70, 0x2a, 0xf0, 0xc2, 0x05, 0xd2, 0xaf, 0x22, 0x12, 0x0e, 0x8c, 0x30, 0x45, 0x03, 
    0x12, 0x0f, 0x51, 0x30, 0x01, 0x06, 0x20, 0x09, 0x03, 0x12, 0x0f, 0x4b, 0x30, 0x02, 0x06, 0x20, 
    0x0a, 0x03, 0x12, 0x0f, 0x4e, 0x30, 0x03, 0x06, 0x20, 0x0b, 0x03, 0x12, 0x0f, 0x4e, 0x30, 0x04, 
    0x06, 0x20, 0x0c, 0x03, 0x12, 0x00, 0x16, 0x20, 0x13, 0x09, 0x20, 0x11, 0x06, 0xe5, 0x2b, 0x45, 
    0x2c, 0x60, 0x03, 0xd3, 0x80, 0x01, 0xc3, 0x92, 0xa9, 0x12, 0x0e, 0xd5, 0x80, 0xbf, 0x30, 0x14, 
    0x30, 0x90, 0x70, 0x19, 0xe0, 0x55, 0x27, 0xff, 0x90, 0x70, 0x18, 0xe0, 0x4f, 0xf5, 0x27, 0x90, 
    0x02, 0x29, 0xe0, 0xff, 0x90, 0x70, 0x19, 0xe0, 0xfe, 0xef, 0x5e, 0x90, 0x02, 0x29, 0xf0, 0x30, 
    0x47, 0x04, 0xaf, 0x27, 0x80, 0x04, 0xe5, 0x27, 0xf4, 0xff, 0x90, 0x02, 0x28, 0xef, 0xf0, 0xc2, 
    0x14, 0x22, 0xef, 0xf4, 0x60, 0x2d, 0xe4, 0xfe, 0x74, 0x14, 0x2e, 0xf5, 0x82, 0xe4, 0x34, 0x70, 
    0xf5, 0x83, 0xe0, 0xb4, 0xff, 0x19, 0x74, 0x14, 0x2e, 0xf5, 0x82, 0xe4, 0x34, 0x70, 0xf5, 0x83, 
    0xef, 0xf0, 0x74, 0x1c, 0x2e, 0xf5, 0x82, 0xe4, 0x34, 0x70, 0xf5, 0x83, 0xed, 0xf0, 0x22, 0x0e, 
    0xbe, 0x04, 0xd5, 0x22, 0xe5, 0x47, 0xb4, 0x0b, 0x10, 0x90, 0x02, 0x29, 0xe0, 0x54, 0xeb, 0xf0, 
    0xe5, 0x27, 0x54, 0xeb, 0x45, 0x45, 0xf5, 0x27, 0x22, 0xe4, 0x90, 0x02, 0x29, 0xf0, 0x30, 0x47, 
    0x04, 0xaf, 0x45, 0x80, 0x04, 0xe5, 0x45, 0xf4, 0xff, 0x90, 0x02, 0x28, 0xef, 0xf0, 0x22, 0xc2, 
    0x43, 0xd2, 0x45, 0xe4, 0xf5, 0x20, 0xf5, 0x21, 0xf5, 0x53, 0xf5, 0x46, 0xf5, 0x2b, 0xf5, 0x2c, 
    0xc2, 0x42, 0xf5, 0x51, 0xf5, 0x52, 0xf5, 0x55, 0x90, 0x04, 0x18, 0x74, 0x80, 0xf0, 0x90, 0x04, 
    0x1a, 0x74, 0x08, 0xf0, 0x22, 0xd0, 0x83, 0xd0, 0x82, 0xf8, 0xe4, 0x93, 0x70, 0x12, 0x74, 0x01, 
    0x93, 0x70, 0x0d, 0xa3, 0xa3, 0x93, 0xf8, 0x74, 0x01, 0x93, 0xf5, 0x82, 0x88, 0x83, 0xe4, 0x73, 
    0x74, 0x02, 0x93, 0x68, 0x60, 0xef, 0xa3, 0xa3, 0xa3, 0x80, 0xdf, 0xef, 0xf4, 0x60, 0x1f, 0xe4, 
    0xfe, 0x12, 0x0f, 0x28, 0xe0, 0xb4, 0xff, 0x12, 0x12, 0x0f, 0x28, 0xef, 0xf0, 0x74, 0x1c, 0x2e, 
    0xf5, 0x82, 0xe4, 0x34, 0x70, 0xf5, 0x83, 0xed, 0xf0, 0x22, 0x0e, 0xbe, 0x04, 0xe3, 0x22, 0xc0, 
    0xe0, 0xc0, 0xf0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0xd0, 0x75, 0xd0, 0x08, 0xc2, 0xaf, 0x30, 0x45, 
    0x03, 0x12, 0x0f, 0x54, 0xd2, 0xaf, 0xd0, 0xd0, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xf0, 0xd0, 0xe0, 
    0x32, 0xd2, 0x15, 0xe5, 0x47, 0x24, 0xf5, 0x60, 0x0b, 0x24, 0xcb, 0x60, 0x07, 0x24, 0x40, 0x70, 
    0x06, 0xc2, 0x15, 0x22, 0x12, 0x0d, 0x74, 0x12, 0x04, 0x92, 0xc2, 0x15, 0xc2, 0xaf, 0xc2, 0x04, 
    0xd2, 0xaf, 0x22, 0xe5, 0x53, 0x70, 0x1a, 0x30, 0x60, 0x09, 0xb2, 0x4d, 0x30, 0x4d, 0x04, 0x05, 
    0x46, 0xc2, 0x04, 0xe5, 0x4f, 0x45, 0x4e, 0x60, 0x08, 0xe5, 0x4f, 0x15, 0x4f, 0x70, 0x02, 0x15, 
    0x4e, 0x22, 0x90, 0x10, 0x1c, 0xed, 0xf0, 0xa3, 0xef, 0xf0, 0xa3, 0x74, 0x0a, 0xf0, 0x90, 0x10, 
    0x1c, 0xe0, 0xf5, 0x58, 0x90, 0x10, 0x1e, 0xe0, 0x20, 0xe1, 0xf3, 0x22, 0xc2, 0xaf, 0x12, 0x0e, 
    0xff, 0x12, 0x0d, 0x9f, 0x12, 0x0e, 0xbe, 0xe4, 0xf5, 0x22, 0xf5, 0x47, 0x90, 0x04, 0x00, 0x74, 
    0x80, 0xf0, 0xd2, 0xaf, 0x22, 0x90, 0x10, 0x1d, 0xef, 0xf0, 0xa3, 0x74, 0x0b, 0xf0, 0x90, 0x10, 
    0x1c, 0xe0, 0xf5, 0x58, 0x90, 0x10, 0x1e, 0xe0, 0x20, 0xe1, 0xf3, 0xaf, 0x58, 0x22, 0x75, 0x89, 
    0x02, 0xe4, 0xf5, 0x8c, 0xf5, 0x8a, 0xf5, 0x88, 0xf5, 0xb8, 0xf5, 0xe8, 0x75, 0x90, 0x18, 0xd2, 
    0x8c, 0x75, 0xa8, 0x05, 0x22, 0x30, 0x45, 0x03, 0x12, 0x0f, 0x63, 0xe5, 0x20, 0x70, 0x03, 0x20, 
    0x10, 0x03, 0x30, 0x11, 0x03, 0x43, 0x87, 0x01, 0x22, 0xce, 0xef, 0xce, 0xee, 0x60, 0x08, 0x7f, 
    0xff, 0x12, 0x0e, 0xf8, 0x1e, 0x80, 0xf5, 0x22, 0xef, 0x60, 0x03, 0x1f, 0x80, 0xfa, 0x22, 0x22, 
    0xc0, 0x26, 0x74, 0x0f, 0xc0, 0xe0, 0xc0, 0x82, 0xc0, 0x83, 0x75, 0x26, 0x0a, 0x22, 0xc0, 0x26, 
    0x74, 0x0f, 0xc0, 0xe0, 0xc0, 0x82, 0xc0, 0x83, 0x75, 0x26, 0x18, 0x22, 0x78, 0x7f, 0xe4, 0xf6, 
    0xd8, 0xfd, 0x75, 0x81, 0x5f, 0x02, 0x0c, 0xca, 0x74, 0x14, 0x2e, 0xf5, 0x82, 0xe4, 0x34, 0x70, 
    0xf5, 0x83, 0x22, 0xef, 0x90, 0x0f, 0x3c, 0x93, 0x90, 0x0f, 0x00, 0x73, 0x0a, 0x18, 0x30, 0x05, 
    0x06, 0x20, 0x0d, 0x03, 0x12, 0x0c, 0x7f, 0x22, 0x02, 0x0a, 0x5e, 0x02, 0x07, 0xee, 0x02, 0x09, 
    0x65, 0x02, 0x0f, 0x3e, 0x02, 0x0f, 0x67, 0x02, 0x00, 0x1e, 0x02, 0x0f, 0x68, 0x02, 0x0e, 0x53, 
    0x02, 0x0f, 0x69, 0x02, 0x0d, 0x0e, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0xac, 0x12, 
};


#define FIRMWAREIMAGE_LENGTH_4K                     (sizeof (FirmwareImage4K) / sizeof(UCHAR))
#endif
#define FIRMWARE_MAJOR_VERSION_4K     0
#define FIRMWARE_MINOR_VERSION_4K     34

const unsigned short ccitt_16Table[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
#define ByteCRC16(v, crc) \
	(unsigned short)((crc << 8) ^  ccitt_16Table[((crc >> 8) ^ (v)) & 255])

unsigned char BitReverse(unsigned char x)
{
	int i;
	unsigned char Temp=0;
	for(i=0; ; i++)
	{
		if(x & 0x80)	Temp |= 0x80;
		if(i==7)		break;
		x	<<= 1;
		Temp >>= 1;
	}
	return Temp;
}

// Unify all delay routine by using NdisStallExecution
/*
__inline    VOID    RTMPusecDelay(
    IN      ULONG   usec)
{
	ULONG   i;

	for (i = 0; i < (usec / 50); i++)
		KeStallExecutionProcessor(50);

	if (usec % 50)
		KeStallExecutionProcessor(usec % 50);
}
*/

//#pragma code_seg("INIT") // start INIT section

//++
// Function:	DriverEntry
//
// Description:
//		Initializes the driver.
//
// Arguments:
//		pDriverObject - Passed from I/O Manager
//		pRegistryPath - UNICODE_STRING pointer to
//						registry info (service key)
//						for this driver
//
// Return value:
//		NTSTATUS signaling success or failure
//--
NTSTATUS DriverEntry (
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath	
	) 
{
	ULONG ulDeviceNumber = 0;
	NTSTATUS status = STATUS_SUCCESS;	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> DriverEntry\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("================================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Build Date: "__DATE__" Time: "__TIME__"\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("================================================\n"));

#ifdef RTMP_USB_SUPPORT

	//pDriverObject->MajorFunction[IRP_MJ_CLEANUP]        = BulkUsb_DispatchClean;
	//pDriverObject->MajorFunction[IRP_MJ_READ]           =
   	//pDriverObject->MajorFunction[IRP_MJ_WRITE]          = BulkUsb_DispatchReadWrite;
	pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = BulkUsb_DispatchSysCtrl;
#endif

	// Announce other driver entry points
	pDriverObject->DriverUnload = DriverUnload;

	pDriverObject->DriverStartIo = StartIo;

	// Announce the PNP AddDevice entry point
	pDriverObject->DriverExtension->AddDevice =	AddDevice;

	// Announce the PNP Major Function entry point
	pDriverObject->MajorFunction[IRP_MJ_PNP] = DispPnp;

	// This includes Dispatch routines for Create, Write & Read
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
       pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

	pDriverObject->MajorFunction[IRP_MJ_POWER] = Wdm2Power; //DispPower;

	// Notice that no device objects are created by DriverEntry.
	// Instead, we await the PnP call to AddDevice

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: DriverEntry <=== \n"));

	return status;
}
//#pragma code_seg() // end INIT section

//++
// Function:	AddDevice
//
// Description:
//	Called by the PNP Manager when a new device is
//	detected on a bus.  The responsibilities include
//	creating an FDO, device name, and symbolic link.
//
// Arguments:
//	pDriverObject - Passed from PNP Manager
//	pdo		    - pointer to Physcial Device Object
//				 passed from PNP Manager
//
// Return value:
//	NTSTATUS signaling success or failure
//--
NTSTATUS AddDevice (	IN PDRIVER_OBJECT pDriverObject,
						IN PDEVICE_OBJECT pDo	) 
{
	NTSTATUS status = 0;
	PDEVICE_OBJECT pfdo;
	RTMP_ADAPTER *pDevExt;
	static int ulDeviceNumber = 0;
	UCHAR				resBuf[CM_RESOURCE_BUF_SIZE];
	PCM_RESOURCE_LIST	resList = (PCM_RESOURCE_LIST)resBuf;
	ULONG				bufSize = CM_RESOURCE_BUF_SIZE;
	PCM_FULL_RESOURCE_DESCRIPTOR  	pResDesc;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR	pPartialResDesc;
	ULONG				ResultbutSize;
	ULONG				index,inner;
#ifdef RTMP_USB_SUPPORT
	RTMP_ADAPTER *deviceExtension;
    	POWER_STATE       state;
    	KIRQL             oldIrql;

	UNREFERENCED_PARAMETER( oldIrql );
#endif

	g_FWSeqUI = 0;
	g_FWSeqMCU = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: sys minusPCICSR0 AddDevice ===> \n"));

//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"PciDumpr: AddDevice; current DeviceNumber = %d\n",
//				ulDeviceNumber);


#ifdef RTMP_USB_SUPPORT
	// TODO: FILE_DEVICE_SECURE_OPEN or FILE_AUTOGENERATED_DEVICE_NAME
	//
	status = IoCreateDevice(
					pDriverObject,                   // our driver object
					sizeof(RTMP_ADAPTER),//
					NULL,                           // name for this device
					FILE_DEVICE_UNKNOWN,
					FILE_DEVICE_SECURE_OPEN, //FILE_AUTOGENERATED_DEVICE_NAME, // device characteristics
					FALSE,                          // Not exclusive
					&pfdo);   

	RtlZeroMemory(pfdo->DeviceExtension, sizeof(RTMP_ADAPTER));

    //
    // Initialize the device extension
    //

    deviceExtension = (RTMP_ADAPTER *) pfdo->DeviceExtension;
    deviceExtension->FunctionalDeviceObject = pfdo;
    deviceExtension->PhysicalDeviceObject = pDo;
#if 0	
    RtlZeroMemory(&deviceExtension->OtherCounters, sizeof(deviceExtension->OtherCounters));
    // default buffer mode
    deviceExtension->EfuseMode= MODE_BUFFER;
    RtlZeroMemory(&deviceExtension->EfuseContent, EFUSESIZE);
    deviceExtension->IsTxSetFrequency = FALSE;
	//FW
	deviceExtension->LoadingFWCount = 0;
	RtlZeroMemory(&deviceExtension->FWImage, MAXFIRMWARESIZE);
	deviceExtension->FWSize = 0;
	deviceExtension->IsFWImageInUse = FALSE;
	deviceExtension->IsUISetFW = FALSE;
	deviceExtension->bIsWaitFW = FALSE;
	deviceExtension->MACVersion = 0;
	deviceExtension->FwCmdSeqNum = 0;
	deviceExtension->RomPatchSemStatus = 0xFF;
	RtlZeroMemory(&deviceExtension->ScatterBuf, SCATTER_SIZE);
	deviceExtension->FWMode = FWROM;
	deviceExtension->FWon = FALSE;
#endif	
	
    pfdo->Flags |= DO_DIRECT_IO;

    //
    // initialize the device state lock and set the device state
    //

    KeInitializeSpinLock(&deviceExtension->DevStateLock);
    INITIALIZE_PNP_STATE(deviceExtension);

    //
    //initialize OpenHandleCount
    //
    deviceExtension->OpenHandleCount = 0;

    //
    // Initialize the selective suspend variables
    //
    KeInitializeSpinLock(&deviceExtension->IdleReqStateLock);
    deviceExtension->IdleReqPend = 0;
    deviceExtension->PendingIdleIrp = NULL;

    //
    // Hold requests until the device is started
    //

    deviceExtension->QueueState = HoldRequests;

    //
    // Initialize the queue and the queue spin lock
    //

    InitializeListHead(&deviceExtension->NewRequestsQueue);
    KeInitializeSpinLock(&deviceExtension->QueueLock);

    //
    // Initialize the remove event to not-signaled.
    //

    KeInitializeEvent(&deviceExtension->RemoveEvent, 
                      SynchronizationEvent, 
                      FALSE);

    //
    // Initialize the stop event to signaled.
    // This event is signaled when the OutstandingIO becomes 1
    //

    KeInitializeEvent(&deviceExtension->StopEvent, 
                      SynchronizationEvent, 
                      TRUE);

	//
	// Initialize the NoIdleReqPendEvent to ensure that the idle request
	// is indeed complete before we unload the drivers.
	//
	KeInitializeEvent(&deviceExtension->NoIdleReqPendEvent,
				  NotificationEvent,
				  TRUE);

    //
    // OutstandingIo count biased to 1.
    // Transition to 0 during remove device means IO is finished.
    // Transition to 1 means the device can be stopped
    //

    deviceExtension->OutStandingIO = 1;
    KeInitializeSpinLock(&deviceExtension->IOCountLock);

    //
    // Delegating to WMILIB
    //:TODO:   don't do this???
    //ntStatus = BulkUsb_WmiRegistration(deviceExtension);

    //if(!NT_SUCCESS(ntStatus)) {
//	 MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"BulkUsb_WmiRegistration failed with %X\n", ntStatus);
    //    IoDeleteDevice(deviceObject);
    //    return ntStatus;
    //}

    //
    // set the flags as underlying PDO
    //

    if(pDo->Flags & DO_POWER_PAGABLE) {

        pfdo->Flags |= DO_POWER_PAGABLE;
    }

    //
    // Typically, the function driver for a device is its 
    // power policy owner, although for some devices another 
    // driver or system component may assume this role. 
    // Set the initial power state of the device, if known, by calling 
    // PoSetPowerState.
    // 

    deviceExtension->DevPower = PowerDeviceD0;
    deviceExtension->SysPower = PowerSystemWorking;

    state.DeviceState = PowerDeviceD0;
    PoSetPowerState(pfdo, DevicePowerState, state);

    //
    // attach our driver to device stack
    // The return value of IoAttachDeviceToDeviceStack is the top of the
    // attachment chain.  This is where all the IRPs should be routed.
    //

    deviceExtension->TopOfStackDeviceObject = 
                IoAttachDeviceToDeviceStack(pfdo,
                                            pDo);
    //:TODO:   don't do this???
    //if(NULL == deviceExtension->TopOfStackDeviceObject) {
//
    //    BulkUsb_WmiDeRegistration(deviceExtension);
    //    IoDeleteDevice(deviceObject);
    //    return STATUS_NO_SUCH_DEVICE;
    //}
        
    //
    // Register device interfaces
    //

    status = IoRegisterDeviceInterface(deviceExtension->PhysicalDeviceObject, 
                                         &GUID_QA_MT7603U, 
                                         NULL, 
                                         &deviceExtension->InterfaceName);

    if(!NT_SUCCESS(status)) {

        //BulkUsb_WmiDeRegistration(deviceExtension); //:TODO:   don't do this???
        IoDetachDevice(deviceExtension->TopOfStackDeviceObject);
        IoDeleteDevice(pfdo);
        return status;
    }
/*
	//event for load fw
	KeInitializeEvent(&deviceExtension->LoadFWEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled
	deviceExtension->bLoadingFW = FALSE;
	//event for fw bulkout
	KeInitializeEvent(&deviceExtension->WaitFWEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled
	
	//event for Rom Patch bulkout
	KeInitializeEvent(&deviceExtension->RomPatchEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled	
*/                      
	status = InitAdapterEvent(deviceExtension);
	status = InitAdapterCommon(deviceExtension);
	//SubmitIdleRequestIrp(pAdapter);
#else
	//PCIe
	// Now create the device
	status = IoCreateDevice( 
						pDriverObject,
						sizeof(RTMP_ADAPTER),
						NULL,
						FILE_DEVICE_UNKNOWN,
						0, 
						FALSE,
						&pfdo 
						);

	if (!NT_SUCCESS(status))
		return status;

	// Choose to use BUFFERED_IO
	pfdo->Flags |= DO_BUFFERED_IO;

	// Initialize the Device Extension
	pDevExt = (RTMP_ADAPTER *)pfdo->DeviceExtension;
	pDevExt->pDeviceObject = pfdo;	// back pointer
	
	pDevExt->pPhyDeviceObj = pDo;
	
	pDevExt->DeviceNumber = ulDeviceNumber;
	pDevExt->pIntObj = NULL;
	pDevExt->state = Stopped;
	pDevExt->EfuseMode = MODE_BUFFER;
	RtlZeroMemory(&pDevExt->EfuseContent, EFUSESIZE);
	pDevExt->IsTxSetFrequency = FALSE;
	pDevExt->bRxEnable[BULKIN_PIPENUMBER_0] = FALSE;
	pDevExt->bRxEnable[BULKIN_PIPENUMBER_1] = FALSE;
	pDevExt->bBulkOutRunning[BULKOUT_PIPENUMBER_0] = FALSE;
	pDevExt->bBulkOutRunning[BULKOUT_PIPENUMBER_1] = FALSE;
	pDevExt->bBulkOutRunning[BULKOUT_PIPENUMBER_2] = FALSE;
	pDevExt->bBulkOutRunning[BULKOUT_PIPENUMBER_3] = FALSE;
	pDevExt->bBulkOutRunning[BULKOUT_PIPENUMBER_4] = FALSE;
	pDevExt->bBulkOutRunning[BULKOUT_PIPENUMBER_5] = FALSE;

	pDevExt->ulBulkInRunning[0] = 0;
	pDevExt->ulBulkInRunning[1] = 0;
	
	//-----------------------------------------	
	// Initialize Rx Spin Lock
	KeInitializeSpinLock(&pDevExt->RxSpinLock[0]);
	KeInitializeSpinLock(&pDevExt->RxSpinLock[1]);	
	//-----------------------------------------	
	
	// Register and enable our device interface
	status = IoRegisterDeviceInterface(pDo, &GUID_QA_MT7603E, NULL, &pDevExt->ustrSymLinkName);
	if( !NT_SUCCESS(status))
	{
		IoDeleteDevice(pfdo);
		return status;
	}

	// Form the symbolic link name
	IoSetDeviceInterfaceState(&pDevExt->ustrSymLinkName, TRUE);

	// Pile this new fdo on top of the existing lower stack
	pDevExt->pLowerDevice =		// downward pointer
		IoAttachDeviceToDeviceStack( pfdo, pDo);

	if (!pDevExt->pLowerDevice) 
	{
		// if it fails now, must delete Device object
		IoDeleteDevice( pfdo );
		return status;
	}

	status=IoGetDeviceProperty(
			pDo,
			DevicePropertyBootConfigurationTranslated ,
			bufSize,
			resList,			
			&ResultbutSize
    );
	if (status==STATUS_BUFFER_TOO_SMALL)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: STATUS_BUFFER_TOO_SMALL \n") );
	}
	else if (status==STATUS_INVALID_PARAMETER_2)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: STATUS_INVALID_PARAMETER_2 \n") );
	}
	else if (status==STATUS_INVALID_DEVICE_REQUEST)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: STATUS_INVALID_DEVICE_REQUEST \n") );
	}
	else if( NT_SUCCESS(status))	
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: STATUS=%d \n",status));
	}
	else if( !NT_SUCCESS(status))	
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: !NT_SUCCESS(status)=%d \n",status));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: resList->Count=%d \n",resList->Count));

	// We need a DpcForIsr registration
	IoInitializeDpcRequest( pfdo, RTPCIDpcForIsr );

	

	// Made it
	ulDeviceNumber++;

	status = STATUS_SUCCESS;
#endif

	//  Clear the Device Initializing bit since the FDO was created
	//  outside of DriverEntry.
	pfdo->Flags &= ~DO_DEVICE_INITIALIZING;
	

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: AddDevice <=== \n"));

	return status;
}


#ifdef RTMP_PCI_SUPPORT
NTSTATUS DispPower(IN PDEVICE_OBJECT pDO,
					IN PIRP pIrp ) 
{
	// obtain current IRP stack location
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)(pDO->DeviceExtension);
	PIO_STACK_LOCATION pIrpStack;
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: DispPower IRP: %d\n", pIrpStack->MinorFunction));

	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);

	return PoCallDriver(pDevExt->pLowerDevice, pIrp);
}

NTSTATUS PassDownPnP( IN RTMP_ADAPTER *pAd,
					IN PIRP pIrp ) 
{	
	//RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDO->DeviceExtension;

	IoSkipCurrentIrpStackLocation( pIrp );
	
	return IoCallDriver(pAd->pLowerDevice, pIrp);
}


NTSTATUS HandleStopDevice(	IN PDEVICE_OBJECT pDO,
							IN PIRP pIrp ) 
{
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDO->DeviceExtension;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: HandleStopDevice ===> \n"));

	pDevExt->state = Stopped;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: HandleStopDevice <=== \n"));
	return PassDownPnP(pDevExt, pIrp);
}
#endif /* RTMP_PCI_SUPPORT */


#ifdef RTMP_USB_SUPPORT
NTSTATUS DispPnpUSB(IN PDEVICE_OBJECT pDO,
					IN PIRP pIrp)
{
	 PIO_STACK_LOCATION irpStack;
    RTMP_ADAPTER *deviceExtension;
//    KEVENT             startDeviceEvent;
    NTSTATUS           ntStatus = 0;
	//
    // initialize variables
    //

    irpStack = IoGetCurrentIrpStackLocation(pIrp);
    deviceExtension = (RTMP_ADAPTER *)pDO->DeviceExtension;
	//
	// since the device is removed, fail the Irp.
	//
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("USB %s  MinorFunction=0x%x ==>\n",__FUNCTION__,irpStack->MinorFunction));
	if(USBRemoved == deviceExtension->DeviceState) 
	{

	        ntStatus = STATUS_DELETE_PENDING;

	        pIrp->IoStatus.Status = ntStatus;
	        pIrp->IoStatus.Information = 0;

	        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		return ntStatus;
	}
	BulkUsb_IoIncrement(deviceExtension);
	if(irpStack->MinorFunction == IRP_MN_START_DEVICE) 
	{
        	ASSERT(deviceExtension->IdleReqPend == 0);
	}
	else 
	{
		/*if(deviceExtension->SSEnable) 
		{
	            CancelSelectSuspend(deviceExtension);
		}*/
	}

	switch(irpStack->MinorFunction) 
	{

    		case IRP_MN_START_DEVICE:
			
		        ntStatus = HandleStartDevice(deviceExtension, pIrp);

		        break;

		case IRP_MN_QUERY_STOP_DEVICE:

		        //
		        // if we cannot stop the device, we fail the query stop irp
		        //

		        ntStatus = CanStopDevice(pDO, pIrp);

		        if(NT_SUCCESS(ntStatus)) {

		            ntStatus = HandleQueryStopDevice(pDO, pIrp);

		            return ntStatus;
		        }
		        break;

		case IRP_MN_CANCEL_STOP_DEVICE:

		        ntStatus = HandleCancelStopDevice(pDO, pIrp);

		        //BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 3, ("BulkUsb_DispatchPnP::IRP_MN_CANCEL_STOP_DEVICE::"));
		        BulkUsb_IoDecrement(deviceExtension);

		        return ntStatus;
		     
		case IRP_MN_STOP_DEVICE:

#ifdef RTMP_PCI_SUPPORT
			// TODO: How about USB device???
		        ntStatus = HandleStopDevice(pDO, pIrp);
#endif /* RTMP_PCI_SUPPORT */
			
		        //BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 3, ("BulkUsb_DispatchPnP::IRP_MN_STOP_DEVICE::"));
		        BulkUsb_IoDecrement(deviceExtension);

		        return ntStatus;

		case IRP_MN_QUERY_REMOVE_DEVICE:

		        //
		        // if we cannot remove the device, we fail the query remove irp
		        //
		        ntStatus = HandleQueryRemoveDevice(pDO, pIrp);

		        return ntStatus;

		case IRP_MN_CANCEL_REMOVE_DEVICE:

		        ntStatus = HandleCancelRemoveDevice(pDO, pIrp);

		        //BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 3, ("BulkUsb_DispatchPnP::IRP_MN_CANCEL_REMOVE_DEVICE::"));
		        BulkUsb_IoDecrement(deviceExtension);

		        return ntStatus;

		case IRP_MN_SURPRISE_REMOVAL:

		        ntStatus = HandleSurpriseRemoval(pDO, pIrp);

		        //BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 3, ("BulkUsb_DispatchPnP::IRP_MN_SURPRISE_REMOVAL::"));
		        BulkUsb_IoDecrement(deviceExtension);

		        return ntStatus;

		case IRP_MN_REMOVE_DEVICE:

		        ntStatus = HandleRemoveDevice(pDO, pIrp);

		        return ntStatus;

		case IRP_MN_QUERY_CAPABILITIES:

		        ntStatus = HandleQueryCapabilities(pDO, pIrp);

		        break;

		default:

		        IoSkipCurrentIrpStackLocation(pIrp);

		        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, pIrp);

		       MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BulkUsb_DispatchPnP::default::"));
		        BulkUsb_IoDecrement(deviceExtension);

		        return ntStatus;

		    } // switch

	//
	// complete request 
	//

	    pIrp->IoStatus.Status = ntStatus;
	    pIrp->IoStatus.Information = 0;

	    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	//
	// decrement count
	//
	   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BulkUsb_DispatchPnP::"));
	    BulkUsb_IoDecrement(deviceExtension);

	return ntStatus;

}
#endif /* _USB */


#ifdef RTMP_PCI_SUPPORT
NTSTATUS DispPnpPCI(	IN PDEVICE_OBJECT pDO,
					IN PIRP pIrp ) 
{
	// obtain current IRP stack location
	PIO_STACK_LOCATION pIrpStack;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) pDO->DeviceExtension;
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: Received PNP IRP: %d\n",
				pIrpStack->MinorFunction));


	switch (pIrpStack->MinorFunction) 
	{
	case IRP_MN_START_DEVICE:
		return HandleStartDevice(pDO, pIrp );
	case IRP_MN_STOP_DEVICE:
		return HandleStopDevice( pDO, pIrp );
	case IRP_MN_REMOVE_DEVICE:
		return HandleRemoveDevice( pDO, pIrp );
	default:
		// if not supported here, just pass it down
		return PassDownPnP(pAd, pIrp);
	}

	// all paths from the switch statement will "return"
	// the results of the handler invoked
}
#endif /* RTMP_PCI_SUPPORT */


NTSTATUS DispPnp(	IN PDEVICE_OBJECT pDO,
					IN PIRP pIrp ) 
{
#ifdef RTMP_USB_SUPPORT
	return DispPnpUSB(pDO, pIrp);
#endif /* RTMP_USB_SUPPORT */

#ifdef RTMP_PCI_SUPPORT
	return DispPnpPCI(pDO, pIrp);
#endif /* RTMP_PCI_SUPPORT */

	return 0;
}


NTSTATUS HandleStartDevice(	IN RTMP_ADAPTER *pAd,
							IN PIRP pIrp ) 
{
#ifdef RTMP_USB_SUPPORT
    KIRQL             oldIrql;
    KEVENT            startDeviceEvent;
    NTSTATUS          ntStatus = 0;
    //RTMP_ADAPTER *deviceExtension;
    LARGE_INTEGER     dueTime;

    //
    // initialize variables
    //
    //deviceExtension = (RTMP_ADAPTER *) pDO->DeviceExtension;
    pAd->UsbConfigurationDescriptor = NULL;
    pAd->UsbInterface = NULL;
    pAd->PipeContext = NULL;

    //
    // We cannot touch the device (send it any non pnp irps) until a
    // start device has been passed down to the lower drivers.
    // first pass the Irp down
    //

    KeInitializeEvent(&startDeviceEvent, NotificationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(pIrp);

    IoSetCompletionRoutine(pIrp, 
                           IrpCompletionRoutine, 
                           (PVOID)&startDeviceEvent, 
                           TRUE, 
                           TRUE, 
                           TRUE);

    ntStatus = IoCallDriver(pAd->TopOfStackDeviceObject, pIrp);

    if(ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(&startDeviceEvent, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);

        ntStatus = pIrp->IoStatus.Status;
    }
    if(!NT_SUCCESS(ntStatus)) 
    {
        BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 1, ("Lower drivers failed this Irp\n"));
        return ntStatus;
    }

    //
    // Read the device descriptor, configuration descriptor 
    // and select the interface descriptors
    //

    ntStatus = ReadandSelectDescriptors(pAd);

    if(!NT_SUCCESS(ntStatus)) 
    {

        //BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 1, ("ReadandSelectDescriptors failed\n"));
        return ntStatus;
    }

     //
    // enable the symbolic links for system components to open
    // handles to the device
    //

    ntStatus = IoSetDeviceInterfaceState(&pAd->InterfaceName, 
                                         TRUE);

    if(!NT_SUCCESS(ntStatus)) {

        BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 1, ("IoSetDeviceInterfaceState:enable:failed\n"));
        return ntStatus;
    }

    KeAcquireSpinLock(&pAd->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(pAd, USBWorking);
    pAd->QueueState = AllowRequests;

    KeReleaseSpinLock(&pAd->DevStateLock, oldIrql);

    //
    // initialize wait wake outstanding flag to false.
    // and issue a wait wake.
    
    pAd->FlagWWOutstanding = 0;
    pAd->FlagWWCancel = 0;
    pAd->WaitWakeIrp = NULL;
    
    /*if(deviceExtension->WaitWakeEnable) {

        IssueWaitWake(deviceExtension);
    }*/

    ProcessQueuedRequests(pAd);


	ntStatus = NICInit(pAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HandleStartDevice - ends\n"));

	return ntStatus;
#else
//PCIe

	PCM_RESOURCE_LIST				pResourceList;
	PCM_FULL_RESOURCE_DESCRIPTOR	pFullDescriptor;
	PCM_PARTIAL_RESOURCE_LIST		pPartialList;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialDescriptor;
	NTSTATUS						status;
	int								i, AddressCount = 0;
	// The stack location contains the Parameter info
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	//RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDO->DeviceExtension;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: HandleStartDevice ===> \n"));
	
	pResourceList = pIrpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
	pFullDescriptor = pResourceList->List;
	pPartialList = &pFullDescriptor->PartialResourceList;
	for (i=0; i<(int)pPartialList->Count; i++)
	{
		pPartialDescriptor = &pPartialList->PartialDescriptors[i];
		
		switch (pPartialDescriptor->Type) 
		{
		case CmResourceTypeInterrupt:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: CmResourceTypeInterrupt \n"));
			
			pAd->IRQL = (KIRQL)	pPartialDescriptor->u.Interrupt.Level;
			pAd->InterruptLevel = pPartialDescriptor->u.Interrupt.Level;
			pAd->Vector = pPartialDescriptor->u.Interrupt.Vector;
			pAd->Affinity = pPartialDescriptor->u.Interrupt.Affinity;			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: Claiming Interrupt Resources: "
					 "InterruptLevel=%d, IRQ=%d Vector=0x%03X Affinity=%X\n",
					 pAd->InterruptLevel ,pAd->IRQL, pAd->Vector, pAd->Affinity));
#ifdef MSI
			{
			//MSI
				// deviceExtension is a pointer to the driver's device extension. 
				//     deviceExtension->IntObj is a PKINTERRUPT.
				// deviceInterruptService is a pointer to the driver's InterruptService routine.
				// IntResource is a CM_PARTIAL_RESOURCE_DESCRIPTOR structure of either type CmResourceTypeInterrupt or CmResourceTypeMessageInterrupt.
				// PhysicalDeviceObject is a pointer to the device's PDO. 
				// ServiceContext is a pointer to driver-specified context for the ISR.

				RtlZeroMemory( &pAd->params, sizeof(IO_CONNECT_INTERRUPT_PARAMETERS) );
				pAd->params.Version = CONNECT_FULLY_SPECIFIED;
				pAd->params.FullySpecified.PhysicalDeviceObject = pAd->pPhyDeviceObj;
				pAd->params.FullySpecified.InterruptObject = (PKINTERRUPT*)&pAd->pConnectionContext;
				pAd->params.FullySpecified.ServiceRoutine = Isr;
				pAd->params.FullySpecified.ServiceContext = pAd;
				pAd->params.FullySpecified.FloatingSave = FALSE;
				pAd->params.FullySpecified.SpinLock = NULL;

				if (pPartialDescriptor->Flags & CM_RESOURCE_INTERRUPT_MESSAGE) 
				{
					// The resource is for a message-based interrupt. Use the u.MessageInterrupt.Translated member of IntResource.

					pAd->params.FullySpecified.Vector = pPartialDescriptor->u.MessageInterrupt.Translated.Vector;
					pAd->params.FullySpecified.Irql = (KIRQL)pPartialDescriptor->u.MessageInterrupt.Translated.Level;
					pAd->params.FullySpecified.SynchronizeIrql = (KIRQL)pPartialDescriptor->u.MessageInterrupt.Translated.Level;
					pAd->params.FullySpecified.ProcessorEnableMask = pPartialDescriptor->u.MessageInterrupt.Translated.Affinity;
				} 
				else 
				{
					// The resource is for a line-based interrupt. Use the u.Interrupt member of IntResource.

					pAd->params.FullySpecified.Vector = pPartialDescriptor->u.Interrupt.Vector;
					pAd->params.FullySpecified.Irql = (KIRQL)pPartialDescriptor->u.Interrupt.Level;
					pAd->params.FullySpecified.SynchronizeIrql = (KIRQL)pPartialDescriptor->u.Interrupt.Level;
					pAd->params.FullySpecified.ProcessorEnableMask = pPartialDescriptor->u.Interrupt.Affinity;
				}

				pAd->params.FullySpecified.InterruptMode = (pPartialDescriptor->Flags & CM_RESOURCE_INTERRUPT_LATCHED ? Latched : LevelSensitive);
				pAd->params.FullySpecified.ShareVector = (BOOLEAN)(pPartialDescriptor->ShareDisposition == CmResourceShareShared);
			}
#endif
			break;

		case CmResourceTypeDma:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n\n\nPciDumpr: CmResourceTypeDma\n\n\n"));
			break;

		case CmResourceTypePort:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: CmResourceTypePort \n"));			
			pAd->ExtendPciAddress.PciAddress[AddressCount].TranslateAddress = pPartialDescriptor->u.Port.Start.LowPart;
			pAd->ExtendPciAddress.PciAddress[AddressCount].ShareMemorySize = pPartialDescriptor->u.Port.Length;
			AddressCount ++;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: pDevExt->ExtendPciAddress.PciAddress[AddressCount].TranslateAddress = %x\n",pAd->ExtendPciAddress.PciAddress[AddressCount].TranslateAddress));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: pDevExt->ExtendPciAddress.PciAddress[AddressCount].ShareMemorySize = %x\n",pAd->ExtendPciAddress.PciAddress[AddressCount].ShareMemorySize));
			break;

		case CmResourceTypeMemory:			
			{
			PHYSICAL_ADDRESS PhyAddress = pPartialDescriptor->u.Memory.Start;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: CmResourceTypeMemory  call MmMapIoSpace\n"));			
			
			pAd->MemorySize = pPartialDescriptor->u.Memory.Length;
			
			pAd->virtualaddr = (PULONG)MmMapIoSpace(PhyAddress, pAd->MemorySize, MmNonCached);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPhyAddress = 0x%08x \n",PhyAddress));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\npDevExt->MemorySize = 0x%08x \n",pAd->MemorySize));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\npDevExt->virtualaddr = 0x%08x \n",pAd->virtualaddr));
			

			pAd->ExtendPciAddress.PciAddress[AddressCount].TranslateAddress = pPartialDescriptor->u.Memory.Start.LowPart;
			pAd->ExtendPciAddress.PciAddress[AddressCount].ShareMemorySize = pPartialDescriptor->u.Memory.Length;
			pAd->ExtendPciAddress.PciAddress[AddressCount].MapIoAddress = *pAd->virtualaddr;

			AddressCount ++;
			}
			break;
		}
	}
	pAd->ExtendPciAddress.Count = AddressCount;	
	
	// Create & connect to an Interrupt object
	if (pAd->IRQL == 0 )
	{
		return STATUS_BIOS_FAILED_TO_CONNECT_INTERRUPT;	
	}

#ifdef MSI
	if (pAd->bConnectedInterrupt == FALSE)
	{
		status = IoConnectInterruptEx(&pAd->params);
		
		//Save the type of interrup connected. We'll use this later when we need to disconnect from the interrupt.
		pAd->TypeOfInterruptVersion = pAd->params.Version;

		pAd->bConnectedInterrupt = TRUE;
	}
#else
	{
		status = IoConnectInterrupt(
				&pAd->pIntObj,				// the Interrupt object
				Isr,							// our ISR
				pAd,						// Service Context
				NULL,							// no spin lock
				pAd->Vector,				// vector
				pAd->IRQL,					// DIRQL
				pAd->IRQL,					// DIRQL
				LevelSensitive,					// Latched or LevelSensitive
				TRUE,							// Shared?
				pAd->Affinity,				// processors in an MP set
				FALSE							// save FP registers? 
				);
	}
#endif

	if(status == STATUS_INVALID_PARAMETER)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: STATUS_INVALID_PARAMETER \n"));
	}

	if(status == STATUS_INSUFFICIENT_RESOURCES)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: STATUS_INSUFFICIENT_RESOURCES \n"));
	}

	if (!NT_SUCCESS(status)) 
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: Interrupt connection failure: %X\n", status));
		return status;
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: Interrupt successfully connected\n"));

	AllocateDMAMemory(pAd);

#ifdef SNIFF
	pAd->pPacketBuf = ExAllocatePoolWithTag(
											NonPagedPool,
											(RX_RING_PACKET_BUFFER * MAX_FRAME_SIZE),
											'yroR');
	if(pAd->pPacketBuf)
		RtlZeroMemory(pAd->pPacketBuf,(RX_RING_PACKET_BUFFER * MAX_FRAME_SIZE));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: Error! PacketBuf not allocated\n"));	
#endif

	// init comTxBuffer
	pAd->comTxBuffer.pComTxBuf = (PUCHAR)ExAllocatePoolWithTag(
											NonPagedPool,
											MAX_COM_BUFFER_SIZE,
											'yroR');
	if(pAd->comTxBuffer.pComTxBuf)
		RtlZeroMemory(pAd->comTxBuffer.pComTxBuf, MAX_COM_BUFFER_SIZE);
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: Error! pComTxBuf not allocated\n"));

	pAd->comTxBuffer.TxInsertPointer = 0;
	pAd->comTxBuffer.TxRemovePointer = 0;
	pAd->comTxBuffer.TxFreeCount	 = MAX_COM_BUFFER_SIZE;

	// init comRxBuffer
	pAd->comRxBuffer.pComRxBuf = (PUCHAR)ExAllocatePoolWithTag(
											NonPagedPool,
											MAX_COM_BUFFER_SIZE,
											'yroR');
	if(pAd->comRxBuffer.pComRxBuf)
		RtlZeroMemory(pAd->comRxBuffer.pComRxBuf, MAX_COM_BUFFER_SIZE);
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: Error! pComRxBuf not allocated\n"));

	pAd->comRxBuffer.RxInsertPointer = 0;
	pAd->comRxBuffer.RxRemovePointer = 0;
	pAd->comRxBuffer.RxFreeCount	 = MAX_COM_BUFFER_SIZE;

	{
		LARGE_INTEGER  CurrentTime;

		KeQuerySystemTime(&CurrentTime);
		LfsrInit(pAd, CurrentTime.LowPart);
	}

	pAd->state = Started;			
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: HandleStartDevice <=== \n\n\n"));	
	return PassDownPnP(pAd, pIrp);

#endif
}

NTSTATUS
ReadWriteConfigSpace(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          ReadOrWrite, // 0 for read 1 for write
    IN PVOID          Buffer,
    IN ULONG          Offset,
    IN ULONG          Length
    )
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT targetObject;

    PAGED_CODE();
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    targetObject = IoGetAttachedDeviceReference( DeviceObject );
    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        targetObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );
    if (irp == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }
    irpStack = IoGetNextIrpStackLocation( irp );
    if (ReadOrWrite == 0) {
        irpStack->MinorFunction = IRP_MN_READ_CONFIG;
    }else {
        irpStack->MinorFunction = IRP_MN_WRITE_CONFIG;
    }
    irpStack->Parameters.ReadWriteConfig.WhichSpace = PCI_WHICHSPACE_CONFIG;
    irpStack->Parameters.ReadWriteConfig.Buffer = Buffer;
    irpStack->Parameters.ReadWriteConfig.Offset = Offset;
    irpStack->Parameters.ReadWriteConfig.Length = Length;
    // Initialize the status to error in case the bus driver does not 
    // set it correctly.
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    status = IoCallDriver( targetObject, irp );
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
        status = ioStatusBlock.Status;
    }
End:
    // Done with reference
    ObDereferenceObject( targetObject );
    return status;
}


NTSTATUS HandleRemoveDevice(IN PDEVICE_OBJECT pDO,
							IN PIRP pIrp ) 
{
#ifdef RTMP_USB_SUPPORT
	KIRQL             oldIrql;
//    KEVENT            event;
    ULONG             requestCount;
    NTSTATUS          ntStatus;
    RTMP_ADAPTER *deviceExtension;

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HandleRemoveDevice - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (RTMP_ADAPTER *) pDO->DeviceExtension;

    //
    // The Plug & Play system has dictated the removal of this device.  We
    // have no choice but to detach and delete the device object.
    // (If we wanted to express an interest in preventing this removal,
    // we should have failed the query remove IRP).
    //

    if(USBSurpriseRemoved != deviceExtension->DeviceState) {

        //
        // we are here after QUERY_REMOVE
        //

        KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

        deviceExtension->QueueState = FailRequests;
        
        KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

        /*if(deviceExtension->WaitWakeEnable) {
        
            CancelWaitWake(deviceExtension);
        }*/

	

        ProcessQueuedRequests(deviceExtension);

        ntStatus = IoSetDeviceInterfaceState(&deviceExtension->InterfaceName, 
                                             FALSE);

        if(!NT_SUCCESS(ntStatus)) {

            BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 1, ("IoSetDeviceInterfaceState::disable:failed\n"));
        }

        RtlFreeUnicodeString(&deviceExtension->InterfaceName);

        BulkUsb_WmiDeRegistration(deviceExtension);

        BulkUsb_AbortPipes(deviceExtension);
    }
	UsbBulkMemoryFree(deviceExtension);

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, USBRemoved);
    
    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);
    
    //
    // need 2 decrements
    //

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HandleRemoveDevice::"));
    requestCount = BulkUsb_IoDecrement(deviceExtension);

    ASSERT(requestCount > 0);

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HandleRemoveDevice::"));
    requestCount = BulkUsb_IoDecrement(deviceExtension);

    KeWaitForSingleObject(&deviceExtension->RemoveEvent, 
                          Executive, 
                          KernelMode, 
                          FALSE, 
                          NULL);

    ReleaseMemory(pDO);
    //
    // We need to send the remove down the stack before we detach,
    // but we don't need to wait for the completion of this operation
    // (and to register a completion routine).
    //

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(pIrp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, pIrp);

    //
    // Detach the FDO from the device stack
    //
    IoDetachDevice(deviceExtension->TopOfStackDeviceObject);
    IoDeleteDevice(pDO);

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HandleRemoveDevice - ends\n"));

    return ntStatus;
#else

	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDO->DeviceExtension;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: HandleRemoveDevice ===> \n"));

//	if (IS_RT3290(pDevExt->MACVersion))
	//	RTMPEnableWlan(pDevExt, FALSE); // For MT7650
	
	if(pDevExt->DmaStatus == TRUE)
	{
		FreeDMAMemory(pDO);
		pDevExt->DmaStatus = FALSE;
	}

#ifdef SNIFF
	if(pDevExt->pPacketBuf)
		ExFreePool(pDevExt->pPacketBuf);
#endif

	if (pDevExt->state == Started) 
	{		
#ifdef MSI
		if (pDevExt->bConnectedInterrupt == TRUE)
		{
			IO_DISCONNECT_INTERRUPT_PARAMETERS discParams;
			RtlZeroMemory(&discParams, sizeof(IO_DISCONNECT_INTERRUPT_PARAMETERS));
			discParams.Version = pDevExt->TypeOfInterruptVersion;
			discParams.ConnectionContext.Generic = pDevExt->pConnectionContext;
			IoDisconnectInterruptEx(&discParams);
			pDevExt->bConnectedInterrupt = FALSE;
		}
#else
		if (pDevExt->pIntObj)		
		{
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\nPciDumpr: PCI Interrrupt disable! \n"));
			
			IoDisconnectInterrupt( pDevExt->pIntObj );			//disconnect interrupt
		}
#endif
		if(pDevExt->virtualaddr)
		{
			MmUnmapIoSpace(pDevExt->virtualaddr,pDevExt->MemorySize);
			pDevExt->virtualaddr = NULL;
		}
	}

	// disable device interface
	IoSetDeviceInterfaceState(&pDevExt->ustrSymLinkName, FALSE);
	RtlFreeUnicodeString(&pDevExt->ustrSymLinkName);	
		
	if (pDevExt->pLowerDevice)		IoDetachDevice(pDevExt->pLowerDevice);	// unattach from stack
	
	IoDeleteDevice( pDO );								// Delete the device

	pDevExt->state = Removed;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: HandleRemoveDevice <=== \n"));
	return PassDownPnP( pDevExt, pIrp );

#endif//USB and PCIe
}


VOID StartIo(
				IN PDEVICE_OBJECT pDO,
				IN PIRP pIrp )
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: StartIo ===> \n"));
}
//++
// Function:	DriverUnload
//
// Description:
//		Stops & Deletes devices controlled by this driver.
//		Stops interrupt processing (if any)
//		Releases kernel resources consumed by driver
//
// Arguments:
//		pDriverObject - Passed from I/O Manager
//
// Return value:
//		None
//--

VOID DriverUnload ( IN PDRIVER_OBJECT	pDriverObject ) 
{
#ifdef RTMP_USB_SUPPORT
 	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("USB DriverUnload==>\n"));
 	/*PUNICODE_STRING registryPath;
	UNREFERENCED_PARAMETER( pDriverObject );
	PAGED_CODE();
	registryPath = &Globals.BulkUsb_RegistryPath;

	if(registryPath->Buffer) 
	{
       	ExFreePool(registryPath->Buffer);
       	registryPath->Buffer = NULL;
	}*/

	
#else
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: DriverUnload\n"));

#endif
}

//++
// Function:	DispatchCreate
//
// Description:
//		Handles call from Win32 CreateFile request
//		Does nothing
//
// Arguments:
//		pDevObj - Passed from I/O Manager
//		pIrp - Passed from I/O Manager
//
// Return value:
//		NTSTATUS - success or failure code
//--
//LONG *g_pTestSSEnable;
NTSTATUS DispatchCreate (	IN PDEVICE_OBJECT	pDO,
							IN PIRP				pIrp ) 
{	
#ifdef RTMP_USB_SUPPORT
	ULONG                       i;
       NTSTATUS                    status = STATUS_SUCCESS;
       PFILE_OBJECT                fileObject;
       RTMP_ADAPTER *deviceExtension;
       PIO_STACK_LOCATION          irpStack;
       PBULKUSB_PIPE_CONTEXT       pipeContext;
       PUSBD_INTERFACE_INFORMATION interface;
	RTMP_ADAPTER *dx = (RTMP_ADAPTER *)pDO->DeviceExtension;
	LARGE_INTEGER CurrentTime;
       PAGED_CODE();

	//
       // initialize variables
       //
       irpStack = IoGetCurrentIrpStackLocation(pIrp);
       fileObject = irpStack->FileObject;
       deviceExtension = (RTMP_ADAPTER *) pDO->DeviceExtension;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("USB: DispatchCreate\n"));
	if(deviceExtension->DeviceState != USBWorking) 
	{
	        status = STATUS_INVALID_DEVICE_STATE;
       	 goto BulkUsb_DispatchCreate_Exit;
   	}

	if(deviceExtension->UsbInterface) 
	{
        	interface = deviceExtension->UsbInterface;
    	}
    	else 
	{
	      status = STATUS_INVALID_DEVICE_STATE;
	      goto BulkUsb_DispatchCreate_Exit;
	}

	//
       // FsContext is Null for the device
       //
       if(fileObject) 
	{        
        	fileObject->FsContext = NULL; 
       }
       else 
	{
	        status = STATUS_INVALID_PARAMETER;
	        goto BulkUsb_DispatchCreate_Exit;
       }
#ifdef RTMP_PCI_SUPPORT
	dx->FwReqSequence = 1;
#endif
	KeQuerySystemTime(&CurrentTime);
	LfsrInit(dx, CurrentTime.LowPart);
	//-----------------------------------------	
	// Initialize Bulk Memory///::TODO
	//g_pTestSSEnable = &deviceExtension->SSEnable;
	

	if(0 == fileObject->FileName.Length)
	{
	        //
	        // opening a device as opposed to pipe.
	        //
	        status = STATUS_SUCCESS;

	        InterlockedIncrement(&deviceExtension->OpenHandleCount);

	        //
	        // the device is idle if it has no open handles or pending PnP Irps
	        // since we just received an open handle request, cancel idle req.
	        //
	        /*if(deviceExtension->SSEnable) 
		 {        
	            CancelSelectSuspend(deviceExtension);
	        }*/

	        goto BulkUsb_DispatchCreate_Exit;
       }
    
      pipeContext = BulkUsb_PipeWithName(pDO, &fileObject->FileName);
  
      if(pipeContext == NULL) 
      {
  
          status = STATUS_INVALID_PARAMETER;
          goto BulkUsb_DispatchCreate_Exit;
      }
  
  	status = STATUS_INVALID_PARAMETER;
  
      for(i=0; i<interface->NumberOfPipes; i++) 
      {
  
          if(pipeContext == &deviceExtension->PipeContext[i]) 
  	{
  
              //
              // found a match
              //
              //BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 3, ("open pipe %d\n", i));
  
              fileObject->FsContext = &interface->Pipes[i];
              
              ASSERT(fileObject->FsContext);
  
              pipeContext->PipeOpen = TRUE;
  
              status = STATUS_SUCCESS;
  
              //
              // increment OpenHandleCounts
              //
              InterlockedIncrement(&deviceExtension->OpenHandleCount);
  
              //
              // the device is idle if it has no open handles or pending PnP Irps
              // since we just received an open handle request, cancel idle req.
              //
             /* if(deviceExtension->SSEnable) 
  	    {
  
                  CancelSelectSuspend(deviceExtension);
              }*/
          }
      }
	

BulkUsb_DispatchCreate_Exit:

    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
#else//PCIe
	NTSTATUS status = STATUS_SUCCESS;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDO->DeviceExtension;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "DispatchCreate");
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: DispatchCreate ===>\n"));	
	
	if (pDevExt->state != Started)		
		status = STATUS_DEVICE_REMOVED;

	DriverInit(pDO); // 050624 From DispatchCreate move here

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: DispatchCreate <===\n"));
	return status;

#endif
}

//++
// Function:	DispatchClose
//
// Description:
//		Handles call from Win32 CreateHandle request
//		For this driver, frees any buffer
//
// Arguments:
//		pDevObj - Passed from I/O Manager
//		pIrp - Passed from I/O Manager
//
// Return value:
//		NTSTATUS - success or failure code
//--

NTSTATUS DispatchClose (	IN PDEVICE_OBJECT	pDO,
							IN PIRP				pIrp ) 
{
#ifdef RTMP_USB_SUPPORT
	NTSTATUS               ntStatus;
    PFILE_OBJECT           fileObject;
    RTMP_ADAPTER *deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PBULKUSB_PIPE_CONTEXT  pipeContext;
    PUSBD_PIPE_INFORMATION pipeInformation;
    
    PAGED_CODE();

    //
    // initialize variables
    //
    irpStack = IoGetCurrentIrpStackLocation(pIrp);
    fileObject = irpStack->FileObject;
    pipeContext = NULL;
    pipeInformation = NULL;
    deviceExtension = (RTMP_ADAPTER *) pDO->DeviceExtension;
//	deviceExtension->ulBulkInRunning[0] = 0;
//	deviceExtension->ulBulkInRunning[1] = 0;
//	deviceExtension->bBulkInIoCallDriverFlag[0] = FALSE;
//	deviceExtension->bBulkInIoCallDriverFlag[1] = FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("#@#Bulkin[0] = %d, Bulkin[1] = %d\n", deviceExtension->ulBulkInRunning[0], deviceExtension->ulBulkInRunning[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("#@#bRxEnable[0] = %d, bRxEnable[1] = %d\n", deviceExtension->bRxEnable[0], deviceExtension->bRxEnable[1]));

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BulkUsb_DispatchClose - begins\n"));
    	
    if(fileObject && fileObject->FsContext) {

        pipeInformation = (PUSBD_PIPE_INFORMATION)fileObject->FsContext;

        if(0 != fileObject->FileName.Length) {

            pipeContext = BulkUsb_PipeWithName(pDO, 
                                               &fileObject->FileName);
        }

        if(pipeContext && pipeContext->PipeOpen) {
            
            pipeContext->PipeOpen = FALSE;
        }
    }

    //
    // set ntStatus to STATUS_SUCCESS 
    //
    ntStatus = STATUS_SUCCESS;

    pIrp->IoStatus.Status = ntStatus;
    pIrp->IoStatus.Information = 0;

    //UsbBulkMemoryFree(pDO);

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    InterlockedDecrement(&deviceExtension->OpenHandleCount);

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BulkUsb_DispatchClose - ends\n"));

    return ntStatus;
#endif /* _USB */

#ifdef RTMP_PCI_SUPPORT
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDO->DeviceExtension;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: DispatchClose ===>\n"));

	if (pDevExt->deviceBuffer != NULL) {
		ExFreePool(pDevExt->deviceBuffer);
		pDevExt->deviceBuffer = NULL;
		pDevExt->deviceBufferSize = 0;
	}

	//CSR1: System control register host ready disable
//	WRITE_Dummy_REGISTER_ULONG(((PULONG)pDevExt->virtualaddr + CSR1 / 4),0);

	// Disable interrupt

	pDevExt->IntrMask = (IRQ_MASK)&0xFFFFFFFF;	
	RTMP_Real_IO_WRITE32(pDevExt, RA_INT_MASK, pDevExt->IntrMask);//|IRQ_MSK_SPECIAL_BITS);		

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;	
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

//	 KeCancelTimer(&pDevExt->m_IsrTimer);
	NICIssueReset(pDevExt);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: DispatchClose <===\n"));
	return STATUS_SUCCESS;
#endif /* RTMP_PCI_SUPPORT */
}


//++
// Function:	DispatchCancel
//
// Description:
//		Handles canceled IRP
//
// Arguments:
//		pDevObj - Passed from I/O Manager
//		pIrp - Passed from I/O Manager
//
// Return value:
//		NTSTATUS - success or failuer code
//--

VOID DispatchCancel (	IN PDEVICE_OBJECT	pDevObj,
						IN PIRP				pIrp )
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IRP Canceled\n"));
	
	pIrp->IoStatus.Status = STATUS_CANCELLED;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	IoStartNextPacket( pDevObj, TRUE );
}


#ifdef RTMP_USB_SUPPORT
NTSTATUS DispatchDeviceControlUSB(IN RTMP_ADAPTER *pAd,
									IN PIRP Irp)
{
	PIO_STACK_LOCATION  irpStack;
	PULONG              pIBuffer;
	ULONG               inputBufferLength;
	PVOID               ioBuffer;
	ULONG               outputBufferLength;
	ULONG               ioControlCode;
	ULONG               byteCount = 0;
	NTSTATUS            ntStatus;

	    //ULONG              code;
	UINT32	IoControlCode;
	    ULONG              info;
           ULONG InputLength = 0;
           ULONG OutputLength = 0;
	    //RTMP_ADAPTER *deviceExtension;
	    //RTMP_ADAPTER *dx = (RTMP_ADAPTER *)DeviceObject->DeviceExtension;

	    //
	    // initialize variables
	    //
	    info = 0;
	    irpStack = IoGetCurrentIrpStackLocation(Irp);
	    IoControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
	    InputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
           OutputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	    //deviceExtension = (RTMP_ADAPTER *) DeviceObject->DeviceExtension;

	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("DeviceIoControl: Control code %x InputLength %d OutputLength %d\n",
				IoControlCode, InputLength, OutputLength));

	    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
	    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("USB: DispatchDeviceControl\n"));
	    if(pAd->DeviceState != USBWorking) {

	        BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 1, ("Invalid device state\n"));

	        Irp->IoStatus.Status = ntStatus = STATUS_INVALID_DEVICE_STATE;
	        Irp->IoStatus.Information = info;

	        IoCompleteRequest(Irp, IO_NO_INCREMENT);
	        return ntStatus;
	    }

          MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BulkUsb_DispatchDevCtrl::"));
	    BulkUsb_IoIncrement(pAd);

	    //
	    // It is true that the client driver cancelled the selective suspend
	    // request in the dispatch routine for create.
	    // But there is no guarantee that it has indeed been completed.
	    // so wait on the NoIdleReqPendEvent and proceed only if this event
	    // is signalled.
	    //

	    switch(IoControlCode) {

    	    case IOCTL_BULKUSB_RESET_PIPE:
	    {
	        PFILE_OBJECT           fileObject;
	        PUSBD_PIPE_INFORMATION pipe;

	        pipe = NULL;
	        fileObject = NULL;

	        //
	        // FileObject is the address of the kernel file object to
	        // which the IRP is directed. Drivers use the FileObject
	        // to correlate IRPs in a queue.
	        //
	        fileObject = irpStack->FileObject;

	        if(fileObject == NULL) {

	            ntStatus = STATUS_INVALID_PARAMETER;

	            break;
	        }

	        pipe = (PUSBD_PIPE_INFORMATION) fileObject->FsContext;

	        if(pipe == NULL) {

	            ntStatus = STATUS_INVALID_PARAMETER;
	        }
	        else {
	            
	            ntStatus = BulkUsb_ResetPipe(pAd, pipe);
	        }

	        break;
	    }

	    case IOCTL_BULKUSB_GET_CONFIG_DESCRIPTOR:
	    {
	     		ULONG length;

		        if(pAd->UsbConfigurationDescriptor) 
			{

		            length = pAd->UsbConfigurationDescriptor->wTotalLength;

		            if(outputBufferLength >= length) 
				{

		                RtlCopyMemory(ioBuffer,
		                              pAd->UsbConfigurationDescriptor,
		                              length);

		                info = length;

		                ntStatus = STATUS_SUCCESS;
			       }
			       else 
				{
			                
			             ntStatus = STATUS_BUFFER_TOO_SMALL;
			        }
		        }
		        else 
			{
		            
		            ntStatus = STATUS_UNSUCCESSFUL;
		        }

	        break;
	    }


		case IOCTL_BULKUSB_RESET_DEVICE:
	       {
		 MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s: IOCTL_BULKUSB_RESET_DEVICE\n",__FUNCTION__));
	        ntStatus = BulkUsb_ResetDevice(pAd);

	        break;
		}

		case IOCTL_USB_SUSPEND:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s: IOCTL_USB_SUSPEND\n",__FUNCTION__));
			
			info = 0;						
			StopRx1(pAd);
			ntStatus = SubmitIdleRequestIrp(pAd);			
	    	}
		break;

		case IOCTL_USB_RESUME:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s: IOCTL_USB_RESUME\n",__FUNCTION__));
			info = 0;
			CancelSelectSuspend(pAd);			
	    	}
		break;
		 default :
			//common ioctl
			ntStatus = DevIoctlHandler(pAd, InputLength, ioBuffer, OutputLength, ioBuffer, IoControlCode, &info);     

	        break;
    }
   
    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = info;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    //BulkUsb_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, 3, ("BulkUsb_DispatchDevCtrl::"));
    BulkUsb_IoDecrement(pAd);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s <<---\n",__FUNCTION__));
    return ntStatus;

//#endif
}

#endif
#ifdef RTMP_PCI_SUPPORT
NTSTATUS DispatchDeviceControlPCI(IN PDEVICE_OBJECT DeviceObject,
									IN PIRP Irp )
{
	PIO_STACK_LOCATION  irpStack;
	PULONG              pIBuffer;
	ULONG               inputBufferLength;
	PVOID               ioBuffer;
	ULONG               outputBufferLength;
	ULONG               ioControlCode;
	ULONG               byteCount = 0;
	NTSTATUS            ntStatus;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)DeviceObject->DeviceExtension;

//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"PciDumpr: DispatchDeviceControl ===>\n");	

	//
	// Set default values in the IRP.  Note that this driver will not queue
	// any IRPs (i.e. pend requests).
	//

	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	//
	// Get a pointer to the current location in the Irp. This is where
	// the function codes and parameters are located.
	//

	irpStack = IoGetCurrentIrpStackLocation(Irp);

	//
	// Get the pointer to the output buffer and it's length.  The IOCTLs for
	// this driver have been defined as METHOD_BUFFERED, so we can use the
	// buffers directly (i.e. there is no MDL).
	//

	pIBuffer = (PULONG)Irp->AssociatedIrp.SystemBuffer;
	ioBuffer = Irp->AssociatedIrp.SystemBuffer;

	inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

	ASSERT(irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL);

	switch (irpStack->Parameters.DeviceIoControl.IoControlCode) 
	{	
		case IOCTL_QA_GET_PACKETCMD_SEQNUM://USB and PCIe
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_QA_GET_FW_STATUS g_FWSeqMCU= %d\n", g_FWSeqMCU));
			
			RtlCopyMemory(ioBuffer, &g_PacketCMDSeqMCU,sizeof(g_PacketCMDSeqMCU));
			
			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = sizeof(g_PacketCMDSeqMCU);		
		}
		break;

		case IOCTL_QA_GET_PACKETCMD_RESPONSE://USB and PCIe
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_QA_GET_FW_STATUS g_FWSeqMCU= %d\n", g_FWSeqMCU));
			
			RtlCopyMemory(ioBuffer, &g_PacketCMDRspData,sizeof(g_PacketCMDRspData));
			

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = sizeof(g_PacketCMDRspData);

		}
		break;
		
	
		case IOCTL_QA_GET_FWD_SEQNUM://USB and PCIe
		{
			ULONG BytesTxd;
			ULONG status;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_QA_GET_FW_STATUS g_FWSeqMCU= %d\n", g_FWSeqMCU));

			RtlCopyMemory(ioBuffer, &g_FWSeqMCU,sizeof(g_FWSeqMCU));
			BytesTxd = sizeof(g_FWSeqMCU);

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;
			status = Irp->IoStatus.Status;

		}
		break;

		case IOCTL_QA_GET_FWD_STATUS:
		{
			ULONG BytesTxd;
			ULONG status;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_QA_GET_FWD_STATUS g_FWRspStatus= %d\n", g_FWRspStatus));

			RtlCopyMemory(ioBuffer, &g_FWRspStatus,sizeof(g_FWRspStatus));
			BytesTxd = sizeof(g_FWRspStatus);

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;
			status = Irp->IoStatus.Status;

		}
		break;


			


		///////	DMA 4K Boundary Test	
		case IOCTL_PCIDUMPR_DMA_4K_BOUNDARY_TEST:
		{
			ULONG BytesTxd;
			ULONG status;
			
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			
			ULONG	b4kBoundaryCheck = (*pInBuf);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("b4kBoundaryCheck = %d\n", b4kBoundaryCheck));

			g_bDam4kBoundaryCheck = (BOOLEAN)b4kBoundaryCheck;

			BytesTxd = 0;	
			status = STATUS_SUCCESS;	

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;		
		}
		break;

		//0: BufferMode, 1: Efuse, 2: EEPROM
		case IOCTL_SET_EFUSE_MODE:
		{
			UCHAR* pInBuf = (UCHAR*)ioBuffer;			

			pAd->EfuseMode = pInBuf[0];
			
			ntStatus = STATUS_SUCCESS;
	    	}
		break;

		case IOCTL_WRITE_EEPROM_BUFFER:
		{			
			ULONG* pInBuf = (ULONG*)ioBuffer;			
			ULONG addr = (ULONG)pInBuf[0];
			UCHAR size = (UCHAR)pInBuf[1];
			UCHAR idx = 0;
			UCHAR idx2 = 0;	
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("kun WRITE_EEPROM_BUFFER, addr = 0x%x, size = %d\n", addr, size ));
		
			if (size == 16)
			{
				for (idx=0; idx<4; idx++)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("kun 0x%x \n", pInBuf[2+idx] ));
					pAd->EfuseContent[addr+idx*4] = (UCHAR)(pInBuf[2+idx]);
					pAd->EfuseContent[addr+idx*4+1] = (UCHAR)(pInBuf[2+idx] >> 8);
					pAd->EfuseContent[addr+idx*4+2] = (UCHAR)(pInBuf[2+idx] >> 16);
					pAd->EfuseContent[addr+idx*4+3] = (UCHAR)(pInBuf[2+idx] >> 24);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[kun] EfuseContent[0x%x] = 0x%x \n", addr+idx*4,  (UCHAR)(pInBuf[2+idx] )));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[kun] EfuseContent[0x%x] = 0x%x \n", addr+idx*4+1, (UCHAR)(pInBuf[2+idx] >> 8)));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[kun] EfuseContent[0x%x] = 0x%x \n", addr+idx*4+2, (UCHAR)(pInBuf[2+idx] >> 16)));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[kun] EfuseContent[0x%x] = 0x%x \n", addr+idx*4+3, (UCHAR)(pInBuf[2+idx] >> 24)));
				}
				
			/*
				for (idx=0; idx<4; idx++)
				{
				
					RtlCopyMemory(
						&pAd->EfuseContent[addr+idx*4],
						(PUCHAR)pInBuf[2+idx],				 
						size
						);
						
				}//RtlCopyMemory((PULONG)pAd->Ac1RingMain[idx].DmaBuf.AllocVa, (pIBuffer+20), length);
				*/
				/*
				for (idx2 = 0; idx2<16;idx2++)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("kun 0x%x \n", pInBuf[2+idx]));						
				}
				*/
				
			}
			else
			{
			
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[kun] size != 16 \n"));	
				/*
				UCHAR DWNumbers = size/4;
				UCHAR DWMods = size%4;
				for (idx=0; idx<DWNumbers; idx++)
				{
				
					RtlCopyMemory(
						&pAd->EfuseContent[addr+idx*4],			 
						(PVOID)pInBuf[2+idx],				 
						size
						);					

						
				}
				for (idx=0; idx<DWMods; idx++)
				{
					pAd->EfuseContent[addr+DWNumbers*4+idx]  = (UCHAR)(pInBuf[DWNumbers]>>(8*idx)); 				
				}
				*/
			}	
			/*
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=====efuse content=====\n" ));
			for (idx = 0; idx<EFUSESIZE;idx++)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("0x%02x \n", pAd->EfuseContent[idx]));	
				if(EFUSESIZE%16 == 0)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n"));
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=====efuse content=====\n" ));
			*/
			ntStatus = STATUS_SUCCESS;
			

		}

		break;

		case IOCTL_HAL_CMD:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			ULONG status = STATUS_SUCCESS;
			ULONG BytesTxd = 0;
			ULONG iCmd = pInBuf[0];					//addr
			ULONG iType = pInBuf[1];
			//ULONG Length = pInBuf[1];					//length
			ULONG temp = 0;
			UCHAR TempChar = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("iCmd = %d\n", iCmd));
			switch(iCmd)
			{
				case CMD_ASIC_TOPINIT:
					MtAsicTOPInit(pAd);
					break;

				case CMD_HIF_INIT:
					mt_hif_sys_init(pAd);
					break;

				case CMD_TMAC_INIT:
					asic_set_tmac_info_template(pAd);
					break;

				case CMD_INITMAC:
					mt7603_init_mac_cr(pAd);
					//SetConfigMIBCounter(pAd,STATISTICS_DEFAULT_EN_COUNTS,0, TRUE);
					break;

				case CMD_CONTI_TX_START:
					StartContinuesTXTest(pAd, (UCHAR)pInBuf[1], (UCHAR)pInBuf[2], (UCHAR)pInBuf[3], (UCHAR)pInBuf[4], (UCHAR)pInBuf[5]);
					break;

				case CMD_CONTI_TX_STOP:
					StopContinuTXTest(pAd);					
					break;

				case CMD_CONTI_TONE_START:
					StartTXToneTest(pAd, iType);
					break;

				case CMD_CONTI_TONE_STOP:
					StopTXToneTest(pAd);									
					break;

				case CMD_GET_MAC_MDRDY:
					GetStaticNumber(pAd,&temp);
					RtlCopyMemory(ioBuffer, &temp,sizeof(temp));

					//pOutBuf[0] = temp;
					//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("temp = %d, pOutBuf[0]=%d\n", temp,pOutBuf[0]));
					BytesTxd = 4;	
					break;

				case  CMD_SET_TXPOWER:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%d ,%d ,%d \n",pInBuf[1], pInBuf[2], pInBuf[3]));
					TxSetFramePower(pAd, pInBuf[1], pInBuf[2], (UCHAR)pInBuf[3]);
					break;

				case CMD_SET_AIFS://(IPG)
					{
						int i;
						for(i=AIFS_0;i<=AIFS_14;i++)//set all rings
						{
							SetAIFS(pAd,i,pInBuf[1]);
						}
					}
					break;

				case CMD_FREQ_OFFSET:
					TxSetFrequencyOffset (pAd, pInBuf[1]);
					break;

				case CMD_TXPOWER_RANGE:
					TxConfigPowerRange(pAd, (UCHAR)pInBuf[1],(UCHAR) pInBuf[2]);
					break;				

				case CMD_GET_DRIVER_RX_COUNT:
					RtlCopyMemory(ioBuffer, &pAd->OtherCounters.Rx0ReceivedCount,sizeof(pAd->OtherCounters.Rx0ReceivedCount));
					BytesTxd = sizeof(pAd->OtherCounters.Rx0ReceivedCount);
					BytesTxd = 4;
					break;

				case CMD_GET_RXSNR:
					pOutBuf[0] = pAd->OtherCounters.SNR0;
					pOutBuf[1] = pAd->OtherCounters.SNR1;
					BytesTxd = 8;
					break;

				case CMD_GET_RCPI:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RCPI0 %d ,RCPI1 %d \n", pAd->OtherCounters.RCPI0, pAd->OtherCounters.RCPI1));
					pOutBuf[0] = pAd->OtherCounters.RCPI0;
					pOutBuf[1] = pAd->OtherCounters.RCPI1;
					BytesTxd = 8;
					break;

				case CMD_GET_FREQOFFSETFROM_RX:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("FreqOffstFromRX %d \n", pAd->OtherCounters.FreqOffstFromRX));
					pOutBuf[0] = pAd->OtherCounters.FreqOffstFromRX;
					BytesTxd = 4;
					break;

				case CMD_GET_TX_COUNT:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CMD_GET_TX_COUNT %d \n", pAd->OtherCounters.Ac0TxedCount));
					pOutBuf[0] = pAd->OtherCounters.Ac0TxedCount;
					BytesTxd = sizeof(pAd->OtherCounters.Ac0TxedCount);
					break;

				case CMD_SET_TSSI:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CMD_SET_TSSI\n"));
					SetTSSI(pAd, pInBuf[1], (UCHAR)pInBuf[2]);
					BytesTxd = 0;
					break;	
				
				case CMD_SET_DPD:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CMD_SET_DPD\n"));
					SetDPD(pAd, pInBuf[1], (UCHAR)pInBuf[2]);
					BytesTxd = 0;
					break;	

				case CMD_GET_MACFCSERRORCNT_COUNT:
					GetMACFCSErrCnt(pAd, &pInBuf[0]);
					BytesTxd = 4;
					break;

				case CMD_GET_PHY_MDRDYCOUNT:
					GetPHYMdrdyErrCnt(pAd, &pInBuf[0], &pInBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_PHY_FCSERRCOUNT:
					GetPHYFCSErrCnt(pAd, &pInBuf[0], &pInBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_PD:
					GetPhyPD(pAd, &pInBuf[0], &pInBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_CCK_SIG_SFD:
					GetPhyCCK_SIG_SFD(pAd, &pInBuf[0], &pInBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_OFDM_ERR:
					GetPhyOFDMErr(pAd, &pInBuf[0], &pInBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_RSSI:
					GetPhyRSSI(pAd, &pInBuf[0], &pInBuf[1], &pInBuf[2], &pInBuf[3]);
					BytesTxd = 32;
					break;

				case CMD_TMR_SETTING:
					SetTMR(pAd, (UCHAR)pInBuf[1]);
					BytesTxd = 0;
					break;

				case CMD_GET_SECURITY_RESULT:
					 pInBuf[0] = pAd->SecurityFail;
					BytesTxd = 4;
					break;

			}
			
			status = STATUS_SUCCESS;	

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;

		}
		break;
		
		///////	Set Channel
		case IOCTL_PCIQA_SELECT_CHANNEL:
		{
			ULONG BytesTxd;
			ULONG status;
			
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			
			ULONG	Channel = (*pInBuf);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("channel = %d\n", Channel));

			BytesTxd = 0;			
			HwHal_SetChannel(pAd, Channel);

			BytesTxd = 0;	
			status = STATUS_SUCCESS;	

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;
			
		}
		break;

		case IOCTL_QA_GET_FW_CMD_RSP:
		{
			ULONG BytesTxd;
			ULONG status;
			PFW_RXD_STRUC pFwReceiveBuffer;

			Testdbg("IOCTL_QA_GET_FW_CMD_RSP\n");

			pFwReceiveBuffer = (PFW_RXD_STRUC) &pAd->FwReceiveBuffer;
			Testdbg("pFwReceiveBuffer->CMD_SEQ = %d\n", pFwReceiveBuffer->CMD_SEQ);
			Testdbg("pAd->FwReqSequence = %d\n", pAd->FwReqSequence);
			
			if ( pFwReceiveBuffer->CMD_SEQ == pAd->FwReqSequence )
			{
				RtlCopyMemory(ioBuffer, &pAd->FwReceiveBuffer, pFwReceiveBuffer->PktLength + sizeof(FW_RXD_STRUC));

				BytesTxd = pFwReceiveBuffer->PktLength + sizeof(FW_RXD_STRUC);
			}
			else
			{
				BytesTxd = 0;
			}

			Irp->IoStatus.Status = STATUS_SUCCESS;

			Irp->IoStatus.Information = BytesTxd;
			status = Irp->IoStatus.Status;
		}
		break;
			
		///////	Write Rf Register
		case IOCTL_PCIQA_RF_WRITE:
		{
			ULONG BytesTxd;
			ULONG status;
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			ULONG Value = pInBuf[0];					//Value
		
			BytesTxd = 0;
			HwHal_WriteRfRegister(pAd, Value);

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;
		
		}
		break;


		///////	Read Mac Registers
		case IOCTL_PCIQA_PCI_MAC_MULTI_READ:
		{
			ULONG BytesTxd;
			ULONG status;

			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;

			ULONG Offset = pInBuf[0];					//addr
			ULONG Length = pInBuf[1];					//length

			//USHORT* pValueX = &pInBuf[2];					//value ...			

			BytesTxd = 0;

			HwHal_ReadMacRegisters(pAd, Offset, Length, pOutBuf);

			BytesTxd = Length;
			status = STATUS_SUCCESS;
			
			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;			
	    	}
		break;
		
		///////	Write Mac Registers
		case IOCTL_PCIQA_PCI_MAC_MULTI_WRITE:
		{
			ULONG BytesTxd;
			ULONG status;
			
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;


			ULONG Offset = pInBuf[0];					//addr
			ULONG Length = pInBuf[1];					//length

			ULONG* pValueX = &pInBuf[2];					//value ...			

			BytesTxd = 0;

			HwHal_WriteMacRegisters(pAd, Offset, Length, pValueX);

			BytesTxd = 0;
			status = STATUS_SUCCESS;

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;			
	    	}
		break;

		///////	Read Eeprom Registers
		case IOCTL_PCIQA_PCI_EEPROM_READ:
		{
			ULONG BytesTxd;
			ULONG status;
			
			USHORT* pInBuf = (USHORT*)ioBuffer;
			USHORT* pOutBuf = (USHORT*)ioBuffer;

			USHORT Offset = pInBuf[0];					//addr
			USHORT Length = pInBuf[1];					//length

			//USHORT* pValueX = &pInBuf[2];					//value ...			

			BytesTxd = 0;

			HwHal_ReadEeRegisters(pAd, Offset, Length, pOutBuf);
			BytesTxd = Length;
			
			status = STATUS_SUCCESS;

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;	
	    	}
		break;
		
		///////	Write Eeprom Registers
		case IOCTL_PCIQA_PCI_EEPROM_WRITE:
		{
			ULONG BytesTxd;
			ULONG status;
			
			USHORT* pInBuf = (USHORT*)ioBuffer;
			USHORT* pOutBuf = (USHORT*)ioBuffer;

			USHORT Offset = pInBuf[0];					//addr
			USHORT Length = pInBuf[1];					//length

			USHORT* pValueX = &pInBuf[2];					//value ...			

			BytesTxd = 0;

			HwHal_WriteEeRegisters(pAd, Offset, Length, pValueX);
			
			BytesTxd = 0;
			status = STATUS_SUCCESS;

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;		
	    	}
		break;

		///////	Read Bbp Register
		case IOCTL_PCIQA_PCI_BBP_READ:
		{
			ULONG BytesTxd;
			ULONG status;
			
			UCHAR* pInBuf = (UCHAR*)ioBuffer;
			UCHAR* pOutBuf = (UCHAR*)ioBuffer;

			UCHAR Id = pInBuf[0];					//Id

			//UCHAR* pValue = &pOutBuf[1];					//pValue		

			BytesTxd = 0;
			HwHal_ReadBbpRegister(pAd, Id, pOutBuf);
			
			BytesTxd = 1;	
			status = STATUS_SUCCESS;

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;		
	    	}
		break;
		
		///////	Write Bbp Register
		case IOCTL_PCIQA_PCI_BBP_WRITE:
		{
			ULONG BytesTxd;
			ULONG status;
			
			UCHAR* pInBuf = (UCHAR*)ioBuffer;
			UCHAR* pOutBuf = (UCHAR*)ioBuffer;


			UCHAR Id = pInBuf[0];					//Id
			UCHAR Value = pInBuf[1];						//Value
		

			BytesTxd = 0;
			HwHal_WriteBbpRegister(pAd, Id, Value);

			BytesTxd = 0;
			status = STATUS_SUCCESS;

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;		
	    	}
		break;

		///////	Set Rx Antenna
		case IOCTL_PCIQA_RX_ANTENNA_SELECTED:
		{
			ULONG BytesTxd;
			ULONG status;
			
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			
			ULONG	Antenna = *pInBuf;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rx Antenna =%d\n", Antenna));

			BytesTxd = 0;					
			HwHal_SetRxAntenna(pAd, Antenna);

			BytesTxd = 0;	
			status = STATUS_SUCCESS;	

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;			
		}
		break;

		///////	Set Tx Antenna
		case IOCTL_PCIQA_TX_ANTENNA_SELECTED:
		{
			ULONG BytesTxd;
			ULONG status;
			
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;

			ULONG	Antenna = *pInBuf;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rx Antenna =%d\n", Antenna));
			
			BytesTxd = 0;					
			HwHal_SetTxAntenna(pAd, Antenna);

			BytesTxd = 0;	
			status = STATUS_SUCCESS;		

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;			
		}
		break;

		///////	Set Rf Antenna
		case IOCTL_PCIQA_RF_ANTENNA_SELECTED:
		{
			ULONG BytesTxd;
			ULONG status;
			
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)ioBuffer;
			
			ULONG	Antenna = *pInBuf;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rx Antenna =%d\n", Antenna));

			BytesTxd = 0;					
			HwHal_SetRfAntenna(pAd, Antenna);

			BytesTxd = 0;	
			status = STATUS_SUCCESS;	

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = BytesTxd;			
		}
		break;
		
		//--------------------------------------------------------------
			
	        case IOCTL_PCIDUMPR_DUMP_PCI_DATA:
		{
			ULONG Bus, length, propertyAddress;
			ULONG Function, Device;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_DUMP_PCI_DATA \n"));
			
			IoGetDeviceProperty(pAd->pPhyDeviceObj,
						DevicePropertyBusNumber,
						sizeof(ULONG),
						(PVOID)&Bus,
						&length);

			IoGetDeviceProperty(pAd->pPhyDeviceObj,
						DevicePropertyAddress,
						sizeof(ULONG),
						(PVOID)&propertyAddress,
						&length);

			Function = (propertyAddress) & 0x0000FFFF;
			Device = ((propertyAddress) >> 16) & 0x0000FFFF;

			RtlCopyMemory(pIBuffer, &Bus, sizeof(ULONG));
			RtlCopyMemory(pIBuffer + 1, &Function, sizeof(ULONG));
			RtlCopyMemory(pIBuffer + 2, &Device, sizeof(ULONG));

	        
			//
		        // Scan the PCI slots and dump the PCI configuration data to the
		        // user's buffer.
		        //
				ReadWriteConfigSpace(DeviceObject, 
										0, // 0 for read 1 for write
										pIBuffer + 3,
										0,
										256
										);

		        //
		        // If everything was OK, then update the returned data byte count.
		        //
				byteCount = 3 * sizeof(ULONG) + 256 + 64;

		        if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
		        }	       		
		}
		 break;

		 case IOCTL_READ_PCI_ConfigSpace:
		 {
			ULONG	Bus, length, propertyAddress;
			ULONG	Function, Device;

		 	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_READ_PCI_ConfigSpace \n"));
			
			IoGetDeviceProperty(pAd->pPhyDeviceObj,
						DevicePropertyBusNumber,
						sizeof(ULONG),
						(PVOID)&Bus,
						&length);

			IoGetDeviceProperty(pAd->pPhyDeviceObj,
						DevicePropertyAddress,
						sizeof(ULONG),
						(PVOID)&propertyAddress,
						&length);

			Function = (propertyAddress) & 0x0000FFFF;
			Device = ((propertyAddress) >> 16) & 0x0000FFFF;

			RtlCopyMemory(pIBuffer, &Bus, sizeof(ULONG));
			RtlCopyMemory(pIBuffer + 1, &Function, sizeof(ULONG));
			RtlCopyMemory(pIBuffer + 2, &Device, sizeof(ULONG));

	        
			//
		        // Scan the PCI slots and dump the PCI configuration data to the
		        // user's buffer.
		        //
			if ( pAd->OSMajorVersion == 5) // XP
			{
				PULONG				MmVirtualAddr;
				PHYSICAL_ADDRESS		QueryPhyAddr;
				QueryPhyAddr.LowPart = pAd->ConfigurationSpacePhyAddr.LowPart;
				QueryPhyAddr.LowPart += (Bus*32*8*0x1000) + (Device*8*0x1000) + (Function*0x1000);
				QueryPhyAddr.LowPart += *(pIBuffer + 3);

				MmVirtualAddr = (PULONG)MmMapIoSpace(QueryPhyAddr, 4, MmCached);

				*(pIBuffer + 3) = *MmVirtualAddr;
				
				MmUnmapIoSpace((PVOID)MmVirtualAddr, 4);
			}
			else
			{
					ReadWriteConfigSpace(DeviceObject, 
												0, // 0 for read 1 for write
												ioBuffer,
												*(pIBuffer + 3),
												4
												);
			}
		        //
		        // If everything was OK, then update the returned data byte count.
		        //
			byteCount = sizeof(ULONG);

		        if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
		        }
		 }
		 break;

		case IOCTL_WRITE_PCI_ConfigSpace:
		{
			ULONG	Bus, length, propertyAddress;
			ULONG	Function, Device;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_WRITE_PCI_ConfigSpace \n"));
			
			IoGetDeviceProperty(pAd->pPhyDeviceObj,
						DevicePropertyBusNumber,
						sizeof(ULONG),
						(PVOID)&Bus,
						&length);

			IoGetDeviceProperty(pAd->pPhyDeviceObj,
						DevicePropertyAddress,
						sizeof(ULONG),
						(PVOID)&propertyAddress,
						&length);

			Function = (propertyAddress) & 0x0000FFFF;
			Device = ((propertyAddress) >> 16) & 0x0000FFFF;

			RtlCopyMemory(pIBuffer, &Bus, sizeof(ULONG));
			RtlCopyMemory(pIBuffer + 1, &Function, sizeof(ULONG));
			RtlCopyMemory(pIBuffer + 2, &Device, sizeof(ULONG));

	        
			//
		        // Scan the PCI slots and dump the PCI configuration data to the
		        // user's buffer.
		        //
		        if ( pAd->OSMajorVersion == 5) // XP
			{
				PULONG				MmVirtualAddr;
				PHYSICAL_ADDRESS		QueryPhyAddr;
				QueryPhyAddr.LowPart = pAd->ConfigurationSpacePhyAddr.LowPart;
				QueryPhyAddr.LowPart += (Bus*32*8*0x1000) + (Device*8*0x1000) + (Function*0x1000);
				QueryPhyAddr.LowPart += *(pIBuffer + 3);

				MmVirtualAddr = (PULONG)MmMapIoSpace(QueryPhyAddr, 4, MmCached);

				*MmVirtualAddr = *(pIBuffer + 3);

				MmUnmapIoSpace((PVOID)MmVirtualAddr, 4);
			}
			else
			{
				ReadWriteConfigSpace(DeviceObject, 
											1, // 0 for read 1 for write
											((PULONG)ioBuffer + 4),
											*(pIBuffer + 3),
											4
											);
			}

		        //
		        // If everything was OK, then update the returned data byte count.
		        //
			byteCount = 0;

		        if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
		        }
		}
		break;
		
		case IOCTL_PCIDUMPR_GET_PCI_ADDRESS :
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_GET_PCI_ADDRESS \n"));

			RtlCopyMemory(ioBuffer,&pAd->ExtendPciAddress, sizeof(EXTENDED_PCI_ADDRESS));
			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = sizeof(EXTENDED_PCI_ADDRESS);
		break;

		//
		// Read Memry in defined format
		//
		case IOCTL_PCIDUMPR_READ_MEMORY:
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_READ_MEMORY \n"));

			Irp->IoStatus.Status = ReadMemory(DeviceObject,
											pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount
											);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS) 
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		//
		// Write Memry in defined format
		//
		case IOCTL_PCIDUMPR_WRITE_MEMORY:
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_WRITE_MEMORY \n"));

			Irp->IoStatus.Status = WriteMemory(DeviceObject,
											pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount
											);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		//
		// Read Memry in defined length
		//

		case IOCTL_PCIDUMPR_BUFFER_READ_MEMORY:
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_BUFFER_READ_MEMORY \n"));

			Irp->IoStatus.Status = BufferReadMemory(DeviceObject,
												pIBuffer,
												(PUCHAR)ioBuffer,
												outputBufferLength,
												&byteCount
												);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		//
		// Write Memry in defined length
		//

		case IOCTL_PCIDUMPR_BUFFER_WRITE_MEMORY:
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_BUFFER_WRITE_MEMORY \n"));

			Irp->IoStatus.Status = BufferWriteMemory(pIBuffer,
												(PUCHAR)ioBuffer,
												outputBufferLength,
												&byteCount
												);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;


		case IOCTL_PCIDUMPR_READ_BLOCK_EEPROM:
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_READ_BLOCK_EEPROM \n"));

			Irp->IoStatus.Status = BufferReadEEPROM(DeviceObject,
												pIBuffer,
												(PUCHAR)ioBuffer,
												outputBufferLength,
												&byteCount
												);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS) 
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		//
		// Write Memry in defined length
		//

		case IOCTL_PCIDUMPR_WRITE_BLOCK_EEPROM:
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_WRITE_BLOCK_EEPROM \n"));

			Irp->IoStatus.Status = BufferWriteEEPROM(DeviceObject,
												(PUCHAR)pIBuffer,
												(PUCHAR)ioBuffer,
												outputBufferLength,
												&byteCount
												);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		case IOCTL_PCIDUMPR_PROBE_DMA_DATA:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_GEN_DMA \n"));

			Irp->IoStatus.Status = ProbeDmaData(DeviceObject,
											pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount
											);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS) 
			{
				Irp->IoStatus.Information = byteCount;
			}
		    break;

		case IOCTL_PCIDUMPR_TRANSFER_DMA_DATA_BUFFER:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_TRANSFER_DMA_DATA_BUFFER \n"));

			Irp->IoStatus.Status = TransferDMADataBuffer(
													DeviceObject,
													(PUCHAR)pIBuffer,
													(PUCHAR)ioBuffer,
													outputBufferLength,
													&byteCount
													);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS) 
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;


		case IOCTL_PCIDUMPR_TRANSFER_DMA_DESCRIPTOR:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_TRANSFER_DMA_DESCRIPTOR \n"));

			Irp->IoStatus.Status = TransferDMADescriptor(
													DeviceObject,
													(PUCHAR)pIBuffer,
													(PUCHAR)ioBuffer,
													outputBufferLength,
													&byteCount
													);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		case IOCTL_PCIDUMPR_DUMP_DMA:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_DUMP_DMA \n"));

			Irp->IoStatus.Status = DumpDMA(DeviceObject,
										pIBuffer,
										(PUCHAR)ioBuffer,
										outputBufferLength,
										&byteCount
										);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS) 
			{
				Irp->IoStatus.Information = byteCount;
			}

		break;

		case IOCTL_PCIDUMPR_ERASE_DMA:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_ERASE_DMA \n"));

			Irp->IoStatus.Status = EraseDMA(DeviceObject,
										pIBuffer,
										(PUCHAR)ioBuffer,
										outputBufferLength,
										&byteCount
										);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		case IOCTL_PCIDUMPR_START_TX:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_START_TX \n"));

			Irp->IoStatus.Status = RTPCIStartTX(DeviceObject,
											pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount
											);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		case IOCTL_PCIDUMPR_START_RX:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_START_RX \n"));

			Irp->IoStatus.Status = RTPCIStartRX(
											DeviceObject,
											pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount	
											);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;


		case IOCTL_PCIDUMPR_COM_WRITE:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_COM_WRITE \n"));

			Irp->IoStatus.Status = RTPCIComWrite(
											DeviceObject,
											(PUCHAR)pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount	
											);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		case IOCTL_PCIDUMPR_COM_READ:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_COM_READ \n"));

			Irp->IoStatus.Status = RTPCIComRead(
											DeviceObject,
											pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount	
											);

			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

#ifdef	SNIFF
		case IOCTL_PCIDUMPR_SNIFFER:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_SNIFFER \n"));

			Irp->IoStatus.Status = RTPCIStartSniffer(
												DeviceObject,
												pIBuffer,
												(PUCHAR)ioBuffer,
												outputBufferLength,
												&byteCount	
												);
			//
			// If everything was OK, then update the returned data byte count.
			//

			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;	    
#endif

		case IOCTL_ENABLE_BEACON:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_ENABLE_BEACON \n"));

			Irp->IoStatus.Status = EnableBeacon(
											DeviceObject,
											pIBuffer,
											(PUCHAR)ioBuffer,
											outputBufferLength,
											&byteCount	
											);
			//
			// If everything was OK, then update the returned data byte count.
			//
			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		case IOCTL_DISABLE_BEACON:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_DISABLE_BEACON \n"));

			Irp->IoStatus.Status = DisableBeacon(
											DeviceObject,									
											&byteCount	
											);
			//
			// If everything was OK, then update the returned data byte count.
			//
			if (Irp->IoStatus.Status == STATUS_SUCCESS)
			{
				Irp->IoStatus.Information = byteCount;
			}
		break;

		case IOCTL_SET_TCPIP_PAYLOAD:
			{
				ULONG* pInBuf = (ULONG*)ioBuffer;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_SET_TCPIP_PAYLOAD \n"));
					
				pAd->SetTcpIpPayload = *pInBuf;
				
				//
				// If everything was OK, then update the returned data byte count.
				//
				Irp->IoStatus.Status = STATUS_SUCCESS;
				Irp->IoStatus.Information = 0;
			}
		break;

		case IOCTL_PCIDUMPR_GET_FIRMWARE_VERSION:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_PCIDUMPR_GET_FIRMWARE_VERSION \n"));

			RtlCopyMemory(ioBuffer,&pAd->FirmwareVersion, sizeof(ULONG));
			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = sizeof(ULONG);
			
		break;

		case IOCTL_ENABLE_ANTENNA_DIVERSITY:
			{
				ULONG* pInBuf = (ULONG*)ioBuffer;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: IOCTL_ENABLE_ANTENNA_DIVERSITY \n"));
				
				pAd->AntennaDiversity = 1;
				pAd->SelAnt = *pInBuf;
				Irp->IoStatus.Status = STATUS_SUCCESS;
				Irp->IoStatus.Information = 0;
			}
		break;

		default:
			Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: unknown IRP_MJ_DEVICE_CONTROL\n"));
		break;
	}

	//
	// As soon as IoCompleteRequest is called, the status field of that IRP
	// is inaccessible.  So fill it in before the call to IoCompleteRequest.
	//

	ntStatus = Irp->IoStatus.Status;

	IoCompleteRequest(				// indicates the caller has completed all processing for a given I/O request and is returning the given IRP to the I/O Manager.
					Irp,			// Points to the IRP to be completed. 
					IO_NO_INCREMENT	// This value is IO_NO_INCREMENT if the original thread requested an operation the driver could complete quickly 
					);

	//
	// We never have pending operation so always return the status code.
	//

//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"PciDumpr: DispatchDeviceControl <===\n");	

	return ntStatus;
}
#endif // _USB //


/*++

Routine Description:

    Process the IOCTLs sent to this device.

Arguments:

    DeviceObject - Target device object.

    Irp - IRP_MJ_DEVICE_CONTROL

Return Value:

    NTSTATUS

--*/
NTSTATUS DispatchDeviceControl(IN PDEVICE_OBJECT DeviceObject,
									IN PIRP Irp )

{
	
#ifdef RTMP_USB_SUPPORT
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) DeviceObject->DeviceExtension;
	return DispatchDeviceControlUSB(pAd, Irp);
#else
	return DispatchDeviceControlPCI(DeviceObject, Irp);
#endif 
}   // PciDumprDeviceControl

#ifdef RTMP_PCI_SUPPORT
NTSTATUS
DriverInit(
    IN PDEVICE_OBJECT pDO
    )
{
	NTSTATUS				ntStatus = STATUS_SUCCESS;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDO->DeviceExtension;
	ULONG				value, data = 0, MAC_Value;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("DriverInit====> \n"));	
	//------------------------------------------------------------------
	// Initialize Adapter's variables	

	RtlZeroMemory(&pDevExt->WlanCounters, sizeof(COUNTER_802_11));
	RtlZeroMemory(&pDevExt->OtherCounters, sizeof(OTHER_STATISTICS));
	
	pDevExt->CurRx0Index = 0;
	pDevExt->CurRx1Index = 0;
	
	pDevExt->CurAc0TxIndex = 0;
	pDevExt->NextAc0TxDoneIndex = 0;
	pDevExt->CurAc1TxIndex = 0;
	pDevExt->NextAc1TxDoneIndex = 0;
	pDevExt->CurAc2TxIndex = 0;
	pDevExt->NextAc2TxDoneIndex = 0;
	pDevExt->CurAc3TxIndex = 0;
	pDevExt->NextAc3TxDoneIndex = 0;
	pDevExt->CurAc4TxIndex = 0;
	pDevExt->NextAc4TxDoneIndex = 0;
	pDevExt->CurAc5TxIndex = 0;
	pDevExt->NextAc5TxDoneIndex = 0;
	pDevExt->CurAc6TxIndex = 0;
	pDevExt->NextAc6TxDoneIndex = 0;
	pDevExt->CurAc7TxIndex = 0;
	pDevExt->NextAc7TxDoneIndex = 0;
	pDevExt->CurMgmtTxIndex = 0; // AC8
	pDevExt->NextMgmtTxDoneIndex = 0; // AC8
	pDevExt->CurHccaTxIndex = 0; // AC9
	pDevExt->NextHccaTxDoneIndex = 0; // AC9	

	pDevExt->Ac0_MAX_TX_PROCESS = 0;
	pDevExt->Ac1_MAX_TX_PROCESS = 0;
	pDevExt->Ac2_MAX_TX_PROCESS = 0;
	pDevExt->Ac3_MAX_TX_PROCESS = 0;
	pDevExt->Mgmt_MAX_TX_PROCESS = 0;
	pDevExt->Hcca_MAX_TX_PROCESS = 0;

	g_bDam4kBoundaryCheck = FALSE;
		
#ifdef	SNIFF
	pDevExt->bStartSniff = FALSE;
	pDevExt->PacketBufferDriverIdx = 0;
	pDevExt->PacketBufferSnifferIdx = 0;
	pDevExt->PacketBufferWrapAround = FALSE;
#endif
	
	pDevExt->bStartTx = FALSE;
	pDevExt->bStartRx = FALSE;

	pDevExt->AntennaDiversity = 0;
	pDevExt->SelAnt= 1;
	pDevExt->IsSaveRXVLog = 0 ;
	pDevExt->RXVFileHandle = 0 ;
	//pDevExt->RX0DataCount = 0;
	RtlZeroMemory(&pDevExt->OtherCounters, sizeof(pDevExt->OtherCounters));

	pDevExt->IsFwRsp = FALSE;

	{
		int		Index = 0,i;
		ULONG	MacCsr12;
		ULONG	TempReg, MacValue;
		WPDMA_GLO_CFG_STRUC	GloCfg;

		//
		// Make sure MAC gets ready.
		//
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("DriverEntry Make sure MAC gets ready. \n"));
		Index = 0;
		/*do --werner
		{
//			pDevExt->MACVersion = RTMP_Real_IO_READ32(pDevExt, RA_ASIC_VERSION);

			if ((pDevExt->MACVersion != 0x00) && (pDevExt->MACVersion != 0xFFFFFFFF))
				break;

			RTMPusecDelay(100);
		} while (Index++ < 1000);*/


		//
#if 1
		//1reset
		//RTMP_IO_READ32(pDevExt, 0x730,&TempReg);
		TempReg = 0;
		// check common register if FW has been loaded
		if((TempReg & 0x01) == 0)
		{			
			//RTMP_IO_READ32(pDevExt, RA_MISC_CTRL,&TempReg);
			TempReg |= 0x80000;
			//RTMP_Real_IO_WRITE32(pDevExt, RA_MISC_CTRL, TempReg);//--werner
			RTMPusecDelay(100);
			TempReg &= (~0x80000);
			//RTMP_Real_IO_WRITE32(pDevExt, RA_MISC_CTRL, TempReg);//--werner
		}	
#endif	
		Index = 0;
		/*do --werner
		{
//			pDevExt->MACVersion = RTMP_Real_IO_READ32(pDevExt, RA_ASIC_VERSION);

			if ((pDevExt->MACVersion != 0x00) && (pDevExt->MACVersion != 0xFFFFFFFF))
				break;

			RTMPusecDelay(100);
		} while (Index++ < 1000);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"RA_ASIC_VERSION  [ Ver:Rev=0x%08x]\n", pDevExt->MACVersion);*/

		TXWI_SIZE = GetTXWISize (pDevExt);
		Testdbg("RA_ASIC_VERSION : 0x%08X, TXWI_SIZE = %d\n", pDevExt->MACVersion, TXWI_SIZE);

		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"QA_Driver: FUNCTION: %s, LINE%d\n",__FUNCTION__,__LINE__);

		//DebugPrintWhereAmI();
	}

	
	// DMA initialization
	InitDMA(pDevExt);

	// Initialize Asic
	//Eeprom_Init(pDevExt);

	// Set Delay INT CFG
	//RTMP_Real_IO_WRITE32(pDevExt, RA_DELAY_INT_CFG, DELAY_VALUE);	

	// Enable interrupt			
	pDevExt->IntrMask = IRQ_MASK;
	RTMP_Real_IO_WRITE32(pDevExt, RA_INT_MASK, pDevExt->IntrMask|IRQ_MSK_SPECIAL_BITS);		

	// MAC TX/RX Enable	
	/*RTMP_Real_IO_WRITE32(pDevExt, RA_MAC_SYS_CTRL, 0xC);		

	// Enable bit1 for RX Queue in RA_PBF_CFG	
//	value = RTMP_Real_IO_READ32(pDevExt, RA_PBF_CFG);
	data = value | 0x00000002;
	RTMP_Real_IO_WRITE32(pDevExt, RA_PBF_CFG, data);
	*/

	// Disable L0s
	{
		ULONG	Bus = 0, length = 0, propertyAddress = 0;
		ULONG	Function = 0, Device = 0, Value = 0;

		//
		// Scan the PCI slots and dump the PCI configuration data to the
		// user's buffer.
		//
	    ReadWriteConfigSpace(pDO, 
								0, // 0 for read 1 for write
								&Value,
								0x80,
								4
								);
		Value &= 0xfffffefc;
		ReadWriteConfigSpace(pDO, 
								1, // 0 for read 1 for write
								&Value,
								0x80,
								4
								);
	}
	//DebugPrintWhereAmI();
{
	USHORT	Bus = 0, Slot = 0, Func = 0;
	ULONG	SubBus = 0, MTKSubBus=0, length = 0;
	ULONG	Vendor = 0;
	ULONG	DeviceID = 0;
	ULONG	Class = 0;
	ULONG	Configuration = 0;
	ULONG	offset = 0, i=0;

	IoGetDeviceProperty(pDevExt->pPhyDeviceObj,
						DevicePropertyBusNumber,
						sizeof(ULONG),
						(PVOID)&MTKSubBus,
						&length);
	//DebugPrintWhereAmI();
	// 2.Find PCI host, and its subbus is the same as Ralink's bus.
	for (Bus = 0; Bus < MAX_PCI_BUS; Bus++)
	{
		//DebugPrintWhereAmI();
		for (Slot = 0; Slot < MAX_PCI_DEVICE; Slot++)
		{
			//DebugPrintWhereAmI();
			for (Func = 0; Func < MAX_FUNC_NUM; Func++)
			{
				Vendor    = RTMPReadCBConfig(Bus, Slot, Func, 0x00);
				DeviceID = Vendor & 0xFF000000;
				Vendor    = Vendor & 0x0000FFFF;
				// Skip non-exist deice right away
				if (Vendor == UNKNOWN)
					continue;
				// AMD PCI host seems to has problem. So only support Intel PCI bus .
				// If Find Intel Vendor ID. Always use toggle.
				//DebugPrintWhereAmI();
				Class     = RTMPReadCBConfig(Bus, Slot, Func, 0x08) >> 16;
				SubBus    = RTMPReadCBConfig(Bus, Slot, Func, 0x18);
				SubBus &= 0xff00;
				SubBus = (SubBus >> 8);
		
				if ((Class == CARD_PCIBRIDGEPCI_CLASS) && (SubBus == MTKSubBus))
				{
					// find LinkControl offset according to PCI spec.
					offset = 0x34;
					i = 0;
					do 
					{
						Configuration = RTMPReadCBConfig(Bus, Slot, Func, offset);
						if ((Configuration&0xff) == 0x10)
						{	
							offset += 0x10;
							DeviceID = DeviceID>>16;
							Configuration = RTMPReadCBConfig(Bus, Slot, Func, offset);
							Configuration &= 0xfffffefc;
							Configuration |= (0x0);

							RTMPWriteCBConfig(Bus, Slot, Func, offset, Configuration);
							break;
						}
						else if (offset == 0x34)
						{
							offset = Configuration&0xff;
						}
						else
						{
							offset = (Configuration&0xff00) >> 8;
						}
						i++;
					}while (i < 10);
					break;
				}
			}
		}
	}
}
//DebugPrintWhereAmI();
	{
		////////////////////////////////////////////
		// Read MCFG table
		////////////////////////////////////////////
	        HANDLE				KeyHandle=NULL;
	        OBJECT_ATTRIBUTES	ObjectAttributes;
		UNICODE_STRING		UnicodeUSBPRINTRegPath;
		UNICODE_STRING		ValueName;
		PKEY_VALUE_PARTIAL_INFORMATION	KeyValueInformation = NULL;
		ULONG				ResultLength1, ResultLength2, iIdx=0;
		const char			szBuf[8]={0}, MCFG[] = {"MCFG"};
		const char* Ptr = NULL;

		RtlInitUnicodeString(&UnicodeUSBPRINTRegPath, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\mssmbios\\Data");
		RtlInitUnicodeString(&ValueName, L"AcpiData");

		InitializeObjectAttributes(&ObjectAttributes, &UnicodeUSBPRINTRegPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	        ntStatus = ZwOpenKey(&KeyHandle, KEY_QUERY_VALUE, &ObjectAttributes);
//DebugPrintWhereAmI();
		ntStatus = ZwQueryValueKey(
								KeyHandle,
								&ValueName,
								KeyValuePartialInformation,
								KeyValueInformation,
								sizeof(KEY_VALUE_PARTIAL_INFORMATION),
								&ResultLength1
								);
//DebugPrintWhereAmI();
		KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ResultLength1, 'MgeR');

		ntStatus = ZwQueryValueKey(
								KeyHandle,
								&ValueName,
								KeyValuePartialInformation,
								KeyValueInformation,
								ResultLength1,
								&ResultLength2
								);

		Ptr = (const char*)KeyValueInformation->Data;

		for (iIdx=0; iIdx<ResultLength1-sizeof(MCFG); iIdx++)
		{
			memcpy((void*)szBuf, (const void*)(Ptr + iIdx), strlen(MCFG));
			if ( strcmp(szBuf, MCFG) == 0 )
			{
				pDevExt->ConfigurationSpacePhyAddr.LowPart = *((ULONG*)(Ptr + iIdx + 0x2C));
				pDevExt->ConfigurationSpacePhyAddr.HighPart = *((ULONG*)(Ptr + iIdx + 0x30));
//				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "pDevExt->ConfigurationSpacePhyAddr.LowPart  = 0x%08X", pDevExt->ConfigurationSpacePhyAddr.LowPart);
//				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "pDevExt->ConfigurationSpacePhyAddr.HighPart  = 0x%08X", pDevExt->ConfigurationSpacePhyAddr.HighPart);
				break;
			}
		}
	//DebugPrintWhereAmI();	
		ExFreePoolWithTag(KeyValueInformation, 'MgeR');
		ntStatus = ZwClose(KeyHandle);
	}
//DebugPrintWhereAmI();
	{
		////////////////////////////////////////////
		// Read MCFG table
		////////////////////////////////////////////
		RTL_OSVERSIONINFOW	OSVersionInfo;
		OSVersionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

		if ( RtlGetVersion((RTL_OSVERSIONINFOW*)&OSVersionInfo) == STATUS_SUCCESS)
			pDevExt->OSMajorVersion = OSVersionInfo.dwMajorVersion;
		else
			pDevExt->OSMajorVersion = 5; // XP
	}

	return ntStatus;
}

NTSTATUS
BufferReadMemory(
	IN PDEVICE_OBJECT DeviceObject,
    IN PULONG pIBuffer,
    IN PUCHAR UserBuffer,
    IN ULONG BufferMaxSize,
    OUT PULONG ByteCount
    )
/*++

Routine Description:
	Read memory from assign location in defined format

Arguments:

    UserBuffer - Pointer to the user's storage location for the PCI configuration
                 data.

    BufferMaxSize - The maximum size of the user's storage area.

    ByteCount - The running count of the number of bytes currently stored in the
                user's buffer.  At the end of the searching, this is how many
                bytes to be returned to the caller.


Return Value:

    STATUS_SUCCESS if successful.
    Various NTSTATUS values if unsuccessful.

--*/
{
    NTSTATUS	ntStatus = STATUS_SUCCESS;
	ULONG		InBuf[5];
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)DeviceObject->DeviceExtension;

	InBuf[0] = *(pIBuffer+0);					//offset
	InBuf[1] = *(pIBuffer+1);					//length
	InBuf[2] = *(pIBuffer+2);					//value

	READ_REGISTER_BUFFER_ULONG((PULONG)pDevExt->virtualaddr + InBuf[0], (PULONG)UserBuffer, InBuf[1]/4);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL: BufferReadMemory()\n"));
	
	*ByteCount = InBuf[1];
    return ntStatus;
}   // BufferReadMemory

NTSTATUS
BufferWriteMemory(
	IN PULONG pIBuffer,
    IN PUCHAR UserBuffer,
    IN ULONG BufferMaxSize,
    OUT PULONG ByteCount
    )
/*++

Routine Description:

  	Read memory from assign location in defined length

Arguments:

    UserBuffer - Pointer to the user's storage location for the PCI configuration
                 data.

    BufferMaxSize - The maximum size of the user's storage area.

    ByteCount - The running count of the number of bytes currently stored in the
                user's buffer.  At the end of the searching, this is how many
                bytes to be returned to the caller.


Return Value:

    STATUS_SUCCESS if successful.
    Various NTSTATUS values if unsuccessful.

--*/
{
    NTSTATUS		ntStatus = STATUS_SUCCESS;
    ULONG			InBuf[5], Reg, Buf;
	
	InBuf[0] = *(pIBuffer);							//busnum
	InBuf[1] = *(pIBuffer+1);						//address
	InBuf[2] = *(pIBuffer+2);						//offset
	InBuf[3] = *(pIBuffer+3);						//length
	InBuf[4] = *(pIBuffer+4);						//value	

	Reg = InBuf[1]+InBuf[2];
	WRITE_REGISTER_BUFFER_ULONG((PULONG) &Reg, (PULONG)(pIBuffer+5), InBuf[3]/4);

//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"IOCTL: BufferWriteMemory()\n");
	
	*ByteCount = 0;
    return ntStatus;
}   // BufferWriteMemory



NTSTATUS
ReadMemory(
	IN PDEVICE_OBJECT DeviceObject,
    IN PULONG pIBuffer,
    IN PUCHAR UserBuffer,
    IN ULONG BufferMaxSize,
    OUT PULONG ByteCount
    )
/*++

Routine Description:
	Read memory from assign location in defined format

Arguments:

    UserBuffer - Pointer to the user's storage location for the PCI configuration
                 data.

    BufferMaxSize - The maximum size of the user's storage area.

    ByteCount - The running count of the number of bytes currently stored in the
                user's buffer.  At the end of the searching, this is how many
                bytes to be returned to the caller.


Return Value:

    STATUS_SUCCESS if successful.
    Various NTSTATUS values if unsuccessful.

--*/
{
    NTSTATUS	ntStatus = STATUS_SUCCESS;
	ULONG		InBuf[128];
	ULONG		data[128];
	ULONG temp = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)DeviceObject->DeviceExtension;
	RtlZeroMemory(data,sizeof(data));
	InBuf[0] = *(pIBuffer+0);			//offset
	InBuf[1] = *(pIBuffer+1);			//length
	InBuf[2] = *(pIBuffer+2);			//value
//	Testdbg("IOCTL: ReadMemory_4B[0x%08x] = 0x%08x\n", InBuf[0]), InBuf[2];	
	if(InBuf[0] &0xFF000000)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL: Error ReadMemory[0x%08x] = 0x%08x !!!!!\n", InBuf[0], InBuf[2]));
		*ByteCount = 0;
   		return ntStatus;
	}

	if(InBuf[1] == 1)
	{
		*data = READ_REGISTER_UCHAR((PUCHAR)pDevExt->virtualaddr + InBuf[0]);
//		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"IOCTL: ReadMemory_1B[0x%02x] = 0x%02x\n", (UCHAR)InBuf[0], (UCHAR)data);				
	}	
	else if(InBuf[1] == 2)
	{
		if ( (InBuf[0] % 2) == 0 )
		{
			*data = READ_REGISTER_USHORT((PUSHORT)pDevExt->virtualaddr + (InBuf[0]/2));
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"IOCTL: ReadMemory_2B[0x%04x] = 0x%04x\n", (USHORT)InBuf[0], (USHORT)data);
		}
		else
		{
			ULONG	tmp;
			*data	= 0;
			tmp = READ_REGISTER_UCHAR((PUCHAR)pDevExt->virtualaddr + InBuf[0]);
			*data |= tmp;
			tmp = READ_REGISTER_UCHAR((PUCHAR)pDevExt->virtualaddr + InBuf[0] + 1);
			*data |= tmp<<8;
		}
	}
	else if(InBuf[1] == 4)
	{
		if ( (InBuf[0] % 4) == 0 )
		{
			*data = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + (InBuf[0]/4));
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"IOCTL: ReadMemory_4B[0x%08x] = 0x%08x\n", InBuf[0]), data;
	//		Testdbg("IOCTL: ReadMemory_4B[0x%08x] = 0x%08x\n", InBuf[0]), InBuf[2];
		}
		else
		{
			ULONG	tmp;
			*data	= 0;
			tmp = READ_REGISTER_UCHAR((PUCHAR)pDevExt->virtualaddr + InBuf[0]);
			*data |= tmp;
			tmp = READ_REGISTER_UCHAR((PUCHAR)pDevExt->virtualaddr + InBuf[0] + 1);
			*data |= tmp<<8;
			tmp = READ_REGISTER_UCHAR((PUCHAR)pDevExt->virtualaddr + InBuf[0] + 2);
			*data |= tmp<<16;
			tmp = READ_REGISTER_UCHAR((PUCHAR)pDevExt->virtualaddr + InBuf[0] + 3);
			*data |= tmp<<24;
		}
	}
	/*else if(InBuf[1] == 8)
	{
		if ( (InBuf[0] % 4) == 0 )
		{
			data[0] = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + (InBuf[0]/4));
			data[1] = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + ((4+InBuf[0])/4));
		}
		
	}
	else if(InBuf[1] == 12)
	{
		if ( (InBuf[0] % 4) == 0 )
		{
			data[0] = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + (InBuf[0]/4));
			data[1] = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + ((4+InBuf[0])/4));
			data[2] = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + ((8+InBuf[0])/4));
		}
		
	}
	else if(InBuf[1] == 16)
	{
		if ( (InBuf[0] % 4) == 0 )
		{
			temp = InBuf[1];
			temp = temp/4;
			for (ULONG i=0 ;i<temp ;i++)
			{
				data[i] = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + ((i*4+InBuf[0])/4));
			}			
		}
		
	}*/
	else
	{
		if ( (InBuf[0] % 4) == 0 )
		{
			ULONG i;
			
			temp = InBuf[1];
			temp = temp/4;
			for (i=0 ;i<temp ;i++)
			{
				data[i] = READ_REGISTER_ULONG((PULONG)pDevExt->virtualaddr + ((i*4+InBuf[0])/4));
			}			
		}
	}


	RtlCopyMemory(UserBuffer,data,InBuf[1]);

    *ByteCount = InBuf[1];
	return ntStatus;
}   // ReadMemory


NTSTATUS
WriteMemory(
	IN PDEVICE_OBJECT DeviceObject,
	IN PULONG pIBuffer,
    IN PUCHAR UserBuffer,
    IN ULONG BufferMaxSize,
    OUT PULONG ByteCount
    )
/*++

Routine Description:
  	Read memory from assign location in defined format

Arguments:
    UserBuffer - Pointer to the user's storage location for the PCI configuration
                 data.
    BufferMaxSize - The maximum size of the user's storage area.
    ByteCount - The running count of the number of bytes currently stored in the
                user's buffer.  At the end of the searching, this is how many
                bytes to be returned to the caller.


Return Value:
    STATUS_SUCCESS if successful.
    Various NTSTATUS values if unsuccessful.

--*/
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    ULONG InBuf[5];
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)DeviceObject->DeviceExtension;
	
	InBuf[0] = *(pIBuffer + 0);			//offset
	InBuf[1] = *(pIBuffer + 1);			//length
	InBuf[2] = *(pIBuffer + 2);			//value

	if(InBuf[0] &0xFF000000)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL: Error WriteMemory[0x%08x] = 0x%08x !!!!!\n", InBuf[0], InBuf[2]));
		*ByteCount = 0;
   		return ntStatus;
	}

	if(InBuf[1] == 1)
	{
		WRITE_Real_REGISTER_UCHAR(((PUCHAR)pDevExt->virtualaddr + InBuf[0]), (UCHAR)InBuf[2]);
//		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"IOCTL: WriteMemory_1B[0x%02x] = 0x%02x\n", (UCHAR)InBuf[0], (UCHAR)InBuf[2]);				
	}	
	else if(InBuf[1] == 2)
	{
		WRITE_Real_REGISTER_USHORT(((PUSHORT)pDevExt->virtualaddr + InBuf[0]), (USHORT)InBuf[2]);
//		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"IOCTL: WriteMemory_2B[0x%04x] = 0x%04x\n", (USHORT)InBuf[0], (USHORT)InBuf[2]);				
	}
	else if(InBuf[1] == 4)
	{
		WRITE_Real_REGISTER_ULONG(((PULONG)pDevExt->virtualaddr + InBuf[0]), (ULONG)InBuf[2]);
//		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"IOCTL: WriteMemory_4B[0x%08x] = 0x%08x\n", InBuf[0], InBuf[2]);		
	}
	
	*ByteCount = 0;
    return ntStatus;
}   // WriteMemory


ULONG g_CntIsr = 0;
ULONG g_CntDpc = 0;

BOOLEAN Isr (IN PKINTERRUPT pIntObj,
			IN PVOID pServiceContext ) //msi
{
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pServiceContext;
//	INTSRC_STRUC	IntSource;
	BOOLEAN			RetValue = FALSE;//TRUE;
	ULONG			MaskValue, IntSourceValue, Tmp;
	
	// Disable interrupt

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr()  ==>\n"));

	RTMP_IO_READ32(pDevExt, RA_INT_MASK,&MaskValue);
	RTMP_IO_READ32(pDevExt, RA_INT_STATUS,&IntSourceValue);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IntSourceValue %x\n",IntSourceValue));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("MaskValue %x\n",MaskValue));
	MaskValue &= IRQ_MSK_SPECIAL_BITS;
	if ( (MaskValue&IRQ_MSK_SPECIAL_BITS) != IRQ_MSK_SPECIAL_BITS )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("MaskValue&IRQ_MSK_SPECIAL_BITS = %x return FALSE\n",MaskValue&IRQ_MSK_SPECIAL_BITS));
		return FALSE;
	}
	
	RTMP_IO_READ32(pDevExt, RA_INT_STATUS,&IntSourceValue);
	if ( IntSourceValue == 0 )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IntSourceValue = %x ,return FALSE\n",MaskValue));
		return FALSE;
	}
	
	//pDevExt->IntrMask = (IRQ_MASK)&0xFFFFFFFF;	
	RTMP_Real_IO_WRITE32(pDevExt, RA_INT_MASK, 0 /*pDevExt->IntrMask & ~IRQ_MSK_SPECIAL_BITS*/);
	RTMP_IO_READ32(pDevExt, RA_INT_MASK,&MaskValue);
	
	//
	// Get the interrupt sources & saved to local variable
	//

	RTMP_IO_READ32(pDevExt, RA_INT_STATUS,&pDevExt->IntSource );
	RTMP_Real_IO_WRITE32(pDevExt, RA_INT_STATUS, pDevExt->IntSource);
	RTMP_IO_READ32(pDevExt, RA_INT_STATUS,&Tmp);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("HWM_R[RA_INT_STATUS]  = %08x \n", pDevExt->IntSource));
	
	if(!(pDevExt->IntSource & IRQ_MASK))
	{
		RetValue = FALSE;
		
		pDevExt->IntrMask =0;	
		RTMP_Real_IO_WRITE32(pDevExt, RA_INT_MASK, pDevExt->IntrMask|IRQ_MSK_SPECIAL_BITS);
		RTMP_IO_READ32(pDevExt, RA_INT_STATUS,&Tmp);
		
		goto EXIT;
	}

	//Queue DPC routine
	RetValue = TRUE;
	IoRequestDpc(pDevExt->pDeviceObject, NULL, pDevExt);			

EXIT:
		g_CntIsr++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_CntIsr = 0x%08x\n", g_CntIsr));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_CntDpc = 0x%08x\n", g_CntDpc));		
		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr()  <==\n"));

	return RetValue;
}


//++
// Function:
//		RTPCIDpcForIsr
//
// Description:
//		This function performs the low-IRQL
//		post-processing of I/O requests
//
// Arguments:
//		Pointer to a DPC object
//		Pointer to the Device object
//		Pointer to the IRP for this request
//		Pointer to the Device Extension
//
// Return Value:
//		(None)
//--
VOID
RTPCIDpcForIsr(	IN PKDPC pDpc,
			IN PDEVICE_OBJECT pDevObj,
			IN PIRP pIrp,
			IN PVOID pContext )
{
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	ULONG	IntMaskValue;
//	INTSRC_STRUC	IntSource;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIDpcForIsr()  ==>\n"));

	//
	// Get the interrupt sources & saved to local variable
	//
	
	//IntSource.word = RTMP_Real_IO_READ32(pDevExt, RA_INT_STATUS);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("HWM_R[RA_INT_STATUS]  = %08x \n",pDevExt->IntSource));
	
	//RTMP_Real_IO_WRITE32(pDevExt, RA_INT_STATUS, IntSource.word);		

	//
	// Handle interrupt, walk through all bits
	// Should start from highest priority interrupt
	// The priority can be adjust by altering processing if statement
	//
	// If required spinlock, each interrupt service routine has to acquire and release itself.
	//
	
	if (pDevExt->IntSource&INT_Bit_ANY_TxDone)	
	{
		
		if (pDevExt->IntSource&INT_Bit_Ac0TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc0TxRingTxDoneInterrupt Check bit[4]\n"));
			RTMPHandleAc0TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac1TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc1TxRingTxDoneInterrupt Check bit[5]\n"));
			RTMPHandleAc1TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac2TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc2TxRingTxDoneInterrupt Check bit[6]\n"));
			RTMPHandleAc2TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac3TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc3TxRingTxDoneInterrupt Check bit[7]\n"));
			RTMPHandleAc3TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac4TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc4TxRingTxDoneInterrupt Check bit[8]\n"));
			RTMPHandleAc4TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac5TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc5TxRingTxDoneInterrupt Check bit[9]\n"));
			RTMPHandleAc5TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac6TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc6TxRingTxDoneInterrupt Check bit[10]\n"));
			RTMPHandleAc6TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac7TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc7TxRingTxDoneInterrupt Check bit[11]\n"));
			RTMPHandleAc7TxRingTxDoneInterrupt(pDevExt);
		}

		//++werner-->
		if (pDevExt->IntSource&INT_Bit_Ac8TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc8TxRingTxDoneInterrupt Check bit[12]\n"));
			//RTMPHandleAc7TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac9TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc9TxRingTxDoneInterrupt Check bit[13]\n"));
			//RTMPHandleAc7TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac10TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc10TxRingTxDoneInterrupt Check bit[14]\n"));
			//RTMPHandleAc7TxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_Ac11TxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleAc11TxRingTxDoneInterrupt Check bit[15]\n"));
			//RTMPHandleAc7TxRingTxDoneInterrupt(pDevExt);
		}
		//++werner<--

		if (pDevExt->IntSource&INT_Bit_MgmtTxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleMgmtTxRingTxDoneInterrupt Check bit[16]\n"));
			RTMPHandleMgmtTxRingTxDoneInterrupt(pDevExt);
		}

		if (pDevExt->IntSource&INT_Bit_HccaTxDone)	
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTMPHandleHccaTxRingTxDoneInterrupt Check bit[17]\n"));
			RTMPHandleHccaTxRingTxDoneInterrupt(pDevExt);
		}	
	 }


	if (pDevExt->IntSource & INT_Bit_ANY_RxDone)
	{
		if(pDevExt->IntSource &INT_Bit_RX0TxDone)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTPCIHandleRx0DoneInterrupt\n"));
			RTPCIHandleRx0DoneInterrupt(pDevExt);
		}

		if(pDevExt->IntSource &INT_Bit_RX1TxDone)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("====> RTPCIHandleRx1DoneInterrupt\n"));
			RTPCIHandleRx1DoneInterrupt(pDevExt);
		}
		
		
	}

	
	//
	// Re-enable the interrupt (disabled in Isr)
	//

	pDevExt->IntrMask = 0;	
	RTMP_Real_IO_WRITE32(pDevExt, RA_INT_MASK, pDevExt->IntrMask|IRQ_MSK_SPECIAL_BITS);
	RTMP_IO_READ32(pDevExt, RA_INT_MASK,&IntMaskValue);

	g_CntDpc++;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_CntIsr = 0x%08x\n", g_CntIsr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_CntDpc = 0x%08x\n", g_CntDpc));		

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TransmittedCount = %d\n", pDevExt->OtherCounters.TransmittedCount));	
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIDpcForIsr()  <==\n"));
/*	
	if ( pDevExt->WlanCounters.CRCErrorCount .LowPart != 0)
	{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"error stop\n");
			RTMP_Real_IO_WRITE32(pDevExt, RA_INT_MASK, 0);		
	}
*/	
	
}

VOID
 RTPCIHandleRx1DoneInterrupt(
			IN PVOID pContext )
{
	UCHAR				Count=0;
	PUCHAR				pData;
	PRXD_STRUC			pRxD, pSnifRxD;
	PRXINFO_STRUC		pRxInfo;
	PHEADER_802_11		pHeader;
	NTSTATUS				Status = STATUS_SUCCESS;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	ULONG				RX_CNT, RX_DRX_IDX, RX_CRX_IDX, value;
	USHORT				SeqNum;
	int					i;
	LONG				length;
	ULONG				TmpValue;
	int					Temp;
	ULONG				SchReg4;
	PULONG		pTemULONGptr;
	PUCHAR		pTemUCHARptr;
	BOOLEAN			bPrintReceiveData;
	FwCMDRspTxD_STRUC FwCMDRspTxD;
	//	BOOLEAN					bPayload_Error = FALSE;
	ULONG *pUlong = NULL;
	RX_INFO_DW0 RxInfoDW0;
	RX_V_GROUP3 RxVG3;
	CMD_CH_PRIVILEGE_T ChannelPayload;
	RXV			rxv;
	RXV_HEADER rxv_header;
	char			rxvPrintBuffer[1024];
	UNICODE_STRING     uniName;
    	OBJECT_ATTRIBUTES  objAttr;
	IO_STATUS_BLOCK    ioStatusBlock;

	KeAcquireSpinLockAtDpcLevel(&pDevExt->RxRingMainLock);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RXCheck: RTPCIHandleRx1DoneInterrupt()\n"));
	//Testdbg("PciDumpr: RXCheck: RTPCIHandleRxDoneInterrupt()\n");

	RTMP_IO_READ32(pDevExt, RA_FS_DRX_IDX1,&RX_DRX_IDX);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RX1Check:DTX = %d\n",RX_DRX_IDX));

	if (RX_DRX_IDX > pDevExt->CurRx1Index) 
		RX_CNT = RX_DRX_IDX -pDevExt->CurRx1Index;
	else
		RX_CNT = RX_DRX_IDX + RX0_RING_SIZE - pDevExt->CurRx1Index;

	do
	{
		PULONG	ptr;
		
		//====================================================//
		// Processing FW response
		//====================================================//
		/*{
			PFW_RXD_STRUC	pFwRxD;
			pFwRxD = (PFW_RXD_STRUC) pDevExt->Rx1RingMain[pDevExt->CurRx1Index].AllocVa;

			if (pFwRxD->DDONE != 1)
			{
				break;
			}
			
			if ( pFwRxD->INFO_TYPE == INFO_TYPE_RSP_EVT )
			{
				Testdbg("1111pFwRxD->INFO_TYPE == INFO_TYPE_RSP_EVT\n");
				Testdbg("1111pFwRxD->PktLength = %d, sizeof(FW_RXD_STRUC) = %d\n", pFwRxD->PktLength, sizeof(FW_RXD_STRUC));
				RtlCopyMemory(&pDevExt->FwReceiveBuffer, (UCHAR*)pFwRxD, pFwRxD->PktLength + sizeof(FW_RXD_STRUC));

				pDevExt->IsFwRsp = TRUE;
			}
		}*/
		//====================================================//
		
		pRxD = (PRXD_STRUC) pDevExt->Rx1RingMain[pDevExt->CurRx1Index].AllocVa;

		ptr = (PULONG)pRxD;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX1 RXD 0 0x%08X\n", *ptr));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX1 RXD 1 0x%08X\n", *(ptr+1)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX1 RXD 2 0x%08X\n", *(ptr+2)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX1 RXD 3 0x%08X\n", *(ptr+3)));

		if (pRxD->DDONE != 1)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RxD->DDONE != 1 !!!!!!\n"));
			//break;
		}

		do
		{
			PRX_WI_STRUC pRxWI;
			
			if ( pDevExt->IsFwRsp == TRUE )
			{
				pDevExt->IsFwRsp = FALSE;
				break;
			}
			
			pData	= (PUCHAR) (pDevExt->Rx1RingMain[pDevExt->CurRx1Index].DmaBuf.AllocVa);

			pRxInfo = (PRXINFO_STRUC) pData;
			bPrintReceiveData = FALSE;
			// Cast to 802.11 header for flags checking

			pHeader	= (PHEADER_802_11) ((PUCHAR)pData);//++werner	
			pRxWI = (PRX_WI_STRUC) (pData+sizeof(RXINFO_STRUC)); 
			pDevExt->OtherCounters.ReceivedCount++;
			//++werner-> dump receive data
			
			RTMP_IO_READ32(pDevExt, RA_SCHEDULER_REG4,&SchReg4);
			if(SchReg4&0x00000020)
			{
				PTX_WI_STRUC pTXWI;
				
				//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"ReceivedCount=%d\n", pDevExt->OtherCounters.ReceivedCount);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("ReceivedCount=%d\n", pDevExt->OtherCounters.ReceivedCount));

				//compare tx data and rx data 
				pTXWI=  (PTX_WI_STRUC) ((PUCHAR)pData);
				//compare length
				if(pRxD->SDL0!=g_ulTXPacketLength)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Packet compare error, RX data length=%d  PacketLength=%d\n", pRxD->SDL0,g_ulTXPacketLength));
					bPrintReceiveData = TRUE;
				}
				
				pTemULONGptr = (PULONG)pHeader;

				//compare TXD first, skip DW0
				if(memcmp((pTemULONGptr+1),(g_TxDPattern+4),sizeof(TX_WI_STRUC)-4)!=0)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXD compare ,fail\n"));
					bPrintReceiveData = TRUE;
				}

				//compare payload g_PayloadPattern
				if(memcmp((pTemULONGptr+TXWI_SIZE/4),g_PayloadPattern,g_ulTXPacketLength-TXWI_SIZE)!=0)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Payload compare ,fail\n"));
					bPrintReceiveData = TRUE;
				}


				//++werner-> dump receive data			
				Temp = pRxD->SDL0;

				if(bPrintReceiveData)
				{
					PULONG	ptr;
					
					Temp = pRxD->SDL0;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pRxD->SDL0= %d = 0x%x  Temp=%d\n", pRxD->SDL0, pRxD->SDL0,Temp));
					for (i = 0; i < Temp/4; i++)
					{
						ptr = (PULONG)pHeader;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Receive 4byte %d   0x%08X\n",i, *(ptr+i)));
					}
					//print TXD
					Temp = sizeof(g_TxDPattern);
					for (i = 0; i < Temp/4; i++)
					{
						ptr = (PULONG)g_TxDPattern;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_TxDPattern  %d   0x%08X\n",i, *(ptr+i)));
					}

					//print g_PayloadPattern, payload.
					Temp = g_ulTXPacketLength - sizeof(g_TxDPattern);
					for (i = 0; i < Temp/4; i++)
					{
						ptr = (PULONG)g_PayloadPattern;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_PayloadPattern  %d   0x%08X\n",i, *(ptr+i)));
					}

					
				}
				else
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rx1 Receive compare success\n"));

		
				//++werner<- dump receive data
			}
			

			

			///////////////////////////////////////////////////////////
			// Check for all RxD errors
			if (pRxInfo->CrcErr)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIHandleRx1DoneInterrupt CrcErr=%d ,break\n",pRxInfo->CrcErr));
				break; // give up this frame
			}

			if (pRxInfo->IcvErr || pRxInfo->MicErr)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIHandleRx1DoneInterrupt IcvErr=%d ,MicErr=%d ,break\n",pRxInfo->IcvErr,pRxInfo->MicErr));
				break;
			}

			// Check Last Section
			if (pRxD->LS0 != 1)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIHandleRx1DoneInterrupt pRxD->LS0 =%d != 1,break \n",pRxD->LS0));
				break; // give up this frame
			}

			//check if this packet is FW download rsp or packet cmd
			Temp = pRxD->SDL0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s SDL0 = %d, sizeof(FwCMDRspTxD) = %d\n",__FUNCTION__,Temp,sizeof(FwCMDRspTxD)));
			pUlong = (ULONG*)pData;			
			for (i=0; i<Temp/4; i++)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s RX DW%d  0x%08x\n",__FUNCTION__,i,*(pUlong+i)));
			}

			if(pRxD->SDL0>=sizeof(FwCMDRspTxD) )
			{
				memcpy(&FwCMDRspTxD,pData,sizeof(FwCMDRspTxD));
				if(0xE000==FwCMDRspTxD.FwEventTxD.u2PacketType)
				{	
					if(FwCMDRspTxD.FwEventTxD.ucSeqNum <= MAX_FW_DOWNLOAD_SEQ)
					{//fw download seq num
						g_FWSeqMCU = (UCHAR)FwCMDRspTxD.FwEventTxD.ucSeqNum;
						g_FWRspStatus = (UCHAR)FwCMDRspTxD.ucStatus;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s FW download  get seqnum = %d, from MCU  ,g_FWRspStatus =%d\n",__FUNCTION__,g_FWSeqMCU,g_FWRspStatus));

						//switch channel event seq is 0, so detect here.
						if (FW_PKT_CMD_CH_PRIVILEGE_EVENT==FwCMDRspTxD.FwEventTxD.ucEID)
						{
							if ( (sizeof(FwCMDRspTxD)+sizeof(CMD_CH_PRIVILEGE_T)) == FwCMDRspTxD.FwEventTxD.u2RxByteCount)
							{
								pData += sizeof(FwCMDRspTxD);
								memcpy(&ChannelPayload,pData,sizeof(ChannelPayload));	
								g_PacketCMDSeqMCU = ChannelPayload.ucTokenID;

								MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s Get switchchannel seqnum =%d \n",__FUNCTION__, g_PacketCMDSeqMCU));
							}
							else
							{
								MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s u2RxByteCount =%d, != sizeof(FwCMDRspTxD)+sizeof(CMD_CH_PRIVILEGE_T), error  !!!\n",__FUNCTION__, FwCMDRspTxD.FwEventTxD.u2RxByteCount));
							}

						}
					} 
					else
					{//packet cmd
						g_PacketCMDSeqMCU  = (UCHAR)FwCMDRspTxD.FwEventTxD.ucSeqNum;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s Packet CMD get seqnum = %d, from MCU, u2RxByteCount=%d\n",__FUNCTION__,g_PacketCMDSeqMCU,FwCMDRspTxD.FwEventTxD.u2RxByteCount ));
						if( (FwCMDRspTxD.FwEventTxD.u2RxByteCount-sizeof(FwTxD_STRUC))<= sizeof(g_PacketCMDRspData))
						{
							memset(g_PacketCMDRspData,0,sizeof(g_PacketCMDRspData));
							memcpy(g_PacketCMDRspData,pData,FwCMDRspTxD.FwEventTxD.u2RxByteCount);
						}
						else
						{
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s u2RxByteCount=%d > sizeof(g_PacketCMDRspData)(%d), error!!!!!\n",__FUNCTION__,FwCMDRspTxD.FwEventTxD.u2RxByteCount,sizeof(g_PacketCMDRspData)));
						}
						
					}
				}
				else
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("u2PacketType =0x%X not from FW(MCU) \n",FwCMDRspTxD.FwEventTxD.u2PacketType));
				}
			}

			//check if the packet is RX vector
#ifndef RTMP_USB_SUPPORT//always PCIe function here
			if(pRxD->SDL0>4)
			{
				HandleRXVector(pData,pDevExt);				
			}
#endif
			//
			// Do RxD release operation	for	all	failure	frames
			//
			if(Status == STATUS_SUCCESS)
			{
				// pData : Pointer skip	the	first 24 bytes,	802.11 HEADER
				//
				// Start of	main loop to parse receiving frames.
				// The sequence	will be	Type first,	then subtype...
				//			            
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pRxInfo->CrcErr = %d\n", pRxInfo->CrcErr));
				if(!pRxInfo->CrcErr)
				{			
					/*pDevExt->OtherCounters.RSSI0 = pRxWI->RSSI_0;
					pDevExt->OtherCounters.RSSI1 = pRxWI->RSSI_1;
					pDevExt->OtherCounters.RSSI2 = pRxWI->RSSI_2;
					pDevExt->OtherCounters.SNR0 = pRxWI->SNR_0;
					pDevExt->OtherCounters.SNR1 = pRxWI->SNR_1;

					pDevExt->OtherCounters.UdpErr = pRxD->UDPerr;
					pDevExt->OtherCounters.TcpErr = pRxD->TCPerr;
					pDevExt->OtherCounters.IpErr = pRxD->IPerr;*/

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pHeader->Controlhead.Frame.Type = %d\n", pHeader->Controlhead.Frame.Type));
					switch (pHeader->Controlhead.Frame.Type)
					{
						case BTYPE_DATA:
							if (pRxInfo->U2M)
							{
								INC_COUNTER(pDevExt->WlanCounters.U2MDataCount);
							}
							else	// ( !U2M || Mcast || Bcast )
							{
								INC_COUNTER(pDevExt->WlanCounters.OtherDataCount);
							}
							break;

						case BTYPE_MGMT:
						case BTYPE_CNTL:
							if(pHeader->Controlhead.Frame.Subtype == SUBTYPE_BEACON)
							{
								INC_COUNTER(pDevExt->WlanCounters.BeaconCount);
							}
							else
							{
								INC_COUNTER(pDevExt->WlanCounters.othersCount);
							}
							break;

						default	:
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RXCheck:  Unknow data type Error\n"));
							break;
					}
				}	 
			}

// Sniffer Function
#ifdef	SNIFF
			if(pDevExt->bStartSniff == TRUE)
			{
				if((pDevExt->PacketBufferDriverIdx >= RX_RING_PACKET_BUFFER) && (pDevExt->PacketBufferWrapAround == FALSE))
				{
					pDevExt->PacketBufferDriverIdx = 0;
					pDevExt->PacketBufferWrapAround = TRUE;

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RxDone ===> Wrap Around!\n"));
				}
				else if((pDevExt->PacketBufferDriverIdx >= RX_RING_PACKET_BUFFER) && (pDevExt->PacketBufferWrapAround == TRUE))
				{
					pDevExt->PacketBufferDriverIdx = 0;
					pDevExt->PacketBufferWrapAround = TRUE;

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RxDone ===> Error!!! Buffer Full!\n"));
				}

				pSnifRxD = (PRXD_STRUC) ((PUCHAR)pDevExt->pPacketBuf + pDevExt->PacketBufferDriverIdx * MAX_FRAME_SIZE);


				RtlCopyMemory((PUCHAR)pDevExt->pPacketBuf + (pDevExt->PacketBufferDriverIdx * MAX_FRAME_SIZE), 
								pRxD, RING_RX_DESCRIPTOR_SIZE);						// Descriptor
				RtlCopyMemory((PUCHAR)pDevExt->pPacketBuf + (pDevExt->PacketBufferDriverIdx * MAX_FRAME_SIZE) + RING_RX_DESCRIPTOR_SIZE,
								pData, pRxD->SDL0);

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RxDone ===> pDevExt->PacketBufferdriverIdx = %d\n",pDevExt->PacketBufferDriverIdx));

				pDevExt->PacketBufferDriverIdx++;
			}
#endif
		}while (FALSE);

		pRxD->DDONE= 0;
		pRxD->SDL0 = 8192;
		pRxD->LS0 = 0;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RXCheck: SetHw pRxD->DDONE = 0\n"));

		RTMP_Real_IO_WRITE32(pDevExt, RA_RX_CALC_IDX1, pDevExt->CurRx1Index);
		pDevExt->CurRx1Index++;

		if (pDevExt->CurRx1Index >= RX1_RING_SIZE)
		{
			pDevExt->CurRx1Index = 0;
		}

		Count++;

		RTMP_IO_READ32(pDevExt, RA_RX_CALC_IDX1,&TmpValue);

	}while (Count < RX_CNT);

	KeReleaseSpinLockFromDpcLevel(&pDevExt->RxRingMainLock);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s()<--\n",__FUNCTION__));
}


VOID
RTPCIHandleRx0DoneInterrupt(
			IN PVOID pContext )
{
	UCHAR				Count=0;
	PUCHAR				pData;
	PRXD_STRUC			pRxD, pSnifRxD;
	PRXINFO_STRUC		pRxInfo;
	PHEADER_802_11		pHeader;
	NTSTATUS				Status = STATUS_SUCCESS;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	ULONG				RX_CNT, RX_DRX_IDX, RX_CRX_IDX, value;
	USHORT				SeqNum;
	int					i;
	LONG				length;
	ULONG				TmpValue;
	int					Temp;
	ULONG				SchReg4;
	BOOLEAN					bPrintReceiveData;
//	BOOLEAN					bPayload_Error = FALSE;
	PULONG		pTemULONGptr;
	PUCHAR		pTemUCHARptr;
	ULONG iPattern2Location;
	FwCMDRspTxD_STRUC FwCMDRspTxD;
	RX_V_GROUP1 group1;
	memset(&group1,0,sizeof(group1));

	KeAcquireSpinLockAtDpcLevel(&pDevExt->RxRingMainLock);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RXCheck: RTPCIHandleRx0DoneInterrupt()\n"));
	//Testdbg("PciDumpr: RXCheck: RTPCIHandleRxDoneInterrupt()\n");

	RTMP_IO_READ32(pDevExt, RA_FS_DRX_IDX0,&RX_DRX_IDX);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RXCheck:DTX = %d\n",RX_DRX_IDX));
	//Testdbg("PciDumpr: RXCheck:DTX = %d\n",RX_DRX_IDX);
	if (RX_DRX_IDX > pDevExt->CurRx0Index) 
		RX_CNT = RX_DRX_IDX -pDevExt->CurRx0Index;
	else
		RX_CNT = RX_DRX_IDX + RX0_RING_SIZE - pDevExt->CurRx0Index;

	do
	{
		PULONG	ptr;
		
		bPrintReceiveData = FALSE;
		//====================================================//
		// Processing FW response
		//====================================================//
		{
			PFW_RXD_STRUC	pFwRxD;
			pFwRxD = (PFW_RXD_STRUC) pDevExt->Rx0RingMain[pDevExt->CurRx0Index].AllocVa;

			if (pFwRxD->DDONE != 1)
			{
				break;
			}
			
			if ( pFwRxD->INFO_TYPE == INFO_TYPE_RSP_EVT )
			{
				Testdbg("1111pFwRxD->INFO_TYPE == INFO_TYPE_RSP_EVT\n");
				Testdbg("1111pFwRxD->PktLength = %d, sizeof(FW_RXD_STRUC) = %d\n", pFwRxD->PktLength, sizeof(FW_RXD_STRUC));
				RtlCopyMemory(&pDevExt->FwReceiveBuffer, (UCHAR*)pFwRxD, pFwRxD->PktLength + sizeof(FW_RXD_STRUC));

				pDevExt->IsFwRsp = TRUE;
			}
		}
		//====================================================//
		
		pRxD = (PRXD_STRUC) pDevExt->Rx0RingMain[pDevExt->CurRx0Index].AllocVa;
		ptr = (PULONG)pRxD;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX0 RXD 0 0x%08X\n", *ptr));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX0 RXD 1 0x%08X\n", *(ptr+1)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX0 RXD 2 0x%08X\n", *(ptr+2)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RX0 RXD 3 0x%08X\n", *(ptr+3)));	

		if(*ptr&0x70000000)
		{
			pDevExt->OtherCounters.Rx0ReceivedCount++;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Driver Rx Count = %d\n",pDevExt->OtherCounters.Rx0ReceivedCount));
		}

		if (pRxD->DDONE != 1)
		{
			break;
		}

		do
		{
			PRX_WI_STRUC pRxWI;
			PTX_WI_STRUC pTXWI;
			
			if ( pDevExt->IsFwRsp == TRUE )
			{
				pDevExt->IsFwRsp = FALSE;
				break;
			}
			
			pData	= (PUCHAR) (pDevExt->Rx0RingMain[pDevExt->CurRx0Index].DmaBuf.AllocVa);

			pRxInfo = (PRXINFO_STRUC) pData;

			// Cast to 802.11 header for flags checking
			pHeader	= (PHEADER_802_11) ((PUCHAR)pData);//++werner	
			pRxWI = (PRX_WI_STRUC) (pData+sizeof(RXINFO_STRUC)); 

			pDevExt->OtherCounters.ReceivedCount++;
			//++werner-> dump receive data
			
			RTMP_IO_READ32(pDevExt, RA_SCHEDULER_REG4,&SchReg4);
			if(SchReg4&0x00000020)//loopback mode
			{	
				//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"ReceivedCount=%d\n", pDevExt->OtherCounters.ReceivedCount);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("ReceivedCount=%d\n", pDevExt->OtherCounters.ReceivedCount));
				//compare tx data and rx data 
				pTXWI=  (PTX_WI_STRUC) ((PUCHAR)pData);
				//compare length
				if(pRxD->SDL0!=g_ulTXPacketLength)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Packet compare error, RX data length=%d  PacketLength=%d\n", pRxD->SDL0,g_ulTXPacketLength));
					bPrintReceiveData = TRUE;
				}
				
				pTemULONGptr = (PULONG)pHeader;

				//compare TXD first, skip DW0
				if(memcmp((pTemULONGptr+1),(g_TxDPattern+4),sizeof(TX_WI_STRUC)-4)!=0)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXD compare ,fail\n"));
					bPrintReceiveData = TRUE;
				}

				//compare payload g_PayloadPattern
				if(memcmp((pTemULONGptr+TXWI_SIZE/4),g_PayloadPattern,g_ulTXPacketLength-TXWI_SIZE)!=0)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Payload compare ,fail\n"));
					bPrintReceiveData = TRUE;
				}
				
				if(bPrintReceiveData)
				{
					Temp = pRxD->SDL0;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pRxD->SDL0= %d = 0x%x  Temp=%d\n", pRxD->SDL0, pRxD->SDL0,Temp));
					for (i = 0; i < Temp/4; i++)
					{
						PULONG	ptr = (PULONG)pHeader;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Receive 4byte %d   0x%08X\n",i, *(ptr+i)));
					}
					//print TXD
					Temp = sizeof(g_TxDPattern);
					for (i = 0; i < Temp/4; i++)
					{
						PULONG	ptr = (PULONG)g_TxDPattern;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_TxDPattern  %d   0x%08X\n",i, *(ptr+i)));
					}

					//print g_PayloadPattern, payload.
					Temp = g_ulTXPacketLength - sizeof(g_TxDPattern);
					for (i = 0; i < Temp/4; i++)
					{
						PULONG	ptr = (PULONG)g_PayloadPattern;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_PayloadPattern  %d   0x%08X\n",i, *(ptr+i)));
					}

					
				}
				else
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rx0 Receive compare success\n"));
					//for test
					Temp = pRxD->SDL0;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pRxD->SDL0= %d = 0x%x  Temp=%d\n", pRxD->SDL0, pRxD->SDL0,Temp));
					for (i = 0; i < Temp/4; i++)
					{
						PULONG	ptr = (PULONG)pHeader;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Receive 4byte %d   0x%08X\n",i, *(ptr+i)));
					}
					//print TXD
					Temp = sizeof(g_TxDPattern);
					for (i = 0; i < Temp/4; i++)
					{
						PULONG	ptr = (PULONG)g_TxDPattern;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_TxDPattern  %d   0x%08X\n",i, *(ptr+i)));
					}

					//print g_PayloadPattern, payload.
					Temp = g_ulTXPacketLength - sizeof(g_TxDPattern);
					for (i = 0; i < Temp/4; i++)
					{
						PULONG	ptr = (PULONG)g_PayloadPattern;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("g_PayloadPattern  %d   0x%08X\n",i, *(ptr+i)));
					}

				}
							
				
			}
			else
			{//normal receive data
				Temp = pRxD->SDL0;
				if(Temp>=80)
				{
					Temp = 80;
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=======Dump receive packet===Max 20 dword======\n"));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("receive length=%d\n", Temp));
				ptr = (PULONG)pHeader;
				for (i = 0; i < Temp/4; i++)
				{					
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Receive 4byte %d   0x%08X\n",i, *(ptr+i)));					
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=======================================\n"));

#ifndef RTMP_USB_SUPPORT//always PCIe function here
				CheckSecurityResult(ptr,pDevExt);
#endif
			}
			//++werner<- dump receive data

			///////////////////////////////////////////////////////////
			// Check for all RxD errors
			/*if (pRxInfo->CrcErr)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIHandleRx0DoneInterrupt CrcErr=%d ,break\n",pRxInfo->CrcErr));
				break; // give up this frame
			}

			if (pRxInfo->IcvErr || pRxInfo->MicErr)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIHandleRx0DoneInterrupt IcvErr=%d ,MicErr=%d ,break\n",pRxInfo->IcvErr,pRxInfo->MicErr));
				break;
			}*/

			// Check Last Section
			if (pRxD->LS0 != 1)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTPCIHandleRx0DoneInterrupt pRxD->LS0 =%d != 1,break \n",pRxD->LS0));
				break; // give up this frame
			}
			
			//check if this packet is FW download rsp or packet cmd
			if(pRxD->SDL0==sizeof(FwCMDRspTxD) )
			{
				memcpy(&FwCMDRspTxD,pData,sizeof(FwCMDRspTxD));
				if(0xE000==FwCMDRspTxD.FwEventTxD.u2PacketType)
				{	
					if(FwCMDRspTxD.FwEventTxD.ucSeqNum <= MAX_FW_DOWNLOAD_SEQ)
					{//fw download seq num
						g_FWSeqMCU = (UCHAR)FwCMDRspTxD.FwEventTxD.ucSeqNum;
						g_FWRspStatus = (UCHAR)FwCMDRspTxD.ucStatus;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s FW download  get seqnum = %d, from MCU  ,g_FWRspStatus =%d\n",__FUNCTION__,g_FWSeqMCU,g_FWRspStatus));
					}
					else
					{//packet cmd
						g_PacketCMDSeqMCU  = (UCHAR)FwCMDRspTxD.FwEventTxD.ucSeqNum;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s Packet CMD get seqnum = %d, from MCU, u2RxByteCount=%d\n",__FUNCTION__,g_PacketCMDSeqMCU,FwCMDRspTxD.FwEventTxD.u2RxByteCount ));
						if( (FwCMDRspTxD.FwEventTxD.u2RxByteCount-sizeof(FwTxD_STRUC))<= sizeof(g_PacketCMDRspData))
						{
							memset(g_PacketCMDRspData,0,sizeof(g_PacketCMDRspData));
							memcpy(g_PacketCMDRspData,pData+sizeof(FwTxD_STRUC),FwCMDRspTxD.FwEventTxD.u2RxByteCount-sizeof(FwTxD_STRUC));
						}
						else
						{
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s u2RxByteCount-12 =0%d > sizeof  g_PacketCMDRspData, error!!!!!\n",__FUNCTION__,FwCMDRspTxD.FwEventTxD.u2RxByteCount-12));
						}
						
					}
				}
				else
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("u2PacketType =0x%X error \n",FwCMDRspTxD.FwEventTxD.u2PacketType));
				}
			}
			

			//
			// Do RxD release operation	for	all	failure	frames
			//
			if(Status == STATUS_SUCCESS)
			{
				// pData : Pointer skip	the	first 24 bytes,	802.11 HEADER
				//
				// Start of	main loop to parse receiving frames.
				// The sequence	will be	Type first,	then subtype...
				//			            
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pRxInfo->CrcErr = %d\n", pRxInfo->CrcErr));
				if(!pRxInfo->CrcErr)
				{			
					pDevExt->OtherCounters.RSSI0 = pRxWI->RSSI_0;
					pDevExt->OtherCounters.RSSI1 = pRxWI->RSSI_1;
					pDevExt->OtherCounters.RSSI2 = pRxWI->RSSI_2;
					pDevExt->OtherCounters.SNR0 = pRxWI->SNR_0 -16;
					pDevExt->OtherCounters.SNR1 = pRxWI->SNR_1 -16;

					pDevExt->OtherCounters.UdpErr = pRxD->UDPerr;
					pDevExt->OtherCounters.TcpErr = pRxD->TCPerr;
					pDevExt->OtherCounters.IpErr = pRxD->IPerr;

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pHeader->Controlhead.Frame.Type = %d\n", pHeader->Controlhead.Frame.Type));
					switch (pHeader->Controlhead.Frame.Type)
					{
						case BTYPE_DATA:
							if (pRxInfo->U2M)
							{
								INC_COUNTER(pDevExt->WlanCounters.U2MDataCount);
							}
							else	// ( !U2M || Mcast || Bcast )
							{
								INC_COUNTER(pDevExt->WlanCounters.OtherDataCount);
							}
							break;

						case BTYPE_MGMT:
						case BTYPE_CNTL:
							if(pHeader->Controlhead.Frame.Subtype == SUBTYPE_BEACON)
							{
								INC_COUNTER(pDevExt->WlanCounters.BeaconCount);
							}
							else
							{
								INC_COUNTER(pDevExt->WlanCounters.othersCount);
							}
							break;

						default	:
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RXCheck:  Unknow data type Error\n"));
							break;
					}
				}	 
			}

// Sniffer Function
#ifdef	SNIFF
			if(pDevExt->bStartSniff == TRUE)
			{
				if((pDevExt->PacketBufferDriverIdx >= RX_RING_PACKET_BUFFER) && (pDevExt->PacketBufferWrapAround == FALSE))
				{
					pDevExt->PacketBufferDriverIdx = 0;
					pDevExt->PacketBufferWrapAround = TRUE;

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RxDone ===> Wrap Around!\n"));
				}
				else if((pDevExt->PacketBufferDriverIdx >= RX_RING_PACKET_BUFFER) && (pDevExt->PacketBufferWrapAround == TRUE))
				{
					pDevExt->PacketBufferDriverIdx = 0;
					pDevExt->PacketBufferWrapAround = TRUE;

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RxDone ===> Error!!! Buffer Full!\n"));
				}

				pSnifRxD = (PRXD_STRUC) ((PUCHAR)pDevExt->pPacketBuf + pDevExt->PacketBufferDriverIdx * MAX_FRAME_SIZE);

				//jliao [040816 - P4]
				//pRxD->High32TSF = RTMP_Dummy_IO_READ32(pDevExt, CSR17);		// TSF value
				//pRxD->Low32TSF = RTMP_Dummy_IO_READ32(pDevExt, CSR16);		// TSF vlaue

				RtlCopyMemory((PUCHAR)pDevExt->pPacketBuf + (pDevExt->PacketBufferDriverIdx * MAX_FRAME_SIZE), 
								pRxD, RING_RX_DESCRIPTOR_SIZE);						// Descriptor
				RtlCopyMemory((PUCHAR)pDevExt->pPacketBuf + (pDevExt->PacketBufferDriverIdx * MAX_FRAME_SIZE) + RING_RX_DESCRIPTOR_SIZE,
								pData, pRxD->SDL0);

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RxDone ===> pDevExt->PacketBufferdriverIdx = %d\n",pDevExt->PacketBufferDriverIdx));

				pDevExt->PacketBufferDriverIdx++;
			}
#endif
		}while (FALSE);

		pRxD->DDONE= 0;
		pRxD->SDL0 = 8192;
		pRxD->LS0 = 0;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr: RXCheck: SetHw pRxD->DDONE = 0\n"));

		RTMP_Real_IO_WRITE32(pDevExt, RA_RX_CALC_IDX0, pDevExt->CurRx0Index);
		pDevExt->CurRx0Index++;

		if (pDevExt->CurRx0Index >= RX0_RING_SIZE)
		{
			pDevExt->CurRx0Index = 0;
		}

		Count++;

		RTMP_IO_READ32(pDevExt, RA_RX_CALC_IDX0,&TmpValue);

	}while (Count < RX_CNT);

	KeReleaseSpinLockFromDpcLevel(&pDevExt->RxRingMainLock);
}

VOID
RTMPHandleAc0TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc0RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX0, TX_DTX_IDX0, FREE, Tmp_Length;
	PULONG				pulTemp;


	do
	{  	       	      
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc0TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc0TxIndex= %d, NextAc0TxDoneIndex= %d\n", pDevExt->CurAc0TxIndex,pDevExt->NextAc0TxDoneIndex));	

		Testdbg("\n");
		Testdbg("PciDumpr:  CurAc0TxIndex= %d, NextAc0TxDoneIndex= %d\n", pDevExt->CurAc0TxIndex,pDevExt->NextAc0TxDoneIndex);	


		pTxD = (PTXD_STRUC)	pDevExt->Ac0RingMain[pDevExt->NextAc0TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
		{
			Testdbg("pTxD->DDONE != 1\n");			
			Testdbg("pDevExt->Ac0_MAX_TX_PROCESS = %d\n", pDevExt->Ac0_MAX_TX_PROCESS);
			Testdbg("pDevExt->CurAc0TxIndex = %d\n", pDevExt->CurAc0TxIndex);
			Testdbg("pDevExt->NextAc0TxDoneIndex = %d\n", pDevExt->NextAc0TxDoneIndex);
			break;
		}

//		Testdbg("pDevExt->CurAc0TxIndex = %d\n", pDevExt->CurAc0TxIndex);
//		Testdbg("pDevExt->NextAc0TxDoneIndex = %d\n", pDevExt->NextAc0TxDoneIndex);
		if(pTxD->DDONE)
		{
			pTxD->DDONE = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok, set DDONE=0\n"));
		}
		else
		{			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE isn't trueok, may error happen\n"));	
		}
		
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac0TxedCount++;		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: TransmittedCount=%d  Ac0TxedCount=%d\n",pDevExt->OtherCounters.TransmittedCount,pDevExt->OtherCounters.Ac0TxedCount));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Count = %d  ,  pDevExt->Ac0_MAX_TX_PROCESS = %d\n", Count, pDevExt->Ac0_MAX_TX_PROCESS));
		//Testdbg ("Count = %d  ,  pDevExt->Ac0_MAX_TX_PROCESS = %d\n", Count, pDevExt->Ac0_MAX_TX_PROCESS);
		RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX0,&TX_CTX_IDX0);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  RA_TX_CTX_IDX0 = %d\n", TX_CTX_IDX0));
		//Testdbg ("PciDumpr:  RA_TX_CTX_IDX0 = %d\n", TX_CTX_IDX0);
		RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX0,&TX_DTX_IDX0);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  RA_TX_DTX_IDX0 = %d\n", TX_DTX_IDX0));
		//Testdbg ("PciDumpr:  RA_TX_DTX_IDX0 = %d\n", TX_DTX_IDX0);
		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
			//Testdbg("TXT: transmit next packet\n");
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                                         
			if (TX_DTX_IDX0 > pDevExt->CurAc0TxIndex) 
				FREE = TX_DTX_IDX0 -pDevExt->CurAc0TxIndex -1;
			else
				FREE = TX_DTX_IDX0 + AC0_RING_SIZE -pDevExt->CurAc0TxIndex -1;

			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
				if (pDevExt->SetTXWI_NSEQ == 0)
				{
					if(pDevExt->FrameType == 21)
					{
						int j=pDevExt->NextAc0TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac0RingMain[j].DmaBuf.AllocVa, 0);
					}
					else
					{
						int j=pDevExt->NextAc0TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac0RingMain[j].DmaBuf.AllocVa, 10);
					}
				}
			}
			
			SEQdbg ("FREE = %d\n", FREE);
			if (  FREE >=1 )
			{					       
                            
				pTxD = (PTXD_STRUC)	pDevExt->Ac0RingMain[pDevExt->CurAc0TxIndex].AllocVa;

				if (pDevExt->RingType&0x01000000) // QoS Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE - 2;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0));
					//Testdbg("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0);
					pAc0RingTxWI = (PTX_WI_STRUC) pDevExt->Ac0RingMain[pDevExt->CurAc0TxIndex].DmaBuf.AllocVa;
					//pAc0RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc0RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc0RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				if(pDevExt->FrameType == 17)		// Random length
				{				   
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s() Random length\n",__FUNCTION__));
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc0RingTxWI = (PTX_WI_STRUC) pDevExt->Ac0RingMain[pDevExt->CurAc0TxIndex].DmaBuf.AllocVa;
					//pAc0RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc0RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc0RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s()Inc length\n",__FUNCTION__));
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;	
					
					pTxD->SDL0 = pDevExt->CurLength;	

					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}

					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc0RingTxWI = (PTX_WI_STRUC) pDevExt->Ac0RingMain[pDevExt->CurAc0TxIndex].DmaBuf.AllocVa;
					//pAc0RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc0RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					Testdbg("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0);
					Testdbg("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length);
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc0RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;
                            
				pDevExt->CurAc0TxIndex++;
				if(pDevExt->CurAc0TxIndex >= AC0_RING_SIZE)				
					pDevExt->CurAc0TxIndex = 0;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CurAc0TxIndex++ = %d\n", pDevExt->CurAc0TxIndex));
				Testdbg ("CurAc0TxIndex++ = %d\n", pDevExt->CurAc0TxIndex);

			}
			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Write CurAc0TxIndex = %d\n", pDevExt->NextAc0TxDoneIndex));
			RTMP_Real_IO_WRITE32(pDevExt,RA_TX_CTX_IDX0,	pDevExt->CurAc0TxIndex  );			
			
		}

		pDevExt->NextAc0TxDoneIndex++;
		if (pDevExt->NextAc0TxDoneIndex >= AC0_RING_SIZE)
			pDevExt->NextAc0TxDoneIndex = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("NextAc0TxDoneIndex++ = %d\n", pDevExt->NextAc0TxDoneIndex));
		Testdbg ("NextAc0TxDoneIndex++ = %d\n", pDevExt->NextAc0TxDoneIndex);

	}while(++Count < pDevExt->Ac0_MAX_TX_PROCESS);
}

VOID
RTMPHandleAc1TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc1RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX1, TX_DTX_IDX1, FREE, Tmp_Length;
	PULONG				pulTemp;


	do
	{  	       	      
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc1TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc1TxIndex= %d, NextAc1TxDoneIndex= %d\n", pDevExt->CurAc1TxIndex,pDevExt->NextAc1TxDoneIndex));	

		Testdbg("\n");
		Testdbg("PciDumpr:  CurAc1TxIndex= %d, NextAc1TxDoneIndex= %d\n", pDevExt->CurAc1TxIndex,pDevExt->NextAc1TxDoneIndex);	


		pTxD = (PTXD_STRUC)	pDevExt->Ac1RingMain[pDevExt->NextAc1TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
		{
//			Testdbg("pTxD->DDONE != 1\n");			
//			Testdbg("pDevExt->Ac1_MAX_TX_PROCESS = %d\n", pDevExt->Ac1_MAX_TX_PROCESS);
//			Testdbg("pDevExt->CurAc1TxIndex = %d\n", pDevExt->CurAc1TxIndex);
//			Testdbg("pDevExt->NextAc1TxDoneIndex = %d\n", pDevExt->NextAc1TxDoneIndex);
			break;
		}

//		Testdbg("pDevExt->CurAc1TxIndex = %d\n", pDevExt->CurAc1TxIndex);
//		Testdbg("pDevExt->NextAc1TxDoneIndex = %d\n", pDevExt->NextAc1TxDoneIndex);
		
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		Testdbg("TXT:tx descriptor DDONE ok\n");
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac1TxedCount++;				
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Count = %d  ,  pDevExt->Ac1_MAX_TX_PROCESS = %d\n", Count, pDevExt->Ac1_MAX_TX_PROCESS));
		Testdbg ("Count = %d  ,  pDevExt->Ac1_MAX_TX_PROCESS = %d\n", Count, pDevExt->Ac1_MAX_TX_PROCESS);
		RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX1,&TX_CTX_IDX1);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  RA_TX_CTX_IDX1 = %d\n", TX_CTX_IDX1));
		Testdbg ("PciDumpr:  RA_TX_CTX_IDX1 = %d\n", TX_CTX_IDX1);
		RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX1,&TX_DTX_IDX1);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  RA_TX_DTX_IDX1 = %d\n", TX_DTX_IDX1));
		Testdbg ("PciDumpr:  RA_TX_DTX_IDX1 = %d\n", TX_DTX_IDX1);

		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
			Testdbg("TXT: transmit next packet\n");
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)
				pDevExt->OtherCounters.TxRepeatCount--;
  
			if (TX_DTX_IDX1 > pDevExt->CurAc1TxIndex) 
				FREE = TX_DTX_IDX1 -pDevExt->CurAc1TxIndex -1;
			else
				FREE = TX_DTX_IDX1 + AC1_RING_SIZE -pDevExt->CurAc1TxIndex -1;

			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
				if (pDevExt->SetTXWI_NSEQ == 0)
				{
					if(pDevExt->FrameType == 21)
					{
						int j=pDevExt->NextAc1TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac1RingMain[j].DmaBuf.AllocVa, 0);
					}
					else
					{
						int j=pDevExt->NextAc1TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac1RingMain[j].DmaBuf.AllocVa, 10);
					}
				}
			}
			
			SEQdbg ("FREE = %d\n", FREE);
			if (  FREE >=1 )
			{					       
                            
				pTxD = (PTXD_STRUC)	pDevExt->Ac1RingMain[pDevExt->CurAc1TxIndex].AllocVa;

				if (pDevExt->RingType&0x01000000) // QoS Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE - 2;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0));
					Testdbg("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0);
					pAc1RingTxWI = (PTX_WI_STRUC) pDevExt->Ac1RingMain[pDevExt->CurAc1TxIndex].DmaBuf.AllocVa;
					//pAc1RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc1RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc1RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				if(pDevExt->FrameType == 17)		// Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc1RingTxWI = (PTX_WI_STRUC) pDevExt->Ac1RingMain[pDevExt->CurAc1TxIndex].DmaBuf.AllocVa;
					//pAc1RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc1RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc1RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;	
					
					pTxD->SDL0 = pDevExt->CurLength;	

					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}

					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc1RingTxWI = (PTX_WI_STRUC) pDevExt->Ac1RingMain[pDevExt->CurAc1TxIndex].DmaBuf.AllocVa;
					//pAc1RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc1RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					Testdbg("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0);
					Testdbg("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length);
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc1RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;
                            
				pDevExt->CurAc1TxIndex++;
				if(pDevExt->CurAc1TxIndex >= AC1_RING_SIZE)				
					pDevExt->CurAc1TxIndex = 0;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CurAc1TxIndex++ = %d\n", pDevExt->CurAc1TxIndex));
				Testdbg ("CurAc1TxIndex++ = %d\n", pDevExt->CurAc1TxIndex);

			}
			
			RTMP_Real_IO_WRITE32(pDevExt,RA_TX_CTX_IDX1,	pDevExt->CurAc1TxIndex  );			
			
		}

		pDevExt->NextAc1TxDoneIndex++;
		if (pDevExt->NextAc1TxDoneIndex >= AC1_RING_SIZE)
			pDevExt->NextAc1TxDoneIndex = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("NextAc1TxDoneIndex++ = %d\n", pDevExt->NextAc1TxDoneIndex));
		Testdbg ("NextAc1TxDoneIndex++ = %d\n", pDevExt->NextAc1TxDoneIndex);

	}while(++Count < pDevExt->Ac1_MAX_TX_PROCESS);
}

VOID
RTMPHandleAc2TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc2RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX2, TX_DTX_IDX2, FREE, Tmp_Length;
	PULONG				pulTemp;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc2TxRingTxDoneInterrupt()\n"));
	do
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc2TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc2TxIndex= %d, NextAc2TxDoneIndex= %d\n", pDevExt->CurAc2TxIndex,pDevExt->NextAc2TxDoneIndex));	
		pTxD = (PTXD_STRUC)	pDevExt->Ac2RingMain[pDevExt->NextAc2TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
			break;
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac2TxedCount++;	
		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                    
		       if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
			       if (pDevExt->SetTXWI_NSEQ == 0)
                    	       {
                                   if(pDevExt->FrameType == 21)
                                   {
                                        int j=pDevExt->NextAc2TxDoneIndex;
                                        IncSequenceNumber((UCHAR*)pDevExt->Ac2RingMain[j].DmaBuf.AllocVa, 2);
                                   }
		                     else
		                     {
		                          int j=pDevExt->NextAc2TxDoneIndex;
                                        IncSequenceNumber((UCHAR*)pDevExt->Ac2RingMain[j].DmaBuf.AllocVa, 10);
		                      }
                            }
			}
 

			RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX2,&TX_CTX_IDX2);
			RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX2,&TX_DTX_IDX2);
			if (TX_DTX_IDX2 > pDevExt->CurAc2TxIndex) 
				FREE = TX_DTX_IDX2 -pDevExt->CurAc2TxIndex -1;
			else
				FREE = TX_DTX_IDX2+ AC2_RING_SIZE -pDevExt->CurAc2TxIndex -1;
			
			if (  FREE >=1)
			{

				pTxD = (PTXD_STRUC)	pDevExt->Ac2RingMain[pDevExt->CurAc2TxIndex].AllocVa;
				
				if((pDevExt->FrameType == 17) || (pDevExt->RingType&0x01000000))		// Random length
				{
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc2RingTxWI = (PTX_WI_STRUC) pDevExt->Ac2RingMain[pDevExt->CurAc2TxIndex].DmaBuf.AllocVa;
					//pAc2RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc2RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc2RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)
						pDevExt->CurLength = pDevExt->MinLength;					
					
					pTxD->SDL0 = pDevExt->CurLength;				
					
					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}
					
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc2RingTxWI = (PTX_WI_STRUC) pDevExt->Ac2RingMain[pDevExt->CurAc2TxIndex].DmaBuf.AllocVa;
					//pAc2RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc2RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc2RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;

				pDevExt->CurAc2TxIndex++;
				if(pDevExt->CurAc2TxIndex >= AC2_RING_SIZE)				
					pDevExt->CurAc2TxIndex = 0;
				
			}
			RTMP_Real_IO_WRITE32(pDevExt,RA_TX_CTX_IDX2,	pDevExt->CurAc2TxIndex  );
			
		}

		pDevExt->NextAc2TxDoneIndex++;
		if (pDevExt->NextAc2TxDoneIndex >= AC2_RING_SIZE)
			pDevExt->NextAc2TxDoneIndex = 0;

	}while(++Count < pDevExt->Ac2_MAX_TX_PROCESS);
}

VOID
RTMPHandleAc3TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc3RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX3, TX_DTX_IDX3, FREE, Tmp_Length;
	PULONG				pulTemp;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc3TxRingTxDoneInterrupt()\n"));
	do
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc3TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc3TxIndex= %d, NextAc3TxDoneIndex= %d\n", pDevExt->CurAc3TxIndex,pDevExt->NextAc3TxDoneIndex));	
		pTxD = (PTXD_STRUC)	pDevExt->Ac3RingMain[pDevExt->NextAc3TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
			break;
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac3TxedCount++;	
		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                     
			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
			       if (pDevExt->SetTXWI_NSEQ == 0)
                    	       {
                                   if(pDevExt->FrameType == 21)
                                   {
                                        int j=pDevExt->NextAc3TxDoneIndex;
                                        IncSequenceNumber((UCHAR*)pDevExt->Ac3RingMain[j].DmaBuf.AllocVa, 3);
                                   }
		                     else
		                     {
		                          int j=pDevExt->NextAc3TxDoneIndex;
                                        IncSequenceNumber((UCHAR*)pDevExt->Ac3RingMain[j].DmaBuf.AllocVa, 10);
		                      }
                            }
			}


			RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX3,&TX_CTX_IDX3);
			RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX3,&TX_DTX_IDX3);
			if (TX_DTX_IDX3 > pDevExt->CurAc3TxIndex) 
				FREE = TX_DTX_IDX3 -pDevExt->CurAc3TxIndex -1;
			else
				FREE = TX_DTX_IDX3+ AC3_RING_SIZE -pDevExt->CurAc3TxIndex -1;
			
			if (  FREE >=1)
			{

				pTxD = (PTXD_STRUC)	pDevExt->Ac3RingMain[pDevExt->CurAc3TxIndex].AllocVa;

				if((pDevExt->FrameType == 17) || (pDevExt->RingType&0x01000000))		// Random length
				{
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc3RingTxWI = (PTX_WI_STRUC) pDevExt->Ac3RingMain[pDevExt->CurAc3TxIndex].DmaBuf.AllocVa;
					//pAc3RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc3RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc3RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;					

					pTxD->SDL0 = pDevExt->CurLength;				
					
					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}
					
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc3RingTxWI = (PTX_WI_STRUC) pDevExt->Ac3RingMain[pDevExt->CurAc3TxIndex].DmaBuf.AllocVa;
					//pAc3RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc3RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc3RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;

				pDevExt->CurAc3TxIndex++;
				if(pDevExt->CurAc3TxIndex >= AC3_RING_SIZE)				
					pDevExt->CurAc3TxIndex = 0;
				
			}
			
			RTMP_Real_IO_WRITE32(pDevExt,RA_TX_CTX_IDX3,	pDevExt->CurAc3TxIndex  );
			
		}

		pDevExt->NextAc3TxDoneIndex++;
		if (pDevExt->NextAc3TxDoneIndex >= AC3_RING_SIZE)
			pDevExt->NextAc3TxDoneIndex = 0;

	}while(++Count < pDevExt->Ac3_MAX_TX_PROCESS);
}

VOID
RTMPHandleAc4TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc4RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX4, TX_DTX_IDX4, FREE, Tmp_Length;
	PULONG				pulTemp;

	do
	{  	       	      
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc4TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc4TxIndex= %d, NextAc4TxDoneIndex= %d\n", pDevExt->CurAc4TxIndex,pDevExt->NextAc4TxDoneIndex));	
		pTxD = (PTXD_STRUC)	pDevExt->Ac4RingMain[pDevExt->NextAc4TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
			break;
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pDevExt->OtherCounters.TransmittedCount = %d\n", pDevExt->OtherCounters.TransmittedCount));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pDevExt->OtherCounters.TxRepeatCount = %d\n", pDevExt->OtherCounters.TxRepeatCount));
		
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac4TxedCount++;
		RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX4,&TX_CTX_IDX4);
		RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX4,&TX_DTX_IDX4);

		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                                         
			if (TX_DTX_IDX4 > pDevExt->CurAc4TxIndex) 
				FREE = TX_DTX_IDX4 -pDevExt->CurAc4TxIndex -1;
			else
				FREE = TX_DTX_IDX4 + AC4_RING_SIZE -pDevExt->CurAc4TxIndex -1;

			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
				if (pDevExt->SetTXWI_NSEQ == 0)
				{
					if(pDevExt->FrameType == 21)
					{
						int j=pDevExt->NextAc4TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac4RingMain[j].DmaBuf.AllocVa, 4);
					}
					else
					{
						int j=pDevExt->NextAc4TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac4RingMain[j].DmaBuf.AllocVa, 10);
					}
				}
			}

			SEQdbg ("FREE = %d\n", FREE);
			if ( FREE >= 1 )
			{
				pTxD = (PTXD_STRUC)	pDevExt->Ac4RingMain[pDevExt->CurAc4TxIndex].AllocVa;

				if (pDevExt->RingType&0x01000000) // QoS Random length
				{
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE - 2;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0));
					Testdbg("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0);
					pAc4RingTxWI = (PTX_WI_STRUC) pDevExt->Ac4RingMain[pDevExt->CurAc4TxIndex].DmaBuf.AllocVa;
					//pAc4RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc4RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc4RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				if(pDevExt->FrameType == 17)		// Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc4RingTxWI = (PTX_WI_STRUC) pDevExt->Ac4RingMain[pDevExt->CurAc4TxIndex].DmaBuf.AllocVa;
					//pAc4RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc4RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc4RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;	

					pTxD->SDL0 = pDevExt->CurLength;	
					
					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}
					
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc4RingTxWI = (PTX_WI_STRUC) pDevExt->Ac4RingMain[pDevExt->CurAc4TxIndex].DmaBuf.AllocVa;
					//pAc4RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc4RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc4RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}

				pTxD->DDONE = 0;

				pDevExt->CurAc4TxIndex++;
				if(pDevExt->CurAc4TxIndex >= AC4_RING_SIZE)
					pDevExt->CurAc4TxIndex = 0;
			}

			RTMP_Real_IO_WRITE32(pDevExt,RA_TX_CTX_IDX4,	pDevExt->CurAc4TxIndex);
		}

		pDevExt->NextAc4TxDoneIndex++;
		if (pDevExt->NextAc4TxDoneIndex >= AC4_RING_SIZE)
			pDevExt->NextAc4TxDoneIndex = 0;

	}while(++Count < pDevExt->Ac4_MAX_TX_PROCESS);
}


VOID
RTMPHandleAc5TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc5RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX5, TX_DTX_IDX5, FREE, Tmp_Length;
	PULONG				pulTemp;

	do
	{  	       	      
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc5TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc5TxIndex= %d, NextAc5TxDoneIndex= %d\n", pDevExt->CurAc5TxIndex,pDevExt->NextAc5TxDoneIndex));	
		pTxD = (PTXD_STRUC)	pDevExt->Ac5RingMain[pDevExt->NextAc5TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
			break;
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac5TxedCount++;
		RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX5,&TX_CTX_IDX5);
		RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX5,&TX_DTX_IDX5);

		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                                         
			if (TX_DTX_IDX5 > pDevExt->CurAc5TxIndex) 
				FREE = TX_DTX_IDX5 -pDevExt->CurAc5TxIndex -1;
			else
				FREE = TX_DTX_IDX5 + AC5_RING_SIZE -pDevExt->CurAc5TxIndex -1;

			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
				if (pDevExt->SetTXWI_NSEQ == 0)
				{
					if(pDevExt->FrameType == 21)
					{
						int j=pDevExt->NextAc5TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac5RingMain[j].DmaBuf.AllocVa, 5);
					}
					else
					{
						int j=pDevExt->NextAc5TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac5RingMain[j].DmaBuf.AllocVa, 10);
					}
				}
			}

			SEQdbg ("FREE = %d\n", FREE);
			if ( FREE >= 1 )
			{
				pTxD = (PTXD_STRUC)	pDevExt->Ac5RingMain[pDevExt->CurAc5TxIndex].AllocVa;

				if (pDevExt->RingType&0x01000000) // QoS Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE - 2;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0));
					Testdbg("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0);
					pAc5RingTxWI = (PTX_WI_STRUC) pDevExt->Ac5RingMain[pDevExt->CurAc5TxIndex].DmaBuf.AllocVa;
					//pAc5RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc5RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc5RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				if(pDevExt->FrameType == 17)		// Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc5RingTxWI = (PTX_WI_STRUC) pDevExt->Ac5RingMain[pDevExt->CurAc5TxIndex].DmaBuf.AllocVa;
					//pAc5RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc5RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc5RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;	

					pTxD->SDL0 = pDevExt->CurLength;	
					
					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}
					
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc5RingTxWI = (PTX_WI_STRUC) pDevExt->Ac5RingMain[pDevExt->CurAc5TxIndex].DmaBuf.AllocVa;
					//pAc5RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc5RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc5RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;

				pDevExt->CurAc5TxIndex++;
				if(pDevExt->CurAc5TxIndex >= AC5_RING_SIZE)				
					pDevExt->CurAc5TxIndex = 0;
			}
			
			RTMP_Real_IO_WRITE32(pDevExt, RA_TX_CTX_IDX5,	pDevExt->CurAc5TxIndex  );			
			
		}

		pDevExt->NextAc5TxDoneIndex++;
		if (pDevExt->NextAc5TxDoneIndex >= AC5_RING_SIZE)
			pDevExt->NextAc5TxDoneIndex = 0;

	}while(++Count < pDevExt->Ac5_MAX_TX_PROCESS);
}

VOID
RTMPHandleAc6TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc6RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX6, TX_DTX_IDX6, FREE, Tmp_Length;
	PULONG				pulTemp;

	do
	{  	       	      
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc6TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc6TxIndex= %d, NextAc6TxDoneIndex= %d\n", pDevExt->CurAc6TxIndex,pDevExt->NextAc6TxDoneIndex));	
		pTxD = (PTXD_STRUC)	pDevExt->Ac6RingMain[pDevExt->NextAc6TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
			break;
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac6TxedCount++;
		RTMP_IO_READ32(pDevExt, RA_TX_CTX_IDX6,&TX_CTX_IDX6);
		RTMP_IO_READ32(pDevExt, RA_TX_DTX_IDX6,&TX_DTX_IDX6);

		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                                         
			if (TX_DTX_IDX6 > pDevExt->CurAc6TxIndex) 
				FREE = TX_DTX_IDX6 -pDevExt->CurAc6TxIndex -1;
			else
				FREE = TX_DTX_IDX6 + AC6_RING_SIZE -pDevExt->CurAc6TxIndex -1;

			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
				if (pDevExt->SetTXWI_NSEQ == 0)
				{
					if(pDevExt->FrameType == 21)
					{
						int j=pDevExt->NextAc6TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac6RingMain[j].DmaBuf.AllocVa, 6);
					}
					else
					{
						int j=pDevExt->NextAc6TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac6RingMain[j].DmaBuf.AllocVa, 10);
					}
				}
			}

			SEQdbg ("FREE = %d\n", FREE);
			if ( FREE >= 1 )
			{
				pTxD = (PTXD_STRUC)	pDevExt->Ac6RingMain[pDevExt->CurAc6TxIndex].AllocVa;

				if (pDevExt->RingType&0x01000000) // QoS Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE - 2;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0));
					Testdbg("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0);
					pAc6RingTxWI = (PTX_WI_STRUC) pDevExt->Ac6RingMain[pDevExt->CurAc6TxIndex].DmaBuf.AllocVa;
					//pAc6RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc6RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc6RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				if(pDevExt->FrameType == 17)		// Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc6RingTxWI = (PTX_WI_STRUC) pDevExt->Ac6RingMain[pDevExt->CurAc6TxIndex].DmaBuf.AllocVa;
					//pAc6RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc6RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc6RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;	

					pTxD->SDL0 = pDevExt->CurLength;	
					
					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}
					
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc6RingTxWI = (PTX_WI_STRUC) pDevExt->Ac6RingMain[pDevExt->CurAc6TxIndex].DmaBuf.AllocVa;
					//pAc6RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc6RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc6RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;

				pDevExt->CurAc6TxIndex++;
				if(pDevExt->CurAc6TxIndex >= AC6_RING_SIZE)				
					pDevExt->CurAc6TxIndex = 0;
			}
			
			RTMP_Real_IO_WRITE32(pDevExt,RA_TX_CTX_IDX6,	pDevExt->CurAc6TxIndex  );			
			
		}

		pDevExt->NextAc6TxDoneIndex++;
		if (pDevExt->NextAc6TxDoneIndex >= AC6_RING_SIZE)
			pDevExt->NextAc6TxDoneIndex = 0;

	}while(++Count < pDevExt->Ac6_MAX_TX_PROCESS);
}

VOID
RTMPHandleAc7TxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pAc7RingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				TX_CTX_IDX7, TX_DTX_IDX7, FREE, Tmp_Length;
	PULONG				pulTemp;

	do
	{  	       	      
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleAc7TxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurAc7TxIndex= %d, NextAc7TxDoneIndex= %d\n", pDevExt->CurAc7TxIndex,pDevExt->NextAc7TxDoneIndex));	
		pTxD = (PTXD_STRUC)	pDevExt->Ac7RingMain[pDevExt->NextAc7TxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
			break;
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.Ac7TxedCount++;
		RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX7,&TX_CTX_IDX7);
		RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX7,&TX_DTX_IDX7);

		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                                         
			if (TX_DTX_IDX7 > pDevExt->CurAc7TxIndex) 
				FREE = TX_DTX_IDX7 -pDevExt->CurAc7TxIndex -1;
			else
				FREE = TX_DTX_IDX7 + AC7_RING_SIZE -pDevExt->CurAc7TxIndex -1;

			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
				if (pDevExt->SetTXWI_NSEQ == 0)
				{
					if(pDevExt->FrameType == 21)
					{
						int j=pDevExt->NextAc7TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac7RingMain[j].DmaBuf.AllocVa, 7);
					}
					else
					{
						int j=pDevExt->NextAc7TxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->Ac7RingMain[j].DmaBuf.AllocVa, 10);
					}
				}
			}

			SEQdbg ("FREE = %d\n", FREE);
			if ( FREE >= 1 )
			{
				pTxD = (PTXD_STRUC)	pDevExt->Ac7RingMain[pDevExt->CurAc7TxIndex].AllocVa;

				if (pDevExt->RingType&0x01000000) // QoS Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE - 2;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0));
					Testdbg("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0);
					pAc7RingTxWI = (PTX_WI_STRUC) pDevExt->Ac7RingMain[pDevExt->CurAc7TxIndex].DmaBuf.AllocVa;
					//pAc7RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc7RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc7RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				if(pDevExt->FrameType == 17)		// Random length
				{
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc7RingTxWI = (PTX_WI_STRUC) pDevExt->Ac7RingMain[pDevExt->CurAc7TxIndex].DmaBuf.AllocVa;
					//pAc7RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc7RingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc7RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;	

					pTxD->SDL0 = pDevExt->CurLength;	
					
					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}
					
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pAc7RingTxWI = (PTX_WI_STRUC) pDevExt->Ac7RingMain[pDevExt->CurAc7TxIndex].DmaBuf.AllocVa;
					//pAc7RingTxWI->MPDU_SIZE = Tmp_Length;
					pAc7RingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pAc7RingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;

				pDevExt->CurAc7TxIndex++;
				if(pDevExt->CurAc7TxIndex >= AC7_RING_SIZE)				
					pDevExt->CurAc7TxIndex = 0;
			}
			
			RTMP_Real_IO_WRITE32(pDevExt, RA_TX_CTX_IDX7,	pDevExt->CurAc7TxIndex  );			
			
		}

		pDevExt->NextAc7TxDoneIndex++;
		if (pDevExt->NextAc7TxDoneIndex >= AC7_RING_SIZE)
			pDevExt->NextAc7TxDoneIndex = 0;

	}while(++Count < pDevExt->Ac7_MAX_TX_PROCESS);
}

VOID
RTMPHandleMgmtTxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pMgmtRingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				MGN_CTX_IDX, MGN_DTX_IDX, FREE, Tmp_Length;
	PULONG				pulTemp;

	do
	{  	       	      
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleMgmtTxRingTxDoneInterrupt()\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurMgmtTxIndex= %d, NextMgmtTxDoneIndex= %d\n", pDevExt->CurMgmtTxIndex, pDevExt->NextMgmtTxDoneIndex));	

		pTxD = (PTXD_STRUC)	pDevExt->MgmtRingMain[pDevExt->NextMgmtTxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
		{
			break;
		}
		
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		Testdbg("TXT:tx descriptor DDONE ok\n");
//		pDevExt->OtherCounters.TransmittedCount++;
		
//		pDevExt->OtherCounters.MgmtTxedCount++;

		RTMP_IO_READ32(pDevExt, RA_TX_CTX_IDX8,&MGN_CTX_IDX);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  RA_TX_CTX_IDX8 = %d\n", MGN_CTX_IDX));
		RTMP_IO_READ32(pDevExt, RA_TX_DTX_IDX8,&MGN_DTX_IDX);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  RA_TX_DTX_IDX8 = %d\n", MGN_DTX_IDX));
		
		//if(pDevExt->OtherCounters.TxRepeatCount > 0)
		if(pDevExt->FirmwarePktCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
			Testdbg("TXT: transmit next packet\n");
  			//if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)
			//	pDevExt->OtherCounters.TxRepeatCount--;
			if(pDevExt->FirmwarePktCount != 0xFFFFFFFF)
				pDevExt->FirmwarePktCount--;
  
			if (MGN_DTX_IDX > pDevExt->CurMgmtTxIndex) 
				FREE = MGN_DTX_IDX -pDevExt->CurMgmtTxIndex -1;
			else
				FREE = MGN_DTX_IDX + MGMT_RING_SIZE -pDevExt->CurMgmtTxIndex -1;

			//if (pDevExt->OtherCounters.TxRepeatCount > 0)
			if(pDevExt->FirmwarePktCount > 0)
			{			      
				if (pDevExt->SetTXWI_NSEQ == 0)
				{
					if(pDevExt->FrameType == 21)
					{
						int j=pDevExt->NextMgmtTxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->MgmtRingMain[j].DmaBuf.AllocVa, 8);
					}
					else
					{
						int j=pDevExt->NextMgmtTxDoneIndex;
						IncSequenceNumber((UCHAR*)pDevExt->MgmtRingMain[j].DmaBuf.AllocVa, 10);
					}
				}
			}
			
			SEQdbg ("FREE = %d\n", FREE);
			if ( FREE >= 1 )
			{
				pTxD = (PTXD_STRUC) pDevExt->MgmtRingMain[pDevExt->CurMgmtTxIndex].AllocVa;

				if (pDevExt->RingType&0x01000000) // QoS Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE - 2;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0));
					Testdbg("pTxD->SDL0->SDL0 : %d===========\n", pTxD->SDL0);
					pMgmtRingTxWI = (PTX_WI_STRUC) pDevExt->MgmtRingMain[pDevExt->CurMgmtTxIndex].DmaBuf.AllocVa;
					//pMgmtRingTxWI->MPDU_SIZE = Tmp_Length;
					pMgmtRingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pMgmtRingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}

				if(pDevExt->FrameType == 17)		// Random length
				{				       
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pMgmtRingTxWI = (PTX_WI_STRUC) pDevExt->MgmtRingMain[pDevExt->CurMgmtTxIndex].DmaBuf.AllocVa;
					//pMgmtRingTxWI->MPDU_SIZE = Tmp_Length;
					pMgmtRingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pMgmtRingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)	
						pDevExt->CurLength = pDevExt->MinLength;	
					
					pTxD->SDL0 = pDevExt->CurLength;	

					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}

					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pMgmtRingTxWI = (PTX_WI_STRUC) pDevExt->MgmtRingMain[pDevExt->CurMgmtTxIndex].DmaBuf.AllocVa;
					//pMgmtRingTxWI->MPDU_SIZE = Tmp_Length;
					pMgmtRingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					Testdbg("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0);
					Testdbg("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length);
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pMgmtRingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;
                            
				pDevExt->CurMgmtTxIndex++;
				if(pDevExt->CurMgmtTxIndex >= MGMT_RING_SIZE)				
					pDevExt->CurMgmtTxIndex = 0;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CurMgmtTxIndex++ = %d\n", pDevExt->CurMgmtTxIndex));
				Testdbg ("CurMgmtTxIndex++ = %d\n", pDevExt->CurMgmtTxIndex);

			}
			
			RTMP_Real_IO_WRITE32(pDevExt, RA_TX_CTX_IDX8,	 pDevExt->CurMgmtTxIndex  );			
			
		}

		pDevExt->NextMgmtTxDoneIndex++;
		if (pDevExt->NextMgmtTxDoneIndex >= MGMT_RING_SIZE)
			pDevExt->NextMgmtTxDoneIndex = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("NextMgmtTxDoneIndex++ = %d\n", pDevExt->NextMgmtTxDoneIndex));
		Testdbg ("NextMgmtTxDoneIndex++ = %d\n", pDevExt->NextMgmtTxDoneIndex);

	}while(++Count < pDevExt->Mgmt_MAX_TX_PROCESS);
}

VOID
RTMPHandleHccaTxRingTxDoneInterrupt(
	IN PVOID pContext )
{
	PTXD_STRUC			pTxD;
	PTX_WI_STRUC		pHccaRingTxWI;
	UCHAR				Count = 0;
	RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pContext;
	UCHAR				i;
	ULONG				HCCA_CTX_IDX, HCCA_DTX_IDX, FREE, Tmp_Length;
	PULONG				pulTemp;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: RTMPHandleHccaTxRingTxDoneInterrupt()\n"));
	do
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("PciDumpr:  CurTxIndex= %d, NextHccaTxDoneIndex= %d\n", pDevExt->CurHccaTxIndex,pDevExt->NextHccaTxDoneIndex));	
		pTxD = (PTXD_STRUC)	pDevExt->HccaRingMain[pDevExt->NextHccaTxDoneIndex].AllocVa;
		if ((pTxD->DDONE != DESC_DMA_DONE))
			break;
		pTxD->DDONE = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT:tx descriptor DDONE ok\n"));
		pDevExt->OtherCounters.TransmittedCount++;
		
		pDevExt->OtherCounters.HccaTxedCount++;	
		if(pDevExt->OtherCounters.TxRepeatCount > 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXT: transmit next packet\n"));
  			if(pDevExt->OtherCounters.TxRepeatCount != 0xFFFFFFFF)				
				pDevExt->OtherCounters.TxRepeatCount--;
                     
			if (pDevExt->OtherCounters.TxRepeatCount > 0)
			{			      
			       if (pDevExt->SetTXWI_NSEQ == 0)
                    	       {
                                   if(pDevExt->FrameType == 21)
                                   {
                                        int j=pDevExt->NextHccaTxDoneIndex;
                                        IncSequenceNumber((UCHAR*)pDevExt->HccaRingMain[j].DmaBuf.AllocVa, 9);
                                   }
		                     else
		                     {
		                          int j=pDevExt->NextHccaTxDoneIndex;
                                        IncSequenceNumber((UCHAR*)pDevExt->HccaRingMain[j].DmaBuf.AllocVa, 10);
		                      }
                            }
			}


			RTMP_IO_READ32(pDevExt,RA_TX_CTX_IDX9,&HCCA_CTX_IDX);
			RTMP_IO_READ32(pDevExt,RA_TX_DTX_IDX9,&HCCA_DTX_IDX);
			if (HCCA_DTX_IDX> pDevExt->CurHccaTxIndex) 
				FREE = HCCA_DTX_IDX -pDevExt->CurHccaTxIndex -1;
			else
				FREE = HCCA_DTX_IDX+ HCCA_RING_SIZE -pDevExt->CurHccaTxIndex -1;
			
			if (  FREE >=1)
			{

				pTxD = (PTXD_STRUC)	pDevExt->HccaRingMain[pDevExt->CurHccaTxIndex].AllocVa;
				
				if((pDevExt->FrameType == 17) || (pDevExt->RingType&0x01000000))		// Random length
				{
					pTxD->SDL0 = (((RandomByte(pDevExt) << 8) + RandomByte(pDevExt)) % 1500);
					if(pTxD->SDL0< 50)	
					{
						pTxD->SDL0 += 50;
					}
					pDevExt->CurLength = pTxD->SDL0;
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pHccaRingTxWI = (PTX_WI_STRUC) pDevExt->HccaRingMain[pDevExt->CurHccaTxIndex].DmaBuf.AllocVa;
					//pHccaRingTxWI->MPDU_SIZE = Tmp_Length;
					pHccaRingTxWI->TXByteCount = Tmp_Length;
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pHccaRingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				else if(pDevExt->FrameType == 18)		// Inc length
				{
					pDevExt->CurLength++;
					if(pDevExt->CurLength > pDevExt->MaxLength)
						pDevExt->CurLength = pDevExt->MinLength;					
					pTxD->SDL0 = pDevExt->CurLength;		
					
					if(pTxD->SDL0 < 44)
					{
						pTxD->SDL0 = 44;
					}
					
					Tmp_Length =  pTxD->SDL0 - TXWI_SIZE;
					pHccaRingTxWI = (PTX_WI_STRUC) pDevExt->HccaRingMain[pDevExt->CurHccaTxIndex].DmaBuf.AllocVa;
					//pHccaRingTxWI->MPDU_SIZE = Tmp_Length;
					pHccaRingTxWI->TXByteCount = Tmp_Length;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len: SDL0 = %x\n",pTxD->SDL0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Isr-->Inc_len:WI MSDU = %x\n",Tmp_Length));
					if ( pDevExt->SetTcpIpPayload )
					{
						pulTemp = (PULONG)pHccaRingTxWI;
						pulTemp += (TXWI_SIZE + 8);
						*pulTemp = (*pulTemp & 0xFFFF0000);
						*pulTemp = *pulTemp | (( Tmp_Length & 0x0000FFFF ) - 32); // Total length in IP header is mpdu_total_length - wifi_header_length(24 bytes) - llc_length(8bytes).
					}
				}
				
				pTxD->DDONE = 0;

				pDevExt->CurHccaTxIndex++;
				if(pDevExt->CurHccaTxIndex >= HCCA_RING_SIZE)				
					pDevExt->CurHccaTxIndex = 0;
				
			}
			
			RTMP_Real_IO_WRITE32(pDevExt,RA_TX_CTX_IDX9,	pDevExt->CurHccaTxIndex  );
			
		}

		pDevExt->NextHccaTxDoneIndex++;
		if (pDevExt->NextHccaTxDoneIndex >= HCCA_RING_SIZE)
			pDevExt->NextHccaTxDoneIndex = 0;

	}while(++Count < pDevExt->Hcca_MAX_TX_PROCESS);
}


VOID LfsrInit(
    IN RTMP_ADAPTER *pAd, 
    IN ULONG Seed) 
{
	//RTMP_ADAPTER *pDevExt = (RTMP_ADAPTER *)pDevObj->DeviceExtension;

    if (Seed == 0) 
        pAd->ShiftReg = 1;
    else 
        pAd->ShiftReg = Seed;
}
#endif /* RTMP_PCI_SUPPORT */


UCHAR RandomByte(RTMP_ADAPTER *pDevExt) 
{
    ULONG i;
    UCHAR R, Result;

    R = 0;

    for (i = 0; i < 8; i++) 
    {
        if (pDevExt->ShiftReg & 0x00000001) 
        {
            pDevExt->ShiftReg = ((pDevExt->ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
            Result = 1;
        } 
        else 
        {
            pDevExt->ShiftReg = pDevExt->ShiftReg >> 1;
            Result = 0;
        }
        R = (R << 1) | Result;
    }

    return R;
}


USHORT RandomShort(RTMP_ADAPTER *pDevExt) 
{
    ULONG i;
    USHORT R, Result;

    R = 0;

    for (i = 0; i < 16; i++) 
    {
        if (pDevExt->ShiftReg & 0x00000001) 
        {
            pDevExt->ShiftReg = ((pDevExt->ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
            Result = 1;
        } 
        else 
        {
            pDevExt->ShiftReg = pDevExt->ShiftReg >> 1;
            Result = 0;
        }
        R = (R << 1) | Result;
    }

    return R;
}


#ifdef RTMP_PCI_SUPPORT

//
// This routine will read a long (32 bits) from configuration registers.
//
// IRQL = PASSIVE_LEVEL
ULONG	RTMPReadCBConfig(
	IN	ULONG	Bus,
	IN	ULONG	Slot,
	IN	ULONG	Func,
	IN	ULONG	Offset)
{
	ULONG	Value;
	ULONG	ConfigAddr;

	ConfigAddr = (Bus << 16) | (Slot << 11) | (Func << 8) | Offset | 0x80000000L;
	WRITE_PORT_ULONG((PULONG)PCI_CFG_ADDR_PORT, ConfigAddr);
	KeStallExecutionProcessor(10);
	Value = READ_PORT_ULONG((PULONG)PCI_CFG_DATA_PORT);

	return(Value);
}

//
// This routine will write a long (32 bits) from configuration registers.
//
// IRQL = PASSIVE_LEVEL
VOID	RTMPWriteCBConfig(
	IN	ULONG	Bus,
	IN	ULONG	Slot,
	IN	ULONG	Func,
	IN	ULONG	Offset,
	IN	ULONG	Value)
{
	ULONG	ConfigAddr;

	ConfigAddr = (Bus << 16) | (Slot << 11) | (Func << 8) | Offset | 0x80000000L;
	
	WRITE_PORT_ULONG((PULONG)PCI_CFG_ADDR_PORT, (ULONG)ConfigAddr);
	KeStallExecutionProcessor(10);
	WRITE_PORT_ULONG((PULONG)PCI_CFG_DATA_PORT, Value);
}
#endif /* RTMP_PCI_SUPPORT */

/*
	========================================================================
	
	Routine Description:
		Be called in HandleDeviceStart to init some settings

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS		Succeed
		NDIS_STATUS_FAILURE			Failed

	IRQL = PASSIVE_LEVEL
		
	========================================================================
*/

//USB
NTSTATUS NICInit(
	IN RTMP_ADAPTER *pAdapter)
{
	NTSTATUS 				ntStatus = STATUS_SUCCESS;
	NTSTATUS 				CRStatus = STATUS_SUCCESS;
	U3DMA_WLCFG         		U3DMAWLCFG;
	ULONG BulkInNum = 0;

	//To get Chip ID
	CRStatus = RTMP_IO_READ32(pAdapter, MCU_CFG_HW_CODE, &pAdapter->ChipID);
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s Read Mac Register MCU_CFG_HW_CODE failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s ChipID = 0x%x\n", __FUNCTION__, pAdapter->ChipID));
	}
	//deviceExtension->MACVersion = 0x76360000;

	if(IS_MT7603(pAdapter))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("####################################  MT7603  ############################################\n"));
	}
	else if(IS_MT76x6(pAdapter))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("####################################  MT7636  ############################################\n"));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s Not support Nic MacValue = 0x%x\n", __FUNCTION__, pAdapter->ChipID));
	}
	do
	{
		UCHAR BulkInNum = 0;
		ntStatus = UsbBulkMemoryAlloc(pAdapter);
		if(ntStatus == STATUS_INSUFFICIENT_RESOURCES)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===================UsbBulkMemoryAlloc   STATUS_INSUFFICIENT_RESOURCES===================\n"));
			break;
		}		
		
		//return STATUS_SUCCESS;

	
//	
//		goto BulkUsb_DispatchCreate_Exit;	
	}while(FALSE);
	// Enable UDMA Tx/Rx	
	CRStatus = RTMP_IO_READ32(pAdapter, UDMA_WLCFG_0, &U3DMAWLCFG.word);
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s Read Mac Register UDMA_WLCFG_0 failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
	    
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("1 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));
	U3DMAWLCFG.Default.WL_TX_EN = 1;
	U3DMAWLCFG.Default.WL_RX_EN = 1;
	U3DMAWLCFG.Default.WL_RX_MPSZ_PAD0 = 1;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("2 U3DMAWLCFG = %x\n", U3DMAWLCFG.word));	
	CRStatus = RTMP_IO_WRITE32(pAdapter, UDMA_WLCFG_0, U3DMAWLCFG.word);
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s Write Mac Register UDMA_WLCFG_0 failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
	//RXStartStop(pAdapter, START, BULKIN_PIPENUMBER_0);
	//RXStartStop(pAdapter, START, BULKIN_PIPENUMBER_1);
	#if 0	//load by driver self need to start rx first
		
	for (BulkInNum = 0; BulkInNum<MAX_RX_BULK_PIPE_NUM; BulkInNum++)
	{		
		if(!pAdapter->bRxEnable[BulkInNum])
		{//now rx0 is stop can start.				
			KIRQL Irql;				
			pAdapter->bRxEnable[BulkInNum] = TRUE;

			pAdapter->ContinBulkIn[BulkInNum] = TRUE;				

			KeAcquireSpinLock(&pAdapter->RxSpinLock[BulkInNum], &Irql);
			if(pAdapter->ulBulkInRunning[BulkInNum] > 0)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=================== ulBulkInRunning[BULKIN_PIPENUMBER_0] > 0===================\n"));
				//KIRQL Irql;
				//KeAcquireSpinLock(&dx->RxSpinLock[0], &Irql);
				pAdapter->ulBulkInRunning[BulkInNum] ++;
				KeReleaseSpinLock(&pAdapter->RxSpinLock[BulkInNum], Irql);
			}
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=================== UsbDoBulkInTransfer===================\n"));
			
				pAdapter->ulBulkInRunning[BulkInNum] ++;
				KeReleaseSpinLock(&pAdapter->RxSpinLock[BulkInNum], Irql);

				RtlZeroMemory(&pAdapter->ReceivedCount[BulkInNum], sizeof(LARGE_INTEGER));
				//Kick bulk in
				UsbDoBulkInTransfer(pAdapter,BulkInNum);
			}

		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===================IS   pAdapter->bRxEnable[BULKIN_PIPENUMBER_0]===================\n"));
		}
		
	}
#endif	

#if 0//USB load FW
	//driver download FW
	//only 7636 need to download rom patch
	if(IS_MT76x6(pAdapter))
	{
		ntStatus = NICLoadRomPatch(pAdapter);
		if (ntStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s NICLoadRomPatch failed\n", __FUNCTION__));
			return STATUS_UNSUCCESSFUL;
		}
	}
	ntStatus = NICLoadFirmware(pAdapter);
	if (ntStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s NICLoadFirmware failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
#endif
	return ntStatus;
}
/*
	========================================================================
	
	Routine Description:
		Used in load by driver self not by UI

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS         firmware image load ok
		NDIS_STATUS_FAILURE         image not found

	IRQL = PASSIVE_LEVEL
		
	========================================================================
*/
ULONG GetTXWISize (IN	RTMP_ADAPTER *pAdapter)
{
	ULONG TXWISize = 0;

	TXWISize = sizeof(TX_WI_STRUC);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXWISize = %d.\n", TXWISize));
	return TXWISize;
}
