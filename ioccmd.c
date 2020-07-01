#include "ioc.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/device.h>
#include <linux/types.h>

struct fake_device {
	char data[100];
	struct semaphore sem;
} virtual_device;


static struct cdev *mcdev;
static int major_num;
static int ret;
static dev_t dev_num;
static struct class *cl;
static struct device *dev;
#define DEVICE_NAME	"ayushdevice"

static char message[5];
static int device_open(struct inode *inode, struct file *filep){
	if(down_interruptible(&virtual_device.sem) !=0){
		printk(KERN_ALERT "could not lock during open");
		return 1;
	}	
	printk(KERN_INFO "opened device");
	return 0;
}

static int device_close(struct inode *inode, struct file* filep){
	up(&virtual_device.sem);
	printk(KERN_INFO "closed device");
	return 0;
}

static long device_ioctl(struct file *fp, unsigned int cmd, unsigned long arg){
	char* userData;
	switch(cmd){
		case IOCTL_READ_DATA:
			userData = (char*) arg;
			printk(KERN_INFO "Data sending from user %s", message);
			copy_to_user(userData, message, 5);

			break;
		case IOCTL_WRITE_DATA:
			userData = (char*) arg;
			copy_from_user(message, userData, 5);
			printk(KERN_INFO "Data recieved from user %s", message);		
			break;

	}
	return 0;

}

static struct file_operations fops= {
	.owner = THIS_MODULE,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_close
};


static int driver_entry(void){
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret<0){
		printk(KERN_ALERT "Ayush something went wrong: failed to allocate major number");
	return ret;
	}
	major_num=MAJOR(dev_num);
	printk(KERN_INFO "Ayush major number is %d", major_num);
	cl = class_create(THIS_MODULE, "CharDeClass");
	if(cl==NULL){
		printk(KERN_ALERT "Could not create class");
		unregister_chrdev_region(dev_num, 1);
		return 0;
	}
	dev = device_create(cl , NULL, MKDEV(major_num, 0), NULL, "CharDe");
	if(dev==NULL){
		printk(KERN_ALERT "Could not create device");
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return 0;
	}
	mcdev= cdev_alloc();
	mcdev->ops= &fops;
	mcdev->owner = THIS_MODULE;
	ret = cdev_add(mcdev, dev_num, 1);
	if(ret<0){
		printk(KERN_ALERT "unable to add cdev to kernel");
		device_destroy(cl, MKDEV(major_num, 0));		
		class_destroy(cl);			
		unregister_chrdev_region(dev_num, 1);
		return 0;
	}
	sema_init(&virtual_device.sem, 1);
	return 0;
}

static void driver_exit(void){
	cdev_del(mcdev);
	device_destroy(cl, MKDEV(major_num, 0));
	class_destroy(cl);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "unloaded module");
}

MODULE_LICENSE("GPL");

module_init(driver_entry);
module_exit(driver_exit);

