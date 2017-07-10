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

	double pdf(const BSDFSamplingInfos& infos);

	virtual bool isShadowCatcher() const { return true; }

protected:
	double myHighThreshold;
	double myRatio;
};