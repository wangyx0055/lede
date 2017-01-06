/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: softq_stat
	softq_statistic.c
*/


#include <linux/init.h>
#include <linux/module.h>
#include <os/rt_linux_txrx_hook.h>

#include "softq_stat.h"


/*
 *   Defination 
*/
#define DRIVER_DESC "Register TX/RX callback for debug software queue statistic "


/*
 *   Gloab variable
 */

softq_stat_t softq_statistic;

unsigned int glen=0;
char *gBuffer = NULL;

/*
 *  Local function
 */
PRTMP_ADAPTER pGAd = NULL;


static int softq_state_enQueue(unsigned int queIdx)
{
	
	if (queIdx < 15)
	{
		softq_statistic.apSendPacket[queIdx]++;
	}
	return 0;
}

static int softq_state_drop(unsigned int queIdx)
{
	if (queIdx <  15) 
	{
		if (softq_statistic.swDropPacket[queIdx] == 0)
		{
			printk("%s():drop pkt, QueIdx=%d\n", __FUNCTION__, queIdx);
		}
		softq_statistic.swDropPacket[queIdx] +=1;
	}
	return 0;
}

static int softq_state_deQueue(unsigned int queIdx,RTMP_ADAPTER *pAd)
{
	unsigned int hTsf,lTsf;
	unsigned long tsf ;
	if (queIdx < 15)
	{
		softq_statistic.apDequeuePacket[queIdx]++;
		if(softq_statistic.fstate == SOFTQ_F_START && 0)
		{
			AsicGetTsfTime(pAd,&hTsf,&lTsf);
			tsf = hTsf <<32  | lTsf; 
			glen += sprintf(gBuffer+glen,"%d,%lu,%d\n",queIdx,tsf,softq_statistic.apDequeuePacket[queIdx]);	
			if(glen >= GSIZE) { 
				glen = 0;				
				softq_stat_fileWrite(gBuffer,GSIZE);
				memset(gBuffer,0,GSIZE);
			}
		}
	}
	return 0;
}

