#pragma once
//#include "Color.h"

#include <map>

class Parameters;
class Color;

///Measure performed on a BSDF or during light sampling :
///solid angle or delta distribution
enum Measure
{
	DISCRETE,
	SOLID_ANGLE
};

class IORHelper
{
public:
	static void evalConductorIOR(const Parameters& params, Color& eta, Color& k);

protected:
	static void initMap();

	static void extractAndConvertSpectrumFromFile(const std::string& path, Color& values);

	static std::map<std::string, std::string> TechNameFromConvenientName;
};
