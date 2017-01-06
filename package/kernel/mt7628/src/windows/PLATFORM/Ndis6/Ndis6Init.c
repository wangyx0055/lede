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
    Ndis6Init.c

    Abstract:
    Miniport main initialization routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#include    "MtConfig.h"

extern ULONG    G_MemoryAllocate_cnt;
extern ULONG    G_MemoryFree_cnt;
extern ULONG    G_MemoryAllocate_Len;
extern ULONG    G_MemoryFree_Len;

WDF_REQUEST_SEND_OPTIONS  G_RequestOptions;



/*
    ========================================================================
    
    Routine Description:
        Allocate MP_PORT data block and do some initialization

    Arguments:
        MP_Port     Pointer to port structure

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_FAILURE

    IRQL = PASSIVE_LEVEL

    Note:
    
    ========================================================================
*/
NDIS_STATUS N6AllocPort(
    IN    NDIS_HANDLE      MiniportAdapterHandle,
    IN    MP_PORT_TYPE     PortType,
    IN    MP_PORT_SUBTYPE  PortSubType,
    OUT   PMP_PORT       *ppPort)
{
    PMP_PORT pPort;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    *ppPort = NULL;
    FUNC_ENTER;
    
    do
    {
        // Allocate MP_ADAPTER memory block
        pPort = (PMP_PORT)NdisAllocateMemoryWithTagPriority( MiniportAdapterHandle, 
                                                                                            sizeof(MP_PORT), 
                                                                                            NIC_TAG,
                                                                                            NormalPoolPriority);



        if (pPort == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Failed to allocate memory - ADAPTER\n"));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        G_MemoryAllocate_cnt ++;
        G_MemoryAllocate_Len += sizeof(MP_PORT);        // Clean up the memory block

        PlatformZeroMemory(pPort, sizeof(MP_PORT));
        pPort->PortType = PortType;
        pPort->PortSubtype = PortSubType;

        //
        // Dynamic memory allocation for pP2pCtrll
        //
#if 0
        pPort->pP2pCtrll = (PP2P_CTRL_T) NdisAllocateMemoryWithTagPriority(MiniportAdapterHandle,
                                                                       sizeof(P2P_CTRL_T),
                                                                       NIC_TAG,
                                                                       NormalPoolPriority);

        if (pPort->pP2pCtrll == NULL)
        {
            DBGPRINT_ERR(("Failed to allocate memory - pP2pCtrll\n"));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        PlatformZeroMemory(pPort->pP2pCtrll, sizeof(P2P_CTRL_T));
        G_MemoryAllocate_cnt ++;
        G_MemoryAllocate_Len += sizeof(P2P_CTRL_T);
#endif
    } while (FALSE);
    
    *ppPort = pPort;
 
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6AllocateNdisPort(
     IN  PMP_PORT pPort,
     OUT PNDIS_PORT_NUMBER       AllocatedPortNumber)
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_PORT_CHARACTERISTICS   portChar;
    FUNC_ENTER;
    // Call NDIS to allocate the port
    PlatformZeroMemory(&portChar, sizeof(NDIS_PORT_CHARACTERISTICS));

    MP_ASSIGN_NDIS_OBJECT_HEADER(portChar.Header, NDIS_OBJECT_TYPE_DEFAULT,
        NDIS_PORT_CHARACTERISTICS_REVISION_1, sizeof(NDIS_PORT_CHARACTERISTICS));

    portChar.Flags = NDIS_PORT_CHAR_USE_DEFAULT_AUTH_SETTINGS;
    portChar.Type = NdisPortTypeUndefined;
    portChar.MediaConnectState = MediaConnectStateConnected;
    portChar.XmitLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
    portChar.RcvLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
    portChar.Direction = NET_IF_DIRECTION_SENDRECEIVE;
    portChar.SendControlState = NdisPortControlStateUnknown;
    portChar.RcvControlState = NdisPortControlStateUnknown;
    portChar.SendAuthorizationState = NdisPortControlStateUncontrolled; // Ignored
    portChar.RcvAuthorizationState = NdisPortControlStateUncontrolled; // Ignored

    ndisStatus = NdisMAllocatePort(pPort->AdapterHandle, &portChar);
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Failed to allocate NDIS port. Status = 0x%08x\n",
        ndisStatus));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Associated Port Number %d with allocated port\n", 
            portChar.PortNumber));

        // Return the NDIS port number that has been allocated to this port
        *AllocatedPortNumber = portChar.PortNumber;
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    
    return ndisStatus;
}


NDIS_STATUS
N6PortActivateNdisPort(
    IN  PMP_ADAPTER pAd,
    IN  NDIS_PORT_NUMBER PortNumberToActivate
    )
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_SUCCESS;
    NET_PNP_EVENT_NOTIFICATION  netPnpEventNotification;
    NDIS_PORT                   ndisPort;
    PNDIS_PORT_CHARACTERISTICS  portChar;
    FUNC_ENTER;
    
    PlatformZeroMemory(&netPnpEventNotification, sizeof(NET_PNP_EVENT_NOTIFICATION));
    PlatformZeroMemory(&ndisPort, sizeof(NDIS_PORT));

    MP_ASSIGN_NDIS_OBJECT_HEADER(netPnpEventNotification.Header, NDIS_OBJECT_TYPE_DEFAULT,
                    NET_PNP_EVENT_NOTIFICATION_REVISION_1, sizeof(NET_PNP_EVENT_NOTIFICATION));

    netPnpEventNotification.NetPnPEvent.NetEvent = NetEventPortActivation;

    // Refill the characteristics structure for the port
    portChar = &(ndisPort.PortCharacteristics);
    MP_ASSIGN_NDIS_OBJECT_HEADER(portChar->Header, NDIS_OBJECT_TYPE_DEFAULT,
                    NDIS_PORT_CHARACTERISTICS_REVISION_1, sizeof(NDIS_PORT_CHARACTERISTICS));

    portChar->Flags = NDIS_PORT_CHAR_USE_DEFAULT_AUTH_SETTINGS;
    portChar->Type = NdisPortTypeUndefined;
    portChar->MediaConnectState = MediaConnectStateConnected;
    portChar->XmitLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
    portChar->RcvLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
    portChar->Direction = NET_IF_DIRECTION_SENDRECEIVE;
    portChar->SendControlState = NdisPortControlStateUnknown;
    portChar->RcvControlState = NdisPortControlStateUnknown;
    portChar->SendAuthorizationState = NdisPortControlStateUncontrolled; // Ignored
    portChar->RcvAuthorizationState = NdisPortControlStateUncontrolled; // Ignored
    portChar->PortNumber = PortNumberToActivate;

    // Single port is being activated
    ndisPort.Next = NULL;

    // We need to save a pointer to the NDIS_PORT in the NetPnPEvent::Buffer field
    netPnpEventNotification.NetPnPEvent.Buffer = (PVOID)&ndisPort;
    netPnpEventNotification.NetPnPEvent.BufferLength = sizeof(NDIS_PORT);

    ndisStatus = NdisMNetPnPEvent(pAd->AdapterHandle, &netPnpEventNotification);    
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("N6PortActivateNdisPort::Failed to activate NDIS port %d. Status = 0x%08x\n", PortNumberToActivate, ndisStatus));       
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6PortActivateNdisPort::Activated Port Number [%d] Success\n", PortNumberToActivate));
    }

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

VOID
N6PortDeactivateNdisPort(
    IN  PMP_ADAPTER pAd,
    IN  NDIS_PORT_NUMBER PortNumberToDeactivate
    )
{
    NDIS_STATUS                     ndisStatus = NDIS_STATUS_SUCCESS;
    NET_PNP_EVENT_NOTIFICATION      netPnpEventNotification;
    NDIS_PORT_NUMBER                portNumberArray[1];
    PMP_PORT                      pPort = pAd->PortList[PortNumberToDeactivate];
    FUNC_ENTER;

    PlatformZeroMemory(&netPnpEventNotification, sizeof(NET_PNP_EVENT_NOTIFICATION));
    
    MP_ASSIGN_NDIS_OBJECT_HEADER(netPnpEventNotification.Header, NDIS_OBJECT_TYPE_DEFAULT,
                        NET_PNP_EVENT_NOTIFICATION_REVISION_1, sizeof(NET_PNP_EVENT_NOTIFICATION));

    netPnpEventNotification.NetPnPEvent.NetEvent = NetEventPortDeactivation;

    // We need to save a pointer to the NDIS_PORT_NUMBER in the NetPnPEvent::Buffer field
    portNumberArray[0] = PortNumberToDeactivate;
    netPnpEventNotification.NetPnPEvent.Buffer = (PVOID)portNumberArray;
    netPnpEventNotification.NetPnPEvent.BufferLength = sizeof(NDIS_PORT_NUMBER);

    ndisStatus = NdisMNetPnPEvent(pAd->AdapterHandle, &netPnpEventNotification);
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s:Failed to deactivate NDIS port %d. Status = 0x%08x\n", __FUNCTION__, PortNumberToDeactivate, ndisStatus));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s:Deactivated Port Number [%d] Success  (%x:%x:%x:%x:%x:%x)\n", __FUNCTION__, PortNumberToDeactivate,  
            pPort->CurrentAddress[0], pPort->CurrentAddress[1], pPort->CurrentAddress[2], pPort->CurrentAddress[3], pPort->CurrentAddress[4], pPort->CurrentAddress[5]));

    }

    FUNC_LEAVE;
}

