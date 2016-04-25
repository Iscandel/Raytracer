#pragma once

#include "BSDF.h"
#include "Parameters.h"

class Phong : public BSDF
{
public:
	Phong(const Parameters& list);
	~Phong();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	double pdf(const BSDFSamplingInfos& infos);

protected:
	double myKd;
	double myKs;
	double myAlpha;
	double myR;

	Color myAlbedo;
};

