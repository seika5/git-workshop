// https://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
// https://www.redhat.com/sysadmin/secure-boot-systemtap
// https://github.com/torvalds/linux/blob/master/drivers/char/mem.c

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathaniel Heidemann");

#define DEVICE_NAME "a"

static int major; // major number of this device

static const char a_buf[512] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

static ssize_t dev_read(struct file *f, char __user *u, size_t cnt, loff_t *off)
{
	if (!cnt)
		return 0;

	size_t set = 0;
	for (;;) {
		for (size_t i = 0; i < PAGE_SIZE / sizeof a_buf; i++) {
			size_t chunk = min_t(size_t, cnt, sizeof a_buf);
			size_t left = copy_to_user(u, a_buf, chunk);
			if (unlikely(left)) {
				set += chunk - left;
				if (!set)
					return -EFAULT;
				goto done;
			}
			set += chunk;
			cnt -= chunk;
			if (!cnt)
				goto done;
		}
		if (signal_pending(current))
			break;
		cond_resched();
	}

done:
	return set;
}

static ssize_t dev_write(struct file *f, const char __user *u, size_t cnt, loff_t *off)
{
	return 0;
}

static int dev_open(struct inode *in, struct file *f)
{
	//MOD_INC_USE_COUNT;
	return 0;
}

static int dev_release(struct inode *in, struct file *f) {
	//MOD_DEC_USE_COUNT;
	return 0;
}

static const struct file_operations fops = {
	.owner   = THIS_MODULE,
	.read    = dev_read,
	.write   = dev_write,
	.open    = dev_open,
	.release = dev_release,
};

static int __init m_init(void)
{
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0)
		return major;

	printk(KERN_INFO "loaded dev_a, assigned major number %d\n", major);
	return 0;
}

static void __exit m_exit(void)
{
	unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "unloaded dev_a");
}

module_init(m_init);
module_exit(m_exit);
