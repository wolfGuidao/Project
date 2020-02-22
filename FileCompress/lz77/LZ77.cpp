#include"LZ77.h"
#include<iostream>

const USH MIN_LOOKAHEAD = MAX_MATCH + MIN_MATCH + 1; //要保证最后一次匹配,最大匹配长度258 
const USH MAX_DIST = WSIZE - MIN_LOOKAHEAD;//最大的匹配距离，如果太远再去进行匹配，耗时，不划算    

LZ77::LZ77()
  :pWin_(new UCH[WSIZE * 2])
   ,ht_(WSIZE)
{}

LZ77::~LZ77() 
{
  delete[] pWin_;
  pWin_ = nullptr;

}

void LZ77::CompressionFile(const std::string& fileName) 
{
  FILE* fIn = fopen(fileName.c_str(), "rb");
  if (!fIn) 
  {
    std::cout << "待压缩文件打开失败!" << std::endl;
    return;
  }

  //计算文件大小
  fseek(fIn, 0, SEEK_END);//把文件指针移动到文件末尾
  ULL fileSize = ftell(fIn);//获取当前文件指针到文件起始位置的偏移量即文件大小
  if (fileSize <= MIN_MATCH) 
  {
    std::cout << "文件太小,不进行压缩！！！" << std::endl;
    return;
  }

  //将文件指针置回起始位置
  fseek(fIn, 0, SEEK_SET);

  //读取文件到缓冲区
  ULL readSize = fread(pWin_, sizeof(UCH), 2 * WSIZE, fIn);

  USH hashAddr = 0;//计算前两个字符的哈希地址
  for (UCH i = 0; i < MIN_MATCH - 1; ++i) 
  {
    ht_.hashFunc(hashAddr, pWin_[i]);
  }

  //与查找最长匹配相关的变量
  USH matchHead = 0;//匹配链的头
  USH curMatchLen = 0; //最长匹配链的长度
  USH curMatchDist = 0; //最长匹配链的距离
  USH start = 0;

  FILE* fOutD = fopen("2.lzp", "wb");//写压缩数据
  FILE* fOutF = fopen("3.txt", "wb");//写数据的标记
  if (!fOutD || !fOutF) 
  {
    std::cout << "2xt/3.txt 文件打开失败" << std::endl;
    return;
  }

  //与标记相关的变量
  UCH chFlag = 0;   //将要写入的标记
  UCH bitCount = 0; //记录 标记写了多少位
  ULL lookAhead = readSize;//表示当前先行缓冲区中剩余的字节数

  while (lookAhead) 
  {
    //将首字符串插入哈希表
    ht_.Insert(matchHead, pWin_[start + 2], start, hashAddr);

    //因为不只一次匹配，需要置为0
    curMatchLen = 0;//当前匹配链的长度
    curMatchDist = 0;//当前匹配的距离

    if (matchHead > 0) 
    { 
      //找到了匹配链
      //找最长匹配链
      curMatchLen = LongestMatch(matchHead, curMatchDist, start);
    }

    if (curMatchLen < MIN_MATCH) 
    {
      //未找到匹配链
      //写原字符
      fputc(pWin_[start], fOutD);

      //写标记
      WriteFlag(fOutF, chFlag, bitCount, false);

      ++start;
      --lookAhead;
    }
    else 
    {
      //写长度
      UCH chlen = curMatchLen - 3;
      fputc(chlen, fOutD);

      //写距离
      fwrite(&curMatchDist, sizeof(curMatchDist), 1, fOutD);

      //写标记
      WriteFlag(fOutF, chFlag, bitCount, true);

      //将匹配的字符串三个一组插入哈希表
      lookAhead -= curMatchLen;
      ++start;
      --curMatchLen;//第一个字符已经插入
      while (curMatchLen) 
      {
        ht_.Insert(matchHead, pWin_[start + 2], start, hashAddr);
        ++start;
        --curMatchLen;
      }
    }

    if (bitCount > 0 && bitCount < 8) 
    {
      chFlag <<= (8 - bitCount);
      fputc(chFlag, fOutF);
    }

    if (readSize < 2 * WSIZE)
      break;

    readSize = fread(pWin_, sizeof(UCH), 2 * WSIZE, fIn);
  }

  fclose(fIn);
  fclose(fOutD);
  fclose(fOutF);
}

USH LZ77::LongestMatch(USH matchHead, USH& MatchDist, USH start) 
{
  //找最长匹配
  USH curMatchLen = 0;  //一次匹配的长度
  USH maxMatchLen = 0;//
  UCH maxMatchCount = 255;   //最大的匹配次数，解决环状链
  USH curMatchStart = 0;     //当前匹配在查找缓冲区中的起始位置

  //在先行缓冲区中查找匹配时，不能太远即不能超过MAX_DIST
  USH limit = start > MAX_DIST ? start - MAX_DIST : 0;

  do 
  {
    //匹配范围
    //先行缓冲区
    UCH* pstart = pWin_ + start;
    UCH* pend = pstart + MAX_MATCH;

    //查找缓冲区匹配串的起始
    UCH* pMatchStart = pWin_ + matchHead;
    curMatchLen = 0;

    //可以进行本次匹配
    while (pstart < pend && *pstart == *pMatchStart) 
    {
      ++curMatchLen;
      ++pstart;
      ++pMatchStart;
    }

    //一次匹配结束,看是否需要更新最长匹配字符串的长度
    if (curMatchLen > maxMatchLen) 
    {
      maxMatchLen = curMatchLen;
      curMatchStart = matchHead;
    }
  } while ((matchHead = ht_.GetNext(matchHead)) > limit&&maxMatchCount--);

  MatchDist = start - curMatchStart;
  return maxMatchLen;
}

//chFlag用来区分当前字节是原字符还是长度，0：原字符，1：长度
//bitcount：代表该字节中的多少个bit位已经被设置
//isLen：代表当前字符是字符还是长度
void LZ77::WriteFlag(FILE* fOut, UCH& chFlag, UCH& bitCount, bool isLen) 
{
  chFlag <<= 1;

  if (isLen) 
    chFlag |= 1;

  ++bitCount;

  if (bitCount == 8) 
  {
    fputc(chFlag, fOut);
    bitCount = 0;
    chFlag = 0;
  }
}
void LZ77::UnCompressionFile(const std::string& fileName) 
{

}
