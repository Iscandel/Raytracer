#pragma once
#include "BSDF.h"

#include "core/Parameters.h"

class SheetDielectric :
	public BSDF
{
public:
	SheetDielectric(const Parameters& params);
	~SheetDielectric();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;

protected:
	Texture::ptr myReflectanceTexture;
	Texture::ptr myTransmittedTexture;

	real myEtaI;
	real myEtaT;
};