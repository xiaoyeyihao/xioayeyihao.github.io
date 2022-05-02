#include "./websocketserver.h"
#include "./CJson/cJSON.h"
#include <iostream>

using namespace std;
typedef struct rsu_info {
    double lon;
    double lat;
    double high;
    long rsu_time;
    int rsustate;
    int gnssstate;
};
typedef struct {
    double lon;
    double lat;
    int ele;
}pos_t;
typedef struct rsi_rte_Date {
    //int type; //交通标识还是交通事件
    //int rteId; //10011;
    //int duaring;//120
   // int interval;//100
    int eventtype;
    string description;//事件的描述，随便填充
    // 交通事件的位置
    long lat;
    long lon;
    long ele;
   // int eventRadius;//100 事件范围
    //int priority;


}rsi_rte_Date_t;

typedef struct rsi_rts_Date {
    int signtype;//交通标识类型
    pos_t  signpos;
    string description;


}rsi_rts_Date_t;

typedef struct rsm_parter {
    pos_t pos;
    int speed;
    int heading;
    int angle;
    int width;
    int length;
    int height;
    unsigned char id[6];
}rsm_parter_t;
typedef struct rsi_rsm {
    int datsnum;
    rsm_parter_t  parter[8];
}rsi_rsm_t;

typedef struct RsiDate {
    int rte_Msgcount;    // 交通事件或者交通标识个数
    rsi_rte_Date_t rtearray[8];
    int rts_Msgcount;
    rsi_rts_Date_t  rtsarray[8];  
}RsiDate_t;

typedef struct Bsmdata {
    int obuid;
    int secmark;
    unsigned char vehicleid[24];

    long lon;
    long lat;
    int ele;

    int transmission;
    int speed;
    int heading;

    int width;
    int length;

    int accelset_lon;
    int accelset_lat;
    int accelset_vert;
    int accelset_yaw;

    
}Bsmdata_t;

static rsu_info  inforsu;

