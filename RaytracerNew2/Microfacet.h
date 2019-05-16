#pragma once
#include "BSDF.h"
#include "Parameters.h"
#include "MicrofacetDistribution.h"

class Microfacet :
	public BSDF
{
public:
	Microfacet(const Parameters& params);
	~Microfacet();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	real pdf(const BSDFSamplingInfos& infos);

protected:
	real distributionBlinn(const Vector3d& hr);
	real distributionBeckmann(const Vector3d& hr);

	real shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh);
	real shadowingTermG1(const Vector3d& v, const Vector3d& m);

	Color fresnel(real myEtaExt, real myEtaInt, real cosThetaI);

protected:
	Texture::ptr myKdTexture;
	//Color myKd;
	real myKs;
	real myAlpha;
	//real myR;
	real myEtaInt;
	real myEtaExt;
	real myProbabilitySpecular;

	MicrofacetDistribution myDistribution;
};

