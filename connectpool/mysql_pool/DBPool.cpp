#include "DBPool.h"
#define MIN_DB_CONN_CNT 1
CDBPool::CDBPool(/* args */)
{
}

CDBPool::~CDBPool()
{
}


CResultSet::CResultSet(MYSQL_RES *res) {
   m_res =  res;
   int num_fields = mysql_num_fields(m_res);
   MYSQL_FIELD *fields = mysql_fetch_fields(m_res);
   for(int i= 0 ;i< num_fields;i++) {
       m_key_map.insert(make_pair(fields[i].name,i));
   }

    
}
CResultSet::~CResultSet()
{
	if (m_res)
	{
		mysql_free_result(m_res);
		m_res = NULL;
	}
}
CDBConn::CDBConn(CDBPool *pDBPool) {
    m_pDBPool = pDBPool;
    m_mysql = NULL;


}
int CDBConn::init() {
   m_mysql = mysql_init(NULL);

   if(!m_mysql) {
       perror("mysql_init failed");
       return 1;
   } 
   bool reconnect = true;
   mysql_options(m_mysql,MYSQL_OPT_RECONNECT,&reconnect);
   mysql_options(m_mysql,MYSQL_SET_CHARSET_NAME,"utf8mb4");

   if(!mysql_real_connect(m_mysql,m_pDBPool->GetDBServerIP(),m_pDBPool->GetUsername(),
   m_pDBPool->GetPasswrod(),m_pDBPool->GetDBName(),m_pDBPool->GetDBServerPort(),NULL,0))
   {
       //perror("mysql_real_connect failed:%s\n",mysql_error(m_mysql));
       printf("mysql_real_connect failed: %s\n", mysql_error(m_mysql));
       return 2;
    }
    return 0;
}
bool CDBConn::ExecuteCreate(const char *sql_query) {
    mysql_ping(m_mysql);
    if(mysql_real_query(m_mysql,sql_query,strlen(sql_query))) {
        printf("mysql_real_query failed:%s,qil start transation\n",mysql_error(m_mysql));
        return false;
    }
    return true;
}

bool CDBConn::ExecuteDrop(const char *sql_query)
{
	mysql_ping(m_mysql);	// 如果端开了，能够自动重连

	if (mysql_real_query(m_mysql, sql_query, strlen(sql_query)))
	{
		printf("mysql_real_query failed: %s, sql: start transaction\n", mysql_error(m_mysql));
		return false;
	}

	return true;
}
CResultSet *CDBConn::ExecuteQuery(const char *sql_query) {
    mysql_ping(m_mysql);
    if(mysql_real_query(m_mysql,sql_query,strlen(sql_query))){
        printf("mysql_real_query failed: %s, sql: %s\n", mysql_error(m_mysql), sql_query);
        return NULL;
    }
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res) {
       printf("mysql_store_result failed: %s\n", mysql_error(m_mysql));
       return NULL;
    }

    CResultSet *result_set = new CResultSet(res);	// 存储到CResultSet
	return result_set;

}
bool CDBConn::ExecuteUpdate(const char *sql_query, bool care_affected_rows)
{
	mysql_ping(m_mysql);

	if (mysql_real_query(m_mysql, sql_query, strlen(sql_query)))
	{
		printf("mysql_real_query failed: %s, sql: %s\n", mysql_error(m_mysql), sql_query);
		//g_master_conn_fail_num ++;
		return false;
	}

	if (mysql_affected_rows(m_mysql) > 0)
	{
		return true;
	}
	else
	{ // 影响的行数为0时
		if (care_affected_rows)
		{ // 如果在意影响的行数时, 返回false, 否则返回true
			printf("mysql_real_query failed: %s, sql: %s\n\n", mysql_error(m_mysql), sql_query);
			return false;
		}
		else
		{
			printf("affected_rows=0, sql: %s\n\n", sql_query);
			return true;
		}
	}
}

