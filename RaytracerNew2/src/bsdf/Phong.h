#pragma once

#include "BSDF.h"
#include "core/Parameters.h"
#include "texture/Texture.h"

class Phong : public BSDF
{
public:
	Phong(const Parameters& list);
	~Phong();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	real pdf(const BSDFSamplingInfos& infos);

protected:
	Texture::ptr myKd;
	Texture::ptr myKs;
	real myN;
	real myR;
	real mySamplingWeight;
};

