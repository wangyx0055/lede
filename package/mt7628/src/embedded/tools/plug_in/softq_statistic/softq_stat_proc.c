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
#include "softq_stat.h"

static struct proc_dir_entry *softq_proc=NULL;
static struct proc_dir_entry *state_entry=NULL, *times_entry=NULL,  *staId_entry=NULL, *file_entry=NULL;
extern  softq_stat_t softq_statistic;
extern unsigned int glen;
extern char *gBuffer;

 static int softq_state_show(void)
{
	int i;	
	unsigned int swq = 0, ring = 0;

	for(i=0;i<4;i++)
	{
		swq = softq_statistic.swQueueTotal[i] ? softq_statistic.swQueueTotal[i]/softq_statistic.swQueueCounter[i]:0;
		ring = softq_statistic.swQueueTotal[i] ? softq_statistic.txRingTotal[i]/softq_statistic.txRingCounter[i]:0;
		
		 printk("\tAC%d Drop=%d, SW_Q=%d, Ring=%d, APSendPacket=%d, DequeuePacket=%d\n", i, 
		 softq_statistic.swDropPacket[i],
		swq,
		ring,
		 softq_statistic.apSendPacket[i],
		 softq_statistic.apDequeuePacket[i]);
	}
	return 0;
 }

static  int softq_state_clean(void)
{
	int i;	
	for(i=0;i<4;i++)
	{
		 softq_statistic.swDropPacket[i] = 0;
		 softq_statistic.swQueueCounter[i] = 0;
		 softq_statistic.swQueueTotal[i] = 0;
		 softq_statistic.txRingCounter[i] = 0;
		 softq_statistic.txRingTotal[i] = 0;
		 softq_statistic.apSendPacket[i] = 0;
		 softq_statistic.apDequeuePacket[i] = 0;
		 softq_statistic.apPreSendPacket[i] = 0;
		 softq_statistic.apPreDequeuePacket[i] = 0;
	}	
	softq_statistic.counter = 0;

	return 0;
 }


static int softq_file_state_run(unsigned int fstate)
{
	switch(fstate){
	case SOFTQ_F_START:		
		softq_stat_fileOpen(softq_statistic.fname);		
	break;
	case SOFTQ_F_END:
		softq_stat_fileClose();		
		glen = 0;
		memset(gBuffer,0,GSIZE);		
	break;
	default:
	break;
	}
	return 0;
}




static int softq_state_proc_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,"softq_state: %s\n", 
	softq_statistic.isSoftqStat == SOFTQ_STATE_CLEAN ? "clean":
	softq_statistic.isSoftqStat == SOFTQ_STATE_REPORT ? "report":
	softq_statistic.isSoftqStat == SOFTQ_STATE_REPORT_INTR_PERIOD? "report periodical":
	softq_statistic.isSoftqStat == SOFTQ_STATE_REPORT_INTR_EACHTIME? "report each": "no operation");
	return 0;
}

static int softq_state_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, softq_state_proc_show,NULL);
}


static int softq_state_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char value[64], *end;
	
	if (buff && !copy_from_user(value, buff, len1)) {

		 softq_statistic.isSoftqStat = simple_strtoul(value, &end, 10);

		switch(softq_statistic.isSoftqStat){
		case SOFTQ_STATE_REPORT:
		{
			softq_state_show();			
			printk("\nshow don...\n");
		}
		break;
		case SOFTQ_STATE_CLEAN:
		{
			softq_state_clean();
			printk("\nclean don...\n");
		}
		break;
		case  SOFTQ_STATE_NOP:			
			printk("\nchange to no operation...\n");
		break;
		case SOFTQ_STATE_REPORT_INTR_PERIOD:			
			printk("\nchange to report periodic...\n");
		break;
		case SOFTQ_STATE_REPORT_INTR_EACHTIME:			
			printk("\nchange to report each time...\n");
		break;
		default:
			printk("\nplease echo: [ clean: %d  | report: %d  | report-period: %d  | report-each: %d  | nop: %d]\n",
			SOFTQ_STATE_CLEAN,
			SOFTQ_STATE_REPORT,
			SOFTQ_STATE_REPORT_INTR_PERIOD,
			SOFTQ_STATE_REPORT_INTR_EACHTIME,
			SOFTQ_STATE_NOP);
		break;
		}
	
		 
	}		 
	return len1;
}

static int softq_times_proc_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,"softq_times: %d\n",softq_statistic.times);
	 return 0;        
}


static int softq_times_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, softq_times_proc_show, NULL);
}


