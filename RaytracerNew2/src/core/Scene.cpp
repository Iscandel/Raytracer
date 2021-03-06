#include "Scene.h"

#include "XMLLoader.h"
#include "tools/Tools.h"
#include "core/SampleJob.h"
#include "tools/Logger.h"
#include "tools/Timer.h"
#include "accelStruct/Bvh.h"
#include "integrator/DirectLightingMIS.h"

#include <Eigen\StdVector>

#include <iostream>
#include <fstream>
#include <unordered_map>



Scene::Scene(void)
{
	//myCameraMedium = Medium::ptr(new Homogeneous(Parameters()));
	Color::init();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Scene::~Scene(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Scene::addPrimitive(IPrimitive::ptr object)
{
	myObjects.push_back(object);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Scene::setSampler(Sampler::ptr sampler) 
{
	mySampler = sampler;
}

void Scene::setBlockSize(int sizeX, int sizeY)
{
	myBlockSize = Point2i(sizeX, sizeY);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Scene::setSamplesPerPixel(int samplesPerPixel) 
{
	mySampler->setSampleNumber(samplesPerPixel);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Scene::compute(const std::string& filePath)
{
	Timer clock;

	ILogger::log() << "Parsing the scene...\n";

	xml::XMLLoader loader;
	if (!loader.loadScene(*this, filePath))
	{
		ILogger::log(ILogger::ERRORS) << "File " << filePath << " not parsed.\n";
		return;
	}

	//readWavefrontOBJ(*this);
	//readPly();

	ILogger::log() << "Scene parsed...\n";

	initialize();

	ILogger::log() << "Elapsed time loading + acceleration structure : " << clock.elapsedTime() << "s.\n";

	ILogger::log() << "Creating jobs...\n";

	std::vector<std::shared_ptr<Job> > jobs;
	for(int currentY = 0; currentY < myCamera->getSizeY(); currentY += myBlockSize.y())
	{
		//std::cout << currentY << std::endl;
		for(int currentX = 0; currentX < myCamera->getSizeX(); currentX += myBlockSize.x())
		{	
			std::shared_ptr<Sampler> sampler(mySampler->clone());
			sampler->seed(currentX, currentY);
		
			std::shared_ptr<Job> job(new SampleJob(currentX, currentY, myBlockSize.x(), myBlockSize.y(), this, myCamera, sampler));
			jobs.push_back(job);			
		}	
	}

	ILogger::log() << "Jobs created...\n";

	ILogger::log() << "Computing the scene with " << myManager.getThreadNumber() << " threads...\n";

	myManager.addJobs(jobs);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool Scene::computeIntersection(const Ray& ray, Intersection& inter, bool shadowRay) const 
{
	inter.computeIntersect = true;
	if (myAccelerator->intersection(ray, inter, shadowRay))
		return true;

	return false;
}

void Scene::initialize()
{
	myAccelerator.reset(new Bvh(myObjects));
	myBoundingBox = myAccelerator->getWorldBoundingBox();

	for (Light::ptr light : myLights)
	{
		light->initialize(*this);
	}

	if (myIntegrator == nullptr)
	{
		ILogger::log() << "No integrator defined. Default assigned.\n";
		myIntegrator = Integrator::ptr(new DirectLightingMIS(Parameters()));
	}

	myIntegrator->initialize(*this);

	for (auto object : myObjects)
		object->initialize(*this);

	//Print scene infos
	//printSceneInfos();
}

void Scene::setShowProgress(bool show)
{
	myManager.setShowProgress(show);
}

//void Scene::printSceneInfos()
//{
//	ILogger::log(ILogger::ALL) << "Scene <["
//}