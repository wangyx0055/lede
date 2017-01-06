    
/*
    ========================================================================

    Routine Description:
        Process Report Ethernet Frame to LLC
        
    Arguments:
        pAd     Pointer to our adapter
        pPort   Pointer to our Port
        p80211Header    Pointer to the Packet Header 
        us80211FrameSize Record the Frame Size 
        UserPriority     Record the Frame Priority
        CRCError         Record if the Frame is CRC Error Frame (In monitor mode)
        
    Return Value:
        None
        
    IRQL = DISPATCH_LEVEL
    
    Note:
    
    bSingleIndicate = FALSE --> aggregated indication (beside N6UsbRxStaPacket)
    bSingleIndicate = TRUE  --> single indication
    
    ========================================================================
*/
#include    "N6Indicate.h"


VOID N6PlatformIndicatePktToNdis(
    IN PMP_ADAPTER pAd,
    IN PNET_BUFFER_LIST pNetBufferList,
    IN NDIS_PORT_NUMBER PortNumber,
    IN ULONG NumberOfNetBufferLists)
{
    UCHAR   i = 0;
    do
    {
#if _WIN8_USB_SS_SUPPORTED
        NdisAcquireSpinLock(&pAd->Ss.SsLock);
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
        {
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
            break;
        }
        else
        {
            pAd->PendingNBLIndication++;
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
        }
#endif

        for (i = 0; i < NumberOfNetBufferLists; i++)
        {
            MT_INC_REF(&pAd->pRxCfg->nIndicatedRxPkts);
        }

        DBGPRINT(RT_DEBUG_INFO,("NumberOfNetBufferLists  =%d\n", NumberOfNetBufferLists));
            
        NdisMIndicateReceiveNetBufferLists(pAd->AdapterHandle,
                                        pNetBufferList,
                                        PortNumber,
                                        NumberOfNetBufferLists,
                                        0/*NDIS_RECEIVE_FLAGS_RESOURCES*/);

#if _WIN8_USB_SS_SUPPORTED
        NdisAcquireSpinLock(&pAd->Ss.SsLock);
        pAd->PendingNBLIndication--;
        NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif

    } while (0);

    return;
}


VOID 
N6PlatformIndicateStatusToNdis(
    IN PMP_ADAPTER pAd,
    IN PNDIS_STATUS_INDICATION pStatusIndication
    )
{
    do
    {
#if _WIN8_USB_SS_SUPPORTED
        NdisAcquireSpinLock(&pAd->Ss.SsLock);
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
        {
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
            break;
        }
        else
        {
            pAd->PendingStatusIndication++;
            NdisReleaseSpinLock(&pAd->Ss.SsLock);
        }
#endif

        NdisMIndicateStatusEx(pAd->AdapterHandle, pStatusIndication);

#if _WIN8_USB_SS_SUPPORTED
        NdisAcquireSpinLock(&pAd->Ss.SsLock);
        pAd->PendingStatusIndication--;
        NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif

    } while (0);

}



VOID 
N6PlatformIndicateDot11Status(
    IN  PMP_ADAPTER   pAd,    
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     StatusCode,
    IN  PVOID           RequestID,
    IN  PVOID           pStatusBuffer,
    IN  ULONG           StatusBufferSize)
{
    NDIS_STATUS_INDICATION  statusIndication;
    PDOT11_ASSOCIATION_START_PARAMETERS pAssocStartParameters   = NULL;
    USHORT      saveHeaderSize = 0;
    ULONG       saveBufferSize = 0;

    //
    // We should not indicate any event to upper layer when halt in progress,
    // otherwise OS will crash.
    //
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> fRTMP_ADAPTER_HALT_IN_PROGRESS. RETURN!!!\n", __FUNCTION__));
        return;
    }
    
    // ==> WPS

    // If WPS-related operations have started, the miniport driver shouldn't indicate status to Vista.
    if (((StatusCode == NDIS_STATUS_DOT11_ASSOCIATION_START) || 
         (StatusCode == NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION) || 
         (StatusCode == NDIS_STATUS_DOT11_CONNECTION_START) || 
         (StatusCode == NDIS_STATUS_DOT11_CONNECTION_COMPLETION)) &&
        (pPort->StaCfg.WscControl.WscState >= WSC_STATE_INIT))
    {
        // do nothing.
        DBGPRINT(RT_DEBUG_ERROR, ("%s ==> WPS in operation. RETURN!!!\n", __FUNCTION__));
        return;
    }   

    // <== WPS
    
    PlatformZeroMemory(&statusIndication, sizeof(NDIS_STATUS_INDICATION));

    //
    // Fill in object header
    //
    statusIndication.Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;
    statusIndication.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
    statusIndication.Header.Size = sizeof(NDIS_STATUS_INDICATION);

    // Fill in the port number
    statusIndication.PortNumber = pPort->PortNumber;

    //
    // Fill in the rest of the field
    // 
    statusIndication.StatusCode = StatusCode;
    statusIndication.SourceHandle = pAd->AdapterHandle;
    statusIndication.DestinationHandle = NULL;
    statusIndication.RequestId = RequestID;

    statusIndication.StatusBuffer = pStatusBuffer;
    statusIndication.StatusBufferSize = StatusBufferSize;

    PlatformIndicateStatusToNdis(pAd, &statusIndication);

    DBGPRINT(RT_DEBUG_INFO, ("PlatformIndicateDot11Status - PortNum = %u, RequestID=0x%x, StatusCode=0x%x, StatusBufferSize=%d\n",
                                statusIndication.PortNumber,
                                RequestID,
                                StatusCode,
                                StatusBufferSize));
}


