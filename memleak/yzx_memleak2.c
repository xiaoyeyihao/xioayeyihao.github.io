
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *malloc_hook(size_t size,const char *file,int line) {
	void *p = malloc(size);
	char buff[128] = {0};
	sprintf(buff,"%p.mem",p);
	FILE *fp = fopen(buff,"w");
	fprintf(fp,"[+%s,:%d]---->addr:%p,size:%ld\n",file,line,size);
	fflush(fp);
	fclose(fp);
	return p;
	
}

void free_hook(void *p,const char *file,int line) {
	char buff[128] = {0};
	sprintf(buff,"%p.mem",p);
	if(unlink(buff) < 0) { //这是个系统函数unlink() < 0说明文件不存在,unlink函数作用是删除文件
		printf("double free:%p\n",p);
	}
	free(p);	
}

#define   malloc(size) malloc_hook(size,__FILE__,__LINE__)
#define   free(p)    free_hook(p,__FILE__,__LINE__)    //在预编译的时候就将malloc_hook   和free_hook  把malloc 和  free给替换掉了

int main() 
{
	
	void *p1 = malloc(10);
	void *p2 = malloc(20);
	
	free(p1);
	return 0;
	
}