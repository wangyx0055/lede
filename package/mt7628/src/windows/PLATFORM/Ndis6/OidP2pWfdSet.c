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
    p2pWfd_oids_Set.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER)) 

NDIS_STATUS
N6SetOidDot11WfdDiscoverRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
    
    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;
    
    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_DISCOVER_REQUEST))
    {
        *BytesNeeded = sizeof(DOT11_WFD_DISCOVER_REQUEST);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetDeviceDiscovery(pAd,
                                                     pPort, 
                                                     InformationBuffer,
                                                     InformationBufferLength,
                                                     BytesRead,
                                                     BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSendGoNegotiationResponse(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;
    
    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_SEND_GO_NEGOTIATION_RESPONSE_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_SEND_GO_NEGOTIATION_RESPONSE_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSendGONegoRsp(pAd,
                                                pPort, 
                                                InformationBuffer,
                                                InformationBufferLength,
                                                BytesRead,
                                                BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdDeviceCapability(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_DEVICE_CAPABILITY_CONFIG))
    {
        *BytesNeeded = sizeof(DOT11_WFD_DEVICE_CAPABILITY_CONFIG);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetDeviceCapability(pAd,
                                                      pPort, 
                                                      InformationBuffer,
                                                      InformationBufferLength,
                                                      BytesRead,
                                                      BytesNeeded);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_WFD_DEVICE_CAPABILITY (=%d) \n",pPort->PortCfg.P2pCapability[0]));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSendInvitationResponse(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_SEND_INVITATION_RESPONSE_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_SEND_INVITATION_RESPONSE_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSendInviteRsp(pAd,
                                                pPort, 
                                                InformationBuffer,
                                                InformationBufferLength,
                                                BytesRead,
                                                BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdListenStateDiscoverability(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(ULONG))
    {
        *BytesNeeded = sizeof(ULONG);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetAvailability(pAd,
                                                  pPort, 
                                                  InformationBuffer,
                                                  InformationBufferLength,
                                                  BytesRead,
                                                  BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdGroupOwnerCapability(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_GROUP_OWNER_CAPABILITY_CONFIG))
    {
        *BytesNeeded = sizeof(DOT11_WFD_GROUP_OWNER_CAPABILITY_CONFIG);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetGOCapability(pAd,
                                                  pPort, 
                                                  InformationBuffer,
                                                  InformationBufferLength,
                                                  BytesRead,
                                                  BytesNeeded);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_WFD_GROUP_OWNER_CAPABILITY (=%d)\n", pPort->PortCfg.P2pCapability[1]));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSendProvisionDiscoveryRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_SEND_PROVISION_DISCOVERY_REQUEST_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_SEND_PROVISION_DISCOVERY_REQUEST_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    { 
                ndisStatus = P2pMsSendProvDiscoReq(pAd,
                                                   pPort, 
                                                   InformationBuffer,
                                                   InformationBufferLength,
                                                   BytesRead,
                                                   BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
       
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
     
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSecondaryDeviceTypeList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;
    
    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        ndisStatus = P2pMsSetSecondaryDeviceTypeList(pAd,
                                                     pPort, 
                                                     InformationBuffer,
                                                     InformationBufferLength,
                                                     BytesRead,
                                                     BytesNeeded); 
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdGroupJoinParameters(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_GROUP_JOIN_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_WFD_GROUP_JOIN_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetGroupJoinParameters(pAd,
                                                         pPort,
                                                         InformationBuffer,
                                                         InformationBufferLength,
                                                         BytesRead,
                                                         BytesNeeded);
            } 
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
        
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdDeviceListenChannel(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (InformationBufferLength < sizeof(DOT11_WFD_DEVICE_LISTEN_CHANNEL))
    {
        *BytesNeeded = sizeof(DOT11_WFD_DEVICE_LISTEN_CHANNEL);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
    }
    else
    {
        PDOT11_WFD_DEVICE_LISTEN_CHANNEL pWFDListenChInfo =
            (PDOT11_WFD_DEVICE_LISTEN_CHANNEL) InformationBuffer;
        
        do
        {
            if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                        pWFDListenChInfo->Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_WFD_DEVICE_LISTEN_CHANNEL_REVISION_1,
                        DOT11_SIZEOF_WFD_DEVICE_LISTEN_CHANNEL_REVISION_1))
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            // TODO: [hm] 2011-10-14 set the listen channel
            
            DBGPRINT(RT_DEBUG_TRACE,("%s::OID_DOT11_WFD_DEVICE_LISTEN_CHANNEL(%d)\n", __FUNCTION__));
        } while (FALSE);

        if (NDIS_STATUS_SUCCESS == ndisStatus)
        {
            *BytesRead = sizeof(DOT11_WFD_DEVICE_INFO);
        }
    }
      
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
      
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdAdditionalIe(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer       = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead               = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_ADDITIONAL_IE))
    {
        *BytesNeeded = sizeof(DOT11_WFD_ADDITIONAL_IE);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetAdditionalIe(pAd,
                                                  pPort, 
                                                  InformationBuffer,
                                                  InformationBufferLength,
                                                  BytesRead,
                                                  BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
      
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
      
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdConnectToGroupRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer       = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead               = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;
    
    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        ndisStatus = P2pMsConnectToGroupReq(pAd, pPort);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdGroupStartParameters(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_GROUP_START_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_WFD_GROUP_START_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetGroupStartParameters(pAd,
                                                          pPort,
                                                          InformationBuffer,
                                                          InformationBufferLength,
                                                          BytesRead,
                                                          BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
        
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSendGoNegotiationConfirmation(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_SEND_GO_NEGOTIATION_CONFIRMATION_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_SEND_GO_NEGOTIATION_CONFIRMATION_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSendGONegoConfirm(pAd,
                                                    pPort, 
                                                    InformationBuffer,
                                                    InformationBufferLength,
                                                    BytesRead,
                                                    BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
      
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
      
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSendInvitationRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_SEND_INVITATION_REQUEST_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_SEND_INVITATION_REQUEST_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSendInviteReq(pAd,
                                                pPort, 
                                                InformationBuffer,
                                                InformationBufferLength,
                                                BytesRead,
                                                BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
        
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSendGoNegotiationRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_SEND_GO_NEGOTIATION_REQUEST_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_SEND_GO_NEGOTIATION_REQUEST_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSendGONegoReq(pAd,
                                                pPort, 
                                                InformationBuffer,
                                                InformationBufferLength,
                                                BytesRead,
                                                BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
         
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
   
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdFlushDeviceList(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber              = NdisRequest->PortNumber;
    PMP_PORT         pPort                   = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer       = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead               = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded             = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
    //Remove all entries from P2P group table
        P2pGroupTabInit(pAd); // Ralink P2P
        P2pDiscoTabInit(pAd); // WIN8 P2P
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
          
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
  
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdDeviceInfo(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_DEVICE_INFO))
    {
        *BytesNeeded = sizeof(DOT11_WFD_DEVICE_INFO);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    { 
                ndisStatus = P2pMsSetDeviceInfo(pAd,
                                                pPort, 
                                                InformationBuffer,
                                                InformationBufferLength,
                                                BytesRead,
                                                BytesNeeded);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_WFD_DEVICE_INFO (=%s)\n", pPort->P2PCfg.DeviceName));
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdDesiredGroupId(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_WFD_GROUP_ID))
    {
        *BytesNeeded = sizeof(DOT11_WFD_GROUP_ID);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSetDesiredGroupId(pAd,
                                                    pPort, 
                                                    InformationBuffer,
                                                    InformationBufferLength,
                                                    BytesRead,
                                                    BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdDisconnectFromGroupRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    // EXPAND_FUNC NONE
#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
#ifdef MULTI_CHANNEL_SUPPORT

    pAd->MccCfg.PendedDisconnectOidRequest = NdisRequest;

#endif // MULTI_CHANNEL_SUPPORT            
#endif // COMPILE_WIN8_ABOVE


    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        ndisStatus = P2pMsDisconnectFromGroupReq(pAd, 
                                                 pPort,
                                                 BytesRead,
                                                 BytesNeeded);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT
    
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdStartGoRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {      
#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))              
#ifdef MULTI_CHANNEL_SUPPORT
        if ((PortType == WFD_GO_PORT) && (NdisRequest->DATA.SET_INFORMATION.Oid == OID_DOT11_WFD_START_GO_REQUEST))
        {
            pAd->MccCfg.PendedGoOidRequest = NdisRequest;
            DBGPRINT(RT_DEBUG_TRACE,("%s(%d) - Record GO's OID_DOT11_WFD_START_GO_REQUEST(PortNum=%d)\n", __FUNCTION__, __LINE__, NdisRequest->PortNumber));
        }          
#endif // MULTI_CHANNEL_SUPPORT
#endif // COMPILE_WIN8_ABOVE

        
        // EXPAND_FUNC NONE
#ifdef MULTI_CHANNEL_SUPPORT            
    // For P2pMs GO
    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        // Save port
        pAd->MccCfg.pGoPort = pPort;

        // Set the flag
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_PRE_START);
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Set MULTI_CHANNEL_GO_PRE_START flag\n", __FUNCTION__));

        // Pending this OID
        ndisStatus = NDIS_STATUS_PENDING;
    }
    else
#endif /*MULTI_CHANNEL_SUPPORT*/                
    {
            ndisStatus = P2pMsStartGOReq(pAd, pPort);
    }
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdSendProvisionDiscoveryResponse(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
    if (InformationBufferLength < sizeof(DOT11_SEND_PROVISION_DISCOVERY_RESPONSE_PARAMETERS))
    {
        *BytesNeeded = sizeof(DOT11_SEND_PROVISION_DISCOVERY_RESPONSE_PARAMETERS);
        ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
                break;
    }
    else
    {
                ndisStatus = P2pMsSendProvDiscoRsp(pAd,
                                                   pPort, 
                                                   InformationBuffer,
                                                   InformationBufferLength,
                                                   BytesRead,
                                                   BytesNeeded);
    }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }
            
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11WfdStopDiscovery(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        // EXPAND_FUNC NONE
        ndisStatus = P2pMsStopDeviceDiscovery(pAd,
                                              pPort,
                                              InformationBuffer,
                                              InformationBufferLength,
                                              BytesRead,
                                              BytesNeeded);
    }
    else // EXTAP_PORT
    {
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    } // EXTAP_PORT

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        // important parameter is changed, restart AP
        if (RestartAPIsRequired) 
        {
            if(KeGetCurrentIrql() == DISPATCH_LEVEL)
            {
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESTART_AP_IS_REQUIRED, &PortNumber,sizeof(NDIS_PORT_NUMBER));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters changed, restart SoftAP @ port %d\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                ApRestartAPHandler(pAd, pPort);
            }
        }
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

#endif // COMPILE_WIN8_ABOVE
