#pragma once

#include "core/Geometry.h"
#include "core/RaytracerObject.h"

#include <memory>

class Sampler : public RaytracerObject
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

	virtual real getNextSample1D() = 0;

	virtual Sampler::ptr clone() = 0;

	virtual void seed(uint64_t s, uint64_t l) = 0;

protected:
	int myNumber;
};

