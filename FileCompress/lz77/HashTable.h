#pragma once
#include"Common.hpp"

class HashTable 
{
  public:

    HashTable(USH size);
    ~HashTable();

    //哈希表插入
    void Insert(USH& matchHead, UCH ch, USH pos, USH& hashAddr);

    //哈希函数
    void HashFunc(USH& hashAddr, UCH ch);

    //获取哈希表前一个匹配头
    USH GetNext(USH matchHead);

    //在处理大于64k的大文件时，需要把右窗中的文件拷贝到左窗，需要更新哈希表
    void Update();

  private:

    //哈希函数相关
    USH H_SHIFT();
  private:
    USH* prev_;
    USH* head_;

};
