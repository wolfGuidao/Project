/*
 * 封装mysql的C API类
 * */
#pragma once 
#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include <cassert>
#include <algorithm>
#include <cstdarg> /* 解析可变参数列表 */
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include "../serverSrc/util.hpp"

/* 命名空间 */
namespace dataBaseMysql 
{
#define MAX_QUERY_LEN 1024

    /* 将字符串中的大写字母全部转化为小写字母 */
    inline void toLowerString(std::string& str)
    {
        for(size_t i = 0; i < str.size(); ++i)
        {
            if(str[i] >= 'A' && str[i] <= 'Z')
            {
                str[i] = str[i] + ('a' - 'A');
            }
        }
    }

    /* Field类 
     * 主要是对一行查询结果的描述
     * */
    class Field
    {
    public: 
        /*
         * 枚举类型的DataType
         * 使处理的数据在以下范围内
         * */
        enum DataTypes
        {
            DB_TYPE_UNKNOWN = 0x00,
            DB_TYPE_STRING  = 0x01,
            DB_TYPE_INTEGER = 0x02,
            DB_TYPE_FLOAT   = 0x03,
            DB_TYPE_BOOL    = 0x04
        };

        /* Field类的构造函数
         * 对存储类型初始化为未知类型
         * */
        Field()
            : mType(DB_TYPE_UNKNOWN)
        {
            m_bNULL = false;
        }

        /*
         * Field类的拷贝构造函数
         * */
        Field(Field& f)
        {
            m_strValue = f.m_strValue;
        }

        /*
         * 重载的构造函数 指定类型的value
         * */
        Field(const char* value, enum DataTypes type)
            : mType(type)
        {
            m_strValue = value;
        }

        /* Field类的析构函数
         * 没有管理资源，即使实现也是可以的
         * */
        ~Field()
        {}

        /* 获取当前对象存储元素的类型 */
        enum DataTypes getType() const
        {
            return mType;
        }

        /* 我们把数据都按照字符串存储
         * getString 方法返回 对应的数据
         * */
        const std::string getString() const
        {
            return m_strValue;
        }

        /* 
         * 返回C++string类型的字符串
         * 返回的值可以修改
         * */
        std::string getCppString() const
        {
            return m_strValue;
        }

        /*
         * 将存储数据转化为float类型
         * */
        float getFloat() const
        {
            return static_cast<float>(atof(m_strValue.c_str()));
        }

        /* 
         * 将数据转化为bool类型
         * */
        bool getBool() const
        {
            return atoi(m_strValue.c_str()) > 0;
        }

        /*
         * 转化为int32_t 类型
         * */
        int32_t getInt32() const
        {
            return static_cast<int32_t>(atol(m_strValue.c_str()));
        }

        /*
         * 转化为 uint8_t 类型
         * */
        uint8_t getUInt8() const
        {
            return static_cast<uint8_t>(atol(m_strValue.c_str()));
        }

        /* 
         * 转化为 uint16_t 类型
         * */
        uint16_t getUInt16() const
        {
            return static_cast<uint16_t>(atol(m_strValue.c_str()));
        }

        /* 
         * 转化为 int16_t 类型
         * */
        int16_t getInt16() const
        {
            return static_cast<int16_t>(atol(m_strValue.c_str()));
        }

        /*
         * 转化为 uint32_t 类型
         * */
        uint32_t getUInt32() const
        {
            return static_cast<uint32_t>(atol(m_strValue.c_str()));
        }

        /* 
         * 转化为 uint64_t 类型
         * */
        uint64_t getUInt64() const
        {
            uint64_t value = 0;
            value = atoll(m_strValue.c_str());
            return value;
        }

        /*
         * 设置当前对象的类型
         * */
        void setType(enum DataTypes type)
        {
            mType = type;
        }

        /*
         * 设置当前对象存储的数据
         * */
        void setValue(const char* value, size_t len)
        {
            m_strValue.assign(value, len);
        }

        /*
         * 设置当前存储数据的名称
         * */
        void setName(const std::string& strName)
        {
            m_strFieldName = strName;
            toLowerString(m_strFieldName);
        }

        /*
         * 获取名称
         * */
        const std::string& getName()
        {
            return m_strFieldName;
        }

        /*
         * 判断当前对象是否存储有数据
         * */
        bool isNULL()
        {
            return m_bNULL;
        }

