#include "MitsubaConverter.h"

#include "Logger.h"
#include "Tools.h"

MitsubaConverter::MitsubaConverter()
{
	myDictionary["rgb"] = "color";
	myDictionary["spectrum"] = "color";
	myDictionary["integer"] = "integer";
	myDictionary["string"] = "string";
	myDictionary["boolean"] = "bool";
	myDictionary["float"] = "double";

	myDictionary["point"] = "point";
	myDictionary["vector"] = "vector";
	myDictionary["transform"] = "transform";
	myDictionary["texture"] = "texture";

	myDictionary["filename"] = "path";
	myDictionary["fov"] = "fov";
	myDictionary["toWorld"] = "toWorld";
	myDictionary["width"] = "sizeX";
	myDictionary["height"] = "sizeY";

	myDictionary["obj"] = "mesh";

	myDictionary["perspective"] = "NoriPerspective";

	myDictionary["envmap"] = "environmentLight";
	
	myDictionary["twosided"] = "twosides";
	myDictionary["bumpmap"] = "bumpmapping";
	myDictionary["coating"] = "smoothlayeredbsdf";
	myDictionary["conductor"] = "conductor";
	myDictionary["dielectric"] = "dielectric";
	myDictionary["diffuse"] = _DIFFUSE;
	myDictionary["normalmap"] = "normalmapping";
	myDictionary["phong"] = "phong";
	myDictionary["plastic"] = "smoothLayeredBSDF";
	myDictionary["roughcoating"] = "layeredbsdf";
	myDictionary["roughconductor"] = "roughconductor";
	myDictionary["roughdielectric"] = "roughdielectric";
	myDictionary["roughdiffuse"] = "diffuse";
	myDictionary[_ROUGH_PLASTIC] = "layeredbsdf";
	myDictionary["thindielectric"] = "sheetdielectric";

	myDictionary["k"] = "absorption";
	myDictionary["reflectance"] = "albedo";
	myDictionary["specularReflectance"] = "reflectanceTexture";

	myDictionary["path"] = "pathtracingmis";
	myDictionary["volpath"] = "volpathtracing";

	myDictionary["bitmap"] = "imagetexture";
	myDictionary["checkerboard"] = "checkertexture";
	myDictionary["scale"] = "scaletexture";

	myDictionary["alpha"] = "alpha";
	myDictionary["distribution"] = "distribution";
	myDictionary["intior"] = "etaInt";
	myDictionary["extior"] = "etaExt";
	myDictionary["diffusereflectance"] = "reflectanceTexture";

	SCENE = "scene";
	CAMERA = "camera";
	TYPE = "type";
	TRANSFORM = "transform";
	LOOK_AT = "lookAt";
	ORIGIN = "origin";
	UP = "up";
	DOUBLE = "double";
	FOV = "fov";
	SIZE_X = "sizeX";
	SIZE_Y = "sizeY";
	FILTER = "filter";
	INTEGRATOR = "integrator";
	OBJECTS = "objects";
	NAME = "name";
	VALUE = "value";

	_BSDF = "bsdf";
	_DIFFUSE = "diffuse";
	_ROUGH_PLASTIC = "roughplastic";
}


MitsubaConverter::~MitsubaConverter()
{
}

