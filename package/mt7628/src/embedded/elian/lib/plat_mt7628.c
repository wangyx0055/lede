
#include "plat.h"
#include "../os/linux/osal.h"

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_config.h"

#ifdef CONFIG_STA_SUPPORT
static unsigned int g_iftype = 0;
#endif

static struct timer_list rst_timer;
unsigned long MSC_DEBUG_LEVEL = MSC_DEBUG_TRACE;


int sc_plt_get_CentralChannel(struct chan_info *info, void *priv)
{	
	int centralChannel;	
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;	

	if(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)	
	{		
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)			
			centralChannel = info->chan_id + 2;		
		else			
			if(info->chan_id == 14)				
				centralChannel = info->chan_id - 1;			
			else				
				centralChannel = info->chan_id - 2;	
	}	
	else	
	{		
		centralChannel = info->chan_id;	
	}	

	return centralChannel;
}


void sc_plt_switch_channel(struct chan_info *info, void *priv)
{	
	int centralChannel = 0;	
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;		
	
	centralChannel = sc_plt_get_CentralChannel(info,priv);
	pAd->CommonCfg.Channel = info->chan_id;
	AsicSwitchChannel(pAd, centralChannel, FALSE);		
}


int sc_plt_set_monitor_chan(struct chan_info *info, void *priv)
{
#ifdef CONFIG_STA_SUPPORT

	CFG80211_CB *p80211CB = NULL;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;

	MSC_PRINT(MSC_DEBUG_TRACE,("[MSC] set chl, chan_id:%d, width:%d, chan_flags:0x%x\n",
		 info->chan_id, info->width, info->flags ));

	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = info->chan_id;

	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);
	if (p80211CB == NULL) {
		return -1;
	}

	ChanInfo.MonFilterFlag = 0x17f93;//0x17f93 is a RX filter
	ChanInfo.IfType = RT_CMD_80211_IFTYPE_MONITOR;

	if(info->width == MSC_CHAN_WIDTH_20_NOHT) {
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
	} else if (info->width == MSC_CHAN_WIDTH_20) {
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT20;
	} else if (info->width == MSC_CHAN_WIDTH_40_PLUS) {
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40PLUS;
	} else if (info->width == MSC_CHAN_WIDTH_40_MINUS) {
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40MINUS;
	} else {
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
	}

	/* set channel */
	RTMP_DRIVER_80211_CHAN_SET(pAd, &ChanInfo);
	//PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;
	//AsicSwitchChannel(pAd, info->chan_id, FALSE);
 
	return 0;
#endif

#ifdef CONFIG_AP_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;
	unsigned int filter = 0;

	filter = 0xa;
	pAd->CommonCfg.RegTransmitSetting.field.BW = 1;   /* BW=40*/
	pAd->CommonCfg.BBPCurrentBW = 1;
	if(info->width == MSC_CHAN_WIDTH_40_MINUS)
	{
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = 0x03;
	}
	else if(info->width == MSC_CHAN_WIDTH_40_PLUS)
	{
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = 0x01;
	}

	sc_plt_switch_channel(info, priv);
	MAC_IO_WRITE32(pAd, RMAC_RFCR, filter);
	return 0;
#endif

}

int sc_plt_enter_monitor_mode(struct monitor_info *info, void *priv)
{
#ifdef CONFIG_STA_SUPPORT

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;
	unsigned int filter = 0;

	MSC_PRINT(MSC_DEBUG_TRACE,("[MSC] enter monitor mode: filter:0x%x, chan_id:%d, width:%d, chan_flags:0x%x, monitor_priv=%p, priv=%p\n",
			info->filter, info->chl_info.chan_id, info->chl_info.width,
			info->chl_info.flags, info->priv, priv));

	filter |= 0x07;

	/* 1.Save current state info. */
	g_iftype = pAd->net_dev->ieee80211_ptr->iftype;
	MSC_PRINT(MSC_DEBUG_TRACE,("[MSC] old iftype: 0x%x\n", g_iftype));

	/* 2.Switch to monitor mode. */
	CFG80211DRV_OpsChgVirtualInf(pAd, &filter, RT_CMD_80211_IFTYPE_MONITOR);

	sc_plt_set_monitor_chan(&info->chl_info, priv);

#endif

#ifdef CONFIG_AP_SUPPORT
	
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;
		unsigned int filter = 0;
		//char tmp[5] = {0};
		
		MSC_PRINT(MSC_DEBUG_TRACE,("[MSC] enter monitor mode: filter:0x%x, chan_id:%d, width:%d, chan_flags:0x%x, monitor_priv=%p, priv=%p\n",
				info->filter, info->chl_info.chan_id, info->chl_info.width,
				info->chl_info.flags, info->priv, priv));
	
		filter = 0xa;
	
		/* 1.Set Channel. */
		
		//sprintf(tmp,"%d",info->chl_info.chan_id);
		//Set_Channel_Proc(pAd,tmp);
		pAd->CommonCfg.RegTransmitSetting.field.BW = 1;   /* BW=40*/
		pAd->CommonCfg.BBPCurrentBW = 1;
		if(info->chl_info.width == MSC_CHAN_WIDTH_40_MINUS)
		{
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = 0x03;
		}
		else if(info->chl_info.width == MSC_CHAN_WIDTH_40_PLUS)
		{
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = 0x01;
		}

		sc_plt_switch_channel(&info->chl_info, priv);

	
		/* 2.Set Rx Filter. */
		MAC_IO_WRITE32(pAd, RMAC_RFCR, filter);
	
#endif

	return 0;
}

int sc_plt_leave_monitor_mode(void *priv)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;
	unsigned int filter = 0;
	
	MSC_PRINT(MSC_DEBUG_TRACE,("[MSC] leave monitor mode.\n"));
	CFG80211DRV_OpsChgVirtualInf(pAd, &filter, g_iftype);
#endif

#ifdef CONFIG_AP_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;

	MSC_PRINT(MSC_DEBUG_TRACE,("[MSC] leave monitor mode.\n"));
	AsicSetRxFilter(pAd);
#endif

	return 0;
}

char *sc_plt_get_la(void *priv)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)priv;
	MSC_PRINT(MSC_DEBUG_TRACE,("[MSC] get local address %p : %x:%x:%x:%x:%x:%x\n",
		priv, pAd->CurrentAddress[0]&0xff, pAd->CurrentAddress[1]&0xff, pAd->CurrentAddress[2]&0xff, 
		pAd->CurrentAddress[3]&0xff, pAd->CurrentAddress[4]&0xff, pAd->CurrentAddress[5]&0xff));
	return (char*)pAd->CurrentAddress;
}

void sc_plt_aes128_decrypt(unsigned char *cipher_blk, unsigned int cipher_blk_size, 
				unsigned char *key, unsigned int key_len, 
				unsigned char *plain_blk, unsigned int *plain_blk_size)
{
	return RT_AES_Decrypt(cipher_blk, cipher_blk_size, key, key_len, plain_blk, plain_blk_size);
}

void sc_plt_add_timer(struct etimer *p)
{
	init_timer(&rst_timer);
	rst_timer.expires = jiffies + p->expires * HZ;
	rst_timer.data = p->data;
	rst_timer.function = p->func;
	return add_timer(&rst_timer);	
}

int sc_plt_del_timer(struct etimer *p)
{
	return del_timer(&rst_timer);
}


