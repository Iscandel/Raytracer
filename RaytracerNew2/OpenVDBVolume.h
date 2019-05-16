#pragma once
#include "Volume.h"

#include <openvdb/openvdb.h>
#include "Parameters.h"
class OpenVDBVolume :
	public Volume
{
public:
	OpenVDBVolume(const Parameters& params);
	~OpenVDBVolume();

	real getMaxSigmaT() const override;
	real lookupSigmaT(const Point3d &_p) const override;
	Color lookupEmissivity(const Point3d &_p) const override;
	const BoundingBox& getBoundingBox() const override;
	bool isEmissive() const override { return myIsEmissive; }

	Point3d sample(const Point2d& samplePoint) override;

protected:
	openvdb::FloatGrid::Ptr myDensityGrid;
	openvdb::FloatGrid::Ptr myTemperatureGrid;
	openvdb::Vec3SGrid::Ptr myTempGrid;

	Point3i myDensityResolution;
	Point3i myTemperatureResolution;

	//array coordinates. In openVDB, indices can be < 0
	Point3i myDensityIndexSpaceBoundingBoxMin;
	Point3i myDensityIndexSpaceBoundingBoxMax;

	Point3i myTemperatureIndexSpaceBoundingBoxMin;
	Point3i myTemperatureIndexSpaceBoundingBoxMax;

	real myTemperatureScale;
	real myMaxGridTemperature;
	real myMinGridTemperature;

	int myInterpOrder;

	bool myIsEmissive;
};