VOID
N6PortDeRegisterActivePort(
    IN  PMP_ADAPTER pAd
    )
{
    UINT i=0;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    
    for (i = 1 ; i < RTMP_MAX_NUMBER_OF_PORT; i++)
    {
        if (pAd->PortList[i])
        {
            if (pAd->PortList[i]->bActive)
            {
                MlmePortFreeWorkItem(pAd, pAd->PortList[i]);
                MlmePortFreeTimer(pAd, pAd->PortList[i]);
                N6PortDeactivateNdisPort(pAd, i);
                ndisStatus = NdisMFreePort(pAd->AdapterHandle, i);
                if(ndisStatus != NDIS_STATUS_SUCCESS)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("NdisMFreePort failed.\n"));
                }
            }
        }
    
    }
}

VOID N6FreePort(
    IN PMP_PORT pPort)
{
    FUNC_ENTER;
    if(pPort->pSupportedPhyTypes)
    {
        PlatformFreeMemory(pPort->pSupportedPhyTypes, pPort->pAd->StaCfg.SupportedPhyTypesLen);
        pPort->pAd->StaCfg.SupportedPhyTypesLen =0; 
        pPort->pSupportedPhyTypes = NULL;
    }
    
    //PlatformFreeMemory(pPort->pP2pCtrll , sizeof(P2P_CTRL_T));
    
    
    PlatformFreeMemory(pPort, sizeof(MP_PORT));


    FUNC_LEAVE;
}

/*
    ========================================================================
    
    Routine Description:
        Initialize Port structure

    Arguments:
        Port        Pointer to our APort or adapter port

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_FAILURE
        NDIS_STATUS_RESOURCES

    IRQL = PASSIVE_LEVEL

    Note:
    
    ========================================================================
*/
NDIS_STATUS N6InitPort(
    IN  PMP_PORT  pPort)
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;   
    PMP_ADAPTER pAd = pPort->pAd;

    FUNC_ENTER;

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pPort couldn't be NULL\n", __FUNCTION__));
        return ndisStatus;
    }


    do
    {                     
        NdisAllocateSpinLock(&pPort->BATabLock);
        NdisAllocateSpinLock(&pPort->BADeleteRECEntry);
        //
        // Allocate the workitem we would use
        //
        pPort->OidWorkItem = NdisAllocateIoWorkItem(pAd->AdapterHandle);
        if (pPort->OidWorkItem == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Failed to allocate deferred OID workitem\n"));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
    } while (FALSE);
    
   FUNC_LEAVE_STATUS(ndisStatus);

    return ndisStatus;
}


/*
    ========================================================================
    
    Routine Description:
        Initialize port configuration structure

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    Note:
        
    ========================================================================
*/
VOID    N6PortUserCfgInit(
    IN  PMP_PORT      pPort)
{
    UINT            i;
    PMP_ADAPTER   pAd = pPort->pAd;

    DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit\n"));    

    //
    //  part I. intialize common configuration
    //

    DBGPRINT(RT_DEBUG_TRACE, ("-->sizeof(FRAME_ADDBA_RSP)=%d,     \n",sizeof(FRAME_ADDBA_RSP)));    

    for(i = 0; i < SHARE_KEY_NUM; i++) {
        pPort->SharedKey[BSS0][i].KeyLen = 0;
        pPort->SharedKey[BSS1][i].KeyLen = 0;
        pPort->SharedKey[BSS2][i].KeyLen = 0;
        pPort->SharedKey[BSS3][i].KeyLen = 0;
        pPort->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
        pPort->SharedKey[BSS1][i].CipherAlg = CIPHER_NONE;
        pPort->SharedKey[BSS2][i].CipherAlg = CIPHER_NONE;
        pPort->SharedKey[BSS3][i].CipherAlg = CIPHER_NONE;
    }

    BATableInit(&pPort->BATable);   
    
    // Init timer for reset complete event  
    RX_FILTER_SET_FLAG(pPort, fRX_FILTER_ACCEPT_DIRECT);
    RX_FILTER_CLEAR_FLAG(pPort, fRX_FILTER_ACCEPT_MULTICAST);
    RX_FILTER_SET_FLAG(pPort, fRX_FILTER_ACCEPT_BROADCAST);
    RX_FILTER_SET_FLAG(pPort, fRX_FILTER_ACCEPT_ALL_MULTICAST);

    pPort->PortCfg.OperationMode = DOT11_OPERATION_MODE_EXTENSIBLE_STATION;
    pPort->PortCfg.AuthMode = Ralink802_11AuthModeOpen;

    pPort->PortCfg.WepStatus = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.OrigWepStatus = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.PairCipher = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.GroupCipher = Ralink802_11EncryptionDisabled;
    pPort->PortCfg.bMixCipher = FALSE;  
    pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;
    pPort->PortCfg.DefaultKeyId = 0;
    pPort->PortCfg.OpStatusFlags = 0;
    MP_SET_STATE(pPort, INIT_STATE);
    pPort->PortCfg.AcceptAnyBSSID = TRUE;

    pPort->CommonCfg.FragmentThreshold = DOT11_MAX_MSDU_SIZE;
    
    // global variables mXXXX used in MAC protocol state machines
    OPSTATUS_SET_FLAG(pPort, fOP_STATUS_RECEIVE_DTIM);
    OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_ADHOC_ON);
    OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_INFRA_ON);    
    OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_SHORT_PREAMBLE_INUSED);  // CCK use LONG preamble 
    
    DBGPRINT(RT_DEBUG_TRACE, ("<-- N6PortUserCfgInit autoba =%x\n",pPort->CommonCfg.BACapability.field.AutoBA));
}




NDIS_STATUS
N6IniMiniportAttributes (
    IN  PMP_ADAPTER pAd
    )
{
    NDIS_STATUS NdisStatus;


    NdisStatus = N6RegistrationAttributes(pAd);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s   Line(%d)    Failed to N6RegistrationAttributes\n", __LINE__, __FUNCTION__));
        return  NdisStatus;
    }

    return  NdisStatus;

}




NDIS_STATUS
N6RegistrationAttributes (
    IN  PMP_ADAPTER pAd
    )
{
    NDIS_MINIPORT_ADAPTER_ATTRIBUTES        MiniportAttributes;
    NDIS_STATUS NdisStatus;
     //
    // Set the miniport registration attributes with NDIS
    //
    PlatformZeroMemory(&MiniportAttributes, sizeof(MiniportAttributes));

    MiniportAttributes.RegistrationAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if(NDIS_WIN8_ABOVE(pAd))
        {
            MiniportAttributes.RegistrationAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_2;
            MiniportAttributes.RegistrationAttributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_2;
        }
        else
#endif
        {
            MiniportAttributes.RegistrationAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_1;
            MiniportAttributes.RegistrationAttributes.Header.Size =  NDIS_SIZEOF_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_1;
        }

        MiniportAttributes.RegistrationAttributes.MiniportAdapterContext = pAd;
        MiniportAttributes.RegistrationAttributes.CheckForHangTimeInSeconds = 6;
    

#if IS_BUS_TYPE_PCI(DEV_BUS_TYPE)

    MiniportAttributes.RegistrationAttributes.AttributeFlags = NDIS_MINIPORT_ATTRIBUTES_BUS_MASTER;
    
#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
    if(NDIS_WIN8_ABOVE(pAd)))
    {
            MiniportAttributes.RegistrationAttributes.AttributeFlags |= NDIS_MINIPORT_ATTRIBUTES_NO_PAUSE_ON_SUSPEND;
    }
#endif

   MiniportAttributes.RegistrationAttributes.InterfaceType = NdisInterfacePci;    

#elif IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    //
    // Read Safly Remove control flag.
    // Ture: Means disable Safly Remove Icon on systemtray.
    //
    if (N6UsbReadSafelyRemoveFlag(pAd))
    {
        MiniportAttributes.RegistrationAttributes.AttributeFlags = NDIS_MINIPORT_ATTRIBUTES_SURPRISE_REMOVE_OK;
    }
    else
    {
        MiniportAttributes.RegistrationAttributes.AttributeFlags = 0;
    }

    MiniportAttributes.RegistrationAttributes.AttributeFlags |= NDIS_MINIPORT_ATTRIBUTES_NDIS_WDM;
    
