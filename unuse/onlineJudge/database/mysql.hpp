#pragma once

#include <iostream>
#include <mysql/mysql.h>
#include "util.hpp"

namespace mysql
{
    class mysql
    {
    public:
        /*
         * MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user,                                      
         *                           const char *passwd, const char *db, unsigned int port, 
         *                          const char *unix_socket, unsigned long client_flag)
         * */
        mysql(MYSQL* pMysqlInit= NULL, const std::string& host = "localhost", 
              const std::string& user = "root", const std::string& passwd = "Root@123", 
              const std::string& db = "onlineJudge", unsigned int port = 3306, 
              const std::string& unix_socket = "", unsigned long client_flag = 0)
            :_pMysqlInit(nullptr)
             ,_pRealMysql(nullptr)
        {
            int ret = mysql_library_init(0, NULL, NULL);
            if(ret != 0)
            {
                LOG(util::ERROR) << "mysql_library_init error" << std::endl;
                return;
            }
            _pMysqlInit = mysql_init(pMysqlInit);
            if(!_pMysqlInit)
            {
                LOG(util::ERROR) << "mysql_init error" << std::endl;
                return;
            }
            _pRealMysql = mysql_real_connect(_pMysqlInit, host.c_str(), user.c_str(), passwd.c_str(),
                                             db.c_str(), port, unix_socket.c_str(), client_flag);
            if(!_pRealMysql)
            {
                LOG(util::ERROR) << "mysql_real_connect error" << std::endl;
                return;
            }
        }

        bool sqlQuery(const std::string& queryString)
        {
            int ret = mysql_query(_pRealMysql, "set names utf8");
            if(!ret)
            {
                LOG(util::ERROR) << "mysql_query error" << std::endl;
                return false;
            }
            ret = mysql_query(_pRealMysql, queryString.c_str());
            if(!ret)
            {
                LOG(util::ERROR) << "mysql_query error" << std::endl;
                return false;
            }
            return true;
        }

        ~mysql()
        {
            if(_pRealMysql)
            {
                mysql_close(_pRealMysql);
            }
            mysql_library_end();
        }
    private:
        MYSQL* _pMysqlInit;
        MYSQL* _pRealMysql;
    };
}
