#pragma once
#include "BSDF.h"
#include "texture/Texture.h"

class Parameters;

class Conductor :
	public BSDF
{
public:
	Conductor(const Parameters& params);
	~Conductor();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;

protected:
	Texture::ptr myReflectanceTexture;
	Color myEta;
	Color myAbsorption;
};