#if (COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
    if(NDIS_WIN8_ABOVE(pAd))
    {
           MiniportAttributes.RegistrationAttributes.AttributeFlags |= NDIS_MINIPORT_ATTRIBUTES_NO_PAUSE_ON_SUSPEND;
    }
#endif

    MiniportAttributes.RegistrationAttributes.InterfaceType = NdisInterfaceUSB;    

#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)
#endif

    NdisStatus = NdisMSetMiniportAttributes(
                            pAd->AdapterHandle,
                            &MiniportAttributes
                            );
                            
    return NdisStatus;
}





NDIS_STATUS
N6Set80211Attributes(
    IN  PMP_ADAPTER     pAd
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADAPTER_ATTRIBUTES        MiniportAttributes;
    ULONG                   ndisVersion;

    do
    {
         PlatformZeroMemory(&MiniportAttributes, sizeof(MiniportAttributes));

        MiniportAttributes.Native_802_11_Attributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES;

        ndisVersion = NdisGetVersion();
        if (ndisVersion <= 0x00060001)
        {
            MiniportAttributes.Native_802_11_Attributes.Header.Revision = NDIS_MINIPORT_ADAPTER_802_11_ATTRIBUTES_REVISION_1;
            MiniportAttributes.Native_802_11_Attributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES_REVISION_1;
        }
        else
        {
            MiniportAttributes.Native_802_11_Attributes.Header.Revision = NDIS_MINIPORT_ADAPTER_802_11_ATTRIBUTES_REVISION_2;
            MiniportAttributes.Native_802_11_Attributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES_REVISION_2;
        }

        if (ndisVersion <= 0x00060001)
        {
            MiniportAttributes.Native_802_11_Attributes.OpModeCapability = (DOT11_OPERATION_MODE_EXTENSIBLE_STATION |
                         DOT11_OPERATION_MODE_NETWORK_MONITOR);   
        }
        else
        {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if(NDIS_WIN8_ABOVE(pAd))
            {
                DBGPRINT(RT_DEBUG_TRACE,("[w8] ndis6.3\n"));
                MiniportAttributes.Native_802_11_Attributes.Header.Revision = NDIS_MINIPORT_ADAPTER_802_11_ATTRIBUTES_REVISION_3;
                MiniportAttributes.Native_802_11_Attributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES_REVISION_3;
                MiniportAttributes.Native_802_11_Attributes.OpModeCapability = (DOT11_OPERATION_MODE_EXTENSIBLE_STATION |
                             DOT11_OPERATION_MODE_NETWORK_MONITOR |
                             DOT11_OPERATION_MODE_EXTENSIBLE_AP |
                             DOT11_OPERATION_MODE_WFD_DEVICE |
                             DOT11_OPERATION_MODE_WFD_GROUP_OWNER |
                             DOT11_OPERATION_MODE_WFD_CLIENT);
            }
            else
#endif
            {
                MiniportAttributes.Native_802_11_Attributes.Header.Revision = NDIS_MINIPORT_ADAPTER_802_11_ATTRIBUTES_REVISION_2;
                MiniportAttributes.Native_802_11_Attributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES_REVISION_2;
                MiniportAttributes.Native_802_11_Attributes.OpModeCapability = (DOT11_OPERATION_MODE_EXTENSIBLE_STATION |
                             DOT11_OPERATION_MODE_NETWORK_MONITOR |
                             DOT11_OPERATION_MODE_EXTENSIBLE_AP);

            }
        }

        //
        // Call Hardware and Station lay to fill the attribute structure.
        //
        ndisStatus = N6Hw11Fill80211Attributes(pAd, &(MiniportAttributes.Native_802_11_Attributes));
        if (ndisStatus != NDIS_STATUS_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Hw11Fill80211Attributes [Status=0x%08x]\n", ndisStatus));
            break;
        }

        ndisStatus = N6Sta11Fill80211Attributes(pAd, &(MiniportAttributes.Native_802_11_Attributes));
        if (ndisStatus != NDIS_STATUS_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Sta11Fill80211Attributes[Status=0x%08x]\n", ndisStatus));
            break;
        }

        //
        // Fill Virtual WiFi ability
        //
        ndisStatus = N6Vw11Fill80211Attributes(pAd, &(MiniportAttributes.Native_802_11_Attributes));
        if (ndisStatus != NDIS_STATUS_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Vm11Fill80211Attributes[Status=0x%08x]\n", ndisStatus));
            break;
        }


        //
        // Fill AP attr
        //
        ndisStatus = N6Ap11Fill80211Attributes(pAd, &(MiniportAttributes.Native_802_11_Attributes));
        if (ndisStatus != NDIS_STATUS_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Ap11Fill80211Attributes[Status=0x%08x]\n", ndisStatus));
            break;
        }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if(NDIS_WIN8_ABOVE(pAd))
        {
            //
            // Fill WFD attr
            //
            ndisStatus = N6WfdFill80211Attributes(pAd, &(MiniportAttributes.Native_802_11_Attributes));
            if (ndisStatus != NDIS_STATUS_SUCCESS)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("N6WfdFill80211Attributes[Status=0x%08x]\n", ndisStatus));
                break;
            }
        }
#endif

        //
        // Register the 802.11 miniport attributes with NDIS
        //
        ndisStatus = NdisMSetMiniportAttributes(
                            pAd->AdapterHandle,
                            &MiniportAttributes
                    );      
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("NdisMSetMiniportAttributes, Register the 802.11 miniport attributes failed [Status=0x%08x]\n", ndisStatus));
            break;
        }

    } while (FALSE);

    //
    // Free the attribute structure memory
    //
    if (MiniportAttributes.Native_802_11_Attributes.SupportedPhyAttributes)
    {
        PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.SupportedPhyAttributes, 
        pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries * sizeof(DOT11_PHY_ATTRIBUTES) );
    }

    if (MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes)
    {
        if (MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pSupportedCountryOrRegionStrings)
        {
            PlatformFreeMemory(&MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pSupportedCountryOrRegionStrings, 0);
        }
        if (MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pInfraSupportedUcastAlgoPairs)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pInfraSupportedUcastAlgoPairs, 
            uBSSUnicastAuthCipherPairsSize);
        }

        if (MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pInfraSupportedMcastAlgoPairs)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pInfraSupportedMcastAlgoPairs, 
            uBSSMulticastAuthCipherPairsSize);
        }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if (NDIS_WIN8_ABOVE(pAd) && MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pInfraSupportedMcastMgmtAlgoPairs)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pInfraSupportedMcastMgmtAlgoPairs, 
            uBSSMulticastMgmtAuthCipherPairsSize);
        }
#endif

        if (MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pAdhocSupportedUcastAlgoPairs)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pAdhocSupportedUcastAlgoPairs, 
            uIBSSUnicastAuthCipherPairsSize);
        }

        if (MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pAdhocSupportedMcastAlgoPairs)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes->pAdhocSupportedMcastAlgoPairs, 
            uIBSSMulticastAuthCipherPairsSize);
        }

        PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtSTAAttributes, 
        sizeof(DOT11_EXTSTA_ATTRIBUTES));
    }

    if (MiniportAttributes.Native_802_11_Attributes.VWiFiAttributes)
    {
        LONG freelen=0;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        freelen = sizeof(DOT11_VWIFI_ATTRIBUTES) + NUM_SUPPORTED_VWIFI_COMBINATIONS * DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_3;
#else
        freelen = sizeof(DOT11_VWIFI_ATTRIBUTES) + NUM_SUPPORTED_VWIFI_COMBINATIONS * DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_2;
#endif
        PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.VWiFiAttributes, freelen);
    }

    if (MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes)
    {
        if (MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes->pSupportedCountryOrRegionStrings)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes->pSupportedCountryOrRegionStrings, 0);
        }
        if (MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes->pInfraSupportedUcastAlgoPairs)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes->pInfraSupportedUcastAlgoPairs, 
            uExtAPMulticastAuthCipherPairsSize);
        }
        if (MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes->pInfraSupportedMcastAlgoPairs)
        {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes->pInfraSupportedMcastAlgoPairs, 
            uExtAPUnicastAuthCipherPairsSize);
        }

        PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.ExtAPAttributes, sizeof(DOT11_EXTAP_ATTRIBUTES));
    }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if(NDIS_WIN8_ABOVE(pAd))
    {
        if (MiniportAttributes.Native_802_11_Attributes.WFDAttributes)
        {
            if (MiniportAttributes.Native_802_11_Attributes.WFDAttributes->pSupportedCountryOrRegionStrings)
            {
            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.WFDAttributes->pSupportedCountryOrRegionStrings, 0);
            }

            PlatformFreeMemory(MiniportAttributes.Native_802_11_Attributes.WFDAttributes, 
            sizeof(DOT11_WFD_ATTRIBUTES));
        }
    }