static int rsctl_rsi = 0;
static int rsctl_response = 0; 
static int rsctl_rsm = 0;
static int rteid = 0;
static int rtsid = 0;
static Bsmdata_t *bsm = NULL;
int parse_rsu2mec_bsm(cJSON * root,Bsmdata_t **bsm);
int send2rsu(lws_mes_t *lwsctx) {
    if(lwsctx->wsi == NULL)return -1;
    int ret  = lws_write(lwsctx->wsi, lwsctx->sendbuffer, lwsctx->sendnlen, LWS_WRITE_TEXT);
    return ret;
}
int parse_register(cJSON * root) {
    int ret;
    int rsutime;
    cJSON *child6;
    cJSON *child5;
    cJSON *child4;
    cJSON *child3;
    cJSON *child2;
    cJSON *child1 = cJSON_GetObjectItem(root, "lat");
    if(NULL == child1)
    {
        ret = -1;
        goto END;
    }
    inforsu.lat = child1->valuedouble;

    child2 = cJSON_GetObjectItem(root, "lon");
    if(NULL == child2)
    {
        ret = -1;
        goto END;
    }
    inforsu.lon = child2->valuedouble;

    child3 = cJSON_GetObjectItem(root, "ele");
    if(NULL == child3)
    {
        ret = -1;
        goto END;
    }
    inforsu.high = child3->valuedouble;

    child4 = cJSON_GetObjectItem(root, "rsutime");
    if(NULL == child4)
    {
        ret = -1;
        goto END;
    }
     rsutime = child4->valuedouble;

     child5 = cJSON_GetObjectItem(root, "rsustatus");
    if(NULL == child5)
    {
        ret = -1;
        goto END;
    }
    inforsu.rsustate = child5->valuedouble;

   child6 = cJSON_GetObjectItem(root, "gnssstatus");
    if(NULL == child6)
    {
        ret = -1;
        goto END;
    }
    inforsu.gnssstate = child6->valuedouble;
   
   // cj.Get("ele",inforsu.high);
   // cj.Get("rsctl",rsctl);
    //cj.Get("rsutime",inforsu.rsu_time);
    //cj.Get("rsustatus",inforsu.rsustate);
   // cj.Get("gnssstatus",inforsu.gnssstate);
   END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }
    return 0;
}
int response_register(lws_mes_t *lwsctx) {
    char * str = "123";
    cJSON *root;
    char* str1 = NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root,"frametype",0xc1);
    cJSON_AddNumberToObject(root,"rsctl",rsctl_response++);
    cJSON_AddNumberToObject(root,"acktype",1);
    cJSON_AddBoolToObject(root,"rcvstatus",true);
    cJSON_AddNumberToObject(root,"errstatus",0);
    cJSON_AddNumberToObject(root,"rsuid",123);
    cJSON_AddStringToObject(root,"rsuesn",str);

    str1 =  cJSON_Print(root);





    // CJsonObject cj;
    // cj.Add("frametype",0xc1);
    // cj.Add("rsctl",rsctl);
    // cj.Add("acktype",1);
    // cj.Add("rcvstatus",true);
    // cj.Add("errstatus",0);
    // cj.Add("rsuid",123);
    // cj.Add("rsuesn",str);
    // printf("lws_context.wsi = %p\n",lwsctx->wsi);
    memcpy(lwsctx->sendbuffer,str1,strlen(str1));
    if(lwsctx->wsi) {
         //printf("send starts\n");
         lws_write(lwsctx->wsi, lwsctx->sendbuffer, strlen(str1), LWS_WRITE_TEXT );
         //printf("send success\n");
     }
        
    
    //cj.Clear();
    return 0;
    
}
int lws_callback_recv(lws_mes_t *lwsctx,unsigned char *recvbuf,int nlen) {
    int frametype;
    int ret;
     cJSON * root = cJSON_Parse((const char *)recvbuf);
     if(root == NULL){
       // printf("date_error:cjon parse fault\n");
       // printf("recvbuf = %s\n",recvbuf);
        return -1;
     }
    cJSON *child1 = cJSON_GetObjectItem(root, "frametype");
    if(NULL == child1)
    {
        ret = -1;
        goto END;
    }
    //int frametype;
    frametype = child1->valueint;

    if(frametype == 0xB0) {    //RSU的注册消息
        int ret = parse_register(root);
        response_register(lwsctx);
    }
    if(frametype == 0xb1) {
        printf("收到BSM\n");
        parse_rsu2mec_bsm(root,&bsm);
    }
    if(frametype == 0xc1){
        //rsu发送过来的响应消息
    }
    if(frametype == 0xd2)
    {

    }
     END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }


}
int platcontrol2rsu(unsigned char *recvbuf,int nlen,lws_mes_t *lwsctx) {
    cJSON *root = cJSON_Parse((const char *)recvbuf);
    if(NULL == root){
        printf("root parse error\n");
        return -1;

    }


    cJSON_AddNumberToObject(root,"frametype",0xd1);
    
    char *sendbuff = cJSON_Print(root);
    
    if(sendbuff && lwsctx){
        memcpy(lwsctx->sendbuffer,sendbuff,strlen(sendbuff));
        int ret = send2rsu(lwsctx);
        return ret;
    }
    printf("sendbuffer is NULL\n");
    return -1;
}
int rsu2plantcontrol(unsigned char *recvbuf,int nlen,lws_mes_t *lwsctx) {
    
    cJSON *root = cJSON_Parse((const char *)recvbuf);
    root = cJSON_DetachItemFromObject(root,"frametype");
    if(root)
        return -1;
    char *sendbuff = cJSON_Print(root);
    
    if(sendbuff && lwsctx){
        memcpy(lwsctx->sendbuffer,sendbuff,strlen(sendbuff));
        int ret = send2rsu(lwsctx);
        return ret;
    }
    return -1;
    
}

