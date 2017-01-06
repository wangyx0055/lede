#ifndef __MLMEINFO_H__
#define __MLMEINFO_H__

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER, RTMP_ADAPTER;
typedef struct _MP_PORT MP_PORT, *PMP_PORT;

typedef enum _MP_PORT_TYPE MP_PORT_TYPE, *PMP_PORT_TYPE;
typedef enum _MP_PORT_SUBTYPE MP_PORT_SUBTYPE, *PMP_PORT_SUBTYPE;

typedef union  _PHY_CFG PHY_CFG, *PPHY_CFG;
typedef struct _CIPHER_KEY CIPHER_KEY, *PCIPHER_KEY;
typedef struct _RT_HT_CAPABILITY RT_HT_CAPABILITY, *PRT_HT_CAPABILITY;
typedef struct    _HT_CAPABILITY_IE HT_CAPABILITY_IE, *PHT_CAPABILITY_IE;

ULONG64
MlmeInfoQueryLinkSpeed(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort, 
    IN PPHY_CFG pPhyCfg
    );

NDIS_STATUS 
MlmeInfoWPAAddKeyProc(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PVOID           pBuf
    );  

VOID 
MlmeInfoStaOidSetWEPDefaultKeyID(  
    IN  PMP_PORT      pPort
    );    

NDIS_STATUS 
MlmeInfoAddKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx,
    IN  DOT11_CIPHER_ALGORITHM AlgorithmId,
    IN  ULONG           KeyLen,
    IN  PUCHAR          KeyValue,
    IN  BOOLEAN         bDelete,
    IN  BOOLEAN         bPairwise,
    IN  BOOLEAN         bTxKey
    );    

BOOLEAN
MlmeInfoAPFindKeyMappingKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr
    );    

BOOLEAN
MlmeInfoFindKeyMappingKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr
    );    

PCIPHER_KEY
MlmeInfoFindStaDefaultKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx
    );

NDIS_STATUS
MlmeInfoAddPerStaDefaultKey(    
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx,
    IN  DOT11_CIPHER_ALGORITHM AlgorithmId,
    IN  ULONG           KeyLen,
    IN  PUCHAR          KeyValue
    );

NDIS_STATUS 
MlmeInfoWPARemoveKeyProc(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PVOID           pBuf
    );

VOID    
MlmeInfoWPARemoveAllKeys(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    );

VOID    
MlmeInfoIndicateWPA2Status(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    );

VOID    
MlmeInfoSetPhyMode(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT        pPort,    
    IN  ULONG           phymode,
    IN  BOOLEAN         bForceUpdaetChList
    );

VOID    
MlmeInfoOPModeSwitching(
    IN  PMP_ADAPTER   pAd
    );

VOID    
MlmeInfoAddBSSIDCipher(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR   Aid,
    IN  PNDIS_802_11_KEY    pKey
    );

VOID 
MlmeInfoInitCipherAttrib(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pAddr
    );

VOID 
MlmeInfoUpdateHtIe(
    IN  RT_HT_CAPABILITY    *pRtHt,
    OUT HT_CAPABILITY_IE    *pHtCapability,
    OUT ADD_HT_INFO_IE      *pAddHtInfo
    );

VOID    
MlmeInfoUpdateVhtIe(
    IN    RT_VHT_CAPABILITY   *pRtVht,
    OUT   VHT_CAP_IE          *pVhtCapaIe,
    OUT   VHT_OP_IE           *pVhtOperIe
    );    

ULONG
MlmeInfoGetLinkQuality(
    IN  PMP_ADAPTER   pAd,
    IN  LONG            Rssi,
    IN  BOOLEAN bPeriodicIndicate
    );

ULONG
MlmeInfoGetLinkQualityLevel(
    IN  ULONG Quality
    );    

ULONG
MlmeInfoGetPhyIdByChannel(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel
    );    

void
MlmeInfoPnpSetPowerMgmtMode(
    IN PMP_ADAPTER pAd, 
    IN ULONG PowerMode
    );

VOID 
MlmeInfoMantainAdhcoList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR Addr2
    );

VOID 
MlmeInfoUpdateAdhocList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR Addr2
    );

VOID 
MlmeInfoEnablePromiscuousMode(
    IN PMP_ADAPTER pAd
    );

VOID 
MlmeInfoDisablePromiscuousMode(
    IN PMP_ADAPTER pAd
    );    
#endif