#endif

    return ndisStatus;
}


NDIS_STATUS
N6SetGeneralAttributes(
    IN  PMP_ADAPTER     pAd
    )
{
    NDIS_MINIPORT_ADAPTER_ATTRIBUTES        MiniportAttributes;
    NDIS_PM_CAPABILITIES                    PmCapabilities;
    NDIS_STATUS                             ndisStatus;

    PlatformZeroMemory(&MiniportAttributes, sizeof(MiniportAttributes));
    MiniportAttributes.GeneralAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES;
    MiniportAttributes.GeneralAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    MiniportAttributes.GeneralAttributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;

    MiniportAttributes.GeneralAttributes.MediaType = NdisMediumNative802_11;
    MiniportAttributes.GeneralAttributes.PhysicalMediumType = NdisPhysicalMediumNative802_11;    


    MiniportAttributes.GeneralAttributes.MtuSize = MP_802_11_MAX_FRAME_SIZE - MP_802_11_SHORT_HEADER_SIZE;
    MiniportAttributes.GeneralAttributes.MaxXmitLinkSpeed = HW_LINK_SPEED;
    MiniportAttributes.GeneralAttributes.MaxRcvLinkSpeed = HW_LINK_SPEED;
    MiniportAttributes.GeneralAttributes.XmitLinkSpeed = HW_LINK_SPEED;
    MiniportAttributes.GeneralAttributes.RcvLinkSpeed = HW_LINK_SPEED;
    MiniportAttributes.GeneralAttributes.MediaConnectState = MediaConnectStateConnected;
    MiniportAttributes.GeneralAttributes.MediaDuplexState = MediaDuplexStateFull;
    MiniportAttributes.GeneralAttributes.LookaheadSize = MP_802_11_MAXIMUM_LOOKAHEAD;
    
    PlatformZeroMemory(&PmCapabilities, sizeof(NDIS_PM_CAPABILITIES));
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if(NDIS_WIN8_ABOVE(pAd))
    {
        // Ndis 6.3 needs the data structure PowerManagementCapabilitiesEx
        PmCapabilities.Header.Type      = NDIS_OBJECT_TYPE_DEFAULT;
        PmCapabilities.Header.Revision  = NDIS_PM_CAPABILITIES_REVISION_2;
        PmCapabilities.Header.Size      = NDIS_SIZEOF_NDIS_PM_CAPABILITIES_REVISION_2;
        PmCapabilities.Flags            = 0;//NDIS_PM_SELECTIVE_SUSPEND_SUPPORTED;
        MiniportAttributes.GeneralAttributes.PowerManagementCapabilitiesEx = &PmCapabilities;
    }
    else if(NDIS_WIN7(pAd))
#endif
    {
        // Ndis 6.2 needs the data structure PowerManagementCapabilitiesEx
        PmCapabilities.Header.Type      = NDIS_OBJECT_TYPE_DEFAULT;
        PmCapabilities.Header.Revision  = NDIS_PM_CAPABILITIES_REVISION_1;
        PmCapabilities.Header.Size      = NDIS_SIZEOF_NDIS_PM_CAPABILITIES_REVISION_1;
        MiniportAttributes.GeneralAttributes.PowerManagementCapabilitiesEx = &PmCapabilities;
    }

    MiniportAttributes.GeneralAttributes.MacOptions = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                                         NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                                         NDIS_MAC_OPTION_NO_LOOPBACK;

    MiniportAttributes.GeneralAttributes.SupportedPacketFilters = NDIS_PACKET_TYPE_DIRECTED |
                                                     NDIS_PACKET_TYPE_MULTICAST |
                                                     NDIS_PACKET_TYPE_ALL_MULTICAST |
                                                     NDIS_PACKET_TYPE_BROADCAST;

    MiniportAttributes.GeneralAttributes.MaxMulticastListSize = HW_MAX_MCAST_LIST_SIZE;
    MiniportAttributes.GeneralAttributes.MacAddressLength = ETH_LENGTH_OF_ADDRESS;

    PlatformMoveMemory(
        &MiniportAttributes.GeneralAttributes.PermanentMacAddress,
        pAd->HwCfg.PermanentAddress,
        ETH_LENGTH_OF_ADDRESS
        );

    PlatformMoveMemory(
        &MiniportAttributes.GeneralAttributes.CurrentMacAddress,
        pAd->HwCfg.CurrentAddress,
        ETH_LENGTH_OF_ADDRESS
        );

    MiniportAttributes.GeneralAttributes.RecvScaleCapabilities = NULL;
    MiniportAttributes.GeneralAttributes.AccessType = NET_IF_ACCESS_BROADCAST;
    MiniportAttributes.GeneralAttributes.DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    MiniportAttributes.GeneralAttributes.IfType = IF_TYPE_IEEE80211;
    MiniportAttributes.GeneralAttributes.IfConnectorPresent = TRUE;

    MiniportAttributes.GeneralAttributes.DataBackFillSize = 0;

    MiniportAttributes.GeneralAttributes.SupportedOidList = NICSupportedOids;
    MiniportAttributes.GeneralAttributes.SupportedOidListLength = NIC_SUPPORT_OID_SIZE;

    //
    // SupportedOidListLength can't be 0, otherwise nwfi pro driver will crash.
    //
    DBGPRINT(RT_DEBUG_TRACE, ("N6SetGeneralAttributes::SupportedOidListLength = %d\n", MiniportAttributes.GeneralAttributes.SupportedOidListLength));
    ASSERT(MiniportAttributes.GeneralAttributes.SupportedOidListLength > 0);
    //
    // Register the Generic miniport attributes with NDIS
    //
    ndisStatus = NdisMSetMiniportAttributes(
                                pAd->AdapterHandle,
                                &MiniportAttributes
                                );

    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("NdisMSetMiniportAttributes failed %x\n", ndisStatus));
    }

    return ndisStatus;
}


NDIS_STATUS
N6Vw11Fill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    )
{
    NDIS_STATUS             ndisStatus = NDIS_STATUS_SUCCESS;
    ULONG                   vwifiAttrSize = 0;
    PDOT11_VWIFI_ATTRIBUTES VWiFiAttribs = NULL;

    do
    {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if(NDIS_WIN8_ABOVE(pAd))
        {
            PDOT11_VWIFI_COMBINATION_V3 pDot11VWiFiCombination = NULL;

            vwifiAttrSize = sizeof(DOT11_VWIFI_ATTRIBUTES) + NUM_SUPPORTED_VWIFI_COMBINATIONS * DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_3;

            PlatformAllocateMemory(
                    pAd,
                    &attr->VWiFiAttributes,
                    vwifiAttrSize);

            if (attr->VWiFiAttributes == NULL)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("Failed to allocate %d bytes for VWiFi capability.\n", vwifiAttrSize));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            VWiFiAttribs = attr->VWiFiAttributes;

            PlatformZeroMemory(VWiFiAttribs, vwifiAttrSize);

            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        VWiFiAttribs->Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_ATTRIBUTES_REVISION_1,
                        sizeof(DOT11_VWIFI_ATTRIBUTES));

            VWiFiAttribs->uTotalNumOfEntries = NUM_SUPPORTED_VWIFI_COMBINATIONS;

            pDot11VWiFiCombination = (PDOT11_VWIFI_COMBINATION_V3)&VWiFiAttribs->Combinations[0];

            // 1. support for Infra Only
            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        pDot11VWiFiCombination[0].Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_COMBINATION_REVISION_3,
                        DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_3);

            pDot11VWiFiCombination[0].uNumInfrastructure = 1;

            // 2. support for Infra-SoftAP
            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        pDot11VWiFiCombination[1].Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_COMBINATION_REVISION_3,
                        DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_3);

            pDot11VWiFiCombination[1].uNumInfrastructure = 1;
            pDot11VWiFiCombination[1].uNumSoftAP = 1;

            // 3. Win8, Support infra/softap/WFD/virSta
            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        pDot11VWiFiCombination[2].Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_COMBINATION_REVISION_3,
                        DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_3);

            pDot11VWiFiCombination[2].uNumInfrastructure = 1;
            pDot11VWiFiCombination[2].uNumSoftAP = 1;
#ifdef WIN8_RALINK_WFD_SUPPORT          
            // in WHCK NDIS Test After NDIS version 614, not allow uNumVirtualStation = 1
            if(pAd->LogoTestCfg.OnTestingWHQL == FALSE)
                pDot11VWiFiCombination[2].uNumVirtualStation = 1;
#endif /* WIN8_RALINK_WFD_SUPPORT */            
            pDot11VWiFiCombination[2].uNumWFDGroup = NUM_OF_CONCURRENT_GO_ROLE + NUM_OF_CONCURRENT_CLIENT_ROLE;

        }
        else