VOID 
N6PlatformIndicateAssociationStart(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT pPort,
    IN  PBSS_ENTRY  pBss)
{
    PDOT11_ASSOCIATION_START_PARAMETERS pAssocStartParameters;
    ULONG                               size        = 0;
    NDIS_STATUS                         retCode     = NDIS_STATUS_RESOURCES;

    size        = sizeof(DOT11_ASSOCIATION_START_PARAMETERS);

    DBGPRINT(RT_DEBUG_TRACE, ("PlatformIndicateAssociationStart:: BSSID[%02x:%02x:%02x:%02x:%02x:%02x] \n", pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5])); 
    DBGPRINT(RT_DEBUG_TRACE, ("Allocating assoc start buffer - %d bytes\n", size));

    if (pAd->MlmeAux.BssType == BSS_INFRA)
    {
        PlatformAllocateMemory(pAd, 
                             &pAssocStartParameters, 
                             size);
        
        if (NULL != pAssocStartParameters)
        {
            //
            // Indicate the association start status indication
            //
            PlatformZeroMemory(pAssocStartParameters, size);
            MP_ASSIGN_NDIS_OBJECT_HEADER(pAssocStartParameters->Header, 
                                         NDIS_OBJECT_TYPE_DEFAULT,
                                         DOT11_ASSOCIATION_START_PARAMETERS_REVISION_1,
                                        (USHORT)size);
            
            //assocStartParameters.uIHVDataOffset = 0;
            //assocStartParameters.uIHVDataSize = 0;

            // We only supporting single SSID
            PlatformMoveMemory(&pAssocStartParameters->SSID, &pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));

            // Copy the MAC address from the AP entry
            COPY_MAC_ADDR(&pAssocStartParameters->MacAddr, /*pBss->Bssid*/ pAd->MlmeAux.Bssid); // do not use pBss->Bssid, because second time Bssid and  pAd->MlmeAux.Bssid are not sync

            // 
            // Append beacon data as IHV data.
            //
            pAssocStartParameters->uIHVDataOffset   = 0;
            pAssocStartParameters->uIHVDataSize     = 0;

            PlatformIndicateDot11Status(pAd,pPort, NDIS_STATUS_DOT11_ASSOCIATION_START, NULL, pAssocStartParameters, size);

            PlatformFreeMemory(pAssocStartParameters, size);

            DBGPRINT(RT_DEBUG_TRACE, ("!! NDIS_STATUS_DOT11_ASSOCIATION_START (size=%d) !!\n", size));
        }
    }
    else
    {
        if (pAd->MlmeAux.bStaCanRoam)
        {
            PlatformIndicateAdhocRoamingStart(pAd,pPort,pAd->MlmeAux.Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            PlatformIndicateRoamingCompletion(pAd,pPort,DOT11_ASSOC_STATUS_SUCCESS);
        }
        else
        {
            PlatformIndicateConnectionStart(pAd, pPort,pAd->MlmeAux.BssType, pAd->MlmeAux.Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            PlatformIndicateConnectionCompletion(pAd, pPort,DOT11_CONNECTION_STATUS_SUCCESS);
            pAd->MlmeAux.bStaCanRoam = TRUE;
            LedCtrlConnectionCompletion(pAd,pPort, TRUE);
        }
    }
}


NDIS_STATUS 
N6PlatformIndicateAssociationCompletion(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  DOT11_ASSOC_STATUS  Status)
{
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
    PUCHAR              pStatusBuffer = NULL;
    ULONG               BufferLength = 0;
    BOOLEAN             bUseBackupBuffer = FALSE;
    PDOT11_ASSOCIATION_COMPLETION_PARAMETERS    pCompletionParameters = NULL;
    PUCHAR              pTempPtr = NULL;
    ULONG               CurrentOffset = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS);

    
    if (Status != DOT11_ASSOC_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s():     Indicate AssociationComplete status (0x%x)!=Success \n",__FUNCTION__, Status));
        // Fail to connection and resume previous beacon if necessary
        AsicResumeBssSync(pAd);

        // Reset P2pMsSatetPhase from PROVISIONING to IDLE if encounter any failure during provision
        if (pPort->P2PCfg.P2pMsSatetPhase > P2pMs_STATE_IDLE)
        {
            pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_IDLE;
        }
    }
    
    BufferLength = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS) + 
                    pAd->StaCfg.BeaconFrameSize + 
                    sizeof(ULONG); // PhyID

    if (pAd->StaCfg.AssocRequestLength)
        BufferLength += pAd->StaCfg.AssocRequestLength - DOT11_MGMT_HEADER_SIZE;

    if (pAd->StaCfg.AssocResponseLength)
        BufferLength += pAd->StaCfg.AssocResponseLength - DOT11_MGMT_HEADER_SIZE;

    PlatformAllocateMemory(pAd,  &pStatusBuffer, BufferLength);
    if (pStatusBuffer == NULL)
    {
        bUseBackupBuffer = TRUE;
        pStatusBuffer = (PUCHAR)&pAd->StaCfg.AssocBuffer;
        BufferLength = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS);
        ndisStatus = NDIS_STATUS_RESOURCES;
    }

    PlatformZeroMemory(pStatusBuffer, BufferLength);

    pCompletionParameters = (PDOT11_ASSOCIATION_COMPLETION_PARAMETERS)pStatusBuffer;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if(NDIS_WIN8_ABOVE(pAd) && MFP_ON(pAd, pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("set completion parameters revision 2\n"));
        MP_ASSIGN_NDIS_OBJECT_HEADER(pCompletionParameters->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_ASSOCIATION_COMPLETION_PARAMETERS_REVISION_2,
                                    sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));
    }
    else
#endif
    {
        MP_ASSIGN_NDIS_OBJECT_HEADER(pCompletionParameters->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_ASSOCIATION_COMPLETION_PARAMETERS_REVISION_1,
                                    sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));
    }
    COPY_MAC_ADDR(&pCompletionParameters->MacAddr, pAd->MlmeAux.Bssid);

    DBGPRINT(RT_DEBUG_TRACE, ("PlatformIndicateAssociationCompletion: MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    pAd->MlmeAux.Bssid[0], pAd->MlmeAux.Bssid[1], pAd->MlmeAux.Bssid[2],
                    pAd->MlmeAux.Bssid[3], pAd->MlmeAux.Bssid[4], pAd->MlmeAux.Bssid[5]));

    pCompletionParameters->uStatus = Status;
    if (Status == DOT11_ASSOC_STATUS_SUCCESS)
    {
        //
        // Fill success state parameters
        // Please note, The following message can't be wrong especially AuthMode & UnicastCipher & MulticastCipher
        // Otherwise WPA/WPA2(PSK) will fail since 802.1x supplicant can't handle.
        //
        pCompletionParameters->AuthAlgo = pPort->PortCfg.AuthMode;
        pCompletionParameters->UnicastCipher = pPort->PortCfg.PairCipher; 
        pCompletionParameters->MulticastCipher = pPort->PortCfg.GroupCipher; 
        pCompletionParameters->bFourAddressSupported = FALSE;       // No support for 4 address detection
        pCompletionParameters->bPortAuthorized = FALSE;
        pCompletionParameters->DSInfo = DOT11_DS_UNKNOWN;
        pCompletionParameters->uEncapTableOffset = 0;
        pCompletionParameters->uEncapTableSize = 0;

        pCompletionParameters->bReAssocReq = FALSE;
        pCompletionParameters->bReAssocResp = FALSE;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        //add for Win8
        if(NDIS_WIN8_ABOVE(pAd) && (pCompletionParameters->Header.Revision == DOT11_ASSOCIATION_COMPLETION_PARAMETERS_REVISION_2))
        {
            if(MFP_ON(pAd, pPort))
            {
                pCompletionParameters->MulticastMgmtCipher = DOT11_CIPHER_ALGO_BIP;
            }
            else
                pCompletionParameters->MulticastMgmtCipher = DOT11_CIPHER_ALGO_NONE;
        }
#endif

        pCompletionParameters->bPortAuthorized = FALSE;
        pCompletionParameters->bReAssocResp = FALSE;
        pCompletionParameters->bReAssocReq = FALSE;
    }

    if (bUseBackupBuffer == FALSE)
    {
        // Please note, we must attached the following message before indicate NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION
        // Otherwise WPA/WPA2(PSK) will fail since 802.1x supplicant can't handle.
    
        //
        // 1.) add PHY ID
        //
        pCompletionParameters->uActivePhyListOffset = CurrentOffset;
        pCompletionParameters->uActivePhyListSize = sizeof(ULONG);
        pTempPtr = pStatusBuffer + CurrentOffset;
        *((ULONG UNALIGNED *)pTempPtr) = pAd->StaCfg.ActivePhyId;

        CurrentOffset += sizeof(ULONG);

        //
        // 2.) Beacon
        //
        pCompletionParameters->uBeaconOffset = CurrentOffset;
        pCompletionParameters->uBeaconSize = pAd->StaCfg.BeaconFrameSize;
        pTempPtr = pStatusBuffer + CurrentOffset;
        PlatformMoveMemory(pTempPtr, pAd->StaCfg.BeaconFrame, pAd->StaCfg.BeaconFrameSize);
        CurrentOffset += pAd->StaCfg.BeaconFrameSize;

        //
        // 3.) Association request
        //
        if (pAd->StaCfg.AssocRequestLength)
        {
            pCompletionParameters->uAssocReqOffset = CurrentOffset;
            pCompletionParameters->uAssocReqSize = pAd->StaCfg.AssocRequestLength - DOT11_MGMT_HEADER_SIZE;

            pTempPtr = pStatusBuffer + CurrentOffset;
            PlatformMoveMemory(pTempPtr, 
                            pAd->StaCfg.AssocRequest + DOT11_MGMT_HEADER_SIZE, 
                            pCompletionParameters->uAssocReqSize);
            CurrentOffset += pCompletionParameters->uAssocReqSize;
        }

        //
        // 4.) Association response
        //
        if (pAd->StaCfg.AssocResponseLength)
        {
            pCompletionParameters->uAssocRespOffset = CurrentOffset;
            pCompletionParameters->uAssocRespSize = pAd->StaCfg.AssocResponseLength - DOT11_MGMT_HEADER_SIZE;

            pTempPtr = pStatusBuffer + CurrentOffset;
            PlatformMoveMemory(pTempPtr, 
                            pAd->StaCfg.AssocResponse + DOT11_MGMT_HEADER_SIZE, 
                            pCompletionParameters->uAssocRespSize);
            CurrentOffset += pCompletionParameters->uAssocRespSize;
        }
    }

    pAd->StaCfg.AssocRequestLength = 0;
    pAd->StaCfg.AssocResponseLength = 0;

    PlatformIndicateDot11Status(pAd,pPort, NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION, NULL, pStatusBuffer, BufferLength);

    DBGPRINT(RT_DEBUG_TRACE, ("!! NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION !!\n"));

    if (bUseBackupBuffer == FALSE) 
    {
        PlatformFreeMemory(pStatusBuffer, BufferLength);
        pStatusBuffer = NULL;
    }   

    return ndisStatus;
}

