
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <mcheck.h> //mtrace 需要包含的头文件
//方案4  mtrace
int main() 
{
	
	
	mtrace();
	void *p1 = malloc(10);
	void *p2 = malloc(20);
	
	
	free(p1);
	muntrace();
	return 0;
	
}