#endif
        {
            PDOT11_VWIFI_COMBINATION_V2 pDot11VWiFiCombination = NULL;
        
            vwifiAttrSize = sizeof(DOT11_VWIFI_ATTRIBUTES) + NUM_SUPPORTED_VWIFI_COMBINATIONS * DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_2;

            PlatformAllocateMemory(
                    pAd,
                    &attr->VWiFiAttributes,
                    vwifiAttrSize);

            if (attr->VWiFiAttributes == NULL)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("Failed to allocate %d bytes for VWiFi capability.\n", vwifiAttrSize));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }

            VWiFiAttribs = attr->VWiFiAttributes;

            PlatformZeroMemory(VWiFiAttribs, vwifiAttrSize);

            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        VWiFiAttribs->Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_ATTRIBUTES_REVISION_1,
                        sizeof(DOT11_VWIFI_ATTRIBUTES));

            VWiFiAttribs->uTotalNumOfEntries = NUM_SUPPORTED_VWIFI_COMBINATIONS;
            
            pDot11VWiFiCombination = (PDOT11_VWIFI_COMBINATION_V2)&VWiFiAttribs->Combinations[0];

            // support for Infra only
            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        pDot11VWiFiCombination[0].Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_COMBINATION_REVISION_2,
                        DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_2);

            pDot11VWiFiCombination[0].uNumInfrastructure = 1;

            // support for Infra-SoftAP
            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        pDot11VWiFiCombination[1].Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_COMBINATION_REVISION_2,
                        DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_2);

            pDot11VWiFiCombination[1].uNumInfrastructure = 1;
            pDot11VWiFiCombination[1].uNumSoftAP = 1;

            //Support infra/softap/virtual station
            MP_ASSIGN_NDIS_OBJECT_HEADER(
                        pDot11VWiFiCombination[2].Header,
                        NDIS_OBJECT_TYPE_DEFAULT,
                        DOT11_VWIFI_COMBINATION_REVISION_2,
                        DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_2);

            // support for virSta
            pDot11VWiFiCombination[2].uNumInfrastructure = 1;
            pDot11VWiFiCombination[2].uNumSoftAP = 1;
            pDot11VWiFiCombination[2].uNumVirtualStation = 1;
        }
    } while (FALSE);

    return ndisStatus;
}

NDIS_STATUS
N6Ap11Fill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    )
{
    DOT11_EXTAP_ATTRIBUTES  ExtAPCap;
    NDIS_STATUS             ndisStatus;

    PlatformAllocateMemory(
            pAd,
            &attr->ExtAPAttributes,
            sizeof(DOT11_EXTAP_ATTRIBUTES));

    if (attr->ExtAPAttributes == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Failed to allocate %d bytes for ExtAP capability.\n", sizeof(DOT11_EXTAP_ATTRIBUTES)));
        return NDIS_STATUS_RESOURCES;
    }

    PlatformZeroMemory(attr->ExtAPAttributes, sizeof(DOT11_EXTAP_ATTRIBUTES));

    ndisStatus = N6Sta11QueryExtAPCapability(&ExtAPCap);
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        return ndisStatus;
    }

    do
    {
        MP_ASSIGN_NDIS_OBJECT_HEADER(attr->ExtAPAttributes->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_EXTAP_ATTRIBUTES_REVISION_1,
                                    sizeof(DOT11_EXTAP_ATTRIBUTES));

        //
        // Use default values
        //
        attr->ExtAPAttributes->uScanSSIDListSize = ExtAPCap.uScanSSIDListSize;
        attr->ExtAPAttributes->uDesiredSSIDListSize = ExtAPCap.uDesiredSSIDListSize;

        attr->ExtAPAttributes->uPrivacyExemptionListSize = ExtAPCap.uPrivacyExemptionListSize;
        attr->ExtAPAttributes->uAssociationTableSize = ExtAPCap.uAssociationTableSize;
        attr->ExtAPAttributes->uDefaultKeyTableSize = ExtAPCap.uDefaultKeyTableSize;
        attr->ExtAPAttributes->uWEPKeyValueMaxLength = ExtAPCap.uWEPKeyValueMaxLength;
        attr->ExtAPAttributes->bStrictlyOrderedServiceClassImplemented = FALSE;

        //
        // 11d stuff.
        //
        attr->ExtAPAttributes->uNumSupportedCountryOrRegionStrings = 0;
        attr->ExtAPAttributes->pSupportedCountryOrRegionStrings = NULL;

        //
        // Get unicast algorithm pair list for ExtAP
        //
        PlatformAllocateMemory(pAd,
                        &attr->ExtAPAttributes->pInfraSupportedUcastAlgoPairs,
                        uExtAPUnicastAuthCipherPairsSize);
        if (attr->ExtAPAttributes->pInfraSupportedUcastAlgoPairs == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Ap11Fill80211Attributes, allocate memory failed for pInfraSupportedUcastAlgoPairs(size=%d)\n", uExtAPUnicastAuthCipherPairsSize));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        PlatformMoveMemory(attr->ExtAPAttributes->pInfraSupportedUcastAlgoPairs, ExtAPUnicastAuthCipherPairs, uExtAPUnicastAuthCipherPairsSize);
        attr->ExtAPAttributes->uInfraNumSupportedUcastAlgoPairs = uExtAPUnicastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);

        //
        // Get multicast algorithm pair list for ExtAP same as unicast algorithm pair lis
        //
        PlatformAllocateMemory(pAd,
                        &attr->ExtAPAttributes->pInfraSupportedMcastAlgoPairs,
                        uExtAPMulticastAuthCipherPairsSize);
        if (attr->ExtAPAttributes->pInfraSupportedMcastAlgoPairs == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Ap11Fill80211Attributes, allocate memory failed for pInfraSupportedMcastAlgoPairs(size=%d)\n", uExtAPMulticastAuthCipherPairsSize));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        PlatformMoveMemory(attr->ExtAPAttributes->pInfraSupportedMcastAlgoPairs, ExtAPMulticastAuthCipherPairs, uExtAPMulticastAuthCipherPairsSize);
        attr->ExtAPAttributes->uInfraNumSupportedMcastAlgoPairs = uExtAPMulticastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);
        
    } while (FALSE);

    return ndisStatus;
}

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  

NDIS_STATUS
N6WfdFill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    )
{
    ULONG Index = 0;
    ULONG InterfaceAddressListCount = 0;
    
    DBGPRINT(RT_DEBUG_TRACE,("[w8] N6WfdFill80211Attributes-->\n"));
    PlatformAllocateMemory(
            pAd,
            &attr->WFDAttributes,
            sizeof(DOT11_WFD_ATTRIBUTES));

    if (attr->WFDAttributes == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Failed to allocate %u bytes for WFD attr.\n", sizeof(DOT11_WFD_ATTRIBUTES)));
        return NDIS_STATUS_RESOURCES;
    }

    PlatformZeroMemory(attr->WFDAttributes, sizeof(DOT11_WFD_ATTRIBUTES));
    MP_ASSIGN_NDIS_OBJECT_HEADER(
                attr->WFDAttributes->Header,
                NDIS_OBJECT_TYPE_DEFAULT,
                DOT11_WFD_ATTRIBUTES_REVISION_1,
                sizeof(DOT11_WFD_ATTRIBUTES));

    attr->WFDAttributes->uNumConcurrentGORole = NUM_OF_CONCURRENT_GO_ROLE;
    attr->WFDAttributes->uNumConcurrentClientRole = NUM_OF_CONCURRENT_CLIENT_ROLE;
    attr->WFDAttributes->WPSVersionsSupported = DOT11_WPS_VERSION_2_0;
    InterfaceAddressListCount = MAX_NUM_OF_MS_WFD_ROLE; // = NUM_OF_CONCURRENT_GO_ROLE + NUM_OF_CONCURRENT_CLIENT_ROLE

    // Device Capabilities
    attr->WFDAttributes->bServiceDiscoverySupported = TRUE;
    attr->WFDAttributes->bClientDiscoverabilitySupported = TRUE;
    attr->WFDAttributes->bInfrastructureManagementSupported = FALSE;

    // null Secondary Device Type List
    attr->WFDAttributes->uMaxSecondaryDeviceTypeListSize = 0;

    // device address
    Ndis6CommonGetNewMacAddress(pAd, MULTI_BSSID_MODE, MAC_ADDR_INDEX_2, attr->WFDAttributes->DeviceAddress);

    DBGPRINT(RT_DEBUG_TRACE,("[%s] Mac address for WFD role (the last maclist is for virtual station port):\n", __FUNCTION__));         
    for (Index = 0; Index < (InterfaceAddressListCount + 1); Index++)
    {   // the last mac is for the virtual station port
        Ndis6CommonGetNewMacAddress(pAd, MULTI_BSSID_MODE, Index + MAC_ADDR_INDEX_3, pAd->MacAddressList[Index]);
        DBGPRINT(RT_DEBUG_TRACE,("%02x %02x %02x %02x %02x %02x \n",
            pAd->MacAddressList[Index][0],
            pAd->MacAddressList[Index][1],
            pAd->MacAddressList[Index][2],
            pAd->MacAddressList[Index][3],
            pAd->MacAddressList[Index][4],
            pAd->MacAddressList[Index][5]
            ));         
    }
    attr->WFDAttributes->uInterfaceAddressListCount = InterfaceAddressListCount;
    attr->WFDAttributes->pInterfaceAddressList = pAd->MacAddressList;
    
    // country region
    attr->WFDAttributes->uNumSupportedCountryOrRegionStrings = 0;
    attr->WFDAttributes->pSupportedCountryOrRegionStrings = NULL;

    // Device Functionality
    //attr->WFDAttributes->uDiscoveryDeviceIDListSize = MAX_P2P_DISCOVERY_SIZE;
    attr->WFDAttributes->uDiscoveryFilterListSize = MAX_P2P_DISCOVERY_FILTER_NUM;

    // GO Capabilities
    attr->WFDAttributes->uGORoleClientTableSize = MAX_P2P_GO_TABLE_SIZE;

    DBGPRINT(RT_DEBUG_TRACE,("[w8] N6WfdFill80211Attributes ======<<<\n"));
/*
    attr->bAutoPowerSaveMode = FALSE;
    attr->uMaxNetworkOffloadListSize = FALSE;
*/
    return NDIS_STATUS_SUCCESS;
}

