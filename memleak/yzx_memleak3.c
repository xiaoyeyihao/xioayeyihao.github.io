
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
//方案3 此方法是一个不推荐的方法
typedef void *(*malloc_hook_t)(size_t size,const void *caller3
0);
malloc_hook_t   malloc_f;

typedef    void(*free_hook_t)(void *p,const void *caller);
free_hook_t  free_f;


int replaced = 0;
void *malloc_hook_f(size_t size,const void *caller) {
	mem_untrace();
	void *p = malloc(size);
	
	
	//printf("+%p:addr[%p]\n",caller,ptr);
	
	
	char buff[128] = {0};
	sprintf(buff,"%p.mem",p);
	FILE *fp = fopen(buff,"w");
	fprintf(fp,"[+%p]---->addr:%p,size:%ld\n",calloc,p,size);
	fflush(fp);
	close(fp);//此处是不能够调用close,因为close 会 调用free(),会打印printf("double free:%p\n",p);
	mem_trace();
	return p;
	
}

void free_hook_f(void * p,const void *caller) {
	
	mem_untrace();
	//printf("free+%p:addr[%p]\n",caller,p);
	char buff[128] = {0};
	sprintf(buff,"%p.mem",p);
	if(unlink(buff) < 0) { //这是个系统函数unlink() < 0说明文件不存在,unlink函数作用是删除文件
		printf("double free:%p\n",p);
	}
	free(p);
	mem_trace();
	
}
void mem_trace(void) { //mtrace
    replaced = 1;
	malloc_f = __malloc_hook;//__malloc_hook 是一个函数指针   调用malloc函数的时候，会调用__malloc_hook，free函数如下同理
	free_f = __free_hook;
	
	__malloc_hook = malloc_hook_f;
	__free_hook = free_hook_f;  //挂钩子
	
}
void  mem_untrace(void) {
	
	__malloc_hook = malloc_f;
	__free_hook = free_f;
	 replaced = 0;
}
int main() 
{
	
	
	mem_trace();
	void *p1 = malloc(10);
	void *p2 = malloc(20);
	
	
	free(p1);
	mem_untrace();
	return 0;
	
}