VOID
N6PlatformIndicateConnectionStart(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN DOT11_BSS_TYPE   BSSType,
    IN PUCHAR           AdhocBSSID,
    IN PUCHAR           AdhocSsid,
    IN UCHAR            AdhocSsidLen)
{
    DOT11_CONNECTION_START_PARAMETERS   connectionStartParameters;

    PlatformZeroMemory(&connectionStartParameters, sizeof(DOT11_CONNECTION_START_PARAMETERS));
    MP_ASSIGN_NDIS_OBJECT_HEADER(connectionStartParameters.Header, 
                                 NDIS_OBJECT_TYPE_DEFAULT,
                                 DOT11_CONNECTION_START_PARAMETERS_REVISION_1,
                                 sizeof(DOT11_CONNECTION_START_PARAMETERS));
    if (BSSType == BSS_ADHOC)
    {
        if (AdhocBSSID)
        {
            COPY_MAC_ADDR(&connectionStartParameters.AdhocBSSID, AdhocBSSID);
        }
        else
        {
            COPY_MAC_ADDR(&connectionStartParameters.AdhocBSSID, pPort->PortCfg.Bssid);
        }

        if (AdhocSsidLen)
        {
            PlatformMoveMemory(&connectionStartParameters.AdhocSSID.ucSSID, AdhocSsid, min(AdhocSsidLen, 32));
            connectionStartParameters.AdhocSSID.uSSIDLength = min(AdhocSsidLen, 32);
        }
    }
    connectionStartParameters.BSSType = BSSType;
    PlatformIndicateDot11Status(pAd,pPort, NDIS_STATUS_DOT11_CONNECTION_START, NULL, &connectionStartParameters, sizeof(DOT11_CONNECTION_START_PARAMETERS));

    // LED indication.
    LedCtrlConnectionStart(pAd);

    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]!! NDIS_STATUS_DOT11_CONNECTION_START !!\n", __FUNCTION__, __LINE__));
}

NDIS_STATUS
N6PlatformIndicateConnectionCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort,
    IN  ULONG CompletionStatus)
{
    DOT11_CONNECTION_COMPLETION_PARAMETERS   connectionCompleteParameters;

    //
    // Indicate the connection complete status indication
    //
    PlatformZeroMemory(&connectionCompleteParameters, sizeof(DOT11_CONNECTION_COMPLETION_PARAMETERS));
    MP_ASSIGN_NDIS_OBJECT_HEADER(connectionCompleteParameters.Header, 
                             NDIS_OBJECT_TYPE_DEFAULT,
                             DOT11_CONNECTION_COMPLETION_PARAMETERS_REVISION_1,
                             sizeof(DOT11_CONNECTION_COMPLETION_PARAMETERS));
    connectionCompleteParameters.uStatus = CompletionStatus;


    PlatformIndicateDot11Status(pAd,pPort, NDIS_STATUS_DOT11_CONNECTION_COMPLETION, NULL, &connectionCompleteParameters, sizeof(DOT11_CONNECTION_COMPLETION_PARAMETERS));


    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]!! NDIS_STATUS_DOT11_CONNECTION_COMPLETION !!\n", __FUNCTION__, __LINE__)); 

    if (CompletionStatus != DOT11_CONNECTION_STATUS_SUCCESS)
    {
        MP_SET_STATE(pPort, INIT_STATE);
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] Failed to connect and reset the state to INIT_STATE !!\n", __FUNCTION__, __LINE__));   
    }
    
    return NDIS_STATUS_SUCCESS;
}

VOID
N6PlatformIndicateAdhocRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR       Bssid,
    IN PUCHAR       Ssid,
    IN UCHAR        SsidLen)
{
    DOT11_ROAMING_START_PARAMETERS   roamingStartParameters;

    //
    // Indicate the connection start status indication
    //
    PlatformZeroMemory(&roamingStartParameters, sizeof(DOT11_ROAMING_START_PARAMETERS));
    MP_ASSIGN_NDIS_OBJECT_HEADER(roamingStartParameters.Header, 
                NDIS_OBJECT_TYPE_DEFAULT,
                DOT11_ROAMING_START_PARAMETERS_REVISION_1,
                sizeof(DOT11_ROAMING_START_PARAMETERS));
    
    COPY_MAC_ADDR(&roamingStartParameters.AdhocBSSID, Bssid);

    if (SsidLen)
    {
        PlatformMoveMemory(&roamingStartParameters.AdhocSSID.ucSSID, Ssid, min(SsidLen, 32));
        roamingStartParameters.AdhocSSID.uSSIDLength = min(SsidLen, 32);
    }
    roamingStartParameters.uRoamingReason = DOT11_ASSOC_STATUS_ROAMING_ADHOC;

    PlatformIndicateDot11Status(pAd,pPort, NDIS_STATUS_DOT11_ROAMING_START, NULL, &roamingStartParameters, sizeof(DOT11_ROAMING_START_PARAMETERS));
    DBGPRINT(RT_DEBUG_TRACE, ("!! ADHOC NDIS_STATUS_DOT11_ROAMING_START !!\n"));    
}

VOID
N6PlatformIndicateRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG RoamingReason)
{
    DOT11_ROAMING_START_PARAMETERS   roamingStartParameters;
    if(pAd->MlmeAux.bNeedIndicateRoamStatus == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!! already indicate NDIS_STATUS_DOT11_ROAMING_START !!\n"));
        return;
    }
    //
    // Indicate the connection start status indication
    //
    PlatformZeroMemory(&roamingStartParameters, sizeof(DOT11_ROAMING_START_PARAMETERS));
    MP_ASSIGN_NDIS_OBJECT_HEADER(roamingStartParameters.Header, 
                                 NDIS_OBJECT_TYPE_DEFAULT,
                                 DOT11_ROAMING_START_PARAMETERS_REVISION_1,
                                 sizeof(DOT11_ROAMING_START_PARAMETERS));

    roamingStartParameters.uRoamingReason = RoamingReason;

    PlatformIndicateDot11Status(pAd,pPort, NDIS_STATUS_DOT11_ROAMING_START, NULL, &roamingStartParameters, sizeof(DOT11_ROAMING_START_PARAMETERS));
    DBGPRINT(RT_DEBUG_TRACE, ("!! NDIS_STATUS_DOT11_ROAMING_START !!\n"));
    pAd->MlmeAux.bNeedIndicateRoamStatus = TRUE;
}

VOID
N6PlatformIndicateRoamingCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG CompletionStatus)
{
    DOT11_ROAMING_COMPLETION_PARAMETERS   roamingCompleteParameters;

    //
    // Indicate the connection complete status indication
    //
    PlatformZeroMemory(&roamingCompleteParameters, sizeof(DOT11_ROAMING_COMPLETION_PARAMETERS));
    MP_ASSIGN_NDIS_OBJECT_HEADER(roamingCompleteParameters.Header, 
                                 NDIS_OBJECT_TYPE_DEFAULT,
                                 DOT11_ROAMING_COMPLETION_PARAMETERS_REVISION_1,
                                 sizeof(DOT11_ROAMING_COMPLETION_PARAMETERS));
    roamingCompleteParameters.uStatus = CompletionStatus;

    PlatformIndicateDot11Status(pAd, pPort, NDIS_STATUS_DOT11_ROAMING_COMPLETION, NULL, &roamingCompleteParameters, sizeof(DOT11_ROAMING_COMPLETION_PARAMETERS));
    DBGPRINT(RT_DEBUG_TRACE, ("!! NDIS_STATUS_DOT11_ROAMING_COMPLETION !!\n"));
    pAd->MlmeAux.bNeedIndicateRoamStatus = FALSE;   
}


