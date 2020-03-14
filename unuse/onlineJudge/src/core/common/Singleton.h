/*
 * fileName: Singleton.h 
 * 单例模式
 * */
#pragma once
#include <mutex>

#define SINGLETON_ON_1 1

#ifdef SINGLETON_ON_1
template <typename T>
class Singleton
{
public:
    static T& instance()
    {
        if(nullptr == _value)
        {
            _value = new T();
        }

        return *_value;
    }
private:
    // 构造函数私有化 这样就没办法在类外创建对象
    // 所以需要提供一个静态方法 获取实例，如果实例不存在，需要创建
    Singleton()
    {}

    // c++11 的新玩法，禁止掉拷贝构造函数和赋值运算符重载
    Singleton(const Singleton& singleton) = delete ;
    Singleton& operator= (const Singleton& singleton) = delete;

    static void init()
    {
        _value = new T();
    }

    static void destroy()
    {
        delete _value;
    }
private:
    static T* _value;
};


template <typename T>
T* Singleton<T>::_value = nullptr;

#else

class Singleton
{
public:
    static Singleton* getInstance()
    {
        if(nullptr == m_pInstance)
        {
            m_mutex.lock();
            if(nullptr == m_pInstance)
            {
                m_pInstance = new Singleton();
            }
            m_mutex.unlock();
        }
        return m_pInstance;
    }
private:
    Singleton()
    {}

    Singleton(const Singleton& ) = delete;
    Singleton& operator= (const Singleton& ) = delete;
private:
    static Singleton* volatile m_pInstance;
    static std::mutex m_mutex;
};

Singleton* volatile Singleton::m_pInstance = nullptr;
std::mutex Singleton::m_mutex;
#endif /* !SINGLETON_ON_1 */