static int softq_state_qm(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_SDIO_SUPPORT

	P_TX_CTRL_T prTxCtrl = NULL;
	UINT16 *p;

	prTxCtrl = &pAd->rTxCtrl;
	
	p = &prTxCtrl->rTc.au2MaxNumOfPage[0];
	printk ("Max Page\t\t: %5d %5d %5d %5d %5d\n" , p[0], p[1], p[2], p[3], p[4]);
	p = &prTxCtrl->rTc.au2FreePageCount[0];
	printk("FreePage\t\t: %5d %5d %5d %5d %5d\n", p[0], p[1], p[2], p[3], p[4]);
	p = &prTxCtrl->rTc.au2CurrentTcResource[0];
	printk("CurrentTcResourc\t: %5d %5d %5d %5d %5d\n", p[0], p[1], p[2], p[3], p[4]);
	p = &prTxCtrl->rTc.au2GuaranteedTcResource[0];
	printk("GuaranteedTcResource: %5d %5d %5d %5d %5d\n", p[0], p[1], p[2], p[3], p[4]);
	p = &prTxCtrl->rTc.au2AvgDemand[0];
	printk("AvgDemand\t\t: %5d %5d %5d %5d %5d\n", p[0], p[1], p[2], p[3], p[4]);
	printk("QlenUpdateTime: %d, AdjustUpdateTime: %d\n",prTxCtrl->rTc.u2TimeToUpdateQueLen,prTxCtrl->rTc.u2TimeToAdjustTcResource);
#endif
}
static int softq_state_mlme(unsigned int staId,RTMP_ADAPTER *pAd )
{
	int p ;
	unsigned int freeNO;
	STA_TR_ENTRY *trEntry;
	PQUEUE_HEADER pQueue;
	struct tx_swq_fifo *fifo_swq;

	trEntry = &pAd->MacTab.tr_entry[staId];		

	pGAd = pAd;

	if (softq_statistic.isSoftqStat == SOFTQ_STATE_REPORT_INTR_PERIOD)
	{
		softq_statistic.counter++;
	}	

	/*per 100ms will update once time*/
	for (p=0; p < 4; p++) 
	{				
		pQueue = &trEntry->tx_queue[p];			
		softq_statistic.swQueueCounter[p]++;
		softq_statistic.swQueueTotal[p] += pQueue->Number;
		softq_statistic.txRingCounter[p]++;
		freeNO = GET_TXRING_FREENO(pAd, p);
		softq_statistic.txRingTotal[p] += (TX_RING_SIZE - freeNO);
	}

        if (((softq_statistic.isSoftqStat==SOFTQ_STATE_REPORT_INTR_PERIOD) && (softq_statistic.counter >= softq_statistic.times)) 
	|| softq_statistic.isSoftqStat==SOFTQ_STATE_REPORT_INTR_EACHTIME) 
	{
	
		unsigned int  temp1, temp2;
		unsigned int  freeNO0,freeNO1,freeNO2,freeNO3;
		printk("===========================================================================\n");
		temp1 = softq_statistic.apSendPacket[0]+softq_statistic.apSendPacket[1]+softq_statistic.apSendPacket[2]+softq_statistic.apSendPacket[3];
		temp2 = softq_statistic.apPreSendPacket[0]+softq_statistic.apPreSendPacket[1]+ softq_statistic.apPreSendPacket[2] + softq_statistic.apPreSendPacket[3] ;
                printk(" Send from Charior: AC0=%d, AC1=%d, AC2=%d, AC3=%d, Total=%d\n"
                        , (softq_statistic.apSendPacket[0] - softq_statistic.apPreSendPacket[0])
                        , (softq_statistic.apSendPacket[1] - softq_statistic.apPreSendPacket[1])
                        , (softq_statistic.apSendPacket[2] - softq_statistic.apPreSendPacket[2])
                        , (softq_statistic.apSendPacket[3] - softq_statistic.apPreSendPacket[3])
                        , (temp1 - temp2));

		temp1 = softq_statistic.apDequeuePacket[0]+softq_statistic.apDequeuePacket[1]+softq_statistic.apDequeuePacket[2]+softq_statistic.apDequeuePacket[3];
		temp2 = softq_statistic.apPreDequeuePacket[0]+softq_statistic.apPreDequeuePacket[1]+softq_statistic.apPreDequeuePacket[2]+softq_statistic.apPreDequeuePacket[3];
		printk("Dequeue to Tx Ring: AC0=%d, AC1=%d, AC2=%d, AC3=%d, Total=%d\n"
			, (softq_statistic.apDequeuePacket[0] - softq_statistic.apPreDequeuePacket[0])
			, (softq_statistic.apDequeuePacket[1] - softq_statistic.apPreDequeuePacket[1])
			, (softq_statistic.apDequeuePacket[2] - softq_statistic.apPreDequeuePacket[2])
			, (softq_statistic.apDequeuePacket[3] - softq_statistic.apPreDequeuePacket[3])
			, (temp1 - temp2));

		temp1 = softq_statistic.swDropPacket[0]+softq_statistic.swDropPacket[1]+softq_statistic.swDropPacket[2]+softq_statistic.swDropPacket[3];
		temp2 = softq_statistic.swPreDropPacket[0]+softq_statistic.swPreDropPacket[1]+softq_statistic.swPreDropPacket[2]+softq_statistic.swPreDropPacket[3];
		printk("      Drop packets: AC0=%d, AC1=%d, AC2=%d, AC3=%d, Total=%d\n"
			, (softq_statistic.swDropPacket[0] - softq_statistic.swPreDropPacket[0])
			, (softq_statistic.swDropPacket[1] - softq_statistic.swPreDropPacket[1])
			, (softq_statistic.swDropPacket[2] - softq_statistic.swPreDropPacket[2])
			, (softq_statistic.swDropPacket[3] - softq_statistic.swPreDropPacket[3])
			, (temp1 - temp2));

		freeNO0 = GET_TXRING_FREENO(pAd, 0);
		freeNO1 = GET_TXRING_FREENO(pAd, 1);
		freeNO2 = GET_TXRING_FREENO(pAd, 2);
		freeNO3 = GET_TXRING_FREENO(pAd, 3);
              	printk("      Tx AC Quato: AC0=%d, AC1=%d, AC2=%d, AC3=%d\n\n"
                            , (freeNO0)
                            , (freeNO1)
                            , (freeNO2)
                            , (freeNO3));

		for(p=0;p<4;p++)
		{
			softq_statistic.apPreSendPacket[p] = softq_statistic.apSendPacket[p];
			softq_statistic.apPreDequeuePacket[p] = softq_statistic.apDequeuePacket[p];
			softq_statistic.swPreDropPacket[p] = softq_statistic.swDropPacket[p];
		}

		
		for(p=0;p<4;p++)
		{
			fifo_swq = &pGAd->tx_swq[p];
			printk("QID: %d,enqIdx: %d, deqIdx: %d\n",p,fifo_swq->enqIdx,fifo_swq->deqIdx);
		}
		
		printk("enqCount=%u\n",trEntry->enqCount);
		
		softq_state_qm(pAd);
			
	}


	if (softq_statistic.counter >=softq_statistic.times)
	{
		softq_statistic.counter = 0;
	}
	return 0;
}


