#include "RandomSampler.h"

#include "factory/ObjectFactoryManager.h"
#include "tools/Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RandomSampler::RandomSampler(const Parameters& params)
{
	myNumber = params.getInt("sampleNumber", 1);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RandomSampler::RandomSampler(int sampleNumber)
:Sampler(sampleNumber)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RandomSampler::~RandomSampler(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Point2d RandomSampler::getNextSample2D()
{
	return Point2d(myRng.nextReal(), myRng.nextReal());
	//return Point2d(myRng.random(0., 1.), myRng.random(0., 1.));
}

real RandomSampler::getNextSample1D()
{
	return myRng.nextReal();
	//return myRng.random(0., 1.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Sampler::ptr RandomSampler::clone()
{
	return Sampler::ptr(new RandomSampler(myNumber));
}

RT_REGISTER_TYPE(RandomSampler, Sampler)