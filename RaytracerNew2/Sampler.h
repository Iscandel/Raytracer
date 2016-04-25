#pragma once

#include "Geometry.h"

#include <memory>

class Sampler
{
public:
	typedef std::shared_ptr<Sampler> ptr;

public:
	Sampler() {}
	Sampler(int number);
	virtual ~Sampler(void);

	int getSampleNumber() {return myNumber;}
	void setSampleNumber(int samplesPerPixel) {myNumber = samplesPerPixel;}

	virtual Point2d getNextSample2D() = 0;

	virtual double getNextSample1D() = 0;

	virtual Sampler::ptr clone() = 0;

protected:
	int myNumber;
};

