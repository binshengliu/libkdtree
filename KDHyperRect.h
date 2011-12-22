#pragma once

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
	void Init(const double pos[D]);
	void Expand(const double pos[D]);
	double Distance(const double pos[D]);
};

template<int D>
void CKDHyperRect<D>::Init( const double pos[D] )
{
	// 新建矩形区域
	for (size_t i=0; i < D; ++i) {
		bound[i].min = pos[i];
		bound[i].max = pos[i];
	}
}
template<int D>
void CKDHyperRect<D>::Expand( const double pos[D] )
{
	// 扩展矩形区域
	for (size_t i=0; i < D; ++i) {
		if (pos[i] < bound[i].min) {
			bound[i].min = pos[i];
		}
		if (pos[i] > bound[i].max) {
			bound[i].max = pos[i];
		}
	}
}
template<int D>
double CKDHyperRect<D>::Distance( const double pos[D] )
{
	double result = 0;

	for (size_t i = 0; i < D; ++i) {
		if (pos[i] < bound[i].min) {
			result += pow(bound[i].min - pos[i], 2);
		} else if (pos[i] > bound[i].max) {
			result += pow(bound[i].max - pos[i], 2);
		} else {
			result += pow(pos[i], 2);
		}
	}

	result = sqrt(result);
	return result;
}