int mec2rsu_rsirte(RsiDate_t *rsi_msg,lws_mes_t *lwsctx) {
       cJSON *root;
       cJSON *array;
       cJSON *repos;
       root = cJSON_CreateObject();
       array = cJSON_CreateArray();
       repos = cJSON_CreateObject();
       cJSON_AddNumberToObject(root,"frametype",0xc2);
       cJSON_AddNumberToObject(root,"rsctl",rsctl_rsi++);
       cJSON_AddNumberToObject(root,"rsuid",52);
       cJSON_AddStringToObject(root,"rsuesn","529c9c7d-a3b9-4560-8288-c80aa91798ac");//设备序列号
       //RSU 经纬度
       cJSON_AddNumberToObject(repos,"lon",113.300348777);
       cJSON_AddNumberToObject(repos,"lat",23.566940348);
       cJSON_AddNumberToObject(repos,"ele",0);
       cJSON_AddItemToObject(root,"refpos",repos);   
       for(int i = 0;i < rsi_msg->rte_Msgcount;i++) {
           
           cJSON *arrayobj = cJSON_CreateObject();
           cJSON_AddNumberToObject(arrayobj,"type",101);
           cJSON_AddNumberToObject(arrayobj,"rteId",rteid++);
           cJSON_AddNumberToObject(arrayobj,"eventtype",rsi_msg->rtearray[i].eventtype);
           cJSON_AddStringToObject(arrayobj,"description",rsi_msg->rtearray[i].description.c_str());
           cJSON_AddNumberToObject(arrayobj,"duration",5);
           cJSON_AddNumberToObject(arrayobj,"interval",100);
           cJSON_AddNumberToObject(arrayobj,"eventradius",1000);
           if(rsi_msg->rtearray[i].lat != 0 || rsi_msg->rtearray[i].lon != 0 ) {
               cJSON *eventpos = cJSON_CreateObject();
               cJSON_AddNumberToObject(eventpos,"lat",rsi_msg->rtearray[i].lat);
               //eventpos.Add("lat",rsi_msg->rtearray[i].lat);
               cJSON_AddNumberToObject(eventpos,"lon",rsi_msg->rtearray[i].lon);
               cJSON_AddNumberToObject(eventpos,"ele",0);
               //eventpos.Add("lon",rsi_msg->rtearray[i].lon);
              // cj["datamsg"][i].Add("eventpos",eventpos);
               cJSON_AddItemToObject(arrayobj,"eventpos",eventpos);
           }
          cJSON_AddNumberToObject(arrayobj,"priority",1);  
         cJSON_AddItemToArray(array,arrayobj);
                     

       }
        
        cJSON_AddItemToObject(root,"datamsg",array);
        
       char * sendbuff = cJSON_Print(root); 
       lwsctx->sendnlen = strlen(sendbuff);
       memcpy(lwsctx->sendbuffer,sendbuff,lwsctx->sendnlen);
       send2rsu(lwsctx);
       
}

int mec2rsu_rsirts(RsiDate_t *rsi_msg,lws_mes_t *lwsctx) {
       cJSON *root;
       cJSON *array;
       cJSON *repos;
       root = cJSON_CreateObject();
       array = cJSON_CreateArray();
       repos = cJSON_CreateObject();
       cJSON_AddNumberToObject(root,"frametype",0xc2);
       cJSON_AddNumberToObject(root,"rsctl",rsctl_rsi++);
       cJSON_AddNumberToObject(root,"rsuid",52);
       cJSON_AddStringToObject(root,"rsuesn","529c9c7d-a3b9-4560-8288-c80aa91798ac");//设备序列号
       cJSON_AddNumberToObject(repos,"lon",113.300348777);
       cJSON_AddNumberToObject(repos,"lat",23.566940348);
       cJSON_AddNumberToObject(repos,"ele",0);
       cJSON_AddItemToObject(root,"refpos",repos);

        for(int i = 0;i < rsi_msg->rts_Msgcount;i++) {
            cJSON *arrayobj = cJSON_CreateObject();
            cJSON *signpos = cJSON_CreateObject();

            cJSON_AddNumberToObject(arrayobj,"rtsId",rteid++);
            
            cJSON_AddNumberToObject(arrayobj,"signtype",rsi_msg->rtsarray[i].signtype);
            //rsi_msg->rtsarray[i].signpos.lon
            cJSON_AddNumberToObject(signpos,"lon",rsi_msg->rtsarray[i].signpos.lon);
            cJSON_AddNumberToObject(signpos,"lat",rsi_msg->rtsarray[i].signpos.lat);
            cJSON_AddNumberToObject(signpos,"ele",rsi_msg->rtsarray[i].signpos.ele);
            cJSON_AddItemToObject(arrayobj,"signpos",signpos);
            cJSON_AddNumberToObject(arrayobj,"type",102);
            cJSON_AddNumberToObject(arrayobj,"duration",5);
            cJSON_AddNumberToObject(arrayobj,"interval",100);
            
            cJSON_AddStringToObject(arrayobj,"description",rsi_msg->rtsarray[i].description.c_str());
            cJSON_AddNumberToObject(arrayobj,"priority",1); 

             cJSON_AddItemToArray(array,arrayobj);

        }
         cJSON_AddItemToObject(root,"datamsg",array);
       char * sendbuff = cJSON_Print(root); 
       lwsctx->sendnlen = strlen(sendbuff);
       memcpy(lwsctx->sendbuffer,sendbuff,lwsctx->sendnlen);
       send2rsu(lwsctx);




}

