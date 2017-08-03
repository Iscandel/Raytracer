#pragma once
#include "Medium.h"

#include "Parameters.h"

class Homogeneous :
	public Medium
{
public:
	Homogeneous(const Parameters& params);
	~Homogeneous();

	Color transmittance(const Ray& ray) override;

	bool sampleDistance(const Ray &ray, Point2d& sample, double &t, Color &weight);
protected:
	Color mySigmaA;
	Color mySigmaS;
	Color mySigmaT;

	double myScale;
};