VOID
N6PlatformIndicateLinkQuality(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber
    )
{
    NDIS_STATUS_INDICATION          statusIndication;
    UCHAR                           Buffer[sizeof(DOT11_LINK_QUALITY_PARAMETERS) + sizeof(DOT11_LINK_QUALITY_ENTRY)];
    PDOT11_LINK_QUALITY_PARAMETERS  pLinkQuality;
    DOT11_LINK_QUALITY_ENTRY        LinkQualityEntry;
    CHAR                            Rssi = 0;
    PMP_PORT                      pPort = pAd->PortList[PortNumber];
    
    pLinkQuality = (PDOT11_LINK_QUALITY_PARAMETERS) Buffer;
    MP_ASSIGN_NDIS_OBJECT_HEADER(pLinkQuality->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_LINK_QUALITY_PARAMETERS_REVISION_1,
                                sizeof(DOT11_LINK_QUALITY_PARAMETERS));

    pLinkQuality->uLinkQualityListSize = 1;
    pLinkQuality->uLinkQualityListOffset = sizeof(DOT11_LINK_QUALITY_PARAMETERS);

    COPY_MAC_ADDR(LinkQualityEntry.PeerMacAddr, pPort->PortCfg.Bssid);

    if (pPort->PortSubtype == PORTSUBTYPE_STA)
    {
        if (pAd->HwCfg.Antenna.field.TxPath == 3)
        {
            Rssi = (pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1] + pAd->StaCfg.RssiSample.AvgRssi[2]) / 3;
        }
        else if (pAd->HwCfg.Antenna.field.TxPath == 2)
        {
            Rssi = (pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1]) >> 1;
            DBGPRINT(RT_DEBUG_TRACE, ("RTMPIndicateLinkQuality, RSSI = %d\n", Rssi));
        }
        else
        {
            Rssi = pAd->StaCfg.RssiSample.AvgRssi[0];
        }
        
        if ((pAd->HwCfg.BLNAGain != 0xFF) && (pAd->HwCfg.Antenna.field.RssiIndicationMode == USER_AWARENESS_MODE))
        {
            Rssi = Rssi + GET_LNA_GAIN(pAd); // RSSI indication by Rx LNA output

            DBGPRINT(RT_DEBUG_TRACE, ("PlatformIndicateLinkQuality, USER_AWARENESS_MODE, RSSI = %d\n", Rssi));
        }
    }

    LinkQualityEntry.ucLinkQuality = (UCHAR) MlmeInfoGetLinkQuality(pAd, Rssi,TRUE);

    if (MlmeInfoGetLinkQualityLevel(pPort->LastLinkQuality) != MlmeInfoGetLinkQualityLevel(LinkQualityEntry.ucLinkQuality))
    {
        PlatformMoveMemory(Add2Ptr(pLinkQuality, sizeof(DOT11_LINK_QUALITY_PARAMETERS)),
                        &LinkQualityEntry,
                        sizeof(DOT11_LINK_QUALITY_ENTRY));
    
        //
        // Fill in object headers
        //
        statusIndication.Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;
        statusIndication.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
        statusIndication.Header.Size = sizeof(NDIS_STATUS_INDICATION);
    
        // fill in the correct port number
        statusIndication.PortNumber = PortNumber;
        //
        // Fill in the status buffer
        // 
        statusIndication.StatusCode = NDIS_STATUS_DOT11_LINK_QUALITY;
        statusIndication.SourceHandle = pAd->AdapterHandle;
        statusIndication.DestinationHandle = NULL;
        statusIndication.RequestId = 0;
    
        statusIndication.StatusBuffer = Buffer;
        statusIndication.StatusBufferSize = sizeof(DOT11_LINK_QUALITY_PARAMETERS) + sizeof(DOT11_LINK_QUALITY_ENTRY);
    
        //NdisMIndicateStatusEx(
        //                  pAd->AdapterHandle,
        //                  &statusIndication
        //                  );  
        PlatformIndicateStatusToNdis(pAd, &statusIndication);
        pPort->LastLinkQuality = LinkQualityEntry.ucLinkQuality;
    }
}

