#if 0
#include "HuffManTree.hpp"
#include "FileCompressHuff.h"
#include <Windows.h>

void TestHuffManTree()
{
	vector<int> v{ 3,1,7,5 };
	//HuffManTree<int> t(v,);
	// t.Print;
}

void TestFileCompressHuff()
{
	FileCompressHuff fc;
	//fc.CompressFile("1.txt");
	fc.UnCompressFile("2.txt");
}

int main()
{
	//TestHuffManTree();
	TestFileCompressHuff();
	system("pause");
	return 0;

}

#endif


#define _CRT_SECURE_NO_WARNINGS 1
#include "HuffManTree.hpp"
#include "FileCompressHuff.h"


int main()
{
	FileCompressHuffman fc;
	  fc.CompressFile("1.txt");
	fc.UnCompressFile("2.txt");

	return 0;
}
