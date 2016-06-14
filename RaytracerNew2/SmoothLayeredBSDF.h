#pragma once
#pragma once
#include "BSDF.h"

#include "Rng.h"
#include "RoughDielectric.h"
#include "RoughConductor.h"

class Parameters;

class SmoothLayeredBSDF :
	public BSDF
{
public:
	SmoothLayeredBSDF(const Parameters& params);
	~SmoothLayeredBSDF();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	double pdf(const BSDFSamplingInfos& infos) override;
	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	Color fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT);

	Color evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha);

	Color eval2(const BSDFSamplingInfos & infos);

	RoughDielectric dielectric;
	RoughConductor conductor;

	MicrofacetDistribution myDistribution;

	double myEtaExt;
	double myEtaInt;
	double myAlpha;
	double myThickness;
	Color myAbsorbance;
	Rng myRng;

	Color myEta;
	Color myAbsorption;
};