int mec2rsu_rsm(rsi_rsm_t *rsm,lws_mes_t *lwsctx) {

       cJSON *root;
       cJSON *array;
       
       root = cJSON_CreateObject();
       array = cJSON_CreateArray();
      
       cJSON_AddNumberToObject(root,"frametype",197);
       cJSON_AddNumberToObject(root,"rsctl",rsctl_rsm++);
       cJSON_AddNumberToObject(root,"rsuid",52);
       cJSON_AddStringToObject(root,"rsuesn","529c9c7d-a3b9-4560-8288-c80aa91798ac");//设备序列号
       
       for(int i = 0;i < rsm->datsnum;i++) {
           cJSON *repos;
           cJSON *arrayobj, *vehicle_size,*vehicleclass;
           repos = cJSON_CreateObject();
           arrayobj =cJSON_CreateObject();
           vehicle_size = cJSON_CreateObject();
           vehicleclass = cJSON_CreateObject();
           //113.300348777
          //23.566940348
           cJSON_AddNumberToObject(repos,"lon", rsm->parter[i].pos.lon);
           cJSON_AddNumberToObject(repos,"lat",rsm->parter[i].pos.lat);
           cJSON_AddNumberToObject(repos,"ele",rsm->parter[i].pos.ele);
           cJSON_AddItemToObject(arrayobj,"refpos",repos);
           cJSON_AddNumberToObject(arrayobj,"ptctype",1);
           cJSON_AddNumberToObject(arrayobj,"ptcid",37652);
           cJSON_AddNumberToObject(arrayobj,"secmark",0);
           cJSON_AddNumberToObject(arrayobj,"source",7);
           
           cJSON_AddNumberToObject(arrayobj,"speed",rsm->parter[i].speed);
           cJSON_AddNumberToObject(arrayobj,"heading",rsm->parter[i].heading);
           cJSON_AddNumberToObject(arrayobj,"angle",rsm->parter[i].angle);
           cJSON_AddNullToObject(arrayobj,"motioncfd");
           cJSON_AddNullToObject(arrayobj,"accelset");
            //rsm->parter[i].width
           cJSON_AddNumberToObject(vehicle_size,"width", rsm->parter[i].width);
           cJSON_AddNumberToObject(vehicle_size,"length", rsm->parter[i].length);
           cJSON_AddNumberToObject(vehicle_size,"height", rsm->parter[i].height);
           cJSON_AddItemToObject(arrayobj,"size",vehicle_size);

           cJSON_AddNumberToObject(vehicleclass,"fueltype", 0);
           cJSON_AddNumberToObject(vehicleclass,"vehitype", 0);
            cJSON_AddItemToObject(arrayobj,"vehicleclass",vehicleclass);
            cJSON_AddStringToObject(arrayobj,"id",(const char *)rsm->parter[i].id);

            cJSON_AddItemToArray(array,arrayobj);

       }
       cJSON_AddItemToObject(root,"datas",array);
       char * sendbuff = cJSON_Print(root); 
       lwsctx->sendnlen = strlen(sendbuff);
       memcpy(lwsctx->sendbuffer,sendbuff,lwsctx->sendnlen);
       send2rsu(lwsctx);
}