        template <typename T>
            void conertValue(T& value);
    private:
        /* 用来存储数据 */
        std::string m_strValue; 
        /* 用来存储数据名称，可以理解为表头的名称 */
        std::string m_strFieldName;
        /* 存储元素数据类型 */
        enum DataTypes mType;
    public:
        /* 标记当前是否存储数据 */
        bool m_bNULL;

    };

    /* QueryResult 类主要是保存查询结果 
     * 将结果保存在map里
     * */

    class QueryResult
    {
    public:
        /* 对 std::map<uint32_t, std::string> 改个简单方便好用的名字
         * */
        typedef std::map<uint32_t, std::string> fieldNames;
    public:
        /* 
         * 构造函数 
         * result为SQL查询结果
         * rowCount 为该SQL语句影响的行数
         * fieldCount 列数
         * */
        QueryResult(MYSQL_RES* result, uint64_t rowCount, uint32_t fieldCount)
            : mFieldCount(fieldCount)
            , mRowCount(rowCount)
        {
            mResult = result;
            mCurrentRow = new Field[mFieldCount];
            assert(mCurrentRow);

            /* 
             * 通过 MySQL C API mysql_fetch_fields() 函数返回所有字段类型的数组
             * */
            /* 
             * 关于 MYSQL_FIELD这个结构体可以 grep -R 'MYSQL_FIELD' /usr/local/ 
             * 我们打开文件 /usr/local/mysql-8.0.12-linux-glibc2.12-x86_64/include/mysql.h
             * 然后/MYSQL_FIELD 就可以找到
             * */
            MYSQL_FIELD* fields = mysql_fetch_fields(mResult);
            
            for(uint32_t i = 0; i < mFieldCount; ++i)
            {
                /* 这里的name是列名(name of column)
                 * */
                if(fields[i].name != nullptr)
                {
                    mFieldNames[i] = fields[i].name;
                    m_vtFieldNames.push_back(fields[i].name);
                }
                else
                {
                    /* 表的某一个字段是可能为NULL的
                     * 我们把这个字段设置为空串 ""
                     * */
                    mFieldNames[i] = "";
                    m_vtFieldNames.push_back("");
                }

                /* 设置类型
                 * */
                mCurrentRow[i].setType(convertNativeType(fields[i].type));
            }
        }
        
        /*
         * 析构函数 设置为虚函数
         * */
        virtual ~QueryResult()
        {
            /* 
             * 关于endQuery函数会在下面介绍
             * */
            endQuery();
        }

        /* 
         * nextRow函数： 获取查询结果的下一行
         * */
        virtual bool nextRow()
        {
            /*
             * MYSQL_ROW 是对 char ** 的typedef
             * */
            MYSQL_ROW row;

            if(!mResult)
                return false;

            /*
             * mysql_fetch_row函数：获取结果集的下一行
             * */
            row = mysql_fetch_row(mResult);
            if(!row)
            {
                endQuery();
                return false;
            }

            unsigned long int * ulFieldLen = nullptr;
            /*
             * mysql_fetch_lengths函数：返回当前行中所有列的长度(就是数组元素个数)
             * */
            ulFieldLen = mysql_fetch_lengths(mResult);
            for(uint32_t i = 0; i < mFieldCount; ++i)
            {
                if(row[i] == nullptr)
                {
                    mCurrentRow[i].m_bNULL = true;
                    mCurrentRow[i].setValue("", 0);
                }
                else
                {
                    mCurrentRow[i].m_bNULL = false;
                    mCurrentRow[i].setValue(row[i], ulFieldLen[i]);
                }

                mCurrentRow[i].setName(mFieldNames[i]);
            }

            return true;
        }

        /*
         * 该函数在map里查找第二个元素为name对应的第一个元素，也就是索引index
         * */
        uint32_t getFieldIndex(const std::string& name) const
        {
            for(fieldNames::const_iterator it = getFieldNames().begin(); it != getFieldNames().end(); ++it)
            {
                if(it->second == name)
                    return it->first;
            }

            assert(false && "unknow field name");
            return uint32_t(-1);
        }

        /*
         * 返回当前行
         * */
        Field* fetch() const
        {
            return mCurrentRow;
        }

        /*
         * 重载[]运算符
         * 返回对应索引index的行
         * */
        const Field& operator[](int index) const
        {
            return mCurrentRow[index];
        }

