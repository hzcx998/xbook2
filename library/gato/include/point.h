#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <math.h>
	typedef struct point_t
	{
		float x;
		float y;
	} point_t;

	static point_t constant_point_add_point(point_t p1, point_t p2, float ratio)
	{
		return (point_t){(p1.x + p2.x) * ratio, (p1.y + p2.y) * ratio};
	}

	static point_t point_sub_point(point_t p1, point_t p2)
	{
		return (point_t){(p1.x - p2.x), (p1.y - p2.y)};
	}

	static point_t point_add_point(point_t p1, point_t p2)
	{
		return (point_t){(p1.x + p2.x), (p1.y + p2.y)};
	}

	static point_t point_min(point_t min, point_t p)
	{
		return (point_t){fminf(min.x, p.x), fminf(min.y, p.y)};
	}

	static point_t point_max(point_t min, point_t p)
	{
		return (point_t){fmaxf(min.x, p.x), fmaxf(min.y, p.y)};
	}

	static point_t unit_point(point_t p1)
	{
		float l = sqrtf(p1.x * p1.x + p1.y * p1.y);
		return (point_t){(p1.x / l), (p1.y / l)};
	}

	static float point_cross_point(point_t p1, point_t p2)
	{
		return p1.x * p2.y - p1.y * p2.x;
	}

	static float point_dot_point(point_t p1, point_t p2)
	{
		return p1.x * p2.x + p1.y * p2.y;
	}

	static point_t point_mul_factor(point_t p1, float factor)
	{
		return (point_t){(p1.x * factor), (p1.y * factor)};
	}

#ifdef __cplusplus
}
#endif