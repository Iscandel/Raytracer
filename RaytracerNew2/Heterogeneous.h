#pragma once
#include "Medium.h"

#include "Rng.h"
#include "Transform.h"


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

	bool sampleDistance(const Ray &ray, Sampler::ptr sampler, double &t, Color &weight) override;

protected:
	float lookupSigmaT(const Point3d &_p) const;

protected:
	Color myAlbedo;
	double myMaxSigmaT;

	Point3i myResolution;

	double myScale;

	float* myData;

	Transform::ptr myWorldToMedium;
	Transform::ptr myWorldToArray; //if the array is not in [0, 1]^3, apply this transform

	size_t myFileSize;

	Rng myRng;

	//HANDLE myFileMapping;
	//HANDLE myHfile;
};

