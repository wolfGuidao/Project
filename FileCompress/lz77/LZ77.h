#pragma once
#include "HashTable.h"

class LZ77 
{
  public:

    LZ77();
    ~LZ77();
    void CompressFile(const std::string& strFilePath);
    void UNCompressFile(const std::string& strFilePath);

  private:

    USH LongestMatch(USH matchHead, USH& curMatchDist, USH start);      //找最长匹配

    void WriteFlag(FILE* fOUT, UCH& chFlag, UCH& bitCount, bool isLen); //写标记文件

    //合并压缩数据文件和标记信息文件
    void MergeFile(FILE* fOut, ULL fileSize);

    //在处理大于64k文件时，需要重新填充缓冲区的右窗口
    void FillWindow(FILE* fIn, size_t& lookAhead, USH& start);

    //获取文件的后缀
    std::string GetStr(std::string filename);

  private:

    UCH* pWin_;                                                         //用来保存待压缩数据的缓冲区
    HashTable ht_;
};