static int softq_times_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char value[64], *end;
	
	if (buff && !copy_from_user(value, buff, len1)) {

		 softq_statistic.times = simple_strtoul(value, &end, 10);
		 printk("change periodic time to %d\n",softq_statistic.times);
		 
	}		 
	return len1;
}



static int softq_staId_proc_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,"staId: %d\n",softq_statistic.staId);
	 return 0;        
}


static int softq_staId_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, softq_staId_proc_show, NULL);
}


static int softq_staId_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char value[64], *end;
	
	if (buff && !copy_from_user(value, buff, len1)) {

		 softq_statistic.staId = simple_strtoul(value, &end, 10);
		 printk("change  staId to %d\n",softq_statistic.staId);
		 
	}		 
	return len1;
}


static int softq_file_proc_show(struct seq_file *seq, void *v)
{
	 seq_printf(seq,"filename: %s, state: %d\n",softq_statistic.fname, softq_statistic.fstate);
	 return 0;        
}


static int softq_file_proc_open(struct inode *inode, struct file *file)
{
 	return single_open(file, softq_file_proc_show, NULL);
}



static int softq_file_proc_write( struct file *filp, const char *buff,unsigned long len1, void *data )

{
	char value[64], *end;
	
	if (buff && !copy_from_user(value, buff, len1)) {
		 softq_statistic.fstate = simple_strtoul(value, &end, 10);
		 printk("change file stat  to %d\n",softq_statistic.fstate);
		 softq_file_state_run(softq_statistic.fstate);
	}		 
	return len1;
}


static const struct file_operations proc_file_fops = {
 .owner = THIS_MODULE, 	
 .open = softq_file_proc_open,
 .write  = softq_file_proc_write,
 .read  = seq_read,
  .llseek= seq_lseek,
 .release= single_release,
};


static const struct file_operations proc_id_fops = {
 .owner = THIS_MODULE, 	
 .open = softq_staId_proc_open,
 .write  = softq_staId_proc_write,
 .read  = seq_read,
  .llseek= seq_lseek,
 .release= single_release,
};

static const struct file_operations proc_state_fops = {
 .owner = THIS_MODULE, 
 .open = softq_state_proc_open,
 .write  = softq_state_proc_write,
 .read  = seq_read,
  .llseek= seq_lseek,
 .release= single_release,
};

static const struct file_operations proc_times_fops = {
 .owner = THIS_MODULE,
 .open = softq_times_proc_open,
 .write  = softq_times_proc_write,
 .read  = seq_read,
 .llseek= seq_lseek,
 .release= single_release,
};



int softq_stat_proc_init(void)
{
	softq_proc = proc_mkdir("softq_stat",NULL) ;

	if (!softq_proc) 
	{
		printk(KERN_INFO "Create softq_stat dir failed!!!\n");
		return -1;
	}

	state_entry = proc_create("stat", 0, softq_proc,&proc_state_fops);

	if (!state_entry) 
	{
		printk(KERN_INFO "Create state  entry  failed!!!\n");		
		remove_proc_entry("softq_stat", NULL);
		return -1;
	}

	times_entry = proc_create("times", 0, softq_proc,&proc_times_fops);

	if (!times_entry) 
	{
		printk(KERN_INFO "Create times_entry  failed!!!\n");		
		remove_proc_entry("stat", softq_proc);		
		remove_proc_entry("softq_stat", NULL);
		return -1;
	}


	staId_entry = proc_create("staId", 0, softq_proc,&proc_id_fops);

	if (!staId_entry) 
	{
		printk(KERN_INFO "Create staId_entry  failed!!!\n");		
		remove_proc_entry("stat", softq_proc);		
		remove_proc_entry("times", softq_proc);
		remove_proc_entry("softq_stat", NULL);
		return -1;
	}

	file_entry = proc_create("file", 0, softq_proc,&proc_file_fops);

	if (!file_entry) 
	{
		printk(KERN_INFO "Create staId_entry  failed!!!\n");		
		remove_proc_entry("stat", softq_proc);		
		remove_proc_entry("times", softq_proc);
		remove_proc_entry("softq_stat", NULL);
		return -1;
	}

	printk(KERN_INFO "Create softq_stat ok!!!\n");

	return 0;
}

int softq_stat_proc_exit(void)
{
	remove_proc_entry("stat", softq_proc);		
	remove_proc_entry("times", softq_proc);	
	remove_proc_entry("staId", softq_proc);	
	remove_proc_entry("file", softq_proc);
	remove_proc_entry("softq_stat", NULL);
	return 0;
}


