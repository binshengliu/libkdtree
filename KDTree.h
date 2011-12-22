#pragma once
#include <cmath>
#include "KDHyperRect.h"

double dist(const double pos1[], const double pos2[], size_t len);

template<int D>
class CKDNode
{
public:
	double pos[D];
	int dir;
	int data;

	CKDNode<D> *left, *right;	/* negative/positive side */
};

template<int D>
class CKDTree
{
public:
	CKDTree();
	~CKDTree(void);
	void Clear();

	int Insert(const double pos[D], int data = 0);
	void Nearest(const double pos[D], double result[D], int &data);
	void NearestRange();
private:
	int _Insert(CKDNode<D> *&node, const double pos[D], int data, int dir);
	void _Nearest(CKDNode<D> *&node, const double pos[D], double &radius, CKDHyperRect<D> rect, CKDNode<D> *&result);
	void _Delete(CKDNode<D> *&node);
private:
	void (*Destroy)(int data);
	CKDNode<D> *root;
	CKDHyperRect<D> rect;
};

template<int D>
void CKDTree<D>::Clear()
{

}
template<int D>
CKDTree<D>::CKDTree()
{
	root = 0;
	Destroy = 0;
}

template<int D>
CKDTree<D>::~CKDTree( void )
{
	if (root) {
		delete root;
	}
}

template<int D>
int CKDTree<D>::Insert( const double pos[D], int data )
{
	if (root == 0) {
		rect.Init(pos);
	} else {
		rect.Expand(pos);
	}
	return _Insert(root, pos, data, 0);
}

template<int D>
int CKDTree<D>::_Insert( CKDNode<D> *&node, const double pos[D], int data, int dir )
{
	if(!node) {
		if(!(node = new CKDNode<D>())) {
			return -1;
		}
		for (size_t i = 0; i < D; ++i) {
			node->pos[i] = pos[i];
		}

		node->data = data;
		node->dir = dir;
		node->left = node->right = 0;
		return 0;
	}
	int new_dir = (node->dir + 1) % D;
	if(pos[node->dir] < node->pos[node->dir]) {
		return _Insert(node->left, pos, data, new_dir);
	}
	return _Insert(node->right, pos, data, new_dir);
}

template<int D>
void CKDTree<D>::Nearest( const double pos[D], double result[D], int &data )
{
	double radius = dist(root->pos, pos, D);
	CKDNode<D> *nearest = 0;
	_Nearest(root, pos, radius, rect, nearest);
	for (size_t i = 0; i < D; ++i) {
		result[i] = nearest->pos[i];
	}
	data = nearest->data;
}

template<int D>
void CKDTree<D>::_Nearest( CKDNode<D> *&node, const double pos[D], double &radius, CKDHyperRect<D> rect, CKDNode<D> *&result )
{
	int dir = node->dir;
	double dummy;
	CKDNode<D> *nearer, *farther;
	dummy = pos[dir] - node->pos[dir];
	if (dummy <= 0) {
		nearer = node->left;
		farther = node->right;
	} else {
		nearer = node->right;
		farther = node->left;
	}

	/* Check the distance of the point at the current node, compare it
	 * with our best so far */
	double dist_this = dist(node->pos, pos, D);
	if (dist_this < radius) {
		//*result = node;
		radius = dist_this;
		result = node;
	}

	if (nearer) {
		if (dummy <= 0) {
			rect.bound[dir].max = node->pos[dir];
		} else {
			rect.bound[dir].min = node->pos[dir];
		}
		/* Recurse down into nearer subtree */
		_Nearest(nearer, pos, radius, rect, result);
	}

	if (farther) {
		if (dummy <= 0) {
			rect.bound[dir].min = node->pos[dir];
		} else {
			rect.bound[dir].max = node->pos[dir];
		}
		/* Check if we have to recurse down by calculating the closest
		 * point of the hyperrect and see if it's closer than our
		 * minimum distance in result_dist_sq. */
		if (rect.Distance(pos) < radius) {
			/* Recurse down into farther subtree */
			_Nearest(farther, pos, radius, rect, result);
		}
		///* Undo the slice on the hyperrect */
		//*farther_hyperrect_coord = dummy;
	}
}