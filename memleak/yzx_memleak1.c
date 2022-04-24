
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define  Malloc(size)
extern void *__lib_malloc(size_t size);
int enable_malloc_hook = 1;   //递归的终止标志位


extern void __libc_free(void * p);
int enable_free_hook = 1;



void *malloc(size_t size) {
	
	//printf("malloc \n");
	if(enable_malloc_hook) {
		enable_malloc_hook  = 0;
		
		void *p = __libc_malloc(size);
		
		void *calloc = __builtin_return_address(0);
		
		printf("[+%p]malloc = %ld   p = %p\n",calloc,size,p);
		
		char buff[128] = {0};
		sprintf(buff,"%p.mem",p);
		FILE *fp = fopen(buff,"w");
		fprintf(fp,"[+%p]---->addr:%p,size:%ld\n",calloc,p,size);
		fflush(fp);
		close(fp);//此处是不能够调用close,因为close 会 调用free(),会打印printf("double free:%p\n",p);
		
		enable_malloc_hook = 1;
		return p;
	}else{
		return __libc_malloc(size);
	}
}
void free(void *p) {
	if(enable_free_hook) {
		enable_free_hook = 0;
		printf("free:%p\n",p);
		char buff[128] = {0};
		sprintf(buff,"%p.mem",p);
		if(unlink(buff) < 0) { //这是个系统函数unlink() < 0说明文件不存在,unlink函数作用是删除文件
			printf("double free:%p\n",p);
		}
		__libc_free(p);
		enable_free_hook = 1;
	}
	else{
		__libc_free(p);
	}
}

int main() 
{
	
	void *p1 = malloc(10);
	void *p2 = malloc(20);
	
	free(p1);
	return 0;
	
}