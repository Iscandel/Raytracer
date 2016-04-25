#include "Color.h"


Color::Color(double r, double g, double b)
	:r(r)
	,g(g)
	,b(b)
{
	//validate();
}

Color::Color(double val)
:r(val)
, g(val)
, b(val)
{
	//validate();
}


//Color::Color(int r, int g, int b)
//	:r(r / 255.)
//	,g(g / 255.)
//	,b(b / 255.)
//{
//	//validate();
//}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////

bool operator == (const Color& c1, const Color& c2)
{
	return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}

bool operator != (const Color& c1, const Color& c2)
{
	return !(c1 == c2);
}

Color operator + (const Color& c1, const Color& c2)
{
	Color res;
	res.r = c1.r + c2.r;
	res.g = c1.g + c2.g;
	res.b = c1.b + c2.b;

	//res.validate();

	return res;
}

Color& operator += (Color& c1, const Color& c2)
{
	return c1 = c1 + c2;
}

Color operator - (const Color& c1, const Color& c2)
{
	Color res;
	res.r = c1.r - c2.r;
	res.g = c1.g - c2.g;
	res.b = c1.b - c2.b;

	//res.validate();

	return res;
}

Color& operator -= (Color& c1, const Color& c2)
{
	return c1 = c1 - c2;
}

Color operator * (const Color& c1, const Color& c2)
{
	Color res;
	res.r = c1.r * c2.r;
	res.g = c1.g * c2.g;
	res.b = c1.b * c2.b;

	return res;
}

Color& operator *= (Color& c1, const Color& c2)
{
	return c1 = c1 * c2;
}

Color& operator *= (Color& c1, double val)
{
	return c1 = c1 * val;
}

Color operator *(double value, const Color& other)
{
	Color col;
	col.r = other.r * value;
	col.g = other.g * value;
	col.b = other.b * value;

	//col.validate();

	return col;
}

Color operator *(const Color& other, double value)
{
	//return other * value;

	Color col;
	col.r = other.r * value;
	col.g = other.g * value;
	col.b = other.b * value;

	//col.validate();

	return col;
}

Color operator /(double value, const Color& other)
{
	Color col;
	col.r = other.r / value;
	col.g = other.g / value;
	col.b = other.b / value;

	//col.validate();

	return col;
}

Color operator /(const Color& other, double value)
{
	Color col;
	col.r = other.r / value;
	col.g = other.g / value;
	col.b = other.b / value;

	//col.validate();

	return col;
}

Color& operator /= (Color& c1, double value)
{
	return c1 = c1 / value;
}

//Color operator *(Color& col, double value)
//{
//	return value * col;
//}

Color operator + (const Color& c1, double value)
{
	Color col;
	col.r = c1.r + value;
	col.g = c1.g + value;
	col.b = c1.b + value;

	return col;
}

Color& operator += (Color& c1, double value)
{
	return c1 = c1 + value;
}

Color operator / (const Color& col, const Color& other)
{
	Color res;
	res.r = col.r / other.r;
	res.g = col.g / other.g;
	res.b = col.b / other.b;

	return res;
}

Color operator /= (Color& col, const Color& other)
{
	return col = col / other;
}