#if _WIN8_USB_SS_SUPPORTED
MINIPORT_IDLE_NOTIFICATION N6IdleNotificationHandler;
NDIS_STATUS N6IdleNotificationHandler(
    NDIS_HANDLE MiniportAdapterContext,
    BOOLEAN ForceIdle)
{
    NDIS_STATUS Status = NDIS_STATUS_BUSY;
    PMP_ADAPTER pAd = (PMP_ADAPTER)MiniportAdapterContext;
    PMP_PORT pPort = pAd->PortList[PORT_0];

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]N6IdleNotificationHandler\n"));

    //if it can be suspend...
    do
    {
        //
        // If station port is connected, return busy.
        //
        if (!IDLE_ON(pPort))
        {
            break;
        }

        //
        // If softap is started, return busy.
        //
        if (pPort->SoftAP.bAPStart)
        {
            break;
        }

        //
        // If WFD connected, return busy (todo).
        //

        //
        // Going to be suspended.
        //
        UsbSsIdleRequest(pAd);
        Status = NDIS_STATUS_PENDING;

    } while (0);

    return Status;
}

VOID N6CancelIdleNotificationHandler(
    NDIS_HANDLE MiniportAdapterContext)
{
    PMP_ADAPTER pAd = (PMP_ADAPTER)MiniportAdapterContext;

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]N6CancelIdleNotificationHandler\n"));

    UsbSsCancelIdleRequest(pAd);
}

NDIS_STATUS N6RegisterSelectiveSuspend(
    NDIS_HANDLE NdisMiniportDriverHandle)
{
    NDIS_MINIPORT_SS_CHARACTERISTICS MpSs;

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]N6RegisterSelectiveSuspend\n"));

    PlatformZeroMemory(&MpSs, sizeof(NDIS_MINIPORT_SS_CHARACTERISTICS));

    MpSs.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_SS_CHARACTERISTICS;
    MpSs.Header.Revision = NDIS_MINIPORT_SS_CHARACTERISTICS_REVISION_1;
    MpSs.Header.Size = NDIS_SIZEOF_MINIPORT_SS_CHARACTERISTICS_REVISION_1;

    MpSs.IdleNotificationHandler = N6IdleNotificationHandler;
    MpSs.CancelIdleNotificationHandler = N6CancelIdleNotificationHandler;

    return NdisSetOptionalHandlers(NdisMiniportDriverHandle, (PNDIS_DRIVER_OPTIONAL_HANDLERS)&MpSs);
}
#endif //_WIN8_USB_SS_SUPPORTED

#endif //NDIS630_MINIPORT

NDIS_STATUS
N6Sta11Fill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    )
{
    DOT11_EXTSTA_CAPABILITY ExtStaCap;
     NDIS_STATUS            ndisStatus = NDIS_STATUS_SUCCESS;

    PlatformAllocateMemory(
        pAd,
            &attr->ExtSTAAttributes,
            sizeof(DOT11_EXTSTA_ATTRIBUTES));

    if (attr->ExtSTAAttributes == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }

    PlatformZeroMemory(attr->ExtSTAAttributes, sizeof(DOT11_EXTSTA_ATTRIBUTES));

    //
    // First part of the attribute is the same as the capability. Get it
    // from N6Sta11QueryExtStaCapability.
    //

    ndisStatus = N6Sta11QueryExtStaCapability(&ExtStaCap);
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        return ndisStatus;
    }

    do
    {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))   //for win8 new define revision 3
        if(NDIS_WIN8_ABOVE(pAd))
        {
                MP_ASSIGN_NDIS_OBJECT_HEADER(attr->ExtSTAAttributes->Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_EXTSTA_ATTRIBUTES_REVISION_3,
                                sizeof(DOT11_EXTSTA_ATTRIBUTES));
        }
        else
#endif
        {
                MP_ASSIGN_NDIS_OBJECT_HEADER(attr->ExtSTAAttributes->Header,
                                             NDIS_OBJECT_TYPE_DEFAULT,
                                             DOT11_EXTSTA_ATTRIBUTES_REVISION_1,
                                             sizeof(DOT11_EXTSTA_ATTRIBUTES));
        }

        attr->ExtSTAAttributes->uScanSSIDListSize = ExtStaCap.uScanSSIDListSize;
        attr->ExtSTAAttributes->uDesiredBSSIDListSize = ExtStaCap.uDesiredBSSIDListSize;
        attr->ExtSTAAttributes->uDesiredSSIDListSize = ExtStaCap.uDesiredSSIDListSize;
        attr->ExtSTAAttributes->uExcludedMacAddressListSize = ExtStaCap.uExcludedMacAddressListSize;
        attr->ExtSTAAttributes->uPrivacyExemptionListSize = ExtStaCap.uPrivacyExemptionListSize;
        attr->ExtSTAAttributes->uKeyMappingTableSize = ExtStaCap.uKeyMappingTableSize;
        attr->ExtSTAAttributes->uDefaultKeyTableSize = ExtStaCap.uDefaultKeyTableSize;
        attr->ExtSTAAttributes->uWEPKeyValueMaxLength = ExtStaCap.uWEPKeyValueMaxLength;
        attr->ExtSTAAttributes->uPMKIDCacheSize = ExtStaCap.uPMKIDCacheSize;
        attr->ExtSTAAttributes->uMaxNumPerSTADefaultKeyTables = ExtStaCap.uMaxNumPerSTADefaultKeyTables;
        attr->ExtSTAAttributes->bStrictlyOrderedServiceClassImplemented = FALSE;

        //
        // Qos protocol support
        //
        attr->ExtSTAAttributes->ucSupportedQoSProtocolFlags = DOT11_QOS_PROTOCOL_FLAG_WMM;

        //
        // Safe mode enabled
        //
        attr->ExtSTAAttributes->bSafeModeImplemented = TRUE;

        //
        // 11d stuff.
        //
        attr->ExtSTAAttributes->uNumSupportedCountryOrRegionStrings = 0;
        attr->ExtSTAAttributes->pSupportedCountryOrRegionStrings = NULL;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if(NDIS_WIN8_ABOVE(pAd))
        {
            //
            // PMF
            //
            attr->ExtSTAAttributes->bMFPCapable = pAd->StaCfg.PmfCfg.PmfControl.MFP_Enable;
            DBGPRINT(RT_DEBUG_TRACE, ("MFP: bMFPCapable is %s\n", attr->ExtSTAAttributes->bMFPCapable?"Enabled":"Disabled"));
            //attr->ExtSTAAttributes->bMFPCapable = TRUE;


            //
            // Get multicast algorithm pair list for infrastructure
            //
            PlatformAllocateMemory(pAd,
                            &attr->ExtSTAAttributes->pInfraSupportedMcastMgmtAlgoPairs,
                            uBSSMulticastMgmtAuthCipherPairsSize);
            if (attr->ExtSTAAttributes->pInfraSupportedMcastMgmtAlgoPairs == NULL)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("N6Sta11Fill80211Attributes, allocate memory failed for pInfraSupportedMcastMgmtAlgoPairs(size=%d)\n", uBSSMulticastMgmtAuthCipherPairsSize));
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
            PlatformMoveMemory(attr->ExtSTAAttributes->pInfraSupportedMcastMgmtAlgoPairs, BSSMulticastMgmtAuthCipherPairs, uBSSMulticastMgmtAuthCipherPairsSize);
            attr->ExtSTAAttributes->uInfraNumSupportedMcastMgmtAlgoPairs = uBSSMulticastMgmtAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);
        }
