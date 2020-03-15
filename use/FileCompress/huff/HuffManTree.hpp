#pragma once

#include <vector>
#include <queue>

template<class T>
struct HuffmanTreeNode
{
  //如果T是内置类型，T()就等于0 ，如果T是自定义类型，T()就等于调用默认无参构造函数
	HuffmanTreeNode(const T& weight = T())
		: _pLeft(nullptr)
		, _pRight(nullptr)
		, _pParent(nullptr)//加入指向父亲节点的指针，方便遍历
		, _Weight(weight)
	{}

	HuffmanTreeNode<T>* _pLeft;
	HuffmanTreeNode<T>* _pRight;
	HuffmanTreeNode<T>* _pParent;
	T _Weight;  //权值
};

//自定义比较方式
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

//哈夫曼树
//哈夫曼树的原理是：先从容器中取出两个权值最小的数，并把它们相加的结果放回原来的容器中，依次类推，直到
//容器中只剩下一个权值信息，就说明哈夫曼树构建成功
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
			{
        //无效的字符，即未出现的字符，_count == 0
				continue;
			}
			queue.push(new Node(e));
		}

		//2、将森林中的树不断合并，构造haffman树
		while (queue.size() > 1)
		{
      //因为上面构造的是小堆，所以每次取都是取的最小的权值
			Node* pLeft = queue.top();
			queue.pop();

			Node* pRight = queue.top();
			queue.pop();

      //根据哈夫曼的构造原理，创建出来的新节点是选出来的最小的两个权值之和
			Node* pParent = new Node(pLeft->_Weight + pRight->_Weight);

      //更新相关指针的指向
			pParent->_pLeft = pLeft;
			pParent->_pRight = pRight;
			pLeft->_pParent = pParent;
			pRight->_pParent = pParent;

      //最后把创建出来的新节点放到优先级队列中继续构造哈夫曼树
			queue.push(pParent);
		}

    //返回构造完成的哈夫曼树的根节点
		_pRoot = queue.top();
	}
private:

  //销毁哈夫曼树---后序销毁,防止资源泄露
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
  //哈夫曼树的根节点
	Node* _pRoot;
};
