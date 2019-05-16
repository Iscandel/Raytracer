#pragma once
#include "BSDF.h"
#include "Parameters.h"

class Diffuse :
	public BSDF
{
public:
	Diffuse(const Parameters& list);
	~Diffuse();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	real pdf(const BSDFSamplingInfos& infos);

protected:
	//Color myAlbedo;
	Texture::ptr myAlbedoTexture;
};

