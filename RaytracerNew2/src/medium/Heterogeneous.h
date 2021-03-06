#pragma once
#include "Medium.h"

//#include "tools/Rng.h"
#include "core/Transform.h"
#include "volume/Volume.h"

#include <openvdb/openvdb.h>
//#ifdef WIN32
//#include <Windows.h>
//#endif
//
//#undef min
//#undef max

class Heterogeneous :
	public Medium
{
public:
	Heterogeneous(const Parameters& params);
	~Heterogeneous();

	Color transmittance(const Ray& ray, Sampler::ptr sampler) override;

	bool sampleDistance(const Ray &ray, Sampler::ptr sampler, real &t, Color &weight, Color& emissivity) override;

	void setOwnerBBox(const BoundingBox& box) override;

	bool isEmissive() const override;

	//For emissive volumes, behaving like lights
	Color power() const override;// {
	//	return math::PI * myRadiance * myWorldBoundingBox->surfaceArea();
	//}

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	real pdf(const Point3d& pFrom, const LightSamplingInfos& infos) override;
	//{return 1. / myWorldBoundingBox.getVolumeValue();}

	Color le(const Vector3d& direction, const Point3d& hitPoint = Point3d(), const Normal3d& normal = Normal3d()) const override;

	bool isDelta() const { return false; }

protected:
	real lookupSigmaT(const Point3d &_p) const;
	Color lookupEmissivity(const Point3d &_p) const;

protected:
	Color myAlbedo;
	real myMaxSigmaT;

	//Point3i myResolution;

	real myScale;
	real myScaleEmissivity;

	//float* myData;

	Transform::ptr myWorldToWorldBox;
	//Transform::ptr myWorldBoxToUnitBox;
	Transform::ptr myUnitBoxToMedium;

	//Transform::ptr myWorldToMedium;
	Transform::ptr myWorldToArray; //if the array is not in [0, 1]^3, apply this transform

	BoundingBox myWorldBoundingBox;

	bool myIsCentered;

	//size_t myFileSize;

//	Rng myRng;

	real xmin, ymin, zmin;
	real xmax, ymax, zmax;

	//openvdb::FloatGrid::Ptr grid;

	Volume::ptr myVolume;

	//HANDLE myFileMapping;
	//HANDLE myHfile;
};

