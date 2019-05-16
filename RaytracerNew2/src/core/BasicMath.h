#pragma once
#include "tools/Array2D.h"
#include "Geometry.h"

namespace math
{
#ifdef DOUBLE_PRECISION
	const real PI = 3.14159265358979323846;
	const real INV_PI = 0.3183098861837906715377;
	const real INV_FOUR_PI = 0.07957747154594766788450883936693;
	const real MAX_REAL = 10e9;
	const real EPSILON = 1e-4;
#else
	const real PI = 3.14159265358979323846f;
	const real INV_PI = 0.3183098861837906715377f;
	const real INV_FOUR_PI = 0.07957747154594766788450883936693f;
	const real MAX_REAL = 10e7f;
	const real EPSILON = 1e-4f;
#endif

	inline real toRadian(real degres)
	{
		return (degres * math::PI) / (real)180.;
	}

	inline real toDegre(real radian)
	{
		return (radian * (real)180.) / math::PI;
	}

	template<typename T>
	T max(T a) {
		return a;
	}

	template<typename T, typename... Args>
	T max(T first, Args... args) {
		return std::max(first, max(args...));
	}

	template<class T>
	T thresholding(T val, T min, T max)
	{
		return val < min ? min : val  > max ? max : val;
	}

	template<class U = int, class T>
	U sign(T val)
	{
		if (val >= (T)0)
			return (U)1;
		return (U)-1;
	}

	template<class T>
	inline T interp1(real advance, T start, T end)
	{
		return ((real)1 - advance) * start + advance * end;
	}

	template<class T>
	inline T interp2(const Point2d& xy, const Array2D<T>& array)
	{
		real advanceX = xy.x() - (int)xy.x();
		real advanceY = xy.y() - (int)xy.y();
		int xMin = math::thresholding((int)xy.x(), 0, (int)array.getWidth() - 2);
		int yMin = math::thresholding((int)xy.y(), 0, (int)array.getHeight() - 2);
		int xMax = xMin + 1 < (int)array.getWidth() ? xMin + 1 : xMin;
		xMax = thresholding(xMax, 0, (int)array.getWidth() - 1);
		int yMax = yMin + 1 < (int)array.getHeight() ? yMin + 1 : yMin;
		yMax = thresholding(yMax, 0, (int)array.getHeight() - 1);

		return (1 - advanceX) * (1 - advanceY) * array(xMin, yMin) +
			(1 - advanceX) * advanceY * array(xMin, yMax) +
			advanceX * (1 - advanceY) * array(xMax, yMin) +
			advanceX * advanceY * array(xMax, yMax);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Basic random function. Requires that the seed was initialized 
	/// before
	///
	/// \param min : min random value
	///
	/// \param max : max value included (?)
	///
	/// \return a random value between [min, max]
	///////////////////////////////////////////////////////////////////////////
	inline real random(real min, real max)
	{
		return (rand() / (real)RAND_MAX) * (max - min) + min;
	}
}
