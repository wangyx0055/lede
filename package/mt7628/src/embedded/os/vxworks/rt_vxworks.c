/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_vxworks.c

    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#include "rt_config.h"
#include "time.h"
#include "mt7628End.h"
#include "random.h"
#include "mirrorEnd.h"
#include "bridge.h"
#include "commonType.h"

extern int netTaskId;
extern COUNTRY_CODE_TO_COUNTRY_REGION allCountry[];

struct dev_type_name_map{
	int type;
	char *prefix[2];
};


#define MBLK_FLAG_SET(pMblk, _flags)		((pMblk)->mBlkHdr.mFlags |= (_flags))

#define SECOND_INF_MAIN_DEV_NAME		"rai"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#define SECOND_INF_WDS_DEV_NAME		"wdsi"
#define SECOND_INF_APCLI_DEV_NAME	"apclii"
#define SECOND_INF_MESH_DEV_NAME		"meshi"
#define SECOND_INF_P2P_DEV_NAME		"p2pi"

static UCHAR init_flag = 0;
static PUCHAR nv_ee_start = 0;

void memDump(unsigned int memStart, unsigned int memSize);


UCHAR EeBuffer[EEPROM_SIZE]= {
0x28, 0x76, 0x06, 0x00, 0x00, 0x0c, 0x43, 0xe1, 0x76, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x22, 0x34, 0x00, 0x20, 0xff, 0xff, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x82, 0x00, 0x00, 0x94, 0x40, 0xb0, 0xc0, 0xc9, 0x23, 0x00, 0x00, 0x00, 0x40, 0xc9, 0x23, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0xc6, 0xc6, 0xc4, 0xc4, 0xc4, 0xc0, 0xc0, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc0, 0xc0, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x11, 0x1d, 0x11, 0x1d, 0x1c, 0x35, 0x1c, 0x35, 0x1e, 0x35, 0x1e, 0x35, 0x17, 0x19, 0x17, 0x19, 
0x00, 0x00, 0x00, 0x00, 0xbd, 0x00, 0xce, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x00, 
0x11, 0x1d, 0x11, 0x1d, 0x15, 0x7f, 0x15, 0x7f, 0x17, 0x7f, 0x17, 0x7f, 0x10, 0x3b, 0x10, 0x3b, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
};

static struct dev_type_name_map prefix_map[] =
{
	{INT_MAIN, 		{INF_MAIN_DEV_NAME, SECOND_INF_MAIN_DEV_NAME}},
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	{INT_MBSSID, 	{INF_MBSSID_DEV_NAME, SECOND_INF_MBSSID_DEV_NAME}},
#endif /* MBSS_SUPPORT */
#ifdef APCLI_SUPPORT
	{INT_APCLI, 		{INF_APCLI_DEV_NAME, SECOND_INF_APCLI_DEV_NAME}},
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	{INT_WDS, 		{INF_WDS_DEV_NAME, SECOND_INF_WDS_DEV_NAME}},
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MESH_SUPPORT
	{INT_MESH, 		{INF_MESH_DEV_NAME, SECOND_INF_MESH_DEV_NAME}},
#endif /* MESH_SUPPORT */
#ifdef P2P_SUPPORT
	{INT_P2P, 		{INF_P2P_DEV_NAME, SECOND_INF_P2P_DEV_NAME}},
#endif /* P2P_SUPPORT */
	{0},
};

#define IWEVEXPIRED	0x8C04

/*
	[Problem] For RT2860PCI on the vxWorks, can not use RT_DEBUG_TRACE;
	Or when you switch channel during traffic throughput is running,
	the RT2860PCI AP on the RT2880 board will not send out any packet
	in new channel.
	[Cause] Unknown.
	[Solution] Use RT_DEBUG_ERROR.
*/
#ifdef VXWORKS

typedef struct _WLAN_DRV_CTRL
{
	DRV_CTRL	drvCtrl;
} WLAN_DRV_CTRL;
extern WLAN_DRV_CTRL apCliDryCtrl;
WLAN_DRV_CTRL g_wlan_mbss_drvCtrl[8] = {0};	/* use for eth drv_ctl */
#endif
/* TODO */
#undef RT_CONFIG_IF_OPMODE_ON_AP
#undef RT_CONFIG_IF_OPMODE_ON_STA
extern int wlan_fd;
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)	if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif

ULONG RTDebugLevel = RT_DEBUG_ERROR;
ULONG RTDebugFunc = 0;
ULONG DebugCategory = DBG_CAT_ALL;

UCHAR VxDbgBuffer[256];

#ifdef RTMP_RBUS_SUPPORT
#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)
extern int (
	*ra_classifier_hook_rx) (
	struct sk_buff *skb,
	unsigned long cycle);

extern volatile unsigned long classifier_cur_cycle;
#endif /* CONFIG_RA_CLASSIFIER */
#endif /* RTMP_RBUS_SUPPORT */


#ifdef CACHE_DMA_SUPPORT
UCHAR CACHE_DMA_SIGNAUTURE[]={0x52,0x54,0x4D,0x50};
#endif /* CACHE_DMA_SUPPORT */

#ifdef VENDOR_FEATURE4_SUPPORT
ULONG	OS_NumOfMemAlloc = 0, OS_NumOfMemFree = 0;
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef VENDOR_FEATURE2_SUPPORT
ULONG	OS_NumOfPktAlloc = 0, OS_NumOfPktFree = 0;
#endif /* VENDOR_FEATURE2_SUPPORT */

//woody 20110830
BOOLEAN FlgIsUtilInit = FALSE;
//OS_NDIS_SPIN_LOCK UtilSemLock;

NDIS_SPIN_LOCK UtilSemLock;

#if 0
//add woody
VOID RtmpUtilInit(VOID)
{
	if (FlgIsUtilInit == FALSE) {
		OS_NdisAllocateSpinLock(&UtilSemLock);
		FlgIsUtilInit = TRUE;
	}
}
#endif
/*
	This function copy from target/usr/src/libdl/strsep.c
*/
char *strsep(register char **stringp, register const char *delim)
{
	register char *s;
	register const char *spanp;
	register int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

#if 0
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
        unsigned long result = 0,value;

        if (!base) {
                base = 10;
                if (*cp == '0') {
                        base = 8;
                        cp++;
                        if ((*cp == 'x') && isxdigit(cp[1])) {
                                cp++;
                                base = 16;
                        }
                }
        }
        while (isxdigit(*cp) &&
               (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {
                result = result*base + value;
                cp++;
        }
        if (endp)
                *endp = (char *)cp;
        return result;
}
#endif

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;
	//printk("in simple strtoul------------------\n");
	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			//printk("in simple strtoul---------1111111111---------\n");
			if ((toupper(*cp) == 'X') && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	} else if (base == 16) {
		if (cp[0] == '0' && toupper(cp[1]) == 'X')
			cp += 2;
		//printk("in simple strtoul---------2222222222---------\n");
	}
	while (isxdigit(*cp) &&
	       (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
		//printk("in simple strtoul---------33333333333---------\n");
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}


long simple_strtol(const char *cp,char **endp,unsigned int base)
{
        if(*cp=='-')
                return -simple_strtoul(cp+1,endp,base);
        return simple_strtoul(cp,endp,base);
}

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	return;
}

void hex_dump_woody(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	my_printf("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			my_printf("0x%04x : ", x);
		my_printf("%02x ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			my_printf("\n");
	}
	my_printf("\n");
}

void RtmpAllocDescBuf(
	IN PPCI_DEV pPciDev,
	IN UINT Index,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	phy_addr)
{
#if 0
	dma_addr_t DmaAddr = (dma_addr_t)(*phy_addr);

	*VirtualAddress = (PVOID)pci_alloc_consistent(pPciDev,sizeof(char)*Length, &DmaAddr);
	*phy_addr = (NDIS_PHYSICAL_ADDRESS)DmaAddr;
#endif

	/*POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie; */

#ifdef VXWORKS_5X
	*VirtualAddress = (PVOID) cacheDmaMalloc(Length);
	if (!Cached)
		*VirtualAddress = (PVOID) K0_TO_K1((UINT32)(*VirtualAddress));
#endif /* VXWORKS_5X */
#ifdef VXWORKS_6X
	*VirtualAddress = (PVOID) kmalloc(Length, 0);
	if (!Cached)
		*VirtualAddress = (PVOID) KSEG1((UINT32)(*VirtualAddress));
#endif /* VXWORKS_6X */
		*phy_addr = (ra_dma_addr_t)PCI_VIRT_TO_PHYS(*VirtualAddress);

}


/* Function for free allocated Desc Memory. */
void RtmpFreeDescBuf(
	IN PPCI_DEV pPciDev,
	IN ULONG Length,
	IN VOID *VirtualAddress,
	IN NDIS_PHYSICAL_ADDRESS phy_addr)
{
#if 0
	dma_addr_t DmaAddr = (dma_addr_t)(phy_addr);

	pci_free_consistent(pPciDev, Length, VirtualAddress, DmaAddr);
#endif
	/*POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie; */

#ifdef VXWORKS_5X
	VirtualAddress = (PVOID) K1_TO_K0(VirtualAddress);
	cacheDmaFree((PVOID)VirtualAddress);
#endif /* VXWORKS_5X */
#ifdef VXWORKS_6X
	kfree(VirtualAddress);
#endif /* VXWORKS_6X */
}

/* timeout -- ms */
static inline VOID __RTMP_SetPeriodicTimer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	STATUS status = ERROR;
	
	if (!(pTimer->id))
		pTimer->id = wdCreate();

	if (pTimer->id)
	{	timeout = (timeout * sysClkRateGet()) /1000;
		status = wdStart(pTimer->id, timeout, pTimer->func, (int)pTimer->data);
	}
	
	if (status == ERROR)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("setPeriodicTimer failed!\n"));
	}
}

/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
static inline VOID __RTMP_OS_Init_Timer(
	IN VOID *pReserved,
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN TIMER_FUNCTION function,
	IN PVOID data)
{
	pTimer->id = wdCreate();
	if (pTimer->id)
	{
		pTimer->func = (FUNCPTR)function;
		pTimer->data = data;
	}
	else
	{
		PRALINK_TIMER_STRUCT	pTimer;

		pTimer = (PRALINK_TIMER_STRUCT)data;
		pTimer->Valid = FALSE;
		DBGPRINT(RT_DEBUG_ERROR, ("init_Timer failed!\n"));
	}
}

static inline VOID __RTMP_OS_Add_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	STATUS status = ERROR;
	
	if (!(pTimer->id))
		pTimer->id = wdCreate();
		
	if (pTimer->id)
	{
		timeout = (timeout * sysClkRateGet()) /1000;
		status = wdStart (pTimer->id, timeout, pTimer->func, (int)pTimer->data);
	}
	
	if (status == ERROR)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Add_Timer failed!\n"));
	}
}

static inline VOID __RTMP_OS_Mod_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	STATUS status = ERROR;
	
	if (!(pTimer->id))
		pTimer->id = wdCreate();
		
	if (pTimer->id)
	{
		timeout = (timeout * sysClkRateGet()) /1000;
		status = wdStart (pTimer->id, timeout, pTimer->func, (int)pTimer->data);
	}

	if (status == ERROR)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Mod_Timer failed!\n"));
	}
}

static inline VOID __RTMP_OS_Del_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	OUT BOOLEAN *pCancelled)
{
	STATUS status = OK;

	if (pTimer->id)
		status = wdDelete(pTimer->id);

	if (status == OK)
	{
		pTimer->id = NULL;
		*pCancelled = TRUE;
	}
	else
	{
		logMsg("CancelTimer(func=0x%x) failed!\n", (ULONG)pTimer->func,0,0,0,0,0);
	}
}

static inline VOID __RTMP_OS_Release_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer)
{
	/* nothing to do */
}


/***********************************************************************************
 *
 *	Definition of timer and time related functions
 *
 ***********************************************************************************/
 

/* timeout -- ms */
VOID RTMP_SetPeriodicTimer(IN NDIS_MINIPORT_TIMER *pTimerOrg,
			   IN unsigned long timeout) {
	__RTMP_SetPeriodicTimer(pTimerOrg,
				timeout);
}

/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
VOID RTMP_OS_Init_Timer(
					  IN VOID *pReserved,
					  IN NDIS_MINIPORT_TIMER *pTimerOrg,
					  IN TIMER_FUNCTION function,
					  IN PVOID data,
					  IN LIST_HEADER *pTimerList) {
	__RTMP_OS_Init_Timer(pReserved, pTimerOrg, function, data);
}

VOID RTMP_OS_Add_Timer(IN NDIS_MINIPORT_TIMER *pTimerOrg,
		       IN unsigned long timeout) {
	__RTMP_OS_Add_Timer(pTimerOrg,
			    timeout);
}

VOID RTMP_OS_Mod_Timer(IN NDIS_MINIPORT_TIMER *pTimerOrg,
			 IN unsigned long timeout) {
	__RTMP_OS_Mod_Timer(pTimerOrg,
			    timeout);
}

VOID RTMP_OS_Del_Timer(IN NDIS_MINIPORT_TIMER *pTimerOrg,
			 OUT BOOLEAN *pCancelled) {
	__RTMP_OS_Del_Timer(pTimerOrg, pCancelled);
}

VOID RTMP_OS_Release_Timer(IN NDIS_MINIPORT_TIMER *pTimerOrg) {
	__RTMP_OS_Release_Timer(pTimerOrg);
}



VOID RtmpOsMlmeUp(IN RTMP_OS_TASK *pMlmeQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pMlmeQTask);

#ifdef KTHREAD_SUPPORT
	if ((pTask != NULL) && (pTask->kthread_task)) {
		pTask->kthread_running = TRUE;
		wake_up(&pTask->kthread_q);
	}
#else
	if (pTask != NULL) {
		CHECK_PID_LEGALITY(pTask->taskPID) {
			RTMP_SEM_EVENT_UP(&(pTask->taskSema));
		}
	}
#endif /* KTHREAD_SUPPORT */
}



#if 1
/* Unify all delay routine by using udelay */
VOID RtmpusecDelay(
	IN	ULONG	usec)
{
	sysUDelay(usec);
}

VOID RTMPusecDelay(
	IN	ULONG	usec)
{
	sysUDelay(usec);
}

VOID RtmpOsMsDelay(
	IN	ULONG	msec)
{
	sysMsDelay(msec);
}

VOID RtmpVxMsDelay(
	IN	ULONG	msec)
{
	struct timespec nsTime;
	nsTime.tv_sec = 0;
	nsTime.tv_nsec = 1000000 * msec;
	nanosleep(&nsTime, NULL);
}

#else	

VOID RtmpusecDelay(
	IN	ULONG	usec)
{
	struct timespec nsTime;
	nsTime.tv_sec = 0;
	nsTime.tv_nsec = 1000 * usec;
	nanosleep(&nsTime, NULL);
}

VOID RTMPusecDelay(
	IN	ULONG	usec)
{
	struct timespec nsTime;
	nsTime.tv_sec = 0;
	nsTime.tv_nsec = 1000 * usec;
	nanosleep(&nsTime, NULL);
}

