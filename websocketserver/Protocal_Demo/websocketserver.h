	#include "./../include/libwebsockets.h"
	#include <signal.h>
	#include <string.h>
    #include <stdio.h>

    typedef  struct lws_mes {
        struct lws *wsi;
        unsigned char recvbuf[1024];
        unsigned char sendbuffer[1024];
        int sendnlen;
        int recvnlen;
    }lws_mes_t;

    typedef int (recv_callback_type) (lws_mes_t *,unsigned char *,int );
      
    static lws_mes_t lws_context;
    extern int  lws_init(int lws_callback_recv(lws_mes_t *,unsigned char *,int ));
    extern lws_mes_t *get_lwsctx();
    