VOID
N6PlatformIndicateNewLinkSpeed(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN UCHAR            OpMode)
{
    NDIS_STATUS_INDICATION  statusIndication;
    NDIS_LINK_STATE         linkState;
    ULONG                   rateIdx;
    PMP_PORT              pPort = pAd->PortList[PortNumber];
    ULONG64             CurrentRate = pPort->LastIndicateRate;
    UCHAR               initOffset = 16;
    

    PlatformZeroMemory(&statusIndication, sizeof(NDIS_STATUS_INDICATION));
    PlatformZeroMemory(&linkState, sizeof(NDIS_LINK_STATE));

    //
    // Fill in object headers
    //
    statusIndication.Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;
    statusIndication.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
    statusIndication.Header.Size = sizeof(NDIS_STATUS_INDICATION);

    // fill in the correct port number
    statusIndication.PortNumber = PortNumber;

    linkState.Header.Revision = NDIS_LINK_STATE_REVISION_1;
    linkState.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    linkState.Header.Size = sizeof(NDIS_LINK_STATE);
    
    do 
    {

        //
        // Link state buffer
        //
        linkState.MediaConnectState = MediaConnectStateConnected;
        linkState.MediaDuplexState = MediaDuplexStateFull;
        linkState.RcvLinkSpeed = HW_REPORT_MAX_SPEED; //300Mbps

        if (pAd->StaCfg.FixLinkSpeedStatus == 0) // show Tx rate
        {           
            if ((pPort->PortSubtype == PORTSUBTYPE_STA) ||(pPort->PortSubtype == PORTSUBTYPE_P2PClient))
            {       
                if (pAd->StaCfg.BssType == BSS_ADHOC)
                    CurrentRate = MlmeInfoQueryLinkSpeed(pAd, pPort, &pPort->CommonCfg.TxPhyCfg);
                else
                    CurrentRate = MlmeInfoQueryLinkSpeed(pAd, pPort, &pPort->LastTxRatePhyCfg);     
            }
            else
            {
                if (((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) && (pPort->CommonCfg.PhyMode <= PHY_11G)) ||
                       ((pPort->CommonCfg.bProhibitTKIPonHT) && 
                       ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||
                       (pPort->PortCfg.CipherAlg == CIPHER_TKIP)||
                       (pPort->PortCfg.CipherAlg == CIPHER_TKIP_NO_MIC))))              
                 {
                    if (pPort->CommonCfg.PhyMode == PHY_11B)
                    {
                        // maximum 11b rate
                        CurrentRate  = 11000000;
                    }
                    else
                    {
                        // maximum 11g rate
                        CurrentRate  = 54000000;
                    }               
                }
                else
                {
                    if (VHT_CAPABLE(pAd))
                    {
                        if (pAd->HwCfg.Antenna.field.RxPath == 2) // 2x2
                        {
                            rateIdx = GetVhtRateTableIdx(pAd, 
                                                        READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                        NSS_1, 
                                                        READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                        MCS_7); //2 TODO: Future NICs may support 256 QAM.
                        }
                        else // 1x1
                        {
                            rateIdx = GetVhtRateTableIdx(pAd, 
                                                        READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                        NSS_0, 
                                                        READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                        MCS_7); //2 TODO: Future NICs may support 256 QAM.
                        }
                            // Avoid getting the zero VhtRate, when MCS=9 BW=0 GI=1 NSS=0 (rateIdx=169)
                            if ((rateIdx!=0)&&VhtRateTable[rateIdx]==0)
                            {
                                rateIdx=rateIdx-1;
                            }
                            CurrentRate = VhtRateTable[rateIdx] * 5000 * 100;
                        }
                    else
                    {
                    // Report maximize Tx Rate
                    UCHAR TXMaxMCS = 15;
                    
                    if (pAd->HwCfg.Antenna.field.TxPath == 1)
                    {
                        TXMaxMCS = 7;
                    }
                    else if (pAd->HwCfg.Antenna.field.TxPath == 2)
                    {
                        TXMaxMCS = 15;
                    }
                    else if (pAd->HwCfg.Antenna.field.TxPath == 3)
                    {
                        TXMaxMCS = MCS_23;
                        initOffset = 64;
                    }
                    else
                    {
                        ASSERT(0);
                    }
                    
                        rateIdx = initOffset + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth *16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 *32) + TXMaxMCS;
                        CurrentRate = (CckOfdmHtRateTable[rateIdx] * 5000 * 100);  // in bits per second, OID_GEN_LINK_SPEED is in 100 bits per second.
                    }
                }
            }

            // CH14 use maximum 11b 
            if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
            {
                CurrentRate = 11000000; //11b
            }           
        }
        else if (pAd->StaCfg.FixLinkSpeedStatus == 1) // show Fix Rx rate
        {
            UCHAR MCSRate;

            if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP ||(pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            {

                if (VHT_CAPABLE(pAd))
                {
                    if (pAd->HwCfg.Antenna.field.RxPath == 2) // 2x2
                    {
                        rateIdx = GetVhtRateTableIdx(pAd,
                                                    READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg),
                                                    NSS_1,
                                                    READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg),
                                                    MCS_7); //2 TODO: Future NICs may support 256 QAM.
                    }
                    else // 1x1
                    {
                        rateIdx = GetVhtRateTableIdx(pAd,
                                                    READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg),
                                                    NSS_0,
                                                    READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg),
                                                    MCS_7); //2 TODO: Future NICs may support 256 QAM.
                    }
                    // Avoid getting the zero VhtRate, when MCS=9 BW=0 GI=1 NSS=0 (rateIdx=169)
                    if ((rateIdx!=0)&&VhtRateTable[rateIdx]==0)
                    {
                        rateIdx=rateIdx-1;
                    }

                    CurrentRate = VhtRateTable[rateIdx] * 5000 * 100;
                }
                else
                {
                if (pAd->HwCfg.Antenna.field.TxPath == 1)
                {
                    CurrentRate = 150000000;
                }
                else if (pAd->HwCfg.Antenna.field.TxPath == 2)
                {
                        rateIdx = 16 + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth * 16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 * 32) + MCS_15;
                        CurrentRate = CckOfdmHtRateTable[rateIdx] * 5000 * 100;
                }
                else if (pAd->HwCfg.Antenna.field.TxPath == 3)
                {
                        rateIdx = 64 + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth * 16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 * 32) + MCS_23;
                        CurrentRate = CckOfdmHtRateTable[rateIdx] * 5000  * 100;
                }
                else
                {
                    ASSERT(0);
                }
                }
            }
            else
            {
                if (pPort->PortType == EXTAP_PORT)
                {
                    if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) // N-capable
                    {
                        if (pAd->HwCfg.Antenna.field.RxPath >= 2)
                        {
                            //EEEPC need to report the maximun rx link speed
                            //So we force linkspeed to MCS 15 in two RxPath 
                            MCSRate = MCS_15;
                        }
                        else
                        {
                            //EEEPC need to report the maximun rx link speed
                            //So we force linkspeed to MCS 7 in 2 RxPath 
                            MCSRate = MCS_7;
                        }
        
                        //EEEPC ask us to no change data rate due to environment in connection. 
                        //So I change the ShortGI and BW  to max AP can support. No change due to environment.
                        
                        rateIdx = initOffset + ((UCHAR)READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg) * 16) + (READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg) * 32) + MCSRate;
                        
                        CurrentRate = CckOfdmHtRateTable[rateIdx] *5000 *100;
                    }
                    else // Legacy
                    {
                        if (pPort->CommonCfg.MaxTxRate < RATE_6)
                            CurrentRate = 11000000;
                        else
                            CurrentRate = 54000000;                         
                    }
                }
                else
                {
                    //
                    // pPort->PortType == EXTSTA_PORT
                    //
                    if (ADHOC_ON(pPort))
                    {
                        // [ADHOC PER ENTRY]
                        // Any 11n join us, change to Ht rate.
                        if ((pAd->StaCfg.bAdhocNMode) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE || pAd->StaCfg.AdhocCreator))
                        {   
                            if (VHT_CAPABLE(pAd))
                            {
                                if (pAd->HwCfg.Antenna.field.RxPath == 2) // 2x2
                                {
                                    rateIdx = GetVhtRateTableIdx(pAd, 
                                                       READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       NSS_1, 
                                                       READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       MCS_7); //2 TODO: Future NICs may support 256 QAM.
                                }
                                else // 1x1
                                {
                                    rateIdx = GetVhtRateTableIdx(pAd, 
                                                       READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       NSS_0, 
                                                       READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       MCS_7); //2 TODO: Future NICs may support 256 QAM.
                                }
                                // Avoid getting the zero VhtRate, when MCS=9 BW=0 GI=1 NSS=0 (rateIdx=169)
                                if ((rateIdx!=0)&&VhtRateTable[rateIdx]==0)
                                {
                                    rateIdx=rateIdx-1;
                                }

                                CurrentRate = VhtRateTable[rateIdx] * 5000 * 100;
                            }
                            else
                            {
    
                            if (pAd->HwCfg.Antenna.field.RxPath >= 2)
                            {
                                // EEEPC need to report the maximun rx link speed
                                // So we force linkspeed to MCS 15 in two RxPath 
                                MCSRate = MCS_15;
                            }
                            else
                            {
                                // EEEPC need to report the maximun rx link speed
                                // So we force linkspeed to MCS 7 in one RxPath 
                                MCSRate = MCS_7;
                            }

                            // We force show LINK SPEED in BW=20MHz and ShortGI=0.
                            // In other words, we show 65Mbps (1R Path) or 130Mbps (2R Path).
                                rateIdx = initOffset + ((UCHAR)READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg) * 16) + READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg) * 32 + MCSRate;
                                CurrentRate = CckOfdmHtRateTable[rateIdx] * 5000 * 100;
                            }
                        }                       
                        //
                        //  In Legacy Case.  
                        //   (i) As a client (Joiner).
                        //       We link to Server in B mode. We are N or G. We show 11 Mbps.
                        //       We link to Server in G mode. We are N or G. We show 54 Mbps.
                        //       We link to Server in N mode, but we are G (N disable). We show 54Mbps.
                        
                        //  (ii) As a server (Creator).
                        //       We are in G mode(N disable). Client link to us in B/G/N mixed mode. We show 54 Mbps.
                        else
                        {
                            // [ADHOC PER ENTRY]
                            // Any 11g join us, change to g rate.
                            if (pPort->MacTab.fAnyStationIsCCK && !pPort->MacTab.fAnyStationIsOFDM)
                                CurrentRate = 11000000;
                            else
                                CurrentRate = 54000000;
                        }
                    }
                    else // INFRA_ON
                    {
                        if (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE) // N-capable
                        {

                            if (VHT_CAPABLE(pAd))
                            {
                                if (pAd->HwCfg.Antenna.field.RxPath == 2) // 2x2
                                {
                                    rateIdx = GetVhtRateTableIdx(pAd, 
                                                       READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       NSS_1, 
                                                       READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       MCS_7); //2 TODO: Future NICs may support 256 QAM.
                                }
                                else // 1x1
                                {
                                    rateIdx = GetVhtRateTableIdx(pAd, 
                                                       READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       NSS_0, 
                                                       READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                                       MCS_7); //2 TODO: Future NICs may support 256 QAM.
                                }
                                // Avoid getting the zero VhtRate, when MCS=9 BW=0 GI=1 NSS=0 (rateIdx=169)
                                if ((rateIdx!=0)&&VhtRateTable[rateIdx]==0)
                                {
                                    rateIdx=rateIdx-1;
                                }

                                CurrentRate = VhtRateTable[rateIdx] * 5000 * 100;
                            }
                            else
                            {
                            if (pAd->HwCfg.Antenna.field.RxPath >= 2)
                            {
                                //EEEPC need to report the maximun rx link speed
                                //So we force linkspeed to MCS 15 in two RxPath 
                                MCSRate = MCS_15;
                            }
                            else
                            {
                                //EEEPC need to report the maximun rx link speed
                                //So we force linkspeed to MCS 7 in 2 RxPath 
                                MCSRate = MCS_7;
                            }

                            //EEEPC ask us to no change data rate due to environment in connection. 
                            //So I change the ShortGI and BW  to max AP can support. No change due to environment.
                                rateIdx = initOffset + ((UCHAR)READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg) * 16) + (READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg) * 32) + MCSRate;
                                CurrentRate = CckOfdmHtRateTable[rateIdx] *5000 *100;
                            }
                        }
                        else // Legacy
                        {
                            if (pPort->CommonCfg.MaxTxRate < RATE_6)
                                CurrentRate = 11000000;
                            else
                                CurrentRate = 54000000;                         
                        }
                    }
                }
            }
        }
        else if (pAd->StaCfg.FixLinkSpeedStatus == 2) // show Rx Rate For Belkin
        {           
            if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP ||(pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            {
                if (VHT_CAPABLE(pAd))
                {
                    if (pAd->HwCfg.Antenna.field.RxPath == 2) // 2x2
                    {
                        rateIdx = GetVhtRateTableIdx(pAd, 
                                        READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                        NSS_1, 
                                        READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                        MCS_7); //2 TODO: Future NICs may support 256 QAM.
                    }
                    else // 1x1
                    {
                        rateIdx = GetVhtRateTableIdx(pAd, 
                                        READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                        NSS_0, 
                                        READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg), 
                                        MCS_7); //2 TODO: Future NICs may support 256 QAM.
                    }
                    // Avoid getting the zero VhtRate, when MCS=9 BW=0 GI=1 NSS=0 (rateIdx=169)
                    if ((rateIdx!=0)&&VhtRateTable[rateIdx]==0)
                    {
                        rateIdx=rateIdx-1;
                    }

                    CurrentRate = VhtRateTable[rateIdx] * 5000 * 100;
                }
                else
                {

                if (pAd->HwCfg.Antenna.field.TxPath == 1)
                {
                    CurrentRate = 150000000;
                }
                else if (pAd->HwCfg.Antenna.field.TxPath == 2)
                {
                        rateIdx = 16 + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth * 16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 * 32) + MCS_15;
                        CurrentRate = CckOfdmHtRateTable[rateIdx] * 5000 * 100;
                }
                else if (pAd->HwCfg.Antenna.field.TxPath == 3)
                {
                        rateIdx = 64 + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.HtChannelWidth * 16) + ((UCHAR)pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 * 32) + MCS_23;
                        CurrentRate = CckOfdmHtRateTable[rateIdx] * 5000  * 100;
                }
                else
                {
                    ASSERT(0);
                }
                }
            }
            else
            {
                CurrentRate = MlmeInfoQueryLinkSpeed(pAd, pPort, &pPort->LastRxRatePhyCfg);
            }
        }

        //
        // MS requests not to indicate tx rate if not changed.
        //
        if (CurrentRate != pPort->LastIndicateRate )
        {
            linkState.XmitLinkSpeed = CurrentRate; 
        }
        else
        {
            break;
        }
        
        //
        // Fill in the status buffer
        // 
        statusIndication.StatusCode = NDIS_STATUS_LINK_STATE;
        statusIndication.SourceHandle = pAd->AdapterHandle;
        statusIndication.DestinationHandle = NULL;
        statusIndication.RequestId = 0;
    
        statusIndication.StatusBuffer = &linkState;
        statusIndication.StatusBufferSize = sizeof(NDIS_LINK_STATE);

        //NdisMIndicateStatusEx(
        //                  pAd->AdapterHandle,
        //                  &statusIndication
        //                  );
        PlatformIndicateStatusToNdis(pAd, &statusIndication);

        pPort->LastIndicateRate = CurrentRate;

    }while(FALSE);
}

