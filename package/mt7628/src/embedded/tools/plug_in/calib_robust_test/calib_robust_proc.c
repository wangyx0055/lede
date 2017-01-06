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
	softq_proc.c
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include "calib_robust.h"


#include <linux/netdevice.h>

static struct proc_dir_entry *calib_proc=NULL;
static struct proc_dir_entry *state_entry=NULL, *times_entry=NULL,  *calibId_entry=NULL, *file_entry=NULL, *bw_entry=NULL;
extern  calib_test_t gCalib;
extern unsigned int glen;
extern char *gBuffer;
extern RTMP_ADAPTER *pGAd;


extern INT32 RtmpOsRfTest(RTMP_ADAPTER *pAd, UINT8 Action, UINT8 Mode, UINT8 CalItem);

extern int calib_id_name_get(unsigned int id,char *str);


/*local function*/


static int calib_robust_testModeSet(unsigned int state)
{
	int ret;

	/*change next state to */
	gCalib.stat =  state ?  CALIB_STATE_START_TEST_MODE : CALIB_STATE_STOP_TEST_MODE;
	/*start into test mode*/
	ret = RtmpOsRfTest(pGAd,ACTION_SWITCH_TO_RFTEST,state,0);	
	printk("[calib] enable test mode!\n");
	return ret;
}

static  int calib_robust_testStart(void)
{
	int ret;	
	ret = RtmpOsRfTest(pGAd,ACTION_IN_RFTEST, 0,gCalib.curCalibId);
	return ret;
}


 
static int calib_file_state_run(unsigned int fstate)
{
	switch(fstate){
	case CALIB_F_START:		
		calib_test_fileOpen(gCalib.fname);		
	break;
	case CALIB_F_END:
		calib_test_fileClose();		
	break;
	default:
	break;
	}
	return 0;
}

static int calib_stat_headline_show(struct calibItem *pItem)
{
	unsigned int reg,regId=0;

	memset(gBuffer,0,GSIZE);
	glen=0;
	glen+=sprintf(gBuffer+glen,"%s\r\n",pItem->name);

	if(pItem->mode==CALIB_REG_TYPE_PHY)
	{	
		reg = pItem->regs[regId];
		while(reg !=CALIB_REG_END) 
		{				
			glen+=sprintf(gBuffer+glen,"PHY0_%04x\t",reg);
			glen+=sprintf(gBuffer+glen,"PHY1_%04x\t",reg);
			regId++;				
			reg = pItem->regs[regId];
		};
	}else
	{
		reg = pItem->regs[regId];
		while( reg !=CALIB_REG_END)
		{				
			glen+=sprintf(gBuffer+glen,"CR_%04x\t",reg);
			regId++;				
			reg = pItem->regs[regId];
		};
	}
	glen+=sprintf(gBuffer+glen,"\r\n");
	calib_test_fileWrite(gBuffer,glen);
	memset(gBuffer,0,GSIZE);
	glen = 0;
	return 0;
}

static int calib_state_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char value[64], *end;
	int i;
	struct calibItem *pItem = NULL;
	
	if (buff && !copy_from_user(value, buff, len1)) {

		 gCalib.stat = simple_strtoul(value, &end, 10);

		switch( gCalib.stat ){
		case CALIB_STATE_NOP:
			gCalib.curCount = 0;
		break;
		case CALIB_STATE_START_TEST_MODE:
		{			
			printk("[calib] goto test mode...\n");
			calib_robust_testModeSet(1);
		}
		break;
		case CALIB_STATE_CALIB_START:
		{			
			printk("[calib] start to test...\n");
			pItem = calib_table_get(gCalib.curCalibId);
			if(pItem)
			{		
			
				gCalib.curCount = 0;
	
				while(gCalib.times >gCalib.curCount)
				{
					calib_robust_testStart();	
				}
				
				printk("[calib] count %d\n",gCalib.curCount);
			}
		}
		break;
		case CALIB_STATE_STOP_TEST_MODE:
		{
			printk("[calib]  stop test mode...\n");
			calib_robust_testModeSet(0);			
		}
		break;
		default:
		break;
		}
		 
	}		 
	return len1;
}


static int calib_state_proc_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,"calib_state: %s\n", 
	gCalib.stat == CALIB_STATE_NOP ? "nop":
	gCalib.stat == CALIB_STATE_START_TEST_MODE ? "start test mode":
	gCalib.stat== 	CALIB_STATE_CALIB_START? "calibration start": "stop test mode");
	 return 0;        
}