#endif

        //
        // Get unicast algorithm pair list for infrastructure
        //
        PlatformAllocateMemory(pAd,
                            &attr->ExtSTAAttributes->pInfraSupportedUcastAlgoPairs,
                            uBSSUnicastAuthCipherPairsSize);
        if (attr->ExtSTAAttributes->pInfraSupportedUcastAlgoPairs == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Sta11Fill80211Attributes, allocate memory failed for pInfraSupportedUcastAlgoPairs(size=%d)\n", uBSSUnicastAuthCipherPairsSize));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        PlatformMoveMemory(attr->ExtSTAAttributes->pInfraSupportedUcastAlgoPairs, BSSUnicastAuthCipherPairs, uBSSUnicastAuthCipherPairsSize);
        attr->ExtSTAAttributes->uInfraNumSupportedUcastAlgoPairs = uBSSUnicastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);

        //
        // Get multicast algorithm pair list for infrastructure
        //
        PlatformAllocateMemory(pAd,
                            &attr->ExtSTAAttributes->pInfraSupportedMcastAlgoPairs,
                            uBSSMulticastAuthCipherPairsSize);
        if (attr->ExtSTAAttributes->pInfraSupportedMcastAlgoPairs == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Sta11Fill80211Attributes, allocate memory failed for pInfraSupportedMcastAlgoPairs(size=%d)\n", uBSSMulticastAuthCipherPairsSize));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        PlatformMoveMemory(attr->ExtSTAAttributes->pInfraSupportedMcastAlgoPairs, BSSMulticastAuthCipherPairs, uBSSMulticastAuthCipherPairsSize);
        attr->ExtSTAAttributes->uInfraNumSupportedMcastAlgoPairs = uBSSMulticastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);

        //
        // Get unicast algorithm pair list for ad hoc
        //
        PlatformAllocateMemory(pAd,
                            &attr->ExtSTAAttributes->pAdhocSupportedUcastAlgoPairs,
                            uIBSSUnicastAuthCipherPairsSize);
        if (attr->ExtSTAAttributes->pAdhocSupportedUcastAlgoPairs == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Sta11Fill80211Attributes, allocate memory failed for pAdhocSupportedUcastAlgoPairs(size=%d)\n", uIBSSUnicastAuthCipherPairsSize));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        PlatformMoveMemory(attr->ExtSTAAttributes->pAdhocSupportedUcastAlgoPairs, IBSSUnicastAuthCipherPairs, uIBSSUnicastAuthCipherPairsSize);
            attr->ExtSTAAttributes->uAdhocNumSupportedUcastAlgoPairs = uIBSSUnicastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);

        //
        // Get multicast algorithm pair list for ad hoc
        //
       PlatformAllocateMemory(pAd,
                            &attr->ExtSTAAttributes->pAdhocSupportedMcastAlgoPairs,
                            uIBSSMulticastAuthCipherPairsSize);
        if (attr->ExtSTAAttributes->pAdhocSupportedMcastAlgoPairs == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("N6Sta11Fill80211Attributes, allocate memory failed for pAdhocSupportedUcastAlgoPairs(size=%d)\n", uIBSSMulticastAuthCipherPairsSize));
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        PlatformMoveMemory(attr->ExtSTAAttributes->pAdhocSupportedMcastAlgoPairs, IBSSMulticastAuthCipherPairs, uIBSSMulticastAuthCipherPairsSize);
            attr->ExtSTAAttributes->uAdhocNumSupportedMcastAlgoPairs = uIBSSMulticastAuthCipherPairsSize / sizeof(DOT11_AUTH_CIPHER_PAIR);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if(NDIS_WIN8_ABOVE(pAd))
        {
            attr->ExtSTAAttributes->bAutoPowerSaveMode = TRUE;
            attr->ExtSTAAttributes->uMaxNetworkOffloadListSize = MAX_NUM_OF_NLO_ENTRY;
        }
#endif

    } while (FALSE);

    return ndisStatus;
}

NDIS_STATUS
N6Hw11Fill80211Attributes(
    IN  PMP_ADAPTER pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    )
{
    PDOT11_PHY_ATTRIBUTES   PhyAttr;
    ULONG                  PhyId;
    ULONG                  index;
    UCHAR                  rate;    

    if ((IS_DUAL_BAND_NIC(pAd) && (pAd->HwCfg.DisableABandFromEEPROM != TRUE)))
    {
        pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries = 4;
        pAd->StaCfg.pSupportedPhyTypes->uTotalNumOfEntries = MAX_NUM_PHY_TYPES;

        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[0] = dot11_phy_type_hrdsss;
        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[1] = dot11_phy_type_ofdm;
        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[2] = dot11_phy_type_erp;
        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[3] = dot11_phy_type_ht;
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("N6Hw11Fill80211Attributes bgn mix\n"));
        pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries = 3;
        pAd->StaCfg.pSupportedPhyTypes->uTotalNumOfEntries = MAX_NUM_PHY_TYPES;

        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[0] = dot11_phy_type_hrdsss;
        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[1] = dot11_phy_type_erp;
        pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[2] = dot11_phy_type_ht;
    }

    attr->NumOfTXBuffers = 0;
    attr->NumOfRXBuffers = 128;
    attr->MultiDomainCapabilityImplemented = FALSE;
    attr->NumSupportedPhys =  pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries; //ABG

    PlatformAllocateMemory(
                        pAd, 
                        &attr->SupportedPhyAttributes,
                        attr->NumSupportedPhys * sizeof(DOT11_PHY_ATTRIBUTES));

    if (attr->SupportedPhyAttributes == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("N6Hw11Fill80211Attributes, unable to allocate memory for attr->SupportedPhyAttributes\n"));
        return NDIS_STATUS_RESOURCES;
    }

    for (PhyId = 0, PhyAttr = attr->SupportedPhyAttributes; PhyId < attr->NumSupportedPhys; PhyId++, PhyAttr++)
    {
        MP_ASSIGN_NDIS_OBJECT_HEADER(PhyAttr->Header, 
                                     NDIS_OBJECT_TYPE_DEFAULT,
                                     DOT11_PHY_ATTRIBUTES_REVISION_1,
                                     sizeof(DOT11_PHY_ATTRIBUTES));
        PhyAttr->PhyType =  pAd->StaCfg.pSupportedPhyTypes->dot11PHYType[PhyId];

        PhyAttr->bHardwarePhyState = pAd->StaCfg.bHwRadio;
        PhyAttr->bSoftwarePhyState = pAd->StaCfg.bSwRadio;

        PhyAttr->bCFPollable = FALSE;
        PhyAttr->uMPDUMaxLength = 2346;
        PhyAttr->TempType = dot11_temp_type_1;
        PhyAttr->DiversitySupport = dot11_diversity_support_dynamic;

        PhyAttr->uNumberSupportedPowerLevels = 4;
        PhyAttr->TxPowerLevels[0] = 10;
        PhyAttr->TxPowerLevels[1] = 20;
        PhyAttr->TxPowerLevels[2] = 30; 
        PhyAttr->TxPowerLevels[3] = 50;

        switch (PhyAttr->PhyType)
        {
            case dot11_phy_type_hrdsss:
                PhyAttr->HRDSSSAttributes.bShortPreambleOptionImplemented = FALSE;
                PhyAttr->HRDSSSAttributes.bPBCCOptionImplemented = FALSE;
                PhyAttr->HRDSSSAttributes.bChannelAgilityPresent = FALSE;
                PhyAttr->HRDSSSAttributes.uHRCCAModeSupported = DOT11_CCA_MODE_CS_ONLY;
                break;

            case dot11_phy_type_ofdm:
                PhyAttr->OFDMAttributes.uFrequencyBandsSupported = DOT11_FREQUENCY_BANDS_LOWER | 
                                                                   DOT11_FREQUENCY_BANDS_MIDDLE;
                break;

            case dot11_phy_type_erp:
                PhyAttr->ERPAttributes.bShortPreambleOptionImplemented = TRUE;
                PhyAttr->ERPAttributes.bPBCCOptionImplemented = FALSE;
                PhyAttr->ERPAttributes.bChannelAgilityPresent = FALSE;
                PhyAttr->ERPAttributes.uHRCCAModeSupported = DOT11_CCA_MODE_CS_ONLY;
                PhyAttr->ERPAttributes.bERPPBCCOptionImplemented = FALSE;
                PhyAttr->ERPAttributes.bDSSSOFDMOptionImplemented = FALSE;
                PhyAttr->ERPAttributes.bShortSlotTimeOptionImplemented = TRUE;
                break;

            //
            // Unnecessary to fill any specific attributes for dot11_phy_type_ht
            //
            default:
                break;
        }
        
        PlatformMoveMemory(&PhyAttr->SupportedDataRatesValue,
                        &pAd->pNicCfg->PhyMIB[PhyId].SupportedDataRatesValue,
                        sizeof(DOT11_SUPPORTED_DATA_RATES_VALUE_V2));


        index = 0;
        while ((rate = PhyAttr->SupportedDataRatesValue.ucSupportedTxDataRatesValue[index]) != 0 && 
            index < DOT11_RATE_SET_MAX_LENGTH)
        {
            PhyAttr->DataRateMappingEntries[index].ucDataRateIndex = rate;
            PhyAttr->DataRateMappingEntries[index].ucDataRateFlag = 0;
            PhyAttr->DataRateMappingEntries[index].usDataRateValue = (USHORT)rate;

            index++;
        }

        PhyAttr->uNumDataRateMappingEntries = index;
        DBGPRINT(RT_DEBUG_ERROR, ("N6Hw11Fill80211Attributes uNumDataRateMappingEntries = %d \n",index));
    }
    
    return NDIS_STATUS_SUCCESS;
}

