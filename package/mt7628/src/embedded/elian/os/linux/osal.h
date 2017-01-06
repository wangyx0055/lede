
#ifndef __MSC_OSAL_H__
#define __MSC_OSAL_H__

#include <linux/string.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>


#if 0
#define MSC_WARN printk
#define MSC_INFO printk
#define MSC_DBG printk
#endif


#define MSC_DEBUG_OFF        0
#define MSC_DEBUG_ERROR      1
#define MSC_DEBUG_WARN       2
#define MSC_DEBUG_TRACE      3
#define MSC_DEBUG_INFO       4
#define MSC_DEBUG_LOUD       5

extern unsigned long MSC_DEBUG_LEVEL;

#define MSC_PRINT(Level, Fmt) \
{                                   \
    if (Level <= MSC_DEBUG_LEVEL)      \
    {                               \
        printk Fmt;               \
    }                               \
}	



#define MAX_THREAD_NAME_LEN 16
typedef void (*P_TIMEOUT_HANDLER)(unsigned long);
typedef int (*thread_func)(unsigned long);

typedef struct _osal_timer
{
	struct timer_list timer;
	P_TIMEOUT_HANDLER timeoutHandler;
	unsigned long data;
}osal_timer, *p_osal_timer;

typedef struct _osal_thread
{
	struct task_struct *thread;
	void *thread_func;
	void *data;
	char name[MAX_THREAD_NAME_LEN];
}osal_thread, *p_osal_thread;


extern void* osal_memset(void *buf, int i, unsigned int len);
extern void* osal_memcpy(void *dst, const void *src, unsigned int len);
extern int osal_memcmp(const void *buf1, const void *buf2, unsigned int len);
extern int osal_timer_modify(p_osal_timer ptimer, unsigned ms);
extern int osal_timer_stop(p_osal_timer ptimer);
extern int osal_timer_start(p_osal_timer ptimer, unsigned ms);
extern int osal_timer_create(p_osal_timer ptimer);
extern int osal_thread_create(p_osal_thread thread);
extern int osal_thread_run (p_osal_thread thread);
extern int osal_thread_stop (p_osal_thread thread);
extern int osal_thread_should_stop (p_osal_thread thread);
//extern int osal_msleep(unsigned int ms);
extern unsigned int  osal_strlen(const char *str); 
extern int osal_strcmp(const char *dst, const char *src);
extern int osal_strncmp(const char *dst, const char *src, unsigned int len);
extern char * osal_strcpy(char *dst, const char *src); 
extern long int osal_strtol(const char *str, char **c, int adecimal);
extern char *osal_strstr(const char *haystack, const char *needle);
extern int osal_lock_init(spinlock_t *l);
extern int osal_lock(spinlock_t *l);
extern int osal_unlock(spinlock_t *l);
#endif
