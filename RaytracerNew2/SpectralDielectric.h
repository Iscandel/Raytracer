#pragma once
#include "BSDF.h"

#include "Parameters.h"
#include "Rng.h"

class SpectralDielectric :
	public BSDF
{
public:
	SpectralDielectric(const Parameters& params);
	~SpectralDielectric();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;

protected:
	Texture::ptr myReflectanceTexture;
	Texture::ptr myTransmittedTexture;

	real myEtaI;
	Color myEtaT;

	Rng myRng;
};