VOID
N6PlatformIndicateCurrentPhyPowerState(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN ULONG    PhyId)
{
    NDIS_STATUS_INDICATION  statusIndication;
    DOT11_PHY_STATE_PARAMETERS phyStateParams;
    PlatformZeroMemory(&statusIndication, sizeof(NDIS_STATUS_INDICATION));
    PlatformZeroMemory(&phyStateParams, sizeof(DOT11_PHY_STATE_PARAMETERS));

    if ((pAd->StaCfg.LastSwRadio == pAd->StaCfg.bSwRadio) &&
        (pAd->StaCfg.LastHwRadio == pAd->StaCfg.bHwRadio))
    {
        //
        // No need to indicate NDIS_STATUS_DOT11_PHY_STATE_CHANGED if it is at the same state.
        //
        DBGPRINT(RT_DEBUG_INFO, ("PlatformIndicateCurrentPhyPowerState, not need to indicate DOT11_PHY_STATE_CHANGED, bSwRadio=%d, LastSwRadio=%d, bHwRadio=%d, LastHwRadio=%d\n",
                pAd->StaCfg.bSwRadio, pAd->StaCfg.LastSwRadio, pAd->StaCfg.bHwRadio, pAd->StaCfg.LastHwRadio));

        return;     
    }

    //
    // Fill in object headers
    //
    statusIndication.Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;
    statusIndication.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
    statusIndication.Header.Size = sizeof(NDIS_STATUS_INDICATION);
    phyStateParams.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    phyStateParams.Header.Revision = DOT11_PHY_STATE_PARAMETERS_REVISION_1;
    phyStateParams.Header.Size = sizeof(DOT11_PHY_STATE_PARAMETERS);

    //
    // Phy state buffer
    //
    phyStateParams.uPhyId = PhyId;

    // 
    // bHardwarePhyState/bSoftwarePhyState 
    // If TRUE, the hardware/software power state is enabled. If FALSE, the hardware/software power state is disabled.
    //
    phyStateParams.bHardwarePhyState = pAd->StaCfg.bHwRadio;
    phyStateParams.bSoftwarePhyState = pAd->StaCfg.bSwRadio;

    //
    // Save the last Radio status.
    //
    pAd->StaCfg.LastSwRadio = pAd->StaCfg.bSwRadio;
    pAd->StaCfg.LastHwRadio = pAd->StaCfg.bHwRadio;

    statusIndication.PortNumber = PortNumber;

    //
    // Fill in the status buffer
    // 
    statusIndication.StatusCode = NDIS_STATUS_DOT11_PHY_STATE_CHANGED;
    statusIndication.SourceHandle = pAd->AdapterHandle;
    statusIndication.DestinationHandle = NULL;
    statusIndication.RequestId = 0;

    statusIndication.StatusBuffer = &phyStateParams;
    statusIndication.StatusBufferSize = sizeof(DOT11_PHY_STATE_PARAMETERS);

    //NdisMIndicateStatusEx(
    //                  pAd->AdapterHandle,
    //                  &statusIndication
    //                  );
    PlatformIndicateStatusToNdis(pAd, &statusIndication);

    DBGPRINT(RT_DEBUG_TRACE, ("PlatformIndicateCurrentPhyPowerState:: bHwRadio(=%d), bSwRadio(=%d)\n", pAd->StaCfg.bHwRadio, pAd->StaCfg.bSwRadio));
}


