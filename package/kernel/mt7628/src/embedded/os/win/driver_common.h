
#ifndef _DRIVER_COMMON_H 
#define _DRIVER_COMMON_H


#define BIT0	(1 << 0)
#define BIT1	(1 << 1)
#define BIT2	(1 << 2)
#define BIT3	(1 << 3)
#define BIT4	(1 << 4)
#define BIT5	(1 << 5)
#define BIT6	(1 << 6)
#define BIT7	(1 << 7)
#define BIT8	(1 << 8)
#define BIT9	(1 << 9)
#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)
#define BIT16	(1 << 16)
#define BIT17	(1 << 17)
#define BIT18	(1 << 18)
#define BIT19	(1 << 19)
#define BIT20	(1 << 20)
#define BIT21	(1 << 21)
#define BIT22	(1 << 22)
#define BIT23	(1 << 23)
#define BIT24	(1 << 24)
#define BIT25	(1 << 25)
#define BIT26	(1 << 26)
#define BIT27	(1 << 27)
#define BIT28	(1 << 28)
#define BIT29	(1 << 29)
#define BIT30	(1 << 30)
#define BIT31	(1 << 31)

#define BITSET(_Value, _BIT) (_Value|_BIT)
#define BITCLEAR(_Value, _BIT) (_Value&~_BIT)

#define DEBUG_OFF          0
#define DEBUG_ERROR      1
#define DEBUG_WARN       2
#define DEBUG_TRACE      3
#define DEBUG_INFO        4
#define DEBUG_LOUD        5

#define DBG_LVL_OFF          0
#define DBG_LVL_ERROR      1
#define DBG_LVL_WARN       2
#define DBG_LVL_TRACE      3
#define DBG_LVL_INFO        4
#define DBG_LVL_LOUD        5

//mem allocate
#define TAG0        0x150
#define TAG1        0x151

//__inline    VOID    RTMPusecDelay(
//    IN      UINT32   usec);

// Unify all delay routine by using NdisStallExecution
__inline    VOID    RTMPusecDelay(
    IN      ULONG   usec)
{
	ULONG   i;

	for (i = 0; i < (usec / 50); i++)
		KeStallExecutionProcessor(50);

	if (usec % 50)
		KeStallExecutionProcessor(usec % 50);
}
/*extern ULONG    RTDebugLevel;

#define DBGPRINT_RAW(Category, Level, Fmt)    \
{                                   \
	if (Level <= RTDebugLevel)      \
	{                               \
		DbgPrint  Fmt;               \
	}                               \
}
*/
//#define DBGPRINT(Level, Fmt)    DBGPRINT_RAW(DBG_CAT_ALL, Level, Fmt)
#ifdef DBG
#define DBGPRINT(Level, Fmt)          \
{                                   \
	DbgPrint Fmt;                  \
}

#define DBGPRINT_ERR(Fmt)           \
{                                   \
	DbgPrint("ERROR!!! ");          \
	DbgPrint Fmt;                  \
}

#define MTWF_LOG(Category, SubCategory, Level, Fmt)    DBGPRINT(Level, Fmt)

#else
#define DBGPRINT(Level, Fmt)
#define MTWF_LOG(Category, SubCategory, Level, Fmt)
#define DBGPRINT_RAW(Category, Level, Fmt)
#define DBGPRINT_ERR(Fmt)
#endif

typedef enum { 
	IRPLOCK_COMPLETED, 
	IRPLOCK_CANCELABLE,
	IRPLOCK_CANCE_START,
	IRPLOCK_CANCE_COMPLETE,
}	IRPLOCK;

NTSTATUS
Event_Wait(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	KEVENT			*pEvent,
	IN	LARGE_INTEGER       	TimeOut1Second);

ULONG RTMPEqualMemory(
	IN  PVOID   pSrc1,
	IN  PVOID   pSrc2,
	IN  ULONG   Length);

VOID RTMPZeroMemory(
	IN  PVOID   pSrc,
	IN  ULONG   Length);

ULONG RTMPCompareMemory(
	IN  PVOID   pSrc1,
	IN  PVOID   pSrc2,
	IN  ULONG   Length);

VOID RTMPMoveMemory(
	OUT PVOID   pDest,
	IN  PVOID   pSrc,
	IN  ULONG   Length);

KIRQL CheckIRQL(RTMP_ADAPTER *pAd);
#endif //_DRIVER_COMMON_H