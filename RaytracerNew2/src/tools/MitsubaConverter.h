#pragma once

#include "tools/Common.h"
#include "core/Parameters.h"

#include "ext/tinyxml/tinyxml.h"

#include <fstream>
#include <map>
#include <string>
#include <vector>

class MitsubaConverter
{
public:
	//NB: case independant maps
	typedef std::map < std::string, std::string, Ci_less> MapDictionary;
	typedef std::map < std::string, std::map < std::string, std::string, Ci_less >, Ci_less> MapDictionaryByElement;

public:
	struct Bloc {
		std::string elemType;
		std::string type;
		std::string name;
		std::string id;
		std::string value;
		std::string textValue;
		//Parameters params;
		std::vector<std::shared_ptr<Bloc>> childs;

		void addChild(const std::shared_ptr<Bloc>& bloc) { childs.push_back(bloc); }
	};

public:
	MitsubaConverter();
	~MitsubaConverter();

	bool convert(const std::string& pathFrom, const std::string& pathDest);

	void handleElement(Bloc& vec, TiXmlElement* element);

	void handleProperty(TiXmlElement* element, Bloc& bloc);

	void handleAndCorrectTransform(Bloc& bloc, TiXmlElement* element);

	

protected:
	void writeXML(const std::string& path);

	void addAttribute(std::ofstream& file, const std::string& name, const std::string& value);
	
	void addElement(std::ofstream& file, const std::string& element, int nIndent = 0);

	void addEndElement(std::ofstream& file, const std::string& element, int nIndent = 0);

	void addElementWithAtt(std::ofstream& file, const std::string& element, int nIndent = 0);

	void addEndElementWithAtt(std::ofstream& file);

	void addClosing(std::ofstream& file);

	void writeHeader(std::ofstream& file);

	void writeBloc(std::ofstream& file, Bloc& bloc, int nIndent = 0);

	std::string getValueWithName(const Bloc& bloc, const std::string& value);
	std::vector<std::vector<std::shared_ptr<Bloc>>::iterator> getSubBlocFromElemType(Bloc& bloc, const std::string& elemType);

	std::string findFromDictionary(MapDictionary& map,
		const std::string& key, const std::string msg = "");

	void handleAndCorrectSensor(Bloc& bloc);
	void handleAndCorrectBsdf(Bloc& bloc);
	void handleAndCorrectTextures(Bloc& bloc);

	std::string lowerCase(const std::string& s)
	{
		std::string res = s;
		std::transform(s.begin(), s.end(), res.begin(), ::tolower);

		return res;
	}

protected:
	Bloc myIntegrator;
	Bloc mySensor;
	Bloc mySampler;
	//Bloc myFilter;
	std::vector<Bloc> myBsdfs;
	std::vector<Bloc> myLights;
	std::vector<Bloc> myTextures;
	std::vector<Bloc> myShapes;

	std::map<std::string, Bloc> myBlocsById;

	std::string SCENE;
	std::string CAMERA;
	std::string TYPE;
	std::string TRANSFORM;
	std::string LOOK_AT;
	std::string ORIGIN;
	std::string UP;
	std::string DOUBLE;
	std::string FOV;
	std::string SIZE_X;
	std::string SIZE_Y;
	std::string FILTER;
	std::string INTEGRATOR;
	std::string OBJECTS;
	std::string LIGHTS;
	std::string NAME;
	std::string VALUE;
	std::string STRING;
	std::string REAL;
	std::string INTEGER;
	std::string SAMPLE_NUMBER;
	
	std::string _BSDF;
	std::string _DIFFUSE;
	std::string _ROUGH_PLASTIC;
	
	//Case independant maps
	MapDictionary myDictionary;
	MapDictionaryByElement myDictionaryByBsdf;
	MapDictionaryByElement myDictionaryBySensor;
	MapDictionaryByElement myDictionaryByShape;
	MapDictionaryByElement myDictionaryByLight;
	MapDictionaryByElement myDictionaryByTexture;
};