/*
    ========================================================================
    
    Routine Description:
        A miniport driver must make a NDIS_STATUS_DOT11_DISASSOCIATION indication 
        if the 802.11 station performs a disassociation operation with either an 
        access point (AP) or peer station.

    Arguments:
        pAd           Pointer to our adapter
        MacAddress    MAC Address of the AP or peer station that the 802.11 station has disassociated from.
                      0xFFFFFFFFFFFF means the 802.11 station has disassociated from the AP or all peer stations.
        Reason        the disassociation reason code.

    Return Value:
        NONE
        
    Note:
    
    ========================================================================
*/
VOID
N6PlatformIndicateDisassociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR   MacAddress,
    IN ULONG Reason)
{
    DOT11_DISASSOCIATION_PARAMETERS disassocParameters;

    PlatformZeroMemory(&disassocParameters, sizeof(DOT11_DISASSOCIATION_PARAMETERS));
    MP_ASSIGN_NDIS_OBJECT_HEADER(disassocParameters.Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_DISASSOCIATION_PARAMETERS_REVISION_1,
                                sizeof(DOT11_DISASSOCIATION_PARAMETERS));
    
    disassocParameters.uIHVDataOffset = 0;
    disassocParameters.uIHVDataSize = 0;
    // Copy the MAC address from the AP entry
    PlatformMoveMemory(&disassocParameters.MacAddr, MacAddress, MAC_ADDR_LEN);

    disassocParameters.uReason = Reason;

    PlatformIndicateDot11Status(pAd, pPort,NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocParameters, sizeof(DOT11_DISASSOCIATION_PARAMETERS));
    DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_DOT11_DISASSOCIATION\n"));
}


VOID
N6PlatformIndicateAdhocAssociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT      pPort,
    IN PUCHAR           MacAddress)
{
    DOT11_ASSOCIATION_START_PARAMETERS          assocStart;
    ULONG                                       assocCompSize;
    PDOT11_ASSOCIATION_COMPLETION_PARAMETERS    assocComp;
    PUCHAR                                      pTempPtr = NULL;

    // indicate link speed here to make sure the information for netsh wlan command is correct
    N6PlatformIndicateLinkSpeedForNetshCmd(pAd);
    //
    // Allocate enough memory for ASSOCIATION_COMPLETE indication. If allocation fails,
    // we skip this beaconing station.
    //
    assocCompSize = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS) +
                    pAd->StaCfg.BeaconFrameSize +   // for beacon
                    sizeof(ULONG);                  // for single entry PHY list
    

    PlatformAllocateMemory(pAd,  &assocComp, assocCompSize);
    if (assocComp == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("PlatformIndicateAdhocAssociation, Allocate assocComp for indication failed\n"));
        return;
    }

    //
    // Indicate ASSOCIATION_START
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(assocStart.Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_ASSOCIATION_START_PARAMETERS_REVISION_1,
                                sizeof(DOT11_ASSOCIATION_START_PARAMETERS));

    assocStart.uIHVDataOffset = 0;
    assocStart.uIHVDataSize = 0;

    PlatformMoveMemory(&assocStart.SSID, &pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
    COPY_MAC_ADDR(&assocStart.MacAddr, MacAddress);

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_ASSOCIATION_START,
                        NULL,
                        &assocStart,
                        sizeof(DOT11_ASSOCIATION_START_PARAMETERS));


    DBGPRINT(RT_DEBUG_ERROR, ("PlatformIndicateAdhocAssociation: NDIS_STATUS_DOT11_ASSOCIATION_START, MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    MacAddress[0], MacAddress[1], MacAddress[2],
                    MacAddress[3], MacAddress[4], MacAddress[5]));

    //
    // Indicate ASSOCIATION_COMPLETE
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(assocComp->Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_ASSOCIATION_COMPLETION_PARAMETERS_REVISION_1,
                                sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));

    COPY_MAC_ADDR(&assocComp->MacAddr, MacAddress);
    assocComp->uStatus = 0;

    assocComp->bReAssocReq = FALSE;
    assocComp->bReAssocResp = FALSE;
    assocComp->uAssocReqOffset = 0;
    assocComp->uAssocReqSize = 0;
    assocComp->uAssocRespOffset = 0;
    assocComp->uAssocRespSize = 0;

    //
    // Append the beacon information of this beaconing station.
    //
    pTempPtr = Add2Ptr(assocComp, sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));
    PlatformMoveMemory(pTempPtr, pAd->StaCfg.BeaconFrame, pAd->StaCfg.BeaconFrameSize);
    assocComp->uBeaconOffset = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS);
    assocComp->uBeaconSize = pAd->StaCfg.BeaconFrameSize;

    assocComp->uIHVDataOffset = 0;
    assocComp->uIHVDataSize = 0;

    //
    // Set the auth and cipher algorithm.
    //
    assocComp->AuthAlgo = pPort->PortCfg.AuthMode;
    assocComp->UnicastCipher = pPort->PortCfg.PairCipher; 
    assocComp->MulticastCipher = pPort->PortCfg.GroupCipher; 
    DBGPRINT(RT_DEBUG_TRACE, ("PlatformIndicateAdhocAssociation:AuthAlgo =%x, UnicastCipher=%x, MulticastCipher =%x\n",
        assocComp->AuthAlgo,assocComp->UnicastCipher,assocComp->MulticastCipher));

    //
    // Set the PHY list. It just contains our active phy id.
    //
    assocComp->uActivePhyListOffset = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS) + 
                                        pAd->StaCfg.BeaconFrameSize;
    assocComp->uActivePhyListSize = sizeof(ULONG);
    *((ULONG UNALIGNED *)Add2Ptr(assocComp, assocComp->uActivePhyListOffset)) = pAd->StaCfg.ActivePhyId;

    assocComp->bFourAddressSupported = FALSE;
    assocComp->bPortAuthorized = FALSE;
    assocComp->DSInfo = DOT11_DS_UNKNOWN;

    assocComp->uEncapTableOffset = 0;
    assocComp->uEncapTableSize = 0;

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION,
                        NULL,
                        assocComp,
                        assocCompSize);

    DBGPRINT(RT_DEBUG_ERROR, ("PlatformIndicateAdhocAssociation: NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION, MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    MacAddress[0], MacAddress[1], MacAddress[2],
                    MacAddress[3], MacAddress[4], MacAddress[5]));
    //
    // Free the preallocated ASSOCIATION_COMPLETE indication structure.
    //
    PlatformFreeMemory(assocComp, assocCompSize);

    pAd->MlmeAux.AssocState = dot11_assoc_state_auth_assoc;
}