VOID RtmpOsMsDelay(
	IN	ULONG	msec)
{
	struct timespec nsTime;
	nsTime.tv_sec = 0;
	nsTime.tv_nsec = 1000000 * msec;
	nanosleep(&nsTime, NULL);
}
#endif

#if 0
long RtmpMSleep(IN ULONG mSec)
{
    struct timespec ntp, otp;
 
    ntp.tv_sec = 0;
    ntp.tv_nsec = mSec * 1000;
 
    nanosleep(&ntp, &otp);
 
    return((unsigned)otp.tv_sec);	
}
#endif

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *NowTime)
{
	NowTime->u.HighPart = 0;
	time((time_t *)&NowTime->u.LowPart);
}


void RTMP_GetCurrentSystemTick(ULONG *NowTime)
{
	UINT32 time_tmp;
	Ntime_tmp = 0;
	time((time_t *)&time_tmp);
	*(NowTime) = time_tmp;
}

/*******************************************************************************

	Flash Read/Write related functions.
	
 *******************************************************************************/
#ifdef RTMP_RBUS_SUPPORT
#ifndef CONFIG_RALINK_FLASH_API
void FlashWrite(UCHAR *p, ULONG offset, ULONG size)
{
#if 0
	sysFlashSet(p, size, offset + RTMP_FLASH_BASE_ADDR);
#endif
}


void FlashRead(UCHAR *p, ULONG a, ULONG b)
{
	UCHAR	*pPtr, *pAddr;
	ULONG 	offset;
	
	pAddr = (UCHAR *)(RTMP_FLASH_BASE_ADDR + a);
	pPtr = p;
	for(offset =0 ; offset < b; offset++, pAddr++, pPtr++)
	{
		*pPtr = *pAddr;
	}

//	hex_dump("FlashRead", p, b);//Carter comment

}
#endif /* CONFIG_RALINK_FLASH_API */
#endif /* RTMP_RBUS_SUPPORT */

/*******************************************************************************

	Normal memory allocation related functions.
	
 *******************************************************************************/
/* pAd MUST allow to be NULL */
NDIS_STATUS os_alloc_mem(
	IN	VOID *pAd,
	OUT	UCHAR **mem,
	IN	ULONG  size,
	IN	UCHAR  mem_idx)
{	
#if 1
	if (size <= 256)
	{
		*mem = (PUCHAR) OS_SDMP_MALLOC_ID(size, mem_idx);

		if (*mem == NULL)
		{
			DBG_ERR("allocate from mcb failure! now use system mem, size %d\n", size);
			
			*mem = (PUCHAR) kmalloc(size, 0);			
		}
	}
	else
#endif
		*mem = (PUCHAR) kmalloc(size, 0);
	
	if (INT_CONTEXT())
		logMsg("%s(): kmalloc memory pointer=0x%x!\n", __FUNCTION__, (unsigned long)(*mem),0,0,0,0);
	
	if (*mem)
		return (NDIS_STATUS_SUCCESS);
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("malloc failed size = %d\n", size));
		return (NDIS_STATUS_FAILURE);
	}
}

NDIS_STATUS os_alloc_mem_suspend(
	IN	VOID					*pReserved,
	OUT	UCHAR					**mem,
	IN	ULONG					size)
{
	return os_alloc_mem(pReserved, mem, size,0x2a);
}

/* pAd MUST allow to be NULL */
NDIS_STATUS os_free_mem(
	IN	VOID *pAd,
	IN	PVOID mem)
{
	
	ASSERT(mem);

	if (OS_SDMP_MCB_MEM_CHECK(mem))
	{
		if (OS_SDMP_MCB_MEM_INUSE_CHECK(mem))
		{
			OS_SDMP_FREE(mem);
		}
	} 
	else
	{
		kfree(mem);
	}
	return (NDIS_STATUS_SUCCESS);
}


/*******************************************************************************

	Device IRQ related functions.
	
 *******************************************************************************/
int RtmpOSIRQRequest(IN PNET_DEV pNetDev)
{
	return 0; 
}


int RtmpOSIRQRelease(
	IN	PNET_DEV				pNetDev,
	IN	UINT32					infType,
	IN	PPCI_DEV				pci_dev,
	IN	BOOLEAN					*pHaveMsi)
{
	return 0;
}


/*******************************************************************************

	File open/close related functions.
	
 *******************************************************************************/
RTMP_OS_FD RtmpOSFileOpen(char *pPath,  int flag, int mode)
{
	return open(pPath, flag, mode);
}

int RtmpOSFileClose(RTMP_OS_FD osfd)
{
	return close(osfd);
}


void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset)
{
	lseek(osfd, offset, SEEK_SET);
}


int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen)
{
	return read(osfd, pDataPtr, readLen);
}


int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen)
{
	return write(osfd, pDataPtr, writeLen);
}


void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfo, BOOLEAN bSet)
{
	return;
}



/*******************************************************************************

	Task create/management/kill related functions.
	
 *******************************************************************************/
#ifndef NET_TASK_PRIORITY
#define NET_TASK_PRIORITY       50
#endif

//#define NET_TASK_PRIORITY       51

NDIS_STATUS RtmpOSTaskAttach(
	IN RTMP_OS_TASK *pTask, 
	IN RTMP_OS_TASK_CALLBACK fn, 
	IN ULONG arg)
{
	NDIS_STATUS status;
	
	pTask->taskPID = taskSpawn(pTask->taskName, 51, RTMP_OS_MGMT_TASK_FLAGS, 4096, 
					(FUNCPTR)fn, (int)arg,  0, 0, 0, 0, 0, 0, 0, 0, 0);
	if (pTask->taskPID == ERROR) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Attach task(%s) failed!\n", pTask->taskName));
		pTask->taskPID = -1;
		status = NDIS_STATUS_FAILURE;
	}
	else
		status = NDIS_STATUS_SUCCESS;
	
	return status;
}

NDIS_STATUS RtmpOSTaskInit(
	IN RTMP_OS_TASK *pTask,
	PSTRING	pTaskName,
	VOID *pPriv,
	LIST_HEADER				*pTaskList,
	LIST_HEADER				*pSemList)
{
	int len;
	
	/*pTask = kmalloc(sizeof(RTMP_OS_TASK), GFP_KERNEL); */
	ASSERT(pTask);

	NdisZeroMemory((PUCHAR)(pTask), sizeof(RTMP_OS_TASK));

	len = strlen(pTaskName);
	len = len > (RTMP_OS_TASK_NAME_LEN -1) ? (RTMP_OS_TASK_NAME_LEN-1) : len;
	NdisMoveMemory(&pTask->taskName[0], pTaskName, len); 
	
	RTMP_SEM_EVENT_INIT_LOCKED(&(pTask->taskSema), &pAd->RscSemMemList);

	pTask->priv = pPriv;
	
	/* TODO: For vxWorks, we need to implement a wait_completion mechanism to make sure task running successfully. */
	/*init_completion (&pTask->taskComplete); */
	
	return NDIS_STATUS_SUCCESS;
}


INT RtmpOSTaskNotifyToExit(
	IN RTMP_OS_TASK *pTask)
{
	/* TODO: Shall we do any customization for vxWorks tasks? */
	return 0;
}


void RtmpOSTaskCustomize(
	IN RTMP_OS_TASK *pTask)
{
	/* TODO: Shall we do any customization for vxWorks tasks? */
	return;
}


NDIS_STATUS RtmpOSTaskKill(
	IN RTMP_OS_TASK *pTask)
{
	STATUS retVal;
	
	RTMP_SEM_EVENT_DESTORY(&pTask->taskSema);

	if (0 != pTask->taskPID)
	{
		retVal = taskDelete(pTask->taskPID);
	}

	pTask->task_killed = 0;
	
	return NDIS_STATUS_SUCCESS;
}


#ifdef VXWORKS_5X
M_BLK_ID RtmpVxNetTupleGet(
	NET_POOL_ID		pNetPool,	/* pointer to the net pool */
	int			bufSize,	/* size of the buffer to get */
	int			canWait,	/* wait or don't wait */
	UCHAR		type,		/* type of data */
	BOOL		bestFit		/* TRUE/FALSE */
)
{
	M_BLK_ID pMblk;
	int actualBufSize;

	actualBufSize = bufSize + NETBUF_LEADING_SPACE;
	pMblk = netTupleGet(pNetPool, actualBufSize, canWait, type, /*bestFit*/ FALSE);
	if (pMblk)
	{
		pMblk->mBlkHdr.mData  = (caddr_t)(pMblk->pClBlk->clNode.pClBuf + NETBUF_LEADING_SPACE);
		NdisZeroMemory(pMblk->pClBlk->clNode.pClBuf, CB_MAX_OFFSET);
	}
	else
	{
#if 1
		CL_POOL 		*pClPool;
		pClPool = pNetPool->clTbl[0];
		if (INT_CONTEXT())
		{
			logMsg("int context, pClPool->clNumFree=%d, clSize=%d, netBufSize=%d! errno=0x%x, MEMORY=0x%x, M_netBufLib=0x%x!\n", 
					pClPool->clNumFree, pClPool->clSize, bufSize,errnoGet(),S_netBufLib_NO_POOL_MEMORY,M_netBufLib);
		}
		else
		{
			/*
			printf("pClPool->clNumFree=%d, clSize=%d, netBufSize=%d! errno=0x%x, MEMORY=0x%x, M_netBufLib=0x%x!\n", 
					pClPool->clNumFree, pClPool->clSize, bufSize,errno,S_netBufLib_NO_POOL_MEMORY,M_netBufLib);
			*/
		}
#endif
	}
	return pMblk;
	
}
#endif /* VXWORKS_5X */

#ifdef VXWORKS_6X
M_BLK_ID RtmpVxNetTupleGet(
	NET_POOL_ID		pNetPool,	/* pointer to the net pool */
	int			bufSize,	/* size of the buffer to get */
	int			canWait,	/* wait or don't wait */
	UCHAR		type,		/* type of data */
	BOOL		noSmaller	/* TRUE/FALSE */
)
{
	return netTupleGet(pNetPool, bufSize, canWait, type, noSmaller);
}

#endif /* VXWORKS_6X */


PNDIS_PACKET RtmpOSNetPktAlloc(
	IN VOID 		*pAdSrc,
	IN int size)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	struct mBlk *pMblk;
	POS_COOKIE pObj = NULL;
	
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	/* Add 2 more bytes for ip header alignment*/
	pMblk = RtmpVxNetTupleGet(pObj->pNetPool[RTMP_NETPOOL_TX], (size+2), M_DONTWAIT, MT_DATA, 1);

#ifdef MBLK_DEBUG	
	if (pMblk != NULL)
	{
		pAd->tx_alloc_count++;
		((M_BLK_ID)pMblk)->pClBlk->clFreeArg1 = 60;	
	}
	else
	{
		pAd->tx_alloc_fail_count++;	
	}
#endif

	return ((PNDIS_PACKET)pMblk);
}


PNDIS_PACKET RTMP_AllocateFragPacketBuffer(
	IN VOID 		*pAdSrc,
	IN	ULONG	Length)
{
	struct mBlk *pMblk;
	POS_COOKIE	pObj;

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	pMblk = RtmpVxNetTupleGet(pObj->pNetPool[RTMP_NETPOOL_RX], Length, M_DONTWAIT, MT_DATA, 1);
	
	if (pMblk)
	{
		//RTMP_SET_PACKET_SOURCE(OSPKT_TO_RTPKT(pMblk), PKTSRC_NDIS);
		//DBGPRINT(RT_DEBUG_ERROR, ("allocate pMblk(0x%x) %ld size packet\n",pMblk,Length));

	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("can't allocate frag rx %ld size packet\n",Length));
	}

	return (PNDIS_PACKET) pMblk;
	
}


/* The allocated NDIS PACKET must be freed via RTMPFreeNdisPacket() for TX */
NDIS_STATUS RTMPAllocateNdisPacket(
	IN	VOID			*pAdSrc,
	OUT PNDIS_PACKET	*ppPacket,
	IN	PUCHAR			pHeader,
	IN	UINT			HeaderLen,
	IN	PUCHAR			pData,
	IN	UINT			DataLen)
{
	//DBGPRINT(RT_DEBUG_ERROR, ("-->RTMPAllocateNdisPacket\n"));
	PNDIS_PACKET	pPacket;
	POS_COOKIE pObj = NULL;
	PRTMP_ADAPTER		pAd = (PRTMP_ADAPTER)pAdSrc;
	ASSERT(pData);
	ASSERT(DataLen);

	pObj = (POS_COOKIE)pAd->OS_Cookie;

	pPacket = (PNDIS_PACKET) RtmpVxNetTupleGet(pObj->pNetPool[RTMP_NETPOOL_TX], HeaderLen + DataLen, M_DONTWAIT, MT_DATA, 1);
	if (pPacket == NULL)
	{
		*ppPacket = NULL;
		my_printf("RTMPAllocateNdisPacket Fail\n\n");

#ifdef MBLK_DEBUG
		pAd->tx_alloc_fail_count++;	
#endif
		return NDIS_STATUS_FAILURE;
	}

	/* 2. clone the frame content */

	/* Clone the frame content and update the length of packet */
	if ((HeaderLen > 0) && (pHeader != NULL))
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket), pHeader, HeaderLen);

	if ((DataLen > 0) && (pData != NULL))
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket) + HeaderLen, pData, DataLen);

#if 0	
	if (HeaderLen > 0)
	{
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket), pHeader, HeaderLen);
	}
	if (DataLen > 0)
	{
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket) + HeaderLen, pData, DataLen);
	}
#endif
	/* 3. update length of packet */
	GET_OS_PKT_LEN(pPacket) = (HeaderLen + DataLen);

	//RTMP_SET_PACKET_SOURCE(pPacket, PKTSRC_NDIS);
/*	DBGPRINT(RT_DEBUG_TRACE, ("%s : pPacket = %p, len = %d\n", */
/*				__FUNCTION__, pPacket, GET_OS_PKT_LEN(pPacket))); */
	*ppPacket = pPacket;
	//DBGPRINT(RT_DEBUG_ERROR, ("<--RTMPAllocateNdisPacket\n"));

#ifdef MBLK_DEBUG
	pAd->tx_alloc_count++;
	((M_BLK_ID)pPacket)->pClBlk->clFreeArg1 = 70;
#endif
	return NDIS_STATUS_SUCCESS;
}


/*
  ========================================================================
  Description:
	This routine frees a miniport internally allocated NDIS_PACKET and its
	corresponding NDIS_BUFFER and allocated memory.
  ========================================================================
*/
	
