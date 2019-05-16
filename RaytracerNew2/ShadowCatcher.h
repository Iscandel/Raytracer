#pragma once
#include "BSDF.h"
#include "Parameters.h"

class ShadowCatcher :
	public BSDF
{
public:
	ShadowCatcher(const Parameters& list);
	~ShadowCatcher();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	real pdf(const BSDFSamplingInfos& infos);

	virtual bool isShadowCatcher() const { return true; }

protected:
	real myHighThreshold;
	real myRatio;
	real myShadowRatio;
};