VOID   
N6PlatformIndicateScanStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     ScanConfirmStatus,
    IN  BOOLEAN         ClearBssScanFlag,
    IN  BOOLEAN         SetSyncIdle
    )
{
    PDOT11_BYTE_ARRAY   pDot11ByteArray = NULL;
    NDIS_STATUS         ndisStatus;

    DBGPRINT(RT_DEBUG_TRACE, ("N6PlatformIndicateScanStatus ==> \n"));
    
    if (pAd->MlmeAux.bNeedPlatformIndicateScanStatus)
    {
        pAd->MlmeAux.bNeedPlatformIndicateScanStatus = FALSE;       
        
        if(ClearBssScanFlag)
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);

        NdisAcquireSpinLock(&pAd->pNicCfg->PendingOIDLock);
        if(pAd->pNicCfg->PendOidEntry.PendingNdisRequest != NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("N6PlatformIndicateScanStatus : return Pending Enum Bss List OID\n"));

            pAd->pNicCfg->bUpdateBssList = TRUE;
            ndisStatus = NdisInitGetBssList(pAd, pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.InformationBuffer, pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.OutputBufferLength);

            pDot11ByteArray = (PDOT11_BYTE_ARRAY)pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.InformationBuffer;
            
            if ( ndisStatus == NDIS_STATUS_BUFFER_OVERFLOW)
            {
                pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.BytesWritten = 0;
                pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.BytesNeeded = pDot11ByteArray->uTotalNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
            }
            else
            {
                pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.BytesWritten = pDot11ByteArray->uNumOfBytes + FIELD_OFFSET(DOT11_BYTE_ARRAY, ucBuffer);
                pAd->pNicCfg->PendOidEntry.PendingNdisRequest->DATA.METHOD_INFORMATION.BytesNeeded = 0;
            }
            NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
            
            N6CompletePendingOID( pAd, PENDING_ENUM_BSS_LIST, NDIS_STATUS_SUCCESS);
        }
        else
        {
            NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
        }

        //
        // Report Scan fail to let UI not to update BSS list.
        // Based on whql test's Security_cmn.cpp, we should indicate NDIS_STATUS_SUCCESS even if we can't scan;
        //
        ndisStatus = ScanConfirmStatus;
        PlatformIndicateDot11Status(pAd,
                            pPort,
                            NDIS_STATUS_DOT11_SCAN_CONFIRM,
                            pAd->pNicCfg->ScanRequestID,
                            &ndisStatus,
                            sizeof(NDIS_STATUS));
        DBGPRINT(RT_DEBUG_TRACE, ("N6PlatformIndicateScanStatus : Indicate scan confirm,requestID=%p\n",pAd->pNicCfg->ScanRequestID));

        if(SetSyncIdle)
            pPort->Mlme.SyncMachine.CurrState    = SYNC_IDLE;

    }

#ifdef MULTI_CHANNEL_SUPPORT
    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_MULTI_CHANNEL_SCAN_DONE_EXEC, NULL, 0);     
    }
#endif /*MULTI_CHANNEL_SUPPORT*/    
    
    DBGPRINT(RT_DEBUG_TRACE, ("<== N6PlatformIndicateScanStatus \n"));
}

VOID 
N6PlatformIndicateNLOStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    )
{
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    NDIS_STATUS_INDICATION  statusIndication;

    NDIS_STATUS  ndisStatus = NDIS_STATUS_SUCCESS;

    PDOT11_OFFLOAD_NETWORK_STATUS_PARAMETERS pNLOStatus;

    UCHAR               Buffer[DOT11_SIZEOF_OFFLOAD_NETWORK_STATUS_PARAMETERS_REVISION_1];
    
    pNLOStatus =    (PDOT11_OFFLOAD_NETWORK_STATUS_PARAMETERS) Buffer;

    MP_ASSIGN_NDIS_OBJECT_HEADER(pNLOStatus->Header,
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_OFFLOAD_NETWORK_STATUS_PARAMETERS_REVISION_1,
                                DOT11_SIZEOF_OFFLOAD_NETWORK_STATUS_PARAMETERS_REVISION_1);

    pNLOStatus->Status = STATUS_SUCCESS;

    PlatformZeroMemory(&statusIndication, sizeof(NDIS_STATUS_INDICATION));
    //
    // Fill in object header
    //
    statusIndication.Header.Type     = NDIS_OBJECT_TYPE_STATUS_INDICATION;
    statusIndication.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
    statusIndication.Header.Size     = sizeof(NDIS_STATUS_INDICATION);

    // Fill in the port number
    statusIndication.PortNumber = pPort->PortNumber;

    //
    // Fill in the rest of the structure
    // 
    statusIndication.StatusCode          = NDIS_STATUS_DOT11_OFFLOAD_NETWORK_STATUS_CHANGED;
    statusIndication.SourceHandle        = pAd->AdapterHandle;
    statusIndication.DestinationHandle   = NULL;
    statusIndication.RequestId           = NULL;

// WDK is different to OID_Spec v1.4
#if 1
    statusIndication.StatusBuffer = NULL;
    statusIndication.StatusBufferSize = 0;
#else
    // ?? 
    pNLOStatus->Status = NDIS_STATUS_SUCCESS;

    statusIndication.StatusBuffer = Buffer;
    statusIndication->StatusBufferSize    = DOT11_SIZEOF_OFFLOAD_NETWORK_STATUS_PARAMETERS_REVISION_1;
#endif

    //NdisMIndicateStatusEx(
    //  pAd->AdapterHandle,
    //  &statusIndication
    //  );
    PlatformIndicateStatusToNdis(pAd, &statusIndication);

    DBGPRINT(RT_DEBUG_TRACE, (" %s - Indicate NLO \n", __FUNCTION__));
#endif
}

VOID 
N6PlatformIndicateLinkSpeedForNetshCmd(
    IN PMP_ADAPTER    pAd
    )
{
    NDIS_STATUS_INDICATION  statusIndication;
    NDIS_LINK_STATE         linkState;

    PlatformZeroMemory(&statusIndication, sizeof(NDIS_STATUS_INDICATION));
    PlatformZeroMemory(&linkState, sizeof(NDIS_LINK_STATE));
    //
    // Fill in object headers
    //
    linkState.Header.Revision = NDIS_LINK_STATE_REVISION_1;
    linkState.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    linkState.Header.Size = sizeof(NDIS_LINK_STATE);
    linkState.MediaConnectState = MediaConnectStateConnected;
    linkState.MediaDuplexState = MediaDuplexStateFull;
    
    statusIndication.Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;
    statusIndication.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
    statusIndication.Header.Size = sizeof(NDIS_STATUS_INDICATION);
    statusIndication.PortNumber = PORT_0;
    statusIndication.StatusCode = NDIS_STATUS_LINK_STATE;
    statusIndication.SourceHandle = pAd->AdapterHandle;
    statusIndication.DestinationHandle = NULL;
    statusIndication.RequestId = 0;

    statusIndication.StatusBuffer = &linkState;
    statusIndication.StatusBufferSize = sizeof(NDIS_LINK_STATE);

    switch(pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[pAd->StaCfg.ActivePhyId])
    {       
        case dot11_phy_type_hrdsss: // 802.11b
            linkState.XmitLinkSpeed = 11000000;
            break;
        case dot11_phy_type_erp:
        case dot11_phy_type_ofdm:           
            linkState.XmitLinkSpeed = 54000000;
            break;
        case dot11_phy_type_ht:
            if(pAd->HwCfg.Antenna.field.TxPath == 3)
                linkState.XmitLinkSpeed = 450000000;
            else if(pAd->HwCfg.Antenna.field.TxPath == 2)
                linkState.XmitLinkSpeed = 300000000;
            else
                linkState.XmitLinkSpeed = 150000000;
            break;
        default:
            linkState.XmitLinkSpeed = 11000000;
            break;
    }   
    //NdisMIndicateStatusEx(
    //                  pAd->AdapterHandle,
    //                  &statusIndication
    //                  );
    PlatformIndicateStatusToNdis(pAd, &statusIndication);
}

VOID
N6PlatformIndicateMICFailure(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR   MacAddress,
    IN ULONG    KeyId,
    IN BOOLEAN  IsDefaultKey
    )
{
    PMP_PORT                          pPort = pAd->PortList[PORT_0];
    DOT11_TKIPMIC_FAILURE_PARAMETERS    param;

    RecvReportMicError(pAd, NULL);
    
    MP_ASSIGN_NDIS_OBJECT_HEADER(param.Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_TKIPMIC_FAILURE_PARAMETERS_REVISION_1,
                                sizeof(DOT11_TKIPMIC_FAILURE_PARAMETERS));

    param.bDefaultKeyFailure = IsDefaultKey;
    param.uKeyIndex = KeyId;
    COPY_MAC_ADDR(&param.PeerMac, MacAddress);
    PlatformIndicateDot11Status(pAd,
                        pPort,
                        NDIS_STATUS_DOT11_TKIPMIC_FAILURE,
                        NULL,
                        &param,
                        sizeof(DOT11_TKIPMIC_FAILURE_PARAMETERS));  
    
    DBGPRINT(RT_DEBUG_TRACE, ("N6PlatformIndicateMICFailure, KeyId=%d,IsDefaultKey=%d,MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    KeyId, IsDefaultKey, MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]));
}

