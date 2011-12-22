#include "KDTree.h"

double dist(const double pos1[], const double pos2[], size_t len)
{
	double result = 0.0;
	for (size_t i = 0; i < len; ++i) {
		result += std::pow(pos1[i] - pos2[i], 2);
	}
	return sqrt(result);
}
