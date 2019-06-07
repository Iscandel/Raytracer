#pragma once
#include "BSDF.h"

#include "core/Parameters.h"

class TwoSides :
	public BSDF
{
public:
	TwoSides(const Parameters&);
	~TwoSides();

	Color eval(const BSDFSamplingInfos& infos) override;
	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;
	real pdf(const BSDFSamplingInfos& infos) override;

protected:
	BSDF::ptr myBSDF[2];
};

