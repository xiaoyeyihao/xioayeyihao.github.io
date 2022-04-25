#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

#define MP_ALIGNMENT       		32  
#define MP_PAGE_SIZE			4096
#define MP_MAX_ALLOC_FROM_POOL	(MP_PAGE_SIZE-1)

#define mp_align(n, alignment) (((n)+(alignment-1)) & ~(alignment-1))
#define mp_align_ptr(p, alignment) (void *)((((size_t)p)+(alignment-1)) & ~(alignment-1))   //8个字节对齐

/*
此内存池组件 不对小块内存回收，只对大块内存回收
小块内存:4k
大块内存：>4k

*/

struct mp_large_s {
    struct mp_lagge_s *next;
    void *alloc;
};

struct mp_node_s {
    unsigned char *last;
    unsigned char *end;
    struct mp_node_s *next;
    size_t failed;
};

struct mp_pool_s {
    size_t max; 
    struct mp_node_s *curent;
    struct mp_large_s *large;
    struct mp_node_s head[0];
};

/*
    size:大块小块临界值大小
    返回值：内存池对象（struct mp_pool_s）
*/
static void * mp_alloc(struct mp_pool_s *pool,size_t size);
struct mp_pool_s * mp_create_pool(size_t size) {
    struct mp_pool_s *p;
    int ret = posix_memalign((void **)&p,MP_ALIGNMENT,size+sizeof(struct mp_pool_s)+sizeof(struct mp_node_s));//成功返回0，失败返回非0错误值
    if(ret)return NULL;
    p->max = (size < MP_MAX_ALLOC_FROM_POOL) ? size : MP_MAX_ALLOC_FROM_POOL;
    p->curent = p->head;
    p->large = NULL;

    p->head->last = (unsigned char *)p+sizeof(struct mp_pool_s)+sizeof(sizeof(struct mp_node_s));
    p->head->end = p->head->last + size;
    p->head->failed = 0;
    p->head->next = NULL;
    return p;

}
void mp_destory_pool(struct mp_pool_s *pool) {
    struct mp_node_s *h,*n;
    struct mp_large_s *l;   //释放大块
    for(l = pool->large;l;l = l->next) {  
        if(l->alloc)free(l->alloc);
    }
    h = pool->head->next;
    while(h) {                  //释放小块
        n = h->next;
        free(h);
        h = n;
    }
    free(pool);

}

void mp_reset_pool(struct mp_pool_s *pool) {
    struct mp_node_s *h;
    struct mp_large_s *l;
    for(l = pool->large;l;l=l->next) {
        if(l->alloc)free(l->alloc);
    }
    pool->large = NULL;
    for(h=pool->head->next;h;h = h->next) {
        h->last = (unsigned char *)h+sizeof(struct mp_node_s);
    }

}
/*
创建一个小块的空间和空间管理
*/
static void *mp_alloc_block(struct mp_pool_s *pool,size_t size) {
    unsigned char *m;
    struct mp_node_s *h = pool->head;
    size_t psize = (size_t)(h->end - (unsigned char *)h);//4k+sizeof(mp_node_s)
    int ret = posix_memalign((void **)&m,MP_ALIGNMENT,psize);
    if(ret)return NULL;
    struct mp_node_s *p,*new_node,*current;
    new_node = (struct mp_node_s *)m;
    new_node->next = NULL;
    new_node->last = m+sizeof(struct mp_node_s);
    new_node->end = m+psize;
    new_node->failed = 0;
    new_node->next = NULL;
    m = m + sizeof(struct mp_node_s);
    m = mp_align_ptr(m,MP_ALIGNMENT); //将地址m进行 32字节对齐。
    new_node->last = m + size;
    current = pool->curent;

    for(p = current;p->next;p = p->next) {
        if(p->failed++ > 4) {
            current = p->next;
        }
    }
    p->next = new_node;
    pool->curent = current?current:new_node;
    return m;
}

