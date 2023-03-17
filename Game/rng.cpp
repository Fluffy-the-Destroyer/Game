#include "rng.h"
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

int rng(int lb, int ub) {
	if (ub < lb) {
		return rng(ub, lb);
	}
	std::uniform_int_distribution<>distrib(lb, ub);
	return distrib(gen);
}

float rng(float lb, float ub) {
	if (ub - lb < 0.001) {
		return lb;
	}
	std::uniform_real_distribution<>distrib(lb, ub);
	return static_cast<float>(distrib(gen));
}