VOID RTMPFreeNdisPacket(
	IN VOID 		*pAdSrc,
	IN PNDIS_PACKET  pPacket)
{
	PRTMP_ADAPTER		pAd = (PRTMP_ADAPTER)pAdSrc;
	M_BLK_ID pMblk = (M_BLK_ID)pPacket;
	POS_COOKIE pObj;
	NET_POOL_ID pNetPool;
	
	pObj = (POS_COOKIE)(pAd->OS_Cookie);
	pNetPool = pMblk->pClBlk->pNetPool;

	/*if (pNetPool == pObj->pNetPool[RTMP_NETPOOL_RX]  || pNetPool == pObj->pNetPool[RTMP_NETPOOL_TX])
		pMblk->mBlkHdr.mNext = 0;*/
		/*去掉这部分，修复ping大包导致buffer耗尽的情况*/
#ifdef MBLK_DEBUG
	pMblk->pClBlk->clFreeArg1 = 0;
#endif	
#ifdef VXWORKS_5X
	SET_OS_PKT_NETDEV(pPacket, NULL);
#endif /* VXWORKS_5X */

#ifdef MBLK_DEBUG
	if (pNetPool == pObj->pNetPool[RTMP_NETPOOL_TX])
		pAd->tx_alloc_free_count++;	
#endif
	
	netMblkClChainFree(pMblk);

	pMblk=NULL;
}


/* IRQL = DISPATCH_LEVEL */
/* NOTE: we do have an assumption here, that Byte0 and Byte1 always reasid at the same */
/*			 scatter gather buffer */
NDIS_STATUS Sniff2BytesFromNdisBuffer(
	IN	PNDIS_BUFFER	pFirstBuffer,
	IN	UCHAR			DesiredOffset,
	OUT PUCHAR			pByte0,
	OUT PUCHAR			pByte1)
{
    *pByte0 = *(PUCHAR)(pFirstBuffer + DesiredOffset);
    *pByte1 = *(PUCHAR)(pFirstBuffer + DesiredOffset + 1);

	return NDIS_STATUS_SUCCESS;
}


void * m_data_put(struct mBlk *skb, int n)
{
	PUCHAR p;

	ASSERT(skb);

	p = GET_OS_PKT_DATATAIL(skb);
	skb->m_len += n;
		
	/* TODO: should we check if skb->m_data + n > end of buffer?? */
	
	return p;
}


void * m_data_push(struct mBlk *pMblkBuf, int n)
{
	ASSERT(pMblkBuf);

	pMblkBuf->mBlkHdr.mData -= n;
	pMblkBuf->mBlkHdr.mLen += n;
	pMblkBuf->mBlkPktHdr.len += n;
		
	return pMblkBuf->m_data;
}


PNDIS_PACKET skb_copy(PNDIS_PACKET pSrcPkt, INT flags)
{
	M_BLK_ID pNewMblk, pOldMblk;
	struct netPool *pNetPool;
	struct end_object *pEndObj;
	int bufLen;
	RTMP_ADAPTER *pAd;
	POS_COOKIE	pObj;


	
	
	pOldMblk = (M_BLK_ID)pSrcPkt;
	pEndObj = (END_OBJ *)(pOldMblk->mBlkPktHdr.rcvif);
	if (pEndObj == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): pEndObj is NULL!\n", __FUNCTION__));
		return NULL;
	}

	pAd =  (RTMP_ADAPTER *)(RTMP_OS_NETDEV_GET_PRIV(pEndObj));

	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): pAd is NULL!\n", __FUNCTION__));
		return NULL;	
	}

	pObj = (POS_COOKIE)pAd->OS_Cookie;

	pNetPool = pObj->pNetPool[RTMP_NETPOOL_TX];
	
	bufLen = ((pOldMblk->mBlkPktHdr.len > pOldMblk->mBlkHdr.mLen) ? pOldMblk->mBlkPktHdr.len : pOldMblk->mBlkHdr.mLen);
	pNewMblk = RtmpVxNetTupleGet(pNetPool, bufLen /*pOldMblk->mBlkHdr.mLen*/, M_DONTWAIT, MT_DATA, 1);
	if (pNewMblk == NULL)
	{
		//END_ERR_ADD(pEndObj, MIB2_OUT_ERRS, +1);
#ifdef MBLK_DEBUG
		pAd->skb_copy_err++;
		pAd->skb_copy_fail_count++;	
		if (pAd->skb_copy_err == 8)
		{		
			//DBGPRINT(RT_DEBUG_ERROR, ("%s Tx: cannot get skb_copy mblk\n", RTMP_OS_NETDEV_GET_DEVNAME(pEndObj)));
			my_printf("%s Tx: cannot get skb_copy mblk\n", RTMP_OS_NETDEV_GET_DEVNAME(pEndObj));
			pAd->skb_copy_err=0;
		}
#endif
		return NULL;
	}

	bufLen = netMblkToBufCopy (pOldMblk, pNewMblk->mBlkHdr.mData, NULL);
	ASSERT((pOldMblk->mBlkPktHdr.len == bufLen));
	
	pNewMblk->mBlkHdr.mLen 	= bufLen;
	pNewMblk->mBlkHdr.mType = pOldMblk->mBlkHdr.mType;
	pNewMblk->mBlkHdr.mFlags = pOldMblk->mBlkHdr.mFlags;
	pNewMblk->mBlkPktHdr.len = pOldMblk->mBlkPktHdr.len;
	SET_OS_PKT_NETDEV(pNewMblk, pEndObj);

#ifdef MBLK_DEBUG
	((M_BLK_ID)pNewMblk)->pClBlk->clFreeArg1 = 72;
	pAd->tx_alloc_count++;
#endif	
	
	return (PNDIS_PACKET)pNewMblk;
		
}


#if 0 // yiwei replace with copy!
PNDIS_PACKET skb_clone(PNDIS_PACKET pSrcPkt, int flags)
{
	M_BLK_ID pNewMblk, pOldMblk;
	struct netPool *pNetPool;
	/*END_OBJ  *pEndObj; */

	pOldMblk = (M_BLK_ID)pSrcPkt;
	pNetPool = MBLK_TO_NET_POOL(pOldMblk);

	pNewMblk = NULL;
	if (pNetPool)
	{
		pNewMblk = netMblkGet (pNetPool,  M_DONTWAIT, MT_DATA);
	if (pNewMblk == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): get Mblk failed!\n", __FUNCTION__));
		return NULL;
	}

		if (NULL == netMblkDup(pOldMblk, pNewMblk))
	{
			netMblkFree (pNetPool, pNewMblk);
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): call netMblkDup() failed!\n", __FUNCTION__));
		return NULL;
	}

		if (pNewMblk != NULL)	
			((M_BLK_ID)pNewMblk)->pClBlk->clFreeArg1 = 84;	
	}
	return pNewMblk;
}
#endif

void RTMP_QueryPacketInfo(
	IN  PNDIS_PACKET pPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen)
{
	pPacketInfo->BufferCount = 1;
	pPacketInfo->pFirstBuffer = GET_OS_PKT_DATAPTR(pPacket);
	pPacketInfo->PhysicalBufferCount = 1;
	pPacketInfo->TotalPacketLength = GET_OS_PKT_LEN(pPacket);

	*pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
	*pSrcBufLen = GET_OS_PKT_LEN(pPacket); 
}

#if 0
void RTMP_QueryNextPacketInfo(
	IN  PNDIS_PACKET *ppPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen)
{
    /* Shiang: Not used now!! */
}
#endif /* Unused */

/* TODO: need to check is there any diffence between following four functions???? */
PNDIS_PACKET DuplicatePacket(
/*	IN	PRTMP_ADAPTER	pAd, */
	IN	PNET_DEV		pNetDev,
	IN	PNDIS_PACKET	pPacket)
{
	/*struct mBlk	*pMblkPkt; */
	PNDIS_PACKET	pRetPacket = NULL;
	USHORT			DataSize;
	UCHAR			*pData;

	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);
	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);	


	pRetPacket = skb_clone(RTPKT_TO_OSPKT(pPacket), MEM_ALLOC_FLAG);
	if (pRetPacket)
		SET_OS_PKT_NETDEV(pRetPacket, pNetDev);

	/*DBGPRINT(RT_DEBUG_TRACE, ("%s(): pRetPacket = 0x%x!\n", __FUNCTION__, (unsigned int)pRetPacket)); */
	
	return pRetPacket;
}

#if 0
PNDIS_PACKET duplicate_pkt(
	IN	PNET_DEV		pNetDev,
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID)
{
	M_BLK_ID	pMblk;
	POS_COOKIE		pObj;
	PNDIS_PACKET	pPacket = NULL;
	struct end_object *pEndObj;
	
	pMblk = RtmpVxNetTupleGet(pObj->pNetPool[RTMP_NETPOOL_TX], HdrLen + DataSize + 2, M_DONTWAIT, MT_DATA, 1);
	if (pMblk != NULL)
	{
		skb_reserve(pMblk, 2);				
		NdisMoveMemory(pMblk->m_data, pHeader802_3, HdrLen);
		skb_put(pMblk, HdrLen);
		NdisMoveMemory((pMblk->m_data + pMblk->m_len), pData, DataSize);
		skb_put(pMblk, DataSize);
		pEndObj= (struct end_object *)pNetDev; /*get_netdev_from_bssid(pAd, FromWhichBSSID); */
		SET_OS_PKT_NETDEV(pMblk, pEndObj);
		pPacket = OSPKT_TO_RTPKT(pMblk);
}

	return pPacket;
}
#endif

#define TKIP_TX_MIC_SIZE		8
PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(
	IN	VOID			*pAdSrc,
	IN	PNDIS_PACKET	pPacket)
{
	M_BLK_ID		pMblk, pNewMblk;
	int 				bufLen, tailRoom;
	POS_COOKIE		pObj;
	struct end_object *pEndObj;

	PRTMP_ADAPTER		pAd = (PRTMP_ADAPTER)pAdSrc;
	pObj = pAd->OS_Cookie;
	pMblk = RTPKT_TO_OSPKT(pPacket);
	pEndObj = GET_OS_PKT_NETDEV(pMblk);
	tailRoom = (pMblk->pClBlk->clNode.pClBuf + pMblk->pClBlk->clSize) - (pMblk->m_data + pMblk->m_len);
	if (tailRoom < TKIP_TX_MIC_SIZE)
	{
		bufLen = pMblk->m_len + TKIP_TX_MIC_SIZE;
		/* alloc a new skb and copy the packet */
		pNewMblk = RtmpVxNetTupleGet(pObj->pNetPool[RTMP_NETPOOL_TX], bufLen, M_DONTWAIT, MT_DATA, 1);
		if (pNewMblk == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Extend Tx.MIC for packet failed!, dropping packet!\n"));
#ifdef MBLK_DEBUG
			pAd->tx_alloc_fail_count++;	
#endif
			return NULL;
		}

		bufLen = netMblkToBufCopy(pMblk, pNewMblk->mBlkHdr.mData, NULL);
		ASSERT((pMblk->mBlkPktHdr.len == bufLen));
	
		pNewMblk->mBlkHdr.mLen 	= bufLen;
		pNewMblk->mBlkHdr.mType = pMblk->mBlkHdr.mType;
		pNewMblk->mBlkHdr.mFlags = pMblk->mBlkHdr.mFlags;
		pNewMblk->mBlkPktHdr.len = pMblk->mBlkPktHdr.len;
		SET_OS_PKT_NETDEV(pNewMblk, pEndObj);

		/* release old packet and assign the pointer to the new one */
		RTMPFreeNdisPacket(pAd, pMblk);
		pMblk = pNewMblk;
	}

#ifdef MBLK_DEBUG
	((M_BLK_ID)pPacket)->pClBlk->clFreeArg1 = 74;
	pAd->tx_alloc_count++;	
#endif
	
	return OSPKT_TO_RTPKT(pMblk);
}


#ifdef CONFIG_AP_SUPPORT
UCHAR VLAN_8023_Header_Copy(
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	OUT PUCHAR pData,
	IN UCHAR *TPID)
{
	UINT16 TCI;
	UCHAR VLAN_Size = 0;


	if (VLAN_VID != 0)
	{
		/* need to insert VLAN tag */
		VLAN_Size = LENGTH_802_1Q;

		/* make up TCI field */
		TCI = (VLAN_VID & 0x0fff) | ((VLAN_Priority & 0x7)<<13);

#ifndef RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif /* RT_BIG_ENDIAN */

		/* copy dst + src MAC (12B) */
		memcpy(pData, pHeader802_3, LENGTH_802_3_NO_TYPE);

		/* copy VLAN tag (4B) */
		/* do NOT use memcpy to speed up */
		*(UINT16 *)(pData+LENGTH_802_3_NO_TYPE) = *(UINT16 *)TPID;
		*(UINT16 *)(pData+LENGTH_802_3_NO_TYPE+2) = TCI;

		/* copy type/len (2B) */
		*(UINT16 *)(pData+LENGTH_802_3_NO_TYPE+LENGTH_802_1Q) = \
				*(UINT16 *)&pHeader802_3[LENGTH_802_3-LENGTH_802_3_TYPE];

		/* copy tail if exist */
		if (HdrLen > LENGTH_802_3)
		{
			memcpy(pData+LENGTH_802_3+LENGTH_802_1Q,
					pHeader802_3+LENGTH_802_3,
					HdrLen - LENGTH_802_3);
		} /* End of if */
	}
	else
	{
		/* no VLAN tag is needed to insert */
		memcpy(pData, pHeader802_3, HdrLen);
	} /* End of if */

	return VLAN_Size;
}
#ifdef VXWORKS
PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR wdev_idx)
{
	PNET_DEV dev_p = NULL;

	if (wdev_idx < WDEV_NUM_MAX)
		dev_p = pAd->wdev_list[wdev_idx]->if_dev;

	ASSERT((dev_p != NULL));
	return dev_p;
}
#else
PNET_DEV get_netdev_from_bssid(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			FromWhichBSSID)
{
	PNET_DEV dev_p = NULL;

	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		UCHAR infRealIdx;

		infRealIdx = FromWhichBSSID & (NET_DEVICE_REAL_IDX_MASK);
#ifdef APCLI_SUPPORT
		if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_APCLI)
		{
			dev_p = (infRealIdx > MAX_APCLI_NUM ? NULL : pAd->ApCfg.ApCliTab[infRealIdx].wdev.if_dev);
		} 
		else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
		if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS)
		{
			dev_p = ((infRealIdx > MAX_WDS_ENTRY) ? NULL : pAd->WdsTab.WdsEntry[infRealIdx].dev);
		}
		else
#endif /* WDS_SUPPORT */
		{
			if (FromWhichBSSID >= pAd->ApCfg.BssidNum)
	    		{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: fatal error ssid > ssid num!\n", __FUNCTION__));
				dev_p = pAd->net_dev;
	    		}

	    		if (FromWhichBSSID == BSS0)
				dev_p = pAd->net_dev;
	    		else
	    		{
	    	    		dev_p = pAd->ApCfg.MBSSID[FromWhichBSSID].wdev.if_dev;
	    		}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		dev_p = pAd->net_dev;
	}
#endif /* CONFIG_STA_SUPPORT */

	ASSERT(dev_p);
	return dev_p; /* return one of MBSS */
}
#endif