bool MitsubaConverter::convert(const std::string& pathFrom, const std::string& pathDest)
{
	TiXmlDocument doc(pathFrom.c_str());
	if (!doc.LoadFile())
	{
		return false;
	}

	ILogger::log() << "Parsing " << pathFrom << "\n";
	::TiXmlHandle hdl(&doc);
	TiXmlElement* element = hdl.FirstChildElement().FirstChildElement().Element();

	if (!element)
	{
		ILogger::log(ILogger::ERRORS) << "XML loading : Node to reach doesn't exist.\n";
		return false;
	}

	//loop on all elements
	while (element)
	{
		std::string key = element->ValueStr();
		if (key == "integrator")
		{
			ILogger::log(ILogger::ALL) << "Parsing integrator...\n";
			handleElement(myIntegrator, element);
			ILogger::log(ILogger::ALL) << "Camera integrator...\n";
		}
		else if (key == "sensor")
		{
			ILogger::log(ILogger::ALL) << "Parsing sensor...\n";
			handleElement(mySensor, element);
			ILogger::log(ILogger::ALL) << "sensor parsed...\n";
		}
		else if (key == "bsdf")
		{
			ILogger::log(ILogger::ALL) << "Parsing bsdf id...\n";
			Bloc bloc;
			handleElement(bloc, element);
			myBsdfs.push_back(bloc);
			ILogger::log(ILogger::ALL) << "bsdf ids parsed...\n";
		}
		else if (key == "shape")
		{
			ILogger::log(ILogger::ALL) << "Parsing shapes...\n";
			Bloc bloc;
			handleElement(bloc, element);
			myShapes.push_back(bloc);
			ILogger::log(ILogger::ALL) << "shapes parsed...\n";
		}
		else if (key == "textures")
		{
			ILogger::log(ILogger::ALL) << "Parsing shapes...\n";
			Bloc bloc;
			handleElement(bloc, element);
			myTextures.push_back(bloc);
			ILogger::log(ILogger::ALL) << "shapes parsed...\n";
		}

		element = element->NextSiblingElement();
	}

	writeXML(pathDest);

	return true;
}

	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	void MitsubaConverter::handleElement(Bloc& bloc, TiXmlElement* element)
	{
		//std::string elemType = findFromDictionary(element->ValueStr(), "Element type ");

		//std::string type = element->Attribute("type");
		//const std::string* name = element->Attribute(std::string("name"));
		const std::string* id = element->Attribute(std::string("id"));
		//element = element->FirstChildElement();
		//vec.push_back(MitsubaConverter::Bloc());

		//vec.back().elemType = elemType;

		//type = findFromDictionary(type, "Type ");

		//vec.back().type = type;
		//if(name)
		//	vec.back().name = *name;
		//	
		//for (element; element; element = element->NextSiblingElement())
		//{
		//	handleProperty(element, vec.back());
		//}

		if (id) {
			bloc.id = *id;
			myBlocsById[*id] = bloc;
		}

		//TiXmlAttribute* att = element->FirstAttribute();
		const std::string* attName = element->Attribute(std::string("name"));
		const std::string* attValue = element->Attribute(std::string("value"));

		std::string elemType = findFromDictionary(element->ValueStr(), "Element type ");


		std::string finalAttName;
		if (attName)
			finalAttName = findFromDictionary(*attName, "Att name ");

		bloc.name = finalAttName;
		bloc.elemType = elemType;

		auto childElement = element->FirstChildElement();
		for (childElement; childElement; childElement = childElement->NextSiblingElement())
		{
			std::shared_ptr<Bloc> child(new Bloc);
			handleProperty(childElement, *child);
			bloc.addChild(child);
		}

		if (element->ValueStr() == "boolean" ||
			element->ValueStr() == "integer" ||
			element->ValueStr() == "float" ||
			element->ValueStr() == "string")
		{
			if (finalAttName == "path")
				bloc.value = "./data/living-room/" + *attValue; //to change
			else
				bloc.value = *attValue;
		}

		else if (element->ValueStr() == "point" ||
			element->ValueStr() == "vector")
		{
			const std::string* x = element->Attribute(std::string("x"));
			const std::string* y = element->Attribute(std::string("y"));
			const std::string* z = element->Attribute(std::string("z"));

			bloc.value = *x + " " + *y + " " + *z;
		}
		else if (element->ValueStr() == "rgb" ||
			element->ValueStr() == "spectrum")
		{
			bloc.value = *attValue;
		}
		else if (element->ValueStr() == "transform")
		{
			handleTransform(bloc, element);
		}
		else if (element->ValueStr() == "texture")
		{
			std::string type = element->Attribute("type");

			bloc.type = myDictionary[type];
		}
		else if (element->ValueStr() == "bsdf")
		{
			std::string type = element->Attribute("type");
			//const std::string* id = element->Attribute(std::string("id")); //shouldn't appear here

			if (myDictionary.find(type) == myDictionary.end())
			{
				ILogger::log() << "Type " << type << " not found\n";
			}

			bloc.type = myDictionary[type];

			handleAndCorrectBsdf(bloc); //move to handleElement()
		}
		else if (element->ValueStr() == "sampler")
		{
			std::string type = element->Attribute("type");

			bloc.type = myDictionary[type];
		}
		else if (element->ValueStr() == "film")
		{
			std::string type = element->Attribute("type");

			//child->type = myDictionary[type];

			//bloc.addChild(child);
		}
		else if (element->ValueStr() == "ref")
		{
			std::string id = element->Attribute("id");
			const std::string* name = element->Attribute(std::string("name"));
			bloc = myBlocsById[id];
			//child = std::make_shared<Bloc>(myBlocsById[id]);
			if (name)
				bloc.name = *name;
		}
		//else if (element->ValueStr() == "medium")
		//{
		//	Medium::ptr medium = handleMedium(scene, element);
		//	params.addMedium(*attName, medium);
		//	if (medium->isEmissive())
		//	{
		//		scene.addLight(medium);
		//	}
		//}
		//else if (element->ValueStr() == "phase")
		//{
		//	PhaseFunction::ptr phase = handlePhase(scene, element);
		//	params.addPhaseFunction(*attName, phase);
		//}
		//else if (element->ValueStr() == "volume")
		//{
		//	Volume::ptr volume = handleVolume(scene, element);
		//	params.addVolume(*attName, volume);
		//}
		else
		{
			ILogger::log() << "Unknown property " << element->ValueStr() << " with name  " << (attName ? *attName : " not given") << "\n";
		}
	}

	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	void MitsubaConverter::handleProperty(TiXmlElement* element, Bloc& bloc)
	{
		//TiXmlAttribute* att = element->FirstAttribute();
		const std::string* attName = element->Attribute(std::string("name"));
		const std::string* attValue = element->Attribute(std::string("value"));

		std::string elemType = findFromDictionary(element->ValueStr(), "Element type ");


		std::string finalAttName;
		if (attName)
		{
			finalAttName = findFromDictionary(*attName, "Att name ");
		}

		std::shared_ptr<Bloc> child(new Bloc);
		child->name = finalAttName; 
		child->elemType = elemType;

		if (element->ValueStr() == "boolean" ||
			element->ValueStr()== "integer" ||
			element->ValueStr() == "float" || 
			element->ValueStr() == "string")
		{
			if (finalAttName == "path")
				child->value = "./data/living-room/" + *attValue; //to change
			else
				child->value = *attValue;
			bloc.addChild(child);
		}
		
		else if (element->ValueStr() == "point" ||
			element->ValueStr() == "vector")
		{
			const std::string* x = element->Attribute(std::string("x"));
			const std::string* y = element->Attribute(std::string("y"));
			const std::string* z = element->Attribute(std::string("z"));
			
			child->value = *x + " " + *y + " " + *z;
			bloc.addChild(child);
		}
		else if (element->ValueStr() == "rgb" ||
				element->ValueStr() == "spectrum")
		{
			child->value = *attValue;
			bloc.addChild(child);

		}
		else if (element->ValueStr() == "transform")
		{
			handleTransform(*child, element);
			bloc.addChild(child);
		}
		else if (element->ValueStr() == "texture")
		{
			std::string type = element->Attribute("type");
			
			child->type = myDictionary[type];
			
			//ILogger::log() << "texture " << type << " not implemented\n";
			//child->type = correctTextureType(type);
			element = element->FirstChildElement();
			for (element; element; element = element->NextSiblingElement())
			{
				handleProperty(element, *child);
			}
			
			bloc.addChild(child);
		}
		else if (element->ValueStr() == "bsdf")
		{
			std::string type = element->Attribute("type");
			//const std::string* id = element->Attribute(std::string("id")); //shouldn't appear here

			if (myDictionary.find(type) == myDictionary.end())
			{
				ILogger::log() << "Type " << type << " not found\n";
			}

			child->type = myDictionary[type];
			
			element = element->FirstChildElement();
			for (element; element; element = element->NextSiblingElement())
			{
				handleProperty(element, *child);
			}

			bloc.addChild(child);

			handleAndCorrectBsdf(bloc); //move to handleElement()
		}
		else if (element->ValueStr() == "sampler")
		{
			std::string type = element->Attribute("type");

			child->type = myDictionary[type];

			element = element->FirstChildElement();
			for (element; element; element = element->NextSiblingElement())
			{
				handleProperty(element, *child);
			}

			bloc.addChild(child);
		}
		else if (element->ValueStr() == "film")
		{
			std::string type = element->Attribute("type");

			//child->type = myDictionary[type];

			element = element->FirstChildElement();
			for (element; element; element = element->NextSiblingElement())
			{
				//handleProperty(element, *child);
				handleProperty(element, bloc); //add film properties to camera
			}

			//bloc.addChild(child);
		}
		else if(element->ValueStr() == "ref")
		{
			std::string id = element->Attribute("id");
			const std::string* name = element->Attribute(std::string("name"));
			child = std::make_shared<Bloc>(myBlocsById[id]);
			if (name)
				child->name = *name;

			bloc.addChild(child);
		}
		//else if (element->ValueStr() == "medium")
		//{
		//	Medium::ptr medium = handleMedium(scene, element);
		//	params.addMedium(*attName, medium);
		//	if (medium->isEmissive())
		//	{
		//		scene.addLight(medium);
		//	}
		//}
		//else if (element->ValueStr() == "phase")
		//{
		//	PhaseFunction::ptr phase = handlePhase(scene, element);
		//	params.addPhaseFunction(*attName, phase);
		//}
		//else if (element->ValueStr() == "volume")
		//{
		//	Volume::ptr volume = handleVolume(scene, element);
		//	params.addVolume(*attName, volume);
		//}
		else
		{
			ILogger::log() << "Unknown property " << element->ValueStr() << " with name  " << (attName ? *attName : " not given") << "\n";
		}
	}

	std::string MitsubaConverter::findFromDictionary(const std::string& key, const std::string msg)
	{
		std::string res;
		std::string keyTmp = key;
		std::transform(key.begin(), key.end(), keyTmp.begin(), ::tolower);
		if (myDictionary.find(keyTmp) == myDictionary.end()) {
			res = key;
			if(msg != "")
				ILogger::log() << msg << " " << key << " not found\n";
		}
		else
			res = myDictionary[keyTmp];

		return res;
	}

	void MitsubaConverter::handleAndCorrectBsdf(Bloc& bloc)
	{
		if (lowerCase(bloc.type) == "roughplastic")
		{
			std::shared_ptr<Bloc> childBsdf(new Bloc);
			childBsdf->elemType = _BSDF;
			childBsdf->type = _DIFFUSE;
			
			int cpt = 0;
			for (std::shared_ptr<Bloc> child : bloc.childs)
			{
				if (child->name == "diffuseReflectance") {
					childBsdf->addChild(child);
					bloc.childs.erase(bloc.childs.begin() + cpt);
					break;
				}
				cpt++;
			}

			bloc.addChild(childBsdf);
		}
		if (lowerCase(bloc.type) == "twosided")
		{
			//bloc.type = myDictionary[bloc.type];
			bool doneOne = false;
			for (std::shared_ptr<Bloc> child : bloc.childs)
			{
				if (child->elemType == _BSDF)
					if (child->name == "")
						if (!doneOne) {
							child->name = _BSDF;
							doneOne = true;
						}
						else
							child->name = "bsdf2";
			}
		}

		bloc.type = myDictionary[bloc.type];
		for (std::shared_ptr<Bloc> child : bloc.childs)
		{
			handleAndCorrectBsdf(*child);
		}
	}

	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	void MitsubaConverter::handleTransform(Bloc& bloc, TiXmlElement* element)
	{
		Transform::ptr transform(new Transform);
		std::vector<real> translateVect(3, 0.);
		std::vector<real> rot(3, 0.);
		std::vector<real> scaleVect(3, 1.);

		element = element->FirstChildElement();

		for (element; element; element = element->NextSiblingElement())
		{
			if (element->ValueStr() == "rotate")
			{
			}
			else if (element->ValueStr() == "translation")
			{
			}
			else if (element->ValueStr() == "scale")
			{
			}
			else if (element->ValueStr() == "matrix")
			{
				std::shared_ptr<Bloc> child(new Bloc);

				child->elemType = element->ValueStr();
				std::string value = element->Attribute("value");

				child->value = value;
				bloc.addChild(child);
			}
			else if (element->ValueStr() == "lookAt")
			{
			}
			else
			{
				ILogger::log() << "Unknown parameter " << element->ValueStr() << ".\n";
			}
		}
	}

	void MitsubaConverter::writeXML(const std::string& path)
	{
		std::ofstream file(path);

		writeHeader(file);

		addElement(file, SCENE);

		addElementWithAtt(file, CAMERA);
		
		
		addAttribute(file, TYPE, mySensor.type);
		addClosing(file);
		addElement(file, TRANSFORM);
			addElement(file, LOOK_AT);
				addElement(file, ORIGIN);
				file << getValue(mySensor, ORIGIN);
				addEndElement(file, ORIGIN);
				addElement(file, LOOK_AT);
				file << getValue(mySensor, LOOK_AT);
				addEndElement(file, LOOK_AT);
				addElement(file, UP);
				file << getValue(mySensor, UP);
				addEndElement(file, UP);
			addEndElement(file, LOOK_AT);
		addEndElement(file, TRANSFORM);

		//writeBloc(file, mySensor, 1);
		//file << "\n";
		//writeBloc(file, myIntegrator, 1);
		//file << "\n";

		addElementWithAtt(file, DOUBLE);
		addAttribute(file, NAME, FOV);
		addAttribute(file, VALUE, getValue(mySensor, FOV)); //to change if fov is not present
		addAttribute(file, NAME, SIZE_X);
		addAttribute(file, VALUE, getValue(mySensor, SIZE_X));
		addAttribute(file, NAME, SIZE_Y);
		addAttribute(file, VALUE, getValue(mySensor, SIZE_Y));
		addEndElementWithAtt(file);
		
		addElementWithAtt(file, FILTER);
		addAttribute(file, TYPE, "MitchellNetravaliFilter");
		addEndElementWithAtt(file);
		addEndElement(file, CAMERA);

		file << "\n";
		file << "<parameters>\n"
			"\t<threadNumber>4</threadNumber>\n"
			"\t<blockSizeX>32</blockSizeX>\n"
			"\t<blockSizeY>32</blockSizeY>\n"
			"\t<fileName>./testRendu.png</fileName>\n"
			"\t<showProgress>1</showProgress>\n"
			"\t</parameters>\n"
			"\t<sampler type = \"RandomSampler\">\n"
			"\t<integer name = \"sampleNumber\" value = \"16\"/>\n"
			"\t</sampler>\n";

		addElementWithAtt(file, INTEGRATOR);
		addAttribute(file, TYPE, myIntegrator.type);
		addEndElementWithAtt(file);
			
		file << "\n";
		addElement(file, OBJECTS);

		
		for (Bloc& bloc : myShapes)
		{
			writeBloc(file, bloc, 1);
			file << "\n";
		}

		addEndElement(file, OBJECTS);
		addEndElement(file, SCENE);

		file.close();
	}

	std::string MitsubaConverter::getValue(const Bloc& bloc, const std::string& name)
	{
		std::string res;

		if (bloc.name == name)
			return bloc.value;

		for (std::shared_ptr<Bloc> child : bloc.childs)
		{
			res = getValue(*child, name);
			if (res != "")
				return res;
		}

		return res;
	}

	void MitsubaConverter::writeBloc(std::ofstream& file, Bloc& bloc, int nIndent)
	{
		addElementWithAtt(file, bloc.elemType, nIndent);
		if (bloc.type != "")
			addAttribute(file, TYPE, bloc.type);
		if (bloc.name != "")
			addAttribute(file, NAME, bloc.name);
		if (bloc.value != "")
			addAttribute(file, VALUE, bloc.value);

		if (bloc.childs.size() == 0)
			addEndElementWithAtt(file);
		else
		{
			addClosing(file);
			for (std::shared_ptr<Bloc> child : bloc.childs)
			{
				writeBloc(file, *child, nIndent + 1);
			}

			addEndElement(file, bloc.elemType, nIndent);
		}
	}

	void MitsubaConverter::addAttribute(std::ofstream& file, const std::string& name, const std::string& value) {
		file << " " + name + "=\"" + value + "\"";
	}

	void MitsubaConverter::addElement(std::ofstream& file, const std::string& element, int nIndent) {
		for (int i = 0; i < nIndent; i++)
			file << "\t";
		file << "<" + element + ">\n";
	}

	void MitsubaConverter::addEndElement(std::ofstream& file, const std::string& element, int nIndent) {
		for (int i = 0; i < nIndent; i++)
			file << "\t";
		file << "</" + element + ">\n";
	}

	void MitsubaConverter::addElementWithAtt(std::ofstream& file, const std::string& element, int nIndent) {
		for (int i = 0; i < nIndent; i++)
			file << "\t";
		file << "<" + element;// +" ";
	}

	void MitsubaConverter::addEndElementWithAtt(std::ofstream& file) {
		file << "/>\n";
	}

	void MitsubaConverter::addClosing(std::ofstream& file) {
		file << ">\n";
	}

	void MitsubaConverter::writeHeader(std::ofstream& file) {
		file << "<?xml version='1.0' encoding='utf-8'?>\n";
		file << "\n";
	}
	



