#pragma once

#include "BSDF.h"
#include "core/Parameters.h"

class Phong : public BSDF
{
public:
	Phong(const Parameters& list);
	~Phong();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	real pdf(const BSDFSamplingInfos& infos);

protected:
	real myKd;
	real myKs;
	real myAlpha;
	real myR;

	Color myAlbedo;
};

