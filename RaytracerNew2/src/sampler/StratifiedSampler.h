#pragma once
#include "Sampler.h"

#include "tools/Array2D.h"
#include "tools/Rng.h"
#include "core/Geometry.h"
#include "core/Parameters.h"

class StratifiedSampler :
	public Sampler
{
public:
	StratifiedSampler();
	StratifiedSampler(const Parameters& params);
	~StratifiedSampler();


	Point2d getNextSample2D() override;

	real getNextSample1D() override;

	//Override
	Sampler::ptr clone() override;

	void seed(uint64_t s, uint64_t l) override
	{
		myRng.seed(s, l);
	}

	void startPixel() override;

	void advance() override;

protected:
	template<class T>
	void shuffle(Array2D<T>& array, int dim)
	{
		for (int i = 0; i < myNumber; i++) {
			int other = i + myRng.nextUInt(myNumber - i);
			std::swap(array(dim, i), array(dim, other));
		}
	}

protected:	
	Rng myRng;
	Array2D<real> myArray1D;
	Array2D<Point2d> myArray2D;	

	int myDimension;
	int myCurrentDimension1D;
	int myCurrentDimension2D;
	int myCurrentSample;
	int mySqrtDimension;
};

