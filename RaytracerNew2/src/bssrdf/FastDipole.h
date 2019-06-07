#pragma once
#include "BSSRDF.h"
#include "core/Parameters.h"

struct Octree;

class FastDipole :
	public BSSRDF
{
public:
	FastDipole(const Parameters& params);
	~FastDipole();

	void initialize(Scene& scene) override;
	Color eval(const Point3d& pos, const Intersection& inter, const Vector3d& dir) override;

protected:
	int myNumberOfPoints;
	int myNbSamplesLight;
	std::unique_ptr<Octree> myOctree;

	real myMaxError;
	real myEtaExt;
	real myEtaInt;
	Color mySigmaPrimeT;
	Color mySigmaA;
	real myFdr;
	Color myZr;
	Color myZv;
	Color mySigmaTr;
	bool myIsMultithreadedOctreeInit;
	bool myIsIndirectIrradiance;
	bool myIsInitialized;

	real myScaleResult; //hack
	bool mySqueezeZeroIrradiance;
};

