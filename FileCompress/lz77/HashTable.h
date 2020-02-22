#pragma once
#include"Common.hpp"

class HashTable 
{
  public:

    HashTable(USH size);
    ~HashTable();
    void Insert(USH& matchhead, UCH ch, USH pos, USH& hashAddr);
    void hashFunc(USH& hashAddr, UCH ch);
    USH GetNext(USH matchHead);

  private:

    USH H_SHIFT();
    USH *prev_;
    USH *head_;

};