PNDIS_PACKET duplicate_pkt_with_VLAN(
	IN	VOID			*pReserved, 

	IN	USHORT			VLAN_VID,

	IN	USHORT			VLAN_Priority,
	IN	PUCHAR			pHeader802_3,
    	IN  	UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID,
	IN	UCHAR			*TPID)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pReserved;
	M_BLK_ID		pMblk;
	POS_COOKIE		pObj;
	PNDIS_PACKET	pPacket = NULL;
	UINT16			VLAN_Size;
	int bufLen;
	PUCHAR			pMdataPtr;
	END_OBJ 		*pEndObj;



	pObj = pAd->OS_Cookie;
	bufLen = HdrLen + DataSize + LENGTH_802_1Q + 2;
	pMblk = RtmpVxNetTupleGet(pObj->pNetPool[RTMP_NETPOOL_TX], bufLen, M_DONTWAIT, MT_DATA, 1);

	if (pMblk != NULL)
	{
		skb_reserve(pMblk, 2);

		/* copy header (maybe +VLAN tag) */
		VLAN_Size = VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
											pHeader802_3, HdrLen,
											pMblk->m_data, TPID);
		skb_put(pMblk, HdrLen + VLAN_Size);

		/* copy data body */
		pMdataPtr = GET_OS_PKT_DATATAIL(pMblk);
		NdisMoveMemory(pMdataPtr, pData, DataSize);
		skb_put(pMblk, DataSize);
		
		pEndObj = (END_OBJ *)get_netdev_from_bssid(pAd, FromWhichBSSID);
		SET_OS_PKT_NETDEV(pMblk, pEndObj);

		pPacket = OSPKT_TO_RTPKT(pMblk);
		
#ifdef MBLK_DEBUG
		pAd->tx_alloc_count++;
		((M_BLK_ID)pPacket)->pClBlk->clFreeArg1 = 76;
#endif		
	}
#ifdef MBLK_DEBUG
	else
	{
		pAd->tx_alloc_fail_count++;	
	}
#endif

	return pPacket;
}
#endif /* CONFIG_AP_SUPPORT */


PNDIS_PACKET ClonePacket(
	IN	PNET_DEV ndev, 
	IN	PNDIS_PACKET	pPacket,	
	IN	PUCHAR			pData,
	IN	ULONG			DataSize)
{
	M_BLK_ID	pClonedMblk, pOldMblk;
	struct netPool *pNetPool;
	END_OBJ  *pEndObj;
	//PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

	ASSERT(DataSize < 1530);	

	pEndObj = GET_OS_PKT_NETDEV(pPacket);
	pClonedMblk = skb_clone(pPacket, MEM_ALLOC_FLAG);
	if (pClonedMblk != NULL)
	{
	    	/* set the correct dataptr and data len */
	    	SET_OS_PKT_NETDEV(pClonedMblk, pEndObj);
		pClonedMblk->mBlkHdr.mData = pData;
		pClonedMblk->mBlkHdr.mLen = DataSize;
		pClonedMblk->mBlkPktHdr.len = DataSize;
	}
	
	return pClonedMblk;
	
}




	
void wlan_802_11_to_802_3_packet(
	IN	PNET_DEV		pNetDev,
	IN	VOID			*pAdSrc,
	IN	UCHAR			OpMode,
	IN	USHORT			VLAN_VID,
	IN	USHORT			VLAN_Priority,
	IN	PNDIS_PACKET			pRxPacket,
	IN	UCHAR			*pData,
	IN	ULONG			DataSize,
	IN	PUCHAR			pHeader802_3,
	IN  	UCHAR			FromWhichBSSID,
	IN	UCHAR					*TPID)
{
	//my_printf("-->wlan_802_11_to_802_3_packet opmode=%d\n",OpMode);
	struct mBlk	*pMblkPkt;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

	ASSERT(pRxPacket);
	ASSERT(pHeader802_3);

	pMblkPkt = RTPKT_TO_OSPKT(pRxPacket);
/*	pNetDev = get_netdev_from_bssid(pAd, FromWhichBSSID); */

	SET_OS_PKT_NETDEV(pMblkPkt, pNetDev);
	SET_OS_PKT_DATAPTR(pMblkPkt, pData);
	SET_OS_PKT_LEN(pMblkPkt, DataSize);
	
/*	DBGPRINT(RT_DEBUG_TRACE, ("pRxBlk->pData=0x%x, DataSize=%d!\n", pRxBlk->pData, pRxBlk->DataSize)); */
/*	hex_dump("pRxBlk", pRxBlk->pData, pRxBlk->DataSize); */
	pMblkPkt->mBlkHdr.mData = pData;
	pMblkPkt->mBlkHdr.mLen = pMblkPkt->mBlkPktHdr.len = DataSize;
	pMblkPkt->mBlkHdr.mFlags |= (M_PKTHDR | M_EXT);
	pMblkPkt->mBlkHdr.mType = MT_DATA;
	pMblkPkt->m_next = NULL;
	
		
	/* */
	/* copy 802.3 header */
	/* */
	/* */
#ifdef CONFIG_AP_SUPPORT
/*	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
	if (OpMode == OPMODE_AP)
	{
		/* maybe insert VLAN tag to the received packet */
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;

		/* VLAN related */
		if (VLAN_VID != 0)
			VLAN_Size = LENGTH_802_1Q;
		
		data_p = skb_push(pMblkPkt, LENGTH_802_3+VLAN_Size);

		VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
								pHeader802_3, LENGTH_802_3,
								data_p, TPID);

	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(skb_push(pMblkPkt, LENGTH_802_3), pHeader802_3, LENGTH_802_3);
#endif /* CONFIG_STA_SUPPORT */
/*	DBGPRINT(RT_DEBUG_TRACE, ("After Header Copy: pMblkPkt->mData=0x%x, mLen=%d, len=%d!\n", */
/*				pMblkPkt->mBlkHdr.mData, pMblkPkt->mBlkHdr.mLen, pMblkPkt->mBlkPktHdr.len)); */
/*	hex_dump("pMblkPkt", pMblkPkt->mBlkHdr.mData, pMblkPkt->mBlkHdr.mLen); */

}


void announce_802_3_packet(
        IN VOID *pAdSrc,
        IN PNDIS_PACKET	pPacket,
        IN UCHAR OpMode)
{
	END_OBJ 	*pEndObj;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	UINT8 unit = 0;

	UCHAR	tmp[] = {0x0c,0x43};
	UCHAR	Broadcast[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	
	pEndObj  = GET_OS_PKT_NETDEV(pPacket);
	
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{

		if (MATPktRxNeedConvert(pAd, pEndObj))
			MATEngineRxHandle(pAd, pPacket, 0);

	}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE)
			MATEngineRxHandle(pAd, pPacket, 0);
	}
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	/*hex_dump_mBlk(__FUNCTION__, (M_BLK_ID)pPacket, TRUE); */
#ifdef VXWORKS_5X
	SET_OS_PKT_NETDEV(pPacket, NULL);
#endif /* VXWORKS_5X */

#ifdef CACHE_DMA_SUPPORT
	memcpy(GET_OS_PKT_HEAD(pPacket), CACHE_DMA_SIGNAUTURE, sizeof(CACHE_DMA_SIGNAUTURE));
#endif /* CACHE_DMA_SUPPORT */

		if (RTMPCompareMemory(GET_OS_PKT_DATAPTR(pPacket)+12,tmp,2)==0)
			NdisCopyMemory(GET_OS_PKT_DATAPTR(pPacket), Broadcast, 6);
		//my_printf("-->END_RCV_RTN_CALL pEndObj=0x%x\n ",pEndObj);

	if (RTMP_OS_NETDEV_STATE_RUNNING(pEndObj))
	{
		/* wireless packet. */
		MBLK_FLAG_SET((M_BLK_ID)pPacket, M_WIRELESS);

		unit = ENDOBJ_GET_UNIT(pEndObj);

		/* guest network or iptv packet TAG */
		switch (unit)
		{
		case UNIT_GUESTNETWORK:			/* guest network */
			MBLK_FLAG_SET((M_BLK_ID)pPacket, M_GUESTNETWORK);
			break;
		case UNIT_IPTV:			/* iptv */
			break;
		default:
			break;
		}
#ifdef MBLK_DEBUG
		((M_BLK_ID)pPacket)->pClBlk->clFreeArg1 = 0xFE;
#endif
		END_RCV_RTN_CALL(pEndObj, pPacket);
	}
	else
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}


PRTMP_SCATTER_GATHER_LIST
rt_get_sg_list_from_packet(PNDIS_PACKET pPacket, RTMP_SCATTER_GATHER_LIST *sg)
{
    /*Shiang: Not used now!!!! */
    return NULL;
}

#ifdef SYSTEM_LOG_SUPPORT
/* Shiang: This function is Linux specific feature and should we mapping to a VxWorks-style function??? */
VOID RTMPSendWirelessEvent(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Event_flag,
	IN	PUCHAR 			pAddr,
	IN	UCHAR			BssIdx,
	IN	CHAR			Rssi)
{
	if (pAd->CommonCfg.bWirelessEvent == FALSE)
		return;
}
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID SendSignalToDaemon(
	IN INT		sig,
	IN RTMP_OS_PID	pid,
	IN unsigned long pid_no)
{
	return;
}

/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
	TRUE - send successfully
	FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN Dot1xInternalCmdAction(
	IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN	UINT8			cmd)
{
	INT				apidx = MAIN_MBSSID;
	UCHAR 			Header802_3[14];
	UCHAR 			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	UCHAR			s_addr[MAC_ADDR_LEN];
	POS_COOKIE		pObj = NULL;

	pObj = (POS_COOKIE)pAd->OS_Cookie;

	INT size = sizeof(Header802_3) + sizeof(RalinkIe);
	struct mBlk *skb = RtmpVxNetTupleGet(pObj->pNetPool[RTMP_NETPOOL_TX], (size+2), M_DONTWAIT, MT_DATA, 1);

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	
	if (pEntry)
	{
		apidx = pEntry->apidx;
		NdisMoveMemory(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	}
	else
	{
		/* Fake a Source Address for transmission */
		NdisMoveMemory(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LENGTH);
		s_addr[0] |= 0x80;
	}

	MAKE_802_3_HEADER(Header802_3, 
					  pAd->ApCfg.MBSSID[apidx].wdev.bssid, 
					  s_addr, EAPOL);
		
	if (!skb)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Error! Can't allocate a skb.\n"));
#ifdef MBLK_DEBUG
		pAd->tx_alloc_fail_count++;	
#endif
		return FALSE;
	}
	
	SET_OS_PKT_NETDEV(skb, get_netdev_from_bssid(pAd, apidx));

	skb_reserve(skb, 2);	/* 16 byte align the IP header */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), Header802_3, LENGTH_802_3);
	
	/* Prepare internal command */
	RalinkIe[5] = cmd;
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb) + LENGTH_802_3, RalinkIe, sizeof(RalinkIe));

	skb_put(GET_OS_PKT_TYPE(skb), size);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : cmd(%d)\n", __FUNCTION__, cmd));
	
#ifdef MBLK_DEBUG
	((M_BLK_ID)skb)->pClBlk->clFreeArg1 = 82;
	pAd->tx_alloc_count++;
#endif

	announce_802_3_packet(pAd, skb, OPMODE_AP);

	return TRUE;
}

/*
 ========================================================================
 Routine Description:
	Send Leyer 2 Frame to trigger 802.1x EAP state machine.     

 Arguments:

 Return Value:
	TRUE - send successfully
	FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN Dot1xEapTriggerAction(
	IN  PRTMP_ADAPTER	pAd,
	IN  MAC_TABLE_ENTRY *pEntry)
{	
	return TRUE;
}	    

#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
void STA_MonPktSend(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk)
{

}
#endif /* CONFIG_STA_SUPPORT */


void RTMP_IndicateMediaState(	
	IN	PRTMP_ADAPTER		pAd,
	IN  NDIS_MEDIA_STATE	media_state)
{


}

static UINT32 RtmpOSWirelessEventTranslate(IN UINT32 eventType) {
	switch (eventType) {
		
	case RT_WLAN_EVENT_CUSTOM:
		eventType = IWEVCUSTOM;
		break;

	case RT_WLAN_EVENT_EXPIRED:
		eventType = IWEVEXPIRED;
		break;

	default:
		printk("Unknown event: 0x%x\n", eventType);
		break;
	}

	return eventType;
}

int RtmpOSWrielessEventSend(
	IN PNET_DEV	pNetDev,
	IN UINT32		eventType,
	IN INT			flags,
	IN PUCHAR		pSrcMac,
	IN PUCHAR		pData,
	IN UINT32		dataLen)
{
#if 1
	PRTMP_ADAPTER pAd;
	POS_COOKIE	pObj;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pNetDev);
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	eventType = RtmpOSWirelessEventTranslate(eventType);
	printk("===>RtmpOSWrielessEventSend eventType=0x%x\n",eventType);
	if (eventType == IWEVCUSTOM)
	{
		switch(flags)
		{
#ifdef WSC_AP_SUPPORT
			case RT_WSC_UPNP_EVENT_FLAG:
				if (pObj->WscMsgCallBack)
					pObj->WscMsgCallBack(pData, dataLen);
				break;
#endif /* WSC_AP_SUPPORT */
			default:
				break;
		}
	}
#endif
	return 0;
}


/*
  *	Assign the network dev name for created Ralink WiFi interface.
  */
