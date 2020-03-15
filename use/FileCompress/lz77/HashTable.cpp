#include "HashTable.h"

const USH HASH_BITS = 15;                  //哈希地址15位
const USH HASH_SIZE = (1 << HASH_BITS);    //哈希地址个数 32K
const USH HASH_MASK = HASH_SIZE - 1;       //防止溢出  低15位全1,因为prev的大小就WSIZE，而当start到达右窗
//时，下标明显大于WSIZE，如果不处理就会下标越界

HashTable::HashTable(USH size)
  :prev_(new USH[size * 2])     //哈希表中存放的是索引字符串的首地址(距离字符串开始的相对下标)
   , head_(prev_ + size)
{
  memset(prev_, 0, size * 2 * sizeof(USH));//初始化为0，0也代表当前匹配串是链表的末尾（相当于用数组模拟的链表）
}

HashTable::~HashTable() 
{
  delete[] prev_;
  prev_ = nullptr;

}
//matchHead：匹配链的头  
//ch：查找字符串的第三个字符(也就是最后一个) 
//pos：查找字符串的头到字符串开始的距离
//hashAddr：输入时是上一次的哈希地址,输出时是本次哈希地址       
void HashTable::Insert(USH& matchHead, UCH ch, USH pos, USH& hashAddr) 
{
  HashFunc(hashAddr, ch);   //获取本次插入的哈希地址

  matchHead = head_[hashAddr];//找当前三个字符在查找缓冲区中找到的最近的一个，即匹配链的头

  //将新的哈希地址插入链表
  prev_[pos & HASH_MASK] = head_[hashAddr];//&的目的是防止越界
  head_[hashAddr] = pos;
}

//abcdefgh字符串
//hashaddr1:abc
//hashaddr2:bcd

//hashAddr:前一次计算出的哈希地址  abc
//本次需要计算bcd哈希地址
//ch:本次匹配三个字符中的最后一个
//本次哈希地址是在上一次哈希地址的基础上计算出来的
void HashTable::HashFunc(USH& hashAddr, UCH ch) 
{      
  //hashAddr是输入，输出型参数，ch是所查找字符串中第一个字符
  hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch))&HASH_MASK;
}

USH HashTable::H_SHIFT() 
{
  return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;     //5
}

//获取当前匹配头的下一个匹配头（如果为0代表匹配结束）
USH HashTable::GetNext(USH matchHead) 
{
  return prev_[matchHead & HASH_MASK];
}

//更新哈希表
void HashTable::Update() 
{
  for (USH i = 0; i < WSIZE; ++i) 
  {
    //先更新head,把大于等于WSIXE的下标 减去 WSIZE 放到哈希表的左窗内
    //把head中小于WSIZE 的下标直接置为0，因为小于WSIZE的下标在查找缓冲区中距离先行缓冲区的距离太远了，进行匹配不划算
    if (head_[i] >= WSIZE)
      head_[i] -= WSIZE;
    else
      head_[i] = 0;

    //更新prev，和head同理
    if (prev_[i] >= WSIZE)
      prev_[i] -= WSIZE;
    else
      prev_[i] = 0;
  }
}
