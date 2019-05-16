#pragma once
#include "Volume.h"
#include "core/Parameters.h"

class GridVolume :
	public Volume
{
public:
	GridVolume(const Parameters& params);
	~GridVolume();

	real getMaxSigmaT() const override;
	real lookupSigmaT(const Point3d &_p) const override;
	Color lookupEmissivity(const Point3d &_p) const override { return Color(); }
	const BoundingBox& getBoundingBox() const override;
	bool isEmissive() const { return false;  } //to change
	Point3d sample(const Point2d& samplePoint) override { throw std::runtime_error("Not implemented"); }

protected:
	Point3i myResolution;

	float* myData;
};