        /*
         * 重载[]运算符
         * 先根据name调用getFieldIndex函数获取到对应的索引index
         * 然后调用上一个operator[]得到对应的查询结果(这里说的查询结果是一个Field结构)
         * */
        const Field& operator[](const std::string& name) const
        {
            return mCurrentRow[getFieldIndex(name)];
        }

        /* 
         * 获取当前有多少个Field
         * 由于我们是私有数据成员 mFieldCount 记录当前有多少个Field
         * */
        uint32_t getFieldCount() const
        {
            return mFieldCount;
        }

        /*
         * 获取行数
         * */
        uint32_t getRowCount() const
        {
            return mRowCount;
        }

        /*
         * 获取到map
         * */
        fieldNames const& getFieldNames() const
        {
            return mFieldNames;
        }

        /*
         * 获取到保存列名的vector
         * */
        std::vector<std::string> const& getNames() const
        {
            return m_vtFieldNames;
        }
    private:
        /*
         * convertNativeType函数返回合适的类型
         *
         * */
        enum Field::DataTypes convertNativeType(enum_field_types mysqlType) const
        {
            /*
             * 将MySQL详细的数据数据类型分成几个大类
             * 例如：MySQL是用字符串描述时间戳，日期，时间，我们都归为 Field::DB_TYPE_STRING
             * MySQL的tinyint、short、long、int24 都归为 Field::DB_TYPE_INTEGER
             * MySQL 的 float double 都归为 Field::DB_TYPE_FLOAT
             * 其他都归为未知类型：Field::DB_TYPE_UNKNOWN
             * */
            switch(mysqlType)
            {
            case FIELD_TYPE_TIMESTAMP:
            case FIELD_TYPE_DATE:
            case FIELD_TYPE_TIME:
            case FIELD_TYPE_DATETIME:
            case FIELD_TYPE_YEAR:
            case FIELD_TYPE_STRING:
            case FIELD_TYPE_VAR_STRING:
            case FIELD_TYPE_BLOB:
            case FIELD_TYPE_SET:
            case FIELD_TYPE_NULL:
                return Field::DB_TYPE_STRING;
            case FIELD_TYPE_TINY:
            case FIELD_TYPE_SHORT:
            case FIELD_TYPE_LONG:
            case FIELD_TYPE_INT24:
            case FIELD_TYPE_LONGLONG:
            case FIELD_TYPE_ENUM:
                return Field::DB_TYPE_INTEGER;
            case FIELD_TYPE_DECIMAL:
            case FIELD_TYPE_FLOAT:
            case FIELD_TYPE_DOUBLE:
                return Field::DB_TYPE_FLOAT;
            default:
                return Field::DB_TYPE_UNKNOWN;
            }
        }
    public:
        /*
         * endQuery函数
         * 对 QueryResult类做清理工作
         * 释放动态申请的 mCurrentRow
         * 调用 mysql_free_result函数 销毁 MYSQL_RES结构
         * */
        void endQuery()
        {
            if(mCurrentRow)
            {
                delete[] mCurrentRow;
                mCurrentRow = nullptr;
            }

            if(mResult)
            {
                mysql_free_result(mResult);
                mResult = nullptr;
            }
        }
    protected:
        Field*                   mCurrentRow;   /* 类似于动态顺序表的数据块指针 */
        uint32_t                 mFieldCount;   /* 相当于记录列数 */
        uint64_t                 mRowCount;     /* 行数 */
        fieldNames               mFieldNames;   /* 存储数据的map */
        std::vector<std::string> m_vtFieldNames;/* 记录列名 */

        MYSQL_RES*               mResult;       /* 指针：指向mysql_real_query() 返回结果 */
    };

    /*
     * CDatabaseMysql 类主要封装查询功能
     * 连接数据库
     * 查询
     * 关闭连接
     * */
    class CDatabaseMysql
    {
    public:
        /* 
         * 某一个数据库的信息
         * */
        struct DatabaseInfo
        {
            std::string _strHost; /* 主机 */
            std::string _strUser; /* 用户 */
            std::string _strPwd;  /* 密码 */
            std::string _strDBName; /* 数据库名称 */
        };
    public:
        /* 
         * 构造函数
         * 构造函数中并不做连接数据库的操作
         * */
        CDatabaseMysql()
            : m_Mysql(nullptr)
            , m_bInit(false)
        {}
    