int parse_rsu2mec_bsm(cJSON * root,Bsmdata_t **bsm) {
    Bsmdata_t *bsmdata = NULL;
    //memset(bsmdata,0,sizeof(Bsmdata_t));
    int ret = 0;
    int arraysize;
    cJSON * array = NULL;
    cJSON * child1 = NULL;
    cJSON * child2 = NULL;
    cJSON * child3 = NULL;
    array =  cJSON_GetObjectItem(root,"datas");
    if(array == NULL) {
        ret = -1;
        goto END;
    }
    arraysize = cJSON_GetArraySize(array); 
    bsmdata = (Bsmdata_t *)malloc(sizeof(Bsmdata_t) * arraysize);
    for(int i = 0; i < arraysize;i++) {
        child1 = cJSON_GetArrayItem(array,i);
        if(child1 == NULL) {
            ret = -1;
            goto END;
        }
        child2 = cJSON_GetObjectItem(child1,"obuid");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].obuid =  child2->valueint;
        child2 = cJSON_GetObjectItem(child1,"vehicleid");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        memset(bsmdata[i].vehicleid,0,24);
        memcpy(bsmdata[i].vehicleid,child2->valuestring,strlen(child2->valuestring));
        child2 = cJSON_GetObjectItem(child1,"secmark");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].secmark = child2->valueint;
        child2 = cJSON_GetObjectItem(child1,"refpos");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        child3 = cJSON_GetObjectItem(child2,"lat");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].lat = child3->valueint;
        child3 = cJSON_GetObjectItem(child2,"lon");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].lon = child3->valueint;

        child3 = cJSON_GetObjectItem(child2,"ele");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].ele = child3->valueint;

        child2 = cJSON_GetObjectItem(child1,"transmission");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].transmission = child2->valueint;

        child2 = cJSON_GetObjectItem(child1,"speed");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].speed = child2->valueint;

         child2 = cJSON_GetObjectItem(child1,"heading");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].heading = child2->valueint;

         child2 = cJSON_GetObjectItem(child1,"size");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
         child3 = cJSON_GetObjectItem(child2,"width");
         if(child3 == NULL) {
           ret = -1;
           goto END;
         }

         bsmdata[i].width = child3->valueint;

          child3 = cJSON_GetObjectItem(child2,"length");
         if(child3 == NULL) {
           ret = -1;
           goto END;
         }
         bsmdata[i].length = child3->valueint;

        child2 = cJSON_GetObjectItem(child1,"accelset");
        if(child2 == NULL) {
           ret = -1;
           goto END;
        }
        child3 = cJSON_GetObjectItem(child2,"long");
        if(child3 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].accelset_lon = child3->valueint;

        child3 = cJSON_GetObjectItem(child2,"lat");
        if(child3 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].accelset_lat = child3->valueint;
        child3 = cJSON_GetObjectItem(child2,"vert");
        if(child3 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].accelset_vert = child3->valueint;

          child3 = cJSON_GetObjectItem(child2,"yaw");
        if(child3 == NULL) {
           ret = -1;
           goto END;
        }
        bsmdata[i].accelset_yaw = child3->valueint;
    }
    
    *bsm = bsmdata;
    
    
    
    
    return 0;
    END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

}

int main() {

    lws_init(lws_callback_recv);
    lws_mes_t *lws_ctx = get_lwsctx();
    RsiDate_t rte;
    RsiDate_t rts;
    rsi_rsm_t rsm;

    rte.rte_Msgcount = 1;
    rte.rtearray[0].eventtype = 904;
    rte.rtearray[0].lat = 11;
    rte.rtearray[0].lon - 11;
    rte.rtearray[0].ele = 0;
    rte.rtearray[0].description = string("111");


    rts.rts_Msgcount = 1;
    rts.rtsarray[0].signpos.ele = 0;
    rts.rtsarray[0].signpos.lat = 11;
    rts.rtsarray[0].signpos.lon = 11;
    rts.rtsarray[0].signtype = 904;
    rts.rtsarray[0].description = string("hello");

    rsm.datsnum = 1;
    rsm.parter[0].pos.lon = 123;
    rsm.parter[0].pos.lat = 456;
    rsm.parter[0].pos.ele = 0;
    rsm.parter[0].width = 1;
    rsm.parter[0].length = 2;
    rsm.parter[0].height = 1;
    rsm.parter[0].angle = 0;
    rsm.parter[0].speed = 0;
    rsm.parter[0].heading = 0;
    memcpy(rsm.parter[0].id,"HN2228",6);



    while(1){
        sleep(1);
        //response_register(lws_ctx);
        // mec2rsu_rsirte(&rte,lws_ctx);
        //mec2rsu_rsirts(&rts,lws_ctx);
        //mec2rsu_rsm(&rsm,lws_ctx);

        


        

    }
}