static int calib_state_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, calib_state_proc_show,NULL);
}


static int calib_times_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char value[64], *end;
	
	if (buff && !copy_from_user(value, buff, len1)) {

		 gCalib.times = simple_strtoul(value, &end, 10);
		 printk("[calib] change calibration time to %d\n",gCalib.times);
		 
	}		 
	return len1;
}


static int calib_times_proc_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,"times: %d\n",gCalib.times);
	 return 0;        
}


static int calib_times_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, calib_times_proc_show, NULL);
}



static int calib_id_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char value[64], *end;	
	struct calibItem *pItem = NULL;
		
	if(!buff) 
	{
		printk("[calib] %s():  null buffer\n",__FUNCTION__);
		goto end;
	}

	if(copy_from_user(value, buff, len1)) 
	{
		printk("[calib] %s(), copy from user failed!\n",__FUNCTION__);
		goto end;
	}

	if(!strncmp(value,"RXDCOC",6))
	{
		gCalib.curCalibId  =   CALIB_TEST_RXDCOC;
	}else
	if(!strncmp(value,"RC",2))
	{
		gCalib.curCalibId  =   CALIB_TEST_RC;
	}else
	if(!strncmp(value,"TXLOFT",6))
	{
		gCalib.curCalibId  =   CALIB_TEST_TXLOFT;
	}else
	if(!strncmp(value,"TXIQ",4))
	{
		gCalib.curCalibId  =    CALIB_TEST_TXIQ;
	}else
	if(!strncmp(value,"TXDPD",5))
	{
		gCalib.curCalibId  =    CALIB_TEST_TXDPD;
	}else
	if(!strncmp(value,"RXIQC_FI",8))
	{
		gCalib.curCalibId  =    CALIB_TEST_RXIQC_FI;
	}else
	if(!strncmp(value,"RXIQC_FD",8))
	{
		gCalib.curCalibId  =    CALIB_TEST_RXIQC_FD;
	}else
	if(!strncmp(value,"TXLPFG",6))
	{	
		gCalib.curCalibId  =    CALIB_TEST_TXLPFG;
	}else
	if(!strncmp(value,"RSSIDCOC",8))
	{	
		gCalib.curCalibId  =    CALIB_TEST_RSSIDCOC;
	}else
	{
		goto end;
	}
	printk("[calib] change calibId to %s\n",value);
	pItem = calib_table_get(gCalib.curCalibId);
	if(pItem)
	{
		calib_id_name_get(gCalib.curCalibId,value);
		sprintf(gCalib.fname,"/etc/%s_result.txt",value);
		calib_file_state_run(0);
		calib_file_state_run(1);
		calib_stat_headline_show(pItem);

	}
end:
	return len1;
}

static int calib_id_proc_show(struct seq_file *seq, void *v)
{
	char str[128]="";

	calib_id_name_get(gCalib.curCalibId,str);
	seq_printf(seq,"id: %s\n",str);	
	seq_printf(seq,"supprot list: [ |RXDCOC |RC |  TXLOFT | TXIQ  |TXDPD | RXIQC_FI | RXIQC_FD  | TXLPFG  |RSSIDCOC ]\n");
	 return 0;        
}


static int calib_id_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, calib_id_proc_show,NULL);
}



static int calib_file_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )

{
	char value[64], *end;
	
	if (buff && !copy_from_user(value, buff, len1)) {
		 gCalib.fstate = simple_strtoul(value, &end, 10);
		 printk("[calib]  change file stat  to %d\n",gCalib.fstate);
		 calib_file_state_run(gCalib.fstate);
	}		 
	return len1;
}


static int calib_file_proc_show(struct seq_file *seq, void *v)
{
	 seq_printf(seq,"calib_filename: %s, state: %d\n",gCalib.fname, gCalib.fstate);
	 return 0;        
}


static int calib_file_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, calib_file_proc_show, NULL);
}




static int calib_bw_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )

{
	char value[64], *end;
	unsigned int bw;
	
	if (buff && !copy_from_user(value, buff, len1)) {
		 bw = simple_strtoul(value, &end, 10);
		 printk("[calib] bw=%d\n",bw);
		 if(bw!=20  && bw!=40)
		 {
		 	printk("[calib] only support bw 20 or 40\n");
		 }else{
			 gCalib.bw = bw;
			printk("[calib]  change bw   to %d\n",gCalib.bw);
		 }
	}		 
	return len1;
}