static int RtmpOSNetDevRequestName(
	IN PSTRING pPrefixStr, 
	IN INT	devIdx,
	OUT INT	*avilNum)
{
	int	ifNameIdx;
	int	Status;
	
	for (ifNameIdx = devIdx; ifNameIdx < 32; ifNameIdx++)
	{
		if (muxDevExists(pPrefixStr, ifNameIdx) == FALSE)
			break;
	}
	
	if(ifNameIdx < 32)
	{
		*avilNum = ifNameIdx;
#ifdef RELEASE_EXCLUDE
		DBGPRINT(RT_DEBUG_INFO, ("Assign the net device name as %s%d\n", pPrefixStr, *avilNum));
#endif /* RELEASE_EXCLUDE */
		Status = NDIS_STATUS_SUCCESS;
	} 
	else 
	{
		DBGPRINT(RT_DEBUG_ERROR, 
					("Cannot request DevName with preifx(%s) and in range(0~32) as suffix from OS!\n", pPrefixStr));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


void RtmpOSNetDevClose(
	IN PNET_DEV pNetDev)
{

}


void RtmpOSNetDevFree(PNET_DEV pNetDev)
{
	ASSERT(pNetDev);
	muxDevUnload(pNetDev->devObject.name, pNetDev->devObject.unit);
	kfree(pNetDev);
}


int RtmpOSNetDevAddrSet(
	IN UCHAR	OpMode,
	IN PNET_DEV pNetDev,
	IN PUCHAR	pMacAddr,
	IN PUCHAR	dev_name)
{
	 bcopy (pMacAddr, (char *)(&pNetDev->mib2Tbl.ifPhysAddress.phyAddress), MAC_ADDR_LEN);
	 pNetDev->mib2Tbl.ifPhysAddress.addrLength = MAC_ADDR_LEN;
	 
	return 0;
}

void RtmpOSNetDevProtect(BOOLEAN lock_it)
{
	return;
}

void RtmpOSNetDevDetach(
	IN PNET_DEV pNetDev)
{
	void *pCookie = (void *)(pNetDev);
	muxDevStop(pCookie);
}

int RtmpOSNetDevAttach(
	IN UCHAR					OpMode,
	IN PNET_DEV					pNetDev, 
	IN RTMP_OS_NETDEV_OP_HOOK	*pDevOpHook)
{
	END_OBJ *pEndObj;
	pEndObj = (END_OBJ *)pNetDev;
	RtmpOSNetDevAddrSet(OpMode, pNetDev, &pDevOpHook->devAddr[0], NULL);
	muxDevStart(pNetDev);
	/*END_FLAGS_SET (pEndObj, IFF_UP | IFF_RUNNING);*/
	return NDIS_STATUS_SUCCESS;
}


VOID	RTMPFreeAdapter(
	IN	VOID		*pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	
	os_cookie=(POS_COOKIE)pAd->OS_Cookie;
	free(pAd);
	free(os_cookie);
}


/*
	VxWorks sepecific functions for creations of EndDevice and realted procedures.
*/
int RtmpENDDevInitStrParse(char *initString, RTMP_VX_INIT_STRING *pInitToken)
{
	int count = 0;
	char     *tok, *pPtr = NULL; 
	int 		*pTokenValue = NULL;
	

	/*
		initString format:
			phyNum:devNum:infType:irqNum:devType:privSize:devName
	*/
	tok = strtok_r(initString, ":", &pPtr);
	pTokenValue = (int *)pInitToken;
	while(tok)
	{
		//printf("tok=%s! count=%d!\n", tok, count);
		if (count == 7)
			memcpy(pInitToken->devName, tok, strlen(tok));
		else
			*(pTokenValue + count) = atoi(tok);
		tok = strtok_r (NULL, ":", &pPtr);
		count++;
	}

	//printf("count=%d!\n", count);
	if (count != 8)
		return NDIS_STATUS_FAILURE;

	printf("The initString info:\n");
	printf("\tphyNum=%d!\n", pInitToken->phyNum);
	printf("\tdevNum=%d!\n", pInitToken->devNum);
	printf("\tinfType=%d!\n", pInitToken->infType);
	printf("\tirqNum=%d!\n", pInitToken->irqNum);
	printf("\tdevType=%d!\n", pInitToken->devType);
	printf("\tprivSize=%d!\n", pInitToken->privSize);
	printf("\tcsrAddr=0x%x!\n", pInitToken->csrAddr);
	printf("\tdevName=%s!\n", pInitToken->devName);
	
	return NDIS_STATUS_SUCCESS;
	
}


END_OBJ *RtmpVirtualENDDevCreate(
	IN char *initString, 
	IN void *pBSP)
{
	END_OBJ *pENDDev;
	NET_FUNCS *pEndFuncTb;
	RTMP_VX_INIT_STRING initToken;
	int status;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pBSP;
	/*void 			*pPrivPtr; */
	char				devName[IFNAMSIZ];
	int i;
	DRV_CTRL 	*pDrvCtrl;


	DBGPRINT(RT_DEBUG_TRACE,("%s(): Into phase 2, received initSting=%s!\n", __FUNCTION__, initString));
	memset((unsigned char *)&initToken, 0, sizeof(RTMP_VX_INIT_STRING));
	status = RtmpENDDevInitStrParse(initString, &initToken);
	if(status == NDIS_STATUS_FAILURE)
	{
		printf("Parsing the initString failed!\n");
        	return NULL;
	}

	if (initToken.devType == INT_MBSSID)
	{
		pDrvCtrl = (DRV_CTRL *)(&g_wlan_mbss_drvCtrl[initToken.devNum]);
		bzero((char *)pDrvCtrl, sizeof(WLAN_DRV_CTRL)); 

	      pDrvCtrl->unit = initToken.devNum;
		
	    sprintf(pDrvCtrl->name, "%s", "ra");
	    sprintf(pDrvCtrl->fullName, "%s%d", "ra", initToken.devNum);

	    pENDDev = (END_OBJ *)&pDrvCtrl->endObj;
	}

	if (pENDDev == NULL)
	{
		pENDDev = (END_OBJ *)malloc(sizeof(END_OBJ));
		if (pENDDev == NULL) 
		{
			printf("Could not alloc memory for device structures\n");
	        	return NULL;
		}
	}
	NdisZeroMemory(pENDDev, sizeof(END_OBJ));
	pENDDev->pNetPool = pAd->net_dev->pNetPool;

	switch(initToken.devType)
	{
#ifdef MBSS_SUPPORT
		case INT_MBSSID:
			pEndFuncTb = &RtmpMBSSEndFuncTable;
			break;
#endif /* MBSS_SUPPORT */
#ifdef WDS_SUPPORT
		case INT_WDS:
			pEndFuncTb = &RtmpWDSEndFuncTable;
			break;
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			pEndFuncTb = &RtmpApCliEndFuncTable;
			break;
#endif /* APCLI_SUPPORT */
#ifdef MESH_SUPPORT
		case INT_MESH:
			pEndFuncTb = &RtmpMeshEndFuncTable;
			break;
#endif /* MESH_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR: Not supported device type(%d)!\n", initToken.devType));
			goto err;
	}


	pDrvCtrl->loaded = TRUE;   
    	pDrvCtrl->started  = FALSE;
		
	sprintf(devName, "%s%d", &initToken.devName[0], initToken.phyNum);
	//printf("%s(): devName=%s!\n", __FUNCTION__, devName);
	if ((END_OBJ_INIT(pENDDev, (DEV_OBJ *)pAd, &initToken.devName[0],
					initToken.phyNum, pEndFuncTb, &devName[0]) == ERROR)) 
	{
		printf("Can not init net device %s!\n", &devName[0]);
		goto err;
	}

	if ((END_MIB_INIT(pENDDev, M2_ifType_ethernet_csmacd,
	              &ZERO_MAC_ADDR[0],  6, ETHERMTU, END_SPEED) == ERROR))
	{
		printf("Can not init MIB of net device %s!\n", RTMP_OS_NETDEV_GET_DEVNAME(pENDDev));
		return NULL;
	}

	/*END_OBJ_READY(pENDDev, IFF_NOTRAILERS |IFF_SIMPLEX |IFF_BROADCAST |IFF_ALLMULTI); */
	END_OBJ_READY(pENDDev, 	IFF_NOTRAILERS |IFF_BROADCAST |IFF_MULTICAST);

	/*pDrvCtrl->ifType = IF_TYPE_WLAN;
 	pDrvCtrl->pOwner = NULL;	*/	/* not belong to any bridge or bridge 
									by default */
										
	//printf("<---%s()!\n", __FUNCTION__);
	
	return pENDDev;

err:
	if (pENDDev)
		free(pENDDev);
	return NULL;
	
}


#ifdef MESH_SUPPORT
END_OBJ *RtmpMeshENDDevLoad(
	IN char *initString, 
	IN void *pBSP)
{

	ASSERT(initString);
	
	/*  First pass, we just return the device name for function "muxDevLoad()" to check if the device exists */
	if (initString[0] == '\0') {
		bcopy(INF_MESH_DEV_NAME, initString, strlen(INF_MESH_DEV_NAME));
		printf("%s():Into phase 1, return initSting=%s!\n", __FUNCTION__, initString);
		return NULL;
	}

	/*   Now we are phase 2, now do END_OBJ related initialization */
	return RtmpVirtualENDDevCreate(initString, pBSP);
	
}
#endif /* MESH_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#if 0
END_OBJ *RtmpApCliENDDevLoad(
	IN char *initString, 
	IN void *pBSP)
{

	ASSERT(initString);
	
	/*  First pass, we just return the device name for function "muxDevLoad()" to check if the device exists */
	if (initString[0] == '\0') {
		bcopy(INF_APCLI_DEV_NAME, initString, strlen(INF_APCLI_DEV_NAME));
		//printf("%s():Into phase 1, return initSting=%s!\n", __FUNCTION__, initString);
		return NULL;
	}

	/*   Now we are phase 2, now do END_OBJ related initialization */
	return RtmpVirtualENDDevCreate(initString, pBSP);
}
#endif 
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
END_OBJ *RtmpWDSENDDevLoad(
	IN char *initString, 
	IN void *pBSP)
{
	ASSERT(initString);
	
	/*  First pass, we just return the device name for function "muxDevLoad()" to check if the device exists */
	if (initString[0] == '\0') {
		bcopy(INF_WDS_DEV_NAME, initString, strlen(INF_WDS_DEV_NAME));
		printf("%s():Into phase 1, return initSting=%s!\n", __FUNCTION__, initString);
		return NULL;
	}

	/*   Now we are phase 2, now do END_OBJ related initialization */
	return RtmpVirtualENDDevCreate(initString, pBSP);
}
#endif /* WDS_SUPPORT */


#ifdef MBSS_SUPPORT
END_OBJ *RtmpMBSSENDDevLoad(
	IN char *pInitString, 
	IN void *pBSP)
{	
	ASSERT(pInitString);
	
	/*  First pass, we just return the device name for function "muxDevLoad()" to check if the device exists */
	if (pInitString[0] == '\0') {
		bcopy(INF_MAIN_DEV_NAME, pInitString, strlen(INF_MAIN_DEV_NAME));
		DBGPRINT(RT_DEBUG_TRACE, ("%s():Into phase 1, return pInitString=%s!\n", __FUNCTION__, pInitString));
		return NULL;
	}

	/*   Now we are phase 2, now do END_OBJ related initialization */
	return RtmpVirtualENDDevCreate(pInitString, pBSP);
}
#endif /* MBSS_SUPPORT */


#ifdef APCLI_SUPPORT
END_OBJ *RtmpApCliENDDevLoad(char *initString, void *pBSP)
{
	PNET_DEV				pENDDev = NULL;
	END_OBJ					*pEnd = NULL;
	RTMP_ADAPTER 			*pAd = NULL;
	APCLI_STRUCT			*pApCliEntry;
	DRV_CTRL 				*pDrvCtrl;
	int 					unitNum = 0;
	int 					i = 0;
	POS_COOKIE				pObj = NULL;
	
	if (initString == NULL)
	{
		printf("ERROR: null initString!\n");
		return NULL;
	}
	
	/*  First pass, we just return the device name for function "muxDevLoad()" to check if the device exists */
	if (initString[0] == '\0') {
		bcopy(INF_APCLI_DEV_NAME, initString, strlen(INF_APCLI_DEV_NAME));
		//printf("%s():Into phase 1, return initSting=%s!\n", __FUNCTION__, initString);
		return NULL;
	}
	
	pAd = (PRTMP_ADAPTER)pBSP;

	pDrvCtrl = (DRV_CTRL *)&apCliDryCtrl;
	bzero((char *)pDrvCtrl, sizeof(WLAN_DRV_CTRL));

	pDrvCtrl->unit = 0;
	sprintf(pDrvCtrl->name, "%s", INF_APCLI_DEV_NAME);
    sprintf(pDrvCtrl->fullName, "%s%d", INF_APCLI_DEV_NAME, unitNum);

	pENDDev = (PNET_DEV)&pDrvCtrl->endObj;
	if (pENDDev == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, (" malloc failed for main physical net device!\n"));
		goto err_out_free_radev;
	}

	pDrvCtrl->loaded = TRUE;   
    pDrvCtrl->started  = FALSE;	
	/*pDrvCtrl->ifType = IF_TYPE_APC;
 	pDrvCtrl->pOwner = NULL;*/		/* not belong to any bridge or bridge 
									by default */
	pApCliEntry = &pAd->ApCfg.ApCliTab[0];
	if (pApCliEntry != NULL)
	{
		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(pApCliEntry->wdev.if_addr, pAd->CurrentAddress);
	}
	else
	{
		printk("cli entry is NULL!\n");
	}
	pApCliEntry->wdev.if_dev= (PNET_DEV)pENDDev;
#if 0 //woody
	if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
	{
		if (pAd->ApCfg.BssidNum > 0 || MAX_MESH_NUM > 0) 
		{
			/* 	
				Refer to HW definition - 
					Bit1 of MAC address Byte0 is local administration bit 
					and should be set to 1 in extended multiple BSSIDs'
					Bit3~ of MAC address Byte0 is extended multiple BSSID index.
			 */ 
			pApCliEntry->wdev.if_add[0] += 2; 	
			//pApCliEntry->CurrentAddress[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
			pApCliEntry->wdev.if_add[0] = (pApCliEntry->wdev.if_add[0] & 0xe3) 
											| ((((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2) 
											^ (pApCliEntry->wdev.if_add[0] & 0x1c));
		}
	}
	else
	{
	pApCliEntry->wdev.if_add[ETH_LENGTH_OF_ADDRESS - 1] =
		(pApCliEntry->wdev.if_add[ETH_LENGTH_OF_ADDRESS - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;
	}
#endif	
	if ((END_OBJ_INIT((END_OBJ*)pENDDev, (DEV_OBJ *)pAd, pDrvCtrl->name,
	              	   0, &RtmpApCliEndFuncTable, "apcli0") == ERROR)) 
	{
		printf("Can not init net device %s!\n", "apcli0");
		return NULL;
	}
#if 0
	printf("\napcli mac addr ---- ");
	for (i = 0; i < 6; ++i)
	{
		printf("%02X:", pApCliEntry->CurrentAddress[i]);
	}
	printf("\n");
#endif	
    if (END_MIB_INIT((END_OBJ*)pENDDev, M2_ifType_ethernet_csmacd, &pApCliEntry->wdev.if_addr[0], 
					  MAC_ADDR_LEN, ETHERMTU, END_SPEED) == ERROR)
    {
		printf("Can not init mib net device %s!\n", "apcli0");
		return NULL;
    }

	bcopy ((char *)pDrvCtrl->endObj.mib2Tbl.ifPhysAddress.phyAddress, 
		(char *)pDrvCtrl->enetAddr, pDrvCtrl->endObj.mib2Tbl.ifPhysAddress.addrLength);

	END_OBJ_READY((END_OBJ*)pENDDev, IFF_NOTRAILERS |IFF_BROADCAST |IFF_MULTICAST);

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	((END_OBJ*)pENDDev)->pNetPool = pObj->pNetPool[RTMP_NETPOOL_TX];

#ifdef WSC_AP_SUPPORT
	pApCliEntry->WscControl.pAd = pAd;        
	if (pApCliEntry->WscControl.WscEnrolleePinCode == 0)
		pApCliEntry->WscControl.WscEnrolleePinCode = GenerateWpsPinCode(pAd, TRUE, 0);
	NdisZeroMemory(pApCliEntry->WscControl.EntryAddr, MAC_ADDR_LEN);
    WscInit(pAd, TRUE, unitNum);
#endif /* WSC_AP_SUPPORT */
	
	pAd->flg_apcli_init = TRUE;

	/* 
	 * add to g_ifCtl_freeIfList by default 
	 */								

	return pENDDev;

err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	printf("\n\nerr_out_free_radev!!!\n\n");
	if (pAd)
	RTMPFreeAdapter(pAd);
	
}
#endif

#endif /* CONFIG_AP_SUPPORT */

/*
PNET_DEV RtmpOSNetDevCreate(
	IN RTMP_ADAPTER *pAd,
	IN INT 			devType, 
	IN INT			devNum,
	IN INT			privMemSize,
	IN PSTRING		pNamePrefix)
*/
PNET_DEV RtmpOSNetDevCreate(
	IN VOID *data,
	IN INT32					MC_RowID,
	IN UINT32					*pIoctlIF,
	IN INT 						devType,
	IN INT						devNum,
	IN INT						privMemSize,
	IN PSTRING					pNamePrefix)
{
#if 1
	END_OBJ 		*pNetDev = NULL;
	RTMP_ADAPTER 			*pAd = NULL;
	pAd = (PRTMP_ADAPTER)data;
	struct os_cookie	*pObj = pAd->OS_Cookie;
	char        		devInitStr[32];
	int				status, availableNum;
	void 			*endLoadFunc;
	char				devName[IFNAMSIZ];


	memset(devInitStr, 0, 32);
	memset(devName, 0, IFNAMSIZ);
	switch(devType)
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
		case INT_MBSSID:
			endLoadFunc = RtmpMBSSENDDevLoad;
			break;
#endif /* MBSS_SUPPORT */
#ifdef WDS_SUPPORT
		case INT_WDS:
			endLoadFunc = RtmpWDSENDDevLoad;
			break;
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			endLoadFunc = RtmpApCliENDDevLoad;
			break;
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef MESH_SUPPORT
		case INT_MESH:
			endLoadFunc = RtmpMeshENDDevLoad;
			break;
#endif /* MESH_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR: Not supported device type(%d)!\n", devType));
			return NULL;
	}


	/*
		Prepare the devInitStr
		initString format:
			phyNum:devNum:infType:irqNum:devType:privSize:devName
	*/
	memset(&devInitStr[0], 0, sizeof(devInitStr));
	sprintf(devInitStr, "%d:%d:%d:%d:%d:0:", devNum,4, 0, devType, privMemSize);

	ASSERT((strlen(pNamePrefix) < IFNAMSIZ));
	strcpy(devName, pNamePrefix);
#ifdef MULTIPLE_CARD_SUPPORT
	if (pAd->MC_RowID >= 0)
	{
		char suffixName[4];
		sprintf(suffixName, "%02d_", pAd->MC_RowID);
		strcat(devName, suffixName);
	}
#endif /* MULTIPLE_CARD_SUPPORT */

	ASSERT(((strlen(devInitStr) + strlen(devName)) < 32));
	strcat(devInitStr, devName);
	status = RtmpOSNetDevRequestName(devName, devNum, &availableNum);
	if (status == NDIS_STATUS_FAILURE)
	{
		printf("%s(): request devNum failed!\n", __FUNCTION__);
		return NULL;
	}

	printf("availableNum(%d) devInitStr =%s\n",availableNum,devInitStr);
	/*  Find the endLoadFunc depends on the namePrefix */
	/* allocate a new network device */
	//printf("%s(): Prepare to call muxDevLoad, the devInitStr=%s, unitNum=%d, availableNum=%d\n", 
	//		__FUNCTION__, devInitStr, devNum, availableNum);
	if ((pNetDev = muxDevLoad(availableNum, endLoadFunc, devInitStr, FALSE, pAd)) == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("Error in create END_OBJ(%s%d) with available num=%d!\n", 
									devInitStr, devNum, availableNum));
		return NULL;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("The name of the new %s interface is %s, unit=%d...\n", devInitStr, pNetDev->devObject.name, pNetDev->devObject.unit));
	
	return pNetDev;
#endif
}


/*
 * This function is going to be called by MuxLoad through endDevTbl which is
 * defined in configNet.h. That header file also defines the init string.
 */
END_OBJ *sysRtmpEndLoad(char *initString, void *arg)
{
	END_OBJ					*pENDDev = NULL;
	int						status;
	RTMP_VX_INIT_STRING		initToken;
	char defInitString[] = "0:0:4:25:0:0:2684354560:ra";

	if (initString == NULL)
	{
		printf("ERROR: null initString!\n");
		return NULL;
	}

	/* if the strlen(initString) is zero, it's phase one, return default device name prefix */
	if (initString[0] == EOS)
	{
	  bcopy ((char *)INF_MAIN_DEV_NAME, initString, sizeof(INF_MAIN_DEV_NAME));
		/*printf("First phase! the return iniString=%s!\n", initString); */
		return NULL;
	}

	/* strlen(initString) is not zero, it's phase two, parsing the string and do initiailization. */
	memset(&initToken, 0, sizeof(RTMP_VX_INIT_STRING));
	status = RtmpENDDevInitStrParse(defInitString, &initToken);
	if (status == NDIS_STATUS_FAILURE)
	{
		printf("initString format wrong!str=%s!\n", initString);
		return NULL;
	}

	//initToken.csrAddr

	switch(initToken.infType)
	{
#ifdef RTMP_PCI_SUPPORT
#if 0
		case RTMP_DEV_INF_PCI:
				pENDDev = RtmpVxPciEndLoad(&initToken);
				break;
#endif
#endif /* RTMP_PCI_SUPPORT */
#ifdef RTMP_USB_SUPPORT
		case RTMP_DEV_INF_USB:
				pENDDev = RtmpVxUsbEndLoad(&initToken);
				break;
#endif /* RTMP_USB_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT
		case RTMP_DEV_INF_RBUS:
				pENDDev = RtmpVxRbusEndLoad(&initToken);
				break;
#endif /* RTMP_RBUS_SUPPORT */
		default:
				printf("Un-supported interface type:%d!\n", initToken.infType);
				break;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== sysRtmpEndLoad(), pENDDev=0x%x!\n", pENDDev));
	
	return pENDDev;
	
}


STATUS sysRtmpEndUnLoad(END_OBJ *pEndDev)
{	
	RTMP_ADAPTER *pAd;
	STATUS retval;
	
/* Unregister network device */
	if (pEndDev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("sysRtmpEndUnLoad(): pEndDev is NULL!\n"));
		return ERROR;
	}

	END_OBJECT_UNLOAD(pEndDev);
	DBGPRINT(RT_DEBUG_TRACE, ("sysRtmpEndUnLoad(): (), dev->name=%s!\n", pEndDev->devObject.name));
	pAd = RTMP_OS_NETDEV_GET_PRIV(pEndDev);
	
	if (pAd != NULL)
	{	
		retval = EALREADY;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("sysRtmpEndUnLoad(): (), pAd is NULL!\n"));
		retval = ERROR;
	}
  	
	return retval;

}

/* */
/* change OS packet DataPtr and DataLen */
/* */
VOID RtmpOsPktInit(
	IN	PNDIS_PACKET		pRxPacket,
	IN	PNET_DEV			pNetDev,
	IN	UCHAR				*pData,
	IN	USHORT				DataSize)
{
	struct mBlk	*pMblkPkt;
/*	PNET_DEV	pNetDev; */

	ASSERT(pRxPacket);
	pMblkPkt = RTPKT_TO_OSPKT(pRxPacket);

/*	pNetDev = get_netdev_from_bssid(pAd, FromWhichBSSID); */
	SET_OS_PKT_NETDEV(pMblkPkt, pNetDev);
	pMblkPkt->mBlkHdr.mData = pData;
	pMblkPkt->mBlkHdr.mLen = DataSize;
	pMblkPkt->mBlkPktHdr.len  = DataSize;
	pMblkPkt->mBlkHdr.mFlags |= (M_PKTHDR | M_EXT);
	pMblkPkt->m_next = NULL;
}
#if 0
VOID RtmpOsPktInit(
	IN	PNDIS_PACKET		pNetPkt,
	IN	PNET_DEV			pNetDev,
	IN	UCHAR				*pData,
	IN	USHORT				DataSize)
{
	PNDIS_PACKET	pRxPkt;


	pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	SET_OS_PKT_NETDEV(pRxPkt, pNetDev);
	SET_OS_PKT_DATAPTR(pRxPkt, pData);
	SET_OS_PKT_LEN(pRxPkt, DataSize);
	SET_OS_PKT_DATATAIL(pRxPkt, pData, DataSize);
}
#endif
VOID RtmpUtilInit(VOID)
{
	return;
}

BOOLEAN RtmpOsStatsAlloc(
	IN	VOID					**ppStats,
	IN	VOID					**ppIwStats)
{
	return TRUE;
}

BOOLEAN RtmpOSTaskWait(
	IN	VOID					*pReserved,
	IN	RTMP_OS_TASK			*pTaskOrg,
	IN	INT32					*pStatus)
{
	RTMP_SEM_EVENT_WAIT(&(pTaskOrg->taskSema), *pStatus);
	if (*pStatus == 0)
		return TRUE;
	return FALSE;
}

INT RTMP_AP_IoctlPrepare(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pCB)
{
		return 0;
}

VOID RtmpOsTaskPidInit(
	IN	RTMP_OS_PID				*pPid)
{
	*pPid = THREAD_PID_INIT_VALUE;
}

VOID RtmpOsTaskWakeUp(
	IN	RTMP_OS_TASK			*pTask)
{
#ifdef KTHREAD_SUPPORT
	WAKE_UP(pTask);
#else
	RTMP_SEM_EVENT_UP(&pTask->taskSema);
#endif
}

/*
========================================================================
Routine Description:
	Enable or disable wireless event sent.

Arguments:
	pReserved		- Reserved
	FlgIsWEntSup	- TRUE or FALSE

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsWlanEventSet(
	IN	VOID					*pReserved,
	IN	BOOLEAN					*pCfgWEnt,
	IN	BOOLEAN					FlgIsWEntSup)
{
	*pCfgWEnt = FlgIsWEntSup;
}


#ifdef WDS_SUPPORT
VOID AP_WDS_KeyNameMakeUp(
	IN	STRING						*pKey,
	IN	UINT32						KeyMaxSize,
	IN	INT							KeyId)
{
	snprintf(pKey, KeyMaxSize, "Wds%dKey", KeyId);
}


NET_DEV_STATS *RT28xx_get_wds_ether_stats(
    IN PNET_DEV *net_dev)
{
    return NULL;
}
#endif /* WDS_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
VOID AP_E2PROM_IOCTL_PostCtrl(
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	PSTRING					msg)
{
#if 0
     if ((wrq->u.data.flags== RT_OID_802_11_HARDWARE_REGISTER) && (wrq->u.data.length >= strlen(msg))) {
		wrq->u.data.length = strlen(msg);
		copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	} else {
		DBGPRINT(RT_DEBUG_OFF, ("%s\n", msg));	
	}

	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
#endif	
}


VOID IAPP_L2_UpdatePostCtrl(
	IN PRTMP_ADAPTER	pAd,
    IN UINT8 *mac_p,
    IN INT  bssid)
{
}
#endif /* CONFIG_AP_SUPPORT */

/*
========================================================================
Routine Description:
	Wake up the command thread.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsCmdUp(
	IN RTMP_OS_TASK			*pCmdQTask)
{
#ifdef KTHREAD_SUPPORT
	do{
		OS_TASK	*pTask = RTMP_OS_TASK_GET(pCmdQTask);
		{
			pTask->kthread_running = TRUE;
	        wake_up(&pTask->kthread_q);
		}
	}while(0);
#else
	do{
		OS_TASK	*pTask = RTMP_OS_TASK_GET(pCmdQTask);
		CHECK_PID_LEGALITY(pTask->taskPID)
		{
			RTMP_SEM_EVENT_UP(&(pTask->taskSema));
		}
	}while(0);
#endif /* KTHREAD_SUPPORT */
}

#ifdef DOT1X_SUPPORT
/*
	========================================================================

	Routine Description:
		Send a L2 frame to upper daemon to trigger state machine

	Arguments:
		pAd - pointer to our pAdapter context

	Return Value:

	Note:

	========================================================================
*/
BOOLEAN RTMPL2FrameTxAction(
	IN VOID * pCtrlBkPtr,
	IN PNET_DEV pNetDev,
	IN RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet,
	IN UCHAR apidx,
	IN PUCHAR pData,
	IN UINT32 data_len,
	IN	UCHAR			OpMode)
{
		struct mBlk *skb;
#if 1
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pCtrlBkPtr;
	/* Add 2 more bytes for ip header alignment*/
	skb = RtmpVxNetTupleGet(pAd->net_dev->pNetPool, (data_len+2), M_DONTWAIT, MT_DATA, 1);
#endif
	if (!skb) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s : Error! Can't allocate a skb.\n", __FUNCTION__));
		return FALSE;
	}

	MEM_DBG_PKT_ALLOC_INC(skb);
	/*get_netdev_from_bssid(pAd, apidx)); */
	SET_OS_PKT_NETDEV(skb, pNetDev);

	/* 16 byte align the IP header */
	skb_reserve(skb, 2);

	/* Insert the frame content */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), pData, data_len);

	/* End this frame */
	skb_put(GET_OS_PKT_TYPE(skb), data_len);

	DBGPRINT(RT_DEBUG_TRACE, ("%s done\n", __FUNCTION__));

	_announce_802_3_packet(pCtrlBkPtr, skb, OpMode);

	return TRUE;

}
#endif /*DOT1X_SUPPORT*/