VOID 
N6FreePortMemory(
    IN  PMP_ADAPTER   pAd
    )
{
    PMP_PORT pPort;
    INT i;
    
    if (pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd is NULL\n", __FUNCTION__));  
        return;
    }
    
    for(i = 0 ;  i < RTMP_MAX_NUMBER_OF_PORT ;i++)
    {
        //release port pending packet;
        pPort = pAd->PortList[i];

        //free port context
        if(pPort != NULL)
        {
            N6TermPort(pPort);
            N6FreePort(pPort);
        }
    }
}

void N6TermPort(
    IN  PMP_PORT  pPort)
{
    DBGPRINT(RT_DEBUG_TRACE, ("--> N6TermPort\n"));

    NdisFreeSpinLock(&pPort->BATabLock);
    NdisFreeSpinLock(&pPort->BADeleteRECEntry); 
    if (pPort->OidWorkItem)
    {
        NdisFreeIoWorkItem(pPort->OidWorkItem);
        pPort->OidWorkItem = NULL;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("<-- N6TermPort\n"));
}

NDIS_STATUS
N6Sta11QueryExtStaCapability(
    OUT PDOT11_EXTSTA_CAPABILITY   pDot11ExtStaCap)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    do
    {
        pDot11ExtStaCap->uScanSSIDListSize = STA_MAX_SCAN_SSID_LIST_COUNT;     // minimum required.
        pDot11ExtStaCap->uDesiredBSSIDListSize = 1;  // We only support one BSSID.
        pDot11ExtStaCap->uDesiredSSIDListSize = 1;
        pDot11ExtStaCap->uExcludedMacAddressListSize = 4;
        pDot11ExtStaCap->uPrivacyExemptionListSize = 32;
        pDot11ExtStaCap->uKeyMappingTableSize = (16 - DOT11_MAX_NUM_DEFAULT_KEY);
        pDot11ExtStaCap->uDefaultKeyTableSize = DOT11_MAX_NUM_DEFAULT_KEY;
        pDot11ExtStaCap->uWEPKeyValueMaxLength = 104 / 8;
        pDot11ExtStaCap->uPMKIDCacheSize = STA_PMKID_MAX_COUNT;
        pDot11ExtStaCap->uMaxNumPerSTADefaultKeyTables = 16 - DOT11_MAX_NUM_DEFAULT_KEY;

    } while(FALSE);

    return ndisStatus;
}


NDIS_STATUS
N6Sta11QueryExtAPCapability(
    OUT PDOT11_EXTAP_ATTRIBUTES   pDot11ExtAPCap
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    do
    {
        pDot11ExtAPCap->uScanSSIDListSize = AP_SCAN_SSID_LIST_MAX_SIZE;
        pDot11ExtAPCap->uDesiredSSIDListSize = AP_DESIRED_SSID_LIST_MAX_SIZE;
        pDot11ExtAPCap->uPrivacyExemptionListSize = AP_PRIVACY_EXEMPTION_LIST_MAX_SIZE;
        pDot11ExtAPCap->uAssociationTableSize = AP_DEFAULT_ALLOWED_ASSOCIATION_COUNT;
        pDot11ExtAPCap->uDefaultKeyTableSize = DOT11_MAX_NUM_DEFAULT_KEY;
        pDot11ExtAPCap->uWEPKeyValueMaxLength = 104 / 8;
        pDot11ExtAPCap->bStrictlyOrderedServiceClassImplemented = AP_STRICTLY_ORDERED_SERVICE_CLASS_IMPLEMENTED;
        
    } while (FALSE);

    return ndisStatus;
}

//
// Reference StaInitializeStationConfig
// 
// When the bSetDefaultMIB is TRUE, the STA sets the MIB objects to their default values.
// When the bSetDefaultMIB is FALSE, the STA retains the current value of the MIB objects.
VOID 
N6PortN6StaCfgInit(
    IN PMP_PORT pPort
    )
{
    pPort->PortCfg.ExcludeUnencrypted = FALSE;
    pPort->PortCfg.bHiddenNetworkEnabled = FALSE;   
}

//
// Reference StaInitializeStationConfig
// 
// When the bSetDefaultMIB is TRUE, the STA sets the MIB objects to their default values.
// When the bSetDefaultMIB is FALSE, the STA retains the current value of the MIB objects.
VOID N6StaCfgInit(
    IN PMP_ADAPTER pAd,
    IN BOOLEAN bSetDefaultMIB)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    UNREFERENCED_PARAMETER(bSetDefaultMIB);
    
    pAd->StaCfg.BssType = dot11_BSS_type_infrastructure;    
    pAd->StaCfg.bIbssScanOnce = FALSE;

    //
    // Init desired PHY ID list
    //
    pAd->StaCfg.DesiredPhyCount = 2;

    // NdistTest Statistics_ext set DOT11_PHY_ID_ANY will cause fail,at least ERP or OFDM
    pAd->StaCfg.DesiredPhyList[0] = dot11_phy_type_erp;
    pAd->StaCfg.ActivePhyId = 0;

    pPort->CommonCfg.bSSIDInProbeRequest = TRUE;
    pPort->CommonCfg.bRestoreKey  = FALSE;
    
    pAd->StaCfg.bAutoReconnect = TRUE;
}


NDIS_STATUS
N6InterfaceStart(
    IN  PMP_ADAPTER pAd
    )
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;

#if IS_BUS_TYPE_PCI(DEV_BUS_TYPE)

    NdisStatus = NDIS_STATUS_SUCCESS;
    
#elif IS_BUS_TYPE_USB(DEV_BUS_TYPE)

    NdisStatus = N6USBInterfaceStart(pAd);

#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)

    NdisStatus = NDIS_STATUS_SUCCESS;
    
#endif

    return  NdisStatus;
}

NDIS_STATUS
N6PortInitandCfg(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    PMP_PORT      pNewPort = NULL;
    
    NdisStatus = N6AllocPort(pAd->AdapterHandle, EXTSTA_PORT, PORTSUBTYPE_STA, &pNewPort);
    if(NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT (RT_DEBUG_TRACE, ("N6AllocPort fail in RTMPCreateMac\n"));                       
        return NdisStatus;
    }

    pNewPort->PortNumber = NDIS_DEFAULT_PORT_NUMBER;
    pNewPort->pAd = pAd;
    pNewPort->AdapterHandle = pAd->AdapterHandle;
    pAd->PortList[NDIS_DEFAULT_PORT_NUMBER] = pNewPort;
    pAd->NumberOfPorts++;
    pAd->ucActivePortNum = NDIS_DEFAULT_PORT_NUMBER;

    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
   pAd->pP2pCtrll->pInfraPort = pNewPort;
#endif

     pAd->StaCfg.PmfCfg.MFPC = pAd->StaCfg.PmfCfg.PmfControl.MFP_Enable;     //Init MFP capabilities
    pAd->StaCfg.PmfCfg.MFPR = pAd->StaCfg.PmfCfg.PmfControl.MFP_Required;
    DBGPRINT(RT_DEBUG_TRACE, ("MFP: Init RSNCapability: 0x%2x\n", pAd->StaCfg.PmfCfg.RSNCapability));

    N6InitPort(pNewPort);
    N6PortUserCfgInit(pNewPort);
    pNewPort->bActive = TRUE;    

    // Set Port Sub type to SoftAP if Registry set OPMode to OPMODE_AP
    if (pAd->OpMode == OPMODE_AP)
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] pAd->OpMode == OPMODE_AP\n",__FUNCTION__,__LINE__));            
        pNewPort->PortSubtype = PORTSUBTYPE_SoftapAP;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] pAd->OpMode == OPMODE_STA\n",__FUNCTION__,__LINE__));           
        pNewPort->PortSubtype = PORTSUBTYPE_STA;
    }  

    return NdisStatus;
}
