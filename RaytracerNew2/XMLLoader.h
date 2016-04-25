#pragma once
#include "Scene.h"
#include "Parameters.h"

#include <vector>

//#ifndef TIXML_USE_STL
//	#define TIXML_USE_STL
//#endif

class TiXmlElement;

namespace xml
{
class XMLLoader
{
public:
	XMLLoader(void);
	~XMLLoader(void);

	bool loadScene(Scene& scene, const std::string& path);

	void handleParameters(Scene& scene, TiXmlElement* element);
	void handleCamera(Scene& scene, TiXmlElement* element);
	void handleObject(Scene& scene, TiXmlElement* element);
	void handleLights(Scene& scene, TiXmlElement* element);
	void handleIntegrator(Scene& scene, TiXmlElement* element);
	void handleSampler(Scene& scene, TiXmlElement* element);
	ReconstructionFilter::ptr handleReconstructionFilter(Scene& scene, TiXmlElement* element);
	Transform::ptr handleTransform(TiXmlElement* element);
	
	std::vector<double>handleRotationTransform(TiXmlElement* element);
	std::vector<double> handleTranslationTransform(TiXmlElement* element);
	std::vector<double> handleScaleTransform(TiXmlElement* element);

	std::vector<int> handleCameraDimensions(TiXmlElement* element);
	std::vector<double> handleObjectPosition(TiXmlElement* element);
	void handleProperty(TiXmlElement* element, Parameters& params);
};

}