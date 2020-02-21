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

	virtual bool isTransparent() const override { return myBSDF[0]->isTransparent(); }

protected:
	BSDF::ptr myBSDF[2];
};

