#pragma once
#include <cmath>

template<int D>
double dist(const double pos1[D], const double pos2[D])
{
	double result = 0.0;
	for (size_t i = 0; i < D; ++i) {
		result += std::pow(pos1[i] - pos2[i], 2);
	}
	return sqrt(result);
}

template<int D>
class CKDHyperRect;

template<int D>
double dist_rect(const CKDHyperRect<D> rect, const double pos[D])
{
	double result = 0;

	for (size_t i = 0; i < D; ++i) {
		if (pos[i] < rect.bound[i].min) {
			result += pow(rect.bound[i].min - pos[i], 2);
		} else if (pos[i] > rect.bound[i].max) {
			result += pow(rect.bound[i].max - pos[i], 2);
		} else {
			result += pow(pos[i], 2);
		}
	}

	result = sqrt(result);
	return result;
}


template<int D, class T>
class CKDNode
{
public:
	double pos[D];
	int dir;
	T *data;

	CKDNode<D, T> *left, *right;	/* negative/positive side */
};

class CInterval
{
public:
	CInterval()
	{
		min = max = 0;
	}
	double min;
	double max;
};
template<int D>
class CKDHyperRect
{
public:
	CInterval bound[D];
};

template<int D, class T>
class CKDTreeX
{
public:
	CKDTreeX();
	~CKDTreeX(void);
	void Clear();

	int Insert(const double pos[D], T *data = 0);
	int _Insert(CKDNode<D, T> *&node, const double pos[D], T *data, int dir);
	void Nearest(const double pos[D], double result[D], T *&data);
	void _Nearest(CKDNode<D, T> *&node, const double pos[D], double &radius, CKDHyperRect<D> rect, CKDNode<D, T> *&result);
	void NearestRange();
private:
	static void data_free(void *data)
	{
		delete data;
	}
private:
	CKDNode<D, T> *root;
	CKDHyperRect<D> rect;
};

template<int D, class T>
CKDTreeX<D, T>::CKDTreeX()
{
	root = 0;
}

template<int D, class T>
CKDTreeX<D, T>::~CKDTreeX( void )
{
	if (root) {
		delete root;
	}
}

template<int D, class T>
int CKDTreeX<D, T>::Insert( const double pos[D], T *data )
{
	if (root == 0) {
		// 新建矩形区域
		for (size_t i=0; i < D; ++i) {
			rect.bound[i].min = pos[i];
			rect.bound[i].max = pos[i];
		}
	} else {
		// 扩展矩形区域
		for (size_t i=0; i < D; ++i) {
			if (pos[i] < rect.bound[i].min) {
				rect.bound[i].min = pos[i];
			}
			if (pos[i] > rect.bound[i].max) {
				rect.bound[i].max = pos[i];
			}
		}
	}
	return _Insert(root, pos, data, 0);
}

template<int D, class T>
int CKDTreeX<D, T>::_Insert( CKDNode<D, T> *&node, const double pos[D], T *data, int dir )
{
	if(!node) {
		if(!(node = new CKDNode<D, T>())) {
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

template<int D, class T>
void CKDTreeX<D, T>::Nearest( const double pos[D], double result[D], T *&data )
{
	double radius = dist<D>(root->pos, pos);
	CKDNode<D, T> *nearest = 0;
	_Nearest(root, pos, radius, rect, nearest);
	for (size_t i = 0; i < D; ++i) {
		result[i] = nearest->pos[i];
	}
	data = nearest->data;
}

template<int D, class T>
void CKDTreeX<D, T>::_Nearest( CKDNode<D, T> *&node, const double pos[D], double &radius, CKDHyperRect<D> rect, CKDNode<D, T> *&result )
{
	int dir = node->dir;
	double dummy;
	CKDNode<D, T> *nearer, *farther;
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
	double dist_this = dist<D>(node->pos, pos);
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
		if (dist_rect<2>(rect, pos) < radius) {
			/* Recurse down into farther subtree */
			_Nearest(farther, pos, radius, rect, result);
		}
		///* Undo the slice on the hyperrect */
		//*farther_hyperrect_coord = dummy;
	}
}