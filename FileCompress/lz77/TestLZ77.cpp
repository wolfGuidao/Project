#include"LZ77.h"

int main()
{
  LZ77 lz;
  lz.CompressFile("1.txt");
  lz.UNCompressFile("2.lzp");
  system("pause");
  return 0;

}
