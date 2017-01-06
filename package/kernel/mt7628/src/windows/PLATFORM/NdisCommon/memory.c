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
    memory.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "memory.h"

extern ULONG    G_MemoryAllocate_cnt;
extern ULONG    G_MemoryFree_cnt;
extern ULONG    G_MemoryAllocate_Len;
extern ULONG    G_MemoryFree_Len;

/*
    ========================================================================
    
    Routine Description:
        Compare two memory block

    Arguments:
        pSrc1       Pointer to first memory address
        pSrc2       Pointer to second memory address
        
    Return Value:
        0:          memory is equal
        1:          pSrc1 memory is larger
        2:          pSrc2 memory is larger

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
ULONG   MtkCompareMemory(
    IN  PVOID   pSrc1,
    IN  PVOID   pSrc2,
    IN  ULONG   Len
    )
{
    PUCHAR  pMem1;
    PUCHAR  pMem2;
    ULONG   i = 0;

    pMem1 = (PUCHAR) pSrc1;
    pMem2 = (PUCHAR) pSrc2;

    for (i = 0; i < Len; i++)
    {
        if (pMem1[i] < pMem2[i])
        {
            return (2);
        }
        else if (pMem1[i] > pMem2[i])
        {
            return (1);
        }
    }

    // Equal
    return (0);
}

//======================================
// Memory related function
//======================================

//
//  Description:
//  Memory manipulation routines.
//
NDIS_STATUS
PlatformAllocateMemory(
    IN  PMP_ADAPTER   pAd,
    OUT PVOID   *pPtr,
    IN  ULONG   length
    )
{
    NDIS_HANDLE MiniportAdapterHandle = pAd->AdapterHandle;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    
    if(NDIS_WIN7_BELOW(pAd))
    {
        ndisStatus = NdisAllocateMemoryWithTag( pPtr, length, '3067' );

        if( ndisStatus != NDIS_STATUS_SUCCESS )
        {
          return ndisStatus;
        }
    }
    else
    {
        *pPtr = NdisAllocateMemoryWithTagPriority( MiniportAdapterHandle, 
                                                                        length, 
                                                                        '3067',
                                                                        NormalPoolPriority);
    }
    if (*pPtr == NULL)
    {
        return NDIS_STATUS_FAILURE;
    }

    G_MemoryAllocate_cnt ++;
    G_MemoryAllocate_Len += length;
    return ndisStatus;
}


VOID
PlatformFreeMemory(
    IN  PVOID   ptr,
    IN  ULONG   length
    )
{
    if(ptr)
    {
        NdisFreeMemory( ptr, length, 0);
        ptr = NULL;
        G_MemoryFree_cnt ++;
        G_MemoryFree_Len += length;

        G_MemoryAllocate_Len -= length;
    }
}


//
// Description:
//  Fill all the input memory with 0.
//
VOID
PlatformZeroMemory(
    IN  PVOID   ptr,
    IN  ULONG   length
    )
{
    if(ptr)
        NdisZeroMemory(ptr,length);
}

VOID
PlatformMoveMemory(
    IN  PVOID   pDest,
    IN  PVOID   pSrc,
    IN  ULONG   length
    )
{
    if((pDest != NULL) && (pSrc != NULL) && (length > 0))
    {
         NdisMoveMemory(pDest,pSrc,length);
    }
}

//
// Description:
//  Compare characters in two buffers.
//
LONG
PlatformEqualMemory(
    IN  PVOID   pBuf1,
    IN  PVOID   pBuf2,
    IN  ULONG   length
    )
{
    LONG val =0;
    val = NdisEqualMemory(pBuf1,pBuf2,length);
    return val;
}

//
// Description:
//  Fills a caller-supplied buffer with the given character.
//
VOID
PlatformFillMemory(
    IN  PVOID   pBuf,
    IN  ULONG   Length,
    IN  UCHAR   Fill
    )
{
    if(pBuf != NULL)
    {
        NdisFillMemory(pBuf, Length, Fill);
    }
}