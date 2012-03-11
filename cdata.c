#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "cdata_ioctl.h"

#define MSG(arg) printk(KERN_INFO ##arg)

#define BUF_SIZE 128
#define LCD_SIZE 320*240

static struct cdata_t
{
    unsigned long *fb; //fb is Frame buffer (display), 4 byte alignment
    unsigned char *buf; //buf is a temperal buffer for input data, one byte per writing
    unsigned int fb_offset; //fb_offset is the offset of FB point
    unsigned int buf_offset; //buf_offset is the offset of buf
    
    struct timer_list flush_timer;
    struct timer_list sched_timer;
    
    wait_queue_head_t wq;
};

static int cdata_open(struct inode *inode, struct file *filp)
{
    //初始化的東西放在open driver. 記得離開driver的時候要將該清除的東西放在close

    //開始一個FB Drawer, 利用 ioremap(), 由於要考慮re-entrance 機制, 所以使用一個struct 去放到private_data
    struct cdata_t *cdata;
    int major, minor;

    major = MAJOR(inode->i_rdev);
    minor = MINOR(inode->i_rdev);
    printk(KERN_INFO "Cdata open: Number -1: %d-%d\n", major, minor);
    
        
    cdata = (struct cdata_t *)kmalloc(sizeof(struct cdata_t), GFP_KERNEL);
    
    cdata->fb = ioremap(0x33F00000, LCD_SIZE*4);
    cdata->buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    cdata->buf_offset = 0;
    cdata->fb_offset = 0;

    printk(KERN_INFO "ioremap address: %08x\n", cdata->fb);
    
    init_timer(&cdata->flush_timer);
    init_timer(&cdata->sched_timer);
    
    init_waitqueue_head(&cdata->wq);
    
    filp->private_data = (void *)cdata;

    return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
    
    MSG("Cdata close\n");
    
    del_timer(&cdata->flush_timer);
    del_timer(&cdata->sched_timer);
    
    kfree(cdata->buf);
    kfree(cdata);

    return 0;
}

static int cdata_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{

    MSG("Cdata ioctl\n");
    return 0;
}

static int cdata_flush(struct file *filp)
{
    //flush是 count !=0 的時候, 而Application呼叫close(), 一定會被呼叫到. 所以只需要struct file
    
    MSG("Cdata flush\n");
    
    return 0;
}

static ssize_t cdata_read(struct file *filp, const char *buf, size_t size, loff_t *off)
{
    //read & write driver function 不需要inode, 只需要struct file + buf + size + kernel已經做了幾筆
    
    MSG("Cdata read\n");
    
    return 0;
}

static void cdata_wakeup(unsigned long priv)
{
    struct cdata_t *cdata = (struct cdata_t *)priv;
    
    wait_queue_head_t *wq;
    struct timer_list *sched_timer;
    
    //lock
    wq = &cdata->wq;
    sched_timer = &cdata->sched_timer;
    //unlock
    
    //using waitup() to change process state as TASK_RUNNING
    wake_up(wq);
    
    // update the process scheduling
    sched_timer->expires = jiffies + 10;
    add_timer(sched_timer);
    
}


