#ifndef H__COLOR_170520151941__H
#define H__COLOR_170520151941__H

#include <cmath>
#include <limits.h>
class Color
{
public:
	explicit Color(double r, double g, double b);

	explicit Color(double val = 0.);
	//explicit Color(int r, int g, int b);


	void validate() 
	{
		r = r > 1. ? 1. : r < 0. ? 0. : r;
		g = g > 1. ? 1. : g < 0. ? 0. : g;
		b = b > 1. ? 1. : b < 0. ? 0. : b;
	}

	double luminance() const
	{
		return 0.2126 * r + 0.7152 * g + 0.0722 *b;
	}

	bool isZero() const
	{
		const double eps = 1e-9;
		return r < eps && g < eps && b < eps;
	}

	bool isNan() const
	{
		if (std::isnan(r) || std::isnan(g) || std::isnan(b))
			return true;
		return false;
	}

public:
	double r;
	double g;
	double b;
};


bool operator == (const Color& c1, const Color& c2);

bool operator != (const Color& c1, const Color& c2);

Color operator + (const Color& c1, const Color& c2);

Color& operator += (Color& c1, const Color& c2);

Color operator - (const Color& c1, const Color& c2);

Color& operator -= (Color& c1, const Color& c2);

Color operator * (const Color& c1, const Color& c2);

Color& operator *= (Color& c1, const Color& c2);

Color& operator *= (Color& c1, double val);

Color operator *(double value, const Color& other);

Color operator *(const Color& other, double value);

Color operator /(double value, const Color& other);

Color operator /(const Color& other, double value);

Color& operator /=(Color& other, double value);

Color operator /(const Color& col, const Color& other);

Color operator /= (Color& col, const Color& other);
//Color operator *(Color& col, double value);

Color operator + (const Color& c1, double value);

Color& operator += (Color& c1, double value);


#endif
