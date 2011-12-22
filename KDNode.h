#pragma once

template<int D>
class CKDNode
{
public:
	double pos[D];
	int dir;
	int data;

	CKDNode<D> *left, *right;	/* negative/positive side */
};
