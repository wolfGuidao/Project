#if 0
#pragma once
#include <queue>
#include <iostream>
#include <vector>
using namespace std;

template<class W>
struct HuffManTreeNode
{
	HuffManTreeNode(const W& weight = W())
		:_weight(weight)
		, _pLeft(nullptr)
		, _pRight(nullptr)
		,_pParent(nullptr)
	{}
	HuffManTreeNode<W>* _pLeft;
	HuffManTreeNode<W>* _pRight;
	HuffManTreeNode<W>* _pParent;
	W _weight;//节点的权值
};

template<class W>
class Less
{
	typedef HuffManTreeNode<W> Node;

public:

	bool operator()(const Node* pLeft, const Node* pRight)
	{
		return pLeft->_weight > pRight->_weight;
	}
};

template<class W>
class HuffManTree
{
	typedef HuffManTreeNode<W> Node;

public:
	HuffManTree()
		:_pRoot(nullptr)
	{

	}

	HuffManTree(const vector<W> vWeight,const W& invalid)
	{
		CreatHuffManTree(vWeight,invalid);
	}

	void CreatHuffManTree(const vector<W> vWeight, const W& invalid)
	{
		//1.构建森林
		priority_queue<Node*, vector<Node*>, Less<W> >q;
		for (auto e : vWeight)
		{
			if (e == invalid)
			{
				//如果为无效的字符就跳过.过滤掉权值为0的字符
				continue;
			}
			q.push(new Node(e));
		}

		//2.判断森林中二叉树的个数是否超过两颗
		while (q.size() > 1)
		{
			Node* pLeft = q.top();
			q.pop();
			Node* pRight = q.top();
			q.pop();

			Node* pParent = new Node(pLeft->_weight + pRight->_weight);
			pParent->_pLeft = pLeft;
			pParent->_pRight = pRight;

			pLeft->_pParent = pParent;
			pRight->_pParent = pParent;
			q.push(pParent);
		}

		//3.哈夫曼树创建成功
		_pRoot = q.top();
	}

	Node* GetRoot()
	{
		return _pRoot;
	}

	~HuffManTree()
	{
		_DestroyTree(_pRoot);
	}

private:
	//注意这里如果想要在函数内部改变pRoot的指向，需要传传一级指针的引用或者二级指针
	void _DestroyTree(Node*& pRoot)
	{
		//后序遍历的方式销毁树
		if (pRoot)
		{
			_DestroyTree(pRoot->_pLeft);
			_DestroyTree(pRoot->_pRight);
			delete pRoot;
			pRoot = nullptr;
		}
	}

private:
	Node* _pRoot;
};

#endif

#pragma once

#include <vector>
#include <queue>

template<class T>
struct HuffmanTreeNode
{
	HuffmanTreeNode(const T& weight = T())
		: _pLeft(nullptr)
		, _pRight(nullptr)
		, _pParent(nullptr)
		, _Weight(weight)
	{}
	HuffmanTreeNode<T>* _pLeft;
	HuffmanTreeNode<T>* _pRight;
	HuffmanTreeNode<T>* _pParent;
	T _Weight;  //权值
};

template<class T>
class Less
{
	typedef HuffmanTreeNode<T> Node;

public:
	bool operator()(const Node* pLeft, const Node* pRight)
	{
		return pLeft->_Weight > pRight->_Weight;  //根据大于的方式构造出小根堆
	}
};

template<class T>
class HuffmanTree
{
	typedef HuffmanTreeNode<T> Node;

public:
	HuffmanTree()
		: _pRoot(nullptr)
	{}

	HuffmanTree(const std::vector<T> vWeight, const T& invalid_weight)
		: _pRoot(nullptr)
	{
		CreateHuffmanTree(vWeight, invalid_weight);
	}

	~HuffmanTree()
	{
		Destory(_pRoot);
	}

	Node* GetRoot()
	{
		return _pRoot;
	}


	void CreateHuffmanTree(const std::vector<T> vWeight, const T& invalid_weight)
	{
		//1、构造森林
		std::priority_queue<Node*, std::vector<Node*>, Less<T>> queue;
		for (auto e : vWeight)
		{
			if (e == invalid_weight)
			{//无效的字符，即未出现的字符，_count == 0
				continue;
			}
			queue.push(new Node(e));
		}

		//2、将森林中的树不断合并，构造haffman树
		while (queue.size() > 1)
		{
			Node* pLeft = queue.top();
			queue.pop();

			Node* pRight = queue.top();
			queue.pop();

			Node* pParent = new Node(pLeft->_Weight + pRight->_Weight);
			pParent->_pLeft = pLeft;
			pParent->_pRight = pRight;
			pLeft->_pParent = pParent;
			pRight->_pParent = pParent;

			queue.push(pParent);
		}

		_pRoot = queue.top();
	}
private:

	void Destory(Node*& pRoot)
	{
		if (pRoot)
		{
			Destory(pRoot->_pLeft);
			Destory(pRoot->_pRight);
			delete pRoot;
			pRoot = nullptr;
		}
	}
private:
	Node* _pRoot;
};
