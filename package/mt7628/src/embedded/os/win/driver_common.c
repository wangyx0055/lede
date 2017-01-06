#ifdef RTMP_SDIO_SUPPORT
#include "sdio_config.h"
#endif

#ifdef RTMP_USB_SUPPORT
#include "config.h"
#endif

// Unify all delay routine by using NdisStallExecution
/*
__inline    VOID    RTMPusecDelay(
    IN      UINT32   usec)
{
	ULONG   i;

	for (i = 0; i < (usec / 50); i++)
		KeStallExecutionProcessor(50);

	if (usec % 50)
		KeStallExecutionProcessor(usec % 50);
}*/
 NTSTATUS	Event_Wait(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	KEVENT			*pEvent,
	IN	LARGE_INTEGER       	TimeOut1Second){
	NTSTATUS ntStatus = STATUS_SUCCESS;
	if(TimeOut1Second.QuadPart == 0)
		TimeOut1Second.QuadPart = -(1000*10000);	
	ntStatus = KeWaitForSingleObject(pEvent,
	                                Executive,
	                                KernelMode,
	                                FALSE,
	                                &TimeOut1Second);

	if (ntStatus == STATUS_TIMEOUT)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("ERROR!!! wait  0x%x,  time out \n", TimeOut1Second.QuadPart));		
	}

	if (ntStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("ERROR!!! ntStatus = 0x%x\n", ntStatus));		
	}	
	return ntStatus;
 }

/*
	========================================================================
	
	Routine Description:
		Compare two memory block

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		1:			memory are equal
		0:			memory are not equal

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
ULONG	RTMPEqualMemory(
	IN	PVOID	pSrc1,
	IN	PVOID	pSrc2,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] != pMem2[Index])
		{
			break;
		}
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Length = %d, Index = %d\n",__FUNCTION__, Length, Index));
	if ((Index == Length))
	{
		return (1);
	}
	else
	{
		return (0);
	}
}

/*
	========================================================================
	
	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address
		
	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
ULONG	RTMPCompareMemory(
	IN	PVOID	pSrc1,
	IN	PVOID	pSrc2,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	// Equal
	return (0);
}

/*
	========================================================================
	
	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None
		
	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	RTMPZeroMemory(
	IN	PVOID	pSrc,
	IN	ULONG	Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem[Index] = 0x00;
	}
}

/*
	========================================================================
	
	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size
		
	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	RTMPMoveMemory(
	OUT	PVOID	pDest,
	IN	PVOID	pSrc,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index;

	ASSERT((Length==0) || (pDest && pSrc));

	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem1[Index] = pMem2[Index];
	}
}

KIRQL CheckIRQL(RTMP_ADAPTER *pAd)
{
	KIRQL Irql;
	Irql	= KeGetCurrentIrql();
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("DISPATCH_LEVEL = 0x%x, PASSIVE_LEVEL =  (0x%x)\n", DISPATCH_LEVEL, PASSIVE_LEVEL));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("{}current irql = %d (0x%x)\n", (ULONG)Irql, (ULONG)Irql));
	
	if(Irql == DISPATCH_LEVEL)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, Irql =  DISPATCH_LEVEL\n",__FUNCTION__));
	if(Irql == PASSIVE_LEVEL)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, Irql =  PASSIVE_LEVEL\n",__FUNCTION__));
	if(Irql == APC_LEVEL)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, Irql =  APC_LEVEL\n",__FUNCTION__));

	return Irql;
}
