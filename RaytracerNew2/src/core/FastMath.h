#pragma once

#include "tools/Array2D.h"
#include "Color.h"
#include "Geometry.h"

namespace math
{
	//template<class T>
	//inline T fastExp(const T& value)
	//{
	//	return std::exp(value);
	//}

	//For some reasons, the template function yields compilation errors due to ambiguous functions resolving with Eigen
	inline float fastExp(float value)
	{
		return std::exp(value);
	}

	inline double fastExp(double value)
	{
		return std::exp(value);
	}

	//template<>
	inline Color fastExp(const Color& c)
	{
		return Color(c.exp());
		//real res[Color::NB_SAMPLES];
		//for (int i = 0; i < c.NB_SAMPLES; i++)
		//	res[i] = fastExp(c.coeff(i));
		//return Color(res);
		//return Color(fastExp(c.r()), fastExp(c.g()), fastExp(c.b()));
	}

	//template<class T>
	//inline T fastLog(T value)
	//{
	//	return std::log(value);
	//}

	inline float fastLog(float value)
	{
		return std::log(value);
	}

	inline double fastLog(double value)
	{
		return std::log(value);
	}

	//template<class T>
	//inline T fastSqrt(const T& value)
	//{
	//	return std::sqrt(value);
	//}

	inline float fastSqrt(float value)
	{
		return std::sqrt(value);
	}

	inline double fastSqrt(double value)
	{
		return std::sqrt(value);
	}

	//template<>
	inline Color fastSqrt(const Color& c)
	{
		return Color(c.sqrt());
		//return Color(fastSqrt(c.r()), fastSqrt(c.g()), fastSqrt(c.b()));
	}



}