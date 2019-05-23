#include "StratifiedSampler.h"

#include "factory/ObjectFactoryManager.h"



#include <iomanip>
#include <cmath>
#include <limits>

StratifiedSampler::StratifiedSampler()
:myCurrentDimension1D(0)
,myCurrentDimension2D(0)
,myCurrentSample(0)
{
}

StratifiedSampler::StratifiedSampler(const Parameters& params)
:myCurrentDimension1D(0)
,myCurrentDimension2D(0)
,myCurrentSample(0)
{
	myNumber = params.getInt("sampleNumber", 1);
	myDimension = params.getInt("dimension", 4);

	int i = 0;
	for (i = 0; i < myNumber; i++)
		if (i * i > myNumber)
			break;
	i -= 1;

	if (i * i != myNumber)
	{
		ILogger::log() << "Stratified sample number should be a square. Rounding to " << i*i << "\n";
		myNumber = i*i;
	}

	mySqrtDimension = i;
	myArray1D.setSize(myDimension, myNumber);
	myArray2D.setSize(myDimension, myNumber);
}


StratifiedSampler::~StratifiedSampler()
{
}

Point2d StratifiedSampler::getNextSample2D()
{
	//std::cout << myCurrentDimension2D << " " << myCurrentSample << std::endl;
	if (myCurrentDimension2D < myDimension)
		return myArray2D(myCurrentDimension2D++, myCurrentSample);
	return Point2d(myRng.nextReal(), myRng.nextReal());
}

real StratifiedSampler::getNextSample1D()
{
	if (myCurrentDimension1D < myDimension)
		return myArray1D(myCurrentDimension1D++, myCurrentSample);
	return myRng.nextReal();
}

Sampler::ptr StratifiedSampler::clone()
{
	StratifiedSampler* sampler(new StratifiedSampler);

	sampler->myNumber = myNumber;
	sampler->mySqrtDimension = mySqrtDimension;
	sampler->myDimension = myDimension;

	sampler->myArray1D.setSize(myDimension, myNumber);
	sampler->myArray2D.setSize(myDimension, myNumber);

	return Sampler::ptr(sampler);
}

void StratifiedSampler::startPixel()
{
	myCurrentSample = 0;

	//stratified 2D, first layer
	int cpt = 0;
	real dx = (real)1. / mySqrtDimension;
	for (int i = 0; i < mySqrtDimension; i++)
	{
		for (int j = 0; j < mySqrtDimension; j++)
		{
			for (int k = 0; k < myDimension; k++)
			{
				real jitterX = myRng.nextReal();
				real jitterY = myRng.nextReal();
				real x = std::min((i + jitterX) * dx, math::ONE_MINUS_EPS);
				real y = std::min((j + jitterY) * dx, math::ONE_MINUS_EPS);
			
				myArray2D(k, cpt) = Point2d(x, y);
			}
			cpt++;
		}
	}

	//stratified 1D, first layer
	dx = (real)1. / myNumber;
	for (int i = 0; i < myNumber; i++)
	{
		for (int k = 0; k < myDimension; k++)
		{
			real val = std::min((i + myRng.nextReal()) * dx, math::ONE_MINUS_EPS);
			myArray1D(k, i) = val;
		}
	}

	for (int k = 0; k < myDimension; k++)
	{
		shuffle(myArray1D, k);
		shuffle(myArray2D, k);
	}

}

void StratifiedSampler::advance()
{
	myCurrentSample++;
	myCurrentDimension1D = 0;
	myCurrentDimension2D = 0;
}

RT_REGISTER_TYPE(StratifiedSampler, Sampler)