bool CDBConn::StartTransaction()
{
	mysql_ping(m_mysql);

	if (mysql_real_query(m_mysql, "start transaction\n", 17))
	{
		printf("mysql_real_query failed: %s, sql: start transaction\n", mysql_error(m_mysql));
		return false;
	}

	return true;
}
bool CDBConn::Commit()
{
	mysql_ping(m_mysql);

	if (mysql_real_query(m_mysql, "commit\n", 6))
	{
		log_error("mysql_real_query failed: %s, sql: commit\n", mysql_error(m_mysql));
		return false;
	}

	return true;
}

bool CDBConn::Rollback()
{
	mysql_ping(m_mysql);

	if (mysql_real_query(m_mysql, "rollback\n", 8))
	{
		printf("mysql_real_query failed: %s, sql: rollback\n", mysql_error(m_mysql));
		return false;
	}

	return true;
}


CDBPool::CDBPool(const char *pool_name, const char *db_server_ip, uint16_t db_server_port,
				 const char *username, const char *password, const char *db_name, int max_conn_cnt)
{
	m_pool_name = pool_name;
	m_db_server_ip = db_server_ip;
	m_db_server_port = db_server_port;
	m_username = username;
	m_password = password;
	m_db_name = db_name;
	m_db_max_conn_cnt = max_conn_cnt;	// 
	m_db_cur_conn_cnt = MIN_DB_CONN_CNT; // 最小连接数量
}
CDBPool::~CDBPool() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_abort_request = true;
    m_cond_var.notify_all();
    for(list<CDBConn *>::iterator it = m_free_list.begin();it != m_free_list.end();it++) {
        CDBConn *pConn = *it;
        delete pConn;
    }
    m_free_list.clear();
};
CDBConn *CDBPool::GetDBConn(const int timeout_ms) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_abort_request) {
        perror("hace abort\n");
        return NULL;
    }

    if(m_free_list.empty()) {
        if(m_db_cur_conn_cnt >= m_db_max_conn_cnt) {
            if(timeout_ms <= 0) {

                printf("wiats ms:%d\n",timeout_ms);
                m_cond_var.wait(lock,[this]
                {
                    return (!m_free_list.empty()) | m_abort_request;

                }
                );
            }
            else {
                m_cond_var.wait_for(lock,std::chrono::milliseconds(timeout_ms),[this]{

                     return (!m_free_list.empty()) | m_abort_request;

                });
                if(m_free_list.empty()) {
                    return NULL;
                }
            }
            if(m_abort_request){

                printf("have abort\n");
                return NULL;
            }

        }
        else {
            CDBConn *pDBConn = new CDBConn(this);	//新建连接
			int ret = pDBConn->init();
			if (ret)
			{
				printf("Init DBConnecton failed\n\n");
				delete pDBConn;
				return NULL;
			}
            else {
                m_free_list.push_back(pDBConn);
                m_db_cur_conn_cnt++;
            }

        }
    }
    CDBConn *PConn = m_free_list.front();//获取连接
    m_free_list.pop_front();
    m_used_list.push_back(PConn);
    return PConn;


}
void   CDBPool::RelDBConn(CDBConn* pConn) {
    std::lock_guard<std::mutex> lock(m_mutex);

    list<CDBConn *>::iterator it = m_free_list.begin();
    for(;it != m_free_list.end();it++) {

        if(*it = pConn) {
            break;
        }

    }
    if(it == m_free_list.end()) {
        m_used_list.remove(pConn);
        m_free_list.push_back(pConn);
        m_cond_var.notify_one();

    }else{

        perror("relDBConn failed\n");
    }



}
int CDBPool::init() {
    for(int i = 0; i < m_db_cur_conn_cnt;i++) {

        CDBConn *pDBConn = new CDBConn(this);
        int ret = pDBConn->init();
        if(ret)
        {
            delete pDBConn;
            return ret;
        }

        m_free_list.push_back(pDBConn);
    }
    return 0;


}

