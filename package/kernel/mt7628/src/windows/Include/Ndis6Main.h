/*
 ***************************************************************************
 * MediaTek Inc.
 * 5F, No.5, Tai-Yuan 1st St., 
 * Chupei City, Hsinchu County 30265, Taiwan, R.O.C.
 *
 * (c) Copyright 2014-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    N6Main.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __NDIS6MAIN_H__
#define __NDIS6MAIN_H__

//
//  Miniport routines in rtmp_main.c
//

// Function Type for SDV 
MINIPORT_INITIALIZE N6Initialize;
MINIPORT_RESTART N6Restart;
MINIPORT_PAUSE N6Pause;
MINIPORT_SHUTDOWN N6ShutdownHandler; 
MINIPORT_DEVICE_PNP_EVENT_NOTIFY N6DevicePnPEvent;
MINIPORT_HALT N6Halt;
MINIPORT_UNLOAD N6Unload;
MINIPORT_OID_REQUEST N6OIDRequest;
MINIPORT_CANCEL_OID_REQUEST N6CancelOIDRequest;
MINIPORT_SET_OPTIONS N6SetOptions;
MINIPORT_SEND_NET_BUFFER_LISTS N6XmitSendNetBufferLists;
MINIPORT_CANCEL_SEND N6CancelSendNetBufferLists;
MINIPORT_RETURN_NET_BUFFER_LISTS N6ReturnNetBufferLists;
MINIPORT_CHECK_FOR_HANG N6CheckForHang;
MINIPORT_RESET N6Reset;
#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
MINIPORT_DIRECT_OID_REQUEST N6DirectOidRequest;
#endif

NDIS_STATUS DriverEntry(
    IN  PDRIVER_OBJECT      pDriverObject,
    IN  PUNICODE_STRING     pRegistryPath
    );

NDIS_STATUS
N6Initialize(
    IN  NDIS_HANDLE                        MiniportAdapterHandle,
    IN  NDIS_HANDLE                        MiniportDriverContext,
    IN  PNDIS_MINIPORT_INIT_PARAMETERS     MiniportInitParameters
    );

NDIS_STATUS
N6Restart(
    IN  NDIS_HANDLE                         MiniportAdapterContext,
    IN  PNDIS_MINIPORT_RESTART_PARAMETERS   MiniportRestartParameters
    );

NDIS_STATUS
N6Pause(
    IN  NDIS_HANDLE                         MiniportAdapterContext,
    IN  PNDIS_MINIPORT_PAUSE_PARAMETERS     MiniportPauseParameters
    );

VOID 
N6ShutdownHandler(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_SHUTDOWN_ACTION    ShutdownAction
    );

VOID 
N6Shutdown(
    IN  PMP_ADAPTER             pAd  
    );

VOID
N6DevicePnPEvent(
    IN NDIS_HANDLE                  MiniportAdapterContext,
    IN PNET_DEVICE_PNP_EVENT        NetDevicePnPEvent
    );

VOID
N6Halt(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HALT_ACTION        HaltAction
    );

VOID
N6Unload(
    IN  PDRIVER_OBJECT  DriverObject
    );

NDIS_STATUS
ProcessNdis6OIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );

NDIS_STATUS
ProcessNdis5OIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );

NDIS_STATUS
ProcessOIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );

NDIS_STATUS
ProcessMTKQAOIDRequest(
    IN  PMP_ADAPTER         pAd,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );    

NDIS_STATUS
N6OIDRequest(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNDIS_OID_REQUEST   NdisRequest
    );

BOOLEAN
N6CompletePendingOID(
    IN  PMP_ADAPTER               pAd,
    IN  PENDING_OID_TYPE          OidType,
    IN  NDIS_STATUS                   ndisStatus
    );    

VOID 
N6CancelOIDRequest(
    IN NDIS_HANDLE hMiniportAdapterContext,
    IN PVOID       RequestId
    );

NDIS_STATUS
N6SetOptions(
    IN NDIS_HANDLE  NdisMiniportDriverHandle,
    IN NDIS_HANDLE  MiniportDriverContext
    );

VOID
N6CancelSendNetBufferLists(
    IN    NDIS_HANDLE              MiniportAdapterContext,
    IN    PVOID                    CancelId
    );

VOID
N6ReturnNetBufferLists(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNET_BUFFER_LIST    NetBufferLists,
    IN  ULONG               ReturnFlags
    );

BOOLEAN
N6CheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
N6Reset(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    OUT PBOOLEAN        AddressingReset
    );

VOID 
N6CancelDirectOidRequest(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   RequestId
    );

NDIS_STATUS
N6DirectOidRequest(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNDIS_OID_REQUEST       NdisRequest
    );

#endif