static int calib_bw_proc_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,"bw: %d\n",gCalib.bw);
	 return 0;        
}


static int calib_bw_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, calib_bw_proc_show, NULL);
}



static const struct file_operations proc_file_fops = {
 .owner = THIS_MODULE, 	
 .open = calib_file_proc_open,
 .write  = calib_file_proc_write,
 .read  = seq_read,
  .llseek= seq_lseek,
 .release= single_release,
};


static const struct file_operations proc_id_fops = {
 .owner = THIS_MODULE, 	
 .open = calib_id_proc_open,
 .write  = calib_id_proc_write,
 .read  = seq_read,
  .llseek= seq_lseek,
 .release= single_release,
};

static const struct file_operations proc_state_fops = {
 .owner = THIS_MODULE, 
 .open = calib_state_proc_open,
 .write  = calib_state_proc_write,
 .read  = seq_read,
  .llseek= seq_lseek,
 .release= single_release,
};

static const struct file_operations proc_times_fops = {
 .owner = THIS_MODULE,
 .open = calib_times_proc_open,
 .write  = calib_times_proc_write,
 .read  = seq_read,
 .llseek= seq_lseek,
 .release= single_release,
};

static const struct file_operations proc_bw_fops = {
 .owner = THIS_MODULE,
 .open = calib_bw_proc_open,
 .write  = calib_bw_proc_write,
 .read  = seq_read,
 .llseek= seq_lseek,
 .release= single_release,
};






int calib_test_proc_init(void)
{
	calib_proc = proc_mkdir("calib_test",NULL) ;

	if (!calib_proc) 
	{
		printk(KERN_INFO "[calib] Create cakibration test  dir failed!!!\n");
		return -1;
	}

	state_entry = proc_create("stat", 0, calib_proc,&proc_state_fops);

	if (!state_entry) 
	{
		printk(KERN_INFO "[calib] Create state  entry  failed!!!\n");		
		remove_proc_entry("calib_test", NULL);
		return -1;
	}

	times_entry = proc_create("times", 0, calib_proc,&proc_times_fops);

	if (!times_entry) 
	{
		printk(KERN_INFO "[calib] Create times_entry  failed!!!\n");		
		remove_proc_entry("stat", calib_proc);		
		remove_proc_entry("calib_test", NULL);
		return -1;
	}

	calibId_entry = proc_create("calibId", 0, calib_proc,&proc_id_fops);

	if (!calibId_entry) 
	{
		printk(KERN_INFO "[calib] Create staId_entry  failed!!!\n");		
		remove_proc_entry("stat", calib_proc);		
		remove_proc_entry("times", calib_proc);
		remove_proc_entry("calib_test", NULL);
		return -1;
	}

	file_entry = proc_create("file", 0, calib_proc,&proc_file_fops);


	if (!file_entry) 
	{
		printk(KERN_INFO "[calib] Create file_entry  failed!!!\n");		
		remove_proc_entry("stat", calib_proc);		
		remove_proc_entry("times", calib_proc);		
		remove_proc_entry("calibId", calib_proc);
		remove_proc_entry("calib_test", NULL);
		return -1;
	}

	bw_entry = proc_create("bw", 0, calib_proc,&proc_bw_fops);


	if (!bw_entry) 
	{
		printk(KERN_INFO "[calib] Create bw_entry  failed!!!\n");		
		remove_proc_entry("stat", calib_proc);		
		remove_proc_entry("times", calib_proc);		
		remove_proc_entry("calibId", calib_proc);		
		remove_proc_entry("file", calib_proc);
		remove_proc_entry("calib_test", NULL);
		return -1;
	}

	printk(KERN_INFO "[calib] Create calib_test ok!!!\n");

	return 0;
}

int calib_test_proc_exit(void)
{
	remove_proc_entry("stat", calib_proc);		
	remove_proc_entry("times", calib_proc);	
	remove_proc_entry("calibId", calib_proc);	
	remove_proc_entry("file", calib_proc);	
	remove_proc_entry("bw", calib_proc);
	remove_proc_entry("calib_test", NULL);	
	calib_file_state_run(0);
	return 0;
}


