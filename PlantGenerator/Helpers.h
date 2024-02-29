#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>

// Clamp helpers
float clamp(float& value, float low, float high) {
	return (std::max(low, std::min(value, high)));
}
int clamp(int& value, float low, float high) {
	return (value < low ? low : (value > high ? high : value));
}

#endif