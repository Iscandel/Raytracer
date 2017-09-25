#ifndef H__COLOR_170520151941__H
#define H__COLOR_170520151941__H

#include <algorithm>
#include <cmath>
#include <limits.h>
#include <iostream>
#include <vector>

#include "Logger.h"

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

	double max()
	{
		return std::max(r, std::max(g, b));
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

class ColorSpaceConversion 
{
public:
	void precompute();

	std::vector<double> X;
	std::vector<double> Y;
	std::vector<double> Z;

	double xyz[3];

	static bool computed;

	static const int LAMBDA_START = 400; 
	static const int LAMBDA_END = 700;
	static const int CIE_SAMPLE_NUMBER = 471;
	const double CIE_Y_INTEGRAL = 106.856895;

	static const int SAMPLE_NUMBER = 30;

	static const double CIE_X[CIE_SAMPLE_NUMBER];
	static const double CIE_Y[CIE_SAMPLE_NUMBER];
	static const double CIE_Z[CIE_SAMPLE_NUMBER];
	static const double CIE_lambda[CIE_SAMPLE_NUMBER];

	ColorSpaceConversion(const std::vector<double>& lambda, const std::vector<double>& spectrum);

	void toRGB(double rgb[3]) const;

	void toXYZ(double _xyz[3]) const;

	void XYZToRGB(const double _xyz[3], double rgb[3]) const;

	double AverageSpectrumSamples(const double *lambda, const double *vals,
		int n, double lambdaStart, double lambdaEnd);

protected:
	double c[SAMPLE_NUMBER];

};

inline std::ostream& operator << (std::ostream& o, const Color& color)
{
	o << "[" << color.r << " " << color.g << " " << color.b << "]" << std::endl;

	return o;
}

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
