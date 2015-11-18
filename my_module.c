/* my_module.c: Example char device module.
 *
 */
/* Kernel Programming */
#define MODULE
#define LINUX
#define __KERNEL__

#include <linux/kernel.h>  	
#include <linux/module.h>
#include <linux/fs.h>       		
#include <asm/uaccess.h>
#include <linux/errno.h>  
#include <linux/sched.h>
#include <linux/slab.h>

#include "my_module.h"
#define MY_DEVICE "my_device"
#define SIZE_BUFFER 4096

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anonymous");
/***************************************************************/
/* globals */
int my_major = 0; /* will hold the major # of my device driver */
static char* buffer;	   				   //the main buffer of the device to be written to
static int deviceOpen = 0; //prevent multiple access to the device.
int writePivot = 0; //points to the last written char to 'buffer'.
int readPivot = 0;  //points to the last read char from 'buffer'.
struct file_operations my_fops = {
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .ioctl = my_ioctl
};
/***************************************************************/
int init_module(void)
{
	printk("in init_module\n");
    my_major = register_chrdev(my_major, MY_DEVICE, &my_fops);

    if (my_major < 0)
    {
		printk(KERN_WARNING "can't get dynamic major\n");
		return my_major;
    }
	
    //
    // do_init();
    //
    return 0;
}
/***************************************************************/
void cleanup_module(void)
{
    unregister_chrdev(my_major, MY_DEVICE);

	
    //
    // do clean_up();
    //
    return;
}
/***************************************************************/
int my_open(struct inode *inode, struct file *filp)
{
	if(deviceOpen) {
		return -EBUSY;
	}
	deviceOpen++;
	buffer = kmalloc(SIZE_BUFFER * sizeof(char), GFP_KERNEL);
	writePivot = 0; //move to global.
	readPivot = 0;
	printk("my_open: writePivot = %d, readPivot = %d\n", writePivot, readPivot);
    if (filp->f_mode & FMODE_READ)
    {
		//
		// handle read opening
		//
    }
    
    if (filp->f_mode & FMODE_WRITE)
    {
        //
        // handle write opening
        //
    }
	MOD_INC_USE_COUNT;
    return 0;
}
/***************************************************************/
int my_release(struct inode *inode, struct file *filp)
{
	kfree(buffer);
	deviceOpen--;
    if (filp->f_mode & FMODE_READ)
    {
		//
		// handle read closing
		// 
    }
    
    if (filp->f_mode & FMODE_WRITE)
    {
        //
        // handle write closing
        //
    }
	MOD_DEC_USE_COUNT;
    return 0;
}
/***************************************************************/
/*
* @param buf: the buffer to add the read chars to.
* @param count: the number of chars to be read. */
ssize_t my_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	printk("in my_read\n");
	if((buf == NULL) || (count < 0)) {
		return 0;
	}		
	int readData = 0;
	if((readPivot + count) > writePivot) {
		readData = (writePivot - readPivot  ) ;
	} else {
		readData = count;
	}   	
	int len = 0;
	while(readData != 0) {
		put_user(*(buffer + readPivot - 1 +  len), buf++);
        readData--;
        len++;
	}
	readPivot += len; 
	return len; 		
}
/***************************************************************/
// concatenate the new data to the data which already in the buffer.
// if the buffer does not have enough space to support the new data, don't write anything.
// In this implementation, there is no '\0' between each 2 writes:
//		- write 'hello' [buffer = "hello"]
//		- write 'world' [buffer = "helloworld", and *NOT* "hello\0world\0"]
// @return: number of bytes written to the buffer. 
ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{	
	printk("my_write: writing %s", buf);
	if((buf == NULL) || (count < 0)) {
		printk(KERN_WARNING "ERROR: Invalid arguments in function my_write\n");
    	return -EINVAL;
	}
	int remainingLength = SIZE_BUFFER - writePivot - 1; //(-1) for 0-base counting
	if(count  > remainingLength) { 
		printk("my_write Error: no memory");
		return -ENOMEM;
	}	
	
	copy_from_user(buffer + writePivot - 1, buf, count);	
	writePivot += count ;		

    return count; 
}
/***************************************************************/
int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{	
    switch(cmd)
    {
    case MY_RESET:
		writePivot = 0;
		readPivot  = 0;
	break;
	case MY_RESTART:
		readPivot = 0;
	break;
    default:
		return -ENOTTY;
    }

    return 0;
}