INT get_dev_config_idx(RTMP_ADAPTER *pAd)
{
	INT idx = 0;

		MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL,  DBG_LVL_TRACE, ("-->get_dev_config_idx\n"));

#if 0	
#if defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD)
	INT first_card = 0, second_card = 0;
	
	A2Hex(first_card, FIRST_CHIP_ID);
	A2Hex(second_card, SECOND_CHIP_ID);
	MTWF_LOG(DBG_CAT_ALL, DBG_LVL_TRACE, ("chip_id1=0x%x, chip_id2=0x%x, pAd->MACVersion=0x%x\n", first_card, second_card, pAd->MACVersion));

//	if ((pAd->MACVersion & chip_id) == CONFIG_RT_SECOND_CARD)
//		pAd->flash_offset = SECOND_RF_OFFSET;

	if (IS_RT8592(pAd))
		idx = 0;
	else if (IS_RT5392(pAd) || IS_MT76x0(pAd) || IS_MT76x2(pAd))
		idx = 1;
#endif /* defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD) */
#endif
	idx = 0;
	pAd->dev_idx = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--get_dev_config_idx\n"));
	return idx;
}


int Get_Wdev_idx_From_EndObject(IN END_OBJ * pEndObj)
{
	POS_COOKIE pObj;
	int infIdx, status = 0;
	RTMP_ADAPTER *pAd = NULL;
	
	if ( pEndObj == NULL)
		return -1;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pEndObj);
	

		for (infIdx = 0; infIdx < pAd->ApCfg.BssidNum; infIdx++)
		{
			if (pAd->ApCfg.MBSSID[infIdx].wdev.if_dev== pEndObj)
			{
				DBGPRINT(RT_DEBUG_INFO, ("GET_WDEV_FROM_ENDOBJECT (ra%d)\n", infIdx));
				break;
			}
		}

		if (infIdx < pAd->ApCfg.BssidNum)
		{
			return infIdx;
		} else {
			DBGPRINT(RT_DEBUG_INFO, ("GET_WDEV_FROM_ENDOBJECT (NULL)\n", infIdx));
			return -1;
		}
			
}	



UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map *map;
	INT type_idx = 0, dev_idx = get_dev_config_idx(pAd);

	do {
		map = &prefix_map[type_idx];
		if (map->type == dev_type) {
			MTWF_LOG(DBG_CAT_ALL,DBG_SUBCAT_ALL,  DBG_LVL_TRACE, ("%s(): dev_idx = %d, dev_name_prefix=%s\n",
						__FUNCTION__, dev_idx, map->prefix[dev_idx]));
			return map->prefix[dev_idx];
		}
		type_idx++;
	} while (prefix_map[type_idx].type != 0);

	return NULL;
}


