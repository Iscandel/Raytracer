#pragma once
#include "BSDF.h"

class Parameters;

class Mirror :
	public BSDF
{
public:
	Mirror(const Parameters& params);
	~Mirror();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;
};