        /*
         * 析构函数
         * 关闭连接数据库的句柄
         * */
        ~CDatabaseMysql()
        {
            if(m_Mysql != nullptr)
            {
                if(m_bInit)
                {
                    mysql_close(m_Mysql);
                }
            }
        }
    
    
        /*
         * 在init函数里连接数据库
         * */
        bool initialize(const std::string& host,
                         const std::string& user,
                         const std::string& pwd,
                         const std::string& dbname)
        {
            /*
             * 如果数据库已经连接了，就将其关闭之
             * 并再次根据参数，重连一次
             * */
            // TODO: 是否需要判断一下，本次传参和上次连接的参数相同？
            if(m_bInit)
            {
                mysql_close(m_Mysql);
            }

            /* 
             * 调用mysql_init()函数，返回一个操作数据库的句柄
             * mysql_real_connect()函数：尝试在主机上建立到MySQL数据库引擎的连接
             * */
            m_Mysql = mysql_init(m_Mysql);
            m_Mysql = mysql_real_connect(m_Mysql, host.c_str(), user.c_str(), 
                                         pwd.c_str(), dbname.c_str(), 0, nullptr, 0);
            m_DBInfo._strHost = host;
            m_DBInfo._strUser = user;
            m_DBInfo._strPwd  = pwd;
            m_DBInfo._strDBName = dbname;

            if(m_Mysql)
            {
                /* 设置查询字符集为utf8 */
                mysql_query(m_Mysql, "set names utf8");
                m_bInit = true;
                return true;
            }
            else
            {
                /*
                 * 数据库连接失败，打印日志，返回false
                 * */
                LOG(util::ERROR) << "Could not connect to MYSQL database: " << mysql_error(m_Mysql) << std::endl;
                mysql_close(m_Mysql);
                return false;
            }
            LOG(util::ERROR) << "CDatabaseMysql::initialize, init false" << std::endl;
            return false;
        }

        /*
         * query() 函数负责做查询 他的参数是一个C风格的字符串----SQL语句
         * */
        QueryResult* query(const char* sql)
        {
            if(!m_Mysql)
            {
                /*
                 * 数据库未连接
                 * 调用initialize函数进行初始化
                 * */
                LOG(util::INFO) << "mysql is disconnected!" << std::endl; 
                if(false == initialize(m_DBInfo._strHost, m_DBInfo._strUser, 
                                       m_DBInfo._strPwd, m_DBInfo._strDBName))
                {
                    /*
                     * 数据库初始化失败，返回 nullptr
                     * */
                    return nullptr;
                }                   
            }

            /* 
             * 未知错误导致 m_Mysql 为 nullptr 也返回nullptr吧
             * */
            if(!m_Mysql)
                return nullptr;

            /*
             * 调用 mysql_real_query() 函数后，查询结果需要调用 mysql_store_result() 函数获取 
             * 而 mysql_store_result() 函数，返回一个指向查询结果的指针 ---- MYSQL_RES
             * */
            MYSQL_RES* result = nullptr;
            uint64_t rowCount = 0;
            uint32_t fieldCount = 0;
            {
                /*
                 * 调用 mysql_real_query() 函数对 SQL语句 进行查询
                 * 成功返回0，失败返回非0
                 * */
                LOG(util::INFO) << sql << std::endl;
                int iTempRet = mysql_real_query(m_Mysql, sql, strlen(sql));
                if(iTempRet)
                {
                    /*
                     * 查询失败，分析原因
                     * */
                    unsigned int uErrno = mysql_errno(m_Mysql);
                    LOG(util::ERROR) << "mysql is abnormal, errno : " << uErrno << std::endl;
                    /*
                     * CR_SERVER_GONE_ERROR 客户端无法向服务器发送请求
                     * 由于后台服务器通常与MySQL交互使用长连接的方式，如果会话timeout，MySQL会对当前连接进行关闭
                     * */
                    if(CR_SERVER_GONE_ERROR == uErrno)
                    {
                        /*
                         * 既然被关闭连接了，就调用initialize()函数再次连接
                         * */
                        LOG(util::ERROR) << "mysql is disconnected!" << std::endl; 
                        if(false == initialize(m_DBInfo._strHost, m_DBInfo._strUser,
                                               m_DBInfo._strPwd, m_DBInfo._strDBName))
                        {
                            return nullptr;
                        }
                        LOG(util::INFO) << sql << std::endl;
                        iTempRet = mysql_real_query(m_Mysql, sql, strlen(sql));
                        if(iTempRet)
                        {
                            LOG(util::ERROR) << "SQL:" << sql << std::endl;
                            LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                        }
                    }
                    else
                    {
                        /*
                         * 对于其他错误，暂不处理，直接返回nullptr
                         * */
                        LOG(util::ERROR) << "SQL: " << sql << std::endl;
                        LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                        return nullptr;
                    }
                }

                /*
                 * SQL语句执行完毕，调用mysql_store_result()函数获取查询结果
                 * mysql_affected_rows() 函数：返回最后一次update/delete/insert改变的行数
                 * mysql_field_count()函数：返回最近语句结果的列数
                 * */
                result = mysql_store_result(m_Mysql);
                rowCount = mysql_affected_rows(m_Mysql);
                fieldCount = mysql_field_count(m_Mysql);              
            }
            /*
             * 查询结果为空
             * */
            if(!result)
            {
                return nullptr;
            }

            /*
             * 构造一个QueryResult对象 new出来的
             * 作为本函数 query() 的返回值
             * */
            QueryResult* queryResult = new QueryResult(result, rowCount, fieldCount);
            queryResult->nextRow();
            return queryResult;
        }

