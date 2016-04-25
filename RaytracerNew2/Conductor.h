#pragma once
#include "BSDF.h"

class Parameters;

class Conductor :
	public BSDF
{
public:
	Conductor(const Parameters& params);
	~Conductor();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	double pdf(const BSDFSamplingInfos& infos) override;

protected:
	Color myAlbedo;
	//double myEta;
	//double myAbsorption;
	Color myEta;
	Color myAbsorption;
};

