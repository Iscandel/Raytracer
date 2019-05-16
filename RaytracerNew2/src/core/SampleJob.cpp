#include "SampleJob.h"

#include "Scene.h"
#include "camera/Camera.h"


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
SampleJob::SampleJob(int offsetX, 
					 int offsetY, 
					 int sizeX, 
					 int sizeY, 
					 Scene* scene, 
					 Camera::ptr camera, 
					 std::shared_ptr<Sampler> sampler)
:Job(offsetX, offsetY, sizeX, sizeY, scene, camera, sampler)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
SampleJob::~SampleJob(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void SampleJob::run() 
{
	int startX = std::max(0, myOffsetX);
	int endX = std::min(myCamera->getSizeX(), myOffsetX + mySizeX);
	int startY = std::max(0, myOffsetY);
	int endY = std::min(myCamera->getSizeY(), myOffsetY + mySizeY);

	std::unique_ptr<Screen> subScreen = 
		myCamera->getScreen().createSubScreen(endX - startX, endY - startY, myOffsetX, myOffsetY);

	Integrator* integrator = myScene->getIntegrator().get();

	for(int y = myOffsetY; y < endY; y++)
	{
		//std::cout << y << std::endl;
		for(int x = myOffsetX; x < endX; x++)
		{	
			//std::cout << "x " << x << std::endl;

			for(int i = 0; i < mySampler->getSampleNumber(); i++)
			{
				Point2d sample = mySampler->getNextSample2D();
				//49 311
				//594 163
				//596 160
				real xx = (real)x + sample.x();
				real yy = (real)y + sample.y();

				Point2d lensSample;
				if(myCamera->needsDoFSample())
					lensSample = mySampler->getNextSample2D();
				Ray ray = myCamera->getRay(xx, yy, lensSample);

				Color col = integrator->li(*myScene, mySampler, ray);

				subScreen->addSample(xx, yy, col);
				//myCamera->getScreen().addSample(xx, yy, col);
			}
		}
	}

	//Merge the subblock with the full screen
	myCamera->getScreen().merge(*subScreen);
}