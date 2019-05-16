#pragma once
#include "core/Color.h"

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

class IORHelper
{
public:
	static void evalConductorIOR(const Parameters& params, Color& eta, Color& k);
	static void extractAndConvertSpectrumFromFile(const std::string& path, Color& values);

protected:
	static void initMap();

	static std::map<std::string, std::string> TechNameFromConvenientName;
};
