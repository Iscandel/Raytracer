#include "XMLLoader.h"

#include "BSSRDF.h"
#include "ObjectFactoryManager.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Tools.h"
#include "Geometry.h"
#include "Logger.h"

//#include "../../../../Dependencies/Headers/tinyxml/tinyxml.h"
#include "tinyxml.h"

#include <iostream>
#include <sstream>

namespace xml
{
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
XMLLoader::XMLLoader(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
XMLLoader::~XMLLoader(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool XMLLoader::loadScene(Scene& scene, const std::string& path)
{
	TiXmlDocument doc(path.c_str());
	if(!doc.LoadFile())
	{
		return false;
	}

	ILogger::log() << "Parsing " << path << "\n";
	::TiXmlHandle hdl(&doc);
	TiXmlElement* element = hdl.FirstChildElement().FirstChildElement().Element();

	if(!element)
	{
		ILogger::log(ILogger::ERRORS) << "XML loading : Node to reach doesn't exist.\n";
		return false;
	}

	//loop on all elements
	while (element)
	{
		std::string key = element->ValueStr();
		if(key == "camera")
		{
			ILogger::log(ILogger::ALL) << "Parsing camera...\n";
			handleCamera(scene, element);
			ILogger::log(ILogger::ALL) << "Camera parsed...\n";
		}
		else if(key == "lights")
		{
			ILogger::log(ILogger::ALL) << "Parsing lights...\n";
			handleLights(scene, element); 
			ILogger::log(ILogger::ALL) << "Lights parsed...\n";
		}
		else if(key == "parameters")
		{
			ILogger::log(ILogger::ALL) << "Parsing parameters...\n";
			handleParameters(scene, element); 
			ILogger::log(ILogger::ALL) << "Parameters parsed...\n";
		}
		else if (key == "sampler")
		{
			ILogger::log(ILogger::ALL) << "Parsing sampler...\n";
			handleSampler(scene, element);
			ILogger::log(ILogger::ALL) << "sampler parsed...\n";
		}
		else if (key == "integrator")
		{
			ILogger::log(ILogger::ALL) << "Parsing integrator...\n";
			handleIntegrator(scene, element);
			ILogger::log(ILogger::ALL) << "integrator parsed...\n";
		}
		else if (key == "medium")
		{
			ILogger::log(ILogger::ALL) << "Parsing camera medium...\n";
			handleCameraMedium(scene, element);
			ILogger::log(ILogger::ALL) << "camera medium parsed...\n";
		}
		else if(key == "objects")
		{
			ILogger::log(ILogger::ALL) << "Parsing objects...\n";
			TiXmlElement* childElement = element;
			//Shape
			childElement = childElement->FirstChildElement();
			while (childElement)
			{
				handleObject(scene, childElement);
				childElement = childElement->NextSiblingElement();
			}
			
			ILogger::log(ILogger::ALL) << "Objects parsed...\n";
		}
		//else
		//{
		//	handleProperty(element, params);
		//}

		element = element->NextSiblingElement();
	}

	return true;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleIntegrator(Scene& scene, TiXmlElement* element)
{
	Parameters params;
	std::string type = element->Attribute("type");
	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		handleProperty(scene, element, params);
	}

	ObjectFactory<Integrator> ::ptr factory = ObjectFactoryManager<Integrator>::getInstance()->getFactory(type);
	auto object = factory->create(params);
	scene.addIntegrator(object);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleCameraMedium(Scene& scene, TiXmlElement* element)
{
	Parameters params;
	std::string type = element->Attribute("type");
	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		handleProperty(scene, element, params);
	}

	ObjectFactory<Medium>::ptr factory = ObjectFactoryManager<Medium>::getInstance()->getFactory(type);
	auto object = factory->create(params);
	scene.setCameraMedium(object);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleSampler(Scene& scene, TiXmlElement* element)
{
	Parameters params;
	std::string type = element->Attribute("type");
	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		handleProperty(scene, element, params);
	}

	ObjectFactory<Sampler> ::ptr factory = ObjectFactoryManager<Sampler>::getInstance()->getFactory(type);
	auto object = factory->create(params);
	scene.setSampler(object);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleLights(Scene& scene, TiXmlElement* element)
{
	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		Parameters params;
		std::string type = element->Attribute("type");
		TiXmlElement* lightElement = element->FirstChildElement();
		for (lightElement; lightElement; lightElement = lightElement->NextSiblingElement())
		{
			handleProperty(scene, lightElement, params);
		}

		ObjectFactory<Light> ::ptr factory = ObjectFactoryManager<Light>::getInstance()->getFactory(type);
		auto object = factory->create(params);
		scene.addLight(object);

		if (type == "EnvironmentLight" || type == "DistantLight")
		{
			scene.setEnvironementLight(object);
		}
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleParameters(Scene& scene, TiXmlElement* element)
{
	int blockSizeX = 32;
	int blockSizeY = 32;

	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		//if(element->ValueStr() == "samplesPerPixel")
		//{
		//	int samplesPerPixel = tools::stringToNum<int>(element->GetText());
		//	scene.setSamplesPerPixel(samplesPerPixel);
		//}
		//else if(element->ValueStr() == "recursionLevel")
		//{
		//	int recursionLevel = tools::stringToNum<int>(element->GetText());
		//	scene.setRecursionLevel(recursionLevel);
		//}
		if(element->ValueStr() == "threadNumber")
		{
			int number = tools::stringToNum<int>(element->GetText());
			scene.setThreadNumber(number);
		}
		else if (element->ValueStr() == "fileName")
		{
			std::string fileName = element->GetText();
			scene.setFileName(fileName);
		}
		else if (element->ValueStr() == "blockSizeX")
		{
			blockSizeX = tools::stringToNum<int>(element->GetText());
		}
		else if (element->ValueStr() == "blockSizeY")
		{
			blockSizeY = tools::stringToNum<int>(element->GetText());
		}
		else if (element->ValueStr() == "showProgress")
		{
			int show = tools::stringToNum<int>(element->GetText());
			scene.setShowProgress((bool)show);
		}
		else
		{
			ILogger::log() << "Unknown parameter " << element->ValueStr() << ".\n";
		}
	}

	scene.setBlockSize(blockSizeX, blockSizeY);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleCamera(Scene& scene, TiXmlElement* element)
{
	//std::vector<real> translation;
	//std::vector<real> rot;
	Parameters params;
	Transform::ptr transform;
	std::vector<int> dimensions;
	ReconstructionFilter::ptr filter;

	std::string type = element->Attribute("type");
	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		if (element->ValueStr() == "transform")
		{
			transform = handleTransform(element);
		}
		else if (element->ValueStr() == "filter")
		{
			filter = handleReconstructionFilter(scene, element);
		}
		else
		{
			handleProperty(scene, element, params);
		}
	}

	auto factory = ObjectFactoryManager<Camera>::getInstance()->getFactory(type);
	Camera::ptr camera = factory->create(params);
	scene.setCamera(camera);

	//Transform transform;
	//Transform translate = Transform::translate(Point3d(translation[0], translation[1], translation[2]));
	//Transform rotateX = Transform::rotateX(rot[0]);
	//Transform rotateY = Transform::rotateY(rot[1]);
	//Transform rotateZ = Transform::rotateZ(rot[2]);

	////transform = Transform::translate(Point3d(translation[0], translation[1], translation[2])) *
	////	Transform::rotateX(rot[0]) * Transform::rotateY(rot[1]) * Transform::rotateZ(rot[2]);
	//transform = translate * rotateX * rotateY * rotateZ;

	//std::cout << transform << std::endl;
	scene.getCamera().setWorldTransform(transform);
	scene.getCamera().setReconstructionFilter(filter);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleObject(Scene& scene, TiXmlElement* element)
{
	std::vector<real> pos;
	BSDF::ptr bsdf;
	BSSRDF::ptr bssrdf;
	Light::ptr light;

	Parameters params;

	std::string type = element->Attribute("type");
	//Properties
	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		if (element->ValueStr() == "bsdf")
		{
			if (bsdf)
			{
				ILogger::log(ILogger::ERRORS) << "Only 1 BSDF allowed for " + type + ".\n";
			}
			else
			{
				std::string bsdfType = element->Attribute("type");
				TiXmlElement* bsdfElement = element->FirstChildElement();
				
				Parameters bsdfParams;
				for (bsdfElement; bsdfElement; bsdfElement = bsdfElement->NextSiblingElement())
				{
					handleProperty(scene, bsdfElement, bsdfParams);
				}
				ObjectFactory<BSDF>::ptr factory = ObjectFactoryManager<BSDF>::getInstance()->getFactory(bsdfType);
				bsdf = factory->create(bsdfParams);
				//Ou spécialiser bvh.setBSDF() et ne pas passer par params
				params.addBSDF("bsdf", bsdf);
			}
		}
		else if (element->ValueStr() == "bssrdf")
		{
			if (bssrdf)
			{
				ILogger::log(ILogger::ERRORS) << "Only 1 BSSRDF allowed for " + type + ".\n";
			}
			else
			{
				std::string bssrdfType = element->Attribute("type");
				TiXmlElement* bsdfElement = element->FirstChildElement();

				Parameters bsdfParams;
				for (bsdfElement; bsdfElement; bsdfElement = bsdfElement->NextSiblingElement())
				{
					handleProperty(scene, bsdfElement, bsdfParams);
				}
				ObjectFactory<BSSRDF>::ptr factory = ObjectFactoryManager<BSSRDF>::getInstance()->getFactory(bssrdfType);
				bssrdf = factory->create(bsdfParams);
				//Ou spécialiser bvh.setBSDF() et ne pas passer par params
				params.addBSSRDF("bssrdf", bssrdf);
			}
		}
		else if (element->ValueStr() == "light")
		{
			if (light)
			{
				ILogger::log(ILogger::ERRORS) << "Only 1 light allowed for " + type + ".\n";
			}
			else
			{
				std::string lightType = element->Attribute("type");
				TiXmlElement* lightElement = element->FirstChildElement();

				Parameters lightParams;
				for (lightElement; lightElement; lightElement = lightElement->NextSiblingElement())
				{
					handleProperty(scene, lightElement, lightParams);
				}
				ObjectFactory<Light>::ptr factory = ObjectFactoryManager<Light>::getInstance()->getFactory(lightType);
				light = factory->create(lightParams);
			
				params.addLight("light", light);
				scene.addLight(light);
			}
		}
		else
		{
			handleProperty(scene, element, params);
		}
	}

	ObjectFactory<IPrimitive>::ptr factory = ObjectFactoryManager<IPrimitive>::getInstance()->getFactory(type);
	auto object = factory->create(params);
	//if(bsdf)
	//	object->setBSDF(bsdf);

	scene.addPrimitive(object);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
ReconstructionFilter::ptr XMLLoader::handleReconstructionFilter(Scene& scene, TiXmlElement* element)
{
	Parameters params;
	std::string type = element->Attribute("type");

	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		handleProperty(scene, element, params);
	}

	auto factory = ObjectFactoryManager<ReconstructionFilter>::getInstance()->getFactory(type);
	ReconstructionFilter::ptr filter = factory->create(params);
	//scene.getCamera().setReconstructionFilter(filter);
	return filter;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform XMLLoader::handleRotationTransform(TiXmlElement* element)
{
	std::vector<real> rot(3, 0.);
	
	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();
		if(key == "x")
		{
			rot[0] = tools::stringToNum<real>(element->GetText());
		}
		else if(key == "y")
		{
			rot[1] = tools::stringToNum<real>(element->GetText());
		}
		else if(key == "z")
		{
			rot[2] = tools::stringToNum<real>(element->GetText());
		}
		else
		{
			ILogger::log() << "Unknown parameter " << key << ".\n";
		}
	}

	Transform rotateX = Transform::rotateX(rot[0]);
	Transform rotateY = Transform::rotateY(rot[1]);
	Transform rotateZ = Transform::rotateZ(rot[2]);
	return rotateX * rotateY * rotateZ ;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform XMLLoader::handleScaleTransform(TiXmlElement* element)
{
	std::vector<real> res(3, 1.);

	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();
		if (key == "x")
		{
			res[0] = tools::stringToNum<real>(element->GetText());
		}
		else if (key == "y")
		{
			res[1] = tools::stringToNum<real>(element->GetText());
		}
		else if (key == "z")
		{
			res[2] = tools::stringToNum<real>(element->GetText());
		}
		else
		{
			ILogger::log() << "Unknown parameter " << key << ".\n";
		}
	}

	return Transform::scale(Point3d(res[0], res[1], res[2]));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform XMLLoader::handleTranslationTransform(TiXmlElement* element)
{
	std::vector<real> res(3, 0.);
	
	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();
		if(key == "x")
		{
			res[0] = tools::stringToNum<real>(element->GetText());
		}
		else if(key == "y")
		{
			res[1] = tools::stringToNum<real>(element->GetText());
		}
		else if(key == "z")
		{
			res[2] = tools::stringToNum<real>(element->GetText());
		}
		else
		{
			ILogger::log() << "Unknown parameter " << key << ".\n";
		}
	}

	return Transform::translate(Point3d(res[0], res[1], res[2]));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform XMLLoader::handleMatrixTransform(TiXmlElement* element)
{
	std::vector<real> res;

	std::string values = element->Attribute("value");

	char delim = ' ';
	std::stringstream ss(values);
	std::string item;
	while (std::getline(ss, item, delim)) {
		res.push_back(tools::stringToNum<real>(item));
	}

	return Transform(res.data());
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Texture::ptr XMLLoader::handleTexture(Scene& scene, TiXmlElement* element)
{
	Parameters params;
	std::string type = element->Attribute("type");

	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		handleProperty(scene, element, params);
	}

	auto factory = ObjectFactoryManager<Texture>::getInstance()->getFactory(type);
	Texture::ptr texture = factory->create(params);
	
	return texture;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
BSDF::ptr XMLLoader::handleBSDF(Scene& scene, TiXmlElement* element)
{
	std::string bsdfType = element->Attribute("type");
	TiXmlElement* bsdfElement = element->FirstChildElement();

	Parameters bsdfParams;
	for (bsdfElement; bsdfElement; bsdfElement = bsdfElement->NextSiblingElement())
	{
		handleProperty(scene, bsdfElement, bsdfParams);
	}
	ObjectFactory<BSDF>::ptr factory = ObjectFactoryManager<BSDF>::getInstance()->getFactory(bsdfType);
	BSDF::ptr bsdf = factory->create(bsdfParams);

	return bsdf;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Medium::ptr XMLLoader::handleMedium(Scene& scene, TiXmlElement* element)
{
	std::string mediumType = element->Attribute("type");
	TiXmlElement* mediumElement = element->FirstChildElement();

	Parameters mediumParams;
	for (mediumElement; mediumElement; mediumElement = mediumElement->NextSiblingElement())
	{
		handleProperty(scene, mediumElement, mediumParams);
	}
	ObjectFactory<Medium>::ptr factory = ObjectFactoryManager<Medium>::getInstance()->getFactory(mediumType);
	Medium::ptr medium = factory->create(mediumParams);

	if (medium->isEmissive())
	{
		scene.addLight(medium);
	}

	return medium;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
PhaseFunction::ptr XMLLoader::handlePhase(Scene& scene, TiXmlElement* element)
{
	Parameters params;
	std::string type = element->Attribute("type");

	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		handleProperty(scene, element, params);
	}

	auto factory = ObjectFactoryManager<PhaseFunction>::getInstance()->getFactory(type);
	PhaseFunction::ptr phase = factory->create(params);

	return phase;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Volume::ptr XMLLoader::handleVolume(Scene& scene, TiXmlElement* element)
{
	Parameters params;
	std::string type = element->Attribute("type");

	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		handleProperty(scene, element, params);
	}

	auto factory = ObjectFactoryManager<Volume>::getInstance()->getFactory(type);
	Volume::ptr volume = factory->create(params);

	return volume;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform XMLLoader::handleLookAtTransform(TiXmlElement* element)
{
	Point3d origin, lookAt;
	Vector3d up;

	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();

		if (element->ValueStr() == "origin")
		{
			origin = tools::stringToVector<real>(element->GetText());
		}
		else if (element->ValueStr() == "lookAt")
		{
			lookAt = tools::stringToVector<real>(element->GetText());
		}
		else if (element->ValueStr() == "up")
		{
			up = tools::stringToVector<real>(element->GetText());
		}
		else
		{
			ILogger::log() << "Unknown parameter " << element->ValueStr() << ".\n";
		}
	}

	return Transform::fromLookAt(origin, lookAt, up);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleProperty(Scene& scene, TiXmlElement* element, Parameters& params)
{
	//TiXmlAttribute* att = element->FirstAttribute();
	const std::string* attName = element->Attribute(std::string("name"));
	const std::string* attValue = element->Attribute(std::string("value"));
	if (attName == nullptr)
	{
		ILogger::log() << "No name attribut specified for " << 
			element->ValueStr() << " property.\n";
		return;
	}

	if (element->ValueStr() == "bool")
	{
		params.addBool(*attName, tools::stringToNum<bool>(*attValue));
	}
	else if (element->ValueStr() == "integer")
	{
		params.addInt(*attName, tools::stringToNum<int>(*attValue));
	}
	else if (element->ValueStr() == "double" || element->ValueStr() == "real") //compatibility purpose
	{
		params.addReal(*attName, tools::stringToNum<real>(*attValue));
	}
	else if (element->ValueStr() == "string")
	{
		params.addString(*attName, *attValue);
	}
	else if (element->ValueStr() == "point")
	{
		params.addPoint(*attName, tools::stringToPoint<real>(*attValue));
	}
	else if (element->ValueStr() == "color")
	{
		params.addColor(*attName, tools::stringToColor<real>(*attValue, 1));
	}
	else if (element->ValueStr() == "spectrum")
	{
		bool isPath = false;
		//bug with long path on VS, so try-catch
		try {
			isPath = std::experimental::filesystem::exists(*attValue);
		}
		catch (const std::exception&) {
		}

		if(isPath)
			params.addColor(*attName, tools::stringToColor<real>(*attValue, 2));
		else
			params.addColor(*attName, tools::stringToColor<real>(*attValue, 0));
		
	}
	else if (element->ValueStr() == "vector")
	{
		params.addVector(*attName, tools::stringToVector<real>(*attValue));
	}
	else if (element->ValueStr() == "transform")
	{
		Transform::ptr transform = handleTransform(element);
		params.addTransform(*attName, transform);
	}
	else if (element->ValueStr() == "texture")
	{
		Texture::ptr texture = handleTexture(scene, element);
		params.addTexture(*attName, texture);
	}
	else if (element->ValueStr() == "bsdf")
	{
		BSDF::ptr bsdf = handleBSDF(scene, element);
		params.addBSDF(*attName, bsdf);
	}
	else if (element->ValueStr() == "medium")
	{
		Medium::ptr medium = handleMedium(scene, element);
		params.addMedium(*attName, medium);
		if (medium->isEmissive())
		{
			scene.addLight(medium);
		}
	}
	else if (element->ValueStr() == "phase")
	{
		PhaseFunction::ptr phase = handlePhase(scene, element);
		params.addPhaseFunction(*attName, phase);
	}
	else if (element->ValueStr() == "volume")
	{
		Volume::ptr volume = handleVolume(scene, element);
		params.addVolume(*attName, volume);
	}
	else
	{
		ILogger::log() << "Unknown property " << *attName << "\n";
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::vector<int> XMLLoader::handleCameraDimensions(TiXmlElement* element)
{
	std::vector<int> res(2, 0);
	
	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();
		if(key == "sizeX")
		{
			res[0] = tools::stringToNum<int>(element->GetText());
		}
		else if(key == "sizeY")
		{
			res[1] = tools::stringToNum<int>(element->GetText());
		}
		else
		{
			ILogger::log() << "Unknown parameter " << key << ".\n";
		}
	}

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::ptr XMLLoader::handleTransform(TiXmlElement* element)
{
	Transform::ptr transform(new Transform);
	std::vector<real> translateVect(3, 0.);
	std::vector<real> rot(3, 0.);
	std::vector<real> scaleVect(3, 1.);

	element = element->FirstChildElement();

	for (element; element; element = element->NextSiblingElement())
	{
		if (element->ValueStr() == "rotation")
		{
			Transform matrix = handleRotationTransform(element);
			//Transform rotateX = Transform::rotateX(rot[0]);
			//Transform rotateY = Transform::rotateY(rot[1]);
			//Transform rotateZ = Transform::rotateZ(rot[2]);
			//(*transform) = rotateX * rotateY * rotateZ * (*transform);
			(*transform) = matrix * (*transform);
		}
		else if (element->ValueStr() == "translation")
		{
			Transform translate = handleTranslationTransform(element);
			//Transform translate = Transform::translate(Point3d(translateVect[0], translateVect[1], translateVect[2]));
			(*transform) = translate * (*transform);
		}
		else if (element->ValueStr() == "scale")
		{
			Transform scale = handleScaleTransform(element);
			//Transform scale = Transform::scale(Point3d(scaleVect[0], scaleVect[1], scaleVect[2]));
			(*transform) = scale * (*transform);
		}
		else if (element->ValueStr() == "matrix")
		{
			Transform matrix = handleMatrixTransform(element);
			//Transform matrix = Transform(mat.data());
			(*transform) = matrix * (*transform);
		}
		else if (element->ValueStr() == "lookAt")
		{
			Transform mat = handleLookAtTransform(element);
			(*transform) = mat * (*transform);
		}
		else
		{
			ILogger::log() << "Unknown parameter " << element->ValueStr() << ".\n";
		}
	}

	//Transform translate = Transform::translate(Point3d(translateVect[0], translateVect[1], translateVect[2]));
	//Transform rotateX = Transform::rotateX(rot[0]);
	//Transform rotateY = Transform::rotateY(rot[1]);
	//Transform rotateZ = Transform::rotateZ(rot[2]);
	//Transform scale = Transform::scale(Point3d(scaleVect[0], scaleVect[1], scaleVect[2]));

	//transform = Transform::ptr(new Transform(scale * translate * rotateX * rotateY * rotateZ));
	
	//don't use make_shared with types that need to be aligned
	//transform = std::make_shared<Transform>(new Transform(scale * translate * rotateX * rotateY * rotateZ));

	return transform;
}


} //end namespace xml