static int softq_state_deQueueProgress(unsigned int queIdx,struct tx_swq_fifo *swqFifo)
{
	
	if (queIdx < 15)
	{
		if(softq_statistic.fstate == SOFTQ_F_START)
		{
			glen += sprintf(gBuffer+glen,"DE:%d,%d,%d\n", queIdx,swqFifo->enqIdx,swqFifo->deqIdx);
			if(glen >= GSIZE) { 
				glen = 0;				
				softq_stat_fileWrite(gBuffer,GSIZE);
				memset(gBuffer,0,GSIZE);
			}
			
		}
	}
		return 0;
}

static int softq_state_enQueueProgress(unsigned int queIdx,struct tx_swq_fifo *swqFifo)
{
	
	if (queIdx < 15)
	{
		if(softq_statistic.fstate == SOFTQ_F_START)
		{
			glen += sprintf(gBuffer+glen,"EN:%d,%d,%d\n", queIdx,swqFifo->enqIdx,swqFifo->deqIdx);
			if(glen >= GSIZE) { 
				glen = 0;				
				softq_stat_fileWrite(gBuffer,GSIZE);
				memset(gBuffer,0,GSIZE);
			}
			
		}
	}
		return 0;
}


static int softq_stat_callback(unsigned short hook, struct sk_buff *skb, unsigned char queIdx,void *priv)
{
	switch(hook){
	case WLAN_TX_ENQUEUE:
		softq_state_enQueue(queIdx);
	break;
	case WLAN_TX_DEQUEUE:		
		softq_state_deQueue(queIdx,(RTMP_ADAPTER *)priv);
	break;
	case WLAN_TX_DROP:		
		softq_state_drop(queIdx);
	break;
	case WLAN_TX_MLME_PERIOD:
		softq_state_mlme(softq_statistic.staId,(RTMP_ADAPTER *)priv);
	break;
	case WLAN_TX_DEQUEUE_PROGRESS:
		softq_state_deQueueProgress(queIdx,(struct tx_swq_fifo*)priv);
	break;
	case WLAN_TX_ENQUEUE_PROGRESS:
		softq_state_enQueueProgress(queIdx,(struct tx_swq_fifo*)priv);
	default:
	break;
	}
	return 0;
}


static void sofq_stat_init(void)
{
	memset(&softq_statistic,0,sizeof(softq_statistic));
	softq_statistic.isSoftqStat = SOFTQ_STATE_NOP;
	softq_statistic.times = 10;
	softq_statistic.staId = 1; /*first statision*/
	sprintf(softq_statistic.fname,"/var/softq_stat");
	softq_statistic.fstate = 0;
}

static void sofq_stat_exit(void)
{
	memset(&softq_statistic,0,sizeof(softq_statistic));
}


static void softq_file_init(void)
{
	gBuffer = kmalloc(GSIZE,GFP_KERNEL);
	memset(gBuffer,0,GSIZE);

}

static void softq_file_exit(void)
{
	softq_stat_fileClose();
	memset(gBuffer,0,GSIZE);
	if(gBuffer) 
		kfree(gBuffer);
}


static struct mt_wlanTxRxHookOps  softq_stat_ops __read_mostly = {
	.name	= "SoftQStat",
	.hooks	= (1<< WLAN_TX_ENQUEUE)  | (1 <<WLAN_TX_DEQUEUE) | (1 <<WLAN_TX_DROP) |
			(1<< WLAN_TX_MLME_PERIOD) |(1<<WLAN_TX_DEQUEUE_PROGRESS) |  (1<<WLAN_TX_ENQUEUE_PROGRESS)  ,
	.hookfun	= softq_stat_callback,
	.me		= THIS_MODULE,
	.priority   =  WLAN_HOOK_PRI_SOFTQ_STAT
};

 
static int __init softq_stat_module_init(void)
{
	printk("%s(): module init and register callback for debug\n",__FUNCTION__);

	/*initial global struct*/
	sofq_stat_init();
	/*register proc file*/
	softq_stat_proc_init();
	/*initial file*/
	softq_file_init();
	/*register hook function*/
	RtmpOsTxRxHookRegister(&softq_stat_ops);
	return 0;
}

static void __exit softq_stat_module_exit(void)
{
	printk("%s(): module exist\n",__FUNCTION__);
	RtmpOsTxRxHookUnRegister(&softq_stat_ops);
	sofq_stat_exit();
	softq_file_exit();
	softq_stat_proc_exit();
}

module_init(softq_stat_module_init);
module_exit(softq_stat_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC); /* What does this module do */
MODULE_SUPPORTED_DEVICE("mt7603");

