#include <linux/module.h> // module library
#include <linux/kernel.h> // kernel library
#include <linux/fs.h> // open/close and read/write library
#include <linux/cdev.h> // char device
#include <linux/semaphore.h> //used for synchronization behaviors
//#include <asm/uaccess.h>   
#include <linux/uaccess.h> //copy_to/from_user
#include <linux/random.h> //random library

// create structure
struct fake_device{
    char data[100];
    struct semaphore sem;
} virtual_device;

// register device
struct cdev *mcdev;  
int major_number; 
int ret; //to hold return values of functions; kernel stack is very small
 
dev_t dev_num; // hold major number

#define DEVICE_NAME  "tadad_device"
 
//user open
int device_open(struct inode *inode, struct file *file){
    //allow one process to open
    if(down_interruptible(&virtual_device.sem) != 0){
        printk(KERN_ALERT "ERROR: could not lock device during open");
        return -1;
    }
    
    printk(KERN_INFO "INFO: opened device");
    return 0;
}

//get information from device
ssize_t device_read(struct file* file, char* bufStoreData, size_t bufCount, loff_t* curOffset){
    //take data from kernel space to user space 
    printk(KERN_INFO "INFO: Reading from device");
    ret = copy_to_user(bufStoreData, virtual_device.data, bufCount);
    return ret;
}
 
//send information to device
ssize_t device_write(struct file* file, const char* bufSourceData, size_t bufCount, loff_t* curOffset){
    //send data from user to kernel
    printk(KERN_INFO "INFO: writing to device");
    ret = copy_from_user(virtual_device.data, bufSourceData, bufCount);
    return ret;
}
 
//user close
int device_close(struct inode *inode, struct file *file){
    up(&virtual_device.sem);
    printk(KERN_INFO "INFO: closed device");
    return 0;
}
 
//which functions to call when operating on fake device file
struct file_operations fops={
    .owner = THIS_MODULE,   // prevent unloading when in use
    .open = device_open,    
    .release = device_close,  
    .write = device_write, 
    .read = device_read
};
 
 
//register device with system
static int driver_entry(void){
    //use dynamic allocation to assign our device
    ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
    if (ret < 0) { // if return negative == error
        printk(KERN_ALERT "ERROR: failed to allcate a major number");
        return ret;
    }
    major_number= MAJOR(dev_num); //extract major number and store in our variable 
    printk(KERN_INFO "INFO: major number is %d", major_number);
    printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file", DEVICE_NAME, major_number); //dmesg

    mcdev = cdev_alloc(); //create and initialize cdev structure
    mcdev->ops = &fops;
    mcdev->owner = THIS_MODULE;
    //cdev, add it to kernel
    ret = cdev_add(mcdev,dev_num,1);
    if(ret < 0) { 
        printk(KERN_ALERT "ERROR: unable to add cdev to kernel");
        return ret;
    }
    //initialize semaphore
    sema_init(&virtual_device.sem,1);
 
    return 0;
}
 
static void driver_exit(void){
    //unregister everything in reverse order
    cdev_del(mcdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_ALERT "INFO: unloaded module");
}

//inform kernel where to start/stop with the module/driver
module_init(driver_entry);
module_exit(driver_exit);
