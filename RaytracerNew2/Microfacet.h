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

	double pdf(const BSDFSamplingInfos& infos);

protected:
	double distributionBlinn(const Vector3d& hr);
	double distributionBeckmann(const Vector3d& hr);

	double shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh);
	double shadowingTermG1(const Vector3d& v, const Vector3d& m);

	Color fresnel(double myEtaExt, double myEtaInt, double cosThetaI);

protected:
	Texture::ptr myKdTexture;
	//Color myKd;
	double myKs;
	double myAlpha;
	//double myR;
	double myEtaInt;
	double myEtaExt;
	double myProbabilitySpecular;

	MicrofacetDistribution myDistribution;
};

