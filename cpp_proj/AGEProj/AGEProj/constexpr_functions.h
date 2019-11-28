#pragma once

constexpr double pow_c(double x, int y)
{
	return y == 0 ? 1 : (y > 0 ? x * pow_c(x, y - 1) : pow_c(x, y + 1) / x);
}

constexpr unsigned floor_log2(unsigned x)
{
	return x == 1 ? 0 : 1 + floor_log2(x >> 1);
}

constexpr unsigned ceil_log2(unsigned x)
{
	return x == 1 ? 0 : floor_log2(x - 1) + 1;
}