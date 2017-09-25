#pragma once
#include "Medium.h"

#include "Parameters.h"

class Homogeneous :
	public Medium
{
public:
	Homogeneous(const Parameters& params);
	~Homogeneous();

	Color transmittance(const Ray& ray, Sampler::ptr sampler) override;

	bool sampleDistance(const Ray &ray, Sampler::ptr sample, double &t, Color &weight) override;
protected:
	Color mySigmaA;
	Color mySigmaS;
	Color mySigmaT;

	double myScale;
};

