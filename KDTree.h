#pragma once
#include <cmath>
#include <limits>
#include <cassert>
#include "KDHyperRect.h"
#include "KDNode.h"
#include "KDResult.h"
double dist(const double pos1[], const double pos2[], size_t len);

template<int D>
class CKDTree
{
public:
	CKDTree();
	~CKDTree(void);
	void Clear();

	int Insert(const double pos[D], int data = 0);
	void Nearest(const double pos[D], double result[D], int &data);
	void NearestRange(const double pos[D], double range);
	void SetDestructor(void (*dest)(int));
private:
	int _Insert(CKDNode<D> *&node, const double pos[D], int data, int dir);
	void _Nearest(CKDNode<D> *&node, const double pos[D], double &radius, CKDHyperRect<D> rect, CKDNode<D> *&result);
	void _NearestRange(CKDNode<D> *&node, const double pos[D], double range, CKDHyperRect<D> rect, CKDResult<D> &result);
	void _Delete(CKDNode<D> *&node);
private:
	void (*Destroy)(int data);
	CKDNode<D> *root;
	CKDHyperRect<D> rect;
};

template<int D>
CKDTree<D>::CKDTree()
{
	root = 0;
	Destroy = 0;
}

template<int D>
CKDTree<D>::~CKDTree( void )
{
	Clear();
}

template<int D>
void CKDTree<D>::Clear()
{
	_Delete(root);
}

template<int D>
void CKDTree<D>::_Delete( CKDNode<D> *&node )
{
	if (!node) {
		return;
	}
	_Delete(node->left);
	_Delete(node->right);
	if (Destroy) {
		Destroy(node->data);
	}
	delete node;
	node = 0;
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
	double radius = DBL_MAX;
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
	assert(node);
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
		CKDHyperRect<D> tmpRect = rect;
		if (dummy <= 0) {
			tmpRect.bound[dir].max = node->pos[dir];
		} else {
			tmpRect.bound[dir].min = node->pos[dir];
		}
		/* Recurse down into nearer subtree */
		_Nearest(nearer, pos, radius, tmpRect, result);
	}

	if (farther) {
		CKDHyperRect<D> tmpRect = rect;
		if (dummy <= 0) {
			tmpRect.bound[dir].min = node->pos[dir];
		} else {
			tmpRect.bound[dir].max = node->pos[dir];
		}
		/* Check if we have to recurse down by calculating the closest
		 * point of the hyperrect and see if it's closer than our
		 * minimum distance in result_dist_sq. */
		if (tmpRect.Distance(pos) < radius) {
			/* Recurse down into farther subtree */
			_Nearest(farther, pos, radius, tmpRect, result);
		}
		///* Undo the slice on the hyperrect */
		//*farther_hyperrect_coord = dummy;
	}
}

template<int D>
void CKDTree<D>::NearestRange( const double pos[D], double range )
{
	static CKDResult<D> result;
	result.Reset();
	_NearestRange(root, pos, range, rect, result);
}

template<int D>
void CKDTree<D>::_NearestRange( CKDNode<D> *&node, const double pos[D], double range, CKDHyperRect<D> rect, CKDResult<D> &result )
{
	assert(node);
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
	if (dist_this < range) {
		result.Add(node);
	}

	if (nearer) {
		CKDHyperRect<D> tmpRect = rect;
		if (dummy <= 0) {
			tmpRect.bound[dir].max = node->pos[dir];
		} else {
			tmpRect.bound[dir].min = node->pos[dir];
		}
		/* Recurse down into nearer subtree */
		_NearestRange(nearer, pos, range, tmpRect, result);
	}

	if (farther) {
		CKDHyperRect<D> tmpRect = rect;
		if (dummy <= 0) {
			tmpRect.bound[dir].min = node->pos[dir];
		} else {
			tmpRect.bound[dir].max = node->pos[dir];
		}
		/* Check if we have to recurse down by calculating the closest
		 * point of the hyperrect and see if it's closer than our
		 * minimum distance in result_dist_sq. */
		if (tmpRect.Distance(pos) < range) {
			/* Recurse down into farther subtree */
			_NearestRange(farther, pos, range, rect, result);
		}
		///* Undo the slice on the hyperrect */
		//*farther_hyperrect_coord = dummy;
	}
}
template<int D>
void CKDTree<D>::SetDestructor( void (*dest)(int) )
{
	Destroy = dest;
}