static void flush_lcd(unsigned long priv)
//static void flush_lcd(void * priv)
{

    struct cdata_t *cdata = (struct cdata_t *)priv;
    unsigned char *fb = NULL;
    unsigned char *linebuf;
    unsigned int buf_offset, fb_offset;
    unsigned int i;

    //lock
    fb = (unsigned char *)cdata->fb;
    linebuf = cdata->buf;
    buf_offset = cdata->buf_offset;
    fb_offset = cdata->fb_offset;
    //unlock

    for (i=0; i < buf_offset; i++)
    {
        //since the fb is belong to IO memory
        //printk(KERN_INFO "fb addr: %d ; fb_offset: %d\n", fb, fb_offset);
        
        writeb(linebuf[i], fb + fb_offset);
        fb_offset++;
        
        if (fb_offset >= LCD_SIZE * 4)
        {
            printk(KERN_INFO "BANGO: %d\n", fb_offset);
            fb_offset = 0;
        }
        
    }

    cdata->fb_offset = fb_offset;
    cdata->buf_offset = 0;

}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{
    //read & write driver function 不需要inode, 只需要struct file + buf + size + kernel已經做了幾筆
    
    //改成用flush_lcd function to draw data into FB. Design the small buf for application sending the data.
    
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
    unsigned char *linebuf = NULL;
    unsigned int buf_offset = 0;
    int i=0;
    
    struct timer_list *flush_timer;
    struct timer_list *sched_timer;
    wait_queue_head_t *wq;
    wait_queue_t wait;
    
    //MSG("Cdata write\n");
        
    //lock
    linebuf = cdata->buf;
    buf_offset = cdata->buf_offset;
    flush_timer = &cdata->flush_timer;
    sched_timer = &cdata->sched_timer;
    wq = &cdata->wq;
    //unlock
    
    //printk(KERN_INFO "init-Buf addr: %d ; Buf val: %d\n", linebuf, buf_offset);
   
    for (i=0; i<size; i++)
    {
        if (buf_offset >= BUF_SIZE)
        {
            //printk(KERN_INFO "linebuff full !\n");
            
            cdata->buf_offset = buf_offset;

             /**
set the timer, put flush_lcd() into flush_timer
also change ((void *)cdata) --> ((unsigned long)cdata)
the following timer will involke the flush_lcd() after one sec
*/
            flush_timer->expires = jiffies + 1*HZ;
            flush_timer->data = (unsigned long)cdata;
            flush_timer->function = flush_lcd;
            add_timer(flush_timer);
            
            /**
owing to being not able to change the process state in interrupt mode, we have to design a wait_quene + a timer to change the current process state. Round-Robin schduler will release CPU controlling right after this process is set as TASK_RUNNING state
*/

            
            sched_timer->expires = jiffies + 10;
            sched_timer->data = (unsigned long)cdata;
            sched_timer->function = cdata_wakeup;
            add_timer(sched_timer);

            wait.flags = 0;
            wait.task = current;
            add_wait_queue(wq, &wait);
                        
repeat:
            current->state = TASK_INTERRUPTIBLE;
            schedule();
            
            buf_offset = cdata->buf_offset;
            
            if (buf_offset != 0)
            {
                goto repeat;
            }
            
            //remove wait queue node: wait
            remove_wait_queue(wq, &wait);
            del_timer(sched_timer);
        }

        copy_from_user(linebuf+buf_offset, buf+i, 1);
        buf_offset++;

        //printk(KERN_INFO "loop-Buf addr: %d ; Buf val: %d\n", linebuf, buf_offset);
    }
    
    cdata->buf_offset = buf_offset;
    
   
    return 0;
}

int cdata_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long from;
	unsigned long to;
	unsigned long size;

	printk(KERN_INFO "This is mmap driver \n");

    /* 這個範例是一次配一大塊記憶體, 除非確定你會拿到完整連續的記憶體才能這樣寫 */
    //

	from = vma->vm_start;
	to  = 0x33f00000;
	size = vma->vm_end - vma->vm_start;
	//remap_page_range(from, to, size, PAGE_SHARED);

    //

	/*
     * 正確寫法 
   * PAGE_SHARED 使用時機為user space & kernel space會公用這塊資料
   *  remap_page_range()適用在reserved memory , 用它來建立page table
    */	
	while(size)
	{
		remap_page_range(from, to, PAGE_SIZE, PAGE_SHARED);
		from += PAGE_SIZE;
		to += PAGE_SIZE;
		size -=PAGE_SIZE;
	}


	printk(KERN_INFO "vma start: %08x\n", vma->vm_start);
	printk(KERN_INFO "vma end: %08x\n", vma->vm_end);

	return 0;
}

static struct file_operations cdata_fops =
{
	owner: THIS_MODULE, //讓kernel自己去控制INC / DEC
	open: cdata_open, //open(), close(), 都會使用struct inode and struct file
	release: cdata_close, //struct file的指標通常用 *filp
	ioctl: cdata_ioctl, //ioctl()需要四個參數:
		                //inode & file & unsigned int cmd & unsigned long arg （address)
	flush: cdata_flush,
	read: cdata_read,
	write: cdata_write,
	mmap: cdata_mmap,
};

static int cdata_module_init(void)
{
    //利用register_chrdev()去註冊, 透過major number + fops, 當然還要給個名子
    //在kernel裡面的return value只要是0代表成功. 負值代表失敗
    
    if (register_chrdev(121, "cdata", &cdata_fops) !=0)
    {
        MSG("cannot open\n");
        return -1;
    }
    return 0;
}

static int cdata_module_exit(void)
{
    unregister_chrdev(121, "cdata");
    return 0;
}

module_init(cdata_module_init);
module_exit(cdata_module_exit);

MODULE_LICENSE("GPL");
