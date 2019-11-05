#pragma once
#include "BSDF.h"

#include "texture/Texture.h"

class Parameters;

class AlphaBSDF :
	public BSDF
{
public:
	AlphaBSDF(const Parameters& params);
	~AlphaBSDF();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;

protected:
	Texture::ptr myAlphaMap;
	BSDF::ptr myBSDF;
};

