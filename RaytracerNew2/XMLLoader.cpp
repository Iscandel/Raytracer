#include "XMLLoader.h"

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
		handleProperty(element, params);
	}

	ObjectFactory<Integrator> ::ptr factory = ObjectFactoryManager<Integrator>::getInstance()->getFactory(type);
	auto object = factory->create(params);
	scene.addIntegrator(object);
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
		handleProperty(element, params);
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
			handleProperty(lightElement, params);
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
	int blockSizeX;
	int blockSizeY;

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
	}

	scene.setBlockSize(blockSizeX, blockSizeY);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleCamera(Scene& scene, TiXmlElement* element)
{
	//std::vector<double> translation;
	//std::vector<double> rot;
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
			handleProperty(element, params);
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
	double radius = 0.;
	std::vector<double> pos;
	BSDF::ptr bsdf;
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
					handleProperty(bsdfElement, bsdfParams);
				}
				ObjectFactory<BSDF>::ptr factory = ObjectFactoryManager<BSDF>::getInstance()->getFactory(bsdfType);
				bsdf = factory->create(bsdfParams);
				//Ou spécialiser bvh.setBSDF() et ne pas passer par params
				params.addBSDF("bsdf", bsdf);
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
					handleProperty(lightElement, lightParams);
				}
				ObjectFactory<Light>::ptr factory = ObjectFactoryManager<Light>::getInstance()->getFactory(lightType);
				light = factory->create(lightParams);
			
				params.addLight("light", light);
				scene.addLight(light);
			}
		}
		else
		{
			handleProperty(element, params);
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
		handleProperty(element, params);
	}

	auto factory = ObjectFactoryManager<ReconstructionFilter>::getInstance()->getFactory(type);
	ReconstructionFilter::ptr filter = factory->create(params);
	//scene.getCamera().setReconstructionFilter(filter);
	return filter;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::vector<double> XMLLoader::handleRotationTransform(TiXmlElement* element)
{
	std::vector<double> res(3, 0.);
	
	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();
		if(key == "rx")
		{
			res[0] = tools::stringToNum<double>(element->GetText());
		}
		else if(key == "ry")
		{
			res[1] = tools::stringToNum<double>(element->GetText());
		}
		else if(key == "rz")
		{
			res[2] = tools::stringToNum<double>(element->GetText());
		}
	}

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::vector<double> XMLLoader::handleScaleTransform(TiXmlElement* element)
{
	std::vector<double> res(3, 1.);

	element = element->FirstChildElement();
	for (element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();
		if (key == "sx")
		{
			res[0] = tools::stringToNum<double>(element->GetText());
		}
		else if (key == "sy")
		{
			res[1] = tools::stringToNum<double>(element->GetText());
		}
		else if (key == "sz")
		{
			res[2] = tools::stringToNum<double>(element->GetText());
		}
	}

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::vector<double> XMLLoader::handleTranslationTransform(TiXmlElement* element)
{
	std::vector<double> res(3, 0.);
	
	element = element->FirstChildElement();
	for(element; element; element = element->NextSiblingElement())
	{
		std::string key = element->ValueStr();
		if(key == "tx")
		{
			res[0] = tools::stringToNum<double>(element->GetText());
		}
		else if(key == "ty")
		{
			res[1] = tools::stringToNum<double>(element->GetText());
		}
		else if(key == "tz")
		{
			res[2] = tools::stringToNum<double>(element->GetText());
		}
	}

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void XMLLoader::handleProperty(TiXmlElement* element, Parameters& params)
{
	//TiXmlAttribute* att = element->FirstAttribute();
	const std::string* attName = element->Attribute(std::string("name"));
	const std::string* attValue = element->Attribute(std::string("value"));
	if (attName == nullptr)
		return;

	if (element->ValueStr() == "bool")
	{
		params.addBool(*attName, tools::stringToNum<bool>(*attValue));
	}
	else if (element->ValueStr() == "integer")
	{
		params.addInt(*attName, tools::stringToNum<int>(*attValue));
	}
	else if (element->ValueStr() == "double")
	{
		params.addDouble(*attName, tools::stringToNum<double>(*attValue));
	}
	else if (element->ValueStr() == "string")
	{
		params.addString(*attName, *attValue);
	}
	else if (element->ValueStr() == "point")
	{
		params.addPoint(*attName, tools::stringToPoint<double>(*attValue));
	}
	else if (element->ValueStr() == "color")
	{
		params.addColor(*attName, tools::stringToColor<double>(*attValue));
	}
	else if (element->ValueStr() == "vector")
	{
		params.addVector(*attName, tools::stringToVector<double>(*attValue));
	}
	else if (element->ValueStr() == "transform")
	{
		Transform::ptr transform = handleTransform(element);
		params.addTransform(*attName, transform);
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
	}

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//std::vector<double> XMLLoader::handleObjectPosition(TiXmlElement* element)
//{
//	std::vector<double> res(3, 0.);
//	
//	element = element->FirstChildElement();
//	for(element; element; element = element->NextSiblingElement())
//	{
//		std::string key = element->ValueStr();
//		if(key == "x")
//		{
//			res[0] = tools::stringToNum<double>(element->GetText());
//		}
//		else if(key == "y")
//		{
//			res[1] = tools::stringToNum<double>(element->GetText());
//		}
//		else if(key == "z")
//		{
//			res[2] = tools::stringToNum<double>(element->GetText());
//		}
//	}
//
//	return res;
//}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::ptr XMLLoader::handleTransform(TiXmlElement* element)
{
	Transform::ptr transform(new Transform);
	std::vector<double> translateVect(3, 0.);
	std::vector<double> rot(3, 0.);
	std::vector<double> scaleVect(3, 1.);

	const std::string* type = element->Attribute(std::string("type"));
	element = element->FirstChildElement();
	//Look at transform
	if (type && (*type) == "lookAt")
	{
		Vector3d origin, lookAt, up;

		for (element; element; element = element->NextSiblingElement())
		{
			if (element->ValueStr() == "origin")
			{
				origin = tools::stringToVector<double>(element->GetText());
			}
			else if (element->ValueStr() == "lookAt")
			{
				lookAt = tools::stringToVector<double>(element->GetText());
			}
			else if(element->ValueStr() == "up")
			{
				up = tools::stringToVector<double>(element->GetText());
			}
			else if (element->ValueStr() == "scale")
			{ 
				//warning : camera transform shoudn't have a scale factor. It is only used to invert axis direction
				scaleVect = handleScaleTransform(element);
			}
		}

		transform = Transform::ptr(new Transform(Transform::fromLookAt(origin, lookAt, up)));
		Transform scale = Transform::scale(Point3d(scaleVect[0], scaleVect[1], scaleVect[2]));
		*transform = scale * (*transform);
	}
	else
	{
		for (element; element; element = element->NextSiblingElement())
		{
			if (element->ValueStr() == "rotation")
			{
				rot = handleRotationTransform(element);
				Transform rotateX = Transform::rotateX(rot[0]);
				Transform rotateY = Transform::rotateY(rot[1]);
				Transform rotateZ = Transform::rotateZ(rot[2]);
				(*transform) = rotateX * rotateY * rotateZ * (*transform);
			}
			else if (element->ValueStr() == "translation")
			{
				translateVect = handleTranslationTransform(element);
				Transform translate = Transform::translate(Point3d(translateVect[0], translateVect[1], translateVect[2]));
				(*transform) = translate * (*transform);
			}
			else if (element->ValueStr() == "scale")
			{
				scaleVect = handleScaleTransform(element);
				Transform scale = Transform::scale(Point3d(scaleVect[0], scaleVect[1], scaleVect[2]));
				(*transform) = scale * (*transform);
			}
		}

		//Transform translate = Transform::translate(Point3d(translateVect[0], translateVect[1], translateVect[2]));
		//Transform rotateX = Transform::rotateX(rot[0]);
		//Transform rotateY = Transform::rotateY(rot[1]);
		//Transform rotateZ = Transform::rotateZ(rot[2]);
		//Transform scale = Transform::scale(Point3d(scaleVect[0], scaleVect[1], scaleVect[2]));

		//transform = Transform::ptr(new Transform(scale * translate * rotateX * rotateY * rotateZ));
	}
	//don't use make_shared with types that need to be aligned
	//transform = std::make_shared<Transform>(new Transform(scale * translate * rotateX * rotateY * rotateZ));

	return transform;
}


} //end namespace xml
