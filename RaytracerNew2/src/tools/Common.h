#pragma once
#include "core/Color.h"
#include "tools/Filesystem.h"

#include <map>

class Parameters;
//class Color;

///Measure performed on a BSDF or during light sampling :
///solid angle or delta distribution
enum Measure
{
	DISCRETE,
	SOLID_ANGLE
};

int getCoreNumber();

struct Ci_less
{
	// case-independent (ci) compare_less binary function
	struct nocase_compare
	{
		bool operator() (const unsigned char& c1, const unsigned char& c2) const {
			return tolower(c1) < tolower(c2);
		}
	};
	bool operator() (const std::string & s1, const std::string & s2) const {
		return std::lexicographical_compare
		(s1.begin(), s1.end(),   // source range
			s2.begin(), s2.end(),   // dest range
			nocase_compare());  // comparison
	}
};

class IORHelper
{
public:
	static void evalConductorIOR(const Parameters& params, Color& eta, Color& k);
	static void extractAndConvertSpectrumFromFile(const std::string& path, Color& values);

protected:
	static void initMap();

	static std::map<std::string, std::string> TechNameFromConvenientName;
};


Filesystem& getGlobalFileSystem();