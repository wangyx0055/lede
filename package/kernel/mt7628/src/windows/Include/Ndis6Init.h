#ifndef __NDIS6INIT_H__
#define __NDIS6INIT_H__

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;
typedef struct _MP_PORT MP_PORT, *PMP_PORT;

typedef enum _MP_PORT_TYPE MP_PORT_TYPE, *PMP_PORT_TYPE;
typedef enum _MP_PORT_SUBTYPE MP_PORT_SUBTYPE, *PMP_PORT_SUBTYPE;




typedef struct _ADAPTER_INITIAL_STAGE {
    BOOLEAN bAllocAdapter;
    BOOLEAN bInitXmit;
    BOOLEAN bInitRecv;
    BOOLEAN bInitMLME;
    BOOLEAN bEnableXmit;
    BOOLEAN bEnableRecv;
    BOOLEAN bLoadFw;
    BOOLEAN bAllocSpinLock;
} _ADAPTER_INITIAL_STAGE, *PADAPTER_INITIAL_STAGE;


extern WDF_REQUEST_SEND_OPTIONS G_RequestOptions;


NDIS_STATUS 
N6AllocPort(
    IN  NDIS_HANDLE       MiniportAdapterHandle,
    IN  MP_PORT_TYPE      PortType,
    IN  MP_PORT_SUBTYPE   PortSubType,
    OUT PMP_PORT*       ppPort
    );

NDIS_STATUS
N6AllocateNdisPort(
     IN  PMP_PORT pPort,
     OUT PNDIS_PORT_NUMBER       AllocatedPortNumber);

NDIS_STATUS
N6PortActivateNdisPort(
    IN  PMP_ADAPTER pAd,
    IN  NDIS_PORT_NUMBER PortNumberToActivate
    );

VOID
N6PortDeactivateNdisPort(
    IN  PMP_ADAPTER pAd,
    IN  NDIS_PORT_NUMBER PortNumberToDeactivate
    );

VOID
N6PortDeRegisterActivePort(
    IN  PMP_ADAPTER pAd
    );

VOID 
N6FreePort(
    IN PMP_PORT pPort
    );

NDIS_STATUS 
N6InitPort(
    IN  PMP_PORT  pPort
    );

void 
N6TermPort(
    IN  PMP_PORT  pPort
    );    


NDIS_STATUS
N6RegistrationAttributes (
    IN  PMP_ADAPTER pAd
    );
    
NDIS_STATUS
N6IniMiniportAttributes (
    IN  PMP_ADAPTER   pAd
    );


VOID    
N6PortUserCfgInit(
    IN  PMP_PORT      pPort
    );

NDIS_STATUS
N6Set80211Attributes(
    IN  PMP_ADAPTER     pAd
    );

NDIS_STATUS
N6SetGeneralAttributes(
    IN  PMP_ADAPTER     pAd
    );   

NDIS_STATUS
N6Vw11Fill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    );

NDIS_STATUS
N6Ap11Fill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    );

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
NDIS_STATUS
N6WfdFill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    );
#endif

NDIS_STATUS
N6Sta11Fill80211Attributes(
    IN  PMP_ADAPTER        pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    );

NDIS_STATUS
N6Hw11Fill80211Attributes(
    IN  PMP_ADAPTER pAd,
    OUT PNDIS_MINIPORT_ADAPTER_NATIVE_802_11_ATTRIBUTES attr
    ); 
    

VOID 
N6FreePortMemory(
    IN  PMP_ADAPTER   pAd
    );   

NDIS_STATUS
N6Sta11QueryExtStaCapability(
    OUT PDOT11_EXTSTA_CAPABILITY   pDot11ExtStaCap);

#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
NDIS_STATUS
N6Sta11QueryExtAPCapability(
    OUT PDOT11_EXTAP_ATTRIBUTES   pDot11ExtAPCap
    );
#endif

VOID 
N6PortN6StaCfgInit(    
    IN PMP_PORT pPort
    );

VOID 
N6StaCfgInit(
    IN PMP_ADAPTER pAd,
    IN BOOLEAN bSetDefaultMIB
    );    

NDIS_STATUS
N6InterfaceStart(
    IN  PMP_ADAPTER pAd
    );

NDIS_STATUS
N6PortInitandCfg(
    IN PMP_ADAPTER pAd
    );
#endif