static void *mp_alloc_large(struct mp_pool_s *pool,size_t size) {
    void *p = malloc(size);
    if(p == NULL)return NULL;
    size_t n = 0;
    struct mp_large_s *large = pool->large;
    for(large =  pool->large;large;large = large->next) {
        if(large->alloc == NULL) {
            large->alloc = p;
            return p;
        }
        if(n++ > 3){
            break;
        }
    }
    large = mp_alloc(pool,sizeof(struct mp_pool_s));
    large->next = pool->large;
    pool->large = large;
    large->alloc = p;
    
}
static void * mp_alloc(struct mp_pool_s *pool,size_t size) {
    unsigned char *m;
    struct mp_node_s *p;
    if(size <= pool->max) {
        p = pool->curent;

        do {
           m = mp_align_ptr(p->last, MP_ALIGNMENT);
           if((size_t )(p->end - m) >= size) {
                p->last = m + size;
               return m;
           }
           p = p->next;
        }while(p);
        return mp_alloc_block(pool,size);    
    }
    else {
        mp_alloc_large(pool,size);
    }

}
//字节对齐的方式创建大块
void *mp_memalign(struct mp_pool_s *pool, size_t size, size_t alignment) {

	void *p;
	
	int ret = posix_memalign(&p, alignment, size);
	if (ret) {
		return NULL;
	}

	struct mp_large_s *large = mp_alloc(pool, sizeof(struct mp_large_s));
	if (large == NULL) {
		free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}

//和mp_alloc功能一样，但是mp_alloc 是字节对齐过的，此函数没有字节对齐
void *mp_nalloc(struct mp_pool_s *pool, size_t size) {

	unsigned char *m;
	struct mp_node_s *p;

	if (size <= pool->max) {
		p = pool->curent;

		do {
			m = p->last;
			if ((size_t)(p->end - m) >= size) {
				p->last = m+size;
				return m;
			}
			p = p->next;
		} while (p);

		return mp_alloc_block(pool, size);
	}

	return mp_alloc_large(pool, size);
	
}

void *mp_calloc(struct mp_pool_s *pool, size_t size) {

	void *p = mp_alloc(pool, size);
	if (p) {
		memset(p, 0, size);
	}

	return p;
	
}

void mp_free(struct mp_pool_s *pool, void *p) {

	struct mp_large_s *l;
	for (l = pool->large; l; l = l->next) {
		if (p == l->alloc) {
			free(l->alloc);
			l->alloc = NULL;

			return ;
		}
	}
	
}

int main(int argc, char *argv[]) {

	int size = 1 << 12;

	struct mp_pool_s *p = mp_create_pool(size);

	int i = 0;
	for (i = 0;i < 10;i ++) {

		void *mp = mp_alloc(p, 512);
//		mp_free(mp);
	}

	//printf("mp_create_pool: %ld\n", p->max);
	printf("mp_align(123, 32): %d, mp_align(17, 32): %d\n", mp_align(24, 32), mp_align(17, 32));
	//printf("mp_align_ptr(p->current, 32): %lx, p->current: %lx, mp_align(p->large, 32): %lx, p->large: %lx\n", mp_align_ptr(p->current, 32), p->current, mp_align_ptr(p->large, 32), p->large);

	int j = 0;
	for (i = 0;i < 5;i ++) {

		char *pp = mp_calloc(p, 32);
		for (j = 0;j < 32;j ++) {
			if (pp[j]) {
				printf("calloc wrong\n");
			}
			printf("calloc success\n");
		}
	}

	//printf("mp_reset_pool\n");

	for (i = 0;i < 5;i ++) {
		void *l = mp_alloc(p, 8192);
		printf("i = %d\n",i);
        mp_free(p, l);
	}

	mp_reset_pool(p);

	//printf("mp_destory_pool\n");
	for (i = 0;i < 58;i ++) {
		mp_alloc(p, 256);
	}

	mp_destory_pool(p);

	return 0;

}
