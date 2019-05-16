#pragma once

#include "BoundingBox.h"
#include "Geometry.h"

#include <memory>

class Parameters;

class Volume
{
public:
	typedef std::shared_ptr<Volume> ptr;

public:
	Volume(const Parameters& params);
	virtual ~Volume();

	virtual real getMaxSigmaT() const = 0;
	virtual real lookupSigmaT(const Point3d &_p) const = 0;
	virtual Color lookupEmissivity(const Point3d &_p) const = 0;
	virtual const BoundingBox& getBoundingBox() const = 0;
	virtual bool isEmissive() const = 0;
	virtual Point3d sample(const Point2d& samplePoint) = 0;

protected:
	real myMaxSigmaT;
	BoundingBox myBoundingBox;
};

