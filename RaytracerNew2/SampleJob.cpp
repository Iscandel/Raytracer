#include "SampleJob.h"

#include "Scene.h"
#include "Camera.h"


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

	for(int y = myOffsetY; y < endY; y++)
	{
		//std::cout << y << std::endl;
		for(int x = myOffsetX; x < endX; x++)
		{	
			//std::cout << "x " << x << std::endl;

			for(int i = 0; i < mySampler->getSampleNumber(); i++)
			{
				Point2d sample = mySampler->getNextSample2D();

				double xx = (double)x + sample.x();
				double yy = (double)y + sample.y();
				//double xx = 768;(double)x + sample.x();
				//double yy = 550;(double)y + sample.y();
				//				
				//double xx = 313;(double)x + sample.x();
				//double yy = 416;(double)y + sample.y();

				Ray ray = myCamera->getRay(xx, yy, mySampler->getNextSample2D());

				Color col = myScene->getIntegrator()->li(*myScene, mySampler, ray);

				subScreen->addSample(xx, yy, col);
				//myCamera->getScreen().addSample(xx, yy, col);
			}
		}
	}

	//Merge the subblock with the full screen
	myCamera->getScreen().merge(*subScreen);
}