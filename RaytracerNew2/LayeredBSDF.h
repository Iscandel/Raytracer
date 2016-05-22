#pragma once
#include "BSDF.h"

#include "RoughDielectric.h"
#include "RoughConductor.h"

class Parameters;

class LayeredBSDF :
	public BSDF
{
public:
	LayeredBSDF(const Parameters& params);
	~LayeredBSDF();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	double pdf(const BSDFSamplingInfos& infos) override;

	double shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh);

	double shadowingTermG1(const Vector3d& v, const Vector3d& m);

	RoughDielectric dielectric;
	RoughConductor conductor;

	double myEtaI;
	double myEtaT;
	double myAlpha;
	double myThickness;
};