void RtmpOsSpinLockIrqSave(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
		OS_INT_LOCK(lock, *flags);
}

void RtmpOsSpinUnlockIrqRestore(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
		OS_INT_UNLOCK(lock, *flags);
}


static inline void *netdev_priv(END_OBJ *pENDDev)
{
	return ((pENDDev)->devObject.pDevice);
}


/*
	Used for backward compatible with previous linux version which
	used "net_device->priv" as device driver structure hooking point
*/
static inline void netdev_priv_set( END_OBJ *pENDDev, void *priv)
{
	((pENDDev)->devObject.pDevice)= priv;

}

VOID RtmpOsSetNetDevPriv(VOID *pDev, VOID *pPriv)
{
	struct mt_dev_priv *priv_info = NULL;

	priv_info = ( struct mt_dev_priv *)netdev_priv(( END_OBJ *)pDev);

	priv_info->sys_handle = (VOID *)pPriv;
	priv_info->priv_flags = 0;
}

VOID *RtmpOsGetNetDevPriv(VOID *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv(( END_OBJ *)pDev))->sys_handle;
}


VOID RtmpOsSetNetDevWdev(VOID *net_dev, VOID *wdev)
{
	struct mt_dev_priv *priv_info;

	priv_info = ( struct mt_dev_priv *)netdev_priv(( END_OBJ *)net_dev);
	priv_info->wifi_dev = wdev;
}


VOID *RtmpOsGetNetDevWdev(VOID *pDev)
{
	return ((struct  mt_dev_priv *)netdev_priv(( END_OBJ *)pDev))->wifi_dev;
}

unsigned long RtmpOsWaitForCompletionTimeout(RTMP_OS_COMPLETION *pCompletion, unsigned long timeout)
{
	unsigned long time_expire;
	time_expire=0;
	time_expire=((timeout*1000)/OS_HZ);
#if 1
	LARGE_INTEGER Old, New;
	do {	
		if (taskIdSelf() == netTaskId )
		{
			RtmpOsMsDelay(1);
		} 
		else
			RtmpVxMsDelay(1);
		
		timeout --;

	}while (!pCompletion->done && timeout > 0);

	return timeout;
#endif	
}

void woody (void)
{
			my_printf("++++++++++++++++++++++++FUCK+++++++++++++++++++++++++++++++++++\n");
}


unsigned long RTMPMsecsToJiffies(UINT32 m)
{

	unsigned long timeout;
	timeout = ((m * OS_HZ) / 1000);
	return timeout;
}

void wlanDrvStart(END_OBJ *pEnd)
{	
	RTMP_ADAPTER *pAd = NULL;
	
	UINT unit = 0;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	PNET_DEV pMbssEnd = NULL;
	struct wifi_dev *wdev = NULL;
	
	RTMP_OS_NETDEV_OP_HOOK apCliOpHook;
	struct wifi_dev *pApCliWdev = NULL;
	PNET_DEV pApCliEnd = NULL;
	
    if (pEnd != NULL)
	{
		/*sleep(2);*/
        muxDevStart(pEnd);
    }

	pAd = RTMP_OS_NETDEV_GET_PRIV(pEnd);

	if (TRUE == pAd->ApCfg.MBSSID[MAIN_MBSSID].bcn_buf.bBcnSntReq)
	{
		RTMP_OS_NETDEV_START_QUEUE(pEnd);
	}
	else
	{
		RTMP_OS_NETDEV_STOP_QUEUE(pEnd);
	}	

	for (unit = FIRST_MBSSID; unit < pAd->ApCfg.BssidNum; unit++)
	{
		wdev = &pAd->ApCfg.MBSSID[unit].wdev;
		pMbssEnd = wdev->if_dev;
		if (NULL == pMbssEnd)
		{
			DBG_ERR("end of ssid %d is null\n", unit);
			continue;
		}
		
		NdisZeroMemory(&netDevHook, sizeof(netDevHook));

		netDevHook.open = MBSS_VirtualIF_Open;	/* device opem hook point */
		netDevHook.stop = MBSS_VirtualIF_Close;	/* device close hook point */	
		netDevHook.xmit = MBSS_VirtualIF_PacketSend;	/* hard transmit hook point */
		netDevHook.ioctl = MBSS_VirtualIF_Ioctl;	/* ioctl hook point */

		netDevHook.priv_flags = INT_MBSSID;
		netDevHook.needProtcted = TRUE;
		netDevHook.wdev = wdev;

		/* Init MAC address of virtual network interface */
		NdisMoveMemory(&netDevHook.devAddr[0], &wdev->bssid[0], MAC_ADDR_LEN);

		/* register this device to OS */
		RtmpOSNetDevAttach(pAd->OpMode, pMbssEnd, &netDevHook);

		DBG_INFO("ssid(%d), addr :"__MAC__", bssid : "__MAC__"\n", unit, MAC_PRINT(wdev->if_addr), MAC_PRINT(wdev->bssid));

		if (TRUE == pAd->ApCfg.MBSSID[unit].bcn_buf.bBcnSntReq)
		{
			RTMP_OS_NETDEV_START_QUEUE(pMbssEnd);			
		}
		else
		{
			RTMP_OS_NETDEV_STOP_QUEUE(pMbssEnd);
		}
	}

	pApCliWdev = &pAd->ApCfg.ApCliTab[0].wdev;
	pApCliEnd = pApCliWdev->if_dev;

	if (NULL == pApCliEnd)
	{
		DBG_ERR("end of apcli is null\n");
		return;
	}
	
	apCliOpHook.priv_flags = INT_APCLI; /* we are virtual interface */
	apCliOpHook.needProtcted = TRUE;
	apCliOpHook.wdev = pApCliWdev;
	NdisMoveMemory(&apCliOpHook.devAddr[0], &pApCliWdev->if_addr[0], MAC_ADDR_LEN);

	/* register this device to OS */
	RtmpOSNetDevAttach(pAd->OpMode, pApCliEnd, &apCliOpHook);
	/*RTMP_OS_NETDEV_START_QUEUE(pApCliEnd);
	bridgePortAdd(END_DEV_NAME(pApCliEnd), 0, LAN_BRIDGE_INDEX);*/

	qadInit();
}

VOID VX_APCli_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
#define APCLI_MAX_DEV_NUM	32
	PNET_DEV new_dev_p;
	INT idx;
	APCLI_STRUCT *pApCliEntry;
	struct wifi_dev *wdev;
#ifdef VXWORKS
	struct mt_dev_priv	*priv_info = NULL;
#endif

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_apcli_init != FALSE)
		return;


	/* init */
	for(idx = 0; idx < MAX_APCLI_NUM; idx++)
		pAd->ApCfg.ApCliTab[idx].wdev.if_dev = NULL;

	/* create virtual network interface */
	for (idx = 0; idx < MAX_APCLI_NUM; idx++)
	{
		UINT32 MC_RowID = 0, IoctlIF = 0;
		char *dev_name;

#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		dev_name = get_dev_name_prefix(pAd, INT_APCLI);
#ifdef VXWORKS
		new_dev_p = RtmpOSNetDevCreate(pAd, MC_RowID, &IoctlIF, INT_APCLI, idx, sizeof(PRTMP_ADAPTER), INF_APCLI_DEV_NAME);
#else
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_APCLI, idx,
									sizeof(struct mt_dev_priv), dev_name);
#endif


		if (!new_dev_p) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Create net_device for %s(%d) fail!\n",
						__FUNCTION__, dev_name, idx));
			break;
		}
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
		wdev = &pApCliEntry->wdev;
		wdev->wdev_type = WDEV_TYPE_STA;
		wdev->func_dev = pApCliEntry;
		wdev->func_idx = idx;
		wdev->sys_handle = (void *)pAd;
		wdev->if_dev = new_dev_p;
		wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
		// TODO: shiang-usw, modify this to STASendPacket!
		wdev->tx_pkt_handle = APSendPacket;
		wdev->wdev_hard_tx = APHardTransmit;
		wdev->rx_pkt_allowed = sta_rx_pkt_allow;
		wdev->rx_pkt_foward = sta_rx_fwd_hnd;

#ifdef VXWORKS
		priv_info = (struct mt_dev_priv *) kmalloc(sizeof(struct mt_dev_priv), GFP_ATOMIC);
		((new_dev_p)->devObject.pDevice)= priv_info;
#endif
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
						RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);

#ifdef MT_MAC
		if (pAd->chipCap.hif_type != HIF_MT)
		{
#endif /* MT_MAC */
			if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
			{
				if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0))
				{
					UCHAR MacMask = 0;

					if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
						MacMask = 0xFE;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
						MacMask = 0xFC;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
						MacMask = 0xF8;

					/*
						Refer to HW definition -
							Bit1 of MAC address Byte0 is local administration bit
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					*/
					if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
					{
						/*
							Refer to HW definition -
								Bit1 of MAC address Byte0 is local administration bit
								and should be set to 1 in extended multiple BSSIDs'
								Bit3~ of MAC address Byte0 is extended multiple BSSID index.
						*/
#ifdef ENHANCE_NEW_MBSSID_MODE
						wdev->if_addr[0] &= (MacMask << 2);
#endif /* ENHANCE_NEW_MBSSID_MODE */
						wdev->if_addr[0] |= 0x2;
						wdev->if_addr[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
					}
#ifdef ENHANCE_NEW_MBSSID_MODE
					else
					{
						wdev->if_addr[0] |= 0x2;
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += ((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
					}
#endif /* ENHANCE_NEW_MBSSID_MODE */
				}
			}
			else
			{
				wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;
			}
#ifdef MT_MAC
		}
		else {
			volatile UINT32 Value;
			UCHAR MacByte = 0;

			//TODO: shall we make choosing which byte to be selectable???
			Value = 0x00000000L;
			RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);//read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.
			Value = Value | (0x2 << 29);//Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address
			RTMP_IO_WRITE32(pAd, LPON_BTEIR, Value);
			MacByte = Value >> 29;


			//Carter, I make apcli interface use HWBSSID1 to go.
			//so fill own_mac and BSSID here.
			wdev->if_addr[0] |= 0x2;

			switch (MacByte) {
				case 0x1: /* choose bit[23:20]*/
					wdev->if_addr[2] = (wdev->if_addr[2] = wdev->if_addr[2] & 0x0f);
					break;
				case 0x2: /* choose bit[31:28]*/
					wdev->if_addr[3] = (wdev->if_addr[3] = wdev->if_addr[3] & 0x0f);
					break;
				case 0x3: /* choose bit[39:36]*/
					wdev->if_addr[4] = (wdev->if_addr[4] = wdev->if_addr[4] & 0x0f);
					break;
				case 0x4: /* choose bit [47:44]*/
					wdev->if_addr[5] = (wdev->if_addr[5] = wdev->if_addr[5] & 0x0f);
					break;
				default: /* choose bit[15:12]*/
					wdev->if_addr[1] = (wdev->if_addr[1] = wdev->if_addr[1] & 0x0f);
					break;
			}

//			AsicSetDevMac(pAd, wdev->if_addr, 0x1);//set own_mac to HWBSSID1
			//AsicSetBssid(pAd, wdev->if_addr, 0x1);
		}
#endif /* MT_MAC */

#ifdef LINUX
		pNetDevOps->priv_flags = INT_APCLI; /* we are virtual interface */
		pNetDevOps->needProtcted = TRUE;
		pNetDevOps->wdev = wdev;
		NdisMoveMemory(pNetDevOps->devAddr, &wdev->if_addr[0], MAC_ADDR_LEN);

		/* register this device to OS */
		RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
#endif
	}
#ifdef VXWORKS
	ApCliDetectAttach(pAd);
#endif
	pAd->flg_apcli_init = TRUE;

}

STATUS os_wlanInit(UINT32 devno)
{
	RTMP_ADAPTER *pAd = NULL;
	void *pCookie = NULL;
	UCHAR mac[MAC_ADDR_LEN] = {0};
	INT unitNum = 0;
	INT maxBssNum = 0;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	RTMP_OS_NETDEV_OP_HOOK apCliOpHook;
	
 	pCookie = muxDevLoad(0,
                         sysRtmpEndLoad,
                         "0:1:19:0:0:2684354560:ra",
                         FALSE, NULL);

	if (NULL == pCookie)
	{
		DBG_ERR("end load failed!\n");
		return ERROR;
	}

	if (OK != macGet(mac, MAC_ADDR_SIZE, OS_LAN_UNIT))
	{
		DBG_ERR("lac mac get failure\n");
	}

	pAd = RTMP_OS_NETDEV_GET_PRIV((END_OBJ *)pCookie);
	pAd->lock = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);

	/* init BSSID */
	COPY_MAC_ADDR(pAd->CurrentAddress, mac);
	pAd->bLocalAdminMAC = TRUE;

	/* init multi-ssid */
	maxBssNum= pAd->ApCfg.BssidNum;
	if (maxBssNum > MAX_MBSSID_NUM(pAd))
	{
		maxBssNum = MAX_MBSSID_NUM(pAd);
	}
	
	NdisZeroMemory(&netDevHook, sizeof(netDevHook));
	netDevHook.open = MBSS_VirtualIF_Open;	/* device opem hook point */
	netDevHook.stop = MBSS_VirtualIF_Close;	/* device close hook point */	
	netDevHook.xmit = MBSS_VirtualIF_PacketSend;	/* hard transmit hook point */
	netDevHook.ioctl = MBSS_VirtualIF_Ioctl;	/* ioctl hook point */

	for (unitNum = FIRST_MBSSID; unitNum < maxBssNum; unitNum++)
	{
		VX_MBSS_Init(pAd, &netDevHook, unitNum);
	}

	/* init apcli */
	NdisZeroMemory(&apCliOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	apCliOpHook.open = ApCli_VirtualIF_Open;
	apCliOpHook.stop = ApCli_VirtualIF_Close;
	apCliOpHook.xmit = ApCli_VirtualIF_PacketSend;
	apCliOpHook.ioctl = ApCli_VirtualIF_Ioctl;

	VX_APCli_Init(pAd, &apCliOpHook);

	return OK;	
}

