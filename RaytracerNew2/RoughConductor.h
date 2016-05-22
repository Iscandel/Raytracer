#pragma once
#include "BSDF.h"

#include "MicrofacetDistribution.h"
#include "Parameters.h"
#include "Texture.h"

class RoughConductor :
	public BSDF
{
public:
	RoughConductor(const Parameters& params);
	~RoughConductor();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	double pdf(const BSDFSamplingInfos& infos);

protected:
	double distributionBeckmann(const Vector3d& hr);

	double shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh);
	double shadowingTermG1(const Vector3d& v, const Vector3d& m);

protected:
	Texture::ptr myReflectanceTexture;
	double myAlpha;
	double myR;
	Color myEta;
	Color myAbsorption;
	MicrofacetDistribution myDistribution;
};

