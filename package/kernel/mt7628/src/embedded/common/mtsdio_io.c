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

	Module Name:
	mtusb_io.c
*/

#include	"rt_config.h"

// Make sure drv own
INT32 wlanAcquirePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset)
{
    INT32 Ret = 0;
    UINT32 Value;
    int i = 0;
           
    //printk("[%s]\n",  __func__);
    
    if ((Offset == WCIR) ||
        (Offset == WHLPCR) ||
        (Offset == WSDIOCSR) 
        )
    {
        // These registers are accessible when Low Power 
        return 1;
    }
	
    RTMP_SEM_EVENT_WAIT(&(pAd->PowerLock), Ret);
	if(Ret != 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));

	pAd->bCRAccessing++;

    if (pAd->bDrvOwn){
        // all registers are accessible
        RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
        return 1;
    }

    while(1)
    {
        RTMP_SDIO_READ32(pAd, WHLPCR, &Value);
        
        if (GET_W_FW_OWN_REQ_SET(Value))
        {
            // driver own now
            pAd->bDrvOwn = 1;
            break;
        }
        else
        {
            RTMP_SDIO_WRITE32(pAd, WHLPCR, W_FW_OWN_REQ_CLR);
            i++;
            if (i == 100)
                break;
                
            msleep(1);
        }
    }

    //printk("DrvOwn---------------------------------- bDrvOwn=%d\n", pAd->bDrvOwn);

    if (pAd->bDrvOwn)
    {
        Ret = 1;
	}
    else
    {
        Ret = 0;
        pAd->bCRAccessing--;	//will not continue accessing HW
	}
    
    RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
    
    return Ret;
}

// Drv give up own
void wlanReleasePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset)
{
    INT32 Ret = 0;
    	
    if ((Offset == WCIR) ||
        (Offset == WHLPCR) ||
        (Offset == WSDIOCSR) 
        )
    {
        // These registers are accessible when Low Power 
        return;
    }
                
    RTMP_SEM_EVENT_WAIT(&(pAd->PowerLock), Ret);
	if(Ret != 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));

	pAd->bCRAccessing--;

    RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
    
    return;
}

INT32 MakeFWOwn(RTMP_ADAPTER *pAd)
{
    INT32 Ret = 0;
    UINT32 Value;
    UINT retry = 100;
            
    RTMP_SEM_EVENT_WAIT(&(pAd->PowerLock), Ret);
	if(Ret != 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));

    if (!pAd->bDrvOwn)
    {
        // It is already FW own, do nothing
        RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
        return 1;
    }
    
    if (pAd->bSetFWOwnRunning)
    {
        RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
        return 1;
	}
	
    pAd->bSetFWOwnRunning = 1;

	//Make sure no SDIO access is running, wait until SDIO activity is end
	while(pAd->bCRAccessing & retry)
	{
		RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
		msleep(1);
		RTMP_SEM_EVENT_WAIT(&(pAd->PowerLock), Ret);
		if(Ret != 0)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wait CRAccessing %d\n", pAd->bCRAccessing));
		retry--;
	}
	
	if (retry == 0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("**************WARNING******************\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can not set FW own! gbDrvOwn=%d\n", pAd->bDrvOwn));
		pAd->bSetFWOwnRunning = 0;
        RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
        return 1;
	}
	
	// Set FW own 
    RTMP_SDIO_WRITE32(pAd, WHLPCR, W_FW_OWN_REQ_SET);
        
    RTMP_SDIO_READ32(pAd, WHLPCR, &Value);

    if (GET_W_FW_OWN_REQ_SET(Value))
    {
        // It is still driver own, we fail
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wlanReleasePowerControl\n"));
    }
    else
    {
        // FW own now
        pAd->bDrvOwn = 0;
        Ret = 1;
    }

    //printk("FW own------------------------------ bDrvOwn=%d\n", pAd->bDrvOwn);
        
    pAd->bSetFWOwnRunning = 0;

    RTMP_SEM_EVENT_UP(&(pAd->PowerLock));
    
    return Ret;	
}

INT32 MTSDIORead32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value)
{

	INT32 Ret = 0;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;

    if (!wlanAcquirePowerControl(pAd, Offset))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTSDIORead32 failure(can't drv own )! Offset: %x\n",Offset));
        return 0;
    }

	sdio_claim_host(dev_func);
	*Value = sdio_readl(dev_func, Offset, &Ret);
	sdio_release_host(dev_func);

	if (Ret) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTSDIORead32 failure! Offset: %x\n",Offset));
	}
	
    wlanReleasePowerControl(pAd, Offset);

	return Ret;   
}


INT32 MTSDIOWrite32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value)
{
	INT32 Ret = 0;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;

    if (!wlanAcquirePowerControl(pAd, Offset))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MTSDIOWrite32 failure(can't drv own )! Offset: %x\n",Offset));
        return 0;
    }

	sdio_claim_host(dev_func);
	sdio_writel(dev_func, Value, Offset, &Ret);
	sdio_release_host(dev_func);

    wlanReleasePowerControl(pAd, Offset);
	
	return Ret;
}


INT32 MTSDIOMultiRead(RTMP_ADAPTER *pAd, UINT32 Offset, 
										UCHAR *Buf,UINT32 Length)
{
	UINT32 Count = Length;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;
	UINT32 bNum = 0;
	INT32 Ret;

	while (Count >= pAd->BlockSize) {
		Count -= pAd->BlockSize;
		bNum++;
	}

	if (Count > 0 && bNum > 0) {
		bNum++;
	}
    
    if (!wlanAcquirePowerControl(pAd, Offset))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MTSDIOMultiRead failure(can't drv own )! Offset: %x\n",Offset));
        return 0;
    }   
   
	sdio_claim_host(dev_func);
	
	if (bNum > 0) {
		Ret = sdio_readsb(dev_func, Buf, Offset, pAd->BlockSize * bNum);
        /* ENE workaround */
        {
            int tmp;
            sdio_writel(dev_func, 0x0, 0xc4, &tmp);
        }
	}
	else 
	{
		Ret = sdio_readsb(dev_func, Buf, Offset, Count);
	}

	sdio_release_host(dev_func);


    wlanReleasePowerControl(pAd, Offset);

	return Ret;
}


INT32 MTSDIOMultiWrite(RTMP_ADAPTER *pAd, UINT32 Offset, 
										UCHAR *Buf,UINT32 Length)
{

	UINT32 Count = Length;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;
	INT32 Ret = 0;
	UINT32 bNum = 0;

    // padding DW 0 
    *(PUINT32) (&((Buf)[(Length + (4) - 1) & (~(4 - 1))])) = 0;

	while (Count >= pAd->BlockSize) {
		Count -= pAd->BlockSize;
		bNum++;
	}
   
	if (Count > 0 && bNum > 0) {
		bNum++;
	}
    
    if (!wlanAcquirePowerControl(pAd, Offset))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MTSDIOMultiWrite failure(can't drv own )! Offset: %x\n",Offset));
        return 0;
    }   
    
	sdio_claim_host(dev_func);

	if (bNum > 0) 
	{
    	Ret = sdio_writesb(dev_func, Offset, Buf, pAd->BlockSize * bNum);
        /* ENE workaround */
        {
            int tmp;
            sdio_writel(dev_func, 0x0, 0xc4, &tmp);
        }
	}
	else 
	{
		Ret = sdio_writesb(dev_func, Offset, Buf, Count);
	}

	sdio_release_host(dev_func);

    wlanReleasePowerControl(pAd, Offset);

	return Ret;
}


