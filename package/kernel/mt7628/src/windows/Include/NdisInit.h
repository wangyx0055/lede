#ifndef __NDISINIT_H__
#define __NDISINIT_H__

typedef struct _ADAPTER_INITIAL_STAGE   ADAPTER_INITIAL_STAGE,  *PADAPTER_INITIAL_STAGE;

NDIS_STATUS 
NdisInitAdapterBlock(
    IN  PMP_ADAPTER   pAd,
    OUT PADAPTER_INITIAL_STAGE   pInitialStage
    );

VOID  
NdisInitFreeRepositMemory(
    IN  PMP_ADAPTER   pAd
    );

NDIS_STATUS 
NdisInitGetBssList(
    IN  PMP_ADAPTER   pAd,
    OUT PVOID           InformationBuffer,
    IN  ULONG           OutputBufferLength
    );    
#endif