        /*
         * 重载函数 query()
         * 直接调用 query(sql.c_str())
         * */
        QueryResult* query(const std::string& sql)
        {
            return query(sql.c_str());
        }

        QueryResult* pQuery(const char* format, ...)
        {
            if(!format)
            {
                return nullptr;
            }

            /*
             * 解析变长参数
             * */
            va_list ap;
            char szQuery[MAX_QUERY_LEN];
            va_start(ap, format);
            int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
            va_end(ap);

            if (res == -1)
            {
                LOG(util::ERROR) << "SQL Query truncated (and not execute) for format: " << format << std::endl;
                return nullptr;
            }

            return query(szQuery);
        }

        bool execute(const char* sql)
        {
            if(!m_Mysql)
                return false;

            {
                /*
                 * mysql_query() 函数：执行指定为以null(\0)结尾的字符串的SQL查询
                 * */
                int iTempRet = mysql_query(m_Mysql, sql);
                if(iTempRet)
                {
                    unsigned int uErrno = mysql_errno(m_Mysql);
                    LOG(util::ERROR) << "mysql is abnormal, errno : " << uErrno << std::endl;
                    if(CR_SERVER_GONE_ERROR == uErrno)
                    {
                        LOG(util::ERROR) << "mysql is disconnected! << std::endl";
                        if(false == initialize(m_DBInfo._strHost, m_DBInfo._strUser,
                                               m_DBInfo._strPwd, m_DBInfo._strDBName))
                        {
                            return false;
                        }
                        LOG(util::ERROR) << sql  << std::endl;
                        /*
                         * mysql_real_query() 函数：执行sql语句，sql是由一个SQL语句组成，且不带结束分号(;)或\g
                         * mysql_query()函数，不能用于查询包含二进制数据的语句(二进制数据可能包含\0字符)
                         * mysql_query()将\0解释为语句字符串的末尾
                         * mysql_real_query()函数比mysql_query()函数更快，因为不需要调用strlen()函数
                         * */
                        iTempRet = mysql_real_query(m_Mysql, sql, strlen(sql));
                        if(iTempRet)
                        {
                            LOG(util::ERROR) << "SQL: "<< sql << std::endl;
                            LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                        }
                    }
                    else
                    {
                        LOG(util::ERROR) << "SQL: " << sql << std::endl;
                        LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                    }

                    return false;
                }
            }
            return true;
        }

