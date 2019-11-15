#ifndef H__TOOLS_230520152033__H
#define H__TOOLS_230520152033__H

#include "Array2D.h"
#include "core/Color.h"
#include "core/Geometry.h"
#include <filesystem>
#include <regex>
#include <string>

namespace tools
{
//#ifdef DOUBLE_PRECISION
//	const real PI = 3.14159265358979323846;
//	const real INV_PI = 0.3183098861837906715377;
//	const real INV_FOUR_PI = 0.07957747154594766788450883936693;
//	const real MAX_REAL = 10e9;
//	const real EPSILON = 1e-4;
//#else
//	const real PI = 3.14159265358979323846f;
//	const real INV_PI = 0.3183098861837906715377f;
//	const real INV_FOUR_PI = 0.07957747154594766788450883936693f;
//	const real MAX_REAL = 10e7f;
//	const real EPSILON = 1e-4f;
//#endif
//
//	inline real toRadian(real degres)
//	{
//		return (degres * tools::PI) / 180.f;
//	}
//
//	inline real toDegre(real radian)
//	{
//		return (radian * 180.f ) / tools::PI;
//	}

	inline std::vector<std::string> regexSplit(const std::string& s, const std::string& regex)
	{
		std::vector<std::string> res;
		std::regex words_regex(regex);//"[^\\s.,:;!?]+");
		auto words_begin = std::sregex_iterator(s.begin(), s.end(), words_regex);
		auto words_end = std::sregex_iterator();

		for (std::sregex_iterator i = words_begin; i != words_end; ++i)
			res.push_back((*i).str());
		return res;
	}

	//	return res;
	//	std::vector<std::string> res;

	//	std::string tmp = s;
	//	size_t pos = 0;
	//	std::string token;
	//	while ((pos = s.find(delimiter)) != std::string::npos) {
	//		res.push_back(tmp.substr(0, pos));
	//		tmp.erase(0, pos + delimiter.length());
	//	}
	//	res.push_back(tmp);
	//	return res;
	//}

