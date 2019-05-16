#pragma once
#include "Medium.h"

#include "core/Parameters.h"

class Homogeneous :
	public Medium
{
public:
	Homogeneous(const Parameters& params);
	~Homogeneous();

	Color transmittance(const Ray& ray, Sampler::ptr sampler) override;

	bool sampleDistance(const Ray &ray, Sampler::ptr sample, real &t, Color &weight, Color& emissivity) override;

	bool isEmissive() const override { return !myEmission.isZero(); }//to change 

	void setOwnerBBox(const BoundingBox& box) override
	{
		myWorldBoundingBox = box;
	}

	//For emissive volumes, behaving like lights
	Color power() const {
		return Color(1.); //to change
	}

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	virtual real pdf(const Point3d& pFrom, const LightSamplingInfos& infos) {
		return 1. / myWorldBoundingBox.getVolumeValue();
	}

	virtual Color le(const Vector3d& direction, const Normal3d& normal = Normal3d()) const { return myEmission * myScaleEmissivity; } //to change

	bool isDelta() const { return false; }

protected:
	Color mySigmaA;
	Color mySigmaS;
	Color mySigmaT;

	Color myEmission;
	real myScaleEmissivity;

	BoundingBox myWorldBoundingBox;

	real myScale;
};

