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
    AP_oids_Set.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

NDIS_STATUS
N6SetOidDot11IncomingAssociationDecision(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber = NdisRequest->PortNumber;
    PMP_PORT         pPort = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    PDOT11_INCOMING_ASSOC_DECISION  IncomingAssociationDecision;
    PMAC_TABLE_ENTRY    pEntry;
    
    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;
    
    // EXPAND_FUNC N6APDot11_SetIncomingAssociationDecision()
    do
    {
        *BytesNeeded = 0;
        *BytesRead = 0;

        if (InformationBufferLength < sizeof(DOT11_INCOMING_ASSOC_DECISION))
        {
            *BytesNeeded = sizeof(DOT11_INCOMING_ASSOC_DECISION);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (pPort->PortType == EXTAP_PORT)
        {
            VALIDATE_AP_OP_STATE(pPort);
        }

        if ((pPort->PortType == WFD_GO_PORT) && (!MP_VERIFY_STATE(pPort, OP_STATE)))
        {
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            break;
        }   

        IncomingAssociationDecision = (PDOT11_INCOMING_ASSOC_DECISION) InformationBuffer;

        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(
                            IncomingAssociationDecision->Header,
                            NDIS_OBJECT_TYPE_DEFAULT,
                            DOT11_INCOMING_ASSOC_DECISION_REVISION_1,
                            sizeof(DOT11_INCOMING_ASSOC_DECISION)
                            ))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        //
        // Set is allowed only in AP OP state
        //
        pEntry = MacTableLookup(pAd, pPort, IncomingAssociationDecision->PeerMacAddr);
        if (!pEntry)
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        pEntry->AcceptAssoc = IncomingAssociationDecision->bAccept;
        pEntry->Reason = IncomingAssociationDecision->usReasonCode;
        DBGPRINT(RT_DEBUG_TRACE, ("N6APDot11_SetIncomingAssociationDecision :num=%d,bAccept =%x, Reason=%d\n",pEntry->Aid,pEntry->AcceptAssoc, pEntry->Reason));
        *BytesRead = sizeof(DOT11_INCOMING_ASSOC_DECISION);
    } while (FALSE);

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
N6SetOidDot11DisassociatePeerRequest(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER  PortNumber = NdisRequest->PortNumber;   
    PMP_PORT          pPort = pAd->PortList[PortNumber];
    ULONG            PortType                   = pPort->PortType;
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID             InformationBuffer = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG             InformationBufferLength = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG            BytesRead = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG            BytesNeeded = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;
    
    if (PortType == EXTSTA_PORT || PortType >= WFD_DEVICE_PORT)
    {
        do
        {
            // EXPAND_FUNC NONE
            if (InformationBufferLength < sizeof(DOT11_DISASSOCIATE_PEER_REQUEST))
            {
                *BytesNeeded =  sizeof(DOT11_DISASSOCIATE_PEER_REQUEST);
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else if (pPort->PortType == WFD_GO_PORT)
                    {
                //TODO:: temp solution here, need to consider GO/CLI function in the futures
                ndisStatus = ApOidDisassociatePeerRequest(
                                pAd, 
                                PortNumber, 
                                InformationBuffer, 
                                InformationBufferLength, 
                                BytesRead,
                                BytesNeeded);
                    }
        } while (FALSE);
                }
    else // EXTAP_PORT
                {
        do
            {
            // EXPAND_FUNC NONE
            ndisStatus = ApOidDisassociatePeerRequest(pAd, PortNumber, InformationBuffer, InformationBufferLength, BytesRead, BytesNeeded);
        } while (FALSE);
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
N6SetOidDot11EnumPeerInfo(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC BREAK
        } while (FALSE);
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
N6SetOidDot11AdditionalIe(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC ApOidSetAddtionalIE()
        ULONG requiredSize = 0;
        
        do
        {
        *BytesRead = 0;
        *BytesNeeded = 0;

        ndisStatus = ValiateAdditionalIeSize(
                        (PDOT11_ADDITIONAL_IE)InformationBuffer, 
                        InformationBufferLength,
                        &requiredSize
                        );
        
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            if (NDIS_STATUS_INVALID_LENGTH == ndisStatus)
            {
                // set needed size
                *BytesNeeded = requiredSize;
            }
            break;
        }
        ndisStatus = ApSetAdditionalIe(
                    pPort, 
                    (PDOT11_ADDITIONAL_IE)InformationBuffer
                    );

        if (NDIS_STATUS_SUCCESS == ndisStatus)
        {
            *BytesRead = requiredSize;
        }
    } while (FALSE);
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
N6SetOidDot11AssociationParams(
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
        // EXPAND_FUNC N6Dot11_SetAssociationParam()
        PDOT11_ASSOCIATION_PARAMS   dot11AssocParams = NULL;
        PVOID                       tmpBuf = NULL;
    
        do
        {
        *BytesRead = 0;
        *BytesNeeded = 0;

        if (InformationBufferLength < sizeof(DOT11_ASSOCIATION_PARAMS))
        {
            *BytesNeeded = sizeof(DOT11_ASSOCIATION_PARAMS);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        dot11AssocParams = (PDOT11_ASSOCIATION_PARAMS)InformationBuffer;

        if (!MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(dot11AssocParams->Header, 
            NDIS_OBJECT_TYPE_DEFAULT,
            DOT11_ASSOCIATION_PARAMS_REVISION_1,
            sizeof(DOT11_ASSOCIATION_PARAMS)))
        {
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;            
        }
        
        //
        // Verify IE blob length
        //
        *BytesNeeded = dot11AssocParams->uAssocRequestIEsOffset + dot11AssocParams->uAssocRequestIEsLength;
        if (InformationBufferLength < *BytesNeeded )
        {
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }
        if (dot11AssocParams->uAssocRequestIEsLength > 0)
        {
           PlatformAllocateMemory(pAd, 
            &tmpBuf,  
            dot11AssocParams->uAssocRequestIEsLength);

            if (tmpBuf == NULL) 
            {
                *BytesRead = sizeof(DOT11_ASSOCIATION_PARAMS);
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            PlatformMoveMemory(tmpBuf, 
            Add2Ptr(dot11AssocParams, dot11AssocParams->uAssocRequestIEsOffset),
            dot11AssocParams->uAssocRequestIEsLength);
        }

        if (pPort->PortCfg.AdditionalAssocReqIEData)
        {
            PlatformFreeMemory(pPort->PortCfg.AdditionalAssocReqIEData, pPort->PortCfg.AdditionalAssocReqIESize);
        }

        // Save the parameters
        PlatformMoveMemory(pPort->PortCfg.AssocIEBSSID, dot11AssocParams->BSSID, DOT11_ADDRESS_SIZE);
        pPort->PortCfg.AdditionalAssocReqIESize = dot11AssocParams->uAssocRequestIEsLength;
        pPort->PortCfg.AdditionalAssocReqIEData = tmpBuf;

        *BytesRead  = *BytesNeeded;
    }while(FALSE);
    DBGPRINT(RT_DEBUG_TRACE, ("N6Dot11_SetAssociationParam size=%d\n" ,pPort->PortCfg.AdditionalAssocReqIESize));
    DBGPRINT(RT_DEBUG_TRACE, ("N6Dot11_SetAssociationParam BSSID=%x %x %x %x %x %x\n" , 
        pPort->PortCfg.AssocIEBSSID[0], pPort->PortCfg.AssocIEBSSID[1], pPort->PortCfg.AssocIEBSSID[2],
        pPort->PortCfg.AssocIEBSSID[3], pPort->PortCfg.AssocIEBSSID[4], pPort->PortCfg.AssocIEBSSID[5]));
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
N6SetOidDot11StartApRequest(
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

    // EXPAND_FUNC N6APDot11_StartAPRequest()
    do
    {
        *BytesNeeded = 0;
        *BytesRead = 0;
        if(pAd->StaCfg.bRadio  == FALSE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("===> N6APDot11_StartAPRequest:Port(%d) radio off\n",pPort->PortNumber));           
             ndisStatus = NDIS_STATUS_DOT11_POWER_STATE_INVALID;
             break;
        }

        // BlueWire would start AP instaed of GO under the case of disabled virtual wifi-direct port
        // If we want to run Ralink-P2P only, need to ignore this request. (exclusive mode)
        if ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) && (P2P_ON(pPort)) && (pAd->OpMode == OPMODE_STAP2P))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("===> N6APDot11_StartAPRequest: (%d) Not support the coexistence of vwifiAP and p2p, return \n",pPort->PortNumber));                    
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            break;
        }
        
        // Set is allowed only in AP INIT state
        VALIDATE_AP_INIT_STATE(pPort);
        DBGPRINT(RT_DEBUG_TRACE, ("===> N6APDot11_StartAPRequest:POrt num=%d,authmode =%s,wepstatus = %s\n"
            ,pPort->PortNumber,decodeAuthAlgorithm(pPort->PortCfg.AuthMode),decodeCipherAlgorithm(pPort->PortCfg.WepStatus)));

        //
        // Set AP start to starting
        //
        if(KeGetCurrentIrql() == DISPATCH_LEVEL)
        {
            MTEnqueueInternalCmd(pAd, pPort, OID_DOT11_START_AP_REQUEST, &PortNumber, sizeof(NDIS_PORT_NUMBER));
        }
        else
        {
            ApStartApRequestHandler(pAd, pPort);
        }
    } while (FALSE);


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
N6SetOidDot11AvailableFrequencyList(
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
        ndisStatus = NDIS_STATUS_INVALID_OID;
        DBGPRINT(RT_DEBUG_TRACE, ("%s::PortType:%d, Invalid OID=0x%08X\n",__FUNCTION__, PortType, Oid));
        FUNC_LEAVE_STATUS(ndisStatus);
        return ndisStatus;
    }
    else // EXTAP_PORT
    {
        do
        {
            // EXPAND_FUNC BREAK
        } while (FALSE);
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
N6SetOidDot11BeaconPeriod(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];
    NDIS_OID         Oid                        = NdisRequest->DATA.SET_INFORMATION.Oid;
    PVOID            InformationBuffer          = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    ULONG            InformationBufferLength    = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PULONG           BytesRead                  = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesRead;
    PULONG           BytesNeeded                = (PULONG)&NdisRequest->DATA.SET_INFORMATION.BytesNeeded;

    BOOLEAN RestartAPIsRequired = FALSE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    do
    {
        if (pPort == NULL)
        {
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            DBGPRINT(RT_DEBUG_ERROR,("Set::OID_DOT11_BEACON_PERIOD, pPort can't be NULL\n"));
            break;
        }
        
        // EXPAND_FUNC NONE
        if (!MP_VERIFY_STATE(pPort, INIT_STATE))
        {
            ndisStatus = NDIS_STATUS_INVALID_STATE;
            DBGPRINT(RT_DEBUG_ERROR,("Set::OID_DOT11_BEACON_PERIOD failed on Invalid state\n"));
            break;
        }

        if (InformationBufferLength < sizeof(ULONG))
        {
            *BytesNeeded = sizeof(ULONG);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }
        else
        {
            ULONG BeaconPeriod = *(PULONG)InformationBuffer;
            
            if (BeaconPeriod < 1 || BeaconPeriod > 65535)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pPort->CommonCfg.BeaconPeriod = (USHORT) BeaconPeriod;
            *BytesRead = sizeof(ULONG);
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_BEACON_PERIOD (=%d)\n", BeaconPeriod));
        }
    }while(FALSE);       

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
N6SetOidDot11DtimPeriod(
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
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                ULONG DTimPeriod = *(PULONG)InformationBuffer;

                pPort->SoftAP.ApCfg.DtimPeriod = (UCHAR) DTimPeriod;
                DBGPRINT(RT_DEBUG_INFO, ("Set::OID_DOT11_DTIM_PERIOD (=%d)\n", DTimPeriod));
            }
        } while (FALSE);
    }
    else // EXTAP_PORT
    {
        // EXPAND_FUNC BREAK
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
N6SetOidDot11WpsEnabled(
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


    // EXPAND_FUNC ApOidSetWPSEnable()
    do
    {
        *BytesRead = 0;
        *BytesNeeded = 0;
        if( InformationBufferLength < sizeof(BOOLEAN))
        {
            *BytesNeeded = sizeof(BOOLEAN);
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if(pPort->PortType == EXTSTA_PORT)
        {
            ndisStatus = NDIS_STATUS_FAILURE;
            break;
        }
        
        pPort->SoftAP.ApCfg.bWPSEnable = *(BOOLEAN *) InformationBuffer;
        *BytesRead = sizeof(BOOLEAN);

        // Let GO enter the provisioning state when start WPS engine
        if (pPort->PortType == WFD_GO_PORT)
        {   
            if ((pPort->SoftAP.ApCfg.bWPSEnable == TRUE) &&
                ((pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_IDLE) ||
                (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_INVITATION) ||
                (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_FORMATION) ||
                (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_START_PROGRESS)))   
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - change the state from %s to P2pMs_STATE_PROVISIONING\n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));
                pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_PROVISIONING;
                // Start blocking periodic scan timer
                pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
               
            }
            else if (pPort->SoftAP.ApCfg.bWPSEnable == FALSE)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - change the state from %s to P2pMs_STATE_IDLE\n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));
                pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s : state= %s \n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase)));
        }
    }while(FALSE);
    
    DBGPRINT(RT_DEBUG_TRACE, ("set bWPSEnablel = %x\n",pPort->SoftAP.ApCfg.bWPSEnable));


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