	template<class T>
	std::string numToString(const T& val)
	{
		//Create an output stream
		std::ostringstream oss;
		// write a number
		oss << val;
		// get the string
		return oss.str();
	}

///////////////////////////////////////////////////////////////////////////
/// Converts a number into a wide string. If the conversion fails, the result
/// will be an empty string.
///
/// \param val: value to convert
///
/// \return A wide string representation of the value
///////////////////////////////////////////////////////////////////////////
template<class T>
std::wstring numToWString(const T& num)
{
	// create an output stream
	std::wostringstream oss;
	// write a number in the stream
	oss << num;
	// get the string
	return oss.str();
}

///////////////////////////////////////////////////////////////////////////
/// Converts a string into a number. If the conversion fails, the result
/// could take any value.
///
/// \param val: value to convert
///
/// \return A string representation of the value
///////////////////////////////////////////////////////////////////////////
template<class T>
T stringToNum(const std::string& string)
{
	std::istringstream iss(string);
	T number;
	iss >> number; 
	return number;
}

///////////////////////////////////////////////////////////////////////////
/// Converts a string into a boolean. If the conversion fails, the result
/// could take any value.
///
/// \param val: value to convert
///
/// \return A string representation of the value
///////////////////////////////////////////////////////////////////////////
template<>
inline bool stringToNum(const std::string& string)
{
	std::string sCopy = string;
	std::transform(sCopy.begin(), sCopy.end(), sCopy.begin(), ::tolower);
	if (string == "1" || sCopy == "true")
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////
/// Converts a string into a Point3. If the conversion fails, the result
/// could take any value.
///
/// \param string: value to convert
///
/// \return A string representation of the value
///////////////////////////////////////////////////////////////////////////
template<class T>
Point3<T> stringToPoint(const std::string& string)
{
	std::istringstream iss(string);
	T x, y, z;
	iss >> x >> y >> z;

	Point3<T> point(x, y, z);

	return point;
}

///////////////////////////////////////////////////////////////////////////
/// Converts a string into a color. If the conversion fails, the result
/// could take any value.
///
/// \param string: value to convert
/// \param int type: 0 for spectrum, 1 for rgb, 2 for filename
///
/// \return A string representation of the value
///////////////////////////////////////////////////////////////////////////

template<class T>
Color stringToColor(const std::string& string, int type)
{
	//Split a string considering :
	//-rgb strings with blank space or comma delim : value, value value
	//spectra with blank space or comma delim : lambda:value, lambda :value lambda :   value
	//Firsts [], include evertyhing except spaces and ,
	//Next, include the possible blank spaces in the tokens
	//Next, include ":" character
	//Next, include possible blank spaces
	//Next, include everything, except blank spaces or commas
	//Finally, next block is for rgb color :  include everything except blank spaces and commas
	//NB : the resulting string has all the user inputs mistakes (if any). It is intended for
	//error handling
	std::vector<std::string> splitted = regexSplit(string, "([^\\s,]+\\s*:\\s*[^\\s,]+)|([^\\s,]+)");
	
	try
	{
		if (type == 0)
		{
			std::vector<T> lambda;
			std::vector<T> values;
			for (int i = 0; i < splitted.size(); i++)
			{
				std::vector<std::string> splitted2 = regexSplit(splitted[i], "[^:]+");
				if (splitted2.size() == 2)
				{
					lambda.push_back(stringToNum<real>(splitted2[0]));
					values.push_back(stringToNum<real>(splitted2[1]));
				}
				else
				{
					throw std::runtime_error(std::string("Spectrum:" + string + " wrong size"));
					//ILogger::log(ILogger::ERRORS) << "Spectrum:" + string + " wrong size";
					//return Color();
				}
			}

			Color res;
			res.setFromSampledSpectrum(lambda, values);
			return res;
		}
		else if (type == 2)
		{
			Color col;
			IORHelper::extractAndConvertSpectrumFromFile(string, col);
			return col;
		}
		else //rgb
		{
			for (const std::string& s : splitted) {
				if (s.find(":") != std::string::npos) {
					throw std::runtime_error(std::string("RGB string:" + string + " should not contain \":\" char\n"));
					//ILogger::log(ILogger::ERRORS) << "RGB string:" + string + " should not contain \":\" char\n";
					//return Color();
				}
			}
			if (splitted.size() != 3)
			{
				throw std::runtime_error(std::string("RGB string " + string + " is of wrong size\n"));
				//ILogger::log(ILogger::ERRORS) << "RGB string " + string + " is of wrong size\n";
				//return Color();
			}

			return Color::fromRGB(stringToNum<T>(splitted[0]), 
								  stringToNum<T>(splitted[1]), 
								  stringToNum<T>(splitted[2]));
		}
	} 
	catch (const std::exception& ex)
	{
		ILogger::log(ILogger::ERRORS) << ex.what();
		return Color();
	}

	//T x, y, z;
	//iss >> x >> y >> z;

	//Color col(x, y, z);
	//return col;
}

///////////////////////////////////////////////////////////////////////////
/// Converts a string into a Vector3. If the conversion fails, the result
/// could take any value.
///
/// \param string: value to convert
///
/// \return A string representation of the value
///////////////////////////////////////////////////////////////////////////
template<class T>
Vect3<T> stringToVector(const std::string& string)
{
	std::istringstream iss(string);
	T x, y, z;
	iss >> x >> y >> z;

	Vect3<T> point(x, y, z);
	return point;
}

//template<class T>
//T thresholding(T val, T min, T max)
//{
//	return val < min ? min : val  > max ? max : val;
//}
//
//template<class U = int, class T>
//U sign(T val)
//{
//	if (val >= (T)0)
//		return (U)1;
//	return (U)-1;
//}
//
//template<class T>
//inline real interp1(real advance, T start, T end)
//{
//	return (1 - advance) * start + advance * end;
//}
//
//inline Color interp2(const Point2d& xy, const Array2D<Color>& array)
//{
//	real advanceX = xy.x() - (int)xy.x();
//	real advanceY = xy.y() - (int)xy.y();
//	int xMin = tools::thresholding((int)xy.x(), 0, (int)array.getWidth() - 2);
//	int yMin = tools::thresholding((int)xy.y(), 0, (int)array.getHeight() - 2);
//	int xMax = xMin + 1 < (int)array.getWidth() ? xMin + 1 : xMin;
//	xMax = thresholding(xMax, 0, (int)array.getWidth() - 1);
//	int yMax = yMin + 1 < (int)array.getHeight() ? yMin + 1 : yMin;
//	yMax = thresholding(yMax, 0, (int)array.getHeight() - 1);
//
//	return (1 - advanceX) * (1 - advanceY) * array(xMin, yMin) +
//		(1 - advanceX) * advanceY * array(xMin, yMax) +
//		advanceX * (1 - advanceY) * array(xMax, yMin) +
//		advanceX * advanceY * array(xMax, yMax);
//}

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
//inline real random(real min, real max) 
//{
//	return (rand() / (real) RAND_MAX) * (max - min) + min;
//}

inline std::string lowerCase(const std::string& s)
{
	std::string res = s;
	std::transform(s.begin(), s.end(), res.begin(), ::tolower);

	return res;
}

inline std::string trim(const std::string& s)
{
	std::size_t first = s.find_first_not_of(" \t\r\n");
	std::size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(first == std::string::npos ? 0 : first, end == std::string::npos ? s.size() : end + 1);
}

inline std::string getExtension(const std::string& s)
{
	size_t index = s.find_last_of('.');
	if (index != std::string::npos)
	{
		return s.substr(index);
	}

	return "";
}

}

#endif