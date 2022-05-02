#ifndef  _DBPOOL_H
#define  _DBPOOL_H
#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <map>
#include <stdint.h>

#include <mysql.h>
using namespace std;
typedef  unsigned short uint16_t;
#define MAX_ESCAPE_STRING_LEN 10240

class CResultSet {
    public:
        CResultSet(MYSQL_RES* res);
        virtual ~CResultSet();
    private:
        MYSQL_RES* 			m_res;
        MYSQL_ROW			m_row;
        map<string, int>	m_key_map;

};
class CDBPool;
class CDBConn {
public:
    CDBConn(CDBPool* pDBPool);
    int init();
    uint32_t GetInsertId()
    {
	    return (uint32_t)mysql_insert_id(m_mysql);
    }
    bool StartTransaction();
    bool Rollback();
    bool Commit();
    bool ExecuteCreate(const char *sql_query);
    bool ExecuteDrop(const char *sql_query); 
    CResultSet *ExecuteQuery(const char *sql_query);
private:
	CDBPool* 	m_pDBPool;	// to get MySQL server information
	MYSQL* 		m_mysql;	// 对应一个连接
	char		m_escape_string[MAX_ESCAPE_STRING_LEN + 1];

};



class CDBPool
{
private:
    /* data */
public:
    CDBPool(/* args */);
    CDBPool(const char *pool_name,const char *db_server_ip,uint16_t db_server_port,
    const char *username,const char *passwd,const char *db_name,
    int max_con_cnt);
    virtual ~CDBPool();

    int init();   //连接数据库，创建连接

    
    CDBConn* 	GetDBConn(const int timeout_ms = 0);	// 获取连接资源
    void 		RelDBConn(CDBConn* pConn);	// 归还连接资源
    const char* GetPoolName() { return m_pool_name.c_str(); }
	const char* GetDBServerIP() { return m_db_server_ip.c_str(); }
	uint16_t 	GetDBServerPort() { return m_db_server_port; }
	const char* GetUsername() { return m_username.c_str(); }
	const char* GetPasswrod() { return m_password.c_str(); }
	const char* GetDBName() { return m_db_name.c_str(); }


private:
	string 		m_pool_name;	// 连接池名称
	string 		m_db_server_ip;	// 数据库ip
	uint16_t	m_db_server_port; // 数据库端口
	string 		m_username;  	// 用户名
	string 		m_password;		// 用户密码
	string 		m_db_name;		// db名称
	int			m_db_cur_conn_cnt;	// 当前启用的连接数量
	int 		m_db_max_conn_cnt;	// 最大连接数量
	list<CDBConn*>	m_free_list;	// 空闲的连接

	list<CDBConn*>	m_used_list;		// 记录已经被请求的连接
	std::mutex m_mutex;
    std::condition_variable m_cond_var;
	bool m_abort_request = false;
	// CThreadNotify	m_free_notify;	// 信号量
};



#endif // ! _DBPOOL
