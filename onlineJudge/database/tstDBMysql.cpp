#include <iostream>
#include <string>
#include <memory>
#include "./dataBaseMysql.hpp"

void test()
{
    std::unique_ptr<dataBaseMysql::CDatabaseMysql> pConn;
    pConn.reset(new dataBaseMysql::CDatabaseMysql());
    std::string strHost = "127.0.0.1";
    std::string strUser = "root";
    std::string strPwd  = "Root@123" ;
    std::string strDBName = "test";
    if(!pConn->initialize(strHost, strUser, strPwd, strDBName))
    {
        LOG(util::ERROR) << "initialize db failed, please check params." << std::endl; 
        return;
    }
    std::string sql = "select * from compile;";
    dataBaseMysql::QueryResult* pResult = pConn->query(sql);
    if(!pResult)
    {
        LOG(util::ERROR) << "query() error" << std::endl;
        return;
    }

    dataBaseMysql::Field* pRow = pResult->fetch();
    std::string types;
    while(true)
    {
        if(pRow == nullptr)
            break;
        types = pRow[1].getCppString();
        std::cout << "types = " << types << std::endl;
        if(!pResult->nextRow())
        {
            break;
        }
    }

    sql = "create database test1;";
    pResult = pConn->query(sql);

    if(!pResult)
        std::cout << "pResult is nullptr" << std::endl;
    else
    {
        std::cout << "pResult is not nullptr" << std::endl;
        pResult->endQuery();
    }
}

int main(void)
{
    test();

    return 0;
}
