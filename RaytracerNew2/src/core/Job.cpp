#include "Job.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Job::Job(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Job::Job(int offsetX, int offsetY, int sizeX, int sizeY, Scene* scene, Camera::ptr camera, Sampler::ptr sampler)
:myScene(scene)
,myCamera(camera)
,mySampler(sampler)
,myOffsetX(offsetX)
,myOffsetY(offsetY)
,mySizeX(sizeX)
,mySizeY(sizeY)
,myIsFinished(false)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Job::~Job()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool Job::isFinished() const
{
	return myIsFinished;
}