STATUS os_wlanStart(RTMP_ADAPTER *pAd, UINT32 unit)
{
	END_OBJ *pEnd = NULL;
	
	if (NULL == pAd)
	{
		DBG_ERR("pAd is null, unit = %d\n", unit);
		return ERROR;
	}

	if (FALSE == RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		pEnd = pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_dev;

		if (NULL == pEnd)
		{
			DBG_ERR("end is null\n");
			return ERROR;
		}

		taskSpawn("wlanInit", 200, 0, 15 * 1024, wlanDrvStart, pEnd, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		APStartUp(pAd);

		if (TRUE == pAd->ApCfg.bAutoChannelAtBootup)
		{
			DBG_WARN("auto channel select start\n");
			ApSiteSurvey(pAd, NULL, SCAN_PASSIVE, TRUE);
		}
	}
	
	return OK;
}

STATUS os_wlanExit(void)
{
	return OK;
}

STATUS os_wlanIoctlEx(UINT32 devno, UINT32 unit, UINT32 cmd, void * data, UINT32 len)
{	
	STATUS ret = OK;
	END_OBJ *pEnd = NULL;
	RTMP_ADAPTER *pAd = NULL;

	if (unit >= MAX_VAP_CNT)
	{
		DBG_ERR("vap unit(%u) error, cmd %d, max vap cnt %u.\n", unit, cmd, MAX_VAP_CNT);
		return ERROR;
	}

	pEnd = endFindByName(INF_MAIN_DEV_NAME, 0);
	if (NULL == pEnd)
	{
		DBG_ERR("get end \"%s%d\" failed!\n", INF_MAIN_DEV_NAME, 0);
		return ERROR;
	}

	pAd = RTMP_OS_NETDEV_GET_PRIV(pEnd);
	if (NULL == pAd)
	{
		DBG_ERR("get pAd failed!\n");
		return ERROR;
	}

	if (taskIdSelf() == taskNameToId("tCmdTask"))
	{
		/* 
			NB: not a safe way, but sometimes lock is acquired by other contexts and never
			got released, then this is a backdoor to configure the WLAN driver, so we do not call
			WL_LOCK here
		*/
		ret = wlan_ioctl(pAd, unit, cmd, data, len);
	}
	else
	{
		semTake(pAd->lock, WAIT_FOREVER);
		ret = wlan_ioctl(pAd, unit, cmd, data, len);
		semGive(pAd->lock);
	}
	
	return ret;
}

void os_random(void *p, UINT32 n)
{
	UINT8 *dp = (UINT8 *)p;
	UINT32 val, nb;

	while (n > 0)
	{
		val = random();
		nb = n > sizeof(UINT32) ? sizeof(UINT32) : n;
		OS_MEMCPY(dp, &val, nb);
		dp += sizeof(UINT32);
		n -= nb;
	}
}

void wait_for_completion_timeout(RTMP_OS_COMPLETION* comp,ULONG abstime )
{
	//todo check: call in DSR.
	semTake(comp->wait, abstime);
}

VOID CmdTxPowerCtrl(RTMP_ADAPTER *pAd, UINT8 central_chl)
{

	struct cmd_msg *msg;
	EXT_CMD_TX_POWER_CTRL_T CmdTxPwrCtrl;
	int ret = 0;
	int i,j;
	UINT8 PwrPercentageDelta = 0;
	UINT32 Value;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TX_POWER_CTRL_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_SET_TX_POWER_CTRL, TRUE, 0,
		TRUE, TRUE, 8, NULL, EventExtCmdResult);
	memset(&CmdTxPwrCtrl, 0x00, sizeof(CmdTxPwrCtrl));

	CmdTxPwrCtrl.ucCenterChannel = central_chl;
	CmdTxPwrCtrl.ucTSSIEnable = pAd->EEPROMImage[NIC_CONFIGURE_1_TOP];
	CmdTxPwrCtrl.ucTempCompEnable = pAd->EEPROMImage[NIC_CONFIGURE_1];

	CmdTxPwrCtrl.aucTargetPower[0] = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	CmdTxPwrCtrl.aucTargetPower[1] = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
#ifdef CONFIG_ATE
        /* Replace Target Power from QA Tool manual setting*/
	if (ATE_ON(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s,ATE set tx power\n", __FUNCTION__));
		CmdTxPwrCtrl.aucTargetPower[0] = pAd->ATECtrl.TxPower0;
		CmdTxPwrCtrl.aucTargetPower[1] = pAd->ATECtrl.TxPower1;
	}
#endif
	NdisCopyMemory(&CmdTxPwrCtrl.aucRatePowerDelta[0], &(pAd->EEPROMImage[TX_PWR_CCK_1_2M]), sizeof(CmdTxPwrCtrl.aucRatePowerDelta));
        CmdTxPwrCtrl.ucBWPowerDelta = pAd->EEPROMImage[G_BAND_20_40_BW_PWR_DELTA];
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[0], &(pAd->EEPROMImage[TX0_G_BAND_OFFSET_LOW]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[0]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[1], &(pAd->EEPROMImage[TX0_G_BAND_CHL_PWR_DELTA_MID]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[1]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[2], &(pAd->EEPROMImage[TX0_G_BAND_OFFSET_HIGH]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[2]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[5], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_HIGH]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[5]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[3], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELATE_LOW]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[3]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[4], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_MID]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[4]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucTempCompPower[0], &(pAd->EEPROMImage[STEP_NUM_NEG_7]), sizeof(CmdTxPwrCtrl.aucTempCompPower));

	if (1)
	{	
		if (pAd->CommonCfg.TxPowerPercentage > 90)
		{
			PwrPercentageDelta = 0;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 60) /* reduce Pwr for 1 dB. */
		{
			PwrPercentageDelta = 1;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 30) /* reduce Pwr for 3 dB. */
		{
			PwrPercentageDelta = 3;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 15) /* reduce Pwr for 6 dB. */
		{
			PwrPercentageDelta = 6;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 9)	/* reduce Pwr for 9 dB. */
		{
			PwrPercentageDelta = 9;
		}
		else /* reduce Pwr for 12 dB. */
		{
			PwrPercentageDelta = 12;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PwrPercentageDelta = 0x%x\n", PwrPercentageDelta));
		CmdTxPwrCtrl.ucReserved = PwrPercentageDelta;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucCenterChannel=%x\n", CmdTxPwrCtrl.ucCenterChannel));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucTSSIEnable=%x \n", CmdTxPwrCtrl.ucTSSIEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucTempCompEnable=%x\n", CmdTxPwrCtrl.ucTempCompEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucTargetPower[0]=%x\n", CmdTxPwrCtrl.aucTargetPower[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucTargetPower[1]=%x\n", CmdTxPwrCtrl.aucTargetPower[1]));
	for(i=0; i<14;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucRatePowerDelta[%d]=%x\n", i, CmdTxPwrCtrl.aucRatePowerDelta[i]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucBWPowerDelta=%x \n",CmdTxPwrCtrl.ucBWPowerDelta));
	for(i=0;i<6;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucCHPowerDelta[%d]=%x\n", i, CmdTxPwrCtrl.aucCHPowerDelta[i]));
	for(i=0;i<17;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucTempCompPower[%d]=%x\n", i, CmdTxPwrCtrl.aucTempCompPower[i]));

	AndesAppendCmdMsg(msg, (char *)&CmdTxPwrCtrl, sizeof(CmdTxPwrCtrl));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;

}



VOID CmdSetTxPowerCtrl(RTMP_ADAPTER *pAd, UINT8 central_chl)
{
		UCHAR channel;
		
		channel=central_chl;
		RTEnqueueInternalCmd(pAd, CMDTHREAD_SET_POWER, (VOID *)&channel, sizeof(UCHAR));	

}

static INT32 MT_ATEStop(RTMP_ADAPTER *pAd)
{
	UINT32 tmp=0;
	RTEnqueueInternalCmd(pAd, CMDTHREAD_STOP_ATE, (VOID *)&tmp, sizeof(UINT32));
}

VOID VX_MBSS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps, INT IdBss)
{
	PNET_DEV pDevNew;
	INT status;
	struct wifi_dev *wdev;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->FlgMbssInit[IdBss] != FALSE)
		return;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->MBSS_Init"));

	pAd->ApCfg.MBSSID[IdBss].wdev.if_dev = NULL;

	/* create virtual network interface */
#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
	IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */
    BSS_STRUCT *pMbss;
struct mt_dev_priv	*priv_info = NULL;
	dev_name = get_dev_name_prefix(pAd, INT_MBSSID);
	pDevNew = RtmpOSNetDevCreate(pAd, MC_RowID, &IoctlIF, INT_MBSSID, IdBss, sizeof(PRTMP_ADAPTER), INF_MBSSID_DEV_NAME);

#ifdef HOSTAPD_SUPPORT
	pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */
	if (pDevNew == NULL)
	{
		pAd->ApCfg.BssidNum = IdBss; /* re-assign new MBSS number */
		return;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Register MBSSID IF (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
	}

   	pMbss = &pAd->ApCfg.MBSSID[IdBss];
	wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
	wdev->wdev_type = WDEV_TYPE_AP;
	wdev->func_dev = &pAd->ApCfg.MBSSID[IdBss];
	wdev->func_idx = IdBss;
	wdev->sys_handle = (void *)pAd;
	wdev->if_dev = pDevNew;
	if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
		RtmpOSNetDevFree(pDevNew);
		return;
	}
	
	wdev->tx_pkt_allowed = ApAllowToSendPacket;
	wdev->tx_pkt_handle = APSendPacket;
	wdev->wdev_hard_tx = APHardTransmit;

	wdev->rx_pkt_allowed = ap_rx_pkt_allow;
	wdev->rx_ps_handle = ap_rx_ps_handle;
	wdev->rx_pkt_foward = ap_rx_foward_handle;
		
	priv_info = (struct mt_dev_priv *) kmalloc(sizeof(struct mt_dev_priv), GFP_ATOMIC);
	((pDevNew)->devObject.pDevice) = priv_info;	
	
	RTMP_OS_NETDEV_SET_PRIV(pDevNew, pAd);
	RTMP_OS_NETDEV_SET_WDEV(pDevNew, wdev);

	ASSERT(pMbss);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<---MBSS_Init"));

	pAd->FlgMbssInit[IdBss] = TRUE;

}


#ifdef TP_LINK_SUPPORT
INT RT_CfgSetCountryNum(
	IN PRTMP_ADAPTER	pAd, 
	IN PSTRING			arg)
{
	int countryNum = simple_strtol(arg, 0, 10);
	char tmpbuf[20];
	int index;
	int retval;

	for (index = 0; allCountry[index].CountryNum < 999; index++)
	{
		if (countryNum == allCountry[index].CountryNum)
		{
			break;
		}
	}
	if (allCountry[index].CountryNum >= 999)
	{
		return FALSE;
	}
	//CountryRegion
	sprintf(tmpbuf, "%d", allCountry[index].RegDomainNum11G);
	retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_24G);
	if (TRUE != retval)
	{
		return retval;
	}
	//CountryRegionABand
	sprintf(tmpbuf, "%d", allCountry[index].RegDomainNum11A);
	retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_5G);
	return retval;
}
#endif


int rtmp_ee_flash_write(PRTMP_ADAPTER pAd, USHORT Offset, USHORT Data)
{
	if (!pAd->chipCap.ee_inited)
	{
	}
	else
	{
		memcpy(pAd->eebuf + Offset, &Data, 2);
		memcpy(EeBuffer+ Offset, &Data, 2);
	}
	return 0;
}


VOID rtmp_ee_flash_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	if (!pAd->chipCap.ee_inited)
		return;

	memcpy(Data, pAd->eebuf, EEPROM_SIZE);
}


VOID rtmp_ee_flash_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	if (!pAd->chipCap.ee_inited)
		return;
	memcpy(pAd->eebuf, Data, EEPROM_SIZE);
}


static NDIS_STATUS rtmp_ee_flash_reset(RTMP_ADAPTER *pAd, UCHAR *start)
{
	PUCHAR src;
	RTMP_OS_FS_INFO osFsInfo;
	RTMP_OS_FD srcf;
	INT retval;


	src = EEPROM_DEFAULT_FILE_PATH;


	RtmpOSFSInfoChange(&osFsInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> Error opening file %s\n", src));
			if ( pAd->chipCap.EEPROM_DEFAULT_BIN != NULL )
			{
				NdisMoveMemory(start, pAd->chipCap.EEPROM_DEFAULT_BIN,
					pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE?MAX_EEPROM_BUFFER_SIZE:pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Load EEPROM Buffer from default BIN.\n"));
				return NDIS_STATUS_SUCCESS;
			}
			else
			{
			return NDIS_STATUS_FAILURE;
		}
		}
		else
		{
			/* The object must have a read method*/
			NdisZeroMemory(start, EEPROM_SIZE);

			retval = RtmpOSFileRead(srcf, start, EEPROM_SIZE);
			if (retval < 0)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> Read %s error %d\n", src, -retval));
			}
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> rtmp_ee_flash_reset copy %s to eeprom buffer\n", src));
			}

			retval = RtmpOSFileClose(srcf);
			if (retval)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> Error %d closing %s\n", -retval, src));
			}
		}
	}

	RtmpOSFSInfoChange(&osFsInfo, FALSE);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS rtmp_nv_init(RTMP_ADAPTER *pAd)
{
#ifdef MULTIPLE_CARD_SUPPORT
	UCHAR *eepromBuf;
#endif /* MULTIPLE_CARD_SUPPORT */

	unsigned int eebufLen = EEPROM_SIZE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> rtmp_nv_init\n"));


	/*os_alloc_mem(pAd, &(pAd->eebuf), EEPROM_SIZE);*/
	pAd->eebuf = pAd->EEPROMImage;
	if (pAd->eebuf != NULL)
	{	
		NdisZeroMemory(pAd->eebuf, EEPROM_SIZE);
		//oal_sys_readRadioFlash(pAd->eebuf, &eebufLen);

		OS_READCAL(pAd->eebuf, EEPROM_SIZE);
		
		init_flag = 1;
		nv_ee_start = EeBuffer;
		pAd->chipCap.ee_inited = 1;

		memDump(pAd->eebuf, EEPROM_SIZE);

		if ((pAd->chipCap.ee_valid = validFlashEepromID(pAd)) == FALSE)
		{
			my_printf("The EEPROM in Flash are wrong, use default, %p\n", pAd->eebuf);
			NdisMoveMemory(pAd->eebuf, EeBuffer, EEPROM_SIZE);
		}
		else
		{
			USHORT currentEeVersion = 0xFFFF;
			USHORT flashEeVersion = 0xFFFF;
			memcpy(&flashEeVersion, pAd->eebuf + 2, 2);
			memcpy(&currentEeVersion, EeBuffer + 2, 2);
			if (currentEeVersion != 0x0104 && flashEeVersion == 0x0104)
			{
				/*eeprom version updated*/
				NdisMoveMemory(EeBuffer+0x52, pAd->eebuf+0x52, 28);
				EeBuffer[0x3a] = pAd->eebuf[0x3a];
				NdisMoveMemory(pAd->eebuf, EeBuffer, EEPROM_SIZE);
			}
			else
			{
				NdisMoveMemory(EeBuffer, pAd->eebuf, EEPROM_SIZE);
			}
		}
		/* Set it back for latter initialize. */
		init_flag = 0;
		nv_ee_start = 0;
		return NDIS_STATUS_SUCCESS;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->chipCap.EEPROMImage == NULL!!!\n"));
		return NDIS_STATUS_FAILURE;
	}
}

INT rtmp_cfg_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
	return TRUE;
}


int OS_TEST_BIT(int bit, unsigned long *flags)
{
	if ((( *(flags) & (bit)) != 0))
		return 0;
	else
		return 1;
	
}
void OS_SET_BIT(int bit, unsigned long *flags)
{
	( *(flags) |= (bit));
}

void OS_CLEAR_BIT(int bit, unsigned long *flags)
{
	( *(flags) &= ~ (bit));
}

