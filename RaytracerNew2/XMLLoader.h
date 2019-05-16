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
	Texture::ptr handleTexture(Scene& scene, TiXmlElement* element);
	BSDF::ptr handleBSDF(Scene& scene, TiXmlElement* element);
	Medium::ptr handleMedium(Scene& scene, TiXmlElement* element);
	PhaseFunction::ptr handlePhase(Scene& scene, TiXmlElement* element);
	Volume::ptr handleVolume(Scene& scene, TiXmlElement* element);
	
	Transform handleRotationTransform(TiXmlElement* element);
	Transform handleTranslationTransform(TiXmlElement* element);
	Transform handleScaleTransform(TiXmlElement* element);
	Transform handleMatrixTransform(TiXmlElement* element);
	Transform handleLookAtTransform(TiXmlElement* element);

	std::vector<int> handleCameraDimensions(TiXmlElement* element);
	void handleCameraMedium(Scene& scene, TiXmlElement* element);
	//std::vector<real> handleObjectPosition(TiXmlElement* element);
	void handleProperty(Scene& scene, TiXmlElement* element, Parameters& params);
};

}