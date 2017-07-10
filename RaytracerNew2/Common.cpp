#include "Common.h"

#include "Color.h"
#include "Parameters.h"

std::map<std::string, std::string> IORHelper::TechNameFromConvenientName;

void IORHelper::initMap()
{
	TechNameFromConvenientName["gold"] = "Au";
	TechNameFromConvenientName["silver"] = "Ag";
	TechNameFromConvenientName["aluminium"] = "Al";
	TechNameFromConvenientName["copper"] = "Cu";
	TechNameFromConvenientName["chromium"] = "Cr";
	TechNameFromConvenientName["iron"] = "Fe";
	TechNameFromConvenientName["nickel"] = "Ni_palik";
	TechNameFromConvenientName["tin"] = "SnTe_palik";
}

void IORHelper::evalConductorIOR(const Parameters & params, Color & eta, Color & k)
{
	static bool init = false;

	if (!init)
	{
		initMap();
		init = true;
	}

	std::string materialName = params.getString("material", "");
	std::map<std::string, std::string>::iterator it = TechNameFromConvenientName.find(materialName);

	if (it == TechNameFromConvenientName.end() && materialName != "")
		ILogger::log() << "material " << materialName << " not found.\n";
	if (materialName == "" || it == TechNameFromConvenientName.end())
	{
		eta = params.getColor("eta", Color(0.));
		k = params.getColor("absorption", Color(0.));
	}
	else
	{
		extractAndConvertSpectrumFromFile("./data/IOR/" + it->second + ".eta.spd", eta);
		extractAndConvertSpectrumFromFile("./data/IOR/" + it->second + ".k.spd", k);
	}
}

void IORHelper::extractAndConvertSpectrumFromFile(const std::string & path, Color & values)
{
	std::ifstream file(path);

	if (!file)
		ILogger::log(ILogger::ERRORS) << "Spectrum file " + path + " does not exist.\n";

	std::vector<double> lambda;
	std::vector<double> samples;
	std::string line;
	while (std::getline(file, line))
	{
		if (line[0] == '#')
			continue;
		std::string tmp1, tmp2;
		std::istringstream sLine(line);
		sLine >> tmp1 >> tmp2;
		lambda.push_back(tools::stringToNum<double>(tmp1));
		samples.push_back(tools::stringToNum<double>(tmp2));
	}
	ColorSpaceConversion etaSpectrum(lambda, samples);
	double rgb[3];
	etaSpectrum.toRGB(rgb);
	values = Color(rgb[0], rgb[1], rgb[2]);
	file.close();	
}
