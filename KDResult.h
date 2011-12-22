#pragma once
#include <vector>
#include "KDNode.h"
template<int D>
class CKDResult
{
public:
	CKDResult()
	{
		Rewind();
	}
	void Reset()
	{
		nodes.clear();
		Rewind();
	}
	void Rewind()
	{
		current = -1;
	}

	void Add(CKDNode<D> *node)
	{
		nodes.push_back(node);
	}
	bool HasNext()
	{
		return current < nodes.size() - 1;
	}
	bool Next()
	{
		assert(HasNext());
		++current;
	}
	void GetPos(double pos[D])
	{
		CKDNode<D> *&node = nodes.at(i);
		for (size_t i = 0; i < D; ++i) {
			pos[i] = node->pos[i];
		}
	}
	void GetData(int &data)
	{
		CKDNode<D> *&node = nodes.at(i);
		data = node->data;
	}
private:
	std::vector<CKDNode<D> *> nodes;
	int current;
};