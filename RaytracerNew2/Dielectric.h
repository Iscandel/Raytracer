#pragma once
#include "BSDF.h"

#include "Parameters.h"

class Dielectric :
	public BSDF
{
public:
	Dielectric(const Parameters& params);
	~Dielectric();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	double pdf(const BSDFSamplingInfos& infos) override;

protected:
	Color myAlbedo;
	//double myEta;
	//double myAbsorption;
	double myEtaI;
	double myEtaT;
};