        /*
         * 重载的execute()函数：对sql语句进行查询，并且可以返回本次查询改变的函数和错误码
         * */
        bool execute(const char* sql, uint32_t& uAffectedCount, int& nErrno)
        {
            if(!m_Mysql)
                return false;

            {
                int iTempRet = mysql_query(m_Mysql, sql);
                if(iTempRet)
                {
                    unsigned int uErrno = mysql_errno(m_Mysql);
                    LOG(util::ERROR) << "mysql is abnormal, errno : " << uErrno << std::endl;
                    if(uErrno == CR_SERVER_GONE_ERROR)
                    {
                        LOG(util::ERROR) << "mysql is disconnection!" << std::endl;
                        if(false == initialize(m_DBInfo._strHost, m_DBInfo._strUser, 
                                               m_DBInfo._strPwd, m_DBInfo._strDBName))
                        {
                            return false;
                        }
                        LOG(util::ERROR) << sql << std::endl;
                        iTempRet = mysql_query(m_Mysql, sql);
                        nErrno = iTempRet;
                        if(iTempRet)
                        {
                            LOG(util::ERROR) << "SQL: " << sql << std::endl;
                            LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                        }
                    }
                    else
                    {
                        LOG(util::ERROR) << "SQL: " << sql << std::endl;
                        LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                    }
                    return false;
                }
                uAffectedCount = static_cast<uint32_t>(mysql_affected_rows(m_Mysql));
            }
            return true;
        }

        /*
         * 变长参数列表的 execute()函数
         * */
        bool pExecute(const char* format, ...)
        {
            if(!format)
            {
                return false;
            }

            va_list ap;
            char szQuery[MAX_QUERY_LEN] = {0};
            va_start(ap, format);
            int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
            va_end(ap);

            if(res == -1)
            {
                LOG(util::ERROR) << "SQL query truncated (and not execute) for format: " << format << std::endl;
                return false;
            }

            if(!m_Mysql)
                return false;

            {
                int iTempRet = mysql_query(m_Mysql, szQuery);
                if(iTempRet)
                {
                    unsigned int uErrno = mysql_errno(m_Mysql);
                    LOG(util::ERROR) << "mysql is abnormal, errno : " << uErrno << std::endl;
                    if(CR_SERVER_GONE_ERROR == uErrno)
                    {
                        LOG(util::ERROR) << "mysql is disconnected" << std::endl;
                        if(false == initialize(m_DBInfo._strHost, m_DBInfo._strUser,
                                               m_DBInfo._strPwd, m_DBInfo._strDBName))
                        {
                            return false;
                        }
                        LOG(util::ERROR) << szQuery << std::endl;
                        iTempRet = mysql_query(m_Mysql, szQuery);
                        if(iTempRet)
                        {
                            LOG(util::ERROR) << "SQL: " << szQuery << std::endl;
                            LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                        }
                    }
                    else
                    {
                        LOG(util::ERROR) << "SQL: " << szQuery << std::endl;
                        LOG(util::ERROR) << "query ERROR: " << mysql_error(m_Mysql) << std::endl;
                    }
                    return false;
                }
            }

            return true;
        }

        /*
         * mysql_insert_id()函数，返回前一个INSERT或UPDATE语句为AUTO_INCREMENT列(自增长)生成的值
         * */
        uint32_t getInsertID()
        {
            return (uint32_t)mysql_insert_id(m_Mysql);
        }

        /*
         * 清空上一次查询结果
         * 循环调用 mysql_store_result()函数(检索客户端的完整结果集)，获取所有查询结果
         * 并对结果调用mysql_free_result()函数（释放结果集所占用的内存）
         * */
        void clearStoreResults()
        {
            if(!m_Mysql)
            {
                return;
            }

            MYSQL_RES* result = nullptr;
            while(!mysql_next_result(m_Mysql))
            {
                if((result = mysql_store_result(m_Mysql)) != nullptr)
                {
                    mysql_free_result(result);
                }
            }
        }

        int32_t escapeString(char* szDst, const char* szSrc, uint32_t uSize)
        {
            if(m_Mysql == nullptr)
            {
                return 0;
            }

            if(szDst == nullptr || szSrc == nullptr)
            {
                return 0;
            }

            /*
             * mysql_real_escape_string()函数：转义字符串中的特殊字符, 以在SQL语句中使用
             * */
            return mysql_real_escape_string(m_Mysql, szDst, szSrc, uSize);
        }
        
    private:
        DatabaseInfo m_DBInfo; /* 存储连接数据库的信息 主机、用户、密码、数据库名称 */
        MYSQL       *m_Mysql;  /* 调用 mysql_init() 返回的句柄*/
        bool         m_bInit;  /* 该字段标明，数据库当前是否初始化 */
    };
}
