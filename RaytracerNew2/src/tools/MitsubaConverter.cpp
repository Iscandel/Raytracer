#include "MitsubaConverter.h"

#include "Logger.h"
#include "Tools.h"

MitsubaConverter::MitsubaConverter()
{
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
	LIGHTS = "lights";
	NAME = "name";
	VALUE = "value";
	STRING = "string";
	REAL = "real";
	SAMPLE_NUMBER = "sampleNumber";
	INTEGER = "integer";

	_BSDF = "bsdf";
	_DIFFUSE = "diffuse";
	_ROUGH_PLASTIC = "roughplastic";

	myDictionaryByBsdf["diffuse"].insert(std::make_pair("reflectance", "albedo"));
	myDictionaryByBsdf["diffuse"].insert(std::make_pair("diffusereflectance", "albedo"));

	myDictionaryByBsdf["roughdiffuse"].insert(std::make_pair("reflectance", "albedo"));
	myDictionaryByBsdf["roughdiffuse"].insert(std::make_pair("diffusereflectance", "albedo"));

	myDictionaryByBsdf["dielectric"].insert(std::make_pair("intIOR", "etaInt"));
	myDictionaryByBsdf["dielectric"].insert(std::make_pair("extIOR", "etaExt"));
	myDictionaryByBsdf["dielectric"].insert(std::make_pair("specularReflectance", "reflectanceTexture"));
	myDictionaryByBsdf["dielectric"].insert(std::make_pair("specularTransmittance", "transmittedTexture"));

	myDictionaryByBsdf["thindielectric"].insert(std::make_pair("intIOR", "etaInt"));
	myDictionaryByBsdf["thindielectric"].insert(std::make_pair("extIOR", "etaExt"));
	myDictionaryByBsdf["thindielectric"].insert(std::make_pair("specularReflectance", "reflectanceTexture"));
	myDictionaryByBsdf["thindielectric"].insert(std::make_pair("specularTransmittance", "transmittedTexture"));

	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("intIOR", "etaInt"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("extIOR", "etaExt"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("specularReflectance", "reflectanceTexture"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("specularTransmittance", "transmittedTexture"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("alpha", "alpha"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("distribution", "distribution"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("beckmann", "beckmann"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("phong", "phong"));
	myDictionaryByBsdf["roughdielectric"].insert(std::make_pair("ggx", "ggx"));

	myDictionaryByBsdf["conductor"].insert(std::make_pair("eta", "eta"));
	myDictionaryByBsdf["conductor"].insert(std::make_pair("k", "absorption"));
	myDictionaryByBsdf["conductor"].insert(std::make_pair("specularReflectance", "reflectanceTexture"));
	myDictionaryByBsdf["conductor"].insert(std::make_pair("material", "material"));

	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("eta", "eta"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("k", "absorption"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("specularReflectance", "reflectanceTexture"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("material", "material"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("alpha", "alpha"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("distribution", "distribution"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("beckmann", "beckmann"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("phong", "phong"));
	myDictionaryByBsdf["roughconductor"].insert(std::make_pair("ggx", "ggx"));

	myDictionaryByBsdf["smoothplastic"].insert(std::make_pair("intIOR", "etaInt"));
	myDictionaryByBsdf["smoothplastic"].insert(std::make_pair("extIOR", "etaExt"));

	myDictionaryByBsdf["roughplastic"].insert(std::make_pair("alpha", "alphaTop"));
	myDictionaryByBsdf["roughplastic"].insert(std::make_pair("intIOR", "etaInt"));
	myDictionaryByBsdf["roughplastic"].insert(std::make_pair("extIOR", "etaExt"));

	myDictionaryByBsdf["coating"].insert(std::make_pair("intIOR", "etaInt"));
	myDictionaryByBsdf["coating"].insert(std::make_pair("extIOR", "etaExt"));
	myDictionaryByBsdf["coating"].insert(std::make_pair("thickness", "thickness"));
	myDictionaryByBsdf["coating"].insert(std::make_pair("sigmaA", "absorbance"));

	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("intIOR", "etaInt"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("extIOR", "etaExt"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("thickness", "thickness"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("sigmaA", "absorbance"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("alpha", "alphaTop"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("distribution", "distribution"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("beckmann", "beckmann"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("phong", "phong"));
	myDictionaryByBsdf["roughcoating"].insert(std::make_pair("ggx", "ggx"));

	myDictionaryByBsdf["mask"].insert(std::make_pair("opacity", "alpha"));

	myDictionaryBySensor["perspective"].insert(std::make_pair("fov", "fov"));
	//myDictionaryBySensor["perspective"].insert(std::make_pair("width", "sizeX"));
	//myDictionaryBySensor["perspective"].insert(std::make_pair("height", "sizeY"));

	myDictionaryByShape["obj"].insert(std::make_pair("filename", "path"));

	myDictionaryByShape["envmap"].insert(std::make_pair("filename", "path"));

	myDictionaryByTexture["bitmap"].insert(std::make_pair("filename", "path"));
	//myDictionaryByTexture["bitmap"].insert(std::make_pair("wrapMode", "boundaryCondition"));
	myDictionaryByTexture["bitmap"].insert(std::make_pair("gamma", "gamma"));

	myDictionary["id"] = "id";
	myDictionary["ref"] = "ref";

	myDictionary["rgb"] = "color";
	myDictionary["spectrum"] = "color";
	myDictionary["integer"] = "integer";
	myDictionary["string"] = "string";
	myDictionary["boolean"] = "bool";
	myDictionary["float"] = "real";

	myDictionary["point"] = "point";
	myDictionary["vector"] = "vector";
	myDictionary["transform"] = "transform";
	myDictionary["texture"] = "texture";

	myDictionary["rfilter"] = "filter";
	myDictionary["tent"] = "MitchellNetravaliFilter";
	myDictionary["gaussian"] = "MitchellNetravaliFilter";
	myDictionary["mitchell"] = "MitchellNetravaliFilter";

	myDictionary["translate"] = "translation";
	myDictionary["rotate"] = "rotation";
	myDictionary["scale"] = "scale";
	myDictionary["lookAt"] = "lookAt";
	myDictionary["origin"] = "origin";
	myDictionary["target"] = "lookAt";
	myDictionary["up"] = "up";
	myDictionary["matrix"] = "matrix";

	myDictionary["sensor"] = "camera";
	myDictionary["perspective"] = "NoriPerspective";
	myDictionary["orthographic"] = "othographic";

	myDictionary["sampler"] = "sampler";
	myDictionary["independant"] = "RandomSampler";
	myDictionary["stratified"] = "StratifiedSampler";
	myDictionary["ldsampler"] = "StratifiedSampler";
	myDictionary["hammersley"] = "StratifiedSampler";
	myDictionary["sobol"] = "StratifiedSampler";
	myDictionary["halton"] = "StratifiedSampler";

	//myDictionary["filename"] = "path";
	//myDictionary["fov"] = "fov";
	myDictionary["toWorld"] = "toWorld";
	//myDictionary["width"] = "sizeX";
	//myDictionary["height"] = "sizeY";

	myDictionary["shape"] = "shape";
	myDictionary["obj"] = "mesh";
	myDictionary["cube"] = "cube";
	myDictionary["rectangle"] = "plane";
	myDictionary["sphere"] = "sphere";

	myDictionary["emitter"] = "light";
	myDictionary["point"] = "pointlight";
	myDictionary["envmap"] = "environmentLight";
	myDictionary["area"] = "arealight";
	myDictionary["spot"] = "spotlight";
	myDictionary["emitter"] = "light";
	myDictionary["constant"] = "distantlight";

	myDictionary["bsdf"] = "bsdf";
	myDictionary["twosided"] = "twosides";
	myDictionary["bumpmap"] = "bumpmapping";
	myDictionary["coating"] = "smoothlayeredbsdf";
	myDictionary["conductor"] = "conductor";
	myDictionary["dielectric"] = "dielectric";
	myDictionary["diffuse"] = _DIFFUSE;
	myDictionary["normalmap"] = "normalmapping";
	myDictionary["mask"] = "alphabsdf";
	myDictionary["phong"] = "phong";
	myDictionary["plastic"] = "smoothLayeredBSDF";
	myDictionary["roughcoating"] = "layeredbsdf";
	myDictionary["roughconductor"] = "roughconductor";
	myDictionary["roughdielectric"] = "roughdielectric";
	myDictionary["roughdiffuse"] = "diffuse";
	myDictionary[_ROUGH_PLASTIC] = "layeredbsdf";
	myDictionary["thindielectric"] = "sheetdielectric";

	myDictionary["path"] = "pathtracingmis";
	myDictionary["volpath"] = "volpathtracing";

	myDictionary["bitmap"] = "imagetexture";
	myDictionary["checkerboard"] = "checkertexture";
	myDictionary["scale"] = "scaletexture";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
MitsubaConverter::~MitsubaConverter()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
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
		else if (key == "emitter")
		{
			ILogger::log(ILogger::ALL) << "Parsing emitters...\n";
			Bloc bloc;
			handleElement(bloc, element);
			myLights.push_back(bloc);
			ILogger::log(ILogger::ALL) << "Emitters parsed...\n";
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


	//TiXmlAttribute* att = element->FirstAttribute();
	const std::string* attName = element->Attribute(std::string("name"));
	const std::string* attValue = element->Attribute(std::string("value"));
	const std::string* type = element->Attribute(std::string("type"));

	std::string elemType = findFromDictionary(myDictionary, element->ValueStr(), "Element type ");

	//std::string finalAttName;
	//if (attName)
	//	finalAttName = findFromDictionary(myDictionary, *attName, "Att name ");

	if (attName)
		bloc.name = *attName;
	bloc.elemType = element->ValueStr();//elemType;
	if (type)
		bloc.type = *type;

	auto childElement = element->FirstChildElement();
	for (childElement; childElement; childElement = childElement->NextSiblingElement())
	{
		std::shared_ptr<Bloc> child(new Bloc);
		handleElement(*child, childElement);
		bloc.addChild(child);
	}

	if (element->ValueStr() == "boolean" ||
		element->ValueStr() == "integer" ||
		element->ValueStr() == "float" ||
		element->ValueStr() == "string" ||
		element->ValueStr() == "matrix")
	{
		if (attName && *attName == "filename")
			bloc.value = "./data/bathroom2/" + *attValue; //to change
		else
			bloc.value = *attValue;
	}
	else if (element->ValueStr() == "rotate")
	{
		handleAndCorrectTransform(bloc, element);
	}
	else if (element->ValueStr() == "translate")
	{
		handleAndCorrectTransform(bloc, element);
	}
	else if (element->ValueStr() == "scale")
	{
		handleAndCorrectTransform(bloc, element);
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
	else if (element->ValueStr() == "sensor")
	{
		//bloc.type = *type;
		handleAndCorrectSensor(bloc);
	}
	else if (element->ValueStr() == "transform")
	{
		//handleAndCorrectTransform(bloc, element);
	}
	else if (element->ValueStr() == "emitter")
	{
		//Finish by correcting the names
		for (std::shared_ptr<Bloc> child : bloc.childs)
		{
			//handleAndCorrectShape(bloc);
			child->name = findFromDictionary(myDictionaryByLight[lowerCase(bloc.type)], child->name, "");
			child->type = findFromDictionary(myDictionary, child->type, "");
			child->elemType = findFromDictionary(myDictionary, child->elemType, "");
		}
		bloc.elemType = findFromDictionary(myDictionary, bloc.elemType, "");
		bloc.type = findFromDictionary(myDictionary, bloc.type, "");
	}
	else if (element->ValueStr() == "texture")
	{
		//std::string type = element->Attribute("type");

		//bloc.type = *type;
		handleAndCorrectTextures(bloc);
	}
	else if (element->ValueStr() == "bsdf")
	{
		handleAndCorrectBsdf(bloc);
	}
	else if (element->ValueStr() == "integrator")
	{
		//Finish by correcting the names
		for (std::shared_ptr<Bloc> child : bloc.childs)
		{
			//handleAndCorrectShape(bloc);
			//child->name = findFromDictionary(myDictionaryByIntegrator[lowerCase(bloc.type)], child->name, "");
			child->type = findFromDictionary(myDictionary, child->type, "");
			child->elemType = findFromDictionary(myDictionary, child->elemType, "");
		}
		bloc.elemType = findFromDictionary(myDictionary, bloc.elemType, "");
		bloc.type = findFromDictionary(myDictionary, bloc.type, "");
		//handleAndCorrectIntegrator(bloc); 
	}
	else if (element->ValueStr() == "sampler")
	{
	}
	else if (element->ValueStr() == "shape")
	{
		//Finish by correcting the names
		for (std::shared_ptr<Bloc> child : bloc.childs)
		{
			//handleAndCorrectShape(bloc);
			child->name = findFromDictionary(myDictionaryByShape[lowerCase(bloc.type)], child->name, "");
			child->type = findFromDictionary(myDictionary, child->type, "");
			child->elemType = findFromDictionary(myDictionary, child->elemType, "");
		}
		bloc.elemType = findFromDictionary(myDictionary, bloc.elemType, "");
		bloc.type = findFromDictionary(myDictionary, bloc.type, "");
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

	//when everything is done, copy the bloc
	if (id) {
		bloc.id = *id;
		myBlocsById[*id] = bloc;
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::string MitsubaConverter::findFromDictionary(MapDictionary& map,
	const std::string& key, const std::string msg)
{
	std::string res;
	std::string keyTmp = key;
	std::transform(key.begin(), key.end(), keyTmp.begin(), ::tolower);
	if (map.find(keyTmp) == map.end()) {
		res = key;
		if (msg != "")
			ILogger::log() << msg << " " << key << " not found\n";
	}
	else
		res = map[keyTmp];

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::handleAndCorrectBsdf(Bloc& bloc)
{
	std::string blocType = lowerCase(bloc.type);

	if (blocType == "roughplastic" ||
		blocType == "plastic")
	{
		std::shared_ptr<Bloc> childBsdf(new Bloc);
		childBsdf->elemType = _BSDF;
		childBsdf->type = _DIFFUSE;
		childBsdf->name = "baseBSDF";

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
	else if (blocType == "conductor")
	{
		std::string material = getValueWithName(bloc, "material");
		if (material == "none") {
			bloc.type = "mirror";
		}

	}
	else if (blocType == "coating" || blocType == "roughcoating")
		
	{
		auto childBSDF = getSubBlocFromElemType(bloc, "bsdf");
		if (childBSDF.size() == 1)
		{
			(*childBSDF[0])->name = "baseBSDF";
		}
	}
	else if (blocType == "normalmap" || blocType == "bumpmap" ||
		blocType == "mask")
	{
		auto childBSDF = getSubBlocFromElemType(bloc, "bsdf");
		if (childBSDF.size() == 1)
		{
			(*childBSDF[0])->name = "bsdf";
		}
	}
	else if (blocType == "twosided")
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

	//Finish by correcting the names
	for (std::shared_ptr<Bloc> child : bloc.childs)
	{
		handleAndCorrectBsdf(*child);
		child->name = findFromDictionary(myDictionaryByBsdf[blocType], child->name, "");
		child->type = findFromDictionary(myDictionary, child->type, "");
		child->elemType = findFromDictionary(myDictionary, child->elemType, "");
	}
	bloc.elemType = findFromDictionary(myDictionary, bloc.elemType, "");
	bloc.type = findFromDictionary(myDictionary, bloc.type, "");

}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::handleAndCorrectTextures(Bloc& bloc)
{
	if (lowerCase(bloc.type) == "bitmap")
	{
		std::shared_ptr<Bloc> childT(new Bloc);
		childT->elemType = "bool";
		childT->name = "invertY";
		childT->value = "true";

		bloc.addChild(childT);
	}

	//Finish by correcting the names
	for (std::shared_ptr<Bloc> child : bloc.childs)
	{
		handleAndCorrectBsdf(*child);
		child->name = findFromDictionary(myDictionaryByTexture[lowerCase(bloc.type)], child->name, "");
		child->type = findFromDictionary(myDictionary, child->type, "");
		child->elemType = findFromDictionary(myDictionary, child->elemType, "");
	}
	bloc.elemType = findFromDictionary(myDictionary, bloc.elemType, "");
	bloc.type = findFromDictionary(myDictionary, bloc.type, "");

}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::handleAndCorrectSensor(Bloc& bloc)
{
	bloc.elemType = findFromDictionary(myDictionary, bloc.elemType);
	bloc.type = myDictionary[bloc.type];

	auto addProperty = [](Bloc& parent, const std::string& elemType,
		const std::string& name, const std::string& value) {
		std::shared_ptr<Bloc> b(new Bloc);
		b->elemType = elemType; b->name = name; b->value = value;
		parent.addChild(b);
	};

	auto film = getSubBlocFromElemType(bloc, "film");
	std::string sizeX = getValueWithName(*(*film[0]), "width");
	std::string sizeY = getValueWithName(*(*film[0]), "height");

	auto rfilter = getSubBlocFromElemType(*(*film[0]), "rfilter");
	std::shared_ptr<Bloc> filter(new Bloc);
	filter->elemType = findFromDictionary(myDictionary, (*rfilter[0])->elemType, "");
	filter->type = findFromDictionary(myDictionary, (*rfilter[0])->type, "Filter type");
	addProperty(*filter, REAL, "radiusX", "2.");
	addProperty(*filter, REAL, "radiusY", "2.");

	//call addProperty after all operations on film[0] because it invalidates the iterator	
	bloc.childs.erase(film[0]);
	addProperty(bloc, INTEGER, SIZE_X, sizeX);
	addProperty(bloc, INTEGER, SIZE_Y, sizeY);

	bloc.addChild(filter);

	auto sampler = getSubBlocFromElemType(bloc, "sampler");
	mySampler.elemType = findFromDictionary(myDictionary, (*sampler[0])->elemType, "");
	mySampler.type = findFromDictionary(myDictionary, (*sampler[0])->type, "Sampler type");
	std::string nbSamples = getValueWithName(*(*sampler[0]), "sampleCount");
	addProperty(mySampler, INTEGER, SAMPLE_NUMBER, nbSamples);
	bloc.childs.erase(sampler[0]);

	//int cpt = 0;
	//for (std::shared_ptr<Bloc> child : bloc.childs)
	//{
	//	if (lowerCase(child->name) == "film") {
	//		addProperty(bloc, STRING, SIZE_X, getValueWithName(*child, "width"));
	//		addProperty(bloc, STRING, SIZE_Y, getValueWithName(*child, "height"));

	//		for (std::shared_ptr<Bloc> child2 : bloc.childs)
	//		{
	//			if (lowerCase(child->name) == "rfilter") {
	//				myFilter.type = findFromDictionary(myDictionary, child2->name, "Filter type");
	//				addProperty(myFilter, REAL, "radiusX", "2.");
	//				addProperty(myFilter, REAL, "radiusY", "2.");
	//			}
	//		}

	//		bloc.childs.erase(bloc.childs.begin() + cpt);
	//		break;
	//	}
	//	else if (lowerCase(child->name) == "sampler") {
	//		mySampler.type = findFromDictionary(myDictionary, child->name, "Sampler type");
	//		std::string nbSamples = getValueWithName(*child, "sampleCount");
	//		addProperty(mySampler, STRING, SAMPLE_NUMBER, nbSamples);

	//		bloc.childs.erase(bloc.childs.begin() + cpt);
	//		break;
	//	}
	//	cpt++;
	//}

	//Finish by correcting the names
	for (std::shared_ptr<Bloc> child : bloc.childs)
	{
		child->name = findFromDictionary(myDictionaryBySensor[lowerCase(bloc.type)], child->name, "");
		child->type = findFromDictionary(myDictionary, child->type, "");
		child->elemType = findFromDictionary(myDictionary, child->elemType, "");
	}
	bloc.elemType = findFromDictionary(myDictionary, bloc.elemType, "");
	bloc.type = findFromDictionary(myDictionary, bloc.type, "");
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::handleAndCorrectTransform(Bloc& bloc, TiXmlElement* element)
{
	auto addProperty = [](Bloc& parent, const std::string& elemType, const std::string& value) {
		std::shared_ptr<Bloc> b(new Bloc);
		b->elemType = elemType;
		b->textValue = value;
		parent.addChild(b);
	};

	//bloc.elemType = findFromDictionary(myDictionary, element->ValueStr(), "");

	//element = element->FirstChildElement();

	//for (element; element; element = element->NextSiblingElement())
	{
		if (element->ValueStr() == "rotate")
		{
		}
		else if (element->ValueStr() == "translate")
		{
			const std::string* x = element->Attribute(std::string("x"));
			const std::string* y = element->Attribute(std::string("y"));
			const std::string* z = element->Attribute(std::string("z"));
			bloc.elemType = findFromDictionary(myDictionary, element->ValueStr(), "");
			
			if (x) 
				addProperty(bloc, "x", *x);
			if (y)
				addProperty(bloc, "y", *y);
			if (z)
				addProperty(bloc, "z", *z);
		}
		else if (element->ValueStr() == "scale")
		{
			const std::string* x = element->Attribute(std::string("x"));
			const std::string* y = element->Attribute(std::string("y"));
			const std::string* z = element->Attribute(std::string("z"));
			const std::string* value = element->Attribute(std::string("value"));

			bloc.elemType = findFromDictionary(myDictionary, element->ValueStr(), "");

			if (value) {
				addProperty(bloc, "x", *value);
				addProperty(bloc, "y", *value);
				addProperty(bloc, "z", *value);
			}
			else {
				if (x)
					addProperty(bloc, "x", *x);
				if (y)
					addProperty(bloc, "y", *y);
				if (z)
					addProperty(bloc, "z", *z);
			}
			
		}
		else if (element->ValueStr() == "matrix")
		{
			std::string value = element->Attribute("value");
			std::shared_ptr<Bloc> child(new Bloc);
			child->value = value;
			bloc.addChild(child);
		}
		else if (element->ValueStr() == "lookAt")
		{
			const std::string* origin = element->Attribute(std::string("origin"));
			const std::string* target = element->Attribute(std::string("target"));
			const std::string* up = element->Attribute(std::string("up"));
			bloc.elemType = findFromDictionary(myDictionary, element->ValueStr(), "");

			if (origin)
				addProperty(bloc, "origin", *origin);
			if (target)
				addProperty(bloc, "lookAt", *target);
			if (up)
				addProperty(bloc, "up", *up);
		}
		else
		{
			ILogger::log() << "Unknown parameter " << element->ValueStr() << ".\n";
		}
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::writeXML(const std::string& path)
{
	std::ofstream file(path);

	writeHeader(file);

	addElement(file, SCENE);

	//addElementWithAtt(file, CAMERA);


	//addAttribute(file, TYPE, mySensor.type);
	//addClosing(file);
	//addElement(file, TRANSFORM);
	//	addElement(file, LOOK_AT);
	//		addElement(file, ORIGIN);
	//		file << getValue(mySensor, ORIGIN);
	//		addEndElement(file, ORIGIN);
	//		addElement(file, LOOK_AT);
	//		file << getValue(mySensor, LOOK_AT);
	//		addEndElement(file, LOOK_AT);
	//		addElement(file, UP);
	//		file << getValue(mySensor, UP);
	//		addEndElement(file, UP);
	//	addEndElement(file, LOOK_AT);
	//addEndElement(file, TRANSFORM);

	writeBloc(file, mySensor, 1);
	file << "\n";
	writeBloc(file, myIntegrator, 1);
	file << "\n";
	writeBloc(file, mySampler, 1);
	file << "\n";

	addElement(file, LIGHTS, 1);
	for (Bloc& bloc : myLights)
	{
		writeBloc(file, bloc, 2);
	}
	addEndElement(file, LIGHTS, 1);
	file << "\n";

	//addElementWithAtt(file, DOUBLE);
	//addAttribute(file, NAME, FOV);
	//addAttribute(file, VALUE, getValue(mySensor, FOV)); //to change if fov is not present
	//addAttribute(file, NAME, SIZE_X);
	//addAttribute(file, VALUE, getValue(mySensor, SIZE_X));
	//addAttribute(file, NAME, SIZE_Y);
	//addAttribute(file, VALUE, getValue(mySensor, SIZE_Y));
	//addEndElementWithAtt(file);
	//
	//addElementWithAtt(file, FILTER);
	//addAttribute(file, TYPE, "MitchellNetravaliFilter");
	//addEndElementWithAtt(file);
	//addEndElement(file, CAMERA);

	file << "\n";
	file << "<parameters>\n"
		"\t<threadNumber>4</threadNumber>\n"
		"\t<blockSizeX>32</blockSizeX>\n"
		"\t<blockSizeY>32</blockSizeY>\n"
		"\t<fileName>./testRendu.png</fileName>\n"
		"\t<showProgress>1</showProgress>\n"
		"\t</parameters>\n";

	//addElementWithAtt(file, INTEGRATOR);
	//addAttribute(file, TYPE, myIntegrator.type);
	//addEndElementWithAtt(file);

	file << "\n";
	addElement(file, OBJECTS, 1);


	for (Bloc& bloc : myShapes)
	{
		writeBloc(file, bloc, 2);
		file << "\n";
	}

	addEndElement(file, OBJECTS, 1);
	addEndElement(file, SCENE);

	file.close();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::string MitsubaConverter::getValueWithName(const Bloc& bloc, const std::string& name)
{
	std::string res;

	if (bloc.name == name)
		return bloc.value;

	for (std::shared_ptr<Bloc> child : bloc.childs)
	{
		res = getValueWithName(*child, name);
		if (res != "")
			return res;
	}

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::vector<std::vector<std::shared_ptr<MitsubaConverter::Bloc>>::iterator> MitsubaConverter::getSubBlocFromElemType(
	MitsubaConverter::Bloc& bloc, const std::string& elemType)
{
	std::vector<std::vector<std::shared_ptr<MitsubaConverter::Bloc>>::iterator> res;

	//if (bloc.elemType == elemType)
	//	res.push_back(&bloc);

	std::vector<std::shared_ptr<MitsubaConverter::Bloc>>::iterator it = bloc.childs.begin();
	for (; it != bloc.childs.end(); ++it)
	{
		if (lowerCase((*it)->elemType) == elemType)
			res.push_back(it);
	}

	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::writeBloc(std::ofstream& file, Bloc& bloc, int nIndent)
{
	addElementWithAtt(file, bloc.elemType, nIndent);
	if (bloc.type != "")
		addAttribute(file, TYPE, bloc.type);
	if (bloc.name != "")
		addAttribute(file, NAME, bloc.name);
	if (bloc.value != "")
		addAttribute(file, VALUE, bloc.value);

	if (bloc.childs.size() == 0) {
		if (bloc.textValue != "") {
			addClosing(file);
			file << bloc.textValue;
			addEndElement(file, bloc.elemType, nIndent);
		}
		else {
			addEndElementWithAtt(file);
		}	
	}	
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

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::addAttribute(std::ofstream& file, const std::string& name, const std::string& value) {
	file << " " + name + "=\"" + value + "\"";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::addElement(std::ofstream& file, const std::string& element, int nIndent) {
	for (int i = 0; i < nIndent; i++)
		file << "\t";
	file << "<" + element + ">\n";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::addEndElement(std::ofstream& file, const std::string& element, int nIndent) {
	for (int i = 0; i < nIndent; i++)
		file << "\t";
	file << "</" + element + ">\n";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::addElementWithAtt(std::ofstream& file, const std::string& element, int nIndent) {
	for (int i = 0; i < nIndent; i++)
		file << "\t";
	file << "<" + element;// +" ";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::addEndElementWithAtt(std::ofstream& file) {
	file << "/>\n";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::addClosing(std::ofstream& file) {
	file << ">\n";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void MitsubaConverter::writeHeader(std::ofstream& file) {
	file << "<?xml version='1.0' encoding='utf-8'?>\n";
	file << "\n";
}




