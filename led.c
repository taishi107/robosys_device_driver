/*
 *led.c
 *
 *Copyright (C) 2017 Taishi Mikami <uroko100manen@gmail.com>
 *
 *This program is free software: you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <linux/timer.h>


MODULE_AUTHOR("Tiahi Mikami");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static volatile u32 *gpio_base =NULL;
static dev_t dev;
static struct cdev cdv;

static struct class *cls = NULL;

struct timer_list timer;
struct timer_list timer2;

char time = 1;
char flag = 1;
int iter = 0;
char mode;

static ssize_t led_timer(struct file* flip, const char* buf, size_t count, loff_t* pos)
{
    char str;
    
    if(copy_from_user(&str, buf,sizeof(char)))
        return -EFAULT;
	
    if(str>='0' && str<='9'){
        del_timer(&timer);
        time = (int)str - 48;
        mod_timer(&timer, jiffies + 1*HZ);
    }
    else if(str>='a' && str<='z'){
        mode = str;
    }
    flag = 1;
    iter = 0;

    return 1;
}
static void light_off(unsigned long arg){
    gpio_base[10] = 1 << 25;
    mod_timer(&timer, jiffies + 1*HZ);
    return 1;
}

static void led_write(unsigned long arg)
{
    switch(mode){
        case 't':
            gpio_base[7] = 1 << 25;
            if(flag == 0){
                gpio_base[10] = 1 << 25;
            }
            mod_timer(&timer, jiffies + time*HZ);
            flag = 0;
            break;
        case 'c':
            if(iter < (int)time){
                gpio_base[7] = 1 << 25;
                mod_timer(&timer2, jiffies + 1*HZ);
                iter++;
            }
            break;
        case 'k':
            gpio_base[10] = 1 << 25;
            break;
    }
    return 1;
}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_timer
};

static int __init init_mod(void)
{
	int retval;
	
	gpio_base = ioremap_nocache(0x3f200000, 0xA0);
	const u32 led = 25;
	const u32 index = led/10;
	const u32 shift = (led%10)*3;
	const u32 mask = ~(0x7 << shift);
	gpio_base[index] = (gpio_base[index] & mask ) | (0x1 << shift);
	
    init_timer(&timer);
    timer.expires = jiffies + time*HZ;
    timer.data = 0;
    timer.function = led_write;
    add_timer(&timer);

    init_timer(&timer2);
    timer2.expires = jiffies + time*HZ;
    timer2.data = 0;
    timer2.function = light_off;
    add_timer(&timer2);

    
	retval = alloc_chrdev_region(&dev,0,1,"myled");
	if(retval < 0){
		printk(KERN_ERR " alloc_chrdev_region failed.\n");
		return retval;
	}
	printk(KERN_INFO "%s is losded. major:%d\n",__FILE__,MAJOR(dev));
	
	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval <0){
		printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
		return retval;
	}

	cls = class_create(THIS_MODULE,"myled");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	
	device_create(cls, NULL, dev, NULL, "myled%d",MINOR(dev));

    
	return 0;
	
}




static void __exit cleanup_mod(void)
{
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__,MAJOR(dev));		
    
    del_timer(&timer);
    del_timer(&timer2);
}


module_init(init_mod);
module_exit(cleanup_mod);







